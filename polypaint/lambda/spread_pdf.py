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
import shutil
import subprocess
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

PDF_IMAGE_MAX_PX = int(os.getenv("PDF_IMAGE_MAX_PX", "5000") or "5000")
PDF_PALETTE_MAX_PX = int(os.getenv("PDF_PALETTE_MAX_PX", "800") or "800")

PAGE_BG = HexColor("#1a1a2e")
ACCENT = HexColor("#e94560")
TEXT = HexColor("#f2f2f7")
MUTED = HexColor("#9aa0b4")
PANEL_BG = HexColor("#121829")
PANEL_BORDER = HexColor("#2b3a5e")
CODE_TEXT = HexColor("#e6e9f2")
RULE = HexColor("#5f6678")

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
# Keep a small measured-width reserve beyond the nominal right padding. This
# avoids placing the final glyph directly against the panel edge in PDF viewers.
CODE_WRAP_GUTTER = 2 * mm
TITLE_RULE_W = 0.8
HEADER_RULE_W = 0.5
PANEL_BORDER_W = 0.75


def _vipsthumbnail_path():
    for path in ("/opt/bin/vipsthumbnail", shutil.which("vipsthumbnail")):
        if path and os.path.exists(path):
            return path
    return ""


def _vipsheader_path():
    for path in ("/opt/bin/vipsheader", shutil.which("vipsheader")):
        if path and os.path.exists(path):
            return path
    return ""


def _vips_env():
    env = dict(os.environ)
    ld = env.get("LD_LIBRARY_PATH", "")
    if "/opt/lib" not in ld.split(":"):
        env["LD_LIBRARY_PATH"] = "/opt/lib" + (":" + ld if ld else "")
    return env


def _prepare_pdf_image_with_vips(input_path, output_path, *, max_px, fmt, quality):
    vipsthumbnail = _vipsthumbnail_path()
    if not vipsthumbnail:
        raise RuntimeError("vipsthumbnail is required for PDF image preparation")
    options = ["strip"]
    if fmt == "jpeg":
        options.append(f"Q={int(quality)}")
    output_spec = str(output_path) + "[" + ",".join(options) + "]"
    result = subprocess.run(
        [
            vipsthumbnail,
            str(input_path),
            "-s",
            f"{int(max_px)}x{int(max_px)}",
            "-o",
            output_spec,
        ],
        capture_output=True,
        text=True,
        timeout=300,
        env=_vips_env(),
    )
    if result.returncode != 0:
        stderr = (result.stderr or result.stdout or "").strip()
        raise RuntimeError(f"vipsthumbnail failed while preparing PDF image: {stderr or 'unknown error'}")


def _vips_dimensions(path):
    vipsheader = _vipsheader_path()
    if not vipsheader:
        raise RuntimeError("vipsheader is required for libvips-only PDF image preparation")
    dims = []
    for field in ("width", "height"):
        result = subprocess.run(
            [vipsheader, "-f", field, str(path)],
            capture_output=True,
            text=True,
            timeout=30,
            env=_vips_env(),
        )
        if result.returncode != 0:
            stderr = (result.stderr or result.stdout or "").strip()
            raise RuntimeError(
                f"vipsheader failed reading {field} from {path}: {stderr or 'unknown error'}")
        try:
            value = int((result.stdout or "").strip())
        except (TypeError, ValueError) as exc:
            raise RuntimeError(
                f"vipsheader returned an invalid {field} for {path}: {result.stdout!r}") from exc
        if value <= 0:
            raise RuntimeError(f"vipsheader returned non-positive {field}={value} for {path}")
        dims.append(value)
    return tuple(dims)


def _prepare_pdf_image_vips(input_path, output_path, *, max_px, fmt, quality):
    source_w, source_h = _vips_dimensions(input_path)
    _prepare_pdf_image_with_vips(
        input_path, output_path, max_px=max_px, fmt=fmt, quality=quality)
    prepared_w, prepared_h = _vips_dimensions(output_path)
    if max(prepared_w, prepared_h) > max_px:
        raise RuntimeError(
            f"vipsthumbnail output exceeds max_px={max_px}: {prepared_w}x{prepared_h}")
    return {
        "source_width": int(source_w),
        "source_height": int(source_h),
        "prepared_width": int(prepared_w),
        "prepared_height": int(prepared_h),
        "resized": max(source_w, source_h) > max_px,
        "image_max_px": int(max_px),
        "prepared_path": str(output_path),
        "prepared_format": fmt,
    }


