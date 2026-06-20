import json
import os
import struct
import subprocess

import numpy as np
import pytest
from tests.native_program_helpers import translate_legacy_transforms_for_native


ROOT = os.path.dirname(os.path.dirname(__file__))
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")


def _run_sweep(spec, out_path):
    result = subprocess.run(
        [SWEEP, out_path],
        input=json.dumps(translate_legacy_transforms_for_native(spec)),
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


def _normalize_denom(t1, t2):
    denom = t1 + t2 + 3
    adenom = np.abs(denom)
    if adenom < 1 and adenom > 1e-30:
        denom = denom / adenom
    return denom


def _p11b2_reference(t1, t2):
    n = 71
    v = np.linspace(0, 1, n)
    denom = _normalize_denom(t1, t2)
    u = 7 * n * np.power(v, 3) / denom
    uc = np.exp(1j * np.pi * u)
    sf = np.arange(n) % (int(101 * np.abs(t1 + t2)) % 51 + 1)
    return (sf * uc).astype(np.complex128)


def _p11b2_v1_reference(t1, t2):
    n = 71
    v = np.linspace(0, 1, n)
    denom = _normalize_denom(t1, t2)
    u = 7 * n * np.power(v, 15) / denom
    uc = np.exp(1j * np.pi * u)
    sf = (np.arange(n) + 1) % (int(4583 * np.abs(t1 + t2)) % 71 + 1)
    return ((sf + 1) * uc).astype(np.complex128)


def _p11b2_v2_reference(t1, t2):
    n = 11
    v = np.linspace(0, 1, n)
    denom = _normalize_denom(t1, t2)
    u = 7 * n * np.power(v, 15) / denom
    uc = np.exp(1j * np.pi * u)
    sf = (np.arange(n) + 1) % (int(4583 * np.abs(t1 + t2)) % 11 + 1)
    return ((sf + 1) * uc).astype(np.complex128)


def _p11b2_v3_reference(t1, t2):
    n = 11
    v = np.linspace(0, 1, n)
    denom = _normalize_denom(t1, t2)
    u = 7 * n * np.power(v, 15) / denom
    uc = np.exp(1j * np.pi * u)
    sf = np.power(np.cos(2 * np.pi * (v + 0.5j)), 3) * (n - 1)
    return ((sf + 1) * uc).astype(np.complex128)


def _p11b2_v4_reference(t1, t2):
    n = 11
    v = np.linspace(0, 1, n)
    denom = _normalize_denom(t1, t2)
    u = 7 * n * np.power(v, 15) / denom
    uc = np.exp(1j * np.pi * u)
    sf = np.power(np.cos(2 * np.pi * (v - 0.5j)), 3) * (n - 1)
    return ((sf + 1) * uc).astype(np.complex128)


def _p11b2_v5_reference(t1, t2):
    n = 11
    v1 = np.exp(1j * 2 * np.pi * np.linspace(t1, t2, n))
    v2 = np.exp(1j * 2 * np.pi * np.linspace(t1 + t2, t1 * t2, n))
    v = v1 + 1j * v2
    denom = _normalize_denom(t1, t2)
    u = n * v / denom
    return np.exp(1j * np.pi * u).astype(np.complex128)


def _p11b3_reference(t1, t2):
    t = t1 + t2
    a = np.abs(t1 + t2) / 2
    m = int(a * 251) % 37
    v = (np.arange(11) + 1) / (t + 4)
    p1 = int(7 * a * len(v)) % len(v)
    v[p1] = (p1 + 1) / (t + np.abs(t) * 2 + 1 + m)
    p2 = int(619 * a * len(v)) % len(v)
    v[p2] = (p2 + 1) / (t + np.abs(t) * 2 + 1 + m // 2)
    return np.exp(1j * np.pi * v).astype(np.complex128)


@pytest.mark.parametrize(
    "func_name,ref_func,n_coeffs",
    [
        ("p11b2", _p11b2_reference, 71),
        ("p11b2_v1", _p11b2_v1_reference, 71),
        ("p11b2_v2", _p11b2_v2_reference, 11),
        ("p11b2_v3", _p11b2_v3_reference, 11),
        ("p11b2_v4", _p11b2_v4_reference, 11),
        ("p11b2_v5", _p11b2_v5_reference, 11),
        ("p11b3", _p11b3_reference, 11),
    ],
)
def test_p11b_family_matches_python_reference(func_name, ref_func, n_coeffs):
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
        assert meta["n_coeffs"] == n_coeffs
        assert meta["degree"] == n_coeffs - 1

        got = _read_coeffs(coeffs_path, meta["n_coeffs"])[0]
        want = _sanitize_nonfinite(ref_func(t1, t2))
        # The reference params come from param_dump float32 output, while direct coeffgen
        # uses the in-memory transformed params. This oscillatory family is sensitive
        # enough that a few cases need a slightly wider parity tolerance.
        np.testing.assert_allclose(got.real, want.real, rtol=1e-3, atol=1e-5)
        np.testing.assert_allclose(got.imag, want.imag, rtol=1e-3, atol=1e-5)
    finally:
        for path in [params_path, coeffs_path]:
            if os.path.exists(path):
                os.remove(path)
