import json
import cmath
import math
import os
import struct
import subprocess
import sys
import tempfile


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from coeff_program_chain import compile_coeff_program_chain
from coeff_program_source import compile_coeff_program_source


LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")


def _coeff_program_payload(chain):
    return _compiled_coeff_program_payload(compile_coeff_program_chain(chain))


def _compiled_coeff_program_payload(compiled):
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


def _complex_expr(value):
    return f"{value.real}{'+' if value.imag >= 0 else ''}{value.imag}j"


def test_coeff_program_source_set_affine_and_extended_range_run_in_native_coeffgen():
    compiled = compile_coeff_program_source("""
        push_range(0, 5, 2)
        linear(2+1j, 1-1j)
        poly = pop
        emit
    """)
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [3, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _compiled_coeff_program_payload(compiled),
    })

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    expected = [1 - 1j, 5 + 1j, 9 + 3j]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want.real) <= 1e-6
        assert abs(got.imag - want.imag) <= 1e-6


def test_coeff_program_source_linear_matches_native_chain_linear_for_real_args():
    source = compile_coeff_program_source("""
        push_range(0, 4, 1)
        linear(2, 1)
        emit
    """)
    chain = compile_coeff_program_chain([
        ["push_range", "0", "4", "1"],
        ["legacy", "linear", "pop", "push", "2", "1"],
        ["emit"],
    ])
    spec_base = {
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [4, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
    }

    _source_meta, source_data = _run_coeffgen({
        **spec_base,
        "coeff_program": _compiled_coeff_program_payload(source),
    })
    _chain_meta, chain_data = _run_coeffgen({
        **spec_base,
        "coeff_program": _compiled_coeff_program_payload(chain),
    })

    assert source_data == chain_data
    values = _complex_f32_values(source_data)
    expected = [1.0, 3.0, 5.0, 7.0]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_native_accepts_more_than_old_64_token_cap():
    chain = []
    for idx in range(40):
        chain.append(["push_const", "1", str(idx)])
        chain.append(["emit"])
    compiled = compile_coeff_program_chain(chain)
    assert compiled["token_count"] == 80

    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [1, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _compiled_coeff_program_payload(compiled),
    })

    assert meta["coeff_program_tokens"] == 80
    values = _complex_f32_values(data)
    assert len(values) == 1
    assert abs(values[0].real - 39.0) <= 1e-6
    assert abs(values[0].imag) <= 1e-6


def test_coeff_program_source_extended_linspace_runs_in_native_coeffgen():
    compiled = compile_coeff_program_source("""
        push_linspace(2, 8, 4)
        emit
    """)
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [4, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _compiled_coeff_program_payload(compiled),
    })

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    expected = [2.0, 4.0, 6.0, 8.0]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_typed_dynamic_index_and_broadcast_run_in_native_coeffgen():
    compiled = compile_coeff_program_source("""
        poly[poly_len - 1] = p1 + cf[poly_len - 2]
        poly = multiply(poly, p2)
        poly = add(poly, fill(poly_len, p1))
        emit
    """)
    params = [3.0, 0.0, 4.0, 0.0]
    with tempfile.NamedTemporaryFile(prefix="pp_coeff_program_typed_params_", suffix=".bin", delete=False) as fh:
        params_path = fh.name
        fh.write(struct.pack("<" + "f" * len(params), *params))
    try:
        meta, data = _run_coeffgen({
            "mode": "coeffgen_chunked",
            "function": "const",
            "cfpv": [4, 2, 0],
            "params_file": params_path,
            "step_start": 0,
            "source_step_start": 0,
            "source_n1": 1,
            "source_n2": 1,
            "step_count": 1,
            "coeff_transforms": [],
            "coeff_program": _compiled_coeff_program_payload(compiled),
        })
    finally:
        try:
            os.remove(params_path)
        except FileNotFoundError:
            pass

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    expected = [11.0, 11.0, 11.0, 23.0]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_source_push_scalar_and_push_vec_run_in_native_coeffgen():
    compiled = compile_coeff_program_source("""
        push_vec(4, 2)
        push_scalar(3)
        multiply()
        emit
    """)
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [4, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _compiled_coeff_program_payload(compiled),
    })

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    assert len(values) == 4
    for got in values:
        assert abs(got.real - 6.0) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_scalar_elementary_functions_run_in_native_coeffgen():
    compiled = compile_coeff_program_source("""
        poly = push_vec(5, 0)
        poly[0] = sin(p1)
        poly[1] = cos(p2)
        poly[2] = exp(1j*(p1+p2))
        poly[3] = sqrt(-1)
        poly[4] = log(1j)
    """)
    params = [math.pi / 2.0, 0.0, 0.0, 0.0]
    with tempfile.NamedTemporaryFile(prefix="pp_coeff_program_scalar_funcs_", suffix=".bin", delete=False) as fh:
        params_path = fh.name
        fh.write(struct.pack("<" + "f" * len(params), *params))
    try:
        meta, data = _run_coeffgen({
            "mode": "coeffgen_chunked",
            "function": "const",
            "cfpv": [5, 0, 0],
            "params_file": params_path,
            "step_start": 0,
            "source_step_start": 0,
            "source_n1": 1,
            "source_n2": 1,
            "step_count": 1,
            "coeff_transforms": [],
            "coeff_program": _compiled_coeff_program_payload(compiled),
        })
    finally:
        try:
            os.remove(params_path)
        except FileNotFoundError:
            pass

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    expected = [
        1.0 + 0.0j,
        1.0 + 0.0j,
        0.0 + 1.0j,
        0.0 + 1.0j,
        0.0 + (math.pi / 2.0) * 1j,
    ]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want.real) <= 1e-5
        assert abs(got.imag - want.imag) <= 1e-5


