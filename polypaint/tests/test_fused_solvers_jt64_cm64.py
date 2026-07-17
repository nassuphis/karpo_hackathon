"""
JT64 / CM64: the fused solver modes — Jenkins-Traub and companion-matrix
run with the AE sweep's pipeline shape: the row's f64 coefficients go
straight from the generation loop into the f64 solver core
(solve_jt_coeffs_f64 / solve_companion_coeffs_ws_f64), no coefficient
file and no cast before the f32 ROOT output. The split pipeline solves
from the f32 coefficient file instead, which is exactly what shatters
clustered roots by sqrt(6e-8) ~ 2e-4.

Measured at authoring (exact double root among moving background
roots): FUSED error 1.3e-8 (jt64) / 2.7e-8 (cm64) — the f32
root-output cast floor — vs SPLIT-equivalent 1.4e-4: four orders of
magnitude. Well-conditioned rows agree with np.roots of the emitted
f32 row at ~1.4e-7 (the coefficient-file truncation both would share).

Also pinned: knife parity (rows whose f64 coefficients exceed the f32
transport range are skipped with zeroed root rows, mirroring what the
split path does with inf rows), meta fields, and the cm64-requires-
LAPACK refusal in the plain build.
"""
import json
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
SWEEP_TEST_LAPACK = "/tmp/polypaint_sweep_test_lapack"
sys.path.insert(0, LAMBDA_DIR)

# exact double root at A among moving background roots; the scan's
# floor(k/2) gate keeps slots 0,1 undisplaced so the double is EXACT
A = 0.7 + 0.4j
CLUSTER_SRC = """poly = vector_literal(0.7+0.4i, 0.7+0.4i, 2.1, -1.7+0.9i, 0.3-1.8i, -2.2-0.6i, 1.4+1.5i)
poly = add(poly, scan(7, 0, 0.001*exp(6.283185307179586i*t1)*floor(k/2), prev))
poly = expand_roots(poly)
emit
"""

# a smooth degree-11 family for well-conditioned parity
SMOOTH_SRC = """poly = fill(12, 0)
poly[0] = 1
poly[1] = 2*exp(6.283185307179586i*t1)
poly[3] = 0-1.5*exp(6.283185307179586i*t2)
poly[7] = 0.5*t1+0.25
poly[11] = 0-2
emit
"""

# rows overflow f32 transport (coefficients ~1e40) -> knife parity
KNIFE_SRC = """poly = fill(3, 0)
poly[0] = 1
poly[1] = 1e40
poly[2] = 1
emit
"""


def _ensure_lapack_binary():
    if sys.platform != "darwin":
        return os.path.exists(SWEEP_TEST_LAPACK)
    src = os.path.join(LAMBDA_DIR, "sweep_cli.c")
    hdr = os.path.join(LAMBDA_DIR, "companion_solver.h")
    if os.path.exists(SWEEP_TEST_LAPACK):
        built = os.path.getmtime(SWEEP_TEST_LAPACK)
        if built >= os.path.getmtime(src) and built >= os.path.getmtime(hdr):
            return True
    proc = subprocess.run(
        ["cc", "-O2", "-pthread", "-DHAVE_LAPACK_COMPANION",
         "-DPOLYPAINT_ACCELERATE_NEWLAPACK",
         "-o", SWEEP_TEST_LAPACK, src, "-framework", "Accelerate", "-lm"],
        capture_output=True, text=True,
    )
    return proc.returncode == 0


HAVE_LAPACK_BINARY = _ensure_lapack_binary()


def _compiled(src):
    from coeff_program_source import compile_coeff_program_source

    c = compile_coeff_program_source(src)
    payload = {k: c[k] for k in ("fingerprint", "tokens", "stack_max",
                                 "scalar_exprs", "vector_constants")}
    payload["version"] = 1
    return payload


def _run_grid(binary, program, solver, tag, n=8):
    spec = {
        "mode": "coeffgen",
        "function": "const",
        "cfpv": [1, 0, 0],
        "n1": n, "n2": n, "i1_start": 0, "i1_end": n, "times": 1,
        "param_transforms": [], "coeff_transforms": [],
        "coeff_program": program,
        "fused_solver": solver,
        "roots_file": f"/tmp/fused64_{tag}_roots.bin",
    }
    proc = subprocess.run(
        [binary, f"/tmp/fused64_{tag}_coeffs.bin"],
        input=json.dumps(spec), capture_output=True, text=True, timeout=120)
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:300])
    return json.loads(proc.stdout)


