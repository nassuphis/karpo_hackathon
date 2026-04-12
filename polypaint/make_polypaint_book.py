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
from collections import namedtuple
from pathlib import Path

from reportlab.lib.units import mm
from reportlab.pdfgen import canvas
from reportlab.lib.utils import ImageReader
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from PIL import Image as PILImage

# ── Font registry ─────────────────────────────────────────────────────
#
# All typefaces used by the book are registered here under a stable "key"
# (lowercase, hyphenated). The key is what the CLI and config reference;
# the registered reportlab name is what `setFont` takes.
#
# Every font is a TrueType file so the final PDF has all glyphs EMBEDDED —
# print shops see the exact outlines we used instead of falling back to
# generic Helvetica. The one exception is the Canvas's default font state
# (reportlab writes it to every page's font resources even if we never draw
# with it); we shadow the base "Helvetica" name with Arial.ttf for that.
_SCRIPT_DIR = Path(__file__).parent

# (key, reportlab-name, [candidate paths in priority order])
_FONT_SPECS = [
    # Category: body/title fallback (needed for per-char font fallback when
    # the descriptive body font doesn't ship a glyph — apostrophes, em dashes,
    # middle dots, etc.). Arial Unicode has wide coverage.
    ("arial-unicode", "ArialUnicode", [
        "/Library/Fonts/Arial Unicode.ttf",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
    ]),
    # Canela family (trial) — display serif for titles
    ("canela-light", "Canela-Light", [
        _SCRIPT_DIR / "fonts" / "Canela-Light-Trial.ttf",
    ]),
    ("canela-regular", "Canela-Regular", [
        _SCRIPT_DIR / "fonts" / "Canela-Regular-Trial.ttf",
    ]),
    # Tiempos Text Regular (trial) — reading serif for descriptive copy
    ("tiempos-regular", "Tiempos-Regular", [
        _SCRIPT_DIR / "fonts" / "TiemposText-Regular-Trial.ttf",
    ]),
    # Tiempos Headline Regular (trial) — display serif sibling of Tiempos Text
    ("tiempos-headline", "TiemposHeadline-Regular", [
        _SCRIPT_DIR / "fonts" / "TiemposHeadline-Regular-Trial.ttf",
    ]),
    # Lyon — warm humanist display serif
    ("lyon", "Lyon-Regular", [
        _SCRIPT_DIR / "fonts" / "Lyon-Regular.ttf",
    ]),
    # "Sabon-similar" candidates — three different Sabon-adjacent serifs
    ("baramond", "Baramond-Regular", [
        _SCRIPT_DIR / "fonts" / "Baramond-Regular.ttf",
    ]),
    ("optisarone", "OPTISarone-Regular", [
        _SCRIPT_DIR / "fonts" / "OPTISarone-Regular.ttf",
    ]),
    ("sibila", "Sibila-Regular", [
        _SCRIPT_DIR / "fonts" / "Sibila-Regular.ttf",
    ]),
    # Sohne (trial) — clean sans for title OR body experimentation
    ("sohne-buch", "Sohne-Buch", [
        _SCRIPT_DIR / "fonts" / "Sohne-Buch-Trial.ttf",
    ]),
    # Monospace choices
    ("courier-prime", "CourierPrime", [
        _SCRIPT_DIR / "fonts" / "CourierPrime-Regular.ttf",
    ]),
    ("sohne-mono", "SohneMono-Buch", [
        _SCRIPT_DIR / "fonts" / "SohneMono-Buch-Trial.ttf",
    ]),
    ("jetbrains-mono", "JetBrainsMono", [
        _SCRIPT_DIR / "fonts" / "JetBrainsMono-Regular.ttf",
    ]),
    ("monaco", "Monaco", [
        "/System/Library/Fonts/Monaco.ttf",
    ]),
    # Helvetica shadow — maps the Canvas default "Helvetica" to an Arial TTF
    # so the leftover default-state font reference is embedded too.
    ("helvetica", "Helvetica", [
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
    ]),
]

FONT_REGISTRY = {}  # key -> registered reportlab name
for _key, _name, _paths in _FONT_SPECS:
    for _p in _paths:
        if Path(_p).exists():
            try:
                pdfmetrics.registerFont(TTFont(_name, str(_p)))
                FONT_REGISTRY[_key] = _name
                break
            except Exception as _e:
                print(f"WARNING: failed to register {_key} from {_p}: {_e}")
if "helvetica" not in FONT_REGISTRY:
    print("WARNING: no Arial TTF found to shadow base Helvetica; the PDF "
          "may contain an un-embedded Helvetica reference in page resources.")

# Defaults for the three font categories (title, body, mono). Changeable
# per run via CLI flags.
DEFAULT_TITLE_FONT_KEY = "canela-regular"
DEFAULT_BODY_FONT_KEY = "tiempos-regular"
DEFAULT_MONO_FONT_KEY = "courier-prime"
DEFAULT_BODY_FALLBACK_KEY = "arial-unicode"  # never user-selected

# Module-level convenience names kept for the rest of the file. These are
# the RESOLVED defaults — individual calls can override by passing a
# different FontSet.
BODY_FONT = FONT_REGISTRY.get(DEFAULT_BODY_FALLBACK_KEY) or "Helvetica"
CONTENT_BODY_FONT = FONT_REGISTRY.get(DEFAULT_BODY_FONT_KEY) or BODY_FONT
MONO_FONT = FONT_REGISTRY.get(DEFAULT_MONO_FONT_KEY) or "Courier"
COVER_DISPLAY_FONT = FONT_REGISTRY.get("canela-light") or "Helvetica"
COVER_DISPLAY_FONT_REGULAR = FONT_REGISTRY.get("canela-regular") or "Helvetica"

