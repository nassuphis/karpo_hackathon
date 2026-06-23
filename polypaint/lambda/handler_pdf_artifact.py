"""
PDF artifact Lambda — derive a new immutable PDF artifact from a saved Color image.

V1 supports one subtype only:
- ColorSpread: left page metadata/text, optional centered 5 cm palette square
  beneath that text, right page selected Color artifact image.
"""
import json
import os
from datetime import datetime, timezone

import boto3

from color_artifact_meta import load_color_artifact_head
from coeff_program_source import coeff_source_text_from_chain
from param_program_source import param_source_text_from_chain
from pipeline_programs import coeff_transforms_to_program_chain, param_transforms_to_program_chain
from root_program_source import root_source_text_from_chain
from shared import BUCKET, parse_body, ok_response, report_status
from solve_score_chain import compile_solve_score_chain_or_legacy, read_solve_score_metadata, serialize_solve_score_chain
from solve_score_program_source import solve_score_source_text_from_chain
from spread_pdf import PDF_IMAGE_MAX_PX, PDF_PALETTE_MAX_PX, build_color_spread_pdf, prepare_pdf_image

s3 = boto3.client("s3")


def _utc_now_iso():
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _stringify_meta(value):
    if value is None:
        return ""
    if isinstance(value, bool):
        text = "true" if value else "false"
    elif isinstance(value, (list, dict)):
        text = json.dumps(value, ensure_ascii=True, separators=(",", ":"))
    else:
        text = str(value)
    replacements = {
        "\u03c9": "omega",
        "\u03a9": "Omega",
        "\u2010": "-",
        "\u2011": "-",
        "\u2012": "-",
        "\u2013": "-",
        "\u2014": "-",
        "\u2212": "-",
        "\u2026": "...",
    }
    for src, dst in replacements.items():
        text = text.replace(src, dst)
    return text.encode("ascii", "replace").decode("ascii")


def _phase(job_id, task_id, status, phase, phase_label, **extra):
    report_status(job_id, task_id, status, result_data={"phase": phase, "phase_label": phase_label, **extra})


def _omega_display(enabled, omega):
    if omega in ("", None):
        return ""
    return f"w={omega}" if enabled else "w=off"


def _source_display_name(meta, fallback_id):
    mode = str(meta.get("color_mode", "") or "")
    if mode == "solve_score":
        try:
            score = read_solve_score_metadata("solve", meta, default_metric="proximity")
        except Exception:
            score = None
        palette = str(meta.get("palette", "") or "")
        parts = [f"solve:{score['display']}" if score and score.get("display") else "solve"]
        if palette:
            parts.append(palette)
        return " ".join(parts).strip()
    if mode == "saved_palette":
        try:
            score = read_solve_score_metadata("palette_source", meta, default_metric="proximity")
        except Exception:
            score = None
        palette = str(meta.get("palette", "") or "")
        parts = [f"saved:{score['display']}" if score and score.get("display") else "saved"]
        if palette:
            parts.append(palette)
        return " ".join(parts).strip()
    if mode == "proximity":
        palette = str(meta.get("palette", "") or "")
        return ("prox " + palette).strip() or fallback_id
    return fallback_id


def _title_from(calc, src_meta):
    return "PolyPaint Lambda 1.0"


def _solver_label(raw):
    solver = str(raw or "").strip()
    if solver == "aberth":
        return "AE"
    if solver == "companion_matrix":
        return "CM"
    if solver == "aberth_mt":
        return "AE-MT"
    return solver


def _transforms_summary(raw):
    try:
        rt = json.loads(raw) if isinstance(raw, str) else (raw or [])
    except Exception:
        return ""
    if not isinstance(rt, list) or not rt:
        return "none"
    parts = []
    for item in rt:
        if not isinstance(item, list) or not item:
            continue
        name = str(item[0])
        if len(item) > 1:
            parts.append(name + "(" + ",".join(str(v) for v in item[1:]) + ")")
        else:
            parts.append(name)
    return "; ".join(parts) if parts else "none"


def _first_text(*values):
    for value in values:
        text = str(value or "").strip()
        if text:
            return text
    return ""


def _fmt_pct(value):
    if value in ("", None):
        return ""
    try:
        return f"{float(value) * 100:.1f}%"
    except Exception:
        return str(value)


