"""
giga_2864 iteration variants: seven 1D-map coefficient generators
sharing the base program's scan shape (docs/giga-2864-variants.md).

Oracle discipline: each variant has a PURE-PYTHON reference iteration
(math module, NOT numpy — CPython's math calls the same platform libm
the native VM links, and chaotic maps amplify a 1-ulp sin() divergence
to O(1) by slot 71). Maps built from *, +, -, floor, abs are IEEE-exact
and must match the VM bitwise; sine/exp variants apply libm the same
way on both sides and get a tight tolerance.

Root-space sanity per the fable lessons: the leading slot (ascending
storage -> slot 70) must stay f32-alive on the probe grid, except
spiral's documented origin-corner degree collapse (|c|^71 underflow,
inherited from the base giga_2864 geometric).
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
    path = os.path.join(ROOT, "scripts", "gen_giga_2864_variants.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2864_variants", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _oracle(name, t1, t2):
    """Reference iterations in plain python floats."""
    r2 = t1 * t1 + t2 * t2
    out = []
    if name == "giga_2864_logistic":
        r = 3 + r2 / 2
        x = 0.5
        out.append(x)
        for _ in range(70):
            x = r * x * (1 - x)
            out.append(x)
    elif name == "giga_2864_henon":
        a = 1 + 0.2 * r2
        out = [0.1, 0.1]
        for _ in range(69):
            out.append(1 - a * out[-1] * out[-1] + 0.3 * out[-2])
    elif name == "giga_2864_sine":
        r = 0.7 + 0.15 * r2
        x = 0.5
        out.append(x)
        for _ in range(70):
            x = r * math.sin(3.141592653589793 * x)
            out.append(x)
    elif name == "giga_2864_spiral":
        c = complex(0 - r2, -(t1 * t2))
        z = c
        out.append(z)
        for _ in range(70):
            z = z * c
            out.append(z)
    elif name == "giga_2864_bbaker":
        beta = 1 + 0.49 * r2
        x = 0.5 + 0.37 * t1
        x = x - math.floor(x)
        out.append(x)
        for _ in range(70):
            x = beta * x
            x = x - math.floor(x)
            out.append(x)
    elif name == "giga_2864_tent":
        r = 1 + 0.49 * r2
        x = 0.37
        out.append(x)
        for _ in range(70):
            x = r * (0.5 - abs(x - 0.5)) * 2
            out.append(x)
    elif name == "giga_2864_phases":
        x = 0.5 + 0.37 * t1
        x = x - math.floor(x)
        angles = [x]
        for _ in range(70):
            x = (1.3 + 0.3 * r2) * x + 0.211327
            x = x - math.floor(x)
            angles.append(x)
        import cmath

        tau = 6.283185307179586
        out = [cmath.exp(complex(0, tau * a)) for a in angles]
    else:
        raise AssertionError(name)
    return [complex(v) for v in out]


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
        [SWEEP_TEST, "/tmp/giga2864_variants_row.bin"],
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
        capture_output=True,
        text=True,
        timeout=60,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:300])
    meta = json.loads(proc.stdout)
    return [complex(re, im) for re, im in meta["coeff"]["poly"]]


# maps built only from real *, +, -, floor, abs must match the VM bitwise.
# spiral is complex: the VM's complex-multiply rounding grouping differs
# from CPython's at the ulp level (measured 6e-16 rel, non-amplifying —
# the map is contractive), so it rides the tolerance path with sine
# (libm per step) and phases (libm once).
EXACT_VARIANTS = {"giga_2864_logistic", "giga_2864_henon",
                  "giga_2864_bbaker", "giga_2864_tent"}
LIBM_TOL = 1e-9


class TestGiga2864VariantDocuments(unittest.TestCase):
    def test_generator_documents_are_fresh(self):
        proc = subprocess.run(
            [sys.executable, os.path.join(ROOT, "scripts", "gen_giga_2864_variants.py"), "--check"],
            capture_output=True, text=True,
        )
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

    def test_documents_compile_and_match_generator(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for name, builder in generator.VARIANTS.items():
            with self.subTest(name=name):
                path = os.path.join(ROOT, f"{name}.coeff-program.json")
                stored = json.load(open(path))
                self.assertEqual(stored["name"], name)
                self.assertEqual(stored["program_kind"], "coeff_program")
                self.assertEqual(stored["source_text"], builder())
                compiled = compile_coeff_program_source(stored["source_text"])
                self.assertFalse(compiled.get("diagnostics"))


@unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
class TestGiga2864VariantParity(unittest.TestCase):
    def test_vm_matches_python_oracles(self):
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for name in generator.VARIANTS:
            compiled = compile_coeff_program_source(generator.VARIANTS[name]())
            for t1, t2 in PROBES:
                with self.subTest(name=name, t1=t1, t2=t2):
                    actual = np.array(_run_vm(compiled, t1, t2))
                    expected = np.array(_oracle(name, t1, t2))
                    self.assertEqual(len(actual), 71)
                    if name in EXACT_VARIANTS:
                        self.assertTrue(np.array_equal(actual, expected),
                                        float(np.abs(actual - expected).max()))
                    else:
                        rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
                        self.assertLess(float(rel.max()), LIBM_TOL)

    def test_leading_slot_alive_and_roots_bounded(self):
        """Slot 70 is the leading coefficient (ascending storage): it must
        survive the f32 transport on every probe — except spiral, whose
        origin-corner collapse is the base program's own behavior (tiny
        |c|^71); spiral is asserted away from that corner."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for name in generator.VARIANTS:
            compiled = compile_coeff_program_source(generator.VARIANTS[name]())
            for t1, t2 in PROBES:
                if name == "giga_2864_spiral" and (t1 * t1 + t2 * t2) < 0.5:
                    continue
                with self.subTest(name=name, t1=t1, t2=t2):
                    cf = np.array(_run_vm(compiled, t1, t2))
                    self.assertTrue(np.all(np.isfinite(cf.view(float))))
                    lead32 = complex(np.float32(cf[-1].real), np.float32(cf[-1].imag))
                    self.assertNotEqual(lead32, 0, "leading slot died at f32")
                    roots = np.roots(cf[::-1])
                    self.assertLess(float(np.abs(roots).max()), 50.0)

    def test_phases_variant_is_unimodular(self):
        """The phases variant's whole point: every coefficient sits ON the
        unit circle, so the root cloud hugs a ring with chaotic filigree."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(
            generator.VARIANTS["giga_2864_phases"]())
        for t1, t2 in PROBES:
            cf = np.array(_run_vm(compiled, t1, t2))
            self.assertLess(float(np.abs(np.abs(cf) - 1.0).max()), 1e-12)


if __name__ == "__main__":
    unittest.main()
