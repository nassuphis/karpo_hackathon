#!/usr/bin/env python3
import argparse
import json
import sys
from pathlib import Path

import boto3


ROOT = Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
if str(LAMBDA_DIR) not in sys.path:
    sys.path.insert(0, str(LAMBDA_DIR))

from handler_storage import (_compile_solve_score_program_payload, _solve_score_program_key,
                             _solve_score_program_v2_key, _put_program_v1_object)
from shared import BUCKET, is_missing_s3_error


def _catalog_entries():
    catalog_path = ROOT / "solve-score-programs" / "index.json"
    raw = json.loads(catalog_path.read_text(encoding="utf-8"))
    rows = raw if isinstance(raw, list) else raw.get("programs", [])
    if not isinstance(rows, list):
        raise RuntimeError(f"invalid solve-score-programs catalog: {catalog_path}")
    return rows


def _load_seed_program(entry):
    path_text = str((entry or {}).get("path") or "").strip()
    name = str((entry or {}).get("name") or "").strip()
    if not path_text:
        raise RuntimeError(f"catalog entry is missing path: {entry!r}")
    source_path = ROOT / path_text
    raw = json.loads(source_path.read_text(encoding="utf-8"))
    program = _compile_solve_score_program_payload(
        raw.get("name") or name,
        raw.get("chain"),
    )
    return source_path, program


def _key_exists(s3_client, bucket, key):
    # Only a genuine 404 is "absent"; a transient/throttle/access error
    # propagates so the seed never mis-reads an existing program as missing
    # (code-review-28 F13).
    try:
        s3_client.head_object(Bucket=bucket, Key=key)
        return True
    except Exception as exc:
        if is_missing_s3_error(exc):
            return False
        raise


def main():
    parser = argparse.ArgumentParser(description="Seed solve-score programs from solve-score-programs/index.json into S3.")
    parser.add_argument("--bucket", default=BUCKET, help="target S3 bucket (default: %(default)s)")
    parser.add_argument("--dry-run", action="store_true", help="show planned writes without uploading")
    parser.add_argument("--force", action="store_true", help="overwrite existing S3 objects")
    args = parser.parse_args()

    s3 = boto3.client("s3")
    created = 0
    skipped = 0
    overwritten = 0

    for entry in _catalog_entries():
        source_path, program = _load_seed_program(entry)
        key = _solve_score_program_key(program["id"])
        exists = _key_exists(s3, args.bucket, key)
        if exists and not args.force:
            skipped += 1
            print(f"SKIP  s3://{args.bucket}/{key}  ({source_path})")
            continue
        action = "PUT" if not exists else "OVERWRITE"
        if args.dry_run:
            print(f"{action:<9} s3://{args.bucket}/{key}  ({source_path})")
            if exists:
                overwritten += 1
            else:
                created += 1
            continue
        # Route through the single save primitive so an overwrite of an
        # already-migrated program also drops the stale v2 copy that fetch
        # prefers (code-review-28 F8).
        _put_program_v1_object(
            key, _solve_score_program_v2_key(program["id"]),
            (json.dumps(program, indent=2) + "\n").encode("utf-8"),
            s3_client=s3, bucket=args.bucket,
        )
        print(f"{action:<9} s3://{args.bucket}/{key}  ({source_path})")
        if exists:
            overwritten += 1
        else:
            created += 1

    print(
        f"done bucket={args.bucket} created={created} overwritten={overwritten} skipped={skipped}"
    )


if __name__ == "__main__":
    raise SystemExit(main())
