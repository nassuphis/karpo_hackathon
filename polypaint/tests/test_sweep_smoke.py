"""
Smoke tests for the sweep binary — coeffgen, solve, grid modes, dither uniqueness,
solver correctness, round-trip verification, transforms, edge cases.

Run: cd polypaint && uv run python -m pytest tests/test_sweep_smoke.py -v

Requires: sweep_test binary compiled in lambda/
  cd polypaint/lambda && cc -O3 -o sweep_test sweep_cli.c -lm
"""
import json
import math
import os
import struct
import subprocess
import unittest

import numpy as np

_LAMBDA_DIR = os.path.join(os.path.dirname(__file__), '..', 'lambda')
SWEEP = os.path.join(_LAMBDA_DIR, "sweep_test")


def _run_sweep(spec, out_path):
    """Run sweep binary with JSON spec, return parsed metadata."""
    result = subprocess.run(
        [SWEEP, out_path],
        input=json.dumps(spec),
        capture_output=True, text=True, timeout=30
    )
    if result.returncode != 0:
        raise RuntimeError(f"sweep failed: {result.stderr}")
    return json.loads(result.stdout)


class TestCoeffgenSmoke(unittest.TestCase):
    """Test the coeffgen mode produces correct output."""

    def test_old_379_hand_matches_python_reference(self):
        """old_379 is manually wired into the catalog and matches the Python formula."""
        params_path = "/tmp/test_old_379_params.bin"
        coeffs_path = "/tmp/test_old_379_coeffs.bin"
        param_spec = {
            "mode": "param_dump",
            "param_transforms": [["unit_circle"]],
            "n1": 3, "n2": 3,
            "i1_start": 0, "i1_end": 3,
        }
        _run_sweep(param_spec, params_path)
        dumped_pairs = _read_param_pairs(params_path)
        param_pairs = []
        for i1 in range(3):
            row = dumped_pairs[i1 * 3:(i1 + 1) * 3]
            param_pairs.extend(reversed(row) if (i1 & 1) else row)

        meta = _run_sweep({
            "mode": "coeffgen",
            "function": "old_379",
            "param_transforms": [["unit_circle"]],
            "coeff_transforms": [],
            "n1": 3, "n2": 3,
            "i1_start": 0, "i1_end": 3,
        }, coeffs_path)

        self.assertEqual(meta["n_coeffs"], 35)
        self.assertEqual(meta["degree"], 34)
        got = _read_coeffs(coeffs_path, 35)

        expected = []
        for t1, t2 in param_pairs:
            cf = np.zeros(35, dtype=np.complex128)
            for j in range(1, 36):
                magnitude = np.log(np.abs(t1) + np.abs(t2) + j) * (
                    (np.abs(t1) ** np.sin(j)) + (np.abs(t2) ** np.cos(j))
                )
                angle = (
                    np.angle(t1) * j
                    - np.angle(t2) * (35 - j)
                    + np.sin(j) * np.cos(j)
                )
                cf[j - 1] = magnitude * (np.cos(angle) + 1j * np.sin(angle))
            for k in range(1, 36):
                cf[k - 1] += t1.conjugate() * (t2 ** k) / (k + 1)
            for idx in [4, 9, 14, 19, 24, 29]:
                cf[idx] += 50 * (t1.real - t2.imag) * 1j
            expected.append(cf)
        expected = np.asarray(expected, dtype=np.complex128).astype(np.complex64).astype(np.complex128)

        np.testing.assert_allclose(got, expected, rtol=2e-5, atol=2e-5)

    def test_coeffgen_basic(self):
        """Coeffgen outputs correct metadata and file size."""
        out = "/tmp/test_cg_basic.bin"
        meta = _run_sweep({
            "mode": "coeffgen",
            "function": "giga_30",
            "param_transforms": [["unit_circle"]],
            "coeff_transforms": [],
            "n1": 10, "n2": 10,
            "i1_start": 0, "i1_end": 10,
        }, out)

        self.assertEqual(meta["mode"], "coeffgen")
        self.assertEqual(meta["n_t"], 100)  # 10 rows * 10 cols
        self.assertGreater(meta["n_coeffs"], 0)
        self.assertGreater(meta["degree"], 0)

        actual_size = os.path.getsize(out)
        self.assertEqual(actual_size, meta["data_bytes"])
        expected = meta["n_t"] * meta["n_coeffs"] * 2 * 4
        self.assertEqual(actual_size, expected)

    def test_coeffgen_with_times(self):
        """Coeffgen with times=3 produces 3x the data."""
        out1 = "/tmp/test_cg_t1.bin"
        out3 = "/tmp/test_cg_t3.bin"

        meta1 = _run_sweep({
            "mode": "coeffgen",
            "function": "giga_30",
            "param_transforms": [["unit_circle"]],
            "coeff_transforms": [],
            "n1": 10, "n2": 10,
            "i1_start": 0, "i1_end": 10,
            "times": 1,
        }, out1)

        meta3 = _run_sweep({
            "mode": "coeffgen",
            "function": "giga_30",
            "param_transforms": [["unit_circle"]],
            "coeff_transforms": [],
            "n1": 10, "n2": 10,
            "i1_start": 0, "i1_end": 10,
            "times": 3,
        }, out3)

        self.assertEqual(meta3["n_t"], meta1["n_t"] * 3)
        self.assertEqual(meta3["data_bytes"], meta1["data_bytes"] * 3)
        self.assertEqual(os.path.getsize(out3), os.path.getsize(out1) * 3)