def _viewport_summary(src_meta):
    view_mode = str(src_meta.get("view_mode") or "auto")
    if view_mode == "square":
        extent = str(src_meta.get("square_extent") or "").strip()
        return f"square extent={extent}" if extent else "square"
    parts = [view_mode]
    q = _fmt_pct(src_meta.get("quantile"))
    shim = _fmt_pct(src_meta.get("shim"))
    if q:
        parts.append(f"q={q}")
    if shim:
        parts.append(f"shim={shim}")
    bounds = []
    for key in ("min_re", "max_re", "min_im", "max_im"):
        if src_meta.get(key) not in ("", None):
            bounds.append(f"{key}={src_meta.get(key)}")
    if bounds:
        parts.append(" ".join(bounds))
    return ", ".join(p for p in parts if p)


def _color_summary(src_meta):
    color_mode = str(src_meta.get("color_mode") or "").strip()
    palette = str(src_meta.get("palette") or "").strip()
    if color_mode == "solve_score":
        try:
            score = read_solve_score_metadata("solve", src_meta, default_metric="proximity")
            return str(score.get("display") or score.get("metric") or "score")
        except Exception:
            return "score"
    if color_mode == "saved_palette":
        try:
            score = read_solve_score_metadata("palette_source", src_meta, default_metric="proximity")
            return f"saved palette: {score.get('display') or score.get('metric') or 'score'}"
        except Exception:
            return "saved palette"
    if color_mode == "proximity":
        return "root proximity"
    if color_mode == "constant":
        color = str(src_meta.get("constant_color") or "").strip()
        return f"constant #{color}" if color else "constant"
    return color_mode or ("palette " + palette if palette else "rainbow")


def _program_entry(label, source="", fallback="", language="text"):
    source = str(source or "").strip()
    fallback = str(fallback or "").strip()
    if not source and not fallback:
        return None
    return {
        "label": label,
        "language": language,
        "source": source,
        "fallback": fallback,
    }


def _program_entry_always(label, source="", fallback="", language="text", empty_text="none"):
    entry = _program_entry(label, source, fallback, language)
    if entry:
        return entry
    return {
        "label": label,
        "language": language,
        "source": str(empty_text or "none"),
        "fallback": "",
    }


def _program_fallback_from_chain(label, display, chain):
    display_text = str(display or "").strip()
    if display_text:
        return display_text
    if chain not in ("", None, []):
        try:
            return f"{label} chain display not stored; {len(chain)} chip(s)"
        except Exception:
            return f"{label} chain display not stored"
    return ""


def _program_source_from_chain(chain, to_source):
    if not isinstance(chain, list) or not chain:
        return ""
    try:
        return str(to_source(chain) or "").strip()
    except Exception:
        return ""


def _program_chain_from_pipeline(pipeline, profile):
    chain = pipeline.get(f"{profile}_program_chain")
    return chain if isinstance(chain, list) and chain else []


def _param_source_from_pipeline(pipeline):
    source = _first_text(pipeline.get("param_program_source_text"))
    if source:
        return source
    chain = _program_chain_from_pipeline(pipeline, "param")
    source = _program_source_from_chain(chain, param_source_text_from_chain)
    if source:
        return source
    transforms = pipeline.get("param_transforms")
    if isinstance(transforms, list) and transforms:
        return _program_source_from_chain(param_transforms_to_program_chain(transforms), param_source_text_from_chain)
    return ""


def _coeff_source_from_pipeline(pipeline):
    source = _first_text(pipeline.get("coeff_program_source_text"))
    if source:
        return source
    chain = _program_chain_from_pipeline(pipeline, "coeff")
    source = _program_source_from_chain(chain, coeff_source_text_from_chain)
    if source:
        return source
    transforms = pipeline.get("coeff_transforms")
    if isinstance(transforms, list) and transforms:
        return _program_source_from_chain(coeff_transforms_to_program_chain(transforms), coeff_source_text_from_chain)
    return ""


def _root_source_from_meta(meta):
    meta = meta or {}
    source = _first_text(meta.get("root_program_source_text"))
    if source:
        return source
    raw_chain = meta.get("root_program_chain")
    if raw_chain not in ("", None, []):
        try:
            chain = json.loads(raw_chain) if isinstance(raw_chain, str) else raw_chain
            source = _program_source_from_chain(chain, root_source_text_from_chain)
            if source:
                return source
        except Exception:
            pass
    return ""


