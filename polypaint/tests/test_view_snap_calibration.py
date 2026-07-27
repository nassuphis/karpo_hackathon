import copy
import io
import json
import pathlib
import sys
import zipfile

import pytest


ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "lambda"))

from view_snap_calibration import (  # noqa: E402
    CALIBRATION_DEPENDENCY_FILES,
    calibration_artifact_mode,
    calibration_dependency_hash,
    fixture_admission_digest,
    validate_calibration_artifact,
    validate_calibration_envelope,
)
from view_snap_cost_model import (  # noqa: E402
    enforce_wall_time_limits,
    estimate_camera_wall_times,
)
from scripts import view_snap_calibration as calibration_tool  # noqa: E402


def _identities():
    return {
        "assembler_binary_sha256": "a" * 64,
        "architecture": "arm64",
        "cost_model_sha256": calibration_dependency_hash(ROOT / "lambda"),
        "encoder_binary_sha256": "b" * 64,
        "finalize_memory_mb": 10240,
        "finalize_package_content_sha256": "c" * 64,
        "finalize_timeout_seconds": 900,
        "finalize_tmp_mb": 10240,
        "finalize_workers": 16,
        "libvips_layer_arn": "arn:aws:lambda:us-east-1:1:layer:vips:1",
        "raster_binary_sha256": "d" * 64,
        "raster_memory_mb": 10240,
        "raster_package_content_sha256": "e" * 64,
        "raster_threads": 4,
        "raster_timeout_seconds": 900,
        "raster_tmp_mb": 10240,
        "raster_workers": 10,
        "runtime": "python3.13",
    }


def _rates():
    return {
        "work_units_per_second": 100_000_000.0,
        "raster_prep_seconds": 0.1,
        "raster_upload_setup_seconds": 0.01,
        "raster_upload_bytes_per_second": 100_000_000.0,
        "fragment_request_seconds": 0.01,
        "fragment_download_bytes_per_second": 100_000_000.0,
        "merge_records_per_second": 100_000_000.0,
        "presign_object_seconds": 0.001,
        "sidecar_get_seconds": 0.01,
        "sidecar_download_bytes_per_second": 100_000_000.0,
        "tmp_write_bytes_per_second": 100_000_000.0,
        "sidecar_upload_setup_seconds": 0.01,
        "sidecar_upload_bytes_per_second": 100_000_000.0,
        "publication_put_seconds": 0.01,
        "publication_upload_bytes_per_second": 100_000_000.0,
        "encode_segments": [
            {
                "format": "jpeg",
                "quality_min": 1,
                "quality_max": 100,
                "bytes_per_second": 100_000_000.0,
            },
            {
                "format": "png",
                "quality_min": 1,
                "quality_max": 100,
                "bytes_per_second": 50_000_000.0,
            },
        ],
    }


def _artifact(*, mode="production", digest=""):
    return {
        "schema_version": 1,
        "cost_model_version": 1,
        "mode": mode,
        "fixture_allowlist": [digest] if digest else [],
        "provisional_rates_and_latencies": _rates(),
        "rates_and_latencies": _rates(),
        "derations": {"throughput": 0.5, "latency": 2.0},
        "headrooms": {
            "raster_native_seconds": 30.0,
            "raster_handler_seconds": 60.0,
            "assembler_seconds": 30.0,
            "encoder_seconds": 30.0,
            "finalize_total_seconds": 60.0,
            "fragment_url_setup_margin_seconds": 60.0,
        },
        "identities": _identities(),
        "provenance": {"cells": ["test"]},
    }


def _estimate():
    sections = [
        {
            "section_idx": 0,
            "work_units": 1_000_000,
            "fragment_bytes_upper": 90_000,
            "step_score_bytes": 10_000,
        },
        {
            "section_idx": 1,
            "work_units": 2_000_000,
            "fragment_bytes_upper": 180_000,
            "step_score_bytes": 20_000,
        },
        {
            "section_idx": 2,
            "work_units": 80_000,
            "fragment_bytes_upper": 0,
            "step_score_bytes": 5_000,
        },
    ]
    return {
        "sections": sections,
        "total_fragment_bytes": 270_000,
        "record_size_bytes": 9,
        "step_scores_bytes": 20_000,
        "raw_bytes": 10_000,
        "encoded_output_bytes_upper": 2_020_000,
    }


