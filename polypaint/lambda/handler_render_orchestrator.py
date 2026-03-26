"""
Render orchestrator Lambda — owns the full multi-phase render pipeline.

The browser dispatches one job to this Lambda. It runs all phases
(clean → viewport → raster → finalize → encode) server-side,
polling DDB for completion between phases. If close to Lambda timeout,
it checkpoints and self-reinvokes.

Modes: color, bilevel, coeff_bilevel.

The browser only observes the orchestrator's DDB task row for progress.
"""
import json
import os
import time

import boto3

from shared import BUCKET, JOBS_TABLE, parse_body, ok_response, report_status

s3 = boto3.client("s3")
lambda_client = boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1"))
ddb = boto3.client("dynamodb", region_name=os.environ.get("AWS_REGION", "us-east-1"))

# Dispatch targets → function names from env
FUNCTIONS = {
    "raster": os.environ.get("RASTER_FUNCTION", "polypaint-raster"),
    "finalize": os.environ.get("FINALIZE_FUNCTION", "polypaint-finalize"),
    "encode": os.environ.get("ENCODE_FUNCTION", "polypaint-encode"),
    "viewport": os.environ.get("VIEWPORT_FUNCTION", "polypaint-viewport"),
    "storage": os.environ.get("STORAGE_FUNCTION", "polypaint-storage"),
    "bilevel": os.environ.get("BILEVEL_FUNCTION", "polypaint-bilevel"),
    "bilevel_stitch": os.environ.get("BILEVEL_STITCH_FUNCTION", "polypaint-bilevel-stitch"),
    "solve_proximity": os.environ.get("SOLVE_PROXIMITY_FUNCTION", "polypaint-solve-proximity"),
}

SELF_FUNCTION = os.environ.get("RENDER_ORCHESTRATOR_FUNCTION", "polypaint-render-orchestrator")
REINVOKE_THRESHOLD_MS = 60000
POLL_INTERVAL = 3


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    run_id = params["run_id"]
    mode = params["mode"]
    render_params = params.get("params", {})
    task_id = f"render_run_{mode}_{run_id}"

    # Resume from checkpoint if present
    checkpoint = params.get("_checkpoint", {})
    phase = checkpoint.get("phase", "clean")

    progress = {
        "job_id": job_id,
        "run_id": run_id,
        "mode": mode,
        "phase": phase,
        "phase_label": phase,
        "started_at_ms": checkpoint.get("started_at_ms", int(time.time() * 1000)),
    }

    try:
        if mode == "color":
            run_color(params, render_params, task_id, progress, checkpoint, context)
        elif mode == "bilevel":
            run_bilevel(params, render_params, task_id, progress, checkpoint, context)
        elif mode == "coeff_bilevel":
            run_coeff_bilevel(params, render_params, task_id, progress, checkpoint, context)
        else:
            raise RuntimeError(f"Unknown orchestrator mode: {mode}")

        progress["phase"] = "done"
        progress["phase_label"] = "Done"
        progress["updated_at_ms"] = int(time.time() * 1000)
        report_status(job_id, task_id, "done", result_data=progress)
        return ok_response(progress)

    except _SelfReinvoke:
        return ok_response({"status": "reinvoked", "phase": progress.get("phase")})

    except Exception as e:
        progress["error"] = str(e)[:500]
        progress["updated_at_ms"] = int(time.time() * 1000)
        report_status(job_id, task_id, "error", str(e)[:500], result_data=progress)
        raise


class _SelfReinvoke(Exception):
    pass


# ── Helpers ──────────────────────────────────────────────────────────────

def _update_progress(task_id, progress, phase, phase_label, context, **extra):
    progress["phase"] = phase
    progress["phase_label"] = phase_label
    progress["updated_at_ms"] = int(time.time() * 1000)
    progress.update(extra)
    report_status(progress["job_id"], task_id, phase, result_data=progress)
    _check_timeout(progress, task_id, context)


