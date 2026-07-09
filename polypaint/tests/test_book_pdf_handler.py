"""book_pdf prepare/compose ops (book-maker-design.md §5): idempotence,
snapshot content, phase sequence, saved_at pin, missing-asset failure."""
import io
import json
import os
import sys
import unittest
from unittest.mock import MagicMock, patch

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))


class _FakeS3:
    def __init__(self):
        self.objects = {}
        self.put_headers = {}

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
        self.put_headers[Key] = {"ContentType": ContentType, **_kw}
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
        # progress phases repeat (per-asset, per-flip-page): pin the SEQUENCE
        # of distinct stages, not the tick count
        phases = [p for _, p in self.statuses]
        deduped = [p for i, p in enumerate(phases) if i == 0 or phases[i - 1] != p]
        self.assertEqual(deduped, ["load_assets", "compose_tex", "latex_content",
                                   "latex_cover", "upload", "flipbook", "done"])
        # flip outcome rides the done status for the compile poller's log
        done_status = self.statuses[-1]
        self.assertEqual(done_status[0], "done")
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

    def _fake_pdftoppm(self, total_pages):
        from PIL import Image

        def run(cmd, capture_output=False, text=False, timeout=None, **_kw):
            if os.path.basename(cmd[0]) != "pdftoppm":
                raise AssertionError(f"unexpected subprocess: {cmd}")
            assert "-png" in cmd, "flip pages must render PNG intermediates (4:4:4 re-encode)"
            first = int(cmd[cmd.index("-f") + 1])
            last = int(cmd[cmd.index("-l") + 1])
            prefix = cmd[-1]
            # poppler pads to the digits of the DOCUMENT page count, NOT the
            # -l value (a 16-page book writes page-01.png for -f 1 -l 1)
            digits = len(str(total_pages))
            for n in range(first, last + 1):
                buf = io.BytesIO()
                Image.new("RGB", (2307, 2331), (18, 24, 41)).save(buf, format="PNG")
                with open(f"{prefix}-{str(n).zfill(digits)}.png", "wb") as fh:
                    fh.write(buf.getvalue())
            return MagicMock(returncode=0, stdout="", stderr="")

        return run

    def test_compose_renders_flipbook_pages_and_manifest(self):
        self._seed_book()
        with patch.object(self.book_pdf.subprocess, "run", side_effect=self._fake_pdftoppm(total_pages=8)):
            resp = self.book_pdf.handle_compose({
                "op": "compose", "job_id": "book#test", "task_id": "bookcomp_r9",
                "book_id": "test-book", "compile_id": "c9",
                "expected_saved_at": "2026-07-06T00:00:00Z",
            }, latex_runner=_fake_latex)
        body = json.loads(resp["body"])
        self.assertEqual(body["flip_page_count"], 8)

        immutable = "public, max-age=31536000, immutable"
        for n in range(1, 9):
            key = f"polypaint/books/test-book/out/c9/flip/p{n:04d}.jpg"
            self.assertIn(key, self.fake.objects)
            self.assertEqual(self.fake.put_headers[key]["ContentType"], "image/jpeg")
            self.assertEqual(self.fake.put_headers[key]["CacheControl"], immutable)

        flip_key = "polypaint/books/test-book/out/c9/flip/flip.json"
        flip = json.loads(self.fake.objects[flip_key])
        self.assertEqual(flip["page_count"], 8)
        self.assertEqual(flip["book_id"], "test-book")  # the "id" field, not name
        self.assertEqual(flip["pages"], [f"p{n:04d}.jpg" for n in range(1, 9)])
        self.assertEqual(flip["width_px"], 2307)
        self.assertEqual(flip["height_px"], 2331)
        self.assertEqual(self.fake.put_headers[flip_key]["CacheControl"], immutable)

        latest = json.loads(self.fake.objects["polypaint/books/test-book/out/latest.json"])
        self.assertEqual(latest["flip_key"], flip_key)
        self.assertEqual(latest["flip_page_count"], 8)
        self.assertNotIn("flip_error", latest)
        # the mutable pointer the public viewer polls must never cache
        self.assertEqual(
            self.fake.put_headers["polypaint/books/test-book/out/latest.json"]["CacheControl"],
            "no-cache, max-age=0")

    def test_flipbook_handles_document_padded_page_names(self):
        # 7 entries -> 1 + 14 + 1 pad = 16 pages: poppler writes page-01.png
        # for page 1, which a digits-of(-l) assumption misses (prod bug)
        self._seed_book(entries=7)
        with patch.object(self.book_pdf.subprocess, "run", side_effect=self._fake_pdftoppm(total_pages=16)):
            resp = self.book_pdf.handle_compose({
                "op": "compose", "job_id": "book#test", "task_id": "bookcomp_r11",
                "book_id": "test-book", "compile_id": "c11",
                "expected_saved_at": "2026-07-06T00:00:00Z",
            }, latex_runner=_fake_latex)
        body = json.loads(resp["body"])
        self.assertEqual(body.get("flip_page_count"), 16, body.get("flip_error"))
        flip = json.loads(self.fake.objects["polypaint/books/test-book/out/c11/flip/flip.json"])
        self.assertEqual(flip["pages"], [f"p{n:04d}.jpg" for n in range(1, 17)])

    def test_flip_convert_survives_high_entropy_pages(self):
        # book2 page 17: a perfectly valid PNG whose optimized JPEG encode
        # dies at Pillow's default 64KB MAXBLOCK (libjpeg cannot suspend
        # during optimize's whole-output buffering). Random pixels reproduce
        # the class; the flip pipeline must bump MAXBLOCK and convert.
        import os as _os
        from PIL import Image, ImageFile
        noisy = Image.frombytes("RGB", (1024, 1024), _os.urandom(1024 * 1024 * 3))
        src = "/tmp/noisy_page.png"
        noisy.save(src, format="PNG")
        baseline_failed = False
        old_block = ImageFile.MAXBLOCK
        ImageFile.MAXBLOCK = 65536
        try:
            Image.open(src).save("/tmp/noisy_base.jpg", format="JPEG",
                                 quality=88, subsampling=0, optimize=True)
        except OSError:
            baseline_failed = True
        finally:
            ImageFile.MAXBLOCK = old_block
        self.assertTrue(baseline_failed,
                        "random pixels no longer trip default MAXBLOCK; refresh this test")

        self._seed_book()
        from PIL import Image as PILImage

        def noisy_pdftoppm(cmd, capture_output=False, text=False, timeout=None, **_kw):
            first = int(cmd[cmd.index("-f") + 1])
            prefix = cmd[-1]
            page = PILImage.frombytes("RGB", (1024, 1024), _os.urandom(1024 * 1024 * 3))
            page.save(f"{prefix}-{first}.png", format="PNG")
            return MagicMock(returncode=0, stdout="", stderr="")

        with patch.object(self.book_pdf.subprocess, "run", side_effect=noisy_pdftoppm):
            resp = self.book_pdf.handle_compose({
                "op": "compose", "job_id": "book#test", "task_id": "bookcomp_r13",
                "book_id": "test-book", "compile_id": "c13",
                "expected_saved_at": "2026-07-06T00:00:00Z",
            }, latex_runner=_fake_latex)
        body = json.loads(resp["body"])
        self.assertEqual(body.get("flip_page_count"), 8, body.get("flip_error"))

    def test_flip_page_convert_retries_with_fresh_render(self):
        # a truncated PNG on the first render must heal via re-render, not
        # fail the flipbook (live incident: "broken data stream" once, no
        # in-container repro)
        self._seed_book()
        from PIL import Image
        calls = {"n": 0}

        def flaky_pdftoppm(cmd, capture_output=False, text=False, timeout=None, **_kw):
            if os.path.basename(cmd[0]) != "pdftoppm":
                raise AssertionError(f"unexpected subprocess: {cmd}")
            first = int(cmd[cmd.index("-f") + 1])
            prefix = cmd[-1]
            calls["n"] += 1
            buf = io.BytesIO()
            Image.new("RGB", (2307, 2331), (18, 24, 41)).save(buf, format="PNG")
            data = buf.getvalue()
            if first == 3 and calls["n"] < 20:
                # first render of page 3: truncated png (strict decode fails)
                data = data[: len(data) // 2]
                calls["n"] = 99  # only once
            with open(f"{prefix}-{first}.png", "wb") as fh:
                fh.write(data)
            return MagicMock(returncode=0, stdout="", stderr="")

        with patch.object(self.book_pdf.subprocess, "run", side_effect=flaky_pdftoppm):
            resp = self.book_pdf.handle_compose({
                "op": "compose", "job_id": "book#test", "task_id": "bookcomp_r12",
                "book_id": "test-book", "compile_id": "c12",
                "expected_saved_at": "2026-07-06T00:00:00Z",
            }, latex_runner=_fake_latex)
        body = json.loads(resp["body"])
        self.assertEqual(body.get("flip_page_count"), 8, body.get("flip_error"))
        from PIL import ImageFile
        self.assertFalse(ImageFile.LOAD_TRUNCATED_IMAGES)

    def test_compose_survives_flipbook_failure(self):
        self._seed_book()

        def boom(cmd, capture_output=False, text=False, timeout=None, **_kw):
            return MagicMock(returncode=1, stdout="", stderr="pdftoppm exploded")

        with patch.object(self.book_pdf.subprocess, "run", side_effect=boom):
            resp = self.book_pdf.handle_compose({
                "op": "compose", "job_id": "book#test", "task_id": "bookcomp_r10",
                "book_id": "test-book", "compile_id": "c10",
                "expected_saved_at": "2026-07-06T00:00:00Z",
            }, latex_runner=_fake_latex)
        body = json.loads(resp["body"])
        # the PDF is the primary artifact: compile succeeds without a flipbook
        self.assertIn("polypaint/books/test-book/out/c10/content.pdf", self.fake.objects)
        latest = json.loads(self.fake.objects["polypaint/books/test-book/out/latest.json"])
        self.assertIn("pdftoppm", latest["flip_error"])
        self.assertNotIn("flip_key", latest)
        self.assertEqual(body["flip_error"], latest["flip_error"])

    def test_handler_unknown_op_reports_error(self):
        with self.assertRaises(RuntimeError):
            self.book_pdf.handler({"body": json.dumps({"op": "nope", "job_id": "j",
                                                       "task_id": "t"})}, None)
        self.assertEqual(self.statuses[-1][0], "error")


if __name__ == "__main__":
    unittest.main()


class TestSourceFontBundling(unittest.TestCase):
    """code-review-25 F7: the downloadable source.zip must carry only the
    fonts the template references, and never a trial/demo face."""

    def test_only_referenced_non_trial_fonts_are_bundled(self):
        import book_pdf
        content_tex = (r"\setmainfont{Baramond-Regular.ttf}"
                       r"\newfontfamily\monofont{CourierPrime-Regular.ttf}")
        cover_tex = r"\displayfont{Baramond-Regular.ttf}"
        on_disk = [
            "Baramond-Regular.ttf", "CourierPrime-Regular.ttf",   # referenced, complete
            "TiemposText-Regular-Trial.ttf",                       # trial (never)
            "Sohne-Buch-Trial.ttf", "Lyon-Regular.ttf",           # trial / unreferenced
            "JetBrainsMono-Regular.ttf",                           # unreferenced
            "notes.txt",                                           # not a font
        ]
        picked = book_pdf._redistributable_source_fonts(content_tex, cover_tex, on_disk)
        self.assertEqual(sorted(picked),
                         ["Baramond-Regular.ttf", "CourierPrime-Regular.ttf"])

    def test_a_referenced_trial_font_is_still_excluded(self):
        import book_pdf
        # even if the template ever named a trial face, redistribution is barred
        picked = book_pdf._redistributable_source_fonts(
            r"\setmainfont{TiemposText-Regular-Trial.ttf}", "",
            ["TiemposText-Regular-Trial.ttf"])
        self.assertEqual(picked, [])


class TestViewportSummary(unittest.TestCase):
    def test_explicit_drops_q_shim_and_uses_two_sig_digits(self):
        import book_pdf
        meta = {"view_mode": "explicit", "quantile": 0, "shim": 0.05,
                "min_re": "-0.5905770748695542", "max_re": "-0.029793800946478655",
                "min_im": "-1.1846176287507992", "max_im": "-0.6238343548277241"}
        out = book_pdf._viewport_summary(meta)
        self.assertEqual(out, "explicit, re [-0.59, -0.03], im [-1.2, -0.62]")
        self.assertNotIn("q=", out)
        self.assertNotIn("shim", out)

    def test_auto_keeps_q_shim_and_square_formats_extent(self):
        import book_pdf
        self.assertEqual(book_pdf._viewport_summary(
            {"view_mode": "auto", "quantile": 0.001, "shim": 0.005}),
            "auto, q=0.1%, shim=0.5%")
        self.assertEqual(book_pdf._viewport_summary(
            {"view_mode": "square", "square_extent": "0.6238343548277241"}),
            "square, extent=0.62")

    def test_pdf_handler_viewport_matches(self):
        # both PDF paths (ColorSpread button + book) must format identically
        import book_pdf
        import handler_pdf_artifact
        meta = {"view_mode": "explicit", "min_re": "-0.5", "max_re": "0.25",
                "min_im": "-1", "max_im": "1", "quantile": 0.1}
        self.assertEqual(book_pdf._viewport_summary(meta),
                         handler_pdf_artifact._viewport_summary(meta))
