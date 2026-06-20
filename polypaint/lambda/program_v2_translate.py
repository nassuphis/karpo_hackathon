"""Program v2 translation helpers.

Phase 4 starts with storage-level migration: translate existing v1 saved
program payloads into v2 metadata/fingerprints without switching the native
runtime wire yet. The v2 fingerprint is deliberately distinct from v1 even
when the executable v1 spec is still the backing representation.
"""
from __future__ import annotations

import hashlib
import json

from coeff_program_chain import COEFF_OP_LEGACY, COEFF_OP_NATIVE_TRANSFORM
from coeff_program_chain import compile_coeff_program_chain
from coeff_program_source import coeff_source_text_from_chain, coeff_source_text_from_payload
from merged_opcodes import (
    MERGED_OP_DUPLICATE,
    MERGED_OP_FLUSH,
    MERGED_OP_NATIVE_TRANSFORM,
    MERGED_OP_PARAM_CUBE,
    MERGED_OP_PARAM_DIVIDE,
    MERGED_OP_PARAM_EMIT_P1,
    MERGED_OP_PARAM_EMIT_P2,
    MERGED_OP_PARAM_PUSH_P1,
    MERGED_OP_PARAM_PUSH_P2,
    MERGED_OP_PARAM_PUSH_T1,
    MERGED_OP_PARAM_PUSH_T2,
    MERGED_OP_PARAM_RATIO,
    MERGED_OP_PARAM_RECIPROCAL,
    MERGED_OP_PARAM_SQUARE,
    MERGED_OP_PARAM_UNIT_CIRCLE,
    MERGED_OP_POP,
    MERGED_OP_SWAP,
    MERGED_OP_TYPED_BINARY,
    MERGED_OP_TYPED_PUSH_SCALAR,
    MERGED_OP_TYPED_UNARY,
)
from param_program_chain import (
    PARAM_OP_ABS,
    PARAM_OP_ADD,
    PARAM_OP_CONJ,
    PARAM_OP_CONST,
    PARAM_OP_CUBE,
    PARAM_OP_DIVIDE,
    PARAM_OP_DUPLICATE,
    PARAM_OP_EMIT_P1,
    PARAM_OP_EMIT_P2,
    PARAM_OP_EXP,
    PARAM_OP_FLUSH,
    PARAM_OP_IMAG,
    PARAM_OP_LEGACY,
    PARAM_OP_MUL,
    PARAM_OP_NEGATE,
    PARAM_OP_POP,
    PARAM_OP_PUSH_P1,
    PARAM_OP_PUSH_P2,
    PARAM_OP_PUSH_T1,
    PARAM_OP_PUSH_T2,
    PARAM_OP_RATIO,
    PARAM_OP_REAL,
    PARAM_OP_RECIPROCAL,
    PARAM_OP_SQUARE,
    PARAM_OP_SUBTRACT,
    PARAM_OP_SWAP,
    PARAM_OP_UNIT_CIRCLE,
)
from param_program_chain import compile_param_program_chain
from param_program_source import param_source_text_from_chain, param_source_text_from_payload
from solve_score_chain import (
    compile_solve_score_chain_or_legacy,
    serialize_solve_score_chain,
)


V2_SPEC_VERSION = 2
V2_PROGRAM_VERSION = 2

_V2_TYPED_BINARY_FN = {
    PARAM_OP_ADD: 1,
    PARAM_OP_SUBTRACT: 2,
    PARAM_OP_MUL: 3,
}

_V2_TYPED_UNARY_FN = {
    PARAM_OP_ABS: 3,
    PARAM_OP_NEGATE: 4,
    PARAM_OP_CONJ: 5,
    PARAM_OP_REAL: 8,
    PARAM_OP_IMAG: 9,
    PARAM_OP_EXP: 10,
}

