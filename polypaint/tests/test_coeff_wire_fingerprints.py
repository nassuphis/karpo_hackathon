import hashlib
import json
import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _execution_sha1(compiled):
    return hashlib.sha1(compiled["execution_spec"].encode("utf-8")).hexdigest()


def _wire_corpus():
    path = os.path.join(
        os.path.dirname(__file__),
        "fixtures",
        "coeff-wire-corpus",
        "m3_wire_corpus.json",
    )
    with open(path, "r", encoding="utf-8") as fh:
        payload = json.load(fh)
    if int(payload.get("version") or 0) != 1:
        raise AssertionError("unsupported coeff wire corpus version")
    return payload


class TestCoeffWireFingerprints(unittest.TestCase):
    def test_source_forms_keep_golden_fingerprints(self):
        from coeff_program_source import compile_coeff_program_source

        for case in _wire_corpus()["source_cases"]:
            with self.subTest(name=case["name"]):
                compiled = compile_coeff_program_source(case["source"])
                self.assertEqual(compiled["fingerprint"], case["fingerprint"])
                self.assertEqual(_execution_sha1(compiled), case["execution_sha1"])

    def test_legacy_chain_packed_forms_keep_golden_fingerprints(self):
        from coeff_program_chain import compile_coeff_program_chain

        for case in _wire_corpus()["chain_cases"]:
            with self.subTest(name=case["name"]):
                compiled = compile_coeff_program_chain(case["chain"])
                self.assertEqual(compiled["fingerprint"], case["fingerprint"])
                self.assertEqual(_execution_sha1(compiled), case["execution_sha1"])


if __name__ == "__main__":
    unittest.main()
