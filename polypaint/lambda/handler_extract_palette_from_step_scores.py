from __future__ import annotations

import json
import os
import subprocess
import time

import boto3

from color_artifact_meta import (
    load_color_artifact_head,
    split_color_artifact_metadata,
    write_color_artifact_meta_overlay,
)
from raw_score_render import histogram_from_raw_path, render_score_raw, write_equalization_lut
from raw_sidecar import background_color_hex, build_raw_sidecar, validate_raw_sidecar
from shared import BUCKET, imgpipe_env, ok_response, parse_body, report_status
from solve_score_chain import format_solve_score_chain_display


s3 = boto3.client("s3")
STEP_SCORES_TO_PALETTE_RAW = os.path.join(os.path.dirname(__file__), "step_scores_to_palette_raw")
S3_USER_METADATA_LIMIT_BYTES = 2048
ASSOCIATED_PALETTE_KEYS = (
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
VIEWPORT_METADATA_KEYS = (
    "view_mode",
    "quantile",
    "shim",
    "square_extent",
    "min_re",
    "max_re",
    "min_im",
    "max_im",
    "rotation",
)


def _utc_now_iso():
    return time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())


def _metadata_size_bytes(meta):
    total = 0
    for key, value in (meta or {}).items():
        total += len(str(key).encode("utf-8"))
        total += len(str(value).encode("utf-8"))
    return total


def _load_color_artifact(job_id, artifact_id):
    head = load_color_artifact_head(s3, BUCKET, job_id, artifact_id)
    meta = dict(head.get("metadata", {}) or {})
    meta["artifact_id"] = artifact_id
    meta["image_key"] = head.get("image_key", "")
    return meta


def _clear_associated_palette(metadata):
    for key in ASSOCIATED_PALETTE_KEYS:
        metadata.pop(key, None)


def _apply_associated_palette(metadata, palette_result, *, mode):
    metadata["associated_palette_mode"] = mode
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
    metadata["associated_palette_raw_key"] = palette_result.get("raw_key", "")
    metadata["associated_palette_raw_meta_key"] = palette_result.get("raw_meta_key", "")
    metadata["associated_palette_meta_key"] = palette_result.get("meta_key", "")


def _associated_palette_display_name(chain, metric, quantile, palette):
    label = format_solve_score_chain_display(chain, legacy_quantile=quantile)
    parts = [label] if label else [str(metric or "").strip()]
    if palette:
        parts.append(str(palette))
    return " ".join(part for part in parts if part)


def _write_overlay(job_id, artifact_id, metadata):
    _, overlay_meta = split_color_artifact_metadata(metadata)
    write_color_artifact_meta_overlay(s3, BUCKET, job_id, artifact_id, overlay_meta)
    return overlay_meta


def _copy_existing_associated_palette_fields(target_metadata, source_metadata):
    _clear_associated_palette(target_metadata)
    for key in ASSOCIATED_PALETTE_KEYS:
        value = source_metadata.get(key)
        if value not in ("", None):
            target_metadata[key] = value


def _copy_viewport_metadata(target_metadata, source_metadata):
    for key in VIEWPORT_METADATA_KEYS:
        value = source_metadata.get(key)
        if value not in ("", None):
            target_metadata[key] = value


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


def _resolve_extract_request(job_id, artifact_id):
    selected = _load_color_artifact(job_id, artifact_id)
    if selected.get("step_scores_key") and selected.get("raw_key") and selected.get("raw_meta_key"):
        return {"kind": "fused", "selected": selected, "source": selected}

    current = selected
    seen = {artifact_id}
    while True:
        if current.get("associated_palette_id"):
            kind = "done" if current["artifact_id"] == artifact_id else "attach_generated"
            return {"kind": kind, "selected": selected, "source": current}
        if current.get("color_mode") == "saved_palette" and current.get("palette_source_id"):
            return {"kind": "attach_dependency", "selected": selected, "source": current}
        if current.get("color_mode") == "solve_score":
            return {"kind": "legacy", "selected": selected, "source": current}
        parent_id = str(current.get("derived_from_artifact_id") or "").strip()
        if not parent_id or parent_id in seen:
            break
        seen.add(parent_id)
        current = _load_color_artifact(job_id, parent_id)
    return {"kind": "none", "selected": selected, "source": current}


