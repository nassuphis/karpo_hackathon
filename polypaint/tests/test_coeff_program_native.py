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
from merged_opcodes import MERGED_OP_NATIVE_TRANSFORM


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


def _run_coeffgen_process(spec):
    with tempfile.NamedTemporaryFile(prefix="pp_coeff_program_native_", suffix=".bin", delete=False) as fh:
        out_path = fh.name
    try:
        return subprocess.run(
            [SWEEP, out_path],
            input=json.dumps(spec),
            capture_output=True,
            text=True,
            timeout=30,
        )
    finally:
        try:
            os.remove(out_path)
        except FileNotFoundError:
            pass


def _run_chunked_with_params(coeff_program, params, *, cfpv, source_n1=1, source_n2=1, step_count=1):
    """Run coeffgen_chunked with a float32 params file.

    coeff_program is the payload dict from _coeff_program_payload /
    _compiled_coeff_program_payload; params is the flat [p1r, p1i, p2r, p2i,
    ...] array the chunked mode reads per step.
    """
    with tempfile.NamedTemporaryFile(prefix="pp_coeff_program_params_", suffix=".bin", delete=False) as fh:
        params_path = fh.name
        fh.write(struct.pack("<" + "f" * len(params), *params))
    try:
        return _run_coeffgen({
            "mode": "coeffgen_chunked",
            "function": "const",
            "cfpv": list(cfpv),
            "params_file": params_path,
            "step_start": 0,
            "source_step_start": 0,
            "source_n1": source_n1,
            "source_n2": source_n2,
            "step_count": step_count,
            "coeff_transforms": [],
            "coeff_program": coeff_program,
        })
    finally:
        try:
            os.remove(params_path)
        except FileNotFoundError:
            pass


