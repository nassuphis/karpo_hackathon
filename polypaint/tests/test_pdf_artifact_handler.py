import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "job1",
        "task_id": "pdf_run_1",
        "artifact_id": "pdf_123",
        "source_artifact_id": "color_src",
        "source_image_key": "renders/job1/color/color_src/image.jpeg",
    }
    payload.update(overrides)
    return payload


class TestPdfArtifactHandler(unittest.TestCase):

    @patch("handler_pdf_artifact.report_status")
    @patch("handler_pdf_artifact.build_color_spread_pdf")
    @patch("handler_pdf_artifact.s3")
    def test_handler_uploads_pdf_artifact_with_metadata(self, mock_s3, mock_build, mock_report):
        from handler_pdf_artifact import handler

        mock_s3.head_object.return_value = {
            "Metadata": {
                "family": "color",
                "artifact_id": "color_src",
                "created_at": "2026-04-04T10:00:00Z",
                "color_mode": "solve_score",
                "palette": "tri_redgold",
                "solve_metric": "clusteriness",
                "solve_score_quantile": "0.05",
                "solve_score_omega": "3",
                "associated_palette_mode": "generated",
                "associated_palette_id": "pal_color_src",
                "associated_palette_image_key": "renders/job1/palettes/pal_color_src/image.jpeg",
                "root_transforms": '[["rotate_roots","0.25"]]',
            }
        }

        def get_object(Bucket=None, Key=None):
            if Key == "renders/job1/color/color_src/image.jpeg":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"jpeg-bytes"])}
            if Key == "renders/job1/palettes/pal_color_src/image.jpeg":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"palette-jpeg"])}
            if Key == "renders/job1/calc.json":
                return {"Body": MagicMock(read=lambda: json.dumps({
                    "function": "poly_645",
                    "degree": 24,
                    "N": 500,
                    "times": 3,
                    "solver": "aberth",
                }).encode())}
            raise AssertionError(f"unexpected get_object key: {Key}")

        uploads = {}

        def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
            uploads[key] = {
                "body": fileobj.read(),
                "extra": ExtraArgs or {},
            }

        def fake_build(image_path, output_path, title, body=None, filename=None, meta=None, palette_image_path=None):
            self.assertTrue(str(image_path).endswith("pdf_source.jpeg"))
            self.assertTrue(str(palette_image_path).endswith("pdf_palette.jpeg"))
            self.assertEqual(title, "PolyPaint Lambda 1.0")
            # Structured meta should be provided
            self.assertIsNotNone(meta)
            self.assertIn("pipeline", meta)
            self.assertIn("lines", meta)
            self.assertIn("artifact_id", meta)
            self.assertIn("solve score:", meta["color_mode"])
            self.assertIn("solver=AE", meta["lines"][0])
            self.assertIn("palette: tri_redgold", meta["lines"])
            self.assertEqual(meta["artifact_id"], "color_src")
            with open(output_path, "wb") as fh:
                fh.write(b"%PDF-1.4 fake pdf")
            return output_path

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_build.side_effect = fake_build

        result = handler(_event(), None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["family"], "pdf")
        self.assertEqual(body["artifact_id"], "pdf_123")
        self.assertEqual(body["image_key"], "renders/job1/pdf/pdf_123/document.pdf")

        pdf_key = "renders/job1/pdf/pdf_123/document.pdf"
        self.assertIn(pdf_key, uploads)
        extra = uploads[pdf_key]["extra"]
        self.assertEqual(extra["ContentType"], "application/pdf")
        meta = extra["Metadata"]
        self.assertEqual(meta["family"], "pdf")
        self.assertEqual(meta["artifact_id"], "pdf_123")
        self.assertEqual(meta["pdf_kind"], "color_spread")
        self.assertEqual(meta["source_family"], "color")
        self.assertEqual(meta["source_artifact_id"], "color_src")
        self.assertEqual(meta["source_image_key"], "renders/job1/color/color_src/image.jpeg")
        self.assertEqual(meta["source_palette"], "tri_redgold")
        self.assertEqual(meta["source_solve_metric"], "clusteriness")
        self.assertEqual(meta["source_associated_palette_mode"], "generated")
        self.assertEqual(meta["source_associated_palette_id"], "pal_color_src")
        self.assertEqual(meta["source_associated_palette_image_key"], "renders/job1/palettes/pal_color_src/image.jpeg")
        self.assertEqual(meta["page_count"], "1")
        self.assertEqual(meta["width_mm"], "586")
        self.assertEqual(meta["height_mm"], "296")
        self.assertEqual(meta["function"], "poly_645")
        self.assertEqual(meta["degree"], "24")
        self.assertEqual(meta["N"], "500")
        self.assertEqual(meta["times"], "3")

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "downloading", "processing", "uploading", "done"])

    @patch("handler_pdf_artifact.report_status")
    @patch("handler_pdf_artifact.s3")
    def test_handler_rejects_non_color_source(self, mock_s3, mock_report):
        from handler_pdf_artifact import handler

        mock_s3.head_object.return_value = {"Metadata": {"family": "palette", "artifact_id": "pal_src"}}

        with self.assertRaisesRegex(RuntimeError, "ColorSpread requires Color source"):
            handler(_event(source_artifact_id="pal_src", source_image_key="renders/job1/palettes/pal_src/image.jpeg"), None)


if __name__ == "__main__":
    unittest.main()
