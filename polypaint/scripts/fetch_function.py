#!/usr/bin/env python3
"""
Fetch a single Python function by symbol name.

Examples:

  python3 scripts/fetch_function.py --symbol giga_45
  python3 scripts/fetch_function.py lambda/giga.py --symbol giga_45
  python3 scripts/fetch_function.py --symbol p11b3
"""

from __future__ import annotations

import argparse
import ast
import json
import sys
from pathlib import Path


DEFAULT_COEFF_CATALOG = "lambda/coeff_func_catalog.json"


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def symbol_candidates(symbol: str) -> list[str]:
    candidates = [symbol]
    if not symbol.startswith("poly_"):
        candidates.append(f"poly_{symbol}")
    seen = []
    for candidate in candidates:
        if candidate not in seen:
            seen.append(candidate)
    return seen


def resolve_catalog_source_path(source: str) -> Path | None:
    if not source or source == "unknown":
        return None
    root = repo_root()
    candidates = [
        root / source,
        root / "lambda" / source,
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return None


def load_coeff_catalog_entries(catalog_path: Path, symbol: str) -> list[dict]:
    if not catalog_path.exists():
        return []
    try:
        items = json.loads(catalog_path.read_text(encoding="utf-8"))
    except Exception:
        return []
    wanted = set(symbol_candidates(symbol))
    return [item for item in items if item.get("name") in wanted]


def extract_symbol_source(text: str, symbol: str) -> tuple[str, str]:
    try:
        tree = ast.parse(text)
    except SyntaxError as exc:
        raise ValueError(f"could not parse Python source while looking for symbol {symbol!r}: {exc}") from exc

    wanted = set(symbol_candidates(symbol))
    matches = []
    for node in ast.walk(tree):
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)) and node.name in wanted:
            if getattr(node, "lineno", None) is None or getattr(node, "end_lineno", None) is None:
                continue
            matches.append(node)
    if not matches:
        raise ValueError(f"symbol {symbol!r} not found in Python source")
    matches.sort(key=lambda node: (node.lineno, node.end_lineno))
    node = matches[0]
    lines = text.splitlines()
    snippet = "\n".join(lines[node.lineno - 1:node.end_lineno])
    if text.endswith("\n"):
        snippet += "\n"
    return node.name, snippet


def search_python_sources_for_symbol(symbol: str) -> list[str]:
    root = repo_root()
    wanted = set(symbol_candidates(symbol))
    hits = []
    for path in sorted((root / "lambda").glob("*.py")):
        try:
            text = path.read_text(encoding="utf-8")
        except Exception:
            continue
        try:
            tree = ast.parse(text)
        except SyntaxError:
            continue
        for node in ast.walk(tree):
            if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)) and node.name in wanted:
                hits.append(str(path.relative_to(root)))
                break
    return hits


def resolve_function_search_paths(explicit_path: str | None, symbol: str, catalog_path_str: str) -> list[str]:
    if explicit_path:
        return [explicit_path]

    catalog_entries = load_coeff_catalog_entries(repo_root() / catalog_path_str, symbol)
    resolved = []
    for entry in catalog_entries:
        source = entry.get("source")
        path = resolve_catalog_source_path(source)
        if path is not None and path.suffix == ".py":
            rel = str(path.relative_to(repo_root()))
            if rel not in resolved:
                resolved.append(rel)
    if resolved:
        return resolved

    searched = search_python_sources_for_symbol(symbol)
    if searched:
        return searched
    raise ValueError(f"could not resolve a Python source file for symbol {symbol!r}")


def fetch_function_from_code_sources(
    path_list: list[str],
    symbol: str,
    catalog_path_str: str = DEFAULT_COEFF_CATALOG,
) -> tuple[str, str]:
    if path_list:
        search_paths = path_list
    else:
        search_paths = resolve_function_search_paths(None, symbol, catalog_path_str)
    errors = []
    for path_str in search_paths:
        path = repo_root() / path_str if not Path(path_str).is_absolute() else Path(path_str)
        try:
            text = path.read_text(encoding="utf-8")
        except Exception as exc:
            errors.append(f"{path_str}: {exc}")
            continue
        try:
            resolved_symbol, snippet = extract_symbol_source(text, symbol)
            return f"{path_str}:{resolved_symbol}", snippet
        except ValueError as exc:
            errors.append(f"{path_str}: {exc}")
    joined = "; ".join(errors) if errors else "no code sources provided"
    raise ValueError(f"symbol {symbol!r} not found in provided code sources: {joined}")


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", nargs="?", help="Optional Python source file to search first.")
    parser.add_argument("--symbol", required=True, help="Function name to fetch.")
    parser.add_argument(
        "--coeff-catalog",
        default=DEFAULT_COEFF_CATALOG,
        help=f"Coefficient catalog JSON used for symbol-to-file lookup. Default: {DEFAULT_COEFF_CATALOG}",
    )
    parser.add_argument(
        "--label",
        action="store_true",
        help="Print the resolved file:function label to stderr.",
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_arg_parser()
    args = parser.parse_args(argv)
    try:
        label, snippet = fetch_function_from_code_sources(
            [args.path] if args.path else [],
            args.symbol,
            args.coeff_catalog,
        )
    except ValueError as exc:
        parser.error(str(exc))
    if args.label:
        print(label, file=sys.stderr)
    sys.stdout.write(snippet)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
