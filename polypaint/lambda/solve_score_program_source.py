"""Solve-Score Program source compiler.

Phase 6B source layer for solve-score chains.  The native/runtime wire remains
the existing solve_score_chain JSON plus program_spec; this module adds a
source-first facade and a total chain-to-source serializer for valid chains.
"""
from __future__ import annotations

import json
import re
from dataclasses import dataclass

from program_source_core import (
    ProgramSourceError,
    diagnostic_from_exception,
    find_top_level_assignment,
    format_numeric_literal,
    format_percent_literal,
    is_numeric_literal,
    parse_call,
    parse_keyword_args,
    split_program_statements,
)
from solve_score_chain import (
    COMBINE_CHIPS,
    GENERIC_METRIC_PUBLIC_NAME,
    OUTPUT_CHIPS,
    SOLVE_SCORE_SPEC_VERSION,
    STACK_CHIPS,
    TRANSFER_CHIP_NAME,
    UNARY_CHIPS,
    VALID_SOLVE_SCORE_METRICS,
    VALID_SOLVE_SCORE_SOURCES,
    compile_solve_score_chain,
    compile_solve_score_chain_or_legacy,
    compiled_solve_score_fingerprint,
    public_solve_score_chain,
    render_solve_score_program_spec,
    serialize_solve_score_chain,
)


MAX_SOLVE_SCORE_SOURCE_BYTES = 8192
_IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
_RESERVED = {
    "score",
    "emit",
    "emit_norm",
    "emit_none",
    "metric",
    "const",
    "push",
    "dup",
    "flush",
    "pop",
    *VALID_SOLVE_SCORE_METRICS,
    *UNARY_CHIPS,
    *COMBINE_CHIPS,
}


class SolveScoreProgramSourceError(ProgramSourceError):
    """Solve-score source syntax/lowering error with line/column metadata."""


class SolveScoreProgramSourceCompileError(ValueError):
    """Strict source compile failure carrying structured diagnostics."""

    def __init__(self, diagnostics):
        self.diagnostics = list(diagnostics or [])
        message = "; ".join(d.get("message", "compile error") for d in self.diagnostics)
        super().__init__(message or "invalid solve-score program source")


@dataclass(frozen=True)
class _Expr:
    source: str
    chain: tuple
    materialized: bool = False


def _fmt_number(value):
    return format_numeric_literal(value)


def _fmt_q_percent(value):
    return format_percent_literal(value)


def _is_number(text):
    return is_numeric_literal(text, allow_percent=True)


def _has_top_level_infix(text):
    """Reject only source-level infix, not signed numeric params inside calls."""
    raw = str(text or "")
    paren = bracket = 0
    prev = ""
    for idx, ch in enumerate(raw):
        if ch == "(":
            paren += 1
        elif ch == ")":
            paren -= 1
        elif ch == "[":
            bracket += 1
        elif ch == "]":
            bracket -= 1
        if paren != 0 or bracket != 0:
            if not ch.isspace():
                prev = ch
            continue
        if ch in ("*", "/"):
            return True
        if ch == "+":
            if idx == 0:
                continue
            if prev in ("e", "E"):
                continue
            return True
        if ch == "-":
            if idx == 0:
                continue
            if prev in ("e", "E"):
                continue
            return True
        if not ch.isspace():
            prev = ch
    return False


def _chain_rows(chain):
    return tuple(tuple(row) if isinstance(row, list) else (row,) for row in chain)


def _public_chain(compiled):
    return public_solve_score_chain(compiled["chain"])


def _canonical_program(compiled, *, source_text="", statement_count=0, diagnostics=None):
    public_chain = _public_chain(compiled)
    return {
        "program_kind": "solve_score_program",
        "spec_version": SOLVE_SCORE_SPEC_VERSION,
        "source_text": str(source_text or ""),
        "source_display": str(source_text or "").strip(),
        "chain": public_chain,
        "chain_json": json.dumps(public_chain, separators=(",", ":")),
        "display": compiled["display"],
        "program_spec": compiled["program_spec"],
        "rendered_program_spec": render_solve_score_program_spec(
            compiled["program_spec"],
            version=SOLVE_SCORE_SPEC_VERSION,
        ),
        "fingerprint": compiled_solve_score_fingerprint(compiled),
        "metric": compiled["metric"],
        "metrics": list(compiled.get("metrics") or []),
        "metric_count": int(compiled.get("metric_count") or 0),
        "quantile": compiled["quantile"],
        "omega": compiled["omega"],
        "omega_phase": compiled.get("omega_phase", 0.0),
        "omega_enabled": compiled["omega_enabled"],
        "program_tokens": list(compiled.get("program_tokens") or []),
        "has_explicit_outputs": bool(compiled.get("has_explicit_outputs")),
        "output_channel_count": int(compiled.get("output_channel_count") or 1),
        "output_channels": list(compiled.get("output_channels") or []),
        "statement_count": int(statement_count or len(public_chain)),
        "diagnostics": list(diagnostics or []),
    }