def test_dependency_hash_is_content_and_mode_bound(tmp_path):
    for name in CALIBRATION_DEPENDENCY_FILES:
        (tmp_path / name).write_text(name, encoding="utf-8")
    first = calibration_dependency_hash(tmp_path)
    (tmp_path / "view_snap_cost_model.py").write_text("changed", encoding="utf-8")
    assert calibration_dependency_hash(tmp_path) != first


def test_production_artifact_is_fail_closed_on_identity_and_encode_gaps():
    artifact = _artifact()
    validated = validate_calibration_artifact(
        artifact,
        live_identities=_identities(),
    )
    assert validated["mode"] == "production"

    for identity_key in _identities():
        stale = copy.deepcopy(_identities())
        value = stale[identity_key]
        stale[identity_key] = value + 1 if isinstance(value, int) else f"{value}-stale"
        with pytest.raises(RuntimeError, match=rf"stale.*{identity_key}"):
            validate_calibration_artifact(artifact, live_identities=stale)

    gap = copy.deepcopy(artifact)
    gap["rates_and_latencies"]["encode_segments"][0]["quality_min"] = 2
    with pytest.raises(RuntimeError, match="gap or overlap"):
        validate_calibration_artifact(gap, live_identities=_identities())

    malformed = copy.deepcopy(artifact)
    malformed["fixture_allowlist"] = ["Z" * 64]
    with pytest.raises(RuntimeError, match="canonical sha256"):
        validate_calibration_artifact(malformed, live_identities=_identities())


def test_explicitly_unconfigured_artifact_selects_resource_only_runtime_mode(tmp_path):
    path = tmp_path / "view_snap_calibration.json"
    artifact = _artifact(mode="calibration", digest="a" * 64)
    artifact["provenance"] = {"state": "unconfigured"}
    path.write_text(json.dumps(artifact), encoding="utf-8")

    assert calibration_artifact_mode(path) == "unconfigured"
    with pytest.raises(RuntimeError, match="unconfigured"):
        validate_calibration_artifact(
            artifact,
            live_identities=_identities(),
            fixture_admission_digest="a" * 64,
        )


def test_unconfigured_artifact_still_requires_the_complete_schema():
    artifact = {
        "schema_version": 1,
        "cost_model_version": 1,
        "mode": "calibration",
        "fixture_allowlist": [],
        "provisional_rates_and_latencies": {},
        "rates_and_latencies": {},
        "derations": {},
        "headrooms": {},
        "identities": {},
        "provenance": {"state": "unconfigured"},
    }
    assert validate_calibration_envelope(artifact)["mode"] == "calibration"
    del artifact["headrooms"]
    with pytest.raises(RuntimeError, match="missing headrooms"):
        validate_calibration_envelope(artifact)


def test_calibration_mode_accepts_only_exact_fixture_digest():
    payload = {
        "N": 100,
        "times": 2,
        "source_identity": {"descriptor_sha256": "f" * 64},
    }
    digest = fixture_admission_digest(payload)
    artifact = _artifact(mode="calibration", digest=digest)
    validated = validate_calibration_artifact(
        artifact,
        live_identities=_identities(),
        fixture_admission_digest=digest,
    )
    assert validated["mode"] == "calibration"
    rejected_digest = "0" * 64
    with pytest.raises(
        RuntimeError,
        match=f"not allowlisted: fixture_admission_digest={rejected_digest}",
    ):
        validate_calibration_artifact(
            artifact,
            live_identities=_identities(),
            fixture_admission_digest=rejected_digest,
        )
    with pytest.raises(RuntimeError, match="canonical sha256"):
        validate_calibration_artifact(
            artifact,
            live_identities=_identities(),
            fixture_admission_digest="not-a-digest",
        )