def prepare_pdf_image(input_path, output_path, *, max_px, quality=90, image_format=None):
    """Prepare a source raster with libvips and return dimension metadata."""
    input_path = Path(input_path)
    output_path = Path(output_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fmt = str(image_format or "").strip().lower()
    if not fmt:
        fmt = "jpeg" if output_path.suffix.lower() in {".jpg", ".jpeg"} else "png"
    if fmt == "jpg":
        fmt = "jpeg"
    if fmt not in {"png", "jpeg"}:
        raise ValueError(f"unsupported PDF image format: {fmt!r}")
    valid_suffixes = {"jpeg": {".jpg", ".jpeg"}, "png": {".png"}}[fmt]
    if output_path.suffix.lower() not in valid_suffixes:
        raise ValueError(
            f"PDF image format {fmt!r} does not match output path {output_path}")
    max_px = int(max_px)
    if max_px <= 0:
        raise ValueError(f"PDF image max_px must be positive, got {max_px}")

    # libvips reads and shrinks demand-wise. Pillow must never decode the full
    # render artifact merely to discover dimensions before this normalization.
    return _prepare_pdf_image_vips(
        input_path, output_path, max_px=max_px, fmt=fmt, quality=quality)


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


def _draw_section_header(c, x, y, title, width=CONTENT_W, *, color=ACCENT, tracking=1.0, uppercase=True):
    text = str(title or "")
    if uppercase:
        text = text.upper()
    _draw_tracking_text(c, x, y, text, F_H, color, tracking)
    c.setStrokeColor(RULE)
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


def _code_string_width(text):
    return pdfmetrics.stringWidth(
        _safe_pdf_text(text), F_CODE[0], F_CODE[1])


def _max_code_prefix(text, max_width):
    """Return the longest non-empty prefix that fits the measured width."""
    if not text:
        return 0
    lo = 1
    hi = len(text)
    best = 0
    while lo <= hi:
        mid = (lo + hi) // 2
        if _code_string_width(text[:mid]) <= max_width:
            best = mid
            lo = mid + 1
        else:
            hi = mid - 1
    return max(1, best)


def _preferred_program_break(text, hard_limit):
    """Choose a readable, quote-aware break no later than hard_limit."""
    commas = []
    spaces = []
    operators = []
    depth = 0
    quote = ""
    escaped = False
    i = 0
    operator_chars = "+-*/%=<>|&^"
    while i < min(len(text), hard_limit):
        ch = text[i]
        if quote:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                quote = ""
            i += 1
            continue
        if ch in ("'", '"'):
            quote = ch
        elif ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth = max(0, depth - 1)
        elif ch == ",":
            commas.append((i + 1, depth))
        elif ch.isspace():
            spaces.append(i)
        elif ch in operator_chars:
            end = i + 1
            while end < min(len(text), hard_limit) and text[end] in operator_chars:
                end += 1
            operators.append(end)
            i = end - 1
        i += 1

    # A comma at the shallowest available nesting level is normally a function
    # argument boundary. Prefer it even when it produces a short introductory
    # line such as ``poly = scan(23, 0,``.
    useful_commas = [(index, item_depth) for index, item_depth in commas if index >= 8]
    if useful_commas:
        shallowest = min(item_depth for _, item_depth in useful_commas)
        return max(index for index, item_depth in useful_commas if item_depth == shallowest)

    min_soft_break = max(4, int(hard_limit * 0.45))
    useful_spaces = [index for index in spaces if index >= min_soft_break]
    if useful_spaces:
        return max(useful_spaces)
    useful_operators = [index for index in operators if index >= min_soft_break]
    if useful_operators:
        return max(useful_operators)
    return hard_limit


def _wrap_program_line(line, max_width):
    """Wrap one source line using measured width and source-aware breakpoints."""
    raw = _safe_pdf_text(line).expandtabs(4)
    if raw == "":
        return [""]
    max_width = max(float(max_width), _code_string_width("W"))
    out = []
    rest = raw
    continuation = ""
    while rest:
        available = max_width - _code_string_width(continuation)
        if available < _code_string_width("W"):
            continuation = ""
            available = max_width
        if _code_string_width(rest) <= available:
            out.append(continuation + rest)
            break
        hard_limit = _max_code_prefix(rest, available)
        split_at = _preferred_program_break(rest, hard_limit)
        split_at = max(1, min(int(split_at), len(rest)))
        piece = rest[:split_at].rstrip()
        if not piece:
            piece = rest[:hard_limit]
            split_at = hard_limit
        out.append(continuation + piece)
        rest = rest[split_at:].lstrip()
        continuation = "  "
    return out or [""]


def _wrap_code_lines(lines, width):
    max_width = max(
        _code_string_width("W"),
        float(width) - 2 * CODE_PAD_X - CODE_WRAP_GUTTER,
    )
    wrapped = []
    for line in lines:
        wrapped.extend(_wrap_program_line(line, max_width))
    return wrapped or [""]


def _split_program_statement_line(line):
    parts = []
    start = 0
    depth = 0
    quote = ""
    escaped = False
    text = _safe_pdf_text(line)
    for i, ch in enumerate(text):
        if quote:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                quote = ""
            continue
        if ch in ("'", '"'):
            quote = ch
        elif ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth = max(0, depth - 1)
        elif ch == ";" and depth == 0:
            part = text[start:i].strip()
            if part:
                parts.append(part)
            start = i + 1
    tail = text[start:].strip()
    if tail or not parts:
        parts.append(tail)
    return parts


def _program_source_display_lines(source):
    lines = []
    for line in str(source or "").splitlines() or [""]:
        lines.extend(_split_program_statement_line(line))
    return lines or [""]


def _draw_code_block(c, x, y, width, lines, *, max_lines=None, caption=""):
    wrapped = _wrap_code_lines(lines, width)
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


def _appendix_program_columns(programs):
    left_order = {
        "param program": 0,
        "coefficient function": 1,
        "coeff program": 2,
    }
    right_order = {
        "root program": 0,
        "solve score program": 1,
    }
    left = []
    right = []
    for idx, program in enumerate(programs or []):
        label = str(program.get("label") or "").strip().lower()
        if label in right_order:
            right.append((right_order[label], idx, program))
        else:
            left.append((left_order.get(label, 100), idx, program))
    left.sort(key=lambda item: (item[0], item[1]))
    right.sort(key=lambda item: (item[0], item[1]))
    return [item[2] for item in left], [item[2] for item in right]


def _appendix_start_y():
    return BLEED_3 + CONTENT_NET - MARGIN_TOP - 28


def _wrapped_program_lines(program, width=CONTENT_W):
    return _wrap_code_lines(
        _program_source_display_lines(program.get("source") or ""), width)


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
    y -= 12
    c.setStrokeColor(RULE)
    c.setLineWidth(TITLE_RULE_W)
    c.line(x, y, x + CONTENT_W, y)
    y -= 9 * mm

    y = _draw_kv_grid(c, x, y, report.get("summary_rows") or [])
    y -= 8 * mm

    programs = _programs_for_report(report)
    if palette_reader and palette_size:
        palette_label = _ellipsize(report.get("palette_label") or "palette", 72)
        y = _draw_section_header(c, x, y, palette_label, color=TEXT, tracking=0.0, uppercase=False)

        bottom = BLEED_3 + MARGIN_BOTTOM
        available_h = max(32 * mm, y - bottom)
        palette_side = max(36 * mm, min(CONTENT_W * 0.72, available_h))
        palette_x = x + (CONTENT_W - palette_side) / 2
        palette_y = bottom + max(0, available_h - palette_side) / 2
        c.setFillColor(PANEL_BG)
        c.setStrokeColor(HexColor("#222a42"))
        c.setLineWidth(0.35)
        c.roundRect(
            palette_x - 3 * mm,
            palette_y - 3 * mm,
            palette_side + 6 * mm,
            palette_side + 6 * mm,
            2 * mm,
            fill=1,
            stroke=1,
        )
        _draw_image_contain(c, palette_reader, palette_size, palette_x, palette_y, palette_side, palette_side)
    elif programs:
        c.setFillColor(MUTED)
        _set_pdf_font(c, F_CAP)
        c.drawString(x, BLEED_3 + MARGIN_BOTTOM, "Source details continue in the appendix.")
    c.restoreState()


def _start_appendix_page(c, page_no):
    c.setFillColor(PAGE_BG)
    c.rect(0, 0, SPREAD_W, SPREAD_H, fill=1, stroke=0)
    y = BLEED_3 + CONTENT_NET - MARGIN_TOP
    return [
        [ORIGIN_X, y - 14, CONTENT_W],
        [PAGE_W + MARGIN_L, y - 14, CONTENT_W],
    ]


def _draw_appendix_pages(c, programs):
    if not programs:
        return 0
    page_count = 1
    columns = _start_appendix_page(c, page_count)
    bottom = BLEED_3 + MARGIN_BOTTOM
    total_lines = 0
    left_programs, right_programs = _appendix_program_columns(programs)

    def draw_programs_in_flow(flow_programs, start_col=0, reserve_first_page_right=False):
        nonlocal page_count, columns, total_lines
        col = start_col
        x, y, w = columns[col]

        def next_column():
            nonlocal page_count, columns, col, x, y, w
            if col == 0 and not (reserve_first_page_right and page_count == 1):
                col = 1
                x, y, w = columns[col]
                return
            c.showPage()
            page_count += 1
            columns = _start_appendix_page(c, page_count)
            col = 0
            x, y, w = columns[col]

        for program in flow_programs:
            label = str(program.get("label") or "Program")
            wrapped = _wrapped_program_lines(program, w)
            idx = 0
            first = True
            while idx < len(wrapped):
                if total_lines >= CODE_MAX_LINES_TOTAL:
                    c.setFillColor(MUTED)
                    _set_pdf_font(c, F_CAP)
                    c.drawString(x, max(bottom, y), "... appendix stopped at global source line guard")
                    return False
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
        return True

    # Put solve-score output on the right side of the first appendix spread.
    # The remaining provenance is read top-to-bottom on the left, with the
    # coefficient program after the coefficient function.
    if right_programs:
        if not draw_programs_in_flow(right_programs, start_col=1):
            return page_count
    if left_programs:
        if not draw_programs_in_flow(left_programs, start_col=0, reserve_first_page_right=bool(right_programs)):
            return page_count
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
