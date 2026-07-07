#!/usr/bin/env python3
"""Backfill Cache-Control (+ preview dimension repair) on immutable objects.

deepzoom-speed.md Phase 1: browser-fetched artifact objects live at immutable,
artifact/export-scoped keys but were uploaded without Cache-Control, so
browsers mostly re-download them. This script copies objects in place adding

    Cache-Control: public, max-age=31536000, immutable

and, for preview.png objects, repairs legacy width/height/pix metadata to the
measured PNG-header dimensions (old color previews carry the FULL render size,
e.g. 5000/10000 — deepzoom-speed.md §2.5). The copy is server-side: no image
bytes are downloaded, only ~33-byte ranged GETs for the PNG header.

Self-contained on purpose (stdlib + boto3, no repo imports): runs as-is in
AWS CloudShell (us-east-1), where auth comes from the console session and the
transfers never touch a slow home line.

Default scope: color + palette artifact previews (~2.8k objects) and deepzoom
image.dzi + viewer.html (~500). --scope deepzoom-tiles adds the ~1M tile
objects (~$5 of COPY requests; roughly an hour at 50 workers). meta.json and
deepzoom_latest.json pointers are mutable and are never touched.

Dry-run by default; pass --apply to execute. Idempotent: objects already
carrying the target Cache-Control (and honest dims) are skipped, so re-runs
and resumes are free.
"""
from __future__ import annotations

import argparse
import concurrent.futures
import sys
import threading

import boto3
from botocore.exceptions import ClientError


DEFAULT_BUCKET = "polypaint"
CACHE_IMMUTABLE = "public, max-age=31536000, immutable"
PNG_SIG = b"\x89PNG\r\n\x1a\n"

# Deterministic fallback only — the head's ContentType always wins. Never let
# a CLI guess (.dzi would come out wrong).
CONTENT_TYPE_BY_SUFFIX = {
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".jpeg": "image/jpeg",
    ".dzi": "application/xml",
    ".html": "text/html; charset=utf-8",
}

# CopyObject rejects sources above 5 GB; nothing in scope is close.
COPY_SIZE_LIMIT = 5 * 1024 * 1024 * 1024


def png_dimensions_from_header(data):
    if len(data) < 24 or data[:8] != PNG_SIG or data[12:16] != b"IHDR":
        return None
    width = int.from_bytes(data[16:20], "big")
    height = int.from_bytes(data[20:24], "big")
    if width <= 0 or height <= 0:
        return None
    return width, height


def repaired_dimension_metadata(meta, width, height, *, fill_missing):
    """Return (changed, repaired_meta) with width/height/pix set to measured
    values. Mirrors scripts/repair_preview_metadata.py semantics: only fields
    that are present-and-wrong are fixed unless fill_missing adds absent ones."""
    expected = {"width": str(width), "height": str(height)}
    if width == height:
        expected["pix"] = str(width)
    present = {k for k in ("width", "height", "pix") if k in meta}
    if not present and not fill_missing:
        return False, dict(meta)

    repaired = dict(meta)
    repaired["width"] = expected["width"]
    repaired["height"] = expected["height"]
    if "pix" in expected:
        if fill_missing or "pix" in present:
            repaired["pix"] = expected["pix"]
    elif "pix" in present:
        repaired.pop("pix", None)

    changed = any(meta.get(k) != repaired.get(k) for k in ("width", "height", "pix"))
    return changed, repaired


def _copy_args_from_head(head, key):
    suffix = "." + key.rsplit(".", 1)[-1] if "." in key else ""
    args = {"ContentType": head.get("ContentType") or CONTENT_TYPE_BY_SUFFIX.get(suffix, "application/octet-stream")}
    for name in ("ContentDisposition", "ContentEncoding", "ContentLanguage"):
        if head.get(name):
            args[name] = head[name]
    return args


def process_key(s3, bucket, key, *, apply, fill_missing):
    """Returns (status, detail). Status: ok | copied | would_copy | missing |
    invalid_png | too_large."""
    try:
        head = s3.head_object(Bucket=bucket, Key=key)
    except ClientError as exc:
        code = exc.response.get("Error", {}).get("Code", "")
        if code in {"404", "NoSuchKey", "NotFound"}:
            return "missing", ""
        raise
    if int(head.get("ContentLength") or 0) > COPY_SIZE_LIMIT:
        return "too_large", ""

    meta = dict(head.get("Metadata") or {})
    repaired = dict(meta)
    dims_changed = False
    dims_note = ""
    if key.endswith("/preview.png"):
        obj = s3.get_object(Bucket=bucket, Key=key, Range="bytes=0-32")
        dims = png_dimensions_from_header(obj["Body"].read())
        if dims is None:
            return "invalid_png", ""
        dims_changed, repaired = repaired_dimension_metadata(
            meta, dims[0], dims[1], fill_missing=fill_missing)
        if dims_changed:
            dims_note = (
                f"dims {meta.get('width')}x{meta.get('height')}"
                f" -> {repaired.get('width')}x{repaired.get('height')}"
            )

    needs_cache = head.get("CacheControl") != CACHE_IMMUTABLE
    if not needs_cache and not dims_changed:
        return "ok", ""
    if not apply:
        return "would_copy", dims_note
    s3.copy_object(
        Bucket=bucket,
        Key=key,
        CopySource={"Bucket": bucket, "Key": key},
        Metadata=repaired,
        MetadataDirective="REPLACE",
        CacheControl=CACHE_IMMUTABLE,
        **_copy_args_from_head(head, key),
    )
    return "copied", dims_note


