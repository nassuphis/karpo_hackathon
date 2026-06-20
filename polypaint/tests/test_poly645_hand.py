import json
import os
import struct
import subprocess

import numpy as np
from tests.native_program_helpers import translate_legacy_transforms_for_native


ROOT = os.path.dirname(os.path.dirname(__file__))
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")
PARITY_FUNCTIONS = ["poly_645"]


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


def _poly_645_reference(t1, t2):
    degree = 8
    cf = np.zeros(degree + 1, dtype=np.complex128)
    real_seq = np.linspace(t1.real, t2.real, degree + 1)
    im_seq = np.linspace(t1.imag, t2.imag, degree + 1)
    for j in range(1, degree + 2):
        mag_component = np.log(np.abs(t1) + j) * np.sin(j * np.pi / 4) + np.cos(j * np.pi / 3) * np.abs(t2)
        intricate_part = np.exp(1j * (np.sin(real_seq[j - 1]) + np.cos(im_seq[j - 1])))
        cf[j - 1] = mag_component * intricate_part * np.conj(t2) + np.prod(np.arange(1, j + 1)) * np.sin(j)
    return cf


def test_poly645_matches_python_reference():
    transforms = [["t1radd", "1.0"], ["t1iadd", "0.25"], ["t2radd", "-0.4"], ["t2iadd", "0.75"]]
    params_path = "/tmp/test_poly645_params.bin"
    coeffs_path = "/tmp/test_poly645_coeffs.bin"

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
            "function": "poly_645",
            "n1": 1,
            "n2": 1,
            "i1_start": 0,
            "i1_end": 1,
            "param_transforms": transforms,
            "coeff_transforms": [],
            "times": 1,
        }, coeffs_path)

        assert meta["n_coeffs"] == 9
        assert meta["degree"] == 8

        coeff_steps = _read_coeffs(coeffs_path, meta["n_coeffs"])
        assert len(coeff_steps) == 1
        assert len(param_pairs) == 1

        (t1, t2) = param_pairs[0]
        got = coeff_steps[0]
        want = _poly_645_reference(t1, t2)
        np.testing.assert_allclose(got.real, want.real, rtol=1e-6, atol=1e-6)
        np.testing.assert_allclose(got.imag, want.imag, rtol=1e-6, atol=1e-6)
    finally:
        for path in (params_path, coeffs_path):
            if os.path.exists(path):
                os.remove(path)
