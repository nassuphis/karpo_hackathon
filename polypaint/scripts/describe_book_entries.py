#!/usr/bin/env python3
"""Generate evocative titles + descriptions for a book's entries via Gemini.

Walks the book document, sends each entry's 512px preview.jpg (public URL,
plenty of signal for a VLM — see 2026-07-08 discussion) plus its technical
provenance to the Gemini API, and writes the results into the entries'
existing title_override / body_override fields. The verso template already
renders both (book_tex._verso_report_page), so the next Compile carries the
prose into the PDF and the flipbook with zero backend changes.

Saves go through the REAL /save-book route (storage lambda invoke), so
validation and saved_at semantics hold — never a raw S3 write of the doc.

Dry-run by default: prints every proposed title/description for sign-off.
--apply saves. --overwrite regenerates entries that already have overrides
(default skips them, so reruns are cheap and hand-edited prose survives).

Requires GEMINI_API_KEY (free tier: aistudio.google.com/apikey; ~10 RPM,
hence the pacing sleep). stdlib + boto3 only.
"""
from __future__ import annotations

import argparse
import base64
import json
import os
import sys
import time
import urllib.error
import urllib.request

import boto3

BUCKET_BASE = "https://polypaint.s3.us-east-1.amazonaws.com/"
STORAGE_FUNCTION = os.environ.get("STORAGE_FUNCTION", "polypaint-storage")
GEMINI_URL = "https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent"
DEFAULT_MODEL = "gemini-2.5-flash"
FREE_TIER_PACING_S = 5.0   # ~10 RPM free tier: stay comfortably under

# Edit freely — this is the whole personality of the output.
PROMPT = """You are titling and describing an abstract generative artwork for a
fine-art book. The image is rendered from the roots of polynomials; its
technical provenance:

{provenance}

Return JSON: {{"title": ..., "description": ...}}.
- title: 2-4 evocative words, no quotes, not techy, no "untitled".
- description: 2-3 sentences about what the eye actually finds — form,
  color, movement, depth. Grounded and vivid; no purple prose, no
  mathematics jargon, never mention polynomials, rendering or pixels.
"""


def provenance_lines(entry, report):
    rows = (report or {}).get("summary_rows") or []
    lines = [f"- {k}: {v}" for k, v in rows]
    return "\n".join(lines) or f"- artifact: {entry.get('artifact_id', '')}"


def build_request(image_bytes, entry, report):
    return {
        "contents": [{
            "parts": [
                {"inline_data": {"mime_type": "image/jpeg",
                                 "data": base64.b64encode(image_bytes).decode()}},
                {"text": PROMPT.format(provenance=provenance_lines(entry, report))},
            ],
        }],
        "generationConfig": {
            "temperature": 0.9,
            "responseMimeType": "application/json",
        },
    }


def parse_response(payload):
    """Returns (title, description) or raises with the API's own words."""
    if "error" in payload:
        raise RuntimeError(f"Gemini: {payload['error'].get('message', payload['error'])}")
    try:
        text = payload["candidates"][0]["content"]["parts"][0]["text"]
    except (KeyError, IndexError) as exc:
        raise RuntimeError(f"Gemini returned no text: {json.dumps(payload)[:300]}") from exc
    text = text.strip()
    if text.startswith("```"):
        text = text.strip("`")
        text = text[text.index("{"):text.rindex("}") + 1]
    data = json.loads(text)
    title = str(data.get("title") or "").strip()
    description = str(data.get("description") or "").strip()
    if not title or not description:
        raise RuntimeError(f"Gemini JSON missing fields: {text[:200]}")
    return title, description


RETRYABLE_HTTP = {429, 500, 502, 503}


