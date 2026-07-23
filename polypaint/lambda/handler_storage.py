"""
Storage Lambda handler — S3 metadata operations + DynamoDB status tracking.

Routes:
  POST /list           — list all computed jobs with metadata
  POST /list-favorites — list persisted favorite Color artifact refs
  POST /add-favorite   — add one favorite Color artifact ref
  POST /delete-favorite — delete one favorite Color artifact ref
  POST /list-custom-palettes — fetch the named custom-palette catalog
  POST /save-custom-palettes — conditionally replace the custom-palette catalog
  POST /list-palettes  — list saved palette variants for one job
  POST /delete-palette — delete one saved palette variant
  POST /delete-render-artifact — delete one immutable render artifact variant
  POST /delete         — delete all S3 objects for a job
  POST /save-metadata  — save calc.json to S3
  POST /cleanup        — delete a list of S3 keys
  POST /clean-render   — delete render artifacts (.raw/.jpeg/.png) for a job, preserving .bin
  POST /check-keys     — check which S3 keys exist (for polling render completion)
  POST /check-status   — query DynamoDB for task completion counts (replaces check-keys)
  POST /presign        — generate a presigned URL for an S3 key
  POST /share-mosaic   — snapshot the current AllCol/AllPal manifest and return a standalone share URL
  POST /snapshot-book-cover — freeze the current AllCol flat wall as a Book cover source
"""
import json
import math
import os
import re
import time
import uuid
from urllib.parse import urlencode

import boto3
from botocore.config import Config
from botocore.exceptions import ClientError

from color_artifact_meta import color_artifact_meta_key
from color_render_contract import normalize_color_interpretation
from logical_sections import (
    AUTO_FIXED_OVERHEAD_MB,
    AUTO_PER_THREAD_OVERHEAD_MB,
    AUTO_USABLE_FRACTION,
    DEFAULT_PALETTE_CHUNK_MEMORY_MB,
    DEFAULT_RASTER_MEMORY_MB,
    DEFAULT_SOLVE_SCORE_MEMORY_MB,
    summarize_chunk_items,
)
from shared import (BUCKET, JOBS_TABLE, PRESIGN_EXPIRY, parse_body, ok_response,
                    _get_ddb, parse_boolish, assert_safe_render_image_key,
                    assert_render_identity, is_missing_s3_error, s3_error_reason,
                    s3_error_code, assert_safe_id, CACHE_IMMUTABLE, report_status)
from coeff_program_chain import (
    PROGRAM_KIND as COEFF_PROGRAM_KIND,
    PROGRAM_VERSION as COEFF_PROGRAM_VERSION,
    compile_coeff_program_chain,
)
from coeff_program_source import (
    coeff_source_text_from_chain,
    coeff_source_text_from_payload,
    parse_coeff_program_source,
)
from param_program_chain import (
    PROGRAM_KIND as PARAM_PROGRAM_KIND,
    PROGRAM_VERSION as PARAM_PROGRAM_VERSION,
    compile_param_program_chain,
)
from param_program_source import (
    compile_param_program_source,
    param_source_text_from_chain,
    param_source_text_from_payload,
    parse_param_program_source,
)
from root_program_source import compile_root_program_source
from solve_score_chain import (
    compile_solve_score_chain_or_legacy,
    compiled_solve_score_fingerprint,
    serialize_solve_score_chain,
)
from solve_score_program_source import (
    SolveScoreProgramSourceError,
    compile_solve_score_program_source,
    solve_score_source_text_from_chain,
)
from program_v2_translate import (
    V2_PROGRAM_VERSION,
    V2_SPEC_VERSION,
    translate_coeff_from_old,
    translate_param_from_old,
    translate_solve_score_from_old,
    v1_summary,
)
# Run (compute) migration: translate a calc.json's legacy param/coeff transform
# chains into compiled programs, mirroring handler_render_lores_preview._calc_pipeline.
from pipeline_programs import (
    coeff_transforms_to_program_chain,
    param_transforms_to_program_chain,
)
from program_compile_helpers import (
    compiled_coeff_program_payload,
    compiled_param_program_payload,
)

s3 = boto3.client("s3")

MOSAIC_STATUS_JOB_ID = "__allrenders_mosaic__"
MOSAIC_STATUS_TASK_ID = "color_mosaic_status"
MOSAIC_TASK_IDS = {
    "color": "color_mosaic_status",
    "palette": "palette_mosaic_status",
}
MOSAIC_STATUS_SCHEMA_VERSION = 1
MOSAIC_STATUS_STALE_MS = 30 * 60 * 1000
MOSAIC_WORKERS = 24
MOSAIC_KEEP_LAST = 10
MOSAIC_PROGRESS_JOB_INTERVAL = 10
MOSAIC_PROGRESS_ARTIFACT_INTERVAL = 25
MOSAIC_BASE_URL = f"https://{BUCKET}.s3.{os.environ.get('AWS_REGION', 'us-east-1')}.amazonaws.com/"
MOSAIC_PREFIX = "renders/_index/color_mosaic/"
MOSAIC_PREFIXES = {
    "color": "renders/_index/color_mosaic/",
    "palette": "renders/_index/palette_mosaic/",
}
MOSAIC_SHARE_PREFIX = "renders/_shared_mosaic/"
MOSAIC_SHARE_VIEWER_KEY = "artifact_mosaic_viewer.html"
MOSAIC_SORT_MODES = {"date", "job", "function", "degree", "N", "random"}
MOSAIC_INTERNAL_ACTIONS = {
    "color": "build_color_mosaic",
    "palette": "build_palette_mosaic",
}
FAVORITES_KEY = "polypaint/favorites/color_artifacts.json"
FAVORITES_DDB_JOB_ID = "favorites#color"
CUSTOM_PALETTE_CATALOG_KEY = "polypaint/palettes/custom.json"
CUSTOM_PALETTE_CATALOG_SCHEMA_VERSION = 1
CUSTOM_PALETTE_MAX_ENTRIES = 256
CUSTOM_PALETTE_MAX_NAME_LEN = 80
CUSTOM_PALETTE_MIN_STOPS = 2
CUSTOM_PALETTE_MAX_STOPS = 32
_CUSTOM_PALETTE_HEX_RE = re.compile(r"^[0-9a-fA-F]{6}$")
# Results catalog (results-list.md Phase 2): one DDB row per computed job with
# the Results-table fields, so /list is a Query + cheap prefix listing instead
# of a calc.json GET per job. Self-healing: rows are written the first time a
# job's calc.json is read and pruned when its prefix vanishes.
RESULTS_CATALOG_DDB_JOB_ID = "results#catalog"
# A renders/<job>/ prefix can exist before calc.json lands (mid-compute):
# re-probe calc-less rows younger than this; older ones are permanently junk.
RESULTS_CATALOG_NO_CALC_RETRY_MS = 24 * 3600 * 1000
# Bump whenever _results_entry_fields() or the stored row shape changes: rows
# from older schemas reconcile automatically instead of decoding as zeros
# forever (code-review-30 F10).
RESULTS_CATALOG_SCHEMA_VERSION = 2
FAVORITES_DDB_META_TASK_ID = "__meta__"
FAVORITES_DDB_TASK_PREFIX = "favorite#"
# Bump when the persisted favorite display snapshot shape changes; a row whose
# stored version != this is treated as legacy and re-resolved by exact key
# (favorites-speedup.md Proposal 3).
FAVORITE_SNAPSHOT_VERSION = 2
# Display-only fields projected from a resolved Color artifact entry into the
# compact per-favorite snapshot. Enough to render the Favorites panel row
# (Added / Job / Dims / Size / Summary + preview) without a /render-summary
# fan-out; URLs are derived client-side from the keys, so none are stored (they
# would otherwise be stale one-hour presigns).
_FAVORITE_SNAPSHOT_FIELDS = (
    "created_at", "image_key", "preview_key", "width", "height", "file_size", "size",
    "content_type", "format", "color_mode", "palette", "palette_display_name",
    "color_interpretation",
    "postprocess_kind", "derivation_kind", "derived_from_artifact_id",
    "source_artifact_id", "source_color_artifact_id", "derived_from_color_artifact_id",
    "solve_score_chain", "solve_score_program_source_text", "score_source_text",
    "solve_metric", "solve_score_quantile", "threshold", "legacy", "view_mode",
)
SOLVE_SCORE_PROGRAMS_PREFIX = "polypaint/solve-score-programs/"
SOLVE_SCORE_PROGRAM_VERSION = 1
SOLVE_SCORE_PROGRAM_META_NAME = "solve_score_name"
SOLVE_SCORE_PROGRAM_META_STATEMENT_COUNT = "solve_score_statement_count"
SOLVE_SCORE_PROGRAM_META_SAVED_AT = "solve_score_saved_at"
MAX_SOLVE_SCORE_PROGRAM_NAME_LEN = 120
MAX_SOLVE_SCORE_PROGRAM_STATEMENTS = 256
MAX_SOLVE_SCORE_PROGRAM_CHAIN_BYTES = 16 * 1024
MAX_SOLVE_SCORE_PROGRAM_TOKEN_LEN = 128
PARAM_PROGRAMS_PREFIX = "polypaint/param-programs/"
PARAM_PROGRAM_META_NAME = "param_program_name"
PARAM_PROGRAM_META_STATEMENT_COUNT = "param_program_statement_count"
PARAM_PROGRAM_META_SAVED_AT = "param_program_saved_at"
MAX_PARAM_PROGRAM_NAME_LEN = 120
MAX_PARAM_PROGRAM_STATEMENTS = 256
MAX_PARAM_PROGRAM_CHAIN_BYTES = 24 * 1024
MAX_PARAM_PROGRAM_TOKEN_LEN = 256
ROOT_PROGRAMS_PREFIX = "polypaint/root-programs/"
BOOKS_PREFIX = "polypaint/books/"
BOOK_META_NAME = "book_name"
BOOK_META_ENTRY_COUNT = "book_entry_count"
BOOK_META_SAVED_AT = "book_saved_at"
MAX_BOOK_NAME_LEN = 120
MAX_BOOK_ENTRIES = 200
BOOK_COVER_SOURCE_VERSION = 1
BOOK_DEFAULT_BACKGROUND_COLOR = "1a1a2e"
_BOOK_WALL_REFRESH_RE = re.compile(r"mosaic_[0-9A-Za-z]+_[0-9a-f]{6,12}")
ROOT_PROGRAM_VERSION = 1
ROOT_PROGRAM_META_NAME = "root_program_name"
ROOT_PROGRAM_META_STATEMENT_COUNT = "root_program_statement_count"
ROOT_PROGRAM_META_SAVED_AT = "root_program_saved_at"


class CustomPaletteCatalogConflictError(Exception):
    """The named custom-palette catalog changed after the caller read it."""


class CustomPaletteCatalogReadError(Exception):
    """The stored catalog exists but cannot be decoded safely."""


MAX_ROOT_PROGRAM_NAME_LEN = 120
COEFF_PROGRAMS_PREFIX = "polypaint/coeff-programs/"
COEFF_PROGRAM_META_NAME = "coeff_program_name"
COEFF_PROGRAM_META_STATEMENT_COUNT = "coeff_program_statement_count"
COEFF_PROGRAM_META_SAVED_AT = "coeff_program_saved_at"
MAX_COEFF_PROGRAM_NAME_LEN = 120
MAX_COEFF_PROGRAM_STATEMENTS = 256
MAX_COEFF_PROGRAM_CHAIN_BYTES = 32 * 1024
MAX_COEFF_PROGRAM_TOKEN_LEN = 256
DEFAULT_RESULTS_LIST_WORKERS = 32
MAX_RESULTS_LIST_WORKERS = 64


class _SolveScoreProgramNotFound(RuntimeError):
    pass


class _ParamProgramNotFound(RuntimeError):
    pass


class _CoeffProgramNotFound(RuntimeError):
    pass


class _MigrationConflict(RuntimeError):
    status_code = 409

    def __init__(self, message, *, existing_fingerprint="", migrated_fingerprint=""):
        super().__init__(message)
        self.existing_fingerprint = existing_fingerprint
        self.migrated_fingerprint = migrated_fingerprint


class _MigrationMissingMacros(RuntimeError):
    status_code = 422

    def __init__(self, missing):
        self.missing = list(missing or [])
        super().__init__("macro not migrated: " + ", ".join(self.missing))


def _validate_results_list_workers(value):
    if value in (None, ""):
        value = DEFAULT_RESULTS_LIST_WORKERS
    try:
        workers = int(value)
    except (TypeError, ValueError):
        raise ValueError(f"list_workers must be an integer, got {value!r}")
    if not (1 <= workers <= MAX_RESULTS_LIST_WORKERS):
        raise ValueError(
            f"list_workers must be in [1, {MAX_RESULTS_LIST_WORKERS}], got {workers}"
        )
    return workers


def _results_list_pool_size(workers):
    return max(16, int(workers) * 2)


def _error_response(status_code, message):
    return {
        "statusCode": int(status_code),
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps({"error": str(message)[:1000]}),
    }


def _json_error_response(status_code, body):
    payload = dict(body or {})
    if "error" in payload:
        payload["error"] = str(payload["error"])[:1000]
    return {
        "statusCode": int(status_code),
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps(payload),
    }


def _client_error_message(exc):
    response = getattr(exc, "response", {}) or {}
    err = response.get("Error") or {}
    code = str(err.get("Code") or exc.__class__.__name__)
    message = str(err.get("Message") or exc)
    return f"{code}: {message}"


def _handle_storage_route(fn, event):
    try:
        return fn(event)
    except (_SolveScoreProgramNotFound, _ParamProgramNotFound, _CoeffProgramNotFound, _RootProgramNotFound, _BookNotFound, _GalleryNotFound) as exc:
        return _error_response(404, exc)
    except _MigrationConflict as exc:
        return _json_error_response(409, {
            "error": str(exc),
            "existing_fingerprint": exc.existing_fingerprint,
            "migrated_fingerprint": exc.migrated_fingerprint,
        })
    except BookConflictError as exc:
        return _json_error_response(409, {"error": str(exc), "conflict": "book_saved_at"})
    except GalleryConflictError as exc:
        return _json_error_response(409, {"error": str(exc), "conflict": "gallery_revision"})
    except CustomPaletteCatalogConflictError as exc:
        return _json_error_response(409, {"error": str(exc), "conflict": "custom_palette_revision"})
    except _MigrationMissingMacros as exc:
        return _json_error_response(422, {"error": "macro not migrated", "missing": exc.missing})
    except ClientError as exc:
        response = getattr(exc, "response", {}) or {}
        code = str((response.get("Error") or {}).get("Code") or "")
        status = 404 if code in {"NoSuchKey", "NoSuchBucket", "404", "NotFound"} else 500
        return _error_response(status, _client_error_message(exc))
    except (ValueError, KeyError, TypeError, RuntimeError, json.JSONDecodeError) as exc:
        return _error_response(400, exc)
    except Exception as exc:
        return _error_response(500, exc)


def _utc_now_iso():
    return time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())


def _slugify_solve_score_program_id(name):
    text = str(name or "").strip().lower()
    slug = re.sub(r"[^a-z0-9]+", "-", text).strip("-")
    slug = slug[:64].strip("-")
    return slug or "solve-score-program"


def _slugify_param_program_id(name):
    text = str(name or "").strip().lower()
    slug = re.sub(r"[^a-z0-9]+", "-", text).strip("-")
    slug = slug[:64].strip("-")
    return slug or "param-program"


def _slugify_coeff_program_id(name):
    text = str(name or "").strip().lower()
    slug = re.sub(r"[^a-z0-9]+", "-", text).strip("-")
    slug = slug[:64].strip("-")
    return slug or "coeff-program"


def _normalize_program_id(program_id):
    """Strip any stray ``v2/`` namespace prefix from a program id.

    Saved program ids are flat slugs and never contain ``/``, so this only ever
    undoes a phantom id (e.g. one that leaked from an old list surfacing the
    ``v2/`` subdirectory). It guarantees a v2-key can never double-prefix into
    ``.../v2/v2/<id>.json`` no matter what id a (possibly stale) client sends.
    """
    pid = str(program_id or "").strip()
    while pid.startswith("v2/"):
        pid = pid[len("v2/"):]
    return pid


def _solve_score_program_key(program_id):
    return f"{SOLVE_SCORE_PROGRAMS_PREFIX}{_normalize_program_id(program_id)}.json"


def _solve_score_program_v2_key(program_id):
    return f"{SOLVE_SCORE_PROGRAMS_PREFIX}v2/{_normalize_program_id(program_id)}.json"


def _param_program_key(program_id):
    return f"{PARAM_PROGRAMS_PREFIX}{_normalize_program_id(program_id)}.json"


def _param_program_v2_key(program_id):
    return f"{PARAM_PROGRAMS_PREFIX}v2/{_normalize_program_id(program_id)}.json"


def _coeff_program_key(program_id):
    return f"{COEFF_PROGRAMS_PREFIX}{_normalize_program_id(program_id)}.json"


def _coeff_program_v2_key(program_id):
    return f"{COEFF_PROGRAMS_PREFIX}v2/{_normalize_program_id(program_id)}.json"


def _drop_stale_program_v2_key(v2_key, *, s3_client=None, bucket=None):
    """Remove the migrated v2 copy when its v1 source is re-saved.

    Fetch prefers the v2 key, so a stale v2 copy would shadow every later
    edit forever (and re-migration would 409 on the conflict). A v1 re-save
    supersedes the derived v2 artifact; migration can recreate it.

    Deletes unconditionally: S3 DELETE is idempotent (a no-op on a missing
    key), so a preceding HEAD probe added nothing but a failure mode — a
    transient throttle on that HEAD used to skip the delete and leave the
    stale v2 shadowing the new v1 (code-review-28 F8). A transient DELETE
    error now propagates so the save fails and is retried, rather than
    silently leaving a zombie v2.
    """
    client = s3_client or s3
    client.delete_object(Bucket=bucket or BUCKET, Key=v2_key)


def _put_program_v1_object(v1_key, v2_key, body, *, content_type="application/json",
                           metadata=None, s3_client=None, bucket=None):
    """Single primitive for writing a saved-program v1 object.

    Writes the v1 body, then idempotently drops any migrated v2 copy so fetch
    (which prefers v2) cannot keep serving stale bytes after a v1 re-save
    (code-review-28 F8). Both the API save handlers and the offline
    uploader/seed scripts route through this (the scripts pass their own
    client/bucket), so no writer can forget the v2 cleanup and report a
    successful overwrite while fetch shadows it."""
    client = s3_client or s3
    target_bucket = bucket or BUCKET
    kwargs = {"Bucket": target_bucket, "Key": v1_key, "Body": body, "ContentType": content_type}
    if metadata is not None:
        kwargs["Metadata"] = metadata
    client.put_object(**kwargs)
    _drop_stale_program_v2_key(v2_key, s3_client=client, bucket=target_bucket)


def _delete_program_keys(key, v2_key):
    """Delete a saved program's v1 AND migrated v2 keys.

    Returns the number of keys removed; 0 means neither existed (caller
    raises its kind-specific not-found). Deleting only v1 left a v2 zombie
    that fetch kept serving while list hid it.
    """
    deleted = 0
    if _key_exists(key):
        s3.delete_object(Bucket=BUCKET, Key=key)
        deleted += 1
    if _key_exists(v2_key):
        s3.delete_object(Bucket=BUCKET, Key=v2_key)
        deleted += 1
    return deleted


def _validate_solve_score_program_name(name):
    text = str(name or "").strip()
    if not text:
        raise ValueError("solve-score program name is required")
    if len(text) > MAX_SOLVE_SCORE_PROGRAM_NAME_LEN:
        raise ValueError(
            f"solve-score program name must be at most {MAX_SOLVE_SCORE_PROGRAM_NAME_LEN} characters"
        )
    if any(ch in "\r\n\t" for ch in text) or not all(ch.isprintable() for ch in text):
        raise ValueError("solve-score program name must contain printable single-line text")
    return text


def _validate_param_program_name(name):
    text = str(name or "").strip()
    if not text:
        raise ValueError("param program name is required")
    if len(text) > MAX_PARAM_PROGRAM_NAME_LEN:
        raise ValueError(
            f"param program name must be at most {MAX_PARAM_PROGRAM_NAME_LEN} characters"
        )
    if any(ch in "\r\n\t" for ch in text) or not all(ch.isprintable() for ch in text):
        raise ValueError("param program name must contain printable single-line text")
    return text


def _validate_coeff_program_name(name):
    text = str(name or "").strip()
    if not text:
        raise ValueError("coeff program name is required")
    if len(text) > MAX_COEFF_PROGRAM_NAME_LEN:
        raise ValueError(
            f"coeff program name must be at most {MAX_COEFF_PROGRAM_NAME_LEN} characters"
        )
    if any(ch in "\r\n\t" for ch in text) or not all(ch.isprintable() for ch in text):
        raise ValueError("coeff program name must contain printable single-line text")
    return text


def _validate_solve_score_program_chain_value(value, path):
    if isinstance(value, str):
        if len(value) > MAX_SOLVE_SCORE_PROGRAM_TOKEN_LEN:
            raise ValueError(
                f"{path} string token must be at most {MAX_SOLVE_SCORE_PROGRAM_TOKEN_LEN} characters"
            )
        return
    if isinstance(value, (int, float)):
        return
    if isinstance(value, list):
        for idx, item in enumerate(value):
            _validate_solve_score_program_chain_value(item, f"{path}[{idx}]")
        return
    raise ValueError(f"{path} must contain only arrays, strings, and numbers")


def _validate_param_program_chain_value(value, path):
    if isinstance(value, str):
        if len(value) > MAX_PARAM_PROGRAM_TOKEN_LEN:
            raise ValueError(
                f"{path} string token must be at most {MAX_PARAM_PROGRAM_TOKEN_LEN} characters"
            )
        return
    if isinstance(value, (int, float)):
        return
    if isinstance(value, list):
        for idx, item in enumerate(value):
            _validate_param_program_chain_value(item, f"{path}[{idx}]")
        return
    raise ValueError(f"{path} must contain only arrays, strings, and numbers")


def _validate_coeff_program_chain_value(value, path):
    if isinstance(value, str):
        if len(value) > MAX_COEFF_PROGRAM_TOKEN_LEN:
            raise ValueError(
                f"{path} string token must be at most {MAX_COEFF_PROGRAM_TOKEN_LEN} characters"
            )
        return
    if isinstance(value, (int, float)):
        return
    if isinstance(value, list):
        for idx, item in enumerate(value):
            _validate_coeff_program_chain_value(item, f"{path}[{idx}]")
        return
    raise ValueError(f"{path} must contain only arrays, strings, and numbers")


def _compile_solve_score_program_payload(
    name,
    chain,
    *,
    source_text=None,
    saved_at=None,
    version=SOLVE_SCORE_PROGRAM_VERSION,
    recommended_interpretation=None,
):
    validated_name = _validate_solve_score_program_name(name)
    source_text_value = str(source_text or "")
    source_text_authoritative = bool(source_text_value.strip())
    if source_text_authoritative:
        source_compiled = compile_solve_score_program_source(source_text_value, strict=True)
        chain = source_compiled["chain"]
    if not isinstance(chain, list) or not chain:
        raise ValueError("solve-score program chain must be a non-empty JSON array")
    if len(chain) > MAX_SOLVE_SCORE_PROGRAM_STATEMENTS:
        raise ValueError(
            f"solve-score program chain must contain at most {MAX_SOLVE_SCORE_PROGRAM_STATEMENTS} statements"
        )
    _validate_solve_score_program_chain_value(chain, "chain")
    chain_json = json.dumps(chain, separators=(",", ":"), ensure_ascii=False)
    if len(chain_json.encode("utf-8")) > MAX_SOLVE_SCORE_PROGRAM_CHAIN_BYTES:
        raise ValueError(
            f"solve-score program chain JSON must be at most {MAX_SOLVE_SCORE_PROGRAM_CHAIN_BYTES} bytes"
        )

    compiled = compile_solve_score_chain_or_legacy(
        chain,
        "",
        default_metric="proximity",
    )
    canonical_chain = json.loads(serialize_solve_score_chain(compiled["chain"]))
    if not source_text_value.strip():
        source_text_value = solve_score_source_text_from_chain(canonical_chain)
    saved_at_text = _utc_now_iso() if saved_at is None else str(saved_at or "").strip()
    try:
        version_num = int(version)
    except (TypeError, ValueError):
        version_num = SOLVE_SCORE_PROGRAM_VERSION
    program = {
        "version": version_num,
        "id": _slugify_solve_score_program_id(validated_name),
        "name": validated_name,
        "chain": canonical_chain,
        "metric": compiled["metric"],
        "display": compiled["display"],
        "source_text": source_text_value,
        "source_text_authoritative": source_text_authoritative,
        "source_display": source_text_value.strip(),
        "source_statement_count": sum(1 for split in source_text_value.splitlines() if split.strip()),
        "program_spec": compiled["program_spec"],
        "fingerprint": compiled_solve_score_fingerprint(compiled),
        "output_channel_count": compiled.get("output_channel_count", 1),
        "output_channels": list(compiled.get("output_channels") or []),
        "has_explicit_outputs": bool(compiled.get("has_explicit_outputs")),
        "statement_count": len(canonical_chain),
        "saved_at": saved_at_text,
    }
    if recommended_interpretation not in ("", None):
        program["recommended_interpretation"] = normalize_color_interpretation(recommended_interpretation)
    return program


def _read_param_program_source_chain(program_id):
    key = _param_program_key(program_id)
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise _ParamProgramNotFound(f"param program not found: {program_id}")
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"saved param program is not valid JSON: {program_id}") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"saved param program must be a JSON object: {program_id}")
    source_text = param_source_text_from_payload(payload)
    if source_text is not None:
        parsed = parse_param_program_source(source_text)
        return parsed["chain"]
    chain = payload.get("chain")
    if not isinstance(chain, list):
        raise RuntimeError(f"saved param program chain must be a JSON array: {program_id}")
    return chain


def _param_program_macro_resolver(current_program_id=None):
    def _resolve(macro_id):
        macro_id_text = str(macro_id or "").strip()
        if current_program_id and macro_id_text == current_program_id:
            raise ValueError(f"param program cannot reference itself as macro({macro_id_text})")
        return _read_param_program_source_chain(macro_id_text)

    return _resolve


def _read_coeff_program_source_chain(program_id):
    key = _coeff_program_key(program_id)
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise _CoeffProgramNotFound(f"coeff program not found: {program_id}")
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"saved coeff program is not valid JSON: {program_id}") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"saved coeff program must be a JSON object: {program_id}")
    source_text = coeff_source_text_from_payload(payload)
    if source_text is not None:
        parsed = parse_coeff_program_source(source_text)
        return parsed["chain"]
    chain = payload.get("chain")
    if not isinstance(chain, list):
        raise RuntimeError(f"saved coeff program chain must be a JSON array: {program_id}")
    return chain


def _coeff_program_macro_resolver(current_program_id=None):
    def _resolve(macro_id):
        macro_id_text = str(macro_id or "").strip()
        if current_program_id and macro_id_text == current_program_id:
            raise ValueError(f"coeff program cannot reference itself as macro({macro_id_text})")
        return _read_coeff_program_source_chain(macro_id_text)

    return _resolve


def _compile_param_program_payload(
    name,
    chain=None,
    *,
    source_text=None,
    saved_at=None,
    version=PARAM_PROGRAM_VERSION,
    program_id=None,
):
    validated_name = _validate_param_program_name(name)
    parsed_source = None
    if source_text is not None:
        source_text = str(source_text or "")
        parsed_source = parse_param_program_source(source_text)
        chain = parsed_source["chain"]
    if not isinstance(chain, list) or not chain:
        raise ValueError("param program chain must be a non-empty JSON array")
    if len(chain) > MAX_PARAM_PROGRAM_STATEMENTS:
        raise ValueError(
            f"param program chain must contain at most {MAX_PARAM_PROGRAM_STATEMENTS} statements"
        )
    _validate_param_program_chain_value(chain, "chain")
    chain_json = json.dumps(chain, separators=(",", ":"), ensure_ascii=False)
    if len(chain_json.encode("utf-8")) > MAX_PARAM_PROGRAM_CHAIN_BYTES:
        raise ValueError(
            f"param program chain JSON must be at most {MAX_PARAM_PROGRAM_CHAIN_BYTES} bytes"
        )

    program_id_text = str(program_id or _slugify_param_program_id(validated_name)).strip()
    compiled = compile_param_program_chain(
        chain,
        macro_resolver=_param_program_macro_resolver(program_id_text),
    )
    saved_at_text = _utc_now_iso() if saved_at is None else str(saved_at or "").strip()
    try:
        version_num = int(version)
    except (TypeError, ValueError):
        version_num = PARAM_PROGRAM_VERSION
    program = {
        "version": version_num,
        "program_kind": PARAM_PROGRAM_KIND,
        "id": program_id_text,
        "name": validated_name,
        "chain": compiled["source_chain"],
        "display": compiled["display"],
        "expanded_display": compiled["expanded_display"],
        "fingerprint": compiled["fingerprint"],
        "execution_spec": compiled["execution_spec"],
        # Source statements when saved from text, lowered rows otherwise —
        # same semantics as _compile_coeff_program_payload.
        "statement_count": int(parsed_source["statement_count"]) if parsed_source else len(compiled["source_chain"]),
        "token_count": compiled["token_count"],
        "stack_max": compiled["stack_max"],
        "emits": compiled["emits"],
        "uses_legacy_fast_path": compiled["uses_legacy_fast_path"],
        "macro_expansions": compiled["macro_expansions"],
        "saved_at": saved_at_text,
    }
    if parsed_source is not None:
        program["source_text"] = source_text
        program["source_display"] = parsed_source.get("display") or compiled["display"]
    if compiled["legacy_transforms"]:
        program["legacy_transforms"] = compiled["legacy_transforms"]
    return program


def _compile_coeff_program_payload(
    name,
    chain=None,
    *,
    source_text=None,
    saved_at=None,
    version=COEFF_PROGRAM_VERSION,
    program_id=None,
):
    validated_name = _validate_coeff_program_name(name)
    parsed_source = None
    if source_text is not None:
        source_text = str(source_text or "")
        parsed_source = parse_coeff_program_source(source_text)
        chain = parsed_source["chain"]
    if not isinstance(chain, list):
        raise ValueError("coeff program chain must be a JSON array")
    if len(chain) > MAX_COEFF_PROGRAM_STATEMENTS:
        raise ValueError(
            f"coeff program chain must contain at most {MAX_COEFF_PROGRAM_STATEMENTS} statements"
        )
    _validate_coeff_program_chain_value(chain, "chain")
    chain_json = json.dumps(chain, separators=(",", ":"), ensure_ascii=False)
    if len(chain_json.encode("utf-8")) > MAX_COEFF_PROGRAM_CHAIN_BYTES:
        raise ValueError(
            f"coeff program chain JSON must be at most {MAX_COEFF_PROGRAM_CHAIN_BYTES} bytes"
        )

    program_id_text = str(program_id or _slugify_coeff_program_id(validated_name)).strip()
    compiled = compile_coeff_program_chain(
        chain,
        macro_resolver=_coeff_program_macro_resolver(program_id_text),
    )
    saved_at_text = _utc_now_iso() if saved_at is None else str(saved_at or "").strip()
    try:
        version_num = int(version)
    except (TypeError, ValueError):
        version_num = COEFF_PROGRAM_VERSION
    program = {
        "version": version_num,
        "program_kind": COEFF_PROGRAM_KIND,
        "id": program_id_text,
        "name": validated_name,
        "chain": compiled["source_chain"],
        "display": compiled["display"],
        "expanded_display": compiled["expanded_display"],
        "fingerprint": compiled["fingerprint"],
        "execution_spec": compiled["execution_spec"],
        "statement_count": int(parsed_source["statement_count"]) if parsed_source else len(compiled["source_chain"]),
        "token_count": compiled["token_count"],
        "scalar_expr_count": compiled["scalar_expr_count"],
        "stack_max": compiled["stack_max"],
        "uses_legacy_chain_equivalent": compiled["uses_legacy_chain_equivalent"],
        "macro_expansions": compiled["macro_expansions"],
        "saved_at": saved_at_text,
    }
    if parsed_source is not None:
        program["source_text"] = source_text
        program["source_display"] = parsed_source["display"]
    if compiled["legacy_coeff_transforms"]:
        program["legacy_coeff_transforms"] = compiled["legacy_coeff_transforms"]
    return program


def _read_solve_score_program_object(program_id, prefer_v2=False):
    key = _solve_score_program_key(program_id)
    from_v2 = False
    if prefer_v2 and _key_exists(_solve_score_program_v2_key(program_id)):
        key = _solve_score_program_v2_key(program_id)
        from_v2 = True
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise _SolveScoreProgramNotFound(f"solve-score program not found: {program_id}")
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"saved solve-score program is not valid JSON: {program_id}") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"saved solve-score program must be a JSON object: {program_id}")
    program = _compile_solve_score_program_payload(
        payload.get("name"),
        payload.get("chain"),
        source_text=(
            # Persisted bodies are compiled-program dicts and always carry
            # source_text; the long-key alias exists only for request
            # params (see handle_save/fetch_solve_score_program).
            payload.get("source_text")
            if (from_v2 or payload.get("source_text_authoritative"))
            else None
        ),
        saved_at=payload.get("saved_at", ""),
        version=payload.get("version", SOLVE_SCORE_PROGRAM_VERSION),
        recommended_interpretation=payload.get("recommended_interpretation"),
    )
    program["id"] = str(program_id)
    return program


def _read_param_program_object(program_id, prefer_v2=False):
    key = _param_program_key(program_id)
    if prefer_v2 and _key_exists(_param_program_v2_key(program_id)):
        key = _param_program_v2_key(program_id)
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise _ParamProgramNotFound(f"param program not found: {program_id}")
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"saved param program is not valid JSON: {program_id}") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"saved param program must be a JSON object: {program_id}")
    if payload.get("program_kind") not in (None, PARAM_PROGRAM_KIND):
        raise RuntimeError(f"saved program is not a param program: {program_id}")
    program = _compile_param_program_payload(
        payload.get("name"),
        payload.get("chain"),
        source_text=param_source_text_from_payload(payload),
        saved_at=payload.get("saved_at", ""),
        version=payload.get("version", PARAM_PROGRAM_VERSION),
        program_id=str(program_id),
    )
    return program


def _read_coeff_program_object(program_id, prefer_v2=False):
    key = _coeff_program_key(program_id)
    if prefer_v2 and _key_exists(_coeff_program_v2_key(program_id)):
        key = _coeff_program_v2_key(program_id)
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise _CoeffProgramNotFound(f"coeff program not found: {program_id}")
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"saved coeff program is not valid JSON: {program_id}") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"saved coeff program must be a JSON object: {program_id}")
    if payload.get("program_kind") not in (None, COEFF_PROGRAM_KIND):
        raise RuntimeError(f"saved program is not a coeff program: {program_id}")
    program = _compile_coeff_program_payload(
        payload.get("name"),
        payload.get("chain"),
        source_text=coeff_source_text_from_payload(payload),
        saved_at=payload.get("saved_at", ""),
        version=payload.get("version", COEFF_PROGRAM_VERSION),
        program_id=str(program_id),
    )
    return program


