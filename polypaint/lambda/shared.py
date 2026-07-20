"""
Shared utilities for all polypaint Lambda handlers.
"""
import json
import math
import os
import re
import struct
import threading
import time
import errno
import zlib

import boto3

BUCKET = os.environ.get("BUCKET", "polypaint")
JOBS_TABLE = os.environ.get("JOBS_TABLE", "polypaint-jobs")
PRESIGN_EXPIRY = 3600  # 1 hour
REF_SIZE = 4096  # reference image dimension for scale computation
BILEVEL_SPARSE_PIPELINE = "logical_sections_sparse_fragments_v1"

_ddb = None


def _get_ddb():
    """Lazy-init DynamoDB client (avoids cold-start cost for Lambdas that don't use it)."""
    global _ddb
    if _ddb is None:
        _ddb = boto3.client("dynamodb")
    return _ddb


# CR33 telemetry (build attribution): the deployer stamps these env vars at
# deploy time; every status row and task result then self-describes which
# build produced it. Empty when running outside a deployed environment.
PP_GIT_SHA = os.environ.get("PP_GIT_SHA", "")
PP_BUILD_ID = os.environ.get("PP_BUILD_ID", "")


def build_identity():
    """Build attribution fields for task results (empty dict when unset,
    so local/test payloads stay unchanged)."""
    out = {}
    if PP_GIT_SHA:
        out["git_sha"] = PP_GIT_SHA
    if PP_BUILD_ID:
        out["build_id"] = PP_BUILD_ID
    return out


def resolve_bound_execution_arn(ddb_client, jobs_table, *, job_id, task_id,
                                client_arn, state_machine_arn):
    """CR35-F11: a stop request must not trust the client's execution
    ARN. The authoritative binding is the status row the workflow wrote
    for (job_id, task_id): its stored execution_arn (written by the ASL
    from $$.Execution.Id) is the ONLY execution a stop may target, it
    must belong to this orchestrator's state machine, and the client's
    ARN — kept as a staleness check — must match it exactly."""
    job = str(job_id or "").strip()
    task = str(task_id or "").strip()
    arn = str(client_arn or "").strip()
    if not job or not task or not arn:
        raise RuntimeError("stop requires job_id, task_id, and execution_arn")
    resp = ddb_client.get_item(
        TableName=jobs_table,
        Key={"job_id": {"S": job}, "task_id": {"S": task}},
        ConsistentRead=True,
    )
    item = resp.get("Item")
    if not item:
        raise RuntimeError(f"stop refused: no status row for {job}/{task}")
    stored = ""
    raw_result = item.get("result_data", {}).get("S")
    if raw_result:
        try:
            parsed = json.loads(raw_result)
            if isinstance(parsed, dict):
                stored = str(parsed.get("execution_arn") or "").strip()
        except (TypeError, ValueError):
            stored = ""
    if not stored:
        raise RuntimeError(
            f"stop refused: status row for {job}/{task} carries no execution ARN")
    expected_prefix = str(state_machine_arn or "").replace(
        ":stateMachine:", ":execution:") + ":"
    if not state_machine_arn or not stored.startswith(expected_prefix):
        raise RuntimeError(
            "stop refused: stored execution does not belong to this workflow")
    if arn != stored:
        raise RuntimeError(
            "stop refused: the supplied execution ARN does not match the "
            "run's recorded execution (stale client state?)")
    return stored


def read_task_status(job_id, task_id):
    """Return the current task_status string for (job_id, task_id), or
    None when no row exists. Used to make a replayed worker idempotent
    (a duplicate must not regress a 'done' row)."""
    resp = _get_ddb().get_item(
        TableName=JOBS_TABLE,
        Key={"job_id": {"S": str(job_id)}, "task_id": {"S": str(task_id)}},
        ConsistentRead=True,
    )
    item = resp.get("Item")
    if not item:
        return None
    return item.get("task_status", {}).get("S")


