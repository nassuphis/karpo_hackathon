"""
Coeff-program compiler helpers.

The compiler owns source validation, scalar-expression parsing, macro expansion,
stack effects, canonical tokens, and fingerprints. Native coeffgen receives only
integer opcodes, selectors, numeric args, and scalar-expression bytecode.
"""
from __future__ import annotations

import hashlib
import cmath
import json
import math
import os
import re


PROGRAM_KIND = "coeff_program"
PROGRAM_VERSION = 1
MAX_PROGRAM_TOKENS = 256
MAX_VECTOR_STACK = 64
MAX_VECTOR_LEN = 256
POLY_LEN_SENTINEL = -1
MAX_MACRO_DEPTH = 8
MAX_ARGS = 8
MAX_SCALAR_EXPR_TOKENS = 32

COEFF_OP_CONST = 1
COEFF_OP_PUSH = 2
COEFF_OP_EMIT = 3
COEFF_OP_DUPLICATE = 4
COEFF_OP_SWAP = 5
COEFF_OP_POP = 6
COEFF_OP_FLUSH = 7
COEFF_OP_BLEND = 8
COEFF_OP_LEGACY = 9
COEFF_OP_POKE_POLY = 10
COEFF_OP_POKE_TOS = 11
COEFF_OP_VECTOR_BINARY = 12
COEFF_OP_VECTOR_UNARY = 13
COEFF_OP_VECTOR_ROLL = 14
COEFF_OP_VECTOR_ARGSORT = 15
COEFF_OP_LITTLEWOOD = 16
COEFF_OP_LINSPACE = 17
COEFF_OP_RANGE = 18
COEFF_OP_SET = 19
COEFF_OP_AFFINE = 20
COEFF_OP_TYPED_PUSH_SCALAR = 21
COEFF_OP_TYPED_PUSH_VECTOR = 22
COEFF_OP_TYPED_BINARY = 23
COEFF_OP_TYPED_UNARY = 24
COEFF_OP_TYPED_GET_SCALAR = 25
COEFF_OP_TYPED_SET_POLY = 26
COEFF_OP_TYPED_POKE_POLY = 27
COEFF_OP_TYPED_FILL = 28
COEFF_OP_NATIVE_TRANSFORM = 29
COEFF_OP_TYPED_BLEND = 30

COEFF_SEL_CF = 1
COEFF_SEL_POLY = 2
COEFF_SEL_POP = 3
COEFF_SEL_PEEK = 4
COEFF_SEL_PUSH = 5

COEFF_SCALAR_SRC_P1 = 1
COEFF_SCALAR_SRC_P2 = 2
COEFF_SCALAR_SRC_T1 = 3
COEFF_SCALAR_SRC_T2 = 4
COEFF_SCALAR_SRC_POLY_LEN = 5

EXPR_LITERAL = 1
EXPR_P1 = 2
EXPR_P2 = 3
EXPR_ADD = 4
EXPR_SUB = 5
EXPR_MUL = 6
EXPR_DIV = 7
EXPR_CONJ = 8
EXPR_NEG = 9
EXPR_REAL = 10
EXPR_IMAG = 11
EXPR_POLY_LEN = 12
EXPR_CF_AT = 13
EXPR_POLY_AT = 14
EXPR_TOS_AT = 15
EXPR_T1 = 16
EXPR_T2 = 17
EXPR_ABS = 18
EXPR_LOG = 19
EXPR_CF_AT_DYN = 20
EXPR_POLY_AT_DYN = 21
EXPR_TOS_AT_DYN = 22

_OP_NAMES = {
    COEFF_OP_CONST: "push_const",
    COEFF_OP_PUSH: "push",
    COEFF_OP_EMIT: "emit",
    COEFF_OP_DUPLICATE: "duplicate",
    COEFF_OP_SWAP: "swap",
    COEFF_OP_POP: "pop",
    COEFF_OP_FLUSH: "flush",
    COEFF_OP_BLEND: "blend",
    COEFF_OP_LEGACY: "legacy",
    COEFF_OP_POKE_POLY: "poke_poly",
    COEFF_OP_POKE_TOS: "poke_tos",
    COEFF_OP_VECTOR_BINARY: "vector_binary",
    COEFF_OP_VECTOR_UNARY: "vector_unary",
    COEFF_OP_VECTOR_ROLL: "vector_roll",
    COEFF_OP_VECTOR_ARGSORT: "argsort",
    COEFF_OP_LITTLEWOOD: "littlewood",
    COEFF_OP_LINSPACE: "push_linspace",
    COEFF_OP_RANGE: "push_range",
    COEFF_OP_SET: "set",
    COEFF_OP_AFFINE: "affine",
    COEFF_OP_TYPED_PUSH_SCALAR: "typed_push_scalar",
    COEFF_OP_TYPED_PUSH_VECTOR: "typed_push_vector",
    COEFF_OP_TYPED_BINARY: "typed_binary",
    COEFF_OP_TYPED_UNARY: "typed_unary",
    COEFF_OP_TYPED_GET_SCALAR: "typed_get_scalar",
    COEFF_OP_TYPED_SET_POLY: "typed_set_poly",
    COEFF_OP_TYPED_POKE_POLY: "typed_poke_poly",
    COEFF_OP_TYPED_FILL: "typed_fill",
    COEFF_OP_NATIVE_TRANSFORM: "native_transform",
    COEFF_OP_TYPED_BLEND: "typed_blend",
}

_SOURCE_SELECTORS = {
    "cf": COEFF_SEL_CF,
    "poly": COEFF_SEL_POLY,
    "pop": COEFF_SEL_POP,
    "peek": COEFF_SEL_PEEK,
}

_TARGET_SELECTORS = {
    "poly": COEFF_SEL_POLY,
    "push": COEFF_SEL_PUSH,
}

_VECTOR_SOURCE_SELECTORS = {
    "poly": COEFF_SEL_POLY,
    "pop": COEFF_SEL_POP,
    "peek": COEFF_SEL_PEEK,
}

_SELECTOR_NAMES = {
    COEFF_SEL_CF: "cf",
    COEFF_SEL_POLY: "poly",
    COEFF_SEL_POP: "pop",
    COEFF_SEL_PEEK: "peek",
    COEFF_SEL_PUSH: "push",
}

_STACK_OPS = {
    "duplicate": COEFF_OP_DUPLICATE,
    "dup": COEFF_OP_DUPLICATE,
    "swap": COEFF_OP_SWAP,
    "pop": COEFF_OP_POP,
    "flush": COEFF_OP_FLUSH,
}

_ENUM_ARG_VALUES = {
    "hi": 0.0,
    "lo": 1.0,
}

_LEGACY_NAME_ALIASES = {
    "scale100": "linear",
}

_VECTOR_BINARY_OPS = {
    "add": 1,
    "subtract": 2,
    "multiply": 3,
    "divide": 4,
    "power": 5,
}

_VECTOR_UNARY_OPS = {
    "angle": 1,
    "mod": 2,
    "abs": 3,
    "neg": 4,
    "conj": 5,
    "sqrt": 6,
    "log": 7,
    "real": 8,
    "imag": 9,
}

_VECTOR_ROLL_OPS = {
    "roll": 1,
    "rolr": 2,
}

_VECTOR_BINARY_NAMES = {v: k for k, v in _VECTOR_BINARY_OPS.items()}
_VECTOR_UNARY_NAMES = {v: k for k, v in _VECTOR_UNARY_OPS.items()}
_VECTOR_ROLL_NAMES = {v: k for k, v in _VECTOR_ROLL_OPS.items()}


class _Expr:
    __slots__ = ("tokens", "kind", "dynamic")

    def __init__(self, tokens, *, kind="complex", dynamic=True):
        self.tokens = tokens
        self.kind = kind
        self.dynamic = dynamic


def _registry_path():
    return os.path.join(os.path.dirname(__file__), "coeff_legacy_registry.json")


