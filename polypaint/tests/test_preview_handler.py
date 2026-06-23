import io
import json
import os
import sys
import unittest
from unittest.mock import patch

from botocore.exceptions import ClientError


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(body):
    return {"body": json.dumps(body)}


class TestPreviewHandlerHardening(unittest.TestCase):
    @patch("handler_preview.s3")
    def test_preview_returns_404_when_compute_missing_before_generation(self, mock_s3):
        import handler_preview

        mock_s3.get_object.side_effect = ClientError(
            {"Error": {"Code": "NoSuchKey", "Message": "missing"}},
            "GetObject",
        )

        resp = handler_preview.handler(_event({"job_id": "gone", "preview_size": 8}), None)
        body = json.loads(resp["body"])

        self.assertEqual(resp["statusCode"], 404)
        self.assertIn("compute gone not found", body["error"])
        mock_s3.put_object.assert_not_called()

    @patch("handler_preview.encode_png_gray")
    @patch("handler_preview.compute_viewport_from_bin")
    @patch("handler_preview.s3")
    def test_preview_fails_closed_if_compute_deleted_before_write(
        self, mock_s3, mock_viewport, mock_encode
    ):
        import handler_preview

        mock_s3.get_object.side_effect = [
            {"Body": io.BytesIO(json.dumps({"lores": {"bin_key": "renders/j/lores.bin"}, "degree": 3}).encode("utf-8"))},
            {"Body": io.BytesIO(b"\x00" * 16)},
        ]
        mock_viewport.return_value = {
            "scale": 1.0,
            "center_re": 0.0,
            "center_im": 0.0,
            "n_roots": 1,
            "q_re": [0.0, 1.0],
            "q_im": [0.0, 1.0],
        }
        mock_encode.return_value = b"png"
        mock_s3.head_object.side_effect = ClientError(
            {"Error": {"Code": "404", "Message": "missing"}},
            "HeadObject",
        )

        resp = handler_preview.handler(_event({"job_id": "j", "preview_size": 8}), None)
        body = json.loads(resp["body"])

        self.assertEqual(resp["statusCode"], 404)
        self.assertIn("disappeared before preview write", body["error"])
        mock_s3.put_object.assert_not_called()


if __name__ == "__main__":
    unittest.main()
