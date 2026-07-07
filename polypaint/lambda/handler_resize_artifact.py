"""
Resize Artifact Lambda — derive a new immutable Color artifact from an existing
saved Color image using libvips resize functionality.

Supports two libvips-backed engines:
- thumbnail: high-level scalar-pix resize with no cropping
- resize: low-level kernel/gap controls with vertical scale fixed at 1
"""
import json
import os
import struct
import subprocess
import time
from datetime import datetime, timezone

import boto3

from color_artifact_meta import (
    inherit_associated_palette_metadata,
    load_color_artifact_head,
    split_color_artifact_metadata,
    write_color_artifact_meta_overlay,
)
from shared import BUCKET, CACHE_IMMUTABLE, parse_body, ok_response, report_status, imgpipe_env, png_dimensions_from_path

s3 = boto3.client("s3")

DEFAULT_JPEG_SUBSAMPLE = "on"
DEFAULT_PNG_COMPRESSION = 6
DEFAULT_PNG_Q = 100
DEFAULT_PNG_DITHER = 1.0
DEFAULT_PNG_BITDEPTH = 8
DEFAULT_PNG_EFFORT = 7

_VALID_ENGINES = {"thumbnail", "resize"}
_VALID_SIZE_MODES = {"both", "up", "down", "force"}
_VALID_INTENT = {"perceptual", "relative", "saturation", "absolute"}
_VALID_FAIL_ON = {"none", "truncated", "error", "warning"}
_VALID_KERNEL = {"nearest", "linear", "cubic", "mitchell", "lanczos2", "lanczos3"}
_VALID_SUBSAMPLE = {"auto", "on", "off"}
RAW_SIDECAR_METADATA_KEYS = (
    "raw_key",
    "raw_meta_key",
    "raw_channels",
    "raw_layout",
    "raw_encoding",
    "step_scores_key",
    "step_count",
    "step_scores_grid_n",
)


def _utc_now_iso():
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _parse_bool(value, default=False):
    if value in ("", None):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def _parse_int(value, default, *, minimum=None, maximum=None):
    try:
        parsed = int(float(value))
    except (TypeError, ValueError):
        parsed = default
    if minimum is not None:
        parsed = max(minimum, parsed)
    if maximum is not None:
        parsed = min(maximum, parsed)
    return parsed


def _parse_float(value, default, *, minimum=None, maximum=None):
    try:
        parsed = float(value)
    except (TypeError, ValueError):
        parsed = default
    if minimum is not None:
        parsed = max(minimum, parsed)
    if maximum is not None:
        parsed = min(maximum, parsed)
    return parsed


def _clean_string(value, default=""):
    return str(value or default).strip()


def _head_value(meta, key, default=None):
    value = meta.get(key)
    if value in ("", None):
        return default
    return value


