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
from param_program_source import param_source_text_from_payload


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

    def test_param_non_blank_source_wins(self):
        self.assertEqual(
            param_source_text_from_payload({"chain": [["const", "1"]], "source_text": "p1 = t2"}),
            "p1 = t2",
        )

    def test_param_blank_source_defers_to_chain(self):
        self.assertIsNone(param_source_text_from_payload({"chain": [["const", "1"]], "source_text": ""}))


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

    def test_param_macro_resolver_honors_source_text_over_stale_chain(self):
        from program_compile_helpers import (
            PARAM_PROGRAMS_PREFIX,
            read_saved_program_source_chain,
        )

        payload = {
            "chain": [["const", "1"], ["emit", "p1"]],
            "source_text": "p1 = t2",
        }
        chain = read_saved_program_source_chain(
            PARAM_PROGRAMS_PREFIX,
            "param program",
            "prog",
            s3_client=_fake_s3_returning(payload),
        )

        self.assertEqual(chain, [["const", "t2"], ["emit", "p1"]])


if __name__ == "__main__":
    unittest.main()


class TestStorageAndComputeMacroResolutionAgree(unittest.TestCase):
    def test_same_payload_same_chain_both_paths(self):
        # F6 residue: storage keeps a parallel resolver implementation. Pin
        # that the same saved payload expands to the same chain through the
        # storage-side reader and the shared compute-side resolver, for the
        # exact divergence case (fresh source_text + stale chain).
        from program_compile_helpers import read_saved_program_source_chain

        payload = {
            "chain": [["push", "t1"]],
            "source_text": "p1 = t1 * t2",
        }
        compute_chain = read_saved_program_source_chain(
            "polypaint/param-programs/",
            "param program",
            "same-payload",
            s3_client=_fake_s3_returning(payload),
        )

        import handler_storage

        with patch.object(handler_storage, "s3", _fake_s3_returning(payload)):
            storage_chain = handler_storage._read_param_program_source_chain("same-payload")

        self.assertEqual(compute_chain, storage_chain)

    def test_v2_prefixed_macro_id_resolves_same_key_as_storage(self):
        # H7: storage normalizes away v2/ prefixes; the shared resolver used
        # to build the key verbatim, reading a different object.
        from program_compile_helpers import read_saved_program_source_chain

        payload = {"chain": [["push", "t1"]], "source_text": "p1 = t1"}
        fake = _fake_s3_returning(payload)
        read_saved_program_source_chain(
            "polypaint/param-programs/",
            "param program",
            "v2/foo",
            s3_client=fake,
        )
        requested_key = fake.get_object.call_args.kwargs.get("Key")
        self.assertEqual(requested_key, "polypaint/param-programs/foo.json")
