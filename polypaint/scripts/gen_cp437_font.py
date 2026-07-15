#!/usr/bin/env python3
"""Generate lambda/cp437_font.py from the reference font sheet.

This replicates pyroots' pngfont2pydict.py (at 500685f) BIT FOR BIT —
including its two quirks, which are part of the historical record and must
never be "fixed" here or roots_ascii_literal stops matching what
poly_letter actually produced:

1. Rows are packed LSB-leftmost (`byte |= bit << x_offset`) while the
   consumer reads MSB-leftmost (`row & (1 << (7 - x))`): every glyph is
   horizontally MIRRORED relative to the sheet.
2. Keys are off by one: dict key b<N> holds sheet cell N-1 (16 chars per
   row, 9px cell pitch, 8x8 sampled at x offset +1). The saved giga_2872
   state's `ascii: 178` therefore renders CP437 glyph 177 (the medium
   shade), whose mirror happens to be indistinguishable from itself.

Generation is a development-time operation: it reads the read-only
reference PNG only when regenerating. Production imports the checked-in
module and never touches /Users/nicknassuphis/pyroots.
"""
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "lambda" / "cp437_font.py"
DEFAULT_SHEET = "/Users/nicknassuphis/pyroots/fonts/3dfx8x8.png"


def transcribe(image_path: str) -> tuple[dict[int, list[int]], str]:
    from PIL import Image

    digest = hashlib.sha256(Path(image_path).read_bytes()).hexdigest()
    img = Image.open(image_path).convert("1")
    font = {}
    for char_index in range(256):
        row = char_index // 16
        col = char_index % 16
        x_start = col * 9
        y_start = row * 9
        bitmap = []
        for y_offset in range(8):
            byte = 0
            for x_offset in range(8):
                x = x_start + x_offset + 1
                y = y_start + y_offset
                pixel = img.getpixel((x, y))
                bit = 0 if pixel == 0 else 1
                byte |= bit * 2 ** x_offset
            bitmap.append(byte)
        font[char_index + 1] = bitmap
    return font, digest


def render_module(font: dict[int, list[int]], digest: str) -> str:
    lines = [
        '"""CP437 font rows for roots_ascii_literal — GENERATED, do not edit.',
        "",
        "Transcribed from fonts/3dfx8x8.png (sha256 %s)" % digest,
        "by scripts/gen_cp437_font.py, replicating the historical",
        "pngfont2pydict.py exactly (LSB-leftmost packing + b<N> = sheet cell",
        "N-1). Consumers read pixels with `row & (1 << (7 - x))`, matching",
        "letters.get_letter_coordinates.",
        '"""',
        "",
        "SHEET_SHA256 = %r" % digest,
        "",
        "FONT_ROWS = {",
    ]
    for code in sorted(font):
        rows = ", ".join(f"0b{value:08b}" for value in font[code])
        lines.append(f"    {code}: ({rows}),")
    lines.append("}")
    lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--sheet", default=DEFAULT_SHEET,
                        help="reference font sheet (read-only; regeneration only)")
    args = parser.parse_args()
    font, digest = transcribe(args.sheet)
    rendered = render_module(font, digest)
    if args.check:
        current = OUTPUT.read_text(encoding="utf-8") if OUTPUT.exists() else ""
        if current != rendered:
            raise SystemExit(
                "FATAL: lambda/cp437_font.py is stale; run scripts/gen_cp437_font.py"
            )
        print("lambda/cp437_font.py: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT} ({len(font)} glyphs)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
