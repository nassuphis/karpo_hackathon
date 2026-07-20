"""
giga_87 parity: a python-era (png 2025-01-11) DENSE 51-coeff slot-fill,
xfrm unit_circle, zfrm rev, solve safe (never fires here). The oracle is
the poly.py / polys/giga.py `poly_giga_87` def verbatim (0-indexed — the
authoritative python-era version), evaluated at unit-circle points and
reversed. The dense k-formula compiles to a single scan(51, 0, F, F).
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
sys.path.insert(0, LAMBDA_DIR)

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.31, 0.77), (0.9, 0.9), (0.05, 0.6))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_87_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_87", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _uc(u, v):
    return np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)


def o_giga_87(t1, t2):
    """poly_giga_87 verbatim (poly.py / polys/giga.py), 0-indexed, then the
    zfrm rev (np.flip). Sequential slot overrides read as-written."""
    cf = np.zeros(51, dtype=complex)
    cf[0] = t1 + t2
    cf[1] = 1 + t1 * t2 + np.log(np.abs(t1 + t2) + 1)
    cf[2] = t1 + t2 + np.log(np.abs(1 - t1 * t2) + 1)
    for i in range(3, 51):
        cf[i] = i * t1 + (51 - i) * t2 + np.log(np.abs(t1 - t2 * i) + 1)
    cf[10] = cf[0] + cf[9] - np.sin(t1)
    cf[20] = cf[30] + cf[40] - np.cos(t2)
    cf[30] = cf[20] + cf[40] + np.sin(t1)
    cf[40] = cf[30] + cf[20] - np.cos(t2)
    cf[50] = cf[40] + cf[20] + np.sin(t2)
    return np.flip(cf)


def _run_vm(compiled, u, v):
    payload = {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
        "vector_constants": compiled["vector_constants"],
    }
    proc = subprocess.run(
        [SWEEP_TEST, "/tmp/giga_87_row.bin"],
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


class TestGeneratorDocuments(unittest.TestCase):
    def test_documents_fresh_and_compile(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable,
             os.path.join(ROOT, "scripts", "gen_giga_87_coeff_program.py"),
             "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        stored = json.load(open(os.path.join(ROOT, "giga_87.coeff-program.json")))
        self.assertEqual(stored["name"], "giga_87")
        self.assertEqual(stored["source_text"], generator.build_source_text())
        compiled = compile_coeff_program_source(stored["source_text"])
        errors = [d for d in (compiled.get("diagnostics") or [])
                  if d.get("level") == "error"]
        self.assertFalse(errors)
        self.assertLessEqual(compiled["token_count"], 256)
        # the dense k-formula is ONE scan; the zfrm is the reversal
        self.assertIn("scan(51, 0,", stored["source_text"])
        self.assertIn("poly = rev(poly)", stored["source_text"])
        # 'safe' never fires (finite, O(1e3) magnitudes) so no gate is ported
        self.assertNotIn("safe", stored["source_text"])


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestVmParity(unittest.TestCase):
    def test_vm_rows_match_oracle(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                t1, t2 = _uc(u, v)
                ref = o_giga_87(t1, t2)
                got = _run_vm(compiled, u, v)
                self.assertEqual(len(got), len(ref))
                scale = float(np.abs(ref).max())
                rel = float(np.abs(got - ref).max() / scale)
                self.assertLess(rel, 1e-12, f"coeff parity {rel}")

    def test_root_multisets_match(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        u, v = PROBES[2]
        t1, t2 = _uc(u, v)
        ref = o_giga_87(t1, t2)
        got = _run_vm(compiled, u, v)
        pool = list(np.roots(ref))
        diffs = []
        for z in np.roots(got):
            j = int(np.argmin([abs(z - p) for p in pool]))
            diffs.append(abs(z - pool.pop(j)))
        self.assertFalse(pool)
        self.assertLess(max(diffs) if diffs else 0.0, 1e-7)

    def test_safe_knife_never_fires(self):
        """The reference's 'safe' solver zeros a row when sum|cf| leaves
        (1e-10, 1e10) or any coeff is non-finite. Across 500 draws it never
        does — so omitting the gate is faithful."""
        generator = _load_generator()  # noqa: F841 (documents the source of truth)
        rng = np.random.default_rng(0)
        lo, hi = np.inf, 0.0
        for _ in range(500):
            u, v = rng.random(), rng.random()
            t1, t2 = _uc(u, v)
            cf = o_giga_87(t1, t2)
            self.assertTrue(np.all(np.isfinite(cf)))
            s = float(np.abs(cf).sum())
            lo, hi = min(lo, s), max(hi, s)
        self.assertGreater(lo, 1e-10)
        self.assertLess(hi, 1e10)


if __name__ == "__main__":
    unittest.main()
