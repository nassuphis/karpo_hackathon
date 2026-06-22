"""
Authoritative source-text parser for Coeff Program v1.

The browser may provide highlighting and advisory diagnostics, but this module
is the only source parser used by save, preview, and compute-plan paths.
It lowers source text to the existing coeff_program_chain chip list, then lets
coeff_program_chain compile that list to VM tokens.
"""
from __future__ import annotations

import re

from coeff_program_chain import (
    TEXT_NAME_ALIASES,
    canonical_unary_op_name,
    expr_value_if_static,
    EXPR_ADD,
    EXPR_CF_AT,
    EXPR_CF_AT_DYN,
    EXPR_DIV,
    EXPR_LITERAL,
    EXPR_MUL,
    EXPR_P1,
    EXPR_P2,
    EXPR_POLY_AT,
    EXPR_POLY_AT_DYN,
    EXPR_POLY_LEN,
    EXPR_SUB,
    EXPR_T1,
    EXPR_T2,
    EXPR_TOS_AT,
    EXPR_TOS_AT_DYN,
    FN_LINEAR,
    FN_EXP,
    FN_ROUND,
    FN_POW,
    MAX_VECTOR_LEN,
    ExpressionParser,
    SCALAR_UNARY_EXPR_OPS,
    TYPED_BINARY_NAME_ALIASES,
    VECTOR_BINARY_OPS,
    VECTOR_UNARY_OPS,
    compile_coeff_program_chain,
    display_coeff_program_chain,
    legacy_registry,
    native_transform_stack_arg_limit,
)
from program_source_core import (
    ProgramSourceError,
    ProfileStatementLowerer,
    find_top_level_assignment,
    parse_profile_source,
    parse_call,
    profile_selectors,
    profile_symbols_with_context,
    program_profile,
    split_program_statements,
    split_top_level,
)


MAX_COEFF_PROGRAM_SOURCE_BYTES = 64 * 1024

_IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")

_PROFILE = program_profile("coeff")
_PROFILE_OUTPUTS = tuple(str(name) for name in (_PROFILE.get("outputs") or ()))
_POLY_SYMBOL = _PROFILE_OUTPUTS[0] if _PROFILE_OUTPUTS else "poly"
_SOURCE_NAMES = frozenset(profile_selectors(_PROFILE, "src"))
_VECTOR_SOURCE_NAMES = frozenset(profile_selectors(_PROFILE, "vector_src"))
_TYPED_VECTOR_SOURCE_NAMES = frozenset(profile_selectors(_PROFILE, "typed_vector_src"))
_TARGET_NAMES = frozenset(profile_selectors(_PROFILE, "tgt"))
_PUSH_SOURCE_NAMES = frozenset(profile_selectors(_PROFILE, "push_src"))
_INDEX_BASE_NAMES = frozenset(profile_symbols_with_context(_PROFILE, "expr_index_base"))
_WRITABLE_LHS_NAMES = frozenset(profile_symbols_with_context(_PROFILE, "lhs", access="read_write"))
_INDEX_BASE_RE = re.compile(
    r"^(" + "|".join(re.escape(name) for name in sorted(_INDEX_BASE_NAMES, key=len, reverse=True)) + r")\[",
    re.IGNORECASE,
)
_INDEXED_LHS_RE = re.compile(
    r"^(" + "|".join(re.escape(name) for name in sorted(_WRITABLE_LHS_NAMES, key=len, reverse=True)) + r")\[(.*)\]$",
    re.IGNORECASE,
)

# Canonical names plus the typed shorthands, both derived from the chain
# layer's tables so the vocabularies cannot drift.
_VECTOR_BINARY_ALIASES = {
    **{name: name for name in VECTOR_BINARY_OPS},
    **TYPED_BINARY_NAME_ALIASES,
}

_VECTOR_UNARY_NAMES = frozenset(VECTOR_UNARY_OPS)

# EXPR opcode -> typed unary chip name, inverted from the chain layer's map
# ("mod" and "abs" share an opcode; the canonical chip name wins).
_EXPR_UNARY_CHIP_NAMES = {
    op: canonical_unary_op_name(name)
    for name, op in SCALAR_UNARY_EXPR_OPS.items()
}

# Remaining EXPR-opcode lowering tables (the push/index/binary families).
_EXPR_SCALAR_PUSHES = {
    EXPR_P1: "p1",
    EXPR_P2: "p2",
    EXPR_T1: "t1",
    EXPR_T2: "t2",
    EXPR_POLY_LEN: "poly_len",
}
_EXPR_INDEX_SOURCES = {EXPR_CF_AT: "cf", EXPR_POLY_AT: "poly", EXPR_TOS_AT: "tos"}
_EXPR_DYN_INDEX_SOURCES = {EXPR_CF_AT_DYN: "cf", EXPR_POLY_AT_DYN: "poly", EXPR_TOS_AT_DYN: "tos"}
_EXPR_BINARY_CHIP_NAMES = {
    EXPR_ADD: "add",
    EXPR_SUB: "subtract",
    EXPR_MUL: "multiply",
    EXPR_DIV: "divide",
}

_STACK_ALIASES = {
    "dup": "duplicate",
    "duplicate": "duplicate",
    "swap": "swap",
    "drop": "pop",
    "flush": "flush",
    "emit": "emit",
}

_VECTOR_FILL_NAMES = {"fill", "const", "push_const", "push_vec"}
# Source-text aliases for native transforms whose registry names are shadowed
# by typed-expression builtins (exp/pow/power). Single source of truth:
# coeff_legacy_registry.json, via the chain loader (the chip compiler and the
# generated JS vocab consume the same data).
_NATIVE_TRANSFORM_ALIASES = dict(TEXT_NAME_ALIASES)


def _is_source_name(text):
    return str(text or "").strip().lower() in _SOURCE_NAMES


def _is_vector_source_text(text):
    return str(text or "").strip().lower() in _TYPED_VECTOR_SOURCE_NAMES


