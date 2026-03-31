"""
Companion-matrix solver regression tests.

Tests sweep_cm binary against known polynomials, degenerate cases,
output format, and AE vs CM comparison.

Run: cd polypaint && uv run python -m pytest tests/test_companion_matrix.py -v
Requires: lambda/sweep_cm (compiled locally against LAPACK)
"""
import json
import math
import os
import struct
import subprocess
import tempfile

LAMBDA_DIR = os.path.join(os.path.dirname(__file__), "..", "lambda")
SWEEP_CM = os.path.join(LAMBDA_DIR, "sweep_cm")
SWEEP_AE = os.path.join(LAMBDA_DIR, "sweep")  # Deploy binary, not sweep_test

# Skip all tests if sweep_cm not available or not runnable on this platform.
# These tests run authoritatively in Docker (scripts/test-docker-runtime.sh).
# Host runs are convenience only.
_cm_runnable = False
if os.path.exists(SWEEP_CM):
    try:
        subprocess.run([SWEEP_CM], capture_output=True, timeout=2)
        _cm_runnable = True
    except (OSError, subprocess.TimeoutExpired):
        pass
if not _cm_runnable:
    import pytest
    pytest.skip("sweep_cm not runnable (ARM64 ELF on non-ARM host — use Docker)", allow_module_level=True)


def write_coeff_file(path, polys, n_coeffs):
    """Write coefficient file: each poly padded to n_coeffs entries."""
    with open(path, "wb") as f:
        for cf in polys:
            padded = list(cf) + [0j] * (n_coeffs - len(cf))
            for c in padded[:n_coeffs]:
                f.write(struct.pack("<ff", float(c.real), float(c.imag)))


def read_roots(path, degree):
    """Read interleaved float32 re/im root pairs."""
    with open(path, "rb") as f:
        data = f.read()
    n_floats = len(data) // 4
    vals = struct.unpack("<" + "f" * n_floats, data)
    roots_per_poly = degree
    all_roots = []
    for i in range(0, len(vals), 2):
        all_roots.append(complex(vals[i], vals[i + 1]))
    # Group by polynomial
    polys = []
    for i in range(0, len(all_roots), roots_per_poly):
        polys.append(all_roots[i:i + roots_per_poly])
    return polys


def polyval(cf, z):
    y = 0j
    for a in cf:
        y = y * z + a
    return y


def max_residual(cf, roots):
    if not roots:
        return 0.0
    return max(abs(polyval(cf, r)) for r in roots)


def run_cm(polys, n_coeffs):
    """Run sweep_cm on a list of polynomials. Returns list of root lists."""
    with tempfile.TemporaryDirectory() as td:
        cf_path = os.path.join(td, "cf.bin")
        out_path = os.path.join(td, "roots.bin")
        write_coeff_file(cf_path, polys, n_coeffs)
        spec = json.dumps({
            "mode": "solve_cm",
            "coeffs_file": cf_path,
            "n_coeffs": n_coeffs,
            "n_steps": len(polys),
        })
        r = subprocess.run(
            [SWEEP_CM, out_path], input=spec,
            capture_output=True, text=True, timeout=30)
        assert r.returncode == 0, f"sweep_cm failed: {r.stderr[:200]}"
        meta = json.loads(r.stdout)
        assert meta["n_t"] == len(polys)
        assert meta["degree"] == n_coeffs - 1
        return read_roots(out_path, n_coeffs - 1)


def run_ae(polys, n_coeffs):
    """Run Aberth solver on same coefficient file."""
    with tempfile.TemporaryDirectory() as td:
        cf_path = os.path.join(td, "cf.bin")
        out_path = os.path.join(td, "roots.bin")
        write_coeff_file(cf_path, polys, n_coeffs)
        spec = json.dumps({
            "mode": "solve",
            "coeffs_file": cf_path,
            "n_coeffs": n_coeffs,
            "n2": len(polys),
            "i1_start": 0,
            "i1_end": 1,
            "match_roots": False,
        })
        r = subprocess.run(
            [SWEEP_AE, out_path], input=spec,
            capture_output=True, text=True, timeout=30)
        assert r.returncode == 0, f"sweep failed: {r.stderr[:200]}"
        return read_roots(out_path, n_coeffs - 1)


# ── Test 1: Exact cubic ──────────────────────────────────────────────────

