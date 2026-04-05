"""
Tests for the Lambda sparse pixel pipeline — dispatch, storage, DynamoDB status.

Run: cd polypaint && python -m pytest tests/ -v
"""
import json
import os
import struct
import sys
import time
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

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_sweep_mt_target(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"job_id": "j", "chunk_idx": 0, "coeffs_key": "renders/j/coeffs_0000.bin", "n_coeffs": 24, "n_steps": 1000}]
        event = self._make_event({"target": "sweep_mt", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_repalette_target(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"job_id": "j", "task_id": "repalette_1", "source_palette_id": "pal_src", "new_palette": "tri_redgold"}]
        event = self._make_event({"target": "repalette", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_color_repalette_target(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"job_id": "j", "task_id": "color_repalette_1", "artifact_id": "color_new", "source_artifact_id": "color_src", "source_image_key": "renders/j/color/color_src/image.jpeg", "new_palette": "tri_redgold"}]
        event = self._make_event({"target": "color_repalette", "jobs": jobs})
        result = handler(event, None)
        body = json.loads(result["body"])
        self.assertEqual(body["fired"], 1)

    @patch("handler_dispatch.lambda_client")
    def test_dispatch_pdf_artifact_target(self, mock_client):
        from handler_dispatch import handler
        mock_client.invoke.return_value = {"StatusCode": 202}
        jobs = [{"job_id": "j", "task_id": "pdf_1", "artifact_id": "pdf_new", "source_artifact_id": "color_src", "source_image_key": "renders/j/color/color_src/image.jpeg"}]
        event = self._make_event({"target": "pdf_artifact", "jobs": jobs})
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

    @patch("handler_storage.s3")
    def test_list_returns_only_table_fields(self, mock_s3):
        """Verify /list returns lean table fields, no preview/image checks."""
        from handler_storage import handle_list
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "CommonPrefixes": [{"Prefix": "renders/img_job/"}]
        }]
        mock_s3.get_object.return_value = {
            "Body": MagicMock(read=MagicMock(return_value=json.dumps({
                "function": "giga_1", "degree": 24, "n1": 100,
                "n_stripes": 1,
            }).encode()))
        }

        result = handle_list({"body": "{}"})
        body = json.loads(result["body"])
        r = body["results"][0]
        self.assertEqual(r["function"], "giga_1")
        self.assertEqual(r["degree"], 24)
        self.assertEqual(r["N"], 100)
        # Must NOT contain preview/image fields (moved to /detail)
        self.assertNotIn("has_preview", r)
        self.assertNotIn("has_image", r)
        self.assertNotIn("preview_url", r)
        self.assertNotIn("pipeline", r)

    @patch("handler_storage._key_exists")
    @patch("handler_storage.s3")
    def test_detail_returns_preview_and_pipeline(self, mock_s3, mock_exists):
        """Verify /detail returns preview info and pipeline metadata."""
        from handler_storage import handle_detail
        mock_exists.side_effect = lambda k: "preview.png" in k
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"KeyCount": 5}]
        mock_s3.generate_presigned_url.return_value = "https://signed"

        calc_data = json.dumps({
            "function": "giga_1", "degree": 24, "times": 3,
            "pipeline": {"param_transforms": [["rotate", "90"]], "cfpv": [1, 2]},
        }).encode()
        view_data = json.dumps({"q_re": [-1, 1], "q_im": [-1, 1]}).encode()
        preview_stats_data = json.dumps({
            "n_roots": 5000, "n_roots_total": 7500,
            "q_re": [-0.5, 0.5], "q_im": [-0.5, 0.5],
            "created_at": "2026-03-31T10:00:00Z",
        }).encode()

        def mock_get(**kwargs):
            key = kwargs["Key"]
            if "calc.json" in key:
                return {"Body": MagicMock(read=lambda: calc_data)}
            if "view.json" in key:
                return {"Body": MagicMock(read=lambda: view_data)}
            if "preview_stats.json" in key:
                return {"Body": MagicMock(read=lambda: preview_stats_data)}
            raise Exception("NoSuchKey")
        mock_s3.get_object.side_effect = mock_get

        result = handle_detail({"body": json.dumps({"job_id": "j"})})
        body = json.loads(result["body"])
        self.assertTrue(body["has_preview"])
        self.assertEqual(body["preview_url"], "https://signed")
        self.assertEqual(body["times"], 3)
        self.assertEqual(body["param_transforms"], [["rotate", "90"]])
        self.assertIn("pipeline", body)
        self.assertEqual(body["q_re"], [-1, 1])
        # preview_stats loaded from S3
        self.assertIn("preview_stats", body)
        self.assertEqual(body["preview_stats"]["n_roots"], 5000)
        self.assertEqual(body["preview_stats"]["n_roots_total"], 7500)
        self.assertEqual(body["preview_stats"]["q_re"], [-0.5, 0.5])

    @patch("handler_storage._key_exists")
    @patch("handler_storage.s3")
    def test_detail_without_preview_stats(self, mock_s3, mock_exists):
        """Verify /detail works when preview_stats.json is absent."""
        from handler_storage import handle_detail
        mock_exists.return_value = False
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"KeyCount": 1}]

        def mock_get(**kwargs):
            raise Exception("NoSuchKey")
        mock_s3.get_object.side_effect = mock_get

        result = handle_detail({"body": json.dumps({"job_id": "j"})})
        body = json.loads(result["body"])
        self.assertFalse(body["has_preview"])
        self.assertNotIn("preview_stats", body)


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
        """Clean-render deletes only intermediates for color runs."""
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
        self.assertIn("renders/j/solve_proximity_clip.json", deleted_keys)
        self.assertIn("renders/j/solve_proximity_bins.json", deleted_keys)
        self.assertNotIn("renders/j/preview_color.png", deleted_keys)
        self.assertNotIn("renders/j/image.jpeg", deleted_keys)

    @patch("handler_storage.s3")
    def test_clean_render_color_preserves_final_outputs(self, mock_s3):
        """Color pipeline no longer deletes previews or prior final artifacts."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "color"})}
        handle_clean_render(event)
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertNotIn("renders/j/preview_color.png", deleted_keys)
        self.assertNotIn("renders/j/image.jpeg", deleted_keys)
        self.assertNotIn("renders/j/image.png", deleted_keys)
        self.assertNotIn("renders/j/preview_bilevel.png", deleted_keys)

    @patch("handler_storage.s3")
    def test_clean_render_bilevel_with_no_intermediates_is_noop(self, mock_s3):
        """Bilevel pipeline leaves finals alone and can no-op when no intermediates exist."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "bilevel"})}
        handle_clean_render(event)
        mock_s3.delete_objects.assert_not_called()

    @patch("handler_storage.s3")
    def test_clean_render_bilevel_deletes_stale_bits_and_tiles(self, mock_s3):
        """Bilevel cleanup must purge old job-scoped .bits intermediates before a new render."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator

        def paginate(Bucket, Prefix):
            mapping = {
                "renders/j/bilevel_t": [{"Contents": [{"Key": "renders/j/bilevel_t0000.tif"}]}],
                "renders/j/bits_chunk_": [{"Contents": [{"Key": "renders/j/bits_chunk_0000_t0000.bits"}]}],
            }
            return mapping.get(Prefix, [{"Contents": []}])

        mock_paginator.paginate.side_effect = paginate
        mock_s3.delete_objects.return_value = {"Deleted": [
            {"Key": "renders/j/bilevel_t0000.tif"},
            {"Key": "renders/j/bits_chunk_0000_t0000.bits"},
        ]}

        event = {"body": json.dumps({"job_id": "j", "pipeline": "bilevel"})}
        result = handle_clean_render(event)
        body = json.loads(result["body"])
        self.assertEqual(body["deleted"], 2)

        deleted_keys = [o["Key"] for o in mock_s3.delete_objects.call_args[1]["Delete"]["Objects"]]
        self.assertIn("renders/j/bilevel_t0000.tif", deleted_keys)
        self.assertIn("renders/j/bits_chunk_0000_t0000.bits", deleted_keys)
        self.assertNotIn("renders/j/image_bilevel.tif", deleted_keys)

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
        self.assertIn("renders/j/solve_proximity_clip.json", deleted_keys)
        self.assertNotIn("renders/j/preview_bilevel.png", deleted_keys)

    @patch("handler_storage.s3")
    def test_color_cleanup_preserves_stale_format_siblings(self, mock_s3):
        """Color cleanup preserves prior finals because artifacts are immutable."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "color"})}
        handle_clean_render(event)
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertNotIn("renders/j/image.jpeg", deleted_keys)
        self.assertNotIn("renders/j/image.png", deleted_keys)
        # Cross-family finals must NOT be deleted
        self.assertNotIn("renders/j/image_bilevel.tif", deleted_keys)
        self.assertNotIn("renders/j/image_coeffs_bilevel.tif", deleted_keys)
        self.assertNotIn("renders/j/image_palette.jpeg", deleted_keys)

    @patch("handler_storage.s3")
    def test_clean_render_deletes_solve_proximity_artifacts(self, mock_s3):
        """Clean-render deletes legacy solve_proximity intermediate artifacts."""
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

    @patch("handler_storage.s3")
    def test_clean_render_deletes_solve_scores_artifacts(self, mock_s3):
        """Clean-render deletes solve_scores/ intermediate artifacts (new naming)."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": [
            {"Key": "renders/j/solve_scores/proximity/stripe_0_hist.json"},
            {"Key": "renders/j/solve_scores/proximity_clip.json"},
        ]}]
        mock_s3.delete_objects.return_value = {"Deleted": [
            {"Key": "renders/j/solve_scores/proximity/stripe_0_hist.json"},
            {"Key": "renders/j/solve_scores/proximity_clip.json"},
        ]}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "color"})}
        handle_clean_render(event)
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertIn("renders/j/solve_scores/proximity/stripe_0_hist.json", deleted_keys)
        self.assertIn("renders/j/solve_scores/proximity_clip.json", deleted_keys)


    @patch("handler_storage.s3")
    def test_color_cleanup_preserves_palette_artifacts(self, mock_s3):
        """Color cleanup must NOT delete palette family artifacts."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "color"})}
        handle_clean_render(event)
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertNotIn("renders/j/image_palette.jpeg", deleted_keys)
        self.assertNotIn("renders/j/preview_palette.png", deleted_keys)

    @patch("handler_storage.s3")
    def test_color_cleanup_preserves_bilevel_artifacts(self, mock_s3):
        """Color cleanup must NOT delete bilevel/coeff family artifacts."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "color"})}
        handle_clean_render(event)
        call_args = mock_s3.delete_objects.call_args
        deleted_keys = [o["Key"] for o in call_args[1]["Delete"]["Objects"]]
        self.assertNotIn("renders/j/preview_bilevel.png", deleted_keys)
        self.assertNotIn("renders/j/preview_coeffs.png", deleted_keys)

    @patch("handler_storage.s3")
    def test_bilevel_cleanup_preserves_other_families(self, mock_s3):
        """Bilevel cleanup no longer deletes final artifacts from any family."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "bilevel"})}
        handle_clean_render(event)
        mock_s3.delete_objects.assert_not_called()

    @patch("handler_storage.s3")
    def test_coeff_cleanup_preserves_other_families(self, mock_s3):
        """Coeff cleanup no longer deletes final artifacts from any family."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"Contents": []}]
        mock_s3.delete_objects.return_value = {"Deleted": []}
        event = {"body": json.dumps({"job_id": "j", "pipeline": "coeff_bilevel"})}
        handle_clean_render(event)
        mock_s3.delete_objects.assert_not_called()

    @patch("handler_storage.s3")
    def test_coeff_cleanup_deletes_stale_coeff_bits_and_tiles(self, mock_s3):
        """Coeff bilevel cleanup must purge old coeff bitsets before a new render."""
        from handler_storage import handle_clean_render
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator

        def paginate(Bucket, Prefix):
            mapping = {
                "renders/j/coeff_t": [{"Contents": [{"Key": "renders/j/coeff_t0000.tif"}]}],
                "renders/j/coeff_bits_chunk_": [{"Contents": [{"Key": "renders/j/coeff_bits_chunk_0000_t0000.bits"}]}],
            }
            return mapping.get(Prefix, [{"Contents": []}])

        mock_paginator.paginate.side_effect = paginate
        mock_s3.delete_objects.return_value = {"Deleted": [
            {"Key": "renders/j/coeff_t0000.tif"},
            {"Key": "renders/j/coeff_bits_chunk_0000_t0000.bits"},
        ]}

        event = {"body": json.dumps({"job_id": "j", "pipeline": "coeff_bilevel"})}
        result = handle_clean_render(event)
        body = json.loads(result["body"])
        self.assertEqual(body["deleted"], 2)

        deleted_keys = [o["Key"] for o in mock_s3.delete_objects.call_args[1]["Delete"]["Objects"]]
        self.assertIn("renders/j/coeff_t0000.tif", deleted_keys)
        self.assertIn("renders/j/coeff_bits_chunk_0000_t0000.bits", deleted_keys)
        self.assertNotIn("renders/j/image_coeffs_bilevel.tif", deleted_keys)


class TestRenderSummaryPalette(unittest.TestCase):

    @patch("handler_storage.s3")
    def test_render_summary_includes_palette_jpeg(self, mock_s3):
        """render-summary HEAD-checks palette_jpeg and preview_palette_png."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_s3.generate_presigned_url.return_value = "https://fake"
        event = {"body": json.dumps({"job_id": "j"})}
        result = handle_render_summary(event)
        body = json.loads(result["body"])
        artifacts = body["artifacts"]
        self.assertIn("palette_jpeg", artifacts)
        self.assertIn("preview_palette_png", artifacts)
        # Both should show exists=False since all HEAD calls raise
        self.assertFalse(artifacts["palette_jpeg"]["exists"])
        self.assertFalse(artifacts["preview_palette_png"]["exists"])


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
        self.assertIn("updated_at_ms", item)
        self.assertTrue(int(item["updated_at_ms"]["N"]) > 0)
        shared._ddb = None

    @patch("shared._ddb", None)
    @patch("shared.boto3")
    def test_report_status_writes_updated_at_ms(self, mock_boto3):
        import shared
        shared._ddb = None
        mock_ddb = MagicMock()
        mock_boto3.client.return_value = mock_ddb
        shared.report_status("j", "t", "started")
        item = mock_ddb.put_item.call_args[1]["Item"]
        self.assertIn("updated_at_ms", item)
        ms = int(item["updated_at_ms"]["N"])
        # Should be a recent epoch ms (within last 10 seconds)
        import time
        now_ms = int(time.time() * 1000)
        self.assertAlmostEqual(ms, now_ms, delta=10000)
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
    def test_freshness_fields_with_timestamps(self, mock_get_ddb):
        """check-status returns latest_update_ms, latest_done_ms, latest_nonterminal_ms, stale_for_ms."""
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        now = int(time.time() * 1000)
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "r_0"}, "task_status": {"S": "done"}, "updated_at_ms": {"N": str(now - 5000)}},
                {"task_id": {"S": "r_1"}, "task_status": {"S": "started"}, "updated_at_ms": {"N": str(now - 2000)}},
                {"task_id": {"S": "r_2"}, "task_status": {"S": "error"}, "error_msg": {"S": "fail"},
                 "updated_at_ms": {"N": str(now - 10000)}},
            ],
        }
        event = {"body": json.dumps({"job_id": "j", "task_prefix": "r_", "expected": 3})}
        body = json.loads(handle_check_status(event)["body"])
        self.assertEqual(body["done"], 1)
        self.assertEqual(body["errors"], 1)
        self.assertAlmostEqual(body["latest_update_ms"], now - 2000, delta=100)
        self.assertAlmostEqual(body["latest_done_ms"], now - 5000, delta=100)
        self.assertAlmostEqual(body["latest_nonterminal_ms"], now - 2000, delta=100)
        self.assertAlmostEqual(body["stale_for_ms"], 2000, delta=1000)
        self.assertEqual(body["newest_task"]["task_id"], "r_1")

    @patch("handler_storage._get_ddb")
    def test_stuck_entries_include_age(self, mock_get_ddb):
        """Stuck entries include updated_at_ms and age_ms."""
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        now = int(time.time() * 1000)
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "r_0"}, "task_status": {"S": "started"}, "updated_at_ms": {"N": str(now - 60000)}},
            ],
        }
        event = {"body": json.dumps({"job_id": "j", "task_prefix": "r_", "expected": 1})}
        body = json.loads(handle_check_status(event)["body"])
        s = body["stuck"][0]
        self.assertIn("updated_at_ms", s)
        self.assertAlmostEqual(s["age_ms"], 60000, delta=2000)

    @patch("handler_storage._get_ddb")
    def test_missing_updated_at_ms_rows(self, mock_get_ddb):
        """Rows without updated_at_ms don't crash aggregation."""
        from handler_storage import handle_check_status
        mock_ddb = MagicMock()
        mock_get_ddb.return_value = mock_ddb
        mock_ddb.query.return_value = {
            "Items": [
                {"task_id": {"S": "r_0"}, "task_status": {"S": "done"}},
            ],
        }
        event = {"body": json.dumps({"job_id": "j", "task_prefix": "r_", "expected": 1})}
        body = json.loads(handle_check_status(event)["body"])
        self.assertIsNone(body["latest_update_ms"])
        self.assertIsNone(body["stale_for_ms"])

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