def _check_timeout(progress, task_id, context):
    if context and hasattr(context, "get_remaining_time_in_millis"):
        remaining = context.get_remaining_time_in_millis()
        if remaining < REINVOKE_THRESHOLD_MS:
            # Checkpoint and self-reinvoke
            progress["updated_at_ms"] = int(time.time() * 1000)
            report_status(progress["job_id"], task_id, progress["phase"],
                          result_data={**progress, "_reinvoking": True})
            payload = {
                "job_id": progress["job_id"],
                "run_id": progress["run_id"],
                "mode": progress["mode"],
                "params": progress.get("_params", {}),
                "_checkpoint": progress,
            }
            lambda_client.invoke(
                FunctionName=SELF_FUNCTION,
                InvocationType="Event",
                Payload=json.dumps(payload).encode(),
            )
            raise _SelfReinvoke()


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


def _dispatch_async(function_name, payload):
    """Invoke a Lambda asynchronously (fire and forget)."""
    lambda_client.invoke(
        FunctionName=function_name,
        InvocationType="Event",
        Payload=json.dumps(payload).encode(),
    )


def _dispatch_single(function_name, payload, progress):
    """Dispatch one job and store in progress for checkpoint-safe re-dispatch."""
    _dispatch_async(function_name, payload)
    progress["_last_dispatched_jobs"] = [payload]
    progress["_last_dispatch_function"] = function_name


def _dispatch_batch(function_name, jobs, progress, batch_size=50):
    """Dispatch jobs in batches. Store in progress for checkpoint-safe re-dispatch."""
    for i in range(0, len(jobs), batch_size):
        batch = jobs[i:i + batch_size]
        for job in batch:
            _dispatch_async(function_name, job)
    # Store for re-dispatch after checkpoint resume
    progress["_last_dispatched_jobs"] = jobs
    progress["_last_dispatch_function"] = function_name


STALL_GRACE_S = 45
MAX_REDISPATCH = 2


def _poll_completion(job_id, task_prefix, expected, task_id, progress, context):
    """Poll DDB until all expected tasks complete or error. Re-dispatch on stall."""
    # Re-dispatch info from progress (survives checkpoint)
    jobs = progress.get("_last_dispatched_jobs")
    function_name = progress.get("_last_dispatch_function")
    last_done = 0
    last_progress_time = time.time()
    redispatch_count = 0

    while True:
        time.sleep(POLL_INTERVAL)
        resp = ddb.query(
            TableName=JOBS_TABLE,
            KeyConditionExpression="job_id = :jid AND begins_with(task_id, :pfx)",
            ExpressionAttributeValues={
                ":jid": {"S": job_id},
                ":pfx": {"S": task_prefix},
            },
            ProjectionExpression="task_id, task_status, error_msg",
        )
        done = 0
        errors = []
        status_counts = {}
        found_ids = set()
        for item in resp.get("Items", []):
            status = item["task_status"]["S"]
            status_counts[status] = status_counts.get(status, 0) + 1
            found_ids.add(item["task_id"]["S"])
            if status == "done":
                done += 1
            elif status == "error":
                errors.append(item.get("error_msg", {}).get("S", "unknown"))

        if errors:
            raise RuntimeError(f"{len(errors)} tasks failed: {errors[0][:200]}")

        if done != last_done:
            last_done = done
            last_progress_time = time.time()

        progress["done"] = done
        progress["expected"] = expected
        progress["status_counts"] = status_counts
        progress["updated_at_ms"] = int(time.time() * 1000)
        report_status(job_id, task_id, progress["phase"], result_data=progress)

        if done >= expected:
            return

        # Stall detection: re-dispatch missing tasks
        stall_s = time.time() - last_progress_time
        if stall_s > STALL_GRACE_S and redispatch_count < MAX_REDISPATCH and jobs and function_name:
            missing_jobs = [j for j in jobs if j.get("task_id") and j["task_id"] not in found_ids]
            if missing_jobs:
                redispatch_count += 1
                for j in missing_jobs:
                    _dispatch_async(function_name, j)
                last_progress_time = time.time()

        _check_timeout(progress, task_id, context)


def _storage_call(path, body):
    """Call the storage Lambda synchronously."""
    return _invoke_sync(FUNCTIONS["storage"], {"body": json.dumps(body), "path": path,
                        "requestContext": {"http": {"method": "POST", "path": path}}})


# ── Color Pipeline ───────────────────────────────────────────────────────