def test_exact_cubic():
    """x^3 - 6x^2 + 11x - 6 has roots 1, 2, 3."""
    cf = [1+0j, -6+0j, 11+0j, -6+0j]
    result = run_cm([cf], 4)
    roots = result[0]
    assert len(roots) == 3
    assert all(math.isfinite(r.real) and math.isfinite(r.imag) for r in roots)
    assert max_residual(cf, roots) < 1e-6
    mags = sorted(abs(r) for r in roots)
    assert abs(mags[0] - 1) < 0.01
    assert abs(mags[1] - 2) < 0.01
    assert abs(mags[2] - 3) < 0.01


# ── Test 2: Exact quartic ────────────────────────────────────────────────

def test_exact_quartic():
    """(x-1)(x-2)(x-3)(x-4) = x^4 - 10x^3 + 35x^2 - 50x + 24."""
    cf = [1+0j, -10+0j, 35+0j, -50+0j, 24+0j]
    result = run_cm([cf], 5)
    roots = result[0]
    assert len(roots) == 4
    assert all(math.isfinite(r.real) and math.isfinite(r.imag) for r in roots)
    assert max_residual(cf, roots) < 1e-5
    mags = sorted(abs(r) for r in roots)
    for i, expected in enumerate([1, 2, 3, 4]):
        assert abs(mags[i] - expected) < 0.01


# ── Test 3: Repeated root ────────────────────────────────────────────────

def test_repeated_root():
    """(x-1)^2 = x^2 - 2x + 1. Both roots near 1."""
    cf = [1+0j, -2+0j, 1+0j]
    result = run_cm([cf], 3)
    roots = result[0]
    assert len(roots) == 2
    assert all(math.isfinite(r.real) and math.isfinite(r.imag) for r in roots)
    assert max_residual(cf, roots) < 1e-5
    assert all(abs(r - 1) < 0.01 for r in roots)


# ── Test 4: Leading zero trim ────────────────────────────────────────────

def test_leading_zero():
    """[0, 1, -3, 2] should reduce to x^2 - 3x + 2, roots 1, 2."""
    cf = [0+0j, 1+0j, -3+0j, 2+0j]
    result = run_cm([cf], 4)
    roots = result[0]
    # Degree is still 3 from nCoeffs-1, but effective roots should be 1, 2 + a zero
    finite_roots = [r for r in roots if abs(r) > 0.001]
    reduced_cf = [1+0j, -3+0j, 2+0j]
    for r in finite_roots:
        assert abs(polyval(reduced_cf, r)) < 1e-5, f"root {r} doesn't satisfy reduced poly"


# ── Test 5: All-zero polynomial ──────────────────────────────────────────

def test_all_zero():
    """[0, 0, 0, 0] should not crash, output should be finite."""
    cf = [0+0j, 0+0j, 0+0j, 0+0j]
    result = run_cm([cf], 4)
    roots = result[0]
    assert len(roots) == 3
    assert all(math.isfinite(r.real) and math.isfinite(r.imag) for r in roots)


# ── Test 6: Complex quadratic ────────────────────────────────────────────

def test_complex_quadratic():
    """x^2 + 1 has roots +i, -i."""
    cf = [1+0j, 0+0j, 1+0j]
    result = run_cm([cf], 3)
    roots = result[0]
    assert len(roots) == 2
    assert max_residual(cf, roots) < 1e-6
    # Should have roots near +i and -i
    imags = sorted(r.imag for r in roots)
    assert abs(imags[0] - (-1)) < 0.01
    assert abs(imags[1] - 1) < 0.01


# ── Test 7: Output size check ────────────────────────────────────────────

def test_output_size():
    """Output file size must be n_steps * degree * 2 * 4 bytes."""
    n_coeffs = 6  # degree 5
    polys = [[1+0j, 0, 0, 0, 0, -1+0j]] * 3  # x^5 - 1, three copies
    with tempfile.TemporaryDirectory() as td:
        cf_path = os.path.join(td, "cf.bin")
        out_path = os.path.join(td, "roots.bin")
        write_coeff_file(cf_path, polys, n_coeffs)
        spec = json.dumps({
            "mode": "solve_cm", "coeffs_file": cf_path,
            "n_coeffs": n_coeffs, "n_steps": 3,
        })
        subprocess.run([SWEEP_CM, out_path], input=spec,
                       capture_output=True, text=True, timeout=30, check=True)
        expected_size = 3 * 5 * 2 * 4  # 3 polys × 5 roots × 2 floats × 4 bytes
        actual_size = os.path.getsize(out_path)
        assert actual_size == expected_size, f"Size: {actual_size} != {expected_size}"


# ── Test 8: AE vs CM comparison ──────────────────────────────────────────

