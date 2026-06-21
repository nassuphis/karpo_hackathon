"""Root Program source compiler.

Phase 6A source layer for root transforms. The runtime wire remains the
existing ``root_transforms`` JSON file consumed by root_xforms.h; this module
adds a canonical source/program object around that wire.
"""
from __future__ import annotations

import hashlib
import json
import math
import os

from merged_opcodes import MERGED_OP_NATIVE_TRANSFORM
from program_source_core import (
    ProgramSourceError,
    diagnostic,
    diagnostic_from_exception,
    find_top_level_assignment,
    parse_call,
    split_program_statements,
)


V2_SPEC_VERSION = 2
V2_PROGRAM_VERSION = 2
_LAMBDA_DIR = os.path.dirname(os.path.abspath(__file__))
_REGISTRY_PATH = os.path.join(_LAMBDA_DIR, "root_legacy_registry.json")


class RootProgramSourceError(ProgramSourceError):
    """A root source syntax/lowering error with line/column metadata."""


class RootProgramSourceCompileError(ValueError):
    """Strict root source compile failure carrying structured diagnostics."""

    def __init__(self, diagnostics):
        self.diagnostics = list(diagnostics or [])
        message = "; ".join(d.get("message", "compile error") for d in self.diagnostics)
        super().__init__(message or "invalid root program source")


def _load_registry():
    with open(_REGISTRY_PATH, "r", encoding="utf-8") as fh:
        payload = json.load(fh)
    by_name = {}
    by_index = {}
    for item in payload.get("functions") or []:
        if not isinstance(item, dict) or not item.get("name"):
            continue
        spec = dict(item)
        spec["name"] = str(spec["name"]).strip().lower()
        spec["fn_index"] = int(spec.get("fn_index") or 0)
        spec["args"] = list(spec.get("args") or [])
        by_name[spec["name"]] = spec
        by_index[spec["fn_index"]] = spec
    return by_name, by_index


_REGISTRY_BY_NAME = None
_REGISTRY_BY_INDEX = None


def _registry_by_name():
    global _REGISTRY_BY_NAME, _REGISTRY_BY_INDEX
    if _REGISTRY_BY_NAME is None or _REGISTRY_BY_INDEX is None:
        _REGISTRY_BY_NAME, _REGISTRY_BY_INDEX = _load_registry()
    return _REGISTRY_BY_NAME


def _registry_by_index():
    global _REGISTRY_BY_NAME, _REGISTRY_BY_INDEX
    if _REGISTRY_BY_NAME is None or _REGISTRY_BY_INDEX is None:
        _REGISTRY_BY_NAME, _REGISTRY_BY_INDEX = _load_registry()
    return _REGISTRY_BY_INDEX


def _format_number(value):
    num = float(value)
    if num == 0.0:
        num = 0.0
    if num.is_integer():
        return str(int(num))
    return f"{num:g}"


def _coerce_real_arg(value, label="root transform arg"):
    if isinstance(value, bool):
        raise RootProgramSourceError(f"{label} must be a finite real number, got {value!r}")
    try:
        number = float(value)
    except (TypeError, ValueError) as exc:
        raise RootProgramSourceError(f"{label} must be a finite real number, got {value!r}") from exc
    if not math.isfinite(number):
        raise RootProgramSourceError(f"{label} must be finite, got {value!r}")
    if number == 0.0:
        number = 0.0
    return number


def _registry_spec(name_or_index):
    if isinstance(name_or_index, int):
        spec = _registry_by_index().get(name_or_index)
        if spec is not None:
            return spec
    name = str(name_or_index or "").strip().lower()
    return _registry_by_name().get(name)


def _canonical_row(name, args, *, fn_index=0):
    spec = _registry_spec(int(fn_index or 0)) if fn_index else _registry_spec(name)
    if spec is None:
        label = str(name or f"fn_index={fn_index}").strip()
        raise RootProgramSourceError(f"unknown root transform: {label}")
    raw_args = list(args or [])
    spec_args = list(spec.get("args") or [])
    if len(raw_args) > len(spec_args):
        raise RootProgramSourceError(
            f"{spec['name']} takes at most {len(spec_args)} argument(s), got {len(raw_args)}"
        )
    coerced = []
    for idx, value in enumerate(raw_args):
        arg_name = str((spec_args[idx] or {}).get("name") or f"arg{idx}")
        coerced.append(_coerce_real_arg(value, f"{spec['name']} {arg_name}"))
    for idx in range(len(raw_args), len(spec_args)):
        default = (spec_args[idx] or {}).get("default", 0.0)
        coerced.append(_coerce_real_arg(default, f"{spec['name']} default arg{idx}"))
    return {
        "name": str(spec["name"]),
        "fn_index": int(spec["fn_index"]),
        "args": coerced,
    }


