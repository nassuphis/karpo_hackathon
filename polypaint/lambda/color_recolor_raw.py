from __future__ import annotations

import json
import os
from datetime import datetime, timezone

import boto3

from color_artifact_meta import (
    load_color_artifact_head,
    split_color_artifact_metadata,
    write_color_artifact_meta_overlay,
)
from palette_names import VALID_PALETTE_NAMES
from raw_score_render import histogram_from_raw_path, render_score_raw, write_equalization_lut
from raw_sidecar import background_color_hex, build_raw_sidecar, validate_raw_sidecar
from solve_score_chain import format_solve_score_chain_display, read_solve_score_metadata
from shared import BUCKET, ok_response, report_status


s3 = boto3.client("s3")
S3_USER_METADATA_LIMIT_BYTES = 2048
DEFAULT_BACKGROUND_COLOR = "000000"
ASSOCIATED_PALETTE_METADATA_KEYS = (
    "associated_palette_mode",
    "associated_palette_id",
    "associated_palette_display_name",
    "associated_palette_image_key",
    "associated_palette_preview_key",
    "associated_palette_palette",
    "associated_palette_metric",
    "associated_palette_score_chain",
    "associated_palette_quantile",
    "associated_palette_omega",
    "associated_palette_omega_enabled",
    "associated_palette_raw_key",
    "associated_palette_raw_meta_key",
    "associated_palette_meta_key",
)


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


def _load_json_key(key):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    body = obj["Body"].read()
    data = json.loads(body) if body else {}
    if not isinstance(data, dict):
        raise RuntimeError(f"Expected JSON object in {key}")
    return data


def _load_optional_json_key(key):
    try:
        return _load_json_key(key)
    except Exception:
        return {}


def _download_key_to_path(key, path):
    obj = s3.get_object(Bucket=BUCKET, Key=key)
    body = obj["Body"]
    with open(path, "wb") as fh:
        if hasattr(body, "iter_chunks"):
            for chunk in body.iter_chunks(chunk_size=1024 * 1024):
                fh.write(chunk)
        else:
            fh.write(body.read())


def _metadata_size_bytes(meta):
    total = 0
    for key, value in (meta or {}).items():
        total += len(str(key).encode("utf-8"))
        total += len(str(value).encode("utf-8"))
    return total


def _delete_keys(keys):
    if not keys:
        return
    for i in range(0, len(keys), 1000):
        batch = keys[i:i + 1000]
        s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": key} for key in batch]},
        )


def _parse_score_chain(value):
    if value in ("", None):
        return []
    if isinstance(value, list):
        return value
    if isinstance(value, str):
        try:
            parsed = json.loads(value)
        except Exception:
            return []
        return parsed if isinstance(parsed, list) else []
    return []


def _associated_palette_display_name(chain, metric, quantile, palette):
    label = format_solve_score_chain_display(chain, legacy_quantile=quantile)
    parts = [label] if label else [str(metric or "").strip()]
    if palette:
        parts.append(str(palette))
    return " ".join(part for part in parts if part)


def _clear_associated_palette_metadata(metadata):
    for key in ASSOCIATED_PALETTE_METADATA_KEYS:
        metadata.pop(key, None)


def _histogram_for_sidecar(sidecar, raw_path, *, expected_size):
    histogram = sidecar.get("histogram")
    if isinstance(histogram, list) and len(histogram) == 256:
        normalized = [int(v) for v in histogram]
        if sum(normalized) == int(expected_size):
            return normalized
    return histogram_from_raw_path(raw_path, expected_size=expected_size)


