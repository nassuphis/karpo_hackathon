import importlib.util
import json
import os
import subprocess
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
SWEEP_TEST = os.path.join(LAMBDA_DIR, "sweep_test")
sys.path.insert(0, LAMBDA_DIR)

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.0, 0.999), (0.31, 0.77))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2877_variants.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2877_variants", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _base(generator, t1, t2):
    import numpy as np

    pts = np.array(generator.letter_roots(), dtype=np.complex128)
    def scale(t, s):
        return ((1 - s) / 2 + s * t) * 1
    rts = (pts + complex(scale(t1, 0.9), scale(t2, 0.9))) + complex(-0.5, -0.5)
    return rts, np.poly(rts).astype(complex)


def _oracle(generator, name, t1, t2):
    """Verbatim 8e28adb roots_p chain with the variant's documented pairing."""
    import numpy as np

    rts, cf = _base(generator, t1, t2)
    if name == "giga_2877_v2":
        paired = rts                                    # pool order
    else:
        key = 100000 * np.abs(rts) - np.floor(100000 * np.abs(rts))
        paired = rts[np.argsort(key)]                   # chaotic key
    return 100.0 * np.append(paired, 1) + cf


def _run_vm(compiled, t1, t2):
    import numpy as np

    payload = {
        "version": 1,
        "fingerprint": compiled["fingerprint"],
        "tokens": compiled["tokens"],
        "stack_max": compiled["stack_max"],
        "scalar_exprs": compiled["scalar_exprs"],
        "vector_constants": compiled["vector_constants"],
    }
    proc = subprocess.run(
        [SWEEP_TEST, "/tmp/giga2877_variants_row.bin"],
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
        capture_output=True, text=True, timeout=30,
    )
    if proc.returncode != 0:
        raise AssertionError(proc.stderr[:400])
    meta = json.loads(proc.stdout)
    return np.array(
        [complex(re, im) for re, im in meta["coeff"]["poly"]],
        dtype=np.complex128,
    )


class TestGiga2877Variants(unittest.TestCase):
    EXPECT = {
        "giga_2877_v2": {"tokens": 55},
        "giga_2877_v3": {"tokens": 59},
    }

    def test_generated_documents_are_fresh_portable_and_compile(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        self.assertEqual(len(generator.letter_roots()), 32)
        for name, expect in self.EXPECT.items():
            with self.subTest(name=name):
                expected = generator.build_payload(name)
                path = os.path.join(ROOT, f"{name}.coeff-program.json")
                with open(path, "r", encoding="utf-8") as fh:
                    stored = json.load(fh)
                self.assertEqual(stored, expected)
                self.assertEqual(
                    set(stored),
                    {"version", "program_kind", "name", "chain", "source_text"},
                )
                self.assertEqual(stored["program_kind"], "coeff_program")
                self.assertEqual(stored["name"], name)
                self.assertEqual(stored["chain"], [])
                compiled = compile_coeff_program_source(stored["source_text"])
                self.assertEqual(compiled["token_count"], expect["tokens"])
                self.assertEqual(compiled["stack_max"], 7)
                self.assertEqual(
                    [v["length"] for v in compiled["vector_constants"]], [32, 33]
                )

    def test_round_trips_chain_to_source(self):
        import warnings

        from coeff_program_source import (
            coeff_source_text_from_chain,
            compile_coeff_program_source,
        )

        generator = _load_generator()
        for name, build in generator.VARIANTS.items():
            with self.subTest(name=name):
                compiled = compile_coeff_program_source(build())
                with warnings.catch_warnings(record=True) as caught:
                    warnings.simplefilter("always")
                    regenerated = coeff_source_text_from_chain(
                        compiled["source_chain"])
                self.assertEqual([str(w.message) for w in caught], [])
                self.assertEqual(
                    compile_coeff_program_source(regenerated)["fingerprint"],
                    compiled["fingerprint"],
                )

    def test_probe_rows_have_unique_pairing_keys(self):
        """v3's chaotic key must not tie at the parity probes (frac keys
        within dust of each other would give the sort legitimate freedom)."""
        import numpy as np

        generator = _load_generator()
        for t1, t2 in PROBES:
            rts, _ = _base(generator, t1, t2)
            key = 100000 * np.abs(rts) - np.floor(100000 * np.abs(rts))
            gaps = np.diff(np.sort(key))
            self.assertGreater(float(gaps.min()), 1e-6, (t1, t2))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_documented_formulas(self):
        """Native VM vs each variant's documented pairing. Measured worst
        1.1e-9 over 34 rows each; acceptance 1e-7."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        for name, build in generator.VARIANTS.items():
            compiled = compile_coeff_program_source(build())
            for t1, t2 in PROBES:
                with self.subTest(name=name, t=(t1, t2)):
                    actual = _run_vm(compiled, t1, t2)
                    expected = _oracle(generator, name, t1, t2)
                    self.assertEqual(len(actual), 33)
                    rel = (np.abs(actual - expected)
                           / np.maximum(1.0, np.abs(expected)))
                    self.assertLess(
                        float(rel.max()), 1e-7, (name, t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