class TestSolveSmoke(unittest.TestCase):
    """Test the solve-from-coefficients mode."""

    def test_solve_basic(self):
        """Solve reads all coefficients and produces correct output size."""
        cg_out = "/tmp/test_solve_cg.bin"
        meta_cg = _run_sweep({
            "mode": "coeffgen",
            "function": "giga_30",
            "param_transforms": [["unit_circle"]],
            "coeff_transforms": [],
            "n1": 10, "n2": 10,
            "i1_start": 0, "i1_end": 10,
        }, cg_out)

        solve_out = "/tmp/test_solve_basic.bin"
        meta_solve = _run_sweep({
            "mode": "solve",
            "coeffs_file": cg_out,
            "n_coeffs": meta_cg["n_coeffs"],
            "n2": 10,
            "i1_start": 0, "i1_end": 10,
        }, solve_out)

        self.assertEqual(meta_solve["n_t"], 100)
        degree = meta_cg["degree"]
        expected_size = 100 * degree * 2 * 4
        self.assertEqual(os.path.getsize(solve_out), expected_size)

    def test_solve_reads_all_times(self):
        """Solve with times=3 coefficients processes all 300 steps."""
        cg_out = "/tmp/test_solve_t3_cg.bin"
        meta_cg = _run_sweep({
            "mode": "coeffgen",
            "function": "giga_30",
            "param_transforms": [["unit_circle"], ["sdith", "1"]],
            "coeff_transforms": [],
            "n1": 10, "n2": 10,
            "i1_start": 0, "i1_end": 10,
            "times": 3,
        }, cg_out)

        solve_out = "/tmp/test_solve_t3.bin"
        meta_solve = _run_sweep({
            "mode": "solve",
            "coeffs_file": cg_out,
            "n_coeffs": meta_cg["n_coeffs"],
            "n2": 10,
            "i1_start": 0, "i1_end": 10,
        }, solve_out)

        self.assertEqual(meta_solve["n_t"], 300)
        degree = meta_cg["degree"]
        expected_size = 300 * degree * 2 * 4
        self.assertEqual(os.path.getsize(solve_out), expected_size)

    def test_solve_times_multiplier(self):
        """Solve output for times=3 is exactly 3x the times=1 output."""
        # times=1
        cg1 = "/tmp/test_solve_cmp_cg1.bin"
        _run_sweep({
            "mode": "coeffgen", "function": "giga_30",
            "param_transforms": [["unit_circle"], ["sdith", "1"]],
            "coeff_transforms": [],
            "n1": 10, "n2": 10, "i1_start": 0, "i1_end": 10, "times": 1,
        }, cg1)
        s1 = "/tmp/test_solve_cmp_s1.bin"
        _run_sweep({
            "mode": "solve", "coeffs_file": cg1,
            "n_coeffs": 10, "n2": 10, "i1_start": 0, "i1_end": 10,
        }, s1)

        # times=3
        cg3 = "/tmp/test_solve_cmp_cg3.bin"
        _run_sweep({
            "mode": "coeffgen", "function": "giga_30",
            "param_transforms": [["unit_circle"], ["sdith", "1"]],
            "coeff_transforms": [],
            "n1": 10, "n2": 10, "i1_start": 0, "i1_end": 10, "times": 3,
        }, cg3)
        s3 = "/tmp/test_solve_cmp_s3.bin"
        _run_sweep({
            "mode": "solve", "coeffs_file": cg3,
            "n_coeffs": 10, "n2": 10, "i1_start": 0, "i1_end": 10,
        }, s3)

        self.assertEqual(os.path.getsize(s3), os.path.getsize(s1) * 3)

    def test_solve_reseeds_after_repeated_root_step(self):
        """A repeated-root warm start must reseed before the next distinct solve."""
        coeffs_path = "/tmp/test_solve_reseed_coeffs.bin"
        solve_out = "/tmp/test_solve_reseed_roots.bin"

        with open(coeffs_path, "wb") as fh:
            for coeffs in ([1, -2, 1], [1, 0, 1]):  # (z-1)^2 then z^2+1
                padded = list(coeffs) + [0.0] * (5 - len(coeffs))
                for c in padded[:5]:
                    fh.write(struct.pack("<ff", float(c), 0.0))

        meta = _run_sweep({
            "mode": "solve",
            "coeffs_file": coeffs_path,
            "n_coeffs": 5,
            "n2": 2,
            "i1_start": 0,
            "i1_end": 1,
            "match_roots": False,
        }, solve_out)

        self.assertEqual(meta["n_t"], 2)
        roots = _read_roots(solve_out, meta["degree"])
        complex_step = [z for z in roots[1] if abs(z) > 1e-10]
        self.assertEqual(len(complex_step), 2)
        for z in complex_step:
            self.assertLess(abs(z * z + 1), 1e-2, f"bad reseed root {z}")


class TestGridSmoke(unittest.TestCase):
    """Test the grid mode (inline coefficients + solve)."""

    def test_grid_basic(self):
        """Grid mode produces correct n_t and output size."""
        out = "/tmp/test_grid_basic.bin"
        meta = _run_sweep({
            "mode": "grid",
            "function": "giga_30",
            "n1": 20, "n2": 20,
            "i1_start": 0, "i1_end": 20,
            "match_roots": False,
        }, out)

        self.assertEqual(meta["n_t"], 400)
        degree = meta["degree"]
        expected_size = 400 * degree * 2 * 4
        self.assertEqual(os.path.getsize(out), expected_size)


class TestDitherUniqueness(unittest.TestCase):
    """Verify RNG produces unique dither per pass."""

    def test_passes_differ(self):
        """With times=2 and sdith, pass 0 and pass 1 produce different coefficients."""
        out = "/tmp/test_dither_unique.bin"
        meta = _run_sweep({
            "mode": "coeffgen",
            "function": "giga_30",
            "param_transforms": [["unit_circle"], ["sdith", "1"]],
            "coeff_transforms": [],
            "n1": 10, "n2": 10,
            "i1_start": 0, "i1_end": 10,
            "times": 2,
        }, out)

        size = os.path.getsize(out)
        half = size // 2
        with open(out, "rb") as f:
            first_half = f.read(half)
            second_half = f.read(half)

        self.assertNotEqual(first_half, second_half,
                            "Pass 0 and pass 1 produced identical coefficients — RNG not seeded per pass")

    def test_different_stripes_differ(self):
        """Different i1_start values produce different dither sequences."""
        out_a = "/tmp/test_dither_stripe_a.bin"
        out_b = "/tmp/test_dither_stripe_b.bin"

        spec = {
            "mode": "coeffgen",
            "function": "giga_30",
            "param_transforms": [["unit_circle"], ["sdith", "1"]],
            "coeff_transforms": [],
            "n1": 20, "n2": 10,
            "times": 1,
        }

        _run_sweep({**spec, "i1_start": 0, "i1_end": 10}, out_a)
        _run_sweep({**spec, "i1_start": 10, "i1_end": 20}, out_b)

        with open(out_a, "rb") as f:
            data_a = f.read()
        with open(out_b, "rb") as f:
            data_b = f.read()

        self.assertEqual(len(data_a), len(data_b))
        self.assertNotEqual(data_a, data_b,
                            "Different stripes produced identical dither — RNG not seeded per stripe")


