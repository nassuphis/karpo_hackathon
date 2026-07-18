"""
giga_2893/2894/2895 parity: three poly_chess4 runs. chess4's parity
mask SELECTS which cells exist (odd cells only — no white-square
anchors), both copies displace that same cell set via shape paths
coupled through ONE draw, bimodal mix:

  giga_2893: all defaults — inner==outer EXACTLY (mix is a no-op),
             a single displaced half-lattice, one branch, one draw.
  giga_2894: square-path outer (opolygon4 via floor gates and
             i^e = exp(i pi/2 e)), 0.3-circle inner at phi=0.25,
             a=0.95 (exponent 19.999999999999982 — f64 truth, not
             20), then p2_p: (cf^2+cf+1)*ramp*1e-18 + cf (measured
             up to 1.4e-2 relative shatter).
  giga_2895: degree 72, near-frozen outer circle (ospeed=0.01) vs
             live 0.25 inner circle, a=0.85; coefficients reach
             ~9.3e45 — OVER f32 transport — so the program rescales
             by 1e-15 (roots invariant, the cf10p450 doctrine).

Measured at authoring (5 probes each): relcf 5.8e-14 / 6.5e-14 /
1.9e-12; root multisets max 1.9e-10 / 8.2e-10 / 5.8e-4 (the last is
deg-72 conditioning dust). No solver in-program; plain build.
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

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.9, 0.9))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2893_family_coeff_programs.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2893_family_coeff_programs", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _bimodal(u, E):
    lo = (1 - math.floor(2 * u)) * (math.exp(E * math.log(2 * u)) / 2 if u > 0 else 0.0)
    hi = math.floor(2 * u) * (1 - math.exp(E * math.log(2 - 2 * u)) / 2)
    return lo + hi


def _opoly4(t):
    import numpy as np

    ts = 4 * t
    e = math.floor(ts)
    f = ts - e
    return (1 - f) * np.exp(1j * (math.pi / 2) * e) + f * np.exp(1j * (math.pi / 2) * (e + 1))


def _oracle_coeffs(g, name, u, v):
    import numpy as np

    def frac(x):
        return x - math.floor(x)

    C8 = np.array(g.CELLS8)
    C12 = np.array(g.CELLS12)
    ts, us = g.SEEDS[name]
    t = frac(u * ts[0] + v * ts[1] + ts[2])
    if name == "giga_2893":
        return np.poly(C8 + 0.5 * np.exp(2j * np.pi * t) * (1 + 1j))
    ub = frac(u * us[0] + v * us[1] + us[2])
    if name == "giga_2894":
        b = _bimodal(ub, 19.999999999999982)
        ti = 0.3 * np.exp(2j * np.pi * (t + 0.25))
        to = 0.45 * _opoly4(t)
        cf = np.poly(C8 + to * (1 + 1j)) * b + np.poly(C8 + ti * (1 + 1j)) * (1 - b)
        return (cf * cf + cf + 1) * np.arange(1, 34) * 1e-18 + cf
    b = _bimodal(ub, 6.666666666666666)
    ti = 0.25 * np.exp(2j * np.pi * t)
    to = np.exp(2j * np.pi * 0.01 * t)
    cf = np.poly(C12 + to * (1 + 1j)) * b + np.poly(C12 + ti * (1 + 1j)) * (1 - b)
    return cf * 1e-15


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
        [SWEEP_TEST, "/tmp/giga2893fam_test_row.bin"],
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


EXPECTED_LEN = {"giga_2893": 33, "giga_2894": 33, "giga_2895": 73}


class TestGiga2893FamilyDocuments(unittest.TestCase):
    def test_generator_documents_are_fresh_and_compile(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_giga_2893_family_coeff_programs.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        self.assertEqual(len(generator.CELLS8), 32)
        self.assertEqual(len(generator.CELLS12), 72)
        for name in EXPECTED_LEN:
            with self.subTest(name=name):
                stored = json.load(open(os.path.join(ROOT, f"{name}.coeff-program.json")))
                self.assertEqual(stored["name"], name)
                self.assertEqual(stored["source_text"], generator.build_source_text(name))
                compiled = compile_coeff_program_source(stored["source_text"])
                errors = [d for d in (compiled.get("diagnostics") or []) if d.get("level") == "error"]
                self.assertFalse(errors)
                self.assertLessEqual(compiled["token_count"], 256)
                src = stored["source_text"]
                import re
                self.assertIsNone(re.search(r"(?<!expand_)roots(?:_cm|_jt|_ae)?\(", src))
                # the nested-pop mix one-liner miscomputes (2891 trap)
                self.assertNotIn("multiply(pop", src)
        # run-specific structure pins
        self.assertEqual(generator.build_source_text("giga_2893").count("expand_roots"), 1)
        self.assertIn("1e-18", generator.build_source_text("giga_2894"))
        self.assertIn("1e-15", generator.build_source_text("giga_2895"))


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestGiga2893FamilyVmParity(unittest.TestCase):
    def test_vm_rows_match_oracle_for_all_three(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for name, length in EXPECTED_LEN.items():
            compiled = compile_coeff_program_source(generator.build_source_text(name))
            for u, v in PROBES:
                with self.subTest(name=name, u=u, v=v):
                    got = np.array(_run_vm(compiled, u, v))
                    ref = _oracle_coeffs(generator, name, u, v)
                    self.assertEqual(len(got), length)
                    rel = float(np.abs(got - ref).max() / np.abs(ref).max())
                    self.assertLess(rel, 1e-10)
                    pool = list(np.roots(ref))
                    diffs = []
                    for z in np.roots(got):
                        j = int(np.argmin([abs(z - p) for p in pool]))
                        diffs.append(abs(z - pool.pop(j)))
                    diffs = np.array(diffs)
                    self.assertLess(float(np.median(diffs)), 1e-6)
                    self.assertLess(float(diffs.max()), 5e-3)
                    if name == "giga_2895":
                        # the rescale is doing the transport rescue:
                        # scaled rows fit f32, unscaled would overflow
                        self.assertLess(float(np.abs(got).max()), 3.4e38)
                        self.assertGreater(float(np.abs(got).max()) * 1e15, 3.4e38)


if __name__ == "__main__":
    unittest.main()
