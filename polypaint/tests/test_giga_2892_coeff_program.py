"""
giga_2892 parity: poly_chess3 — the COUPLED chessboard mixer. One
shared draw t0 drives BOTH displacements (phase 0.5*e^(2 pi i t0)(1+1i)
and real shift 0.5*(t0-0.5)(1+1i)), and the mix weight is
bimodal_skewed(0.85) (exponent 1/(1-0.85)=6.666666666666666 via the
exp/log floor-gate spelling). Two unseeded draws remapped to fresh
frac cascades; no solver in-program (plain no-LAPACK build).

Measured at authoring (5 probes): coefficient relative parity 5.9e-11
(deg-64 expand_roots dust floor), leading slot exactly 1, white
anchors <= 8.4e-4 per row, root multiset vs oracle median 5.1e-10 /
max 1.8e-3. The probes' mix weights all land near 0 or 1 — the 0.85
bimodal working as designed.
"""
import importlib.util
import json
import math
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
sys.path.insert(0, LAMBDA_DIR)

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.31, 0.77), (0.9, 0.9), (0.05, 0.6))
EXPONENT = 6.666666666666666


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2892_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2892_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _bimodal(u2):
    lo = (1 - math.floor(2 * u2)) * (math.exp(EXPONENT * math.log(2 * u2)) / 2 if u2 > 0 else 0.0)
    hi = math.floor(2 * u2) * (1 - math.exp(EXPONENT * math.log(2 - 2 * u2)) / 2)
    return lo + hi


def _oracle_coeffs(g, u, v):
    import numpy as np

    def frac(x):
        return x - math.floor(x)

    B = np.array(g.BASE)
    P = np.array(g.PARITY)
    (m1, m2, c1), (m3, m4, c2) = g.SEEDS
    t0 = frac(u * m1 + v * m2 + c1)
    a = _bimodal(frac(u * m3 + v * m4 + c2))
    d1 = 0.5 * np.exp(2j * np.pi * t0) * (1 + 1j)
    d2 = 0.5 * (t0 - 0.5) * (1 + 1j)
    return np.poly(B + P * d2) * a + np.poly(B + P * d1) * (1 - a)


def _run_vm(compiled, t1, t2):
    payload = {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
        "vector_constants": compiled["vector_constants"],
    }
    proc = subprocess.run(
        [SWEEP_TEST, "/tmp/giga2892_test_row.bin"],
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


class TestGiga2892Document(unittest.TestCase):
    def test_generator_document_is_fresh_and_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_giga_2892_coeff_program.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        stored = json.load(open(os.path.join(ROOT, "giga_2892.coeff-program.json")))
        self.assertEqual(stored["name"], "giga_2892")
        self.assertEqual(stored["source_text"], generator.build_source_text())
        compiled = compile_coeff_program_source(stored["source_text"])
        errors = [d for d in (compiled.get("diagnostics") or []) if d.get("level") == "error"]
        self.assertFalse(errors)
        self.assertLessEqual(compiled["token_count"], 256)
        src = stored["source_text"]
        import re
        self.assertIsNone(re.search(r"(?<!expand_)roots(?:_cm|_jt|_ae)?\(", src))
        self.assertEqual(src.count("expand_roots"), 2)
        # the nested-pop mix one-liner miscomputes (2891 trap log)
        self.assertNotIn("multiply(pop", src)
        # the coupled draw: BOTH displacement exprs inline the same t0
        # cascade (2 deltas x 2 scan positions x 2 frac halves = 8)
        self.assertEqual(src.count(f"t1*{generator.SEEDS[0][0]}"), 8)


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestGiga2892VmParity(unittest.TestCase):
    def test_vm_rows_match_oracle_with_exact_white_anchors(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        B = np.array(generator.BASE)
        P = np.array(generator.PARITY)
        white = B[P == 0]
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                got = np.array(_run_vm(compiled, u, v))
                ref = _oracle_coeffs(generator, u, v)
                self.assertEqual(len(got), 65)
                self.assertLess(abs(got[0] - 1.0), 1e-12)
                rel = float(np.abs(got - ref).max() / np.abs(ref).max())
                self.assertLess(rel, 1e-9)
                rg = np.roots(got)
                pool = list(np.roots(ref))
                diffs = []
                for z in rg:
                    j = int(np.argmin([abs(z - p) for p in pool]))
                    diffs.append(abs(z - pool.pop(j)))
                diffs = np.array(diffs)
                self.assertLess(float(np.median(diffs)), 1e-7)
                self.assertLess(float(diffs.max()), 1e-2)
                d = np.abs(rg[None, :] - white[:, None]).min(axis=1)
                self.assertLess(float(d.max()), 5e-3)


if __name__ == "__main__":
    unittest.main()
