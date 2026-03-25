"""
Render preview Lambda — generates a 1024px thumbnail PNG from any S3 image.

Dedicated Lambda with max memory/tmp for large images (e.g. 1.2GB JPEG).
Uses vipsthumbnail from the libvips layer for efficient shrink-on-load.

Invoked async via dispatch, reports status to DynamoDB.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, PRESIGN_EXPIRY, parse_body, ok_response, imgpipe_env, report_status

s3 = boto3.client("s3")


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    source_key = params["source_key"]
    preview_key = params["preview_key"]
    max_size = params.get("max_size", 1024)
    task_id = params.get("task_id", "render_preview")

    ext = source_key.rsplit('.', 1)[-1].lower()
    in_path = f"/tmp/preview_in.{ext}"
    out_path = "/tmp/preview_out.png"

    try:
        report_status(job_id, task_id, "started")

        # Download source image
        t0 = time.time()
        obj = s3.get_object(Bucket=BUCKET, Key=source_key)
        with open(in_path, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)
        dl_ms = int((time.time() - t0) * 1000)
        source_size = os.path.getsize(in_path)

        report_status(job_id, task_id, "generating")

        # Generate thumbnail via vipsthumbnail (shrink-on-load for JPEG)
        vipsthumbnail = "/opt/bin/vipsthumbnail"
        env = imgpipe_env()
        env["PATH"] = "/opt/bin:" + env.get("PATH", "")
        t1 = time.time()
        result = subprocess.run(
            [vipsthumbnail, in_path, "-s", f"{max_size}x{max_size}",
             "-o", out_path + "[strip]"],
            capture_output=True, text=True, timeout=300,
            env=env,
        )
        if result.returncode != 0:
            raise RuntimeError(f"vipsthumbnail failed: {result.stderr.strip()}")
        gen_ms = int((time.time() - t1) * 1000)

        preview_size = os.path.getsize(out_path)

        # Copy width/height metadata from source if available
        img_meta = {}
        try:
            src_head = s3.head_object(Bucket=BUCKET, Key=source_key)
            src_meta = src_head.get("Metadata", {})
            if "width" in src_meta:
                img_meta["width"] = src_meta["width"]
            if "height" in src_meta:
                img_meta["height"] = src_meta["height"]
        except Exception:
            pass

        # Upload preview
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=preview_key,
                          Body=f, ContentType="image/png",
                          Metadata=img_meta)

        report_status(job_id, task_id, "done")

        return ok_response({
            "preview_key": preview_key,
            "source_size": source_size,
            "preview_size": preview_size,
            "dl_ms": dl_ms,
            "gen_ms": gen_ms,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
    finally:
        for p in [in_path, out_path]:
            try:
                os.remove(p)
            except OSError:
                pass
