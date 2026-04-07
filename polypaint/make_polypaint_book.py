#!/usr/bin/env python3
"""
Generate WhiteWall-compatible coffee table book PDFs.

Produces two PDF files conforming to WhiteWall's requirements:
  1. Cover PDF  — single double-page spread (back + spine + front)
  2. Content PDF — continuous single pages, bleed on 3 sides only

Dimensions extracted from WhiteWall IDML templates:
  A3 square, Fuji Crystal semi-matte, 28 pages.

WhiteWall PDF requirements:
  - PDF version 1.4+
  - sRGB color space
  - 180-300 PPI
  - Fonts embedded
  - No crop marks, no form fields, no encryption
  - Cover max 250 MB, content max 1 GB
  - Page count in multiples of 4

Usage:
  python make_book.py --init                     # Create template config
  python make_book.py book_config.json           # Generate PDFs
  python make_book.py book_config.json -o mybook # Custom output prefix
"""

import argparse
import json
import os
import sys
from pathlib import Path

from reportlab.lib.units import mm
from reportlab.pdfgen import canvas
from reportlab.lib.utils import ImageReader
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from PIL import Image as PILImage

# Register a Unicode-capable font for body text (supports subscript digits etc.)
_UNICODE_FONT_PATHS = [
    "/Library/Fonts/Arial Unicode.ttf",
    "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
]
BODY_FONT = "Helvetica"  # fallback
for _p in _UNICODE_FONT_PATHS:
    if os.path.exists(_p):
        pdfmetrics.registerFont(TTFont("ArialUnicode", _p))
        BODY_FONT = "ArialUnicode"
        break

# ── Dimensions from WhiteWall IDML templates ──────────────────────────
# Source: cover_A3square_paper-fujiCrystal-semi-matte_28.idml
#         block_A3square_paper-fujiCrystal-semi-matte_28.idml

# Content pages (square)
CONTENT_NET = 290 * mm          # 290 x 290 mm net page size
BLEED_3 = 3 * mm               # 3 mm bleed on top, bottom, outer edge
# Binding (inside) edge: 0 mm bleed

# Cover spread
COVER_NET_W = 609 * mm           # back panel + spine + front panel
COVER_NET_H = 296 * mm          # content height + 2 x 3 mm board wrap
COVER_BLEED = 10 * mm           # 10 mm bleed all four sides
SPINE_W = 11 * mm               # spine for 28 pp Fuji Crystal semi-matte
SAFETY = 5 * mm                 # safety margin inside trim edges

# Derived
PANEL_W = (COVER_NET_W - SPINE_W) / 2          # ~296 mm per cover panel
CONTENT_GROSS_W = CONTENT_NET + BLEED_3         # 293 mm
CONTENT_GROSS_H = CONTENT_NET + 2 * BLEED_3    # 296 mm
COVER_GROSS_W = COVER_NET_W + 2 * COVER_BLEED  # 629.4 mm
COVER_GROSS_H = COVER_NET_H + 2 * COVER_BLEED  # 316 mm


# ── Helpers ───────────────────────────────────────────────────────────

def _load_image_rgb(image_path):
    """Load image, convert to RGB, return (ImageReader, (w, h) in pixels)."""
    img = PILImage.open(image_path).convert("RGB")
    reader = ImageReader(img)
    return reader, img.size


def _draw_image_cover(c, reader, img_size, x, y, w, h):
    """Draw image to fill (cover) the rectangle, cropping excess."""
    img_w, img_h = img_size
    scale = max(w / img_w, h / img_h)
    draw_w = img_w * scale
    draw_h = img_h * scale
    draw_x = x + (w - draw_w) / 2
    draw_y = y + (h - draw_h) / 2

    c.saveState()
    p = c.beginPath()
    p.rect(x, y, w, h)
    c.clipPath(p, stroke=0)
    c.drawImage(reader, draw_x, draw_y, draw_w, draw_h)
    c.restoreState()


