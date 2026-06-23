import io
import json
import os
import sys
import unittest
from unittest.mock import patch

from botocore.exceptions import ClientError


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(path, body):
    return {"path": path, "body": json.dumps(body)}


class TestStorageHandlerHardening(unittest.TestCase):
    @patch("handler_storage.s3")
    def test_cleanup_rejects_non_render_keys_before_s3_delete(self, mock_s3):
        import handler_storage

        resp = handler_storage.handler(
            _event("/cleanup", {"keys": ["index.html", "js/01-core-compute.js"]}),
            None,
        )

        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("cleanup key not allowed", json.loads(resp["body"])["error"])
        mock_s3.delete_objects.assert_not_called()

    @patch("handler_storage.s3")
    def test_cleanup_counts_deleted_objects_and_reports_s3_errors(self, mock_s3):
        import handler_storage

        mock_s3.delete_objects.return_value = {
            "Deleted": [{"Key": "renders/j/stripe_0.raw"}],
            "Errors": [{"Key": "renders/j/merge_0.raw", "Code": "AccessDenied"}],
        }

        resp = handler_storage.handler(
            _event("/cleanup", {
                "keys": [
                    "renders/j/stripe_0.raw",
                    "renders/j/merge_0.raw",
                    "renders/j/deepzoom_latest.json",
                ],
            }),
            None,
        )
        body = json.loads(resp["body"])

        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(body["deleted"], 1)
        self.assertEqual(body["errors"], ["renders/j/merge_0.raw: AccessDenied"])
        delete_payload = mock_s3.delete_objects.call_args.kwargs["Delete"]
        self.assertNotIn("Quiet", delete_payload)

    @patch("handler_storage.s3")
    def test_unwrapped_routes_now_return_error_envelope(self, mock_s3):
        import handler_storage

        resp = handler_storage.handler(_event("/detail", {}), None)

        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("job_id", json.loads(resp["body"])["error"])

    @patch("handler_storage.s3")
    def test_storage_client_errors_are_json_enveloped(self, mock_s3):
        import handler_storage

        mock_s3.generate_presigned_url.side_effect = ClientError(
            {"Error": {"Code": "AccessDenied", "Message": "denied"}},
            "GeneratePresignedUrl",
        )

        resp = handler_storage.handler(
            _event("/presign", {"key": "renders/j/image.jpeg"}),
            None,
        )

        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("AccessDenied", json.loads(resp["body"])["error"])


class _FakeS3:
    """Stateful S3 double covering the calls handle_detail / migrate make."""

    def __init__(self):
        self.objects = {}

    def get_paginator(self, name):
        outer = self

        class _P:
            def paginate(self, Bucket=None, Prefix=None, Delimiter=None):
                keys = [k for k in outer.objects if k.startswith(Prefix or "")]
                if Delimiter:
                    prefixes = set()
                    for key in keys:
                        rest = key[len(Prefix or ""):]
                        head = rest.split(Delimiter, 1)[0]
                        if head:
                            prefixes.add((Prefix or "") + head + Delimiter)
                    return [{"KeyCount": len(keys), "CommonPrefixes": [{"Prefix": p} for p in sorted(prefixes)]}]
                return [{"KeyCount": len(keys), "Contents": [{"Key": k} for k in keys]}]

        return _P()

    def get_object(self, Bucket=None, Key=None, **kwargs):
        if Key not in self.objects:
            raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")
        return {"Body": io.BytesIO(self.objects[Key])}

    def put_object(self, Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None):
        self.objects[Key] = Body if isinstance(Body, bytes) else str(Body or "").encode("utf-8")
        return {}

    def head_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise ClientError({"Error": {"Code": "404", "Message": "missing"}}, "HeadObject")
        return {}


_CALC_V1 = {
    "function": "g1",
    "times": 1,
    "degree": 1,
    "pipeline": {
        "function": "g1",
        "param_transforms": [["unit_circle"]],
        "coeff_transforms": [["rev"], ["conj"]],
        "cfpv": [],
    },
}

_CALC_V2 = {
    "function": "g1",
    "times": 1,
    "pipeline": {
        "function": "g1",
        "param_transforms": [],
        "coeff_transforms": [],
        "param_program": {"version": 2, "tokens": [["set", "x", "p"]], "fingerprint": "a"},
        "coeff_program": {"version": 2, "tokens": [["set", "poly", "cf"]], "fingerprint": "b"},
        "cfpv": [],
    },
}


