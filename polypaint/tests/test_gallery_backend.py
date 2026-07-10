"""Editable-gallery backend contract (virtual-gallery.md §13).

The Gallery tab curates an EDITABLE gallery document (an S3 object with ETag
CAS, like a book); the DeepZoom tab only ADDS to it; "Open Gallery" snapshots it
into the immutable share manifest the viewer loads. Pins:
  create / list / fetch / save (CAS) / delete / add-to-gallery / create-gallery-share.

Enrichment stays exact-key (no scan): get_paginator raising guards that path.
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
    """Models exact-key GET/HEAD/PUT/DELETE + list_objects_v2 and ETag CAS
    (IfMatch / IfNoneMatch -> PreconditionFailed)."""

    def __init__(self):
        self.objects = {}      # key -> {body, etag, Metadata, ContentLength, ContentType}
        self.fail = {}         # key -> ClientError
        self.puts = []
        self._etag = 0

    def head_object(self, Bucket=None, Key=None):
        if Key in self.fail:
            raise self.fail[Key]
        if Key not in self.objects:
            raise _ce("NoSuchKey", 404, "HeadObject")
        o = self.objects[Key]
        return {"Metadata": o.get("Metadata", {}), "ContentLength": o.get("ContentLength", 0),
                "ContentType": o.get("ContentType", ""), "ETag": f'"{o.get("etag", "0")}"'}

    def get_object(self, Bucket=None, Key=None):
        if Key in self.fail:
            raise self.fail[Key]
        if Key not in self.objects:
            raise _ce("NoSuchKey", 404, "GetObject")
        o = self.objects[Key]
        return {"Body": io.BytesIO(o.get("body", b"{}")), "ETag": f'"{o.get("etag", "0")}"'}

    def put_object(self, Bucket=None, Key=None, Body=None, ContentType=None,
                   CacheControl=None, Metadata=None, IfMatch=None, IfNoneMatch=None):
        exists = Key in self.objects
        if IfNoneMatch == "*" and exists:
            raise _ce("PreconditionFailed", 412, "PutObject")
        if IfMatch is not None:
            cur = self.objects.get(Key, {}).get("etag")
            if not exists or str(IfMatch).strip('"') != cur:
                raise _ce("PreconditionFailed", 412, "PutObject")
        self._etag += 1
        etag = str(self._etag)
        body = Body if isinstance(Body, (bytes, bytearray)) else str(Body).encode()
        self.objects[Key] = {"body": body, "etag": etag,
                             "Metadata": Metadata or {}, "ContentType": ContentType or ""}
        self.puts.append({"Key": Key, "Body": Body, "ContentType": ContentType,
                          "CacheControl": CacheControl, "IfMatch": IfMatch, "IfNoneMatch": IfNoneMatch})
        return {"ETag": f'"{etag}"'}

    def delete_object(self, Bucket=None, Key=None):
        self.objects.pop(Key, None)
        return {}

    def list_objects_v2(self, Bucket=None, Prefix=None, ContinuationToken=None):
        keys = sorted(k for k in self.objects if k.startswith(Prefix or ""))
        return {"Contents": [{"Key": k} for k in keys], "IsTruncated": False}

    def generate_presigned_url(self, *a, **k):
        return "https://example/presigned"

    def get_paginator(self, name):
        raise AssertionError(f"gallery enrichment must not paginate (get_paginator {name!r})")


class GalleryBackendTests(unittest.TestCase):
    def setUp(self):
        self.s3 = _FakeS3()
        self._patches = [
            patch.object(hs, "s3", self.s3),
            patch.object(hs, "_results_list_s3_client", return_value=self.s3),
            patch.object(hs, "handle_render_summary",
                         side_effect=AssertionError("gallery must not call render-summary")),
        ]
        for p in self._patches:
            p.start()

    def tearDown(self):
        for p in self._patches:
            p.stop()

    # ── seeding (color artifact + deepzoom export by EXACT key) ──────────
    def _seed_color(self, job, art, *, created_at="2026-05-01T00:00:00Z",
                    function="poly_1", degree=8, N=2048, times=1):
        prefix = f"renders/{job}/color/{art}/"
        self.s3.objects[prefix + "image.jpeg"] = {"Metadata": {}, "ContentLength": 1000, "ContentType": "image/jpeg"}
        self.s3.objects[prefix + "preview.png"] = {"Metadata": {}, "ContentLength": 50, "ContentType": "image/png"}
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
        self.s3.objects[canonical_dzi] = {"Metadata": {}, "ContentLength": 200, "ContentType": "application/xml"}

    def _event(self, body):
        return {"body": json.dumps(body)}

    def _route(self, fn, **body):
        resp = hs._handle_storage_route(fn, self._event(body))
        return resp, json.loads(resp["body"])

    def _create(self, name="My gallery"):
        return json.loads(hs.handle_create_gallery(self._event({"name": name}))["body"])

    def _add(self, gid, job, art, export=None):
        body = {"gallery_id": gid, "job_id": job, "artifact_id": art}
        if export:
            body["export_id"] = export
        return json.loads(hs.handle_add_to_gallery(self._event(body))["body"])

    # ── create / fetch / list ────────────────────────────────────────────
    def test_create_returns_editable_doc(self):
        body = self._create("Selected works")
        g = body["gallery"]
        self.assertEqual(g["document_kind"], "editable")
        self.assertEqual(g["manifest_type"], "virtual_gallery")
        self.assertEqual(g["name"], "Selected works")
        self.assertEqual(g["pieces"], [])
        self.assertTrue(g["gallery_id"].startswith("gallery_"))
        self.assertTrue(body["revision"])
        # created with IfNoneMatch='*' (create-only)
        put = next(p for p in self.s3.puts if p["Key"].endswith(g["gallery_id"] + ".json"))
        self.assertEqual(put["IfNoneMatch"], "*")

    def test_fetch_and_list(self):
        a = self._create("A")["gallery"]["gallery_id"]
        b = self._create("B")["gallery"]["gallery_id"]
        _, fetched = self._route(hs.handle_fetch_gallery, gallery_id=a)
        self.assertEqual(fetched["gallery"]["gallery_id"], a)
        self.assertTrue(fetched["revision"])
        _, listed = self._route(hs.handle_list_galleries)
        ids = {g["gallery_id"] for g in listed["galleries"]}
        self.assertEqual(ids, {a, b})
        self.assertEqual(listed["count"], 2)

    def test_fetch_missing_is_404(self):
        resp, _ = self._route(hs.handle_fetch_gallery, gallery_id="gallery_nope")
        self.assertEqual(resp["statusCode"], 404)

    # ── add-to-gallery ───────────────────────────────────────────────────
    def test_add_enriches_appends_with_deepzoom(self):
        gid = self._create()["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA", function="poly_9", degree=12)
        self._seed_export("jobA", "cA", "dz_1")
        body = self._add(gid, "jobA", "cA", "dz_1")
        self.assertTrue(body["added"])
        pieces = body["gallery"]["pieces"]
        self.assertEqual(len(pieces), 1)
        self.assertEqual(pieces[0]["job_id"], "jobA")
        self.assertEqual(pieces[0]["image_key"], "renders/jobA/color/cA/image.jpeg")
        self.assertEqual(pieces[0]["function"], "poly_9")
        self.assertEqual(pieces[0]["deepzoom"]["dzi_key"], "deepzoom/jobA/dz_1/image.dzi")
        self.assertEqual(pieces[0]["title"], "")
        # the append CAS-saves with IfMatch (concurrent-safe)
        gput = [p for p in self.s3.puts if p["Key"].endswith(gid + ".json")][-1]
        self.assertIsNotNone(gput["IfMatch"])

    def test_add_zoomless_without_export(self):
        gid = self._create()["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA")
        body = self._add(gid, "jobA", "cA")
        self.assertTrue(body["added"])
        self.assertIsNone(body["gallery"]["pieces"][0]["deepzoom"])

    def test_add_duplicate_not_added(self):
        gid = self._create()["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA")
        self._add(gid, "jobA", "cA")
        body = self._add(gid, "jobA", "cA")
        self.assertFalse(body["added"])
        self.assertEqual(body["reason"], "duplicate")
        self.assertEqual(len(body["gallery"]["pieces"]), 1)

    def test_add_missing_artifact_not_added(self):
        gid = self._create()["gallery"]["gallery_id"]
        body = self._add(gid, "jobGONE", "nope")
        self.assertFalse(body["added"])
        self.assertEqual(len(body["gallery"]["pieces"]), 0)

    def test_add_transient_error_fails(self):
        gid = self._create()["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA")
        self.s3.fail["renders/jobA/color/cA/image.jpeg"] = _ce("SlowDown", 503)
        body = self._add(gid, "jobA", "cA")
        self.assertIn("error", body)

    def test_add_export_identity_mismatch_not_added(self):
        gid = self._create()["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA")
        self._seed_export("jobA", "cA", "dz_x", source_artifact_id="OTHER")
        body = self._add(gid, "jobA", "cA", "dz_x")
        self.assertFalse(body["added"])
        self.assertEqual(body["reason"], "export_identity_mismatch")

    def test_add_to_missing_gallery_is_404(self):
        self._seed_color("jobA", "cA")
        resp, _ = self._route(hs.handle_add_to_gallery, gallery_id="gallery_nope", job_id="jobA", artifact_id="cA")
        self.assertEqual(resp["statusCode"], 404)

    def test_add_rejects_bad_ids(self):
        gid = self._create()["gallery"]["gallery_id"]
        resp, _ = self._route(hs.handle_add_to_gallery, gallery_id=gid, job_id="../evil", artifact_id="cA")
        self.assertEqual(resp["statusCode"], 400)

    # ── save (reorder / retitle / remove) + CAS ──────────────────────────
    def _seed_two_piece_gallery(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA")
        self._seed_color("jobB", "cB")
        self._add(gid, "jobA", "cA")
        fetched = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])
        self._add(gid, "jobB", "cB")
        return gid

    def test_save_reorders_retitles_removes(self):
        gid = self._seed_two_piece_gallery()
        fetched = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])
        doc, revision = fetched["gallery"], fetched["revision"]
        # reverse order, retitle the (new) first, drop nothing
        pieces = list(reversed(doc["pieces"]))
        pieces[0]["title"] = "Opening piece"
        doc["pieces"] = pieces
        doc["name"] = "Renamed"
        _, saved = self._route(hs.handle_save_gallery, gallery=doc, expected_revision=revision)
        g = saved["gallery"]
        self.assertEqual(g["name"], "Renamed")
        self.assertEqual([p["ordinal"] for p in g["pieces"]], [0, 1])
        self.assertEqual(g["pieces"][0]["job_id"], "jobB")     # reversed
        self.assertEqual(g["pieces"][0]["title"], "Opening piece")

    def test_save_removes_pieces(self):
        gid = self._seed_two_piece_gallery()
        fetched = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])
        doc = fetched["gallery"]
        doc["pieces"] = [doc["pieces"][0]]     # keep only the first
        _, saved = self._route(hs.handle_save_gallery, gallery=doc)
        self.assertEqual(len(saved["gallery"]["pieces"]), 1)

    def test_save_stale_revision_is_409(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        stale = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])
        # a concurrent save bumps the etag
        self._route(hs.handle_save_gallery, gallery={"gallery_id": gid, "name": "other", "pieces": []})
        # saving with the stale revision must conflict
        resp, body = self._route(hs.handle_save_gallery,
                                 gallery={"gallery_id": gid, "name": "mine", "pieces": []},
                                 expected_revision=stale["revision"])
        self.assertEqual(resp["statusCode"], 409)
        self.assertEqual(body["conflict"], "gallery_revision")

    def test_save_rejects_cross_identity_key(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        bad = {"gallery_id": gid, "name": "G", "pieces": [{
            "job_id": "jobA", "artifact_id": "cA",
            "preview_key": "renders/OTHERJOB/color/cA/preview.jpg",   # wrong job
            "image_key": "renders/jobA/color/cA/image.jpeg",
            "preview_width": 512, "preview_height": 512}]}
        resp, _ = self._route(hs.handle_save_gallery, gallery=bad)
        self.assertEqual(resp["statusCode"], 400)

    def test_save_rejects_forged_deepzoom(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        bad = {"gallery_id": gid, "name": "G", "pieces": [{
            "job_id": "jobA", "artifact_id": "cA",
            "preview_key": "renders/jobA/color/cA/preview.jpg",
            "image_key": "renders/jobA/color/cA/image.jpeg",
            "preview_width": 512, "preview_height": 512,
            "deepzoom": {"export_id": "dz_1", "dzi_key": "deepzoom/jobA/dz_1/EVIL.dzi",
                         "source_key": "renders/jobA/color/cA/image.jpeg", "source_artifact_id": "cA"}}]}
        resp, _ = self._route(hs.handle_save_gallery, gallery=bad)
        self.assertEqual(resp["statusCode"], 400)

    # ── delete ────────────────────────────────────────────────────────────
    def test_delete_gallery(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        _, body = self._route(hs.handle_delete_gallery, gallery_id=gid)
        self.assertTrue(body["deleted"])
        resp, _ = self._route(hs.handle_fetch_gallery, gallery_id=gid)
        self.assertEqual(resp["statusCode"], 404)

    # ── create-gallery-share (snapshot -> immutable manifest) ────────────
    def test_open_gallery_snapshots_with_titles_and_order(self):
        gid = self._seed_two_piece_gallery()
        fetched = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])
        doc = fetched["gallery"]
        doc["pieces"][0]["title"] = "First"
        doc["pieces"][1]["title"] = "Second"
        self._route(hs.handle_save_gallery, gallery=doc, expected_revision=fetched["revision"])

        _, share = self._route(hs.handle_create_gallery_share, gallery_id=gid)
        self.assertEqual(share["count"], 2)
        self.assertTrue(share["manifest_url"].endswith("/manifest.json"))
        put = next(p for p in self.s3.puts if p["Key"].startswith("renders/_shared_mosaic/gallery/")
                   and p["Key"].endswith("manifest.json"))
        self.assertEqual(put["CacheControl"], "public, max-age=31536000, immutable")
        m = json.loads(put["Body"])
        self.assertEqual(m["manifest_type"], "virtual_gallery")
        self.assertEqual(m["document_kind"], "share")
        self.assertEqual([p["ordinal"] for p in m["pieces"]], [0, 1])
        self.assertEqual(m["pieces"][0]["title"], "First")
        self.assertEqual(m["pieces"][1]["title"], "Second")

    def test_open_empty_gallery_errors(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        _, body = self._route(hs.handle_create_gallery_share, gallery_id=gid)
        self.assertIn("error", body)
        self.assertFalse(any(p["Key"].startswith("renders/_shared_mosaic/gallery/") for p in self.s3.puts))


if __name__ == "__main__":
    unittest.main()
