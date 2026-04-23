"""
Render plan Lambda — computes the workflow plan for Step Functions.

Loads calc metadata, computes viewport, normalizes params, produces
compact chunk/tile arrays and output keys. Does NOT dispatch workers
or poll for completion.

Called once per render execution as the BuildPlan step.
"""
import json
import math
import os
import hashlib
from datetime import datetime, timezone

import boto3

from calc_chunks import (
    build_chunk_items,
    fallback_lores_coeffs_key as calc_fallback_lores_coeffs_key,
    fallback_lores_params_key as calc_fallback_lores_params_key,
)
from logical_sections import (
    build_logical_section_items,
    build_solve_source_manifest,
    compute_safe_sectioning,
    normalize_section_mode,
    summarize_chunk_items,
    validate_section_count,
)
from palette_names import VALID_PALETTE_NAMES
from shared import BUCKET, BILEVEL_SPARSE_PIPELINE, REF_SIZE, parse_body, ok_response
from solve_score_chain import (
    compile_solve_score_chain,
    emit_solve_score_metadata,
    format_solve_score_chain_display,
    serialize_solve_score_chain,
    solve_score_uses_source,
)

s3 = boto3.client("s3")
lambda_client = boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1"))

VIEWPORT_FUNCTION = os.environ.get("VIEWPORT_FUNCTION", "polypaint-viewport")
STORAGE_FUNCTION = os.environ.get("STORAGE_FUNCTION", "polypaint-storage")
RASTER_MT_FUNCTION = os.environ.get("RASTER_MT_FUNCTION", "polypaint-raster-mt")

MAX_PLAN_BYTES = 200 * 1024  # 200 KB — fail fast before hitting 256 KB SFN limit
DEFAULT_BACKGROUND_COLOR = "000000"
DEFAULT_BACKGROUND_THRESHOLD = 4


def _plan_params_digest(*, viewport, pix, tile_size, root_transforms):
    payload = {
        "viewport": viewport,
        "grid": {
            "pix": int(pix),
            "tile_size": int(tile_size),
        },
        "params": {
            "root_transforms": root_transforms or [],
        },
        "raster_binary_sha256": str(os.environ.get("RASTER_BINARY_SHA256") or ""),
    }
    encoded = json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return "sha256:" + hashlib.sha256(encoded).hexdigest()

def _fallback_lores_coeffs_key(job_id, calc):
    return calc_fallback_lores_coeffs_key(job_id, calc)


def _fallback_lores_params_key(job_id, calc):
    return calc_fallback_lores_params_key(job_id, calc)


def _compact_section_ranges(section_items):
    return [
        {
            "section_idx": int(item["section_idx"]),
            "section_count": int(item["section_count"]),
            "step_start": int(item.get("step_start") or 0),
            "step_count": int(item.get("step_count") or 0),
        }
        for item in (section_items or [])
    ]


def _validate_boolish(value, field_name, default=False):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    text = str(value).strip().lower()
    if text in ("1", "true", "yes", "on"):
        return True
    if text in ("0", "false", "no", "off"):
        return False
    raise RuntimeError(f"{field_name} must be boolean-like, got {value!r}")


def _associated_palette_display_name(chain, metric, quantile, palette):
    label = format_solve_score_chain_display(chain, legacy_quantile=quantile)
    parts = [label] if label else [str(metric or "").strip()]
    if palette:
        parts.append(str(palette))
    return " ".join(part for part in parts if part)


def _solve_score_scratch_key(job_id, compiled, root_transforms):
    rt_json = json.dumps(root_transforms or [], separators=(",", ":"))
    rt_hash = hashlib.sha1(rt_json.encode("utf-8")).hexdigest()[:8]
    if compiled:
        metric_slug = str(compiled["metric"] or "score").replace(" ", "_")
        if compiled["legacy_compatible"]:
            prefix = f"renders/{job_id}/solve_scores/{metric_slug}_rt{rt_hash}/"
        else:
            encoded_chain = serialize_solve_score_chain(compiled["chain"])
            chain_id = hashlib.sha1(encoded_chain.encode("utf-8")).hexdigest()[:12]
            prefix = f"renders/{job_id}/solve_scores/{metric_slug}_{chain_id}_rt{rt_hash}/"
    else:
        prefix = f"renders/{job_id}/solve_scores/inactive_rt{rt_hash}/"
    return prefix + "clip.json"


def _coerce_positive_int(value, field_name, *, default=None):
    if value in (None, ""):
        if default is None:
            raise RuntimeError(f"{field_name} is required")
        return int(default)
    try:
        coerced = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if coerced < 1:
        raise RuntimeError(f"{field_name} must be >= 1, got {coerced}")
    return coerced


def _coerce_degree(calc):
    return _coerce_positive_int(calc.get("degree"), "calc.degree", default=1)


def _coerce_n_coeffs(calc, degree):
    return _coerce_positive_int(calc.get("n_coeffs"), "calc.n_coeffs", default=int(degree) + 1)


