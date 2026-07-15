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
    path = os.path.join(ROOT, "scripts", "gen_giga_2872_coeff_program.py")
    spec = importlib.util.spec_from_file_location("gen_giga_2872_coeff_program", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _snapshot_row(generator, t1, t2):
    """The historical formula VERBATIM (letters.square + zfrm.andy1 at
    500685f), independent of the Coeff Program code path. Asserts the
    argsort[0] == 0 regime that makes andy1's int64 cumprod collapse — the
    only faithfully reproducible behavior (recreate_giga_2872.md §1.5)."""
    import numpy as np

    roots = np.array(generator.letter_roots(), dtype=np.complex128)
    shift = complex(0.05 + 0.9 * t1, 0.05 + 0.9 * t2)
    cf = np.poly(roots + shift)
    order = np.argsort(np.abs(cf))
    assert order[0] == 0, "andy1 collapse regime violated: leading not minimal"
    csi = np.cumprod(order) % len(cf)
    cpi = np.cumsum(order) % len(cf)
    return cf[csi] - cf[cpi]


class TestGiga2872CoeffProgram(unittest.TestCase):
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
        self.assertEqual(stored["name"], "giga_2872")
        self.assertEqual(stored["chain"], [])
        self.assertEqual(len(generator.letter_roots()), 32)

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["token_count"], 40)
        self.assertEqual(compiled["stack_max"], 6)
        # one pool constant: the 33 expanded coefficients of the UNSHIFTED
        # checkerboard, monic leading 1
        self.assertEqual(compiled["vector_constant_count"], 1)
        constant = compiled["vector_constants"][0]
        self.assertEqual(constant["length"], 33)
        self.assertEqual(constant["values"][0], 1.0)
        self.assertEqual(constant["values"][1], 0.0)
        self.assertEqual(compiled["scalar_expr_count"], 3)

    def test_roots_ascii_literal_contract(self):
        """The generic glyph pattern behind the saved program: the checked-in
        table replicates the era's pngfont2pydict.py transcription (verified
        against all 255 letters.py b-entries at generation time), including
        its LSB-mirror and b<N> = sheet cell N-1 quirks."""
        from coeff_program_source import (
            coeff_source_text_from_chain,
            compile_coeff_program_source,
        )
        from cp437_font import FONT_ROWS, SHEET_SHA256

        # provenance + structure pins (the reference PNG is not available in
        # CI, so the sheet hash and table shape are pinned here instead of a
        # generator --check gate; hash independently verified against
        # /Users/nicknassuphis/pyroots/fonts/3dfx8x8.png at transcription)
        self.assertEqual(
            SHEET_SHA256,
            "9411cdef9736aae93a12029d0f164cb06203d1710e3603ae790794dd51eeede6",
        )
        self.assertEqual(len(FONT_ROWS), 256)
        self.assertTrue(all(len(rows) == 8 for rows in FONT_ROWS.values()))
        self.assertEqual(
            FONT_ROWS[178],
            (0b10101010, 0b01010101, 0b10101010, 0b01010101,
             0b10101010, 0b01010101, 0b10101010, 0b01010101),
        )
        blanks = sorted(code for code, rows in FONT_ROWS.items() if not any(rows))
        self.assertEqual(blanks, [1, 33, 256])

        # identity: roots_ascii_literal(178) deduplicates to the SAME pool
        # constant as the saved program's constellation
        generator = _load_generator()
        ascii_prog = compile_coeff_program_source(
            "poly = roots_ascii_literal(178)\nemit"
        )
        saved = compile_coeff_program_source(generator.build_source_text())
        self.assertEqual(
            ascii_prog["vector_constants"], saved["vector_constants"]
        )

        # validation matrix
        for bad, message in (
            ("poly = roots_ascii_literal(33)", "no lit pixels"),
            ("poly = roots_ascii_literal(0)", "must be in \\[1, 256\\]"),
            ("poly = roots_ascii_literal(257)", "must be in \\[1, 256\\]"),
            ("poly = roots_ascii_literal(178.5)", "must be an integer"),
            ("poly = roots_ascii_literal(t1)", "must be a static expression"),
            ("poly = roots_ascii_literal(178, 1)", "exactly \\(code\\)"),
        ):
            with self.assertRaisesRegex(Exception, message):
                compile_coeff_program_source(bad + "\nemit")

        # round trip keeps the code spelling
        import warnings

        compiled = compile_coeff_program_source(
            "poly = translate_roots(roots_ascii_literal(66), 0.5)\nemit"
        )
        with warnings.catch_warnings(record=True) as caught:
            warnings.simplefilter("always")
            regenerated = coeff_source_text_from_chain(compiled["source_chain"])
        self.assertEqual([str(w.message) for w in caught], [])
        self.assertIn("roots_ascii_literal(66.0)", regenerated)
        self.assertEqual(
            compile_coeff_program_source(regenerated)["fingerprint"],
            compiled["fingerprint"],
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

    @unittest.skipUnless(os.path.exists(SWEEP_TEST), "sweep_test binary not built")
    def test_vm_rows_match_snapshot_formula(self):
        """Native VM vs verbatim andy1 at four grid points. Acceptance 1e-9
        relative: measured headroom ~3.8e-12 — the residual is
        translate-vs-np.poly expansion-order dust magnified by degree-32
        binomials at the 1e14 coefficient scale."""
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
                [SWEEP_TEST, "/tmp/giga2872_test_row.bin"],
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
            self.assertEqual(len(actual), 33)
            self.assertEqual(actual[0], 0)   # andy1 leading element: exact zero
            rel = np.abs(actual - expected) / np.maximum(1.0, np.abs(expected))
            self.assertLess(float(rel.max()), 1e-9, (t1, t2, float(rel.max())))


if __name__ == "__main__":
    unittest.main()
