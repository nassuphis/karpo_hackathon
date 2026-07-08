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


if __name__ == "__main__":
    unittest.main()
