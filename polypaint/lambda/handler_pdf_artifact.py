"""
PDF artifact Lambda — derive a new immutable PDF artifact from a saved Color image.

V1 supports one subtype only:
- ColorSpread: left page text from metadata, right page selected Color artifact image.
"""
import json
import os
from datetime import datetime, timezone

import boto3

from shared import BUCKET, parse_body, ok_response, report_status
from spread_pdf import build_color_spread_pdf

s3 = boto3.client("s3")


def _utc_now_iso():
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _stringify_meta(value):
    if value is None:
        return ""
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (list, dict)):
        return json.dumps(value, separators=(",", ":"))
    return str(value)


def _phase(job_id, task_id, status, phase, phase_label, **extra):
    report_status(job_id, task_id, status, result_data={"phase": phase, "phase_label": phase_label, **extra})


def _parse_boolish(value, default=True):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def _omega_display(enabled, omega):
    if omega in ("", None):
        return ""
    return f"w={omega}" if enabled else "w=off"


def _source_display_name(meta, fallback_id):
    mode = str(meta.get("color_mode", "") or "")
    if mode == "solve_score":
        metric = str(meta.get("solve_metric", "") or "")
        q = str(meta.get("solve_score_quantile", "") or "")
        omega = str(meta.get("solve_score_omega", "") or "")
        omega_enabled = _parse_boolish(meta.get("solve_score_omega_enabled", True), True)
        palette = str(meta.get("palette", "") or "")
        parts = [f"solve:{metric}" if metric else "solve"]
        if q:
            try:
                parts.append(f"q={float(q) * 100:.1f}%")
            except Exception:
                parts.append(f"q={q}")
        if omega:
            parts.append(_omega_display(omega_enabled, omega))
        if palette:
            parts.append(palette)
        return " ".join(parts).strip()
    if mode == "saved_palette":
        metric = str(meta.get("palette_source_metric") or meta.get("solve_metric") or "")
        q = meta.get("palette_source_quantile") or meta.get("solve_score_quantile") or ""
        omega = meta.get("palette_source_omega") or meta.get("solve_score_omega") or ""
        omega_enabled = _parse_boolish(
            meta.get("palette_source_omega_enabled", meta.get("solve_score_omega_enabled", True)),
            True,
        )
        palette = str(meta.get("palette", "") or "")
        parts = [f"saved:{metric}" if metric else "saved"]
        if q:
            try:
                parts.append(f"q={float(q) * 100:.1f}%")
            except Exception:
                parts.append(f"q={q}")
        if omega:
            parts.append(_omega_display(omega_enabled, omega))
        if palette:
            parts.append(palette)
        return " ".join(parts).strip()
    if mode == "proximity":
        palette = str(meta.get("palette", "") or "")
        return ("prox " + palette).strip() or fallback_id
    return fallback_id


def _title_from(calc, src_meta):
    fn = str(calc.get("function", "") or "").strip()
    degree = calc.get("degree")
    if fn and degree not in ("", None):
        return f"{fn} - degree {degree}"
    if fn:
        return fn
    return str(src_meta.get("artifact_id", "") or "Color Spread")


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


def _body_from(job_id, calc, src_meta, created_at):
    lines = [
        f"Job {job_id}.",
        f"Artifact {src_meta.get('artifact_id', '')}.",
    ]
    fn = calc.get("function")
    if fn:
        lines.append(f"Function {fn}.")
    degree = calc.get("degree")
    if degree not in ("", None):
        lines.append(f"Degree {degree}.")
    n_val = calc.get("N", calc.get("n1"))
    if n_val not in ("", None):
        lines.append(f"N {n_val}.")
    times = calc.get("times")
    if times not in ("", None):
        lines.append(f"Times {times}.")
    color_mode = src_meta.get("color_mode")
    if color_mode:
        lines.append(f"Mode {color_mode}.")
    palette = src_meta.get("palette")
    if palette:
        lines.append(f"Palette {palette}.")
    metric = src_meta.get("solve_metric") or src_meta.get("palette_source_metric")
    if metric:
        lines.append(f"Metric {metric}.")
    q = src_meta.get("solve_score_quantile") or src_meta.get("palette_source_quantile")
    if q not in ("", None):
        try:
            lines.append(f"q {float(q) * 100:.1f} percent.")
        except Exception:
            lines.append(f"q {q}.")
    omega = src_meta.get("solve_score_omega") or src_meta.get("palette_source_omega")
    omega_enabled = _parse_boolish(
        src_meta.get("solve_score_omega_enabled", src_meta.get("palette_source_omega_enabled", True)),
        True,
    )
    if omega not in ("", None):
        lines.append(f"Omega {_omega_display(omega_enabled, omega).replace('w=', '')}.")
    lines.append(f"Transforms {_transforms_summary(src_meta.get('root_transforms'))}.")
    lines.append(f"Created {created_at}.")
    return " ".join(str(x).strip() for x in lines if str(x).strip())


