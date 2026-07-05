"""Authoritative source-text parser for Param Program.

Source text lowers to the existing param_program_chain chip list. The existing
chain compiler remains the only producer of VM tokens, execution specs, and
fingerprints.
"""
from __future__ import annotations

import re
import warnings

from param_program_chain import (
    _canonicalize_legacy_bridge_entry,
    compile_param_program_chain,
    display_param_program_chain,
    legacy_registry,
)
from program_source_core import (
    ProfileStatementLowerer,
    ProgramSourceError,
    diagnostic,
    find_top_level_assignment,
    parse_profile_source,
    program_profile,
    profile_source,
    profile_symbol,
)


MAX_PARAM_PROGRAM_SOURCE_BYTES = 64 * 1024

_PROFILE = program_profile("param")
_SOURCE = profile_source(_PROFILE)
_OUTPUT_SYMBOLS = {
    name
    for name, spec in (_PROFILE.get("symbols") or {}).items()
    if spec.get("role") == "output" and spec.get("access") == "read_write"
}
_STACK_OP_ALIASES = {
    str(key): str(value)
    for key, value in (_SOURCE.get("stack_op_aliases") or {}).items()
}
_BINARY_OPS = {str(name) for name in (_SOURCE.get("binary_ops") or [])}
_UNARY_OPS = {str(name) for name in (_SOURCE.get("unary_ops") or [])}
_TARGETABLE_UNARY = {str(name) for name in (_SOURCE.get("targetable_unary") or [])}
_PUSH_SOURCES = {str(name) for name in (_SOURCE.get("push_sources") or [])}
_EMIT_ALIASES = {
    str(key): str(value)
    for key, value in (_SOURCE.get("emit_aliases") or {}).items()
}
_LEGACY_SRC_SELECTORS = {"p1", "p2", "both", "pop1", "pop2"}
_LEGACY_TGT_SELECTORS = {"p1", "p2", "both", "push1", "push2"}


class ParamProgramSourceError(ProgramSourceError):
    pass


class ParamProgramSourceCompileError(RuntimeError):
    """Strict-mode source compile failure with structured diagnostics."""

    def __init__(self, diagnostics):
        self.diagnostics = list(diagnostics or [])
        message = "; ".join(d["message"] for d in self.diagnostics if d.get("level") == "error")
        super().__init__(
            f"invalid param_program_source_text: {message}"
            if message
            else "invalid param_program_source_text"
        )


def _canonical_expr(text):
    raw = str(text or "").strip()
    if not raw:
        raise ParamProgramSourceError("expression is empty", code="empty_expression")
    return re.sub(r"\s+", "", raw)


def _require_writable_symbol(name, stmt):
    raw = str(name or "").strip().lower()
    if raw in _OUTPUT_SYMBOLS:
        return raw
    if profile_symbol(_PROFILE, raw):
        raise ParamProgramSourceError(
            f"{raw} is read-only in Param Program source; assign to p1 or p2",
            line=stmt.line,
            column=stmt.column,
            code="read_only_symbol",
        )
    raise ParamProgramSourceError(
        f"unknown Param Program output symbol: {name}",
        line=stmt.line,
        column=stmt.column,
        code="unknown_symbol",
    )


def _legacy_allowed_arg_counts(spec):
    # Variable-arity forms come from the registry compat block — the same
    # data the chain compiler enforces. Never shadow it with local literals:
    # a hardcoded copy here once rejected legacy(inv_t_plus_2, ..., 3 args)
    # that the chain compiler accepted.
    variable = legacy_registry()["compat"]["variable_arg_counts"].get(spec["name"])
    if variable:
        return set(variable)
    return set(range(0, len(spec.get("args") or []) + 1))


