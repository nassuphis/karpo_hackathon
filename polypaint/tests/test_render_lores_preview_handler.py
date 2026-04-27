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
    @patch("handler_render_lores_preview.render_score_raw")
    @patch("handler_render_lores_preview.subprocess.run")
    @patch("handler_render_lores_preview.s3")
    def test_returns_inline_png_without_s3_writes(self, mock_s3, mock_run, mock_render):
        from handler_render_lores_preview import TMP_FRAGMENT, handler

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
            return MagicMock(returncode=0, stdout=json.dumps({"roots_plotted": 2, "roots_clipped": 0}), stderr="")

        def render_fake(**kwargs):
            with open(kwargs["out_path"], "wb") as fh:
                fh.write(PNG_1X1)
            return {"file_size": len(PNG_1X1), "preview_file_size": 0}

        mock_run.side_effect = subprocess_fake
        mock_render.side_effect = render_fake

        resp = handler(_event(), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        self.assertEqual(body["content_type"], "image/png")
        self.assertEqual(base64.b64decode(body["image_base64"]), PNG_1X1)
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
        self.assertIn("--score_program=m0-0", summary_cmd)
        self.assertIn("--score_metrics=proximity", summary_cmd)
        self.assertFalse(any(arg.startswith("--metric=") for arg in summary_cmd))
        self.assertIn("--score_output_normalize=1", raster_cmd)
        self.assertIn("--score_output_clip_lo=0.1", raster_cmd)
        self.assertIn("--score_output_clip_hi=0.9", raster_cmd)
        self.assertIn("--fragment_prefix=/tmp/render_lores_preview_fragment", raster_cmd)

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
        from handler_render_lores_preview import TMP_FRAGMENT, handler

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
        from handler_render_lores_preview import TMP_FRAGMENT, handler

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
        from handler_render_lores_preview import TMP_COEFFS, TMP_FRAGMENT, TMP_PARAMS, TMP_ROOTS, handler

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