# The lease must comfortably exceed the longest single native operation a
# worker can run between renewals (one _run_binary call caps at 300s), or
# a live worker mid-render would lose its lease to a redispatch. 420s
# leaves margin above the 300s op while still bounding crash recovery.
CLAIM_LEASE_SECONDS = 420


def _is_conditional_failure(exc):
    code = ""
    resp = getattr(exc, "response", None)
    if isinstance(resp, dict):
        code = resp.get("Error", {}).get("Code", "")
    return (code == "ConditionalCheckFailedException"
            or "ConditionalCheckFailed" in type(exc).__name__)


def claim_task(job_id, task_id, *, owner, lease_seconds=CLAIM_LEASE_SECONDS,
               from_statuses=("started", "accepted")):
    """Atomically claim a task with a LEASE (CR35 round-6 finding 1). The
    conditional UpdateItem claims when the task is claimable:
      - status is one of from_statuses (fresh, never started), OR
      - status is 'running' but the lease has EXPIRED (the previous owner
        died without renewing — stale takeover), OR
      - the caller already owns it (idempotent re-entry).
    On success the row records this owner and a lease expiry now+lease. A
    live owner keeps the lease fresh via renew_claim(); a crashed owner
    lets it expire so a redispatched worker can reclaim after
    lease_seconds instead of being blocked forever. Returns True on
    claim, False when another owner holds a LIVE lease. Never raises on a
    lost claim."""
    now_ms = int(time.time() * 1000)
    lease_ms = now_ms + int(lease_seconds * 1000)
    placeholders = {f":s{i}": {"S": st} for i, st in enumerate(from_statuses)}
    cond = (
        "attribute_exists(task_status) AND ("
        "task_status IN (" + ",".join(placeholders) + ")"
        " OR (task_status = :running AND (attribute_not_exists(lease_expiry_ms)"
        " OR lease_expiry_ms < :now))"
        " OR claim_owner = :owner)"
    )
    try:
        _get_ddb().update_item(
            TableName=JOBS_TABLE,
            Key={"job_id": {"S": str(job_id)}, "task_id": {"S": str(task_id)}},
            UpdateExpression=("SET task_status = :running, claim_owner = :owner, "
                              "lease_expiry_ms = :lease, updated_at_ms = :now"),
            ConditionExpression=cond,
            ExpressionAttributeValues={
                ":running": {"S": "running"},
                ":owner": {"S": str(owner)},
                ":lease": {"N": str(lease_ms)},
                ":now": {"N": str(now_ms)},
                **placeholders,
            },
        )
        return True
    except Exception as exc:
        if _is_conditional_failure(exc):
            return False
        raise


def renew_claim(job_id, task_id, *, owner, lease_seconds=CLAIM_LEASE_SECONDS,
                result_data=None):
    """Extend this owner's lease and optionally update result_data in one
    conditional UpdateItem. Returns True while the caller still owns the
    running task; False when the lease was taken over (the caller must
    abort — a duplicate is now the owner). Progress writes go through
    this instead of report_status so a full-item PutItem can never clobber
    the owner/lease and let a duplicate steal a live task."""
    now_ms = int(time.time() * 1000)
    lease_ms = now_ms + int(lease_seconds * 1000)
    values = {
        ":owner": {"S": str(owner)},
        ":lease": {"N": str(lease_ms)},
        ":now": {"N": str(now_ms)},
        ":running": {"S": "running"},
    }
    expr = "SET lease_expiry_ms = :lease, updated_at_ms = :now, task_status = :running"
    if result_data is not None:
        values[":rd"] = {"S": json.dumps(result_data)}
        expr += ", result_data = :rd"
    try:
        _get_ddb().update_item(
            TableName=JOBS_TABLE,
            Key={"job_id": {"S": str(job_id)}, "task_id": {"S": str(task_id)}},
            UpdateExpression=expr,
            ConditionExpression="claim_owner = :owner",
            ExpressionAttributeValues=values,
        )
        return True
    except Exception as exc:
        if _is_conditional_failure(exc):
            return False
        raise


