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

PROBES = ((0.137, 0.823), (0.75, 0.31), (0.031, 0.977), (0.5, 0.5),
          (0.0, 1.0), (0.9137, 0.412))


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2888_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2888_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _snapshot_row(t1, t2):
    """poly_chess1 at 1230acc, verbatim, with its two t-independent
    uniforms remapped onto the sweep parameters in numpy's own
    low + (high-low)*r form: t = -2pi + 4pi*t1, u = -2pi + 4pi*t2."""
    import numpy as np

    N = 7
    w = 2 * np.pi
    t = -w + 2 * w * t1
    u = -w + 2 * w * t2
    x = np.sin(t) + np.tile(np.arange(1, N + 1), N)
    y = np.cos(t) + np.repeat(np.arange(1, N + 1), N)
    curve = np.array(x + 1j * y, dtype=complex) - ((N + 1) / 2) - 1j * ((N + 1) / 2)
    coeffs = np.poly(curve + 0.1 * np.cos(u))
    cf1 = coeffs
    cf2 = np.pad(curve, (0, 1), constant_values=10j)
    cf3 = (cf1 + cf2 * 0.0001)
    def p2(cf):
        return cf ** 2 + cf + np.full(len(cf), 1.0, dtype=complex)
    return np.array(cf3 + 0.00000000000000000000001 * p2(cf3), dtype=complex)


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
        [SWEEP_TEST, "/tmp/giga2888_test_row.bin"],
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


class TestGiga2888CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2888")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.grid_points()), 49)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 69)
        self.assertEqual(compiled["stack_max"], 7)
        self.assertEqual(compiled["vector_constant_count"], 2)
        self.assertEqual(
            [v["length"] for v in compiled["vector_constants"]], [49, 50]
        )
        for chip in compiled["source_chain"]:
            if isinstance(chip, list):
                for arg in chip[1:]:
                    self.assertLessEqual(len(str(arg)), 256)

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

    def test_grid_and_quadratic_structure(self):
        """The 49 points are poly_chess1's tile/repeat order (column fast),
        and the 1e-23 quadratic term is only alive on astronomically large
        slots: dead on the monic lead, dominant where |cf3| > ~3.2e11."""
        import numpy as np

        generator = _load_generator()
        pts = np.array(generator.grid_points(), dtype=np.complex128)
        N = 7
        x = np.tile(np.arange(1, N + 1), N)
        y = np.repeat(np.arange(1, N + 1), N)
        ref = (x + 1j * y) - ((N + 1) / 2) - 1j * ((N + 1) / 2)
        self.assertTrue(np.array_equal(pts, ref))
        q = _snapshot_row(0.31, 0.77)
        self.assertEqual(len(q), 50)
        # leading = 1 + 0.0001*curve[0] + O(1e-23): still ~1, degree 49
        self.assertLess(abs(q[0] - 1.0), 0.01)
        self.assertGreater(float(np.abs(q).max()), 1e11)

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs verbatim poly_chess1 (remapped uniforms). Measured
        worst 2.1e-9 over 45 rows (degree-49 translate dust); acceptance
        1e-7. No sorts, no solver, no tie rows."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in PROBES:
            actual = _run_vm(compiled, t1, t2)
            expected = _snapshot_row(t1, t2)
            self.assertEqual(len(actual), 50)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-7, (t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