_V2_PARAM_DIRECT_OPS = {
    PARAM_OP_PUSH_T1: MERGED_OP_PARAM_PUSH_T1,
    PARAM_OP_PUSH_T2: MERGED_OP_PARAM_PUSH_T2,
    PARAM_OP_PUSH_P1: MERGED_OP_PARAM_PUSH_P1,
    PARAM_OP_PUSH_P2: MERGED_OP_PARAM_PUSH_P2,
    PARAM_OP_EMIT_P1: MERGED_OP_PARAM_EMIT_P1,
    PARAM_OP_EMIT_P2: MERGED_OP_PARAM_EMIT_P2,
    PARAM_OP_DUPLICATE: MERGED_OP_DUPLICATE,
    PARAM_OP_SWAP: MERGED_OP_SWAP,
    PARAM_OP_POP: MERGED_OP_POP,
    PARAM_OP_FLUSH: MERGED_OP_FLUSH,
    PARAM_OP_RATIO: MERGED_OP_PARAM_RATIO,
    PARAM_OP_DIVIDE: MERGED_OP_PARAM_DIVIDE,
    PARAM_OP_RECIPROCAL: MERGED_OP_PARAM_RECIPROCAL,
    PARAM_OP_UNIT_CIRCLE: MERGED_OP_PARAM_UNIT_CIRCLE,
    PARAM_OP_SQUARE: MERGED_OP_PARAM_SQUARE,
    PARAM_OP_CUBE: MERGED_OP_PARAM_CUBE,
}


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


def _clean_token(token):
    return {
        key: value
        for key, value in token.items()
        if value not in (None, [], {}) and not (key == "registry" and value == "")
    }


def _param_tokens_v2(tokens):
    out = []
    for raw in tokens or []:
        token = dict(raw)
        op = int(token.get("op") or 0)
        if op == PARAM_OP_CONST:
            token["op"] = MERGED_OP_TYPED_PUSH_SCALAR
        elif op in _V2_TYPED_BINARY_FN:
            token["op"] = MERGED_OP_TYPED_BINARY
            token["fn_index"] = _V2_TYPED_BINARY_FN[op]
        elif op in _V2_TYPED_UNARY_FN:
            token["op"] = MERGED_OP_TYPED_UNARY
            token["fn_index"] = _V2_TYPED_UNARY_FN[op]
        elif op == PARAM_OP_LEGACY:
            token["op"] = MERGED_OP_NATIVE_TRANSFORM
            token["registry"] = "param"
        elif op in _V2_PARAM_DIRECT_OPS:
            token["op"] = _V2_PARAM_DIRECT_OPS[op]
        else:
            raise RuntimeError(f"param v2 translation has no merged opcode for v1 op {op}")
        out.append(_clean_token(token))
    return out


def _coeff_tokens_v2(tokens):
    out = []
    for raw in tokens or []:
        token = dict(raw)
        op = int(token.get("op") or 0)
        if op in (COEFF_OP_LEGACY, COEFF_OP_NATIVE_TRANSFORM):
            token["op"] = MERGED_OP_NATIVE_TRANSFORM
            token["registry"] = "coeff"
        out.append(_clean_token(token))
    return out


def _execution_spec_v2(kind, tokens, scalar_exprs=None):
    payload = {
        "version": V2_PROGRAM_VERSION,
        "kind": str(kind),
        "tokens": tokens,
    }
    if scalar_exprs:
        payload["scalar_exprs"] = scalar_exprs
    return json.dumps(payload, sort_keys=True, separators=(",", ":"))


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
    v2_tokens = _param_tokens_v2(compiled["tokens"])
    v2_spec = _execution_spec_v2("param", v2_tokens, [])
    fingerprint = _v2_fingerprint(
        "param",
        {
            "execution_spec": v2_spec,
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
        "tokens": v2_tokens,
        "scalar_exprs": [],
        "display": compiled["display"],
        "expanded_display": compiled["expanded_display"],
        "fingerprint": fingerprint,
        "execution_spec": v2_spec,
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
    v2_tokens = _coeff_tokens_v2(compiled["tokens"])
    v2_scalar_exprs = compiled.get("scalar_exprs") or []
    v2_spec = _execution_spec_v2("coeff", v2_tokens, v2_scalar_exprs)
    fingerprint = _v2_fingerprint(
        "coeff",
        {
            "execution_spec": v2_spec,
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
        "tokens": v2_tokens,
        "scalar_exprs": v2_scalar_exprs,
        "display": compiled["display"],
        "expanded_display": compiled["expanded_display"],
        "fingerprint": fingerprint,
        "execution_spec": v2_spec,
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