def test_every_fixture_cost_field_and_source_identity_changes_the_digest():
    payload = {
        "N": 100,
        "times": 2,
        "degree": 7,
        "n_coeffs": 8,
        "channels": 1,
        "view_projection": "camera",
        "view_camera": {"version": 1, "projection": "perspective"},
        "format": "jpeg",
        "quality": 90,
        "root_program_fingerprint": "sha256:root",
        "solve_score_program_fingerprint": "sha256:score",
        "prelude": {"slv": 1, "cf": 0, "pm": 0},
        "sections": [{"section_idx": 0, "step_start": 0, "step_count": 10_000}],
        "execution": {
            "raster_mt_threads": 4,
            "solve_score_threads": 4,
            "raster_workers": 10,
            "finalize_workers": 16,
        },
        "output": {
            "color_interpretation": "scalar_lut",
            "solve_score_normalize": False,
            "palette": "inferno",
            "background_color": "000000",
        },
        "source_identity": {
            "descriptor_sha256": "f" * 64,
            "descriptor": {"objects": [{"key": "roots.bin", "etag": "abc"}]},
        },
    }
    original = fixture_admission_digest(payload)
    mutations = {
        "N": 101,
        "times": 3,
        "degree": 8,
        "n_coeffs": 9,
        "channels": 3,
        "view_projection": "not-camera",
        "view_camera": {"version": 2, "projection": "perspective"},
        "format": "png",
        "quality": 89,
        "root_program_fingerprint": "sha256:root2",
        "solve_score_program_fingerprint": "sha256:score2",
        "prelude": {"slv": 2, "cf": 0, "pm": 0},
        "sections": [{"section_idx": 0, "step_start": 0, "step_count": 9_999}],
        "execution": {
            "raster_mt_threads": 5,
            "solve_score_threads": 5,
            "raster_workers": 10,
            "finalize_workers": 16,
        },
        "output": {
            "color_interpretation": "scalar_lut",
            "solve_score_normalize": False,
            "palette": "viridis",
            "background_color": "000000",
        },
        "source_identity": {
            "descriptor_sha256": "e" * 64,
            "descriptor": {"objects": [{"key": "roots.bin", "etag": "def"}]},
        },
    }
    for field, value in mutations.items():
        changed = copy.deepcopy(payload)
        changed[field] = value
        assert fixture_admission_digest(changed) != original, field


def test_calibration_prepare_and_promote_are_dry_run_first_and_matrix_complete(
    tmp_path,
    monkeypatch,
    capsys,
):
    artifact_path = tmp_path / "artifact.json"
    input_path = tmp_path / "prepare.json"
    matrix_path = tmp_path / "matrix.json"
    digest = "a" * 64
    unconfigured = {
        "schema_version": 1,
        "cost_model_version": 1,
        "mode": "calibration",
        "fixture_allowlist": [],
        "provisional_rates_and_latencies": {},
        "rates_and_latencies": {},
        "derations": {},
        "headrooms": {},
        "identities": {},
        "provenance": {"state": "unconfigured"},
    }
    artifact_path.write_text(json.dumps(unconfigured), encoding="utf-8")
    input_path.write_text(json.dumps({
        "fixture_allowlist": [digest],
        "provisional_rates_and_latencies": _rates(),
        "derations": {"throughput": 0.5, "latency": 2.0},
        "headrooms": _artifact()["headrooms"],
        "provenance": {"matrix": "test"},
    }), encoding="utf-8")
    monkeypatch.setattr(
        calibration_tool,
        "live_calibration_identities",
        lambda *_args, **_kwargs: _identities(),
    )

    calibration_tool._prepare(artifact_path, input_path, write=False)
    assert json.loads(artifact_path.read_text(encoding="utf-8")) == unconfigured
    assert "DRY RUN" in capsys.readouterr().err

    calibration_tool._prepare(artifact_path, input_path, write=True)
    prepared = json.loads(artifact_path.read_text(encoding="utf-8"))
    assert prepared["mode"] == "calibration"
    assert prepared["provenance"]["state"] == "calibration_prepared"

    matrix = {
        "fixture_allowlist": [digest],
        "identities": _identities(),
        "rates_and_latencies": _rates(),
        "derations": {"throughput": 0.5, "latency": 2.0},
        "headrooms": _artifact()["headrooms"],
        "cells": [{
            "fixture_admission_digest": digest,
            "status": "done",
            "work_units_per_second": 123.0,
        }],
        "provenance": {"matrix": "test"},
    }
    matrix_path.write_text(json.dumps(matrix), encoding="utf-8")

    stale_matrix = copy.deepcopy(matrix)
    stale_matrix["identities"]["raster_threads"] = 8
    matrix_path.write_text(json.dumps(stale_matrix), encoding="utf-8")
    with pytest.raises(RuntimeError, match="identities do not match"):
        calibration_tool._promote(artifact_path, matrix_path, write=False)

    incomplete_matrix = copy.deepcopy(matrix)
    incomplete_matrix["cells"] = []
    matrix_path.write_text(json.dumps(incomplete_matrix), encoding="utf-8")
    with pytest.raises(RuntimeError, match="non-empty cells"):
        calibration_tool._promote(artifact_path, matrix_path, write=False)

    matrix_path.write_text(json.dumps(matrix), encoding="utf-8")
    calibration_tool._promote(artifact_path, matrix_path, write=True)
    promoted = json.loads(artifact_path.read_text(encoding="utf-8"))
    assert promoted["mode"] == "production"
    assert promoted["provenance"]["state"] == "production"


