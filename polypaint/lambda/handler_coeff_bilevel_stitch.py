"""
Coeff-bilevel stitch Lambda handler — joins coeff tile TIFFs into the final image TIFF.

Separate Lambda from coeff-bilevel raster/merge so it can be sized independently
(more memory = more vCPUs for libvips multithreaded encode).

Downloads all tile TIFFs from S3, runs bilevel_merge stitch
(vips_arrayjoin → vips_tiffsave CCITT G4), uploads final TIFF.
"""
import json
import os
import re
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status, imgpipe_env

s3 = boto3.client("s3")
BILEVEL_MERGE = os.path.join(os.path.dirname(__file__), "bilevel_merge")
MAX_STITCH_S3_METADATA_BYTES = 1800
_STITCH_UPLOAD_METADATA_LIMITS = {
    "artifact_id": 128,
    "family": 32,
    "created_at": 64,
    "format": 16,
    "mode": 32,
    "width": 16,
    "height": 16,
    "degree": 32,
    "pix": 32,
    "tile_size": 32,
    "view_mode": 32,
    "quantile": 64,
    "shim": 64,
    "square_extent": 64,
    "min_re": 64,
    "max_re": 64,
    "min_im": 64,
    "max_im": 64,
    "rotation": 64,
    "root_transforms": 512,
    "render_execution": 512,
}
_STITCH_FORWARD_METADATA_KEYS = (
    "artifact_id",
    "family",
    "created_at",
    "format",
    "mode",
    "degree",
    "pix",
    "tile_size",
    "view_mode",
    "quantile",
    "shim",
    "square_extent",
    "min_re",
    "max_re",
    "min_im",
    "max_im",
    "rotation",
    "root_transforms",
    "render_execution",
)


def _metadata_str_value(value):
    if value in ("", None):
        return None
    if isinstance(value, (dict, list)):
        return json.dumps(value, separators=(",", ":"))
    return str(value)


def _set_stitch_upload_metadata_value(target, key, value):
    text = _metadata_str_value(value)
    if text in (None, ""):
        return
    limit = _STITCH_UPLOAD_METADATA_LIMITS.get(str(key))
    if limit is None:
        raise RuntimeError(f"unsupported stitch upload metadata key: {key}")
    encoded = text.encode("utf-8")
    if len(encoded) > int(limit):
        raise RuntimeError(f"stitch upload metadata {key} exceeds {limit} bytes")
    target[str(key)] = text


def _build_stitch_upload_metadata(*, metadata, width, height):
    upload_meta = {}
    metadata = dict(metadata or {})
    for key in _STITCH_FORWARD_METADATA_KEYS:
        if key in metadata:
            _set_stitch_upload_metadata_value(upload_meta, key, metadata.get(key))
    _set_stitch_upload_metadata_value(upload_meta, "width", width)
    _set_stitch_upload_metadata_value(upload_meta, "height", height)
    total_metadata_bytes = sum(len(k.encode("utf-8")) + len(v.encode("utf-8")) for k, v in upload_meta.items())
    if total_metadata_bytes > MAX_STITCH_S3_METADATA_BYTES:
        raise RuntimeError(
            f"stitch upload metadata exceeds {MAX_STITCH_S3_METADATA_BYTES} bytes ({total_metadata_bytes} bytes)"
        )
    return upload_meta


def _extract_handler_entry_params(event):
    params = {}
    if not isinstance(event, dict):
        return params
    body = event.get("body")
    if isinstance(body, dict):
        for key in ("job_id", "task_id"):
            value = body.get(key)
            if value not in ("", None):
                params[key] = value
        return params
    if isinstance(body, str):
        for key in ("job_id", "task_id"):
            match = re.search(rf'"{re.escape(key)}"\s*:\s*"([^"]*)"', body)
            if match and match.group(1) not in ("", None):
                params[key] = match.group(1)
    for key in ("job_id", "task_id"):
        if key not in params:
            value = event.get(key)
            if value not in ("", None):
                params[key] = value
    return params


def _report_handler_entry_error(params, message):
    job_id = str((params or {}).get("job_id") or "").strip()
    if not job_id:
        return
    task_id = str((params or {}).get("task_id") or "coeff_bilevel_stitch").strip() or "coeff_bilevel_stitch"
    report_status(
        job_id,
        task_id,
        "error",
        message,
        result_data={"phase": "handler_entry", "phase_label": "Tile stitch"},
    )


def handler(event, context):
    try:
        params = parse_body(event)
    except Exception as e:
        params = _extract_handler_entry_params(event)
        message = f"Coeff-bilevel stitch handler could not parse request body: {e}"
        _report_handler_entry_error(params, message)
        raise RuntimeError(message) from e
    job_id = params["job_id"]
    n_tile_cols = params["n_tile_cols"]
    n_tile_rows = params["n_tile_rows"]
    n_tiles = n_tile_cols * n_tile_rows
    out_key = params["out_key"]
    preview_key = params.get("preview_key")
    tile_prefix = params.get("tile_prefix", "bilevel")
    task_id = str(params.get("task_id") or "coeff_bilevel_stitch").strip() or "coeff_bilevel_stitch"

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
        preview_path = "/tmp/final_preview.png"
        full_w = params.get("width", n_tile_cols * params.get("tile_size", 4096))
        full_h = params.get("height", n_tile_rows * params.get("tile_size", 4096))
        tile_sz = params.get("tile_size", 4096)
        cmd = [
            BILEVEL_MERGE, "stitch",
            f"--n_cols={n_tile_cols}",
            f"--n_rows={n_tile_rows}",
            f"--width={full_w}",
            f"--height={full_h}",
            f"--tile_size={tile_sz}",
            f"--output={out_path}",
            f"--preview={preview_path}",
            "--preview_size=1024",
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

        # Upload final TIFF with bounded caller-supplied artifact metadata.
        image_metadata = _build_stitch_upload_metadata(
            metadata=params.get("metadata"),
            width=full_w,
            height=full_h,
        )
        with open(out_path, "rb") as f:
            s3.put_object(Bucket=BUCKET, Key=out_key, Body=f, ContentType="image/tiff",
                          Metadata=image_metadata)
        os.remove(out_path)

        # Upload preview PNG
        preview_key = preview_key or out_key.replace('.tif', '_preview.png')
        preview_url = ""
        if os.path.exists(preview_path):
            with open(preview_path, "rb") as f:
                s3.put_object(Bucket=BUCKET, Key=preview_key, Body=f, ContentType="image/png")
            os.remove(preview_path)

        report_status(job_id, task_id, "done")
        return ok_response({
            "out_key": out_key,
            "preview_key": preview_key,
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
