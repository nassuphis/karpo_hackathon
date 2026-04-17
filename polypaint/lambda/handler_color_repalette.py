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
from datetime import datetime, timezone

import boto3
from botocore.config import Config

from color_artifact_meta import load_color_artifact_head
from palette_names import VALID_PALETTE_NAMES
from raw_sidecar import background_color_hex, build_raw_sidecar, validate_raw_sidecar
from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
LAMBDA_INVOKE_READ_TIMEOUT = int(os.environ.get("LAMBDA_INVOKE_READ_TIMEOUT", "930"))
LAMBDA_INVOKE_CONNECT_TIMEOUT = int(os.environ.get("LAMBDA_INVOKE_CONNECT_TIMEOUT", "10"))
lambda_client = boto3.client(
    "lambda",
    region_name=os.environ.get("AWS_REGION", "us-east-1"),
    config=Config(
        read_timeout=LAMBDA_INVOKE_READ_TIMEOUT,
        connect_timeout=LAMBDA_INVOKE_CONNECT_TIMEOUT,
        retries={"max_attempts": 2, "mode": "standard"},
    ),
)

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


def _load_json_key(key):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    body = obj["Body"].read()
    data = json.loads(body) if body else {}
    if not isinstance(data, dict):
        raise RuntimeError(f"Expected JSON object in {key}")
    return data


def _download_key_to_path(key, path):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    body = obj["Body"]
    with open(path, "wb") as fh:
        if hasattr(body, "iter_chunks"):
            for chunk in body.iter_chunks(chunk_size=1024 * 1024):
                fh.write(chunk)
        else:
            fh.write(body.read())


def _equalized_byte_to_palette_bin(value):
    if value <= 0:
        return DEFAULT_PIXEL_BINS_EMPTY
    bin_idx = int(((int(value) - 1) * 10) / 255)
    if bin_idx < 0:
        bin_idx = 0
    if bin_idx > 9:
        bin_idx = 9
    return bin_idx


def _write_palette_bin_tile_from_raw(raw_path, out_path, width, tile_x, tile_y, tile_w, tile_h):
    width = int(width)
    tile_x = int(tile_x)
    tile_y = int(tile_y)
    tile_w = int(tile_w)
    tile_h = int(tile_h)
    with open(raw_path, "rb") as raw_fh, open(out_path, "wb") as out_fh:
        for row in range(tile_h):
            raw_fh.seek((tile_y + row) * width + tile_x)
            row_bytes = raw_fh.read(tile_w)
            if len(row_bytes) != tile_w:
                raise RuntimeError(
                    f"greyscale raw tile read short at y={tile_y + row}: got {len(row_bytes)} bytes, expected {tile_w}"
                )
            mapped = bytearray(tile_w)
            for idx, value in enumerate(row_bytes):
                mapped[idx] = _equalized_byte_to_palette_bin(value)
            out_fh.write(mapped)


