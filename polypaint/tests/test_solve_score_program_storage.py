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


class TestSolveScoreProgramStorage(unittest.TestCase):
    def _event(self, path, body):
        return {"path": path, "body": json.dumps(body)}

    @patch("handler_storage.s3")
    def test_storage_routes_round_trip_saved_program(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.paginate.side_effect = getattr(fake_s3, "paginate", None)
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object
        mock_s3.delete_object.side_effect = fake_s3.delete_object

        save_resp = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {"name": "Proximity q=0.1%", "chain": [["proximity", "0.1"]]},
            ),
            None,
        )
        self.assertEqual(save_resp["statusCode"], 200)
        save_body = json.loads(save_resp["body"])
        program_id = save_body["program"]["id"]
        self.assertFalse(save_body["overwritten"])
        self.assertEqual(save_body["program"]["chain"], [["proximity", "0.1"]])

        list_resp = handler_storage.handler(self._event("/list-solve-score-programs", {}), None)
        list_body = json.loads(list_resp["body"])
        self.assertEqual(list_body["count"], 1)
        self.assertEqual(list_body["programs"][0]["id"], program_id)
        self.assertEqual(list_body["programs"][0]["statement_count"], 1)
        self.assertEqual(list_body["error_count"], 0)

        fetch_resp = handler_storage.handler(
            self._event("/fetch-solve-score-program", {"id": program_id}),
            None,
        )
        fetch_body = json.loads(fetch_resp["body"])
        self.assertEqual(fetch_body["program"]["id"], program_id)
        self.assertEqual(fetch_body["program"]["metric"], "proximity")

        delete_resp = handler_storage.handler(
            self._event("/delete-solve-score-program", {"id": program_id}),
            None,
        )
        delete_body = json.loads(delete_resp["body"])
        self.assertEqual(delete_body, {"id": program_id, "deleted": 1})

        refetch_resp = handler_storage.handler(
            self._event("/fetch-solve-score-program", {"id": program_id}),
            None,
        )
        self.assertEqual(refetch_resp["statusCode"], 404)

    @patch("handler_storage.s3")
    def test_client_supplied_id_and_derived_fields_are_ignored(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        resp = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {
                    "id": "attacker-slug",
                    "version": 999,
                    "name": "My Safe Name",
                    "chain": [["proximity", "0.1"]],
                    "metric": "clusteriness",
                    "display": "bad",
                    "program_spec": "bad",
                    "statement_count": 999,
                    "saved_at": "1999-01-01T00:00:00Z",
                },
            ),
            None,
        )
        body = json.loads(resp["body"])
        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(body["program"]["id"], "my-safe-name")
        self.assertEqual(body["program"]["version"], 1)
        self.assertEqual(body["program"]["metric"], "proximity")
        stored = json.loads(fake_s3.objects["polypaint/solve-score-programs/my-safe-name.json"].decode("utf-8"))
        self.assertEqual(stored["id"], "my-safe-name")
        self.assertEqual(stored["version"], 1)
        self.assertEqual(stored["metric"], "proximity")

    @patch("handler_storage.s3")
    def test_save_compile_error_returns_400_and_writes_nothing(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        resp = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {"name": "Broken Program", "chain": [["weighted_sum", "1", "2"]]},
            ),
            None,
        )
        self.assertEqual(resp["statusCode"], 400)
        body = json.loads(resp["body"])
        self.assertIn("error", body)
        self.assertEqual(fake_s3.objects, {})

    @patch("handler_storage.s3")
    def test_slug_collision_overwrites_same_saved_program_key(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        first = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {"name": "My Prog!", "chain": [["proximity", "0.1"]]},
            ),
            None,
        )
        first_body = json.loads(first["body"])
        program_id = first_body["program"]["id"]
        self.assertFalse(first_body["overwritten"])

        second = handler_storage.handler(
            self._event(
                "/save-solve-score-program",
                {"name": "my prog?", "chain": [["proximity", "0.2"]]},
            ),
            None,
        )
        second_body = json.loads(second["body"])
        self.assertTrue(second_body["overwritten"])
        self.assertEqual(second_body["program"]["id"], program_id)

        fetch = handler_storage.handler(
            self._event("/fetch-solve-score-program", {"id": program_id}),
            None,
        )
        fetch_body = json.loads(fetch["body"])
        self.assertEqual(fetch_body["program"]["name"], "my prog?")
        self.assertEqual(fetch_body["program"]["chain"], [["proximity", "0.2"]])

    @patch("handler_storage.s3")
    def test_list_uses_object_metadata_and_surfaces_corrupt_entries(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        good_key = "polypaint/solve-score-programs/good.json"
        fake_s3.put_object(
            Key=good_key,
            Body=json.dumps({
                "version": 1,
                "id": "good",
                "name": "Good",
                "chain": [["proximity", "0.1"]],
                "metric": "proximity",
                "display": "proximity(slv,0.1)",
                "program_spec": "m0",
                "statement_count": 1,
                "saved_at": "2026-04-20T12:00:00Z",
            }).encode("utf-8"),
            Metadata={
                handler_storage.SOLVE_SCORE_PROGRAM_META_NAME: "Good",
                handler_storage.SOLVE_SCORE_PROGRAM_META_STATEMENT_COUNT: "1",
                handler_storage.SOLVE_SCORE_PROGRAM_META_SAVED_AT: "2026-04-20T12:00:00Z",
            },
        )
        bad_key = "polypaint/solve-score-programs/bad.json"
        fake_s3.put_object(Key=bad_key, Body=b"{not-json}", Metadata={})

        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        resp = handler_storage.handler(self._event("/list-solve-score-programs", {}), None)
        body = json.loads(resp["body"])
        self.assertEqual(body["count"], 1)
        self.assertEqual(body["programs"][0]["id"], "good")
        self.assertEqual(body["error_count"], 1)
        self.assertEqual(body["errors"][0]["id"], "bad")

    @patch("handler_storage.s3")
    def test_validation_limits_and_missing_ids(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object
        mock_s3.delete_object.side_effect = fake_s3.delete_object

        too_long_name = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "x" * 121,
                "chain": [["proximity", "0.1"]],
            }),
            None,
        )
        self.assertEqual(too_long_name["statusCode"], 400)

        too_many_statements = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "Many",
                "chain": [["proximity", "0.1"]] * 257,
            }),
            None,
        )
        self.assertEqual(too_many_statements["statusCode"], 400)

        too_long_token = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "Token",
                "chain": [["proximity", "0.1"], ["sawtooth", "x" * 129]],
            }),
            None,
        )
        self.assertEqual(too_long_token["statusCode"], 400)

        oversized_chain = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "Large",
                "chain": [["proximity", "0.1"]] + [["sawtooth", "1" * 120]] * 150,
            }),
            None,
        )
        self.assertEqual(oversized_chain["statusCode"], 400)

        fetch_missing = handler_storage.handler(
            self._event("/fetch-solve-score-program", {"id": "missing"}),
            None,
        )
        self.assertEqual(fetch_missing["statusCode"], 404)

        delete_missing = handler_storage.handler(
            self._event("/delete-solve-score-program", {"id": "missing"}),
            None,
        )
        self.assertEqual(delete_missing["statusCode"], 404)

        empty_name = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "   ",
                "chain": [["proximity", "0.1"]],
            }),
            None,
        )
        self.assertEqual(empty_name["statusCode"], 400)

    @patch("handler_storage.s3")
    def test_slug_edges_use_fallback_and_truncate(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object

        punctuation_only = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": "!!!",
                "chain": [["proximity", "0.1"]],
            }),
            None,
        )
        punctuation_body = json.loads(punctuation_only["body"])
        self.assertEqual(punctuation_body["program"]["id"], "solve-score-program")

        long_name = "a" * 120
        long_resp = handler_storage.handler(
            self._event("/save-solve-score-program", {
                "name": long_name,
                "chain": [["proximity", "0.1"]],
            }),
            None,
        )
        long_body = json.loads(long_resp["body"])
        self.assertEqual(len(long_body["program"]["id"]), 64)


if __name__ == "__main__":
    unittest.main()
