"""
roots_jt: the Jenkins-Traub mid-chain root trip (registry fn 30) — the
cheaper alternative to roots_cm's zgeev eigensolve (O(n^2) vs O(n^3),
no LAPACK requirement, so it works even in the plain sweep_test build).

Contract pinned here:
  * source/chain compile + registry drift (fn_index 30, roots_cm's
    pad/strip interface verbatim);
  * root MULTISET parity vs np.roots at the f32 solver floor — the
    emission ORDER is JT's found order, deliberately NOT np order
    (follow with sort_* when order matters), so no positional compare;
  * pad slot semantics identical to roots_cm (lo -> roots in low slots,
    zero pad at the top);
  * strip modes: rel zeroes tiny leading structure (protective), exact
    keeps the giant roots — mirroring roots_cm's threshold exactly;
  * availability without LAPACK.
"""
import json
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
sys.path.insert(0, LAMBDA_DIR)


def _run_program(src, t1, t2, binary=SWEEP_TEST):
    from coeff_program_source import compile_coeff_program_source

    compiled = compile_coeff_program_source(src)
    payload = {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
        "vector_constants": compiled["vector_constants"],
    }
    proc = subprocess.run(
        [binary, "/tmp/roots_jt_test_row.bin"],
        input=json.dumps({
            "mode": "compute_debug",
            "function": "const",
            "cfpv": [1, 0, 0],
            "u": t1,
            "v": t2,
            "grid_n": 1000,
            "coeff_transforms": [],
            "coeff_program": payload,
        }),
        capture_output=True, text=True, timeout=120,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:300])
    meta = json.loads(proc.stdout)
    return [complex(re, im) for re, im in meta["coeff"]["poly"]]


# a well-conditioned degree-11 test family (numpy layout, slot 0 leading)
FAMILY = """poly = fill(12, 0)
poly[0] = 1
poly[1] = 2*exp(6.283185307179586i*t1)
poly[3] = 0-1.5*exp(6.283185307179586i*t2)
poly[7] = 0.5*t1+0.25
poly[11] = 0-2
"""


class TestRootsJtTransform(unittest.TestCase):
    def test_registry_and_source_compile(self):
        from coeff_program_chain import legacy_registry
        from coeff_program_source import compile_coeff_program_source

        spec = legacy_registry()["by_name"]["roots_jt"]
        self.assertEqual(spec["fn_index"], 30)
        cm = legacy_registry()["by_name"]["roots_cm"]
        self.assertEqual([a["name"] for a in spec["args"]],
                         [a["name"] for a in cm["args"]])
        self.assertEqual(spec["allowed_src"], cm["allowed_src"])
        self.assertEqual(spec["allowed_tgt"], cm["allowed_tgt"])

        compiled = compile_coeff_program_source(
            FAMILY + "poly = roots_jt(poly, lo, exact)\nemit")
        self.assertTrue(compiled["tokens"])

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_root_multiset_matches_np_roots_without_lapack(self):
        """JT agrees with np.roots as a MULTISET at the f32 solver floor —
        in the plain (no-LAPACK) build, which cannot run roots_cm at all."""
        import numpy as np

        for t1, t2 in ((0.137, 0.823), (0.75, 0.31), (0.31, 0.77)):
            with self.subTest(t1=t1, t2=t2):
                got = np.array(_run_program(
                    FAMILY + "poly = roots_jt(poly, lo, exact)\nemit", t1, t2))
                self.assertEqual(len(got), 12)
                self.assertEqual(got[11], 0)   # lo pad at the top slot
                T1 = np.exp(2j * np.pi * t1)
                T2 = np.exp(2j * np.pi * t2)
                cf = np.zeros(12, dtype=complex)
                cf[0] = 1; cf[1] = 2 * T1; cf[3] = -1.5 * T2
                cf[7] = 0.5 * t1 + 0.25; cf[11] = -2
                ref = np.roots(cf)
                pool = list(ref)
                worst = 0.0
                for g in got[:11]:
                    j = int(np.argmin([abs(g - p) for p in pool]))
                    worst = max(worst, abs(g - pool.pop(j)))
                self.assertLess(worst, 1e-5)

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_strip_modes_mirror_roots_cm_semantics(self):
        """A tiny (1e-9-scale) leading coefficient: rel strips it (roots stay
        modest), exact keeps it (one giant root appears) — the same dichotomy
        roots_cm has, same relative threshold."""
        import numpy as np

        tiny = """poly = fill(6, 0)
poly[0] = 0.000000001
poly[1] = 1
poly[5] = 0-2
"""
        got_rel = np.array(_run_program(tiny + "poly = roots_jt(poly, lo, rel)\nemit", 0.3, 0.7))
        got_exact = np.array(_run_program(tiny + "poly = roots_jt(poly, lo, exact)\nemit", 0.3, 0.7))
        self.assertLess(float(np.abs(got_rel).max()), 10.0)
        self.assertGreater(float(np.abs(got_exact).max()), 1e6)


if __name__ == "__main__":
    unittest.main()
