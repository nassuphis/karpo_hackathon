import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "j",
        "task_id": "raster_0",
        "chunk_idx": 0,
        "bin_key": "renders/j/chunk_0.bin",
        "saved_palette_bins_key": "renders/j/palettes/p1/chunks/palette_bins_chunk_0.bin",
        "n_tile_cols": 1,
        "n_tile_rows": 1,
        "width": 512,
        "height": 512,
        "tile_size": 512,
        "center_re": 0.0,
        "center_im": 0.0,
        "scale": 1.0,
        "degree": 5,
        "color": "saved_palette",
        "palette": "tri_redgold",
        "match": "none",
        "rotation": 0.0,
    }
    payload.update(overrides)
    return payload


class TestRasterSavedPalette(unittest.TestCase):

    @patch("handler_raster.report_status")
    @patch("handler_raster.subprocess.run")
    @patch("handler_raster.s3")
    def test_saved_palette_mode_downloads_chunk_bins_and_passes_file_to_roots2pix(self, mock_s3, mock_run, mock_report):
        import handler_raster as mod

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/chunk_0.bin":
                return {"Body": MagicMock(read=lambda: b"\x00" * 80)}
            if key == "renders/j/palettes/p1/chunks/palette_bins_chunk_0.bin":
                return {"Body": MagicMock(read=lambda: bytes([1, 2, 3, 4]))}
            raise AssertionError(f"unexpected get_object key: {key}")

        mock_s3.get_object.side_effect = get_object
        mock_run.return_value = MagicMock(
            returncode=0,
            stdout=json.dumps({"roots_plotted": 12, "roots_clipped": 0}),
            stderr="",
        )

        with patch("glob.glob", return_value=[]):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["chunk_idx"], 0)
        cmd = mock_run.call_args.args[0]
        self.assertIn("--color=saved_palette", cmd)
        self.assertTrue(any(arg.startswith("--solve_bins_file=") for arg in cmd))
        self.assertFalse(any(arg.startswith("--solve_score_clip_lo=") for arg in cmd))
        self.assertFalse(any(arg.startswith("--solve_metric=") for arg in cmd))

        requested = [call.kwargs["Key"] for call in mock_s3.get_object.call_args_list]
        self.assertIn("renders/j/palettes/p1/chunks/palette_bins_chunk_0.bin", requested)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])

    @patch("handler_raster.report_status")
    @patch("handler_raster.subprocess.run")
    @patch("handler_raster.s3")
    def test_saved_palette_mode_requires_chunk_bins_key(self, mock_s3, mock_run, mock_report):
        import handler_raster as mod

        mock_s3.get_object.return_value = {"Body": MagicMock(read=lambda: b"\x00" * 80)}

        with self.assertRaises(RuntimeError) as ctx:
            mod.handler(_event(saved_palette_bins_key=None), None)

        self.assertIn("saved_palette_bins_key", str(ctx.exception))
        self.assertFalse(mock_run.called)
        self.assertEqual(mock_report.call_args_list[-1].args[2], "error")
