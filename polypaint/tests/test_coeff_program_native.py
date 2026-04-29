import json
import cmath
import os
import struct
import subprocess
import sys
import tempfile


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from coeff_program_chain import compile_coeff_program_chain


LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")


def _coeff_program_payload(chain):
    compiled = compile_coeff_program_chain(chain)
    return {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
    }


def _run_coeffgen(spec):
    with tempfile.NamedTemporaryFile(prefix="pp_coeff_program_native_", suffix=".bin", delete=False) as fh:
        out_path = fh.name
    try:
        proc = subprocess.run(
            [SWEEP, out_path],
            input=json.dumps(spec),
            capture_output=True,
            text=True,
            timeout=30,
        )
        assert proc.returncode == 0, proc.stderr
        with open(out_path, "rb") as fh:
            data = fh.read()
        return json.loads(proc.stdout), data
    finally:
        try:
            os.remove(out_path)
        except FileNotFoundError:
            pass


def _complex_f32_values(data):
    values = struct.unpack("<" + "f" * (len(data) // 4), data)
    return [complex(values[i], values[i + 1]) for i in range(0, len(values), 2)]


def test_coeff_program_vector_ops_run_in_native_coeffgen():
    chain = [
        ["push_const", "4", "1"],
        ["poke_tos", "1", "4"],
        ["poke_tos", "2", "2"],
        ["poke_tos", "3", "3"],
        ["emit"],
        ["argsort", "poly", "poly", "poly"],
        ["roll", "poly", "poly", "1"],
        ["rolr", "poly", "poly", "2"],
        ["add", "poly", "poly", "poly"],
        ["subtract", "poly", "poly", "poly"],
        ["push_const", "4", "2"],
        ["emit"],
        ["multiply", "poly", "poly", "poly"],
        ["divide", "poly", "poly", "poly"],
        ["power", "poly", "poly", "poly"],
        ["angle", "poly", "poly"],
        ["push_const", "4", "3+4j"],
        ["emit"],
        ["mod", "poly", "poly"],
        ["abs", "poly", "poly"],
    ]
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [4, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _coeff_program_payload(chain),
    })

    assert meta["coeff_program_tokens"] == len(chain)
    assert meta["n_coeffs"] == 4
    values = _complex_f32_values(data)
    assert len(values) == 4
    for value in values:
        assert abs(value.real - 5.0) <= 1e-6
        assert abs(value.imag) <= 1e-6


def test_coeff_program_littlewood_runs_in_native_coeffgen():
    chain = [
        ["littlewood", "poly", "0", "1", "0"],
    ]
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [8, 9, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _coeff_program_payload(chain),
    })

    assert meta["coeff_program_tokens"] == 1
    assert meta["n_coeffs"] == 8
    values = _complex_f32_values(data)
    assert len(values) == 8
    assert {round(value.real) for value in values}.issubset({0, 1})
    assert all(abs(value.imag) <= 1e-6 for value in values)

    _repeat_meta, repeat_data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [8, 9, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _coeff_program_payload(chain),
    })
    assert repeat_data == data

    chain_andy = [
        ["littlewood", "poly", "0", "1", "1"],
    ]
    _meta, blended_data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [8, 9, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _coeff_program_payload(chain_andy),
    })
    blended = _complex_f32_values(blended_data)
    assert all(abs(value.real - 9.0) <= 1e-6 and abs(value.imag) <= 1e-6 for value in blended)


def test_coeff_program_exp_accepts_complex_multiplier_and_offset():
    chain = [
        ["legacy", "exp", "poly", "poly", "1+1j", "1"],
    ]
    _meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [1, 1, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _coeff_program_payload(chain),
    })

    values = _complex_f32_values(data)
    assert len(values) == 1
    expected = cmath.exp((1 + 0j) * (1 + 1j) + 1)
    assert abs(values[0].real - expected.real) <= 1e-5
    assert abs(values[0].imag - expected.imag) <= 1e-5


def test_coeff_program_push_const_and_linspace_use_poly_len():
    chain = [
        ["push_const", "poly_len", "2+3j"],
        ["emit"],
        ["push_linspace", "poly_len"],
        ["emit"],
    ]
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [4, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _coeff_program_payload(chain),
    })

    assert meta["coeff_program_tokens"] == len(chain)
    assert meta["n_coeffs"] == 4
    values = _complex_f32_values(data)
    expected = [0.0, 4.0 / 3.0, 8.0 / 3.0, 4.0]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_scalar_expr_reads_cf_poly_tos_and_poly_len():
    chain = [
        ["push_const", "4", "3"],
        ["emit"],
        ["push_const", "4", "5"],
        ["poke_poly", "0", "cf1 + poly2 + tos3 + poly_len"],
        ["pop"],
    ]
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [4, 2, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _coeff_program_payload(chain),
    })

    assert meta["coeff_program_tokens"] == len(chain)
    assert meta["n_coeffs"] == 4
    values = _complex_f32_values(data)
    expected = [14.0, 3.0, 3.0, 3.0]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want) <= 1e-6
        assert abs(got.imag) <= 1e-6
