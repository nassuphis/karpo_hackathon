#!/usr/bin/env python3
"""Generate solve_score_vocab_js.js from solve_score_chain.py metadata.

The solve-score compiler remains the source of truth for metric names, allowed
sources, stack operations, and output operations. This generated mirror lets the
frontend editor and inserter consume the same vocabulary without hand-copying
the lists into js/07-transform-catalogs.js.

Usage:
    gen_solve_score_vocab.py            write solve_score_vocab_js.js at repo root
    gen_solve_score_vocab.py --check    exit 1 if the file on disk is stale
"""

from __future__ import annotations

import json
import os
import sys

LAMBDA_DIR = os.path.dirname(os.path.abspath(__file__))
JS_OUT = os.path.join(LAMBDA_DIR, "..", "solve_score_vocab_js.js")


def _param_defs(name: str, spec: dict) -> list[dict]:
    count = int(spec.get("params", 0))
    if count <= 0:
        return []
    if name == "omega_cosine":
        return [{"ph": "w", "def": "1"}, {"ph": "phase", "def": "0"}]
    if name == "sawtooth":
        return [{"ph": "mult", "def": "10"}]
    if name == "pow":
        return [{"ph": "pow", "def": "2"}]
    if name == "ema":
        return [{"ph": "alpha", "def": "0.99"}]
    if name == "weighted_sum":
        return [{"ph": "a", "def": "0.5"}, {"ph": "b", "def": "0.5"}]
    if name == "const":
        return [{"ph": "value", "def": "0"}]
    return [{"ph": f"arg{i + 1}", "def": "0"} for i in range(count)]


def _tooltip(kind: str, name: str, spec: dict) -> str:
    explicit = {
        "omega_cosine": "unary stack op: g(u)=0.5*(cos(omega*2*pi*u+phase)+1)",
        "sawtooth": "unary stack op: frac(score*mult)",
        "flip": "unary stack op: 1-score",
        "const": "push a finite constant onto the stack",
        "dup": "duplicate the top stack value",
        "flush": "clear the entire score stack",
        "clamp": "clamp the top stack value to [0,1]",
        "sin": "raw sin(score), radians",
        "cos": "raw cos(score), radians",
        "log": "natural log; invalid inputs become 0",
        "exp": "raw exp(score); overflow becomes 0",
        "pow": "raw pow(score, exponent)",
        "add": "raw a+b",
        "mult": "raw a*b",
        "subtract": "raw a-b; top of stack is b",
        "ratio": "raw a/b; zero if denominator is zero",
        "ema": "a*alpha + b*(1-alpha), b is top/latest",
        "emit": "pop one score; raw/norm emit a byte, none discards it for debugging branches",
        "emit_norm": "legacy alias for emit(norm)",
        "emit_none": "legacy alias for emit(none)",
    }
    if name in explicit:
        return explicit[name]
    if kind == "combine":
        return f"stack {int(spec.get('arity', 2))} -> 1"
    if kind == "unary":
        arity = int(spec.get("arity", 1))
        return f"stack {arity} -> {arity}"
    if kind == "output":
        return "stack 1 -> output"
    return ""


def _base_metric(metric: str = "proximity", source: str = "slv") -> str:
    return f"metric({metric}, {source}, q=0.1%)"


def _metric_snippet(name: str, allowed_sources: dict) -> str:
    sources = list(allowed_sources.get(name) or ["slv"])
    source = "slv" if "slv" in sources else sources[0]
    return f"score = {_base_metric(name, source)}"


def _op_snippet(kind: str, name: str, spec: dict) -> str:
    if kind == "stack":
        if name == "const":
            return f"score = add({_base_metric()}, const(0))"
        if name == "dup":
            return f"push({_base_metric()})\ndup()\nflush()\nscore = {_base_metric()}"
        if name == "flush":
            return f"push({_base_metric()})\nflush()\nscore = {_base_metric()}"
    if kind == "output":
        if name == "emit":
            return f"emit({_base_metric()})"
        if name == "emit_none":
            return f"emit_none({_base_metric()})\nemit_norm({_base_metric()})"
        return f"{name}({_base_metric()})"
    if kind == "combine":
        metrics = ["proximity", "spread", "crowding", "area"]
        args = [
            _base_metric(metrics[idx] if idx < len(metrics) else "proximity")
            for idx in range(int(spec.get("arity", 2)))
        ]
        args.extend(str(p["def"]) for p in _param_defs(name, spec))
        return f"score = {name}({', '.join(args)})"
    args = [_base_metric()]
    args.extend(str(p["def"]) for p in _param_defs(name, spec))
    return f"score = {name}({', '.join(args)})"