def _attach_generated_palette(job_id, artifact_id, selected_meta, source_meta):
    if source_meta.get("artifact_id") == artifact_id:
        return {
            "palette_id": str(source_meta.get("associated_palette_id") or ""),
            "display_name": str(source_meta.get("associated_palette_display_name") or source_meta.get("associated_palette_id") or ""),
            "palette": str(source_meta.get("associated_palette_palette") or ""),
            "metric": str(source_meta.get("associated_palette_metric") or ""),
            "score_chain": source_meta.get("associated_palette_score_chain", ""),
            "quantile": source_meta.get("associated_palette_quantile", ""),
            "omega": source_meta.get("associated_palette_omega", ""),
            "omega_enabled": source_meta.get("associated_palette_omega_enabled", ""),
            "image_key": str(source_meta.get("associated_palette_image_key") or ""),
            "preview_key": str(source_meta.get("associated_palette_preview_key") or ""),
            "raw_key": str(source_meta.get("associated_palette_raw_key") or ""),
            "raw_meta_key": str(source_meta.get("associated_palette_raw_meta_key") or ""),
            "meta_key": str(source_meta.get("associated_palette_meta_key") or ""),
        }

    metadata = dict(selected_meta)
    _copy_existing_associated_palette_fields(metadata, source_meta)
    _write_overlay(job_id, artifact_id, metadata)
    return {
        "palette_id": str(metadata.get("associated_palette_id") or ""),
        "display_name": str(metadata.get("associated_palette_display_name") or metadata.get("associated_palette_id") or ""),
        "palette": str(metadata.get("associated_palette_palette") or ""),
        "metric": str(metadata.get("associated_palette_metric") or ""),
        "score_chain": metadata.get("associated_palette_score_chain", ""),
        "quantile": metadata.get("associated_palette_quantile", ""),
        "omega": metadata.get("associated_palette_omega", ""),
        "omega_enabled": metadata.get("associated_palette_omega_enabled", ""),
        "image_key": str(metadata.get("associated_palette_image_key") or ""),
        "preview_key": str(metadata.get("associated_palette_preview_key") or ""),
        "raw_key": str(metadata.get("associated_palette_raw_key") or ""),
        "raw_meta_key": str(metadata.get("associated_palette_raw_meta_key") or ""),
        "meta_key": str(metadata.get("associated_palette_meta_key") or ""),
    }


def _attach_saved_palette_dependency(job_id, artifact_id, selected_meta, source_meta):
    palette_id = str(source_meta.get("palette_source_id") or "").strip()
    if not palette_id:
        raise RuntimeError("saved_palette lineage is missing palette_source_id")
    palette_meta = _load_json_key(f"renders/{job_id}/palettes/{palette_id}/meta.json")
    prefix = f"renders/{job_id}/palettes/{palette_id}/"
    palette_result = {
        "palette_id": palette_id,
        "display_name": str(
            source_meta.get("palette_source_display_name")
            or palette_meta.get("display_name")
            or palette_id
        ),
        "palette": str(source_meta.get("palette_source_palette") or palette_meta.get("palette") or ""),
        "metric": str(source_meta.get("palette_source_metric") or palette_meta.get("metric") or ""),
        "score_chain": source_meta.get("palette_source_score_chain", palette_meta.get("solve_score_chain", "")),
        "quantile": source_meta.get("palette_source_quantile", ""),
        "omega": source_meta.get("palette_source_omega", ""),
        "omega_enabled": source_meta.get("palette_source_omega_enabled", ""),
        "image_key": str(palette_meta.get("image_key") or (prefix + "image.jpeg")),
        "preview_key": str(palette_meta.get("preview_key") or (prefix + "preview.png")),
        "raw_key": str(palette_meta.get("raw_key") or (prefix + "greyscale.raw")),
        "raw_meta_key": str(palette_meta.get("raw_meta_key") or (prefix + "greyscale.meta.json")),
        "meta_key": str(prefix + "meta.json"),
    }
    metadata = dict(selected_meta)
    _clear_associated_palette(metadata)
    _apply_associated_palette(metadata, palette_result, mode="dependency")
    _write_overlay(job_id, artifact_id, metadata)
    return palette_result


