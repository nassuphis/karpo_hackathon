import importlib.util
import json
import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "lambda"))
ENGINE = ROOT / "lambda" / "book_describe.py"


def _load_engine():
    spec = importlib.util.spec_from_file_location("book_describe_under_test", ENGINE)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class DescribeEngineTests(unittest.TestCase):
    def setUp(self):
        self.mod = _load_engine()

    def test_request_payload_carries_image_and_provenance(self):
        entry = {"artifact_id": "a1"}
        report = {"summary_rows": [["Function", "const(2,0,0)"], ["Degree", "50"]]}
        req = self.mod.build_request(b"jpgbytes", entry, report)
        part_img, part_text = req["contents"][0]["parts"]
        self.assertEqual(part_img["inline_data"]["mime_type"], "image/jpeg")
        self.assertIn("Function: const(2,0,0)", part_text["text"])
        self.assertIn("vibrant", part_text["text"])  # ban list is IN the prompt
        self.assertEqual(req["generationConfig"]["responseMimeType"], "application/json")

    def test_build_request_carries_angle_and_used_titles(self):
        req = self.mod.build_request(b"x", {"artifact_id": "a"}, {},
                                     angle="light — where it comes from",
                                     used_titles=("Ember Drift", "Petrol Bloom"))
        text = req["contents"][0]["parts"][1]["text"]
        self.assertIn("light — where it comes from", text)
        self.assertIn("Ember Drift, Petrol Bloom", text)

    def test_find_banned(self):
        hits = self.mod.find_banned("A Vibrant and swirling scene")
        self.assertIn("vibrant", hits)
        self.assertIn("swirling", hits)
        self.assertEqual(self.mod.find_banned("Petrol threads over bone"), [])

    def test_parse_response_plain_fenced_and_errors(self):
        plain = {"candidates": [{"content": {"parts": [{"text":
            '{"title": "Ember Drift", "description": "Warm strands curl."}'}]}}]}
        self.assertEqual(self.mod.parse_response(plain),
                         ("Ember Drift", "Warm strands curl."))
        fenced = {"candidates": [{"content": {"parts": [{"text":
            '```json\n{"title": "T", "description": "D"}\n```'}]}}]}
        self.assertEqual(self.mod.parse_response(fenced), ("T", "D"))
        with self.assertRaises(RuntimeError) as ctx:
            self.mod.parse_response({"error": {"message": "quota exceeded"}})
        self.assertIn("quota exceeded", str(ctx.exception))

    def test_gemini_call_retries_503_then_succeeds(self):
        import io as _io
        import urllib.error
        calls = {"n": 0}

        def http_error(code, message):
            return urllib.error.HTTPError(
                "https://x", code, "err", {"Retry-After": "0"},
                _io.BytesIO(json.dumps({"error": {"message": message}}).encode()))

        def fake_urlopen(req, timeout=None):
            calls["n"] += 1
            if calls["n"] < 3:
                raise http_error(503, "The model is overloaded")
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


