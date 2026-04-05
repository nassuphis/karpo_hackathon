"""
Raster MT Lambda handler — splits one chunk into solve ranges, runs roots2pix in
parallel subprocesses, then merges worker-local sparse outputs back into the
standard chunk/tile .pix and .pbx files.

This preserves the existing external contracts used by finalize/repalette while
trading exact intra-chunk winner semantics for speed.
"""
import concurrent.futures
import glob
import json
import os
import shutil
import subprocess
import time

import boto3

from shared import BUCKET, ok_response, parse_body, report_status

s3 = boto3.client("s3")
ROOTS2PIX = os.path.join(os.path.dirname(__file__), "roots2pix")
DEFAULT_THREADS = int(os.environ.get("RASTER_MT_THREADS", "2") or "2")


def _validate_threads(value):
    try:
        threads = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"raster_mt_threads must be an integer, got {value!r}")
    if threads < 1:
        raise RuntimeError(f"raster_mt_threads must be >= 1, got {threads}")
    return threads


def _parse_boolish(value, default=True):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def _cleanup_tmp():
    for pattern in (
        "/tmp/pix_t*.pix",
        "/tmp/pixbin_t*.pbx",
        "/tmp/pix_w*_t*.pix",
        "/tmp/pixbin_w*_t*.pbx",
        "/tmp/stripe_w*.bin",
        "/tmp/palette_bins_w*.bin",
        "/tmp/root_xforms.json",
    ):
        for stale in glob.glob(pattern):
            try:
                os.remove(stale)
            except OSError:
                pass


def _build_base_cmd(params, bin_path, out_prefix, saved_bins_path=None):
    cmd = [
        ROOTS2PIX,
        bin_path,
        out_prefix,
        f"--width={params['width']}",
        f"--height={params['height']}",
        f"--tile_size={params['tile_size']}",
        f"--n_tile_cols={params['n_tile_cols']}",
        f"--n_tile_rows={params['n_tile_rows']}",
        f"--center_re={params['center_re']}",
        f"--center_im={params['center_im']}",
        f"--scale={params['scale']}",
        f"--degree={params['degree']}",
        f"--color={params.get('color', 'rainbow')}",
        f"--match={params.get('match', 'none')}",
        f"--palette={params.get('palette', 'inferno')}",
        f"--constant_color={params.get('constant_color', 'ffffff')}",
        f"--rotation={params.get('rotation', 0.0)}",
    ]
    if params.get("emit_pixel_bins"):
        cmd.append(f"--pixel_bin_prefix={params['pixel_bin_prefix']}")

    if params.get("color") == "saved_palette":
        if not saved_bins_path:
            raise RuntimeError("saved_palette color mode requires worker-local bins slice")
        cmd = [a for a in cmd if not a.startswith("--color=")]
        cmd.append("--color=saved_palette")
        cmd.append(f"--solve_bins_file={saved_bins_path}")

    ss_data = params.get("solve_score_bins_data")
    color = params.get("color", "rainbow")
    if ss_data and color in ("solve_score", "solve_proximity"):
        req_metric = params.get("solve_metric", "proximity")
        if ss_data.get("family") != "solve_score":
            raise RuntimeError(f"Bins artifact missing or wrong family: {ss_data.get('family')}")
        if ss_data.get("metric") != req_metric:
            raise RuntimeError(f"Bins metric mismatch: expected {req_metric}, got {ss_data.get('metric')}")
        req_q = params.get("solve_score_quantile", 0.001)
        if "clip_quantile" not in ss_data:
            raise RuntimeError("Bins artifact missing clip_quantile")
        if ss_data["clip_quantile"] != req_q:
            raise RuntimeError(f"Bins quantile mismatch: expected {req_q}, got {ss_data['clip_quantile']}")
        req_omega = float(params.get("solve_score_omega", 1.0))
        bins_omega = float(ss_data.get("omega", 1.0))
        if bins_omega != req_omega:
            raise RuntimeError(f"Bins omega mismatch: expected {req_omega}, got {bins_omega}")
        req_omega_enabled = _parse_boolish(params.get("solve_score_omega_enabled", True), True)
        bins_omega_enabled = _parse_boolish(ss_data.get("omega_enabled", True), True)
        if bins_omega_enabled != req_omega_enabled:
            raise RuntimeError(
                f"Bins omega_enabled mismatch: expected {req_omega_enabled}, got {bins_omega_enabled}"
            )
        ss_metric = ss_data.get("metric", req_metric)
        cmd.append("--color=solve_score")
        cmd.append(f"--solve_metric={ss_metric}")
        cmd.append(f"--solve_score_clip_lo={ss_data['clip_lo']}")
        cmd.append(f"--solve_score_clip_hi={ss_data['clip_hi']}")
        cmd.append(f"--solve_score_cuts={','.join(str(c) for c in ss_data['cuts_norm'])}")
        cmd.append(f"--solve_score_omega={bins_omega}")
        cmd.append(f"--solve_score_omega_enabled={1 if bins_omega_enabled else 0}")
        cmd = [a for a in cmd if not a.startswith("--color=") or a == "--color=solve_score"]

    rt_path = params.get("root_xforms_path")
    if rt_path:
        cmd.append(f"--root_xforms={rt_path}")
    return cmd


