"""
Finalize Lambda handler — assembles tile-bucketed .pix files into a .raw tile.

One Lambda per 2D tile. Downloads all .pix files for this tile from all chunks,
pipes them to pixassemble (via stdin), uploads the resulting .raw file.
Reports completion status to DynamoDB for poll-based orchestration.
"""
import logging
import os
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor

import boto3
from botocore.config import Config
from botocore.exceptions import ClientError

from shared import BUCKET, attach_contract_warnings, contract_param, parse_body, ok_response, parse_boolish, report_status

logger = logging.getLogger()
logger.setLevel(logging.INFO)
PIXASSEMBLE = os.path.join(os.path.dirname(__file__), "pixassemble")
PIXBINASSEMBLE = os.path.join(os.path.dirname(__file__), "pixbinassemble")
PIXEL_BINS_RENDER = os.path.join(os.path.dirname(__file__), "pixel_bins_render")
DEFAULT_FINALIZE_WORKERS = 16
MAX_FINALIZE_WORKERS = 64
FINALIZE_PROGRESS_INTERVAL_SEC = 2.0


def _parse_int(value, default):
    try:
        return int(value)
    except (TypeError, ValueError):
        return default


def _tile_shape(tile_idx, width, height, tile_size, n_tile_cols):
    tile_idx = int(tile_idx)
    width = int(width)
    height = int(height)
    tile_size = int(tile_size)
    n_tile_cols = int(n_tile_cols)
    row = tile_idx // n_tile_cols
    col = tile_idx % n_tile_cols
    tile_w = max(0, min(tile_size, width - col * tile_size))
    tile_h = max(0, min(tile_size, height - row * tile_size))
    return tile_w, tile_h


def _finalize_static_progress(
    tile_idx,
    tile_w,
    tile_h,
    n_chunks,
    finalize_workers,
    emit_pixel_bins,
    pixel_bins_drive_rgb=False,
    pixel_bin_fragment_mode="sparse_chunks",
):
    tile_pixels = int(tile_w) * int(tile_h)
    return {
        "phase": "finalize",
        "tile_idx": tile_idx,
        "n_chunks": int(n_chunks),
        "tile_w": int(tile_w),
        "tile_h": int(tile_h),
        "tile_pixels": tile_pixels,
        "raw_expected_size": 12 + tile_pixels * 3,
        "tile_rgb_bytes": tile_pixels * 3,
        "emit_pixel_bins": bool(emit_pixel_bins),
        "pixel_bins_drive_rgb": bool(pixel_bins_drive_rgb),
        "rgb_source": "pixel_bins" if pixel_bins_drive_rgb else "pix",
        "workers": int(finalize_workers),
        "access_mode": "s3_prefetch_stdin",
        "pix_access_mode": "s3_ordered_prefetch",
        "assemble_mode": "pixel_bins_render" if pixel_bins_drive_rgb else "pixassemble_stdin",
        "pixel_bins_access_mode": "s3_ordered_prefetch" if emit_pixel_bins else "",
        "pixel_bin_fragment_mode": pixel_bin_fragment_mode,
    }


def _validate_finalize_workers(value):
    if value in (None, ""):
        env_value = os.environ.get("FINALIZE_WORKERS", DEFAULT_FINALIZE_WORKERS)
        value = env_value
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"finalize_workers must be an integer, got {value!r}")
    if not (1 <= workers <= MAX_FINALIZE_WORKERS):
        raise RuntimeError(f"finalize_workers must be in [1, {MAX_FINALIZE_WORKERS}], got {workers}")
    return workers


def _finalize_s3_client(max_workers):
    pool_size = max(16, int(max_workers) * 2)
    return boto3.client("s3", config=Config(max_pool_connections=pool_size))


def _is_missing_s3_error(exc):
    code = getattr(exc, "response", {}).get("Error", {}).get("Code")
    if code in {"NoSuchKey", "404", "NotFound"}:
        return True
    msg = str(exc)
    return "NoSuchKey" in msg or "NotFound" in msg


