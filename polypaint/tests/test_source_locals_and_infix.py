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

    def test_rebind_substitutes_previous_value(self):
        """Registers: rebinding inlines the PRIOR definition into the new
        one (r1 = r1 + r2 semantics), then replaces the binding."""
        table = self._table()
        table.try_define(_Stmt("a = 1"))
        table.try_define(_Stmt("b = 2"))
        self.assertTrue(table.try_define(_Stmt("a = a + b")))
        self.assertEqual(table.substitute("a"), "(1 + 2)")
        self.assertTrue(table.try_define(_Stmt("a = a * a")))
        self.assertEqual(table.substitute("a"), "((1 + 2) * (1 + 2))")

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

        # k (scan variable) and i/j (imaginary unit) are reserved alias names.
        compiled = compile_coeff_program_source("idx = 4\npoly[idx] = idx * p1\n")
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

    def test_rebind_allowed_and_first_use_self_reference_error(self):
        from coeff_program_source import (
            CoeffProgramSourceCompileError,
            compile_coeff_program_source,
        )

        # rebinding is register-like: the second definition wins and any
        # self-reference inlines the first
        compiled = compile_coeff_program_source("a = 1\na = a + 2\npoly[0] = a\n")
        inlined = compile_coeff_program_source("poly[0] = (1 + 2)\n")
        self.assertEqual(compiled["fingerprint"], inlined["fingerprint"])
        # a FIRST definition still cannot reference itself (undefined value)
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
        # ks is a vector REGISTER now: one store, two loads, and the range
        # lowers exactly once (CR35-F28 replaced a vacuous self-equality)
        ops = [t.get("op") for t in compiled["tokens"]]
        self.assertEqual(ops.count(50), 1, "one local_store for ks")
        self.assertEqual(ops.count(51), 2, "two local_load reads of ks")
        self.assertEqual(ops.count(18), 1, "range evaluates once")
        self.assertTrue(compiled["fingerprint"])


class TestConstantIndexFoldWithTosValue(unittest.TestCase):
    """poly[n-1] = ...tos... — locals inline as text, so the index
    arrives as "11-1"; the tos value forces the legacy poke chip, which
    only takes literal indices. The index must constant-fold (user-
    reported preview 400: "poke_poly index must be numeric, got '11-1'");
    genuinely runtime indices with tos values raise a clear error."""

    def test_constant_index_arithmetic_folds_for_tos_values(self):
        from coeff_program_source import parse_coeff_program_source

        src = ("n=11\npoly = fill(n, 0)\npoly\n"
               "poly[n-1] = 10i * tos[n-1]\n"
               "poly[floor(n/2)-1] = tos[0]\ndrop\nemit")
        chain = parse_coeff_program_source(src)["chain"]
        pokes = [row for row in chain if row and row[0] == "poke_poly"]
        self.assertEqual([row[1] for row in pokes], ["10", "4"])

    def test_runtime_index_with_tos_value_raises_clearly(self):
        from coeff_program_source import (
            CoeffProgramSourceCompileError,
            compile_coeff_program_source,
        )

        with self.assertRaises(CoeffProgramSourceCompileError) as ctx:
            compile_coeff_program_source(
                "poly = fill(4,0)\npoly\npoly[floor(abs(p1))] = tos[0]\ndrop\nemit")
        self.assertIn("constant index", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()


class TestScanSliceReduce(unittest.TestCase):
    """Compile-level pins for the scan/slice/reduce/window primitives.

    Native execution parity is exercised by lambda/port_poly100_programs.py
    (35 ports gated on sweep_test parity); these pin the compile contracts.
    """

    def _compile(self, src):
        from coeff_program_source import compile_coeff_program_source

        return compile_coeff_program_source(src)

    def test_scan_compiles_with_prev_and_k(self):
        c = self._compile(
            "poly = scan(36, 1, p1 + p2, (sin(k*prev) + 1) / abs(sin(k*prev) + 1))\n")
        self.assertEqual(c["token_count"], 2)  # scan + typed_set_poly
        self.assertEqual(c["scalar_expr_count"], 2)
        self.assertIn("scan:36:1:expr0:expr1", c["execution_spec"])

    def test_prev_and_k_rejected_outside_scan(self):
        from coeff_program_source import CoeffProgramSourceCompileError

        with self.assertRaises((CoeffProgramSourceCompileError, RuntimeError)):
            self._compile("poly[0] = prev + 1\n")
        with self.assertRaises((CoeffProgramSourceCompileError, RuntimeError)):
            self._compile("poly[0] = k\n")

    def test_slice_read_write_compile_and_validate(self):
        c = self._compile("poly[2:7] = multiply(poly[2:7], 5)\n")
        self.assertIn("slice:poly:2:7", c["execution_spec"])
        self.assertIn("poke_slice:2:7", c["execution_spec"])
        from coeff_program_source import CoeffProgramSourceCompileError

        with self.assertRaises((CoeffProgramSourceCompileError, RuntimeError)):
            self._compile("poly[7:2] = fill(5, 1)\n")  # reversed bounds
        with self.assertRaises((CoeffProgramSourceCompileError, RuntimeError)):
            self._compile("poly[0:3] = p1\n")  # scalar RHS

    def test_reduce_composes_with_tos_poke(self):
        c = self._compile("sum(poly[0:6])\npoly[0] = tos[0] + 1\ndrop\n")
        self.assertIn("reduce:sum", c["execution_spec"])
        # the tos-consuming poke rides the legacy poke chip (expression
        # plans see the pre-token stack frame)
        self.assertIn("poke_poly:0:expr", c["execution_spec"])

    def test_reduce_leftover_scalar_fails_stack_discipline(self):
        from coeff_program_source import CoeffProgramSourceCompileError

        # reduce pushes a scalar; programs must not strand it — but the
        # compile-time validator only enforces types, the VM enforces the
        # empty-stack exit. Compile succeeds; pin that shape here.
        c = self._compile("prod(cf[0:4])\npoly[0] = tos[0]\ndrop\n")
        self.assertIn("reduce:prod", c["execution_spec"])

    def test_window_and_step_are_pure_sugar(self):
        win = self._compile("poly = multiply(poly, window(3, 6))\n")
        # lowering builds the (x+|x|)/(2|x|) construction: ranges + divide
        self.assertIn("push_range", win["execution_spec"])
        self.assertNotIn("window", win["execution_spec"])
        step = self._compile("poly = multiply(poly, step(5))\n")
        self.assertNotIn("step", step["execution_spec"])

    def test_scan_ident_k_is_reserved_as_local(self):
        from coeff_program_source import CoeffProgramSourceCompileError

        with self.assertRaises((CoeffProgramSourceCompileError, RuntimeError)):
            self._compile("k = 4\npoly[0] = k\n")