def finalize_task(job_id, task_id, *, owner, status, error_msg=None,
                  result_data=None):
    """OWNER-CONDITIONAL terminal write (CR35 round-7 findings 1/2). Set
    the task's terminal status (done/error) ONLY while this caller still
    owns the lease. Returns True when the terminal status was written
    (the caller is still the owner), False when the lease was taken over
    (a successor owns the task — the caller MUST exit without touching
    any shared state, so a stale owner cannot overwrite its successor's
    done row or fail a run the successor is completing). Clears the lease
    on a terminal write so the row is not reclaimable."""
    now_ms = int(time.time() * 1000)
    values = {
        ":owner": {"S": str(owner)},
        ":status": {"S": str(status)},
        ":now": {"N": str(now_ms)},
    }
    # round-8 finding 7: a terminal write clears BOTH the lease AND the
    # owner so the row is unreclaimable — the old code kept claim_owner,
    # and claim_task's `OR claim_owner = :owner` clause would then let the
    # same owner move a done/error row back to running.
    set_expr = "task_status = :status, updated_at_ms = :now"
    if error_msg:
        values[":em"] = {"S": str(error_msg)[:1000]}
        set_expr += ", error_msg = :em"
    if result_data is not None:
        values[":rd"] = {"S": json.dumps(result_data)}
        set_expr += ", result_data = :rd"
    expr = "SET " + set_expr + " REMOVE lease_expiry_ms, claim_owner"
    try:
        _get_ddb().update_item(
            TableName=JOBS_TABLE,
            Key={"job_id": {"S": str(job_id)}, "task_id": {"S": str(task_id)}},
            UpdateExpression=expr,
            ConditionExpression="claim_owner = :owner",
            ExpressionAttributeValues=values,
        )
        return True
    except Exception as exc:
        if _is_conditional_failure(exc):
            return False
        raise


# The heartbeat renews well inside the lease so a HEALTHY worker is never
# reclaimed mid-frame. A single frame chains coeffgen + solve (+ a frozen-
# viewport solve), each a native op capped at 300s — up to ~900s between
# tile writes, far past the 420s lease. Without a background renewal a live
# worker would lose its lease and could then overwrite a successor's keys.
HEARTBEAT_INTERVAL_SECONDS = 90


class LeaseHeartbeat:
    """Background lease renewal for a worker/stitch running long native
    ops between the points where it renews inline (round-8 finding 1).

    While started, a daemon thread renews this owner's lease every
    `interval_s`. If a renewal reveals the lease was TAKEN OVER (a
    successor now owns the task) it latches `lost` and stops; callers
    MUST check `lost` before every shared-state (S3) write and exit
    benignly, so a reclaimed-but-still-running worker never overwrites
    its successor's tiles or artifacts. A TRANSIENT DDB error does NOT
    latch lost (that would be finding-3 in reverse — treating uncertainty
    as loss); the heartbeat simply retries on the next tick, and the real
    lease still expires on its own clock if the errors persist."""

    def __init__(self, job_id, task_id, *, owner,
                 interval_s=HEARTBEAT_INTERVAL_SECONDS,
                 lease_seconds=CLAIM_LEASE_SECONDS):
        self.job_id = job_id
        self.task_id = task_id
        self.owner = owner
        self.interval_s = interval_s
        self.lease_seconds = lease_seconds
        self._stop = threading.Event()
        self._lost = threading.Event()
        self._thread = None

    @property
    def lost(self):
        return self._lost.is_set()

    def _loop(self):
        while not self._stop.wait(self.interval_s):
            try:
                still_owner = renew_claim(
                    self.job_id, self.task_id, owner=self.owner,
                    lease_seconds=self.lease_seconds)
            except Exception:
                continue          # transient: retry next tick, don't latch
            if not still_owner:
                self._lost.set()
                return

    def start(self):
        if self._thread is None:
            self._thread = threading.Thread(target=self._loop, daemon=True)
            self._thread.start()
        return self

    def stop(self):
        self._stop.set()
        if self._thread is not None:
            self._thread.join(timeout=2)
            self._thread = None


