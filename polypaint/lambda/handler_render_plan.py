"""
Render plan Lambda — computes the workflow plan for Step Functions.

Loads calc metadata, computes viewport, normalizes params, produces
compact stripe/tile arrays and output keys. Does NOT dispatch workers
or poll for completion.

Called once per render execution as the BuildPlan step.
"""
import json
import math
import os
from datetime import datetime, timezone

import boto3

from palette_names import VALID_PALETTE_NAMES
from shared import BUCKET, parse_body, ok_response
from solve_score_chain import (
    VALID_SOLVE_SCORE_METRICS,
    compile_solve_score_chain_or_legacy,
    emit_solve_score_metadata,
)

s3 = boto3.client("s3")
lambda_client = boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1"))

VIEWPORT_FUNCTION = os.environ.get("VIEWPORT_FUNCTION", "polypaint-viewport")
STORAGE_FUNCTION = os.environ.get("STORAGE_FUNCTION", "polypaint-storage")
RASTER_FUNCTION = os.environ.get("RASTER_FUNCTION", "polypaint-raster")
RASTER_MT_FUNCTION = os.environ.get("RASTER_MT_FUNCTION", "polypaint-raster-mt")

MAX_PLAN_BYTES = 200 * 1024  # 200 KB — fail fast before hitting 256 KB SFN limit
DEFAULT_BACKGROUND_COLOR = "000000"
DEFAULT_BACKGROUND_THRESHOLD = 4


def _build_chunk_items(calc, job_id):
    chunks = list(calc.get("chunks", calc.get("stripes", [])) or [])
    if chunks:
        degree = int(calc.get("degree", 1) or 1)
        record_bytes = degree * 2 * 4
        chunk_items = []
        step_start = 0
        ordered_chunks = sorted(
            chunks,
            key=lambda raw: raw.get("idx", raw.get("chunk_idx", raw.get("stripe_idx", 0))),
        )
        for raw in ordered_chunks:
            idx = raw.get("idx", raw.get("chunk_idx", raw.get("stripe_idx")))
            bin_key = raw.get("bin_key", raw.get("s3_key"))
            if idx is None or not bin_key:
                raise RuntimeError(f"Invalid chunk metadata: idx={idx} bin_key={bin_key!r}")
            item = {"chunk_idx": int(idx), "bin_key": str(bin_key)}
            bin_size = raw.get("bin_size")
            step_count = raw.get("step_count", raw.get("n_t"))
            if step_count in ("", None):
                if bin_size not in ("", None):
                    step_count = int(bin_size) // record_bytes
            if step_count not in ("", None):
                step_count = int(step_count)
                if step_count <= 0:
                    raise RuntimeError(f"Invalid chunk metadata: idx={idx} step_count={step_count}")
                item["step_start"] = step_start
                item["step_count"] = step_count
                item["bin_size"] = int(bin_size) if bin_size not in ("", None) else int(step_count) * record_bytes
                step_start += step_count
            elif bin_size not in ("", None):
                item["bin_size"] = int(bin_size)
            chunk_items.append(item)
        return chunk_items

    n_chunks = calc.get("n_chunks", calc.get("n_stripes", 10))
    return [{"chunk_idx": c, "bin_key": f"renders/{job_id}/chunk_{c}.bin"} for c in range(n_chunks)]


def _validate_omega(value):
    try:
        omega = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_omega must be numeric, got {value!r}")
    if not (1.0 <= omega <= 10.0):
        raise RuntimeError(f"solve_score_omega must be in [1, 10], got {omega}")
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


def _omega_display(enabled, omega):
    return f"w={omega:g}" if enabled else "w=off"


def _associated_palette_display_name(metric, palette, quantile, omega, omega_enabled):
    parts = []
    if metric:
        parts.append(str(metric))
    if quantile not in ("", None):
        try:
            parts.append(f"q={float(quantile) * 100:.1f}%")
        except Exception:
            parts.append(f"q={quantile}")
    if omega not in ("", None):
        parts.append(_omega_display(omega_enabled, float(omega)))
    if palette:
        parts.append(str(palette))
    return " ".join(part for part in parts if part)


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


def _validate_hist_input_mode(value):
    mode = str(value or "tmpfile").strip().lower()
    if mode not in ("tmpfile", "stdin", "sectioned"):
        raise RuntimeError(f"solve_score_hist_input_mode must be 'tmpfile', 'stdin', or 'sectioned', got {value!r}")
    return mode