def _parse_metric_expr(args, locals_map):
    positional, kwargs = parse_keyword_args(args, allowed={"q", "lag"}, error_cls=SolveScoreProgramSourceError)
    if len(positional) < 2 or len(positional) > 3:
        raise SolveScoreProgramSourceError("metric(...) requires metric name, source, and q=...%")
    metric = positional[0].strip().lower()
    source = positional[1].strip().lower()
    if metric not in VALID_SOLVE_SCORE_METRICS:
        raise SolveScoreProgramSourceError(f"unknown solve-score metric {metric!r}")
    if source not in VALID_SOLVE_SCORE_SOURCES:
        raise SolveScoreProgramSourceError("metric source must be one of slv, cf, pm")
    q_raw = kwargs.get("q")
    if len(positional) == 3:
        if q_raw is not None:
            raise SolveScoreProgramSourceError("metric q supplied twice")
        q_raw = positional[2]
    if q_raw in ("", None):
        raise SolveScoreProgramSourceError("metric(...) requires q=...%")
    lag_raw = kwargs.get("lag", "0")
    try:
        lag = int(str(lag_raw).strip())
    except ValueError as exc:
        raise SolveScoreProgramSourceError("metric lag must be 0 or 1") from exc
    if lag not in (0, 1):
        raise SolveScoreProgramSourceError("metric lag must be 0 or 1")
    source_lag = source if lag == 0 else f"{source}-1"
    q = _fmt_q_percent(q_raw)
    chain = [[metric, source_lag, q]]
    return _Expr(f"metric({metric}, {source}, q={q}%" + (", lag=1)" if lag else ")"), _chain_rows(chain))


def _parse_expr(text, locals_map):
    raw = str(text or "").strip()
    if not raw:
        raise SolveScoreProgramSourceError("empty expression")
    if _has_top_level_infix(raw):
        if not _is_number(raw):
            raise SolveScoreProgramSourceError("solve-score source uses call-tree expressions; infix arithmetic is not supported")
    lower = raw.lower()
    if lower in locals_map:
        return locals_map[lower]
    if lower in ("pop", "pop()"):
        return _Expr("pop()", tuple(), materialized=True)
    if _is_number(raw):
        value = raw[:-1] if raw.endswith("%") else raw
        return _Expr(_fmt_number(value), _chain_rows([["const", _fmt_number(value)]]))
    call = parse_call(raw, error_cls=SolveScoreProgramSourceError)
    if not call:
        raise SolveScoreProgramSourceError(f"unknown solve-score expression {raw!r}")
    name, args = call
    if name == "metric":
        return _parse_metric_expr(args, locals_map)
    if name == "const":
        if len(args) != 1 or not _is_number(args[0]):
            raise SolveScoreProgramSourceError("const(value) requires one finite numeric literal")
        value = _fmt_number(args[0])
        return _Expr(f"const({value})", _chain_rows([["const", value]]))
    if name in UNARY_CHIPS:
        spec = UNARY_CHIPS[name]
        if name == TRANSFER_CHIP_NAME:
            if len(args) not in (2, 3):
                raise SolveScoreProgramSourceError(f"{name}(...) requires score, omega, and optional phase")
        elif len(args) != 1 + int(spec["params"]):
            raise SolveScoreProgramSourceError(f"{name}(...) requires {1 + int(spec['params'])} argument(s)")
        expr = _parse_expr(args[0], locals_map)
        params = [_fmt_number(arg) for arg in args[1:]]
        row = [name, *params] if params else name
        source = f"{name}({', '.join([expr.source, *params])})"
        return _Expr(source, expr.chain + _chain_rows([row]))
    if name in COMBINE_CHIPS:
        spec = COMBINE_CHIPS[name]
        if len(args) != int(spec["arity"]) + int(spec["params"]):
            raise SolveScoreProgramSourceError(f"{name}(...) requires {int(spec['arity']) + int(spec['params'])} argument(s)")
        exprs = [_parse_expr(arg, locals_map) for arg in args[: int(spec["arity"])]]
        params = [_fmt_number(arg) for arg in args[int(spec["arity"]):]]
        row = [name, *params] if params else name
        source = f"{name}({', '.join([*(expr.source for expr in exprs), *params])})"
        chain = tuple()
        for expr in exprs:
            chain += expr.chain
        return _Expr(source, chain + _chain_rows([row]))
    raise SolveScoreProgramSourceError(f"unknown solve-score function {name!r}")


