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
    task_id = params.get("task_id", f"raster_{stripe_idx}")

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

        # Solve-score bins: download JSON, parse, pass as CLI args
        color = params.get("color", "rainbow")
        ss_bins_key = params.get("solve_score_bins_key") or params.get("solve_proximity_bins_key")
        if color in ("solve_score", "solve_proximity") and not ss_bins_key:
            raise RuntimeError(f"{color} color mode requires solve_score_bins_key")
        if ss_bins_key and color in ("solve_score", "solve_proximity"):
            ss_obj = s3.get_object(Bucket=BUCKET, Key=ss_bins_key)
            ss_data = json.loads(ss_obj["Body"].read())
            # Validate bins artifact — must have family and matching metric
            if ss_data.get("family") != "solve_score":
                raise RuntimeError(f"Bins artifact missing or wrong family: {ss_data.get('family')}")
            req_metric = params.get("solve_metric", "proximity")
            if ss_data.get("metric") != req_metric:
                raise RuntimeError(f"Bins metric mismatch: expected {req_metric}, got {ss_data.get('metric')}")
            req_q = params.get("solve_score_quantile", 0.001)
            if "clip_quantile" not in ss_data:
                raise RuntimeError("Bins artifact missing clip_quantile")
            if ss_data["clip_quantile"] != req_q:
                raise RuntimeError(f"Bins quantile mismatch: expected {req_q}, got {ss_data['clip_quantile']}")
            ss_metric = ss_data.get("metric", params.get("solve_metric", "proximity"))
            cmd.append(f"--color=solve_score")
            cmd.append(f"--solve_metric={ss_metric}")
            cmd.append(f"--solve_score_clip_lo={ss_data['clip_lo']}")
            cmd.append(f"--solve_score_clip_hi={ss_data['clip_hi']}")
            cmd.append(f"--solve_score_cuts={','.join(str(c) for c in ss_data['cuts_norm'])}")
            # Override the color arg already in cmd (was set to "solve_proximity" or "solve_score")
            cmd = [a for a in cmd if not a.startswith("--color=") or a == f"--color=solve_score"]

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
