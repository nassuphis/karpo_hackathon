import importlib.util
import io
import json
import sys
import unittest
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "migrate_preview_jpg.py"


def _load_script():
    spec = importlib.util.spec_from_file_location("migrate_preview_jpg", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _png_bytes(width, height, mode="RGB"):
    img = Image.new(mode, (width, height), (200, 40, 90) if mode == "RGB" else 3)
    out = io.BytesIO()
    img.save(out, format="PNG")
    return out.getvalue()


class _Body:
    def __init__(self, data):
        self.data = data

    def read(self):
        return self.data


class _NotFound(Exception):
    def __init__(self):
        self.response = {"Error": {"Code": "404"}}


class _FakeS3:
    def __init__(self):
        self.objects = {}
        self.heads = {}
        self.put_calls = []

    def head_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise _NotFound()
        return dict(self.heads.get(Key, {}))

    def get_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise _NotFound()
        return {"Body": _Body(self.objects[Key])}

    def put_object(self, **kwargs):
        self.put_calls.append(kwargs)
        self.objects[kwargs["Key"]] = kwargs["Body"]
        self.heads[kwargs["Key"]] = {"Metadata": dict(kwargs.get("Metadata") or {})}
        return {}


IMMUTABLE = "public, max-age=31536000, immutable"
COLOR_PREFIX = "renders/j1/color/art1/"
PAL_PREFIX = "renders/j1/palettes/pal1/"


class ConvertTests(unittest.TestCase):
    def setUp(self):
        self.mod = _load_script()
        # make botocore's ClientError check match our fake exception
        self.mod.ClientError = _NotFound

    def test_downscales_1024_to_512_jpeg(self):
        jpg, size, src = self.mod.convert_png_to_jpg(_png_bytes(1024, 1024))
        self.assertEqual(size, (512, 512))
        self.assertEqual(src, (1024, 1024))
        img = Image.open(io.BytesIO(jpg))
        self.assertEqual(img.format, "JPEG")
        self.assertEqual(img.size, (512, 512))

    def test_never_upscales(self):
        _, size, _ = self.mod.convert_png_to_jpg(_png_bytes(500, 500))
        self.assertEqual(size, (500, 500))

    def test_palette_mode_png_converts(self):
        jpg, size, _ = self.mod.convert_png_to_jpg(_png_bytes(64, 64, mode="P"))
        self.assertEqual(Image.open(io.BytesIO(jpg)).mode, "RGB")
        self.assertEqual(size, (64, 64))


class ProcessArtifactTests(unittest.TestCase):
    def setUp(self):
        self.mod = _load_script()
        self.mod.ClientError = _NotFound
        self.s3 = _FakeS3()

    def _seed_color(self, png_size=1024, overlay=None):
        self.s3.objects[COLOR_PREFIX + "preview.png"] = _png_bytes(png_size, png_size)
        if overlay is not None:
            self.s3.objects[COLOR_PREFIX + "meta.json"] = json.dumps(overlay).encode()

    def test_color_apply_writes_jpg_and_merges_overlay(self):
        self._seed_color(overlay={"palette": "tri", "created_at": "2026-01-01T00:00:00Z"})

        status, note = self.mod.process_artifact(
            self.s3, "b", "color", COLOR_PREFIX, apply=True)

        self.assertEqual(status, "converted")
        self.assertIn("1024x1024 png", note)
        jpg_put = next(c for c in self.s3.put_calls if c["Key"].endswith("preview.jpg"))
        # exact upload payload (payload-contract)
        self.assertEqual(jpg_put["ContentType"], "image/jpeg")
        self.assertEqual(jpg_put["CacheControl"], IMMUTABLE)
        self.assertEqual(jpg_put["Metadata"], {"width": "512", "height": "512", "pix": "512"})
        self.assertEqual(Image.open(io.BytesIO(jpg_put["Body"])).size, (512, 512))
        meta_put = next(c for c in self.s3.put_calls if c["Key"].endswith("meta.json"))
        merged = json.loads(meta_put["Body"])
        # additive: pre-existing overlay fields survive
        self.assertEqual(merged["palette"], "tri")
        self.assertEqual(merged["created_at"], "2026-01-01T00:00:00Z")
        self.assertEqual(merged["preview_jpg_key"], COLOR_PREFIX + "preview.jpg")
        self.assertEqual(merged["preview_jpg_width"], "512")
        self.assertEqual(merged["preview_jpg_height"], "512")
        self.assertEqual(meta_put["ContentType"], "application/json")

    def test_color_without_overlay_creates_minimal_one(self):
        self._seed_color(overlay=None)
        status, _ = self.mod.process_artifact(self.s3, "b", "color", COLOR_PREFIX, apply=True)
        self.assertEqual(status, "converted")
        merged = json.loads(self.s3.objects[COLOR_PREFIX + "meta.json"])
        self.assertEqual(set(merged), {"preview_jpg_key", "preview_jpg_width", "preview_jpg_height"})

    def test_palette_without_meta_is_skipped(self):
        self.s3.objects[PAL_PREFIX + "preview.png"] = _png_bytes(512, 512)
        status, _ = self.mod.process_artifact(self.s3, "b", "palettes", PAL_PREFIX, apply=True)
        self.assertEqual(status, "no_meta")
        self.assertEqual(self.s3.put_calls, [])

    def test_palette_meta_preserves_native_types(self):
        self.s3.objects[PAL_PREFIX + "preview.png"] = _png_bytes(512, 512)
        self.s3.objects[PAL_PREFIX + "meta.json"] = json.dumps(
            {"file_size": 123, "solve_score_chain": [{"op": "m0"}]}).encode()

        status, _ = self.mod.process_artifact(self.s3, "b", "palettes", PAL_PREFIX, apply=True)

        self.assertEqual(status, "converted")
        merged = json.loads(self.s3.objects[PAL_PREFIX + "meta.json"])
        self.assertEqual(merged["file_size"], 123)
        self.assertEqual(merged["solve_score_chain"], [{"op": "m0"}])
        self.assertEqual(merged["preview_jpg_width"], "512")

    def test_idempotent_when_jpg_and_meta_exist(self):
        self._seed_color(overlay={"preview_jpg_key": COLOR_PREFIX + "preview.jpg",
                                  "preview_jpg_width": "512", "preview_jpg_height": "512"})
        self.s3.objects[COLOR_PREFIX + "preview.jpg"] = b"jpg"
        self.s3.heads[COLOR_PREFIX + "preview.jpg"] = {"Metadata": {"width": "512", "height": "512"}}

        status, _ = self.mod.process_artifact(self.s3, "b", "color", COLOR_PREFIX, apply=True)

        self.assertEqual(status, "ok")
        self.assertEqual(self.s3.put_calls, [])

    def test_interrupted_run_repairs_meta_only(self):
        self._seed_color(overlay={"palette": "tri"})
        self.s3.objects[COLOR_PREFIX + "preview.jpg"] = b"jpg"
        self.s3.heads[COLOR_PREFIX + "preview.jpg"] = {"Metadata": {"width": "512", "height": "512"}}

        status, _ = self.mod.process_artifact(self.s3, "b", "color", COLOR_PREFIX, apply=True)

        self.assertEqual(status, "meta_repaired")
        self.assertEqual(len(self.s3.put_calls), 1)
        merged = json.loads(self.s3.put_calls[0]["Body"])
        self.assertEqual(merged["palette"], "tri")
        self.assertEqual(merged["preview_jpg_key"], COLOR_PREFIX + "preview.jpg")

    def test_dry_run_writes_nothing_and_skips_conversion(self):
        self._seed_color(overlay={})
        self.s3.heads[COLOR_PREFIX + "preview.png"] = {"ContentLength": 700000}
        real_get = self.s3.get_object

        def guarded_get(Bucket=None, Key=None, **kwargs):
            if Key.endswith("preview.png"):
                raise AssertionError("dry-run must not download the png")
            return real_get(Bucket=Bucket, Key=Key, **kwargs)

        self.s3.get_object = guarded_get
        status, note = self.mod.process_artifact(self.s3, "b", "color", COLOR_PREFIX, apply=False)
        self.assertEqual(status, "would_convert")
        self.assertIn("png 683KB (conversion deferred", note)
        self.assertEqual(self.s3.put_calls, [])

    def test_skip_escape_hatch(self):
        self._seed_color(overlay={})
        status, _ = self.mod.process_artifact(
            self.s3, "b", "color", COLOR_PREFIX, apply=True,
            skip_keys={COLOR_PREFIX + "preview.png"})
        self.assertEqual(status, "skipped")
        self.assertEqual(self.s3.put_calls, [])

    def test_truncated_stream_retries_then_converts(self):
        self._seed_color(overlay={})
        real_get = self.s3.get_object
        attempts = []

        def flaky_get(Bucket=None, Key=None, **kwargs):
            resp = real_get(Bucket=Bucket, Key=Key, **kwargs)
            if Key.endswith("preview.png"):
                attempts.append(Key)
                if len(attempts) == 1:
                    data = resp["Body"].read()
                    return {"Body": _Body(data[: len(data) // 2]), "ContentLength": len(data)}
                resp["ContentLength"] = len(self.s3.objects[Key])
            return resp

        self.s3.get_object = flaky_get
        status, _ = self.mod.process_artifact(self.s3, "b", "color", COLOR_PREFIX, apply=True)
        self.assertEqual(status, "converted")
        self.assertEqual(len(attempts), 2)

    def test_damaged_at_rest_png_recovers_via_tolerant_decode(self):
        # intact wrapper, damaged tail: strict decode refuses, tolerant
        # recovers (the compute_mobo9or2 signature)
        healthy = _png_bytes(64, 64)
        damaged = healthy[:-30]
        self.s3.objects[COLOR_PREFIX + "preview.png"] = damaged
        self.s3.objects[COLOR_PREFIX + "meta.json"] = b"{}"
        real_get = self.s3.get_object

        def sized_get(Bucket=None, Key=None, **kwargs):
            resp = real_get(Bucket=Bucket, Key=Key, **kwargs)
            if Key.endswith("preview.png"):
                resp["ContentLength"] = len(damaged)
            return resp

        self.s3.get_object = sized_get
        status, note = self.mod.process_artifact(self.s3, "b", "color", COLOR_PREFIX, apply=True)
        self.assertEqual(status, "converted", note)
        self.assertIn("RECOVERED via tolerant decode", note)
        jpg_put = next(c for c in self.s3.put_calls if c["Key"].endswith("preview.jpg"))
        self.assertEqual(Image.open(io.BytesIO(jpg_put["Body"])).size, (64, 64))
        # the global tolerant flag must be restored afterwards
        from PIL import ImageFile
        self.assertFalse(ImageFile.LOAD_TRUNCATED_IMAGES)

    def test_genuinely_corrupt_png_reports_after_retries(self):
        self.s3.objects[COLOR_PREFIX + "preview.png"] = b"\x89PNG not really"
        self.s3.objects[COLOR_PREFIX + "meta.json"] = b"{}"
        status, note = self.mod.process_artifact(self.s3, "b", "color", COLOR_PREFIX, apply=True)
        self.assertEqual(status, "invalid_image")
        self.assertTrue(note)
        self.assertFalse(any(c["Key"].endswith("preview.jpg") for c in self.s3.put_calls))

    def test_verify_reports_gaps(self):
        self._seed_color(overlay={})
        status, _ = self.mod.verify_artifact(self.s3, "b", "color", COLOR_PREFIX)
        self.assertEqual(status, "missing_jpg")

        self.mod.process_artifact(self.s3, "b", "color", COLOR_PREFIX, apply=True)
        status, note = self.mod.verify_artifact(self.s3, "b", "color", COLOR_PREFIX)
        self.assertEqual(status, "ok", note)


if __name__ == "__main__":
    unittest.main()
