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


def _event(path, body):
    return {"path": path, "body": json.dumps(body)}


def _patch_s3(mock_s3, fake_s3):
    mock_s3.get_object.side_effect = fake_s3.get_object
    mock_s3.put_object.side_effect = fake_s3.put_object
    mock_s3.head_object.side_effect = fake_s3.head_object
    mock_s3.delete_object.side_effect = fake_s3.delete_object


class TestProgramV2Migration(unittest.TestCase):
    @patch("handler_storage.s3")
    def test_migrate_param_program_dry_run_and_write(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        _patch_s3(mock_s3, fake_s3)
        handler_storage.handler(_event("/save-param-program", {
            "name": "Param V1",
            "source_text": "p1 = t1 + t2\np2 = t1 - t2",
        }), None)

        dry = handler_storage.handler(_event("/migrate-param-program", {"id": "param-v1", "dry_run": True}), None)
        self.assertEqual(dry["statusCode"], 200)
        dry_body = json.loads(dry["body"])
        self.assertFalse(dry_body["wrote"])
        self.assertEqual(dry_body["kind"], "param")
        self.assertEqual(dry_body["migrated"]["spec_version"], 2)
        self.assertEqual(dry_body["migrated"]["program_version"], 2)
        self.assertTrue(dry_body["migrated"]["fingerprint"].startswith("sha256:"))
        self.assertEqual(dry_body["migrated"]["tokens"][0]["op"], 31)  # push_t1 in merged Param range
        self.assertIn(23, [tok["op"] for tok in dry_body["migrated"]["tokens"]])  # typed_binary
        self.assertIn('"version":2', dry_body["migrated"]["execution_spec"])
        self.assertNotIn("polypaint/param-programs/v2/param-v1.json", fake_s3.objects)

        write = handler_storage.handler(_event("/migrate-param-program", {"id": "param-v1", "dry_run": False}), None)
        self.assertEqual(write["statusCode"], 200)
        write_body = json.loads(write["body"])
        self.assertTrue(write_body["wrote"])
        stored = json.loads(fake_s3.objects["polypaint/param-programs/v2/param-v1.json"].decode("utf-8"))
        self.assertEqual(stored["fingerprint"], write_body["migrated"]["fingerprint"])

        same = handler_storage.handler(_event("/migrate-param-program", {"id": "param-v1", "dry_run": False}), None)
        self.assertEqual(same["statusCode"], 200)
        self.assertFalse(json.loads(same["body"])["wrote"])

    @patch("handler_storage.s3")
    def test_resave_after_migration_drops_stale_v2_copy(self, mock_s3):
        # H2: fetch prefers the v2 key, so a re-save that leaves the migrated
        # copy in place shadows every later edit forever and re-migration
        # 409s on the conflict. A v1 re-save must invalidate the v2 copy.
        import handler_storage

        fake_s3 = _FakeS3()
        _patch_s3(mock_s3, fake_s3)
        handler_storage.handler(_event("/save-param-program", {
            "name": "Life",
            "source_text": "p1 = t1 + t2",
        }), None)
        handler_storage.handler(_event("/migrate-param-program", {"id": "life", "dry_run": False}), None)
        self.assertIn("polypaint/param-programs/v2/life.json", fake_s3.objects)

        handler_storage.handler(_event("/save-param-program", {
            "name": "Life",
            "source_text": "p1 = t1 * t2",
        }), None)
        self.assertNotIn(
            "polypaint/param-programs/v2/life.json",
            fake_s3.objects,
            "re-save must drop the stale migrated v2 copy",
        )
        fetched = handler_storage.handler(_event("/fetch-param-program", {"id": "life"}), None)
        self.assertEqual(fetched["statusCode"], 200)
        self.assertIn("t1*t2", json.dumps(json.loads(fetched["body"])))

        # And re-migration succeeds again on the fresh save (no 409).
        remigrate = handler_storage.handler(_event("/migrate-param-program", {"id": "life", "dry_run": False}), None)
        self.assertEqual(remigrate["statusCode"], 200)
        self.assertTrue(json.loads(remigrate["body"])["wrote"])

    @patch("handler_storage.s3")
    def test_delete_removes_migrated_v2_copy_too(self, mock_s3):
        # H2: deleting only the v1 key left a v2 zombie that fetch kept
        # serving while list hid it.
        import handler_storage

        fake_s3 = _FakeS3()
        _patch_s3(mock_s3, fake_s3)
        handler_storage.handler(_event("/save-param-program", {
            "name": "Gone",
            "source_text": "p1 = t1",
        }), None)
        handler_storage.handler(_event("/migrate-param-program", {"id": "gone", "dry_run": False}), None)

        deleted = handler_storage.handler(_event("/delete-param-program", {"id": "gone"}), None)
        self.assertEqual(deleted["statusCode"], 200)
        self.assertEqual(json.loads(deleted["body"])["deleted"], 2)
        self.assertEqual(
            [k for k in fake_s3.objects if "param-programs" in k],
            [],
            "delete must remove both the v1 and the migrated v2 keys",
        )
        fetched = handler_storage.handler(_event("/fetch-param-program", {"id": "gone"}), None)
        self.assertEqual(fetched["statusCode"], 404)

    def test_translate_param_program_compiles_authoritative_source_text(self):
        from program_v2_translate import translate_param_from_old

        migrated = translate_param_from_old({
            "chain": [["const", "1"], ["emit", "p1"]],
            "source_text": "p1 = t2",
        })

        self.assertEqual(migrated["chain"], [["const", "t2"], ["emit", "p1"]])

    def test_translate_param_program_fingerprint_ignores_source_spelling(self):
        from program_v2_translate import _v2_fingerprint, translate_param_from_old

        first = translate_param_from_old({"source_text": "p1 = t1 + t2"})
        second = translate_param_from_old({"source_text": "p1=t1+t2"})

        self.assertEqual(first["execution_spec"], second["execution_spec"])
        self.assertEqual(first["fingerprint"], second["fingerprint"])
        self.assertEqual(
            _v2_fingerprint("param", {"execution_spec": "x", "source_text": "a"}),
            _v2_fingerprint("param", {"execution_spec": "x", "source_text": "b"}),
        )

    @patch("handler_storage.s3")
    def test_migrate_param_program_conflict_and_missing_macro(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        _patch_s3(mock_s3, fake_s3)
        handler_storage.handler(_event("/save-param-program", {
            "name": "Base Param",
            "chain": [["push", "t1"], ["emit", "p1"]],
        }), None)
        handler_storage.handler(_event("/save-param-program", {
            "name": "Uses Macro",
            "chain": [["macro", "base-param"]],
        }), None)

        missing = handler_storage.handler(_event("/migrate-param-program", {"id": "uses-macro", "dry_run": False}), None)
        self.assertEqual(missing["statusCode"], 422)
        self.assertEqual(json.loads(missing["body"])["missing"], ["base-param"])

        fake_s3.put_object(
            Key="polypaint/param-programs/v2/base-param.json",
            Body=json.dumps({"fingerprint": "sha256:base"}).encode("utf-8"),
            Metadata={},
        )
        fake_s3.put_object(
            Key="polypaint/param-programs/v2/uses-macro.json",
            Body=json.dumps({"fingerprint": "sha256:different"}).encode("utf-8"),
            Metadata={},
        )
        conflict = handler_storage.handler(_event("/migrate-param-program", {"id": "uses-macro", "dry_run": False}), None)
        self.assertEqual(conflict["statusCode"], 409)
        self.assertEqual(json.loads(conflict["body"])["existing_fingerprint"], "sha256:different")

    @patch("handler_storage.s3")
    def test_migrate_param_program_requires_transitive_macros_to_be_v2(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        _patch_s3(mock_s3, fake_s3)
        handler_storage.handler(_event("/save-param-program", {
            "name": "Leaf Param",
            "chain": [["push", "t1"], ["emit", "p1"]],
        }), None)
        handler_storage.handler(_event("/save-param-program", {
            "name": "Base Param",
            "chain": [["macro", "leaf-param"]],
        }), None)
        handler_storage.handler(_event("/save-param-program", {
            "name": "Uses Macro",
            "chain": [["macro", "base-param"]],
        }), None)
        fake_s3.put_object(
            Key="polypaint/param-programs/v2/base-param.json",
            Body=json.dumps({"fingerprint": "sha256:base"}).encode("utf-8"),
            Metadata={},
        )

        missing = handler_storage.handler(_event("/migrate-param-program", {"id": "uses-macro", "dry_run": False}), None)
        self.assertEqual(missing["statusCode"], 422)
        self.assertEqual(json.loads(missing["body"])["missing"], ["leaf-param"])

    @patch("handler_storage.s3")
    def test_migrate_coeff_program_dry_run_renders_source_text(self, mock_s3):
        import handler_storage
        from coeff_program_source import parse_coeff_program_source

        fake_s3 = _FakeS3()
        _patch_s3(mock_s3, fake_s3)
        handler_storage.handler(_event("/save-coeff-program", {
            "name": "Coeff V1",
            "chain": [["push", "cf"], ["legacy", "rev", "pop", "push"], ["emit"]],
        }), None)

        resp = handler_storage.handler(_event("/migrate-coeff-program", {"id": "coeff-v1", "dry_run": True}), None)
        self.assertEqual(resp["statusCode"], 200)
        body = json.loads(resp["body"])
        self.assertFalse(body["wrote"])
        self.assertEqual(body["migrated"]["spec_version"], 2)
        self.assertIn("source_text", body["migrated"])
        self.assertIn("rev", body["migrated"]["source_text"])
        native_tokens = [tok for tok in body["migrated"]["tokens"] if tok["op"] == 29]
        self.assertEqual(native_tokens[0]["registry"], "coeff")
        self.assertIn('"version":2', body["migrated"]["execution_spec"])
        reparsed = parse_coeff_program_source(body["migrated"]["source_text"])
        self.assertTrue(reparsed["chain"])

    def test_translate_coeff_program_compiles_authoritative_source_text(self):
        from program_v2_translate import translate_coeff_from_old

        migrated = translate_coeff_from_old({
            "chain": [["push", "cf"], ["emit"]],
            "source_text": "poly = _native_transform(rev, poly, poly)",
        })

        self.assertEqual(migrated["chain"], [["_native_transform", "rev", "poly", "poly"]])

    def test_translate_coeff_program_fingerprint_ignores_source_spelling(self):
        from program_v2_translate import _v2_fingerprint, translate_coeff_from_old

        first = translate_coeff_from_old({"source_text": "poly = cf"})
        second = translate_coeff_from_old({"source_text": "poly=cf"})

        self.assertEqual(first["execution_spec"], second["execution_spec"])
        self.assertEqual(first["fingerprint"], second["fingerprint"])
        self.assertEqual(
            _v2_fingerprint("coeff", {"execution_spec": "x", "source_display": "a"}),
            _v2_fingerprint("coeff", {"execution_spec": "x", "source_display": "b"}),
        )

    @patch("handler_storage.s3")
    def test_migrate_solve_score_program_dry_run_has_source_text(self, mock_s3):
        import handler_storage

        fake_s3 = _FakeS3()
        _patch_s3(mock_s3, fake_s3)
        handler_storage.handler(_event("/save-solve-score-program", {
            "name": "Score V1",
            "chain": [["proximity", "0.1"], ["emit", "norm"]],
        }), None)

        resp = handler_storage.handler(_event("/migrate-solve-score-program", {"id": "score-v1", "dry_run": True}), None)
        self.assertEqual(resp["statusCode"], 200)
        migrated = json.loads(resp["body"])["migrated"]
        self.assertEqual(migrated["spec_version"], 2)
        self.assertIn("program_spec", migrated)
        self.assertIn("source_text", migrated)
        self.assertEqual(
            migrated["source_text"],
            "push(metric(proximity, slv, q=0.1%))\nemit_norm()",
        )
        self.assertTrue(migrated["program_spec"].startswith("v2;"))
        self.assertIn(64, [tok["op"] for tok in migrated["tokens"]])  # reduce_metric
        self.assertIn(65, [tok["op"] for tok in migrated["tokens"]])  # push_metric
        self.assertIn(77, [tok["op"] for tok in migrated["tokens"]])  # emit_norm
        self.assertIn('"version":2', migrated["execution_spec"])

    def test_translate_solve_score_program_fail_closed_when_source_roundtrip_fails(self):
        from program_v2_translate import translate_solve_score_from_old

        with patch(
            "program_v2_translate.solve_score_source_text_from_chain",
            side_effect=RuntimeError("boom"),
        ):
            migrated = translate_solve_score_from_old({
                "id": "score-v1",
                "name": "Score V1",
                "chain": [["proximity", "0.1"], ["emit", "norm"]],
            })

        self.assertEqual(migrated["source_text"], "")
        self.assertEqual(migrated["source_display"], "")
        self.assertEqual(migrated["chain"], [["proximity", "0.1"], ["emit", "norm"]])
        self.assertTrue(migrated["program_spec"].startswith("v2;"))
        self.assertEqual(migrated["diagnostics"][0]["code"], "source_roundtrip_failed")
        self.assertIn("boom", migrated["diagnostics"][0]["message"])

    def test_translate_solve_score_program_requires_non_empty_chain(self):
        from program_v2_translate import translate_solve_score_from_old

        for payload in ({}, {"chain": []}, {"chain": ""}, None):
            with self.subTest(payload=payload):
                with self.assertRaisesRegex(RuntimeError, "requires a non-empty chain"):
                    translate_solve_score_from_old(payload)

    def test_root_translator_stays_deleted(self):
        # translate_root_from_old (and its private helpers) had zero
        # production callers — root programs never shipped a v1→v2 payload
        # migration path. Pin the deletion so it doesn't quietly return.
        import program_v2_translate as translate

        for name in ("translate_root_from_old", "_root_transform_items", "_root_token_from_item"):
            self.assertFalse(hasattr(translate, name), name)


if __name__ == "__main__":
    unittest.main()
