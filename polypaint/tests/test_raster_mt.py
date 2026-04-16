import json
import os
import struct
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


class _InPipe:
    def __init__(self):
        self.data = bytearray()

    def write(self, chunk):
        self.data.extend(chunk)

    def close(self):
        return None


class _FakePixbinAssemblerProc:
    def __init__(self, cmd):
        self.cmd = cmd
        self.stdin = _InPipe()
        self.stderr = MagicMock(read=lambda: b"")

    def wait(self, timeout=None):
        out_arg = next(arg for arg in self.cmd if arg.startswith("--output="))
        tile_w = int(next(arg for arg in self.cmd if arg.startswith("--tile_w=")).split("=", 1)[1])
        tile_h = int(next(arg for arg in self.cmd if arg.startswith("--tile_h=")).split("=", 1)[1])
        empty = int(next(arg for arg in self.cmd if arg.startswith("--empty=")).split("=", 1)[1])
        tile = bytearray([empty] * (tile_w * tile_h))
        data = bytes(self.stdin.data)
        self.test_case.assertEqual(len(data) % 8, 0)
        for pix_idx, bin_idx in struct.iter_unpack("<II", data):
            if pix_idx < len(tile):
                tile[pix_idx] = bin_idx if bin_idx <= 255 else empty
        with open(out_arg.split("=", 1)[1], "wb") as fh:
            fh.write(tile)
        return 0


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
        self.assertEqual(body["pixel_bin_bytes_uploaded"], 8)
        self.assertEqual(body["pixel_bin_tile_bytes"], [{"tile_idx": 0, "bytes": 8, "dense_bytes": 512 * 512}])
        self.assertEqual(body["pixel_bin_dense_bytes_if_full_tiles"], 512 * 512)
        self.assertEqual(body["roots_plotted"], 24)
        self.assertEqual(body["roots_clipped"], 3)
        self.assertEqual(mock_run.call_count, 1)
        self.assertEqual(uploads["renders/j/pix_chunk_0000_t0000.pix"], b"A" * 8)
        self.assertEqual(uploads["renders/j/pixbin_chunk_0000_t0000.pbx"], b"a" * 8)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded_1/1", "rasterized_1/1", "rasterized", "done"])
        done_kwargs = mock_report.call_args_list[-1].kwargs
        self.assertEqual(done_kwargs["result_data"]["engine"], "mt")
        self.assertEqual(done_kwargs["result_data"]["threads"], 2)
        self.assertEqual(done_kwargs["result_data"]["input_mode"], "tmpfile")
        self.assertEqual(done_kwargs["result_data"]["pixel_bin_bytes_uploaded"], 8)
        warned = {w["param"] for w in done_kwargs["result_data"]["contract_warnings"]}
        self.assertIn("raster_mt_threads", warned)
        self.assertIn("raster_input_mode", warned)
        self.assertIn("raster_sectioned_retries", warned)

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_pixel_bins_drive_rgb_mt_skips_pix_upload_and_requests_native_skip(self, mock_s3, mock_run, mock_report):
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
            self.assertIn("--pixel_bin_prefix=/tmp/pixbin", cmd)
            self.assertIn("--skip_pix_output=1", cmd)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"b" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({
                    "threads": 2,
                    "roots_plotted": 24,
                    "roots_clipped": 3,
                    "tiles_with_data": 1,
                    "skip_pix_output": True,
                }),
                stderr="",
            )

        mock_run.side_effect = fake_run

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
        self.assertEqual(uploads["renders/j/pixbin_chunk_0000_t0000.pbx"], b"b" * 8)
        done_data = mock_report.call_args_list[-1].kwargs["result_data"]
        self.assertEqual(done_data["tiles_uploaded"], 0)
        self.assertEqual(done_data["pix_tiles_skipped"], 1)
        self.assertEqual(done_data["rgb_source"], "pixel_bins")
        self.assertEqual(done_data["pixel_bin_bytes_uploaded"], 8)

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
        self.assertEqual(statuses, ["started", "bin_downloaded_1/1", "rasterized_1/1", "rasterized", "done"])

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
        self.assertEqual(statuses, ["started", "bin_downloaded_1/1", "rasterized_1/1", "rasterized", "done"])
        done_kwargs = mock_report.call_args_list[-1].kwargs
        self.assertEqual(done_kwargs["result_data"]["input_mode"], "sectioned")
        self.assertEqual(done_kwargs["result_data"]["download_us"], 1200)

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2", "AWS_LAMBDA_FUNCTION_MEMORY_SIZE": "10240"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_logical_sectioned_mode_uses_multispan_manifests(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod
        from logical_sections import build_solve_source_manifest

        bins_meta = {
            "family": "solve_score",
            "version": 2,
            "metric": "spread",
            "clip_quantile": 0.02,
            "omega": 1.0,
            "omega_enabled": False,
            "clip_lo": 0.0,
            "clip_hi": 1.0,
            "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
            "program": "m0;m1;avg;m2;avg",
            "metrics": [
                {"slot": 0, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                {"slot": 1, "metric": "dist_unit_circle", "source": "cf", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                {"slot": 2, "metric": "t2_abs", "source": "pm", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
            ],
        }

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/solve_scores/crowding_bins.json":
                return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        def presign(_op, Params=None, ExpiresIn=None):
            return f"https://example.com/{Params['Key']}?sig=1"

        mock_s3.get_object.side_effect = get_object
        mock_s3.generate_presigned_url.side_effect = presign
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: None

        solve_source_manifest = build_solve_source_manifest(
            [
                {
                    "chunk_idx": 0,
                    "bin_key": "renders/j/chunk_0.bin",
                    "coeffs_key": "renders/j/coeffs_0000.bin",
                    "step_start": 0,
                    "step_count": 2,
                    "params_key": "renders/j/params.bin",
                    "params_step_start": 0,
                    "params_step_count": 2,
                },
                {
                    "chunk_idx": 1,
                    "bin_key": "renders/j/chunk_1.bin",
                    "coeffs_key": "renders/j/coeffs_0001.bin",
                    "step_start": 2,
                    "step_count": 2,
                    "params_key": "renders/j/params.bin",
                    "params_step_start": 2,
                    "params_step_count": 2,
                },
            ],
            job_id="j",
            degree=5,
            n_coeffs=7,
        )

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--input_mode=multispan_sectioned", cmd)
            input_manifest_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--input_manifest="))
            coeff_manifest_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--score_coeff_manifest="))
            param_manifest_path = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--score_params_manifest="))
            with open(input_manifest_path, "r", encoding="utf-8") as fh:
                input_manifest = json.load(fh)
            with open(coeff_manifest_path, "r", encoding="utf-8") as fh:
                coeff_manifest = json.load(fh)
            with open(param_manifest_path, "r", encoding="utf-8") as fh:
                param_manifest = json.load(fh)
            self.assertEqual(input_manifest["logical_size"], 4 * 5 * 2 * 4)
            self.assertEqual(coeff_manifest["logical_size"], 4 * 7 * 2 * 4)
            self.assertEqual(param_manifest["logical_size"], 4 * 16)
            self.assertEqual(len(input_manifest["spans"]), 2)
            self.assertEqual(len(coeff_manifest["spans"]), 2)
            self.assertEqual(len(param_manifest["spans"]), 2)
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"M" * 8)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"m" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({
                    "threads": 2,
                    "roots_plotted": 9,
                    "roots_clipped": 1,
                    "input_mode": "multispan_sectioned",
                    "download_us": 2200,
                    "native_us": 4100,
                }),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_event(
            section_idx=0,
            section_count=1,
            bin_key="",
            coeffs_key="",
            params_key="",
            logical_section=True,
            solve_source_manifest=solve_source_manifest,
            step_start=0,
            step_count=4,
            bin_size=0,
            coeffs_bin_size=0,
            params_step_start=0,
            params_step_count=4,
            raster_input_mode="sectioned",
            n_coeffs=7,
        ), None)
        body = json.loads(result["body"])

        self.assertEqual(body["input_mode"], "multispan_sectioned")
        done_kwargs = mock_report.call_args_list[-1].kwargs
        self.assertEqual(done_kwargs["result_data"]["requested_input_mode"], "sectioned")
        self.assertEqual(done_kwargs["result_data"]["input_mode"], "multispan_sectioned")

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_v2_solve_score_bins_use_program_cli_flags(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/chunk_0.bin":
                return {"Body": MagicMock(read=lambda: b"\x00" * 160)}
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
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: None

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
                fh.write(b"V" * 8)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"v" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"threads": 2, "roots_plotted": 24, "roots_clipped": 3}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_event(
            solve_metric="spread",
            solve_score_quantile=0.02,
            solve_score_omega=5.0,
            solve_score_omega_enabled=True,
        ), None)
        body = json.loads(result["body"])

        self.assertEqual(body["threads"], 2)
        self.assertEqual(body["engine"], "mt")

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2", "AWS_LAMBDA_FUNCTION_MEMORY_SIZE": "10240"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_v2_mixed_source_sectioned_bins_pass_coeff_url_cli_flags(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        def get_object(**kwargs):
            key = kwargs["Key"]
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
                    "program": "m0;m1;max",
                    "metrics": [
                        {"slot": 0, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                        {"slot": 1, "metric": "spread", "source": "cf", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    ],
                }
                return {"Body": MagicMock(read=lambda: json.dumps(payload).encode())}
            raise AssertionError(f"unexpected get_object key: {key}")

        mock_s3.get_object.side_effect = get_object
        mock_s3.generate_presigned_url.side_effect = [
            "https://example.com/input.bin?sig=1",
            "https://example.com/coeffs.bin?sig=2",
        ]
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: None

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--input_mode=sectioned", cmd)
            self.assertIn("--url=https://example.com/input.bin?sig=1", cmd)
            self.assertIn("--score_sources=slv,cf", cmd)
            self.assertIn("--score_program=m0;m1;max", cmd)
            self.assertIn("--score_coeffs_url=https://example.com/coeffs.bin?sig=2", cmd)
            self.assertIn("--score_coeff_input_size=112", cmd)
            self.assertIn("--score_coeff_degree=7", cmd)
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"Q" * 8)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"q" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"threads": 2, "roots_plotted": 11, "roots_clipped": 1}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_event(
            solve_metric="spread",
            solve_score_quantile=0.02,
            solve_score_bins_key="renders/j/solve_scores/crowding_bins.json",
            raster_input_mode="sectioned",
            bin_size=160,
            coeffs_key="renders/j/coeffs_0000.bin",
            coeffs_bin_size=112,
            n_coeffs=7,
        ), None)
        body = json.loads(result["body"])

        self.assertEqual(body["threads"], 2)
        self.assertEqual(body["input_mode"], "sectioned")
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded_1/1", "rasterized_1/1", "rasterized", "done"])

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2", "AWS_LAMBDA_FUNCTION_MEMORY_SIZE": "10240"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_v2_param_source_sectioned_bins_pass_param_file_cli_flags(self, mock_s3, mock_run, mock_report):
        import handler_raster_mt as mod

        def get_object(**kwargs):
            key = kwargs["Key"]
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
                    "program": "m0;m1;max",
                    "metrics": [
                        {"slot": 0, "metric": "t1_abs", "source": "pm", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                        {"slot": 1, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    ],
                }
                return {"Body": MagicMock(read=lambda: json.dumps(payload).encode())}
            if key == "renders/j/params.bin":
                self.assertEqual(kwargs["Range"], "bytes=64-127")
                return {"Body": MagicMock(read=lambda: b"\x22" * 64)}
            raise AssertionError(f"unexpected get_object key: {key}")

        mock_s3.get_object.side_effect = get_object
        mock_s3.generate_presigned_url.side_effect = [
            "https://example.com/input.bin?sig=1",
        ]
        mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key: None

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--input_mode=sectioned", cmd)
            self.assertIn("--url=https://example.com/input.bin?sig=1", cmd)
            self.assertIn("--score_sources=pm,slv", cmd)
            self.assertIn("--score_program=m0;m1;max", cmd)
            self.assertIn("--score_params_file=/tmp/score_params.bin", cmd)
            with open("/tmp/pix_t0000.pix", "wb") as fh:
                fh.write(b"Q" * 8)
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                fh.write(b"q" * 8)
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"threads": 2, "roots_plotted": 11, "roots_clipped": 1}),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler(_event(
            solve_metric="t1_abs",
            solve_score_quantile=0.02,
            solve_score_bins_key="renders/j/solve_scores/param_bins.json",
            raster_input_mode="sectioned",
            bin_size=160,
            params_key="renders/j/params.bin",
            step_start=4,
            step_count=4,
        ), None)
        body = json.loads(result["body"])

        self.assertEqual(body["threads"], 2)
        self.assertEqual(body["input_mode"], "sectioned")
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "bin_downloaded_1/1", "rasterized_1/1", "rasterized", "done"])

    @patch.dict(os.environ, {"RASTER_MT_THREADS": "2"}, clear=False)
    @patch("handler_raster_mt.report_status")
    @patch("handler_raster_mt.subprocess.Popen")
    @patch("handler_raster_mt.subprocess.run")
    @patch("handler_raster_mt.s3")
    def test_dense_grouped_pixel_bins_runs_all_chunks_and_uploads_dense_group_tiles(self, mock_s3, mock_run, mock_popen, mock_report):
        import handler_raster_mt as mod

        uploads = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key in ("renders/j/chunk_0.bin", "renders/j/chunk_1.bin"):
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

        def fake_run(cmd, capture_output=False, text=False, timeout=None):
            self.assertIn("--skip_pix_output=1", cmd)
            bin_path = cmd[1]
            with open(bin_path, "rb") as fh:
                chunk_bytes = fh.read()
            if chunk_bytes == b"\x00" * 160:
                call_idx = len(mock_run.call_args_list)
            else:
                raise AssertionError("unexpected chunk bytes")
            with open("/tmp/pixbin_t0000.pbx", "wb") as fh:
                if call_idx == 1:
                    fh.write(struct.pack("<II", 0, 2))
                    fh.write(struct.pack("<II", 3, 4))
                else:
                    fh.write(struct.pack("<II", 1, 6))
            return MagicMock(
                returncode=0,
                stdout=json.dumps({"threads": 2, "roots_plotted": 10, "roots_clipped": 1, "tiles_with_data": 1}),
                stderr="",
            )

        def fake_popen(cmd, stdin=None, stderr=None):
            self.assertEqual(os.path.basename(cmd[0]), "pixbinassemble")
            proc = _FakePixbinAssemblerProc(cmd)
            proc.test_case = self
            return proc

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_run.side_effect = fake_run
        mock_popen.side_effect = fake_popen

        result = mod.handler(_event(
            pixel_bins_drive_rgb=True,
            pixel_bin_fragment_mode="dense_grouped",
            group_idx=0,
            section_indices=[0, 1],
            sections=[
                {
                    "section_idx": 0,
                    "section_count": 2,
                    "bin_key": "renders/j/chunk_0.bin",
                    "coeffs_key": "renders/j/coeffs_0000.bin",
                    "coeffs_bin_size": 0,
                    "step_start": 0,
                    "step_count": 0,
                    "bin_size": 160,
                },
                {
                    "section_idx": 1,
                    "section_count": 2,
                    "bin_key": "renders/j/chunk_1.bin",
                    "coeffs_key": "renders/j/coeffs_0001.bin",
                    "coeffs_bin_size": 0,
                    "step_start": 0,
                    "step_count": 0,
                    "bin_size": 160,
                },
            ],
            width=2,
            height=2,
            tile_size=2,
        ), None)
        body = json.loads(result["body"])

        self.assertEqual(mock_run.call_count, 2)
        self.assertEqual(body["pixel_bin_fragment_mode"], "dense_grouped")
        self.assertEqual(body["group_idx"], 0)
        self.assertEqual(body["section_indices"], [0, 1])
        self.assertEqual(body["pixel_bin_tiles_uploaded"], 1)
        self.assertEqual(body["pixel_bin_bytes_uploaded"], 4)
        self.assertEqual(body["pixel_bin_sparse_files_in"], 2)
        self.assertEqual(body["pixel_bin_sparse_bytes_in"], 24)
        self.assertEqual(uploads["renders/j/pixbin_group_0000_t0000.u8"], bytes([2, 6, 255, 4]))
        self.assertNotIn("renders/j/pixbin_chunk_0000_t0000.pbx", uploads)
        self.assertNotIn("renders/j/pix_chunk_0000_t0000.pix", uploads)
        done_data = mock_report.call_args_list[-1].kwargs["result_data"]
        self.assertEqual(done_data["pixel_bin_fragment_mode"], "dense_grouped")
        self.assertEqual(done_data["pixel_bin_sparse_files_in"], 2)


if __name__ == "__main__":
    unittest.main()
