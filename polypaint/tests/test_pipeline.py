"""
Tests for the Lambda sparse pixel pipeline — dispatch, storage, DynamoDB status.

Run: cd polypaint && python -m pytest tests/ -v
"""
import json
import os
import struct
import sys
import unittest
from unittest.mock import MagicMock, patch

# Add lambda/ to path so handler imports work
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'lambda'))



# ── Test: handler_dispatch.py ──────────────────────────────────────────────


class TestDispatchHandler(unittest.TestCase):

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_fires_all_jobs(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"job_id": "j", "stripe_idx": i} for i in range(5)]
        event = self._make_event({"target": "raster", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 5)
        self.assertEqual(body["total"], 5)
        self.assertEqual(mock_client.invoke.call_count, 5)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_missing_target(self, mock_client):
        from handler_dispatch import handler
        event = self._make_event({"jobs": []})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertIn("error", body)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_unknown_target(self, mock_client):
        from handler_dispatch import handler
        event = self._make_event({"target": "bogus", "jobs": []})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertIn("error", body)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_reports_errors(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.side_effect = Exception("throttled")
        jobs = [{"job_id": "j", "stripe_idx": 0}]
        event = self._make_event({"target": "raster", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 0)
        self.assertEqual(len(body["errors"]), 1)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_raster_target(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"job_id": "j", "stripe_idx": 0, "bin_key": "renders/j/stripe_0.bin"}]
        event = self._make_event({"target": "raster", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_finalize_target(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"job_id": "j", "tile_idx": 0, "n_stripes": 10, "tile_w": 4096, "tile_h": 4096}]
        event = self._make_event({"target": "finalize", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_encode_target(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"out_key": "renders/j/image.jpeg", "tile_grid": {}}]
        event = self._make_event({"target": "encode", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)


# ── Test: handler_storage.py (list, check_keys, check_status, clean_render) ──


class TestStorageList(unittest.TestCase):

    @patch("handler_storage._key_exists")
    @patch("handler_storage.s3")
    def test_list_uses_delimiter(self, mock_s3, mock_exists):
        """Verify list uses Delimiter='/' to get folders, not enumerate all objects."""
        from handler_storage import handle_list
        mock_exists.return_value = False
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "CommonPrefixes": [
                {"Prefix": "renders/job_abc/"},
                {"Prefix": "renders/job_def/"},
            ]
        }]
        # Mock calc.json reads
        mock_s3.get_object.side_effect = [
            {"Body": MagicMock(read=MagicMock(return_value=json.dumps({
                "function": "giga_1", "degree": 24, "n1": 100, "n2": 100,
                "n_stripes": 10, "stripes": [{"bin_size": 500}] * 10,
                "total_coeffs_size": 1000,
            }).encode()))},
            {"Body": MagicMock(read=MagicMock(return_value=json.dumps({
                "function": "giga_5", "degree": 25, "n1": 200, "n2": 200,
                "n_stripes": 20,
            }).encode()))},
        ]

        result = handle_list({"body": "{}"})
        body = json.loads(result["body"])

        self.assertEqual(body["count"], 2)
        # Verify Delimiter was used in paginate call
        paginate_kwargs = mock_paginator.paginate.call_args[1]
        self.assertEqual(paginate_kwargs["Delimiter"], "/")

        funcs = {r["function"] for r in body["results"]}
        self.assertEqual(funcs, {"giga_1", "giga_5"})

    @patch("handler_storage._key_exists")
    @patch("handler_storage.s3")
    def test_list_total_size_from_calc(self, mock_s3, mock_exists):
        """Verify total_size computed from calc.json stripes, not S3 enumeration."""
        from handler_storage import handle_list
        mock_exists.return_value = False
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "CommonPrefixes": [{"Prefix": "renders/job1/"}]
        }]
        mock_s3.get_object.return_value = {
            "Body": MagicMock(read=MagicMock(return_value=json.dumps({
                "function": "giga_1", "degree": 24, "n1": 100, "n2": 100,
                "n_stripes": 2,
                "stripes": [{"bin_size": 3000}, {"bin_size": 4000}],
                "total_coeffs_size": 500,
            }).encode()))
        }

        result = handle_list({"body": "{}"})
        body = json.loads(result["body"])
        self.assertEqual(body["results"][0]["total_size"], 7500)

    @patch("handler_storage._key_exists")
    @patch("handler_storage.s3")
    def test_list_missing_calc_json(self, mock_s3, mock_exists):
        """Jobs without calc.json still appear with function='?'."""
        from handler_storage import handle_list
        mock_exists.return_value = False
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "CommonPrefixes": [{"Prefix": "renders/orphan/"}]
        }]
        from botocore.exceptions import ClientError
        mock_s3.get_object.side_effect = ClientError(
            {"Error": {"Code": "NoSuchKey", "Message": ""}}, "GetObject")

        result = handle_list({"body": "{}"})
        body = json.loads(result["body"])
        self.assertEqual(body["count"], 1)
        self.assertEqual(body["results"][0]["function"], "?")
        self.assertEqual(body["results"][0]["total_size"], 0)

    @patch("handler_storage._key_exists")
    @patch("handler_storage.s3")
    def test_list_has_image_checks_head(self, mock_s3, mock_exists):
        """Verify has_preview/has_image use HEAD requests."""
        from handler_storage import handle_list
        # preview.jpg exists, image.jpeg exists
        mock_exists.side_effect = lambda k: "preview" in k or "image.jpeg" in k
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "CommonPrefixes": [{"Prefix": "renders/img_job/"}]
        }]
        mock_s3.get_object.return_value = {
            "Body": MagicMock(read=MagicMock(return_value=json.dumps({
                "function": "giga_1", "degree": 24, "n1": 100, "n2": 100,
                "n_stripes": 1,
            }).encode()))
        }
        mock_s3.generate_presigned_url.return_value = "https://signed"

        result = handle_list({"body": "{}"})
        body = json.loads(result["body"])
        self.assertTrue(body["results"][0]["has_preview"])
        self.assertTrue(body["results"][0]["has_image"])
        self.assertEqual(body["results"][0]["preview_url"], "https://signed")


