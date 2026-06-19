"""
Param-program compiler helpers.

This module owns source-chain validation, macro expansion, stack-effect
validation, canonical execution tokens, and fingerprints. Native row evaluators
must receive only the compiled integer token stream; source names and macro names
are compile-time concerns only.

Legacy bridge rows (``legacy(...)`` and bare legacy transform names) are
supported in Program mode by design: they are the compatibility path for
Param-Chain stacks copied into programs and for old saved programs, and are
pinned by the param-program test suites. Removing them would orphan saved
payloads, so any retirement needs a payload migration, not a code deletion.
"""
from __future__ import annotations

import hashlib
import cmath
import json
import math
import os
import re


PROGRAM_KIND = "param_program"
PROGRAM_VERSION = 1
MAX_PROGRAM_TOKENS = 64
MAX_STACK = 16
MAX_MACRO_DEPTH = 8
MAX_ARGS = 8
MAX_SCALAR_EXPR_TOKENS = 32
MAX_LOWERED_PARAM_TOKENS_PER_EXPR = 32

PARAM_OP_PUSH_T1 = 1
PARAM_OP_PUSH_T2 = 2
PARAM_OP_EMIT_P1 = 3
PARAM_OP_EMIT_P2 = 4
PARAM_OP_CONST = 5
PARAM_OP_DUPLICATE = 6
PARAM_OP_SWAP = 7
PARAM_OP_POP = 8
PARAM_OP_FLUSH = 9
PARAM_OP_ADD = 10
PARAM_OP_SUBTRACT = 11
PARAM_OP_MUL = 12
PARAM_OP_RATIO = 13
PARAM_OP_NEGATE = 14
PARAM_OP_CONJ = 15
PARAM_OP_RECIPROCAL = 16
PARAM_OP_UNIT_CIRCLE = 17
PARAM_OP_SQUARE = 18
PARAM_OP_CUBE = 19
PARAM_OP_EXP = 20
PARAM_OP_LEGACY = 21
PARAM_OP_PUSH_P1 = 22
PARAM_OP_PUSH_P2 = 23
PARAM_OP_REAL = 24
PARAM_OP_IMAG = 25
PARAM_OP_ABS = 26
PARAM_OP_DIVIDE = 27

PARAM_SEL_P1 = 1
PARAM_SEL_P2 = 2
PARAM_SEL_BOTH = 3
PARAM_SEL_POP1 = 4
PARAM_SEL_POP2 = 5
PARAM_SEL_PUSH1 = 6
PARAM_SEL_PUSH2 = 7

EXPR_LITERAL = 1
EXPR_T1 = 2
EXPR_T2 = 3
EXPR_P1 = 4
EXPR_P2 = 5
EXPR_ADD = 6
EXPR_SUB = 7
EXPR_MUL = 8
EXPR_DIV = 9
EXPR_NEG = 10
EXPR_EXP = 11
EXPR_REAL = 12
EXPR_IMAG = 13
EXPR_ABS = 14

_OP_NAMES = {
    PARAM_OP_PUSH_T1: "push_t1",
    PARAM_OP_PUSH_T2: "push_t2",
    PARAM_OP_EMIT_P1: "emit_p1",
    PARAM_OP_EMIT_P2: "emit_p2",
    PARAM_OP_CONST: "const",
    PARAM_OP_DUPLICATE: "duplicate",
    PARAM_OP_SWAP: "swap",
    PARAM_OP_POP: "pop",
    PARAM_OP_FLUSH: "flush",
    PARAM_OP_ADD: "add",
    PARAM_OP_SUBTRACT: "subtract",
    PARAM_OP_MUL: "mul",
    PARAM_OP_RATIO: "ratio",
    PARAM_OP_NEGATE: "negate",
    PARAM_OP_CONJ: "conj",
    PARAM_OP_RECIPROCAL: "reciprocal",
    PARAM_OP_UNIT_CIRCLE: "unit_circle",
    PARAM_OP_SQUARE: "square",
    PARAM_OP_CUBE: "cube",
    PARAM_OP_EXP: "exp",
    PARAM_OP_LEGACY: "legacy",
    PARAM_OP_PUSH_P1: "push_p1",
    PARAM_OP_PUSH_P2: "push_p2",
    PARAM_OP_REAL: "real",
    PARAM_OP_IMAG: "imag",
    PARAM_OP_ABS: "abs",
    PARAM_OP_DIVIDE: "divide",
}

_SELECTOR_NAMES = {
    PARAM_SEL_P1: "p1",
    PARAM_SEL_P2: "p2",
    PARAM_SEL_BOTH: "both",
    PARAM_SEL_POP1: "pop1",
    PARAM_SEL_POP2: "pop2",
    PARAM_SEL_PUSH1: "push1",
    PARAM_SEL_PUSH2: "push2",
}

_SOURCE_SELECTORS = {
    "p1": PARAM_SEL_P1,
    "p2": PARAM_SEL_P2,
    "both": PARAM_SEL_BOTH,
    "pop1": PARAM_SEL_POP1,
    "pop2": PARAM_SEL_POP2,
}

_TARGET_SELECTORS = {
    "p1": PARAM_SEL_P1,
    "p2": PARAM_SEL_P2,
    "both": PARAM_SEL_BOTH,
    "push1": PARAM_SEL_PUSH1,
    "push2": PARAM_SEL_PUSH2,
}

_PUSH_TARGETS = {
    "t1": PARAM_OP_PUSH_T1,
    "0": PARAM_OP_PUSH_T1,
    "t2": PARAM_OP_PUSH_T2,
    "1": PARAM_OP_PUSH_T2,
}

_EMIT_TARGETS = {
    "p1": PARAM_OP_EMIT_P1,
    "0": PARAM_OP_EMIT_P1,
    "p2": PARAM_OP_EMIT_P2,
    "1": PARAM_OP_EMIT_P2,
}