def handle_color_repalette_request(params, *, require_raw_sidecar=False):
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
    temp_copy_keys = []
    progress = {
        "family": "color",
        "artifact_id": artifact_id,
        "source_artifact_id": source_artifact_id,
        "derivation_kind": "color_repalette",
    }

    try:
        _phase(job_id, task_id, "started", "color_repalette", "RePalette", **progress)

        source_head = load_color_artifact_head(s3, BUCKET, job_id, source_artifact_id)
        source_meta = dict(source_head.get("metadata", {}) or {})
        source_image_key = str(source_head.get("image_key") or source_image_key)
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
        has_raw_sidecar = bool(str(source_meta.get("raw_key", "") or "").strip() and str(source_meta.get("raw_meta_key", "") or "").strip())
        if require_raw_sidecar and not has_raw_sidecar:
            raise RuntimeError("Recolor-from-raw requires raw_key and raw_meta_key on the source artifact")
        if str(source_meta.get("repalette_capable", "")).lower() != "true" and not has_raw_sidecar:
            raise RuntimeError("Selected Color artifact is not repalette-capable")

        pixel_bins_prefix = str(source_meta.get("pixel_bins_prefix", "") or "").strip()

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
        source_raw_key = str(source_meta.get("raw_key", "") or "").strip()
        source_raw_meta_key = str(source_meta.get("raw_meta_key", "") or "").strip()
        raw_sidecar = None
        source_raw_path = ""
        reuse_raw_sidecar = bool(source_raw_key and source_raw_meta_key)
        if reuse_raw_sidecar:
            raw_sidecar = validate_raw_sidecar(
                _load_json_key(source_raw_meta_key),
                expected_raw_key=source_raw_key,
                expected_artifact_family="color",
            )
            raw_width = _parse_int(raw_sidecar.get("width"), width)
            raw_height = _parse_int(raw_sidecar.get("height"), height)
            if raw_width != width or raw_height != height:
                raise RuntimeError(
                    f"greyscale raw dimensions mismatch: sidecar={raw_width}x{raw_height}, artifact={width}x{height}"
                )
            background_color = background_color_hex(raw_sidecar.get("background_color", background_color))
            source_raw_path = "/tmp/color_repalette_source_greyscale.raw"
            temp_paths.append(source_raw_path)
            _download_key_to_path(source_raw_key, source_raw_path)
        else:
            if not pixel_bins_prefix:
                raise RuntimeError("Selected Color artifact is missing pixel_bins_prefix")
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
        new_raw_key = prefix + "greyscale.raw"
        new_raw_meta_key = prefix + "greyscale.meta.json"
        new_meta_key = prefix + "meta.json"

        metadata = dict(source_meta)
        metadata.update({
            "artifact_id": artifact_id,
            "family": "color",
            "created_at": created_at,
            "palette": new_palette,
            "derived_from_artifact_id": source_artifact_id,
            "derived_from_image_key": source_image_key,
            "derivation_kind": "color_repalette",
            "background_color": background_color,
            "format": ext,
            "quality": str(quality),
        })
        if reuse_raw_sidecar:
            metadata.update({
                "repalette_capable": "false",
                "pixel_bins_drive_rgb": "false",
                "pixel_bins_prefix": "",
                "pixel_bins_empty": "",
                "pixel_bins_layout": "",
                "rgb_source": "raw",
                "raw_key": new_raw_key,
                "raw_meta_key": new_raw_meta_key,
            })
        else:
            metadata.update({
                "repalette_capable": "true",
                "pixel_bins_drive_rgb": "true",
                "pixel_bins_prefix": new_pixel_bins_prefix,
                "pixel_bins_empty": str(pixel_bins_empty),
                "pixel_bins_layout": pixel_bins_layout,
                "rgb_source": "pixel_bins",
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

            if reuse_raw_sidecar:
                _write_palette_bin_tile_from_raw(
                    source_raw_path,
                    tile_bin_path,
                    width,
                    tile_col * tile_size,
                    tile_row * tile_size,
                    tile_w,
                    tile_h,
                )
            else:
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

            if not reuse_raw_sidecar:
                with open(tile_bin_path, "rb") as fh:
                    s3.put_object(
                        Bucket=BUCKET,
                        Key=dst_bin_key,
                        Body=fh.read(),
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

        if reuse_raw_sidecar:
            _phase(job_id, task_id, "encoding", "sidecar", "Raw sidecar", **progress)
            with open(source_raw_path, "rb") as raw_fh:
                s3.upload_fileobj(raw_fh, BUCKET, new_raw_key)
            temp_copy_keys.append(new_raw_key)
            updated_sidecar = build_raw_sidecar(
                job_id=job_id,
                run_id=task_id,
                artifact_family="color",
                artifact_id=artifact_id,
                width=width,
                height=height,
                chain_fingerprint=raw_sidecar["chain_fingerprint"],
                score_chain=raw_sidecar["score_chain"],
                score_program=raw_sidecar["score_program"],
                clip_slots=raw_sidecar["clip_slots"],
                background_color=raw_sidecar["background_color"],
                plan_params_digest=raw_sidecar["plan_params_digest"],
                render_execution=raw_sidecar["render_execution"],
                raw_key=new_raw_key,
                image_key=image_key,
                preview_key=preview_key,
                meta_key=new_meta_key,
                created_at=created_at,
            )
            s3.put_object(
                Bucket=BUCKET,
                Key=new_raw_meta_key,
                Body=json.dumps(updated_sidecar, separators=(",", ":")).encode("utf-8"),
                ContentType="application/json",
            )
            temp_copy_keys.append(new_raw_meta_key)

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
            "raw_key": new_raw_key if reuse_raw_sidecar else "",
            "raw_meta_key": new_raw_meta_key if reuse_raw_sidecar else "",
            "file_size": encode_result.get("file_size"),
            "derivation_kind": "color_repalette",
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        _delete_keys(temp_raw_keys)
        _delete_keys(temp_copy_keys)
        raise
    finally:
        for path in temp_paths:
            try:
                os.remove(path)
            except OSError:
                pass


def handler(event, context):
    params = parse_body(event)
    return handle_color_repalette_request(params, require_raw_sidecar=False)
