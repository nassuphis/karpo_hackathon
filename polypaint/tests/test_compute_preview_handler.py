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
        "preview_size": 1000,
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
    @patch("handler_compute_preview.compute_viewport_from_bin")
    def test_compute_preview_success_cm_returns_inline_png(self, mock_viewport, mock_run, mock_tmp_stats):
        import handler_compute_preview as mod

        mock_tmp_stats.return_value = {
            "path": "/tmp",
            "free_bytes": 8 * 1024 * 1024 * 1024,
            "total_bytes": 10 * 1024 * 1024 * 1024,
        }
        mock_viewport.return_value = {
            "center_re": 0.0,
            "center_im": 0.0,
            "scale": 4096.0,
            "n_roots": 32 * 32 * 2,
            "q_re": [-1.0, 1.0],
            "q_im": [-1.0, 1.0],
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

        result = mod.handler({"body": json.dumps(_event(solver_mode="companion_matrix", quantile=0.02, shim=0.1))}, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["solver_mode"], "companion_matrix")
        self.assertEqual(body["N_preview"], 32)
        self.assertEqual(body["degree"], 2)
        self.assertEqual(body["preview_size"], 1000)
        self.assertEqual(body["image_width"], 1000)
        self.assertEqual(body["image_height"], 1000)
        self.assertEqual(body["quantile"], 0.02)
        self.assertEqual(body["shim"], 0.1)
        self.assertTrue(body["image_png_base64"].startswith("iVBOR"))
        self.assertGreater(body["n_roots_total"], 0)
        self.assertEqual(calls, ["sweep_coeffgen", "sweep_cm"])
        mock_viewport.assert_called_once()
        _, kwargs = mock_viewport.call_args
        self.assertEqual(kwargs["quantile"], 0.02)
        self.assertEqual(kwargs["shim"], 0.1)

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

    @patch("handler_compute_preview.subprocess.run")
    def test_compute_preview_refuses_large_roots_cm_before_coeffgen(self, mock_run):
        import handler_compute_preview as mod

        result = mod.handler(
            {
                "body": json.dumps(
                    _event(
                        N_preview=256,
                        coeff_transforms=[["power", "8"], ["roots_cm", "hi"]],
                    )
                )
            },
            None,
        )
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 400)
        self.assertIn("roots_cm coefficient transform is too slow", body["message"])
        self.assertIn("N-preview=256", body["message"])
        self.assertIn("N-preview <= 128", body["message"])
        self.assertIn("coeff=power(8),roots_cm(hi)", body["message"])
        mock_run.assert_not_called()

    def test_compute_preview_rejects_invalid_quantile_and_shim(self):
        import handler_compute_preview as mod

        result = mod.handler({"body": json.dumps(_event(quantile=0.6))}, None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 500)
        self.assertIn("preview quantile must be in [0, 0.5)", body["message"])

        result = mod.handler({"body": json.dumps(_event(shim=1.5))}, None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 500)
        self.assertIn("preview shim must be in [0, 1]", body["message"])

        result = mod.handler({"body": json.dumps(_event(preview_size=99999))}, None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 500)
        self.assertIn("preview size must be between 64 and", body["message"])


if __name__ == "__main__":
    unittest.main()
