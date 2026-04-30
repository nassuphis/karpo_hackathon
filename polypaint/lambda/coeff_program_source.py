"""
Authoritative source-text parser for Coeff Program v1.

The browser may provide highlighting and advisory diagnostics, but this module
is the only source parser used by save, preview, and compute-plan paths.
It lowers source text to the existing coeff_program_chain chip list, then lets
coeff_program_chain compile that list to VM tokens.
"""
from __future__ import annotations

import re
from dataclasses import dataclass

from coeff_program_chain import (
    MAX_VECTOR_LEN,
    compile_coeff_program_chain,
    display_coeff_program_chain,
    legacy_registry,
)


MAX_COEFF_PROGRAM_SOURCE_BYTES = 64 * 1024

_IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
_INDEX_RE = re.compile(r"^poly\[(\d+)\]$", re.IGNORECASE)

_VECTOR_BINARY_ALIASES = {
    "add": "add",
    "sub": "subtract",
    "subtract": "subtract",
    "mul": "multiply",
    "multiply": "multiply",
    "div": "divide",
    "divide": "divide",
    "pow": "power",
    "power": "power",
}

_VECTOR_UNARY_NAMES = {
    "angle",
    "mod",
    "abs",
    "neg",
    "conj",
    "sqrt",
    "log",
}

_LEGACY_UNARY_NAMES = {
    "rev",
    "normalize",
    "deriv",
    "safe",
    "negate_odd",
    "max2one",
    "sort_mod_keep_angle",
    "sort_angle_keep_mod",
    "sort_abs",
    "cumsum",
    "cummax",
    "sort_cumsum",
    "swirler",
    "exp",
    "cos",
    "sin",
    "tan",
    "cosh",
    "sinh",
    "tanh",
    "round",
}

_STACK_ALIASES = {
    "dup": "duplicate",
    "duplicate": "duplicate",
    "swap": "swap",
    "drop": "pop",
    "flush": "flush",
    "emit": "emit",
}

_SOURCE_NAMES = {"cf", "poly", "pop", "peek"}
_TARGET_NAMES = {"poly", "push"}


@dataclass(frozen=True)
class _Statement:
    text: str
    line: int
    column: int


class CoeffProgramSourceError(RuntimeError):
    def __init__(self, message: str, *, line: int = 1, column: int = 1):
        super().__init__(message)
        self.line = line
        self.column = column


def _diagnostic(message, *, line=1, column=1, level="error"):
    return {
        "level": level,
        "message": str(message),
        "line": int(line),
        "column": int(column),
    }


def _canonical_expr(text):
    raw = str(text or "").strip()
    if not raw:
        raise CoeffProgramSourceError("expression is empty")
    raw = re.sub(r"\bmod\s*\(", "abs(", raw, flags=re.IGNORECASE)
    raw = re.sub(r"\btau_i\b", "pi2i", raw, flags=re.IGNORECASE)
    raw = re.sub(r"\btau\b", "pi2", raw, flags=re.IGNORECASE)
    return re.sub(r"\s+", "", raw)


def _split_top_level(text, sep=","):
    out = []
    start = 0
    paren = bracket = 0
    for idx, ch in enumerate(text):
        if ch == "(":
            paren += 1
        elif ch == ")":
            paren -= 1
            if paren < 0:
                raise CoeffProgramSourceError("unexpected closing parenthesis")
        elif ch == "[":
            bracket += 1
        elif ch == "]":
            bracket -= 1
            if bracket < 0:
                raise CoeffProgramSourceError("unexpected closing bracket")
        elif ch == sep and paren == 0 and bracket == 0:
            out.append(text[start:idx].strip())
            start = idx + 1
    if paren != 0:
        raise CoeffProgramSourceError("unclosed parenthesis")
    if bracket != 0:
        raise CoeffProgramSourceError("unclosed bracket")
    out.append(text[start:].strip())
    return out


def _find_top_level_assignment(text):
    paren = bracket = 0
    for idx, ch in enumerate(text):
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


def _source_selector(text, *, allow_cf=True):
    raw = str(text or "").strip().lower()
    allowed = _SOURCE_NAMES if allow_cf else (_SOURCE_NAMES - {"cf"})
    if raw not in allowed:
        raise CoeffProgramSourceError(f"expected source selector, got {text!r}")
    return raw


def _target_selector(text):
    raw = str(text or "").strip().lower()
    if raw not in _TARGET_NAMES:
        raise CoeffProgramSourceError(f"expected target selector, got {text!r}")
    return raw


def _lower_const(args, *, target):
    if len(args) == 1:
        return [["push_const", "poly_len", _canonical_expr(args[0])]]
    if len(args) == 2:
        return [["push_const", _canonical_expr(args[0]), _canonical_expr(args[1])]]
    raise CoeffProgramSourceError("fill/const requires value or length, value")