# Legacy compatibility: some older call sites still test whether these paths
# exist to decide between Canela and Helvetica-Bold. Keep the path variables
# so those branches still work unchanged.
_CANELA_LIGHT_PATH = _SCRIPT_DIR / "fonts" / "Canela-Light-Trial.ttf"
_CANELA_REGULAR_PATH = _SCRIPT_DIR / "fonts" / "Canela-Regular-Trial.ttf"


# ── Resolved font / gap sets ─────────────────────────────────────────

FontSet = namedtuple("FontSet", ["title", "body", "body_fallback",
                                 "mono", "mono_fallback"])
# All five members are RESOLVED reportlab font names (strings you can pass
# to setFont), not registry keys. The *_fallback slots are used by
# `_draw_mixed_centered` when the primary font is missing a glyph — picked
# to be fonts with wide ASCII + Latin-1 coverage so experimental limited
# trial fonts still render all their content legibly.

SizeSet = namedtuple("SizeSet", ["title", "body", "mono"])
# Point sizes (float). `mono` is the pipeline-line size; the artifact-id line
# renders at `mono - 1` so the credit line stays quietly smaller.

DEFAULT_SIZES = SizeSet(title=33.6, body=10.5, mono=9.0)

ColorSet = namedtuple("ColorSet", ["text", "mono"])
# Two RGB float triples in [0, 1]:
#   text  — title and body color (also the metadata "choice" line)
#   mono  — pipeline + artifact-id color (single gray, not two-tone)

DEFAULT_COLORS = ColorSet(
    text=(0xF2 / 255, 0xEE / 255, 0xE6 / 255),   # #F2EEE6 warm off-white
    mono=(0x80 / 255, 0x80 / 255, 0x80 / 255),   # #808080 medium gray
)

GapSet = namedtuple("GapSet", ["title_pipeline", "pipeline_body",
                               "body_meta", "meta_palette"])
# All four members are INTEGER LINE UNITS, where 1 line unit == body_leading
# (currently 16pt). "Line unit" is an abstract vertical rhythm unit independent
# of the actual font sizes.

DEFAULT_GAPS = GapSet(title_pipeline=3, pipeline_body=3, body_meta=3, meta_palette=3)


def parse_hex_color(s):
    """Parse a CSS-style hex color `#RRGGBB` or `RRGGBB` into a 3-tuple of
    floats in [0, 1]. Raises ValueError on malformed input."""
    if s is None:
        return None
    s = s.strip().lstrip("#")
    if len(s) != 6:
        raise ValueError(f"hex color must be 6 hex digits, got {s!r}")
    r = int(s[0:2], 16) / 255
    g = int(s[2:4], 16) / 255
    b = int(s[4:6], 16) / 255
    return (r, g, b)


def resolve_fonts(title_key=None, body_key=None, mono_key=None):
    """Map registry keys to reportlab font names, with defaults + fallbacks.

    Unknown keys are reported with a loud WARNING (not silently substituted)
    so experimenting users see why their chosen font didn't take effect.
    """
    def _resolve(key, default_key, category):
        k = key or default_key
        name = FONT_REGISTRY.get(k)
        if name is None:
            print(f"WARNING: {category} font key '{k}' not in registry; "
                  f"available: {sorted(FONT_REGISTRY.keys())}")
            # Fall back to the default key's resolved name if available,
            # else to the registry's Helvetica shadow (still embedded).
            return (FONT_REGISTRY.get(default_key)
                    or FONT_REGISTRY.get("helvetica")
                    or "Helvetica")
        return name
    mono_fallback = (FONT_REGISTRY.get(DEFAULT_MONO_FONT_KEY)  # courier-prime
                     or FONT_REGISTRY.get("monaco")
                     or FONT_REGISTRY.get(DEFAULT_BODY_FALLBACK_KEY)
                     or FONT_REGISTRY.get("helvetica")
                     or "Courier")
    return FontSet(
        title=_resolve(title_key, DEFAULT_TITLE_FONT_KEY, "title"),
        body=_resolve(body_key, DEFAULT_BODY_FONT_KEY, "body"),
        body_fallback=FONT_REGISTRY.get(DEFAULT_BODY_FALLBACK_KEY)
                      or FONT_REGISTRY.get("helvetica")
                      or "Helvetica",
        mono=_resolve(mono_key, DEFAULT_MONO_FONT_KEY, "mono"),
        mono_fallback=mono_fallback,
    )

# Warm off-white used for ALL letter color on cover and content pages.
# Pure white on pure black is harsh; #F2EEE6 is a soft warm white that reads
# as "white" but with less glare.
LETTER_COLOR = (0xF2 / 255, 0xEE / 255, 0xE6 / 255)  # ~(0.949, 0.933, 0.902)

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


def _draw_image_contain(c, reader, img_size, x, y, w, h):
    """Draw image to fit (contain) inside the rectangle, preserving aspect ratio."""
    img_w, img_h = img_size
    scale = min(w / img_w, h / img_h)
    draw_w = img_w * scale
    draw_h = img_h * scale
    draw_x = x + (w - draw_w) / 2
    draw_y = y + (h - draw_h) / 2
    c.drawImage(reader, draw_x, draw_y, draw_w, draw_h)


