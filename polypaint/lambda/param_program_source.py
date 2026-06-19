"""Authoritative source-text parser for Param Program.

Source text lowers to the existing param_program_chain chip list. The existing
chain compiler remains the only producer of VM tokens, execution specs, and
fingerprints.
"""
from __future__ import annotations

import re

from param_program_chain import (
    compile_param_program_chain,
    display_param_program_chain,
    legacy_registry,
)
from program_source_core import (
    ProgramSourceError,
    diagnostic,
    find_top_level_assignment,
    parse_call,
    program_profile,
    profile_symbol,
    split_program_statements,
)


MAX_PARAM_PROGRAM_SOURCE_BYTES = 64 * 1024

_PROFILE = program_profile("param")
_OUTPUT_SYMBOLS = {
    name
    for name, spec in (_PROFILE.get("symbols") or {}).items()
    if spec.get("role") == "output" and spec.get("access") == "read_write"
}
_INPUT_SYMBOLS = {
    name
    for name, spec in (_PROFILE.get("symbols") or {}).items()
    if spec.get("role") == "input"
}
_STACK_OP_ALIASES = {
    "dup": "duplicate",
    "duplicate": "duplicate",
    "swap": "swap",
    "pop": "pop",
    "flush": "flush",
}
_BINARY_OPS = {"add", "subtract", "sub", "mul", "ratio", "div"}
_UNARY_OPS = {
    "negate",
    "conj",
    "conjugate",
    "reciprocal",
    "unit_circle",
    "square",
    "cube",
    "exp",
}
_TARGETABLE_UNARY = set(_UNARY_OPS)
_PUSH_SOURCES = {"t1", "t2"}
_EMIT_ALIASES = {"emit_p1": "p1", "emit_p2": "p2"}


class ParamProgramSourceError(ProgramSourceError):
    pass


def _canonical_expr(text):
    raw = str(text or "").strip()
    if not raw:
        raise ParamProgramSourceError("expression is empty", code="empty_expression")
    return re.sub(r"\s+", "", raw)


def _source_diag(exc, stmt):
    line = int(getattr(exc, "line", 0) or stmt.line)
    column = int(getattr(exc, "column", 0) or stmt.column)
    code = str(getattr(exc, "code", "") or "source_error")
    return diagnostic(str(exc), line=line, column=column, code=code)


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


def _lower_assignment(stmt):
    idx = find_top_level_assignment(stmt.text)
    if idx < 0:
        return None
    lhs = stmt.text[:idx].strip().lower()
    rhs = stmt.text[idx + 1:].strip()
    target = _require_writable_symbol(lhs, stmt)
    return [["const", _canonical_expr(rhs)], ["emit", target]]


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
        return [["legacy", legacy_name, args[1].strip().lower(), args[2].strip().lower()] + [
            _canonical_expr(arg) for arg in args[3:]
        ]]
    if name == "macro":
        if len(args) != 1:
            raise ParamProgramSourceError("macro(name) takes exactly one name", code="bad_arity")
        macro_name = str(args[0] or "").strip()
        if not macro_name:
            raise ParamProgramSourceError("macro name is empty", code="empty_macro")
        return [["macro", macro_name]]
    if name in legacy_registry()["by_name"]:
        return [[name] + [_canonical_expr(arg) for arg in args]]
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
        return [[name]]
    raise ParamProgramSourceError(f"unknown Param Program statement: {raw}", code="unknown_statement")


def split_param_program_statements(source_text):
    return split_program_statements(
        source_text,
        error_cls=ParamProgramSourceError,
        max_bytes=MAX_PARAM_PROGRAM_SOURCE_BYTES,
    )


def parse_param_program_source(source_text, *, strict=True):
    diagnostics = []
    chain = []
    try:
        statements = split_param_program_statements(source_text)
    except ParamProgramSourceError as exc:
        diagnostics.append(diagnostic(str(exc), line=exc.line or 1, column=exc.column or 1, code=exc.code))
        if strict:
            raise RuntimeError(f"invalid param_program_source_text: {diagnostics[0]['message']}") from exc
        return {
            "chain": [],
            "display": "",
            "statement_count": 0,
            "diagnostics": diagnostics,
        }
    for stmt in statements:
        try:
            lowered = _lower_assignment(stmt)
            if lowered is None:
                call = parse_call(stmt.text, error_cls=ParamProgramSourceError)
                lowered = _lower_call(stmt, call[0], call[1]) if call else _lower_bare(stmt)
            chain.extend(lowered)
        except ParamProgramSourceError as exc:
            diagnostics.append(_source_diag(exc, stmt))
            if strict:
                messages = "; ".join(d["message"] for d in diagnostics)
                raise RuntimeError(f"invalid param_program_source_text: {messages}") from exc
    if not chain and not diagnostics:
        diagnostics.append(diagnostic("param program source is empty", line=1, column=1, code="empty_source"))
        if strict:
            raise RuntimeError("invalid param_program_source_text: param program source is empty")
    has_errors = any(d.get("level") == "error" for d in diagnostics)
    return {
        "chain": [] if has_errors else chain,
        "display": display_param_program_chain(chain) if chain else "",
        "statement_count": len(statements),
        "diagnostics": diagnostics,
    }


def compile_param_program_source(source_text, *, macro_resolver=None, strict=True):
    parsed = parse_param_program_source(source_text, strict=strict)
    compiled = compile_param_program_chain(parsed["chain"], macro_resolver=macro_resolver, strict=strict)
    out = dict(compiled)
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
    return "\n".join(lines)
