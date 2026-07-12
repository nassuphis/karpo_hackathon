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
            # constrained decoding: the mime type alone only NUDGES the
            # model toward JSON (early stops still produced unclosed
            # objects) — a responseSchema forces the decoder to emit a
            # complete {title, description} object
            "responseSchema": {
                "type": "OBJECT",
                "properties": {
                    "title": {"type": "STRING"},
                    "description": {"type": "STRING"},
                },
                "required": ["title", "description"],
                "propertyOrdering": ["title", "description"],
            },
            # explicit, generous: replies were arriving cut mid-JSON with no
            # blamable finishReason — remove default-cap ambiguity entirely
            "maxOutputTokens": 4096,
        },
    }


def parse_response(payload):
    """Returns (title, description) or raises with the API's own words."""
    if "error" in payload:
        raise RuntimeError(f"Gemini: {payload['error'].get('message', payload['error'])}")
    return _parse_prose(_gemini_text(payload))


RETRYABLE_HTTP = {429, 500, 502, 503}


def _gemini_call(url, body, api_key, *, attempts=8, headers=None, pacing=FREE_TIER_PACING_S,
                 timeout=120.0, max_retry_wait=90.0):
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
            with urllib.request.urlopen(req, timeout=timeout) as resp:
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
                wait = min(wait, max_retry_wait)   # interactive callers cap the stall
                print(f"    vision API {exc.code} ({detail[:120] or 'transient'}) — "
                      f"retry {attempt}/{attempts - 1} in {wait:.0f}s")
                time.sleep(wait)
                delay = min(delay * 2, 60)
                continue
            raise RuntimeError(f"vision API HTTP {exc.code}: {detail or exc.reason}") from exc
        except (TimeoutError, urllib.error.URLError) as exc:
            # Socket read timeouts ("The read operation timed out") and connect
            # failures were FATAL before — a slow provider response killed the
            # call instead of retrying like any other transient (CR: gallery
            # describe broke on exactly this). URLError wraps connect timeouts;
            # TimeoutError covers read timeouts (socket.timeout is its alias).
            if attempt < attempts:
                wait = min(delay, max_retry_wait)
                print(f"    vision API {type(exc).__name__} — "
                      f"retry {attempt}/{attempts - 1} in {wait:.0f}s")
                time.sleep(wait)
                delay = min(delay * 2, 60)
                continue
            raise RuntimeError(f"vision API timeout: {exc}") from exc
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


def _vision_call(model, api_key, image_bytes, text, *, interactive=False):
    """One image + one prompt -> raw model text. Provider inferred from the
    model id: gemini-* / claude-* / gpt-* & o* (VisionModel config).
    interactive=True (code-review-29 F4): the caller is a synchronous API route,
    so trade the batch engine's patience (8 attempts, 120s socket, 90s waits)
    for a bounded profile that fits the gateway window; batch runs unchanged."""
    quick = {"attempts": 2, "timeout": 12.0, "max_retry_wait": 4.0} if interactive else {}
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
            pacing=0.0 if interactive else 1.5, **quick)
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
            pacing=0.0 if interactive else 1.5, **quick)
        if "error" in payload:
            raise RuntimeError(f"OpenAI: {payload['error'].get('message', payload['error'])}")
        return _extract_text(payload, "OpenAI", "choices", 0, "message", "content")
    # default: Gemini
    req = build_request(image_bytes, None, None, prebuilt_text=text)
    payload = _gemini_call(GEMINI_URL.format(model=model),
                           json.dumps(req).encode(), api_key,
                           pacing=0.0 if interactive else FREE_TIER_PACING_S, **quick)
    if "error" in payload:
        raise RuntimeError(f"Gemini: {payload['error'].get('message', payload['error'])}")
    return _gemini_text(payload)