def _validate_raster_engine(value):
    engine = str(value or "single").strip().lower()
    if engine not in ("single", "mt"):
        raise RuntimeError(f"raster_engine must be 'single' or 'mt', got {value!r}")
    return engine


def _validate_thread_count(value, field_name):
    if value in (None, ""):
        return 4
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if not (1 <= threads <= 16):
        raise RuntimeError(f"{field_name} must be in [1, 16], got {threads}")
    return threads


def _validate_worker_count(value, field_name):
    if value in (None, ""):
        return 16
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if not (1 <= workers <= 64):
        raise RuntimeError(f"{field_name} must be in [1, 64], got {workers}")
    return workers


def _validate_retry_count(value, field_name):
    if value in (None, ""):
        return 2
    try:
        retries = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be an integer, got {value!r}")
    if not (0 <= retries <= 10):
        raise RuntimeError(f"{field_name} must be in [0, 10], got {retries}")
    return retries


def _validate_raster_input_mode(value):
    mode = str(value or "tmpfile").strip().lower()
    if mode not in ("tmpfile", "sectioned"):
        raise RuntimeError(f"raster_input_mode must be 'tmpfile' or 'sectioned', got {value!r}")
    return mode


def _coerce_finite_float(value, field_name):
    try:
        num = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{field_name} must be numeric, got {value!r}")
    if not math.isfinite(num):
        raise RuntimeError(f"{field_name} must be finite, got {value!r}")
    return num


def _explicit_viewport_from_params(rp):
    min_re = _coerce_finite_float(rp.get("min_re"), "min_re")
    max_re = _coerce_finite_float(rp.get("max_re"), "max_re")
    min_im = _coerce_finite_float(rp.get("min_im"), "min_im")
    max_im = _coerce_finite_float(rp.get("max_im"), "max_im")
    if not max_re > min_re:
        raise RuntimeError(f"explicit viewport requires max_re > min_re, got {min_re!r}/{max_re!r}")
    if not max_im > min_im:
        raise RuntimeError(f"explicit viewport requires max_im > min_im, got {min_im!r}/{max_im!r}")
    return {
        "min_re": min_re,
        "max_re": max_re,
        "min_im": min_im,
        "max_im": max_im,
    }


def _fused_render_execution_config(rp):
    return {
        "raster_engine": "mt",
        "save_associated_palette": bool(rp.get("save_associated_palette", False)),
        "raster_mt_threads": int(rp.get("raster_mt_threads", 4) or 4),
        "raster_workers": int(rp.get("raster_workers", 10) or 10),
        "solve_score_threads": int(rp.get("solve_score_threads", 4) or 4),
        "raster_input_mode": "sectioned",
        "raster_sectioned_retries": int(rp.get("raster_sectioned_retries", 2) or 2),
        "raster_section_mode": str(rp.get("raster_section_mode", "logical_sections_auto") or "logical_sections_auto"),
        "raster_section_count": rp.get("raster_section_count", ""),
        "raster_section_count_auto": rp.get("raster_section_count_auto", ""),
        "finalize_workers": int(rp.get("finalize_workers", 16) or 16),
    }
def _reject_fused_unsupported_params(raw_params):
    unsupported = {
        "solve_score_hist_input_mode": "fused color runs a single clip prepass, not a separate histogram stage",
        "solve_score_hist_retries": "fused color runs a single clip prepass, not a separate histogram stage",
        "solve_score_merge_workers": "fused color has no solve-score merge stage",
        "solve_score_section_mode": "fused color has no separate solve-score section stage",
        "solve_score_section_count": "fused color has no separate solve-score section stage",
        "pixel_bin_fragment_mode": "fused color writes raw-score fragments only",
        "raster_bin_group_size": "fused color does not support grouped fragment output",
        "palette_chunk_threads": "fused color emits the associated palette inline during finalize",
        "palette_chunk_input_mode": "fused color emits the associated palette inline during finalize",
        "palette_chunk_retries": "fused color emits the associated palette inline during finalize",
        "palette_chunk_workers": "fused color emits the associated palette inline during finalize",
        "palette_section_mode": "fused color emits the associated palette inline during finalize",
        "palette_section_count": "fused color emits the associated palette inline during finalize",
        "solve_metric": "fused color takes only solve_score_chain; metric is derived from the chain",
        "solve_score_quantile": "fused color takes only solve_score_chain; quantiles live on metric chips",
        "solve_score_omega": "fused color takes only solve_score_chain; transfer ops live in the chain",
        "solve_score_omega_enabled": "fused color takes only solve_score_chain; transfer ops live in the chain",
    }
    for key, reason in unsupported.items():
        value = raw_params.get(key)
        if value not in (None, "", False):
            raise RuntimeError(f"{key} is not supported for fused color: {reason}")


def _require_pix_only_grid_params(rp):
    if rp.get("width") not in (None, "") or rp.get("height") not in (None, ""):
        raise RuntimeError("render plan no longer accepts width/height; pass pix for square output")
    try:
        pix = int(rp.get("pix"))
    except (TypeError, ValueError) as exc:
        raise RuntimeError("render plan requires integer pix") from exc
    if pix <= 0:
        raise RuntimeError(f"render plan requires pix > 0, got {pix}")
    rp["pix"] = pix
    return pix


