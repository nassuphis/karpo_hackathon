import io
import json
import os
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _DummyStdin:
    def __init__(self):
        self.data = b""

    def write(self, chunk):
        self.data += chunk
        return len(chunk)

    def close(self):
        return None


class _DummyProc:
    def __init__(self, stdout_bytes, stderr_text):
        self.stdin = _DummyStdin()
        self.stdout = io.BytesIO(stdout_bytes)
        self.stderr = io.BytesIO(stderr_text.encode("utf-8"))
        self.returncode = 0

    def wait(self, timeout=None):
        return 0


class _DummyCompleted:
    def __init__(self, stdout_text, stderr_text="", returncode=0):
        self.stdout = stdout_text
        self.stderr = stderr_text
        self.returncode = returncode


class _NoCloseBytesIO(io.BytesIO):
    def close(self):
        self.seek(0)


class TestCoeffgenParamGenHandler(unittest.TestCase):
    @patch("handler_coeffgen.report_status")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess.Popen")
    def test_param_gen_forwards_n_threads_and_reports_threads(self, mock_popen, mock_s3, mock_report):
        import handler_coeffgen as mod

        proc = _DummyProc(
            b"\x00" * 64,
            json.dumps({
                "mode": "param_gen",
                "n1": 4,
                "n2": 4,
                "times": 1,
                "n_steps": 4,
                "total_steps": 16,
                "step_start": 4,
                "step_count": 4,
                "data_bytes": 64,
                "threads": 6,
                "elapsed_us": 1234,
            }),
        )
        mock_popen.return_value = proc
        mock_s3.create_multipart_upload.return_value = {"UploadId": "u1"}
        mock_s3.upload_part.return_value = {"ETag": "etag-1"}

        result = mod.handle_param_gen({
            "job_id": "compute_j",
            "task_id": "compute_run_mt_param_gen",
            "N": 4,
            "times": 1,
            "param_transforms": [["unit_circle"]],
            "n_threads": 6,
            "params_key": "renders/compute_j/params.bin",
            "step_start": 4,
            "step_count": 4,
        })

        body = json.loads(result["body"])
        self.assertEqual(body["threads"], 6)
        self.assertEqual(body["params_key"], "renders/compute_j/params.bin")

        spec = json.loads(proc.stdin.data.decode("utf-8"))
        self.assertEqual(spec["mode"], "param_gen")
        self.assertEqual(spec["n_threads"], 6)
        self.assertEqual(spec["param_transforms"], [["unit_circle"]])
        self.assertEqual(spec["step_start"], 4)
        self.assertEqual(spec["step_count"], 4)

        started_call = mock_report.call_args_list[0]
        self.assertEqual(started_call.args[:3], ("compute_j", "compute_run_mt_param_gen", "started"))
        self.assertEqual(started_call.kwargs["result_data"]["threads"], 6)
        self.assertEqual(started_call.kwargs["result_data"]["n_steps"], 4)
        self.assertEqual(started_call.kwargs["result_data"]["data_bytes"], 64)

        done_call = mock_report.call_args_list[-1]
        self.assertEqual(done_call.args[:3], ("compute_j", "compute_run_mt_param_gen", "done"))
        self.assertEqual(done_call.kwargs["result_data"]["threads"], 6)

    @patch("handler_coeffgen.report_status")
    @patch("handler_coeffgen.s3")
    @patch("handler_coeffgen.subprocess.Popen")
    @patch("handler_coeffgen.time.time")
    def test_param_gen_reports_periodic_progress(self, mock_time, mock_popen, mock_s3, mock_report):
        import handler_coeffgen as mod

        proc = _DummyProc(
            b"\x00" * (9 * 1024 * 1024),
            json.dumps({
                "mode": "param_gen",
                "n1": 768,
                "n2": 768,
                "times": 1,
                "n_steps": 589824,
                "data_bytes": 9 * 1024 * 1024,
                "threads": 4,
                "elapsed_us": 4000000,
            }),
        )
        mock_popen.return_value = proc
        mock_s3.create_multipart_upload.return_value = {"UploadId": "u1"}
        mock_s3.upload_part.return_value = {"ETag": "etag-1"}

        times = iter([1000.0, 1003.0, 1003.0, 1004.0, 1006.0])
        mock_time.side_effect = lambda: next(times)

        mod.handle_param_gen({
            "job_id": "compute_j",
            "task_id": "compute_run_mt_param_gen",
            "N": 768,
            "times": 1,
            "param_transforms": [],
            "n_threads": 4,
            "params_key": "renders/compute_j/params.bin",
        })

        progress_calls = [
            call for call in mock_report.call_args_list
            if call.args[:3] == ("compute_j", "compute_run_mt_param_gen", "started")
            and int(call.kwargs["result_data"].get("uploaded_bytes", 0) or 0) > 0
        ]
        self.assertGreaterEqual(len(progress_calls), 1)
        progress_data = progress_calls[0].kwargs["result_data"]
        self.assertEqual(progress_data["threads"], 4)
        self.assertGreater(progress_data["uploaded_bytes"], 0)
        self.assertGreater(progress_data["uploaded_steps_est"], 0)
        self.assertGreater(progress_data["progress"], 0.0)

    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.os.path.getsize")
    @patch("handler_coeffgen.open", create=True)
    @patch("handler_coeffgen.subprocess.run")
    @patch("handler_coeffgen.report_status")
    @patch("handler_coeffgen.s3")
    def test_coeffgen_chunked_forwards_n_threads_and_reports_threads(self, mock_s3, mock_report, mock_run, mock_open, mock_getsize, mock_remove):
        import handler_coeffgen as mod

        mock_s3.get_object.return_value = {"Body": io.BytesIO(b"\x00" * 64)}
        mock_run.return_value = _DummyCompleted(json.dumps({
            "mode": "coeffgen_chunked",
            "function": "g1",
            "n_coeffs": 8,
            "degree": 7,
            "step_start": 0,
            "step_count": 4,
            "n_t": 4,
            "data_bytes": 256,
            "threads": 5,
            "elapsed_us": 3210,
        }))
        mock_getsize.return_value = 256

        params_data = _NoCloseBytesIO()
        coeffs_data = _NoCloseBytesIO(b"\x00" * 256)

        def _open_side_effect(path, mode="r", *args, **kwargs):
            if path == "/tmp/params_chunk.bin" and "wb" in mode:
                params_data.seek(0)
                params_data.truncate(0)
                return params_data
            if path == "/tmp/coeffs_chunk_2.bin" and "rb" in mode:
                coeffs_data.seek(0)
                return coeffs_data
            raise AssertionError(f"unexpected open({path!r}, {mode!r})")

        mock_open.side_effect = _open_side_effect

        result = mod.handle_coeffgen_chunked({
            "job_id": "compute_j",
            "chunk_idx": 2,
            "step_start": 10,
            "step_count": 4,
            "params_key": "renders/compute_j/params.bin",
            "params_step_start": 0,
            "params_step_count": 4,
            "task_id": "compute_run_coeffgen_2",
            "function": "g1",
            "coeff_transforms": [],
            "n_threads": 5,
            "s3_key": "renders/compute_j/coeffs_0002.bin",
        })

        body = json.loads(result["body"])
        self.assertEqual(body["threads"], 5)
        get_kwargs = mock_s3.get_object.call_args.kwargs
        self.assertEqual(get_kwargs["Range"], "bytes=0-63")
        spec = json.loads(mock_run.call_args.kwargs["input"])
        self.assertEqual(spec["n_threads"], 5)
        started_call = mock_report.call_args_list[0]
        self.assertEqual(started_call.kwargs["result_data"]["threads"], 5)
        done_call = mock_report.call_args_list[-1]
        self.assertEqual(done_call.kwargs["result_data"]["threads"], 5)

    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.subprocess.run")
    def test_degree_probe_reports_stable_shape_and_fused_estimate(self, mock_run, mock_remove):
        import handler_coeffgen as mod

        mock_run.side_effect = [
            _DummyCompleted(json.dumps({"degree": 7, "n_coeffs": 8, "data_bytes": 256})),
            _DummyCompleted(json.dumps({"degree": 7, "n_coeffs": 8, "data_bytes": 256})),
        ]

        result = mod.handle_degree_probe({
            "function": "g1",
            "param_transforms": [],
            "coeff_transforms": [],
            "cfpv": [],
            "N": 64,
            "times": 2,
            "n_chunks": 4,
            "solver_mode": "aberth_mt",
            "fused_threads": 4,
            "auto_hires_chunks": True,
        })
        body = json.loads(result["body"])
        self.assertTrue(body["probe_stable"])
        self.assertEqual(body["degree"], 7)
        self.assertEqual(body["n_coeffs"], 8)
        self.assertIn("probe_signature", body)
        self.assertNotIn("param_gen_us", body)
        self.assertGreater(body["elapsed_us"], 0)
        self.assertIn("fused_estimate", body)
        self.assertGreaterEqual(body["fused_estimate"]["actual_chunks"], body["fused_estimate"]["min_safe_chunks"])

    @patch("handler_coeffgen.os.remove")
    @patch("handler_coeffgen.subprocess.run")
    def test_degree_probe_marks_unstable_mismatch(self, mock_run, mock_remove):
        import handler_coeffgen as mod

        mock_run.side_effect = [
            _DummyCompleted(json.dumps({"degree": 7, "n_coeffs": 8, "data_bytes": 256})),
            _DummyCompleted(json.dumps({"degree": 9, "n_coeffs": 10, "data_bytes": 320})),
        ]

        result = mod.handle_degree_probe({
            "function": "g1",
            "param_transforms": [],
            "coeff_transforms": [],
            "cfpv": [],
        })
        body = json.loads(result["body"])
        self.assertFalse(body["probe_stable"])
        self.assertEqual(len(body["samples"]), 2)


if __name__ == "__main__":
    unittest.main()
