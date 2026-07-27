"""Fail-closed runtime calibration contract for ViewSnap admission.

Deterministic byte accounting lives in :mod:`view_snap_cost_model`.  This
module owns the measured coefficients needed to compare that work with the
actual Raster/Finalize Lambda timeouts.  Camera renders never fall back to
guessed rates when the packaged artifact is missing, stale, or incomplete.
"""

from __future__ import annotations

import hashlib
import json
import math
import os
import re
from pathlib import Path

from program_source_core import canonical_number_g17
from view_snap_cost_model import (
    ASSEMBLER_TIMEOUT_SECONDS,
    ENCODER_TIMEOUT_SECONDS,
    FINALIZE_HANDLER_TIMEOUT_SECONDS,
    FRAGMENT_URL_EXPIRY_SECONDS,
    RASTER_HANDLER_TIMEOUT_SECONDS,
    RASTER_NATIVE_TIMEOUT_SECONDS,
    enforce_wall_time_limits,
    estimate_camera_wall_times,
)


CALIBRATION_SCHEMA_VERSION = 1
COST_MODEL_VERSION = 1
CALIBRATION_FILENAME = "view_snap_calibration.json"
SUPPORTED_FORMATS = ("jpeg", "png")
SUPPORTED_QUALITY_MIN = 1
SUPPORTED_QUALITY_MAX = 100
SHA256_HEX_RE = re.compile(r"[0-9a-f]{64}")

# Every file here changes admission behavior or the normalized inputs supplied
# to it.  The hash includes paths, bytes, and executable/type metadata.
CALIBRATION_DEPENDENCY_FILES = (
    "handler_render_plan.py",
    "logical_sections.py",
    "merged_opcodes.json",
    "param_legacy_registry.json",
    "coeff_legacy_registry.json",
    "root_legacy_registry.json",
    "pipeline_programs.py",
    "program_source_core.py",
    "program_profiles.json",
    "root_pipeline_programs.py",
    "root_program_source.py",
    "solve_score_chain.py",
    "solve_score_pipeline_programs.py",
    "solve_score_program_source.py",
    "structural_chips.json",
    "view_camera.py",
    "view_snap_calibration.py",
    "view_snap_cost_model.py",
)

_IDENTITY_ENV = {
    "raster_binary_sha256": "VIEW_SNAP_RASTER_BINARY_SHA256",
    "assembler_binary_sha256": "VIEW_SNAP_ASSEMBLER_BINARY_SHA256",
    "encoder_binary_sha256": "VIEW_SNAP_ENCODER_BINARY_SHA256",
    "raster_package_content_sha256": "VIEW_SNAP_RASTER_PACKAGE_HASH",
    "finalize_package_content_sha256": "VIEW_SNAP_FINALIZE_PACKAGE_HASH",
    "libvips_layer_arn": "VIEW_SNAP_LIBVIPS_LAYER_ARN",
    "runtime": "VIEW_SNAP_RUNTIME",
    "architecture": "VIEW_SNAP_ARCHITECTURE",
    "raster_memory_mb": "VIEW_SNAP_RASTER_MEMORY_MB",
    "raster_tmp_mb": "VIEW_SNAP_RASTER_TMP_MB",
    "raster_timeout_seconds": "VIEW_SNAP_RASTER_TIMEOUT_SECONDS",
    "finalize_memory_mb": "VIEW_SNAP_FINALIZE_MEMORY_MB",
    "finalize_tmp_mb": "VIEW_SNAP_FINALIZE_TMP_MB",
    "finalize_timeout_seconds": "VIEW_SNAP_FINALIZE_TIMEOUT_SECONDS",
    "raster_threads": "VIEW_SNAP_RASTER_THREADS",
    "raster_workers": "VIEW_SNAP_RASTER_WORKERS",
    "finalize_workers": "VIEW_SNAP_FINALIZE_WORKERS",
}

