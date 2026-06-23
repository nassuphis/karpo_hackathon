"""
Shared PDF spread builder used by the local CLI helper and the PDF artifact Lambda.

The spread is a two-page horizontal layout:
- left page: black background with centered metadata/text
- right page: full-page image with cover-fit placement

For ColorSpread PDFs, the left page can also show an associated palette image.
When present, the palette is rendered as a centered 5 cm square beneath the
metadata lines and above the artifact ID, with a thin white border so the
palette remains legible against the black page.

The visual treatment intentionally matches make_book.py:
- same content-page gross dimensions
- same centered text page
- same cover-fit image placement
"""
import os
from pathlib import Path

try:
    from PIL import Image as PILImage
    from reportlab.lib.colors import HexColor, black
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


def _draw_image_contain(c, reader, img_size, x, y, w, h):
    img_w, img_h = img_size
    scale = min(w / img_w, h / img_h)
    draw_w = img_w * scale
    draw_h = img_h * scale
    dx = x + (w - draw_w) / 2
    dy = y + (h - draw_h) / 2
    c.drawImage(reader, dx, dy, width=draw_w, height=draw_h,
                preserveAspectRatio=False, mask='auto')


def _draw_text_page(c, title, body, is_right, filename=None, meta=None, palette_reader=None, palette_size=None):
    """Draw the text page of a spread.

    If `meta` is provided, it overrides `body` with structured render metadata:
      meta.pipeline    — e.g. "[unit_circle,coeff7] poly_155 [rev] N=500, times=1"
      meta.viewport    — e.g. "LL: -1.17, -1.16  UR: 1.14, 1.16"
      meta.color_mode  — e.g. "RAINBOW", "SOLVE SCORE: crowding q=5.0% w=1 reef"
      meta.degree      — e.g. "Degree: 70"
      meta.artifact_id — e.g. "color_run_1775151791677_vni4ec"

    Layout on the left page is intentionally simple and centered:
    - title near the top of the centered text block
    - structured metadata lines stacked downward with fixed leading
    - optional associated palette shown as a centered 5 cm square below the
      metadata block
    - artifact ID below the palette in small grey type
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

        artifact_y = y - 20
        if palette_reader and palette_size:
            # The associated palette is presented as a standalone square swatch
            # block on the text page, centered horizontally so it reads like a
            # companion object to the metadata rather than a corner thumbnail.
            palette_side = 50 * mm  # 5 cm square
            palette_gap = 10 * mm
            palette_x = center_x - palette_side / 2
            palette_y = max(trim_y + 26 * mm, y - palette_gap - palette_side)
            c.setStrokeColorRGB(1, 1, 1)
            c.setLineWidth(0.5)
            c.rect(palette_x, palette_y, palette_side, palette_side, fill=0, stroke=1)
            _draw_image_contain(c, palette_reader, palette_size, palette_x, palette_y, palette_side, palette_side)
            artifact_y = palette_y - 14

        # Artifact ID at the bottom in grey
        artifact_id = meta.get("artifact_id", filename or "")
        if artifact_id:
            c.setFont("Courier", 8)
            c.setFillColorRGB(0.5, 0.5, 0.5)
            c.drawCentredString(center_x, artifact_y, artifact_id)
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


# ==== CR21 refreshed PDF artifact path =====================================
#
# The public renderer still supports the old report=None call shape for local
# previews, while the Lambda path passes a richer provenance report.

PDF_TARGET_DPI = int(os.getenv("PDF_TARGET_DPI", "300") or "300")
PDF_IMAGE_MAX_PX = int(os.getenv("PDF_IMAGE_MAX_PX", "3600") or "3600")
PDF_PALETTE_MAX_PX = int(os.getenv("PDF_PALETTE_MAX_PX", "800") or "800")
PDF_MAX_SOURCE_PIXELS = int(os.getenv("PDF_MAX_SOURCE_PIXELS", "150000000") or "150000000")
PDF_IMAGE_FORMAT = str(os.getenv("PDF_IMAGE_FORMAT", "png") or "png").lower()

if PILImage.MAX_IMAGE_PIXELS is None or PILImage.MAX_IMAGE_PIXELS < PDF_MAX_SOURCE_PIXELS:
    PILImage.MAX_IMAGE_PIXELS = PDF_MAX_SOURCE_PIXELS

PAGE_BG = HexColor("#1a1a2e")
ACCENT = HexColor("#e94560")
TEXT = HexColor("#f2f2f7")
MUTED = HexColor("#9aa0b4")
PANEL_BG = HexColor("#121829")
PANEL_BORDER = HexColor("#2b3a5e")
CODE_TEXT = HexColor("#e6e9f2")

MARGIN_L = MARGIN_R = 24 * mm
MARGIN_TOP = 26 * mm
MARGIN_BOTTOM = 22 * mm
ORIGIN_X = BLEED_3 + MARGIN_L
CONTENT_W = CONTENT_NET - MARGIN_L - MARGIN_R

F_TITLE = ("Helvetica-Bold", 30)
F_TITLE2 = ("Helvetica-Bold", 19)
F_H = ("Helvetica-Bold", 15)
F_LABEL = ("Helvetica-Bold", 10)
F_VALUE = ("Helvetica", 12)
F_CODE = ("Courier", 10.5)
F_CAP = ("Helvetica-Oblique", 9)

KV_PITCH = 17
CODE_LEADING = 13
CODE_MAX_LINES_TOTAL = 2000
SECTION_GAP = 9 * mm
CODE_PAD_X = 6 * mm
CODE_PAD_Y = 5 * mm
TITLE_RULE_W = 0.8
HEADER_RULE_W = 0.5
PANEL_BORDER_W = 0.75


def _resample_filter():
    resampling = getattr(PILImage, "Resampling", PILImage)
    return getattr(resampling, "LANCZOS", getattr(PILImage, "BICUBIC", 3))


def prepare_pdf_image(input_path, output_path, *, max_px, quality=90, image_format=None, max_source_pixels=PDF_MAX_SOURCE_PIXELS):
    """Prepare a source raster for PDF embedding and return dimension metadata."""
    input_path = Path(input_path)
    output_path = Path(output_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fmt = str(image_format or PDF_IMAGE_FORMAT or "png").strip().lower()
    if fmt == "jpg":
        fmt = "jpeg"
    if fmt not in {"png", "jpeg"}:
        fmt = "png"
    max_px = int(max_px)
    if max_px <= 0:
        raise ValueError(f"PDF image max_px must be positive, got {max_px}")

    try:
        with PILImage.open(input_path) as img:
            source_w, source_h = img.size
            source_pixels = int(source_w) * int(source_h)
            if source_pixels > int(max_source_pixels):
                raise ValueError(
                    f"PDF source image is too large: {source_w}x{source_h} "
                    f"({source_pixels} pixels), limit is {int(max_source_pixels)}"
                )
            resized = max(source_w, source_h) > max_px
            if resized:
                try:
                    img.draft("RGB", (max_px, max_px))
                except Exception:
                    pass
            work = img
            if resized:
                work.thumbnail((max_px, max_px), _resample_filter())
            if work.mode != "RGB":
                work = work.convert("RGB")
            prepared_w, prepared_h = work.size
            if fmt == "jpeg":
                work.save(output_path, format="JPEG", quality=int(quality), optimize=True)
            else:
                work.save(output_path, format="PNG", optimize=True)
    except PILImage.DecompressionBombError as exc:
        raise ValueError(
            "PDF source image exceeds the configured safety limit; "
            f"set PDF_MAX_SOURCE_PIXELS deliberately if this image is expected: {exc}"
        ) from exc

    return {
        "source_width": int(source_w),
        "source_height": int(source_h),
        "prepared_width": int(prepared_w),
        "prepared_height": int(prepared_h),
        "resized": bool(resized),
        "image_max_px": int(max_px),
        "prepared_path": str(output_path),
        "prepared_format": fmt,
    }


def _safe_pdf_text(value):
    text = "" if value is None else str(value)
    return text.encode("latin-1", "replace").decode("latin-1")


def _ellipsize(text, max_chars):
    text = _safe_pdf_text(text)
    if len(text) <= max_chars:
        return text
    return text[:max(1, max_chars - 3)] + "..."


def _set_pdf_font(c, font_spec):
    c.setFont(font_spec[0], font_spec[1])


def _draw_tracking_text(c, x, y, text, font_spec, color, char_space=1.0):
    c.setFillColor(color)
    t = c.beginText(x, y)
    t.setFont(font_spec[0], font_spec[1])
    t.setCharSpace(char_space)
    t.textLine(_safe_pdf_text(text))
    c.drawText(t)


def _draw_section_header(c, x, y, title, width=CONTENT_W):
    _draw_tracking_text(c, x, y, str(title or "").upper(), F_H, ACCENT, 1.0)
    c.setStrokeColor(PANEL_BORDER)
    c.setLineWidth(HEADER_RULE_W)
    c.line(x, y - 5, x + width, y - 5)
    return y - F_H[1] - 4 * mm


def _draw_kv_grid(c, x, y, rows, *, label_w=46 * mm, max_value_chars=90):
    for label, value in rows:
        c.setFillColor(MUTED)
        _set_pdf_font(c, F_LABEL)
        c.drawString(x, y, _ellipsize(str(label).upper(), 18))
        c.setFillColor(TEXT)
        _set_pdf_font(c, F_VALUE)
        c.drawString(x + label_w, y, _ellipsize("" if value is None else value, max_value_chars))
        y -= KV_PITCH
    return y


def _code_chars_for_width(width):
    return max(20, int((float(width) - 2 * CODE_PAD_X) / (0.6 * F_CODE[1])))


def _wrap_monospace_line(line, max_chars):
    max_chars = max(2, int(max_chars))
    raw = _safe_pdf_text(line).expandtabs(4)
    if raw == "":
        return [""]
    out = []
    rest = raw
    prefix = ""
    while len(prefix) + len(rest) > max_chars:
        take = max(1, max_chars - len(prefix))
        out.append(prefix + rest[:take])
        rest = rest[take:]
        prefix = "  " if max_chars > 2 else ""
    out.append(prefix + rest)
    return out


def _draw_code_block(c, x, y, width, lines, *, max_lines=None, caption=""):
    max_chars = _code_chars_for_width(width)
    wrapped = []
    for line in lines:
        wrapped.extend(_wrap_monospace_line(line, max_chars))
    total = len(wrapped)
    shown = wrapped if max_lines is None else wrapped[:max_lines]
    truncated = total > len(shown)
    if not shown:
        shown = [""]
    panel_h = len(shown) * CODE_LEADING + 2 * CODE_PAD_Y
    y0 = y - panel_h

    c.setFillColor(PANEL_BG)
    c.setStrokeColor(PANEL_BORDER)
    c.setLineWidth(PANEL_BORDER_W)
    c.roundRect(x, y0, width, panel_h, 2 * mm, fill=1, stroke=1)

    c.setFillColor(CODE_TEXT)
    _set_pdf_font(c, F_CODE)
    text_y = y - CODE_PAD_Y - F_CODE[1]
    for line in shown:
        c.drawString(x + CODE_PAD_X, text_y, _safe_pdf_text(line))
        text_y -= CODE_LEADING

    y = y0 - 4
    if truncated or caption:
        c.setFillColor(MUTED)
        _set_pdf_font(c, F_CAP)
        cap = caption or f"... truncated after {len(shown)} of {total} wrapped lines"
        c.drawString(x, y - F_CAP[1], _safe_pdf_text(cap))
        y -= F_CAP[1] + 4
    return y


def _programs_for_report(report):
    programs = []
    for item in (report or {}).get("programs", []) or []:
        if not isinstance(item, dict):
            continue
        label = str(item.get("label") or "").strip()
        source = str(item.get("source") or "").strip()
        fallback = str(item.get("fallback") or "").strip()
        if not label:
            continue
        if source:
            programs.append({**item, "source": source, "is_fallback": False})
        elif fallback:
            programs.append({**item, "source": fallback, "is_fallback": True})
    return programs


def _appendix_start_y():
    return BLEED_3 + CONTENT_NET - MARGIN_TOP - 28


def _wrapped_program_lines(program, width=CONTENT_W):
    wrapped = []
    for line in str(program.get("source") or "").splitlines() or [""]:
        wrapped.extend(_wrap_monospace_line(line, _code_chars_for_width(width)))
    return wrapped


def _assign_appendix_pages(report):
    programs = _programs_for_report(report)
    page = 1
    col = 0
    y = _appendix_start_y()
    bottom = BLEED_3 + MARGIN_BOTTOM
    total_lines = 0

    def next_column():
        nonlocal page, col, y
        if col == 0:
            col = 1
            y = _appendix_start_y()
            return
        page += 1
        col = 0
        y = _appendix_start_y()

    for program in programs:
        wrapped = _wrapped_program_lines(program)
        idx = 0
        assigned = False
        while idx < len(wrapped):
            if total_lines >= CODE_MAX_LINES_TOTAL:
                break
            if y < bottom + 90:
                next_column()
            if not assigned:
                program["appendix_page"] = page
                assigned = True
            y_after_header = y - F_H[1] - 4 * mm
            available_lines = max(1, int((y_after_header - bottom - 2 * CODE_PAD_Y - 6) / CODE_LEADING))
            take = min(len(wrapped) - idx, available_lines, CODE_MAX_LINES_TOTAL - total_lines)
            panel_h = take * CODE_LEADING + 2 * CODE_PAD_Y
            y = y_after_header - panel_h - 4 - SECTION_GAP
            idx += take
            total_lines += take
        if not assigned:
            program["appendix_page"] = page
    if isinstance(report, dict):
        report["programs"] = programs
    return programs


def _draw_report_summary(c, report, palette_reader=None, palette_size=None):
    c.saveState()
    c.setFillColor(PAGE_BG)
    c.rect(0, 0, PAGE_W, PAGE_H, fill=1, stroke=0)

    x = ORIGIN_X
    y = BLEED_3 + CONTENT_NET - MARGIN_TOP
    c.setFillColor(TEXT)
    _set_pdf_font(c, F_TITLE)
    c.drawString(x, y, _ellipsize(report.get("compute_id") or report.get("title") or "", 34))
    y -= 34
    c.setFillColor(ACCENT)
    _set_pdf_font(c, F_TITLE2)
    c.drawString(x, y, _ellipsize(report.get("color_artifact_id") or "", 46))
    y -= 10
    c.setStrokeColor(ACCENT)
    c.setLineWidth(TITLE_RULE_W)
    c.line(x, y, x + CONTENT_W, y)
    y -= 9 * mm

    y = _draw_kv_grid(c, x, y, report.get("summary_rows") or [])
    y -= 8 * mm

    programs = _programs_for_report(report)
    if palette_reader and palette_size:
        y = _draw_section_header(c, x, y, "palette")
        palette_label = _ellipsize(report.get("palette_label") or "palette", 72)
        c.setFillColor(TEXT)
        _set_pdf_font(c, ("Helvetica-Bold", 13))
        c.drawCentredString(x + CONTENT_W / 2, y, _safe_pdf_text(palette_label))
        y -= 8 * mm

        bottom = BLEED_3 + MARGIN_BOTTOM + (9 * mm if programs else 0)
        available_h = max(32 * mm, y - bottom)
        palette_side = max(36 * mm, min(CONTENT_W * 0.72, available_h))
        palette_x = x + (CONTENT_W - palette_side) / 2
        palette_y = bottom + max(0, available_h - palette_side) / 2
        c.setFillColor(PANEL_BG)
        c.setStrokeColor(PANEL_BORDER)
        c.setLineWidth(PANEL_BORDER_W)
        c.roundRect(
            palette_x - 4 * mm,
            palette_y - 4 * mm,
            palette_side + 8 * mm,
            palette_side + 8 * mm,
            2 * mm,
            fill=1,
            stroke=1,
        )
        _draw_image_contain(c, palette_reader, palette_size, palette_x, palette_y, palette_side, palette_side)
        if programs:
            c.setFillColor(MUTED)
            _set_pdf_font(c, F_CAP)
            c.drawCentredString(x + CONTENT_W / 2, BLEED_3 + MARGIN_BOTTOM, "Source details continue in the appendix.")
    elif programs:
        c.setFillColor(MUTED)
        _set_pdf_font(c, F_CAP)
        c.drawString(x, BLEED_3 + MARGIN_BOTTOM, "Source details continue in the appendix.")
    c.restoreState()


def _start_appendix_page(c, page_no):
    c.setFillColor(PAGE_BG)
    c.rect(0, 0, SPREAD_W, SPREAD_H, fill=1, stroke=0)
    x = ORIGIN_X
    y = BLEED_3 + CONTENT_NET - MARGIN_TOP
    c.setFillColor(TEXT)
    _set_pdf_font(c, ("Helvetica-Bold", 22))
    c.drawString(x, y, f"Source Appendix {page_no}")
    c.setStrokeColor(ACCENT)
    c.setLineWidth(TITLE_RULE_W)
    c.line(x, y - 8, SPREAD_W - ORIGIN_X, y - 8)
    return [
        [ORIGIN_X, y - 28, CONTENT_W],
        [PAGE_W + MARGIN_L, y - 28, CONTENT_W],
    ]


def _draw_appendix_pages(c, programs):
    if not programs:
        return 0
    page_count = 1
    columns = _start_appendix_page(c, page_count)
    col = 0
    x, y, w = columns[col]
    bottom = BLEED_3 + MARGIN_BOTTOM
    total_lines = 0

    def next_column():
        nonlocal page_count, columns, col, x, y, w
        if col == 0:
            col = 1
            x, y, w = columns[col]
            return
        c.showPage()
        page_count += 1
        columns = _start_appendix_page(c, page_count)
        col = 0
        x, y, w = columns[col]

    for program in programs:
        label = str(program.get("label") or "Program")
        wrapped = _wrapped_program_lines(program, w)
        idx = 0
        first = True
        while idx < len(wrapped):
            if total_lines >= CODE_MAX_LINES_TOTAL:
                c.setFillColor(MUTED)
                _set_pdf_font(c, F_CAP)
                c.drawString(x, max(bottom, y), "... appendix stopped at global source line guard")
                return page_count
            if y < bottom + 90:
                next_column()
            header = label.upper() if first else f"{label.upper()} (CONT.)"
            y = _draw_section_header(c, x, y, header, width=w)
            available_lines = max(1, int((y - bottom - 2 * CODE_PAD_Y - 6) / CODE_LEADING))
            take = min(len(wrapped) - idx, available_lines, CODE_MAX_LINES_TOTAL - total_lines)
            y = _draw_code_block(c, x, y, w, wrapped[idx:idx + take])
            y -= SECTION_GAP
            idx += take
            total_lines += take
            first = False
    return page_count


def build_color_spread_pdf(image_path, output_pdf_path, title, body=None, filename=None, meta=None, palette_image_path=None, report=None):
    """Build a ColorSpread PDF and return {'path': Path, 'page_count': int}."""
    image_path = Path(image_path)
    output_pdf_path = Path(output_pdf_path)
    output_pdf_path.parent.mkdir(parents=True, exist_ok=True)

    reader, img_size = _load_image_rgb(image_path)
    palette_reader = None
    palette_size = None
    if palette_image_path:
        palette_reader, palette_size = _load_image_rgb(Path(palette_image_path))

    programs = _assign_appendix_pages(report) if isinstance(report, dict) else []
    c = canvas.Canvas(str(output_pdf_path), pagesize=(SPREAD_W, SPREAD_H))
    c.setTitle(_safe_pdf_text(title or image_path.stem))
    c.setAuthor("spread_pdf.py")

    if isinstance(report, dict):
        _draw_report_summary(c, report, palette_reader=palette_reader, palette_size=palette_size)
    else:
        c.saveState()
        c.setFillColor(black)
        c.rect(0, 0, PAGE_W, PAGE_H, fill=1, stroke=0)
        _draw_text_page(
            c,
            title or "",
            body or "",
            is_right=False,
            filename=filename,
            meta=meta,
            palette_reader=palette_reader,
            palette_size=palette_size,
        )
        c.restoreState()

    c.saveState()
    c.translate(PAGE_W, 0)
    c.setFillColor(black)
    c.rect(0, 0, PAGE_W, PAGE_H, fill=1, stroke=0)
    _draw_image_cover(c, reader, img_size, 0, 0, PAGE_W, PAGE_H)
    c.restoreState()

    appendix_pages = 0
    if programs:
        c.showPage()
        appendix_pages = _draw_appendix_pages(c, programs)
    c.showPage()
    c.save()
    return {"path": output_pdf_path, "page_count": 1 + appendix_pages}
