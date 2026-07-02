"""Shared source-parser utilities for program DSLs.

The profile-specific modules own semantic lowering. This core owns the pieces
that must not drift between Param/Coeff/Solve-Score text frontends: statement
splitting, top-level call/assignment parsing, source spans, diagnostics, and
profile symbol lookup.
"""
from __future__ import annotations

import json
import os
import re
from dataclasses import dataclass


_IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
_NUMBER_RE = re.compile(r"^[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?%?$")


@dataclass(frozen=True)
class SourceStatement:
    text: str
    line: int
    column: int


class ProgramSourceError(ValueError):
    def __init__(self, message: str, *, line: int = 0, column: int = 0, code: str = "source_error"):
        super().__init__(message)
        self.line = int(line or 0)
        self.column = int(column or 0)
        self.code = str(code or "source_error")


def classify_source_error_code(message):
    """Best-effort stable code for a source error raised without one.

    Most raise sites predate coded diagnostics and default to source_error,
    which a frontend switch cannot branch on. Sites may still pass an
    explicit code=; this classifier only fills the default. Mirrors the
    chain-stage classifiers (_chain_compile_diagnostic and the solve-score
    equivalent) so parse- and compile-stage codes agree.
    """
    lowered = str(message or "").lower()
    if " q must be " in lowered or "quantile" in lowered:
        return "bad_quantile"
    if "unknown" in lowered:
        return "unknown_operator"
    if "selector" in lowered or "does not support src" in lowered or "does not support tgt" in lowered:
        return "bad_selector"
    if (
        "argument" in lowered
        or "arity" in lowered
        or "expects" in lowered
        or "at most" in lowered
        or "requires" in lowered
    ):
        return "bad_arity"
    if "stack" in lowered:
        return "stack_error"
    if (
        "finite" in lowered
        or "division" in lowered
        or "numeric" in lowered
        or "overflow" in lowered
        or "must be in" in lowered
        or "empty" in lowered
    ):
        return "bad_numeric_arg"
    return "source_error"


def diagnostic(message, *, line=1, column=1, code="source_error", level="error"):
    return {
        "level": str(level or "error"),
        "message": str(message),
        "line": int(line or 1),
        "column": int(column or 1),
        "code": str(code or "source_error"),
    }


def diagnostic_from_exception(exc, *, line=1, column=1, level="error"):
    return diagnostic(
        str(exc),
        line=line,
        column=column,
        code=getattr(exc, "code", "source_error"),
        level=level,
    )


def _profiles_path():
    return os.path.join(os.path.dirname(__file__), "program_profiles.json")


_PROFILES = None


def load_program_profiles():
    global _PROFILES
    if _PROFILES is None:
        try:
            from program_profiles import PROGRAM_PROFILES

            _PROFILES = PROGRAM_PROFILES
        except Exception:
            with open(_profiles_path(), "r", encoding="utf-8") as fh:
                _PROFILES = json.load(fh)
    return _PROFILES


def program_profile(name):
    profiles = load_program_profiles()
    profile = (profiles.get("profiles") or {}).get(str(name or ""))
    if not isinstance(profile, dict):
        raise ProgramSourceError(f"unknown program profile: {name}", code="unknown_profile")
    return profile


def profile_symbol(profile, name):
    symbols = profile.get("symbols") or {}
    symbol = symbols.get(str(name or "").strip())
    return symbol if isinstance(symbol, dict) else None


def profile_selectors(profile, name):
    # Presence semantics: a missing selector key is an empty allow-set, but a
    # present-and-malformed value must fail loudly instead of silently
    # rejecting every selector at parse time.
    selectors = profile.get("selectors") or {}
    key = str(name or "").strip()
    if key not in selectors:
        return ()
    values = selectors.get(key)
    if not isinstance(values, (list, tuple)):
        raise ValueError(f"profile selectors[{key!r}] must be a list, got {type(values).__name__}")
    return tuple(str(value) for value in values)


