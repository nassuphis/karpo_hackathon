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


class TestRasterMT(unittest.TestCase):

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_solve_score_mt_invokes_native_binary_once_and_uploads_outputs(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        uploads = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/chunk_0.bin":
                return {"Body": MagicMock(read=lambda: b"\x00" * 160)}
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
            self.assertTrue(cmd[0].endswith("roots2pix_mt"))
            self.assertEqual(cmd[1], "/tmp/stripe.bin")
            self.assertEqual(cmd[2], "/tmp/pix")
            self.assertIn("--input_mode=tmpfile", cmd)
            self.assertIn("--color=solve_score", cmd)
            self.assertIn("--threads=2", cmd)
            self.assertIn("--pixel_bin_prefix=/tmp/pixbin", cmd)
            self.assertIn("--solve_metric=crowding", cmd)
            self.assertIn("--solve_score_omega=4.0", cmd)
            self.assertIn("--solve_score_omega_enabled=0", cmd)
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"A" * 8)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"a" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"threads": 2, "roots_plotted": 24, "roots_clipped": 3}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_event(), None)
        body = json.loads(result["body"])

        self.assertEqual(body["threads"], 2)
        self.assertEqual(body["engine"], "mt")
        self.assertEqual(body["input_mode"], "tmpfile")
        self.assertEqual(body["tiles_uploaded"], 1)
        self.assertEqual(body["pixel_bin_tiles_uploaded"], 1)
        self.assertEqual(body["roots_plotted"], 24)
        self.assertEqual(body["roots_clipped"], 3)
        self.assertEqual(mock_run.call_count, 1)
        self.assertEqual(uploads["renders/j/pix_chunk_0000_t0000.pix"], b"A" * 8)
        self.assertEqual(uploads["renders/j/pixbin_chunk_0000_t0000.pbx"], b"a" * 8)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])
        done_kwargs = mock_report.call_args_list[-1].kwargs
        self.assertEqual(done_kwargs["result_data"]["engine"], "mt")
        self.assertEqual(done_kwargs["result_data"]["threads"], 2)
        self.assertEqual(done_kwargs["result_data"]["input_mode"], "tmpfile")
        warned = {w["param"] for w in done_kwargs["result_data"]["contract_warnings"]}
        self.assertIn("raster_mt_threads", warned)
        self.assertIn("raster_input_mode", warned)
        self.assertIn("raster_sectioned_retries", warned)

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_request_can_override_default_thread_count(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        mock_s3.get_object.side_effect = lambda **kwargs: (
            {"Body": MagicMock(read=lambda: b"\x00" * 160)}
            if kwargs["Key"] == "renders/j/chunk_0.bin"
            else {"Body": MagicMock(read=lambda: json.dumps({
                "family": "solve_score",
                "metric": "crowding",
                "clip_quantile": 0.01,
                "omega": 4.0,
                "omega_enabled": False,
                "clip_lo": -1.0,
                "clip_hi": 2.0,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
            }).encode())}
        )
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: None

        seen_cmds = []

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            seen_cmds.append(cmd)
            self.assertIn("--threads=3", cmd)
            self.assertIn("--input_mode=tmpfile", cmd)
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"X" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"threads": 3, "roots_plotted": 1, "roots_clipped": 0}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_event(raster_mt_threads=3), None)
        body = json.loads(result["body"])

        self.assertEqual(body["threads"], 3)
        self.assertEqual(len(seen_cmds), 1)

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_saved_palette_mt_downloads_full_chunk_bins_once(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        seen_bins = []

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/chunk_0.bin":
                return {"Body": MagicMock(read=lambda: b"\x00" * 160)}
            if key == "renders/j/palettes/p1/chunks/palette_bins_chunk_0.bin":
                return {"Body": MagicMock(read=lambda: bytes([9, 8, 7, 6]))}
            raise AssertionError(f"unexpected get_object key: {key}")

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: None

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            solve_bins_arg = next(arg for arg in cmd if arg.startswith("--solve_bins_file="))
            bins_path = solve_bins_arg.split("=", 1)[1]
            with open(bins_path, "rb") as fh:
                seen_bins.append(fh.read())
            self.assertIn("--color=saved_palette", cmd)
            self.assertIn("--threads=2", cmd)
            self.assertIn("--input_mode=tmpfile", cmd)
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"\x01" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"threads": 2, "roots_plotted": 10, "roots_clipped": 0}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_event(
            color="saved_palette",
            emit_pixel_bins=False,
            solve_score_bins_key=None,
            saved_palette_bins_key="renders/j/palettes/p1/chunks/palette_bins_chunk_0.bin",
            palette="tri_redgold",
        ), None)
        body = json.loads(result["body"])

        self.assertEqual(body["threads"], 2)
        self.assertEqual(seen_bins, [bytes([9, 8, 7, 6])])
        self.assertEqual(mock_run.call_count, 1)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2", "AWS_LAMBDA_FUNCTION_MEMORY_SIZE": "10240"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_sectioned_mode_uses_presigned_url_and_skips_python_chunk_download(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        def get_object(**kwargs):
            key = kwargs["Key"]
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

        mock_s3.get_object.side_effect = get_object
        mock_s3.head_object.return_value = {"ContentLength": 160}
        mock_s3.generate_presigned_url.return_value = "https://example.com/input.bin?sig=1"
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: None

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--input_mode=sectioned", cmd)
            self.assertIn("--url=https://example.com/input.bin?sig=1", cmd)
            self.assertIn("--input_size=160", cmd)
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"Z" * 8)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"z" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({
                    "threads": 2,
                    "roots_plotted": 11,
                    "roots_clipped": 1,
                    "input_mode": "sectioned",
                    "download_us": 1200,
                    "native_us": 3400,
                }),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_event(raster_input_mode="sectioned"), None)
        body = json.loads(result["body"])

        self.assertEqual(body["input_mode"], "sectioned")
        self.assertEqual(body["threads"], 2)
        mock_s3.head_object.assert_called_once_with(Bucket="polypaint", Key="renders/j/chunk_0.bin")
        mock_s3.generate_presigned_url.assert_called_once()
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])
        done_kwargs = mock_report.call_args_list[-1].kwargs
        self.assertEqual(done_kwargs["result_data"]["input_mode"], "sectioned")
        self.assertEqual(done_kwargs["result_data"]["download_us"], 1200)


if __name__ == "__main__":
    unittest.main()