def _build_fused_color_plan(
    *,
    job_id,
    run_id,
    task_id,
    rp,
    viewport,
    calc,
    degree,
    calc_n_coeffs,
    full_n,
    times,
    chunk_items,
    chunk_summary,
):
    raw_params = dict(rp)
    _reject_fused_unsupported_params(raw_params)

    fused_params = {
        "pix": rp["pix"],
        "tile_size": rp.get("tile_size", 2048),
        "view_mode": rp.get("view_mode", "auto"),
        "quantile": rp.get("quantile", 0.0),
        "shim": rp.get("shim", 0.05),
        "square_extent": rp.get("square_extent", 2.0),
    }
    defaults = {
        "root_transforms": [],
        "rotation": 0,
        "palette": "inferno",
        "match_mode": "none",
        "quality": 90,
        "fmt": "jpeg",
        "color_mode": "solve_score",
        "raster_engine": "mt",
        "raster_mt_threads": 4,
        "raster_workers": 10,
        "raster_input_mode": "sectioned",
        "raster_sectioned_retries": 2,
        "raster_section_mode": "logical_sections_auto",
        "raster_section_count": "",
        "solve_score_threads": "",
        "finalize_workers": 16,
        "solve_score_chain": "",
        "save_associated_palette": False,
    }
    for key, default in defaults.items():
        fused_params[key] = rp.get(key, default)

    fused_params["color_mode"] = str(fused_params.get("color_mode") or "solve_score").strip().lower()
    if fused_params["color_mode"] != "solve_score":
        raise RuntimeError("fused color supports only color_mode=solve_score")

    fused_params["match_mode"] = str(fused_params.get("match_mode") or "none").strip().lower()
    if fused_params["match_mode"] != "none":
        raise RuntimeError("fused color does not support match_mode overrides")

    palette = str(fused_params.get("palette") or "inferno").strip()
    if palette not in VALID_PALETTE_NAMES:
        raise RuntimeError(f"Invalid palette: {palette}")
    fused_params["palette"] = palette

    fused_params["raster_engine"] = _validate_raster_engine(fused_params.get("raster_engine", "mt"))
    if fused_params["raster_engine"] != "mt":
        raise RuntimeError("fused color requires raster_engine=mt")

    fused_params["raster_mt_threads"] = _validate_thread_count(
        fused_params.get("raster_mt_threads", 4),
        "raster_mt_threads",
    )
    fused_params["raster_workers"] = _validate_worker_count(
        fused_params.get("raster_workers", 10),
        "raster_workers",
    )
    solve_score_threads_value = fused_params.get("solve_score_threads", "")
    if solve_score_threads_value in (None, ""):
        solve_score_threads_value = fused_params["raster_mt_threads"]
    fused_params["solve_score_threads"] = _validate_thread_count(
        solve_score_threads_value,
        "solve_score_threads",
    )
    fused_params["raster_sectioned_retries"] = _validate_retry_count(
        fused_params.get("raster_sectioned_retries", 2),
        "raster_sectioned_retries",
    )
    fused_params["finalize_workers"] = _validate_worker_count(
        fused_params.get("finalize_workers", 16),
        "finalize_workers",
    )
    fused_params["save_associated_palette"] = _validate_boolish(
        fused_params.get("save_associated_palette", False),
        "save_associated_palette",
        False,
    )

    fused_input_mode = _validate_raster_input_mode(fused_params.get("raster_input_mode", "sectioned"))
    if fused_input_mode != "sectioned":
        raise RuntimeError("fused color requires raster_input_mode=sectioned")
    fused_params["raster_input_mode"] = "sectioned"

    fused_section_mode = normalize_section_mode(fused_params.get("raster_section_mode", "logical_sections_auto"))
    if fused_section_mode == "physical_chunks":
        raise RuntimeError("fused color does not support physical chunk raster sections")
    if fused_section_mode not in ("logical_sections", "logical_sections_auto"):
        raise RuntimeError(f"unsupported fused raster_section_mode: {fused_section_mode!r}")
    fused_params["raster_section_mode"] = fused_section_mode
    fused_params["raster_section_count"] = validate_section_count(
        fused_params.get("raster_section_count", ""),
        "raster_section_count",
        default="",
    )

    if not full_n:
        raise RuntimeError(
            "fused color requires calc N/n1 so step_scores.raw can be written "
            "for ExtractPalette and associated-palette parity"
        )
    if not chunk_summary["chunk_step_metadata_complete"]:
        raise RuntimeError(
            "fused color requires chunk step metadata on every chunk so logical sections "
            "and step_scores.raw are well-defined"
        )

    solve_score_chain = fused_params.get("solve_score_chain", "")
    if solve_score_chain in ("", None, []):
        raise RuntimeError("fused color requires solve_score_chain")
    solve_score_compiled = compile_solve_score_chain(solve_score_chain)
    solve_score_chain = solve_score_compiled["chain"]
    solve_metric = solve_score_compiled["metric"]
    solve_score_quantile = solve_score_compiled["quantile"]
    solve_score_omega = solve_score_compiled["omega"]
    solve_score_omega_enabled = solve_score_compiled["omega_enabled"]
    solve_score_uses_coeff = bool(solve_score_uses_source(solve_score_compiled, "cf"))
    solve_score_uses_param = bool(solve_score_uses_source(solve_score_compiled, "pm"))
    fused_params["solve_score_chain"] = solve_score_chain

    pix = fused_params["pix"]
    tile_size = fused_params.get("tile_size", 2048)
    n_tile_cols = math.ceil(int(pix) / tile_size)
    n_tile_rows = math.ceil(int(pix) / tile_size)
    n_tiles = n_tile_cols * n_tile_rows

    raster_section_auto = compute_safe_sectioning(
        chunk_summary["total_solves"],
        degree,
        calc_n_coeffs,
        fused_params["raster_mt_threads"],
        "raster",
        include_coeff=solve_score_uses_coeff,
        include_param=solve_score_uses_param,
    )
    fused_params["raster_section_count_auto"] = raster_section_auto["computed_section_count"]
    selected_raster_sections = fused_params["raster_section_count"]
    if fused_params["raster_section_mode"] == "logical_sections_auto":
        selected_raster_sections = raster_section_auto["computed_section_count"]
    elif selected_raster_sections in ("", None):
        selected_raster_sections = raster_section_auto["min_safe_sections"]
    if int(selected_raster_sections) < int(raster_section_auto["min_safe_sections"]):
        raise RuntimeError(
            f"raster_section_count={selected_raster_sections} is below the safe minimum "
            f"{raster_section_auto['min_safe_sections']}"
        )
    fused_params["raster_section_count"] = int(selected_raster_sections)

    raster_section_items = build_logical_section_items(
        chunk_items,
        section_count=fused_params["raster_section_count"],
        degree=degree,
        n_coeffs=calc_n_coeffs,
        include_coeff=solve_score_uses_coeff,
        include_param=solve_score_uses_param,
    )
    raster_map_items = _compact_section_ranges(raster_section_items)

    solve_score_clip_key = _solve_score_scratch_key(
        job_id,
        solve_score_compiled,
        fused_params.get("root_transforms", []),
    )
    solve_score = {
        "enabled": True,
        "threads": fused_params["solve_score_threads"],
        "chain": solve_score_chain,
        "clip_key": solve_score_clip_key,
    }

    finalize = {
        "workers": fused_params["finalize_workers"],
    }

    raster = {
        "requested_engine": "mt",
        "requested_threads": fused_params["raster_mt_threads"],
        "requested_workers": fused_params["raster_workers"],
        "requested_input_mode": "sectioned",
        "requested_sectioned_retries": fused_params["raster_sectioned_retries"],
        "requested_section_mode": fused_section_mode,
        "requested_section_count": fused_params["raster_section_count"],
        "item_count": len(raster_map_items),
        "section_item_count": len(raster_section_items),
        "threads": fused_params["raster_mt_threads"],
        "workers": fused_params["raster_workers"],
        "engine": "mt",
        "input_mode": "sectioned",
        "sectioned_retries": fused_params["raster_sectioned_retries"],
        "section_mode": fused_section_mode,
        "section_count": fused_params["raster_section_count"],
        "section_count_auto": raster_section_auto["computed_section_count"],
        "section_budget_bytes": raster_section_auto["budget_bytes"],
        "section_memory_mb": raster_section_auto["memory_mb"],
        "section_min_safe_count": raster_section_auto["min_safe_sections"],
        "logical_section": True,
        "map_items": raster_map_items,
        "function_name": RASTER_MT_FUNCTION,
        "emit_raw_score_bins": True,
        "eligible": True,
        "reason": "fused_solve_score",
    }

    artifact_id = f"color_{run_id}"
    artifact_prefix = f"renders/{job_id}/color/{artifact_id}/"
    created_at = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    associated_palette = {
        "enabled": False,
        "mode": "none",
        "palette_id": "",
        "display_name": "",
        "image_key": "",
        "preview_key": "",
        "meta_key": "",
        "raw_key": "",
        "raw_meta_key": "",
        "fragment_prefix": "",
        "source_color_artifact_id": "",
        "metric": "",
        "palette": "",
        "quantile": None,
        "omega": None,
        "omega_enabled": True,
        "score_chain": "",
    }
    if fused_params["save_associated_palette"]:
        assoc_palette_id = f"pal_{artifact_id}"
        assoc_prefix = f"renders/{job_id}/palettes/{assoc_palette_id}/"
        associated_palette = {
            "enabled": True,
            "mode": "generated",
            "palette_id": assoc_palette_id,
            "display_name": _associated_palette_display_name(
                solve_score_chain,
                solve_metric,
                solve_score_quantile,
                palette,
            ),
            "image_key": assoc_prefix + "image.jpeg",
            "preview_key": assoc_prefix + "preview.png",
            "meta_key": assoc_prefix + "meta.json",
            "raw_key": assoc_prefix + "greyscale.raw",
            "raw_meta_key": assoc_prefix + "greyscale.meta.json",
            "fragment_prefix": assoc_prefix + "fragments/section_",
            "source_color_artifact_id": artifact_id,
            "metric": solve_metric,
            "palette": palette,
            "quantile": solve_score_quantile,
            "omega": solve_score_omega,
            "omega_enabled": solve_score_omega_enabled,
            "score_chain": solve_score_chain,
        }

    render_execution = _fused_render_execution_config(fused_params)
    solve_source_manifest = build_solve_source_manifest(
        chunk_items,
        job_id=job_id,
        degree=degree,
        n_coeffs=calc_n_coeffs,
        include_coeff=solve_score_uses_coeff,
        include_param=solve_score_uses_param,
    )

    artifact_meta = {
        "artifact_id": artifact_id,
        "family": "color",
        "created_at": created_at,
        "degree": str(degree),
        "pix": str(pix),
        "tile_size": str(tile_size),
        "view_mode": str(fused_params.get("view_mode", "auto")),
        "quantile": str(fused_params.get("quantile", 0.0)),
        "shim": str(fused_params.get("shim", 0.05)),
        "square_extent": str(fused_params.get("square_extent", 2.0)),
        "min_re": str(viewport["min_re"]),
        "max_re": str(viewport["max_re"]),
        "min_im": str(viewport["min_im"]),
        "max_im": str(viewport["max_im"]),
        "rotation": str(fused_params.get("rotation", 0.0)),
        "root_transforms": json.dumps(fused_params.get("root_transforms", [])),
        "render_execution": json.dumps(render_execution, separators=(",", ":")),
        "format": "jpeg" if fused_params.get("fmt", "jpeg") != "png" else "png",
        "quality": str(fused_params.get("quality", 90)),
        "color_mode": "solve_score",
        "match_mode": "none",
        "palette": palette,
        "background_color": DEFAULT_BACKGROUND_COLOR,
        "background_threshold": str(DEFAULT_BACKGROUND_THRESHOLD),
        "repalette_capable": "false",
        "pixel_bins_drive_rgb": "false",
        "rgb_source": "raw_score_bins",
        "pixel_bins_prefix": "",
        "pixel_bins_empty": "",
        "pixel_bins_layout": "",
        "raw_key": artifact_prefix + "greyscale.raw",
        "raw_meta_key": artifact_prefix + "greyscale.meta.json",
        "fragment_prefix": artifact_prefix + "fragments/section_",
    }
    artifact_meta.update(
        emit_solve_score_metadata(
            "solve",
            metric=solve_metric,
            quantile=solve_score_quantile,
            omega=solve_score_omega,
            omega_enabled=solve_score_omega_enabled,
            chain=solve_score_chain,
            include_legacy_scalars=False,
        )
    )
    artifact_meta["score_program"] = solve_score_compiled["program_spec"]
    if associated_palette["enabled"]:
        artifact_meta.update({
            "associated_palette_mode": associated_palette["mode"],
            "associated_palette_id": associated_palette["palette_id"],
            "associated_palette_display_name": associated_palette["display_name"],
            "associated_palette_image_key": associated_palette["image_key"],
            "associated_palette_preview_key": associated_palette["preview_key"],
            "associated_palette_palette": str(associated_palette["palette"]),
        })
        artifact_meta.update(
            emit_solve_score_metadata(
                "associated_palette",
                metric=associated_palette["metric"],
                quantile=associated_palette["quantile"],
                omega=associated_palette["omega"],
                omega_enabled=associated_palette["omega_enabled"],
                chain=associated_palette["score_chain"],
            )
        )

    plan_params_digest = _plan_params_digest(
        viewport=viewport,
        pix=pix,
        tile_size=tile_size,
        root_transforms=fused_params.get("root_transforms", []),
    )
    ext = "png" if fused_params.get("fmt", "jpeg") == "png" else "jpeg"
    outputs = {
        "family": "color",
        "artifact_id": artifact_id,
        "artifact_prefix": artifact_prefix,
        "created_at": created_at,
        "image_key": artifact_prefix + f"image.{ext}",
        "preview_key": artifact_prefix + "preview.png",
        "meta_key": artifact_prefix + "meta.json",
        "raw_key": artifact_prefix + "greyscale.raw",
        "raw_meta_key": artifact_prefix + "greyscale.meta.json",
        "fragment_prefix": artifact_prefix + "fragments/section_",
        "plan_params_digest": plan_params_digest,
        "metadata": artifact_meta,
        "repalette_capable": False,
        "pixel_bins_drive_rgb": False,
    }

    plan = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": "color",
        "params": fused_params,
        "viewport": viewport,
        "calc": {
            "degree": degree,
            "N": full_n,
            "times": times,
            "n_chunks": len(chunk_items),
            "lores_bin_key": calc.get("lores", {}).get("bin_key", ""),
            "lores_coeffs_key": _fallback_lores_coeffs_key(job_id, calc),
            "lores_params_key": _fallback_lores_params_key(job_id, calc),
            "n_coeffs": calc_n_coeffs,
        },
        "grid": {
            "pix": pix,
            "tile_size": tile_size,
            "n_tile_cols": n_tile_cols,
            "n_tile_rows": n_tile_rows,
            "n_tiles": n_tiles,
        },
        "solve_source_manifest": solve_source_manifest,
        "physical_source_items": [],
        "tile_items": [],
        "solve_score": solve_score,
        "finalize": finalize,
        "raster": raster,
        "associated_palette": associated_palette,
        "render_execution": render_execution,
        "outputs": outputs,
    }
    return plan