class TestStorageCheckKeys(unittest.TestCase):

    @patch("handler_storage.s3")
    def test_check_keys_counts_matching_suffix(self, mock_s3):
        from handler_storage import handle_check_keys
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "Contents": [
                {"Key": "renders/j/stripe_0_t0.raw"},
                {"Key": "renders/j/stripe_0_t1.raw"},
                {"Key": "renders/j/calc.json"},
                {"Key": "renders/j/stripe_1_t0.raw"},
            ]
        }]
        event = {"body": json.dumps({
            "prefix": "renders/j/stripe_",
            "expected": 3,
            "suffix": ".raw",
        })}
        result = handle_check_keys(event)
        body = json.loads(result["body"])
        self.assertEqual(body["found"], 3)
        self.assertTrue(body["done"])


class TestStorageCleanRender(unittest.TestCase):

    @patch("handler_storage.s3")
    def test_clean_render_deletes_intermediates_only(self, mock_s3):
        """Clean-render deletes pix/raw/tile intermediates, preserves final images."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        # Paginator returns per-prefix results — only intermediates match
        mock_paginator.paginate.return_value = [{
            "Contents": [
                {"Key": "renders/j/tile_0000.raw"},
                {"Key": "renders/j/pix_0000_t0000.pix"},
            ]
        }]
        mock_s3.delete_objects.return_value = {"Deleted": [
            {"Key": "renders/j/tile_0000.raw"},
            {"Key": "renders/j/pix_0000_t0000.pix"},
        ]}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "color"})}
        result = handle_clean_render(event)
        body = json.loads(result["body"])
        self.assertGreaterEqual(body["deleted"], 2)

        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertIn("renders/j/pix_0000_t0000.pix", deleted_keys)
        self.assertIn("renders/j/tile_0000.raw", deleted_keys)
        self.assertIn("renders/j/preview_color.png", deleted_keys)


    @patch("handler_storage.s3")
    def test_clean_render_color_deletes_only_color_preview(self, mock_s3):
        """Color pipeline deletes preview_color.png but not preview_bilevel.png."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "color"})}
        handle_clean_render(event)
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertIn("renders/j/preview_color.png", deleted_keys)
        self.assertNotIn("renders/j/preview_bilevel.png", deleted_keys)

    @patch("handler_storage.s3")
    def test_clean_render_bilevel_deletes_only_bilevel_preview(self, mock_s3):
        """Bilevel pipeline deletes preview_bilevel.png but not preview_color.png."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "bilevel"})}
        handle_clean_render(event)
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertIn("renders/j/preview_bilevel.png", deleted_keys)
        self.assertNotIn("renders/j/preview_color.png", deleted_keys)

    @patch("handler_storage.s3")
    def test_clean_render_default_pipeline_is_color(self, mock_s3):
        """No pipeline param defaults to color (backward compat)."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j"})}
        handle_clean_render(event)
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertIn("renders/j/preview_color.png", deleted_keys)
        self.assertNotIn("renders/j/preview_bilevel.png", deleted_keys)

    @patch("handler_storage.s3")
    def test_clean_render_preserves_final_images(self, mock_s3):
        """Clean-render does not delete final output images."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "color"})}
        handle_clean_render(event)
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        # Only explicit keys should be here, no final images
        for key in deleted_keys:
            self.assertFalse(key.endswith("image.jpeg"), "image.jpeg should not be deleted")
            self.assertFalse(key.endswith("image.png"), "image.png should not be deleted")
            self.assertFalse(key.endswith("image_bilevel.tif"), "image_bilevel.tif should not be deleted")
            self.assertFalse(key.endswith("calc.json") and "solve_proximity" not in key,
                             "calc.json should not be deleted")

    @patch("handler_storage.s3")
    def test_clean_render_deletes_solve_proximity_artifacts(self, mock_s3):
        """Clean-render deletes solve_proximity intermediate artifacts."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": [
            {"Key": "renders/j/solve_proximity/stripe_0_hist.json"},
        ]}]
        mock_s3.delete_objects.return_value = {"Deleted": [{"Key": "renders/j/solve_proximity/stripe_0_hist.json"}]}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "color"})}
        handle_clean_render(event)
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertIn("renders/j/solve_proximity/stripe_0_hist.json", deleted_keys)
        self.assertIn("renders/j/solve_proximity_clip.json", deleted_keys)
        self.assertIn("renders/j/solve_proximity_bins.json", deleted_keys)


class TestStoragePresign(unittest.TestCase):

    @patch("handler_storage.s3")
    def test_presign_without_filename(self, mock_s3):
        from handler_storage import handle_presign
        mock_s3.generate_presigned_url.return_value = "https://example.com/signed"
        event = {"body": json.dumps({"key": "renders/j/image.jpeg"})}
        result = handle_presign(event)
        body = json.loads(result["body"])
        self.assertEqual(body["url"], "https://example.com/signed")
        # Verify no ResponseContentDisposition in params
        call_params = mock_s3.generate_presigned_url.call_args[1]["Params"]
        self.assertNotIn("ResponseContentDisposition", call_params)

    @patch("handler_storage.s3")
    def test_presign_with_filename(self, mock_s3):
        from handler_storage import handle_presign
        mock_s3.generate_presigned_url.return_value = "https://example.com/signed"
        event = {"body": json.dumps({
            "key": "renders/j/image.jpeg",
            "filename": "my_image.jpeg",
        })}
        result = handle_presign(event)
        body = json.loads(result["body"])
        self.assertEqual(body["url"], "https://example.com/signed")
        call_params = mock_s3.generate_presigned_url.call_args[1]["Params"]
        self.assertIn("ResponseContentDisposition", call_params)
        self.assertIn("my_image.jpeg", call_params["ResponseContentDisposition"])


# ── Test: shared.py ───────────────────────────────────────────────────────