def _token_from_row(row):
    args = list(row.get("args") or [])
    token = {
        "op": MERGED_OP_NATIVE_TRANSFORM,
        "registry": "root",
        "fn_index": int(row["fn_index"]),
        "name": str(row["name"]),
        "n_args": len(args),
        "args": [float(arg) for arg in args],
        "args_im": [0.0 for _ in args],
    }
    return {key: value for key, value in token.items() if value not in (None, [], {})}


def _execution_spec(tokens):
    return json.dumps(
        {
            "version": V2_PROGRAM_VERSION,
            "kind": "root",
            "tokens": tokens,
        },
        sort_keys=True,
        separators=(",", ":"),
    )


def _fingerprint(*, execution_spec, chain):
    packed = json.dumps(
        {
            "program_kind": "root",
            "spec_version": V2_SPEC_VERSION,
            "payload": {
                "execution_spec": execution_spec,
                "chain": chain,
            },
        },
        sort_keys=True,
        separators=(",", ":"),
    )
    return f"sha256:{hashlib.sha256(packed.encode('utf-8')).hexdigest()}"


def serialize_root_program_chain(chain):
    return json.dumps(compile_root_program_chain(chain, strict=True)["chain"], separators=(",", ":"))


def display_root_program_chain(chain):
    rows = compile_root_program_chain(chain, strict=True)["chain"]
    parts = []
    for row in rows:
        args = ", ".join(_format_number(arg) for arg in row.get("args") or [])
        parts.append(f"{row['name']}({args})" if args else f"{row['name']}()")
    return ", ".join(parts)


def root_transforms_from_program_chain(chain):
    rows = compile_root_program_chain(chain, strict=True)["chain"]
    out = []
    for row in rows:
        args = [_format_number(arg) for arg in row.get("args") or []]
        out.append([row["name"], *args])
    return out


def root_program_chain_from_transforms(root_transforms):
    return compile_root_program_chain(root_transforms, strict=True)["chain"]


def _root_transform_items(payload):
    if payload in ("", None):
        return []
    raw = payload
    if isinstance(payload, dict):
        raw = (
            payload.get("root_program", {}).get("chain")
            if isinstance(payload.get("root_program"), dict)
            else None
        )
        if raw in ("", None):
            raw = (
                payload.get("root_transforms")
                or payload.get("root_transform_chain")
                or payload.get("chain")
                or []
            )
    if isinstance(raw, str):
        try:
            raw = json.loads(raw)
        except Exception as exc:
            raise RootProgramSourceError(f"root transform JSON is invalid: {exc}") from exc
    if not isinstance(raw, list):
        raise RootProgramSourceError(f"root transform chain must be a list, got {type(raw).__name__}")
    return raw


def _row_from_item(item):
    if isinstance(item, str):
        return _canonical_row(item, [])
    if isinstance(item, dict):
        name = str(item.get("name") or "").strip().lower()
        args = item.get("args")
        if args is None:
            args = item.get("params") or []
        if not isinstance(args, list):
            raise RootProgramSourceError(f"root transform args must be a list, got {args!r}")
        return _canonical_row(name, args, fn_index=int(item.get("fn_index") or 0))
    if isinstance(item, (list, tuple)) and item:
        name = str(item[0] or "").strip().lower()
        return _canonical_row(name, list(item[1:]))
    raise RootProgramSourceError(f"invalid root transform row: {item!r}")


def canonicalize_root_transform_item(item):
    """Return one default-expanded canonical root row from an old/new row."""
    return _row_from_item(item)


def compile_root_program_chain(chain, strict=True):
    diagnostics = []
    rows = []
    for idx, item in enumerate(_root_transform_items(chain)):
        try:
            rows.append(_row_from_item(item))
        except RootProgramSourceError as exc:
            diagnostics.append(
                diagnostic(
                    str(exc),
                    line=idx + 1,
                    column=1,
                    code=getattr(exc, "code", "root_chain_error"),
                )
            )
    if diagnostics and strict:
        raise RootProgramSourceCompileError(diagnostics)
    tokens = [_token_from_row(row) for row in rows]
    execution_spec = _execution_spec(tokens)
    fingerprint = _fingerprint(execution_spec=execution_spec, chain=rows)
    return {
        "chain": rows,
        "root_transforms": [[row["name"], *[_format_number(arg) for arg in row.get("args") or []]] for row in rows],
        "tokens": tokens,
        "execution_spec": execution_spec,
        "fingerprint": fingerprint,
        "display": display_root_program_chain_no_compile(rows),
        "statement_count": len(rows),
        "token_count": len(tokens),
        "diagnostics": [],
        "spec_version": V2_SPEC_VERSION,
    }


