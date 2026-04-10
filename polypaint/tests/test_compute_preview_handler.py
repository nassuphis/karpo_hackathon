import json
import math
import os
import struct
import sys
import unittest
from unittest.mock import patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "solver_mode": "aberth",
        "N_preview": 32,
        "function": "g1",
        "param_transforms": [],
        "coeff_transforms": [],
        "cfpv": [],
    }
    payload.update(overrides)
    return payload


def _roots_bytes(n_steps, degree):
    out = bytearray()
    for i in range(n_steps * degree):
        angle = (i % 32) * 0.1963495408
        out.extend(struct.pack("<ff", 0.8 * math.cos(angle), 0.8 * math.sin(angle)))
    return bytes(out)


class TestComputePreviewHandler(unittest.TestCase):

    @patch("handler_compute_preview.tmp_space_stats")
    @patch("handler_compute_preview.subprocess.run")
    def test_compute_preview_success_cm_returns_inline_png(self, mock_run, mock_tmp_stats):
        import handler_compute_preview as mod

        mock_tmp_stats.return_value = {
            "path": "/tmp",
            "free_bytes": 8 * 1024 * 1024 * 1024,
            "total_bytes": 10 * 1024 * 1024 * 1024,
        }
        calls = []

        def fake_run(cmd, input=None, capture_output=None, text=None, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            calls.append(exe)
            out_path = cmd[1]
            if exe == "sweep_coeffgen":
                with open(out_path, "wb") as fh:
                    fh.write(b"\0" * (32 * 32 * 3 * 8))
                return unittest.mock.MagicMock(
                    returncode=0,
                    stdout=json.dumps({"data_bytes": 32 * 32 * 3 * 8, "n_coeffs": 3, "degree": 2}),
                    stderr="",
                )
            if exe == "sweep_cm":
                with open(out_path, "wb") as fh:
                    fh.write(_roots_bytes(32 * 32, 2))
                return unittest.mock.MagicMock(
                    returncode=0,
                    stdout=json.dumps({"n_t": 32 * 32, "degree": 2, "avg_iterations": 0, "skipped_overflow": 0}),
                    stderr="",
                )
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        result = mod.handler({"body": json.dumps(_event(solver_mode="companion_matrix"))}, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["solver_mode"], "companion_matrix")
        self.assertEqual(body["N_preview"], 32)
        self.assertEqual(body["degree"], 2)
        self.assertEqual(body["image_width"], 32)
        self.assertEqual(body["image_height"], 32)
        self.assertTrue(body["image_png_base64"].startswith("iVBOR"))
        self.assertGreater(body["n_roots_total"], 0)
        self.assertEqual(calls, ["sweep_coeffgen", "sweep_cm"])

    @patch("handler_compute_preview.tmp_space_stats")
    @patch("handler_compute_preview.subprocess.run")
    def test_compute_preview_coeffgen_failure_reports_context(self, mock_run, mock_tmp_stats):
        import handler_compute_preview as mod

        mock_tmp_stats.return_value = {
            "path": "/tmp",
            "free_bytes": 8 * 1024 * 1024 * 1024,
            "total_bytes": 10 * 1024 * 1024 * 1024,
        }
        mock_run.return_value = unittest.mock.MagicMock(
            returncode=1,
            stdout="",
            stderr="bad transform",
        )

        result = mod.handler({"body": json.dumps(_event(coeff_transforms=[["roots_cm", "lo"]]))}, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 500)
        self.assertIn("coeffgen failed: bad transform", body["message"])
        self.assertIn("solver=AE", body["message"])
        self.assertIn("N_preview=32", body["message"])
        self.assertIn("function=g1", body["message"])
        self.assertIn("coeff=roots_cm(lo)", body["message"])


if __name__ == "__main__":
    unittest.main()
