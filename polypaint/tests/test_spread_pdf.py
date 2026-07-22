import os
import base64
import re
import sys
import tempfile
import unittest
import zlib
from types import SimpleNamespace
from unittest.mock import patch


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

    def test_program_source_display_lines_split_top_level_semicolons(self):
        from spread_pdf import _program_source_display_lines

        lines = _program_source_display_lines(
            "legacy(unit_circle, both, both); "
            "legacy(moebius, both, both, 3+6j, 2+1j, -1+4j, -3-10j); "
            "coeff2"
        )

        self.assertEqual(lines, [
            "legacy(unit_circle, both, both)",
            "legacy(moebius, both, both, 3+6j, 2+1j, -1+4j, -3-10j)",
            "coeff2",
        ])

        lines = _program_source_display_lines("emit(note='a;b'); emit(score)")
        self.assertEqual(lines, ["emit(note='a;b')", "emit(score)"])

    def test_program_source_wraps_at_syntax_boundaries_within_panel_width(self):
        from reportlab.pdfbase import pdfmetrics
        from spread_pdf import (
            CODE_PAD_X,
            CODE_WRAP_GUTTER,
            CONTENT_W,
            F_CODE,
            _wrapped_program_lines,
        )

        term = "+".join(["floor(1/(1+k))"] * 14)
        source = f"poly = scan(23, 0, tos[0]*({term}), tos[k]*({term}))"
        lines = _wrapped_program_lines({"source": source})
        usable_width = CONTENT_W - 2 * CODE_PAD_X - CODE_WRAP_GUTTER

        self.assertGreater(len(lines), 2)
        self.assertEqual(lines[0], "poly = scan(23, 0,")
        self.assertTrue(all(
            pdfmetrics.stringWidth(line, F_CODE[0], F_CODE[1]) <= usable_width + 1e-6
            for line in lines
        ))
        self.assertTrue(all(line.startswith("  ") for line in lines[1:]))

        # Even a source token with no safe punctuation gets a measured hard
        # break rather than crossing the right edge of the code panel.
        token_lines = _wrapped_program_lines({"source": "x" * 500})
        self.assertGreater(len(token_lines), 2)
        self.assertTrue(all(
            pdfmetrics.stringWidth(line, F_CODE[0], F_CODE[1]) <= usable_width + 1e-6
            for line in token_lines
        ))

    def test_appendix_program_columns_place_score_on_right(self):
        from spread_pdf import _appendix_program_columns

        programs = [
            {"label": "Param Program"},
            {"label": "Coeff Program"},
            {"label": "Solve Score Program"},
            {"label": "Root Program"},
        ]

        left, right = _appendix_program_columns(programs)

        self.assertEqual([p["label"] for p in left], [
            "Param Program",
            "Coeff Program",
        ])
        self.assertEqual([p["label"] for p in right], ["Root Program", "Solve Score Program"])

    def test_prepare_pdf_image_downsamples_large_input(self):
        from PIL import Image
        from spread_pdf import prepare_pdf_image

        with tempfile.TemporaryDirectory() as td:
            src = os.path.join(td, "src.png")
            out = os.path.join(td, "prepared.png")
            Image.new("RGB", (320, 160), (20, 40, 80)).save(src)

            def fake_run(cmd, **_kwargs):
                if cmd[0] == "/opt/bin/vipsheader":
                    field = cmd[cmd.index("-f") + 1]
                    dims = (320, 160) if cmd[-1] == src else (80, 40)
                    return SimpleNamespace(
                        returncode=0,
                        stdout=f"{dims[0 if field == 'width' else 1]}\n",
                        stderr="",
                    )
                output_arg = cmd[cmd.index("-o") + 1]
                output_path = output_arg.split("[", 1)[0]
                Image.new("RGB", (80, 40), (20, 40, 80)).save(output_path)
                return SimpleNamespace(returncode=0, stdout="", stderr="")

            with patch("spread_pdf._vipsthumbnail_path",
                       return_value="/opt/bin/vipsthumbnail"), \
                    patch("spread_pdf._vipsheader_path",
                          return_value="/opt/bin/vipsheader"), \
                    patch("spread_pdf.subprocess.run", side_effect=fake_run):
                info = prepare_pdf_image(src, out, max_px=80)

            self.assertTrue(os.path.exists(out))
            self.assertEqual(info["source_width"], 320)
            self.assertEqual(info["source_height"], 160)
            self.assertEqual(info["prepared_width"], 80)
            self.assertEqual(info["prepared_height"], 40)
            self.assertTrue(info["resized"])
            self.assertEqual(info["prepared_format"], "png")

    def test_prepare_pdf_image_requires_vipsheader(self):
        from spread_pdf import prepare_pdf_image

        with tempfile.TemporaryDirectory() as td:
            src = os.path.join(td, "src.png")
            out = os.path.join(td, "prepared.png")
            with open(src, "wb") as fh:
                fh.write(b"source")
            with patch("spread_pdf._vipsheader_path", return_value=""):
                with self.assertRaisesRegex(RuntimeError, "vipsheader is required"):
                    prepare_pdf_image(src, out, max_px=32)

    def test_prepare_pdf_image_uses_vipsthumbnail_when_available(self):
        from PIL import Image
        from spread_pdf import prepare_pdf_image

        with tempfile.TemporaryDirectory() as td:
            src = os.path.join(td, "src.png")
            out = os.path.join(td, "prepared.jpg")
            Image.new("RGB", (320, 160), (20, 40, 80)).save(src)

            def fake_run(cmd, **_kwargs):
                if cmd[0] == "/opt/bin/vipsheader":
                    field = cmd[cmd.index("-f") + 1]
                    dims = (320, 160) if cmd[-1] == src else (80, 40)
                    return SimpleNamespace(
                        returncode=0,
                        stdout=f"{dims[0 if field == 'width' else 1]}\n",
                        stderr="",
                    )
                output_arg = cmd[cmd.index("-o") + 1]
                output_path = output_arg.split("[", 1)[0]
                Image.new("RGB", (80, 40), (20, 40, 80)).save(output_path, format="JPEG")
                return SimpleNamespace(returncode=0, stdout="", stderr="")

            with patch("spread_pdf._vipsthumbnail_path", return_value="/opt/bin/vipsthumbnail"), \
                    patch("spread_pdf._vipsheader_path", return_value="/opt/bin/vipsheader"), \
                    patch("spread_pdf.subprocess.run", side_effect=fake_run) as mock_run:
                info = prepare_pdf_image(
                    src, out, max_px=80, quality=92, image_format="jpeg")

            self.assertTrue(mock_run.called)
            self.assertTrue(os.path.exists(out))
            self.assertEqual(info["source_width"], 320)
            self.assertEqual(info["source_height"], 160)
            self.assertEqual(info["prepared_width"], 80)
            self.assertEqual(info["prepared_height"], 40)
            self.assertTrue(info["resized"])
            thumbnail_calls = [call for call in mock_run.call_args_list
                               if call.args[0][0] == "/opt/bin/vipsthumbnail"]
            self.assertEqual(len(thumbnail_calls), 1)
            self.assertIn("Q=92", thumbnail_calls[0].args[0][-1])

    def test_prepare_pdf_image_handles_30k_source_without_pillow(self):
        import spread_pdf

        with tempfile.TemporaryDirectory() as td:
            src = os.path.join(td, "source.png")
            out = os.path.join(td, "prepared.jpg")
            with open(src, "wb") as fh:
                fh.write(b"mock source")

            def fake_run(cmd, **_kwargs):
                if cmd[0] == "/opt/bin/vipsheader":
                    field = cmd[cmd.index("-f") + 1]
                    path = cmd[-1]
                    dimensions = ((30000, 30000) if path == src else (5000, 5000))
                    value = dimensions[0 if field == "width" else 1]
                    return SimpleNamespace(returncode=0, stdout=f"{value}\n", stderr="")
                if cmd[0] == "/opt/bin/vipsthumbnail":
                    output_arg = cmd[cmd.index("-o") + 1]
                    output_path = output_arg.split("[", 1)[0]
                    with open(output_path, "wb") as fh:
                        fh.write(b"mock prepared")
                    return SimpleNamespace(returncode=0, stdout="", stderr="")
                raise AssertionError(f"unexpected command: {cmd}")

            with patch("spread_pdf._vipsthumbnail_path",
                       return_value="/opt/bin/vipsthumbnail"), \
                    patch("spread_pdf._vipsheader_path",
                          return_value="/opt/bin/vipsheader"), \
                    patch("spread_pdf.PILImage.open") as pillow_open, \
                    patch("spread_pdf.subprocess.run", side_effect=fake_run):
                info = spread_pdf.prepare_pdf_image(
                    src, out, max_px=5000, quality=92, image_format="jpeg")

            pillow_open.assert_not_called()
            self.assertEqual(info["source_width"], 30000)
            self.assertEqual(info["source_height"], 30000)
            self.assertEqual(info["prepared_width"], 5000)
            self.assertEqual(info["prepared_height"], 5000)
            self.assertTrue(info["resized"])
            self.assertTrue(os.path.exists(out))

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
            self.assertNotIn("color_demo", stream_text)
            self.assertIn("pal_demo_123", stream_text)
            self.assertNotIn("Source details continue in the appendix.", stream_text)
            self.assertNotIn("program excerpts", stream_text.lower())
            self.assertNotIn("full source on appendix", stream_text)
            self.assertNotIn("Source Appendix 1", stream_text)
            self.assertIn("PARAM PROGRAM", stream_text)
            self.assertIn("UNIQ_PROG_LINE_42", stream_text)

    def test_long_program_flows_across_appendix_spreads_without_truncation(self):
        from PIL import Image
        from spread_pdf import build_color_spread_pdf

        with tempfile.TemporaryDirectory() as td:
            img = os.path.join(td, "image.png")
            pdf = os.path.join(td, "out.pdf")
            Image.new("RGB", (32, 32), (20, 30, 40)).save(img)
            source = "\n".join(
                f"ROW_{i:03d} = scan(23, 0, tos[0] + {i}, prev + tos[k] + {i})"
                for i in range(180)
            )
            report = {
                "title": "compute_long / color_long",
                "compute_id": "compute_long",
                "summary_rows": [("Function", "const")],
                "programs": [{"label": "Coeff Program", "source": source}],
            }

            result = build_color_spread_pdf(
                img, pdf, report["title"], report=report)

            self.assertGreaterEqual(result["page_count"], 3)
            stream_text = _pdf_stream_text(pdf)
            self.assertIn("ROW_000", stream_text)
            self.assertIn("ROW_179", stream_text)
            self.assertNotIn("appendix stopped at global source line guard", stream_text)


if __name__ == "__main__":
    unittest.main()
