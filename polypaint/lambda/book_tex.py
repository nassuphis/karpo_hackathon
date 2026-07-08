"""Book Maker LaTeX templates (book-maker-design.md §6).

Pure string templating: the compose op renders book.tex / cover.tex from the
book document + provenance snapshots, then runs lualatex. Everything here is
testable without a TeX installation.

Geometry (verified against make_polypaint_book.py:246-262, computed values —
the script's own comments are wrong):
  content page: 290 mm net -> 293 x 296 mm gross (bleed on 3 sides, none at
  the binding edge); cover: 609 net + 2x10 bleed -> 629 x 316 mm gross,
  panel (609-11)/2 = 299 mm.

Page plan: front matter must be an ODD page count so entry text lands on
versos (left) and images on rectos (right) of the same opening
(make_polypaint_book.py:905-916 is the reference). p1 = title recto, then
2 pages per entry, then black pads to a multiple of 4.
"""

CONTENT_W_MM = 293
CONTENT_H_MM = 296
COVER_W_MM = 629
COVER_H_MM = 316
COVER_PANEL_MM = 299
COVER_SPINE_MM = 11

# The compose build dir lays prepared images out under assets/.
ASSET_DIR = "assets"

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


def _content_preamble():
    # Colours ported from spread_pdf.py so the book verso matches the
    # ColorSpread PDF report page (deep blue that matches the app).
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
        _font_setup(),
        r"\pagestyle{empty}",
        r"\setlength{\parindent}{0pt}",
        r"\definecolor{pagebg}{HTML}{1A1A2E}",   # PAGE_BG
        r"\definecolor{accent}{HTML}{E94560}",   # ACCENT
        r"\definecolor{bodytext}{HTML}{F2F2F7}", # TEXT
        r"\definecolor{monotext}{HTML}{9AA0B4}", # MUTED
        r"\definecolor{panelbg}{HTML}{121829}",  # PANEL_BG
        r"\definecolor{panelborder}{HTML}{2B3A5E}",  # PANEL_BORDER
        r"\definecolor{rulecol}{HTML}{5F6678}",  # RULE
    ])


def _black_page():
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
    """Verso heading: an override if set, else the compute id (the
    compute/color_run id is 'good enough for now' — no artsy title yet)."""
    override = str(entry.get("title_override") or "").strip()
    if override:
        return override
    report = (provenance or {}).get("report") or {}
    return str(report.get("compute_id") or entry.get("job_id") or entry.get("artifact_id") or "")


def _report_rows(entry, provenance):
    report = (provenance or {}).get("report") or {}
    rows = report.get("summary_rows") or []
    return [(str(r[0]), str(r[1])) for r in rows if isinstance(r, (list, tuple)) and len(r) == 2]


PALETTE_MAX_MM = 160
PALETTE_MIN_MM = 120