class TestShared(unittest.TestCase):

    def test_parse_body_string(self):
        from shared import parse_body
        event = {"body": '{"foo": 42}'}
        self.assertEqual(parse_body(event), {"foo": 42})

    def test_parse_body_dict(self):
        from shared import parse_body
        event = {"body": {"foo": 42}}
        self.assertEqual(parse_body(event), {"foo": 42})

    def test_parse_body_no_body(self):
        from shared import parse_body
        event = {"foo": 42}
        self.assertEqual(parse_body(event), {"foo": 42})

    def test_ok_response_format(self):
        from shared import ok_response
        resp = ok_response({"x": 1})
        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(resp["headers"]["Content-Type"], "application/json")
        self.assertEqual(resp["headers"]["Access-Control-Allow-Origin"], "*")
        self.assertEqual(json.loads(resp["body"]), {"x": 1})

    def test_compute_viewport_empty(self):
        from shared import compute_viewport_from_bin
        result = compute_viewport_from_bin(b"")
        self.assertEqual(result["n_roots"], 0)
        self.assertEqual(result["scale"], 1.0)

    def test_compute_viewport_single_root(self):
        from shared import compute_viewport_from_bin
        data = struct.pack("<ff", 1.0, 2.0)
        result = compute_viewport_from_bin(data)
        self.assertEqual(result["n_roots"], 1)
        self.assertAlmostEqual(result["center_re"], 1.0)
        self.assertAlmostEqual(result["center_im"], 2.0)

    def test_compute_viewport_filters_nan(self):
        from shared import compute_viewport_from_bin
        data = struct.pack("<ffff", float('nan'), float('nan'), 1.0, 2.0)
        result = compute_viewport_from_bin(data)
        self.assertEqual(result["n_roots"], 1)


# ── Test: DynamoDB status tracking ──────────────────────────────────────────


class TestReportStatus(unittest.TestCase):

    @patch("shared._ddb", None)
    @patch("shared.boto3")
    def test_report_done(self, mock_boto3):
        import shared
        shared._ddb = None
        mock_ddb = MagicMock()
        mock_boto3.client.return_value = mock_ddb

        shared.report_status("job1", "raster_0", "done")

        mock_ddb.put_item.assert_called_once()
        item = mock_ddb.put_item.call_args[1]["Item"]
        self.assertEqual(item["job_id"]["S"], "job1")
        self.assertEqual(item["task_id"]["S"], "raster_0")
        self.assertEqual(item["task_status"]["S"], "done")
        self.assertIn("ttl", item)
        self.assertNotIn("error_msg", item)
        shared._ddb = None

    @patch("shared._ddb", None)
    @patch("shared.boto3")
    def test_report_error(self, mock_boto3):
        import shared
        shared._ddb = None
        mock_ddb = MagicMock()
        mock_boto3.client.return_value = mock_ddb

        shared.report_status("job1", "tile_3", "error", "pixassemble failed: OOM")

        item = mock_ddb.put_item.call_args[1]["Item"]
        self.assertEqual(item["task_status"]["S"], "error")
        self.assertEqual(item["error_msg"]["S"], "pixassemble failed: OOM")
        shared._ddb = None

    @patch("shared._ddb", None)
    @patch("shared.boto3")
    def test_error_msg_truncated(self, mock_boto3):
        import shared
        shared._ddb = None
        mock_ddb = MagicMock()
        mock_boto3.client.return_value = mock_ddb

        shared.report_status("job1", "t", "error", "x" * 2000)

        item = mock_ddb.put_item.call_args[1]["Item"]
        self.assertEqual(len(item["error_msg"]["S"]), 1000)
        shared._ddb = None


class TestCheckStatus(unittest.TestCase):

    @patch("handler_storage._get_ddb")
    def test_all_done(self, mock_get_ddb):
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "raster_0"}, "task_status": {"S": "done"}},
                {"task_id": {"S": "raster_1"}, "task_status": {"S": "done"}},
                {"task_id": {"S": "raster_2"}, "task_status": {"S": "done"}},
            ],
        }
        event = {"body": json.dumps({
            "job_id": "j", "task_prefix": "raster_", "expected": 3,
        })}
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertEqual(body["done"], 3)
        self.assertEqual(body["errors"], 0)
        self.assertTrue(body["complete"])

    @patch("handler_storage._get_ddb")
    def test_partial_with_errors(self, mock_get_ddb):
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "tile_0"}, "task_status": {"S": "done"}},
                {"task_id": {"S": "tile_1"}, "task_status": {"S": "error"},
                 "error_msg": {"S": "pixassemble failed"}},
            ],
        }
        event = {"body": json.dumps({
            "job_id": "j", "task_prefix": "tile_", "expected": 3,
        })}
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertEqual(body["done"], 1)
        self.assertEqual(body["errors"], 1)
        self.assertFalse(body["complete"])
        self.assertEqual(body["error_details"][0]["error_msg"], "pixassemble failed")

    @patch("handler_storage._get_ddb")
    def test_empty_result(self, mock_get_ddb):
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {"Items": []}
        event = {"body": json.dumps({
            "job_id": "j", "task_prefix": "raster_", "expected": 500,
        })}
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertEqual(body["done"], 0)
        self.assertFalse(body["complete"])

    @patch("handler_storage._get_ddb")
    def test_paginated_query(self, mock_get_ddb):
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.side_effect = [
            {
                "Items": [
                    {"task_id": {"S": "raster_0"}, "task_status": {"S": "done"}},
                    {"task_id": {"S": "raster_1"}, "task_status": {"S": "done"}},
                ],
                "LastEvaluatedKey": {"job_id": {"S": "j"}, "task_id": {"S": "raster_1"}},
            },
            {
                "Items": [
                    {"task_id": {"S": "raster_2"}, "task_status": {"S": "done"}},
                ],
            },
        ]
        event = {"body": json.dumps({
            "job_id": "j", "task_prefix": "raster_", "expected": 3,
        })}
        result = handle_check_status(event)
        body = json.loads(result["body"])
        self.assertEqual(body["done"], 3)
        self.assertTrue(body["complete"])
        self.assertEqual(mock_ddb.query.call_count, 2)


class TestCleanRenderDynamoDB(unittest.TestCase):

    @patch("handler_storage._get_ddb")
    @patch("handler_storage.s3")
    def test_clean_render_clears_ddb(self, mock_s3, mock_get_ddb):
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": [
            {"Key": "renders/j/tile_0000.raw"},
        ]}]
        mock_s3.delete_objects.return_value = {"Deleted": [{"Key": "renders/j/tile_0000.raw"}]}

        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {
            "Items": [
                {"job_id": {"S": "j"}, "task_id": {"S": "raster_0"}},
                {"job_id": {"S": "j"}, "task_id": {"S": "tile_0"}},
            ],
        }
        mock_ddb.batch_write_item.return_value = {"UnprocessedItems": {}}

        event = {"body": json.dumps({"job_id": "j"})}
        result = handle_clean_render(event)
        body = json.loads(result["body"])

        self.assertEqual(body["deleted"], 1)
        self.assertEqual(body["ddb_deleted"], 2)
        mock_ddb.batch_write_item.assert_called_once()


