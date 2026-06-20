#!/usr/bin/env python3
"""
Run coefficient-function parity pytest suites and write generated parity metadata.

The generated JSON is consumed by gen_catalog.py and overrides any stale
hand-edited agreement percentages for functions that have real parity coverage.

Usage:
  python3 lambda/gen_parity_results.py
  python3 lambda/gen_parity_results.py --output /tmp/coeff_func_parity.json
  python3 lambda/gen_parity_results.py tests/test_poly164_hand.py
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import os
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
TESTS_DIR = ROOT / "tests"
DEFAULT_OUTPUT = Path(__file__).resolve().parent / "coeff_func_parity.json"

for _path in (ROOT, TESTS_DIR, ROOT / "lambda"):
    _text = str(_path)
    if _text not in sys.path:
        sys.path.insert(0, _text)


def _load_test_module(path: Path):
    module_name = f"_parity_{path.stem}"
    spec = importlib.util.spec_from_file_location(module_name, path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


def discover_parity_tests():
    paths = []
    for path in sorted(TESTS_DIR.glob("test*_hand.py")):
        module = _load_test_module(path)
        funcs = getattr(module, "PARITY_FUNCTIONS", None)
        if funcs:
            paths.append(path)
    return paths


class _ParityCollector:
    def __init__(self):
        self.node_to_func = {}
        self.results = {}

    def pytest_collection_modifyitems(self, session, config, items):
        del session, config
        for item in items:
            func_name = self._func_for_item(item)
            if not func_name:
                continue
            self.node_to_func[item.nodeid] = func_name
            record = self.results.setdefault(func_name, {
                "agreement_pct": 100,
                "parity_verified": True,
                "parity_cases": 0,
                "parity_test_files": set(),
            })
            record["parity_test_files"].add(Path(str(item.fspath)).name)

    def pytest_runtest_logreport(self, report):
        if report.when != "call":
            return
        func_name = self.node_to_func.get(report.nodeid)
        if not func_name:
            return
        record = self.results[func_name]
        record["parity_cases"] += 1
        if report.failed:
            record["agreement_pct"] = 0
            record["parity_verified"] = False

    @staticmethod
    def _func_for_item(item):
        callspec = getattr(item, "callspec", None)
        if callspec is not None and "func_name" in callspec.params:
            return callspec.params["func_name"]

        funcs = getattr(item.module, "PARITY_FUNCTIONS", None)
        if isinstance(funcs, (list, tuple)) and len(funcs) == 1:
            return funcs[0]
        return None


def _normalize_results(results):
    return {
        name: {
            "agreement_pct": data["agreement_pct"],
            "parity_verified": bool(data["parity_verified"]),
            "parity_cases": int(data["parity_cases"]),
            "parity_test_files": sorted(data["parity_test_files"]),
        }
        for name, data in sorted(results.items())
        if data["parity_cases"] > 0
    }


def _parse_args(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "tests",
        nargs="*",
        help="Optional pytest files to run. Default: all discovered parity suites.",
    )
    parser.add_argument(
        "--output",
        default=str(DEFAULT_OUTPUT),
        help=f"Output JSON path (default: {DEFAULT_OUTPUT})",
    )
    return parser.parse_args(argv)


def main(argv=None):
    args = _parse_args(argv or sys.argv[1:])
    try:
        import pytest
    except ImportError as exc:
        print(
            "ERROR: pytest is required to generate coeff parity results. "
            "Use the project virtualenv or `uv run python`.",
            file=sys.stderr,
        )
        raise SystemExit(2) from exc

    test_paths = [Path(p).resolve() for p in args.tests] if args.tests else discover_parity_tests()
    if not test_paths:
        print("ERROR: no parity test files found", file=sys.stderr)
        raise SystemExit(2)

    rel_paths = [os.path.relpath(path, ROOT) for path in test_paths]
    print(f"Running parity suites: {', '.join(rel_paths)}")

    collector = _ParityCollector()
    rc = pytest.main(rel_paths, plugins=[collector])
    if rc != 0:
        raise SystemExit(rc)

    results = _normalize_results(collector.results)
    if not results:
        print("ERROR: parity suites collected no parity-bearing test cases", file=sys.stderr)
        raise SystemExit(2)

    output_path = Path(args.output).resolve()
    output_path.write_text(json.dumps(results, indent=2, sort_keys=True) + "\n")
    print(f"Wrote {output_path}")
    print(f"Parity entries: {len(results)}")


if __name__ == "__main__":
    main()