def _palette_mm(rows, body_override):
    """Palette square sized to what the verso's content leaves over — the
    one-page spread invariant is sacred. With vmargin=1mm the text block is
    294mm, so the full 160mm fits every realistic verso (10 rows + a body
    paragraph); the shrink only guards pathological content. Budget (mm):
    294 block − ~30 title zone (6mm start + 26/30pt title + rule + artifact
    + gap) − 6.5/row − 5.5/body line − 16 palette chrome − 8 safety."""
    body_lines = 0
    for line in str(body_override or "").strip().splitlines():
        body_lines += max(1, (len(line) + 84) // 85)
    avail = 294 - 30 - 6.5 * len(rows) - 5.5 * body_lines - (4 if body_lines else 0) - 16 - 8
    return int(max(PALETTE_MIN_MM, min(PALETTE_MAX_MM, avail)))


def _verso_report_page(entry, provenance):
    """Deep-blue report page matching the ColorSpread PDF: title, accent
    rule, artifact id, fixed-pitch KV rows, and the palette at a FIXED
    120x120mm in a centered table. Everything is fixed-size, so the worst
    case (10 rows) totals ~230mm < the 248mm text block: the page can
    never overflow. Plain LaTeX, no overlay tricks."""
    report = (provenance or {}).get("report") or {}
    title = tex_escape(entry_title(entry, provenance))
    artifact = tex_escape(str(report.get("artifact_id") or entry.get("artifact_id") or ""))
    rows = _report_rows(entry, provenance)
    body_override = str(entry.get("body_override") or "").strip()

    parts = [
        r"\newpage",
        r"\pagecolor{pagebg}\color{bodytext}",
        # explicit vertical placement (vmargin=1mm): cap height lands ~10mm
        # below the physical page top
        r"\vspace*{6mm}",
        r"{\displayfont\fontsize{26}{30}\selectfont %s\par}" % (title or "PolyPaint"),
        r"\vspace{2.5mm}",
        r"{\color{accent}\rule{\linewidth}{0.8pt}}\par",
        r"\vspace{1.5mm}",
        r"{\monofont\footnotesize\color{monotext} %s\par}" % artifact,
        r"\vspace{7mm}",
    ]
    # fixed 6.5mm-pitch rows (reference PDF: KV_PITCH = 17pt)
    for label, value in rows:
        parts.append(
            r"\noindent\hbox to \linewidth{\hbox to 42mm{\monofont\footnotesize\color{monotext} %s\hss}"
            r"{\color{bodytext} %s}\hss}\vspace*{\dimexpr6.5mm-\baselineskip\relax}\par"
            % (tex_escape(label.upper()), tex_escape(value)))
    if body_override:
        parts.append(r"\vspace{4mm}")
        for line in body_override.splitlines():
            parts.append(r"{\normalsize %s\par}" % tex_escape(line))

    if report.get("has_palette"):
        palette_label = tex_escape(str(report.get("palette_label") or "palette"))
        parts.extend([
            r"\vfill",
            r"\begin{center}",
            r"\begin{tabular}{c}",
            r"\fcolorbox{panelborder}{panelbg}{\includegraphics[width=%dmm,height=%dmm,keepaspectratio]{%s/%s.palette.jpg}} \\[2mm]"
            % (_palette_mm(rows, body_override), _palette_mm(rows, body_override),
               ASSET_DIR, entry.get("entry_id")),
            r"{\monofont\footnotesize\color{monotext} %s} \\" % palette_label,
            r"\end{tabular}",
            r"\end{center}",
            r"\vfill",
        ])
    parts.append(r"\null")
    return "\n".join(parts)


def render_content_tex(book, provenance_by_entry=None):
    """Emit the content PDF source: title page + one spread per entry + pads."""
    provenance_by_entry = provenance_by_entry or {}
    entries = list(book.get("entries") or [])
    total, pad = page_plan(len(entries))
    parts = [
        _content_preamble(),
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
    for entry in entries:
        prov = provenance_by_entry.get(entry.get("entry_id") or "")
        parts.append(_verso_report_page(entry, prov))
        parts.append(_full_bleed_image(f"{ASSET_DIR}/{entry.get('entry_id')}.jpg"))
    for _ in range(pad):
        parts.append(_black_page())
    parts.append(r"\end{document}")
    return "\n".join(parts) + "\n", total


def render_cover_tex(book, cover_asset_rel=None):
    """Emit the cover PDF source: one 629x316 mm page, back|spine|front."""
    title = tex_escape(book.get("title") or book.get("name") or "PolyPaint")
    parts = [
        r"\documentclass{article}",
        r"\usepackage[paperwidth=%dmm, paperheight=%dmm, margin=0mm]{geometry}"
        % (COVER_W_MM, COVER_H_MM),
        r"\usepackage{graphicx}",
        r"\usepackage{xcolor}",
        r"\usepackage{tikz}",
        r"\usetikzlibrary{calc}",
        _font_setup(),
        r"\pagestyle{empty}",
        r"\begin{document}",
        r"\pagecolor{black}\color{white}",
        r"\begin{tikzpicture}[remember picture, overlay]",
        # spine text, centered on the spread
        r"\node[rotate=90, text=white] at ($(current page.center)$) {\displayfont %s};" % title,
    ]
    # front panel: right half; image at 2/3 panel width above the title
    if cover_asset_rel:
        parts.append(
            r"\node[anchor=center] at ($(current page.center)+(%.1fmm, 20mm)$)"
            r" {\includegraphics[width=%.1fmm]{%s}};"
            % (COVER_SPINE_MM / 2 + COVER_PANEL_MM / 2, COVER_PANEL_MM * 2 / 3, cover_asset_rel))
    parts.append(
        r"\node[anchor=center] at ($(current page.center)+(%.1fmm, -110mm)$)"
        r" {\displayfont\fontsize{36}{40}\selectfont %s};"
        % (COVER_SPINE_MM / 2 + COVER_PANEL_MM / 2, title))
    parts.extend([
        r"\end{tikzpicture}",
        r"\null",
        r"\end{document}",
    ])
    return "\n".join(parts) + "\n"
