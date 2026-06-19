import io
import json
import os
import sys
import unittest
from unittest.mock import patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _FakeS3:
    def __init__(self):
        self.objects = {}
        self.metadata = {}

    def get_paginator(self, name):
        if name != "list_objects_v2":
            raise AssertionError(f"unexpected paginator {name}")
        return self

    def paginate(self, Bucket=None, Prefix=None, Delimiter=None):
        keys = sorted(key for key in self.objects if key.startswith(Prefix or ""))
        return [{"Contents": [{"Key": key} for key in keys]}]

    def get_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise Exception("NoSuchKey")
        return {"Body": io.BytesIO(self.objects[Key])}

    def put_object(self, Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None):
        if isinstance(Body, bytes):
            data = Body
        else:
            data = str(Body or "").encode("utf-8")
        self.objects[Key] = data
        self.metadata[Key] = dict(Metadata or {})
        return {}

    def head_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise Exception("NoSuchKey")
        return {"Metadata": dict(self.metadata.get(Key) or {})}

    def delete_object(self, Bucket=None, Key=None):
        self.objects.pop(Key, None)
        self.metadata.pop(Key, None)
        return {}


class TestParamProgramStorage(unittest.TestCase):
    def _event(self, path, body):
        return {"path": path, "body": json.dumps(body)}

    def _patch_s3(self, mock_s3, fake_s3):
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object
        mock_s3.delete_object.side_effect = fake_s3.delete_object

    @patch("handler_storage.s3")
    def test_storage_routes_round_trip_param_program(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        save_resp = handler_storage.handler(
            self._event(
                "/save-param-program",
                {
                    "name": "Sum Difference",
                    "chain": [
                        ["push", "t1"],
                        ["push", "t2"],
                        ["add"],
                        ["emit", "p1"],
                        ["push", "t1"],
                        ["push", "t2"],
                        ["subtract"],
                        ["emit", "p2"],
                    ],
                },
            ),
            None,
        )
        self.assertEqual(save_resp["statusCode"], 200)
        save_body = json.loads(save_resp["body"])
        program = save_body["program"]
        program_id = program["id"]
        self.assertEqual(program["program_kind"], "param_program")
        self.assertEqual(program["token_count"], 8)
        self.assertEqual(program["stack_max"], 2)
        self.assertEqual(program["emits"], ["p1", "p2"])
        self.assertFalse(program["uses_legacy_fast_path"])
        self.assertFalse(save_body["overwritten"])

        stored = json.loads(fake_s3.objects[f"polypaint/param-programs/{program_id}.json"].decode("utf-8"))
        self.assertEqual(stored["chain"], program["chain"])
        self.assertNotIn("execution_tokens", stored)

        list_resp = handler_storage.handler(self._event("/list-param-programs", {}), None)
        list_body = json.loads(list_resp["body"])
        self.assertEqual(list_body["count"], 1)
        self.assertEqual(list_body["programs"][0]["id"], program_id)
        self.assertEqual(list_body["programs"][0]["statement_count"], 8)
        self.assertEqual(list_body["error_count"], 0)

        fetch_resp = handler_storage.handler(self._event("/fetch-param-program", {"id": program_id}), None)
        fetch_body = json.loads(fetch_resp["body"])
        self.assertEqual(fetch_body["program"]["fingerprint"], program["fingerprint"])
        self.assertEqual(fetch_body["program"]["display"], program["display"])
        self.assertEqual(fetch_body["program"]["chain"], program["chain"])

        delete_resp = handler_storage.handler(self._event("/delete-param-program", {"id": program_id}), None)
        self.assertEqual(json.loads(delete_resp["body"]), {"id": program_id, "deleted": 1})
        missing = handler_storage.handler(self._event("/fetch-param-program", {"id": program_id}), None)
        self.assertEqual(missing["statusCode"], 404)

    @patch("handler_storage.s3")
    def test_macro_save_expands_against_saved_catalogue(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        base = handler_storage.handler(
            self._event("/save-param-program", {
                "name": "Base Fold",
                "chain": [["unit_circle", "both"]],
            }),
            None,
        )
        self.assertEqual(base["statusCode"], 200)
        base_body = json.loads(base["body"])
        base_id = base_body["program"]["id"]

        macro = handler_storage.handler(
            self._event("/save-param-program", {
                "name": "Macro Use",
                "chain": [["macro", base_id]],
            }),
            None,
        )
        self.assertEqual(macro["statusCode"], 200)
        macro_body = json.loads(macro["body"])
        self.assertEqual(macro_body["program"]["macro_expansions"], 1)
        self.assertEqual(macro_body["program"]["fingerprint"], base_body["program"]["fingerprint"])

    @patch("handler_storage.s3")
    def test_storage_routes_round_trip_param_program_source_text(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        compile_resp = handler_storage.handler(
            self._event("/compile-param-program-source", {"source_text": "p1 = t1 + t2\np2 = t1 - t2"}),
            None,
        )
        self.assertEqual(compile_resp["statusCode"], 200)
        compiled = json.loads(compile_resp["body"])
        self.assertTrue(compiled["ok"])
        self.assertEqual(compiled["chain"][0], ["const", "t1+t2"])
        self.assertTrue(compiled["fingerprint"])

        save_resp = handler_storage.handler(
            self._event(
                "/save-param-program",
                {
                    "name": "Source Param",
                    "source_text": "p1 = t1 + t2\np2 = t1 - t2",
                    "chain": [["push", "t1"]],
                },
            ),
            None,
        )
        self.assertEqual(save_resp["statusCode"], 200)
        program = json.loads(save_resp["body"])["program"]
        self.assertEqual(program["chain"][0], ["const", "t1+t2"])
        self.assertIn("source_text", program)

        fetch_resp = handler_storage.handler(self._event("/fetch-param-program", {"id": program["id"]}), None)
        fetched = json.loads(fetch_resp["body"])["program"]
        self.assertEqual(fetched["fingerprint"], program["fingerprint"])
        self.assertEqual(fetched["source_text"], program["source_text"])

    @patch("handler_storage.s3")
    def test_self_macro_reference_is_rejected(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        resp = handler_storage.handler(
            self._event("/save-param-program", {
                "name": "Self",
                "chain": [["macro", "self"]],
            }),
            None,
        )
        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("cannot reference itself", json.loads(resp["body"])["error"])
        self.assertEqual(fake_s3.objects, {})

    @patch("handler_storage.s3")
    def test_list_uses_metadata_and_surfaces_corrupt_entries(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        good_key = "polypaint/param-programs/good.json"
        fake_s3.put_object(
            Key=good_key,
            Body=json.dumps({
                "version": 1,
                "program_kind": "param_program",
                "id": "good",
                "name": "Good",
                "chain": [["unit_circle", "both"]],
                "saved_at": "2026-04-20T12:00:00Z",
            }).encode("utf-8"),
            Metadata={
                handler_storage.PARAM_PROGRAM_META_NAME: "Good",
                handler_storage.PARAM_PROGRAM_META_STATEMENT_COUNT: "1",
                handler_storage.PARAM_PROGRAM_META_SAVED_AT: "2026-04-20T12:00:00Z",
            },
        )
        fake_s3.put_object(Key="polypaint/param-programs/bad.json", Body=b"{bad-json}", Metadata={})
        self._patch_s3(mock_s3, fake_s3)

        resp = handler_storage.handler(self._event("/list-param-programs", {}), None)
        body = json.loads(resp["body"])
        self.assertEqual(body["count"], 1)
        self.assertEqual(body["programs"][0]["id"], "good")
        self.assertEqual(body["error_count"], 1)
        self.assertEqual(body["errors"][0]["id"], "bad")

    @patch("handler_storage.s3")
    def test_validation_limits_and_missing_ids(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        for body in [
            {"name": "x" * 121, "chain": [["unit_circle", "both"]]},
            {"name": "Many", "chain": [["unit_circle", "both"]] * 257},
            {"name": "Token", "chain": [["macro", "x" * 257]]},
            {"name": "Broken", "chain": [["emit", "p1"]]},
        ]:
            resp = handler_storage.handler(self._event("/save-param-program", body), None)
            self.assertEqual(resp["statusCode"], 400)

        fetch_missing = handler_storage.handler(self._event("/fetch-param-program", {"id": "missing"}), None)
        self.assertEqual(fetch_missing["statusCode"], 404)

        delete_missing = handler_storage.handler(self._event("/delete-param-program", {"id": "missing"}), None)
        self.assertEqual(delete_missing["statusCode"], 404)


if __name__ == "__main__":
    unittest.main()
