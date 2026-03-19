"""
Bilevel Lambda handler — single-step bilevel render for one tile.

Downloads stripe .bin files from S3, runs bilevel C binary (projection + bitset
+ 1-bit PNG via libvips), uploads the resulting tile PNG. No intermediate RGB.

Each invocation handles one tile, processing all stripes sequentially.
Memory: one stripe .bin at a time + tile bitset (2 MB for 4096x4096).
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status, imgpipe_env

s3 = boto3.client("s3")
BILEVEL = os.path.join(os.path.dirname(__file__), "bilevel")


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    tile_idx = params["tile_idx"]
    tile_col = params["tile_col"]
    tile_row = params["tile_row"]
    tile_w = params["tile_w"]
    tile_h = params["tile_h"]
    n_stripes = params["n_stripes"]
    task_id = f"bilevel_{tile_idx}"

    try:
        report_status(job_id, task_id, "started")

        # Download stripe .bin files to /tmp (sequentially, one at a time)
        bin_paths = []
        t_dl = time.time()
        for s in range(n_stripes):
            bin_key = f"renders/{job_id}/stripe_{s}.bin"
            local_path = f"/tmp/stripe_{s}.bin"
            obj = s3.get_object(Bucket=BUCKET, Key=bin_key)
            with open(local_path, "wb") as f:
                for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                    f.write(chunk)
            bin_paths.append(local_path)
        dl_ms = int((time.time() - t_dl) * 1000)

        report_status(job_id, task_id, "downloaded")

        # Run bilevel binary
        out_path = "/tmp/tile.png"
        cmd = [
            BILEVEL,
            f"--full_w={params['full_w']}",
            f"--full_h={params['full_h']}",
            f"--tile_col={tile_col}",
            f"--tile_row={tile_row}",
            f"--tile_w={tile_w}",
            f"--tile_h={tile_h}",
            f"--tile_size={params['tile_size']}",
            f"--center_re={params['center_re']}",
            f"--center_im={params['center_im']}",
            f"--scale={params['scale']}",
            f"--degree={params['degree']}",
            f"--rotation={params.get('rotation', 0.0)}",
            f"--output={out_path}",
        ] + bin_paths

        t_render = time.time()
        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=600,
            env=imgpipe_env()
        )
        if result.returncode != 0:
            raise RuntimeError(f"bilevel failed: {result.stderr.strip()}")
        render_meta = json.loads(result.stdout)
        render_ms = int((time.time() - t_render) * 1000)

        # Clean up stripe files
        for p in bin_paths:
            try:
                os.remove(p)
            except OSError:
                pass

        report_status(job_id, task_id, "rendered")

        # Upload tile PNG
        tile_key = f"renders/{job_id}/bilevel_t{tile_idx:04d}.png"
        with open(out_path, "rb") as fh:
            s3.upload_fileobj(fh, BUCKET, tile_key)

        try:
            os.remove(out_path)
        except OSError:
            pass

        report_status(job_id, task_id, "done")
        return ok_response({
            "tile_idx": tile_idx,
            "tile_col": tile_col,
            "tile_row": tile_row,
            "pixels_set": render_meta["pixels_set"],
            "file_size": render_meta["file_size"],
            "dl_ms": dl_ms,
            "render_ms": render_ms,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        # Clean up
        for p in bin_paths if 'bin_paths' in dir() else []:
            try:
                os.remove(p)
            except OSError:
                pass
        raise
