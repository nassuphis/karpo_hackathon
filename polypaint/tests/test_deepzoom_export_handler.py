import io
import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "test_dz",
        "source_key": "renders/test_dz/color/color_src/image.jpeg",
        "raw_key": "renders/test_dz/color/color_src/greyscale.raw",
        "raw_meta_key": "renders/test_dz/color/color_src/greyscale.meta.json",
        "export_id": "dz_raw_test",
    }
    payload.update(overrides)
    return {"body": json.dumps(payload)}


class TestDeepZoomExportRaw(unittest.TestCase):
    @patch("handler_deepzoom_export.report_status")
    @patch("handler_deepzoom_export.s3")
    @patch("handler_deepzoom_export.subprocess")
    def test_export_can_materialize_from_fused_greyscale_raw(self, mock_subprocess, mock_s3, mock_report):
        import handler_deepzoom_export as mod

        put_calls = []

        def get_object(Bucket=None, Key=None):
            if Key == "renders/test_dz/color/color_src/greyscale.meta.json":
                body = {
                    "version": 1,
                    "job_id": "test_dz",
                    "run_id": "run_src",
                    "artifact_family": "color",
                    "artifact_id": "color_src",
                    "width": 2,
                    "height": 2,
                    "encoding": {
                        "type": "u8_clipped_score_v1",
                        "background_byte": 0,
                        "foreground_min": 1,
                        "foreground_max": 255,
                        "row_major": True,
                    },
                    "chain_fingerprint": "fp_src",
                    "score_chain": [["proximity", "0.1"]],
                    "score_program": "m0",
                    "clip_slots": [{"slot": 0, "metric": "proximity", "source": "slv", "clip_lo": 0.1, "clip_hi": 0.9}],
                    "background_color": [0, 0, 0],
                    "plan_params_digest": "sha256:plan_src",
                    "render_execution": {"color_pipeline": "fused", "raster_engine": "mt"},
                    "keys": {
                        "raw_key": "renders/test_dz/color/color_src/greyscale.raw",
                        "image_key": "renders/test_dz/color/color_src/image.jpeg",
                        "preview_key": "renders/test_dz/color/color_src/preview.png",
                        "meta_key": "renders/test_dz/color/color_src/meta.json",
                    },
                    "created_at": "2026-04-03T09:00:00Z",
                }
                return {"Body": MagicMock(read=lambda: json.dumps(body).encode("utf-8"))}
            if Key == "renders/test_dz/color/color_src/greyscale.raw":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [bytes([0, 1, 128, 255])])}
            raise AssertionError(f"unexpected get_object key: {Key}")

        def put_object(**kwargs):
            body = kwargs.get("Body")
            if hasattr(body, "read"):
                body = body.read()
            put_calls.append({**kwargs, "Body": body})

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        mock_s3.head_object.return_value = {"Metadata": {"palette": "inferno"}}

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "pixel_bins_render":
                with open(cmd[1], "rb") as fh:
                    bins = fh.read()
                self.assertEqual(bins, bytes([255, 0, 4, 9]))
                with open(cmd[2], "wb") as fh:
                    fh.write(b"FAKE_RGB_RAW")
                return MagicMock(returncode=0, stdout="", stderr="")
            if exe == "raw2jpeg":
                with open(cmd[2], "wb") as fh:
                    fh.write(b"PNGDATA")
                return MagicMock(returncode=0, stdout=json.dumps({"file_size": 7}), stderr="")
            if exe == "dz_export":
                return MagicMock(returncode=0, stdout=json.dumps({"width": 2, "height": 2}), stderr="")
            raise AssertionError(f"unexpected executable {exe}")

        mock_subprocess.run.side_effect = run_side_effect

        import builtins
        real_open = builtins.open
        fake_file = MagicMock(read=lambda: b"fake", write=lambda x: None)
        fake_cm = MagicMock(__enter__=MagicMock(return_value=fake_file), __exit__=MagicMock(return_value=False))

        def selective_open(*args, **kwargs):
            path = args[0] if args else kwargs.get("file", "")
            if isinstance(path, str) and (
                path.endswith("_template.html")
                or path.startswith("/tmp/source.")
                or path.endswith(".bins")
                or path.endswith(".raw")
            ):
                return real_open(*args, **kwargs)
            return fake_cm

        with patch.object(os, "makedirs", side_effect=lambda *a, **kw: None), \
             patch.object(os, "remove", side_effect=lambda *a: None), \
             patch.object(os.path, "exists", return_value=True), \
             patch.object(os.path, "isdir", return_value=True), \
             patch.object(os, "walk", return_value=[("/tmp/dz/image_files/0", [], ["0_0.png"])]), \
             patch("shutil.rmtree"), \
             patch("builtins.open", side_effect=selective_open):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["source_kind"], "raw")

        meta_puts = [c for c in put_calls if c.get("Key") == "deepzoom/test_dz/dz_raw_test/meta.json"]
        self.assertEqual(len(meta_puts), 1)
        manifest = json.loads(meta_puts[0]["Body"])
        self.assertEqual(manifest["source_kind"], "raw")
        self.assertEqual(manifest["raw_key"], "renders/test_dz/color/color_src/greyscale.raw")
        self.assertEqual(manifest["raw_meta_key"], "renders/test_dz/color/color_src/greyscale.meta.json")
        self.assertEqual(manifest["palette"], "inferno")

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "generating", "uploading", "done"])


if __name__ == "__main__":
    unittest.main()