def _draw_outlined_text(c, text, x, y, font, size,
                        fill=(1, 1, 1), shadow=(0, 0, 0), spacing=0):
    """Draw centered text with a dark outline for readability over images.

    If spacing > 0, characters are drawn individually with extra space between.
    """
    c.saveState()
    c.setFont(font, size)

    def _draw_centered(cx, cy, txt):
        if spacing and len(txt) > 1:
            # Manual letter-spacing: measure total width, draw each char
            widths = [c.stringWidth(ch, font, size) for ch in txt]
            total = sum(widths) + spacing * (len(txt) - 1)
            sx = cx - total / 2
            for i, ch in enumerate(txt):
                c.drawString(sx, cy, ch)
                sx += widths[i] + spacing
        else:
            c.drawCentredString(cx, cy, txt)

    c.setFillColorRGB(*shadow)
    for dx, dy in [(-1.5, -1.5), (-1.5, 1.5), (1.5, -1.5), (1.5, 1.5),
                   (-2.5, 0), (2.5, 0), (0, -2.5), (0, 2.5),
                   (-3, -1), (3, -1), (-3, 1), (3, 1),
                   (-1, -3), (1, -3), (-1, 3), (1, 3)]:
        _draw_centered(x + dx, y + dy, text)
    c.setFillColorRGB(*fill)
    _draw_centered(x, y, text)
    c.restoreState()


def _wrap_text(c, text, font, size, max_width):
    """Word-wrap text to fit within max_width, return list of lines."""
    words = text.split()
    lines = []
    line = ""
    for word in words:
        test = f"{line} {word}".strip()
        if c.stringWidth(test, font, size) <= max_width:
            line = test
        else:
            if line:
                lines.append(line)
            line = word
    if line:
        lines.append(line)
    return lines


def _draw_text_page(c, title, body, is_right, filename=None, job_id=None):
    """Draw centered title + body text, white on black.

    Text is placed within the trim area with safety margins.
    is_right: whether this is a recto page (affects trim offset).
    filename: optional artifact id shown below body in Courier.
    job_id: optional compute job id shown alongside filename.
    """
    # Trim area origin (bottom-left of trim box within gross page)
    if is_right:
        trim_x = 0
    else:
        trim_x = BLEED_3
    trim_y = BLEED_3

    # Text area: trim area inset by generous margins
    text_margin = 40 * mm
    tx = trim_x + text_margin
    tw = CONTENT_NET - 2 * text_margin
    center_x = tx + tw / 2
    center_y = trim_y + CONTENT_NET / 2 + 15 * mm  # slightly above vertical center

    c.setFillColorRGB(1, 1, 1)

    if title:
        c.setFont("Helvetica-Bold", 28)
        c.drawCentredString(center_x, center_y + 40, title)

    if body:
        c.setFont(BODY_FONT, 11)
        y = center_y - 20
        # Render each line separately, centered
        for para in body.split("\n"):
            para = para.strip()
            if not para:
                y -= 8
                continue
            # Word-wrap within each line if needed
            wrapped = _wrap_text(c, para, BODY_FONT, 11, tw)
            for ln in wrapped:
                c.drawCentredString(center_x, y, ln)
                y -= 16
            y -= 4  # small gap between metadata lines

    if filename or job_id:
        c.setFont("Courier", 8)
        c.setFillColorRGB(0.5, 0.5, 0.5)
        id_parts = [p for p in [job_id, filename] if p]
        c.drawCentredString(center_x, y - 16, " \u00b7 ".join(id_parts))


# ── Content PDF ───────────────────────────────────────────────────────

