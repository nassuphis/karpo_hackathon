"""
Color RePalette Lambda — derive a new immutable Color artifact by reusing
persisted winner-per-pixel solve bins from an existing solve-score Color render.

This is intentionally fast: it skips solve scoring, binning, and root rasterization.
It recolors saved per-tile uint8 binmaps, invokes the existing encode Lambda to
stitch tiles into the final image, invokes the render-preview Lambda, and copies
the tile binmaps forward so the derived artifact remains repalette-capable.
"""
import json
import math
import os
import subprocess
import time
from datetime import datetime, timezone

import boto3

from palette_names import VALID_PALETTE_NAMES
from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
lambda_client = boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1"))

PIXEL_BINS_RENDER = os.path.join(os.path.dirname(__file__), "pixel_bins_render")
ENCODE_FUNCTION = os.environ.get("ENCODE_FUNCTION", "polypaint-encode")
RENDER_PREVIEW_FUNCTION = os.environ.get("RENDER_PREVIEW_FUNCTION", "polypaint-render-preview")

DEFAULT_BACKGROUND_COLOR = "000000"
DEFAULT_PIXEL_BINS_EMPTY = 255


def _utc_now_iso():
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _normalize_background_color(value):
    s = str(value or "").strip().lower()
    if s.startswith("#"):
        s = s[1:]
    if len(s) == 6 and all(ch in "0123456789abcdef" for ch in s):
        return s
    return DEFAULT_BACKGROUND_COLOR


def _parse_int(value, default=0):
    try:
        return int(float(value))
    except (TypeError, ValueError):
        return default


def _phase(job_id, task_id, status, phase, phase_label, **extra):
    report_status(job_id, task_id, status, result_data={"phase": phase, "phase_label": phase_label, **extra})


def _invoke_sync(function_name, payload):
    resp = lambda_client.invoke(
        FunctionName=function_name,
        InvocationType="RequestResponse",
        Payload=json.dumps({"body": json.dumps(payload)}),
    )
    if resp.get("FunctionError"):
        raw = resp["Payload"].read().decode("utf-8", errors="replace")
        raise RuntimeError(f"{function_name} invoke failed: {raw[:500]}")
    raw = resp["Payload"].read()
    decoded = json.loads(raw or b"{}")
    if decoded.get("statusCode") != 200:
        raise RuntimeError(f"{function_name} returned status {decoded.get('statusCode')}")
    body = decoded.get("body", {})
    return json.loads(body) if isinstance(body, str) else body