def report_status(job_id, task_id, status, error_msg=None, result_data=None):
    """Write task completion status to DynamoDB. TTL = 24h auto-cleanup.
    Optional result_data dict is stored as JSON string for later retrieval."""
    now_ms = int(time.time() * 1000)
    item = {
        "job_id": {"S": job_id},
        "task_id": {"S": task_id},
        "task_status": {"S": status},
        "updated_at_ms": {"N": str(now_ms)},
        "ttl": {"N": str(int(time.time()) + 86400)},
    }
    if PP_BUILD_ID:
        item["build_id"] = {"S": PP_BUILD_ID}
    if PP_GIT_SHA:
        item["git_sha"] = {"S": PP_GIT_SHA}
    if error_msg:
        item["error_msg"] = {"S": str(error_msg)[:1000]}
    if result_data:
        item["result_data"] = {"S": json.dumps(result_data)}
    _get_ddb().put_item(TableName=JOBS_TABLE, Item=item)


def contract_param(params, key, default, warnings=None, *, missing_values=(None, ""), warning_default=None):
    """Fetch a behavioral param and record a contract warning if it is missing.

    This is for workflow/API contract fields where a silent handler default can mask
    a missing Step Functions/UI payload field and change behavior.
    """
    missing = key not in params or params.get(key) in missing_values
    if missing:
        if warnings is not None:
            warnings.append({
                "kind": "missing_param_default",
                "param": key,
                "default": warning_default if warning_default is not None else default,
            })
        return default
    return params.get(key)


def attach_contract_warnings(result_data, warnings):
    if result_data is None or not warnings:
        return result_data
    result_data["contract_warnings"] = list(warnings)
    result_data["contract_warning_count"] = len(warnings)
    return result_data


def parse_body(event):
    """Parse request body from various invocation formats."""
    if isinstance(event.get("body"), str):
        return json.loads(event["body"])
    elif "body" in event and event["body"] is not None:
        return event["body"]
    return event


def ok_response(body):
    # CR34 post-mortem follow-up: EVERY task/API response self-describes the
    # build that produced it, injected here at the single emission point so
    # coverage cannot drift per handler (the F1 lesson). setdefault keeps
    # handlers that already merge build_identity() authoritative, and the
    # fields vanish entirely outside deployed environments (env unset).
    if isinstance(body, dict):
        for key, value in build_identity().items():
            body.setdefault(key, value)
    return {
        "statusCode": 200,
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps(body),
    }


def parse_boolish(value, default=True, *, strict=False, label="value"):
    if value in (None, ""):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    text = str(value).strip().lower()
    if text in ("1", "true", "yes", "on"):
        return True
    if text in ("0", "false", "no", "off"):
        return False
    if strict:
        raise RuntimeError(f"{label} must be boolean-like, got {value!r}")
    return default


def encode_png_gray(width, height, gray_buf):
    """Encode a grayscale image buffer as PNG bytes."""

    def _chunk(ctype, data):
        chunk = ctype + data
        crc = zlib.crc32(chunk) & 0xFFFFFFFF
        return struct.pack(">I", len(data)) + chunk + struct.pack(">I", crc)

    raw = bytearray()
    for y in range(height):
        raw.append(0)
        raw.extend(gray_buf[y * width:(y + 1) * width])

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 0, 0, 0, 0)
    idat = zlib.compress(bytes(raw), 6)

    out = b"\x89PNG\r\n\x1a\n"
    out += _chunk(b"IHDR", ihdr)
    out += _chunk(b"IDAT", idat)
    out += _chunk(b"IEND", b"")
    return out



def imgpipe_env():
    """Environment for imgpipe (needs libvips from /opt/lib)."""
    env = dict(os.environ)
    ld = env.get("LD_LIBRARY_PATH", "")
    if "/opt/lib" not in ld:
        env["LD_LIBRARY_PATH"] = "/opt/lib:" + ld
    return env