def test_coeff_program_chain_scalar_elementary_functions_run_in_native_coeffgen():
    compiled = compile_coeff_program_chain([
        ["push_vec", "3", "sin(p1)+cos(p2)+exp(1j*(p1+p2))"],
        ["emit"],
    ])
    params = [math.pi / 2.0, 0.0, 0.0, 0.0]
    with tempfile.NamedTemporaryFile(prefix="pp_coeff_program_chain_scalar_funcs_", suffix=".bin", delete=False) as fh:
        params_path = fh.name
        fh.write(struct.pack("<" + "f" * len(params), *params))
    try:
        meta, data = _run_coeffgen({
            "mode": "coeffgen_chunked",
            "function": "const",
            "cfpv": [3, 0, 0],
            "params_file": params_path,
            "step_start": 0,
            "source_step_start": 0,
            "source_n1": 1,
            "source_n2": 1,
            "step_count": 1,
            "coeff_transforms": [],
            "coeff_program": _compiled_coeff_program_payload(compiled),
        })
    finally:
        try:
            os.remove(params_path)
        except FileNotFoundError:
            pass

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    for got in values:
        assert abs(got.real - 2.0) <= 1e-5
        assert abs(got.imag - 1.0) <= 1e-5


def test_coeff_program_static_and_dynamic_poly_index_parity_in_native_coeffgen():
    compiled = compile_coeff_program_source("""
        push_range(0, 5, 1)
        emit
        poly[0] = poly[3]
        poly[1] = poly[poly_len - 2]
        emit
    """)
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [5, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _compiled_coeff_program_payload(compiled),
    })

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    expected = [3.0, 3.0, 2.0, 3.0, 4.0]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_new_unary_ops_run_in_native_coeffgen():
    cases = [
        ("neg", 1 + 2j, -1 - 2j),
        ("conj", 1 + 2j, 1 - 2j),
        ("sqrt", 3 + 4j, cmath.sqrt(3 + 4j)),
        ("log", math.e + 0j, 1 + 0j),
    ]
    for op, value, expected in cases:
        meta, data = _run_coeffgen({
            "mode": "coeffgen",
            "function": "const",
            "cfpv": [1, 0, 0],
            "n1": 1,
            "n2": 1,
            "coeff_transforms": [],
            "coeff_program": _coeff_program_payload([
                ["push_const", "1", _complex_expr(value)],
                [op, "poly", "pop"],
                ["emit"],
            ]),
        })
        assert meta["coeff_program_tokens"] == 3
        got = _complex_f32_values(data)[0]
        assert abs(got.real - expected.real) <= 1e-5
        assert abs(got.imag - expected.imag) <= 1e-5


