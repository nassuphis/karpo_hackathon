"""
Finalize Lambda handler — assembles tile-bucketed .pix files into a .raw tile.

One Lambda per 2D tile. Downloads all .pix files for this tile from all stripes,
pipes them to pixassemble (via stdin), uploads the resulting .raw file.
Reports completion status to DynamoDB for poll-based orchestration.
"""
import json
import os
import subprocess

import boto3
from botocore.exceptions import ClientError

from shared import BUCKET, parse_body, ok_response, report_status

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

    try:
        report_status(job_id, task_id, "started")

        raw_path = "/tmp/tile.raw"
        proc = subprocess.Popen(
            [PIXASSEMBLE,
             f"--tile_w={tile_w}",
             f"--tile_h={tile_h}",
             f"--output={raw_path}"],
            stdin=subprocess.PIPE)

        # Stream .pix files from all stripes into pixassemble's stdin
        piped = 0
        for s in range(n_stripes):
            key = f"renders/{job_id}/pix_{s:04d}_t{tile_idx:04d}.pix"
            try:
                obj = s3.get_object(Bucket=BUCKET, Key=key)
                for chunk in obj["Body"].iter_chunks(1024 * 1024):
                    proc.stdin.write(chunk)
                piped += 1
            except ClientError as e:
                if e.response["Error"]["Code"] == "NoSuchKey":
                    pass  # stripe had no roots in this tile
                else:
                    raise
            if s % 100 == 99:
                report_status(job_id, task_id, f"reading_{s+1}")

        proc.stdin.close()
        rc = proc.wait(timeout=120)
        if rc != 0:
            raise RuntimeError(f"pixassemble failed (rc={rc})")

        report_status(job_id, task_id, "assembled")

        # Stream upload — do not f.read() the full tile into Python memory
        raw_key = f"renders/{job_id}/tile_{tile_idx:04d}.raw"
        with open(raw_path, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, raw_key)

        raw_size = os.path.getsize(raw_path)
        os.remove(raw_path)

        report_status(job_id, task_id, "done")
        return ok_response({
            "tile_idx": tile_idx,
            "raw_key": raw_key,
            "raw_size": raw_size,
            "pix_files": piped,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