_BINARY_OPS = {
    "add": PARAM_OP_ADD,
    "subtract": PARAM_OP_SUBTRACT,
    "sub": PARAM_OP_SUBTRACT,
    "mul": PARAM_OP_MUL,
    "ratio": PARAM_OP_RATIO,
    "div": PARAM_OP_RATIO,
}

_UNARY_OPS = {
    "negate": PARAM_OP_NEGATE,
    "conj": PARAM_OP_CONJ,
    "conjugate": PARAM_OP_CONJ,
    "reciprocal": PARAM_OP_RECIPROCAL,
    "unit_circle": PARAM_OP_UNIT_CIRCLE,
    "square": PARAM_OP_SQUARE,
    "cube": PARAM_OP_CUBE,
    "exp": PARAM_OP_EXP,
}

_STACK_OPS = {
    "duplicate": PARAM_OP_DUPLICATE,
    "dup": PARAM_OP_DUPLICATE,
    "swap": PARAM_OP_SWAP,
    "pop": PARAM_OP_POP,
    "flush": PARAM_OP_FLUSH,
}

_TARGETABLE_UNARY_SOURCE = {
    "unit_circle",
    "square",
    "cube",
    "reciprocal",
    "conj",
    "conjugate",
    "negate",
    "exp",
}

_REDUNDANT_LEGACY_TARGET_ARG_NAMES = {
    "unit_circle",
    "square",
    "cube",
    "reciprocal",
    "conjugate",
    "negate",
    "exp",
    "xim",
}

_LEGACY_TARGET_ARG_INDEXES = {
    "rtheta": 1,
    "crd": 0,
    "hrt": 0,
    "spdl": 0,
    "lmc": 0,
    "rsc": 0,
    "lss": 0,
    "ast": 0,
    "asp": 0,
    "lsp": 0,
    "dlt": 0,
    "rply": 0,
    "star": 0,
    "rect": 0,
    "rrect": 0,
}

_VARIABLE_LEGACY_ARG_COUNTS = {
    "moebius": {0, 4, 8},
    "inv_t_plus_2": {0, 1, 2, 3, 4},
}


def _registry_path():
    return os.path.join(os.path.dirname(__file__), "param_legacy_registry.json")


def _load_legacy_registry():
    with open(_registry_path(), "r", encoding="utf-8") as fh:
        payload = json.load(fh)
    if int(payload.get("version") or 0) != 1:
        raise RuntimeError("param legacy registry version must be 1")
    by_name = {}
    by_index = {}
    for fn in payload.get("functions") or []:
        name = str(fn.get("name") or "").strip()
        if not name:
            raise RuntimeError("param legacy registry function missing name")
        if name in by_name:
            raise RuntimeError(f"duplicate param legacy function name: {name}")
        try:
            fn_index = int(fn.get("fn_index"))
        except (TypeError, ValueError):
            raise RuntimeError(f"param legacy function {name} has invalid fn_index")
        if fn_index <= 0:
            raise RuntimeError(f"param legacy function {name} fn_index must be positive")
        if fn_index in by_index:
            raise RuntimeError(f"duplicate param legacy fn_index: {fn_index}")
        args_raw = fn.get("args") or []
        if len(args_raw) > MAX_ARGS:
            raise RuntimeError(f"param legacy function {name} has too many args")
        args = []
        for idx, arg in enumerate(args_raw):
            if not isinstance(arg, dict):
                raise RuntimeError(f"param legacy function {name} arg {idx} must be an object")
            normalized_arg = dict(arg)
            arg_type = str(normalized_arg.get("type") or "real").strip().lower()
            if arg_type not in {"real", "complex"}:
                raise RuntimeError(
                    f"param legacy function {name} arg {idx} has unsupported type {arg_type!r}"
                )
            normalized_arg["type"] = arg_type
            args.append(normalized_arg)
        spec = {
            "name": name,
            "fn_index": fn_index,
            "kind": str(fn.get("kind") or "").strip(),
            "allowed_src": tuple(str(x).strip() for x in (fn.get("allowed_src") or [])),
            "allowed_tgt": tuple(str(x).strip() for x in (fn.get("allowed_tgt") or [])),
            "args": tuple(args),
        }
        by_name[name] = spec
        by_index[fn_index] = spec
    return {"by_name": by_name, "by_index": by_index}


_LEGACY_REGISTRY = None


def legacy_registry():
    global _LEGACY_REGISTRY
    if _LEGACY_REGISTRY is None:
        _LEGACY_REGISTRY = _load_legacy_registry()
    return _LEGACY_REGISTRY


def validate_legacy_registry():
    registry = legacy_registry()
    return {
        "version": 1,
        "count": len(registry["by_name"]),
        "names": sorted(registry["by_name"]),
        "fn_indices": sorted(registry["by_index"]),
    }


def _finite_number(value, label):
    try:
        number = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{label} must be numeric, got {value!r}")
    if not math.isfinite(number):
        raise RuntimeError(f"{label} must be finite, got {value!r}")
    return number


_COMPLEX_TERM_RE = re.compile(r"[+-]?(?:(?:\d+(?:\.\d*)?)|(?:\.\d+))(?:[eE][+-]?\d+)?")


