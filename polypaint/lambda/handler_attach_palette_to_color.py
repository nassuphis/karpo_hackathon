"""
Attach or update associated-palette metadata on an existing Color artifact.

This writes a sidecar metadata JSON for the Color artifact. The image bytes and
image-object headers stay unchanged so large Color artifacts do not fail with
S3 MetadataTooLarge when a palette association is added later.
"""
import json

import boto3

from color_artifact_meta import (
    associated_palette_metadata,
    color_artifact_meta_key,
    load_color_artifact_meta_overlay,
    load_color_artifact_head,
    merge_metadata,
)
from shared import (
    BUCKET,
    attach_contract_warnings,
    contract_param,
    ok_response,
    parse_body,
    report_status,
)


s3 = boto3.client("s3")


def _parse_boolish(value, default=True):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def handler(event, context):
    params = parse_body(event)
    contract_warnings = []
    job_id = params["job_id"]
    task_id = params["task_id"]
    artifact_id = params["artifact_id"]
    mode = str(contract_param(params, "associated_palette_mode", "generated", contract_warnings) or "").strip()
    palette_id = str(contract_param(params, "associated_palette_id", "", contract_warnings) or "").strip()
    display_name = str(contract_param(params, "associated_palette_display_name", "", contract_warnings) or "").strip()
    image_key = str(contract_param(params, "associated_palette_image_key", "", contract_warnings) or "").strip()
    preview_key = str(contract_param(params, "associated_palette_preview_key", "", contract_warnings) or "").strip()
    palette = str(contract_param(params, "associated_palette_palette", "", contract_warnings) or "").strip()
    metric = str(contract_param(params, "associated_palette_metric", "", contract_warnings) or "").strip()
    quantile = contract_param(params, "associated_palette_quantile", "", contract_warnings)
    omega = contract_param(params, "associated_palette_omega", "", contract_warnings)
    omega_enabled = _parse_boolish(contract_param(params, "associated_palette_omega_enabled", True, contract_warnings), True)

    if mode not in ("generated", "dependency"):
        raise RuntimeError(f"associated_palette_mode must be 'generated' or 'dependency', got {mode!r}")
    if not palette_id:
        raise RuntimeError("associated_palette_id is required")
    if not image_key:
        raise RuntimeError("associated_palette_image_key is required")

    progress = attach_contract_warnings(
        {
            "phase": "attach_associated_palette",
            "artifact_id": artifact_id,
            "associated_palette_mode": mode,
            "associated_palette_id": palette_id,
        },
        contract_warnings,
    )

    try:
        report_status(job_id, task_id, "started", result_data=progress)
        head_info = load_color_artifact_head(s3, BUCKET, job_id, artifact_id)
        assoc_meta = associated_palette_metadata(
            mode=mode,
            palette_id=palette_id,
            display_name=display_name,
            image_key=image_key,
            preview_key=preview_key,
            palette=palette,
            metric=metric,
            quantile=quantile,
            omega=omega,
            omega_enabled=omega_enabled,
        )
        existing_overlay = load_color_artifact_meta_overlay(s3, BUCKET, job_id, artifact_id)
        merged_meta = merge_metadata(existing_overlay, assoc_meta)
        meta_key = color_artifact_meta_key(job_id, artifact_id)
        s3.put_object(
            Bucket=BUCKET,
            Key=meta_key,
            Body=json.dumps(merged_meta, separators=(",", ":")).encode("utf-8"),
            ContentType="application/json",
        )
        result_data = attach_contract_warnings(
            {
                "artifact_id": artifact_id,
                "image_key": head_info["image_key"],
                "meta_key": meta_key,
                "associated_palette_mode": mode,
                "associated_palette_id": palette_id,
            },
            contract_warnings,
        )
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response(result_data)
    except Exception as e:
        report_status(job_id, task_id, "error", str(e), result_data=progress)
        raise
