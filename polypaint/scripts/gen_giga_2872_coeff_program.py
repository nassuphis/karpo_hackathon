#!/usr/bin/env python3
"""Generate the portable giga_2872 Coeff Program document.

The roots are the 32 lit pixels of font glyph b178 (CP437 sheet cell 177,
the medium shade) via roots_ascii_literal, whose checked-in table
(lambda/cp437_font.py) replicates the era's pngfont2pydict.py transcription
bit for bit. The program translates that pooled constellation per row and
applies zfrm.andy1 in its always-observed collapsed form (see
recreate_giga_2872.md sections 1.4-1.5 and 8). The roots_ascii_literal(178)
spelling compiles to the IDENTICAL pool constant and fingerprint as the
explicit 32-root listing it replaced. Parity against the verbatim historical
formula is pinned by tests/test_giga_2872_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2872.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))


def letter_roots() -> list[complex]:
    """Glyph b178's lit pixels via the checked-in table, read exactly as
    letters.get_letter_coordinates did: bit (x, y) -> (x-3.5, (7-y)-3.5)."""
    from cp437_font import FONT_ROWS

    coords = []
    for y, row in enumerate(FONT_ROWS[178]):
        for x in range(8):
            if row & (1 << (7 - x)):
                coords.append(complex(x - 3.5, (7 - y) - 3.5))
    return coords


def build_source_text() -> str:
    if len(letter_roots()) != 32:
        raise RuntimeError("b178 must contain exactly 32 lit pixels")
    return "\n".join(
        [
            "poly = translate_roots(roots_ascii_literal(178), (0.05+0.9*t1) + 1i*(0.05+0.9*t2))",
            "poly",
            "poly = arange(0, poly_len)",
            "poly = argsort(poly, peek)",
            "poly",
            "poly = scan(poly_len, 0, tos[0], prev + tos[k])",
            "poly = rem(poly, poly_len)",
            "drop",
            "poly = scan(poly_len, 0, tos[poly[k]], tos[poly[k]])",
            "drop",
            "poly = multiply(poly, 0-1)",
            "poly = add(poly, 1)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2872",
        "chain": [],
        "source_text": build_source_text(),
    }


def _render(payload: dict) -> str:
    return json.dumps(payload, indent=2, ensure_ascii=True) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    rendered = _render(build_payload())
    if args.check:
        current = OUTPUT.read_text(encoding="utf-8") if OUTPUT.exists() else ""
        if current != rendered:
            raise SystemExit(
                "FATAL: giga_2872.coeff-program.json is stale; run "
                "scripts/gen_giga_2872_coeff_program.py"
            )
        print("giga_2872.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
