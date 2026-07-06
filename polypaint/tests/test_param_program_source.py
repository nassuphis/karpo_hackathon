import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestParamProgramSource(unittest.TestCase):
    def test_assignments_compile_equivalent_to_const_emit_chain(self):
        from param_program_chain import compile_param_program_chain
        from param_program_source import parse_param_program_source

        source = """
        p1 = t1 + t2
        p2 = t1 - t2
        """
        parsed = parse_param_program_source(source)
        expected_chain = [
            ["const", "t1+t2"],
            ["emit", "p1"],
            ["const", "t1-t2"],
            ["emit", "p2"],
        ]
        self.assertEqual(parsed["chain"], expected_chain)
        self.assertEqual(
            compile_param_program_chain(parsed["chain"])["execution_spec"],
            compile_param_program_chain(expected_chain)["execution_spec"],
        )

    def test_stack_legacy_and_targetable_unary_forms(self):
        from param_program_source import parse_param_program_source

        parsed = parse_param_program_source(
            """
            push(t1); push(t2); add; emit_p1
            square(p2)
            legacy(rtheta, both, both, p1+p2)
            """
        )
        self.assertEqual(parsed["chain"][0:4], [["push", "t1"], ["push", "t2"], ["add"], ["emit", "p1"]])
        self.assertEqual(parsed["chain"][4], ["square", "p2"])
        self.assertEqual(parsed["chain"][5], ["legacy", "rtheta", "both", "both", "p1+p2"])

    def test_legacy_forms_validate_name_selector_and_arity_at_parse_time(self):
        from param_program_source import parse_param_program_source

        cases = [
            ("legacy(nope, both, both)", "unknown_legacy_transform"),
            ("legacy(unit_circle, none, both)", "bad_selector"),
            ("legacy(square, pop2, push2)", "bad_selector"),
            ("crd(5, 9)", "bad_arity"),
        ]
        for source, code in cases:
            with self.subTest(source=source):
                parsed = parse_param_program_source(source, strict=False)
                self.assertEqual(parsed["chain"], [])
                self.assertEqual(parsed["diagnostics"][0]["code"], code)

    def test_noncanonical_emit_is_rejected_with_diagnostics(self):
        from param_program_source import parse_param_program_source

        parsed = parse_param_program_source("emit(p1)", strict=False)
        self.assertEqual(parsed["chain"], [])
        self.assertEqual(parsed["diagnostics"][0]["code"], "noncanonical_emit")
        self.assertIn("emit_p1", parsed["diagnostics"][0]["message"])

    def test_profile_backed_rejected_forms_keep_existing_diagnostics(self):
        from param_program_source import parse_param_program_source

        cases = [
            ("push(both)", "bad_selector"),
            ("emit(p1)", "noncanonical_emit"),
            ("t1 = p1", "read_only_symbol"),
            # An unreserved LHS ("missing = p1") is now a local alias
            # definition; reserved names still route to the old diagnostic.
            ("sin = p1", "unknown_symbol"),
            ("p1[0] = p2", "unknown_symbol"),
            ("p1 =", "empty_expression"),
            ("", "empty_source"),
        ]
        for source, code in cases:
            with self.subTest(source=source):
                parsed = parse_param_program_source(source, strict=False)
                self.assertEqual(parsed["chain"], [])
                self.assertEqual(parsed["diagnostics"][0]["code"], code)

    def test_indexed_output_assignment_gets_param_specific_diagnostic(self):
        from param_program_source import parse_param_program_source

        parsed = parse_param_program_source("p1[0] = p2", strict=False)
        self.assertEqual(parsed["chain"], [])
        self.assertEqual(parsed["diagnostics"][0]["code"], "unknown_symbol")
        self.assertIn("not valid Param Program source", parsed["diagnostics"][0]["message"])

    def test_strict_errors_carry_structured_diagnostics(self):
        from param_program_source import ParamProgramSourceCompileError, parse_param_program_source

        with self.assertRaises(ParamProgramSourceCompileError) as caught:
            parse_param_program_source("emit(p1)")
        self.assertIn("invalid param_program_source_text", str(caught.exception))
        self.assertEqual(caught.exception.diagnostics[0]["code"], "noncanonical_emit")
        self.assertEqual(caught.exception.diagnostics[0]["line"], 1)

    def test_source_text_from_chain_round_trips(self):
        from param_program_chain import compile_param_program_chain
        from param_program_source import param_source_text_from_chain, parse_param_program_source

        chain = [
            ["const", "t1+t2"],
            ["emit", "p1"],
            ["push", "t1"],
            ["push", "t2"],
            ["subtract"],
            ["emit", "p2"],
            ["legacy", "rtheta", "both", "both", "real(p1)"],
        ]
        source = param_source_text_from_chain(chain)
        reparsed = parse_param_program_source(source)
        self.assertEqual(
            compile_param_program_chain(reparsed["chain"])["fingerprint"],
            compile_param_program_chain(chain)["fingerprint"],
        )

    def test_source_text_from_chain_does_not_emit_unparseable_two_arg_const(self):
        from param_program_source import param_source_text_from_chain, parse_param_program_source

        chain = [["const", "1", "2"], ["emit", "p1"]]
        source = param_source_text_from_chain(chain)
        parsed = parse_param_program_source(source)

        self.assertTrue(parsed["chain"])

    def test_payload_source_precedence(self):
        from param_program_source import param_source_text_from_payload

        self.assertEqual(param_source_text_from_payload({"source_text": "p1 = t1", "chain": [["push"]]}), "p1 = t1")
        self.assertEqual(param_source_text_from_payload({"source_text": "", "chain": []}), "")
        self.assertIsNone(param_source_text_from_payload({"chain": [["push"]]}))

    def test_compile_source_exposes_lowered_chain_for_storage_contract(self):
        from param_program_source import compile_param_program_source

        compiled = compile_param_program_source("p1 = t1 + t2", strict=False)
        self.assertEqual(compiled["chain"], [["const", "t1+t2"], ["emit", "p1"]])
        self.assertEqual(compiled["source_chain"], compiled["chain"])
        self.assertTrue(compiled["fingerprint"])

    def test_variable_arity_source_accepts_every_chain_accepted_count(self):
        # Source-parse arity must come from registry compat, never a local
        # copy: a shadowing hardcode once rejected legacy(inv_t_plus_2, ...,
        # 1, 2, 3) at parse while the chain compiler accepted it, so a saved
        # 3-arg chain serialized to source that would not reparse.
        from param_program_chain import compile_param_program_chain, legacy_registry
        from param_program_source import parse_param_program_source

        compat_counts = legacy_registry()["compat"]["variable_arg_counts"]
        self.assertIn("inv_t_plus_2", compat_counts)
        for name, counts in compat_counts.items():
            for count in sorted(counts):
                args = ", ".join(["1"] * count)
                source = f"legacy({name}, both, both{', ' + args if args else ''})"
                parsed = parse_param_program_source(source, strict=False)
                errors = [d for d in parsed["diagnostics"] if d.get("level") == "error"]
                self.assertEqual(errors, [], f"{name} count={count}: {errors}")
                compiled = compile_param_program_chain(parsed["chain"])
                self.assertTrue(compiled["fingerprint"], f"{name} count={count}")