def _bilevel_section_plan(chunk_items, *, chunk_summary, degree, calc_n_coeffs, rp):
    section_auto = compute_safe_sectioning(
        chunk_summary["total_solves"],
        degree,
        calc_n_coeffs,
        1,
        "raster",
        include_coeff=False,
        include_param=False,
    )
    requested_mode = normalize_section_mode(rp.get("raster_section_mode", "logical_sections_auto"))
    if requested_mode == "physical_chunks":
        requested_mode = "logical_sections_auto"
    requested_count = validate_section_count(
        rp.get("raster_section_count", ""),
        "raster_section_count",
        default="",
    )
    if not chunk_summary["chunk_step_metadata_complete"]:
        raise RuntimeError("bilevel logical sections require chunk step metadata on every chunk")

    selected_count = requested_count
    if requested_mode == "logical_sections_auto":
        selected_count = section_auto["computed_section_count"]
    elif selected_count in ("", None):
        selected_count = section_auto["min_safe_sections"]
    if int(selected_count) < int(section_auto["min_safe_sections"]):
        raise RuntimeError(
            f"raster_section_count={selected_count} is below the safe minimum "
            f"{section_auto['min_safe_sections']}"
        )
    section_items = build_logical_section_items(
        chunk_items,
        section_count=int(selected_count),
        degree=degree,
        n_coeffs=calc_n_coeffs,
        include_coeff=False,
        include_param=False,
    )
    return {
        "section_mode": requested_mode,
        "section_count": int(selected_count),
        "section_count_auto": section_auto["computed_section_count"],
        "section_budget_bytes": section_auto["budget_bytes"],
        "section_memory_mb": section_auto["memory_mb"],
        "section_min_safe_count": section_auto["min_safe_sections"],
        "logical_section": True,
        "item_count": len(section_items),
        "section_items": _compact_section_ranges(section_items),
    }


