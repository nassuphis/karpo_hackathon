"""
Tests for the Lambda sparse pixel pipeline — dispatch, storage, DynamoDB status.

Run: cd polypaint/lambda && python -m pytest test_pipeline.py -v
"""
import json
import struct
import unittest
from unittest.mock import MagicMock, patch


# ── Test: handler_dispatch.py ──────────────────────────────────────────────


class TestDispatchHandler(unittest.TestCase):

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_fires_all_jobs(self, mock_client):
        from handler_dispatch import handler
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
        jobs = [{"job_id": "j", "stripe_idx": 0, "bin_key": "renders/j/stripe_0.bin"}]
        event = self._make_event({"target": "raster", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_finalize_target(self, mock_client):
        from handler_dispatch import handler
        jobs = [{"job_id": "j", "tile_idx": 0, "n_stripes": 10, "tile_w": 4096, "tile_h": 4096}]
        event = self._make_event({"target": "finalize", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_encode_target(self, mock_client):
        from handler_dispatch import handler
        jobs = [{"out_key": "renders/j/image.jpeg", "tile_grid": {}}]
        event = self._make_event({"target": "encode", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)


# ── Test: handler_storage.py (check_keys, check_status, clean_render) ──────


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
    def test_clean_render_deletes_raw_jpeg_pix_files(self, mock_s3):
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "Contents": [
                {"Key": "renders/j/tile_0000.raw"},
                {"Key": "renders/j/pix_0000_t0000.pix"},
                {"Key": "renders/j/image.jpeg"},
                {"Key": "renders/j/calc.json"},
                {"Key": "renders/j/stripe_0.bin"},
            ]
        }]
        mock_s3.delete_objects.return_value = {"Deleted": [
            {"Key": "renders/j/tile_0000.raw"},
            {"Key": "renders/j/pix_0000_t0000.pix"},
            {"Key": "renders/j/image.jpeg"},
        ]}
        event = {"body": json.dumps({"job_id": "j"})}
        result = handle_clean_render(event)
        body = json.loads(result["body"])
        self.assertEqual(body["deleted"], 3)

        # Verify which files were passed to delete_objects
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertNotIn("renders/j/calc.json", deleted_keys)
        self.assertNotIn("renders/j/stripe_0.bin", deleted_keys)
        self.assertIn("renders/j/pix_0000_t0000.pix", deleted_keys)


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

        event = {"body": json.dumps({"job_id": "j"})}
        result = handle_clean_render(event)
        body = json.loads(result["body"])

        self.assertEqual(body["deleted"], 1)
        self.assertEqual(body["ddb_deleted"], 2)
        mock_ddb.batch_write_item.assert_called_once()


# ── Test: handler_coeffgen.py ─────────────────────────────────────────────


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
    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_basic(self, mock_subprocess, mock_s3, mock_remove, mock_open):
        mock_open.side_effect = self._mock_open
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
            "function": "giga_1",
            "n1": 10,
            "n2": 10,
        })
        result = handler(event, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["job_id"], "test-job")
        self.assertEqual(body["coeffs_key"], "renders/test-job/coeffs.bin")
        self.assertEqual(body["n_coeffs"], 24)
        self.assertEqual(body["degree"], 24)
        self.assertEqual(body["coeffs_size"], 9600)

        # Verify sweep binary was called with correct spec
        call_args = mock_subprocess.run.call_args
        spec = json.loads(call_args[1]["input"])
        self.assertEqual(spec["mode"], "coeffgen")
        self.assertEqual(spec["function"], "giga_1")
        self.assertEqual(spec["n1"], 10)
        self.assertEqual(spec["n2"], 10)
        self.assertEqual(spec["i1_start"], 0)
        self.assertEqual(spec["i1_end"], 10)

    @patch("builtins.open")
    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_with_transforms(self, mock_subprocess, mock_s3, mock_remove, mock_open):
        mock_open.side_effect = self._mock_open
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
            "function": "giga_5",
            "param_transforms": ["unit_circle", "square"],
            "coeff_transforms": ["rev", "conj"],
            "n1": 10,
            "n2": 10,
        })
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 200)

        # Verify transforms passed through to sweep spec
        spec = json.loads(mock_subprocess.run.call_args[1]["input"])
        self.assertEqual(spec["param_transforms"], ["unit_circle", "square"])
        self.assertEqual(spec["coeff_transforms"], ["rev", "conj"])

    @patch("builtins.open")
    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_s3_upload(self, mock_subprocess, mock_s3, mock_remove, mock_open):
        mock_open.side_effect = self._mock_open
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
            "function": "giga_1",
            "n1": 5,
            "n2": 5,
        })
        handler(event, None)

        # Verify S3 upload was called with correct key
        mock_s3.put_object.assert_called_once()
        call_kwargs = mock_s3.put_object.call_args[1]
        self.assertEqual(call_kwargs["Key"], "renders/upload-test/coeffs.bin")
        self.assertEqual(call_kwargs["ContentType"], "application/octet-stream")

    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_sweep_failure(self, mock_subprocess, mock_s3):
        from handler_coeffgen import handler
        mock_result = MagicMock()
        mock_result.returncode = 1
        mock_result.stderr = "bad function name"
        mock_subprocess.run.return_value = mock_result

        event = self._make_event({
            "job_id": "fail-test",
            "function": "bad_func",
            "n1": 5,
            "n2": 5,
        })
        with self.assertRaises(RuntimeError) as ctx:
            handler(event, None)
        self.assertIn("coeffgen failed", str(ctx.exception))

    @patch("builtins.open")
    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess")
    def test_coeffgen_default_transforms(self, mock_subprocess, mock_s3, mock_remove, mock_open):
        """Omitting transforms defaults to empty lists."""
        mock_open.side_effect = self._mock_open
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
            "function": "giga_1",
            "n1": 5,
            "n2": 5,
        })
        handler(event, None)

        spec = json.loads(mock_subprocess.run.call_args[1]["input"])
        self.assertEqual(spec["param_transforms"], [])
        self.assertEqual(spec["coeff_transforms"], [])