if __name__ == "__main__":
    unittest.main()


class TestParamProgramRegistersSource(unittest.TestCase):
    def test_r9_and_higher_stay_legal_write_once_locals(self):
        # r1..r8 are registers; r9+ were legal locals before registers
        # shipped and must remain so (reserved pattern is ^r[1-8]$ only).
        from param_program_chain import compile_param_program_chain
        from param_program_source import parse_param_program_source

        parsed = parse_param_program_source("r9 = t1 + 1\np1 = r9 * r9\np2 = t2")
        errors = [d for d in parsed["diagnostics"] if d.get("level") == "error"]
        self.assertEqual(errors, [])
        # local substitution: no register ops in the compiled tokens
        from param_program_chain import PARAM_OP_PUSH_REG, PARAM_OP_STORE_REG
        compiled = compile_param_program_chain(parsed["chain"])
        ops = {t["op"] for t in compiled["tokens"]}
        self.assertNotIn(PARAM_OP_PUSH_REG, ops)
        self.assertNotIn(PARAM_OP_STORE_REG, ops)

    def test_unary_target_rejects_scratch_register_cleanly(self):
        from param_program_source import parse_param_program_source

        parsed = parse_param_program_source("r1 = t1\nsquare(r1)\np1 = r1", strict=False)
        errors = [d for d in parsed["diagnostics"] if d.get("level") == "error"]
        self.assertTrue(errors, "square(r1) must fail at the source layer")
        self.assertEqual(errors[0].get("code"), "bad_unary_target")
        self.assertIn("p1 or p2", errors[0].get("message", ""))

    def test_register_program_source_round_trips_through_converter(self):
        from param_program_chain import compile_param_program_chain
        from param_program_source import (
            param_source_text_from_chain,
            parse_param_program_source,
        )

        source = "r1 = t1 + 1\nr1 = r1 * r1\np1 = r1\np2 = r3 + t2"
        chain = parse_param_program_source(source, strict=True)["chain"]
        fingerprint = compile_param_program_chain(chain)["fingerprint"]
        text = param_source_text_from_chain(chain)
        for line in ("r1 = t1+1", "r1 = r1*r1"):
            self.assertIn(line, text)
        reparsed = parse_param_program_source(text, strict=True)["chain"]
        self.assertEqual(
            compile_param_program_chain(reparsed)["fingerprint"], fingerprint,
            "converter output must recompile to the same fingerprint",
        )
