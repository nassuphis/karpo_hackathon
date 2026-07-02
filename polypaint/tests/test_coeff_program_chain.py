import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestCoeffProgramChain(unittest.TestCase):
    def test_direct_legacy_chain_is_legacy_equivalent(self):
        from coeff_program_chain import COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([
            ["legacy", "rev", "poly", "poly"],
            ["legacy", "cumsum", "poly", "poly"],
            ["legacy", "sort_abs", "poly", "poly"],
            ["legacy", "exp", "poly", "poly", "0.5"],
        ])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LEGACY] * 4)
        self.assertTrue(compiled["uses_legacy_chain_equivalent"])
        self.assertEqual(
            compiled["legacy_coeff_transforms"],
            [["rev"], ["cumsum"], ["sort_abs"], ["exp", "0.5"]],
        )

    def test_direct_transform_chips_compile_to_native_transform(self):
        from coeff_program_chain import COEFF_OP_NATIVE_TRANSFORM, compile_coeff_program_chain

        with self.assertRaisesRegex(RuntimeError, "rev chip requires target, source"):
            compile_coeff_program_chain([["rev"]])
        compiled = compile_coeff_program_chain([["rev", "poly", "poly"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_NATIVE_TRANSFORM])
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])
        with self.assertRaisesRegex(RuntimeError, "unknown coeff program chip: poly-rev"):
            compile_coeff_program_chain([["poly-rev"]])

    def test_scale100_is_linear_back_compat_alias(self):
        from coeff_program_chain import compile_coeff_program_chain

        old_form = compile_coeff_program_chain([["legacy", "scale100", "poly", "poly", "1+2j", "3-4j"]])
        new_form = compile_coeff_program_chain([["legacy", "linear", "poly", "poly", "1+2j", "3-4j"]])
        self.assertEqual(old_form["execution_spec"], new_form["execution_spec"])
        self.assertEqual(new_form["legacy_coeff_transforms"], [["linear", "1+2i", "3-4i"]])

    def test_pow_accepts_compact_complex_args_and_expression_andy(self):
        from coeff_program_chain import COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["legacy", "pow", "poly", "poly", "p1", "p2", "real(p1)"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LEGACY])
        token = compiled["tokens"][0]
        self.assertEqual(token["n_args"], 2)
        self.assertEqual(token["expr_refs"], [0, 1])
        self.assertEqual(token["andy_expr_ref"], 2)
        self.assertEqual(compiled["scalar_expr_count"], 3)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])

    def test_exp_accepts_complex_multiplier_offset_and_expression_andy(self):
        from coeff_program_chain import COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["legacy", "exp", "poly", "poly", "p1", "p2", "real(p1)"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LEGACY])
        token = compiled["tokens"][0]
        self.assertEqual(token["n_args"], 4)
        self.assertEqual(token["expr_refs"], [0, 1, 2, 3])
        # andy real(p1) dedups to the multiplier's real-component expression.
        self.assertEqual(token["andy_expr_ref"], 0)
        self.assertEqual(compiled["scalar_expr_count"], 4)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])

    def test_reported_littlewood_exp_program_shape_compiles(self):
        from coeff_program_chain import COEFF_OP_EMIT, COEFF_OP_LEGACY, COEFF_OP_LITTLEWOOD, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([
            ["littlewood", "push", "10", "-10j", "0"],
            ["legacy", "exp", "pop", "push", "1j+1", "1"],
            ["emit"],
        ])
        self.assertEqual(
            [tok["op"] for tok in compiled["tokens"]],
            [COEFF_OP_LITTLEWOOD, COEFF_OP_LEGACY, COEFF_OP_EMIT],
        )
        exp_token = compiled["tokens"][1]
        self.assertEqual(exp_token["n_args"], 4)
        self.assertEqual(exp_token["args"], [1.0, 1.0, 1.0, 0.0])
        self.assertEqual(compiled["stack_max"], 1)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])

    def test_pow_old_four_real_args_remain_accepted(self):
        from coeff_program_chain import COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["legacy", "pow", "poly", "poly", "1", "2", "3", "4"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LEGACY])
        self.assertEqual(compiled["tokens"][0]["n_args"], 4)
        self.assertEqual(compiled["legacy_coeff_transforms"], [["pow", "1", "2", "3", "4"]])

    def test_legacy_direct_chip_allows_redundant_emit_commit(self):
        from coeff_program_chain import COEFF_OP_EMIT, COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["legacy", "rev", "poly", "poly"], ["emit"]])
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
        coeff_chain = [["legacy", "rev", "poly", "poly"], ["emit"]]
        param = compile_param_program_chain(param_chain)
        coeff = compile_coeff_program_chain(coeff_chain)
        self.assertEqual(param["token_count"], 4)
        self.assertEqual(coeff["token_count"], 2)

    def test_push_const_with_poly_len_param_expression_uses_vm_path(self):
        from coeff_program_chain import COEFF_OP_CONST, COEFF_OP_EMIT, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["push_const", "poly_len", "p1+p2"], ["emit"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_CONST, COEFF_OP_EMIT])
        self.assertEqual(compiled["tokens"][0]["args"][0], -1)
        self.assertEqual(compiled["stack_max"], 1)
        self.assertEqual(compiled["scalar_expr_count"], 1)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])
        self.assertEqual(compiled["legacy_coeff_transforms"], [])

        alias = compile_coeff_program_chain([["const", "35", "p1+p2"], ["emit"]])
        self.assertEqual([tok["op"] for tok in alias["tokens"]], [COEFF_OP_CONST, COEFF_OP_EMIT])
        self.assertEqual(alias["tokens"][0]["args"][0], 35)

    def test_scalar_expressions_accept_pi_constants(self):
        import math
        from coeff_program_chain import COEFF_OP_CONST, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["push_const", "poly_len", "pi + pi2i"], ["emit"]])
        token = compiled["tokens"][0]
        self.assertEqual(token["op"], COEFF_OP_CONST)
        self.assertAlmostEqual(token["args"][1], math.pi)
        self.assertAlmostEqual(token["args_im"][1], 2.0 * math.pi)
        self.assertEqual(token["expr_refs"], [-1, -1])
        self.assertEqual(compiled["scalar_expr_count"], 0)
        with self.assertRaisesRegex(RuntimeError, "real-valued"):
            compile_coeff_program_chain([["blend", "pi2i"]])

    def test_source_native_ops_compile(self):
        from coeff_program_chain import (
            COEFF_OP_AFFINE,
            COEFF_OP_EMIT,
            COEFF_OP_RANGE,
            COEFF_OP_SET,
            COEFF_OP_TYPED_BINARY,
            COEFF_OP_TYPED_POKE_POLY,
            COEFF_OP_TYPED_PUSH_SCALAR,
            COEFF_OP_TYPED_PUSH_VECTOR,
            COEFF_OP_VECTOR_UNARY,
        )
        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source("""
            arange(1, poly_len + 1)
            linear(1j*p1, p2)
            poly = sqrt(pop)
            poly[10] = p1*p2*real(poly[6]) + imag(poly[18])*p1**3
            emit
        """)
        ops = [tok["op"] for tok in compiled["tokens"]]
        self.assertEqual(ops[0], COEFF_OP_RANGE)
        self.assertIn(COEFF_OP_TYPED_PUSH_VECTOR, ops)
        self.assertIn(COEFF_OP_TYPED_PUSH_SCALAR, ops)
        self.assertIn(COEFF_OP_TYPED_BINARY, ops)
        self.assertIn(COEFF_OP_VECTOR_UNARY, ops)
        self.assertIn(COEFF_OP_TYPED_POKE_POLY, ops)
        self.assertNotIn(COEFF_OP_AFFINE, ops)
        self.assertEqual(ops[-1], COEFF_OP_EMIT)
        self.assertEqual(compiled["source_statement_count"], 5)
        self.assertEqual(compiled["scalar_expr_count"], 1)

        copy = compile_coeff_program_source("poly = cf")
        self.assertEqual(copy["tokens"][0]["op"], COEFF_OP_SET)

    def test_source_accepts_typed_tokens_from_chain_serializer(self):
        # Populating from a saved result synthesizes coeff source from the saved
        # chain; for a const polynomial that chain is the lowered "_typed_*"
        # poke ladder (chain->source serializers emit these). Coeff programs
        # reach native only as source text, so the parser must accept these
        # tokens or the populated program cannot compute (the reported bug:
        # "unknown coeff program source function '_typed_push_scalar'").
        from coeff_program_chain import (
            COEFF_OP_TYPED_BINARY,
            COEFF_OP_TYPED_POKE_POLY,
            COEFF_OP_TYPED_PUSH_SCALAR,
            COEFF_OP_TYPED_UNARY,
        )
        from coeff_program_source import compile_coeff_program_source

        src = (
            "_typed_push_scalar(0.0+0.0j)\n"
            "_typed_push_scalar(1.0+0.0j)\n"
            "_typed_poke_poly\n"
            "_typed_push_scalar(19.0+0.0j)\n"
            "_typed_push_scalar(9.0+0.0j)\n"
            "_typed_unary(neg)\n"
            "_typed_push_scalar(p1)\n"
            "_typed_push_scalar(1000.0+0.0j)\n"
            "_typed_binary(multiply)\n"
            "_typed_binary(add)\n"
            "_typed_poke_poly\n"
        )
        compiled = compile_coeff_program_source(src)
        ops = [tok["op"] for tok in compiled["tokens"]]
        self.assertEqual(compiled["token_count"], 11)
        self.assertIn(COEFF_OP_TYPED_PUSH_SCALAR, ops)
        self.assertIn(COEFF_OP_TYPED_POKE_POLY, ops)
        self.assertIn(COEFF_OP_TYPED_BINARY, ops)
        self.assertIn(COEFF_OP_TYPED_UNARY, ops)

    def test_chain_serializer_falls_back_when_pretty_scalar_source_is_not_byte_preserving(self):
        from coeff_program_chain import compile_coeff_program_chain
        from coeff_program_source import coeff_source_text_from_chain, compile_coeff_program_source

        chain = [
            ["_typed_push_scalar", "29.0+0.0j"],
            ["_typed_push_scalar", "1.0+0.0j"],
            ["_typed_push_scalar", "p2"],
            ["_typed_unary", "abs"],
            ["_typed_binary", "multiply"],
            ["_typed_push_scalar", "0.0+1.0j"],
            ["_typed_binary", "multiply"],
            ["_typed_poke_poly"],
        ]
        source = coeff_source_text_from_chain(chain)
        self.assertIn("_typed_push_scalar(0.0+1.0j)", source)
        self.assertEqual(
            compile_coeff_program_source(source)["fingerprint"],
            compile_coeff_program_chain(chain)["fingerprint"],
        )

    def test_typed_op_passthrough_covers_chain_compiler(self):
        # Every "_typed_*" op the chain compiler accepts must also be accepted by
        # the source parser, so chip->source->parse round-trips stay closed.
        import coeff_program_chain as cc
        from coeff_program_source import _ROUNDTRIP_PASSTHROUGH_NAMES

        compiler_typed_ops = {
            name
            for name in {**cc._CHIP_COMPILERS, **cc._ZERO_ARG_CHIP_OPS}
            if name.startswith("_typed_") or name.startswith("_native_transform")
        }
        self.assertTrue(compiler_typed_ops)
        missing = compiler_typed_ops - _ROUNDTRIP_PASSTHROUGH_NAMES
        self.assertEqual(missing, set(), f"source parser missing typed ops: {sorted(missing)}")

    def test_source_dynamic_index_and_mixed_ops_compile_to_typed_tokens(self):
        from coeff_program_chain import (
            COEFF_OP_TYPED_BINARY,
            COEFF_OP_TYPED_FILL,
            COEFF_OP_TYPED_GET_SCALAR,
            COEFF_OP_TYPED_POKE_POLY,
            COEFF_OP_TYPED_SET_POLY,
            COEFF_OP_TYPED_PUSH_SCALAR,
            COEFF_OP_TYPED_PUSH_VECTOR,
        )
        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source("""
            poly[poly_len - 1] = p1 + poly[poly_len - 2]
            poly = multiply(poly, p2)
            poly = add(poly, fill(poly_len, p1))
        """)
        ops = [tok["op"] for tok in compiled["tokens"]]
        self.assertIn(COEFF_OP_TYPED_GET_SCALAR, ops)
        self.assertIn(COEFF_OP_TYPED_POKE_POLY, ops)
        self.assertIn(COEFF_OP_TYPED_BINARY, ops)
        self.assertIn(COEFF_OP_TYPED_FILL, ops)
        self.assertIn(COEFF_OP_TYPED_SET_POLY, ops)
        self.assertIn(COEFF_OP_TYPED_PUSH_SCALAR, ops)
        self.assertIn(COEFF_OP_TYPED_PUSH_VECTOR, ops)
        self.assertEqual(compiled["stack_max"], 4)

    def test_source_elementary_vector_unary_uses_typed_vector_path(self):
        from coeff_program_chain import COEFF_OP_NATIVE_TRANSFORM, COEFF_OP_VECTOR_UNARY, compile_coeff_program_chain
        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source("poly = sin(poly)\nemit")
        ops = [tok["op"] for tok in compiled["tokens"]]
        self.assertEqual(ops[0], COEFF_OP_VECTOR_UNARY)
        self.assertNotIn(COEFF_OP_NATIVE_TRANSFORM, ops)

        affine_exp = compile_coeff_program_source("poly = exp_affine(poly, 1j, 0)\nemit")
        self.assertIn(COEFF_OP_NATIVE_TRANSFORM, [tok["op"] for tok in affine_exp["tokens"]])

        with self.assertRaisesRegex(RuntimeError, "use exp_affine"):
            compile_coeff_program_source("poly = exp(poly, 1j, 0)")

        alias = compile_coeff_program_chain([["exp_affine", "poly", "poly", "1j", "0", "0.25"]])
        self.assertEqual([tok["op"] for tok in alias["tokens"]], [COEFF_OP_NATIVE_TRANSFORM])

        legacy_style = compile_coeff_program_chain([["exp", "poly", "poly", "1j", "0", "0.25"]])
        self.assertEqual([tok["op"] for tok in legacy_style["tokens"]], [COEFF_OP_NATIVE_TRANSFORM])

    def test_source_push_vec_and_push_scalar_compile_to_typed_stack(self):
        from coeff_program_chain import (
            COEFF_OP_CONST,
            COEFF_OP_EMIT,
            COEFF_OP_POP,
            COEFF_OP_TYPED_BINARY,
            COEFF_OP_TYPED_FILL,
            COEFF_OP_TYPED_PUSH_SCALAR,
            compile_coeff_program_chain,
        )
        from coeff_program_source import compile_coeff_program_source

        scalar = compile_coeff_program_source("push_scalar(p1)\npush_scalar(p2)\nadd()\ndrop")
        scalar_ops = [tok["op"] for tok in scalar["tokens"]]
        self.assertEqual(
            scalar_ops,
            [
                COEFF_OP_TYPED_PUSH_SCALAR,
                COEFF_OP_TYPED_PUSH_SCALAR,
                COEFF_OP_TYPED_BINARY,
                COEFF_OP_POP,
            ],
        )
        self.assertEqual(scalar["stack_max"], 2)

        vector = compile_coeff_program_source("push_vec(p1)\npush_scalar(p2)\nmultiply()\nemit")
        vector_ops = [tok["op"] for tok in vector["tokens"]]
        self.assertIn(COEFF_OP_TYPED_FILL, vector_ops)
        self.assertIn(COEFF_OP_TYPED_BINARY, vector_ops)
        self.assertEqual(vector_ops[-1], COEFF_OP_EMIT)

        alias = compile_coeff_program_chain([["push_vec", "poly_len", "p1"], ["emit"]])
        self.assertEqual([tok["op"] for tok in alias["tokens"]], [COEFF_OP_CONST, COEFF_OP_EMIT])

        with self.assertRaisesRegex(RuntimeError, "poly assignment requires a vector"):
            compile_coeff_program_source("poly = push_scalar(p1)")

    def test_source_vector_cf_staging_preserves_binary_order(self):
        from coeff_program_chain import COEFF_OP_PUSH, COEFF_OP_VECTOR_BINARY
        from coeff_program_source import parse_coeff_program_source, compile_coeff_program_source

        parsed = parse_coeff_program_source("poly = sub(poly, cf)")
        self.assertEqual(parsed["chain"], [
            ["push", "cf"],
            ["push", "poly"],
            ["subtract", "poly", "pop", "pop"],
        ])
        compiled = compile_coeff_program_source("poly = sub(poly, cf)")
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_PUSH, COEFF_OP_PUSH, COEFF_OP_VECTOR_BINARY])

    def test_source_rejects_standalone_pop_and_peek(self):
        from coeff_program_source import compile_coeff_program_source

        with self.assertRaisesRegex(RuntimeError, "pop is not a standalone statement"):
            compile_coeff_program_source("pop")
        with self.assertRaisesRegex(RuntimeError, "peek is not a standalone statement"):
            compile_coeff_program_source("peek")

        dropped = compile_coeff_program_source("cf\ndrop")
        self.assertEqual(dropped["source_statement_count"], 2)
        assigned = compile_coeff_program_source("cf\npoly = pop\nemit")
        self.assertEqual(assigned["source_statement_count"], 3)

    def test_source_accepts_explicit_poke_calls_for_chain_roundtrip(self):
        from coeff_program_chain import COEFF_OP_POKE_TOS, COEFF_OP_TYPED_POKE_POLY
        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source("push_const(3, 0)\npoke_tos(1, p1)\npoke_poly(2, p2)\ndrop")
        self.assertIn(COEFF_OP_POKE_TOS, [tok["op"] for tok in compiled["tokens"]])
        self.assertIn(COEFF_OP_TYPED_POKE_POLY, [tok["op"] for tok in compiled["tokens"]])

    def test_source_accepts_explicit_legacy_wrapper_for_wire_preserving_roundtrip(self):
        from coeff_program_chain import COEFF_OP_LEGACY, compile_coeff_program_chain
        from coeff_program_source import compile_coeff_program_source

        source = compile_coeff_program_source("legacy(rev, poly, poly)")
        chain = compile_coeff_program_chain([["legacy", "rev", "poly", "poly"]])
        self.assertEqual(source["tokens"][0]["op"], COEFF_OP_LEGACY)
        self.assertEqual(source["fingerprint"], chain["fingerprint"])

        with self.assertRaisesRegex(RuntimeError, "legacy names its own target"):
            compile_coeff_program_source("poly = legacy(rev, poly, poly)")
        with self.assertRaisesRegex(RuntimeError, "unknown legacy coeff transform 'nope'"):
            compile_coeff_program_source("legacy(nope, poly, poly)")

    def test_source_native_transform_args_lower_to_typed_stack_args(self):
        from coeff_program_chain import COEFF_OP_NATIVE_TRANSFORM, COEFF_OP_TYPED_PUSH_SCALAR
        from coeff_program_source import compile_coeff_program_source

        compiled = compile_coeff_program_source("poly = exp_affine(poly, p1, p2)\nemit")
        ops = [tok["op"] for tok in compiled["tokens"]]
        self.assertEqual(ops[:3], [COEFF_OP_TYPED_PUSH_SCALAR, COEFF_OP_TYPED_PUSH_SCALAR, COEFF_OP_NATIVE_TRANSFORM])
        self.assertEqual(compiled["tokens"][2]["stack_arg_count"], 2)
        self.assertEqual(compiled["scalar_expr_count"], 0)

    def test_native_transform_stack_args_validate_arity_and_preserve_andy(self):
        from coeff_program_chain import COEFF_OP_NATIVE_TRANSFORM, compile_coeff_program_chain
        from coeff_program_source import compile_coeff_program_source

        with self.assertRaisesRegex(RuntimeError, "rev stack arg count must be <= 0"):
            compile_coeff_program_chain([["_native_transform_stack_args", "rev", "poly", "poly", "1"]])

        compiled = compile_coeff_program_source("poly = exp_affine(poly, p1, p2, real(p1))\nemit")
        token = next(tok for tok in compiled["tokens"] if tok["op"] == COEFF_OP_NATIVE_TRANSFORM)
        self.assertEqual(token["stack_arg_count"], 2)
        self.assertEqual(token["andy_expr_ref"], 0)
        self.assertEqual(compiled["scalar_expr_count"], 1)

    def test_typed_stack_validator_rejects_type_mismatches(self):
        from coeff_program_chain import compile_coeff_program_chain

        with self.assertRaisesRegex(RuntimeError, "get_scalar.*source is scalar"):
            compile_coeff_program_chain([
                ["_typed_push_scalar", "1"],
                ["_typed_push_scalar", "0"],
                ["_typed_get_scalar"],
            ])

        with self.assertRaisesRegex(RuntimeError, "typed_blend.*t is vector"):
            compile_coeff_program_chain([
                ["push_const", "2", "1"],
                ["push_const", "2", "2"],
                ["push", "cf"],
                ["_typed_blend"],
            ])

    def test_push_linspace_with_poly_len_compiles(self):
        from coeff_program_chain import COEFF_OP_EMIT, COEFF_OP_LINSPACE, COEFF_OP_RANGE, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["push_linspace", "poly_len"], ["emit"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LINSPACE, COEFF_OP_EMIT])
        self.assertEqual(compiled["tokens"][0]["args"], [-1])
        self.assertEqual(compiled["stack_max"], 1)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])

        range_compiled = compile_coeff_program_chain([["push_range", "poly_len"], ["emit"]])
        self.assertEqual([tok["op"] for tok in range_compiled["tokens"]], [COEFF_OP_RANGE, COEFF_OP_EMIT])
        self.assertEqual(range_compiled["tokens"][0]["args"], [-1])

    def test_scalar_expressions_accept_abs_and_log(self):
        from coeff_program_chain import (
            EXPR_ABS,
            EXPR_ANGLE,
            EXPR_COS,
            EXPR_COSH,
            EXPR_EXP,
            EXPR_LOG,
            EXPR_SIN,
            EXPR_SINH,
            EXPR_SQRT,
            EXPR_TAN,
            EXPR_TANH,
            compile_coeff_program_chain,
        )

        compiled = compile_coeff_program_chain([
            ["push_const", "poly_len", "log(abs(p1+p2)+1)*1j"],
            ["emit"],
            ["push_const", "poly_len", "sin(p1)+cos(p2)+exp(1j*p1)"],
            ["emit"],
            ["push_const", "poly_len", "sqrt(p2)+tan(p1)+sinh(p1)"],
            ["emit"],
            ["push_const", "poly_len", "cosh(p2)+tanh(p1)+angle(p2)"],
            ["emit"],
        ])
        self.assertEqual(compiled["scalar_expr_count"], 4)
        ops = []
        for expr in compiled["scalar_exprs"]:
            ops.extend(int(expr[idx]) for idx in range(0, len(expr), 3))
        self.assertIn(EXPR_ABS, ops)
        self.assertIn(EXPR_LOG, ops)
        self.assertIn(EXPR_SIN, ops)
        self.assertIn(EXPR_COS, ops)
        self.assertIn(EXPR_EXP, ops)
        self.assertIn(EXPR_SQRT, ops)
        self.assertIn(EXPR_TAN, ops)
        self.assertIn(EXPR_SINH, ops)
        self.assertIn(EXPR_COSH, ops)
        self.assertIn(EXPR_TANH, ops)
        self.assertIn(EXPR_ANGLE, ops)

    def test_scalar_expression_elementary_functions_static_fold(self):
        from coeff_program_chain import compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["push_const", "1", "sin(pi/2)"], ["emit"]])
        token = compiled["tokens"][0]
        self.assertEqual(token["expr_refs"], [-1, -1])
        self.assertAlmostEqual(token["args"][1], 1.0)
        self.assertAlmostEqual(token["args_im"][1], 0.0)
        self.assertEqual(compiled["scalar_expr_count"], 0)

    def test_legacy_chip_accepts_explicit_src_tgt(self):
        from coeff_program_chain import COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["push", "cf"], ["legacy", "rev", "pop", "push"], ["emit"]])
        self.assertEqual(compiled["tokens"][1]["op"], COEFF_OP_LEGACY)
        self.assertEqual(compiled["tokens"][1]["src"], 3)
        self.assertEqual(compiled["tokens"][1]["tgt"], 5)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])

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
            ["push_const", "3", "p1+p2"],
            ["emit"],
            ["push_const", "3", "p1-p2"],
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

    def test_vector_binary_unary_roll_and_argsort_compile(self):
        from coeff_program_chain import (
            COEFF_OP_EMIT,
            COEFF_OP_VECTOR_ARGSORT,
            COEFF_OP_VECTOR_BINARY,
            COEFF_OP_VECTOR_ROLL,
            COEFF_OP_VECTOR_UNARY,
            compile_coeff_program_chain,
        )

        compiled = compile_coeff_program_chain([
            ["add", "push", "poly", "poly"],
            ["angle", "push", "peek"],
            ["roll", "push", "peek", "2"],
            ["argsort", "push", "peek", "poly"],
            ["emit"],
            ["emit"],
            ["emit"],
            ["emit"],
        ])
        self.assertEqual(
            [tok["op"] for tok in compiled["tokens"]],
            [
                COEFF_OP_VECTOR_BINARY,
                COEFF_OP_VECTOR_UNARY,
                COEFF_OP_VECTOR_ROLL,
                COEFF_OP_VECTOR_ARGSORT,
                COEFF_OP_EMIT,
                COEFF_OP_EMIT,
                COEFF_OP_EMIT,
                COEFF_OP_EMIT,
            ],
        )
        self.assertEqual(compiled["tokens"][0]["fn_index"], 1)
        self.assertEqual(compiled["tokens"][1]["fn_index"], 1)
        self.assertEqual(compiled["tokens"][2]["fn_index"], 1)
        self.assertEqual(compiled["stack_max"], 4)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])

    def test_vector_power_is_not_legacy_power(self):
        from coeff_program_chain import (
            COEFF_OP_DUPLICATE,
            COEFF_OP_EMIT,
            COEFF_OP_PUSH,
            COEFF_OP_VECTOR_BINARY,
            compile_coeff_program_chain,
        )

        compiled = compile_coeff_program_chain([
            ["push", "cf"],
            ["duplicate"],
            ["power", "push", "pop", "pop"],
            ["emit"],
        ])
        self.assertEqual(
            [tok["op"] for tok in compiled["tokens"]],
            [COEFF_OP_PUSH, COEFF_OP_DUPLICATE, COEFF_OP_VECTOR_BINARY, COEFF_OP_EMIT],
        )
        self.assertEqual(compiled["tokens"][2]["fn_index"], 5)
        with self.assertRaisesRegex(RuntimeError, "unknown coeff program chip: poly-power"):
            compile_coeff_program_chain([["poly-power", "poly", "poly", "push", "pop"]])

    def test_vector_pop_sources_are_validated_in_order(self):
        from coeff_program_chain import compile_coeff_program_chain

        with self.assertRaisesRegex(RuntimeError, "vector src1"):
            compile_coeff_program_chain([["add", "poly", "pop", "poly"]])

        compiled = compile_coeff_program_chain([
            ["push", "cf"],
            ["push", "poly"],
            ["subtract", "push", "pop", "pop"],
            ["emit"],
        ])
        self.assertEqual(compiled["stack_max"], 2)

        with self.assertRaisesRegex(RuntimeError, "roll n must be an integer"):
            compile_coeff_program_chain([["roll", "poly", "poly", "1.5"]])

    def test_littlewood_compiles_complex_fields_and_andy_expression(self):
        from coeff_program_chain import COEFF_OP_EMIT, COEFF_OP_LITTLEWOOD, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([
            ["littlewood", "push", "p1", "p2", "real(p1)"],
            ["emit"],
        ])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LITTLEWOOD, COEFF_OP_EMIT])
        self.assertEqual(compiled["tokens"][0]["tgt"], 5)
        self.assertEqual(compiled["tokens"][0]["n_args"], 3)
        self.assertEqual(compiled["tokens"][0]["expr_refs"], [0, 1, 2])
        self.assertEqual(compiled["scalar_expr_count"], 3)
        self.assertEqual(compiled["stack_max"], 1)
        self.assertFalse(compiled["uses_legacy_chain_equivalent"])

    def test_macro_expands_before_hashing(self):
        from coeff_program_chain import compile_coeff_program_chain

        macros = {"smooth": [["legacy", "rev", "poly", "poly"], ["legacy", "cumsum", "poly", "poly"]]}

        def resolver(name):
            return macros[name]

        macro = compile_coeff_program_chain([["macro", "smooth"]], macro_resolver=resolver)
        expanded = compile_coeff_program_chain([["legacy", "rev", "poly", "poly"], ["legacy", "cumsum", "poly", "poly"]])
        self.assertEqual(macro["fingerprint"], expanded["fingerprint"])
        self.assertEqual(macro["macro_expansions"], 1)
        self.assertEqual(macro["display"], "macro(smooth)")

    def test_invalid_final_stack_is_rejected(self):
        from coeff_program_chain import compile_coeff_program_chain

        with self.assertRaisesRegex(RuntimeError, "final stack depth"):
            compile_coeff_program_chain([["push", "cf"]])

    def test_program_token_cap_allows_typed_source_expansion_over_old_64_limit(self):
        from coeff_program_chain import MAX_PROGRAM_TOKENS, compile_coeff_program_chain

        chain = []
        for idx in range(65):
            chain.append(["push_const", "1", str(idx)])
            chain.append(["emit"])
        compiled = compile_coeff_program_chain(chain)

        self.assertEqual(compiled["token_count"], 130)
        self.assertLessEqual(compiled["token_count"], MAX_PROGRAM_TOKENS)
        self.assertEqual(compiled["stack_max"], 1)

    def test_legacy_args_are_real_int_or_enum_only(self):
        from coeff_program_chain import legacy_registry

        allowed_complex_args = {("round", "multiplier")}
        for spec in legacy_registry()["by_name"].values():
            for arg in spec["args"]:
                if (spec["name"], arg.get("name")) in allowed_complex_args:
                    self.assertEqual(arg["type"], "complex")
                else:
                    self.assertIn(arg["type"], {"real", "int", "enum"})

    def test_scalar_expressions_can_read_coeff_register_values(self):
        from coeff_program_chain import (
            EXPR_CF_AT,
            EXPR_POLY_AT,
            EXPR_POLY_LEN,
            EXPR_T1,
            EXPR_T2,
            EXPR_TOS_AT,
            compile_coeff_program_chain,
        )

        compiled = compile_coeff_program_chain([
            ["push", "cf"],
            ["poke_poly", "0", "cf1 + poly2 + tos3 + poly_len + t1 + t2 + p1 + p2"],
            ["pop"],
        ])
        self.assertEqual(compiled["scalar_expr_count"], 1)
        expr = compiled["scalar_exprs"][0]
        ops = [int(expr[i]) for i in range(0, len(expr), 3)]
        self.assertIn(EXPR_CF_AT, ops)
        self.assertIn(EXPR_POLY_AT, ops)
        self.assertIn(EXPR_TOS_AT, ops)
        self.assertIn(EXPR_POLY_LEN, ops)
        self.assertIn(EXPR_T1, ops)
        self.assertIn(EXPR_T2, ops)

    def test_round_accepts_compact_complex_multiplier_expression(self):
        from coeff_program_chain import COEFF_OP_LEGACY, compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["legacy", "round", "poly", "poly", "p1"]])
        self.assertEqual([tok["op"] for tok in compiled["tokens"]], [COEFF_OP_LEGACY])
        self.assertEqual(compiled["tokens"][0]["n_args"], 2)
        self.assertEqual(compiled["tokens"][0]["expr_refs"], [0, 1])
        self.assertEqual(compiled["scalar_expr_count"], 2)

        old_with_andy = compile_coeff_program_chain([["legacy", "round", "poly", "poly", "1", "2", "0.25"]])
        self.assertEqual(old_with_andy["tokens"][0]["args"], [1.0, 2.0])
        self.assertEqual(old_with_andy["tokens"][0]["andy"], 0.25)


