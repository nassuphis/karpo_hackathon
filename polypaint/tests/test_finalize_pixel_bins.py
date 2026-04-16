import json
import os
import struct
import sys
import tempfile
import time
import unittest
from unittest.mock import MagicMock, patch

from botocore.exceptions import ClientError


sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


TEST_JOB_ID = "test_renders_finalize_pixel_bins"
TEST_ARTIFACT_ID = "test_fixture"
TEST_PIXEL_BINS_KEY = (
    f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/pixel_bins/tile_0000.bin"
)
TEST_RAW_KEY = f"renders/{TEST_JOB_ID}/tile_0000.raw"


def _event(**overrides):
    payload = {
        "job_id": TEST_JOB_ID,
        "task_id": "tile_0",
        "tile_idx": 0,
        "n_chunks": 1,
        "tile_w": 2,
        "tile_h": 2,
        "emit_pixel_bins": True,
        "pixel_bins_out_key": TEST_PIXEL_BINS_KEY,
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
            if "--input_format=dense_layers" in self.cmd:
                tile_w = int(next(arg for arg in self.cmd if arg.startswith("--tile_w=")).split("=", 1)[1])
                tile_h = int(next(arg for arg in self.cmd if arg.startswith("--tile_h=")).split("=", 1)[1])
                empty = int(next(arg for arg in self.cmd if arg.startswith("--empty=")).split("=", 1)[1])
                layer_size = tile_w * tile_h
                self.test_case.assertEqual(len(self.stdin.data) % layer_size, 0)
                tile = bytearray([empty] * layer_size)
                for start in range(0, len(self.stdin.data), layer_size):
                    layer = self.stdin.data[start:start + layer_size]
                    for idx, value in enumerate(layer):
                        if value != empty:
                            tile[idx] = value
                with open(out_path, "wb") as fh:
                    fh.write(tile)
                return 0
            with open(out_path, "wb") as fh:
                fh.write(bytes([255, 1, 2, 3]))
        else:
            raise AssertionError(f"unexpected executable {exe}")
        return 0


class TestFinalizePixelBins(unittest.TestCase):

    def test_is_missing_s3_error_is_defensive(self):
        import handler_finalize as mod

        self.assertTrue(mod._is_missing_s3_error(ClientError({"Error": {"Code": "NoSuchKey"}}, "GetObject")))

        class WeirdError(Exception):
            response = {}

        self.assertFalse(mod._is_missing_s3_error(WeirdError("boom")))

    @patch("handler_finalize.report_status")
    @patch("handler_finalize.subprocess.Popen")
    @patch("handler_finalize._finalize_s3_client")
    def test_finalize_uploads_dense_pixel_bin_tile_when_enabled(self, mock_client_factory, mock_popen, mock_report):
        import handler_finalize as mod

        uploads = {}
        fake_s3 = MagicMock()

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == f"renders/{TEST_JOB_ID}/pix_chunk_0000_t0000.pix":
                return {"Body": _Body(b"\x00" * 8)}
            if key == f"renders/{TEST_JOB_ID}/pixbin_chunk_0000_t0000.pbx":
                return {"Body": _Body(b"\x00" * 8)}
            raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        fake_s3.get_object.side_effect = get_object
        fake_s3.upload_fileobj.side_effect = upload_fileobj
        mock_client_factory.return_value = fake_s3
        mock_popen.side_effect = lambda *args, **kwargs: _FakeProc(args[0])

        with tempfile.TemporaryDirectory() as td, \
             patch("handler_finalize.PIXASSEMBLE", "pixassemble"), \
             patch("handler_finalize.PIXBINASSEMBLE", "pixbinassemble"):
            result = mod.handler(_event(), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pixel_bins_key"], TEST_PIXEL_BINS_KEY)
        self.assertIn(TEST_RAW_KEY, uploads)
        self.assertIn(TEST_PIXEL_BINS_KEY, uploads)
        self.assertEqual(uploads[TEST_PIXEL_BINS_KEY], bytes([255, 1, 2, 3]))
        mock_client_factory.assert_called_once_with(1)
        reports = {call.args[2]: call.kwargs["result_data"] for call in mock_report.call_args_list}
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertIn("started", statuses)
        self.assertIn("assembling_pix", statuses)
        self.assertIn("assembled", statuses)
        self.assertIn("uploading_raw", statuses)
        self.assertIn("assembling_pbx", statuses)
        self.assertIn("uploading_pbx", statuses)
        self.assertIn("done", statuses)
        started_data = reports["started"]
        assembled_data = reports["assembled"]
        done_data = reports["done"]
        self.assertEqual(started_data["workers"], 1)
        self.assertEqual(started_data["access_mode"], "s3_prefetch_stdin")
        self.assertEqual(started_data["assemble_mode"], "pixassemble_stdin")
        self.assertEqual(started_data["tile_w"], 2)
        self.assertEqual(started_data["tile_h"], 2)
        self.assertEqual(started_data["raw_expected_size"], 24)
        self.assertEqual(assembled_data["pix_files"], 1)
        self.assertEqual(assembled_data["pix_bytes"], 8)
        self.assertEqual(done_data["pixel_bin_files"], 1)
        self.assertEqual(done_data["pixel_bin_bytes"], 8)

    @patch("handler_finalize.report_status")
    @patch("handler_finalize.subprocess.run")
    @patch("handler_finalize.subprocess.Popen")
    @patch("handler_finalize._finalize_s3_client")
    def test_finalize_can_render_rgb_from_pixel_bins_without_pix_fragments(self, mock_client_factory, mock_popen, mock_run, mock_report):
        import handler_finalize as mod

        uploads = {}
        requested_keys = []
        fake_s3 = MagicMock()

        def get_object(**kwargs):
            key = kwargs["Key"]
            requested_keys.append(key)
            if key == f"renders/{TEST_JOB_ID}/pixbin_chunk_0000_t0000.pbx":
                return {"Body": _Body(b"\x05" * 8)}
            raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None):
            self.assertEqual(os.path.basename(cmd[0]), "pixel_bins_render")
            self.assertEqual(cmd[1], "/tmp/tile.pixel_bins.bin")
            self.assertEqual(cmd[2], "/tmp/tile.raw")
            self.assertIn("--palette=tri_redgold", cmd)
            with open(cmd[2], "wb") as fh:
                fh.write(struct.pack("<III", 2, 2, 3))
                fh.write(b"\x10\x20\x30" * 4)
            return MagicMock(returncode=0, stderr="")

        fake_s3.get_object.side_effect = get_object
        fake_s3.upload_fileobj.side_effect = upload_fileobj
        mock_client_factory.return_value = fake_s3
        mock_popen.side_effect = lambda *args, **kwargs: _FakeProc(args[0])
        mock_run.side_effect = run_side_effect

        with tempfile.TemporaryDirectory() as td, \
             patch("handler_finalize.PIXBINASSEMBLE", "pixbinassemble"), \
             patch("handler_finalize.PIXEL_BINS_RENDER", "pixel_bins_render"):
            result = mod.handler(_event(
                pixel_bins_drive_rgb=True,
                palette="tri_redgold",
                background_color="001122",
                pixel_bins_empty=255,
            ), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pix_files"], 0)
        self.assertEqual(body["pixel_bins_key"], TEST_PIXEL_BINS_KEY)
        self.assertTrue(body["pixel_bins_drive_rgb"])
        self.assertEqual(body["rgb_source"], "pixel_bins")
        self.assertIn(TEST_RAW_KEY, uploads)
        self.assertIn(TEST_PIXEL_BINS_KEY, uploads)
        self.assertEqual(uploads[TEST_PIXEL_BINS_KEY], bytes([255, 1, 2, 3]))
        self.assertEqual(uploads[TEST_RAW_KEY], struct.pack("<III", 2, 2, 3) + b"\x10\x20\x30" * 4)
        self.assertNotIn(f"renders/{TEST_JOB_ID}/pix_chunk_0000_t0000.pix", requested_keys)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertIn("assembling_pbx", statuses)
        self.assertIn("uploading_pbx", statuses)
        self.assertIn("rendering_rgb_from_bins", statuses)
        self.assertNotIn("assembling_pix", statuses)
        done_data = mock_report.call_args_list[-1].kwargs["result_data"]
        self.assertEqual(done_data["assemble_mode"], "pixel_bins_render")
        self.assertEqual(done_data["rgb_source"], "pixel_bins")
        self.assertEqual(done_data["pixel_bin_files"], 1)
        self.assertEqual(done_data["pixel_bin_bytes"], 8)
        self.assertEqual(done_data["pix_files"], 0)

    @patch("handler_finalize.report_status")
    @patch("handler_finalize.subprocess.run")
    @patch("handler_finalize.subprocess.Popen")
    @patch("handler_finalize._finalize_s3_client")
    def test_finalize_derives_tile_shape_and_pixel_bin_key_from_prefix(self, mock_client_factory, mock_popen, mock_run, mock_report):
        import handler_finalize as mod

        uploads = {}
        fake_s3 = MagicMock()
        pixel_bin_prefix = f"renders/{TEST_JOB_ID}/color/{TEST_ARTIFACT_ID}/pixel_bins/tile_"
        expected_key = f"{pixel_bin_prefix}0001.bin"

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key == f"renders/{TEST_JOB_ID}/pixbin_chunk_0000_t0001.pbx":
                return {"Body": _Body(b"\x05" * 4)}
            raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None):
            self.assertEqual(os.path.basename(cmd[0]), "pixel_bins_render")
            self.assertIn("--tile_w=1", cmd)
            self.assertIn("--tile_h=2", cmd)
            with open(cmd[2], "wb") as fh:
                fh.write(struct.pack("<III", 1, 2, 3))
                fh.write(b"\xaa\xbb\xcc" * 2)
            return MagicMock(returncode=0, stderr="")

        fake_s3.get_object.side_effect = get_object
        fake_s3.upload_fileobj.side_effect = upload_fileobj
        mock_client_factory.return_value = fake_s3
        mock_popen.side_effect = lambda *args, **kwargs: _FakeProc(args[0])
        mock_run.side_effect = run_side_effect

        result = mod.handler(_event(
            tile_idx=1,
            tile_w=None,
            tile_h=None,
            width=3,
            height=3,
            tile_size=2,
            n_tile_cols=2,
            n_tile_rows=2,
            pixel_bins_out_key="",
            pixel_bins_out_prefix=pixel_bin_prefix,
            pixel_bins_drive_rgb=True,
        ), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pixel_bins_key"], expected_key)
        self.assertIn(expected_key, uploads)
        started_data = mock_report.call_args_list[0].kwargs["result_data"]
        self.assertEqual(started_data["tile_w"], 1)
        self.assertEqual(started_data["tile_h"], 2)

    @patch("handler_finalize.report_status")
    @patch("handler_finalize.subprocess.run")
    @patch("handler_finalize.subprocess.Popen")
    @patch("handler_finalize._finalize_s3_client")
    def test_finalize_can_render_rgb_from_dense_group_pixel_bins(self, mock_client_factory, mock_popen, mock_run, mock_report):
        import handler_finalize as mod

        uploads = {}
        requested_keys = []
        procs = []
        fake_s3 = MagicMock()

        def get_object(**kwargs):
            key = kwargs["Key"]
            requested_keys.append(key)
            if key == f"renders/{TEST_JOB_ID}/pixbin_group_0000_t0000.u8":
                return {"Body": _Body(bytes([255, 1, 255, 3]))}
            if key == f"renders/{TEST_JOB_ID}/pixbin_group_0001_t0000.u8":
                return {"Body": _Body(bytes([2, 255, 4, 255]))}
            raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        def popen_side_effect(*args, **kwargs):
            proc = _FakeProc(args[0])
            proc.test_case = self
            procs.append(proc)
            return proc

        def run_side_effect(cmd, capture_output=False, text=False, timeout=None):
            self.assertEqual(os.path.basename(cmd[0]), "pixel_bins_render")
            with open(cmd[2], "wb") as fh:
                fh.write(struct.pack("<III", 2, 2, 3))
                fh.write(b"\xaa\xbb\xcc" * 4)
            return MagicMock(returncode=0, stderr="")

        fake_s3.get_object.side_effect = get_object
        fake_s3.upload_fileobj.side_effect = upload_fileobj
        mock_client_factory.return_value = fake_s3
        mock_popen.side_effect = popen_side_effect
        mock_run.side_effect = run_side_effect

        with tempfile.TemporaryDirectory() as td, \
             patch("handler_finalize.PIXBINASSEMBLE", "pixbinassemble"), \
             patch("handler_finalize.PIXEL_BINS_RENDER", "pixel_bins_render"):
            result = mod.handler(_event(
                pixel_bins_drive_rgb=True,
                pixel_bin_fragment_mode="dense_grouped",
                raster_item_count=2,
                n_chunks=4,
                pixel_bins_empty=255,
            ), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pixel_bin_fragment_mode"], "dense_grouped")
        self.assertEqual(body["raster_item_count"], 2)
        self.assertEqual(uploads[TEST_PIXEL_BINS_KEY], bytes([2, 1, 4, 3]))
        self.assertEqual(procs[0].stdin.data, bytearray([255, 1, 255, 3, 2, 255, 4, 255]))
        self.assertIn(f"renders/{TEST_JOB_ID}/pixbin_group_0000_t0000.u8", requested_keys)
        self.assertIn(f"renders/{TEST_JOB_ID}/pixbin_group_0001_t0000.u8", requested_keys)
        self.assertNotIn(f"renders/{TEST_JOB_ID}/pixbin_chunk_0000_t0000.pbx", requested_keys)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertIn("assembling_dense_pbx", statuses)
        self.assertIn("uploading_dense_pbx", statuses)
        done_data = mock_report.call_args_list[-1].kwargs["result_data"]
        self.assertEqual(done_data["pixel_bin_fragment_mode"], "dense_grouped")
        self.assertEqual(done_data["pixel_bin_files"], 2)
        self.assertEqual(done_data["pixel_bin_bytes"], 8)

    @patch("handler_finalize.report_status")
    @patch("handler_finalize.subprocess.Popen")
    @patch("handler_finalize._finalize_s3_client")
    def test_finalize_preserves_chunk_order_under_concurrent_prefetch(self, mock_client_factory, mock_popen, mock_report):
        import handler_finalize as mod

        uploads = {}
        fake_s3 = MagicMock()
        procs = []
        pix_payloads = {
            f"renders/{TEST_JOB_ID}/pix_chunk_0000_t0000.pix": b"A" * 8,
            f"renders/{TEST_JOB_ID}/pix_chunk_0001_t0000.pix": b"B" * 8,
            f"renders/{TEST_JOB_ID}/pix_chunk_0002_t0000.pix": b"C" * 8,
        }
        delays = {
            f"renders/{TEST_JOB_ID}/pix_chunk_0000_t0000.pix": 0.03,
            f"renders/{TEST_JOB_ID}/pix_chunk_0001_t0000.pix": 0.01,
            f"renders/{TEST_JOB_ID}/pix_chunk_0002_t0000.pix": 0.0,
        }

        def get_object(**kwargs):
            key = kwargs["Key"]
            if key in pix_payloads:
                time.sleep(delays[key])
                return {"Body": _Body(pix_payloads[key])}
            raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        def popen_side_effect(*args, **kwargs):
            proc = _FakeProc(args[0])
            procs.append(proc)
            return proc

        fake_s3.get_object.side_effect = get_object
        fake_s3.upload_fileobj.side_effect = upload_fileobj
        mock_client_factory.return_value = fake_s3
        mock_popen.side_effect = popen_side_effect

        with tempfile.TemporaryDirectory() as td, \
             patch("handler_finalize.PIXASSEMBLE", "pixassemble"), \
             patch("handler_finalize.PIXBINASSEMBLE", "pixbinassemble"):
            result = mod.handler(_event(n_chunks=3, emit_pixel_bins=False, pixel_bins_out_key=""), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pix_files"], 3)
        self.assertIn(TEST_RAW_KEY, uploads)
        self.assertGreaterEqual(len(procs), 1)
        self.assertEqual(procs[0].stdin.data, b"A" * 8 + b"B" * 8 + b"C" * 8)
        mock_client_factory.assert_called_once_with(3)
        statuses = [call.args[2] for call in mock_report.call_args_list]
        self.assertIn("started", statuses)
        self.assertIn("assembling_pix", statuses)
        self.assertIn("assembled", statuses)
        self.assertIn("uploading_raw", statuses)
        self.assertIn("done", statuses)

    @patch("handler_finalize.report_status")
    @patch("handler_finalize.subprocess.Popen")
    @patch("handler_finalize._finalize_s3_client")
    def test_finalize_does_not_fallback_to_legacy_pix_keys(self, mock_client_factory, mock_popen, mock_report):
        import handler_finalize as mod

        uploads = {}
        requested_keys = []
        fake_s3 = MagicMock()

        def get_object(**kwargs):
            key = kwargs["Key"]
            requested_keys.append(key)
            if key == f"renders/{TEST_JOB_ID}/pix_chunk_0000_t0000.pix":
                raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")
            if key == f"renders/{TEST_JOB_ID}/pix_0000_t0000.pix":
                return {"Body": _Body(b"\x99" * 8)}
            raise ClientError({"Error": {"Code": "NoSuchKey", "Message": "missing"}}, "GetObject")

        def upload_fileobj(fileobj, bucket, key):
            uploads[key] = fileobj.read()

        fake_s3.get_object.side_effect = get_object
        fake_s3.upload_fileobj.side_effect = upload_fileobj
        mock_client_factory.return_value = fake_s3
        mock_popen.side_effect = lambda *args, **kwargs: _FakeProc(args[0])

        with tempfile.TemporaryDirectory() as td, \
             patch("handler_finalize.PIXASSEMBLE", "pixassemble"):
            result = mod.handler(_event(emit_pixel_bins=False, pixel_bins_out_key=""), None)

        body = json.loads(result["body"])
        self.assertEqual(body["pix_files"], 0)
        self.assertIn(TEST_RAW_KEY, uploads)
        self.assertIn(f"renders/{TEST_JOB_ID}/pix_chunk_0000_t0000.pix", requested_keys)
        self.assertNotIn(f"renders/{TEST_JOB_ID}/pix_0000_t0000.pix", requested_keys)


if __name__ == "__main__":
    unittest.main()