_RATE_FIELDS = (
    "work_units_per_second",
    "raster_prep_seconds",
    "raster_upload_setup_seconds",
    "raster_upload_bytes_per_second",
    "fragment_request_seconds",
    "fragment_download_bytes_per_second",
    "merge_records_per_second",
    "presign_object_seconds",
    "sidecar_get_seconds",
    "sidecar_download_bytes_per_second",
    "tmp_write_bytes_per_second",
    "sidecar_upload_setup_seconds",
    "sidecar_upload_bytes_per_second",
    "publication_put_seconds",
    "publication_upload_bytes_per_second",
)

_HEADROOM_FIELDS = (
    "raster_native_seconds",
    "raster_handler_seconds",
    "assembler_seconds",
    "encoder_seconds",
    "finalize_total_seconds",
    "fragment_url_setup_margin_seconds",
)

_ARTIFACT_FIELDS = frozenset((
    "schema_version",
    "cost_model_version",
    "mode",
    "fixture_allowlist",
    "provisional_rates_and_latencies",
    "rates_and_latencies",
    "derations",
    "headrooms",
    "identities",
    "provenance",
))


class ViewSnapCalibrationError(RuntimeError):
    pass


def _positive_number(value, label, *, allow_zero=False):
    if isinstance(value, bool):
        raise ViewSnapCalibrationError(f"{label} must be a finite number")
    try:
        number = float(value)
    except (TypeError, ValueError) as exc:
        raise ViewSnapCalibrationError(f"{label} must be a finite number") from exc
    if not math.isfinite(number) or number < 0.0 or (number == 0.0 and not allow_zero):
        qualifier = "non-negative" if allow_zero else "positive"
        raise ViewSnapCalibrationError(f"{label} must be finite and {qualifier}")
    return number


def _exact_int(value, label):
    if isinstance(value, bool):
        raise ViewSnapCalibrationError(f"{label} must be an integer")
    try:
        number = int(value)
    except (TypeError, ValueError) as exc:
        raise ViewSnapCalibrationError(f"{label} must be an integer") from exc
    if str(value).strip() not in (str(number), f"{number}.0"):
        try:
            if float(value) != number:
                raise ValueError
        except (TypeError, ValueError) as exc:
            raise ViewSnapCalibrationError(f"{label} must be an integer") from exc
    return number


def _canonical_value(value):
    if isinstance(value, float):
        return canonical_number_g17(value)
    if isinstance(value, list):
        return [_canonical_value(item) for item in value]
    if isinstance(value, dict):
        return {
            str(key): _canonical_value(value[key])
            for key in sorted(value)
        }
    return value