def _lower_range(name, args):
    if name == "linspace":
        if len(args) not in {1, 3}:
            raise CoeffProgramSourceError("linspace requires length or start, stop, count")
        return [["push_linspace"] + [_canonical_expr(arg) for arg in args]]
    if len(args) not in {1, 2, 3}:
        raise CoeffProgramSourceError("arange/range requires length, start/stop, or start/stop/step")
    return [["push_range"] + [_canonical_expr(arg) for arg in args]]


def _lower_affine(name, args, *, target):
    if name == "scale":
        if len(args) == 1:
            src, a = "pop", args[0]
        elif len(args) == 2:
            src, a = _source_selector(args[0]), args[1]
        else:
            raise CoeffProgramSourceError("scale requires multiplier or source, multiplier")
        return [["affine", target, src, _canonical_expr(a), "0"]]
    if name == "shift":
        if len(args) == 1:
            src, b = "pop", args[0]
        elif len(args) == 2:
            src, b = _source_selector(args[0]), args[1]
        else:
            raise CoeffProgramSourceError("shift requires offset or source, offset")
        return [["affine", target, src, "1", _canonical_expr(b)]]
    if len(args) == 2:
        src, a, b = "pop", args[0], args[1]
    elif len(args) == 3:
        src, a, b = _source_selector(args[0]), args[1], args[2]
    else:
        raise CoeffProgramSourceError("linear requires multiplier, offset or source, multiplier, offset")
    return [["affine", target, src, _canonical_expr(a), _canonical_expr(b)]]


def _lower_vector_binary(name, args, *, target):
    chip = _VECTOR_BINARY_ALIASES[name]
    if len(args) == 0:
        src1, src2 = "pop", "pop"
    elif len(args) == 2:
        src1, src2 = _source_selector(args[0]), _source_selector(args[1])
    else:
        raise CoeffProgramSourceError(f"{name} requires no args or src1, src2")
    if src1 == "cf" or src2 == "cf":
        if src1 not in {"cf", "poly"} or src2 not in {"cf", "poly"}:
            raise CoeffProgramSourceError(
                f"{name} with cf can only stage cf/poly sources in v1"
            )
        staged = []
        for src in (src2, src1):
            staged.append(["push", src])
        staged.append([chip, target, "pop", "pop"])
        return staged
    return [[chip, target, src1, src2]]


def _lower_vector_unary(name, args, *, target):
    chip = "abs" if name == "mod" else name
    if len(args) == 0:
        src = "pop"
    elif len(args) == 1:
        src = _source_selector(args[0], allow_cf=False)
    else:
        raise CoeffProgramSourceError(f"{name} requires no args or one source")
    return [[chip, target, src]]


def _lower_legacy_call(name, args, *, target):
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
    return [["legacy", name, src, target] + [_canonical_expr(arg) for arg in fn_args]]


def _lower_call(name, args, *, target="push"):
    name = str(name or "").strip().lower()
    if name in {"fill", "const", "push_const"}:
        return _lower_const(args, target=target)
    if name in {"arange", "range", "push_range"}:
        return _lower_range("range", args)
    if name in {"linspace", "push_linspace"}:
        return _lower_range("linspace", args)
    if name in {"scale", "shift", "linear", "affine"}:
        if name == "affine":
            if len(args) != 4:
                raise CoeffProgramSourceError("affine requires target, source, multiplier, offset")
            return [["affine", _target_selector(args[0]), _source_selector(args[1]), _canonical_expr(args[2]), _canonical_expr(args[3])]]
        return _lower_affine(name, args, target=target)
    if name in _VECTOR_BINARY_ALIASES:
        return _lower_vector_binary(name, args, target=target)
    if name in _VECTOR_UNARY_NAMES:
        return _lower_vector_unary(name, args, target=target)
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
        if target != "push":
            raise CoeffProgramSourceError("blend writes to stack; assign it with emit or set afterward")
        if len(args) != 1:
            raise CoeffProgramSourceError("blend requires t")
        return [["blend", _canonical_expr(args[0])]]
    if name in {"poke_poly", "poke_tos"}:
        if len(args) != 2:
            raise CoeffProgramSourceError(f"{name} requires index, value")
        return [[name, _canonical_expr(args[0]), _canonical_expr(args[1])]]
    if name == "littlewood":
        if len(args) not in {2, 3}:
            raise CoeffProgramSourceError("littlewood requires value1, value2, optional andy")
        return [["littlewood", target] + [_canonical_expr(arg) for arg in args]]
    if name == "macro":
        if len(args) != 1:
            raise CoeffProgramSourceError("macro requires one program name")
        macro_name = args[0].strip()
        if not macro_name:
            raise CoeffProgramSourceError("macro name is empty")
        return [["macro", macro_name]]
    if name == "legacy":
        if len(args) < 3:
            raise CoeffProgramSourceError("legacy requires name, src, tgt, optional args")
        return [["legacy", args[0].strip().lower(), _source_selector(args[1]), _target_selector(args[2])] + [_canonical_expr(arg) for arg in args[3:]]]
    if name in _LEGACY_UNARY_NAMES or name in legacy_registry()["by_name"]:
        return _lower_legacy_call(name, args, target=target)
    raise CoeffProgramSourceError(f"unknown coeff program source function {name!r}")


