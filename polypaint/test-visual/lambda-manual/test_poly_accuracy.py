"""
Test that transpiled C poly functions match the Python originals.

Run: uv run python test-visual/lambda-manual/test_poly_accuracy.py
"""
import json
import os
import struct
import subprocess
import sys
import numpy as np

_LAMBDA_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "lambda"))


def poly_16(t1, t2):
    """Copied from poly100.py (can't import due to relative imports)."""
    cf = np.zeros(51, dtype=complex)
    cf[0] = t1 + t2
    cf[1] = np.real(t1**2 - t2**2)
    primes = np.array([2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
                       53, 59, 61, 67, 71, 73, 79, 83, 89, 97])
    for k in range(2, 25):
        cf[k] = np.imag(cf[k - 1] * primes[k - 2]) * np.angle(t1) * np.abs(t2)
    for k in range(25, 50):
        cf[k] = np.abs(cf[k - 1] * primes[k - 25] ** 2) * np.angle(t2) * np.real(t1)
    cf[50] = np.sum(cf) + np.sin(np.real(t2)) * np.log(np.abs(t1) + 1)
    return cf.astype(np.complex128)


def poly_17(t1, t2):
    """Copied from poly100.py."""
    cf = np.zeros(71, dtype=complex)
    cf[0:10] = (t1 + t2) * np.arange(1, 11)
    cf[10:20] = np.real(t1 - t2)**3 * np.arange(11, 21)
    cf[20:30] = np.imag(t1 + t2)**2 * np.arange(21, 31)
    cf[30:40] = np.abs(t1 - t2) * np.arange(31, 41)
    cf[40:50] = np.angle(t1 * t2) * np.arange(41, 51)
    cf[50] = np.sin(t1) * np.cos(t2) + np.sin(t2) * np.cos(t1)
    return cf.astype(np.complex128)


SWEEP = os.path.join(_LAMBDA_DIR, "sweep_test")

def run_c_coeffgen(func_name, t1r, t1i, t2r, t2i):
    """Run the C binary for a single grid point and return coefficients."""
    # We use a 1x1 grid where the single point is (t1, t2).
    # param_transforms are empty so x1=i1/n1, x2=i2/n2.
    # To get specific t1,t2 we'd need to set n1=1,n2=1 and i1=0,i2=0 → (0,0).
    # Instead, use unit_circle transform: t = e^{2πi·x} so x=angle/(2π).
    # But that only gives unit-circle points.
    #
    # Simplest: use no transforms, n1=n2=1, so x1=0/1=0, x2=0/1=0.
    # That only tests (0,0). For arbitrary points we need a trick.
    #
    # Better: use a large N and pick the right grid indices.
    # x1 = i1/n1, x2 = i2/n2. We want x1=t1r, x2=t2r (real inputs, no imag).
    # Use n1=n2=10000, i1=int(t1r*10000), i2=int(t2r*10000).

    N = 10000
    i1 = max(0, min(N-1, int(t1r * N)))
    i2 = max(0, min(N-1, int(t2r * N)))

    spec = {
        "mode": "coeffgen",
        "param_transforms": [],
        "function": func_name,
        "coeff_transforms": [],
        "n1": N, "n2": N,
        "i1_start": i1, "i1_end": i1 + 1,
    }

    result = subprocess.run(
        [SWEEP, "/tmp/test_poly_acc.bin"],
        input=json.dumps(spec),
        capture_output=True, text=True, timeout=10
    )
    if result.returncode != 0:
        raise RuntimeError(f"sweep failed: {result.stderr}")

    meta = json.loads(result.stdout)
    n_coeffs = meta["n_coeffs"]

    with open("/tmp/test_poly_acc.bin", "rb") as f:
        data = f.read()

    # Binary format: n2 grid points, each with n_coeffs * 2 floats (re, im pairs)
    # We want the point at i2
    point_size = n_coeffs * 2 * 4  # float32

    # The stripe has rows from i1_start to i1_end (just 1 row).
    # Within that row, columns are iterated. For even i1, j goes 0..n2-1 so i2=j.
    # For odd i1, j goes n2-1..0, so the column at position j corresponds to i2=n2-1-j.
    if i1 % 2 == 0:
        offset = i2 * point_size
    else:
        offset = (N - 1 - i2) * point_size

    floats = struct.unpack_from(f'<{n_coeffs * 2}f', data, offset)
    coeffs = []
    for k in range(n_coeffs):
        coeffs.append(complex(floats[k*2], floats[k*2+1]))

    return np.array(coeffs)


def compare_poly(func_name, py_func, test_points, tol=1e-4):
    """Compare Python and C implementations of a poly function at given test points."""
    print(f"Testing {func_name}: Python vs C")
    print("=" * 60)

    all_pass = True
    for t1r, t2r in test_points:
        N = 10000
        i1 = max(0, min(N-1, int(t1r * N)))
        i2 = max(0, min(N-1, int(t2r * N)))
        actual_t1 = complex(i1 / N, 0)
        actual_t2 = complex(i2 / N, 0)

        py_coeffs = py_func(actual_t1, actual_t2)
        c_coeffs = run_c_coeffgen(func_name, i1/N, 0, i2/N, 0)

        n = min(len(py_coeffs), len(c_coeffs))
        diffs = np.abs(py_coeffs[:n] - c_coeffs[:n])
        sum_abs_diff = np.sum(diffs)
        max_err = np.max(diffs)
        worst_k = int(np.argmax(diffs))

        status = "PASS" if max_err < tol else "FAIL"
        if status == "FAIL":
            all_pass = False

        print(f"  t1={actual_t1.real:.4f}, t2={actual_t2.real:.4f}: "
              f"sum|diff|={sum_abs_diff:.2e}  max_err={max_err:.2e} at cf[{worst_k}] — {status}")

        if max_err >= tol:
            for k in range(n):
                if diffs[k] > 1e-6:
                    print(f"    cf[{k}]: py={py_coeffs[k]:.6f}  c={c_coeffs[k]:.6f}  err={diffs[k]:.2e}")

    print()
    if all_pass:
        print("All tests PASSED")
    else:
        print("Some tests FAILED")

    return all_pass


TEST_POINTS = [
    (0.3, 0.7),
    (0.1, 0.9),
    (0.5, 0.5),
    (0.8, 0.2),
    (0.01, 0.99),
]


def test_poly_16():
    assert compare_poly("poly_16", poly_16, TEST_POINTS)


def test_poly_17():
    assert compare_poly("poly_17", poly_17, TEST_POINTS)


if __name__ == "__main__":
    ok = True
    ok = compare_poly("poly_16", poly_16, TEST_POINTS) and ok
    ok = compare_poly("poly_17", poly_17, TEST_POINTS) and ok
    sys.exit(0 if ok else 1)