def _read_json_program_key(key, label):
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise RuntimeError(f"{label} not found")
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"{label} is not valid JSON") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"{label} must be a JSON object")
    return payload


def _migration_kind_config(kind):
    raw = str(kind or "").strip().lower()
    if raw in {"solve-score", "solve_score", "solve"}:
        return {
            "kind": "solve-score",
            "read": _read_solve_score_program_object,
            "v2_key": _solve_score_program_v2_key,
            "put_meta": _solve_score_program_put_metadata,
            "translate": lambda program: translate_solve_score_from_old(program),
            "macro_kind": None,
        }
    if raw == "param":
        return {
            "kind": "param",
            "read": _read_param_program_object,
            "v2_key": _param_program_v2_key,
            "put_meta": _param_program_put_metadata,
            "translate": lambda program: translate_param_from_old(program, macro_resolver=_param_program_macro_resolver(program.get("id"))),
            "macro_kind": "param",
        }
    if raw == "coeff":
        return {
            "kind": "coeff",
            "read": _read_coeff_program_object,
            "v2_key": _coeff_program_v2_key,
            "put_meta": _coeff_program_put_metadata,
            "translate": lambda program: translate_coeff_from_old(program, macro_resolver=_coeff_program_macro_resolver(program.get("id"))),
            "macro_kind": "coeff",
        }
    raise ValueError(f"unknown program migration kind: {kind}")


def _v2_key_exists_for_kind(kind, program_id):
    if kind == "param":
        return _key_exists(_param_program_v2_key(program_id))
    if kind == "coeff":
        return _key_exists(_coeff_program_v2_key(program_id))
    if kind == "solve-score":
        return _key_exists(_solve_score_program_v2_key(program_id))
    raise ValueError(f"unknown migration kind: {kind}")


def _direct_macro_ids(chain):
    out = []
    for chip in chain or []:
        if isinstance(chip, list) and len(chip) >= 2 and str(chip[0]).strip().lower() == "macro":
            macro_id = str(chip[1] or "").strip()
            if macro_id:
                out.append(macro_id)
    return out


def _collect_missing_v2_macros(kind, program_id, seen):
    program_id = str(program_id or "").strip()
    if not program_id or program_id in seen:
        return []
    seen.add(program_id)
    cfg = _migration_kind_config(kind)
    program = cfg["read"](program_id)
    missing = []
    if not _v2_key_exists_for_kind(kind, program_id):
        missing.append(program_id)
    for child_id in _direct_macro_ids(program.get("chain") or []):
        missing.extend(_collect_missing_v2_macros(kind, child_id, seen))
    return missing


def _missing_v2_macros(kind, migrated):
    if kind not in {"param", "coeff"}:
        return []
    missing = []
    for macro_id in migrated.get("macro_ids") or []:
        missing.extend(_collect_missing_v2_macros(kind, macro_id, set()))
    return sorted(set(missing))


def _calc_pipeline_program_version(pipeline):
    """Structural program-version of a compute's calc.json pipeline.

    calc.json carries no explicit version. A run is v1 (legacy) while it still
    has non-empty param/coeff transform chains; it is v2 once those compile to
    param_program/coeff_program — the shape fresh program computes write.
    """
    pipeline = pipeline or {}
    has_legacy = bool(pipeline.get("param_transforms")) or bool(pipeline.get("coeff_transforms"))
    return 1 if has_legacy else 2


def _migrate_calc_pipeline(pipeline):
    """Compile a calc.json pipeline's legacy transforms to programs.

    Returns (migrated_pipeline, changed). Mirrors the recompute boundary in
    handler_render_lores_preview._calc_pipeline: legacy transforms compile to a
    program, the transform arrays clear, and the *_display fields preserve the
    original chain. Legacy transforms never reference macros, so no
    macro_resolver is needed. An existing program drops any stray transforms.
    """
    out = dict(pipeline or {})
    changed = False

    param_program = out.get("param_program") if isinstance(out.get("param_program"), dict) else {}
    param_transforms = out.get("param_transforms") if isinstance(out.get("param_transforms"), list) else []
    if not param_program and param_transforms:
        chain = param_transforms_to_program_chain(param_transforms)
        compiled = compile_param_program_chain(chain)
        out["param_program"] = compiled_param_program_payload(compiled)
        out["param_program_chain"] = chain
        out["param_program_display"] = compiled.get("display", "")
        out["param_program_fingerprint"] = compiled.get("fingerprint", "")
        out["param_program_uses_legacy_fast_path"] = bool(compiled.get("uses_legacy_fast_path"))
        if not out.get("param_transforms_display"):
            out["param_transforms_display"] = param_transforms
        out["param_transforms"] = []
        changed = True
    elif param_program and param_transforms:
        out["param_transforms"] = []
        changed = True

    coeff_program = out.get("coeff_program") if isinstance(out.get("coeff_program"), dict) else {}
    coeff_transforms = out.get("coeff_transforms") if isinstance(out.get("coeff_transforms"), list) else []
    if not coeff_program and coeff_transforms:
        chain = coeff_transforms_to_program_chain(coeff_transforms)
        compiled = compile_coeff_program_chain(chain)
        out["coeff_program"] = compiled_coeff_program_payload(compiled)
        out["coeff_program_chain"] = chain
        out["coeff_program_display"] = compiled.get("display", "")
        out["coeff_program_fingerprint"] = compiled.get("fingerprint", "")
        out["coeff_program_uses_legacy_chain_equivalent"] = bool(compiled.get("uses_legacy_chain_equivalent"))
        if not out.get("coeff_transforms_display"):
            out["coeff_transforms_display"] = coeff_transforms
        out["coeff_transforms"] = []
        changed = True
    elif coeff_program and coeff_transforms:
        out["coeff_transforms"] = []
        changed = True

    return out, changed


def _handle_migrate_compute(event):
    """Migrate one compute's stored calc.json pipeline from legacy v1 to v2.

    dry_run (default True) reports the detected version without writing.
    Idempotent: a run already in v2 reports already_current with wrote=False.
    Rewrites renders/<job_id>/calc.json in place; the original chain stays
    recoverable from the preserved *_transforms_display / *_program_chain fields.
    """
    params = parse_body(event)
    job_id = str(params.get("job_id") or "").strip()
    if not job_id:
        raise ValueError("compute migration requires job_id")
    dry_run = parse_boolish(params.get("dry_run", True), True, strict=True, label="dry_run")

    calc_key = f"renders/{job_id}/calc.json"
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=calc_key)
        calc = json.loads(obj["Body"].read())
    except Exception as exc:
        if _is_missing_s3_error(exc):
            return _error_response(404, f"compute {job_id} not found")
        raise

    pipeline = calc.get("pipeline") or {}
    from_version = _calc_pipeline_program_version(pipeline)
    migrated_pipeline, changed = _migrate_calc_pipeline(pipeline)

    response = {
        "job_id": job_id,
        "from_version": from_version,
        "to_version": 2,
        "wrote": False,
        "already_current": not changed,
    }
    if not changed or dry_run:
        return ok_response(response)

    calc["pipeline"] = migrated_pipeline
    s3.put_object(
        Bucket=BUCKET,
        Key=calc_key,
        Body=(json.dumps(calc) + "\n").encode("utf-8"),
        ContentType="application/json",
    )
    response["wrote"] = True
    return ok_response(response)


def _handle_migrate_program(event, kind):
    params = parse_body(event)
    cfg = _migration_kind_config(kind)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError(f"{cfg['kind']} program migration requires id")
    dry_run = parse_boolish(params.get("dry_run", True), True, strict=True, label="dry_run")
    program = cfg["read"](program_id)
    migrated = cfg["translate"](program)
    migrated["id"] = program_id
    migrated["saved_at"] = _utc_now_iso()
    response = {
        "id": program_id,
        "kind": cfg["kind"],
        "migrated": migrated,
        "v1": v1_summary(program),
        "wrote": False,
    }
    if dry_run:
        return ok_response(response)

    missing = _missing_v2_macros(cfg["kind"], migrated)
    if missing:
        raise _MigrationMissingMacros(missing)

    key = cfg["v2_key"](program_id)
    if _key_exists(key):
        existing = _read_json_program_key(key, f"v2 {cfg['kind']} program {program_id}")
        existing_fp = str(existing.get("fingerprint") or "")
        migrated_fp = str(migrated.get("fingerprint") or "")
        if existing_fp != migrated_fp:
            raise _MigrationConflict(
                "v2 exists",
                existing_fingerprint=existing_fp,
                migrated_fingerprint=migrated_fp,
            )
        return ok_response(response)

    s3.put_object(
        Bucket=BUCKET,
        Key=key,
        Body=(json.dumps(migrated, indent=2) + "\n").encode("utf-8"),
        ContentType="application/json",
        Metadata=cfg["put_meta"](migrated),
    )
    response["wrote"] = True
    return ok_response(response)


def _solve_score_program_put_metadata(program):
    return {
        SOLVE_SCORE_PROGRAM_META_NAME: str(program.get("name") or ""),
        SOLVE_SCORE_PROGRAM_META_STATEMENT_COUNT: str(int(program.get("statement_count") or 0)),
        SOLVE_SCORE_PROGRAM_META_SAVED_AT: str(program.get("saved_at") or ""),
    }


def _param_program_put_metadata(program):
    return {
        PARAM_PROGRAM_META_NAME: str(program.get("name") or ""),
        PARAM_PROGRAM_META_STATEMENT_COUNT: str(int(program.get("statement_count") or 0)),
        PARAM_PROGRAM_META_SAVED_AT: str(program.get("saved_at") or ""),
    }


def _coeff_program_put_metadata(program):
    return {
        COEFF_PROGRAM_META_NAME: str(program.get("name") or ""),
        COEFF_PROGRAM_META_STATEMENT_COUNT: str(int(program.get("statement_count") or 0)),
        COEFF_PROGRAM_META_SAVED_AT: str(program.get("saved_at") or ""),
    }


def _solve_score_program_summary_from_head(program_id):
    resp = s3.head_object(Bucket=BUCKET, Key=_solve_score_program_key(program_id))
    meta = resp.get("Metadata") or {}
    name = str(meta.get(SOLVE_SCORE_PROGRAM_META_NAME) or "").strip()
    saved_at = str(meta.get(SOLVE_SCORE_PROGRAM_META_SAVED_AT) or "").strip()
    statement_count_raw = str(meta.get(SOLVE_SCORE_PROGRAM_META_STATEMENT_COUNT) or "").strip()
    if not name or not saved_at or not statement_count_raw:
        raise RuntimeError(f"solve-score program summary metadata missing for {program_id}")
    try:
        statement_count = int(statement_count_raw)
    except (TypeError, ValueError) as exc:
        raise RuntimeError(
            f"solve-score program summary metadata invalid statement_count for {program_id}"
        ) from exc
    return {
        "id": str(program_id),
        "name": name,
        "statement_count": statement_count,
        "saved_at": saved_at,
    }


def _param_program_summary_from_head(program_id):
    resp = s3.head_object(Bucket=BUCKET, Key=_param_program_key(program_id))
    meta = resp.get("Metadata") or {}
    name = str(meta.get(PARAM_PROGRAM_META_NAME) or "").strip()
    saved_at = str(meta.get(PARAM_PROGRAM_META_SAVED_AT) or "").strip()
    statement_count_raw = str(meta.get(PARAM_PROGRAM_META_STATEMENT_COUNT) or "").strip()
    if not name or not saved_at or not statement_count_raw:
        raise RuntimeError(f"param program summary metadata missing for {program_id}")
    try:
        statement_count = int(statement_count_raw)
    except (TypeError, ValueError) as exc:
        raise RuntimeError(
            f"param program summary metadata invalid statement_count for {program_id}"
        ) from exc
    return {
        "id": str(program_id),
        "name": name,
        "statement_count": statement_count,
        "saved_at": saved_at,
    }


def _coeff_program_summary_from_head(program_id):
    resp = s3.head_object(Bucket=BUCKET, Key=_coeff_program_key(program_id))
    meta = resp.get("Metadata") or {}
    name = str(meta.get(COEFF_PROGRAM_META_NAME) or "").strip()
    saved_at = str(meta.get(COEFF_PROGRAM_META_SAVED_AT) or "").strip()
    statement_count_raw = str(meta.get(COEFF_PROGRAM_META_STATEMENT_COUNT) or "").strip()
    if not name or not saved_at or not statement_count_raw:
        raise RuntimeError(f"coeff program summary metadata missing for {program_id}")
    try:
        statement_count = int(statement_count_raw)
    except (TypeError, ValueError) as exc:
        raise RuntimeError(
            f"coeff program summary metadata invalid statement_count for {program_id}"
        ) from exc
    return {
        "id": str(program_id),
        "name": name,
        "statement_count": statement_count,
        "saved_at": saved_at,
    }


def _results_list_s3_client(max_workers):
    return boto3.client(
        "s3",
        config=Config(max_pool_connections=_results_list_pool_size(max_workers)),
    )


def _is_missing_s3_error(exc):
    # Canonical policy lives in shared.is_missing_s3_error (code-review-28 F13);
    # this module-private alias keeps existing call sites terse.
    return is_missing_s3_error(exc)


def _favorite_task_id(job_id, artifact_id):
    return f"{FAVORITES_DDB_TASK_PREFIX}{job_id}#{artifact_id}"


def _load_legacy_favorites():
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=FAVORITES_KEY)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            return []
        raise
    raw = obj["Body"].read()
    data = json.loads(raw) if raw else []
    if isinstance(data, list):
        return data
    if isinstance(data, dict) and isinstance(data.get("favorites"), list):
        return data["favorites"]
    return []


def _list_favorite_rows():
    ddb = _get_ddb()
    kwargs = {
        "TableName": JOBS_TABLE,
        "KeyConditionExpression": "job_id = :jid",
        "ExpressionAttributeValues": {
            ":jid": {"S": FAVORITES_DDB_JOB_ID},
        },
    }
    rows = []
    while True:
        resp = ddb.query(**kwargs)
        rows.extend(resp.get("Items", []))
        if "LastEvaluatedKey" not in resp:
            break
        kwargs["ExclusiveStartKey"] = resp["LastEvaluatedKey"]
    return rows


def _favorite_store_initialized():
    ddb = _get_ddb()
    resp = ddb.get_item(
        TableName=JOBS_TABLE,
        Key={
            "job_id": {"S": FAVORITES_DDB_JOB_ID},
            "task_id": {"S": FAVORITES_DDB_META_TASK_ID},
        },
        ProjectionExpression="job_id, task_id",
    )
    return "Item" in resp


def _favorite_from_row(row):
    entry = {
        "job_id": row["favorite_job_id"]["S"],
        "artifact_id": row["favorite_artifact_id"]["S"],
        "family": row.get("family", {}).get("S", "color"),
        "added_at": row.get("added_at", {}).get("S", ""),
    }
    if row.get("hydration_state", {}).get("S") == "missing":
        entry["hydration_state"] = "missing"   # persisted authoritative verdict (CR30 F7)
    inc = row.get("incarnation", {}).get("S")
    if inc:
        entry["incarnation"] = inc             # rides along for conditional hydration writes (CR30 follow-up F2)
    for field in ("display_name", "image_key", "preview_key"):
        value = row.get(field, {}).get("S")
        if value:
            entry[field] = value
    return entry


def _sort_favorites(items):
    return sorted(items, key=lambda item: item.get("added_at", ""), reverse=True)


def _put_favorite_meta():
    try:
        _get_ddb().put_item(
            TableName=JOBS_TABLE,
            Item={
                "job_id": {"S": FAVORITES_DDB_JOB_ID},
                "task_id": {"S": FAVORITES_DDB_META_TASK_ID},
                "family": {"S": "color"},
                "updated_at_ms": {"N": str(int(time.time() * 1000))},
            },
            ConditionExpression="attribute_not_exists(job_id) AND attribute_not_exists(task_id)",
        )
    except ClientError as exc:
        code = exc.response.get("Error", {}).get("Code")
        if code != "ConditionalCheckFailedException":
            raise


def _put_favorite_entry(entry, snapshot=None):
    item = {
        "job_id": {"S": FAVORITES_DDB_JOB_ID},
        "task_id": {"S": _favorite_task_id(entry["job_id"], entry["artifact_id"])},
        "favorite_job_id": {"S": entry["job_id"]},
        "favorite_artifact_id": {"S": entry["artifact_id"]},
        "family": {"S": "color"},
        "added_at": {"S": entry["added_at"]},
        "updated_at_ms": {"N": str(int(time.time() * 1000))},
        # Immutable per-incarnation token (CR30 follow-up F2): delete + re-add reuses the
        # same key, so hydration writes captured against the OLD incarnation
        # must fail conditionally instead of stamping the new row.
        "incarnation": {"S": uuid.uuid4().hex[:12]},
    }
    for field in ("display_name", "image_key", "preview_key"):
        value = entry.get(field)
        if value:
            item[field] = {"S": value}
    if snapshot is not None:
        item["snapshot_version"] = {"N": str(FAVORITE_SNAPSHOT_VERSION)}
        item["snapshot"] = {"S": json.dumps(snapshot, separators=(",", ":"))}
    try:
        _get_ddb().put_item(
            TableName=JOBS_TABLE,
            Item=item,
            ConditionExpression="attribute_not_exists(job_id) AND attribute_not_exists(task_id)",
        )
        return True
    except ClientError as exc:
        code = exc.response.get("Error", {}).get("Code")
        if code == "ConditionalCheckFailedException":
            return False
        raise


def _delete_favorite_entry(job_id, artifact_id):
    resp = _get_ddb().delete_item(
        TableName=JOBS_TABLE,
        Key={
            "job_id": {"S": FAVORITES_DDB_JOB_ID},
            "task_id": {"S": _favorite_task_id(job_id, artifact_id)},
        },
        ReturnValues="ALL_OLD",
    )
    return "Attributes" in resp


def _favorite_color_prefix(job_id, artifact_id):
    return f"renders/{job_id}/color/{artifact_id}/"


def _favorite_snapshot_from_entry(entry):
    """Project a resolved Color artifact entry to the compact display snapshot."""
    snap = {}
    for field in _FAVORITE_SNAPSHOT_FIELDS:
        value = entry.get(field)
        if value not in (None, ""):
            snap[field] = value
    return snap


def _resolve_favorite_color_snapshot(job_id, artifact_id, *, s3_client=None, parallel_heads=True):
    """Resolve a favorite's Color artifact by EXACT key: HEAD the known image /
    preview candidates and read that artifact's own overlay. No job scan, no
    /render-summary, no list_objects_v2 (favorites-speedup.md Proposal 3).

    Returns (snapshot|None, state, reason). state:
      'ready'   snapshot built from the resolved artifact;
      'missing' genuine 404 — the artifact is gone (mark the favorite missing);
      'error'   transient/throttle/access/overlay failure — the bytes may well
                exist, so the favorite must NOT be relabeled missing (CR28 F13)."""
    client = s3_client or s3
    shape = RENDER_FAMILY_SHAPES["color"]
    prefix = _favorite_color_prefix(job_id, artifact_id)
    image_candidates = [prefix + k for k in shape["image_candidates"]]
    preview_candidates = [prefix + k for k in shape["preview_candidates"]]
    try:
        head = _head_artifact_keys(image_candidates + preview_candidates,
                                   presign=False, s3_client=client, parallel=parallel_heads)
    except Exception as exc:
        return None, "error", s3_error_reason(exc)
    image_info = _first_existing(head, image_candidates)
    if not image_info:
        # Distinguish a real 404 from a transient/permission failure using the
        # per-key error_reason _head_artifact_keys now surfaces (CR28 F13).
        for key in image_candidates:
            reason = (head.get(key) or {}).get("error_reason")
            if reason:
                return None, "error", reason
        return None, "missing", "not found"
    # Identity guard: the key we built must name this exact artifact.
    assert_render_identity(image_info["key"], job_id, artifact_id, "favorite image_key")
    preview_info = _first_existing(head, preview_candidates)
    try:
        overlay = _load_color_artifact_overlay(job_id, artifact_id, s3_client=client)
    except Exception as exc:
        return None, "error", s3_error_reason(exc)
    entry = _render_artifact_entry("color", artifact_id, image_info, preview_info,
                                   fallback_meta=overlay)
    return _favorite_snapshot_from_entry(entry), "ready", ""


def _favorite_panel_row(ref, snapshot, state, reason):
    """Build the panel-ready favorite row the frontend renders directly (no
    client-side hydration). URLs are derived client-side from the keys."""
    row = {
        "family": "color",
        "artifact_id": ref["artifact_id"],
        "favorite_job_id": ref["job_id"],
        "favorite_artifact_id": ref["artifact_id"],
        "favorite_added_at": ref.get("added_at", ""),
        "display_name": ref.get("display_name") or ref["artifact_id"],
        "image_key": ref.get("image_key", ""),
        "preview_key": ref.get("preview_key", ""),
        "hydration_state": state,
        "missing": state == "missing",
    }
    if snapshot:
        row.update(snapshot)
        row["image_key"] = snapshot.get("image_key") or row["image_key"]
        row["preview_key"] = snapshot.get("preview_key") or row["preview_key"]
    if state == "missing":
        row["missing_reason"] = reason or "not found"
    elif state == "error":
        # NOT missing: keep the row displayable from its stored keys; flag stale.
        row["error_reason"] = reason or "error"
    return row


def _favorite_incarnation_condition(incarnation):
    """Conditional guard for hydration writes (CR30 follow-up F2): the row must still be
    the SAME incarnation the resolution was computed against. Legacy rows
    predate the token — for those, require it to still be absent (a re-add
    stamps one, which correctly fails the stale write)."""
    if incarnation:
        return ("attribute_exists(task_id) AND incarnation = :inc",
                {":inc": {"S": incarnation}})
    return ("attribute_exists(task_id) AND attribute_not_exists(incarnation)", {})


def _backfill_favorite_snapshot(job_id, artifact_id, snapshot, incarnation=None):
    """Persist a resolved snapshot onto an existing favorite row so the next
    list is zero-S3 for it. Conditioned on the SAME row incarnation (CR30 follow-up F2).
    Also clears any persisted missing marker — the artifact evidently exists."""
    cond, extra = _favorite_incarnation_condition(incarnation)
    _get_ddb().update_item(
        TableName=JOBS_TABLE,
        Key={
            "job_id": {"S": FAVORITES_DDB_JOB_ID},
            "task_id": {"S": _favorite_task_id(job_id, artifact_id)},
        },
        UpdateExpression="SET snapshot_version = :v, snapshot = :s, updated_at_ms = :u"
                         " REMOVE hydration_state, missing_at_ms",
        ExpressionAttributeValues={
            ":v": {"N": str(FAVORITE_SNAPSHOT_VERSION)},
            ":s": {"S": json.dumps(snapshot, separators=(",", ":"))},
            ":u": {"N": str(int(time.time() * 1000))},
            **extra,
        },
        ConditionExpression=cond,
    )


def _persist_favorite_missing(job_id, artifact_id, incarnation=None):
    """Persist an AUTHORITATIVE missing verdict (code-review-30 F7): drop the
    stale snapshot and mark the row, so the next zero-S3 cached list shows the
    favorite as missing instead of resurrecting the old snapshot. The row stays
    (product rule: a stale favorite remains visible until the user removes it);
    a later forced refresh that finds the artifact again heals it via the
    backfill. Transient/throttle/access errors must never reach here."""
    cond, extra = _favorite_incarnation_condition(incarnation)
    _get_ddb().update_item(
        TableName=JOBS_TABLE,
        Key={
            "job_id": {"S": FAVORITES_DDB_JOB_ID},
            "task_id": {"S": _favorite_task_id(job_id, artifact_id)},
        },
        UpdateExpression="SET hydration_state = :m, missing_at_ms = :t, updated_at_ms = :t"
                         " REMOVE snapshot, snapshot_version",
        ExpressionAttributeValues={
            ":m": {"S": "missing"},
            ":t": {"N": str(int(time.time() * 1000))},
            **extra,
        },
        ConditionExpression=cond,
    )


def _ensure_favorites_store_ready():
    if _favorite_store_initialized():
        return
    legacy = _load_legacy_favorites()
    _put_favorite_meta()
    for entry in legacy:
        if entry.get("job_id") and entry.get("artifact_id"):
            _put_favorite_entry({
                "job_id": entry["job_id"],
                "artifact_id": entry["artifact_id"],
                "family": "color",
                "added_at": entry.get("added_at") or time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                "display_name": entry.get("display_name"),
                "image_key": entry.get("image_key"),
                "preview_key": entry.get("preview_key"),
            })


def handler(event, context):
    if event.get("internal_action") == "describe_gallery":
        return _run_describe_gallery_worker(event.get("params") or {})
    if event.get("internal_action") == "build_color_mosaic":
        refresh_id = str(event.get("refresh_id") or "").strip()
        if not refresh_id:
            return ok_response({"error": "missing refresh_id"})
        return ok_response(_run_mosaic_worker("color", refresh_id))
    if event.get("internal_action") == "build_palette_mosaic":
        refresh_id = str(event.get("refresh_id") or "").strip()
        if not refresh_id:
            return ok_response({"error": "missing refresh_id"})
        return ok_response(_run_mosaic_worker("palette", refresh_id))

    path = event.get("rawPath", event.get("path", "/"))
    if path.endswith("/list"):
        return _handle_storage_route(handle_list, event)
    elif path.endswith("/list-solve-score-programs"):
        return _handle_storage_route(handle_list_solve_score_programs, event)
    elif path.endswith("/fetch-solve-score-program"):
        return _handle_storage_route(handle_fetch_solve_score_program, event)
    elif path.endswith("/save-solve-score-program"):
        return _handle_storage_route(handle_save_solve_score_program, event)
    elif path.endswith("/delete-solve-score-program"):
        return _handle_storage_route(handle_delete_solve_score_program, event)
    elif path.endswith("/migrate-solve-score-program"):
        return _handle_storage_route(lambda ev: _handle_migrate_program(ev, "solve-score"), event)
    elif path.endswith("/compile-solve-score-program-source"):
        return _handle_storage_route(handle_compile_solve_score_program_source, event)
    elif path.endswith("/solve-score-chain-to-source"):
        return _handle_storage_route(handle_solve_score_chain_to_source, event)
    elif path.endswith("/compile-root-program-source"):
        return _handle_storage_route(handle_compile_root_program_source, event)
    elif path.endswith("/save-root-program"):
        return _handle_storage_route(handle_save_root_program, event)
    elif path.endswith("/list-root-programs"):
        return _handle_storage_route(handle_list_root_programs, event)
    elif path.endswith("/fetch-root-program"):
        return _handle_storage_route(handle_fetch_root_program, event)
    elif path.endswith("/list-books"):
        return _handle_storage_route(handle_list_books, event)
    elif path.endswith("/fetch-book"):
        return _handle_storage_route(handle_fetch_book, event)
    elif path.endswith("/save-book"):
        return _handle_storage_route(handle_save_book, event)
    elif path.endswith("/snapshot-book-cover"):
        return _handle_storage_route(handle_snapshot_book_cover, event)
    elif path.endswith("/delete-book"):
        return _handle_storage_route(handle_delete_book, event)
    elif path.endswith("/fetch-vision-config"):
        return _handle_storage_route(handle_fetch_vision_config, event)
    elif path.endswith("/save-vision-config"):
        return _handle_storage_route(handle_save_vision_config, event)
    elif path.endswith("/delete-root-program"):
        return _handle_storage_route(handle_delete_root_program, event)
    elif path.endswith("/list-param-programs"):
        return _handle_storage_route(handle_list_param_programs, event)
    elif path.endswith("/fetch-param-program"):
        return _handle_storage_route(handle_fetch_param_program, event)
    elif path.endswith("/save-param-program"):
        return _handle_storage_route(handle_save_param_program, event)
    elif path.endswith("/compile-param-program-source"):
        return _handle_storage_route(handle_compile_param_program_source, event)
    elif path.endswith("/delete-param-program"):
        return _handle_storage_route(handle_delete_param_program, event)
    elif path.endswith("/migrate-param-program"):
        return _handle_storage_route(lambda ev: _handle_migrate_program(ev, "param"), event)
    elif path.endswith("/list-coeff-programs"):
        return _handle_storage_route(handle_list_coeff_programs, event)
    elif path.endswith("/fetch-coeff-program"):
        return _handle_storage_route(handle_fetch_coeff_program, event)
    elif path.endswith("/save-coeff-program"):
        return _handle_storage_route(handle_save_coeff_program, event)
    elif path.endswith("/compile-coeff-program-source"):
        return _handle_storage_route(handle_compile_coeff_program_source, event)
    elif path.endswith("/delete-coeff-program"):
        return _handle_storage_route(handle_delete_coeff_program, event)
    elif path.endswith("/migrate-coeff-program"):
        return _handle_storage_route(lambda ev: _handle_migrate_program(ev, "coeff"), event)
    elif path.endswith("/migrate-compute"):
        return _handle_storage_route(_handle_migrate_compute, event)
    elif path.endswith("/list-favorites"):
        return _handle_storage_route(handle_list_favorites, event)
    elif path.endswith("/add-favorite"):
        return _handle_storage_route(handle_add_favorite, event)
    elif path.endswith("/delete-favorite"):
        return _handle_storage_route(handle_delete_favorite, event)
    elif path.endswith("/list-custom-palettes"):
        return _handle_storage_route(handle_list_custom_palettes, event)
    elif path.endswith("/save-custom-palettes"):
        return _handle_storage_route(handle_save_custom_palettes, event)
    elif path.endswith("/list-palettes"):
        return _handle_storage_route(handle_list_palettes, event)
    elif path.endswith("/delete-palette"):
        return _handle_storage_route(handle_delete_palette, event)
    elif path.endswith("/delete-render-artifact"):
        return _handle_storage_route(handle_delete_render_artifact, event)
    elif path.endswith("/delete"):
        return _handle_storage_route(handle_delete, event)
    elif path.endswith("/save-metadata"):
        return _handle_storage_route(handle_save_metadata, event)
    elif path.endswith("/cleanup"):
        return _handle_storage_route(handle_cleanup, event)
    elif path.endswith("/clean-render"):
        return _handle_storage_route(handle_clean_render, event)
    elif path.endswith("/check-keys"):
        return _handle_storage_route(handle_check_keys, event)
    elif path.endswith("/check-status"):
        return _handle_storage_route(handle_check_status, event)
    elif path.endswith("/detail"):
        return _handle_storage_route(handle_detail, event)
    elif path.endswith("/presign"):
        return _handle_storage_route(handle_presign, event)
    elif path.endswith("/list-prefix"):
        return _handle_storage_route(handle_list_prefix, event)
    elif path.endswith("/head-keys"):
        return _handle_storage_route(handle_head_keys, event)
    elif path.endswith("/render-summary"):
        return _handle_storage_route(handle_render_summary, event)
    elif path.endswith("/render-count"):
        return _handle_storage_route(handle_render_count, event)
    elif path.endswith("/list-color-mosaic"):
        return _handle_storage_route(handle_list_color_mosaic, event)
    elif path.endswith("/list-palette-mosaic"):
        return _handle_storage_route(handle_list_palette_mosaic, event)
    elif path.endswith("/share-mosaic"):
        return _handle_storage_route(handle_share_mosaic, event)
    elif path.endswith("/create-gallery-share"):
        return _handle_storage_route(handle_create_gallery_share, event)
    elif path.endswith("/describe-gallery"):
        return _handle_storage_route(handle_describe_gallery, event)
    elif path.endswith("/create-gallery"):
        return _handle_storage_route(handle_create_gallery, event)
    elif path.endswith("/list-galleries"):
        return _handle_storage_route(handle_list_galleries, event)
    elif path.endswith("/fetch-gallery"):
        return _handle_storage_route(handle_fetch_gallery, event)
    elif path.endswith("/save-gallery"):
        return _handle_storage_route(handle_save_gallery, event)
    elif path.endswith("/add-to-gallery"):
        return _handle_storage_route(handle_add_to_gallery, event)
    elif path.endswith("/delete-gallery"):
        return _handle_storage_route(handle_delete_gallery, event)
    elif path.endswith("/delete-task"):
        return _handle_storage_route(handle_delete_task, event)
    elif path.endswith("/delete-prefix"):
        return _handle_storage_route(handle_delete_prefix, event)
    elif path.endswith("/list-sheets"):
        return _handle_storage_route(handle_list_sheets, event)
    elif path.endswith("/list-deepzoom"):
        return _handle_storage_route(handle_list_deepzoom, event)
    return {
        "statusCode": 400,
        "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
        "body": json.dumps({"error": f"Unknown route: {path}"}),
    }


def _favorite_refs_from_rows(rows):
    """Extract favorite refs + parsed snapshots from raw DDB rows. Returns
    (refs_sorted_desc, {(job,artifact): snapshot}, saw_meta)."""
    refs = []
    snapshots = {}
    saw_meta = False
    for row in rows:
        task_id = row.get("task_id", {}).get("S")
        if task_id == FAVORITES_DDB_META_TASK_ID:
            saw_meta = True
            continue
        if not task_id or not task_id.startswith(FAVORITES_DDB_TASK_PREFIX):
            continue
        ref = _favorite_from_row(row)
        refs.append(ref)
        version = _parse_int((row.get("snapshot_version") or {}).get("N"))
        snap_raw = (row.get("snapshot") or {}).get("S")
        if version == FAVORITE_SNAPSHOT_VERSION and snap_raw:
            try:
                snapshots[(ref["job_id"], ref["artifact_id"])] = json.loads(snap_raw)
            except (ValueError, TypeError):
                pass
    return _sort_favorites(refs), snapshots, saw_meta


