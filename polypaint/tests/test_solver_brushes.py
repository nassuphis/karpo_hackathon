"""
Solver-brush wave: Jenkins-Traub (CPOLY port) and Newton+deflation row
loops inside sweep_cm, plus the capped-Aberth max_iter knob in sweep_mt.

The brushes trade accuracy for speed and TEXTURE (docs/solver-brushes.md):
errors are paint here, so these tests pin two different things —
  * correctness where the solver is SUPPOSED to be right (JT everywhere,
    Newton on easy well-conditioned inputs), and
  * determinism everywhere (byte-identical across runs and thread counts),
    because a brush that repaints differently per run is useless.
Local macOS builds against Accelerate; the deployed netlib lineage is
covered by scripts/test-roots-cm-strip-docker.sh.
"""
import json
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_CM_LOCAL = "/tmp/polypaint_sweep_cm_brush_test"
SWEEP_MT_LOCAL = "/tmp/polypaint_sweep_mt_brush_test"


def _ensure_binary(out_path, src_name, extra_headers, cmd_tail):
    src = os.path.join(LAMBDA_DIR, src_name)
    deps = [src] + [os.path.join(LAMBDA_DIR, h) for h in extra_headers]
    if os.path.exists(out_path):
        built = os.path.getmtime(out_path)
        if all(built >= os.path.getmtime(d) for d in deps):
            return True
    proc = subprocess.run(
        ["cc", "-O3", "-pthread", "-o", out_path, src] + cmd_tail,
        capture_output=True, text=True,
    )
    return proc.returncode == 0


def _ensure_binaries():
    if sys.platform != "darwin":
        return False
    cm = _ensure_binary(
        SWEEP_CM_LOCAL, "sweep_cm.c",
        ["companion_solver.h", "jt_solver.h", "newton_solver.h"],
        ["-DPOLYPAINT_ACCELERATE_NEWLAPACK", "-framework", "Accelerate", "-lm"],
    )
    mt = _ensure_binary(SWEEP_MT_LOCAL, "sweep_mt.c", [], ["-lm"])
    return cm and mt


HAVE_BINARIES = _ensure_binaries()


def _write_rows(path, cf):
    import numpy as np

    rows, nc = cf.shape
    inter = np.empty((rows, nc * 2), dtype=np.float32)
    inter[:, 0::2] = cf.real
    inter[:, 1::2] = cf.imag
    inter.tofile(path)


def _random_rows(seed, rows, n_coeffs):
    import numpy as np

    rng = np.random.default_rng(seed)
    return (rng.standard_normal((rows, n_coeffs))
            + 1j * rng.standard_normal((rows, n_coeffs))).astype(np.complex64)


def _run_cm_binary(mode, coeffs_path, n_coeffs, out_path, n_threads=1):
    spec = {"mode": mode, "coeffs_file": coeffs_path,
            "n_coeffs": n_coeffs, "n_threads": n_threads}
    proc = subprocess.run(
        [SWEEP_CM_LOCAL, out_path], input=json.dumps(spec),
        capture_output=True, text=True, timeout=300,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:400])
    return json.loads(proc.stdout)


def _roots_from(path, rows, degree):
    import numpy as np

    raw = np.fromfile(path, dtype=np.float32).reshape(rows, degree, 2)
    return raw[..., 0] + 1j * raw[..., 1]


def _multiset_rel_err(got, want):
    """Greedy nearest matching over the root multisets; adequate at the
    tolerances asserted here (mismatches would surface as O(1))."""
    import numpy as np

    pool = list(want)
    worst = 0.0
    for g in got:
        j = int(np.argmin([abs(g - w) for w in pool]))
        w = pool.pop(j)
        worst = max(worst, abs(g - w) / max(1.0, abs(w)))
    return worst


