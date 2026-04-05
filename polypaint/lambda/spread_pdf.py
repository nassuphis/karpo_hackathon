"""
Shared PDF spread builder used by the local CLI helper and the PDF artifact Lambda.

The visual treatment intentionally matches make_book.py:
- same content-page gross dimensions
- same centered text page
- same cover-fit image placement
"""
from pathlib import Path

try:
    from PIL import Image as PILImage
    from reportlab.lib.colors import black
    from reportlab.lib.units import mm
    from reportlab.lib.utils import ImageReader
    from reportlab.pdfbase import pdfmetrics
    from reportlab.pdfbase.ttfonts import TTFont
    from reportlab.pdfgen import canvas
except ModuleNotFoundError as exc:
    raise SystemExit(
        "Missing PDF/image dependency. Install reportlab and Pillow in the active environment."
    ) from exc


_UNICODE_FONT_PATHS = [
    "/Library/Fonts/Arial Unicode.ttf",
    "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
]
BODY_FONT = "Helvetica"
for _p in _UNICODE_FONT_PATHS:
    if Path(_p).exists():
        pdfmetrics.registerFont(TTFont("ArialUnicode", _p))
        BODY_FONT = "ArialUnicode"
        break


CONTENT_NET = 290 * mm
BLEED_3 = 3 * mm
CONTENT_GROSS_W = CONTENT_NET + BLEED_3
CONTENT_GROSS_H = CONTENT_NET + 2 * BLEED_3
PAGE_W = CONTENT_GROSS_W
PAGE_H = CONTENT_GROSS_H
SPREAD_W = PAGE_W * 2
SPREAD_H = PAGE_H


def _load_image_rgb(image_path):
    img = PILImage.open(image_path).convert("RGB")
    reader = ImageReader(img)
    return reader, img.size


def _draw_image_cover(c, reader, img_size, x, y, w, h):
    img_w, img_h = img_size
    scale = max(w / img_w, h / img_h)
    draw_w = img_w * scale
    draw_h = img_h * scale
    dx = x + (w - draw_w) / 2
    dy = y + (h - draw_h) / 2
    c.drawImage(reader, dx, dy, width=draw_w, height=draw_h,
                preserveAspectRatio=False, mask='auto')


def _draw_text_page(c, title, body, is_right, filename=None, meta=None):
    """Draw the text page of a spread.

    If `meta` is provided, it overrides `body` with structured render metadata:
      meta.pipeline    — e.g. "[unit_circle,coeff7] poly_155 [rev] N=500, times=1"
      meta.viewport    — e.g. "LL: -1.17, -1.16  UR: 1.14, 1.16"
      meta.color_mode  — e.g. "RAINBOW", "SOLVE SCORE: crowding q=5.0% w=1 reef"
      meta.degree      — e.g. "Degree: 70"
      meta.artifact_id — e.g. "color_run_1775151791677_vni4ec"
    """
    if is_right:
        trim_x = 0
    else:
        trim_x = BLEED_3
    trim_y = BLEED_3

    text_margin = 40 * mm
    tx = trim_x + text_margin
    tw = CONTENT_NET - 2 * text_margin
    center_x = tx + tw / 2
    center_y = trim_y + CONTENT_NET / 2 + 15 * mm

    c.setFillColorRGB(1, 1, 1)

    if title:
        c.setFont("Helvetica-Bold", 28)
        c.drawCentredString(center_x, center_y + 40, title)

    if meta:
        # Structured metadata lines, centered, white.
        structured_lines = meta.get("lines") if isinstance(meta, dict) else None
        y = center_y - 20
        desc_lines = []
        if structured_lines:
            desc_lines = [str(ln) for ln in structured_lines if str(ln).strip()]
        else:
            if meta.get("pipeline"):
                desc_lines.append(meta["pipeline"])
            if meta.get("viewport"):
                desc_lines.append(meta["viewport"])
            if meta.get("color_mode"):
                desc_lines.append(meta["color_mode"])
            if meta.get("degree"):
                desc_lines.append(meta["degree"])

        c.setFont(BODY_FONT, 11)
        for ln in desc_lines:
            c.drawCentredString(center_x, y, ln)
            y -= 18

        # Artifact ID at the bottom in grey
        artifact_id = meta.get("artifact_id", filename or "")
        if artifact_id:
            c.setFont("Courier", 8)
            c.setFillColorRGB(0.5, 0.5, 0.5)
            c.drawCentredString(center_x, y - 20, artifact_id)
    else:
        y = center_y - 20
        if body:
            c.setFont(BODY_FONT, 11)
            words = body.split()
            lines = []
            line = ""
            for word in words:
                test = f"{line} {word}".strip()
                if c.stringWidth(test, BODY_FONT, 11) <= tw:
                    line = test
                else:
                    if line:
                        lines.append(line)
                    line = word
            if line:
                lines.append(line)

            for ln in lines:
                c.drawCentredString(center_x, y, ln)
                y -= 16

        if filename:
            c.setFont("Courier", 8)
            c.setFillColorRGB(0.5, 0.5, 0.5)
            c.drawCentredString(center_x, y - 16, filename)


def build_color_spread_pdf(image_path, output_pdf_path, title, body=None, filename=None, meta=None):
    """Build a two-page spread PDF: text page (left) + image page (right).

    If `meta` is provided (dict with pipeline, viewport, color_mode, degree, artifact_id),
    the text page shows structured render metadata instead of freeform body text.
    """
    image_path = Path(image_path)
    output_pdf_path = Path(output_pdf_path)
    output_pdf_path.parent.mkdir(parents=True, exist_ok=True)

    reader, img_size = _load_image_rgb(image_path)

    c = canvas.Canvas(str(output_pdf_path), pagesize=(SPREAD_W, SPREAD_H))
    c.setTitle(title or image_path.stem)
    c.setAuthor("spread_pdf.py")

    c.saveState()
    c.setFillColor(black)
    c.rect(0, 0, PAGE_W, PAGE_H, fill=1, stroke=0)
    _draw_text_page(c, title or "", body or "", is_right=False, filename=filename, meta=meta)
    c.restoreState()

    c.saveState()
    c.translate(PAGE_W, 0)
    c.setFillColor(black)
    c.rect(0, 0, PAGE_W, PAGE_H, fill=1, stroke=0)
    _draw_image_cover(c, reader, img_size, 0, 0, PAGE_W, PAGE_H)
    c.restoreState()

    c.showPage()
    c.save()
    return output_pdf_path
