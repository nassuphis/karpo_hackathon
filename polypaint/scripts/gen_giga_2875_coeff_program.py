#!/usr/bin/env python3
"""Generate the portable giga_2875 Coeff Program document.

giga_2875 = the giga_2871 chain (translate + sort_abs) with giga_2874's
jump xfrm in front, at double strength: jt = t + count(levels > t) * jmp
with levels = [0.1..0.9] and jmp = -0.1 (deltas reach -0.76). The counts
use vector gt (strict real >) exactly as in giga_2874 — see
recreate_giga_2875.md and recreate_giga_2874.md §4 for why floor()
spellings are wrong and how the counts bridge into the translate delta
(in-poly cumsum + sparse poke poly[0] = tos[8]).

Unlike giga_2871, the sorted leading is NOT always the monic 1: the
strong jump breaks that regime on 59/20000 seeded rows (|cf|min down to
~0.156, never exactly zero — the staircase's image has gaps, so no
reachable delta puts a root exactly at the origin). sort_abs is verbatim
either way; only the test oracle distinguishes the regimes.

The roots are the 32 lit pixels of font glyph b2 (CP437 sheet cell 1,
the smiley, mirrored per the historical transcription quirks) via
roots_ascii_literal. Parity is pinned by
tests/test_giga_2875_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2875.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

LEVELS_TEXT = "0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9"


def letter_roots() -> list[complex]:
    """Glyph b2's lit pixels via the checked-in table, read exactly as
    letters.get_letter_coordinates did: bit (x, y) -> (x-3.5, (7-y)-3.5)."""
    from cp437_font import FONT_ROWS

    coords = []
    for y, row in enumerate(FONT_ROWS[2]):
        for x in range(8):
            if row & (1 << (7 - x)):
                coords.append(complex(x - 3.5, (7 - y) - 3.5))
    return coords


def build_source_text() -> str:
    if len(letter_roots()) != 32:
        raise RuntimeError("b2 must contain exactly 32 lit pixels")
    return "\n".join(
        [
            # --- xfrm = jump: level counts for t1 and t2 ---------------
            "fill(9, t1)",
            f"poly = vector_literal({LEVELS_TEXT})",
            "poly = gt(poly, pop)",
            "poly = scan(9, 0, poly[0], prev + poly[k])",
            "poly",
            "fill(9, t2)",
            f"poly = vector_literal({LEVELS_TEXT})",
            "poly = gt(poly, pop)",
            "poly = scan(9, 0, poly[0], prev + poly[k])",
            "poly[0] = tos[8]",
            "drop",
            # poly[0] = count(levels > t1), poly[8] = count(levels > t2)
            "poly = translate_roots(roots_ascii_literal(2),"
            " (0.05+0.9*(t1+(0-0.1)*poly[0]))"
            " + 1i*(0.05+0.9*(t2+(0-0.1)*poly[8])))",
            "poly = sort_abs(poly)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2875",
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
                "FATAL: giga_2875.coeff-program.json is stale; run "
                "scripts/gen_giga_2875_coeff_program.py"
            )
        print("giga_2875.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
