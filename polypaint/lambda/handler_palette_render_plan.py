"""
Palette render plan Lambda — computes the workflow plan for reusable full-resolution
palette artifacts.

The palette image/preview still uses pass 0 only, but the durable numeric payload is
all-pass chunk-local bin/score data so saved palettes can later drive Color renders.
"""
import hashlib
import json

import boto3

from calc_chunks import (
    build_chunk_items,
    fallback_lores_coeffs_key as calc_fallback_lores_coeffs_key,
    fallback_lores_params_key as calc_fallback_lores_params_key,
    fallback_params_global_key,
)
from color_artifact_meta import load_color_artifact_head, parse_root_transforms
from color_render_contract import normalize_color_interpretation, validate_color_output_contract
from logical_sections import build_physical_section_items, build_solve_source_manifest, write_solve_source_manifest
from palette_names import is_valid_palette_name, normalize_palette_display_name
from param_source import chunk_items_have_params
from pipeline_programs import root_program_for_run, solve_score_program_for_run
from shared import BUCKET, parse_body, ok_response
from solve_score_chain import (
    SOLVE_SCORE_SPEC_VERSION,
    VALID_SOLVE_SCORE_METRICS,
    compile_solve_score_chain_or_legacy,
    compiled_solve_score_fingerprint,
    emit_solve_score_metadata,
    format_solve_score_chain_display,
    public_solve_score_chain,
    read_solve_score_metadata,
    solve_score_chain_from_scalars,
    solve_score_lag_prelude_by_source,
    solve_score_uses_source,
    solve_score_uses_non_solve_sources,
    solve_score_chain_id,
)
from solve_score_program_source import solve_score_source_text_from_chain

s3 = boto3.client("s3")

MAX_PLAN_BYTES = 200 * 1024  # fail fast before 256KB Step Functions limit
VALID_METRICS = VALID_SOLVE_SCORE_METRICS
PALETTE_VARIANT_SPEC_VERSION = 1


def _root_program_payload(compiled):
    return {
        "program_kind": "root_program",
        "spec_version": int(compiled.get("spec_version") or 2),
        "source_text": str(compiled.get("source_text") or ""),
        "chain": compiled.get("chain") or [],
        "root_transforms": compiled.get("root_transforms") or [],
        "display": compiled.get("display") or "",
        "fingerprint": compiled.get("fingerprint") or "",
        "execution_spec": compiled.get("execution_spec") or "",
        "tokens": compiled.get("tokens") or [],
        "token_count": int(compiled.get("token_count") or 0),
    }


def _apply_root_program_to_params(params):
    compiled = root_program_for_run(params)
    payload = _root_program_payload(compiled)
    params["root_transforms"] = payload["root_transforms"]
    params["root_program_source_text"] = payload["source_text"]
    params["root_program"] = payload
    params["root_program_fingerprint"] = payload["fingerprint"]
    params["root_spec_version"] = payload["spec_version"]
    return payload


def _validate_threads(value, default=1):
    if value in (None, ""):
        return default
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_threads must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"solve_score_threads must be in [1, 16], got {threads}")
    return threads


def _validate_palette_chunk_threads(value, default=4):
    if value in (None, ""):
        value = default
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"palette_chunk_threads must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"palette_chunk_threads must be in [1, 16], got {threads}")
    return threads


def _validate_merge_workers(value, default=16):
    if value in (None, ""):
        value = default
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_merge_workers must be an integer, got {value!r}")
    if not (1 <= workers <= 64):
        raise RuntimeError(f"solve_score_merge_workers must be in [1, 64], got {workers}")
    return workers


def _validate_palette_chunk_workers(value, default=16):
    if value in (None, ""):
        value = default
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"palette_chunk_workers must be an integer, got {value!r}")
    if not (1 <= workers <= 64):
        raise RuntimeError(f"palette_chunk_workers must be in [1, 64], got {workers}")
    return workers


def _validate_hist_input_mode(value):
    mode = str(value or "tmpfile").strip().lower()
    if mode not in {"tmpfile", "stdin", "sectioned"}:
        raise RuntimeError(
            "solve_score_hist_input_mode must be one of sectioned, stdin, tmpfile, "
            f"got {value!r}"
        )
    return mode


def _validate_palette_chunk_input_mode(value):
    mode = str(value or "sectioned").strip().lower()
    if mode not in {"tmpfile", "sectioned"}:
        raise RuntimeError(
            "palette_chunk_input_mode must be one of sectioned, tmpfile, "
            f"got {value!r}"
        )
    return mode


def _validate_sectioned_retries(value):
    if value in (None, ""):
        value = 2
    try:
        retries = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_hist_retries must be an integer, got {value!r}")
    if not (0 <= retries <= 10):
        raise RuntimeError(f"solve_score_hist_retries must be in [0, 10], got {retries}")
    return retries


def _validate_omega(value):
    try:
        omega = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_omega must be numeric, got {value!r}")
    if not (omega == omega and abs(omega) != float("inf")):
        raise RuntimeError(f"solve_score_omega must be finite, got {value!r}")
    return omega


def _validate_omega_enabled(value):
    if value in (None, ""):
        return True
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    text = str(value).strip().lower()
    if text in ("1", "true", "yes", "on"):
        return True
    if text in ("0", "false", "no", "off"):
        return False
    raise RuntimeError(f"solve_score_omega_enabled must be boolean-like, got {value!r}")


def _slug(value, default="score"):
    text = str(value or default).strip().lower().replace(" ", "_")
    return "".join(ch if (ch.isalnum() or ch in "_-") else "_" for ch in text).strip("_") or default


def _interpretation_uses_palette(interpretation):
    return normalize_color_interpretation(interpretation) in {"scalar_lut", "rgb_lut", "hsv_lut"}


def _color_interpretation_from_params(pp, default="scalar_lut"):
    primary = pp.get("color_interpretation")
    alias = pp.get("score_output_interpretation")
    mode = normalize_color_interpretation(primary if primary not in ("", None) else default)
    if alias not in ("", None):
        alias_mode = normalize_color_interpretation(alias)
        if primary not in ("", None) and alias_mode != mode:
            raise RuntimeError(
                "color_interpretation and score_output_interpretation disagree: "
                f"{primary!r} vs {alias!r}"
            )
        mode = alias_mode
    return mode