class CoeffProgramSourceError(ProgramSourceError):
    # line/column default to 0 (unknown) so callers can fall back to the
    # enclosing statement's location with `exc.line or stmt.line`. A truthy
    # default of 1 used to pin every lowering error to line 1, column 1.
    def __init__(self, message: str, *, line: int = 0, column: int = 0, code: str = "source_error"):
        super().__init__(message, line=line, column=column, code=code)


class CoeffProgramSourceCompileError(RuntimeError):
    """Strict-mode source compile failure with structured diagnostics."""

    def __init__(self, diagnostics):
        self.diagnostics = list(diagnostics or [])
        message = "; ".join(d["message"] for d in self.diagnostics if d.get("level") == "error")
        super().__init__(message or "invalid coeff program source")


def _canonical_expr(text):
    # The chain expression parser natively understands mod()/tau/tau_i, so
    # only whitespace normalization is needed here.
    raw = str(text or "").strip()
    if not raw:
        raise CoeffProgramSourceError("expression is empty")
    return re.sub(r"\s+", "", raw)


def _split_top_level(text, sep=","):
    return split_top_level(text, sep=sep, error_cls=CoeffProgramSourceError)


def _find_top_level_assignment(text):
    return find_top_level_assignment(text)


def _parse_call(text):
    return parse_call(text, error_cls=CoeffProgramSourceError)


def _source_selector(text, *, allow_cf=True):
    raw = str(text or "").strip().lower()
    allowed = _SOURCE_NAMES if allow_cf else _VECTOR_SOURCE_NAMES
    if raw not in allowed:
        if raw in _SOURCE_NAMES:
            raise CoeffProgramSourceError(
                f"{raw} is not allowed as a source here; use one of {', '.join(sorted(allowed))}"
            )
        raise CoeffProgramSourceError(f"expected source selector, got {text!r}")
    return raw


def _target_selector(text):
    raw = str(text or "").strip().lower()
    if raw not in _TARGET_NAMES:
        raise CoeffProgramSourceError(f"expected target selector, got {text!r}")
    return raw


def _canonical_native_name(name):
    raw = str(name or "").strip().lower()
    return _NATIVE_TRANSFORM_ALIASES.get(raw, raw)


def _split_native_transform_andy(name, args):
    """Strip the trailing andy arg when the count matches the fn's packing.

    Mirrors coeffProgramNativeTransformOp's packed-arg cases (the FN_*
    registry ids) and coeff_program_chain's _legacy_args family.
    """
    spec = legacy_registry()["by_name"].get(_canonical_native_name(name))
    if not spec or not spec.get("supports_andy"):
        return list(args), None
    raw_args = list(args)
    fn_index = int(spec.get("fn_index") or 0)
    if fn_index in {FN_LINEAR, FN_POW}:
        andy_arities = {3, 5}
    elif fn_index == FN_EXP:
        andy_arities = {3}
    elif fn_index == FN_ROUND:
        andy_arities = {2, 3}
    else:
        andy_arities = {len(spec.get("args") or []) + 1}
    if len(raw_args) in andy_arities:
        return raw_args[:-1], raw_args[-1]
    return raw_args, None


def _append_typed_target(chain, value_type, *, target):
    if target == "push":
        return chain
    if target != _POLY_SYMBOL:
        raise CoeffProgramSourceError(f"unsupported typed target {target!r}")
    if value_type != "vector":
        raise CoeffProgramSourceError(f"{_POLY_SYMBOL} assignment requires a vector-valued expression")
    return chain + [["_typed_set_poly"]]


def _format_scalar_literal(value):
    re_part = 0.0 if value.real == 0 else float(value.real)
    im_part = 0.0 if value.imag == 0 else float(value.imag)
    if im_part >= 0:
        return f"{re_part!r}+{im_part!r}j"
    return f"{re_part!r}{im_part!r}j"


def _typed_lower_scalar(text):
    try:
        expr = ExpressionParser(_canonical_expr(text)).parse()
        static_value = expr_value_if_static(expr)
    except Exception as exc:
        raise CoeffProgramSourceError(str(exc))
    if static_value is not None:
        # Fold constant expressions to a single literal push so text-authored
        # constants cost one token and fingerprint identically to chip-
        # authored ones.
        return [["_typed_push_scalar", _format_scalar_literal(static_value)]]
    chain = []
    for token in expr.tokens:
        op = int(token.get("op") or 0)
        if op == EXPR_LITERAL:
            chain.append(["_typed_push_scalar", f"{token.get('a', 0.0)}+{token.get('b', 0.0)}j"])
        elif op in _EXPR_SCALAR_PUSHES:
            chain.append(["_typed_push_scalar", _EXPR_SCALAR_PUSHES[op]])
        elif op in _EXPR_INDEX_SOURCES:
            chain.extend(_typed_lower_vector_source(_EXPR_INDEX_SOURCES[op]))
            chain.append(["_typed_push_scalar", str(int(token.get("a") or 0))])
            chain.append(["_typed_get_scalar"])
        elif op in _EXPR_DYN_INDEX_SOURCES:
            chain.extend(_typed_lower_vector_source(_EXPR_DYN_INDEX_SOURCES[op]))
            chain.append(["swap"])
            chain.append(["_typed_get_scalar"])
        elif op in _EXPR_BINARY_CHIP_NAMES:
            chain.append(["_typed_binary", _EXPR_BINARY_CHIP_NAMES[op]])
        elif op in _EXPR_UNARY_CHIP_NAMES:
            chain.append(["_typed_unary", _EXPR_UNARY_CHIP_NAMES[op]])
        else:
            raise CoeffProgramSourceError(f"unsupported scalar expression opcode {op}")
    return chain


def _typed_lower_vector_source(name):
    raw = str(name or "").strip().lower()
    if raw not in _TYPED_VECTOR_SOURCE_NAMES:
        raise CoeffProgramSourceError(f"expected vector source, got {name!r}")
    if raw == "tos":
        raw = "peek"
    if raw not in _SOURCE_NAMES:
        raise CoeffProgramSourceError(f"expected vector source, got {name!r}")
    return [["_typed_push_vector", raw]]