def _read_roots(path, degree):
    """Read root .bin file, return array of shape (n_steps, degree) of complex."""
    data = open(path, "rb").read()
    n_floats = len(data) // 4
    floats = struct.unpack(f'<{n_floats}f', data)
    stride = degree * 2
    n_steps = n_floats // stride
    roots = []
    for s in range(n_steps):
        step_roots = []
        for k in range(degree):
            re = floats[s * stride + k * 2]
            im = floats[s * stride + k * 2 + 1]
            step_roots.append(complex(re, im))
        roots.append(step_roots)
    return np.array(roots)


def _read_coeffs(path, n_coeffs):
    """Read coefficient .bin file, return array of shape (n_steps, n_coeffs) of complex."""
    data = open(path, "rb").read()
    n_floats = len(data) // 4
    floats = struct.unpack(f'<{n_floats}f', data)
    stride = n_coeffs * 2
    n_steps = n_floats // stride
    coeffs = np.empty((n_steps, n_coeffs), dtype=np.complex128)
    for s in range(n_steps):
        for k in range(n_coeffs):
            coeffs.real[s, k] = floats[s * stride + k * 2]
            coeffs.imag[s, k] = floats[s * stride + k * 2 + 1]
    return coeffs


def _read_param_pairs(path):
    data = open(path, "rb").read()
    n_floats = len(data) // 4
    floats = struct.unpack(f'<{n_floats}f', data)
    pairs = []
    for i in range(0, n_floats, 4):
        pairs.append((complex(floats[i], floats[i + 1]), complex(floats[i + 2], floats[i + 3])))
    return pairs


def _eval_poly(coeffs, z):
    """Evaluate polynomial with leading-first coefficients at z using Horner."""
    result = complex(0, 0)
    for c in coeffs:
        result = result * z + c
    return result


class TestSolverCorrectness(unittest.TestCase):
    """Verify roots are actually roots of the polynomial."""

    def test_known_polynomial(self):
        """Solve (z-1)(z-2)(z+3) = z^3 + 0z^2 - 7z + 6, verify roots ~ {1, 2, -3}."""
        # Leading-first: [1, 0, -7, 6]
        coeffs_data = struct.pack('<8f',
            1.0, 0.0,   # c[0] = 1
            0.0, 0.0,   # c[1] = 0
            -7.0, 0.0,  # c[2] = -7
            6.0, 0.0,   # c[3] = 6
        )
        coeffs_path = "/tmp/test_known_poly_coeffs.bin"
        with open(coeffs_path, "wb") as f:
            f.write(coeffs_data)

        solve_out = "/tmp/test_known_poly_roots.bin"
        meta = _run_sweep({
            "mode": "solve",
            "coeffs_file": coeffs_path,
            "n_coeffs": 4,
            "n2": 1,
            "i1_start": 0, "i1_end": 1,
        }, solve_out)

        self.assertEqual(meta["degree"], 3)
        roots = _read_roots(solve_out, 3)
        self.assertEqual(roots.shape, (1, 3))

        # Check that each root is close to one of {1, 2, -3}
        expected = {1.0, 2.0, -3.0}
        found = set()
        for r in roots[0]:
            self.assertAlmostEqual(r.imag, 0.0, places=3)
            best = min(expected - found, key=lambda e: abs(r.real - e))
            self.assertAlmostEqual(r.real, best, places=3,
                                   msg=f"Root {r} not close to any expected root")
            found.add(best)
        self.assertEqual(found, expected)

    def test_quadratic(self):
        """Solve z^2 - 5z + 6 = 0, roots should be 2 and 3."""
        coeffs_data = struct.pack('<6f',
            1.0, 0.0,   # z^2
            -5.0, 0.0,  # z
            6.0, 0.0,   # constant
        )
        coeffs_path = "/tmp/test_quadratic_coeffs.bin"
        with open(coeffs_path, "wb") as f:
            f.write(coeffs_data)

        solve_out = "/tmp/test_quadratic_roots.bin"
        meta = _run_sweep({
            "mode": "solve",
            "coeffs_file": coeffs_path,
            "n_coeffs": 3,
            "n2": 1,
            "i1_start": 0, "i1_end": 1,
        }, solve_out)

        roots = _read_roots(solve_out, 2)
        root_reals = sorted([r.real for r in roots[0]])
        self.assertAlmostEqual(root_reals[0], 2.0, places=3)
        self.assertAlmostEqual(root_reals[1], 3.0, places=3)

    def test_linear(self):
        """Solve 2z + 6 = 0, root should be -3."""
        coeffs_data = struct.pack('<4f', 2.0, 0.0, 6.0, 0.0)
        coeffs_path = "/tmp/test_linear_coeffs.bin"
        with open(coeffs_path, "wb") as f:
            f.write(coeffs_data)

        solve_out = "/tmp/test_linear_roots.bin"
        meta = _run_sweep({
            "mode": "solve",
            "coeffs_file": coeffs_path,
            "n_coeffs": 2,
            "n2": 1,
            "i1_start": 0, "i1_end": 1,
        }, solve_out)

        roots = _read_roots(solve_out, 1)
        self.assertAlmostEqual(roots[0, 0].real, -3.0, places=3)
        self.assertAlmostEqual(roots[0, 0].imag, 0.0, places=3)

    def test_complex_roots(self):
        """Solve z^2 + 1 = 0, roots should be +i and -i."""
        coeffs_data = struct.pack('<6f',
            1.0, 0.0,  # z^2
            0.0, 0.0,  # z
            1.0, 0.0,  # constant
        )
        coeffs_path = "/tmp/test_complex_roots_coeffs.bin"
        with open(coeffs_path, "wb") as f:
            f.write(coeffs_data)

        solve_out = "/tmp/test_complex_roots.bin"
        _run_sweep({
            "mode": "solve",
            "coeffs_file": coeffs_path,
            "n_coeffs": 3,
            "n2": 1,
            "i1_start": 0, "i1_end": 1,
        }, solve_out)

        roots = _read_roots(solve_out, 2)
        imags = sorted([r.imag for r in roots[0]])
        self.assertAlmostEqual(imags[0], -1.0, places=3)
        self.assertAlmostEqual(imags[1], 1.0, places=3)
        for r in roots[0]:
            self.assertAlmostEqual(r.real, 0.0, places=3)