def generate_content_pdf(output_path, pages_config):
    """Generate content pages PDF.

    Each page is CONTENT_GROSS_W x CONTENT_GROSS_H (293 x 296 mm).
    Bleed of 3 mm on top, bottom, and outer edge; 0 on binding edge.
    TrimBox alternates for recto (right) / verso (left) pages.
    Page count padded to a multiple of 4.
    """
    c = canvas.Canvas(str(output_path),
                      pagesize=(CONTENT_GROSS_W, CONTENT_GROSS_H))
    c.setTitle("PolyPaint - Content Pages")
    c.setAuthor("make_book.py")

    # Layout: page 1 (recto) is blank black. Then each config entry
    # becomes a visible spread: verso (left) = text, recto (right) = image.
    # When the book is open you see pages 2n, 2n+1 together.
    #
    # Page sequence:
    #   1 (R) = blank
    #   2 (L) = text for entry 0  }  spread visible together
    #   3 (R) = image for entry 0 }
    #   4 (L) = text for entry 1  }  spread visible together
    #   5 (R) = image for entry 1 }
    #   ...
    # Pad to multiple of 4.

    n_content = 1 + len(pages_config) * 2  # 1 blank + 2 per entry
    n_pages = n_content + (4 - n_content % 4) % 4
    n_pages = max(4, n_pages)

    def _emit_page(is_right):
        if is_right:
            c.setTrimBox((
                0, BLEED_3, CONTENT_NET, BLEED_3 + CONTENT_NET,
            ))
        else:
            c.setTrimBox((
                BLEED_3, BLEED_3, BLEED_3 + CONTENT_NET, BLEED_3 + CONTENT_NET,
            ))
        c.setFillColorRGB(0, 0, 0)
        c.rect(0, 0, CONTENT_GROSS_W, CONTENT_GROSS_H, fill=1, stroke=0)

    page_num = 0

    # Page 1: blank recto
    _emit_page(is_right=True)
    c.showPage()
    page_num += 1

    # Each config entry: verso (text) + recto (image)
    for entry in pages_config:
        # Verso (left page): text
        _emit_page(is_right=False)
        snap_name = entry.get("filename") or os.path.splitext(os.path.basename(entry.get("image", "")))[0]
        _draw_text_page(c, entry.get("title", ""),
                        entry.get("text", ""), is_right=False,
                        filename=snap_name, job_id=entry.get("job_id", ""))
        c.showPage()
        page_num += 1

        # Recto (right page): image
        _emit_page(is_right=True)
        img_path = entry.get("image")
        if img_path and os.path.exists(img_path):
            reader, size = _load_image_rgb(img_path)
            _draw_image_cover(c, reader, size,
                              0, 0, CONTENT_GROSS_W, CONTENT_GROSS_H)
        c.showPage()
        page_num += 1

    # Pad remaining pages to reach n_pages
    while page_num < n_pages:
        is_right = (page_num % 2 == 0)
        _emit_page(is_right)
        c.showPage()
        page_num += 1

    c.save()
    print(f"  Content PDF: {output_path}  ({n_pages} pages, "
          f"{CONTENT_GROSS_W/mm:.0f} x {CONTENT_GROSS_H/mm:.0f} mm gross)")


# ── Cover PDF ─────────────────────────────────────────────────────────