def _parse_complex_literal(value):
    raw = str(value or "").strip().replace("i", "j").replace("I", "j").replace(" ", "")
    if not raw:
        raise ValueError("empty complex literal")
    real = 0.0
    imag = 0.0
    pos = 0
    saw = False
    while pos < len(raw):
        match = _COMPLEX_TERM_RE.match(raw, pos)
        if match:
            number = float(match.group(0))
            pos = match.end()
            if pos < len(raw) and raw[pos] == "j":
                imag += number
                pos += 1
            else:
                real += number
            saw = True
        elif raw[pos] == "j":
            imag += 1.0
            pos += 1
            saw = True
        elif raw[pos] in "+-" and pos + 1 < len(raw) and raw[pos + 1] == "j":
            imag += -1.0 if raw[pos] == "-" else 1.0
            pos += 2
            saw = True
        else:
            raise ValueError(f"invalid complex literal {value!r}")
        if not math.isfinite(real) or not math.isfinite(imag):
            raise ValueError(f"non-finite complex literal {value!r}")
        if pos < len(raw) and raw[pos] not in "+-":
            raise ValueError(f"invalid complex literal {value!r}")
    if not saw:
        raise ValueError(f"invalid complex literal {value!r}")
    return complex(real, imag)


def _format_number(value):
    number = _finite_number(value, "number")
    if number == 0:
        number = 0.0
    return format(number, ".17g")


def _format_complex_number(real, imag):
    real_number = _finite_number(real, "complex real")
    imag_number = _finite_number(imag, "complex imag")
    if real_number == 0:
        real_number = 0.0
    if imag_number == 0:
        imag_number = 0.0
    real_text = _format_number(real_number)
    imag_text = _format_number(abs(imag_number))
    if imag_number == 0:
        return real_text
    if real_number == 0:
        return f"{'-' if imag_number < 0 else ''}{imag_text}i"
    return f"{real_text}{'-' if imag_number < 0 else '+'}{imag_text}i"


def _slugify_macro_id(value):
    text = str(value or "").strip()
    if not text:
        raise RuntimeError("macro name is required")
    return re.sub(r"[^a-zA-Z0-9._-]+", "-", text).strip("-") or text


def _chip_name(value, idx):
    name = str(value or "").strip()
    if not name:
        raise RuntimeError(f"param program chip {idx} has empty name")
    return name.lower()


def _chip_args(chip):
    if isinstance(chip, str):
        return _chip_name(chip, "?"), []
    if isinstance(chip, (list, tuple)):
        if not chip:
            raise RuntimeError("param program chip cannot be an empty array")
        return _chip_name(chip[0], "?"), list(chip[1:])
    raise RuntimeError(f"param program chip must be a string or array, got {chip!r}")


def _canonical_source_chain(chain):
    if not isinstance(chain, list):
        raise RuntimeError("param program chain must be a JSON array")
    out = []
    for idx, chip in enumerate(chain):
        if isinstance(chip, str):
            out.append(_chip_name(chip, idx))
            continue
        if not isinstance(chip, list) or not chip:
            raise RuntimeError(f"param program chip {idx} must be a non-empty array or string")
        name = _chip_name(chip[0], idx)
        entry = [name]
        for arg_idx, arg in enumerate(chip[1:]):
            if isinstance(arg, str):
                if len(arg) > 256:
                    raise RuntimeError(f"param program chip {idx} arg {arg_idx} is too long")
                entry.append(arg.strip())
            elif isinstance(arg, (int, float)):
                entry.append(_format_number(arg))
            else:
                raise RuntimeError(
                    f"param program chip {idx} arg {arg_idx} must be a string or number"
                )
        if name == "const" and len(entry) == 3:
            entry = ["const", f"({entry[1]})+({entry[2]})*1j"]
        out.append(_canonicalize_legacy_bridge_entry(entry))
    return out


def _target_value_for_selector(selector):
    normalized = _normalize_target(selector)
    if normalized == "p1":
        return "0"
    if normalized == "p2":
        return "1"
    return "2"


def _migrate_legacy_target_arg(name, src, tgt, args, *, force=False):
    target_idx = _LEGACY_TARGET_ARG_INDEXES.get(name)
    values = [str(arg).strip() for arg in args]
    if target_idx is None or len(values) <= target_idx:
        return src, tgt, values
    registry_spec = legacy_registry()["by_name"].get(name)
    declared_arg_count = len(registry_spec["args"]) if registry_spec else 0
    if not force and len(values) <= declared_arg_count:
        return src, tgt, values
    try:
        selector = _normalize_target(values[target_idx])
    except RuntimeError:
        return src, tgt, values
    next_src = selector if src == "both" else src
    next_tgt = selector if tgt == "both" else tgt
    return next_src, next_tgt, [arg for idx, arg in enumerate(values) if idx != target_idx]


def _canonicalize_legacy_bridge_entry(entry):
    if entry and entry[0] != "legacy" and entry[0] in legacy_registry()["by_name"]:
        name = entry[0]
        src, tgt, args = _migrate_legacy_target_arg(name, "both", "both", entry[1:], force=True)
        if src != "both" or tgt != "both" or name in _LEGACY_TARGET_ARG_INDEXES:
            return ["legacy", name, src, tgt, *args]
        return entry
    if (
        len(entry) == 5 and
        entry[0] == "legacy" and
        entry[1] in _REDUNDANT_LEGACY_TARGET_ARG_NAMES
    ):
        try:
            selector = _normalize_target(entry[4])
        except RuntimeError:
            return entry
        cleaned = entry[:4]
        if cleaned[2] == "both" and cleaned[3] == "both":
            cleaned[2] = selector
            cleaned[3] = selector
        return cleaned
    if entry and entry[0] == "legacy" and len(entry) > 4:
        src, tgt, args = _migrate_legacy_target_arg(entry[1], entry[2], entry[3], entry[4:])
        if src != entry[2] or tgt != entry[3] or len(args) != len(entry[4:]):
            return ["legacy", entry[1], src, tgt, *args]
    return entry


def _display_chip(chip):
    if isinstance(chip, str):
        return chip
    if len(chip) == 1:
        return chip[0]
    return f"{chip[0]}(" + ", ".join(str(arg) for arg in chip[1:]) + ")"


def display_param_program_chain(chain):
    return "; ".join(_display_chip(chip) for chip in chain)


