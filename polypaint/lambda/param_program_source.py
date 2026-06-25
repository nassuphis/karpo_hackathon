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


class ParamStatementLowerer(ProfileStatementLowerer):
    def __init__(self):
        super().__init__(_PROFILE, error_cls=ParamProgramSourceError)

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