def generate_cover_pdf(output_path, title="Polynomiography", subtitle="",
                       front_image=None, back_image=None,
                       description="", author=""):
    """Generate cover PDF as a single double-page spread.

    Layout within the net area (left to right):
      Back panel (PANEL_W) | Spine (SPINE_W) | Front panel (PANEL_W)
    """
    c = canvas.Canvas(str(output_path),
                      pagesize=(COVER_GROSS_W, COVER_GROSS_H))
    c.setTitle(f"{title} - Cover")
    c.setAuthor("make_book.py")

    # TrimBox
    c.setTrimBox((
        COVER_BLEED,
        COVER_BLEED,
        COVER_BLEED + COVER_NET_W,
        COVER_BLEED + COVER_NET_H,
    ))

    # Black background (full gross area including bleed)
    c.setFillColorRGB(0, 0, 0)
    c.rect(0, 0, COVER_GROSS_W, COVER_GROSS_H, fill=1, stroke=0)

    # ── Front cover (right panel) ──
    front_left = COVER_BLEED + PANEL_W + SPINE_W
    front_cx = front_left + PANEL_W / 2
    front_cy = COVER_GROSS_H / 2

    if front_image and os.path.exists(front_image):
        reader, size = _load_image_rgb(front_image)
        # Image at 2/3 of panel size, centered horizontally, in upper portion
        img_scale = 2.0 / 3.0
        img_w = PANEL_W * img_scale
        img_h = COVER_NET_H * img_scale
        img_x = front_left + (PANEL_W - img_w) / 2
        gap_below_img = 25 * mm
        title_block_h = 36 + (20 if subtitle else 0)  # approximate title + subtitle height
        # Vertically center: image + gap + title block
        total_block = img_h + gap_below_img + title_block_h
        block_top = COVER_BLEED + (COVER_NET_H + total_block) / 2
        img_y = block_top - img_h
        _draw_image_cover(c, reader, size, img_x, img_y, img_w, img_h)

        # Title below the image, no text on the image
        title_y = img_y - gap_below_img
        c.setFillColorRGB(1, 1, 1)
        c.setFont("Helvetica-Bold", 36)
        c.drawCentredString(front_cx, title_y, title.upper())
        if subtitle:
            c.setFont("Helvetica", 13)
            c.drawCentredString(front_cx, title_y - 24, subtitle)
    else:
        # Text-only front cover
        c.setFillColorRGB(1, 1, 1)
        c.setFont("Helvetica-Bold", 48)
        c.drawCentredString(front_cx, front_cy + 30, title)
        if subtitle:
            c.setFont("Helvetica", 24)
            c.drawCentredString(front_cx, front_cy - 30, subtitle)

    # ── Spine text (rotated 90 CCW, read bottom-to-top) ──
    spine_cx = COVER_BLEED + PANEL_W + SPINE_W / 2
    spine_cy = COVER_GROSS_H / 2
    c.saveState()
    c.translate(spine_cx, spine_cy)
    c.rotate(90)
    c.setFillColorRGB(1, 1, 1)
    c.setFont("Helvetica", 7)
    c.drawCentredString(0, -2.5, title)
    c.restoreState()

    # ── Back cover (left panel) ──
    # Vertically center the block: image + gap + description + gap + title
    back_cx = COVER_BLEED + PANEL_W / 2
    back_cy = COVER_BLEED + COVER_NET_H / 2  # vertical center of trim

    img_dim = 85 * mm
    gap_img_text = 15 * mm
    gap_text_title = 20 * mm
    text_leading = 13.5
    text_margin = 35 * mm
    tw = PANEL_W - 2 * text_margin

    # Pre-compute description lines to know total block height
    c.setFont("Helvetica", 9)
    desc_lines = _wrap_text(c, description, "Helvetica", 9, tw) if description else []
    desc_h = len(desc_lines) * text_leading if desc_lines else 0

    # Total block height
    block_h = 0
    if back_image and os.path.exists(back_image):
        block_h += img_dim + gap_img_text
    block_h += desc_h
    if author:
        block_h += gap_text_title + 12  # 12pt for author line

    # Start drawing from top of centered block
    block_top = back_cy + block_h / 2
    y = block_top

    # Small centered image on back cover
    if back_image and os.path.exists(back_image):
        img_x = back_cx - img_dim / 2
        img_y = y - img_dim
        reader, size = _load_image_rgb(back_image)
        _draw_image_cover(c, reader, size,
                          img_x, img_y, img_dim, img_dim)
        y = img_y - gap_img_text

    # Description text
    if desc_lines:
        c.setFillColorRGB(0.6, 0.6, 0.6)
        c.setFont("Helvetica", 9)
        for ln in desc_lines:
            c.drawCentredString(back_cx, y, ln)
            y -= text_leading

    # Author name
    if author:
        c.setFillColorRGB(1, 1, 1)
        c.setFont("Helvetica", 10)
        c.drawCentredString(back_cx, y - gap_text_title, author)

    # Title at bottom of back cover
    c.setFillColorRGB(0.45, 0.45, 0.45)
    c.setFont("Helvetica", 8)
    c.drawCentredString(back_cx, COVER_BLEED + 15 * mm, title)

    c.showPage()
    c.save()
    print(f"  Cover PDF:   {output_path}  "
          f"({COVER_GROSS_W/mm:.0f} x {COVER_GROSS_H/mm:.0f} mm gross, "
          f"spine {SPINE_W/mm:.0f} mm)")