def _load_palette_meta(job_id, palette_id):
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=f"renders/{job_id}/palettes/{palette_id}/meta.json")
    except Exception as e:
        raise RuntimeError(f"Saved palette not found: {palette_id}") from e
    meta = json.loads(obj["Body"].read())
    if meta.get("job_id") and meta.get("job_id") != job_id:
        raise RuntimeError(f"Saved palette {palette_id} belongs to {meta.get('job_id')}, not {job_id}")
    return meta


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    run_id = params["run_id"]
    task_id = params["task_id"]
    mode = params["mode"]
    rp = params.get("params", {})

    # Clean previous render intermediates
    pipeline = "color" if mode == "color" else mode
    _storage_call("/clean-render", {"job_id": job_id, "pipeline": pipeline})

    # Load calc metadata
    calc = _load_calc(job_id)

    # Compute viewport
    viewport = _compute_viewport(job_id, rp)

    # Extract calc fields
    degree = calc.get("degree", 1)
    full_n = int(calc.get("N", calc.get("n1", 0)) or 0)
    times = int(calc.get("times", 1) or 1)
    chunk_items = _build_chunk_items(calc, job_id)
    n_chunks = len(chunk_items)

    # Grid computation
    pix = rp["pix"]
    tile_size = rp.get("tile_size", 2048 if mode == "color" else 4096)
    n_tile_cols = math.ceil(pix / tile_size)
    n_tile_rows = math.ceil(pix / tile_size)
    n_tiles = n_tile_cols * n_tile_rows

    # Precompute tile keys for encode
    tile_keys = [f"renders/{job_id}/tile_{t:04d}.raw" for t in range(n_tiles)]

    # Compact tile items (precompute tile_w/tile_h to avoid ASL arithmetic)
    tile_items = []
    for t in range(n_tiles):
        t_row = t // n_tile_cols
        t_col = t % n_tile_cols
        tw = min(tile_size, pix - t_col * tile_size)
        th = min(tile_size, pix - t_row * tile_size)
        tile_items.append({
            "tile_idx": t, "tile_row": t_row, "tile_col": t_col,
            "tile_w": tw, "tile_h": th,
        })

    # Ensure ALL fields referenced by ASL JSONPaths exist in params.
    # ASL crashes with States.Runtime on missing paths — no null fallback.
    _PARAM_DEFAULTS = {
        "root_transforms": [],
        "rotation": 0,
        "constant_color": "ffffff",
        "palette": "inferno",
        "saved_palette_id": "",
        "match_mode": "none",
        "quality": 90,
        "fmt": "jpeg",
        "color_mode": "rainbow",
        "raster_engine": "single",
        "raster_mt_threads": 4,
        "raster_input_mode": "tmpfile",
        "raster_sectioned_retries": 2,
        "solve_score_threads": "",
        "solve_score_merge_workers": 16,
        "finalize_workers": 16,
        "solve_metric": "proximity",
        "solve_score_chain": "",
        "solve_score_quantile": 0.001,
        "solve_score_omega": 1.0,
        "solve_score_omega_enabled": True,
        "solve_score_hist_input_mode": "tmpfile",
        "solve_score_hist_retries": 2,
        "save_associated_palette": False,
        "palette_chunk_threads": "",
        "palette_chunk_input_mode": "",
        "palette_chunk_retries": "",
        "palette_chunk_workers": "",
    }
    for key, default in _PARAM_DEFAULTS.items():
        if key not in rp:
            rp[key] = default
    rp["raster_engine"] = _validate_raster_engine(rp.get("raster_engine", "single"))
    rp["raster_mt_threads"] = _validate_thread_count(rp.get("raster_mt_threads", 4), "raster_mt_threads")
    rp["raster_input_mode"] = _validate_raster_input_mode(rp.get("raster_input_mode", "tmpfile"))
    rp["raster_sectioned_retries"] = _validate_retry_count(
        rp.get("raster_sectioned_retries", 2),
        "raster_sectioned_retries",
    )
    solve_score_threads_value = rp.get("solve_score_threads", "")
    if solve_score_threads_value in (None, ""):
        solve_score_threads_value = rp["raster_mt_threads"] if rp["raster_engine"] == "mt" else 1
    rp["solve_score_threads"] = _validate_thread_count(solve_score_threads_value, "solve_score_threads")
    rp["solve_score_merge_workers"] = _validate_worker_count(
        rp.get("solve_score_merge_workers", 16),
        "solve_score_merge_workers",
    )
    rp["finalize_workers"] = _validate_worker_count(
        rp.get("finalize_workers", 16),
        "finalize_workers",
    )
    rp["solve_score_hist_input_mode"] = _validate_hist_input_mode(rp.get("solve_score_hist_input_mode", "tmpfile"))
    rp["solve_score_hist_retries"] = _validate_retry_count(
        rp.get("solve_score_hist_retries", 2),
        "solve_score_hist_retries",
    )
    rp["save_associated_palette"] = _validate_boolish(
        rp.get("save_associated_palette", False),
        "save_associated_palette",
        False,
    )
    palette_chunk_threads_value = rp.get("palette_chunk_threads", "")
    if palette_chunk_threads_value in (None, ""):
        palette_chunk_threads_value = rp["raster_mt_threads"] if rp["raster_engine"] == "mt" else 4
    palette_chunk_input_value = rp.get("palette_chunk_input_mode", "")
    if palette_chunk_input_value in (None, ""):
        palette_chunk_input_value = rp["raster_input_mode"] if rp["raster_engine"] == "mt" else "sectioned"
    palette_chunk_retries_value = rp.get("palette_chunk_retries", "")
    if palette_chunk_retries_value in (None, ""):
        palette_chunk_retries_value = rp["raster_sectioned_retries"] if palette_chunk_input_value == "sectioned" else 0
    palette_chunk_workers_value = rp.get("palette_chunk_workers", "")
    if palette_chunk_workers_value in (None, ""):
        palette_chunk_workers_value = 16
    rp["palette_chunk_threads"] = _validate_thread_count(palette_chunk_threads_value, "palette_chunk_threads")
    rp["palette_chunk_input_mode"] = _validate_raster_input_mode(palette_chunk_input_value)
    rp["palette_chunk_retries"] = _validate_retry_count(palette_chunk_retries_value, "palette_chunk_retries")
    rp["palette_chunk_workers"] = _validate_worker_count(palette_chunk_workers_value, "palette_chunk_workers")

    # Normalize solve-score params
    color_mode = rp.get("color_mode", "rainbow")
    if color_mode == "solve_proximity":
        color_mode = "solve_score"
        rp["color_mode"] = "solve_score"
        if not rp.get("solve_metric"):
            rp["solve_metric"] = "proximity"

    solve_metric = rp.get("solve_metric", "proximity")
    solve_score_chain = rp.get("solve_score_chain", "")
    solve_score_quantile = rp.get("solve_score_quantile", 0.001)
    solve_score_omega = rp.get("solve_score_omega", 1.0)
    solve_score_omega_enabled = rp.get("solve_score_omega_enabled", True)
    palette = rp.get("palette", "inferno")
    solve_score_compiled = None
    saved_palette_meta = {}
    saved_palette = {
        "enabled": False,
        "palette_id": "",
        "display_name": "",
        "palette": "",
        "metric": "",
        "quantile": None,
        "omega": 1.0,
        "omega_enabled": True,
        "score_chain": "",
        "chunk_bins_prefix": "",
        "data_layout": "",
    }

    if color_mode == "saved_palette":
        saved_palette_id = str(rp.get("saved_palette_id", "")).strip()
        if not saved_palette_id:
            raise RuntimeError("saved_palette color mode requires saved_palette_id")
        source_meta = _load_palette_meta(job_id, saved_palette_id)
        if not source_meta.get("render_reusable"):
            raise RuntimeError(
                "Selected palette artifact is not render-reusable. Regenerate the palette to make it reusable."
            )
        if source_meta.get("data_layout") != "chunk_all_pass_v1":
            raise RuntimeError(
                f"Saved palette {saved_palette_id} has unsupported data layout: {source_meta.get('data_layout')!r}"
            )
        saved_palette_meta = dict(source_meta)
        src_n = int(source_meta.get("N", 0) or 0)
        src_degree = int(source_meta.get("degree", 0) or 0)
        src_times = int(source_meta.get("times", 0) or 0)
        calc_n = int(calc.get("N", calc.get("n1", 0)) or 0)
        calc_times = int(calc.get("times", 1) or 1)
        if src_n and calc_n and src_n != calc_n:
            raise RuntimeError(f"Saved palette N mismatch: palette={src_n}, calc={calc_n}")
        if src_degree and degree and src_degree != degree:
            raise RuntimeError(f"Saved palette degree mismatch: palette={src_degree}, calc={degree}")
        if src_times and calc_times and src_times != calc_times:
            raise RuntimeError(f"Saved palette times mismatch: palette={src_times}, calc={calc_times}")

        requested_palette = str(rp.get("palette", "") or "").strip()
        source_palette = str(source_meta.get("palette", "") or "").strip()
        palette = requested_palette or source_palette or palette
        if palette not in VALID_PALETTE_NAMES:
            raise RuntimeError(f"Invalid palette: {palette}")
        source_compiled = compile_solve_score_chain_or_legacy(
            source_meta.get("solve_score_chain", ""),
            source_meta.get("metric", solve_metric),
            source_meta.get("solve_score_omega", solve_score_omega),
            source_meta.get("solve_score_omega_enabled", True),
            default_metric=source_meta.get("metric", solve_metric) or "proximity",
        )
        solve_metric = source_compiled["metric"]
        solve_score_quantile = float(source_meta.get("solve_score_quantile", solve_score_quantile))
        solve_score_omega = source_compiled["omega"]
        solve_score_omega_enabled = source_compiled["omega_enabled"]
        solve_score_chain = source_compiled["chain"]
        rp["palette"] = palette
        rp["root_transforms"] = list(source_meta.get("root_transforms") or [])
        saved_palette = {
            "enabled": True,
            "palette_id": saved_palette_id,
            "display_name": source_meta.get("display_name", ""),
            "palette": source_palette,
            "metric": solve_metric,
            "quantile": solve_score_quantile,
            "omega": solve_score_omega,
            "omega_enabled": solve_score_omega_enabled,
            "score_chain": source_compiled["chain"],
            "chunk_bins_prefix": source_meta.get("chunk_bins_prefix", f"renders/{job_id}/palettes/{saved_palette_id}/chunks/palette_bins_chunk_"),
            "data_layout": source_meta.get("data_layout", ""),
        }
    elif palette not in VALID_PALETTE_NAMES:
        raise RuntimeError(f"Invalid palette: {palette}")

    solve_score_enabled = color_mode == "solve_score"
    if solve_score_enabled:
        try:
            solve_score_quantile = float(solve_score_quantile)
        except (TypeError, ValueError):
            raise RuntimeError(f"solve_score_quantile must be numeric, got {solve_score_quantile!r}")
        if not (0.001 <= solve_score_quantile <= 0.05):
            raise RuntimeError(f"solve_score_quantile must be in [0.001, 0.05], got {solve_score_quantile}")
        solve_score_compiled = compile_solve_score_chain_or_legacy(
            solve_score_chain,
            solve_metric,
            solve_score_omega,
            solve_score_omega_enabled,
            default_metric="proximity",
        )
        solve_metric = solve_score_compiled["metric"]
        solve_score_omega = solve_score_compiled["omega"]
        solve_score_omega_enabled = solve_score_compiled["omega_enabled"]
        solve_score_chain = solve_score_compiled["chain"]
        rp["solve_score_chain"] = solve_score_chain
    elif color_mode == "saved_palette":
        solve_score_compiled = compile_solve_score_chain_or_legacy(
            saved_palette["score_chain"],
            saved_palette["metric"],
            saved_palette["omega"],
            saved_palette["omega_enabled"],
            default_metric=saved_palette["metric"] or "proximity",
        )
        solve_score_chain = solve_score_compiled["chain"]
        rp["solve_score_chain"] = solve_score_chain
    else:
        if solve_metric and solve_metric not in VALID_SOLVE_SCORE_METRICS:
            raise RuntimeError(f"Invalid solve-score metric: {solve_metric!r}")
        solve_score_omega = _validate_omega(solve_score_omega)
        solve_score_omega_enabled = _validate_omega_enabled(solve_score_omega_enabled)
    rp["solve_score_omega"] = solve_score_omega
    rp["solve_score_omega_enabled"] = solve_score_omega_enabled
    rp["solve_metric"] = solve_metric

    solve_score = {
        "enabled": solve_score_enabled,
        "threads": rp["solve_score_threads"] if solve_score_enabled else 1,
        "merge_workers": rp["solve_score_merge_workers"] if solve_score_enabled else 1,
        "metric": solve_metric,
        "quantile": solve_score_quantile,
        "omega": solve_score_omega,
        "omega_enabled": solve_score_omega_enabled,
        "chain": solve_score_chain,
        "hist_input_mode": rp["solve_score_hist_input_mode"] if solve_score_enabled else "tmpfile",
        "hist_retries": rp["solve_score_hist_retries"] if solve_score_enabled else 0,
        "clip_key": f"renders/{job_id}/solve_scores/{solve_metric}_clip.json",
        "hist_prefix": f"renders/{job_id}/solve_scores/{solve_metric}/",
        "bins_key": f"renders/{job_id}/solve_scores/{solve_metric}_bins.json",
    }
    finalize = {
        "workers": rp["finalize_workers"],
    }

    color_repalette_capable = mode == "color" and color_mode in ("solve_score", "saved_palette")
    requested_raster_engine = rp.get("raster_engine", "single")
    requested_raster_threads = rp.get("raster_mt_threads", 4)
    raster = {
        "requested_engine": requested_raster_engine,
        "requested_threads": requested_raster_threads,
        "requested_input_mode": rp.get("raster_input_mode", "tmpfile"),
        "requested_sectioned_retries": rp.get("raster_sectioned_retries", 2),
        "threads": 1,
        "engine": "single",
        "input_mode": "tmpfile",
        "sectioned_retries": 0,
        "function_name": RASTER_FUNCTION,
        "eligible": False,
        "reason": "mode_not_color" if mode != "color" else "unsupported_color_mode",
    }
    if mode == "color":
        match_mode = rp.get("match_mode", "none")
        mt_reason = None
        if color_mode in ("solve_score", "saved_palette", "constant"):
            mt_reason = color_mode
        elif color_mode == "rainbow" and match_mode == "none":
            mt_reason = "rainbow_match_none"
        elif color_mode == "rainbow":
            raster["reason"] = f"rainbow_match_{match_mode}"
        elif color_mode == "proximity":
            raster["reason"] = "proximity_single_thread_only"
        if mt_reason:
            raster["eligible"] = True
            raster["reason"] = mt_reason
            if requested_raster_engine == "mt":
                raster["engine"] = "mt"
                raster["threads"] = requested_raster_threads
                raster["input_mode"] = rp.get("raster_input_mode", "tmpfile")
                raster["sectioned_retries"] = rp.get("raster_sectioned_retries", 2)
                raster["function_name"] = RASTER_MT_FUNCTION
        elif requested_raster_engine == "mt" and raster["reason"]:
            raster["reason"] = f"mt_requested_but_{raster['reason']}"

    # Immutable artifact outputs
    artifact_family = "coeffs" if mode == "coeff_bilevel" else mode
    artifact_id = f"{artifact_family}_{run_id}"
    artifact_prefix = f"renders/{job_id}/{artifact_family}/{artifact_id}/"
    created_at = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

    associated_palette = {
        "enabled": False,
        "mode": "none",
        "palette_id": "",
        "display_name": "",
        "image_key": "",
        "preview_key": "",
        "meta_key": "",
        "chunks_prefix": "",
        "chunk_scores_prefix": "",
        "chunk_bins_prefix": "",
        "chunk_meta_prefix": "",
        "metric": "",
        "palette": "",
        "quantile": None,
        "omega": None,
        "omega_enabled": True,
        "score_chain": "",
    }
    if mode == "color" and rp["save_associated_palette"]:
        if color_mode == "saved_palette":
            associated_palette = {
                "enabled": True,
                "mode": "dependency",
                "palette_id": saved_palette["palette_id"],
                "display_name": saved_palette_meta.get("display_name", saved_palette["display_name"]),
                "image_key": saved_palette_meta.get("image_key", f"renders/{job_id}/palettes/{saved_palette['palette_id']}/image.jpeg"),
                "preview_key": saved_palette_meta.get("preview_key", f"renders/{job_id}/palettes/{saved_palette['palette_id']}/preview.png"),
                "meta_key": f"renders/{job_id}/palettes/{saved_palette['palette_id']}/meta.json",
                "chunks_prefix": "",
                "chunk_scores_prefix": "",
                "chunk_bins_prefix": saved_palette["chunk_bins_prefix"],
                "chunk_meta_prefix": "",
                "metric": saved_palette["metric"],
                "palette": saved_palette["palette"],
                "quantile": saved_palette["quantile"],
                "omega": saved_palette["omega"],
                "omega_enabled": saved_palette["omega_enabled"],
                "score_chain": saved_palette["score_chain"],
            }
        elif solve_score_enabled:
            if not full_n:
                raise RuntimeError("save_associated_palette requires calc N/n1 for solve_score renders")
            missing_step_items = [item["chunk_idx"] for item in chunk_items if "step_start" not in item or "step_count" not in item]
            if missing_step_items:
                raise RuntimeError(
                    "save_associated_palette requires chunk step metadata; "
                    f"missing step_count/bin_size for chunks {missing_step_items[:8]}"
                )
            assoc_palette_id = f"pal_{artifact_id}"
            assoc_prefix = f"renders/{job_id}/palettes/{assoc_palette_id}/"
            assoc_chunks_prefix = assoc_prefix + "chunks/"
            associated_palette = {
                "enabled": True,
                "mode": "generated",
                "palette_id": assoc_palette_id,
                "display_name": _associated_palette_display_name(
                    solve_metric,
                    palette,
                    solve_score_quantile,
                    solve_score_omega,
                    solve_score_omega_enabled,
                ),
                "image_key": assoc_prefix + "image.jpeg",
                "preview_key": assoc_prefix + "preview.png",
                "meta_key": assoc_prefix + "meta.json",
                "chunks_prefix": assoc_chunks_prefix,
                "chunk_scores_prefix": assoc_chunks_prefix + "score_chunk_",
                "chunk_bins_prefix": assoc_chunks_prefix + "palette_bins_chunk_",
                "chunk_meta_prefix": assoc_chunks_prefix + "meta_chunk_",
                "metric": solve_metric,
                "palette": palette,
                "quantile": solve_score_quantile,
                "omega": solve_score_omega,
                "omega_enabled": solve_score_omega_enabled,
                "score_chain": solve_score_chain,
                "chunk_threads": rp["palette_chunk_threads"],
                "chunk_input_mode": rp["palette_chunk_input_mode"],
                "chunk_retries": rp["palette_chunk_retries"],
                "chunk_workers": rp["palette_chunk_workers"],
            }

    artifact_meta = {
        "artifact_id": artifact_id,
        "family": artifact_family,
        "created_at": created_at,
        "degree": str(degree),
        "pix": str(pix),
        "tile_size": str(tile_size),
        "view_mode": str(rp.get("view_mode", "auto")),
        "quantile": str(rp.get("quantile", 0.0)),
        "shim": str(rp.get("shim", 0.05)),
        "square_extent": str(rp.get("square_extent", 2.0)),
        "rotation": str(rp.get("rotation", 0.0)),
        "root_transforms": json.dumps(rp.get("root_transforms", [])),
    }

    fmt = rp.get("fmt", "jpeg")
    ext = "png" if fmt == "png" else "jpeg"
    outputs = {
        "family": artifact_family,
        "artifact_id": artifact_id,
        "artifact_prefix": artifact_prefix,
        "created_at": created_at,
        "image_key": artifact_prefix + (f"image.{ext}" if mode == "color" else "image.tif"),
        "preview_key": artifact_prefix + "preview.png",
        "bilevel_key": artifact_prefix + "image.tif",
        "coeff_bilevel_key": artifact_prefix + "image.tif",
        "metadata": artifact_meta,
        "repalette_capable": color_repalette_capable if mode == "color" else False,
    }
    if mode == "color":
        outputs["metadata"].update({
            "format": ext,
            "quality": str(rp.get("quality", 90)),
            "color_mode": rp.get("color_mode", "rainbow"),
            "match_mode": rp.get("match_mode", "none"),
            "palette": palette,
            "constant_color": rp.get("constant_color", "ffffff"),
            "background_color": DEFAULT_BACKGROUND_COLOR,
            "background_threshold": str(DEFAULT_BACKGROUND_THRESHOLD),
            "repalette_capable": "true" if color_repalette_capable else "false",
            "pixel_bins_prefix": artifact_prefix + "pixel_bins/tile_" if color_repalette_capable else "",
            "pixel_bins_empty": "255" if color_repalette_capable else "",
            "pixel_bins_layout": "tile_u8_v1" if color_repalette_capable else "",
        })
        if solve_score_enabled:
            outputs["metadata"].update(
                emit_solve_score_metadata(
                    "solve",
                    metric=solve_metric,
                    quantile=solve_score_quantile,
                    omega=solve_score_omega,
                    omega_enabled=solve_score_omega_enabled,
                    chain=solve_score_chain,
                )
            )
        if associated_palette["enabled"]:
            outputs["metadata"].update({
                "associated_palette_mode": associated_palette["mode"],
                "associated_palette_id": associated_palette["palette_id"],
                "associated_palette_display_name": associated_palette["display_name"],
                "associated_palette_image_key": associated_palette["image_key"],
                "associated_palette_preview_key": associated_palette["preview_key"],
                "associated_palette_palette": str(associated_palette["palette"]),
            })
            outputs["metadata"].update(
                emit_solve_score_metadata(
                    "associated_palette",
                    metric=associated_palette["metric"],
                    quantile=associated_palette["quantile"],
                    omega=associated_palette["omega"],
                    omega_enabled=associated_palette["omega_enabled"],
                    chain=associated_palette["score_chain"],
                )
            )
        if color_mode == "saved_palette":
            outputs["metadata"].update({
                "palette_source_id": saved_palette["palette_id"],
                "palette_source_display_name": saved_palette["display_name"],
                "palette_source_palette": str(saved_palette["palette"]),
            })
            outputs["metadata"].update(
                emit_solve_score_metadata(
                    "solve",
                    metric=solve_metric,
                    quantile=solve_score_quantile,
                    omega=solve_score_omega,
                    omega_enabled=solve_score_omega_enabled,
                    chain=solve_score_chain,
                )
            )
            outputs["metadata"].update(
                emit_solve_score_metadata(
                    "palette_source",
                    metric=saved_palette["metric"],
                    quantile=saved_palette["quantile"],
                    omega=saved_palette["omega"],
                    omega_enabled=saved_palette["omega_enabled"],
                    chain=saved_palette["score_chain"],
                )
            )
    elif mode == "bilevel":
        outputs["metadata"].update({
            "format": "tif",
            "mode": "bilevel",
        })
    elif mode == "coeff_bilevel":
        outputs["metadata"].update({
            "format": "tif",
            "mode": "coeffs",
        })

    plan = {
        "job_id": job_id,
        "run_id": run_id,
        "task_id": task_id,
        "mode": mode,
        "params": rp,
        "viewport": viewport,
        "calc": {
            "degree": degree,
            "N": full_n,
            "times": times,
            "n_chunks": n_chunks,
            "lores_bin_key": calc.get("lores", {}).get("bin_key", ""),
            "coeffs_keys": calc.get("coeffs_keys", []),
            "n_coeffs": calc.get("n_coeffs", degree + 1),
        },
        "grid": {
            "pix": pix,
            "tile_size": tile_size,
            "n_tile_cols": n_tile_cols,
            "n_tile_rows": n_tile_rows,
            "n_tiles": n_tiles,
            "tile_keys": tile_keys,
            "pixel_bin_tile_keys": [
                artifact_prefix + f"pixel_bins/tile_{t:04d}.bin" for t in range(n_tiles)
            ] if color_repalette_capable else ["" for _ in range(n_tiles)],
        },
        "chunk_items": chunk_items,
        "tile_items": tile_items,
        "solve_score": solve_score,
        "finalize": finalize,
        "raster": raster,
        "saved_palette": saved_palette,
        "associated_palette": associated_palette,
        "outputs": outputs,
    }

    # Compactness check
    plan_json = json.dumps(plan)
    if len(plan_json) > MAX_PLAN_BYTES:
        raise RuntimeError(
            f"Plan too large: {len(plan_json)} bytes > {MAX_PLAN_BYTES} limit. "
            f"Reduce tile count or stripe count."
        )

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
    pix = rp["pix"]
    if rp.get("view_mode") == "square":
        ext = rp.get("square_extent", 2.0)
        return {"center_re": 0, "center_im": 0, "scale": pix / (2 * ext)}
    else:
        vp = _invoke_sync(VIEWPORT_FUNCTION, {
            "job_id": job_id,
            "quantile": rp.get("quantile", 0.0),
            "shim": rp.get("shim", 0.05),
        })
        ref_size = 4096
        vp["scale"] = vp.get("scale_ref", vp.get("scale", 256)) * pix / ref_size
        return {
            "center_re": vp.get("center_re", 0),
            "center_im": vp.get("center_im", 0),
            "scale": vp["scale"],
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
