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
    path = os.path.join(ROOT, "scripts", "gen_giga_265_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_265_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _snapshot_row(t1, t2):
    """The recovered historical chain VERBATIM (xfrm.py at 483801d: bkr1,
    uc1, epow1; snapshot batman n=9; zfrm rev), independent of the Coeff
    Program code path so parity is a real comparison."""
    import numpy as np

    def bkr1(t):
        x, y = np.real(t), np.imag(t)
        x_fold, y_fold = x % 1, y % 1
        x_new = (2 * x_fold) % 1
        shift = np.floor(2 * x_fold)
        return x_new + 1j * (y_fold + shift) / 2

    p1 = np.exp(np.exp(1j * 2 * np.pi * bkr1(t1)))
    p2 = np.exp(np.exp(1j * 2 * np.pi * bkr1(t2)))
    cf = np.arange(1, 10)
    mag = np.log(np.abs(p1) + np.abs(p2) + cf) * cf * cf
    ang = np.angle(p1) * np.sin(cf) + np.angle(p2) * np.cos(cf)
    return np.flip(mag * (np.cos(ang) + 1j * np.sin(ang)))


class TestGiga265CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_265")
        self.assertEqual(stored["chain"], [])

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 158)
        self.assertEqual(compiled["stack_max"], 9)
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
        """Native VM vs the recovered chain at four grid points covering both
        baker branches and the exact branch edges t=0 and t=0.5 — acceptance
        bound 1e-12; measured headroom ~1.5e-16."""
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
        for t1, t2 in ((0.137, 0.823), (0.75, 0.31), (0.5, 0.999), (0.0, 0.499)):
            proc = subprocess.run(
                [SWEEP_TEST, "/tmp/giga265_test_row.bin"],
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