class _Expr:
    __slots__ = ("tokens", "kind", "dynamic", "value")

    def __init__(self, tokens, *, kind="complex", dynamic=True, value=None):
        self.tokens = tokens
        self.kind = kind
        self.dynamic = dynamic
        self.value = value


def _expr_literal(value):
    if not math.isfinite(value.real) or not math.isfinite(value.imag):
        raise RuntimeError("param expression literal must be finite")
    kind = "complex" if value.imag else "real"
    return _Expr(
        [{"op": EXPR_LITERAL, "a": value.real, "b": value.imag}],
        kind=kind,
        dynamic=False,
        value=value,
    )


def _expr_dynamic(op, *, kind="complex"):
    return _Expr([{"op": op}], kind=kind, dynamic=True, value=None)


def _expr_binary(left, right, op):
    if left.value is not None and right.value is not None:
        if op == EXPR_ADD:
            return _expr_literal(left.value + right.value)
        if op == EXPR_SUB:
            return _expr_literal(left.value - right.value)
        if op == EXPR_MUL:
            return _expr_literal(left.value * right.value)
        if op == EXPR_DIV:
            if abs(right.value) <= 1e-300:
                raise RuntimeError("param expression division by zero")
            return _expr_literal(left.value / right.value)
    return _Expr(
        left.tokens + right.tokens + [{"op": op}],
        kind="complex" if left.kind == "complex" or right.kind == "complex" else "real",
        dynamic=True,
        value=None,
    )


def _expr_unary(expr, op, *, kind=None):
    if expr.value is not None:
        if op == EXPR_NEG:
            return _expr_literal(-expr.value)
        if op == EXPR_EXP:
            return _expr_literal(cmath.exp(expr.value))
        if op == EXPR_REAL:
            return _expr_literal(complex(expr.value.real, 0.0))
        if op == EXPR_IMAG:
            return _expr_literal(complex(expr.value.imag, 0.0))
        if op == EXPR_ABS:
            return _expr_literal(complex(abs(expr.value), 0.0))
    return _Expr(
        expr.tokens + [{"op": op}],
        kind=kind or expr.kind,
        dynamic=True,
        value=None,
    )


_EXPR_TOKEN_RE = re.compile(
    r"\s*(?:(?P<number>(?:(?:\d+(?:\.\d*)?)|(?:\.\d+))(?:[eE][+-]?\d+)?)(?P<imag>[ijIJ])?|(?P<ident>[A-Za-z_][A-Za-z0-9_]*)|(?P<op>[()+\-*/]))"
)

_EXPR_CONSTANTS = {
    "pi": complex(math.pi, 0.0),
    "pi2": complex(2.0 * math.pi, 0.0),
    "pi2i": complex(0.0, 2.0 * math.pi),
}


class _ExpressionParser:
    def __init__(self, text):
        self.text = str(text or "").strip()
        self.tokens = self._tokenize(self.text)
        self.pos = 0

    @staticmethod
    def _tokenize(text):
        out = []
        pos = 0
        while pos < len(text):
            match = _EXPR_TOKEN_RE.match(text, pos)
            if not match:
                raise RuntimeError(f"invalid param expression near {text[pos:]!r}")
            pos = match.end()
            if match.group("number") is not None:
                number = float(match.group("number"))
                if not math.isfinite(number):
                    raise RuntimeError("param expression number must be finite")
                out.append(("number", complex(0.0, number) if match.group("imag") else complex(number, 0.0)))
            elif match.group("ident") is not None:
                out.append(("ident", match.group("ident").lower()))
            else:
                out.append((match.group("op"), match.group("op")))
        return out

    def _peek(self):
        return self.tokens[self.pos] if self.pos < len(self.tokens) else (None, None)

    def _take(self):
        token = self._peek()
        self.pos += 1
        return token

    def parse(self):
        if not self.tokens:
            raise RuntimeError("param expression is empty")
        expr = self._expr()
        if self.pos != len(self.tokens):
            raise RuntimeError(f"unexpected param expression token {self._peek()[1]!r}")
        if len(expr.tokens) > MAX_SCALAR_EXPR_TOKENS:
            raise RuntimeError(
                f"param expression has {len(expr.tokens)} tokens; max is {MAX_SCALAR_EXPR_TOKENS}"
            )
        return expr

    def _expr(self):
        left = self._term()
        while self._peek()[0] in {"+", "-"}:
            op = self._take()[0]
            right = self._term()
            left = _expr_binary(left, right, EXPR_ADD if op == "+" else EXPR_SUB)
        return left

    def _term(self):
        left = self._unary()
        while self._peek()[0] in {"*", "/"}:
            op = self._take()[0]
            right = self._unary()
            left = _expr_binary(left, right, EXPR_MUL if op == "*" else EXPR_DIV)
        return left

    def _unary(self):
        token_type, token_value = self._peek()
        if token_type == "+":
            self._take()
            return self._unary()
        if token_type == "-":
            self._take()
            return _expr_unary(self._unary(), EXPR_NEG)
        if token_type == "ident" and token_value in {"exp", "real", "imag", "abs", "mod"}:
            self._take()
            if self._take()[0] != "(":
                raise RuntimeError(f"{token_value} requires parentheses")
            expr = self._expr()
            if self._take()[0] != ")":
                raise RuntimeError(f"{token_value} missing closing parenthesis")
            if token_value == "exp":
                return _expr_unary(expr, EXPR_EXP, kind="complex")
            if token_value == "real":
                return _expr_unary(expr, EXPR_REAL, kind="real")
            if token_value == "imag":
                return _expr_unary(expr, EXPR_IMAG, kind="real")
            return _expr_unary(expr, EXPR_ABS, kind="real")
        return self._primary()

    def _primary(self):
        token_type, token_value = self._take()
        if token_type == "number":
            return _expr_literal(token_value)
        if token_type == "ident":
            if token_value in {"i", "j"}:
                return _expr_literal(complex(0.0, 1.0))
            if token_value in _EXPR_CONSTANTS:
                return _expr_literal(_EXPR_CONSTANTS[token_value])
            if token_value == "t1":
                return _expr_dynamic(EXPR_T1)
            if token_value == "t2":
                return _expr_dynamic(EXPR_T2)
            if token_value == "p1":
                return _expr_dynamic(EXPR_P1)
            if token_value == "p2":
                return _expr_dynamic(EXPR_P2)
            raise RuntimeError(f"unknown param expression identifier {token_value!r}")
        if token_type == "(":
            expr = self._expr()
            if self._take()[0] != ")":
                raise RuntimeError("param expression missing closing parenthesis")
            return expr
        raise RuntimeError(f"unexpected param expression token {token_value!r}")