def _sanitize_resize_params(params, source_meta, source_image_key=""):
    source_fmt = str(source_meta.get("format") or "").strip().lower()
    if source_fmt == "jpg":
        source_fmt = "jpeg"
    if source_fmt not in ("jpeg", "png"):
        source_fmt = "png" if str(source_image_key or "").lower().endswith(".png") else "jpeg"

    quality_default = _parse_int(_head_value(source_meta, "quality", 90), 90, minimum=1, maximum=100)
    subsample_default = _clean_string(_head_value(source_meta, "jpeg_subsample_mode", DEFAULT_JPEG_SUBSAMPLE), DEFAULT_JPEG_SUBSAMPLE).lower()
    if subsample_default not in _VALID_SUBSAMPLE:
        subsample_default = DEFAULT_JPEG_SUBSAMPLE

    out = {
        "engine": "thumbnail",
        "target_size": _parse_int(_head_value(source_meta, "pix", _head_value(source_meta, "width", 0)), 0, minimum=1, maximum=200000),
        "size_mode": "both",
        "linear": False,
        "no_rotate": False,
        "input_profile": "",
        "output_profile": "",
        "intent": "perceptual",
        "fail_on": "none",
        "kernel": "lanczos3",
        "gap": None,
        "format": source_fmt,
        "quality": quality_default,
        "jpeg_subsample_mode": subsample_default,
        "jpeg_optimize_coding": False,
        "jpeg_interlace": False,
        "png_compression": DEFAULT_PNG_COMPRESSION,
        "png_interlace": False,
        "png_palette": False,
        "png_Q": DEFAULT_PNG_Q,
        "png_dither": DEFAULT_PNG_DITHER,
        "png_bitdepth": DEFAULT_PNG_BITDEPTH,
        "png_effort": DEFAULT_PNG_EFFORT,
    }
    supplied = dict(params or {})
    if "crop" in supplied:
        raise RuntimeError("resize no longer accepts crop; outputs are full-image square thumbnails")
    if "vscale" in supplied:
        raise RuntimeError("resize no longer accepts vscale; outputs use square pix only")
    out.update(supplied)

    out["engine"] = _clean_string(out.get("engine"), "thumbnail").lower()
    if out["engine"] not in _VALID_ENGINES:
        raise RuntimeError(f"resize engine must be one of {sorted(_VALID_ENGINES)}, got {out['engine']!r}")

    out["target_size"] = _parse_int(out.get("target_size"), out["target_size"], minimum=1, maximum=200000)
    out["size_mode"] = _clean_string(out.get("size_mode"), "both").lower()
    if out["size_mode"] not in _VALID_SIZE_MODES:
        raise RuntimeError(f"size_mode must be one of {sorted(_VALID_SIZE_MODES)}, got {out['size_mode']!r}")

    out["linear"] = _parse_bool(out.get("linear"), False)
    out["no_rotate"] = _parse_bool(out.get("no_rotate"), False)
    out["input_profile"] = _clean_string(out.get("input_profile"), "")
    out["output_profile"] = _clean_string(out.get("output_profile"), "")

    out["intent"] = _clean_string(out.get("intent"), "perceptual").lower()
    if out["intent"] not in _VALID_INTENT:
        raise RuntimeError(f"intent must be one of {sorted(_VALID_INTENT)}, got {out['intent']!r}")

    out["fail_on"] = _clean_string(out.get("fail_on"), "none").lower()
    if out["fail_on"] not in _VALID_FAIL_ON:
        raise RuntimeError(f"fail_on must be one of {sorted(_VALID_FAIL_ON)}, got {out['fail_on']!r}")

    out["kernel"] = _clean_string(out.get("kernel"), "lanczos3").lower()
    if out["kernel"] not in _VALID_KERNEL:
        raise RuntimeError(f"kernel must be one of {sorted(_VALID_KERNEL)}, got {out['kernel']!r}")

    gap_raw = out.get("gap")
    if gap_raw in ("", None):
        out["gap"] = None
    else:
        out["gap"] = _parse_float(gap_raw, 2.0, minimum=0.0, maximum=100.0)
    out["format"] = _clean_string(out.get("format"), source_fmt).lower()
    if out["format"] == "jpg":
        out["format"] = "jpeg"
    if out["format"] not in ("jpeg", "png"):
        raise RuntimeError(f"resize output format must be jpeg or png, got {out['format']!r}")

    out["quality"] = _parse_int(out.get("quality"), quality_default, minimum=1, maximum=100)
    out["jpeg_subsample_mode"] = _clean_string(out.get("jpeg_subsample_mode"), DEFAULT_JPEG_SUBSAMPLE).lower()
    if out["jpeg_subsample_mode"] not in _VALID_SUBSAMPLE:
        raise RuntimeError(f"jpeg_subsample_mode must be one of {sorted(_VALID_SUBSAMPLE)}, got {out['jpeg_subsample_mode']!r}")
    out["jpeg_optimize_coding"] = _parse_bool(out.get("jpeg_optimize_coding"), False)
    out["jpeg_interlace"] = _parse_bool(out.get("jpeg_interlace"), False)

    out["png_compression"] = _parse_int(out.get("png_compression"), DEFAULT_PNG_COMPRESSION, minimum=0, maximum=9)
    out["png_interlace"] = _parse_bool(out.get("png_interlace"), False)
    out["png_palette"] = _parse_bool(out.get("png_palette"), False)
    out["png_Q"] = _parse_int(out.get("png_Q"), DEFAULT_PNG_Q, minimum=0, maximum=100)
    out["png_dither"] = _parse_float(out.get("png_dither"), DEFAULT_PNG_DITHER, minimum=0.0, maximum=1.0)
    out["png_bitdepth"] = _parse_int(out.get("png_bitdepth"), DEFAULT_PNG_BITDEPTH)
    if out["png_bitdepth"] not in (1, 2, 4, 8, 16):
        raise RuntimeError("png_bitdepth must be one of 1,2,4,8,16")
    out["png_effort"] = _parse_int(out.get("png_effort"), DEFAULT_PNG_EFFORT, minimum=1, maximum=10)

    return out


