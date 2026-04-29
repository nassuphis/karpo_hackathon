import json
import os
import sys
import unittest


os.environ.setdefault("AWS_EC2_METADATA_DISABLED", "true")
os.environ.setdefault("AWS_DEFAULT_REGION", "us-east-1")
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class TestParamDebugHandler(unittest.TestCase):
    def test_rejects_oversized_sync_requests_before_native_work(self):
        import handler_param_debug as mod

        resp = mod.handler({"N": mod.MAX_SYNC_PARAM_DEBUG_N + 1, "pix": 512}, None)
        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("synchronous", json.loads(resp["body"])["message"])

        resp = mod.handler({"N": 64, "pix": mod.MAX_SYNC_PARAM_DEBUG_PIX + 1}, None)
        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("pix", json.loads(resp["body"])["message"])


if __name__ == "__main__":
    unittest.main()
