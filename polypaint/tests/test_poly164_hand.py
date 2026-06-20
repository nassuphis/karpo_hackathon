import json
import os
import struct
import subprocess

import numpy as np
from tests.native_program_helpers import translate_legacy_transforms_for_native


ROOT = os.path.dirname(os.path.dirname(__file__))
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")
PARITY_FUNCTIONS = ["poly_164"]


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


def _poly_164_reference(t1, t2):
    cf = np.zeros(71, dtype=np.complex128)
    for k in range(1, 72):
        if k % 4 == 1:
            cf[k - 1] = t1**k + t2**(k - 1) * np.sin(t1)
        elif k % 4 == 2:
            cf[k - 1] = (t1 + t2) ** k * np.cos(t2)
        elif k % 4 == 3:
            cf[k - 1] = np.log(np.abs(t1) + 1) * t2**k
        else:
            cf[k - 1] = (t1 - t2) ** k * np.sin(t1 * t2)
    cf[0] = t1 + 2 * t2
    cf[70] = t1**35 - t2**35 + 1j * t1 * t2
    return cf.astype(np.complex128)


def test_poly164_matches_python_reference():
    params_path = "/tmp/test_poly164_params.bin"
    coeffs_path = "/tmp/test_poly164_coeffs.bin"

    params = [
        (0.1 + 0.2j, 0.3 - 0.4j),
        (-0.7 + 0.5j, 0.2 + 0.8j),
        (1.0 + 0.0j, -0.5 + 0.25j),
    ]

    try:
        with open(params_path, "wb") as fh:
            for t1, t2 in params:
                fh.write(struct.pack(
                    "<ffff",
                    np.float32(t1.real),
                    np.float32(t1.imag),
                    np.float32(t2.real),
                    np.float32(t2.imag),
                ))

        meta = _run_sweep({
            "mode": "coeffgen_chunked",
            "function": "poly_164",
            "params_file": params_path,
            "step_start": 0,
            "step_count": len(params),
            "coeff_transforms": [],
        }, coeffs_path)

        assert meta["n_coeffs"] == 71
        assert meta["degree"] == 70

        coeff_steps = _read_coeffs(coeffs_path, meta["n_coeffs"])
        assert len(coeff_steps) == len(params)

        for got, (t1, t2) in zip(coeff_steps, params):
            # coeffgen_chunked reads float32 parameter payloads; compare against
            # the exact values it actually receives.
            t1 = complex(np.float32(t1.real), np.float32(t1.imag))
            t2 = complex(np.float32(t2.real), np.float32(t2.imag))
            want = _poly_164_reference(t1, t2)
            np.testing.assert_allclose(got.real, want.real, rtol=2e-6, atol=2e-4)
            np.testing.assert_allclose(got.imag, want.imag, rtol=2e-6, atol=2e-4)
    finally:
        for path in (params_path, coeffs_path):
            if os.path.exists(path):
                os.remove(path)
