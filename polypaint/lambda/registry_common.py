"""Shared registry metadata helpers for program vocab generators.

This module is intentionally pure: it reads and validates registry-shaped JSON,
but does not import Param/Coeff compilers or runtime constants. Profile-specific
runtime compatibility stays in the profile modules and drift tests.
"""

from __future__ import annotations

import json
import sys


def load_json(path):
    with open(path, "r", encoding="utf-8") as fh:
        return json.load(fh)


def _raise(error_type, message):
    raise error_type(message)


def require_registry_version(payload, label, version=1, error_type=SystemExit):
    actual = payload.get("version")
    if actual != version:
        _raise(error_type, f"{label} registry version must be {version}, got {actual!r}")


def registry_functions(payload, *, sort_by_index=True):
    functions = list(payload.get("functions") or [])
    if sort_by_index:
        return sorted(functions, key=lambda fn: int(fn["fn_index"]))
    return functions


def validate_function_identity(fn, seen_names, seen_indices, label, *, lower=False, error_type=SystemExit):
    name = str(fn.get("name") or "").strip()
    if lower:
        name = name.lower()
    if not name:
        _raise(error_type, f"{label} registry function missing name")
    if name in seen_names:
        _raise(error_type, f"duplicate {label} registry function name: {name}")
    try:
        fn_index = int(fn.get("fn_index"))
    except (TypeError, ValueError):
        _raise(error_type, f"{label} registry function {name} has invalid fn_index")
    if fn_index <= 0:
        _raise(error_type, f"{label} registry function {name} fn_index must be positive")
    if fn_index in seen_indices:
        _raise(error_type, f"duplicate {label} registry fn_index: {fn_index}")
    seen_names.add(name)
    seen_indices.add(fn_index)
    return name, fn_index


def default_text(value):
    if value is None:
        return ""
    if isinstance(value, float) and value.is_integer():
        return str(int(value))
    return str(value)


def normalize_name_set(values):
    return frozenset(str(value).strip() for value in (values or ()) if str(value).strip())


def normalize_name_int_map(values):
    return {
        str(name).strip(): int(value)
        for name, value in (values or {}).items()
        if str(name).strip()
    }


def normalize_name_int_set_map(values):
    return {
        str(name).strip(): frozenset(int(item) for item in (items or ()))
        for name, items in (values or {}).items()
        if str(name).strip()
    }


def require_function_ui_desc(fn, label):
    name = str(fn.get("name") or "")
    ui = dict(fn.get("ui") or {})
    if not ui.get("desc"):
        raise SystemExit(f"{label} function {name} is missing ui.desc")
    return ui


def extract_category_meta(payload, *, paths, label):
    for path in paths:
        current = payload
        for key in path:
            if not isinstance(current, dict) or key not in current:
                current = None
                break
            current = current[key]
        if isinstance(current, dict):
            return current
    raise SystemExit(f"{label} registry category metadata is missing")


def render_js_assignment(var_name, payload, *, header):
    body = json.dumps(payload, indent=2)
    return f"{header}{var_name} = {body};\n"


def check_generated_file(path, rendered, *, label):
    try:
        with open(path, "r", encoding="utf-8") as fh:
            current = fh.read()
    except FileNotFoundError:
        current = ""
    if current != rendered:
        sys.stderr.write(f"FATAL: {label} is stale; regenerate it\n")
        return False
    return True
