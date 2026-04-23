import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _Body:
    def __init__(self, data):
        self._data = data

    def iter_chunks(self, chunk_size=1024 * 1024):
        for idx in range(0, len(self._data), chunk_size):
            yield self._data[idx:idx + chunk_size]


def _event(**overrides):
    payload = {
        "job_id": "j",
        "task_id": "coeff_stitch_1",
        "n_tile_cols": 1,
        "n_tile_rows": 1,
        "pix": 2,
        "tile_size": 2,
        "out_key": "renders/j/coeffs/coeffs_1/image.tif",
        "preview_key": "renders/j/coeffs/coeffs_1/preview.png",
        "tile_prefix": "coeff",
        "metadata": {
            "artifact_id": "coeffs_1",
            "family": "coeffs",
            "created_at": "2026-04-22T00:00:00Z",
            "format": "tif",
            "mode": "coeffs",
            "degree": "5",
            "pix": "2",
            "tile_size": "2",
            "view_mode": "explicit",
            "quantile": "0.0",
            "shim": "0.05",
            "square_extent": "2.0",
            "min_re": "-1.5",
            "max_re": "1.5",
            "min_im": "-1.0",
            "max_im": "1.0",
            "rotation": "0.0",
            "root_transforms": "[]",
            "render_execution": '{"raster_engine":"mt"}',
        },
    }
    payload.update(overrides)
    return payload


class TestBilevelStitchHandler(unittest.TestCase):
    @patch("handler_coeff_bilevel_stitch.report_status")
    def test_handler_reports_malformed_json_at_entry(self, mock_report):
        import handler_coeff_bilevel_stitch as mod

        with self.assertRaisesRegex(RuntimeError, "could not parse request body"):
            mod.handler({"body": '{"job_id":"j","task_id":"coeff_stitch_1",'}, None)

        mock_report.assert_called_once()
        self.assertEqual(mock_report.call_args.args[:3], ("j", "coeff_stitch_1", "error"))
        self.assertIn("could not parse request body", mock_report.call_args.args[3])
        self.assertEqual(mock_report.call_args.kwargs["result_data"]["phase"], "handler_entry")

    @patch("handler_coeff_bilevel_stitch.report_status")
    @patch("handler_coeff_bilevel_stitch.subprocess.run")
    @patch("handler_coeff_bilevel_stitch.s3")
    def test_handler_rejects_oversized_upload_metadata_before_put(self, mock_s3, mock_run, mock_report):
        import handler_coeff_bilevel_stitch as mod

        mock_s3.get_object.return_value = {"Body": _Body(b"TIFFTILE")}

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            out_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--output="))
            preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
            with open(out_path, "wb") as fh:
                fh.write(b"FINALTIFF")
            with open(preview_path, "wb") as fh:
                fh.write(b"PREVIEWPNG")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 9}), stderr="")

        mock_run.side_effect = run_side_effect

        huge_transforms = [["rotate_roots", f"{idx / 1000:.6f}"] for idx in range(200)]
        payload = _event(metadata={**_event()["metadata"], "root_transforms": huge_transforms})

        with self.assertRaisesRegex(RuntimeError, "stitch upload metadata root_transforms exceeds"):
            mod.handler(payload, None)

        mock_s3.put_object.assert_not_called()
        self.assertEqual([call.args[2] for call in mock_report.call_args_list], ["started", "stitching", "error"])

    @patch("handler_coeff_bilevel_stitch.report_status")
    @patch("handler_coeff_bilevel_stitch.subprocess.run")
    @patch("handler_coeff_bilevel_stitch.s3")
    def test_handler_uploads_bounded_coeff_metadata(self, mock_s3, mock_run, mock_report):
        import handler_coeff_bilevel_stitch as mod

        uploads = {}

        def get_object(Bucket=None, Key=None):
            self.assertEqual(Key, "renders/j/coeff_t0000.tif")
            return {"Body": _Body(b"TIFFTILE")}

        def put_object(**kwargs):
            body = kwargs["Body"]
            uploads[kwargs["Key"]] = {
                "data": body.read() if hasattr(body, "read") else body,
                "content_type": kwargs.get("ContentType"),
                "metadata": kwargs.get("Metadata"),
            }

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            out_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--output="))
            preview_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--preview="))
            with open(out_path, "wb") as fh:
                fh.write(b"FINALTIFF")
            with open(preview_path, "wb") as fh:
                fh.write(b"PREVIEWPNG")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 9}), stderr="")

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_run.side_effect = run_side_effect

        result = mod.handler(_event(), None)
        body = json.loads(result["body"])

        self.assertEqual(body["out_key"], "renders/j/coeffs/coeffs_1/image.tif")
        self.assertEqual(uploads["renders/j/coeffs/coeffs_1/image.tif"]["data"], b"FINALTIFF")
        self.assertEqual(uploads["renders/j/coeffs/coeffs_1/image.tif"]["content_type"], "image/tiff")
        image_meta = uploads["renders/j/coeffs/coeffs_1/image.tif"]["metadata"]
        self.assertEqual(image_meta["artifact_id"], "coeffs_1")
        self.assertEqual(image_meta["family"], "coeffs")
        self.assertEqual(image_meta["render_execution"], '{"raster_engine":"mt"}')
        self.assertEqual(image_meta["width"], "2")
        self.assertEqual(image_meta["height"], "2")
        self.assertNotIn("unknown_key", image_meta)
        self.assertEqual(uploads["renders/j/coeffs/coeffs_1/preview.png"]["data"], b"PREVIEWPNG")
        self.assertEqual([call.args[2] for call in mock_report.call_args_list], ["started", "stitching", "done"])