def _phase(job_id, task_id, status, phase, phase_label, **extra):
    report_status(job_id, task_id, status, result_data={"phase": phase, "phase_label": phase_label, **extra})


def _output_spec(out_path, params):
    if params["format"] == "jpeg":
        opts = [f"Q={params['quality']}", f"subsample_mode={params['jpeg_subsample_mode']}"]
        if params["jpeg_optimize_coding"]:
            opts.append("optimize_coding")
        if params["jpeg_interlace"]:
            opts.append("interlace")
        return f"{out_path}[{','.join(opts)}]"

    opts = [
        f"compression={params['png_compression']}",
        f"Q={params['png_Q']}",
        f"dither={params['png_dither']}",
        f"bitdepth={params['png_bitdepth']}",
        f"effort={params['png_effort']}",
    ]
    if params["png_interlace"]:
        opts.append("interlace")
    if params["png_palette"]:
        opts.append("palette")
    return f"{out_path}[{','.join(opts)}]"


def _command_for_resize(in_path, out_spec, width, height, params):
    target = float(params["target_size"])
    base_scale = target / float(max(width, height))
    cmd = [
        "/opt/bin/vips",
        "resize",
        in_path,
        out_spec,
        str(base_scale),
        "--kernel", params["kernel"],
    ]
    if params["gap"] is not None:
        cmd.extend(["--gap", str(params["gap"])])
    return cmd


def _command_for_thumbnail(in_path, out_spec, params):
    target = str(int(params["target_size"]))
    cmd = [
        "/opt/bin/vips",
        "thumbnail",
        in_path,
        out_spec,
        target,
        "--height", target,
        "--size", params["size_mode"],
        "--intent", params["intent"],
        "--fail-on", params["fail_on"],
    ]
    if params["no_rotate"]:
        cmd.append("--no-rotate")
    if params["linear"]:
        cmd.append("--linear")
    if params["input_profile"]:
        cmd.extend(["--input-profile", params["input_profile"]])
    if params["output_profile"]:
        cmd.extend(["--output-profile", params["output_profile"]])
    return cmd


def _probe_png_dims(path):
    with open(path, "rb") as fh:
        header = fh.read(24)
    if len(header) < 24 or header[:8] != b"\x89PNG\r\n\x1a\n" or header[12:16] != b"IHDR":
        raise RuntimeError("resized PNG is missing a valid IHDR header")
    return struct.unpack(">II", header[16:24])


def _probe_jpeg_dims(path):
    with open(path, "rb") as fh:
        if fh.read(2) != b"\xff\xd8":
            raise RuntimeError("resized JPEG is missing SOI marker")
        while True:
            marker = fh.read(1)
            if not marker:
                break
            while marker == b"\xff":
                marker = fh.read(1)
                if not marker:
                    break
            if not marker:
                break
            code = marker[0]
            if code in (0xD8, 0xD9):
                continue
            seg_len_bytes = fh.read(2)
            if len(seg_len_bytes) != 2:
                break
            seg_len = struct.unpack(">H", seg_len_bytes)[0]
            if seg_len < 2:
                break
            if code in (0xC0, 0xC1, 0xC2, 0xC3, 0xC5, 0xC6, 0xC7, 0xC9, 0xCA, 0xCB, 0xCD, 0xCE, 0xCF):
                data = fh.read(5)
                if len(data) != 5:
                    break
                _precision = data[0]
                height, width = struct.unpack(">HH", data[1:5])
                return width, height
            fh.seek(seg_len - 2, os.SEEK_CUR)
    raise RuntimeError("resized JPEG is missing a valid SOF marker")


