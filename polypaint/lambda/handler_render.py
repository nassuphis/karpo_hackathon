"""
Render Lambda handler — converts root data (.bin) to raw pixel buffers (.raw).

Single operation: download .bin from S3, render to .raw via roots2raw, upload.
Reports completion status to DynamoDB for poll-based orchestration.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
ROOTS2RAW = os.path.join(os.path.dirname(__file__), "roots2raw")


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    stripe_idx = params["stripe_idx"]
    tile_idx = params.get("tile_idx")
    task_id = f"stripe_{stripe_idx}_t{tile_idx}" if tile_idx is not None else f"stripe_{stripe_idx}"

    try:
        report_status(job_id, task_id, "started")

        bin_key = params["bin_key"]
        bin_path = "/tmp/stripe.bin"
        raw_path = "/tmp/stripe.raw"

        # Download .bin from S3
        obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
        with open(bin_path, "wb") as f:
            f.write(obj["Body"].read())

        report_status(job_id, task_id, "bin_downloaded")

        # Render via roots2raw
        t1 = time.time()
        color_mode = params.get("color", "rainbow")
        match_mode = params.get("match", "none")
        palette = params.get("palette", "inferno")
        constant_color = params.get("constant_color", "ffffff")
        y_start = params.get("y_start", 0)
        y_height = params.get("y_height", params["height"])
        cmd = [
            ROOTS2RAW, bin_path, raw_path,
            f"--width={params['width']}", f"--height={params['height']}",
            f"--center_re={params['center_re']}", f"--center_im={params['center_im']}",
            f"--scale={params['scale']}", f"--degree={params['degree']}",
            f"--color={color_mode}", f"--match={match_mode}",
            f"--palette={palette}", f"--constant_color={constant_color}",
            f"--y_start={y_start}", f"--y_height={y_height}",
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
        if result.returncode != 0:
            raise RuntimeError(f"roots2raw failed: {result.stderr.strip()}")
        render_meta = json.loads(result.stdout)
        render_us = int((time.time() - t1) * 1e6)

        report_status(job_id, task_id, "rendered")

        # Upload .raw to S3
        if tile_idx is not None:
            s3_key = f"renders/{job_id}/stripe_{stripe_idx}_t{tile_idx}.raw"
        else:
            s3_key = f"renders/{job_id}/stripe_{stripe_idx}.raw"
        with open(raw_path, "rb") as f:
            raw_data = f.read()
        s3.put_object(Bucket=BUCKET, Key=s3_key,
                      Body=raw_data, ContentType="application/octet-stream")

        for p in [bin_path, raw_path]:
            try:
                os.remove(p)
            except OSError:
                pass

        report_status(job_id, task_id, "done")
        return ok_response({
            "stripe_idx": stripe_idx,
            "s3_key": s3_key,
            "raw_size": len(raw_data),
            "render_us": render_us,
            "roots_plotted": render_meta["roots_plotted"],
            "roots_clipped": render_meta["roots_clipped"],
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
