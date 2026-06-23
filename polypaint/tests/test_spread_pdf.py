import os
import base64
import re
import sys
import tempfile
import unittest
import zlib


ROOT = os.path.join(os.path.dirname(__file__), "..")
sys.path.insert(0, os.path.join(ROOT, "lambda"))
try:
    import PIL  # noqa: F401
    import reportlab  # noqa: F401
except ModuleNotFoundError:
    # The Lambda layer is Linux-built. Prefer the local venv when present; use
    # the layer only on compatible test hosts that do not have local deps.
    sys.path.insert(0, os.path.join(ROOT, "lambda", "layer-build-pdf", "python"))


def _pdf_stream_text(path):
    """Return decoded PDF stream text without depending on pypdf/pdfplumber."""
    with open(path, "rb") as fh:
        data = fh.read()
    chunks = []
    for match in re.finditer(rb"stream\r?\n(.*?)endstream", data, re.S):
        raw = match.group(1).strip()
        candidates = [raw]
        try:
            candidates.append(zlib.decompress(raw))
        except Exception:
            pass
        try:
            candidates.append(zlib.decompress(base64.a85decode(raw, adobe=True)))
        except Exception:
            pass
        for candidate in candidates:
            try:
                chunks.append(candidate.decode("latin-1", "ignore"))
            except Exception:
                pass
    return "\n".join(chunks)


class TestSpreadPdf(unittest.TestCase):

    def test_prepare_pdf_image_downsamples_large_input(self):
        from PIL import Image
        from spread_pdf import prepare_pdf_image

        with tempfile.TemporaryDirectory() as td:
            src = os.path.join(td, "src.png")
            out = os.path.join(td, "prepared.png")
            Image.new("RGB", (320, 160), (20, 40, 80)).save(src)

            info = prepare_pdf_image(src, out, max_px=80)

            self.assertTrue(os.path.exists(out))
            self.assertEqual(info["source_width"], 320)
            self.assertEqual(info["source_height"], 160)
            self.assertEqual(info["prepared_width"], 80)
            self.assertEqual(info["prepared_height"], 40)
            self.assertTrue(info["resized"])
            self.assertEqual(info["prepared_format"], "png")

    def test_build_color_spread_pdf_report_smoke(self):
        from PIL import Image
        from spread_pdf import build_color_spread_pdf

        with tempfile.TemporaryDirectory() as td:
            img = os.path.join(td, "image.png")
            palette = os.path.join(td, "palette.png")
            pdf = os.path.join(td, "out.pdf")
            Image.new("RGB", (80, 60), (120, 20, 30)).save(img)
            Image.new("RGB", (32, 32), (20, 90, 160)).save(palette)
            coeff_source = "\n".join([
                "cf",
                "poly = rev(poly)",
                "emit",
                "poly[1] = p1",
                "poly[2] = p2",
                "poly[3] = p1+p2",
                "poly[4] = p1*p2",
                "poly[5] = p1-p2",
                "poly[6] = p2-p1",
                "UNIQ_PROG_LINE_42 = poly_plus_one",
            ])
            report = {
                "title": "compute_demo / color_demo",
                "compute_id": "compute_demo",
                "color_artifact_id": "color_demo",
                "palette_label": "pal_demo_123",
                "summary_rows": [
                    ("Function", "poly_1"),
                    ("Degree", "25"),
                    ("N", "1000"),
                    ("Times", "1"),
                    ("Solver", "AE-MT"),
                    ("Color mode", "solve score"),
                    ("Interpretation", "rgb"),
                    ("Palette", "none"),
                    ("Output channels", "3"),
                    ("Viewport", "auto, q=0.5%"),
                ],
                "programs": [
                    {
                        "label": "Param Program",
                        "language": "poly-param",
                        "source": "p1 = exp(t1*pi2i)\np2 = exp(t2*pi2i)",
                    },
                    {
                        "label": "Coeff Program",
                        "language": "poly-coeff",
                        "source": coeff_source,
                    },
                ],
            }

            result = build_color_spread_pdf(img, pdf, "compute_demo / color_demo", report=report, palette_image_path=palette)

            self.assertTrue(os.path.exists(pdf))
            self.assertGreater(os.path.getsize(pdf), 1000)
            self.assertGreaterEqual(result["page_count"], 2)
            with open(pdf, "rb") as fh:
                data = fh.read()
            self.assertEqual(data[:5], b"%PDF-")
            self.assertIn(b"compute_demo", data)
            self.assertIn(b"color_demo", data)

            stream_text = _pdf_stream_text(pdf)
            self.assertIn("compute_demo", stream_text)
            self.assertIn("color_demo", stream_text)
            self.assertIn("pal_demo_123", stream_text)
            self.assertIn("Source details continue in the appendix.", stream_text)
            self.assertNotIn("program excerpts", stream_text.lower())
            self.assertNotIn("full source on appendix", stream_text)
            self.assertIn("Source Appendix 1", stream_text)
            self.assertIn("UNIQ_PROG_LINE_42", stream_text)


if __name__ == "__main__":
    unittest.main()
