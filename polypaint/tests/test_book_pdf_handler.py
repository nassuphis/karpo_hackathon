"""book_pdf prepare/compose ops (book-maker-design.md §5): idempotence,
snapshot content, phase sequence, saved_at pin, missing-asset failure."""
import io
import json
import os
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _FakeS3:
    def __init__(self):
        self.objects = {}

    def get_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise Exception("NoSuchKey")

        class _Body(io.BytesIO):
            def iter_chunks(self, chunk_size=1024):
                while True:
                    chunk = self.read(chunk_size)
                    if not chunk:
                        return
                    yield chunk

        return {"Body": _Body(self.objects[Key])}

    def head_object(self, Bucket=None, Key=None):
        if Key not in self.objects:
            raise Exception("NoSuchKey")
        return {"Metadata": {"color_mode": "proximity", "palette": "tri_ember"}}

    def put_object(self, Bucket=None, Key=None, Body=None, ContentType=None, **_kw):
        self.objects[Key] = Body if isinstance(Body, bytes) else str(Body or "").encode()
        return {}

    def upload_file(self, path, Bucket, Key, ExtraArgs=None):
        self.objects[Key] = open(path, "rb").read()

    def upload_fileobj(self, fh, Bucket, Key, ExtraArgs=None):
        self.objects[Key] = fh.read()

    def download_file(self, Bucket, Key, path):
        if Key not in self.objects:
            raise Exception("NoSuchKey")
        with open(path, "wb") as out:
            out.write(self.objects[Key])


def _tiny_jpeg():
    from PIL import Image
    buf = io.BytesIO()
    Image.new("RGB", (64, 64), (40, 10, 60)).save(buf, format="JPEG")
    return buf.getvalue()


def _fake_latex(build_dir, name):
    with open(os.path.join(build_dir, f"{name}.pdf"), "wb") as fh:
        fh.write(b"%PDF-1.5 fake\n")


def _prepare_params(**over):
    params = {
        "op": "prepare", "job_id": "book#test", "task_id": "bookprep_r1_e1",
        "book_id": "test-book", "entry_id": "e1",
        "source_job_id": "srcjob", "source_artifact_id": "art1",
        "source_image_key": "renders/srcjob/color/art1/image.jpeg",
    }
    params.update(over)
    return params