@unittest.skipUnless(HAVE_BINARIES, "local solver binaries unavailable")
class TestJenkinsTraubBrush(unittest.TestCase):
    def test_jt_matches_np_roots_on_random_rows(self):
        """The CPOLY port is a REAL solver: on generic random degree-35
        rows it must agree with np.roots to f32 I/O precision (measured
        worst 5.8e-8 at authoring; 1e-6 leaves margin, order-of-magnitude
        errors would mean a port bug)."""
        import numpy as np

        rows, nc = 64, 36
        cf = _random_rows(11, rows, nc)
        coeffs = "/tmp/brush_jt_parity.bin"
        _write_rows(coeffs, cf)
        meta = _run_cm_binary("solve_jt", coeffs, nc, "/tmp/brush_jt_parity_out.bin")
        self.assertEqual(meta["mode"], "solve_jt")
        got = _roots_from("/tmp/brush_jt_parity_out.bin", rows, nc - 1)
        worst = 0.0
        for r in range(rows):
            ref = np.roots(cf[r].astype(np.complex128))
            worst = max(worst, _multiset_rel_err(got[r], ref))
        self.assertLess(worst, 1e-6, f"JT diverged from np.roots: {worst}")

    def test_jt_guard_rows_and_leading_strip(self):
        """Pipeline guard rows survive the JT path: all-zero row -> zero
        roots; inf row -> skipped (zeros); exact leading zeros strip like
        solve_companion_coeffs (degree drops, tail slots stay zero)."""
        import numpy as np

        rows, nc = 8, 12
        cf = _random_rows(5, rows, nc)
        cf[2] = 0
        cf[4, 0] = np.inf
        cf[6, :5] = 0
        coeffs = "/tmp/brush_jt_guards.bin"
        _write_rows(coeffs, cf)
        meta = _run_cm_binary("solve_jt", coeffs, nc, "/tmp/brush_jt_guards_out.bin")
        self.assertEqual(meta["skipped_overflow"], 1)
        got = _roots_from("/tmp/brush_jt_guards_out.bin", rows, nc - 1)
        self.assertTrue(np.all(got[2] == 0))
        self.assertTrue(np.all(got[4] == 0))
        ref = np.roots(cf[6].astype(np.complex128))
        self.assertEqual(len(ref), nc - 1 - 5)
        self.assertLess(_multiset_rel_err(got[6][: len(ref)], ref), 1e-6)
        self.assertTrue(np.all(got[6][len(ref):] == 0))


@unittest.skipUnless(HAVE_BINARIES, "local solver binaries unavailable")
class TestNewtonBrush(unittest.TestCase):
    def test_newton_exact_on_easy_low_degree(self):
        """Newton+deflation from the fixed seed must be RIGHT on easy
        inputs (degree 5, well-conditioned): measured worst 5.7e-8 at
        authoring. If this breaks, the deflation is wrong — degree-35
        wildness (below) is texture, not license for low-degree bugs."""
        import numpy as np

        rows, nc = 64, 6
        cf = _random_rows(23, rows, nc)
        coeffs = "/tmp/brush_newton_easy.bin"
        _write_rows(coeffs, cf)
        meta = _run_cm_binary("solve_newton", coeffs, nc, "/tmp/brush_newton_easy_out.bin")
        self.assertEqual(meta["mode"], "solve_newton")
        got = _roots_from("/tmp/brush_newton_easy_out.bin", rows, nc - 1)
        worst = 0.0
        for r in range(rows):
            ref = np.roots(cf[r].astype(np.complex128))
            worst = max(worst, _multiset_rel_err(got[r], ref))
        self.assertLess(worst, 1e-6, f"Newton broke on easy inputs: {worst}")

    def test_newton_texture_is_finite_and_deterministic(self):
        """At degree 35 the compounding deflation error IS the brush:
        many rows deviate from np.roots (34/64 beyond 1e-3 at authoring),
        but every output stays finite (non-finite guards) and two runs
        are byte-identical — a brush must repaint the same stroke."""
        import numpy as np

        rows, nc = 64, 36
        cf = _random_rows(11, rows, nc)
        coeffs = "/tmp/brush_newton_texture.bin"
        _write_rows(coeffs, cf)
        _run_cm_binary("solve_newton", coeffs, nc, "/tmp/brush_newton_a.bin")
        _run_cm_binary("solve_newton", coeffs, nc, "/tmp/brush_newton_b.bin")
        with open("/tmp/brush_newton_a.bin", "rb") as fh:
            run_a = fh.read()
        with open("/tmp/brush_newton_b.bin", "rb") as fh:
            run_b = fh.read()
        self.assertEqual(run_a, run_b)

        got = _roots_from("/tmp/brush_newton_a.bin", rows, nc - 1)
        self.assertTrue(np.all(np.isfinite(got.view(np.float32))))
        deviating = 0
        for r in range(rows):
            ref = np.roots(cf[r].astype(np.complex128))
            if _multiset_rel_err(got[r], ref) > 1e-3:
                deviating += 1
        self.assertGreater(deviating, 5,
                           "Newton texture vanished — did the brush get 'fixed'?")


