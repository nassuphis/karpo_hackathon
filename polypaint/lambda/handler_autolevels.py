"""
Autolevels Lambda — derive a new immutable color artifact from a saved color image.

Downloads the selected source image, runs the native autolevels_render binary,
generates a preview PNG, uploads the derived color artifact, and reports status
to DynamoDB for the existing Render observer.
"""
import json
import os
import subprocess
import time
from datetime import datetime, timezone

import boto3

from shared import BUCKET, parse_body, ok_response, report_status, imgpipe_env

s3 = boto3.client("s3")
AUTOLEVELS = os.path.join(os.path.dirname(__file__), "autolevels_render")
DEFAULT_BACKGROUND_COLOR = "000000"
DEFAULT_BACKGROUND_THRESHOLD = 4


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


def _normalize_background_color(value):
    s = str(value or "").strip().lower()
    if s.startswith("#"):
        s = s[1:]
    if len(s) == 6 and all(ch in "0123456789abcdef" for ch in s):
        return s
    return DEFAULT_BACKGROUND_COLOR


def _parse_background_threshold(value):
    try:
        threshold = int(float(value))
    except (TypeError, ValueError):
        threshold = DEFAULT_BACKGROUND_THRESHOLD
    if threshold < 0:
        threshold = 0
    if threshold > 255:
        threshold = 255
    return threshold


def _sanitize_params(params):
    provided = dict(params or {})
    defaults = {
        "bins": 256,
        "enable_levels": True,
        "clip_low": 0.0,
        "clip_high": 1.0,
        "enable_peak_limit": False,
        "peak_factor": 0.0,
        "enable_gamma": False,
        "gamma": 1.0,
        "enable_auto_gamma": False,
        "auto_gamma": "median",
        "target": 0.5,
        "enable_sigmoid": False,
        "sigmoid_strength": 0.0,
        "sigmoid_mid": 0.5,
        "enable_vibrance": False,
        "vibrance": 0.0,
        "enable_pooled_rgb": True,
        "pooled_rgb": 0.1,
        "quality": 90,
        "jpeg_subsample_mode": "on",
        "jpeg_optimize_coding": False,
        "jpeg_interlace": False,
        "exclude_background": True,
        "background_threshold": None,
    }
    out = dict(defaults)
    out.update(provided)
    out["bins"] = int(out["bins"])
    for key in ("clip_low", "clip_high", "peak_factor", "gamma", "target",
                "sigmoid_strength", "sigmoid_mid", "vibrance", "pooled_rgb"):
        out[key] = float(out[key])
    out["quality"] = int(out["quality"])
    out["auto_gamma"] = str(out.get("auto_gamma", "median") or "median").strip().lower()
    if out["auto_gamma"] not in ("none", "median"):
        raise RuntimeError(f"auto_gamma must be none or median, got {out['auto_gamma']!r}")
    out["enable_levels"] = bool(provided.get("enable_levels", out["enable_levels"]))
    out["enable_peak_limit"] = bool(provided.get("enable_peak_limit", abs(out["peak_factor"]) > 1e-12))
    out["enable_auto_gamma"] = bool(provided.get("enable_auto_gamma", out["auto_gamma"] != "none"))
    out["enable_gamma"] = bool(provided.get("enable_gamma", (not out["enable_auto_gamma"]) and abs(out["gamma"] - 1.0) > 1e-12))
    out["enable_sigmoid"] = bool(provided.get("enable_sigmoid", abs(out["sigmoid_strength"]) > 1e-12))
    out["enable_vibrance"] = bool(provided.get("enable_vibrance", abs(out["vibrance"]) > 1e-12))
    out["enable_pooled_rgb"] = bool(provided.get("enable_pooled_rgb", out["pooled_rgb"] > 0.0))
    if out["auto_gamma"] == "none":
        out["enable_auto_gamma"] = False
    out["jpeg_subsample_mode"] = str(out.get("jpeg_subsample_mode", "on") or "on").strip().lower()
    if out["jpeg_subsample_mode"] not in ("auto", "on", "off"):
        raise RuntimeError(f"jpeg_subsample_mode must be auto/on/off, got {out['jpeg_subsample_mode']!r}")
    out["jpeg_optimize_coding"] = bool(out.get("jpeg_optimize_coding", False))
    out["jpeg_interlace"] = bool(out.get("jpeg_interlace", False))
    out["exclude_background"] = bool(out.get("exclude_background", True))
    bg_threshold = out.get("background_threshold", None)
    out["background_threshold"] = None if bg_threshold in ("", None) else _parse_background_threshold(bg_threshold)
    if out["bins"] != 256:
        raise RuntimeError("autolevels currently supports bins=256 only")
    if out["quality"] < 1 or out["quality"] > 100:
        raise RuntimeError(f"quality must be in [1,100], got {out['quality']}")
    return out