def handle_list_favorites(event):
    """Panel-ready favorites in one DDB query (favorites-speedup.md Proposal 3).

    Rows with a current snapshot return with zero S3 work; legacy/incomplete
    rows are resolved by EXACT key (no /render-summary, no list_objects_v2) and
    backfilled. `{"refresh": true}` re-resolves every row. A transient S3 error
    is surfaced as `error`, never as a missing artifact (CR28 F13)."""
    params = parse_body(event) if event else {}
    refresh = parse_boolish(params.get("refresh"), False)
    t0 = time.time()

    # Single round trip on the initialized path: query once, and only run the
    # legacy migration + re-query if the __meta__ marker is absent (Proposal 5).
    rows = _list_favorite_rows()
    refs, snapshots, saw_meta = _favorite_refs_from_rows(rows)
    if not saw_meta:
        _ensure_favorites_store_ready()
        rows = _list_favorite_rows()
        refs, snapshots, _ = _favorite_refs_from_rows(rows)

    to_resolve = [r for r in refs
                  if refresh or ((r["job_id"], r["artifact_id"]) not in snapshots
                                 and r.get("hydration_state") != "missing")]
    incarnation_by_key = {(r["job_id"], r["artifact_id"]): r.get("incarnation") for r in refs}
    resolved = {}
    hydration_pool = None
    if to_resolve:
        import concurrent.futures
        client = _results_list_s3_client(len(to_resolve))

        def work(ref):
            # serial HEADs inside this bounded pool — ONE executor total (CR30 F11)
            return (ref["job_id"], ref["artifact_id"]), _resolve_favorite_color_snapshot(
                ref["job_id"], ref["artifact_id"], s3_client=client, parallel_heads=False)

        # kept open: the persistence phase below reuses this ONE pool (CR30 follow-up F9)
        hydration_pool = concurrent.futures.ThreadPoolExecutor(
            max_workers=min(len(to_resolve), 16))
        for keypair, result in hydration_pool.map(work, to_resolve):
            resolved[keypair] = result

    panel = []
    diag = {"snapshot_hits": 0, "snapshot_backfills": 0, "missing": 0, "errors": 0}
    backfills = []
    fresh_missing = []
    for ref in refs:
        keypair = (ref["job_id"], ref["artifact_id"])
        if keypair in resolved:
            snap, state, reason = resolved[keypair]
            panel.append(_favorite_panel_row(ref, snap, state, reason))
            if state == "ready" and snap is not None:
                backfills.append((ref["job_id"], ref["artifact_id"], snap))
                diag["snapshot_backfills"] += 1
            elif state == "missing":
                fresh_missing.append(keypair)
                diag["missing"] += 1
            elif state == "error":
                diag["errors"] += 1
        elif keypair in snapshots:
            panel.append(_favorite_panel_row(ref, snapshots[keypair], "ready", ""))
            diag["snapshot_hits"] += 1
        else:
            # persisted missing verdict: shown WITHOUT any S3 probing (CR30 F7)
            panel.append(_favorite_panel_row(ref, None, "missing", "not found"))
            diag["missing"] += 1

    # Persistence phase: parallel in the SAME pool, failures COUNTED — a
    # silently dropped missing-marker write re-enables the resurrection CR30 F7
    # closed, so the response must say when persistence didn't stick (CR30 follow-up F9).
    def _persist_one(task):
        kind, job_id, artifact_id, payload = task
        try:
            inc = incarnation_by_key.get((job_id, artifact_id))
            if kind == "backfill":
                _backfill_favorite_snapshot(job_id, artifact_id, payload, incarnation=inc)
            else:
                _persist_favorite_missing(job_id, artifact_id, incarnation=inc)
            return True
        except Exception:
            return False   # best-effort, but observable

    write_tasks = [("backfill", j, a, snap) for j, a, snap in backfills]
    write_tasks += [("missing", j, a, None) for j, a in fresh_missing]
    diag["hydration_write_failures"] = 0
    if write_tasks:
        if hydration_pool is not None:
            outcomes = list(hydration_pool.map(_persist_one, write_tasks))
        else:
            outcomes = [_persist_one(t) for t in write_tasks]
        diag["hydration_write_failures"] = sum(1 for ok in outcomes if not ok)
    if hydration_pool is not None:
        hydration_pool.shutdown(wait=True)

    return ok_response({
        "favorites": panel,
        "count": len(panel),
        "snapshot_hits": diag["snapshot_hits"],
        "snapshot_backfills": diag["snapshot_backfills"],
        "missing": diag["missing"],
        "errors": diag["errors"],
        "hydration_write_failures": diag.get("hydration_write_failures", 0),
        "hydrate_us": int((time.time() - t0) * 1e6),
    })


def handle_add_favorite(event):
    """Resolve + validate the artifact server-side by EXACT key, store canonical
    keys + a compact display snapshot, and return the single affected row
    (favorites-speedup.md Proposals 3 & 4). No full-partition reread."""
    params = parse_body(event)
    job_id = assert_safe_id(params["job_id"], "job_id")
    artifact_id = assert_safe_id(params["artifact_id"], "artifact_id")
    family = params.get("family", "color")
    if family != "color":
        raise ValueError("Only color favorites are supported")
    _ensure_favorites_store_ready()

    # Do NOT trust caller-provided image/preview keys: resolve them ourselves.
    snapshot, state, reason = _resolve_favorite_color_snapshot(job_id, artifact_id)
    if state == "missing":
        raise ValueError(f"color artifact {artifact_id} not found under job {job_id}")
    if state == "error":
        raise RuntimeError(f"could not resolve color artifact {artifact_id}: {reason}")

    entry = {
        "job_id": job_id,
        "artifact_id": artifact_id,
        "family": "color",
        "added_at": params.get("added_at") or time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "display_name": str(params.get("display_name") or artifact_id),
        "image_key": snapshot.get("image_key", ""),
        "preview_key": snapshot.get("preview_key", ""),
    }
    added = _put_favorite_entry(entry, snapshot=snapshot)
    if not added:
        # Already favorited: return the STORED row (original added_at and
        # display_name), never a freshly fabricated one — the frontend would
        # otherwise reorder/rename an existing favorite (code-review-30 F8).
        # ConsistentRead (CR30 follow-up F4): the conflicting put proves the row exists;
        # an eventually-consistent read can miss a row written moments ago
        # (double-click) and fall through to the fabricated shape.
        resp = _get_ddb().get_item(
            TableName=JOBS_TABLE,
            Key={"job_id": {"S": FAVORITES_DDB_JOB_ID},
                 "task_id": {"S": _favorite_task_id(job_id, artifact_id)}},
            ConsistentRead=True)
        row = resp.get("Item")
        if row:
            stored_ref = _favorite_from_row(row)
            stored_snap = None
            snap_raw = (row.get("snapshot") or {}).get("S")
            if _parse_int((row.get("snapshot_version") or {}).get("N")) == FAVORITE_SNAPSHOT_VERSION and snap_raw:
                try:
                    stored_snap = json.loads(snap_raw)
                except (ValueError, TypeError):
                    stored_snap = None
            if stored_ref.get("hydration_state") == "missing":
                # The re-add just PROVED the artifact exists — persist the
                # healing so the next cached list doesn't say missing (CR30 follow-up F4).
                try:
                    _backfill_favorite_snapshot(job_id, artifact_id, snapshot,
                                                incarnation=stored_ref.get("incarnation"))
                except Exception:
                    pass
                return ok_response({"added": False,
                                    "favorite": _favorite_panel_row(stored_ref, snapshot, "ready", "")})
            return ok_response({"added": False,
                                "favorite": _favorite_panel_row(stored_ref, stored_snap or snapshot, "ready", "")})
    return ok_response({"added": added, "favorite": _favorite_panel_row(entry, snapshot, "ready", "")})


def handle_delete_favorite(event):
    params = parse_body(event)
    job_id = params["job_id"]
    artifact_id = params["artifact_id"]
    _ensure_favorites_store_ready()
    deleted = _delete_favorite_entry(job_id, artifact_id)
    return ok_response({"deleted": deleted, "job_id": job_id, "artifact_id": artifact_id})


def _normalize_custom_palette_entries(raw_entries):
    if not isinstance(raw_entries, list):
        raise ValueError("custom palettes must be a list")
    if len(raw_entries) > CUSTOM_PALETTE_MAX_ENTRIES:
        raise ValueError(
            f"custom palette catalog supports at most {CUSTOM_PALETTE_MAX_ENTRIES} entries"
        )

    entries = []
    seen_names = set()
    seen_palettes = set()
    for idx, raw in enumerate(raw_entries):
        if not isinstance(raw, dict):
            raise ValueError(f"custom palette row {idx + 1} must be an object")
        name = str(raw.get("name") or "").strip()
        if not name:
            raise ValueError(f"custom palette row {idx + 1} requires a name")
        if len(name) > CUSTOM_PALETTE_MAX_NAME_LEN:
            raise ValueError(
                f"custom palette name must be at most {CUSTOM_PALETTE_MAX_NAME_LEN} characters"
            )
        if any(not ch.isprintable() for ch in name):
            raise ValueError("custom palette names must be printable single-line text")

        raw_stops = raw.get("stops")
        if not isinstance(raw_stops, list):
            raise ValueError(f"custom palette {name!r} stops must be a list")
        if not (CUSTOM_PALETTE_MIN_STOPS <= len(raw_stops) <= CUSTOM_PALETTE_MAX_STOPS):
            raise ValueError(
                f"custom palette {name!r} requires {CUSTOM_PALETTE_MIN_STOPS}.."
                f"{CUSTOM_PALETTE_MAX_STOPS} colors"
            )
        stops = []
        for stop in raw_stops:
            value = str(stop or "").strip()
            if value.startswith("#"):
                value = value[1:]
            if not _CUSTOM_PALETTE_HEX_RE.fullmatch(value):
                raise ValueError(
                    f"custom palette {name!r} contains invalid color {stop!r}"
                )
            stops.append(value.lower())

        name_key = name.casefold()
        palette = "custom:" + "-".join(stops)
        if name_key in seen_names:
            raise ValueError(f"duplicate custom palette name: {name!r}")
        if palette in seen_palettes:
            raise ValueError(
                f"custom palette {name!r} duplicates another row's colors"
            )
        seen_names.add(name_key)
        seen_palettes.add(palette)
        entries.append({"name": name, "stops": stops, "palette": palette})
    return entries


def _custom_palette_catalog_response(doc, revision):
    return {
        "schema_version": CUSTOM_PALETTE_CATALOG_SCHEMA_VERSION,
        "updated_at": str((doc or {}).get("updated_at") or ""),
        "palettes": list((doc or {}).get("palettes") or []),
        "revision": str(revision or ""),
    }


def _read_custom_palette_catalog():
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=CUSTOM_PALETTE_CATALOG_KEY)
    except Exception as exc:
        if is_missing_s3_error(exc):
            return {
                "schema_version": CUSTOM_PALETTE_CATALOG_SCHEMA_VERSION,
                "updated_at": "",
                "palettes": [],
            }, ""
        raise
    try:
        doc = json.loads(obj["Body"].read())
        if not isinstance(doc, dict):
            raise ValueError("catalog root is not an object")
        if int(doc.get("schema_version") or 0) != CUSTOM_PALETTE_CATALOG_SCHEMA_VERSION:
            raise ValueError(
                f"unsupported schema_version {doc.get('schema_version')!r}"
            )
        doc["palettes"] = _normalize_custom_palette_entries(doc.get("palettes"))
    except (ValueError, TypeError, json.JSONDecodeError) as exc:
        raise CustomPaletteCatalogReadError(
            f"stored custom palette catalog is invalid: {exc}"
        ) from exc
    return doc, str(obj.get("ETag") or "").strip('"')


def handle_list_custom_palettes(event):
    parse_body(event)
    doc, revision = _read_custom_palette_catalog()
    return ok_response(_custom_palette_catalog_response(doc, revision))


def handle_save_custom_palettes(event):
    params = parse_body(event)
    entries = _normalize_custom_palette_entries(params.get("palettes"))
    expected_revision = str(params.get("expected_revision") or "").strip().strip('"')
    doc = {
        "schema_version": CUSTOM_PALETTE_CATALOG_SCHEMA_VERSION,
        "updated_at": _utc_now_iso(),
        "palettes": entries,
    }
    put_kwargs = {
        "Bucket": BUCKET,
        "Key": CUSTOM_PALETTE_CATALOG_KEY,
        "Body": json.dumps(
            doc, separators=(",", ":"), ensure_ascii=False
        ).encode("utf-8"),
        "ContentType": "application/json",
    }
    if expected_revision:
        put_kwargs["IfMatch"] = expected_revision
    else:
        put_kwargs["IfNoneMatch"] = "*"
    try:
        result = s3.put_object(**put_kwargs)
    except ClientError as exc:
        code = str((exc.response.get("Error") or {}).get("Code") or "")
        if code in ("PreconditionFailed", "412", "ConditionalRequestConflict"):
            raise CustomPaletteCatalogConflictError(
                "custom palette catalog changed; close and reopen the popup before saving"
            ) from exc
        raise
    revision = str((result or {}).get("ETag") or "").strip('"')
    if not revision:
        head = s3.head_object(Bucket=BUCKET, Key=CUSTOM_PALETTE_CATALOG_KEY)
        revision = str(head.get("ETag") or "").strip('"')
    if not revision:
        raise RuntimeError("custom palette save did not return an object revision")
    return ok_response(_custom_palette_catalog_response(doc, revision))


def handle_list_solve_score_programs(event):
    parse_body(event)
    programs = []
    errors = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=SOLVE_SCORE_PROGRAMS_PREFIX):
        for obj in page.get("Contents", []):
            key = obj.get("Key", "")
            if not key.endswith(".json") or key.endswith("/"):
                continue
            program_id = key[len(SOLVE_SCORE_PROGRAMS_PREFIX):-5]
            if not program_id or "/" in program_id:
                continue
            try:
                program = _solve_score_program_summary_from_head(program_id)
            except _SolveScoreProgramNotFound:
                continue
            except Exception as exc:
                try:
                    program = _read_solve_score_program_object(program_id)
                except _SolveScoreProgramNotFound:
                    continue
                except Exception as read_exc:
                    err_text = str(read_exc)
                    print(f"solve-score program list skipped {key}: {err_text}")
                    errors.append({
                        "id": program_id,
                        "error": err_text[:240],
                    })
                    continue
                else:
                    print(
                        f"solve-score program list used full read fallback for {key}: "
                        f"{type(exc).__name__}: {exc}"
                    )
            programs.append(program)
    # Tie-break id ascending for identical timestamps.
    programs = sorted(programs, key=lambda row: row["id"])
    programs = sorted(programs, key=lambda row: row.get("saved_at") or "", reverse=True)
    return ok_response({
        "programs": programs,
        "count": len(programs),
        "order": "saved_at_desc",
        "errors": errors,
        "error_count": len(errors),
    })


def handle_fetch_solve_score_program(event):
    params = parse_body(event)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError("solve-score program fetch requires id")
    return ok_response({"program": _read_solve_score_program_object(program_id, prefer_v2=True)})


def handle_save_solve_score_program(event):
    params = parse_body(event)
    program = _compile_solve_score_program_payload(
        params.get("name"),
        params.get("chain"),
        source_text=params.get("source_text") or params.get("solve_score_program_source_text"),
        recommended_interpretation=params.get("recommended_interpretation"),
    )
    key = _solve_score_program_key(program["id"])
    overwritten = _key_exists(key)
    _put_program_v1_object(
        key, _solve_score_program_v2_key(program["id"]),
        (json.dumps(program, indent=2) + "\n").encode("utf-8"),
        metadata=_solve_score_program_put_metadata(program),
    )
    return ok_response({"program": program, "overwritten": overwritten})


def handle_delete_solve_score_program(event):
    params = parse_body(event)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError("solve-score program delete requires id")
    deleted = _delete_program_keys(
        _solve_score_program_key(program_id), _solve_score_program_v2_key(program_id)
    )
    if not deleted:
        raise _SolveScoreProgramNotFound(f"solve-score program not found: {program_id}")
    return ok_response({"id": program_id, "deleted": deleted})


def handle_compile_solve_score_program_source(event):
    params = parse_body(event)
    source_text = str(params.get("source_text") or params.get("solve_score_program_source_text") or "")
    strict = parse_boolish(params.get("strict"), False, strict=False, label="strict")
    compiled = compile_solve_score_program_source(source_text, strict=strict)
    ok = not compiled.get("diagnostics")
    program = {
        "program_kind": "solve_score_program",
        "spec_version": compiled["spec_version"],
        "source_text": compiled["source_text"],
        "source_display": compiled["source_display"],
        "chain": compiled["chain"],
        "display": compiled["display"],
        "program_spec": compiled["program_spec"],
        "fingerprint": compiled["fingerprint"],
        "output_channel_count": compiled["output_channel_count"],
        "output_channels": compiled["output_channels"],
        "has_explicit_outputs": compiled["has_explicit_outputs"],
        "statement_count": compiled["statement_count"],
        "metric_count": compiled["metric_count"],
    }
    return ok_response({
        "ok": ok,
        "chain": compiled["chain"],
        "display": compiled["display"],
        "program_spec": compiled["program_spec"],
        "fingerprint": compiled["fingerprint"],
        "output_channel_count": compiled["output_channel_count"],
        "output_channels": compiled["output_channels"],
        "statement_count": compiled["statement_count"],
        "diagnostics": compiled.get("diagnostics") or [],
        "program": program,
    })


def _solve_score_chain_to_source_payload(raw_chain):
    if not isinstance(raw_chain, list) or not raw_chain:
        raise ValueError("solve-score chain-to-source requires non-empty chain")
    source_text = solve_score_source_text_from_chain(raw_chain)
    compiled = compile_solve_score_chain_or_legacy(
        raw_chain,
        "",
        default_metric="proximity",
    )
    canonical_chain = json.loads(serialize_solve_score_chain(compiled["chain"]))
    return {
        "ok": True,
        "source_text": source_text,
        "chain": canonical_chain,
        "fingerprint": compiled_solve_score_fingerprint(compiled),
        "program_spec": compiled["program_spec"],
        "output_channel_count": compiled.get("output_channel_count", 1),
        "output_channels": list(compiled.get("output_channels") or []),
        "has_explicit_outputs": bool(compiled.get("has_explicit_outputs")),
        "display": compiled.get("display", ""),
    }


def handle_solve_score_chain_to_source(event):
    params = parse_body(event)
    raw_chain = params.get("chain")
    if raw_chain is None:
        raw_chain = params.get("solve_score_chain")
    try:
        return ok_response(_solve_score_chain_to_source_payload(raw_chain))
    except SolveScoreProgramSourceError as exc:
        return _json_error_response(400, {
            "ok": False,
            "error": str(exc),
            "code": getattr(exc, "code", "source_error"),
            "line": getattr(exc, "line", 0),
            "column": getattr(exc, "column", 0),
        })
    except (ValueError, TypeError, RuntimeError, json.JSONDecodeError) as exc:
        return _json_error_response(400, {
            "ok": False,
            "error": str(exc),
            "code": getattr(exc, "code", "invalid_chain"),
        })


def handle_list_param_programs(event):
    parse_body(event)
    programs = []
    errors = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=PARAM_PROGRAMS_PREFIX):
        for obj in page.get("Contents", []):
            key = obj.get("Key", "")
            if not key.endswith(".json") or key.endswith("/"):
                continue
            program_id = key[len(PARAM_PROGRAMS_PREFIX):-5]
            if not program_id or "/" in program_id:
                continue
            try:
                program = _param_program_summary_from_head(program_id)
            except _ParamProgramNotFound:
                continue
            except Exception as exc:
                try:
                    program = _read_param_program_object(program_id)
                except _ParamProgramNotFound:
                    continue
                except Exception as read_exc:
                    err_text = str(read_exc)
                    print(f"param program list skipped {key}: {err_text}")
                    errors.append({
                        "id": program_id,
                        "error": err_text[:240],
                    })
                    continue
                else:
                    print(
                        f"param program list used full read fallback for {key}: "
                        f"{type(exc).__name__}: {exc}"
                    )
            programs.append(program)
    programs = sorted(programs, key=lambda row: row["id"])
    programs = sorted(programs, key=lambda row: row.get("saved_at") or "", reverse=True)
    return ok_response({
        "programs": programs,
        "count": len(programs),
        "order": "saved_at_desc",
        "errors": errors,
        "error_count": len(errors),
    })


def handle_fetch_param_program(event):
    params = parse_body(event)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError("param program fetch requires id")
    return ok_response({"program": _read_param_program_object(program_id, prefer_v2=True)})


def handle_save_param_program(event):
    params = parse_body(event)
    program = _compile_param_program_payload(
        params.get("name"),
        params.get("chain"),
        source_text=param_source_text_from_payload(params),
    )
    key = _param_program_key(program["id"])
    overwritten = _key_exists(key)
    _put_program_v1_object(
        key, _param_program_v2_key(program["id"]),
        (json.dumps(program, indent=2) + "\n").encode("utf-8"),
        metadata=_param_program_put_metadata(program),
    )
    return ok_response({"program": program, "overwritten": overwritten})


def handle_delete_param_program(event):
    params = parse_body(event)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError("param program delete requires id")
    deleted = _delete_program_keys(
        _param_program_key(program_id), _param_program_v2_key(program_id)
    )
    if not deleted:
        raise _ParamProgramNotFound(f"param program not found: {program_id}")
    return ok_response({"id": program_id, "deleted": deleted})


def handle_compile_param_program_source(event):
    params = parse_body(event)
    source_text = str(params.get("source_text") or "")
    compiled = compile_param_program_source(
        source_text,
        macro_resolver=_param_program_macro_resolver(None),
        strict=False,
    )
    diagnostics = list(compiled.get("diagnostics") or [])
    has_errors = any(d.get("level") == "error" for d in diagnostics)
    chain_out = [] if has_errors else (compiled.get("chain") or [])
    fingerprint = "" if has_errors else (compiled.get("fingerprint") or "")
    display = compiled.get("source_display") or compiled.get("display") or ""
    return ok_response({
        "ok": not has_errors,
        "chain": chain_out,
        "display": display,
        "statement_count": compiled.get("source_statement_count") or 0,
        "fingerprint": fingerprint,
        "diagnostics": diagnostics,
        "program": {
            "chain": chain_out,
            "display": display,
            "fingerprint": fingerprint,
            "execution_spec": "" if has_errors else (compiled.get("execution_spec") or ""),
            "token_count": 0 if has_errors else (compiled.get("token_count") or 0),
            "stack_max": 0 if has_errors else (compiled.get("stack_max") or 0),
        },
    })


def handle_compile_root_program_source(event):
    params = parse_body(event)
    source_text = str(params.get("source_text") or "")
    compiled = compile_root_program_source(source_text, strict=False)
    diagnostics = list(compiled.get("diagnostics") or [])
    has_errors = any(d.get("level") == "error" for d in diagnostics)
    chain_out = [] if has_errors else compiled.get("chain", [])
    root_transforms = [] if has_errors else compiled.get("root_transforms", [])
    fingerprint = "" if has_errors else (compiled.get("fingerprint") or "")
    return ok_response({
        "ok": not has_errors,
        "chain": chain_out,
        "root_transforms": root_transforms,
        "display": compiled.get("display") or "",
        "statement_count": compiled.get("statement_count") or 0,
        "fingerprint": fingerprint,
        "diagnostics": diagnostics,
        "program": {
            "program_kind": "root_program",
            "spec_version": compiled.get("spec_version") or 2,
            "source_text": "" if has_errors else source_text,
            "chain": chain_out,
            "root_transforms": root_transforms,
            "display": compiled.get("display") or "",
            "fingerprint": fingerprint,
            "execution_spec": "" if has_errors else (compiled.get("execution_spec") or ""),
            "tokens": [] if has_errors else (compiled.get("tokens") or []),
            "token_count": 0 if has_errors else (compiled.get("token_count") or 0),
        },
    })


def handle_list_coeff_programs(event):
    parse_body(event)
    programs = []
    errors = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=COEFF_PROGRAMS_PREFIX):
        for obj in page.get("Contents", []):
            key = obj.get("Key", "")
            if not key.endswith(".json") or key.endswith("/"):
                continue
            program_id = key[len(COEFF_PROGRAMS_PREFIX):-5]
            if not program_id or "/" in program_id:
                continue
            try:
                program = _coeff_program_summary_from_head(program_id)
            except _CoeffProgramNotFound:
                continue
            except Exception as exc:
                try:
                    program = _read_coeff_program_object(program_id)
                except _CoeffProgramNotFound:
                    continue
                except Exception as read_exc:
                    err_text = str(read_exc)
                    print(f"coeff program list skipped {key}: {err_text}")
                    errors.append({
                        "id": program_id,
                        "error": err_text[:240],
                    })
                    continue
                else:
                    print(
                        f"coeff program list used full read fallback for {key}: "
                        f"{type(exc).__name__}: {exc}"
                    )
            programs.append(program)
    programs = sorted(programs, key=lambda row: row["id"])
    programs = sorted(programs, key=lambda row: row.get("saved_at") or "", reverse=True)
    return ok_response({
        "programs": programs,
        "count": len(programs),
        "order": "saved_at_desc",
        "errors": errors,
        "error_count": len(errors),
    })


def handle_fetch_coeff_program(event):
    params = parse_body(event)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError("coeff program fetch requires id")
    return ok_response({"program": _read_coeff_program_object(program_id, prefer_v2=True)})


def handle_save_coeff_program(event):
    params = parse_body(event)
    program = _compile_coeff_program_payload(
        params.get("name"),
        params.get("chain"),
        source_text=coeff_source_text_from_payload(params),
    )
    key = _coeff_program_key(program["id"])
    overwritten = _key_exists(key)
    _put_program_v1_object(
        key, _coeff_program_v2_key(program["id"]),
        (json.dumps(program, indent=2) + "\n").encode("utf-8"),
        metadata=_coeff_program_put_metadata(program),
    )
    return ok_response({"program": program, "overwritten": overwritten})


class _RootProgramNotFound(RuntimeError):
    pass


class _BookNotFound(RuntimeError):
    pass


def _book_key(book_id):
    return f"{BOOKS_PREFIX}{_normalize_program_id(book_id)}.json"


def _book_allcol_cover_source_key(book_id, refresh_id):
    return f"{BOOKS_PREFIX}{book_id}/cover/allcol-{refresh_id}.jpg"


def _book_allcol_cover_preview_key(book_id, refresh_id):
    return f"{BOOKS_PREFIX}{book_id}/cover/allcol-{refresh_id}-preview.jpg"


def _validate_book_cover_source(raw_source, *, book_id, entry_ids, legacy_cover=""):
    """Normalize the additive cover-source union while preserving v1 books."""
    if raw_source is None:
        if legacy_cover:
            raw_source = {"kind": "entry", "entry_id": legacy_cover}
        else:
            raw_source = {"kind": "none"}
    if not isinstance(raw_source, dict):
        raise ValueError("book cover_source must be an object")

    kind = str(raw_source.get("kind") or "none").strip().lower()
    if kind == "none":
        return {"version": BOOK_COVER_SOURCE_VERSION, "kind": "none"}, ""
    if kind == "entry":
        entry_id = str(raw_source.get("entry_id") or legacy_cover or "").strip()
        if not entry_id or entry_id not in entry_ids:
            raise ValueError("book entry cover_source does not match any entry")
        return {
            "version": BOOK_COVER_SOURCE_VERSION,
            "kind": "entry",
            "entry_id": entry_id,
        }, entry_id
    if kind != "allcol_wall":
        raise ValueError("book cover_source kind must be none, entry, or allcol_wall")

    refresh_id = str(raw_source.get("refresh_id") or "").strip()
    if not _BOOK_WALL_REFRESH_RE.fullmatch(refresh_id):
        raise ValueError("book AllCol cover refresh_id is invalid")
    image_key = str(raw_source.get("image_key") or "").strip()
    expected_image_key = _book_allcol_cover_source_key(book_id, refresh_id)
    if image_key != expected_image_key:
        raise ValueError(
            f"book AllCol cover image_key must be the frozen book source {expected_image_key!r}")
    preview_key = str(raw_source.get("preview_key") or "").strip()
    expected_preview_key = _book_allcol_cover_preview_key(book_id, refresh_id)
    if preview_key and preview_key != expected_preview_key:
        raise ValueError(
            f"book AllCol cover preview_key must be {expected_preview_key!r}")
    try:
        width = int(raw_source.get("width") or 0)
        height = int(raw_source.get("height") or 0)
    except (TypeError, ValueError):
        raise ValueError("book AllCol cover dimensions must be integers")
    if width <= 0 or height <= 0:
        raise ValueError("book AllCol cover dimensions must be positive")
    selected_at = str(raw_source.get("selected_at") or "")
    if (len(selected_at) > 64 or any(ch in "\r\n\t" for ch in selected_at)
            or not all(ch.isprintable() for ch in selected_at)):
        raise ValueError("book AllCol cover selected_at must be printable single-line text")
    return {
        "version": BOOK_COVER_SOURCE_VERSION,
        "kind": "allcol_wall",
        "refresh_id": refresh_id,
        "image_key": image_key,
        "preview_key": preview_key,
        "width": width,
        "height": height,
        "selected_at": selected_at,
    }, ""


def _slugify_book_id(name):
    slug = _slugify_coeff_program_id(name)
    return "book" if slug == "coeff-program" else slug


def _normalize_book_background_color(value):
    text = str(value or BOOK_DEFAULT_BACKGROUND_COLOR).strip().lower()
    if text.startswith("#"):
        text = text[1:]
    if len(text) == 3 and all(ch in "0123456789abcdef" for ch in text):
        text = "".join(ch + ch for ch in text)
    if len(text) != 6 or any(ch not in "0123456789abcdef" for ch in text):
        raise ValueError("book background_color must be 6-digit hex")
    return text


def _validate_book_payload(raw):
    """Validate + normalize a book document (see book-maker-design.md §4)."""
    if not isinstance(raw, dict):
        raise ValueError("book payload must be an object")
    kind = str(raw.get("book_kind") or "book")
    if kind != "book":
        raise ValueError(f"unknown book_kind {kind!r}")
    def _single_line(value, label):
        text = str(value or "")
        if any(ch in "\r\n\t" for ch in text) or not all(ch.isprintable() for ch in text):
            raise ValueError(f"book {label} must be printable single-line text")
        return text

    name = str(raw.get("name") or "").strip()
    if not name:
        raise ValueError("book name is required")
    if len(name) > MAX_BOOK_NAME_LEN:
        raise ValueError(f"book name must be at most {MAX_BOOK_NAME_LEN} characters")
    _single_line(name, "name")
    book_id = str(raw.get("id") or _slugify_book_id(name)).strip()
    if not re.fullmatch(r"[a-z0-9-]{1,64}", book_id):
        raise ValueError("book id must be a lowercase slug [a-z0-9-]{1,64}")
    entries_raw = raw.get("entries")
    if entries_raw is None:
        entries_raw = []
    if not isinstance(entries_raw, list):
        raise ValueError("book entries must be a list")
    if len(entries_raw) > MAX_BOOK_ENTRIES:
        raise ValueError(f"book has {len(entries_raw)} entries; max is {MAX_BOOK_ENTRIES}")
    entries = []
    seen_entry_ids = set()
    for idx, item in enumerate(entries_raw):
        if not isinstance(item, dict):
            raise ValueError(f"book entry {idx} must be an object")
        entry = {
            "entry_id": str(item.get("entry_id") or f"e{idx}_{uuid.uuid4().hex[:6]}").strip(),
            "job_id": str(item.get("job_id") or "").strip(),
            "artifact_id": str(item.get("artifact_id") or "").strip(),
            "image_key": str(item.get("image_key") or "").strip(),
            "display_name": str(item.get("display_name") or ""),
            "added_at": str(item.get("added_at") or ""),
            "title_override": str(item.get("title_override") or ""),
            "body_override": str(item.get("body_override") or ""),
        }
        for field in ("job_id", "artifact_id", "image_key"):
            if not entry[field]:
                raise ValueError(f"book entry {idx} is missing {field}")
        # image_key is trusted downstream as a raw LaTeX macro argument
        # (\qrcode{URL}) and an S3 GET target — pin it to render output, and
        # require it to name the SAME artifact as job_id/artifact_id so a page
        # image can't be paired with another artifact's metadata (F3)
        assert_safe_render_image_key(entry["image_key"], f"book entry {idx} image_key")
        assert_render_identity(entry["image_key"], entry["job_id"], entry["artifact_id"],
                               f"book entry {idx} image_key")
        if not re.fullmatch(r"[A-Za-z0-9._-]{1,64}", entry["entry_id"]):
            raise ValueError(f"book entry {idx} entry_id must match [A-Za-z0-9._-]{{1,64}}")
        if entry["entry_id"] in seen_entry_ids:
            raise ValueError(f"book entry {idx} has a duplicate entry_id")
        seen_entry_ids.add(entry["entry_id"])
        entries.append(entry)
    legacy_cover = str(raw.get("cover_entry_id") or "")
    if legacy_cover and legacy_cover not in seen_entry_ids:
        raise ValueError("cover_entry_id does not match any entry")
    cover_source, cover_entry_id = _validate_book_cover_source(
        raw.get("cover_source"),
        book_id=book_id,
        entry_ids=seen_entry_ids,
        legacy_cover=legacy_cover,
    )
    spread_layout = str(raw.get("spread_layout") or "color_primary")
    if spread_layout not in ("color_primary", "palette_primary"):
        raise ValueError("book spread_layout must be color_primary or palette_primary")
    return {
        "version": 2,
        "book_kind": "book",
        "id": book_id,
        "name": name,
        "title": _single_line(raw.get("title"), "title"),
        "subtitle": _single_line(raw.get("subtitle"), "subtitle"),
        "author": _single_line(raw.get("author"), "author"),
        "background_color": _normalize_book_background_color(raw.get("background_color")),
        # cover_entry_id remains for old clients and source archives; the
        # discriminated cover_source is authoritative for new code.
        "cover_entry_id": cover_entry_id,
        "cover_source": cover_source,
        # Layout tab: which artifact owns the full recto page. palette_primary
        # swaps the palette onto the recto and the color into the verso square.
        "spread_layout": spread_layout,
        "entries": entries,
        "saved_at": _utc_now_iso(),
    }


def _book_put_metadata(book):
    return {
        BOOK_META_NAME: str(book.get("name") or ""),
        BOOK_META_ENTRY_COUNT: str(len(book.get("entries") or [])),
        BOOK_META_SAVED_AT: str(book.get("saved_at") or ""),
    }


def _read_book_object_with_etag(book_id):
    key = _book_key(book_id)
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise _BookNotFound(f"book not found: {book_id}")
        raise
    payload = json.loads(obj["Body"].read())
    if str(payload.get("book_kind") or "") != "book":
        raise ValueError(f"object at {key} is not a book document")
    return payload, str(obj.get("ETag") or "").strip('"')


def _read_book_object(book_id):
    return _read_book_object_with_etag(book_id)[0]


VISION_CONFIG_JOB_ID = "__config__"
VISION_CONFIG_TASK_ID = "vision_model"


VISION_PROVIDERS = ("gemini", "anthropic", "openai")


def _vision_item():
    resp = _get_ddb().get_item(
        TableName=JOBS_TABLE,
        Key={"job_id": {"S": VISION_CONFIG_JOB_ID},
             "task_id": {"S": VISION_CONFIG_TASK_ID}})
    return resp.get("Item") or {}


def _vision_summary(item):
    from shared import vision_provider
    model = (item.get("model") or {}).get("S", "")
    providers = {}
    for prov in VISION_PROVIDERS:
        key = (item.get(f"api_key_{prov}") or {}).get("S", "")
        if not key and prov == "gemini":
            key = (item.get("api_key") or {}).get("S", "")  # legacy single-key
        providers[prov] = {"key_set": bool(key),
                           "key_hint": f"…{key[-4:]}" if len(key) >= 8 else ""}
    current = providers.get(vision_provider(model), {})
    return {
        "model": model,
        "providers": providers,
        "key_set": bool(current.get("key_set")),
        "key_hint": current.get("key_hint", ""),
        "updated_at": (item.get("updated_at") or {}).get("S", ""),
    }


def handle_fetch_vision_config(event):
    """Model + per-provider key presence. Keys NEVER leave the server —
    only last-4 hints."""
    del event
    return ok_response(_vision_summary(_vision_item()))


