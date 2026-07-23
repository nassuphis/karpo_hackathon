"""Book Maker LaTeX templates (book-maker-design.md §6).

Pure string templating: the compose op renders book.tex / cover.tex from the
book document + provenance snapshots, then runs lualatex. Everything here is
testable without a TeX installation.

Geometry (verified against the WhiteWall IDML templates):
  content page: 290 mm net -> 293 x 296 mm gross (bleed on 3 sides, none at
  the binding edge); cover panels are 296 mm square with 10 mm outer bleed.
  The spine, and therefore the gross cover width, follows the content page
  count. The 44-page template is 296 + 14 + 296 mm trim -> 626 x 316 mm gross.

Page plan: front matter must be an ODD page count so entry text lands on
versos (left) and images on rectos (right) of the same opening
(make_polypaint_book.py:905-916 is the reference). p1 = title recto, then
2 pages per entry, then selected-background pads to a multiple of 4.
"""

CONTENT_W_MM = 293
CONTENT_H_MM = 296
COVER_PANEL_MM = 296
COVER_BLEED_MM = 10
COVER_TRIM_H_MM = 296
COVER_H_MM = COVER_TRIM_H_MM + 2 * COVER_BLEED_MM

# WhiteWall Fuji Crystal semi-matte IDML measurements:
#   28 pages: 603 mm trim = 296 + 11 + 296
#   44 pages: 606 mm trim = 296 + 14 + 296
# The paper block adds 0.75 mm per four content pages. WhiteWall supports
# 28-112 pages for this paper; keeping the same linear geometry outside that
# range preserves small app-only flipbook fixtures without claiming that the
# publisher accepts their page count.
COVER_SPINE_REFERENCE_PAGES = 28
COVER_SPINE_REFERENCE_MM = 11
COVER_SPINE_MM_PER_PAGE = 3 / 16
DEFAULT_BOOK_BACKGROUND_COLOR = "1A1A2E"

# The compose build dir lays prepared images out under assets/.
ASSET_DIR = "assets"
S3_PUBLIC_BASE = "https://polypaint.s3.us-east-1.amazonaws.com/"

_TEX_SPECIALS = {
    "\\": r"\textbackslash{}",
    "{": r"\{",
    "}": r"\}",
    "$": r"\$",
    "&": r"\&",
    "#": r"\#",
    "%": r"\%",
    "^": r"\textasciicircum{}",
    "_": r"\_",
    "~": r"\textasciitilde{}",
}


def tex_escape(text):
    """Escape user/provenance text for LaTeX body use; control chars drop."""
    out = []
    for ch in str(text or ""):
        if ch in _TEX_SPECIALS:
            out.append(_TEX_SPECIALS[ch])
        elif ch == "\n":
            out.append("\\\\\n")
        elif ch.isprintable():
            out.append(ch)
    return "".join(out)


def book_background_color(book):
    """Canonical six-digit HTML colour embedded into both Book PDFs."""
    raw = (book or {}).get("background_color") if isinstance(book, dict) else None
    text = str(raw or DEFAULT_BOOK_BACKGROUND_COLOR).strip()
    if text.startswith("#"):
        text = text[1:]
    if len(text) == 3 and all(ch.lower() in "0123456789abcdef" for ch in text):
        text = "".join(ch + ch for ch in text)
    if len(text) != 6 or any(ch.lower() not in "0123456789abcdef" for ch in text):
        raise ValueError(f"book background_color must be 6-digit hex, got {raw!r}")
    return text.upper()


def _relative_luminance(hex_color):
    channels = [int(hex_color[i:i + 2], 16) / 255.0 for i in (0, 2, 4)]

    def linear(channel):
        return channel / 12.92 if channel <= 0.04045 else ((channel + 0.055) / 1.055) ** 2.4

    red, green, blue = (linear(channel) for channel in channels)
    return 0.2126 * red + 0.7152 * green + 0.0722 * blue


def _book_color_scheme(book):
    background = book_background_color(book)
    # Select whichever foreground has the stronger WCAG contrast against the
    # chosen page colour. QR colours remain fixed separately below.
    light_background = _relative_luminance(background) > 0.179
    if light_background:
        return {
            "pagebg": background,
            "bodytext": "111827",
            "monotext": "4B5563",
            "panelbg": "F3F4F6",
            "panelborder": "CBD5E1",
            "rulecol": "6B7280",
        }
    return {
        "pagebg": background,
        "bodytext": "F2F2F7",
        "monotext": "9AA0B4",
        "panelbg": "121829",
        "panelborder": "2B3A5E",
        "rulecol": "5F6678",
    }