class HandleDescribeTests(unittest.TestCase):
    def setUp(self):
        self.mod = _load_engine()
        self.phases = []
        self.p_status = patch.object(
            self.mod, "report_status",
            lambda job, task, status, error_msg=None, result_data=None:
                self.phases.append((status, (result_data or {}).get("phase"))))
        self.p_status.start()
        self.addCleanup(self.p_status.stop)
        self.p_env = patch.dict(self.mod.os.environ, {"GEMINI_API_KEY": "k"})
        self.p_env.start()
        self.addCleanup(self.p_env.stop)
        self.p_sleep = patch.object(self.mod.time, "sleep", lambda *_: None)
        self.p_sleep.start()
        self.addCleanup(self.p_sleep.stop)
        self.p_cfg = patch.object(self.mod, "_load_vision_config", lambda: {})
        self.p_cfg.start()
        self.addCleanup(self.p_cfg.stop)

    def _fake_s3(self, doc):
        fake = MagicMock()
        objects = {
            "polypaint/books/b1.json": json.dumps(doc).encode(),
            "renders/j/color/a1/preview.jpg": b"jpg1",
            "renders/j/color/a2/preview.jpg": b"jpg2",
            "polypaint/books/b1/assets/e1.provenance.json":
                json.dumps({"report": {"summary_rows": [["Degree", "5"]]}}).encode(),
        }

        def get_object(Bucket=None, Key=None):
            if Key not in objects:
                raise Exception("NoSuchKey")
            body = MagicMock()
            body.read = lambda k=Key: objects[k]
            return {"Body": body}

        fake.get_object.side_effect = get_object
        return fake

    def test_describe_flow_saves_via_storage_route(self):
        doc = {"id": "b1", "name": "b1", "saved_at": "S1",
               "entries": [
                   {"entry_id": "e1", "job_id": "j", "artifact_id": "a1"},
                   {"entry_id": "e2", "job_id": "j", "artifact_id": "a2",
                    "title_override": "Kept Title", "body_override": "kept"},
               ]}
        gemini = {"candidates": [{"content": {"parts": [{"text":
            '{"title": "Petrol Lattice", "description": "Threads over bone."}'}]}}]}
        invokes = []
        fake_lambda = MagicMock()

        def invoke(FunctionName=None, Payload=None):
            invokes.append((FunctionName, json.loads(Payload)))
            resp = MagicMock()
            resp.__getitem__ = lambda s2, k: MagicMock(
                read=lambda: json.dumps({"statusCode": 200, "body": json.dumps(
                    {"book": {"name": "b1", "saved_at": "S2"}})}).encode())
            return {"Payload": MagicMock(read=lambda: json.dumps(
                {"statusCode": 200, "body": json.dumps({"book": {"name": "b1"}})}).encode())}

        fake_lambda.invoke.side_effect = invoke
        with patch.object(self.mod, "s3", self._fake_s3(doc)), \
             patch.object(self.mod, "boto3") as fb, \
             patch.object(self.mod, "_gemini_call", return_value=gemini):
            fb.client.return_value = fake_lambda
            resp = self.mod.handle_describe({
                "job_id": "book#b1", "task_id": "bookdesc_r1",
                "book_id": "b1", "expected_saved_at": "S1"})

        body = json.loads(resp["body"])
        self.assertEqual(body["described"], 1)
        self.assertEqual(body["skipped"], 1)   # e2's hand prose survives
        fn, payload = invokes[-1]
        self.assertEqual(payload["path"], "/save-book")
        saved = json.loads(payload["body"])["book"]
        self.assertEqual(saved["entries"][0]["title_override"], "Petrol Lattice")
        self.assertEqual(saved["entries"][0]["body_override"], "Threads over bone.")
        self.assertEqual(saved["entries"][1]["title_override"], "Kept Title")
        deduped = [p for i, p in enumerate(self.phases) if i == 0 or self.phases[i-1] != p]
        self.assertEqual(deduped, [("started", "load_book"), ("processing", "describe"),
                                   ("processing", "save"), ("done", "done")])

    def test_entry_ids_subset_with_overwrite_regenerates_selection(self):
        doc = {"id": "b1", "name": "b1", "saved_at": "S1",
               "entries": [
                   {"entry_id": "e1", "job_id": "j", "artifact_id": "a1",
                    "title_override": "Old Title", "body_override": "old"},
                   {"entry_id": "e2", "job_id": "j", "artifact_id": "a2"},
               ]}
        gemini = {"candidates": [{"content": {"parts": [{"text":
            '{"title": "Fresh Title", "description": "New words."}'}]}}]}
        invokes = []
        fake_lambda = MagicMock()
        fake_lambda.invoke.side_effect = lambda FunctionName=None, Payload=None: (
            invokes.append(json.loads(Payload)) or
            {"Payload": MagicMock(read=lambda: json.dumps(
                {"statusCode": 200, "body": json.dumps({"book": {"name": "b1"}})}).encode())})
        with patch.object(self.mod, "s3", self._fake_s3(doc)), \
             patch.object(self.mod, "boto3") as fb, \
             patch.object(self.mod, "_gemini_call", return_value=gemini):
            fb.client.return_value = fake_lambda
            resp = self.mod.handle_describe({
                "job_id": "book#b1", "task_id": "t", "book_id": "b1",
                "expected_saved_at": "S1",
                "entry_ids": ["e1"], "overwrite": True})
        body = json.loads(resp["body"])
        self.assertEqual(body["described"], 1)
        saved = json.loads(invokes[-1]["body"])["book"]
        self.assertEqual(saved["entries"][0]["title_override"], "Fresh Title")
        # e2 was outside the selection: untouched
        self.assertNotIn("title_override", saved["entries"][1])

    def test_missing_key_and_saved_at_mismatch(self):
        with patch.dict(self.mod.os.environ, {"GEMINI_API_KEY": ""}):
            with self.assertRaises(RuntimeError) as ctx:
                self.mod.handle_describe({"book_id": "b1"})
        self.assertIn("GEMINI_API_KEY", str(ctx.exception))

        doc = {"id": "b1", "saved_at": "S2", "entries": [{"entry_id": "e1"}]}
        with patch.object(self.mod, "s3", self._fake_s3(doc)):
            with self.assertRaises(RuntimeError) as ctx:
                self.mod.handle_describe({
                    "job_id": "j", "task_id": "t",
                    "book_id": "b1", "expected_saved_at": "S1"})
        self.assertIn("saved mid-describe", str(ctx.exception))