def _draw_centered_tracked(c, text, cx, y, font, size, char_space, fill_rgb=None):
    """Draw text centered around cx at baseline y with additional character
    spacing (tracking). `char_space` is in points (text-space units).

    Uses a text object because `setCharSpace` is only available there, not on
    the Canvas directly. Wraps in saveState/restoreState so the Tc (character
    spacing) does NOT leak into subsequent text draws — otherwise later
    drawCentredString calls would be shifted right because their stringWidth
    calculation doesn't know about the inherited Tc.
    """
    if not text:
        return
    base_width = c.stringWidth(text, font, size)
    total_width = base_width + char_space * max(0, len(text) - 1)
    start_x = cx - total_width / 2
    c.saveState()
    text_obj = c.beginText(start_x, y)
    text_obj.setFont(font, size)
    text_obj.setCharSpace(char_space)
    if fill_rgb is not None:
        text_obj.setFillColorRGB(*fill_rgb)
    text_obj.textOut(text)
    c.drawText(text_obj)
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


# ── Per-character font fallback ──────────────────────────────────────
# Trial versions of Canela and Tiempos only ship 66 glyphs — letters, digits,
# space, ',', '-', '.'. Descriptive copy needs apostrophes, em dashes, colons,
# middle dots, x-signs, etc. We split each string into runs: every char that
# the primary font has stays in the primary font; the rest is drawn in the
# fallback font (BODY_FONT, which is ArialUnicode or Helvetica — both cover
# Latin-1 + common typographic chars).

def _font_has_glyph(font_name, ch):
    """Return True if the registered font has a glyph for `ch`.

    For TTFonts, checks the font's cmap directly. For built-in PDF base
    fonts (Helvetica, Courier, Times, ...), assumes all Latin-1 + common
    typographic chars are available via WinAnsi encoding.
    """
    try:
        font = pdfmetrics.getFont(font_name)
    except KeyError:
        return False
    # Registered TTFont — inspect its cmap
    face = getattr(font, "face", None)
    char_to_glyph = getattr(face, "charToGlyph", None) if face else None
    if isinstance(char_to_glyph, dict):
        return ord(ch) in char_to_glyph
    # Built-in base font: assume Latin-1 + the common punctuation chars
    # that WinAnsi covers (em dash, curly quotes, bullet, multiplication,
    # middle dot, etc.).
    return True


def _split_font_runs(text, primary, fallback):
    """Split `text` into a list of (font, substring) runs."""
    if not text:
        return []
    runs = []
    cur_font = None
    cur = []
    for ch in text:
        f = primary if _font_has_glyph(primary, ch) else fallback
        if f != cur_font:
            if cur:
                runs.append((cur_font, "".join(cur)))
            cur_font = f
            cur = [ch]
        else:
            cur.append(ch)
    if cur:
        runs.append((cur_font, "".join(cur)))
    return runs


def _mixed_string_width(c, text, primary, fallback, size):
    """Advance-sum width of `text` when drawn with per-char font fallback."""
    return sum(c.stringWidth(s, f, size) for f, s in _split_font_runs(text, primary, fallback))


def _draw_mixed_centered(c, text, cx, y, primary, fallback, size):
    """Draw `text` centered at cx, using primary where available and
    fallback for characters the primary font is missing."""
    runs = _split_font_runs(text, primary, fallback)
    total = sum(c.stringWidth(s, f, size) for f, s in runs)
    x = cx - total / 2
    for f, s in runs:
        c.setFont(f, size)
        c.drawString(x, y, s)
        x += c.stringWidth(s, f, size)


def _wrap_text_mixed(c, text, primary, fallback, size, max_width):
    """Word-wrap `text` to fit within `max_width`, measuring with per-char
    font fallback so lines with mixed-font runs are wrapped correctly."""
    words = text.split()
    lines = []
    line = ""
    for word in words:
        test = f"{line} {word}".strip()
        if _mixed_string_width(c, test, primary, fallback, size) <= max_width:
            line = test
        else:
            if line:
                lines.append(line)
            line = word
    if line:
        lines.append(line)
    return lines


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


def _pipeline_from_meta(meta):
    """Build the compute pipeline string in the form
    `[ param_transforms ]   function   [ coeff_transforms ]`.

    Returns None if the function is missing — caller should skip rendering.
    Empty/none transform slots render as `[ ]` so the pipeline shape is
    always visible. Brackets have internal padding and elements are separated
    by three spaces so the line reads wider and airier than a dense default.
    """
    if not meta:
        return None
    compute = meta.get("compute") or {}
    fn = (compute.get("function") or "").strip()
    if not fn:
        return None
    pt = (compute.get("param_transforms") or "").strip()
    ct = (compute.get("coeff_transforms") or "").strip()
    if pt.lower() == "none":
        pt = ""
    if ct.lower() == "none":
        ct = ""
    pt_box = f"[ {pt} ]" if pt else "[ ]"
    ct_box = f"[ {ct} ]" if ct else "[ ]"
    return f"{pt_box}   {fn}   {ct_box}"


def _load_image_meta(image_path):
    """Load the `_meta.json` sidecar for an image.

    First tries the exact `<stem>_meta.json` sidecar. If that's missing but
    the filename starts with `compute_<job_id>_`, falls back to any other
    `compute_<job_id>_*_meta.json` in the same directory so the pipeline /
    compute info can still be recovered (the compute payload is identical
    across artifacts of the same job — only the post-processing differs).
    """
    if not image_path:
        return None
    p = Path(image_path)
    meta_path = p.with_name(p.stem + "_meta.json")
    if meta_path.exists():
        try:
            with open(meta_path) as f:
                return json.load(f)
        except (OSError, json.JSONDecodeError):
            pass

    # Fallback: any other meta from the same compute job.
    parts = p.stem.split("_")
    if len(parts) >= 2 and parts[0] == "compute":
        job_prefix = f"compute_{parts[1]}_"
        for candidate in sorted(p.parent.glob(f"{job_prefix}*_meta.json")):
            try:
                with open(candidate) as f:
                    return json.load(f)
            except (OSError, json.JSONDecodeError):
                continue
    return None