class TestComputeMigration(unittest.TestCase):
    def _patch(self, mock_s3, fake):
        mock_s3.get_paginator.side_effect = fake.get_paginator
        mock_s3.get_object.side_effect = fake.get_object
        mock_s3.put_object.side_effect = fake.put_object
        mock_s3.head_object.side_effect = fake.head_object

    def _store(self, fake, job_id, calc):
        fake.objects[f"renders/{job_id}/calc.json"] = json.dumps(calc).encode("utf-8")

    @patch("handler_storage._results_list_s3_client")
    @patch("handler_storage.s3")
    def test_list_skips_preview_orphan_prefix_without_calc_json(self, mock_s3, mock_list_s3_client):
        # Lazy Results preview writes renders/<job>/preview.png. If a compute is
        # deleted while that background write is in flight, a preview-only
        # prefix must not reappear as a broken Results row.
        import handler_storage

        fake = _FakeS3()
        self._patch(mock_s3, fake)
        mock_list_s3_client.return_value = fake
        self._store(fake, "good", _CALC_V2)
        fake.objects["renders/orphan/preview.png"] = b"png"

        resp = handler_storage.handler(_event("/list", {"list_workers": 1}), None)
        body = json.loads(resp["body"])

        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual([row["job_id"] for row in body["results"]], ["good"])
        self.assertEqual(body["count"], 1)
        self.assertEqual(body["skipped_missing_calc"], 1)
        self.assertEqual(body["metadata_error_count"], 0)

    @patch("handler_storage.s3")
    def test_detail_reports_legacy_run_as_v1_migratable(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        self._patch(mock_s3, fake)
        self._store(fake, "jv1", _CALC_V1)

        body = json.loads(handler_storage.handler(_event("/detail", {"job_id": "jv1"}), None)["body"])
        self.assertEqual(body["pipeline_program_version"], 1)
        self.assertTrue(body["pipeline_migratable"])

    @patch("handler_storage.s3")
    def test_detail_reports_program_run_as_v2_not_migratable(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        self._patch(mock_s3, fake)
        self._store(fake, "jv2", _CALC_V2)

        body = json.loads(handler_storage.handler(_event("/detail", {"job_id": "jv2"}), None)["body"])
        self.assertEqual(body["pipeline_program_version"], 2)
        self.assertFalse(body["pipeline_migratable"])

    @patch("handler_storage.s3")
    def test_detail_reconstructs_readable_coeff_source_for_chain_only_result(self, mock_s3):
        # Older results store the coeff program only as a lowered _typed_* chain
        # with no coeff_program_source_text; Populate then synthesized unparseable
        # _typed_* source. /detail now reconstructs readable, equivalent source.
        import handler_storage
        from coeff_program_source import compile_coeff_program_source

        fake = _FakeS3()
        self._patch(mock_s3, fake)
        calc = {
            "function": "const",
            "pipeline": {
                "function": "const",
                "coeff_program_chain": [
                    ["_typed_push_scalar", "0.0+0.0j"],
                    ["_typed_push_scalar", "1.0+0.0j"],
                    ["_typed_poke_poly"],
                    ["_typed_push_scalar", "19.0+0.0j"],
                    ["_typed_push_scalar", "9.0+0.0j"],
                    ["_typed_unary", "neg"],
                    ["_typed_push_scalar", "p1"],
                    ["_typed_push_scalar", "1000.0+0.0j"],
                    ["_typed_binary", "multiply"],
                    ["_typed_binary", "add"],
                    ["_typed_poke_poly"],
                ],
            },
        }
        self._store(fake, "jrecon", calc)

        detail = json.loads(handler_storage.handler(_event("/detail", {"job_id": "jrecon"}), None)["body"])
        src = detail.get("coeff_program_source_text")
        self.assertTrue(src and src.strip(), "detail should reconstruct coeff source from a chain-only result")
        self.assertNotIn("_typed_push_scalar", src)  # readable, not raw VM tokens
        # ...and the reconstructed source is the SAME program as the raw chain.
        raw = (
            "_typed_push_scalar(0.0+0.0j)\n_typed_push_scalar(1.0+0.0j)\n_typed_poke_poly\n"
            "_typed_push_scalar(19.0+0.0j)\n_typed_push_scalar(9.0+0.0j)\n_typed_unary(neg)\n"
            "_typed_push_scalar(p1)\n_typed_push_scalar(1000.0+0.0j)\n"
            "_typed_binary(multiply)\n_typed_binary(add)\n_typed_poke_poly\n"
        )
        self.assertEqual(
            compile_coeff_program_source(src)["fingerprint"],
            compile_coeff_program_source(raw)["fingerprint"],
        )

    @patch("handler_storage.s3")
    def test_detail_translates_legacy_param_transforms_for_populate(self, mock_s3):
        # A v1 result with only legacy param_transforms (no program chain) — e.g.
        # function poly_42_serendipity with param_transforms unit_circle, exp.
        # Populate forces program mode, so detail must translate the transforms
        # into a program chain + readable source or the editor shows nothing.
        import handler_storage
        from param_program_chain import compile_param_program_chain
        from param_program_source import compile_param_program_source
        from pipeline_programs import param_transforms_to_program_chain

        fake = _FakeS3()
        self._patch(mock_s3, fake)
        calc = {
            "function": "poly_42_serendipity",
            "pipeline": {
                "function": "poly_42_serendipity",
                "param_transforms": [["unit_circle"], ["exp"]],
                "coeff_transforms": [],
            },
        }
        self._store(fake, "jlegacyp", calc)

        detail = json.loads(handler_storage.handler(_event("/detail", {"job_id": "jlegacyp"}), None)["body"])
        self.assertEqual(detail["pipeline_program_version"], 1)
        chain = detail.get("param_program_chain")
        src = detail.get("param_program_source_text")
        self.assertTrue(chain, "detail should translate legacy param transforms to a program chain")
        self.assertTrue(src and src.strip(), "detail should reconstruct readable param source for Populate")
        # ...and the translated program is equivalent to the legacy transforms.
        expected_fp = compile_param_program_chain(
            param_transforms_to_program_chain([["unit_circle"], ["exp"]])
        )["fingerprint"]
        self.assertEqual(compile_param_program_source(src)["fingerprint"], expected_fp)
        # The poly function has no coeff program; nothing is fabricated there.
        self.assertNotIn("coeff_program_source_text", detail)
        self.assertNotIn("coeff_program_chain", detail)

    @patch("handler_storage.s3")
    def test_detail_keeps_stored_coeff_source_when_present(self, mock_s3):
        # Stored source text always wins over reconstruction.
        import handler_storage

        fake = _FakeS3()
        self._patch(mock_s3, fake)
        calc = {
            "function": "g1",
            "pipeline": {
                "function": "g1",
                "coeff_program_source_text": "poly = rev(cf)\nemit\n",
                "coeff_program_chain": [["_typed_push_scalar", "0.0+0.0j"], ["_typed_poke_poly"]],
            },
        }
        self._store(fake, "jstored", calc)

        detail = json.loads(handler_storage.handler(_event("/detail", {"job_id": "jstored"}), None)["body"])
        # No top-level reconstruction override; the stored source stays authoritative.
        self.assertNotIn("coeff_program_source_text", detail)
        self.assertEqual(detail["pipeline"]["coeff_program_source_text"], "poly = rev(cf)\nemit\n")

    @patch("handler_storage.s3")
    def test_migrate_compute_translates_legacy_calc_in_place(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        self._patch(mock_s3, fake)
        self._store(fake, "jmig", _CALC_V1)

        resp = handler_storage.handler(
            _event("/migrate-compute", {"job_id": "jmig", "dry_run": False}), None
        )
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        self.assertTrue(body["wrote"])
        self.assertEqual(body["from_version"], 1)
        self.assertEqual(body["to_version"], 2)
        self.assertFalse(body["already_current"])

        stored = json.loads(fake.objects["renders/jmig/calc.json"].decode("utf-8"))
        pl = stored["pipeline"]
        self.assertEqual(pl["param_transforms"], [])
        self.assertEqual(pl["coeff_transforms"], [])
        self.assertTrue(pl["param_program"].get("tokens"))
        self.assertTrue(pl["coeff_program"].get("tokens"))
        self.assertTrue(pl["param_program_chain"])
        self.assertEqual(pl["param_transforms_display"], [["unit_circle"]])  # original chain preserved

        # Idempotent: a second migrate is a no-op.
        body2 = json.loads(
            handler_storage.handler(_event("/migrate-compute", {"job_id": "jmig", "dry_run": False}), None)["body"]
        )
        self.assertFalse(body2["wrote"])
        self.assertTrue(body2["already_current"])
        self.assertEqual(body2["from_version"], 2)

    @patch("handler_storage.s3")
    def test_migrate_compute_dry_run_does_not_write(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        self._patch(mock_s3, fake)
        self._store(fake, "jdry", _CALC_V1)

        body = json.loads(
            handler_storage.handler(_event("/migrate-compute", {"job_id": "jdry", "dry_run": True}), None)["body"]
        )
        self.assertFalse(body["wrote"])
        self.assertEqual(body["from_version"], 1)
        self.assertFalse(body["already_current"])

        stored = json.loads(fake.objects["renders/jdry/calc.json"].decode("utf-8"))
        self.assertEqual(stored["pipeline"]["param_transforms"], [["unit_circle"]])  # untouched
        self.assertNotIn("param_program", stored["pipeline"])

    @patch("handler_storage.s3")
    def test_migrate_compute_missing_job_returns_404(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        self._patch(mock_s3, fake)

        resp = handler_storage.handler(
            _event("/migrate-compute", {"job_id": "ghost", "dry_run": False}), None
        )
        self.assertEqual(resp["statusCode"], 404)
        self.assertIn("ghost", json.loads(resp["body"])["error"])

    @patch("handler_storage.s3")
    def test_migrate_compute_requires_job_id(self, mock_s3):
        import handler_storage

        resp = handler_storage.handler(_event("/migrate-compute", {}), None)
        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("job_id", json.loads(resp["body"])["error"])


if __name__ == "__main__":
    unittest.main()
