import importlib.util
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "backfill_cache_headers.py"


def _load_script():
    spec = importlib.util.spec_from_file_location("backfill_cache_headers", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _png_header(width=512, height=512):
    return (
        b"\x89PNG\r\n\x1a\n"
        + (13).to_bytes(4, "big")
        + b"IHDR"
        + int(width).to_bytes(4, "big")
        + int(height).to_bytes(4, "big")
    )


class _Body:
    def __init__(self, data):
        self.data = data

    def read(self):
        return self.data


class _FakeS3:
    def __init__(self):
        self.heads = {}
        self.bodies = {}
        self.copy_calls = []
        self.prefixes = {}

    def head_object(self, Bucket=None, Key=None):
        return dict(self.heads[Key])

    def get_object(self, Bucket=None, Key=None, Range=None):
        return {"Body": _Body(self.bodies[Key][:33])}

    def copy_object(self, **kwargs):
        self.copy_calls.append(kwargs)
        return {}

    def get_paginator(self, name):
        fake = self

        class _Paginator:
            def paginate(self, Bucket=None, Prefix=None, Delimiter=None):
                if Delimiter == "/":
                    children = fake.prefixes.get(Prefix, [])
                    yield {"CommonPrefixes": [{"Prefix": c} for c in children]}
                else:
                    keys = [k for k in sorted(fake.heads) if k.startswith(Prefix or "")]
                    yield {"Contents": [{"Key": k} for k in keys]}

        return _Paginator()


IMMUTABLE = "public, max-age=31536000, immutable"


class BackfillCacheHeadersTests(unittest.TestCase):
    def setUp(self):
        self.mod = _load_script()
        self.s3 = _FakeS3()

    def test_preview_copy_payload_repairs_legacy_dims(self):
        key = "renders/j1/color/art1/preview.png"
        self.s3.heads[key] = {
            "ContentLength": 700000,
            "ContentType": "image/png",
            "Metadata": {"width": "5000", "height": "5000", "pix": "5000", "palette": "tri"},
        }
        self.s3.bodies[key] = _png_header(1024, 1024)

        status, note = self.mod.process_key(self.s3, "b", key, apply=True, fill_missing=False)

        self.assertEqual(status, "copied")
        self.assertIn("5000x5000 -> 1024x1024", note)
        self.assertEqual(len(self.s3.copy_calls), 1)
        call = self.s3.copy_calls[0]
        # exact copy payload (payload-contract): headers + honest dims,
        # non-dimension metadata preserved
        self.assertEqual(call["CopySource"], {"Bucket": "b", "Key": key})
        self.assertEqual(call["MetadataDirective"], "REPLACE")
        self.assertEqual(call["CacheControl"], IMMUTABLE)
        self.assertEqual(call["ContentType"], "image/png")
        self.assertEqual(call["Metadata"],
                         {"width": "1024", "height": "1024", "pix": "1024", "palette": "tri"})

    def test_skips_object_already_done(self):
        key = "renders/j1/color/art1/preview.png"
        self.s3.heads[key] = {
            "ContentLength": 1000,
            "ContentType": "image/png",
            "CacheControl": IMMUTABLE,
            "Metadata": {"width": "512", "height": "512", "pix": "512"},
        }
        self.s3.bodies[key] = _png_header(512, 512)

        status, _ = self.mod.process_key(self.s3, "b", key, apply=True, fill_missing=False)

        self.assertEqual(status, "ok")
        self.assertEqual(self.s3.copy_calls, [])

    def test_dry_run_never_copies(self):
        key = "renders/j1/palettes/pal1/preview.png"
        self.s3.heads[key] = {"ContentLength": 1000, "ContentType": "image/png", "Metadata": {}}
        self.s3.bodies[key] = _png_header(512, 512)

        status, _ = self.mod.process_key(self.s3, "b", key, apply=False, fill_missing=False)

        self.assertEqual(status, "would_copy")  # header still missing
        self.assertEqual(self.s3.copy_calls, [])

    def test_preview_without_dim_metadata_left_alone_unless_fill_missing(self):
        key = "renders/j1/palettes/pal1/preview.png"
        self.s3.heads[key] = {
            "ContentLength": 1000,
            "ContentType": "image/png",
            "CacheControl": IMMUTABLE,
            "Metadata": {"palette": "tri"},
        }
        self.s3.bodies[key] = _png_header(500, 500)

        status, _ = self.mod.process_key(self.s3, "b", key, apply=True, fill_missing=False)
        self.assertEqual(status, "ok")
        self.assertEqual(self.s3.copy_calls, [])

        status, _ = self.mod.process_key(self.s3, "b", key, apply=True, fill_missing=True)
        self.assertEqual(status, "copied")
        self.assertEqual(self.s3.copy_calls[0]["Metadata"],
                         {"palette": "tri", "width": "500", "height": "500", "pix": "500"})

    def test_dzi_and_viewer_metadata_pass_through(self):
        key = "deepzoom/j1/dz_1/viewer.html"
        self.s3.heads[key] = {
            "ContentLength": 4000,
            "ContentType": "text/html; charset=utf-8",
            "Metadata": {},
        }

        status, _ = self.mod.process_key(self.s3, "b", key, apply=True, fill_missing=False)

        self.assertEqual(status, "copied")
        call = self.s3.copy_calls[0]
        self.assertEqual(call["ContentType"], "text/html; charset=utf-8")
        self.assertEqual(call["CacheControl"], IMMUTABLE)
        self.assertEqual(call["Metadata"], {})

    def test_default_targets_exclude_mutable_pointers(self):
        self.s3.prefixes = {
            "renders/": ["renders/j1/", "renders/_index/"],
            "renders/j1/color/": ["renders/j1/color/art1/"],
            "renders/j1/palettes/": ["renders/j1/palettes/pal1/"],
            "deepzoom/": ["deepzoom/j1/"],
            "deepzoom/j1/": ["deepzoom/j1/dz_1/"],
        }
        keys = list(self.mod.iter_target_keys(self.s3, "b", ["previews", "deepzoom-meta"]))
        self.assertEqual(keys, [
            "renders/j1/color/art1/preview.png",
            "renders/j1/palettes/pal1/preview.png",
            "deepzoom/j1/dz_1/image.dzi",
            "deepzoom/j1/dz_1/viewer.html",
        ])
        # meta.json / deepzoom_latest.json pointers are mutable: never targeted
        self.assertFalse(any(k.endswith("meta.json") for k in keys))

    def test_tile_scope_filters_to_image_files(self):
        self.s3.heads = {
            "deepzoom/j1/dz_1/image_files/12/3_4.png": {},
            "deepzoom/j1/dz_1/image.dzi": {},
            "deepzoom/j1/dz_1/meta.json": {},
        }
        keys = list(self.mod.iter_deepzoom_tile_keys(self.s3, "b"))
        self.assertEqual(keys, ["deepzoom/j1/dz_1/image_files/12/3_4.png"])


if __name__ == "__main__":
    unittest.main()
