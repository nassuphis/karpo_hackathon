"""Shared ViewSnap camera and fragment contracts.

The render planner and raster worker both validate the same normalized camera
payload. Native code receives only fields returned by ``validate_view_camera``.
"""

from __future__ import annotations

import hashlib
import json
import math

from program_source_core import canonical_number_g17


VIEW_CAMERA_VERSION = 1
VIEW_CAMERA_SLICES = frozenset((0, 2, 3, 4, 5, 8, 11, 16, 24, 32))
LEGACY_SCALAR_FRAGMENT_ENCODING = "u32le_u8_v1"
LEGACY_MULTI_FRAGMENT_ENCODING = "u32le_pixel_idx_plus_u8_channels_v1"
CAMERA_FRAGMENT_ENCODING = "u32le_f32depth_u8_channels_v1"

# Point and appearance keys are accepted only because immutable pre-v2 viewer
# pages still send them. They are deliberately absent from normalized output.
_TOP_LEVEL_KEYS = frozenset((
    "version",
    "projection",
    "matrix_layout",
    "model_view_matrix",
    "projection_matrix",
    "vertical",
    "slices",
    "effective_tlo",
    "effective_thi",
    "point_world_size",
    "point_scale",
    "point_min_fraction",
    "point_max_fraction",
    "style",
    "show",
    "frame",
    "debug",
))
_DEBUG_KEYS = frozenset((
    "camera_position",
    "camera_target",
    "vertical_fov_degrees",
    "near",
    "far",
    "height",
    "control_zlo",
    "control_zhi",
    "point_control",
))


def _finite_number(value, field):
    if isinstance(value, bool):
        raise RuntimeError(f"{field} must be a finite number")
    try:
        number = float(value)
    except (TypeError, ValueError) as exc:
        raise RuntimeError(f"{field} must be a finite number") from exc
    if not math.isfinite(number):
        raise RuntimeError(f"{field} must be finite")
    return 0.0 if number == 0.0 else number


def _finite_vector(value, length, field):
    if not isinstance(value, list) or len(value) != length:
        raise RuntimeError(f"{field} must contain exactly {length} numbers")
    return [_finite_number(item, f"{field}[{idx}]") for idx, item in enumerate(value)]


def _exact_integer(value, field):
    if isinstance(value, bool):
        raise RuntimeError(f"{field} must be an integer")
    try:
        number = float(value)
    except (TypeError, ValueError) as exc:
        raise RuntimeError(f"{field} must be an integer") from exc
    if not math.isfinite(number) or not number.is_integer():
        raise RuntimeError(f"{field} must be an integer")
    return int(number)


def _close(actual, expected, *, tolerance=1e-9):
    return math.isclose(float(actual), float(expected), rel_tol=tolerance, abs_tol=tolerance)


def _validate_projection_matrix(matrix):
    if not _close(matrix[11], -1.0) or not _close(matrix[15], 0.0):
        raise RuntimeError("view_camera.projection_matrix is not perspective")
    if not matrix[0] > 0.0 or not matrix[5] > 0.0:
        raise RuntimeError("view_camera.projection_matrix has invalid scale")
    for idx in (1, 2, 3, 4, 6, 7, 8, 9, 12, 13):
        if not _close(matrix[idx], 0.0):
            raise RuntimeError(
                f"view_camera.projection_matrix[{idx}] must be zero for a centered perspective camera"
            )
    if not _close(matrix[0], matrix[5], tolerance=1e-7):
        raise RuntimeError("view_camera.projection_matrix must have unit aspect")
    near_denom = matrix[10] - 1.0
    far_denom = matrix[10] + 1.0
    if near_denom == 0.0 or far_denom == 0.0:
        raise RuntimeError("view_camera.projection_matrix has invalid near/far terms")
    near = matrix[14] / near_denom
    far = matrix[14] / far_denom
    if not math.isfinite(near) or not math.isfinite(far) or not (0.0 < near < far):
        raise RuntimeError("view_camera.projection_matrix has invalid near/far planes")


def _validate_model_view_matrix(matrix):
    for idx, expected in ((3, 0.0), (7, 0.0), (11, 0.0), (15, 1.0)):
        if not _close(matrix[idx], expected):
            raise RuntimeError("view_camera.model_view_matrix must be affine")


def _validate_optional_debug(raw):
    if raw in (None, {}):
        return {}
    if not isinstance(raw, dict):
        raise RuntimeError("view_camera.debug must be an object")
    unknown = sorted(set(raw) - _DEBUG_KEYS)
    if unknown:
        raise RuntimeError(f"view_camera.debug has unexpected field(s): {', '.join(unknown)}")
    out = {}
    for key in ("camera_position", "camera_target"):
        if key in raw:
            out[key] = _finite_vector(raw[key], 3, f"view_camera.debug.{key}")
    for key in (
        "vertical_fov_degrees",
        "near",
        "far",
        "height",
        "control_zlo",
        "control_zhi",
    ):
        if key in raw:
            out[key] = _finite_number(raw[key], f"view_camera.debug.{key}")
    return out


