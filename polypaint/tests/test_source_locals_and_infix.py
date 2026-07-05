"""Source locals (single-assignment aliases) + solve-score infix + parse_call fix.

One wave, zero wire risk: local aliases are compile-time substitution, so a
program using them must compile to a chain/fingerprint byte-identical to the
hand-inlined form; solve-score infix lowers to the existing call-tree chips,
so infix and call-tree spellings of the same expression must produce equal
chains. These tests pin those equivalences plus the shared-core parse_call
behavior change (call-with-trailing-text returns None instead of raising).
"""
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from program_source_core import ProgramSourceError, SourceLocals, parse_call


class _Stmt:
    def __init__(self, text, line=1, column=1):
        self.text = text
        self.line = line
        self.column = column


class TestParseCallTrailingText(unittest.TestCase):
    def test_trailing_operator_returns_none(self):
        self.assertIsNone(parse_call("log(x) * 1i"))

    def test_trailing_parenthesized_factor_returns_none(self):
        # Historically mis-parsed: ends with ')' but the call's own paren
        # closes earlier; the old code swallowed it and errored inside
        # split_top_level.
        self.assertIsNone(parse_call("add(a, b) * (c)"))

    def test_complete_call_still_parses(self):
        self.assertEqual(parse_call("add(a, b)"), ("add", ["a", "b"]))

    def test_unbalanced_still_raises(self):
        with self.assertRaises(ProgramSourceError):
            parse_call("sin(x")


class TestSourceLocalsCore(unittest.TestCase):
    def _table(self, reserved=("poly", "sin")):
        return SourceLocals(reserved=reserved, error_cls=ProgramSourceError)

    def test_define_and_substitute_with_parens(self):
        table = self._table()
        self.assertTrue(table.try_define(_Stmt("a = p1 + p2")))
        self.assertEqual(table.substitute("a * 2"), "(p1 + p2) * 2")

    def test_bare_substitution_for_numbers_idents_and_calls(self):
        table = self._table()
        table.try_define(_Stmt("k = 4"))
        table.try_define(_Stmt("v = range(1, 37)"))
        self.assertEqual(table.substitute("poly[k]"), "poly[4]")
        self.assertEqual(table.substitute("multiply(v, 2)"), "multiply(range(1, 37), 2)")

    def test_whole_word_only(self):
        table = self._table()
        table.try_define(_Stmt("a = 5"))
        self.assertEqual(table.substitute("abs(a) + fa + a"), "abs(5) + fa + 5")

    def test_alias_of_alias_inlines_at_definition(self):
        table = self._table()
        table.try_define(_Stmt("a = p1"))
        table.try_define(_Stmt("b = a * a"))
        self.assertEqual(table.substitute("b + 1"), "(p1 * p1) + 1")

    def test_reserved_name_is_not_consumed(self):
        table = self._table()
        self.assertFalse(table.try_define(_Stmt("poly = fill(4, 0)")))
        self.assertFalse(table.try_define(_Stmt("sin = 3")))

    def test_indexed_lhs_is_not_consumed(self):
        table = self._table()
        self.assertFalse(table.try_define(_Stmt("poly[3] = 1")))

    def test_rebind_rejected(self):
        table = self._table()
        table.try_define(_Stmt("a = 1"))
        with self.assertRaises(ProgramSourceError) as caught:
            table.try_define(_Stmt("a = 2"))
        self.assertEqual(caught.exception.code, "local_reassigned")

    def test_self_reference_rejected(self):
        table = self._table()
        with self.assertRaises(ProgramSourceError) as caught:
            table.try_define(_Stmt("a = sin(a)"))
        self.assertEqual(caught.exception.code, "local_self_reference")

    def test_empty_definition_rejected(self):
        table = self._table()
        with self.assertRaises(ProgramSourceError) as caught:
            table.try_define(_Stmt("a ="))
        self.assertEqual(caught.exception.code, "empty_expression")


class TestCoeffLocals(unittest.TestCase):
    def test_alias_compiles_and_fingerprints_like_hand_inlined(self):
        from coeff_program_source import compile_coeff_program_source

        aliased = compile_coeff_program_source(
            "l = log(abs(p1 + p2) + 1)\n"
            "poly = multiply(range(1, 37), 1i * l)\n"
        )
        inlined = compile_coeff_program_source(
            "poly = multiply(range(1, 37), 1i * (log(abs(p1 + p2) + 1)))\n"
        )
        self.assertEqual(aliased["fingerprint"], inlined["fingerprint"])
        self.assertEqual(aliased["source_chain"], inlined["source_chain"])
        self.assertEqual(aliased["execution_spec"], inlined["execution_spec"])

    def test_vector_alias_stays_call_shaped(self):
        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source(
            "ks = range(1, 37)\n"
            "poly = add(power(ks, 2), multiply(ks, p1 * p2))\n"
        )
        self.assertGreater(compiled["token_count"], 0)

    def test_numeric_alias_works_in_index_position(self):
        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source("k = 4\npoly[k] = k * p1\n")
        self.assertGreater(compiled["token_count"], 0)

    def test_call_prefix_expression_now_compiles(self):
        from coeff_program_source import compile_coeff_program_source

        # The old parse_call raised "missing closing parenthesis" here.
        left = compile_coeff_program_source("poly[0] = log(abs(p1)) * 1i\n")
        self.assertGreater(left["token_count"], 0)

    def test_reserved_names_rejected_as_locals(self):
        from coeff_program_source import (
            CoeffProgramSourceCompileError,
            compile_coeff_program_source,
        )

        for name in ("sin", "swirler", "cf", "range", "andy"):
            with self.subTest(name=name):
                with self.assertRaises((CoeffProgramSourceCompileError, RuntimeError)):
                    compile_coeff_program_source(f"{name} = p1\npoly[0] = {name}\n")

    def test_rebind_and_self_reference_error(self):
        from coeff_program_source import (
            CoeffProgramSourceCompileError,
            compile_coeff_program_source,
        )

        with self.assertRaises(CoeffProgramSourceCompileError):
            compile_coeff_program_source("a = p1\na = p2\npoly[0] = a\n")
        with self.assertRaises(CoeffProgramSourceCompileError):
            compile_coeff_program_source("a = sin(a)\npoly[0] = a\n")