def _typed_lower_index_reference(text):
    raw = str(text or "").strip()
    match = _INDEX_BASE_RE.match(raw)
    if not match or not raw.endswith("]"):
        return None
    # Only a bare reference when the opening bracket's match closes at the
    # very end. Otherwise this is a larger expression (e.g. cf[0]*cf[1]) and
    # the scalar lowering path owns it; a greedy regex used to swallow those.
    depth = 0
    open_idx = match.end() - 1
    close_idx = -1
    for idx in range(open_idx, len(raw)):
        ch = raw[idx]
        if ch == "[":
            depth += 1
        elif ch == "]":
            depth -= 1
            if depth == 0:
                close_idx = idx
                break
    if close_idx != len(raw) - 1:
        return None
    name = match.group(1).lower()
    index_expr = raw[open_idx + 1:close_idx].strip()
    if not index_expr:
        raise CoeffProgramSourceError(f"{name}[...] index expression is empty")
    chain = _typed_lower_vector_source(name)
    chain.extend(_typed_lower_scalar(index_expr))
    chain.append(["_typed_get_scalar"])
    return chain, "scalar"


def _typed_lower_fill(args):
    if len(args) == 1:
        length_expr = "poly_len"
        value_expr = args[0]
    elif len(args) == 2:
        length_expr, value_expr = args
    else:
        raise CoeffProgramSourceError("fill requires fill(value) or fill(length, value)")
    chain = _typed_lower_scalar(length_expr)
    chain.extend(_typed_lower_scalar(value_expr))
    chain.append(["_typed_fill"])
    return chain, "vector"


def _typed_lower_push_scalar(args):
    if len(args) != 1:
        raise CoeffProgramSourceError("push_scalar requires one value")
    return _typed_lower_scalar(args[0]), "scalar"


def _typed_lower_binary(name, args):
    if len(args) != 2:
        raise CoeffProgramSourceError(f"{name} requires two arguments in typed expressions")
    chip = _VECTOR_BINARY_ALIASES[name]
    chain, left_type = _typed_lower_value(args[0])
    right_chain, right_type = _typed_lower_value(args[1])
    chain.extend(right_chain)
    chain.append(["_typed_binary", chip])
    return chain, "vector" if "vector" in {left_type, right_type} else "scalar"


def _typed_lower_unary(name, args):
    if len(args) != 1:
        raise CoeffProgramSourceError(f"{name} requires one argument in typed expressions")
    chain, value_type = _typed_lower_value(args[0])
    chain.append(["_typed_unary", canonical_unary_op_name(name)])
    return chain, value_type


_AFFINE_SHORTHANDS = {
    # name -> (value word for diagnostics, typed binary chip)
    "scale": ("multiplier", "multiply"),
    "shift": ("offset", "add"),
}


def _typed_lower_affine(name, args):
    # The shorthand forms resolve args positionally, so a vector-source name
    # in a value slot (scale(poly), linear(poly, 5)) would silently bind the
    # source as a multiplier/offset; reject those shapes outright.
    if name in _AFFINE_SHORTHANDS:
        value_word, chip = _AFFINE_SHORTHANDS[name]
        if len(args) == 1:
            if _is_vector_source_text(args[0]):
                raise CoeffProgramSourceError(
                    f"{name}(source) is missing the {value_word}; "
                    f"write {name}(source, {value_word}) or {name}({value_word})"
                )
            src, value = "pop", args[0]
        elif len(args) == 2:
            if not _is_vector_source_text(args[0]):
                raise CoeffProgramSourceError(
                    f"{name} source must be one of {', '.join(sorted(_TYPED_VECTOR_SOURCE_NAMES))}, got {args[0]!r}"
                )
            src, value = args
        else:
            raise CoeffProgramSourceError(f"{name} requires {value_word} or source, {value_word}")
        return _typed_lower_binary(chip, [src, value])
    if len(args) == 2:
        if _is_vector_source_text(args[0]):
            raise CoeffProgramSourceError(
                "linear(source, multiplier) is ambiguous; use linear(source, multiplier, offset)"
            )
        src, multiplier, offset = "pop", args[0], args[1]
    elif len(args) == 3:
        if not _is_vector_source_text(args[0]):
            raise CoeffProgramSourceError(
                "linear with three values is read as (source, multiplier, offset); "
                "for (multiplier, offset, andy) use linear(source, multiplier, offset, andy)"
            )
        src, multiplier, offset = args
    else:
        raise CoeffProgramSourceError("linear requires multiplier, offset or source, multiplier, offset")
    chain, multiplied_type = _typed_lower_binary("multiply", [src, multiplier])
    offset_chain, offset_type = _typed_lower_value(offset)
    chain.extend(offset_chain)
    chain.append(["_typed_binary", "add"])
    return chain, "vector" if "vector" in {multiplied_type, offset_type} else "scalar"


def _typed_lower_value(text):
    raw = str(text or "").strip()
    if not raw:
        raise CoeffProgramSourceError("expression is empty")
    indexed = _typed_lower_index_reference(raw)
    if indexed is not None:
        return indexed
    lowered = raw.lower()
    if lowered in _TYPED_VECTOR_SOURCE_NAMES:
        return _typed_lower_vector_source(lowered), "vector"
    call = _parse_call(raw)
    if call:
        name, args = call
        if name in _VECTOR_FILL_NAMES:
            return _typed_lower_fill(args)
        if name == "push_scalar":
            return _typed_lower_push_scalar(args)
        if name in {"arange", "range", "push_range"}:
            return _lower_range("range", args, target="push"), "vector"
        if name in {"linspace", "push_linspace"}:
            return _lower_range("linspace", args, target="push"), "vector"
        if name in _VECTOR_BINARY_ALIASES:
            return _typed_lower_binary(name, args)
        if name in _VECTOR_UNARY_NAMES:
            return _typed_lower_unary(name, args)
        if name in {"scale", "shift", "linear"}:
            return _typed_lower_affine(name, args)
    return _typed_lower_scalar(raw), "scalar"


