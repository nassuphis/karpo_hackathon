"""
DeepZoom export Lambda — generates OpenSeadragon tile pyramid from TIFF.

Downloads source TIFF, runs dz_export (libvips dzsave), uploads .dzi +
tile files to S3 under deepzoom/{job_id}/{export_id}/.

Public access via bucket policy on deepzoom/ prefix (no per-object ACL).
"""
import json
import os
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor

import boto3

from shared import BUCKET, parse_body, ok_response, report_status, imgpipe_env

s3 = boto3.client("s3")
DZ_EXPORT = os.path.join(os.path.dirname(__file__), "dz_export")


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    source_key = params["source_key"]
    export_id = params.get("export_id", f"dz_{int(time.time())}")
    task_id = "deepzoom_export"

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

        report_status(job_id, task_id, "generating")

        # Run dz_export
        dz_base = "/tmp/dz/image"
        os.makedirs("/tmp/dz", exist_ok=True)
        t1 = time.time()
        result = subprocess.run(
            [DZ_EXPORT, in_path, dz_base],
            capture_output=True, text=True, timeout=600,
            env=imgpipe_env()
        )
        if result.returncode != 0:
            raise RuntimeError(f"dz_export failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        gen_ms = int((time.time() - t1) * 1000)

        os.remove(in_path)

        report_status(job_id, task_id, "uploading")

        # Upload .dzi + tile files to S3
        s3_prefix = f"deepzoom/{job_id}/{export_id}"
        dzi_path = dz_base + ".dzi"
        tiles_dir = dz_base + "_files"
        uploaded = 0

        # Collect all files to upload
        upload_tasks = []
        if os.path.exists(dzi_path):
            upload_tasks.append((dzi_path, f"{s3_prefix}/image.dzi", "application/xml"))

        if os.path.isdir(tiles_dir):
            for root, dirs, files in os.walk(tiles_dir):
                for fname in files:
                    local = os.path.join(root, fname)
                    rel = os.path.relpath(local, os.path.dirname(tiles_dir))
                    s3_key = f"{s3_prefix}/{rel}"
                    ct = "image/png" if fname.endswith(".png") else "application/octet-stream"
                    upload_tasks.append((local, s3_key, ct))

        # Parallel upload (public access via bucket policy on deepzoom/ prefix)
        def upload_one(task):
            local_path, s3_key, content_type = task
            with open(local_path, "rb") as fh:
                s3.put_object(
                    Bucket=BUCKET, Key=s3_key, Body=fh.read(),
                    ContentType=content_type)
            return 1

        t2 = time.time()
        with ThreadPoolExecutor(max_workers=50) as pool:
            results = list(pool.map(upload_one, upload_tasks))
        uploaded = sum(results)
        upload_ms = int((time.time() - t2) * 1000)

        # Write meta.json
        region = os.environ.get("AWS_REGION", "us-east-1")
        dzi_url = f"https://{BUCKET}.s3.{region}.amazonaws.com/{s3_prefix}/image.dzi"
        manifest = {
            "job_id": job_id,
            "export_id": export_id,
            "created_at": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
            "source_key": source_key,
            "dzi_key": f"{s3_prefix}/image.dzi",
            "dzi_url": dzi_url,
            "tile_prefix": f"{s3_prefix}/image_files",
            "width": meta["width"],
            "height": meta["height"],
            "tiles_uploaded": uploaded,
        }
        s3.put_object(
            Bucket=BUCKET,
            Key=f"{s3_prefix}/meta.json",
            Body=json.dumps(manifest),
            ContentType="application/json"
        )

        # Cleanup /tmp
        import shutil
        shutil.rmtree("/tmp/dz", ignore_errors=True)

        report_status(job_id, task_id, "done")
        return ok_response({
            "export_id": export_id,
            "dzi_url": dzi_url,
            "tiles_uploaded": uploaded,
            "dl_ms": dl_ms,
            "gen_ms": gen_ms,
            "upload_ms": upload_ms,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        import shutil
        shutil.rmtree("/tmp/dz", ignore_errors=True)
        try:
            os.remove("/tmp/source.tif")
        except OSError:
            pass
        raise
