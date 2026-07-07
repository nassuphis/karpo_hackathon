#!/usr/bin/env python3
"""Migrate artifact previews to JPEG siblings for the mosaic walls.

deepzoom-speed.md Phase 2: the AllCol/AllPal walls fetch every artifact's
full preview.png (~1.7 GB + ~431 MB measured). This script gives every color
and palette artifact a `preview.jpg` sibling — quality 92, 4:4:4 chroma,
normalized to <=512px (Lanczos, never upscaled) — and records it additively
in the artifact meta:

    preview_jpg_key / preview_jpg_width / preview_jpg_height

The manifest builder prefers the jpg when those fields exist and falls back
to the png otherwise, so a half-migrated wall stays correct. preview.png is
NEVER touched: every other consumer keeps working.

Self-contained on purpose (stdlib + boto3 + Pillow): upload this one file to
AWS CloudShell (us-east-1), `pip install pillow`, and the ~2.2 GB of PNG
reads never touch a slow home line. Auth comes from the console session.

Dry-run by default; --apply to execute. Idempotent and resumable: artifacts
whose preview.jpg already exists (and whose meta carries the fields) are
skipped, so re-running after an interrupt — or as a --top-up after new
renders — only processes what's missing.

Visual sign-off (CloudShell has no display): `--sample 12` converts a random
sample and uploads ONLY the jpgs to `_scratch/preview_migration_samples/`,
then prints public before/after URL pairs (the bucket is public) to open in
browser tabs. Delete the scratch prefix after sign-off.
"""
from __future__ import annotations

import argparse
import concurrent.futures
import io
import json
import random
import sys
import threading

import boto3
from botocore.exceptions import ClientError

try:
    from PIL import Image
except ImportError:  # pragma: no cover - guidance for CloudShell
    print("Pillow is required: pip install pillow", file=sys.stderr)
    raise


DEFAULT_BUCKET = "polypaint"
CACHE_IMMUTABLE = "public, max-age=31536000, immutable"
MAX_PX = 512
JPEG_QUALITY = 92
SAMPLE_PREFIX = "_scratch/preview_migration_samples"
FAMILIES = ("color", "palettes")


def convert_png_to_jpg(png_bytes, *, max_px=MAX_PX, quality=JPEG_QUALITY):
    """Returns (jpg_bytes, (width, height), (source_width, source_height)).

    <=max_px normalization collapses the legacy 1024/512 preview split
    (deepzoom-speed.md §2.4); 4:4:4 subsampling protects hard palette edges."""
    img = Image.open(io.BytesIO(png_bytes))
    img.load()
    if img.mode != "RGB":
        img = img.convert("RGB")
    source_size = img.size
    width, height = img.size
    largest = max(width, height)
    if largest > max_px:
        scale = max_px / float(largest)
        img = img.resize(
            (max(1, round(width * scale)), max(1, round(height * scale))),
            Image.LANCZOS,
        )
    out = io.BytesIO()
    img.save(out, format="JPEG", quality=quality, subsampling=0, optimize=True)
    return out.getvalue(), img.size, source_size


def merged_meta_fields(meta, jpg_key, width, height):
    """Additive merge — string values, matching the color overlay store."""
    merged = dict(meta)
    merged["preview_jpg_key"] = jpg_key
    merged["preview_jpg_width"] = str(int(width))
    merged["preview_jpg_height"] = str(int(height))
    return merged


def _head(s3, bucket, key):
    try:
        return s3.head_object(Bucket=bucket, Key=key)
    except ClientError as exc:
        code = exc.response.get("Error", {}).get("Code", "")
        if code in {"404", "NoSuchKey", "NotFound"}:
            return None
        raise


def _get_json(s3, bucket, key):
    try:
        obj = s3.get_object(Bucket=bucket, Key=key)
    except ClientError as exc:
        code = exc.response.get("Error", {}).get("Code", "")
        if code in {"404", "NoSuchKey", "NotFound"}:
            return None
        raise
    try:
        data = json.loads(obj["Body"].read())
    except (json.JSONDecodeError, UnicodeDecodeError):
        return None
    return data if isinstance(data, dict) else None


