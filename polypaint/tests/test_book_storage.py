"""Book document CRUD (book-maker-design.md §4/§5): S3-JSON docs under
polypaint/books/ following the saved-program storage contracts."""
import io
import json
import os
import sys
import unittest
from unittest.mock import patch

from botocore.exceptions import ClientError

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _FakeS3:
    def __init__(self):
        self.objects = {}
        self.metadata = {}
        self.etags = {}
        self._seq = 0

    def get_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise ClientError({"Error": {"Code": "NoSuchKey"}}, "GetObject")
        return {"Body": io.BytesIO(self.objects[Key]),
                "ETag": self.etags.get(Key, '"e0"'),
                "Metadata": dict(self.metadata.get(Key) or {})}

    def put_object(self, Bucket=None, Key=None, Body=None, ContentType=None,
                   Metadata=None, IfMatch=None):
        # simulate S3 conditional PutObject: reject if the current ETag doesn't
        # match IfMatch (real atomic CAS)
        if IfMatch is not None:
            cur = self.etags.get(Key, "").strip('"')
            if cur != str(IfMatch).strip('"'):
                raise ClientError({"Error": {"Code": "PreconditionFailed"}}, "PutObject")
        data = Body if isinstance(Body, bytes) else str(Body or "").encode("utf-8")
        self.objects[Key] = data
        self.metadata[Key] = dict(Metadata or {})
        self._seq += 1
        etag = f'"e{self._seq}"'
        self.etags[Key] = etag
        return {"ETag": etag}

    def copy_object(self, Bucket=None, CopySource=None, Key=None, ContentType=None,
                    Metadata=None, MetadataDirective=None, **_kw):
        source_key = (CopySource or {}).get("Key")
        if source_key not in self.objects:
            raise ClientError({"Error": {"Code": "NoSuchKey"}}, "CopyObject")
        self.objects[Key] = self.objects[source_key]
        self.metadata[Key] = dict(Metadata or {})
        self._seq += 1
        self.etags[Key] = f'"e{self._seq}"'
        return {"CopyObjectResult": {"ETag": self.etags[Key]}}

    def head_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise Exception("NoSuchKey")
        return {"Metadata": dict(self.metadata.get(Key) or {})}

    def delete_object(self, Bucket=None, Key=None):
        self.objects.pop(Key, None)
        self.metadata.pop(Key, None)
        return {}

    def delete_objects(self, Bucket=None, Delete=None):
        for obj in (Delete or {}).get("Objects") or []:
            self.objects.pop(obj["Key"], None)
            self.metadata.pop(obj["Key"], None)
        return {}

    def list_objects_v2(self, Bucket=None, Prefix=None, **_kw):
        keys = sorted(k for k in self.objects if k.startswith(Prefix or ""))
        return {"Contents": [{"Key": k} for k in keys], "IsTruncated": False}


def _patch_s3(mock_s3, fake_s3):
    for name in ("get_object", "put_object", "copy_object", "head_object", "delete_object", "delete_objects", "list_objects_v2"):
        getattr(mock_s3, name).side_effect = getattr(fake_s3, name)


def _event(path, body):
    return {"path": path, "body": json.dumps(body)}


def _entry(i, **over):
    entry = {
        "job_id": f"job{i}",
        "artifact_id": f"art{i}",
        "image_key": f"renders/job{i}/color/art{i}/image.jpeg",
    }
    entry.update(over)
    return entry


