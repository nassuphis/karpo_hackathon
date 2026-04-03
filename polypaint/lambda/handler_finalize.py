"""
Finalize Lambda handler — assembles tile-bucketed .pix files into a .raw tile.

One Lambda per 2D tile. Downloads all .pix files for this tile from all stripes,
pipes them to pixassemble (via stdin), uploads the resulting .raw file.
Reports completion status to DynamoDB for poll-based orchestration.
"""
import json
import logging
import os
import subprocess
import time

import boto3
from botocore.exceptions import ClientError

from shared import BUCKET, parse_body, ok_response, report_status

logger = logging.getLogger()
logger.setLevel(logging.INFO)

s3 = boto3.client("s3")
PIXASSEMBLE = os.path.join(os.path.dirname(__file__), "pixassemble")
PIXBINASSEMBLE = os.path.join(os.path.dirname(__file__), "pixbinassemble")


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

    t0 = time.time()
    logger.info(f"[{task_id}] START tile_idx={tile_idx} n_chunks={n_chunks} tile={tile_w}x{tile_h}")

    try:
        report_status(job_id, task_id, "started")

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
        for c in range(n_chunks):
            key = f"renders/{job_id}/pix_chunk_{c:04d}_t{tile_idx:04d}.pix"
            try:
                try:
                    obj = s3.get_object(Bucket=BUCKET, Key=key)
                except ClientError as e:
                    if e.response["Error"]["Code"] != "NoSuchKey":
                        raise
                    legacy_key = f"renders/{job_id}/pix_{c:04d}_t{tile_idx:04d}.pix"
                    obj = s3.get_object(Bucket=BUCKET, Key=legacy_key)
                    key = legacy_key
                for chunk in obj["Body"].iter_chunks(1024 * 1024):
                    proc.stdin.write(chunk)
                    pix_bytes += len(chunk)
                piped += 1
            except ClientError as e:
                if e.response["Error"]["Code"] == "NoSuchKey":
                    missing += 1
                else:
                    raise
            # Report progress every 10 chunks (or every 100 for large counts)
            interval = 10 if n_chunks <= 100 else 100
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
        report_status(job_id, task_id, "assembled")

        # Stream upload — do not f.read() the full tile into Python memory
        raw_key = f"renders/{job_id}/tile_{tile_idx:04d}.raw"
        with open(raw_path, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, raw_key)

        pixel_bins_size = None
        if emit_pixel_bins:
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
            for c in range(n_chunks):
                key = f"renders/{job_id}/pixbin_chunk_{c:04d}_t{tile_idx:04d}.pbx"
                try:
                    obj = s3.get_object(Bucket=BUCKET, Key=key)
                except ClientError as e:
                    if e.response["Error"]["Code"] == "NoSuchKey":
                        continue
                    raise
                for chunk in obj["Body"].iter_chunks(1024 * 1024):
                    bin_proc.stdin.write(chunk)
                    bin_bytes += len(chunk)
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
                s3.upload_fileobj(fh, BUCKET, pixel_bins_out_key)
            logger.info(
                f"[{task_id}] PIXBINS {pixel_bins_out_key} ({pixel_bins_size} bytes, {piped_bins} chunk files, {bin_bytes} bytes in)"
            )
            os.remove(pixel_bins_path)

        t_upload = time.time() - t0
        logger.info(f"[{task_id}] UPLOADED {raw_key} ({raw_size} bytes) in {t_upload:.1f}s total")
        os.remove(raw_path)

        report_status(job_id, task_id, "done")
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
        report_status(job_id, task_id, "error", str(e))
        raise
    finally:
        for tmp_path in ("/tmp/tile.raw", "/tmp/tile.pixel_bins.bin"):
            try:
                os.remove(tmp_path)
            except OSError:
                pass
