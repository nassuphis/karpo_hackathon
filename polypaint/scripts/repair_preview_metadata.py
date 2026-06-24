#!/usr/bin/env python3
"""Repair stale width/height metadata on immutable color artifact previews.

Autolevels used to copy the full-size image dimensions onto preview.png. This
script treats the PNG IHDR as authoritative and rewrites only dimension metadata
that is present and wrong, unless --fill-missing is supplied.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

import boto3
from botocore.exceptions import ClientError


ROOT = Path(__file__).resolve().parents[1]
LAMBDA_DIR = ROOT / "lambda"
if str(LAMBDA_DIR) not in sys.path:
    sys.path.insert(0, str(LAMBDA_DIR))

from shared import BUCKET  # noqa: E402


PNG_SIG = b"\x89PNG\r\n\x1a\n"


def _png_dimensions_from_header(data: bytes):
    if len(data) < 24 or data[:8] != PNG_SIG or data[12:16] != b"IHDR":
        return None
    width = int.from_bytes(data[16:20], "big")
    height = int.from_bytes(data[20:24], "big")
    if width <= 0 or height <= 0:
        return None
    return width, height


def _read_preview_dimensions(s3, bucket: str, key: str):
    obj = s3.get_object(Bucket=bucket, Key=key, Range="bytes=0-32")
    return _png_dimensions_from_header(obj["Body"].read())


def _iter_preview_keys(
    s3,
    bucket: str,
    *,
    keys_file: str = "",
    offset: int = 0,
    job_id: str = "",
    artifact_id: str = "",
    prefix: str = "",
    limit: int = 0,
    start_after_job: str = "",
    max_jobs: int = 0,
    stats: dict | None = None,
):
    if keys_file:
        yielded = 0
        skipped = 0
        jobs = set()
        for key in _iter_keys_file(keys_file, bucket):
            if offset and skipped < offset:
                skipped += 1
                continue
            job = _job_id_from_preview_key(key)
            if stats is not None and job:
                jobs.add(job)
                stats["jobs_scanned"] = len(jobs)
                stats.setdefault("first_job", job)
                if not stats.get("first_job"):
                    stats["first_job"] = job
                stats["last_job"] = job
            yield key
            yielded += 1
            if limit and yielded >= limit:
                return
        return

    if artifact_id and not job_id:
        raise ValueError("--artifact-id requires --job-id")
    if artifact_id:
        yield f"renders/{job_id}/color/{artifact_id}/preview.png"
        return

    if not prefix:
        yielded = 0
        job_ids = [job_id] if job_id else _iter_job_ids(
            s3,
            bucket,
            start_after_job=start_after_job,
            max_jobs=max_jobs,
            stats=stats,
        )
        for jid in job_ids:
            for color_prefix in _iter_color_artifact_prefixes(s3, bucket, jid):
                yield color_prefix + "preview.png"
                yielded += 1
                if limit and yielded >= limit:
                    return
        return

    scan_prefix = prefix
    seen = 0
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=bucket, Prefix=scan_prefix):
        for obj in page.get("Contents", []):
            key = str(obj.get("Key") or "")
            if not _is_color_preview_key(key):
                continue
            yield key
            seen += 1
            if limit and seen >= limit:
                return


def _iter_job_ids(s3, bucket: str, *, start_after_job: str = "", max_jobs: int = 0, stats: dict | None = None):
    emitted = 0
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=bucket, Prefix="renders/", Delimiter="/"):
        for item in page.get("CommonPrefixes", []):
            parts = str(item.get("Prefix") or "").split("/")
            if len(parts) < 2 or not parts[1] or parts[1].startswith("_"):
                continue
            job_id = parts[1]
            if start_after_job and job_id <= start_after_job:
                continue
            emitted += 1
            if stats is not None:
                stats["jobs_scanned"] = emitted
                stats.setdefault("first_job", job_id)
                stats["last_job"] = job_id
            yield job_id
            if max_jobs and emitted >= max_jobs:
                return


def _iter_color_artifact_prefixes(s3, bucket: str, job_id: str):
    paginator = s3.get_paginator("list_objects_v2")
    base = f"renders/{job_id}/color/"
    for page in paginator.paginate(Bucket=bucket, Prefix=base, Delimiter="/"):
        for item in page.get("CommonPrefixes", []):
            prefix = str(item.get("Prefix") or "")
            if prefix.startswith(base):
                yield prefix


def _iter_keys_file(path: str, bucket: str):
    with open(path, "r", encoding="utf-8") as fh:
        for raw in fh:
            key = _parse_key_line(raw, bucket)
            if key and _is_color_preview_key(key):
                yield key


def _parse_key_line(line: str, bucket: str):
    text = line.strip()
    if not text or text.startswith("#"):
        return ""
    for token in reversed(text.split()):
        if token.startswith("s3://"):
            prefix = f"s3://{bucket}/"
            if token.startswith(prefix):
                return token[len(prefix):]
            return ""
    return text


def _job_id_from_preview_key(key: str):
    parts = str(key or "").split("/")
    return parts[1] if len(parts) >= 2 and parts[0] == "renders" else ""


def _is_color_preview_key(key: str) -> bool:
    parts = key.split("/")
    return (
        len(parts) == 5
        and parts[0] == "renders"
        and parts[2] == "color"
        and parts[4] == "preview.png"
        and bool(parts[1])
        and bool(parts[3])
    )


def _metadata_needs_repair(meta: dict, width: int, height: int, *, fill_missing: bool):
    expected = {
        "width": str(width),
        "height": str(height),
    }
    if width == height:
        expected["pix"] = str(width)
    present = {k for k in ("width", "height", "pix") if k in meta}
    if not present and not fill_missing:
        return False, meta

    repaired = dict(meta)
    if fill_missing or present:
        repaired["width"] = expected["width"]
        repaired["height"] = expected["height"]
    if "pix" in expected and (fill_missing or "pix" in present):
        repaired["pix"] = expected["pix"]
    elif "pix" in present:
        repaired.pop("pix", None)

    for key in ("width", "height", "pix"):
        if meta.get(key) != repaired.get(key):
            return True, repaired
    return False, repaired


def _copy_args_from_head(head: dict):
    args = {}
    for src, dst in (
        ("ContentType", "ContentType"),
        ("CacheControl", "CacheControl"),
        ("ContentDisposition", "ContentDisposition"),
        ("ContentEncoding", "ContentEncoding"),
        ("ContentLanguage", "ContentLanguage"),
        ("Expires", "Expires"),
        ("WebsiteRedirectLocation", "WebsiteRedirectLocation"),
    ):
        if head.get(src):
            args[dst] = head[src]
    return args


def _repair_key(s3, bucket: str, key: str, *, apply: bool, fill_missing: bool):
    head = s3.head_object(Bucket=bucket, Key=key)
    meta = dict(head.get("Metadata") or {})
    dims = _read_preview_dimensions(s3, bucket, key)
    if not dims:
        return "invalid_png", key, meta, meta
    width, height = dims
    needs_repair, repaired = _metadata_needs_repair(meta, width, height, fill_missing=fill_missing)
    if not needs_repair:
        return "ok", key, meta, repaired
    if apply:
        s3.copy_object(
            Bucket=bucket,
            Key=key,
            CopySource={"Bucket": bucket, "Key": key},
            Metadata=repaired,
            MetadataDirective="REPLACE",
            **_copy_args_from_head(head),
        )
    return "repaired" if apply else "would_repair", key, meta, repaired


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Repair stale preview.png dimension metadata under renders/<job>/color/<artifact>/."
    )
    parser.add_argument("--bucket", default=BUCKET, help="S3 bucket (default: %(default)s)")
    parser.add_argument("--keys-file", default="", help="file containing preview keys or s3://bucket/key lines")
    parser.add_argument("--offset", type=int, default=0, help="skip this many entries from --keys-file before processing")
    parser.add_argument("--job-id", default="", help="limit to one compute job")
    parser.add_argument("--artifact-id", default="", help="limit to one color artifact; requires --job-id")
    parser.add_argument("--prefix", default="", help="custom S3 prefix to scan")
    parser.add_argument("--limit", type=int, default=0, help="maximum preview keys to inspect")
    parser.add_argument("--start-after-job", default="", help="resume after this compute job id")
    parser.add_argument("--max-jobs", type=int, default=0, help="maximum compute job prefixes to scan")
    parser.add_argument("--fill-missing", action="store_true", help="also add width/height metadata when absent")
    parser.add_argument("--apply", action="store_true", help="rewrite S3 metadata; default is dry-run")
    parser.add_argument("--quiet", action="store_true", help="suppress per-object output; print summary only")
    args = parser.parse_args(argv)

    if args.keys_file and any((args.prefix, args.job_id, args.artifact_id, args.start_after_job, args.max_jobs)):
        parser.error("--keys-file cannot be combined with --prefix/--job-id/--artifact-id/--start-after-job/--max-jobs")
    if args.prefix and (args.job_id or args.artifact_id):
        parser.error("--prefix cannot be combined with --job-id/--artifact-id")

    s3 = boto3.client("s3")
    counts = {
        "checked": 0,
        "ok": 0,
        "would_repair": 0,
        "repaired": 0,
        "invalid_png": 0,
        "missing": 0,
        "errors": 0,
    }
    scan_stats = {"jobs_scanned": 0, "first_job": "", "last_job": ""}

    for key in _iter_preview_keys(
        s3,
        args.bucket,
        keys_file=args.keys_file.strip(),
        offset=max(0, args.offset),
        job_id=args.job_id.strip(),
        artifact_id=args.artifact_id.strip(),
        prefix=args.prefix.strip(),
        limit=max(0, args.limit),
        start_after_job=args.start_after_job.strip(),
        max_jobs=max(0, args.max_jobs),
        stats=scan_stats,
    ):
        counts["checked"] += 1
        try:
            status, key, old_meta, new_meta = _repair_key(
                s3,
                args.bucket,
                key,
                apply=args.apply,
                fill_missing=args.fill_missing,
            )
            counts[status] += 1
            if status in {"would_repair", "repaired", "invalid_png"} and not args.quiet:
                print(f"{status.upper():<12} s3://{args.bucket}/{key} old={_dims(old_meta)} new={_dims(new_meta)}")
        except ClientError as exc:
            code = exc.response.get("Error", {}).get("Code", type(exc).__name__)
            if code in {"404", "NoSuchKey", "NotFound"}:
                counts["missing"] += 1
                if not args.quiet:
                    print(f"MISSING      s3://{args.bucket}/{key} {code}")
                continue
            counts["errors"] += 1
            if not args.quiet:
                print(f"ERROR        s3://{args.bucket}/{key} {code}", file=sys.stderr)

    mode = "APPLY" if args.apply else "DRY-RUN"
    print(
        f"{mode} done bucket={args.bucket} checked={counts['checked']} ok={counts['ok']} "
        f"would_repair={counts['would_repair']} repaired={counts['repaired']} "
        f"invalid_png={counts['invalid_png']} missing={counts['missing']} errors={counts['errors']} "
        f"jobs_scanned={scan_stats['jobs_scanned']} first_job={scan_stats['first_job']} "
        f"last_job={scan_stats['last_job']}"
    )
    return 1 if counts["errors"] else 0


def _dims(meta: dict) -> dict:
    return {key: meta.get(key) for key in ("width", "height", "pix") if key in meta}


if __name__ == "__main__":
    raise SystemExit(main())