def validate_view_camera(raw):
    if not isinstance(raw, dict):
        raise RuntimeError("view_camera must be an object")
    unknown = sorted(set(raw) - _TOP_LEVEL_KEYS)
    if unknown:
        raise RuntimeError(f"view_camera has unexpected field(s): {', '.join(unknown)}")

    version = _exact_integer(raw.get("version"), "view_camera.version")
    if version != VIEW_CAMERA_VERSION:
        raise RuntimeError(f"view_camera.version must be {VIEW_CAMERA_VERSION}")
    if raw.get("projection") != "perspective":
        raise RuntimeError("view_camera.projection must be perspective")
    if raw.get("matrix_layout") != "column_major":
        raise RuntimeError("view_camera.matrix_layout must be column_major")

    model_view = _finite_vector(raw.get("model_view_matrix"), 16, "view_camera.model_view_matrix")
    projection = _finite_vector(raw.get("projection_matrix"), 16, "view_camera.projection_matrix")
    _validate_model_view_matrix(model_view)
    _validate_projection_matrix(projection)

    vertical = str(raw.get("vertical") or "").strip().lower()
    if vertical not in ("t1", "t2"):
        raise RuntimeError("view_camera.vertical must be t1 or t2")
    slices = _exact_integer(raw.get("slices"), "view_camera.slices")
    if slices not in VIEW_CAMERA_SLICES:
        raise RuntimeError(
            "view_camera.slices must be one of "
            + ", ".join(str(value) for value in sorted(VIEW_CAMERA_SLICES))
        )

    tlo = _finite_number(raw.get("effective_tlo"), "view_camera.effective_tlo")
    thi = _finite_number(raw.get("effective_thi"), "view_camera.effective_thi")
    if not (0.0 <= tlo <= thi <= 1.0):
        raise RuntimeError("view_camera effective t bounds must satisfy 0 <= lo <= hi <= 1")

    frame = raw.get("frame")
    if not isinstance(frame, dict) or set(frame) != {"aspect", "crop"}:
        raise RuntimeError("view_camera.frame must contain exactly aspect and crop")
    aspect = _finite_number(frame.get("aspect"), "view_camera.frame.aspect")
    if not _close(aspect, 1.0) or frame.get("crop") != "center_square":
        raise RuntimeError("ViewSnap requires a center-square unit-aspect frame")

    normalized = {
        "version": version,
        "projection": "perspective",
        "matrix_layout": "column_major",
        "model_view_matrix": model_view,
        "projection_matrix": projection,
        "vertical": vertical,
        "slices": slices,
        "effective_tlo": tlo,
        "effective_thi": thi,
        "frame": {"aspect": 1.0, "crop": "center_square"},
    }
    debug = _validate_optional_debug(raw.get("debug"))
    if debug:
        normalized["debug"] = debug
    return normalized


def camera_execution_subset(camera):
    normalized = validate_view_camera(camera)
    return {
        key: normalized[key]
        for key in (
            "version",
            "projection",
            "matrix_layout",
            "model_view_matrix",
            "projection_matrix",
            "vertical",
            "slices",
            "effective_tlo",
            "effective_thi",
            "frame",
        )
    }


def _canonical_digest_value(value):
    if isinstance(value, float):
        return canonical_number_g17(value)
    if isinstance(value, list):
        return [_canonical_digest_value(item) for item in value]
    if isinstance(value, dict):
        return {
            key: _canonical_digest_value(value[key])
            for key in sorted(value)
        }
    return value


def camera_execution_hash(camera):
    payload = json.dumps(
        _canonical_digest_value(camera_execution_subset(camera)),
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def fragment_contract(projection, channels, *, has_explicit_outputs=False):
    try:
        channels = int(channels)
    except (TypeError, ValueError) as exc:
        raise RuntimeError("fragment channels must be an integer") from exc
    if not (1 <= channels <= 8):
        raise RuntimeError("fragment channels must be in [1,8]")
    projection = str(projection or "plan").strip().lower()
    if projection == "camera":
        encoding = CAMERA_FRAGMENT_ENCODING
        record_size = 8 + channels
    elif channels == 1 and not has_explicit_outputs:
        encoding = LEGACY_SCALAR_FRAGMENT_ENCODING
        record_size = 5
    else:
        encoding = LEGACY_MULTI_FRAGMENT_ENCODING
        record_size = 4 + channels
    return {
        "encoding": encoding,
        "record_size_bytes": record_size,
        "channels": channels,
    }


def validate_fragment_contract(raw, *, projection, channels, has_explicit_outputs=False):
    expected = fragment_contract(
        projection,
        channels,
        has_explicit_outputs=has_explicit_outputs,
    )
    if not isinstance(raw, dict):
        raise RuntimeError("fragment_contract must be an object")
    actual = {
        "encoding": str(raw.get("encoding") or "").strip(),
        "record_size_bytes": int(raw.get("record_size_bytes") or 0),
        "channels": int(raw.get("channels") or 0),
    }
    if actual != expected:
        raise RuntimeError(
            f"fragment_contract mismatch: expected {expected!r}, got {actual!r}"
        )
    return expected