class VisionRoutingTests(unittest.TestCase):
    def setUp(self):
        self.mod = _load_engine()
        self.calls = []

        def capture(url, body, api_key, headers=None, pacing=None, **_kw):
            self.calls.append({"url": url, "body": json.loads(body),
                               "headers": headers or {}})
            if "anthropic" in url:
                return {"content": [{"text": '{"title": "T", "description": "D"}'}]}
            if "openai" in url:
                return {"choices": [{"message": {"content": '{"title": "T", "description": "D"}'}}]}
            return {"candidates": [{"content": {"parts": [{"text": '{"title": "T", "description": "D"}'}]}}]}

        self.p = patch.object(self.mod, "_gemini_call", side_effect=capture)
        self.p.start()
        self.addCleanup(self.p.stop)

    def test_claude_model_uses_anthropic_wire_shape(self):
        out = self.mod._vision_call("claude-sonnet-4-6", "sk-ant-x", b"jpg", "prompt")
        self.assertEqual(json.loads(out) if out.startswith("{") else out,
                         {"title": "T", "description": "D"})
        call = self.calls[0]
        self.assertIn("api.anthropic.com/v1/messages", call["url"])
        self.assertEqual(call["headers"]["x-api-key"], "sk-ant-x")
        self.assertEqual(call["headers"]["anthropic-version"], "2023-06-01")
        content = call["body"]["messages"][0]["content"]
        self.assertEqual(content[0]["type"], "image")
        self.assertEqual(content[0]["source"]["media_type"], "image/jpeg")
        self.assertEqual(content[1]["text"], "prompt")

    def test_openai_model_uses_chat_completions_shape(self):
        self.mod._vision_call("gpt-4.1-mini", "sk-oai", b"jpg", "prompt")
        call = self.calls[0]
        self.assertIn("api.openai.com/v1/chat/completions", call["url"])
        self.assertEqual(call["headers"]["Authorization"], "Bearer sk-oai")
        content = call["body"]["messages"][0]["content"]
        self.assertTrue(content[0]["image_url"]["url"].startswith("data:image/jpeg;base64,"))
        self.assertEqual(call["body"]["response_format"], {"type": "json_object"})

    def test_gemini_model_uses_generatecontent(self):
        self.mod._vision_call("gemini-2.5-flash", "gk", b"jpg", "prompt")
        self.assertIn("generativelanguage.googleapis.com", self.calls[0]["url"])

    def test_missing_key_error_names_the_config(self):
        with patch.object(self.mod, "_load_vision_config",
                          lambda: {"model": "claude-sonnet-4-6", "api_key": ""}), \
             patch.dict(self.mod.os.environ, {"GEMINI_API_KEY": "gk"}), \
             patch.object(self.mod, "report_status", lambda *a, **k: None):
            with self.assertRaises(RuntimeError) as ctx:
                self.mod.handle_describe({"job_id": "j", "task_id": "t", "book_id": "b1"})
        self.assertIn("VisionModel config", str(ctx.exception))
        self.assertIn("claude-sonnet-4-6", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