def _validate_legacy_source_entry(entry):
    if not entry:
        raise ParamProgramSourceError("legacy transform is empty", code="bad_arity")
    registry = legacy_registry()["by_name"]
    if entry[0] == "legacy":
        if len(entry) < 4:
            raise ParamProgramSourceError("legacy(name, src, tgt, ...) requires at least 3 args", code="bad_arity")
        legacy_name = str(entry[1] or "").strip().lower()
        src = str(entry[2] or "").strip().lower()
        tgt = str(entry[3] or "").strip().lower()
        raw_args = list(entry[4:])
    else:
        legacy_name = str(entry[0] or "").strip().lower()
        src = "both"
        tgt = "both"
        raw_args = list(entry[1:])
    spec = registry.get(legacy_name)
    if spec is None:
        raise ParamProgramSourceError(
            f"unknown legacy param transform: {legacy_name}",
            code="unknown_legacy_transform",
        )
    if src not in _LEGACY_SRC_SELECTORS:
        raise ParamProgramSourceError(
            f"legacy({legacy_name}) src must be one of: {', '.join(sorted(_LEGACY_SRC_SELECTORS))}",
            code="bad_selector",
        )
    if tgt not in _LEGACY_TGT_SELECTORS:
        raise ParamProgramSourceError(
            f"legacy({legacy_name}) tgt must be one of: {', '.join(sorted(_LEGACY_TGT_SELECTORS))}",
            code="bad_selector",
        )
    if src not in spec["allowed_src"]:
        raise ParamProgramSourceError(
            f"legacy({legacy_name}) does not support src={src}; allowed: {', '.join(spec['allowed_src'])}",
            code="bad_selector",
        )
    if tgt not in spec["allowed_tgt"]:
        raise ParamProgramSourceError(
            f"legacy({legacy_name}) does not support tgt={tgt}; allowed: {', '.join(spec['allowed_tgt'])}",
            code="bad_selector",
        )
    allowed_counts = _legacy_allowed_arg_counts(spec)
    if len(raw_args) not in allowed_counts:
        expected = ", ".join(str(n) for n in sorted(allowed_counts))
        raise ParamProgramSourceError(
            f"legacy({legacy_name}) got {len(raw_args)} argument(s); expected one of: {expected}",
            code="bad_arity",
        )
    return entry


def _lower_call(stmt, name, args):
    if name == "push":
        if len(args) > 1:
            raise ParamProgramSourceError("push() takes at most one source", code="bad_arity")
        if not args:
            return [["push"]]
        src = str(args[0] or "").strip().lower()
        if src not in _PUSH_SOURCES:
            raise ParamProgramSourceError("push(source) source must be t1 or t2", code="bad_selector")
        return [["push", src]]
    if name == "const":
        if len(args) != 1:
            raise ParamProgramSourceError("const(value) takes exactly one expression", code="bad_arity")
        return [["const", _canonical_expr(args[0])]]
    if name == "emit":
        raise ParamProgramSourceError(
            "emit(p1) is not canonical Param source; use emit_p1, emit_p2, or p1 = expr",
            code="noncanonical_emit",
        )
    if name in _EMIT_ALIASES and not args:
        return [["emit", _EMIT_ALIASES[name]]]
    if name in _STACK_OP_ALIASES:
        if args:
            raise ParamProgramSourceError(f"{name} takes no arguments", code="bad_arity")
        return [[_STACK_OP_ALIASES[name]]]
    if name in _BINARY_OPS:
        if args:
            raise ParamProgramSourceError(f"{name} takes no arguments", code="bad_arity")
        return [[name]]
    if name in _UNARY_OPS:
        if not args:
            return [[name]]
        if name not in _TARGETABLE_UNARY or len(args) != 1:
            raise ParamProgramSourceError(f"{name} takes no arguments or one p1/p2 target", code="bad_arity")
        target = _require_writable_symbol(args[0], stmt)
        return [[name, target]]
    if name == "legacy":
        if len(args) < 3:
            raise ParamProgramSourceError("legacy(name, src, tgt, ...) requires at least 3 args", code="bad_arity")
        legacy_name = str(args[0] or "").strip().lower()
        entry = ["legacy", legacy_name, args[1].strip().lower(), args[2].strip().lower()] + [
            _canonical_expr(arg) for arg in args[3:]
        ]
        entry = _canonicalize_legacy_bridge_entry(entry)
        return [_validate_legacy_source_entry(entry)]
    if name == "macro":
        if len(args) != 1:
            raise ParamProgramSourceError("macro(name) takes exactly one name", code="bad_arity")
        macro_name = str(args[0] or "").strip()
        if not macro_name:
            raise ParamProgramSourceError("macro name is empty", code="empty_macro")
        return [["macro", macro_name]]
    if name in legacy_registry()["by_name"]:
        entry = _canonicalize_legacy_bridge_entry([name] + [_canonical_expr(arg) for arg in args])
        return [_validate_legacy_source_entry(entry)]
    raise ParamProgramSourceError(f"unknown Param Program command: {name}", code="unknown_command")


