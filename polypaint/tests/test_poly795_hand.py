import json
import os
import struct
import subprocess

import numpy as np


ROOT = os.path.dirname(os.path.dirname(__file__))
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")
PARITY_FUNCTIONS = ["poly_795"]


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


def _poly_795_reference(t1, t2):
    cf = np.zeros(25, dtype=np.complex128)
    for i in range(25):
        cf[i] = (i ** 2 + t1) * np.exp((1 + 0j) * i * t2) / (1 + np.abs(t1 * t2))
    cf[4:15] = np.real(cf[4:15]) * np.cos(np.imag(cf[4:15]))
    cf[16:25] = np.abs(cf[16:25]) * np.exp(1j * np.angle(t1 + t2))
    cf[2] = (t1 + 1j * t2) ** 3 - np.conj(t1 + 1j * t2) ** 3
    cf[6] = cf[22] = (np.abs(t1) ** 3 + np.abs(t2) ** 3) * np.exp(1j * np.angle(t2 - t1))
    cf[18] = np.where(np.abs(t2) > 1, np.log(np.abs(t2)), 0)
    return cf.astype(np.complex128)


def _assert_poly_795_case(transforms, expect_t2_gt_one):
    params_path = "/tmp/test_poly795_params.bin"
    coeffs_path = "/tmp/test_poly795_coeffs.bin"
    try:
        _run_sweep({
            "mode": "param_dump",
            "n1": 1,
            "n2": 1,
            "param_transforms": transforms,
        }, params_path)
        param_pairs = _read_params(params_path)

        meta = _run_sweep({
            "mode": "coeffgen",
            "function": "poly_795",
            "n1": 1,
            "n2": 1,
            "i1_start": 0,
            "i1_end": 1,
            "param_transforms": transforms,
            "coeff_transforms": [],
            "times": 1,
        }, coeffs_path)

        assert meta["n_coeffs"] == 25
        assert meta["degree"] == 24
        coeff_steps = _read_coeffs(coeffs_path, meta["n_coeffs"])
        assert len(coeff_steps) == 1
        assert len(param_pairs) == 1

        t1, t2 = param_pairs[0]
        if expect_t2_gt_one:
            assert np.abs(t2) > 1.0
        else:
            assert np.abs(t2) <= 1.0

        got = coeff_steps[0]
        want = _poly_795_reference(t1, t2)
        np.testing.assert_allclose(got.real, want.real, rtol=2e-3, atol=1e-5)
        np.testing.assert_allclose(got.imag, want.imag, rtol=2e-3, atol=1e-5)
    finally:
        for path in (params_path, coeffs_path):
            if os.path.exists(path):
                os.remove(path)


def test_poly795_matches_python_reference_for_both_where_branches():
    _assert_poly_795_case(
        [["t1radd", "0.35"], ["t1iadd", "-0.4"], ["t2radd", "0.2"], ["t2iadd", "1.1"]],
        expect_t2_gt_one=True,
    )
    _assert_poly_795_case(
        [["t1radd", "-0.6"], ["t1iadd", "0.15"], ["t2radd", "0.2"], ["t2iadd", "0.3"]],
        expect_t2_gt_one=False,
    )