def test_ae_vs_cm_comparison():
    """Both solvers should produce finite roots with small residuals on the same input."""
    if not os.path.exists(SWEEP_AE):
        print("  SKIPPED: lambda/sweep not found (run deploy.sh to compile)")
        return
    # Check if AE binary is actually runnable (might be wrong arch on host)
    try:
        subprocess.run([SWEEP_AE], capture_output=True, timeout=2)
    except (OSError, subprocess.TimeoutExpired):
        print("  SKIPPED: lambda/sweep not runnable on this platform (use Docker)")
        return

    polys = [
        [1+0j, -6+0j, 11+0j, -6+0j],              # cubic: roots 1,2,3
        [1+0j, -10+0j, 35+0j, -50+0j, 24+0j],       # quartic: roots 1,2,3,4 (pad to 5)
        [1+0j, -2+0j, 1+0j],                         # repeated: root 1,1
        [1+0j, 0+0j, 1+0j],                           # complex: roots +i,-i
    ]
    # Pad all to n_coeffs=5
    n_coeffs = 5
    padded = [p + [0j] * (n_coeffs - len(p)) for p in polys]

    cm_results = run_cm(padded, n_coeffs)
    ae_results = run_ae(padded, n_coeffs)

    assert len(cm_results) == len(ae_results) == len(polys)

    for i, (cf, cm_roots, ae_roots) in enumerate(zip(padded, cm_results, ae_results)):
        # Both should have degree roots
        assert len(cm_roots) == n_coeffs - 1, f"poly {i}: CM root count wrong"
        assert len(ae_roots) == n_coeffs - 1, f"poly {i}: AE root count wrong"

        # All finite
        for r in cm_roots:
            assert math.isfinite(r.real) and math.isfinite(r.imag), f"poly {i}: CM non-finite root"
        for r in ae_roots:
            assert math.isfinite(r.real) and math.isfinite(r.imag), f"poly {i}: AE non-finite root"

        # Residuals should be small for both (using original unpadded cf for residual)
        orig_cf = polys[i]
        cm_resid = max_residual(orig_cf, [r for r in cm_roots if abs(r) > 1e-10])
        ae_resid = max_residual(orig_cf, [r for r in ae_roots if abs(r) > 1e-10])
        assert cm_resid < 1e-3, f"poly {i}: CM residual {cm_resid:.2e}"
        assert ae_resid < 1e-3, f"poly {i}: AE residual {ae_resid:.2e}"


# ── Test 9: Linear polynomial ────────────────────────────────────────────

def test_linear():
    """x - 3 has root 3."""
    cf = [1+0j, -3+0j]
    result = run_cm([cf], 2)
    roots = result[0]
    assert len(roots) == 1
    assert abs(roots[0] - 3) < 1e-6


# ── Test 10: Inf coefficient overflow ───────────────────────────────────

def test_inf_coefficients():
    """Polynomials with inf coefficients should not crash — roots zeroed, skipped counted."""
    n_coeffs = 4  # degree 3
    # Build a coefficient file with inf values manually
    with tempfile.TemporaryDirectory() as td:
        cf_path = os.path.join(td, "cf.bin")
        out_path = os.path.join(td, "roots.bin")
        with open(cf_path, "wb") as f:
            # Poly 1: normal cubic (roots 1,2,3)
            for c in [1, 0, -6, 0, 11, 0, -6, 0]:
                f.write(struct.pack("<f", c))
            # Poly 2: leading coeff is inf
            f.write(struct.pack("<f", float('inf')))
            f.write(struct.pack("<f", 0.0))
            for _ in range(3):
                f.write(struct.pack("<ff", 1.0, 0.0))
            # Poly 3: normal cubic again
            for c in [1, 0, -6, 0, 11, 0, -6, 0]:
                f.write(struct.pack("<f", c))

        spec = json.dumps({
            "mode": "solve_cm", "coeffs_file": cf_path,
            "n_coeffs": n_coeffs,
        })
        r = subprocess.run(
            [SWEEP_CM, out_path], input=spec,
            capture_output=True, text=True, timeout=30)
        assert r.returncode == 0, f"sweep_cm crashed: rc={r.returncode} stderr={r.stderr[:200]}"
        assert r.stdout.strip().startswith("{"), f"No JSON output: {r.stdout[:200]!r}"
        meta = json.loads(r.stdout)
        assert meta["n_t"] == 3
        assert meta["skipped_overflow"] >= 1, f"Expected skipped_overflow >= 1, got {meta}"

        # Output file should still have 3 polys × 3 roots × 2 floats × 4 bytes
        expected_size = 3 * 3 * 2 * 4
        actual_size = os.path.getsize(out_path)
        assert actual_size == expected_size, f"Size: {actual_size} != {expected_size}"

        # The overflowed poly should have zero roots
        all_roots = read_roots(out_path, 3)
        assert len(all_roots) == 3
        # Poly 2 (index 1) should be all zeros
        for r in all_roots[1]:
            assert r == 0j, f"Overflowed poly root should be 0, got {r}"
        # Poly 1 and 3 should have valid roots
        assert max_residual([1+0j, -6+0j, 11+0j, -6+0j], all_roots[0]) < 1e-5
        assert max_residual([1+0j, -6+0j, 11+0j, -6+0j], all_roots[2]) < 1e-5


