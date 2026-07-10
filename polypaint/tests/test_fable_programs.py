"""Gate for the authored fable coeff-program corpus (code-review-26 F5).

fable_programs.py --dry-run is a manual check; this makes it a deployment
gate so a coefficient-language / VM / compiler change can't silently break a
fable and still pass predeploy. Compile-only checks run with no binary; the
native parity checks build sweep_test the same way the coeff catalog tests do.
"""
import os
import subprocess
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LAMBDA_DIR = ROOT / "lambda"
sys.path.insert(0, str(LAMBDA_DIR))

import fable_programs as fp
from coeff_program_source import compile_coeff_program_source


class FableCompileTests(unittest.TestCase):
    def test_all_sources_compile(self):
        self.assertEqual(len(fp.FABLES), 44)
        names = [n for n, _, _ in fp.FABLES]
        self.assertEqual(names, [f"fable-{i}" for i in range(1, 45)])
        for name, source, _ in fp.FABLES:
            compiled = compile_coeff_program_source(source)
            self.assertGreater(compiled["token_count"], 0, name)
            self.assertLessEqual(compiled["token_count"], 256, name)  # MAX_PROGRAM_TOKENS

    def test_names_are_unique_kebab(self):
        names = [n for n, _, _ in fp.FABLES]
        self.assertEqual(len(names), len(set(names)))


class FableNativeParityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        subprocess.run(
            ["cc", "-O2", "-pthread", "-o", str(LAMBDA_DIR / "sweep_test"),
             str(LAMBDA_DIR / "sweep_cli.c"), "-lm"],
            cwd=ROOT, check=True)

    def test_all_fables_match_their_numpy_reference(self):
        from port_poly100_programs import parity_check, PARITY_REL_TOL
        for name, source, ref in fp.FABLES:
            compiled = compile_coeff_program_source(source)
            stats, err = parity_check(compiled, ref, fp.N_PARITY)
            self.assertIsNone(err, f"{name}: {err}")
            self.assertLessEqual(stats["worst"], PARITY_REL_TOL, name)

    def test_fables_hold_across_their_declared_degrees(self):
        # CR28 F4: validate every fable at its min, 36, and max_coeffs, not just
        # one length. max is the parity-VERIFICATION ceiling — the largest degree
        # where parity_check still confirms the fable against numpy with margin
        # (fables render past it too, with inf tails the solver handles; the
        # ceilings were re-derived after F3's checked packer was reverted). The
        # sparse fables' declared minimum is the short-length indexing bound
        # where their poked slots are all valid + distinct.
        import functools
        from port_poly100_programs import parity_check, PARITY_REL_TOL
        for name, source, ref in fp.FABLES:
            lo, hi = fp.fable_limits(name)
            lengths = sorted({lo, min(max(36, lo), hi), hi})
            compiled = compile_coeff_program_source(source)
            for L in lengths:
                stats, err = parity_check(compiled, functools.partial(ref, n=L), L)
                self.assertIsNone(err, f"{name} n={L}: {err}")
                self.assertLessEqual(stats["worst"], PARITY_REL_TOL, f"{name} n={L}")


class FableCliTests(unittest.TestCase):
    def _run(self, *args):
        return subprocess.run(
            [sys.executable, str(LAMBDA_DIR / "fable_programs.py"), *args],
            cwd=ROOT, capture_output=True, text=True)

    def test_only_rejects_unknown_name(self):
        # F6: a typo'd --only must fail loudly, not exit 0 with zero checks
        proc = self._run("--only", "does_not_exist", "--dry-run")
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("unknown fable", proc.stdout + proc.stderr)

    def test_only_known_name_runs(self):
        proc = self._run("--only", "fable-7", "--dry-run")
        self.assertEqual(proc.returncode, 0, proc.stderr)
        self.assertIn("fable-7", proc.stdout)


if __name__ == "__main__":
    unittest.main()
