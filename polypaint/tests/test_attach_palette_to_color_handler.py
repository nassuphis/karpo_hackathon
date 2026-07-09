"""
Tests for handler_attach_palette_to_color.py.
"""
import json
import os
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestAttachPaletteToColorHandler(unittest.TestCase):

    @patch("handler_attach_palette_to_color.report_status")
    @patch("handler_attach_palette_to_color.s3")
    def test_rejects_unsafe_associated_palette_image_key(self, mock_s3, mock_report):
        # code-review-26 F13: this key is later downloaded + rasterized by
        # book_pdf — pin it to render output at write time
        from handler_attach_palette_to_color import handler
        base = {
            "job_id": "j", "task_id": "palette_run_attach", "artifact_id": "color_src",
            "associated_palette_mode": "generated", "associated_palette_id": "pal_123",
        }
        for bad in ("config/secret.json",
                    "renders/j/palettes/pal_123/image.jpeg} \\input{x",
                    "renders/j/palettes/pal_123/image.svg"):
            with self.assertRaises(Exception):
                handler({**base, "associated_palette_image_key": bad}, None)
        mock_s3.put_object.assert_not_called()

    @patch("handler_attach_palette_to_color.report_status")
    @patch("handler_attach_palette_to_color.s3")
    def test_attach_writes_sidecar_overlay_metadata(self, mock_s3, mock_report):
        from handler_attach_palette_to_color import handler

        mock_s3.head_object.return_value = {
            "ContentType": "image/jpeg",
            "Metadata": {
                "artifact_id": "color_src",
                "family": "color",
                "color_mode": "solve_score",
                "palette": "magma",
            },
        }

        result = handler({
            "job_id": "j",
            "task_id": "palette_run_attach",
            "artifact_id": "color_src",
            "associated_palette_mode": "generated",
            "associated_palette_id": "pal_123",
            "associated_palette_display_name": "spread q=1.0% w=4 inferno",
            "associated_palette_image_key": "renders/j/palettes/pal_123/image.jpeg",
            "associated_palette_preview_key": "renders/j/palettes/pal_123/preview.png",
            "associated_palette_palette": "inferno",
            "associated_palette_metric": "spread",
            "associated_palette_quantile": 0.01,
            "associated_palette_omega": 4.0,
            "associated_palette_omega_enabled": False,
        }, None)
        body = json.loads(result["body"])

        self.assertEqual(body["artifact_id"], "color_src")
        self.assertEqual(body["associated_palette_id"], "pal_123")
        self.assertEqual(body["meta_key"], "renders/j/color/color_src/meta.json")

        put_kwargs = mock_s3.put_object.call_args.kwargs
        self.assertEqual(put_kwargs["Key"], "renders/j/color/color_src/meta.json")
        self.assertEqual(put_kwargs["ContentType"], "application/json")
        meta = json.loads(put_kwargs["Body"].decode("utf-8"))
        self.assertEqual(meta["associated_palette_mode"], "generated")
        self.assertEqual(meta["associated_palette_id"], "pal_123")
        self.assertEqual(meta["associated_palette_image_key"], "renders/j/palettes/pal_123/image.jpeg")
        self.assertEqual(meta["associated_palette_omega_enabled"], "false")

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "done"])


if __name__ == "__main__":
    unittest.main()