def _read_body_bytes(body):
    if hasattr(body, "read"):
        return body.read()
    chunks = []
    for chunk in body.iter_chunks(1024 * 1024):
        chunks.append(chunk)
    return b"".join(chunks)


def _load_finalize_blob(finalize_s3, key):
    try:
        obj = finalize_s3.get_object(Bucket=BUCKET, Key=key)
        return {"key": key, "data": _read_body_bytes(obj["Body"])}
    except ClientError as exc:
        if not _is_missing_s3_error(exc):
            raise
    return None


def _ordered_prefetch(n_items, workers, load_fn):
    if n_items <= 0:
        return
    bounded_workers = max(1, min(int(workers), int(n_items)))
    with ThreadPoolExecutor(max_workers=bounded_workers) as executor:
        pending = {}
        next_submit = 0
        next_emit = 0
        while next_emit < n_items:
            while next_submit < n_items and len(pending) < bounded_workers:
                idx = next_submit
                pending[idx] = executor.submit(load_fn, idx)
                next_submit += 1
            loaded = pending.pop(next_emit).result()
            yield next_emit, loaded
            next_emit += 1


def _assemble_pixel_bins_tile(
    *,
    finalize_s3,
    job_id,
    task_id,
    tile_idx,
    n_chunks,
    tile_w,
    tile_h,
    pixel_bins_path,
    pixel_bins_out_key,
    pixel_bins_empty,
    finalize_workers,
    progress,
):
    t_pixel_bins_start = time.time()
    bin_proc = subprocess.Popen(
        [PIXBINASSEMBLE,
         f"--tile_w={tile_w}",
         f"--tile_h={tile_h}",
         f"--empty={pixel_bins_empty}",
         f"--output={pixel_bins_path}"],
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE)

    piped_bins = 0
    bin_bytes = 0
    missing_bins = 0
    last_bin_progress = t_pixel_bins_start
    for bin_idx, loaded in _ordered_prefetch(
        n_chunks,
        finalize_workers,
        lambda idx: _load_finalize_blob(
            finalize_s3,
            f"renders/{job_id}/pixbin_chunk_{idx:04d}_t{tile_idx:04d}.pbx",
        ),
    ):
        if loaded is None:
            missing_bins += 1
            continue
        bin_proc.stdin.write(loaded["data"])
        bin_bytes += len(loaded["data"])
        piped_bins += 1
        now = time.time()
        if now - last_bin_progress >= FINALIZE_PROGRESS_INTERVAL_SEC:
            progress.update({
                "stage": "read_pbx",
                "pixel_bin_files_seen": bin_idx + 1,
                "pixel_bin_files_piped": piped_bins,
                "pixel_bin_files_missing": missing_bins,
                "pixel_bin_bytes": bin_bytes,
                "pixel_bins_progress_ms": int((now - t_pixel_bins_start) * 1000),
            })
            report_status(job_id, task_id, f"reading_pbx_{bin_idx+1}/{n_chunks}", result_data=progress)
            last_bin_progress = now

    bin_proc.stdin.close()
    progress.update({
        "stage": "assemble_pbx",
        "pixel_bin_files_piped": piped_bins,
        "pixel_bin_files_missing": missing_bins,
        "pixel_bin_bytes": bin_bytes,
    })
    report_status(job_id, task_id, "assembling_pbx", result_data=progress)
    rc = bin_proc.wait(timeout=120)
    stderr_out = bin_proc.stderr.read().decode("utf-8", errors="replace")
    if stderr_out:
        logger.info(f"[{task_id}] pixbinassemble stderr: {stderr_out[:500]}")
    if rc != 0:
        raise RuntimeError(f"pixbinassemble failed (rc={rc}), stderr: {stderr_out[:500]}")
    pixel_bins_size = os.path.getsize(pixel_bins_path)
    progress.update({
        "stage": "upload_pbx",
        "pixel_bin_size": pixel_bins_size,
        "pixel_bin_files_piped": piped_bins,
        "pixel_bin_files_missing": missing_bins,
        "pixel_bin_bytes": bin_bytes,
    })
    report_status(job_id, task_id, "uploading_pbx", result_data=progress)
    with open(pixel_bins_path, "rb") as fh:
        finalize_s3.upload_fileobj(fh, BUCKET, pixel_bins_out_key)
    logger.info(
        f"[{task_id}] PIXBINS {pixel_bins_out_key} ({pixel_bins_size} bytes, {piped_bins} chunk files, {bin_bytes} bytes in, {missing_bins} missing)"
    )
    pixel_bins_ms = int((time.time() - t_pixel_bins_start) * 1000)
    progress.update({
        "pixel_bins_ms": pixel_bins_ms,
        "pixel_bin_tiles": 1,
        "pixel_bin_size": pixel_bins_size,
        "pixel_bin_files": piped_bins,
        "pixel_bin_files_piped": piped_bins,
        "pixel_bin_files_missing": missing_bins,
        "pixel_bin_bytes": bin_bytes,
    })
    return {
        "pixel_bins_size": pixel_bins_size,
        "pixel_bin_files": piped_bins,
        "pixel_bin_bytes": bin_bytes,
        "pixel_bin_files_missing": missing_bins,
        "pixel_bins_ms": pixel_bins_ms,
    }


