"""
Tile-Reduce Lambda handler — merges a row-slice (tile) of two raw pixel buffers.

Uses S3 byte-range reads to download only the relevant tile portion from each input,
runs rawreduce on the tile-sized files, uploads the merged tile.

For round 0 (full .raw inputs): uses row_start/tile_rows to byte-range read.
For round 1+ (tile .raw inputs): downloads full files (row_start=0, tile_rows=full height).

Reports completion status to DynamoDB for poll-based orchestration.
"""
import json
import os
import struct
import subprocess

import boto3

from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
RAWREDUCE = os.path.join(os.path.dirname(__file__), "rawreduce")


def handler(event, context):
    params = parse_body(event)
    out_key = params["out_key"]
    # Derive job_id and task_id from out_key: "renders/{job_id}/merge_0_42_t3.raw"
    job_id = params.get("job_id") or out_key.split("/")[1]
    task_id = out_key.rsplit("/", 1)[-1].replace(".raw", "")

    try:
        left_key = params["left_key"]
        right_key = params["right_key"]
        row_start = params.get("row_start", 0)
        tile_rows = params.get("tile_rows")
        gamma = params.get("gamma", 2.2)

        report_status(job_id, task_id, "started")

        # Read header from left input to get width, height, bands
        head = s3.get_object(Bucket=BUCKET, Key=left_key, Range="bytes=0-11")["Body"].read()
        width, height, bands = struct.unpack("<III", head)

        if tile_rows is None:
            tile_rows = height  # full file mode

        left_path = "/tmp/left_tile.raw"
        right_path = "/tmp/right_tile.raw"
        out_path = "/tmp/merged_tile.raw"

        if row_start == 0 and tile_rows >= height:
            # Full file download — stream to disk one at a time to limit memory
            tile_rows = height
            tile_header = struct.pack("<III", width, tile_rows, bands)

            raw = s3.get_object(Bucket=BUCKET, Key=left_key)["Body"].read()
            with open(left_path, "wb") as f:
                f.write(tile_header)
                f.write(raw[12:])
            del raw

            raw = s3.get_object(Bucket=BUCKET, Key=right_key)["Body"].read()
            with open(right_path, "wb") as f:
                f.write(tile_header)
                f.write(raw[12:])
            del raw
        else:
            # Byte-range download (first round, slicing full .raw files)
            tile_header = struct.pack("<III", width, tile_rows, bands)
            byte_start = 12 + row_start * width * bands
            byte_end = byte_start + tile_rows * width * bands - 1
            range_str = f"bytes={byte_start}-{byte_end}"

            pixels = s3.get_object(Bucket=BUCKET, Key=left_key, Range=range_str)["Body"].read()
            with open(left_path, "wb") as f:
                f.write(tile_header)
                f.write(pixels)
            del pixels

            pixels = s3.get_object(Bucket=BUCKET, Key=right_key, Range=range_str)["Body"].read()
            with open(right_path, "wb") as f:
                f.write(tile_header)
                f.write(pixels)
            del pixels

        report_status(job_id, task_id, "tiles_read")

        # Merge via rawreduce
        cmd = [RAWREDUCE, left_path, right_path, out_path, f"--gamma={gamma}"]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        if result.returncode != 0:
            raise RuntimeError(f"rawreduce failed: {result.stderr.strip()}")

        report_status(job_id, task_id, "tiles_merged")

        # Upload merged tile
        with open(out_path, "rb") as f:
            out_data = f.read()
        s3.put_object(Bucket=BUCKET, Key=out_key, Body=out_data,
                      ContentType="application/octet-stream")

        for p in [left_path, right_path, out_path]:
            try:
                os.remove(p)
            except OSError:
                pass

        report_status(job_id, task_id, "done")
        return ok_response({
            "out_key": out_key,
            "size": len(out_data),
            "tile_rows": tile_rows,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
