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

    def get_object(self, Bucket=None, Key=None, Range=None):
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
                     source_family="color", source_artifact_id=None, export_job=None):
        export_job = export_job or job
        image_key = image_key or f"renders/{job}/color/{art}/image.jpeg"
        canonical_dzi = f"deepzoom/{export_job}/{export_id}/image.dzi"
        self.s3.objects[f"deepzoom/{export_job}/{export_id}/meta.json"] = {"body": json.dumps({
            "job_id": export_job, "export_id": export_id, "source_family": source_family,
            "source_artifact_id": source_artifact_id or art, "source_key": image_key,
            "dzi_key": dzi_key or canonical_dzi}).encode()}
        # a REAL parseable descriptor + single-tile preview level, so tests that
        # expect rejection prove fail-CLOSED against a fully usable pyramid
        self.s3.objects[canonical_dzi] = {"Metadata": {}, "ContentLength": 200, "ContentType": "application/xml",
            "body": ('<?xml version="1.0"?><Image Format="jpeg" Overlap="1" TileSize="254">'
                     '<Size Width="512" Height="512"/></Image>').encode()}
        self.s3.objects[f"deepzoom/{export_job}/{export_id}/image_files/7/0_0.jpeg"] = {
            "Metadata": {}, "ContentLength": 900, "ContentType": "image/jpeg"}

    def _event(self, body):
        return {"body": json.dumps(body)}

    def _route(self, fn, **body):
        resp = hs._handle_storage_route(fn, self._event(body))
        return resp, json.loads(resp["body"])

    def _create(self, name="My gallery"):
        return json.loads(hs.handle_create_gallery(self._event({"name": name}))["body"])

    def _add(self, gid, job, art, export=None, export_job=None):
        body = {"gallery_id": gid, "job_id": job, "artifact_id": art}
        if export:
            body["export_id"] = export
        if export_job:
            body["export_job_id"] = export_job
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
        _, saved = self._route(hs.handle_save_gallery, gallery=doc, expected_revision=fetched["revision"])
        self.assertEqual(len(saved["gallery"]["pieces"]), 1)

    def test_save_requires_revision(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        resp, _ = self._route(hs.handle_save_gallery, gallery={"gallery_id": gid, "name": "x", "pieces": []})
        self.assertEqual(resp["statusCode"], 400)   # expected_revision is required

    def test_save_never_creates_a_missing_gallery(self):
        resp, _ = self._route(hs.handle_save_gallery,
                              gallery={"gallery_id": "gallery_nope", "name": "x", "pieces": []},
                              expected_revision="whatever")
        self.assertEqual(resp["statusCode"], 404)   # save never creates a caller-chosen id

    def test_save_stale_revision_is_409(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        stale = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])["revision"]
        # a concurrent save (valid revision) bumps the etag
        self._route(hs.handle_save_gallery, gallery={"gallery_id": gid, "name": "other", "pieces": []}, expected_revision=stale)
        # saving again with the now-stale revision must conflict
        resp, body = self._route(hs.handle_save_gallery,
                                 gallery={"gallery_id": gid, "name": "mine", "pieces": []},
                                 expected_revision=stale)
        self.assertEqual(resp["statusCode"], 409)
        self.assertEqual(body["conflict"], "gallery_revision")

    def test_save_rejects_unknown_piece(self):
        # save can reorder/retitle/remove but NOT add — an identity not already in
        # the stored gallery is rejected (adds go through /add-to-gallery).
        gid = self._create("G")["gallery"]["gallery_id"]
        rev = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])["revision"]
        bad = {"gallery_id": gid, "name": "G", "pieces": [{
            "job_id": "jobA", "artifact_id": "cA",
            "preview_key": "renders/jobA/color/cA/preview.jpg",
            "image_key": "renders/jobA/color/cA/image.jpeg", "preview_width": 512, "preview_height": 512}]}
        resp, _ = self._route(hs.handle_save_gallery, gallery=bad, expected_revision=rev)
        self.assertEqual(resp["statusCode"], 400)

    def test_save_preserves_server_fields_takes_only_title_and_order(self):
        # Add a server-enriched piece, then save with a TAMPERED image_key, dims,
        # and function. The stored fields must be preserved; only title + order
        # are taken from the client (review finding 7).
        gid = self._create("G")["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA", function="poly_real", degree=12)
        self._add(gid, "jobA", "cA")
        fetched = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])
        doc, rev = fetched["gallery"], fetched["revision"]
        doc["pieces"][0]["title"] = "Curator title"
        doc["pieces"][0]["image_key"] = "renders/EVIL/color/cA/image.jpeg"   # tamper
        doc["pieces"][0]["preview_width"] = -1                                # tamper
        doc["pieces"][0]["function"] = "HACKED"                               # tamper
        _, saved = self._route(hs.handle_save_gallery, gallery=doc, expected_revision=rev)
        p = saved["gallery"]["pieces"][0]
        self.assertEqual(p["title"], "Curator title")                         # client title kept
        self.assertEqual(p["image_key"], "renders/jobA/color/cA/image.jpeg")  # server value preserved
        self.assertEqual(p["preview_width"], 512)                             # server value preserved
        self.assertEqual(p["function"], "poly_real")                          # server value preserved

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
        _, saved = self._route(hs.handle_save_gallery, gallery=doc, expected_revision=fetched["revision"])

        _, share = self._route(hs.handle_create_gallery_share, gallery_id=gid, expected_revision=saved["revision"])
        self.assertEqual(share["count"], 2)
        self.assertTrue(share["manifest_url"].endswith("/manifest.json"))
        self.assertLessEqual(len(share["share_id"]), 12)   # short-link promise
        put = next(p for p in self.s3.puts if p["Key"].startswith("renders/_shared_mosaic/gallery/")
                   and p["Key"].endswith("manifest.json"))
        self.assertEqual(put["CacheControl"], "public, max-age=31536000, immutable")
        m = json.loads(put["Body"])
        self.assertEqual(m["manifest_type"], "virtual_gallery")
        self.assertEqual(m["document_kind"], "share")
        self.assertEqual([p["ordinal"] for p in m["pieces"]], [0, 1])
        self.assertEqual(m["pieces"][0]["title"], "First")
        self.assertEqual(m["pieces"][1]["title"], "Second")

    def test_open_with_stale_revision_is_409(self):
        # Open pins the share to the reviewed revision: a concurrent change must
        # block the snapshot rather than silently share a different set (finding 6).
        gid = self._seed_two_piece_gallery()
        stale = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])["revision"]
        self._route(hs.handle_save_gallery, gallery={"gallery_id": gid, "name": "moved", "pieces": []},
                    expected_revision=stale)   # moves the gallery
        resp, body = self._route(hs.handle_create_gallery_share, gallery_id=gid, expected_revision=stale)
        self.assertEqual(resp["statusCode"], 409)
        self.assertEqual(body["conflict"], "gallery_revision")

    def test_open_empty_gallery_errors(self):
        created = self._create("G")
        gid = created["gallery"]["gallery_id"]
        _, body = self._route(hs.handle_create_gallery_share, gallery_id=gid, expected_revision=created["revision"])
        self.assertIn("error", body)
        self.assertFalse(any(p["Key"].startswith("renders/_shared_mosaic/gallery/") for p in self.s3.puts))

    def test_open_requires_revision(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        resp, _ = self._route(hs.handle_create_gallery_share, gallery_id=gid)
        self.assertEqual(resp["statusCode"], 400)   # revision required, like save (finding 6)

    # ── scene settings (sky + wall colour) ───────────────────────────────
    def test_create_has_default_settings(self):
        g = self._create()["gallery"]
        self.assertEqual(g["settings"], {"sky": "stars", "wall_color": "#ece4d6", "wall_coverage": 35, "wall_self_tint": True, "wall_edge_px": 1, "wall_layout": "maze"})

    def test_save_persists_settings_and_open_snapshots_them(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA")
        self._add(gid, "jobA", "cA")
        fetched = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])
        doc, rev = fetched["gallery"], fetched["revision"]
        doc["settings"] = {"sky": "dark", "wall_color": "#AABBCC", "wall_coverage": 250, "wall_self_tint": False, "wall_edge_px": 40, "wall_layout": "serpentine"}
        _, saved = self._route(hs.handle_save_gallery, gallery=doc, expected_revision=rev)
        self.assertEqual(saved["gallery"]["settings"], {"sky": "dark", "wall_color": "#aabbcc", "wall_coverage": 100, "wall_self_tint": False, "wall_edge_px": 12, "wall_layout": "serpentine"})
        _, _share = self._route(hs.handle_create_gallery_share, gallery_id=gid, expected_revision=saved["revision"])
        put = next(p for p in self.s3.puts if p["Key"].startswith("renders/_shared_mosaic/gallery/")
                   and p["Key"].endswith("manifest.json"))
        self.assertEqual(json.loads(put["Body"])["settings"], {"sky": "dark", "wall_color": "#aabbcc", "wall_coverage": 100, "wall_self_tint": False, "wall_edge_px": 12, "wall_layout": "serpentine"})

    def test_save_defaults_invalid_settings(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA")
        self._add(gid, "jobA", "cA")
        fetched = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])
        doc = fetched["gallery"]
        doc["settings"] = {"sky": "rainbow", "wall_color": "not-a-hex", "wall_coverage": "junk", "wall_layout": "zigzag"}
        _, saved = self._route(hs.handle_save_gallery, gallery=doc, expected_revision=fetched["revision"])
        self.assertEqual(saved["gallery"]["settings"], {"sky": "stars", "wall_color": "#ece4d6", "wall_coverage": 35, "wall_self_tint": True, "wall_edge_px": 1, "wall_layout": "maze"})


    def test_add_accepts_export_from_other_image_variant(self):
        # The artifact has image.jpeg (resolved first) but the export was built
        # from image.png of the SAME artifact — same picture, must be accepted
        # (the exact-key equality check wrongly rejected this as a mismatch).
        gid = self._create()["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA")
        self._seed_export("jobA", "cA", "dz_v", image_key="renders/jobA/color/cA/image.png")
        body = self._add(gid, "jobA", "cA", "dz_v")
        self.assertTrue(body["added"])
        self.assertEqual(body["gallery"]["pieces"][0]["deepzoom"]["dzi_key"], "deepzoom/jobA/dz_v/image.dzi")

    def test_add_accepts_legacy_export_meta_without_identity_fields(self):
        # Older exports lack source_family/source_artifact_id (added later); the
        # source_key parse is the identity. Absent fields must not be mismatches.
        gid = self._create()["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA")
        self.s3.objects["deepzoom/jobA/dz_old/meta.json"] = {"body": json.dumps({
            "source_key": "renders/jobA/color/cA/image.jpeg",
            "dzi_key": "deepzoom/jobA/dz_old/image.dzi"}).encode()}
        self.s3.objects["deepzoom/jobA/dz_old/image.dzi"] = {"Metadata": {}, "ContentLength": 200, "ContentType": "application/xml"}
        body = self._add(gid, "jobA", "cA", "dz_old")
        self.assertTrue(body["added"], body)
        self.assertEqual(body["gallery"]["pieces"][0]["deepzoom"]["dzi_key"], "deepzoom/jobA/dz_old/image.dzi")

    def test_add_builds_piece_from_dzi_when_no_color_artifact(self):
        # THE RULE: has a DZI => curatable. No color artifact exists at all
        # (legacy/deleted render): the piece is built from the export itself —
        # preview = the largest single-tile pyramid level, zoom = the DZI, and
        # no original link (source object gone).
        gid = self._create()["gallery"]["gallery_id"]
        self.s3.objects["deepzoom/jobL/dz_L/meta.json"] = {"body": json.dumps({
            "source_key": "renders/jobL/color/cL/image.jpeg",   # deleted since
            "created_at": "2026-06-01T00:00:00Z"}).encode()}
        self.s3.objects["deepzoom/jobL/dz_L/image.dzi"] = {"body": (
            '<?xml version="1.0"?><Image Format="jpeg" Overlap="1" TileSize="254">'
            '<Size Width="4096" Height="4096"/></Image>').encode()}
        self.s3.objects["deepzoom/jobL/dz_L/image_files/7/0_0.jpeg"] = {
            "Metadata": {}, "ContentLength": 900, "ContentType": "image/jpeg"}
        body = self._add(gid, "jobL", "cL", "dz_L")
        self.assertTrue(body["added"], body)
        p = body["gallery"]["pieces"][0]
        self.assertEqual(p["preview_key"], "deepzoom/jobL/dz_L/image_files/7/0_0.jpeg")
        self.assertEqual(p["preview_width"], 128)
        self.assertIsNone(p["image_key"])                       # original not linkable
        self.assertEqual(p["deepzoom"]["dzi_key"], "deepzoom/jobL/dz_L/image.dzi")
        # true provenance is preserved even when the original is gone
        self.assertEqual(p["deepzoom"]["source_key"], "renders/jobL/color/cL/image.jpeg")

    def test_add_dzi_fallback_links_original_when_it_exists(self):
        gid = self._create()["gallery"]["gallery_id"]
        self.s3.objects["renders/jobM/bilevel/bM/image.tif"] = {"Metadata": {}, "ContentLength": 10, "ContentType": "image/tiff"}
        self.s3.objects["deepzoom/jobM/dz_M/meta.json"] = {"body": json.dumps({
            "source_key": "renders/jobM/bilevel/bM/image.tif"}).encode()}
        self.s3.objects["deepzoom/jobM/dz_M/image.dzi"] = {"body": (
            '<?xml version="1.0"?><Image Format="png" Overlap="1" TileSize="254">'
            '<Size Width="512" Height="512"/></Image>').encode()}
        self.s3.objects["deepzoom/jobM/dz_M/image_files/7/0_0.png"] = {
            "Metadata": {}, "ContentLength": 900, "ContentType": "image/png"}
        body = self._add(gid, "jobM", "bM", "dz_M")
        self.assertTrue(body["added"], body)
        p = body["gallery"]["pieces"][0]
        self.assertEqual(p["image_key"], "renders/jobM/bilevel/bM/image.tif")
        self.assertEqual(p["deepzoom"]["source_key"], "renders/jobM/bilevel/bM/image.tif")

    def test_add_cross_job_export(self):
        # FINDING 1 REGRESSION: the export OWNER (compute_x) differs from the
        # render-source job (rjobX). Both identities travel; the DZI is looked
        # up under the owner and the piece enriches from the render job.
        gid = self._create()["gallery"]["gallery_id"]
        self._seed_color("rjobX", "cX")
        self._seed_export("rjobX", "cX", "dz_X", export_job="compute_x")
        body = self._add(gid, "rjobX", "cX", "dz_X", export_job="compute_x")
        self.assertTrue(body["added"], body)
        p = body["gallery"]["pieces"][0]
        self.assertEqual(p["job_id"], "rjobX")
        self.assertEqual(p["export_job_id"], "compute_x")
        self.assertEqual(p["deepzoom"]["dzi_key"], "deepzoom/compute_x/dz_X/image.dzi")

    def test_dzi_fallback_rejects_contradicting_meta(self):
        # FINDING 2 REGRESSION: no color artifact, fully USABLE pyramid, but the
        # meta names a DIFFERENT artifact — the fallback must stay fail-closed.
        gid = self._create()["gallery"]["gallery_id"]
        self._seed_export("jobZ", "cZ", "dz_Z", source_artifact_id="OTHER")
        body = self._add(gid, "jobZ", "cZ", "dz_Z")
        self.assertFalse(body["added"])
        self.assertEqual(body["reason"], "export_identity_mismatch")

    def test_add_legacy_root_source_key_not_linked_but_curated(self):
        # FINDING 5: a legacy root-shaped source key exists but is not a shape the
        # viewer accepts — curate the piece, keep the raw key as provenance, and
        # do NOT link it as the original.
        gid = self._create()["gallery"]["gallery_id"]
        self.s3.objects["renders/jobR/image_bilevel.tif"] = {"Metadata": {}, "ContentLength": 10, "ContentType": "image/tiff"}
        self.s3.objects["deepzoom/jobR/dz_R/meta.json"] = {"body": json.dumps({
            "source_key": "renders/jobR/image_bilevel.tif"}).encode()}
        self.s3.objects["deepzoom/jobR/dz_R/image.dzi"] = {"body": (
            '<?xml version="1.0"?><Image Format="png" Overlap="1" TileSize="254">'
            '<Size Width="512" Height="512"/></Image>').encode()}
        self.s3.objects["deepzoom/jobR/dz_R/image_files/7/0_0.png"] = {
            "Metadata": {}, "ContentLength": 900, "ContentType": "image/png"}
        body = self._add(gid, "jobR", "aR", "dz_R")
        self.assertTrue(body["added"], body)
        p = body["gallery"]["pieces"][0]
        self.assertIsNone(p["image_key"])                                          # viewer-safe
        self.assertEqual(p["deepzoom"]["source_key"], "renders/jobR/image_bilevel.tif")  # provenance

    def test_malformed_dzi_descriptor_rejected_not_500(self):
        # FINDING 7: TileSize="0" (division bomb) and unsupported formats must be
        # classified rejections, never a 500 or a piece the viewer later drops.
        gid = self._create()["gallery"]["gallery_id"]
        for export_id, xml in (
            ("dz_zero", '<Image Format="jpeg" Overlap="1" TileSize="0"><Size Width="512" Height="512"/></Image>'),
            ("dz_tiff", '<Image Format="tiff" Overlap="1" TileSize="254"><Size Width="512" Height="512"/></Image>'),
        ):
            self.s3.objects[f"deepzoom/jobQ/{export_id}/meta.json"] = {"body": b"{}"}
            self.s3.objects[f"deepzoom/jobQ/{export_id}/image.dzi"] = {"body": xml.encode()}
            body = self._add(gid, "jobQ", "cQ", export_id)
            self.assertFalse(body["added"], body)
            self.assertEqual(body["reason"], "export_dzi_invalid")

    def test_add_rejects_unknown_preview_dimensions(self):
        # No meta.json dims and an unparseable preview: the viewer could not lay
        # this piece out, so the add must fail loudly instead of succeeding and
        # being silently dropped by the viewer.
        gid = self._create()["gallery"]["gallery_id"]
        prefix = "renders/jobU/color/cU/"
        self.s3.objects[prefix + "image.jpeg"] = {"Metadata": {}, "ContentLength": 1000, "ContentType": "image/jpeg"}
        self.s3.objects[prefix + "preview.png"] = {"Metadata": {}, "ContentLength": 50, "ContentType": "image/png"}
        self.s3.objects["renders/jobU/calc.json"] = {"body": b"{}"}
        body = self._add(gid, "jobU", "cU")
        self.assertFalse(body["added"])
        self.assertEqual(body["reason"], "unknown_preview_dimensions")

    def test_save_stale_revision_with_changed_pieces_is_409_not_400(self):
        # A stale revision whose piece set changed must be a CONFLICT (409, the
        # client reloads), not an "unknown piece" validation error (400).
        gid = self._create("G")["gallery"]["gallery_id"]
        self._seed_color("jobA", "cA")
        self._add(gid, "jobA", "cA")
        fetched = json.loads(hs.handle_fetch_gallery(self._event({"gallery_id": gid}))["body"])
        doc, rev = fetched["gallery"], fetched["revision"]
        # concurrent save removes the piece (bumps revision)
        self._route(hs.handle_save_gallery, gallery={"gallery_id": gid, "name": "G", "pieces": []}, expected_revision=rev)
        # stale save still referencing the removed piece -> 409, not 400
        resp, body = self._route(hs.handle_save_gallery, gallery=doc, expected_revision=rev)
        self.assertEqual(resp["statusCode"], 409)
        self.assertEqual(body["conflict"], "gallery_revision")


    # ── describe-gallery (vision titles, book engine reuse) ──────────────
    def _seed_dzi_piece_gallery(self):
        gid = self._create("G")["gallery"]["gallery_id"]
        self.s3.objects["deepzoom/jobT/dz_T/meta.json"] = {"body": json.dumps({
            "source_key": "renders/jobT/color/cT/image.jpeg"}).encode()}
        self.s3.objects["deepzoom/jobT/dz_T/image.dzi"] = {"body": (
            '<?xml version="1.0"?><Image Format="png" Overlap="1" TileSize="254">'
            '<Size Width="512" Height="512"/></Image>').encode()}
        self.s3.objects["deepzoom/jobT/dz_T/image_files/7/0_0.png"] = {
            "Metadata": {}, "ContentLength": 900, "ContentType": "image/png",
            "body": b"fakepng"}
        self._add(gid, "jobT", "cT", "dz_T")
        return gid

    def test_settings_accept_spiral_layout(self):
        s = hs._clean_gallery_settings({"wall_layout": "spiral"})
        self.assertEqual(s["wall_layout"], "spiral")

    def test_describe_gallery_titles_selection(self):
        import book_describe
        gid = self._seed_dzi_piece_gallery()
        with patch.object(book_describe, "_vision_call", return_value='noise {"title": "Night Lattice"} tail') as vc, \
             patch.object(book_describe, "_load_vision_config",
                          return_value={"model": "gemini-2.5-flash", "api_key_gemini": "k"}), \
             patch.object(book_describe, "_downscale_for_vision", side_effect=lambda b: b):
            resp, body = self._route(hs.handle_describe_gallery, gallery_id=gid,
                                     pieces=[{"job_id": "jobT", "artifact_id": "cT"}], overwrite=True)
        self.assertEqual(body["described"], 1, body)
        self.assertEqual(body["errors"], [])
        self.assertEqual(body["gallery"]["pieces"][0]["title"], "Night Lattice")
        # per-piece persistence: the doc was CAS-saved with the title
        gput = [p for p in self.s3.puts if p["Key"].endswith(gid + ".json")][-1]
        self.assertIsNotNone(gput["IfMatch"])
        self.assertIn("Night Lattice", str(gput["Body"]))
        # the thumbnail sent to the model is the piece's pyramid preview
        self.assertEqual(vc.call_args[0][2], b"fakepng")

    def test_describe_gallery_isolates_failures(self):
        import book_describe
        gid = self._seed_dzi_piece_gallery()
        with patch.object(book_describe, "_vision_call", side_effect=RuntimeError("model exploded")), \
             patch.object(book_describe, "_load_vision_config",
                          return_value={"model": "gemini-2.5-flash", "api_key_gemini": "k"}), \
             patch.object(book_describe, "_downscale_for_vision", side_effect=lambda b: b):
            _, body = self._route(hs.handle_describe_gallery, gallery_id=gid, overwrite=True)
        self.assertEqual(body["described"], 0)
        self.assertEqual(len(body["errors"]), 1)
        self.assertIn("model exploded", body["errors"][0]["error"])
        self.assertEqual(body["gallery"]["pieces"][0]["title"], "")   # unchanged

    def test_describe_gallery_without_key_is_clean_error(self):
        import book_describe
        gid = self._seed_dzi_piece_gallery()
        with patch.object(book_describe, "_load_vision_config", return_value={}), \
             patch.dict(os.environ, {"GEMINI_API_KEY": ""}):
            _, body = self._route(hs.handle_describe_gallery, gallery_id=gid, overwrite=True)
        self.assertIn("error", body)
        self.assertIn("vision API key", body["error"])


if __name__ == "__main__":
    unittest.main()