def _lower_bare(stmt):
    raw = stmt.text.strip()
    name = raw.lower()
    if name == "push":
        return [["push"]]
    if name in _EMIT_ALIASES:
        return [["emit", _EMIT_ALIASES[name]]]
    if name in _STACK_OP_ALIASES:
        return [[_STACK_OP_ALIASES[name]]]
    if name in _BINARY_OPS or name in _UNARY_OPS:
        return [[name]]
    if name in legacy_registry()["by_name"]:
        return [_validate_legacy_source_entry([name])]
    raise ParamProgramSourceError(f"unknown Param Program statement: {raw}", code="unknown_statement")


# Names local aliases must not shadow. Substitution is whole-word textual,
# so any word with meaning anywhere in Param source — statement names,
# expression identifiers, selector args like `both`/`pop1`, legacy transform
# names — is off-limits. Over-reserving is harmless.
_LOCALS_RESERVED_EXTRA = frozenset({
    "legacy", "t1", "t2", "p1", "p2", "i", "j",
    "pi", "pi2", "pi2i",
    "conj", "neg", "real", "imag", "abs", "mod", "log", "sqrt",
    "exp", "sin", "cos", "tan", "sinh", "cosh", "tanh", "angle",
})

_LOCALS_RESERVED_CACHE = None


def _locals_reserved_names():
    global _LOCALS_RESERVED_CACHE
    if _LOCALS_RESERVED_CACHE is None:
        names = set(_LOCALS_RESERVED_EXTRA)
        names |= set(_OUTPUT_SYMBOLS)
        names |= set(_STACK_OP_ALIASES) | set(_STACK_OP_ALIASES.values())
        names |= set(_BINARY_OPS) | set(_UNARY_OPS) | set(_TARGETABLE_UNARY)
        names |= set(_PUSH_SOURCES)
        names |= set(_EMIT_ALIASES) | set(_EMIT_ALIASES.values())
        names |= _LEGACY_SRC_SELECTORS | _LEGACY_TGT_SELECTORS
        names |= set(legacy_registry()["by_name"])
        _LOCALS_RESERVED_CACHE = frozenset(str(n).lower() for n in names)
    return _LOCALS_RESERVED_CACHE


class ParamStatementLowerer(ProfileStatementLowerer):
    def __init__(self):
        super().__init__(_PROFILE, error_cls=ParamProgramSourceError)

    def reserved_local_names(self):
        return _locals_reserved_names()

    def reserved_local_patterns(self):
        return (r"^(p|t)\d+$",)

    def lower_statement(self, statement):
        text = statement.text.strip()
        assignment = find_top_level_assignment(text)
        if assignment >= 0 and not text[assignment + 1:].strip():
            raise self.source_error(
                "expression is empty",
                line=statement.line,
                column=assignment + 2,
                code="empty_expression",
            )
        return super().lower_statement(statement)

    def lower_assignment(self, statement, lhs, rhs):
        if self._indexed_lhs_re and self._indexed_lhs_re.match(lhs):
            return super().lower_assignment(statement, lhs, rhs)
        target = _require_writable_symbol(lhs, statement)
        return self.lower_symbol_assignment(statement, target, rhs)

    def lower_indexed_assignment(self, statement, lhs_name, index_expr, rhs):
        raise self.source_error(
            f"{lhs_name}[...] is not valid Param Program source; assign to p1 or p2",
            line=statement.line,
            column=statement.column,
            code="unknown_symbol",
        )

    def lower_symbol_assignment(self, statement, lhs, rhs):
        return [["const", _canonical_expr(rhs)], ["emit", lhs]]

    def lower_call_statement(self, statement, name, args):
        return _lower_call(statement, name, args)

    def lower_bare_statement(self, statement, text):
        return _lower_bare(statement)


def parse_param_program_source(source_text, *, strict=True):
    parsed = parse_profile_source(
        source_text,
        lowerer=ParamStatementLowerer(),
        display_fn=display_param_program_chain,
        error_cls=ParamProgramSourceError,
        compile_error_cls=ParamProgramSourceCompileError,
        max_bytes=MAX_PARAM_PROGRAM_SOURCE_BYTES,
        strict=strict,
    )
    if not parsed["chain"] and not parsed["diagnostics"]:
        parsed["diagnostics"].append(
            diagnostic("param program source is empty", line=1, column=1, code="empty_source")
        )
        if strict:
            raise ParamProgramSourceCompileError(parsed["diagnostics"])
    has_errors = any(d.get("level") == "error" for d in parsed["diagnostics"])
    if has_errors:
        parsed["chain"] = []
        parsed["display"] = ""
    return parsed


