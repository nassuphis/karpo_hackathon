import json
import os
import struct
import subprocess

import numpy as np
import pytest


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


def _p11a_reference(t1, t2):
    n = 40
    a = np.abs(t1 + t2) / 2
    m = int((10 * a) % 13) + 3
    v = np.arange(n) / (max(m, 3) + t1 + t2)
    uc = np.exp(1j * np.pi * v)
    sf = np.arange(n) % (m * 2)
    return (sf * uc).astype(np.complex128)


def _p11a_v1_reference(t1, t2):
    n = 40
    a = np.abs(t1 + t2) / 2
    m = int((5 * a) % 21) + 3
    v = np.power(np.linspace(0, 1, n), 0.75) / (max(m, 3) + t1 + t2)
    uc = np.exp(1j * 50 * np.pi * v)
    sf = np.arange(n) % (m + 10)
    return (sf * uc).astype(np.complex128)


def _p11a1_reference(t1, t2):
    n = 40
    a = np.abs(t1 + t2) / 2
    m = int((10 * a) % 13)
    v0 = np.linspace(0, 1, n)
    v = n * v0 / (m + 3 + t1 + t2)
    uc = np.exp(1j * np.pi * v)
    sf = np.arange(n) % (m * 2)
    return (sf * uc).astype(np.complex128)


def _p11a2_reference(t1, t2):
    n = 40
    a = np.abs(t1 + t2) / 2
    m = int((101 * a) % 17)
    v0 = np.linspace(0, 1, n)
    v = n * v0 / (m + 3 + t1 + t2)
    uc = np.exp(1j * np.pi * v)
    sf = np.arange(n) % (m * 2)
    return (sf * uc).astype(np.complex128)


@pytest.mark.parametrize(
    "func_name,ref_func",
    [
        ("p11a", _p11a_reference),
        ("p11a_v1", _p11a_v1_reference),
        ("p11a1", _p11a1_reference),
        ("p11a2", _p11a2_reference),
    ],
)
def test_p11a_family_matches_python_reference(func_name, ref_func):
    transforms = [
        ["unit_circle"],
        ["t1radd", "0.17"],
        ["t1iadd", "-0.09"],
        ["t2radd", "0.05"],
        ["t2iadd", "0.13"],
    ]
    params_path = f"/tmp/test_{func_name}_params.bin"
    coeffs_path = f"/tmp/test_{func_name}_coeffs.bin"

    try:
        _run_sweep({
            "mode": "param_dump",
            "n1": 1,
            "n2": 1,
            "param_transforms": transforms,
        }, params_path)
        (t1, t2) = _read_params(params_path)[0]

        meta = _run_sweep({
            "mode": "coeffgen",
            "function": func_name,
            "n1": 1,
            "n2": 1,
            "i1_start": 0,
            "i1_end": 1,
            "param_transforms": transforms,
            "coeff_transforms": [],
        }, coeffs_path)
        assert meta["n_coeffs"] == 40
        assert meta["degree"] == 39

        got = _read_coeffs(coeffs_path, meta["n_coeffs"])[0]
        want = _sanitize_nonfinite(ref_func(t1, t2))
        # The reference params come from param_dump float32 output, while direct coeffgen
        # uses the in-memory transformed params. This family is oscillatory enough that
        # a small parity tolerance is appropriate.
        np.testing.assert_allclose(got.real, want.real, rtol=1e-3, atol=1e-5)
        np.testing.assert_allclose(got.imag, want.imag, rtol=1e-3, atol=1e-5)
    finally:
        for path in [params_path, coeffs_path]:
            if os.path.exists(path):
                os.remove(path)
