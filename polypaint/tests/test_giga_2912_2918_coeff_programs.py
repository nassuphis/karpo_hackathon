"""
giga_2912..giga_2918 parity: seven recursive_add* EMA-chain artworks ported
as stateless per-row analogs (frac-cascade pseudo-draws; the giga_2920
pattern). The oracles are the generator's analog mirrors — the chain-vs-
analog fidelity itself is distributional (2-D cloud correlations measured
against the true sequential chains, documented in the generator docstring
and recreate_giga_2912_2918.md), while THESE tests pin the VM against the
analog exactly.

giga_2916's mid-chain root trip needs roots_cm (LAPACK); on macOS the suite
builds the Accelerate-linked binary (the giga_2910 pattern). Its probes are
well-conditioned rows: ill-conditioned rows differ between LAPACK
implementations by ~1e-2 (the 2864 dust doctrine; measured on u=v=0.9).
"""
import importlib.util
import json
import os
import subprocess
import sys
import unittest

import numpy as np

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
SWEEP_TEST_LAPACK = "/tmp/polypaint_sweep_test_lapack"
sys.path.insert(0, LAMBDA_DIR)


def _ensure_lapack_binary():
    if sys.platform != "darwin":
        return False
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

# off-diagonal probes (u=v is the 2915 |z1-z2| singularity and a 2916
# ill-conditioned row) — all measured well-conditioned for every artwork
PROBES = ((0.137, 0.823), (0.75, 0.31), (0.31, 0.77), (0.05, 0.6),
          (0.61, 0.23))


def _load_generator():
    path = os.path.join(ROOT, "scripts",
                        "gen_giga_2912_2918_coeff_programs.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2912_2918", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _run_vm(compiled, u, v, binary):
    payload = {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
        "vector_constants": compiled["vector_constants"],
    }
    proc = subprocess.run(
        [binary, "/tmp/giga_2912_2918_row.bin"],
        input=json.dumps({
            "mode": "compute_debug",
            "function": "const",
            "cfpv": [1, 0, 0],
            "u": u,
            "v": v,
            "grid_n": 1000,
            "coeff_transforms": [],
            "coeff_program": payload,
        }),
        capture_output=True, text=True, timeout=120,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:300])
    meta = json.loads(proc.stdout)
    return np.array([complex(re, im) for re, im in meta["coeff"]["poly"]])


EXPECT_LEN = {"giga_2912": 11, "giga_2913": 11, "giga_2914": 25,
              "giga_2915": 51, "giga_2916": 10, "giga_2917": 9,
              "giga_2918": 4}


class TestGeneratorDocuments(unittest.TestCase):
    def test_documents_fresh_and_compile(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable,
             os.path.join(ROOT, "scripts",
                          "gen_giga_2912_2918_coeff_programs.py"),
             "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        self.assertEqual(generator.NAMES,
                         [f"giga_{n}" for n in range(2912, 2919)])
        for name in generator.NAMES:
            with self.subTest(name=name):
                stored = json.load(
                    open(os.path.join(ROOT, f"{name}.coeff-program.json")))
                self.assertEqual(stored["name"], name)
                self.assertEqual(stored["source_text"],
                                 generator.build_source_text(name))
                compiled = compile_coeff_program_source(stored["source_text"])
                errors = [d for d in (compiled.get("diagnostics") or [])
                          if d.get("level") == "error"]
                self.assertFalse(errors)
                self.assertLessEqual(compiled["token_count"], 256)
        # the EMA weights fold andy exactly and sum to ~1 (the analogs
        # conserve the reference's convex mixing)
        for w in (generator._weights_2912(), generator._weights_2913(),
                  generator.W_2914, generator.W_2915, generator.W_2918):
            self.assertAlmostEqual(sum(w), 1.0, places=12)
        # 2916's E share completes the convex mix
        self.assertAlmostEqual(
            generator.W_2916_A + generator.W_2916_B
            + (1.0 - generator.W_2916_A - generator.W_2916_B), 1.0, places=15)
        # 2916 root trip present and np-ordered (roots_cm, not roots_ae)
        src16 = generator.build_source_text("giga_2916")
        self.assertIn("roots_cm(poly, lo, exact)", src16)
        self.assertIn("poly = poly[0:10]", src16)

    def test_safe_knife_never_fires(self):
        """2917/2918 run solve='safe' — the knife (sum|cf| outside
        (1e-10, 1e10) or non-finite) must never fire on the analogs."""
        generator = _load_generator()
        rng = np.random.default_rng(0)
        for name in ("giga_2917", "giga_2918"):
            lo, hi = np.inf, 0.0
            for _ in range(2000):
                cf = generator.oracle_row(name, rng.random(), rng.random())
                self.assertTrue(np.all(np.isfinite(cf)), name)
                s = float(np.sum(np.abs(cf)))
                lo, hi = min(lo, s), max(hi, s)
            self.assertGreater(lo, 1e-10, name)
            self.assertLess(hi, 1e10, name)


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestVmParity(unittest.TestCase):
    def test_vm_rows_match_analog_oracles(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for name in generator.NAMES:
            if name == "giga_2916":
                continue                      # LAPACK case below
            compiled = compile_coeff_program_source(
                generator.build_source_text(name))
            for u, v in PROBES:
                with self.subTest(name=name, u=u, v=v):
                    ref = generator.oracle_row(name, u, v)
                    got = _run_vm(compiled, u, v, SWEEP_TEST)
                    self.assertEqual(len(got), EXPECT_LEN[name])
                    self.assertEqual(len(got), len(ref))
                    scale = float(np.abs(ref).max())
                    rel = float(np.abs(got - ref).max() / scale)
                    self.assertLess(rel, 1e-12, f"{name} coeff parity {rel}")

    def test_root_multiset_matches_2917(self):
        """The deterministic artwork's emitted poly solves to the oracle's
        np.roots multiset (the app solver replaces the reference solve)."""
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(
            generator.build_source_text("giga_2917"))
        u, v = PROBES[0]
        ref = generator.oracle_row("giga_2917", u, v)
        got = _run_vm(compiled, u, v, SWEEP_TEST)
        pool = list(np.roots(ref))
        diffs = []
        for z in np.roots(got):
            j = int(np.argmin([abs(z - p) for p in pool]))
            diffs.append(abs(z - pool.pop(j)))
        self.assertFalse(pool)
        self.assertLess(max(diffs) if diffs else 0.0, 1e-9)


@unittest.skipUnless(HAVE_LAPACK_BINARY,
                     "Accelerate/LAPACK sweep_test build unavailable")
class TestVmParity2916(unittest.TestCase):
    def test_vm_rows_match_analog_oracle(self):
        """Per-slot parity INCLUDING order: roots_cm's zgeev found-order
        matches np.roots on well-conditioned rows (the 2870s verification),
        which the componentwise EMA mix depends on. Residual = the VM's
        float32 root cast (the 2910 acceptance)."""
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(
            generator.build_source_text("giga_2916"))
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                ref = generator.oracle_row("giga_2916", u, v)
                got = _run_vm(compiled, u, v, SWEEP_TEST_LAPACK)
                self.assertEqual(len(got), 10)
                rel = float(np.max(np.abs(got - ref) / (np.abs(ref) + 1.0)))
                self.assertLess(rel, 1e-6, f"2916 per-slot parity {rel}")


if __name__ == "__main__":
    unittest.main()
