"""Gemini describe engine for book entries (server-side home).

The Book tab's Describe button dispatches op=describe to the book_pdf
lambda, which runs this module: per entry, the 512px preview + provenance
rows go to Gemini and the returned title/description land in the entry's
title_override/body_override. Anti-sludge measures (banned words with an
enforcement rewrite, rotating angles, cross-entry title decorrelation)
live here so the CLI wrapper and the lambda share one source of truth.

Saves go through the storage lambda's /save-book (validation + saved_at),
never a raw S3 write. GEMINI_API_KEY arrives as a lambda env var injected
at deploy time from the deployer's shell — never committed.
"""
import base64
import json
import os
import re
import time
import urllib.error
import urllib.request

import boto3

from shared import BUCKET, ok_response, report_status

BOOKS_PREFIX = "polypaint/books/"
STORAGE_FUNCTION = os.environ.get("STORAGE_FUNCTION", "polypaint-storage")
GEMINI_URL = "https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent"
DEFAULT_MODEL = "gemini-2.5-flash"
FREE_TIER_PACING_S = 5.0   # ~10 RPM free tier: stay comfortably under

s3 = boto3.client("s3")


BANNED_WORDS = [
    "vibrant", "intricate", "luminous", "ethereal", "mesmerizing",
    "captivating", "profound", "otherworldly", "cosmic", "radiant",
    "enigmatic", "delicate", "stunning", "breathtaking", "swirling",
    "shimmering", "glowing", "majestic", "dynamic", "dance",
]

# each entry gets a different lens so the book doesn't read like one
# bored critic repeating themselves
ANGLES = [
    "material and surface — what would this be made of if it were physical",
    "motion — what just happened or is about to happen in this form",
    "light — where it comes from, what it hides",
    "scale ambiguity — microscopic or astronomical, commit to one",
    "botany or biology — what organism this echoes, without naming it kitschily",
    "geology or weather — strata, erosion, storms, currents",
    "architecture or machinery — structure, load, mechanism",
    "textile and craft — weave, fold, thread, dye",
]

PROMPT = """You write terse, confident catalogue notes for a fine-art book of
abstract works. Technical provenance of this piece:

{provenance}

Angle for this piece: {angle}.

Titles already used in this book — do NOT reuse their words or their
pattern: {used_titles}

Return JSON: {{"title": ..., "description": ...}}.
- title: 2-4 words. Concrete nouns beat adjectives. No "untitled".
- description: 2-3 sentences, 60 words max. Name colors precisely
  (petrol, rust, bone, verdigris — not "colorful"). Describe ONE dominant
  structure and ONE small detail worth finding. At most one adjective per
  noun. Never use these words or their variants: {banned}.
  Never write "sense of", "draws the eye", "the viewer". No mathematics,
  no rendering talk.
"""


def find_banned(text):
    low = text.lower()
    return sorted({w for w in BANNED_WORDS if w in low})


def provenance_lines(entry, report):
    rows = (report or {}).get("summary_rows") or []
    lines = [f"- {k}: {v}" for k, v in rows]
    return "\n".join(lines) or f"- artifact: {entry.get('artifact_id', '')}"


