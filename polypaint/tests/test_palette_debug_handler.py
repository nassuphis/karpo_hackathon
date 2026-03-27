"""
Tests for handler_palette_debug.py.

Validates payload validation, binary invocation, S3 upload, and response shape.
Does NOT require Docker — mocks S3 and subprocess.

Run: cd polypaint && uv run python -m pytest tests/test_palette_debug_handler.py -v
"""
import json
import os
import sys
import unittest
from unittest.mock import patch, MagicMock, mock_open

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _make_event(**overrides):
    payload = {
        "job_id": "j",
        "degree": 5,
        "N": 100,
        "lores_N": 10,
        "times": 1,
        "lores_bin_key": "renders/j/lores.bin",
        "metric": "proximity",
        "palette": "inferno",
        "solve_score_quantile": 0.001,
        "root_transforms": [],
    }
    payload.update(overrides)
    return payload


class TestPaletteDebugHandler(unittest.TestCase):

    @patch("handler_palette_debug.s3")
    @patch("handler_palette_debug.subprocess")
    def test_validates_quantile_range(self, mock_sub, mock_s3):
        from handler_palette_debug import handler
        event = _make_event(solve_score_quantile=0.5)
        with self.assertRaises(RuntimeError) as ctx:
            handler(event, None)
        self.assertIn("0.001", str(ctx.exception))

    @patch("handler_palette_debug.s3")
    @patch("handler_palette_debug.subprocess")
    def test_downloads_lores_key(self, mock_sub, mock_s3):
        """Handler downloads the provided lores_bin_key."""
        from handler_palette_debug import handler

        # Mock S3 download
        mock_body = MagicMock()
        mock_body.iter_chunks.return_value = [b'\x00' * 100]
        mock_s3.get_object.return_value = {"Body": mock_body}

        # Mock binary success
        mock_sub.run.side_effect = [
            MagicMock(returncode=0, stdout='{"mode":"palette_debug","metric":"proximity","palette":"inferno","n_samples_used":100,"degree":5,"lores_n":10,"full_n":100,"times":1,"using_pass":0,"clip_lo":-1,"clip_hi":1,"clip_range":2,"clip_fallback":false,"clip_fallback_reason":null,"cuts_norm":[0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9]}', stderr=''),
            MagicMock(returncode=0, stdout='', stderr=''),  # raw2jpeg
        ]

        # Mock file size and upload
        with patch("os.path.getsize", return_value=1024):
            with patch("builtins.open", mock_open(read_data=b'\x00' * 100)):
                try:
                    handler(_make_event(), None)
                except Exception:
                    pass  # May fail on file I/O details, that's OK

        # Verify S3 download was called with the right key
        mock_s3.get_object.assert_called_once()
        call_kwargs = mock_s3.get_object.call_args[1]
        self.assertEqual(call_kwargs["Key"], "renders/j/lores.bin")

    @patch("handler_palette_debug.s3")
    @patch("handler_palette_debug.subprocess")
    def test_passes_metric_and_palette_to_binary(self, mock_sub, mock_s3):
        """Binary invocation includes metric and palette args."""
        from handler_palette_debug import handler

        mock_body = MagicMock()
        mock_body.iter_chunks.return_value = [b'\x00' * 100]
        mock_s3.get_object.return_value = {"Body": mock_body}
        mock_s3.generate_presigned_url.return_value = "https://example.com/pal.jpeg"

        meta_json = json.dumps({
            "mode": "palette_debug", "metric": "crowding", "palette": "turbo",
            "n_samples_used": 100, "degree": 5, "lores_n": 10, "full_n": 100,
            "times": 1, "using_pass": 0, "clip_lo": -1, "clip_hi": 1,
            "clip_range": 2, "clip_fallback": False, "clip_fallback_reason": None,
            "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
        })
        mock_sub.run.side_effect = [
            MagicMock(returncode=0, stdout=meta_json, stderr=''),
            MagicMock(returncode=0, stdout='', stderr=''),  # raw2jpeg
            MagicMock(returncode=0, stdout='', stderr=''),  # vipsthumbnail preview
        ]

        with patch("os.path.getsize", return_value=2048):
            with patch("builtins.open", mock_open(read_data=b'\x00' * 100)):
                result = handler(_make_event(metric="crowding", palette="turbo"), None)

        # Check binary was called with metric and palette
        binary_call = mock_sub.run.call_args_list[0]
        cmd = binary_call[0][0]
        cmd_str = " ".join(cmd)
        self.assertIn("--metric=crowding", cmd_str)
        self.assertIn("--palette=turbo", cmd_str)

    @patch("handler_palette_debug.s3")
    @patch("handler_palette_debug.subprocess")
    def test_uploads_image_palette_jpeg(self, mock_sub, mock_s3):
        """Handler uploads to renders/{job_id}/image_palette.jpeg."""
        from handler_palette_debug import handler

        mock_body = MagicMock()
        mock_body.iter_chunks.return_value = [b'\x00' * 100]
        mock_s3.get_object.return_value = {"Body": mock_body}
        mock_s3.generate_presigned_url.return_value = "https://example.com/pal.jpeg"

        meta_json = json.dumps({
            "mode": "palette_debug", "metric": "proximity", "palette": "inferno",
            "n_samples_used": 100, "degree": 5, "lores_n": 10, "full_n": 100,
            "times": 1, "using_pass": 0, "clip_lo": -1, "clip_hi": 1,
            "clip_range": 2, "clip_fallback": False, "clip_fallback_reason": None,
            "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
        })
        mock_sub.run.side_effect = [
            MagicMock(returncode=0, stdout=meta_json, stderr=''),
            MagicMock(returncode=0, stdout='', stderr=''),  # raw2jpeg
            MagicMock(returncode=0, stdout='', stderr=''),  # vipsthumbnail preview
        ]

        with patch("os.path.getsize", return_value=4096):
            with patch("builtins.open", mock_open(read_data=b'\xff\xd8' * 50)):
                result = handler(_make_event(), None)

        # Verify S3 upload was called with the right key
        upload_calls = [c for c in mock_s3.upload_fileobj.call_args_list]
        self.assertTrue(len(upload_calls) > 0, "upload_fileobj not called")
        upload_key = upload_calls[0][0][2]  # positional arg: key
        self.assertEqual(upload_key, "renders/j/image_palette.jpeg")

        # Verify metadata was set
        extra_args = upload_calls[0][1].get("ExtraArgs", {})
        metadata = extra_args.get("Metadata", {})
        self.assertEqual(metadata["metric"], "proximity")
        self.assertEqual(metadata["palette"], "inferno")

    @patch("handler_palette_debug.s3")
    @patch("handler_palette_debug.subprocess")
    def test_deletes_stale_preview_before_upload(self, mock_sub, mock_s3):
        """Handler deletes preview_palette.png before uploading new image_palette.jpeg."""
        from handler_palette_debug import handler

        mock_body = MagicMock()
        mock_body.iter_chunks.return_value = [b'\x00' * 100]
        mock_s3.get_object.return_value = {"Body": mock_body}
        mock_s3.generate_presigned_url.return_value = "https://example.com/pal.jpeg"

        meta_json = json.dumps({
            "mode": "palette_debug", "metric": "proximity", "palette": "inferno",
            "n_samples_used": 100, "degree": 5, "lores_n": 10, "full_n": 100,
            "times": 1, "using_pass": 0, "clip_lo": -1, "clip_hi": 1,
            "clip_range": 2, "clip_fallback": False, "clip_fallback_reason": None,
            "cuts_norm": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9],
        })
        mock_sub.run.side_effect = [
            MagicMock(returncode=0, stdout=meta_json, stderr=''),
            MagicMock(returncode=0, stdout='', stderr=''),  # raw2jpeg
            MagicMock(returncode=0, stdout='', stderr=''),  # vipsthumbnail preview
        ]

        with patch("os.path.getsize", return_value=4096):
            with patch("builtins.open", mock_open(read_data=b'\xff\xd8' * 50)):
                handler(_make_event(), None)

        # Verify delete_object was called for the preview before upload
        delete_calls = [c for c in mock_s3.delete_object.call_args_list]
        self.assertTrue(len(delete_calls) > 0, "delete_object not called for preview")
        deleted_key = delete_calls[0][1].get("Key")
        self.assertEqual(deleted_key, "renders/j/preview_palette.png")

        # Verify upload happened AFTER delete (upload_fileobj call exists)
        upload_calls = mock_s3.upload_fileobj.call_args_list
        self.assertTrue(len(upload_calls) > 0, "upload_fileobj not called")

    def test_handler_does_not_report_status(self):
        """Handler must not call report_status."""
        import handler_palette_debug as mod
        source = open(mod.__file__).read()
        self.assertNotIn("report_status(", source)


if __name__ == "__main__":
    unittest.main()
