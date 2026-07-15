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
        from coeff_program_chain import expand_monic_roots
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        payload = generator.build_payload()
        compiled = compile_coeff_program_source(payload["source_text"])
        source_roots, target_roots = generator._layout_roots(generator.RJAIL3)
        expected = [
            expand_monic_roots(source_roots),
            expand_monic_roots(target_roots),
        ]
        for constant, wanted, roots in zip(
            compiled["vector_constants"], expected, (source_roots, target_roots)
        ):
            got = [
                complex(constant["values"][i], constant["values"][i + 1])
                for i in range(0, len(constant["values"]), 2)
            ]
            self.assertEqual(got, wanted)
            # The layout is recoverable FROM the pool: solving the base
            # polynomial returns the 33 grid coordinates. Degree-33 recovery
            # of this family is ill-conditioned (max|c| ~ 1e22; measured
            # worst nearest-neighbor error 6.3e-2), so match UNORDERED with
            # a tolerance that still identifies each unit-spaced grid cell
            # unambiguously. Lexicographic sort mispairs nearly-tied reals.
            import numpy as np

            recovered = np.roots(np.array(got))
            layout = np.array(roots, dtype=np.complex128)
            distances = np.abs(recovered[:, None] - layout[None, :])
            self.assertLess(float(distances.min(axis=1).max()), 0.1)
            # ...and every grid cell is hit
            self.assertLess(float(distances.min(axis=0).max()), 0.1)

    def test_roots_literal_matches_vector_literal_spelling_and_fingerprint(self):
        """The regenerated roots_literal document must compile to the SAME
        pool and fingerprint as the earlier vector_literal spelling — the
        source form changed, the program identity must not."""
        from coeff_program_chain import expand_monic_roots
        from coeff_program_source import compile_coeff_program_source

        generator = _load_generator()
        roots_doc = generator.build_payload()
        compiled_roots = compile_coeff_program_source(roots_doc["source_text"])
        self.assertIn("roots_literal(", roots_doc["source_text"])

        source_roots, target_roots = generator._layout_roots(generator.RJAIL3)
        lines = roots_doc["source_text"].splitlines()
        tail = "\n".join(line for line in lines if not line.startswith((" ", ")", "roots_literal(")))
        vector_spelling = "\n".join(
            [
                "vector_literal(%s)" % ", ".join(
                    generator._complex(value)
                    for value in expand_monic_roots(source_roots)
                ),
                "vector_literal(%s)" % ", ".join(
                    generator._complex(value)
                    for value in expand_monic_roots(target_roots)
                ),
                tail,
            ]
        )
        compiled_vectors = compile_coeff_program_source(vector_spelling)
        self.assertEqual(
            compiled_roots["fingerprint"], compiled_vectors["fingerprint"]
        )
        self.assertEqual(
            compiled_roots["vector_constants"], compiled_vectors["vector_constants"]
        )

    def test_roots_literal_contract(self):
        from coeff_program_chain import MAX_VECTOR_LEN
        from coeff_program_source import compile_coeff_program_source

        # expansion: roots 1,2 -> z^2 - 3z + 2
        compiled = compile_coeff_program_source("poly = roots_literal(1, 2)\nemit")
        self.assertEqual(
            compiled["vector_constants"],
            [{"length": 3, "values": [1.0, 0.0, -3.0, 0.0, 2.0, 0.0]}],
        )

        # byte-identical results share ONE pool entry across BOTH forms
        merged = compile_coeff_program_source(
            "roots_literal(1, 2)\n"
            "vector_literal(1, -3, 2)\n"
            "poly = blend(0.5)\n"
            "emit"
        )
        self.assertEqual(merged["vector_constant_count"], 1)

        # static-only roots; root-count cap leaves room for the +1 coefficient
        with self.assertRaisesRegex(Exception, "must be a static expression"):
            compile_coeff_program_source("poly = roots_literal(1, t1)")
        too_many = ",".join("1" for _ in range(MAX_VECTOR_LEN))
        with self.assertRaisesRegex(Exception, f"max is {MAX_VECTOR_LEN - 1}"):
            compile_coeff_program_source(f"poly = roots_literal({too_many})")

        # chain->source round trip keeps the roots spelling
        from coeff_program_source import coeff_source_text_from_chain

        regenerated = coeff_source_text_from_chain(compiled["source_chain"])
        self.assertIn("roots_literal(", regenerated)
        self.assertEqual(
            compile_coeff_program_source(regenerated)["fingerprint"],
            compiled["fingerprint"],
        )

    def test_old_program_spec_does_not_gain_an_empty_constant_pool(self):
        from coeff_program_chain import compile_coeff_program_chain

        compiled = compile_coeff_program_chain([["push", "cf"], ["emit"]])
        self.assertNotIn("vectors=", compiled["execution_spec"])
        self.assertEqual(compiled["vector_constants"], [])

    def test_root_pattern_literals_expand_into_the_pool(self):
        """Standard patterns are compile-time sugar over roots_literal:
        same pool, same opcode, byte-identical results deduplicate."""
        import numpy as np
        from coeff_program_source import compile_coeff_program_source

        # ring(4, 1, 0) is EXACTLY z^4 - 1 (cardinal angles are exact-cased
        # by integer index arithmetic, never value proximity)
        ring = compile_coeff_program_source("poly = roots_ring_literal(4, 1, 0)\nemit")
        self.assertEqual(
            ring["vector_constants"],
            [{"length": 5, "values": [1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0]}],
        )

        # chess(3, 2, 0): dark cells of a 3x3 board, step 1 — corners plus
        # center — deduplicates against the equivalent handwritten literal
        merged = compile_coeff_program_source(
            "roots_chess_literal(3, 2, 0)\n"
            "vector_literal(1, 0, 0)\n"  # keep two stack entries for blend
            "poly = blend(0.5)\nemit"
        )
        handwritten = compile_coeff_program_source(
            "poly = roots_literal(-1+1i, 1+1i, 0, -1-1i, 1-1i)\nemit"
        )
        self.assertEqual(
            merged["vector_constants"][0], handwritten["vector_constants"][0]
        )

        # grid(2, 1, 1+1i): four corners around the offset center
        grid = compile_coeff_program_source("poly = roots_grid_literal(2, 1, 1+1i)\nemit")
        got = grid["vector_constants"][0]
        base = [complex(got["values"][i], got["values"][i + 1])
                for i in range(0, len(got["values"]), 2)]
        recovered = np.roots(np.array(base))
        expected = np.array([0.5 + 1.5j, 1.5 + 1.5j, 0.5 + 0.5j, 1.5 + 0.5j])
        distances = np.abs(recovered[:, None] - expected[None, :])
        self.assertLess(float(distances.min(axis=1).max()), 1e-9)

        # validation: caps, positivity, integrality, staticness
        for bad, message in (
            ("poly = roots_chess_literal(23, 1, 0)", "must be in \\[1, 22\\]"),
            ("poly = roots_grid_literal(16, 1, 0)", "must be in \\[1, 15\\]"),
            ("poly = roots_chess_literal(5, 0, 0)", "positive real"),
            ("poly = roots_ring_literal(4, 0, 0)", "must be nonzero"),
            ("poly = roots_grid_literal(2.5, 1, 0)", "must be an integer"),
            ("poly = roots_chess_literal(5, 1, t1)", "must be a static expression"),
            ("poly = roots_ring_literal(7, 1)", "exactly \\(d, w, o\\)"),
        ):
            with self.assertRaisesRegex(Exception, message):
                compile_coeff_program_source(bad + "\nemit")

    def test_root_pattern_literals_round_trip_with_house_style_args(self):
        import warnings

        from coeff_program_source import (
            coeff_source_text_from_chain,
            compile_coeff_program_source,
        )

        for src, expected_line in (
            ("poly = roots_chess_literal(5, 1, 1+1i)\nemit",
             "poly = roots_chess_literal(5.0, 1.0, 1.0+1.0i)"),
            ("poly = roots_ring_literal(7, 0.5i, -1)\nemit",
             "poly = roots_ring_literal(7.0, 0.5i, -1.0)"),
        ):
            compiled = compile_coeff_program_source(src)
            with warnings.catch_warnings(record=True) as caught:
                warnings.simplefilter("always")
                regenerated = coeff_source_text_from_chain(compiled["source_chain"])
            self.assertEqual([str(w.message) for w in caught], [])
            self.assertEqual(regenerated.splitlines()[0], expected_line)
            self.assertEqual(
                compile_coeff_program_source(regenerated)["fingerprint"],
                compiled["fingerprint"],
            )

    def test_canonical_program_round_trips_chain_to_source(self):
        """Review finding 1: the CANONICAL saved program (not a reduced
        spelling) must decompile to fingerprint-preserving source with no
        readable-candidate fallback. It originally failed twice: imaginary
        statics rendered as '0.0+1.0j' (re-lowers as an addition) and
        pop-first-arg vectors were collapsed into nested calls (drops the
        _typed_push_vector('pop') token)."""
        import warnings

        from coeff_program_source import (
            coeff_source_text_from_chain,
            compile_coeff_program_source,
        )

        generator = _load_generator()
        cases = {
            "canonical": generator.build_source_text(),
            "pop-first-arg general form": (
                "vector_literal(1, -3, 2)\n"
                "poly = translate_roots(pop, 0.1*exp(pi2i*t1))\n"
                "poly\n"
                "vector_literal(1, 2, 0)\n"
                "poly = translate_roots(pop, 0.2*exp(pi2i*(t1+0.25)))\n"
                "poly\n"
                "poly = blend(bimodal(t2, 0.7))\n"
                "emit"
            ),
        }
        for label, source in cases.items():
            compiled = compile_coeff_program_source(source)
            with warnings.catch_warnings(record=True) as caught:
                warnings.simplefilter("always")
                regenerated = coeff_source_text_from_chain(compiled["source_chain"])
            self.assertEqual(
                [str(w.message) for w in caught], [], f"{label}: fallback warned"
            )
            self.assertEqual(
                compile_coeff_program_source(regenerated)["fingerprint"],
                compiled["fingerprint"],
                f"{label}: chain->source round trip changed the fingerprint",
            )


if __name__ == "__main__":
    unittest.main()
