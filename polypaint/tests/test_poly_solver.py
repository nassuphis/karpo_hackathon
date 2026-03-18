"""
Test that sweep solver roots match numpy.roots for the same coefficients.

Pipeline: coeffgen produces coefficients, solve finds roots.
We read both outputs and verify that np.roots(coeffs) ≈ sweep roots
for each grid step.

Run: cd polypaint && uv run python -m pytest tests/test_poly_solver.py -v

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


def _read_f32_complex(path, n_per_step):
    """Read binary file of float32 re/im pairs, return list of complex arrays."""
    data = open(path, "rb").read()
    n_floats = len(data) // 4
    floats = struct.unpack(f'<{n_floats}f', data)
    stride = n_per_step * 2
    n_steps = n_floats // stride
    result = []
    for s in range(n_steps):
        step = np.array([
            complex(floats[s * stride + k * 2], floats[s * stride + k * 2 + 1])
            for k in range(n_per_step)
        ])
        result.append(step)
    return result


def _match_root_sets(roots_a, roots_b):
    """Match two sets of roots by nearest-neighbor. Returns max distance."""
    if len(roots_a) == 0:
        return 0.0
    used = set()
    max_dist = 0.0
    for ra in roots_a:
        best_j = None
        best_d = float('inf')
        for j, rb in enumerate(roots_b):
            if j in used:
                continue
            d = abs(ra - rb)
            if d < best_d:
                best_d = d
                best_j = j
        if best_j is not None:
            used.add(best_j)
            max_dist = max(max_dist, best_d)
    return max_dist


class TestPolySolver(unittest.TestCase):
    """Compare sweep solver roots with numpy.roots for real coefficient pipelines."""

    def _run_pipeline(self, function, param_transforms, coeff_transforms, n):
        """Run coeffgen + solve, return (coeffs_list, roots_list, degree)."""
        cg_out = "/tmp/test_ps_cg.bin"
        meta_cg = _run_sweep({
            "mode": "coeffgen",
            "function": function,
            "param_transforms": param_transforms,
            "coeff_transforms": coeff_transforms,
            "n1": n, "n2": n,
            "i1_start": 0, "i1_end": n,
        }, cg_out)

        n_coeffs = meta_cg["n_coeffs"]
        degree = meta_cg["degree"]

        solve_out = "/tmp/test_ps_solve.bin"
        meta_solve = _run_sweep({
            "mode": "solve",
            "coeffs_file": cg_out,
            "n_coeffs": n_coeffs,
            "n2": n,
            "i1_start": 0, "i1_end": n,
            "match_roots": False,
        }, solve_out)

        coeffs_list = _read_f32_complex(cg_out, n_coeffs)
        roots_list = _read_f32_complex(solve_out, degree)

        return coeffs_list, roots_list, degree, n_coeffs

    def _compare_roots(self, coeffs_list, roots_list, label, tol=1.0):
        """Compare sweep roots with np.roots for each step. Returns max distance."""
        n_steps = len(coeffs_list)
        max_dist_all = 0.0
        compared = 0
        failures = []

        for s in range(n_steps):
            coeffs = coeffs_list[s]

            # Strip leading zeros to find effective degree (same as C solver)
            first_nonzero = 0
            while first_nonzero < len(coeffs) - 1 and abs(coeffs[first_nonzero]) < 1e-10:
                first_nonzero += 1
            effective_coeffs = coeffs[first_nonzero:]

            if len(effective_coeffs) < 2:
                continue  # constant or empty — no roots

            np_roots = np.roots(effective_coeffs)
            sweep_roots = roots_list[s]

            sweep_finite = np.array([r for r in sweep_roots
                                     if math.isfinite(r.real) and math.isfinite(r.imag) and abs(r) < 1e6])
            np_finite = np.array([r for r in np_roots if np.isfinite(r) and abs(r) < 1e6])

            if len(sweep_finite) == 0 or len(np_finite) == 0:
                continue

            compared += 1
            max_dist = _match_root_sets(sweep_finite, np_finite)
            max_dist_all = max(max_dist_all, max_dist)

            if max_dist > tol:
                failures.append((s, max_dist, effective_coeffs[:5], sweep_finite[:3], np_finite[:3]))

        if failures:
            msg_parts = []
            for s, d, c, sr, nr in failures[:5]:
                msg_parts.append(
                    f"  step {s}: max_dist={d:.4f}\n"
                    f"    coeffs[:5]={c}\n"
                    f"    sweep[:3]={sr}\n"
                    f"    numpy[:3]={nr}"
                )
            self.fail(
                f"{label}: {len(failures)}/{n_steps} steps have max_dist > {tol}:\n"
                + "\n".join(msg_parts)
            )

        print(f"\n  {label}: max root distance = {max_dist_all:.2e} "
              f"({compared}/{n_steps} steps compared)")
        return max_dist_all

    def test_unit_circle_poly27_rev(self):
        """[unit_circle] poly_27 [rev] — 4x4 grid (16 points), compare with np.roots."""
        coeffs_list, roots_list, degree, n_coeffs = self._run_pipeline(
            function="poly_27",
            param_transforms=[["unit_circle"]],
            coeff_transforms=["rev"],
            n=4,
        )

        self.assertEqual(len(coeffs_list), 16)
        self.assertEqual(len(roots_list), 16)

        self._compare_roots(coeffs_list, roots_list,
                            "poly_27 [unit_circle] [rev] 4x4", tol=5.0)

    def test_unit_circle_poly16_no_transforms(self):
        """[unit_circle] poly_16 [] — 3x3 grid, compare with np.roots."""
        coeffs_list, roots_list, degree, n_coeffs = self._run_pipeline(
            function="poly_16",
            param_transforms=[["unit_circle"]],
            coeff_transforms=[],
            n=3,
        )

        self.assertEqual(len(coeffs_list), 9)
        # Degree 50 with float32 coefficients: some roots are ill-conditioned.
        self._compare_roots(coeffs_list, roots_list,
                            "poly_16 [unit_circle] [] 3x3", tol=5.0)

    def test_unit_circle_giga30_rev(self):
        """[unit_circle] giga_30 [rev] — 3x3 grid, low degree (9), should be very accurate."""
        coeffs_list, roots_list, degree, n_coeffs = self._run_pipeline(
            function="giga_30",
            param_transforms=[["unit_circle"]],
            coeff_transforms=["rev"],
            n=3,
        )

        self.assertEqual(len(coeffs_list), 9)
        self.assertEqual(degree, 9)
        # Low degree should be very accurate
        self._compare_roots(coeffs_list, roots_list,
                            "giga_30 [unit_circle] [rev] 3x3", tol=0.1)


if __name__ == "__main__":
    unittest.main()