def page_plan(n_entries):
    """Return (total_pages, pad_pages): 1 title recto + 2 per entry + pad,
    total ≡ 0 (mod 4). Even N -> pad 3, odd N -> pad 1."""
    n = int(n_entries)
    if n < 0:
        raise ValueError("entry count must be >= 0")
    used = 1 + 2 * n
    pad = (4 - used % 4) % 4
    total = max(4, used + pad)
    return total, total - used


def cover_geometry(content_pages):
    """Return page-count-specific WhiteWall cover geometry in millimetres."""
    if isinstance(content_pages, bool):
        raise ValueError("cover content page count must be an integer")
    try:
        pages = int(content_pages)
    except (TypeError, ValueError) as exc:
        raise ValueError("cover content page count must be an integer") from exc
    if pages != content_pages or pages <= 0 or pages % 4:
        raise ValueError("cover content page count must be a positive multiple of 4")
    spine_mm = (
        COVER_SPINE_REFERENCE_MM
        + (pages - COVER_SPINE_REFERENCE_PAGES) * COVER_SPINE_MM_PER_PAGE
    )
    if spine_mm <= 0:
        raise ValueError(f"cover spine width is invalid for {pages} content pages")
    trim_width_mm = 2 * COVER_PANEL_MM + spine_mm
    return {
        "content_pages": pages,
        "panel_mm": float(COVER_PANEL_MM),
        "spine_mm": float(spine_mm),
        "bleed_mm": float(COVER_BLEED_MM),
        "trim_width_mm": float(trim_width_mm),
        "trim_height_mm": float(COVER_TRIM_H_MM),
        "width_mm": float(trim_width_mm + 2 * COVER_BLEED_MM),
        "height_mm": float(COVER_H_MM),
    }


def _format_mm(value):
    return f"{float(value):.6f}".rstrip("0").rstrip(".")


def _font_setup():
    # TTFs are installed into the image's texmf tree by the Dockerfile; the
    # names here must match those files (polypaint/fonts/, git-tracked).
    # IMPORTANT: use only genuinely complete faces. The trial fonts (Canela,
    # Tiempos, Sohne) are missing ( ) = _ %, and the Lyon *demo* maps them to
    # a "DEMO" watermark ornament — both render as garbage on technical values.
    # Baramond (a complete Garamond text serif) and CourierPrime (mono) render
    # all Latin punctuation cleanly. Verified by rendering ( ) = _ % per font.
    return "\n".join([
        r"\usepackage{fontspec}",
        r"\setmainfont{Baramond-Regular.ttf}",
        r"\newfontfamily\displayfont{Baramond-Regular.ttf}",
        r"\newfontfamily\monofont{CourierPrime-Regular.ttf}",
        r"\usepackage{microtype}",
    ])


def _content_preamble(book):
    # The default scheme matches the app/ColorSpread deep blue; a Book may
    # replace the page colour and gets a contrast-safe light/dark text scheme.
    colors = _book_color_scheme(book)
    return "\n".join([
        r"\documentclass{article}",
        # vertical margins are 1mm: vertical placement is EXPLICIT (title at
        # a chosen height, palette sized to fit the freed space). Horizontal
        # margins stay 24mm — that is text measure + trim safety.
        r"\usepackage[paperwidth=%dmm, paperheight=%dmm, hmargin=24mm, vmargin=1mm]{geometry}"
        % (CONTENT_W_MM, CONTENT_H_MM),
        r"\usepackage{graphicx}",
        r"\usepackage{xcolor}",
        r"\usepackage{eso-pic}",
        r"\usepackage{array}",
        r"\usepackage{qrcode}",
        _font_setup(),
        r"\pagestyle{empty}",
        r"\setlength{\parindent}{0pt}",
        r"\definecolor{pagebg}{HTML}{%s}" % colors["pagebg"],
        r"\definecolor{accent}{HTML}{E94560}",   # ACCENT
        r"\definecolor{bodytext}{HTML}{%s}" % colors["bodytext"],
        r"\definecolor{monotext}{HTML}{%s}" % colors["monotext"],
        r"\definecolor{panelbg}{HTML}{%s}" % colors["panelbg"],
        r"\definecolor{panelborder}{HTML}{%s}" % colors["panelborder"],
        r"\definecolor{rulecol}{HTML}{%s}" % colors["rulecol"],
        r"\definecolor{qrbg}{HTML}{FFFFFF}",
        r"\definecolor{qrfg}{HTML}{111827}",
    ])