def canonical_digest(value):
    encoded = json.dumps(
        _canonical_value(value),
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(encoded).hexdigest()


def calibration_dependency_hash(base_dir=None):
    root = Path(base_dir or Path(__file__).resolve().parent)
    digest = hashlib.sha256()
    for name in sorted(CALIBRATION_DEPENDENCY_FILES):
        path = root / name
        if not path.is_file():
            raise ViewSnapCalibrationError(
                f"ViewSnap calibration dependency is missing: {name}"
            )
        stat = path.stat()
        mode = stat.st_mode & 0o170777
        digest.update(name.encode("utf-8"))
        digest.update(b"\0")
        digest.update(f"{mode:o}".encode("ascii"))
        digest.update(b"\0")
        digest.update(hashlib.sha256(path.read_bytes()).digest())
        digest.update(b"\n")
    return digest.hexdigest()


def live_calibration_identities(environ=None, *, base_dir=None):
    env = os.environ if environ is None else environ
    missing = [
        env_name
        for env_name in _IDENTITY_ENV.values()
        if not str(env.get(env_name) or "").strip()
    ]
    if missing:
        raise ViewSnapCalibrationError(
            "ViewSnap calibration identity is incomplete; missing "
            + ", ".join(sorted(missing))
        )
    identities = {
        key: str(env[env_name]).strip()
        for key, env_name in _IDENTITY_ENV.items()
    }
    for key in (
        "raster_memory_mb",
        "raster_tmp_mb",
        "raster_timeout_seconds",
        "finalize_memory_mb",
        "finalize_tmp_mb",
        "finalize_timeout_seconds",
        "raster_threads",
        "raster_workers",
        "finalize_workers",
    ):
        identities[key] = _exact_int(identities[key], f"identity.{key}")
        if identities[key] <= 0:
            raise ViewSnapCalibrationError(f"identity.{key} must be positive")
    identities["cost_model_sha256"] = calibration_dependency_hash(base_dir)
    return identities


def _normalize_encode_segments(raw):
    if not isinstance(raw, list) or not raw:
        raise ViewSnapCalibrationError(
            "rates_and_latencies.encode_segments must be a non-empty array"
        )
    segments = []
    by_format = {fmt: [] for fmt in SUPPORTED_FORMATS}
    for idx, item in enumerate(raw):
        if not isinstance(item, dict):
            raise ViewSnapCalibrationError(f"encode_segments[{idx}] must be an object")
        fmt = str(item.get("format") or "").strip().lower()
        if fmt not in SUPPORTED_FORMATS:
            raise ViewSnapCalibrationError(
                f"encode_segments[{idx}].format must be jpeg or png"
            )
        q_min = _exact_int(item.get("quality_min"), f"encode_segments[{idx}].quality_min")
        q_max = _exact_int(item.get("quality_max"), f"encode_segments[{idx}].quality_max")
        if not (
            SUPPORTED_QUALITY_MIN
            <= q_min
            <= q_max
            <= SUPPORTED_QUALITY_MAX
        ):
            raise ViewSnapCalibrationError(
                f"encode_segments[{idx}] has invalid quality interval"
            )
        rate = _positive_number(
            item.get("bytes_per_second"),
            f"encode_segments[{idx}].bytes_per_second",
        )
        normalized = {
            "format": fmt,
            "quality_min": q_min,
            "quality_max": q_max,
            "bytes_per_second": rate,
        }
        segments.append(normalized)
        by_format[fmt].append(normalized)

    for fmt, fmt_segments in by_format.items():
        expected = SUPPORTED_QUALITY_MIN
        for item in sorted(fmt_segments, key=lambda row: row["quality_min"]):
            if item["quality_min"] != expected:
                raise ViewSnapCalibrationError(
                    f"encode_segments has a gap or overlap for {fmt} at quality {expected}"
                )
            expected = item["quality_max"] + 1
        if expected != SUPPORTED_QUALITY_MAX + 1:
            raise ViewSnapCalibrationError(
                f"encode_segments does not cover all {fmt} qualities"
            )
    return sorted(segments, key=lambda row: (row["format"], row["quality_min"]))


def _normalize_rates(raw, *, label):
    if not isinstance(raw, dict):
        raise ViewSnapCalibrationError(f"{label} must be an object")
    normalized = {
        field: _positive_number(raw.get(field), f"{label}.{field}")
        for field in _RATE_FIELDS
    }
    normalized["encode_segments"] = _normalize_encode_segments(
        raw.get("encode_segments")
    )
    return normalized


def _normalize_headrooms(raw):
    if not isinstance(raw, dict):
        raise ViewSnapCalibrationError("headrooms must be an object")
    headrooms = {
        field: _positive_number(raw.get(field), f"headrooms.{field}")
        for field in _HEADROOM_FIELDS
    }
    if (
        FRAGMENT_URL_EXPIRY_SECONDS
        <= ASSEMBLER_TIMEOUT_SECONDS
        + headrooms["fragment_url_setup_margin_seconds"]
    ):
        raise ViewSnapCalibrationError(
            "fragment URL expiry must exceed assembler timeout plus setup margin"
        )
    timeout_pairs = (
        ("raster_native_seconds", RASTER_NATIVE_TIMEOUT_SECONDS),
        ("raster_handler_seconds", RASTER_HANDLER_TIMEOUT_SECONDS),
        ("assembler_seconds", ASSEMBLER_TIMEOUT_SECONDS),
        ("encoder_seconds", ENCODER_TIMEOUT_SECONDS),
        ("finalize_total_seconds", FINALIZE_HANDLER_TIMEOUT_SECONDS),
    )
    for key, timeout in timeout_pairs:
        if headrooms[key] >= timeout:
            raise ViewSnapCalibrationError(
                f"headrooms.{key} must be smaller than its {int(timeout)}s timeout"
            )
    return headrooms


def _normalize_derations(raw):
    if not isinstance(raw, dict):
        raise ViewSnapCalibrationError("derations must be an object")
    throughput = _positive_number(raw.get("throughput"), "derations.throughput")
    latency = _positive_number(raw.get("latency"), "derations.latency")
    if throughput > 1.0:
        raise ViewSnapCalibrationError("derations.throughput must be <= 1")
    if latency < 1.0:
        raise ViewSnapCalibrationError("derations.latency must be >= 1")
    return {"throughput": throughput, "latency": latency}


def _normalize_identities(raw):
    if not isinstance(raw, dict) or not raw:
        raise ViewSnapCalibrationError("identities must be a non-empty object")
    return _canonical_value(raw)


def validate_calibration_envelope(raw):
    """Validate the artifact shape, including the disabled bootstrap state."""

    if not isinstance(raw, dict):
        raise ViewSnapCalibrationError(
            "ViewSnap calibration artifact must be an object"
        )
    missing = sorted(_ARTIFACT_FIELDS - set(raw))
    unknown = sorted(set(raw) - _ARTIFACT_FIELDS)
    if missing or unknown:
        details = []
        if missing:
            details.append("missing " + ", ".join(missing))
        if unknown:
            details.append("unexpected " + ", ".join(unknown))
        raise ViewSnapCalibrationError(
            "ViewSnap calibration artifact fields are invalid: "
            + "; ".join(details)
        )
    if _exact_int(raw.get("schema_version"), "schema_version") != CALIBRATION_SCHEMA_VERSION:
        raise ViewSnapCalibrationError(
            f"unsupported ViewSnap calibration schema; expected {CALIBRATION_SCHEMA_VERSION}"
        )
    if _exact_int(raw.get("cost_model_version"), "cost_model_version") != COST_MODEL_VERSION:
        raise ViewSnapCalibrationError(
            f"unsupported ViewSnap cost model; expected {COST_MODEL_VERSION}"
        )
    mode = str(raw.get("mode") or "").strip().lower()
    if mode not in ("calibration", "production"):
        raise ViewSnapCalibrationError(
            "ViewSnap calibration mode must be calibration or production"
        )
    fixture_allowlist = raw.get("fixture_allowlist")
    if not isinstance(fixture_allowlist, list) or any(
        not isinstance(value, str) or SHA256_HEX_RE.fullmatch(value) is None
        for value in fixture_allowlist
    ):
        raise ViewSnapCalibrationError(
            "fixture_allowlist must contain canonical sha256 hex digests"
        )
    for field in (
        "provisional_rates_and_latencies",
        "rates_and_latencies",
        "derations",
        "headrooms",
        "identities",
        "provenance",
    ):
        if not isinstance(raw.get(field), dict):
            raise ViewSnapCalibrationError(f"{field} must be an object")
    return {
        "mode": mode,
        "fixture_allowlist": sorted(set(fixture_allowlist)),
        "provenance": dict(raw["provenance"]),
    }


def validate_calibration_artifact(
    raw,
    *,
    live_identities,
    fixture_admission_digest="",
):
    envelope = validate_calibration_envelope(raw)
    provenance = envelope["provenance"]
    if str(provenance.get("state") or "").strip().lower() == "unconfigured":
        raise ViewSnapCalibrationError(
            "ViewSnap calibration is unconfigured; run the calibration lifecycle before camera renders"
        )
    mode = envelope["mode"]

    expected_identities = _normalize_identities(raw.get("identities"))
    actual_identities = _normalize_identities(live_identities)
    if expected_identities != actual_identities:
        differing = sorted(
            key
            for key in set(expected_identities) | set(actual_identities)
            if expected_identities.get(key) != actual_identities.get(key)
        )
        raise ViewSnapCalibrationError(
            "ViewSnap calibration is stale for this deployment"
            + (": " + ", ".join(differing) if differing else "")
        )

    fixture_allowlist = envelope["fixture_allowlist"]
    if mode == "calibration":
        digest = str(fixture_admission_digest or "").strip().lower()
        if not digest:
            raise ViewSnapCalibrationError(
                "ViewSnap calibration mode accepts only server-owned fixtures"
            )
        if SHA256_HEX_RE.fullmatch(digest) is None:
            raise ViewSnapCalibrationError(
                "ViewSnap calibration fixture digest must be canonical sha256 hex"
            )
        if digest not in fixture_allowlist:
            raise ViewSnapCalibrationError(
                "ViewSnap calibration fixture is not allowlisted: "
                f"fixture_admission_digest={digest}"
            )
        rates_key = "provisional_rates_and_latencies"
    else:
        rates_key = "rates_and_latencies"

    normalized = {
        "schema_version": CALIBRATION_SCHEMA_VERSION,
        "cost_model_version": COST_MODEL_VERSION,
        "mode": mode,
        "fixture_allowlist": fixture_allowlist,
        "rates_and_latencies": _normalize_rates(raw.get(rates_key), label=rates_key),
        "derations": _normalize_derations(raw.get("derations")),
        "headrooms": _normalize_headrooms(raw.get("headrooms")),
        "identities": expected_identities,
        "provenance": provenance,
    }
    return normalized


def load_calibration_artifact(
    path=None,
    *,
    environ=None,
    fixture_admission_digest="",
    base_dir=None,
):
    artifact_path = Path(
        path
        or (Path(base_dir or Path(__file__).resolve().parent) / CALIBRATION_FILENAME)
    )
    try:
        raw = json.loads(artifact_path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise ViewSnapCalibrationError(
            f"ViewSnap calibration artifact is missing: {artifact_path.name}"
        ) from exc
    except (OSError, json.JSONDecodeError) as exc:
        raise ViewSnapCalibrationError(
            f"ViewSnap calibration artifact is unreadable: {artifact_path.name}"
        ) from exc
    identities = live_calibration_identities(environ, base_dir=base_dir)
    return validate_calibration_artifact(
        raw,
        live_identities=identities,
        fixture_admission_digest=fixture_admission_digest,
    )


def calibration_artifact_mode(path=None, *, base_dir=None):
    artifact_path = Path(
        path
        or (Path(base_dir or Path(__file__).resolve().parent) / CALIBRATION_FILENAME)
    )
    try:
        raw = json.loads(artifact_path.read_text(encoding="utf-8"))
    except FileNotFoundError as exc:
        raise ViewSnapCalibrationError(
            f"ViewSnap calibration artifact is missing: {artifact_path.name}"
        ) from exc
    except (OSError, json.JSONDecodeError) as exc:
        raise ViewSnapCalibrationError(
            f"ViewSnap calibration artifact is unreadable: {artifact_path.name}"
        ) from exc
    envelope = validate_calibration_envelope(raw)
    provenance = envelope["provenance"]
    if str(provenance.get("state") or "").strip().lower() == "unconfigured":
        raise ViewSnapCalibrationError(
            "ViewSnap calibration is unconfigured; run the calibration lifecycle before camera renders"
        )
    return envelope["mode"]


def fixture_admission_digest(payload):
    if not isinstance(payload, dict):
        raise ViewSnapCalibrationError("fixture admission payload must be an object")
    return canonical_digest(payload)