def _validate_local_name(name):
    lowered = str(name or "").strip().lower()
    if not _IDENT_RE.fullmatch(lowered):
        raise SolveScoreProgramSourceError(f"invalid local name {name!r}")
    if lowered in _RESERVED:
        raise SolveScoreProgramSourceError(f"{lowered!r} is reserved")
    return lowered


def parse_solve_score_program_source(source_text, strict=True):
    diagnostics = []
    try:
        statements = split_program_statements(
            source_text,
            error_cls=SolveScoreProgramSourceError,
            max_bytes=MAX_SOLVE_SCORE_SOURCE_BYTES,
        )
    except SolveScoreProgramSourceError as exc:
        diagnostics.append(diagnostic_from_exception(exc))
        if strict:
            raise SolveScoreProgramSourceCompileError(diagnostics) from exc
        statements = []

    locals_map = {}
    chain = []
    explicit_output = False
    implicit_score = False
    stack_depth = 0

    def append_expr(expr):
        nonlocal stack_depth
        chain.extend(json.loads(json.dumps([list(row) if len(row) > 1 else row[0] for row in expr.chain])))
        stack_depth += 1

    for stmt in statements:
        try:
            text = stmt.text.strip()
            idx = find_top_level_assignment(text)
            if idx >= 0:
                lhs = text[:idx].strip().lower()
                rhs = text[idx + 1:].strip()
                if lhs == "score":
                    if explicit_output:
                        raise SolveScoreProgramSourceError("score assignment cannot be mixed with explicit emit statements")
                    expr = _parse_expr(rhs, locals_map)
                    chain.extend(json.loads(json.dumps([list(row) if len(row) > 1 else row[0] for row in expr.chain])))
                    implicit_score = True
                    continue
                local_name = _validate_local_name(lhs)
                if local_name in locals_map:
                    raise SolveScoreProgramSourceError(f"local {local_name!r} is already assigned")
                locals_map[local_name] = _parse_expr(rhs, locals_map)
                continue

            call = parse_call(text, error_cls=SolveScoreProgramSourceError)
            if not call:
                raise SolveScoreProgramSourceError("bare expressions are not valid solve-score statements")
            name, args = call
            if name == "push":
                if len(args) != 1:
                    raise SolveScoreProgramSourceError("push(expr) requires one expression")
                append_expr(_parse_expr(args[0], locals_map))
                continue
            if name in ("dup", "flush"):
                if args:
                    raise SolveScoreProgramSourceError(f"{name}() takes no arguments")
                if name == "dup" and stack_depth < 1:
                    raise SolveScoreProgramSourceError("dup() requires stack depth at least 1")
                chain.append(name)
                stack_depth = stack_depth + 1 if name == "dup" else 0
                continue
            if name in OUTPUT_CHIPS:
                if len(args) > 1:
                    raise SolveScoreProgramSourceError(f"{name}(...) takes zero or one expression")
                if implicit_score:
                    raise SolveScoreProgramSourceError("explicit emit statements cannot be mixed with score assignment")
                if args:
                    append_expr(_parse_expr(args[0], locals_map))
                elif stack_depth < 1:
                    raise SolveScoreProgramSourceError(f"{name}() requires stack depth at least 1")
                chain.append({"emit": ["emit", "raw"], "emit_norm": ["emit", "norm"], "emit_none": ["emit", "none"]}[name])
                stack_depth -= 1
                explicit_output = True
                continue
            if name in UNARY_CHIPS or name in COMBINE_CHIPS:
                # Compatibility stack-op statement, used by chain-to-source round trips.
                spec = UNARY_CHIPS.get(name) or COMBINE_CHIPS.get(name) or {}
                needed = int(spec.get("arity") or 0)
                needed_params = int(spec.get("params") or 0)
                if args and not (len(args) == needed_params and stack_depth >= needed):
                    expr = _parse_expr(text, locals_map)
                    append_expr(expr)
                    continue
                if stack_depth < needed:
                    raise SolveScoreProgramSourceError(f"{name}() requires stack depth at least {needed}")
                if len(args) != needed_params:
                    raise SolveScoreProgramSourceError(f"{name}() requires exactly {needed_params} parameter(s)")
                params = [_fmt_number(arg) for arg in args]
                chain.append([name, *params] if params else name)
                stack_depth -= max(0, needed - 1)
                continue
            raise SolveScoreProgramSourceError(f"unknown solve-score statement {name!r}")
        except SolveScoreProgramSourceError as exc:
            diagnostics.append(diagnostic_from_exception(exc, line=stmt.line, column=stmt.column))

    if diagnostics and strict:
        raise SolveScoreProgramSourceCompileError(diagnostics)
    return {
        "source_text": str(source_text or ""),
        "chain": chain,
        "statement_count": len(statements),
        "diagnostics": diagnostics,
    }


