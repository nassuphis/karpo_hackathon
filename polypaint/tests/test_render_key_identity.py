"""Structured render-key identity (code-review-28 F12).

assert_render_identity used a substring test (`/<artifact_id>/ in key`) that
accepted the artifact id 'color'/'palettes' or the literal job id for an
unrelated key, letting a book/PDF page pair image B with metadata A. One
structured parser (shared.parse_render_key) now compares EXACT components.
"""
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "lambda"))

from shared import parse_render_key, assert_render_identity


class TestParseRenderKey(unittest.TestCase):
    def test_canonical(self):
        p = parse_render_key("renders/jobA/color/real-art/image.jpeg")
        self.assertEqual(p["variant"], "canonical")
        self.assertEqual(p["job"], "jobA")
        self.assertEqual(p["family"], "color")
        self.assertEqual(p["artifact_id"], "real-art")
        self.assertEqual(p["leaf"], "image.jpeg")

    def test_canonical_palettes_family(self):
        p = parse_render_key("renders/j/palettes/pal_7/image.jpeg")
        self.assertEqual(p["variant"], "canonical")
        self.assertEqual(p["family"], "palettes")
        self.assertEqual(p["artifact_id"], "pal_7")

    def test_canonical_views_family(self):
        # CR36 follow-up: views are a first-class family — every family
        # parser must agree, or DeepZoom provenance loses the artifact
        parsed = parse_render_key("renders/j/views/view_9_iso_t1/image.jpeg")
        self.assertEqual(parsed["variant"], "canonical")
        self.assertEqual(parsed["family"], "views")
        self.assertEqual(parsed["artifact_id"], "view_9_iso_t1")

    def test_canonical_deeper_leaf(self):
        p = parse_render_key("renders/j/color/a/chunks/section_0.bin")
        self.assertEqual(p["variant"], "canonical")
        self.assertEqual(p["artifact_id"], "a")
        self.assertEqual(p["leaf"], "section_0.bin")

    def test_legacy_root(self):
        for k in ("renders/j/image.jpeg", "renders/j/preview_color.png", "renders/j/calc.json"):
            p = parse_render_key(k)
            self.assertEqual(p["variant"], "legacy_root", k)
            self.assertIsNone(p["artifact_id"])
            self.assertEqual(p["job"], "j")

    def test_job_scoped_chunk_dir(self):
        p = parse_render_key("renders/j/color/a")  # no leaf beyond artifact
        self.assertIn(p["variant"], ("job_scoped",))
        p2 = parse_render_key("renders/j/notafamily/x/y.bin")
        self.assertEqual(p2["variant"], "job_scoped")
        self.assertIsNone(p2["family"])
        self.assertIsNone(p2["artifact_id"])

    def test_invalid(self):
        for k in ("", "foo/bar", "renders", "renders/", "notrenders/j/color/a/i.jpg"):
            self.assertEqual(parse_render_key(k)["variant"], "invalid", repr(k))

    def test_never_raises(self):
        for k in (None, 123, "renders//color//"):
            parse_render_key(k)  # must not raise


class TestAssertRenderIdentity(unittest.TestCase):
    KEY = "renders/jobA/color/real-art/image.jpeg"

    def test_accepts_exact_artifact(self):
        self.assertEqual(assert_render_identity(self.KEY, "jobA", "real-art"), self.KEY)

    def test_rejects_family_dir_as_artifact(self):
        # 'color' is the family segment, not the artifact — old substring accepted it
        with self.assertRaises(ValueError):
            assert_render_identity(self.KEY, "jobA", "color")

    def test_rejects_job_id_as_artifact(self):
        # 'jobA' appears as /jobA/ in the key but is not the artifact segment
        with self.assertRaises(ValueError):
            assert_render_identity(self.KEY, "jobA", "jobA")

    def test_rejects_palettes_dir_as_artifact(self):
        pal_key = "renders/jobA/palettes/real-pal/image.jpeg"
        with self.assertRaises(ValueError):
            assert_render_identity(pal_key, "jobA", "palettes")
        # but the real palette artifact id is accepted
        self.assertEqual(assert_render_identity(pal_key, "jobA", "real-pal"), pal_key)

    def test_rejects_job_mismatch(self):
        with self.assertRaises(ValueError):
            assert_render_identity(self.KEY, "otherjob", "real-art")

    def test_rejects_legacy_root_key(self):
        # a legacy single-artifact-per-job key has no artifact segment to match
        with self.assertRaises(ValueError):
            assert_render_identity("renders/jobA/image.jpeg", "jobA", "real-art")

    def test_rejects_blank_artifact(self):
        with self.assertRaises(ValueError):
            assert_render_identity(self.KEY, "jobA", "")


if __name__ == "__main__":
    unittest.main()