def _build_non_color_plan(
    *,
    job_id,
    run_id,
    task_id,
    mode,
    rp,
    viewport,
    calc,
    degree,
    calc_n_coeffs,
    full_n,
    times,
    chunk_items,
    chunk_summary,
):
    if mode not in ("bilevel", "coeff_bilevel"):
        raise RuntimeError(f"unsupported non-color mode: {mode!r}")

    non_color_params = {
        "pix": rp["pix"],
        "tile_size": rp.get("tile_size", 4096),
        "view_mode": rp.get("view_mode", "auto"),
        "quantile": rp.get("quantile", 0.0),
        "shim": rp.get("shim", 0.05),
        "square_extent": rp.get("square_extent", 2.0),
        "root_transforms": rp.get("root_transforms", []),
        "rotation": rp.get("rotation", 0),
    }
    if mode == "bilevel":
        non_color_params["raster_section_mode"] = normalize_section_mode(
            rp.get("raster_section_mode", "logical_sections_auto")
        )
        non_color_params["raster_section_count"] = validate_section_count(
            rp.get("raster_section_count", ""),
            "raster_section_count",
            default="",
        )

    pix = int(non_color_params["pix"])
    tile_size = int(non_color_params["tile_size"])
    n_tile_cols = math.ceil(pix / tile_size)
    n_tile_rows = math.ceil(pix / tile_size)
    n_tiles = n_tile_cols * n_tile_rows
    raw_tile_prefix = f"renders/{job_id}/tile_"
    tile_items = [{"tile_idx": t} for t in range(n_tiles)]

    bilevel = {
        "enabled": mode == "bilevel",
        "section_mode": "",
        "section_count": "",
        "section_count_auto": "",
        "section_budget_bytes": 0,
        "section_memory_mb": 0,
        "section_min_safe_count": 0,
        "logical_section": False,
        "item_count": 0,
        "section_items": [],
        "fragment_prefix": "",
    }
    if mode == "bilevel":
        bilevel.update(
            _bilevel_section_plan(
                chunk_items,
                chunk_summary=chunk_summary,
                degree=degree,
                calc_n_coeffs=calc_n_coeffs,
                rp=non_color_params,
            )
        )
        bilevel["fragment_prefix"] = f"renders/{job_id}/bilevel_section_"
        non_color_params["raster_section_count_auto"] = bilevel["section_count_auto"]
        non_color_params["raster_section_count"] = bilevel["section_count"]

    solve_source_manifest = (
        build_solve_source_manifest(
            chunk_items,
            job_id=job_id,
            degree=degree,
            n_coeffs=calc_n_coeffs,
            include_coeff=False,
            include_param=False,
        )
        if mode == "bilevel" else {}
    )

    artifact_family = "coeffs" if mode == "coeff_bilevel" else mode
    artifact_id = f"{artifact_family}_{run_id}"
    artifact_prefix = f"renders/{job_id}/{artifact_family}/{artifact_id}/"
    created_at = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    render_execution = {}
    artifact_meta = {
        "artifact_id": artifact_id,
        "family": artifact_family,
        "created_at": created_at,
        "degree": str(degree),
        "pix": str(pix),
        "tile_size": str(tile_size),
        "view_mode": str(non_color_params.get("view_mode", "auto")),
        "quantile": str(non_color_params.get("quantile", 0.0)),
        "shim": str(non_color_params.get("shim", 0.05)),
        "square_extent": str(non_color_params.get("square_extent", 2.0)),
        "min_re": str(viewport["min_re"]),
        "max_re": str(viewport["max_re"]),
        "min_im": str(viewport["min_im"]),
        "max_im": str(viewport["max_im"]),
        "rotation": str(non_color_params.get("rotation", 0.0)),
        "root_transforms": json.dumps(non_color_params.get("root_transforms", [])),
        "render_execution": json.dumps(render_execution, separators=(",", ":")),
    }
    if mode == "bilevel":
        artifact_meta.update({
            "format": "tif",
            "mode": "bilevel",
            "bilevel_pipeline": BILEVEL_SPARSE_PIPELINE,
            "bilevel_section_mode": str(bilevel["section_mode"]),
            "bilevel_section_count": str(bilevel["section_count"]),
        })
    else:
        artifact_meta.update({
            "format": "tif",
            "mode": "coeffs",
        })

    plan_params_digest = _plan_params_digest(
        viewport=viewport,
        pix=pix,
        tile_size=tile_size,
        root_transforms=non_color_params.get("root_transforms", []),
    )
    outputs = {
        "family": artifact_family,
        "artifact_id": artifact_id,
        "artifact_prefix": artifact_prefix,
        "created_at": created_at,
        "image_key": artifact_prefix + "image.tif",
        "preview_key": artifact_prefix + "preview.png",
        "meta_key": artifact_prefix + "meta.json",
        "raw_key": "",
        "raw_meta_key": "",
        "fragment_prefix": "",
        "plan_params_digest": plan_params_digest,
        "bilevel_key": artifact_prefix + "image.tif",
        "coeff_bilevel_key": artifact_prefix + "image.tif",
        "metadata": artifact_meta,
        "repalette_capable": False,
        "pixel_bins_drive_rgb": False,
    }

    plan = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": mode,
        "params": non_color_params,
        "viewport": viewport,
        "calc": {
            "degree": degree,
            "N": full_n,
            "times": times,
            "n_chunks": len(chunk_items),
            "coeffs_keys": calc.get("coeffs_keys", []) if mode == "coeff_bilevel" else [],
            "n_coeffs": calc_n_coeffs,
        },
        "grid": {
            "pix": pix,
            "tile_size": tile_size,
            "n_tile_cols": n_tile_cols,
            "n_tile_rows": n_tile_rows,
            "n_tiles": n_tiles,
            "raw_tile_prefix": raw_tile_prefix,
        },
        "solve_source_manifest": solve_source_manifest,
        "physical_source_items": chunk_items if mode == "coeff_bilevel" else [],
        "tile_items": tile_items if mode == "coeff_bilevel" else [],
        "bilevel": bilevel,
        "render_execution": render_execution,
        "outputs": outputs,
    }
    return plan


