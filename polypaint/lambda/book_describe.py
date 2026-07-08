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
JOBS_TABLE = os.environ.get("JOBS_TABLE", "polypaint-jobs")
STORAGE_FUNCTION = os.environ.get("STORAGE_FUNCTION", "polypaint-storage")
GEMINI_URL = "https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent"
DEFAULT_MODEL = "gemini-2.5-flash"
# live 429 on a 21-spread book: gemini-2.5-flash free tier = 20 req/min and
# the banned-word rewrite doubles calls, so pacing must sit at the CALL
# layer (every request, rewrites included), not per entry
FREE_TIER_PACING_S = 6.5
VISION_CONFIG_KEYS = ("__config__", "vision_model")

s3 = boto3.client("s3")
_pacing = {"last": 0.0}


def _pace(min_interval):
    wait = _pacing["last"] + min_interval - time.monotonic()
    if wait > 0:
        time.sleep(wait)
    _pacing["last"] = time.monotonic()


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


def build_request(image_bytes, entry, report, *, angle="", used_titles=(), extra="", prebuilt_text=None):
    text = prebuilt_text if prebuilt_text is not None else _prompt_text(
        entry, report, angle=angle, used_titles=used_titles, extra=extra)
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


def _gemini_call(url, body, api_key, *, attempts=8, headers=None, pacing=FREE_TIER_PACING_S):
    """POST with backoff: free tiers throw 503 ("model overloaded")
    routinely and 429 when a rate window fills. 429s wait out the window
    (Retry-After honored, up to ~90s per attempt) instead of dying —
    a quota message like "retry in 52s" must pause the run, not kill it."""
    delay = 3.0
    for attempt in range(1, attempts + 1):
        _pace(pacing)
        req = urllib.request.Request(
            url, data=body,
            headers=headers or {"Content-Type": "application/json", "x-goog-api-key": api_key})
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
                wait = min(float(exc.headers.get("Retry-After") or delay), 90.0)
                if exc.code == 429:
                    wait = max(wait, 15.0)
                print(f"    vision API {exc.code} ({detail[:120] or 'transient'}) — "
                      f"retry {attempt}/{attempts - 1} in {wait:.0f}s")
                time.sleep(wait)
                delay = min(delay * 2, 60)
                continue
            raise RuntimeError(f"vision API HTTP {exc.code}: {detail or exc.reason}") from exc
    raise RuntimeError("unreachable")


def _prompt_text(entry, report, *, angle="", used_titles=(), extra=""):
    text = PROMPT.format(
        provenance=provenance_lines(entry, report),
        angle=angle or ANGLES[0],
        used_titles=", ".join(used_titles) or "(none yet)",
        banned=", ".join(BANNED_WORDS),
    )
    return text + (f"\n{extra}" if extra else "")


def _extract_text(payload, path_desc, *chain):
    node = payload
    try:
        for step in chain:
            node = node[step]
        return str(node)
    except (KeyError, IndexError, TypeError) as exc:
        raise RuntimeError(f"{path_desc}: no text in {json.dumps(payload)[:300]}") from exc


def _vision_call(model, api_key, image_bytes, text):
    """One image + one prompt -> raw model text. Provider inferred from the
    model id: gemini-* / claude-* / gpt-* & o* (VisionModel config)."""
    b64 = base64.b64encode(image_bytes).decode()
    if model.startswith("claude"):
        body = {
            "model": model, "max_tokens": 1024,
            "messages": [{"role": "user", "content": [
                {"type": "image", "source": {"type": "base64",
                                             "media_type": "image/jpeg", "data": b64}},
                {"type": "text", "text": text},
            ]}],
        }
        payload = _gemini_call(
            "https://api.anthropic.com/v1/messages",
            json.dumps(body).encode(), api_key,
            headers={"Content-Type": "application/json", "x-api-key": api_key,
                     "anthropic-version": "2023-06-01"},
            pacing=1.5)
        if "error" in payload:
            raise RuntimeError(f"Anthropic: {payload['error'].get('message', payload['error'])}")
        return _extract_text(payload, "Anthropic", "content", 0, "text")
    if model.startswith(("gpt", "o")):
        body = {
            "model": model,
            "messages": [{"role": "user", "content": [
                {"type": "image_url",
                 "image_url": {"url": f"data:image/jpeg;base64,{b64}"}},
                {"type": "text", "text": text},
            ]}],
            "response_format": {"type": "json_object"},
            "max_tokens": 1024,
        }
        payload = _gemini_call(
            "https://api.openai.com/v1/chat/completions",
            json.dumps(body).encode(), api_key,
            headers={"Content-Type": "application/json",
                     "Authorization": f"Bearer {api_key}"},
            pacing=1.5)
        if "error" in payload:
            raise RuntimeError(f"OpenAI: {payload['error'].get('message', payload['error'])}")
        return _extract_text(payload, "OpenAI", "choices", 0, "message", "content")
    # default: Gemini
    req = build_request(image_bytes, None, None, prebuilt_text=text)
    payload = _gemini_call(GEMINI_URL.format(model=model),
                           json.dumps(req).encode(), api_key)
    if "error" in payload:
        raise RuntimeError(f"Gemini: {payload['error'].get('message', payload['error'])}")
    return _extract_text(payload, "Gemini", "candidates", 0, "content", "parts", 0, "text")


