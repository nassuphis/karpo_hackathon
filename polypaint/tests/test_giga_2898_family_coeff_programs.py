"""
giga_2898 family parity: five poly_chess5 siblings (2898, 2899, 2900,
2903, 2904) with phi=0 (both letterform copies displaced identically)
and a coefficient-space perturbation zfrm — the reference's *_p blend
f(cf)*andy + cf with f = sort_angles_keep_moduli / sort_abs /
symmetrize, all native registry transforms here (sort_angle_keep_mod /
sort_abs / rev+add). The bimodal_skewed(0.7) weight uses the
non-integer exponent 1/(1-0.7) spelled exp(E*log(.)). No root trip in
any program; the plain no-LAPACK sweep_test build runs everything.

Measured at authoring (5 probes per run): coefficient relative parity
<= 2.7e-15 (f64 end-to-end), root multiset median <= 7.1e-11, worst
tail 1.7e-8. Pins below carry orders-of-magnitude headroom.
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
EXPONENT = 3.333333333333333


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2898_family_coeff_programs.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2898_family_coeff_programs", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _bimodal(u2):
    """Pure mirror of the VM weight: floor gates + exp(E*log(.))."""
    lo = (1 - math.floor(2 * u2)) * (math.exp(EXPONENT * math.log(2 * u2)) / 2 if u2 > 0 else 0.0)
    hi = math.floor(2 * u2) * (1 - math.exp(EXPONENT * math.log(2 - 2 * u2)) / 2)
    return lo + hi


def _oracle_coeffs(g, spec, u, v):
    import numpy as np

    def frac(x):
        return x - math.floor(x)

    name, layout, rho, andy, kind, seeds = spec
    S = np.array(g.LAYOUTS[layout][0])
    T = np.array(g.LAYOUTS[layout][1])
    (m1, m2, c1), (m3, m4, c2) = seeds
    u1 = frac(u * m1 + v * m2 + c1)
    u2 = frac(u * m3 + v * m4 + c2)
    d = float(rho) * np.exp(2j * np.pi * u1) * (1 + 1j)
    a = _bimodal(u2)
    mix = np.poly(T + d) * a + np.poly(S + d) * (1 - a)
    if kind == "sortabs":
        f = mix[np.argsort(np.abs(mix), kind="stable")]
    elif kind == "angles":
        f = np.abs(mix) * np.exp(1j * np.sort(np.angle(mix)))
    else:
        f = mix + np.flip(mix)
    return f * float(andy) + mix


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
        [SWEEP_TEST, "/tmp/giga2898fam_test_row.bin"],
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


EXPECTED = {
    "giga_2898": ("rjail2", 21, "sort_angle_keep_mod"),
    "giga_2899": ("rjail2", 21, "sort_abs"),
    "giga_2900": ("rjail2", 21, "rev"),
    "giga_2903": ("rp3", 25, "sort_abs"),
    "giga_2904": ("rjail6", 8, "sort_abs"),
}


class TestGiga2898FamilyDocuments(unittest.TestCase):
    def test_generator_documents_are_fresh_and_compile(self):
        from coeff_program_source import compile_coeff_program_source

        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_giga_2898_family_coeff_programs.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

        generator = _load_generator()
        self.assertEqual([r[0] for r in generator.RUNS], list(EXPECTED))
        for spec in generator.RUNS:
            name, layout, _, _, _, _ = spec
            with self.subTest(name=name):
                stored = json.load(open(os.path.join(ROOT, f"{name}.coeff-program.json")))
                self.assertEqual(stored["name"], name)
                self.assertEqual(stored["source_text"], generator.build_source_text(name))
                compiled = compile_coeff_program_source(stored["source_text"])
                errors = [d for d in (compiled.get("diagnostics") or []) if d.get("level") == "error"]
                self.assertFalse(errors)
                self.assertLessEqual(compiled["token_count"], 256)
                exp_layout, degree, ztoken = EXPECTED[name]
                self.assertEqual(layout, exp_layout)
                self.assertEqual(len(generator.LAYOUTS[layout][0]), degree)
                self.assertEqual(len(generator.LAYOUTS[layout][1]), degree)
                src = stored["source_text"]
                self.assertIn(ztoken, src)
                # no solver trip in any of the five
                import re
                self.assertIsNone(re.search(r"(?<!expand_)roots(?:_cm|_jt|_ae)?\(", src))
                self.assertEqual(src.count("expand_roots"), 2)


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestGiga2898FamilyVmParity(unittest.TestCase):
    def test_vm_rows_match_oracle_for_all_five(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for spec in generator.RUNS:
            name, layout = spec[0], spec[1]
            compiled = compile_coeff_program_source(generator.build_source_text(name))
            degree = len(generator.LAYOUTS[layout][0])
            for u, v in PROBES:
                with self.subTest(name=name, u=u, v=v):
                    got = np.array(_run_vm(compiled, u, v))
                    ref = _oracle_coeffs(generator, spec, u, v)
                    self.assertEqual(len(got), degree + 1)
                    rel = float(np.abs(got - ref).max() / np.abs(ref).max())
                    self.assertLess(rel, 1e-12)
                    pool = list(np.roots(ref))
                    diffs = []
                    for z in np.roots(got):
                        j = int(np.argmin([abs(z - p) for p in pool]))
                        diffs.append(abs(z - pool.pop(j)))
                    diffs = np.array(diffs)
                    self.assertLess(float(np.median(diffs)), 1e-8)
                    self.assertLess(float(diffs.max()), 1e-5)


if __name__ == "__main__":
    unittest.main()
