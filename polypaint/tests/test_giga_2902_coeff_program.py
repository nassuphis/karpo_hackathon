import importlib.util
import json
import os
import sys
import unittest


ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
sys.path.insert(0, LAMBDA_DIR)


def _load_generator():
    path = os.path.join(ROOT, "scripts", "gen_giga_2902_coeff_program.py")
    spec = importlib.util.spec_from_file_location(
        "gen_giga_2902_coeff_program", path
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestGiga2902CoeffProgram(unittest.TestCase):
    def test_vector_literal_deduplicates_and_round_trips(self):
        from coeff_program_source import (
            coeff_source_text_from_chain,
            compile_coeff_program_source,
        )

        source = (
            "vector_literal(1, -3, 2)\n"
            "vector_literal(1, -3, 2)\n"
            "poly = blend(bimodal(t2, 0.7))\n"
            "poly = translate_roots(poly, 0.5)\n"
            "emit"
        )
        compiled = compile_coeff_program_source(source)
        self.assertEqual(compiled["vector_constant_count"], 1)
        self.assertEqual(
            [token["args"][0] for token in compiled["tokens"] if token["op"] == 48],
            [0.0, 0.0],
        )
        regenerated = coeff_source_text_from_chain(compiled["source_chain"])
        self.assertEqual(
            compile_coeff_program_source(regenerated)["fingerprint"],
            compiled["fingerprint"],
        )

    def test_vector_literal_requires_static_finite_values(self):
        from coeff_program_source import compile_coeff_program_source

        with self.assertRaisesRegex(Exception, "must be a static expression"):
            compile_coeff_program_source("poly = vector_literal(1, t1)")
        with self.assertRaisesRegex(Exception, "division by zero"):
            compile_coeff_program_source("poly = vector_literal(1, 1/0)")
        with self.assertRaisesRegex(Exception, "finite"):
            compile_coeff_program_source("poly = vector_literal(1e309)")

    def test_vector_literal_pool_limits_and_signed_zero_deduplication(self):
        from coeff_program_chain import (
            MAX_VECTOR_CONSTANTS,
            MAX_VECTOR_CONSTANT_ELEMENTS,
            MAX_VECTOR_LEN,
        )
        from coeff_program_source import compile_coeff_program_source

        signed_zero = compile_coeff_program_source(
            "vector_literal(0, -(0))\n"
            "vector_literal(-0.0, 0*-1)\n"
            "poly = blend(0.5)\n"
            "emit"
        )
        self.assertEqual(signed_zero["vector_constant_count"], 1)
        self.assertEqual(
            signed_zero["vector_constants"][0]["values"],
            [0.0, 0.0, 0.0, 0.0],
        )

        too_long = ",".join("0" for _ in range(MAX_VECTOR_LEN + 1))
        with self.assertRaisesRegex(Exception, f"max is {MAX_VECTOR_LEN}"):
            compile_coeff_program_source(f"poly = vector_literal({too_long})")

        too_many = "\n".join(
            f"vector_literal({index})"
            for index in range(MAX_VECTOR_CONSTANTS + 1)
        )
        with self.assertRaisesRegex(
            Exception, f"more than {MAX_VECTOR_CONSTANTS} vector constants"
        ):
            compile_coeff_program_source(too_many)

        vectors = []
        for index in range(MAX_VECTOR_CONSTANT_ELEMENTS // MAX_VECTOR_LEN + 1):
            values = [str(index + 1), *(["0"] * (MAX_VECTOR_LEN - 1))]
            vectors.append(f"vector_literal({','.join(values)})")
        with self.assertRaisesRegex(
            Exception, f"max is {MAX_VECTOR_CONSTANT_ELEMENTS}"
        ):
            compile_coeff_program_source("\n".join(vectors))

    def test_bimodal_static_contract(self):
        from coeff_program_chain import ExpressionParser, expr_value_if_static

        def value(text):
            return expr_value_if_static(ExpressionParser(text).parse()).real

        self.assertEqual(value("bimodal(0, 0.7)"), 0.0)
        self.assertEqual(value("bimodal(0.5, 0.7)"), 0.5)
        self.assertEqual(value("bimodal(1, 0.7)"), 1.0)
        self.assertAlmostEqual(value("bimodal(0.25, 0)"), 0.25)
        with self.assertRaisesRegex(RuntimeError, "u must be"):
            value("bimodal(1.1, 0.7)")
        with self.assertRaisesRegex(RuntimeError, "a must be"):
            value("bimodal(0.5, 1)")

    def test_generated_document_is_fresh_portable_and_compiles(self):
        from coeff_program_chain import (
            COEFF_OP_PUSH_VECTOR_CONST,
            COEFF_OP_TRANSLATE_ROOTS,
        )
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
        self.assertEqual(stored["name"], "giga_2902")
        self.assertEqual(stored["chain"], [])

        compiled = compile_coeff_program_source(stored["source_text"])
        self.assertEqual(compiled["vector_constant_count"], 2)
        self.assertEqual(compiled["vector_constant_elements"], 68)
        ops = [token["op"] for token in compiled["tokens"]]
        self.assertEqual(ops.count(COEFF_OP_PUSH_VECTOR_CONST), 2)
        self.assertEqual(ops.count(COEFF_OP_TRANSLATE_ROOTS), 1)

    def test_constant_pool_holds_unshifted_root_polynomials(self):
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        payload = generator.build_payload()
        compiled = compile_coeff_program_source(payload["source_text"])
        source_roots, target_roots = generator._layout_roots(generator.RJAIL3)
        expected = [
            generator._coefficients_from_roots(source_roots),
            generator._coefficients_from_roots(target_roots),
        ]
        for constant, wanted in zip(compiled["vector_constants"], expected):
            got = [
                complex(constant["values"][i], constant["values"][i + 1])
                for i in range(0, len(constant["values"]), 2)
            ]
            self.assertEqual(got, wanted)

    def test_old_program_spec_does_not_gain_an_empty_constant_pool(self):
        from coeff_program_chain import compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["push", "cf"], ["emit"]])
        self.assertNotIn("vectors=", compiled["execution_spec"])
        self.assertEqual(compiled["vector_constants"], [])


if __name__ == "__main__":
    unittest.main()