def _plan_size_error_message(plan, plan_size):
    grid = dict(plan.get("grid") or {})
    raster = dict(plan.get("raster") or {})
    calc = dict(plan.get("calc") or {})
    return (
        f"Plan too large: {plan_size} bytes > {MAX_PLAN_BYTES} limit. "
        f"Counts: chunks={int(calc.get('n_chunks') or 0)}, "
        f"raster_items={int(raster.get('item_count') or 0)}, "
        f"tiles={int(grid.get('n_tiles') or 0)}. "
        f"Controls: reduce pix, increase tile_size, or lower logical section counts."
    )


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    run_id = params["run_id"]
    task_id = params["task_id"]
    mode = params["mode"]
    rp = dict(params.get("params", {}) or {})
    _require_pix_only_grid_params(rp)

    # Clean previous render intermediates
    pipeline = "color" if mode == "color" else mode
    _storage_call("/clean-render", {"job_id": job_id, "pipeline": pipeline})

    # Load calc metadata
    calc = _load_calc(job_id)

    # Compute viewport
    viewport = _compute_viewport(job_id, rp)

    # Extract calc fields
    degree = _coerce_degree(calc)
    calc_n_coeffs = _coerce_n_coeffs(calc, degree)
    full_n = int(calc.get("N", calc.get("n1", 0)) or 0)
    times = int(calc.get("times", 1) or 1)
    chunk_items = build_chunk_items(calc, job_id)
    n_chunks = len(chunk_items)
    chunk_summary = summarize_chunk_items(chunk_items, degree, calc_n_coeffs)

    if mode == "color":
        plan = _build_fused_color_plan(
            job_id=job_id,
            run_id=run_id,
            task_id=task_id,
            rp=rp,
            viewport=viewport,
            calc=calc,
            degree=degree,
            calc_n_coeffs=calc_n_coeffs,
            full_n=full_n,
            times=times,
            chunk_items=chunk_items,
            chunk_summary=chunk_summary,
        )
        plan_json = json.dumps(plan)
        if len(plan_json) > MAX_PLAN_BYTES:
            raise RuntimeError(_plan_size_error_message(plan, len(plan_json)))
        return ok_response(plan)
    plan = _build_non_color_plan(
        job_id=job_id,
        run_id=run_id,
        task_id=task_id,
        mode=mode,
        rp=rp,
        viewport=viewport,
        calc=calc,
        degree=degree,
        calc_n_coeffs=calc_n_coeffs,
        full_n=full_n,
        times=times,
        chunk_items=chunk_items,
        chunk_summary=chunk_summary,
    )
    plan_json = json.dumps(plan)
    if len(plan_json) > MAX_PLAN_BYTES:
        raise RuntimeError(_plan_size_error_message(plan, len(plan_json)))
    return ok_response(plan)