# ── Test: handler_coeffgen.py (striped coefficient generation) ────────────


class TestCoeffgenHandler(unittest.TestCase):

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    def _mock_open(self, path, mode="r", **kwargs):
        """Mock open() for /tmp files — return a file-like mock."""
        if "/tmp/" in str(path) and "b" in mode:
            m = MagicMock()
            m.__enter__ = MagicMock(return_value=m)
            m.__exit__ = MagicMock(return_value=False)
            m.read = MagicMock(return_value=b"\x00" * 100)
            return m
        return self._original_open(path, mode, **kwargs)

    def setUp(self):
        import builtins
        self._original_open = builtins.open

    @patch("builtins.open")
    @patch("handler_coeffgen.os.path.getsize")
    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.report_status")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_basic(self, mock_subprocess, mock_s3, mock_report, mock_remove, mock_getsize, mock_open):
        mock_open.side_effect = self._mock_open
        mock_getsize.return_value = 9600
        from handler_coeffgen import handler
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "data_bytes": 9600,
            "n_coeffs": 24,
            "degree": 24,
        })
        mock_subprocess.run.return_value = mock_result

        event = self._make_event({
            "job_id": "test-job",
            "stripe_idx": 3,
            "function": "giga_1",
            "n1": 100,
            "n2": 100,
            "i1_start": 30,
            "i1_end": 40,
        })
        result = handler(event, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["job_id"], "test-job")
        self.assertEqual(body["stripe_idx"], 3)
        self.assertEqual(body["coeffs_key"], "renders/test-job/coeffs_0003.bin")
        self.assertEqual(body["n_coeffs"], 24)
        self.assertEqual(body["degree"], 24)
        self.assertEqual(body["coeffs_size"], 9600)

        # Verify sweep binary was called with correct spec
        call_args = mock_subprocess.run.call_args
        spec = json.loads(call_args[1]["input"])
        self.assertEqual(spec["mode"], "coeffgen")
        self.assertEqual(spec["function"], "giga_1")
        self.assertEqual(spec["n1"], 100)
        self.assertEqual(spec["n2"], 100)
        self.assertEqual(spec["i1_start"], 30)
        self.assertEqual(spec["i1_end"], 40)

    @patch("builtins.open")
    @patch("handler_coeffgen.os.path.getsize")
    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.report_status")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_with_transforms(self, mock_subprocess, mock_s3, mock_report, mock_remove, mock_getsize, mock_open):
        mock_open.side_effect = self._mock_open
        mock_getsize.return_value = 4800
        from handler_coeffgen import handler
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "data_bytes": 4800,
            "n_coeffs": 12,
            "degree": 12,
        })
        mock_subprocess.run.return_value = mock_result

        event = self._make_event({
            "job_id": "t-job",
            "stripe_idx": 0,
            "function": "giga_5",
            "param_transforms": ["unit_circle", "square"],
            "coeff_transforms": ["rev", "conj"],
            "n1": 10,
            "n2": 10,
            "i1_start": 0,
            "i1_end": 10,
        })
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 200)

        # Verify transforms passed through to sweep spec
        spec = json.loads(mock_subprocess.run.call_args[1]["input"])
        self.assertEqual(spec["param_transforms"], ["unit_circle", "square"])
        self.assertEqual(spec["coeff_transforms"], ["rev", "conj"])

    @patch("builtins.open")
    @patch("handler_coeffgen.os.path.getsize")
    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.report_status")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_s3_upload(self, mock_subprocess, mock_s3, mock_report, mock_remove, mock_getsize, mock_open):
        mock_open.side_effect = self._mock_open
        mock_getsize.return_value = 100
        from handler_coeffgen import handler
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "data_bytes": 100,
            "n_coeffs": 5,
            "degree": 5,
        })
        mock_subprocess.run.return_value = mock_result

        event = self._make_event({
            "job_id": "upload-test",
            "stripe_idx": 7,
            "function": "giga_1",
            "n1": 100,
            "n2": 100,
            "i1_start": 70,
            "i1_end": 80,
        })
        handler(event, None)

        # Verify S3 upload was called with per-stripe key
        mock_s3.put_object.assert_called_once()
        call_kwargs = mock_s3.put_object.call_args[1]
        self.assertEqual(call_kwargs["Key"], "renders/upload-test/coeffs_0007.bin")
        self.assertEqual(call_kwargs["ContentType"], "application/octet-stream")

    @patch("handler_coeffgen.report_status")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_sweep_failure(self, mock_subprocess, mock_s3, mock_report):
        from handler_coeffgen import handler
        mock_result = MagicMock()
        mock_result.returncode = 1
        mock_result.stderr = "bad function name"
        mock_subprocess.run.return_value = mock_result

        event = self._make_event({
            "job_id": "fail-test",
            "stripe_idx": 0,
            "function": "bad_func",
            "n1": 5,
            "n2": 5,
            "i1_start": 0,
            "i1_end": 5,
        })
        with self.assertRaises(RuntimeError) as ctx:
            handler(event, None)
        self.assertIn("coeffgen failed", str(ctx.exception))

        # Verify error was reported to DDB
        mock_report.assert_any_call("fail-test", "coeffgen_0", "error", unittest.mock.ANY)

    @patch("builtins.open")
    @patch("handler_coeffgen.os.path.getsize")
    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.report_status")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_default_transforms(self, mock_subprocess, mock_s3, mock_report, mock_remove, mock_getsize, mock_open):
        """Omitting transforms defaults to empty lists."""
        mock_open.side_effect = self._mock_open
        mock_getsize.return_value = 100
        from handler_coeffgen import handler
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "data_bytes": 100,
            "n_coeffs": 5,
            "degree": 5,
        })
        mock_subprocess.run.return_value = mock_result

        event = self._make_event({
            "job_id": "j",
            "stripe_idx": 0,
            "function": "giga_1",
            "n1": 5,
            "n2": 5,
            "i1_start": 0,
            "i1_end": 5,
        })
        handler(event, None)

        spec = json.loads(mock_subprocess.run.call_args[1]["input"])
        self.assertEqual(spec["param_transforms"], [])
        self.assertEqual(spec["coeff_transforms"], [])

    @patch("builtins.open")
    @patch("handler_coeffgen.os.path.getsize")
    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.report_status")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_ddb_status(self, mock_subprocess, mock_s3, mock_report, mock_remove, mock_getsize, mock_open):
        """Verify started/done status reported to DynamoDB."""
        mock_open.side_effect = self._mock_open
        mock_getsize.return_value = 100
        from handler_coeffgen import handler
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "data_bytes": 100,
            "n_coeffs": 5,
            "degree": 5,
        })
        mock_subprocess.run.return_value = mock_result

        event = self._make_event({
            "job_id": "ddb-test",
            "stripe_idx": 2,
            "function": "giga_1",
            "n1": 10,
            "n2": 10,
            "i1_start": 4,
            "i1_end": 6,
        })
        handler(event, None)

        # Verify started + done status reported
        calls = mock_report.call_args_list
        self.assertEqual(calls[0], unittest.mock.call("ddb-test", "coeffgen_2", "started"))
        self.assertEqual(calls[1], unittest.mock.call("ddb-test", "coeffgen_2", "done"))

    @patch("builtins.open")
    @patch("handler_coeffgen.os.path.getsize")
    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.report_status")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_s3_key_override(self, mock_subprocess, mock_s3, mock_report, mock_remove, mock_getsize, mock_open):
        """Verify s3_key override for lores coefficients."""
        mock_open.side_effect = self._mock_open
        mock_getsize.return_value = 200
        from handler_coeffgen import handler
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "data_bytes": 200,
            "n_coeffs": 5,
            "degree": 5,
        })
        mock_subprocess.run.return_value = mock_result

        event = self._make_event({
            "job_id": "lores-test",
            "stripe_idx": 9999,
            "function": "giga_1",
            "n1": 100,
            "n2": 100,
            "i1_start": 0,
            "i1_end": 100,
            "s3_key": "renders/lores-test/lores_coeffs.bin",
        })
        result = handler(event, None)
        body = json.loads(result["body"])

        self.assertEqual(body["coeffs_key"], "renders/lores-test/lores_coeffs.bin")
        call_kwargs = mock_s3.put_object.call_args[1]
        self.assertEqual(call_kwargs["Key"], "renders/lores-test/lores_coeffs.bin")