def _source_associated_palette_spec(source_meta, job_id):
    mode = str(source_meta.get("associated_palette_mode") or "").strip()
    palette_id = str(source_meta.get("associated_palette_id") or "").strip()
    raw_key = str(source_meta.get("associated_palette_raw_key") or "").strip()
    raw_meta_key = str(source_meta.get("associated_palette_raw_meta_key") or "").strip()
    if not palette_id and not (raw_key and raw_meta_key):
        return None
    prefix = f"renders/{job_id}/palettes/{palette_id}/" if palette_id else ""
    score_meta = {
        "solve_score_chain": source_meta.get("associated_palette_score_chain", source_meta.get("solve_score_chain", "")),
        "solve_metric": source_meta.get("associated_palette_metric", source_meta.get("solve_metric", "")),
        "solve_score_quantile": source_meta.get("associated_palette_quantile", source_meta.get("solve_score_quantile", "")),
        "solve_score_omega": source_meta.get("associated_palette_omega", source_meta.get("solve_score_omega", "")),
        "solve_score_omega_enabled": source_meta.get(
            "associated_palette_omega_enabled",
            source_meta.get("solve_score_omega_enabled", ""),
        ),
    }
    score = read_solve_score_metadata("solve", score_meta, default_metric="proximity")
    return {
        "mode": mode,
        "palette_id": palette_id,
        "display_name": str(source_meta.get("associated_palette_display_name") or palette_id),
        "palette": str(source_meta.get("associated_palette_palette") or source_meta.get("palette") or "").strip(),
        "metric": str(score["metric"] or "").strip(),
        "score_chain": score["chain_json"],
        "quantile": score["quantile"],
        "omega": score["omega"],
        "omega_enabled": score["omega_enabled"],
        "raw_key": str(raw_key or (prefix + "greyscale.raw")).strip(),
        "raw_meta_key": str(raw_meta_key or (prefix + "greyscale.meta.json")).strip(),
        "image_key": str(source_meta.get("associated_palette_image_key") or (prefix + "image.jpeg")).strip(),
        "preview_key": str(source_meta.get("associated_palette_preview_key") or (prefix + "preview.png")).strip(),
        "meta_key": str(source_meta.get("associated_palette_meta_key") or (prefix + "meta.json")).strip(),
    }


def _copy_object(source_key, dest_key):
    s3.copy_object(
        Bucket=BUCKET,
        CopySource={"Bucket": BUCKET, "Key": source_key},
        Key=dest_key,
    )


def _copy_raw_object(source_key, dest_key):
    _copy_object(source_key, dest_key)


def _apply_associated_palette_metadata(metadata, palette_result):
    metadata["associated_palette_mode"] = "generated"
    metadata["associated_palette_id"] = palette_result["palette_id"]
    metadata["associated_palette_display_name"] = palette_result["display_name"]
    metadata["associated_palette_image_key"] = palette_result["image_key"]
    metadata["associated_palette_preview_key"] = palette_result["preview_key"]
    metadata["associated_palette_palette"] = palette_result["palette"]
    metadata["associated_palette_metric"] = palette_result["metric"]
    metadata["associated_palette_score_chain"] = palette_result["score_chain"]
    metadata["associated_palette_quantile"] = palette_result["quantile"]
    metadata["associated_palette_omega"] = palette_result["omega"]
    metadata["associated_palette_omega_enabled"] = palette_result["omega_enabled"]
    metadata["associated_palette_raw_key"] = palette_result["raw_key"]
    metadata["associated_palette_raw_meta_key"] = palette_result["raw_meta_key"]
    metadata["associated_palette_meta_key"] = palette_result["meta_key"]


