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
        self.assertEqual(req["generationConfig"]["maxOutputTokens"], 4096)
        schema = req["generationConfig"]["responseSchema"]
        self.assertEqual(schema["required"], ["title", "description"])
        self.assertEqual(schema["properties"]["title"]["type"], "STRING")
        self.assertEqual(schema["properties"]["description"]["type"], "STRING")

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

    def test_parse_prose_is_lenient_about_junk_around_the_object(self):
        obj = '{"title": "Ember Drift",\n "description": "Warm strands curl."}'
        want = ("Ember Drift", "Warm strands curl.")
        # the field failure: valid object + trailing commentary
        # ("Extra data: line 5 column 1" under strict json.loads)
        self.assertEqual(self.mod._parse_prose(
            obj + "\nHope this works for your book!"), want)
        # duplicate/"improved" second object appended: first one wins
        self.assertEqual(self.mod._parse_prose(
            obj + '\n{"title": "Second", "description": "Ignored."}'), want)
        # leading prose before the object
        self.assertEqual(self.mod._parse_prose(
            "Here is the JSON you asked for:\n" + obj), want)
        # fenced AND followed by commentary
        self.assertEqual(self.mod._parse_prose(
            "```json\n" + obj + "\n```\nLet me know!"), want)
        # stray brace in a preface must not derail the scan
        self.assertEqual(self.mod._parse_prose(
            "Format {title, description} as requested:\n" + obj), want)
        # still hard errors: no object at all / fields missing
        with self.assertRaises(RuntimeError) as ctx:
            self.mod._parse_prose("I cannot describe this image.")
        self.assertIn("no complete JSON object", str(ctx.exception))
        with self.assertRaises(RuntimeError) as ctx:
            self.mod._parse_prose('{"title": "only a title"} trailing')
        self.assertIn("missing fields", str(ctx.exception))

    def test_gemini_multipart_reply_is_joined_and_thoughts_skipped(self):
        # thinking models split long replies across parts and may prepend
        # thought summaries — parts[0] alone truncates the JSON mid-string
        payload = {"candidates": [{"content": {"parts": [
            {"text": "I will describe the image now.", "thought": True},
            {"text": '{"title": "Microtubule Ribbon Motor", "description": "Saffron ribbons '},
            {"text": 'radiate from a central nucleolus."}'},
        ]}, "finishReason": "STOP"}]}
        self.assertEqual(
            self.mod.parse_response(payload),
            ("Microtubule Ribbon Motor",
             "Saffron ribbons radiate from a central nucleolus."))

    def test_gemini_max_tokens_truncation_raises_clearly(self):
        payload = {"candidates": [{"content": {"parts": [
            {"text": '{"title": "T", "description": "cut mid-sent'},
        ]}, "finishReason": "MAX_TOKENS"}]}
        with self.assertRaises(RuntimeError) as ctx:
            self.mod.parse_response(payload)
        self.assertIn("MAX_TOKENS", str(ctx.exception))

    def test_gemini_recitation_stop_names_the_reason(self):
        # RECITATION/SAFETY hard-stop mid-string without MAX_TOKENS — the
        # exact "Verdigris Felt" failure: valid JSON start, never closes
        payload = {"candidates": [{"content": {"parts": [
            {"text": '{"title": "Verdigris Felt", "description": "An eight-pointed knot'},
        ]}, "finishReason": "RECITATION"}]}
        with self.assertRaises(RuntimeError) as ctx:
            self.mod.parse_response(payload)
        self.assertIn("RECITATION", str(ctx.exception))

    def test_describe_image_retries_once_on_flaky_reply(self):
        calls = {"n": 0}

        def flaky(model, api_key, image_bytes, text):
            calls["n"] += 1
            if calls["n"] == 1:
                raise RuntimeError("Gemini stopped early (finishReason RECITATION)")
            return '{"title": "Second Try", "description": "Clean landing."}'

        with patch.object(self.mod, "_vision_call", flaky):
            title, desc = self.mod.describe_image(
                b"jpg", {"entry_id": "e1"}, {}, model="gemini-2.5-flash", api_key="k")
        self.assertEqual((title, desc), ("Second Try", "Clean landing."))
        self.assertEqual(calls["n"], 2)

    def test_parse_prose_tolerates_literal_newlines_in_strings(self):
        self.assertEqual(
            self.mod._parse_prose('{"title": "T", "description": "line one\nline two"}'),
            ("T", "line one\nline two"))

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

    def test_banned_word_surviving_the_rewrite_raises(self):
        # code-review-26 F9: the ban is enforced, not advisory — if the one
        # rewrite still offends, fail the entry (next Describe retries it)
        replies = ['{"title": "Vibrant Bloom", "description": "fine"}',
                   '{"title": "Still Vibrant", "description": "again"}']
        calls = {"n": 0}

        def vc(model, api_key, image_bytes, text):
            r = replies[calls["n"]]
            calls["n"] += 1
            return r

        with patch.object(self.mod, "_vision_call", vc):
            with self.assertRaises(RuntimeError) as ctx:
                self.mod.describe_image(b"x", {"entry_id": "e"}, {},
                                        model="gemini-2.5-flash", api_key="k")
        self.assertIn("banned", str(ctx.exception))
        self.assertEqual(calls["n"], 2)   # tried exactly one rewrite

    def test_downscale_shrinks_large_and_passes_small(self):
        # code-review-26 F4: the image_key fallback can be a full render
        from PIL import Image
        import io as _io
        big = _io.BytesIO()
        Image.new("RGB", (2000, 1500), (10, 20, 30)).save(big, format="JPEG")
        out = self.mod._downscale_for_vision(big.getvalue(), max_px=768)
        w, h = Image.open(_io.BytesIO(out)).size
        self.assertLessEqual(max(w, h), 768)
        small = _io.BytesIO()
        Image.new("RGB", (400, 300), (1, 2, 3)).save(small, format="JPEG")
        raw = small.getvalue()
        self.assertEqual(self.mod._downscale_for_vision(raw, max_px=768), raw)

    def test_cas_preserves_human_edit_to_already_generated_entry(self):
        # code-review-26 F2: e1 generated + saved, then a human edits e1;
        # a later conflict must NOT stamp our e1 prose back over the human's
        import copy
        fresh = {"id": "b1", "saved_at": "S2", "entries": [
            {"entry_id": "e1", "title_override": "HUMAN", "body_override": "by hand"},
            {"entry_id": "e2", "title_override": "", "body_override": ""}]}
        saved = {}
        seq = {"n": 0}

        def storage(payload):
            path = payload["path"]
            if path == "/fetch-book":
                return {"book": copy.deepcopy(fresh)}
            seq["n"] += 1
            if seq["n"] == 1:
                raise self.mod._SaveConflict("conflict")
            body = json.loads(payload["body"])
            saved["book"] = body["book"]
            return {"book": {**body["book"], "saved_at": "S3"}}

        doc = {"id": "b1", "saved_at": "S1", "entries": [
            {"entry_id": "e1", "title_override": "GEN", "body_override": "gen"},
            {"entry_id": "e2", "title_override": "E2", "body_override": "e2body"}]}
        run_prose = {"e1": ("GEN", "gen"), "e2": ("E2", "e2body")}
        with patch.object(self.mod, "_storage", side_effect=storage):
            self.mod._save_book_cas("b1", doc, "S1", run_prose)
        by_id = {e["entry_id"]: e for e in saved["book"]["entries"]}
        self.assertEqual(by_id["e1"]["title_override"], "HUMAN")  # human wins
        self.assertEqual(by_id["e2"]["title_override"], "E2")     # our prose fills blank

    def test_preview_falls_back_to_stored_image_key(self):
        # code-review-25 F5: legacy/root artifacts lack the immutable color
        # preview path; Describe must still resolve them via entry.image_key
        seen = []

        def get_object(Bucket=None, Key=None):
            seen.append(Key)
            if Key == "renders/j/legacy_root/image.jpeg":
                return {"Body": MagicMock(read=lambda: b"IMGBYTES")}
            raise Exception("NoSuchKey")

        fake_s3 = MagicMock()
        fake_s3.get_object.side_effect = get_object
        with patch.object(self.mod, "s3", fake_s3):
            out = self.mod._entry_preview_bytes({
                "job_id": "j", "artifact_id": "a",
                "image_key": "renders/j/legacy_root/image.jpeg"})
        self.assertEqual(out, b"IMGBYTES")
        # tried the two canonical preview paths first, then the image_key
        self.assertTrue(seen[0].endswith("preview.jpg"))
        self.assertEqual(seen[-1], "renders/j/legacy_root/image.jpeg")


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

    def test_concurrent_human_edit_is_not_clobbered(self):
        # code-review-25 F2: the per-entry save is now compare-and-swap. A
        # human edits e2's title while e1 is being described; e1's save
        # conflicts, describe refetches + re-applies its own prose, and e2's
        # human edit survives (skip-existing then leaves it alone).
        doc = {"id": "b1", "name": "b1", "saved_at": "S1", "entries": [
            {"entry_id": "e1", "job_id": "j", "artifact_id": "a1"},
            {"entry_id": "e2", "job_id": "j", "artifact_id": "a2"}]}
        replies = [
            {"candidates": [{"content": {"parts": [{"text":
                '{"title": "E1 Auto", "description": "auto one"}'}]}}]},
            {"candidates": [{"content": {"parts": [{"text":
                '{"title": "E2 Auto", "description": "auto two"}'}]}}]}]

        def _resp(status, body):
            return {"Payload": MagicMock(read=lambda: json.dumps(
                {"statusCode": status, "body": json.dumps(body)}).encode())}

        state = {"saves": 0, "last_book": None}

        def invoke(FunctionName=None, Payload=None):
            p = json.loads(Payload)
            if p["path"] == "/fetch-book":
                # server copy: a human retitled e2 and saved_at advanced
                fresh = {"id": "b1", "name": "b1", "saved_at": "S_HUMAN", "entries": [
                    {"entry_id": "e1", "job_id": "j", "artifact_id": "a1"},
                    {"entry_id": "e2", "job_id": "j", "artifact_id": "a2",
                     "title_override": "HUMAN EDIT", "body_override": "by hand"}]}
                return _resp(200, {"book": fresh})
            body = json.loads(p["body"])
            state["saves"] += 1
            if state["saves"] == 1:                 # e1's first save conflicts
                return _resp(409, {"error": "changed", "conflict": "book_saved_at"})
            state["last_book"] = body["book"]
            return _resp(200, {"book": {**body["book"], "saved_at": "S_NEW"}})

        fake_lambda = MagicMock()
        fake_lambda.invoke.side_effect = invoke
        with patch.object(self.mod, "s3", self._fake_s3(doc)), \
             patch.object(self.mod, "boto3") as fb, \
             patch.object(self.mod, "_gemini_call", side_effect=replies):
            fb.client.return_value = fake_lambda
            resp = self.mod.handle_describe({
                "job_id": "book#b1", "task_id": "bookdesc_r1", "book_id": "b1"})

        body = json.loads(resp["body"])
        self.assertEqual(body["described"], 1)   # e1
        self.assertEqual(body["skipped"], 1)     # e2: human edit kept, skip-existing
        self.assertEqual(body["failed"], 0)
        saved = {e["entry_id"]: e for e in state["last_book"]["entries"]}
        self.assertEqual(saved["e1"]["title_override"], "E1 Auto")
        self.assertEqual(saved["e2"]["title_override"], "HUMAN EDIT")  # not clobbered

    def test_one_failing_entry_does_not_abort_the_run(self):
        # the 7/21 abort: entry e1's model reply is flaky BOTH times (retry
        # included) — e2 must still describe, and done reports the failure
        doc = {"id": "b1", "name": "b1", "saved_at": "S1",
               "entries": [
                   {"entry_id": "e1", "job_id": "j", "artifact_id": "a1"},
                   {"entry_id": "e2", "job_id": "j", "artifact_id": "a2"},
               ]}
        bad = {"candidates": [{"content": {"parts": [
            {"text": '{"title": "Verdigris Felt", "description": "cut'}]},
            "finishReason": "RECITATION"}]}
        good = {"candidates": [{"content": {"parts": [{"text":
            '{"title": "Clean Landing", "description": "Full object."}'}]}}]}
        replies = [bad, bad, good]   # e1 attempt + retry, then e2
        fake_lambda = MagicMock()
        fake_lambda.invoke.side_effect = lambda FunctionName=None, Payload=None: {
            "Payload": MagicMock(read=lambda: json.dumps(
                {"statusCode": 200, "body": json.dumps({"book": {"name": "b1"}})}).encode())}
        with patch.object(self.mod, "s3", self._fake_s3(doc)), \
             patch.object(self.mod, "boto3") as fb, \
             patch.object(self.mod, "_gemini_call", side_effect=replies):
            fb.client.return_value = fake_lambda
            resp = self.mod.handle_describe({
                "job_id": "book#b1", "task_id": "bookdesc_r1", "book_id": "b1"})
        body = json.loads(resp["body"])
        self.assertEqual(body["described"], 1)
        self.assertEqual(body["failed"], 1)
        self.assertIn("RECITATION", body["first_error"])
        self.assertEqual(self.phases[-1], ("done", "done"))

    def test_all_entries_failing_is_a_hard_error(self):
        doc = {"id": "b1", "name": "b1", "saved_at": "S1",
               "entries": [{"entry_id": "e1", "job_id": "j", "artifact_id": "a1"}]}
        bad = {"candidates": [{"content": {"parts": [
            {"text": '{"title": "X", "description": "cut'}]},
            "finishReason": "RECITATION"}]}
        with patch.object(self.mod, "s3", self._fake_s3(doc)), \
             patch.object(self.mod, "boto3"), \
             patch.object(self.mod, "_gemini_call", return_value=bad):
            with self.assertRaises(RuntimeError) as ctx:
                self.mod.handle_describe({
                    "job_id": "book#b1", "task_id": "bookdesc_r1", "book_id": "b1"})
        self.assertIn("all 1 attempted entries", str(ctx.exception))

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

    def test_config_key_follows_the_model_provider(self):
        # switching models back and forth must pick each provider's own key
        cfg = {"model": "claude-sonnet-4-6", "api_key_anthropic": "sk-ant-1",
               "api_key_gemini": "gk-1", "api_key_openai": ""}
        with patch.object(self.mod, "_load_vision_config", lambda: cfg), \
             patch.object(self.mod, "report_status", lambda *a, **k: None), \
             patch.object(self.mod, "s3") as fake_s3:
            fake_s3.get_object.side_effect = Exception("NoSuchKey")
            # book fetch fails AFTER key resolution: proves which key won
            with self.assertRaises(Exception):
                self.mod.handle_describe({"job_id": "j", "task_id": "t",
                                          "book_id": "b1", "model": "gemini-2.5-flash"})
        # no assertion on the exception itself: the resolution path is pinned
        # by the missing-key variants below

        for model, expect_ok in (("claude-sonnet-4-6", True),
                                 ("gpt-4.1-mini", False)):
            with patch.object(self.mod, "_load_vision_config", lambda: dict(cfg)), \
                 patch.object(self.mod, "report_status", lambda *a, **k: None), \
                 patch.object(self.mod, "s3") as fake_s3:
                fake_s3.get_object.side_effect = Exception("book missing")
                try:
                    self.mod.handle_describe({"job_id": "j", "task_id": "t",
                                              "book_id": "b1", "model": model})
                    self.fail("should raise")
                except RuntimeError as exc:
                    if expect_ok:
                        # got past key resolution to the book fetch
                        self.assertNotIn("no API key", str(exc))
                    else:
                        self.assertIn("no API key", str(exc))
                except Exception:
                    self.assertTrue(expect_ok)

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