def build_vocab() -> dict:
    sys.path.insert(0, LAMBDA_DIR)
    from solve_score_chain import (  # pylint: disable=import-error,import-outside-toplevel
        COMBINE_CHIPS,
        GENERIC_METRIC_CHIP_NAME,
        GENERIC_METRIC_PUBLIC_NAME,
        GENERIC_METRIC_SOURCES,
        OUTPUT_CHIPS,
        PARAM_CAPABLE_SOLVE_SCORE_METRICS,
        PARAM_SOLVE_SCORE_METRICS,
        SOLVE_SCORE_QUANTILE_PERCENT_MAX,
        SOLVE_SCORE_QUANTILE_PERCENT_MIN,
        STACK_CHIPS,
        UNARY_CHIPS,
        VALID_SOLVE_SCORE_LAG_DEPTHS,
        VALID_SOLVE_SCORE_METRICS,
        VALID_SOLVE_SCORE_SOURCES,
        _METRIC_ALLOWED_SOURCES,
    )

    metric_names = sorted(VALID_SOLVE_SCORE_METRICS)
    param_metrics = sorted(PARAM_SOLVE_SCORE_METRICS)
    param_capable_metrics = sorted(PARAM_CAPABLE_SOLVE_SCORE_METRICS)
    source_rank = {"slv": 0, "cf": 1, "pm": 2}
    allowed_sources = {
        name: sorted(_METRIC_ALLOWED_SOURCES[name], key=lambda item: (source_rank.get(item, 99), item))
        for name in metric_names
    }
    generic_metric_names = [
        name
        for name in metric_names
        if GENERIC_METRIC_SOURCES.issubset(set(allowed_sources[name]))
    ]

    def _op_specs(kind: str, specs: dict) -> dict:
        out = {}
        for name in sorted(specs):
            spec = dict(specs[name])
            entry = {
                "arity": int(spec.get("arity", 1)),
                "params": _param_defs(name, spec),
                "tooltip": _tooltip(kind, name, spec),
                "snippet": _op_snippet(kind, name, spec),
            }
            if "delta" in spec:
                entry["delta"] = spec["delta"]
            out[name] = entry
        return out

    output_specs = {}
    for name in sorted(OUTPUT_CHIPS):
        spec = OUTPUT_CHIPS[name]
        entry = {
            "arity": 1,
            "params": [],
            "tooltip": _tooltip("output", name, spec),
            "snippet": _op_snippet("output", name, spec),
        }
        if name == "emit":
            entry["params"] = [{"ph": "mode", "def": "norm", "choices": ["raw", "norm", "none"]}]
        if spec.get("legacy_alias"):
            entry["legacy_alias"] = list(spec["legacy_alias"])
        output_specs[name] = entry

    return {
        "metricNames": metric_names,
        "paramMetricNames": param_metrics,
        "paramCapableMetricNames": param_capable_metrics,
        "allowedSourcesByMetric": allowed_sources,
        "sourceNames": sorted(VALID_SOLVE_SCORE_SOURCES, key=lambda item: (source_rank.get(item, 99), item)),
        "quantilePercentRange": [
            float(SOLVE_SCORE_QUANTILE_PERCENT_MIN),
            float(SOLVE_SCORE_QUANTILE_PERCENT_MAX),
        ],
        "lagDepths": sorted(int(v) for v in VALID_SOLVE_SCORE_LAG_DEPTHS),
        "starterSnippets": [
            {"label": "score = proximity", "snippet": f"score = {_base_metric()}"},
            {"label": "emit_norm proximity", "snippet": f"emit_norm({_base_metric()})"},
            {
                "label": "two channels",
                "snippet": f"emit_norm({_base_metric()})\nemit_norm({_base_metric('spread')})",
            },
        ],
        "metricSnippets": {
            name: _metric_snippet(name, allowed_sources)
            for name in metric_names
        },
        "genericMetricPublicName": GENERIC_METRIC_PUBLIC_NAME,
        "genericMetricChipName": GENERIC_METRIC_CHIP_NAME,
        "genericMetricSources": sorted(GENERIC_METRIC_SOURCES, key=lambda item: (source_rank.get(item, 99), item)),
        "genericMetricNames": generic_metric_names,
        "unarySpecs": _op_specs("unary", UNARY_CHIPS),
        "combineSpecs": _op_specs("combine", COMBINE_CHIPS),
        "outputSpecs": output_specs,
        "stackSpecs": _op_specs("stack", STACK_CHIPS),
    }


def render_js() -> str:
    body = json.dumps(build_vocab(), indent=2, sort_keys=False)
    return (
        "// AUTO-GENERATED by lambda/gen_solve_score_vocab.py from solve_score_chain.py — do not edit\n"
        "// Solve-score metric/source/op vocabulary shared with the frontend.\n"
        f"window._solveScoreVocab = {body};\n"
    )


def main(argv: list[str]) -> int:
    rendered = render_js()
    if "--check" in argv[1:]:
        try:
            with open(JS_OUT, "r", encoding="utf-8") as fh:
                on_disk = fh.read()
        except FileNotFoundError:
            on_disk = ""
        if on_disk != rendered:
            sys.stderr.write(
                "FATAL: solve_score_vocab_js.js is stale; regenerate with"
                " lambda/gen_solve_score_vocab.py\n"
            )
            return 1
        print("solve_score_vocab_js.js: OK")
        return 0
    with open(JS_OUT, "w", encoding="utf-8") as fh:
        fh.write(rendered)
    print(f"Wrote {os.path.normpath(JS_OUT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