def profile_source(profile):
    source = profile.get("source") or {}
    return source if isinstance(source, dict) else {}


def profile_symbols_with_context(profile, context, *, access=None, role=None):
    wanted_context = str(context or "")
    out = []
    for name, spec in (profile.get("symbols") or {}).items():
        if wanted_context and wanted_context not in (spec.get("contexts") or []):
            continue
        if access is not None and spec.get("access") != access:
            continue
        if role is not None and spec.get("role") != role:
            continue
        out.append(str(name))
    return tuple(out)


def symbol_access(profile, name):
    symbol = profile_symbol(profile, name)
    return str((symbol or {}).get("access") or "")


def is_identifier(text):
    return bool(_IDENT_RE.match(str(text or "").strip()))


def split_top_level(text, sep=",", *, error_cls=ProgramSourceError):
    out = []
    start = 0
    paren = bracket = 0
    for idx, ch in enumerate(str(text or "")):
        if ch == "(":
            paren += 1
        elif ch == ")":
            paren -= 1
            if paren < 0:
                raise error_cls("unexpected closing parenthesis", column=idx + 1, code="unexpected_close")
        elif ch == "[":
            bracket += 1
        elif ch == "]":
            bracket -= 1
            if bracket < 0:
                raise error_cls("unexpected closing bracket", column=idx + 1, code="unexpected_close")
        elif ch == sep and paren == 0 and bracket == 0:
            out.append(str(text)[start:idx].strip())
            start = idx + 1
    if paren != 0:
        raise error_cls("unclosed parenthesis", code="unclosed_parenthesis")
    if bracket != 0:
        raise error_cls("unclosed bracket", code="unclosed_bracket")
    out.append(str(text or "")[start:].strip())
    return out


def is_numeric_literal(text, *, allow_percent=False):
    raw = str(text or "").strip()
    if not allow_percent and raw.endswith("%"):
        return False
    return bool(_NUMBER_RE.fullmatch(raw))


# ── Canonical number policies (FROZEN wire formats) ─────────────────────
#
# Every program kind hashes formatted numbers into its fingerprint, which is
# a persisted render-artifact cache key. There are exactly TWO policies; each
# kind is frozen to one and changing either orphans caches for its kinds:
#
#   canonical_number_g17   .17g + signed-zero fold        param v1, coeff v1
#   canonical_number_repr  repr + int-collapse + 0-fold   root, solve-score
#
# (format_numeric_literal is the historical name of the repr policy; kept as
# the primary symbol because solve-score source parsing imports it.)


def canonical_number_g17(value):
    number = float(value)
    if number == 0:
        number = 0.0
    return format(number, ".17g")


def format_numeric_literal(value):
    num = float(value)
    if num == 0.0:
        num = 0.0
    if num.is_integer():
        return str(int(num))
    return repr(num)


canonical_number_repr = format_numeric_literal


def format_percent_literal(value):
    raw = str(value or "").strip()
    if raw.endswith("%"):
        raw = raw[:-1].strip()
    return format_numeric_literal(raw)


def find_top_level_assignment(text):
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


def parse_keyword_args(args, *, allowed, error_cls=ProgramSourceError):
    positional = []
    kwargs = {}
    allowed_keys = {str(key).strip().lower() for key in (allowed or [])}
    for raw in args:
        idx = find_top_level_assignment(raw)
        if idx < 0:
            positional.append(str(raw).strip())
            continue
        key = str(raw[:idx]).strip().lower()
        value = str(raw[idx + 1:]).strip()
        if key not in allowed_keys:
            raise error_cls(f"unknown keyword argument {key!r}")
        if key in kwargs:
            raise error_cls(f"duplicate keyword argument {key!r}")
        kwargs[key] = value
    return positional, kwargs


