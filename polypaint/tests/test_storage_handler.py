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
        self.metadata = {}
        self.deleted = []

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

    def put_object(self, Bucket=None, Key=None, Body=None, ContentType=None, Metadata=None, CacheControl=None):
        self.objects[Key] = Body if isinstance(Body, bytes) else str(Body or "").encode("utf-8")
        self.metadata[Key] = {
            "ContentType": ContentType or "",
            "Metadata": Metadata or {},
            "CacheControl": CacheControl or "",
        }
        return {}

    def head_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise ClientError({"Error": {"Code": "404", "Message": "missing"}}, "HeadObject")
        meta = self.metadata.get(Key, {})
        return {
            "ContentLength": len(self.objects.get(Key, b"")),
            "ContentType": meta.get("ContentType", ""),
            "Metadata": meta.get("Metadata", {}),
            "LastModified": meta.get("LastModified"),
        }

    def delete_objects(self, Bucket=None, Delete=None):
        deleted = []
        for obj in (Delete or {}).get("Objects", []):
            key = obj.get("Key")
            if key in self.objects:
                del self.objects[key]
            self.metadata.pop(key, None)
            self.deleted.append(key)
            deleted.append({"Key": key})
        return {"Deleted": deleted}

    def generate_presigned_url(self, method, Params=None, ExpiresIn=None):
        return "https://example.test/" + (Params or {}).get("Key", "")


class _FakeDDB:
    def __init__(self):
        self.items = {}
        self.put_calls = []

    def _key(self, item_or_key):
        return (
            item_or_key["job_id"]["S"],
            item_or_key["task_id"]["S"],
        )

    def _plain(self, attr):
        if "S" in attr:
            return attr["S"]
        if "N" in attr:
            raw = attr["N"]
            value = float(raw)
            return int(value) if value.is_integer() else value
        if "M" in attr:
            return {k: self._plain(v) for k, v in attr["M"].items()}
        if "BOOL" in attr:
            return bool(attr["BOOL"])
        if "NULL" in attr:
            return None
        if "L" in attr:
            return [self._plain(v) for v in attr["L"]]
        return None

    def _conditional_failed(self):
        return ClientError(
            {"Error": {"Code": "ConditionalCheckFailedException", "Message": "condition failed"}},
            "PutItem",
        )

    def get_item(self, TableName=None, Key=None, ConsistentRead=None):
        item = self.items.get(self._key(Key))
        return {"Item": item} if item else {}

    def put_item(self, TableName=None, Item=None, ConditionExpression=None, ExpressionAttributeNames=None, ExpressionAttributeValues=None):
        key = self._key(Item)
        existing = self.items.get(key)
        if ConditionExpression and existing is not None:
            state = self._plain(existing.get("state", {"S": ""}))
            updated_at_ms = self._plain(existing.get("updated_at_ms", {"N": "0"}))
            refresh_id = self._plain(existing.get("refresh_id", {"S": ""}))
            values = ExpressionAttributeValues or {}
            if "attribute_not_exists(job_id)" in ConditionExpression:
                computing = self._plain(values[":computing"])
                stale_before = self._plain(values[":stale_before"])
                if not (state != computing or updated_at_ms < stale_before):
                    raise self._conditional_failed()
            elif "refresh_id = :refresh_id" in ConditionExpression:
                wanted_refresh = self._plain(values[":refresh_id"])
                computing = self._plain(values[":computing"])
                if not (refresh_id == wanted_refresh and state == computing):
                    raise self._conditional_failed()
            else:
                raise AssertionError(f"unsupported fake condition: {ConditionExpression}")
        self.items[key] = Item
        self.put_calls.append((key, Item, ConditionExpression))
        return {}

    def update_item(self, *args, **kwargs):
        raise AssertionError("mosaic status must use conditional put_item, not update_item")