def handle_save_vision_config(event):
    """Set the vision model and/or paste a key. Keys are stored PER
    PROVIDER (derived from the model being saved) so switching models
    back and forth reuses the right stored key. DynamoDB only — the
    public bucket must never hold secrets. Empty api_key keeps the
    provider's existing key; api_key="-" clears it."""
    from shared import vision_provider
    params = parse_body(event)
    model = str(params.get("model") or "").strip()
    if model and not all(c.isalnum() or c in ".-_:" for c in model):
        raise ValueError("vision model id has unexpected characters")
    if len(model) > 80:
        raise ValueError("vision model id too long")
    api_key = str(params.get("api_key") or "").strip()
    if len(api_key) > 300:
        raise ValueError("api key too long")

    item = _vision_item()
    new_model = model or (item.get("model") or {}).get("S", "")
    prov = vision_provider(new_model)
    keys = {}
    for p in VISION_PROVIDERS:
        keys[p] = (item.get(f"api_key_{p}") or {}).get("S", "")
    if not keys["gemini"]:
        keys["gemini"] = (item.get("api_key") or {}).get("S", "")  # legacy migrate
    if api_key == "-":
        keys[prov] = ""
    elif api_key:
        keys[prov] = api_key

    new_item = {
        "job_id": {"S": VISION_CONFIG_JOB_ID},
        "task_id": {"S": VISION_CONFIG_TASK_ID},
        "model": {"S": new_model},
        "updated_at": {"S": _utc_now_iso()},
    }
    for p in VISION_PROVIDERS:
        new_item[f"api_key_{p}"] = {"S": keys[p]}
    _get_ddb().put_item(TableName=JOBS_TABLE, Item=new_item)
    return ok_response(_vision_summary(new_item))


class BookConflictError(Exception):
    """Optimistic-concurrency failure: the stored book moved under a caller
    that passed expected_saved_at (Describe's per-entry saves)."""


def handle_save_book(event):
    params = parse_body(event)
    book = _validate_book_payload(params.get("book"))
    key = _book_key(book["id"])
    overwritten = _key_exists(key)
    put_kwargs = dict(
        Bucket=BUCKET, Key=key,
        Body=json.dumps(book).encode("utf-8"),
        ContentType="application/json",
        Metadata=_book_put_metadata(book))
    # Real compare-and-swap (CR28 F5): when the caller passes the opaque
    # `revision` it fetched, S3 atomically refuses the write if the object's
    # ETag has changed (a concurrent save landed). No read-then-write TOCTOU
    # and no 1-second timestamp collision — this is what stops a long Describe
    # run from clobbering a human edit. Interactive Save omits it and keeps
    # last-write-wins, which is what a human clicking Save wants.
    expected_revision = str(params.get("expected_revision") or "").strip()
    if expected_revision:
        put_kwargs["IfMatch"] = expected_revision
    try:
        resp = s3.put_object(**put_kwargs)
    except ClientError as exc:
        code = str((exc.response.get("Error") or {}).get("Code") or "")
        if code in ("PreconditionFailed", "412", "ConditionalRequestConflict"):
            raise BookConflictError(
                f"book {book['id']} changed since revision {expected_revision!r}; "
                f"refetch and retry")
        raise
    new_rev = str((resp or {}).get("ETag") or "").strip('"')
    return ok_response({"book": book, "overwritten": overwritten, "revision": new_rev})


def handle_snapshot_book_cover(event):
    """Freeze the current full-resolution AllCol wall under one book.

    This is intentionally a byte-for-byte S3 copy. The Book prepare worker,
    not the mosaic worker, owns the later 5K libvips normalization.
    """
    params = parse_body(event)
    book_id = str(params.get("book_id") or "").strip()
    if not re.fullmatch(r"[a-z0-9-]{1,64}", book_id):
        raise ValueError("snapshot-book-cover requires a valid book_id")
    _read_book_object(book_id)  # fail before copying into a nonexistent book

    status = _read_mosaic_status("color", consistent=True)
    refresh_id = str(status.get("wall_refresh_id") or "").strip()
    wall_json_key = str(status.get("wall_json_key") or "").strip()
    if (status.get("state") != "ready" or status.get("wall_state") != "ready"
            or not _BOOK_WALL_REFRESH_RE.fullmatch(refresh_id)):
        raise ValueError(
            "Current AllCol wall is not ready; open AllCol, Refresh, and wait for the wall pyramid")
    wall_prefix = f"renders/_index/color_mosaic/{refresh_id}/"
    expected_wall_json_key = wall_prefix + "wall.json"
    if wall_json_key != expected_wall_json_key:
        raise ValueError("Current AllCol wall metadata does not match its refresh")

    wall_obj = s3.get_object(Bucket=BUCKET, Key=wall_json_key)
    wall = json.loads(wall_obj["Body"].read() or b"{}")
    if (not isinstance(wall, dict)
            or wall.get("manifest_type") != "artifact_wall_pyramid"
            or wall.get("kind") != "color"
            or str(wall.get("refresh_id") or "") != refresh_id):
        raise ValueError("Current AllCol wall metadata is malformed")
    source_key = str(wall.get("image_key") or "").strip()
    expected_source_key = wall_prefix + "wall.jpg"
    if not wall.get("flat_jpeg") or source_key != expected_source_key:
        raise ValueError(
            "Current AllCol wall has no flat JPEG (it exceeds the JPEG dimension limit); "
            "this wall cannot be used as a Book cover source")
    try:
        width = int(wall.get("width") or 0)
        height = int(wall.get("height") or 0)
    except (TypeError, ValueError):
        raise ValueError("Current AllCol wall dimensions are malformed")
    if width <= 0 or height <= 0:
        raise ValueError("Current AllCol wall dimensions are missing")

    selected_at = _utc_now_iso()
    frozen_key = _book_allcol_cover_source_key(book_id, refresh_id)
    try:
        s3.copy_object(
            Bucket=BUCKET,
            CopySource={"Bucket": BUCKET, "Key": source_key},
            Key=frozen_key,
            ContentType="image/jpeg",
            CacheControl=CACHE_IMMUTABLE,
            MetadataDirective="REPLACE",
            Metadata={
                "source_kind": "allcol_wall",
                "source_refresh_id": refresh_id,
                "source_width": str(width),
                "source_height": str(height),
            },
        )
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise ValueError("Current AllCol wall image disappeared; refresh AllCol and retry")
        raise

    # Level <= 8 is a one-tile overview of the whole DZI (tile size 256).
    # Copying it avoids making the 260px Book preview download the full wall.
    max_level = int(math.ceil(math.log2(max(width, height)))) if max(width, height) > 1 else 0
    preview_level = min(8, max_level)
    wall_preview_key = wall_prefix + f"wall_files/{preview_level}/0_0.jpg"
    frozen_preview_key = _book_allcol_cover_preview_key(book_id, refresh_id)
    preview_key = ""
    try:
        s3.copy_object(
            Bucket=BUCKET,
            CopySource={"Bucket": BUCKET, "Key": wall_preview_key},
            Key=frozen_preview_key,
            ContentType="image/jpeg",
            CacheControl=CACHE_IMMUTABLE,
            MetadataDirective="REPLACE",
            Metadata={"source_kind": "allcol_wall_preview", "source_refresh_id": refresh_id},
        )
        preview_key = frozen_preview_key
    except Exception as exc:
        print(f"Book AllCol cover preview copy skipped for {book_id}/{refresh_id}: {exc}")

    cover_source = {
        "version": BOOK_COVER_SOURCE_VERSION,
        "kind": "allcol_wall",
        "refresh_id": refresh_id,
        "image_key": frozen_key,
        "preview_key": preview_key,
        "width": width,
        "height": height,
        "selected_at": selected_at,
    }
    return ok_response({
        "book_id": book_id,
        "cover_source": cover_source,
        "preview_url": _s3_public_url(preview_key) if preview_key else "",
    })


def handle_fetch_book(event):
    params = parse_body(event)
    book_id = str(params.get("id") or "").strip()
    if not book_id:
        raise ValueError("book fetch requires id")
    book, revision = _read_book_object_with_etag(book_id)
    latest_output = None
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=f"{BOOKS_PREFIX}{book_id}/out/latest.json")
        latest_output = json.loads(obj["Body"].read())
    except Exception as exc:
        if not _is_missing_s3_error(exc):
            raise
    # `revision` is the opaque CAS token (S3 ETag) — pass it back on save (F5)
    return ok_response({"book": book, "latest_output": latest_output, "revision": revision})


def handle_list_books(event):
    del event
    books = []
    errors = []
    kwargs = {"Bucket": BUCKET, "Prefix": BOOKS_PREFIX}
    while True:
        resp = s3.list_objects_v2(**kwargs)
        for item in resp.get("Contents") or []:
            key = item.get("Key") or ""
            if not key.endswith(".json"):
                continue
            book_id = key[len(BOOKS_PREFIX):-len(".json")]
            # per-book assets/outputs share the prefix: skip nested keys
            if not book_id or "/" in book_id:
                continue
            # Body read, not HEAD: the selector shows title/subtitle now,
            # and S3 metadata is ASCII-constrained while titles are unicode.
            # Same request count as the old HEAD fast path; docs are small.
            try:
                payload = _read_book_object(book_id)
                books.append({
                    "id": book_id,
                    "name": payload.get("name") or book_id,
                    "title": str(payload.get("title") or ""),
                    "subtitle": str(payload.get("subtitle") or ""),
                    "entry_count": len(payload.get("entries") or []),
                    "saved_at": payload.get("saved_at") or "",
                })
            except Exception as inner:
                errors.append({"id": book_id, "error": str(inner)[:240]})
        if resp.get("IsTruncated"):
            kwargs["ContinuationToken"] = resp.get("NextContinuationToken")
            continue
        break
    books.sort(key=lambda row: (row.get("saved_at") or "", row.get("id") or ""), reverse=True)
    return ok_response({
        "books": books,
        "count": len(books),
        "order": "saved_at_desc",
        "errors": errors,
        "error_count": len(errors),
    })


def handle_delete_book(event):
    params = parse_body(event)
    book_id = str(params.get("id") or "").strip()
    if not book_id or "/" in book_id:
        raise ValueError("book delete requires a flat id")
    deleted = 0

    def _delete_keys(keys):
        nonlocal deleted
        for start in range(0, len(keys), 1000):
            chunk = keys[start:start + 1000]
            s3.delete_objects(Bucket=BUCKET, Delete={"Objects": [{"Key": k} for k in chunk]})
            deleted += len(chunk)

    doc_key = _book_key(book_id)
    if _key_exists(doc_key):
        _delete_keys([doc_key])
    # per-book assets + outputs live under a guarded prefix
    prefix = f"{BOOKS_PREFIX}{book_id}/"
    assert prefix.startswith(BOOKS_PREFIX)
    kwargs = {"Bucket": BUCKET, "Prefix": prefix}
    while True:
        resp = s3.list_objects_v2(**kwargs)
        keys = [item["Key"] for item in resp.get("Contents") or []]
        if keys:
            _delete_keys(keys)
        if resp.get("IsTruncated"):
            kwargs["ContinuationToken"] = resp.get("NextContinuationToken")
            continue
        break
    if deleted == 0:
        raise _BookNotFound(f"book not found: {book_id}")
    return ok_response({"id": book_id, "deleted": deleted})


def _root_program_key(program_id):
    return f"{ROOT_PROGRAMS_PREFIX}{_normalize_program_id(program_id)}.json"


def _validate_root_program_name(name):
    text = str(name or "").strip()
    if not text:
        raise ValueError("root program name is required")
    if len(text) > MAX_ROOT_PROGRAM_NAME_LEN:
        raise ValueError(f"root program name must be at most {MAX_ROOT_PROGRAM_NAME_LEN} characters")
    if any(ch in "\r\n\t" for ch in text) or not all(ch.isprintable() for ch in text):
        # the name travels in S3 metadata headers; control characters die
        # as opaque 500s inside botocore instead of a clean 400 here
        raise ValueError("root program name must contain printable single-line text")
    return text


def _slugify_root_program_id(name):
    slug = _slugify_coeff_program_id(name)
    return "root-program" if slug == "coeff-program" else slug


def _compile_root_program_payload(name, *, source_text, saved_at=None, program_id=None):
    """Saved root programs are source-first: the payload persists the source
    text plus the compiled artifacts (chain rows, fingerprint, execution
    spec), and the load path recompiles from source_text like the other
    program kinds."""
    validated_name = _validate_root_program_name(name)
    source_text = str(source_text or "")
    compiled = compile_root_program_source(source_text, strict=True)
    if not int(compiled.get("statement_count") or 0):
        raise ValueError("root program source is empty")
    program_id_text = str(program_id or _slugify_root_program_id(validated_name)).strip()
    saved_at_text = _utc_now_iso() if saved_at is None else str(saved_at or "").strip()
    return {
        "version": ROOT_PROGRAM_VERSION,
        "program_kind": "root_program",
        "id": program_id_text,
        "name": validated_name,
        "source_text": source_text,
        "chain": compiled.get("chain") or [],
        "root_transforms": compiled.get("root_transforms") or [],
        "fingerprint": compiled.get("fingerprint") or "",
        "execution_spec": compiled.get("execution_spec") or "",
        "display": compiled.get("display") or "",
        "statement_count": int(compiled.get("statement_count") or 0),
        "spec_version": compiled.get("spec_version"),
        "saved_at": saved_at_text,
    }


def _root_program_put_metadata(program):
    return {
        ROOT_PROGRAM_META_NAME: str(program.get("name") or ""),
        ROOT_PROGRAM_META_STATEMENT_COUNT: str(int(program.get("statement_count") or 0)),
        ROOT_PROGRAM_META_SAVED_AT: str(program.get("saved_at") or ""),
    }


def _read_root_program_object(program_id):
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=_root_program_key(program_id))
    except Exception as exc:
        if _is_missing_s3_error(exc):
            raise _RootProgramNotFound(f"root program not found: {program_id}")
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"saved root program is not valid JSON: {program_id}") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"saved root program must be a JSON object: {program_id}")
    if payload.get("program_kind") not in (None, "root_program"):
        raise RuntimeError(f"saved object is not a root program: {program_id}")
    if not str(payload.get("source_text") or "").strip() and payload.get("chain"):
        # Same guard the coeff loader documents: a hand-edited object with a
        # blank source but a real chain must not silently load as an empty
        # program.
        raise RuntimeError(f"saved root program has a blank source_text but a non-empty chain: {program_id}")
    return _compile_root_program_payload(
        payload.get("name"),
        source_text=str(payload.get("source_text") or ""),
        saved_at=payload.get("saved_at"),
        program_id=program_id,
    )


def _root_program_summary_from_head(program_id):
    resp = s3.head_object(Bucket=BUCKET, Key=_root_program_key(program_id))
    meta = resp.get("Metadata") or {}
    name = str(meta.get(ROOT_PROGRAM_META_NAME) or "").strip()
    saved_at = str(meta.get(ROOT_PROGRAM_META_SAVED_AT) or "").strip()
    count_raw = str(meta.get(ROOT_PROGRAM_META_STATEMENT_COUNT) or "").strip()
    if not name or not saved_at or not count_raw:
        raise RuntimeError(f"root program summary metadata missing for {program_id}")
    return {
        "id": str(program_id),
        "name": name,
        "statement_count": int(count_raw),
        "saved_at": saved_at,
    }


def handle_save_root_program(event):
    params = parse_body(event)
    program = _compile_root_program_payload(
        params.get("name"),
        source_text=str(params.get("source_text") or ""),
    )
    key = _root_program_key(program["id"])
    overwritten = _key_exists(key)
    s3.put_object(
        Bucket=BUCKET,
        Key=key,
        Body=(json.dumps(program, indent=2) + "\n").encode("utf-8"),
        ContentType="application/json",
        Metadata=_root_program_put_metadata(program),
    )
    return ok_response({"program": program, "overwritten": overwritten})


def handle_list_root_programs(event):
    parse_body(event)
    programs = []
    errors = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=ROOT_PROGRAMS_PREFIX):
        for obj in page.get("Contents", []):
            key = obj.get("Key", "")
            if not key.endswith(".json") or key.endswith("/"):
                continue
            program_id = key[len(ROOT_PROGRAMS_PREFIX):-5]
            if not program_id or "/" in program_id:
                continue
            try:
                programs.append(_root_program_summary_from_head(program_id))
            except Exception:
                try:
                    programs.append(_read_root_program_object(program_id))
                except Exception as exc:
                    # never silent: a program whose source stopped compiling
                    # must stay visible (and deletable) somewhere
                    print(f"list-root-programs: {program_id} unreadable: {exc}")
                    errors.append({"id": program_id, "error": str(exc)})
    programs.sort(key=lambda p: (str(p.get("saved_at") or ""), str(p.get("id") or "")), reverse=True)
    return ok_response({
        "programs": programs,
        "count": len(programs),
        "order": "saved_at_desc",
        "errors": errors,
        "error_count": len(errors),
    })


def handle_fetch_root_program(event):
    params = parse_body(event)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError("root program fetch requires id")
    return ok_response({"program": _read_root_program_object(program_id)})


def handle_delete_root_program(event):
    params = parse_body(event)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError("root program delete requires id")
    key = _root_program_key(program_id)
    if not _key_exists(key):
        raise _RootProgramNotFound(f"root program not found: {program_id}")
    s3.delete_object(Bucket=BUCKET, Key=key)
    return ok_response({"id": program_id, "deleted": 1})


def handle_compile_coeff_program_source(event):
    params = parse_body(event)
    source_text = str(params.get("source_text") or "")
    parsed = parse_coeff_program_source(source_text, strict=False)
    compiled = compile_coeff_program_chain(
        parsed["chain"],
        macro_resolver=_coeff_program_macro_resolver(None),
        strict=False,
    )
    diagnostics = list(parsed.get("diagnostics") or []) + list(compiled.get("diagnostics") or [])
    has_errors = any(d.get("level") == "error" for d in diagnostics)
    # Non-strict parsing skips failing statements and keeps lowering the rest.
    # Never hand back that partial chain/fingerprint: a caller that persists
    # the program payload without checking `ok` would store a truncated program.
    chain_out = [] if has_errors else parsed["chain"]
    fingerprint = "" if has_errors else (compiled.get("fingerprint") or "")
    return ok_response({
        "ok": not has_errors,
        "chain": chain_out,
        "display": parsed["display"],
        "statement_count": parsed["statement_count"],
        "fingerprint": fingerprint,
        "diagnostics": diagnostics,
        "program": {
            "chain": chain_out,
            "display": parsed["display"],
            "fingerprint": fingerprint,
            "execution_spec": "" if has_errors else (compiled.get("execution_spec") or ""),
            "token_count": 0 if has_errors else (compiled.get("token_count") or 0),
            "stack_max": 0 if has_errors else (compiled.get("stack_max") or 0),
        },
    })


def handle_delete_coeff_program(event):
    params = parse_body(event)
    program_id = str(params.get("id") or "").strip()
    if not program_id:
        raise ValueError("coeff program delete requires id")
    deleted = _delete_program_keys(
        _coeff_program_key(program_id), _coeff_program_v2_key(program_id)
    )
    if not deleted:
        raise _CoeffProgramNotFound(f"coeff program not found: {program_id}")
    return ok_response({"id": program_id, "deleted": deleted})


def _results_entry_fields(job_id, calc):
    """Derive the Results-table fields from a calc.json dict — the ONE source of
    truth for both the catalog rows and a direct calc read."""
    entry = {"job_id": job_id}
    entry["function"] = calc.get("function", "?")
    entry["degree"] = calc.get("degree", 0)
    entry["N"] = calc.get("N", calc.get("n1", 0))
    entry["n1"] = calc.get("n1", entry["N"])
    entry["n_chunks"] = calc.get("n_chunks", calc.get("n_stripes", 0))
    entry["times"] = calc.get("times", 1)
    chunks = calc.get("chunks", calc.get("stripes", []))
    entry["total_size"] = sum(s.get("bin_size", 0) for s in chunks)
    entry["total_size"] += calc.get("total_coeffs_size", 0)
    entry["total_roots"] = calc.get("total_roots",
        sum(s.get("bin_size", 0) for s in chunks) // 8)
    return entry


_RESULTS_CATALOG_NUM_FIELDS = ("degree", "N", "n1", "n_chunks", "times",
                               "total_size", "total_roots")


def _results_catalog_task_id(job_id):
    return f"result#{job_id}"


def _results_catalog_item(entry, *, no_calc=False):
    item = {
        "job_id": {"S": RESULTS_CATALOG_DDB_JOB_ID},
        "task_id": {"S": _results_catalog_task_id(entry["job_id"])},
        "result_job_id": {"S": entry["job_id"]},
        "cached_at_ms": {"N": str(int(time.time() * 1000))},
        "v": {"N": str(RESULTS_CATALOG_SCHEMA_VERSION)},
    }
    if no_calc:
        item["no_calc"] = {"S": "1"}
        return item
    etag = str(entry.get("_calc_etag") or "").strip('"')
    if etag:
        item["calc_etag"] = {"S": etag}   # source identity for repair tooling/diagnostics
    item["fn"] = {"S": str(entry.get("function", "?"))}
    for field in _RESULTS_CATALOG_NUM_FIELDS:
        try:
            item[field] = {"N": str(int(entry.get(field) or 0))}
        except (TypeError, ValueError):
            item[field] = {"N": "0"}
    return item


def _results_entry_from_catalog_item(item):
    if "no_calc" in item:
        return None
    entry = {"job_id": item["result_job_id"]["S"],
             "function": item.get("fn", {}).get("S", "?")}
    for field in _RESULTS_CATALOG_NUM_FIELDS:
        entry[field] = int(item.get(field, {}).get("N", "0"))
    return entry


def _read_results_catalog():
    """All catalog rows as {job_id: raw item} — one paginated Query."""
    ddb = _get_ddb()
    rows = {}
    kwargs = {
        "TableName": JOBS_TABLE,
        "KeyConditionExpression": "job_id = :jid",
        "ExpressionAttributeValues": {":jid": {"S": RESULTS_CATALOG_DDB_JOB_ID}},
    }
    while True:
        resp = ddb.query(**kwargs)
        for item in resp.get("Items", []):
            jid = item.get("result_job_id", {}).get("S", "")
            if jid:
                rows[jid] = item
        lek = resp.get("LastEvaluatedKey")
        if not lek:
            return rows
        kwargs["ExclusiveStartKey"] = lek


def _results_catalog_write_batch(put_items, delete_job_ids):
    """Best-effort BatchWriteItem in 25-item chunks with bounded
    UnprocessedItems retries (code-review-30 F9) — the first catalog build /
    rebuild=true writes hundreds of rows, and serial PutItem round trips put
    tens of seconds inside the /list request. Failures self-heal on the next
    reconcile. Returns (attempted, failed)."""
    requests = [{"PutRequest": {"Item": it}} for it in put_items]
    requests += [{"DeleteRequest": {"Key": {
        "job_id": {"S": RESULTS_CATALOG_DDB_JOB_ID},
        "task_id": {"S": _results_catalog_task_id(j)}}}} for j in delete_job_ids]
    if not requests:
        return 0, 0
    ddb = _get_ddb()
    attempted, failed = len(requests), 0
    for i in range(0, len(requests), 25):
        chunk = requests[i:i + 25]
        for _retry in range(3):
            try:
                resp = ddb.batch_write_item(RequestItems={JOBS_TABLE: chunk})
            except Exception:
                failed += len(chunk)
                chunk = []
                break
            chunk = (resp.get("UnprocessedItems") or {}).get(JOBS_TABLE) or []
            if not chunk:
                break
            time.sleep(0.05 * (_retry + 1))   # brief backoff before re-offering (CR30 follow-up F10)
        failed += len(chunk)
    return attempted, failed


def _results_catalog_put(item):
    """Best-effort cache write: a DDB blip must never fail the read path —
    the row self-heals on the next /list reconcile."""
    try:
        _get_ddb().put_item(TableName=JOBS_TABLE, Item=item)
        return True
    except Exception:
        return False


def _results_catalog_delete(job_id):
    try:
        _get_ddb().delete_item(TableName=JOBS_TABLE, Key={
            "job_id": {"S": RESULTS_CATALOG_DDB_JOB_ID},
            "task_id": {"S": _results_catalog_task_id(job_id)},
        })
    except Exception:
        pass


def handle_list(event):
    """List all computed results (results-list.md Phase 2: catalog + reconcile).

    Membership truth is the cheap renders/ prefix listing (O(n_jobs/1000)
    requests); per-job table fields come from the DDB catalog. Reconcile makes
    it self-healing with NO pipeline hooks required: jobs missing a row get
    their calc.json read once (then cached forever — calc.json is written once,
    at compute completion), rows whose prefix vanished are pruned, calc-less
    prefixes younger than 24h are re-probed (mid-compute window), and
    rebuild=true re-reads everything (escape hatch after manual S3 surgery).
    Transient calc-read errors are surfaced but never cached.
    """
    import concurrent.futures

    params = parse_body(event)
    requested_workers = _validate_results_list_workers(params.get("list_workers"))
    rebuild = parse_boolish(params.get("rebuild"), default=False)
    list_s3 = _results_list_s3_client(requested_workers)
    t0 = time.time()

    # 1) Membership: folder prefixes under renders/ — O(n_jobs), not O(all_objects)
    job_ids = []
    t_prefix_0 = time.time()
    paginator = list_s3.get_paginator('list_objects_v2')
    for page in paginator.paginate(Bucket=BUCKET, Prefix='renders/',
                                   Delimiter='/'):
        for prefix in page.get('CommonPrefixes', []):
            # prefix['Prefix'] = 'renders/job_id/'
            job_id = prefix['Prefix'].split('/')[1]
            # renders/_index, renders/_shared_mosaic hold shared internals
            if job_id and not job_id.startswith('_'):
                job_ids.append(job_id)
    prefix_list_us = int((time.time() - t_prefix_0) * 1e6)

    # 2) Catalog rows — one paginated Query
    t_cat_0 = time.time()
    rows = _read_results_catalog()
    catalog_read_us = int((time.time() - t_cat_0) * 1e6)

    # 3) Reconcile: which jobs need a calc.json read THIS call?
    now_ms = int(time.time() * 1000)

    def needs_read(job_id):
        item = rows.get(job_id)
        if item is None or rebuild:
            return True
        if _parse_int((item.get("v") or {}).get("N")) != RESULTS_CATALOG_SCHEMA_VERSION:
            return True   # older/unversioned schema — reconcile, don't decode as zeros (F10)
        if "no_calc" in item:
            cached = int(item.get("cached_at_ms", {}).get("N", "0"))
            return (now_ms - cached) < RESULTS_CATALOG_NO_CALC_RETRY_MS
        return False

    to_read = [j for j in job_ids if needs_read(j)]

    # Read calc.json for each reconciled job (parallelized) — table fields only
    def read_calc(job_id):
        entry = {"job_id": job_id}
        try:
            obj = list_s3.get_object(Bucket=BUCKET,
                                     Key=f"renders/{job_id}/calc.json")
            calc = json.loads(obj["Body"].read())
            entry = _results_entry_fields(job_id, calc)
            entry["_calc_etag"] = str(obj.get("ETag") or "")
        except Exception as exc:
            if isinstance(exc, ClientError) and _is_missing_s3_error(exc):
                # A render prefix without calc.json is not a usable compute
                # result: it may be an in-progress/failed run, or a lazy
                # preview artifact that raced with deletion. Do not surface it
                # as a broken "?" row in Results.
                entry["_skip_missing_calc"] = True
            else:
                entry["function"] = "?"
                entry["total_size"] = 0
                entry["_metadata_error"] = f"{type(exc).__name__}: {exc}"

        return entry

    list_workers = min(requested_workers, max(1, len(to_read) or 1))
    t_calc_0 = time.time()
    fresh = []
    if to_read:
        with concurrent.futures.ThreadPoolExecutor(max_workers=list_workers) as pool:
            fresh = list(pool.map(read_calc, to_read))
    calc_fetch_us = int((time.time() - t_calc_0) * 1e6)

    # 4) Fold fresh reads into the catalog + this response. Only authoritative
    # answers are cached: a missing calc.json caches a no_calc marker, a good
    # calc caches its fields, a TRANSIENT error is returned but never cached.
    metadata_errors = []
    entries = {}
    pending_items = []
    for entry in fresh:
        if entry.pop("_skip_missing_calc", False):
            pending_items.append(_results_catalog_item(entry, no_calc=True))
            continue
        err = entry.pop("_metadata_error", None)
        if err:
            metadata_errors.append({"job_id": entry["job_id"], "error": err[:200]})
            entry.pop("_calc_etag", None)
            entries[entry["job_id"]] = entry
            continue
        pending_items.append(_results_catalog_item(entry))
        entry.pop("_calc_etag", None)
        entries[entry["job_id"]] = entry

    # Cached rows serve every job not read this call.
    for job_id in job_ids:
        if job_id in entries:
            continue
        item = rows.get(job_id)
        entry = _results_entry_from_catalog_item(item) if item is not None else None
        if entry is not None:
            entries[job_id] = entry

    # 4b) Flush fresh rows in bounded batches (code-review-30 F9).
    t_w0 = time.time()
    writes_attempted, writes_failed = _results_catalog_write_batch(pending_items, [])
    catalog_write_us = int((time.time() - t_w0) * 1e6)

    # 5) Prune rows whose job prefix vanished (deletes, manual surgery).
    present = set(job_ids)
    prune_jobs = [jid for jid in rows if jid not in present]
    t_p0 = time.time()
    _, prune_failed = _results_catalog_write_batch([], prune_jobs)
    catalog_prune_us = int((time.time() - t_p0) * 1e6)
    # HONEST accounting (CR30 follow-up F10): report what actually pruned, not what was
    # requested — a failed delete self-heals next reconcile but must not lie.
    pruned = len(prune_jobs) - prune_failed

    skipped_missing_calc = len([j for j in job_ids if j not in entries])
    fresh_jobs = {e.get("job_id") for e in fresh}
    catalog_hits = len([j for j in entries if j not in fresh_jobs])
    results = list(entries.values())

    # Sort by job_id descending (job_ids contain timestamps)
    t_sort_0 = time.time()
    results.sort(key=lambda r: r["job_id"], reverse=True)
    sort_us = int((time.time() - t_sort_0) * 1e6)

    return ok_response({
        "results": results,
        "count": len(results),
        "list_us": int((time.time() - t0) * 1e6),
        "prefix_list_us": prefix_list_us,
        "calc_fetch_us": calc_fetch_us,
        "catalog_read_us": catalog_read_us,
        "catalog_write_us": catalog_write_us,
        "catalog_writes_attempted": writes_attempted,
        "catalog_writes_failed": writes_failed,
        "catalog_prune_us": catalog_prune_us,
        "catalog_prune_failed": prune_failed,
        "catalog_hits": catalog_hits,
        "catalog_misses": len(to_read),
        "catalog_pruned": pruned,
        "sort_us": sort_us,
        "list_workers": list_workers,
        "s3_pool_connections": _results_list_pool_size(requested_workers),
        "skipped_missing_calc": skipped_missing_calc,
        "metadata_error_count": len(metadata_errors),
        "metadata_errors": metadata_errors[:20],
    })


def handle_list_palettes(event):
    """List immutable palette artifacts for a single job.
    Returns newest-first entries from renders/{job_id}/palettes/*/meta.json.
    """
    params = parse_body(event)
    job_id = params["job_id"]
    palettes = _list_saved_palettes(job_id)
    palettes = _order_palette_variants(palettes)
    return ok_response({"job_id": job_id, "palettes": palettes, "count": len(palettes)})


def handle_delete_palette(event):
    """Delete one immutable palette variant under renders/{job_id}/palettes/{palette_id}/."""
    params = parse_body(event)
    job_id = params["job_id"]
    palette_id = params["palette_id"]
    prefix = f"renders/{job_id}/palettes/{palette_id}/"

    objects = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        objects.extend(page.get("Contents", []))

    if not objects:
        return ok_response({"job_id": job_id, "palette_id": palette_id, "deleted": 0})

    total_deleted = 0
    for i in range(0, len(objects), 1000):
        batch = objects[i:i + 1000]
        resp = s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": obj["Key"]} for obj in batch]},
        )
        total_deleted += len(resp.get("Deleted", []))

    return ok_response({"job_id": job_id, "palette_id": palette_id, "deleted": total_deleted})


RENDER_FAMILY_DIRS = {
    "color": "color",
    "bilevel": "bilevel",
    "coeffs": "coeffs",
    "pdf": "pdf",
}

RENDER_FAMILY_SHAPES = {
    "color": {
        "image_candidates": ["image.jpeg", "image.png"],
        "preview_candidates": ["preview.png"],
        "legacy_image_candidates": ["image.jpeg", "image.png"],
        "legacy_preview_candidates": ["preview_color.png"],
    },
    "bilevel": {
        "image_candidates": ["image.tif", "image.png"],
        "preview_candidates": ["preview.png"],
        "legacy_image_candidates": ["image_bilevel.tif"],
        "legacy_preview_candidates": ["preview_bilevel.png", "image_bilevel_preview.png"],
    },
    "coeffs": {
        "image_candidates": ["image.tif"],
        "preview_candidates": ["preview.png"],
        "legacy_image_candidates": ["image_coeffs_bilevel.tif"],
        "legacy_preview_candidates": ["preview_coeffs.png", "image_coeffs_bilevel_preview.png"],
    },
    "palette": {
        "legacy_image_candidates": ["image_palette.jpeg"],
        "legacy_preview_candidates": ["preview_palette.png"],
    },
    "pdf": {
        "image_candidates": ["document.pdf"],
        "preview_candidates": [],
        "legacy_image_candidates": [],
        "legacy_preview_candidates": [],
    },
}


def _parse_root_transforms(raw):
    if not raw:
        return []
    try:
        return json.loads(raw)
    except Exception:
        return []


def _palette_artifact_prefixes(job_id, *, s3_client=None):
    client = s3_client or s3
    base_prefix = f"renders/{job_id}/palettes/"
    palette_prefixes = []
    paginator = client.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=base_prefix, Delimiter="/"):
        palette_prefixes.extend(p["Prefix"] for p in page.get("CommonPrefixes", []))
    return palette_prefixes