def _palette_image_for_entry(image_path, meta):
    """Resolve the associated palette image for an entry, if any.

    Preferred source: `meta["palette_file"]`, treated as a filename relative to
    the image's directory. Fallback: `<image_stem>_palette<image_suffix>` next
    to the image. Returns the path string if found, else None.
    """
    if not image_path:
        return None
    img = Path(image_path)

    if meta:
        palette_file = meta.get("palette_file")
        if palette_file:
            candidate = img.parent / palette_file
            if candidate.exists():
                return str(candidate)

    fallback = img.with_name(f"{img.stem}_palette{img.suffix}")
    if fallback.exists():
        return str(fallback)

    return None


def _scan_string_from_compute(compute):
    """Build the parameter scan string `WxH` or `WxHxN` from compute fields."""
    n_val = compute.get("N")
    if not n_val:
        return ""
    scan = f"{n_val}\u00d7{n_val}"
    times = compute.get("times")
    try:
        if times and int(times) > 1:
            scan += f"\u00d7{int(times)}"
    except (TypeError, ValueError):
        pass
    return scan


def _solver_label(solver):
    """Map a solver token to its human-readable label."""
    if solver == "companion_matrix":
        return "Companion Matrix"
    if solver == "aberth_mt":
        return "Aberth-Ehrlich (parallel)"
    if solver in ("aberth", "", None):
        return "Aberth-Ehrlich"
    return solver


def _score_or_color_label(meta):
    """Return the metric name (for solve_score) or the color method otherwise."""
    color_mode = (meta.get("color_mode") or "").strip()
    if color_mode == "solve_score":
        metric = (meta.get("solve_metric") or "").replace("_", " ").strip()
        return metric or "solve score"
    if color_mode == "saved_palette":
        return "saved palette"
    if color_mode in ("rainbow", "constant", "proximity"):
        return color_mode
    return color_mode


def _palette_or_bilevel_label(meta):
    """Return the palette name, or 'bilevel' for bilevel renders, or '' otherwise."""
    family = (meta.get("family") or "").strip().lower()
    if family == "bilevel":
        return "bilevel"
    color_mode = (meta.get("color_mode") or "").strip()
    if color_mode in ("rainbow", "constant"):
        return ""
    palette = (meta.get("palette") or "").strip()
    if not palette:
        return ""
    return palette.replace("tri_", "tri ").replace("long_", "long ").replace("_", " ")


def _metadata_line_from_meta(meta):
    """Build the metadata line for the body footer.

    Format: `degree N · WxH[xT] · solver · score-or-color · palette-or-bilevel`
    Empty slots are dropped.
    """
    if not meta:
        return ""
    compute = meta.get("compute") or {}
    parts = []

    degree = compute.get("degree") or meta.get("degree")
    if degree:
        parts.append(f"degree {degree}")

    scan = _scan_string_from_compute(compute)
    if scan:
        parts.append(scan)

    solver = _solver_label(compute.get("solver", ""))
    if solver:
        parts.append(solver)

    score_or_color = _score_or_color_label(meta)
    if score_or_color:
        parts.append(score_or_color)

    palette_or_bilevel = _palette_or_bilevel_label(meta)
    if palette_or_bilevel:
        parts.append(palette_or_bilevel)

    return " \u00b7 ".join(parts)


def _strip_legacy_metadata_line(body):
    """Drop the trailing `<fn> · degree N · ...` line from a hand-written body.

    The legacy convention puts the metadata line in its own paragraph (separated
    from the narrative by a blank line). We detect it by looking for ` · degree `
    in the last `\\n\\n`-separated chunk.
    """
    if not body:
        return body
    parts = body.split("\n\n")
    if not parts:
        return body
    last = parts[-1].strip()
    if " \u00b7 degree " in last and "\n" not in last:
        parts = parts[:-1]
    return "\n\n".join(parts).rstrip()


def _measure_body_last_baseline_offset(c, body, primary, fallback, size, max_width,
                                       leading=16, para_gap=6, blank_gap=10):
    """Dry-run the body layout and return the distance from the first body
    baseline to the LAST body baseline (i.e., the y offset of the last line
    drawn, positive). If body has no drawable content, returns 0.

    Mirrors the body loop in `_draw_text_page` exactly.
    """
    if not body:
        return 0
    y = 0
    last = None
    for para in body.split("\n"):
        para = para.strip()
        if not para:
            y -= blank_gap
            continue
        wrapped = _wrap_text_mixed(c, para, primary, fallback, size, max_width)
        for _ in wrapped:
            last = y
            y -= leading
        y -= para_gap
    return -last if last is not None else 0


