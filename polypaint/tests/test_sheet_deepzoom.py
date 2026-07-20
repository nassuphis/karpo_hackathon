"""
Sheet DeepZoom export: handle_sheet_deepzoom builds the pyramid from a
sheets/{id}/sheet.png source constructed SERVER-SIDE (no caller key),
accepts NON-SQUARE mosaics (the render path requires square), and
writes deepzoom/{sheet_id}/{export_id}/meta.json so /list-deepzoom
discovers sheet exports with no listing changes. dz_export is mocked
(linux binary); the S3 stub captures the uploads.
"""
import io
import json
import os
import sys
import unittest

ROOT = os.path.join(os.path.dirname(__file__), "..")
LAMBDA_DIR = os.path.join(ROOT, "lambda")
sys.path.insert(0, LAMBDA_DIR)


class _S3Stub:
    def __init__(self):
        self.objects = {}

    def put_object(self, Bucket, Key, Body, ContentType=None, CacheControl=None):
        self.objects[Key] = Body if isinstance(Body, (bytes, bytearray)) else Body.encode()

    def head_object(self, Bucket, Key):
        if Key not in self.objects:
            from botocore.exceptions import ClientError
            raise ClientError({"Error": {"Code": "404"}}, "HeadObject")
        return {"ContentLength": len(self.objects[Key])}

    def get_object(self, Bucket, Key):
        if Key not in self.objects:
            from botocore.exceptions import ClientError
            raise ClientError({"Error": {"Code": "NoSuchKey"}}, "GetObject")
        return {"Body": io.BytesIO(bytes(self.objects[Key]))}


def _fake_dz_run(cmd, capture_output=True, text=True, timeout=600, env=None):
    """Stand-in for the linux dz_export binary: writes a minimal
    pyramid to the requested base path and reports NON-SQUARE dims."""
    dz_base = cmd[2]
    if cmd[3:] != ["--bilevel"]:
        raise AssertionError(f"sheet DeepZoom did not request bilevel output: {cmd}")
    os.makedirs(os.path.dirname(dz_base), exist_ok=True)
    with open(dz_base + ".dzi", "w") as fh:
        fh.write("<Image/>")
    tiles = os.path.join(dz_base + "_files", "0")
    os.makedirs(tiles, exist_ok=True)
    with open(os.path.join(tiles, "0_0.png"), "wb") as fh:
        fh.write(b"\x89PNG fake tile")

    class _R:
        returncode = 0
        stdout = json.dumps({"width": 1044, "height": 528, "bitdepth": 1})
        stderr = ""
    return _R()


class TestSheetDeepZoom(unittest.TestCase):
    def test_non_square_sheet_export(self):
        import handler_deepzoom_export as mod

        stub = _S3Stub()
        stub.objects["sheets/sheet_abc/sheet.png"] = b"\x89PNG mosaic bytes"
        orig = (mod.s3, mod.subprocess.run, mod.report_status)
        mod.s3 = stub
        mod.subprocess.run = _fake_dz_run
        mod.report_status = lambda *a, **k: None
        try:
            resp = mod.handle_sheet_deepzoom({
                "sheet_id": "sheet_abc",
                "export_id": "dz_test",
            })
        finally:
            (mod.s3, mod.subprocess.run, mod.report_status) = orig

        body = json.loads(resp["body"])
        self.assertEqual(body["source_kind"], "sheet")
        self.assertIn("deepzoom/sheet_abc/dz_test/viewer.html", body["share_url"])

        meta = json.loads(stub.objects["deepzoom/sheet_abc/dz_test/meta.json"])
        self.assertEqual(meta["source_key"], "sheets/sheet_abc/sheet.png")
        self.assertEqual(meta["sheet_id"], "sheet_abc")
        # non-square accepted (the render export path would refuse this)
        self.assertEqual((meta["width"], meta["height"]), (1044, 528))
        self.assertEqual(meta["tile_bitdepth"], 1)
        self.assertIn("deepzoom/sheet_abc/dz_test/viewer.html", stub.objects)
        self.assertIn("deepzoom/sheet_abc/dz_test/image.dzi", stub.objects)
        self.assertIn("deepzoom/sheet_abc/dz_test/image_files/0/0_0.png", stub.objects)
        # sheets never write the render-tab pointer
        self.assertFalse([k for k in stub.objects if k.startswith("renders/")])

    def test_reused_export_id_refused(self):
        import handler_deepzoom_export as mod

        stub = _S3Stub()
        stub.objects["sheets/sheet_abc/sheet.png"] = b"png"
        stub.objects["deepzoom/sheet_abc/dz_test/meta.json"] = b"{}"
        orig = (mod.s3, mod.report_status)
        mod.s3 = stub
        mod.report_status = lambda *a, **k: None
        try:
            with self.assertRaises(RuntimeError) as ctx:
                mod.handle_sheet_deepzoom({
                    "sheet_id": "sheet_abc",
                    "export_id": "dz_test",
                })
        finally:
            (mod.s3, mod.report_status) = orig
        self.assertIn("already exists", str(ctx.exception))

    def test_sheet_id_is_validated(self):
        import handler_deepzoom_export as mod

        orig = mod.report_status
        mod.report_status = lambda *a, **k: None
        try:
            with self.assertRaises(ValueError):
                mod.handle_sheet_deepzoom({"sheet_id": "../evil",
                                           "export_id": "dz_x"})
        finally:
            mod.report_status = orig


if __name__ == "__main__":
    unittest.main()