def _lower_const(args, *, target):
    chain, value_type = _typed_lower_fill(args)
    return _append_typed_target(chain, value_type, target=target)


def _lower_range(name, args, *, target):
    # Takes target like its sibling lowerers so callers cannot forget the
    # poly-assignment wrapper.
    if name == "linspace":
        if len(args) not in {1, 3}:
            raise CoeffProgramSourceError("linspace requires length or start, stop, count")
        chain = [["push_linspace"] + [_canonical_expr(arg) for arg in args]]
    else:
        if len(args) not in {1, 2, 3}:
            raise CoeffProgramSourceError("arange/range requires length, start/stop, or start/stop/step")
        chain = [["push_range"] + [_canonical_expr(arg) for arg in args]]
    return _append_typed_target(chain, "vector", target=target)


def _lower_affine(name, args, *, target):
    chain, value_type = _typed_lower_affine(name, args)
    return _append_typed_target(chain, value_type, target=target)


def _lower_vector_binary(name, args, *, target):
    chip = _VECTOR_BINARY_ALIASES[name]
    if len(args) == 0:
        chain = [["_typed_binary", chip]]
        return _append_typed_target(chain, "vector", target=target)
    elif len(args) == 2:
        if not (_is_source_name(args[0]) and _is_source_name(args[1])):
            chain, value_type = _typed_lower_binary(name, args)
            return _append_typed_target(chain, value_type, target=target)
        src1, src2 = _source_selector(args[0]), _source_selector(args[1])
    else:
        raise CoeffProgramSourceError(f"{name} requires no args or src1, src2")
    if src1 == "cf" or src2 == "cf":
        if src1 not in {"cf", "poly"} or src2 not in {"cf", "poly"}:
            raise CoeffProgramSourceError(
                f"{name} with cf can only stage cf/poly sources in v1"
            )
        staged = []
        # Pushed in reverse: the staged chip pops src1 first, then src2.
        for src in (src2, src1):
            staged.append(["push", src])
        staged.append([chip, target, "pop", "pop"])
        return staged
    return [[chip, target, src1, src2]]


def _lower_vector_unary(name, args, *, target):
    chip = canonical_unary_op_name(name)
    if len(args) == 0:
        chain = [["_typed_unary", chip]]
        return _append_typed_target(chain, "vector", target=target)
    elif len(args) == 1:
        if str(args[0]).strip().lower() == "cf" or not _is_source_name(args[0]):
            chain, value_type = _typed_lower_unary(name, args)
            return _append_typed_target(chain, value_type, target=target)
        # cf and non-source args were consumed above, so args[0] is a plain
        # poly/pop/peek selector here.
        src = _source_selector(args[0])
    else:
        raise CoeffProgramSourceError(f"{name} requires no args or one source")
    return [[chip, target, src]]


def _lower_native_transform_call(name, args, *, target):
    name = _canonical_native_name(name)
    if len(args) == 0:
        src = "pop"
        fn_args = []
    else:
        first = args[0].strip().lower()
        if first in _SOURCE_NAMES:
            src = _source_selector(first)
            fn_args = args[1:]
        else:
            src = "pop"
            fn_args = args
    fn_args, andy_arg = _split_native_transform_andy(name, fn_args)
    # The VM caps stack args per transform (e.g. round takes one packed
    # multiplier); over-limit arg lists must take the full-args token, which
    # knows the back-compat packings like round(a, b, andy).
    if fn_args and len(fn_args) <= native_transform_stack_arg_limit(name):
        try:
            chain = []
            for arg in fn_args:
                chain.extend(_typed_lower_scalar(arg))
            chip = ["_native_transform_stack_args", name, src, target, str(len(fn_args))]
            if andy_arg is not None:
                chip.append(_canonical_expr(andy_arg))
            chain.append(chip)
            return chain
        except CoeffProgramSourceError:
            # Args that are not scalar expressions (e.g. enum names like
            # hi/lo for roots) take the full-args token below, which
            # validates them against the registry declaration.
            pass
    fallback_args = list(fn_args)
    if andy_arg is not None:
        fallback_args.append(andy_arg)
    return [["_native_transform", name, src, target] + [_canonical_expr(arg) for arg in fallback_args]]


# Synthesizer-internal typed VM chips accepted on parse for round-trip closure.
# The frontend chip->source synthesizer (_coeffProgramSourceFromRows) and the
# backend chain->source serializer (coeff_source_text_from_chain) both fall back
# to emitting these "_typed_*" tokens for chip sequences with no high-level
# source form (e.g. a const polynomial's poke ladder). Coeff programs reach the
# native runtime only as source text, so the parser must accept them or such
# programs cannot compute at all. Mirrors the "_typed_*" entries in
# coeff_program_chain._CHIP_COMPILERS / _ZERO_ARG_CHIP_OPS; the drift guard
# test_typed_op_passthrough_covers_chain_compiler keeps these in sync.
_TYPED_OP_PASSTHROUGH_NAMES = frozenset({
    "_typed_push_scalar",
    "_typed_push_vector",
    "_typed_binary",
    "_typed_unary",
    "_typed_get_scalar",
    "_typed_set_poly",
    "_typed_poke_poly",
    "_typed_fill",
    "_typed_blend",
})