def _lower_statement(statement):
    text = statement.text.strip()
    assignment = _find_top_level_assignment(text)
    if assignment >= 0:
        lhs = re.sub(r"\s+", "", text[:assignment])
        rhs = text[assignment + 1:].strip()
        if not rhs:
            raise CoeffProgramSourceError("assignment right-hand side is empty", line=statement.line, column=assignment + 2)
        index_match = _INDEX_RE.match(lhs)
        if index_match:
            idx = int(index_match.group(1))
            if idx < 0 or idx >= MAX_VECTOR_LEN:
                raise CoeffProgramSourceError(
                    f"poly index must be in [0,{MAX_VECTOR_LEN - 1}], got {idx}",
                    line=statement.line,
                    column=statement.column,
                )
            return [["poke_poly", str(idx), _canonical_expr(rhs)]]
        if lhs.lower() != "poly":
            raise CoeffProgramSourceError("only poly assignments are supported in v1", line=statement.line, column=statement.column)
        call = _parse_call(rhs)
        if call:
            return _lower_call(call[0], call[1], target="poly")
        bare = rhs.strip().lower()
        if bare in _SOURCE_NAMES:
            return [["set", "poly", bare]]
        if _IDENT_RE.match(bare):
            return _lower_call(bare, [], target="poly")
        raise CoeffProgramSourceError("poly assignment requires a source or function call", line=statement.line, column=statement.column)

    call = _parse_call(text)
    if call:
        return _lower_call(call[0], call[1], target="push")
    bare = text.strip().lower()
    if bare in {"cf", "poly"}:
        return [["push", bare]]
    if bare in _STACK_ALIASES:
        return [[_STACK_ALIASES[bare]]]
    if bare in {"pop", "peek"}:
        hint = "use drop to discard the stack top, or write poly = pop / poly = peek explicitly"
        raise CoeffProgramSourceError(f"{bare} is not a standalone statement; {hint}", line=statement.line, column=statement.column)
    if _IDENT_RE.match(bare):
        return _lower_call(bare, [], target="push")
    raise CoeffProgramSourceError(f"cannot parse coeff program statement {text!r}", line=statement.line, column=statement.column)


def split_coeff_program_statements(source_text):
    text = str(source_text or "")
    source_bytes = len(text.encode("utf-8"))
    if source_bytes > MAX_COEFF_PROGRAM_SOURCE_BYTES:
        raise CoeffProgramSourceError(
            f"coeff program source is {source_bytes} bytes; max is {MAX_COEFF_PROGRAM_SOURCE_BYTES}"
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
        if ch == "#" and paren == 0 and bracket == 0:
            in_comment = True
            col += 1
            continue
        if ch == "(":
            paren += 1
        elif ch == ")":
            paren -= 1
            if paren < 0:
                raise CoeffProgramSourceError("unexpected closing parenthesis", line=line, column=col)
        elif ch == "[":
            bracket += 1
        elif ch == "]":
            bracket -= 1
            if bracket < 0:
                raise CoeffProgramSourceError("unexpected closing bracket", line=line, column=col)
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
        raise CoeffProgramSourceError("unclosed parenthesis", line=line, column=col)
    if bracket != 0:
        raise CoeffProgramSourceError("unclosed bracket", line=line, column=col)
    stmt = "".join(buffer).strip()
    if stmt:
        statements.append(_Statement(stmt, start_line, start_col))
    return statements


def parse_coeff_program_source(source_text, *, strict=True):
    diagnostics = []
    chain = []
    try:
        statements = split_coeff_program_statements(source_text)
    except CoeffProgramSourceError as exc:
        diagnostics.append(_diagnostic(exc, line=exc.line, column=exc.column))
        if strict:
            raise RuntimeError(str(exc))
        return {"chain": [], "display": "", "statement_count": 0, "diagnostics": diagnostics}
    for stmt in statements:
        try:
            chain.extend(_lower_statement(stmt))
        except CoeffProgramSourceError as exc:
            diagnostics.append(_diagnostic(exc, line=exc.line or stmt.line, column=exc.column or stmt.column))
        except Exception as exc:
            diagnostics.append(_diagnostic(exc, line=stmt.line, column=stmt.column))
    if diagnostics and strict:
        raise RuntimeError("; ".join(d["message"] for d in diagnostics if d.get("level") == "error"))
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


def compile_coeff_program_source_diagnostics(source_text, *, macro_resolver=None):
    return compile_coeff_program_source(source_text, macro_resolver=macro_resolver, strict=False)