def process_artifact(s3, bucket, family, prefix, *, apply, skip_keys=(),
                     max_px=MAX_PX, quality=JPEG_QUALITY):
    """Returns (status, note). Status: converted | would_convert | ok |
    meta_repaired | missing | no_meta | skipped | invalid_image."""
    png_key = prefix + "preview.png"
    jpg_key = prefix + "preview.jpg"
    meta_key = prefix + "meta.json"
    if png_key in skip_keys:
        return "skipped", ""

    meta = _get_json(s3, bucket, meta_key)
    if family == "palettes" and meta is None:
        # a palette without meta.json is invisible to the wall — nothing to do
        return "no_meta", ""
    meta = meta or {}

    jpg_head = _head(s3, bucket, jpg_key)
    if jpg_head is not None:
        if str(meta.get("preview_jpg_key") or "").strip():
            return "ok", ""
        # interrupted between upload and meta write: finish the meta merge
        jpg_meta = jpg_head.get("Metadata") or {}
        width = int(jpg_meta.get("width") or 0)
        height = int(jpg_meta.get("height") or 0)
        if not (width and height):
            return "invalid_image", "existing jpg lacks dimension metadata"
        if not apply:
            return "would_convert", "meta merge only"
        _put_meta(s3, bucket, meta_key, merged_meta_fields(meta, jpg_key, width, height))
        return "meta_repaired", ""

    if not apply:
        # dry-run stays cheap: no png download, no conversion — just prove
        # the work exists. Real sizes come from --sample / --apply output.
        png_head = _head(s3, bucket, png_key)
        if png_head is None:
            return "missing", ""
        size_kb = int(png_head.get("ContentLength") or 0) // 1024
        return "would_convert", f"png {size_kb}KB (conversion deferred to --apply)"

    # S3 streams occasionally truncate under concurrency (seen live on
    # CloudShell: a healthy png raised "broken data stream"). A short read
    # or decode failure retries with a fresh GET before being declared
    # genuinely corrupt.
    png_bytes = jpg_bytes = None
    last_error = ""
    for _ in range(3):
        try:
            png_obj = s3.get_object(Bucket=bucket, Key=png_key)
        except ClientError as exc:
            code = exc.response.get("Error", {}).get("Code", "")
            if code in {"404", "NoSuchKey", "NotFound"}:
                return "missing", ""
            raise
        data = png_obj["Body"].read()
        expected = int(png_obj.get("ContentLength") or 0)
        if expected and len(data) != expected:
            last_error = f"short read {len(data)}/{expected}"
            continue
        try:
            jpg_bytes, (width, height), (src_w, src_h) = convert_png_to_jpg(
                data, max_px=max_px, quality=quality)
        except Exception as exc:  # noqa: BLE001 — retry, then report
            last_error = str(exc)
            continue
        png_bytes = data
        break
    if png_bytes is None or jpg_bytes is None:
        return "invalid_image", last_error

    note = f"{src_w}x{src_h} png {len(png_bytes) // 1024}KB -> {width}x{height} jpg {len(jpg_bytes) // 1024}KB"
    if max(src_w, src_h) > 1024:
        note += " ANOMALY: source larger than any known preview"

    object_meta = {"width": str(width), "height": str(height)}
    if width == height:
        object_meta["pix"] = str(width)
    s3.put_object(
        Bucket=bucket,
        Key=jpg_key,
        Body=jpg_bytes,
        ContentType="image/jpeg",
        CacheControl=CACHE_IMMUTABLE,
        Metadata=object_meta,
    )
    _put_meta(s3, bucket, meta_key, merged_meta_fields(meta, jpg_key, width, height))
    return "converted", note


def _put_meta(s3, bucket, meta_key, merged):
    s3.put_object(
        Bucket=bucket,
        Key=meta_key,
        Body=json.dumps(merged, separators=(",", ":")).encode("utf-8"),
        ContentType="application/json",
    )


def verify_artifact(s3, bucket, family, prefix):
    """Returns (status, note): ok | missing_jpg | missing_meta_fields |
    dims_mismatch | no_preview | no_meta."""
    png_key = prefix + "preview.png"
    jpg_key = prefix + "preview.jpg"
    if _head(s3, bucket, png_key) is None:
        return "no_preview", ""
    meta = _get_json(s3, bucket, prefix + "meta.json")
    if family == "palettes" and meta is None:
        return "no_meta", ""
    meta = meta or {}
    jpg_head = _head(s3, bucket, jpg_key)
    if jpg_head is None:
        return "missing_jpg", ""
    if not str(meta.get("preview_jpg_key") or "").strip():
        return "missing_meta_fields", ""
    jpg_meta = jpg_head.get("Metadata") or {}
    if (str(meta.get("preview_jpg_width")) != str(jpg_meta.get("width"))
            or str(meta.get("preview_jpg_height")) != str(jpg_meta.get("height"))):
        return "dims_mismatch", (
            f"meta {meta.get('preview_jpg_width')}x{meta.get('preview_jpg_height')}"
            f" vs object {jpg_meta.get('width')}x{jpg_meta.get('height')}")
    return "ok", ""


def _iter_child_prefixes(s3, bucket, prefix):
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=bucket, Prefix=prefix, Delimiter="/"):
        for item in page.get("CommonPrefixes", []):
            child = str(item.get("Prefix") or "")
            if child:
                yield child


def iter_artifact_prefixes(s3, bucket, families):
    for job_prefix in _iter_child_prefixes(s3, bucket, "renders/"):
        job_id = job_prefix.split("/")[1]
        if not job_id or job_id.startswith("_"):
            continue
        for family in families:
            for artifact_prefix in _iter_child_prefixes(s3, bucket, f"{job_prefix}{family}/"):
                yield family, artifact_prefix