# ── Test: handler_sweep.py (solve-from-coefficients path) ────────────────


class TestSolveFromCoeffs(unittest.TestCase):

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    @patch("handler_sweep.os.remove")
    @patch("handler_sweep.s3")
    @patch("handler_sweep.subprocess")
    def test_solve_routes_on_coeffs_key(self, mock_subprocess, mock_s3, mock_remove):
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

        # Mock S3 range read
        mock_body = MagicMock()
        mock_body.read.return_value = b"\x00" * 1000
        mock_s3.get_object.return_value = {"Body": mock_body}

        # Mock file reads for bin upload
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
                "coeffs_key": "renders/solve-test/coeffs.bin",
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
        # i1_start should be 0 (offset already handled by range read)
        self.assertEqual(spec["i1_start"], 0)
        self.assertEqual(spec["i1_end"], 10)

    @patch("handler_sweep.os.remove")
    @patch("handler_sweep.s3")
    @patch("handler_sweep.subprocess")
    def test_solve_s3_range_read(self, mock_subprocess, mock_s3, mock_remove):
        """Verify correct S3 byte range calculation for coefficient read."""
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
                "job_id": "range-test",
                "stripe_idx": 2,
                "coeffs_key": "renders/range-test/coeffs.bin",
                "n_coeffs": 10,
                "n1": 100,
                "n2": 50,
                "i1_start": 20,
                "i1_end": 30,
            })
            result = handler(event, None)

        # Verify S3 range read
        # bytes_per_row = n2 * n_coeffs * 2 * 4 = 50 * 10 * 2 * 4 = 4000
        # start_byte = 20 * 4000 = 80000
        # end_byte = 30 * 4000 - 1 = 119999
        call_kwargs = mock_s3.get_object.call_args[1]
        self.assertEqual(call_kwargs["Range"], "bytes=80000-119999")
        self.assertEqual(call_kwargs["Key"], "renders/range-test/coeffs.bin")

    @patch("handler_sweep.os.remove")
    @patch("handler_sweep.s3")
    @patch("handler_sweep.subprocess")
    def test_solve_custom_s3_key(self, mock_subprocess, mock_s3, mock_remove):
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
                "coeffs_key": "renders/key-test/coeffs.bin",
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
        upload_kwargs = mock_s3.put_object.call_args[1]
        self.assertEqual(upload_kwargs["Key"], "renders/key-test/lores.bin")
        self.assertEqual(body["s3_key"], "renders/key-test/lores.bin")

    @patch("handler_sweep.os.remove")
    @patch("handler_sweep.s3")
    @patch("handler_sweep.subprocess")
    def test_solve_failure_raises(self, mock_subprocess, mock_s3, mock_remove):
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
                "coeffs_key": "renders/fail-test/coeffs.bin",
                "n_coeffs": 5,
                "n1": 10,
                "n2": 10,
                "i1_start": 0,
                "i1_end": 5,
            })
            with self.assertRaises(RuntimeError) as ctx:
                handler(event, None)
            self.assertIn("solve failed", str(ctx.exception))

    @patch("handler_sweep.multiprocessing")
    @patch("handler_sweep.os.remove")
    @patch("handler_sweep.s3")
    @patch("handler_sweep.subprocess")
    def test_grid_mode_still_works(self, mock_subprocess, mock_s3, mock_remove, mock_mp):
        """Without coeffs_key, handler uses existing grid path."""
        from handler_sweep import handler
        mock_mp.cpu_count.return_value = 1
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "n_t": 100,
            "degree": 24,
            "avg_iterations": 10.0,
        })
        mock_subprocess.run.return_value = mock_result

        import builtins
        original_open = builtins.open

        def mock_open(path, mode="r", **kwargs):
            if "/tmp/" in str(path) and "b" in mode:
                m = MagicMock()
                m.__enter__ = MagicMock(return_value=m)
                m.__exit__ = MagicMock(return_value=False)
                m.read = MagicMock(return_value=b"\x00" * 200)
                return m
            return original_open(path, mode, **kwargs)

        with patch("builtins.open", side_effect=mock_open):
            event = self._make_event({
                "job_id": "grid-test",
                "stripe_idx": 0,
                "function": "giga_1",
                "n1": 10,
                "n2": 10,
                "i1_start": 0,
                "i1_end": 5,
            })
            result = handler(event, None)

        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 200)

        # Verify sweep was called in grid mode (not solve)
        spec = json.loads(mock_subprocess.run.call_args[1]["input"])
        self.assertEqual(spec["mode"], "grid")


if __name__ == "__main__":
    unittest.main()
