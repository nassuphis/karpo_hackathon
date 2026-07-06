import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestParamProgramChain(unittest.TestCase):
    def test_target_macro_lowers_to_canonical_tokens(self):
        from param_program_chain import (
            PARAM_OP_EMIT_P1,
            PARAM_OP_EMIT_P2,
            PARAM_OP_PUSH_T1,
            PARAM_OP_PUSH_T2,
            PARAM_OP_UNIT_CIRCLE,
            compile_param_program_chain,
        )

        compiled = compile_param_program_chain([["unit_circle", "both"]])
        self.assertEqual(
            [tok["op"] for tok in compiled["tokens"]],
            [
                PARAM_OP_PUSH_T1,
                PARAM_OP_UNIT_CIRCLE,
                PARAM_OP_EMIT_P1,
                PARAM_OP_PUSH_T2,
                PARAM_OP_UNIT_CIRCLE,
                PARAM_OP_EMIT_P2,
            ],
        )
        self.assertEqual(compiled["stack_max"], 1)
        self.assertEqual(compiled["emits"], ["p1", "p2"])
        self.assertEqual(len(compiled["lowered_to_source"]), len(compiled["tokens"]))
        self.assertEqual(compiled["execution_format"], "single_vm")
        self.assertFalse(compiled["uses_legacy_fast_path"])

    def test_register_legacy_transform_is_fast_path_eligible(self):
        from param_program_chain import (
            PARAM_OP_LEGACY,
            PARAM_SEL_BOTH,
            compile_param_program_chain,
        )

        compiled = compile_param_program_chain([["legacy", "rtheta", "both", "both", "1"]])
        self.assertEqual(compiled["tokens"][0]["op"], PARAM_OP_LEGACY)
        self.assertEqual(compiled["tokens"][0]["src"], PARAM_SEL_BOTH)
        self.assertEqual(compiled["tokens"][0]["tgt"], PARAM_SEL_BOTH)
        self.assertTrue(compiled["uses_legacy_fast_path"])
        self.assertEqual(compiled["legacy_transforms"], [["rtheta", "1"]])

    def test_redundant_legacy_target_arg_is_canonicalized(self):
        from param_program_chain import (
            PARAM_SEL_BOTH,
            PARAM_SEL_P1,
            compile_param_program_chain,
        )

        both = compile_param_program_chain([["legacy", "unit_circle", "both", "both", "2"]])
        self.assertEqual(both["source_chain"], [["legacy", "unit_circle", "both", "both"]])
        self.assertEqual(both["tokens"][0]["src"], PARAM_SEL_BOTH)
        self.assertEqual(both["tokens"][0]["tgt"], PARAM_SEL_BOTH)
        self.assertEqual(both["tokens"][0].get("args") or [], [])

        p1 = compile_param_program_chain([["legacy", "unit_circle", "both", "both", "0"]])
        self.assertEqual(p1["source_chain"], [["legacy", "unit_circle", "p1", "p1"]])
        self.assertEqual(p1["tokens"][0]["src"], PARAM_SEL_P1)
        self.assertEqual(p1["tokens"][0]["tgt"], PARAM_SEL_P1)
        self.assertEqual(p1["tokens"][0].get("args") or [], [])

    def test_old_transform_chain_target_args_are_migrated_to_selectors(self):
        from param_program_chain import PARAM_SEL_BOTH, PARAM_SEL_P1, compile_param_program_chain

        both = compile_param_program_chain([["crd", "2", "1"]])
        self.assertTrue(both["uses_legacy_fast_path"])
        self.assertEqual(both["source_chain"], [["legacy", "crd", "both", "both", "1"]])
        self.assertEqual(both["legacy_transforms"], [["crd", "2", "1"]])
        self.assertEqual(both["tokens"][0]["src"], PARAM_SEL_BOTH)
        self.assertEqual(both["tokens"][0]["tgt"], PARAM_SEL_BOTH)

        p1 = compile_param_program_chain([["crd", "0", "1"]])
        self.assertFalse(p1["uses_legacy_fast_path"])
        self.assertEqual(p1["source_chain"], [["legacy", "crd", "p1", "p1", "1"]])
        self.assertEqual(p1["tokens"][0]["src"], PARAM_SEL_P1)
        self.assertEqual(p1["tokens"][0]["tgt"], PARAM_SEL_P1)

    def test_old_transform_chain_compiles_as_register_legacy(self):
        from param_program_chain import compile_param_program_chain

        compiled = compile_param_program_chain([["rtheta", "1"], ["crd", "2", "1"]])
        self.assertTrue(compiled["uses_legacy_fast_path"])
        self.assertEqual(compiled["legacy_transforms"], [["rtheta", "1"], ["crd", "2", "1"]])

    def test_legacy_coeff_chips_are_available_in_param_program(self):
        from param_program_chain import PARAM_OP_LEGACY, compile_param_program_chain

        chain = [["coeff2"], ["coeff3"], ["legacy", "coeff12", "both", "both"]]
        compiled = compile_param_program_chain(chain)
        self.assertTrue(compiled["uses_legacy_fast_path"])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [PARAM_OP_LEGACY] * 3)
        self.assertEqual(compiled["legacy_transforms"], [["coeff2"], ["coeff3"], ["coeff12"]])

    def test_no_arg_legacy_chips_reject_useless_args(self):
        from param_program_chain import compile_param_program_chain

        compiled = compile_param_program_chain([["legacy", "roots2", "both", "both"]])
        self.assertEqual(compiled["legacy_transforms"], [["roots2"]])

        with self.assertRaisesRegex(RuntimeError, r"legacy\(roots2\) takes no arguments"):
            compile_param_program_chain([["legacy", "roots2", "both", "both", "1"]])

        with self.assertRaisesRegex(RuntimeError, r"legacy\(coeff2\) takes no arguments"):
            compile_param_program_chain([["coeff2", "1"]])

    def test_moebius_legacy_chip_accepts_complex_coefficients(self):
        from param_program_chain import PARAM_OP_LEGACY, compile_param_program_chain

        compiled = compile_param_program_chain([
            ["legacy", "moebius", "both", "both", "1-2j", "2+1j", "-2j+4", "10j-3"]
        ])
        self.assertEqual(compiled["tokens"][0]["op"], PARAM_OP_LEGACY)
        self.assertEqual(
            compiled["tokens"][0]["args"],
            [1.0, 2.0, 4.0, -3.0],
        )
        self.assertEqual(
            compiled["tokens"][0]["args_im"],
            [-2.0, 1.0, -2.0, 10.0],
        )
        self.assertEqual(compiled["legacy_transforms"][0][0], "moebius")
        self.assertEqual(len(compiled["legacy_transforms"][0]), 5)
        self.assertEqual(
            [
                complex(value.replace("i", "j"))
                for value in compiled["legacy_transforms"][0][1:]
            ],
            [1 - 2j, 2 + 1j, 4 - 2j, -3 + 10j],
        )

    def test_moebius_legacy_chip_rejects_bad_complex_coefficients(self):
        from param_program_chain import compile_param_program_chain

        with self.assertRaisesRegex(RuntimeError, r"legacy\(moebius\) expects 0, 4, or 8 arguments"):
            compile_param_program_chain([["legacy", "moebius", "both", "both", "1"]])

        with self.assertRaisesRegex(RuntimeError, r"unknown param expression identifier 'nan'"):
            compile_param_program_chain([
                ["legacy", "moebius", "both", "both", "nan+i", "0", "0", "1"]
            ])

    def test_const_accepts_dynamic_scalar_expression(self):
        from param_program_chain import (
            PARAM_OP_ADD,
            PARAM_OP_EMIT_P1,
            PARAM_OP_PUSH_P1,
            PARAM_OP_PUSH_P2,
            compile_param_program_chain,
        )

        compiled = compile_param_program_chain([["const", "p1+p2"], ["emit", "p1"]])
        self.assertEqual(
            [tok["op"] for tok in compiled["tokens"]],
            [PARAM_OP_PUSH_P1, PARAM_OP_PUSH_P2, PARAM_OP_ADD, PARAM_OP_EMIT_P1],
        )
        self.assertEqual(compiled["scalar_exprs"], [])
        self.assertNotIn("scalar_exprs", compiled["execution_spec"])
        self.assertFalse(compiled["uses_legacy_fast_path"])

    def test_scalar_expressions_accept_pi_constants(self):
        import math
        from param_program_chain import compile_param_program_chain

        compiled = compile_param_program_chain([["const", "pi + pi2i"], ["emit", "p1"]])
        token = compiled["tokens"][0]
        self.assertAlmostEqual(token["a"], math.pi)
        self.assertAlmostEqual(token["b"], 2.0 * math.pi)
        self.assertEqual(compiled["scalar_exprs"], [])

        legacy = compile_param_program_chain([["legacy", "rtheta", "both", "both", "pi2"]])
        self.assertAlmostEqual(legacy["tokens"][0]["args"][0], 2.0 * math.pi)
        self.assertTrue(legacy["uses_legacy_fast_path"])

    def test_old_const_two_field_form_is_canonicalized(self):
        from param_program_chain import compile_param_program_chain

        compiled = compile_param_program_chain([["const", "1", "2"], ["emit", "p1"]])
        self.assertEqual(compiled["source_chain"][0], ["const", "(1)+(2)*1j"])
        self.assertEqual(compiled["tokens"][0]["a"], 1.0)
        self.assertEqual(compiled["tokens"][0]["b"], 2.0)

    def test_abs_and_mod_compile_to_same_expression_opcode(self):
        from param_program_chain import compile_param_program_chain

        abs_compiled = compile_param_program_chain([["const", "abs(p1)"], ["emit", "p1"]])
        mod_compiled = compile_param_program_chain([["const", "mod(p1)"], ["emit", "p1"]])
        self.assertEqual(abs_compiled["tokens"], mod_compiled["tokens"])

    def test_legacy_arg_accepts_dynamic_real_expression(self):
        from param_program_chain import (
            PARAM_OP_ABS,
            PARAM_OP_CONST,
            PARAM_OP_DIVIDE,
            PARAM_OP_LEGACY,
            PARAM_OP_PUSH_P1,
            compile_param_program_chain,
        )

        compiled = compile_param_program_chain([["legacy", "rtheta", "both", "both", "abs(p1)/2"]])
        self.assertEqual(
            [tok["op"] for tok in compiled["tokens"]],
            [PARAM_OP_PUSH_P1, PARAM_OP_ABS, PARAM_OP_CONST, PARAM_OP_DIVIDE, PARAM_OP_LEGACY],
        )
        self.assertEqual(compiled["tokens"][-1]["stack_arg_count"], 1)
        self.assertFalse(compiled["uses_legacy_fast_path"])
        self.assertEqual(compiled["legacy_transforms"], [])

    def test_legacy_real_arg_signed_zero_has_one_fingerprint(self):
        from param_program_chain import compile_param_program_chain

        zero = compile_param_program_chain([["legacy", "rtheta", "both", "both", "0"]])
        negative_zero = compile_param_program_chain([["legacy", "rtheta", "both", "both", "0*-1.0"]])

        self.assertEqual(zero["tokens"], negative_zero["tokens"])
        self.assertEqual(zero["fingerprint"], negative_zero["fingerprint"])
        self.assertEqual(zero["tokens"][0]["args_im"], [0.0])

    def test_trailing_zero_defaults_are_real_arguments_not_empty_exprs(self):
        from param_program_chain import compile_param_program_chain

        hrt = compile_param_program_chain([["legacy", "hrt", "both", "both"]])
        rect = compile_param_program_chain([["legacy", "rect", "both", "both"]])

        self.assertEqual(hrt["tokens"][0]["args"], [1.0, 0.0])
        self.assertEqual(hrt["tokens"][0]["args_im"], [0.0, 0.0])
        self.assertEqual(rect["tokens"][0]["args"], [2.0, 1.0, 0.0])
        self.assertEqual(rect["tokens"][0]["args_im"], [0.0, 0.0, 0.0])

    def test_legacy_real_arg_rejects_complex_expression(self):
        from param_program_chain import compile_param_program_chain

        with self.assertRaisesRegex(RuntimeError, r"real-valued"):
            compile_param_program_chain([["legacy", "rtheta", "both", "both", "p1"]])

    def test_moebius_accepts_dynamic_complex_expressions(self):
        from param_program_chain import compile_param_program_chain

        compiled = compile_param_program_chain([
            ["legacy", "moebius", "both", "both", "1", "p1", "0", "exp(t2)"]
        ])
        self.assertEqual(compiled["tokens"][-1]["stack_arg_count"], 4)
        self.assertEqual(compiled["tokens"][-1]["n_args"], 4)
        self.assertEqual(compiled["scalar_exprs"], [])

    def test_macro_expression_expands_into_parent_expression_table(self):
        from param_program_chain import compile_param_program_chain

        macros = {"expr": [["const", "t1+t2"], ["emit", "p1"]]}

        def resolver(name):
            return macros[name]

        macro = compile_param_program_chain([["macro", "expr"]], macro_resolver=resolver)
        direct = compile_param_program_chain([["const", "t1+t2"], ["emit", "p1"]])
        self.assertEqual(macro["fingerprint"], direct["fingerprint"])
        self.assertEqual(macro["tokens"], direct["tokens"])
        self.assertEqual(macro["scalar_exprs"], [])

    def test_expressive_stack_program_has_stable_fingerprint(self):
        from param_program_chain import compile_param_program_chain

        chain = [
            ["push", "t1"],
            ["push", "t2"],
            ["add"],
            ["emit", "p1"],
            ["push", "t1"],
            ["push", "t2"],
            ["subtract"],
            ["emit", "p2"],
        ]
        a = compile_param_program_chain(chain)
        b = compile_param_program_chain(chain)
        self.assertEqual(a["fingerprint"], b["fingerprint"])
        self.assertEqual(a["token_count"], 8)
        self.assertEqual(a["stack_max"], 2)
        self.assertFalse(a["uses_legacy_fast_path"])

    def test_macro_expands_before_hashing(self):
        from param_program_chain import compile_param_program_chain

        macros = {"fold": [["unit_circle", "both"]]}

        def resolver(name):
            return macros[name]

        macro = compile_param_program_chain([["macro", "fold"]], macro_resolver=resolver)
        expanded = compile_param_program_chain([["unit_circle", "both"]])
        self.assertEqual(macro["fingerprint"], expanded["fingerprint"])
        self.assertEqual(macro["macro_expansions"], 1)
        self.assertEqual(macro["display"], "macro(fold)")

    def test_macro_cycle_rejected(self):
        from param_program_chain import compile_param_program_chain

        macros = {
            "a": [["macro", "b"]],
            "b": [["macro", "a"]],
        }

        def resolver(name):
            return macros[name]

        with self.assertRaisesRegex(RuntimeError, "macro cycle"):
            compile_param_program_chain([["macro", "a"]], macro_resolver=resolver)

    def test_stack_errors_are_specific_and_diagnostics_can_be_permissive(self):
        from param_program_chain import compile_param_program_chain, compile_param_program_diagnostics

        with self.assertRaisesRegex(RuntimeError, "emit\\(p1\\).*stack depth is 0"):
            compile_param_program_chain([["emit", "p1"]])

        diag = compile_param_program_diagnostics([["emit", "p1"]])
        self.assertEqual(diag["diagnostics"][0]["level"], "error")
        self.assertIn("stack depth is 0", diag["diagnostics"][0]["message"])

    def test_readonly_inputs_and_output_registers_are_not_silent_aliases(self):
        from param_program_chain import compile_param_program_chain

        with self.assertRaisesRegex(RuntimeError, "push source cannot be p1/p2"):
            compile_param_program_chain([["push", "p1"], ["emit", "p2"]])

        with self.assertRaisesRegex(RuntimeError, "emit target cannot be t1/t2"):
            compile_param_program_chain([["push", "t1"], ["emit", "t1"]])

        with self.assertRaisesRegex(RuntimeError, "legacy src cannot be t1"):
            compile_param_program_chain([["legacy", "unit_circle", "t1", "p2"]])

        with self.assertRaisesRegex(RuntimeError, "legacy tgt cannot be t2"):
            compile_param_program_chain([["legacy", "unit_circle", "p1", "t2"]])

    def test_legacy_registry_is_valid(self):
        from param_program_chain import validate_legacy_registry

        summary = validate_legacy_registry()
        self.assertGreater(summary["count"], 10)
        self.assertIn("rtheta", summary["names"])
        self.assertEqual(len(summary["fn_indices"]), len(set(summary["fn_indices"])))