def _complex_f32_values(data):
    values = struct.unpack("<" + "f" * (len(data) // 4), data)
    return [complex(values[i], values[i + 1]) for i in range(0, len(values), 2)]


def _complex_expr(value):
    return f"{value.real}{'+' if value.imag >= 0 else ''}{value.imag}j"


def test_coeffgen_rejects_native_coeff_transforms():
    proc = _run_coeffgen_process({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [3, 1, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [["rev"]],
    })
    assert proc.returncode != 0
    assert "coeff_transforms is no longer accepted by the native runtime" in proc.stderr


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


def test_coeff_program_native_accepts_missing_v1_version_and_v2_equivalent_payloads():
    compiled = compile_coeff_program_chain([["push_const", "1", "7"], ["emit"]])
    payload = _compiled_coeff_program_payload(compiled)
    payload_without_version = dict(payload)
    payload_without_version.pop("version", None)
    base = {
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [1, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
    }

    meta, data = _run_coeffgen({**base, "coeff_program": payload_without_version})
    assert meta["coeff_program_tokens"] == compiled["token_count"]
    assert _complex_f32_values(data)[0] == complex(7.0, 0.0)

    v2_payload = dict(payload)
    v2_payload["version"] = 2
    meta, data = _run_coeffgen({**base, "coeff_program": v2_payload})
    assert meta["coeff_program_tokens"] == compiled["token_count"]
    assert _complex_f32_values(data)[0] == complex(7.0, 0.0)

    rev = compile_coeff_program_chain([["push", "cf"], ["_native_transform", "rev", "pop", "push"], ["emit"]])
    rev_payload = _compiled_coeff_program_payload(rev)
    rev_payload["version"] = 2
    for token in rev_payload["tokens"]:
        if token["op"] == MERGED_OP_NATIVE_TRANSFORM:
            token["registry"] = "coeff"
    meta, data = _run_coeffgen({**base, "cfpv": [3, 1, 0], "coeff_program": rev_payload})
    values = _complex_f32_values(data)
    assert [round(v.real) for v in values] == [1, 1, 1]

    bad_payload = dict(rev_payload)
    bad_payload["tokens"] = [dict(t) for t in rev_payload["tokens"]]
    for token in bad_payload["tokens"]:
        if token["op"] == MERGED_OP_NATIVE_TRANSFORM:
            token["registry"] = "param"
    proc = _run_coeffgen_process({**base, "coeff_program": bad_payload})
    assert proc.returncode != 0
    assert "registry must be coeff" in proc.stderr


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
    meta, data = _run_chunked_with_params(_compiled_coeff_program_payload(compiled), params, cfpv=[4, 2, 0])

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
    meta, data = _run_chunked_with_params(_compiled_coeff_program_payload(compiled), params, cfpv=[5, 0, 0])

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
    meta, data = _run_chunked_with_params(_compiled_coeff_program_payload(compiled), params, cfpv=[3, 0, 0])

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


def _run_chain_values(chain, *, n_coeffs=4):
    meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [n_coeffs, 0, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _coeff_program_payload(chain),
    })
    return meta, _complex_f32_values(data)


# Build poly = [1, 4, 2, 3] (distinct, unsorted, so reorders are observable).
_VEC_SETUP = [
    ["push_const", "4", "1"],
    ["poke_tos", "1", "4"],
    ["poke_tos", "2", "2"],
    ["poke_tos", "3", "3"],
    ["emit"],
]


def test_coeff_program_vector_roll_ops_native_values():
    _meta, rolled = _run_chain_values(_VEC_SETUP + [["roll", "poly", "poly", "1"]])
    assert [round(v.real) for v in rolled] == [4, 2, 3, 1]
    _meta, rolled_right = _run_chain_values(_VEC_SETUP + [["rolr", "poly", "poly", "1"]])
    assert [round(v.real) for v in rolled_right] == [3, 1, 4, 2]
    # Shift wraps modulo the vector length.
    _meta, wrapped = _run_chain_values(_VEC_SETUP + [["roll", "poly", "poly", "5"]])
    assert [round(v.real) for v in wrapped] == [4, 2, 3, 1]


def test_coeff_program_argsort_native_values():
    _meta, values = _run_chain_values(_VEC_SETUP + [["argsort", "poly", "poly", "poly"]])
    assert [round(v.real) for v in values] == [1, 2, 3, 4]


def test_coeff_program_vector_binary_ops_native_values():
    second = [
        ["push_const", "4", "10"],
        ["poke_tos", "1", "20"],
        ["poke_tos", "2", "30"],
        ["poke_tos", "3", "40"],
    ]
    cases = {
        "add": [11.0, 24.0, 32.0, 43.0],
        "subtract": [-9.0, -16.0, -28.0, -37.0],
        "multiply": [10.0, 80.0, 60.0, 120.0],
        "divide": [0.1, 0.2, 2.0 / 30.0, 3.0 / 40.0],
    }
    for op, expected in cases.items():
        _meta, values = _run_chain_values(_VEC_SETUP + second + [[op, "poly", "poly", "pop"]])
        assert len(values) == 4, op
        for got, want in zip(values, expected):
            assert abs(got.real - want) <= 1e-6, op
            assert abs(got.imag) <= 1e-6, op
    power_chain = [
        ["push_const", "4", "2"],
        ["poke_tos", "1", "3"],
        ["poke_tos", "2", "4"],
        ["poke_tos", "3", "5"],
        ["emit"],
        ["push_const", "4", "2"],
        ["power", "poly", "poly", "pop"],
    ]
    _meta, values = _run_chain_values(power_chain)
    assert [round(v.real) for v in values] == [4, 9, 16, 25]


def test_coeff_program_vector_unary_elementary_ops_native_values():
    z = complex(0.5, 0.25)
    cases = {
        "angle": complex(cmath.phase(z), 0.0),
        "real": complex(z.real, 0.0),
        "imag": complex(z.imag, 0.0),
        "exp": cmath.exp(z),
        "sin": cmath.sin(z),
        "cos": cmath.cos(z),
        "tan": cmath.tan(z),
        "sinh": cmath.sinh(z),
        "cosh": cmath.cosh(z),
        "tanh": cmath.tanh(z),
        "neg": -z,
        "conj": z.conjugate(),
        "sqrt": cmath.sqrt(z),
        "log": cmath.log(z),
    }
    for op, want in cases.items():
        chain = [
            ["push_const", "2", _complex_expr(z)],
            ["emit"],
            [op, "poly", "poly"],
        ]
        _meta, values = _run_chain_values(chain, n_coeffs=2)
        assert len(values) == 2, op
        for got in values:
            assert abs(got.real - want.real) <= 1e-6, op
            assert abs(got.imag - want.imag) <= 1e-6, op


def test_coeff_program_typed_unary_runs_in_native_coeffgen():
    # Bare sin() operates on the typed stack top (COEFF_OP_TYPED_UNARY),
    # a different opcode from the vector-unary chip exercised above.
    compiled = compile_coeff_program_source("""
        cf
        sin()
        poly = pop
    """)
    _meta, data = _run_coeffgen({
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [3, 2, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
        "coeff_program": _compiled_coeff_program_payload(compiled),
    })
    values = _complex_f32_values(data)
    want = cmath.sin(2.0 + 0.0j)
    assert len(values) == 3
    for got in values:
        assert abs(got.real - want.real) <= 1e-6
        assert abs(got.imag - want.imag) <= 1e-6


def test_coeff_program_affine_chip_runs_in_native_coeffgen():
    chain = [
        ["push_const", "4", "3"],
        ["emit"],
        ["affine", "poly", "poly", "2+1j", "1"],
    ]
    _meta, values = _run_chain_values(chain)
    want = 3.0 * complex(2.0, 1.0) + 1.0
    assert len(values) == 4
    for got in values:
        assert abs(got.real - want.real) <= 1e-6
        assert abs(got.imag - want.imag) <= 1e-6


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
    # Seed is fingerprint-derived and deterministic; both values must appear
    # (an all-zeros regression used to pass the old subset-only assertion).
    assert {round(value.real) for value in values} == {0, 1}
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


def test_coeff_program_push_const_with_poly_len_native_values():
    _meta, values = _run_chain_values([["push_const", "poly_len", "2+3j"], ["emit"]])
    assert len(values) == 4
    for got in values:
        assert abs(got.real - 2.0) <= 1e-6
        assert abs(got.imag - 3.0) <= 1e-6


def test_coeff_program_push_linspace_single_arg_native_values():
    # Single-arg linspace: length samples from 0 to length inclusive,
    # i.e. start=0, stop=len, step len/(len-1).
    _meta, values = _run_chain_values([["push_linspace", "poly_len"], ["emit"]])
    expected = [0.0, 4.0 / 3.0, 8.0 / 3.0, 4.0]
    assert len(values) == len(expected)
    for got, want in zip(values, expected):
        assert abs(got.real - want) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_push_range_with_poly_len_native_values():
    _meta, values = _run_chain_values([["push_range", "poly_len"], ["emit"]])
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
    meta, data = _run_chunked_with_params(
        _coeff_program_payload(chain), params, cfpv=[1, 0, 0], source_n1=2, source_n2=2, step_count=4
    )

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
    meta, data = _run_chunked_with_params(_coeff_program_payload(chain), params, cfpv=[1, 0, 0])

    assert meta["coeff_program_tokens"] == len(chain)
    values = _complex_f32_values(data)
    assert len(values) == 1
    assert abs(values[0].real) <= 1e-6
    assert abs(values[0].imag - math.log(21.0)) <= 1e-6


def _run_source_with_params(source_text, params, *, cfpv):
    compiled = compile_coeff_program_source(source_text)
    meta, data = _run_chunked_with_params(_compiled_coeff_program_payload(compiled), params, cfpv=cfpv)
    return meta, _complex_f32_values(data)


def test_coeff_program_dynamic_scalar_elementary_ops_match_cmath():
    # Every expression references p1, so nothing folds statically: this is
    # the C COEFF_EXPR_* evaluator running, not the Python compiler.
    p1 = 0.5
    _meta, values = _run_source_with_params(
        """
        poly = push_vec(12, 0)
        poly[0] = sqrt(-4*p1)
        poly[1] = 1/(p1*2)
        poly[2] = conj(p1+1j)
        poly[3] = -(p1)
        poly[4] = real(p1+2j)
        poly[5] = imag(p1*1j+2j)
        poly[6] = tan(p1)
        poly[7] = sinh(p1)
        poly[8] = cosh(p1)
        poly[9] = tanh(p1)
        poly[10] = angle(p1*1j)
        poly[11] = log(p1+1j)
        """,
        [p1, 0.0, 0.0, 0.0],
        cfpv=[12, 0, 0],
    )
    expected = [
        cmath.sqrt(-4 * p1),
        1.0 / (p1 * 2),
        complex(p1, -1.0),
        complex(-p1, 0.0),
        complex(p1, 0.0),
        complex(p1 + 2.0, 0.0),
        cmath.tan(p1),
        cmath.sinh(p1),
        cmath.cosh(p1),
        cmath.tanh(p1),
        complex(math.atan2(p1, 0.0), 0.0),
        cmath.log(complex(p1, 1.0)),
    ]
    assert len(values) == len(expected)
    for idx, (got, want) in enumerate(zip(values, expected)):
        assert abs(got.real - want.real) <= 1e-5, idx
        assert abs(got.imag - want.imag) <= 1e-5, idx


def test_coeff_program_dynamic_division_and_sqrt_extremes_match_python_folds():
    # Tiny denominators and magnitudes used to silently collapse to zero in
    # the C VM (c_div's 1e-15 cutoff; c_powr's 1e-30 magnitude cutoff) while
    # the Python compiler folded the same text exactly.
    p1 = 0.5
    _meta, values = _run_source_with_params(
        """
        poly = push_vec(2, 0)
        poly[0] = 1/(p1*2e-17)
        poly[1] = sqrt(p1*2e-40)
        """,
        [p1, 0.0, 0.0, 0.0],
        cfpv=[2, 0, 0],
    )
    assert abs(values[0].real - 1e17) <= 1e11
    assert abs(values[0].imag) <= 1e-6
    assert abs(values[1].real - 1e-20) <= 1e-26
    assert abs(values[1].imag) <= 1e-26


def test_coeff_program_tos_reads_vector_and_scalar_slots():
    # Static tos0 against a scalar stack top used to read a stale vector
    # slot from a previous row (push_scalar never writes stack_re/stack_im).
    # All cases go through chain-mode value expressions: that is where the
    # EXPR_TOS_AT / EXPR_TOS_AT_DYN bytecode reads are reachable.
    p1 = 0.5
    cases = [
        # (setup chip, value expr, expected real)
        (["push_range", "4"], "tos[p1*2]", 1.0),
        (["push_range", "4"], "tos1", 1.0),
        (["push_scalar", "p1"], "tos0", p1),
        (["push_scalar", "p1"], "tos[p1-p1]", p1),
    ]
    for setup, value_expr, expected in cases:
        chain = [
            setup,
            ["push_const", "4", value_expr],
            ["emit"],
            ["pop"],
        ]
        compiled = compile_coeff_program_chain(chain)
        _meta, data = _run_chunked_with_params(
            _compiled_coeff_program_payload(compiled), [p1, 0.0, 0.0, 0.0], cfpv=[4, 0, 0]
        )
        values = _complex_f32_values(data)
        for got in values:
            assert abs(got.real - expected) <= 1e-6, value_expr
            assert abs(got.imag) <= 1e-6, value_expr


def test_coeff_program_resolves_dynamic_args_before_source_pop():
    # Phase 2A resolved-arg ordering: affine(src=pop, multiplier=tos0)
    # must read tos0 from the pre-token stack frame. The old source-first
    # handler ordering popped the source before resolving the dynamic arg.
    _meta, values = _run_chain_values([
        ["push_const", "4", "2"],
        ["affine", "push", "pop", "tos0", "0"],
        ["emit"],
    ])
    assert len(values) == 4
    for got in values:
        assert abs(got.real - 4.0) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_sin_with_andy_blends_natively():
    # sin(poly, andy): source routes to the native transform (fn 18); andy=1
    # keeps the input, andy=0 applies sin fully.
    compiled_full = compile_coeff_program_source("poly = sin(poly, 0)")
    compiled_keep = compile_coeff_program_source("poly = sin(poly, 1)")
    base = {
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [3, 2, 0],
        "n1": 1,
        "n2": 1,
        "coeff_transforms": [],
    }
    _meta, full_data = _run_coeffgen({**base, "coeff_program": _compiled_coeff_program_payload(compiled_full)})
    _meta, keep_data = _run_coeffgen({**base, "coeff_program": _compiled_coeff_program_payload(compiled_keep)})
    full = _complex_f32_values(full_data)
    keep = _complex_f32_values(keep_data)
    want = cmath.sin(2.0 + 0.0j)
    for got in full:
        assert abs(got.real - want.real) <= 1e-6
        assert abs(got.imag - want.imag) <= 1e-6
    for got in keep:
        assert abs(got.real - 2.0) <= 1e-6
        assert abs(got.imag) <= 1e-6


def test_coeff_program_angle_branch_parity_static_vs_dynamic():
    # angle of (-x, -0) used to give -pi dynamically (raw atan2) while the
    # canonicalized helpers gave +pi; both paths now canonicalize signed zero.
    p1 = 1.0
    _meta, values = _run_source_with_params(
        """
        poly = push_vec(2, 0)
        poly[0] = angle(neg(1))
        poly[1] = angle(neg(p1))
        """,
        [p1, 0.0, 0.0, 0.0],
        cfpv=[2, 0, 0],
    )
    assert abs(values[0].real - math.pi) <= 1e-6
    assert abs(values[1].real - math.pi) <= 1e-6


def test_coeff_program_abs_uses_full_range_magnitude():
    # abs of |z| ~ 1e-200 used to underflow to 0 via sqrt(|z|^2); log(abs(z))
    # then hit the -700 sentinel. With hypot it matches the static fold.
    p1 = 1.0
    _meta, values = _run_source_with_params(
        """
        poly = push_vec(2, 0)
        poly[0] = log(abs(2e-200))
        poly[1] = log(abs(p1*2e-200))
        """,
        [p1, 0.0, 0.0, 0.0],
        cfpv=[2, 0, 0],
    )
    want = math.log(2e-200)
    assert abs(values[0].real - want) <= 1e-3
    assert abs(values[1].real - want) <= 1e-3


def test_coeff_program_elementwise_divide_by_zero_yields_zero():
    # Elementwise division is deliberately forgiving: a zero denominator
    # element yields 0 and the row continues (scalar expressions error).
    chain = [
        ["push_const", "2", "1"],
        ["poke_tos", "1", "4"],
        ["emit"],
        ["push_const", "2", "0"],
        ["poke_tos", "1", "2"],
        ["divide", "poly", "poly", "pop"],
    ]
    _meta, values = _run_chain_values(chain, n_coeffs=2)
    assert abs(values[0].real) <= 1e-12
    assert abs(values[1].real - 2.0) <= 1e-12
