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


def _poly_giga_138_reference(t1, t2):
    n = 11
    cf = np.zeros(n, dtype=np.complex128)
    denom = t1 + t2 + 3
    adenom = np.abs(denom)
    if adenom < 1 and adenom > 1e-30:
        denom = denom / adenom
    z1 = t1
    z2 = t1 + t2
    step1 = (t2 - t1) / (n - 1)
    step2 = (t1 * t2 - z2) / (n - 1)
    for k in range(n):
        v1 = np.exp(1j * 2 * np.pi * z1)
        v2 = np.exp(1j * 2 * np.pi * z2)
        v = v1 + 1j * v2
        cf[k] = np.exp(1j * np.pi * (n * v / denom))
        z1 = z1 + step1
        z2 = z2 + step2
    return cf


def test_giga138_matches_python_reference():
    transforms = [
        ["unit_circle"],
        ["t1radd", "0.17"],
        ["t1iadd", "-0.23"],
        ["t2radd", "0.11"],
        ["t2iadd", "0.07"],
    ]
    n = 1

    params_path = "/tmp/test_giga138_params.bin"
    coeffs_path = "/tmp/test_giga138_coeffs.bin"

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
            "function": "giga_138",
            "n1": n,
            "n2": n,
            "i1_start": 0,
            "i1_end": n,
            "param_transforms": transforms,
            "coeff_transforms": [],
            "times": 1,
        }, coeffs_path)
        assert meta["n_coeffs"] == 11
        assert meta["degree"] == 10

        coeff_steps = _read_coeffs(coeffs_path, meta["n_coeffs"])
        assert len(coeff_steps) == 1
        assert len(param_pairs) == 1

        (t1, t2) = param_pairs[0]
        got = coeff_steps[0]
        want = _sanitize_nonfinite(_poly_giga_138_reference(t1, t2))
        np.testing.assert_allclose(
            got.real, want.real, rtol=2e-5, atol=1e-5,
            err_msg="real mismatch",
        )
        np.testing.assert_allclose(
            got.imag, want.imag, rtol=2e-5, atol=1e-5,
            err_msg="imag mismatch",
        )
    finally:
        for path in [params_path, coeffs_path]:
            if os.path.exists(path):
                os.remove(path)
