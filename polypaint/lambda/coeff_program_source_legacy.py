"""Frozen Coeff source parser-shell oracle for CR18 equivalence tests.

This module is intentionally test-only. It preserves the pre-shared-core
statement splitter, top-level assignment finder, and call parser that existed
before `coeff_program_source.py` delegated those responsibilities to
`program_source_core.py`.

The semantic lowerers are imported from the production module so the oracle
isolates the parser-shell migration: old shell -> same lowerers -> same chain.
Do not import this module from production handlers.
"""
from __future__ import annotations

import re
from contextlib import contextmanager
from dataclasses import dataclass

import coeff_program_source as _current
from coeff_program_chain import compile_coeff_program_chain, display_coeff_program_chain


MAX_COEFF_PROGRAM_SOURCE_BYTES = _current.MAX_COEFF_PROGRAM_SOURCE_BYTES


@dataclass(frozen=True)
class _Statement:
    text: str
    line: int
    column: int


class CoeffProgramSourceError(RuntimeError):
    def __init__(self, message: str, *, line: int = 0, column: int = 0, code: str = "source_error"):
        super().__init__(message)
        self.line = int(line or 0)
        self.column = int(column or 0)
        self.code = str(code or "source_error")


class CoeffProgramSourceCompileError(RuntimeError):
    def __init__(self, diagnostics):
        self.diagnostics = list(diagnostics or [])
        message = "; ".join(d["message"] for d in self.diagnostics if d.get("level") == "error")
        super().__init__(message or "invalid coeff program source")


def _diagnostic(message, *, line=1, column=1, code=None):
    return {
        "level": "error",
        "message": str(message),
        "line": int(line),
        "column": int(column),
        "code": str(code or getattr(message, "code", "source_error")),
    }


def _split_top_level(text, sep=","):
    out = []
    start = 0
    paren = bracket = 0
    for idx, ch in enumerate(str(text or "")):
        if ch == "(":
            paren += 1
        elif ch == ")":
            paren -= 1
            if paren < 0:
                raise CoeffProgramSourceError("unexpected closing parenthesis", column=idx + 1, code="unexpected_close")
        elif ch == "[":
            bracket += 1
        elif ch == "]":
            bracket -= 1
            if bracket < 0:
                raise CoeffProgramSourceError("unexpected closing bracket", column=idx + 1, code="unexpected_close")
        elif ch == sep and paren == 0 and bracket == 0:
            out.append(str(text)[start:idx].strip())
            start = idx + 1
    if paren != 0:
        raise CoeffProgramSourceError("unclosed parenthesis", code="unclosed_parenthesis")
    if bracket != 0:
        raise CoeffProgramSourceError("unclosed bracket", code="unclosed_bracket")
    out.append(str(text or "")[start:].strip())
    return out


def _find_top_level_assignment(text):
    paren = bracket = 0
    for idx, ch in enumerate(str(text or "")):
        if ch == "(":
            paren += 1
        elif ch == ")":
            paren -= 1
        elif ch == "[":
            bracket += 1
        elif ch == "]":
            bracket -= 1
        elif ch == "=" and paren == 0 and bracket == 0:
            return idx
    return -1


def _parse_call(text):
    raw = str(text or "").strip()
    match = re.match(r"^([A-Za-z_][A-Za-z0-9_]*)\s*\(", raw)
    if not match or not raw.endswith(")"):
        return None
    name = match.group(1).lower()
    inner = raw[match.end():-1]
    args = [] if not inner.strip() else _split_top_level(inner)
    return name, args