def _gemini_text(payload):
    """Join ALL text parts of the first candidate. Gemini splits long
    replies across multiple parts (thinking models especially) — taking
    parts[0] alone truncates the JSON mid-string. Thought parts
    (thought: true) are reasoning summaries, not the answer: skipped."""
    cand = (payload.get("candidates") or [{}])[0]
    parts = ((cand.get("content") or {}).get("parts")) or []
    text = "".join(str(p.get("text") or "") for p in parts
                   if isinstance(p, dict) and not p.get("thought"))
    if not text.strip():
        raise RuntimeError(f"Gemini returned no text: {json.dumps(payload)[:300]}")
    fr = str(cand.get("finishReason") or "")
    if fr not in ("", "STOP") and not text.rstrip().endswith("}"):
        # MAX_TOKENS, RECITATION, SAFETY, ... all hard-stop mid-string —
        # name the real reason instead of a generic parse error
        raise RuntimeError(
            f"Gemini stopped early (finishReason {fr}) mid-reply: {text[-160:]}")
    return text


def _parse_prose(text):
    """Extract {"title","description"} from a model reply, leniently.

    Models sometimes wrap the object in ``` fences, preface it with
    prose, or append trailing commentary / a duplicate object — strict
    json.loads then dies with "Extra data". raw_decode reads the FIRST
    complete JSON value and ignores whatever follows; scanning forward
    over '{' candidates also skips leading junk (fences included)."""
    raw = str(text or "")
    # strict=False: literal newlines/tabs inside string values parse
    # instead of dying on "Invalid control character"
    decoder = json.JSONDecoder(strict=False)
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
        # head AND tail: the head always looks fine on truncation — the
        # tail is where a cut-off vs a malformed escape shows itself
        raise RuntimeError(
            f"no complete JSON object in vision reply "
            f"(len={len(raw)}, tail=…{raw[-120:]!r}): {raw[:160]}")
    title = str(data.get("title") or "").strip()
    description = str(data.get("description") or "").strip()
    if not title or not description:
        raise RuntimeError(f"vision JSON missing fields: {raw[:200]}")
    return title, description


def describe_image(image_bytes, entry, report, *, model, api_key,
                   angle="", used_titles=()):
    text = _prompt_text(entry, report, angle=angle, used_titles=used_titles)
    try:
        title, description = _parse_prose(_vision_call(model, api_key, image_bytes, text))
    except RuntimeError:
        # flaky stop (RECITATION/SAFETY/truncation) or malformed JSON:
        # one resample usually lands — temperature 0.9 gives a fresh draw
        title, description = _parse_prose(_vision_call(model, api_key, image_bytes, text))
    # enforce the ban: one rewrite pass naming the offending words
    offenders = find_banned(f"{title} {description}")
    if offenders:
        retry_text = _prompt_text(
            entry, report, angle=angle, used_titles=used_titles,
            extra=f"Your previous attempt used banned words: {', '.join(offenders)}. "
                  f"Rewrite completely without them or any synonym-sludge.")
        title, description = _parse_prose(_vision_call(model, api_key, image_bytes, retry_text))
        # re-check after the rewrite (code-review-26 F9): the ban is enforced,
        # not advisory — if the rewrite still offends, fail this entry so the
        # next Describe retries it rather than persisting banned prose
        still = find_banned(f"{title} {description}")
        if still:
            raise RuntimeError(
                f"banned words survived the rewrite: {', '.join(still)}")
    return title, description


class _SaveConflict(RuntimeError):
    """/save-book compare-and-swap rejected our write (409): the stored book
    moved under us (a concurrent human edit)."""


def _storage(payload):
    client = boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1"))
    resp = client.invoke(FunctionName=STORAGE_FUNCTION,
                         Payload=json.dumps(payload).encode())
    body = json.loads(resp["Payload"].read())
    status = int(body.get("statusCode", 500))
    if status != 200:
        inner = body.get("body")
        if status == 409:
            raise _SaveConflict(f"storage {payload.get('path')}: {inner}")
        raise RuntimeError(f"storage {payload.get('path')}: {inner}")
    return json.loads(body["body"])


def _fetch_book(book_id):
    resp = _storage({"path": "/fetch-book", "body": json.dumps({"id": book_id})})
    return resp.get("book") or {}, str(resp.get("revision") or "")