def _recolor_associated_palette(
    *,
    job_id,
    task_id,
    artifact_id,
    source_meta,
    palette_spec,
    new_palette,
    background_color,
    created_at,
    temp_paths,
    temp_copy_keys,
):
    palette_sidecar = validate_raw_sidecar(
        _load_json_key(palette_spec["raw_meta_key"]),
        expected_raw_key=palette_spec["raw_key"],
        expected_artifact_family="palette",
    )
    width = int(palette_sidecar["width"])
    height = int(palette_sidecar["height"])
    if width != height:
        raise RuntimeError(f"Associated palette raw sidecar must be square, got {width}x{height}")
    palette_meta = _load_optional_json_key(palette_spec["meta_key"])

    palette_raw_path = "/tmp/color_repalette_palette_source_greyscale.raw"
    eq_lut_path = "/tmp/color_repalette_palette_eq_lut.bin"
    image_path = "/tmp/color_repalette_palette_image.jpeg"
    preview_path = "/tmp/color_repalette_palette_preview.png"
    temp_paths.extend([palette_raw_path, eq_lut_path, image_path, preview_path])

    _download_key_to_path(palette_spec["raw_key"], palette_raw_path)
    histogram = _histogram_for_sidecar(palette_sidecar, palette_raw_path, expected_size=width * height)
    write_equalization_lut(eq_lut_path, histogram)
    encode_meta = render_score_raw(
        raw_path=palette_raw_path,
        out_path=image_path,
        preview_path=preview_path,
        pix=width,
        eq_lut_path=eq_lut_path,
        palette=new_palette,
        background_color=background_color,
        quality=90,
    )

    palette_id = f"pal_{artifact_id}"
    prefix = f"renders/{job_id}/palettes/{palette_id}/"
    new_raw_key = prefix + "greyscale.raw"
    new_raw_meta_key = prefix + "greyscale.meta.json"
    new_image_key = prefix + "image.jpeg"
    new_preview_key = prefix + "preview.png"
    new_meta_key = prefix + "meta.json"

    with open(image_path, "rb") as out_fh:
        s3.put_object(
            Bucket=BUCKET,
            Key=new_image_key,
            Body=out_fh,
            ContentType="image/jpeg",
            Metadata={
                "pix": str(width),
                "width": str(width),
                "height": str(height),
                "palette": str(new_palette),
                "full_n": str(width),
                "times": str(int(_parse_int(palette_meta.get("times"), 1))),
                "using_pass": str(int(_parse_int(palette_meta.get("using_pass"), 0))),
            },
        )
    temp_copy_keys.append(new_image_key)
    with open(preview_path, "rb") as preview_fh:
        s3.put_object(
            Bucket=BUCKET,
            Key=new_preview_key,
            Body=preview_fh,
            ContentType="image/png",
        )
    temp_copy_keys.append(new_preview_key)

    _copy_raw_object(palette_spec["raw_key"], new_raw_key)
    temp_copy_keys.append(new_raw_key)
    new_sidecar = build_raw_sidecar(
        job_id=job_id,
        run_id=task_id,
        artifact_family="palette",
        artifact_id=palette_id,
        width=width,
        height=height,
        chain_fingerprint=palette_sidecar["chain_fingerprint"],
        score_chain=palette_sidecar["score_chain"],
        score_program=palette_sidecar["score_program"],
        clip_slots=palette_sidecar["clip_slots"],
        score_output_normalize=palette_sidecar.get("score_output_normalize", False),
        score_output_clip_lo=palette_sidecar.get("score_output_clip_lo", 0.0),
        score_output_clip_hi=palette_sidecar.get("score_output_clip_hi", 1.0),
        background_color=palette_sidecar["background_color"],
        plan_params_digest=palette_sidecar["plan_params_digest"],
        render_execution=palette_sidecar["render_execution"],
        raw_key=new_raw_key,
        image_key=new_image_key,
        preview_key=new_preview_key,
        meta_key=new_meta_key,
        created_at=created_at,
        histogram=histogram,
    )
    s3.put_object(
        Bucket=BUCKET,
        Key=new_raw_meta_key,
        Body=json.dumps(new_sidecar, separators=(",", ":")).encode("utf-8"),
        ContentType="application/json",
    )
    temp_copy_keys.append(new_raw_meta_key)

    display_name = _associated_palette_display_name(
        _parse_score_chain(palette_spec["score_chain"]),
        palette_spec["metric"],
        palette_spec["quantile"],
        new_palette,
    )
    meta_body = {
        "job_id": job_id,
        "palette_id": palette_id,
        "created_at": created_at,
        "display_name": display_name or palette_id,
        "palette": str(new_palette),
        "degree": int(_parse_int(palette_meta.get("degree"), _parse_int(source_meta.get("degree"), 0))),
        "N": width,
        "times": int(_parse_int(palette_meta.get("times"), 1)),
        "using_pass": int(_parse_int(palette_meta.get("using_pass"), 0)),
        "image_pass": int(_parse_int(palette_meta.get("image_pass"), 0)),
        "base_grid_solves": int(_parse_int(palette_meta.get("base_grid_solves"), width * height)),
        "total_solves": int(_parse_int(palette_meta.get("total_solves"), width * height)),
        "pass_count": int(_parse_int(palette_meta.get("pass_count"), _parse_int(palette_meta.get("times"), 1))),
        "data_layout": str(palette_meta.get("data_layout") or "fused_pass0_raw_v1"),
        "render_reusable": False,
        "file_size": int(encode_meta["file_size"]),
        "image_key": new_image_key,
        "preview_key": new_preview_key,
        "raw_key": new_raw_key,
        "raw_meta_key": new_raw_meta_key,
        "metric": str(palette_spec["metric"] or ""),
        "solve_score_chain": palette_spec["score_chain"],
        "chain_fingerprint": palette_sidecar["chain_fingerprint"],
        "derived_from_color_artifact_id": str(artifact_id),
        "derivation_kind": "color_repalette",
        "assemble_ms": 0,
        "render_ms": 0,
        "encode_ms": 0,
        "file_size_bytes": int(encode_meta["file_size"]),
        "nonzero_pixels": int(sum(histogram[1:])),
        "background_pixels": int(histogram[0]),
    }
    s3.put_object(
        Bucket=BUCKET,
        Key=new_meta_key,
        Body=json.dumps(meta_body, separators=(",", ":")).encode("utf-8"),
        ContentType="application/json",
    )
    temp_copy_keys.append(new_meta_key)

    return {
        "palette_id": palette_id,
        "display_name": display_name or palette_id,
        "palette": str(new_palette),
        "metric": str(palette_spec["metric"] or ""),
        "score_chain": palette_spec["score_chain"],
        "quantile": palette_spec["quantile"],
        "omega": palette_spec["omega"],
        "omega_enabled": palette_spec["omega_enabled"],
        "image_key": new_image_key,
        "preview_key": new_preview_key,
        "raw_key": new_raw_key,
        "raw_meta_key": new_raw_meta_key,
        "meta_key": new_meta_key,
        "file_size": int(encode_meta["file_size"]),
    }