def _palette_entry_from_prefix(job_id, prefix, *, presign=True, s3_client=None, strict=False):
    client = s3_client or s3
    try:
        obj = client.get_object(Bucket=BUCKET, Key=prefix + "meta.json")
        meta = json.loads(obj["Body"].read())
    except Exception as exc:
        if strict and not _is_missing_s3_error(exc):
            raise
        return None
    if not isinstance(meta, dict):
        if strict:
            raise ValueError(f"palette meta is not an object: {prefix}meta.json")
        return None

    palette_id = meta.get("palette_id") or prefix.rstrip("/").split("/")[-1]
    image_key = meta.get("image_key", prefix + "image.jpeg")
    preview_key = meta.get("preview_key", prefix + "preview.png")
    score_key = meta.get("score_key")
    palette_bins_key = meta.get("palette_bins_key")
    section_scores_prefix = meta.get("section_scores_prefix", meta.get("chunk_scores_prefix", prefix + "chunks/score_section_"))
    section_bins_prefix = meta.get("section_bins_prefix", meta.get("chunk_bins_prefix", prefix + "chunks/palette_bins_section_"))
    section_meta_prefix = meta.get("section_meta_prefix", meta.get("chunk_meta_prefix", prefix + "chunks/meta_section_"))
    render_reusable = _parse_bool(meta.get("render_reusable"), False)
    data_layout = meta.get("data_layout", "")
    meta["family"] = "palette"
    meta["palette_id"] = palette_id
    meta["artifact_id"] = palette_id
    meta["image_key"] = image_key
    meta["preview_key"] = preview_key
    if score_key:
        meta["score_key"] = score_key
    if palette_bins_key:
        meta["palette_bins_key"] = palette_bins_key
    meta["section_scores_prefix"] = section_scores_prefix
    meta["section_bins_prefix"] = section_bins_prefix
    meta["section_meta_prefix"] = section_meta_prefix
    meta["chunk_scores_prefix"] = section_scores_prefix
    meta["chunk_bins_prefix"] = section_bins_prefix
    meta["chunk_meta_prefix"] = section_meta_prefix
    meta["render_reusable"] = render_reusable
    meta["data_layout"] = data_layout
    color_interpretation = _parse_color_interpretation(
        meta.get("color_interpretation")
        or meta.get("score_output_interpretation")
        or meta.get("interpretation")
    )
    meta["color_interpretation"] = color_interpretation
    meta["score_output_interpretation"] = color_interpretation
    meta["score_output_channel_count"] = _parse_int(meta.get("score_output_channel_count")) or 1
    meta["raw_channels"] = _parse_int(meta.get("raw_channels")) or meta["score_output_channel_count"]
    meta["raw_layout"] = meta.get("raw_layout", "")
    meta["raw_key"] = meta.get("raw_key", "")
    meta["raw_meta_key"] = meta.get("raw_meta_key", "")
    meta["palette_variant_fingerprint"] = meta.get("palette_variant_fingerprint", "")
    meta["content_fingerprint"] = meta.get("content_fingerprint", "")
    meta["solve_score_omega_enabled"] = _parse_bool(meta.get("solve_score_omega_enabled"), True)
    meta["solve_score_chain"] = meta.get("solve_score_chain", [])
    meta["derived_from_palette_id"] = meta.get("derived_from_palette_id", "")
    # Authoritative owner of a palette artifact — written by palette finalize
    # (code-review-28 F17). AllPal "Add to Book" must read this, never infer the
    # color id by stripping a 'pal_' prefix off the display id.
    meta["derived_from_color_artifact_id"] = meta.get("derived_from_color_artifact_id", "")
    if presign:
        meta["image_url"] = client.generate_presigned_url(
            "get_object", Params={"Bucket": BUCKET, "Key": image_key},
            ExpiresIn=PRESIGN_EXPIRY,
        )
        meta["preview_url"] = client.generate_presigned_url(
            "get_object", Params={"Bucket": BUCKET, "Key": preview_key},
            ExpiresIn=PRESIGN_EXPIRY,
        )
        meta["viewer_url"] = meta["preview_url"] or meta["image_url"]
    meta["file_size"] = meta.get("file_size", 0)
    meta["size"] = meta.get("file_size", 0)
    meta["format"] = "jpeg"
    return meta


def _list_saved_palettes(job_id):
    import concurrent.futures

    palette_prefixes = _palette_artifact_prefixes(job_id)

    def read_meta(prefix):
        return _palette_entry_from_prefix(job_id, prefix, presign=True, strict=False)

    with concurrent.futures.ThreadPoolExecutor(max_workers=min(len(palette_prefixes), 20) or 1) as pool:
        return [m for m in pool.map(read_meta, palette_prefixes) if m]


def _first_existing(head_results, keys):
    for key in keys:
        info = head_results.get(key)
        if info and info.get("exists"):
            return info
    return None


def _parse_float(value):
    if value in ("", None):
        return None
    try:
        return float(value)
    except Exception:
        return None


def _parse_int(value):
    if value in ("", None):
        return None
    try:
        return int(value)
    except Exception:
        return None


def _parse_bool(value, default=False):
    if value in ("", None):
        return default
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return bool(value)
    return str(value).strip().lower() in ("1", "true", "yes", "on")


def _parse_color_interpretation(value):
    if value in ("", None):
        return ""
    try:
        return normalize_color_interpretation(value)
    except Exception:
        return ""


def _parse_json(value):
    if value in ("", None):
        return None
    if isinstance(value, (dict, list)):
        return value
    try:
        return json.loads(value)
    except Exception:
        return None


def _solve_score_source_from_nonempty_chain(chain):
    if not isinstance(chain, list) or not chain:
        return ""
    try:
        return solve_score_source_text_from_chain(chain)
    except Exception:
        return ""


def _legacy_solve_score_program_from_meta(meta):
    if str((meta or {}).get("color_mode") or "").strip() != "solve_score":
        return None
    if not any((meta or {}).get(key) not in ("", None) for key in (
        "solve_metric",
        "solve_score_quantile",
        "solve_score_omega",
        "solve_score_omega_enabled",
    )):
        return None
    try:
        compiled = compile_solve_score_chain_or_legacy(
            (meta or {}).get("solve_score_chain", ""),
            (meta or {}).get("solve_metric", ""),
            (meta or {}).get("solve_score_quantile", ""),
            (meta or {}).get("solve_score_omega", 1.0),
            (meta or {}).get("solve_score_omega_enabled", True),
            default_metric="proximity",
        )
        public_chain = json.loads(serialize_solve_score_chain(compiled["chain"]))
        source_text = solve_score_source_text_from_chain(public_chain)
        return {"chain": public_chain, "source_text": source_text}
    except Exception:
        return None


def _load_color_artifact_overlay(job_id, artifact_id, *, s3_client=None):
    client = s3_client or s3
    try:
        obj = client.get_object(Bucket=BUCKET, Key=color_artifact_meta_key(job_id, artifact_id))
    except Exception as exc:
        # Genuine absence -> no overlay (expected). A throttle/5xx/access error
        # is NOT absence: propagate so the summary retries rather than silently
        # dropping this artifact's provenance overlay (code-review-28 F13).
        if is_missing_s3_error(exc):
            return None
        raise
    try:
        body = obj["Body"].read()
        data = json.loads(body)
    except Exception:
        # Object exists but is unreadable/corrupt JSON — genuinely unusable, so
        # skip the overlay (distinct from a transient fetch failure above).
        return None
    return data if isinstance(data, dict) else None


def _render_artifact_entry(family, artifact_id, image_info, preview_info=None, fallback_meta=None, legacy=False):
    meta = {}
    if image_info:
        meta.update(image_info.get("user_meta", {}) or {})
    if fallback_meta:
        meta.update(fallback_meta)

    image_key = image_info["key"]
    image_url = image_info.get("url")
    preview_key = preview_info["key"] if preview_info and preview_info.get("exists") else None
    preview_url = preview_info.get("url") if preview_info and preview_info.get("exists") else None
    created_at = meta.get("created_at") or image_info.get("modified_at")
    entry = {
        "family": family,
        "artifact_id": meta.get("artifact_id", artifact_id),
        "created_at": created_at,
        "image_key": image_key,
        "image_url": image_url,
        "preview_key": preview_key,
        "preview_url": preview_url,
        "viewer_url": preview_url or image_url,
        "preview_jpg_key": str(meta.get("preview_jpg_key") or ""),
        "preview_jpg_width": _parse_int(meta.get("preview_jpg_width")) or None,
        "preview_jpg_height": _parse_int(meta.get("preview_jpg_height")) or None,
        "width": image_info.get("width"),
        "height": image_info.get("height"),
        "file_size": image_info.get("size", 0),
        "size": image_info.get("size", 0),
        "content_type": image_info.get("type", ""),
        "format": meta.get("format") or image_key.rsplit(".", 1)[-1].lower(),
        "root_transforms": _parse_root_transforms(meta.get("root_transforms")),
        "root_program_source_text": meta.get("root_program_source_text", ""),
        "root_program": _parse_json(meta.get("root_program")) or {},
        "root_program_fingerprint": meta.get("root_program_fingerprint", ""),
        "root_spec_version": _parse_int(meta.get("root_spec_version")) or 1,
        "solve_score_chain": _parse_json(meta.get("solve_score_chain")),
        "solve_score_program_source_text": (
            meta.get("solve_score_program_source_text")
            or meta.get("score_source_text")
            or ""
        ),
        "score_source_text": (
            meta.get("score_source_text")
            or meta.get("solve_score_program_source_text")
            or ""
        ),
        "palette_source_score_source_text": meta.get("palette_source_score_source_text", ""),
        "palette_source_solve_score_program_source_text": meta.get("palette_source_solve_score_program_source_text", ""),
        "associated_palette_score_source_text": meta.get("associated_palette_score_source_text", ""),
        "associated_palette_solve_score_program_source_text": meta.get("associated_palette_solve_score_program_source_text", ""),
        "rotation": _parse_float(meta.get("rotation")),
        "degree": meta.get("degree"),
        "pix": meta.get("pix"),
        "quality": _parse_float(meta.get("quality")),
        "view_mode": meta.get("view_mode", ""),
        "quantile": _parse_float(meta.get("quantile")),
        "shim": _parse_float(meta.get("shim")),
        "square_extent": _parse_float(meta.get("square_extent")),
        "min_re": _parse_float(meta.get("min_re")),
        "max_re": _parse_float(meta.get("max_re")),
        "min_im": _parse_float(meta.get("min_im")),
        "max_im": _parse_float(meta.get("max_im")),
        "legacy": legacy,
    }
    if not str(entry["solve_score_program_source_text"] or "").strip():
        reconstructed = _solve_score_source_from_nonempty_chain(entry["solve_score_chain"])
        if reconstructed:
            entry["solve_score_program_source_text"] = reconstructed
            if not str(entry["score_source_text"] or "").strip():
                entry["score_source_text"] = reconstructed
        else:
            legacy_program = _legacy_solve_score_program_from_meta(meta)
            if legacy_program:
                entry["solve_score_chain"] = legacy_program["chain"]
                entry["solve_score_program_source_text"] = legacy_program["source_text"]
                if not str(entry["score_source_text"] or "").strip():
                    entry["score_source_text"] = legacy_program["source_text"]

    if family == "color":
        repalette_capable = str(meta.get("repalette_capable", "")).strip().lower() == "true"
        entry["color_mode"] = meta.get("color_mode", "")
        entry["match_mode"] = meta.get("match_mode", "")
        entry["palette"] = meta.get("palette", "")
        entry["palette_display_name"] = meta.get("palette_display_name", "")
        entry["constant_color"] = meta.get("constant_color", "")
        entry["background_color"] = meta.get("background_color", "")
        entry["background_threshold"] = _parse_float(meta.get("background_threshold"))
        entry["solve_score_normalize"] = _parse_bool(meta.get("solve_score_normalize"), False)
        entry["score_output_normalize"] = _parse_bool(meta.get("score_output_normalize"), False)
        entry["score_output_clip_lo"] = _parse_float(meta.get("score_output_clip_lo"))
        entry["score_output_clip_hi"] = _parse_float(meta.get("score_output_clip_hi"))
        color_interpretation = _parse_color_interpretation(
            meta.get("color_interpretation")
            or meta.get("score_output_interpretation")
            or meta.get("interpretation")
        )
        entry["color_interpretation"] = color_interpretation
        entry["score_output_interpretation"] = color_interpretation
        entry["score_output_channel_count"] = _parse_int(meta.get("score_output_channel_count"))
        entry["raw_channels"] = _parse_int(meta.get("raw_channels"))
        entry["raw_layout"] = meta.get("raw_layout", "")
        entry["palette_source_id"] = meta.get("palette_source_id", "")
        entry["palette_source_display_name"] = meta.get("palette_source_display_name", "")
        entry["palette_source_palette"] = meta.get("palette_source_palette", "")
        entry["palette_source_metric"] = meta.get("palette_source_metric", "")
        entry["palette_source_score_chain"] = _parse_json(meta.get("palette_source_score_chain"))
        if not str(entry["palette_source_solve_score_program_source_text"] or entry["palette_source_score_source_text"] or "").strip():
            reconstructed = _solve_score_source_from_nonempty_chain(entry["palette_source_score_chain"])
            if reconstructed:
                entry["palette_source_solve_score_program_source_text"] = reconstructed
                entry["palette_source_score_source_text"] = reconstructed
        src_q = meta.get("palette_source_quantile", "")
        entry["palette_source_quantile"] = float(src_q) if src_q not in ("", None) else None
        src_omega = meta.get("palette_source_omega", "")
        entry["palette_source_omega"] = float(src_omega) if src_omega not in ("", None) else None
        entry["palette_source_omega_enabled"] = _parse_bool(meta.get("palette_source_omega_enabled"), True)
        entry["associated_palette_mode"] = meta.get("associated_palette_mode", "")
        entry["associated_palette_id"] = meta.get("associated_palette_id", "")
        entry["associated_palette_display_name"] = meta.get("associated_palette_display_name", "")
        entry["associated_palette_image_key"] = meta.get("associated_palette_image_key", "")
        entry["associated_palette_preview_key"] = meta.get("associated_palette_preview_key", "")
        entry["associated_palette_palette"] = meta.get("associated_palette_palette", "")
        entry["associated_palette_color_interpretation"] = _parse_color_interpretation(
            meta.get("associated_palette_color_interpretation")
        )
        entry["associated_palette_metric"] = meta.get("associated_palette_metric", "")
        entry["associated_palette_score_chain"] = _parse_json(meta.get("associated_palette_score_chain"))
        if not str(entry["associated_palette_solve_score_program_source_text"] or entry["associated_palette_score_source_text"] or "").strip():
            reconstructed = _solve_score_source_from_nonempty_chain(entry["associated_palette_score_chain"])
            if reconstructed:
                entry["associated_palette_solve_score_program_source_text"] = reconstructed
                entry["associated_palette_score_source_text"] = reconstructed
        assoc_q = meta.get("associated_palette_quantile", "")
        entry["associated_palette_quantile"] = float(assoc_q) if assoc_q not in ("", None) else None
        assoc_omega = meta.get("associated_palette_omega", "")
        entry["associated_palette_omega"] = float(assoc_omega) if assoc_omega not in ("", None) else None
        entry["associated_palette_omega_enabled"] = _parse_bool(
            meta.get("associated_palette_omega_enabled"),
            bool(entry["associated_palette_mode"]),
        )
        entry["render_execution"] = _parse_json(meta.get("render_execution"))
        entry["derived_from_artifact_id"] = meta.get("derived_from_artifact_id", "")
        entry["derivation_kind"] = meta.get("derivation_kind", "")
        entry["postprocess_kind"] = meta.get("postprocess_kind", "")
        entry["postprocess_profile"] = meta.get("postprocess_profile", "")
        entry["autolevels_params"] = _parse_json(meta.get("autolevels_params"))
        entry["resize_params"] = _parse_json(meta.get("resize_params"))
        entry["repalette_capable"] = repalette_capable
        entry["raw_key"] = meta.get("raw_key", "")
        entry["raw_meta_key"] = meta.get("raw_meta_key", "")
        entry["step_scores_key"] = meta.get("step_scores_key", "")
        entry["step_count"] = _parse_int(meta.get("step_count"))
        entry["step_scores_grid_n"] = _parse_int(meta.get("step_scores_grid_n"))
    elif family in ("bilevel", "coeffs"):
        entry["derived_from_artifact_id"] = meta.get("derived_from_artifact_id", "")
        entry["derived_from_image_key"] = meta.get("derived_from_image_key", "")
        entry["postprocess_kind"] = meta.get("postprocess_kind", "")
        entry["postprocess_profile"] = meta.get("postprocess_profile", "")
        entry["threshold"] = _parse_int(meta.get("threshold"))
        entry["render_execution"] = _parse_json(meta.get("render_execution"))
        entry["bilevel_pipeline"] = meta.get("bilevel_pipeline", "")
        entry["bilevel_section_mode"] = meta.get("bilevel_section_mode", "")
        entry["bilevel_section_count"] = _parse_int(meta.get("bilevel_section_count"))
    elif family == "pdf":
        page_count = meta.get("page_count")
        entry["pdf_kind"] = meta.get("pdf_kind", "")
        entry["source_family"] = meta.get("source_family", "")
        entry["source_artifact_id"] = meta.get("source_artifact_id", "")
        entry["source_image_key"] = meta.get("source_image_key", "")
        entry["source_display_name"] = meta.get("source_display_name", "")
        entry["source_color_mode"] = meta.get("source_color_mode", "")
        entry["source_palette"] = meta.get("source_palette", "")
        entry["source_palette_display_name"] = meta.get("source_palette_display_name", "")
        entry["source_solve_metric"] = meta.get("source_solve_metric", "")
        entry["source_solve_score_quantile"] = _parse_float(meta.get("source_solve_score_quantile"))
        entry["source_solve_score_omega"] = _parse_float(meta.get("source_solve_score_omega"))
        entry["source_solve_score_omega_enabled"] = _parse_bool(meta.get("source_solve_score_omega_enabled"), True)
        entry["page_count"] = int(page_count) if page_count not in ("", None) else None
    return entry


def _sort_variants_by_created_desc(items):
    items.sort(key=lambda a: a.get("created_at", ""), reverse=True)
    return items


def _order_color_variants(variants):
    by_id = {v.get("artifact_id"): v for v in variants if v.get("artifact_id")}
    children = {}
    top = []
    for art in variants:
        parent_id = art.get("derived_from_artifact_id") or ""
        if parent_id and parent_id in by_id and parent_id != art.get("artifact_id"):
            children.setdefault(parent_id, []).append(art)
        else:
            top.append(art)

    for art_list in children.values():
        _sort_variants_by_created_desc(art_list)
    _sort_variants_by_created_desc(top)

    ordered = []
    seen = set()

    def append_with_children(art):
        aid = art.get("artifact_id")
        if aid in seen:
            return
        if aid:
            seen.add(aid)
        for child in children.get(aid, []):
            append_with_children(child)
        ordered.append(art)

    for art in top:
        append_with_children(art)
    for art in variants:
        append_with_children(art)
    return ordered


def _order_palette_variants(variants):
    by_id = {v.get("palette_id") or v.get("artifact_id"): v for v in variants if v.get("palette_id") or v.get("artifact_id")}
    children = {}
    top = []
    for art in variants:
        aid = art.get("palette_id") or art.get("artifact_id")
        parent_id = art.get("derived_from_palette_id") or ""
        if parent_id and parent_id in by_id and parent_id != aid:
            children.setdefault(parent_id, []).append(art)
        else:
            top.append(art)

    for art_list in children.values():
        _sort_variants_by_created_desc(art_list)
    _sort_variants_by_created_desc(top)

    ordered = []
    seen = set()

    def append_with_children(art):
        aid = art.get("palette_id") or art.get("artifact_id")
        if aid in seen:
            return
        if aid:
            seen.add(aid)
        for child in children.get(aid, []):
            append_with_children(child)
        ordered.append(art)

    for art in top:
        append_with_children(art)
    for art in variants:
        append_with_children(art)
    return ordered


def _render_family_artifact_prefixes(job_id, family, *, s3_client=None):
    client = s3_client or s3
    base_prefix = f"renders/{job_id}/{RENDER_FAMILY_DIRS[family]}/"
    artifact_prefixes = []
    paginator = client.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=base_prefix, Delimiter="/"):
        artifact_prefixes.extend(p["Prefix"] for p in page.get("CommonPrefixes", []))
    return artifact_prefixes


def _render_family_entry_from_prefix(job_id, family, prefix, *, presign=True, s3_client=None):
    shape = RENDER_FAMILY_SHAPES[family]
    artifact_id = prefix.rstrip("/").split("/")[-1]
    keys = [prefix + k for k in shape["image_candidates"] + shape["preview_candidates"]]
    head_results = _head_artifact_keys(keys, presign=presign, s3_client=s3_client)
    image_info = _first_existing(head_results, [prefix + k for k in shape["image_candidates"]])
    if not image_info:
        return None
    preview_info = _first_existing(head_results, [prefix + k for k in shape["preview_candidates"]])
    fallback_meta = _load_color_artifact_overlay(job_id, artifact_id, s3_client=s3_client) if family == "color" else None
    return _render_artifact_entry(family, artifact_id, image_info, preview_info, fallback_meta=fallback_meta)


def _list_render_family_variants(job_id, family, *, presign=True, s3_client=None):
    import concurrent.futures

    artifact_prefixes = _render_family_artifact_prefixes(job_id, family, s3_client=s3_client)

    def read_prefix(prefix):
        return _render_family_entry_from_prefix(
            job_id,
            family,
            prefix,
            presign=presign,
            s3_client=s3_client,
        )

    variants = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=min(len(artifact_prefixes), 20) or 1) as pool:
        variants.extend(v for v in pool.map(read_prefix, artifact_prefixes) if v)

    return variants


def _legacy_render_variant(job_id, family, *, presign=True, s3_client=None):
    client = s3_client or s3
    shape = RENDER_FAMILY_SHAPES[family]
    prefix = f"renders/{job_id}/"
    keys = [prefix + k for k in shape["legacy_image_candidates"] + shape["legacy_preview_candidates"]]
    head_results = _head_artifact_keys(keys, presign=presign, s3_client=client)
    image_info = _first_existing(head_results, [prefix + k for k in shape["legacy_image_candidates"]])
    if not image_info:
        return None
    preview_info = _first_existing(head_results, [prefix + k for k in shape["legacy_preview_candidates"]])
    fallback_meta = _load_color_artifact_overlay(job_id, f"legacy_{family}", s3_client=client) if family == "color" else None
    return _render_artifact_entry(family, f"legacy_{family}", image_info, preview_info, fallback_meta=fallback_meta, legacy=True)


def _s3_public_url(key):
    from urllib.parse import quote
    return MOSAIC_BASE_URL + quote(str(key or ""), safe="/")


def _mosaic_now_ms():
    return int(time.time() * 1000)


def _mosaic_refresh_id():
    return f"mosaic_{time.strftime('%Y%m%dT%H%M%SZ', time.gmtime())}_{uuid.uuid4().hex[:8]}"


def _ddb_value(value):
    if value is None:
        return {"NULL": True}
    if isinstance(value, bool):
        return {"BOOL": value}
    if isinstance(value, int) and not isinstance(value, bool):
        return {"N": str(value)}
    if isinstance(value, float):
        return {"N": str(value)}
    if isinstance(value, dict):
        return {"M": {str(k): _ddb_value(v) for k, v in value.items()}}
    if isinstance(value, list):
        return {"L": [_ddb_value(v) for v in value]}
    return {"S": str(value)}


def _plain_value(attr):
    if not isinstance(attr, dict):
        return None
    if "S" in attr:
        return attr["S"]
    if "N" in attr:
        raw = attr["N"]
        try:
            value = float(raw)
            return int(value) if value.is_integer() else value
        except Exception:
            return raw
    if "BOOL" in attr:
        return bool(attr["BOOL"])
    if "NULL" in attr:
        return None
    if "M" in attr:
        return {k: _plain_value(v) for k, v in attr["M"].items()}
    if "L" in attr:
        return [_plain_value(v) for v in attr["L"]]
    return None


def _mosaic_task_id(kind):
    return MOSAIC_TASK_IDS.get(kind, MOSAIC_STATUS_TASK_ID)


def _mosaic_prefix(kind):
    return MOSAIC_PREFIXES.get(kind, MOSAIC_PREFIX)


def _mosaic_internal_action(kind):
    return MOSAIC_INTERNAL_ACTIONS.get(kind, MOSAIC_INTERNAL_ACTIONS["color"])


def _mosaic_status_item(status, *, kind="color"):
    item = {
        "job_id": {"S": MOSAIC_STATUS_JOB_ID},
        "task_id": {"S": _mosaic_task_id(kind)},
    }
    for key, value in status.items():
        if key in {"job_id", "task_id"}:
            continue
        item[key] = _ddb_value(value)
    return item


def _mosaic_status_from_item(item):
    if not item:
        return None
    status = {key: _plain_value(value) for key, value in item.items()}
    status.pop("job_id", None)
    status.pop("task_id", None)
    return _normalize_mosaic_status(status)


def _normalize_mosaic_status(status):
    status = dict(status or {})
    status.setdefault("schema_version", MOSAIC_STATUS_SCHEMA_VERSION)
    status.setdefault("state", "missing")
    status.setdefault("refresh_id", "")
    status.setdefault("started_at", "")
    status.setdefault("updated_at", "")
    status.setdefault("completed_at", "")
    status.setdefault("manifest_key", "")
    status.setdefault("manifest_url", "")
    status.setdefault("last_ready_manifest_key", "")
    status.setdefault("last_ready_manifest_url", "")
    status.setdefault("last_ready_completed_at", "")
    status.setdefault("last_ready_count", 0)
    status.setdefault("count", 0)
    status.setdefault("source_counts", {})
    status.setdefault("progress_stage", "")
    status.setdefault("progress_message", "")
    status.setdefault("progress_jobs_done", 0)
    status.setdefault("progress_jobs_total", 0)
    status.setdefault("progress_artifacts_done", 0)
    status.setdefault("progress_artifacts_total", 0)
    status.setdefault("progress_tiles", 0)
    status.setdefault("progress_last_job", "")
    status.setdefault("skipped_non_square", 0)
    status.setdefault("skipped_missing_preview", 0)
    status.setdefault("skipped_missing_image", 0)
    status.setdefault("skipped_missing_meta", 0)
    status.setdefault("skipped_bad_meta", 0)
    status.setdefault("skipped_legacy", 0)
    status.setdefault("unknown_dimensions", 0)
    # composite wall pyramid (deepzoom-speed.md §7.1): built async by the
    # deepzoom-export lambda after each refresh; "" = never attempted
    status.setdefault("wall_state", "")
    status.setdefault("wall_refresh_id", "")
    status.setdefault("wall_json_key", "")
    status.setdefault("wall_error", "")
    status.setdefault("error", "")
    status.setdefault("updated_at_ms", 0)
    return status


def _missing_mosaic_status():
    return _normalize_mosaic_status({"state": "missing"})


def _read_mosaic_status(kind="color", *, consistent=True):
    resp = _get_ddb().get_item(
        TableName=JOBS_TABLE,
        Key={
            "job_id": {"S": MOSAIC_STATUS_JOB_ID},
            "task_id": {"S": _mosaic_task_id(kind)},
        },
        ConsistentRead=consistent,
    )
    return _mosaic_status_from_item(resp.get("Item")) or _missing_mosaic_status()


def _put_mosaic_status(status, *, kind="color", condition_expression=None, expression_values=None, expression_names=None):
    kwargs = {
        "TableName": JOBS_TABLE,
        "Item": _mosaic_status_item(_normalize_mosaic_status(status), kind=kind),
    }
    if condition_expression:
        kwargs["ConditionExpression"] = condition_expression
        if expression_values:
            kwargs["ExpressionAttributeValues"] = {
                key: _ddb_value(value) for key, value in expression_values.items()
            }
        if expression_names:
            kwargs["ExpressionAttributeNames"] = expression_names
    _get_ddb().put_item(**kwargs)


def _is_conditional_failure(exc):
    return isinstance(exc, ClientError) and exc.response.get("Error", {}).get("Code") == "ConditionalCheckFailedException"


def _copy_last_ready_fields(existing):
    existing = existing or {}
    if existing.get("state") == "ready" and existing.get("manifest_key"):
        return {
            "last_ready_manifest_key": existing.get("manifest_key", ""),
            "last_ready_manifest_url": existing.get("manifest_url", ""),
            "last_ready_completed_at": existing.get("completed_at", ""),
            "last_ready_count": existing.get("count", 0),
        }
    return {
        "last_ready_manifest_key": existing.get("last_ready_manifest_key", ""),
        "last_ready_manifest_url": existing.get("last_ready_manifest_url", ""),
        "last_ready_completed_at": existing.get("last_ready_completed_at", ""),
        "last_ready_count": existing.get("last_ready_count", 0),
    }


def _start_mosaic_refresh(kind="color"):
    now_ms = _mosaic_now_ms()
    existing = _read_mosaic_status(kind, consistent=True)
    if (
        existing.get("state") == "computing"
        and int(existing.get("updated_at_ms") or 0) >= now_ms - MOSAIC_STATUS_STALE_MS
    ):
        return existing

    refresh_id = _mosaic_refresh_id()
    now_iso = _utc_now_iso()
    status = _normalize_mosaic_status({
        "state": "computing",
        "refresh_id": refresh_id,
        "started_at": now_iso,
        "updated_at": now_iso,
        "updated_at_ms": now_ms,
        **_copy_last_ready_fields(existing),
    })
    try:
        _put_mosaic_status(
            status,
            kind=kind,
            condition_expression=(
                "attribute_not_exists(job_id) OR "
                "#state <> :computing OR "
                "updated_at_ms < :stale_before"
            ),
            expression_names={"#state": "state"},
            expression_values={
                ":computing": "computing",
                ":stale_before": now_ms - MOSAIC_STATUS_STALE_MS,
            },
        )
    except ClientError as exc:
        if _is_conditional_failure(exc):
            return _read_mosaic_status(kind, consistent=True)
        raise

    try:
        boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1")).invoke(
            FunctionName=os.environ.get("AWS_LAMBDA_FUNCTION_NAME", "polypaint-storage"),
            InvocationType="Event",
            Payload=json.dumps({
                "internal_action": _mosaic_internal_action(kind),
                "refresh_id": refresh_id,
            }).encode("utf-8"),
        )
    except Exception as exc:
        error_status = _error_mosaic_status(refresh_id, status, exc)
        try:
            _put_owned_mosaic_status(error_status, refresh_id, kind=kind)
        except ClientError as put_exc:
            if not _is_conditional_failure(put_exc):
                raise
            return _read_mosaic_status(kind, consistent=True)
        return error_status
    return status


def _start_color_mosaic_refresh():
    return _start_mosaic_refresh("color")


def _start_palette_mosaic_refresh():
    return _start_mosaic_refresh("palette")


def _png_dimensions_from_header(data):
    if not isinstance(data, (bytes, bytearray)) or len(data) < 24:
        return None
    if bytes(data[:8]) != b"\x89PNG\r\n\x1a\n":
        return None
    if bytes(data[12:16]) != b"IHDR":
        return None
    width = int.from_bytes(data[16:20], "big")
    height = int.from_bytes(data[20:24], "big")
    if width <= 0 or height <= 0:
        return None
    return width, height


def _mosaic_preview_dimensions(key, *, s3_client=None):
    if not key:
        return None
    client = s3_client or s3
    # Generic width/height metadata belongs to some full-size images, not the
    # thumbnail itself. The PNG header is the authoritative preview size.
    try:
        obj = client.get_object(Bucket=BUCKET, Key=key, Range="bytes=0-32")
        data = obj["Body"].read()
        return _png_dimensions_from_header(data)
    except Exception as exc:
        if not _is_missing_s3_error(exc):
            raise
        return None


def _mosaic_job_id_from_prefix(prefix):
    parts = str(prefix or "").split("/")
    if len(parts) < 2 or parts[0] != "renders":
        return ""
    return parts[1]


def _mosaic_refresh_id_from_manifest_key(key):
    parts = [p for p in str(key or "").split("/") if p]
    if len(parts) < 4 or parts[0] != "renders" or parts[1] != "_index":
        return ""
    if parts[-1] == "all.json":
        return parts[-2]
    return parts[-1]


def _list_mosaic_job_ids(client):
    job_ids = []
    paginator = client.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix="renders/", Delimiter="/"):
        for prefix in page.get("CommonPrefixes", []):
            job_id = _mosaic_job_id_from_prefix(prefix.get("Prefix", ""))
            if job_id and not job_id.startswith("_"):
                job_ids.append(job_id)
    return job_ids


def _read_mosaic_calc_meta(client, job_id):
    try:
        obj = client.get_object(Bucket=BUCKET, Key=f"renders/{job_id}/calc.json")
    except Exception as exc:
        if not _is_missing_s3_error(exc):
            raise
        calc = {}
    else:
        try:
            calc = json.loads(obj["Body"].read())
        except (json.JSONDecodeError, TypeError, UnicodeDecodeError):
            calc = {}
        if not isinstance(calc, dict):
            calc = {}
    return {
        "function": calc.get("function", "?"),
        "degree": calc.get("degree", 0),
        "N": calc.get("N", calc.get("n1", 0)),
        "times": calc.get("times", 1),
    }


def _mosaic_tile_from_entry(client, job_id, entry, calc_meta):
    preview_key = entry.get("preview_key")
    if not preview_key:
        return None, "missing_preview"
    # Migrated artifacts carry a preview.jpg sibling plus its dims in the
    # meta (deepzoom-speed.md §2.2): the wall serves the jpg and skips the
    # per-artifact ranged-GET header read. Everything else falls back to the
    # png + measured dims, so a half-migrated wall stays correct throughout.
    jpg_key = str(entry.get("preview_jpg_key") or "").strip()
    jpg_width = _parse_int(entry.get("preview_jpg_width"))
    jpg_height = _parse_int(entry.get("preview_jpg_height"))
    if jpg_key and jpg_width and jpg_height:
        if jpg_width != jpg_height:
            return None, "non_square"
        tile_key, width, height = jpg_key, jpg_width, jpg_height
        dims = (width, height)
    else:
        tile_key = preview_key
        dims = _mosaic_preview_dimensions(preview_key, s3_client=client)
        if dims is None:
            width = height = None
        else:
            width, height = dims
            if width != height:
                return None, "non_square"
    tile = {
        "key": tile_key,
        "job_id": job_id,
        "artifact_id": entry.get("artifact_id", ""),
        "created_at": entry.get("created_at", ""),
        "function": calc_meta.get("function", "?"),
        "degree": calc_meta.get("degree", 0),
        "N": calc_meta.get("N", 0),
        "times": calc_meta.get("times", 1),
        "preview_width": width,
        "preview_height": height,
        "image_key": entry.get("image_key", ""),
        "palette": entry.get("palette", ""),
        "palette_display_name": entry.get("palette_display_name", ""),
    }
    return tile, "unknown" if dims is None else f"{width}x{height}"


