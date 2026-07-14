"""Shared compile payload and saved-program macro helpers.

These functions keep compute-plan, preview, coeffgen, and param-debug on one
implementation for the Param/Coeff Program payload shape and saved macro
resolution rules.
"""

import json

from coeff_program_source import coeff_source_text_from_payload, parse_coeff_program_source
from param_program_source import param_source_text_from_payload, parse_param_program_source
from shared import BUCKET, is_missing_s3_error  # noqa: F401 (re-exported below)

PARAM_PROGRAMS_PREFIX = "polypaint/param-programs/"
COEFF_PROGRAMS_PREFIX = "polypaint/coeff-programs/"


def compiled_coeff_program_payload(compiled):
    payload = {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "display": compiled["display"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "scalar_expr_count": compiled["scalar_expr_count"],
        "uses_legacy_chain_equivalent": compiled["uses_legacy_chain_equivalent"],
        "tokens": compiled["tokens"],
        "scalar_exprs": compiled["scalar_exprs"],
    }
    vector_constants = compiled.get("vector_constants") or []
    if vector_constants:
        payload["vector_constants"] = vector_constants
        payload["vector_constant_count"] = len(vector_constants)
        payload["vector_constant_elements"] = sum(
            int(item.get("length") or 0) for item in vector_constants
        )
    return payload


def compiled_param_program_payload(compiled):
    payload = {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "display": compiled["display"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "uses_legacy_fast_path": compiled["uses_legacy_fast_path"],
        "tokens": compiled["tokens"],
    }
    scalar_exprs = compiled.get("scalar_exprs") or []
    if scalar_exprs:
        payload["scalar_exprs"] = scalar_exprs
    return payload


# is_missing_s3_error is re-exported from shared (code-review-28 F13): one
# policy for S3 absence. NoSuchBucket is intentionally NOT absence — it is a
# configuration error and must propagate, not be swallowed as "object missing".


def _resolve_s3_client(s3_client=None, s3_client_factory=None):
    if s3_client_factory is not None:
        return s3_client_factory()
    if s3_client is not None:
        return s3_client
    raise RuntimeError("saved-program macro resolver requires an S3 client")


def read_saved_program_source_chain(
    prefix,
    program_kind,
    program_id,
    *,
    s3_client=None,
    s3_client_factory=None,
    bucket=BUCKET,
):
    macro_id = str(program_id or "").strip()
    # Strip any stray v2/ namespace prefix exactly like the storage-side
    # _normalize_program_id, so macro(v2/foo) resolves the same saved object
    # at compute time as it does at save time.
    while macro_id.startswith("v2/"):
        macro_id = macro_id[len("v2/"):]
    if not macro_id:
        raise RuntimeError(f"{program_kind} macro name is required")
    key = f"{prefix}{macro_id}.json"
    try:
        obj = _resolve_s3_client(s3_client, s3_client_factory).get_object(Bucket=bucket, Key=key)
    except Exception as exc:
        if is_missing_s3_error(exc):
            raise RuntimeError(f"{program_kind} macro not found: {macro_id}") from None
        raise
    raw = obj["Body"].read()
    try:
        payload = json.loads(raw) if raw else {}
    except Exception as exc:
        raise RuntimeError(f"{program_kind} macro is not valid JSON: {macro_id}") from exc
    if not isinstance(payload, dict):
        raise RuntimeError(f"{program_kind} macro must be a JSON object: {macro_id}")
    if program_kind == "coeff program":
        source_text = coeff_source_text_from_payload(payload)
        if source_text is not None:
            parsed = parse_coeff_program_source(source_text)
            return parsed["chain"]
    if program_kind == "param program":
        source_text = param_source_text_from_payload(payload)
        if source_text is not None:
            parsed = parse_param_program_source(source_text)
            return parsed["chain"]
    chain = payload.get("chain")
    if not isinstance(chain, list):
        raise RuntimeError(f"{program_kind} macro chain must be a JSON array: {macro_id}")
    return chain


def param_program_macro_resolver(*, s3_client=None, s3_client_factory=None, bucket=BUCKET):
    return lambda macro_id: read_saved_program_source_chain(
        PARAM_PROGRAMS_PREFIX,
        "param program",
        macro_id,
        s3_client=s3_client,
        s3_client_factory=s3_client_factory,
        bucket=bucket,
    )


def coeff_program_macro_resolver(*, s3_client=None, s3_client_factory=None, bucket=BUCKET):
    return lambda macro_id: read_saved_program_source_chain(
        COEFF_PROGRAMS_PREFIX,
        "coeff program",
        macro_id,
        s3_client=s3_client,
        s3_client_factory=s3_client_factory,
        bucket=bucket,
    )