def test_coeff_program_source_native_transform_runs_in_native_coeffgen():
    compiled = compile_coeff_program_source("""
        push_range(0, 4, 1)
        poly = rev(pop)
        emit
    """)
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [4, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _compiled_coeff_program_payload(compiled),
    })

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    expected = [3.0, 2.0, 1.0, 0.0]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_source_typed_blend_runs_in_native_coeffgen():
    compiled = compile_coeff_program_source("""
        fill(3, 1)
        fill(3, 3)
        blend(0.25)
        emit
    """)
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [3, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _compiled_coeff_program_payload(compiled),
    })

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    assert len(values) == 3
    for got in values:
        assert abs(got.real - 1.5) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_source_native_transform_stack_args_run_in_native_coeffgen():
    compiled = compile_coeff_program_source("""
        push_range(0, 2, 1)
        poly = exp_affine(pop, 1j, 0)
        emit
    """)
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [2, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _compiled_coeff_program_payload(compiled),
    })

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    expected = [1 + 0j, cmath.exp(1j)]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want.real) <= 1e-6
        assert abs(got.imag - want.imag) <= 1e-6


def test_coeff_program_source_native_transform_stack_args_preserve_andy_in_native_coeffgen():
    compiled = compile_coeff_program_source("""
        push_range(0, 3, 1)
        poly = exp_affine(pop, 0, 0, 1)
        emit
    """)
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [3, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _compiled_coeff_program_payload(compiled),
    })

    assert meta["coeff_program_tokens"] == compiled["token_count"]
    values = _complex_f32_values(data)
    expected = [0.0, 1.0, 2.0]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want) <= 1e-6
        assert abs(got.imag) <= 1e-6


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


def test_coeff_program_push_const_linspace_and_range_use_poly_len():
    chain = [
        ["push_const", "poly_len", "2+3j"],
        ["emit"],
        ["push_linspace", "poly_len"],
        ["emit"],
        ["push_range", "poly_len"],
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
    expected = [0.0, 1.0, 2.0, 3.0]
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


def test_coeff_program_scalar_expr_reads_source_t1_t2_in_chunked_mode():
    chain = [
        ["poke_poly", "0", "t1 + t2 + p1 + p2"],
    ]
    params = []
    for _ in range(4):
        params.extend([9.0, 0.0, 11.0, 0.0])
    with tempfile.NamedTemporaryFile(prefix="pp_coeff_program_params_", suffix=".bin", delete=False) as fh:
        params_path = fh.name
        fh.write(struct.pack("<" + "f" * len(params), *params))
    try:
        meta, data = _run_coeffgen({
            "mode": "coeffgen_chunked",
            "function": "const",
            "cfpv": [1, 0, 0],
            "params_file": params_path,
            "step_start": 0,
            "source_step_start": 0,
            "source_n1": 2,
            "source_n2": 2,
            "step_count": 4,
            "coeff_transforms": [],
            "coeff_program": _coeff_program_payload(chain),
        })
    finally:
        try:
            os.remove(params_path)
        except FileNotFoundError:
            pass

    assert meta["coeff_program_tokens"] == len(chain)
    values = _complex_f32_values(data)
    expected = [20.0, 20.5, 21.0, 20.5]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_scalar_expr_abs_log_in_native_coeffgen():
    chain = [
        ["push_const", "1", "log(abs(p1+p2)+1)*1j"],
        ["emit"],
    ]
    params = [9.0, 0.0, 11.0, 0.0]
    with tempfile.NamedTemporaryFile(prefix="pp_coeff_program_abs_log_params_", suffix=".bin", delete=False) as fh:
        params_path = fh.name
        fh.write(struct.pack("<" + "f" * len(params), *params))
    try:
        meta, data = _run_coeffgen({
            "mode": "coeffgen_chunked",
            "function": "const",
            "cfpv": [1, 0, 0],
            "params_file": params_path,
            "step_start": 0,
            "source_step_start": 0,
            "source_n1": 1,
            "source_n2": 1,
            "step_count": 1,
            "coeff_transforms": [],
            "coeff_program": _coeff_program_payload(chain),
        })
    finally:
        try:
            os.remove(params_path)
        except FileNotFoundError:
            pass

    assert meta["coeff_program_tokens"] == len(chain)
    values = _complex_f32_values(data)
    assert len(values) == 1
    assert abs(values[0].real) <= 1e-6
    assert abs(values[0].imag - math.log(21.0)) <= 1e-6
