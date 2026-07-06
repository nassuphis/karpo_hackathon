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
PROV = {"e1": {"summary": {"function": "poly_42", "pipeline": "[ ]  poly_42  [ ]",
                           "coloring": "root proximity", "solver": "Solved by AE"}}}

import os
os.makedirs("/build/assets", exist_ok=True)

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
content, expected_pages = book_tex.render_content_tex(BOOK, PROV)
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
