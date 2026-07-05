import json
import os
import sys
import unittest


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(path, body):
    return {"path": path, "body": json.dumps(body)}


class TestRootProgramStorage(unittest.TestCase):
    def test_compile_root_program_source_route(self):
        import handler_storage

        resp = handler_storage.handler(
            _event("/compile-root-program-source", {"source_text": "rotate_roots(0.25)\npull_unit_circle()"}),
            None,
        )
        self.assertEqual(resp["statusCode"], 200)
        body = json.loads(resp["body"])
        self.assertTrue(body["ok"])
        self.assertEqual(body["chain"][1], {"name": "pull_unit_circle", "fn_index": 2, "args": [0.75, 1.0]})
        self.assertEqual(body["root_transforms"][1], ["pull_unit_circle", "0.75", "1"])
        self.assertTrue(body["fingerprint"].startswith("sha256:"))
        self.assertEqual(body["program"]["program_kind"], "root_program")
        self.assertIn("execution_spec", body["program"])

    def test_compile_root_program_source_route_reports_diagnostics(self):
        import handler_storage

        resp = handler_storage.handler(
            _event("/compile-root-program-source", {"source_text": "roots = roots\nunknown_op(1)"}),
            None,
        )
        self.assertEqual(resp["statusCode"], 200)
        body = json.loads(resp["body"])
        self.assertFalse(body["ok"])
        self.assertEqual(body["chain"], [])
        self.assertEqual(body["program"]["chain"], [])
        self.assertGreaterEqual(len(body["diagnostics"]), 2)


if __name__ == "__main__":
    unittest.main()


class TestRootProgramPersistence(unittest.TestCase):
    """Save/list/fetch/delete lifecycle with a mocked S3."""

    def setUp(self):
        import handler_storage
        from unittest import mock

        self.hs = handler_storage
        self.store = {}
        self.meta = {}

        def put_object(Bucket, Key, Body, ContentType=None, Metadata=None):
            self.store[Key] = Body
            self.meta[Key] = Metadata or {}
            return {}

        def get_object(Bucket, Key):
            if Key not in self.store:
                raise self.hs.s3.exceptions.NoSuchKey({"Error": {"Code": "NoSuchKey"}}, "GetObject") \
                    if hasattr(self.hs.s3, "exceptions") else KeyError(Key)
            import io
            return {"Body": io.BytesIO(self.store[Key])}

        def head_object(Bucket, Key):
            if Key not in self.store:
                raise KeyError(Key)
            return {"Metadata": self.meta.get(Key, {})}

        def delete_object(Bucket, Key):
            self.store.pop(Key, None)
            self.meta.pop(Key, None)
            return {}

        class _Paginator:
            def __init__(self, store):
                self._store = store

            def paginate(self, Bucket, Prefix):
                yield {"Contents": [{"Key": k} for k in sorted(self._store) if k.startswith(Prefix)]}

        self.patches = [
            mock.patch.object(self.hs.s3, "put_object", side_effect=put_object),
            mock.patch.object(self.hs.s3, "get_object", side_effect=get_object),
            mock.patch.object(self.hs.s3, "head_object", side_effect=head_object),
            mock.patch.object(self.hs.s3, "delete_object", side_effect=delete_object),
            mock.patch.object(self.hs.s3, "get_paginator", return_value=_Paginator(self.store)),
        ]
        for p in self.patches:
            p.start()
        self.addCleanup(lambda: [p.stop() for p in self.patches])

    def _call(self, path, body):
        resp = self.hs.handler(_event(path, body), None)
        self.assertEqual(resp["statusCode"], 200, resp)
        return json.loads(resp["body"])

    def test_save_list_fetch_delete_roundtrip(self):
        src = "rotate_roots(0.25)\npull_unit_circle()"
        saved = self._call("/save-root-program", {"name": "spin v1", "source_text": src})
        program = saved["program"]
        self.assertEqual(program["id"], "spin-v1")
        self.assertEqual(program["program_kind"], "root_program")
        self.assertEqual(program["source_text"], src)
        self.assertTrue(program["fingerprint"].startswith("sha256:"))
        self.assertFalse(saved["overwritten"])
        # exact key + metadata contract
        key = "polypaint/root-programs/spin-v1.json"
        self.assertIn(key, self.store)
        self.assertEqual(self.meta[key]["root_program_name"], "spin v1")
        self.assertEqual(self.meta[key]["root_program_statement_count"], "2")

        listed = self._call("/list-root-programs", {})
        self.assertEqual([row["id"] for row in listed["programs"]], ["spin-v1"])
        self.assertEqual(listed["programs"][0]["statement_count"], 2)

        fetched = self._call("/fetch-root-program", {"id": "spin-v1"})
        self.assertEqual(fetched["program"]["source_text"], src)
        self.assertEqual(fetched["program"]["fingerprint"], program["fingerprint"])

        deleted = self._call("/delete-root-program", {"id": "spin-v1"})
        self.assertEqual(deleted["deleted"], 1)
        self.assertEqual(self._call("/list-root-programs", {})["programs"], [])

    def test_missing_id_fetch_and_delete_return_404(self):
        for path in ("/fetch-root-program", "/delete-root-program"):
            resp = self.hs.handler(_event(path, {"id": "ghost"}), None)
            self.assertEqual(resp["statusCode"], 404, (path, resp))

    def test_save_rejects_control_char_name_and_empty_source(self):
        resp = self.hs.handler(
            _event("/save-root-program", {"name": "foo\nbar", "source_text": "rotate_roots(0.25)"}), None)
        self.assertEqual(resp["statusCode"], 400)
        resp = self.hs.handler(
            _event("/save-root-program", {"name": "empty", "source_text": "  "}), None)
        self.assertEqual(resp["statusCode"], 400)

    def test_punctuation_only_name_gets_root_slug_fallback(self):
        saved = self._call("/save-root-program", {"name": "!!!", "source_text": "rotate_roots(0.25)"})
        self.assertEqual(saved["program"]["id"], "root-program")

    def test_blank_source_with_chain_does_not_load_silently_empty(self):
        key = "polypaint/root-programs/handmade.json"
        self.store[key] = json.dumps({
            "name": "handmade", "source_text": "",
            "chain": [{"name": "rotate_roots", "fn_index": 1, "args": [0.25]}],
        }).encode("utf-8")
        self.meta[key] = {}
        resp = self.hs.handler(_event("/fetch-root-program", {"id": "handmade"}), None)
        self.assertGreaterEqual(resp["statusCode"], 400)
        listed = self._call("/list-root-programs", {})
        self.assertEqual(listed["programs"], [])
        self.assertEqual(listed["error_count"], 1)
        self.assertEqual(listed["errors"][0]["id"], "handmade")

    def test_save_rejects_invalid_source(self):
        resp = self.hs.handler(
            _event("/save-root-program", {"name": "bad", "source_text": "unknown_fn(1)"}), None)
        self.assertGreaterEqual(resp["statusCode"], 400)
