"""
Render preview Lambda — generates a 1024px thumbnail PNG from any S3 image.

Dedicated Lambda with max memory/tmp for large images (e.g. 1.2GB JPEG).
Uses vipsthumbnail from the libvips layer for efficient shrink-on-load.

Invoked async via dispatch, reports status to DynamoDB.
"""
import json
import os
import resource
import subprocess
import time

import boto3

from shared import BUCKET, PRESIGN_EXPIRY, parse_body, ok_response, imgpipe_env, report_status

s3 = boto3.client("s3")


def _fmt_size(n):
    if n < 1024:
        return f"{n} B"
    if n < 1024 * 1024:
        return f"{n / 1024:.0f} KB"
    if n < 1024 * 1024 * 1024:
        return f"{n / (1024 * 1024):.1f} MB"
    return f"{n / (1024 * 1024 * 1024):.2f} GB"


def _get_runtime_info():
    """Get Lambda runtime environment info."""
    info = {}
    # Memory from Lambda environment
    mem_limit = os.environ.get("AWS_LAMBDA_FUNCTION_MEMORY_SIZE", "?")
    info["mem_limit_mb"] = int(mem_limit) if mem_limit != "?" else 0
    # CPU count
    try:
        info["cpus"] = os.cpu_count() or 1
    except Exception:
        info["cpus"] = 1
    # /tmp disk space
    try:
        st = os.statvfs("/tmp")
        info["tmp_total_mb"] = (st.f_blocks * st.f_frsize) // (1024 * 1024)
        info["tmp_free_mb"] = (st.f_bavail * st.f_frsize) // (1024 * 1024)
    except Exception:
        pass
    return info


def _get_mem_usage_mb():
    """Get current process RSS in MB."""
    try:
        # resource.getrusage returns KB on Linux
        ru = resource.getrusage(resource.RUSAGE_CHILDREN)
        return ru.ru_maxrss // 1024  # KB → MB on Linux
    except Exception:
        return 0


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

    runtime = _get_runtime_info()
    progress = {"runtime": runtime, "source_key": source_key}

    try:
        report_status(job_id, task_id, "started", result_data=progress)

        # Download source image with progress
        t0 = time.time()
        source_size = 0
        obj = s3.get_object(Bucket=BUCKET, Key=source_key)
        content_length = obj.get("ContentLength", 0)
        with open(in_path, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)
                source_size += len(chunk)
        dl_ms = int((time.time() - t0) * 1000)
        dl_speed = source_size / (dl_ms / 1000) if dl_ms > 0 else 0

        progress.update({
            "source_size": source_size,
            "source_size_human": _fmt_size(source_size),
            "dl_ms": dl_ms,
            "dl_speed": f"{_fmt_size(int(dl_speed))}/s",
            "phase": "vipsthumbnail",
        })
        report_status(job_id, task_id, "generating", result_data=progress)

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
        gen_ms = int((time.time() - t1) * 1000)
        vips_mem_mb = _get_mem_usage_mb()

        if result.returncode != 0:
            raise RuntimeError(f"vipsthumbnail failed (rc={result.returncode}): {result.stderr.strip()}")

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
        t2 = time.time()
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=preview_key,
                          Body=f, ContentType="image/png",
                          Metadata=img_meta)
        upload_ms = int((time.time() - t2) * 1000)

        total_ms = int((time.time() - t0) * 1000)

        result_data = {
            "preview_key": preview_key,
            "source_size": source_size,
            "source_size_human": _fmt_size(source_size),
            "preview_size": preview_size,
            "preview_size_human": _fmt_size(preview_size),
            "dl_ms": dl_ms,
            "dl_speed": f"{_fmt_size(int(dl_speed))}/s",
            "gen_ms": gen_ms,
            "upload_ms": upload_ms,
            "total_ms": total_ms,
            "vips_mem_mb": vips_mem_mb,
            "runtime": runtime,
        }
        if img_meta.get("width"):
            result_data["source_dims"] = f"{img_meta['width']}x{img_meta['height']}"

        report_status(job_id, task_id, "done", result_data=result_data)

        return ok_response(result_data)

    except Exception as e:
        progress["error"] = str(e)
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        for p in [in_path, out_path]:
            try:
                os.remove(p)
            except OSError:
                pass
