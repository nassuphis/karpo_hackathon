"""
Bilevel stitch Lambda handler — joins tile TIFFs into final image TIFF.

Separate Lambda from bilevel raster/merge so it can be sized independently
(more memory = more vCPUs for libvips multithreaded encode).

Downloads all tile TIFFs from S3, runs bilevel_merge stitch
(vips_arrayjoin → vips_tiffsave CCITT G4), uploads final TIFF.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status, imgpipe_env

s3 = boto3.client("s3")
BILEVEL_MERGE = os.path.join(os.path.dirname(__file__), "bilevel_merge")


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    n_tile_cols = params["n_tile_cols"]
    n_tile_rows = params["n_tile_rows"]
    n_tiles = n_tile_cols * n_tile_rows
    out_key = params["out_key"]
    tile_prefix = params.get("tile_prefix", "bilevel")
    task_id = params.get("task_id", "bilevel_stitch")

    try:
        report_status(job_id, task_id, "started")

        # Download all tile TIFFs
        tile_paths = []
        t_dl = time.time()
        for t in range(n_tiles):
            tile_key = f"renders/{job_id}/{tile_prefix}_t{t:04d}.tif"
            local_path = f"/tmp/tile_{t:04d}.tif"
            obj = s3.get_object(Bucket=BUCKET, Key=tile_key)
            with open(local_path, "wb") as f:
                for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                    f.write(chunk)
            tile_paths.append(local_path)
        dl_ms = int((time.time() - t_dl) * 1000)

        report_status(job_id, task_id, "stitching")

        # Run bilevel_merge stitch (timeout 870s — 30s headroom before Lambda's 900s limit)
        out_path = "/tmp/final.tif"
        cmd = [
            BILEVEL_MERGE, "stitch",
            f"--n_cols={n_tile_cols}",
            f"--n_rows={n_tile_rows}",
            f"--output={out_path}",
        ] + tile_paths

        t_stitch = time.time()
        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=870,
            env=imgpipe_env()
        )
        if result.returncode != 0:
            raise RuntimeError(f"bilevel_merge stitch failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        stitch_ms = int((time.time() - t_stitch) * 1000)

        # Clean up tile TIFFs
        for p in tile_paths:
            try:
                os.remove(p)
            except OSError:
                pass

        # Upload final TIFF
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=out_key, Body=f, ContentType="image/tiff")
        os.remove(out_path)

        report_status(job_id, task_id, "done")
        return ok_response({
            "out_key": out_key,
            "file_size": meta.get("file_size", 0),
            "dl_ms": dl_ms,
            "stitch_ms": stitch_ms,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        # Clean up /tmp to avoid stale files on warm container reuse
        for p in tile_paths if 'tile_paths' in dir() else []:
            try:
                os.remove(p)
            except OSError:
                pass
        try:
            os.remove("/tmp/final.tif")
        except OSError:
            pass
        raise
