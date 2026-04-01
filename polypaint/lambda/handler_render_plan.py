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

s3 = boto3.client("s3")
lambda_client = boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1"))

VIEWPORT_FUNCTION = os.environ.get("VIEWPORT_FUNCTION", "polypaint-viewport")
STORAGE_FUNCTION = os.environ.get("STORAGE_FUNCTION", "polypaint-storage")

MAX_PLAN_BYTES = 200 * 1024  # 200 KB — fail fast before hitting 256 KB SFN limit


def _validate_omega(value):
    try:
        omega = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"solve_score_omega must be numeric, got {value!r}")
    if not (1.0 <= omega <= 10.0):
        raise RuntimeError(f"solve_score_omega must be in [1, 10], got {omega}")
    return omega


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
    n_chunks = calc.get("n_chunks", calc.get("n_stripes", 10))
    degree = calc.get("degree", 1)

    # Grid computation
    pix = rp["pix"]
    tile_size = rp.get("tile_size", 2048 if mode == "color" else 4096)
    n_tile_cols = math.ceil(pix / tile_size)
    n_tile_rows = math.ceil(pix / tile_size)
    n_tiles = n_tile_cols * n_tile_rows

    # Precompute tile keys for encode
    tile_keys = [f"renders/{job_id}/tile_{t:04d}.raw" for t in range(n_tiles)]

    # Compact chunk items
    chunk_items = [{"chunk_idx": c} for c in range(n_chunks)]

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
        "match_mode": "none",
        "quality": 90,
        "fmt": "jpeg",
        "color_mode": "rainbow",
        "solve_metric": "proximity",
        "solve_score_quantile": 0.001,
        "solve_score_omega": 1.0,
    }
    for key, default in _PARAM_DEFAULTS.items():
        if key not in rp:
            rp[key] = default

    # Normalize solve-score params
    color_mode = rp.get("color_mode", "rainbow")
    if color_mode == "solve_proximity":
        color_mode = "solve_score"
        rp["color_mode"] = "solve_score"
        if not rp.get("solve_metric"):
            rp["solve_metric"] = "proximity"

    solve_metric = rp.get("solve_metric", "proximity")
    solve_score_quantile = rp.get("solve_score_quantile", 0.001)
    solve_score_omega = rp.get("solve_score_omega", 1.0)
    palette = rp.get("palette", "inferno")
    if palette not in VALID_PALETTE_NAMES:
        raise RuntimeError(f"Invalid palette: {palette}")

    solve_score_enabled = color_mode == "solve_score"
    if solve_score_enabled:
        try:
            solve_score_quantile = float(solve_score_quantile)
        except (TypeError, ValueError):
            raise RuntimeError(f"solve_score_quantile must be numeric, got {solve_score_quantile!r}")
        if not (0.001 <= solve_score_quantile <= 0.05):
            raise RuntimeError(f"solve_score_quantile must be in [0.001, 0.05], got {solve_score_quantile}")
        solve_score_omega = _validate_omega(solve_score_omega)
    else:
        solve_score_omega = _validate_omega(solve_score_omega)
    rp["solve_score_omega"] = solve_score_omega

    solve_score = {
        "enabled": solve_score_enabled,
        "metric": solve_metric,
        "quantile": solve_score_quantile,
        "omega": solve_score_omega,
        "clip_key": f"renders/{job_id}/solve_scores/{solve_metric}_clip.json",
        "hist_prefix": f"renders/{job_id}/solve_scores/{solve_metric}/",
        "bins_key": f"renders/{job_id}/solve_scores/{solve_metric}_bins.json",
    }

    # Immutable artifact outputs
    artifact_family = "coeffs" if mode == "coeff_bilevel" else mode
    artifact_id = f"{artifact_family}_{run_id}"
    artifact_prefix = f"renders/{job_id}/{artifact_family}/{artifact_id}/"
    created_at = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

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
    }
    if mode == "color":
        outputs["metadata"].update({
            "format": ext,
            "color_mode": rp.get("color_mode", "rainbow"),
            "match_mode": rp.get("match_mode", "none"),
            "palette": palette,
            "constant_color": rp.get("constant_color", "ffffff"),
            "solve_metric": solve_metric if solve_score_enabled else "",
            "solve_score_quantile": str(solve_score_quantile if solve_score_enabled else ""),
            "solve_score_omega": str(solve_score_omega if solve_score_enabled else ""),
        })
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
        },
        "chunk_items": chunk_items,
        "tile_items": tile_items,
        "solve_score": solve_score,
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
