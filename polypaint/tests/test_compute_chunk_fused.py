import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestComputeChunkFused(unittest.TestCase):

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
                    "roots_size", "lambda_memory_mb", "arch"):
            self.assertIn(key, telemetry)
        self.assertEqual(telemetry["roots_size"], 560)
        self.assertEqual(mock_upload.call_count, 3)
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
        self.assertEqual(mock_upload.call_count, 1)

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
        self.assertGreater(body["upload_params_us"], 0)
        self.assertEqual(body["upload_coeffs_us"], 0)
        self.assertEqual(mock_param.call_count, 1)
        self.assertEqual(mock_coeff.call_count, 0)
        self.assertEqual(mock_download.call_count, 1)
        self.assertEqual(mock_upload.call_count, 2)
        self.assertEqual(mock_download.call_args.args[0], "renders/compute_j/coeffs_0001.bin")

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
