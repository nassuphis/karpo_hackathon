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

    def test_noncanonical_emit_is_rejected_with_diagnostics(self):
        from param_program_source import parse_param_program_source

        parsed = parse_param_program_source("emit(p1)", strict=False)
        self.assertEqual(parsed["chain"], [])
        self.assertEqual(parsed["diagnostics"][0]["code"], "noncanonical_emit")
        self.assertIn("emit_p1", parsed["diagnostics"][0]["message"])

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

    def test_payload_source_precedence(self):
        from param_program_source import param_source_text_from_payload

        self.assertEqual(param_source_text_from_payload({"source_text": "p1 = t1", "chain": [["push"]]}), "p1 = t1")
        self.assertEqual(param_source_text_from_payload({"source_text": "", "chain": []}), "")
        self.assertIsNone(param_source_text_from_payload({"chain": [["push"]]}))


if __name__ == "__main__":
    unittest.main()