def _build_color_mosaic_manifest(refresh_id, *, progress_cb=None):
    import concurrent.futures

    client = _results_list_s3_client(MOSAIC_WORKERS)
    job_ids = _list_mosaic_job_ids(client)
    job_total = len(job_ids)
    calc_by_job = {}
    work = []
    if progress_cb:
        progress_cb(
            stage="jobs",
            message=f"Scanning jobs: 0/{job_total}",
            jobs_done=0,
            jobs_total=job_total,
        )

    def read_job(job_id):
        calc = _read_mosaic_calc_meta(client, job_id)
        prefixes = _render_family_artifact_prefixes(job_id, "color", s3_client=client)
        legacy = bool(_legacy_render_variant(job_id, "color", presign=False, s3_client=client))
        return job_id, calc, prefixes, legacy

    with concurrent.futures.ThreadPoolExecutor(max_workers=min(MOSAIC_WORKERS, max(1, len(job_ids)))) as pool:
        legacy_count = 0
        for idx, (job_id, calc, prefixes, legacy) in enumerate(pool.map(read_job, job_ids), start=1):
            calc_by_job[job_id] = calc
            if legacy:
                legacy_count += 1
            for prefix in prefixes:
                work.append((job_id, prefix))
            if progress_cb and (idx % MOSAIC_PROGRESS_JOB_INTERVAL == 0 or idx == job_total):
                progress_cb(
                    stage="jobs",
                    message=f"Scanning jobs: {idx}/{job_total}; found {len(work)} color artifacts",
                    jobs_done=idx,
                    jobs_total=job_total,
                    artifacts_total=len(work),
                    last_job=job_id,
                )

    counts = {
        "skipped_non_square": 0,
        "skipped_missing_preview": 0,
        "skipped_missing_image": 0,
        "skipped_legacy": 0,
        "unknown_dimensions": 0,
    }
    source_counts = {}
    size_counts = {}
    tiles = []
    artifact_total = len(work)
    if progress_cb:
        progress_cb(
            stage="artifacts",
            message=f"Reading previews: 0/{artifact_total}",
            jobs_done=job_total,
            jobs_total=job_total,
            artifacts_done=0,
            artifacts_total=artifact_total,
        )

    def read_artifact(item):
        job_id, prefix = item
        entry = _render_family_entry_from_prefix(
            job_id,
            "color",
            prefix,
            presign=False,
            s3_client=client,
        )
        if not entry:
            return None, "missing_image"
        return _mosaic_tile_from_entry(client, job_id, entry, calc_by_job.get(job_id, {}))

    with concurrent.futures.ThreadPoolExecutor(max_workers=min(MOSAIC_WORKERS, max(1, len(work)))) as pool:
        for idx, (tile, status) in enumerate(pool.map(read_artifact, work), start=1):
            if tile:
                tiles.append(tile)
                source_counts[status] = source_counts.get(status, 0) + 1
                if status == "unknown":
                    counts["unknown_dimensions"] += 1
                width = tile.get("preview_width")
                height = tile.get("preview_height")
                if isinstance(width, int) and width > 0 and width == height:
                    size_counts[width] = size_counts.get(width, 0) + 1
            elif status == "missing_image":
                counts["skipped_missing_image"] += 1
            elif status == "missing_preview":
                counts["skipped_missing_preview"] += 1
            elif status == "non_square":
                counts["skipped_non_square"] += 1
            if progress_cb and (idx % MOSAIC_PROGRESS_ARTIFACT_INTERVAL == 0 or idx == artifact_total):
                progress_cb(
                    stage="artifacts",
                    message=f"Reading previews: {idx}/{artifact_total}; kept {len(tiles)} tiles",
                    jobs_done=job_total,
                    jobs_total=job_total,
                    artifacts_done=idx,
                    artifacts_total=artifact_total,
                    tiles=len(tiles),
                )

    counts["skipped_legacy"] = legacy_count
    if progress_cb:
        progress_cb(
            stage="manifest",
            message=f"Writing manifest: {len(tiles)} tiles",
            jobs_done=job_total,
            jobs_total=job_total,
            artifacts_done=artifact_total,
            artifacts_total=artifact_total,
            tiles=len(tiles),
        )

    tiles.sort(key=lambda t: (str(t.get("job_id") or ""), str(t.get("artifact_id") or "")))
    tiles.sort(key=lambda t: str(t.get("created_at") or ""), reverse=True)
    manifest_key = f"{_mosaic_prefix('color')}{refresh_id}/all.json"
    return {
        "schema_version": 1,
        "manifest_type": "artifact_mosaic",
        "artifact_kind": "color",
        "computed_at": _utc_now_iso(),
        "base": MOSAIC_BASE_URL,
        "refresh_id": refresh_id,
        "manifest_key": manifest_key,
        "manifest_kind": "all",
        "dimension_filter": "all-square",
        "tile_size": 512,
        "sizes": sorted(size_counts),
        "size_counts": {str(k): v for k, v in sorted(size_counts.items())},
        "count": len(tiles),
        "source_counts": source_counts,
        **counts,
        "tiles": tiles,
    }


def _build_palette_mosaic_manifest(refresh_id, *, progress_cb=None):
    import concurrent.futures

    client = _results_list_s3_client(MOSAIC_WORKERS)
    job_ids = _list_mosaic_job_ids(client)
    job_total = len(job_ids)
    calc_by_job = {}
    work = []
    if progress_cb:
        progress_cb(
            stage="jobs",
            message=f"Scanning jobs: 0/{job_total}",
            jobs_done=0,
            jobs_total=job_total,
        )

    def read_job(job_id):
        calc = _read_mosaic_calc_meta(client, job_id)
        prefixes = _palette_artifact_prefixes(job_id, s3_client=client)
        return job_id, calc, prefixes

    with concurrent.futures.ThreadPoolExecutor(max_workers=min(MOSAIC_WORKERS, max(1, len(job_ids)))) as pool:
        for idx, (job_id, calc, prefixes) in enumerate(pool.map(read_job, job_ids), start=1):
            calc_by_job[job_id] = calc
            for prefix in prefixes:
                work.append((job_id, prefix))
            if progress_cb and (idx % MOSAIC_PROGRESS_JOB_INTERVAL == 0 or idx == job_total):
                progress_cb(
                    stage="jobs",
                    message=f"Scanning jobs: {idx}/{job_total}; found {len(work)} palette artifacts",
                    jobs_done=idx,
                    jobs_total=job_total,
                    artifacts_total=len(work),
                    last_job=job_id,
                )

    counts = {
        "skipped_non_square": 0,
        "skipped_missing_preview": 0,
        "skipped_missing_image": 0,
        "skipped_missing_meta": 0,
        "skipped_bad_meta": 0,
        "unknown_dimensions": 0,
    }
    source_counts = {}
    size_counts = {}
    tiles = []
    artifact_total = len(work)
    if progress_cb:
        progress_cb(
            stage="artifacts",
            message=f"Reading palette previews: 0/{artifact_total}",
            jobs_done=job_total,
            jobs_total=job_total,
            artifacts_done=0,
            artifacts_total=artifact_total,
        )

    def read_artifact(item):
        job_id, prefix = item
        try:
            entry = _palette_entry_from_prefix(
                job_id,
                prefix,
                presign=False,
                s3_client=client,
                strict=True,
            )
        except (json.JSONDecodeError, TypeError, UnicodeDecodeError, ValueError):
            return None, "bad_meta"
        if not entry:
            return None, "missing_meta"
        image_key = entry.get("image_key")
        preview_key = entry.get("preview_key")
        head_results = _head_artifact_keys([image_key, preview_key], presign=False, s3_client=client)
        image_info = _first_existing(head_results, [image_key])
        preview_info = _first_existing(head_results, [preview_key])
        if not image_info:
            return None, "missing_image"
        if not preview_info:
            return None, "missing_preview"
        if not str(entry.get("created_at") or "").strip():
            entry["created_at"] = preview_info.get("modified_at") or image_info.get("modified_at") or ""
        tile, status = _mosaic_tile_from_entry(client, job_id, entry, calc_by_job.get(job_id, {}))
        if tile:
            tile.update({
                "palette_id": entry.get("palette_id", ""),
                "metric": entry.get("metric", ""),
                "palette": entry.get("palette", ""),
                "palette_display_name": entry.get("palette_display_name", ""),
                "render_reusable": bool(entry.get("render_reusable")),
                "data_layout": entry.get("data_layout", ""),
                "color_interpretation": entry.get("color_interpretation", ""),
                "score_output_channel_count": entry.get("score_output_channel_count", 1),
                "derived_from_palette_id": entry.get("derived_from_palette_id", ""),
                # F17: the exact color artifact this palette derives from, so the
                # AllPal book action never guesses ownership from the display id.
                "derived_from_color_artifact_id": entry.get("derived_from_color_artifact_id", ""),
            })
        return tile, status

    with concurrent.futures.ThreadPoolExecutor(max_workers=min(MOSAIC_WORKERS, max(1, len(work)))) as pool:
        for idx, (tile, status) in enumerate(pool.map(read_artifact, work), start=1):
            if tile:
                tiles.append(tile)
                source_counts[status] = source_counts.get(status, 0) + 1
                if status == "unknown":
                    counts["unknown_dimensions"] += 1
                width = tile.get("preview_width")
                height = tile.get("preview_height")
                if isinstance(width, int) and width > 0 and width == height:
                    size_counts[width] = size_counts.get(width, 0) + 1
            elif status == "missing_meta":
                counts["skipped_missing_meta"] += 1
            elif status == "bad_meta":
                counts["skipped_bad_meta"] += 1
            elif status == "missing_image":
                counts["skipped_missing_image"] += 1
            elif status == "missing_preview":
                counts["skipped_missing_preview"] += 1
            elif status == "non_square":
                counts["skipped_non_square"] += 1
            if progress_cb and (idx % MOSAIC_PROGRESS_ARTIFACT_INTERVAL == 0 or idx == artifact_total):
                progress_cb(
                    stage="artifacts",
                    message=f"Reading palette previews: {idx}/{artifact_total}; kept {len(tiles)} tiles",
                    jobs_done=job_total,
                    jobs_total=job_total,
                    artifacts_done=idx,
                    artifacts_total=artifact_total,
                    tiles=len(tiles),
                )

    if progress_cb:
        progress_cb(
            stage="manifest",
            message=f"Writing palette manifest: {len(tiles)} tiles",
            jobs_done=job_total,
            jobs_total=job_total,
            artifacts_done=artifact_total,
            artifacts_total=artifact_total,
            tiles=len(tiles),
        )

    tiles.sort(key=lambda t: (str(t.get("job_id") or ""), str(t.get("palette_id") or t.get("artifact_id") or "")))
    tiles.sort(key=lambda t: str(t.get("created_at") or ""), reverse=True)
    manifest_key = f"{_mosaic_prefix('palette')}{refresh_id}/all.json"
    return {
        "schema_version": 1,
        "manifest_type": "artifact_mosaic",
        "artifact_kind": "palette",
        "computed_at": _utc_now_iso(),
        "base": MOSAIC_BASE_URL,
        "refresh_id": refresh_id,
        "manifest_key": manifest_key,
        "manifest_kind": "all",
        "dimension_filter": "preview-size",
        "tile_size": 512,
        "sizes": sorted(size_counts),
        "size_counts": {str(k): v for k, v in sorted(size_counts.items())},
        "count": len(tiles),
        "source_counts": source_counts,
        **counts,
        "tiles": tiles,
    }


def _build_mosaic_manifest(kind, refresh_id, *, progress_cb=None):
    if kind == "palette":
        return _build_palette_mosaic_manifest(refresh_id, progress_cb=progress_cb)
    return _build_color_mosaic_manifest(refresh_id, progress_cb=progress_cb)


def _delete_s3_prefix(client, prefix):
    objects = []
    paginator = client.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        objects.extend({"Key": obj["Key"]} for obj in page.get("Contents", []))
    deleted = 0
    for i in range(0, len(objects), 1000):
        batch = objects[i:i + 1000]
        if not batch:
            continue
        resp = client.delete_objects(Bucket=BUCKET, Delete={"Objects": batch})
        deleted += len(resp.get("Deleted", batch))
    return deleted


def _prune_mosaic_manifests(kind="color", *, keep_refresh_ids):
    client = _results_list_s3_client(8)
    prefixes = []
    paginator = client.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=_mosaic_prefix(kind), Delimiter="/"):
        prefixes.extend(p.get("Prefix", "") for p in page.get("CommonPrefixes", []))
    by_refresh = {}
    for prefix in prefixes:
        refresh_id = _mosaic_refresh_id_from_manifest_key(prefix)
        if refresh_id:
            by_refresh[refresh_id] = prefix
    keep = set(keep_refresh_ids or [])
    keep.update(sorted(by_refresh.keys(), reverse=True)[:MOSAIC_KEEP_LAST])
    deleted = 0
    for refresh_id, prefix in by_refresh.items():
        if refresh_id in keep:
            continue
        deleted += _delete_s3_prefix(client, prefix)
    return deleted


def _prune_color_mosaic_manifests(*, keep_refresh_ids):
    return _prune_mosaic_manifests("color", keep_refresh_ids=keep_refresh_ids)


def _ready_mosaic_status(refresh_id, manifest, existing):
    now_iso = _utc_now_iso()
    now_ms = _mosaic_now_ms()
    manifest_key = manifest.get("manifest_key", "")
    manifest_url = _s3_public_url(manifest_key)
    status = _normalize_mosaic_status({
        "state": "ready",
        "refresh_id": refresh_id,
        "started_at": existing.get("started_at", now_iso),
        "updated_at": now_iso,
        "updated_at_ms": now_ms,
        "completed_at": now_iso,
        "manifest_key": manifest_key,
        "manifest_url": manifest_url,
        "last_ready_manifest_key": manifest_key,
        "last_ready_manifest_url": manifest_url,
        "last_ready_completed_at": now_iso,
        "last_ready_count": manifest.get("count", 0),
        "count": manifest.get("count", 0),
        "source_counts": manifest.get("source_counts", {}),
        "skipped_non_square": manifest.get("skipped_non_square", 0),
        "skipped_missing_preview": manifest.get("skipped_missing_preview", 0),
        "skipped_missing_image": manifest.get("skipped_missing_image", 0),
        "skipped_missing_meta": manifest.get("skipped_missing_meta", 0),
        "skipped_bad_meta": manifest.get("skipped_bad_meta", 0),
        "skipped_legacy": manifest.get("skipped_legacy", 0),
        "unknown_dimensions": manifest.get("unknown_dimensions", 0),
    })
    return status


def _error_mosaic_status(refresh_id, existing, exc):
    now_iso = _utc_now_iso()
    status = _normalize_mosaic_status({
        "state": "error",
        "refresh_id": refresh_id,
        "started_at": existing.get("started_at", now_iso),
        "updated_at": now_iso,
        "updated_at_ms": _mosaic_now_ms(),
        "completed_at": now_iso,
        "error": f"{type(exc).__name__}: {exc}"[:500],
        **_copy_last_ready_fields(existing),
    })
    return status


def _progress_mosaic_status(
    refresh_id,
    existing,
    *,
    stage,
    message,
    jobs_done=0,
    jobs_total=0,
    artifacts_done=0,
    artifacts_total=0,
    tiles=0,
    last_job="",
):
    now_iso = _utc_now_iso()
    return _normalize_mosaic_status({
        "state": "computing",
        "refresh_id": refresh_id,
        "started_at": existing.get("started_at", now_iso),
        "updated_at": now_iso,
        "updated_at_ms": _mosaic_now_ms(),
        "progress_stage": str(stage or ""),
        "progress_message": str(message or "")[:240],
        "progress_jobs_done": int(jobs_done or 0),
        "progress_jobs_total": int(jobs_total or 0),
        "progress_artifacts_done": int(artifacts_done or 0),
        "progress_artifacts_total": int(artifacts_total or 0),
        "progress_tiles": int(tiles or 0),
        "progress_last_job": str(last_job or "")[:120],
        **_copy_last_ready_fields(existing),
    })


def _put_owned_mosaic_status(status, refresh_id, *, kind="color"):
    _put_mosaic_status(
        status,
        kind=kind,
        condition_expression="refresh_id = :refresh_id AND #state = :computing",
        expression_names={"#state": "state"},
        expression_values={
            ":refresh_id": refresh_id,
            ":computing": "computing",
        },
    )


def _kick_wall_pyramid_build(kind, refresh_id, manifest_key):
    """deepzoom-speed.md §7.1: chain the composite wall build to the
    deepzoom-export lambda (libvips + wall_dz live there; storage stays pure
    Python). Best-effort — the manifest is already ready and the wall falls
    back to the per-tile grid, so a failed kick only marks wall_state=error."""
    try:
        boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1")).invoke(
            FunctionName=os.environ.get("DEEPZOOM_EXPORT_FUNCTION", "polypaint-deepzoom-export"),
            InvocationType="Event",
            Payload=json.dumps({
                "internal_action": "build_wall_pyramid",
                "kind": kind,
                "refresh_id": refresh_id,
                "manifest_key": manifest_key,
            }).encode("utf-8"),
        )
    except Exception as exc:  # noqa: BLE001
        try:
            _get_ddb().update_item(
                TableName=JOBS_TABLE,
                Key={"job_id": {"S": MOSAIC_STATUS_JOB_ID},
                     "task_id": {"S": _mosaic_task_id(kind)}},
                UpdateExpression="SET wall_state = :ws, wall_error = :we",
                ConditionExpression="refresh_id = :rid",
                ExpressionAttributeValues={
                    ":ws": {"S": "error"},
                    ":we": {"S": str(exc)[:512]},
                    ":rid": {"S": refresh_id},
                },
            )
        except Exception:
            pass


def _run_mosaic_worker(kind, refresh_id):
    existing = _read_mosaic_status(kind, consistent=True)

    def publish_progress(**kwargs):
        _put_owned_mosaic_status(_progress_mosaic_status(refresh_id, existing, **kwargs), refresh_id, kind=kind)

    try:
        manifest = _build_mosaic_manifest(kind, refresh_id, progress_cb=publish_progress)
        manifest_key = manifest["manifest_key"]
        s3.put_object(
            Bucket=BUCKET,
            Key=manifest_key,
            Body=json.dumps(manifest, separators=(",", ":"), ensure_ascii=False).encode("utf-8"),
            ContentType="application/json",
            CacheControl="no-cache, max-age=0",
        )
        status = _ready_mosaic_status(refresh_id, manifest, existing)
        status["wall_state"] = "computing"
        status["wall_refresh_id"] = refresh_id
        _put_owned_mosaic_status(status, refresh_id, kind=kind)
        _kick_wall_pyramid_build(kind, refresh_id, manifest_key)
        try:
            previous_refresh_id = _mosaic_refresh_id_from_manifest_key(existing.get("last_ready_manifest_key") or existing.get("manifest_key") or "")
            keep = {refresh_id}
            if previous_refresh_id:
                keep.add(previous_refresh_id)
            _prune_mosaic_manifests(kind, keep_refresh_ids=keep)
        except Exception:
            pass
        return status
    except Exception as exc:
        error_status = _error_mosaic_status(refresh_id, existing, exc)
        try:
            _put_owned_mosaic_status(error_status, refresh_id, kind=kind)
        except ClientError as put_exc:
            if not _is_conditional_failure(put_exc):
                raise
        return error_status


def _run_color_mosaic_worker(refresh_id):
    return _run_mosaic_worker("color", refresh_id)


def _run_palette_mosaic_worker(refresh_id):
    return _run_mosaic_worker("palette", refresh_id)


def _handle_list_mosaic(event, kind):
    params = parse_body(event)
    refresh = parse_boolish(params.get("refresh"), False)
    if refresh:
        return ok_response(_start_mosaic_refresh(kind))
    return ok_response(_read_mosaic_status(kind, consistent=True))


def handle_list_color_mosaic(event):
    return _handle_list_mosaic(event, "color")


def handle_list_palette_mosaic(event):
    return _handle_list_mosaic(event, "palette")


def _normalize_mosaic_share_kind(value):
    kind = str(value or "color").strip().lower()
    if kind in ("allcol", "col"):
        kind = "color"
    if kind in ("allpal", "pal"):
        kind = "palette"
    if kind not in MOSAIC_TASK_IDS:
        raise ValueError("share-mosaic kind must be color or palette")
    return kind


def _normalize_mosaic_share_size(value):
    raw = str(value or "all").strip().lower()
    if not raw or raw == "all":
        return "all"
    try:
        size = int(float(raw))
    except (TypeError, ValueError):
        raise ValueError("share-mosaic size must be all or a positive integer")
    if size <= 0:
        raise ValueError("share-mosaic size must be all or a positive integer")
    return str(size)


def _normalize_mosaic_share_sort(value):
    sort = str(value or "date").strip()
    if not sort:
        sort = "date"
    if sort not in MOSAIC_SORT_MODES:
        raise ValueError(f"share-mosaic sort must be one of {', '.join(sorted(MOSAIC_SORT_MODES))}")
    return sort


def _normalize_mosaic_share_cols(value):
    raw = str(value or "").strip()
    if not raw:
        return ""
    try:
        cols = int(float(raw))
    except (TypeError, ValueError):
        raise ValueError("share-mosaic cols must be blank or a positive integer")
    if cols <= 0:
        raise ValueError("share-mosaic cols must be blank or a positive integer")
    return str(cols)


def _mosaic_manifest_key_for_share(kind):
    status = _read_mosaic_status(kind, consistent=True)
    manifest_key = str(status.get("manifest_key") or "").strip()
    if status.get("state") != "ready":
        manifest_key = str(status.get("last_ready_manifest_key") or manifest_key).strip()
    if not manifest_key:
        raise ValueError(f"No ready {kind} mosaic manifest to share; refresh the mosaic first")
    return status, manifest_key


def _load_mosaic_manifest_for_share(manifest_key, kind):
    obj = s3.get_object(Bucket=BUCKET, Key=manifest_key)
    raw = obj["Body"].read()
    manifest = json.loads(raw or b"{}")
    if not isinstance(manifest, dict) or not isinstance(manifest.get("tiles"), list):
        raise ValueError("Mosaic manifest is malformed")
    if manifest.get("manifest_type") != "artifact_mosaic":
        raise ValueError("Mosaic manifest is not an artifact mosaic")
    artifact_kind = str(manifest.get("artifact_kind") or kind)
    if artifact_kind != kind:
        raise ValueError(f"Mosaic manifest kind mismatch: expected {kind}, got {artifact_kind}")
    return manifest


def handle_share_mosaic(event):
    params = parse_body(event)
    kind = _normalize_mosaic_share_kind(params.get("kind"))
    size = _normalize_mosaic_share_size(params.get("size"))
    sort = _normalize_mosaic_share_sort(params.get("sort"))
    cols = _normalize_mosaic_share_cols(params.get("cols"))
    status, manifest_key = _mosaic_manifest_key_for_share(kind)
    manifest = _load_mosaic_manifest_for_share(manifest_key, kind)

    now_iso = _utc_now_iso()
    share_id = f"share_{int(time.time() * 1000)}_{uuid.uuid4().hex[:8]}"
    snapshot_key = f"{MOSAIC_SHARE_PREFIX}{kind}/{share_id}/manifest.json"
    snapshot_manifest = dict(manifest)
    snapshot_manifest.update({
        "shared_at": now_iso,
        "share_id": share_id,
        "share_kind": kind,
        "source_manifest_key": manifest_key,
        # point manifest_key at THIS immutable snapshot, not the moving _index
        # manifest it was copied from (code-review-26 F14)
        "manifest_key": snapshot_key,
    })
    s3.put_object(
        Bucket=BUCKET,
        Key=snapshot_key,
        Body=json.dumps(snapshot_manifest, separators=(",", ":"), ensure_ascii=False).encode("utf-8"),
        ContentType="application/json",
        CacheControl="no-cache, max-age=0",
    )

    manifest_url = _s3_public_url(snapshot_key)
    query = {
        "kind": kind,
        "manifest": manifest_url,
        "size": size,
        "sort": sort,
    }
    if cols:
        query["cols"] = cols
    share_url = _s3_public_url(MOSAIC_SHARE_VIEWER_KEY) + "?" + urlencode(query)
    return ok_response({
        "kind": kind,
        "share_id": share_id,
        "share_key": snapshot_key,
        "manifest_key": snapshot_key,
        "manifest_url": manifest_url,
        "source_manifest_key": manifest_key,
        "share_url": share_url,
        "size": size,
        "sort": sort,
        "cols": cols,
        "count": snapshot_manifest.get("count", status.get("count", 0)),
    })


# ── Virtual gallery (virtual-gallery.md §13) ──────────────────────────────
# Two documents: an EDITABLE gallery the Gallery tab curates (S3 object with
# ETag CAS, like a book), and the IMMUTABLE share snapshot the standalone viewer
# loads (written by create-gallery-share, preserving sequence + titles).
GALLERY_SHARE_PREFIX = MOSAIC_SHARE_PREFIX + "gallery/"   # renders/_shared_mosaic/gallery/ (immutable shares)
GALLERIES_PREFIX = "polypaint/galleries/"                 # editable gallery documents
GALLERY_MAX_PIECES = 64
# Describe runs in a self-invoked storage worker (async — the interactive
# route only dispatches), so the budget is a worker wall-time bound, not an
# API-gateway fit. Leftover pieces are reported back, not attempted. The
# deadline is threaded into the provider layer (every attempt's socket timeout
# and retry sleep shrink to the remaining budget), and a reserve guarantees
# the worker always has time to write its terminal task status.
DESCRIBE_TIME_BUDGET_S = 240.0
DESCRIBE_STATUS_RESERVE_S = 10.0
GALLERY_SCHEMA_VERSION = 1
GALLERY_NAME_MAX = 120
GALLERY_TITLE_MAX = 160


def _read_deepzoom_export_meta(job_id, export_id, client):
    """Load a DeepZoom export's meta.json. Returns the dict, None if the export
    is genuinely absent; a transient/other S3 error propagates (CR28 F13)."""
    try:
        obj = client.get_object(Bucket=BUCKET, Key=f"deepzoom/{job_id}/{export_id}/meta.json")
    except Exception as exc:
        if is_missing_s3_error(exc):
            return None
        raise
    try:
        data = json.loads(obj["Body"].read())
    except (ValueError, TypeError):
        return None
    return data if isinstance(data, dict) else None


def _gallery_resolve_color_tile(job_id, artifact_id, calc_cache, *, client):
    """Enrich one gallery pick to a piece tile by EXACT key — HEAD the color
    image/preview candidates + overlay + calc.json (reusing the mosaic tile
    builder). No job scan. Returns (tile|None, state, reason): state is
    'ready' | 'missing' (genuinely absent — skip) | 'error' (transient — fail)."""
    shape = RENDER_FAMILY_SHAPES["color"]
    prefix = f"renders/{job_id}/color/{artifact_id}/"
    image_candidates = [prefix + k for k in shape["image_candidates"]]
    preview_candidates = [prefix + k for k in shape["preview_candidates"]]
    try:
        head = _head_artifact_keys(image_candidates + preview_candidates,
                                   presign=False, s3_client=client)
    except Exception as exc:
        return None, "error", s3_error_reason(exc)
    image_info = _first_existing(head, image_candidates)
    if not image_info:
        for key in image_candidates:
            reason = (head.get(key) or {}).get("error_reason")
            if reason:
                return None, "error", reason
        return None, "missing", "missing_image"
    preview_info = _first_existing(head, preview_candidates)
    try:
        overlay = _load_color_artifact_overlay(job_id, artifact_id, s3_client=client)
    except Exception as exc:
        return None, "error", s3_error_reason(exc)
    entry = _render_artifact_entry("color", artifact_id, image_info, preview_info,
                                   fallback_meta=overlay)
    if job_id not in calc_cache:
        try:
            calc_cache[job_id] = _read_mosaic_calc_meta(client, job_id)
        except Exception as exc:
            return None, "error", s3_error_reason(exc)
    tile, status = _mosaic_tile_from_entry(client, job_id, entry, calc_cache[job_id])
    if tile is None:
        return None, "missing", status  # missing_preview / non_square
    # The 3D viewer requires finite positive preview dims (aspect/layout math);
    # accepting a dimension-less tile here would let /add-to-gallery report
    # success for a piece the viewer then silently drops.
    if not tile.get("preview_width") or not tile.get("preview_height"):
        return None, "missing", "unknown_preview_dimensions"
    return tile, "ready", ""


def _export_viewer_exists(export_job_id, export_id, client):
    """Capability check (code-review-29 F5): exports older than 9a3c5d7 have no
    standalone viewer.html, yet remain curatable (has a DZI => curatable). The
    admitted piece records whether the viewer page exists so Go DeepZoom /
    Copy link can be truthful instead of opening a known-dead URL."""
    try:
        client.head_object(Bucket=BUCKET, Key=f"deepzoom/{export_job_id}/{export_id}/viewer.html")
        return True
    except Exception as exc:
        if is_missing_s3_error(exc):
            return False
        raise


def _validate_gallery_export(job_id, artifact_id, export_id, image_key, *, client, export_job_id=None):
    """Validate a caller-named DeepZoom export for a piece (virtual-gallery.md
    §3.1 step 4): exact identity match + canonical dzi_key + a live DZI. Returns
    (deepzoom_ref|None, reason). A transient error propagates."""
    export_job_id = export_job_id or job_id
    meta = _read_deepzoom_export_meta(export_job_id, export_id, client)
    if meta is None:
        return None, "export_not_found"
    # Identity is (job, color, artifact) — NOT the exact source file. An artifact
    # can carry both image.jpeg and image.png (png-export/resize variants), and
    # enrichment resolves the first existing candidate while the export may have
    # been generated from the other; both depict the same artifact. Requiring
    # exact source_key equality wrongly rejected those ("export_identity_mismatch").
    src = str(meta.get("source_key") or "")
    src_parts = src.split("/")
    src_matches = (len(src_parts) == 5 and src_parts[0] == "renders"
                   and src_parts[1] == job_id and src_parts[2] == "color"
                   and src_parts[3] == artifact_id)
    # Explicit identity fields are authoritative WHEN PRESENT — older export
    # metas predate them (source_family/source_artifact_id added later). For
    # those, the parsed source_key (required above) carries the identity; the
    # meta was already read from deepzoom/<job>/<export>/ so its location pins
    # job+export. An absent field must not read as a mismatched field.
    mexp = str(meta.get("export_id") or "")
    fam = str(meta.get("source_family") or "")
    if ((mexp and mexp != export_id)
            or (fam and fam != "color")
            or not src_matches
            or _export_identity_conflicts(meta, job_id, artifact_id, export_job_id)):
        return None, "export_identity_mismatch"
    dzi_key = f"deepzoom/{export_job_id}/{export_id}/image.dzi"
    if str(meta.get("dzi_key") or "") != dzi_key:
        return None, "export_dzi_key_mismatch"
    try:
        client.head_object(Bucket=BUCKET, Key=dzi_key)
    except Exception as exc:
        if is_missing_s3_error(exc):
            return None, "export_dzi_absent"
        raise
    return {"export_id": export_id, "dzi_key": dzi_key,
            "source_key": str(meta.get("source_key") or "") or image_key,   # true provenance
            "source_artifact_id": artifact_id,
            "viewer": _export_viewer_exists(export_job_id, export_id, client)}, ""


def _read_dzi_descriptor(export_job_id, export_id, client):
    """Parse + validate the export's image.dzi XML. Returns (desc, reason):
    desc = (format, tile_size, width, height) with reason "", or (None, reason).
    Validation matches what the viewer accepts: positive TileSize/Size and a
    jpeg/jpg/png tile format (anything else would 500 in the level math here or
    be silently dropped by the viewer's preview-tile allowlist later)."""
    try:
        obj = client.get_object(Bucket=BUCKET, Key=f"deepzoom/{export_job_id}/{export_id}/image.dzi")
        xml = obj["Body"].read().decode("utf-8", "replace")
    except Exception as exc:
        if is_missing_s3_error(exc):
            return None, "export_dzi_absent"
        raise
    fmt = re.search(r'Format="(\w+)"', xml)
    ts = re.search(r'TileSize="(\d+)"', xml)
    w = re.search(r'Width="(\d+)"', xml)
    h = re.search(r'Height="(\d+)"', xml)
    if not (fmt and ts and w and h):
        return None, "export_dzi_invalid"
    fmt_v = fmt.group(1).lower()
    ts_v, w_v, h_v = int(ts.group(1)), int(w.group(1)), int(h.group(1))
    if fmt_v not in ("jpeg", "jpg", "png") or ts_v <= 0 or w_v <= 0 or h_v <= 0:
        return None, "export_dzi_invalid"
    return (fmt_v, ts_v, w_v, h_v), ""


def _export_identity_conflicts(meta, job_id, artifact_id, export_job_id):
    """Cross-check an export meta against the caller-claimed identity. Explicit
    fields are authoritative WHEN PRESENT; absent fields (older metas) impose no
    constraint. A 5-part source_key must name the same render job + artifact
    (any family); legacy root-shaped source keys carry no identity. Returns
    "export_identity_mismatch" on contradiction, else None."""
    mjob = str(meta.get("job_id") or "")
    art = str(meta.get("source_artifact_id") or "")
    if mjob and mjob != export_job_id:
        return "export_identity_mismatch"
    if art and art != artifact_id:
        return "export_identity_mismatch"
    src = str(meta.get("source_key") or "")
    parts = src.split("/")
    if len(parts) == 5 and parts[0] == "renders":
        if parts[1] != job_id or parts[3] != artifact_id:
            return "export_identity_mismatch"
    return None


def _dzi_piece_from_export(job_id, artifact_id, export_id, calc_cache, *, client, export_job_id=None):
    """Build a gallery piece purely from a DeepZoom export — the user rule is
    "has a DZI => curatable". Preview = the largest single-tile pyramid level;
    zoom = the DZI (under the export OWNER's prefix); the original is linked only
    if the source object still exists AND is a key shape the viewer accepts.
    Identity is meta-gated: a meta that CONTRADICTS the claimed identity is
    terminal — this fallback covers legacy/absent fields, never mislabeling.
    Returns (piece|None, reason)."""
    import math
    export_job_id = export_job_id or job_id
    meta = _read_deepzoom_export_meta(export_job_id, export_id, client)
    if meta is None:
        return None, "export_not_found"
    conflict = _export_identity_conflicts(meta, job_id, artifact_id, export_job_id)
    if conflict:
        return None, conflict
    desc, desc_reason = _read_dzi_descriptor(export_job_id, export_id, client)
    if desc is None:
        return None, desc_reason
    fmt, tile_size, width, height = desc
    max_dim = max(width, height)
    max_level = max(0, math.ceil(math.log2(max_dim)) if max_dim > 1 else 0)
    off = math.ceil(math.log2(max_dim / tile_size)) if max_dim > tile_size else 0
    level = max(0, max_level - off)
    scale = 2 ** (max_level - level)
    pw, ph = math.ceil(width / scale), math.ceil(height / scale)
    preview_key = f"deepzoom/{export_job_id}/{export_id}/image_files/{level}/0_0.{fmt}"
    try:
        client.head_object(Bucket=BUCKET, Key=preview_key)
    except Exception as exc:
        if is_missing_s3_error(exc):
            return None, "export_preview_tile_missing"
        raise
    # Link the original only when it is still there AND its key is a shape the
    # standalone viewer accepts (renders/<job>/<family>/<artifact>/<leaf> naming
    # this artifact) — linking a legacy root-shaped key would make the viewer
    # drop the piece's original at share load. The RAW source_key still travels
    # in the deepzoom ref as opaque provenance.
    image_key = None
    src_key = str(meta.get("source_key") or "")
    src_parts = src_key.split("/")
    family = ""
    if len(src_parts) == 5 and src_parts[0] == "renders" and re.fullmatch(r"[a-z]{1,24}", src_parts[2] or ""):
        family = src_parts[2]
        if src_parts[3] == artifact_id:
            try:
                client.head_object(Bucket=BUCKET, Key=src_key)
                image_key = src_key
            except Exception as exc:
                if not is_missing_s3_error(exc):
                    raise
    if not family:
        fam_meta = str(meta.get("source_family") or "")
        family = fam_meta if re.fullmatch(r"[a-z]{1,24}", fam_meta) else ""
    if job_id not in calc_cache:
        try:
            calc_cache[job_id] = _read_mosaic_calc_meta(client, job_id)
        except Exception:
            calc_cache[job_id] = {}
    calc = calc_cache[job_id] or {}
    dzi_key = f"deepzoom/{export_job_id}/{export_id}/image.dzi"
    return {
        "job_id": job_id,
        "export_job_id": export_job_id,
        "family": family,
        "artifact_id": artifact_id,
        "preview_key": preview_key,
        "image_key": image_key,
        "preview_width": pw,
        "preview_height": ph,
        "function": calc.get("function", "?"),
        "degree": calc.get("degree", 0),
        "N": calc.get("N", 0),
        "times": calc.get("times", 1),
        "created_at": str(meta.get("created_at") or ""),
        "palette": str(meta.get("palette") or meta.get("source_palette") or ""),
        "palette_display_name": str(
            meta.get("palette_display_name")
            or meta.get("source_palette_display_name")
            or ""
        ),
        "deepzoom": {"export_id": export_id, "dzi_key": dzi_key,
                      "source_key": src_key or None,   # true provenance, opaque to the viewer
                      "source_artifact_id": artifact_id,
                      "viewer": _export_viewer_exists(export_job_id, export_id, client)},
    }, ""