def _find_entry(doc, entry_id):
    for e in doc.get("entries") or []:
        if str(e.get("entry_id") or "") == entry_id:
            return e
    return None


def _save_book_cas(book_id, doc, revision, run_prose, run_base, attempts=4):
    """Save the book with a REAL compare-and-swap: /save-book passes the opaque
    revision (S3 ETag) it last saw, and S3 atomically rejects the write if the
    object moved (CR28 F5 — no timestamp/read-then-write pseudo-CAS). On
    conflict, refetch and re-apply THIS run's generated prose onto the fresh
    doc, but ONLY where the refetched entry still equals the BASE we captured
    before generating (or already equals our prose). If a human has since
    edited that entry, it no longer equals the base, so we keep their edit and
    drop ours (CR28 F11). Returns (authoritative_doc, new_revision)."""
    for _ in range(attempts):
        body = {"book": doc, "expected_revision": revision}
        try:
            resp = _storage({"path": "/save-book", "body": json.dumps(body)})
            return doc, str(resp.get("revision") or "")
        except _SaveConflict:
            doc, revision = _fetch_book(book_id)
            for eid, (title, desc) in run_prose.items():
                e = _find_entry(doc, eid)
                if e is None:
                    continue
                cur = (str(e.get("title_override") or ""), str(e.get("body_override") or ""))
                base = run_base.get(eid, ("", ""))
                if cur == base or cur == (title, desc):
                    e["title_override"], e["body_override"] = title, desc
                # else: a human edited this entry since we generated it —
                # preserve their edit, do not stamp our prose back over it
    raise RuntimeError(
        f"book {book_id} kept changing during describe (CAS gave up after {attempts} tries)")


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


VISION_MAX_PX = 768   # Describe only needs a thumbnail-level visual


def _downscale_for_vision(data, max_px=VISION_MAX_PX):
    """Shrink an image so its long edge is <= max_px before it goes to the
    Vision API. The image_key fallback can be a full-size render; sending it
    raw inflates latency, cost, and failure rate (code-review-26 F4). Best
    effort — if Pillow is unavailable or the decode fails, return as-is."""
    try:
        from PIL import Image
        import io as _io
        with Image.open(_io.BytesIO(data)) as im:
            if max(im.size) <= max_px:
                return data
            im = im.convert("RGB")
            im.thumbnail((max_px, max_px), Image.LANCZOS)
            buf = _io.BytesIO()
            im.save(buf, format="JPEG", quality=85)
            return buf.getvalue()
    except Exception:
        return data