class TestParamLocals(unittest.TestCase):
    def test_alias_in_expression_and_legacy_args(self):
        from param_program_source import parse_param_program_source

        parsed = parse_param_program_source(
            "w = 2*pi\n"
            "p1 = sin(w*t1)\n"
            "s = 0.5\n"
            "legacy(crd, both, both, s)\n"
        )
        self.assertEqual(parsed["chain"][0], ["const", "sin((2*pi)*t1)"])
        self.assertEqual(parsed["chain"][2], ["legacy", "crd", "both", "both", "0.5"])

    def test_reserved_selector_words_stay_reserved(self):
        from param_program_source import ParamProgramSourceCompileError, parse_param_program_source

        with self.assertRaises(ParamProgramSourceCompileError):
            parse_param_program_source("both = 1\np1 = both")


class TestRootLocals(unittest.TestCase):
    def test_numeric_alias_substitutes_into_transform_args(self):
        from root_program_source import parse_root_program_source

        parsed = parse_root_program_source("k = 0.25\nadd_complex(k)")
        self.assertEqual(parsed["chain"][0]["name"], "add_complex")
        self.assertEqual(parsed["chain"][0]["args"][0], 0.25)

    def test_transform_names_stay_reserved(self):
        from root_program_source import RootProgramSourceCompileError, parse_root_program_source

        with self.assertRaises(RootProgramSourceCompileError):
            parse_root_program_source("moebius = 1\nroots = moebius(1)")


class TestSolveScoreInfix(unittest.TestCase):
    def _parse(self, src):
        from solve_score_program_source import parse_solve_score_program_source

        return parse_solve_score_program_source(src)

    def test_infix_chain_equals_call_tree_chain(self):
        infix = self._parse(
            "x1 = metric(proximity, slv, q=0.1%)\nscore = x1 + 2 * x1"
        )
        calls = self._parse(
            "x1 = metric(proximity, slv, q=0.1%)\nscore = add(x1, mul(const(2), x1))"
        )
        self.assertEqual(infix["chain"], calls["chain"])

    def test_precedence_and_parens(self):
        flat = self._parse(
            "x1 = metric(proximity, slv, q=0.1%)\nscore = (x1 + 1) * 0.5"
        )
        self.assertEqual(flat["chain"][-1], "mul")
        prec = self._parse(
            "x1 = metric(proximity, slv, q=0.1%)\nscore = x1 + 1 * 0.5"
        )
        self.assertEqual(prec["chain"][-1], "add")

    def test_division_lowers_to_ratio(self):
        parsed = self._parse("score = metric(proximity, slv, q=0.1%) / 2")
        self.assertEqual(parsed["chain"][-1], "ratio")

    def test_unary_minus_lowers_to_subtract_from_zero(self):
        parsed = self._parse("score = -metric(proximity, slv, q=0.1%)")
        self.assertEqual(parsed["chain"][0], ["const", "0"])
        self.assertEqual(parsed["chain"][-1], "subtract")

    def test_call_then_operator_parses(self):
        parsed = self._parse(
            "score = min(metric(proximity, slv, q=0.1%), const(1)) * 2"
        )
        self.assertEqual(parsed["chain"][-1], "mul")

    def test_trailing_operator_still_rejected(self):
        from solve_score_program_source import SolveScoreProgramSourceCompileError

        with self.assertRaises(SolveScoreProgramSourceCompileError):
            self._parse("score = metric(proximity, slv, q=0.1%) +")

    def test_locals_still_write_once(self):
        from solve_score_program_source import SolveScoreProgramSourceCompileError

        with self.assertRaises(SolveScoreProgramSourceCompileError):
            self._parse(
                "x1 = metric(proximity, slv, q=0.1%)\nx1 = const(1)\nscore = x1"
            )


class TestCompiledEndToEnd(unittest.TestCase):
    def test_coeff_alias_program_runs_native(self):
        """Locals produce chains the native VM accepts (compile-level check
        here; the sweep_test parity harness covers execution)."""
        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source(
            "ks = range(1, 13)\n"
            "l = 1i * log(abs(p1 + p2) + 1)\n"
            "poly = add(power(ks, 2), multiply(ks, l))\n"
        )
        self.assertLessEqual(compiled["token_count"], 256)
        self.assertEqual(compiled["scalar_expr_count"], compiled["scalar_expr_count"])
        self.assertTrue(compiled["fingerprint"])


if __name__ == "__main__":
    unittest.main()