def compile_solve_score_program_source(source_text, strict=True):
    parsed = parse_solve_score_program_source(source_text, strict=strict)
    if parsed["diagnostics"]:
        return _canonical_program(
            compile_solve_score_chain([["proximity", "0.1"]]),
            source_text=source_text,
            statement_count=parsed["statement_count"],
            diagnostics=parsed["diagnostics"],
        )
    try:
        compiled = compile_solve_score_chain(parsed["chain"])
    except Exception as exc:
        diagnostics = [diagnostic_from_exception(exc)]
        if strict:
            raise SolveScoreProgramSourceCompileError(diagnostics) from exc
        return _canonical_program(
            compile_solve_score_chain([["proximity", "0.1"]]),
            source_text=source_text,
            statement_count=parsed["statement_count"],
            diagnostics=diagnostics,
        )
    return _canonical_program(compiled, source_text=source_text, statement_count=parsed["statement_count"])


def _row_name_params(row):
    if isinstance(row, str):
        return row, []
    if isinstance(row, dict):
        return str(row.get("name") or ""), [str(v) for v in (row.get("params") or [])]
    if isinstance(row, (list, tuple)) and row:
        return str(row[0]), [str(v) for v in row[1:]]
    raise SolveScoreProgramSourceError(f"invalid solve-score chain row: {row!r}")


def _metric_source(metric, params):
    if metric == GENERIC_METRIC_PUBLIC_NAME:
        if len(params) != 3:
            raise SolveScoreProgramSourceError("metric row requires metric, source, q")
        metric_name, source_lag, q = params
    else:
        metric_name = metric
        if len(params) == 1:
            source_lag, q = "slv", params[0]
        elif len(params) == 2:
            source_lag, q = params
        else:
            raise SolveScoreProgramSourceError(f"metric row {metric!r} requires q or source,q")
    source = str(source_lag).lower()
    lag = 0
    if source.endswith("-1"):
        source = source[:-2]
        lag = 1
    q = _fmt_q_percent(q)
    return f"metric({metric_name}, {source}, q={q}%" + (", lag=1)" if lag else ")")


def _materialize(stack, lines):
    if not stack:
        raise SolveScoreProgramSourceError("cannot materialize empty stack")
    top = stack[-1]
    if not top.materialized:
        lines.append(f"push({top.source})")
        stack[-1] = _Expr(top.source, top.chain, materialized=True)


def _materialize_all(stack, lines):
    for idx, expr in enumerate(stack):
        if not expr.materialized:
            lines.append(f"push({expr.source})")
            stack[idx] = _Expr(expr.source, expr.chain, materialized=True)


def _stack_call_line(name, params):
    return f"{name}({', '.join(params)})" if params else f"{name}()"


