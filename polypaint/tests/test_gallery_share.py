"""/share-gallery route contract (virtual-gallery.md §3.1).

Pins the Phase-0 gallery handoff: validate + enrich color picks by EXACT key
(no scan, no /render-summary, no list_objects_v2), validate a supplied export
exactly (identity + canonical dzi_key + live DZI), assign ordinals after
filtering, write a `virtual_gallery` share manifest, and return `skipped[]`.
"""
import io
import json
import os
import sys
import unittest
from unittest.mock import patch

from botocore.exceptions import ClientError

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

import handler_storage as hs


def _ce(code, status, op="HeadObject"):
    return ClientError(
        {"Error": {"Code": code, "Message": code},
         "ResponseMetadata": {"HTTPStatusCode": status}}, op)


class _FakeS3:
    def __init__(self):
        self.objects = {}      # key -> {Metadata, ContentLength, ContentType, body}
        self.fail = {}         # key -> ClientError
        self.puts = []

    def head_object(self, Bucket=None, Key=None):
        if Key in self.fail:
            raise self.fail[Key]
        if Key not in self.objects:
            raise _ce("NoSuchKey", 404, "HeadObject")
        o = self.objects[Key]
        return {"Metadata": o.get("Metadata", {}), "ContentLength": o.get("ContentLength", 0),
                "ContentType": o.get("ContentType", "")}

    def get_object(self, Bucket=None, Key=None):
        if Key in self.fail:
            raise self.fail[Key]
        if Key not in self.objects:
            raise _ce("NoSuchKey", 404, "GetObject")
        return {"Body": io.BytesIO(self.objects[Key].get("body", b"{}"))}

    def put_object(self, Bucket=None, Key=None, Body=None, ContentType=None, CacheControl=None):
        self.puts.append({"Key": Key, "Body": Body, "ContentType": ContentType,
                          "CacheControl": CacheControl})
        return {}

    def generate_presigned_url(self, *a, **k):
        return "https://example/presigned"

    def get_paginator(self, name):
        raise AssertionError(f"gallery share must not list objects (get_paginator {name!r})")


