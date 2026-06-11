"""Blank source_text must not shadow a non-empty saved chain.

The UI omits source_text when blank, and the compute request paths already
apply non-empty-source-wins precedence to run params. But imported or
hand-edited saved programs can carry both keys ({"chain": [...],
"source_text": ""}); the saved-program fetch and the three macro resolvers
used to parse the blank source and silently turn such a program into an
empty chain. All readers now share coeff_source_text_from_payload().
"""
import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from coeff_program_source import coeff_source_text_from_payload


REV_CHAIN = [["legacy", "rev", "poly", "poly"]]
STALE_PAYLOAD = {"chain": REV_CHAIN, "source_text": ""}


class TestSourceTextPrecedenceHelper(unittest.TestCase):
    def test_non_blank_source_wins(self):
        self.assertEqual(
            coeff_source_text_from_payload({"chain": REV_CHAIN, "source_text": "poly = cf"}),
            "poly = cf",
        )

    def test_blank_source_defers_to_chain(self):
        self.assertIsNone(coeff_source_text_from_payload(dict(STALE_PAYLOAD)))
        self.assertIsNone(coeff_source_text_from_payload({"chain": REV_CHAIN, "source_text": "   "}))

    def test_blank_source_without_chain_stays_source(self):
        # Nothing to fall back to: keep the historical parse-empty behavior.
        self.assertEqual(coeff_source_text_from_payload({"source_text": ""}), "")

    def test_absent_key_or_non_dict_means_chain(self):
        self.assertIsNone(coeff_source_text_from_payload({"chain": REV_CHAIN}))
        self.assertIsNone(coeff_source_text_from_payload(None))
        self.assertIsNone(coeff_source_text_from_payload("source_text"))


def _fake_s3_returning(payload):
    fake = MagicMock()
    body = json.dumps(payload).encode("utf-8")
    fake.get_object.return_value = {"Body": MagicMock(read=lambda: body)}
    return fake


class TestMacroResolversCompileChainOnBlankSource(unittest.TestCase):
    def test_compute_plan_macro_resolver(self):
        import handler_compute_plan as mod

        with patch.object(mod, "s3", _fake_s3_returning(STALE_PAYLOAD)):
            chain = mod._coeff_program_macro_resolver()("prog")
        self.assertEqual(chain, REV_CHAIN)

    def test_coeffgen_macro_resolver(self):
        import handler_coeffgen as mod

        with patch.object(mod, "s3", _fake_s3_returning(STALE_PAYLOAD)):
            chain = mod._coeff_program_macro_resolver()("prog")
        self.assertEqual(chain, REV_CHAIN)

    def test_compute_preview_macro_resolver(self):
        import handler_compute_preview as mod

        fake = _fake_s3_returning(STALE_PAYLOAD)
        with patch.object(mod, "_s3_client", lambda: fake):
            chain = mod._coeff_program_macro_resolver()("prog")
        self.assertEqual(chain, REV_CHAIN)

    def test_non_blank_source_still_wins_over_chain(self):
        import handler_compute_plan as mod

        payload = {"chain": REV_CHAIN, "source_text": "poly = cf"}
        with patch.object(mod, "s3", _fake_s3_returning(payload)):
            chain = mod._coeff_program_macro_resolver()("prog")
        self.assertEqual(chain, [["set", "poly", "cf"]])


if __name__ == "__main__":
    unittest.main()