def test_canonical_zip_hash_ignores_compression_but_binds_executable_mode():
    def make_zip(compression, mode):
        stream = io.BytesIO()
        with zipfile.ZipFile(stream, "w", compression=compression) as archive:
            info = zipfile.ZipInfo("worker")
            info.external_attr = (0o100000 | mode) << 16
            info.compress_type = compression
            archive.writestr(info, b"same bytes")
        return stream.getvalue()

    stored = make_zip(zipfile.ZIP_STORED, 0o755)
    deflated = make_zip(zipfile.ZIP_DEFLATED, 0o755)
    nonexec = make_zip(zipfile.ZIP_DEFLATED, 0o644)
    assert (
        calibration_tool.canonical_zip_content_hash(stored)
        == calibration_tool.canonical_zip_content_hash(deflated)
    )
    assert (
        calibration_tool.canonical_zip_content_hash(stored)
        != calibration_tool.canonical_zip_content_hash(nonexec)
    )


def test_all_five_wall_gates_are_independent_and_named():
    calibration = validate_calibration_artifact(
        _artifact(),
        live_identities=_identities(),
    )
    estimate = _estimate()
    wall = estimate_camera_wall_times(
        estimate,
        section_count=3,
        fmt="jpeg",
        quality=90,
        calibration=calibration,
    )
    gates = enforce_wall_time_limits(wall, calibration=calibration)
    assert set(gates) == {
        "raster_native",
        "raster_handler",
        "finalize_assembler",
        "finalize_encoder",
        "finalize_total",
    }

    cases = (
        ("work_units_per_second", 1.0, "Raster native"),
        ("raster_upload_bytes_per_second", 1.0, "Raster handler"),
        ("fragment_download_bytes_per_second", 1.0, "Finalize assembler"),
        ("encode_segments", None, "Finalize encoder"),
        ("sidecar_upload_bytes_per_second", 1.0, "Finalize total"),
    )
    for field, value, expected in cases:
        slow_artifact = _artifact()
        if field == "encode_segments":
            for segment in slow_artifact["rates_and_latencies"][field]:
                segment["bytes_per_second"] = 1.0
        else:
            slow_artifact["rates_and_latencies"][field] = value
        slow = validate_calibration_artifact(
            slow_artifact,
            live_identities=_identities(),
        )
        slow_wall = estimate_camera_wall_times(
            estimate,
            section_count=3,
            fmt="jpeg",
            quality=90,
            calibration=slow,
        )
        with pytest.raises(RuntimeError, match=expected):
            enforce_wall_time_limits(slow_wall, calibration=slow)