def _iter_child_prefixes(s3, bucket, prefix):
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=bucket, Prefix=prefix, Delimiter="/"):
        for item in page.get("CommonPrefixes", []):
            child = str(item.get("Prefix") or "")
            if child:
                yield child


def iter_preview_keys(s3, bucket):
    """renders/{job}/{color|palettes}/{artifact}/preview.png without listing
    the ~1.2M-object renders/ prefix recursively."""
    for job_prefix in _iter_child_prefixes(s3, bucket, "renders/"):
        job_id = job_prefix.split("/")[1]
        if not job_id or job_id.startswith("_"):
            continue
        for family in ("color", "palettes"):
            for artifact_prefix in _iter_child_prefixes(s3, bucket, f"{job_prefix}{family}/"):
                yield artifact_prefix + "preview.png"


def iter_deepzoom_meta_keys(s3, bucket):
    """deepzoom/{job}/{export}/image.dzi + viewer.html — the tiny per-export
    files — without listing the ~1M tiles."""
    for job_prefix in _iter_child_prefixes(s3, bucket, "deepzoom/"):
        for export_prefix in _iter_child_prefixes(s3, bucket, job_prefix):
            yield export_prefix + "image.dzi"
            yield export_prefix + "viewer.html"


def iter_deepzoom_tile_keys(s3, bucket):
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=bucket, Prefix="deepzoom/"):
        for obj in page.get("Contents", []):
            key = str(obj.get("Key") or "")
            if "/image_files/" in key:
                yield key


def iter_target_keys(s3, bucket, scopes):
    if "previews" in scopes:
        yield from iter_preview_keys(s3, bucket)
    if "deepzoom-meta" in scopes:
        yield from iter_deepzoom_meta_keys(s3, bucket)
    if "deepzoom-tiles" in scopes:
        yield from iter_deepzoom_tile_keys(s3, bucket)


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Copy-in-place immutable Cache-Control (+ preview dims repair) backfill."
    )
    parser.add_argument("--bucket", default=DEFAULT_BUCKET, help="S3 bucket (default: %(default)s)")
    parser.add_argument(
        "--scope",
        action="append",
        choices=["previews", "deepzoom-meta", "deepzoom-tiles"],
        help="repeatable; default: previews + deepzoom-meta",
    )
    parser.add_argument("--limit", type=int, default=0, help="maximum keys to inspect")
    parser.add_argument("--workers", type=int, default=16, help="concurrent HEAD/copy workers")
    parser.add_argument("--fill-missing", action="store_true",
                        help="also stamp width/height metadata on previews that have none")
    parser.add_argument("--apply", action="store_true", help="perform copies; default is dry-run")
    parser.add_argument("--quiet", action="store_true", help="summary only")
    args = parser.parse_args(argv)

    scopes = args.scope or ["previews", "deepzoom-meta"]
    s3 = boto3.client("s3")
    counts = {"checked": 0, "ok": 0, "copied": 0, "would_copy": 0,
              "dims_repaired": 0, "missing": 0, "invalid_png": 0,
              "too_large": 0, "errors": 0}
    lock = threading.Lock()

    def work(key):
        try:
            status, note = process_key(
                s3, args.bucket, key, apply=args.apply, fill_missing=args.fill_missing)
        except Exception as exc:  # noqa: BLE001 — count + report, keep sweeping
            with lock:
                counts["errors"] += 1
            if not args.quiet:
                print(f"ERROR        s3://{args.bucket}/{key} {exc}", file=sys.stderr)
            return
        with lock:
            counts[status] += 1
            if note:
                counts["dims_repaired"] += 1
        if note and not args.quiet:
            print(f"{status.upper():<12} s3://{args.bucket}/{key} {note}")

    def keys():
        for idx, key in enumerate(iter_target_keys(s3, args.bucket, scopes)):
            if args.limit and idx >= args.limit:
                return
            counts["checked"] += 1
            yield key

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, args.workers)) as pool:
        for _ in pool.map(work, keys()):
            pass

    mode = "APPLY" if args.apply else "DRY-RUN"
    print(
        f"{mode} done bucket={args.bucket} scopes={','.join(scopes)} "
        f"checked={counts['checked']} ok={counts['ok']} copied={counts['copied']} "
        f"would_copy={counts['would_copy']} dims_repaired={counts['dims_repaired']} "
        f"missing={counts['missing']} invalid_png={counts['invalid_png']} "
        f"too_large={counts['too_large']} errors={counts['errors']}"
    )
    return 1 if counts["errors"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
