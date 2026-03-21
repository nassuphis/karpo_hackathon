"""
TIFF compatibility Lambda — converts tiled bilevel TIFF to strip-based
for macOS Preview compatibility.

Post-render operation: downloads the tiled TIFF, runs tiff_compat,
uploads the strip-based file, returns presigned URL.
"""
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
TIFF_COMPAT = os.path.join(os.path.dirname(__file__), "tiff_compat")
PRESIGN_EXPIRY = 3600


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    source_key = params["source_key"]
    compat_key = params.get("compat_key",
                            source_key.replace(".tif", "_compat.tif"))
    task_id = "tiff_compat"

    try:
        report_status(job_id, task_id, "started")

        # Download source tiled TIFF
        in_path = "/tmp/source.tif"
        t0 = time.time()
        obj = s3.get_object(Bucket=BUCKET, Key=source_key)
        with open(in_path, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)
        dl_ms = int((time.time() - t0) * 1000)

        report_status(job_id, task_id, "converting")

        # Convert tiled → strip-based
        out_path = "/tmp/compat.tif"
        t1 = time.time()
        result = subprocess.run(
            [TIFF_COMPAT, in_path, out_path],
            capture_output=True, text=True, timeout=600
        )
        if result.returncode != 0:
            raise RuntimeError(f"tiff_compat failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        convert_ms = int((time.time() - t1) * 1000)

        os.remove(in_path)

        # Upload compatibility TIFF
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=compat_key,
                          Body=f, ContentType="image/tiff")
        os.remove(out_path)

        # Generate presigned URL
        url = s3.generate_presigned_url(
            "get_object",
            Params={"Bucket": BUCKET, "Key": compat_key},
            ExpiresIn=PRESIGN_EXPIRY
        )

        report_status(job_id, task_id, "done")
        return ok_response({
            "compat_key": compat_key,
            "url": url,
            "file_size": meta.get("file_size", 0),
            "dl_ms": dl_ms,
            "convert_ms": convert_ms,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        for p in ["/tmp/source.tif", "/tmp/compat.tif"]:
            try:
                os.remove(p)
            except OSError:
                pass
        raise