def run_sample(s3, bucket, families, count, seed, region):
    targets = list(iter_artifact_prefixes(s3, bucket, families))
    random.Random(seed).shuffle(targets)
    base = f"https://{bucket}.s3.{region}.amazonaws.com/"
    printed = 0
    for family, prefix in targets:
        if printed >= count:
            break
        png_key = prefix + "preview.png"
        try:
            png_bytes = s3.get_object(Bucket=bucket, Key=png_key)["Body"].read()
            jpg_bytes, (w, h), (sw, sh) = convert_png_to_jpg(png_bytes)
        except Exception:  # noqa: BLE001 — sample skips broken artifacts
            continue
        artifact = prefix.rstrip("/").split("/")[-1]
        sample_key = f"{SAMPLE_PREFIX}/{artifact}.jpg"
        s3.put_object(Bucket=bucket, Key=sample_key, Body=jpg_bytes,
                      ContentType="image/jpeg")
        printed += 1
        print(f"[{printed}/{count}] {family} {artifact}  {sw}x{sh} "
              f"{len(png_bytes) // 1024}KB -> {w}x{h} {len(jpg_bytes) // 1024}KB")
        print(f"  before: {base}{png_key}")
        print(f"  after:  {base}{sample_key}")
    print(f"\nInspect the pairs in browser tabs. Clean up with:\n"
          f"  aws s3 rm --recursive s3://{bucket}/{SAMPLE_PREFIX}/")


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Create <=512px preview.jpg siblings + meta fields for the mosaic walls."
    )
    parser.add_argument("--bucket", default=DEFAULT_BUCKET, help="S3 bucket (default: %(default)s)")
    parser.add_argument("--family", choices=["color", "palettes", "all"], default="all")
    parser.add_argument("--limit", type=int, default=0, help="maximum artifacts to inspect")
    parser.add_argument("--workers", type=int, default=8,
                        help="concurrent workers (CloudShell has 1 vCPU; 8 overlaps IO and decode)")
    parser.add_argument("--apply", action="store_true", help="write jpgs + meta; default is dry-run")
    parser.add_argument("--sample", type=int, default=0, metavar="N",
                        help="convert N random previews to %s and print URL pairs, then exit" % SAMPLE_PREFIX)
    parser.add_argument("--verify", action="store_true",
                        help="check jpg + meta fields for every artifact, report gaps, then exit")
    parser.add_argument("--skip", action="append", default=[], metavar="PREVIEW_KEY",
                        help="preview.png key to leave on PNG (repeatable escape hatch)")
    parser.add_argument("--max-px", type=int, default=MAX_PX)
    parser.add_argument("--quality", type=int, default=JPEG_QUALITY)
    parser.add_argument("--seed", type=int, default=7, help="sample shuffle seed")
    parser.add_argument("--quiet", action="store_true", help="summary only")
    args = parser.parse_args(argv)

    families = FAMILIES if args.family == "all" else (args.family,)
    s3 = boto3.client("s3")
    region = s3.meta.region_name or "us-east-1"

    if args.sample:
        run_sample(s3, args.bucket, families, args.sample, args.seed, region)
        return 0

    counts = {}
    lock = threading.Lock()
    skip_keys = set(args.skip)

    def work(item):
        family, prefix = item
        try:
            if args.verify:
                status, note = verify_artifact(s3, args.bucket, family, prefix)
            else:
                status, note = process_artifact(
                    s3, args.bucket, family, prefix, apply=args.apply,
                    skip_keys=skip_keys, max_px=args.max_px, quality=args.quality)
        except Exception as exc:  # noqa: BLE001 — count + report, keep sweeping
            status, note = "errors", str(exc)
        with lock:
            counts[status] = counts.get(status, 0) + 1
        if not args.quiet and (note or status == "errors"):
            stream = sys.stderr if status == "errors" else sys.stdout
            print(f"{status.upper():<14} {prefix} {note}", file=stream)

    def items():
        for idx, item in enumerate(iter_artifact_prefixes(s3, args.bucket, families)):
            if args.limit and idx >= args.limit:
                return
            yield item

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, args.workers)) as pool:
        for _ in pool.map(work, items()):
            pass

    mode = "VERIFY" if args.verify else ("APPLY" if args.apply else "DRY-RUN")
    summary = " ".join(f"{k}={v}" for k, v in sorted(counts.items()))
    print(f"{mode} done bucket={args.bucket} families={','.join(families)} {summary}")
    bad = counts.get("errors", 0)
    if args.verify:
        bad += counts.get("missing_jpg", 0) + counts.get("missing_meta_fields", 0) + counts.get("dims_mismatch", 0)
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main())
