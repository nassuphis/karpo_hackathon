#!/usr/bin/env python3
"""Prepare and validate the packaged ViewSnap calibration artifact.

The deploy script exports the exact worker identities before calling this
tool. An explicitly unconfigured artifact selects resource-only admission;
once timing calibration is configured, every identity mismatch or incomplete
coefficient blocks deployment.
"""

from __future__ import annotations

import argparse
import hashlib
import io
import json
import os
import sys
import tempfile
import urllib.request
import zipfile
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
LAMBDA = ROOT / "lambda"
sys.path.insert(0, str(LAMBDA))

from view_snap_calibration import (  # noqa: E402
    CALIBRATION_SCHEMA_VERSION,
    CALIBRATION_DEPENDENCY_FILES,
    COST_MODEL_VERSION,
    ViewSnapCalibrationError,
    calibration_dependency_hash,
    fixture_admission_digest,
    live_calibration_identities,
    validate_calibration_artifact,
    validate_calibration_envelope,
)


DEFAULT_ARTIFACT = LAMBDA / "view_snap_calibration.json"


def _read(path):
    try:
        data = json.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ViewSnapCalibrationError(f"cannot read {path}: {exc}") from exc
    if not isinstance(data, dict):
        raise ViewSnapCalibrationError(f"{path} must contain a JSON object")
    return data