def parse_call(text, *, error_cls=ProgramSourceError):
    raw = str(text or "").strip()
    match = re.match(r"^([A-Za-z_][A-Za-z0-9_]*)\s*\(", raw)
    if not match:
        return None
    if not raw.endswith(")"):
        raise error_cls("function call is missing closing parenthesis", code="unclosed_parenthesis")
    name = match.group(1).lower()
    inner = raw[match.end():-1]
    args = [] if not inner.strip() else split_top_level(inner, error_cls=error_cls)
    return name, args


def split_program_statements(source_text, *, error_cls=ProgramSourceError, max_bytes=None):
    raw = str(source_text or "")
    if max_bytes is not None and len(raw.encode("utf-8")) > int(max_bytes):
        raise error_cls(
            f"source text exceeds {int(max_bytes)} UTF-8 bytes",
            line=1,
            column=1,
            code="source_too_large",
        )
    statements = []
    buf = []
    start_line = 1
    start_col = 1
    line = 1
    col = 1
    paren = bracket = 0
    in_comment = False

    def flush(end_line, end_col):
        nonlocal buf, start_line, start_col
        text = "".join(buf).strip()
        if text:
            leading = len("".join(buf)) - len("".join(buf).lstrip())
            statements.append(SourceStatement(text=text, line=start_line, column=start_col + leading))
        buf = []
        start_line = end_line
        start_col = end_col

    for ch in raw:
        if in_comment:
            if ch == "\n":
                in_comment = False
                if paren == 0 and bracket == 0:
                    flush(line, col)
                line += 1
                col = 1
                if not buf:
                    start_line = line
                    start_col = col
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
                raise error_cls("unexpected closing parenthesis", line=line, column=col, code="unexpected_close")
        elif ch == "[":
            bracket += 1
        elif ch == "]":
            bracket -= 1
            if bracket < 0:
                raise error_cls("unexpected closing bracket", line=line, column=col, code="unexpected_close")

        if ch == ";" and paren == 0 and bracket == 0:
            flush(line, col + 1)
            col += 1
            start_line = line
            start_col = col
            continue
        if ch == "\n":
            if paren == 0 and bracket == 0:
                flush(line + 1, 1)
            else:
                buf.append(ch)
            line += 1
            col = 1
            if not buf:
                start_line = line
                start_col = col
            continue
        if not buf:
            start_line = line
            start_col = col
        buf.append(ch)
        col += 1

    if paren != 0:
        raise error_cls("unclosed parenthesis", line=line, column=col, code="unclosed_parenthesis")
    if bracket != 0:
        raise error_cls("unclosed bracket", line=line, column=col, code="unclosed_bracket")
    flush(line, col)
    return statements