def _legacy_solve_score_source_from_meta(meta):
    meta = meta or {}
    # Older color artifacts predate solve_score_chain/source metadata and only
    # carry scalar fields. Compile through the same legacy bridge used by
    # Populate so PDF provenance matches the editable Render text box.
    metric = _first_text(meta.get("solve_metric"))
    if not metric:
        return ""
    omega_enabled = meta.get("solve_score_omega_enabled")
    if omega_enabled is None:
        omega_enabled = True
    try:
        compiled = compile_solve_score_chain_or_legacy(
            raw_chain=[],
            metric=metric,
            quantile=meta.get("solve_score_quantile"),
            omega=meta.get("solve_score_omega"),
            omega_enabled=omega_enabled,
            default_metric="proximity",
        )
        public_chain = json.loads(serialize_solve_score_chain(compiled["chain"]))
        return solve_score_source_text_from_chain(public_chain)
    except Exception:
        return ""


def _solve_score_source_from_meta(meta):
    source = _first_text(
        (meta or {}).get("solve_score_program_source_text"),
        (meta or {}).get("score_source_text"),
    )
    if source:
        return source
    raw_chain = (meta or {}).get("solve_score_chain")
    if raw_chain not in ("", None, []):
        try:
            return solve_score_source_text_from_chain(raw_chain)
        except Exception:
            return ""
    return _legacy_solve_score_source_from_meta(meta)


