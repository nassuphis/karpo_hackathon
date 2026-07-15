#!/usr/bin/env python3
"""Generate the portable giga_2871 Coeff Program document.

The checkerboard glyph (b178) translated per row, then zfrm.sort_abs —
coefficients reordered by ascending magnitude. The monic leading term is
the magnitude minimum for every observed row, so the sorted polynomial
stays monic degree 32 (see recreate_giga_2871.md). Parity is pinned by
tests/test_giga_2871_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2871.coeff-program.json"
sys.path.insert(0, str(ROOT / "lambda"))


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
        "name": "giga_2871",
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
                "FATAL: giga_2871.coeff-program.json is stale; run "
                "scripts/gen_giga_2871_coeff_program.py"
            )
        print("giga_2871.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
