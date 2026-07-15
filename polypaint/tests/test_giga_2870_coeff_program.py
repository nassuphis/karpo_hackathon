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


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2870_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2870_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _snapshot_row(generator, t1, t2):
    """The historical formula VERBATIM (letters.square + zfrm.sort_abs at
    500685f). No regime assertion here: the '3' glyph has a pixel at
    (-0.5, -0.5), so the shift (0.5, 0.5) puts one root EXACTLY at the
    origin — the constant term is 0 and sorts first. Both pipelines strip
    the zero leading term identically (that row solves degree 22); generic
    rows keep the monic 1 first (see test_generic_rows_stay_monic)."""
    import numpy as np

    roots = np.array(generator.letter_roots(), dtype=np.complex128)
    cf = np.poly(roots + complex(0.05 + 0.9 * t1, 0.05 + 0.9 * t2))
    return cf[np.argsort(np.abs(cf))]


class TestGiga2870CoeffProgram(unittest.TestCase):
    def test_generated_document_is_fresh_portable_and_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        expected = generator.build_payload()
        with open(generator.OUTPUT, "r", encoding="utf-8") as fh:
            stored = json.load(fh)
        self.assertEqual(stored, expected)
        self.assertEqual(
            set(stored),
            {"version", "program_kind", "name", "chain", "source_text"},
        )
        self.assertEqual(stored["program_kind"], "coeff_program")
        self.assertEqual(stored["name"], "giga_2870")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.letter_roots()), 23)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 18)
        self.assertEqual(compiled["stack_max"], 6)
        self.assertEqual(compiled["vector_constant_count"], 1)
        self.assertEqual(compiled["vector_constants"][0]["length"], 24)

    def test_round_trips_chain_to_source(self):
        import warnings

        from coeff_program_source import (
            coeff_source_text_from_chain,
            compile_coeff_program_source,
        )

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        with warnings.catch_warnings(record=True) as caught:
            warnings.simplefilter("always")
            regenerated = coeff_source_text_from_chain(compiled["source_chain"])
        self.assertEqual([str(w.message) for w in caught], [])
        self.assertEqual(
            compile_coeff_program_source(regenerated)["fingerprint"],
            compiled["fingerprint"],
        )

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs verbatim sort_abs at four grid points. Acceptance
        1e-9 relative; measured headroom ~1.2e-13 (translate-vs-np.poly
        expansion dust — sort_abs adds no arithmetic of its own)."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        payload = {
            "version": 1,
            "fingerprint": compiled["fingerprint"],
            "tokens": compiled["tokens"],
            "stack_max": compiled["stack_max"],
            "scalar_exprs": compiled["scalar_exprs"],
            "vector_constants": compiled["vector_constants"],
        }
        for t1, t2 in ((0.137, 0.823), (0.75, 0.31), (0.0, 0.999), (0.5, 0.5)):
            proc = subprocess.run(
                [SWEEP_TEST, "/tmp/giga2871_test_row.bin"],
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
            self.assertEqual(proc.returncode, 0, proc.stderr[:400])
            meta = json.loads(proc.stdout)
            actual = np.array(
                [complex(re, im) for re, im in meta["coeff"]["poly"]],
                dtype=np.complex128,
            )
            expected = _snapshot_row(generator, t1, t2)
            self.assertEqual(len(actual), 24)
            if (t1, t2) == (0.5, 0.5):
                # the known degenerate row: a root at the origin zeroes the
                # constant term, which sorts first — degree drops to 22 for
                # this single row, identically in both pipelines
                self.assertEqual(actual[0], 0)
            else:
                self.assertEqual(actual[0], 1)   # sorted leading = the monic 1
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-9, (t1, t2, float(rel.max())))


    def test_generic_rows_stay_monic(self):
        """The sort_abs regime for generic rows: the monic 1 is the
        magnitude minimum (200 random rows). The single measure-zero
        exception is the origin-root row pinned above."""
        import numpy as np

        generator = _load_generator()
        roots = np.array(generator.letter_roots(), dtype=np.complex128)
        rng = np.random.RandomState(2870)
        for _ in range(200):
            t1, t2 = rng.random(), rng.random()
            cf = np.poly(roots + complex(0.05 + 0.9 * t1, 0.05 + 0.9 * t2))
            self.assertEqual(cf[np.argsort(np.abs(cf))][0], 1)


if __name__ == "__main__":
    unittest.main()