def _compile_expr(value, *, label, expected="complex"):
    try:
        expr = _expr_literal(_parse_complex_literal(value))
    except (TypeError, ValueError):
        try:
            expr = _ExpressionParser(value).parse()
        except RuntimeError as exc:
            raise RuntimeError(f"{label}: {exc}") from None
    if expected == "real" and expr.kind != "real":
        raise RuntimeError(
            f"{label} must be real-valued; use real(...), imag(...), abs(...), or mod(...) explicitly"
        )
    return expr


def _expr_to_param_tokens(expr):
    if len(expr.tokens) > MAX_LOWERED_PARAM_TOKENS_PER_EXPR:
        raise RuntimeError(
            f"param expression lowers to {len(expr.tokens)} tokens; "
            f"max is {MAX_LOWERED_PARAM_TOKENS_PER_EXPR}"
        )
    out = []
    for token in expr.tokens:
        op = int(token.get("op") or 0)
        if op == EXPR_LITERAL:
            out.append(_token(
                PARAM_OP_CONST,
                a=float(token.get("a", 0.0) or 0.0),
                b=float(token.get("b", 0.0) or 0.0),
            ))
        elif op == EXPR_T1:
            out.append(_token(PARAM_OP_PUSH_T1))
        elif op == EXPR_T2:
            out.append(_token(PARAM_OP_PUSH_T2))
        elif op == EXPR_P1:
            out.append(_token(PARAM_OP_PUSH_P1))
        elif op == EXPR_P2:
            out.append(_token(PARAM_OP_PUSH_P2))
        elif op == EXPR_ADD:
            out.append(_token(PARAM_OP_ADD))
        elif op == EXPR_SUB:
            out.append(_token(PARAM_OP_SUBTRACT))
        elif op == EXPR_MUL:
            out.append(_token(PARAM_OP_MUL))
        elif op == EXPR_DIV:
            out.append(_token(PARAM_OP_DIVIDE))
        elif op == EXPR_NEG:
            out.append(_token(PARAM_OP_NEGATE))
        elif op == EXPR_EXP:
            out.append(_token(PARAM_OP_EXP))
        elif op == EXPR_REAL:
            out.append(_token(PARAM_OP_REAL))
        elif op == EXPR_IMAG:
            out.append(_token(PARAM_OP_IMAG))
        elif op == EXPR_ABS:
            out.append(_token(PARAM_OP_ABS))
        else:
            raise RuntimeError(f"unsupported param expression opcode: {op}")
    return out


def _token(op, **fields):
    tok = {"op": int(op)}
    for key in ("fn_index", "src", "tgt", "n_args", "stack_arg_count"):
        value = fields.get(key)
        if value not in (None, 0):
            tok[key] = int(value)
    if "a" in fields:
        tok["a"] = _finite_number(fields["a"], "token a")
    if "b" in fields:
        tok["b"] = _finite_number(fields["b"], "token b")
    args = fields.get("args")
    if args:
        tok["args"] = [_finite_number(x, "token arg") for x in args]
    args_im = fields.get("args_im")
    if args_im:
        tok["args_im"] = [_finite_number(x, "token arg imag") for x in args_im]
    return tok


def _normalize_target(value, *, default="both"):
    raw = str(default if value in (None, "") else value).strip().lower()
    if raw in {"t1", "p1", "0"}:
        return "p1"
    if raw in {"t2", "p2", "1"}:
        return "p2"
    if raw in {"both", "2"}:
        return "both"
    raise RuntimeError(f"param target must be t1, t2, or both, got {value!r}")


def _emit_target_op(value):
    raw = str(value or "").strip().lower()
    if raw in {"t1", "t2"}:
        raise RuntimeError("emit target cannot be t1/t2; t1/t2 are read-only inputs, emit to p1 or p2")
    if raw not in _EMIT_TARGETS:
        raise RuntimeError(f"emit target must be p1 or p2, got {value!r}")
    return _EMIT_TARGETS[raw]


def _push_target_ops(value):
    raw = str(value or "both").strip().lower()
    if raw in {"both", "2"}:
        return [PARAM_OP_PUSH_T1, PARAM_OP_PUSH_T2]
    if raw in {"p1", "p2"}:
        raise RuntimeError("push source cannot be p1/p2 in v1; use push(t1)/push(t2) or legacy(..., src=p1/p2, tgt=push1)")
    if raw not in _PUSH_TARGETS:
        raise RuntimeError(f"push source must be t1, t2, or both, got {value!r}")
    return [_PUSH_TARGETS[raw]]


def _selector_value(value, mapping, label):
    raw = str(value or "").strip().lower()
    if raw in {"t1", "t2"}:
        raise RuntimeError(
            f"{label} cannot be {raw}; t1/t2 are read-only inputs accessed via push(t1)/push(t2)"
        )
    if raw not in mapping:
        raise RuntimeError(f"{label} selector is invalid: {value!r}")
    return raw, mapping[raw]