def compile_param_program_source(source_text, *, macro_resolver=None, strict=True):
    parsed = parse_param_program_source(source_text, strict=strict)
    compiled = compile_param_program_chain(parsed["chain"], macro_resolver=macro_resolver, strict=strict)
    out = dict(compiled)
    out["chain"] = parsed["chain"]
    out["source_text"] = str(source_text or "")
    out["source_display"] = parsed.get("display") or compiled.get("display") or ""
    out["source_statement_count"] = parsed.get("statement_count") or 0
    out["diagnostics"] = list(parsed.get("diagnostics") or []) + list(compiled.get("diagnostics") or [])
    return out


def param_source_text_from_payload(payload):
    if not isinstance(payload, dict) or "source_text" not in payload:
        return None
    raw = str(payload.get("source_text") or "")
    if raw.strip() or not payload.get("chain"):
        return raw
    return None


def _chip_name_and_args(chip):
    if isinstance(chip, str):
        return chip, []
    if isinstance(chip, list) and chip:
        return str(chip[0]), [str(arg) for arg in chip[1:]]
    return str(chip), []


def _call(name, args):
    return f"{name}({', '.join(args)})" if args else str(name)


def _raw_chain_source_text(chain):
    lines = []
    for row in chain or ():
        name, args = _chip_name_and_args(row)
        lines.append(_call(name, args))
    return "\n".join(line for line in lines if str(line).strip())


def _source_text_preserves_compiled_chain(compiled_chain, source_text):
    try:
        from_source = compile_param_program_source(source_text)
    except Exception:
        return False
    return (
        from_source.get("fingerprint") == compiled_chain.get("fingerprint")
        and from_source.get("tokens") == compiled_chain.get("tokens")
    )


def param_source_text_from_chain(chain):
    if not isinstance(chain, list):
        return ""
    lines = []
    idx = 0
    while idx < len(chain):
        name, args = _chip_name_and_args(chain[idx])
        lname = name.lower()
        if lname == "const" and len(args) == 1 and idx + 1 < len(chain):
            next_name, next_args = _chip_name_and_args(chain[idx + 1])
            if next_name.lower() == "emit" and len(next_args) == 1 and next_args[0].lower() in _OUTPUT_SYMBOLS:
                lines.append(f"{next_args[0].lower()} = {args[0]}")
                idx += 2
                continue
        if lname == "const" and len(args) == 2 and idx + 1 < len(chain):
            next_name, next_args = _chip_name_and_args(chain[idx + 1])
            if next_name.lower() == "emit" and len(next_args) == 1 and next_args[0].lower() in _OUTPUT_SYMBOLS:
                lines.append(f"{next_args[0].lower()} = ({args[0]})+({args[1]})*1j")
                idx += 2
                continue
        if lname == "push":
            lines.append(_call("push", [arg.lower() for arg in args]))
        elif lname == "emit" and len(args) == 1 and args[0].lower() in _OUTPUT_SYMBOLS:
            lines.append(f"emit_{args[0].lower()}")
        elif lname in {"duplicate", "dup"}:
            lines.append("dup")
        elif lname in _STACK_OP_ALIASES or lname in _BINARY_OPS or lname in _UNARY_OPS:
            lines.append(_call(lname, args))
        elif lname == "legacy" and len(args) >= 3:
            lines.append(_call("legacy", args))
        elif lname == "macro":
            lines.append(_call("macro", args))
        else:
            lines.append(_call(lname, args))
        idx += 1
    candidate = "\n".join(lines)
    try:
        compiled_chain = compile_param_program_chain(chain)
    except Exception:
        warnings.warn(
            "param_source_text_from_chain could not compile input chain; returning readable candidate",
            RuntimeWarning,
            stacklevel=2,
        )
        return candidate
    if _source_text_preserves_compiled_chain(compiled_chain, candidate):
        return candidate
    raw = _raw_chain_source_text(chain)
    if raw and raw != candidate and _source_text_preserves_compiled_chain(compiled_chain, raw):
        return raw
    warnings.warn(
        "param_source_text_from_chain could not produce fingerprint-preserving source; returning readable candidate",
        RuntimeWarning,
        stacklevel=2,
    )
    return candidate
