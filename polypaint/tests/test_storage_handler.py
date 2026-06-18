import json
import os
import sys
import unittest
from unittest.mock import patch

from botocore.exceptions import ClientError


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(path, body):
    return {"path": path, "body": json.dumps(body)}


class TestStorageHandlerHardening(unittest.TestCase):
    @patch("handler_storage.s3")
    def test_cleanup_rejects_non_render_keys_before_s3_delete(self, mock_s3):
        import handler_storage

        resp = handler_storage.handler(
            _event("/cleanup", {"keys": ["index.html", "js/01-core-compute.js"]}),
            None,
        )

        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("cleanup key not allowed", json.loads(resp["body"])["error"])
        mock_s3.delete_objects.assert_not_called()

    @patch("handler_storage.s3")
    def test_cleanup_counts_deleted_objects_and_reports_s3_errors(self, mock_s3):
        import handler_storage

        mock_s3.delete_objects.return_value = {
            "Deleted": [{"Key": "renders/j/stripe_0.raw"}],
            "Errors": [{"Key": "renders/j/merge_0.raw", "Code": "AccessDenied"}],
        }

        resp = handler_storage.handler(
            _event("/cleanup", {
                "keys": [
                    "renders/j/stripe_0.raw",
                    "renders/j/merge_0.raw",
                    "renders/j/deepzoom_latest.json",
                ],
            }),
            None,
        )
        body = json.loads(resp["body"])

        self.assertEqual(resp["statusCode"], 200)
        self.assertEqual(body["deleted"], 1)
        self.assertEqual(body["errors"], ["renders/j/merge_0.raw: AccessDenied"])
        delete_payload = mock_s3.delete_objects.call_args.kwargs["Delete"]
        self.assertNotIn("Quiet", delete_payload)

    @patch("handler_storage.s3")
    def test_unwrapped_routes_now_return_error_envelope(self, mock_s3):
        import handler_storage

        resp = handler_storage.handler(_event("/detail", {}), None)

        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("job_id", json.loads(resp["body"])["error"])

    @patch("handler_storage.s3")
    def test_storage_client_errors_are_json_enveloped(self, mock_s3):
        import handler_storage

        mock_s3.generate_presigned_url.side_effect = ClientError(
            {"Error": {"Code": "AccessDenied", "Message": "denied"}},
            "GeneratePresignedUrl",
        )

        resp = handler_storage.handler(
            _event("/presign", {"key": "renders/j/image.jpeg"}),
            None,
        )

        self.assertEqual(resp["statusCode"], 500)
        self.assertIn("AccessDenied", json.loads(resp["body"])["error"])


if __name__ == "__main__":
    unittest.main()