if __name__ == "__main__":
    unittest.main()


class TestParamProgramRegisterWire(unittest.TestCase):
    def test_register_token_wire_forms(self):
        # r1 serializes with fn_index omitted ({"op": 29}) because the shared
        # token serializer drops zero fields; the C parser memsets tokens so
        # a missing fn_index reads as slot 0. r3 must carry fn_index=2.
        from param_program_chain import (
            PARAM_OP_PUSH_REG,
            PARAM_OP_STORE_REG,
            compile_param_program_chain,
        )

        compiled = compile_param_program_chain([
            ["const", "t1"], ["emit", "r1"],
            ["const", "r3"], ["emit", "r3"],
        ])
        tokens = compiled["tokens"]
        stores = [t for t in tokens if t["op"] == PARAM_OP_STORE_REG]
        pushes = [t for t in tokens if t["op"] == PARAM_OP_PUSH_REG]
        self.assertEqual(len(stores), 2)
        self.assertNotIn("fn_index", stores[0])
        self.assertEqual(stores[1].get("fn_index"), 2)
        self.assertEqual(len(pushes), 1)
        self.assertEqual(pushes[0].get("fn_index"), 2)

    def test_register_index_changes_fingerprint(self):
        from param_program_chain import compile_param_program_chain

        fp1 = compile_param_program_chain([["const", "t1"], ["emit", "r1"], ["const", "r1"], ["emit", "p1"]])["fingerprint"]
        fp2 = compile_param_program_chain([["const", "t1"], ["emit", "r2"], ["const", "r2"], ["emit", "p1"]])["fingerprint"]
        self.assertNotEqual(fp1, fp2)

    def test_emit_and_expression_register_errors_name_the_range(self):
        from param_program_chain import compile_param_program_chain

        with self.assertRaises(RuntimeError) as ctx:
            compile_param_program_chain([["const", "t1"], ["emit", "r9"]])
        self.assertIn("r1..r8", str(ctx.exception))
        with self.assertRaises(RuntimeError) as ctx:
            compile_param_program_chain([["const", "r9"], ["emit", "p1"]])
        self.assertIn("r1..r8", str(ctx.exception))

    def test_register_index_rejects_non_ascii_digits(self):
        from param_program_chain import _register_index

        self.assertIsNone(_register_index("r٣"))  # Arabic-Indic 3
        self.assertIsNone(_register_index("r²"))  # superscript 2
        self.assertEqual(_register_index("r3"), 2)