def _render_palette_from_step_scores(job_id, artifact_id, source_meta, task_id):
    source_raw_key = str(source_meta.get("raw_key") or "").strip()
    source_raw_meta_key = str(source_meta.get("raw_meta_key") or "").strip()
    if not (source_raw_key and source_raw_meta_key):
        raise RuntimeError("fused ExtractPalette requires raw_key and raw_meta_key")
    raw_sidecar = validate_raw_sidecar(
        _load_json_key(source_raw_meta_key),
        expected_raw_key=source_raw_key,
        expected_artifact_family="color",
    )
    step_scores_key = str(raw_sidecar.get("step_scores_key") or source_meta.get("step_scores_key") or "").strip()
    step_count = int(raw_sidecar.get("step_count") or source_meta.get("step_count") or 0)
    grid_n = int(raw_sidecar.get("step_scores_grid_n") or source_meta.get("step_scores_grid_n") or 0)
    if not step_scores_key or step_count <= 0 or grid_n <= 0:
        raise RuntimeError("fused ExtractPalette requires v3 raw sidecar step score metadata")
    if source_meta.get("step_scores_key") and str(source_meta.get("step_scores_key")).strip() != step_scores_key:
        raise RuntimeError("step_scores_key mismatch between color metadata and raw sidecar")

    created_at = _utc_now_iso()
    palette_id = f"pal_{artifact_id}"
    prefix = f"renders/{job_id}/palettes/{palette_id}/"
    raw_path = "/tmp/extract_palette_greyscale.raw"
    step_scores_path = "/tmp/extract_palette_step_scores.raw"
    eq_lut_path = "/tmp/extract_palette_eq_lut.bin"
    image_path = "/tmp/extract_palette.jpeg"
    preview_path = "/tmp/extract_palette_preview.png"
    raw_key = prefix + "greyscale.raw"
    raw_meta_key = prefix + "greyscale.meta.json"
    image_key = prefix + "image.jpeg"
    preview_key = prefix + "preview.png"
    meta_key = prefix + "meta.json"
    temp_paths = [raw_path, step_scores_path, eq_lut_path, image_path, preview_path]
    try:
        _download_key_to_path(step_scores_key, step_scores_path)
        actual_step_count = os.path.getsize(step_scores_path)
        if actual_step_count < step_count:
            raise RuntimeError(
                f"step_scores.raw shorter than sidecar step_count: expected {step_count}, got {actual_step_count}"
            )

        proc = subprocess.run(
            [
                STEP_SCORES_TO_PALETTE_RAW,
                f"--input={step_scores_path}",
                f"--output={raw_path}",
                f"--grid-n={grid_n}",
                f"--step-count={step_count}",
            ],
            capture_output=True,
            text=True,
            timeout=600,
        )
        if proc.returncode != 0:
            raise RuntimeError(f"step_scores_to_palette_raw failed: {proc.stderr.strip() or 'unknown error'}")

        histogram = histogram_from_raw_path(raw_path, expected_size=grid_n * grid_n)
        write_equalization_lut(eq_lut_path, histogram)
        encode_meta = render_score_raw(
            raw_path=raw_path,
            out_path=image_path,
            preview_path=preview_path,
            width=grid_n,
            height=grid_n,
            eq_lut_path=eq_lut_path,
            palette=str(source_meta.get("palette") or "inferno"),
            background_color=background_color_hex(raw_sidecar.get("background_color", [0, 0, 0])),
            quality=90,
        )

        sidecar = build_raw_sidecar(
            job_id=job_id,
            run_id=task_id,
            artifact_family="palette",
            artifact_id=palette_id,
            width=grid_n,
            height=grid_n,
            chain_fingerprint=raw_sidecar["chain_fingerprint"],
            score_chain=raw_sidecar["score_chain"],
            score_program=raw_sidecar["score_program"],
            clip_slots=raw_sidecar["clip_slots"],
            background_color=raw_sidecar["background_color"],
            plan_params_digest=raw_sidecar["plan_params_digest"],
            render_execution=raw_sidecar["render_execution"],
            raw_key=raw_key,
            image_key=image_key,
            preview_key=preview_key,
            meta_key=meta_key,
            created_at=created_at,
            histogram=histogram,
        )
        with open(raw_path, "rb") as raw_fh:
            s3.put_object(
                Bucket=BUCKET,
                Key=raw_key,
                Body=raw_fh,
                ContentType="application/octet-stream",
            )
        s3.put_object(
            Bucket=BUCKET,
            Key=raw_meta_key,
            Body=json.dumps(sidecar, separators=(",", ":")).encode("utf-8"),
            ContentType="application/json",
        )
        image_metadata = {
            "width": str(grid_n),
            "height": str(grid_n),
            "palette": str(source_meta.get("palette") or ""),
            "full_n": str(grid_n),
            "times": "1",
            "using_pass": "0",
        }
        _copy_viewport_metadata(image_metadata, source_meta)
        if _metadata_size_bytes(image_metadata) > S3_USER_METADATA_LIMIT_BYTES:
            raise RuntimeError("palette image metadata too large before upload")
        with open(image_path, "rb") as image_fh:
            s3.put_object(
                Bucket=BUCKET,
                Key=image_key,
                Body=image_fh,
                ContentType="image/jpeg",
                Metadata=image_metadata,
            )
        with open(preview_path, "rb") as preview_fh:
            s3.put_object(
                Bucket=BUCKET,
                Key=preview_key,
                Body=preview_fh,
                ContentType="image/png",
            )

        quantile = source_meta.get("solve_score_quantile", "")
        omega = source_meta.get("solve_score_omega", "")
        omega_enabled = source_meta.get("solve_score_omega_enabled", "")
        score_chain = raw_sidecar["score_chain"]
        metric = str(source_meta.get("solve_metric") or "").strip()
        display_name = _associated_palette_display_name(score_chain, metric, quantile, source_meta.get("palette"))
        meta_body = {
            "job_id": job_id,
            "palette_id": palette_id,
            "created_at": created_at,
            "display_name": display_name or palette_id,
            "palette": str(source_meta.get("palette") or ""),
            "degree": int(source_meta.get("degree") or 0),
            "N": grid_n,
            "times": 1,
            "using_pass": 0,
            "image_pass": 0,
            "base_grid_solves": grid_n * grid_n,
            "total_solves": grid_n * grid_n,
            "pass_count": 1,
            "data_layout": "fused_pass0_raw_v1",
            "render_reusable": False,
            "file_size": int(encode_meta["file_size"]),
            "image_key": image_key,
            "preview_key": preview_key,
            "raw_key": raw_key,
            "raw_meta_key": raw_meta_key,
            "metric": metric,
            "solve_score_chain": score_chain,
            "chain_fingerprint": raw_sidecar["chain_fingerprint"],
            "derived_from_color_artifact_id": artifact_id,
            "derivation_kind": "extract_palette",
            "assemble_ms": 0,
            "render_ms": 0,
            "encode_ms": 0,
            "file_size_bytes": int(encode_meta["file_size"]),
            "nonzero_pixels": int(sum(histogram[1:])),
            "background_pixels": int(histogram[0]),
        }
        _copy_viewport_metadata(meta_body, source_meta)
        s3.put_object(
            Bucket=BUCKET,
            Key=meta_key,
            Body=json.dumps(meta_body, separators=(",", ":")).encode("utf-8"),
            ContentType="application/json",
        )
        return {
            "palette_id": palette_id,
            "display_name": display_name or palette_id,
            "palette": str(source_meta.get("palette") or ""),
            "metric": metric,
            "score_chain": score_chain,
            "quantile": quantile,
            "omega": omega,
            "omega_enabled": omega_enabled,
            "image_key": image_key,
            "preview_key": preview_key,
            "raw_key": raw_key,
            "raw_meta_key": raw_meta_key,
            "meta_key": meta_key,
            "file_size": int(encode_meta["file_size"]),
        }
    finally:
        for path in temp_paths:
            try:
                os.remove(path)
            except OSError:
                pass


