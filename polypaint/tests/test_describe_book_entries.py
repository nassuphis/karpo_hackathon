import importlib.util
import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "describe_book_entries.py"


def _load_script():
    spec = importlib.util.spec_from_file_location("describe_book_entries", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class DescribeBookEntriesTests(unittest.TestCase):
    def setUp(self):
        self.mod = _load_script()

    def test_request_payload_carries_image_and_provenance(self):
        entry = {"artifact_id": "a1"}
        report = {"summary_rows": [["Function", "const(2,0,0)"], ["Degree", "50"]]}
        req = self.mod.build_request(b"jpgbytes", entry, report)
        part_img, part_text = req["contents"][0]["parts"]
        self.assertEqual(part_img["inline_data"]["mime_type"], "image/jpeg")
        self.assertIn("Function: const(2,0,0)", part_text["text"])
        self.assertIn("Degree: 50", part_text["text"])
        self.assertEqual(req["generationConfig"]["responseMimeType"], "application/json")

    def test_provenance_falls_back_to_artifact_id(self):
        text = self.mod.provenance_lines({"artifact_id": "a9"}, {})
        self.assertIn("a9", text)

    def test_parse_response_plain_and_fenced(self):
        plain = {"candidates": [{"content": {"parts": [{"text":
            '{"title": "Ember Drift", "description": "Warm strands curl."}'}]}}]}
        self.assertEqual(self.mod.parse_response(plain),
                         ("Ember Drift", "Warm strands curl."))
        fenced = {"candidates": [{"content": {"parts": [{"text":
            '```json\n{"title": "T", "description": "D"}\n```'}]}}]}
        self.assertEqual(self.mod.parse_response(fenced), ("T", "D"))

    def test_parse_response_surfaces_api_error_and_missing_fields(self):
        with self.assertRaises(RuntimeError) as ctx:
            self.mod.parse_response({"error": {"message": "quota exceeded"}})
        self.assertIn("quota exceeded", str(ctx.exception))
        with self.assertRaises(RuntimeError):
            self.mod.parse_response({"candidates": [{"content": {"parts": [{"text":
                '{"title": "", "description": ""}'}]}}]})


class GeminiCallRetryTests(unittest.TestCase):
    def setUp(self):
        self.mod = _load_script()

    def _http_error(self, code, message):
        import io as _io
        import urllib.error
        return urllib.error.HTTPError(
            "https://x", code, "err", {"Retry-After": "0"},
            _io.BytesIO(json.dumps({"error": {"message": message}}).encode()))

    def test_retries_503_then_succeeds(self):
        from unittest.mock import patch, MagicMock
        calls = {"n": 0}

        def fake_urlopen(req, timeout=None):
            calls["n"] += 1
            if calls["n"] < 3:
                raise self._http_error(503, "The model is overloaded")
            m = MagicMock()
            m.__enter__ = lambda s2: s2
            m.__exit__ = lambda *a: False
            m.read = lambda: json.dumps({"ok": True}).encode()
            return m

        with patch.object(self.mod.urllib.request, "urlopen", side_effect=fake_urlopen), \
             patch.object(self.mod.time, "sleep", lambda *_: None):
            out = self.mod._gemini_call("https://x", b"{}", "k")
        self.assertEqual(out, {"ok": True})
        self.assertEqual(calls["n"], 3)

    def test_non_retryable_surfaces_gemini_message(self):
        from unittest.mock import patch

        def fake_urlopen(req, timeout=None):
            raise self._http_error(400, "API key not valid")

        with patch.object(self.mod.urllib.request, "urlopen", side_effect=fake_urlopen):
            with self.assertRaises(RuntimeError) as ctx:
                self.mod._gemini_call("https://x", b"{}", "k")
        self.assertIn("API key not valid", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