# ── Auto-discovery ────────────────────────────────────────────────────

def _discover_snaps(snaps_dir):
    """Find image+meta pairs in snaps_dir. Returns list of (image_path, meta_dict)."""
    snaps_dir = Path(snaps_dir)
    pairs = []
    for img in sorted(snaps_dir.glob("*.jpeg")):
        meta_path = img.with_name(img.stem + "_meta.json")
        if meta_path.exists():
            with open(meta_path) as f:
                meta = json.load(f)
            pairs.append((str(img), meta))
        else:
            # Image without meta — use filename as fallback
            pairs.append((str(img), {}))
    return pairs


def _solver_tag(solver):
    if solver == "companion_matrix":
        return "Companion Matrix"
    if solver == "aberth_mt":
        return "Aberth-Ehrlich (parallel)"
    if solver in ("aberth", ""):
        return "Aberth-Ehrlich"
    return solver


def _color_description(meta):
    color_mode = str(meta.get("color_mode", "") or "").strip()
    palette = str(meta.get("palette", "") or "").replace("tri_", "").replace("long_", "").replace("_", " ")
    metric = str(meta.get("solve_metric", "") or "").replace("_", " ")
    if color_mode == "solve_score":
        return f"Colored by {metric} score, {palette} palette"
    if color_mode == "proximity":
        return f"Root proximity coloring, {palette} palette"
    if color_mode == "constant":
        return "Uniform coloring"
    return "Index rainbow coloring"


def _title_from_meta(meta):
    """Generate an artsy title from the polynomial function and parameters."""
    compute = meta.get("compute", {})
    fn = compute.get("function", "")
    if not fn:
        return "Title"
    # Strip poly_ prefix for display
    fn_short = fn.replace("poly_", "").replace("giga_", "g")
    return f"Study {fn_short}"


def _text_from_meta(meta):
    """Build artsy text page content from artifact _meta.json."""
    compute = meta.get("compute", {})
    fn = compute.get("function", "")
    pt = compute.get("param_transforms", "none")
    ct = compute.get("coeff_transforms", "none")
    cfpv = compute.get("cfpv", "")
    degree = compute.get("degree", meta.get("degree", ""))
    n_val = compute.get("N", "")
    times = compute.get("times", 1)
    solver = compute.get("solver", "")
    total_roots = compute.get("total_roots", "")

    lines = []

    # Line 1: the polynomial identity
    fn_str = f"{fn}({cfpv})" if cfpv else fn
    if fn_str:
        lines.append(fn_str)

    # Line 2: parameter space
    if pt and pt != "none":
        lines.append(f"Parameters swept through {pt}")
    if ct and ct != "none":
        lines.append(f"Coefficients transformed by {ct}")

    # Line 3: scale
    scale_parts = []
    if degree:
        scale_parts.append(f"degree {degree}")
    if n_val:
        grid = f"{n_val}\u00d7{n_val}"
        if times and int(times) > 1:
            grid += f"\u00d7{times}"
        scale_parts.append(f"{grid} grid")
    if total_roots:
        try:
            scale_parts.append(f"{int(total_roots):,} roots")
        except (ValueError, TypeError):
            pass
    if scale_parts:
        lines.append(", ".join(scale_parts))

    # Line 4: coloring
    color_desc = _color_description(meta)
    if color_desc:
        lines.append(color_desc)

    # Line 5: solver
    stag = _solver_tag(solver)
    if stag:
        lines.append(f"Solved by {stag}")

    body = "\n".join(lines) if lines else ""
    filename = meta.get("artifact_id", "")
    job_id = meta.get("job_id", "")
    title = _title_from_meta(meta)
    return title, body, filename, job_id


