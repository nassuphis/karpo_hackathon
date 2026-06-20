"""Program v2 translation helpers.

Phase 4 starts with storage-level migration: translate existing v1 saved
program payloads into v2 metadata/fingerprints without switching the native
runtime wire yet. The v2 fingerprint is deliberately distinct from v1 even
when the executable v1 spec is still the backing representation.
"""
from __future__ import annotations

import hashlib
import json

from coeff_program_chain import compile_coeff_program_chain
from coeff_program_source import coeff_source_text_from_chain, coeff_source_text_from_payload
from param_program_chain import compile_param_program_chain
from param_program_source import param_source_text_from_chain, param_source_text_from_payload
from solve_score_chain import (
    compile_solve_score_chain_or_legacy,
    serialize_solve_score_chain,
)


V2_SPEC_VERSION = 2
V2_PROGRAM_VERSION = 2


def _v2_fingerprint(kind, payload):
    packed = json.dumps(
        {
            "program_kind": str(kind),
            "spec_version": V2_SPEC_VERSION,
            "payload": payload,
        },
        sort_keys=True,
        separators=(",", ":"),
    )
    return f"sha256:{hashlib.sha256(packed.encode('utf-8')).hexdigest()}"


def _macro_ids(chain):
    out = []
    for chip in chain or []:
        if isinstance(chip, list) and len(chip) >= 2 and str(chip[0]).strip().lower() == "macro":
            macro_id = str(chip[1] or "").strip()
            if macro_id:
                out.append(macro_id)
    return out


def v1_summary(program):
    return {
        "fingerprint": str(program.get("fingerprint") or ""),
        "spec_version": int(program.get("spec_version") or 1),
        "program_version": int(program.get("version") or 1),
    }


def translate_param_from_old(program, *, macro_resolver=None):
    source_text = param_source_text_from_payload(program)
    if source_text is None:
        source_text = param_source_text_from_chain(program.get("chain") or [])
    parsed_chain = program.get("chain") or []
    compiled = compile_param_program_chain(parsed_chain, macro_resolver=macro_resolver)
    fingerprint = _v2_fingerprint(
        "param",
        {
            "execution_spec": compiled["execution_spec"],
            "source_text": source_text,
        },
    )
    return {
        "program_kind": "param_program",
        "version": V2_PROGRAM_VERSION,
        "program_version": V2_PROGRAM_VERSION,
        "spec_version": V2_SPEC_VERSION,
        "id": str(program.get("id") or ""),
        "name": str(program.get("name") or ""),
        "source_text": source_text,
        "chain": compiled["source_chain"],
        "display": compiled["display"],
        "expanded_display": compiled["expanded_display"],
        "fingerprint": fingerprint,
        "execution_spec": compiled["execution_spec"],
        "statement_count": len(compiled["source_chain"]),
        "token_count": compiled["token_count"],
        "stack_max": compiled["stack_max"],
        "emits": compiled["emits"],
        "macro_expansions": compiled["macro_expansions"],
        "macro_ids": _macro_ids(compiled["source_chain"]),
        "uses_legacy_fast_path": compiled["uses_legacy_fast_path"],
    }


def translate_coeff_from_old(program, *, macro_resolver=None):
    source_text = coeff_source_text_from_payload(program)
    if source_text is None:
        source_text = coeff_source_text_from_chain(program.get("chain") or [])
    parsed_chain = program.get("chain") or []
    compiled = compile_coeff_program_chain(parsed_chain, macro_resolver=macro_resolver)
    fingerprint = _v2_fingerprint(
        "coeff",
        {
            "execution_spec": compiled["execution_spec"],
            "source_text": source_text,
        },
    )
    return {
        "program_kind": "coeff_program",
        "version": V2_PROGRAM_VERSION,
        "program_version": V2_PROGRAM_VERSION,
        "spec_version": V2_SPEC_VERSION,
        "id": str(program.get("id") or ""),
        "name": str(program.get("name") or ""),
        "source_text": source_text,
        "chain": compiled["source_chain"],
        "display": compiled["display"],
        "expanded_display": compiled["expanded_display"],
        "fingerprint": fingerprint,
        "execution_spec": compiled["execution_spec"],
        "statement_count": len(compiled["source_chain"]),
        "token_count": compiled["token_count"],
        "scalar_expr_count": compiled["scalar_expr_count"],
        "stack_max": compiled["stack_max"],
        "macro_expansions": compiled["macro_expansions"],
        "macro_ids": _macro_ids(compiled["source_chain"]),
        "uses_legacy_chain_equivalent": compiled["uses_legacy_chain_equivalent"],
    }


def translate_solve_score_from_old(program):
    compiled = compile_solve_score_chain_or_legacy(
        program.get("chain") or [],
        "",
        default_metric="proximity",
    )
    canonical_chain = json.loads(serialize_solve_score_chain(compiled["chain"]))
    fingerprint = _v2_fingerprint(
        "solve-score",
        {
            "program_spec": compiled["program_spec"],
            "metrics": compiled.get("metrics") or [],
            "chain": canonical_chain,
        },
    )
    migrated = {
        "version": V2_PROGRAM_VERSION,
        "program_version": V2_PROGRAM_VERSION,
        "spec_version": V2_SPEC_VERSION,
        "id": str(program.get("id") or ""),
        "name": str(program.get("name") or ""),
        "chain": canonical_chain,
        "metric": compiled["metric"],
        "display": compiled["display"],
        "program_spec": compiled["program_spec"],
        "fingerprint": fingerprint,
        "statement_count": len(canonical_chain),
        "metric_count": len(compiled.get("metrics") or []),
        "output_channel_count": compiled.get("output_channel_count", 0),
        "has_explicit_outputs": bool(compiled.get("has_explicit_outputs")),
    }
    if program.get("recommended_interpretation") not in ("", None):
        migrated["recommended_interpretation"] = program["recommended_interpretation"]
    return migrated
