"""
giga_2897 parity: poly_chess5 over the rloc13 letterform lattice
(37 S + 37 T cells), two copies displaced along (1+1i) by antipodal
circle phases, expanded and convex-mixed with the bimodal_skewed(0.75)
weight; the two unseeded reference draws are remapped to frac-cascade
uniforms of (t1, t2). No root trip inside the program (solve=solve:
the pipeline paints np.roots of the emitted mix), so the plain
no-LAPACK sweep_test build runs everything.

Measured at authoring (5 probes): coefficient relative parity <= 7e-16
(f64 end-to-end), leading slot exactly 1, per-row root multiset median
<= 4e-8 with an ill-conditioned halo tail <= 8.4e-4, cloud max |z| ~75.
Pins below carry ~10-100x headroom over those measurements.
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
    path = os.path.join(ROOT, "scripts", "gen_giga_2897_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2897_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _oracle_coeffs(g, u, v):
    """Pure mirror of the VM program: same frac cascade, same repeated-
    product bimodal weight grouping, np.poly for expand_roots."""
    import numpy as np

    def frac(x):
        return x - math.floor(x)

    S = np.array(g.S_POINTS)
    T = np.array(g.T_POINTS)
    u1 = frac(u * 5477.4409 + v * 49807.87 + 0.375)
    u2 = frac(u * 2833.9973 + v * 68111.53 + 0.625)
    ds = 0.33 * np.exp(2j * np.pi * u1) * (1 + 1j)
    dt = 0.33 * np.exp(2j * np.pi * (u1 + 0.5)) * (1 + 1j)
    w = 2 * u2
    vv = 2 - 2 * u2
    a = (1 - math.floor(2 * u2)) * w * w * w * w / 2 \
        + math.floor(2 * u2) * (1 - vv * vv * vv * vv / 2)
    return np.poly(S + ds) * (1 - a) + np.poly(T + dt) * a


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
        [SWEEP_TEST, "/tmp/giga2897_test_row.bin"],
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


class TestGiga2897Document(unittest.TestCase):
    def test_generator_document_is_fresh_and_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_giga_2897_coeff_program.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        stored = json.load(open(os.path.join(ROOT, "giga_2897.coeff-program.json")))
        self.assertEqual(stored["name"], "giga_2897")
        self.assertEqual(stored["source_text"], generator.build_source_text())
        compiled = compile_coeff_program_source(stored["source_text"])
        errors = [d for d in (compiled.get("diagnostics") or []) if d.get("level") == "error"]
        self.assertFalse(errors)
        self.assertLessEqual(compiled["token_count"], 256)
        # the simplicity claim: no root trip anywhere in the program —
        # exactly two expand_roots and no roots_* solver transforms
        src = stored["source_text"]
        self.assertEqual(src.count("expand_roots"), 2)
        for banned in ("roots_cm", "roots_jt", "roots_ae"):
            self.assertNotIn(banned, src)
        # the plain roots(k, pad) trip either — expand_roots is the only
        # roots-named token in the program
        import re
        self.assertIsNone(re.search(r"(?<!expand_)roots\(", src))
        # deployed-compiler dialect: statement transforms only, no
        # registers (r1 =) and no tos-in-expression-arg scans
        self.assertNotIn("r1", src)
        # the lattice: 37 + 37 points, degree 37 -> emitted length 38
        self.assertEqual(len(generator.S_POINTS), 37)
        self.assertEqual(len(generator.T_POINTS), 37)


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestGiga2897VmParity(unittest.TestCase):
    def test_vm_rows_match_oracle_and_carry_the_lattice(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for u, v in PROBES:
            with self.subTest(u=u, v=v):
                got = np.array(_run_vm(compiled, u, v))
                ref = _oracle_coeffs(generator, u, v)
                self.assertEqual(len(got), 38)
                # convex mix of two monics: leading slot is 1
                self.assertLess(abs(got[0] - 1.0), 1e-12)
                # f64 end-to-end coefficient parity (measured 7e-16)
                rel = float(np.abs(got - ref).max() / np.abs(ref).max())
                self.assertLess(rel, 1e-13)
                # root multiset: median at the dust floor, tail bounded
                # by the mix's ill-conditioned outer roots (2920 class)
                pool = list(np.roots(ref))
                diffs = []
                for z in np.roots(got):
                    j = int(np.argmin([abs(z - p) for p in pool]))
                    diffs.append(abs(z - pool.pop(j)))
                diffs = np.array(diffs)
                self.assertLess(float(np.median(diffs)), 1e-5)
                self.assertLess(float(diffs.max()), 0.05)
                self.assertLess(float(np.abs(np.roots(got)).max()), 150.0)


if __name__ == "__main__":
    unittest.main()
