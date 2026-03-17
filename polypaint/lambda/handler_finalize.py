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


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    tile_idx = params["tile_idx"]
    n_stripes = params["n_stripes"]
    tile_w = params["tile_w"]
    tile_h = params["tile_h"]
    task_id = f"tile_{tile_idx}"

    t0 = time.time()
    logger.info(f"[{task_id}] START tile_idx={tile_idx} n_stripes={n_stripes} tile={tile_w}x{tile_h}")

    try:
        report_status(job_id, task_id, "started")

        raw_path = "/tmp/tile.raw"
        proc = subprocess.Popen(
            [PIXASSEMBLE,
             f"--tile_w={tile_w}",
             f"--tile_h={tile_h}",
             f"--output={raw_path}"],
            stdin=subprocess.PIPE,
            stderr=subprocess.PIPE)

        # Stream .pix files from all stripes into pixassemble's stdin
        piped = 0
        pix_bytes = 0
        missing = 0
        for s in range(n_stripes):
            key = f"renders/{job_id}/pix_{s:04d}_t{tile_idx:04d}.pix"
            try:
                obj = s3.get_object(Bucket=BUCKET, Key=key)
                for chunk in obj["Body"].iter_chunks(1024 * 1024):
                    proc.stdin.write(chunk)
                    pix_bytes += len(chunk)
                piped += 1
            except ClientError as e:
                if e.response["Error"]["Code"] == "NoSuchKey":
                    missing += 1
                else:
                    raise
            # Report progress every 10 stripes (or every 100 for large counts)
            interval = 10 if n_stripes <= 100 else 100
            if (s + 1) % interval == 0:
                report_status(job_id, task_id, f"reading_{s+1}/{n_stripes}_{piped}pix_{pix_bytes}B")

        t_read = time.time() - t0
        logger.info(f"[{task_id}] READ {piped}/{n_stripes} pix files ({pix_bytes} bytes, {missing} missing) in {t_read:.1f}s")

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
        })

    except Exception as e:
        logger.error(f"[{task_id}] FAILED after {time.time() - t0:.1f}s: {type(e).__name__}: {e}")
        report_status(job_id, task_id, "error", str(e))
        raise