def _write(path, data):
    Path(path).write_text(
        json.dumps(data, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )


def _validate(path, *, allow_unconfigured, schema_only=False):
    artifact = _read(path)
    state = str((artifact.get("provenance") or {}).get("state") or "").strip()
    if allow_unconfigured and state == "unconfigured":
        validate_calibration_envelope(artifact)
        print(
            "ViewSnap calibration: UNCONFIGURED "
            "(resource-only camera admission is active)"
        )
        return 0
    identities = (
        artifact.get("identities")
        if schema_only
        else live_calibration_identities(os.environ, base_dir=LAMBDA)
    )
    mode = str(artifact.get("mode") or "").strip().lower()
    digest = ""
    if mode == "calibration":
        allowlist = artifact.get("fixture_allowlist")
        if not isinstance(allowlist, list) or not allowlist:
            raise ViewSnapCalibrationError(
                "calibration mode requires a non-empty fixture_allowlist"
            )
        digest = str(allowlist[0])
    validate_calibration_artifact(
        artifact,
        live_identities=identities,
        fixture_admission_digest=digest,
    )
    print(f"ViewSnap calibration: OK ({mode})")
    return 0


def canonical_zip_content_hash(data):
    if isinstance(data, (str, os.PathLike)):
        payload = Path(data).read_bytes()
    else:
        payload = bytes(data)
    digest = hashlib.sha256()
    with zipfile.ZipFile(io.BytesIO(payload)) as archive:
        names = sorted(
            name for name in archive.namelist() if not name.endswith("/")
        )
        for name in names:
            info = archive.getinfo(name)
            mode_and_type = (info.external_attr >> 16) & 0o170777
            digest.update(name.encode("utf-8"))
            digest.update(b"\0")
            digest.update(f"{mode_and_type:o}".encode("ascii"))
            digest.update(b"\0")
            digest.update(hashlib.sha256(archive.read(name)).digest())
            digest.update(b"\n")
    return digest.hexdigest()


def _function_bundle(lambda_client, function_name):
    try:
        lambda_client.get_waiter("function_updated_v2").wait(
            FunctionName=function_name,
            WaiterConfig={"Delay": 2, "MaxAttempts": 150},
        )
        response = lambda_client.get_function(FunctionName=function_name)
        location = str((response.get("Code") or {}).get("Location") or "")
        if not location:
            raise ViewSnapCalibrationError(
                f"{function_name} has no downloadable Lambda code location"
            )
        with urllib.request.urlopen(location, timeout=60) as stream:
            bundle = stream.read()
    except ViewSnapCalibrationError:
        raise
    except Exception as exc:
        raise ViewSnapCalibrationError(
            f"cannot inspect deployed Lambda {function_name}: {exc}"
        ) from exc
    return dict(response.get("Configuration") or {}), bundle


def _zip_member_sha256(bundle, member, function_name):
    try:
        with zipfile.ZipFile(io.BytesIO(bundle)) as archive:
            return hashlib.sha256(archive.read(member)).hexdigest()
    except (KeyError, zipfile.BadZipFile) as exc:
        raise ViewSnapCalibrationError(
            f"{function_name} deployment is missing {member}"
        ) from exc


def _deployed_plan_identities(plan_config, plan_bundle):
    env = dict(((plan_config.get("Environment") or {}).get("Variables") or {}))
    with tempfile.TemporaryDirectory(prefix="view-snap-plan-") as tmp:
        root = Path(tmp)
        try:
            with zipfile.ZipFile(io.BytesIO(plan_bundle)) as archive:
                for name in CALIBRATION_DEPENDENCY_FILES:
                    info = archive.getinfo(name)
                    path = root / name
                    path.write_bytes(archive.read(name))
                    path.chmod((info.external_attr >> 16) & 0o777)
        except (KeyError, zipfile.BadZipFile) as exc:
            raise ViewSnapCalibrationError(
                f"deployed Render Plan bundle is missing a calibration dependency: {exc}"
            ) from exc
        identities = live_calibration_identities(env, base_dir=root)
        if identities["cost_model_sha256"] != calibration_dependency_hash(root):
            raise ViewSnapCalibrationError(
                "deployed Render Plan cost-model identity is internally inconsistent"
            )
    return identities


def _single_config_value(configs, field, label):
    values = {str(config.get(field) or "") for config in configs}
    if len(values) != 1 or not next(iter(values)):
        raise ViewSnapCalibrationError(
            f"deployed Raster/Finalize {label} values do not match"
        )
    return next(iter(values))


def _verify_live(
    artifact_path,
    *,
    raster_function,
    finalize_function,
    plan_function,
    region,
):
    artifact = _read(artifact_path)
    state = str((artifact.get("provenance") or {}).get("state") or "").strip()
    if state == "unconfigured":
        print(
            "ViewSnap post-deploy verification: SKIPPED "
            "(artifact unconfigured; resource-only camera admission is active)"
        )
        return 0

    try:
        import boto3
    except ImportError as exc:
        raise ViewSnapCalibrationError(
            "boto3 is required for ViewSnap live verification"
        ) from exc

    client = boto3.client("lambda", region_name=region)
    raster_config, raster_bundle = _function_bundle(client, raster_function)
    finalize_config, finalize_bundle = _function_bundle(client, finalize_function)
    plan_config, plan_bundle = _function_bundle(client, plan_function)

    declared = _deployed_plan_identities(plan_config, plan_bundle)
    expected_layer = str(
        (artifact.get("identities") or {}).get("libvips_layer_arn") or ""
    )
    finalize_layers = {
        str(item.get("Arn") or "") for item in finalize_config.get("Layers") or []
    }
    if not expected_layer or expected_layer not in finalize_layers:
        raise ViewSnapCalibrationError(
            "deployed Finalize Lambda does not use the calibrated libvips layer"
        )

    configs = (raster_config, finalize_config)
    architecture_values = {
        tuple(config.get("Architectures") or []) for config in configs
    }
    if len(architecture_values) != 1:
        raise ViewSnapCalibrationError(
            "deployed Raster/Finalize architecture values do not match"
        )
    architectures = next(iter(architecture_values))
    if len(architectures) != 1:
        raise ViewSnapCalibrationError(
            "deployed Raster/Finalize must each have one architecture"
        )

    actual = dict(declared)
    actual.update({
        "raster_binary_sha256": _zip_member_sha256(
            raster_bundle, "roots2pix_mt", raster_function
        ),
        "assembler_binary_sha256": _zip_member_sha256(
            finalize_bundle, "assemble_greyscale", finalize_function
        ),
        "encoder_binary_sha256": _zip_member_sha256(
            finalize_bundle, "score_raw_render", finalize_function
        ),
        "raster_package_content_sha256": canonical_zip_content_hash(raster_bundle),
        "finalize_package_content_sha256": canonical_zip_content_hash(finalize_bundle),
        "libvips_layer_arn": expected_layer,
        "runtime": _single_config_value(configs, "Runtime", "runtime"),
        "architecture": architectures[0],
        "raster_memory_mb": int(raster_config.get("MemorySize") or 0),
        "raster_tmp_mb": int(
            (raster_config.get("EphemeralStorage") or {}).get("Size") or 0
        ),
        "raster_timeout_seconds": int(raster_config.get("Timeout") or 0),
        "finalize_memory_mb": int(finalize_config.get("MemorySize") or 0),
        "finalize_tmp_mb": int(
            (finalize_config.get("EphemeralStorage") or {}).get("Size") or 0
        ),
        "finalize_timeout_seconds": int(finalize_config.get("Timeout") or 0),
    })

    mode = str(artifact.get("mode") or "").strip().lower()
    allowlist = artifact.get("fixture_allowlist") or []
    digest = str(allowlist[0]) if mode == "calibration" and allowlist else ""
    validate_calibration_artifact(
        artifact,
        live_identities=declared,
        fixture_admission_digest=digest,
    )
    validate_calibration_artifact(
        artifact,
        live_identities=actual,
        fixture_admission_digest=digest,
    )

    try:
        with zipfile.ZipFile(io.BytesIO(plan_bundle)) as archive:
            deployed_artifact = json.loads(
                archive.read("view_snap_calibration.json").decode("utf-8")
            )
    except (KeyError, ValueError, zipfile.BadZipFile) as exc:
        raise ViewSnapCalibrationError(
            "deployed Render Plan has no valid ViewSnap calibration artifact"
        ) from exc
    if deployed_artifact != artifact:
        raise ViewSnapCalibrationError(
            "deployed Render Plan calibration artifact differs from the local artifact"
        )

    print(
        "ViewSnap post-deploy verification: OK "
        f"(mode={mode}, raster={raster_function}, finalize={finalize_function})"
    )
    return 0


def _write_identities(path):
    artifact = _read(path)
    artifact["identities"] = live_calibration_identities(
        os.environ,
        base_dir=LAMBDA,
    )
    provenance = (
        dict(artifact.get("provenance"))
        if isinstance(artifact.get("provenance"), dict)
        else {}
    )
    provenance["state"] = "identity_prepared"
    provenance["identity_prepared_at"] = datetime.now(timezone.utc).isoformat()
    artifact["provenance"] = provenance
    _write(path, artifact)
    print(f"ViewSnap calibration identities written to {path}")
    return 0


def _candidate_allowlist(existing, source):
    values = source.get("fixture_allowlist")
    if values in (None, []):
        values = existing.get("fixture_allowlist")
    if not isinstance(values, list) or not values:
        raise ViewSnapCalibrationError(
            "calibration preparation requires a non-empty fixture_allowlist"
        )
    return sorted(set(values))


def _write_or_print(path, candidate, *, write):
    if write:
        _write(path, candidate)
        print(f"ViewSnap calibration artifact written to {path}")
    else:
        print(json.dumps(candidate, indent=2, sort_keys=True))
        print(
            "DRY RUN: artifact not written; repeat with --write after review",
            file=sys.stderr,
        )
    return 0


def _prepare(path, input_path, *, write):
    if not input_path:
        raise ViewSnapCalibrationError("prepare requires --input")
    existing = _read(path)
    source = _read(input_path)
    identities = live_calibration_identities(os.environ, base_dir=LAMBDA)
    allowlist = _candidate_allowlist(existing, source)
    provenance = (
        dict(source.get("provenance"))
        if isinstance(source.get("provenance"), dict)
        else {}
    )
    provenance.update({
        "state": "calibration_prepared",
        "prepared_at": datetime.now(timezone.utc).isoformat(),
        "input_file": str(Path(input_path)),
    })
    candidate = {
        "schema_version": CALIBRATION_SCHEMA_VERSION,
        "cost_model_version": COST_MODEL_VERSION,
        "mode": "calibration",
        "fixture_allowlist": allowlist,
        "provisional_rates_and_latencies": source.get(
            "provisional_rates_and_latencies"
        ),
        "rates_and_latencies": {},
        "derations": source.get("derations"),
        "headrooms": source.get("headrooms"),
        "identities": identities,
        "provenance": provenance,
    }
    validate_calibration_artifact(
        candidate,
        live_identities=identities,
        fixture_admission_digest=allowlist[0],
    )
    return _write_or_print(path, candidate, write=write)


def _promote(path, input_path, *, write):
    if not input_path:
        raise ViewSnapCalibrationError("promote requires --input")
    existing = _read(path)
    source = _read(input_path)
    identities = live_calibration_identities(os.environ, base_dir=LAMBDA)
    allowlist = _candidate_allowlist(existing, source)
    prepared_allowlist = sorted(set(existing.get("fixture_allowlist") or []))
    if allowlist != prepared_allowlist:
        raise ViewSnapCalibrationError(
            "promotion fixture_allowlist must exactly match the prepared artifact"
        )
    prepared_state = str(
        (existing.get("provenance") or {}).get("state") or ""
    ).strip().lower()
    if prepared_state != "calibration_prepared":
        raise ViewSnapCalibrationError(
            "promotion requires the checked-in calibration_prepared artifact"
        )
    validate_calibration_artifact(
        existing,
        live_identities=identities,
        fixture_admission_digest=allowlist[0],
    )
    matrix_identities = source.get("identities")
    if not isinstance(matrix_identities, dict) or not matrix_identities:
        raise ViewSnapCalibrationError(
            "promotion input requires the deployed calibration identities"
        )
    if fixture_admission_digest(matrix_identities) != fixture_admission_digest(
        identities
    ):
        raise ViewSnapCalibrationError(
            "promotion matrix identities do not match the prepared deployment"
        )
    cells = source.get("cells")
    if not isinstance(cells, list) or not cells:
        raise ViewSnapCalibrationError(
            "promotion input requires a non-empty cells array"
        )
    completed = set()
    for idx, cell in enumerate(cells):
        if not isinstance(cell, dict):
            raise ViewSnapCalibrationError(f"cells[{idx}] must be an object")
        digest = str(cell.get("fixture_admission_digest") or "").strip().lower()
        status = str(cell.get("status") or "").strip().lower()
        if status not in ("done", "completed", "passed"):
            raise ViewSnapCalibrationError(
                f"cells[{idx}] is not complete (status={status!r})"
            )
        completed.add(digest)
    missing = sorted(set(allowlist) - completed)
    if missing:
        raise ViewSnapCalibrationError(
            "promotion is missing completed matrix cells for "
            + ", ".join(missing)
        )
    provenance = (
        dict(source.get("provenance"))
        if isinstance(source.get("provenance"), dict)
        else {}
    )
    provenance.update({
        "state": "production",
        "promoted_at": datetime.now(timezone.utc).isoformat(),
        "input_file": str(Path(input_path)),
        "cells": cells,
    })
    candidate = {
        "schema_version": CALIBRATION_SCHEMA_VERSION,
        "cost_model_version": COST_MODEL_VERSION,
        "mode": "production",
        "fixture_allowlist": allowlist,
        "provisional_rates_and_latencies": (
            existing.get("provisional_rates_and_latencies")
            or source.get("provisional_rates_and_latencies")
            or {}
        ),
        "rates_and_latencies": source.get("rates_and_latencies"),
        "derations": source.get("derations"),
        "headrooms": source.get("headrooms"),
        "identities": identities,
        "provenance": provenance,
    }
    validate_calibration_artifact(
        candidate,
        live_identities=identities,
    )
    return _write_or_print(path, candidate, write=write)


def _digest_fixture(path):
    print(fixture_admission_digest(_read(path)))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "command",
        choices=(
            "validate",
            "write-identities",
            "digest-fixture",
            "verify-live",
            "prepare",
            "promote",
        ),
    )
    parser.add_argument("--artifact", default=str(DEFAULT_ARTIFACT))
    parser.add_argument("--fixture")
    parser.add_argument("--input")
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--allow-unconfigured", action="store_true")
    parser.add_argument("--schema-only", action="store_true")
    parser.add_argument("--raster-function")
    parser.add_argument("--finalize-function")
    parser.add_argument("--plan-function")
    parser.add_argument("--region", default=os.environ.get("AWS_REGION", "us-east-1"))
    args = parser.parse_args(argv)
    try:
        if args.command == "validate":
            return _validate(
                args.artifact,
                allow_unconfigured=args.allow_unconfigured,
                schema_only=args.schema_only,
            )
        if args.command == "write-identities":
            return _write_identities(args.artifact)
        if args.command == "prepare":
            return _prepare(
                args.artifact,
                args.input,
                write=args.write,
            )
        if args.command == "promote":
            return _promote(
                args.artifact,
                args.input,
                write=args.write,
            )
        if args.command == "verify-live":
            required = {
                "--raster-function": args.raster_function,
                "--finalize-function": args.finalize_function,
                "--plan-function": args.plan_function,
            }
            missing = [name for name, value in required.items() if not value]
            if missing:
                parser.error(
                    "verify-live requires " + ", ".join(missing)
                )
            return _verify_live(
                args.artifact,
                raster_function=args.raster_function,
                finalize_function=args.finalize_function,
                plan_function=args.plan_function,
                region=args.region,
            )
        if not args.fixture:
            parser.error("digest-fixture requires --fixture")
        return _digest_fixture(args.fixture)
    except ViewSnapCalibrationError as exc:
        print(f"FATAL: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
