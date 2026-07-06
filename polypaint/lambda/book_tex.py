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
    return "\n".join([
        r"\usepackage{fontspec}",
        r"\setmainfont{TiemposText-Regular-Trial.ttf}",
        r"\newfontfamily\displayfont{Canela-Regular-Trial.ttf}",
        r"\newfontfamily\monofont{CourierPrime-Regular.ttf}",
        r"\usepackage{microtype}",
    ])


def _content_preamble():
    return "\n".join([
        r"\documentclass{article}",
        r"\usepackage[paperwidth=%dmm, paperheight=%dmm, margin=24mm]{geometry}"
        % (CONTENT_W_MM, CONTENT_H_MM),
        r"\usepackage{graphicx}",
        r"\usepackage{xcolor}",
        r"\usepackage{eso-pic}",
        _font_setup(),
        r"\pagestyle{empty}",
        r"\definecolor{pagebg}{HTML}{000000}",
        r"\definecolor{bodytext}{HTML}{F2F2F7}",
        r"\definecolor{monotext}{HTML}{9AA0B4}",
    ])


def _black_page():
    return "\n".join([
        r"\newpage",
        r"\pagecolor{black}",
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
    override = str(entry.get("title_override") or "").strip()
    if override:
        return override
    fn = str(((provenance or {}).get("summary") or {}).get("function") or "").strip()
    return f"Study {fn}" if fn else str(entry.get("display_name") or entry.get("artifact_id") or "")


def entry_body_lines(entry, provenance):
    override = str(entry.get("body_override") or "").strip()
    if override:
        return override.splitlines()
    lines = []
    summary = (provenance or {}).get("summary") or {}
    for key in ("pipeline", "scale", "coloring", "solver"):
        value = str(summary.get(key) or "").strip()
        if value:
            lines.append(value)
    return lines


def _verso_text_page(entry, provenance):
    title = tex_escape(entry_title(entry, provenance))
    body = [tex_escape(line) for line in entry_body_lines(entry, provenance)]
    meta_line = tex_escape(f"{entry.get('job_id', '')} / {entry.get('artifact_id', '')}")
    parts = [
        r"\newpage",
        r"\pagecolor{black}\color{bodytext}",
        r"\vspace*{40mm}",
        r"{\displayfont\Huge %s\par}" % title,
        r"\vspace{12mm}",
    ]
    for line in body:
        parts.append(r"{\normalsize %s\par}" % line)
    parts.extend([
        r"\vfill",
        r"{\monofont\small\color{monotext} %s\par}" % meta_line,
        r"\vspace*{18mm}",
    ])
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
        r"\pagecolor{black}\color{bodytext}",
        r"\vspace*{80mm}",
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
        parts.append(_verso_text_page(entry, prov))
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
