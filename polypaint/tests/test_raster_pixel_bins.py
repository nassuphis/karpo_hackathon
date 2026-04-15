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
        "solve_score_omega_enabled": False,
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
                    "omega_enabled": False,
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
            self.assertIn("--solve_score_omega_enabled=0", cmd)
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
        self.assertEqual(body["pixel_bin_bytes_uploaded"], 8)
        self.assertEqual(body["pixel_bin_tile_bytes"], [{"tile_idx": 0, "bytes": 8, "dense_bytes": 512 * 512}])
        self.assertEqual(body["pixel_bin_dense_bytes_if_full_tiles"], 512 * 512)
        self.assertIn("renders/j/pix_chunk_0000_t0000.pix", uploads)
        self.assertIn("renders/j/pixbin_chunk_0000_t0000.pbx", uploads)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])

    @patch("handler_raster.report_status")
    @patch("handler_raster.subprocess.run")
    @patch("handler_raster.s3")
    def test_pixel_bins_drive_rgb_skips_pix_upload_and_requests_native_skip(self, mock_s3, mock_run, mock_report):
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
                    "omega_enabled": False,
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
            self.assertIn("--skip_pix_output=1", cmd)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"\x09" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"roots_plotted": 12, "roots_clipped": 0, "tiles_with_data": 1, "skip_pix_output": True}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        with patch("glob.glob", return_value=[]):
            result = mod.handler(_event(pixel_bins_drive_rgb=True), None)

        body = json.loads(result["body"])
        self.assertEqual(body["tiles_uploaded"], 0)
        self.assertEqual(body["pixel_bin_tiles_uploaded"], 1)
        self.assertEqual(body["pixel_bin_bytes_uploaded"], 8)
        self.assertEqual(body["pixel_bin_tile_bytes"], [{"tile_idx": 0, "bytes": 8, "dense_bytes": 512 * 512}])
        self.assertTrue(body["pixel_bins_drive_rgb"])
        self.assertEqual(body["rgb_source"], "pixel_bins")
        self.assertEqual(body["pix_tiles_skipped"], 1)
        self.assertNotIn("renders/j/pix_chunk_0000_t0000.pix", uploads)
        self.assertEqual(uploads["renders/j/pixbin_chunk_0000_t0000.pbx"], b"\x09" * 8)
        done_data = mock_report.call_args_list[-1].kwargs["result_data"]
        self.assertEqual(done_data["tiles_uploaded"], 0)
        self.assertEqual(done_data["pix_tiles_skipped"], 1)
        self.assertEqual(done_data["rgb_source"], "pixel_bins")
        self.assertEqual(done_data["pixel_bin_bytes_uploaded"], 8)

    @patch("handler_raster.report_status")
    @patch("handler_raster.subprocess.run")
    @patch("handler_raster.s3")
    def test_v2_solve_score_bins_use_program_cli_flags(self, mock_s3, mock_run, mock_report):
        import handler_raster as mod

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/chunk_0.bin":
                return {"Body": MagicMock(read=lambda: b"\x00" * 80)}
            if key == "renders/j/solve_scores/crowding_bins.json":
                payload = {
                    "family": "solve_score",
                    "version": 2,
                    "metric": "spread",
                    "clip_quantile": 0.02,
                    "omega": 5.0,
                    "omega_enabled": True,
                    "clip_lo": -1.0,
                    "clip_hi": 2.0,
                    "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
                    "program": "m0;m1;weighted_sum:0.7:0.3;omega_cosine:5",
                    "metrics": [
                        {"slot": 0, "metric": "spread", "quantile": 0.02, "clip_lo": -1.0, "clip_hi": 2.0},
                        {"slot": 1, "metric": "shelliness", "quantile": 0.03, "clip_lo": -0.5, "clip_hi": 1.5},
                    ],
                }
                return {"Body": MagicMock(read=lambda: json.dumps(payload).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        mock_s3.get_object.side_effect = get_object

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--color=solve_score", cmd)
            self.assertIn("--score_metrics=spread,shelliness", cmd)
            self.assertIn("--score_clip_los=-1,-0.5", cmd)
            self.assertIn("--score_clip_his=2,1.5", cmd)
            self.assertIn("--score_program=m0;m1;weighted_sum:0.7:0.3;omega_cosine:5", cmd)
            self.assertFalse(any(arg.startswith("--solve_metric=") for arg in cmd))
            self.assertFalse(any(arg.startswith("--solve_score_clip_lo=") for arg in cmd))
            self.assertFalse(any(arg.startswith("--solve_score_omega=") for arg in cmd))
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"\x01" * 8)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"\x02" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"roots_plotted": 20, "roots_clipped": 1}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        with patch("glob.glob", return_value=[]):
            result = mod.handler(_event(
                solve_metric="spread",
                solve_score_quantile=0.02,
                solve_score_omega=5.0,
                solve_score_omega_enabled=True,
            ), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pixel_bin_tiles_uploaded"], 1)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])

    @patch("handler_raster.report_status")
    @patch("handler_raster.subprocess.run")
    @patch("handler_raster.s3")
    def test_v2_mixed_source_bins_download_coeff_chunk_and_pass_cli_flags(self, mock_s3, mock_run, mock_report):
        import handler_raster as mod

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/chunk_0.bin":
                return {"Body": MagicMock(read=lambda: b"\x00" * 80)}
            if key == "renders/j/coeffs_0000.bin":
                return {"Body": MagicMock(read=lambda: b"\x11" * 112)}
            if key == "renders/j/solve_scores/crowding_bins.json":
                payload = {
                    "family": "solve_score",
                    "version": 2,
                    "metric": "spread",
                    "clip_quantile": 0.02,
                    "omega": 1.0,
                    "omega_enabled": False,
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
                    "program": "m0;m1;avg",
                    "metrics": [
                        {"slot": 0, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                        {"slot": 1, "metric": "spread", "source": "cf", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    ],
                }
                return {"Body": MagicMock(read=lambda: json.dumps(payload).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: None

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--score_metrics=spread,spread", cmd)
            self.assertIn("--score_sources=slv,cf", cmd)
            self.assertIn("--score_program=m0;m1;avg", cmd)
            self.assertIn("--score_coeffs_file=/tmp/score_coeffs.bin", cmd)
            self.assertIn("--score_coeff_degree=7", cmd)
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"\x03" * 8)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"\x04" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"roots_plotted": 10, "roots_clipped": 0}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        with patch("glob.glob", return_value=[]):
            result = mod.handler(_event(
                solve_metric="spread",
                solve_score_quantile=0.02,
                solve_score_bins_key="renders/j/solve_scores/crowding_bins.json",
                coeffs_key="renders/j/coeffs_0000.bin",
                n_coeffs=7,
            ), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pixel_bin_tiles_uploaded"], 1)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])

    @patch("handler_raster.report_status")
    @patch("handler_raster.subprocess.run")
    @patch("handler_raster.s3")
    def test_v2_param_source_bins_download_param_slice_and_pass_cli_flags(self, mock_s3, mock_run, mock_report):
        import handler_raster as mod

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/chunk_0.bin":
                return {"Body": MagicMock(read=lambda: b"\x00" * 80)}
            if key == "renders/j/params.bin":
                self.assertEqual(kwargs["Range"], "bytes=64-127")
                return {"Body": MagicMock(read=lambda: b"\x22" * 64)}
            if key == "renders/j/solve_scores/param_bins.json":
                payload = {
                    "family": "solve_score",
                    "version": 2,
                    "metric": "t1_abs",
                    "clip_quantile": 0.02,
                    "omega": 1.0,
                    "omega_enabled": False,
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
                    "program": "m0;m1;avg",
                    "metrics": [
                        {"slot": 0, "metric": "t1_abs", "source": "pm", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                        {"slot": 1, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    ],
                }
                return {"Body": MagicMock(read=lambda: json.dumps(payload).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: None

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--score_metrics=t1_abs,spread", cmd)
            self.assertIn("--score_sources=pm,slv", cmd)
            self.assertIn("--score_program=m0;m1;avg", cmd)
            self.assertIn("--score_params_file=/tmp/score_params.bin", cmd)
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"\x03" * 8)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"\x04" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"roots_plotted": 10, "roots_clipped": 0}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        with patch("glob.glob", return_value=[]):
            result = mod.handler(_event(
                solve_metric="t1_abs",
                solve_score_quantile=0.02,
                solve_score_bins_key="renders/j/solve_scores/param_bins.json",
                params_key="renders/j/params.bin",
                step_start=4,
                step_count=4,
            ), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pixel_bin_tiles_uploaded"], 1)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])


if __name__ == "__main__":
    unittest.main()