def run_color(params, rp, task_id, progress, checkpoint, context):
    job_id = params["job_id"]
    run_id = params["run_id"]
    progress["_params"] = rp
    phase = checkpoint.get("phase", "clean")

    # Phase: clean
    if phase == "clean":
        _update_progress(task_id, progress, "clean", "Cleaning", context)
        _storage_call("/clean-render", {"job_id": job_id, "pipeline": "color"})
        phase = "viewport"

    # Phase: viewport
    if phase == "viewport":
        _update_progress(task_id, progress, "viewport", "Viewport", context)
        if rp.get("view_mode") == "square":
            ext = rp.get("square_extent", 2.0)
            vp = {"center_re": 0, "center_im": 0, "scale": rp["pix"] / (2 * ext)}
        else:
            vp = _invoke_sync(FUNCTIONS["viewport"], {
                "job_id": job_id,
                "quantile": rp.get("quantile", 0.0),
                "shim": rp.get("shim", 0.05),
            })
            ref_size = 4096
            vp["scale"] = vp.get("scale_ref", vp.get("scale", 256)) * rp["pix"] / ref_size
        progress["_viewport"] = vp
        phase = "calc_meta"

    vp = progress.get("_viewport", checkpoint.get("_viewport", {}))

    # Phase: calc_meta
    if phase == "calc_meta":
        _update_progress(task_id, progress, "calc_meta", "Loading metadata", context)
        detail = _storage_call("/detail", {"job_id": job_id})
        calc = detail.get("calc", {})
        if not calc:
            raise RuntimeError(f"calc.json missing for {job_id}")
        progress["_calc"] = calc
        progress["n_stripes"] = calc.get("n_stripes", calc.get("n_chunks", 10))
        progress["degree"] = calc.get("degree", 1)
        phase = "solve_proximity_check"

    calc = progress.get("_calc", checkpoint.get("_calc", {}))
    n_stripes = progress.get("n_stripes", checkpoint.get("n_stripes", 10))
    degree = progress.get("degree", checkpoint.get("degree", 1))

    # Phase: solve_proximity prepass (optional)
    solve_prox_bins_key = checkpoint.get("solve_prox_bins_key")
    if phase == "solve_proximity_check":
        if rp.get("color_mode") == "solve_proximity":
            phase = "solve_proximity_clip"
        else:
            phase = "raster_dispatch"

    if phase == "solve_proximity_clip":
        _update_progress(task_id, progress, "solve_proximity_clip", "Solve proximity: clip", context)
        clip_key = f"renders/{job_id}/solve_proximity_clip.json"
        clip_task = f"render_{run_id}_solve_proximity_clip"
        lores_key = calc.get("lores", {}).get("bin_key")
        if not lores_key:
            raise RuntimeError("lores.bin_key missing — needed for solve proximity")
        rt = rp.get("root_transforms") or None
        _dispatch_single(FUNCTIONS["solve_proximity"], {
            "phase": "clip", "job_id": job_id, "degree": degree,
            "lores_bin_key": lores_key, "root_transforms": rt,
            "out_key": clip_key, "task_id": clip_task,
        }, progress)
        _poll_completion(job_id, clip_task, 1, task_id, progress, context)
        progress["_clip_key"] = clip_key
        phase = "solve_proximity_hist"

    clip_key = progress.get("_clip_key", checkpoint.get("_clip_key"))

    if phase == "solve_proximity_hist":
        _update_progress(task_id, progress, "solve_proximity_hist", "Solve proximity: hist", context)
        hist_prefix = f"renders/{job_id}/solve_proximity/"
        rt = rp.get("root_transforms") or None
        hist_jobs = []
        for s in range(n_stripes):
            hist_jobs.append({
                "phase": "hist", "job_id": job_id, "stripe_idx": s,
                "bin_key": f"renders/{job_id}/stripe_{s}.bin", "degree": degree,
                "clip_key": clip_key, "hist_bins": 100, "root_transforms": rt,
                "out_key": f"{hist_prefix}stripe_{s}_hist.json",
                "task_id": f"render_{run_id}_solve_proximity_hist_{s}",
            })
        _dispatch_batch(FUNCTIONS["solve_proximity"], hist_jobs, progress)
        _poll_completion(job_id, f"render_{run_id}_solve_proximity_hist_",
                         n_stripes, task_id, progress, context)
        phase = "solve_proximity_merge"

    if phase == "solve_proximity_merge":
        _update_progress(task_id, progress, "solve_proximity_merge", "Solve proximity: merge", context)
        bins_key = f"renders/{job_id}/solve_proximity_bins.json"
        merge_task = f"render_{run_id}_solve_proximity_merge"
        _dispatch_single(FUNCTIONS["solve_proximity"], {
            "phase": "merge", "job_id": job_id, "n_stripes": n_stripes,
            "hist_prefix": f"renders/{job_id}/solve_proximity/",
            "clip_key": clip_key, "out_key": bins_key, "task_id": merge_task,
        }, progress)
        _poll_completion(job_id, merge_task, 1, task_id, progress, context)
        solve_prox_bins_key = bins_key
        progress["solve_prox_bins_key"] = bins_key
        phase = "raster_dispatch"

    # Phase: raster
    pix = rp["pix"]
    tile_size = rp.get("tile_size", 2048)
    import math
    n_tile_cols = math.ceil(pix / tile_size)
    n_tile_rows = math.ceil(pix / tile_size)
    n_tiles = n_tile_cols * n_tile_rows
    progress["n_tiles"] = n_tiles

    if phase == "raster_dispatch":
        _update_progress(task_id, progress, "raster_dispatch", "Raster: dispatching", context)
        raster_jobs = []
        for s in range(n_stripes):
            job = {
                "job_id": job_id, "stripe_idx": s,
                "task_id": f"render_{run_id}_raster_{s}",
                "bin_key": f"renders/{job_id}/stripe_{s}.bin",
                "width": pix, "height": pix, "tile_size": tile_size,
                "n_tile_cols": n_tile_cols, "n_tile_rows": n_tile_rows,
                "center_re": vp.get("center_re", 0), "center_im": vp.get("center_im", 0),
                "scale": vp.get("scale", 256), "rotation": rp.get("rotation", 0),
                "degree": degree,
                "color": rp.get("color_mode", "rainbow"),
                "match": rp.get("match_mode", "none"),
                "palette": rp.get("palette", "inferno"),
                "constant_color": rp.get("constant_color", "ffffff"),
            }
            if rp.get("root_transforms"):
                job["root_transforms"] = rp["root_transforms"]
            if solve_prox_bins_key:
                job["solve_proximity_bins_key"] = solve_prox_bins_key
            raster_jobs.append(job)
        _dispatch_batch(FUNCTIONS["raster"], raster_jobs, progress)
        phase = "raster_poll"

    if phase == "raster_poll":
        _update_progress(task_id, progress, "raster_poll", "Raster", context)
        _poll_completion(job_id, f"render_{run_id}_raster_",
                         n_stripes, task_id, progress, context)
        phase = "finalize_dispatch"

    # Phase: finalize
    if phase == "finalize_dispatch":
        _update_progress(task_id, progress, "finalize_dispatch", "Finalize: dispatching", context)
        finalize_jobs = []
        for t in range(n_tiles):
            t_row = t // n_tile_cols
            t_col = t % n_tile_cols
            tw = min(tile_size, pix - t_col * tile_size)
            th = min(tile_size, pix - t_row * tile_size)
            finalize_jobs.append({
                "job_id": job_id, "tile_idx": t,
                "task_id": f"render_{run_id}_tile_{t}",
                "tile_row": t_row, "tile_col": t_col,
                "n_stripes": n_stripes, "tile_w": tw, "tile_h": th,
                "n_tile_cols": n_tile_cols, "n_tile_rows": n_tile_rows,
            })
        _dispatch_batch(FUNCTIONS["finalize"], finalize_jobs, progress)
        phase = "finalize_poll"

    if phase == "finalize_poll":
        _update_progress(task_id, progress, "finalize_poll", "Finalize", context)
        _poll_completion(job_id, f"render_{run_id}_tile_",
                         n_tiles, task_id, progress, context)
        phase = "encode_dispatch"

    # Phase: encode
    if phase == "encode_dispatch":
        _update_progress(task_id, progress, "encode_dispatch", "Encode: dispatching", context)
        ext = "png" if rp.get("fmt") == "png" else "jpeg"
        out_key = f"renders/{job_id}/image.{ext}"
        tile_keys = []
        for t in range(n_tiles):
            tile_keys.append(f"renders/{job_id}/tile_{t:04d}.raw")
        _dispatch_single(FUNCTIONS["encode"], {
            "job_id": job_id,
            "task_id": f"render_{run_id}_encode",
            "out_key": out_key,
            "format": ext,
            "quality": rp.get("quality", 90),
            "width": pix, "height": pix,
            "tile_grid": {
                "n_cols": n_tile_cols, "n_rows": n_tile_rows,
                "tile_keys": tile_keys,
            },
        }, progress)
        progress["image_key"] = out_key
        phase = "encode_poll"

    if phase == "encode_poll":
        _update_progress(task_id, progress, "encode_poll", "Encode", context)
        _poll_completion(job_id, f"render_{run_id}_encode",
                         1, task_id, progress, context)
        phase = "cleanup"

    # Phase: cleanup intermediates
    if phase == "cleanup":
        _update_progress(task_id, progress, "cleanup", "Cleanup", context)
        # Intermediates are cleaned by the next render launch, not here
        phase = "done"