def _draw_text_page(c, title, body, is_right, filename=None, job_id=None,
                    pipeline=None, palette_path=None, meta_line=None,
                    fonts=None, sizes=None, colors=None, gaps=None):
    """Draw centered title + body text, white on black.

    Text is placed within the trim area with safety margins.
    is_right: whether this is a recto page (affects trim offset).
    filename: optional artifact id shown below body in Courier.
    job_id: optional compute job id shown alongside filename.
    pipeline: optional `[param_transforms] function [coeff_transforms]` line
              shown under the title in Courier (monospace).
    palette_path: optional path to an associated palette image; when provided,
              an 8 cm square showing the palette is drawn between the body text
              and the artifact codes, centered horizontally with a thin white
              border. When a palette is shown, the entire block (title →
              pipeline → body → palette → artifact codes) is vertically
              centered on the page; without a palette, the original
              top-anchored layout is preserved.
    """
    # Trim area origin (bottom-left of trim box within gross page)
    if is_right:
        trim_x = 0
    else:
        trim_x = BLEED_3
    trim_y = BLEED_3

    # Text area: trim area inset by generous margins. Body text uses a
    # NARROWER measure (text_width_body) inside the outer text frame so long
    # paragraphs don't feel dense when set centered on black. Title and
    # technical lines still span the full text_width.
    text_margin = 40 * mm
    tx = trim_x + text_margin
    tw = CONTENT_NET - 2 * text_margin
    center_x = tx + tw / 2
    body_width = tw - 35 * mm  # narrower measure for Tiempos descriptive copy

    fonts = fonts or resolve_fonts()
    sizes = sizes or DEFAULT_SIZES
    colors = colors or DEFAULT_COLORS
    gaps = gaps or DEFAULT_GAPS

    palette_side = 80 * mm  # 8 cm square

    # Title + gap constants.
    # All inter-element gaps are integer LINE UNITS (1 line unit == body_leading),
    # plumbed through `gaps` so they're CLI-controllable for layout experimentation.
    title_size = sizes.title
    body_leading = 16  # also used as "line unit" for vertical rhythm
    title_to_pipeline = gaps.title_pipeline * body_leading
    pipeline_to_body  = gaps.pipeline_body  * body_leading
    body_to_meta      = gaps.body_meta      * body_leading
    palette_gap       = gaps.meta_palette   * body_leading  # meta baseline -> palette top
    title_to_body_no_pipeline = 5 * body_leading  # 80pt (when no pipeline line)

    # Body typography — size from `sizes.body`.
    # Per-char font fallback to `fonts.body_fallback` for any glyph the
    # descriptive body font doesn't ship (apostrophes, em dashes, middle
    # dots, multiplication signs, etc.).
    body_font = fonts.body
    body_fallback = fonts.body_fallback
    body_size = sizes.body
    body_para_gap = 6
    body_blank_gap = 10

    # Monospace line sizes: pipeline at `sizes.mono`, artifact-id one point
    # smaller for the quieter credit line. Both share the same `colors.mono`.
    mono_pipeline_size = sizes.mono
    mono_artifact_size = max(6.0, sizes.mono - 1.0)
    mono_color = colors.mono
    text_color = colors.text

    title_to_body = (title_to_pipeline + pipeline_to_body) if pipeline else title_to_body_no_pipeline

    # Title baseline:
    # - With palette: vertically center the entire block on the page.
    # - Without palette: keep the existing top-anchored offset.
    if palette_path:
        last_body_offset = _measure_body_last_baseline_offset(
            c, body, body_font, body_fallback, body_size, body_width,
            leading=body_leading, para_gap=body_para_gap, blank_gap=body_blank_gap)
        # Block extent measured from title baseline DOWN to artifact baseline:
        #   title             -> first body baseline:   title_to_body
        #   first body bl     -> last body bl:          last_body_offset
        #   last body bl      -> meta baseline:         body_to_meta (if meta)
        #   last visible bl   -> palette top:           palette_gap
        #   palette top       -> palette bottom:        palette_side
        #   palette bottom    -> artifact baseline:     14pt
        meta_extent = body_to_meta if meta_line else 0
        block_baseline_span = (title_to_body + last_body_offset + meta_extent
                               + palette_gap + palette_side + 14)
        page_center_y = trim_y + CONTENT_NET / 2
        title_y = page_center_y + block_baseline_span / 2
    else:
        title_y = trim_y + CONTENT_NET / 2 + 15 * mm + 40

    body_first_baseline = title_y - title_to_body

    c.setFillColorRGB(*text_color)

    if title:
        c.setFont(fonts.title, title_size)
        c.drawCentredString(center_x, title_y, title)

    if pipeline:
        c.setFillColorRGB(*mono_color)
        _draw_mixed_centered(c, pipeline, center_x,
                             title_y - title_to_pipeline,
                             fonts.mono, fonts.mono_fallback,
                             mono_pipeline_size)
        c.setFillColorRGB(*text_color)

    # Render body, tracking the last drawn baseline for downstream layout.
    last_baseline = body_first_baseline
    y = body_first_baseline
    if body:
        for para in body.split("\n"):
            para = para.strip()
            if not para:
                y -= body_blank_gap
                continue
            wrapped = _wrap_text_mixed(c, para, body_font, body_fallback,
                                       body_size, body_width)
            for ln in wrapped:
                _draw_mixed_centered(c, ln, center_x, y,
                                     body_font, body_fallback, body_size)
                last_baseline = y
                y -= body_leading
            y -= body_para_gap

    # Metadata "choice" line — fixed 3-empty-line gap from the last body baseline.
    if meta_line:
        meta_y = last_baseline - body_to_meta
        _draw_mixed_centered(c, meta_line, center_x, meta_y,
                             body_font, body_fallback, body_size)
        last_baseline = meta_y

    artifact_y = last_baseline - 16
    if palette_path:
        try:
            palette_reader, palette_size = _load_image_rgb(palette_path)
        except Exception:
            palette_reader = None
            palette_size = None
        if palette_reader is not None:
            palette_x = center_x - palette_side / 2
            # Fixed vertical distance from the last rendered baseline (meta
            # line or last body line) to palette top.
            palette_y = last_baseline - palette_gap - palette_side
            _draw_image_contain(c, palette_reader, palette_size,
                                palette_x, palette_y, palette_side, palette_side)
            artifact_y = palette_y - 14

    if filename or job_id:
        c.setFillColorRGB(*mono_color)
        id_parts = [p for p in [job_id, filename] if p]
        _draw_mixed_centered(c, " \u00b7 ".join(id_parts), center_x, artifact_y,
                             fonts.mono, fonts.mono_fallback,
                             mono_artifact_size)