class TestRoundTrip(unittest.TestCase):
    """Coeffgen → solve → verify p(root) ≈ 0."""

    def test_roundtrip_giga_30(self):
        """Generate coefficients, solve, verify |p(root)| is small."""
        cg_out = "/tmp/test_rt_cg.bin"
        meta_cg = _run_sweep({
            "mode": "coeffgen",
            "function": "giga_30",
            "param_transforms": [["unit_circle"]],
            "coeff_transforms": [],
            "n1": 5, "n2": 5,
            "i1_start": 0, "i1_end": 5,
        }, cg_out)

        n_coeffs = meta_cg["n_coeffs"]
        degree = meta_cg["degree"]

        solve_out = "/tmp/test_rt_solve.bin"
        _run_sweep({
            "mode": "solve",
            "coeffs_file": cg_out,
            "n_coeffs": n_coeffs,
            "n2": 5,
            "i1_start": 0, "i1_end": 5,
        }, solve_out)

        coeffs = _read_coeffs(cg_out, n_coeffs)
        roots = _read_roots(solve_out, degree)

        # Check a sample of steps
        n_steps = min(len(roots), 10)
        max_residual = 0.0
        for s in range(n_steps):
            for r in roots[s]:
                if not (math.isfinite(r.real) and math.isfinite(r.imag)):
                    continue
                val = _eval_poly(coeffs[s], r)
                residual = abs(val)
                max_residual = max(max_residual, residual)

        # float32 coefficients limit precision; 1e-2 is reasonable for degree ~9
        self.assertLess(max_residual, 1e-1,
                        f"Max |p(root)| = {max_residual:.2e}, expected < 0.1")


