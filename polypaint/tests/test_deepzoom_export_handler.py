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
    def test_export_uses_exact_source_image_even_when_raw_sidecar_is_present(self, mock_subprocess, mock_s3, mock_report):
        import handler_deepzoom_export as mod

        put_calls = []

        def get_object(Bucket=None, Key=None):
            if Key == "renders/test_dz/color/color_src/image.jpeg":
                return {"Body": MagicMock(iter_chunks=lambda chunk_size=None: [b"exact image bytes"])}
            raise AssertionError(f"unexpected get_object key: {Key}")

        def put_object(**kwargs):
            body = kwargs.get("Body")
            if hasattr(body, "read"):
                body = body.read()
            put_calls.append({**kwargs, "Body": body})

        mock_s3.get_object.side_effect = get_object
        mock_s3.put_object.side_effect = put_object
        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            exe = os.path.basename(cmd[0])
            if exe == "dz_export":
                self.assertEqual(cmd[1], "/tmp/deepzoom_source.jpeg")
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
                or path.startswith("/tmp/deepzoom_source")
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
        self.assertEqual(body["source_kind"], "image")

        meta_puts = [c for c in put_calls if c.get("Key") == "deepzoom/test_dz/dz_raw_test/meta.json"]
        self.assertEqual(len(meta_puts), 1)
        manifest = json.loads(meta_puts[0]["Body"])
        self.assertEqual(manifest["source_kind"], "image")
        self.assertNotIn("raw_key", manifest)
        self.assertNotIn("raw_meta_key", manifest)
        self.assertNotIn("palette", manifest)

        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "generating", "uploading", "done"])


if __name__ == "__main__":
    unittest.main()
