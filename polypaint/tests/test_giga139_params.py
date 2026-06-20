import json
import os
import struct
import subprocess

import numpy as np
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


def _poly_giga_139_reference(t1, t2, int1=251, int2=37, int3=619, deg=11):
    deg = int(max(2, deg))
    int1 = int(int1)
    int2 = max(1, int(int2))
    int3 = int(int3)
    t = t1 + t2
    a = np.abs(t1 + t2) / 2
    m = int(a * int1) % int2
    v = (np.arange(deg) + 1) / (t + 4)
    p1 = int(7 * a * len(v)) % len(v)
    v[p1] = (p1 + 1) / (t + np.abs(t) * 2 + 1 + m)
    p2 = int(int3 * a * len(v)) % len(v)
    v[p2] = (p2 + 1) / (t + np.abs(t) * 2 + 1 + m // 2)
    return np.exp(1j * np.pi * v).astype(np.complex128)


def test_giga139_defaults_match_python_reference():
    transforms = [["unit_circle"], ["t1radd", "0.17"], ["t2iadd", "0.09"]]
    params_path = "/tmp/test_giga139_params.bin"
    coeffs_path = "/tmp/test_giga139_coeffs.bin"

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
            "function": "giga_139",
            "n1": 1,
            "n2": 1,
            "i1_start": 0,
            "i1_end": 1,
            "param_transforms": transforms,
            "coeff_transforms": [],
        }, coeffs_path)
        assert meta["n_coeffs"] == 11
        assert meta["degree"] == 10
        got = _read_coeffs(coeffs_path, meta["n_coeffs"])[0]
        want = _sanitize_nonfinite(_poly_giga_139_reference(t1, t2))
        np.testing.assert_allclose(got.real, want.real, rtol=1e-5, atol=1e-5)
        np.testing.assert_allclose(got.imag, want.imag, rtol=1e-5, atol=1e-5)
    finally:
        for path in [params_path, coeffs_path]:
            if os.path.exists(path):
                os.remove(path)


def test_giga139_cfpv_override_changes_degree_and_matches_reference():
    transforms = [["unit_circle"], ["t1iadd", "-0.11"], ["t2radd", "0.06"]]
    params_path = "/tmp/test_giga139_params2.bin"
    coeffs_path = "/tmp/test_giga139_coeffs2.bin"
    cfpv = [251, 37, 443, 13]

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
            "function": "giga_139",
            "n1": 1,
            "n2": 1,
            "i1_start": 0,
            "i1_end": 1,
            "param_transforms": transforms,
            "coeff_transforms": [],
            "cfpv": cfpv,
        }, coeffs_path)
        assert meta["n_coeffs"] == 13
        assert meta["degree"] == 12
        got = _read_coeffs(coeffs_path, meta["n_coeffs"])[0]
        want = _sanitize_nonfinite(_poly_giga_139_reference(t1, t2, *cfpv))
        np.testing.assert_allclose(got.real, want.real, rtol=1e-5, atol=1e-5)
        np.testing.assert_allclose(got.imag, want.imag, rtol=1e-5, atol=1e-5)
    finally:
        for path in [params_path, coeffs_path]:
            if os.path.exists(path):
                os.remove(path)