def _palette_identity_payload(
    *,
    job_id,
    compiled_score_fingerprint,
    score_chain_public,
    metric,
    quantile,
    omega,
    omega_enabled,
    color_interpretation,
    output_channel_count,
    output_channels,
    palette,
    root_transforms,
    palette_display_name="",
):
    mode = normalize_color_interpretation(color_interpretation)
    payload = {
        "scheme": "palette_variant_id_v2",
        "job_id": str(job_id),
        "solve_score_fingerprint": str(compiled_score_fingerprint),
        "solve_score_chain": score_chain_public or [],
        "metric": str(metric or ""),
        "quantile": float(quantile),
        "omega": float(omega),
        "omega_enabled": bool(omega_enabled),
        "color_interpretation": mode,
        "output_channel_count": int(output_channel_count or 1),
        "output_channels": output_channels or [],
        "palette": str(palette or "") if _interpretation_uses_palette(mode) else "none",
        "root_transforms": root_transforms or [],
    }
    if palette_display_name:
        payload["palette_display_name"] = str(palette_display_name)
    return payload


def _palette_variant_identity(**kwargs):
    payload = _palette_identity_payload(**kwargs)
    encoded = json.dumps(payload, sort_keys=True, separators=(",", ":"))
    digest = hashlib.sha256(encoded.encode("utf-8")).hexdigest()
    palette_id = f"pal_{_slug(payload.get('metric'))}_{payload['color_interpretation']}_{digest[:12]}"
    return palette_id, f"sha256:{digest}", payload


def _omega_display(enabled, omega):
    return f"w={float(omega):g}" if enabled else "w=off"


def _palette_display_name(chain, metric, q, omega, omega_enabled, palette):
    label = format_solve_score_chain_display(chain, legacy_quantile=q)
    return " ".join(part for part in (label or metric, palette) if part)


def _solve_score_scratch_keys(job_id, chain, quantile, root_transforms, metric="proximity", omega=1.0, omega_enabled=False):
    rt_json = json.dumps(root_transforms or [], separators=(",", ":"))
    rt_hash = hashlib.sha1(rt_json.encode("utf-8")).hexdigest()[:8]
    compiled = compile_solve_score_chain_or_legacy(
        chain,
        metric,
        quantile,
        omega,
        omega_enabled,
        default_metric=metric or "proximity",
    )
    metric_slug = str(compiled["metric"] or "score").replace(" ", "_")
    if compiled["legacy_compatible"]:
        prefix = f"renders/{job_id}/solve_scores/{metric_slug}_rt{rt_hash}/"
    else:
        chain_id = solve_score_chain_id(compiled["chain"], legacy_quantile=compiled["quantile"])
        prefix = f"renders/{job_id}/solve_scores/{metric_slug}_{chain_id}_rt{rt_hash}/"
    return prefix + "clip.json", prefix + "hist/", prefix + "bins.json"


def _load_calc(job_id):
    obj = s3.get_object(Bucket=BUCKET, Key=f"renders/{job_id}/calc.json")
    return json.loads(obj["Body"].read())


def _load_palette_meta(job_id, palette_id):
    key = f"renders/{job_id}/palettes/{palette_id}/meta.json"
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    meta = json.loads(obj["Body"].read())
    if meta.get("job_id") and meta.get("job_id") != job_id:
        raise RuntimeError(f"Palette artifact {palette_id} belongs to {meta.get('job_id')}, not {job_id}")
    return meta


def _s3_key_exists(key):
    if not key:
        return False
    try:
        res = s3.head_object(Bucket=BUCKET, Key=key)
        return isinstance(res, dict)
    except Exception as exc:
        if _missing_s3_key(exc):
            return False
        raise


def _existing_palette_artifact_is_complete(meta):
    if not _s3_key_exists(str(meta.get("image_key") or "")):
        return False
    if not _s3_key_exists(str(meta.get("preview_key") or "")):
        return False
    channels = int(meta.get("raw_channels") or meta.get("score_output_channel_count") or 1)
    raw_key = str(meta.get("raw_key") or "")
    raw_meta_key = str(meta.get("raw_meta_key") or "")
    if channels > 1 or raw_key or raw_meta_key:
        return _s3_key_exists(raw_key) and _s3_key_exists(raw_meta_key)
    if meta.get("render_reusable") is True or str(meta.get("render_reusable", "")).lower() == "true":
        return bool(meta.get("section_bins_prefix") or meta.get("chunk_bins_prefix") or meta.get("palette_bins_key"))
    return bool(meta.get("palette_bins_key") or meta.get("section_bins_prefix") or meta.get("chunk_bins_prefix"))


def _load_existing_palette_for_identity(job_id, palette_id, fingerprint):
    meta_key = f"renders/{job_id}/palettes/{palette_id}/meta.json"
    if not _s3_key_exists(meta_key):
        return None
    try:
        meta = _load_palette_meta(job_id, palette_id)
    except Exception as exc:
        if _missing_s3_key(exc):
            return None
        raise
    actual = str(meta.get("palette_variant_fingerprint") or meta.get("content_fingerprint") or "").strip()
    if actual != str(fingerprint):
        raise RuntimeError(
            f"Palette artifact id collision for {palette_id}: expected fingerprint {fingerprint}, got {actual or '<missing>'}"
        )
    if not _existing_palette_artifact_is_complete(meta):
        return None
    return meta


def _missing_s3_key(exc):
    code = str(getattr(exc, "response", {}).get("Error", {}).get("Code", "")).strip()
    return code in {"404", "NoSuchKey", "NotFound", "NotFoundException"}


def _associated_palette_ref(job_id, metadata):
    palette_id = str((metadata or {}).get("associated_palette_id") or "").strip()
    if not palette_id:
        return None
    return {
        "palette_id": palette_id,
        "image_key": str(
            (metadata or {}).get("associated_palette_image_key")
            or f"renders/{job_id}/palettes/{palette_id}/image.jpeg"
        ),
    }


