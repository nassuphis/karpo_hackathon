"""
Tests for handler_palette_chunk.py.

Validates chunk scoring/bin upload and failure handling without invoking the real binary.
"""
import json
import os
import sys
import tempfile
import unittest
from array import array
from unittest.mock import MagicMock, patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "j",
        "task_id": "palette_run_chunk_3",
        "chunk_idx": 3,
        "bin_key": "renders/j/chunk_3.bin",
        "degree": 5,
        "metric": "crowding",
        "solve_score_quantile": 0.01,
        "solve_score_bins_key": "renders/j/palettes/p1/solve_score/crowding_bins.json",
        "step_start": 10,
        "step_count": 4,
        "root_transforms": [["rotate_roots", "0.25"]],
        "palette_chunk_threads": 1,
        "palette_chunk_input_mode": "tmpfile",
        "palette_chunk_retries": 2,
        "palette_chunk_workers": 16,
        "score_key": "renders/j/palettes/p1/chunks/score_chunk_3.bin",
        "palette_bins_key": "renders/j/palettes/p1/chunks/palette_bins_chunk_3.bin",
        "meta_key": "renders/j/palettes/p1/chunks/meta_chunk_3.json",
    }
    payload.update(overrides)
    return payload


def _fingerprint(chain, metric="proximity", quantile=0.001, omega=1.0, omega_enabled=True):
    from solve_score_chain import compile_solve_score_chain_or_legacy, compiled_solve_score_fingerprint
    compiled = compile_solve_score_chain_or_legacy(
        chain,
        metric,
        quantile,
        omega,
        omega_enabled,
        default_metric=metric,
    )
    return compiled_solve_score_fingerprint(compiled)