# ── Test 11: Near-overflow large coefficients ──────────────────────────

def test_near_overflow_coefficients():
    """Coefficients near float32 max (~3.4e38) that produce inf after normalization."""
    n_coeffs = 4
    with tempfile.TemporaryDirectory() as td:
        cf_path = os.path.join(td, "cf.bin")
        out_path = os.path.join(td, "roots.bin")
        with open(cf_path, "wb") as f:
            # Leading coeff tiny, other coeffs huge → normalized coeffs overflow
            # lead=1e-30, others=3e38 → ratio = 3e68, overflows double? No, double is fine.
            # lead=1e-30, others=inf → normalized = inf
            f.write(struct.pack("<ff", 1e-30, 0.0))  # tiny leading coeff
            f.write(struct.pack("<ff", float('inf'), 0.0))  # inf second coeff
            f.write(struct.pack("<ff", 1.0, 0.0))
            f.write(struct.pack("<ff", 1.0, 0.0))

        spec = json.dumps({
            "mode": "solve_cm", "coeffs_file": cf_path,
            "n_coeffs": n_coeffs,
        })
        r = subprocess.run(
            [SWEEP_CM, out_path], input=spec,
            capture_output=True, text=True, timeout=30)
        assert r.returncode == 0, f"sweep_cm crashed: {r.stderr[:200]}"
        meta = json.loads(r.stdout)
        assert meta["skipped_overflow"] >= 1
        # Roots should be finite (zeros for skipped poly)
        all_roots = read_roots(out_path, 3)
        for r in all_roots[0]:
            assert math.isfinite(r.real) and math.isfinite(r.imag), f"Non-finite root: {r}"


# ── Test 12: Mixed normal and overflow in batch ────────────────────────

def test_mixed_batch_overflow():
    """A batch with some normal and some overflowed polys completes fully."""
    n_coeffs = 3  # degree 2
    normal_cf = [1+0j, -3+0j, 2+0j]  # roots 1, 2
    polys = []
    for i in range(10):
        if i % 3 == 1:
            # Overflow poly
            polys.append([complex(float('inf'), 0), 1+0j, 1+0j])
        else:
            polys.append(normal_cf)

    with tempfile.TemporaryDirectory() as td:
        cf_path = os.path.join(td, "cf.bin")
        out_path = os.path.join(td, "roots.bin")
        write_coeff_file(cf_path, polys, n_coeffs)
        spec = json.dumps({
            "mode": "solve_cm", "coeffs_file": cf_path,
            "n_coeffs": n_coeffs,
        })
        r = subprocess.run(
            [SWEEP_CM, out_path], input=spec,
            capture_output=True, text=True, timeout=30)
        assert r.returncode == 0
        meta = json.loads(r.stdout)
        assert meta["n_t"] == 10
        # 4 overflow polys (indices 1,4,7 — wait, i%3==1: 1,4,7 = 3 polys)
        assert meta["skipped_overflow"] >= 3

        # All 10 polys should have output
        all_roots = read_roots(out_path, 2)
        assert len(all_roots) == 10

        # Normal polys should solve correctly
        for i in range(10):
            if i % 3 != 1:
                assert max_residual(normal_cf, all_roots[i]) < 1e-5, f"poly {i} bad residual"


if __name__ == "__main__":
    test_exact_cubic()
    test_exact_quartic()
    test_repeated_root()
    test_leading_zero()
    test_all_zero()
    test_complex_quadratic()
    test_output_size()
    test_linear()
    test_inf_coefficients()
    test_near_overflow_coefficients()
    test_mixed_batch_overflow()
    test_ae_vs_cm_comparison()
    print("\nAll companion matrix tests passed.")