class TestSolveFromCoeffsMT(unittest.TestCase):

    def _make_event(self, body):
        return {"body": json.dumps(body)}

    @patch("handler_sweep_mt.report_status")
    @patch("handler_sweep_mt.os.path.getsize", return_value=400)
    @patch("handler_sweep_mt.os.remove")
    @patch("handler_sweep_mt.s3")
    @patch("handler_sweep_mt.subprocess")
    def test_solve_mt_routes_on_coeffs_key(self, mock_subprocess, mock_s3, mock_remove, mock_getsize, mock_ddb):
        from handler_sweep_mt import handler
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({
            "n_t": 500,
            "degree": 24,
            "avg_iterations": 12.3,
            "n_threads": 4,
        })
        mock_subprocess.run.return_value = mock_result

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
                "job_id": "solve-mt-test",
                "stripe_idx": 0,
                "coeffs_key": "renders/solve-mt-test/coeffs_0000.bin",
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
        self.assertEqual(body["n_threads"], 4)
        self.assertEqual(body["n_procs"], 4)

        spec = json.loads(mock_subprocess.run.call_args[1]["input"])
        self.assertEqual(spec["mode"], "solve_mt")
        self.assertEqual(spec["n_coeffs"], 24)
        self.assertEqual(spec["i1_start"], 0)
        self.assertEqual(spec["i1_end"], 1)
        self.assertEqual(spec["n2"], 1000)
        self.assertFalse(spec["match_roots"])


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

        # Verify PNG + preview_stats.json uploaded to S3
        self.assertEqual(mock_s3.put_object.call_count, 2)
        put_calls = {c[1]["Key"]: c[1] for c in mock_s3.put_object.call_args_list}
        # PNG
        self.assertIn("renders/test/preview.png", put_calls)
        png_call = put_calls["renders/test/preview.png"]
        self.assertEqual(png_call["ContentType"], "image/png")
        self.assertTrue(png_call["Body"][:4] == b'\x89PNG')
        # Preview stats
        self.assertIn("renders/test/preview_stats.json", put_calls)
        stats_call = put_calls["renders/test/preview_stats.json"]
        self.assertEqual(stats_call["ContentType"], "application/json")
        stats = json.loads(stats_call["Body"])
        self.assertIn("n_roots", stats)
        self.assertIn("n_roots_total", stats)
        self.assertIn("q_re", stats)
        self.assertIn("q_im", stats)
        self.assertIn("created_at", stats)

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
    def test_render_summary_returns_empty_family_catalogs_when_no_artifacts(self, mock_s3):
        """render-summary returns empty family catalogs when nothing exists."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_s3.generate_presigned_url.return_value = "https://fake"
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"CommonPrefixes": []}]

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        self.assertEqual(body["schema_version"], 2)
        self.assertEqual(body["families"], {"color": [], "bilevel": [], "coeffs": [], "palette": [], "pdf": []})
        self.assertGreaterEqual(mock_s3.get_paginator.call_count, 5)

    @patch("handler_storage.s3")
    def test_render_summary_returns_existing_artifact_urls(self, mock_s3):
        """Existing immutable artifacts are returned in the family catalogs."""
        from handler_storage import handle_render_summary

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator

        def paginate_side_effect(**kwargs):
            prefix = kwargs.get("Prefix", "")
            if prefix == "renders/j/color/":
                return [{"CommonPrefixes": [{"Prefix": "renders/j/color/color_run_1/"}]}]
            return [{"CommonPrefixes": []}]

        mock_paginator.paginate.side_effect = paginate_side_effect

        def mock_head(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/color_run_1/image.jpeg":
                return {"ContentLength": 1234, "ContentType": "image/jpeg",
                        "Metadata": {"width": "4096", "height": "4096", "artifact_id": "color_run_1", "created_at": "2026-03-31T10:00:00Z", "family": "color", "color_mode": "rainbow", "format": "jpeg", "root_transforms": "[]", "view_mode": "auto", "quantile": "0.01", "shim": "0.07", "rotation": "-1.57079632679", "match_mode": "greedy", "background_color": "000000", "background_threshold": "4"}}
            if key == "renders/j/color/color_run_1/preview.png":
                return {"ContentLength": 500, "ContentType": "image/png", "Metadata": {}}
            raise Exception("NoSuchKey")

        mock_s3.head_object.side_effect = mock_head
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_s3.generate_presigned_url.return_value = "https://signed"

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        cj = body["families"]["color"][0]
        self.assertEqual(cj["artifact_id"], "color_run_1")
        self.assertEqual(cj["file_size"], 1234)
        self.assertEqual(cj["width"], 4096)
        self.assertEqual(cj["viewer_url"], "https://signed")
        self.assertEqual(cj["view_mode"], "auto")
        self.assertAlmostEqual(cj["quantile"], 0.01)
        self.assertAlmostEqual(cj["shim"], 0.07)
        self.assertAlmostEqual(cj["rotation"], -1.57079632679)
        self.assertEqual(cj["match_mode"], "greedy")
        self.assertEqual(cj["background_color"], "000000")
        self.assertAlmostEqual(cj["background_threshold"], 4.0)

    @patch("handler_storage.s3")
    def test_render_summary_returns_pdf_family_entries(self, mock_s3):
        from handler_storage import handle_render_summary

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator

        def paginate_side_effect(**kwargs):
            prefix = kwargs.get("Prefix", "")
            if prefix == "renders/j/pdf/":
                return [{"CommonPrefixes": [{"Prefix": "renders/j/pdf/pdf_1/"}]}]
            return [{"CommonPrefixes": []}]

        mock_paginator.paginate.side_effect = paginate_side_effect

        def mock_head(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/pdf/pdf_1/document.pdf":
                return {"ContentLength": 3456, "ContentType": "application/pdf",
                        "Metadata": {"artifact_id": "pdf_1", "family": "pdf", "created_at": "2026-04-04T10:00:00Z", "format": "pdf", "pdf_kind": "color_spread", "source_family": "color", "source_artifact_id": "color_src", "source_display_name": "solve:clusteriness q=5.0% w=1 inferno", "source_color_mode": "solve_score", "source_palette": "inferno", "source_solve_metric": "clusteriness", "source_solve_score_quantile": "0.05", "source_solve_score_omega": "1", "page_count": "1"}}
            raise Exception("NoSuchKey")

        mock_s3.head_object.side_effect = mock_head
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_s3.generate_presigned_url.return_value = "https://signed"

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        self.assertIn("pdf", body["families"])
        pdfs = body["families"]["pdf"]
        self.assertEqual(len(pdfs), 1)
        art = pdfs[0]
        self.assertEqual(art["artifact_id"], "pdf_1")
        self.assertEqual(art["family"], "pdf")
        self.assertEqual(art["format"], "pdf")
        self.assertEqual(art["content_type"], "application/pdf")
        self.assertEqual(art["pdf_kind"], "color_spread")
        self.assertEqual(art["source_artifact_id"], "color_src")
        self.assertEqual(art["source_display_name"], "solve:clusteriness q=5.0% w=1 inferno")
        self.assertEqual(art["viewer_url"], "https://signed")

    @patch("handler_storage.s3")
    def test_render_summary_parses_solve_score_omega(self, mock_s3):
        from handler_storage import handle_render_summary

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator

        def paginate_side_effect(**kwargs):
            prefix = kwargs.get("Prefix", "")
            if prefix == "renders/j/color/":
                return [{"CommonPrefixes": [{"Prefix": "renders/j/color/color_run_ss/"}]}]
            return [{"CommonPrefixes": []}]

        mock_paginator.paginate.side_effect = paginate_side_effect

        def mock_head(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/color_run_ss/image.jpeg":
                return {"ContentLength": 1234, "ContentType": "image/jpeg",
                        "Metadata": {"width": "4096", "height": "4096", "artifact_id": "color_run_ss", "created_at": "2026-03-31T10:00:00Z", "family": "color", "color_mode": "solve_score", "format": "jpeg", "root_transforms": "[]", "view_mode": "auto", "quantile": "0.01", "shim": "0.07", "rotation": "0", "match_mode": "greedy", "solve_metric": "anisotropy", "solve_score_quantile": "0.02", "solve_score_omega": "6"}}
            if key == "renders/j/color/color_run_ss/preview.png":
                return {"ContentLength": 500, "ContentType": "image/png", "Metadata": {}}
            raise Exception("NoSuchKey")

        mock_s3.head_object.side_effect = mock_head
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_s3.generate_presigned_url.return_value = "https://signed"

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])
        cj = body["families"]["color"][0]
        self.assertEqual(cj["solve_metric"], "anisotropy")
        self.assertAlmostEqual(cj["solve_score_quantile"], 0.02)
        self.assertAlmostEqual(cj["solve_score_omega"], 6.0)

    @patch("handler_storage.s3")
    def test_render_summary_exposes_color_repalette_metadata(self, mock_s3):
        from handler_storage import handle_render_summary

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator

        def paginate_side_effect(**kwargs):
            prefix = kwargs.get("Prefix", "")
            if prefix == "renders/j/color/":
                return [{"CommonPrefixes": [{"Prefix": "renders/j/color/color_repal/"}]}]
            return [{"CommonPrefixes": []}]

        mock_paginator.paginate.side_effect = paginate_side_effect

        def mock_head(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/color_repal/image.jpeg":
                return {
                    "ContentLength": 1234,
                    "ContentType": "image/jpeg",
                    "Metadata": {
                        "width": "4096",
                        "height": "4096",
                        "artifact_id": "color_repal",
                        "created_at": "2026-04-03T10:00:00Z",
                        "family": "color",
                        "color_mode": "solve_score",
                        "format": "jpeg",
                        "root_transforms": "[]",
                        "view_mode": "auto",
                        "quantile": "0.01",
                        "shim": "0.07",
                        "rotation": "0",
                        "match_mode": "greedy",
                        "solve_metric": "crowding",
                        "solve_score_quantile": "0.02",
                        "solve_score_omega": "4",
                        "palette": "tri_redgold",
                        "repalette_capable": "true",
                        "pixel_bins_prefix": "renders/j/color/color_repal/pixel_bins/tile_",
                        "pixel_bins_empty": "255",
                        "pixel_bins_layout": "tile_u8_v1",
                        "derived_from_artifact_id": "color_src",
                        "derivation_kind": "color_repalette",
                    },
                }
            if key == "renders/j/color/color_repal/preview.png":
                return {"ContentLength": 500, "ContentType": "image/png", "Metadata": {}}
            raise Exception("NoSuchKey")

        mock_s3.head_object.side_effect = mock_head
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_s3.generate_presigned_url.return_value = "https://signed"

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])
        cj = body["families"]["color"][0]
        self.assertTrue(cj["repalette_capable"])
        self.assertEqual(cj["pixel_bins_prefix"], "renders/j/color/color_repal/pixel_bins/tile_")
        self.assertEqual(cj["pixel_bins_empty"], 255)
        self.assertEqual(cj["pixel_bins_layout"], "tile_u8_v1")
        self.assertEqual(cj["derived_from_artifact_id"], "color_src")
        self.assertEqual(cj["derivation_kind"], "color_repalette")

    @patch("handler_storage.s3")
    def test_render_summary_orders_autolevel_children_above_parent(self, mock_s3):
        from handler_storage import handle_render_summary

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator

        def paginate_side_effect(**kwargs):
            prefix = kwargs.get("Prefix", "")
            if prefix == "renders/j/color/":
                return [{
                    "CommonPrefixes": [
                        {"Prefix": "renders/j/color/color_base/"},
                        {"Prefix": "renders/j/color/autolevels_child/"},
                        {"Prefix": "renders/j/color/color_newer/"},
                    ]
                }]
            return [{"CommonPrefixes": []}]

        mock_paginator.paginate.side_effect = paginate_side_effect

        meta_by_key = {
            "renders/j/color/color_base/image.jpeg": {
                "ContentLength": 1100,
                "ContentType": "image/jpeg",
                "Metadata": {
                    "artifact_id": "color_base",
                    "created_at": "2026-04-02T10:00:00Z",
                    "family": "color",
                    "format": "jpeg",
                    "color_mode": "solve_score",
                    "solve_metric": "anisotropy",
                    "solve_score_quantile": "0.02",
                    "solve_score_omega": "6",
                    "palette": "tri_redgold",
                    "root_transforms": "[]",
                    "view_mode": "auto",
                    "quantile": "0.01",
                    "shim": "0.05",
                    "rotation": "0",
                    "pix": "3000",
                    "quality": "77",
                    "width": "3000",
                    "height": "3000",
                },
            },
            "renders/j/color/color_base/preview.png": {"ContentLength": 100, "ContentType": "image/png", "Metadata": {}},
            "renders/j/color/autolevels_child/image.jpeg": {
                "ContentLength": 1200,
                "ContentType": "image/jpeg",
                "Metadata": {
                    "artifact_id": "autolevels_child",
                    "created_at": "2026-04-02T10:05:00Z",
                    "family": "color",
                    "format": "jpeg",
                    "color_mode": "solve_score",
                    "solve_metric": "anisotropy",
                    "solve_score_quantile": "0.02",
                    "solve_score_omega": "6",
                    "palette": "tri_redgold",
                    "root_transforms": "[]",
                    "view_mode": "auto",
                    "quantile": "0.01",
                    "shim": "0.05",
                    "rotation": "0",
                    "pix": "3000",
                    "quality": "83",
                    "width": "3000",
                    "height": "3000",
                    "derived_from_artifact_id": "color_base",
                    "postprocess_kind": "autolevels",
                    "postprocess_profile": "preview_default_v1",
                    "background_color": "000000",
                    "background_threshold": "4",
                    "autolevels_params": "{\"quality\":83,\"gamma\":1.1}",
                },
            },
            "renders/j/color/autolevels_child/preview.png": {"ContentLength": 100, "ContentType": "image/png", "Metadata": {}},
            "renders/j/color/color_newer/image.jpeg": {
                "ContentLength": 1300,
                "ContentType": "image/jpeg",
                "Metadata": {
                    "artifact_id": "color_newer",
                    "created_at": "2026-04-02T11:00:00Z",
                    "family": "color",
                    "format": "jpeg",
                    "color_mode": "rainbow",
                    "root_transforms": "[]",
                    "view_mode": "auto",
                    "quantile": "0.01",
                    "shim": "0.05",
                    "rotation": "0",
                    "pix": "3000",
                    "quality": "90",
                    "width": "3000",
                    "height": "3000",
                },
            },
            "renders/j/color/color_newer/preview.png": {"ContentLength": 100, "ContentType": "image/png", "Metadata": {}},
        }

        def mock_head(**kwargs):
            key = kwargs["Key"]
            if key in meta_by_key:
                return meta_by_key[key]
            raise Exception("NoSuchKey")

        mock_s3.head_object.side_effect = mock_head
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_s3.generate_presigned_url.return_value = "https://signed"

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])
        arts = body["families"]["color"]

        self.assertEqual([a["artifact_id"] for a in arts[:3]], ["color_newer", "autolevels_child", "color_base"])
        child = arts[1]
        self.assertEqual(child["derived_from_artifact_id"], "color_base")
        self.assertEqual(child["postprocess_kind"], "autolevels")
        self.assertEqual(child["postprocess_profile"], "preview_default_v1")
        self.assertEqual(child["background_color"], "000000")
        self.assertAlmostEqual(child["background_threshold"], 4.0)
        self.assertEqual(child["autolevels_params"]["quality"], 83)
        self.assertAlmostEqual(child["quality"], 83.0)

    @patch("handler_storage.s3")
    def test_render_summary_missing_artifacts_are_false(self, mock_s3):
        """Legacy top-level artifacts are still returned as exists=false when missing."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"CommonPrefixes": []}]

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
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"CommonPrefixes": []}]

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
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"CommonPrefixes": []}]

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

    @patch("handler_storage.s3")
    def test_render_summary_missing_deepzoom_pointer_not_error(self, mock_s3):
        """Missing deepzoom_latest.json → exists=false, no error."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")
        mock_s3.get_object.side_effect = Exception("NoSuchKey")
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"CommonPrefixes": []}]

        result = handle_render_summary(self._make_event({"job_id": "j"}))
        body = json.loads(result["body"])

        self.assertFalse(body["deepzoom_latest"]["exists"])

    @patch("handler_storage.s3")
    def test_render_summary_malformed_deepzoom_pointer_ignored(self, mock_s3):
        """Bad JSON in deepzoom_latest.json → exists=false, no crash."""
        from handler_storage import handle_render_summary
        mock_s3.head_object.side_effect = Exception("NoSuchKey")
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"CommonPrefixes": []}]

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
        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{"CommonPrefixes": []}]

        dz_manifest = {
            "job_id": "j", "export_id": "dz_123",
            "created_at": "2026-03-26T10:00:00Z",
            "source_key": "renders/j/image.jpeg",
            "dzi_key": "deepzoom/j/dz_123/image.dzi",
            "dzi_url": "https://bucket.s3.amazonaws.com/deepzoom/j/dz_123/image.dzi",
            "share_url": "https://bucket.s3.amazonaws.com/deepzoom/j/dz_123/viewer.html",
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
        self.assertEqual(dz["share_url"], "https://bucket.s3.amazonaws.com/deepzoom/j/dz_123/viewer.html")
        self.assertEqual(dz["width"], 8192)
        self.assertEqual(dz["height"], 8192)
        self.assertEqual(dz["tiles_uploaded"], 400)


class TestListDeepZoom(unittest.TestCase):
    """Test the /list-deepzoom server-side inventory endpoint."""

    @patch("handler_storage.s3")
    def test_returns_exports_sorted_newest_first(self, mock_s3):
        from handler_storage import handle_list_deepzoom

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator

        # Level 1: one job prefix
        # Level 2: two export prefixes under that job
        call_count = [0]
        def paginate_side_effect(**kwargs):
            call_count[0] += 1
            prefix = kwargs.get("Prefix", "")
            if prefix == "deepzoom/":
                return [{"CommonPrefixes": [{"Prefix": "deepzoom/job_a/"}]}]
            elif prefix == "deepzoom/job_a/":
                return [{"CommonPrefixes": [
                    {"Prefix": "deepzoom/job_a/dz_old/"},
                    {"Prefix": "deepzoom/job_a/dz_new/"},
                ]}]
            return [{}]
        mock_paginator.paginate.side_effect = paginate_side_effect

        meta_old = json.dumps({"job_id": "job_a", "export_id": "dz_old",
                               "created_at": "2026-03-20T10:00:00Z"})
        meta_new = json.dumps({"job_id": "job_a", "export_id": "dz_new",
                               "created_at": "2026-03-25T10:00:00Z"})

        def mock_get(**kwargs):
            key = kwargs["Key"]
            if "dz_old" in key:
                return {"Body": MagicMock(read=lambda: meta_old.encode())}
            if "dz_new" in key:
                return {"Body": MagicMock(read=lambda: meta_new.encode())}
            raise Exception("NoSuchKey")
        mock_s3.get_object.side_effect = mock_get

        result = handle_list_deepzoom({"body": "{}"})
        body = json.loads(result["body"])

        self.assertEqual(body["count"], 2)
        # Newest first
        self.assertEqual(body["exports"][0]["export_id"], "dz_new")
        self.assertEqual(body["exports"][1]["export_id"], "dz_old")

    @patch("handler_storage.s3")
    def test_returns_empty_when_no_exports(self, mock_s3):
        from handler_storage import handle_list_deepzoom

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{}]

        result = handle_list_deepzoom({"body": "{}"})
        body = json.loads(result["body"])
        self.assertEqual(body["count"], 0)
        self.assertEqual(body["exports"], [])

    @patch("handler_storage.s3")
    def test_skips_exports_with_missing_meta(self, mock_s3):
        from handler_storage import handle_list_deepzoom

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator

        def paginate_side_effect(**kwargs):
            prefix = kwargs.get("Prefix", "")
            if prefix == "deepzoom/":
                return [{"CommonPrefixes": [{"Prefix": "deepzoom/job_a/"}]}]
            elif prefix == "deepzoom/job_a/":
                return [{"CommonPrefixes": [
                    {"Prefix": "deepzoom/job_a/dz_good/"},
                    {"Prefix": "deepzoom/job_a/dz_broken/"},
                ]}]
            return [{}]
        mock_paginator.paginate.side_effect = paginate_side_effect

        meta_good = json.dumps({"job_id": "job_a", "export_id": "dz_good",
                                "created_at": "2026-03-25T10:00:00Z"})

        def mock_get(**kwargs):
            if "dz_good" in kwargs["Key"]:
                return {"Body": MagicMock(read=lambda: meta_good.encode())}
            raise Exception("NoSuchKey")
        mock_s3.get_object.side_effect = mock_get

        result = handle_list_deepzoom({"body": "{}"})
        body = json.loads(result["body"])
        self.assertEqual(body["count"], 1)
        self.assertEqual(body["exports"][0]["export_id"], "dz_good")


class TestPaletteInventory(unittest.TestCase):
    """Test palette inventory list/delete endpoints."""

    @patch("handler_storage.s3")
    def test_list_palettes_returns_newest_first(self, mock_s3):
        from handler_storage import handle_list_palettes

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "CommonPrefixes": [
                {"Prefix": "renders/job_a/palettes/pal_old/"},
                {"Prefix": "renders/job_a/palettes/pal_new/"},
            ]
        }]

        meta_old = json.dumps({
            "job_id": "job_a", "palette_id": "pal_old", "created_at": "2026-03-20T10:00:00Z",
            "metric": "proximity", "image_key": "renders/job_a/palettes/pal_old/image.jpeg",
            "preview_key": "renders/job_a/palettes/pal_old/preview.png",
        })
        meta_new = json.dumps({
            "job_id": "job_a", "palette_id": "pal_new", "created_at": "2026-03-25T10:00:00Z",
            "metric": "crowding", "image_key": "renders/job_a/palettes/pal_new/image.jpeg",
            "preview_key": "renders/job_a/palettes/pal_new/preview.png",
        })

        def mock_get(**kwargs):
            key = kwargs["Key"]
            if "pal_old" in key:
                return {"Body": MagicMock(read=lambda: meta_old.encode())}
            if "pal_new" in key:
                return {"Body": MagicMock(read=lambda: meta_new.encode())}
            raise Exception("NoSuchKey")

        mock_s3.get_object.side_effect = mock_get
        mock_s3.generate_presigned_url.side_effect = lambda op, Params=None, ExpiresIn=None: "https://example.com/" + Params["Key"]

        result = handle_list_palettes({"body": json.dumps({"job_id": "job_a"})})
        body = json.loads(result["body"])

        self.assertEqual(body["count"], 2)
        self.assertEqual(body["palettes"][0]["palette_id"], "pal_new")
        self.assertEqual(body["palettes"][1]["palette_id"], "pal_old")
        self.assertIn("image_url", body["palettes"][0])
        self.assertIn("preview_url", body["palettes"][0])

    @patch("handler_storage.s3")
    def test_delete_palette_deletes_prefix(self, mock_s3):
        from handler_storage import handle_delete_palette

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "Contents": [
                {"Key": "renders/job_a/palettes/pal_1/meta.json"},
                {"Key": "renders/job_a/palettes/pal_1/image.jpeg"},
                {"Key": "renders/job_a/palettes/pal_1/preview.png"},
            ]
        }]
        mock_s3.delete_objects.return_value = {"Deleted": [{}, {}, {}]}

        result = handle_delete_palette({"body": json.dumps({"job_id": "job_a", "palette_id": "pal_1"})})
        body = json.loads(result["body"])

        self.assertEqual(body["deleted"], 3)
        delete_batch = mock_s3.delete_objects.call_args[1]["Delete"]["Objects"]
        self.assertEqual(delete_batch[0]["Key"], "renders/job_a/palettes/pal_1/meta.json")


class TestRenderArtifactInventory(unittest.TestCase):

    @patch("handler_storage.s3")
    def test_delete_render_artifact_deletes_family_prefix(self, mock_s3):
        from handler_storage import handle_delete_render_artifact

        mock_paginator = MagicMock()
        mock_s3.get_paginator.return_value = mock_paginator
        mock_paginator.paginate.return_value = [{
            "Contents": [
                {"Key": "renders/job_a/color/color_run_1/image.jpeg"},
                {"Key": "renders/job_a/color/color_run_1/preview.png"},
            ]
        }]
        mock_s3.delete_objects.return_value = {"Deleted": [{}, {}]}

        result = handle_delete_render_artifact({
            "body": json.dumps({"job_id": "job_a", "family": "color", "artifact_id": "color_run_1"})
        })
        body = json.loads(result["body"])

        self.assertEqual(body["deleted"], 2)
        delete_batch = mock_s3.delete_objects.call_args[1]["Delete"]["Objects"]
        self.assertEqual(delete_batch[0]["Key"], "renders/job_a/color/color_run_1/image.jpeg")

    @patch("handler_storage.s3")
    def test_delete_render_artifact_legacy_palette_deletes_top_level_keys(self, mock_s3):
        from handler_storage import handle_delete_render_artifact

        result = handle_delete_render_artifact({
            "body": json.dumps({"job_id": "job_a", "family": "palette", "artifact_id": "legacy_palette"})
        })
        body = json.loads(result["body"])

        self.assertEqual(body["deleted"], 2)
        deleted = [c.kwargs["Key"] for c in mock_s3.delete_object.call_args_list]
        self.assertIn("renders/job_a/image_palette.jpeg", deleted)
        self.assertIn("renders/job_a/preview_palette.png", deleted)


class TestFavoritesStorage(unittest.TestCase):

    def test_favorites_key_under_polypaint_prefix(self):
        import handler_storage

        self.assertEqual(handler_storage.FAVORITES_KEY, "polypaint/favorites/color_artifacts.json")

    @patch("handler_storage.s3")
    def test_list_favorites_returns_empty_when_missing(self, mock_s3):
        from handler_storage import handle_list_favorites

        mock_s3.get_object.side_effect = Exception("NoSuchKey")

        result = handle_list_favorites({"body": "{}"})
        body = json.loads(result["body"])

        self.assertEqual(body["favorites"], [])
        self.assertEqual(body["count"], 0)

    @patch("handler_storage._write_favorites")
    @patch("handler_storage._read_favorites")
    def test_add_favorite_prepends_new_ref(self, mock_read, mock_write):
        from handler_storage import handle_add_favorite

        mock_read.return_value = [
            {"job_id": "job_old", "artifact_id": "color_old", "family": "color", "added_at": "2026-01-01T00:00:00Z"}
        ]

        result = handle_add_favorite({
            "body": json.dumps({
                "job_id": "job_new",
                "artifact_id": "color_new",
                "family": "color",
                "display_name": "color_new",
                "image_key": "renders/job_new/color/color_new/image.jpeg",
                "preview_key": "renders/job_new/color/color_new/preview.png",
            })
        })
        body = json.loads(result["body"])

        self.assertTrue(body["added"])
        self.assertEqual(body["count"], 2)
        written = mock_write.call_args[0][0]
        self.assertEqual(written[0]["job_id"], "job_new")
        self.assertEqual(written[0]["artifact_id"], "color_new")
        self.assertEqual(written[0]["image_key"], "renders/job_new/color/color_new/image.jpeg")

    @patch("handler_storage._write_favorites")
    @patch("handler_storage._read_favorites")
    def test_add_favorite_dedupes_existing_ref(self, mock_read, mock_write):
        from handler_storage import handle_add_favorite

        mock_read.return_value = [
            {"job_id": "job_a", "artifact_id": "color_1", "family": "color", "added_at": "2026-01-01T00:00:00Z"}
        ]

        result = handle_add_favorite({
            "body": json.dumps({
                "job_id": "job_a",
                "artifact_id": "color_1",
                "family": "color",
            })
        })
        body = json.loads(result["body"])

        self.assertFalse(body["added"])
        self.assertEqual(body["count"], 1)
        mock_write.assert_not_called()

    @patch("handler_storage._write_favorites")
    @patch("handler_storage._read_favorites")
    def test_delete_favorite_removes_only_matching_ref(self, mock_read, mock_write):
        from handler_storage import handle_delete_favorite

        mock_read.return_value = [
            {"job_id": "job_a", "artifact_id": "color_1", "family": "color", "added_at": "2026-01-01T00:00:00Z"},
            {"job_id": "job_b", "artifact_id": "color_2", "family": "color", "added_at": "2026-01-02T00:00:00Z"},
        ]

        result = handle_delete_favorite({
            "body": json.dumps({
                "job_id": "job_a",
                "artifact_id": "color_1",
            })
        })
        body = json.loads(result["body"])

        self.assertTrue(body["deleted"])
        self.assertEqual(body["count"], 1)
        written = mock_write.call_args[0][0]
        self.assertEqual(written, [{"job_id": "job_b", "artifact_id": "color_2", "family": "color", "added_at": "2026-01-02T00:00:00Z"}])


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

        # Allow real open for template file, mock for everything else
        import builtins
        real_open = builtins.open
        fake_file = MagicMock(read=lambda: b"fake", write=lambda x: None)
        fake_cm = MagicMock(__enter__=MagicMock(return_value=fake_file),
                            __exit__=MagicMock(return_value=False))
        def selective_open(*args, **kwargs):
            path = args[0] if args else kwargs.get("file", "")
            if isinstance(path, str) and path.endswith("_template.html"):
                return real_open(*args, **kwargs)
            return fake_cm

        # Patch paths to use our temp dir
        with patch.object(os, 'makedirs', side_effect=lambda *a, **kw: None), \
             patch.object(os, 'remove', side_effect=lambda *a: None), \
             patch.object(os.path, 'exists', return_value=True), \
             patch.object(os.path, 'isdir', return_value=True), \
             patch.object(os, 'walk', return_value=[(tiles_dir + "/0", [], ["0_0.png"])]), \
             patch('shutil.rmtree'), \
             patch('builtins.open', side_effect=selective_open):

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
        self.assertIn("share_url", manifest)
        self.assertIn("width", manifest)
        self.assertIn("height", manifest)

        # share_url must point to viewer.html, not index.html
        self.assertIn("viewer.html", manifest["share_url"])
        self.assertNotIn("index.html", manifest["share_url"])

        # deepzoom_latest.json must also contain share_url
        pointer_manifest = json.loads(pointer_body)
        self.assertIn("share_url", pointer_manifest)
        self.assertEqual(pointer_manifest["share_url"], manifest["share_url"])

        shutil.rmtree(tmp_dir, ignore_errors=True)

    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    @patch("handler_deepzoom_export.subprocess")
    def test_export_uploads_viewer_html(self, mock_subprocess, mock_s3, mock_report):
        """Export handler uploads viewer.html with correct content type."""
        import handler_deepzoom_export as dze

        mock_s3.get_object.return_value = {
            "Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"fake image data"])
        }
        mock_result = MagicMock()
        mock_result.returncode = 0
        mock_result.stdout = json.dumps({"width": 64, "height": 64})
        mock_subprocess.run.return_value = mock_result

        put_calls = []
        def track_put(**kwargs):
            put_calls.append(kwargs)
        mock_s3.put_object.side_effect = track_put

        # Allow real open for template file, mock for everything else
        import builtins
        real_open = builtins.open
        fake_file = MagicMock(read=lambda: b"fake", write=lambda x: None)
        fake_cm = MagicMock(__enter__=MagicMock(return_value=fake_file),
                            __exit__=MagicMock(return_value=False))
        def selective_open(*args, **kwargs):
            path = args[0] if args else kwargs.get("file", "")
            if isinstance(path, str) and path.endswith("_template.html"):
                return real_open(*args, **kwargs)
            return fake_cm

        with patch.object(os, 'makedirs', side_effect=lambda *a, **kw: None), \
             patch.object(os, 'remove', side_effect=lambda *a: None), \
             patch.object(os.path, 'exists', return_value=True), \
             patch.object(os.path, 'isdir', return_value=True), \
             patch.object(os, 'walk', return_value=[("/tmp/dz/image_files/0", [], ["0_0.png"])]), \
             patch('shutil.rmtree'), \
             patch('builtins.open', side_effect=selective_open):

            try:
                dze.handler({"body": json.dumps({
                    "job_id": "test_dz", "source_key": "renders/test_dz/image.tif",
                    "export_id": "dz_viewer_test"
                })}, None)
            except Exception:
                pass

        # Find the viewer.html upload
        viewer_puts = [c for c in put_calls if "viewer.html" in c.get("Key", "")]
        self.assertEqual(len(viewer_puts), 1,
            "Expected exactly one viewer.html upload, got: %s" % [c.get("Key") for c in put_calls])

        # Verify content type
        self.assertEqual(viewer_puts[0]["ContentType"], "text/html; charset=utf-8")

        # Verify key path
        self.assertEqual(viewer_puts[0]["Key"], "deepzoom/test_dz/dz_viewer_test/viewer.html")

        # Verify HTML content contains expected elements
        body = viewer_puts[0]["Body"]
        self.assertIn("OpenSeadragon", body)
        self.assertIn("test_dz", body)
        self.assertIn("dz_viewer_test", body)
        # OSD fetches the real DZI descriptor via relative path
        self.assertIn("tileSources: 'image.dzi'", body)
        self.assertNotIn("{job_id}", body, "Template placeholders must be replaced")
        self.assertNotIn("{export_id}", body, "Template placeholders must be replaced")


class TestDeepZoomViewerTemplate(unittest.TestCase):
    """Test that the viewer template renders correctly."""

    def test_template_file_exists(self):
        """Viewer template file must exist in lambda dir."""
        tmpl_path = os.path.join(os.path.dirname(__file__), "..", "lambda",
                                 "deepzoom_viewer_template.html")
        self.assertTrue(os.path.exists(tmpl_path), "Template file missing")

    def test_template_renders_all_placeholders(self):
        """All placeholders are replaced by _render_viewer."""
        from handler_deepzoom_export import _render_viewer
        html = _render_viewer("job_abc", "dz_123", "2026-03-27T12:00:00Z")
        # No unreplaced placeholders
        self.assertNotIn("{job_id}", html)
        self.assertNotIn("{export_id}", html)
        self.assertNotIn("{created_at}", html)
        # Contains actual values
        self.assertIn("job_abc", html)
        self.assertIn("dz_123", html)
        self.assertIn("2026-03-27T12:00:00Z", html)

    def test_template_is_standalone(self):
        """Viewer HTML must not reference index.html or app APIs."""
        from handler_deepzoom_export import _render_viewer
        html = _render_viewer("j", "e", "2026-01-01")
        self.assertNotIn("index.html", html)
        self.assertNotIn("lambdaPost", html)
        self.assertNotIn("localStorage", html)
        self.assertIn("openseadragon", html.lower())

    def test_template_uses_relative_dzi_tilesources(self):
        """tileSources must be 'image.dzi' — OSD fetches the real descriptor."""
        from handler_deepzoom_export import _render_viewer
        html = _render_viewer("j", "e", "2026-01-01")
        self.assertIn("tileSources: 'image.dzi'", html)
        # No absolute S3 URLs anywhere in the page
        self.assertNotIn("s3.us-east-1.amazonaws.com", html)


if __name__ == "__main__":
    unittest.main()