def _associated_palette_exists(job_id, metadata):
    ref = _associated_palette_ref(job_id, metadata)
    if not ref or not ref["image_key"]:
        return False
    try:
        s3.head_object(Bucket=BUCKET, Key=ref["image_key"])
        return True
    except Exception as exc:
        if _missing_s3_key(exc):
            return False
        raise


def _artifact_meta_quantile(meta, field_name):
    raw = meta.get(field_name, "")
    if raw in ("", None):
        return None
    return float(raw)


def _resolve_color_lineage(job_id, artifact_id):
    selected = _load_color_artifact(job_id, artifact_id)
    current = selected
    seen = {artifact_id}
    while True:
        if _associated_palette_exists(job_id, current):
            return selected, current, "associated"
        if current.get("color_mode") == "saved_palette" and current.get("palette_source_id"):
            return selected, current, "saved_palette"
        if current.get("color_mode") == "solve_score":
            return selected, current, "solve_score"
        parent_id = str(current.get("derived_from_artifact_id") or "").strip()
        if not parent_id or parent_id in seen:
            break
        seen.add(parent_id)
        current = _load_color_artifact(job_id, parent_id)
    raise RuntimeError(
        f"ExtractPalette could not resolve reusable palette lineage from Color artifact {artifact_id}"
    )


def _load_color_artifact(job_id, artifact_id):
    head_info = load_color_artifact_head(s3, BUCKET, job_id, artifact_id)
    meta = dict(head_info["metadata"])
    meta["artifact_id"] = artifact_id
    meta["image_key"] = head_info["image_key"]
    meta["root_transforms"] = parse_root_transforms(meta.get("root_transforms"))
    return meta


def _scratch_matches(job_id, chain, metric, quantile, omega, omega_enabled, root_transforms):
    chain = chain or solve_score_chain_from_scalars(metric, quantile, omega, omega_enabled)
    clip_key, _, bins_key = _solve_score_scratch_keys(job_id, chain, quantile, root_transforms, metric, omega, omega_enabled)
    try:
        clip = json.loads(s3.get_object(Bucket=BUCKET, Key=clip_key)["Body"].read())
        bins = json.loads(s3.get_object(Bucket=BUCKET, Key=bins_key)["Body"].read())
    except Exception:
        return False, clip_key, bins_key
    expected_xforms = root_transforms or []
    for obj in (clip, bins):
        if obj.get("family") != "solve_score":
            return False, clip_key, bins_key
        if obj.get("metric") != metric:
            return False, clip_key, bins_key
        if float(obj.get("clip_quantile", -1)) != float(quantile):
            return False, clip_key, bins_key
        if float(obj.get("omega", 1.0)) != float(omega):
            return False, clip_key, bins_key
        if _validate_omega_enabled(obj.get("omega_enabled", True)) != bool(omega_enabled):
            return False, clip_key, bins_key
        if parse_root_transforms(obj.get("root_transforms")) != expected_xforms:
            return False, clip_key, bins_key
    return True, clip_key, bins_key

def _base_extract_plan(
    job_id,
    run_id,
    task_id,
    selected,
    palette_id,
    display_name,
    image_key,
    preview_key,
    metric,
    palette,
    q,
    omega,
    omega_enabled,
    score_chain,
    color_interpretation="scalar_lut",
    output_channel_count=1,
    output_channels=None,
    raw_key="",
    raw_meta_key="",
    raw_layout="",
    raw_channels=1,
    palette_variant_fingerprint="",
):
    mode = normalize_color_interpretation(color_interpretation)
    return {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "extract_palette",
        "params": {},
        "palette_id": palette_id,
        "extract": {},
        "attach": {
            "enabled": True,
            "artifact_id": selected["artifact_id"],
            "mode": "",
            "palette_id": palette_id,
            "display_name": display_name,
            "image_key": image_key,
            "preview_key": preview_key,
            "palette": palette,
            "metric": metric,
            "quantile": q,
            "omega": omega,
            "omega_enabled": omega_enabled,
            "score_chain": public_solve_score_chain(score_chain) if score_chain not in ("", None, []) else score_chain,
            "color_interpretation": mode,
            "raw_key": raw_key,
            "raw_meta_key": raw_meta_key,
            "meta_key": f"renders/{job_id}/palettes/{palette_id}/meta.json" if palette_id else "",
        },
        "solve_score": {
            "chain_fingerprint": "",
            "spec_version": SOLVE_SCORE_SPEC_VERSION,
            "output_channel_count": int(output_channel_count or 1),
            "output_channels": list(output_channels or []),
        },
        "outputs": {
            "image_key": image_key,
            "preview_key": preview_key,
            "meta_key": f"renders/{job_id}/palettes/{palette_id}/meta.json" if palette_id else "",
            "raw_key": raw_key,
            "raw_meta_key": raw_meta_key,
            "raw_layout": raw_layout,
            "raw_channels": int(raw_channels or output_channel_count or 1),
            "palette_variant_fingerprint": palette_variant_fingerprint,
            "palette_variant_spec_version": PALETTE_VARIANT_SPEC_VERSION,
            "chunks_prefix": "",
            "section_scores_prefix": "",
            "section_bins_prefix": "",
            "section_meta_prefix": "",
            "chunk_scores_prefix": "",
            "chunk_bins_prefix": "",
            "chunk_meta_prefix": "",
        },
    }


def _execution_params(raw_params):
    pp = dict(raw_params or {})
    return {
        "solve_score_threads": _validate_threads(pp.get("solve_score_threads", 1), default=1),
        "solve_score_hist_input_mode": _validate_hist_input_mode(pp.get("solve_score_hist_input_mode", "tmpfile")),
        "solve_score_hist_retries": _validate_sectioned_retries(pp.get("solve_score_hist_retries", 2)),
        "solve_score_merge_workers": _validate_merge_workers(pp.get("solve_score_merge_workers", 16), default=16),
        "palette_chunk_threads": _validate_palette_chunk_threads(pp.get("palette_chunk_threads", 4), default=4),
        "palette_chunk_input_mode": _validate_palette_chunk_input_mode(pp.get("palette_chunk_input_mode", "sectioned")),
        "palette_chunk_retries": _validate_sectioned_retries(pp.get("palette_chunk_retries", 2)),
        "palette_chunk_workers": _validate_palette_chunk_workers(pp.get("palette_chunk_workers", 16), default=16),
    }


