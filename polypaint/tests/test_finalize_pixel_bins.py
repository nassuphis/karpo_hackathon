import json
import os
import struct
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, patch

from botocore.exceptions import ClientError


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


def _event(**overrides):
    payload = {
        "job_id": "j",
        "task_id": "tile_0",
        "tile_idx": 0,
        "n_chunks": 1,
        "tile_w": 2,
        "tile_h": 2,
        "emit_pixel_bins": True,
        "pixel_bins_out_key": "renders/j/color/color_x/pixel_bins/tile_0000.bin",
    }
    payload.update(overrides)
    return payload


class _Body:
    def __init__(self, data):
        self._data = data

    def iter_chunks(self, chunk_size):
        yield self._data


class _InPipe:
    def __init__(self):
        self.data = bytearray()

    def write(self, chunk):
        self.data.extend(chunk)

    def close(self):
        return None


class _FakeProc:
    def __init__(self, cmd):
        self.cmd = cmd
        self.stdin = _InPipe()
        self.stderr = MagicMock(read=lambda: b"")

    def wait(self, timeout=None):
        out_arg = next(arg for arg in self.cmd if arg.startswith("--output="))
        out_path = out_arg.split("=", 1)[1]
        exe = os.path.basename(self.cmd[0])
        if exe == "pixassemble":
            with open(out_path, "wb") as fh:
                fh.write(struct.pack("<III", 2, 2, 3))
                fh.write(b"\x00" * 12)
        elif exe == "pixbinassemble":
            with open(out_path, "wb") as fh:
                fh.write(bytes([255, 1, 2, 3]))
        else:
            raise AssertionError(f"unexpected executable {exe}")
        return 0


class TestFinalizePixelBins(unittest.TestCase):

    @patch("handler_finalize.report_status")
    @patch("handler_finalize.subprocess.Popen")
    @patch("handler_finalize.s3")
    def test_finalize_uploads_dense_pixel_bin_tile_when_enabled(self, mock_s3, mock_popen, mock_report):
        import handler_finalize as mod

        uploads = {}

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == "renders/j/pix_chunk_0000_t0000.pix":
                return {"Body": _Body(b"\x00" * 8)}
            if key == "renders/j/pixbin_chunk_0000_t0000.pbx":
                return {"Body": _Body(b"\x00" * 8)}
            raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        mock_s3.get_object.side_effect = get_object
        mock_s3.upload_fileobj.side_effect = upload_fileobj
        mock_popen.side_effect = lambda *args, **kwargs: _FakeProc(args[0])

        with tempfile.TemporaryDirectory() as td, \
             patch("handler_finalize.PIXASSEMBLE", "pixassemble"), \
             patch("handler_finalize.PIXBINASSEMBLE", "pixbinassemble"):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pixel_bins_key"], "renders/j/color/color_x/pixel_bins/tile_0000.bin")
        self.assertIn("renders/j/tile_0000.raw", uploads)
        self.assertIn("renders/j/color/color_x/pixel_bins/tile_0000.bin", uploads)
        self.assertEqual(uploads["renders/j/color/color_x/pixel_bins/tile_0000.bin"], bytes([255, 1, 2, 3]))
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertEqual(statuses, ["started", "assembled", "done"])


if __name__ == "__main__":
    unittest.main()
