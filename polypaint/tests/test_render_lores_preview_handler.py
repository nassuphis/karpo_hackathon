import base64
import json
import os
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

        self.assertFalse(mock_s3.put_object.called)
        self.assertFalse(mock_s3.upload_file.called)
        self.assertFalse(mock_s3.upload_fileobj.called)

        summary_cmd = mock_run.call_args_list[0][0][0]
        raster_cmd = mock_run.call_args_list[-1][0][0]
        self.assertIn("--score_output_normalize=1", summary_cmd)
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
