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


def _poly_giga_62_reference(t1, t2):
    cf = np.zeros(25, dtype=np.complex128)
    cf[0:5] = np.array([abs(t1 + t2) ** (i + 1) for i in range(5)], dtype=np.complex128)
    cf[5:10] = ((t1 + 2j * t2) ** 3).real * np.log(np.abs(np.conj(t1 * t2)))
    cf[10:15] = ((t1 - t2) ** 2).imag / np.angle(t1 * t2)
    cf[15:20] = np.abs(cf[5:10]) ** 0.5 + np.angle(cf[0:5])
    cf[20:25] = np.array([abs(t1 * t2) ** (i + 1) for i in range(5)], dtype=np.complex128)
    return cf


def test_giga62_hand_matches_python_reference():
    transforms = [["t1radd", "1.0"], ["t1iadd", "0.1"], ["t2radd", "-0.4"], ["t2iadd", "0.2"]]
    n = 1

    params_path = "/tmp/test_giga62_params.bin"
    coeffs_path = "/tmp/test_giga62_coeffs.bin"

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
            "function": "giga_62",
            "n1": n,
            "n2": n,
            "i1_start": 0,
            "i1_end": n,
            "param_transforms": transforms,
            "coeff_transforms": [],
            "times": 1,
        }, coeffs_path)
        assert meta["n_coeffs"] == 25
        assert meta["degree"] == 24

        coeff_steps = _read_coeffs(coeffs_path, meta["n_coeffs"])
        assert len(coeff_steps) == 1
        assert len(param_pairs) == 1

        (t1, t2) = param_pairs[0]
        got = coeff_steps[0]
        want = _sanitize_nonfinite(_poly_giga_62_reference(t1, t2))
        np.testing.assert_allclose(
            got.real, want.real, rtol=1e-5, atol=1e-5,
            err_msg="real mismatch",
        )
        np.testing.assert_allclose(
            got.imag, want.imag, rtol=1e-6, atol=1e-6,
            err_msg="imag mismatch",
        )
    finally:
        for path in [params_path, coeffs_path]:
            if os.path.exists(path):
                os.remove(path)