def _legacy_arg_exprs(spec, raw_args):
    raw_args = list(raw_args)
    if spec["name"] == "moebius":
        if len(raw_args) == 0:
            return []
        if len(raw_args) == 4:
            exprs = []
            for idx, value in enumerate(raw_args):
                exprs.append(_compile_expr(
                    value,
                    label=f"legacy(moebius) coefficient {idx}",
                    expected="complex",
                ))
            return exprs
        if len(raw_args) == 8:
            exprs = []
            for idx, value in enumerate(raw_args):
                exprs.append(_compile_expr(
                    value,
                    label=f"legacy(moebius) old component arg {idx}",
                    expected="real",
                ))
            return exprs
        raise RuntimeError(f"legacy(moebius) expects 0, 4, or 8 arguments, got {len(raw_args)}")
    if spec["name"] == "inv_t_plus_2" and len(raw_args) in {0, 1, 2}:
        values = list(raw_args)
        if len(values) == 0:
            values = ["2", "2"]
        elif len(values) == 1:
            values.append("2")
        exprs = []
        for idx, value in enumerate(values):
            exprs.append(_compile_expr(
                value,
                label=f"legacy(inv_t_plus_2) coefficient {idx}",
                expected="complex",
            ))
        return exprs
    if spec["name"] in _VARIABLE_LEGACY_ARG_COUNTS:
        allowed = _VARIABLE_LEGACY_ARG_COUNTS[spec["name"]]
        if len(raw_args) not in allowed:
            counts = ", ".join(str(count) for count in sorted(allowed))
            raise RuntimeError(f"legacy({spec['name']}) expects {counts} arguments, got {len(raw_args)}")
        exprs = []
        for idx, value in enumerate(raw_args):
            exprs.append(_compile_expr(value, label=f"legacy({spec['name']}) arg {idx}", expected="real"))
        return exprs
    declared = list(spec["args"])
    if not declared:
        if raw_args:
            raise RuntimeError(f"legacy({spec['name']}) takes no arguments")
        return []
    if len(raw_args) > len(declared):
        raise RuntimeError(f"legacy({spec['name']}) got too many arguments")
    exprs = []
    for idx in range(max(len(raw_args), len(declared))):
        if idx < len(raw_args):
            raw = raw_args[idx]
        elif idx < len(declared):
            raw = declared[idx].get("default", 0.0)
        else:
            continue
        arg_type = str(declared[idx].get("type") or "real").strip().lower()
        expr = _compile_expr(raw, label=f"legacy({spec['name']}) arg {idx}", expected=arg_type)
        exprs.append(expr)
    if len(exprs) > MAX_ARGS:
        raise RuntimeError(f"legacy({spec['name']}) got too many arguments")
    return exprs


def _legacy_tokens(name, src, tgt, args):
    registry = legacy_registry()["by_name"]
    if name not in registry:
        raise RuntimeError(f"unknown legacy param transform: {name}")
    spec = registry[name]
    src_name, src_val = _selector_value(src, _SOURCE_SELECTORS, "legacy src")
    tgt_name, tgt_val = _selector_value(tgt, _TARGET_SELECTORS, "legacy tgt")
    if src_name not in spec["allowed_src"]:
        allowed = ", ".join(spec["allowed_src"])
        raise RuntimeError(f"legacy({name}) does not support src={src_name}; allowed: {allowed}")
    if tgt_name not in spec["allowed_tgt"]:
        allowed = ", ".join(spec["allowed_tgt"])
        raise RuntimeError(f"legacy({name}) does not support tgt={tgt_name}; allowed: {allowed}")
    exprs = _legacy_arg_exprs(spec, args)
    if any(expr.value is None for expr in exprs):
        tokens = []
        for expr in exprs:
            tokens.extend(_expr_to_param_tokens(expr))
        tokens.append(_token(
            PARAM_OP_LEGACY,
            fn_index=spec["fn_index"],
            src=src_val,
            tgt=tgt_val,
            n_args=len(exprs),
            stack_arg_count=len(exprs),
        ))
        return tokens
    values = [expr.value.real for expr in exprs]
    values_im = [expr.value.imag for expr in exprs]
    return [_token(
        PARAM_OP_LEGACY,
        fn_index=spec["fn_index"],
        src=src_val,
        tgt=tgt_val,
        n_args=len(values),
        args=values,
        args_im=values_im,
    )]


def _expand_macros(chain, macro_resolver, stack=None, depth=0):
    if stack is None:
        stack = []
    if depth > MAX_MACRO_DEPTH:
        raise RuntimeError(f"param program macro expansion exceeded depth {MAX_MACRO_DEPTH}")
    expanded = []
    expanded_count = 0
    for chip in chain:
        name, args = _chip_args(chip)
        if name != "macro":
            expanded.append(chip)
            continue
        if len(args) != 1:
            raise RuntimeError("macro chip requires exactly one name")
        macro_id = _slugify_macro_id(args[0])
        if macro_id in stack:
            cycle = " -> ".join(stack + [macro_id])
            raise RuntimeError(f"param program macro cycle: {cycle}")
        if macro_resolver is None:
            raise RuntimeError(f"param program macro resolver is required for macro({macro_id})")
        resolved = macro_resolver(macro_id)
        resolved_chain = _canonical_source_chain(resolved)
        nested, nested_count = _expand_macros(
            resolved_chain,
            macro_resolver,
            stack=stack + [macro_id],
            depth=depth + 1,
        )
        expanded.extend(nested)
        expanded_count += 1 + nested_count
    return expanded, expanded_count


