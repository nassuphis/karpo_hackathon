import json
import os
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "j",
        "task_id": "raster_0",
        "chunk_idx": 0,
        "bin_key": "renders/j/chunk_0.bin",
        "solve_score_bins_key": "renders/j/solve_scores/crowding_bins.json",
        "n_tile_cols": 1,
        "n_tile_rows": 1,
        "width": 512,
        "height": 512,
        "tile_size": 512,
        "center_re": 0.0,
        "center_im": 0.0,
        "scale": 1.0,
        "degree": 5,
        "color": "solve_score",
        "palette": "inferno",
        "match": "none",
        "rotation": 0.0,
        "solve_metric": "crowding",
        "solve_score_quantile": 0.01,
        "solve_score_omega": 4.0,
        "emit_pixel_bins": True,
    }
    payload.update(overrides)
    return payload


class TestRasterPixelBins(unittest.TestCase):

    @patch("handler_raster.report_status")
    @patch("handler_raster.subprocess.run")
    @patch("handler_raster.s3")
    def test_solve_score_mode_uploads_sparse_pixel_bin_sidecars(self, mock_s3, mock_run, mock_report):
        import handler_raster as mod

        uploads = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/chunk_0.bin":
                return {"Body": MagicMock(read=lambda: b"\x00" * 80)}
            if key == "renders/j/solve_scores/crowding_bins.json":
                payload = {
                    "family": "solve_score",
                    "metric": "crowding",
                    "clip_quantile": 0.01,
                    "omega": 4.0,
                    "clip_lo": -1.0,
                    "clip_hi": 2.0,
                    "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
                }
                return {"Body": MagicMock(read=lambda: json.dumps(payload).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--pixel_bin_prefix=/tmp/pixbin", cmd)
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"\x00" * 8)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"\x00" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"roots_plotted": 12, "roots_clipped": 0}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        with patch("glob.glob", return_value=[]):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pixel_bin_tiles_uploaded"], 1)
        self.assertIn("renders/j/pix_chunk_0000_t0000.pix", uploads)
        self.assertIn("renders/j/pixbin_chunk_0000_t0000.pbx", uploads)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])


if __name__ == "__main__":
    unittest.main()
