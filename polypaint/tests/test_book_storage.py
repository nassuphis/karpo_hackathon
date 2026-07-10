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
    for name in ("get_object", "put_object", "head_object", "delete_object", "delete_objects", "list_objects_v2"):
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