class TestBookPdfHandler(unittest.TestCase):
    def setUp(self):
        import book_pdf
        self.book_pdf = book_pdf
        self.fake = _FakeS3()
        self.statuses = []
        self.p_s3 = patch.object(book_pdf, "s3", self.fake)
        self.p_status = patch.object(
            book_pdf, "report_status",
            lambda job, task, status, error_msg=None, result_data=None:
                self.statuses.append((status, (result_data or {}).get("phase"))))
        self.p_s3.start()
        self.p_status.start()
        self.addCleanup(self.p_s3.stop)
        self.addCleanup(self.p_status.stop)

    def test_prepare_writes_asset_and_snapshot_then_short_circuits(self):
        self.fake.objects["renders/srcjob/color/art1/image.jpeg"] = _tiny_jpeg()
        self.fake.objects["renders/srcjob/calc.json"] = json.dumps({
            "degree": "50", "N": 4000, "solver": "aberth_mt",
            "pipeline": {"function": "poly_42"},
        }).encode()

        resp = self.book_pdf.handle_prepare(_prepare_params())
        body = json.loads(resp["body"])
        self.assertFalse(body["cached"])
        self.assertIn("polypaint/books/test-book/assets/e1.jpg", self.fake.objects)
        snap = json.loads(self.fake.objects["polypaint/books/test-book/assets/e1.provenance.json"])
        # report model mirrors the ColorSpread KV rows
        rows = {r[0]: r[1] for r in snap["report"]["summary_rows"]}
        self.assertEqual(rows["Function"], "poly_42")
        self.assertEqual(rows["Degree"], "50")
        self.assertEqual(rows["Solver"], "AE-MT")            # aberth_mt label
        self.assertIn("proximity", rows.get("Color mode", ""))
        self.assertEqual(snap["report"]["compute_id"], "srcjob")
        self.assertFalse(snap["report"]["has_palette"])      # no associated palette
        self.assertEqual([s for s, _ in self.statuses][-1], "done")

        # second call without force: cache short-circuit, no source read needed
        self.statuses.clear()
        resp = self.book_pdf.handle_prepare(_prepare_params())
        self.assertTrue(json.loads(resp["body"])["cached"])
        self.assertEqual(self.statuses, [("done", "done")])

        # Compile passes force=true: re-prepares even though the asset is cached
        self.statuses.clear()
        resp = self.book_pdf.handle_prepare(_prepare_params(force=True))
        self.assertFalse(json.loads(resp["body"])["cached"])
        self.assertIn("load_source", [p for _, p in self.statuses])

    def test_prepare_finds_palette_via_overlay_meta(self):
        # associated_palette_image_key lives in the color-artifact overlay
        # meta.json (renders/{job}/color/{art}/meta.json) — NOT the S3 object
        # metadata. This is the source the ColorSpread PDF button uses; reading
        # only head_object metadata (the earlier bug) missed the palette.
        self.fake.objects["renders/srcjob/color/art1/image.jpeg"] = _tiny_jpeg()
        self.fake.objects["renders/srcjob/calc.json"] = json.dumps(
            {"solver": "aberth", "pipeline": {"function": "poly_9"}}).encode()
        self.fake.objects["renders/srcjob/color/art1/meta.json"] = json.dumps({
            "associated_palette_image_key": "renders/srcjob/palette/p1/image.jpeg",
            "associated_palette_id": "tri_ember",
        }).encode()
        self.fake.objects["renders/srcjob/palette/p1/image.jpeg"] = _tiny_jpeg()

        resp = self.book_pdf.handle_prepare(_prepare_params())
        self.assertFalse(json.loads(resp["body"])["cached"])
        snap = json.loads(self.fake.objects["polypaint/books/test-book/assets/e1.provenance.json"])
        self.assertTrue(snap["report"]["has_palette"])
        self.assertEqual(snap["report"]["palette_label"], "tri_ember")
        # the prepared palette swatch was uploaded next to the image asset
        self.assertIn("polypaint/books/test-book/assets/e1.palette.jpg", self.fake.objects)

    def _seed_book(self, saved_at="2026-07-06T00:00:00Z", entries=2):
        book = {
            "book_kind": "book", "id": "test-book", "name": "T", "title": "T",
            "saved_at": saved_at, "cover_entry_id": "e0",
            "entries": [{"entry_id": f"e{i}", "job_id": "j", "artifact_id": f"a{i}",
                         "image_key": f"k{i}"} for i in range(entries)],
        }
        self.fake.objects["polypaint/books/test-book.json"] = json.dumps(book).encode()
        for i in range(entries):
            self.fake.objects[f"polypaint/books/test-book/assets/e{i}.jpg"] = _tiny_jpeg()
            self.fake.objects[f"polypaint/books/test-book/assets/e{i}.provenance.json"] = \
                json.dumps({"report": {"compute_id": "j", "summary_rows": [["Function", f"poly_{i}"]],
                                       "has_palette": False}}).encode()

    def test_compose_uploads_outputs_and_latest_pointer(self):
        self._seed_book()
        resp = self.book_pdf.handle_compose({
            "op": "compose", "job_id": "book#test", "task_id": "bookcomp_r1",
            "book_id": "test-book", "compile_id": "c1",
            "expected_saved_at": "2026-07-06T00:00:00Z",
        }, latex_runner=_fake_latex)
        body = json.loads(resp["body"])
        self.assertEqual(body["content_pages"], 8)  # 1 + 4 + 3 pad
        for key in ("content.pdf", "cover.pdf", "source.zip"):
            self.assertIn(f"polypaint/books/test-book/out/c1/{key}", self.fake.objects)
        latest = json.loads(self.fake.objects["polypaint/books/test-book/out/latest.json"])
        self.assertEqual(latest["compile_id"], "c1")
        self.assertEqual(latest["spread_count"], 2)
        phases = [p for _, p in self.statuses]
        self.assertEqual(phases, ["load_assets", "compose_tex", "latex_content",
                                  "latex_cover", "upload", "done"])
        import zipfile
        zf = zipfile.ZipFile(io.BytesIO(
            self.fake.objects["polypaint/books/test-book/out/c1/source.zip"]))
        self.assertIn("book.tex", zf.namelist())
        self.assertIn("assets/e0.jpg", zf.namelist())

    def test_compose_rejects_saved_at_mismatch_and_missing_assets(self):
        self._seed_book(saved_at="2026-07-06T00:00:00Z")
        with self.assertRaises(RuntimeError) as ctx:
            self.book_pdf.handle_compose({
                "op": "compose", "job_id": "j", "task_id": "t",
                "book_id": "test-book", "compile_id": "c2",
                "expected_saved_at": "different",
            }, latex_runner=_fake_latex)
        self.assertIn("saved mid-compile", str(ctx.exception))

        del self.fake.objects["polypaint/books/test-book/assets/e1.jpg"]
        with self.assertRaises(RuntimeError) as ctx:
            self.book_pdf.handle_compose({
                "op": "compose", "job_id": "j", "task_id": "t",
                "book_id": "test-book", "compile_id": "c3",
                "expected_saved_at": "2026-07-06T00:00:00Z",
            }, latex_runner=_fake_latex)
        self.assertIn("missing prepared assets", str(ctx.exception))
        self.assertIn("e1", str(ctx.exception))

    def test_handler_unknown_op_reports_error(self):
        with self.assertRaises(RuntimeError):
            self.book_pdf.handler({"body": json.dumps({"op": "nope", "job_id": "j",
                                                       "task_id": "t"})}, None)
        self.assertEqual(self.statuses[-1][0], "error")


if __name__ == "__main__":
    unittest.main()