class TestCoeffTransforms(unittest.TestCase):
    """Test coefficient transforms produce expected results."""

    def _coeffgen(self, coeff_transforms, out_path):
        return _run_sweep({
            "mode": "coeffgen",
            "function": "giga_30",
            "param_transforms": [["unit_circle"]],
            "coeff_transforms": coeff_transforms,
            "n1": 3, "n2": 3,
            "i1_start": 0, "i1_end": 3,
        }, out_path)

    def _coeffgen_chunked_plain(self, params_out_path, coeff_out_path):
        _run_sweep({
            "mode": "param_gen",
            "n1": 3, "n2": 3,
            "times": 1,
            "param_transforms": [["unit_circle"]],
        }, params_out_path)
        return _run_sweep({
            "mode": "coeffgen_chunked",
            "function": "giga_30",
            "params_file": params_out_path,
            "step_start": 0,
            "step_count": 9,
            "coeff_transforms": [],
        }, coeff_out_path)

    @staticmethod
    def _as_written_complex64(values):
        arr = np.asarray(values, dtype=np.complex128).copy()
        bad = ~np.isfinite(arr.real) | ~np.isfinite(arr.imag)
        arr.real[bad] = 0.0
        arr.imag[bad] = 0.0
        return arr.astype(np.complex64).astype(np.complex128)

    def test_rev_reverses(self):
        """ct_rev reverses the coefficient order."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain.bin")
        meta_rev = self._coeffgen(["rev"], "/tmp/test_ct_rev.bin")

        n = meta_plain["n_coeffs"]
        self.assertEqual(n, meta_rev["n_coeffs"])

        plain = _read_coeffs("/tmp/test_ct_plain.bin", n)
        rev = _read_coeffs("/tmp/test_ct_rev.bin", n)

        # Each step's coefficients should be reversed
        for s in range(len(plain)):
            np.testing.assert_allclose(
                rev[s], plain[s][::-1], atol=1e-6,
                err_msg=f"Step {s}: rev transform did not reverse coefficients")

    def test_conj_negates_imag(self):
        """ct_conj negates imaginary parts."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain2.bin")
        meta_conj = self._coeffgen(["conj"], "/tmp/test_ct_conj.bin")

        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain2.bin", n)
        conj = _read_coeffs("/tmp/test_ct_conj.bin", n)

        for s in range(len(plain)):
            np.testing.assert_allclose(
                conj[s].real, plain[s].real, atol=1e-6)
            np.testing.assert_allclose(
                conj[s].imag, -plain[s].imag, atol=1e-6)

    def test_deriv_reduces_degree(self):
        """ct_deriv reduces degree by 1."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain3.bin")
        meta_deriv = self._coeffgen(["deriv"], "/tmp/test_ct_deriv.bin")

        self.assertEqual(meta_deriv["degree"], meta_plain["degree"] - 1)
        self.assertEqual(meta_deriv["n_coeffs"], meta_plain["n_coeffs"] - 1)

    def test_linear_multiplies(self):
        """linear defaults to old scale100 behavior: z*100+0."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain4.bin")
        meta_scaled = self._coeffgen(["linear"], "/tmp/test_ct_linear.bin")

        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain4.bin", n)
        scaled = _read_coeffs("/tmp/test_ct_linear.bin", n)

        for s in range(len(plain)):
            np.testing.assert_allclose(
                scaled[s], plain[s] * 100, rtol=1e-5,
                err_msg=f"Step {s}: linear did not multiply by 100")

    def test_linear_affine(self):
        """linear is the affine transform z*param1+param2."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain_linear.bin")
        meta_linear = self._coeffgen(
            [["linear", "1+3j", "1-1e-5-2j"]],
            "/tmp/test_ct_linear_affine.bin",
        )

        n = meta_plain["n_coeffs"]
        self.assertEqual(n, meta_linear["n_coeffs"])
        plain = _read_coeffs("/tmp/test_ct_plain_linear.bin", n)
        linear = _read_coeffs("/tmp/test_ct_linear_affine.bin", n)

        expected = self._as_written_complex64(plain * (1.0 + 3.0j) + (1.0 - 1e-5 - 2.0j))
        np.testing.assert_allclose(linear, expected, rtol=2e-5, atol=2e-5)

    def test_scale100_alias_remains_accepted(self):
        """Old saved coeff-transform chains can still use scale100."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain_scale100_alias.bin")
        meta_scaled = self._coeffgen(["scale100"], "/tmp/test_ct_scale100_alias.bin")

        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain_scale100_alias.bin", n)
        scaled = _read_coeffs("/tmp/test_ct_scale100_alias.bin", n)
        np.testing.assert_allclose(scaled, plain * 100, rtol=1e-5)

    def test_coeff_transform_andy_blends_with_original(self):
        """Final andy parameter blends f(z) back toward the original coefficients."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain_andy.bin")
        meta_blend = self._coeffgen([["linear", "100", "0", "1e-5"]], "/tmp/test_ct_linear_andy.bin")

        n = meta_plain["n_coeffs"]
        self.assertEqual(n, meta_blend["n_coeffs"])
        plain = _read_coeffs("/tmp/test_ct_plain_andy.bin", n)
        blended = _read_coeffs("/tmp/test_ct_linear_andy.bin", n)

        andy = 1e-5
        expected = self._as_written_complex64((plain * 100.0) * (1.0 - andy) + plain * andy)
        np.testing.assert_allclose(blended, expected, rtol=1e-5, atol=1e-5)

    def test_negate_odd(self):
        """ct_negate_odd negates odd-indexed coefficients."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain5.bin")
        meta_neg = self._coeffgen(["negate_odd"], "/tmp/test_ct_negodd.bin")

        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain5.bin", n)
        neg = _read_coeffs("/tmp/test_ct_negodd.bin", n)

        for s in range(len(plain)):
            for k in range(n):
                if k % 2 == 0:
                    self.assertAlmostEqual(abs(neg[s, k] - plain[s, k]), 0, places=5)
                else:
                    self.assertAlmostEqual(abs(neg[s, k] + plain[s, k]), 0, places=5)

    def test_sort_angle_keep_mod(self):
        """sort_angle_keep_mod sorts coefficient angles while preserving slot magnitudes."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain_sort_angle.bin")
        meta_sorted = self._coeffgen(["sort_angle_keep_mod"], "/tmp/test_ct_sort_angle.bin")

        n = meta_plain["n_coeffs"]
        self.assertEqual(n, meta_sorted["n_coeffs"])
        plain = _read_coeffs("/tmp/test_ct_plain_sort_angle.bin", n)
        sorted_angle = _read_coeffs("/tmp/test_ct_sort_angle.bin", n)

        expected = np.zeros_like(plain, dtype=np.complex128)
        for s in range(len(plain)):
            mods = np.abs(plain[s])
            angles = np.sort(np.angle(plain[s]))
            expected[s] = mods * (np.cos(angles) + 1j * np.sin(angles))
        expected = self._as_written_complex64(expected)

        np.testing.assert_allclose(sorted_angle, expected, rtol=2e-5, atol=2e-5)

    def test_power_matches_python_formula(self):
        """ct_power(k) matches the intended elementwise geometric-series formula."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain6.bin")
        meta_pow = self._coeffgen([["power", "6"]], "/tmp/test_ct_power6.bin")

        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain6.bin", n)
        powered = _read_coeffs("/tmp/test_ct_power6.bin", n)

        idx = np.arange(1, n + 1, dtype=np.complex128)
        expected = np.zeros_like(plain, dtype=np.complex128)
        for s in range(len(plain)):
            geom = np.ones(n, dtype=np.complex128)
            acc = np.ones(n, dtype=np.complex128)
            for _ in range(6):
                geom = geom * plain[s]
                acc = acc + geom
            expected[s] = acc * idx

        np.testing.assert_allclose(powered, expected, rtol=1e-5, atol=1e-5)

    def test_power_andy_is_linear_blend(self):
        """power(k, andy) linearly blends with the original coefficients."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain_power_andy.bin")
        meta_near_original = self._coeffgen(
            [["power", "6", "0.99999"]],
            "/tmp/test_ct_power6_andy099999.bin",
        )

        n = meta_plain["n_coeffs"]
        self.assertEqual(n, meta_near_original["n_coeffs"])
        plain = _read_coeffs("/tmp/test_ct_plain_power_andy.bin", n)
        near_original = _read_coeffs("/tmp/test_ct_power6_andy099999.bin", n)

        idx = np.arange(1, n + 1, dtype=np.complex128)
        transformed = np.zeros_like(plain, dtype=np.complex128)
        for s in range(len(plain)):
            geom = np.ones(n, dtype=np.complex128)
            acc = np.ones(n, dtype=np.complex128)
            for _ in range(6):
                geom = geom * plain[s]
                acc = acc + geom
            transformed[s] = acc * idx

        andy = 0.99999
        expected = self._as_written_complex64(transformed * (1.0 - andy) + plain * andy)
        np.testing.assert_allclose(near_original, expected, rtol=2e-5, atol=1e-5)

    def test_invpower_matches_python_formula(self):
        """ct_invpower(k) matches the intended reciprocal-threshold formula."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain7.bin")
        meta_invpow = self._coeffgen([["invpower", "4"]], "/tmp/test_ct_invpower4.bin")

        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain7.bin", n)
        invpowered = _read_coeffs("/tmp/test_ct_invpower4.bin", n)

        expected = np.zeros_like(plain, dtype=np.complex128)
        for s in range(len(plain)):
            geom = np.ones(n, dtype=np.complex128)
            acc = np.ones(n, dtype=np.complex128)
            for _ in range(4):
                geom = geom * plain[s]
                acc = acc + geom
            expected[s] = np.where(np.abs(acc) > 1.0, 1.0 / acc, 1.0 + 0.0j)

        np.testing.assert_allclose(invpowered, expected, rtol=1e-5, atol=1e-5)

    def test_exp_matches_python_formula(self):
        """exp(a,b) should match exp(z*(a+ib)) elementwise."""
        meta_plain = self._coeffgen_chunked_plain("/tmp/test_ct_exp_params.bin", "/tmp/test_ct_plain_exp.bin")
        meta_exp = self._coeffgen([["exp", "0.75", "-0.5"]], "/tmp/test_ct_exp.bin")

        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain_exp.bin", n)
        transformed = _read_coeffs("/tmp/test_ct_exp.bin", n)
        expected = self._as_written_complex64(np.exp(plain * (0.75 - 0.5j)))

        np.testing.assert_allclose(transformed, expected, rtol=1e-5, atol=1e-5)

    def test_trig_transforms_match_numpy(self):
        """cos/sin/tan should match NumPy complex trig elementwise."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain_trig.bin")
        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain_trig.bin", n)

        cases = [
            ("cos", np.cos, "/tmp/test_ct_cos.bin"),
            ("sin", np.sin, "/tmp/test_ct_sin.bin"),
            ("tan", np.tan, "/tmp/test_ct_tan.bin"),
        ]
        for name, fn, path in cases:
            self._coeffgen([name], path)
            actual = _read_coeffs(path, n)
            expected = self._as_written_complex64(fn(plain))
            np.testing.assert_allclose(actual, expected, rtol=5e-5, atol=5e-5, err_msg=name)

    def test_hyperbolic_transforms_match_numpy(self):
        """cosh/sinh/tanh should match NumPy complex hyperbolic functions."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain_hyper.bin")
        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain_hyper.bin", n)

        cases = [
            ("cosh", np.cosh, "/tmp/test_ct_cosh.bin"),
            ("sinh", np.sinh, "/tmp/test_ct_sinh.bin"),
            ("tanh", np.tanh, "/tmp/test_ct_tanh.bin"),
        ]
        for name, fn, path in cases:
            self._coeffgen([name], path)
            actual = _read_coeffs(path, n)
            expected = self._as_written_complex64(fn(plain))
            np.testing.assert_allclose(actual, expected, rtol=5e-5, atol=5e-5, err_msg=name)

    def test_cummax_tracks_running_maximum_by_magnitude(self):
        """cummax should keep the running maximum coefficient by magnitude."""
        meta_plain = self._coeffgen([], "/tmp/test_ct_plain_cummax.bin")
        meta_cummax = self._coeffgen(["cummax"], "/tmp/test_ct_cummax.bin")

        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain_cummax.bin", n)
        actual = _read_coeffs("/tmp/test_ct_cummax.bin", n)

        expected = np.zeros_like(plain, dtype=np.complex128)
        for s in range(len(plain)):
            best = plain[s, 0]
            best_mag = abs(best)
            for k in range(n):
                if abs(plain[s, k]) >= best_mag:
                    best = plain[s, k]
                    best_mag = abs(best)
                expected[s, k] = best

        np.testing.assert_allclose(actual, expected, rtol=1e-5, atol=1e-5)

    def test_round_matches_componentwise_complex_round(self):
        """round(a,b) should round the real and imaginary parts after affine scaling."""
        meta_plain = self._coeffgen_chunked_plain("/tmp/test_ct_round_params.bin", "/tmp/test_ct_plain_round.bin")
        meta_round = self._coeffgen([["round", "1.234", "-0.567"]], "/tmp/test_ct_round.bin")

        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain_round.bin", n)
        actual = _read_coeffs("/tmp/test_ct_round.bin", n)
        expected = self._as_written_complex64(np.round(plain * (1.234 - 0.567j)))

        np.testing.assert_allclose(actual, expected, rtol=1e-5, atol=1e-5)

    def test_pow_matches_python_formula_with_zero_guard(self):
        """pow(a,b,c,d) should match pow(z*(a+ib), c+id), mapping zero base to zero."""
        meta_plain = self._coeffgen_chunked_plain("/tmp/test_ct_pow_params.bin", "/tmp/test_ct_plain_pow.bin")
        meta_pow = self._coeffgen([["pow", "0.8", "0.3", "1.2", "-0.4"]], "/tmp/test_ct_pow.bin")

        n = meta_plain["n_coeffs"]
        plain = _read_coeffs("/tmp/test_ct_plain_pow.bin", n)
        actual = _read_coeffs("/tmp/test_ct_pow.bin", n)

        base = plain * (0.8 + 0.3j)
        expected = np.zeros_like(base, dtype=np.complex128)
        mask = np.abs(base) > 1e-30
        expected[mask] = np.power(base[mask], 1.2 - 0.4j)
        expected = self._as_written_complex64(expected)

        np.testing.assert_allclose(actual, expected, rtol=1e-5, atol=1e-5)

    def test_roots_hi_lo_only_change_zero_padding_side(self):
        """roots(k,hi|lo) should compute the same roots and only move the padding zero."""
        meta_hi = self._coeffgen([["roots", "5", "hi"]], "/tmp/test_ct_roots_hi.bin")
        meta_lo = self._coeffgen([["roots", "5", "lo"]], "/tmp/test_ct_roots_lo.bin")

        n = meta_hi["n_coeffs"]
        self.assertEqual(n, meta_lo["n_coeffs"])

        roots_hi = _read_coeffs("/tmp/test_ct_roots_hi.bin", n)
        roots_lo = _read_coeffs("/tmp/test_ct_roots_lo.bin", n)

        np.testing.assert_allclose(roots_hi[:, 1:], roots_lo[:, :-1], rtol=1e-5, atol=1e-5)
        np.testing.assert_allclose(roots_hi[:, 0], 0.0 + 0.0j, atol=1e-7)
        np.testing.assert_allclose(roots_lo[:, -1], 0.0 + 0.0j, atol=1e-7)


