"""Pin the blank/empty program semantics at the run boundary, per kind (G8).

The four kinds deliberately diverge today (param/coeff blank-as-absent is
documented ASL-JSONPath behavior; solve-score fails closed; root compiles an
empty no-op). This matrix turns that folklore into contract: each case below
is the CURRENT pinned behavior — changing any of them is a deliberate
decision that must update this file, not an accident.

Cases per kind: source key absent / source key blank / chain key absent /
chain key explicit [] / chain key JSON null.
"""

import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestParamCoeffRunBoundary(unittest.TestCase):
    def test_source_key_absent_or_blank_is_absent(self):
        from pipeline_programs import coeff_source_text_for_run, param_source_text_for_run

        for params in ({}, {"param_program_source_text": ""}, {"param_program_source_text": "   "}):
            self.assertIsNone(param_source_text_for_run(params, None), params)
        for params in ({}, {"coeff_program_source_text": ""}, {"coeff_program_source_text": "   "}):
            self.assertIsNone(coeff_source_text_for_run(params, None), params)

    def test_explicit_chain_semantics(self):
        from pipeline_programs import explicit_program_chain_for_run

        for key in ("param_program_chain", "coeff_program_chain"):
            self.assertEqual(explicit_program_chain_for_run({}, key, key), (None, False))
            self.assertEqual(explicit_program_chain_for_run({key: None}, key, key), (None, False))
            self.assertEqual(explicit_program_chain_for_run({key: []}, key, key), ([], True))
            with self.assertRaises(ValueError):
                explicit_program_chain_for_run({key: "nope"}, key, key)


class TestRootRunBoundary(unittest.TestCase):
    def test_absent_blank_null_all_compile_empty(self):
        from root_program_source import compile_root_program_chain

        for payload in ({}, {"root_transforms": None}, {"root_transforms": ""}, {"chain": None}):
            compiled = compile_root_program_chain(payload)
            self.assertEqual(compiled["chain"], [], payload)

    def test_explicit_empty_stops_the_cascade(self):
        from root_program_source import compile_root_program_chain

        compiled = compile_root_program_chain(
            {"root_transforms": [], "chain": [["rotate_roots", 0.5]]}
        )
        self.assertEqual(compiled["chain"], [], "explicit [] must not fall through to chain")


class TestSolveScoreRunBoundary(unittest.TestCase):
    def test_blank_source_with_no_fallback_fails_closed(self):
        from solve_score_pipeline_programs import solve_score_program_for_run

        with self.assertRaises(RuntimeError):
            solve_score_program_for_run({"solve_score_program_source_text": "   "})

    def test_absent_everything_uses_default_metric(self):
        from solve_score_pipeline_programs import solve_score_program_for_run

        program = solve_score_program_for_run({})
        self.assertTrue(program.get("program_spec"))


if __name__ == "__main__":
    unittest.main()
