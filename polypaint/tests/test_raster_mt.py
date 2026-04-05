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
    def test_solve_score_mt_merges_worker_outputs_and_reports_perf(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        uploads = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/chunk_0.bin":
                return {"Body": MagicMock(read=lambda: b"\x00" * 160)}  # 4 solves, degree 5
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
            out_prefix = cmd[2]
            self.assertIn("--pixel_bin_prefix=" + out_prefix.replace("/tmp/pix_w", "/tmp/pixbin_w"), cmd)
            if out_prefix.endswith("w00"):
                pix_bytes = b"A" * 8
                pbx_bytes = b"a" * 8
                plotted = 11
                clipped = 1
            else:
                pix_bytes = b"B" * 8
                pbx_bytes = b"b" * 8
                plotted = 13
                clipped = 2
            with open(out_prefix + "_t0000.pix", "wb") as fh:
                fh.write(pix_bytes)
            with open(out_prefix.replace("/tmp/pix_w", "/tmp/pixbin_w") + "_t0000.pbx", "wb") as fh:
                fh.write(pbx_bytes)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"roots_plotted": plotted, "roots_clipped": clipped}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_event(), None)
        body = json.loads(result["body"])

        self.assertEqual(body["threads"], 2)
        self.assertEqual(body["engine"], "mt")
        self.assertEqual(body["tiles_uploaded"], 1)
        self.assertEqual(body["pixel_bin_tiles_uploaded"], 1)
        self.assertEqual(body["roots_plotted"], 24)
        self.assertEqual(body["roots_clipped"], 3)
        self.assertEqual(uploads["renders/j/pix_chunk_0000_t0000.pix"], b"A" * 8 + b"B" * 8)
        self.assertEqual(uploads["renders/j/pixbin_chunk_0000_t0000.pbx"], b"a" * 8 + b"b" * 8)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])
        done_kwargs = mock_report.call_args_list[-1].kwargs
        self.assertEqual(done_kwargs["result_data"]["engine"], "mt")
        self.assertEqual(done_kwargs["result_data"]["threads"], 2)

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

        seen = []

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            seen.append(cmd[2])
            out_prefix = cmd[2]
            with open(out_prefix + "_t0000.pix", "wb") as fh:
                fh.write(b"X" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"roots_plotted": 1, "roots_clipped": 0}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_event(raster_mt_threads=3), None)
        body = json.loads(result["body"])

        self.assertEqual(body["threads"], 3)
        self.assertEqual(len(seen), 3)

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_saved_palette_mt_slices_chunk_bins_per_worker(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        seen_slices = []

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/chunk_0.bin":
                return {"Body": MagicMock(read=lambda: b"\x00" * 160)}  # 4 solves
            if key == "renders/j/palettes/p1/chunks/palette_bins_chunk_0.bin":
                return {"Body": MagicMock(read=lambda: bytes([9, 8, 7, 6]))}
            raise AssertionError(f"unexpected get_object key: {key}")

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: None

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            solve_bins_arg = next(arg for arg in cmd if arg.startswith("--solve_bins_file="))
            bins_path = solve_bins_arg.split("=", 1)[1]
            with open(bins_path, "rb") as fh:
                seen_slices.append(fh.read())
            out_prefix = cmd[2]
            with open(out_prefix + "_t0000.pix", "wb") as fh:
                fh.write(b"\x01" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"roots_plotted": 10, "roots_clipped": 0}),
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
        self.assertEqual(sorted(seen_slices), sorted([bytes([9, 8]), bytes([7, 6])]))
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded", "rasterized", "done"])


if __name__ == "__main__":
    unittest.main()