# ── Config & CLI ──────────────────────────────────────────────────────

DEFAULT_CONFIG = {
    "title": "PolyPaint",
    "subtitle": "Polynomial Root Visualizations",
    "description": "",
    "author": "",
    "snaps_dir": "snaps",
    "cover_image": None,
    "back_image": None,
}

CONFIG_PATH = "polypaint_book_config.json"


def init_config(path=CONFIG_PATH, snaps_dir="snaps"):
    """Scan snaps/ for image+meta pairs and write a full config with per-page entries.

    The generated config is the source of truth — edit titles and text before generating.
    Re-running --init overwrites the config, so back it up if you've made edits.
    """
    pairs = _discover_snaps(snaps_dir)

    pages = []
    for img_path, meta in pairs:
        title_text, body, filename, job_id = _text_from_meta(meta)
        pages.append({
            "image": img_path,
            "title": title_text,
            "text": body,
            "filename": filename,
            "job_id": job_id,
        })

    config = dict(DEFAULT_CONFIG)
    config["snaps_dir"] = snaps_dir
    config["pages"] = pages
    if pages:
        config["cover_image"] = pages[0]["image"]

    with open(path, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Created {path} with {len(pages)} page(s) from {snaps_dir}/")
    if pages:
        print("  Edit titles and text in the config, then run without --init to generate PDFs.")


def main():
    parser = argparse.ArgumentParser(
        description="Generate WhiteWall coffee table book PDFs from polypaint render artifacts")
    parser.add_argument("config", nargs="?", default=CONFIG_PATH,
                        help=f"Path to config (default: {CONFIG_PATH})")
    parser.add_argument("-o", "--output-prefix", default="polypaint_book",
                        help="Output file prefix (default: polypaint_book)")
    parser.add_argument("--init", action="store_true",
                        help=f"Scan snaps/ and create/overwrite {CONFIG_PATH}")
    parser.add_argument("--snaps-dir", default="snaps",
                        help="Snaps directory for --init (default: snaps)")
    args = parser.parse_args()

    if args.init:
        init_config(args.config, args.snaps_dir)
        return

    if not os.path.exists(args.config):
        print(f"Config not found: {args.config}")
        print(f"Run: python make_polypaint_book.py --init")
        sys.exit(1)

    with open(args.config) as f:
        config = json.load(f)

    title = config.get("title", "PolyPaint")
    subtitle = config.get("subtitle", "")
    description = config.get("description", "")
    author = config.get("author", "")
    pages = config.get("pages", [])

    if not pages:
        print("No pages in config. Run --init to populate from snaps/")
        sys.exit(1)

    front_image = config.get("cover_image") or (pages[0]["image"] if pages else None)
    back_image = config.get("back_image")

    print(f"Generating WhiteWall PDFs: '{title}'")
    n_content = 1 + len(pages) * 2
    n_padded = n_content + (4 - n_content % 4) % 4
    n_padded = max(4, n_padded)
    print(f"  {len(pages)} spread(s) -> {n_padded} content pages")
    print()

    cover_path = f"{args.output_prefix}_cover.pdf"
    content_path = f"{args.output_prefix}_content.pdf"

    generate_cover_pdf(cover_path, title, subtitle, front_image,
                       back_image, description, author)
    generate_content_pdf(content_path, pages)

    print()
    print("Done! Upload to WhiteWall:")
    print(f"  Cover:   {cover_path}")
    print(f"  Content: {content_path}")


if __name__ == "__main__":
    main()