def build_pdf_report_model(job_id, calc, src_meta, source_artifact_id):
    calc = calc if isinstance(calc, dict) else {}
    src_meta = src_meta if isinstance(src_meta, dict) else {}
    pipeline = calc.get("pipeline") if isinstance(calc.get("pipeline"), dict) else {}
    function_name = _first_text(pipeline.get("function"), calc.get("function"), "?")
    cfpv_display = _first_text(pipeline.get("cfpv_display"))
    function_display = f"{function_name}({cfpv_display})" if cfpv_display else function_name
    degree = _first_text(calc.get("degree"), calc.get("probe_degree"))
    n_coeffs = _first_text(calc.get("n_coeffs"), calc.get("probe_n_coeffs"))
    n_value = _first_text(calc.get("N"), calc.get("n1"))
    times = _first_text(calc.get("times"))
    solver = _solver_label(calc.get("solver"))
    color_summary = _color_summary(src_meta)
    color_mode = _first_text(src_meta.get("color_mode"), color_summary)
    interpretation = _first_text(src_meta.get("color_interpretation"), src_meta.get("score_output_interpretation"))
    output_channels = _first_text(src_meta.get("score_output_channel_count"), src_meta.get("raw_channels"))
    palette = _first_text(src_meta.get("palette"))
    palette_label = _first_text(
        src_meta.get("associated_palette_id"),
        src_meta.get("palette_artifact_id"),
        src_meta.get("palette_id"),
        palette,
    )
    viewport = _viewport_summary(src_meta)

    summary_rows = [
        ("Function", function_display),
        ("Degree", degree),
        ("N", n_value),
        ("Times", times),
        ("Solver", solver),
        ("Interpretation", interpretation),
        ("Palette", palette),
        ("Output channels", output_channels),
        ("Viewport", viewport),
    ]
    if str(src_meta.get("color_mode") or "").strip() != "solve_score":
        summary_rows.insert(5, ("Color mode", color_summary or color_mode))

    programs = []
    param_source = _param_source_from_pipeline(pipeline)
    param_fallback = _program_fallback_from_chain(
        "Param Program",
        pipeline.get("param_program_display", ""),
        pipeline.get("param_program_chain", []),
    )
    programs.append(_program_entry_always("Param Program", param_source, param_fallback, "poly-param"))

    coeff_source = _coeff_source_from_pipeline(pipeline)
    coeff_fallback = _program_fallback_from_chain(
        "Coeff Program",
        pipeline.get("coeff_program_display", ""),
        pipeline.get("coeff_program_chain", []),
    )
    programs.append(_program_entry_always("Coeff Program", coeff_source, coeff_fallback, "poly-coeff"))

    root_source = _root_source_from_meta(src_meta)
    root_fallback = _transforms_summary(src_meta.get("root_transforms"))
    if root_fallback == "none":
        root_fallback = ""
    programs.append(_program_entry_always("Root Program", root_source, root_fallback, "poly-root"))

    score_source = _solve_score_source_from_meta(src_meta)
    score_fallback = _first_text(src_meta.get("score_program"))
    entry = _program_entry("Solve Score Program", score_source, score_fallback, "poly-score")
    if entry:
        programs.append(entry)

    return {
        "title": f"{job_id} / {source_artifact_id}",
        "compute_id": job_id,
        "color_artifact_id": source_artifact_id,
        "palette_label": palette_label,
        "summary_rows": summary_rows,
        "programs": [p for p in programs if p],
    }


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    task_id = params["task_id"]
    artifact_id = params["artifact_id"]
    source_artifact_id = params["source_artifact_id"]
    source_image_key = params["source_image_key"]

    prefix = f"renders/{job_id}/pdf/{artifact_id}/"
    pdf_key = prefix + "document.pdf"
    created_at = _utc_now_iso()
    source_ext = source_image_key.rsplit(".", 1)[-1].lower()
    source_local = f"/tmp/pdf_source.{source_ext}"
    prepared_source_local = "/tmp/pdf_source_prepared.png"
    output_local = "/tmp/pdf_document.pdf"
    palette_local = None
    prepared_palette_local = None

    progress = {
        "family": "pdf",
        "artifact_id": artifact_id,
        "source_artifact_id": source_artifact_id,
        "pdf_kind": "color_spread",
    }

    try:
        _phase(job_id, task_id, "started", "pdf", "ColorSpread", **progress)

        src_head = s3.head_object(Bucket=BUCKET, Key=source_image_key)
        try:
            src_meta = dict(load_color_artifact_head(s3, BUCKET, job_id, source_artifact_id).get("metadata", {}) or {})
        except Exception:
            src_meta = dict(src_head.get("Metadata", {}) or {})
        src_family = str(src_meta.get("family", "") or "color")
        if src_family not in ("", "color"):
            raise RuntimeError(f"ColorSpread requires Color source, got {src_family!r}")
        if src_meta.get("artifact_id") and src_meta.get("artifact_id") != source_artifact_id:
            raise RuntimeError(
                f"Source artifact mismatch: expected {source_artifact_id}, found {src_meta.get('artifact_id')}"
            )

        _phase(job_id, task_id, "downloading", "load_source", "Load source", **progress)
        obj = s3.get_object(Bucket=BUCKET, Key=source_image_key)
        with open(source_local, "wb") as fh:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                fh.write(chunk)

        associated_palette_image_key = str(src_meta.get("associated_palette_image_key") or "").strip()
        associated_palette_mode = str(src_meta.get("associated_palette_mode") or "").strip()
        associated_palette_id = str(src_meta.get("associated_palette_id") or "").strip()
        if associated_palette_image_key:
            palette_ext = associated_palette_image_key.rsplit(".", 1)[-1].lower()
            palette_local = f"/tmp/pdf_palette.{palette_ext}"
            try:
                pal_obj = s3.get_object(Bucket=BUCKET, Key=associated_palette_image_key)
            except Exception as e:
                raise RuntimeError(
                    f"Failed to download associated palette image s3://{BUCKET}/{associated_palette_image_key}: {e}"
                ) from e
            with open(palette_local, "wb") as pf:
                for chunk in pal_obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                    pf.write(chunk)

        try:
            calc_obj = s3.get_object(Bucket=BUCKET, Key=f"renders/{job_id}/calc.json")
            calc = json.loads(calc_obj["Body"].read())
        except Exception:
            calc = {}

        _phase(job_id, task_id, "processing", "prepare_image", "Prepare PDF images", **progress)
        source_image_info = prepare_pdf_image(
            source_local,
            prepared_source_local,
            max_px=int(os.getenv("PDF_IMAGE_MAX_PX", PDF_IMAGE_MAX_PX)),
        )
        palette_image_info = {}
        if palette_local:
            prepared_palette_local = "/tmp/pdf_palette_prepared.png"
            palette_image_info = prepare_pdf_image(
                palette_local,
                prepared_palette_local,
                max_px=int(os.getenv("PDF_PALETTE_MAX_PX", PDF_PALETTE_MAX_PX)),
            )
        image_progress = {
            "source_width": source_image_info["source_width"],
            "source_height": source_image_info["source_height"],
            "prepared_width": source_image_info["prepared_width"],
            "prepared_height": source_image_info["prepared_height"],
            "image_resized": bool(source_image_info["resized"]),
            "image_max_px": source_image_info["image_max_px"],
        }
        if palette_image_info:
            image_progress.update({
                "palette_source_width": palette_image_info["source_width"],
                "palette_source_height": palette_image_info["source_height"],
                "palette_prepared_width": palette_image_info["prepared_width"],
                "palette_prepared_height": palette_image_info["prepared_height"],
                "palette_image_resized": bool(palette_image_info["resized"]),
                "palette_image_max_px": palette_image_info["image_max_px"],
            })
        _phase(job_id, task_id, "processing", "prepare_image", "Prepare PDF images", **progress, **image_progress)

        report = build_pdf_report_model(job_id, calc, src_meta, source_artifact_id)
        title = report.get("title") or _title_from(calc, src_meta)
        source_display_name = _source_display_name(src_meta, source_artifact_id)
        try:
            source_score = read_solve_score_metadata("solve", src_meta, default_metric="proximity")
        except Exception:
            source_score = None

        _phase(job_id, task_id, "processing", "compose_pdf", "Compose PDF", **progress)
        build_result = build_color_spread_pdf(
            prepared_source_local,
            output_local,
            title,
            report=report,
            palette_image_path=prepared_palette_local,
        )
        page_count = 1
        if isinstance(build_result, dict):
            try:
                page_count = int(build_result.get("page_count") or 1)
            except Exception:
                page_count = 1

        meta = {
            "family": "pdf",
            "artifact_id": artifact_id,
            "created_at": created_at,
            "format": "pdf",
            "pdf_kind": "color_spread",
            "source_family": "color",
            "source_artifact_id": source_artifact_id,
            "source_image_key": source_image_key,
            "source_display_name": source_display_name,
            "source_color_mode": src_meta.get("color_mode", ""),
            "source_palette": src_meta.get("palette", ""),
            "source_solve_metric": source_score["metric"] if source_score else "",
            "source_solve_score_quantile": _stringify_meta(source_score["quantile"] if source_score else ""),
            "source_solve_score_omega": _stringify_meta(source_score["omega"] if source_score else ""),
            "source_solve_score_omega_enabled": "true" if (source_score["omega_enabled"] if source_score else True) else "false",
            "source_root_transforms": _stringify_meta(src_meta.get("root_transforms", "")),
            "source_associated_palette_mode": associated_palette_mode,
            "source_associated_palette_id": associated_palette_id,
            "source_associated_palette_image_key": associated_palette_image_key,
            "page_count": _stringify_meta(page_count),
            "width_mm": "586",
            "height_mm": "296",
            "function": _stringify_meta(_first_text(
                (calc.get("pipeline") or {}).get("function") if isinstance(calc.get("pipeline"), dict) else "",
                calc.get("function"),
            )),
            "degree": _stringify_meta(calc.get("degree")),
            "N": _stringify_meta(calc.get("N", calc.get("n1"))),
            "times": _stringify_meta(calc.get("times")),
            "source_width": _stringify_meta(source_image_info["source_width"]),
            "source_height": _stringify_meta(source_image_info["source_height"]),
            "prepared_width": _stringify_meta(source_image_info["prepared_width"]),
            "prepared_height": _stringify_meta(source_image_info["prepared_height"]),
            "image_resized": _stringify_meta(source_image_info["resized"]),
            "image_max_px": _stringify_meta(source_image_info["image_max_px"]),
        }
        if palette_image_info:
            meta.update({
                "palette_source_width": _stringify_meta(palette_image_info["source_width"]),
                "palette_source_height": _stringify_meta(palette_image_info["source_height"]),
                "palette_prepared_width": _stringify_meta(palette_image_info["prepared_width"]),
                "palette_prepared_height": _stringify_meta(palette_image_info["prepared_height"]),
                "palette_image_resized": _stringify_meta(palette_image_info["resized"]),
                "palette_image_max_px": _stringify_meta(palette_image_info["image_max_px"]),
            })
        meta = {str(key): _stringify_meta(value) for key, value in meta.items()}

        _phase(job_id, task_id, "uploading", "upload", "Upload PDF", **progress)
        with open(output_local, "rb") as fh:
            s3.upload_fileobj(
                fh,
                BUCKET,
                pdf_key,
                ExtraArgs={
                    "ContentType": "application/pdf",
                    "Metadata": meta,
                },
            )

        _phase(job_id, task_id, "done", "done", "Done", **progress, image_key=pdf_key, page_count=page_count, **image_progress)
        return ok_response({
            "job_id": job_id,
            "artifact_id": artifact_id,
            "family": "pdf",
            "image_key": pdf_key,
            "format": "pdf",
            "pdf_kind": "color_spread",
            "page_count": page_count,
        })
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        for path in (source_local, prepared_source_local, output_local, palette_local, prepared_palette_local):
            try:
                if path:
                    os.remove(path)
            except OSError:
                pass
