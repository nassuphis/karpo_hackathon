"""
giga_2891 parity: poly_chess2 PLAIN — the memoryless chessboard mixer
(giga_2920 without the EMA chain and without any root trip). Two
parity-displaced 8x8 lattice copies (rotating phase vs real diagonal
shift), np.poly both, plain-uniform convex mix; three unseeded draws
remapped to frac cascades. No solver in-program: the plain no-LAPACK
build runs everything, and white squares are per-row EXACT anchors
(no EMA pairing drag, unlike 2920's distributional anchors).

Measured at authoring (5 probes): coefficient relative parity 5.1e-11
(the deg-64 expand_roots accumulation-dust floor), white anchors
<= 2.9e-4 (np.roots conditioning dust, 2920's measured class), root
multiset vs oracle median 1.4e-9 / max 3.0e-4, leading slot exactly 1
(convex mix of monics).

This program also pinned a NEW TRAP found during authoring: the mix
one-liner add(poly, multiply(pop, scan(...))) MISCOMPUTES — add parks
its left operand as a stack temp and the nested pop grabs the temp,
yielding cf1 + cf2*a*(1-a) instead of the convex mix (VM-verified with
literal vectors). The program weights each branch in statement
position and mixes with the proven add(poly, pop).
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


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2891_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2891_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _oracle_coeffs(g, u, v):
    import numpy as np

    def frac(x):
        return x - math.floor(x)

    B = np.array(g.BASE)
    P = np.array(g.PARITY)
    (m1, m2, c1), (m3, m4, c2), (m5, m6, c3) = g.SEEDS
    u1 = frac(u * m1 + v * m2 + c1)
    u2 = frac(u * m3 + v * m4 + c2)
    u3 = frac(u * m5 + v * m6 + c3)
    d1 = 0.5 * np.exp(2j * np.pi * u1) * (1 + 1j)
    d2 = 0.5 * (u2 - 0.5) * (1 + 1j)
    # reference orientation: cf2 (real copy) gets a, cf1 (phase) 1-a
    return np.poly(B + P * d2) * u3 + np.poly(B + P * d1) * (1 - u3)


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
        [SWEEP_TEST, "/tmp/giga2891_test_row.bin"],
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


class TestGiga2891Document(unittest.TestCase):
    def test_generator_document_is_fresh_and_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_giga_2891_coeff_program.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        stored = json.load(open(os.path.join(ROOT, "giga_2891.coeff-program.json")))
        self.assertEqual(stored["name"], "giga_2891")
        self.assertEqual(stored["source_text"], generator.build_source_text())
        compiled = compile_coeff_program_source(stored["source_text"])
        errors = [d for d in (compiled.get("diagnostics") or []) if d.get("level") == "error"]
        self.assertFalse(errors)
        self.assertLessEqual(compiled["token_count"], 256)
        src = stored["source_text"]
        # no solver trip; exactly two expansions; and the miscomputing
        # nested-pop mix one-liner must never come back
        import re
        self.assertIsNone(re.search(r"(?<!expand_)roots(?:_cm|_jt|_ae)?\(", src))
        self.assertEqual(src.count("expand_roots"), 2)
        self.assertNotIn("multiply(pop", src)


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestGiga2891VmParity(unittest.TestCase):
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
                # convex mix of monics: leading slot is 1
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
                self.assertLess(float(diffs.max()), 2e-3)
                # white squares: per-row exact anchors (measured 2.9e-4
                # np.roots conditioning dust)
                d = np.abs(rg[None, :] - white[:, None]).min(axis=1)
                self.assertLess(float(d.max()), 2e-3)


if __name__ == "__main__":
    unittest.main()