def _done_plan_for_existing_palette(
    *,
    job_id,
    run_id,
    task_id,
    palette_id,
    meta,
    metric,
    palette,
    palette_display_name,
    quantile,
    omega,
    omega_enabled,
    score_chain,
    color_interpretation,
    output_channel_count,
    output_channels,
    palette_variant_fingerprint,
):
    plan = _base_extract_plan(
        job_id,
        run_id,
        task_id,
        {"artifact_id": ""},
        palette_id=palette_id,
        display_name=str(meta.get("display_name") or ""),
        image_key=str(meta.get("image_key") or f"renders/{job_id}/palettes/{palette_id}/image.jpeg"),
        preview_key=str(meta.get("preview_key") or f"renders/{job_id}/palettes/{palette_id}/preview.png"),
        metric=metric,
        palette=palette,
        q=quantile,
        omega=omega,
        omega_enabled=omega_enabled,
        score_chain=score_chain,
        color_interpretation=color_interpretation,
        output_channel_count=output_channel_count,
        output_channels=output_channels,
        raw_key=str(meta.get("raw_key") or ""),
        raw_meta_key=str(meta.get("raw_meta_key") or ""),
        raw_layout=str(meta.get("raw_layout") or ""),
        raw_channels=int(meta.get("raw_channels") or output_channel_count or 1),
        palette_variant_fingerprint=palette_variant_fingerprint,
    )
    plan["mode"] = "palette"
    plan["params"] = {
        "metric": metric,
        "palette": palette,
        "palette_display_name": palette_display_name,
        "solve_score_chain": score_chain,
        "solve_score_quantile": quantile,
        "solve_score_omega": omega,
        "solve_score_omega_enabled": omega_enabled,
        "root_transforms": meta.get("root_transforms") or [],
        "color_interpretation": normalize_color_interpretation(color_interpretation),
    }
    plan["extract"] = {
        "action": "done",
        "reason": "palette_artifact_already_exists",
        "source_artifact_id": "",
        "target_artifact_id": "",
    }
    plan["attach"]["enabled"] = False
    plan["solve_score"].update({
        "metric": metric,
        "quantile": quantile,
        "omega": omega,
        "omega_enabled": omega_enabled,
        "chain": score_chain,
        "chain_fingerprint": str(meta.get("chain_fingerprint") or meta.get("solve_score_chain_fingerprint") or ""),
        "output_channel_count": int(output_channel_count or 1),
        "output_channels": list(output_channels or []),
    })
    return plan


def _fallback_lores_coeffs_key(job_id, calc):
    return calc_fallback_lores_coeffs_key(job_id, calc)


def _fallback_lores_params_key(job_id, calc):
    return calc_fallback_lores_params_key(job_id, calc)


def _fallback_params_key(job_id, calc):
    return fallback_params_global_key(job_id, calc)


