import json
import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


FIXTURE_DIR = os.path.join(os.path.dirname(__file__), "fixtures", "coeff-program-corpus")


def _load_json(rel_path):
    with open(os.path.join(FIXTURE_DIR, rel_path), "r", encoding="utf-8") as fh:
        return json.load(fh)


def _macro_resolver(macros):
    macros = dict(macros or {})

    def resolve(name):
        key = str(name or "").strip()
        if key not in macros:
            raise RuntimeError(f"test macro not found: {key}")
        return macros[key]

    return resolve


def _selected_compile_fields(compiled):
    return {
        "source_chain": compiled["source_chain"],
        "expanded_chain": compiled["expanded_chain"],
        "tokens": compiled["tokens"],
        "scalar_exprs": compiled["scalar_exprs"],
        "execution_spec": compiled["execution_spec"],
        "legacy_coeff_transforms": compiled["legacy_coeff_transforms"],
        "fingerprint": compiled["fingerprint"],
    }


def _diagnostic_shape(diag):
    return {
        "line": int(diag.get("line") or 0),
        "column": int(diag.get("column") or 0),
        "level": str(diag.get("level") or ""),
        "code": str(diag.get("code") or "source_error"),
    }


class TestCoeffSourceEquivalence(unittest.TestCase):
    def _corpus(self):
        return _load_json("corpus.json")

    def test_current_parser_matches_legacy_shell_oracle_on_valid_corpus(self):
        import coeff_program_source as current
        import coeff_program_source_legacy as legacy

        for item in self._corpus()["valid"]:
            with self.subTest(item=item["id"]):
                payload = _load_json(item["path"])
                source_text = payload["source_text"]
                resolver = _macro_resolver(payload.get("macros"))

                old_parsed = legacy.parse_coeff_program_source(source_text, strict=True)
                new_parsed = current.parse_coeff_program_source(source_text, strict=True)
                self.assertEqual(old_parsed["diagnostics"], [])
                self.assertEqual(new_parsed["diagnostics"], [])
                self.assertEqual(new_parsed["chain"], old_parsed["chain"])
                self.assertEqual(new_parsed["statement_count"], old_parsed["statement_count"])

                old_compiled = legacy.compile_coeff_program_source(
                    source_text,
                    macro_resolver=resolver,
                    strict=True,
                )
                new_compiled = current.compile_coeff_program_source(
                    source_text,
                    macro_resolver=resolver,
                    strict=True,
                )
                self.assertEqual(_selected_compile_fields(new_compiled), _selected_compile_fields(old_compiled))

                canonical = current.coeff_source_text_from_chain(new_parsed["chain"])
                self.assertNotIn("_typed_", canonical)
                reparsed = current.compile_coeff_program_source(
                    canonical,
                    macro_resolver=resolver,
                    strict=True,
                )
                self.assertEqual(reparsed["execution_spec"], new_compiled["execution_spec"])
                self.assertEqual(reparsed["fingerprint"], new_compiled["fingerprint"])

    def test_canonical_source_regeneration_for_representable_chain(self):
        import coeff_program_source as current

        chain = [["legacy", "rev", "poly", "poly"], ["emit"]]
        source = current.coeff_source_text_from_chain(chain)
        self.assertNotIn("_typed_", source)
        compiled = current.compile_coeff_program_source(source, strict=True)
        self.assertEqual(compiled["execution_spec"], current.compile_coeff_program_source("poly = rev(poly)\nemit")["execution_spec"])

    def test_canonical_source_regeneration_for_typed_internal_chain(self):
        import coeff_program_source as current

        chain = [
            ["push_range", "1", "poly_len+1"],
            ["_typed_set_poly"],
            ["_typed_push_vector", "poly"],
            ["_typed_push_scalar", "0.25+0.0j"],
            ["_typed_binary", "multiply"],
            ["_typed_push_scalar", "1.0+0.0j"],
            ["_typed_binary", "add"],
            ["_typed_set_poly"],
            ["_typed_push_scalar", "poly_len"],
            ["_typed_push_scalar", "1.0+0.0j"],
            ["_typed_binary", "subtract"],
            ["_typed_push_scalar", "p1"],
            ["_typed_push_scalar", "poly_len"],
            ["_typed_push_scalar", "2.0+0.0j"],
            ["_typed_binary", "subtract"],
            ["_typed_push_vector", "poly"],
            ["swap"],
            ["_typed_get_scalar"],
            ["_typed_binary", "add"],
            ["_typed_poke_poly"],
            ["emit"],
        ]
        source = current.coeff_source_text_from_chain(chain)
        self.assertNotIn("_typed_", source)
        self.assertIn("poly[(poly_len-1.0)]", source)
        compiled = current.compile_coeff_program_source(source, strict=True)
        direct = current.compile_coeff_program_source(
            """
            poly = arange(1, poly_len+1)
            poly = add(multiply(poly, 0.25), 1.0)
            poly[(poly_len-1.0)] = (p1+poly[(poly_len-2.0)])
            emit
            """,
            strict=True,
        )
        self.assertEqual(compiled["execution_spec"], direct["execution_spec"])
        self.assertEqual(compiled["fingerprint"], direct["fingerprint"])

    def test_canonical_source_regeneration_consumes_native_stack_args(self):
        import coeff_program_source as current

        chain = [
            ["_typed_push_scalar", "0.0+1.0j"],
            ["_typed_push_scalar", "p2"],
            ["_native_transform_stack_args", "exp", "poly", "poly", "2", "0.25"],
            ["emit"],
        ]
        source = current.coeff_source_text_from_chain(chain)
        self.assertIn("exp_affine(poly, 0.0+1.0j, p2, 0.25)", source)
        compiled = current.compile_coeff_program_source(source, strict=True)
        direct = current.compile_coeff_program_source(
            "poly = exp_affine(poly, 0.0+1.0j, p2, 0.25)\nemit",
            strict=True,
        )
        self.assertEqual(compiled["fingerprint"], direct["fingerprint"])

    def test_current_parser_matches_legacy_shell_oracle_diagnostic_shape(self):
        import coeff_program_source as current
        import coeff_program_source_legacy as legacy

        for item in self._corpus()["invalid"]:
            with self.subTest(item=item["id"]):
                old_parsed = legacy.parse_coeff_program_source(item["source_text"], strict=False)
                new_parsed = current.parse_coeff_program_source(item["source_text"], strict=False)
                self.assertEqual(
                    [_diagnostic_shape(d) for d in new_parsed["diagnostics"]],
                    [_diagnostic_shape(d) for d in old_parsed["diagnostics"]],
                )
                with self.assertRaises(RuntimeError) as new_ctx:
                    current.parse_coeff_program_source(item["source_text"], strict=True)
                self.assertTrue(getattr(new_ctx.exception, "diagnostics", None))


if __name__ == "__main__":
    unittest.main()