class TestParamTransforms(unittest.TestCase):
    """Test parameter transforms produce expected results."""

    def test_unit_circle_produces_varied_coefficients(self):
        """unit_circle param transform produces different coefficients across grid rows."""
        # Use poly_17 which does NOT apply unit_circle internally (CoeffFuncC),
        # so the external unit_circle transform is the only one.
        out = "/tmp/test_pt_uc.bin"
        meta = _run_sweep({
            "mode": "coeffgen",
            "function": "poly_17",
            "param_transforms": [["unit_circle"]],
            "coeff_transforms": [],
            "n1": 4, "n2": 4,
            "i1_start": 0, "i1_end": 4,
        }, out)

        n_coeffs = meta["n_coeffs"]
        coeffs = _read_coeffs(out, n_coeffs)
        self.assertEqual(meta["n_t"], 16)  # 4x4

        # Steps from different rows (different x1) should differ
        # Row 0 starts at step 0, row 1 starts at step 4
        self.assertFalse(np.allclose(coeffs[0], coeffs[4], atol=1e-6),
                         "Different x1 values should produce different coefficients")

    def test_no_transforms_vs_unit_circle(self):
        """Coefficients differ with and without unit_circle transform."""
        out_none = "/tmp/test_pt_none.bin"
        out_uc = "/tmp/test_pt_uc2.bin"

        _run_sweep({
            "mode": "coeffgen", "function": "giga_30",
            "param_transforms": [],
            "coeff_transforms": [],
            "n1": 4, "n2": 4,
            "i1_start": 0, "i1_end": 4,
        }, out_none)

        _run_sweep({
            "mode": "coeffgen", "function": "giga_30",
            "param_transforms": [["unit_circle"]],
            "coeff_transforms": [],
            "n1": 4, "n2": 4,
            "i1_start": 0, "i1_end": 4,
        }, out_uc)

        with open(out_none, "rb") as f:
            data_none = f.read()
        with open(out_uc, "rb") as f:
            data_uc = f.read()

        self.assertEqual(len(data_none), len(data_uc))
        self.assertNotEqual(data_none, data_uc,
                            "unit_circle should change the coefficients")

    def test_moebius_param_transform_zero_args_keeps_legacy_behavior(self):
        """Zero-arg moebius should preserve the old fixed 1/(t+2) map."""
        out = "/tmp/test_pt_moebius_legacy.bin"
        _run_sweep({
            "mode": "param_dump",
            "param_transforms": [["moebius"]],
            "n1": 2, "n2": 2,
            "i1_start": 0, "i1_end": 2,
        }, out)

        pairs = _read_param_pairs(out)
        self.assertAlmostEqual(pairs[0][0].real, 0.5, places=6)
        self.assertAlmostEqual(pairs[0][0].imag, 0.0, places=6)
        self.assertAlmostEqual(pairs[0][1].real, 0.5, places=6)
        self.assertAlmostEqual(pairs[0][1].imag, 0.0, places=6)
        self.assertAlmostEqual(pairs[1][1].real, 0.4, places=6)
        self.assertAlmostEqual(pairs[2][0].real, 0.4, places=6)

    def test_moebius_param_transform_identity_coefficients_are_noop(self):
        """Four-arg moebius should honor coefficients and identity should be exact."""
        out = "/tmp/test_pt_moebius_identity.bin"
        _run_sweep({
            "mode": "param_dump",
            "param_transforms": [["moebius", "1", "0", "0", "1"]],
            "n1": 2, "n2": 2,
            "i1_start": 0, "i1_end": 2,
        }, out)

        pairs = _read_param_pairs(out)
        expected = [
            (complex(0.0, 0.0), complex(0.0, 0.0)),
            (complex(0.0, 0.0), complex(0.5, 0.0)),
            (complex(0.5, 0.0), complex(0.0, 0.0)),
            (complex(0.5, 0.0), complex(0.5, 0.0)),
        ]
        self.assertEqual(pairs, expected)

    def test_moebius_param_transform_accepts_complex_scientific_literals(self):
        """Four-arg moebius should parse complex literals in either term order."""
        out = "/tmp/test_pt_moebius_complex.bin"
        _run_sweep({
            "mode": "param_dump",
            "param_transforms": [["moebius", "1e0-3e0j", "1i+3", "0", "1"]],
            "n1": 2, "n2": 1,
            "i1_start": 0, "i1_end": 2,
        }, out)

        pairs = _read_param_pairs(out)
        self.assertAlmostEqual(pairs[0][0].real, 3.0, places=6)
        self.assertAlmostEqual(pairs[0][0].imag, 1.0, places=6)
        self.assertAlmostEqual(pairs[1][0].real, 3.5, places=6)
        self.assertAlmostEqual(pairs[1][0].imag, -0.5, places=6)

    def test_add_param_transform_accepts_two_complex_offsets(self):
        """Two-arg add should offset z1 and z2 independently with complex literals."""
        out = "/tmp/test_pt_add_complex.bin"
        _run_sweep({
            "mode": "param_dump",
            "param_transforms": [["add", "1-3j", "1i+3"]],
            "n1": 2, "n2": 1,
            "i1_start": 0, "i1_end": 2,
        }, out)

        pairs = _read_param_pairs(out)
        self.assertAlmostEqual(pairs[0][0].real, 1.0, places=6)
        self.assertAlmostEqual(pairs[0][0].imag, -3.0, places=6)
        self.assertAlmostEqual(pairs[0][1].real, 3.0, places=6)
        self.assertAlmostEqual(pairs[0][1].imag, 1.0, places=6)
        self.assertAlmostEqual(pairs[1][0].real, 1.5, places=6)
        self.assertAlmostEqual(pairs[1][0].imag, -3.0, places=6)

    def test_add_param_transform_one_arg_keeps_legacy_behavior(self):
        """One-arg add should preserve the old add-to-all-components behavior."""
        out = "/tmp/test_pt_add_legacy.bin"
        _run_sweep({
            "mode": "param_dump",
            "param_transforms": [["add", "1"]],
            "n1": 1, "n2": 2,
            "i1_start": 0, "i1_end": 1,
        }, out)

        pairs = _read_param_pairs(out)
        self.assertEqual(pairs[0], (complex(1.0, 1.0), complex(1.0, 1.0)))
        self.assertEqual(pairs[1], (complex(1.0, 1.0), complex(1.5, 1.0)))