def _assemble_dense_group_pixel_bins_tile(
    *,
    finalize_s3,
    job_id,
    task_id,
    tile_idx,
    raster_item_count,
    tile_w,
    tile_h,
    pixel_bins_path,
    pixel_bins_out_key,
    pixel_bins_empty,
    finalize_workers,
    progress,
):
    t_pixel_bins_start = time.time()
    bin_proc = subprocess.Popen(
        [PIXBINASSEMBLE,
         f"--tile_w={tile_w}",
         f"--tile_h={tile_h}",
         f"--empty={pixel_bins_empty}",
         f"--output={pixel_bins_path}",
         "--input_format=dense_layers"],
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE)

    piped_bins = 0
    bin_bytes = 0
    missing_bins = 0
    expected_layer_bytes = int(tile_w) * int(tile_h)
    last_bin_progress = t_pixel_bins_start
    for group_idx, loaded in _ordered_prefetch(
        raster_item_count,
        finalize_workers,
        lambda idx: _load_finalize_blob(
            finalize_s3,
            f"renders/{job_id}/pixbin_group_{idx:04d}_t{tile_idx:04d}.u8",
        ),
    ):
        if loaded is None:
            missing_bins += 1
            continue
        data = loaded["data"]
        if len(data) != expected_layer_bytes:
            raise RuntimeError(
                f"Dense pixel-bin group tile has wrong size: {loaded['key']} "
                f"{len(data)} bytes != {expected_layer_bytes}"
            )
        bin_proc.stdin.write(data)
        bin_bytes += len(data)
        piped_bins += 1
        now = time.time()
        if now - last_bin_progress >= FINALIZE_PROGRESS_INTERVAL_SEC:
            progress.update({
                "stage": "read_dense_pbx",
                "pixel_bin_files_seen": group_idx + 1,
                "pixel_bin_files_piped": piped_bins,
                "pixel_bin_files_missing": missing_bins,
                "pixel_bin_bytes": bin_bytes,
                "pixel_bins_progress_ms": int((now - t_pixel_bins_start) * 1000),
            })
            report_status(job_id, task_id, f"reading_dense_pbx_{group_idx+1}/{raster_item_count}", result_data=progress)
            last_bin_progress = now

    bin_proc.stdin.close()
    progress.update({
        "stage": "assemble_dense_pbx",
        "pixel_bin_files_piped": piped_bins,
        "pixel_bin_files_missing": missing_bins,
        "pixel_bin_bytes": bin_bytes,
        "pixel_bin_layer_bytes": expected_layer_bytes,
    })
    report_status(job_id, task_id, "assembling_dense_pbx", result_data=progress)
    rc = bin_proc.wait(timeout=120)
    stderr_out = bin_proc.stderr.read().decode("utf-8", errors="replace")
    if stderr_out:
        logger.info(f"[{task_id}] pixbinassemble dense stderr: {stderr_out[:500]}")
    if rc != 0:
        raise RuntimeError(f"pixbinassemble dense failed (rc={rc}), stderr: {stderr_out[:500]}")
    pixel_bins_size = os.path.getsize(pixel_bins_path)
    progress.update({
        "stage": "upload_dense_pbx",
        "pixel_bin_size": pixel_bins_size,
        "pixel_bin_files_piped": piped_bins,
        "pixel_bin_files_missing": missing_bins,
        "pixel_bin_bytes": bin_bytes,
    })
    report_status(job_id, task_id, "uploading_dense_pbx", result_data=progress)
    with open(pixel_bins_path, "rb") as fh:
        finalize_s3.upload_fileobj(fh, BUCKET, pixel_bins_out_key)
    logger.info(
        f"[{task_id}] DENSE PIXBINS {pixel_bins_out_key} ({pixel_bins_size} bytes, {piped_bins} group files, {bin_bytes} bytes in, {missing_bins} missing)"
    )
    pixel_bins_ms = int((time.time() - t_pixel_bins_start) * 1000)
    progress.update({
        "pixel_bins_ms": pixel_bins_ms,
        "pixel_bin_tiles": 1,
        "pixel_bin_size": pixel_bins_size,
        "pixel_bin_files": piped_bins,
        "pixel_bin_files_piped": piped_bins,
        "pixel_bin_files_missing": missing_bins,
        "pixel_bin_bytes": bin_bytes,
    })
    return {
        "pixel_bins_size": pixel_bins_size,
        "pixel_bin_files": piped_bins,
        "pixel_bin_bytes": bin_bytes,
        "pixel_bin_files_missing": missing_bins,
        "pixel_bins_ms": pixel_bins_ms,
    }