def _merge_outputs(n_tiles, n_workers, emit_pixel_bins):
    uploaded = 0
    uploaded_pixel_bins = 0
    for t in range(n_tiles):
        pix_out = f"/tmp/pix_t{t:04d}.pix"
        wrote_pix = False
        with open(pix_out, "wb") as out_fh:
            for worker_idx in range(n_workers):
                worker_path = f"/tmp/pix_w{worker_idx:02d}_t{t:04d}.pix"
                if os.path.exists(worker_path):
                    if os.path.getsize(worker_path) > 0:
                        with open(worker_path, "rb") as in_fh:
                            shutil.copyfileobj(in_fh, out_fh)
                        wrote_pix = True
                    os.remove(worker_path)
        if wrote_pix:
            uploaded += 1
        else:
            os.remove(pix_out)

        if emit_pixel_bins:
            pbx_out = f"/tmp/pixbin_t{t:04d}.pbx"
            wrote_pbx = False
            with open(pbx_out, "wb") as out_fh:
                for worker_idx in range(n_workers):
                    worker_path = f"/tmp/pixbin_w{worker_idx:02d}_t{t:04d}.pbx"
                    if os.path.exists(worker_path):
                        if os.path.getsize(worker_path) > 0:
                            with open(worker_path, "rb") as in_fh:
                                shutil.copyfileobj(in_fh, out_fh)
                            wrote_pbx = True
                        os.remove(worker_path)
            if wrote_pbx:
                uploaded_pixel_bins += 1
            else:
                os.remove(pbx_out)

    return uploaded, uploaded_pixel_bins


def _upload_outputs(job_id, chunk_idx, n_tiles, emit_pixel_bins):
    uploaded = 0
    uploaded_pixel_bins = 0
    for t in range(n_tiles):
        pix_path = f"/tmp/pix_t{t:04d}.pix"
        if os.path.exists(pix_path) and os.path.getsize(pix_path) > 0:
            s3_key = f"renders/{job_id}/pix_chunk_{chunk_idx:04d}_t{t:04d}.pix"
            with open(pix_path, "rb") as fh:
                s3.upload_fileobj(fh, BUCKET, s3_key)
            os.remove(pix_path)
            uploaded += 1
        pbx_path = f"/tmp/pixbin_t{t:04d}.pbx"
        if emit_pixel_bins and os.path.exists(pbx_path):
            if os.path.getsize(pbx_path) > 0:
                pbx_key = f"renders/{job_id}/pixbin_chunk_{chunk_idx:04d}_t{t:04d}.pbx"
                with open(pbx_path, "rb") as fh:
                    s3.upload_fileobj(fh, BUCKET, pbx_key)
                uploaded_pixel_bins += 1
            os.remove(pbx_path)
    return uploaded, uploaded_pixel_bins


