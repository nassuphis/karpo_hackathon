import base64
import json
import os
import struct
import sys
import unittest
from unittest.mock import MagicMock, patch

from botocore.exceptions import ClientError


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


PNG_1X1 = base64.b64decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+/p9sAAAAASUVORK5CYII="
)


def _event(**overrides):
    payload = {
        "job_id": "j",
        "degree": 2,
        "n_coeffs": 0,
        "preview_pix": 16,
        "quality": 90,
        "palette": "inferno",
        "view_mode": "explicit",
        "min_re": -2.0,
        "max_re": 2.0,
        "min_im": -2.0,
        "max_im": 2.0,
        "rotation": 0.0,
        "solve_score_chain": [["proximity", "slv", "0.1"]],
        "solve_score_normalize": True,
        "lores_bin_key": "renders/j/lores.bin",
        "root_transforms": [],
    }
    payload.update(overrides)
    return payload


class _ChunkBody:
    def __init__(self, data):
        self._data = data

    def iter_chunks(self, chunk_size=1024 * 1024):
        for i in range(0, len(self._data), chunk_size):
            yield self._data[i:i + chunk_size]


class TestRenderLoresPreviewHandler(unittest.TestCase):
    @patch("handler_render_lores_preview._run_json_binary")
    def test_recompute_vector_constants_use_normal_coeffgen_and_cm(self, mock_binary):
        import handler_render_lores_preview as mod

        modes = []

        def run_binary(_binary, out_path, spec, **_kwargs):
            modes.append(spec["mode"])
            if spec["mode"] == "param_gen":
                with open(out_path, "wb") as fh:
                    fh.write(b"\0" * (2 * 2 * 16))
                return {"mode": "param_gen", "data_bytes": 2 * 2 * 16}
            if spec["mode"] == "coeffgen_chunked":
                self.assertTrue(spec["coeff_program"]["vector_constants"])
                with open(out_path, "wb") as fh:
                    fh.write(b"\0" * (2 * 2 * 3 * 8))
                return {
                    "mode": spec["mode"],
                    "data_bytes": 2 * 2 * 3 * 8,
                    "n_coeffs": 3,
                    "degree": 2,
                }
            if spec["mode"] == "solve_cm":
                with open(out_path, "wb") as fh:
                    fh.write(b"\0" * (2 * 2 * 2 * 8))
                return {
                    "mode": spec["mode"],
                    "n_t": 2 * 2,
                    "degree": 2,
                }
            raise AssertionError(f"unexpected native mode: {spec['mode']}")

        mock_binary.side_effect = run_binary
        calc = {
            "N": 2,
            "times": 1,
            "degree": 2,
            "n_coeffs": 3,
            "solver": "companion_matrix",
            "pipeline": {
                "function": "const",
                "param_transforms": [],
                "coeff_transforms": [],
                "cfpv": [3, 0, 0],
                "coeff_program": {
                    "version": 1,
                    "tokens": [{"op": 48, "n_args": 1, "args": [0]}],
                    "vector_constants": [
                        {"length": 3, "values": [1, 0, -3, 0, 2, 0]}
                    ],
                },
            },
        }
        try:
            result = mod._materialize_recomputed_preview(
                params={}, calc=calc, job_id="j", degree=2, n_coeffs=3, view_n=2
            )
        finally:
            for path in (mod.TMP_PARAMS, mod.TMP_COEFFS, mod.TMP_ROOTS):
                try:
                    os.remove(path)
                except OSError:
                    pass

        self.assertEqual(modes, ["param_gen", "coeffgen_chunked", "solve_cm"])
        self.assertEqual(result["solver_mode"], "companion_matrix")
        self.assertNotIn("direct_coeff_solve", result)
        self.assertNotIn("coeff_precision", result)

    def test_recompute_routes_brush_solvers_and_aberth_iters(self):
        """Solver-brush wave: calc.solver jenkins_traub/newton recompute via
        the sweep_cm binary with their own mode strings; calc.solver_iters
        flows to the aberth spec as max_iter."""
        import handler_render_lores_preview as mod

        self.assertEqual(mod._calc_solver_mode({"solver": "jenkins_traub"}), "jenkins_traub")
        self.assertEqual(mod._calc_solver_mode({"solver": "solve_jt"}), "jenkins_traub")
        self.assertEqual(mod._calc_solver_mode({"solver": "newton"}), "newton")
        self.assertEqual(mod._calc_solver_mode({"solver": "solve_newton"}), "newton")
        self.assertEqual(mod._calc_solver_mode({"solver": "nonsense"}), "aberth_mt")
        self.assertEqual(mod._calc_solver_iters({"solver_iters": 5}), 5)
        self.assertEqual(mod._calc_solver_iters({"solver_iters": 0}), 0)
        self.assertEqual(mod._calc_solver_iters({"solver_iters": 999}), 0)
        self.assertEqual(mod._calc_solver_iters({}), 0)

    def test_preview_palette_grid_requires_complete_pass_grid(self):
        from handler_render_lores_preview import _preview_palette_grid_n

        self.assertEqual(_preview_palette_grid_n({"view_N": 4}, 16), 4)
        self.assertEqual(_preview_palette_grid_n({"view_N": 4}, 32), 4)
        self.assertEqual(_preview_palette_grid_n({"view_N": 4}, 18), 0)
        self.assertEqual(_preview_palette_grid_n({}, 9), 3)

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_returns_inline_png_without_s3_writes(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_FRAGMENT, TMP_PALETTE_FRAGMENT, handler

        mock_s3.get_object.return_value = {"Body": _ChunkBody(b"\x00" * (3 * 2 * 2 * 4))}
        mock_s3.generate_presigned_url.return_value = "https://example.test/lores.bin"

        def subprocess_fake(cmd, **kwargs):
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "min_score": 0.0,
                    "max_score": 1.0,
                    "n_solves": 3,
                    "threads": 1,
                }), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 2,
                    "n_solves": 3,
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "min_score": 0.0,
                    "q05": 0.1,
                    "q95": 0.9,
                    "max_score": 1.0,
                    "threads": 1,
                }), stderr="")
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
                fh.write((5).to_bytes(4, "little") + bytes([220]))
            with open(TMP_PALETTE_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 2, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(lores_N=1, background_color="#abc"), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        self.assertEqual(body["content_type"], "image/png")
        self.assertEqual(base64.b64decode(body["image_base64"]), PNG_1X1)
        self.assertEqual(base64.b64decode(body["palette_image_base64"]), PNG_1X1)
        self.assertEqual(body["palette_pix"], 1)
        self.assertEqual(body["palette_fragment_entries"], 1)
        self.assertEqual(body["preview_pix"], 16)
        self.assertEqual(body["fragment_entries"], 2)
        self.assertEqual(body["nonzero_pixels"], 2)
        self.assertEqual(body["emission_histograms"][0]["label"], "E1")
        self.assertEqual(body["emission_histograms"][0]["histogram"][10], 1)
        self.assertEqual(body["emission_histograms"][0]["histogram"][220], 1)
        self.assertEqual(body["emission_histograms"][0]["total"], 2)

        self.assertFalse(mock_s3.put_object.called)
        self.assertFalse(mock_s3.upload_file.called)
        self.assertFalse(mock_s3.upload_fileobj.called)

        summary_cmd = next(call[0][0] for call in mock_run.call_args_list if "--mode=summary" in call[0][0])
        raster_cmd = mock_run.call_args_list[-1][0][0]
        self.assertIn("--score_output_normalize=1", summary_cmd)
        self.assertIn("--score_program=v2;m0-0", summary_cmd)
        self.assertIn("--score_metrics=proximity", summary_cmd)
        self.assertFalse(any(arg.startswith("--metric=") for arg in summary_cmd))
        self.assertIn("--score_output_normalize=1", raster_cmd)
        self.assertIn("--score_output_clip_lo=0.1", raster_cmd)
        self.assertIn("--score_output_clip_hi=0.9", raster_cmd)
        self.assertIn("--fragment_prefix=/tmp/render_lores_preview_fragment", raster_cmd)
        self.assertIn("--associated_palette_fragment_prefix=/tmp/render_lores_preview_palette_fragment", raster_cmd)
        self.assertIn("--palette_grid_n=1", raster_cmd)
        self.assertIn("--palette_step_start=0", raster_cmd)
        self.assertFalse(any(arg.startswith("--xformed_roots_output=") for arg in raster_cmd))
        image_render_call = mock_render.call_args_list[0].kwargs
        self.assertEqual(image_render_call["background_color"], "aabbcc")
        palette_render_call = mock_render.call_args_list[1].kwargs
        self.assertFalse(palette_render_call["zero_background"])

    def test_rejects_invalid_background_color(self):
        from handler_render_lores_preview import handler

        resp = handler(_event(background_color="not-a-color"), None)
        self.assertEqual(resp["statusCode"], 500)
        body = json.loads(resp["body"])
        self.assertIn("background_color must be 6-digit hex", body["detail"])
        self.assertEqual(body["phase"], "render-lores-preview")

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_rejects_incomplete_preview_palette_fragment(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_FRAGMENT, TMP_PALETTE_FRAGMENT, handler

        mock_s3.get_object.return_value = {"Body": _ChunkBody(b"\x00" * (4 * 2 * 2 * 4))}
        mock_s3.generate_presigned_url.return_value = "https://example.test/lores.bin"

        def subprocess_fake(cmd, **kwargs):
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "min_score": 0.0,
                    "max_score": 1.0,
                    "n_solves": 4,
                    "threads": 1,
                }), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 2,
                    "n_solves": 4,
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "min_score": 0.0,
                    "q05": 0.1,
                    "q95": 0.9,
                    "max_score": 1.0,
                    "threads": 1,
                }), stderr="")
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
            with open(TMP_PALETTE_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 1, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(lores_N=2), None)
        self.assertEqual(resp["statusCode"], 500)
        body = json.loads(resp["body"])
        self.assertIn("preview palette fragment entry count mismatch", body["detail"])
        self.assertEqual(body["phase"], "render-lores-preview")

    @patch("handler_render_lores_preview.s3")
    def test_rejects_invalid_preview_pix(self, mock_s3):
        from handler_render_lores_preview import handler

        resp = handler(_event(preview_pix=99999), None)
        self.assertEqual(resp["statusCode"], 500)
        body = json.loads(resp["body"])
        self.assertIn("preview_pix", body["detail"])
        self.assertEqual(body["phase"], "render-lores-preview")
        self.assertFalse(mock_s3.get_object.called)

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_coeff_source_defaults_missing_n_coeffs_to_degree_plus_one(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_FRAGMENT, TMP_PALETTE_FRAGMENT, handler

        root_bytes = b"\x00" * (3 * 2 * 2 * 4)
        coeff_bytes = b"\x00" * (3 * 3 * 2 * 4)

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key == "renders/j/lores.bin":
                return {"Body": _ChunkBody(root_bytes)}
            if key == "renders/j/lores_coeffs.bin":
                return {"Body": _ChunkBody(coeff_bytes)}
            raise AssertionError(f"unexpected key: {key}")

        mock_s3.get_object.side_effect = get_object
        mock_s3.generate_presigned_url.return_value = "https://example.test/object.bin"

        def subprocess_fake(cmd, **kwargs):
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "min_score": 0.0,
                    "max_score": 1.0,
                    "n_solves": 3,
                    "threads": 1,
                }), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 2,
                    "n_solves": 3,
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "min_score": 0.0,
                    "q05": 0.1,
                    "q95": 0.9,
                    "max_score": 1.0,
                    "threads": 1,
                }), stderr="")
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([128]))
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 1, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        payload = _event(
            n_coeffs=None,
            solve_score_chain=[["proximity", "cf", "0.1"]],
            lores_coeffs_key="",
        )
        resp = handler(payload, None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        self.assertEqual(body["n_coeffs"], 3)

        raster_cmd = mock_run.call_args_list[-1][0][0]
        self.assertIn("--score_coeff_degree=3", raster_cmd)
        self.assertTrue(any(arg.startswith("--score_coeff_manifest=") for arg in raster_cmd))

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_logical_lores_preview_materializes_hires_subset_to_local_manifest(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_FRAGMENT, TMP_PALETTE_FRAGMENT, handler

        roots_key = "renders/j/chunk_0.bin"
        roots_bytes = bytearray()
        for idx in range(25):
            roots_bytes.extend(struct.pack("<ff", float(idx), 0.0))
        calc = {
            "N": 5,
            "times": 1,
            "degree": 1,
            "n_coeffs": 2,
            "lores": {"N": 5, "n_steps": 25},
            "chunks": [{
                "idx": 0,
                "bin_key": roots_key,
                "step_count": 25,
                "bin_size": len(roots_bytes),
            }],
        }

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key == "renders/j/calc.json":
                return {"Body": _ChunkBody(json.dumps(calc).encode("utf-8"))}
            if key == roots_key:
                body = bytes(roots_bytes)
                range_hdr = kwargs.get("Range")
                if range_hdr:
                    raw = range_hdr[len("bytes="):]
                    lo, hi = raw.split("-", 1)
                    body = body[int(lo):int(hi) + 1]
                return {"Body": _ChunkBody(body)}
            raise AssertionError(f"unexpected key: {key}")

        mock_s3.get_object.side_effect = get_object
        seen_local_manifest = {}

        def subprocess_fake(cmd, **kwargs):
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "min_score": 0.0,
                    "max_score": 1.0,
                    "n_solves": 25,
                    "threads": 1,
                }), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 1,
                    "n_solves": 25,
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "min_score": 0.0,
                    "q05": 0.1,
                    "q95": 0.9,
                    "max_score": 1.0,
                    "threads": 1,
                }), stderr="")
            manifest_arg = next(arg for arg in cmd if arg.startswith("--input_manifest="))
            with open(manifest_arg.split("=", 1)[1], "r", encoding="utf-8") as fh:
                manifest = json.load(fh)
            seen_local_manifest["url"] = manifest["sources"][0]["url"]
            seen_local_manifest["logical_size"] = manifest["logical_size"]
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([64]))
            with open(TMP_PALETTE_FRAGMENT, "wb") as fh:
                for idx in range(25):
                    fh.write(idx.to_bytes(4, "little") + bytes([idx + 1]))
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 1, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(
            degree=1,
            n_coeffs=2,
            logical_lores=True,
            logical_lores_size=5,
            lores_bin_key="",
            solve_score_chain=[["centroid_re", "slv", "0.1"]],
        ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        self.assertEqual(body["source"]["mode"], "logical")
        self.assertEqual(body["source"]["full_N"], 5)
        self.assertEqual(body["source"]["view_N"], 5)
        self.assertEqual(body["n_solves"], 25)
        self.assertTrue(seen_local_manifest["url"].startswith("file://"))
        self.assertEqual(seen_local_manifest["logical_size"], 25 * 1 * 2 * 4)
        self.assertTrue(any("Logical lores materialize:" in line for line in body["logs"]))
        self.assertFalse(mock_s3.generate_presigned_url.called)
        self.assertFalse(mock_s3.put_object.called)

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_recompute_preview_generates_tmp_params_coeffs_roots(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_COEFFS, TMP_FRAGMENT, TMP_PALETTE_FRAGMENT, TMP_PARAMS, TMP_ROOTS, handler

        calc = {
            "N": 5,
            "times": 1,
            "degree": 1,
            "n_coeffs": 2,
            "solver": "aberth_mt",
            "lores": {"N": 5, "n_steps": 25},
            "pipeline": {
                "function": "g1",
                "param_transforms": [],
                "coeff_transforms": [],
                "cfpv": [],
            },
        }

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key == "renders/j/calc.json":
                return {"Body": _ChunkBody(json.dumps(calc).encode("utf-8"))}
            raise AssertionError(f"unexpected key: {key}")

        mock_s3.get_object.side_effect = get_object
        phases = []

        def subprocess_fake(cmd, **kwargs):
            stdin = json.loads(kwargs.get("input") or "{}")
            if stdin.get("mode") == "param_gen":
                phases.append("param_gen")
                with open(TMP_PARAMS, "wb") as fh:
                    fh.write(b"\x00" * (25 * 16))
                return MagicMock(returncode=0, stdout=json.dumps({"mode": "param_gen", "data_bytes": 25 * 16, "threads": 4}), stderr="")
            if stdin.get("mode") == "coeffgen_chunked":
                phases.append("coeffgen")
                with open(TMP_COEFFS, "wb") as fh:
                    fh.write(b"\x00" * (25 * 2 * 2 * 4))
                return MagicMock(returncode=0, stdout=json.dumps({"mode": "coeffgen_chunked", "degree": 1, "n_coeffs": 2, "data_bytes": 25 * 2 * 2 * 4, "threads": 4}), stderr="")
            if stdin.get("mode") == "solve_mt":
                phases.append("solve")
                with open(TMP_ROOTS, "wb") as fh:
                    fh.write(b"\x00" * (25 * 1 * 2 * 4))
                return MagicMock(returncode=0, stdout=json.dumps({"mode": "solve_mt", "avg_iterations": 3.0, "n_threads": 4}), stderr="")
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "min_score": 0.0,
                    "max_score": 1.0,
                    "n_solves": 25,
                    "threads": 1,
                }), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 1,
                    "n_solves": 25,
                    "clip_lo": 0.0,
                    "clip_hi": 1.0,
                    "min_score": 0.0,
                    "q05": 0.1,
                    "q95": 0.9,
                    "max_score": 1.0,
                    "threads": 1,
                }), stderr="")
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([64]))
            with open(TMP_PALETTE_FRAGMENT, "wb") as fh:
                for idx in range(25):
                    fh.write(idx.to_bytes(4, "little") + bytes([idx + 1]))
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 1, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(
            degree=1,
            n_coeffs=2,
            preview_source_mode="recompute",
            preview_source_size=5,
            lores_bin_key="",
            solve_score_chain=[["centroid_re", "slv", "0.1"]],
        ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        self.assertEqual(body["source"]["mode"], "recompute")
        self.assertEqual(body["source"]["view_N"], 5)
        self.assertEqual(body["n_solves"], 25)
        self.assertEqual(phases, ["param_gen", "coeffgen", "solve"])
        self.assertTrue(any("Recompute preview materialize:" in line for line in body["logs"]))
        self.assertFalse(mock_s3.generate_presigned_url.called)
        self.assertFalse(mock_s3.put_object.called)

    @staticmethod
    def _artifact_calc(n=4):
        roots_key = "renders/j/chunk_0.bin"
        roots = bytearray()
        for idx in range(n * n):
            roots.extend(struct.pack("<ff", float(idx), 0.0))
        calc = {
            "N": n,
            "times": 1,
            "degree": 1,
            "n_coeffs": 2,
            "lores": {"N": n, "n_steps": n * n},
            "chunks": [{
                "idx": 0,
                "bin_key": roots_key,
                "step_count": n * n,
                "bin_size": len(roots),
            }],
        }
        return calc, roots_key, bytes(roots)

    @staticmethod
    def _artifact_get_object(calc, roots_key, roots_bytes, step_scores, artifact_id="color_run_abc"):
        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key == "renders/j/calc.json":
                return {"Body": _ChunkBody(json.dumps(calc).encode("utf-8"))}
            if key == roots_key:
                body = roots_bytes
                range_hdr = kwargs.get("Range")
                if range_hdr:
                    raw = range_hdr[len("bytes="):]
                    lo, hi = raw.split("-", 1)
                    body = body[int(lo):int(hi) + 1]
                return {"Body": _ChunkBody(body)}
            if key == "renders/j/step_scores.raw":
                return {"Body": _ChunkBody(step_scores)}
            if key == f"renders/j/color/{artifact_id}/meta.json":
                return {"Body": _ChunkBody(b"{}")}   # empty overlay
            raise AssertionError(f"unexpected key: {key}")
        return get_object

    @classmethod
    def _artifact_head_router(cls, **meta_overrides):
        # head_object router: artifact image heads resolve; sculptures/
        # publication markers are ABSENT (the fail-closed id mint heads them)
        head = cls._artifact_head(**meta_overrides)

        def _head(Bucket=None, Key=None, **kw):
            if str(Key or "").startswith("sculptures/"):
                raise ClientError({"Error": {"Code": "404", "Message": "absent"}}, "HeadObject")
            return head
        return _head

    @staticmethod
    def _artifact_head(**meta_overrides):
        metadata = {
            "step_scores_key": "renders/j/step_scores.raw",
            "step_scores_grid_n": "4",
            "step_count": "16",
            "score_output_channel_count": "1",
            "score_output_interpretation": "scalar_lut",
            "palette": "viridis",
            "background_color": "101010",
            "min_re": "-2.0", "max_re": "2.0", "min_im": "-2.0", "max_im": "2.0",
            "rotation": "0.25",
            "root_transforms": json.dumps([{"fn": 30}]),
        }
        metadata.update(meta_overrides)
        metadata = {k: v for k, v in metadata.items() if v is not None}
        return {"Metadata": metadata, "ContentType": "image/png", "ContentLength": 10}

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_artifact_sculpture_generates_from_stored_scores(self, mock_s3, mock_run, mock_render):
        # THE Wave-B invariant: everything comes from the artifact's recorded
        # provenance — viewport/rotation/transform chain feed the raster, the
        # STORED step_scores (solve order) subsample to the de-serpentined
        # palette raw, and no score evaluation (clip/summary) ever runs.
        from handler_render_lores_preview import TMP_XFORMED_ROOTS, handler

        calc, roots_key, roots_bytes = self._artifact_calc(4)
        step_scores = bytes(range(16))   # solve-order scores 0..15
        mock_s3.get_object.side_effect = self._artifact_get_object(calc, roots_key, roots_bytes, step_scores)
        mock_s3.head_object.return_value = self._artifact_head()
        xformed = b"T" * (16 * 1 * 2 * 2)   # u16 dump
        seen = {}

        def subprocess_fake(cmd, **kwargs):
            self.assertNotIn("--mode=clip", cmd)
            self.assertNotIn("--mode=summary", cmd)
            seen["cmd"] = list(cmd)
            xforms_arg = next(arg for arg in cmd if arg.startswith("--root_xforms="))
            with open(xforms_arg.split("=", 1)[1], "r", encoding="utf-8") as fh:
                seen["xforms"] = json.load(fh)
            with open(TMP_XFORMED_ROOTS, "wb") as fh:
                fh.write(xformed)
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 16, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            seen["render"] = dict(kwargs)
            with open(kwargs["raw_path"], "rb") as fh:
                seen["palette_raw"] = fh.read()
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        with patch("handler_render_lores_preview.report_status") as mock_report:
            resp = handler(_event(
                artifact_sculpture={"artifact_id": "color_run_abc"},
                preview_source_size=4,
                sculpture_task_id="sculpture_artifact_9",
            ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        sc = body["sculpture"]
        self.assertEqual(sc["format"], "u16")
        self.assertEqual(sc["grid_n"], 4)
        self.assertEqual(sc["degree"], 1)
        self.assertEqual(sc["step_count"], 16)
        self.assertEqual(sc["source_artifact_id"], "color_run_abc")
        self.assertEqual(sc["viewport"], {"min_re": -2.0, "max_re": 2.0, "min_im": -2.0, "max_im": 2.0})
        self.assertIn("/renders/j/sculpture_roots.bin?v=", sc["roots_url"])
        # the raster ran with the ARTIFACT's provenance, not request params
        self.assertIn("--rotation=0.25", seen["cmd"])
        self.assertIn("--min_re=-2.0", seen["cmd"])
        self.assertIn("--xformed_roots_format=u16", seen["cmd"])
        self.assertEqual(seen["xforms"], [{"fn": 30}])
        # stored solve-order scores -> row-major palette raw: odd solve rows
        # walk right-to-left (serpentine), so rows 1 and 3 reverse
        self.assertEqual(
            list(seen["palette_raw"]),
            [0, 1, 2, 3, 7, 6, 5, 4, 8, 9, 10, 11, 15, 14, 13, 12])
        self.assertEqual(seen["render"]["palette"], "viridis")
        self.assertEqual(seen["render"]["background_color"], "101010")
        self.assertEqual(seen["render"]["pix"], 4)
        self.assertEqual(seen["render"]["channels"], 1)
        self.assertEqual(seen["render"]["interpretation"], "scalar_lut")
        self.assertFalse(seen["render"]["zero_background"])
        by_key = {call.kwargs["Key"]: call.kwargs for call in mock_s3.put_object.call_args_list}
        cache_json_keys = [k for k in by_key if k.startswith("renders/j/sculpture_cache/") and k.endswith("sculpture.json")]
        self.assertEqual(len(cache_json_keys), 1)   # the reuse index rode along
        # the cache namespace is SCHEMA-VERSIONED (CR: entries a buggy build
        # may have poisoned live under the old signatures — v2 orphans them)
        import hashlib as _hashlib
        sig2 = json.dumps({
            "cache_schema": 2,
            "artifact_id": "color_run_abc",
            "view_n": 4,
            "format": "u16",
            "step_scores_key": "renders/j/step_scores.raw",
            "grid_n": 4,
            "channels": 1,
            "interpretation": "scalar_lut",
            "palette": "viridis",
            "background_color": "101010",
            "rotation": 0.25,
            "viewport": {"min_re": -2.0, "max_re": 2.0, "min_im": -2.0, "max_im": 2.0},
            "root_transforms": [{"fn": 30}],
        }, sort_keys=True, separators=(",", ":"))
        expected_prefix = f"renders/j/sculpture_cache/{_hashlib.sha1(sig2.encode('utf-8')).hexdigest()[:16]}/"
        self.assertEqual(cache_json_keys[0], expected_prefix + "sculpture.json")
        cached_block = json.loads(by_key[cache_json_keys[0]]["Body"])
        self.assertEqual(cached_block["source_artifact_id"], "color_run_abc")
        self.assertNotIn("roots_url", cached_block)   # stamps rebuilt per hit
        self.assertEqual(
            {k for k in set(by_key) - set(cache_json_keys) if "/sculpture_cache/" not in k},
            {"renders/j/sculpture_roots.bin", "renders/j/sculpture_palette.png"})
        self.assertEqual(by_key["renders/j/sculpture_roots.bin"]["Body"], xformed)
        self.assertEqual(by_key["renders/j/sculpture_roots.bin"]["CacheControl"], "no-cache")
        self.assertEqual(by_key["renders/j/sculpture_palette.png"]["ContentType"], "image/png")
        # the cache binaries are UPLOADED from this invocation's local bytes
        # — never copied through the job-wide mutable keys (CR: a concurrent
        # same-job run could swap them mid-copy and poison the cache)
        cache_prefix = cache_json_keys[0].rsplit("/", 1)[0] + "/"
        self.assertEqual(mock_s3.copy_object.call_args_list, [])
        cache_puts = [c.kwargs["Key"] for c in mock_s3.put_object.call_args_list
                      if c.kwargs["Key"].startswith(cache_prefix)]
        self.assertEqual({k.rsplit("/", 1)[-1] for k in cache_puts},
                         {"roots.bin", "palette.png", "sculpture.json"})
        self.assertEqual(cache_puts[-1], cache_prefix + "sculpture.json")   # commit marker LAST
        self.assertEqual(by_key[cache_prefix + "roots.bin"]["Body"], xformed)
        self.assertEqual(body["cache"], {"hit": False, "prefix": cache_prefix})
        calls = mock_report.call_args_list
        self.assertEqual(calls[0].args, ("j", "sculpture_artifact_9", "running"))
        self.assertEqual(calls[-1].args, ("j", "sculpture_artifact_9", "done"))
        done = calls[-1].kwargs["result_data"]
        self.assertEqual(done["sculpture"]["source_artifact_id"], "color_run_abc")

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_save_full_persists_from_local_bytes_before_done(self, mock_s3, mock_run, mock_render):
        # CR: SaveFull must NEVER copy the job-wide mutable sculpture_* keys
        # — on a fresh run the durable save is uploaded from the bytes THIS
        # invocation produced, and the task reports done only after.
        from handler_render_lores_preview import TMP_XFORMED_ROOTS, handler

        calc, roots_key, roots_bytes = self._artifact_calc(4)
        step_scores = bytes(range(16))
        mock_s3.get_object.side_effect = self._artifact_get_object(calc, roots_key, roots_bytes, step_scores)
        mock_s3.head_object.side_effect = self._artifact_head_router()
        xformed = b"T" * (16 * 1 * 2 * 2)

        def subprocess_fake(cmd, **kwargs):
            with open(TMP_XFORMED_ROOTS, "wb") as fh:
                fh.write(xformed)
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 16, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        with patch("handler_render_lores_preview.report_status") as mock_report:
            resp = handler(_event(
                artifact_sculpture={"artifact_id": "color_run_abc"},
                preview_source_size=4,
                sculpture_task_id="sculpture_artifact_9",
                save_full={"view": {"point": 20, "style": "ghost", "zaxis": "t1",
                                    "junk": "x", "show": {"points": True, "clu": True}},
                           "title": ""},
            ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        saved = body["saved_sculpture"]
        self.assertTrue(saved["id"].startswith("scu_"))
        # collision-proof id: ms base36 + 6 random b36 chars (CR: same-ms
        # Lambdas must not share a global prefix)
        self.assertGreaterEqual(len(saved["id"]), 4 + 8 + 6)
        self.assertEqual(saved["title"], saved["id"])          # ids name saves
        self.assertEqual(saved["grid_n"], 4)
        self.assertEqual(saved["format"], "u16")
        self.assertEqual(saved["source_artifact_id"], "color_run_abc")
        self.assertEqual(saved["view"]["point"], 20)
        self.assertEqual(saved["view"]["style"], "ghost")
        self.assertEqual(saved["view"]["zaxis"], "t1")
        self.assertNotIn("junk", saved["view"])                # whitelisted
        self.assertIn(f"/sculptures/{saved['id']}/viewer.html", saved["share_url"])
        # the durable objects were UPLOADED (local bytes), not server-copied
        # from the mutable job keys
        sprefix = f"sculptures/{saved['id']}/"
        put_keys = {c.kwargs["Key"]: c.kwargs for c in mock_s3.put_object.call_args_list
                    if c.kwargs["Key"].startswith(sprefix)}
        self.assertEqual(set(put_keys), {sprefix + "roots.bin", sprefix + "palette.png",
                                         sprefix + "meta.json", sprefix + "viewer.html"})
        self.assertEqual(put_keys[sprefix + "roots.bin"]["Body"], xformed)
        copy_keys = {c.kwargs["Key"] for c in mock_s3.copy_object.call_args_list}
        self.assertFalse({k for k in copy_keys if k.startswith("sculptures/")})
        # the CACHE too is written from local bytes — never copied through
        # the job-wide mutable keys a concurrent run can swap (CR)
        self.assertFalse({k for k in copy_keys if "/sculpture_cache/" in k})
        cache_puts = [c.kwargs["Key"] for c in mock_s3.put_object.call_args_list
                      if "/sculpture_cache/" in c.kwargs["Key"]]
        self.assertEqual(cache_puts[-1].rsplit("/", 1)[-1], "sculpture.json")   # commit marker LAST
        self.assertEqual({k.rsplit("/", 1)[-1] for k in cache_puts},
                         {"roots.bin", "palette.png", "sculpture.json"})
        # publication order: meta.json (the listing's row marker) goes LAST
        sprefix_puts = [c.kwargs["Key"] for c in mock_s3.put_object.call_args_list
                        if c.kwargs["Key"].startswith(sprefix)]
        self.assertEqual(sprefix_puts[-1], sprefix + "meta.json")
        self.assertLess(sprefix_puts.index(sprefix + "viewer.html"),
                        sprefix_puts.index(sprefix + "meta.json"))
        meta_row = json.loads(put_keys[sprefix + "meta.json"]["Body"])
        self.assertEqual(meta_row["view"]["show"], {"points": True, "ribbons": False,
                                                    "threads": False, "clu": True, "splats": False})
        # done reported AFTER the save, carrying the saved row
        done = mock_report.call_args_list[-1]
        self.assertEqual(done.args[2], "done")
        self.assertEqual(done.kwargs["result_data"]["saved_sculpture"]["id"], saved["id"])

    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_save_full_on_cache_hit_copies_the_immutable_cache(self, mock_s3, mock_run):
        # cache hit: no local bytes exist — the save copies from the
        # content-addressed cache prefix, never the mutable job keys
        from handler_render_lores_preview import handler

        cached_block = {
            "format": "u16", "grid_n": 4, "degree": 1, "step_count": 16,
            "pass_count": 1, "roots_bytes": 64,
            "viewport": {"min_re": -2.0, "max_re": 2.0, "min_im": -2.0, "max_im": 2.0},
            "palette": "viridis", "source_artifact_id": "color_run_abc",
            "step_scores_key": "renders/j/step_scores.raw",
        }
        calc, roots_key, roots_bytes = self._artifact_calc(4)

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key.endswith("sculpture_cache") or "sculpture_cache" in key and key.endswith("sculpture.json"):
                body = MagicMock()
                body.read.return_value = json.dumps(cached_block).encode("utf-8")
                return {"Body": body}
            return self._artifact_get_object(calc, roots_key, roots_bytes, b"")(**kwargs)

        mock_s3.get_object.side_effect = get_object
        mock_s3.head_object.side_effect = self._artifact_head_router()
        with patch("handler_render_lores_preview.report_status") as mock_report:
            resp = handler(_event(
                artifact_sculpture={"artifact_id": "color_run_abc"},
                preview_source_size=4,
                sculpture_task_id="sculpture_artifact_10",
                save_full={"view": {"point": 12}},
            ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        self.assertTrue(body["cache"]["hit"])
        saved = body["saved_sculpture"]
        sprefix = f"sculptures/{saved['id']}/"
        copies = {c.kwargs["Key"]: c.kwargs["CopySource"]["Key"]
                  for c in mock_s3.copy_object.call_args_list
                  if c.kwargs["Key"].startswith(sprefix)}
        self.assertEqual(set(copies), {sprefix + "roots.bin", sprefix + "palette.png"})
        for src in copies.values():
            self.assertIn("/sculpture_cache/", src)            # immutable source
            self.assertNotIn("sculpture_roots.bin", src)
            self.assertNotIn("sculpture_palette.png", src)
        done = mock_report.call_args_list[-1]
        self.assertEqual(done.args[2], "done")
        self.assertEqual(done.kwargs["result_data"]["saved_sculpture"]["id"], saved["id"])

    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_cache_hit_save_failures_propagate_instead_of_regenerating(self, mock_s3, mock_run):
        # CR: the cache-hit fallback catch must cover ONLY cache loading —
        # a failure AFTER the hit (save or terminal status) must propagate,
        # never silently rerun the full generation (which could mint a
        # second sculpture after a successful save).
        from handler_render_lores_preview import handler

        cached_block = {
            "format": "u16", "grid_n": 4, "degree": 1, "step_count": 16,
            "pass_count": 1, "roots_bytes": 64,
            "viewport": {"min_re": -2.0, "max_re": 2.0, "min_im": -2.0, "max_im": 2.0},
            "palette": "viridis", "source_artifact_id": "color_run_abc",
        }
        calc, roots_key, roots_bytes = self._artifact_calc(4)
        calc_fetches = {"count": 0}

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key == "renders/j/calc.json":
                calc_fetches["count"] += 1
            if "sculpture_cache" in key and key.endswith("sculpture.json"):
                body = MagicMock()
                body.read.return_value = json.dumps(cached_block).encode("utf-8")
                return {"Body": body}
            return self._artifact_get_object(calc, roots_key, roots_bytes, b"")(**kwargs)

        mock_s3.get_object.side_effect = get_object
        mock_s3.head_object.side_effect = self._artifact_head_router()
        with patch("handler_render_lores_preview.report_status") as mock_report:
            # the DONE write fails after a successful cache-hit save
            mock_report.side_effect = [None, RuntimeError("ddb write failed")]
            resp = handler(_event(
                artifact_sculpture={"artifact_id": "color_run_abc"},
                preview_source_size=4,
                sculpture_task_id="sculpture_artifact_11",
                save_full={"view": {"point": 12}},
            ), None)
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("ddb write failed", json.loads(resp["body"])["detail"])
        self.assertEqual(calc_fetches["count"], 0)     # NO regeneration

    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_save_full_fails_closed_on_id_collision(self, mock_s3, mock_run):
        # CR: two Lambdas in the same millisecond must never share a global
        # sculptures/ prefix — an existing publication marker is an ERROR
        from handler_render_lores_preview import handler

        cached_block = {
            "format": "u16", "grid_n": 4, "degree": 1, "step_count": 16,
            "pass_count": 1, "roots_bytes": 64,
            "viewport": {"min_re": -2.0, "max_re": 2.0, "min_im": -2.0, "max_im": 2.0},
            "palette": "viridis", "source_artifact_id": "color_run_abc",
        }
        calc, roots_key, roots_bytes = self._artifact_calc(4)

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if "sculpture_cache" in key and key.endswith("sculpture.json"):
                body = MagicMock()
                body.read.return_value = json.dumps(cached_block).encode("utf-8")
                return {"Body": body}
            return self._artifact_get_object(calc, roots_key, roots_bytes, b"")(**kwargs)

        mock_s3.get_object.side_effect = get_object
        # every sculptures/ marker head SUCCEEDS -> the prefix is taken
        mock_s3.head_object.return_value = self._artifact_head()
        with patch("handler_render_lores_preview.report_status"):
            resp = handler(_event(
                artifact_sculpture={"artifact_id": "color_run_abc"},
                preview_source_size=4,
                save_full={"view": {"point": 12}},
            ), None)
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("collision", json.loads(resp["body"])["detail"])
        # nothing was written under the taken prefix
        self.assertFalse([c for c in mock_s3.put_object.call_args_list
                          if c.kwargs["Key"].startswith("sculptures/")])

    def _ambiguous_meta_put_scenario(self, mock_s3, mock_run, mock_render, *,
                                     marker_read, delete_errors=None, fail_at="meta"):
        # shared harness: SaveFull where a publication PUT raises; marker_read
        # decides what the resolving read-back sees (fail_at="viewer" models
        # the marker-never-attempted branch)
        from handler_render_lores_preview import TMP_XFORMED_ROOTS, handler

        calc, roots_key, roots_bytes = self._artifact_calc(4)
        base_get = self._artifact_get_object(calc, roots_key, roots_bytes, bytes(range(16)))
        written = {}

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if str(key).startswith("sculptures/") and str(key).endswith("meta.json"):
                if marker_read == "landed":
                    body = MagicMock()
                    body.read.return_value = written.get(key, b"")
                    return {"Body": body}
                if marker_read == "absent":
                    raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "gone"}}, "GetObject")
                raise ClientError({"Error": {"Code": "SlowDown", "Message": "throttle"}}, "GetObject")
            return base_get(**kwargs)

        def put_object(Bucket=None, Key=None, Body=b"", **kw):
            if fail_at == "viewer" and str(Key).startswith("sculptures/") and str(Key).endswith("viewer.html"):
                raise RuntimeError("viewer upload failed")
            if fail_at == "meta" and str(Key).startswith("sculptures/") and str(Key).endswith("meta.json"):
                written[Key] = Body        # the PUT "fails" but may have landed
                raise RuntimeError("socket timeout mid-PUT")
            return {}

        def subprocess_fake(cmd, **kwargs):
            with open(TMP_XFORMED_ROOTS, "wb") as fh:
                fh.write(b"T" * (16 * 1 * 2 * 2))
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 16, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_s3.get_object.side_effect = get_object
        mock_s3.head_object.side_effect = self._artifact_head_router()
        mock_s3.put_object.side_effect = put_object
        mock_s3.delete_objects.return_value = {"Errors": delete_errors or []}
        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake
        with patch("handler_render_lores_preview.report_status"):
            return handler(_event(
                artifact_sculpture={"artifact_id": "color_run_abc"},
                preview_source_size=4,
                save_full={"view": {"point": 12}},
            ), None)

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_ambiguous_marker_put_that_landed_is_a_success(self, mock_s3, mock_run, mock_render):
        # CR: resolve an ambiguous marker PUT by reading it back — the exact
        # bytes present means the publication COMPLETED; report success and
        # delete nothing
        resp = self._ambiguous_meta_put_scenario(mock_s3, mock_run, mock_render,
                                                 marker_read="landed")
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        self.assertTrue(body["saved_sculpture"]["id"].startswith("scu_"))
        mock_s3.delete_objects.assert_not_called()

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_absent_at_read_still_preserves_the_payload(self, mock_s3, mock_run, mock_render):
        # CR (ambiguous-write invariant): a 404 read-back does NOT prove a
        # timed-out marker PUT will never land — it may complete AFTER the
        # read and after any cleanup, recreating marker-over-deleted-files.
        # Once the marker was ATTEMPTED, anything but the exact read-back
        # preserves everything.
        resp = self._ambiguous_meta_put_scenario(mock_s3, mock_run, mock_render,
                                                 marker_read="absent")
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("PRESERVED", json.loads(resp["body"])["detail"])
        mock_s3.delete_objects.assert_not_called()

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_unknown_marker_state_preserves_everything(self, mock_s3, mock_run, mock_render):
        # CR: when the resolving read fails non-404 the marker state is
        # UNKNOWN — deleting payload could orphan a live marker; preserve
        resp = self._ambiguous_meta_put_scenario(mock_s3, mock_run, mock_render,
                                                 marker_read="unknown")
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("PRESERVED", json.loads(resp["body"])["detail"])
        mock_s3.delete_objects.assert_not_called()

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_never_attempted_marker_cleans_payload_only_and_checks_errors(self, mock_s3, mock_run, mock_render):
        # cleanup is legal ONLY when the marker PUT was never attempted (a
        # viewer failure) — it sweeps exactly the payload, never the marker
        # key, and per-key DeleteObjects Errors surface instead of passing
        resp = self._ambiguous_meta_put_scenario(mock_s3, mock_run, mock_render,
                                                 marker_read="absent", fail_at="viewer")
        self.assertEqual(resp["statusCode"], 500)
        calls = [ [o["Key"].rsplit("/", 1)[-1] for o in c.kwargs["Delete"]["Objects"]]
                  for c in mock_s3.delete_objects.call_args_list ]
        self.assertEqual(calls, [["roots.bin", "palette.png", "viewer.html"]])

        mock_s3.reset_mock()
        resp = self._ambiguous_meta_put_scenario(
            mock_s3, mock_run, mock_render, marker_read="absent", fail_at="viewer",
            delete_errors=[{"Key": "roots.bin", "Code": "InternalError"}])
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("could not delete", json.loads(resp["body"])["detail"])

    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_mint_propagates_non_404_head_errors(self, mock_s3, mock_run):
        # CR28-F13 taxonomy: a throttle during the availability HEAD is NOT
        # "available" — proceeding would reopen the overwrite window
        from handler_render_lores_preview import handler

        cached_block = {
            "format": "u16", "grid_n": 4, "degree": 1, "step_count": 16,
            "pass_count": 1, "roots_bytes": 64,
            "viewport": {"min_re": -2.0, "max_re": 2.0, "min_im": -2.0, "max_im": 2.0},
            "palette": "viridis", "source_artifact_id": "color_run_abc",
        }
        calc, roots_key, roots_bytes = self._artifact_calc(4)

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if "sculpture_cache" in key and key.endswith("sculpture.json"):
                body = MagicMock()
                body.read.return_value = json.dumps(cached_block).encode("utf-8")
                return {"Body": body}
            return self._artifact_get_object(calc, roots_key, roots_bytes, b"")(**kwargs)

        head = self._artifact_head()

        def head_object(Bucket=None, Key=None, **kw):
            if str(Key or "").startswith("sculptures/"):
                raise ClientError({"Error": {"Code": "SlowDown", "Message": "throttle"}}, "HeadObject")
            return head

        mock_s3.get_object.side_effect = get_object
        mock_s3.head_object.side_effect = head_object
        with patch("handler_render_lores_preview.report_status"):
            resp = handler(_event(
                artifact_sculpture={"artifact_id": "color_run_abc"},
                preview_source_size=4,
                save_full={"view": {"point": 12}},
            ), None)
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("could not verify", json.loads(resp["body"])["detail"])
        self.assertFalse([c for c in mock_s3.put_object.call_args_list
                          if c.kwargs.get("Key", "").startswith("sculptures/")])

    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_splat_bake_cache_hit_generate_reads_the_immutable_cache(self, mock_s3, mock_run):
        # CR: when the composed generate cache-HITS, the bake has no local
        # files — its fallback must read the content-addressed prefix,
        # never the job-wide mutable sculpture_* keys
        from handler_render_lores_preview import TMP_PALETTE_IMAGE, TMP_XFORMED_ROOTS, handler
        import sys
        sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))
        from png_rgb import encode_png_rgb

        for stale in (TMP_XFORMED_ROOTS, TMP_PALETTE_IMAGE):
            if os.path.exists(stale):
                os.remove(stale)
        cached_block = {
            "format": "u16", "grid_n": 4, "degree": 2, "step_count": 16,
            "pass_count": 1, "roots_bytes": 128,
            "viewport": {"min_re": -1.0, "max_re": 1.0, "min_im": -1.0, "max_im": 1.0},
            "palette": "viridis", "source_artifact_id": "color_run_abc",
        }
        calc, roots_key, roots_bytes = self._artifact_calc(4)
        fetched = []
        palette_png = encode_png_rgb(4, 4, bytes([10, 20, 30] * 16))
        roots_blob = b"R" * 128

        def get_object(**kwargs):
            key = kwargs.get("Key")
            fetched.append(key)
            if "sculpture_cache" in key:
                if key.endswith("sculpture.json"):
                    body = MagicMock()
                    body.read.return_value = json.dumps(cached_block).encode("utf-8")
                    return {"Body": body}
                if key.endswith("roots.bin"):
                    return {"Body": _ChunkBody(roots_blob)}
                if key.endswith("palette.png"):
                    return {"Body": _ChunkBody(palette_png)}
            return self._artifact_get_object(calc, roots_key, roots_bytes, b"")(**kwargs)

        mock_s3.get_object.side_effect = get_object
        mock_s3.head_object.side_effect = self._artifact_head_router()
        seen = {}
        mock_run.side_effect = self._bake_tool_fake(lambda cmd: seen.update(cmd=list(cmd)))
        with patch("handler_render_lores_preview.report_status"):
            resp = handler(_event(
                splat_bake={"source": {"kind": "artifact", "artifact_id": "color_run_abc", "n": 4},
                            "params": {"res": 64}},
                sculpture_task_id="splat_bake_9",
            ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        cache_reads = [k for k in fetched if "/sculpture_cache/" in k]
        self.assertTrue(any(k.endswith("roots.bin") for k in cache_reads))
        self.assertTrue(any(k.endswith("palette.png") for k in cache_reads))
        self.assertNotIn("renders/j/sculpture_roots.bin", fetched)
        self.assertNotIn("renders/j/sculpture_palette.png", fetched)

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_artifact_sculpture_subsamples_scores_on_the_logical_lattice(self, mock_s3, mock_run, mock_render):
        # view 2 over grid 4: the score subsample must pick EXACTLY the
        # solves the roots materializer walks (floor(i*full/view) rows and
        # serpentine-corrected columns) — solve indices 0,2 (row 0) and
        # 8,10 (row 2), de-serpentined row-major
        from handler_render_lores_preview import TMP_XFORMED_ROOTS, handler

        calc, roots_key, roots_bytes = self._artifact_calc(4)
        step_scores = bytes(range(16))
        mock_s3.get_object.side_effect = self._artifact_get_object(calc, roots_key, roots_bytes, step_scores)
        mock_s3.head_object.return_value = self._artifact_head()
        seen = {}

        def subprocess_fake(cmd, **kwargs):
            with open(TMP_XFORMED_ROOTS, "wb") as fh:
                fh.write(b"Q" * (4 * 1 * 2 * 2))   # 2x2 solves, u16
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 4, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["raw_path"], "rb") as fh:
                seen["palette_raw"] = fh.read()
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(
            artifact_sculpture={"artifact_id": "color_run_abc"},
            preview_source_size=2,
        ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        self.assertEqual(body["sculpture"]["grid_n"], 2)
        self.assertEqual(body["sculpture"]["step_count"], 4)
        self.assertEqual(list(seen["palette_raw"]), [0, 2, 8, 10])

    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_artifact_sculpture_cache_hit_serves_without_solving(self, mock_s3, mock_run):
        # a prior run's sculpture.json under the content-hash prefix means:
        # no calc read, no materialization, no raster — just server-side
        # copies into the fixed keys and freshly stamped URLs
        from handler_render_lores_preview import handler

        cached_block = {
            "format": "u16", "grid_n": 4, "degree": 1, "step_count": 16,
            "pass_count": 1, "roots_bytes": 64,
            "viewport": {"min_re": -2.0, "max_re": 2.0, "min_im": -2.0, "max_im": 2.0},
            "palette": "viridis", "source_artifact_id": "color_run_abc",
        }

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key.startswith("renders/j/sculpture_cache/") and key.endswith("sculpture.json"):
                return {"Body": _ChunkBody(json.dumps(cached_block).encode("utf-8"))}
            if key == "renders/j/color/color_run_abc/meta.json":
                return {"Body": _ChunkBody(b"{}")}
            raise AssertionError(f"unexpected key: {key}")

        mock_s3.get_object.side_effect = get_object
        mock_s3.head_object.return_value = self._artifact_head()

        with patch("handler_render_lores_preview.report_status") as mock_report:
            resp = handler(_event(
                artifact_sculpture={"artifact_id": "color_run_abc"},
                preview_source_size=4,
                sculpture_task_id="sculpture_artifact_9",
            ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        self.assertTrue(body["cache"]["hit"])
        sc = body["sculpture"]
        self.assertEqual(sc["source_artifact_id"], "color_run_abc")
        self.assertEqual(sc["roots_key"], "renders/j/sculpture_roots.bin")
        self.assertIn("/renders/j/sculpture_roots.bin?v=", sc["roots_url"])
        mock_run.assert_not_called()               # no raster
        mock_s3.put_object.assert_not_called()     # no uploads — copies only
        copies = {c.kwargs["Key"]: c.kwargs["CopySource"]["Key"] for c in mock_s3.copy_object.call_args_list}
        self.assertEqual(set(copies), {"renders/j/sculpture_roots.bin", "renders/j/sculpture_palette.png"})
        self.assertTrue(copies["renders/j/sculpture_roots.bin"].endswith("/roots.bin"))
        done = mock_report.call_args_list[-1]
        self.assertEqual(done.args, ("j", "sculpture_artifact_9", "done"))
        self.assertTrue(done.kwargs["result_data"]["cache_hit"])

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_artifact_sculpture_cache_key_tracks_the_palette(self, mock_s3, mock_run, mock_render):
        # the signature covers the palette: a repalette of the SAME
        # artifact_id must hash to a DIFFERENT cache prefix (so it rebuilds
        # instead of serving the old colors) — run once per palette and pin
        # that the probed prefixes differ
        from handler_render_lores_preview import TMP_XFORMED_ROOTS, handler

        calc, roots_key, roots_bytes = self._artifact_calc(4)
        base_get = self._artifact_get_object(calc, roots_key, roots_bytes, bytes(range(16)))
        probes = []

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key.startswith("renders/j/sculpture_cache/"):
                probes.append(key)
                raise AssertionError("miss")   # nothing cached under any hash
            return base_get(**kwargs)

        mock_s3.get_object.side_effect = get_object

        def subprocess_fake(cmd, **kwargs):
            with open(TMP_XFORMED_ROOTS, "wb") as fh:
                fh.write(b"T" * (16 * 1 * 2 * 2))
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 16, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        prefixes = []
        for palette in ("viridis", "magma"):
            probes.clear()
            mock_s3.head_object.return_value = self._artifact_head(palette=palette)
            resp = handler(_event(
                artifact_sculpture={"artifact_id": "color_run_abc"},
                preview_source_size=4,
            ), None)
            self.assertEqual(resp["statusCode"], 200, resp["body"])
            body = json.loads(resp["body"])
            self.assertFalse(body["cache"]["hit"])
            self.assertEqual(len(probes), 1)
            prefixes.append(body["cache"]["prefix"])
        self.assertNotEqual(prefixes[0], prefixes[1])   # palette is IN the key

    @patch("handler_render_lores_preview.report_status")
    @patch("handler_render_lores_preview.s3")
    def test_artifact_sculpture_without_step_scores_reports_error(self, mock_s3, mock_report):
        # artifacts predating step-scores storage fail with the re-render
        # message, and the async row goes running -> error
        from handler_render_lores_preview import handler

        mock_s3.head_object.return_value = self._artifact_head(step_scores_key="")
        resp = handler(_event(
            artifact_sculpture={"artifact_id": "color_run_abc"},
            preview_source_size=4,
            sculpture_task_id="sculpture_artifact_9",
        ), None)
        self.assertEqual(resp["statusCode"], 500)
        body = json.loads(resp["body"])
        self.assertIn("re-render", body["detail"])
        calls = mock_report.call_args_list
        self.assertEqual(calls[0].args, ("j", "sculpture_artifact_9", "running"))
        self.assertEqual(calls[-1].args, ("j", "sculpture_artifact_9", "error"))
        self.assertIn("re-render", calls[-1].kwargs["error_msg"])

    @patch("handler_render_lores_preview.s3")
    def test_artifact_sculpture_rejects_view_beyond_the_solve_grid(self, mock_s3):
        from handler_render_lores_preview import handler

        mock_s3.head_object.return_value = self._artifact_head()
        mock_s3.get_object.side_effect = self._artifact_get_object(*self._artifact_calc(4), b"")
        resp = handler(_event(
            artifact_sculpture={"artifact_id": "color_run_abc"},
            preview_source_size=8,
        ), None)
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("exceeds the solve grid", json.loads(resp["body"])["detail"])

    @staticmethod
    def _bake_saved_get_object(meta=None):
        import sys
        sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))
        from png_rgb import encode_png_rgb
        saved_meta = {
            "version": 1, "id": "scu_src1", "title": "Saved Piece", "job_id": "j",
            "grid_n": 4, "degree": 2, "format": "u16",
            "viewport": {"min_re": -1.0, "max_re": 1.0, "min_im": -1.0, "max_im": 1.0},
            "source_artifact_id": "color_run_abc",
        }
        if meta:
            saved_meta.update(meta)
        png = encode_png_rgb(4, 4, bytes([10, 20, 30] * 16))
        roots = b"\x00" * (4 * 4 * 2 * 2 * 2)

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key == "sculptures/scu_src1/meta.json":
                return {"Body": _ChunkBody(json.dumps(saved_meta).encode("utf-8"))}
            if key == "sculptures/scu_src1/roots.bin":
                return {"Body": _ChunkBody(roots)}
            if key == "sculptures/scu_src1/palette.png":
                return {"Body": _ChunkBody(png)}
            raise AssertionError(f"unexpected key: {key}")
        return get_object

    @staticmethod
    def _bake_tool_fake(assertions=None):
        def subprocess_fake(cmd, **kwargs):
            self_check = assertions or (lambda c: None)
            self_check(cmd)
            out_arg = next(a for a in cmd if a.startswith("--out="))
            with open(out_arg.split("=", 1)[1], "wb") as fh:
                fh.write(b"P" * (22 * 2))
            return MagicMock(returncode=0, stdout=json.dumps({
                "count": 2, "points_used": 4, "points_clipped": 0,
                "cmin": [0, 0, 0], "cmax": [1, 1, 1], "amax": 0.01,
            }), stderr="")
        return subprocess_fake

    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_splat_bake_saved_source_mints_a_hosted_viewer(self, mock_s3, mock_run):
        from handler_render_lores_preview import handler

        mock_s3.get_object.side_effect = self._bake_saved_get_object()
        mock_s3.head_object.side_effect = self._artifact_head_router()
        seen = {}

        def check(cmd):
            seen["cmd"] = list(cmd)
        mock_run.side_effect = self._bake_tool_fake(check)

        with patch("handler_render_lores_preview.report_status") as mock_report:
            resp = handler(_event(
                splat_bake={
                    "source": {"kind": "saved", "saved_id": "scu_src1"},
                    "params": {"res": 64, "zaxis": "t2", "slices": 2, "mode": 0,
                               "intensity": 2, "yscale": 0.5, "scalemul": 2,
                               "cam": [0, 0, 1.5], "tour": "orbit", "tourSpeed": 2,
                               "title": "My <Bake> & Co"},
                },
                sculpture_task_id="splat_bake_7",
            ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        row = body["sculpture"]
        self.assertEqual(row["kind"], "splatbake")
        self.assertEqual(row["splat_count"], 2)
        self.assertEqual(row["title"], "My <Bake> & Co")
        self.assertEqual(row["source_saved_id"], "scu_src1")
        self.assertEqual(row["source_artifact_id"], "color_run_abc")
        self.assertTrue(row["share_url"].endswith(f"/sculptures/{row['id']}/viewer.html"))
        # the C tool ran with the SAVED row's geometry + the sanitized params
        self.assertIn("--roots_format=u16", seen["cmd"])
        self.assertIn("--grid_n=4", seen["cmd"])
        self.assertIn("--res=64", seen["cmd"])
        self.assertIn("--slices=2", seen["cmd"])
        self.assertIn("--yscale=0.5", seen["cmd"])
        self.assertIn("--scalemul=2.0", seen["cmd"])
        # the hosted page: immutable single object + list-row meta
        puts = {c.kwargs["Key"]: c.kwargs for c in mock_s3.put_object.call_args_list}
        viewer_key = f"sculptures/{row['id']}/viewer.html"
        self.assertIn(viewer_key, puts)
        self.assertEqual(puts[viewer_key]["CacheControl"], "public, max-age=31536000, immutable")
        html = puts[viewer_key]["Body"].decode("utf-8")
        self.assertIn("My &lt;Bake> &amp; Co", html)          # title entity-escaped
        self.assertIn('"tour": "orbit"'.replace(" ", ""), html.replace(" ", ""))
        self.assertIn('var B64 = "' + ("UFBQ"[0:2]), html)     # pack embedded
        self.assertNotIn("__HEADER_JSON__", html)
        meta = json.loads(puts[f"sculptures/{row['id']}/meta.json"]["Body"])
        self.assertEqual(meta["kind"], "splatbake")
        self.assertEqual(meta["bake_params"]["res"], 64)
        calls = mock_report.call_args_list
        self.assertEqual(calls[0].args, ("j", "splat_bake_7", "running"))
        self.assertEqual(calls[-1].args, ("j", "splat_bake_7", "done"))
        self.assertEqual(calls[-1].kwargs["result_data"]["sculpture"]["kind"], "splatbake")

    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_splat_bake_cache_source_binds_to_the_job(self, mock_s3, mock_run):
        from handler_render_lores_preview import handler

        mock_s3.head_object.side_effect = self._artifact_head_router()
        import sys
        sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))
        from png_rgb import encode_png_rgb

        prefix = "renders/j/sculpture_cache/0123456789abcdef/"
        block = {"grid_n": 4, "degree": 2, "format": "u16",
                 "viewport": {"min_re": -1.0, "max_re": 1.0, "min_im": -1.0, "max_im": 1.0},
                 "source_artifact_id": "color_run_abc"}

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key == prefix + "sculpture.json":
                return {"Body": _ChunkBody(json.dumps(block).encode("utf-8"))}
            if key == prefix + "roots.bin":
                return {"Body": _ChunkBody(b"\x00" * (4 * 4 * 2 * 2 * 2))}
            if key == prefix + "palette.png":
                return {"Body": _ChunkBody(encode_png_rgb(4, 4, bytes([1, 2, 3] * 16)))}
            raise AssertionError(f"unexpected key: {key}")
        mock_s3.get_object.side_effect = get_object
        mock_run.side_effect = self._bake_tool_fake()

        resp = handler(_event(splat_bake={
            "source": {"kind": "cache", "cache_prefix": prefix}, "params": {},
        }), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        row = json.loads(resp["body"])["sculpture"]
        self.assertEqual(row["source_artifact_id"], "color_run_abc")

        # another job's prefix must be refused
        resp = handler(_event(splat_bake={
            "source": {"kind": "cache",
                       "cache_prefix": "renders/OTHER/sculpture_cache/0123456789abcdef/"},
        }), None)
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("this job's cache prefix", json.loads(resp["body"])["detail"])

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_splat_bake_artifact_source_chains_the_generate_core(self, mock_s3, mock_run, mock_render):
        # parameters -> baked share in ONE job: the bake runs the artifact
        # generate first (real code path), then bakes from the fixed keys
        # the generate just refreshed
        from handler_render_lores_preview import TMP_XFORMED_ROOTS, handler
        import sys
        sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))
        from png_rgb import encode_png_rgb

        calc, roots_key, roots_bytes = self._artifact_calc(4)
        base_get = self._artifact_get_object(calc, roots_key, roots_bytes, bytes(range(16)))
        uploaded = {}

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key in uploaded:
                return {"Body": _ChunkBody(uploaded[key])}
            if key.startswith("renders/j/sculpture_cache/"):
                raise AssertionError("miss")
            return base_get(**kwargs)

        def put_object(**kwargs):
            body = kwargs.get("Body")
            uploaded[kwargs["Key"]] = body if isinstance(body, bytes) else bytes(body)
            return {}
        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.head_object.side_effect = self._artifact_head_router()

        def subprocess_fake(cmd, **kwargs):
            if any(a.startswith("--xformed_roots_output") for a in cmd):
                with open(TMP_XFORMED_ROOTS, "wb") as fh:
                    fh.write(b"T" * (16 * 1 * 2 * 2))
                return MagicMock(returncode=0, stdout=json.dumps(
                    {"roots_plotted": 16, "roots_clipped": 0}), stderr="")
            return self._bake_tool_fake()(cmd, **kwargs)
        mock_run.side_effect = subprocess_fake

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(encode_png_rgb(4, 4, bytes([10, 20, 30] * 16)))
            return {"file_size": 100, "preview_file_size": 0}
        mock_render.side_effect = render_fake

        resp = handler(_event(splat_bake={
            "source": {"kind": "artifact", "artifact_id": "color_run_abc", "n": 4},
            "params": {"res": 96},
        }), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body2 = json.loads(resp["body"])
        row = body2["sculpture"]
        self.assertEqual(row["kind"], "splatbake")
        self.assertEqual(row["source_artifact_id"], "color_run_abc")
        # the generate core really ran: its ephemeral keys were uploaded and
        # the baked viewer landed as a hosted prefix
        self.assertIn("renders/j/sculpture_roots.bin", uploaded)
        self.assertIn(f"sculptures/{row['id']}/viewer.html", uploaded)
        # provenance says what happened, and the fresh dump was reused from
        # local disk — no 36MB S3 round trip after a miss
        self.assertEqual(body2["provenance"]["generate_cache_hit"], False)
        self.assertTrue(body2["provenance"]["roots_reused_locally"])

    @patch("handler_render_lores_preview.s3")
    def test_splat_bake_sanitizes_params_and_sources(self, mock_s3):
        from handler_render_lores_preview import handler

        resp = handler(_event(splat_bake={
            "source": {"kind": "saved", "saved_id": "scu_src1"},
            "params": {"res": 77},
        }), None)
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("splat res", json.loads(resp["body"])["detail"])

        mock_s3.get_object.side_effect = self._bake_saved_get_object(meta={"kind": "splatbake"})
        resp = handler(_event(splat_bake={
            "source": {"kind": "saved", "saved_id": "scu_src1"}, "params": {},
        }), None)
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("already a baked viewer", json.loads(resp["body"])["detail"])

        resp = handler(_event(splat_bake={"source": {"kind": "nope"}}), None)
        self.assertEqual(resp["statusCode"], 500)

    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_artifact_sculpture_rejects_grid_mismatch_with_transport(self, mock_s3, mock_run):
        # the artifact's step_scores grid MUST equal the transport grid —
        # otherwise scores and roots describe different solves
        from handler_render_lores_preview import handler

        calc, roots_key, roots_bytes = self._artifact_calc(4)
        mock_s3.get_object.side_effect = self._artifact_get_object(calc, roots_key, roots_bytes, b"")
        mock_s3.head_object.return_value = self._artifact_head(step_scores_grid_n="5")
        resp = handler(_event(
            artifact_sculpture={"artifact_id": "color_run_abc"},
            preview_source_size=2,
        ), None)
        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("does not match the transport grid", json.loads(resp["body"])["detail"])

    def test_calc_pipeline_compiles_legacy_transforms_to_programs(self):
        # Old calc.json artifacts carry legacy param/coeff transform chains. The
        # native runtime rejects non-empty legacy chains, so _calc_pipeline must
        # compile them to programs and clear the transform arrays.
        from handler_render_lores_preview import _calc_pipeline

        pipeline = _calc_pipeline({
            "pipeline": {
                "function": "g1",
                "param_transforms": [["unit_circle"]],
                "coeff_transforms": [["rev"], ["conj"]],
                "cfpv": [],
            }
        })

        self.assertEqual(pipeline["param_transforms"], [])
        self.assertEqual(pipeline["coeff_transforms"], [])
        self.assertTrue(pipeline["param_program"].get("tokens"))
        self.assertTrue(pipeline["coeff_program"].get("tokens"))

    def test_calc_pipeline_prefers_existing_program_and_drops_transforms(self):
        # An existing program wins; legacy transforms are dropped, not recompiled.
        from handler_render_lores_preview import _calc_pipeline

        program = {"version": 2, "tokens": [["set", "poly", "cf"]], "fingerprint": "x"}
        pipeline = _calc_pipeline({
            "pipeline": {
                "function": "g1",
                "coeff_transforms": [["rev"]],
                "coeff_program": program,
            }
        })

        self.assertEqual(pipeline["coeff_transforms"], [])
        self.assertEqual(pipeline["coeff_program"], program)

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_recompute_preview_compiles_legacy_transforms_for_native(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_COEFFS, TMP_FRAGMENT, TMP_PALETTE_FRAGMENT, TMP_PARAMS, TMP_ROOTS, handler

        calc = {
            "N": 5,
            "times": 1,
            "degree": 1,
            "n_coeffs": 2,
            "solver": "aberth_mt",
            "lores": {"N": 5, "n_steps": 25},
            "pipeline": {
                "function": "g1",
                "param_transforms": [["unit_circle"]],
                "coeff_transforms": [["rev"], ["conj"]],
                "cfpv": [],
            },
        }

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key == "renders/j/calc.json":
                return {"Body": _ChunkBody(json.dumps(calc).encode("utf-8"))}
            raise AssertionError(f"unexpected key: {key}")

        mock_s3.get_object.side_effect = get_object
        specs = {}

        def subprocess_fake(cmd, **kwargs):
            stdin = json.loads(kwargs.get("input") or "{}")
            if stdin.get("mode") == "param_gen":
                specs["param_gen"] = stdin
                with open(TMP_PARAMS, "wb") as fh:
                    fh.write(b"\x00" * (25 * 16))
                return MagicMock(returncode=0, stdout=json.dumps({"mode": "param_gen", "data_bytes": 25 * 16, "threads": 4}), stderr="")
            if stdin.get("mode") == "coeffgen_chunked":
                specs["coeffgen"] = stdin
                with open(TMP_COEFFS, "wb") as fh:
                    fh.write(b"\x00" * (25 * 2 * 2 * 4))
                return MagicMock(returncode=0, stdout=json.dumps({"mode": "coeffgen_chunked", "degree": 1, "n_coeffs": 2, "data_bytes": 25 * 2 * 2 * 4, "threads": 4}), stderr="")
            if stdin.get("mode") == "solve_mt":
                with open(TMP_ROOTS, "wb") as fh:
                    fh.write(b"\x00" * (25 * 1 * 2 * 4))
                return MagicMock(returncode=0, stdout=json.dumps({"mode": "solve_mt", "avg_iterations": 3.0, "n_threads": 4}), stderr="")
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0, "clip_hi": 1.0, "min_score": 0.0, "max_score": 1.0, "n_solves": 25, "threads": 1,
                }), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 1, "n_solves": 25, "clip_lo": 0.0, "clip_hi": 1.0,
                    "min_score": 0.0, "q05": 0.1, "q95": 0.9, "max_score": 1.0, "threads": 1,
                }), stderr="")
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([64]))
            with open(TMP_PALETTE_FRAGMENT, "wb") as fh:
                for idx in range(25):
                    fh.write(idx.to_bytes(4, "little") + bytes([idx + 1]))
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 1, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(
            degree=1,
            n_coeffs=2,
            preview_source_mode="recompute",
            preview_source_size=5,
            lores_bin_key="",
            solve_score_chain=[["centroid_re", "slv", "0.1"]],
        ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])

        # Legacy transforms compiled to programs; native sees no legacy chain.
        self.assertEqual(specs["param_gen"].get("param_transforms"), [])
        self.assertTrue(specs["param_gen"]["param_program"].get("tokens"))
        self.assertEqual(specs["coeffgen"].get("coeff_transforms"), [])
        self.assertTrue(specs["coeffgen"]["coeff_program"].get("tokens"))

    def test_malformed_body_returns_contextual_json(self):
        from handler_render_lores_preview import handler

        resp = handler({"body": "{"}, None)
        self.assertEqual(resp["statusCode"], 500)
        body = json.loads(resp["body"])
        self.assertEqual(body["error"], "render lores preview failed")
        self.assertEqual(body["phase"], "render-lores-preview")
        self.assertIn("Expecting", body["detail"])

    def test_non_object_body_returns_contextual_json(self):
        from handler_render_lores_preview import handler

        resp = handler({"body": "[]"}, None)
        self.assertEqual(resp["statusCode"], 500)
        body = json.loads(resp["body"])
        self.assertEqual(body["phase"], "render-lores-preview")
        self.assertIn("JSON object", body["detail"])


if __name__ == "__main__":
    unittest.main()