# ── Bilevel Pipeline ─────────────────────────────────────────────────────

def run_bilevel(params, rp, task_id, progress, checkpoint, context):
    job_id = params["job_id"]
    run_id = params["run_id"]
    progress["_params"] = rp
    phase = checkpoint.get("phase", "clean")

    if phase == "clean":
        _update_progress(task_id, progress, "clean", "Cleaning", context)
        _storage_call("/clean-render", {"job_id": job_id, "pipeline": "bilevel"})
        phase = "viewport"

    if phase == "viewport":
        _update_progress(task_id, progress, "viewport", "Viewport", context)
        if rp.get("view_mode") == "square":
            ext = rp.get("square_extent", 2.0)
            vp = {"center_re": 0, "center_im": 0, "scale": rp["pix"] / (2 * ext)}
        else:
            vp = _invoke_sync(FUNCTIONS["viewport"], {
                "job_id": job_id,
                "quantile": rp.get("quantile", 0.0),
                "shim": rp.get("shim", 0.05),
            })
            ref_size = 4096
            vp["scale"] = vp.get("scale_ref", vp.get("scale", 256)) * rp["pix"] / ref_size
        progress["_viewport"] = vp
        phase = "calc_meta"

    vp = progress.get("_viewport", checkpoint.get("_viewport", {}))

    if phase == "calc_meta":
        _update_progress(task_id, progress, "calc_meta", "Loading metadata", context)
        detail = _storage_call("/detail", {"job_id": job_id})
        calc = detail.get("calc", {})
        if not calc:
            raise RuntimeError(f"calc.json missing for {job_id}")
        progress["_calc"] = calc
        progress["n_stripes"] = calc.get("n_stripes", calc.get("n_chunks", 10))
        progress["degree"] = calc.get("degree", 1)
        phase = "bilevel_raster_dispatch"

    calc = progress.get("_calc", checkpoint.get("_calc", {}))
    n_stripes = progress.get("n_stripes", checkpoint.get("n_stripes", 10))
    degree = progress.get("degree", checkpoint.get("degree", 1))
    pix = rp["pix"]
    tile_size = rp.get("tile_size", 4096)
    import math
    n_tile_cols = math.ceil(pix / tile_size)
    n_tile_rows = math.ceil(pix / tile_size)
    n_tiles = n_tile_cols * n_tile_rows
    progress["n_tiles"] = n_tiles

    if phase == "bilevel_raster_dispatch":
        _update_progress(task_id, progress, "bilevel_raster_dispatch", "BiLevel raster: dispatching", context)
        raster_jobs = []
        for s in range(n_stripes):
            job = {
                "phase": "raster", "job_id": job_id, "stripe_idx": s,
                "task_id": f"render_{run_id}_bilevel_raster_{s}",
                "bin_key": f"renders/{job_id}/stripe_{s}.bin",
                "width": pix, "height": pix, "tile_size": tile_size,
                "n_tile_cols": n_tile_cols, "n_tile_rows": n_tile_rows,
                "center_re": vp.get("center_re", 0), "center_im": vp.get("center_im", 0),
                "scale": vp.get("scale", 256), "rotation": rp.get("rotation", 0),
                "degree": degree,
            }
            if rp.get("root_transforms"):
                job["root_transforms"] = rp["root_transforms"]
            raster_jobs.append(job)
        _dispatch_batch(FUNCTIONS["bilevel"], raster_jobs, progress)
        phase = "bilevel_raster_poll"

    if phase == "bilevel_raster_poll":
        _update_progress(task_id, progress, "bilevel_raster_poll", "BiLevel raster", context)
        _poll_completion(job_id, f"render_{run_id}_bilevel_raster_",
                         n_stripes, task_id, progress, context)
        phase = "bilevel_merge_dispatch"

    if phase == "bilevel_merge_dispatch":
        _update_progress(task_id, progress, "bilevel_merge_dispatch", "BiLevel merge: dispatching", context)
        merge_jobs = []
        for t in range(n_tiles):
            t_row = t // n_tile_cols
            t_col = t % n_tile_cols
            merge_jobs.append({
                "phase": "merge", "job_id": job_id, "tile_idx": t,
                "task_id": f"render_{run_id}_bilevel_merge_{t}",
                "tile_row": t_row, "tile_col": t_col,
                "n_stripes": n_stripes, "n_tile_cols": n_tile_cols, "n_tile_rows": n_tile_rows,
                "width": pix, "height": pix, "tile_size": tile_size,
            })
        _dispatch_batch(FUNCTIONS["bilevel"], merge_jobs, progress)
        phase = "bilevel_merge_poll"

    if phase == "bilevel_merge_poll":
        _update_progress(task_id, progress, "bilevel_merge_poll", "BiLevel merge", context)
        _poll_completion(job_id, f"render_{run_id}_bilevel_merge_",
                         n_tiles, task_id, progress, context)
        phase = "bilevel_stitch_dispatch"

    if phase == "bilevel_stitch_dispatch":
        _update_progress(task_id, progress, "bilevel_stitch_dispatch", "BiLevel stitch", context)
        out_key = f"renders/{job_id}/image_bilevel.tif"
        _dispatch_single(FUNCTIONS["bilevel_stitch"], {
            "job_id": job_id,
            "task_id": f"render_{run_id}_bilevel_stitch",
            "n_tile_cols": n_tile_cols, "n_tile_rows": n_tile_rows,
            "width": pix, "height": pix, "tile_size": tile_size,
            "out_key": out_key,
        }, progress)
        progress["image_key"] = out_key
        phase = "bilevel_stitch_poll"

    if phase == "bilevel_stitch_poll":
        _update_progress(task_id, progress, "bilevel_stitch_poll", "BiLevel stitch", context)
        _poll_completion(job_id, f"render_{run_id}_bilevel_stitch",
                         1, task_id, progress, context)
        phase = "done"


