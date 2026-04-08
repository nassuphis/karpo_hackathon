"""
Finalize Lambda handler — assembles tile-bucketed .pix files into a .raw tile.

One Lambda per 2D tile. Downloads all .pix files for this tile from all stripes,
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

from shared import BUCKET, parse_body, ok_response, report_status

logger = logging.getLogger()
logger.setLevel(logging.INFO)
PIXASSEMBLE = os.path.join(os.path.dirname(__file__), "pixassemble")
PIXBINASSEMBLE = os.path.join(os.path.dirname(__file__), "pixbinassemble")
DEFAULT_FINALIZE_WORKERS = 16
MAX_FINALIZE_WORKERS = 64


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
    return exc.response["Error"]["Code"] == "NoSuchKey"


def _read_body_bytes(body):
    if hasattr(body, "read"):
        return body.read()
    chunks = []
    for chunk in body.iter_chunks(1024 * 1024):
        chunks.append(chunk)
    return b"".join(chunks)


def _load_finalize_blob(finalize_s3, key, legacy_key=None):
    try:
        obj = finalize_s3.get_object(Bucket=BUCKET, Key=key)
        return {"key": key, "data": _read_body_bytes(obj["Body"])}
    except ClientError as exc:
        if not _is_missing_s3_error(exc):
            raise
    if not legacy_key:
        return None
    try:
        obj = finalize_s3.get_object(Bucket=BUCKET, Key=legacy_key)
        return {"key": legacy_key, "data": _read_body_bytes(obj["Body"])}
    except ClientError as exc:
        if _is_missing_s3_error(exc):
            return None
        raise


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


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    tile_idx = params["tile_idx"]
    n_chunks = params.get("n_chunks", params.get("n_stripes"))
    if n_chunks is None:
        raise RuntimeError("finalize requires n_chunks")
    tile_w = params["tile_w"]
    tile_h = params["tile_h"]
    task_id = params.get("task_id", f"tile_{tile_idx}")
    emit_pixel_bins = bool(params.get("emit_pixel_bins")) and bool(params.get("pixel_bins_out_key"))
    pixel_bins_out_key = params.get("pixel_bins_out_key")
    finalize_workers = min(_validate_finalize_workers(params.get("finalize_workers")), max(1, int(n_chunks)))
    finalize_s3 = _finalize_s3_client(finalize_workers)

    t0 = time.time()
    logger.info(f"[{task_id}] START tile_idx={tile_idx} n_chunks={n_chunks} tile={tile_w}x{tile_h} workers={finalize_workers}")
    progress = {
        "phase": "finalize",
        "tile_idx": tile_idx,
        "n_chunks": n_chunks,
        "emit_pixel_bins": emit_pixel_bins,
        "workers": finalize_workers,
    }

    try:
        report_status(job_id, task_id, "started", result_data=progress)

        raw_path = "/tmp/tile.raw"
        pixel_bins_path = "/tmp/tile.pixel_bins.bin"
        proc = subprocess.Popen(
            [PIXASSEMBLE,
             f"--tile_w={tile_w}",
             f"--tile_h={tile_h}",
             f"--output={raw_path}"],
            stdin=subprocess.PIPE,
            stderr=subprocess.PIPE)

        # Stream .pix files from all chunks into pixassemble's stdin
        piped = 0
        pix_bytes = 0
        missing = 0
        interval = 10 if n_chunks <= 100 else 100
        for c, loaded in _ordered_prefetch(
            n_chunks,
            finalize_workers,
            lambda idx: _load_finalize_blob(
                finalize_s3,
                f"renders/{job_id}/pix_chunk_{idx:04d}_t{tile_idx:04d}.pix",
                legacy_key=f"renders/{job_id}/pix_{idx:04d}_t{tile_idx:04d}.pix",
            ),
        ):
            if loaded is None:
                missing += 1
            else:
                proc.stdin.write(loaded["data"])
                pix_bytes += len(loaded["data"])
                piped += 1
            # Report progress every 10 chunks (or every 100 for large counts)
            if (c + 1) % interval == 0:
                report_status(job_id, task_id, f"reading_{c+1}/{n_chunks}_{piped}pix_{pix_bytes}B")

        t_read = time.time() - t0
        logger.info(f"[{task_id}] READ {piped}/{n_chunks} pix files ({pix_bytes} bytes, {missing} missing) in {t_read:.1f}s")

        proc.stdin.close()
        rc = proc.wait(timeout=120)
        stderr_out = proc.stderr.read().decode('utf-8', errors='replace')
        if stderr_out:
            logger.info(f"[{task_id}] pixassemble stderr: {stderr_out[:500]}")
        if rc != 0:
            raise RuntimeError(f"pixassemble failed (rc={rc}), stderr: {stderr_out[:500]}")

        t_assemble = time.time() - t0
        raw_size = os.path.getsize(raw_path)
        logger.info(f"[{task_id}] ASSEMBLED {raw_size} bytes in {t_assemble:.1f}s")
        progress["read_ms"] = int(t_read * 1000)
        progress["assemble_ms"] = int((t_assemble - t_read) * 1000)
        progress["pix_files"] = piped
        progress["pix_bytes"] = pix_bytes
        report_status(job_id, task_id, "assembled", result_data=progress)

        # Stream upload — do not f.read() the full tile into Python memory
        raw_key = f"renders/{job_id}/tile_{tile_idx:04d}.raw"
        with open(raw_path, "rb") as fh:
            finalize_s3.upload_fileobj(fh, BUCKET, raw_key)

        pixel_bins_size = None
        if emit_pixel_bins:
            t_pixel_bins_start = time.time()
            bin_proc = subprocess.Popen(
                [PIXBINASSEMBLE,
                 f"--tile_w={tile_w}",
                 f"--tile_h={tile_h}",
                 "--empty=255",
                 f"--output={pixel_bins_path}"],
                stdin=subprocess.PIPE,
                stderr=subprocess.PIPE)

            piped_bins = 0
            bin_bytes = 0
            missing_bins = 0
            for _, loaded in _ordered_prefetch(
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

            bin_proc.stdin.close()
            rc = bin_proc.wait(timeout=120)
            stderr_out = bin_proc.stderr.read().decode("utf-8", errors="replace")
            if stderr_out:
                logger.info(f"[{task_id}] pixbinassemble stderr: {stderr_out[:500]}")
            if rc != 0:
                raise RuntimeError(f"pixbinassemble failed (rc={rc}), stderr: {stderr_out[:500]}")
            pixel_bins_size = os.path.getsize(pixel_bins_path)
            with open(pixel_bins_path, "rb") as fh:
                finalize_s3.upload_fileobj(fh, BUCKET, pixel_bins_out_key)
            logger.info(
                f"[{task_id}] PIXBINS {pixel_bins_out_key} ({pixel_bins_size} bytes, {piped_bins} chunk files, {bin_bytes} bytes in, {missing_bins} missing)"
            )
            progress["pixel_bins_ms"] = int((time.time() - t_pixel_bins_start) * 1000)
            progress["pixel_bin_tiles"] = 1
            progress["pixel_bin_size"] = pixel_bins_size
            os.remove(pixel_bins_path)

        t_upload = time.time() - t0
        logger.info(f"[{task_id}] UPLOADED {raw_key} ({raw_size} bytes) in {t_upload:.1f}s total")
        os.remove(raw_path)

        progress["upload_ms"] = int((t_upload - t_assemble) * 1000)
        progress["raw_key"] = raw_key
        progress["raw_size"] = raw_size
        progress["missing_pix_files"] = missing
        report_status(job_id, task_id, "done", result_data=progress)
        logger.info(f"[{task_id}] DONE in {time.time() - t0:.1f}s (read={t_read:.1f}s assemble={t_assemble - t_read:.1f}s upload={t_upload - t_assemble:.1f}s)")
        return ok_response({
            "tile_idx": tile_idx,
            "raw_key": raw_key,
            "raw_size": raw_size,
            "pix_files": piped,
            "pixel_bins_key": pixel_bins_out_key if emit_pixel_bins else "",
            "pixel_bins_size": pixel_bins_size,
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
