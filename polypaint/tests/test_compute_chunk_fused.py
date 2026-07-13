import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _FakeStreamer:
    """Stands in for RootsStreamUploader: records lifecycle calls and lets
    tests force the fallback path."""
    instances = []
    finish_ok = True

    def __init__(self, s3_client, bucket, key, data_path, progress_path,
                 total_bytes, **kwargs):
        self.key = key
        self.data_path = data_path
        self.progress_path = progress_path
        self.total_bytes = total_bytes
        self.started = False
        self.finished = False
        self.aborted = False
        self.parts_during_solve = 3
        self.parts_reverified = 3
        self.parts_repaired = 0
        self.span_us = 4200
        self.fail_reason = None
        _FakeStreamer.instances.append(self)

    def start(self):
        self.started = True

    def finish(self):
        self.finished = True
        if not _FakeStreamer.finish_ok:
            self.fail_reason = "FakeError: injected stream failure"
        return _FakeStreamer.finish_ok

    def abort(self):
        self.aborted = True

    @classmethod
    def reset(cls, *, finish_ok=True):
        cls.instances = []
        cls.finish_ok = finish_ok


class TestComputeChunkFused(unittest.TestCase):

    def setUp(self):
        _FakeStreamer.reset()

    @patch("handler_compute_chunk_fused.RootsStreamUploader", _FakeStreamer)
    @patch("handler_compute_chunk_fused.os.path.getsize")
    @patch("handler_compute_chunk_fused.report_status")
    @patch("handler_compute_chunk_fused._upload_file")
    @patch("handler_compute_chunk_fused._s3_size_matches")
    @patch("handler_compute_chunk_fused._run_solve_local")
    @patch("handler_compute_chunk_fused._run_coeffgen_local")
    @patch("handler_compute_chunk_fused._run_param_gen_local")
    def test_fused_chunk_generates_all_stages(self, mock_param, mock_coeff, mock_solve, mock_match, mock_upload, mock_report, mock_getsize):
        import handler_compute_chunk_fused as mod

        mock_match.return_value = False
        mock_param.return_value = {"elapsed_us": 111, "threads": 4}
        mock_coeff.return_value = {"threads": 4}
        mock_solve.return_value = {"n_t": 10, "degree": 7, "avg_iterations": 3.5}
        mock_getsize.side_effect = [160, 560, 560]

        result = mod.handle_fused_chunk({
            "job_id": "compute_j",
            "chunk_idx": 2,
            "step_start": 20,
            "step_count": 10,
            "N": 100,
            "times": 1,
            "n_coeffs": 7,
            "degree": 7,
            "fused_threads": 4,
            "solver_mode": "aberth_mt",
            "task_id": "compute_run_fused_2",
            "function": "g1",
            "param_transforms": [],
            "coeff_transforms": [],
            "cfpv": [],
            "params_key": "renders/compute_j/params_0002.bin",
            "coeffs_key": "renders/compute_j/coeffs_0002.bin",
            "bin_key": "renders/compute_j/chunk_2.bin",
        })
        body = json.loads(result["body"])
        self.assertEqual(body["chunk_idx"], 2)
        self.assertEqual(body["params_size"], 160)
        self.assertEqual(body["params_key"], "renders/compute_j/params_0002.bin")
        self.assertEqual(body["params_step_start"], 0)
        self.assertEqual(body["params_step_count"], 10)
        self.assertEqual(body["coeffs_size"], 560)
        self.assertEqual(body["bin_size"], 560)
        self.assertEqual(body["param_gen_us"], 111)
        self.assertEqual(body["fused_threads"], 4)
        self.assertEqual(body["execution_method"], "fused_chunk_pipeline")
        # CR33 telemetry contract: one structured stage summary per chunk
        telemetry = body["stage_telemetry"]
        for key in ("param_scheduler", "param_native_us", "param_tokens",
                    "param_legacy_static", "param_legacy_dynamic",
                    "param_legacy_prepared", "online_cpus",
                    "coeff_native_us", "coeff_tokens",
                    "coeff_tok_typed_scalar", "coeff_tok_typed_vector",
                    "coeff_tok_selector", "coeff_tok_native",
                    "coeff_fused_regions", "coeff_fused_tokens",
                    "roots_size", "solve_native_us", "handler_wall_us",
                    "lambda_memory_mb", "arch"):
            self.assertIn(key, telemetry)
        self.assertEqual(telemetry["roots_size"], 560)
        # CR34 §12-1 contract: params+coeffs go through the background
        # executor (still _upload_file underneath); roots go through the
        # streaming uploader, so _upload_file sees exactly 2 calls.
        self.assertEqual(mock_upload.call_count, 2)
        streamer = _FakeStreamer.instances[-1]
        self.assertTrue(streamer.started)
        self.assertTrue(streamer.finished)
        self.assertFalse(streamer.aborted)
        self.assertEqual(streamer.key, "renders/compute_j/chunk_2.bin")
        self.assertEqual(streamer.total_bytes, 10 * 7 * 8)
        self.assertEqual(body["roots_upload_fallback"], 0)
        self.assertEqual(body["roots_parts_during_solve"], 3)
        self.assertEqual(body["roots_parts_reverified"], 3)
        self.assertEqual(body["roots_parts_repaired"], 0)
        self.assertEqual(body["upload_roots_span_us"], 4200)
        self.assertNotIn("upload_roots_us", body)
        self.assertNotIn("roots_stream_fail_reason", body)
        # review F1: scratch paths are invocation-unique (stale /tmp from an
        # abnormally killed invocation can never alias a retry's files)
        self.assertTrue(streamer.data_path.startswith("/tmp/fused_roots_2_"),
                        streamer.data_path)
        self.assertEqual(streamer.progress_path, streamer.data_path + ".progress")
        self.assertGreaterEqual(body["upload_roots_tail_us"], 0)
        self.assertGreaterEqual(body["pre_solve_upload_wait_us"], 0)
        self.assertGreaterEqual(body["upload_params_us"], 0)
        self.assertGreaterEqual(body["upload_coeffs_us"], 0)
        # the solver was pointed at the progress sidecar (aberth_mt)
        self.assertEqual(mock_solve.call_args.kwargs["progress_path"],
                         streamer.progress_path)
        self.assertEqual(mock_report.call_args_list[-1].args[:3], ("compute_j", "compute_run_fused_2", "done"))
        self.assertEqual(mock_param.call_args.kwargs["fused_threads"], 4)
        self.assertEqual(mock_match.call_args_list[0].kwargs["expected_metadata"], {
            "pp-stage": "params",
            "pp-step-start": "20",
            "pp-step-count": "10",
        })
        self.assertEqual(mock_match.call_args_list[1].kwargs["expected_metadata"], {
            "pp-stage": "coeffs",
            "pp-step-start": "20",
            "pp-step-count": "10",
            "pp-n-coeffs": "7",
            "pp-degree": "7",
        })
        self.assertEqual(mock_upload.call_args_list[0].kwargs["metadata"], {
            "pp-stage": "params",
            "pp-step-start": "20",
            "pp-step-count": "10",
        })
        self.assertEqual(mock_upload.call_args_list[1].kwargs["metadata"], {
            "pp-stage": "coeffs",
            "pp-step-start": "20",
            "pp-step-count": "10",
            "pp-n-coeffs": "7",
            "pp-degree": "7",
        })
        self.assertEqual(mock_coeff.call_args.kwargs["n"], 100)
        self.assertEqual(mock_coeff.call_args.kwargs["source_step_start"], 20)

    @patch("handler_compute_chunk_fused.RootsStreamUploader", _FakeStreamer)
    @patch("handler_compute_chunk_fused.os.path.getsize")
    @patch("handler_compute_chunk_fused.report_status")
    @patch("handler_compute_chunk_fused._download_file")
    @patch("handler_compute_chunk_fused._upload_file")
    @patch("handler_compute_chunk_fused._s3_size_matches")
    @patch("handler_compute_chunk_fused._run_solve_local")
    @patch("handler_compute_chunk_fused._run_coeffgen_local")
    @patch("handler_compute_chunk_fused._run_param_gen_local")
    def test_fused_chunk_reuses_params_and_coeffs(self, mock_param, mock_coeff, mock_solve, mock_match, mock_upload, mock_download, mock_report, mock_getsize):
        import handler_compute_chunk_fused as mod

        mock_match.side_effect = [True, True]
        mock_solve.return_value = {"n_t": 10, "degree": 7, "avg_iterations": 1.5}
        mock_getsize.side_effect = [160, 560, 560]

        result = mod.handle_fused_chunk({
            "job_id": "compute_j",
            "chunk_idx": 0,
            "step_start": 0,
            "step_count": 10,
            "N": 100,
            "times": 1,
            "n_coeffs": 7,
            "degree": 7,
            "fused_threads": 4,
            "solver_mode": "aberth_mt",
            "task_id": "compute_run_fused_0",
            "function": "g1",
            "param_transforms": [],
            "coeff_transforms": [],
            "cfpv": [],
            "params_key": "renders/compute_j/params_0000.bin",
            "coeffs_key": "renders/compute_j/coeffs_0000.bin",
            "bin_key": "renders/compute_j/chunk_0.bin",
        })
        body = json.loads(result["body"])
        self.assertEqual(body["reused_params"], 1)
        self.assertEqual(body["reused_coeffs"], 1)
        self.assertEqual(body["upload_params_us"], 0)
        self.assertEqual(body["upload_coeffs_us"], 0)
        self.assertEqual(mock_param.call_count, 0)
        self.assertEqual(mock_coeff.call_count, 0)
        self.assertEqual(mock_download.call_count, 2)
        # nothing to background-upload; roots go through the streamer
        self.assertEqual(mock_upload.call_count, 0)
        self.assertTrue(_FakeStreamer.instances[-1].finished)
        self.assertEqual(body["roots_upload_fallback"], 0)

    @patch("handler_compute_chunk_fused.RootsStreamUploader", _FakeStreamer)
    @patch("handler_compute_chunk_fused.os.path.getsize")
    @patch("handler_compute_chunk_fused.report_status")
    @patch("handler_compute_chunk_fused._download_file")
    @patch("handler_compute_chunk_fused._upload_file")
    @patch("handler_compute_chunk_fused._s3_size_matches")
    @patch("handler_compute_chunk_fused._run_solve_local")
    @patch("handler_compute_chunk_fused._run_coeffgen_local")
    @patch("handler_compute_chunk_fused._run_param_gen_local")
    def test_fused_chunk_regenerates_params_but_reuses_coeffs(self, mock_param, mock_coeff, mock_solve, mock_match, mock_upload, mock_download, mock_report, mock_getsize):
        import handler_compute_chunk_fused as mod

        mock_match.side_effect = [False, True]
        mock_param.return_value = {"elapsed_us": 222, "threads": 4}
        mock_solve.return_value = {"n_t": 10, "degree": 7, "avg_iterations": 1.5}
        mock_getsize.side_effect = [160, 560, 560]

        result = mod.handle_fused_chunk({
            "job_id": "compute_j",
            "chunk_idx": 1,
            "step_start": 10,
            "step_count": 10,
            "N": 100,
            "times": 1,
            "n_coeffs": 7,
            "degree": 7,
            "fused_threads": 4,
            "solver_mode": "aberth_mt",
            "task_id": "compute_run_fused_1",
            "function": "g1",
            "param_transforms": [],
            "coeff_transforms": [],
            "cfpv": [],
            "params_key": "renders/compute_j/params_0001.bin",
            "coeffs_key": "renders/compute_j/coeffs_0001.bin",
            "bin_key": "renders/compute_j/chunk_1.bin",
        })

        body = json.loads(result["body"])
        self.assertEqual(body["reused_params"], 0)
        self.assertEqual(body["reused_coeffs"], 1)
        self.assertGreaterEqual(body["upload_params_us"], 0)
        self.assertEqual(body["upload_coeffs_us"], 0)
        self.assertEqual(mock_param.call_count, 1)
        self.assertEqual(mock_coeff.call_count, 0)
        self.assertEqual(mock_download.call_count, 1)
        # only the regenerated params background-upload; roots stream
        self.assertEqual(mock_upload.call_count, 1)
        self.assertEqual(mock_download.call_args.args[0], "renders/compute_j/coeffs_0001.bin")

    @patch("handler_compute_chunk_fused.RootsStreamUploader", _FakeStreamer)
    @patch("handler_compute_chunk_fused.os.path.getsize")
    @patch("handler_compute_chunk_fused.report_status")
    @patch("handler_compute_chunk_fused._upload_file")
    @patch("handler_compute_chunk_fused._s3_size_matches")
    @patch("handler_compute_chunk_fused._run_solve_local")
    @patch("handler_compute_chunk_fused._run_coeffgen_local")
    @patch("handler_compute_chunk_fused._run_param_gen_local")
    def test_fused_chunk_paths_unique_per_invocation_and_stale_tmp_swept(self, mock_param, mock_coeff, mock_solve, mock_match, mock_upload, mock_report, mock_getsize):
        """Review F1 regression: two invocations of the SAME chunk must use
        different scratch paths, and fused_* leftovers from an abnormally
        killed invocation are swept at entry — a stale same-sized roots
        file/sidecar could otherwise feed the streaming poller wrong bytes
        before the solver truncates them."""
        import handler_compute_chunk_fused as mod

        stale = "/tmp/fused_roots_9_deadbeef.bin.progress"
        with open(stale, "wb") as fh:
            fh.write(b"PPR1stale")
        try:
            mock_match.return_value = False
            mock_param.return_value = {"elapsed_us": 111, "threads": 4}
            mock_coeff.return_value = {"threads": 4}
            mock_solve.return_value = {"n_t": 10, "degree": 7, "avg_iterations": 3.5}
            mock_getsize.side_effect = [160, 560, 560, 160, 560, 560]

            mod.handle_fused_chunk(self._base_params(chunk_idx=3))
            self.assertFalse(os.path.exists(stale), "stale sidecar not swept")
            mod.handle_fused_chunk(self._base_params(chunk_idx=3))
            first, second = _FakeStreamer.instances[-2:]
            self.assertNotEqual(first.data_path, second.data_path)
            self.assertTrue(first.data_path.startswith("/tmp/fused_roots_3_"))
        finally:
            try:
                os.remove(stale)
            except OSError:
                pass

    @patch("handler_compute_chunk_fused.RootsStreamUploader", _FakeStreamer)
    @patch("handler_compute_chunk_fused.os.path.getsize")
    @patch("handler_compute_chunk_fused.report_status")
    @patch("handler_compute_chunk_fused._upload_file")
    @patch("handler_compute_chunk_fused._s3_size_matches")
    @patch("handler_compute_chunk_fused._run_solve_local")
    @patch("handler_compute_chunk_fused._run_coeffgen_local")
    @patch("handler_compute_chunk_fused._run_param_gen_local")
    def test_fused_chunk_stream_failure_falls_back_to_serial_put(self, mock_param, mock_coeff, mock_solve, mock_match, mock_upload, mock_report, mock_getsize):
        import handler_compute_chunk_fused as mod

        _FakeStreamer.reset(finish_ok=False)
        mock_match.return_value = False
        mock_param.return_value = {"elapsed_us": 111, "threads": 4}
        mock_coeff.return_value = {"threads": 4}
        mock_solve.return_value = {"n_t": 10, "degree": 7, "avg_iterations": 3.5}
        mock_getsize.side_effect = [160, 560, 560]

        result = mod.handle_fused_chunk(self._base_params(chunk_idx=5))
        body = json.loads(result["body"])
        self.assertEqual(body["roots_upload_fallback"], 1)
        self.assertIn("upload_roots_us", body)
        # review F5: one field never carries two meanings — the multipart
        # span is omitted on fallback, and the failure reason is surfaced
        self.assertNotIn("upload_roots_span_us", body)
        self.assertEqual(body["roots_stream_fail_reason"],
                         "FakeError: injected stream failure")
        # params + coeffs backgrounded + the serial roots fallback PUT
        # (background calls record from worker threads, so order is not
        # deterministic — assert presence, not position)
        self.assertEqual(mock_upload.call_count, 3)
        uploaded_keys = {c.args[1] for c in mock_upload.call_args_list}
        self.assertIn("renders/compute_j/chunk_5.bin", uploaded_keys)

    @patch("handler_compute_chunk_fused.RootsStreamUploader", _FakeStreamer)
    @patch("handler_compute_chunk_fused.os.path.getsize")
    @patch("handler_compute_chunk_fused.report_status")
    @patch("handler_compute_chunk_fused._upload_file")
    @patch("handler_compute_chunk_fused._s3_size_matches")
    @patch("handler_compute_chunk_fused._run_solve_local")
    @patch("handler_compute_chunk_fused._run_coeffgen_local")
    @patch("handler_compute_chunk_fused._run_param_gen_local")
    def test_fused_chunk_solve_failure_aborts_multipart(self, mock_param, mock_coeff, mock_solve, mock_match, mock_upload, mock_report, mock_getsize):
        import handler_compute_chunk_fused as mod

        mock_match.return_value = False
        mock_param.return_value = {"elapsed_us": 111, "threads": 4}
        mock_coeff.return_value = {"threads": 4}
        mock_solve.side_effect = RuntimeError("fused solve failed: boom")
        mock_getsize.side_effect = [160, 560]

        with self.assertRaises(RuntimeError):
            mod.handle_fused_chunk(self._base_params(chunk_idx=6))
        streamer = _FakeStreamer.instances[-1]
        self.assertTrue(streamer.started)
        self.assertTrue(streamer.aborted)
        self.assertFalse(streamer.finished)
        self.assertEqual(mock_report.call_args_list[-1].args[2], "error")

    @patch("handler_compute_chunk_fused.RootsStreamUploader", _FakeStreamer)
    @patch("handler_compute_chunk_fused.os.path.getsize")
    @patch("handler_compute_chunk_fused.report_status")
    @patch("handler_compute_chunk_fused._upload_file")
    @patch("handler_compute_chunk_fused._s3_size_matches")
    @patch("handler_compute_chunk_fused._run_solve_local")
    @patch("handler_compute_chunk_fused._run_coeffgen_local")
    @patch("handler_compute_chunk_fused._run_param_gen_local")
    def test_fused_chunk_background_upload_failure_fails_the_task(self, mock_param, mock_coeff, mock_solve, mock_match, mock_upload, mock_report, mock_getsize):
        import handler_compute_chunk_fused as mod

        mock_match.return_value = False
        mock_param.return_value = {"elapsed_us": 111, "threads": 4}
        mock_coeff.return_value = {"threads": 4}
        mock_solve.return_value = {"n_t": 10, "degree": 7, "avg_iterations": 3.5}
        mock_getsize.side_effect = [160, 560, 560]
        mock_upload.side_effect = RuntimeError("params PUT failed")

        with self.assertRaises(RuntimeError) as ctx:
            mod.handle_fused_chunk(self._base_params(chunk_idx=7))
        self.assertIn("params PUT failed", str(ctx.exception))
        self.assertEqual(mock_report.call_args_list[-1].args[2], "error")

    def _base_params(self, *, chunk_idx):
        return {
            "job_id": "compute_j",
            "chunk_idx": chunk_idx,
            "step_start": 0,
            "step_count": 10,
            "N": 100,
            "times": 1,
            "n_coeffs": 7,
            "degree": 7,
            "fused_threads": 4,
            "solver_mode": "aberth_mt",
            "task_id": f"compute_run_fused_{chunk_idx}",
            "function": "g1",
            "param_transforms": [],
            "coeff_transforms": [],
            "cfpv": [],
            "params_key": f"renders/compute_j/params_{chunk_idx:04d}.bin",
            "coeffs_key": f"renders/compute_j/coeffs_{chunk_idx:04d}.bin",
            "bin_key": f"renders/compute_j/chunk_{chunk_idx}.bin",
        }

    @patch("handler_compute_chunk_fused.time.time", side_effect=[100.0, 100.25])
    @patch("handler_compute_chunk_fused.subprocess.run")
    def test_run_param_gen_local_forwards_fused_threads(self, mock_run, mock_time):
        import handler_compute_chunk_fused as mod

        proc = MagicMock()
        proc.returncode = 0
        proc.stdout = '{"n_threads":6}'
        proc.stderr = ""
        mock_run.return_value = proc

        result = mod._run_param_gen_local(
            output_path="/tmp/fused_params_test.bin",
            n=64,
            times=2,
            step_start=10,
            step_count=20,
            param_transforms=[["roots2"]],
            fused_threads=6,
        )

        # CR33 F1 invocation contract: the REAL output path is argv[1] — the
        # "-" stream argument selected the ordered-ring scheduler and bypassed
        # the static pwrite scheduler for every multithreaded fused chunk.
        argv = mock_run.call_args.args[0]
        self.assertEqual(argv[1], "/tmp/fused_params_test.bin")
        self.assertNotIn("-", argv)
        spec = json.loads(mock_run.call_args.kwargs["input"])
        self.assertEqual(spec["n_threads"], 6)
        self.assertEqual(spec["step_start"], 10)
        self.assertEqual(spec["step_count"], 20)
        # file mode: metadata decoded from stdout
        self.assertEqual(result["n_threads"], 6)
        self.assertEqual(result["param_scheduler"], "static_file")
        self.assertEqual(result["elapsed_us"], 250000)

    @patch("handler_compute_chunk_fused.subprocess.run")
    def test_run_param_gen_local_failure_uses_stderr(self, mock_run):
        import handler_compute_chunk_fused as mod

        proc = MagicMock()
        proc.returncode = 1
        proc.stdout = ""
        proc.stderr = "param_gen static write failed"
        mock_run.return_value = proc

        with self.assertRaises(RuntimeError) as ctx:
            mod._run_param_gen_local(
                output_path="/tmp/fused_params_test.bin",
                n=8, times=1, step_start=0, step_count=64,
                param_transforms=[], fused_threads=4,
            )
        self.assertIn("param_gen static write failed", str(ctx.exception))

    @patch("handler_compute_chunk_fused.s3")
    def test_s3_size_matches_checks_metadata(self, mock_s3):
        import handler_compute_chunk_fused as mod

        mock_s3.head_object.return_value = {
            "ContentLength": 160,
            "Metadata": {
                "pp-stage": "params",
                "pp-step-start": "0",
                "pp-step-count": "10",
            },
        }
        self.assertTrue(mod._s3_size_matches(
            "renders/compute_j/params_0000.bin",
            160,
            expected_metadata={
                "pp-stage": "params",
                "pp-step-start": "0",
                "pp-step-count": "10",
            },
        ))
        self.assertFalse(mod._s3_size_matches(
            "renders/compute_j/params_0000.bin",
            160,
            expected_metadata={
                "pp-stage": "params",
                "pp-step-start": "5",
                "pp-step-count": "10",
            },
        ))

    def test_fused_chunk_requires_non_empty_params_key(self):
        import handler_compute_chunk_fused as mod

        with self.assertRaises(RuntimeError) as ctx:
            mod.handle_fused_chunk({
                "job_id": "compute_j",
                "chunk_idx": 0,
                "step_start": 0,
                "step_count": 10,
                "N": 100,
                "times": 1,
                "n_coeffs": 7,
                "degree": 7,
                "fused_threads": 4,
                "solver_mode": "aberth_mt",
                "task_id": "compute_run_fused_0",
                "function": "g1",
                "param_transforms": [],
                "coeff_transforms": [],
                "cfpv": [],
                "params_key": "",
                "coeffs_key": "renders/compute_j/coeffs_0000.bin",
                "bin_key": "renders/compute_j/chunk_0.bin",
            })
        self.assertIn("requires non-empty params_key", str(ctx.exception))

    def test_fused_chunk_requires_integer_step_count(self):
        import handler_compute_chunk_fused as mod

        with self.assertRaises(RuntimeError) as ctx:
            mod.handle_fused_chunk({
                "job_id": "compute_j",
                "chunk_idx": 0,
                "step_start": 0,
                "step_count": "ten",
                "N": 100,
                "times": 1,
                "n_coeffs": 7,
                "degree": 7,
                "fused_threads": 4,
                "solver_mode": "aberth_mt",
                "task_id": "compute_run_fused_0",
                "function": "g1",
                "param_transforms": [],
                "coeff_transforms": [],
                "cfpv": [],
                "params_key": "renders/compute_j/params_0000.bin",
                "coeffs_key": "renders/compute_j/coeffs_0000.bin",
                "bin_key": "renders/compute_j/chunk_0.bin",
            })
        self.assertIn("requires integer step_count", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