def _load_legacy_registry():
    with open(_registry_path(), "r", encoding="utf-8") as fh:
        payload = json.load(fh)
    if int(payload.get("version") or 0) != 1:
        raise RuntimeError("coeff legacy registry version must be 1")
    by_name = {}
    by_index = {}
    for fn in payload.get("functions") or []:
        name = str(fn.get("name") or "").strip().lower()
        if not name:
            raise RuntimeError("coeff legacy registry function missing name")
        if name in by_name:
            raise RuntimeError(f"duplicate coeff legacy function name: {name}")
        fn_index = int(fn.get("fn_index") or 0)
        if fn_index <= 0:
            raise RuntimeError(f"coeff legacy function {name} fn_index must be positive")
        if fn_index in by_index:
            raise RuntimeError(f"duplicate coeff legacy fn_index: {fn_index}")
        args_raw = list(fn.get("args") or [])
        if len(args_raw) > MAX_ARGS:
            raise RuntimeError(f"coeff legacy function {name} has too many args")
        args = []
        for idx, arg in enumerate(args_raw):
            if not isinstance(arg, dict):
                raise RuntimeError(f"coeff legacy function {name} arg {idx} must be an object")
            normalized_arg = dict(arg)
            arg_type = str(normalized_arg.get("type") or "real").strip().lower()
            if arg_type not in {"real", "int", "enum"}:
                raise RuntimeError(
                    f"coeff legacy function {name} arg {idx} has unsupported type {arg_type!r}; "
                    "v1 legacy bridge supports real, int, and enum args only"
                )
            normalized_arg["type"] = arg_type
            args.append(normalized_arg)
        spec = {
            "name": name,
            "fn_index": fn_index,
            "category": str(fn.get("category") or "").strip(),
            "allowed_src": tuple(str(x).strip() for x in (fn.get("allowed_src") or [])),
            "allowed_tgt": tuple(str(x).strip() for x in (fn.get("allowed_tgt") or [])),
            "args": tuple(args),
            "supports_andy": bool(fn.get("supports_andy")),
            "length_policy": str(fn.get("length_policy") or "unknown"),
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


def _canonical_legacy_name(name):
    raw = str(name or "").strip().lower()
    return _LEGACY_NAME_ALIASES.get(raw, raw)


def validate_legacy_registry():
    registry = legacy_registry()
    return {
        "version": 1,
        "count": len(registry["by_name"]),
        "names": sorted(registry["by_name"]),
        "fn_indices": sorted(registry["by_index"]),
    }


_COMPLEX_TERM_RE = re.compile(r"[+-]?(?:(?:\d+(?:\.\d*)?)|(?:\.\d+))(?:[eE][+-]?\d+)?")


def _parse_complex_literal(value):
    raw = str("" if value is None else value).strip().replace("i", "j").replace("I", "j").replace(" ", "")
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


def _finite_number(value, label):
    try:
        number = float(value)
    except (TypeError, ValueError):
        raise RuntimeError(f"{label} must be numeric, got {value!r}")
    if not math.isfinite(number):
        raise RuntimeError(f"{label} must be finite, got {value!r}")
    return number


def _integer_literal(value, label):
    number = _finite_number(value, label)
    if not number.is_integer():
        raise RuntimeError(f"{label} must be an integer, got {value!r}")
    return int(number)


def _vector_length_arg(value, label):
    raw = str(value).strip().lower()
    if raw == "poly_len":
        return POLY_LEN_SENTINEL
    length = _integer_literal(value, label)
    if length < 1 or length > MAX_VECTOR_LEN:
        raise RuntimeError(f"{label} must be in [1,{MAX_VECTOR_LEN}] or poly_len, got {value!r}")
    return length


def _format_number(value):
    number = _finite_number(value, "number")
    if number == 0:
        number = 0.0
    return format(number, ".17g")


def _format_complex_number(real, imag):
    real = _finite_number(real, "complex real")
    imag = _finite_number(imag, "complex imag")
    if real == 0:
        real = 0.0
    if imag == 0:
        imag = 0.0
    real_text = _format_number(real)
    imag_text = _format_number(abs(imag))
    if imag == 0:
        return real_text
    if real == 0:
        return f"{'-' if imag < 0 else ''}{imag_text}i"
    return f"{real_text}{'-' if imag < 0 else '+'}{imag_text}i"


def _format_length_arg(value):
    number = int(value)
    return "poly_len" if number == POLY_LEN_SENTINEL else _format_number(number)


def _format_length_or_number(value):
    number = _finite_number(value, "length")
    if number == POLY_LEN_SENTINEL or float(number).is_integer():
        return _format_length_arg(number)
    return _format_number(number)


def _chip_name(value, idx):
    name = str(value or "").strip()
    if not name:
        raise RuntimeError(f"coeff program chip {idx} has empty name")
    return name.lower()


def _chip_args(chip):
    if isinstance(chip, str):
        return _chip_name(chip, "?"), []
    if isinstance(chip, (list, tuple)):
        if not chip:
            raise RuntimeError("coeff program chip cannot be an empty array")
        return _chip_name(chip[0], "?"), list(chip[1:])
    raise RuntimeError(f"coeff program chip must be a string or array, got {chip!r}")


def _canonical_source_chain(chain):
    if not isinstance(chain, list):
        raise RuntimeError("coeff program chain must be a JSON array")
    out = []
    for idx, chip in enumerate(chain):
        if isinstance(chip, str):
            out.append(_chip_name(chip, idx))
            continue
        if not isinstance(chip, list) or not chip:
            raise RuntimeError(f"coeff program chip {idx} must be a non-empty array or string")
        name = _chip_name(chip[0], idx)
        entry = [name]
        for arg_idx, arg in enumerate(chip[1:]):
            if isinstance(arg, str):
                if len(arg) > 256:
                    raise RuntimeError(f"coeff program chip {idx} arg {arg_idx} is too long")
                entry.append(arg.strip())
            elif isinstance(arg, (int, float)):
                entry.append(_format_number(arg))
            else:
                raise RuntimeError(
                    f"coeff program chip {idx} arg {arg_idx} must be a string or number"
                )
        out.append(entry)
    return out


def _slugify_macro_id(value):
    text = str(value or "").strip()
    if not text:
        raise RuntimeError("macro name is required")
    return re.sub(r"[^a-zA-Z0-9._-]+", "-", text).strip("-") or text


_EXPR_TOKEN_RE = re.compile(
    r"\s*(?:(?P<number>(?:(?:\d+(?:\.\d*)?)|(?:\.\d+))(?:[eE][+-]?\d+)?)(?P<imag>[ijIJ])?|(?P<ident>[A-Za-z_][A-Za-z0-9_]*)|(?P<op>\*\*|[()[\]+\-*/]))"
)

_EXPR_CONSTANTS = {
    "pi": complex(math.pi, 0.0),
    "pi2": complex(2.0 * math.pi, 0.0),
    "pi2i": complex(0.0, 2.0 * math.pi),
    "tau": complex(2.0 * math.pi, 0.0),
    "tau_i": complex(0.0, 2.0 * math.pi),
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
                raise RuntimeError(f"invalid scalar expression near {text[pos:]!r}")
            pos = match.end()
            if match.group("number") is not None:
                number = float(match.group("number"))
                if not math.isfinite(number):
                    raise RuntimeError("scalar expression number must be finite")
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
            raise RuntimeError("scalar expression is empty")
        expr = self._expr()
        if self.pos != len(self.tokens):
            raise RuntimeError(f"unexpected scalar expression token {self._peek()[1]!r}")
        if len(expr.tokens) > MAX_SCALAR_EXPR_TOKENS:
            raise RuntimeError(
                f"scalar expression has {len(expr.tokens)} tokens; max is {MAX_SCALAR_EXPR_TOKENS}"
            )
        return expr

    def _expr(self):
        left = self._term()
        while self._peek()[0] in {"+", "-"}:
            op = self._take()[0]
            right = self._term()
            left = _Expr(
                left.tokens + right.tokens + [{"op": EXPR_ADD if op == "+" else EXPR_SUB}],
                kind="complex" if left.kind == "complex" or right.kind == "complex" else "real",
                dynamic=left.dynamic or right.dynamic,
            )
        return left

    def _term(self):
        left = self._power()
        while self._peek()[0] in {"*", "/"}:
            op = self._take()[0]
            right = self._power()
            left = _Expr(
                left.tokens + right.tokens + [{"op": EXPR_MUL if op == "*" else EXPR_DIV}],
                kind="complex" if left.kind == "complex" or right.kind == "complex" else "real",
                dynamic=left.dynamic or right.dynamic,
            )
        return left

    def _unary(self):
        token_type, token_value = self._peek()
        if token_type == "+":
            self._take()
            return self._unary()
        if token_type == "-":
            self._take()
            expr = self._unary()
            return _Expr(expr.tokens + [{"op": EXPR_NEG}], kind=expr.kind, dynamic=expr.dynamic)
        if token_type == "ident" and token_value in {"conj", "neg", "real", "imag", "abs", "mod", "log"}:
            self._take()
            if self._take()[0] != "(":
                raise RuntimeError(f"{token_value} requires parentheses")
            expr = self._expr()
            if self._take()[0] != ")":
                raise RuntimeError(f"{token_value} missing closing parenthesis")
            op = {
                "conj": EXPR_CONJ,
                "neg": EXPR_NEG,
                "real": EXPR_REAL,
                "imag": EXPR_IMAG,
                "abs": EXPR_ABS,
                "mod": EXPR_ABS,
                "log": EXPR_LOG,
            }[token_value]
            return _Expr(
                expr.tokens + [{"op": op}],
                kind="real" if token_value in {"real", "imag", "abs", "mod"} else expr.kind,
                dynamic=expr.dynamic,
            )
        return self._primary()

    def _power(self):
        left = self._unary()
        if self._peek()[0] != "**":
            return left
        self._take()
        right = self._unary()
        value = _expr_value_if_static(right)
        if value is None or abs(value.imag) > 1e-12 or not float(value.real).is_integer():
            raise RuntimeError("scalar expression ** exponent must be an integer literal")
        exponent = int(value.real)
        if abs(exponent) > 32:
            raise RuntimeError("scalar expression ** exponent magnitude must be <= 32")
        if exponent == 0:
            return _Expr([{"op": EXPR_LITERAL, "a": 1.0, "b": 0.0}], kind="real", dynamic=False)
        tokens = list(left.tokens)
        for _ in range(abs(exponent) - 1):
            tokens.extend(left.tokens)
            tokens.append({"op": EXPR_MUL})
        if exponent < 0:
            tokens = [{"op": EXPR_LITERAL, "a": 1.0, "b": 0.0}] + tokens + [{"op": EXPR_DIV}]
        return _Expr(tokens, kind=left.kind, dynamic=left.dynamic)

    def _indexed_reference(self, name):
        if self._take()[0] != "[":
            raise RuntimeError(f"{name}[...] missing opening bracket")
        index_expr = self._expr()
        if self._take()[0] != "]":
            raise RuntimeError(f"{name}[...] missing closing bracket")
        value = _expr_value_if_static(index_expr)
        if value is not None:
            if abs(value.imag) > 1e-12 or not value.real.is_integer():
                raise RuntimeError(f"{name}[index] requires an integer index")
            index = int(value.real)
            if index < 0 or index >= MAX_VECTOR_LEN:
                raise RuntimeError(f"{name} index must be in [0,{MAX_VECTOR_LEN - 1}], got {index}")
            op = {"cf": EXPR_CF_AT, "poly": EXPR_POLY_AT, "tos": EXPR_TOS_AT}[name]
            return _Expr([{"op": op, "a": index}], kind="complex", dynamic=True)
        op = {"cf": EXPR_CF_AT_DYN, "poly": EXPR_POLY_AT_DYN, "tos": EXPR_TOS_AT_DYN}[name]
        return _Expr(index_expr.tokens + [{"op": op}], kind="complex", dynamic=True)

    def _primary(self):
        token_type, token_value = self._take()
        if token_type == "number":
            return _Expr(
                [{"op": EXPR_LITERAL, "a": token_value.real, "b": token_value.imag}],
                kind="complex" if token_value.imag else "real",
                dynamic=False,
            )
        if token_type == "ident":
            if token_value in {"i", "j"}:
                return _Expr(
                    [{"op": EXPR_LITERAL, "a": 0.0, "b": 1.0}],
                    kind="complex",
                    dynamic=False,
                )
            if token_value in _EXPR_CONSTANTS:
                value = _EXPR_CONSTANTS[token_value]
                return _Expr(
                    [{"op": EXPR_LITERAL, "a": value.real, "b": value.imag}],
                    kind="complex" if value.imag else "real",
                    dynamic=False,
                )
            if token_value == "p1":
                return _Expr([{"op": EXPR_P1}], kind="complex", dynamic=True)
            if token_value == "p2":
                return _Expr([{"op": EXPR_P2}], kind="complex", dynamic=True)
            if token_value == "t1":
                return _Expr([{"op": EXPR_T1}], kind="complex", dynamic=True)
            if token_value == "t2":
                return _Expr([{"op": EXPR_T2}], kind="complex", dynamic=True)
            if token_value == "poly_len":
                return _Expr([{"op": EXPR_POLY_LEN}], kind="real", dynamic=True)
            if token_value in {"cf", "poly", "tos"} and self._peek()[0] == "[":
                return self._indexed_reference(token_value)
            match = re.fullmatch(r"(cf|poly|tos)(\d+)", token_value)
            if match:
                index = int(match.group(2))
                if index < 0 or index >= MAX_VECTOR_LEN:
                    raise RuntimeError(
                        f"{match.group(1)} index must be in [0,{MAX_VECTOR_LEN - 1}], got {index}"
                    )
                op = {"cf": EXPR_CF_AT, "poly": EXPR_POLY_AT, "tos": EXPR_TOS_AT}[match.group(1)]
                return _Expr([{"op": op, "a": index}], kind="complex", dynamic=True)
            raise RuntimeError(f"unknown scalar expression identifier {token_value!r}")
        if token_type == "(":
            expr = self._expr()
            if self._take()[0] != ")":
                raise RuntimeError("scalar expression missing closing parenthesis")
            return expr
        raise RuntimeError(f"unexpected scalar expression token {token_value!r}")


def _compile_expr(value, *, label, expected="complex"):
    try:
        literal = _parse_complex_literal(value)
        expr = _Expr(
            [{"op": EXPR_LITERAL, "a": literal.real, "b": literal.imag}],
            kind="complex" if literal.imag else "real",
            dynamic=False,
        )
    except ValueError:
        expr = _ExpressionParser(value).parse()
    if expected == "real" and expr.kind != "real":
        raise RuntimeError(f"{label} must be real-valued; use real(...) or imag(...) explicitly")
    return expr


def _expr_value_if_static(expr):
    if expr.dynamic:
        return None
    stack = []
    for token in expr.tokens:
        op = int(token["op"])
        if op == EXPR_LITERAL:
            stack.append(complex(float(token.get("a") or 0.0), float(token.get("b") or 0.0)))
        elif op == EXPR_ADD:
            b = stack.pop(); a = stack.pop(); stack.append(a + b)
        elif op == EXPR_SUB:
            b = stack.pop(); a = stack.pop(); stack.append(a - b)
        elif op == EXPR_MUL:
            b = stack.pop(); a = stack.pop(); stack.append(a * b)
        elif op == EXPR_DIV:
            b = stack.pop(); a = stack.pop()
            if abs(b) <= 1e-300:
                raise RuntimeError("scalar expression division by zero")
            stack.append(a / b)
        elif op == EXPR_CONJ:
            stack.append(stack.pop().conjugate())
        elif op == EXPR_NEG:
            stack.append(-stack.pop())
        elif op == EXPR_REAL:
            stack.append(complex(stack.pop().real, 0.0))
        elif op == EXPR_IMAG:
            stack.append(complex(stack.pop().imag, 0.0))
        elif op == EXPR_ABS:
            stack.append(complex(abs(stack.pop()), 0.0))
        elif op == EXPR_LOG:
            value = stack.pop()
            if abs(value) <= 0.0:
                stack.append(complex(-700.0, 0.0))
            else:
                stack.append(cmath.log(value))
        else:
            raise RuntimeError(f"non-static scalar expression opcode: {op}")
    if len(stack) != 1:
        raise RuntimeError("scalar expression stack did not finish with one value")
    value = stack[0]
    if not math.isfinite(value.real) or not math.isfinite(value.imag):
        raise RuntimeError("scalar expression result must be finite")
    return value


def _flatten_expr(expr):
    flat = []
    for token in expr.tokens:
        flat.extend([
            float(token.get("op") or 0),
            float(token.get("a", 0.0) or 0.0),
            float(token.get("b", 0.0) or 0.0),
        ])
    return flat


def _display_chip(chip):
    if isinstance(chip, str):
        return chip
    if len(chip) == 1:
        return chip[0]
    return f"{chip[0]}(" + ", ".join(str(arg) for arg in chip[1:]) + ")"


def display_coeff_program_chain(chain):
    return "; ".join(_display_chip(chip) for chip in chain)


def _token(op, **fields):
    tok = {"op": int(op)}
    for key in ("fn_index", "src", "tgt", "n_args", "stack_arg_count"):
        value = fields.get(key)
        if value not in (None, 0):
            tok[key] = int(value)
    if "args" in fields:
        tok["args"] = [_finite_number(x, "token arg") for x in (fields.get("args") or [])]
    if "args_im" in fields:
        tok["args_im"] = [_finite_number(x, "token arg imag") for x in (fields.get("args_im") or [])]
    if "expr_refs" in fields:
        tok["expr_refs"] = [int(x) for x in (fields.get("expr_refs") or [])]
    if "andy" in fields:
        tok["andy"] = _finite_number(fields["andy"], "token andy")
    if "andy_expr_ref" in fields and int(fields["andy_expr_ref"]) >= 0:
        tok["andy_expr_ref"] = int(fields["andy_expr_ref"])
    return tok


def _add_arg_expr(expr, scalar_exprs, *, expected="complex"):
    value = _expr_value_if_static(expr)
    if value is not None:
        if expected == "real":
            if abs(value.imag) > 1e-12:
                raise RuntimeError("scalar expression result must be real-valued")
            return value.real, 0.0, -1
        return value.real, value.imag, -1
    ref = len(scalar_exprs)
    scalar_exprs.append(_flatten_expr(expr))
    return 0.0, 0.0, ref


def _compile_andy(raw, scalar_exprs, label):
    expr = _compile_expr(raw, label=label, expected="real")
    andy, _imag, andy_expr_ref = _add_arg_expr(expr, scalar_exprs, expected="real")
    return andy, andy_expr_ref


def _compile_complex_components(raw, scalar_exprs, label):
    expr = _compile_expr(raw, label=label, expected="complex")
    value = _expr_value_if_static(expr)
    if value is not None:
        return value.real, value.imag, -1, -1
    re_expr = _compile_expr(f"real({raw})", label=f"{label} real component", expected="real")
    im_expr = _compile_expr(f"imag({raw})", label=f"{label} imaginary component", expected="real")
    re, _re_imag, re_ref = _add_arg_expr(re_expr, scalar_exprs, expected="real")
    im, _im_imag, im_ref = _add_arg_expr(im_expr, scalar_exprs, expected="real")
    return re, im, re_ref, im_ref


def _selector_value(value, mapping, label):
    raw = str(value or "").strip().lower()
    if raw not in mapping:
        raise RuntimeError(f"{label} selector is invalid: {value!r}")
    return raw, mapping[raw]


def _compile_const(args, scalar_exprs):
    if len(args) != 2:
        raise RuntimeError("push_const chip requires length and value")
    length = _vector_length_arg(args[0], "push_const length")
    expr = _compile_expr(args[1], label="push_const value", expected="complex")
    re, im, ref = _add_arg_expr(expr, scalar_exprs, expected="complex")
    return _token(COEFF_OP_CONST, n_args=2, args=[length, re], args_im=[0.0, im], expr_refs=[-1, ref])


def _compile_real_arg(raw, scalar_exprs, label):
    expr = _compile_expr(raw, label=label, expected="real")
    value, _imag, ref = _add_arg_expr(expr, scalar_exprs, expected="real")
    return value, ref


def _compile_real_args(args, scalar_exprs, label):
    values = []
    refs = []
    for idx, raw in enumerate(args):
        value, ref = _compile_real_arg(raw, scalar_exprs, f"{label} arg {idx}")
        values.append(value)
        refs.append(ref)
    return values, refs


def _compile_linspace(args, scalar_exprs):
    if len(args) == 1:
        try:
            length = _vector_length_arg(args[0], "push_linspace length")
            return _token(COEFF_OP_LINSPACE, n_args=1, args=[length], expr_refs=[-1])
        except RuntimeError:
            values, refs = _compile_real_args(args, scalar_exprs, "push_linspace")
            return _token(COEFF_OP_LINSPACE, n_args=1, args=values, expr_refs=refs)
    if len(args) != 3:
        raise RuntimeError("push_linspace chip requires length or start, stop, count")
    values, refs = _compile_real_args(args, scalar_exprs, "push_linspace")
    return _token(COEFF_OP_LINSPACE, n_args=3, args=values, expr_refs=refs)


def _compile_range(args, scalar_exprs):
    if len(args) == 1:
        try:
            length = _vector_length_arg(args[0], "push_range length")
            return _token(COEFF_OP_RANGE, n_args=1, args=[length], expr_refs=[-1])
        except RuntimeError:
            values, refs = _compile_real_args(args, scalar_exprs, "push_range")
            return _token(COEFF_OP_RANGE, n_args=1, args=values, expr_refs=refs)
    if len(args) not in {2, 3}:
        raise RuntimeError("push_range chip requires length, start/stop, or start/stop/step")
    values, refs = _compile_real_args(args, scalar_exprs, "push_range")
    return _token(COEFF_OP_RANGE, n_args=len(values), args=values, expr_refs=refs)


def _compile_set(args):
    if len(args) != 2:
        raise RuntimeError("set chip requires tgt and src")
    tgt_name, tgt_val = _selector_value(args[0], {"poly": COEFF_SEL_POLY}, "set tgt")
    _src_name, src_val = _selector_value(args[1], _SOURCE_SELECTORS, "set src")
    return _token(COEFF_OP_SET, src=src_val, tgt=tgt_val)


def _compile_affine(args, scalar_exprs):
    if len(args) != 4:
        raise RuntimeError("affine chip requires tgt, src, multiplier, offset")
    _tgt_name, tgt_val = _selector_value(args[0], _TARGET_SELECTORS, "affine tgt")
    _src_name, src_val = _selector_value(args[1], _SOURCE_SELECTORS, "affine src")
    mult = _compile_expr(args[2], label="affine multiplier", expected="complex")
    offset = _compile_expr(args[3], label="affine offset", expected="complex")
    mult_re, mult_im, mult_ref = _add_arg_expr(mult, scalar_exprs, expected="complex")
    off_re, off_im, off_ref = _add_arg_expr(offset, scalar_exprs, expected="complex")
    return _token(
        COEFF_OP_AFFINE,
        src=src_val,
        tgt=tgt_val,
        n_args=2,
        args=[mult_re, off_re],
        args_im=[mult_im, off_im],
        expr_refs=[mult_ref, off_ref],
    )


def _compile_blend(args, scalar_exprs):
    if len(args) != 1:
        raise RuntimeError("blend chip requires one t argument")
    expr = _compile_expr(args[0], label="blend t", expected="real")
    value, _imag, ref = _add_arg_expr(expr, scalar_exprs, expected="real")
    return _token(COEFF_OP_BLEND, n_args=1, args=[value], expr_refs=[ref])


def _compile_poke(op, args, scalar_exprs, label):
    if len(args) != 2:
        raise RuntimeError(f"{label} chip requires index and value")
    index = int(_finite_number(args[0], f"{label} index"))
    if index < 0 or index >= MAX_VECTOR_LEN:
        raise RuntimeError(f"{label} index must be in [0,{MAX_VECTOR_LEN - 1}], got {index}")
    expr = _compile_expr(args[1], label=f"{label} value", expected="complex")
    re, im, ref = _add_arg_expr(expr, scalar_exprs, expected="complex")
    return _token(op, n_args=2, args=[index, re], args_im=[0.0, im], expr_refs=[-1, ref])


def _compile_vector_binary(name, args):
    if len(args) != 3:
        raise RuntimeError(f"{name} chip requires tgt, src1, src2")
    _tgt_name, tgt_val = _selector_value(args[0], _TARGET_SELECTORS, f"{name} tgt")
    _src1_name, src1_val = _selector_value(args[1], _VECTOR_SOURCE_SELECTORS, f"{name} src1")
    _src2_name, src2_val = _selector_value(args[2], _VECTOR_SOURCE_SELECTORS, f"{name} src2")
    return _token(
        COEFF_OP_VECTOR_BINARY,
        fn_index=_VECTOR_BINARY_OPS[name],
        src=src1_val,
        tgt=tgt_val,
        n_args=1,
        args=[src2_val],
    )


def _compile_vector_unary(name, args):
    if len(args) != 2:
        raise RuntimeError(f"{name} chip requires tgt and src")
    _tgt_name, tgt_val = _selector_value(args[0], _TARGET_SELECTORS, f"{name} tgt")
    _src_name, src_val = _selector_value(args[1], _VECTOR_SOURCE_SELECTORS, f"{name} src")
    return _token(
        COEFF_OP_VECTOR_UNARY,
        fn_index=_VECTOR_UNARY_OPS[name],
        src=src_val,
        tgt=tgt_val,
    )


def _compile_vector_roll(name, args):
    if len(args) != 3:
        raise RuntimeError(f"{name} chip requires tgt, src, n")
    _tgt_name, tgt_val = _selector_value(args[0], _TARGET_SELECTORS, f"{name} tgt")
    _src_name, src_val = _selector_value(args[1], _VECTOR_SOURCE_SELECTORS, f"{name} src")
    n = _integer_literal(args[2], f"{name} n")
    return _token(
        COEFF_OP_VECTOR_ROLL,
        fn_index=_VECTOR_ROLL_OPS[name],
        src=src_val,
        tgt=tgt_val,
        n_args=1,
        args=[n],
    )


def _compile_vector_argsort(args):
    if len(args) != 3:
        raise RuntimeError("argsort chip requires tgt, src1, src2")
    _tgt_name, tgt_val = _selector_value(args[0], _TARGET_SELECTORS, "argsort tgt")
    _src1_name, src1_val = _selector_value(args[1], _VECTOR_SOURCE_SELECTORS, "argsort src1")
    _src2_name, src2_val = _selector_value(args[2], _VECTOR_SOURCE_SELECTORS, "argsort src2")
    return _token(
        COEFF_OP_VECTOR_ARGSORT,
        src=src1_val,
        tgt=tgt_val,
        n_args=1,
        args=[src2_val],
    )


def _compile_typed_push_scalar(args, scalar_exprs):
    if len(args) != 1:
        raise RuntimeError("typed_push_scalar requires one expression")
    raw = str(args[0]).strip().lower()
    scalar_sources = {
        "p1": COEFF_SCALAR_SRC_P1,
        "p2": COEFF_SCALAR_SRC_P2,
        "t1": COEFF_SCALAR_SRC_T1,
        "t2": COEFF_SCALAR_SRC_T2,
        "poly_len": COEFF_SCALAR_SRC_POLY_LEN,
    }
    if raw in scalar_sources:
        return _token(COEFF_OP_TYPED_PUSH_SCALAR, fn_index=scalar_sources[raw])
    expr = _compile_expr(args[0], label="typed scalar", expected="complex")
    re, im, ref = _add_arg_expr(expr, scalar_exprs, expected="complex")
    return _token(
        COEFF_OP_TYPED_PUSH_SCALAR,
        n_args=1,
        args=[re],
        args_im=[im],
        expr_refs=[ref],
    )


def _compile_typed_push_vector(args):
    if len(args) != 1:
        raise RuntimeError("typed_push_vector requires one source")
    src_name, src_val = _selector_value(args[0], _SOURCE_SELECTORS, "typed vector source")
    return _token(COEFF_OP_TYPED_PUSH_VECTOR, src=src_val)


def _compile_typed_binary(args):
    if len(args) != 1:
        raise RuntimeError("typed_binary requires one op name")
    name = str(args[0]).strip().lower()
    name = {"sub": "subtract", "mul": "multiply", "div": "divide", "pow": "power"}.get(name, name)
    if name not in _VECTOR_BINARY_OPS:
        raise RuntimeError(f"unknown typed binary op: {args[0]}")
    return _token(COEFF_OP_TYPED_BINARY, fn_index=_VECTOR_BINARY_OPS[name])


def _compile_typed_unary(args):
    if len(args) != 1:
        raise RuntimeError("typed_unary requires one op name")
    name = str(args[0]).strip().lower()
    name = "abs" if name == "mod" else name
    if name not in _VECTOR_UNARY_OPS:
        raise RuntimeError(f"unknown typed unary op: {args[0]}")
    return _token(COEFF_OP_TYPED_UNARY, fn_index=_VECTOR_UNARY_OPS[name])


def _compile_littlewood(args, scalar_exprs):
    if len(args) not in {3, 4}:
        raise RuntimeError("littlewood chip requires tgt, field1, field2, and optional andy")
    _tgt_name, tgt_val = _selector_value(args[0], _TARGET_SELECTORS, "littlewood tgt")
    field1_expr = _compile_expr(args[1], label="littlewood field1", expected="complex")
    field2_expr = _compile_expr(args[2], label="littlewood field2", expected="complex")
    field1_re, field1_im, field1_ref = _add_arg_expr(field1_expr, scalar_exprs, expected="complex")
    field2_re, field2_im, field2_ref = _add_arg_expr(field2_expr, scalar_exprs, expected="complex")
    andy = 0.0
    andy_ref = -1
    if len(args) == 4:
        andy, andy_ref = _compile_andy(args[3], scalar_exprs, "littlewood andy")
    return _token(
        COEFF_OP_LITTLEWOOD,
        tgt=tgt_val,
        n_args=3,
        args=[field1_re, field2_re, andy],
        args_im=[field1_im, field2_im, 0.0],
        expr_refs=[field1_ref, field2_ref, andy_ref],
    )


def _linear_legacy_args(spec, raw_args, scalar_exprs):
    raw_args = list(raw_args)
    andy = 0.0
    andy_expr_ref = -1
    andy_raw = None
    if spec.get("supports_andy") and len(raw_args) in {3, 5}:
        andy_raw = raw_args[-1]
        raw_args = raw_args[:-1]
    if len(raw_args) in {0, 1, 2}:
        raw_multiplier = raw_args[0] if len(raw_args) >= 1 else "100"
        raw_offset = raw_args[1] if len(raw_args) >= 2 else "0"
        args = []
        args_im = []
        expr_refs = []
        for label, raw in (("multiplier", raw_multiplier), ("offset", raw_offset)):
            expr = _compile_expr(raw, label=f"legacy(linear) {label}", expected="complex")
            re, im, ref = _add_arg_expr(expr, scalar_exprs, expected="complex")
            args.append(re)
            args_im.append(im)
            expr_refs.append(ref)
        if andy_raw is not None:
            andy, andy_expr_ref = _compile_andy(andy_raw, scalar_exprs, "legacy(linear) andy")
        return args, args_im, expr_refs, andy, andy_expr_ref
    if len(raw_args) == 4:
        args = []
        args_im = []
        expr_refs = []
        for idx, raw in enumerate(raw_args):
            expr = _compile_expr(raw, label=f"legacy(scale100) arg {idx}", expected="real")
            re, im, ref = _add_arg_expr(expr, scalar_exprs, expected="real")
            args.append(re)
            args_im.append(im)
            expr_refs.append(ref)
        if andy_raw is not None:
            andy, andy_expr_ref = _compile_andy(andy_raw, scalar_exprs, "legacy(linear) andy")
        return args, args_im, expr_refs, andy, andy_expr_ref
    raise RuntimeError(
        "legacy(linear) expects multiplier and offset, or legacy scale100 four real arguments"
    )


def _pow_legacy_args(spec, raw_args, scalar_exprs):
    raw_args = list(raw_args)
    andy = 0.0
    andy_expr_ref = -1
    andy_raw = None
    if spec.get("supports_andy") and len(raw_args) in {3, 5}:
        andy_raw = raw_args[-1]
        raw_args = raw_args[:-1]
    if len(raw_args) in {0, 1, 2}:
        raw_multiplier = raw_args[0] if len(raw_args) >= 1 else "1"
        raw_exponent = raw_args[1] if len(raw_args) >= 2 else "1"
        args = []
        args_im = []
        expr_refs = []
        for label, raw in (("multiplier", raw_multiplier), ("exponent", raw_exponent)):
            expr = _compile_expr(raw, label=f"legacy(pow) {label}", expected="complex")
            re, im, ref = _add_arg_expr(expr, scalar_exprs, expected="complex")
            args.append(re)
            args_im.append(im)
            expr_refs.append(ref)
        if andy_raw is not None:
            andy, andy_expr_ref = _compile_andy(andy_raw, scalar_exprs, "legacy(pow) andy")
        return args, args_im, expr_refs, andy, andy_expr_ref
    if len(raw_args) == 4:
        args = []
        args_im = []
        expr_refs = []
        for idx, raw in enumerate(raw_args):
            expr = _compile_expr(raw, label=f"legacy(pow) old real arg {idx}", expected="real")
            re, im, ref = _add_arg_expr(expr, scalar_exprs, expected="real")
            args.append(re)
            args_im.append(im)
            expr_refs.append(ref)
        if andy_raw is not None:
            andy, andy_expr_ref = _compile_andy(andy_raw, scalar_exprs, "legacy(pow) andy")
        return args, args_im, expr_refs, andy, andy_expr_ref
    raise RuntimeError("legacy(pow) expects multiplier and exponent, or old four real arguments")


def _exp_legacy_args(spec, raw_args, scalar_exprs):
    raw_args = list(raw_args)
    andy = 0.0
    andy_expr_ref = -1
    andy_raw = None
    if spec.get("supports_andy") and len(raw_args) == 3:
        andy_raw = raw_args[-1]
        raw_args = raw_args[:-1]
    if len(raw_args) > 2:
        raise RuntimeError("legacy(exp) expects multiplier, optional offset, and optional andy")
    raw_multiplier = raw_args[0] if len(raw_args) >= 1 else "1"
    raw_offset = raw_args[1] if len(raw_args) >= 2 else "0"
    mult_re, mult_im, mult_re_ref, mult_im_ref = _compile_complex_components(
        raw_multiplier, scalar_exprs, "legacy(exp) multiplier"
    )
    off_re, off_im, off_re_ref, off_im_ref = _compile_complex_components(
        raw_offset, scalar_exprs, "legacy(exp) offset"
    )
    if andy_raw is not None:
        andy, andy_expr_ref = _compile_andy(andy_raw, scalar_exprs, "legacy(exp) andy")
    return (
        [mult_re, mult_im, off_re, off_im],
        [0.0, 0.0, 0.0, 0.0],
        [mult_re_ref, mult_im_ref, off_re_ref, off_im_ref],
        andy,
        andy_expr_ref,
    )


def _round_legacy_args(spec, raw_args, scalar_exprs):
    raw_args = list(raw_args)
    andy = 0.0
    andy_expr_ref = -1
    andy_raw = None
    if spec.get("supports_andy") and len(raw_args) == 3:
        # Back-compat for the old real/imag component form: round(a, b, andy).
        andy_raw = raw_args[-1]
        raw_args = raw_args[:-1]
        args = []
        args_im = []
        expr_refs = []
        for idx, raw in enumerate(raw_args):
            expr = _compile_expr(raw, label=f"legacy(round) old real arg {idx}", expected="real")
            re, im, ref = _add_arg_expr(expr, scalar_exprs, expected="real")
            args.append(re)
            args_im.append(im)
            expr_refs.append(ref)
        andy, andy_expr_ref = _compile_andy(andy_raw, scalar_exprs, "legacy(round) andy")
        return args, args_im, expr_refs, andy, andy_expr_ref
    if spec.get("supports_andy") and len(raw_args) == 2:
        andy_raw = raw_args[-1]
        raw_args = raw_args[:-1]
    if len(raw_args) > 1:
        raise RuntimeError("legacy(round) expects multiplier and optional andy")
    raw_multiplier = raw_args[0] if raw_args else "1"
    mult_re, mult_im, mult_re_ref, mult_im_ref = _compile_complex_components(
        raw_multiplier, scalar_exprs, "legacy(round) multiplier"
    )
    if andy_raw is not None:
        andy, andy_expr_ref = _compile_andy(andy_raw, scalar_exprs, "legacy(round) andy")
    return (
        [mult_re, mult_im],
        [0.0, 0.0],
        [mult_re_ref, mult_im_ref],
        andy,
        andy_expr_ref,
    )


def _legacy_args(spec, raw_args, scalar_exprs):
    if int(spec.get("fn_index") or 0) == 14:
        return _linear_legacy_args(spec, raw_args, scalar_exprs)
    if int(spec.get("fn_index") or 0) == 16:
        return _exp_legacy_args(spec, raw_args, scalar_exprs)
    if int(spec.get("fn_index") or 0) == 23:
        return _round_legacy_args(spec, raw_args, scalar_exprs)
    if int(spec.get("fn_index") or 0) == 24:
        return _pow_legacy_args(spec, raw_args, scalar_exprs)
    declared = list(spec["args"])
    raw_args = list(raw_args)
    andy = 0.0
    andy_expr_ref = -1
    andy_raw = None
    if spec.get("supports_andy") and len(raw_args) == len(declared) + 1:
        andy_raw = raw_args[-1]
        raw_args = raw_args[:-1]
    if len(raw_args) > len(declared):
        raise RuntimeError(f"legacy({spec['name']}) got too many arguments")
    args = []
    args_im = []
    expr_refs = []
    for idx, decl in enumerate(declared):
        raw = raw_args[idx] if idx < len(raw_args) else decl.get("default", 0.0)
        arg_type = str(decl.get("type") or "real")
        if arg_type == "enum":
            value = str(raw).strip().lower()
            choices = [str(x).strip().lower() for x in (decl.get("choices") or [])]
            if value not in choices:
                raise RuntimeError(
                    f"legacy({spec['name']}) arg {idx} must be one of {choices}, got {raw!r}"
                )
            args.append(_ENUM_ARG_VALUES[value])
            args_im.append(0.0)
            expr_refs.append(-1)
        elif arg_type == "int":
            number = int(_finite_number(raw, f"legacy({spec['name']}) arg {idx}"))
            args.append(float(number))
            args_im.append(0.0)
            expr_refs.append(-1)
        elif arg_type == "real":
            expr = _compile_expr(raw, label=f"legacy({spec['name']}) arg {idx}", expected="real")
            re, im, ref = _add_arg_expr(expr, scalar_exprs, expected="real")
            args.append(re)
            args_im.append(im)
            expr_refs.append(ref)
        else:
            raise RuntimeError(
                f"legacy({spec['name']}) arg {idx} has unsupported type {arg_type!r}"
            )
    if andy_raw is not None:
        andy, andy_expr_ref = _compile_andy(andy_raw, scalar_exprs, f"legacy({spec['name']}) andy")
    return args, args_im, expr_refs, andy, andy_expr_ref


def _legacy_token(name, src, tgt, args, scalar_exprs):
    registry = legacy_registry()["by_name"]
    canonical_name = _canonical_legacy_name(name)
    if canonical_name not in registry:
        raise RuntimeError(f"unknown legacy coeff transform: {name}")
    spec = registry[canonical_name]
    src_name, src_val = _selector_value(src, _SOURCE_SELECTORS, "legacy src")
    tgt_name, tgt_val = _selector_value(tgt, _TARGET_SELECTORS, "legacy tgt")
    if src_name not in spec["allowed_src"]:
        raise RuntimeError(f"legacy({canonical_name}) does not support src={src_name}")
    if tgt_name not in spec["allowed_tgt"]:
        raise RuntimeError(f"legacy({canonical_name}) does not support tgt={tgt_name}")
    values, values_im, expr_refs, andy, andy_expr_ref = _legacy_args(spec, args, scalar_exprs)
    return _token(
        COEFF_OP_LEGACY,
        fn_index=spec["fn_index"],
        src=src_val,
        tgt=tgt_val,
        n_args=len(values),
        args=values,
        args_im=values_im,
        expr_refs=expr_refs,
        andy=andy,
        andy_expr_ref=andy_expr_ref,
    )


def _native_transform_token(name, src, tgt, args, scalar_exprs):
    registry = legacy_registry()["by_name"]
    canonical_name = _canonical_legacy_name(name)
    if canonical_name not in registry:
        raise RuntimeError(f"unknown native coeff transform: {name}")
    spec = registry[canonical_name]
    src_name, src_val = _selector_value(src, _SOURCE_SELECTORS, "native transform src")
    tgt_name, tgt_val = _selector_value(tgt, _TARGET_SELECTORS, "native transform tgt")
    if src_name not in spec["allowed_src"]:
        raise RuntimeError(f"{canonical_name} does not support src={src_name}")
    if tgt_name not in spec["allowed_tgt"]:
        raise RuntimeError(f"{canonical_name} does not support tgt={tgt_name}")
    values, values_im, expr_refs, andy, andy_expr_ref = _legacy_args(spec, args, scalar_exprs)
    return _token(
        COEFF_OP_NATIVE_TRANSFORM,
        fn_index=spec["fn_index"],
        src=src_val,
        tgt=tgt_val,
        n_args=len(values),
        args=values,
        args_im=values_im,
        expr_refs=expr_refs,
        andy=andy,
        andy_expr_ref=andy_expr_ref,
    )


def _max_native_transform_stack_arg_count(spec):
    fn_index = int(spec.get("fn_index") or 0)
    if fn_index in {14, 16, 24}:
        return 2
    if fn_index == 23:
        return 1
    return len(spec.get("args") or [])


def _native_transform_stack_arg_token(name, src, tgt, stack_arg_count, scalar_exprs, andy_arg=None):
    registry = legacy_registry()["by_name"]
    canonical_name = _canonical_legacy_name(name)
    if canonical_name not in registry:
        raise RuntimeError(f"unknown native coeff transform: {name}")
    spec = registry[canonical_name]
    src_name, src_val = _selector_value(src, _SOURCE_SELECTORS, "native transform src")
    tgt_name, tgt_val = _selector_value(tgt, _TARGET_SELECTORS, "native transform tgt")
    if src_name not in spec["allowed_src"]:
        raise RuntimeError(f"{canonical_name} does not support src={src_name}")
    if tgt_name not in spec["allowed_tgt"]:
        raise RuntimeError(f"{canonical_name} does not support tgt={tgt_name}")
    count = int(stack_arg_count)
    if count < 0 or count > MAX_ARGS:
        raise RuntimeError(f"{canonical_name} stack arg count must be in [0,{MAX_ARGS}]")
    max_count = _max_native_transform_stack_arg_count(spec)
    if count > max_count:
        raise RuntimeError(f"{canonical_name} stack arg count must be <= {max_count}, got {count}")
    fields = dict(
        fn_index=spec["fn_index"],
        src=src_val,
        tgt=tgt_val,
        stack_arg_count=count,
    )
    if andy_arg is not None:
        if not spec.get("supports_andy"):
            raise RuntimeError(f"{canonical_name} does not support andy")
        andy, andy_expr_ref = _compile_andy(andy_arg, scalar_exprs, f"native transform {canonical_name} andy")
        fields["andy"] = andy
        fields["andy_expr_ref"] = andy_expr_ref
    return _token(
        COEFF_OP_NATIVE_TRANSFORM,
        **fields,
    )


def _expand_macros(chain, macro_resolver, stack=None, depth=0):
    if stack is None:
        stack = []
    if depth > MAX_MACRO_DEPTH:
        raise RuntimeError(f"coeff program macro expansion exceeded depth {MAX_MACRO_DEPTH}")
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
            raise RuntimeError(f"coeff program macro cycle: {cycle}")
        if macro_resolver is None:
            raise RuntimeError(f"coeff program macro resolver is required for macro({macro_id})")
        resolved = _canonical_source_chain(macro_resolver(macro_id))
        nested, nested_count = _expand_macros(
            resolved,
            macro_resolver,
            stack=stack + [macro_id],
            depth=depth + 1,
        )
        expanded.extend(nested)
        expanded_count += 1 + nested_count
    return expanded, expanded_count


def _lower_chip(chip, scalar_exprs):
    name, args = _chip_args(chip)
    if name == "macro":
        raise RuntimeError("macro chip survived expansion")
    if name in {"push_const", "const"}:
        return [_compile_const(args, scalar_exprs)]
    if name == "push_linspace":
        return [_compile_linspace(args, scalar_exprs)]
    if name == "push_range":
        return [_compile_range(args, scalar_exprs)]
    if name == "set":
        return [_compile_set(args)]
    if name in {"affine", "linear"}:
        return [_compile_affine(args, scalar_exprs)]
    if name == "push":
        if len(args) != 1:
            raise RuntimeError("push chip requires source cf or poly")
        src_name, src_val = _selector_value(args[0], {"cf": COEFF_SEL_CF, "poly": COEFF_SEL_POLY}, "push source")
        return [_token(COEFF_OP_PUSH, src=src_val)]
    if name == "emit":
        if args:
            raise RuntimeError("emit chip takes no arguments")
        return [_token(COEFF_OP_EMIT)]
    if name == "blend":
        return [_compile_blend(args, scalar_exprs)]
    if name == "poke_poly":
        return [_compile_poke(COEFF_OP_POKE_POLY, args, scalar_exprs, "poke_poly")]
    if name == "poke_tos":
        return [_compile_poke(COEFF_OP_POKE_TOS, args, scalar_exprs, "poke_tos")]
    if name in _VECTOR_BINARY_OPS:
        return [_compile_vector_binary(name, args)]
    if name in _VECTOR_UNARY_OPS:
        return [_compile_vector_unary(name, args)]
    if name in _VECTOR_ROLL_OPS:
        return [_compile_vector_roll(name, args)]
    if name == "argsort":
        return [_compile_vector_argsort(args)]
    if name == "_typed_push_scalar":
        return [_compile_typed_push_scalar(args, scalar_exprs)]
    if name == "_typed_push_vector":
        return [_compile_typed_push_vector(args)]
    if name == "_typed_binary":
        return [_compile_typed_binary(args)]
    if name == "_typed_unary":
        return [_compile_typed_unary(args)]
    if name == "_typed_get_scalar":
        if args:
            raise RuntimeError("typed_get_scalar takes no arguments")
        return [_token(COEFF_OP_TYPED_GET_SCALAR)]
    if name == "_typed_set_poly":
        if args:
            raise RuntimeError("typed_set_poly takes no arguments")
        return [_token(COEFF_OP_TYPED_SET_POLY)]
    if name == "_typed_poke_poly":
        if args:
            raise RuntimeError("typed_poke_poly takes no arguments")
        return [_token(COEFF_OP_TYPED_POKE_POLY)]
    if name == "_typed_fill":
        if args:
            raise RuntimeError("typed_fill takes no arguments")
        return [_token(COEFF_OP_TYPED_FILL)]
    if name == "_native_transform":
        if len(args) < 3:
            raise RuntimeError("native transform chip requires name, src, tgt, and optional args")
        return [_native_transform_token(str(args[0]).strip().lower(), args[1], args[2], args[3:], scalar_exprs)]
    if name == "_native_transform_stack_args":
        if len(args) not in {4, 5}:
            raise RuntimeError("native transform stack-args chip requires name, src, tgt, arg count, and optional andy")
        andy_arg = args[4] if len(args) == 5 else None
        return [_native_transform_stack_arg_token(str(args[0]).strip().lower(), args[1], args[2], args[3], scalar_exprs, andy_arg)]
    if name == "_typed_blend":
        if args:
            raise RuntimeError("typed_blend takes no arguments")
        return [_token(COEFF_OP_TYPED_BLEND)]
    if name == "littlewood":
        return [_compile_littlewood(args, scalar_exprs)]
    if name in legacy_registry()["by_name"]:
        if len(args) < 2:
            raise RuntimeError(f"{name} chip requires target, source, and optional args")
        return [_native_transform_token(name, args[1], args[0], args[2:], scalar_exprs)]
    if name in _STACK_OPS:
        if args:
            raise RuntimeError(f"{name} chip takes no arguments")
        return [_token(_STACK_OPS[name])]
    if name == "legacy":
        if len(args) < 3:
            raise RuntimeError("legacy chip requires name, src, tgt, and optional args")
        return [_legacy_token(str(args[0]).strip().lower(), args[1], args[2], args[3:], scalar_exprs)]
    raise RuntimeError(f"unknown coeff program chip: {name}")


def _lower_chain(chain):
    tokens = []
    scalar_exprs = []
    for chip in chain:
        tokens.extend(_lower_chip(chip, scalar_exprs))
    if len(tokens) > MAX_PROGRAM_TOKENS:
        raise RuntimeError(
            f"coeff program has {len(tokens)} tokens after expansion; max is {MAX_PROGRAM_TOKENS}"
        )
    return tokens, scalar_exprs


def _validate_stack(tokens):
    types = []
    max_depth = 0
    diagnostics = []

    def depth():
        return len(types)

    def need_any(idx, label):
        if not types:
            raise RuntimeError(f"{label} at token {idx}: stack depth is 0 (need >=1)")
        return types.pop()

    def need_vector_pop(idx, label):
        before = depth()
        item = need_any(idx, label)
        if item != "vector":
            raise RuntimeError(f"{label} at token {idx}: top of stack is {item} (need vector)")

    def need_vector_peek(idx, label):
        if not types:
            raise RuntimeError(f"{label}(peek) at token {idx}: stack depth is 0 (need >=1)")
        if types[-1] != "vector":
            raise RuntimeError(f"{label}(peek) at token {idx}: top of stack is {types[-1]} (need vector)")

    def vector_source(selector, idx, label):
        if selector == COEFF_SEL_POP:
            need_vector_pop(idx, label)
        elif selector == COEFF_SEL_PEEK:
            need_vector_peek(idx, label)

    def vector_target(selector):
        if selector == COEFF_SEL_PUSH:
            types.append("vector")

    for idx, token in enumerate(tokens):
        op = int(token.get("op") or 0)
        before = depth()
        if op in {COEFF_OP_CONST, COEFF_OP_PUSH, COEFF_OP_LINSPACE, COEFF_OP_RANGE}:
            types.append("vector")
        elif op == COEFF_OP_SET:
            vector_source(int(token.get("src") or 0), idx, "set src")
        elif op == COEFF_OP_AFFINE:
            vector_source(int(token.get("src") or 0), idx, "affine src")
            vector_target(int(token.get("tgt") or 0))
        elif op == COEFF_OP_EMIT:
            # Coeff Program keeps poly as an explicit mutable output register.
            # emit commits the stack top when present; with an empty stack it is
            # a harmless "commit current poly" marker after direct poly writes.
            if types:
                item = types.pop()
                if item != "vector":
                    raise RuntimeError(f"emit at token {idx}: top of stack is {item} (need vector)")
        elif op == COEFF_OP_DUPLICATE:
            if depth() < 1:
                raise RuntimeError(f"duplicate at token {idx}: stack depth is {before} (need >=1)")
            types.append(types[-1])
        elif op == COEFF_OP_SWAP:
            if depth() < 2:
                raise RuntimeError(f"swap at token {idx}: stack depth is {before} (need >=2)")
            types[-1], types[-2] = types[-2], types[-1]
        elif op == COEFF_OP_POP:
            if depth() < 1:
                raise RuntimeError(f"pop at token {idx}: stack depth is {before} (need >=1)")
            types.pop()
        elif op == COEFF_OP_FLUSH:
            types = []
        elif op == COEFF_OP_BLEND:
            if depth() < 2:
                raise RuntimeError(f"blend at token {idx}: stack depth is {before} (need >=2)")
            need_vector_pop(idx, "blend top")
            need_vector_pop(idx, "blend below")
            types.append("vector")
        elif op == COEFF_OP_POKE_POLY:
            pass
        elif op == COEFF_OP_POKE_TOS:
            if depth() < 1:
                raise RuntimeError(f"poke_tos at token {idx}: stack depth is {before} (need >=1)")
            if types[-1] != "vector":
                raise RuntimeError(f"poke_tos at token {idx}: top of stack is {types[-1]} (need vector)")
        elif op in {COEFF_OP_VECTOR_BINARY, COEFF_OP_VECTOR_ARGSORT}:
            src2 = int((token.get("args") or [0])[0])
            vector_source(int(token.get("src") or 0), idx, "vector src1")
            vector_source(src2, idx, "vector src2")
            vector_target(int(token.get("tgt") or 0))
        elif op in {COEFF_OP_VECTOR_UNARY, COEFF_OP_VECTOR_ROLL}:
            vector_source(int(token.get("src") or 0), idx, "vector src")
            vector_target(int(token.get("tgt") or 0))
        elif op == COEFF_OP_LITTLEWOOD:
            vector_target(int(token.get("tgt") or 0))
        elif op in {COEFF_OP_LEGACY, COEFF_OP_NATIVE_TRANSFORM}:
            stack_arg_count = int(token.get("stack_arg_count") or 0)
            for _ in range(stack_arg_count):
                item = need_any(idx, "native transform arg")
                if item != "scalar":
                    raise RuntimeError(f"native transform arg at token {idx}: top of stack is {item} (need scalar)")
            src = int(token.get("src") or 0)
            tgt = int(token.get("tgt") or 0)
            if src == COEFF_SEL_POP:
                need_vector_pop(idx, "native transform")
            elif src == COEFF_SEL_PEEK:
                need_vector_peek(idx, "native transform")
            vector_target(tgt)
            spec = legacy_registry()["by_index"].get(int(token.get("fn_index") or 0))
            if spec and spec.get("length_policy") == "may_change":
                diagnostics.append({"level": "info", "message": f"{spec['name']} may change vector length"})
        elif op == COEFF_OP_TYPED_PUSH_SCALAR:
            types.append("scalar")
        elif op == COEFF_OP_TYPED_PUSH_VECTOR:
            vector_source(int(token.get("src") or 0), idx, "typed vector source")
            types.append("vector")
        elif op == COEFF_OP_TYPED_BINARY:
            if depth() < 2:
                raise RuntimeError(f"typed binary at token {idx}: stack depth is {before} (need >=2)")
            right = types.pop()
            left = types.pop()
            types.append("vector" if "vector" in {left, right} else "scalar")
        elif op == COEFF_OP_TYPED_UNARY:
            if depth() < 1:
                raise RuntimeError(f"typed unary at token {idx}: stack depth is {before} (need >=1)")
        elif op == COEFF_OP_TYPED_GET_SCALAR:
            if depth() < 2:
                raise RuntimeError(f"get_scalar at token {idx}: stack depth is {before} (need >=2)")
            index_type = types.pop()
            vector_type = types.pop()
            if index_type != "scalar":
                raise RuntimeError(f"get_scalar at token {idx}: index is {index_type} (need scalar)")
            if vector_type != "vector":
                raise RuntimeError(f"get_scalar at token {idx}: source is {vector_type} (need vector)")
            types.append("scalar")
        elif op == COEFF_OP_TYPED_SET_POLY:
            if depth() < 1:
                raise RuntimeError(f"typed_set_poly at token {idx}: stack depth is {before} (need >=1)")
            item = types.pop()
            if item != "vector":
                raise RuntimeError(f"typed_set_poly at token {idx}: top of stack is {item} (need vector)")
        elif op == COEFF_OP_TYPED_POKE_POLY:
            if depth() < 2:
                raise RuntimeError(f"typed_poke_poly at token {idx}: stack depth is {before} (need >=2)")
            value_type = types.pop()
            index_type = types.pop()
            if value_type != "scalar":
                raise RuntimeError(f"typed_poke_poly at token {idx}: value is {value_type} (need scalar)")
            if index_type != "scalar":
                raise RuntimeError(f"typed_poke_poly at token {idx}: index is {index_type} (need scalar)")
        elif op == COEFF_OP_TYPED_FILL:
            if depth() < 2:
                raise RuntimeError(f"typed_fill at token {idx}: stack depth is {before} (need >=2)")
            value_type = types.pop()
            length_type = types.pop()
            if value_type != "scalar":
                raise RuntimeError(f"typed_fill at token {idx}: value is {value_type} (need scalar)")
            if length_type != "scalar":
                raise RuntimeError(f"typed_fill at token {idx}: length is {length_type} (need scalar)")
            types.append("vector")
        elif op == COEFF_OP_TYPED_BLEND:
            if depth() < 3:
                raise RuntimeError(f"typed_blend at token {idx}: stack depth is {before} (need >=3)")
            t_type = types.pop()
            top_type = types.pop()
            below_type = types.pop()
            if t_type != "scalar":
                raise RuntimeError(f"typed_blend at token {idx}: t is {t_type} (need scalar)")
            if top_type != "vector" or below_type != "vector":
                raise RuntimeError(f"typed_blend at token {idx}: inputs must be vectors")
            types.append("vector")
        else:
            raise RuntimeError(f"unknown coeff program opcode at token {idx}: {op}")
        if depth() > MAX_VECTOR_STACK:
            raise RuntimeError(f"coeff program stack depth {depth()} exceeds max {MAX_VECTOR_STACK} at token {idx}")
        max_depth = max(max_depth, depth())
    if depth() != 0:
        raise RuntimeError(f"coeff program final stack depth is {depth()}; expected 0")
    return {"stack_max": max_depth, "diagnostics": diagnostics}


def _execution_spec(tokens, scalar_exprs):
    parts = []
    for token in tokens:
        op = int(token["op"])
        fields = [_OP_NAMES.get(op, str(op))]
        if op == COEFF_OP_CONST:
            fields.extend(["length", _format_length_arg((token.get("args") or [0])[0])])
            ref = (token.get("expr_refs") or [-1, -1])[1]
            if ref >= 0:
                fields.append(f"expr{ref}")
            else:
                fields.append(_format_complex_number((token.get("args") or [0, 0])[1], (token.get("args_im") or [0, 0])[1]))
        elif op == COEFF_OP_LINSPACE:
            args = token.get("args") or []
            refs = token.get("expr_refs") or []
            for idx, value in enumerate(args):
                fields.append(f"expr{refs[idx]}" if idx < len(refs) and refs[idx] >= 0 else _format_length_or_number(value) if len(args) == 1 else _format_number(value))
        elif op == COEFF_OP_RANGE:
            args = token.get("args") or []
            refs = token.get("expr_refs") or []
            for idx, value in enumerate(args):
                fields.append(f"expr{refs[idx]}" if idx < len(refs) and refs[idx] >= 0 else _format_length_or_number(value) if len(args) == 1 else _format_number(value))
        elif op == COEFF_OP_SET:
            fields.extend([
                _SELECTOR_NAMES.get(int(token.get("tgt") or 0), str(token.get("tgt") or 0)),
                _SELECTOR_NAMES.get(int(token.get("src") or 0), str(token.get("src") or 0)),
            ])
        elif op == COEFF_OP_AFFINE:
            fields.extend([
                _SELECTOR_NAMES.get(int(token.get("tgt") or 0), str(token.get("tgt") or 0)),
                _SELECTOR_NAMES.get(int(token.get("src") or 0), str(token.get("src") or 0)),
            ])
            args = token.get("args") or [0, 0]
            args_im = token.get("args_im") or [0, 0]
            refs = token.get("expr_refs") or [-1, -1]
            for idx, value in enumerate(args):
                fields.append(f"expr{refs[idx]}" if idx < len(refs) and refs[idx] >= 0 else _format_complex_number(value, args_im[idx] if idx < len(args_im) else 0.0))
        elif op == COEFF_OP_TYPED_PUSH_SCALAR:
            scalar_src_names = {
                COEFF_SCALAR_SRC_P1: "p1",
                COEFF_SCALAR_SRC_P2: "p2",
                COEFF_SCALAR_SRC_T1: "t1",
                COEFF_SCALAR_SRC_T2: "t2",
                COEFF_SCALAR_SRC_POLY_LEN: "poly_len",
            }
            scalar_src = int(token.get("fn_index") or 0)
            if scalar_src:
                fields.append(scalar_src_names.get(scalar_src, str(scalar_src)))
            else:
                refs = token.get("expr_refs") or [-1]
                args = token.get("args") or [0]
                args_im = token.get("args_im") or [0]
                fields.append(f"expr{refs[0]}" if refs and refs[0] >= 0 else _format_complex_number(args[0], args_im[0] if args_im else 0.0))
        elif op == COEFF_OP_TYPED_PUSH_VECTOR:
            fields.append(_SELECTOR_NAMES.get(int(token.get("src") or 0), str(token.get("src") or 0)))
        elif op == COEFF_OP_TYPED_BINARY:
            fields.append(_VECTOR_BINARY_NAMES.get(int(token.get("fn_index") or 0), str(token.get("fn_index") or 0)))
        elif op == COEFF_OP_TYPED_UNARY:
            fields.append(_VECTOR_UNARY_NAMES.get(int(token.get("fn_index") or 0), str(token.get("fn_index") or 0)))
        elif op in {COEFF_OP_TYPED_GET_SCALAR, COEFF_OP_TYPED_SET_POLY, COEFF_OP_TYPED_POKE_POLY, COEFF_OP_TYPED_FILL, COEFF_OP_TYPED_BLEND}:
            pass
        elif op == COEFF_OP_PUSH:
            fields.append(_SELECTOR_NAMES.get(int(token.get("src") or 0), str(token.get("src") or 0)))
        elif op == COEFF_OP_BLEND:
            ref = (token.get("expr_refs") or [-1])[0]
            fields.append(f"expr{ref}" if ref >= 0 else _format_number((token.get("args") or [0])[0]))
        elif op in {COEFF_OP_POKE_POLY, COEFF_OP_POKE_TOS}:
            args = token.get("args") or [0, 0]
            args_im = token.get("args_im") or [0, 0]
            refs = token.get("expr_refs") or [-1, -1]
            fields.append(_format_number(args[0]))
            fields.append(f"expr{refs[1]}" if len(refs) > 1 and refs[1] >= 0 else _format_complex_number(args[1], args_im[1] if len(args_im) > 1 else 0.0))
        elif op == COEFF_OP_VECTOR_BINARY:
            args = token.get("args") or [0]
            fields.extend([
                _VECTOR_BINARY_NAMES.get(int(token.get("fn_index") or 0), str(token.get("fn_index") or 0)),
                _SELECTOR_NAMES.get(int(token.get("tgt") or 0), str(token.get("tgt") or 0)),
                _SELECTOR_NAMES.get(int(token.get("src") or 0), str(token.get("src") or 0)),
                _SELECTOR_NAMES.get(int(args[0]), str(args[0])),
            ])
        elif op == COEFF_OP_VECTOR_UNARY:
            fields.extend([
                _VECTOR_UNARY_NAMES.get(int(token.get("fn_index") or 0), str(token.get("fn_index") or 0)),
                _SELECTOR_NAMES.get(int(token.get("tgt") or 0), str(token.get("tgt") or 0)),
                _SELECTOR_NAMES.get(int(token.get("src") or 0), str(token.get("src") or 0)),
            ])
        elif op == COEFF_OP_VECTOR_ROLL:
            args = token.get("args") or [0]
            fields.extend([
                _VECTOR_ROLL_NAMES.get(int(token.get("fn_index") or 0), str(token.get("fn_index") or 0)),
                _SELECTOR_NAMES.get(int(token.get("tgt") or 0), str(token.get("tgt") or 0)),
                _SELECTOR_NAMES.get(int(token.get("src") or 0), str(token.get("src") or 0)),
                _format_number(args[0]),
            ])
        elif op == COEFF_OP_VECTOR_ARGSORT:
            args = token.get("args") or [0]
            fields.extend([
                _SELECTOR_NAMES.get(int(token.get("tgt") or 0), str(token.get("tgt") or 0)),
                _SELECTOR_NAMES.get(int(token.get("src") or 0), str(token.get("src") or 0)),
                _SELECTOR_NAMES.get(int(args[0]), str(args[0])),
            ])
        elif op == COEFF_OP_LITTLEWOOD:
            args = token.get("args") or [0, 0, 0]
            args_im = token.get("args_im") or [0, 0, 0]
            refs = token.get("expr_refs") or [-1, -1, -1]
            fields.append(_SELECTOR_NAMES.get(int(token.get("tgt") or 0), str(token.get("tgt") or 0)))
            fields.append(f"expr{refs[0]}" if len(refs) > 0 and refs[0] >= 0 else _format_complex_number(args[0], args_im[0] if len(args_im) > 0 else 0.0))
            fields.append(f"expr{refs[1]}" if len(refs) > 1 and refs[1] >= 0 else _format_complex_number(args[1], args_im[1] if len(args_im) > 1 else 0.0))
            fields.append(f"expr{refs[2]}" if len(refs) > 2 and refs[2] >= 0 else _format_number(args[2] if len(args) > 2 else 0.0))
        elif op in {COEFF_OP_LEGACY, COEFF_OP_NATIVE_TRANSFORM}:
            spec = legacy_registry()["by_index"].get(int(token.get("fn_index") or 0))
            fields.extend([
                spec["name"] if spec else str(token.get("fn_index")),
                _SELECTOR_NAMES.get(int(token.get("src") or 0), str(token.get("src") or 0)),
                _SELECTOR_NAMES.get(int(token.get("tgt") or 0), str(token.get("tgt") or 0)),
            ])
            refs = token.get("expr_refs") or []
            args = token.get("args") or []
            args_im = token.get("args_im") or []
            for idx, arg in enumerate(args):
                if idx < len(refs) and refs[idx] >= 0:
                    fields.append(f"expr{refs[idx]}")
                elif idx < len(args_im) and args_im[idx]:
                    fields.append(_format_complex_number(arg, args_im[idx]))
                else:
                    fields.append(_format_number(arg))
            if int(token.get("andy_expr_ref", -1)) >= 0:
                fields.append(f"andy=expr{int(token.get('andy_expr_ref'))}")
            elif token.get("andy"):
                fields.append(f"andy={_format_number(token.get('andy'))}")
            if int(token.get("stack_arg_count") or 0):
                fields.append(f"stack_args={int(token.get('stack_arg_count') or 0)}")
        parts.append(":".join(fields))
    if scalar_exprs:
        parts.append("exprs=" + json.dumps(scalar_exprs, sort_keys=True, separators=(",", ":")))
    return ";".join(parts)


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
        if int(token.get("op") or 0) != COEFF_OP_LEGACY:
            return False
        if int(token.get("src") or 0) != COEFF_SEL_POLY:
            return False
        if int(token.get("tgt") or 0) != COEFF_SEL_POLY:
            return False
        if any(int(ref) >= 0 for ref in (token.get("expr_refs") or [])):
            return False
        if int(token.get("andy_expr_ref", -1)) >= 0:
            return False
        if int(token.get("fn_index") or 0) == 16:
            args = list(token.get("args") or [])
            if len(args) == 4:
                offset_re = args[2] if len(args) > 2 else 0.0
                offset_im = args[3] if len(args) > 3 else 0.0
                if abs(offset_re) > 1e-12 or abs(offset_im) > 1e-12:
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
        if int(token.get("fn_index") or 0) == 14:
            args = list(token.get("args") or [])
            args_im = list(token.get("args_im") or [])
            if len(args) == 2:
                multiplier = _format_complex_number(args[0], args_im[0] if len(args_im) > 0 else 0.0)
                offset = _format_complex_number(args[1], args_im[1] if len(args_im) > 1 else 0.0)
            else:
                multiplier = _format_complex_number(args[0] if len(args) > 0 else 100.0, args[1] if len(args) > 1 else 0.0)
                offset = _format_complex_number(args[2] if len(args) > 2 else 0.0, args[3] if len(args) > 3 else 0.0)
            entry = ["linear"]
            if multiplier != "100" or offset != "0" or token.get("andy"):
                entry.extend([multiplier, offset])
            if token.get("andy"):
                entry.append(_format_number(token["andy"]))
            out.append(entry)
            continue
        if int(token.get("fn_index") or 0) == 24:
            args = list(token.get("args") or [])
            args_im = list(token.get("args_im") or [])
            if len(args) == 2:
                multiplier = _format_complex_number(args[0], args_im[0] if len(args_im) > 0 else 0.0)
                exponent = _format_complex_number(args[1], args_im[1] if len(args_im) > 1 else 0.0)
                entry = ["pow"]
                if multiplier != "1" or exponent != "1" or token.get("andy"):
                    entry.extend([multiplier, exponent])
                if token.get("andy"):
                    entry.append(_format_number(token["andy"]))
                out.append(entry)
                continue
        if int(token.get("fn_index") or 0) == 16:
            args = list(token.get("args") or [])
            refs = list(token.get("expr_refs") or [])
            if len(args) == 4 and not any(int(ref) >= 0 for ref in refs):
                multiplier_re = args[0] if len(args) > 0 else 1.0
                multiplier_im = args[1] if len(args) > 1 else 0.0
                offset_re = args[2] if len(args) > 2 else 0.0
                offset_im = args[3] if len(args) > 3 else 0.0
                if abs(offset_re) > 1e-12 or abs(offset_im) > 1e-12:
                    return []
                entry = ["exp"]
                if abs(multiplier_re - 1.0) > 1e-12 or abs(multiplier_im) > 1e-12 or token.get("andy"):
                    entry.append(_format_number(multiplier_re))
                if abs(multiplier_im) > 1e-12 or token.get("andy"):
                    entry.append(_format_number(multiplier_im))
                if token.get("andy"):
                    entry.append(_format_number(token["andy"]))
                out.append(entry)
                continue
            return []
        entry = [spec["name"]]
        args = list(token.get("args") or [])
        while args:
            idx = len(args) - 1
            default = spec["args"][idx].get("default", 0) if idx < len(spec["args"]) else 0
            try:
                if abs(float(args[-1]) - float(default)) > 1e-12:
                    break
            except (TypeError, ValueError):
                break
            args.pop()
        for idx, value in enumerate(args):
            if idx < len(spec["args"]) and spec["args"][idx].get("type") == "enum":
                entry.append("lo" if int(value) == 1 else "hi")
            else:
                entry.append(_format_number(value))
        if token.get("andy"):
            entry.append(_format_number(token["andy"]))
        out.append(entry)
    return out


def compile_coeff_program_chain(chain, *, macro_resolver=None, strict=True):
    diagnostics = []
    try:
        source_chain = _canonical_source_chain(chain)
        expanded_chain, macro_count = _expand_macros(source_chain, macro_resolver)
        tokens, scalar_exprs = _lower_chain(expanded_chain)
        stack_info = _validate_stack(tokens)
        diagnostics.extend(stack_info["diagnostics"])
        spec = _execution_spec(tokens, scalar_exprs)
        fast_path = _legacy_fast_path(tokens)
        return {
            "version": PROGRAM_VERSION,
            "program_kind": PROGRAM_KIND,
            "source_chain": source_chain,
            "expanded_chain": expanded_chain,
            "tokens": tokens,
            "execution_tokens": tokens,
            "scalar_exprs": scalar_exprs,
            "execution_spec": spec,
            "fingerprint": _fingerprint(spec),
            "display": display_coeff_program_chain(source_chain),
            "expanded_display": display_coeff_program_chain(expanded_chain),
            "statement_count": len(source_chain),
            "token_count": len(tokens),
            "scalar_expr_count": len(scalar_exprs),
            "stack_max": stack_info["stack_max"],
            "macro_expansions": macro_count,
            "uses_legacy_chain_equivalent": fast_path,
            "legacy_coeff_transforms": _legacy_transforms(tokens),
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
            "execution_tokens": [],
            "scalar_exprs": [],
            "execution_spec": "",
            "fingerprint": "",
            "display": "",
            "expanded_display": "",
            "statement_count": 0,
            "token_count": 0,
            "scalar_expr_count": 0,
            "stack_max": 0,
            "macro_expansions": 0,
            "uses_legacy_chain_equivalent": False,
            "legacy_coeff_transforms": [],
            "diagnostics": diagnostics,
        }


def compile_coeff_program_diagnostics(chain, *, macro_resolver=None):
    return compile_coeff_program_chain(chain, macro_resolver=macro_resolver, strict=False)


def coeff_program_chain_id(chain, *, macro_resolver=None):
    return compile_coeff_program_chain(chain, macro_resolver=macro_resolver)["fingerprint"]