def _parse_prose(text):
    """Extract {"title","description"} from a model reply, leniently.

    Models sometimes wrap the object in ``` fences, preface it with
    prose, or append trailing commentary / a duplicate object — strict
    json.loads then dies with "Extra data". raw_decode reads the FIRST
    complete JSON value and ignores whatever follows; scanning forward
    over '{' candidates also skips leading junk (fences included)."""
    raw = str(text or "")
    decoder = json.JSONDecoder()
    idx = raw.find("{")
    data = None
    while idx >= 0:
        try:
            candidate, _ = decoder.raw_decode(raw, idx)
        except json.JSONDecodeError:
            idx = raw.find("{", idx + 1)
            continue
        if isinstance(candidate, dict):
            data = candidate
            break
        idx = raw.find("{", idx + 1)
    if data is None:
        raise RuntimeError(f"vision reply has no JSON object: {raw[:200]}")
    title = str(data.get("title") or "").strip()
    description = str(data.get("description") or "").strip()
    if not title or not description:
        raise RuntimeError(f"vision JSON missing fields: {raw[:200]}")
    return title, description


def describe_image(image_bytes, entry, report, *, model, api_key,
                   angle="", used_titles=()):
    text = _prompt_text(entry, report, angle=angle, used_titles=used_titles)
    title, description = _parse_prose(_vision_call(model, api_key, image_bytes, text))
    # enforce the ban: one rewrite pass naming the offending words
    offenders = find_banned(f"{title} {description}")
    if offenders:
        retry_text = _prompt_text(
            entry, report, angle=angle, used_titles=used_titles,
            extra=f"Your previous attempt used banned words: {', '.join(offenders)}. "
                  f"Rewrite completely without them or any synonym-sludge.")
        title, description = _parse_prose(_vision_call(model, api_key, image_bytes, retry_text))
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


def _load_vision_config():
    """Model + PER-PROVIDER keys (api_key_gemini/anthropic/openai, with the
    legacy single api_key as gemini fallback) so model switches reuse the
    right stored key."""
    try:
        ddb = boto3.client("dynamodb", region_name=os.environ.get("AWS_REGION", "us-east-1"))
        resp = ddb.get_item(
            TableName=JOBS_TABLE,
            Key={"job_id": {"S": VISION_CONFIG_KEYS[0]},
                 "task_id": {"S": VISION_CONFIG_KEYS[1]}})
        item = resp.get("Item") or {}
        cfg = {"model": (item.get("model") or {}).get("S", "")}
        for prov in ("gemini", "anthropic", "openai"):
            cfg[f"api_key_{prov}"] = (item.get(f"api_key_{prov}") or {}).get("S", "")
        if not cfg["api_key_gemini"]:
            cfg["api_key_gemini"] = (item.get("api_key") or {}).get("S", "")
        return cfg
    except Exception:
        return {}


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

    # resolution: explicit request model > VisionModel config (DynamoDB,
    # set in the app) > env GEMINI_API_KEY default. The config key wins for
    # its model; env key only covers gemini models.
    from shared import vision_provider
    config = _load_vision_config()
    if not str(params.get("model") or "").strip():
        model = str(config.get("model") or DEFAULT_MODEL)
    provider = vision_provider(model)
    api_key = str(config.get(f"api_key_{provider}") or "").strip()
    if not api_key and provider == "gemini":
        api_key = os.environ.get("GEMINI_API_KEY", "").strip()
    if not api_key:
        raise RuntimeError(
            f"no API key for {model}: set the VisionModel config in the Book tab "
            f"(model + key), or export GEMINI_API_KEY and redeploy for gemini models")

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
        image = _entry_preview_bytes(entry)
        report = _entry_report(book_id, entry)
        title, description = describe_image(
            image, entry, report, model=model, api_key=api_key,
            angle=ANGLES[(idx - 1) % len(ANGLES)], used_titles=used_titles)
        entry["title_override"] = title
        entry["body_override"] = description
        used_titles.append(title)
        described += 1
        # save after EVERY entry: a mid-run quota death keeps the prose so
        # far, and skip-existing makes the rerun resume where it stopped.
        # Our in-memory doc stays the source of truth for the whole run —
        # adopting the server's returned copy would orphan the loop's entry
        # references and drop later entries' prose from subsequent saves.
        _storage({"path": "/save-book", "body": json.dumps({"book": doc})})
        _phase(job_id, task_id, "processing", "describe",
               f"Described {idx}/{len(entries)}: {title}")

    if described:
        _phase(job_id, task_id, "processing", "save", "Save book")
    _phase(job_id, task_id, "done", "done", "Done",
           described=described, skipped=skipped)
    return ok_response({"book_id": book_id, "described": described, "skipped": skipped})