class _FakeLambdaClient:
    def __init__(self, exc=None):
        self.invocations = []
        self.exc = exc

    def invoke(self, **kwargs):
        self.invocations.append(kwargs)
        if self.exc:
            raise self.exc
        return {"StatusCode": 202}


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
    def test_render_count_counts_displayable_color_artifacts_and_legacy(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        self._patch(mock_s3, fake)
        fake.objects["renders/job/color/color_a/image.jpeg"] = b"jpeg"
        fake.objects["renders/job/color/color_b/image.png"] = b"png"
        fake.objects["renders/job/color/incomplete/preview.png"] = b"preview"
        fake.objects["renders/job/image.jpeg"] = b"legacy"

        resp = handler_storage.handler(_event("/render-count", {"job_id": "job"}), None)
        body = json.loads(resp["body"])

        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(body["job_id"], "job")
        self.assertEqual(body["family"], "color")
        self.assertEqual(body["color_artifact_count"], 2)
        self.assertEqual(body["legacy_color_artifact_count"], 1)
        self.assertEqual(body["color_render_count"], 3)

    @patch("handler_storage.s3")
    def test_render_count_requires_job_id(self, mock_s3):
        import handler_storage

        resp = handler_storage.handler(_event("/render-count", {}), None)
        body = json.loads(resp["body"])

        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("render-count requires job_id", body["error"])

    @patch("handler_storage._get_ddb")
    @patch("handler_storage.boto3.client")
    @patch.dict(os.environ, {"AWS_LAMBDA_FUNCTION_NAME": "polypaint-storage", "AWS_REGION": "us-east-1"})
    def test_color_mosaic_refresh_starts_async_worker_with_conditional_put(self, mock_boto_client, mock_get_ddb):
        import handler_storage

        fake_ddb = _FakeDDB()
        fake_lambda = _FakeLambdaClient()
        mock_get_ddb.return_value = fake_ddb
        mock_boto_client.return_value = fake_lambda

        resp = handler_storage.handler(_event("/list-color-mosaic", {"refresh": True}), None)
        body = json.loads(resp["body"])

        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(body["state"], "computing")
        self.assertTrue(body["refresh_id"].startswith("mosaic_"))
        self.assertEqual(len(fake_lambda.invocations), 1)
        payload = json.loads(fake_lambda.invocations[0]["Payload"].decode("utf-8"))
        self.assertEqual(payload["internal_action"], "build_color_mosaic")
        self.assertEqual(payload["refresh_id"], body["refresh_id"])
        self.assertEqual(len(fake_ddb.put_calls), 1)
        self.assertIn("attribute_not_exists(job_id)", fake_ddb.put_calls[0][2])

    @patch("handler_storage._get_ddb")
    @patch("handler_storage.boto3.client")
    @patch.dict(os.environ, {"AWS_LAMBDA_FUNCTION_NAME": "polypaint-storage", "AWS_REGION": "us-east-1"})
    def test_palette_mosaic_refresh_starts_async_worker_with_separate_status_row(self, mock_boto_client, mock_get_ddb):
        import handler_storage

        fake_ddb = _FakeDDB()
        fake_lambda = _FakeLambdaClient()
        mock_get_ddb.return_value = fake_ddb
        mock_boto_client.return_value = fake_lambda

        resp = handler_storage.handler(_event("/list-palette-mosaic", {"refresh": True}), None)
        body = json.loads(resp["body"])

        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(body["state"], "computing")
        self.assertTrue(body["refresh_id"].startswith("mosaic_"))
        self.assertEqual(fake_ddb.put_calls[0][0], ("__allrenders_mosaic__", "palette_mosaic_status"))
        self.assertEqual(len(fake_lambda.invocations), 1)
        payload = json.loads(fake_lambda.invocations[0]["Payload"].decode("utf-8"))
        self.assertEqual(payload["internal_action"], "build_palette_mosaic")
        self.assertEqual(payload["refresh_id"], body["refresh_id"])

    @patch("handler_storage._get_ddb")
    @patch("handler_storage.boto3.client")
    @patch.dict(os.environ, {"AWS_LAMBDA_FUNCTION_NAME": "polypaint-storage", "AWS_REGION": "us-east-1"})
    def test_color_mosaic_refresh_records_error_when_self_invoke_fails(self, mock_boto_client, mock_get_ddb):
        import handler_storage

        fake_ddb = _FakeDDB()
        fake_lambda = _FakeLambdaClient(exc=RuntimeError("invoke throttled"))
        mock_get_ddb.return_value = fake_ddb
        mock_boto_client.return_value = fake_lambda

        resp = handler_storage.handler(_event("/list-color-mosaic", {"refresh": True}), None)
        body = json.loads(resp["body"])
        current = handler_storage._read_mosaic_status()

        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(body["state"], "error")
        self.assertIn("invoke throttled", body["error"])
        self.assertEqual(current["state"], "error")
        self.assertEqual(current["refresh_id"], body["refresh_id"])

    @patch("handler_storage._results_list_s3_client")
    @patch("handler_storage._get_ddb")
    @patch("handler_storage.s3")
    def test_color_mosaic_worker_builds_manifest_and_ready_status(self, mock_s3, mock_get_ddb, mock_list_s3_client):
        import handler_storage

        fake = _FakeS3()
        fake_ddb = _FakeDDB()
        self._patch(mock_s3, fake)
        mock_list_s3_client.return_value = fake
        mock_get_ddb.return_value = fake_ddb
        self._store(fake, "job", {"function": "g", "degree": 7, "N": 512, "times": 2})
        fake.objects["renders/job/color/color_a/image.jpeg"] = b"jpeg"
        fake.objects["renders/job/color/color_a/preview.png"] = (
            b"\x89PNG\r\n\x1a\n" + (13).to_bytes(4, "big") + b"IHDR"
            + (512).to_bytes(4, "big") + (512).to_bytes(4, "big")
        )
        fake.objects["renders/job/image.jpeg"] = b"legacy"
        fake.metadata["renders/job/color/color_a/preview.png"] = {
            "Metadata": {"width": "4000", "height": "4000"},
            "ContentType": "image/png",
        }

        with patch("handler_storage.boto3.client", return_value=_FakeLambdaClient()):
            status = handler_storage._start_color_mosaic_refresh()
        ready = handler_storage._run_color_mosaic_worker(status["refresh_id"])

        self.assertEqual(ready["state"], "ready")
        self.assertEqual(ready["count"], 1)
        self.assertEqual(ready["source_counts"], {"512x512": 1})
        self.assertEqual(ready["skipped_legacy"], 1)
        progress_rows = [
            handler_storage._mosaic_status_from_item(item)
            for _, item, _ in fake_ddb.put_calls
            if handler_storage._mosaic_status_from_item(item).get("progress_stage")
        ]
        self.assertIn("jobs", {row["progress_stage"] for row in progress_rows})
        self.assertIn("artifacts", {row["progress_stage"] for row in progress_rows})
        self.assertIn("manifest", {row["progress_stage"] for row in progress_rows})
        self.assertTrue(any(row["progress_artifacts_total"] == 1 for row in progress_rows))
        self.assertTrue(any(row["progress_tiles"] == 1 for row in progress_rows))
        manifest_key = ready["manifest_key"]
        self.assertIn(manifest_key, fake.objects)
        manifest = json.loads(fake.objects[manifest_key])
        self.assertEqual(manifest["tiles"][0]["job_id"], "job")
        self.assertEqual(manifest["tiles"][0]["artifact_id"], "color_a")
        self.assertEqual(manifest["tiles"][0]["function"], "g")
        self.assertEqual(manifest["tiles"][0]["degree"], 7)
        self.assertEqual(manifest["tiles"][0]["N"], 512)
        self.assertEqual(manifest["tiles"][0]["times"], 2)
        self.assertEqual(manifest["tiles"][0]["preview_width"], 512)
        self.assertEqual(manifest["tiles"][0]["preview_height"], 512)
        self.assertEqual(manifest["manifest_type"], "artifact_mosaic")
        self.assertEqual(manifest["artifact_kind"], "color")
        self.assertEqual(manifest["sizes"], [512])
        self.assertEqual(manifest["size_counts"], {"512": 1})
        self.assertEqual(manifest["skipped_legacy"], 1)

    @patch("handler_storage._results_list_s3_client")
    @patch("handler_storage._get_ddb")
    @patch("handler_storage.s3")
    def test_palette_mosaic_worker_builds_manifest_and_ready_status(self, mock_s3, mock_get_ddb, mock_list_s3_client):
        import handler_storage

        fake = _FakeS3()
        fake_ddb = _FakeDDB()
        self._patch(mock_s3, fake)
        mock_list_s3_client.return_value = fake
        mock_get_ddb.return_value = fake_ddb
        self._store(fake, "job", {"function": "g", "degree": 7, "N": 512, "times": 2})
        fake.objects["renders/job/palettes/pal_a/meta.json"] = json.dumps({
            "palette_id": "pal_a",
            "metric": "spread",
            "palette": "reef",
            "render_reusable": "true",
            "data_layout": "raw",
            "image_key": "renders/job/palettes/pal_a/image.jpeg",
            "preview_key": "renders/job/palettes/pal_a/preview.png",
        }).encode("utf-8")
        fake.objects["renders/job/palettes/pal_a/image.jpeg"] = b"jpeg"
        fake.objects["renders/job/palettes/pal_a/preview.png"] = (
            b"\x89PNG\r\n\x1a\n" + (13).to_bytes(4, "big") + b"IHDR"
            + (512).to_bytes(4, "big") + (512).to_bytes(4, "big")
        )
        class _Stamp:
            def strftime(self, _fmt):
                return "2026-02-03T04:05:06Z"

        fake.metadata["renders/job/palettes/pal_a/preview.png"] = {"LastModified": _Stamp()}
        fake.objects["renders/job/palettes/pal_missing/image.jpeg"] = b"orphan"
        fake.objects["renders/job/palettes/pal_bad/meta.json"] = b"{not json"

        with patch("handler_storage.boto3.client", return_value=_FakeLambdaClient()):
            status = handler_storage._start_palette_mosaic_refresh()
        ready = handler_storage._run_palette_mosaic_worker(status["refresh_id"])
        current = handler_storage._read_mosaic_status("palette")

        self.assertEqual(ready["state"], "ready")
        self.assertEqual(current["state"], "ready")
        self.assertEqual(current["count"], 1)
        self.assertEqual(current["skipped_missing_meta"], 1)
        self.assertEqual(current["skipped_bad_meta"], 1)
        self.assertEqual(ready["source_counts"], {"512x512": 1})
        manifest = json.loads(fake.objects[ready["manifest_key"]])
        self.assertEqual(manifest["artifact_kind"], "palette")
        self.assertEqual(manifest["dimension_filter"], "preview-size")
        self.assertEqual(manifest["sizes"], [512])
        self.assertEqual(manifest["size_counts"], {"512": 1})
        self.assertEqual(manifest["skipped_missing_meta"], 1)
        self.assertEqual(manifest["skipped_bad_meta"], 1)
        self.assertEqual(manifest["tiles"][0]["job_id"], "job")
        self.assertEqual(manifest["tiles"][0]["artifact_id"], "pal_a")
        self.assertEqual(manifest["tiles"][0]["palette_id"], "pal_a")
        self.assertEqual(manifest["tiles"][0]["created_at"], "2026-02-03T04:05:06Z")
        self.assertEqual(manifest["tiles"][0]["metric"], "spread")
        self.assertEqual(manifest["tiles"][0]["palette"], "reef")
        self.assertEqual(manifest["tiles"][0]["render_reusable"], True)

    @patch("handler_storage._get_ddb")
    @patch("handler_storage.s3")
    def test_share_mosaic_snapshots_ready_manifest_and_returns_standalone_url(self, mock_s3, mock_get_ddb):
        import handler_storage

        fake = _FakeS3()
        fake_ddb = _FakeDDB()
        self._patch(mock_s3, fake)
        mock_get_ddb.return_value = fake_ddb
        manifest_key = "renders/_index/color_mosaic/mosaic_ready/all.json"
        manifest = {
            "schema_version": 1,
            "manifest_type": "artifact_mosaic",
            "artifact_kind": "color",
            "refresh_id": "mosaic_ready",
            "base": "https://bucket.test/",
            "count": 1,
            "tiles": [{"job_id": "job", "artifact_id": "color_a", "key": "renders/job/color/color_a/preview.png"}],
        }
        fake.objects[manifest_key] = json.dumps(manifest).encode("utf-8")
        handler_storage._put_mosaic_status({
            "state": "ready",
            "refresh_id": "mosaic_ready",
            "updated_at_ms": handler_storage._mosaic_now_ms(),
            "manifest_key": manifest_key,
            "manifest_url": handler_storage._s3_public_url(manifest_key),
            "count": 1,
        }, kind="color")

        resp = handler_storage.handler(_event("/share-mosaic", {
            "kind": "allcol",
            "size": "1024",
            "sort": "job",
            "cols": "7",
        }), None)
        body = json.loads(resp["body"])

        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(body["kind"], "color")
        self.assertEqual(body["size"], "1024")
        self.assertEqual(body["sort"], "job")
        self.assertEqual(body["cols"], "7")
        self.assertTrue(body["share_key"].startswith("renders/_shared_mosaic/color/share_"))
        self.assertIn("artifact_mosaic_viewer.html", body["share_url"])
        self.assertIn("kind=color", body["share_url"])
        self.assertIn("size=1024", body["share_url"])
        self.assertIn("sort=job", body["share_url"])
        self.assertIn("cols=7", body["share_url"])
        snapshot = json.loads(fake.objects[body["share_key"]])
        self.assertEqual(snapshot["source_manifest_key"], manifest_key)
        self.assertEqual(snapshot["share_id"], body["share_id"])
        self.assertEqual(snapshot["tiles"][0]["artifact_id"], "color_a")
        self.assertEqual(fake.metadata[body["share_key"]]["ContentType"], "application/json")

    @patch("handler_storage._results_list_s3_client")
    @patch("handler_storage._get_ddb")
    @patch("handler_storage.s3")
    def test_color_mosaic_worker_fails_visible_on_transient_calc_read_error(self, mock_s3, mock_get_ddb, mock_list_s3_client):
        import handler_storage

        fake = _FakeS3()
        fake_ddb = _FakeDDB()
        self._patch(mock_s3, fake)
        mock_list_s3_client.return_value = fake
        mock_get_ddb.return_value = fake_ddb
        self._store(fake, "job", {"function": "g"})
        fake.objects["renders/job/color/color_a/image.jpeg"] = b"jpeg"
        fake.objects["renders/job/color/color_a/preview.png"] = (
            b"\x89PNG\r\n\x1a\n" + (13).to_bytes(4, "big") + b"IHDR"
            + (512).to_bytes(4, "big") + (512).to_bytes(4, "big")
        )
        original_get_object = fake.get_object

        def get_object_with_slowdown(Bucket=None, Key=None, **kwargs):
            if Key == "renders/job/calc.json":
                raise ClientError({"Error": {"Code": "SlowDown", "Message": "slow"}}, "GetObject")
            return original_get_object(Bucket=Bucket, Key=Key, **kwargs)

        fake.get_object = get_object_with_slowdown

        with patch("handler_storage.boto3.client", return_value=_FakeLambdaClient()):
            status = handler_storage._start_color_mosaic_refresh()
        result = handler_storage._run_color_mosaic_worker(status["refresh_id"])

        self.assertEqual(result["state"], "error")
        self.assertIn("SlowDown", result["error"])

    @patch("handler_storage._results_list_s3_client")
    @patch("handler_storage._get_ddb")
    @patch("handler_storage.s3")
    def test_color_mosaic_worker_defaults_optional_bad_calc_json(self, mock_s3, mock_get_ddb, mock_list_s3_client):
        import handler_storage

        fake = _FakeS3()
        fake_ddb = _FakeDDB()
        self._patch(mock_s3, fake)
        mock_list_s3_client.return_value = fake
        mock_get_ddb.return_value = fake_ddb
        fake.objects["renders/job/calc.json"] = b"{not json"
        fake.objects["renders/job/color/color_a/image.jpeg"] = b"jpeg"
        fake.objects["renders/job/color/color_a/preview.png"] = (
            b"\x89PNG\r\n\x1a\n" + (13).to_bytes(4, "big") + b"IHDR"
            + (512).to_bytes(4, "big") + (512).to_bytes(4, "big")
        )

        with patch("handler_storage.boto3.client", return_value=_FakeLambdaClient()):
            status = handler_storage._start_color_mosaic_refresh()
        result = handler_storage._run_color_mosaic_worker(status["refresh_id"])
        manifest = json.loads(fake.objects[result["manifest_key"]])

        self.assertEqual(result["state"], "ready")
        self.assertEqual(manifest["tiles"][0]["function"], "?")
        self.assertEqual(manifest["tiles"][0]["degree"], 0)
        self.assertEqual(manifest["sizes"], [512])
        self.assertEqual(manifest["size_counts"], {"512": 1})

    @patch("handler_storage._results_list_s3_client")
    @patch("handler_storage._get_ddb")
    @patch("handler_storage.s3")
    def test_color_mosaic_prune_keeps_previous_ready_manifest(self, mock_s3, mock_get_ddb, mock_list_s3_client):
        import handler_storage

        fake = _FakeS3()
        fake_ddb = _FakeDDB()
        self._patch(mock_s3, fake)
        mock_list_s3_client.return_value = fake
        mock_get_ddb.return_value = fake_ddb
        self._store(fake, "job", {"function": "g"})
        fake.objects["renders/job/color/color_a/image.jpeg"] = b"jpeg"
        fake.objects["renders/job/color/color_a/preview.png"] = (
            b"\x89PNG\r\n\x1a\n" + (13).to_bytes(4, "big") + b"IHDR"
            + (512).to_bytes(4, "big") + (512).to_bytes(4, "big")
        )
        fake.objects["renders/_index/color_mosaic/aaa_previous/all.json"] = b"previous"
        fake.objects["renders/_index/color_mosaic/current/all.json"] = b"current"
        for idx in range(12):
            fake.objects[f"renders/_index/color_mosaic/zzz_{idx:02d}/all.json"] = b"old"

        now = handler_storage._utc_now_iso()
        handler_storage._put_mosaic_status({
            "state": "computing",
            "refresh_id": "current",
            "started_at": now,
            "updated_at": now,
            "updated_at_ms": handler_storage._mosaic_now_ms(),
            "last_ready_manifest_key": "renders/_index/color_mosaic/aaa_previous/all.json",
            "last_ready_manifest_url": "https://example.test/aaa_previous/all.json",
        })

        result = handler_storage._run_color_mosaic_worker("current")

        self.assertEqual(result["state"], "ready")
        self.assertIn("renders/_index/color_mosaic/aaa_previous/all.json", fake.objects)
        self.assertIn("renders/_index/color_mosaic/current/all.json", fake.objects)

    @patch("handler_storage._results_list_s3_client")
    @patch("handler_storage._get_ddb")
    @patch("handler_storage.s3")
    def test_color_mosaic_stale_worker_cannot_overwrite_new_refresh(self, mock_s3, mock_get_ddb, mock_list_s3_client):
        import handler_storage

        fake = _FakeS3()
        fake_ddb = _FakeDDB()
        self._patch(mock_s3, fake)
        mock_list_s3_client.return_value = fake
        mock_get_ddb.return_value = fake_ddb
        self._store(fake, "job", {"function": "g"})
        fake.objects["renders/job/color/color_a/image.jpeg"] = b"jpeg"
        fake.objects["renders/job/color/color_a/preview.png"] = (
            b"\x89PNG\r\n\x1a\n" + (13).to_bytes(4, "big") + b"IHDR"
            + (512).to_bytes(4, "big") + (512).to_bytes(4, "big")
        )
        fake.objects["renders/_index/color_mosaic/old/all.json"] = b"old"

        now = handler_storage._utc_now_iso()
        handler_storage._put_mosaic_status({
            "state": "computing",
            "refresh_id": "new",
            "started_at": now,
            "updated_at": now,
            "updated_at_ms": handler_storage._mosaic_now_ms(),
        })

        result = handler_storage._run_color_mosaic_worker("old")
        current = handler_storage._read_mosaic_status()

        self.assertEqual(result["state"], "error")
        self.assertEqual(current["state"], "computing")
        self.assertEqual(current["refresh_id"], "new")
        self.assertIn("renders/_index/color_mosaic/old/all.json", fake.objects)

    @patch("handler_storage.s3")
    def test_render_family_variant_presign_default_and_false_regression(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        self._patch(mock_s3, fake)
        fake.objects["renders/job/color/color_a/image.jpeg"] = b"jpeg"
        fake.objects["renders/job/color/color_a/preview.png"] = b"preview"

        default_entry = handler_storage._list_render_family_variants("job", "color")[0]
        raw_entry = handler_storage._list_render_family_variants("job", "color", presign=False)[0]

        self.assertTrue(default_entry["image_url"].startswith("https://example.test/"))
        self.assertTrue(default_entry["preview_url"].startswith("https://example.test/"))
        self.assertIsNone(raw_entry["image_url"])
        self.assertIsNone(raw_entry["preview_url"])

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


class TestMosaicJpgPreference(unittest.TestCase):
    """deepzoom-speed.md §2.2: the wall prefers a migrated preview.jpg (dims
    from meta, no ranged GET) and falls back to preview.png otherwise."""

    def _entry(self, **overrides):
        entry = {
            "preview_key": "renders/j/color/a/preview.png",
            "artifact_id": "a",
            "created_at": "2026-01-01T00:00:00Z",
            "image_key": "renders/j/color/a/image.jpeg",
        }
        entry.update(overrides)
        return entry

    def test_migrated_entry_serves_jpg_without_touching_s3(self):
        import handler_storage

        class _NoReadClient:
            def get_object(self, **kwargs):
                raise AssertionError("jpg-backed tiles must not read the png header")

        entry = self._entry(
            preview_jpg_key="renders/j/color/a/preview.jpg",
            preview_jpg_width=512,
            preview_jpg_height=512,
        )
        tile, status = handler_storage._mosaic_tile_from_entry(
            _NoReadClient(), "j", entry, {"function": "f", "degree": 5, "N": 100, "times": 1})

        self.assertEqual(status, "512x512")
        self.assertEqual(tile["key"], "renders/j/color/a/preview.jpg")
        self.assertEqual(tile["preview_width"], 512)
        self.assertEqual(tile["preview_height"], 512)

    def test_unmigrated_entry_falls_back_to_png_header_read(self):
        import handler_storage

        header = (
            b"\x89PNG\r\n\x1a\n" + (13).to_bytes(4, "big") + b"IHDR"
            + (1024).to_bytes(4, "big") + (1024).to_bytes(4, "big")
        )

        class _PngClient:
            def get_object(self, Bucket=None, Key=None, Range=None):
                assert Key == "renders/j/color/a/preview.png"
                return {"Body": io.BytesIO(header)}

        tile, status = handler_storage._mosaic_tile_from_entry(
            _PngClient(), "j", self._entry(), {"function": "f"})

        self.assertEqual(status, "1024x1024")
        self.assertEqual(tile["key"], "renders/j/color/a/preview.png")
        self.assertEqual(tile["preview_width"], 1024)

    def test_non_square_jpg_dims_reject_tile(self):
        import handler_storage

        entry = self._entry(
            preview_jpg_key="renders/j/color/a/preview.jpg",
            preview_jpg_width=512,
            preview_jpg_height=256,
        )
        tile, status = handler_storage._mosaic_tile_from_entry(object(), "j", entry, {})
        self.assertIsNone(tile)
        self.assertEqual(status, "non_square")

    def test_render_entry_passes_jpg_fields_from_overlay(self):
        import handler_storage

        image_info = {
            "exists": True, "key": "renders/j/color/a/image.jpeg", "url": None,
            "user_meta": {}, "modified_at": "2026-01-01T00:00:00Z",
            "width": 5000, "height": 5000, "size": 10, "type": "image/jpeg",
        }
        preview_info = {"exists": True, "key": "renders/j/color/a/preview.png", "url": None}
        overlay = {
            "preview_jpg_key": "renders/j/color/a/preview.jpg",
            "preview_jpg_width": "512",
            "preview_jpg_height": "512",
        }
        entry = handler_storage._render_artifact_entry(
            "color", "a", image_info, preview_info, fallback_meta=overlay)

        self.assertEqual(entry["preview_jpg_key"], "renders/j/color/a/preview.jpg")
        self.assertEqual(entry["preview_jpg_width"], 512)
        self.assertEqual(entry["preview_jpg_height"], 512)

    def test_entry_without_migration_reports_empty_jpg_fields(self):
        import handler_storage

        image_info = {
            "exists": True, "key": "renders/j/color/a/image.jpeg", "url": None,
            "user_meta": {}, "modified_at": "2026-01-01T00:00:00Z",
            "width": 5000, "height": 5000, "size": 10, "type": "image/jpeg",
        }
        entry = handler_storage._render_artifact_entry("color", "a", image_info, None)
        self.assertEqual(entry["preview_jpg_key"], "")
        self.assertIsNone(entry["preview_jpg_width"])
        self.assertIsNone(entry["preview_jpg_height"])


if __name__ == "__main__":
    unittest.main()
