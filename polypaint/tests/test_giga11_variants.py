import json
import os
import struct
import subprocess

import numpy as np


ROOT = os.path.dirname(os.path.dirname(__file__))
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")


def _run_sweep(spec, out_path):
    result = subprocess.run(
        [SWEEP, out_path],
        input=json.dumps(spec),
        capture_output=True,
        text=True,
        timeout=30,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr)
    return json.loads(result.stdout)


def _read_params(path):
    data = open(path, "rb").read()
    floats = struct.unpack(f"<{len(data) // 4}f", data)
    return [
        (complex(floats[i], floats[i + 1]), complex(floats[i + 2], floats[i + 3]))
        for i in range(0, len(floats), 4)
    ]


def _read_coeffs(path, n_coeffs):
    data = open(path, "rb").read()
    floats = struct.unpack(f"<{len(data) // 4}f", data)
    stride = n_coeffs * 2
    coeffs = []
    for start in range(0, len(floats), stride):
        coeffs.append(np.array([
            complex(floats[start + j * 2], floats[start + j * 2 + 1])
            for j in range(n_coeffs)
        ], dtype=np.complex128))
    return coeffs


def _sanitize_nonfinite(arr):
    out = np.array(arr, dtype=np.complex128, copy=True)
    bad = ~np.isfinite(out.real) | ~np.isfinite(out.imag)
    out[bad] = 0.0 + 0.0j
    return out


def _giga_11a_reference(t1, t2):
    n = 25
    k = np.arange(1, n + 1)
    base = t1 + t2
    return (base ** k / k).astype(np.complex128)


def _giga_11b_reference(t1, t2):
    n = 40
    cf = np.zeros(n, dtype=np.complex128)
    m = int(5 * abs(t1 + t2) % 17) + 1
    modular_values = np.arange(n) % m
    for k in range(n):
        scale_factor = modular_values[k]
        cf[k] = scale_factor * np.exp(1j * np.pi * (k + 1) / (m + t1 + t2))
    return cf


def _assert_matches(function_name, want_fn, expected_degree):
    transforms = [
        ["unit_circle"],
        ["t1radd", "0.17"],
        ["t1iadd", "-0.23"],
        ["t2radd", "0.11"],
        ["t2iadd", "0.07"],
    ]
    n = 1
    params_path = "/tmp/test_giga11_params.bin"
    coeffs_path = "/tmp/test_giga11_coeffs.bin"

    try:
        _run_sweep({
            "mode": "param_dump",
            "n1": n,
            "n2": n,
            "param_transforms": transforms,
        }, params_path)
        param_pairs = _read_params(params_path)

        meta = _run_sweep({
            "mode": "coeffgen",
            "function": function_name,
            "n1": n,
            "n2": n,
            "i1_start": 0,
            "i1_end": n,
            "param_transforms": transforms,
            "coeff_transforms": [],
            "times": 1,
        }, coeffs_path)
        assert meta["degree"] == expected_degree

        coeff_steps = _read_coeffs(coeffs_path, meta["n_coeffs"])
        (t1, t2) = param_pairs[0]
        got = coeff_steps[0]
        want = _sanitize_nonfinite(want_fn(t1, t2))
        np.testing.assert_allclose(got.real, want.real, rtol=2e-5, atol=1e-5)
        np.testing.assert_allclose(got.imag, want.imag, rtol=2e-5, atol=1e-5)
    finally:
        for path in [params_path, coeffs_path]:
            if os.path.exists(path):
                os.remove(path)


def test_giga_11a_matches_python_reference():
    _assert_matches("giga_11a", _giga_11a_reference, 24)


def test_giga_11b_matches_python_reference():
    _assert_matches("giga_11b", _giga_11b_reference, 39)