def _lower_call(name, args, *, target="push"):
    name = str(name or "").strip().lower()
    if name in _TYPED_OP_PASSTHROUGH_NAMES:
        # Not for hand-authoring, but emitted by chain->source serializers; pass
        # the row through verbatim and let the chain compiler validate operands.
        return [[name, *[str(arg) for arg in args]]]
    if name in _STACK_ALIASES:
        # Accept the call forms emit()/dup()/swap()/drop()/flush() alongside
        # the bare statements; every other zero-arg construct allows both.
        if args:
            raise CoeffProgramSourceError(f"{name} takes no arguments")
        if target == _POLY_SYMBOL:
            raise CoeffProgramSourceError(f"{name} is a statement; it cannot be assigned to {_POLY_SYMBOL}")
        return [[_STACK_ALIASES[name]]]
    if name in {"pop", "peek"}:
        if args:
            raise CoeffProgramSourceError(f"{name} takes no arguments")
        if target == _POLY_SYMBOL:
            return [["set", _POLY_SYMBOL, name]]
        raise CoeffProgramSourceError(
            f"{name} is not a standalone statement; use drop to discard the stack top, or write {_POLY_SYMBOL} = pop / {_POLY_SYMBOL} = peek"
        )
    if name in _VECTOR_FILL_NAMES:
        return _lower_const(args, target=target)
    if name == "push_scalar":
        chain, value_type = _typed_lower_push_scalar(args)
        return _append_typed_target(chain, value_type, target=target)
    if name in {"arange", "range", "push_range"}:
        return _lower_range("range", args, target=target)
    if name in {"linspace", "push_linspace"}:
        return _lower_range("linspace", args, target=target)
    if name in {"scale", "shift", "linear", "affine"}:
        if name == "affine":
            if target == _POLY_SYMBOL:
                raise CoeffProgramSourceError(
                    f"affine names its own target; write affine({_POLY_SYMBOL}, src, multiplier, offset) as a statement"
                )
            if len(args) != 4:
                raise CoeffProgramSourceError("affine requires target, source, multiplier, offset")
            return [["affine", _target_selector(args[0]), _source_selector(args[1]), _canonical_expr(args[2]), _canonical_expr(args[3])]]
        if name == "linear" and len(args) == 4 and _is_source_name(args[0]):
            # linear(src, multiplier, offset, andy): the typed affine lowering
            # has no andy; route to the native transform (registry fn 14).
            return _lower_native_transform_call(name, args, target=target)
        return _lower_affine(name, args, target=target)
    if name in _VECTOR_BINARY_ALIASES:
        return _lower_vector_binary(name, args, target=target)
    if name in _VECTOR_UNARY_NAMES:
        if len(args) > 1:
            if name == "exp":
                raise CoeffProgramSourceError("exp(x) is unary; use exp_affine(source, multiplier, offset[, andy]) for exp(source*multiplier+offset)")
            registry = legacy_registry()["by_name"]
            if name in registry and registry[name].get("supports_andy") and not registry[name].get("args"):
                # sin/cos/tan/sinh/cosh/tanh with a trailing andy reach the
                # native transform (registry fn 17-22); the bare unary forms
                # stay on the typed/vector unary path.
                return _lower_native_transform_call(name, args, target=target)
            raise CoeffProgramSourceError(f"{name} requires no args or one source")
        return _lower_vector_unary(name, args, target=target)
    if name in _NATIVE_TRANSFORM_ALIASES:
        return _lower_native_transform_call(name, args, target=target)
    if name in {"roll", "rolr"}:
        if len(args) == 1:
            src, n = "pop", args[0]
        elif len(args) == 2:
            src, n = _source_selector(args[0], allow_cf=False), args[1]
        else:
            raise CoeffProgramSourceError(f"{name} requires n or src, n")
        return [[name, target, src, _canonical_expr(n)]]
    if name == "argsort":
        if len(args) == 2:
            src1, src2 = _source_selector(args[0], allow_cf=False), _source_selector(args[1], allow_cf=False)
        else:
            raise CoeffProgramSourceError("argsort requires src1, src2")
        return [["argsort", target, src1, src2]]
    if name == "blend":
        if len(args) != 1:
            raise CoeffProgramSourceError("blend requires t")
        chain = _typed_lower_scalar(args[0])
        chain.append(["_typed_blend"])
        return _append_typed_target(chain, "vector", target=target)
    if name in {"poke_poly", "poke_tos"}:
        if target == _POLY_SYMBOL:
            raise CoeffProgramSourceError(
                f"{name} writes in place and returns nothing; use it as a statement"
            )
        if len(args) != 2:
            raise CoeffProgramSourceError(f"{name} requires index, value")
        if name == "poke_poly":
            chain = _typed_lower_scalar(args[0])
            chain.extend(_typed_lower_scalar(args[1]))
            chain.append(["_typed_poke_poly"])
            return chain
        return [[name, _canonical_expr(args[0]), _canonical_expr(args[1])]]
    if name == "littlewood":
        if len(args) not in {2, 3}:
            raise CoeffProgramSourceError("littlewood requires value1, value2, optional andy")
        return [["littlewood", target] + [_canonical_expr(arg) for arg in args]]
    if name == "macro":
        if target == _POLY_SYMBOL:
            raise CoeffProgramSourceError(
                "macro inlines another program and has no single value; use it as a statement"
            )
        if len(args) != 1:
            raise CoeffProgramSourceError("macro requires one program name")
        macro_name = args[0].strip()
        if not macro_name:
            raise CoeffProgramSourceError("macro name is empty")
        return [["macro", macro_name]]
    if name == "legacy":
        raise CoeffProgramSourceError(
            "legacy(...) is not valid in coeff source text; call the transform directly"
        )
    if name in legacy_registry()["by_name"]:
        return _lower_native_transform_call(name, args, target=target)
    raise CoeffProgramSourceError(f"unknown coeff program source function {name!r}")