def compute_viewport_from_bin(bin_data, quantile=0.0, shim=0.05):
    """Compute viewport (center, scale) from binary root data.
    .bin format: raw f32 pairs [re, im, re, im, ...] with no header.
    Returns dict with center_re, center_im, scale (for REF_SIZE reference),
    n_roots, q_re, q_im.
    """
    n_floats = len(bin_data) // 4
    n_pairs = n_floats // 2
    if n_pairs == 0:
        return {"center_re": 0, "center_im": 0, "scale": 1.0, "n_roots": 0,
                "q_re": [0, 0], "q_im": [0, 0]}

    # Stream f32 pairs to avoid materializing all floats into a Python tuple
    usable = n_pairs * 8  # 2 floats × 4 bytes
    _isfinite = math.isfinite
    all_re = []
    all_im = []
    for re, im in struct.iter_unpack('<ff', bin_data[:usable]):
        if _isfinite(re) and _isfinite(im):
            all_re.append(re)
            all_im.append(im)

    n_roots = len(all_re)
    if n_roots == 0:
        return {"center_re": 0, "center_im": 0, "scale": 1.0, "n_roots": 0,
                "q_re": [0, 0], "q_im": [0, 0]}

    all_re.sort()
    all_im.sort()

    shim_mul = 1.0 + shim
    if n_roots > 10:
        lo = int(n_roots * quantile)
        hi = int(n_roots * (1.0 - quantile)) - 1
        lo = max(0, lo)
        hi = min(n_roots - 1, hi)
        if hi <= lo:
            lo, hi = 0, n_roots - 1
    else:
        lo, hi = 0, n_roots - 1

    q_min_re, q_max_re = all_re[lo], all_re[hi]
    q_min_im, q_max_im = all_im[lo], all_im[hi]

    center_re = (q_min_re + q_max_re) / 2.0
    center_im = (q_min_im + q_max_im) / 2.0

    range_re = (q_max_re - q_min_re) * shim_mul
    range_im = (q_max_im - q_min_im) * shim_mul

    if range_re > 0 and range_im > 0:
        scale = min(REF_SIZE / range_re, REF_SIZE / range_im)
    elif range_re > 0:
        scale = REF_SIZE / range_re
    elif range_im > 0:
        scale = REF_SIZE / range_im
    else:
        # Keep degenerate single-point previews bounded instead of exploding
        # to the full 4096-world-unit fallback span.
        scale = float(REF_SIZE)

    return {
        "center_re": center_re,
        "center_im": center_im,
        "scale": scale,
        "n_roots": n_roots,
        "q_re": [q_min_re, q_max_re],
        "q_im": [q_min_im, q_max_im],
    }


def format_bytes(n):
    try:
        n = int(n)
    except (TypeError, ValueError):
        return "?"
    units = ["B", "KB", "MB", "GB", "TB"]
    value = float(n)
    unit = units[0]
    for unit in units:
        if abs(value) < 1024.0 or unit == units[-1]:
            break
        value /= 1024.0
    if unit == "B":
        return f"{int(value)}{unit}"
    return f"{value:.1f}{unit}"


def tmp_space_stats(path="/tmp"):
    st = os.statvfs(path)
    total = int(st.f_blocks) * int(st.f_frsize)
    free = int(st.f_bavail) * int(st.f_frsize)
    return {"path": path, "total_bytes": total, "free_bytes": free}


# Browser-fetched artifact objects live at immutable, artifact/export-scoped
# keys (a new artifact always gets a new key), so clients may cache them
# forever. Mutable pointers and manifests must NOT use this.
CACHE_IMMUTABLE = "public, max-age=31536000, immutable"


# Book entries / prepare sources accept only render-scoped image keys. A book
# image_key becomes a raw LaTeX macro argument at compose time (\qrcode{URL})
# and an S3 GET at prepare time, so hostile values (braces, backslashes, path
# escapes, arbitrary prefixes) must never reach either. The charset blocks all
# TeX specials; the renders/ prefix + image extension pins it to render output.
_SAFE_RENDER_IMAGE_KEY = re.compile(r"renders/[A-Za-z0-9._/-]+\.(?:jpe?g|png)")
# derived-artifact workers (png/tiff export, deepzoom) legitimately consume
# .tif render objects too; the strict image-key check above stays jpg/png-only
# for book image_key + QR payloads.
_SAFE_RENDER_OBJECT_KEY = re.compile(r"renders/[A-Za-z0-9._/-]+\.(?:jpe?g|png|tiff?)")

