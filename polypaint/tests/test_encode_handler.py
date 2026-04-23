import json
import os
import struct
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _Body:
    def __init__(self, data):
        self._data = data

    def read(self):
        return self._data


class TestEncodeHandler(unittest.TestCase):
    @patch("handler_encode.report_status")
    @patch("handler_encode.subprocess.run")
    @patch("handler_encode.s3")
    def test_encode_accepts_tile_prefix(self, mock_s3, mock_run, mock_report):
        import handler_encode as mod

        tile0 = struct.pack("<III", 1, 2, 3) + b"\x01\x02\x03" * 2
        tile1 = struct.pack("<III", 1, 2, 3) + b"\x04\x05\x06" * 2
        uploaded = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/tile_0000.raw":
                return {"Body": _Body(tile0)}
            if key == "renders/j/tile_0001.raw":
                return {"Body": _Body(tile1)}
            raise AssertionError(f"unexpected key {key}")

        def put_object(**kwargs):
            uploaded["key"] = kwargs["Key"]
            uploaded["body"] = kwargs["Body"].read()
            uploaded["content_type"] = kwargs["ContentType"]
            uploaded["metadata"] = kwargs["Metadata"]

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            self.assertEqual(cmd[1], "/tmp/encode_in.raw")
            self.assertEqual(cmd[2], "/tmp/encode_out.png")
            with open(cmd[2], "wb") as fh:
                fh.write(b"PNG")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 3}), stderr="")

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.generate_presigned_url.return_value = "https://signed"
        mock_run.side_effect = run_side_effect

        result = mod.handler({
            "job_id": "j",
            "task_id": "encode_t",
            "out_key": "renders/j/image.png",
            "format": "png",
            "pix": 2,
            "tile_grid": {
                "n_cols": 2,
                "n_rows": 1,
                "tile_prefix": "renders/j/tile_",
            },
            "metadata": {"mode": "color"},
        }, None)

        body = json.loads(result["body"])
        self.assertEqual(body["out_key"], "renders/j/image.png")
        self.assertEqual(uploaded["key"], "renders/j/image.png")
        self.assertEqual(uploaded["body"], b"PNG")
        self.assertEqual(uploaded["content_type"], "image/png")
        self.assertEqual(uploaded["metadata"]["mode"], "color")
        self.assertEqual(uploaded["metadata"]["pix"], "2")
        self.assertEqual(uploaded["metadata"]["width"], "2")
        self.assertEqual(uploaded["metadata"]["height"], "2")
        self.assertEqual(mock_report.call_args_list[-1].args[2], "done")

    @patch("handler_encode.write_color_artifact_meta_overlay")
    @patch("handler_encode.report_status")
    @patch("handler_encode.subprocess.run")
    @patch("handler_encode.s3")
    def test_encode_splits_color_metadata_into_overlay(self, mock_s3, mock_run, mock_report, mock_overlay):
        import handler_encode as mod

        tile = struct.pack("<III", 1, 1, 3) + b"\x01\x02\x03"
        uploaded = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/color/color_a/tile_0000.raw":
                return {"Body": _Body(tile)}
            raise AssertionError(f"unexpected key {key}")

        def put_object(**kwargs):
            uploaded["key"] = kwargs["Key"]
            uploaded["body"] = kwargs["Body"].read()
            uploaded["metadata"] = kwargs["Metadata"]

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            with open(cmd[2], "wb") as fh:
                fh.write(b"JPEG")
            return MagicMock(returncode=0, stdout=json.dumps({"file_size": 4}), stderr="")

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.generate_presigned_url.return_value = "https://signed"
        mock_run.side_effect = run_side_effect

        result = mod.handler({
            "job_id": "j",
            "task_id": "encode_color_t",
            "out_key": "renders/j/color/color_a/image.jpeg",
            "format": "jpeg",
            "pix": 1,
            "tile_grid": {
                "n_cols": 1,
                "n_rows": 1,
                "tile_prefix": "renders/j/color/color_a/tile_",
            },
            "metadata": {
                "family": "color",
                "artifact_id": "color_a",
                "created_at": "2026-04-16T12:00:00Z",
                "format": "jpeg",
                "quality": "90",
                "color_mode": "solve_score",
                "render_execution": {"raster_engine": "mt", "solve_score_hist_input_mode": "sectioned"},
                "solve_score_chain": [["spread", "slv", "0.1"], ["flip"]],
            },
        }, None)

        body = json.loads(result["body"])
        self.assertEqual(body["out_key"], "renders/j/color/color_a/image.jpeg")
        self.assertEqual(uploaded["key"], "renders/j/color/color_a/image.jpeg")
        self.assertEqual(uploaded["metadata"]["family"], "color")
        self.assertEqual(uploaded["metadata"]["artifact_id"], "color_a")
        self.assertNotIn("color_mode", uploaded["metadata"])
        self.assertNotIn("render_execution", uploaded["metadata"])
        self.assertNotIn("solve_score_chain", uploaded["metadata"])
        mock_overlay.assert_called_once()
        _, _, job_id, artifact_id, overlay_meta = mock_overlay.call_args.args
        self.assertEqual(job_id, "j")
        self.assertEqual(artifact_id, "color_a")
        self.assertEqual(overlay_meta["color_mode"], "solve_score")
        self.assertIn("render_execution", overlay_meta)
        self.assertIn("solve_score_chain", overlay_meta)
        self.assertEqual(mock_report.call_args_list[-1].args[2], "done")


if __name__ == "__main__":
    unittest.main()