def _build_spread_meta(job_id, calc, src_meta, source_artifact_id):
    """Build structured metadata dict for the PDF text page."""
    # Pipeline line: [transforms] function [coeff_transforms] N=..., times=...
    pipeline_parts = []
    pt = _transforms_summary(src_meta.get("root_transforms"))
    fn = str(calc.get("function", "") or "").strip()
    pipeline = calc.get("pipeline", {}) if isinstance(calc.get("pipeline"), dict) else {}
    ct_raw = pipeline.get("coeff_transforms", [])
    ct = ",".join(str(x) for x in ct_raw) if ct_raw else ""
    cfpv_display = pipeline.get("cfpv_display", "")
    fn_str = fn
    if cfpv_display:
        fn_str += f"({cfpv_display})"
    pt_display = pipeline.get("param_transforms_display", [])
    if pt_display:
        pt_parts = []
        for a in pt_display:
            if isinstance(a, list) and len(a) > 1:
                pt_parts.append(f"{a[0]}({','.join(str(v) for v in a[1:])})")
            elif isinstance(a, list) and a:
                pt_parts.append(str(a[0]))
            else:
                pt_parts.append(str(a))
        pt_str = ",".join(pt_parts)
    else:
        pt_str = pt
    pipeline_line = f"[{pt_str}] {fn_str}"
    if ct:
        pipeline_line += f" [{ct}]"
    n_val = calc.get("N", calc.get("n1", ""))
    times = calc.get("times", 1)
    if n_val not in ("", None):
        pipeline_line += f" N={n_val}"
    if times not in ("", None):
        pipeline_line += f", times={times}"

    # Viewport line
    vp_parts = []
    quantile_val = src_meta.get("quantile", "")
    shim_val = src_meta.get("shim", "")
    view_mode = src_meta.get("view_mode", "auto")
    sq_ext = src_meta.get("square_extent", "")
    if view_mode == "square" and sq_ext:
        vp_parts.append(f"Square extent={sq_ext}")
    else:
        if quantile_val not in ("", None):
            try:
                vp_parts.append(f"q={float(quantile_val)*100:.1f}%")
            except Exception:
                pass
        if shim_val not in ("", None):
            try:
                vp_parts.append(f"shim={float(shim_val)*100:.1f}%")
            except Exception:
                pass
    viewport_line = "View: " + ", ".join(vp_parts) if vp_parts else ""

    # Color mode line
    color_mode = str(src_meta.get("color_mode", "") or "").strip()
    palette = str(src_meta.get("palette", "") or "")
    if color_mode == "solve_score":
        metric = str(src_meta.get("solve_metric", "") or "")
        q = src_meta.get("solve_score_quantile", "")
        omega = src_meta.get("solve_score_omega", "")
        omega_enabled = _parse_boolish(src_meta.get("solve_score_omega_enabled", True), True)
        cm_parts = [f"SOLVE SCORE: {metric}"]
        if q not in ("", None):
            try:
                cm_parts.append(f"q={float(q)*100:.1f}%")
            except Exception:
                pass
        if omega not in ("", None):
            cm_parts.append(_omega_display(omega_enabled, omega))
        if palette:
            cm_parts.append(palette)
        color_line = " ".join(cm_parts)
    elif color_mode == "saved_palette":
        metric = str(src_meta.get("palette_source_metric") or src_meta.get("solve_metric") or "")
        color_line = f"SAVED PALETTE: {metric}"
        if palette:
            color_line += f" {palette}"
    elif color_mode == "proximity":
        color_line = f"ROOT PROXIMITY: {palette}" if palette else "ROOT PROXIMITY"
    elif color_mode == "constant":
        cc = str(src_meta.get("constant_color", "") or "")
        color_line = f"CONSTANT: #{cc}" if cc else "CONSTANT"
    else:
        color_line = "RAINBOW"

    # Degree line
    degree = calc.get("degree", "")
    degree_line = f"Degree: {degree}" if degree not in ("", None) else ""

    return {
        "pipeline": pipeline_line,
        "viewport": viewport_line,
        "color_mode": color_line,
        "degree": degree_line,
        "artifact_id": source_artifact_id,
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
    output_local = "/tmp/pdf_document.pdf"

    progress = {
        "family": "pdf",
        "artifact_id": artifact_id,
        "source_artifact_id": source_artifact_id,
        "pdf_kind": "color_spread",
    }

    try:
        _phase(job_id, task_id, "started", "pdf", "ColorSpread", **progress)

        src_head = s3.head_object(Bucket=BUCKET, Key=source_image_key)
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

        try:
            calc_obj = s3.get_object(Bucket=BUCKET, Key=f"renders/{job_id}/calc.json")
            calc = json.loads(calc_obj["Body"].read())
        except Exception:
            calc = {}

        title = _title_from(calc, src_meta)
        filename = os.path.splitext(os.path.basename(source_image_key))[0]
        source_display_name = _source_display_name(src_meta, source_artifact_id)

        # Build structured metadata for the text page
        spread_meta = _build_spread_meta(job_id, calc, src_meta, source_artifact_id)

        _phase(job_id, task_id, "processing", "compose_pdf", "Compose PDF", **progress)
        build_color_spread_pdf(source_local, output_local, title, meta=spread_meta)

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
            "source_solve_metric": src_meta.get("solve_metric", ""),
            "source_solve_score_quantile": src_meta.get("solve_score_quantile", ""),
            "source_solve_score_omega": src_meta.get("solve_score_omega", ""),
            "source_solve_score_omega_enabled": "true" if _parse_boolish(src_meta.get("solve_score_omega_enabled", True), True) else "false",
            "source_root_transforms": _stringify_meta(src_meta.get("root_transforms", "")),
            "page_count": "1",
            "width_mm": "586",
            "height_mm": "296",
            "function": _stringify_meta(calc.get("function")),
            "degree": _stringify_meta(calc.get("degree")),
            "N": _stringify_meta(calc.get("N", calc.get("n1"))),
            "times": _stringify_meta(calc.get("times")),
        }

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

        _phase(job_id, task_id, "done", "done", "Done", **progress, image_key=pdf_key)
        return ok_response({
            "job_id": job_id,
            "artifact_id": artifact_id,
            "family": "pdf",
            "image_key": pdf_key,
            "format": "pdf",
            "pdf_kind": "color_spread",
        })
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        for path in (source_local, output_local):
            try:
                os.remove(path)
            except OSError:
                pass