def _delete_keys(keys):
    if not keys:
        return
    for i in range(0, len(keys), 1000):
        batch = keys[i:i + 1000]
        s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": key} for key in batch]},
        )


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    task_id = params["task_id"]
    artifact_id = params["artifact_id"]
    source_artifact_id = params["source_artifact_id"]
    source_image_key = params["source_image_key"]
    new_palette = str(params["new_palette"]).strip()
    if new_palette not in VALID_PALETTE_NAMES:
        raise RuntimeError(f"Invalid palette: {new_palette}")

    temp_paths = []
    temp_raw_keys = []
    progress = {
        "family": "color",
        "artifact_id": artifact_id,
        "source_artifact_id": source_artifact_id,
        "derivation_kind": "color_repalette",
    }

    try:
        _phase(job_id, task_id, "started", "color_repalette", "RePalette", **progress)

        source_head = s3.head_object(Bucket=BUCKET, Key=source_image_key)
        source_meta = dict(source_head.get("Metadata", {}) or {})
        if source_meta.get("family") not in ("", "color"):
            raise RuntimeError("Selected source artifact is not a Color artifact")
        if source_meta.get("artifact_id") and source_meta.get("artifact_id") != source_artifact_id:
            raise RuntimeError(
                f"Source artifact mismatch: expected {source_artifact_id}, found {source_meta.get('artifact_id')}"
            )
        if source_meta.get("postprocess_kind"):
            raise RuntimeError("Post-processed Color artifacts cannot be fast-repaletted")
        color_mode = str(source_meta.get("color_mode", "") or "")
        if color_mode not in ("solve_score", "saved_palette"):
            raise RuntimeError(f"Color RePalette requires solve_score or saved_palette source, got {color_mode!r}")
        if str(source_meta.get("repalette_capable", "")).lower() != "true":
            raise RuntimeError("Selected Color artifact is not repalette-capable")

        pixel_bins_prefix = str(source_meta.get("pixel_bins_prefix", "") or "").strip()
        if not pixel_bins_prefix:
            raise RuntimeError("Selected Color artifact is missing pixel_bins_prefix")

        width = _parse_int(source_meta.get("width"), _parse_int(source_meta.get("pix"), 0))
        height = _parse_int(source_meta.get("height"), _parse_int(source_meta.get("pix"), 0))
        tile_size = _parse_int(source_meta.get("tile_size"), 2048)
        quality = _parse_int(source_meta.get("quality"), 90)
        if width <= 0 or height <= 0:
            raise RuntimeError("Selected Color artifact is missing valid width/height metadata")
        if tile_size <= 0:
            raise RuntimeError("Selected Color artifact is missing valid tile_size metadata")

        background_color = _normalize_background_color(source_meta.get("background_color"))
        pixel_bins_empty = _parse_int(source_meta.get("pixel_bins_empty"), DEFAULT_PIXEL_BINS_EMPTY)
        pixel_bins_layout = str(source_meta.get("pixel_bins_layout", "") or "tile_u8_v1")
        if pixel_bins_layout != "tile_u8_v1":
            raise RuntimeError(f"Unsupported pixel bin layout: {pixel_bins_layout!r}")

        n_cols = math.ceil(width / tile_size)
        n_rows = math.ceil(height / tile_size)
        n_tiles = n_cols * n_rows
        created_at = _utc_now_iso()
        fmt = str(source_meta.get("format", source_image_key.rsplit(".", 1)[-1].lower()) or "jpeg").lower()
        ext = "png" if fmt == "png" else "jpeg"

        prefix = f"renders/{job_id}/color/{artifact_id}/"
        image_key = prefix + f"image.{ext}"
        preview_key = prefix + "preview.png"
        new_pixel_bins_prefix = prefix + "pixel_bins/tile_"

        metadata = dict(source_meta)
        metadata.update({
            "artifact_id": artifact_id,
            "family": "color",
            "created_at": created_at,
            "palette": new_palette,
            "derived_from_artifact_id": source_artifact_id,
            "derived_from_image_key": source_image_key,
            "derivation_kind": "color_repalette",
            "repalette_capable": "true",
            "pixel_bins_prefix": new_pixel_bins_prefix,
            "pixel_bins_empty": str(pixel_bins_empty),
            "pixel_bins_layout": pixel_bins_layout,
            "background_color": background_color,
            "format": ext,
            "quality": str(quality),
        })
        metadata.pop("postprocess_kind", None)
        metadata.pop("postprocess_profile", None)
        metadata.pop("autolevels_params", None)

        _phase(job_id, task_id, "rendering", "render_tiles", f"RePalette tiles 0/{n_tiles}", **progress)
        for tile_idx in range(n_tiles):
            tile_row = tile_idx // n_cols
            tile_col = tile_idx % n_cols
            tile_w = min(tile_size, width - tile_col * tile_size)
            tile_h = min(tile_size, height - tile_row * tile_size)
            src_bin_key = f"{pixel_bins_prefix}{tile_idx:04d}.bin"
            dst_bin_key = f"{new_pixel_bins_prefix}{tile_idx:04d}.bin"
            tile_bin_path = f"/tmp/color_repalette_tile_{tile_idx:04d}.bin"
            tile_raw_path = f"/tmp/color_repalette_tile_{tile_idx:04d}.raw"
            temp_paths.extend([tile_bin_path, tile_raw_path])

            obj = s3.get_object(Bucket=BUCKET, Key=src_bin_key)
            tile_bins = obj["Body"].read()
            expected_size = tile_w * tile_h
            if len(tile_bins) != expected_size:
                raise RuntimeError(
                    f"Pixel bin tile {src_bin_key} size mismatch: got {len(tile_bins)} expected {expected_size}"
                )

            with open(tile_bin_path, "wb") as fh:
                fh.write(tile_bins)

            render = subprocess.run(
                [
                    PIXEL_BINS_RENDER,
                    tile_bin_path,
                    tile_raw_path,
                    f"--tile_w={tile_w}",
                    f"--tile_h={tile_h}",
                    f"--palette={new_palette}",
                    f"--background_color={background_color}",
                    f"--empty={pixel_bins_empty}",
                ],
                capture_output=True,
                text=True,
                timeout=300,
            )
            if render.returncode != 0:
                raise RuntimeError(f"pixel_bins_render failed: {render.stderr.strip() or 'unknown error'}")

            s3.put_object(
                Bucket=BUCKET,
                Key=dst_bin_key,
                Body=tile_bins,
                ContentType="application/octet-stream",
            )

            raw_key = prefix + f"_tmp/tile_{tile_idx:04d}.raw"
            with open(tile_raw_path, "rb") as fh:
                s3.upload_fileobj(fh, BUCKET, raw_key)
            temp_raw_keys.append(raw_key)

            for tmp_path in (tile_bin_path, tile_raw_path):
                try:
                    os.remove(tmp_path)
                except OSError:
                    pass
            if tile_idx + 1 == n_tiles or tile_idx == 0 or (tile_idx + 1) % 4 == 0:
                _phase(
                    job_id,
                    task_id,
                    "rendering",
                    "render_tiles",
                    f"RePalette tiles {tile_idx + 1}/{n_tiles}",
                    **progress,
                )

        _phase(job_id, task_id, "encoding", "encode", "Encode", **progress)
        encode_result = _invoke_sync(
            ENCODE_FUNCTION,
            {
                "out_key": image_key,
                "format": ext,
                "quality": quality,
                "metadata": metadata,
                "width": width,
                "height": height,
                "tile_grid": {
                    "n_cols": n_cols,
                    "n_rows": n_rows,
                    "tile_keys": temp_raw_keys,
                },
            },
        )

        _phase(job_id, task_id, "preview", "preview", "Preview", **progress)
        _invoke_sync(
            RENDER_PREVIEW_FUNCTION,
            {
                "job_id": job_id,
                "source_key": image_key,
                "preview_key": preview_key,
                "task_id": f"{task_id}_preview",
            },
        )

        _delete_keys(temp_raw_keys)
        temp_raw_keys = []

        _phase(
            job_id,
            task_id,
            "done",
            "done",
            "Done",
            **progress,
            image_key=image_key,
            preview_key=preview_key,
        )
        return ok_response({
            "job_id": job_id,
            "artifact_id": artifact_id,
            "family": "color",
            "image_key": image_key,
            "preview_key": preview_key,
            "file_size": encode_result.get("file_size"),
            "derivation_kind": "color_repalette",
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        _delete_keys(temp_raw_keys)
        raise
    finally:
        for path in temp_paths:
            try:
                os.remove(path)
            except OSError:
                pass