def _build_extract_plan(job_id, run_id, task_id, artifact_id, raw_params=None):
    execution = _execution_params(raw_params)
    selected, source, source_kind = _resolve_color_lineage(job_id, artifact_id)

    if _associated_palette_exists(job_id, selected):
        assoc_selected = read_solve_score_metadata("associated_palette", selected, default_metric="proximity")
        palette_id = str(selected.get("associated_palette_id"))
        image_key = str(selected.get("associated_palette_image_key") or f"renders/{job_id}/palettes/{palette_id}/image.jpeg")
        preview_key = str(selected.get("associated_palette_preview_key") or f"renders/{job_id}/palettes/{palette_id}/preview.png")
        plan = _base_extract_plan(
            job_id, run_id, task_id, selected,
            palette_id=palette_id,
            display_name=str(selected.get("associated_palette_display_name") or ""),
            image_key=image_key,
            preview_key=preview_key,
            metric=assoc_selected["metric"],
            palette=str(selected.get("associated_palette_palette") or ""),
            q=assoc_selected["quantile"],
            omega=assoc_selected["omega"],
            omega_enabled=assoc_selected["omega_enabled"],
            score_chain=assoc_selected["chain_public"],
            color_interpretation=selected.get("associated_palette_color_interpretation", "scalar_lut"),
            raw_key=selected.get("associated_palette_raw_key", ""),
            raw_meta_key=selected.get("associated_palette_raw_meta_key", ""),
            raw_channels=int(selected.get("associated_palette_raw_channels") or selected.get("raw_channels") or 1),
        )
        plan["params"].update(execution)
        plan["extract"] = {"action": "done", "reason": "already_associated", "source_artifact_id": selected["artifact_id"]}
        plan["attach"]["enabled"] = False
        return plan

    if source_kind == "associated":
        assoc_source = read_solve_score_metadata("associated_palette", source, default_metric="proximity")
        palette_id = str(source.get("associated_palette_id"))
        image_key = str(source.get("associated_palette_image_key") or f"renders/{job_id}/palettes/{palette_id}/image.jpeg")
        preview_key = str(source.get("associated_palette_preview_key") or f"renders/{job_id}/palettes/{palette_id}/preview.png")
        plan = _base_extract_plan(
            job_id, run_id, task_id, selected,
            palette_id=palette_id,
            display_name=str(source.get("associated_palette_display_name") or ""),
            image_key=image_key,
            preview_key=preview_key,
            metric=assoc_source["metric"],
            palette=str(source.get("associated_palette_palette") or ""),
            q=assoc_source["quantile"],
            omega=assoc_source["omega"],
            omega_enabled=assoc_source["omega_enabled"],
            score_chain=assoc_source["chain_public"],
            color_interpretation=source.get("associated_palette_color_interpretation", "scalar_lut"),
            raw_key=source.get("associated_palette_raw_key", ""),
            raw_meta_key=source.get("associated_palette_raw_meta_key", ""),
            raw_channels=int(source.get("associated_palette_raw_channels") or source.get("raw_channels") or 1),
        )
        plan["params"].update(execution)
        plan["extract"] = {"action": "attach", "reason": "inherit_existing_association", "source_artifact_id": source["artifact_id"]}
        plan["attach"]["mode"] = str(source.get("associated_palette_mode") or "generated")
        return plan

    if source_kind == "saved_palette":
        palette_id = str(source.get("palette_source_id") or "").strip()
        if not palette_id:
            raise RuntimeError(f"Saved-palette Color artifact {source['artifact_id']} is missing palette_source_id")
        palette_meta = _load_palette_meta(job_id, palette_id)
        source_score = read_solve_score_metadata(
            "palette_source",
            source,
            default_metric=palette_meta.get("metric") or "proximity",
        )
        plan = _base_extract_plan(
            job_id, run_id, task_id, selected,
            palette_id=palette_id,
            display_name=str(source.get("palette_source_display_name") or palette_meta.get("display_name") or ""),
            image_key=str(palette_meta.get("image_key") or f"renders/{job_id}/palettes/{palette_id}/image.jpeg"),
            preview_key=str(palette_meta.get("preview_key") or f"renders/{job_id}/palettes/{palette_id}/preview.png"),
            metric=source_score["metric"],
            palette=str(source.get("palette_source_palette") or palette_meta.get("palette") or ""),
            q=source_score["quantile"],
            omega=source_score["omega"],
            omega_enabled=source_score["omega_enabled"],
            score_chain=source_score["chain_public"],
            color_interpretation=palette_meta.get("color_interpretation", "scalar_lut"),
            output_channel_count=int(palette_meta.get("score_output_channel_count") or palette_meta.get("raw_channels") or 1),
            output_channels=palette_meta.get("score_output_channels") or [],
            raw_key=str(palette_meta.get("raw_key") or ""),
            raw_meta_key=str(palette_meta.get("raw_meta_key") or ""),
            raw_layout=str(palette_meta.get("raw_layout") or ""),
            raw_channels=int(palette_meta.get("raw_channels") or palette_meta.get("score_output_channel_count") or 1),
        )
        plan["params"].update(execution)
        plan["extract"] = {"action": "attach", "reason": "saved_palette_dependency", "source_artifact_id": source["artifact_id"]}
        plan["attach"]["mode"] = "dependency"
        return plan

    source_score = read_solve_score_metadata("solve", source, default_metric="proximity")
    if source_score.get("has_explicit_outputs") or int(source.get("score_output_channel_count") or source.get("raw_channels") or 1) != 1:
        raise RuntimeError("ExtractPalette requires a scalar Color artifact; multi-output/direct-RGB artifacts are not palette-extractable in v1")
    metric = source_score["metric"]
    prelude_by_source = solve_score_lag_prelude_by_source(source_score)
    uses_lag = bool(source_score.get("uses_lag"))
    if uses_lag:
        execution["solve_score_hist_input_mode"] = "sectioned"
        execution["palette_chunk_input_mode"] = "sectioned"
    palette = str(source.get("palette") or "").strip()
    if not is_valid_palette_name(palette):
        raise RuntimeError(f"Solve-score Color artifact {source['artifact_id']} has invalid palette {palette!r}")
    palette_display_name = normalize_palette_display_name(
        source.get("palette_display_name"),
        palette,
    )
    q = _artifact_meta_quantile(source, "solve_score_quantile")
    if q is None:
        raise RuntimeError(f"Solve-score Color artifact {source['artifact_id']} is missing solve_score_quantile")
    omega = source_score["omega"]
    omega_enabled = source_score["omega_enabled"]
    root_params = {
        "root_transforms": list(source.get("root_transforms") or []),
        "root_program_source_text": source.get("root_program_source_text", ""),
        "root_program": source.get("root_program"),
    }
    root_program_payload = _apply_root_program_to_params(root_params)
    root_transforms = root_params["root_transforms"]

    calc = _load_calc(job_id)
    degree = calc.get("degree")
    full_n = calc.get("N", calc.get("n1"))
    times = int(calc.get("times", 1) or 1)
    lores_bin_key = (calc.get("lores") or {}).get("bin_key", "")
    if not degree or not full_n or not lores_bin_key:
        raise RuntimeError(f"calc.json missing degree, N, or lores.bin_key for {job_id}")
    chunk_items = build_chunk_items(calc, job_id, require_chunks=True)
    section_items = build_physical_section_items(chunk_items)
    pass0_steps = int(full_n) * int(full_n)
    if sum(int(item["step_count"]) for item in chunk_items) < pass0_steps:
        raise RuntimeError("calc.json chunk metadata is too small for pass-0 palette extraction")

    source_score_chain_internal = source_score["chain"]
    source_score_chain_public = source_score["chain_public"]
    source_score_source_text = str(source.get("solve_score_program_source_text") or "").strip()
    if not source_score_source_text:
        source_score_source_text = solve_score_source_text_from_chain(source_score_chain_public)
    scratch_ok, clip_key, bins_key = _scratch_matches(job_id, source_score_chain_internal, metric, q, omega, omega_enabled, root_transforms)
    output_channel_count = 1
    output_channels = []
    color_interpretation = "scalar_lut"
    chain_fingerprint = source_score["chain_fingerprint"]
    palette_id, palette_variant_fingerprint, _identity_payload = _palette_variant_identity(
        job_id=job_id,
        compiled_score_fingerprint=chain_fingerprint,
        score_chain_public=source_score_chain_public,
        metric=metric,
        quantile=q,
        omega=omega,
        omega_enabled=omega_enabled,
        color_interpretation=color_interpretation,
        output_channel_count=output_channel_count,
        output_channels=output_channels,
        palette=palette,
        root_transforms=root_transforms,
        palette_display_name=palette_display_name,
    )
    prefix = f"renders/{job_id}/palettes/{palette_id}/"
    solve_prefix = prefix + "solve_score/"
    chunks_prefix = prefix + "chunks/"
    display_name = source.get("associated_palette_display_name") or _palette_display_name(
        source_score_chain_internal,
        metric,
        q,
        omega,
        omega_enabled,
        palette_display_name or palette,
    )

    plan = _base_extract_plan(
        job_id, run_id, task_id, selected,
        palette_id=palette_id,
        display_name=display_name,
        image_key=prefix + "image.jpeg",
        preview_key=prefix + "preview.png",
        metric=metric,
        palette=palette,
        q=q,
        omega=omega,
        omega_enabled=omega_enabled,
        score_chain=source_score_chain_public,
        color_interpretation=color_interpretation,
        output_channel_count=output_channel_count,
        output_channels=output_channels,
        palette_variant_fingerprint=palette_variant_fingerprint,
    )
    plan["mode"] = "extract_palette"
    plan["params"] = {
        "metric": metric,
        "palette": palette,
        "palette_display_name": palette_display_name,
        "solve_score_chain": source_score_chain_public,
        "solve_score_program_source_text": source_score_source_text,
        "solve_score_quantile": q,
        "solve_score_omega": omega,
        "solve_score_omega_enabled": omega_enabled,
        "root_transforms": root_transforms,
        "root_program_source_text": root_program_payload.get("source_text", ""),
        "root_program": root_program_payload,
        "root_program_fingerprint": root_program_payload.get("fingerprint", ""),
        "root_spec_version": root_program_payload.get("spec_version", 2),
        "color_interpretation": color_interpretation,
        **execution,
    }
    plan["extract"] = {
        "action": "generate_reuse" if scratch_ok else "generate_full",
        "reason": "reuse_solve_score_scratch" if scratch_ok else "rerun_solve_score_prepass",
        "source_artifact_id": source["artifact_id"],
        "target_artifact_id": selected["artifact_id"],
    }
    plan["attach"]["mode"] = "generated"
    if solve_score_uses_non_solve_sources(source_score):
        if solve_score_uses_source(source_score, "cf") and not _fallback_lores_coeffs_key(job_id, calc):
            raise RuntimeError("Mixed-source solve score requires lores.coeffs_key")
        if solve_score_uses_source(source_score, "pm"):
            if not _fallback_lores_params_key(job_id, calc):
                raise RuntimeError("Param-source solve score requires lores.params_key")
            if not chunk_items_have_params(chunk_items):
                raise RuntimeError("Param-source solve score requires full-res params metadata on every chunk")
    n_coeffs = int(calc.get("n_coeffs", degree + 1) or (degree + 1))
    solve_source_manifest = build_solve_source_manifest(
        chunk_items,
        job_id=job_id,
        degree=degree,
        n_coeffs=n_coeffs,
    )
    solve_source_manifest_ref = (
        write_solve_source_manifest(
            s3,
            BUCKET,
            solve_source_manifest,
            job_id=job_id,
            run_id=run_id,
            suffix="palette_reuse_solve_source_manifest",
        )
        if uses_lag
        else {"key": "", "bytes": 0}
    )
    plan["logical_section"] = uses_lag
    plan["solve_source_manifest"] = {}
    plan["solve_source_manifest_key"] = solve_source_manifest_ref["key"]
    plan["solve_source_manifest_bytes"] = solve_source_manifest_ref["bytes"]
    plan["calc"] = {
        "degree": degree,
        "N": full_n,
        "times": times,
        "n_chunks": len(chunk_items),
        "n_sections": len(section_items),
        "pass0_steps": pass0_steps,
        "lores_bin_key": lores_bin_key,
        "lores_coeffs_key": _fallback_lores_coeffs_key(job_id, calc),
        "lores_params_key": _fallback_lores_params_key(job_id, calc),
        "params_key": _fallback_params_key(job_id, calc),
        "param_storage_mode": str(calc.get("param_storage_mode") or ("chunked" if not _fallback_params_key(job_id, calc) else "global")),
        "n_coeffs": n_coeffs,
    }
    plan["section_items"] = section_items
    plan["solve_score"] = {
        "metric": metric,
        "quantile": q,
        "omega": omega,
        "omega_enabled": omega_enabled,
        "chain": source_score_chain_public,
        "source_text": source_score_source_text,
        "metrics": source_score["metrics"],
        "program": source_score["program_spec"],
        "chain_fingerprint": chain_fingerprint,
        "spec_version": source_score.get("spec_version", SOLVE_SCORE_SPEC_VERSION),
        "output_channel_count": output_channel_count,
        "output_channels": output_channels,
        "has_explicit_outputs": False,
        "raw_output_path": False,
        "uses_lag": uses_lag,
        "prelude_by_source": prelude_by_source,
        "prelude_rows": int(prelude_by_source.get("slv", 0)),
        "score_coeff_prelude_rows": int(prelude_by_source.get("cf", 0)),
        "score_param_prelude_rows": int(prelude_by_source.get("pm", 0)),
        "cleanup_scratch": not scratch_ok,
        "clip_key": clip_key if scratch_ok else prefix + f"solve_score/{metric}_clip.json",
        "hist_prefix": solve_prefix,
        "bins_key": bins_key if scratch_ok else prefix + f"solve_score/{metric}_bins.json",
    }
    plan["outputs"] = {
        "image_key": prefix + "image.jpeg",
        "preview_key": prefix + "preview.png",
        "meta_key": prefix + "meta.json",
        "raw_key": "",
        "raw_meta_key": "",
        "raw_layout": "",
        "raw_channels": output_channel_count,
        "palette_variant_fingerprint": palette_variant_fingerprint,
        "palette_variant_spec_version": PALETTE_VARIANT_SPEC_VERSION,
        "chunks_prefix": chunks_prefix,
        "section_scores_prefix": chunks_prefix + "score_section_",
        "section_bins_prefix": chunks_prefix + "palette_bins_section_",
        "section_meta_prefix": chunks_prefix + "meta_section_",
        "chunk_scores_prefix": chunks_prefix + "score_section_",
        "chunk_bins_prefix": chunks_prefix + "palette_bins_section_",
        "chunk_meta_prefix": chunks_prefix + "meta_section_",
    }
    return plan


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    run_id = params["run_id"]
    task_id = params["task_id"]
    artifact_id = str(params.get("artifact_id") or "").strip()
    if artifact_id:
        plan = _build_extract_plan(job_id, run_id, task_id, artifact_id, params.get("params", {}))
        plan_json = json.dumps(plan)
        if len(plan_json) > MAX_PLAN_BYTES:
            raise RuntimeError(
                f"Plan too large: {len(plan_json)} bytes > {MAX_PLAN_BYTES} limit. "
                f"Reduce chunk count."
            )
        return ok_response(plan)
    pp = dict(params.get("params", {}))

    execution = _execution_params(pp)
    color_interpretation = _color_interpretation_from_params(pp)
    compiled_score = solve_score_program_for_run(pp)
    output_channel_count = int(compiled_score.get("output_channel_count") or 1)
    color_contract = validate_color_output_contract(
        interpretation=color_interpretation,
        output_channel_count=output_channel_count,
        output_channels=compiled_score.get("output_channels") or [],
    )
    color_interpretation = color_contract["interpretation"]
    output_channels = color_contract["channels"]
    chain_fingerprint = compiled_solve_score_fingerprint(compiled_score)
    has_explicit_outputs = bool(compiled_score.get("has_explicit_outputs"))
    raw_output_path = has_explicit_outputs or output_channel_count != 1
    prelude_by_source = solve_score_lag_prelude_by_source(compiled_score)
    uses_lag = bool(compiled_score.get("uses_lag"))
    if uses_lag:
        # Lag can cross physical chunk boundaries, so use manifest-backed reads.
        execution["solve_score_hist_input_mode"] = "sectioned"
        execution["palette_chunk_input_mode"] = "sectioned"
    if raw_output_path:
        execution["palette_chunk_input_mode"] = "sectioned"
    metric = compiled_score["metric"]
    requested_palette = pp.get("palette", "inferno")
    palette = requested_palette if _interpretation_uses_palette(color_interpretation) else ""
    palette_display_name = normalize_palette_display_name(
        pp.get("palette_display_name"),
        palette,
    )
    root_params = {
        "root_transforms": pp.get("root_transforms", []),
        "root_program_source_text": pp.get("root_program_source_text", ""),
        "root_program": pp.get("root_program"),
    }
    root_program_payload = _apply_root_program_to_params(root_params)
    root_transforms = root_params["root_transforms"]
    if metric not in VALID_METRICS:
        raise RuntimeError(f"Invalid metric: {metric}")
    if _interpretation_uses_palette(color_interpretation) and not is_valid_palette_name(palette):
        raise RuntimeError(f"Invalid palette: {palette}")
    q = compiled_score["quantile"]
    omega = compiled_score["omega"]
    omega_enabled = compiled_score["omega_enabled"]
    compiled_score_chain_public = public_solve_score_chain(compiled_score["chain"])
    compiled_score_source_text = str(compiled_score.get("source_text") or "")

    palette_id, palette_variant_fingerprint, _identity_payload = _palette_variant_identity(
        job_id=job_id,
        compiled_score_fingerprint=chain_fingerprint,
        score_chain_public=compiled_score_chain_public,
        metric=metric,
        quantile=q,
        omega=omega,
        omega_enabled=omega_enabled,
        color_interpretation=color_interpretation,
        output_channel_count=output_channel_count,
        output_channels=output_channels,
        palette=palette,
        root_transforms=root_transforms,
        palette_display_name=palette_display_name,
    )
    existing = _load_existing_palette_for_identity(job_id, palette_id, palette_variant_fingerprint)
    if existing:
        return ok_response(
            _done_plan_for_existing_palette(
                job_id=job_id,
                run_id=run_id,
                task_id=task_id,
                palette_id=palette_id,
                meta=existing,
                metric=metric,
                palette=palette,
                palette_display_name=palette_display_name,
                quantile=q,
                omega=omega,
                omega_enabled=omega_enabled,
                score_chain=compiled_score_chain_public,
                color_interpretation=color_interpretation,
                output_channel_count=output_channel_count,
                output_channels=output_channels,
                palette_variant_fingerprint=palette_variant_fingerprint,
            )
        )

    calc = _load_calc(job_id)
    degree = calc.get("degree")
    full_n = calc.get("N", calc.get("n1"))
    times = int(calc.get("times", 1) or 1)
    lores_bin_key = (calc.get("lores") or {}).get("bin_key", "")
    if not degree or not full_n or not lores_bin_key:
        raise RuntimeError(f"calc.json missing degree, N, or lores.bin_key for {job_id}")
    if solve_score_uses_non_solve_sources(compiled_score):
        if solve_score_uses_source(compiled_score, "cf"):
            lores_coeffs_key = _fallback_lores_coeffs_key(job_id, calc)
            if not lores_coeffs_key:
                raise RuntimeError("Mixed-source solve score requires lores.coeffs_key")
            try:
                n_coeffs = int(calc.get("n_coeffs"))
            except (TypeError, ValueError):
                raise RuntimeError(f"Mixed-source solve score requires numeric n_coeffs, got {calc.get('n_coeffs')!r}")
            if n_coeffs < 1:
                raise RuntimeError(f"Mixed-source solve score requires n_coeffs >= 1, got {n_coeffs}")
        if solve_score_uses_source(compiled_score, "pm"):
            lores_params_key = _fallback_lores_params_key(job_id, calc)
            if not lores_params_key:
                raise RuntimeError("Param-source solve score requires lores.params_key")

    pass0_steps = full_n * full_n
    chunk_items = build_chunk_items(calc, job_id, require_chunks=True)
    section_items = build_physical_section_items(chunk_items)
    step_start = sum(int(item["step_count"]) for item in chunk_items)

    if step_start < pass0_steps:
        raise RuntimeError(f"Full solve metadata too small: only {step_start} solves, expected at least {pass0_steps}")
    if solve_score_uses_source(compiled_score, "pm") and not chunk_items_have_params(chunk_items):
        raise RuntimeError("Param-source solve score requires full-res params metadata on every chunk")

    prefix = f"renders/{job_id}/palettes/{palette_id}/"
    clip_key, solve_prefix, bins_key = _solve_score_scratch_keys(
        job_id,
        compiled_score["chain"],
        q,
        root_transforms,
        metric,
        omega,
        omega_enabled,
    )
    chunks_prefix = prefix + "chunks/"
    section_scores_prefix = chunks_prefix + "score_section_"
    section_bins_prefix = chunks_prefix + "palette_bins_section_"
    section_meta_prefix = chunks_prefix + "meta_section_"
    n_coeffs = int(calc.get("n_coeffs", degree + 1) or (degree + 1))
    solve_source_manifest = build_solve_source_manifest(
        chunk_items,
        job_id=job_id,
        degree=degree,
        n_coeffs=n_coeffs,
    )
    solve_source_manifest_ref = (
        write_solve_source_manifest(
            s3,
            BUCKET,
            solve_source_manifest,
            job_id=job_id,
            run_id=run_id,
            suffix="palette_solve_source_manifest",
        )
        if uses_lag
        else {"key": "", "bytes": 0}
    )

    plan = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "palette",
        "params": {
            "metric": metric,
            "palette": palette,
            "palette_display_name": palette_display_name,
            "requested_palette": requested_palette,
            "color_interpretation": color_interpretation,
            "score_output_interpretation": color_interpretation,
            "solve_score_chain": compiled_score_chain_public,
            "solve_score_program_source_text": compiled_score_source_text,
            "solve_score_quantile": q,
            "solve_score_omega": omega,
            "solve_score_omega_enabled": omega_enabled,
            "root_transforms": root_transforms,
            "root_program_source_text": root_program_payload.get("source_text", ""),
            "root_program": root_program_payload,
            "root_program_fingerprint": root_program_payload.get("fingerprint", ""),
            "root_spec_version": root_program_payload.get("spec_version", 2),
            **execution,
        },
        "palette_id": palette_id,
        "extract": {
            "action": "generate_full",
            "reason": "normal_palette_generation",
            "source_artifact_id": "",
            "target_artifact_id": "",
        },
        "attach": {
            "enabled": False,
            "artifact_id": "",
            "mode": "",
            "palette_id": "",
            "display_name": "",
            "image_key": "",
            "preview_key": "",
            "palette": "",
            "metric": "",
            "quantile": "",
            "omega": "",
            "omega_enabled": True,
            "score_chain": "",
            "color_interpretation": color_interpretation,
            "raw_key": "",
            "raw_meta_key": "",
            "meta_key": "",
        },
        "prefix": prefix,
        "logical_section": uses_lag,
        "solve_source_manifest": {},
        "solve_source_manifest_key": solve_source_manifest_ref["key"],
        "solve_source_manifest_bytes": solve_source_manifest_ref["bytes"],
        "calc": {
            "degree": degree,
            "N": full_n,
            "times": times,
            "n_chunks": len(chunk_items),
            "n_sections": len(section_items),
            "pass0_steps": pass0_steps,
            "lores_bin_key": lores_bin_key,
            "lores_coeffs_key": _fallback_lores_coeffs_key(job_id, calc),
            "lores_params_key": _fallback_lores_params_key(job_id, calc),
            "params_key": _fallback_params_key(job_id, calc),
            "param_storage_mode": str(calc.get("param_storage_mode") or ("chunked" if not _fallback_params_key(job_id, calc) else "global")),
            "n_coeffs": n_coeffs,
        },
        "section_items": section_items,
        "solve_score": {
            "metric": metric,
            "quantile": q,
            "omega": omega,
            "omega_enabled": omega_enabled,
            "chain": compiled_score_chain_public,
            "source_text": compiled_score_source_text,
            "metrics": compiled_score["metrics"],
            "program": compiled_score["program_spec"],
            "chain_fingerprint": chain_fingerprint,
            "spec_version": SOLVE_SCORE_SPEC_VERSION,
            "output_channel_count": output_channel_count,
            "output_channels": output_channels,
            "has_explicit_outputs": has_explicit_outputs,
            "raw_output_path": raw_output_path,
            "uses_lag": uses_lag,
            "prelude_by_source": prelude_by_source,
            "prelude_rows": int(prelude_by_source.get("slv", 0)),
            "score_coeff_prelude_rows": int(prelude_by_source.get("cf", 0)),
            "score_param_prelude_rows": int(prelude_by_source.get("pm", 0)),
            "cleanup_scratch": True,
            "clip_key": clip_key,
            "hist_prefix": solve_prefix,
            "bins_key": bins_key,
        },
        "outputs": {
            "image_key": prefix + "image.jpeg",
            "preview_key": prefix + "preview.png",
            "meta_key": prefix + "meta.json",
            "raw_key": prefix + "greyscale.raw" if raw_output_path else "",
            "raw_meta_key": prefix + "greyscale.meta.json" if raw_output_path else "",
            "raw_layout": "u8_packed_channels_row_major" if output_channel_count > 1 else ("u8_scalar_row_major" if raw_output_path else ""),
            "raw_channels": output_channel_count,
            "palette_variant_fingerprint": palette_variant_fingerprint,
            "palette_variant_spec_version": PALETTE_VARIANT_SPEC_VERSION,
            "chunks_prefix": chunks_prefix,
            "section_scores_prefix": section_scores_prefix,
            "section_bins_prefix": section_bins_prefix,
            "section_meta_prefix": section_meta_prefix,
            "chunk_scores_prefix": section_scores_prefix,
            "chunk_bins_prefix": section_bins_prefix,
            "chunk_meta_prefix": section_meta_prefix,
        },
    }

    plan_json = json.dumps(plan)
    if len(plan_json) > MAX_PLAN_BYTES:
        raise RuntimeError(
            f"Plan too large: {len(plan_json)} bytes > {MAX_PLAN_BYTES} limit. "
            f"Reduce chunk count."
        )
    return ok_response(plan)