def split_coeff_program_statements(source_text):
    text = str(source_text or "")
    source_bytes = len(text.encode("utf-8"))
    if source_bytes > MAX_COEFF_PROGRAM_SOURCE_BYTES:
        raise CoeffProgramSourceError(
            f"coeff program source is {source_bytes} bytes; max is {MAX_COEFF_PROGRAM_SOURCE_BYTES}",
            code="source_too_large",
        )
    statements = []
    buffer = []
    start_line = 1
    start_col = 1
    line = 1
    col = 1
    paren = bracket = 0
    in_comment = False
    for ch in text:
        if in_comment:
            if ch == "\n":
                in_comment = False
                if paren == 0 and bracket == 0:
                    stmt = "".join(buffer).strip()
                    if stmt:
                        statements.append(_Statement(stmt, start_line, start_col))
                    buffer = []
                    start_line = line + 1
                    start_col = 1
                else:
                    buffer.append(ch)
                line += 1
                col = 1
            else:
                col += 1
            continue
        if ch == "#":
            in_comment = True
            col += 1
            continue
        if ch == "(":
            paren += 1
        elif ch == ")":
            paren -= 1
            if paren < 0:
                raise CoeffProgramSourceError("unexpected closing parenthesis", line=line, column=col, code="unexpected_close")
        elif ch == "[":
            bracket += 1
        elif ch == "]":
            bracket -= 1
            if bracket < 0:
                raise CoeffProgramSourceError("unexpected closing bracket", line=line, column=col, code="unexpected_close")
        if (ch == ";" or ch == "\n") and paren == 0 and bracket == 0:
            stmt = "".join(buffer).strip()
            if stmt:
                statements.append(_Statement(stmt, start_line, start_col))
            buffer = []
            if ch == "\n":
                line += 1
                col = 1
                start_line = line
                start_col = col
            else:
                col += 1
                start_line = line
                start_col = col
            continue
        buffer.append(ch)
        if ch == "\n":
            line += 1
            col = 1
        else:
            col += 1
    if paren != 0:
        raise CoeffProgramSourceError("unclosed parenthesis", line=line, column=col, code="unclosed_parenthesis")
    if bracket != 0:
        raise CoeffProgramSourceError("unclosed bracket", line=line, column=col, code="unclosed_bracket")
    stmt = "".join(buffer).strip()
    if stmt:
        statements.append(_Statement(stmt, start_line, start_col))
    return statements


@contextmanager
def _legacy_shell_installed():
    names = {
        "_split_top_level": _split_top_level,
        "_find_top_level_assignment": _find_top_level_assignment,
        "_parse_call": _parse_call,
        "CoeffProgramSourceError": CoeffProgramSourceError,
    }
    saved = {name: getattr(_current, name) for name in names}
    try:
        for name, value in names.items():
            setattr(_current, name, value)
        yield
    finally:
        for name, value in saved.items():
            setattr(_current, name, value)


def parse_coeff_program_source(source_text, *, strict=True):
    diagnostics = []
    chain = []
    try:
        statements = split_coeff_program_statements(source_text)
    except CoeffProgramSourceError as exc:
        diagnostics.append(_diagnostic(exc, line=exc.line or 1, column=exc.column or 1))
        if strict:
            raise CoeffProgramSourceCompileError(diagnostics) from exc
        return {"chain": [], "display": "", "statement_count": 0, "diagnostics": diagnostics}
    with _legacy_shell_installed():
        for stmt in statements:
            try:
                chain.extend(_current._legacy_lower_statement(stmt))
            except Exception as exc:
                diagnostics.append(_diagnostic(exc, line=getattr(exc, "line", 0) or stmt.line, column=getattr(exc, "column", 0) or stmt.column))
    if diagnostics and strict:
        raise CoeffProgramSourceCompileError(diagnostics)
    return {
        "chain": chain,
        "display": display_coeff_program_chain(chain),
        "statement_count": len(statements),
        "diagnostics": diagnostics,
    }


def compile_coeff_program_source(source_text, *, macro_resolver=None, strict=True):
    parsed = parse_coeff_program_source(source_text, strict=strict)
    compiled = compile_coeff_program_chain(parsed["chain"], macro_resolver=macro_resolver, strict=strict)
    compiled["source_text"] = str(source_text or "")
    compiled["source_display"] = parsed["display"]
    compiled["source_statement_count"] = parsed["statement_count"]
    compiled["diagnostics"] = list(parsed.get("diagnostics") or []) + list(compiled.get("diagnostics") or [])
    return compiled
