#!/usr/bin/env python3
import json
import pathlib
import statistics
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "lambda"))
sys.path.insert(0, str(ROOT / "tests"))

from bench_vm import _compiled_coeff_program_payload, _compiled_param_program_payload, _run_json


BINARY = pathlib.Path("/tmp/polypaint_sweep_vm_o3")
REPS = 11


def bench(payload, workdir, name):
    rows = [_run_json(BINARY, payload, workdir / f"{name}_{i}.bin") for i in range(REPS)]
    values = [int(row["elapsed_us"]) for row in rows]
    return {
        "median_us": statistics.median(values),
        "min_us": min(values),
        "max_us": max(values),
        "tokens": (rows[0].get("param_program_tokens", 0)
                   if rows[0].get("mode") == "param_gen"
                   else rows[0].get("coeff_program_tokens", 0)),
        "rows": rows[0].get("n_steps", rows[0].get("n_t", 0)),
        "degree": rows[0].get("degree"),
    }


def param_payload(program=None):
    payload = {
        "mode": "param_gen",
        "n1": 256,
        "n2": 256,
        "times": 4,
        "n_threads": 1,
    }
    if program is not None:
        payload["param_program"] = _compiled_param_program_payload(program)
    return payload


def param_thread_payload(threads):
    payload = param_payload([
        ["const", "exp(t1*6.283185307179586j)"], ["emit", "p1"],
        ["const", "exp(t2*6.283185307179586j)"], ["emit", "p2"],
    ])
    payload["n_threads"] = threads
    return payload


def coeff_payload(source=None, function="poly_1", n=128):
    payload = {
        "mode": "coeffgen",
        "function": function,
        "cfpv": [5, 1, 0] if function == "const" else [],
        "n1": n,
        "n2": n,
        "times": 1,
    }
    if source is not None:
        payload["coeff_program"] = _compiled_coeff_program_payload(source)
    return payload


def chunked_coeff_payload(params_file, threads, source=None, n=256):
    payload = {
        "mode": "coeffgen_chunked",
        "function": "poly_1",
        "cfpv": [],
        "params_file": str(params_file),
        "step_start": 0,
        "step_count": n * n,
        "source_step_start": 0,
        "source_n1": n,
        "source_n2": n,
        "n_threads": threads,
    }
    if source is not None:
        payload["coeff_program"] = _compiled_coeff_program_payload(source)
    return payload


def main():
    with tempfile.TemporaryDirectory(prefix="cr31_vm_matrix_") as td:
        workdir = pathlib.Path(td)
        params_file = workdir / "params.bin"
        _run_json(BINARY, {
            "mode": "param_gen", "n1": 256, "n2": 256,
            "times": 1, "n_threads": 4,
        }, params_file)
        cases = {
            "param_baseline": param_payload(),
            "param_identity_4tok": param_payload([
                ["push", "t1"], ["emit", "p1"],
                ["push", "t2"], ["emit", "p2"],
            ]),
            "param_arithmetic_8tok": param_payload([
                ["push", "t1"], ["push", "t2"], ["add"], ["emit", "p1"],
                ["push", "t1"], ["push", "t2"], ["subtract"], ["emit", "p2"],
            ]),
            "param_unit_circle_native": param_payload([["unit_circle", "both"]]),
            "param_unit_circle_expr": param_payload([
                ["const", "exp(t1*6.283185307179586j)"], ["emit", "p1"],
                ["const", "exp(t2*6.283185307179586j)"], ["emit", "p2"],
            ]),
            "param_expr_t2": param_thread_payload(2),
            "param_expr_t4": param_thread_payload(4),
            "param_expr_t8": param_thread_payload(8),
            "coeff35_baseline": coeff_payload(),
            "coeff35_rev": coeff_payload("poly = rev(poly)"),
            "coeff35_neg": coeff_payload("poly = neg(poly)"),
            "coeff35_add": coeff_payload("poly = add(poly, poly)"),
            "coeff35_sin": coeff_payload("poly = sin(poly)"),
            "coeff35_sin_rev": coeff_payload("poly = sin(poly)\npoly = rev(poly)"),
            "coeff35_baseline256": coeff_payload(n=256),
            "coeff35_sin256": coeff_payload("poly = sin(poly)", n=256),
            "coeff35_rev16": coeff_payload("\n".join(["poly = rev(poly)"] * 16)),
            "coeff35_neg16": coeff_payload("\n".join(["poly = neg(poly)"] * 16)),
            "coeff4_scalar21": coeff_payload("\n".join([
                "poly[0] = 1",
                "poly[1] = p1 + p2",
                "poly[2] = p1 * p2",
                "poly[3] = sin(p1) + cos(p2)",
                "poly = rev(poly)",
            ]), function="const", n=128),
            "coeff4_assign16": coeff_payload("\n".join([
                "poly[0] = p1 + p2",
            ] * 16), function="const", n=128),
            "coeff4_baseline": coeff_payload(function="const", n=128),
            "chunked35_t1": chunked_coeff_payload(params_file, 1),
            "chunked35_t2": chunked_coeff_payload(params_file, 2),
            "chunked35_t4": chunked_coeff_payload(params_file, 4),
            "chunked35_t8": chunked_coeff_payload(params_file, 8),
            "chunked35_sin_t1": chunked_coeff_payload(params_file, 1, "poly = sin(poly)"),
            "chunked35_sin_t4": chunked_coeff_payload(params_file, 4, "poly = sin(poly)"),
        }
        print(json.dumps({name: bench(payload, workdir, name) for name, payload in cases.items()}, indent=2))


if __name__ == "__main__":
    main()
