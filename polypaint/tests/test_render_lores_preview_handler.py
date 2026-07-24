import base64
import json
import os
import struct
import sys
import unittest
from unittest.mock import MagicMock, patch


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

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_sculpture_uploads_transformed_roots_and_palette(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import BUCKET, TMP_FRAGMENT, TMP_PALETTE_FRAGMENT, TMP_XFORMED_ROOTS, handler

        mock_s3.get_object.return_value = {"Body": _ChunkBody(b"\x00" * (3 * 2 * 2 * 4))}
        xformed_bytes = b"T" * (3 * 2 * 2 * 4)

        def subprocess_fake(cmd, **kwargs):
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0, "clip_hi": 1.0, "min_score": 0.0, "max_score": 1.0,
                    "n_solves": 3, "threads": 1}), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 2, "n_solves": 3, "clip_lo": 0.0, "clip_hi": 1.0,
                    "min_score": 0.0, "q05": 0.1, "q95": 0.9, "max_score": 1.0,
                    "threads": 1}), stderr="")
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
            with open(TMP_PALETTE_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
            self.assertIn(f"--xformed_roots_output={TMP_XFORMED_ROOTS}", cmd)
            with open(TMP_XFORMED_ROOTS, "wb") as fh:
                fh.write(xformed_bytes)
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 2, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(lores_N=1, sculpture=True), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        sc = body["sculpture"]
        # the uploaded roots are the raster's TRANSFORMED dump (root-transform
        # script + rotation applied) — never the raw transport artifact, which
        # ignores both. No lores.bin reuse even in physical mode.
        self.assertEqual(sc["roots_key"], "renders/j/sculpture_roots.bin")
        self.assertTrue(sc["roots_url"].startswith("https://"))
        # version-stamped URLs: fixed keys + browser heuristic caching served
        # stale palettes across runs
        self.assertIn("/renders/j/sculpture_roots.bin?v=", sc["roots_url"])
        self.assertEqual(sc["palette_key"], "renders/j/sculpture_palette.png")
        self.assertIn("/renders/j/sculpture_palette.png?v=", sc["palette_url"])
        self.assertEqual(sc["grid_n"], 1)
        self.assertEqual(sc["degree"], 2)
        self.assertEqual(sc["step_count"], 3)
        self.assertEqual(sc["pass_count"], 3)
        self.assertEqual(sc["roots_bytes"], 3 * 2 * 2 * 4)
        self.assertIn("min_re", sc["viewport"])
        self.assertTrue(any(line.startswith("Sculpture export:") for line in body["logs"]))
        self.assertEqual(mock_s3.put_object.call_count, 2)
        by_key = {call.kwargs["Key"]: call.kwargs for call in mock_s3.put_object.call_args_list}
        roots_put = by_key["renders/j/sculpture_roots.bin"]
        self.assertEqual(roots_put["Bucket"], BUCKET)
        self.assertEqual(roots_put["Body"], xformed_bytes)
        self.assertEqual(roots_put["ContentType"], "application/octet-stream")
        self.assertEqual(roots_put["CacheControl"], "no-cache")
        pal_put = by_key["renders/j/sculpture_palette.png"]
        self.assertEqual(pal_put["ContentType"], "image/png")
        self.assertEqual(pal_put["CacheControl"], "no-cache")
        self.assertEqual(pal_put["Body"], PNG_1X1)

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_sculpture_recompute_uploads_fresh_roots(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_COEFFS, TMP_FRAGMENT, TMP_PALETTE_FRAGMENT, TMP_PARAMS, TMP_ROOTS, TMP_XFORMED_ROOTS, handler

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
        roots_bytes = b"\x00" * (25 * 1 * 2 * 4)
        xformed_bytes = b"X" * (25 * 1 * 2 * 4)

        def get_object(**kwargs):
            key = kwargs.get("Key")
            if key == "renders/j/calc.json":
                return {"Body": _ChunkBody(json.dumps(calc).encode("utf-8"))}
            raise AssertionError(f"unexpected key: {key}")

        mock_s3.get_object.side_effect = get_object

        def subprocess_fake(cmd, **kwargs):
            stdin = json.loads(kwargs.get("input") or "{}")
            if stdin.get("mode") == "param_gen":
                with open(TMP_PARAMS, "wb") as fh:
                    fh.write(b"\x00" * (25 * 16))
                return MagicMock(returncode=0, stdout=json.dumps({"mode": "param_gen", "data_bytes": 25 * 16, "threads": 4}), stderr="")
            if stdin.get("mode") == "coeffgen_chunked":
                with open(TMP_COEFFS, "wb") as fh:
                    fh.write(b"\x00" * (25 * 2 * 2 * 4))
                return MagicMock(returncode=0, stdout=json.dumps({"mode": "coeffgen_chunked", "degree": 1, "n_coeffs": 2, "data_bytes": 25 * 2 * 2 * 4, "threads": 4}), stderr="")
            if stdin.get("mode") == "solve_mt":
                with open(TMP_ROOTS, "wb") as fh:
                    fh.write(roots_bytes)
                return MagicMock(returncode=0, stdout=json.dumps({"mode": "solve_mt", "avg_iterations": 3.0, "n_threads": 4}), stderr="")
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0, "clip_hi": 1.0, "min_score": 0.0, "max_score": 1.0,
                    "n_solves": 25, "threads": 1}), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 1, "n_solves": 25, "clip_lo": 0.0, "clip_hi": 1.0,
                    "min_score": 0.0, "q05": 0.1, "q95": 0.9, "max_score": 1.0,
                    "threads": 1}), stderr="")
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([64]))
            with open(TMP_PALETTE_FRAGMENT, "wb") as fh:
                for idx in range(25):
                    fh.write(idx.to_bytes(4, "little") + bytes([idx + 1]))
            with open(TMP_XFORMED_ROOTS, "wb") as fh:
                fh.write(xformed_bytes)
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
            sculpture=True,
        ), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        sc = body["sculpture"]
        self.assertEqual(sc["roots_key"], "renders/j/sculpture_roots.bin")
        self.assertEqual(sc["palette_key"], "renders/j/sculpture_palette.png")
        self.assertEqual(sc["grid_n"], 5)
        self.assertEqual(sc["step_count"], 25)
        self.assertEqual(sc["pass_count"], 1)
        self.assertEqual(mock_s3.put_object.call_count, 2)
        by_key = {call.kwargs["Key"]: call.kwargs for call in mock_s3.put_object.call_args_list}
        self.assertEqual(sorted(by_key), ["renders/j/sculpture_palette.png", "renders/j/sculpture_roots.bin"])
        self.assertEqual(by_key["renders/j/sculpture_roots.bin"]["Body"], xformed_bytes)
        self.assertEqual(by_key["renders/j/sculpture_roots.bin"]["ContentType"], "application/octet-stream")
        self.assertEqual(by_key["renders/j/sculpture_roots.bin"]["CacheControl"], "no-cache")
        self.assertIn("/renders/j/sculpture_roots.bin?v=", sc["roots_url"])

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_sculpture_u16_format_flags_the_raster_and_halves_the_dump(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_FRAGMENT, TMP_PALETTE_FRAGMENT, TMP_XFORMED_ROOTS, handler

        mock_s3.get_object.return_value = {"Body": _ChunkBody(b"\x00" * (3 * 2 * 2 * 4))}
        xformed_u16 = b"Q" * (3 * 2 * 2 * 2)   # steps*degree*2 u16 values

        def subprocess_fake(cmd, **kwargs):
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0, "clip_hi": 1.0, "min_score": 0.0, "max_score": 1.0,
                    "n_solves": 3, "threads": 1}), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 2, "n_solves": 3, "clip_lo": 0.0, "clip_hi": 1.0,
                    "min_score": 0.0, "q05": 0.1, "q95": 0.9, "max_score": 1.0,
                    "threads": 1}), stderr="")
            self.assertIn("--xformed_roots_format=u16", cmd)
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
            with open(TMP_PALETTE_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
            with open(TMP_XFORMED_ROOTS, "wb") as fh:
                fh.write(xformed_u16)
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 2, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(lores_N=1, sculpture=True, sculpture_format="u16"), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        sc = body["sculpture"]
        self.assertEqual(sc["format"], "u16")
        self.assertEqual(sc["roots_bytes"], 3 * 2 * 2 * 2)
        by_key = {call.kwargs["Key"]: call.kwargs for call in mock_s3.put_object.call_args_list}
        self.assertEqual(by_key["renders/j/sculpture_roots.bin"]["Body"], xformed_u16)

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_sculpture_async_publishes_running_then_done(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_FRAGMENT, TMP_PALETTE_FRAGMENT, TMP_XFORMED_ROOTS, handler

        mock_s3.get_object.return_value = {"Body": _ChunkBody(b"\x00" * (3 * 2 * 2 * 4))}
        xformed_bytes = b"T" * (3 * 2 * 2 * 4)

        def subprocess_fake(cmd, **kwargs):
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0, "clip_hi": 1.0, "min_score": 0.0, "max_score": 1.0,
                    "n_solves": 3, "threads": 1}), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 2, "n_solves": 3, "clip_lo": 0.0, "clip_hi": 1.0,
                    "min_score": 0.0, "q05": 0.1, "q95": 0.9, "max_score": 1.0,
                    "threads": 1}), stderr="")
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
            with open(TMP_PALETTE_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
            with open(TMP_XFORMED_ROOTS, "wb") as fh:
                fh.write(xformed_bytes)
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 2, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(lores_N=1, sculpture=True, sculpture_async=True), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        statuses = [
            json.loads(c.kwargs["Body"])
            for c in mock_s3.put_object.call_args_list
            if c.kwargs["Key"] == "renders/j/sculpture_hires_result.json"
        ]
        self.assertEqual([r["status"] for r in statuses], ["running", "done"])
        self.assertEqual(statuses[1]["sculpture"]["roots_key"], "renders/j/sculpture_roots.bin")
        self.assertEqual(statuses[1]["sculpture"]["grid_n"], 1)

    @patch("handler_render_lores_preview.s3")
    def test_sculpture_async_publishes_error(self, mock_s3):
        from handler_render_lores_preview import handler

        resp = handler(_event(lores_N=1, sculpture=True, sculpture_async=True,
                              background_color="not-a-color"), None)
        self.assertEqual(resp["statusCode"], 500)
        statuses = [
            json.loads(c.kwargs["Body"])
            for c in mock_s3.put_object.call_args_list
            if c.kwargs["Key"] == "renders/j/sculpture_hires_result.json"
        ]
        self.assertEqual(statuses[-1]["status"], "error")
        self.assertIn("background_color", statuses[-1]["detail"])

    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_sculpture_requires_square_grid(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_FRAGMENT, handler

        # 3 steps with no lores_N: no grid can be inferred -> palette_grid_n=0
        mock_s3.get_object.return_value = {"Body": _ChunkBody(b"\x00" * (3 * 2 * 2 * 4))}

        def subprocess_fake(cmd, **kwargs):
            if "--mode=clip" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "clip_lo": 0.0, "clip_hi": 1.0, "min_score": 0.0, "max_score": 1.0,
                    "n_solves": 3, "threads": 1}), stderr="")
            if "--mode=summary" in cmd:
                return MagicMock(returncode=0, stdout=json.dumps({
                    "degree": 2, "n_solves": 3, "clip_lo": 0.0, "clip_hi": 1.0,
                    "min_score": 0.0, "q05": 0.1, "q95": 0.9, "max_score": 1.0,
                    "threads": 1}), stderr="")
            with open(TMP_FRAGMENT, "wb") as fh:
                fh.write((0).to_bytes(4, "little") + bytes([10]))
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 2, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(sculpture=True), None)
        self.assertEqual(resp["statusCode"], 500)
        body = json.loads(resp["body"])
        self.assertIn("sculpture export needs a square parameter grid", body["detail"])
        self.assertFalse(mock_s3.put_object.called)

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
