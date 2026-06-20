"""
Verify creative9 C implementation against Python reference.

Run: cd polypaint && uv run python tests/test_poly_creative9.py
"""
import cmath
import json
import math
import os
import struct
import subprocess
import numpy as np
from tests.native_program_helpers import translate_legacy_transforms_for_native

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP = os.path.join(LAMBDA_DIR, "sweep_test")

N1 = N2 = 10
NC = 71


def poly_creative9_py(t1, t2):
    """Python reference."""
    n = 71
    cf = np.zeros(n, dtype=np.complex128)
    for k in range(n):
        freq_t1 = (k + 1) * cmath.phase(t1)
        freq_t2 = (k + 1) * cmath.phase(t2)
        cf[k] = (np.sin(freq_t1) + 1j * np.cos(freq_t2)) * np.exp(-abs(t1 * t2) * k / n)
    for k in range(1, n - 1):
        cf[k] = (cf[k - 1] + cf[k + 1]) * 0.5 * (t1 + t2)
    return cf


def run_c():
    """Run C creative9 via coeffgen and return raw coefficient data."""
    cf_path = "/tmp/creative9_test.bin"
    spec = json.dumps(translate_legacy_transforms_for_native({
        "mode": "coeffgen",
        "function": "creative9",
        "n1": N1, "n2": N2,
        "i1_start": 0, "i1_end": N1,
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [],
        "times": 1,
    }))
    r = subprocess.run([SWEEP, cf_path], input=spec, capture_output=True, text=True, timeout=30)
    assert r.returncode == 0, f"coeffgen failed: {r.stderr[:200]}"
    meta = json.loads(r.stdout)
    assert meta["n_coeffs"] == NC
    with open(cf_path, "rb") as f:
        data = f.read()
    os.remove(cf_path)
    return data


def get_c_coeffs(data, i1, i2):
    """Extract coefficients for grid point (i1, i2) accounting for serpentine."""
    j = (N2 - 1 - i2) if (i1 & 1) else i2
    idx = i1 * N2 + j
    off = idx * NC * 2 * 4
    vals = struct.unpack_from(f"<{NC * 2}f", data, off)
    return [complex(vals[2 * k], vals[2 * k + 1]) for k in range(NC)]


def test_creative9():
    print("Running creative9 C vs Python comparison...")
    data = run_c()

    max_err = 0
    worst = None
    tested = 0

    for i1 in range(N1):
        for i2 in range(N2):
            # C coefficients
            c_cf = get_c_coeffs(data, i1, i2)

            # Python: apply unit_circle transform
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)
            py_cf = poly_creative9_py(t1, t2)

            # Compare
            for k in range(NC):
                err = abs(c_cf[k] - py_cf[k])
                if err > max_err:
                    max_err = err
                    worst = (i1, i2, k, c_cf[k], py_cf[k])
            tested += 1

    print(f"  Tested {tested} grid points")
    print(f"  Max error: {max_err:.2e}")
    if worst:
        i1, i2, k, cv, pv = worst
        print(f"  Worst at ({i1},{i2}) k={k}: C={cv:.6f} Py={pv:.6f}")

    # Per-point breakdown for top errors
    errors = []
    for i1 in range(N1):
        for i2 in range(N2):
            c_cf = get_c_coeffs(data, i1, i2)
            t1 = np.exp(2j * np.pi * i1 / N1)
            t2 = np.exp(2j * np.pi * i2 / N2)
            py_cf = poly_creative9_py(t1, t2)
            pt_err = max(abs(c_cf[k] - py_cf[k]) for k in range(NC))
            errors.append((pt_err, i1, i2))
    errors.sort(reverse=True)
    print(f"\n  Top 5 worst points:")
    for err, i1, i2 in errors[:5]:
        print(f"    ({i1},{i2}): max coeff error = {err:.2e}")

    threshold = 1e-3
    passing = sum(1 for e, _, _ in errors if e < threshold)
    print(f"\n  {passing}/{tested} points below {threshold}")
    print(f"  {'PASS' if max_err < threshold else 'FAIL'}")


if __name__ == "__main__":
    test_creative9()
