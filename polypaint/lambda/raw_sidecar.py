from __future__ import annotations

import json
import math


RAW_SIDECAR_VERSION = 3
HISTOGRAM_RAW_SIDECAR_VERSION = 2
LEGACY_RAW_SIDECAR_VERSION = 1
RAW_ENCODING = {
    "type": "u8_clipped_score_v1",
    "background_byte": 0,
    "foreground_min": 1,
    "foreground_max": 255,
    "row_major": True,
}


def _coerce_int(value, label):
    try:
        return int(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{label} must be an integer, got {value!r}")


def _coerce_finite_float(value, label):
    try:
        number = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{label} must be numeric, got {value!r}")
    if not math.isfinite(number):
        raise RuntimeError(f"{label} must be finite, got {value!r}")
    return number


def _parse_chain(value):
    if value in ("", None):
        return []
    if isinstance(value, str):
        try:
            parsed = json.loads(value)
        except Exception as exc:
            raise RuntimeError(f"score_chain must be valid JSON, got {value!r}") from exc
        if not isinstance(parsed, list):
            raise RuntimeError(f"score_chain must decode to a list, got {type(parsed).__name__}")
        return parsed
    if isinstance(value, list):
        return value
    raise RuntimeError(f"score_chain must be a JSON string or list, got {type(value).__name__}")


def _normalize_clip_slots(value):
    if not isinstance(value, list) or not value:
        raise RuntimeError("clip_slots must be a non-empty list")
    normalized = []
    for idx, row in enumerate(value):
        if not isinstance(row, dict):
            raise RuntimeError(f"clip_slots[{idx}] must be an object")
        normalized.append(
            {
                "slot": _coerce_int(row.get("slot"), f"clip_slots[{idx}].slot"),
                "metric": str(row.get("metric") or "").strip(),
                "source": str(row.get("source") or "slv").strip(),
                "clip_lo": _coerce_finite_float(row.get("clip_lo"), f"clip_slots[{idx}].clip_lo"),
                "clip_hi": _coerce_finite_float(row.get("clip_hi"), f"clip_slots[{idx}].clip_hi"),
            }
        )
        if not normalized[-1]["metric"]:
            raise RuntimeError(f"clip_slots[{idx}].metric must be non-empty")
        if not normalized[-1]["source"]:
            raise RuntimeError(f"clip_slots[{idx}].source must be non-empty")
    return normalized


def _normalize_background_color(value):
    if isinstance(value, str):
        s = value.strip().lower()
        if s.startswith("#"):
            s = s[1:]
        if len(s) != 6 or any(ch not in "0123456789abcdef" for ch in s):
            raise RuntimeError(f"background_color must be 6-digit hex or [r,g,b], got {value!r}")
        return [int(s[0:2], 16), int(s[2:4], 16), int(s[4:6], 16)]
    if not isinstance(value, (list, tuple)) or len(value) != 3:
        raise RuntimeError(f"background_color must be a 3-element array, got {value!r}")
    rgb = []
    for idx, item in enumerate(value):
        channel = _coerce_int(item, f"background_color[{idx}]")
        if not (0 <= channel <= 255):
            raise RuntimeError(f"background_color[{idx}] must be in [0,255], got {channel}")
        rgb.append(channel)
    return rgb


def _normalize_histogram(value, *, required=False):
    if value in ("", None):
        if required:
            raise RuntimeError("histogram is required for raw sidecar version 2+")
        return None
    if not isinstance(value, list) or len(value) != 256:
        raise RuntimeError(f"histogram must be a 256-element array, got {value!r}")
    histogram = []
    for idx, item in enumerate(value):
        count = _coerce_int(item, f"histogram[{idx}]")
        if count < 0:
            raise RuntimeError(f"histogram[{idx}] must be non-negative, got {count}")
        histogram.append(count)
    return histogram


def _normalize_step_scores_key(value, *, required=False):
    key = str(value or "").strip()
    if required and not key:
        raise RuntimeError("step_scores_key is required for raw sidecar version 3")
    return key


def _normalize_positive_int(value, label, *, required=False):
    if value in ("", None):
        if required:
            raise RuntimeError(f"{label} is required for raw sidecar version 3")
        return None
    number = _coerce_int(value, label)
    if number <= 0:
        raise RuntimeError(f"{label} must be > 0, got {number}")
    return number


def background_color_hex(value):
    rgb = _normalize_background_color(value)
    return "".join(f"{channel:02x}" for channel in rgb)


def build_raw_sidecar(
    *,
    job_id,
    run_id,
    artifact_family,
    artifact_id,
    width,
    height,
    chain_fingerprint,
    score_chain,
    score_program,
    clip_slots,
    background_color,
    plan_params_digest,
    render_execution,
    raw_key,
    image_key,
    preview_key,
    meta_key,
    created_at,
    histogram,
    step_scores_key=None,
    step_count=None,
    step_scores_grid_n=None,
):
    chain_fingerprint = str(chain_fingerprint or "").strip()
    if not chain_fingerprint:
        raise RuntimeError("chain_fingerprint is required for greyscale sidecar")
    score_program = str(score_program or "").strip()
    if not score_program:
        raise RuntimeError("score_program is required for greyscale sidecar")
    include_step_scores = (
        str(step_scores_key or "").strip() != ""
        or step_count not in ("", None, 0)
        or step_scores_grid_n not in ("", None, 0)
    )
    version = RAW_SIDECAR_VERSION if include_step_scores else HISTOGRAM_RAW_SIDECAR_VERSION
    sidecar = {
        "version": version,
        "job_id": str(job_id),
        "run_id": str(run_id),
        "artifact_family": str(artifact_family),
        "artifact_id": str(artifact_id),
        "width": _coerce_int(width, "width"),
        "height": _coerce_int(height, "height"),
        "encoding": dict(RAW_ENCODING),
        "chain_fingerprint": chain_fingerprint,
        "score_chain": _parse_chain(score_chain),
        "score_program": score_program,
        "clip_slots": _normalize_clip_slots(clip_slots),
        "background_color": _normalize_background_color(background_color),
        "plan_params_digest": str(plan_params_digest or "").strip(),
        "render_execution": dict(render_execution or {}),
        "keys": {
            "raw_key": str(raw_key),
            "image_key": str(image_key),
            "preview_key": str(preview_key),
            "meta_key": str(meta_key),
        },
        "created_at": str(created_at),
        "histogram": _normalize_histogram(histogram, required=True),
    }
    if not sidecar["plan_params_digest"]:
        raise RuntimeError("plan_params_digest is required for greyscale sidecar")
    if include_step_scores:
        sidecar["step_scores_key"] = _normalize_step_scores_key(step_scores_key, required=True)
        sidecar["step_count"] = _normalize_positive_int(step_count, "step_count", required=True)
        sidecar["step_scores_grid_n"] = _normalize_positive_int(
            step_scores_grid_n,
            "step_scores_grid_n",
            required=True,
        )
    return sidecar


def validate_raw_sidecar(sidecar, *, expected_raw_key=None, expected_artifact_family=None):
    if not isinstance(sidecar, dict):
        raise RuntimeError("raw sidecar must be a JSON object")
    version = _coerce_int(sidecar.get("version"), "raw sidecar version")
    if version not in (LEGACY_RAW_SIDECAR_VERSION, HISTOGRAM_RAW_SIDECAR_VERSION, RAW_SIDECAR_VERSION):
        raise RuntimeError(
            "raw sidecar version must be "
            f"{LEGACY_RAW_SIDECAR_VERSION}, {HISTOGRAM_RAW_SIDECAR_VERSION}, or {RAW_SIDECAR_VERSION}, "
            f"got {version}"
        )
    encoding = sidecar.get("encoding")
    if not isinstance(encoding, dict):
        raise RuntimeError("raw sidecar encoding must be an object")
    if encoding != RAW_ENCODING:
        raise RuntimeError(f"unsupported raw sidecar encoding: {encoding!r}")
    family = str(sidecar.get("artifact_family") or "").strip()
    if expected_artifact_family and family != expected_artifact_family:
        raise RuntimeError(
            f"raw sidecar artifact_family mismatch: expected {expected_artifact_family!r}, got {family!r}"
        )
    keys = sidecar.get("keys")
    if not isinstance(keys, dict):
        raise RuntimeError("raw sidecar keys must be an object")
    raw_key = str(keys.get("raw_key") or "").strip()
    if expected_raw_key and raw_key != expected_raw_key:
        raise RuntimeError(f"raw sidecar keys.raw_key mismatch: expected {expected_raw_key!r}, got {raw_key!r}")
    return {
        "version": version,
        "job_id": str(sidecar.get("job_id") or ""),
        "run_id": str(sidecar.get("run_id") or ""),
        "artifact_family": family,
        "artifact_id": str(sidecar.get("artifact_id") or ""),
        "width": _coerce_int(sidecar.get("width"), "raw sidecar width"),
        "height": _coerce_int(sidecar.get("height"), "raw sidecar height"),
        "encoding": dict(encoding),
        "chain_fingerprint": str(sidecar.get("chain_fingerprint") or "").strip(),
        "score_chain": _parse_chain(sidecar.get("score_chain")),
        "score_program": str(sidecar.get("score_program") or "").strip(),
        "clip_slots": _normalize_clip_slots(sidecar.get("clip_slots")),
        "background_color": _normalize_background_color(sidecar.get("background_color")),
        "plan_params_digest": str(sidecar.get("plan_params_digest") or "").strip(),
        "render_execution": dict(sidecar.get("render_execution") or {}),
        "keys": {
            "raw_key": raw_key,
            "image_key": str(keys.get("image_key") or "").strip(),
            "preview_key": str(keys.get("preview_key") or "").strip(),
            "meta_key": str(keys.get("meta_key") or "").strip(),
        },
        "created_at": str(sidecar.get("created_at") or "").strip(),
        "histogram": _normalize_histogram(
            sidecar.get("histogram"),
            required=(version >= HISTOGRAM_RAW_SIDECAR_VERSION),
        ),
        "step_scores_key": _normalize_step_scores_key(
            sidecar.get("step_scores_key"),
            required=(version >= RAW_SIDECAR_VERSION),
        ),
        "step_count": _normalize_positive_int(
            sidecar.get("step_count"),
            "step_count",
            required=(version >= RAW_SIDECAR_VERSION),
        ),
        "step_scores_grid_n": _normalize_positive_int(
            sidecar.get("step_scores_grid_n"),
            "step_scores_grid_n",
            required=(version >= RAW_SIDECAR_VERSION),
        ),
    }