class GalleryShareTests(unittest.TestCase):
    def setUp(self):
        self.s3 = _FakeS3()
        self._patches = [
            patch.object(hs, "s3", self.s3),
            patch.object(hs, "_results_list_s3_client", return_value=self.s3),
            # any accidental render-summary call must blow up
            patch.object(hs, "handle_render_summary",
                         side_effect=AssertionError("gallery must not call render-summary")),
        ]
        for p in self._patches:
            p.start()

    def tearDown(self):
        for p in self._patches:
            p.stop()

    # ── seeding ──────────────────────────────────────────────────────
    def _seed_color(self, job, art, *, created_at="2026-05-01T00:00:00Z",
                    function="poly_1", degree=8, N=2048, times=1):
        prefix = f"renders/{job}/color/{art}/"
        self.s3.objects[prefix + "image.jpeg"] = {"Metadata": {}, "ContentLength": 1000, "ContentType": "image/jpeg"}
        self.s3.objects[prefix + "preview.png"] = {"Metadata": {}, "ContentLength": 50, "ContentType": "image/png"}
        # overlay with migrated preview.jpg dims (avoids a ranged dim read)
        self.s3.objects[prefix + "meta.json"] = {"body": json.dumps({
            "created_at": created_at, "preview_jpg_key": prefix + "preview.jpg",
            "preview_jpg_width": 512, "preview_jpg_height": 512}).encode()}
        self.s3.objects[f"renders/{job}/calc.json"] = {"body": json.dumps({
            "function": function, "degree": degree, "N": N, "times": times}).encode()}

    def _seed_export(self, job, art, export_id, *, image_key=None, dzi_key=None,
                     source_family="color", source_artifact_id=None):
        image_key = image_key or f"renders/{job}/color/{art}/image.jpeg"
        canonical_dzi = f"deepzoom/{job}/{export_id}/image.dzi"
        self.s3.objects[f"deepzoom/{job}/{export_id}/meta.json"] = {"body": json.dumps({
            "job_id": job, "export_id": export_id, "source_family": source_family,
            "source_artifact_id": source_artifact_id or art, "source_key": image_key,
            "dzi_key": dzi_key or canonical_dzi}).encode()}
        # the DZI object HEADed by validation
        self.s3.objects[canonical_dzi] = {"Metadata": {}, "ContentLength": 200, "ContentType": "application/xml"}

    def _event(self, body):
        return {"body": json.dumps(body)}

    def _call(self, picks, **extra):
        body = {"picks": picks}
        body.update(extra)
        return json.loads(hs.handle_share_gallery(self._event(body))["body"])

    # ── happy path ───────────────────────────────────────────────────
    def test_writes_virtual_gallery_manifest_with_exports(self):
        self._seed_color("jobA", "cA")
        self._seed_export("jobA", "cA", "dz_1")
        self._seed_color("jobB", "cB", function="poly_9", degree=12)
        self._seed_export("jobB", "cB", "dz_2")
        body = self._call([
            {"job_id": "jobA", "artifact_id": "cA", "export_id": "dz_1"},
            {"job_id": "jobB", "artifact_id": "cB", "export_id": "dz_2"},
        ], seed=7)
        self.assertEqual(body["count"], 2)
        self.assertEqual(body["skipped"], [])
        self.assertTrue(body["manifest_url"].endswith("/manifest.json"))
        put = next(p for p in self.s3.puts if p["Key"].endswith("manifest.json"))
        self.assertTrue(put["Key"].startswith("renders/_shared_mosaic/gallery/"))
        self.assertEqual(put["CacheControl"], "public, max-age=31536000, immutable")
        m = json.loads(put["Body"])
        self.assertEqual(m["manifest_type"], "virtual_gallery")
        self.assertEqual(m["document_kind"], "share")
        self.assertEqual(m["artifact_kind"], "color")
        self.assertEqual(m["layout"], {"mode": "auto", "seed": 7})
        self.assertEqual([p["ordinal"] for p in m["pieces"]], [0, 1])
        p0 = m["pieces"][0]
        self.assertEqual(p0["job_id"], "jobA")
        self.assertEqual(p0["image_key"], "renders/jobA/color/cA/image.jpeg")
        self.assertEqual(p0["function"], "poly_1")
        self.assertEqual(p0["deepzoom"]["dzi_key"], "deepzoom/jobA/dz_1/image.dzi")
        self.assertEqual(m["pieces"][1]["degree"], 12)

    def test_zoomless_piece_without_export(self):
        self._seed_color("jobA", "cA")
        body = self._call([{"job_id": "jobA", "artifact_id": "cA"}])
        self.assertEqual(body["count"], 1)
        m = json.loads(next(p for p in self.s3.puts if p["Key"].endswith("manifest.json"))["Body"])
        self.assertIsNone(m["pieces"][0]["deepzoom"])

    # ── skipped[] ────────────────────────────────────────────────────
    def test_missing_artifact_is_skipped_and_ordinals_reassigned(self):
        self._seed_color("jobA", "cA")
        self._seed_export("jobA", "cA", "dz_1")
        body = self._call([
            {"job_id": "jobGONE", "artifact_id": "nope"},                 # missing
            {"job_id": "jobA", "artifact_id": "cA", "export_id": "dz_1"},  # ok
        ])
        self.assertEqual(body["count"], 1)
        self.assertEqual(len(body["skipped"]), 1)
        self.assertEqual(body["skipped"][0]["job_id"], "jobGONE")
        m = json.loads(next(p for p in self.s3.puts if p["Key"].endswith("manifest.json"))["Body"])
        self.assertEqual([p["ordinal"] for p in m["pieces"]], [0])  # reassigned after filtering
        self.assertEqual(m["pieces"][0]["job_id"], "jobA")

    def test_export_identity_mismatch_is_skipped(self):
        self._seed_color("jobA", "cA")
        # export whose meta claims a DIFFERENT source artifact
        self._seed_export("jobA", "cA", "dz_x", source_artifact_id="OTHER")
        body = self._call([{"job_id": "jobA", "artifact_id": "cA", "export_id": "dz_x"}])
        self.assertEqual(body["count"], 0)
        self.assertEqual(body["skipped"][0]["reason"], "export_identity_mismatch")

    def test_export_dzi_key_mismatch_is_skipped(self):
        self._seed_color("jobA", "cA")
        self._seed_export("jobA", "cA", "dz_y", dzi_key="deepzoom/jobA/dz_y/EVIL.dzi")
        body = self._call([{"job_id": "jobA", "artifact_id": "cA", "export_id": "dz_y"}])
        self.assertEqual(body["skipped"][0]["reason"], "export_dzi_key_mismatch")

    def test_export_dzi_absent_is_skipped(self):
        self._seed_color("jobA", "cA")
        self._seed_export("jobA", "cA", "dz_z")
        del self.s3.objects["deepzoom/jobA/dz_z/image.dzi"]  # meta ok, DZI gone
        body = self._call([{"job_id": "jobA", "artifact_id": "cA", "export_id": "dz_z"}])
        self.assertEqual(body["skipped"][0]["reason"], "export_dzi_absent")

    def test_export_not_found_is_skipped(self):
        self._seed_color("jobA", "cA")
        body = self._call([{"job_id": "jobA", "artifact_id": "cA", "export_id": "dz_missing"}])
        self.assertEqual(body["skipped"][0]["reason"], "export_not_found")

    # ── failure / validation ─────────────────────────────────────────
    def test_transient_resolve_error_fails_whole_request(self):
        self._seed_color("jobA", "cA")
        self.s3.fail["renders/jobA/color/cA/image.jpeg"] = _ce("SlowDown", 503)
        body = self._call([{"job_id": "jobA", "artifact_id": "cA"}])
        self.assertIn("error", body)
        self.assertFalse(any(p["Key"].endswith("manifest.json") for p in self.s3.puts))

    def test_dedupes_repeated_pick(self):
        self._seed_color("jobA", "cA")
        body = self._call([
            {"job_id": "jobA", "artifact_id": "cA"},
            {"job_id": "jobA", "artifact_id": "cA"},
        ])
        self.assertEqual(body["count"], 1)

    def test_rejects_bad_ids(self):
        for bad in ({"job_id": "../evil", "artifact_id": "cA"},
                    {"job_id": "jobA", "artifact_id": "a/b"}):
            with self.assertRaises(ValueError):
                hs.handle_share_gallery(self._event({"picks": [bad]}))

    def test_rejects_empty_and_oversized(self):
        self.assertIn("error", self._call([]))
        big = [{"job_id": f"j{i}", "artifact_id": f"c{i}"} for i in range(hs.GALLERY_MAX_PIECES + 1)]
        self.assertIn("error", self._call(big))

    def test_all_skipped_returns_error_no_manifest(self):
        body = self._call([{"job_id": "jobGONE", "artifact_id": "nope"}])
        self.assertIn("error", body)
        self.assertEqual(body["count"], 0)
        self.assertEqual(len(body["skipped"]), 1)
        self.assertFalse(any(p["Key"].endswith("manifest.json") for p in self.s3.puts))


if __name__ == "__main__":
    unittest.main()