def handler(event, context):
    params = parse_body(event)
    contract_warnings = []
    job_id = params["job_id"]
    tile_idx = params["tile_idx"]
    n_chunks = params.get("n_chunks", params.get("source_item_count", params.get("n_stripes")))
    if n_chunks is None:
        raise RuntimeError("finalize requires n_chunks or source_item_count")
    tile_w = params.get("tile_w")
    tile_h = params.get("tile_h")
    if tile_w in (None, "") or tile_h in (None, ""):
        tile_w, tile_h = _tile_shape(
            tile_idx,
            params["width"],
            params["height"],
            params["tile_size"],
            params["n_tile_cols"],
        )
    task_id = params.get("task_id", f"tile_{tile_idx}")
    emit_pixel_bins_requested = parse_boolish(params.get("emit_pixel_bins"), False)
    pixel_bins_out_key = params.get("pixel_bins_out_key")
    if not pixel_bins_out_key:
        pixel_bins_out_prefix = str(params.get("pixel_bins_out_prefix") or "").strip()
        if pixel_bins_out_prefix:
            pixel_bins_out_key = f"{pixel_bins_out_prefix}{int(tile_idx):04d}.bin"
    emit_pixel_bins = emit_pixel_bins_requested and bool(pixel_bins_out_key)
    pixel_bins_drive_rgb = parse_boolish(params.get("pixel_bins_drive_rgb"), False)
    if pixel_bins_drive_rgb and not emit_pixel_bins:
        raise RuntimeError("pixel_bins_drive_rgb requires emit_pixel_bins and pixel_bins_out_key")
    pixel_bin_fragment_mode = str(params.get("pixel_bin_fragment_mode") or "sparse_chunks").strip().lower()
    if pixel_bin_fragment_mode not in ("sparse_chunks", "dense_grouped"):
        raise RuntimeError(f"pixel_bin_fragment_mode must be 'sparse_chunks' or 'dense_grouped', got {pixel_bin_fragment_mode!r}")
    raster_item_count = _parse_int(params.get("raster_item_count"), int(n_chunks))
    if raster_item_count < 1:
        raster_item_count = int(n_chunks)
    palette = str(params.get("palette") or "inferno").strip() or "inferno"
    background_color = str(params.get("background_color") or "000000").strip() or "000000"
    pixel_bins_empty = _parse_int(params.get("pixel_bins_empty"), 255)
    worker_item_count = raster_item_count if pixel_bin_fragment_mode == "dense_grouped" and pixel_bins_drive_rgb else int(n_chunks)
    finalize_workers = min(
        _validate_finalize_workers(contract_param(params, "finalize_workers", None, contract_warnings, warning_default=os.environ.get("FINALIZE_WORKERS", DEFAULT_FINALIZE_WORKERS))),
        max(1, int(worker_item_count))
    )
    finalize_s3 = _finalize_s3_client(finalize_workers)

    t0 = time.time()
    logger.info(f"[{task_id}] START tile_idx={tile_idx} n_chunks={n_chunks} tile={tile_w}x{tile_h} workers={finalize_workers}")
    progress = attach_contract_warnings(
        _finalize_static_progress(
            tile_idx,
            tile_w,
            tile_h,
            raster_item_count if pixel_bin_fragment_mode == "dense_grouped" and pixel_bins_drive_rgb else n_chunks,
            finalize_workers,
            emit_pixel_bins,
            pixel_bins_drive_rgb,
            pixel_bin_fragment_mode,
        ),
        contract_warnings,
    )
    progress["chunk_count"] = int(n_chunks)
    progress["raster_item_count"] = int(raster_item_count)
    progress["palette"] = palette
    progress["background_color"] = background_color
    progress["pixel_bins_empty"] = pixel_bins_empty

    try:
        report_status(job_id, task_id, "started", result_data=progress)

        raw_path = "/tmp/tile.raw"
        pixel_bins_path = "/tmp/tile.pixel_bins.bin"
        piped = 0
        pix_bytes = 0
        missing = 0
        pixel_bins_size = None
        if pixel_bins_drive_rgb:
            progress.update({
                "stage": "bin_first",
                "read_ms": 0,
                "pix_files": 0,
                "pix_files_piped": 0,
                "pix_bytes": 0,
                "pix_files_missing": 0,
            })
            if pixel_bin_fragment_mode == "dense_grouped":
                bin_result = _assemble_dense_group_pixel_bins_tile(
                    finalize_s3=finalize_s3,
                    job_id=job_id,
                    task_id=task_id,
                    tile_idx=tile_idx,
                    raster_item_count=raster_item_count,
                    tile_w=tile_w,
                    tile_h=tile_h,
                    pixel_bins_path=pixel_bins_path,
                    pixel_bins_out_key=pixel_bins_out_key,
                    pixel_bins_empty=pixel_bins_empty,
                    finalize_workers=finalize_workers,
                    progress=progress,
                )
            else:
                bin_result = _assemble_pixel_bins_tile(
                    finalize_s3=finalize_s3,
                    job_id=job_id,
                    task_id=task_id,
                    tile_idx=tile_idx,
                    n_chunks=n_chunks,
                    tile_w=tile_w,
                    tile_h=tile_h,
                    pixel_bins_path=pixel_bins_path,
                    pixel_bins_out_key=pixel_bins_out_key,
                    pixel_bins_empty=pixel_bins_empty,
                    finalize_workers=finalize_workers,
                    progress=progress,
                )
            pixel_bins_size = bin_result["pixel_bins_size"]
            t_render_bins = time.time()
            progress.update({
                "stage": "render_rgb_from_bins",
                "pixel_bin_size": pixel_bins_size,
            })
            report_status(job_id, task_id, "rendering_rgb_from_bins", result_data=progress)
            render = subprocess.run(
                [
                    PIXEL_BINS_RENDER,
                    pixel_bins_path,
                    raw_path,
                    f"--tile_w={tile_w}",
                    f"--tile_h={tile_h}",
                    f"--palette={palette}",
                    f"--background_color={background_color}",
                    f"--empty={pixel_bins_empty}",
                ],
                capture_output=True,
                text=True,
                timeout=300,
            )
            if render.returncode != 0:
                raise RuntimeError(f"pixel_bins_render failed: {render.stderr.strip() or 'unknown error'}")
            t_assemble = time.time()
            raw_size = os.path.getsize(raw_path)
            progress["assemble_ms"] = int((t_assemble - t_render_bins) * 1000)
            progress["rgb_from_bins_ms"] = progress["assemble_ms"]
            progress["stage"] = "assembled"
            progress["progress"] = 1.0
            report_status(job_id, task_id, "assembled", result_data=progress)
        else:
            proc = subprocess.Popen(
                [PIXASSEMBLE,
                 f"--tile_w={tile_w}",
                 f"--tile_h={tile_h}",
                 f"--output={raw_path}"],
                stdin=subprocess.PIPE,
                stderr=subprocess.PIPE)

            # Stream .pix files from all chunks into pixassemble's stdin.
            read_started = time.time()
            last_progress = read_started
            for c, loaded in _ordered_prefetch(
                n_chunks,
                finalize_workers,
                lambda idx: _load_finalize_blob(
                    finalize_s3,
                    f"renders/{job_id}/pix_chunk_{idx:04d}_t{tile_idx:04d}.pix",
                ),
            ):
                if loaded is None:
                    missing += 1
                else:
                    proc.stdin.write(loaded["data"])
                    pix_bytes += len(loaded["data"])
                    piped += 1
                now = time.time()
                if now - last_progress >= FINALIZE_PROGRESS_INTERVAL_SEC:
                    progress.update({
                        "stage": "read_pix",
                        "pix_files_seen": c + 1,
                        "pix_files_piped": piped,
                        "pix_files_missing": missing,
                        "pix_bytes": pix_bytes,
                        "read_progress_ms": int((now - read_started) * 1000),
                        "progress": (c + 1) / max(1, int(n_chunks)),
                    })
                    report_status(job_id, task_id, f"reading_pix_{c+1}/{n_chunks}", result_data=progress)
                    last_progress = now

            t_read = time.time() - t0
            logger.info(f"[{task_id}] READ {piped}/{n_chunks} pix files ({pix_bytes} bytes, {missing} missing) in {t_read:.1f}s")

            proc.stdin.close()
            progress.update({
                "stage": "assemble_pix",
                "read_ms": int(t_read * 1000),
                "pix_files_piped": piped,
                "pix_files_missing": missing,
                "pix_bytes": pix_bytes,
            })
            report_status(job_id, task_id, "assembling_pix", result_data=progress)
            rc = proc.wait(timeout=120)
            stderr_out = proc.stderr.read().decode('utf-8', errors='replace')
            if stderr_out:
                logger.info(f"[{task_id}] pixassemble stderr: {stderr_out[:500]}")
            if rc != 0:
                raise RuntimeError(f"pixassemble failed (rc={rc}), stderr: {stderr_out[:500]}")

            t_assemble = time.time()
            raw_size = os.path.getsize(raw_path)
            logger.info(f"[{task_id}] ASSEMBLED {raw_size} bytes in {t_assemble - t0:.1f}s")
            progress["read_ms"] = int(t_read * 1000)
            progress["assemble_ms"] = int((t_assemble - t0 - t_read) * 1000)
            progress["pix_files"] = piped
            progress["pix_files_piped"] = piped
            progress["pix_bytes"] = pix_bytes
            progress["pix_files_missing"] = missing
            progress["stage"] = "assembled"
            progress["progress"] = 1.0
            report_status(job_id, task_id, "assembled", result_data=progress)

            if emit_pixel_bins:
                if pixel_bin_fragment_mode == "dense_grouped":
                    bin_result = _assemble_dense_group_pixel_bins_tile(
                        finalize_s3=finalize_s3,
                        job_id=job_id,
                        task_id=task_id,
                        tile_idx=tile_idx,
                        raster_item_count=raster_item_count,
                        tile_w=tile_w,
                        tile_h=tile_h,
                        pixel_bins_path=pixel_bins_path,
                        pixel_bins_out_key=pixel_bins_out_key,
                        pixel_bins_empty=pixel_bins_empty,
                        finalize_workers=finalize_workers,
                        progress=progress,
                    )
                else:
                    bin_result = _assemble_pixel_bins_tile(
                        finalize_s3=finalize_s3,
                        job_id=job_id,
                        task_id=task_id,
                        tile_idx=tile_idx,
                        n_chunks=n_chunks,
                        tile_w=tile_w,
                        tile_h=tile_h,
                        pixel_bins_path=pixel_bins_path,
                        pixel_bins_out_key=pixel_bins_out_key,
                        pixel_bins_empty=pixel_bins_empty,
                        finalize_workers=finalize_workers,
                        progress=progress,
                    )
                pixel_bins_size = bin_result["pixel_bins_size"]

        # Stream upload — do not f.read() the full tile into Python memory.
        raw_key = f"renders/{job_id}/tile_{tile_idx:04d}.raw"
        progress.update({
            "stage": "upload_raw",
            "raw_key": raw_key,
            "raw_size": raw_size,
        })
        report_status(job_id, task_id, "uploading_raw", result_data=progress)
        t_raw_upload_start = time.time()
        with open(raw_path, "rb") as fh:
            finalize_s3.upload_fileobj(fh, BUCKET, raw_key)
        if os.path.exists(pixel_bins_path):
            os.remove(pixel_bins_path)

        t_upload = time.time() - t0
        logger.info(f"[{task_id}] UPLOADED {raw_key} ({raw_size} bytes) in {t_upload:.1f}s total")
        os.remove(raw_path)

        progress["upload_ms"] = int((time.time() - t_raw_upload_start) * 1000)
        progress["raw_key"] = raw_key
        progress["raw_size"] = raw_size
        progress["missing_pix_files"] = missing
        progress["stage"] = "done"
        report_status(job_id, task_id, "done", result_data=progress)
        logger.info(
            f"[{task_id}] DONE in {time.time() - t0:.1f}s "
            f"(read={progress.get('read_ms', 0) / 1000:.1f}s "
            f"assemble={progress.get('assemble_ms', 0) / 1000:.1f}s "
            f"upload={progress.get('upload_ms', 0) / 1000:.1f}s "
            f"pbx={progress.get('pixel_bins_ms', 0) / 1000:.1f}s)"
        )
        return ok_response({
            "tile_idx": tile_idx,
            "raw_key": raw_key,
            "raw_size": raw_size,
            "pix_files": piped,
            "pixel_bins_key": pixel_bins_out_key if emit_pixel_bins else "",
            "pixel_bins_size": pixel_bins_size,
            "pixel_bins_drive_rgb": pixel_bins_drive_rgb,
            "rgb_source": "pixel_bins" if pixel_bins_drive_rgb else "pix",
            "pixel_bin_fragment_mode": pixel_bin_fragment_mode,
            "raster_item_count": raster_item_count,
        })

    except Exception as e:
        logger.error(f"[{task_id}] FAILED after {time.time() - t0:.1f}s: {type(e).__name__}: {e}")
        progress["error"] = str(e)
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        for tmp_path in ("/tmp/tile.raw", "/tmp/tile.pixel_bins.bin"):
            try:
                os.remove(tmp_path)
            except OSError:
                pass