# ── Coeff Bilevel Pipeline ───────────────────────────────────────────────

def run_coeff_bilevel(params, rp, task_id, progress, checkpoint, context):
    job_id = params["job_id"]
    run_id = params["run_id"]
    progress["_params"] = rp
    phase = checkpoint.get("phase", "clean")

    if phase == "clean":
        _update_progress(task_id, progress, "clean", "Cleaning", context)
        _storage_call("/clean-render", {"job_id": job_id, "pipeline": "coeff_bilevel"})
        phase = "viewport"

    if phase == "viewport":
        _update_progress(task_id, progress, "viewport", "Viewport", context)
        if rp.get("view_mode") == "square":
            ext = rp.get("square_extent", 2.0)
            vp = {"center_re": 0, "center_im": 0, "scale": rp["pix"] / (2 * ext)}
        else:
            vp = _invoke_sync(FUNCTIONS["viewport"], {
                "job_id": job_id,
                "quantile": rp.get("quantile", 0.0),
                "shim": rp.get("shim", 0.05),
            })
            ref_size = 4096
            vp["scale"] = vp.get("scale_ref", vp.get("scale", 256)) * rp["pix"] / ref_size
        progress["_viewport"] = vp
        phase = "calc_meta"

    vp = progress.get("_viewport", checkpoint.get("_viewport", {}))

    if phase == "calc_meta":
        _update_progress(task_id, progress, "calc_meta", "Loading metadata", context)
        detail = _storage_call("/detail", {"job_id": job_id})
        calc = detail.get("calc", {})
        if not calc:
            raise RuntimeError(f"calc.json missing for {job_id}")
        progress["_calc"] = calc
        progress["n_stripes"] = calc.get("n_stripes", calc.get("n_chunks", 10))
        progress["degree"] = calc.get("degree", 1)
        phase = "coeff_raster_dispatch"

    calc = progress.get("_calc", checkpoint.get("_calc", {}))
    n_stripes = progress.get("n_stripes", checkpoint.get("n_stripes", 10))
    degree = progress.get("degree", checkpoint.get("degree", 1))
    pix = rp["pix"]
    tile_size = rp.get("tile_size", 4096)
    import math
    n_tile_cols = math.ceil(pix / tile_size)
    n_tile_rows = math.ceil(pix / tile_size)
    n_tiles = n_tile_cols * n_tile_rows
    progress["n_tiles"] = n_tiles

    if phase == "coeff_raster_dispatch":
        _update_progress(task_id, progress, "coeff_raster_dispatch", "Coeffs raster: dispatching", context)
        raster_jobs = []
        coeffs_keys = calc.get("coeffs_keys", [])
        n_coeffs = calc.get("n_coeffs", degree + 1)
        for s in range(n_stripes):
            coeffs_key = coeffs_keys[s] if s < len(coeffs_keys) else f"renders/{job_id}/coeffs_{s:04d}.bin"
            job = {
                "phase": "coeff_raster", "job_id": job_id, "stripe_idx": s,
                "task_id": f"render_{run_id}_coeff_bilevel_raster_{s}",
                "coeffs_key": coeffs_key,
                "width": pix, "height": pix, "tile_size": tile_size,
                "n_tile_cols": n_tile_cols, "n_tile_rows": n_tile_rows,
                "center_re": vp.get("center_re", 0), "center_im": vp.get("center_im", 0),
                "scale": vp.get("scale", 256), "rotation": rp.get("rotation", 0),
                "degree": degree, "n_coeffs": n_coeffs,
            }
            if rp.get("root_transforms"):
                job["root_transforms"] = rp["root_transforms"]
            raster_jobs.append(job)
        _dispatch_batch(FUNCTIONS["bilevel"], raster_jobs, progress)
        phase = "coeff_raster_poll"

    if phase == "coeff_raster_poll":
        _update_progress(task_id, progress, "coeff_raster_poll", "Coeffs raster", context)
        _poll_completion(job_id, f"render_{run_id}_coeff_bilevel_raster_",
                         n_stripes, task_id, progress, context)
        phase = "coeff_merge_dispatch"

    if phase == "coeff_merge_dispatch":
        _update_progress(task_id, progress, "coeff_merge_dispatch", "Coeffs merge: dispatching", context)
        merge_jobs = []
        for t in range(n_tiles):
            t_row = t // n_tile_cols
            t_col = t % n_tile_cols
            merge_jobs.append({
                "phase": "merge", "job_id": job_id, "tile_idx": t,
                "task_id": f"render_{run_id}_coeff_bilevel_merge_{t}",
                "tile_row": t_row, "tile_col": t_col,
                "n_stripes": n_stripes, "n_tile_cols": n_tile_cols, "n_tile_rows": n_tile_rows,
                "width": pix, "height": pix, "tile_size": tile_size,
                "bits_prefix": "coeff_bits", "tile_prefix": "coeff",
                "task_prefix": "coeff_bilevel_merge",
            })
        _dispatch_batch(FUNCTIONS["bilevel"], merge_jobs, progress)
        phase = "coeff_merge_poll"

    if phase == "coeff_merge_poll":
        _update_progress(task_id, progress, "coeff_merge_poll", "Coeffs merge", context)
        _poll_completion(job_id, f"render_{run_id}_coeff_bilevel_merge_",
                         n_tiles, task_id, progress, context)
        phase = "coeff_stitch_dispatch"

    if phase == "coeff_stitch_dispatch":
        _update_progress(task_id, progress, "coeff_stitch_dispatch", "Coeffs stitch", context)
        out_key = f"renders/{job_id}/image_coeffs_bilevel.tif"
        _dispatch_single(FUNCTIONS["bilevel_stitch"], {
            "job_id": job_id,
            "task_id": f"render_{run_id}_coeff_bilevel_stitch",
            "n_tile_cols": n_tile_cols, "n_tile_rows": n_tile_rows,
            "width": pix, "height": pix, "tile_size": tile_size,
            "out_key": out_key,
            "tile_prefix": "coeff",
        }, progress)
        progress["image_key"] = out_key
        phase = "coeff_stitch_poll"

    if phase == "coeff_stitch_poll":
        _update_progress(task_id, progress, "coeff_stitch_poll", "Coeffs stitch", context)
        _poll_completion(job_id, f"render_{run_id}_coeff_bilevel_stitch",
                         1, task_id, progress, context)
        phase = "done"