def _load(tag, degree):
    import numpy as np

    roots = np.fromfile(f"/tmp/fused64_{tag}_roots.bin", dtype=np.float32)
    coeffs = np.fromfile(f"/tmp/fused64_{tag}_coeffs.bin", dtype=np.float32)
    return (roots.reshape(-1, degree, 2), coeffs.reshape(-1, degree + 1, 2))


class TestFusedSolverCores(unittest.TestCase):
    """Solver-level checks that run in any build (jt64) or the LAPACK
    build (cm64)."""

    def _cluster_case(self, binary, solver):
        import numpy as np

        meta = _run_grid(binary, _compiled(CLUSTER_SRC), solver, solver)
        self.assertEqual(meta["fused_solver"], solver)
        self.assertEqual(int(meta["solve_skipped"]), 0)
        roots, coeffs = _load(solver, 7)
        self.assertEqual(len(roots), 64)
        fused_err, split_err = [], []
        for i in range(len(roots)):
            got = roots[i, :, 0].astype(float) + 1j * roots[i, :, 1].astype(float)
            fused_err += [abs(z - A) for z in sorted(got, key=lambda z: abs(z - A))[:2]]
            cf32 = coeffs[i, :, 0].astype(float) + 1j * coeffs[i, :, 1].astype(float)
            split = sorted(np.roots(cf32), key=lambda z: abs(z - A))[:2]
            split_err += [abs(z - A) for z in split]
        # the point of the whole exercise: the fused solver keeps the
        # double root together at the f32 ROOT-output floor, while the
        # split path (roots of the f32 coefficient row) shatters it
        self.assertLess(max(fused_err), 5e-7)
        self.assertGreater(float(np.median(split_err)), 1e-5)

    def _smooth_case(self, binary, solver):
        import numpy as np

        _run_grid(binary, _compiled(SMOOTH_SRC), solver, solver + "_smooth")
        roots, coeffs = _load(solver + "_smooth", 11)
        worst = 0.0
        for i in range(len(roots)):
            cf = coeffs[i, :, 0].astype(float) + 1j * coeffs[i, :, 1].astype(float)
            got = roots[i, :, 0].astype(float) + 1j * roots[i, :, 1].astype(float)
            pool = list(np.roots(cf))
            for g in got:
                j = int(np.argmin([abs(g - p) for p in pool]))
                worst = max(worst, abs(g - pool.pop(j)))
        self.assertLess(worst, 1e-5)

    def _knife_case(self, binary, solver):
        import numpy as np

        meta = _run_grid(binary, _compiled(KNIFE_SRC), solver, solver + "_knife", n=3)
        # every row exceeds the f32 transport range -> knifed, mirroring
        # the split path where those rows arrive as inf
        self.assertEqual(int(meta["solve_skipped"]), 9)
        roots, _ = _load(solver + "_knife", 2)
        self.assertTrue(np.all(roots == 0.0))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_jt64_cluster_smooth_and_knife(self):
        self._cluster_case(SWEEP_TEST, "jt64")
        self._smooth_case(SWEEP_TEST, "jt64")
        self._knife_case(SWEEP_TEST, "jt64")

    @unittest.skipUnless(HAVE_LAPACK_BINARY, "LAPACK sweep_test build unavailable")
    def test_cm64_cluster_smooth_and_knife(self):
        self._cluster_case(SWEEP_TEST_LAPACK, "cm64")
        self._smooth_case(SWEEP_TEST_LAPACK, "cm64")
        self._knife_case(SWEEP_TEST_LAPACK, "cm64")

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_cm64_requires_lapack_build(self):
        spec = {
            "mode": "coeffgen", "function": "const", "cfpv": [1, 0, 0],
            "n1": 2, "n2": 2, "i1_start": 0, "i1_end": 2, "times": 1,
            "param_transforms": [], "coeff_transforms": [],
            "coeff_program": _compiled(SMOOTH_SRC),
            "fused_solver": "cm64", "roots_file": "/tmp/fused64_nolapack.bin",
        }
        proc = subprocess.run(
            [SWEEP_TEST, "/tmp/fused64_nolapack_coeffs.bin"],
            input=json.dumps(spec), capture_output=True, text=True, timeout=60)
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("LAPACK", proc.stderr)


class TestFusedSolverPlumbing(unittest.TestCase):
    def test_chunk_handler_accepts_and_routes_fused_modes(self):
        sys.path.insert(0, LAMBDA_DIR)
        import handler_compute_chunk_fused as mod

        self.assertEqual(mod.FUSED_64_MODES, {"jt64", "cm64"})

    def test_preview_solver_tags(self):
        import handler_compute_preview as mod

        self.assertEqual(mod._solver_tag("jt64"), "JT64")
        self.assertEqual(mod._solver_tag("cm64"), "CM64")


if __name__ == "__main__":
    unittest.main()
