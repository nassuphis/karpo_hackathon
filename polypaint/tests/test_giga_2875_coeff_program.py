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

# Rows from the seeded 20k sweep where the strong jump (jmp = -0.1) breaks
# the sorted-leading == monic-1 regime (argsort(|cf|)[0] != 0). sort_abs is
# verbatim either way; these pin the non-monic regime end to end.
NONMONIC_PROBES = (
    (0.4490, 0.4988),
    (0.4398, 0.5109),
)


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2875_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2875_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _row_cf(generator, t1, t2):
    """xfrm.jump (jmp = -0.1, strict >) + letters.square + np.poly at
    500685f, verbatim."""
    import numpy as np

    levels = np.array([0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9])
    jmp = -0.1
    jt1 = t1 + np.sum(levels > t1) * jmp
    jt2 = t2 + np.sum(levels > t2) * jmp
    roots = np.array(generator.letter_roots(), dtype=np.complex128)
    def scale(t, s):
        return ((1 - s) / 2 + s * t) * 1
    return np.poly(roots + complex(scale(jt1, 0.9), scale(jt2, 0.9))).astype(complex)


def _snapshot_row(generator, t1, t2):
    """zfrm.sort_abs at 500685f, verbatim."""
    import numpy as np

    cf = _row_cf(generator, t1, t2)
    return cf[np.argsort(np.abs(cf))]


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
        [SWEEP_TEST, "/tmp/giga2875_test_row.bin"],
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


class TestGiga2875CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2875")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.letter_roots()), 32)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 53)
        self.assertEqual(compiled["stack_max"], 9)
        self.assertEqual(compiled["vector_constant_count"], 2)
        self.assertEqual(
            [v["length"] for v in compiled["vector_constants"]], [9, 33]
        )

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

    def test_sorted_leading_regime_and_its_violations(self):
        """jmp = -0.1 deltas reach -0.76: on most rows the sorted leading is
        the monic 1 (giga_2871's regime), but seeded rows violate it with a
        small nonzero leading — never exactly zero, so the degree holds at
        32 everywhere (the jump staircase's image has gaps; no reachable
        delta lands a root exactly on the origin)."""
        import numpy as np

        generator = _load_generator()
        rng = np.random.default_rng(2875)
        violations = 0
        for _ in range(5000):
            t1, t2 = rng.random(), rng.random()
            cf = _row_cf(generator, t1, t2)
            order = np.argsort(np.abs(cf))
            if order[0] != 0:
                violations += 1
                self.assertGreater(float(np.abs(cf)[order[0]]), 0.0)
        self.assertGreater(violations, 0)
        for t1, t2 in NONMONIC_PROBES:
            cf = _row_cf(generator, t1, t2)
            self.assertNotEqual(int(np.argsort(np.abs(cf))[0]), 0)

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs verbatim jump+sort_abs. Measured worst 3.4e-9 over
        67 rows (both regimes — sort_abs only permutes, so the non-monic
        rows carry no cancellation penalty); acceptance 1e-7. (0.3, 0.7)
        sits exactly ON level values and pins the strict-> counts (6, 2)."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        monic_probes = ((0.137, 0.823), (0.75, 0.31), (0.0, 0.999), (0.5, 0.5),
                        (0.3, 0.7))
        for t1, t2 in monic_probes + NONMONIC_PROBES:
            actual = _run_vm(compiled, t1, t2)
            expected = _snapshot_row(generator, t1, t2)
            self.assertEqual(len(actual), 33)
            if (t1, t2) in NONMONIC_PROBES:
                self.assertNotEqual(actual[0], 1)
            else:
                self.assertEqual(actual[0], 1)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-7, (t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
