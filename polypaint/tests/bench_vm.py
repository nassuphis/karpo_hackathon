#!/usr/bin/env python3
"""CR18 DD6 benchmark harness.

This is intentionally not a pytest test: timing gates are host-sensitive. It
writes a report under reports/ and can compare the current run to a saved
baseline with a configurable regression threshold.
"""
from __future__ import annotations

import argparse
import json
import pathlib
import statistics
import subprocess
import sys
import tempfile
import time

ROOT = pathlib.Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
REPORTS_DIR = ROOT / "reports"
sys.path.insert(0, str(ROOT / "tests"))
sys.path.insert(0, str(LAMBDA_DIR))

from oracle_runner import compile_sweep_binary, run_case


def _run_json(binary, payload, out_path):
    result = subprocess.run(
        [str(binary), str(out_path)],
        input=json.dumps(payload, sort_keys=True, separators=(",", ":")),
        capture_output=True,
        text=True,
        timeout=120,
    )
    if result.returncode != 0:
        raise RuntimeError(f"benchmark command failed: {result.stderr}")
    return json.loads(result.stdout.strip().splitlines()[-1])


def _compiled_param_program_payload(chain):
    from param_program_chain import compile_param_program_chain

    compiled = compile_param_program_chain(chain)
    payload = {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "display": compiled["display"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "uses_legacy_fast_path": compiled["uses_legacy_fast_path"],
        "tokens": compiled["tokens"],
    }
    if compiled.get("scalar_exprs"):
        payload["scalar_exprs"] = compiled["scalar_exprs"]
    return payload


def _compiled_coeff_program_payload(source_text):
    from coeff_program_source import compile_coeff_program_source

    compiled = compile_coeff_program_source(source_text)
    return {
        "version": compiled["version"],
        "fingerprint": compiled["fingerprint"],
        "display": compiled["display"],
        "stack_max": compiled["stack_max"],
        "token_count": compiled["token_count"],
        "scalar_expr_count": compiled["scalar_expr_count"],
        "uses_legacy_chain_equivalent": compiled["uses_legacy_chain_equivalent"],
        "tokens": compiled["tokens"],
        "scalar_exprs": compiled["scalar_exprs"],
    }


def _time_func(fn, reps):
    values = []
    last = None
    for _ in range(reps):
        t0 = time.perf_counter()
        last = fn()
        values.append((time.perf_counter() - t0) * 1_000_000.0)
    return {"median_us": statistics.median(values), "min_us": min(values), "max_us": max(values), "last": last}


def _bench_native_json(binary, payload, workdir, name, reps):
    metas = []
    for i in range(max(1, reps)):
        metas.append(_run_json(binary, payload, workdir / f"{name}_{i}.bin"))
    elapsed = [float(meta.get("elapsed_us", 0.0) or 0.0) for meta in metas]
    median = statistics.median(elapsed)
    chosen = dict(metas[min(range(len(elapsed)), key=lambda idx: abs(elapsed[idx] - median))])
    chosen["elapsed_us"] = median
    chosen["min_us"] = min(elapsed)
    chosen["max_us"] = max(elapsed)
    chosen["reps"] = max(1, reps)
    return chosen


def _bench_root_case(binary, case, workdir, reps):
    metas = []
    for i in range(max(1, reps)):
        run_dir = workdir / f"root_{i}"
        run_dir.mkdir(parents=True, exist_ok=True)
        result = run_case(binary, case, run_dir)
        metas.append(result["solve_meta"])
    elapsed = [float(meta.get("elapsed_us", 0.0) or 0.0) for meta in metas]
    median = statistics.median(elapsed)
    chosen = dict(metas[min(range(len(elapsed)), key=lambda idx: abs(elapsed[idx] - median))])
    chosen["elapsed_us"] = median
    chosen["min_us"] = min(elapsed)
    chosen["max_us"] = max(elapsed)
    chosen["reps"] = max(1, reps)
    return chosen


def _bench_solve_score_python(reps):
    from solve_score_chain import compile_solve_score_chain
    from solve_score_eval import eval_solve_score

    compiled = compile_solve_score_chain([
        ["crowding", "slv", "1"],
        ["spread", "cf", "2"],
        ["weighted_sum", "0.6", "0.4"],
        ["emit", "raw"],
        ["t1_abs", "pm", "1"],
        ["emit", "raw"],
        ["proximity", "slv", "1"],
        ["emit", "raw"],
    ])
    metrics = []
    for metric in compiled["metrics"]:
        metrics.append({**metric, "clip_lo": -10.0, "clip_hi": 10.0})
    compiled = {**compiled, "metrics": metrics}
    roots = [(0.0, 0.0), (1.0, 0.0), (0.25, 0.75), (-0.5, 0.4)]
    coeff_roots = [(-1.0, 0.0), (0.5, 0.2), (0.1, -0.6)]
    param_values = [(0.2, -0.3), (0.7, 0.1)]
    return _time_func(lambda: eval_solve_score(compiled, roots, coeff_roots=coeff_roots, param_values=param_values).outputs, reps)


def run_benchmarks(reps=5):
    with tempfile.TemporaryDirectory() as td:
        workdir = pathlib.Path(td)
        binary = compile_sweep_binary(workdir / "sweep_bench")
        results = {}

        param_payload = {
            "mode": "param_gen",
            "n1": 128,
            "n2": 128,
            "times": 4,
            "n_threads": 1,
            "param_program": _compiled_param_program_payload([
                ["const", "exp(t1*6.283185307179586j)"],
                ["emit", "p1"],
                ["const", "exp(t2*6.283185307179586j)"],
                ["emit", "p2"],
            ]),
        }
        results["param_only"] = _bench_native_json(binary, param_payload, workdir, "param", reps)

        scalar_source = "\n".join([
            "poly[0] = 1",
            "poly[1] = p1 + p2",
            "poly[2] = p1 * p2",
            "poly[3] = sin(p1) + cos(p2)",
            "poly = rev(poly)",
        ])
        scalar_payload = {
            "mode": "coeffgen",
            "function": "const",
            "cfpv": [5, 1, 0],
            "n1": 128,
            "n2": 128,
            "times": 1,
            "coeff_program": _compiled_coeff_program_payload(scalar_source),
        }
        results["coeff_scalar"] = _bench_native_json(binary, scalar_payload, workdir, "coeff_scalar", reps)

        vector_payload = {
            "mode": "coeffgen",
            "function": "poly_1",
            "cfpv": [],
            "n1": 64,
            "n2": 64,
            "times": 1,
            "coeff_program": _compiled_coeff_program_payload("poly = sin(poly)\npoly = rev(poly)"),
        }
        results["coeff_vector"] = _bench_native_json(binary, vector_payload, workdir, "coeff_vector", reps)

        root_case = {
            "name": "root_raster",
            "coeffgen": {
                "mode": "coeffgen",
                "function": "poly_1",
                "cfpv": [],
                "n1": 24,
                "n2": 24,
                "times": 1,
                "coeff_transforms": ["rev"],
            },
            "solve": {"match_roots": True},
        }
        results["root_raster"] = _bench_root_case(binary, root_case, workdir, reps)
        results["solve_score_multi_channel_python"] = _bench_solve_score_python(max(10, reps * 10))
        return results


def _compare_against_baseline(report, baseline, threshold):
    failures = []
    for key, current in report["benchmarks"].items():
        old = (baseline.get("benchmarks") or {}).get(key)
        if not old:
            continue
        current_us = float(current.get("elapsed_us", current.get("median_us", 0.0)) or 0.0)
        old_us = float(old.get("elapsed_us", old.get("median_us", 0.0)) or 0.0)
        if old_us <= 0.0 or current_us <= 0.0:
            continue
        ratio = current_us / old_us
        if ratio > 1.0 + threshold:
            failures.append(f"{key}: {current_us:.1f}us vs baseline {old_us:.1f}us ({ratio:.3f}x)")
    return failures


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--reps", type=int, default=5)
    parser.add_argument("--output", type=pathlib.Path, default=REPORTS_DIR / "vm_bench_latest.json")
    parser.add_argument("--write-baseline", type=pathlib.Path)
    parser.add_argument("--against-baseline", type=pathlib.Path)
    parser.add_argument("--threshold", type=float, default=0.10)
    args = parser.parse_args(argv)

    report = {"version": 1, "benchmarks": run_benchmarks(reps=args.reps)}
    REPORTS_DIR.mkdir(parents=True, exist_ok=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if args.write_baseline:
        args.write_baseline.parent.mkdir(parents=True, exist_ok=True)
        args.write_baseline.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if args.against_baseline:
        baseline = json.loads(args.against_baseline.read_text(encoding="utf-8"))
        failures = _compare_against_baseline(report, baseline, args.threshold)
        if failures:
            for failure in failures:
                print(f"BENCH REGRESSION: {failure}", file=sys.stderr)
            return 1
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