def _effective_params(params):
    out = dict(params)
    out["peak_factor"] = out["peak_factor"] if out.get("enable_peak_limit") else 0.0
    out["gamma"] = out["gamma"] if out.get("enable_gamma") else 1.0
    out["auto_gamma"] = out["auto_gamma"] if out.get("enable_auto_gamma") else "none"
    out["sigmoid_strength"] = out["sigmoid_strength"] if out.get("enable_sigmoid") else 0.0
    out["vibrance"] = out["vibrance"] if out.get("enable_vibrance") else 0.0
    out["pooled_rgb"] = out["pooled_rgb"] if out.get("enable_pooled_rgb") else 0.0
    return out


def _phase(job_id, task_id, status, phase, phase_label, **extra):
    result_data = {"phase": phase, "phase_label": phase_label, **extra}
    report_status(job_id, task_id, status, result_data=result_data)


def _autolevel_debug_result(meta, background_color, background_threshold, exclude_background):
    return {
        "background_color": background_color,
        "background_threshold": background_threshold,
        "exclude_background": bool(exclude_background),
        "black_bin": meta.get("black_bin"),
        "white_bin": meta.get("white_bin"),
        "black": meta.get("black"),
        "white": meta.get("white"),
        "gamma": meta.get("gamma"),
        "final_stretch": meta.get("final_stretch"),
        "final_lo_bin": meta.get("final_lo_bin"),
        "final_hi_bin": meta.get("final_hi_bin"),
        "r_min_bin": meta.get("r_min_bin"),
        "r_max_bin": meta.get("r_max_bin"),
        "g_min_bin": meta.get("g_min_bin"),
        "g_max_bin": meta.get("g_max_bin"),
        "b_min_bin": meta.get("b_min_bin"),
        "b_max_bin": meta.get("b_max_bin"),
        "included_pixels": meta.get("included_pixels"),
        "excluded_pixels": meta.get("excluded_pixels"),
    }


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    task_id = params["task_id"]
    artifact_id = params["artifact_id"]
    source_artifact_id = params["source_artifact_id"]
    source_image_key = params["source_image_key"]
    autolevel_params = _sanitize_params(params.get("autolevels_params") or {})
    effective_params = _effective_params(autolevel_params)

    src_ext = source_image_key.rsplit(".", 1)[-1].lower()
    out_ext = "png" if src_ext == "png" else "jpeg"
    prefix = f"renders/{job_id}/color/{artifact_id}/"
    image_key = prefix + f"image.{out_ext}"
    preview_key = prefix + "preview.png"
    created_at = _utc_now_iso()

    in_path = f"/tmp/autolevels_in.{src_ext}"
    out_path = f"/tmp/autolevels_out.{out_ext}"
    preview_path = "/tmp/autolevels_preview.png"

    try:
        _phase(job_id, task_id, "started", "autolevels", "Autolevels", artifact_id=artifact_id, family="color")

        _phase(job_id, task_id, "downloading", "download", "Download", artifact_id=artifact_id, family="color")
        obj = s3.get_object(Bucket=BUCKET, Key=source_image_key)
        with open(in_path, "wb") as f:
            for chunk in obj["Body"].iter_chunks(chunk_size=1024 * 1024):
                f.write(chunk)

        try:
            src_head = s3.head_object(Bucket=BUCKET, Key=source_image_key)
            src_meta = dict(src_head.get("Metadata", {}) or {})
        except Exception:
            src_head = {}
            src_meta = {}

        background_color = _normalize_background_color(src_meta.get("background_color"))
        background_threshold = _parse_background_threshold(
            autolevel_params.get("background_threshold")
            if autolevel_params.get("background_threshold") is not None
            else src_meta.get("background_threshold")
        )

        cmd = [
            AUTOLEVELS,
            in_path,
            out_path,
            f"--bins={effective_params['bins']}",
            f"--enable-levels={1 if autolevel_params['enable_levels'] else 0}",
            f"--clip-low={effective_params['clip_low']}",
            f"--clip-high={effective_params['clip_high']}",
            f"--peak-factor={effective_params['peak_factor']}",
            f"--gamma={effective_params['gamma']}",
            f"--auto-gamma={effective_params['auto_gamma']}",
            f"--target={effective_params['target']}",
            f"--sigmoid-strength={effective_params['sigmoid_strength']}",
            f"--sigmoid-mid={effective_params['sigmoid_mid']}",
            f"--vibrance={effective_params['vibrance']}",
            f"--pooled-rgb={effective_params['pooled_rgb']}",
            f"--quality={effective_params['quality']}",
            f"--jpeg-subsample={effective_params['jpeg_subsample_mode']}",
            f"--background-color={background_color}",
            f"--background-threshold={background_threshold}",
            f"--exclude-background={1 if autolevel_params['exclude_background'] else 0}",
        ]
        if effective_params["jpeg_optimize_coding"]:
            cmd.append("--jpeg-optimize-coding")
        if effective_params["jpeg_interlace"]:
            cmd.append("--jpeg-interlace")

        _phase(job_id, task_id, "processing", "process", "Autolevel", artifact_id=artifact_id, family="color")
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600, env=imgpipe_env())
        if result.returncode != 0:
            stderr = (result.stderr or "").strip()
            raise RuntimeError(f"autolevels_render failed: {stderr or 'unknown error'}")
        meta = json.loads(result.stdout)

        _phase(job_id, task_id, "preview", "preview", "Preview", artifact_id=artifact_id, family="color")
        env = imgpipe_env()
        env["PATH"] = "/opt/bin:" + env.get("PATH", "")
        prev_result = subprocess.run(
            ["/opt/bin/vipsthumbnail", out_path, "-s", "512x512", "-o", preview_path + "[strip]"],
            capture_output=True, text=True, timeout=120, env=env,
        )
        if prev_result.returncode != 0:
            raise RuntimeError(f"Preview generation failed: {prev_result.stderr.strip()}")

        _phase(job_id, task_id, "uploading", "upload", "Upload", artifact_id=artifact_id, family="color")
        img_meta = dict(src_meta)
        img_meta.update({
            "artifact_id": artifact_id,
            "family": "color",
            "created_at": created_at,
            "format": out_ext,
            "quality": str(autolevel_params["quality"]),
            "derived_from_artifact_id": source_artifact_id,
            "derived_from_image_key": source_image_key,
            "postprocess_kind": "autolevels",
            "postprocess_profile": "preview_default_v1",
            "autolevels_params": json.dumps(autolevel_params, separators=(",", ":")),
            "background_color": background_color,
            "background_threshold": str(background_threshold),
            "repalette_capable": "false",
            "pixel_bins_prefix": "",
            "pixel_bins_empty": "",
            "pixel_bins_layout": "",
            "derivation_kind": "",
        })
        if "width" not in img_meta and meta.get("width") is not None:
            img_meta["width"] = str(meta["width"])
        if "height" not in img_meta and meta.get("height") is not None:
            img_meta["height"] = str(meta["height"])
        normalized_meta = {str(k): _stringify_meta(v) for k, v in img_meta.items() if v not in ("", None)}

        content_type = "image/png" if out_ext == "png" else "image/jpeg"
        with open(out_path, "rb") as fh:
            s3.upload_fileobj(
                fh, BUCKET, image_key,
                ExtraArgs={"ContentType": content_type, "Metadata": normalized_meta},
            )
        preview_meta = {}
        if "width" in normalized_meta:
            preview_meta["width"] = normalized_meta["width"]
        if "height" in normalized_meta:
            preview_meta["height"] = normalized_meta["height"]
        with open(preview_path, "rb") as pfh:
            s3.upload_fileobj(
                pfh, BUCKET, preview_key,
                ExtraArgs={"ContentType": "image/png", "Metadata": preview_meta},
            )

        _phase(
            job_id, task_id, "done", "done", "Done",
            artifact_id=artifact_id,
            family="color",
            image_key=image_key,
            source_artifact_id=source_artifact_id,
            postprocess_kind="autolevels",
            autolevel_debug=_autolevel_debug_result(
                meta,
                background_color=background_color,
                background_threshold=background_threshold,
                exclude_background=autolevel_params["exclude_background"],
            ),
        )
        return ok_response({
            "job_id": job_id,
            "artifact_id": artifact_id,
            "family": "color",
            "image_key": image_key,
            "preview_key": preview_key,
            "meta": meta,
        })

    except Exception as e:
        report_status(
            job_id, task_id, "error", str(e),
            result_data={
                "phase": "error",
                "phase_label": "Error",
                "artifact_id": artifact_id,
                "family": "color",
                "source_artifact_id": source_artifact_id,
            },
        )
        raise
    finally:
        for path in (in_path, out_path, preview_path):
            try:
                os.remove(path)
            except OSError:
                pass