def _split_ranges(n_points, n_workers):
    if n_points <= 0:
        return []
    n_workers = max(1, min(n_workers, n_points))
    base = n_points // n_workers
    extra = n_points % n_workers
    ranges = []
    start = 0
    for idx in range(n_workers):
        width = base + (1 if idx < extra else 0)
        end = start + width
        ranges.append((idx, start, end))
        start = end
    return ranges


def _slice_file(src_path, dest_path, start_byte, size_bytes):
    with open(src_path, "rb") as src, open(dest_path, "wb") as dest:
        src.seek(start_byte)
        remaining = size_bytes
        while remaining > 0:
            chunk = src.read(min(1024 * 1024, remaining))
            if not chunk:
                break
            dest.write(chunk)
            remaining -= len(chunk)
    return dest_path


def _run_worker(worker_params):
    cmd = _build_base_cmd(
        worker_params["params"],
        worker_params["bin_path"],
        worker_params["out_prefix"],
        worker_params.get("saved_bins_path"),
    )
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
    if result.returncode != 0:
        raise RuntimeError(f"roots2pix failed: {result.stderr.strip()}")
    return json.loads(result.stdout)


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    chunk_idx = params.get("chunk_idx", params.get("stripe_idx"))
    if chunk_idx is None:
        raise RuntimeError("raster requires chunk_idx")
    bin_key = params["bin_key"]
    n_tile_cols = params["n_tile_cols"]
    n_tile_rows = params["n_tile_rows"]
    n_tiles = n_tile_cols * n_tile_rows
    degree = int(params["degree"])
    task_id = params.get("task_id", f"raster_{chunk_idx}")

    perf = {
        "engine": "mt",
        "threads": 1,
        "download_us": 0,
        "native_us": 0,
        "upload_us": 0,
        "tiles_uploaded": 0,
        "pixel_bin_tiles_uploaded": 0,
        "roots_plotted": 0,
        "roots_clipped": 0,
    }
    emit_pixel_bins = bool(params.get("emit_pixel_bins"))
    bin_path = "/tmp/stripe.bin"
    saved_bins_path = "/tmp/palette_bins_chunk.bin"

    try:
        report_status(job_id, task_id, "started")

        _cleanup_tmp()

        t0 = time.perf_counter()
        obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
        with open(bin_path, "wb") as f:
            f.write(obj["Body"].read())

        params = dict(params)
        params["root_xforms_path"] = None
        rt_chain = params.get("root_transforms", [])
        if rt_chain:
            rt_path = "/tmp/root_xforms.json"
            with open(rt_path, "w") as rtf:
                json.dump(rt_chain, rtf)
            params["root_xforms_path"] = rt_path

        ss_bins_key = params.get("solve_score_bins_key") or params.get("solve_proximity_bins_key")
        color = params.get("color", "rainbow")
        if color in ("solve_score", "solve_proximity") and not ss_bins_key:
            raise RuntimeError(f"{color} color mode requires solve_score_bins_key")
        if color == "saved_palette":
            saved_palette_bins_key = params.get("saved_palette_bins_key")
            if not saved_palette_bins_key:
                raise RuntimeError("saved_palette color mode requires saved_palette_bins_key")
            bins_obj = s3.get_object(Bucket=BUCKET, Key=saved_palette_bins_key)
            with open(saved_bins_path, "wb") as bf:
                bf.write(bins_obj["Body"].read())
        elif ss_bins_key and color in ("solve_score", "solve_proximity"):
            ss_obj = s3.get_object(Bucket=BUCKET, Key=ss_bins_key)
            params["solve_score_bins_data"] = json.loads(ss_obj["Body"].read())

        perf["download_us"] = int((time.perf_counter() - t0) * 1e6)

        report_status(job_id, task_id, "bin_downloaded")

        record_bytes = degree * 8
        stripe_bytes = os.path.getsize(bin_path)
        if stripe_bytes % record_bytes != 0:
            raise RuntimeError(
                f"chunk byte size {stripe_bytes} is not divisible by degree record size {record_bytes}"
            )
        n_points = stripe_bytes // record_bytes

        requested_threads = _validate_threads(params.get(
            "raster_mt_threads",
            os.environ.get("RASTER_MT_THREADS", str(DEFAULT_THREADS)) or str(DEFAULT_THREADS),
        ))
        ranges = _split_ranges(n_points, requested_threads)
        perf["threads"] = max(1, len(ranges))

        t1 = time.perf_counter()
        worker_payloads = []
        saved_bins_size = os.path.getsize(saved_bins_path) if os.path.exists(saved_bins_path) else 0
        if os.path.exists(saved_bins_path) and saved_bins_size != n_points:
            raise RuntimeError(
                f"saved palette bins size mismatch: expected {n_points} bytes, got {saved_bins_size}"
            )

        for worker_idx, start, end in ranges:
            start_byte = start * record_bytes
            size_bytes = (end - start) * record_bytes
            worker_bin_path = f"/tmp/stripe_w{worker_idx:02d}.bin"
            _slice_file(bin_path, worker_bin_path, start_byte, size_bytes)
            worker_params = dict(params)
            worker_params["pixel_bin_prefix"] = f"/tmp/pixbin_w{worker_idx:02d}"
            worker_saved_bins_path = None
            if os.path.exists(saved_bins_path):
                worker_saved_bins_path = f"/tmp/palette_bins_w{worker_idx:02d}.bin"
                _slice_file(saved_bins_path, worker_saved_bins_path, start, end - start)
            worker_payloads.append({
                "params": worker_params,
                "bin_path": worker_bin_path,
                "saved_bins_path": worker_saved_bins_path,
                "out_prefix": f"/tmp/pix_w{worker_idx:02d}",
            })

        worker_results = []
        if worker_payloads:
            with concurrent.futures.ThreadPoolExecutor(max_workers=len(worker_payloads)) as pool:
                futures = [pool.submit(_run_worker, payload) for payload in worker_payloads]
                for fut in concurrent.futures.as_completed(futures):
                    worker_results.append(fut.result())

        merged_pix, merged_pbx = _merge_outputs(n_tiles, len(ranges), emit_pixel_bins)
        perf["native_us"] = int((time.perf_counter() - t1) * 1e6)
        perf["roots_plotted"] = sum(int(res.get("roots_plotted", 0)) for res in worker_results)
        perf["roots_clipped"] = sum(int(res.get("roots_clipped", 0)) for res in worker_results)

        report_status(job_id, task_id, "rasterized")

        t2 = time.perf_counter()
        uploaded, uploaded_pixel_bins = _upload_outputs(job_id, chunk_idx, n_tiles, emit_pixel_bins)
        perf["upload_us"] = int((time.perf_counter() - t2) * 1e6)
        perf["tiles_uploaded"] = uploaded
        perf["pixel_bin_tiles_uploaded"] = uploaded_pixel_bins

        report_status(job_id, task_id, "done", result_data=perf)
        return ok_response({
            "chunk_idx": chunk_idx,
            "stripe_idx": chunk_idx,
            "tiles_uploaded": uploaded,
            "pixel_bin_tiles_uploaded": uploaded_pixel_bins,
            "raster_us": perf["native_us"],
            "roots_plotted": perf["roots_plotted"],
            "roots_clipped": perf["roots_clipped"],
            "engine": perf["engine"],
            "threads": perf["threads"],
            "merged_tile_files": merged_pix,
            "merged_pixel_bin_files": merged_pbx,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=perf)
        raise
    finally:
        _cleanup_tmp()
        for tmp_path in (bin_path, saved_bins_path):
            try:
                os.remove(tmp_path)
            except OSError:
                pass