def _lower_chip(chip):
    name, args = _chip_args(chip)
    if name == "macro":
        raise RuntimeError("macro chip survived expansion")
    if name == "push":
        if len(args) > 1:
            raise RuntimeError("push chip takes at most one source")
        return [_token(op) for op in _push_target_ops(args[0] if args else "both")]
    if name == "emit":
        if len(args) != 1:
            raise RuntimeError("emit chip requires target p1 or p2")
        return [_token(_emit_target_op(args[0]))]
    if name == "const":
        if len(args) not in {1, 2}:
            raise RuntimeError("const chip requires value, or legacy real value plus optional imaginary value")
        raw = args[0] if len(args) == 1 else f"({args[0]})+({args[1]})*1j"
        expr = _compile_expr(raw, label="const value", expected="complex")
        return _expr_to_param_tokens(expr)
    if name in _STACK_OPS:
        if args:
            raise RuntimeError(f"{name} chip takes no arguments")
        return [_token(_STACK_OPS[name])]
    if name in _BINARY_OPS:
        if args:
            raise RuntimeError(f"{name} chip takes no arguments")
        return [_token(_BINARY_OPS[name])]
    if name in _UNARY_OPS:
        if name in _TARGETABLE_UNARY_SOURCE and len(args) == 1:
            target = _normalize_target(args[0])
            op = _UNARY_OPS[name]
            if target == "p1":
                return [_token(PARAM_OP_PUSH_T1), _token(op), _token(PARAM_OP_EMIT_P1)]
            if target == "p2":
                return [_token(PARAM_OP_PUSH_T2), _token(op), _token(PARAM_OP_EMIT_P2)]
            return [
                _token(PARAM_OP_PUSH_T1),
                _token(op),
                _token(PARAM_OP_EMIT_P1),
                _token(PARAM_OP_PUSH_T2),
                _token(op),
                _token(PARAM_OP_EMIT_P2),
            ]
        if args:
            raise RuntimeError(f"{name} chip takes no arguments")
        return [_token(_UNARY_OPS[name])]
    if name == "legacy":
        if len(args) < 3:
            raise RuntimeError("legacy chip requires name, src, tgt, and optional args")
        legacy_name = str(args[0] or "").strip().lower()
        return _legacy_tokens(legacy_name, args[1], args[2], args[3:])
    if name in legacy_registry()["by_name"]:
        return _legacy_tokens(name, "both", "both", args)
    raise RuntimeError(f"unknown param program chip: {name}")


def _lower_chain(chain):
    tokens = []
    lowered_to_source = []
    for source_chip, chip in enumerate(chain):
        chip_tokens = _lower_chip(chip)
        tokens.extend(chip_tokens)
        lowered_to_source.extend(
            {"source_chip": source_chip, "source_arg": None, "source_arg_index": None}
            for _ in chip_tokens
        )
    if len(tokens) > MAX_PROGRAM_TOKENS:
        raise RuntimeError(
            f"param program has {len(tokens)} tokens after expansion; max is {MAX_PROGRAM_TOKENS}"
        )
    return tokens, [], lowered_to_source


def _validate_stack(tokens):
    depth = 0
    max_depth = 0
    emits = []
    for idx, token in enumerate(tokens):
        op = int(token.get("op") or 0)
        before = depth
        if op in {PARAM_OP_PUSH_T1, PARAM_OP_PUSH_T2, PARAM_OP_PUSH_P1, PARAM_OP_PUSH_P2, PARAM_OP_CONST}:
            depth += 1
        elif op == PARAM_OP_EMIT_P1:
            if depth < 1:
                raise RuntimeError(f"emit(p1) at token {idx}: stack depth is {before} (need >=1)")
            depth -= 1
            emits.append("p1")
        elif op == PARAM_OP_EMIT_P2:
            if depth < 1:
                raise RuntimeError(f"emit(p2) at token {idx}: stack depth is {before} (need >=1)")
            depth -= 1
            emits.append("p2")
        elif op == PARAM_OP_DUPLICATE:
            if depth < 1:
                raise RuntimeError(f"duplicate at token {idx}: stack depth is {before} (need >=1)")
            depth += 1
        elif op == PARAM_OP_SWAP:
            if depth < 2:
                raise RuntimeError(f"swap at token {idx}: stack depth is {before} (need >=2)")
        elif op == PARAM_OP_POP:
            if depth < 1:
                raise RuntimeError(f"pop at token {idx}: stack depth is {before} (need >=1)")
            depth -= 1
        elif op == PARAM_OP_FLUSH:
            depth = 0
        elif op in {PARAM_OP_ADD, PARAM_OP_SUBTRACT, PARAM_OP_MUL, PARAM_OP_RATIO, PARAM_OP_DIVIDE}:
            if depth < 2:
                raise RuntimeError(f"{_OP_NAMES[op]} at token {idx}: stack depth is {before} (need >=2)")
            depth -= 1
        elif op in {
            PARAM_OP_NEGATE,
            PARAM_OP_CONJ,
            PARAM_OP_RECIPROCAL,
            PARAM_OP_UNIT_CIRCLE,
            PARAM_OP_SQUARE,
            PARAM_OP_CUBE,
            PARAM_OP_EXP,
            PARAM_OP_REAL,
            PARAM_OP_IMAG,
            PARAM_OP_ABS,
        }:
            if depth < 1:
                raise RuntimeError(f"{_OP_NAMES[op]} at token {idx}: stack depth is {before} (need >=1)")
        elif op == PARAM_OP_LEGACY:
            src = int(token.get("src") or 0)
            tgt = int(token.get("tgt") or 0)
            stack_arg_count = int(token.get("stack_arg_count") or 0)
            n_args = int(token.get("n_args") or 0)
            if stack_arg_count < 0 or stack_arg_count > MAX_ARGS or stack_arg_count > n_args:
                raise RuntimeError(f"legacy at token {idx}: invalid stack_arg_count={stack_arg_count}")
            if stack_arg_count:
                if depth < stack_arg_count:
                    raise RuntimeError(
                        f"legacy at token {idx}: stack depth is {before} (need >={stack_arg_count} for args)"
                    )
                depth -= stack_arg_count
            if src == PARAM_SEL_POP1:
                if depth < 1:
                    raise RuntimeError(f"legacy at token {idx}: stack depth is {before} (need >=1)")
                depth -= 1
            elif src == PARAM_SEL_POP2:
                if depth < 2:
                    raise RuntimeError(f"legacy at token {idx}: stack depth is {before} (need >=2)")
                depth -= 2
            if tgt == PARAM_SEL_PUSH1:
                depth += 1
            elif tgt == PARAM_SEL_PUSH2:
                depth += 2
        else:
            raise RuntimeError(f"unknown param program opcode at token {idx}: {op}")
        if depth > MAX_STACK:
            raise RuntimeError(f"param program stack depth {depth} exceeds max {MAX_STACK} at token {idx}")
        max_depth = max(max_depth, depth)
    if depth != 0:
        raise RuntimeError(f"param program final stack depth is {depth}; expected 0")
    return {"stack_max": max_depth, "emits": emits}