def solve_score_source_text_from_chain(chain):
    compiled = compile_solve_score_chain_or_legacy(chain, "", default_metric="proximity")
    public_chain = _public_chain(compiled)
    stack = []
    lines = []
    explicit_emits = False

    for row in public_chain:
        name, params = _row_name_params(row)
        name = name.strip()
        if name == GENERIC_METRIC_PUBLIC_NAME or name in VALID_SOLVE_SCORE_METRICS:
            src = _metric_source(name, params)
            stack.append(_Expr(src, _chain_rows([[name, *params] if params else name])))
            continue
        if name == "const":
            if len(params) != 1:
                raise SolveScoreProgramSourceError("const row requires one parameter")
            src = f"const({_fmt_number(params[0])})"
            stack.append(_Expr(src, _chain_rows([["const", _fmt_number(params[0])]])))
            continue
        if name == "dup":
            _materialize_all(stack, lines)
            if not stack:
                raise SolveScoreProgramSourceError("dup requires stack depth at least 1")
            lines.append("dup()")
            stack.append(stack[-1])
            continue
        if name == "flush":
            _materialize_all(stack, lines)
            lines.append("flush()")
            stack = []
            continue
        if name in OUTPUT_CHIPS or (name == "emit" and params):
            mode = "raw"
            if name == "emit_norm" or params == ["norm"]:
                mode = "norm"
            elif name == "emit_none" or params == ["none"]:
                mode = "none"
            fn = {"raw": "emit", "norm": "emit_norm", "none": "emit_none"}[mode]
            if not stack:
                raise SolveScoreProgramSourceError(f"{fn} requires stack depth at least 1")
            top = stack.pop()
            if top.materialized:
                lines.append(f"{fn}()")
            else:
                lines.append(f"{fn}({top.source})")
            explicit_emits = True
            continue
        if name in UNARY_CHIPS:
            if len(stack) < 1:
                raise SolveScoreProgramSourceError(f"{name} requires one input")
            spec = UNARY_CHIPS[name]
            needed_params = int(spec["params"])
            if name == TRANSFER_CHIP_NAME and len(params) in (1, 2):
                pass
            elif len(params) != needed_params:
                raise SolveScoreProgramSourceError(f"{name} requires {needed_params} parameter(s)")
            top = stack.pop()
            if top.materialized:
                clean_params = [_fmt_number(param) for param in params]
                lines.append(_stack_call_line(name, clean_params))
                stack.append(_Expr("pop()", tuple(), materialized=True))
            else:
                stack.append(_Expr(f"{name}({', '.join([top.source, *params])})", top.chain + _chain_rows([[name, *params] if params else name])))
            continue
        if name in COMBINE_CHIPS:
            spec = COMBINE_CHIPS[name]
            arity = int(spec["arity"])
            needed_params = int(spec["params"])
            if len(stack) < arity:
                raise SolveScoreProgramSourceError(f"{name} requires {arity} inputs")
            if len(params) != needed_params:
                raise SolveScoreProgramSourceError(f"{name} requires {needed_params} parameter(s)")
            args = stack[-arity:]
            del stack[-arity:]
            if any(arg.materialized for arg in args):
                stack.extend(args)
                _materialize_all(stack, lines)
                del stack[-arity:]
                clean_params = [_fmt_number(param) for param in params]
                lines.append(_stack_call_line(name, clean_params))
                stack.append(_Expr("pop()", tuple(), materialized=True))
            else:
                chain_rows = tuple()
                for arg in args:
                    chain_rows += arg.chain
                stack.append(_Expr(f"{name}({', '.join([*(arg.source for arg in args), *params])})", chain_rows + _chain_rows([[name, *params] if params else name])))
            continue
        raise SolveScoreProgramSourceError(f"cannot serialize solve-score row {row!r}")

    if not explicit_emits:
        if len(stack) != 1:
            raise SolveScoreProgramSourceError(f"solve-score chain ended with stack depth {len(stack)}")
        top = stack[-1]
        if top.materialized:
            lines.append("score = pop()")
        else:
            lines.append(f"score = {top.source}")
    elif stack:
        raise SolveScoreProgramSourceError(f"explicit-output chain ended with stack depth {len(stack)}")

    source_text = "\n".join(lines)
    reparsed = compile_solve_score_program_source(source_text, strict=True)
    if reparsed["program_spec"] != compiled["program_spec"]:
        raise SolveScoreProgramSourceError("source round-trip changed program_spec", code="source_roundtrip_failed")
    if reparsed["fingerprint"] != compiled_solve_score_fingerprint(compiled):
        raise SolveScoreProgramSourceError("source round-trip changed fingerprint", code="source_roundtrip_failed")
    return source_text


def solve_score_source_text_from_payload(payload):
    if isinstance(payload, dict):
        raw = str(payload.get("source_text") or payload.get("solve_score_program_source_text") or "")
        if raw.strip():
            return raw
        if "chain" in payload:
            return solve_score_source_text_from_chain(payload.get("chain"))
    return solve_score_source_text_from_chain(payload)