def _entry_preview_bytes(entry):
    prefix = f"renders/{entry.get('job_id')}/color/{entry.get('artifact_id')}/"
    previews = [prefix + "preview.jpg", prefix + "preview.png"]
    for key in previews:
        try:
            return s3.get_object(Bucket=BUCKET, Key=key)["Body"].read()
        except Exception:
            continue
    # fall back to the entry's stored image_key (legacy/root-shaped artifacts
    # lack the immutable color preview path) but DOWNSCALE it — that key is
    # the full render image, not a 512 thumbnail
    image_key = str(entry.get("image_key") or "").strip()
    if image_key:
        try:
            return _downscale_for_vision(
                s3.get_object(Bucket=BUCKET, Key=image_key)["Body"].read())
        except Exception:
            pass
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
    revision = str(obj.get("ETag") or "").strip('"')   # opaque CAS token (F5)
    if expected_saved_at and str(doc.get("saved_at") or "") != expected_saved_at:
        raise RuntimeError(f"book {book_id} was saved mid-describe; retry")
    entries = doc.get("entries") or []
    if not entries:
        raise RuntimeError(f"book {book_id} has no entries")

    # Iterate a STABLE list of entry ids captured at load, but re-locate each
    # entry in the CURRENT doc every step: a concurrent edit can trigger a CAS
    # refetch that swaps `doc` for the server's fresh copy mid-run, and the
    # id lookup keeps us pointed at the right (possibly newly-merged) entry.
    work_ids = [str(e.get("entry_id") or "") for e in entries]
    total = len(work_ids)
    used_titles = [str(e.get("title_override") or "").strip()
                   for e in entries if str(e.get("title_override") or "").strip()]
    run_prose = {}   # entry_id -> (title, body): re-applied on CAS conflict
    run_base = {}    # entry_id -> (title, body) BEFORE we generated (F11)
    described = skipped = 0
    failures = []
    for idx, entry_id in enumerate(work_ids, start=1):
        if only_ids and entry_id not in only_ids:
            continue
        entry = _find_entry(doc, entry_id)
        if entry is None:
            continue  # removed by a concurrent edit
        has_prose = bool(str(entry.get("title_override") or "").strip()
                         or str(entry.get("body_override") or "").strip())
        if has_prose and not overwrite:
            skipped += 1
            continue
        # one flaky entry must not abort the rest of the run: record the
        # failure and keep going — the entry stays blank, so the next
        # Describe (skip-existing) retries exactly the failures.
        try:
            image = _entry_preview_bytes(entry)
            report = _entry_report(book_id, entry)
            title, description = describe_image(
                image, entry, report, model=model, api_key=api_key,
                angle=ANGLES[(idx - 1) % len(ANGLES)], used_titles=used_titles)
        except Exception as exc:
            failures.append({"entry_id": entry_id, "error": str(exc)[:300]})
            _phase(job_id, task_id, "processing", "describe",
                   f"Failed {idx}/{total}: {str(exc)[:120]}")
            continue
        # capture the entry's prior state BEFORE we overwrite it, so a save
        # failure restores exactly that (in overwrite mode it may hold old
        # prose, which blanking would destroy — CR28 F11)
        base_title = str(entry.get("title_override") or "")
        base_body = str(entry.get("body_override") or "")
        entry["title_override"] = title
        entry["body_override"] = description
        used_titles.append(title)
        run_prose[entry_id] = (title, description)
        run_base[entry_id] = (base_title, base_body)
        described += 1
        # save after EVERY entry: a mid-run quota death keeps the prose so
        # far, and skip-existing makes the rerun resume where it stopped.
        # Atomic revision CAS: a concurrent human edit no longer gets clobbered
        # by our stale full-document write — on conflict we refetch, re-apply
        # this run's prose (base-aware), and adopt the merged doc.
        try:
            doc, revision = _save_book_cas(book_id, doc, revision, run_prose, run_base)
        except Exception as exc:
            # roll the failed entry back to its captured base in BOTH the run
            # record and the in-memory doc, so a later successful save can't
            # persist prose we reported as failed AND we don't destroy any
            # pre-existing prose (CR28 F11)
            run_prose.pop(entry_id, None)
            run_base.pop(entry_id, None)
            reverted = _find_entry(doc, entry_id)
            if reverted is not None:
                reverted["title_override"] = base_title
                reverted["body_override"] = base_body
            if used_titles and used_titles[-1] == title:
                used_titles.pop()
            failures.append({"entry_id": entry_id, "error": f"save: {str(exc)[:280]}"})
            _phase(job_id, task_id, "processing", "describe",
                   f"Save failed {idx}/{total}: {str(exc)[:110]}")
            described -= 1
            continue
        _phase(job_id, task_id, "processing", "describe",
               f"Described {idx}/{total}: {title}")

    if failures and not described and not skipped:
        # nothing worked at all — systemic (bad key, dead model): hard fail
        raise RuntimeError(
            f"describe failed on all {len(failures)} attempted entries; "
            f"first: {failures[0]['error']}")
    if described:
        _phase(job_id, task_id, "processing", "save", "Save book")
    first_error = failures[0]["error"] if failures else ""
    _phase(job_id, task_id, "done", "done", "Done",
           described=described, skipped=skipped,
           failed=len(failures), first_error=first_error)
    return ok_response({"book_id": book_id, "described": described,
                        "skipped": skipped, "failed": len(failures),
                        "first_error": first_error})
