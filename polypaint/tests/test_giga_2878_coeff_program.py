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
# delta = 0 exactly: symmetric constellation with exact |cf| ties (and an
# exact origin root from the ro/io-centered cell) — sort order legitimately
# free in both pipelines; pinned structurally, not positionally.
TIE_ROW = (0.5, 0.5)


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2878_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2878_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _cf(generator, t1, t2):
    import numpy as np

    pts = np.array(generator.letter_roots(), dtype=np.complex128)
    def scale(t, s):
        return ((1 - s) / 2 + s * t) * 1
    rts = (pts + complex(scale(t1, 0.9), scale(t2, 0.9))) + complex(-0.5, -0.5)
    return np.poly(rts).astype(complex)


def _snapshot_row(generator, t1, t2):
    """zfrm.sort_abs_p at 8e28adb, verbatim: sort_abs(cf)*andy + cf."""
    import numpy as np

    cf = _cf(generator, t1, t2)
    return cf[np.argsort(np.abs(cf))] * 0.075 + cf


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
        [SWEEP_TEST, "/tmp/giga2878_test_row.bin"],
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


class TestGiga2878CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2878")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.letter_roots()), 32)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 32)
        self.assertEqual(compiled["stack_max"], 6)
        self.assertEqual(compiled["vector_constant_count"], 1)
        self.assertEqual(compiled["vector_constants"][0]["length"], 33)

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

    def test_leading_never_vanishes_and_probes_are_tie_free(self):
        """q[0] = 1 + 0.075*s0 with |s0| <= 1 (the minimum-|.| coefficient
        cannot exceed the monic 1), so the degree holds at 32 everywhere —
        including the 21/5000 seeded rows where s0 is not the monic 1
        (verbatim in both pipelines). Parity probes asserted tie-free."""
        import numpy as np

        generator = _load_generator()
        rng = np.random.default_rng(2878)
        nonmonic = 0
        for _ in range(2000):
            t1, t2 = rng.random(), rng.random()
            cf = _cf(generator, t1, t2)
            mags = np.abs(cf)
            s0 = cf[np.argsort(mags)[0]]
            self.assertLessEqual(abs(s0), 1.0 + 1e-12)
            self.assertGreater(abs(1.0 + 0.075 * s0), 0.9)
            if np.argsort(mags)[0] != 0:
                nonmonic += 1
        self.assertGreater(nonmonic, 0)
        for t1, t2 in PROBES:
            mags = np.abs(_cf(generator, t1, t2))
            self.assertEqual(len(np.unique(mags)), len(mags), (t1, t2))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs verbatim sort_abs_p. Measured worst 1.3e-9 over 44
        tie-free rows; acceptance 1e-7."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        for t1, t2 in PROBES:
            actual = _run_vm(compiled, t1, t2)
            expected = _snapshot_row(generator, t1, t2)
            self.assertEqual(len(actual), 33)
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-7, (t1, t2, float(rel.max())))

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_tie_row_is_a_valid_sorted_admixture(self):
        """At delta = 0 the positional oracle is ambiguous (exact |cf| ties),
        so pin the STRUCTURE: recovered = (q - cf)/0.075 against the VM's own
        cf (first program line + emit) must be an |.|-ascending permutation
        of cf's exact multiset."""
        import numpy as np

        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        compiled = compile_coeff_program_source(generator.build_source_text())
        actual = _run_vm(compiled, *TIE_ROW)
        first_line = generator.build_source_text().split("\n")[0]
        cf_probe = compile_coeff_program_source(first_line + "\nemit")
        cf = _run_vm(cf_probe, *TIE_ROW)
        recovered = (actual - cf) / 0.075
        # tolerances scale with |cf| (~1e6 mid-slots): recovery dust is
        # ulp(|cf|)/0.075. The symmetric row's cf contains EXACT duplicate
        # values, so compare as lexsorted multisets (nearest-neighbor
        # injectivity would legitimately collapse on the duplicates).
        mags = np.abs(recovered)
        self.assertTrue(
            np.all(np.diff(mags) >= -(1e-6 * np.maximum(1.0, mags[:-1]))))
        rec_sorted = recovered[np.lexsort((recovered.imag, recovered.real))]
        cf_sorted = cf[np.lexsort((cf.imag, cf.real))]
        scale = np.maximum(1.0, np.abs(cf_sorted))
        self.assertLess(float((np.abs(rec_sorted - cf_sorted) / scale).max()), 1e-6)


if __name__ == "__main__":
    unittest.main()