class TestPaletteChunkHandler(unittest.TestCase):

    def test_v2_bins_allow_lagged_score_programs(self):
        import handler_palette_chunk as mod

        bins_meta = {
            "family": "solve_score",
            "version": 2,
            "program": "m0-0;m0-1;abs_diff",
            "metrics": [
                {
                    "slot": 0,
                    "source": "slv",
                    "metric": "proximity",
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                }
            ],
        }

        args = mod._solve_score_program_args(bins_meta)
        self.assertIn("--score_program=v2;m0-0;m0-1;abs_diff", args)
        self.assertIn("--score_metrics=proximity", args)

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_success_uploads_score_bins_and_meta(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT", os.path.join(td, "input.bin")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_XFORMS", os.path.join(td, "xforms.json")):

            chunk_bytes = b"\x00" * (5 * 2 * 4 * 4)
            bins_meta = {
                "family": "solve_score",
                "metric": "crowding",
                "clip_quantile": 0.01,
                "clip_lo": -1.5,
                "clip_hi": 2.5,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
            }

            def get_object(**kwargs):
                key = kwargs["Key"]
                if key == "renders/j/chunk_3.bin":
                    body = MagicMock()
                    body.iter_chunks.return_value = [chunk_bytes]
                    return {"Body": body}
                if key == "renders/j/palettes/p1/solve_score/crowding_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                raise AssertionError(f"unexpected get_object key: {key}")

            mock_s3.get_object.side_effect = get_object
            uploads = {}

            def upload_fileobj(fileobj, bucket, key, ExtraArgs=None):
                uploads[key] = fileobj.read()

            mock_s3.upload_fileobj.side_effect = upload_fileobj

            def run_side_effect(cmd, capture_output, text, timeout):
                self.assertIn("--degree=5", cmd)
                self.assertIn("--metric=crowding", cmd)
                self.assertIn("--step_count=4", cmd)
                self.assertTrue(any(a.startswith("--cuts=") for a in cmd))
                self.assertTrue(any(a.startswith("--root_xforms=") for a in cmd))

                scores = array("f", [1.25, 2.25, 3.25, 4.25])
                with open(mod._TMP_SCORES, "wb") as f:
                    scores.tofile(f)
                with open(mod._TMP_BINS, "wb") as f:
                    f.write(bytes([1, 3, 5, 7]))
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({"min_score": 1.25, "max_score": 4.25}),
                    stderr="",
                )

            mock_run.side_effect = run_side_effect

            result = mod.handler(_event(), None)
            body = json.loads(result["body"])

            self.assertEqual(body["section_idx"], 3)
            self.assertEqual(body["step_start"], 10)
            self.assertEqual(body["step_count"], 4)
            self.assertIn("renders/j/palettes/p1/chunks/score_chunk_3.bin", uploads)
            self.assertIn("renders/j/palettes/p1/chunks/palette_bins_chunk_3.bin", uploads)
            self.assertEqual(len(uploads["renders/j/palettes/p1/chunks/score_chunk_3.bin"]), 16)
            self.assertEqual(uploads["renders/j/palettes/p1/chunks/palette_bins_chunk_3.bin"], bytes([1, 3, 5, 7]))

            meta_call = mock_s3.put_object.call_args.kwargs
            meta = json.loads(meta_call["Body"])
            self.assertEqual(meta["section_idx"], 3)
            self.assertEqual(meta["step_start"], 10)
            self.assertEqual(meta["step_count"], 4)
            self.assertEqual(meta["metric"], "crowding")
            self.assertEqual(meta["min_score"], 1.25)
            self.assertEqual(meta["max_score"], 4.25)

            statuses = [c.args[2] for c in mock_report.call_args_list]
            self.assertEqual(statuses, ["started", "bin_downloaded", "computed", "done"])
            done_kwargs = mock_report.call_args_list[-1].kwargs
            self.assertEqual(done_kwargs["result_data"].get("contract_warning_count", 0), 0)
            warned = {w["param"] for w in done_kwargs["result_data"].get("contract_warnings", [])}
            self.assertEqual(warned, set())
            self.assertEqual(done_kwargs["result_data"]["threads"], 1)
            self.assertEqual(done_kwargs["result_data"]["input_mode"], "tmpfile")
            self.assertEqual(done_kwargs["result_data"]["retries"], 2)
            self.assertEqual(done_kwargs["result_data"]["workers"], 16)

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_explicit_rgb_output_uses_mt_and_channel_packed_outputs(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT", os.path.join(td, "input.bin")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_XFORMS", os.path.join(td, "xforms.json")):

            chunk_bytes = b"\x00" * (5 * 2 * 4 * 4)
            bins_meta = {
                "family": "solve_score",
                "version": 2,
                "program": "m0-0;emit_norm;m1-0;emit_norm;m2-0;emit_norm",
                "metrics": [
                    {"slot": 0, "source": "slv", "metric": "proximity", "clip_lo": 0.0, "clip_hi": 1.0},
                    {"slot": 1, "source": "slv", "metric": "spread", "clip_lo": 0.0, "clip_hi": 2.0},
                    {"slot": 2, "source": "slv", "metric": "angular_entropy_16", "clip_lo": 0.0, "clip_hi": 3.0},
                ],
                "chain_fingerprint": "fp_rgb",
                "score_output_channel_count": 3,
                "score_output_channels": [
                    {"channel": 0, "range_normalized": True, "clip_lo": 0.0, "clip_hi": 1.0},
                    {"channel": 1, "range_normalized": True, "clip_lo": 0.0, "clip_hi": 2.0},
                    {"channel": 2, "range_normalized": True, "clip_lo": 0.0, "clip_hi": 3.0},
                ],
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
            }

            def get_object(**kwargs):
                key = kwargs["Key"]
                if key == "renders/j/chunk_3.bin":
                    body = MagicMock()
                    body.iter_chunks.return_value = [chunk_bytes]
                    return {"Body": body}
                if key == "renders/j/palettes/p1/solve_score/crowding_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                raise AssertionError(f"unexpected get_object key: {key}")

            mock_s3.get_object.side_effect = get_object
            uploads = {}
            mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key, ExtraArgs=None: uploads.setdefault(key, fileobj.read())

            def run_side_effect(cmd, capture_output, text, timeout):
                self.assertEqual(cmd[0], mod.BINARY_MT)
                self.assertIn("--score_output_channel_count=3", cmd)
                self.assertIn("--score_output_clip_los=0.0,0.0,0.0", cmd)
                self.assertIn("--score_output_clip_his=1.0,2.0,3.0", cmd)
                scores = array("f", [float(i) for i in range(12)])
                with open(mod._TMP_SCORES, "wb") as f:
                    scores.tofile(f)
                with open(mod._TMP_BINS, "wb") as f:
                    f.write(bytes(range(12)))
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({"min_score": 0.0, "max_score": 11.0, "output_channel_count": 3}),
                    stderr="",
                )

            mock_run.side_effect = run_side_effect

            result = mod.handler(_event(
                score_output_channel_count=3,
                score_output_channels=bins_meta["score_output_channels"],
                score_output_has_explicit_outputs=True,
                color_interpretation="rgb",
                solve_score_chain_fingerprint="fp_rgb",
            ), None)
            body = json.loads(result["body"])

            self.assertEqual(len(uploads["renders/j/palettes/p1/chunks/score_chunk_3.bin"]), 48)
            self.assertEqual(uploads["renders/j/palettes/p1/chunks/palette_bins_chunk_3.bin"], bytes(range(12)))
            self.assertEqual(body["score_output_channel_count"], 3)
            self.assertEqual(body["raw_channels"], 3)
            self.assertEqual(body["color_interpretation"], "rgb")
            meta = json.loads(mock_s3.put_object.call_args.kwargs["Body"])
            self.assertEqual(meta["raw_channels"], 3)
            self.assertEqual(meta["score_output_channel_count"], 3)
            self.assertTrue(meta["raw_output_path"])

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_bins_quantile_mismatch_raises_and_reports_error(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT", os.path.join(td, "input.bin")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_XFORMS", os.path.join(td, "xforms.json")):

            chunk_bytes = b"\x00" * 32
            bins_meta = {
                "family": "solve_score",
                "metric": "crowding",
                "clip_quantile": 0.02,
                "clip_lo": -1.5,
                "clip_hi": 2.5,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
            }

            def get_object(**kwargs):
                key = kwargs["Key"]
                if key == "renders/j/chunk_3.bin":
                    body = MagicMock()
                    body.iter_chunks.return_value = [chunk_bytes]
                    return {"Body": body}
                if key == "renders/j/palettes/p1/solve_score/crowding_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                raise AssertionError(f"unexpected get_object key: {key}")

            mock_s3.get_object.side_effect = get_object

            with self.assertRaises(RuntimeError) as ctx:
                mod.handler(_event(), None)

            self.assertIn("quantile mismatch", str(ctx.exception))
            mock_run.assert_not_called()
            self.assertEqual(mock_report.call_args_list[-1].args[2], "error")

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_sectioned_path_passes_mt_knobs_and_presigned_url(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT", os.path.join(td, "input.bin")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_XFORMS", os.path.join(td, "xforms.json")), \
             patch.object(mod, "_sectioned_input_size_limit", return_value=10_000_000):

            bins_meta = {
                "family": "solve_score",
                "metric": "crowding",
                "clip_quantile": 0.01,
                "clip_lo": -1.5,
                "clip_hi": 2.5,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
            }

            def get_object(**kwargs):
                key = kwargs["Key"]
                if key == "renders/j/palettes/p1/solve_score/crowding_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                raise AssertionError(f"unexpected get_object key: {key}")

            mock_s3.get_object.side_effect = get_object
            mock_s3.head_object.return_value = {"ContentLength": 160}
            mock_s3.generate_presigned_url.return_value = "https://example.com/chunk_3.bin"
            mock_s3.upload_fileobj.side_effect = lambda *args, **kwargs: None

            def run_side_effect(cmd, capture_output, text, timeout):
                self.assertEqual(cmd[0], mod.BINARY_MT)
                self.assertIn("--threads=6", cmd)
                self.assertIn("--input_mode=sectioned", cmd)
                self.assertIn("--retries=4", cmd)
                self.assertIn("--url=https://example.com/chunk_3.bin", cmd)
                self.assertIn("--input_size=160", cmd)
                scores = array("f", [1.0, 2.0, 3.0, 4.0])
                with open(mod._TMP_SCORES, "wb") as f:
                    scores.tofile(f)
                with open(mod._TMP_BINS, "wb") as f:
                    f.write(bytes([0, 1, 2, 3]))
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({
                        "min_score": 1.0,
                        "max_score": 4.0,
                        "threads": 6,
                        "input_mode": "sectioned",
                        "retries": 4,
                        "download_ms": 11,
                        "compute_ms": 22,
                    }),
                    stderr="",
                )

            mock_run.side_effect = run_side_effect

            mod.handler(_event(
                palette_chunk_threads=6,
                palette_chunk_input_mode="sectioned",
                palette_chunk_retries=4,
                palette_chunk_workers=32,
            ), None)

            done_kwargs = mock_report.call_args_list[-1].kwargs
            self.assertEqual(done_kwargs["result_data"]["threads"], 6)
            self.assertEqual(done_kwargs["result_data"]["input_mode"], "sectioned")
            self.assertEqual(done_kwargs["result_data"]["retries"], 4)
            self.assertEqual(done_kwargs["result_data"]["workers"], 32)
            self.assertEqual(done_kwargs["result_data"]["dl_ms"], 11)
            self.assertEqual(done_kwargs["result_data"]["compute_ms"], 22)

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_v2_bins_use_program_cli_flags(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT", os.path.join(td, "input.bin")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_XFORMS", os.path.join(td, "xforms.json")):

            chunk_bytes = b"\x00" * (5 * 2 * 4 * 4)
            bins_meta = {
                "family": "solve_score",
                "version": 2,
                "metric": "spread",
                "clip_quantile": 0.02,
                "omega": 5.0,
                "omega_enabled": True,
                "clip_lo": -1.5,
                "clip_hi": 2.5,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
                "program": "m0;m1;weighted_sum:0.7:0.3;omega_cosine:5",
                "metrics": [
                    {"slot": 0, "metric": "spread", "quantile": 0.02, "clip_lo": -1.5, "clip_hi": 2.5},
                    {"slot": 1, "metric": "shelliness", "quantile": 0.03, "clip_lo": -0.75, "clip_hi": 1.75},
                ],
            }

            def get_object(**kwargs):
                key = kwargs["Key"]
                if key == "renders/j/chunk_3.bin":
                    body = MagicMock()
                    body.iter_chunks.return_value = [chunk_bytes]
                    return {"Body": body}
                if key == "renders/j/palettes/p1/solve_score/crowding_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                raise AssertionError(f"unexpected get_object key: {key}")

            mock_s3.get_object.side_effect = get_object
            mock_s3.upload_fileobj.side_effect = lambda *args, **kwargs: None

            def run_side_effect(cmd, capture_output, text, timeout):
                self.assertIn("--score_metrics=spread,shelliness", cmd)
                self.assertIn("--score_clip_los=-1.5,-0.75", cmd)
                self.assertIn("--score_clip_his=2.5,1.75", cmd)
                self.assertIn("--score_program=v2;m0-0;m1-0;weighted_sum:0.7:0.3;omega_cosine:5", cmd)
                self.assertFalse(any(a.startswith("--metric=") for a in cmd))
                self.assertFalse(any(a.startswith("--clip_lo=") for a in cmd))
                self.assertFalse(any(a.startswith("--clip_hi=") for a in cmd))
                scores = array("f", [1.25, 2.25, 3.25, 4.25])
                with open(mod._TMP_SCORES, "wb") as f:
                    scores.tofile(f)
                with open(mod._TMP_BINS, "wb") as f:
                    f.write(bytes([1, 3, 5, 7]))
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({"min_score": 1.25, "max_score": 4.25}),
                    stderr="",
                )

            mock_run.side_effect = run_side_effect

            result = mod.handler(_event(
                metric="spread",
                solve_score_quantile=0.02,
                solve_score_omega=5.0,
                solve_score_omega_enabled=True,
            ), None)
            body = json.loads(result["body"])

            self.assertEqual(body["section_idx"], 3)
            statuses = [c.args[2] for c in mock_report.call_args_list]
            self.assertEqual(statuses, ["started", "bin_downloaded", "computed", "done"])

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_v2_mixed_source_sectioned_bins_pass_coeff_url_cli_flags(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT", os.path.join(td, "input.bin")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_XFORMS", os.path.join(td, "xforms.json")), \
             patch.object(mod, "_sectioned_input_size_limit", return_value=10_000_000):

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
                "program": "m0;m1;max",
                "metrics": [
                    {"slot": 0, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    {"slot": 1, "metric": "spread", "source": "cf", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                ],
            }

            def get_object(**kwargs):
                key = kwargs["Key"]
                if key == "renders/j/palettes/p1/solve_score/crowding_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                raise AssertionError(f"unexpected get_object key: {key}")

            mock_s3.get_object.side_effect = get_object
            mock_s3.generate_presigned_url.side_effect = [
                "https://example.com/chunk_3.bin",
                "https://example.com/coeffs_3.bin",
            ]
            mock_s3.upload_fileobj.side_effect = lambda *args, **kwargs: None

            def run_side_effect(cmd, capture_output, text, timeout):
                self.assertEqual(cmd[0], mod.BINARY_MT)
                self.assertIn("--input_mode=sectioned", cmd)
                self.assertIn("--url=https://example.com/chunk_3.bin", cmd)
                self.assertIn("--score_sources=slv,cf", cmd)
                self.assertIn("--score_program=v2;m0-0;m1-0;max", cmd)
                self.assertIn("--score_coeffs_url=https://example.com/coeffs_3.bin", cmd)
                self.assertIn("--score_coeff_input_size=112", cmd)
                self.assertIn("--score_coeff_degree=7", cmd)
                scores = array("f", [1.0, 2.0, 3.0, 4.0])
                with open(mod._TMP_SCORES, "wb") as f:
                    scores.tofile(f)
                with open(mod._TMP_BINS, "wb") as f:
                    f.write(bytes([0, 1, 2, 3]))
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({
                        "min_score": 1.0,
                        "max_score": 4.0,
                        "threads": 4,
                        "input_mode": "sectioned",
                        "retries": 2,
                    }),
                    stderr="",
                )

            mock_run.side_effect = run_side_effect

            mod.handler(_event(
                metric="spread",
                solve_score_quantile=0.02,
                palette_chunk_input_mode="sectioned",
                bin_size=160,
                coeffs_key="renders/j/coeffs_0003.bin",
                coeffs_bin_size=112,
                n_coeffs=7,
            ), None)

            statuses = [c.args[2] for c in mock_report.call_args_list]
            self.assertEqual(statuses, ["started", "bin_downloaded", "computed", "done"])

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_v2_logical_section_rebuilds_spans_from_source_manifest(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod
        from logical_sections import build_solve_source_manifest

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT", os.path.join(td, "input.bin")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_SCORE_COEFFS", os.path.join(td, "coeffs.bin")):

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
                "program": "m0;m1;max",
                "metrics": [
                    {"slot": 0, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    {"slot": 1, "metric": "spread", "source": "cf", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                ],
            }

            def get_object(**kwargs):
                key = kwargs["Key"]
                rng = kwargs.get("Range")
                if key == "renders/j/palettes/p1/solve_score/crowding_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                expected_ranges = {
                    ("renders/j/chunk_0.bin", "bytes=0-47"): b"\x11" * 48,
                    ("renders/j/chunk_1.bin", "bytes=0-31"): b"\x22" * 32,
                    ("renders/j/coeffs_0000.bin", "bytes=0-71"): b"\x33" * 72,
                    ("renders/j/coeffs_0001.bin", "bytes=0-47"): b"\x44" * 48,
                }
                data = expected_ranges.get((key, rng))
                if data is None:
                    raise AssertionError(f"unexpected get_object key/range: {key} {rng}")
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=1024 * 1024, d=data: [d])}

            mock_s3.get_object.side_effect = get_object
            uploads = {}
            mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key, ExtraArgs=None: uploads.setdefault(key, fileobj.read())
            solve_source_manifest = build_solve_source_manifest(
                [
                    {
                        "chunk_idx": 0,
                        "bin_key": "renders/j/chunk_0.bin",
                        "coeffs_key": "renders/j/coeffs_0000.bin",
                        "step_start": 0,
                        "step_count": 3,
                    },
                    {
                        "chunk_idx": 1,
                        "bin_key": "renders/j/chunk_1.bin",
                        "coeffs_key": "renders/j/coeffs_0001.bin",
                        "step_start": 3,
                        "step_count": 3,
                    },
                ],
                job_id="j",
                degree=2,
                n_coeffs=3,
            )

            def run_side_effect(cmd, capture_output, text, timeout):
                self.assertEqual(cmd[0], mod.BINARY_MT)
                self.assertIn(f"--score_coeffs_file={mod._TMP_SCORE_COEFFS}", cmd)
                scores = array("f", [1.0, 2.0, 3.0, 4.0, 5.0])
                with open(mod._TMP_SCORES, "wb") as f:
                    scores.tofile(f)
                with open(mod._TMP_BINS, "wb") as f:
                    f.write(bytes([0, 1, 2, 3, 4]))
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({
                        "min_score": 1.0,
                        "max_score": 5.0,
                        "threads": 2,
                        "input_mode": "tmpfile",
                        "retries": 2,
                    }),
                    stderr="",
                )

            mock_run.side_effect = run_side_effect

            mod.handler(_event(
                metric="spread",
                solve_score_quantile=0.02,
                logical_section=True,
                solve_source_manifest=solve_source_manifest,
                step_start=0,
                step_count=5,
                degree=2,
                n_coeffs=3,
                palette_chunk_threads=2,
                bin_key="",
                coeffs_key="",
            ), None)

            statuses = [c.args[2] for c in mock_report.call_args_list]
            self.assertEqual(statuses, ["started", "bin_downloaded", "computed", "done"])
            computed_kwargs = mock_report.call_args_list[-2].kwargs
            done_kwargs = mock_report.call_args_list[-1].kwargs
            self.assertEqual(computed_kwargs["result_data"]["source_size"], 80)
            self.assertEqual(computed_kwargs["result_data"]["source_coeffs_size"], 120)
            self.assertEqual(done_kwargs["result_data"]["source_size"], 80)
            self.assertIn("renders/j/palettes/p1/chunks/score_chunk_3.bin", uploads)
            self.assertIn("renders/j/palettes/p1/chunks/palette_bins_chunk_3.bin", uploads)

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_v2_logical_section_sectioned_uses_multispan_manifests(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod
        from logical_sections import build_solve_source_manifest

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT_MANIFEST", os.path.join(td, "input_manifest.json")), \
             patch.object(mod, "_TMP_SCORE_COEFFS_MANIFEST", os.path.join(td, "coeff_manifest.json")), \
             patch.object(mod, "_TMP_SCORE_PARAMS_MANIFEST", os.path.join(td, "param_manifest.json")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")):

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
                "program": "m0;m1;m2;avg",
                "metrics": [
                    {"slot": 0, "metric": "spread", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    {"slot": 1, "metric": "spread", "source": "cf", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                    {"slot": 2, "metric": "t1_abs", "source": "pm", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                ],
            }
            solve_source_manifest = build_solve_source_manifest(
                [
                    {
                        "chunk_idx": 0,
                        "bin_key": "renders/j/chunk_0.bin",
                        "coeffs_key": "renders/j/coeffs_0000.bin",
                        "params_key": "renders/j/params_0000.bin",
                        "step_start": 0,
                        "step_count": 3,
                        "params_step_start": 0,
                        "params_step_count": 3,
                    },
                    {
                        "chunk_idx": 1,
                        "bin_key": "renders/j/chunk_1.bin",
                        "coeffs_key": "renders/j/coeffs_0001.bin",
                        "params_key": "renders/j/params_0001.bin",
                        "step_start": 3,
                        "step_count": 3,
                        "params_step_start": 0,
                        "params_step_count": 3,
                    },
                ],
                job_id="j",
                degree=2,
                n_coeffs=3,
            )

            def get_object(**kwargs):
                key = kwargs["Key"]
                if key == "renders/j/palettes/p1/solve_score/crowding_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                raise AssertionError(f"unexpected get_object key: {key}")

            mock_s3.get_object.side_effect = get_object
            mock_s3.generate_presigned_url.side_effect = lambda op, Params, ExpiresIn: f"https://example.com/{Params['Key']}"
            mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key, ExtraArgs=None: None

            def run_side_effect(cmd, capture_output, text, timeout):
                self.assertEqual(cmd[0], mod.BINARY_MT)
                self.assertIn("--input_mode=multispan_sectioned", cmd)
                input_manifest = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--input_manifest="))
                coeff_manifest = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--score_coeff_manifest="))
                param_manifest = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--score_params_manifest="))
                with open(input_manifest) as fh:
                    input_meta = json.load(fh)
                with open(coeff_manifest) as fh:
                    coeff_meta = json.load(fh)
                with open(param_manifest) as fh:
                    param_meta = json.load(fh)
                self.assertEqual(input_meta["source_family"], "slv")
                self.assertEqual(coeff_meta["source_family"], "cf")
                self.assertEqual(param_meta["source_family"], "pm")
                self.assertEqual(input_meta["logical_size"], 5 * 2 * 2 * 4)
                self.assertEqual(coeff_meta["logical_size"], 5 * 3 * 2 * 4)
                self.assertEqual(param_meta["logical_size"], 5 * 16)
                self.assertEqual(len(input_meta["spans"]), 2)
                self.assertEqual(len(coeff_meta["spans"]), 2)
                self.assertEqual(len(param_meta["spans"]), 2)
                scores = array("f", [1.0, 2.0, 3.0, 4.0, 5.0])
                with open(mod._TMP_SCORES, "wb") as f:
                    scores.tofile(f)
                with open(mod._TMP_BINS, "wb") as f:
                    f.write(bytes([0, 1, 2, 3, 4]))
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({
                        "min_score": 1.0,
                        "max_score": 5.0,
                        "threads": 2,
                        "input_mode": "multispan_sectioned",
                        "retries": 2,
                    }),
                    stderr="",
                )

            mock_run.side_effect = run_side_effect

            mod.handler(_event(
                section_idx=3,
                metric="spread",
                solve_score_quantile=0.02,
                logical_section=True,
                solve_source_manifest=solve_source_manifest,
                step_start=0,
                step_count=5,
                degree=2,
                n_coeffs=3,
                palette_chunk_threads=2,
                palette_chunk_input_mode="sectioned",
                bin_key="",
                coeffs_key="",
                params_key="",
            ), None)

            done_kwargs = mock_report.call_args_list[-1].kwargs
            self.assertEqual(done_kwargs["result_data"]["input_mode"], "multispan_sectioned")
            self.assertEqual(done_kwargs["result_data"]["logical_section"], True)

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_lagged_v2_logical_section_threads_prelude_to_native(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod
        from logical_sections import build_solve_source_manifest

        chain = [["proximity", "slv", "2"], ["proximity", "slv-1", "2"], ["abs_diff"]]
        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT_MANIFEST", os.path.join(td, "input_manifest.json")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")):

            bins_meta = {
                "family": "solve_score",
                "version": 2,
                "metric": "proximity",
                "clip_quantile": 0.02,
                "omega": 1.0,
                "omega_enabled": True,
                "clip_lo": 0.0,
                "clip_hi": 1.0,
                "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
                "program": "m0-0;m0-1;abs_diff",
                "chain_fingerprint": _fingerprint(chain, metric="proximity", quantile=0.02),
                "metrics": [
                    {"slot": 0, "metric": "proximity", "source": "slv", "quantile": 0.02, "clip_lo": 0.0, "clip_hi": 1.0},
                ],
            }
            solve_source_manifest = build_solve_source_manifest(
                [
                    {"chunk_idx": 0, "bin_key": "renders/j/chunk_0.bin", "step_start": 0, "step_count": 3},
                    {"chunk_idx": 1, "bin_key": "renders/j/chunk_1.bin", "step_start": 3, "step_count": 3},
                ],
                job_id="j",
                degree=2,
                n_coeffs=3,
            )

            mock_s3.get_object.side_effect = lambda **kwargs: {
                "Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())
            }
            mock_s3.generate_presigned_url.side_effect = lambda op, Params, ExpiresIn: f"https://example.com/{Params['Key']}"
            mock_s3.upload_fileobj.side_effect = lambda fileobj, bucket, key, ExtraArgs=None: None

            def run_side_effect(cmd, capture_output, text, timeout):
                self.assertEqual(cmd[0], mod.BINARY_MT)
                self.assertIn("--input_mode=multispan_sectioned", cmd)
                self.assertIn("--prelude_rows=1", cmd)
                self.assertIn("--score_coeff_prelude_rows=0", cmd)
                self.assertIn("--score_param_prelude_rows=0", cmd)
                self.assertIn("--score_program=v2;m0-0;m0-1;abs_diff", cmd)
                input_manifest = next(arg.split("=", 1)[1] for arg in cmd if arg.startswith("--input_manifest="))
                with open(input_manifest) as fh:
                    input_meta = json.load(fh)
                self.assertEqual(input_meta["source_family"], "slv")
                self.assertEqual(input_meta["logical_size"], 3 * 2 * 2 * 4)
                self.assertEqual(len(input_meta["spans"]), 2)
                scores = array("f", [0.0, 0.5])
                with open(mod._TMP_SCORES, "wb") as f:
                    scores.tofile(f)
                with open(mod._TMP_BINS, "wb") as f:
                    f.write(bytes([0, 5]))
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({
                        "min_score": 0.0,
                        "max_score": 0.5,
                        "threads": 2,
                        "input_mode": "multispan_sectioned",
                        "retries": 2,
                    }),
                    stderr="",
                )

            mock_run.side_effect = run_side_effect
            mod.handler(_event(
                section_idx=1,
                metric="proximity",
                solve_score_quantile=0.02,
                solve_score_chain=chain,
                logical_section=True,
                solve_source_manifest=solve_source_manifest,
                prelude_rows=1,
                step_start=3,
                step_count=2,
                degree=2,
                palette_chunk_threads=2,
                palette_chunk_input_mode="sectioned",
                bin_key="",
            ), None)

            done_kwargs = mock_report.call_args_list[-1].kwargs
            self.assertEqual(done_kwargs["result_data"]["input_mode"], "multispan_sectioned")
            self.assertEqual(done_kwargs["result_data"]["prelude_rows"], 1)

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_v2_logical_section_rejects_malformed_solve_source_manifest(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod

        with self.assertRaises(RuntimeError) as ctx:
            mod.handler(_event(
                section_idx=3,
                metric="spread",
                solve_score_quantile=0.02,
                logical_section=True,
                solve_source_manifest={"version": 1, "job_id": "j", "sources": {}},
                step_start=0,
                step_count=5,
                degree=2,
                n_coeffs=3,
                palette_chunk_threads=2,
                palette_chunk_input_mode="sectioned",
                bin_key="",
                coeffs_key="",
                params_key="",
            ), None)
        self.assertIn("missing row_bytes for source family slv", str(ctx.exception))
        mock_run.assert_not_called()

    @patch("handler_palette_chunk.report_status")
    @patch("handler_palette_chunk.s3")
    @patch("handler_palette_chunk.subprocess.run")
    def test_v2_param_source_sectioned_bins_pass_params_file_cli_flags(self, mock_run, mock_s3, mock_report):
        import handler_palette_chunk as mod

        with tempfile.TemporaryDirectory() as td, \
             patch.object(mod, "_TMP_INPUT", os.path.join(td, "input.bin")), \
             patch.object(mod, "_TMP_SCORES", os.path.join(td, "scores.bin")), \
             patch.object(mod, "_TMP_BINS", os.path.join(td, "bins.bin")), \
             patch.object(mod, "_TMP_XFORMS", os.path.join(td, "xforms.json")), \
             patch.object(mod, "_TMP_SCORE_PARAMS", os.path.join(td, "params.bin")), \
             patch.object(mod, "_sectioned_input_size_limit", return_value=10_000_000):

            bins_meta = {
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

            def get_object(**kwargs):
                key = kwargs["Key"]
                if key == "renders/j/palettes/p1/solve_score/param_bins.json":
                    return {"Body": MagicMock(read=lambda: json.dumps(bins_meta).encode())}
                if key == "renders/j/params.bin":
                    self.assertEqual(kwargs["Range"], "bytes=160-223")
                    return {"Body": MagicMock(iter_chunks=lambda chunk_size=1024 * 1024: [b"\x33" * 64])}
                raise AssertionError(f"unexpected get_object key: {key}")

            mock_s3.get_object.side_effect = get_object
            mock_s3.generate_presigned_url.side_effect = [
                "https://example.com/chunk_3.bin",
            ]
            mock_s3.upload_fileobj.side_effect = lambda *args, **kwargs: None

            def run_side_effect(cmd, capture_output, text, timeout):
                self.assertEqual(cmd[0], mod.BINARY_MT)
                self.assertIn("--input_mode=sectioned", cmd)
                self.assertIn("--url=https://example.com/chunk_3.bin", cmd)
                self.assertIn("--score_sources=pm,slv", cmd)
                self.assertIn("--score_program=v2;m0-0;m1-0;max", cmd)
                self.assertIn(f"--score_params_file={mod._TMP_SCORE_PARAMS}", cmd)
                scores = array("f", [1.0, 2.0, 3.0, 4.0])
                with open(mod._TMP_SCORES, "wb") as f:
                    scores.tofile(f)
                with open(mod._TMP_BINS, "wb") as f:
                    f.write(bytes([0, 1, 2, 3]))
                return MagicMock(
                    returncode=0,
                    stdout=json.dumps({
                        "min_score": 1.0,
                        "max_score": 4.0,
                        "threads": 4,
                        "input_mode": "sectioned",
                        "retries": 2,
                    }),
                    stderr="",
                )

            mock_run.side_effect = run_side_effect

            mod.handler(_event(
                metric="t1_abs",
                solve_score_quantile=0.02,
                solve_score_bins_key="renders/j/palettes/p1/solve_score/param_bins.json",
                palette_chunk_input_mode="sectioned",
                bin_size=160,
                params_key="renders/j/params.bin",
                step_start=10,
                step_count=4,
            ), None)

            statuses = [c.args[2] for c in mock_report.call_args_list]
            self.assertEqual(statuses, ["started", "bin_downloaded", "computed", "done"])


if __name__ == "__main__":
    unittest.main()