def _probe_dims(path):
    with open(path, "rb") as fh:
        magic = fh.read(8)
    if magic.startswith(b"\x89PNG\r\n\x1a\n"):
        return _probe_png_dims(path)
    if magic[:2] == b"\xff\xd8":
        return _probe_jpeg_dims(path)
    raise RuntimeError("resized artifact format probe only supports JPEG and PNG outputs")


def _drop_raw_sidecar_metadata(meta):
    # Resize transforms the rendered image. Parent raw sidecars describe the
    # source pixels, not the resized child pixels.
    for key in RAW_SIDECAR_METADATA_KEYS:
        meta.pop(key, None)


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    task_id = params["task_id"]
    artifact_id = params["artifact_id"]
    source_artifact_id = params["source_artifact_id"]
    source_image_key = params["source_image_key"]

    in_path = None
    out_path = None
    preview_path = None

    progress = {
        "family": "color",
        "artifact_id": artifact_id,
        "source_artifact_id": source_artifact_id,
        "source_key": source_image_key,
        "postprocess_kind": "resize",
    }

    try:
        _phase(job_id, task_id, "started", "resize_artifact", "Resize", **progress)

        source_head = s3.head_object(Bucket=BUCKET, Key=source_image_key)
        try:
            source_meta = dict(load_color_artifact_head(s3, BUCKET, job_id, source_artifact_id).get("metadata", {}) or {})
        except Exception:
            source_meta = dict(source_head.get("Metadata", {}) or {})
        if source_meta.get("family") not in ("", "color"):
            raise RuntimeError("Selected source artifact is not a Color artifact")

        resize_params = _sanitize_resize_params(params.get("resize_params") or {}, source_meta, source_image_key)
        progress.update({
            "engine": resize_params["engine"],
            "target_size": resize_params["target_size"],
        })

        source_fmt = str(source_meta.get("format") or "").strip().lower()
        if source_fmt == "jpg":
            source_fmt = "jpeg"
        if source_fmt not in ("jpeg", "png"):
            source_fmt = "png" if str(source_image_key).lower().endswith(".png") else "jpeg"
        source_ext = "png" if source_fmt == "png" else "jpeg"
        out_ext = "png" if resize_params["format"] == "png" else "jpeg"

        width = _parse_int(source_meta.get("width"), _parse_int(source_meta.get("pix"), 0))
        height = _parse_int(source_meta.get("height"), _parse_int(source_meta.get("pix"), 0))
        if width <= 0 or height <= 0:
            raise RuntimeError("Selected Color artifact is missing valid pix metadata")
        if width != height:
            raise RuntimeError(f"Resize requires square source artifact, got {width}x{height}")

        created_at = _utc_now_iso()
        prefix = f"renders/{job_id}/color/{artifact_id}/"
        image_key = prefix + f"image.{out_ext}"
        preview_key = prefix + "preview.png"

        in_path = f"/tmp/resize_in.{source_ext}"
        out_path = f"/tmp/resize_out.{out_ext}"
        preview_path = "/tmp/resize_preview.png"

        _phase(job_id, task_id, "downloading", "download", "Download", **progress, source_size=source_head.get("ContentLength", 0))
        obj = s3.get_object(Bucket=BUCKET, Key=source_image_key)
        with open(in_path, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)

        out_spec = _output_spec(out_path, resize_params)
        if resize_params["engine"] == "thumbnail":
            cmd = _command_for_thumbnail(in_path, out_spec, resize_params)
        else:
            cmd = _command_for_resize(in_path, out_spec, width, height, resize_params)

        env = imgpipe_env()
        env["PATH"] = "/opt/bin:" + env.get("PATH", "")

        _phase(job_id, task_id, "processing", "process", "Resize", **progress)
        t0 = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600, env=env)
        process_ms = int((time.time() - t0) * 1000)
        if result.returncode != 0:
            stderr = (result.stderr or "").strip()
            raise RuntimeError(f"libvips resize failed: {stderr or 'unknown error'}")

        out_width, out_height = _probe_dims(out_path)
        if out_width != out_height:
            raise RuntimeError(f"Resize output must be square, got {out_width}x{out_height}")
        file_size = os.path.getsize(out_path)

        _phase(job_id, task_id, "preview", "preview", "Preview", **progress)
        prev_result = subprocess.run(
            ["/opt/bin/vipsthumbnail", out_path, "-s", "512x512", "-o", preview_path + "[strip]"],
            capture_output=True, text=True, timeout=120, env=env,
        )
        if prev_result.returncode != 0:
            raise RuntimeError(f"Preview generation failed: {prev_result.stderr.strip()}")

        _phase(job_id, task_id, "uploading", "upload", "Upload", **progress)
        img_meta = dict(source_meta)
        img_meta.update({
            "artifact_id": artifact_id,
            "family": "color",
            "created_at": created_at,
            "format": out_ext,
            "quality": str(resize_params["quality"]) if out_ext == "jpeg" else "",
            "derived_from_artifact_id": source_artifact_id,
            "derived_from_image_key": source_image_key,
            "postprocess_kind": "resize",
            "postprocess_profile": "libvips_resize_v1",
            "resize_params": json.dumps(resize_params, separators=(",", ":")),
            "repalette_capable": "false",
            "derivation_kind": "",
            "pix": str(out_width),
            "width": str(out_width),
            "height": str(out_height),
        })
        img_meta.update(inherit_associated_palette_metadata(source_meta))
        _drop_raw_sidecar_metadata(img_meta)
        if out_ext != "jpeg":
            img_meta.pop("jpeg_subsample_mode", None)
        image_meta, overlay_meta = split_color_artifact_metadata(img_meta)

        content_type = "image/png" if out_ext == "png" else "image/jpeg"
        with open(out_path, "rb") as fh:
            s3.upload_fileobj(
                fh, BUCKET, image_key,
                ExtraArgs={"ContentType": content_type, "Metadata": image_meta},
            )
        write_color_artifact_meta_overlay(s3, BUCKET, job_id, artifact_id, overlay_meta)
        preview_width, preview_height = png_dimensions_from_path(preview_path)
        preview_meta = {"width": str(preview_width), "height": str(preview_height)}
        if preview_width == preview_height:
            preview_meta["pix"] = str(preview_width)
        with open(preview_path, "rb") as pfh:
            s3.upload_fileobj(
                pfh, BUCKET, preview_key,
                ExtraArgs={"ContentType": "image/png", "Metadata": preview_meta, "CacheControl": CACHE_IMMUTABLE},
            )

        _phase(
            job_id,
            task_id,
            "done",
            "done",
            "Done",
            **progress,
            image_key=image_key,
            preview_key=preview_key,
            resize_debug={
                "engine": resize_params["engine"],
                "target_size": resize_params["target_size"],
                "size_mode": resize_params["size_mode"] if resize_params["engine"] == "thumbnail" else "",
                "kernel": resize_params["kernel"] if resize_params["engine"] == "resize" else "",
                "gap": resize_params["gap"] if resize_params["engine"] == "resize" else None,
                "linear": resize_params["linear"] if resize_params["engine"] == "thumbnail" else None,
                "source_dims": f"{width}x{height}",
                "out_dims": f"{out_width}x{out_height}",
                "file_size": file_size,
                "process_ms": process_ms,
                "format": out_ext,
            },
        )
        return ok_response({
            "job_id": job_id,
            "artifact_id": artifact_id,
            "family": "color",
            "image_key": image_key,
            "preview_key": preview_key,
            "file_size": file_size,
            "process_ms": process_ms,
        })

    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
    finally:
        for path in (in_path, out_path, preview_path):
            if not path:
                continue
            try:
                os.remove(path)
            except OSError:
                pass
