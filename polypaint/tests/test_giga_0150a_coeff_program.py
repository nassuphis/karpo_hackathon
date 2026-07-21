"""
giga_0150a parity: the RAW-parameter band polynomial p7f (xfrm none,
zfrm none, solve safe, degree 22). The oracle is the generator's exact
program mirror; the generator also carries the VERBATIM numpy reference
(reference_row) so the analog-vs-reference delta — np.linspace endpoint
forcing plus association dust, ~5e-11 — is pinned here rather than
asserted from memory. Ten probes cover every t2-decile band, including
band 0 (the t1-scaled amplitude) and band 2 (the cos branch).
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

# one probe per band: (t1, t2) with t2 in each decile
PROBES = ((0.137, 0.05), (0.6, 0.15), (0.3, 0.25), (0.8, 0.31),
          (0.25, 0.47), (0.75, 0.55), (0.41, 0.68), (0.9, 0.71),
          (0.11, 0.86), (0.52, 0.97))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_0150a_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_0150a", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


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
        [SWEEP_TEST, "/tmp/giga_0150a_row.bin"],
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
             os.path.join(ROOT, "scripts", "gen_giga_0150a_coeff_program.py"),
             "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        stored = json.load(
            open(os.path.join(ROOT, "giga_0150a.coeff-program.json")))
        self.assertEqual(stored["name"], "giga_0150a")
        self.assertEqual(stored["source_text"], generator.build_source_text())
        compiled = compile_coeff_program_source(stored["source_text"])
        errors = [d for d in (compiled.get("diagnostics") or [])
                  if d.get("level") == "error"]
        self.assertFalse(errors)
        self.assertLessEqual(compiled["token_count"], 256)
        # the amp/mult tables are the verbatim p7f constants
        self.assertEqual(generator.AMPS,
                         [10.0, 100.0, 599.0, 443.0, 293.0, 541.0, 379.0,
                          233.0, 173.0, 257.0])
        self.assertEqual(generator.MULTS,
                         [11.0, 17.0, 83.0, 179.0, 127.0, 103.0, 283.0,
                          3.0, 5.0, 23.0])

    def test_analog_matches_verbatim_reference(self):
        """The stateless analog vs the VERBATIM p7f: only linspace endpoint
        forcing + association dust remain (documented ~5e-11)."""
        generator = _load_generator()
        worst = 0.0
        rng = np.random.default_rng(150)
        pts = list(PROBES) + [(rng.random(), rng.random()) for _ in range(200)]
        for u, v in pts:
            ora = generator.oracle_row(u, v)
            ref = generator.reference_row(u, v)
            worst = max(worst, float(np.max(np.abs(ora - ref))
                                     / np.max(np.abs(ref))))
        self.assertLess(worst, 1e-9, f"analog-vs-reference {worst}")

    def test_safe_knife_never_fires(self):
        """solve='safe': sum|cf| stays inside (1e-10, 1e10) — the slot-22
        addition keeps even tiny band-0 rows above ~211."""
        generator = _load_generator()
        rng = np.random.default_rng(0)
        lo, hi = np.inf, 0.0
        for _ in range(20000):
            cf = generator.reference_row(rng.random(), rng.random())
            self.assertTrue(np.all(np.isfinite(cf)))
            s = float(np.sum(np.abs(cf)))
            lo, hi = min(lo, s), max(hi, s)
        self.assertGreater(lo, 1e-10)
        self.assertLess(hi, 1e10)


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestVmParity(unittest.TestCase):
    def test_vm_rows_match_oracle_all_bands(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                ref = generator.oracle_row(u, v)
                got = _run_vm(compiled, u, v)
                self.assertEqual(len(got), 23)
                scale = float(np.abs(ref).max())
                rel = float(np.abs(got - ref).max() / scale)
                self.assertLess(rel, 1e-12, f"coeff parity {rel}")

    def test_root_multisets_match(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        u, v = PROBES[2]                     # the cos band
        ref = generator.oracle_row(u, v)
        got = _run_vm(compiled, u, v)
        pool = list(np.roots(ref))
        diffs = []
        for z in np.roots(got):
            j = int(np.argmin([abs(z - p) for p in pool]))
            diffs.append(abs(z - pool.pop(j)))
        self.assertFalse(pool)
        self.assertLess(max(diffs) if diffs else 0.0, 1e-8)


if __name__ == "__main__":
    unittest.main()
