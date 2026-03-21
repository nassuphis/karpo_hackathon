"""
PNG export Lambda — converts bilevel TIFF to 1-bit PNG.

Post-render operation: downloads the bilevel TIFF, runs png_export,
uploads the PNG, returns presigned URL.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status, imgpipe_env

s3 = boto3.client("s3")
PNG_EXPORT = os.path.join(os.path.dirname(__file__), "png_export")
PRESIGN_EXPIRY = 3600


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    source_key = params["source_key"]
    png_key = params.get("png_key",
                         source_key.replace(".tif", ".png"))
    task_id = "png_export"

    try:
        report_status(job_id, task_id, "started")

        # Download source TIFF
        in_path = "/tmp/source.tif"
        t0 = time.time()
        obj = s3.get_object(Bucket=BUCKET, Key=source_key)
        with open(in_path, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)
        dl_ms = int((time.time() - t0) * 1000)

        report_status(job_id, task_id, "converting")

        # Convert TIFF → PNG
        out_path = "/tmp/export.png"
        t1 = time.time()
        result = subprocess.run(
            [PNG_EXPORT, in_path, out_path],
            capture_output=True, text=True, timeout=600,
            env=imgpipe_env()
        )
        if result.returncode != 0:
            raise RuntimeError(f"png_export failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        convert_ms = int((time.time() - t1) * 1000)

        os.remove(in_path)

        # Upload PNG
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=png_key,
                          Body=f, ContentType="image/png")
        os.remove(out_path)

        url = s3.generate_presigned_url(
            "get_object",
            Params={"Bucket": BUCKET, "Key": png_key},
            ExpiresIn=PRESIGN_EXPIRY
        )

        report_status(job_id, task_id, "done")
        return ok_response({
            "png_key": png_key,
            "url": url,
            "file_size": meta.get("file_size", 0),
            "dl_ms": dl_ms,
            "convert_ms": convert_ms,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        for p in ["/tmp/source.tif", "/tmp/export.png"]:
            try:
                os.remove(p)
            except OSError:
                pass
        raise