class ProfileStatementLowerer:
    """Shared statement dispatcher for profile-backed source languages.

    Profile modules still own semantic lowering. This class centralizes the
    grammar split that must not drift between Param/Coeff: assignment vs call
    vs bare statement, RHS emptiness checks, indexed-lvalue detection, and
    profile-symbol writability checks. Subclasses implement the profile hooks.
    """

    def __init__(self, profile, *, error_cls=ProgramSourceError):
        self.profile = profile
        self.error_cls = error_cls
        self._writable_lhs = set(
            profile_symbols_with_context(profile, "lhs", access="read_write")
        )
        if self._writable_lhs:
            names = "|".join(
                re.escape(name)
                for name in sorted(self._writable_lhs, key=len, reverse=True)
            )
            self._indexed_lhs_re = re.compile(r"^(" + names + r")\[(.*)\]$", re.IGNORECASE)
        else:
            self._indexed_lhs_re = None

    @property
    def writable_lhs(self):
        return frozenset(self._writable_lhs)

    def source_error(self, message, *, line=0, column=0, code="source_error"):
        return self.error_cls(message, line=line, column=column, code=code)

    def lower_statement(self, statement):
        text = statement.text.strip()
        assignment = find_top_level_assignment(text)
        if assignment >= 0:
            lhs = re.sub(r"\s+", "", text[:assignment])
            rhs = text[assignment + 1:].strip()
            if not rhs:
                raise self.source_error(
                    "assignment right-hand side is empty",
                    line=statement.line,
                    column=assignment + 2,
                )
            return self.lower_assignment(statement, lhs, rhs)

        call = parse_call(text, error_cls=self.error_cls)
        if call:
            return self.lower_call_statement(statement, call[0], call[1])
        return self.lower_bare_statement(statement, text.strip())

    def lower_assignment(self, statement, lhs, rhs):
        lowered_lhs = str(lhs or "").lower()
        indexed_lhs = self._indexed_lhs_re.match(lhs) if self._indexed_lhs_re else None
        if indexed_lhs:
            lhs_name = indexed_lhs.group(1).lower()
            index_expr = indexed_lhs.group(2).strip()
            if lhs_name not in self._writable_lhs:
                raise self.source_error(
                    f"{lhs_name}[...] is read-only in source",
                    line=statement.line,
                    column=statement.column,
                )
            if not index_expr:
                raise self.source_error(
                    f"{lhs_name}[...] index expression is empty",
                    line=statement.line,
                    column=statement.column,
                )
            return self.lower_indexed_assignment(statement, lhs_name, index_expr, rhs)

        if lowered_lhs not in self._writable_lhs:
            allowed = ", ".join(sorted(self._writable_lhs)) or "<none>"
            raise self.source_error(
                f"only {allowed} assignments are supported in this source profile",
                line=statement.line,
                column=statement.column,
            )
        return self.lower_symbol_assignment(statement, lowered_lhs, rhs)

    def lower_indexed_assignment(self, statement, lhs_name, index_expr, rhs):
        raise NotImplementedError

    def lower_symbol_assignment(self, statement, lhs, rhs):
        raise NotImplementedError

    def lower_call_statement(self, statement, name, args):
        raise NotImplementedError

    def lower_bare_statement(self, statement, text):
        raise NotImplementedError


def parse_profile_source(
    source_text,
    *,
    lowerer,
    display_fn,
    error_cls=ProgramSourceError,
    compile_error_cls=None,
    max_bytes=None,
    strict=True,
):
    """Split, lower, and diagnose a profile-backed source program.

    The active profile supplies only `lowerer.lower_statement`. This shared
    routine owns the orchestration contract for source parsers: one splitter,
    consistent source spans, strict/non-strict behavior, diagnostic shape, and
    display rendering.
    """

    diagnostics = []
    chain = []
    try:
        statements = split_program_statements(
            source_text,
            error_cls=error_cls,
            max_bytes=max_bytes,
        )
    except error_cls as exc:
        diagnostics.append(
            diagnostic_from_exception(
                exc,
                line=getattr(exc, "line", 0) or 1,
                column=getattr(exc, "column", 0) or 1,
            )
        )
        if strict:
            if compile_error_cls is not None:
                raise compile_error_cls(diagnostics) from exc
            messages = "; ".join(d["message"] for d in diagnostics)
            raise RuntimeError(messages or "invalid program source") from exc
        return {"chain": [], "display": "", "statement_count": 0, "diagnostics": diagnostics}

    for stmt in statements:
        try:
            chain.extend(lowerer.lower_statement(stmt))
        except error_cls as exc:
            diagnostics.append(
                diagnostic_from_exception(
                    exc,
                    line=getattr(exc, "line", 0) or stmt.line,
                    column=getattr(exc, "column", 0) or stmt.column,
                )
            )
        except Exception as exc:
            diagnostics.append(diagnostic_from_exception(exc, line=stmt.line, column=stmt.column))
    if diagnostics and strict:
        if compile_error_cls is not None:
            raise compile_error_cls(diagnostics)
        messages = "; ".join(d["message"] for d in diagnostics)
        raise RuntimeError(messages or "invalid program source")
    return {
        "chain": chain,
        "display": display_fn(chain) if chain else "",
        "statement_count": len(statements),
        "diagnostics": diagnostics,
    }