# Canonical render family directories under renders/<job>/. 'palettes' holds
# per-job palette artifacts. A canonical artifact key is always
# renders/<job>/<family>/<artifact>/<leaf> — every artifact has a family dir.
RENDER_FAMILY_DIRS = ("color", "bilevel", "coeffs", "pdf", "palettes")


def is_safe_render_image_key(key):
    k = str(key or "")
    return bool(_SAFE_RENDER_IMAGE_KEY.fullmatch(k)) and ".." not in k


def is_safe_render_object_key(key):
    k = str(key or "")
    return bool(_SAFE_RENDER_OBJECT_KEY.fullmatch(k)) and ".." not in k


def assert_safe_render_image_key(key, label="image_key"):
    if not is_safe_render_image_key(key):
        raise ValueError(
            f"{label} must be a render image key "
            f"renders/.../*.jpg|jpeg|png (no braces, backslashes, or '..'): {key!r}")
    return str(key)


_SAFE_ID = re.compile(r"[A-Za-z0-9_-]{1,64}")


def assert_safe_id(value, label="id"):
    """Validate a caller id that feeds a Step Functions execution name, a DDB
    partition/sort key, or an S3 prefix. Rejects slash/whitespace/colon/
    control chars and over-length so a bad run_id can't produce InvalidName,
    a poisoned key, or a path-like string (code-review-27 F9)."""
    v = str(value or "")
    if not _SAFE_ID.fullmatch(v):
        raise ValueError(f"{label} must match [A-Za-z0-9_-]{{1,64}}: {value!r}")
    return v


def assert_render_source(key, job_id, artifact_id=None, label="source_key"):
    """Validate a caller-supplied derived-artifact source key BEFORE any S3
    head_object/get_object (code-review-27 F5). Requires a safe render key,
    and ties it to the declared identity: with an artifact_id, the key must
    name that artifact (job + /artifact/ segment); legacy_color or a missing
    artifact_id pins only the job scope so a worker can't be pointed at
    another job's bytes while writing provenance for this one."""
    if not is_safe_render_object_key(key):
        raise ValueError(
            f"{label} must be a render object key renders/.../*.jpg|jpeg|png|tif|tiff "
            f"(no braces, backslashes, or '..'): {key!r}")
    aid = str(artifact_id or "")
    if aid and aid != "legacy_color":
        assert_render_identity(key, job_id, aid, label)
    elif not str(key).startswith(f"renders/{job_id}/"):
        raise ValueError(f"{label} {key!r} is not under renders/{job_id}/ (job mismatch)")
    return str(key)


def parse_render_key(key):
    """Parse an S3 render key into structured identity components.

    A canonical render artifact key is renders/<job>/<family>/<artifact>/<leaf>.
    Returns a dict {job, family, artifact_id, leaf, variant, segments}. variant:
      'canonical'   renders/<job>/<family>/<artifact>/<leaf...>  (family known)
      'legacy_root' renders/<job>/<leaf>   (single legacy artifact-per-job file)
      'job_scoped'  renders/<job>/<other>/...  (job files, chunk dirs, etc.)
      'invalid'     not under renders/<job>/

    Identity checks must compare EXACT components, never a substring: a
    substring test accepts artifact id 'color'/'palettes' or the job id for an
    unrelated key (code-review-28 F12). Never raises."""
    k = str(key or "")
    parts = k.split("/")
    base = {"job": None, "family": None, "artifact_id": None, "leaf": None,
            "variant": "invalid", "segments": parts}
    if len(parts) < 3 or parts[0] != "renders" or not parts[1]:
        return base
    job = parts[1]
    leaf = parts[-1]
    if len(parts) >= 5 and parts[2] in RENDER_FAMILY_DIRS and parts[3] and leaf:
        return {"job": job, "family": parts[2], "artifact_id": parts[3],
                "leaf": leaf, "variant": "canonical", "segments": parts}
    if len(parts) == 3 and parts[2]:
        return {"job": job, "family": None, "artifact_id": None,
                "leaf": leaf, "variant": "legacy_root", "segments": parts}
    return {"job": job,
            "family": parts[2] if parts[2] in RENDER_FAMILY_DIRS else None,
            "artifact_id": None, "leaf": leaf, "variant": "job_scoped",
            "segments": parts}


