import hashlib
import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _execution_sha1(compiled):
    return hashlib.sha1(compiled["execution_spec"].encode("utf-8")).hexdigest()


class TestCoeffWireFingerprints(unittest.TestCase):
    def test_source_forms_keep_golden_fingerprints(self):
        from coeff_program_source import compile_coeff_program_source

        cases = {
            "exp_affine_complex_andy": (
                "poly = exp_affine(poly, 1+2j, 3+4j, 0.5)\nemit",
                "5446337869df389dd516dc6dcf10a023a111f51d",
                "8a78c38e22fbad788f3f40163b9a20be4ddcb3e4",
            ),
            "round_complex_andy": (
                "poly = round(poly, 1+2j, 0.25)\nemit",
                "0609cfe76b881b65738132e4a971eea937f164cb",
                "24acb11075ddbbf3f894418049e6a84a5179919d",
            ),
            "affine_imaginary": (
                "affine(poly, poly, 1+2j, 3-4j)\nemit",
                "9892cccfad4efcd38f4e182188da6d46579f6b94",
                "3ef8f27f6797c398ced44b47e48ca75f990e9423",
            ),
            "littlewood_andy": (
                "poly = littlewood(10, -10j, 0.25)\nemit",
                "278cba39687593538cff97b384675753fc69642b",
                "37cef03d6374dd96797012ed885ffb3cded5fbf2",
            ),
        }
        for name, (source, fingerprint, exec_sha1) in cases.items():
            with self.subTest(name=name):
                compiled = compile_coeff_program_source(source)
                self.assertEqual(compiled["fingerprint"], fingerprint)
                self.assertEqual(_execution_sha1(compiled), exec_sha1)

    def test_legacy_chain_packed_forms_keep_golden_fingerprints(self):
        from coeff_program_chain import compile_coeff_program_chain

        cases = {
            "linear_old_four_real": (
                [["legacy", "linear", "poly", "poly", "1", "2", "3", "4"]],
                "1101e3a411a8cede66c44db453236cc23a40a738",
                "98f050101a92e6e17e66c158a7a559a2c6fca696",
            ),
            "pow_old_four_real": (
                [["legacy", "pow", "poly", "poly", "1", "2", "3", "4"]],
                "2d2f8ca0c37aa68d3a4e579a9a05043e66c89c5a",
                "7ce6838bb27a4725f20ed409a4aec9a8793c9e68",
            ),
            "exp_flat": (
                [["legacy", "exp", "poly", "poly", "1", "2", "0.5"]],
                "a158050ea65185841f20e5d0dfcf816089a82f24",
                "ed697ec25660511b3cf48cdcd0c60f87f4b35fa0",
            ),
            "round_flat": (
                [["legacy", "round", "poly", "poly", "1", "2", "0.25"]],
                "da7565cc27b2cfad41f7a8334af53979caee1373",
                "66da7f18e11af8ed2437203bcca1321b492f8af6",
            ),
        }
        for name, (chain, fingerprint, exec_sha1) in cases.items():
            with self.subTest(name=name):
                compiled = compile_coeff_program_chain(chain)
                self.assertEqual(compiled["fingerprint"], fingerprint)
                self.assertEqual(_execution_sha1(compiled), exec_sha1)


if __name__ == "__main__":
    unittest.main()