def _legacy_lower_statement(statement):
    text = statement.text.strip()
    assignment = _find_top_level_assignment(text)
    if assignment >= 0:
        lhs = re.sub(r"\s+", "", text[:assignment])
        rhs = text[assignment + 1:].strip()
        if not rhs:
            raise CoeffProgramSourceError("assignment right-hand side is empty", line=statement.line, column=assignment + 2)
        indexed_lhs = _INDEXED_LHS_RE.match(lhs)
        if indexed_lhs:
            lhs_name = indexed_lhs.group(1).lower()
            index_expr = indexed_lhs.group(2).strip()
            if lhs_name not in _WRITABLE_LHS_NAMES:
                raise CoeffProgramSourceError(
                    f"{lhs_name}[...] is read-only in Coeff Program source",
                    line=statement.line,
                    column=statement.column,
                )
            if not index_expr:
                raise CoeffProgramSourceError(f"{lhs_name}[...] index expression is empty", line=statement.line, column=statement.column)
            if index_expr.isdigit():
                idx = int(index_expr)
                if idx < 0 or idx >= MAX_VECTOR_LEN:
                    raise CoeffProgramSourceError(
                        f"{lhs_name} index must be in [0,{MAX_VECTOR_LEN - 1}], got {idx}",
                        line=statement.line,
                        column=statement.column,
                    )
            chain = _typed_lower_scalar(index_expr)
            chain.extend(_typed_lower_scalar(rhs))
            chain.append(["_typed_poke_poly"])
            return chain
        if lhs.lower() not in _WRITABLE_LHS_NAMES:
            raise CoeffProgramSourceError(
                f"only {', '.join(sorted(_WRITABLE_LHS_NAMES))} assignments are supported in v1",
                line=statement.line,
                column=statement.column,
            )
        call = _parse_call(rhs)
        if call:
            return _lower_call(call[0], call[1], target=lhs.lower())
        bare = rhs.strip().lower()
        if bare in _SOURCE_NAMES:
            return [["set", lhs.lower(), bare]]
        if _IDENT_RE.match(bare):
            return _lower_call(bare, [], target=lhs.lower())
        chain, value_type = _typed_lower_value(rhs)
        return _append_typed_target(chain, value_type, target=lhs.lower())

    call = _parse_call(text)
    if call:
        return _lower_call(call[0], call[1], target="push")
    bare = text.strip().lower()
    if bare in _PUSH_SOURCE_NAMES:
        return [["push", bare]]
    if bare in _STACK_ALIASES:
        return [[_STACK_ALIASES[bare]]]
    if bare in {"pop", "peek"}:
        hint = f"use drop to discard the stack top, or write {_POLY_SYMBOL} = pop / {_POLY_SYMBOL} = peek explicitly"
        raise CoeffProgramSourceError(f"{bare} is not a standalone statement; {hint}", line=statement.line, column=statement.column)
    if _IDENT_RE.match(bare):
        return _lower_call(bare, [], target="push")
    raise CoeffProgramSourceError(f"cannot parse coeff program statement {text!r}", line=statement.line, column=statement.column)


class _CoeffStatementLowerer(ProfileStatementLowerer):
    """Coeff source semantics through the shared statement dispatcher."""

    def __init__(self):
        super().__init__(_PROFILE, error_cls=CoeffProgramSourceError)

    def lower_indexed_assignment(self, statement, lhs_name, index_expr, rhs):
        if lhs_name not in _WRITABLE_LHS_NAMES:
            raise CoeffProgramSourceError(
                f"{lhs_name}[...] is read-only in Coeff Program source",
                line=statement.line,
                column=statement.column,
            )
        if index_expr.isdigit():
            idx = int(index_expr)
            if idx < 0 or idx >= MAX_VECTOR_LEN:
                raise CoeffProgramSourceError(
                    f"{lhs_name} index must be in [0,{MAX_VECTOR_LEN - 1}], got {idx}",
                    line=statement.line,
                    column=statement.column,
                )
        chain = _typed_lower_scalar(index_expr)
        chain.extend(_typed_lower_scalar(rhs))
        chain.append(["_typed_poke_poly"])
        return chain

    def lower_symbol_assignment(self, statement, lhs, rhs):
        call = _parse_call(rhs)
        if call:
            return _lower_call(call[0], call[1], target=lhs)
        bare = rhs.strip().lower()
        if bare in _SOURCE_NAMES:
            return [["set", lhs, bare]]
        if _IDENT_RE.match(bare):
            return _lower_call(bare, [], target=lhs)
        chain, value_type = _typed_lower_value(rhs)
        return _append_typed_target(chain, value_type, target=lhs)

    def lower_call_statement(self, statement, name, args):
        return _lower_call(name, args, target="push")

    def lower_bare_statement(self, statement, text):
        bare = text.strip().lower()
        if bare in _PUSH_SOURCE_NAMES:
            return [["push", bare]]
        if bare in _STACK_ALIASES:
            return [[_STACK_ALIASES[bare]]]
        if bare in {"pop", "peek"}:
            hint = f"use drop to discard the stack top, or write {_POLY_SYMBOL} = pop / {_POLY_SYMBOL} = peek explicitly"
            raise CoeffProgramSourceError(
                f"{bare} is not a standalone statement; {hint}",
                line=statement.line,
                column=statement.column,
            )
        if _IDENT_RE.match(bare):
            return _lower_call(bare, [], target="push")
        raise CoeffProgramSourceError(
            f"cannot parse coeff program statement {text!r}",
            line=statement.line,
            column=statement.column,
        )


_COEFF_STATEMENT_LOWERER = _CoeffStatementLowerer()


def _lower_statement(statement):
    return _COEFF_STATEMENT_LOWERER.lower_statement(statement)


def split_coeff_program_statements(source_text):
    """Split source text into _Statement(text, line, column) records.

    Statements break on newlines and semicolons at bracket depth zero;
    # comments run to end of line; parenthesized statements may span lines.
    Mirrored by _coeffProgramSourceStatements in index.html.
    """
    return split_program_statements(
        source_text,
        error_cls=CoeffProgramSourceError,
        max_bytes=MAX_COEFF_PROGRAM_SOURCE_BYTES,
    )


def coeff_source_text_from_payload(payload):
    """Source-vs-chain precedence for saved coeff-program payloads.

    Returns the source text when it is authoritative — non-blank, or present
    with no chain to fall back to — else None, meaning use the chain. This is
    the same rule the UI applies on load and the compute request paths apply
    to run params: a payload carrying both keys with a blank source_text
    (e.g. an imported or hand-edited S3 object) must compile its chain, not
    silently become an empty program.
    """
    if not isinstance(payload, dict) or "source_text" not in payload:
        return None
    raw = str(payload.get("source_text") or "")
    if raw.strip() or not payload.get("chain"):
        return raw
    return None


