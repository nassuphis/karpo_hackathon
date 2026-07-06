"""Program v2 translation helpers.

Phase 4 starts with storage-level migration: translate existing v1 saved
program payloads into v2 metadata/fingerprints without switching the native
runtime wire yet. The v2 fingerprint is deliberately distinct from v1 even
when the executable v1 spec is still the backing representation.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path

from coeff_program_chain import COEFF_OP_LEGACY, COEFF_OP_NATIVE_TRANSFORM
from coeff_program_chain import compile_coeff_program_chain
from coeff_program_source import (
    coeff_source_text_from_chain,
    coeff_source_text_from_payload,
    parse_coeff_program_source,
)
import merged_opcodes as merged
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
    MERGED_OP_PARAM_PUSH_REG,
    MERGED_OP_PARAM_PUSH_T1,
    MERGED_OP_PARAM_PUSH_T2,
    MERGED_OP_PARAM_RATIO,
    MERGED_OP_PARAM_STORE_REG,
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
    PARAM_OP_PUSH_REG,
    PARAM_OP_DIVIDE,
    PARAM_OP_STORE_REG,
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
from param_program_source import (
    param_source_text_from_chain,
    param_source_text_from_payload,
    parse_param_program_source,
)
from solve_score_chain import (
    compile_solve_score_chain_or_legacy,
    render_solve_score_program_spec,
    serialize_solve_score_chain,
)
from solve_score_program_source import (
    SolveScoreProgramSourceError,
    solve_score_source_text_from_chain,
)


V2_SPEC_VERSION = 2
V2_PROGRAM_VERSION = 2
_LAMBDA_DIR = Path(__file__).resolve().parent

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
    PARAM_OP_PUSH_REG: MERGED_OP_PARAM_PUSH_REG,
    PARAM_OP_STORE_REG: MERGED_OP_PARAM_STORE_REG,
}


def _v2_fingerprint(kind, payload):
    identity_payload = {
        key: value
        for key, value in (payload or {}).items()
        if key not in {"source_text", "source_display", "display", "expanded_display"}
    }
    packed = json.dumps(
        {
            "program_kind": str(kind),
            "spec_version": V2_SPEC_VERSION,
            "payload": identity_payload,
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


def _load_root_registry_by_name():
    path = _LAMBDA_DIR / "root_legacy_registry.json"
    with path.open("r", encoding="utf-8") as fh:
        payload = json.load(fh)
    return {
        str(item["name"]): dict(item)
        for item in payload.get("functions") or []
        if isinstance(item, dict) and item.get("name")
    }


_SOLVE_SCORE_KIND_TO_OP = {
    "avg": merged.MERGED_OP_SCORE_AVG,
    "min": merged.MERGED_OP_SCORE_MIN,
    "max": merged.MERGED_OP_SCORE_MAX,
    "mul": merged.MERGED_OP_SCORE_MUL,
    "weighted_sum": merged.MERGED_OP_SCORE_WEIGHTED_SUM,
    "abs_diff": merged.MERGED_OP_SCORE_ABS_DIFF,
    "geometric_mean": merged.MERGED_OP_SCORE_GEOMETRIC_MEAN,
    "omega_cosine": merged.MERGED_OP_SCORE_OMEGA_COSINE,
    "sawtooth": merged.MERGED_OP_SCORE_SAWTOOTH,
    "flip": merged.MERGED_OP_SCORE_FLIP,
    "emit": merged.MERGED_OP_SCORE_EMIT,
    "const": merged.MERGED_OP_SCORE_CONST,
    "dup": merged.MERGED_OP_SCORE_DUP,
    "add": merged.MERGED_OP_SCORE_ADD,
    "mult": merged.MERGED_OP_SCORE_MULT,
    "subtract": merged.MERGED_OP_SCORE_SUBTRACT,
    "ratio": merged.MERGED_OP_SCORE_RATIO,
    "clamp": merged.MERGED_OP_SCORE_CLAMP,
    "ema": merged.MERGED_OP_SCORE_EMA,
    "sin": merged.MERGED_OP_SCORE_SIN,
    "cos": merged.MERGED_OP_SCORE_COS,
    "log": merged.MERGED_OP_SCORE_LOG,
    "exp": merged.MERGED_OP_SCORE_EXP,
    "pow": merged.MERGED_OP_SCORE_POW,
    "flush": merged.MERGED_OP_SCORE_FLUSH,
}


def _solve_score_tokens_v2(compiled):
    tokens = []
    for metric in compiled.get("metrics") or []:
        tokens.append(_clean_token({
            "op": merged.MERGED_OP_SCORE_REDUCE_METRIC,
            "metric": str(metric["metric"]),
            "source": str(metric.get("source", "slv")),
            "quantile": float(metric["quantile"]),
            "slot": int(metric["slot"]),
        }))
    for raw in compiled.get("program_tokens") or []:
        token = dict(raw)
        kind = str(token.get("kind") or "")
        if kind == "metric":
            out = {
                "op": merged.MERGED_OP_SCORE_PUSH_METRIC,
                "metric_slot": int(token["slot"]),
                "lag_depth": int(token.get("lag", 0) or 0),
            }
        elif kind == "const":
            out = {"op": merged.MERGED_OP_SCORE_CONST, "value": float(token["value"])}
        elif kind == "weighted_sum":
            out = {
                "op": merged.MERGED_OP_SCORE_WEIGHTED_SUM,
                "a": float(token["a"]),
                "b": float(token["b"]),
            }
        elif kind == "ema":
            out = {"op": merged.MERGED_OP_SCORE_EMA, "alpha": float(token["alpha"])}
        elif kind == "pow":
            out = {"op": merged.MERGED_OP_SCORE_POW, "exponent": float(token["exponent"])}
        elif kind == "omega_cosine":
            out = {
                "op": merged.MERGED_OP_SCORE_OMEGA_COSINE,
                "omega": float(token["omega"]),
                "phase": float(token.get("phase", 0.0) or 0.0),
            }
        elif kind == "sawtooth":
            out = {"op": merged.MERGED_OP_SCORE_SAWTOOTH, "mult": float(token["mult"])}
        elif kind == "emit":
            mode = str(token.get("mode", "raw") or "raw")
            op = {
                "raw": merged.MERGED_OP_SCORE_EMIT,
                "norm": merged.MERGED_OP_SCORE_EMIT_NORM,
                "none": merged.MERGED_OP_SCORE_EMIT_NONE,
            }.get(mode)
            if op is None:
                raise RuntimeError(f"solve-score v2 translation has no emit mode {mode!r}")
            out = {"op": op, "mode": mode}
            if mode != "none":
                out["channel"] = int(token.get("channel", 0) or 0)
        else:
            op = _SOLVE_SCORE_KIND_TO_OP.get(kind)
            if op is None:
                raise RuntimeError(f"solve-score v2 translation has no merged opcode for {kind!r}")
            out = {"op": op}
        tokens.append(_clean_token(out))
    return tokens


def v1_summary(program):
    return {
        "fingerprint": str(program.get("fingerprint") or ""),
        "spec_version": int(program.get("spec_version") or 1),
        "program_version": int(program.get("version") or 1),
    }


def translate_param_from_old(program, *, macro_resolver=None):
    source_text = param_source_text_from_payload(program)
    if source_text is not None:
        parsed_chain = parse_param_program_source(source_text)["chain"]
    else:
        parsed_chain = program.get("chain") or []
        source_text = param_source_text_from_chain(parsed_chain)
    compiled = compile_param_program_chain(parsed_chain, macro_resolver=macro_resolver)
    v2_tokens = _param_tokens_v2(compiled["tokens"])
    v2_spec = _execution_spec_v2("param", v2_tokens, [])
    fingerprint = _v2_fingerprint(
        "param",
        {
            "execution_spec": v2_spec,
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
    if source_text is not None:
        parsed_chain = parse_coeff_program_source(source_text)["chain"]
    else:
        parsed_chain = program.get("chain") or []
        source_text = coeff_source_text_from_chain(parsed_chain)
    compiled = compile_coeff_program_chain(parsed_chain, macro_resolver=macro_resolver)
    v2_tokens = _coeff_tokens_v2(compiled["tokens"])
    v2_scalar_exprs = compiled.get("scalar_exprs") or []
    v2_spec = _execution_spec_v2("coeff", v2_tokens, v2_scalar_exprs)
    fingerprint = _v2_fingerprint(
        "coeff",
        {
            "execution_spec": v2_spec,
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
    if not isinstance(program, dict) or "chain" not in program or program.get("chain") in (None, "", []):
        raise RuntimeError("solve-score v2 migration requires a non-empty chain")
    compiled = compile_solve_score_chain_or_legacy(
        program.get("chain"),
        "",
        default_metric="proximity",
    )
    canonical_chain = json.loads(serialize_solve_score_chain(compiled["chain"]))
    diagnostics = []
    try:
        source_text = solve_score_source_text_from_chain(canonical_chain)
    except (SolveScoreProgramSourceError, RuntimeError, ValueError) as exc:
        source_text = ""
        diagnostics.append({
            "level": "warning",
            "code": getattr(exc, "code", "source_roundtrip_failed") or "source_roundtrip_failed",
            "message": f"solve-score source regeneration failed during migration: {exc}",
        })
    program_spec = render_solve_score_program_spec(compiled["program_spec"], version=V2_SPEC_VERSION)
    v2_tokens = _solve_score_tokens_v2(compiled)
    v2_spec = _execution_spec_v2("solve-score", v2_tokens, [])
    fingerprint = _v2_fingerprint(
        "solve-score",
        {
            "execution_spec": v2_spec,
            "program_spec": program_spec,
            "metrics": compiled.get("metrics") or [],
            "chain": canonical_chain,
        },
    )
    migrated = {
        # Return-dict metadata only — the _v2_fingerprint payload above is
        # frozen wire and must not gain keys.
        "program_kind": "solve_score_program",
        "version": V2_PROGRAM_VERSION,
        "program_version": V2_PROGRAM_VERSION,
        "spec_version": V2_SPEC_VERSION,
        "id": str(program.get("id") or ""),
        "name": str(program.get("name") or ""),
        "source_text": source_text,
        "source_display": source_text,
        "chain": canonical_chain,
        "metric": compiled["metric"],
        "display": compiled["display"],
        "program_spec": program_spec,
        "tokens": v2_tokens,
        "execution_spec": v2_spec,
        "fingerprint": fingerprint,
        "statement_count": len(canonical_chain),
        "token_count": len(v2_tokens),
        "metric_count": len(compiled.get("metrics") or []),
        "output_channel_count": compiled.get("output_channel_count", 0),
        "output_channels": compiled.get("output_channels") or [],
        "has_explicit_outputs": bool(compiled.get("has_explicit_outputs")),
        "diagnostics": diagnostics,
    }
    if program.get("recommended_interpretation") not in ("", None):
        migrated["recommended_interpretation"] = program["recommended_interpretation"]
    return migrated