@unittest.skipUnless(HAVE_BINARIES, "local solver binaries unavailable")
class TestBrushThreading(unittest.TestCase):
    def test_jt_and_newton_thread_byte_identity(self):
        """JT/Newton share sweep_cm's static row partition: output must be
        byte-identical at any thread count, incl. the guard rows."""
        import numpy as np

        rows, nc = 400, 36
        cf = _random_rows(31, rows, nc)
        cf[7] = 0
        cf[13, 0] = np.inf
        cf[21, :20] = 0
        coeffs = "/tmp/brush_threads_fixture.bin"
        _write_rows(coeffs, cf)
        for mode in ("solve_jt", "solve_newton"):
            with self.subTest(mode=mode):
                meta1 = _run_cm_binary(mode, coeffs, nc, f"/tmp/brush_{mode}_t1.bin", n_threads=1)
                meta4 = _run_cm_binary(mode, coeffs, nc, f"/tmp/brush_{mode}_t4.bin", n_threads=4)
                self.assertEqual(meta1["n_threads"], 1)
                self.assertEqual(meta4["n_threads"], 4)
                self.assertEqual(meta4["mode"], mode)
                with open(f"/tmp/brush_{mode}_t1.bin", "rb") as fh:
                    out1 = fh.read()
                with open(f"/tmp/brush_{mode}_t4.bin", "rb") as fh:
                    out4 = fh.read()
                self.assertEqual(len(out1), rows * (nc - 1) * 2 * 4)
                self.assertEqual(out1, out4)


@unittest.skipUnless(HAVE_BINARIES, "local solver binaries unavailable")
class TestCappedAberth(unittest.TestCase):
    def _run_mt(self, coeffs, rows, nc, out_path, max_iter=None):
        spec = {"mode": "solve_mt", "coeffs_file": coeffs, "n_coeffs": nc,
                "n2": rows, "i1_start": 0, "i1_end": 1,
                "match_roots": False, "n_threads": 1}
        if max_iter is not None:
            spec["max_iter"] = max_iter
        proc = subprocess.run(
            [SWEEP_MT_LOCAL, out_path], input=json.dumps(spec),
            capture_output=True, text=True, timeout=300,
        )
        if proc.returncode != 0:
            raise AssertionError(proc.stderr[:400])
        return json.loads(proc.stdout)

    def test_max_iter_caps_convergence_and_echoes_in_meta(self):
        """max_iter is the capped-Aberth brush: absent/64 -> today's full
        solve byte-for-byte; a small cap must CHANGE the output (partially
        converged state) and stay deterministic; out-of-range values are
        ignored (default sticks)."""
        rows, nc = 400, 36
        cf = _random_rows(31, rows, nc)
        coeffs = "/tmp/brush_mt_fixture.bin"
        _write_rows(coeffs, cf)

        meta_default = self._run_mt(coeffs, rows, nc, "/tmp/brush_mt_default.bin")
        meta_64 = self._run_mt(coeffs, rows, nc, "/tmp/brush_mt_64.bin", max_iter=64)
        meta_3 = self._run_mt(coeffs, rows, nc, "/tmp/brush_mt_3.bin", max_iter=3)
        meta_3b = self._run_mt(coeffs, rows, nc, "/tmp/brush_mt_3b.bin", max_iter=3)
        meta_bad = self._run_mt(coeffs, rows, nc, "/tmp/brush_mt_bad.bin", max_iter=0)

        self.assertEqual(meta_default["max_iter"], 64)
        self.assertEqual(meta_64["max_iter"], 64)
        self.assertEqual(meta_3["max_iter"], 3)
        self.assertEqual(meta_bad["max_iter"], 64)

        with open("/tmp/brush_mt_default.bin", "rb") as fh:
            out_default = fh.read()
        with open("/tmp/brush_mt_64.bin", "rb") as fh:
            out_64 = fh.read()
        with open("/tmp/brush_mt_3.bin", "rb") as fh:
            out_3 = fh.read()
        with open("/tmp/brush_mt_3b.bin", "rb") as fh:
            out_3b = fh.read()
        with open("/tmp/brush_mt_bad.bin", "rb") as fh:
            out_bad = fh.read()

        self.assertEqual(out_default, out_64)
        self.assertEqual(out_default, out_bad)
        self.assertNotEqual(out_default, out_3)
        self.assertEqual(out_3, out_3b)
        self.assertLessEqual(meta_3["avg_iterations"], 3.0)


if __name__ == "__main__":
    unittest.main()
