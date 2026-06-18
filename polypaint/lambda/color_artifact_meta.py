"""
Helpers for loading and updating Color artifact metadata.

Base Color artifact metadata can live on the image object, but bulky or
retroactive updates use a sidecar JSON overlay so large image objects do not hit
S3 metadata header limits.
"""
from __future__ import annotations

import json
from typing import Dict

from shared import parse_boolish
from solve_score_chain import emit_solve_score_metadata


COLOR_IMAGE_CANDIDATES = ("image.jpeg", "image.png")
LEGACY_COLOR_IMAGE_CANDIDATES = ("image.jpeg", "image.png")
COLOR_IMAGE_HEADER_KEYS = {
    "artifact_id",
    "family",
    "created_at",
    "format",
    "quality",
    "width",
    "height",
    "pix",
    "derived_from_artifact_id",
    "derived_from_image_key",
    "postprocess_kind",
    "postprocess_profile",
}


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


def stringify_color_metadata(meta: Dict[str, object]) -> Dict[str, str]:
    out = {}
    for key, value in (meta or {}).items():
        if value in ("", None):
            continue
        if isinstance(value, bool):
            out[str(key)] = "true" if value else "false"
        elif isinstance(value, (list, dict)):
            out[str(key)] = json.dumps(value, separators=(",", ":"))
        else:
            out[str(key)] = str(value)
    return out


def split_color_artifact_metadata(full_meta: Dict[str, object]):
    normalized = stringify_color_metadata(full_meta)
    image_meta = {key: value for key, value in normalized.items() if key in COLOR_IMAGE_HEADER_KEYS}
    overlay_meta = {key: value for key, value in normalized.items() if key not in COLOR_IMAGE_HEADER_KEYS}
    return image_meta, overlay_meta


def write_color_artifact_meta_overlay(s3_client, bucket: str, job_id: str, artifact_id: str, meta: Dict[str, object]):
    normalized = stringify_color_metadata(meta)
    s3_client.put_object(
        Bucket=bucket,
        Key=color_artifact_meta_key(job_id, artifact_id),
        Body=json.dumps(normalized, separators=(",", ":")).encode("utf-8"),
        ContentType="application/json",
    )
    return normalized


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
    score_chain=None,
    color_interpretation: str = "",
    raw_key: str = "",
    raw_meta_key: str = "",
    meta_key: str = "",
) -> Dict[str, str]:
    meta = {
        "associated_palette_mode": str(mode or ""),
        "associated_palette_id": str(palette_id or ""),
        "associated_palette_display_name": str(display_name or ""),
        "associated_palette_image_key": str(image_key or ""),
        "associated_palette_preview_key": str(preview_key or ""),
        "associated_palette_palette": str(palette or ""),
    }
    if color_interpretation not in ("", None):
        meta["associated_palette_color_interpretation"] = str(color_interpretation)
    if raw_key not in ("", None):
        meta["associated_palette_raw_key"] = str(raw_key)
    if raw_meta_key not in ("", None):
        meta["associated_palette_raw_meta_key"] = str(raw_meta_key)
    if meta_key not in ("", None):
        meta["associated_palette_meta_key"] = str(meta_key)
    meta.update(
        emit_solve_score_metadata(
            "associated_palette",
            metric=metric,
            quantile=quantile,
            omega=omega,
            omega_enabled=omega_enabled,
            chain=score_chain,
        )
    )
    return meta


def inherit_associated_palette_metadata(source_meta: Dict[str, str]) -> Dict[str, str]:
    mode = str((source_meta or {}).get("associated_palette_mode") or "").strip()
    palette_id = str((source_meta or {}).get("associated_palette_id") or "").strip()
    if mode not in ("generated", "dependency") or not palette_id:
        return {}
    meta = associated_palette_metadata(
        mode=mode,
        palette_id=palette_id,
        display_name=(source_meta or {}).get("associated_palette_display_name", ""),
        image_key=(source_meta or {}).get("associated_palette_image_key", ""),
        preview_key=(source_meta or {}).get("associated_palette_preview_key", ""),
        palette=(source_meta or {}).get("associated_palette_palette", ""),
        metric=(source_meta or {}).get("associated_palette_metric", ""),
        quantile=(source_meta or {}).get("associated_palette_quantile", ""),
        omega=(source_meta or {}).get("associated_palette_omega", ""),
        omega_enabled=parse_boolish((source_meta or {}).get("associated_palette_omega_enabled", True), True),
        score_chain=(source_meta or {}).get("associated_palette_score_chain", ""),
        color_interpretation=(source_meta or {}).get("associated_palette_color_interpretation", ""),
        raw_key=(source_meta or {}).get("associated_palette_raw_key", ""),
        raw_meta_key=(source_meta or {}).get("associated_palette_raw_meta_key", ""),
        meta_key=(source_meta or {}).get("associated_palette_meta_key", ""),
    )
    for key in (
        "associated_palette_color_interpretation",
        "associated_palette_score_chain",
        "associated_palette_metric",
        "associated_palette_quantile",
        "associated_palette_omega",
        "associated_palette_omega_enabled",
        "associated_palette_raw_key",
        "associated_palette_raw_meta_key",
        "associated_palette_meta_key",
    ):
        if (source_meta or {}).get(key) not in ("", None):
            meta.update(stringify_color_metadata({key: (source_meta or {}).get(key)}))
    return meta


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
