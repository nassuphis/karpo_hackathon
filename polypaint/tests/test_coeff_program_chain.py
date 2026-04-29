import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestCoeffProgramChain(unittest.TestCase):
    def test_direct_legacy_chain_is_legacy_equivalent(self):
        from coeff_program_chain import COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["poly-rev"], ["poly-cumsum"], ["poly-sort_abs"], ["poly-exp", "0.5"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LEGACY] * 4)
        self.assertTrue(compiled["uses_legacy_chain_equivalent"])
        self.assertEqual(
            compiled["legacy_coeff_transforms"],
            [["rev"], ["cumsum"], ["sort_abs"], ["exp", "0.5"]],
        )

    def test_bare_legacy_sugar_remains_back_compat_alias(self):
        from coeff_program_chain import compile_coeff_program_chain

        old_form = compile_coeff_program_chain([["rev"]])
        new_form = compile_coeff_program_chain([["poly-rev"]])
        self.assertEqual(old_form["execution_spec"], new_form["execution_spec"])
        self.assertEqual(old_form["legacy_coeff_transforms"], [["rev"]])
        self.assertEqual(new_form["legacy_coeff_transforms"], [["rev"]])

    def test_scale100_is_linear_back_compat_alias(self):
        from coeff_program_chain import compile_coeff_program_chain

        old_form = compile_coeff_program_chain([["poly-scale100", "1+2j", "3-4j"]])
        new_form = compile_coeff_program_chain([["poly-linear", "1+2j", "3-4j"]])
        self.assertEqual(old_form["execution_spec"], new_form["execution_spec"])
        self.assertEqual(new_form["legacy_coeff_transforms"], [["linear", "1+2i", "3-4i"]])

    def test_pow_accepts_compact_complex_args_and_expression_andy(self):
        from coeff_program_chain import COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["poly-pow", "p1", "p2", "real(p1)"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LEGACY])
        token = compiled["tokens"][0]
        self.assertEqual(token["n_args"], 2)
        self.assertEqual(token["expr_refs"], [0, 1])
        self.assertEqual(token["andy_expr_ref"], 2)
        self.assertEqual(compiled["scalar_expr_count"], 3)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])

    def test_pow_old_four_real_args_remain_accepted(self):
        from coeff_program_chain import COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["poly-pow", "1", "2", "3", "4"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LEGACY])
        self.assertEqual(compiled["tokens"][0]["n_args"], 4)
        self.assertEqual(compiled["legacy_coeff_transforms"], [["pow", "1", "2", "3", "4"]])

    def test_legacy_direct_chip_allows_redundant_emit_commit(self):
        from coeff_program_chain import COEFF_OP_EMIT, COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["poly-rev"], ["emit"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LEGACY, COEFF_OP_EMIT])
        self.assertEqual(compiled["stack_max"], 0)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])

    def test_reported_preview_program_shape_compiles(self):
        from coeff_program_chain import compile_coeff_program_chain
        from param_program_chain import compile_param_program_chain

        param_chain = [
            ["legacy", "unit_circle", "both", "both"],
            ["legacy", "moebius", "both", "both", "13-22j", "-11-7j", "-3+7j", "-3-13j"],
            ["legacy", "roots6", "both", "both"],
            ["legacy", "asp", "both", "both", "0", "0.1"],
        ]
        coeff_chain = [["rev"], ["emit"]]
        param = compile_param_program_chain(param_chain)
        coeff = compile_coeff_program_chain(coeff_chain)
        self.assertEqual(param["token_count"], 4)
        self.assertEqual(coeff["token_count"], 2)

    def test_const_with_param_expression_uses_vm_path(self):
        from coeff_program_chain import COEFF_OP_CONST, COEFF_OP_EMIT, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["const", "35", "p1+p2"], ["emit"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_CONST, COEFF_OP_EMIT])
        self.assertEqual(compiled["stack_max"], 1)
        self.assertEqual(compiled["scalar_expr_count"], 1)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])
        self.assertEqual(compiled["legacy_coeff_transforms"], [])

    def test_poke_poly_and_poke_tos_compile_to_vm_tokens(self):
        from coeff_program_chain import (
            COEFF_OP_EMIT,
            COEFF_OP_POKE_POLY,
            COEFF_OP_POKE_TOS,
            COEFF_OP_PUSH,
            compile_coeff_program_chain,
        )

        compiled = compile_coeff_program_chain([
            ["poke_poly", "0", "100j*p1"],
            ["push", "poly"],
            ["poke_tos", "1", "p1+p2"],
            ["emit"],
        ])
        self.assertEqual(
            [tok["op"] for tok in compiled["tokens"]],
            [COEFF_OP_POKE_POLY, COEFF_OP_PUSH, COEFF_OP_POKE_TOS, COEFF_OP_EMIT],
        )
        self.assertEqual(compiled["stack_max"], 1)
        self.assertEqual(compiled["scalar_expr_count"], 2)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])

    def test_poke_tos_requires_stack_value(self):
        from coeff_program_chain import compile_coeff_program_chain

        with self.assertRaisesRegex(RuntimeError, "poke_tos"):
            compile_coeff_program_chain([["poke_tos", "0", "p1"]])

    def test_multiple_param_expressions_get_distinct_refs(self):
        from coeff_program_chain import COEFF_OP_CONST, COEFF_OP_EMIT, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([
            ["const", "3", "p1+p2"],
            ["emit"],
            ["const", "3", "p1-p2"],
            ["emit"],
        ])
        self.assertEqual(
            [tok["op"] for tok in compiled["tokens"]],
            [COEFF_OP_CONST, COEFF_OP_EMIT, COEFF_OP_CONST, COEFF_OP_EMIT],
        )
        self.assertEqual(compiled["scalar_expr_count"], 2)
        self.assertEqual(compiled["tokens"][0]["expr_refs"], [-1, 0])
        self.assertEqual(compiled["tokens"][2]["expr_refs"], [-1, 1])

    def test_legacy_pop_push_and_blend_stack_validation(self):
        from coeff_program_chain import COEFF_OP_BLEND, COEFF_OP_LEGACY, compile_coeff_program_chain

        chain = [
            ["push", "cf"],
            ["legacy", "deriv", "pop", "push"],
            ["push", "cf"],
            ["blend", "0.25"],
            ["emit"],
        ]
        compiled = compile_coeff_program_chain(chain)
        self.assertIn(COEFF_OP_LEGACY, [tok["op"] for tok in compiled["tokens"]])
        self.assertIn(COEFF_OP_BLEND, [tok["op"] for tok in compiled["tokens"]])
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])
        self.assertEqual(compiled["stack_max"], 2)

    def test_macro_expands_before_hashing(self):
        from coeff_program_chain import compile_coeff_program_chain

        macros = {"smooth": [["rev"], ["cumsum"]]}

        def resolver(name):
            return macros[name]

        macro = compile_coeff_program_chain([["macro", "smooth"]], macro_resolver=resolver)
        expanded = compile_coeff_program_chain([["rev"], ["cumsum"]])
        self.assertEqual(macro["fingerprint"], expanded["fingerprint"])
        self.assertEqual(macro["macro_expansions"], 1)
        self.assertEqual(macro["display"], "macro(smooth)")

    def test_invalid_final_stack_is_rejected(self):
        from coeff_program_chain import compile_coeff_program_chain

        with self.assertRaisesRegex(RuntimeError, "final stack depth"):
            compile_coeff_program_chain([["push", "cf"]])

    def test_legacy_args_are_real_int_or_enum_only(self):
        from coeff_program_chain import legacy_registry

        for spec in legacy_registry()["by_name"].values():
            for arg in spec["args"]:
                self.assertIn(arg["type"], {"real", "int", "enum"})

    def test_real_legacy_arg_rejects_complex_param_expression(self):
        from coeff_program_chain import compile_coeff_program_chain

        with self.assertRaisesRegex(RuntimeError, "must be real-valued"):
            compile_coeff_program_chain([["exp", "p1"]])

        compiled = compile_coeff_program_chain([["exp", "real(p1)"]])
        self.assertEqual(compiled["scalar_expr_count"], 1)


if __name__ == "__main__":
    unittest.main()