class TestEdgeCases(unittest.TestCase):
    """Edge case tests for the solver."""

    def test_degree_1(self):
        """Degree-1 polynomial is solved analytically."""
        # 3z + 9 = 0 → z = -3
        coeffs_data = struct.pack('<4f', 3.0, 0.0, 9.0, 0.0)
        with open("/tmp/test_deg1_coeffs.bin", "wb") as f:
            f.write(coeffs_data)

        solve_out = "/tmp/test_deg1_roots.bin"
        meta = _run_sweep({
            "mode": "solve",
            "coeffs_file": "/tmp/test_deg1_coeffs.bin",
            "n_coeffs": 2,
            "n2": 1,
            "i1_start": 0, "i1_end": 1,
        }, solve_out)

        self.assertEqual(meta["degree"], 1)
        roots = _read_roots(solve_out, 1)
        self.assertAlmostEqual(roots[0, 0].real, -3.0, places=3)

    def test_all_zero_coefficients(self):
        """All-zero coefficients should not crash."""
        coeffs_data = struct.pack('<8f', 0, 0, 0, 0, 0, 0, 0, 0)
        with open("/tmp/test_allzero_coeffs.bin", "wb") as f:
            f.write(coeffs_data)

        solve_out = "/tmp/test_allzero_roots.bin"
        meta = _run_sweep({
            "mode": "solve",
            "coeffs_file": "/tmp/test_allzero_coeffs.bin",
            "n_coeffs": 4,
            "n2": 1,
            "i1_start": 0, "i1_end": 1,
        }, solve_out)

        # Should produce roots (all zero), not crash
        self.assertEqual(meta["n_t"], 1)

    def test_high_degree(self):
        """High degree polynomial (200+) doesn't crash."""
        n_coeffs = 201  # degree 200
        # Build z^200 + 1 = 0 (roots are 200th roots of -1)
        coeffs = [0.0] * (n_coeffs * 2)
        coeffs[0] = 1.0  # leading coefficient (z^200)
        coeffs[(n_coeffs - 1) * 2] = 1.0  # constant term
        coeffs_data = struct.pack(f'<{n_coeffs * 2}f', *coeffs)

        with open("/tmp/test_highdeg_coeffs.bin", "wb") as f:
            f.write(coeffs_data)

        solve_out = "/tmp/test_highdeg_roots.bin"
        meta = _run_sweep({
            "mode": "solve",
            "coeffs_file": "/tmp/test_highdeg_coeffs.bin",
            "n_coeffs": n_coeffs,
            "n2": 1,
            "i1_start": 0, "i1_end": 1,
        }, solve_out)

        self.assertEqual(meta["degree"], 200)
        roots = _read_roots(solve_out, 200)
        # All roots should have magnitude close to 1 (roots of unity * phase)
        mags = np.abs(roots[0])
        self.assertTrue(np.all(mags > 0.5), "Some roots have suspiciously small magnitude")
        self.assertTrue(np.all(mags < 2.0), "Some roots have suspiciously large magnitude")

    def test_multiple_steps(self):
        """Multiple coefficient sets in one file are all solved."""
        # Two copies of z^2 - 1 = 0 (roots: +1, -1)
        step = struct.pack('<6f', 1, 0, 0, 0, -1, 0)
        with open("/tmp/test_multi_coeffs.bin", "wb") as f:
            f.write(step * 5)

        solve_out = "/tmp/test_multi_roots.bin"
        meta = _run_sweep({
            "mode": "solve",
            "coeffs_file": "/tmp/test_multi_coeffs.bin",
            "n_coeffs": 3,
            "n2": 1,
            "i1_start": 0, "i1_end": 1,
        }, solve_out)

        self.assertEqual(meta["n_t"], 5)
        roots = _read_roots(solve_out, 2)
        for s in range(5):
            reals = sorted([r.real for r in roots[s]])
            self.assertAlmostEqual(reals[0], -1.0, places=3)
            self.assertAlmostEqual(reals[1], 1.0, places=3)