def parse_coeff_program_source(source_text, *, strict=True):
    """Lower source text to a chip chain: {chain, display, statement_count,
    diagnostics}.

    The shared parser core owns splitting, diagnostic shape, and strict vs
    non-strict behavior. Coeff supplies only profile-specific semantic hooks.
    """
    return parse_profile_source(
        source_text,
        lowerer=_COEFF_STATEMENT_LOWERER,
        display_fn=display_coeff_program_chain,
        error_cls=CoeffProgramSourceError,
        compile_error_cls=CoeffProgramSourceCompileError,
        max_bytes=MAX_COEFF_PROGRAM_SOURCE_BYTES,
        strict=strict,
    )


def compile_coeff_program_source(source_text, *, macro_resolver=None, strict=True):
    """Parse + compile in one call: the chain compiler's payload plus
    source_text/source_display/source_statement_count and merged diagnostics.

    Production handlers currently compose parse_coeff_program_source and
    compile_coeff_program_chain themselves; this wrapper is the convenience
    form (used by the native tests).
    """
    parsed = parse_coeff_program_source(source_text, strict=strict)
    compiled = compile_coeff_program_chain(parsed["chain"], macro_resolver=macro_resolver, strict=strict)
    compiled["source_text"] = str(source_text or "")
    compiled["source_display"] = parsed["display"]
    compiled["source_statement_count"] = parsed["statement_count"]
    compiled["diagnostics"] = list(parsed.get("diagnostics") or []) + list(compiled.get("diagnostics") or [])
    return compiled


def _source_call(name, args):
    return f"{name}({', '.join(str(arg) for arg in args)})" if args else str(name)


def _chain_row_name_args(row):
    if isinstance(row, str):
        return row, []
    if isinstance(row, list) and row:
        return str(row[0]), [str(arg) for arg in row[1:]]
    return str(row), []


def _native_transform_source_line(name, src, tgt, args):
    name = _source_native_name(name, args)
    src = str(src or "pop").strip().lower()
    tgt = str(tgt or "push").strip().lower()
    call_args = []
    if src != "pop" or args:
        call_args.append(src)
    call_args.extend(str(arg) for arg in (args or []))
    call = _source_call(name, call_args)
    return f"poly = {call}" if tgt == "poly" else call


def _source_native_name(name, args=None):
    """Return the source-safe transform name for a canonical registry name."""
    canonical = _canonical_native_name(name)
    argc = len(args or [])
    # exp/pow/power are shadowed by typed source syntax. Use the text aliases
    # so regenerated source parses back to the native transform, not to the
    # typed unary/binary forms.
    if canonical == "exp" and argc:
        return "exp_affine"
    if canonical == "pow":
        return "pow_affine"
    if canonical == "power" and argc:
        return "power_series"
    return canonical


_SCALAR_BINARY_INFIX = {
    "add": "+",
    "subtract": "-",
    "multiply": "*",
    "divide": "/",
    "power": "**",
}


def _typed_source_binary_expr(name, left, right, result_type):
    chip = _VECTOR_BINARY_ALIASES.get(str(name or "").strip().lower(), str(name or "").strip().lower())
    if result_type == "scalar" and chip in _SCALAR_BINARY_INFIX:
        return f"({left}{_SCALAR_BINARY_INFIX[chip]}{right})"
    return _source_call(chip, [left, right])


def _typed_source_unary_expr(name, value):
    return _source_call(canonical_unary_op_name(str(name or "").strip().lower()), [value])


def _source_scalar_text(text):
    raw = _canonical_expr(text)
    try:
        static_value = expr_value_if_static(ExpressionParser(raw).parse())
    except Exception:
        static_value = None
    if static_value is None:
        return raw
    if static_value.imag == 0:
        real_value = 0.0 if static_value.real == 0 else float(static_value.real)
        return f"{real_value!r}"
    return _format_scalar_literal(static_value)


def _typed_pending_line(kind, text):
    if kind == "scalar":
        return _source_call("push_scalar", [text])
    raw = str(text or "").strip()
    if raw in {"cf", "poly"}:
        return raw
    return raw


