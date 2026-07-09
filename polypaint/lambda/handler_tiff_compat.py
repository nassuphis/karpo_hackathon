"""
TIFF compatibility Lambda — convert a bilevel TIFF into a derived immutable
bilevel compatibility TIFF artifact, or fall back to the legacy top-level
compatibility TIFF when no artifact_id is provided.
"""
import json
import os
import subprocess
import time
from datetime import datetime, timezone

import boto3

from shared import (BUCKET, parse_body, ok_response, report_status, imgpipe_env,
                    assert_render_source)

s3 = boto3.client("s3")
TIFF_COMPAT = os.path.join(os.path.dirname(__file__), "tiff_compat")
PRESIGN_EXPIRY = 3600


def _utc_now_iso():
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _stringify_meta(value):
    if value is None:
        return ""
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (list, dict)):
        return json.dumps(value, separators=(",", ":"))
    return str(value)


def _phase(job_id, task_id, status, phase, phase_label, **extra):
    report_status(job_id, task_id, status, result_data={"phase": phase, "phase_label": phase_label, **extra})


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    artifact_id = str(params.get("artifact_id") or "").strip()
    source_artifact_id = str(params.get("source_artifact_id") or "").strip()
    # code-review-27 F5: pin the source key to the job (and artifact when
    # declared) before the head/get below
    source_key = assert_render_source(params["source_key"], job_id, source_artifact_id, "source_key")
    task_id = str(params.get("task_id") or (f"tiff_compat_{artifact_id}" if artifact_id else "tiff_compat"))

    in_path = "/tmp/source.tif"
    out_path = "/tmp/compat.tif"
    preview_path = "/tmp/compat_preview.png"
    progress = {
        "family": "bilevel",
        "artifact_id": artifact_id,
        "source_artifact_id": source_artifact_id,
        "source_key": source_key,
        "postprocess_kind": "tiff_compat",
    }

    try:
        _phase(job_id, task_id, "started", "tiff_compat", "Preview-Compatible TIFF", **progress)

        try:
            source_head = s3.head_object(Bucket=BUCKET, Key=source_key)
            source_meta = dict(source_head.get("Metadata", {}) or {})
        except Exception:
            source_head = {}
            source_meta = {}

        if source_meta.get("family") not in ("", "bilevel"):
            raise RuntimeError("Selected source artifact is not a BiLevel artifact")
        if source_artifact_id and source_meta.get("artifact_id") and source_meta.get("artifact_id") != source_artifact_id:
            raise RuntimeError(
                f"Source artifact mismatch: expected {source_artifact_id}, found {source_meta.get('artifact_id')}"
            )

        _phase(job_id, task_id, "downloading", "download", "Download", **progress, source_size=source_head.get("ContentLength", 0))
        t0 = time.time()
        obj = s3.get_object(Bucket=BUCKET, Key=source_key)
        with open(in_path, "wb") as fh:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                fh.write(chunk)
        dl_ms = int((time.time() - t0) * 1000)

        _phase(job_id, task_id, "converting", "convert", "Preview-Compatible TIFF", **progress)
        t1 = time.time()
        result = subprocess.run(
            [TIFF_COMPAT, in_path, out_path],
            capture_output=True,
            text=True,
            timeout=600,
            env=imgpipe_env(),
        )
        if result.returncode != 0:
            raise RuntimeError(f"tiff_compat failed: {result.stderr.strip()}")
        meta = json.loads(result.stdout)
        convert_ms = int((time.time() - t1) * 1000)

        if artifact_id:
            prefix = f"renders/{job_id}/bilevel/{artifact_id}/"
            compat_key = prefix + "image.tif"
            preview_key = prefix + "preview.png"
        else:
            compat_key = params.get("compat_key", source_key.replace(".tif", "_compat.tif"))
            preview_key = ""

        if artifact_id:
            _phase(job_id, task_id, "preview", "preview", "Preview", **progress)
            env = imgpipe_env()
            env["PATH"] = "/opt/bin:" + env.get("PATH", "")
            prev_result = subprocess.run(
                ["/opt/bin/vipsthumbnail", out_path, "-s", "512x512", "-o", preview_path + "[strip]"],
                capture_output=True,
                text=True,
                timeout=120,
                env=env,
            )
            if prev_result.returncode != 0:
                raise RuntimeError(f"Preview generation failed: {prev_result.stderr.strip()}")

        created_at = _utc_now_iso()
        width = int(meta.get("width") or source_meta.get("pix") or source_meta.get("width") or 0)
        height = int(meta.get("height") or source_meta.get("pix") or source_meta.get("height") or 0)
        if width <= 0 or height <= 0:
            raise RuntimeError("TIFF compatibility export could not determine output pix")
        if width != height:
            raise RuntimeError(f"TIFF compatibility export requires square output, got {width}x{height}")
        img_meta = dict(source_meta)
        img_meta.update({
            "artifact_id": artifact_id or source_meta.get("artifact_id", ""),
            "family": "bilevel" if artifact_id else source_meta.get("family", ""),
            "created_at": created_at if artifact_id else source_meta.get("created_at", ""),
            "format": "tif",
            "derived_from_artifact_id": source_artifact_id if artifact_id else source_meta.get("derived_from_artifact_id", ""),
            "derived_from_image_key": source_key if artifact_id else source_meta.get("derived_from_image_key", ""),
            "postprocess_kind": "tiff_compat" if artifact_id else source_meta.get("postprocess_kind", ""),
            "postprocess_profile": "bilevel_tiff_compat_v1" if artifact_id else source_meta.get("postprocess_profile", ""),
            "pix": str(width),
            "width": str(width),
            "height": str(height),
        })
        normalized_meta = {str(k): _stringify_meta(v) for k, v in img_meta.items() if v not in ("", None)}

        _phase(job_id, task_id, "uploading", "upload", "Upload", **progress)
        with open(out_path, "rb") as fh:
            s3.upload_fileobj(
                fh,
                BUCKET,
                compat_key,
                ExtraArgs={"ContentType": "image/tiff", "Metadata": normalized_meta},
            )

        if artifact_id:
            with open(preview_path, "rb") as pfh:
                s3.upload_fileobj(
                    pfh,
                    BUCKET,
                    preview_key,
                    ExtraArgs={
                        "ContentType": "image/png",
                        "Metadata": {
                            "pix": str(width),
                            "width": str(width),
                            "height": str(height),
                        },
                    },
                )

        url = s3.generate_presigned_url(
            "get_object",
            Params={"Bucket": BUCKET, "Key": compat_key},
            ExpiresIn=PRESIGN_EXPIRY,
        )

        _phase(
            job_id,
            task_id,
            "done",
            "done",
            "Done",
            **progress,
            image_key=compat_key,
            preview_key=preview_key,
            file_size=meta.get("file_size", 0),
            width=width,
            height=height,
            bigtiff=meta.get("bigtiff"),
            convert_ms=convert_ms,
            dl_ms=dl_ms,
        )
        return ok_response({
            "artifact_id": artifact_id,
            "family": "bilevel" if artifact_id else "",
            "compat_key": compat_key,
            "image_key": compat_key,
            "preview_key": preview_key,
            "url": url,
            "file_size": meta.get("file_size", 0),
            "dl_ms": dl_ms,
            "convert_ms": convert_ms,
            "bigtiff": meta.get("bigtiff"),
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        for path in (in_path, out_path, preview_path):
            try:
                os.remove(path)
            except OSError:
                pass
