"""book_tex template layer (book-maker-design.md §6/§10): page-plan math,
escaping, geometry constants, override-vs-auto — all TeX-free."""
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

import book_tex


def _book(n_entries, **over):
    book = {
        "name": "Test Book",
        "title": "PolyPaint",
        "entries": [
            {"entry_id": f"e{i}", "job_id": f"j{i}", "artifact_id": f"a{i}",
             "image_key": f"renders/j{i}/color/a{i}/image.jpeg"}
            for i in range(n_entries)
        ],
    }
    book.update(over)
    return book


class TestPagePlan(unittest.TestCase):
    def test_front_matter_is_odd_and_total_is_multiple_of_four(self):
        for n in range(0, 60):
            total, pad = book_tex.page_plan(n)
            self.assertEqual(total % 4, 0, n)
            self.assertEqual(total, max(4, 1 + 2 * n + pad), n)
            # even N -> pad 3, odd N -> pad 1 (design §6)
            if 1 + 2 * n >= 4:
                self.assertEqual(pad, 3 if n % 2 == 0 else 1, n)

    def test_36_entries_is_76_pages(self):
        self.assertEqual(book_tex.page_plan(36), (76, 3))


class TestEscaping(unittest.TestCase):
    def test_specials_all_escape(self):
        hot = "\\ { } $ & # % ^ _ ~"
        out = book_tex.tex_escape(hot)
        for raw in ["$", "&", "#", "%", "_", "~", "{", "}"]:
            self.assertNotIn(raw, out.replace("\\" + raw, "").replace(
                r"\textasciitilde{}", "").replace(r"\textasciicircum{}", "").replace(
                r"\textbackslash{}", "").replace(r"\{", "").replace(r"\}", ""))
        self.assertIn(r"\$", out)
        self.assertIn(r"\%", out)
        self.assertIn(r"\textbackslash{}", out)

    def test_control_chars_drop_and_newlines_break(self):
        out = book_tex.tex_escape("a\x00b\nc")
        self.assertNotIn("\x00", out)
        self.assertIn("\\\\", out)

    def test_user_text_reaches_tex_escaped(self):
        book = _book(1)
        book["entries"][0]["title_override"] = "100% pure $ art"
        tex, _ = book_tex.render_content_tex(book)
        self.assertIn(r"100\% pure \$ art", tex)
        self.assertNotIn("100% pure $ art", tex)


class TestGeometryAndStructure(unittest.TestCase):
    def test_content_preamble_pins_whitewall_geometry(self):
        tex, total = book_tex.render_content_tex(_book(3))
        self.assertIn("paperwidth=293mm", tex)
        self.assertIn("hmargin=24mm, vmargin=1mm", tex)
        self.assertIn("paperheight=296mm", tex)
        self.assertEqual(total, 8)  # 1 + 6 + 1 pad
        self.assertEqual(tex.count(r"\includegraphics"), 3)
        self.assertIn("assets/e0.jpg", tex)
        self.assertIn(r"\usepackage{fontspec}", tex)
        self.assertIn(r"\usepackage{microtype}", tex)

    def test_cover_pins_computed_dims_not_script_comments(self):
        tex = book_tex.render_cover_tex(_book(2), "assets/e1.jpg")
        self.assertIn("paperwidth=629mm", tex)
        self.assertIn("paperheight=316mm", tex)
        self.assertNotIn("629.4", tex)
        self.assertIn("assets/e1.jpg", tex)

    def test_pad_pages_are_deep_blue(self):
        tex, total = book_tex.render_content_tex(_book(2))  # 1+4 -> pad 3, total 8
        self.assertEqual(total, 8)
        self.assertEqual(tex.count(r"\pagecolor{pagebg}" + "\n" + r"\null"), 3)
        self.assertIn(r"\definecolor{pagebg}{HTML}{1A1A2E}", tex)  # app deep blue


class TestReportPage(unittest.TestCase):
    def test_report_kv_grid_and_palette_from_provenance(self):
        book = _book(1)
        prov = {"e0": {"report": {
            "compute_id": "compute_mr7kkhg2",
            "artifact_id": "color_run_123",
            "summary_rows": [["Function", "const(2,0,0)"], ["Degree", "50"],
                             ["Solver", "AE-MT"], ["Color mode", "root proximity"]],
            "palette_label": "tri_ember",
            "has_palette": True,
        }}}
        tex, _ = book_tex.render_content_tex(book, prov)
        # compute id is the title (no artsy title yet), on the deep-blue page
        self.assertIn("compute\\_mr7kkhg2", tex)
        self.assertIn(r"\pagecolor{pagebg}\color{bodytext}", tex)
        # KV grid, uppercased labels, no program-source dump
        self.assertIn("FUNCTION", tex)
        self.assertIn("const(2,0,0)", tex)
        self.assertIn("AE-MT", tex)
        self.assertNotIn("typed", tex)   # the old token-dump body is gone
        # palette swatch in a panel: fixed 132mm square, label BELOW the image
        self.assertIn("e0.palette.jpg", tex)
        self.assertIn(r"\fcolorbox{panelborder}{panelbg}", tex)
        self.assertIn("width=160mm,height=160mm,keepaspectratio", tex)
        self.assertIn("tri\\_ember", tex)
        self.assertLess(tex.index("e0.palette.jpg"), tex.index("tri\\_ember"))

    def test_palette_shrinks_on_dense_versos_never_spills(self):
        # empirically calibrated on book2: a 9-row verso overflowed at a
        # fixed 160mm — the square must adapt so the spread invariant holds
        # vmargin=1mm frees ~46mm: 160 fits every realistic verso now,
        # including 10 rows with a body paragraph
        self.assertEqual(book_tex._palette_mm([("k", "v")] * 9, ""), 160)
        self.assertEqual(book_tex._palette_mm([("k", "v")] * 10, ""), 160)
        body = "A petrol lattice over bone. " * 6   # ~2 wrapped lines
        self.assertEqual(book_tex._palette_mm([("k", "v")] * 10, body), 160)
        # the guard still exists for pathological content
        huge_body = "line\n" * 12
        self.assertLess(book_tex._palette_mm([("k", "v")] * 12, huge_body), 160)
        self.assertGreaterEqual(book_tex._palette_mm([("k", "v")] * 12, huge_body),
                                book_tex.PALETTE_MIN_MM)

    def test_no_palette_omits_swatch(self):
        book = _book(1)
        prov = {"e0": {"report": {"compute_id": "c", "summary_rows": [["N", "4000"]],
                                  "has_palette": False}}}
        tex, _ = book_tex.render_content_tex(book, prov)
        self.assertNotIn(".palette.jpg", tex)
        self.assertIn("4000", tex)

    def test_overrides_win(self):
        book = _book(1)
        book["entries"][0]["title_override"] = "My Title"
        book["entries"][0]["body_override"] = "line one\nline two"
        tex, _ = book_tex.render_content_tex(
            book, {"e0": {"report": {"compute_id": "c", "summary_rows": []}}})
        self.assertIn("My Title", tex)
        self.assertIn("line one", tex)


if __name__ == "__main__":
    unittest.main()