class TestTimesWithoutDither(unittest.TestCase):
    """Verify times > 1 without dither produces identical passes."""

    def test_times3_no_dither_identical_passes(self):
        """Without sdith, times=3 should produce 3 identical copies of the same coefficients."""
        out = "/tmp/test_nodith_t3.bin"
        meta = _run_sweep({
            "mode": "coeffgen",
            "function": "giga_30",
            "param_transforms": [["unit_circle"]],  # no sdith
            "coeff_transforms": [],
            "n1": 5, "n2": 5,
            "i1_start": 0, "i1_end": 5,
            "times": 3,
        }, out)

        n_coeffs = meta["n_coeffs"]
        coeffs = _read_coeffs(out, n_coeffs)
        steps_per_pass = 25  # 5*5
        self.assertEqual(len(coeffs), 75)  # 3 * 25

        pass0 = coeffs[:steps_per_pass]
        pass1 = coeffs[steps_per_pass:2*steps_per_pass]
        pass2 = coeffs[2*steps_per_pass:]

        np.testing.assert_allclose(pass0, pass1, atol=1e-6,
                                   err_msg="Pass 0 and pass 1 differ without dither")
        np.testing.assert_allclose(pass0, pass2, atol=1e-6,
                                   err_msg="Pass 0 and pass 2 differ without dither")


class TestGridVsCoeffgenSolve(unittest.TestCase):
    """Grid mode and coeffgen+solve should produce the same roots."""

    def test_equivalence(self):
        """Grid mode roots match coeffgen→solve roots (same polynomial, same grid)."""
        # Grid mode
        grid_out = "/tmp/test_equiv_grid.bin"
        meta_grid = _run_sweep({
            "mode": "grid",
            "function": "giga_30",
            "n1": 5, "n2": 5,
            "i1_start": 0, "i1_end": 5,
            "match_roots": False,
        }, grid_out)

        # Coeffgen + solve must use the legacy old_giga_30 wrapper.
        # Grid mode still dispatches through lookupFunction() and calls the
        # original old-ABI giga_30(x1, x2) implementation directly. The bare
        # "giga_30" coeffgen entry now points at the newer transpiled catalog
        # implementation, which is a different polynomial family.
        cg_out = "/tmp/test_equiv_cg.bin"
        meta_cg = _run_sweep({
            "mode": "coeffgen",
            "function": "old_giga_30",
            "param_transforms": [],
            "coeff_transforms": [],
            "n1": 5, "n2": 5,
            "i1_start": 0, "i1_end": 5,
        }, cg_out)

        solve_out = "/tmp/test_equiv_solve.bin"
        _run_sweep({
            "mode": "solve",
            "coeffs_file": cg_out,
            "n_coeffs": meta_cg["n_coeffs"],
            "n2": 5,
            "i1_start": 0, "i1_end": 5,
            "match_roots": False,
        }, solve_out)

        degree = meta_grid["degree"]
        grid_roots = _read_roots(grid_out, degree)
        solve_roots = _read_roots(solve_out, degree)

        self.assertEqual(grid_roots.shape, solve_roots.shape)

        # Compare root SETS per step (order may differ)
        for s in range(len(grid_roots)):
            # Sort by real part then imaginary for deterministic comparison
            gr = sorted(grid_roots[s], key=lambda z: (round(z.real, 4), round(z.imag, 4)))
            sr = sorted(solve_roots[s], key=lambda z: (round(z.real, 4), round(z.imag, 4)))
            for g, sv in zip(gr, sr):
                self.assertAlmostEqual(g.real, sv.real, places=2,
                                       msg=f"Step {s}: grid root {g} != solve root {sv}")
                self.assertAlmostEqual(g.imag, sv.imag, places=2,
                                       msg=f"Step {s}: grid root {g} != solve root {sv}")


if __name__ == "__main__":
    unittest.main()