def build_request(image_bytes, entry, report, *, angle="", used_titles=(), extra=""):
    text = PROMPT.format(
        provenance=provenance_lines(entry, report),
        angle=angle or ANGLES[0],
        used_titles=", ".join(used_titles) or "(none yet)",
        banned=", ".join(BANNED_WORDS),
    ) + (f"\n{extra}" if extra else "")
    return {
        "contents": [{
            "parts": [
                {"inline_data": {"mime_type": "image/jpeg",
                                 "data": base64.b64encode(image_bytes).decode()}},
                {"text": text},
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


def describe_image(image_bytes, entry, report, *, model, api_key,
                   angle="", used_titles=()):
    payload = _gemini_call(
        GEMINI_URL.format(model=model),
        json.dumps(build_request(image_bytes, entry, report,
                                 angle=angle, used_titles=used_titles)).encode(),
        api_key)
    title, description = parse_response(payload)
    # enforce the ban: one rewrite pass naming the offending words
    offenders = find_banned(f"{title} {description}")
    if offenders:
        payload = _gemini_call(
            GEMINI_URL.format(model=model),
            json.dumps(build_request(
                image_bytes, entry, report, angle=angle, used_titles=used_titles,
                extra=f"Your previous attempt used banned words: {', '.join(offenders)}. "
                      f"Rewrite completely without them or any synonym-sludge.")).encode(),
            api_key)
        title, description = parse_response(payload)
    return title, description


def _storage(payload):
    client = boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1"))
    resp = client.invoke(FunctionName=STORAGE_FUNCTION,
                         Payload=json.dumps(payload).encode())
    body = json.loads(resp["Payload"].read())
    if int(body.get("statusCode", 500)) != 200:
        raise RuntimeError(f"storage {payload.get('path')}: {body.get('body')}")
    return json.loads(body["body"])


def _phase(job_id, task_id, status, phase, phase_label, **extra):
    report_status(job_id, task_id, status,
                  result_data={"phase": phase, "phase_label": phase_label,
                               "family": "book", "op": "describe", **extra})


def _safe_id(value, label):
    if not re.fullmatch(r"[A-Za-z0-9._#-]{1,80}", str(value or "")):
        raise ValueError(f"book describe {label} has an unsafe value: {value!r}")
    return str(value)


def _entry_preview_bytes(entry):
    prefix = f"renders/{entry.get('job_id')}/color/{entry.get('artifact_id')}/"
    for name in ("preview.jpg", "preview.png"):
        try:
            return s3.get_object(Bucket=BUCKET, Key=prefix + name)["Body"].read()
        except Exception:
            continue
    raise RuntimeError(f"no preview for {entry.get('artifact_id')}")


def _entry_report(book_id, entry):
    key = f"{BOOKS_PREFIX}{book_id}/assets/{entry.get('entry_id')}.provenance.json"
    try:
        snap = json.loads(s3.get_object(Bucket=BUCKET, Key=key)["Body"].read())
        return snap.get("report") or {}
    except Exception:
        return {}


def handle_describe(params):
    job_id = params.get("job_id", "")
    task_id = params.get("task_id", "")
    book_id = _safe_id(params.get("book_id"), "book_id")
    expected_saved_at = str(params.get("expected_saved_at") or "")
    overwrite = bool(params.get("overwrite"))
    only_ids = {str(x) for x in (params.get("entry_ids") or []) if str(x)}
    model = str(params.get("model") or DEFAULT_MODEL)

    api_key = os.environ.get("GEMINI_API_KEY", "").strip()
    if not api_key:
        raise RuntimeError("GEMINI_API_KEY is not configured on the book lambda — "
                           "export it in your shell and redeploy")

    _phase(job_id, task_id, "started", "load_book", "Load book")
    obj = s3.get_object(Bucket=BUCKET, Key=f"{BOOKS_PREFIX}{book_id}.json")
    doc = json.loads(obj["Body"].read())
    if expected_saved_at and str(doc.get("saved_at") or "") != expected_saved_at:
        raise RuntimeError(f"book {book_id} was saved mid-describe; retry")
    entries = doc.get("entries") or []
    if not entries:
        raise RuntimeError(f"book {book_id} has no entries")

    used_titles = [str(e.get("title_override") or "").strip()
                   for e in entries if str(e.get("title_override") or "").strip()]
    described = skipped = 0
    for idx, entry in enumerate(entries, start=1):
        if only_ids and str(entry.get("entry_id") or "") not in only_ids:
            continue
        has_prose = bool(str(entry.get("title_override") or "").strip()
                         or str(entry.get("body_override") or "").strip())
        if has_prose and not overwrite:
            skipped += 1
            continue
        if described:
            time.sleep(FREE_TIER_PACING_S)
        image = _entry_preview_bytes(entry)
        report = _entry_report(book_id, entry)
        title, description = describe_image(
            image, entry, report, model=model, api_key=api_key,
            angle=ANGLES[(idx - 1) % len(ANGLES)], used_titles=used_titles)
        entry["title_override"] = title
        entry["body_override"] = description
        used_titles.append(title)
        described += 1
        _phase(job_id, task_id, "processing", "describe",
               f"Described {idx}/{len(entries)}: {title}")

    if described:
        _phase(job_id, task_id, "processing", "save", "Save book")
        _storage({"path": "/save-book", "body": json.dumps({"book": doc})})
    _phase(job_id, task_id, "done", "done", "Done",
           described=described, skipped=skipped)
    return ok_response({"book_id": book_id, "described": described, "skipped": skipped})
