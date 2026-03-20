"""
Raster Lambda handler — converts root data (.bin) to tile-bucketed .pix files.

Single operation: download .bin from S3, run roots2pix (tile-bucketed sparse
pixel output), upload non-empty .pix files to S3.
Reports completion status to DynamoDB for poll-based orchestration.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
ROOTS2PIX = os.path.join(os.path.dirname(__file__), "roots2pix")


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    stripe_idx = params["stripe_idx"]
    bin_key = params["bin_key"]
    n_tile_cols = params["n_tile_cols"]
    n_tile_rows = params["n_tile_rows"]
    n_tiles = n_tile_cols * n_tile_rows
    task_id = f"raster_{stripe_idx}"

    try:
        report_status(job_id, task_id, "started")

        # Download .bin from S3
        bin_path = "/tmp/stripe.bin"
        obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
        with open(bin_path, "wb") as f:
            f.write(obj["Body"].read())

        report_status(job_id, task_id, "bin_downloaded")

        # Clean stale .pix files from previous invocations (warm container reuse)
        import glob
        for stale in glob.glob("/tmp/pix_t*.pix"):
            os.remove(stale)

        # Run roots2pix — writes tile-bucketed .pix files to /tmp/pix_t*.pix
        t1 = time.time()
        cmd = [
            ROOTS2PIX, bin_path, "/tmp/pix",
            f"--width={params['width']}", f"--height={params['height']}",
            f"--tile_size={params['tile_size']}",
            f"--n_tile_cols={n_tile_cols}",
            f"--n_tile_rows={n_tile_rows}",
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
        # Write root transforms sidecar if present
        rt_chain = params.get("root_transforms", [])
        if rt_chain:
            rt_path = "/tmp/root_xforms.json"
            with open(rt_path, "w") as rtf:
                rtf.write(json.dumps(rt_chain))
            cmd.append(f"--root_xforms={rt_path}")

        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        if result.returncode != 0:
            raise RuntimeError(f"roots2pix failed: {result.stderr.strip()}")
        raster_meta = json.loads(result.stdout)
        raster_us = int((time.time() - t1) * 1e6)

        os.remove(bin_path)

        report_status(job_id, task_id, "rasterized")

        # Upload .pix files — stream from file, do NOT f.read() into memory
        uploaded = 0
        for t in range(n_tiles):
            pix_path = f"/tmp/pix_t{t:04d}.pix"
            if os.path.exists(pix_path) and os.path.getsize(pix_path) > 0:
                s3_key = f"renders/{job_id}/pix_{stripe_idx:04d}_t{t:04d}.pix"
                with open(pix_path, "rb") as fh:
                    s3.upload_fileobj(fh, BUCKET, s3_key)
                os.remove(pix_path)
                uploaded += 1

        report_status(job_id, task_id, "done")
        return ok_response({
            "stripe_idx": stripe_idx,
            "tiles_uploaded": uploaded,
            "raster_us": raster_us,
            "roots_plotted": raster_meta["roots_plotted"],
            "roots_clipped": raster_meta["roots_clipped"],
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
