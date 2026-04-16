import json
import os
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, patch

from botocore.exceptions import ClientError


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _Body:
    def __init__(self, data):
        self._data = data

    def read(self):
        return self._data


class TestBilevelHandler(unittest.TestCase):
    @patch("handler_bilevel.report_status")
    @patch("handler_bilevel.subprocess.run")
    @patch("handler_bilevel.s3")
    def test_merge_does_not_fallback_to_legacy_bits_keys(self, mock_s3, mock_run, mock_report):
        import handler_bilevel as mod

        requested_keys = []
        uploaded = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            requested_keys.append(key)
            if key == "renders/job-bits/bits_chunk_0000_t0000.bits":
                raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")
            if key == "renders/job-bits/bits_s0000_t0000.bits":
                return {"Body": _Body(b"\x01\x02")}
            raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")

        def upload_fileobj(fileobj, bucket, key):
            uploaded[key] = fileobj.read()

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None, env=None):
            with open("/tmp/tile.tif", "wb") as fh:
                fh.write(b"II*\x00tile")
            return MagicMock(returncode=0, stdout=json.dumps({"pixels_set": 0, "file_size": 8}), stderr="")

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_run.side_effect = run_side_effect

        with tempfile.TemporaryDirectory():
            result = mod.handle_merge({
                "job_id": "job-bits",
                "task_id": "merge_0",
                "tile_idx": 0,
                "tile_w": 2,
                "tile_h": 2,
                "n_chunks": 1,
                "width": 2,
                "height": 2,
                "tile_size": 2,
                "n_tile_cols": 1,
            })

        body = json.loads(result["body"])
        self.assertEqual(body["pixels_set"], 0)
        self.assertIn("renders/job-bits/bilevel_t0000.tif", uploaded)
        self.assertIn("renders/job-bits/bits_chunk_0000_t0000.bits", requested_keys)
        self.assertNotIn("renders/job-bits/bits_s0000_t0000.bits", requested_keys)


if __name__ == "__main__":
    unittest.main()