# ── Content PDF ───────────────────────────────────────────────────────

def _emit_black_page(c, is_right):
    """Set the trim box for verso/recto and paint a full black background."""
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


def _render_entry_verso(c, entry, fonts, sizes, colors, gaps):
    """Render the text side (verso) for one config entry on the current page.
    Assumes `_emit_black_page(c, is_right=False)` has already run.
    """
    snap_name = entry.get("filename") or os.path.splitext(os.path.basename(entry.get("image", "")))[0]
    meta = _load_image_meta(entry.get("image"))
    pipeline = _pipeline_from_meta(meta)
    palette_path = _palette_image_for_entry(entry.get("image"), meta)

    body = entry.get("text", "")
    new_meta_line = _metadata_line_from_meta(meta)
    if new_meta_line:
        body = _strip_legacy_metadata_line(body)

    _draw_text_page(c, entry.get("title", ""),
                    body, is_right=False,
                    filename=snap_name, job_id=entry.get("job_id", ""),
                    pipeline=pipeline, palette_path=palette_path,
                    meta_line=new_meta_line,
                    fonts=fonts, sizes=sizes, colors=colors, gaps=gaps)


def _render_entry_recto(c, entry):
    """Draw the image side (recto) for one config entry on the current page.
    Assumes `_emit_black_page(c, is_right=True)` has already run.
    """
    img_path = entry.get("image")
    if img_path and os.path.exists(img_path):
        reader, size = _load_image_rgb(img_path)
        _draw_image_cover(c, reader, size,
                          0, 0, CONTENT_GROSS_W, CONTENT_GROSS_H)


def generate_content_pdf(output_path, pages_config,
                         fonts=None, sizes=None, colors=None, gaps=None):
    """Generate the full content pages PDF.

    Each page is CONTENT_GROSS_W x CONTENT_GROSS_H (293 x 296 mm).
    Bleed of 3 mm on top, bottom, and outer edge; 0 on binding edge.
    TrimBox alternates for recto (right) / verso (left) pages.
    Page count padded to a multiple of 4.
    """
    fonts = fonts or resolve_fonts()
    sizes = sizes or DEFAULT_SIZES
    colors = colors or DEFAULT_COLORS
    gaps = gaps or DEFAULT_GAPS
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

    page_num = 0

    # Page 1: blank recto
    _emit_black_page(c, is_right=True)
    c.showPage()
    page_num += 1

    # Each config entry: verso (text) + recto (image)
    for entry in pages_config:
        _emit_black_page(c, is_right=False)
        _render_entry_verso(c, entry, fonts, sizes, colors, gaps)
        c.showPage()
        page_num += 1

        _emit_black_page(c, is_right=True)
        _render_entry_recto(c, entry)
        c.showPage()
        page_num += 1

    # Pad remaining pages to reach n_pages
    while page_num < n_pages:
        is_right = (page_num % 2 == 0)
        _emit_black_page(c, is_right)
        c.showPage()
        page_num += 1

    c.save()
    print(f"  Content PDF: {output_path}  ({n_pages} pages, "
          f"{CONTENT_GROSS_W/mm:.0f} x {CONTENT_GROSS_H/mm:.0f} mm gross)")


def generate_spread_pdf(output_path, pages_config, spread_index,
                        fonts=None, sizes=None, colors=None, gaps=None):
    """Render ONE spread (verso text + recto image) as a 2-page PDF for
    fast layout/font experimentation. `spread_index` is 1-based into
    pages_config.
    """
    fonts = fonts or resolve_fonts()
    sizes = sizes or DEFAULT_SIZES
    colors = colors or DEFAULT_COLORS
    gaps = gaps or DEFAULT_GAPS
    if not pages_config:
        raise ValueError("pages_config is empty")
    if spread_index < 1 or spread_index > len(pages_config):
        raise ValueError(
            f"spread index {spread_index} out of range 1..{len(pages_config)}")
    entry = pages_config[spread_index - 1]

    c = canvas.Canvas(str(output_path),
                      pagesize=(CONTENT_GROSS_W, CONTENT_GROSS_H))
    c.setTitle(f"PolyPaint - Spread {spread_index}")
    c.setAuthor("make_book.py")

    # Verso (text)
    _emit_black_page(c, is_right=False)
    _render_entry_verso(c, entry, fonts, sizes, colors, gaps)
    c.showPage()

    # Recto (image)
    _emit_black_page(c, is_right=True)
    _render_entry_recto(c, entry)
    c.showPage()

    c.save()
    title = entry.get("title", "") or "(untitled)"
    print(f"  Spread PDF:  {output_path}  "
          f"(spread {spread_index}/{len(pages_config)} — {title!r})")
    print(f"    fonts:  title={fonts.title}  body={fonts.body}  mono={fonts.mono}")
    print(f"    sizes:  title={sizes.title}pt  body={sizes.body}pt  mono={sizes.mono}pt")
    print(f"    colors: text=#{int(colors.text[0]*255):02X}{int(colors.text[1]*255):02X}{int(colors.text[2]*255):02X}  "
          f"mono=#{int(colors.mono[0]*255):02X}{int(colors.mono[1]*255):02X}{int(colors.mono[2]*255):02X}")
    print(f"    gaps (line units): title-pipeline={gaps.title_pipeline} "
          f"pipeline-body={gaps.pipeline_body} body-meta={gaps.body_meta} "
          f"meta-palette={gaps.meta_palette}")