class TestCoeffProgramReviewFixes(unittest.TestCase):
    """Regression tests for the code-review-1.md fixes."""

    def _parse(self, text, strict=False):
        from coeff_program_source import parse_coeff_program_source
        return parse_coeff_program_source(text, strict=strict)

    def test_assignment_to_range_and_linspace_sets_poly(self):
        for text in ("poly = range(5)", "poly = linspace(4)", "poly = arange(1, 6)"):
            parsed = self._parse(text)
            self.assertEqual(parsed["diagnostics"], [], text)
            self.assertEqual(parsed["chain"][-1], ["_typed_set_poly"], text)

    def test_assignment_to_statement_only_forms_is_rejected(self):
        for text in ("poly = affine(poly, cf, 1, 0)", "poly = macro(x)", "poly = poke_tos(1, 2)"):
            parsed = self._parse(text)
            self.assertTrue(parsed["diagnostics"], text)

    def test_lowering_errors_carry_statement_line_numbers(self):
        parsed = self._parse("poly = sin(poly)\npoly = nosuchfn(poly)")
        self.assertEqual(len(parsed["diagnostics"]), 1)
        self.assertEqual(parsed["diagnostics"][0]["line"], 2)
        self.assertIn("code", parsed["diagnostics"][0])

    def test_source_parser_uses_shared_core_diagnostic_codes(self):
        parsed = self._parse("poly = sin(poly\npoly = cos(poly)")
        self.assertEqual(parsed["chain"], [])
        self.assertEqual(len(parsed["diagnostics"]), 1)
        self.assertEqual(parsed["diagnostics"][0]["code"], "unclosed_parenthesis")

    def test_strict_source_errors_keep_structured_diagnostics(self):
        from coeff_program_source import CoeffProgramSourceCompileError, parse_coeff_program_source
        with self.assertRaises(CoeffProgramSourceCompileError) as ctx:
            parse_coeff_program_source("poly = sin(poly", strict=True)
        self.assertEqual(ctx.exception.diagnostics[0]["code"], "unclosed_parenthesis")
        self.assertIn("line", ctx.exception.diagnostics[0])
        self.assertIn("column", ctx.exception.diagnostics[0])

    def test_source_parser_validates_registry_transform_args(self):
        parsed = self._parse("poly = roots(poly, 2.7, lo)")
        self.assertEqual(len(parsed["diagnostics"]), 1)
        self.assertEqual(parsed["diagnostics"][0]["code"], "bad_native_transform")
        self.assertIn("integer", parsed["diagnostics"][0]["message"])

        parsed = self._parse("poly = roots(poly, 1, 2, 3)")
        self.assertEqual(len(parsed["diagnostics"]), 1)
        self.assertEqual(parsed["diagnostics"][0]["code"], "bad_native_transform")

        parsed = self._parse("legacy(roots, poly, poly, 2.7, lo)")
        self.assertEqual(len(parsed["diagnostics"]), 1)
        self.assertEqual(parsed["diagnostics"][0]["code"], "bad_legacy_transform")

    def test_source_parser_splits_shared_core_comments_semicolons_and_brackets(self):
        parsed = self._parse("poly[poly_len - 1] = p1; # tail edit\npoly = cos(poly)")
        self.assertEqual(parsed["diagnostics"], [])
        self.assertEqual(parsed["statement_count"], 2)

    def test_compound_index_expressions_parse_in_value_position(self):
        from coeff_program_chain import compile_coeff_program_chain
        parsed = self._parse("poly = add(cf[0]*cf[1], poly)")
        self.assertEqual(parsed["diagnostics"], [])
        compile_coeff_program_chain(parsed["chain"])
        parsed = self._parse("poly = add(cf[poly_len-1], poly)")
        self.assertEqual(parsed["diagnostics"], [])

    def test_round_old_form_compiles_from_source_text(self):
        from coeff_program_chain import COEFF_OP_NATIVE_TRANSFORM, compile_coeff_program_chain
        parsed = self._parse("poly = round(poly, 1, 2, 0.5)")
        self.assertEqual(parsed["diagnostics"], [])
        compiled = compile_coeff_program_chain(parsed["chain"])
        token = next(t for t in compiled["tokens"] if t["op"] == COEFF_OP_NATIVE_TRANSFORM)
        self.assertEqual(token["fn_index"], 23)
        self.assertEqual(token["andy"], 0.5)

    def test_round_two_arg_source_treats_second_arg_as_andy(self):
        from coeff_program_chain import COEFF_OP_NATIVE_TRANSFORM, compile_coeff_program_chain
        parsed = self._parse("poly = round(poly, 1, 2)")
        self.assertEqual(parsed["diagnostics"], [])
        compiled = compile_coeff_program_chain(parsed["chain"])
        token = next(t for t in compiled["tokens"] if t["op"] == COEFF_OP_NATIVE_TRANSFORM)
        self.assertEqual(token["fn_index"], 23)
        self.assertEqual(token["stack_arg_count"], 1)
        self.assertEqual(token["andy"], 2.0)

    def test_trig_with_andy_routes_to_native_transform(self):
        from coeff_program_chain import (
            COEFF_OP_NATIVE_TRANSFORM,
            COEFF_OP_VECTOR_UNARY,
            compile_coeff_program_chain,
        )
        parsed = self._parse("poly = cos(poly, 0.5)")
        self.assertEqual(parsed["diagnostics"], [])
        compiled = compile_coeff_program_chain(parsed["chain"])
        token = compiled["tokens"][0]
        self.assertEqual(token["op"], COEFF_OP_NATIVE_TRANSFORM)
        self.assertEqual(token["fn_index"], 17)
        self.assertEqual(token["andy"], 0.5)
        bare = compile_coeff_program_chain(self._parse("poly = cos(poly)")["chain"])
        self.assertEqual(bare["tokens"][0]["op"], COEFF_OP_VECTOR_UNARY)

    def test_shadowed_transform_aliases_compile(self):
        from coeff_program_chain import COEFF_OP_NATIVE_TRANSFORM, compile_coeff_program_chain
        cases = {
            "poly = pow_affine(poly, 2, 1)": 24,
            "poly = power_series(poly, 8)": 25,
            "poly = exp_affine(poly)": 16,
            "poly = linear(poly, 2, 1, 0.5)": 14,
        }
        for text, fn_index in cases.items():
            parsed = self._parse(text)
            self.assertEqual(parsed["diagnostics"], [], text)
            compiled = compile_coeff_program_chain(parsed["chain"])
            token = next(t for t in compiled["tokens"] if t["op"] == COEFF_OP_NATIVE_TRANSFORM)
            self.assertEqual(token["fn_index"], fn_index, text)

    def test_scalar_expr_count_is_capped_and_deduped(self):
        from coeff_program_chain import MAX_SCALAR_EXPRS, compile_coeff_program_chain
        big = [["poke_poly", str(i % 4), f"p1+p1*{i}"] for i in range(MAX_SCALAR_EXPRS + 6)]
        with self.assertRaisesRegex(RuntimeError, "scalar expressions"):
            compile_coeff_program_chain(big)
        deduped = compile_coeff_program_chain([
            ["poke_poly", "0", "p1+p2"],
            ["poke_poly", "1", "p1+p2"],
        ])
        self.assertEqual(deduped["scalar_expr_count"], 1)
        refs = [t["expr_refs"][1] for t in deduped["tokens"]]
        self.assertEqual(refs, [0, 0])

    def test_macro_expansion_is_budgeted(self):
        from coeff_program_chain import compile_coeff_program_chain

        def resolver(macro_id):
            n = int(macro_id[1:])
            if n >= 8:
                return [["push_range", "4"], ["emit"]]
            return [["macro", f"m{n + 1}"]] * 4

        with self.assertRaisesRegex(RuntimeError, "macro"):
            compile_coeff_program_chain([["macro", "m1"]], macro_resolver=resolver)

        def small(macro_id):
            return [["push_range", "4"], ["emit"]]

        ok = compile_coeff_program_chain([["macro", "a"], ["macro", "b"]], macro_resolver=small)
        self.assertEqual(ok["token_count"], 4)

    def test_power_expansion_is_budgeted_before_materializing(self):
        from coeff_program_chain import compile_coeff_program_chain
        with self.assertRaises(RuntimeError):
            compile_coeff_program_chain([["push_scalar", "(((p1**32)**32)**32)**32"], ["pop"]])
        ok = compile_coeff_program_chain([["push_scalar", "p1**16"], ["pop"]])
        self.assertEqual(ok["token_count"], 2)
        with self.assertRaisesRegex(RuntimeError, "exponent magnitude"):
            compile_coeff_program_chain([["push_scalar", "p1**33"], ["pop"]])

    def test_signed_zero_is_canonical_in_token_args(self):
        import math
        from coeff_program_chain import compile_coeff_program_chain
        negative = compile_coeff_program_chain([["push_const", "4", "-(0)"], ["emit"]])
        positive = compile_coeff_program_chain([["push_const", "4", "0"], ["emit"]])
        self.assertEqual(negative["fingerprint"], positive["fingerprint"])
        for value in negative["tokens"][0]["args"] + negative["tokens"][0].get("args_im", []):
            self.assertGreater(math.copysign(1.0, value), 0.0)

    def test_static_range_lengths_are_bounds_checked(self):
        from coeff_program_chain import compile_coeff_program_chain
        for bad in ("500", "0", "-3", "-1", "250+250"):
            with self.assertRaisesRegex(RuntimeError, r"\[1,256\]", msg=bad):
                compile_coeff_program_chain([["push_range", bad], ["emit"]])
        for good in ("4", "poly_len", "2+2"):
            compile_coeff_program_chain([["push_range", good], ["emit"]])
        dynamic = compile_coeff_program_chain([["push_range", "poly_len-1"], ["emit"]])
        self.assertEqual(dynamic["scalar_expr_count"], 1)

    def test_roll_shift_and_poke_index_are_validated(self):
        from coeff_program_chain import compile_coeff_program_chain
        with self.assertRaisesRegex(RuntimeError, "roll n"):
            compile_coeff_program_chain([["roll", "poly", "poly", "1e18"]])
        with self.assertRaisesRegex(RuntimeError, "poke_tos index"):
            compile_coeff_program_chain([["poke_tos", "1.9", "5"]])
        with self.assertRaisesRegex(RuntimeError, "stack arg count"):
            compile_coeff_program_chain([["_native_transform_stack_args", "linear", "poly", "poly", "1.5"]])

    def test_legacy_int_args_are_clamped(self):
        from coeff_program_chain import compile_coeff_program_chain
        with self.assertRaisesRegex(RuntimeError, "4096"):
            compile_coeff_program_chain([["legacy", "power", "poly", "poly", "1000000000"]])
        compile_coeff_program_chain([["legacy", "power", "poly", "poly", "64"]])

    def test_expression_constants_tau_and_bare_i_fold(self):
        import math
        from coeff_program_chain import compile_coeff_program_chain
        cases = {
            "tau": complex(2.0 * math.pi, 0.0),
            "tau_i": complex(0.0, 2.0 * math.pi),
            "pi2": complex(2.0 * math.pi, 0.0),
            "i": complex(0.0, 1.0),
        }
        for name, want in cases.items():
            compiled = compile_coeff_program_chain([["push_const", "2", name], ["emit"]])
            token = compiled["tokens"][0]
            self.assertAlmostEqual(token["args"][1], want.real, msg=name)
            self.assertAlmostEqual(token["args_im"][1], want.imag, msg=name)
            self.assertEqual(compiled["scalar_expr_count"], 0, name)

    def test_source_comments_and_semicolons(self):
        parsed = self._parse("# leading comment\ncf; rev # trailing\npoly = pop")
        self.assertEqual(parsed["diagnostics"], [])
        self.assertEqual(parsed["statement_count"], 3)
        parsed = self._parse("push_scalar(p1 # inline comment\n + 1)\ndrop")
        self.assertEqual(parsed["diagnostics"], [])

    def test_source_stack_ops_and_scalar_dup(self):
        from coeff_program_chain import compile_coeff_program_chain
        parsed = self._parse("push_scalar(1)\ndup\nswap\ndrop\ndrop\ncf\nflush")
        self.assertEqual(parsed["diagnostics"], [])
        compile_coeff_program_chain(parsed["chain"])

    def test_source_macro_statement_round_trips(self):
        from coeff_program_chain import compile_coeff_program_chain
        parsed = self._parse("macro(my-prog)\npoly = pop")
        self.assertEqual(parsed["diagnostics"], [])

        def resolver(macro_id):
            self.assertEqual(macro_id, "my-prog")
            return [["push_range", "4"]]

        compiled = compile_coeff_program_chain(parsed["chain"], macro_resolver=resolver)
        self.assertEqual(compiled["macro_expansions"], 1)

    def test_bare_tos_is_a_typed_vector_source(self):
        from coeff_program_chain import compile_coeff_program_chain
        parsed = self._parse("cf\npoly = add(tos, poly)\ndrop")
        self.assertEqual(parsed["diagnostics"], [])
        compile_coeff_program_chain(parsed["chain"])

    def test_registry_rejects_unknown_enum_choices(self):
        import coeff_program_chain as chain_mod
        original = dict(chain_mod._ENUM_ARG_VALUES)
        try:
            chain_mod._ENUM_ARG_VALUES.clear()
            chain_mod._ENUM_ARG_VALUES["hi"] = 0.0
            with self.assertRaisesRegex(RuntimeError, "enum choice"):
                chain_mod._load_legacy_registry()
        finally:
            chain_mod._ENUM_ARG_VALUES.clear()
            chain_mod._ENUM_ARG_VALUES.update(original)