def assert_render_identity(key, job_id, artifact_id, label="image_key"):
    """A render image key must belong to the same artifact its sibling fields
    name, so a book/PDF page can't pair image B with metadata A.

    Parses the key into renders/<job>/<family>/<artifact>/<leaf> and compares
    EXACT components (code-review-26 F3, tightened per code-review-28 F12): the
    old substring test (`/<artifact_id>/ in key`) accepted artifact id 'color'
    or 'palettes' or the literal job id for an unrelated key. Assumes the key
    already passed assert_safe_render_image_key."""
    k = str(key or "")
    jid = str(job_id or "")
    aid = str(artifact_id or "")
    parsed = parse_render_key(k)
    if not jid or parsed["job"] != jid:
        raise ValueError(f"{label} {k!r} is not under renders/{jid}/ (job_id mismatch)")
    if parsed["variant"] != "canonical":
        raise ValueError(
            f"{label} {k!r} is not a canonical renders/<job>/<family>/<artifact>/... key")
    if not aid or parsed["artifact_id"] != aid:
        raise ValueError(
            f"{label} {k!r} artifact segment {parsed['artifact_id']!r} "
            f"does not equal declared artifact_id {aid!r}")
    return k


def vision_provider(model):
    """Provider from a vision model id: gemini-* -> google, claude-* ->
    anthropic, gpt-*/o* -> openai. Keys are stored per provider so the
    user can switch models freely (VisionModel config)."""
    m = str(model or "")
    if m.startswith("claude"):
        return "anthropic"
    if m.startswith(("gpt", "o")):
        return "openai"
    return "gemini"


def png_dimensions_from_path(path):
    """Read PNG dimensions from a local file's IHDR (no imaging deps).

    Preview object metadata must describe the preview itself, never the
    full-size source (deepzoom-speed.md §2.5)."""
    with open(path, "rb") as fh:
        data = fh.read(33)
    if len(data) < 24 or data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        raise RuntimeError(f"invalid PNG: {path}")
    width = int.from_bytes(data[16:20], "big")
    height = int.from_bytes(data[20:24], "big")
    if width <= 0 or height <= 0:
        raise RuntimeError(f"invalid PNG dimensions: {width}x{height}")
    return width, height


def is_enospc(exc):
    return isinstance(exc, OSError) and getattr(exc, "errno", None) == errno.ENOSPC


# ── S3 error taxonomy (code-review-28 F13) ─────────────────────────────────
# Exactly one policy decides whether an S3 exception means "the object is
# genuinely absent" versus a transient or configuration failure that must NOT
# be silently relabeled as missing. A throttled HEAD (503 SlowDown / 429), a
# 5xx, a transport error, and AccessDenied are all "present-or-unknown": the
# caller must retry or propagate, never treat them as "not there". Turning any
# of those into absence is what lets render summaries drop real artifacts,
# mosaic refresh publish incomplete manifests, and program cleanup get skipped.

def s3_error_code(exc):
    """Best-effort S3/botocore error code for exc as a string ('' if none).

    Prefers the structured ClientError code; falls back to the HTTP status so a
    bare 404/403/503 is still classifiable when no error code is present."""
    response = getattr(exc, "response", None)
    if isinstance(response, dict):
        code = (response.get("Error") or {}).get("Code")
        if code:
            return str(code)
        status = (response.get("ResponseMetadata") or {}).get("HTTPStatusCode")
        if status:
            return str(status)
    return ""


