#!/bin/bash
# Book Maker docker gate (book-maker-design.md §10): build the book_pdf image
# and compile a 3-entry fixture book inside it. Asserts page count, MediaBox
# dims, and embedded font streams — the WhiteWall preflight in miniature.
# Required whenever the image or the tex templates change (sweep_cli.c rule).
set -euo pipefail
cd "$(dirname "$0")/.."

IMG=polypaint-book-pdf:gate
docker buildx build --platform linux/arm64 -f lambda/book_pdf.Dockerfile -t "$IMG" --load .

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

cat > "$WORK/fixture.py" <<'PYEOF'
import re
import subprocess
import sys

sys.path.insert(0, "/var/task")
import book_tex
from PIL import Image

BOOK = {
    "name": "Gate Fixture",
    "title": "Gate & Fixture 100%",   # escaping must survive compile
    "subtitle": "three entries",
    "entries": [
        {"entry_id": f"e{i}", "job_id": f"j{i}", "artifact_id": f"a{i}",
         "image_key": f"renders/j{i}/color/a{i}/image.jpeg",
         "title_override": "" if i else "First $tudy"}
        for i in range(3)
    ],
}
# WORST CASE on purpose: the maximum row count a real artifact produces
# (10 rows incl. Interpretation/Output channels) + a square palette. The
# page-count assertion below fails if the verso ever overflows again.
PROV = {"e1": {"report": {
    "compute_id": "compute_mr7kkhg2", "artifact_id": "color_run_1783245470391_uovu3x",
    "summary_rows": [["Function", "const(2,0,0)"], ["Degree", "50"], ["N", "10000"],
                     ["Times", "1"], ["Solver", "AE-MT"], ["Interpretation", "scalar_lut"],
                     ["Color mode", "root proximity"],
                     ["Palette", "long_washington_stripe_teal_orange_11"],
                     ["Output channels", "1"],
                     ["Viewport", "explicit, re [-3.4, -1.3], im [-0.95, 0.93]"]],
    "palette_label": "pal_color_run_1783245470391_uovu3x", "has_palette": True,
}}}

import os
os.makedirs("/build/assets", exist_ok=True)
# palette swatch for the report page (verifies the \includegraphics panel)
Image.new("RGB", (1600, 1600), (60, 20, 90)).save("/build/assets/e1.palette.jpg", quality=90)

# exercise the vips prepare path end to end (design uses it to resize
# sources to <=3600px). A broken vips layer raises here rather than
# silently degrading, so this asserts vipsthumbnail + its .so's work.
import spread_pdf
big = "/build/big_source.png"
Image.new("RGB", (5000, 5000), (12, 40, 80)).save(big)
for i in range(3):
    info = spread_pdf.prepare_pdf_image(big, f"/build/assets/e{i}.jpg",
                                        max_px=3600, quality=90, image_format="jpeg")
    assert max(info["prepared_width"], info["prepared_height"]) == 3600, info
assert spread_pdf._vipsthumbnail_path(), "vipsthumbnail not on PATH in image"
content, expected_pages = book_tex.render_content_tex(
    BOOK, PROV,
    pdf_url=book_tex.S3_PUBLIC_BASE + "books/gate/out/cmp_gate/content.pdf")
assert "\\qrcode[height=14mm,level=M]{https://" in content.split("\\newpage", 1)[0], \
    "title-page QR missing from gate compile"
cover = book_tex.render_cover_tex(BOOK, "assets/e1.jpg")
# test-only: keep objects uncompressed so the assertions can grep the PDF
prologue = "\\pdfvariable objcompresslevel 0\n\\pdfvariable compresslevel 0\n"
open("/build/book.tex", "w").write(prologue + content)
open("/build/cover.tex", "w").write(prologue + cover)

for name in ("book", "cover"):
    for _ in range(2):
        run = subprocess.run(
            ["lualatex", "-interaction=nonstopmode", "-halt-on-error", f"{name}.tex"],
            cwd="/build", capture_output=True, text=True)
        if run.returncode != 0:
            log = open(f"/build/{name}.log").read()
            print(log[-3000:])
            sys.exit(f"{name}.tex failed")

log = open("/build/book.log").read()
m = re.search(r"Output written on book\.pdf \((\d+) page", log)
assert m, "no page count in log"
assert int(m.group(1)) == expected_pages, (m.group(1), expected_pages)

# flipbook rasterization (flipbook.md §5.1): poppler-utils must be in the
# image, and a 120dpi page must come out at the geometry the viewer assumes
import struct
import time as _time
t0 = _time.time()
fr = subprocess.run(["pdftoppm", "-png", "-r", "200",
                     "-f", "1", "-l", "1", "/build/book.pdf", "/build/flip_page"],
                    capture_output=True, text=True)
assert fr.returncode == 0, f"pdftoppm failed: {fr.stderr[:300]}"
flip_files = [f for f in os.listdir("/build") if f.startswith("flip_page-")]
assert flip_files, "pdftoppm produced no output"
pimg = Image.open(f"/build/{flip_files[0]}").convert("RGB")
pimg.save("/build/flip_page.jpg", format="JPEG", quality=88, subsampling=0, optimize=True)
fp = open("/build/flip_page.jpg", "rb").read()
assert fp[:2] == b"\xff\xd8", "flip page is not a JPEG"
i = 2
fw = fh_ = 0
while i + 9 < len(fp):
    if fp[i] != 0xFF:
        i += 1
        continue
    marker = fp[i + 1]
    if 0xC0 <= marker <= 0xCF and marker not in (0xC4, 0xC8, 0xCC):
        fh_, fw = struct.unpack(">HH", fp[i + 5:i + 9])
        break
    i += 2 + struct.unpack(">H", fp[i + 2:i + 4])[0]
assert abs(fw - 2308) <= 3 and abs(fh_ - 2332) <= 3, f"flip page {fw}x{fh_}, want ~2308x2332"
print(f"flipbook page: {fw}x{fh_} jpg in {_time.time() - t0:.2f}s")

pdf = open("/build/book.pdf", "rb").read()
# 293x296mm at 72.27pt/in: 830.4 x 838.9 bp — lualatex writes bp with 5 dp
assert re.search(rb"/MediaBox\s*\[[^\]]*830\.\d+\s+83[89]\.\d+", pdf), "content MediaBox wrong"
assert b"FontFile2" in pdf or b"FontFile3" in pdf or b"FontFile" in pdf, \
    "fonts not embedded (WhiteWall hard requirement)"

cpdf = open("/build/cover.pdf", "rb").read()
assert re.search(rb"/MediaBox\s*\[[^\]]*178[23]\.\d+\s+89[56]\.\d+", cpdf), "cover MediaBox wrong"
assert b"FontFile" in cpdf, "cover fonts not embedded"

print(f"GATE OK: {expected_pages} pages, MediaBoxes + FontFile streams verified")
PYEOF

docker run --rm --platform linux/arm64 --entrypoint python \
    --read-only --tmpfs /tmp:exec,size=2g \
    -v "$WORK":/fixture -v "$WORK/build":/build "$IMG" /fixture/fixture.py

echo "=== Book Docker Gate PASSED ==="
