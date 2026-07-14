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
        data = Body if isinstance(Body, bytes) else str(Body or "").encode("utf-8")
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


class TestCoeffProgramStorage(unittest.TestCase):
    def _event(self, path, body):
        return {"path": path, "body": json.dumps(body)}

    def _patch_s3(self, mock_s3, fake_s3):
        mock_s3.get_paginator.side_effect = fake_s3.get_paginator
        mock_s3.get_object.side_effect = fake_s3.get_object
        mock_s3.put_object.side_effect = fake_s3.put_object
        mock_s3.head_object.side_effect = fake_s3.head_object
        mock_s3.delete_object.side_effect = fake_s3.delete_object

    @patch("handler_storage.s3")
    def test_vector_constants_use_ordinary_saved_coeff_program_contract(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)
        upload = {
            "version": 1,
            "program_kind": "coeff_program",
            "name": "Vector Translation",
            "chain": [],
            "source_text": (
                "poly = translate_roots(vector_literal(1, -3, 2), 0.5)\n"
                "emit"
            ),
        }

        save_resp = handler_storage.handler(
            self._event("/save-coeff-program", upload), None
        )
        self.assertEqual(save_resp["statusCode"], 200, save_resp["body"])
        saved = json.loads(save_resp["body"])["program"]
        self.assertEqual(saved["id"], "vector-translation")
        self.assertEqual(saved["name"], "Vector Translation")
        self.assertEqual(saved["source_text"], upload["source_text"])
        self.assertEqual(saved["statement_count"], 2)

        key = "polypaint/coeff-programs/vector-translation.json"
        self.assertIn(key, fake_s3.objects)
        self.assertEqual(
            fake_s3.metadata[key],
            {
                "coeff_program_name": "Vector Translation",
                "coeff_program_statement_count": "2",
                "coeff_program_saved_at": saved["saved_at"],
            },
        )

        listed = json.loads(handler_storage.handler(
            self._event("/list-coeff-programs", {}), None
        )["body"])
        self.assertEqual(listed["count"], 1)
        self.assertEqual(listed["programs"][0]["id"], "vector-translation")

        fetched = json.loads(handler_storage.handler(
            self._event("/fetch-coeff-program", {"id": "vector-translation"}), None
        )["body"])["program"]
        self.assertEqual(fetched["fingerprint"], saved["fingerprint"])
        self.assertEqual(fetched["source_text"], upload["source_text"])

    @patch("handler_storage.s3")
    def test_storage_routes_round_trip_coeff_program(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        chain = [["const", "35", "p1+p2"], ["emit"]]
        save_resp = handler_storage.handler(
            self._event("/save-coeff-program", {"name": "P Sum Const", "chain": chain}),
            None,
        )
        self.assertEqual(save_resp["statusCode"], 200)
        save_body = json.loads(save_resp["body"])
        program = save_body["program"]
        program_id = program["id"]
        self.assertEqual(program["program_kind"], "coeff_program")
        self.assertEqual(program["token_count"], 2)
        self.assertEqual(program["scalar_expr_count"], 1)
        self.assertEqual(program["stack_max"], 1)
        self.assertFalse(program["uses_legacy_chain_equivalent"])
        self.assertFalse(save_body["overwritten"])

        stored = json.loads(fake_s3.objects[f"polypaint/coeff-programs/{program_id}.json"].decode("utf-8"))
        self.assertEqual(stored["chain"], program["chain"])
        self.assertNotIn("execution_tokens", stored)

        list_resp = handler_storage.handler(self._event("/list-coeff-programs", {}), None)
        list_body = json.loads(list_resp["body"])
        self.assertEqual(list_body["count"], 1)
        self.assertEqual(list_body["programs"][0]["id"], program_id)
        self.assertEqual(list_body["programs"][0]["statement_count"], 2)
        self.assertEqual(list_body["error_count"], 0)

        fetch_resp = handler_storage.handler(self._event("/fetch-coeff-program", {"id": program_id}), None)
        fetch_body = json.loads(fetch_resp["body"])
        self.assertEqual(fetch_body["program"]["fingerprint"], program["fingerprint"])
        self.assertEqual(fetch_body["program"]["display"], program["display"])
        self.assertEqual(fetch_body["program"]["chain"], program["chain"])

        delete_resp = handler_storage.handler(self._event("/delete-coeff-program", {"id": program_id}), None)
        self.assertEqual(json.loads(delete_resp["body"]), {"id": program_id, "deleted": 1})
        missing = handler_storage.handler(self._event("/fetch-coeff-program", {"id": program_id}), None)
        self.assertEqual(missing["statusCode"], 404)

    @patch("handler_storage.s3")
    def test_macro_save_expands_against_saved_catalogue(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        base = handler_storage.handler(
            self._event("/save-coeff-program", {
                "name": "Legacy Smooth",
                "chain": [["legacy", "rev", "poly", "poly"], ["legacy", "cumsum", "poly", "poly"]],
            }),
            None,
        )
        self.assertEqual(base["statusCode"], 200)
        base_body = json.loads(base["body"])
        base_id = base_body["program"]["id"]

        macro = handler_storage.handler(
            self._event("/save-coeff-program", {
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
    def test_fetch_prefers_v2_copy_when_present(self, mock_s3):
        # Load prefers the migrated v2/ copy, falls back to v1 when absent.
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        save_resp = handler_storage.handler(
            self._event("/save-coeff-program", {"name": "C", "chain": [["const", "35", "p1+p2"], ["emit"]]}),
            None,
        )
        program_id = json.loads(save_resp["body"])["program"]["id"]

        v2_obj = {
            "program_kind": "coeff_program",
            "version": 2,
            "id": program_id,
            "name": "C",
            "source_text": "poly = cf\n",
            "chain": [["set", "poly", "cf"]],
        }
        v2_key = f"polypaint/coeff-programs/v2/{program_id}.json"
        fake_s3.objects[v2_key] = (json.dumps(v2_obj) + "\n").encode("utf-8")
        fake_s3.metadata[v2_key] = {}

        prog = json.loads(
            handler_storage.handler(self._event("/fetch-coeff-program", {"id": program_id}), None)["body"]
        )["program"]
        self.assertNotIn("const", json.dumps(prog["chain"]))  # v2 (poly = cf) preferred

        del fake_s3.objects[v2_key]
        prog2 = json.loads(
            handler_storage.handler(self._event("/fetch-coeff-program", {"id": program_id}), None)["body"]
        )["program"]
        self.assertIn("const", json.dumps(prog2["chain"]))  # v1 fallback

    @patch("handler_storage.s3")
    def test_force_resave_after_migration_drops_stale_v2(self, mock_s3):
        # code-review-28 F8: a v1 re-save must drop the migrated v2 copy that
        # fetch prefers, or the edit is invisible forever. Exercises the single
        # save primitive (_put_program_v1_object) used by both API and scripts.
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        first = handler_storage.handler(
            self._event("/save-coeff-program", {"name": "Migrated", "chain": [["const", "35", "p1+p2"], ["emit"]]}),
            None,
        )
        program_id = json.loads(first["body"])["program"]["id"]

        # Simulate migration: a v2 object now shadows the v1 body on fetch.
        v2_key = f"polypaint/coeff-programs/v2/{program_id}.json"
        fake_s3.objects[v2_key] = (json.dumps({
            "program_kind": "coeff_program", "version": 2, "id": program_id,
            "name": "Migrated", "source_text": "poly = cf\n", "chain": [["set", "poly", "cf"]],
        }) + "\n").encode("utf-8")
        fake_s3.metadata[v2_key] = {}
        prefer_v2 = json.loads(
            handler_storage.handler(self._event("/fetch-coeff-program", {"id": program_id}), None)["body"]
        )["program"]
        self.assertNotIn("const", json.dumps(prefer_v2["chain"]))  # v2 shadows v1

        # Force-overwrite the v1 program with a new chain.
        resave = handler_storage.handler(
            self._event("/save-coeff-program", {"name": "Migrated", "chain": [["const", "42", "p1*p2"], ["emit"]]}),
            None,
        )
        self.assertEqual(resave["statusCode"], 200)
        self.assertTrue(json.loads(resave["body"])["overwritten"])

        # The stale v2 shadow is gone, and fetch now serves the new v1 body.
        self.assertNotIn(v2_key, fake_s3.objects, "stale v2 copy must be dropped on v1 re-save")
        after = json.loads(
            handler_storage.handler(self._event("/fetch-coeff-program", {"id": program_id}), None)["body"]
        )["program"]
        self.assertIn("42", json.dumps(after["chain"]))  # new v1 body served

    @patch("handler_storage.s3")
    def test_stale_v2_drop_is_unconditional_no_head_probe(self, mock_s3):
        # code-review-28 F8/F13: the v2 delete must NOT be gated on a HEAD probe.
        # A transient throttle on that HEAD used to skip the delete and leave the
        # stale v2 shadowing the new v1. Assert the v2 key is deleted and never
        # HEAD-probed during the save.
        import handler_storage
        from botocore.exceptions import ClientError

        fake_s3 = _FakeS3()

        def head_object(Bucket=None, Key=None):
            # A HEAD on the v2 key would be a throttle in the wild; the save must
            # never issue it. v1 key HEADs (overwrite check) still resolve normally.
            if "/v2/" in (Key or ""):
                raise ClientError({"Error": {"Code": "SlowDown"},
                                   "ResponseMetadata": {"HTTPStatusCode": 503}}, "HeadObject")
            if Key not in fake_s3.objects:
                raise Exception("NoSuchKey")
            return {"Metadata": dict(fake_s3.metadata.get(Key) or {})}

        self._patch_s3(mock_s3, fake_s3)
        mock_s3.head_object.side_effect = head_object

        first = handler_storage.handler(
            self._event("/save-coeff-program", {"name": "NoProbe", "chain": [["const", "35", "p1+p2"], ["emit"]]}),
            None,
        )
        program_id = json.loads(first["body"])["program"]["id"]
        v2_key = f"polypaint/coeff-programs/v2/{program_id}.json"
        fake_s3.objects[v2_key] = b"{}\n"
        fake_s3.metadata[v2_key] = {}

        resave = handler_storage.handler(
            self._event("/save-coeff-program", {"name": "NoProbe", "chain": [["const", "42", "p1*p2"], ["emit"]]}),
            None,
        )
        self.assertEqual(resave["statusCode"], 200, resave)
        self.assertNotIn(v2_key, fake_s3.objects, "v2 must be deleted despite a throttling HEAD")
        v2_heads = [c for c in mock_s3.head_object.call_args_list if "/v2/" in (c.kwargs.get("Key") or "")]
        self.assertEqual(v2_heads, [], "the v2 key must never be HEAD-probed before delete")

    @patch("handler_storage.s3")
    def test_source_text_save_fetch_and_compile_route(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        source = "cf\nrev\nemit\n"
        save_resp = handler_storage.handler(
            self._event("/save-coeff-program", {"name": "Source Rev", "source_text": source}),
            None,
        )
        self.assertEqual(save_resp["statusCode"], 200)
        program = json.loads(save_resp["body"])["program"]
        self.assertEqual(program["source_text"], source)
        self.assertEqual(program["chain"], [["push", "cf"], ["_native_transform", "rev", "pop", "push"], ["emit"]])
        self.assertEqual(program["statement_count"], 3)

        fetch_resp = handler_storage.handler(self._event("/fetch-coeff-program", {"id": program["id"]}), None)
        fetch_program = json.loads(fetch_resp["body"])["program"]
        self.assertEqual(fetch_program["source_text"], source)
        self.assertEqual(fetch_program["fingerprint"], program["fingerprint"])

        compile_resp = handler_storage.handler(
            self._event("/compile-coeff-program-source", {"source_text": "poly = cf"}),
            None,
        )
        compile_body = json.loads(compile_resp["body"])
        self.assertTrue(compile_body["ok"])
        self.assertEqual(compile_body["chain"], [["set", "poly", "cf"]])

    @patch("handler_storage.s3")
    def test_blank_source_text_does_not_shadow_saved_chain(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        # Imported/hand-edited saved object carrying both keys: blank source
        # must not shadow the chain on fetch or macro expansion.
        stale = {
            "name": "Stale Both Keys",
            "program_kind": "coeff_program",
            "chain": [["legacy", "rev", "poly", "poly"]],
            "source_text": "",
        }
        fake_s3.objects["polypaint/coeff-programs/stale-both-keys.json"] = (
            json.dumps(stale).encode("utf-8")
        )

        fetched = handler_storage.handler(
            self._event("/fetch-coeff-program", {"id": "stale-both-keys"}), None
        )
        self.assertEqual(fetched["statusCode"], 200)
        program = json.loads(fetched["body"])["program"]
        self.assertEqual(program["chain"], [["legacy", "rev", "poly", "poly"]])
        self.assertGreater(program["token_count"], 0)

        macro = handler_storage.handler(
            self._event("/save-coeff-program", {
                "name": "Macro Over Stale",
                "chain": [["macro", "stale-both-keys"]],
            }),
            None,
        )
        self.assertEqual(macro["statusCode"], 200)
        macro_program = json.loads(macro["body"])["program"]
        self.assertEqual(macro_program["macro_expansions"], 1)
        self.assertGreater(macro_program["token_count"], 0)

        # The save route accepts the same shape from non-UI clients.
        saved = handler_storage.handler(
            self._event("/save-coeff-program", {
                "name": "Save Both Keys",
                "chain": [["legacy", "rev", "poly", "poly"]],
                "source_text": "",
            }),
            None,
        )
        self.assertEqual(saved["statusCode"], 200)
        save_program = json.loads(saved["body"])["program"]
        self.assertEqual(save_program["chain"], [["legacy", "rev", "poly", "poly"]])

    @patch("handler_storage.s3")
    def test_validation_limits_and_missing_ids(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        self._patch_s3(mock_s3, fake_s3)

        for body in [
            {"name": "x" * 121, "chain": [["legacy", "rev", "poly", "poly"]]},
            {"name": "Many", "chain": [["legacy", "rev", "poly", "poly"]] * 257},
            {"name": "Token", "chain": [["macro", "x" * 257]]},
            {"name": "Broken", "chain": [["push", "cf"]]},
        ]:
            resp = handler_storage.handler(self._event("/save-coeff-program", body), None)
            self.assertEqual(resp["statusCode"], 400)

        fetch_missing = handler_storage.handler(self._event("/fetch-coeff-program", {"id": "missing"}), None)
        self.assertEqual(fetch_missing["statusCode"], 404)

        delete_missing = handler_storage.handler(self._event("/delete-coeff-program", {"id": "missing"}), None)
        self.assertEqual(delete_missing["statusCode"], 404)


if __name__ == "__main__":
    unittest.main()
