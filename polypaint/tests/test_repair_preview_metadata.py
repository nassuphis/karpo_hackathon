import importlib.util
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "repair_preview_metadata.py"
sys.path.insert(0, str(ROOT / "lambda"))


def _load_script():
    spec = importlib.util.spec_from_file_location("repair_preview_metadata", SCRIPT)
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
        self.objects = {}
        self.metadata = {}
        self.copy_calls = []

    def head_object(self, Bucket=None, Key=None):
        return {
            "ContentType": "image/png",
            "CacheControl": "no-cache",
            "Metadata": dict(self.metadata.get(Key, {})),
        }

    def get_object(self, Bucket=None, Key=None, Range=None):
        return {"Body": _Body(self.objects[Key][:33])}

    def copy_object(self, **kwargs):
        self.copy_calls.append(kwargs)
        key = kwargs["Key"]
        self.metadata[key] = dict(kwargs["Metadata"])
        return {}


class RepairPreviewMetadataTests(unittest.TestCase):
    def test_repair_key_uses_png_header_and_preserves_unrelated_metadata(self):
        mod = _load_script()
        s3 = _FakeS3()
        key = "renders/job/color/art/preview.png"
        s3.objects[key] = _png_header(512, 512)
        s3.metadata[key] = {
            "width": "4000",
            "height": "4000",
            "pix": "4000",
            "other": "keep",
        }

        status, _, old_meta, repaired = mod._repair_key(
            s3,
            "bucket",
            key,
            apply=True,
            fill_missing=False,
        )

        self.assertEqual(status, "repaired")
        self.assertEqual(old_meta["width"], "4000")
        self.assertEqual(repaired["width"], "512")
        self.assertEqual(repaired["height"], "512")
        self.assertEqual(repaired["pix"], "512")
        self.assertEqual(repaired["other"], "keep")
        self.assertEqual(len(s3.copy_calls), 1)
        call = s3.copy_calls[0]
        self.assertEqual(call["MetadataDirective"], "REPLACE")
        self.assertEqual(call["ContentType"], "image/png")
        self.assertEqual(call["CacheControl"], "no-cache")

    def test_default_does_not_add_missing_dimension_metadata(self):
        mod = _load_script()
        needs, repaired = mod._metadata_needs_repair(
            {"other": "keep"},
            512,
            512,
            fill_missing=False,
        )

        self.assertFalse(needs)
        self.assertEqual(repaired, {"other": "keep"})

    def test_fill_missing_adds_dimensions(self):
        mod = _load_script()
        needs, repaired = mod._metadata_needs_repair(
            {"other": "keep"},
            512,
            512,
            fill_missing=True,
        )

        self.assertTrue(needs)
        self.assertEqual(repaired["width"], "512")
        self.assertEqual(repaired["height"], "512")
        self.assertEqual(repaired["pix"], "512")
        self.assertEqual(repaired["other"], "keep")


if __name__ == "__main__":
    unittest.main()
