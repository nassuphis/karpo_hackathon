#!/usr/bin/env python3
"""Harvest real calc.json payloads into the M3 program-oracle fixture corpus.

This is a manual maintenance tool, not a test dependency. It snapshots saved
`renders/<job_id>/calc.json` objects that contain Param/Coeff program payloads
into `tests/fixtures/program-m3-oracle/harvested/`, where the offline oracle
test can consume them without talking to AWS.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LAMBDA_DIR = ROOT / "lambda"
FIXTURE_DIR = ROOT / "tests" / "fixtures" / "program-m3-oracle"
HARVEST_DIR = FIXTURE_DIR / "harvested"
if str(LAMBDA_DIR) not in sys.path:
    sys.path.insert(0, str(LAMBDA_DIR))

from coeff_program_source import coeff_source_text_from_chain  # noqa: E402
from param_program_source import param_source_text_from_chain  # noqa: E402
from pipeline_programs import (  # noqa: E402
    coeff_transforms_to_program_chain,
    param_transforms_to_program_chain,
)

PROGRAM_KEYS = {
    "param_program_source_text",
    "param_program_chain",
    "coeff_program_source_text",
    "coeff_program_chain",
}

PIPELINE_PROVENANCE_KEYS = (
    "pipeline_mode",
    "function",
    "cfpv",
    "param_transforms",
    "param_transforms_display",
    "coeff_transforms",
    "param_program_fingerprint",
    "param_program_uses_legacy_fast_path",
    "coeff_program_fingerprint",
    "coeff_program_uses_legacy_chain_equivalent",
)


def _job_id_from_prefix(prefix: str) -> str:
    parts = str(prefix or "").split("/")
    if len(parts) >= 3 and parts[0] == "renders" and parts[1] and not parts[1].startswith("_"):
        return parts[1]
    return ""


def _job_id_from_calc_path(path: str) -> str:
    key = str(path or "").strip()
    if key.startswith("s3://"):
        parts = key.split("/", 3)
        key = parts[3] if len(parts) >= 4 else ""
    if key.startswith("renders/"):
        job = _job_id_from_prefix(key.removesuffix("calc.json"))
        if job:
            return job
    if key.endswith("/calc.json"):
        job = key.removesuffix("/calc.json").split("/")[-1]
        if job and not job.startswith("_"):
            return job
    return ""


def _safe_name(job_id: str) -> str:
    safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", str(job_id or "").strip())
    return safe[:120] or "unknown"


def _params(calc: dict) -> dict:
    value = calc.get("params") or calc.get("coeffgen") or {}
    return value if isinstance(value, dict) else {}


def _has_program_payload(calc: dict) -> bool:
    params = _params(calc)
    for key in PROGRAM_KEYS:
        value = params.get(key)
        if isinstance(value, str) and value.strip():
            return True
        if isinstance(value, list) and value:
            return True
    return False


def _enrich_pipeline_program_payload(calc: dict) -> dict:
    calc = dict(calc)
    params = dict(_params(calc))
    pipeline = calc.get("pipeline") if isinstance(calc.get("pipeline"), dict) else {}
    for profile, to_chain, to_source in (
        ("param", param_transforms_to_program_chain, param_source_text_from_chain),
        ("coeff", coeff_transforms_to_program_chain, coeff_source_text_from_chain),
    ):
        chain_key = f"{profile}_program_chain"
        source_key = f"{profile}_program_source_text"
        chain = pipeline.get(chain_key) or params.get(chain_key)
        source = pipeline.get(source_key) or params.get(source_key)
        if not (isinstance(chain, list) and chain):
            transforms = pipeline.get(f"{profile}_transforms")
            if isinstance(transforms, list) and transforms:
                try:
                    chain = to_chain(transforms)
                except Exception:
                    chain = None
        if isinstance(chain, list) and chain:
            params.setdefault(chain_key, chain)
            if not (isinstance(source, str) and source.strip()):
                try:
                    source = to_source(chain)
                except Exception:
                    source = ""
            if isinstance(source, str) and source.strip():
                params.setdefault(source_key, source)
    if params:
        calc["params"] = params
    return calc


def _iter_job_ids(s3, bucket: str, *, start_after_job: str = "", max_jobs: int = 0):
    emitted = 0
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=bucket, Prefix="renders/", Delimiter="/"):
        for item in page.get("CommonPrefixes", []):
            job_id = _job_id_from_prefix(str(item.get("Prefix") or ""))
            if not job_id:
                continue
            if start_after_job and job_id <= start_after_job:
                continue
            yield job_id
            emitted += 1
            if max_jobs and emitted >= max_jobs:
                return


def _job_id_from_s5cmd_ls_line(line: str) -> str:
    # s5cmd ls emits: YYYY/MM/DD HH:MM:SS <size> <key-relative-to-pattern>
    parts = str(line or "").strip().split()
    if not parts:
        return ""
    return _job_id_from_calc_path(parts[-1])


def _iter_job_ids_s5cmd(bucket: str, *, start_after_job: str = "", max_jobs: int = 0):
    if not shutil.which("s5cmd"):
        raise RuntimeError("s5cmd is not installed")
    proc = subprocess.Popen(
        ["s5cmd", "ls", f"s3://{bucket}/renders/*/calc.json"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    emitted = 0
    assert proc.stdout is not None
    for line in proc.stdout:
        job_id = _job_id_from_s5cmd_ls_line(line)
        if not job_id:
            continue
        if start_after_job and job_id <= start_after_job:
            continue
        yield job_id
        emitted += 1
        if max_jobs and emitted >= max_jobs:
            proc.terminate()
            break
    _stdout, stderr = proc.communicate()
    if proc.returncode not in (0, -15) and not (max_jobs and emitted >= max_jobs):
        raise RuntimeError(f"s5cmd ls failed with code {proc.returncode}: {stderr.strip()}")


def _iter_jobs_file(path: Path):
    with path.open("r", encoding="utf-8") as fh:
        for raw in fh:
            text = raw.strip()
            if not text or text.startswith("#"):
                continue
            if text.startswith("s3://"):
                job = _job_id_from_calc_path(text)
                if job:
                    yield job
                    continue
            job = _job_id_from_s5cmd_ls_line(text)
            if job:
                yield job
                continue
            yield text


def _read_calc(s3, bucket: str, job_id: str):
    key = f"renders/{job_id}/calc.json"
    try:
        obj = s3.get_object(Bucket=bucket, Key=key)
    except Exception as exc:
        code = getattr(exc, "response", {}).get("Error", {}).get("Code", "")
        if code in {"NoSuchKey", "404", "NotFound"}:
            return None
        raise
    payload = json.loads(obj["Body"].read().decode("utf-8"))
    if not isinstance(payload, dict):
        return None
    payload.setdefault("job_id", job_id)
    payload.setdefault("_m3_oracle_source_key", key)
    return _enrich_pipeline_program_payload(payload)


def _write_case(calc: dict, job_id: str) -> str:
    HARVEST_DIR.mkdir(parents=True, exist_ok=True)
    path = HARVEST_DIR / f"{_safe_name(job_id)}.calc.json"
    with path.open("w", encoding="utf-8") as fh:
        json.dump(_fixture_calc(calc, job_id), fh, indent=2, sort_keys=True)
        fh.write("\n")
    return path.relative_to(FIXTURE_DIR).as_posix()


def _fixture_calc(calc: dict, job_id: str) -> dict:
    """Keep harvested fixtures small and relevant to program equivalence."""
    out = {
        "job_id": str(calc.get("job_id") or job_id),
        "_m3_oracle_source_key": str(calc.get("_m3_oracle_source_key") or f"renders/{job_id}/calc.json"),
    }
    for key in ("function", "degree", "N", "n1", "times", "n_chunks", "pipeline_program_version"):
        if key in calc:
            out[key] = calc[key]
    pipeline = calc.get("pipeline")
    if isinstance(pipeline, dict) and pipeline:
        compact_pipeline = {key: pipeline[key] for key in PIPELINE_PROVENANCE_KEYS if key in pipeline}
        if compact_pipeline:
            out["pipeline"] = compact_pipeline
    params = _params(calc)
    if params:
        compact_params = {key: params[key] for key in PROGRAM_KEYS if key in params}
        if compact_params:
            out["params"] = compact_params
    return out


def _write_index(cases: list[dict], *, bucket: str):
    HARVEST_DIR.mkdir(parents=True, exist_ok=True)
    payload = {
        "version": 1,
        "source": {
            "bucket": bucket,
            "note": "Generated by scripts/harvest_program_m3_oracle_corpus.py.",
        },
        "cases": cases,
    }
    with (HARVEST_DIR / "corpus.json").open("w", encoding="utf-8") as fh:
        json.dump(payload, fh, indent=2, sort_keys=True)
        fh.write("\n")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Snapshot saved calc.json program payloads into the offline M3 oracle corpus."
    )
    parser.add_argument("--bucket", default=os.environ.get("BUCKET", "polypaint"), help="S3 bucket (default: %(default)s)")
    parser.add_argument("--jobs-file", default="", help="optional file of job ids or s3://.../calc.json keys")
    parser.add_argument(
        "--list-with",
        choices=("auto", "s5cmd", "boto3"),
        default="auto",
        help="how to discover calc.json keys when --jobs-file is omitted (default: %(default)s)",
    )
    parser.add_argument("--start-after-job", default="", help="skip listed S3 job prefixes <= this job id")
    parser.add_argument("--max-jobs", type=int, default=0, help="maximum job prefixes to scan")
    parser.add_argument("--max-scan", type=int, default=0, help="maximum calc.json objects to read after filtering/listing")
    parser.add_argument("--limit-cases", type=int, default=0, help="maximum matching calc payloads to write")
    parser.add_argument("--progress-every", type=int, default=100, help="print progress every N scanned calc files")
    parser.add_argument("--include-empty", action="store_true", help="also snapshot calc files without program payloads")
    parser.add_argument("--dry-run", action="store_true", help="scan and report without writing fixtures")
    args = parser.parse_args(argv)

    try:
        import boto3
    except ModuleNotFoundError as exc:
        raise SystemExit("boto3 is required to harvest from S3; run this with the project uv environment") from exc

    s3 = boto3.client("s3")
    if args.jobs_file:
        jobs = list(_iter_jobs_file(Path(args.jobs_file)))
    elif args.list_with in {"auto", "s5cmd"} and shutil.which("s5cmd"):
        jobs = _iter_job_ids_s5cmd(
            args.bucket,
            start_after_job=args.start_after_job,
            max_jobs=max(0, args.max_jobs),
        )
    elif args.list_with == "s5cmd":
        raise SystemExit("s5cmd was requested but is not installed")
    else:
        jobs = _iter_job_ids(
            s3,
            args.bucket,
            start_after_job=args.start_after_job,
            max_jobs=max(0, args.max_jobs),
        )

    cases = []
    scanned = missing = skipped = errors = 0
    for job_id in jobs:
        scanned += 1
        if args.max_scan and scanned > args.max_scan:
            scanned -= 1
            break
        if args.progress_every and (scanned == 1 or scanned % args.progress_every == 0):
            print(
                f"PROGRESS scanned={scanned} cases={len(cases)} "
                f"skipped_no_program={skipped} missing={missing} errors={errors} last={job_id}",
                file=sys.stderr,
                flush=True,
            )
        try:
            calc = _read_calc(s3, args.bucket, job_id)
        except Exception as exc:
            errors += 1
            print(f"ERROR   {job_id}: {exc}", file=sys.stderr)
            continue
        if calc is None:
            missing += 1
            continue
        if not args.include_empty and not _has_program_payload(calc):
            skipped += 1
            continue
        rel_path = f"harvested/{_safe_name(job_id)}.calc.json" if args.dry_run else _write_case(calc, job_id)
        cases.append({"name": job_id, "calc": rel_path})
        print(f"CASE    {job_id} -> {rel_path}")
        if args.limit_cases and len(cases) >= args.limit_cases:
            break
        if args.max_scan and scanned >= args.max_scan:
            break

    if not args.dry_run:
        _write_index(cases, bucket=args.bucket)
    print(
        "done "
        f"bucket={args.bucket} scanned={scanned} cases={len(cases)} "
        f"skipped_no_program={skipped} missing={missing} errors={errors} "
        f"dry_run={bool(args.dry_run)}"
    )
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
