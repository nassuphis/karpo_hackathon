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

    @patch("handler_storage.s3")
    def test_migrate_solve_score_program_dry_run_has_no_source_text(self, mock_s3):
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
        self.assertTrue(migrated["program_spec"].startswith("v2;"))
        self.assertIn(64, [tok["op"] for tok in migrated["tokens"]])  # reduce_metric
        self.assertIn(65, [tok["op"] for tok in migrated["tokens"]])  # push_metric
        self.assertIn(77, [tok["op"] for tok in migrated["tokens"]])  # emit_norm
        self.assertIn('"version":2', migrated["execution_spec"])
        self.assertNotIn("source_text", migrated)

    def test_translate_root_transforms_from_old_uses_registry_indices(self):
        from program_v2_translate import translate_root_from_old

        migrated = translate_root_from_old({
            "root_transforms": [
                ["rotate_roots", "0.25"],
                {"name": "mul_complex", "args": [0, 1]},
                ["unknown_root_op", "1"],
            ]
        })

        self.assertEqual(migrated["spec_version"], 2)
        self.assertEqual([tok["registry"] for tok in migrated["tokens"]], ["root", "root"])
        self.assertEqual([tok["fn_index"] for tok in migrated["tokens"]], [1, 7])
        self.assertEqual(migrated["tokens"][0]["op"], 29)
        self.assertEqual(migrated["tokens"][1]["args"], [0.0, 1.0])
        self.assertEqual(migrated["diagnostics"][0]["level"], "warning")
        self.assertIn("unknown_root_op", migrated["diagnostics"][0]["message"])


if __name__ == "__main__":
    unittest.main()