def _background_page():
    return "\n".join([
        r"\newpage",
        r"\pagecolor{pagebg}",
        r"\null",
    ])


def _full_bleed_image(rel_path):
    return "\n".join([
        r"\newpage",
        r"\AddToShipoutPictureBG*{\AtPageLowerLeft{"
        r"\includegraphics[width=\paperwidth,height=\paperheight]{%s}}}" % rel_path,
        r"\null",
    ])


def entry_title(entry, provenance):
    """Verso heading parts: (compute id, optional generated/hand title).
    The verso joins them on one line with a small grey rule between."""
    report = (provenance or {}).get("report") or {}
    base = str(report.get("compute_id") or entry.get("job_id") or entry.get("artifact_id") or "")
    override = str(entry.get("title_override") or "").strip()
    return base, override


def _report_rows(entry, provenance):
    report = (provenance or {}).get("report") or {}
    rows = report.get("summary_rows") or []
    return [(str(r[0]), str(r[1])) for r in rows if isinstance(r, (list, tuple)) and len(r) == 2]


PALETTE_MAX_MM = 160
PALETTE_MIN_MM = 120


def _palette_mm(rows, body_override):
    """Palette square sized to what the verso's content leaves over — the
    one-page spread invariant is sacred. The description sits BESIDE the KV
    rows, so the middle band costs max(rows, description) not their sum.
    Budget (mm): 294 block − ~30 title zone − max(6.5/row, ~4.6/desc line at
    the 108mm column) − 16 palette chrome − 8 safety."""
    body_lines = 0
    for line in str(body_override or "").strip().splitlines():
        body_lines += max(1, (len(line) + 43) // 44)   # ~44 chars/line at 12/16pt
    band = max(6.5 * len(rows), 5.7 * body_lines)
    avail = 294 - 30 - band - 16 - 8
    return int(max(PALETTE_MIN_MM, min(PALETTE_MAX_MM, avail)))


def book_spread_layout(book):
    """color_primary (default) or palette_primary — the Layout tab option."""
    layout = str((book or {}).get("spread_layout") or "color_primary")
    return layout if layout in ("color_primary", "palette_primary") else "color_primary"


def _verso_report_page(entry, provenance, layout="color_primary"):
    """Deep-blue report page matching the ColorSpread PDF: title, accent
    rule, artifact id, fixed-pitch KV rows, and the palette at a FIXED
    120x120mm in a centered table. Everything is fixed-size, so the worst
    case (10 rows) totals ~230mm < the 248mm text block: the page can
    never overflow. Plain LaTeX, no overlay tricks."""
    report = (provenance or {}).get("report") or {}
    base_title, override_title = entry_title(entry, provenance)
    # separator: a small raised hairline in the muted grey (a "::" read as
    # punctuation noise; a little rule reads as design)
    title = tex_escape(base_title or override_title or "")
    if base_title and override_title:
        title = (tex_escape(base_title)
                 + r"\hspace{4mm}\raisebox{0.18em}{\color{rulecol}\rule{7mm}{0.6pt}}\hspace{4mm}"
                 + tex_escape(override_title))
    # QR on EVERY spread with an image (title or not): scans to the public
    # full-res image — the printed page linking to its digital original
    image_key = str(entry.get("image_key") or "").strip()
    if image_key:
        url = S3_PUBLIC_BASE + image_key
        # dark modules on a light chip (an inverted QR scans unreliably);
        # 1.5mm fboxsep = the quiet zone the spec wants. 14mm + level L:
        # image URLs are ~118 chars, and at 12mm/level M the module pitch
        # fell below what the 200dpi flipbook raster can carry — L drops
        # the grid a version, and with 14mm the pitch lands above the
        # title-page QR's proven-decodable density.
        # -3.8mm centers the 17mm chip on the caps' midline (~3.2mm above
        # baseline at 26pt); the top \vspace* gives back the extra depth
        title += (r"\hfill\raisebox{0.3mm}{\setlength{\fboxsep}{1.5mm}"
                  r"\colorbox{qrbg}{\color{qrfg}\qrcode[height=14mm,level=L]{%s}}}" % url)
    artifact = tex_escape(str(report.get("artifact_id") or entry.get("artifact_id") or ""))
    rows = _report_rows(entry, provenance)
    body_override = str(entry.get("body_override") or "").strip()
    palette_label = tex_escape(str(report.get("palette_label") or "palette"))
    # palette_primary flips the spread: the palette owns the recto, the color
    # sits in the verso square. Entries without a palette keep color_primary.
    swapped = layout == "palette_primary" and bool(report.get("has_palette"))

    parts = [
        r"\newpage",
        r"\pagecolor{pagebg}\color{bodytext}",
        # explicit vertical placement (vmargin=1mm): cap height lands ~8mm
        # below the physical page top; the 2mm shaved off here pays for the
        # QR chip's deeper raisebox so the rule and everything below stay put
        r"\vspace*{4mm}",
        r"{\displayfont\fontsize{26}{30}\selectfont %s\par}" % (title or "PolyPaint"),
        r"\vspace{2.5mm}",
        r"{\color{rulecol}\rule{\linewidth}{0.8pt}}\par",
        r"\vspace{1.5mm}",
        # swapped spreads: the recto is the palette, so its title labels it
        # from up here, flush RIGHT toward the image it names (user spec);
        # the artifact id keeps its flush-left home
        (r"{\monofont\footnotesize\color{monotext}\hbox to \linewidth{%s\hss %s}\par}"
         % (artifact, palette_label)) if swapped else
        (r"{\monofont\footnotesize\color{monotext} %s\par}" % artifact),
        r"\vspace{7mm}",
    ]
    # KV rows on the left (fixed 6.5mm pitch, reference PDF KV_PITCH=17pt);
    # the description sits in a column to their right
    parts.append(r"\noindent\begin{minipage}[t]{128mm}")
    for label, value in rows:
        parts.append(
            r"\noindent\hbox to \linewidth{\hbox to 42mm{\monofont\footnotesize\color{monotext} %s\hss}"
            r"{\color{bodytext} %s}\hss}\vspace*{\dimexpr6.5mm-\baselineskip\relax}\par"
            % (tex_escape(label.upper()), tex_escape(value)))
    parts.append(r"\end{minipage}%")
    if body_override:
        parts.append(r"\hfill\begin{minipage}[t]{108mm}")
        parts.append(r"\setlength{\parskip}{2mm}\raggedright")
        for line in body_override.splitlines():
            parts.append(r"{\fontsize{12}{16}\selectfont\color{bodytext} %s\par}" % tex_escape(line))
        parts.append(r"\end{minipage}")
    parts.append(r"\par")

    if report.get("has_palette"):
        square_asset = (f"{entry.get('entry_id')}.jpg" if swapped
                        else f"{entry.get('entry_id')}.palette.jpg")
        square_label = artifact if swapped else palette_label
        parts.extend([
            r"\vfill",
            r"\begin{center}",
            r"\begin{tabular}{c}",
            r"\fcolorbox{panelborder}{panelbg}{\includegraphics[width=%dmm,height=%dmm,keepaspectratio]{%s/%s}} \\[2mm]"
            % (_palette_mm(rows, body_override), _palette_mm(rows, body_override),
               ASSET_DIR, square_asset),
            r"{\monofont\footnotesize\color{monotext} %s} \\" % square_label,
            r"\end{tabular}",
            r"\end{center}",
            r"\vfill",
        ])
    parts.append(r"\null")
    return "\n".join(parts)


def render_content_tex(book, provenance_by_entry=None, pdf_url=None):
    """Emit the content PDF source: title page + one spread per entry + pads.

    pdf_url: public URL of the content PDF being built (known before TeX
    runs). When set, the title page carries a QR to it at bottom center —
    self-referential on purpose: scanned from the flipbook or the printed
    book, it downloads this exact compile."""
    provenance_by_entry = provenance_by_entry or {}
    entries = list(book.get("entries") or [])
    total, pad = page_plan(len(entries))
    parts = [
        _content_preamble(book),
        r"\begin{document}",
        # p1: title page (recto)
        r"\pagecolor{pagebg}\color{bodytext}",
        r"\vspace*{103mm}",
        r"\begin{center}",
        r"{\displayfont\fontsize{44}{50}\selectfont %s\par}" % tex_escape(book.get("title") or book.get("name") or "PolyPaint"),
    ]
    subtitle = str(book.get("subtitle") or "").strip()
    if subtitle:
        parts.append(r"\vspace{10mm}{\normalsize %s\par}" % tex_escape(subtitle))
    author = str(book.get("author") or "").strip()
    if author:
        parts.append(r"\vspace{6mm}{\small %s\par}" % tex_escape(author))
    parts.append(r"\end{center}")
    if pdf_url:
        # same chip idiom as the verso QRs (dark modules on a light chip,
        # 1.5mm quiet zone), same 12mm size
        parts.extend([
            r"\vfill",
            r"\begin{center}",
            r"{\setlength{\fboxsep}{1.5mm}"
            r"\colorbox{qrbg}{\color{qrfg}\qrcode[height=14mm,level=M]{%s}}\par}" % pdf_url,
            r"\vspace{2.5mm}",
            r"{\monofont\footnotesize\color{monotext} download pdf\par}",
            r"\end{center}",
            r"\vspace*{14mm}",
        ])
    layout = book_spread_layout(book)
    for entry in entries:
        prov = provenance_by_entry.get(entry.get("entry_id") or "")
        report = (prov or {}).get("report") if isinstance(prov, dict) else None
        has_palette = bool((report or {}).get("has_palette"))
        swapped = layout == "palette_primary" and has_palette
        parts.append(_verso_report_page(entry, prov, layout))
        recto_asset = (f"{entry.get('entry_id')}.palette.jpg" if swapped
                       else f"{entry.get('entry_id')}.jpg")
        parts.append(_full_bleed_image(f"{ASSET_DIR}/{recto_asset}"))
    for _ in range(pad):
        parts.append(_background_page())
    parts.append(r"\end{document}")
    return "\n".join(parts) + "\n", total


def render_cover_tex(book, cover_asset_rel=None, content_pages=None):
    """Emit one page-count-specific back|spine|front cover PDF source."""
    if content_pages is None:
        content_pages, _ = page_plan(len(book.get("entries") or []))
    geometry = cover_geometry(content_pages)
    panel_mm = geometry["panel_mm"]
    spine_mm = geometry["spine_mm"]
    front_center_mm = (spine_mm + panel_mm) / 2
    title = tex_escape(book.get("title") or book.get("name") or "PolyPaint")
    colors = _book_color_scheme(book)
    parts = [
        r"\documentclass{article}",
        r"\usepackage[paperwidth=%smm, paperheight=%smm, margin=0mm]{geometry}"
        % (_format_mm(geometry["width_mm"]), _format_mm(geometry["height_mm"])),
        r"\usepackage{graphicx}",
        r"\usepackage{xcolor}",
        r"\usepackage{tikz}",
        r"\usetikzlibrary{calc}",
        _font_setup(),
        r"\pagestyle{empty}",
        r"\definecolor{pagebg}{HTML}{%s}" % colors["pagebg"],
        r"\definecolor{bodytext}{HTML}{%s}" % colors["bodytext"],
        r"\begin{document}",
        r"\pagecolor{pagebg}\color{bodytext}",
        r"\begin{tikzpicture}[remember picture, overlay]",
        # spine text, centered on the spread
        r"\node[rotate=90, text=bodytext] at ($(current page.center)$) {\displayfont %s};" % title,
    ]
    # front panel: right half; image at 2/3 panel width above the title
    if cover_asset_rel:
        parts.append(
            r"\node[anchor=center] at ($(current page.center)+(%.1fmm, 20mm)$)"
            r" {\includegraphics[width=%.1fmm]{%s}};"
            % (front_center_mm, panel_mm * 2 / 3, cover_asset_rel))
    parts.append(
        r"\node[anchor=center, text=bodytext] at ($(current page.center)+(%.1fmm, -110mm)$)"
        r" {\displayfont\fontsize{36}{40}\selectfont %s};"
        % (front_center_mm, title))
    parts.extend([
        r"\end{tikzpicture}",
        r"\null",
        r"\end{document}",
    ])
    return "\n".join(parts) + "\n"