def _enrich_gallery_pick(job_id, artifact_id, export_id, calc_cache, *, client, export_job_id=None):
    """Validate + enrich ONE gallery pick to a piece dict (no ordinal/title yet).
    Returns (piece|None, reason|None, fatal). Prefers the rich color-artifact
    path; ONLY when no color artifact exists at all does it fall back to building
    the piece from the export (rule: has a DZI => curatable, for classified
    legacy cases). When the color artifact resolves, export validation is
    authoritative — an explicit identity mismatch is TERMINAL, never fail-open."""
    export_job_id = export_job_id or job_id
    tile, state, reason = _gallery_resolve_color_tile(job_id, artifact_id, calc_cache, client=client)
    if state == "error":
        return None, reason, True
    if tile is None:
        if export_id:
            piece, dz_reason = _dzi_piece_from_export(
                job_id, artifact_id, export_id, calc_cache, client=client, export_job_id=export_job_id)
            if piece is not None:
                return piece, None, False
            return None, dz_reason or reason or "missing", False
        return None, reason or "missing", False
    deepzoom = None
    if export_id:
        dz, dz_reason = _validate_gallery_export(
            job_id, artifact_id, export_id, tile["image_key"], client=client, export_job_id=export_job_id)
        if dz is None:
            return None, dz_reason, False
        deepzoom = dz
    piece = {
        "job_id": job_id,
        "export_job_id": export_job_id,
        "family": "color",
        "artifact_id": artifact_id,
        "preview_key": tile["key"],
        "image_key": tile["image_key"],
        "preview_width": tile.get("preview_width"),
        "preview_height": tile.get("preview_height"),
        "function": tile.get("function", "?"),
        "degree": tile.get("degree", 0),
        "N": tile.get("N", 0),
        "times": tile.get("times", 1),
        "created_at": tile.get("created_at", ""),
        "palette": tile.get("palette", ""),
        "palette_display_name": tile.get("palette_display_name", ""),
        "deepzoom": deepzoom,
    }
    return piece, None, False


def _write_gallery_share_manifest(pieces, *, source_kind, seed, settings=None):
    """Write an immutable virtual_gallery SHARE manifest (the document the viewer
    loads) and return (public_url, key, share_id, count). `pieces` order is
    authoritative; ordinals are assigned here and curator titles carried through."""
    out_pieces = []
    for i, p in enumerate(pieces):
        piece = {k: p.get(k) for k in (
            "job_id", "export_job_id", "family", "artifact_id", "preview_key",
            "image_key", "preview_width",
            "preview_height", "function", "degree", "N", "times", "created_at",
            "palette", "palette_display_name", "deepzoom")}
        piece["ordinal"] = i
        title = str(p.get("title") or "").strip()
        if title:
            piece["title"] = title
        out_pieces.append(piece)
    # Truthful top-level kind (code-review-29 F3): DZI-fallback pieces can carry
    # any render family, and the viewer is told so instead of a hardcoded color.
    families = {str(p.get("family") or "") or "color" for p in pieces}
    artifact_kind = "color" if families <= {"color"} else "mixed"
    # SHORT id (the share link is gallery.html?share=<id>, the id is the whole
    # payload of the link) + CREATE-ONLY write (code-review-29 F6): the manifest
    # is served with an immutable cache header, so a colliding id must fail the
    # put and retry a fresh id — never overwrite an existing share.
    for _ in range(4):
        share_id = uuid.uuid4().hex[:10]
        snapshot_key = f"{GALLERY_SHARE_PREFIX}{share_id}/manifest.json"
        manifest = {
            "schema_version": GALLERY_SCHEMA_VERSION,
            "manifest_type": "virtual_gallery",
            "document_kind": "share",
            "artifact_kind": artifact_kind,
            "created_at": _utc_now_iso(),
            "share_id": share_id,
            "manifest_key": snapshot_key,
            "source": {"kind": source_kind, "share_id": share_id},
            "layout": {"mode": "auto", "seed": seed},
            "settings": _clean_gallery_settings(settings),
            "pieces": out_pieces,
        }
        try:
            s3.put_object(
                Bucket=BUCKET,
                Key=snapshot_key,
                Body=json.dumps(manifest, separators=(",", ":"), ensure_ascii=False).encode("utf-8"),
                ContentType="application/json",
                CacheControl=CACHE_IMMUTABLE,
                IfNoneMatch="*",
            )
        except Exception as exc:
            if s3_error_code(exc) in ("PreconditionFailed", "412"):
                continue   # collided with an existing share — mint a fresh id
            raise
        return _s3_public_url(snapshot_key), snapshot_key, share_id, len(out_pieces)
    raise RuntimeError("could not allocate a unique gallery share id — try again")


# ── Editable gallery documents (the Gallery tab curates these) ─────────────
class _GalleryNotFound(Exception):
    pass


class GalleryConflictError(Exception):
    """Optimistic-concurrency failure: the stored gallery moved under a caller
    that passed the revision it read (a concurrent Add/Save landed)."""


def _gallery_doc_key(gallery_id):
    return f"{GALLERIES_PREFIX}{gallery_id}.json"


def _new_gallery_id():
    return f"gallery_{int(time.time() * 1000)}_{uuid.uuid4().hex[:8]}"


def _clean_gallery_name(value):
    name = str(value or "").strip()
    if len(name) > GALLERY_NAME_MAX:
        name = name[:GALLERY_NAME_MAX].rstrip()
    return name or "Untitled gallery"


def _clean_gallery_title(value):
    """The one client-editable piece field on save — a curator title, clamped."""
    title = str(value or "").strip()
    if len(title) > GALLERY_TITLE_MAX:
        title = title[:GALLERY_TITLE_MAX].rstrip()
    return title


# 'stars' = procedural starfield, 'dark' = none; the rest are photographic
# equirect JPEGs published by deploy.sh under s3://<bucket>/skybox/<id>.jpg
# (converted from skybox/ source TIFF/EXR files).
GALLERY_SKY_MODES = ("stars", "dark", "galaxies", "milkyway", "moonlit")
_GALLERY_HEX = re.compile(r"^#[0-9a-fA-F]{6}$")


def _clean_gallery_settings(raw):
    """Scene settings the viewer applies: sky mode + wall colour (validated)."""
    raw = raw if isinstance(raw, dict) else {}
    sky = str(raw.get("sky") or "stars")
    if sky not in GALLERY_SKY_MODES:
        sky = "stars"
    wall = str(raw.get("wall_color") or "").strip()
    if not _GALLERY_HEX.match(wall):
        wall = "#ece4d6"
    coverage = _parse_int(raw.get("wall_coverage"))
    if coverage is None:
        coverage = 35
    coverage = max(5, min(100, coverage))
    # Self-tinted walls: the picked colour glows through the blue moonlight so
    # white reads white. Default ON; explicit false gives fully-lit walls.
    self_tint = raw.get("wall_self_tint")
    self_tint = True if self_tint is None else bool(self_tint)
    edge_px = _parse_int(raw.get("wall_edge_px"))
    if edge_px is None:
        edge_px = 1
    edge_px = max(0, min(12, edge_px))
    layout = str(raw.get("wall_layout") or "maze")
    if layout not in ("maze", "serpentine", "exhibition", "spiral",
                      "standalone", "standalone2", "standalone4"):
        layout = "maze"
    return {"sky": sky, "wall_color": wall.lower(), "wall_coverage": coverage,
            "wall_self_tint": self_tint, "wall_edge_px": edge_px, "wall_layout": layout}


def _new_gallery_doc(gallery_id, name, pieces=None):
    now = _utc_now_iso()
    return {
        "schema_version": GALLERY_SCHEMA_VERSION,
        "manifest_type": "virtual_gallery",
        "document_kind": "editable",
        "artifact_kind": "color",
        "gallery_id": gallery_id,
        "name": name,
        "created_at": now,
        "updated_at": now,
        "source": {"kind": "deepzoom_selection"},
        "layout": {"mode": "auto", "seed": 1},
        "settings": _clean_gallery_settings(None),
        "pieces": pieces or [],
    }


def _read_gallery_doc_with_etag(gallery_id):
    key = _gallery_doc_key(gallery_id)
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=key)
    except Exception as exc:
        if is_missing_s3_error(exc):
            raise _GalleryNotFound(f"gallery not found: {gallery_id}")
        raise
    payload = json.loads(obj["Body"].read())
    if str(payload.get("document_kind") or "") != "editable":
        raise ValueError(f"object at {key} is not an editable gallery")
    return payload, str(obj.get("ETag") or "").strip('"')


def _gallery_summary(doc):
    return {
        "gallery_id": doc.get("gallery_id"),
        "name": doc.get("name"),
        "count": len(doc.get("pieces") or []),
        "created_at": doc.get("created_at"),
        "updated_at": doc.get("updated_at"),
    }


def _put_gallery_doc(doc, *, expected_revision=None, create_only=False):
    """Write an editable gallery doc; return the new revision (ETag). Raises
    GalleryConflictError on a CAS failure (concurrent write)."""
    put_kwargs = dict(
        Bucket=BUCKET, Key=_gallery_doc_key(doc["gallery_id"]),
        Body=json.dumps(doc, separators=(",", ":"), ensure_ascii=False).encode("utf-8"),
        ContentType="application/json")
    if create_only:
        put_kwargs["IfNoneMatch"] = "*"
    elif expected_revision:
        put_kwargs["IfMatch"] = expected_revision
    try:
        resp = s3.put_object(**put_kwargs)
    except ClientError as exc:
        code = str((exc.response.get("Error") or {}).get("Code") or "")
        if code in ("PreconditionFailed", "412", "ConditionalRequestConflict"):
            raise GalleryConflictError(
                f"gallery {doc['gallery_id']} changed; refetch and retry")
        raise
    return str((resp or {}).get("ETag") or "").strip('"')


def handle_create_gallery(event):
    """Create a new empty editable gallery. Returns the doc + its CAS revision."""
    params = parse_body(event)
    name = _clean_gallery_name(params.get("name"))
    doc = _new_gallery_doc(_new_gallery_id(), name)
    revision = _put_gallery_doc(doc, create_only=True)
    return ok_response({"gallery": doc, "revision": revision})


def handle_list_galleries(event):
    """Panel-ready list of editable galleries (id, name, count, timestamps),
    newest-updated first."""
    del event
    galleries = []
    kwargs = {"Bucket": BUCKET, "Prefix": GALLERIES_PREFIX}
    while True:
        resp = s3.list_objects_v2(**kwargs)
        for item in resp.get("Contents") or []:
            key = item.get("Key") or ""
            if not key.endswith(".json"):
                continue
            gallery_id = key[len(GALLERIES_PREFIX):-len(".json")]
            if not gallery_id or "/" in gallery_id:
                continue
            try:
                obj = s3.get_object(Bucket=BUCKET, Key=key)
            except Exception as exc:
                if is_missing_s3_error(exc):
                    continue          # deleted between list and get — skip
                raise                 # throttling / 5xx / other transient — surface it
            try:
                doc = json.loads(obj["Body"].read())
            except (ValueError, TypeError):
                continue              # malformed object — skip deliberately
            if str(doc.get("document_kind") or "") != "editable":
                continue
            galleries.append(_gallery_summary(doc))
        if not resp.get("IsTruncated"):
            break
        kwargs["ContinuationToken"] = resp.get("NextContinuationToken")
    galleries.sort(key=lambda g: str(g.get("updated_at") or ""), reverse=True)
    return ok_response({"galleries": galleries, "count": len(galleries)})


def handle_fetch_gallery(event):
    params = parse_body(event)
    gallery_id = assert_safe_id(params.get("gallery_id") or params.get("id"), "gallery_id")
    doc, revision = _read_gallery_doc_with_etag(gallery_id)
    return ok_response({"gallery": doc, "revision": revision})


def handle_save_gallery(event):
    """Persist CURATION of an existing gallery: rename, reorder, retitle, remove.
    The gallery must already exist and the caller must pass the CAS revision it
    read. Piece IDENTITY, order, and title come from the client; every other
    piece field (keys, dims, function/degree/N/times, deepzoom) is PRESERVED from
    the stored document — a save cannot inject or rewrite server-owned data, and
    cannot ADD a piece (adds go through /add-to-gallery)."""
    params = parse_body(event)
    incoming = params.get("gallery")
    if not isinstance(incoming, dict):
        raise ValueError("save-gallery requires a gallery object")
    gallery_id = assert_safe_id(incoming.get("gallery_id"), "gallery_id")
    expected_revision = str(params.get("expected_revision") or "").strip()
    if not expected_revision:
        raise ValueError("save-gallery requires expected_revision (refetch the gallery)")
    # Must already exist — save never creates a gallery.
    existing, current = _read_gallery_doc_with_etag(gallery_id)   # _GalleryNotFound -> 404
    # Conflict check BEFORE piece validation: with a stale revision, the piece
    # set may legitimately differ (another browser removed a piece) — that must
    # surface as a 409 (client offers reload), not an "unknown piece" 400.
    if expected_revision != current:
        raise GalleryConflictError(
            f"gallery {gallery_id} changed since revision {expected_revision!r}; refetch and retry")
    stored_by_id = {(p.get("job_id"), p.get("family") or "color", p.get("artifact_id")): p
                    for p in (existing.get("pieces") or [])}

    raw_pieces = incoming.get("pieces")
    if not isinstance(raw_pieces, list):
        raise ValueError("gallery pieces must be a list")
    if len(raw_pieces) > GALLERY_MAX_PIECES:
        raise ValueError(f"too many pieces (max {GALLERY_MAX_PIECES})")
    pieces = []
    seen = set()
    for raw in raw_pieces:
        if not isinstance(raw, dict):
            raise ValueError("gallery piece must be an object")
        job_id = assert_safe_id(raw.get("job_id"), "job_id")
        artifact_id = assert_safe_id(raw.get("artifact_id"), "artifact_id")
        family = str(raw.get("family") or "") or "color"
        ident = (job_id, family, artifact_id)
        stored = stored_by_id.get(ident)
        if stored is None:
            raise ValueError(f"unknown piece {job_id}/{artifact_id}: add pieces via the DeepZoom tab")
        if ident in seen:
            continue
        seen.add(ident)
        piece = dict(stored)                                  # server-owned fields preserved
        piece["ordinal"] = len(pieces)                        # client order
        piece["title"] = _clean_gallery_title(raw.get("title"))  # client title (only)
        pieces.append(piece)

    doc = _new_gallery_doc(gallery_id, _clean_gallery_name(incoming.get("name")), pieces)
    doc["created_at"] = existing.get("created_at") or _utc_now_iso()
    doc["updated_at"] = _utc_now_iso()
    seed = _parse_int((incoming.get("layout") or {}).get("seed")) or (existing.get("layout") or {}).get("seed") or 1
    doc["layout"] = {"mode": "auto", "seed": seed}
    # Scene settings (sky, wall colour) are client-editable; fall back to the
    # stored ones when the client omits them.
    doc["settings"] = _clean_gallery_settings(incoming.get("settings") or existing.get("settings"))
    revision = _put_gallery_doc(doc, expected_revision=expected_revision)   # IfMatch -> 409
    return ok_response({"gallery": doc, "revision": revision})


def handle_delete_gallery(event):
    params = parse_body(event)
    gallery_id = assert_safe_id(params.get("gallery_id") or params.get("id"), "gallery_id")
    key = _gallery_doc_key(gallery_id)
    deleted = _key_exists(key)
    if deleted:
        s3.delete_object(Bucket=BUCKET, Key=key)
    return ok_response({"deleted": bool(deleted), "gallery_id": gallery_id})


def handle_add_to_gallery(event):
    """Append ONE color export to an editable gallery (the DeepZoom tab's only
    gallery action). Enriches + validates the single piece, dedups, and CAS-saves
    against the revision it read."""
    params = parse_body(event)
    gallery_id = assert_safe_id(params.get("gallery_id"), "gallery_id")
    job_id = assert_safe_id(params.get("job_id"), "job_id")
    artifact_id = assert_safe_id(params.get("artifact_id"), "artifact_id")
    export_id = params.get("export_id")
    export_id = assert_safe_id(export_id, "export_id") if export_id not in (None, "") else None
    # The export OWNER's job (deepzoom/<export_job_id>/<export_id>/...) can differ
    # from the render-source job (renders/<job_id>/...): carry both identities.
    export_job_id = params.get("export_job_id")
    export_job_id = assert_safe_id(export_job_id, "export_job_id") if export_job_id not in (None, "") else job_id

    doc, revision = _read_gallery_doc_with_etag(gallery_id)
    pieces = doc.get("pieces") or []

    client = _results_list_s3_client(1)
    piece, reason, fatal = _enrich_gallery_pick(
        job_id, artifact_id, export_id, {}, client=client, export_job_id=export_job_id)
    if piece is None:
        if fatal:
            return ok_response({"error": f"could not resolve {job_id}/{artifact_id}: {reason}"})
        return ok_response({"gallery": doc, "revision": revision, "added": False, "reason": reason})
    # Dedup on (job, family, artifact): color and non-color exports sharing ids
    # are distinct pieces. Legacy stored pieces without family count as color.
    ident = (job_id, piece.get("family") or "color", artifact_id)
    if any((p.get("job_id"), p.get("family") or "color", p.get("artifact_id")) == ident for p in pieces):
        return ok_response({"gallery": doc, "revision": revision, "added": False, "reason": "duplicate"})
    if len(pieces) >= GALLERY_MAX_PIECES:
        return ok_response({"gallery": doc, "revision": revision, "added": False, "reason": "gallery_full"})
    piece["ordinal"] = len(pieces)
    piece["title"] = ""
    pieces.append(piece)
    doc["pieces"] = pieces
    doc["updated_at"] = _utc_now_iso()
    new_revision = _put_gallery_doc(doc, expected_revision=revision)
    return ok_response({"gallery": doc, "revision": new_revision, "added": True,
                        "job_id": job_id, "artifact_id": artifact_id})


def _gallery_title_from_reply(raw):
    """Extract {"title": ...} from a vision reply (same tolerant scan the book
    engine uses: first complete JSON object wins)."""
    idx = raw.find("{")
    while idx != -1:
        for end in range(len(raw), idx, -1):
            try:
                data = json.loads(raw[idx:end])
                break
            except ValueError:
                continue
        else:
            data = None
        if isinstance(data, dict) and str(data.get("title") or "").strip():
            return str(data["title"]).strip()
        idx = raw.find("{", idx + 1)
    raise RuntimeError(f"no title in vision reply: {raw[:160]!r}")


def handle_describe_gallery(event):
    """Dispatch a describe run: vision I/O CANNOT live in the request path (a
    provider read can outlast the API gateway window — "The read operation
    timed out"), so this route only validates, writes a task row, and
    self-invokes the storage worker (the mosaic-build pattern). The client
    polls /check-status on the returned task and refetches the gallery."""
    from book_describe import _load_vision_config
    from shared import vision_provider
    params = parse_body(event)
    gallery_id = assert_safe_id(params.get("gallery_id"), "gallery_id")
    if not isinstance(params.get("pieces"), (list, type(None))):
        raise ValueError("pieces must be a list when present")
    # The ownership contract is REQUIRED, not advisory (CR30 follow-up F6): a caller
    # omitting base_title would let a generated title overwrite a human edit
    # made between dispatch and worker start.
    if not str(params.get("expected_revision") or "").strip():
        raise ValueError("describe-gallery requires expected_revision (refetch the gallery)")
    for t in (params.get("pieces") or []):
        if not isinstance(t, dict) or "base_title" not in t:
            raise ValueError("each explicit describe target requires base_title "
                             "(the title as reviewed at dispatch time)")
    # Fail fast on missing config — no point dispatching a doomed worker.
    cfg = _load_vision_config()
    model = str(cfg.get("model") or "") or "gemini-2.5-flash"
    prov = vision_provider(model)
    api_key = cfg.get(f"api_key_{prov}") or (os.environ.get("GEMINI_API_KEY", "") if prov == "gemini" else "")
    if not api_key:
        return ok_response({"error": "no vision API key configured — set one in the Config panel (top right)"})
    task_id = f"describe_{uuid.uuid4().hex[:8]}"
    report_status(gallery_id, task_id, "started")
    worker_params = dict(params)
    worker_params["task_id"] = task_id
    try:
        boto3.client("lambda", region_name=os.environ.get("AWS_REGION", "us-east-1")).invoke(
            FunctionName=os.environ.get("AWS_LAMBDA_FUNCTION_NAME", "polypaint-storage"),
            InvocationType="Event",
            Payload=json.dumps({"internal_action": "describe_gallery",
                                "params": worker_params}).encode("utf-8"),
        )
    except Exception as exc:  # noqa: BLE001
        report_status(gallery_id, task_id, "error", f"describe dispatch failed: {exc}")
        raise
    return ok_response({"dispatched": True, "task_id": task_id, "job_id": gallery_id})


def _run_describe_gallery_worker(params):
    """Self-invoked worker wrapper: run the describe, then mark the task row.
    Partial success is DONE (titles were CAS-saved per piece); zero successes
    with errors is an error row carrying the first cause."""
    gallery_id = str(params.get("gallery_id") or "")
    task_id = str(params.get("task_id") or "describe")
    try:
        result = _describe_gallery_run(params)
    except Exception as exc:  # noqa: BLE001
        report_status(gallery_id, task_id, "error", str(exc)[:300])
        raise
    errors = result.get("errors") or []
    if not result.get("described") and errors:
        report_status(gallery_id, task_id, "error", str(errors[0].get("error") or "describe failed")[:300])
    else:
        report_status(gallery_id, task_id, "done")
    return ok_response({"described": result.get("described", 0), "errors": errors})


def _describe_gallery_run(params):
    """Generate short curator titles for gallery pieces from their thumbnails,
    reusing the Book tab's vision engine + model/key config (lambda/book_describe).
    SMALL-BATCH: at most 4 pieces per run — each success is CAS-saved
    immediately so progress survives later failures.

    OWNERSHIP (code-review-30 F1, the Book run_base pattern): each target
    carries a base_title — the caller's dispatch-time value when provided,
    else the title observed when this run started. The generated title is
    written ONLY while the piece's current title still equals that base; a
    newer human title always wins (skipped with a coded error, no vision
    spend when detectable up front). CAS conflicts re-read, re-check
    ownership, and retry IN PROCESS — they never escape to the platform's
    async retry (storage has retries=0 regardless)."""
    from book_describe import _vision_call, _load_vision_config, _downscale_for_vision
    from shared import vision_provider
    gallery_id = assert_safe_id(params.get("gallery_id"), "gallery_id")
    raw_targets = params.get("pieces")
    overwrite = bool(params.get("overwrite"))
    doc, revision = _read_gallery_doc_with_etag(gallery_id)

    def key3(p):
        return (str(p.get("job_id") or ""), str(p.get("family") or "") or "color",
                str(p.get("artifact_id") or ""))

    def find_piece(d, k):
        for p in (d.get("pieces") or []):
            if key3(p) == k:
                return p
        return None

    # PRESENCE of `pieces` chooses explicit mode (CR30 follow-up F5): an empty list is an
    # explicit request for nothing (clean no-op), never a bulk describe.
    explicit = isinstance(raw_targets, list)
    base_by_key = {}
    missing_errors = []
    if explicit:
        want = []
        for t in raw_targets[:8]:
            if not isinstance(t, dict):
                raise ValueError("pieces entries must be objects")
            k = (assert_safe_id(t.get("job_id"), "job_id"),
                 str(t.get("family") or "") or "color",
                 assert_safe_id(t.get("artifact_id"), "artifact_id"))
            want.append(k)
            if "base_title" in t:
                base_by_key[k] = _clean_gallery_title(t.get("base_title"))
        present = {key3(p) for p in (doc.get("pieces") or [])}
        target_keys = [k for k in want if k in present]
        # EVERY requested-but-absent member is an error — mixed lists included
        # (CR30 follow-up F5: described=1 with silently dropped missing members is a lie).
        missing_errors = [{"artifact_id": k[2], "error": "gallery_piece_missing"}
                          for k in want if k not in present]
        if not target_keys:
            return {"gallery": doc, "revision": revision, "described": 0,
                    "errors": missing_errors}
    else:
        target_keys = [key3(p) for p in (doc.get("pieces") or [])]
    if not overwrite:
        target_keys = [k for k in target_keys
                       if not str((find_piece(doc, k) or {}).get("title") or "").strip()]
    target_keys = target_keys[:4]   # small batch per run — bounded worker wall time
    if not target_keys:
        return {"gallery": doc, "revision": revision, "described": 0, "errors": []}
    for k in target_keys:
        base_by_key.setdefault(k, _clean_gallery_title((find_piece(doc, k) or {}).get("title")))

    cfg = _load_vision_config()
    model = str(cfg.get("model") or "") or "gemini-2.5-flash"
    prov = vision_provider(model)
    api_key = cfg.get(f"api_key_{prov}") or (os.environ.get("GEMINI_API_KEY", "") if prov == "gemini" else "")
    if not api_key:
        return {"gallery": doc, "revision": revision, "described": 0,
                "errors": [{"artifact_id": "", "error": "no vision API key configured"}]}

    used = [str(p.get("title") or "").strip() for p in (doc.get("pieces") or [])
            if str(p.get("title") or "").strip()]
    described, errors = 0, list(missing_errors)
    deadline = time.time() + DESCRIBE_TIME_BUDGET_S
    for i, k in enumerate(target_keys):
        if time.time() >= deadline - DESCRIBE_STATUS_RESERVE_S:
            errors.append({"artifact_id": "", "error":
                f"time budget reached with {len(target_keys) - i} piece(s) left — run Describe again to continue"})
            break
        try:
            p = find_piece(doc, k)
            if p is None:
                errors.append({"artifact_id": k[2], "error": "gallery_piece_missing"})
                continue
            base = base_by_key[k]
            if _clean_gallery_title(p.get("title")) != base:
                # A newer (human) title landed after dispatch — it wins, and we
                # detect it BEFORE spending a vision call.
                errors.append({"artifact_id": k[2], "error": "title_changed — kept the newer title"})
                continue
            obj = s3.get_object(Bucket=BUCKET, Key=str(p.get("preview_key") or ""))
            img = _downscale_for_vision(obj["Body"].read())
            prompt = (
                "You are titling one abstract mathematical artwork for a gallery wall. "
                "Look at the image and reply with ONLY a JSON object: {\"title\": \"...\"}. "
                "2-4 words, evocative but concrete, no colons, no quotes inside. "
                + ("Do not reuse any of these existing titles: " + "; ".join(used[-20:]) + ". " if used else "")
            )
            remaining = max(5.0, deadline - time.time() - DESCRIBE_STATUS_RESERVE_S)
            title = _gallery_title_from_reply(
                _vision_call(model, api_key, img, prompt, budget_s=remaining))
            new_title = _clean_gallery_title(title)
            for _attempt in range(3):
                p["title"] = new_title
                doc["updated_at"] = _utc_now_iso()
                try:
                    revision = _put_gallery_doc(doc, expected_revision=revision)   # per-piece persistence
                    used.append(new_title)
                    described += 1
                    break
                except GalleryConflictError:
                    # The gallery moved mid-run: re-read, re-check FIELD ownership,
                    # and retry in process. A changed title means a human edit won.
                    doc, revision = _read_gallery_doc_with_etag(gallery_id)
                    p = find_piece(doc, k)
                    if p is None:
                        errors.append({"artifact_id": k[2], "error": "gallery_piece_missing"})
                        break
                    if _clean_gallery_title(p.get("title")) != base:
                        errors.append({"artifact_id": k[2], "error": "title_changed — kept the newer title"})
                        break
            else:
                errors.append({"artifact_id": k[2], "error": "save_conflict — the gallery kept moving"})
        except Exception as exc:   # per-piece isolation: one failure never voids the rest
            errors.append({"artifact_id": k[2], "error": str(exc)[:200]})
    return {"gallery": doc, "revision": revision, "described": described, "errors": errors}


def handle_create_gallery_share(event):
    """Snapshot an editable gallery into the immutable share manifest the viewer
    loads (the 'Open Gallery' action) — sequence + titles preserved."""
    params = parse_body(event)
    gallery_id = assert_safe_id(params.get("gallery_id") or params.get("id"), "gallery_id")
    doc, revision = _read_gallery_doc_with_etag(gallery_id)
    # Pin the share to the revision the user reviewed (REQUIRED, like save): the
    # snapshot must be the reviewed set, so a missing or moved revision is refused
    # rather than silently sharing whatever is current.
    expected_revision = str(params.get("expected_revision") or "").strip()
    if not expected_revision:
        raise ValueError("create-gallery-share requires expected_revision (refetch the gallery)")
    if expected_revision != revision:
        raise GalleryConflictError(
            f"gallery {gallery_id} changed since revision {expected_revision!r}; refetch and re-open")
    pieces = doc.get("pieces") or []
    if not pieces:
        return ok_response({"error": "gallery is empty"})
    source_kind = str((doc.get("source") or {}).get("kind") or "deepzoom_selection")
    seed = _parse_int((doc.get("layout") or {}).get("seed")) or 1
    manifest_url, manifest_key, share_id, count = _write_gallery_share_manifest(
        pieces, source_kind=source_kind, seed=seed, settings=doc.get("settings"))
    return ok_response({
        "manifest_url": manifest_url,
        "manifest_key": manifest_key,
        "share_id": share_id,
        "count": count,
        "gallery_id": gallery_id,
    })


def handle_render_count(event):
    params = parse_body(event)
    job_id = str(params.get("job_id") or "").strip()
    if not job_id:
        raise ValueError("render-count requires job_id")

    t0 = time.time()
    immutable = len(_list_render_family_variants(job_id, "color"))
    legacy = 1 if _legacy_render_variant(job_id, "color") else 0
    return ok_response({
        "job_id": job_id,
        "family": "color",
        "color_artifact_count": immutable,
        "legacy_color_artifact_count": legacy,
        "color_render_count": immutable + legacy,
        "count_us": int((time.time() - t0) * 1e6),
    })


def _delete_prefix_objects(prefix):
    objects = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        objects.extend(page.get("Contents", []))

    if not objects:
        return 0

    total_deleted = 0
    for i in range(0, len(objects), 1000):
        batch = objects[i:i + 1000]
        resp = s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": obj["Key"]} for obj in batch]},
        )
        total_deleted += len(resp.get("Deleted", []))
    return total_deleted


def handle_delete_render_artifact(event):
    params = parse_body(event)
    job_id = params["job_id"]
    family = params["family"]
    artifact_id = params["artifact_id"]

    if family == "palette":
        if artifact_id == "legacy_palette":
            keys = [f"renders/{job_id}/image_palette.jpeg", f"renders/{job_id}/preview_palette.png"]
            deleted = 0
            for key in keys:
                try:
                    s3.delete_object(Bucket=BUCKET, Key=key)
                    deleted += 1
                except Exception:
                    pass
            return ok_response({"job_id": job_id, "family": family, "artifact_id": artifact_id, "deleted": deleted})
        return handle_delete_palette({"body": json.dumps({"job_id": job_id, "palette_id": artifact_id})})

    if artifact_id == f"legacy_{family}":
        deleted = 0
        for suffix in RENDER_FAMILY_SHAPES[family]["legacy_image_candidates"] + RENDER_FAMILY_SHAPES[family]["legacy_preview_candidates"]:
            try:
                s3.delete_object(Bucket=BUCKET, Key=f"renders/{job_id}/{suffix}")
                deleted += 1
            except Exception:
                pass
        return ok_response({"job_id": job_id, "family": family, "artifact_id": artifact_id, "deleted": deleted})

    family_dir = RENDER_FAMILY_DIRS.get(family)
    if not family_dir:
        raise RuntimeError(f"Unknown render family: {family}")
    prefix = f"renders/{job_id}/{family_dir}/{artifact_id}/"
    deleted = _delete_prefix_objects(prefix)
    return ok_response({"job_id": job_id, "family": family, "artifact_id": artifact_id, "deleted": deleted})


def _key_exists(key):
    """Check if an S3 key exists via HEAD (fast, no data transfer).

    Returns False ONLY for a genuine 404/NoSuchKey. A throttle, 5xx, transport
    error, or AccessDenied propagates (code-review-28 F13): a transient failure
    must never be reported as 'absent', because callers use this for v1/v2
    program lifecycle and overwrite decisions where a false 'absent' skips
    required cleanup or mislabels a save."""
    try:
        s3.head_object(Bucket=BUCKET, Key=key)
        return True
    except Exception as exc:
        if is_missing_s3_error(exc):
            return False
        raise


def handle_delete(event):
    """Delete all S3 objects for a given job_id."""
    params = parse_body(event)
    # renders/_index/... and renders/_shared_mosaic/... hold shared mosaic
    # state; a leading-underscore job_id would wipe them (code-review-26 F11)
    job_id = _assert_mutable_job_partition(params["job_id"])
    # Drop the catalog row up front so a forced refresh can never resurrect the
    # job while its objects are mid-delete (reconcile would also prune it later).
    _results_catalog_delete(job_id)

    # List all objects for this job
    prefix = f"renders/{job_id}/"
    objects = []
    paginator = s3.get_paginator('list_objects_v2')
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        objects.extend(page.get('Contents', []))

    if not objects:
        return ok_response({"job_id": job_id, "deleted": 0})

    # Delete in batches of 1000
    total_deleted = 0
    errors = []
    for i in range(0, len(objects), 1000):
        batch = objects[i:i + 1000]
        resp = s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": obj["Key"]} for obj in batch]},
        )
        total_deleted += len(resp.get("Deleted", []))
        for err in resp.get("Errors", []):
            errors.append(f"{err['Key']}: {err['Code']}")

    result = {"job_id": job_id, "deleted": total_deleted}
    if errors:
        result["errors"] = errors
    return ok_response(result)