# ── Test: handler_sweep.py (solve-from-coefficients path) ────────────────


class TestSolveFromCoeffs(unittest.TestCase):

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    @patch("handler_sweep.report_status")
    @patch("handler_sweep.os.path.getsize", return_value=400)
    @patch("handler_sweep.os.remove")
    @patch("handler_sweep.s3")
    @patch("handler_sweep.subprocess")
    def test_solve_routes_on_coeffs_key(self, mock_subprocess, mock_s3, mock_remove, mock_getsize, mock_ddb):
        """When coeffs_key is present, handler routes to solve path."""
        from handler_sweep import handler
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "n_t": 500,
            "degree": 24,
            "avg_iterations": 12.3,
        })
        mock_subprocess.run.return_value = mock_result

        # Mock S3 full file download (no range reads)
        mock_body = MagicMock()
        mock_body.read.return_value = b"\x00" * 1000
        mock_s3.get_object.return_value = {"Body": mock_body}

        import builtins
        original_open = builtins.open

        def mock_open(path, mode="r", **kwargs):
            if "/tmp/" in str(path) and "b" in mode:
                m = MagicMock()
                m.__enter__ = MagicMock(return_value=m)
                m.__exit__ = MagicMock(return_value=False)
                if "w" in mode:
                    m.write = MagicMock()
                else:
                    m.read = MagicMock(return_value=b"\x00" * 500)
                return m
            return original_open(path, mode, **kwargs)

        with patch("builtins.open", side_effect=mock_open):
            event = self._make_event({
                "job_id": "solve-test",
                "stripe_idx": 0,
                "coeffs_key": "renders/solve-test/coeffs_0000.bin",
                "n_coeffs": 24,
                "n1": 100,
                "n2": 100,
                "i1_start": 0,
                "i1_end": 10,
            })
            result = handler(event, None)

        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["stripe_idx"], 0)
        self.assertEqual(body["n_t"], 500)
        self.assertEqual(body["degree"], 24)
        self.assertEqual(body["n_procs"], 1)

        # Verify sweep was called in solve mode
        call_args = mock_subprocess.run.call_args
        spec = json.loads(call_args[1]["input"])
        self.assertEqual(spec["mode"], "solve")
        self.assertEqual(spec["n_coeffs"], 24)
        # Chunk-native: n_steps = (i1_end - i1_start) * n2 = 10 * 100 = 1000
        # Passed as n2=1000, i1_start=0, i1_end=1
        self.assertEqual(spec["i1_start"], 0)
        self.assertEqual(spec["i1_end"], 1)
        self.assertEqual(spec["n2"], 1000)

    @patch("handler_sweep.report_status")
    @patch("handler_sweep.os.path.getsize", return_value=400)
    @patch("handler_sweep.os.remove")
    @patch("handler_sweep.s3")
    @patch("handler_sweep.subprocess")
    def test_solve_downloads_full_file(self, mock_subprocess, mock_s3, mock_remove, mock_getsize, mock_ddb):
        """Verify S3 get_object downloads full per-stripe file (no Range)."""
        from handler_sweep import handler
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "n_t": 100,
            "degree": 10,
            "avg_iterations": 5.0,
        })
        mock_subprocess.run.return_value = mock_result

        mock_body = MagicMock()
        mock_body.read.return_value = b"\x00" * 8000
        mock_s3.get_object.return_value = {"Body": mock_body}

        import builtins
        original_open = builtins.open

        def mock_open(path, mode="r", **kwargs):
            if "/tmp/" in str(path) and "b" in mode:
                m = MagicMock()
                m.__enter__ = MagicMock(return_value=m)
                m.__exit__ = MagicMock(return_value=False)
                if "w" in mode:
                    m.write = MagicMock()
                else:
                    m.read = MagicMock(return_value=b"\x00" * 400)
                return m
            return original_open(path, mode, **kwargs)

        with patch("builtins.open", side_effect=mock_open):
            event = self._make_event({
                "job_id": "full-dl-test",
                "stripe_idx": 2,
                "coeffs_key": "renders/full-dl-test/coeffs_0002.bin",
                "n_coeffs": 10,
                "n1": 100,
                "n2": 50,
                "i1_start": 20,
                "i1_end": 30,
            })
            result = handler(event, None)

        # Verify S3 get_object called with Key only (no Range parameter)
        call_kwargs = mock_s3.get_object.call_args[1]
        self.assertNotIn("Range", call_kwargs)
        self.assertEqual(call_kwargs["Key"], "renders/full-dl-test/coeffs_0002.bin")

    @patch("handler_sweep.report_status")
    @patch("handler_sweep.os.path.getsize", return_value=50)
    @patch("handler_sweep.os.remove")
    @patch("handler_sweep.s3")
    @patch("handler_sweep.subprocess")
    def test_solve_custom_s3_key(self, mock_subprocess, mock_s3, mock_remove, mock_getsize, mock_ddb):
        """Verify s3_key override works for solve path."""
        from handler_sweep import handler
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "n_t": 50,
            "degree": 5,
            "avg_iterations": 3.0,
        })
        mock_subprocess.run.return_value = mock_result

        mock_body = MagicMock()
        mock_body.read.return_value = b"\x00" * 100
        mock_s3.get_object.return_value = {"Body": mock_body}

        import builtins
        original_open = builtins.open

        def mock_open(path, mode="r", **kwargs):
            if "/tmp/" in str(path) and "b" in mode:
                m = MagicMock()
                m.__enter__ = MagicMock(return_value=m)
                m.__exit__ = MagicMock(return_value=False)
                if "w" in mode:
                    m.write = MagicMock()
                else:
                    m.read = MagicMock(return_value=b"\x00" * 50)
                return m
            return original_open(path, mode, **kwargs)

        with patch("builtins.open", side_effect=mock_open):
            event = self._make_event({
                "job_id": "key-test",
                "stripe_idx": 0,
                "coeffs_key": "renders/key-test/lores_coeffs.bin",
                "n_coeffs": 5,
                "n1": 10,
                "n2": 10,
                "i1_start": 0,
                "i1_end": 5,
                "s3_key": "renders/key-test/lores.bin",
            })
            result = handler(event, None)

        body = json.loads(result["body"])
        # Verify the custom s3_key was used for upload
        upload_args = mock_s3.upload_fileobj.call_args[0]
        self.assertEqual(upload_args[2], "renders/key-test/lores.bin")
        self.assertEqual(body["s3_key"], "renders/key-test/lores.bin")

    @patch("handler_sweep.report_status")
    @patch("handler_sweep.os.remove")
    @patch("handler_sweep.s3")
    @patch("handler_sweep.subprocess")
    def test_solve_failure_raises(self, mock_subprocess, mock_s3, mock_remove, mock_ddb):
        """Verify solve mode raises RuntimeError on sweep failure."""
        from handler_sweep import handler
        mock_result = MagicMock()
        mock_result.returncode = 1
        mock_result.stderr = "solver diverged"
        mock_subprocess.run.return_value = mock_result

        mock_body = MagicMock()
        mock_body.read.return_value = b"\x00" * 100
        mock_s3.get_object.return_value = {"Body": mock_body}

        import builtins
        original_open = builtins.open

        def mock_open(path, mode="r", **kwargs):
            if "/tmp/" in str(path) and "b" in mode:
                m = MagicMock()
                m.__enter__ = MagicMock(return_value=m)
                m.__exit__ = MagicMock(return_value=False)
                m.write = MagicMock()
                return m
            return original_open(path, mode, **kwargs)

        with patch("builtins.open", side_effect=mock_open):
            event = self._make_event({
                "job_id": "fail-test",
                "stripe_idx": 0,
                "coeffs_key": "renders/fail-test/coeffs_0000.bin",
                "n_coeffs": 5,
                "n1": 10,
                "n2": 10,
                "i1_start": 0,
                "i1_end": 5,
            })
            with self.assertRaises(RuntimeError) as ctx:
                handler(event, None)
            self.assertIn("solve failed", str(ctx.exception))

    # test_grid_mode_still_works: removed — handle_compute_only_stripe deleted