if __name__ == "__main__":
    unittest.main()


class TestCodeReview3Fixes(unittest.TestCase):
    """Regression tests for the code-review-3.md fixes."""

    def _parse(self, text):
        from coeff_program_source import parse_coeff_program_source
        return parse_coeff_program_source(text, strict=False)

    def test_shadowed_transform_chips_compile_in_chain_mode(self):
        from coeff_program_chain import COEFF_OP_NATIVE_TRANSFORM, compile_coeff_program_chain
        for name, fn_index in (("pow_affine", 24), ("power_series", 25), ("exp_affine", 16)):
            compiled = compile_coeff_program_chain([[name, "poly", "poly"]])
            token = compiled["tokens"][0]
            self.assertEqual(token["op"], COEFF_OP_NATIVE_TRANSFORM, name)
            self.assertEqual(token["fn_index"], fn_index, name)

    def test_affine_shorthand_rejects_source_like_value_slots(self):
        cases = {
            "cf\npoly = linear(poly, 5)": "ambiguous",
            "cf\npoly = scale(poly)": "missing the multiplier",
            "cf\npoly = shift(poly)": "missing the offset",
            "poly = scale(2, 3)": "source must be",
            "poly = shift(2, 3)": "source must be",
            "linear(100, 0, 1)\ndrop": "three values",
        }
        for text, fragment in cases.items():
            parsed = self._parse(text)
            self.assertTrue(
                any(fragment in d["message"] for d in parsed["diagnostics"]),
                (text, parsed["diagnostics"]),
            )

    def test_affine_shorthand_valid_forms_still_lower(self):
        for text in (
            "poly = linear(poly, 5, 0)",
            "poly = linear(2, 3)",
            "poly = linear(poly, 2, 3, 0.5)",
            "poly = scale(poly, 2)",
            "cf\npoly = scale(2)",
            "cf\npoly = shift(3)",
        ):
            parsed = self._parse(text)
            self.assertEqual(parsed["diagnostics"], [], text)

    def test_mod_and_abs_share_one_wire_encoding(self):
        from coeff_program_chain import compile_coeff_program_chain
        via_mod = compile_coeff_program_chain([["mod", "poly", "poly"]])
        via_abs = compile_coeff_program_chain([["abs", "poly", "poly"]])
        self.assertEqual(via_mod["fingerprint"], via_abs["fingerprint"])
        self.assertIn("vector_unary:abs", via_mod["execution_spec"])

    def test_blend_supports_poly_assignment(self):
        from coeff_program_chain import compile_coeff_program_chain
        parsed = self._parse("cf\ncf\npoly = blend(0.25)")
        self.assertEqual(parsed["diagnostics"], [])
        self.assertEqual(parsed["chain"][-1], ["_typed_set_poly"])
        compile_coeff_program_chain(parsed["chain"])

    def test_source_constants_fold_to_single_literal_push(self):
        from coeff_program_chain import compile_coeff_program_chain
        parsed = self._parse("push_scalar(1-2j)\ndrop")
        self.assertEqual(len(parsed["chain"]), 2)
        self.assertEqual(parsed["chain"][0][0], "_typed_push_scalar")
        folded = compile_coeff_program_chain(parsed["chain"])
        chip = compile_coeff_program_chain([["push_scalar", "1-2j"], ["pop"]])
        self.assertEqual(folded["fingerprint"], chip["fingerprint"])
        with_pi = self._parse("push_scalar(pi/2)\ndrop")
        self.assertEqual(len(with_pi["chain"]), 2)

    def test_source_static_division_by_zero_is_rejected(self):
        parsed = self._parse("push_scalar(1/0)\ndrop")
        self.assertTrue(any("division" in d["message"] for d in parsed["diagnostics"]))

    def test_embedded_static_division_by_zero_is_rejected(self):
        # The guard lives in the expr parser itself, so a zero denominator
        # buried inside a larger expression is caught at compile time —
        # previously only a bare 1/0 was rejected (at the source fold layer).
        from coeff_program_chain import compile_coeff_program_chain
        for bad in ("t1+1/0", "t1+1/(2-2)", "t1+0**-1", "t1+(1-1)**-2"):
            with self.assertRaisesRegex(RuntimeError, "division by zero"):
                compile_coeff_program_chain([["poke_poly", "0", bad]])
        # Wire preservation: valid divisions are untouched (validate, no fold),
        # so the dynamic expr still ships its DIV bytecode via an expr ref.
        for ok in ("t1+1/2", "t1/t2"):
            compiled = compile_coeff_program_chain([["poke_poly", "0", ok]])
            self.assertEqual(compiled["scalar_expr_count"], 1, ok)

    def test_static_complex_kind_with_real_value_accepted_for_real_args(self):
        # kind is textual (1j*1j reads "complex"); statics fold and are judged
        # by value, matching param. Folded wire is identical to spelling -1.
        from coeff_program_chain import compile_coeff_program_chain
        pre = [["push_const", "3", "1"], ["push_const", "3", "2"]]
        post = [["pop"]]
        folded = compile_coeff_program_chain(pre + [["blend", "1j*1j"]] + post)
        direct = compile_coeff_program_chain(pre + [["blend", "-1"]] + post)
        self.assertEqual(folded["tokens"], direct["tokens"])
        self.assertEqual(folded["fingerprint"], direct["fingerprint"])
        with self.assertRaisesRegex(RuntimeError, "must be real-valued"):
            compile_coeff_program_chain(pre + [["blend", "1j*2"]] + post)
        with self.assertRaisesRegex(RuntimeError, "must be real-valued"):
            compile_coeff_program_chain(pre + [["blend", "t2"]] + post)

    def test_stack_op_call_forms_and_poly_pop_call(self):
        parsed = self._parse("cf\ndup()\nemit()\nemit")
        self.assertEqual(parsed["diagnostics"], [])
        parsed = self._parse("cf\npoly = pop()")
        self.assertEqual(parsed["diagnostics"], [])
        self.assertIn(["set", "poly", "pop"], parsed["chain"])
        parsed = self._parse("cf\ndup(1)")
        self.assertTrue(any("takes no arguments" in d["message"] for d in parsed["diagnostics"]))

    def test_disallowed_source_selector_message_names_the_restriction(self):
        parsed = self._parse("cf\nroll(cf, 2)\ndrop")
        self.assertTrue(
            any("not allowed as a source" in d["message"] for d in parsed["diagnostics"]),
            parsed["diagnostics"],
        )

    def test_branch_folds_canonicalize_signed_zero(self):
        import math
        from coeff_program_chain import compile_coeff_program_chain
        angle = compile_coeff_program_chain([["push_const", "2", "angle(neg(1))"], ["emit"]])
        self.assertAlmostEqual(angle["tokens"][0]["args"][1], math.pi)
        sqrt = compile_coeff_program_chain([["push_const", "2", "sqrt(neg(1))"], ["emit"]])
        self.assertAlmostEqual(sqrt["tokens"][0]["args_im"][1], 1.0)

    def test_round_two_arg_native_form_is_multiplier_and_andy(self):
        # Pinned decision: source/native round(src, multiplier, andy) treats
        # the second value as andy. Explicit legacy(...) rows stay the old
        # packed real/imag compatibility form.
        from coeff_program_chain import compile_coeff_program_chain
        compiled = compile_coeff_program_chain([["_native_transform", "round", "poly", "poly", "2", "3"]])
        token = compiled["tokens"][0]
        self.assertEqual(token["args"], [2.0, 0.0])
        self.assertEqual(token["andy"], 3.0)

    def test_dead_entry_points_removed(self):
        import coeff_program_chain as chain_mod
        import coeff_program_source as source_mod
        for missing in ("compile_coeff_program_diagnostics", "coeff_program_chain_id", "validate_legacy_registry"):
            self.assertFalse(hasattr(chain_mod, missing), missing)
        self.assertFalse(hasattr(source_mod, "compile_coeff_program_source_diagnostics"))
        self.assertFalse(hasattr(source_mod, "_LEGACY_UNARY_NAMES"))
        compiled = chain_mod.compile_coeff_program_chain([["push_range", "4"], ["emit"]])
        self.assertNotIn("execution_tokens", compiled)