def _load_calc(job_id):
    """Load calc.json from S3."""
    resp = _storage_call("/detail", {"job_id": job_id})
    calc = resp.get("calc", {})
    if not calc:
        raise RuntimeError(f"calc.json missing for {job_id}")
    return calc


def _compute_viewport(job_id, rp):
    """Compute viewport from params."""
    view_mode = str(rp.get("view_mode") or "auto").strip().lower()
    if view_mode == "explicit":
        return _explicit_viewport_from_params(rp)
    if view_mode == "square":
        ext = _coerce_finite_float(rp.get("square_extent", 2.0), "square_extent")
        if ext <= 0.0:
            raise RuntimeError(f"square_extent must be > 0, got {ext}")
        return {
            "min_re": -ext,
            "max_re": ext,
            "min_im": -ext,
            "max_im": ext,
        }
    if view_mode != "auto":
        raise RuntimeError(f"unsupported view_mode: {view_mode!r}")

    shim = _coerce_finite_float(rp.get("shim", 0.05), "shim")
    vp = _invoke_sync(VIEWPORT_FUNCTION, {
        "job_id": job_id,
        "quantile": rp.get("quantile", 0.0),
        "shim": shim,
    })
    q_re = vp.get("q_re")
    q_im = vp.get("q_im")
    if not (isinstance(q_re, (list, tuple)) and len(q_re) == 2):
        raise RuntimeError("viewport lambda response missing q_re bounds")
    if not (isinstance(q_im, (list, tuple)) and len(q_im) == 2):
        raise RuntimeError("viewport lambda response missing q_im bounds")
    q_min_re = _coerce_finite_float(q_re[0], "q_re[0]")
    q_max_re = _coerce_finite_float(q_re[1], "q_re[1]")
    q_min_im = _coerce_finite_float(q_im[0], "q_im[0]")
    q_max_im = _coerce_finite_float(q_im[1], "q_im[1]")
    center_re = (q_min_re + q_max_re) / 2.0
    center_im = (q_min_im + q_max_im) / 2.0
    range_re = (q_max_re - q_min_re) * (1.0 + shim)
    range_im = (q_max_im - q_min_im) * (1.0 + shim)
    fallback_span = None
    if range_re <= 0.0 or range_im <= 0.0:
        positive_span = max(range_re, range_im)
        if positive_span > 0.0:
            # When one axis collapses to a line, keep auto mode bounded by
            # inheriting the non-degenerate span instead of exploding to a
            # full REF_SIZE-derived camera box.
            fallback_span = positive_span
        else:
            fallback_scale_ref = _coerce_finite_float(vp.get("scale_ref", vp.get("scale")), "scale_ref")
            if fallback_scale_ref <= 0.0:
                raise RuntimeError(f"viewport lambda returned non-positive scale_ref: {fallback_scale_ref!r}")
            fallback_span = float(REF_SIZE) / fallback_scale_ref
    if range_re <= 0.0:
        range_re = fallback_span
    if range_im <= 0.0:
        range_im = fallback_span
    return {
        "min_re": center_re - (range_re / 2.0),
        "max_re": center_re + (range_re / 2.0),
        "min_im": center_im - (range_im / 2.0),
        "max_im": center_im + (range_im / 2.0),
    }


def _invoke_sync(function_name, payload):
    """Invoke a Lambda synchronously and return parsed response."""
    r = lambda_client.invoke(
        FunctionName=function_name,
        InvocationType="RequestResponse",
        Payload=json.dumps(payload).encode(),
    )
    body = json.loads(r["Payload"].read())
    if isinstance(body, dict) and "body" in body:
        return json.loads(body["body"])
    return body


def _storage_call(path, body):
    """Call the storage Lambda synchronously."""
    return _invoke_sync(STORAGE_FUNCTION, {
        "body": json.dumps(body),
        "path": path,
        "requestContext": {"http": {"method": "POST", "path": path}},
    })