# ── Test: handler_preview.py (single-call preview generation) ────────────


class TestPreviewHandler(unittest.TestCase):

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    def _make_lores_bin(self, n_roots=10):
        """Create fake lores.bin: n_roots f32 pairs in a circle."""
        import struct as st
        import math
        data = bytearray()
        for i in range(n_roots):
            angle = 2 * math.pi * i / n_roots
            re = math.cos(angle)
            im = math.sin(angle)
            data.extend(st.pack('<ff', re, im))
        return bytes(data)

    @patch("handler_preview.s3")
    def test_preview_basic(self, mock_s3):
        from handler_preview import handler
        lores_bin = self._make_lores_bin(50)
        calc_json = json.dumps({
            "lores": {"bin_key": "renders/test/lores.bin"},
            "degree": 5,
        }).encode()

        # Mock S3: first call = calc.json, second = lores.bin
        mock_s3.get_object.side_effect = [
            {"Body": MagicMock(read=MagicMock(return_value=calc_json))},
            {"Body": MagicMock(read=MagicMock(return_value=lores_bin))},
        ]
        mock_s3.generate_presigned_url.return_value = "https://signed-url"

        event = self._make_event({"job_id": "test", "quantile": 0.0, "shim": 0.05})
        result = handler(event, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["job_id"], "test")
        self.assertEqual(body["degree"], 5)
        self.assertEqual(body["preview_size"], 256)
        self.assertEqual(body["image_url"], "https://signed-url")
        self.assertGreater(body["png_size"], 0)
        self.assertGreater(body["n_roots"], 0)

        # Verify PNG uploaded to S3
        mock_s3.put_object.assert_called_once()
        call_kwargs = mock_s3.put_object.call_args[1]
        self.assertEqual(call_kwargs["Key"], "renders/test/preview.png")
        self.assertEqual(call_kwargs["ContentType"], "image/png")
        # Verify it's valid PNG (starts with PNG magic bytes)
        png_data = call_kwargs["Body"]
        self.assertTrue(png_data[:4] == b'\x89PNG')

    @patch("handler_preview.s3")
    def test_preview_custom_size(self, mock_s3):
        from handler_preview import handler
        lores_bin = self._make_lores_bin(20)
        calc_json = json.dumps({
            "lores": {"bin_key": "renders/sz/lores.bin"},
            "degree": 10,
        }).encode()

        mock_s3.get_object.side_effect = [
            {"Body": MagicMock(read=MagicMock(return_value=calc_json))},
            {"Body": MagicMock(read=MagicMock(return_value=lores_bin))},
        ]
        mock_s3.generate_presigned_url.return_value = "https://url"

        event = self._make_event({
            "job_id": "sz", "preview_size": 128,
        })
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["preview_size"], 128)

    @patch("handler_preview.s3")
    def test_preview_empty_lores(self, mock_s3):
        """Preview with no roots should still succeed (black image)."""
        from handler_preview import handler
        calc_json = json.dumps({
            "lores": {"bin_key": "renders/empty/lores.bin"},
            "degree": 5,
        }).encode()

        mock_s3.get_object.side_effect = [
            {"Body": MagicMock(read=MagicMock(return_value=calc_json))},
            {"Body": MagicMock(read=MagicMock(return_value=b""))},  # empty lores.bin
        ]
        mock_s3.generate_presigned_url.return_value = "https://url"

        event = self._make_event({"job_id": "empty"})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["n_roots"], 0)
        self.assertGreater(body["png_size"], 0)  # still a valid PNG

    @patch("handler_preview.s3")
    def test_preview_png_valid(self, mock_s3):
        """Verify grayscale PNG output is structurally valid."""
        from handler_preview import _encode_png_gray
        gray = bytearray(4 * 4)  # 4x4 black image
        gray[0] = 255  # white pixel at (0,0)
        png = _encode_png_gray(4, 4, gray)
        self.assertTrue(png.startswith(b'\x89PNG\r\n\x1a\n'))
        # IHDR chunk follows
        self.assertEqual(png[12:16], b'IHDR')