def coeff_source_text_from_chain(chain):
    """Render a canonical, reparseable Coeff source approximation.

    This is for migration/populate flows. It is intentionally separate from
    display_coeff_program_chain, which is a log/UI display string.
    """
    if not isinstance(chain, list):
        return ""
    lines = []

    # Pending source values mirror the VM stack only while a lowered typed
    # sequence is being decompiled. They are flushed before user-visible stack
    # operations (drop/dup/native calls) so canonical regeneration preserves
    # explicit stack effects instead of optimizing them away.
    pending = []

    def push_pending(kind, text):
        pending.append((kind, str(text)))

    def pop_pending(expected=None):
        if not pending:
            return None
        item = pending.pop()
        if expected and item[0] != expected:
            pending.append(item)
            return None
        return item

    def flush_pending():
        while pending:
            kind, text = pending.pop(0)
            lines.append(_typed_pending_line(kind, text))

    def materialize_vector_for_index(kind, text):
        raw = str(text).strip().lower()
        if raw == "peek":
            return "tos"
        if raw in {"cf", "poly", "tos"}:
            return raw
        flush_pending()
        lines.append(_typed_pending_line(kind, text))
        return "tos"

    def source_vector_from_selector(src):
        src = str(src or "").strip().lower()
        if src == "tos":
            src = "peek"
        if src == "pop" and pending:
            return pop_pending("vector")
        if src == "peek":
            # Preserve the VM source selector. Replacing peek with the known
            # pending vector (e.g. cf) is semantically equivalent in many
            # cases, but it changes the compiled opcode from TOS_AT to CF_AT.
            return ("vector", "tos")
        return ("vector", "tos" if src == "peek" else src)

    def pop_stack_arg_scalars(count):
        values = []
        for _ in range(max(0, int(count or 0))):
            item = pop_pending("scalar")
            if not item:
                break
            values.append(item[1])
        values.reverse()
        return values

    for row in chain:
        name, args = _chain_row_name_args(row)
        lname = name.strip().lower()

        if lname == "_typed_push_scalar" and len(args) == 1:
            push_pending("scalar", _source_scalar_text(args[0]))
        elif lname == "_typed_push_vector" and len(args) == 1:
            kind, text = source_vector_from_selector(args[0])
            push_pending(kind, text)
        elif lname == "_typed_binary" and len(args) == 1:
            right = pop_pending()
            left = pop_pending()
            if not left or not right:
                flush_pending()
                lines.append(_source_call(_VECTOR_BINARY_ALIASES.get(args[0], args[0]), []))
                continue
            result_type = "vector" if "vector" in {left[0], right[0]} else "scalar"
            push_pending(result_type, _typed_source_binary_expr(args[0], left[1], right[1], result_type))
        elif lname == "_typed_unary" and len(args) == 1:
            value = pop_pending()
            if not value:
                flush_pending()
                lines.append(_source_call(canonical_unary_op_name(args[0]), []))
                continue
            push_pending(value[0], _typed_source_unary_expr(args[0], value[1]))
        elif lname == "_typed_get_scalar" and not args:
            index = pop_pending("scalar")
            vector = pop_pending("vector")
            if not index or not vector:
                flush_pending()
                lines.append("_typed_get_scalar")
                continue
            source = materialize_vector_for_index(vector[0], vector[1])
            push_pending("scalar", f"{source}[{index[1]}]")
        elif lname == "_typed_set_poly" and not args:
            value = pop_pending("vector")
            if not value:
                flush_pending()
                lines.append("_typed_set_poly")
                continue
            flush_pending()
            lines.append(f"poly = {value[1]}")
        elif lname == "_typed_poke_poly" and not args:
            value = pop_pending("scalar")
            index = pop_pending("scalar")
            if not index or not value:
                flush_pending()
                lines.append("_typed_poke_poly")
                continue
            flush_pending()
            lines.append(f"poly[{index[1]}] = {value[1]}")
        elif lname == "_typed_fill" and not args:
            value = pop_pending("scalar")
            length = pop_pending("scalar")
            if not length or not value:
                flush_pending()
                lines.append("fill()")
                continue
            push_pending("vector", _source_call("fill", [length[1], value[1]]))
        elif lname == "_typed_blend" and not args:
            t = pop_pending("scalar")
            if not t:
                flush_pending()
                lines.append("blend()")
                continue
            # blend(t) consumes two vectors from the runtime stack. Materialize
            # any pending vectors first, then keep the result pending so a
            # following _typed_set_poly can render `poly = blend(t)`.
            flush_pending()
            push_pending("vector", _source_call("blend", [t[1]]))
        elif lname == "swap" and not args and len(pending) >= 2:
            pending[-1], pending[-2] = pending[-2], pending[-1]
        elif lname in {"cf", "poly"} and not args:
            push_pending("vector", lname)
        elif lname == "push" and len(args) == 1:
            push_pending("vector", args[0].strip().lower())
        elif lname == "emit" and not args:
            flush_pending()
            lines.append("emit")
        elif lname == "pop" and not args:
            flush_pending()
            lines.append("drop")
        elif lname in {"duplicate", "dup", "swap", "flush"} and not args:
            flush_pending()
            lines.append("dup" if lname == "duplicate" else lname)
        elif lname == "set" and len(args) == 2:
            flush_pending()
            lines.append(f"{args[0].strip().lower()} = {args[1].strip().lower()}")
        elif lname in {"const", "push_const", "push_vec", "fill"}:
            if len(args) == 1:
                push_pending("vector", _source_call("fill", ["poly_len", args[0]]))
            else:
                push_pending("vector", _source_call("fill", args))
        elif lname in {"push_linspace", "linspace"}:
            push_pending("vector", _source_call("linspace", args))
        elif lname in {"push_range", "range", "arange"}:
            push_pending("vector", _source_call("arange", args))
        elif lname in {"legacy", "_native_transform"} and len(args) >= 3:
            flush_pending()
            line = _native_transform_source_line(args[0], args[1], args[2], args[3:])
            if args[2].strip().lower() == "push":
                push_pending("vector", line)
            else:
                lines.append(line)
        elif lname == "_native_transform_stack_args" and len(args) >= 4:
            fn_name, src, tgt, count = args[0], args[1], args[2], int(args[3] or 0)
            fn_args = pop_stack_arg_scalars(count)
            if len(args) > 4:
                fn_args.append(args[4])
            flush_pending()
            line = _native_transform_source_line(fn_name, src, tgt, fn_args)
            if tgt.strip().lower() == "push":
                push_pending("vector", line)
            else:
                lines.append(line)
        elif lname in _VECTOR_BINARY_ALIASES and len(args) == 3:
            flush_pending()
            call = _source_call(lname, [args[1], args[2]])
            if args[0].strip().lower() == "push":
                push_pending("vector", call)
            else:
                lines.append(f"{args[0].strip().lower()} = {call}")
        elif lname in _VECTOR_UNARY_NAMES and len(args) == 2:
            flush_pending()
            call = _source_call(lname, [args[1]])
            if args[0].strip().lower() == "push":
                push_pending("vector", call)
            else:
                lines.append(f"{args[0].strip().lower()} = {call}")
        elif lname in {"roll", "rolr"} and len(args) == 3:
            flush_pending()
            lines.append(f"{args[0].strip().lower()} = {_source_call(lname, [args[1], args[2]])}")
        elif lname == "argsort" and len(args) == 3:
            flush_pending()
            lines.append(f"{args[0].strip().lower()} = {_source_call(lname, [args[1], args[2]])}")
        elif lname in {"poke_poly", "poke_tos", "littlewood", "macro"}:
            flush_pending()
            lines.append(_source_call(lname, args))
        else:
            flush_pending()
            lines.append(_source_call(lname, args))
    flush_pending()
    return "\n".join(lines)