def _gemini_call(url, body, api_key, *, attempts=6):
    """POST with backoff: the free tier throws 503 ("model overloaded")
    routinely, and 429 when pacing slips. Non-retryable errors surface
    Gemini's own message instead of a bare HTTPError."""
    delay = 3.0
    for attempt in range(1, attempts + 1):
        req = urllib.request.Request(
            url, data=body,
            headers={"Content-Type": "application/json", "x-goog-api-key": api_key})
        try:
            with urllib.request.urlopen(req, timeout=120) as resp:
                return json.loads(resp.read())
        except urllib.error.HTTPError as exc:
            detail = ""
            try:
                detail = json.loads(exc.read()).get("error", {}).get("message", "")
            except Exception:
                pass
            if exc.code in RETRYABLE_HTTP and attempt < attempts:
                wait = float(exc.headers.get("Retry-After") or delay)
                print(f"    Gemini {exc.code} ({detail or 'transient'}) — "
                      f"retry {attempt}/{attempts - 1} in {wait:.0f}s")
                time.sleep(wait)
                delay = min(delay * 2, 60)
                continue
            raise RuntimeError(f"Gemini HTTP {exc.code}: {detail or exc.reason}") from exc
    raise RuntimeError("unreachable")


def describe_image(image_bytes, entry, report, *, model, api_key):
    payload = _gemini_call(
        GEMINI_URL.format(model=model),
        json.dumps(build_request(image_bytes, entry, report)).encode(),
        api_key)
    return parse_response(payload)


def _storage(payload):
    client = boto3.client("lambda", region_name="us-east-1")
    resp = client.invoke(FunctionName=STORAGE_FUNCTION,
                         Payload=json.dumps(payload).encode())
    body = json.loads(resp["Payload"].read())
    if int(body.get("statusCode", 500)) != 200:
        raise RuntimeError(f"storage {payload.get('path')}: {body.get('body')}")
    return json.loads(body["body"])


def fetch_public(url):
    with urllib.request.urlopen(url, timeout=60) as resp:
        return resp.read()


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Gemini titles + descriptions into a book's entry overrides.")
    parser.add_argument("--book", required=True, help="book id (e.g. book2)")
    parser.add_argument("--model", default=DEFAULT_MODEL)
    parser.add_argument("--apply", action="store_true", help="save; default prints only")
    parser.add_argument("--overwrite", action="store_true",
                        help="regenerate entries that already have overrides")
    parser.add_argument("--limit", type=int, default=0)
    args = parser.parse_args(argv)

    api_key = os.environ.get("GEMINI_API_KEY", "").strip()
    if not api_key:
        sys.exit("GEMINI_API_KEY is not set (free key: aistudio.google.com/apikey)")

    fetched = _storage({"path": "/fetch-book", "body": json.dumps({"id": args.book})})
    doc = fetched["book"]
    entries = doc.get("entries") or []
    if not entries:
        sys.exit(f"book {args.book} has no entries")

    changed = 0
    for idx, entry in enumerate(entries, start=1):
        if args.limit and changed >= args.limit:
            break
        has_prose = bool(str(entry.get("title_override") or "").strip()
                         or str(entry.get("body_override") or "").strip())
        if has_prose and not args.overwrite:
            print(f"[{idx}/{len(entries)}] {entry.get('artifact_id')}: has overrides, skipping")
            continue

        prefix = f"polypaint/books/{args.book}/assets/{entry.get('entry_id')}"
        report = {}
        try:
            report = json.loads(fetch_public(f"{BUCKET_BASE}{prefix}.provenance.json")).get("report") or {}
        except Exception:
            pass  # provenance is grounding, not a requirement
        preview_url = f"{BUCKET_BASE}renders/{entry.get('job_id')}/color/{entry.get('artifact_id')}/preview.jpg"
        try:
            image = fetch_public(preview_url)
        except Exception:
            # pre-migration artifact: fall back to the png
            image = fetch_public(preview_url.replace("preview.jpg", "preview.png"))

        title, description = describe_image(image, entry, report,
                                            model=args.model, api_key=api_key)
        print(f"[{idx}/{len(entries)}] {entry.get('artifact_id')}")
        print(f"    title: {title}")
        print(f"    body:  {description}")
        entry["title_override"] = title
        entry["body_override"] = description
        changed += 1
        time.sleep(FREE_TIER_PACING_S)

    if not changed:
        print("nothing to do")
        return 0
    if not args.apply:
        print(f"\nDRY-RUN: {changed} entries described, nothing saved. "
              f"Re-run with --apply to write them, then Compile the book.")
        return 0
    saved = _storage({"path": "/save-book", "body": json.dumps({"book": doc})})
    print(f"\nSaved {changed} descriptions to \"{saved['book'].get('name')}\" "
          f"(saved_at {saved['book'].get('saved_at')}). Compile to publish.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
