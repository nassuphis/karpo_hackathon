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
    path = os.path.join(ROOT, "scripts", "gen_giga_263_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_263_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _snapshot_row(t1, t2):
    """The historical formula VERBATIM from giga_263_polyfun.txt (xfrm chain
    unit_circle -> coeff7, poly batman n=9, zfrm rev), kept independent of
    the Coeff Program code path so parity is a real comparison."""
    import numpy as np

    u1, u2 = np.exp(2j * np.pi * t1), np.exp(2j * np.pi * t2)
    p1 = (u1 + np.sin(u1)) / (u1 + np.cos(u1))
    p2 = (u2 + np.sin(u2)) / (u2 + np.cos(u2))
    cf = np.arange(1, 10)
    mag = np.log(np.abs(p1) + np.abs(p2) + cf) * cf * cf
    ang = np.angle(p1) * np.sin(cf) + np.angle(p2) * np.cos(cf)
    return np.flip(mag * (np.cos(ang) + 1j * np.sin(ang)))


class TestGiga263CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_263")
        self.assertEqual(stored["chain"], [])

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 122)
        self.assertEqual(compiled["stack_max"], 7)
        # the whole formula is per-row arithmetic: no pools of any kind
        self.assertEqual(compiled["vector_constant_count"], 0)
        self.assertEqual(compiled["scalar_expr_count"], 0)

    def test_round_trips_chain_to_source(self):
        import warnings

        from coeff_program_source import (
            coeff_source_text_from_chain,
            compile_coeff_program_source,
        )

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.SOURCE_TEXT)
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
        """Native VM output vs the snapshot's own math at fixed grid points —
        the acceptance bound is 1e-12 relative; measured headroom ~1.7e-16."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.SOURCE_TEXT)
        payload = {
            "version": 1,
            "fingerprint": compiled["fingerprint"],
            "tokens": compiled["tokens"],
            "stack_max": compiled["stack_max"],
            "scalar_exprs": compiled["scalar_exprs"],
        }
        for t1, t2 in ((0.137, 0.823), (0.5, 0.25), (0.901, 0.043)):
            proc = subprocess.run(
                [SWEEP_TEST, "/tmp/giga263_test_row.bin"],
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
            expected = _snapshot_row(t1, t2)
            self.assertEqual(len(actual), 9)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-12, (t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