def handle_list_prefix(event):
    """List S3 keys under a prefix, optionally filtered by suffix.
    Input: {prefix, suffix (optional), delimiter (optional), max_keys (optional, default 1000)}
    When delimiter is set, returns {prefixes: [...]} (CommonPrefixes) instead of keys.
    Returns: {keys: [...]} or {prefixes: [...]}
    """
    params = parse_body(event)
    prefix = params["prefix"]
    suffix = params.get("suffix", "")
    delimiter = params.get("delimiter", "")
    max_keys = params.get("max_keys", 1000)

    paginator = s3.get_paginator('list_objects_v2')
    paginate_kwargs = {"Bucket": BUCKET, "Prefix": prefix}
    if delimiter:
        paginate_kwargs["Delimiter"] = delimiter

    if delimiter:
        # Return CommonPrefixes (folder-level listing)
        prefixes = []
        for page in paginator.paginate(**paginate_kwargs):
            for cp in page.get('CommonPrefixes', []):
                prefixes.append(cp['Prefix'])
                if len(prefixes) >= max_keys:
                    break
            if len(prefixes) >= max_keys:
                break
        return ok_response({"prefixes": prefixes, "count": len(prefixes)})
    else:
        keys = []
        for page in paginator.paginate(**paginate_kwargs):
            for obj in page.get('Contents', []):
                if not suffix or obj['Key'].endswith(suffix):
                    keys.append(obj['Key'])
                    if len(keys) >= max_keys:
                        break
            if len(keys) >= max_keys:
                break
        return ok_response({"keys": keys, "count": len(keys)})


def handle_check_keys(event):
    """Check how many expected S3 keys exist under a prefix.
    Input: {prefix: "renders/jobid/", expected: 4000, suffix: ".raw"}
    Uses S3 list (fast) instead of per-key HEAD.
    Kept for backward compatibility — prefer /check-status for new code.
    """
    params = parse_body(event)
    prefix = params["prefix"]
    expected = params["expected"]
    suffix = params.get("suffix", ".raw")

    found = 0
    paginator = s3.get_paginator('list_objects_v2')
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        for obj in page.get('Contents', []):
            if obj['Key'].endswith(suffix):
                found += 1

    return ok_response({"found": found, "expected": expected, "done": found >= expected})


def handle_check_status(event):
    """Query DynamoDB for task completion status.
    Input: {job_id, task_prefix, expected}
    task_prefix examples: "render_<run>_raster_" or "merge_0_"
    Returns: {done, errors, error_details, total, expected, complete}
    """
    params = parse_body(event)
    job_id = params["job_id"]
    task_prefix = params["task_prefix"]
    expected = params["expected"]

    return_ids = params.get("return_ids", False)

    now_ms = int(time.time() * 1000)
    ddb = _get_ddb()
    done = 0
    found_ids = []
    error_details = []
    stuck_tasks = []
    status_counts = {}
    results = []
    # Freshness tracking
    latest_update_ms = None
    latest_done_ms = None
    latest_nonterminal_ms = None
    newest_task = None

    kwargs = {
        "TableName": JOBS_TABLE,
        "KeyConditionExpression": "job_id = :jid AND begins_with(task_id, :pfx)",
        "ExpressionAttributeValues": {
            ":jid": {"S": job_id},
            ":pfx": {"S": task_prefix},
        },
        "ProjectionExpression": "task_id, task_status, error_msg, result_data, updated_at_ms",
    }
    while True:
        resp = ddb.query(**kwargs)
        for item in resp["Items"]:
            status = item["task_status"]["S"]
            task_id_val = item["task_id"]["S"]
            status_counts[status] = status_counts.get(status, 0) + 1
            if return_ids:
                found_ids.append(task_id_val)

            # Parse updated_at_ms (may be absent on old rows)
            row_ms = None
            if "updated_at_ms" in item:
                try:
                    row_ms = int(item["updated_at_ms"]["N"])
                except (ValueError, KeyError):
                    pass

            # Track freshness
            if row_ms is not None:
                if latest_update_ms is None or row_ms > latest_update_ms:
                    latest_update_ms = row_ms
                    newest_task = {"task_id": task_id_val, "status": status, "updated_at_ms": row_ms}
                if status == "done" and (latest_done_ms is None or row_ms > latest_done_ms):
                    latest_done_ms = row_ms
                if status not in ("done", "error") and (latest_nonterminal_ms is None or row_ms > latest_nonterminal_ms):
                    latest_nonterminal_ms = row_ms

            # Collect result_data
            parsed_rd = None
            rd = item.get("result_data", {}).get("S")
            if rd:
                try:
                    parsed_rd = json.loads(rd)
                    results.append(parsed_rd)
                except Exception:
                    pass

            if status == "done":
                done += 1
            elif status == "error":
                detail = {
                    "task_id": task_id_val,
                    "error_msg": item.get("error_msg", {}).get("S", "unknown"),
                }
                if parsed_rd is not None:
                    detail["result_data"] = parsed_rd
                error_details.append(detail)
            else:
                entry = {"task_id": task_id_val, "status": status}
                if row_ms is not None:
                    entry["updated_at_ms"] = row_ms
                    entry["age_ms"] = now_ms - row_ms
                stuck_tasks.append(entry)

        if "LastEvaluatedKey" not in resp:
            break
        kwargs["ExclusiveStartKey"] = resp["LastEvaluatedKey"]

    total = done + len(error_details)
    resp_body = {
        "done": done,
        "errors": len(error_details),
        "error_details": error_details[:20],
        "stuck": stuck_tasks[:50],
        "status_counts": status_counts,
        "total": total,
        "expected": expected,
        "complete": total >= expected,
        "latest_update_ms": latest_update_ms,
        "latest_done_ms": latest_done_ms,
        "latest_nonterminal_ms": latest_nonterminal_ms,
        "stale_for_ms": (now_ms - latest_update_ms) if latest_update_ms else None,
    }
    if newest_task:
        resp_body["newest_task"] = newest_task
    if results:
        resp_body["results"] = results
    if return_ids:
        resp_body["found_ids"] = found_ids
    return ok_response(resp_body)


# Canonical ownership mapping: each family owns its own intermediates, previews, and stale siblings
ARTIFACT_FAMILIES = {
    "color": {
        "intermediate_prefixes": ["pix_", "raw_", "solve_proximity/", "solve_scores/"],
        "intermediate_keys": ["solve_proximity_clip.json", "solve_proximity_bins.json"],
        "preview": [],
        "same_family_stale": [],
    },
    "bilevel": {
        "intermediate_prefixes": ["bilevel_t", "bits_chunk_", "bilevel_section_"],
        "intermediate_keys": [],
        "preview": [],
        "same_family_stale": [],
    },
    "coeff_bilevel": {
        "intermediate_prefixes": ["coeff_bilevel_section_", "coeff_t", "coeff_bits_chunk_"],
        "intermediate_keys": [],
        "preview": [],
        "same_family_stale": [],
    },
    "palette": {
        "intermediate_prefixes": [],
        "intermediate_keys": [],
        "preview": [],
        "same_family_stale": [],
    },
}


# Internal state that generic job-scoped mutation routes must never touch:
# - DDB sentinels __config__ (vision keys) / __allrenders_mosaic__ (mosaic
#   status), which start with '_'
# - the favorites partition favorites#color
# - S3 pseudo-jobs renders/_index/... (mosaic manifests + wall pyramids) and
#   renders/_shared_mosaic/... (share snapshots), whose job segment starts '_'
# A single leading-underscore rule covers every internal render/DDB namespace;
# real render/compute jobs never begin with '_'. (code-review-25 F1 +
# code-review-26 F11)
RESERVED_JOB_PREFIXES = ("_", "favorites#", "results#")


def _assert_mutable_job_partition(job_id):
    jid = str(job_id or "")
    if not jid or jid.startswith(RESERVED_JOB_PREFIXES):
        raise ValueError(
            f"job_id {jid!r} is a reserved internal partition; "
            f"it cannot be mutated or deleted through a generic job route")
    return jid


def handle_clean_render(event):
    """Family-scoped cleanup: delete only the specified family's intermediates,
    previews, and stale same-family siblings. Never touches other families."""
    params = parse_body(event)
    job_id = _assert_mutable_job_partition(params["job_id"])
    prefix = f"renders/{job_id}/"
    pipeline = params.get("pipeline", "color")

    family = ARTIFACT_FAMILIES.get(pipeline, ARTIFACT_FAMILIES["color"])

    objects = []
    paginator = s3.get_paginator('list_objects_v2')
    for rp in family["intermediate_prefixes"]:
        for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix + rp):
            for obj in page.get('Contents', []):
                objects.append(obj)
    for key_suffix in family["intermediate_keys"]:
        objects.append({"Key": prefix + key_suffix})
    for key_suffix in family["preview"]:
        objects.append({"Key": prefix + key_suffix})
    for key_suffix in family["same_family_stale"]:
        objects.append({"Key": prefix + key_suffix})

    total_deleted = 0
    if objects:
        for i in range(0, len(objects), 1000):
            batch = objects[i:i + 1000]
            resp = s3.delete_objects(
                Bucket=BUCKET,
                Delete={"Objects": [{"Key": obj["Key"]} for obj in batch]},
            )
            total_deleted += len(resp.get("Deleted", []))

    # CR33 telemetry retention: this handler used to delete EVERY DynamoDB
    # status row in the job partition, destroying compute telemetry (24h TTL)
    # whenever a render started and the previous render's telemetry whenever
    # the next one did. Every progress reader queries with a RUN-SCOPED
    # task_id prefix (e.g. "render_<run>_raster_"), and run IDs are unique,
    # so stale rows are invisible to the UI and expire via TTL on their own.
    # Broad partition deletion discarded benchmark evidence without providing
    # a correctness benefit — rows are now left to TTL.
    ddb_deleted = 0
    ddb_errors = []

    result = {
        "job_id": job_id,
        "deleted": total_deleted,
        "ddb_deleted": ddb_deleted,
    }
    if ddb_errors:
        result["ddb_errors"] = ddb_errors
    return ok_response(result)


def handle_save_metadata(event):
    """Save calc.json metadata to S3.
    Input: {job_id, metadata} where metadata is the calc.json content.
    """
    params = parse_body(event)
    job_id = params["job_id"]
    metadata = params["metadata"]

    s3.put_object(Bucket=BUCKET,
                  Key=f"renders/{job_id}/calc.json",
                  Body=json.dumps(metadata),
                  ContentType="application/json")

    # Keep the results catalog row in lockstep (results-list.md Phase 2) —
    # best-effort: a miss self-heals on the next /list reconcile.
    if isinstance(metadata, dict):
        _results_catalog_put(_results_catalog_item(_results_entry_fields(job_id, metadata)))

    return ok_response({"job_id": job_id, "saved": "calc.json"})


def _safe_download_filename(raw):
    """Sanitize a caller-supplied download name before it goes into a
    Content-Disposition header: strip path/quote/control chars so the public
    /presign route can't emit a malformed header (code-review-26 F15)."""
    name = str(raw or "").strip()
    if not name:
        return ""
    name = name.replace("\\", "/").rsplit("/", 1)[-1]   # drop any path
    name = re.sub(r'[\x00-\x1f\x7f"\\;]', "", name)       # control/quote/sep
    name = name.strip() or "download"
    return name[:120]


def handle_presign(event):
    """Generate a presigned URL for an S3 key.
    Input: {key: "renders/job_id/image.jpeg", filename: "optional_download_name.jpeg"}
    If filename is provided, the URL forces a download with that filename.
    Returns: {url: presigned_url, key: key}
    """
    params = parse_body(event)
    key = params["key"]
    # presign is read-only but mints a GET URL for whatever key it's handed;
    # restrict to the prefixes the app actually serves so it can't be turned
    # into an arbitrary-object exfiltration oracle.
    if not (key.startswith("renders/") or key.startswith(BOOKS_PREFIX)):
        raise ValueError("presign key must be under renders/ or polypaint/books/")
    s3_params = {"Bucket": BUCKET, "Key": key}
    filename = _safe_download_filename(params.get("filename"))
    if filename:
        s3_params["ResponseContentDisposition"] = f'attachment; filename="{filename}"'
    url = s3.generate_presigned_url(
        "get_object",
        Params=s3_params,
        ExpiresIn=PRESIGN_EXPIRY)
    return ok_response({"url": url, "key": key})


def _is_cleanup_allowed_key(key):
    key = str(key or "").strip()
    if not key or key.startswith("/") or "\\" in key:
        return False
    parts = key.split("/")
    if any(part in ("", ".", "..") for part in parts):
        return False
    if len(parts) < 3 or parts[0] != "renders":
        return False
    if len(parts) == 3 and parts[-1] == "deepzoom_latest.json":
        return True
    filename = parts[-1]
    temp_suffixes = (".tmp", ".part", ".frag")
    if filename.endswith(temp_suffixes):
        return True
    temp_markers = ("stripe", "merge", "chunk", "section", "temp", "tmp")
    if any(marker in filename for marker in temp_markers):
        return filename.endswith((".raw", ".bin", ".json", ".frag"))
    return False


def handle_cleanup(event):
    """Delete explicit temp S3 keys under renders/<job>/ only.
    Input: {keys: ["renders/job/chunk_0.raw", ...]}
    """
    params = parse_body(event)
    keys = params.get("keys", [])
    if not isinstance(keys, list):
        raise ValueError("cleanup keys must be an array")

    if not keys:
        return ok_response({"deleted": 0})

    normalized = [str(key or "").strip() for key in keys]
    invalid = [key for key in normalized if not _is_cleanup_allowed_key(key)]
    if invalid:
        sample = ", ".join(invalid[:5])
        raise ValueError(f"cleanup key not allowed: {sample}")

    total_deleted = 0
    errors = []
    for i in range(0, len(keys), 1000):
        batch = normalized[i:i + 1000]
        resp = s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": k} for k in batch]},
        )
        total_deleted += len(resp.get("Deleted", []))
        for err in resp.get("Errors", []):
            errors.append(f"{err.get('Key', '?')}: {err.get('Code', 'Error')}")

    result = {"deleted": total_deleted}
    if errors:
        result["errors"] = errors
    return ok_response(result)


def _detail_populate_program_form(pipeline, profile):
    """Program chain + readable source for Populate, derived read-only.

    Populate restores programs in program mode, but older results store the
    program either as a lowered _typed_* chain or as legacy *_transforms with no
    program chain at all (e.g. a v1 result with param_transforms unit_circle,exp).
    Translate legacy transforms when there is no chain, then reconstruct readable,
    reparseable source (the same equivalent reconstruction migration uses) so
    Populate shows a clean, computable program instead of an empty editor.
    Returns (chain_to_provide_or_None, source_to_provide_or_None); a stored
    source text always wins (source is None then). Never rewrites the result.
    """
    if profile == "param":
        to_chain = param_transforms_to_program_chain
        to_source = param_source_text_from_chain
    else:
        to_chain = coeff_transforms_to_program_chain
        to_source = coeff_source_text_from_chain
    chain = pipeline.get("%s_program_chain" % profile)
    provided_chain = None
    if not (isinstance(chain, list) and chain):
        transforms = pipeline.get("%s_transforms" % profile)
        if isinstance(transforms, list) and transforms:
            chain = to_chain(transforms)
            provided_chain = chain  # Populate has no program chain otherwise
    if not (isinstance(chain, list) and chain):
        return None, None
    source = None
    if not pipeline.get("%s_program_source_text" % profile):
        text = to_source(chain)
        if text and text.strip():
            source = text
    return provided_chain, source


def handle_detail(event):
    """Return file_count and compute-job viewport/metadata for a single job."""
    params = parse_body(event)
    job_id = params["job_id"]
    prefix = f"renders/{job_id}/"
    result = {"job_id": job_id}

    # Count files
    try:
        n_files = 0
        for page in s3.get_paginator('list_objects_v2').paginate(
                Bucket=BUCKET, Prefix=prefix):
            n_files += page.get('KeyCount', 0)
        result["file_count"] = n_files
    except Exception:
        result["file_count"] = 0

    # Read compute view.json coordinates.
    try:
        vobj = s3.get_object(Bucket=BUCKET,
                             Key=f"renders/{job_id}/view.json")
        view = json.loads(vobj["Body"].read())
        result["compute_q_re"] = view.get("q_re")
        result["compute_q_im"] = view.get("q_im")
        # Compatibility alias for older /detail consumers.
        result["q_re"] = result["compute_q_re"]
        result["q_im"] = result["compute_q_im"]
    except Exception:
        pass

    # Read calc.json for full compute metadata + pipeline info
    try:
        cobj = s3.get_object(Bucket=BUCKET,
                             Key=f"renders/{job_id}/calc.json")
        calc = json.loads(cobj["Body"].read())
        result["calc"] = calc
        # Extract pipeline info for the info panel
        pipeline = calc.get("pipeline", {})
        result["times"] = calc.get("times", 1)
        result["param_transforms"] = pipeline.get("param_transforms", [])
        result["param_transforms_display"] = pipeline.get("param_transforms_display", [])
        result["coeff_transforms"] = pipeline.get("coeff_transforms", [])
        # Parity with param: migrated pipelines empty coeff_transforms and
        # carry the human-readable list here; without this the sidebar showed
        # coeff transforms as "none" after /migrate-compute.
        result["coeff_transforms_display"] = pipeline.get("coeff_transforms_display", [])
        result["pipeline"] = pipeline
        version = _calc_pipeline_program_version(pipeline)
        result["pipeline_program_version"] = version
        result["pipeline_migratable"] = version == 1
        # Give Populate a clean, computable program form for older results:
        # translate legacy *_transforms when there is no program chain, and
        # reconstruct readable source from the chain. Read-only; stored source
        # text wins; failures fall back silently (the program editor then stays
        # empty as before rather than breaking the response).
        for _profile in ("param", "coeff"):
            try:
                _chain, _source = _detail_populate_program_form(pipeline, _profile)
                if _chain is not None:
                    result["%s_program_chain" % _profile] = _chain
                if _source is not None:
                    result["%s_program_source_text" % _profile] = _source
            except Exception:
                pass
    except Exception:
        pass

    # Check for preview (presign if found)
    preview_key = None
    if _key_exists(f"renders/{job_id}/preview.png"):
        preview_key = f"renders/{job_id}/preview.png"
    elif _key_exists(f"renders/{job_id}/preview.jpg"):
        preview_key = f"renders/{job_id}/preview.jpg"
    result["has_preview"] = preview_key is not None
    if preview_key:
        result["preview_url"] = s3.generate_presigned_url(
            "get_object",
            Params={"Bucket": BUCKET, "Key": preview_key},
            ExpiresIn=PRESIGN_EXPIRY)

    # Load preview stats only if preview exists (avoid stale stats)
    if preview_key:
        try:
            ps_obj = s3.get_object(Bucket=BUCKET,
                                   Key=f"renders/{job_id}/preview_stats.json")
            result["preview_stats"] = json.loads(ps_obj["Body"].read())
        except Exception:
            pass

    return ok_response(result)


def _head_artifact_keys(keys, presign=True, *, s3_client=None, parallel=True):
    """HEAD-check a list of S3 keys — in parallel by default; parallel=False
    runs serially for callers ALREADY inside a bounded pool (code-review-30
    F11: an inner executor per favorite nested inside the refresh pool just
    multiplied threads contending for one shared S3 connection pool)."""
    import concurrent.futures
    if not keys:
        return {}
    client = s3_client or s3

    def check(key):
        try:
            resp = client.head_object(Bucket=BUCKET, Key=key)
            info = {
                "exists": True,
                "key": key,
                "size": resp.get("ContentLength", 0),
                "type": resp.get("ContentType", ""),
                "width": None,
                "height": None,
                "url": None,
                "modified_at": resp.get("LastModified").strftime("%Y-%m-%dT%H:%M:%SZ") if resp.get("LastModified") else None,
                "user_meta": {},
            }
            user_meta = resp.get("Metadata", {})
            info["user_meta"] = user_meta
            if "width" in user_meta and "height" in user_meta:
                info["width"] = int(user_meta["width"])
                info["height"] = int(user_meta["height"])
            if presign:
                info["url"] = client.generate_presigned_url(
                    "get_object",
                    Params={"Bucket": BUCKET, "Key": key},
                    ExpiresIn=PRESIGN_EXPIRY)
            return key, info
        except Exception as exc:
            # Fail-soft is intentional here (one bad key must not 500 the whole
            # batch), but a transient/throttle/access error is NOT absence
            # (code-review-28 F13). Only a genuine 404 is a clean "missing";
            # otherwise surface the reason so a summary can retry or show
            # "unknown" rather than silently dropping a real artifact.
            absent = {"exists": False, "key": key, "size": 0, "type": "", "width": None,
                      "height": None, "url": None, "modified_at": None, "user_meta": {}}
            if not is_missing_s3_error(exc):
                absent["error"] = True
                absent["error_reason"] = s3_error_reason(exc)
            return key, absent

    if not parallel:
        return dict(check(key) for key in keys)
    with concurrent.futures.ThreadPoolExecutor(max_workers=min(len(keys), 20)) as pool:
        results = dict(pool.map(check, keys))
    return results


def handle_head_keys(event):
    """Check which S3 keys exist via HEAD (batch), return metadata."""
    params = parse_body(event)
    keys = params.get("keys", [])
    presign = params.get("presign", False)

    if not keys:
        return ok_response({"exists": [], "meta": {}})

    result = _head_artifact_keys(keys, presign=presign)
    exists = [k for k, v in result.items() if v["exists"]]
    meta = {k: v for k, v in result.items() if v["exists"]}
    return ok_response({"exists": exists, "meta": meta})


def _calc_chunk_items_for_summary(calc):
    calc = calc or {}
    degree = int(calc.get("degree", 1) or 1)
    n_coeffs = int(calc.get("n_coeffs", degree + 1) or (degree + 1))
    record_bytes = degree * 2 * 4
    items = []
    step_start = 0
    chunks = list(calc.get("chunks", calc.get("stripes", [])) or [])
    for raw in sorted(chunks, key=lambda row: row.get("idx", row.get("chunk_idx", row.get("stripe_idx", 0)))):
        step_count = raw.get("step_count", raw.get("n_t"))
        bin_size = raw.get("bin_size")
        if step_count in ("", None) and bin_size not in ("", None) and record_bytes > 0:
            step_count = int(bin_size) // record_bytes
        if step_count in ("", None):
            continue
        step_count = int(step_count)
        if step_count < 1:
            continue
        items.append({
            "chunk_idx": int(raw.get("idx", raw.get("chunk_idx", raw.get("stripe_idx", len(items))))),
            "step_start": step_start,
            "step_count": step_count,
            "bin_size": int(bin_size) if bin_size not in ("", None) else step_count * record_bytes,
            "coeffs_bin_size": int(raw.get("coeffs_size") or 0) or (step_count * n_coeffs * 2 * 4),
            "params_bin_size": int(raw.get("params_bin_size") or 0) or (step_count * 16),
        })
        step_start += step_count
    return items


def _render_summary_calc(calc_data):
    calc = {
        "exists": True,
        "N": calc_data.get("N", calc_data.get("n1")),
        "n1": calc_data.get("n1", calc_data.get("N")),
        "degree": calc_data.get("degree"),
    }
    chunk_items = _calc_chunk_items_for_summary(calc_data)
    if chunk_items:
        degree = int(calc_data.get("degree", 1) or 1)
        n_coeffs = int(calc_data.get("n_coeffs", degree + 1) or (degree + 1))
        summary = summarize_chunk_items(chunk_items, degree, n_coeffs)
        calc.update({
            "n_chunks": len(chunk_items),
            "n_coeffs": n_coeffs,
            "job_size": {
                **summary,
                "solve_hist_memory_mb": DEFAULT_SOLVE_SCORE_MEMORY_MB,
                "palette_chunk_memory_mb": DEFAULT_PALETTE_CHUNK_MEMORY_MB,
                "raster_memory_mb": DEFAULT_RASTER_MEMORY_MB,
                "auto_usable_fraction": AUTO_USABLE_FRACTION,
                "auto_fixed_overhead_mb": AUTO_FIXED_OVERHEAD_MB,
                "auto_per_thread_overhead_mb": AUTO_PER_THREAD_OVERHEAD_MB,
                "lores_root_bytes": int(((calc_data.get("lores") or {}).get("bin_size") or 0) or 0),
                "lores_coeff_bytes": int(((calc_data.get("lores") or {}).get("coeffs_size") or 0) or 0),
                "lores_param_bytes": int(((calc_data.get("lores") or {}).get("params_size") or 0) or 0),
            },
        })
    return calc


def handle_render_summary(event):
    """Single-call Render refresh.
    Returns immutable per-family artifact catalogs plus legacy top-level artifacts
    for compatibility with older jobs."""
    params = parse_body(event)
    job_id = params["job_id"]
    prefix = f"renders/{job_id}/"

    # Legacy top-level artifacts kept for compatibility with older jobs.
    artifact_map = {
        "color_jpeg": prefix + "image.jpeg",
        "color_png": prefix + "image.png",
        "bilevel_tif": prefix + "image_bilevel.tif",
        "bilevel_preview_png": prefix + "image_bilevel_preview.png",
        "bilevel_compat_tif": prefix + "image_bilevel_compat.tif",
        "bilevel_png": prefix + "image_bilevel.png",
        "coeff_tif": prefix + "image_coeffs_bilevel.tif",
        "coeff_preview_png": prefix + "image_coeffs_bilevel_preview.png",
        "preview_coeffs_png": prefix + "preview_coeffs.png",
        "preview_color_png": prefix + "preview_color.png",
        "preview_bilevel_png": prefix + "preview_bilevel.png",
        "palette_jpeg": prefix + "image_palette.jpeg",
        "preview_palette_png": prefix + "preview_palette.png",
    }
    head_results = _head_artifact_keys(list(artifact_map.values()), presign=True)
    artifacts = {}
    for logical_name, s3_key in artifact_map.items():
        artifacts[logical_name] = head_results.get(s3_key, {
            "exists": False, "key": s3_key, "size": 0, "type": "",
            "width": None, "height": None, "url": None, "modified_at": None, "user_meta": {}
        })

    # 2. Read calc.json server-side
    calc = {"exists": False, "N": None, "n1": None, "degree": None}
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=prefix + "calc.json")
        calc_data = json.loads(obj["Body"].read())
        calc = _render_summary_calc(calc_data)
    except Exception:
        pass

    # 3. Read deepzoom pointer server-side (no list/scan)
    deepzoom_latest = {"exists": False}
    try:
        obj = s3.get_object(Bucket=BUCKET, Key=prefix + "deepzoom_latest.json")
        dz_data = json.loads(obj["Body"].read())
        # Presign the DZI URL for browser access
        dzi_key = dz_data.get("dzi_key", "")
        dzi_url = dz_data.get("dzi_url", "")
        deepzoom_latest = {
            "exists": True,
            "export_id": dz_data.get("export_id", ""),
            "created_at": dz_data.get("created_at", ""),
            "source_key": dz_data.get("source_key", ""),
            "source_artifact_id": dz_data.get("source_artifact_id", ""),
            "source_family": dz_data.get("source_family", ""),
            "source_rotation": dz_data.get("source_rotation"),
            "viewport_min_re": dz_data.get("viewport_min_re"),
            "viewport_max_re": dz_data.get("viewport_max_re"),
            "viewport_min_im": dz_data.get("viewport_min_im"),
            "viewport_max_im": dz_data.get("viewport_max_im"),
            "dzi_key": dzi_key,
            "dzi_url": dzi_url,
            "share_url": dz_data.get("share_url", ""),
            "tile_prefix": dz_data.get("tile_prefix", ""),
            "width": dz_data.get("width"),
            "height": dz_data.get("height"),
            "tiles_uploaded": dz_data.get("tiles_uploaded"),
        }
    except Exception:
        pass

    families = {
        "color": _list_render_family_variants(job_id, "color"),
        "bilevel": _list_render_family_variants(job_id, "bilevel"),
        "coeffs": _list_render_family_variants(job_id, "coeffs"),
        "palette": _list_saved_palettes(job_id),
        "pdf": _list_render_family_variants(job_id, "pdf"),
    }

    for family in ("color", "bilevel", "coeffs", "palette", "pdf"):
        legacy = _legacy_render_variant(job_id, family)
        if legacy:
            families[family].append(legacy)
        if family == "color":
            families[family] = _order_color_variants(families[family])
        elif family == "palette":
            families[family] = _order_palette_variants(families[family])
        else:
            families[family].sort(key=lambda a: a.get("created_at", ""), reverse=True)

    return ok_response({
        "job_id": job_id,
        "schema_version": 2,
        "calc": calc,
        "artifacts": artifacts,
        "families": families,
        "deepzoom_latest": deepzoom_latest,
    })


def handle_delete_task(event):
    """Delete a single DynamoDB task status row.
    Input: {job_id, task_id}
    Used to clear stale status before re-dispatching a task with a fixed task_id.
    """
    params = parse_body(event)
    job_id = _assert_mutable_job_partition(params["job_id"])
    task_id = params["task_id"]
    ddb = _get_ddb()
    ddb.delete_item(
        TableName=JOBS_TABLE,
        Key={"job_id": {"S": job_id}, "task_id": {"S": task_id}},
    )
    return ok_response({"deleted": f"{job_id}/{task_id}"})


# exactly deepzoom/<job_id>/<export_id>/ — the single-export delete the UI
# issues. Broader prefixes (deepzoom/, deepzoom/<job>/) would wipe every
# export or a whole job's exports through a direct API call (code-review-27 F11).
_DEEPZOOM_EXPORT_PREFIX = re.compile(r"deepzoom/[A-Za-z0-9_-]{1,64}/[A-Za-z0-9_-]{1,64}/")


def handle_delete_prefix(event):
    """Delete all S3 objects under one DeepZoom export prefix.
    Input: {prefix} — must be exactly deepzoom/<job_id>/<export_id>/.
    """
    params = parse_body(event)
    prefix = str(params.get("prefix") or "")
    if not _DEEPZOOM_EXPORT_PREFIX.fullmatch(prefix):
        return {
            "statusCode": 400,
            "headers": {"Content-Type": "application/json", "Access-Control-Allow-Origin": "*"},
            "body": json.dumps({"error": "delete-prefix requires exactly deepzoom/<job_id>/<export_id>/"}),
        }

    objects = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix=prefix):
        objects.extend(page.get("Contents", []))

    if not objects:
        return ok_response({"prefix": prefix, "deleted": 0})

    total_deleted = 0
    for i in range(0, len(objects), 1000):
        batch = objects[i:i + 1000]
        resp = s3.delete_objects(
            Bucket=BUCKET,
            Delete={"Objects": [{"Key": obj["Key"]} for obj in batch]},
        )
        total_deleted += len(resp.get("Deleted", []))

    return ok_response({"prefix": prefix, "deleted": total_deleted})


def handle_list_sheets(event):
    """List poly-sheet artifacts. Delimiter listing (CR35-F20): the old
    flat scan walked EVERY key under sheets/ — including up to 512
    temporary frame tiles per stranded run — so refresh latency grew
    with debris, not with sheets. Now: one delimiter pass enumerates
    the sheet prefixes, then only each prefix's direct children are
    listed (tiles live one level deeper and are never touched)."""
    import concurrent.futures

    paginator = s3.get_paginator("list_objects_v2")
    prefixes = []
    for page in paginator.paginate(Bucket=BUCKET, Prefix="sheets/",
                                   Delimiter="/"):
        prefixes.extend(p["Prefix"] for p in page.get("CommonPrefixes", []))

    def manifest_row(prefix):
        sheet_id = prefix[len("sheets/"):].rstrip("/")
        if not sheet_id:
            return None
        run = None
        run_modified = None
        try:
            run_obj = s3.get_object(Bucket=BUCKET, Key=prefix + "run.json")
            run = json.loads(run_obj["Body"].read())
            run_modified = run_obj["LastModified"].isoformat()
        except ClientError as exc:
            # round-5 finding 6: a transient S3 error must NOT be read as
            # "no run record" (which would fabricate legacy fixed keys for
            # a pointer-only sheet). Only a confirmed-missing key is a miss.
            if not is_missing_s3_error(exc):
                raise
            run = None
        except (ValueError, TypeError):
            run = None

        # round-3 findings 2/6: the run record is the PUBLICATION POINTER.
        # A published run names its generation's immutable artifacts; a
        # sheet WITHOUT a pointer falls back to the legacy fixed keys.
        row = None
        if isinstance(run, dict) and run.get("published_png_key"):
            row = {
                "sheet_id": sheet_id,
                "manifest_key": run.get("published_manifest_key")
                or (prefix + "sheet.json"),
                "png_key": run["published_png_key"],
                "modified": run_modified,
                "size": 0,
            }
        else:
            try:
                head = s3.head_object(Bucket=BUCKET, Key=prefix + "sheet.json")
                row = {
                    "sheet_id": sheet_id,
                    "manifest_key": prefix + "sheet.json",
                    "png_key": prefix + "sheet.png",
                    "modified": head["LastModified"].isoformat(),
                    "size": int(head["ContentLength"]),
                }
            except ClientError:
                row = None

        # expose ONLY genuinely-running runs for client discovery/resume
        if isinstance(run, dict) and run.get("status") == "running":
            if row is None:
                row = {
                    "sheet_id": sheet_id,
                    "manifest_key": prefix + "sheet.json",
                    "png_key": prefix + "sheet.png",
                    "modified": run_modified,
                    "size": 0,
                }
            row["run_status"] = "running"
            row["run_generation"] = str(run.get("generation") or "")
            row["run_key"] = prefix + "run.json"
        return row

    with concurrent.futures.ThreadPoolExecutor(max_workers=16) as pool:
        sheets = [row for row in pool.map(manifest_row, prefixes) if row]
    sheets.sort(key=lambda r: r["modified"], reverse=True)
    return ok_response({"sheets": sheets})


def handle_list_deepzoom(event):
    """List all DeepZoom exports server-side in one call.
    Two-level prefix scan + parallel meta.json reads.
    Returns: {exports: [{job_id, export_id, ...}, ...]}
    """
    import concurrent.futures

    # Level 1: job prefixes under deepzoom/
    job_prefixes = []
    paginator = s3.get_paginator("list_objects_v2")
    for page in paginator.paginate(Bucket=BUCKET, Prefix="deepzoom/",
                                   Delimiter="/"):
        job_prefixes.extend(p["Prefix"] for p in page.get("CommonPrefixes", []))

    # Level 2: export prefixes under each job (parallelized)
    def list_exports(job_prefix):
        prefixes = []
        for page in paginator.paginate(Bucket=BUCKET, Prefix=job_prefix,
                                       Delimiter="/"):
            prefixes.extend(p["Prefix"] for p in page.get("CommonPrefixes", []))
        return prefixes

    export_prefixes = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=20) as pool:
        for batch in pool.map(list_exports, job_prefixes):
            export_prefixes.extend(batch)

    # Level 3: read meta.json for each export (parallelized)
    def read_meta(prefix):
        try:
            obj = s3.get_object(Bucket=BUCKET, Key=prefix + "meta.json")
            return json.loads(obj["Body"].read())
        except Exception:
            return None

    with concurrent.futures.ThreadPoolExecutor(max_workers=20) as pool:
        exports = [m for m in pool.map(read_meta, export_prefixes) if m]

    exports.sort(key=lambda e: e.get("created_at", ""), reverse=True)
    return ok_response({"exports": exports, "count": len(exports)})
