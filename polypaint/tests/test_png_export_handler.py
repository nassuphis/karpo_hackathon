import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _png_header(width, height):
    return (
        b"\x89PNG\r\n\x1a\n"
        + (13).to_bytes(4, "big")
        + b"IHDR"
        + int(width).to_bytes(4, "big")
        + int(height).to_bytes(4, "big")
    )


def _event(**overrides):
    payload = {
        "job_id": "job1",
        "task_id": "png_export_png_child",
        "artifact_id": "png_child",
        "source_artifact_id": "bil_base",
        "source_key": "renders/job1/bilevel/bil_base/image.tif",
    }
    payload.update(overrides)
    return payload


class TestPngExportHandler(unittest.TestCase):

    def test_rejects_cross_job_source_key(self):
        # code-review-27 F5: a source key from another job must be refused
        # BEFORE any S3 head/get
        from handler_png_export import handler
        with self.assertRaises(ValueError):
            handler({"body": json.dumps(_event(source_key="renders/OTHERJOB/color/color_src/image.tif"))}, None)


    @patch("handler_png_export.report_status")
    @patch("handler_png_export.subprocess.run")
    @patch("handler_png_export.s3")
    def test_handler_uploads_derived_bilevel_png_artifact(self, mock_s3, mock_run, mock_report):
        from handler_png_export import handler

        mock_s3.head_object.return_value = {
            "ContentLength": 3456,
            "Metadata": {
                "artifact_id": "bil_base",
                "family": "bilevel",
                "created_at": "2026-04-10T10:00:00Z",
                "format": "tif",
                "width": "4096",
                "height": "4096",
                "pix": "4096",
                "view_mode": "explicit",
                "min_re": "-3.5",
                "max_re": "1.25",
                "min_im": "-0.75",
                "max_im": "2.0",
                "rotation": "0.125",
                "root_transforms": '[["rotate_roots","0.25"]]',
            },
        }
        mock_s3.get_object.return_value = {
            "Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"src-tiff-bytes"])
        }

        uploads = {}

        def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
            uploads[key] = {"body": fileobj.read(), "extra": ExtraArgs or {}}

        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_s3.generate_presigned_url.return_value = "https://signed"

        def fake_run(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "png_export":
                with open(cmd[2], "wb") as fh:
                    fh.write(b"\x89PNGderived")
                return MagicMock(returncode=0, stdout=json.dumps({
                    "width": 4096,
                    "height": 4096,
                    "file_size": 777,
                }), stderr="")
            if exe == "vipsthumbnail":
                out_path = cmd[5].split("[", 1)[0]
                with open(out_path, "wb") as fh:
                    fh.write(_png_header(512, 512))
                return MagicMock(returncode=0, stdout="", stderr="")
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        result = handler({"body": json.dumps(_event())}, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["artifact_id"], "png_child")
        self.assertEqual(body["family"], "bilevel")

        image_key = "renders/job1/bilevel/png_child/image.png"
        preview_key = "renders/job1/bilevel/png_child/preview.png"
        self.assertIn(image_key, uploads)
        self.assertIn(preview_key, uploads)

        image_meta = uploads[image_key]["extra"]["Metadata"]
        self.assertEqual(image_meta["artifact_id"], "png_child")
        self.assertEqual(image_meta["family"], "bilevel")
        self.assertEqual(image_meta["derived_from_artifact_id"], "bil_base")
        self.assertEqual(image_meta["derived_from_image_key"], "renders/job1/bilevel/bil_base/image.tif")
        self.assertEqual(image_meta["postprocess_kind"], "png_export")
        self.assertEqual(image_meta["postprocess_profile"], "bilevel_png_export_v1")
        self.assertEqual(image_meta["format"], "png")
        self.assertEqual(image_meta["pix"], "4096")
        self.assertEqual(image_meta["width"], "4096")
        self.assertEqual(image_meta["height"], "4096")
        self.assertEqual(image_meta["view_mode"], "explicit")
        self.assertEqual(image_meta["min_re"], "-3.5")
        self.assertEqual(image_meta["max_re"], "1.25")
        self.assertEqual(image_meta["min_im"], "-0.75")
        self.assertEqual(image_meta["max_im"], "2.0")
        self.assertEqual(image_meta["rotation"], "0.125")

        # preview metadata must describe the preview itself, not the full-size
        # source (deepzoom-speed.md §2.5), and previews are immutable-cacheable
        preview_extra = uploads[preview_key]["extra"]
        self.assertEqual(preview_extra["ContentType"], "image/png")
        self.assertEqual(preview_extra["Metadata"]["pix"], "512")
        self.assertEqual(preview_extra["Metadata"]["width"], "512")
        self.assertEqual(preview_extra["Metadata"]["height"], "512")
        self.assertEqual(preview_extra["CacheControl"], "public, max-age=31536000, immutable")
        self.assertNotIn("CacheControl", uploads[image_key]["extra"])

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertIn("started", statuses)
        self.assertIn("done", statuses)
        done_call = mock_report.call_args_list[-1]
        self.assertEqual(done_call.kwargs["result_data"]["image_key"], image_key)
        self.assertEqual(done_call.kwargs["result_data"]["preview_key"], preview_key)


if __name__ == "__main__":
    unittest.main()