def display_root_program_chain_no_compile(rows):
    parts = []
    for row in rows or []:
        args = ", ".join(_format_number(arg) for arg in row.get("args") or [])
        parts.append(f"{row['name']}({args})" if args else f"{row['name']}()")
    return ", ".join(parts)


def root_source_text_from_chain(chain):
    rows = compile_root_program_chain(chain, strict=True)["chain"]
    lines = []
    for row in rows:
        args = ", ".join(_format_number(arg) for arg in row.get("args") or [])
        lines.append(f"{row['name']}({args})" if args else f"{row['name']}()")
    return "\n".join(lines)


def root_source_text_from_payload(payload):
    if not isinstance(payload, dict):
        return None
    raw = payload.get("root_program_source_text")
    if raw is not None and str(raw).strip():
        return str(raw)
    root_program = payload.get("root_program")
    if isinstance(root_program, dict):
        raw = root_program.get("source_text")
        if raw is not None and str(raw).strip():
            return str(raw)
    raw = payload.get("source_text")
    if raw is not None and str(raw).strip():
        return str(raw)
    return None


def _parse_root_call(text, *, line=1, column=1, allow_roots_arg=True):
    call = parse_call(text, error_cls=RootProgramSourceError)
    if not call:
        raise RootProgramSourceError("root statement must be a transform call", line=line, column=column)
    name, args = call
    name = str(name or "").strip().lower()
    stripped = [str(arg).strip() for arg in args]
    if stripped and stripped[0].lower() == "roots":
        if not allow_roots_arg:
            raise RootProgramSourceError(
                "bare root transform calls omit the roots argument",
                line=line,
                column=column,
            )
        stripped = stripped[1:]
    return _canonical_row(name, stripped)


def _lower_statement(stmt):
    text = stmt.text.strip()
    assignment = find_top_level_assignment(text)
    if assignment >= 0:
        lhs = "".join(text[:assignment].split()).lower()
        rhs = text[assignment + 1:].strip()
        if lhs != "roots":
            raise RootProgramSourceError(
                "only roots assignments are supported in root source",
                line=stmt.line,
                column=stmt.column,
            )
        if rhs.lower() == "roots":
            raise RootProgramSourceError("roots = roots is a no-op", line=stmt.line, column=assignment + 2)
        return _parse_root_call(rhs, line=stmt.line, column=assignment + 2, allow_roots_arg=True)
    return _parse_root_call(text, line=stmt.line, column=stmt.column, allow_roots_arg=True)


def parse_root_program_source(source_text, strict=True):
    diagnostics = []
    rows = []
    try:
        statements = split_program_statements(
            source_text,
            error_cls=RootProgramSourceError,
            max_bytes=8192,
        )
    except RootProgramSourceError as exc:
        diagnostics.append(diagnostic_from_exception(exc))
        if strict:
            raise RootProgramSourceCompileError(diagnostics) from exc
        statements = []
    for stmt in statements:
        try:
            rows.append(_lower_statement(stmt))
        except RootProgramSourceError as exc:
            diagnostics.append(
                diagnostic_from_exception(
                    exc,
                    line=getattr(exc, "line", 0) or stmt.line,
                    column=getattr(exc, "column", 0) or stmt.column,
                )
            )
    if diagnostics and strict:
        raise RootProgramSourceCompileError(diagnostics)
    if diagnostics:
        return {
            "chain": [],
            "root_transforms": [],
            "display": "",
            "statement_count": len(statements),
            "diagnostics": diagnostics,
        }
    compiled = compile_root_program_chain(rows, strict=True)
    return {
        "chain": compiled["chain"],
        "root_transforms": compiled["root_transforms"],
        "display": compiled["display"],
        "statement_count": len(statements),
        "diagnostics": [],
    }


def compile_root_program_source(source_text, strict=True):
    parsed = parse_root_program_source(source_text, strict=strict)
    if parsed.get("diagnostics") and not strict:
        return {
            "chain": [],
            "root_transforms": [],
            "tokens": [],
            "execution_spec": "",
            "fingerprint": "",
            "display": parsed.get("display") or "",
            "statement_count": parsed.get("statement_count") or 0,
            "token_count": 0,
            "diagnostics": parsed.get("diagnostics") or [],
            "spec_version": V2_SPEC_VERSION,
        }
    compiled = compile_root_program_chain(parsed["chain"], strict=strict)
    compiled["statement_count"] = parsed["statement_count"]
    compiled["diagnostics"] = list(parsed.get("diagnostics") or []) + list(compiled.get("diagnostics") or [])
    return compiled