class TestRenderSummary(unittest.TestCase):

    def _make_event(self, body):
        return {"body": json.dumps(body), "rawPath": "/render-summary"}

    @patch("handler_storage.s3")
    def test_render_summary_heads_exact_keys_only(self, mock_s3):
        """render-summary uses HEAD for exactly 10 artifact keys, never paginator."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_s3.generate_presigned_url.return_value = "https://fake"

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        # Must never use paginator
        mock_s3.get_paginator.assert_not_called()

        # HEAD should be called for exactly 10 keys
        head_keys = [c.kwargs.get("Key") or c.args[1] if len(c.args) > 1 else c.kwargs.get("Key")
                     for c in mock_s3.head_object.call_args_list]
        self.assertEqual(len(head_keys), 10, f"expected 10 HEAD calls, got {len(head_keys)}")

    @patch("handler_storage.s3")
    def test_render_summary_returns_existing_artifact_urls(self, mock_s3):
        """Existing artifacts get presigned URLs, sizes, dimensions."""
        from handler_storage import handle_render_summary

        def mock_head(**kwargs):
            key = kwargs["Key"]
            if "image.jpeg" in key:
                return {"ContentLength": 1234, "ContentType": "image/jpeg",
                        "Metadata": {"width": "4096", "height": "4096"}}
            if "preview_color" in key:
                return {"ContentLength": 500, "ContentType": "image/png", "Metadata": {}}
            raise Exception("NoSuchKey")

        mock_s3.head_object.side_effect = mock_head
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_s3.generate_presigned_url.return_value = "https://signed"

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        cj = body["artifacts"]["color_jpeg"]
        self.assertTrue(cj["exists"])
        self.assertEqual(cj["size"], 1234)
        self.assertEqual(cj["width"], 4096)
        self.assertIsNotNone(cj["url"])

        pc = body["artifacts"]["preview_color_png"]
        self.assertTrue(pc["exists"])

    @patch("handler_storage.s3")
    def test_render_summary_missing_artifacts_are_false(self, mock_s3):
        """Missing artifacts return exists=false."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")
        mock_s3.get_object.side_effect = Exception("NoSuchKey")

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        for name, art in body["artifacts"].items():
            self.assertFalse(art["exists"], f"{name} should not exist")
            self.assertIsNone(art["url"])

    @patch("handler_storage.s3")
    def test_render_summary_reads_calc_server_side(self, mock_s3):
        """calc.json is read server-side, not presigned for browser."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")

        calc_json = json.dumps({"N": 5000, "n1": 5000, "degree": 70}).encode()
        def mock_get(**kwargs):
            key = kwargs["Key"]
            if "calc.json" in key:
                return {"Body": MagicMock(read=lambda: calc_json)}
            raise Exception("NoSuchKey")
        mock_s3.get_object.side_effect = mock_get

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        self.assertTrue(body["calc"]["exists"])
        self.assertEqual(body["calc"]["N"], 5000)
        self.assertEqual(body["calc"]["degree"], 70)

    @patch("handler_storage.s3")
    def test_render_summary_uses_deepzoom_latest_pointer_only(self, mock_s3):
        """DeepZoom info comes from renders/{job}/deepzoom_latest.json, no listing."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")

        dz_json = json.dumps({
            "dzi_url": "https://dz/image.dzi",
            "export_id": "dz_123",
            "created_at": "2026-03-26T10:00:00Z",
            "width": 8192, "height": 8192,
            "tiles_uploaded": 400,
        }).encode()
        def mock_get(**kwargs):
            key = kwargs["Key"]
            if "deepzoom_latest.json" in key:
                return {"Body": MagicMock(read=lambda: dz_json)}
            raise Exception("NoSuchKey")
        mock_s3.get_object.side_effect = mock_get

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        self.assertTrue(body["deepzoom_latest"]["exists"])
        self.assertEqual(body["deepzoom_latest"]["dzi_url"], "https://dz/image.dzi")
        mock_s3.get_paginator.assert_not_called()

    @patch("handler_storage.s3")
    def test_render_summary_missing_deepzoom_pointer_not_error(self, mock_s3):
        """Missing deepzoom_latest.json → exists=false, no error."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")
        mock_s3.get_object.side_effect = Exception("NoSuchKey")

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        self.assertFalse(body["deepzoom_latest"]["exists"])

    @patch("handler_storage.s3")
    def test_render_summary_malformed_deepzoom_pointer_ignored(self, mock_s3):
        """Bad JSON in deepzoom_latest.json → exists=false, no crash."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")

        def mock_get(**kwargs):
            key = kwargs["Key"]
            if "deepzoom_latest.json" in key:
                return {"Body": MagicMock(read=lambda: b"NOT JSON{{")}
            raise Exception("NoSuchKey")
        mock_s3.get_object.side_effect = mock_get

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        self.assertFalse(body["deepzoom_latest"]["exists"])

    @patch("handler_storage.s3")
    def test_render_summary_deepzoom_pointer_has_expected_fields(self, mock_s3):
        """deepzoom_latest.json read returns all expected fields."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")

        dz_manifest = {
            "job_id": "j", "export_id": "dz_123",
            "created_at": "2026-03-26T10:00:00Z",
            "source_key": "renders/j/image.jpeg",
            "dzi_key": "deepzoom/j/dz_123/image.dzi",
            "dzi_url": "https://bucket.s3.amazonaws.com/deepzoom/j/dz_123/image.dzi",
            "tile_prefix": "deepzoom/j/dz_123/image_files",
            "width": 8192, "height": 8192,
            "tiles_uploaded": 400,
        }

        def mock_get(**kwargs):
            key = kwargs["Key"]
            if "deepzoom_latest.json" in key:
                return {"Body": MagicMock(read=lambda: json.dumps(dz_manifest).encode())}
            raise Exception("NoSuchKey")
        mock_s3.get_object.side_effect = mock_get

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])
        dz = body["deepzoom_latest"]

        self.assertTrue(dz["exists"])
        self.assertEqual(dz["export_id"], "dz_123")
        self.assertEqual(dz["created_at"], "2026-03-26T10:00:00Z")
        self.assertEqual(dz["source_key"], "renders/j/image.jpeg")
        self.assertEqual(dz["dzi_url"], "https://bucket.s3.amazonaws.com/deepzoom/j/dz_123/image.dzi")
        self.assertEqual(dz["width"], 8192)
        self.assertEqual(dz["height"], 8192)
        self.assertEqual(dz["tiles_uploaded"], 400)
        mock_s3.get_paginator.assert_not_called()


class TestDeepZoomExportPointerWrite(unittest.TestCase):
    """Test that handler_deepzoom_export writes both meta.json and deepzoom_latest.json."""

    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    @patch("handler_deepzoom_export.subprocess")
    def test_export_writes_meta_and_pointer(self, mock_subprocess, mock_s3, mock_report):
        """Running the export handler produces two JSON writes with the correct keys."""
        import handler_deepzoom_export as dze

        # Mock S3 download
        mock_s3.get_object.return_value = {
            "Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"fake image data"])
        }

        # Mock the dz_export subprocess
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({"width": 64, "height": 64})
        mock_subprocess.run.return_value = mock_result

        # Track put_object calls
        put_calls = []
        def track_put(**kwargs):
            put_calls.append(kwargs)
        mock_s3.put_object.side_effect = track_put

        # Mock file system operations needed by the handler
        import tempfile, shutil
        tmp_dir = tempfile.mkdtemp()
        dz_dir = os.path.join(tmp_dir, "dz")
        os.makedirs(dz_dir)
        # Create fake dzi and tile files
        dz_base = os.path.join(dz_dir, "image")
        with open(dz_base + ".dzi", "w") as f:
            f.write("<Image/>")
        tiles_dir = dz_base + "_files"
        os.makedirs(os.path.join(tiles_dir, "0"))
        with open(os.path.join(tiles_dir, "0", "0_0.png"), "wb") as f:
            f.write(b"fake tile")

        # Patch paths to use our temp dir
        with patch.object(os, 'makedirs', side_effect=lambda *a, **kw: None), \
             patch.object(os, 'remove', side_effect=lambda *a: None), \
             patch.object(os.path, 'exists', return_value=True), \
             patch.object(os.path, 'isdir', return_value=True), \
             patch.object(os, 'walk', return_value=[(tiles_dir + "/0", [], ["0_0.png"])]), \
             patch('shutil.rmtree'), \
             patch('builtins.open', MagicMock(return_value=MagicMock(
                 __enter__=MagicMock(return_value=MagicMock(read=lambda: b"fake", write=lambda x: None)),
                 __exit__=MagicMock(return_value=False)))):

            try:
                dze.handler({"body": json.dumps({
                    "job_id": "test_dz", "source_key": "renders/test_dz/image_bilevel.tif",
                    "export_id": "dz_test_123"
                })}, None)
            except Exception:
                pass  # may fail on cleanup, that's OK

        # Verify the two JSON writes
        json_puts = [c for c in put_calls
                     if c.get("ContentType") == "application/json"]
        meta_puts = [c for c in json_puts if "meta.json" in c.get("Key", "")]
        pointer_puts = [c for c in json_puts if "deepzoom_latest.json" in c.get("Key", "")]

        self.assertGreaterEqual(len(meta_puts), 1,
            "Expected at least one meta.json write, got: %s" % [c.get("Key") for c in json_puts])
        self.assertGreaterEqual(len(pointer_puts), 1,
            "Expected at least one deepzoom_latest.json write, got: %s" % [c.get("Key") for c in json_puts])

        # Verify pointer key is under renders/{job_id}/
        pointer_key = pointer_puts[0]["Key"]
        self.assertEqual(pointer_key, "renders/test_dz/deepzoom_latest.json")

        # Verify both writes have the same body
        meta_body = meta_puts[0].get("Body", "")
        pointer_body = pointer_puts[0].get("Body", "")
        self.assertEqual(meta_body, pointer_body, "meta.json and pointer should have same body")

        # Verify the body is valid JSON with expected fields
        manifest = json.loads(meta_body)
        self.assertEqual(manifest["job_id"], "test_dz")
        self.assertIn("dzi_url", manifest)
        self.assertIn("width", manifest)
        self.assertIn("height", manifest)

        shutil.rmtree(tmp_dir, ignore_errors=True)


if __name__ == "__main__":
    unittest.main()