def handle_color_recolor_from_raw_request(params, *, source_head=None, already_started=False):
    job_id = params["job_id"]
    task_id = params["task_id"]
    artifact_id = params["artifact_id"]
    source_artifact_id = params["source_artifact_id"]
    source_image_key = params["source_image_key"]
    new_palette = str(params["new_palette"]).strip()
    if new_palette not in VALID_PALETTE_NAMES:
        raise RuntimeError(f"Invalid palette: {new_palette}")

    temp_paths = []
    temp_copy_keys = []
    progress = {
        "family": "color",
        "artifact_id": artifact_id,
        "source_artifact_id": source_artifact_id,
        "derivation_kind": "color_repalette",
    }

    try:
        if not already_started:
            _phase(job_id, task_id, "started", "color_repalette", "RePalette", **progress)

        if source_head is None:
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

        source_raw_key = str(source_meta.get("raw_key", "") or "").strip()
        source_raw_meta_key = str(source_meta.get("raw_meta_key", "") or "").strip()
        if not (source_raw_key and source_raw_meta_key):
            raise RuntimeError("Recolor-from-raw requires raw_key and raw_meta_key on the source artifact")

        width = _parse_int(source_meta.get("width"), _parse_int(source_meta.get("pix"), 0))
        height = _parse_int(source_meta.get("height"), _parse_int(source_meta.get("pix"), 0))
        quality = _parse_int(source_meta.get("quality"), 90)
        if width <= 0 or height <= 0:
            raise RuntimeError("Selected Color artifact is missing valid width/height metadata")
        if width != height:
            raise RuntimeError(f"Recolor-from-raw requires square source artifact, got {width}x{height}")

        background_color = _normalize_background_color(source_meta.get("background_color"))
        raw_sidecar = validate_raw_sidecar(
            _load_json_key(source_raw_meta_key),
            expected_raw_key=source_raw_key,
            expected_artifact_family="color",
            require_scalar=True,
            feature="Recolor-from-raw",
        )
        raw_width = _parse_int(raw_sidecar.get("width"), width)
        raw_height = _parse_int(raw_sidecar.get("height"), height)
        if raw_width != width or raw_height != height:
            raise RuntimeError(
                f"greyscale raw dimensions mismatch: sidecar={raw_width}x{raw_height}, artifact={width}x{height}"
            )
        if raw_width != raw_height:
            raise RuntimeError(f"greyscale raw sidecar must be square, got {raw_width}x{raw_height}")
        background_color = background_color_hex(raw_sidecar.get("background_color", background_color))

        created_at = _utc_now_iso()
        fmt = str(source_meta.get("format", source_image_key.rsplit(".", 1)[-1].lower()) or "jpeg").lower()
        ext = "png" if fmt == "png" else "jpeg"

        prefix = f"renders/{job_id}/color/{artifact_id}/"
        image_key = prefix + f"image.{ext}"
        preview_key = prefix + "preview.png"
        new_raw_key = prefix + "greyscale.raw"
        new_raw_meta_key = prefix + "greyscale.meta.json"
        new_meta_key = prefix + "meta.json"
        new_step_scores_key = prefix + "step_scores.raw"

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
            "repalette_capable": "false",
            "rgb_source": "raw",
            "raw_key": new_raw_key,
            "raw_meta_key": new_raw_meta_key,
        })
        metadata.pop("postprocess_kind", None)
        metadata.pop("postprocess_profile", None)
        metadata.pop("autolevels_params", None)
        metadata.pop("step_scores_key", None)
        metadata.pop("step_count", None)
        metadata.pop("step_scores_grid_n", None)
        _clear_associated_palette_metadata(metadata)
        source_palette_spec = _source_associated_palette_spec(source_meta, job_id)

        source_raw_path = "/tmp/color_repalette_source_greyscale.raw"
        eq_lut_path = "/tmp/color_repalette_eq_lut.bin"
        encode_out_path = f"/tmp/color_repalette_image.{ext}"
        preview_out_path = "/tmp/color_repalette_preview.png"
        temp_paths.extend([source_raw_path, eq_lut_path, encode_out_path, preview_out_path])
        _download_key_to_path(source_raw_key, source_raw_path)

        _phase(job_id, task_id, "rendering", "render_raw", "RePalette raw", **progress)
        histogram = _histogram_for_sidecar(raw_sidecar, source_raw_path, expected_size=width * height)
        write_equalization_lut(eq_lut_path, histogram)
        encode_result = render_score_raw(
            raw_path=source_raw_path,
            out_path=encode_out_path,
            preview_path=preview_out_path,
            pix=width,
            eq_lut_path=eq_lut_path,
            palette=new_palette,
            background_color=background_color,
            quality=quality,
        )

        associated_palette_result = None
        if source_palette_spec:
            associated_palette_result = _recolor_associated_palette(
                job_id=job_id,
                task_id=task_id,
                artifact_id=artifact_id,
                source_meta=source_meta,
                palette_spec=source_palette_spec,
                new_palette=new_palette,
                background_color=background_color,
                created_at=created_at,
                temp_paths=temp_paths,
                temp_copy_keys=temp_copy_keys,
            )
            _apply_associated_palette_metadata(metadata, associated_palette_result)

        _phase(job_id, task_id, "encoding", "sidecar", "Raw sidecar", **progress)
        _copy_raw_object(source_raw_key, new_raw_key)
        temp_copy_keys.append(new_raw_key)
        copied_step_scores_key = ""
        copied_step_count = None
        copied_step_scores_grid_n = None
        if raw_sidecar.get("step_scores_key"):
            _copy_object(raw_sidecar["step_scores_key"], new_step_scores_key)
            temp_copy_keys.append(new_step_scores_key)
            copied_step_scores_key = new_step_scores_key
            copied_step_count = raw_sidecar.get("step_count")
            copied_step_scores_grid_n = raw_sidecar.get("step_scores_grid_n")
            metadata["step_scores_key"] = copied_step_scores_key
            metadata["step_count"] = copied_step_count
            metadata["step_scores_grid_n"] = copied_step_scores_grid_n
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
            score_output_normalize=raw_sidecar.get("score_output_normalize", False),
            score_output_clip_lo=raw_sidecar.get("score_output_clip_lo", 0.0),
            score_output_clip_hi=raw_sidecar.get("score_output_clip_hi", 1.0),
            background_color=raw_sidecar["background_color"],
            plan_params_digest=raw_sidecar["plan_params_digest"],
            render_execution=raw_sidecar["render_execution"],
            raw_key=new_raw_key,
            image_key=image_key,
            preview_key=preview_key,
            meta_key=new_meta_key,
            created_at=created_at,
            histogram=histogram,
            step_scores_key=copied_step_scores_key or None,
            step_count=copied_step_count,
            step_scores_grid_n=copied_step_scores_grid_n,
        )
        s3.put_object(
            Bucket=BUCKET,
            Key=new_raw_meta_key,
            Body=json.dumps(updated_sidecar, separators=(",", ":")).encode("utf-8"),
            ContentType="application/json",
        )
        temp_copy_keys.append(new_raw_meta_key)

        _phase(job_id, task_id, "encoding", "upload", "Upload image", **progress)
        image_meta, overlay_meta = split_color_artifact_metadata(metadata)
        final_headers = {"pix": str(width), "width": str(width), "height": str(height), **image_meta}
        metadata_size = _metadata_size_bytes(final_headers)
        if metadata_size > S3_USER_METADATA_LIMIT_BYTES:
            raise RuntimeError(
                f"image metadata too large before upload: {metadata_size} bytes > {S3_USER_METADATA_LIMIT_BYTES} limit"
            )
        content_type = "image/png" if ext == "png" else "image/jpeg"
        with open(encode_out_path, "rb") as out_fh:
            s3.put_object(
                Bucket=BUCKET,
                Key=image_key,
                Body=out_fh,
                ContentType=content_type,
                Metadata=final_headers,
            )
        temp_copy_keys.append(image_key)
        with open(preview_out_path, "rb") as preview_fh:
            s3.put_object(
                Bucket=BUCKET,
                Key=preview_key,
                Body=preview_fh,
                ContentType="image/png",
            )
        temp_copy_keys.append(preview_key)
        write_color_artifact_meta_overlay(s3, BUCKET, job_id, artifact_id, overlay_meta)
        temp_copy_keys.append(new_meta_key)

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
            "raw_key": new_raw_key,
            "raw_meta_key": new_raw_meta_key,
            "file_size": encode_result.get("file_size"),
            "derivation_kind": "color_repalette",
            **({"associated_palette": associated_palette_result} if associated_palette_result else {}),
        })

    except Exception as exc:
        report_status(job_id, task_id, "error", str(exc), result_data=progress)
        _delete_keys(temp_copy_keys)
        raise
    finally:
        for path in temp_paths:
            try:
                os.remove(path)
            except OSError:
                pass