def _execution_spec(tokens, scalar_exprs):
    del scalar_exprs
    return json.dumps(
        {"tokens": tokens},
        sort_keys=True,
        separators=(",", ":"),
    )


def _fingerprint(spec):
    payload = json.dumps(
        {"version": PROGRAM_VERSION, "execution_spec": spec},
        sort_keys=True,
        separators=(",", ":"),
    )
    return hashlib.sha1(payload.encode("utf-8")).hexdigest()


def _legacy_fast_path(tokens):
    if not tokens:
        return True
    for token in tokens:
        if int(token.get("op") or 0) != PARAM_OP_LEGACY:
            return False
        if int(token.get("stack_arg_count") or 0) != 0:
            return False
        if int(token.get("src") or 0) != PARAM_SEL_BOTH:
            return False
        if int(token.get("tgt") or 0) != PARAM_SEL_BOTH:
            return False
    return True


def _legacy_transforms(tokens):
    if not _legacy_fast_path(tokens):
        return []
    out = []
    for token in tokens:
        spec = legacy_registry()["by_index"].get(int(token.get("fn_index") or 0))
        if not spec:
            return []
        entry = [spec["name"]]
        token_args = list(token.get("args") or [])
        token_args_im = list(token.get("args_im") or [])
        if spec["name"] == "moebius" and len(token_args) == 4:
            args = [
                _format_complex_number(token_args[idx], token_args_im[idx] if idx < len(token_args_im) else 0.0)
                for idx in range(4)
            ]
        elif spec["name"] == "moebius" and len(token_args) == 8:
            args = [
                _format_complex_number(token_args[0], token_args[1]),
                _format_complex_number(token_args[2], token_args[3]),
                _format_complex_number(token_args[4], token_args[5]),
                _format_complex_number(token_args[6], token_args[7]),
            ]
        elif spec["name"] == "inv_t_plus_2" and len(token_args) == 2:
            args = [
                _format_number(token_args[0]),
                _format_number(token_args_im[0] if len(token_args_im) > 0 else 0.0),
                _format_number(token_args[1]),
                _format_number(token_args_im[1] if len(token_args_im) > 1 else 0.0),
            ]
        else:
            args = [_format_number(arg) for arg in token_args]
        target_idx = _LEGACY_TARGET_ARG_INDEXES.get(spec["name"])
        if target_idx is not None:
            target = _target_value_for_selector(_SELECTOR_NAMES.get(int(token.get("tgt") or 0), "both"))
            if spec["name"] != "rtheta" or target != "2":
                args = args[:target_idx] + [target] + args[target_idx:]
        entry.extend(args)
        out.append(entry)
    return out


def compile_param_program_chain(chain, *, macro_resolver=None, strict=True):
    diagnostics = []
    try:
        source_chain = _canonical_source_chain(chain)
        expanded_chain, macro_count = _expand_macros(source_chain, macro_resolver)
        tokens, scalar_exprs, lowered_to_source = _lower_chain(expanded_chain)
        stack_info = _validate_stack(tokens)
        spec = _execution_spec(tokens, scalar_exprs)
        fast_path = _legacy_fast_path(tokens)
        return {
            "version": PROGRAM_VERSION,
            "program_kind": PROGRAM_KIND,
            "source_chain": source_chain,
            "expanded_chain": expanded_chain,
            "tokens": tokens,
            "scalar_exprs": scalar_exprs,
            "execution_format": "single_vm",
            "execution_spec": spec,
            "fingerprint": _fingerprint(spec),
            "display": display_param_program_chain(source_chain),
            "expanded_display": display_param_program_chain(expanded_chain),
            "statement_count": len(source_chain),
            "token_count": len(tokens),
            "stack_max": stack_info["stack_max"],
            "emits": stack_info["emits"],
            "macro_expansions": macro_count,
            "lowered_to_source": lowered_to_source,
            "uses_legacy_fast_path": fast_path,
            "legacy_transforms": _legacy_transforms(tokens),
            "diagnostics": diagnostics,
        }
    except Exception as exc:
        diagnostics.append({"level": "error", "message": str(exc)})
        if strict:
            raise
        return {
            "version": PROGRAM_VERSION,
            "program_kind": PROGRAM_KIND,
            "source_chain": chain if isinstance(chain, list) else [],
            "expanded_chain": [],
            "tokens": [],
            "scalar_exprs": [],
            "execution_format": "single_vm",
            "execution_spec": "",
            "fingerprint": "",
            "display": "",
            "expanded_display": "",
            "statement_count": 0,
            "token_count": 0,
            "stack_max": 0,
            "emits": [],
            "macro_expansions": 0,
            "lowered_to_source": [],
            "uses_legacy_fast_path": False,
            "legacy_transforms": [],
            "diagnostics": diagnostics,
        }


def compile_param_program_diagnostics(chain, *, macro_resolver=None):
    return compile_param_program_chain(chain, macro_resolver=macro_resolver, strict=False)