# ── Cover PDF ─────────────────────────────────────────────────────────

def generate_cover_pdf(output_path, title="Polynomiography", subtitle="",
                       front_image=None, back_image=None,
                       description="", author="",
                       imprint=None, spine_palette_image=None):
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

    # Warm off-white for display typography (#F2EEE6) — reads as white but
    # softer than pure 1,1,1 on a black cover.
    warm_white = LETTER_COLOR
    # Slightly open tracking for the display title (point-space char spacing).
    title_char_space = 2.0  # ~55 units at 36pt — "slightly open, not too much"
    title_font_size = 36
    title_sub_size = 13

    if front_image and os.path.exists(front_image):
        reader, size = _load_image_rgb(front_image)
        # Image at 2/3 of panel size, centered horizontally, in upper portion
        img_scale = 2.0 / 3.0
        img_w = PANEL_W * img_scale
        img_h = COVER_NET_H * img_scale
        img_x = front_left + (PANEL_W - img_w) / 2
        gap_below_img = 25 * mm
        title_block_h = title_font_size + (20 if subtitle else 0)
        # Vertically center: image + gap + title block
        total_block = img_h + gap_below_img + title_block_h
        block_top = COVER_BLEED + (COVER_NET_H + total_block) / 2
        img_y = block_top - img_h
        _draw_image_cover(c, reader, size, img_x, img_y, img_w, img_h)

        # Title below the image — Canela Light, Title Case, slightly warm white,
        # slightly open tracking.
        title_y = img_y - gap_below_img
        _draw_centered_tracked(c, title, front_cx, title_y,
                               COVER_DISPLAY_FONT, title_font_size,
                               title_char_space, fill_rgb=warm_white)
        if subtitle:
            _draw_centered_tracked(c, subtitle, front_cx, title_y - 24,
                                   COVER_DISPLAY_FONT, title_sub_size,
                                   title_char_space * 0.3, fill_rgb=warm_white)
    else:
        # Text-only front cover
        _draw_centered_tracked(c, title, front_cx, front_cy + 30,
                               COVER_DISPLAY_FONT, 48,
                               title_char_space, fill_rgb=warm_white)
        if subtitle:
            _draw_centered_tracked(c, subtitle, front_cx, front_cy - 30,
                                   COVER_DISPLAY_FONT, 24,
                                   title_char_space * 0.5, fill_rgb=warm_white)

    # ── Spine text (rotated 90 CCW, read bottom-to-top) ──
    spine_cx = COVER_BLEED + PANEL_W + SPINE_W / 2
    spine_cy = COVER_GROSS_H / 2
    c.saveState()
    c.translate(spine_cx, spine_cy)
    c.rotate(90)
    c.setFillColorRGB(*warm_white)
    c.setFont(COVER_DISPLAY_FONT_REGULAR, 7)
    c.drawCentredString(0, -2.5, title)
    c.restoreState()

    # ── Tiny palette square at the bottom of the spine ──
    if spine_palette_image and os.path.exists(spine_palette_image):
        try:
            spine_palette_reader, spine_palette_size = _load_image_rgb(spine_palette_image)
        except Exception:
            spine_palette_reader = None
            spine_palette_size = None
        if spine_palette_reader is not None:
            sp_side = SPINE_W - 2  # 2pt padding inside the spine width
            if sp_side > 0:
                sp_x = spine_cx - sp_side / 2
                sp_y = COVER_BLEED + 15 * mm
                _draw_image_contain(c, spine_palette_reader, spine_palette_size,
                                    sp_x, sp_y, sp_side, sp_side)

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
    c.setFont(COVER_DISPLAY_FONT_REGULAR, 9)
    desc_lines = _wrap_text(c, description, COVER_DISPLAY_FONT_REGULAR, 9, tw) if description else []
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
        c.setFont(COVER_DISPLAY_FONT_REGULAR, 9)
        for ln in desc_lines:
            c.drawCentredString(back_cx, y, ln)
            y -= text_leading

    # Author name
    if author:
        c.setFillColorRGB(*warm_white)
        c.setFont(COVER_DISPLAY_FONT_REGULAR, 10)
        c.drawCentredString(back_cx, y - gap_text_title, author)

    # Imprint (falling back to title) at bottom of back cover
    c.setFillColorRGB(*warm_white)
    c.setFont(COVER_DISPLAY_FONT_REGULAR, 8)
    c.drawCentredString(back_cx, COVER_BLEED + 15 * mm, imprint or title)

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
    parser.add_argument("--cover-only", action="store_true",
                        help="Regenerate only the cover PDF (skip content)")
    parser.add_argument("--spread", type=int, metavar="N",
                        help="Render ONLY spread N (1-based) to <prefix>_spread<N>.pdf "
                             "for quick layout/font experiments")
    parser.add_argument("--list-fonts", action="store_true",
                        help="List registered fonts (by key) and exit")

    # Font selection — keys from FONT_REGISTRY
    font_keys = sorted(FONT_REGISTRY.keys())
    parser.add_argument("--title-font", choices=font_keys, default=None,
                        help=f"Title font key (default: {DEFAULT_TITLE_FONT_KEY})")
    parser.add_argument("--body-font", choices=font_keys, default=None,
                        help=f"Descriptive body copy font key (default: {DEFAULT_BODY_FONT_KEY})")
    parser.add_argument("--mono-font", choices=font_keys, default=None,
                        help=f"Monospace/technical font key (default: {DEFAULT_MONO_FONT_KEY})")

    # Font sizes (points)
    parser.add_argument("--title-size", type=float, default=DEFAULT_SIZES.title,
                        metavar="PT", help=f"Title point size (default: {DEFAULT_SIZES.title})")
    parser.add_argument("--body-size", type=float, default=DEFAULT_SIZES.body,
                        metavar="PT", help=f"Body point size (default: {DEFAULT_SIZES.body})")
    parser.add_argument("--mono-size", type=float, default=DEFAULT_SIZES.mono,
                        metavar="PT", help=f"Mono (pipeline) point size; artifact-id is mono-size - 1 "
                                            f"(default: {DEFAULT_SIZES.mono})")

    # Colors (CSS hex: #RRGGBB or RRGGBB)
    def _color_hex(default):
        return f"#{int(default[0]*255):02X}{int(default[1]*255):02X}{int(default[2]*255):02X}"
    parser.add_argument("--text-color", default=None, metavar="HEX",
                        help=f"Hex color for title + body + metadata line "
                             f"(default: {_color_hex(DEFAULT_COLORS.text)})")
    parser.add_argument("--mono-color", default=None, metavar="HEX",
                        help=f"Hex color for pipeline + artifact-id lines "
                             f"(default: {_color_hex(DEFAULT_COLORS.mono)})")

    # Gap control — all in LINE UNITS (1 unit == body_leading == 16pt)
    parser.add_argument("--gap-title-pipeline", type=int, default=DEFAULT_GAPS.title_pipeline,
                        metavar="N", help="Line-unit gap from title baseline to pipeline baseline "
                                           f"(default: {DEFAULT_GAPS.title_pipeline})")
    parser.add_argument("--gap-pipeline-body", type=int, default=DEFAULT_GAPS.pipeline_body,
                        metavar="N", help="Line-unit gap from pipeline baseline to first body baseline "
                                           f"(default: {DEFAULT_GAPS.pipeline_body})")
    parser.add_argument("--gap-body-meta", type=int, default=DEFAULT_GAPS.body_meta,
                        metavar="N", help="Line-unit gap from last body baseline to metadata-line baseline "
                                           f"(default: {DEFAULT_GAPS.body_meta})")
    parser.add_argument("--gap-meta-palette", type=int, default=DEFAULT_GAPS.meta_palette,
                        metavar="N", help="Line-unit gap from metadata-line baseline to palette-square top "
                                           f"(default: {DEFAULT_GAPS.meta_palette})")

    args = parser.parse_args()

    if args.list_fonts:
        print("Registered fonts (key -> reportlab name):")
        for key in sorted(FONT_REGISTRY.keys()):
            print(f"  {key:20s} -> {FONT_REGISTRY[key]}")
        print()
        print(f"Defaults: title={DEFAULT_TITLE_FONT_KEY} "
              f"body={DEFAULT_BODY_FONT_KEY} mono={DEFAULT_MONO_FONT_KEY}")
        return

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
    imprint = config.get("imprint")
    spine_palette_image = config.get("spine_palette_image")

    fonts = resolve_fonts(title_key=args.title_font,
                          body_key=args.body_font,
                          mono_key=args.mono_font)
    sizes = SizeSet(title=args.title_size,
                    body=args.body_size,
                    mono=args.mono_size)
    try:
        text_color = parse_hex_color(args.text_color) if args.text_color else DEFAULT_COLORS.text
        mono_color = parse_hex_color(args.mono_color) if args.mono_color else DEFAULT_COLORS.mono
    except ValueError as e:
        print(f"ERROR: bad color: {e}")
        sys.exit(2)
    colors = ColorSet(text=text_color, mono=mono_color)
    gaps = GapSet(title_pipeline=args.gap_title_pipeline,
                  pipeline_body=args.gap_pipeline_body,
                  body_meta=args.gap_body_meta,
                  meta_palette=args.gap_meta_palette)

    # Single-spread experiment mode — skip cover + book-wide content, just
    # render one 2-page PDF with the chosen fonts and gaps.
    if args.spread is not None:
        spread_path = f"{args.output_prefix}_spread{args.spread}.pdf"
        print(f"Generating single spread {args.spread}/{len(pages)}...")
        print()
        generate_spread_pdf(spread_path, pages, args.spread,
                            fonts=fonts, sizes=sizes, colors=colors, gaps=gaps)
        return

    print(f"Generating WhiteWall PDFs: '{title}'")
    n_content = 1 + len(pages) * 2
    n_padded = n_content + (4 - n_content % 4) % 4
    n_padded = max(4, n_padded)
    print(f"  {len(pages)} spread(s) -> {n_padded} content pages")
    print(f"  fonts: title={fonts.title} body={fonts.body} mono={fonts.mono}")
    print()

    cover_path = f"{args.output_prefix}_cover.pdf"
    content_path = f"{args.output_prefix}_content.pdf"

    generate_cover_pdf(cover_path, title, subtitle, front_image,
                       back_image, description, author,
                       imprint=imprint, spine_palette_image=spine_palette_image)
    if not args.cover_only:
        generate_content_pdf(content_path, pages,
                             fonts=fonts, sizes=sizes, colors=colors, gaps=gaps)

    print()
    print("Done! Upload to WhiteWall:")
    print(f"  Cover:   {cover_path}")
    if not args.cover_only:
        print(f"  Content: {content_path}")


if __name__ == "__main__":
    main()
