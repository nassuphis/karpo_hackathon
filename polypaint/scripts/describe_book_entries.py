#!/usr/bin/env python3
"""CLI wrapper over lambda/book_describe.py — the engine now lives
server-side (Book tab Describe button); this wrapper keeps the local
dry-run workflow for prompt tuning: it calls Gemini directly and prints
the prose without saving unless --apply (which saves through the real
/save-book route, same as the lambda).

Requires GEMINI_API_KEY in the shell. stdlib + boto3."""
from __future__ import annotations

import argparse
import json
import os
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "lambda"))

import book_describe as eng  # noqa: E402


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--book", required=True)
    parser.add_argument("--model", default=eng.DEFAULT_MODEL)
    parser.add_argument("--apply", action="store_true", help="save; default prints only")
    parser.add_argument("--overwrite", action="store_true")
    parser.add_argument("--limit", type=int, default=0)
    args = parser.parse_args(argv)

    api_key = os.environ.get("GEMINI_API_KEY", "").strip()
    if not api_key:
        sys.exit("GEMINI_API_KEY is not set (free key: aistudio.google.com/apikey)")

    fetched = eng._storage({"path": "/fetch-book", "body": json.dumps({"id": args.book})})
    doc = fetched["book"]
    entries = doc.get("entries") or []
    if not entries:
        sys.exit(f"book {args.book} has no entries")

    used_titles = [str(e.get("title_override") or "").strip()
                   for e in entries if str(e.get("title_override") or "").strip()]
    changed = 0
    for idx, entry in enumerate(entries, start=1):
        if args.limit and changed >= args.limit:
            break
        has_prose = bool(str(entry.get("title_override") or "").strip()
                         or str(entry.get("body_override") or "").strip())
        if has_prose and not args.overwrite:
            print(f"[{idx}/{len(entries)}] {entry.get('artifact_id')}: has overrides, skipping")
            continue
        if changed:
            time.sleep(eng.FREE_TIER_PACING_S)
        image = eng._entry_preview_bytes(entry)
        report = eng._entry_report(args.book, entry)
        title, description = eng.describe_image(
            image, entry, report, model=args.model, api_key=api_key,
            angle=eng.ANGLES[(idx - 1) % len(eng.ANGLES)], used_titles=used_titles)
        print(f"[{idx}/{len(entries)}] {entry.get('artifact_id')}")
        print(f"    title: {title}")
        print(f"    body:  {description}")
        entry["title_override"] = title
        entry["body_override"] = description
        used_titles.append(title)
        changed += 1

    if not changed:
        print("nothing to do")
        return 0
    if not args.apply:
        print(f"\nDRY-RUN: {changed} entries described, nothing saved. "
              f"Re-run with --apply, or use the Book tab's Describe button.")
        return 0
    saved = eng._storage({"path": "/save-book", "body": json.dumps({"book": doc})})
    print(f"\nSaved {changed} descriptions to \"{saved['book'].get('name')}\". Compile to publish.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