def handler(event, context):
    params = parse_body(event)
    job_id = params["job_id"]
    artifact_id = str(params["artifact_id"])
    task_id = str(params.get("task_id") or f"extract_palette_{artifact_id}")
    progress = {"phase": "extract_palette_fused", "artifact_id": artifact_id}

    try:
        report_status(job_id, task_id, "started", result_data=progress)
        resolution = _resolve_extract_request(job_id, artifact_id)
        selected = resolution["selected"]
        source = resolution["source"]
        kind = resolution["kind"]
        progress["resolution"] = kind

        if kind == "fused":
            report_status(job_id, task_id, "extracting", result_data=progress)
            palette_result = _render_palette_from_step_scores(job_id, artifact_id, source, task_id)
            metadata = dict(selected)
            _clear_associated_palette(metadata)
            _apply_associated_palette(metadata, palette_result, mode="generated")
            _write_overlay(job_id, artifact_id, metadata)
            report_status(job_id, task_id, "done", result_data={**progress, **palette_result})
            return ok_response({
                "job_id": job_id,
                "artifact_id": artifact_id,
                "resolution": kind,
                "associated_palette": palette_result,
            })

        if kind in ("done", "attach_generated"):
            palette_result = _attach_generated_palette(job_id, artifact_id, selected, source)
            report_status(job_id, task_id, "done", result_data={**progress, **palette_result})
            return ok_response({
                "job_id": job_id,
                "artifact_id": artifact_id,
                "resolution": kind,
                "associated_palette": palette_result,
            })

        if kind == "attach_dependency":
            palette_result = _attach_saved_palette_dependency(job_id, artifact_id, selected, source)
            report_status(job_id, task_id, "done", result_data={**progress, **palette_result})
            return ok_response({
                "job_id": job_id,
                "artifact_id": artifact_id,
                "resolution": kind,
                "associated_palette": palette_result,
            })

        if kind == "legacy":
            raise RuntimeError(
                f"Color artifact {artifact_id} does not expose fused step_scores.raw metadata; use the legacy palette extractor"
            )
        raise RuntimeError(f"Color artifact {artifact_id} does not expose extractable palette lineage")
    except Exception as exc:
        report_status(job_id, task_id, "error", str(exc), result_data=progress)
        raise
