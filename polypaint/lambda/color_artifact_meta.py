"""
Helpers for loading and updating Color artifact metadata.

Base Color artifact metadata still lives on the image object, but retroactive
updates such as ExtractPalette use a sidecar JSON overlay so large image objects
do not need an in-place CopyObject metadata rewrite.
"""
from __future__ import annotations

import json
from typing import Dict


COLOR_IMAGE_CANDIDATES = ("image.jpeg", "image.png")
LEGACY_COLOR_IMAGE_CANDIDATES = ("image.jpeg", "image.png")


def color_artifact_image_candidates(job_id: str, artifact_id: str):
    if artifact_id == "legacy_color":
        prefix = f"renders/{job_id}/"
        return [prefix + name for name in LEGACY_COLOR_IMAGE_CANDIDATES]
    prefix = f"renders/{job_id}/color/{artifact_id}/"
    return [prefix + name for name in COLOR_IMAGE_CANDIDATES]


def color_artifact_meta_key(job_id: str, artifact_id: str) -> str:
    if artifact_id == "legacy_color":
        return f"renders/{job_id}/meta.json"
    return f"renders/{job_id}/color/{artifact_id}/meta.json"


def load_color_artifact_meta_overlay(s3_client, bucket: str, job_id: str, artifact_id: str) -> Dict[str, object]:
    try:
        obj = s3_client.get_object(Bucket=bucket, Key=color_artifact_meta_key(job_id, artifact_id))
    except Exception:
        return {}
    try:
        body = obj["Body"].read()
        data = json.loads(body)
    except Exception:
        return {}
    return data if isinstance(data, dict) else {}


def load_color_artifact_head(s3_client, bucket: str, job_id: str, artifact_id: str) -> Dict[str, object]:
    errors = []
    for key in color_artifact_image_candidates(job_id, artifact_id):
        try:
            head = s3_client.head_object(Bucket=bucket, Key=key)
        except Exception as exc:  # boto3 client-specific exception types are awkward to share
            errors.append(f"{key}: {type(exc).__name__}")
            continue
        meta = dict(head.get("Metadata", {}) or {})
        meta.setdefault("artifact_id", artifact_id)
        meta.setdefault("family", "color")
        meta.setdefault("format", key.rsplit(".", 1)[-1].lower())
        meta.update(load_color_artifact_meta_overlay(s3_client, bucket, job_id, artifact_id))
        return {
            "artifact_id": artifact_id,
            "image_key": key,
            "content_type": head.get("ContentType") or _guess_content_type(key),
            "cache_control": head.get("CacheControl"),
            "content_disposition": head.get("ContentDisposition"),
            "content_encoding": head.get("ContentEncoding"),
            "content_language": head.get("ContentLanguage"),
            "expires": head.get("Expires"),
            "metadata": meta,
        }
    raise RuntimeError(
        f"Color artifact not found: {artifact_id} under job {job_id} "
        f"(tried {', '.join(color_artifact_image_candidates(job_id, artifact_id))}; errors: {', '.join(errors)})"
    )


def parse_root_transforms(raw):
    if raw in ("", None):
        return []
    if isinstance(raw, list):
        return raw
    try:
        return json.loads(raw)
    except Exception:
        return []


def associated_palette_metadata(
    mode: str,
    palette_id: str,
    display_name: str,
    image_key: str,
    preview_key: str,
    palette: str,
    metric: str,
    quantile,
    omega,
    omega_enabled: bool,
) -> Dict[str, str]:
    return {
        "associated_palette_mode": str(mode or ""),
        "associated_palette_id": str(palette_id or ""),
        "associated_palette_display_name": str(display_name or ""),
        "associated_palette_image_key": str(image_key or ""),
        "associated_palette_preview_key": str(preview_key or ""),
        "associated_palette_palette": str(palette or ""),
        "associated_palette_metric": str(metric or ""),
        "associated_palette_quantile": "" if quantile in ("", None) else str(quantile),
        "associated_palette_omega": "" if omega in ("", None) else str(omega),
        "associated_palette_omega_enabled": "true" if bool(omega_enabled) else "false",
    }


def inherit_associated_palette_metadata(source_meta: Dict[str, str]) -> Dict[str, str]:
    mode = str((source_meta or {}).get("associated_palette_mode") or "").strip()
    palette_id = str((source_meta or {}).get("associated_palette_id") or "").strip()
    if mode not in ("generated", "dependency") or not palette_id:
        return {}
    return associated_palette_metadata(
        mode=mode,
        palette_id=palette_id,
        display_name=(source_meta or {}).get("associated_palette_display_name", ""),
        image_key=(source_meta or {}).get("associated_palette_image_key", ""),
        preview_key=(source_meta or {}).get("associated_palette_preview_key", ""),
        palette=(source_meta or {}).get("associated_palette_palette", ""),
        metric=(source_meta or {}).get("associated_palette_metric", ""),
        quantile=(source_meta or {}).get("associated_palette_quantile", ""),
        omega=(source_meta or {}).get("associated_palette_omega", ""),
        omega_enabled=_parse_boolish((source_meta or {}).get("associated_palette_omega_enabled", True), True),
    )


def merge_metadata(existing_meta: Dict[str, str], assoc_meta: Dict[str, str]) -> Dict[str, str]:
    merged = dict(existing_meta or {})
    merged.update(assoc_meta)
    return merged


def _guess_content_type(key: str) -> str:
    suffix = key.rsplit(".", 1)[-1].lower()
    if suffix == "png":
        return "image/png"
    if suffix in ("jpg", "jpeg"):
        return "image/jpeg"
    return "application/octet-stream"


def _parse_boolish(value, default=True):
    if value in ("", None):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")