def is_missing_s3_error(exc):
    """True only when exc means the object is genuinely absent (404/NoSuchKey).

    Everything else — throttling, 5xx, transport failures, AccessDenied,
    NoSuchBucket, malformed responses — returns False so the caller retries or
    propagates instead of reporting a real object as missing. NoSuchBucket is a
    configuration error, not an absent object, so it is deliberately excluded."""
    code = s3_error_code(exc)
    if code in {"NoSuchKey", "404", "NotFound"}:
        return True
    if code:
        # A real S3/HTTP code that is not absence (403, 503, 5xx, SlowDown, ...).
        return False
    # No structured code (a non-ClientError wrapper): match explicit absence
    # markers in the message only — never a blanket True.
    msg = str(exc)
    return "NoSuchKey" in msg or "NotFound" in msg


_S3_ACCESS_DENIED_CODES = {
    "AccessDenied", "403", "AllAccessDisabled",
    "InvalidAccessKeyId", "SignatureDoesNotMatch",
}
_S3_THROTTLE_CODES = {
    "SlowDown", "429", "503", "ServiceUnavailable",
    "RequestLimitExceeded", "Throttling", "ThrottlingException",
}
_S3_TRANSPORT_EXC_NAMES = {
    "EndpointConnectionError", "ConnectTimeoutError", "ReadTimeoutError",
    "ConnectionClosedError", "ConnectionError", "IncompleteReadError",
}


_S3_SERVER_ERROR_CODES = {"InternalError", "InternalServerError", "ServiceUnavailable"}


def s3_http_status(exc):
    """Numeric HTTP status for an S3 exception, or 0 if none is present."""
    response = getattr(exc, "response", None)
    if isinstance(response, dict):
        try:
            return int((response.get("ResponseMetadata") or {}).get("HTTPStatusCode") or 0)
        except (TypeError, ValueError):
            return 0
    return 0


def s3_error_reason(exc):
    """Coarse reason bucket for a failed S3 op, for surfacing per-reason error
    counts where a fail-soft read still wants to say *why* a key was
    unavailable: 'missing' | 'access_denied' | 'throttled' | 'server_error' |
    'transport' | 'error'."""
    code = s3_error_code(exc)
    status = s3_http_status(exc)
    if code in {"NoSuchKey", "404", "NotFound"} or status == 404:
        return "missing"
    if code in _S3_ACCESS_DENIED_CODES or status == 403:
        return "access_denied"
    if code in _S3_THROTTLE_CODES or status in (429, 503):
        return "throttled"
    if code in _S3_SERVER_ERROR_CODES or code[:1] == "5" or 500 <= status < 600:
        return "server_error"
    if isinstance(exc, (ConnectionError, TimeoutError)):
        return "transport"
    if type(exc).__name__ in _S3_TRANSPORT_EXC_NAMES:
        return "transport"
    return "error"


def build_tmp_enospc_message(*, solver_label, phase, tmp_file, coeffs_key,
                             coeffs_size, n_coeffs, n_steps, job_id,
                             chunk_idx, task_id):
    degree = max(0, int(n_coeffs) - 1)
    roots_bytes = int(n_steps) * degree * 8
    peak_tmp_bytes = int(coeffs_size) + roots_bytes
    stats = tmp_space_stats("/tmp")
    return (
        f"{solver_label} {phase} failed: no space left on device while writing {tmp_file} "
        f"(device={stats['path']}, free={format_bytes(stats['free_bytes'])}, total={format_bytes(stats['total_bytes'])}, "
        f"coeffs=s3://{BUCKET}/{coeffs_key}, coeffs_size={format_bytes(coeffs_size)}, "
        f"estimated_roots_size={format_bytes(roots_bytes)}, estimated_peak_tmp={format_bytes(peak_tmp_bytes)}, "
        f"n_coeffs={n_coeffs}, degree={degree}, n_steps={n_steps}, job={job_id}, chunk={chunk_idx}, task={task_id})"
    )
