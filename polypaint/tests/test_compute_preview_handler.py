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
        "solver_mode": "aberth_mt",
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
        self.assertEqual(body["viewport"]["center_re"], 0.0)
        self.assertEqual(body["viewport"]["center_im"], 0.0)
        self.assertEqual(body["viewport"]["scale_ref"], 4096.0)
        self.assertAlmostEqual(body["viewport"]["min_re"], -0.5)
        self.assertAlmostEqual(body["viewport"]["max_re"], 0.5)
        self.assertAlmostEqual(body["viewport"]["min_im"], -0.5)
        self.assertAlmostEqual(body["viewport"]["max_im"], 0.5)
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
        self.assertIn("solver=AE-MT", body["message"])
        self.assertIn("N_preview=32", body["message"])
        self.assertIn("function=g1", body["message"])
        self.assertIn("coeff=roots_cm(lo)", body["message"])

    @patch("handler_compute_preview.tmp_space_stats")
    @patch("handler_compute_preview.subprocess.run")
    @patch("handler_compute_preview.compute_viewport_from_bin")
    def test_compute_preview_forwards_param_program_chain(self, mock_viewport, mock_run, mock_tmp_stats):
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
        specs = []

        def fake_run(cmd, input=None, capture_output=None, text=None, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            specs.append(json.loads(input))
            out_path = cmd[1]
            if exe == "sweep_coeffgen":
                with open(out_path, "wb") as fh:
                    fh.write(b"\0" * (32 * 32 * 3 * 8))
                return unittest.mock.MagicMock(
                    returncode=0,
                    stdout=json.dumps({"data_bytes": 32 * 32 * 3 * 8, "n_coeffs": 3, "degree": 2}),
                    stderr="",
                )
            if exe == "sweep_mt":
                with open(out_path, "wb") as fh:
                    fh.write(_roots_bytes(32 * 32, 2))
                return unittest.mock.MagicMock(
                    returncode=0,
                    stdout=json.dumps({"n_t": 32 * 32, "degree": 2, "avg_iterations": 0, "skipped_overflow": 0}),
                    stderr="",
                )
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        result = mod.handler({"body": json.dumps(_event(
            param_transforms=[["unit_circle"]],
            param_program_chain=[
                ["push", "t1"],
                ["push", "t2"],
                ["add"],
                ["emit", "p1"],
                ["push", "t1"],
                ["push", "t2"],
                ["subtract"],
                ["emit", "p2"],
            ],
        ))}, None)

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(specs[0]["param_transforms"], [])
        self.assertIn("param_program", specs[0])
        self.assertEqual(specs[0]["param_program"]["token_count"], 8)
        self.assertNotIn("scalar_exprs", specs[0]["param_program"])

    @patch("handler_compute_preview.tmp_space_stats")
    @patch("handler_compute_preview.subprocess.run")
    @patch("handler_compute_preview.compute_viewport_from_bin")
    @patch("handler_compute_preview._s3_client")
    def test_compute_preview_resolves_coeff_program_macro(self, mock_s3_client, mock_viewport, mock_run, mock_tmp_stats):
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
        saved = json.dumps({"chain": [["legacy", "rev", "poly", "poly"]]}).encode()
        mock_s3_client.return_value.get_object.return_value = {
            "Body": unittest.mock.MagicMock(read=lambda: saved)
        }
        specs = []

        def fake_run(cmd, input=None, capture_output=None, text=None, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            specs.append(json.loads(input))
            out_path = cmd[1]
            if exe == "sweep_coeffgen":
                with open(out_path, "wb") as fh:
                    fh.write(b"\0" * (32 * 32 * 3 * 8))
                return unittest.mock.MagicMock(
                    returncode=0,
                    stdout=json.dumps({"data_bytes": 32 * 32 * 3 * 8, "n_coeffs": 3, "degree": 2}),
                    stderr="",
                )
            if exe == "sweep_mt":
                with open(out_path, "wb") as fh:
                    fh.write(_roots_bytes(32 * 32, 2))
                return unittest.mock.MagicMock(
                    returncode=0,
                    stdout=json.dumps({"n_t": 32 * 32, "degree": 2, "avg_iterations": 0}),
                    stderr="",
                )
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        result = mod.handler({"body": json.dumps(_event(
            pipeline_mode="program",
            function="const",
            cfpv=[3, 1, 0],
            coeff_program_chain=[["macro", "poly-test1"]],
        ))}, None)

        self.assertEqual(result["statusCode"], 200)
        mock_s3_client.return_value.get_object.assert_called_once_with(
            Bucket=mod.BUCKET,
            Key="polypaint/coeff-programs/poly-test1.json",
        )
        self.assertEqual(specs[0]["coeff_transforms"], [["rev"]])
        self.assertNotIn("coeff_program", specs[0])

    @patch("handler_compute_preview.subprocess.run")
    def test_compute_debug_poly_forwards_poke_coeff_program(self, mock_run):
        import handler_compute_preview as mod

        specs = []

        def fake_run(cmd, input=None, capture_output=None, text=None, timeout=None, env=None):
            specs.append(json.loads(input))
            out_path = cmd[1]
            with open(out_path, "wb") as fh:
                fh.write(struct.pack("<ffffff", 1.0, 0.0, 2.0, 0.0, 3.0, 0.0))
            return unittest.mock.MagicMock(
                returncode=0,
                stdout=json.dumps({
                    "mode": "compute_debug",
                    "function": "const",
                    "u": 0.25,
                    "v": 0.75,
                    "grid_n": 32,
                    "param": {"t1": [0.25, 0], "t2": [0.75, 0], "p1": [0.25, 0], "p2": [0.75, 0]},
                    "coeff": {
                        "cf_len": 3,
                        "poly_len": 3,
                        "n_coeffs": 3,
                        "degree": 2,
                        "cf": [[1, 0], [1, 0], [1, 0]],
                        "poly": [[0, 25], [1, 0], [1, 0]],
                    },
                    "data_bytes": 24,
                }),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler({"body": json.dumps(_event(
            debug_stage="poly",
            pipeline_mode="program",
            function="const",
            cfpv=[3, 1, 0],
            u=0.25,
            v=0.75,
            coeff_program_chain=[["poke_poly", "0", "100j*p1"]],
        ))}, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(body["stage"], "poly")
        self.assertEqual(specs[0]["mode"], "compute_debug")
        self.assertIn("coeff_program", specs[0])
        self.assertEqual(specs[0]["coeff_program"]["token_count"], 1)
        self.assertEqual(specs[0]["coeff_program"]["tokens"][0]["op"], 10)

    @patch("handler_compute_preview.subprocess.run")
    def test_compute_debug_poly_compiles_coeff_program_source_text(self, mock_run):
        import handler_compute_preview as mod

        specs = []

        def fake_run(cmd, input=None, capture_output=None, text=None, timeout=None, env=None):
            specs.append(json.loads(input))
            out_path = cmd[1]
            with open(out_path, "wb") as fh:
                fh.write(struct.pack("<ffffff", 1.0, 0.0, 2.0, 0.0, 3.0, 0.0))
            return unittest.mock.MagicMock(
                returncode=0,
                stdout=json.dumps({
                    "mode": "compute_debug",
                    "function": "const",
                    "u": 0.25,
                    "v": 0.75,
                    "grid_n": 32,
                    "param": {"t1": [0.25, 0], "t2": [0.75, 0], "p1": [0.25, 0], "p2": [0.75, 0]},
                    "coeff": {"cf_len": 3, "poly_len": 3, "n_coeffs": 3, "degree": 2},
                    "data_bytes": 24,
                }),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler({"body": json.dumps(_event(
            debug_stage="poly",
            pipeline_mode="program",
            function="const",
            cfpv=[3, 1, 0],
            u=0.25,
            v=0.75,
            coeff_program_source_text="cf\nrev\nemit\n",
        ))}, None)

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(specs[0]["coeff_transforms"], [])
        self.assertIn("coeff_program", specs[0])
        self.assertEqual(specs[0]["coeff_program"]["token_count"], 3)

    def test_compute_debug_poly_source_parse_error_returns_structured_diagnostics(self):
        import handler_compute_preview as mod

        result = mod.handler({"body": json.dumps(_event(
            debug_stage="poly",
            pipeline_mode="program",
            function="const",
            cfpv=[3, 1, 0],
            u=0.25,
            v=0.75,
            coeff_program_source_text="bad(",
        ))}, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 400)
        self.assertIn("diagnostics", body)
        self.assertGreaterEqual(len(body["diagnostics"]), 1)
        self.assertIn("line", body["diagnostics"][0])
        self.assertIn("column", body["diagnostics"][0])
        self.assertIn("message", body["diagnostics"][0])

    @patch("handler_compute_preview.subprocess.run")
    def test_compute_debug_poly_chain_mode_ignores_coeff_program_source_text(self, mock_run):
        import handler_compute_preview as mod

        specs = []

        def fake_run(cmd, input=None, capture_output=None, text=None, timeout=None, env=None):
            specs.append(json.loads(input))
            out_path = cmd[1]
            with open(out_path, "wb") as fh:
                fh.write(struct.pack("<ffffff", 1.0, 0.0, 2.0, 0.0, 3.0, 0.0))
            return unittest.mock.MagicMock(
                returncode=0,
                stdout=json.dumps({
                    "mode": "compute_debug",
                    "function": "const",
                    "u": 0.25,
                    "v": 0.75,
                    "grid_n": 32,
                    "param": {"t1": [0.25, 0], "t2": [0.75, 0], "p1": [0.25, 0], "p2": [0.75, 0]},
                    "coeff": {"cf_len": 3, "poly_len": 3, "n_coeffs": 3, "degree": 2},
                    "data_bytes": 24,
                }),
                stderr="",
            )

        mock_run.side_effect = fake_run

        result = mod.handler({"body": json.dumps(_event(
            debug_stage="poly",
            pipeline_mode="chain",
            function="const",
            cfpv=[3, 1, 0],
            u=0.25,
            v=0.75,
            coeff_transforms=[["rev"]],
            coeff_program_chain=[["const", "3", "1"], ["emit"]],
            coeff_program_source_text="bad(",
        ))}, None)

        self.assertEqual(result["statusCode"], 200)
        self.assertEqual(specs[0]["coeff_transforms"], [["rev"]])
        self.assertNotIn("coeff_program", specs[0])

    @patch("handler_compute_preview.tmp_space_stats")
    @patch("handler_compute_preview.subprocess.run")
    @patch("handler_compute_preview.compute_viewport_from_bin")
    def test_compute_preview_accepts_moebius_imag_first_coefficients(self, mock_viewport, mock_run, mock_tmp_stats):
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
        specs = []

        def fake_run(cmd, input=None, capture_output=None, text=None, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            specs.append(json.loads(input))
            out_path = cmd[1]
            if exe == "sweep_coeffgen":
                with open(out_path, "wb") as fh:
                    fh.write(b"\0" * (32 * 32 * 3 * 8))
                return unittest.mock.MagicMock(
                    returncode=0,
                    stdout=json.dumps({"data_bytes": 32 * 32 * 3 * 8, "n_coeffs": 3, "degree": 2}),
                    stderr="",
                )
            if exe == "sweep_mt":
                with open(out_path, "wb") as fh:
                    fh.write(_roots_bytes(32 * 32, 2))
                return unittest.mock.MagicMock(
                    returncode=0,
                    stdout=json.dumps({"n_t": 32 * 32, "degree": 2, "avg_iterations": 0}),
                    stderr="",
                )
            raise AssertionError(f"unexpected subprocess call: {cmd}")

        mock_run.side_effect = fake_run

        result = mod.handler({"body": json.dumps(_event(
            function="poly_1",
            param_program_chain=[
                ["legacy", "unit_circle", "both", "both"],
                ["legacy", "moebius", "both", "both", "1-2j", "2+1j", "-2j+4", "10j-3"],
            ],
            coeff_transforms=[["rev", "0"]],
        ))}, None)

        self.assertEqual(result["statusCode"], 200)
        self.assertNotIn("param_program", specs[0])
        self.assertEqual(specs[0]["param_transforms"][0], ["unit_circle"])
        self.assertEqual(specs[0]["param_transforms"][1][0], "moebius")
        self.assertEqual(
            [complex(value.replace("i", "j")) for value in specs[0]["param_transforms"][1][1:]],
            [1 - 2j, 2 + 1j, 4 - 2j, -3 + 10j],
        )

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
        # Client-input mistakes are 400s, not server faults (CR14): invalid
        # request values must not be indistinguishable from backend crashes.
        import handler_compute_preview as mod

        result = mod.handler({"body": json.dumps(_event(quantile=0.6))}, None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 400)
        self.assertIn("preview quantile must be in [0, 0.5)", body["message"])

        result = mod.handler({"body": json.dumps(_event(shim=1.5))}, None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 400)
        self.assertIn("preview shim must be in [0, 1]", body["message"])

        result = mod.handler({"body": json.dumps(_event(preview_size=99999))}, None)
        body = json.loads(result["body"])
        self.assertEqual(result["statusCode"], 400)
        self.assertIn("preview size must be between 64 and", body["message"])

    def test_compute_preview_rejects_removed_ae_solver(self):
        import handler_compute_preview as mod

        result = mod.handler({"body": json.dumps(_event(solver_mode="aberth"))}, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 400)
        self.assertIn("unsupported preview solver_mode: aberth", body["message"])

    def test_compute_preview_reports_invalid_param_program_as_request_error(self):
        import handler_compute_preview as mod

        result = mod.handler({"body": json.dumps(_event(
            param_program_chain=[["legacy", "moebius", "both", "both", "bad", "0", "0", "1"]]
        ))}, None)
        body = json.loads(result["body"])

        self.assertEqual(result["statusCode"], 400)
        self.assertIn("invalid param_program_chain", body["message"])
        self.assertIn("legacy(moebius) coefficient 0", body["message"])


if __name__ == "__main__":
    unittest.main()