class TestBookStorage(unittest.TestCase):
    @patch("handler_storage.s3")
    def test_save_fetch_roundtrip_and_overwrite(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        _patch_s3(mock_s3, fake)
        resp = handler_storage.handler(_event("/save-book", {"book": {
            "name": "My First Book",
            "title": "PolyPaint",
            "entries": [_entry(1), _entry(2, title_override="Study 2")],
        }}), None)
        self.assertEqual(resp["statusCode"], 200)
        body = json.loads(resp["body"])
        self.assertFalse(body["overwritten"])
        self.assertEqual(body["book"]["id"], "my-first-book")
        self.assertEqual(len(body["book"]["entries"]), 2)
        self.assertEqual(body["book"]["background_color"], "1a1a2e")
        self.assertTrue(all(e["entry_id"] for e in body["book"]["entries"]))
        self.assertTrue(body["book"]["saved_at"])

        again = handler_storage.handler(_event("/save-book", {"book": body["book"]}), None)
        self.assertTrue(json.loads(again["body"])["overwritten"])

        fetched = handler_storage.handler(_event("/fetch-book", {"id": "my-first-book"}), None)
        fbody = json.loads(fetched["body"])
        self.assertEqual(fbody["book"]["name"], "My First Book")
        self.assertIsNone(fbody["latest_output"])

        # latest_output pointer surfaces once compose writes it
        fake.put_object(Key="polypaint/books/my-first-book/out/latest.json",
                        Body=json.dumps({"compile_id": "c1", "content_pages": 6}))
        fetched = handler_storage.handler(_event("/fetch-book", {"id": "my-first-book"}), None)
        self.assertEqual(json.loads(fetched["body"])["latest_output"]["compile_id"], "c1")

    @patch("handler_storage.s3")
    def test_unicode_book_name_saves_with_ascii_s3_metadata(self, mock_s3):
        """Book names/titles are unicode; S3 user metadata is ASCII-only.
        The body keeps the exact text, the metadata gets the ASCII spelling
        (same class as the ExtractPalette em-dash crash, 2026-07-24)."""
        import handler_storage
        from shared import ascii_metadata_value

        fake = _FakeS3()
        _patch_s3(mock_s3, fake)
        name = "Chromatic Fields \u2014 B\u00f6cklin studies"
        resp = handler_storage.handler(_event("/save-book", {"book": {"name": name}}), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        saved = json.loads(resp["body"])["book"]
        self.assertEqual(saved["name"], name)   # body: exact unicode
        put_meta = fake.metadata.get(f"polypaint/books/{saved['id']}.json") or {}
        for value in put_meta.values():
            self.assertTrue(str(value).isascii(), repr(put_meta))
        self.assertEqual(ascii_metadata_value(name),
                         "Chromatic Fields - Bocklin studies")

    @patch("handler_storage.s3")
    def test_list_books_carries_title_and_subtitle(self, mock_s3):
        import handler_storage

        _patch_s3(mock_s3, _FakeS3())
        resp = handler_storage.handler(_event("/save-book", {"book": {
            "name": "field notes", "title": "Chromatic Fields",
            "subtitle": "roots in colour"}}), None)
        self.assertEqual(resp["statusCode"], 200)
        resp = handler_storage.handler(_event("/list-books", {}), None)
        self.assertEqual(resp["statusCode"], 200)
        rows = json.loads(resp["body"])["books"]
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["name"], "field notes")
        self.assertEqual(rows[0]["title"], "Chromatic Fields")
        self.assertEqual(rows[0]["subtitle"], "roots in colour")
        self.assertEqual(rows[0]["entry_count"], 0)

    @patch("handler_storage.s3")
    def test_spread_layout_roundtrips_and_defaults(self, mock_s3):
        import handler_storage

        _patch_s3(mock_s3, _FakeS3())
        # default: absent -> color_primary persisted explicitly
        resp = handler_storage.handler(_event("/save-book", {"book": {"name": "Lay"}}), None)
        self.assertEqual(resp["statusCode"], 200)
        saved = json.loads(resp["body"])["book"]
        self.assertEqual(saved["spread_layout"], "color_primary")
        # palette_primary round-trips through save -> fetch
        resp = handler_storage.handler(_event("/save-book", {
            "book": {"name": "Lay", "id": saved["id"], "spread_layout": "palette_primary"}}), None)
        self.assertEqual(resp["statusCode"], 200)
        resp = handler_storage.handler(_event("/fetch-book", {"id": saved["id"]}), None)
        fetched = json.loads(resp["body"])["book"]
        self.assertEqual(fetched["spread_layout"], "palette_primary")

    @patch("handler_storage.s3")
    def test_legacy_entry_cover_upgrades_to_cover_source(self, mock_s3):
        import handler_storage

        _patch_s3(mock_s3, _FakeS3())
        resp = handler_storage.handler(_event("/save-book", {"book": {
            "name": "Legacy Cover",
            "entries": [_entry(1, entry_id="front")],
            "cover_entry_id": "front",
        }}), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        book = json.loads(resp["body"])["book"]
        self.assertEqual(book["version"], 2)
        self.assertEqual(book["cover_entry_id"], "front")
        self.assertEqual(book["cover_source"], {
            "version": 1,
            "kind": "entry",
            "entry_id": "front",
        })

    @patch("handler_storage.s3")
    def test_book_background_color_is_canonical_and_persisted(self, mock_s3):
        import handler_storage

        _patch_s3(mock_s3, _FakeS3())
        resp = handler_storage.handler(_event("/save-book", {"book": {
            "name": "Warm Book",
            "background_color": "#AbC",
            "entries": [],
        }}), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        self.assertEqual(json.loads(resp["body"])["book"]["background_color"], "aabbcc")

    @patch("handler_storage.s3")
    def test_snapshot_current_allcol_wall_into_book_cover_source(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        _patch_s3(mock_s3, fake)
        book_id = "wall-book"
        refresh_id = "mosaic_20260722T120000Z_abcdef01"
        wall_prefix = f"renders/_index/color_mosaic/{refresh_id}/"
        fake.put_object(
            Key=f"polypaint/books/{book_id}.json",
            Body=json.dumps({
                "book_kind": "book", "id": book_id, "name": "Wall Book", "entries": [],
            }),
        )
        fake.objects[wall_prefix + "wall.jpg"] = b"full-resolution-wall"
        fake.objects[wall_prefix + "wall_files/8/0_0.jpg"] = b"small-dzi-overview"
        fake.objects[wall_prefix + "wall.json"] = json.dumps({
            "manifest_type": "artifact_wall_pyramid",
            "kind": "color",
            "refresh_id": refresh_id,
            "image_key": wall_prefix + "wall.jpg",
            "dzi_key": wall_prefix + "wall.dzi",
            "flat_jpeg": True,
            "width": 32768,
            "height": 16384,
        }).encode()
        status = {
            "state": "ready",
            "wall_state": "ready",
            "wall_refresh_id": refresh_id,
            "wall_json_key": wall_prefix + "wall.json",
        }
        with patch.object(handler_storage, "_read_mosaic_status", return_value=status):
            resp = handler_storage.handler(
                _event("/snapshot-book-cover", {"book_id": book_id}), None)

        self.assertEqual(resp["statusCode"], 200, resp["body"])
        body = json.loads(resp["body"])
        source = body["cover_source"]
        frozen_key = f"polypaint/books/{book_id}/cover/allcol-{refresh_id}.jpg"
        preview_key = f"polypaint/books/{book_id}/cover/allcol-{refresh_id}-preview.jpg"
        self.assertEqual(source["kind"], "allcol_wall")
        self.assertEqual(source["image_key"], frozen_key)
        self.assertEqual(source["preview_key"], preview_key)
        self.assertEqual(fake.objects[frozen_key], b"full-resolution-wall")
        self.assertEqual(fake.objects[preview_key], b"small-dzi-overview")

        # The returned descriptor is accepted by the book schema and remains
        # the authoritative source after a normal Save.
        saved = handler_storage.handler(_event("/save-book", {"book": {
            "id": book_id,
            "name": "Wall Book",
            "entries": [],
            "cover_source": source,
        }}), None)
        self.assertEqual(saved["statusCode"], 200, saved["body"])
        saved_book = json.loads(saved["body"])["book"]
        self.assertEqual(saved_book["cover_source"], source)
        self.assertEqual(saved_book["cover_entry_id"], "")

    @patch("handler_storage.s3")
    def test_snapshot_allpal_wall_and_entry_palette_cover_sources(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        _patch_s3(mock_s3, fake)
        book_id = "pal-book"
        refresh_id = "mosaic_20260722T130000Z_beefcaf1"
        wall_prefix = f"renders/_index/palette_mosaic/{refresh_id}/"
        fake.put_object(
            Key=f"polypaint/books/{book_id}.json",
            Body=json.dumps({
                "book_kind": "book", "id": book_id, "name": "Pal Book", "entries": [],
            }),
        )
        fake.objects[wall_prefix + "wall.jpg"] = b"pal-wall"
        fake.objects[wall_prefix + "wall_files/8/0_0.jpg"] = b"pal-overview"
        fake.objects[wall_prefix + "wall.json"] = json.dumps({
            "manifest_type": "artifact_wall_pyramid",
            "kind": "palette",
            "refresh_id": refresh_id,
            "image_key": wall_prefix + "wall.jpg",
            "dzi_key": wall_prefix + "wall.dzi",
            "flat_jpeg": True,
            "width": 16384,
            "height": 16384,
        }).encode()
        status = {
            "state": "ready",
            "wall_state": "ready",
            "wall_refresh_id": refresh_id,
            "wall_json_key": wall_prefix + "wall.json",
        }
        with patch.object(handler_storage, "_read_mosaic_status", return_value=status) as mock_status:
            resp = handler_storage.handler(
                _event("/snapshot-book-cover", {"book_id": book_id, "wall": "allpal"}), None)
        mock_status.assert_called_once_with("palette", consistent=True)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        source = json.loads(resp["body"])["cover_source"]
        frozen_key = f"polypaint/books/{book_id}/cover/allpal-{refresh_id}.jpg"
        self.assertEqual(source["kind"], "allpal_wall")
        self.assertEqual(source["image_key"], frozen_key)
        self.assertEqual(
            fake.objects[frozen_key], b"pal-wall",
            "the AllPal cover must be a frozen byte copy of the wall")
        # the frozen source round-trips through save-book validation
        resp = handler_storage.handler(_event("/save-book", {"book": {
            "name": "Pal Book", "id": book_id, "cover_source": source,
        }}), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        saved = json.loads(resp["body"])["book"]
        self.assertEqual(saved["cover_source"]["kind"], "allpal_wall")
        self.assertEqual(saved["cover_entry_id"], "")

        # entry_palette: valid entry round-trips with empty legacy cover id;
        # unknown entry rejects
        resp = handler_storage.handler(_event("/save-book", {"book": {
            "name": "Pal Book", "id": book_id, "entries": [_entry(1, entry_id="front1")],
            "cover_source": {"kind": "entry_palette", "entry_id": "front1"},
        }}), None)
        self.assertEqual(resp["statusCode"], 200, resp["body"])
        saved = json.loads(resp["body"])["book"]
        self.assertEqual(saved["cover_source"]["kind"], "entry_palette")
        self.assertEqual(saved["cover_entry_id"], "")
        resp = handler_storage.handler(_event("/save-book", {"book": {
            "name": "Pal Book", "id": book_id, "entries": [_entry(1, entry_id="front1")],
            "cover_source": {"kind": "entry_palette", "entry_id": "ghost"},
        }}), None)
        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("entry_palette", json.loads(resp["body"])["error"])

    @patch("handler_storage.s3")
    def test_snapshot_book_cover_rejects_wall_without_flat_jpeg(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        _patch_s3(mock_s3, fake)
        book_id = "wall-book"
        refresh_id = "mosaic_20260722T120000Z_abcdef01"
        wall_prefix = f"renders/_index/color_mosaic/{refresh_id}/"
        fake.put_object(
            Key=f"polypaint/books/{book_id}.json",
            Body=json.dumps({"book_kind": "book", "id": book_id, "name": "Wall Book"}),
        )
        fake.objects[wall_prefix + "wall.json"] = json.dumps({
            "manifest_type": "artifact_wall_pyramid",
            "kind": "color",
            "refresh_id": refresh_id,
            "image_key": "",
            "flat_jpeg": False,
            "width": 70000,
            "height": 70000,
        }).encode()
        status = {
            "state": "ready",
            "wall_state": "ready",
            "wall_refresh_id": refresh_id,
            "wall_json_key": wall_prefix + "wall.json",
        }
        with patch.object(handler_storage, "_read_mosaic_status", return_value=status):
            resp = handler_storage.handler(
                _event("/snapshot-book-cover", {"book_id": book_id}), None)
        self.assertEqual(resp["statusCode"], 400)
        self.assertIn("no flat JPEG", json.loads(resp["body"])["error"])

    @patch("handler_storage.s3")
    def test_validation_rejects_bad_payloads(self, mock_s3):
        import handler_storage

        _patch_s3(mock_s3, _FakeS3())
        cases = [
            ({"book": {"name": ""}}, "name is required"),
            ({"book": {"name": "x", "book_kind": "album"}}, "unknown book_kind"),
            ({"book": {"name": "x", "entries": [{"job_id": "j"}]}}, "missing artifact_id"),
            ({"book": {"name": "x", "entries": [_entry(1)], "cover_entry_id": "nope"}},
             "cover_entry_id"),
            ({"book": {"name": "x", "id": "a/b"}}, "slug"),
            ({"book": {"name": "x", "entries": [_entry(i) for i in range(201)]}}, "max is 200"),
            # XSS / TeX-injection vectors: entry_id and free-text fields
            ({"book": {"name": "x", "entries": [_entry(1, entry_id='a}b')]}}, "entry_id must match"),
            ({"book": {"name": "x", "entries": [_entry(1, entry_id='x%y')]}}, "entry_id must match"),
            ({"book": {"name": "x", "entries": [_entry(1, entry_id='q" onx="')]}}, "entry_id must match"),
            ({"book": {"name": 'a</option><img src=x onerror=alert(1)>'}}, None),  # printable, allowed but escaped in UI
            ({"book": {"name": "x", "title": "line\nbreak"}}, "title must be printable"),
            ({"book": {"name": "x", "author": "bad\ttab"}}, "author must be printable"),
            ({"book": {"name": "x", "background_color": "midnight"}},
             "background_color must be 6-digit hex"),
            ({"book": {"name": "x", "spread_layout": "sideways"}},
             "spread_layout must be color_primary or palette_primary"),
            ({"book": {"name": "x", "cover_source": {
                "kind": "allcol_wall",
                "refresh_id": "mosaic_20260722T120000Z_abcdef01",
                "image_key": "config/secret.jpg",
                "width": 1000,
                "height": 1000,
            }}}, "frozen book source"),
        ]
        for payload, needle in cases:
            resp = handler_storage.handler(_event("/save-book", payload), None)
            if needle is None:
                self.assertEqual(resp["statusCode"], 200)  # printable name is stored; UI escapes
                continue
            self.assertEqual(resp["statusCode"], 400, needle)
            self.assertIn(needle, json.loads(resp["body"])["error"])

    @patch("handler_storage.s3")
    def test_image_key_must_be_a_render_image_key(self, mock_s3):
        # code-review-25 F3: image_key becomes a raw LaTeX arg (\qrcode{URL})
        # and an S3 GET — hostile values must be rejected at /save-book
        import handler_storage
        _patch_s3(mock_s3, _FakeS3())
        hostile = [
            "renders/j/color/a/image.jpeg} \\input{/etc/passwd",   # TeX injection
            "renders/j/color/a/image.jpeg\\qrcode",                 # backslash
            "renders/../../secret.json",                            # path escape (no ext)
            "s3://polypaint/renders/j/color/a/image.jpeg",          # scheme prefix
            "config/vision_model.json",                             # arbitrary key
            "renders/j/color/a/image.svg",                          # non-image ext
        ]
        for key in hostile:
            resp = handler_storage.handler(_event("/save-book", {"book": {
                "name": "x", "entries": [_entry(1, image_key=key)]}}), None)
            self.assertEqual(resp["statusCode"], 400, key)
            self.assertIn("image_key", json.loads(resp["body"])["error"])
        # real color + palette + preview keys still pass (consistent triples)
        good = [
            {"job_id": "job1", "artifact_id": "art1",
             "image_key": "renders/job1/color/art1/image.jpeg"},
            {"job_id": "j", "artifact_id": "pal_x",
             "image_key": "renders/j/palettes/pal_x/image.jpeg"},
            {"job_id": "j", "artifact_id": "a",
             "image_key": "renders/j/color/a/preview.png"}]
        for entry in good:
            resp = handler_storage.handler(_event("/save-book", {"book": {
                "name": "ok", "entries": [entry]}}), None)
            self.assertEqual(resp["statusCode"], 200, entry["image_key"])

    @patch("handler_storage.s3")
    def test_image_key_must_match_job_and_artifact(self, mock_s3):
        # code-review-26 F3: the image can't come from a different artifact
        # than job_id/artifact_id name (else the page image and its metadata
        # describe different artifacts)
        import handler_storage
        _patch_s3(mock_s3, _FakeS3())
        mism = handler_storage.handler(_event("/save-book", {"book": {
            "name": "x", "entries": [{
                "job_id": "jobA", "artifact_id": "artA",
                "image_key": "renders/jobB/color/artB/image.jpeg"}]}}), None)
        self.assertEqual(mism["statusCode"], 400)
        self.assertIn("image_key", json.loads(mism["body"])["error"])
        # right job, wrong artifact
        mism2 = handler_storage.handler(_event("/save-book", {"book": {
            "name": "x", "entries": [{
                "job_id": "jobA", "artifact_id": "artA",
                "image_key": "renders/jobA/color/artB/image.jpeg"}]}}), None)
        self.assertEqual(mism2["statusCode"], 400)
        # consistent triple passes
        ok = handler_storage.handler(_event("/save-book", {"book": {
            "name": "ok", "entries": [{
                "job_id": "jobA", "artifact_id": "artA",
                "image_key": "renders/jobA/color/artA/image.jpeg"}]}}), None)
        self.assertEqual(ok["statusCode"], 200)

    @patch("handler_storage.s3")
    def test_save_book_atomic_revision_cas(self, mock_s3):
        # CR28 F5: /save-book uses an ATOMIC S3 conditional write (IfMatch on
        # the opaque revision/ETag), not a timestamp read-then-write.
        import handler_storage
        _patch_s3(mock_s3, _FakeS3())
        first = json.loads(handler_storage.handler(_event("/save-book", {"book": {
            "name": "CAS Book", "entries": [_entry(1)]}}), None)["body"])
        book = first["book"]
        rev1 = first["revision"]
        self.assertTrue(rev1)
        # fetch returns the current revision
        fetched = json.loads(handler_storage.handler(
            _event("/fetch-book", {"id": "cas-book"}), None)["body"])
        self.assertEqual(fetched["revision"], rev1)
        # stale revision -> 409 conflict, S3 rejects the write atomically
        stale = handler_storage.handler(_event("/save-book", {
            "book": book, "expected_revision": '"nope"'}), None)
        self.assertEqual(stale["statusCode"], 409)
        self.assertEqual(json.loads(stale["body"])["conflict"], "book_saved_at")
        # correct revision -> 200, and it advances the revision
        ok = json.loads(handler_storage.handler(_event("/save-book", {
            "book": book, "expected_revision": rev1}), None)["body"])
        self.assertNotEqual(ok["revision"], rev1)
        # replaying the now-stale rev1 fails (someone already moved it)
        replay = handler_storage.handler(_event("/save-book", {
            "book": book, "expected_revision": rev1}), None)
        self.assertEqual(replay["statusCode"], 409)
        # no expected -> unconditional overwrite still works (human Save)
        plain = handler_storage.handler(_event("/save-book", {"book": book}), None)
        self.assertEqual(plain["statusCode"], 200)

    @patch("handler_storage.s3")
    def test_list_skips_nested_keys_and_reports_errors(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        _patch_s3(mock_s3, fake)
        handler_storage.handler(_event("/save-book", {"book": {
            "name": "Alpha", "entries": [_entry(1)]}}), None)
        handler_storage.handler(_event("/save-book", {"book": {
            "name": "Beta", "entries": []}}), None)
        # nested per-book objects share the prefix and MUST be skipped
        fake.put_object(Key="polypaint/books/alpha/assets/e1.provenance.json", Body=b"{}")
        fake.put_object(Key="polypaint/books/alpha/out/latest.json", Body=b"{}")
        # corrupt doc: listed under errors[], never breaks the listing
        fake.put_object(Key="polypaint/books/broken.json", Body=b"not-json")

        resp = handler_storage.handler(_event("/list-books", {}), None)
        body = json.loads(resp["body"])
        self.assertEqual(body["order"], "saved_at_desc")
        ids = [row["id"] for row in body["books"]]
        self.assertEqual(sorted(ids), ["alpha", "beta"])
        self.assertEqual(body["error_count"], 1)
        self.assertEqual(body["errors"][0]["id"], "broken")
        alpha = next(r for r in body["books"] if r["id"] == "alpha")
        self.assertEqual(alpha["entry_count"], 1)
        self.assertEqual(alpha["name"], "Alpha")

    @patch("handler_storage.s3")
    def test_delete_removes_doc_and_prefix_but_guards_unknown(self, mock_s3):
        import handler_storage

        fake = _FakeS3()
        _patch_s3(mock_s3, fake)
        handler_storage.handler(_event("/save-book", {"book": {
            "name": "Gone", "entries": [_entry(1)]}}), None)
        fake.put_object(Key="polypaint/books/gone/assets/e1.jpg", Body=b"x")
        fake.put_object(Key="polypaint/books/gone/out/c1/content.pdf", Body=b"x")

        resp = handler_storage.handler(_event("/delete-book", {"id": "gone"}), None)
        body = json.loads(resp["body"])
        self.assertEqual(body["deleted"], 3)
        self.assertFalse([k for k in fake.objects if "gone" in k])

        missing = handler_storage.handler(_event("/delete-book", {"id": "gone"}), None)
        self.assertEqual(missing["statusCode"], 404)

        bad = handler_storage.handler(_event("/delete-book", {"id": "a/b"}), None)
        self.assertEqual(bad["statusCode"], 400)


if __name__ == "__main__":
    unittest.main()
