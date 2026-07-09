"""
Shared utilities for all polypaint Lambda handlers.
"""
import json
import math
import os
import re
import struct
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


def assert_render_identity(key, job_id, artifact_id, label="image_key"):
    """A render image key must belong to the same artifact its sibling fields
    name, so a book/PDF page can't pair image B with metadata A. Ties the key
    to renders/<job_id>/ and to a /<artifact_id>/ path segment
    (code-review-26 F3). Assumes the key already passed
    assert_safe_render_image_key."""
    k = str(key or "")
    jid = str(job_id or "")
    aid = str(artifact_id or "")
    if not jid or not k.startswith(f"renders/{jid}/"):
        raise ValueError(f"{label} {k!r} is not under renders/{jid}/ (job_id mismatch)")
    if not aid or f"/{aid}/" not in k:
        raise ValueError(f"{label} {k!r} does not contain /{aid}/ (artifact_id mismatch)")
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
