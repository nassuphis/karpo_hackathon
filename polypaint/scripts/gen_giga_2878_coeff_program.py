#!/usr/bin/env python3
"""Generate the portable giga_2878 Coeff Program document.

giga_2878 = the giga_2877 constellation (glyph b178 with the ro/io
offset -0.5-0.5i, snapshot 8e28adb) under zfrm sort_abs_p, the gentlest
of the _p admixture family:

    q = sort_abs(cf) * andy + cf          (andy = 0.075)

No root-finder, no append, no pairing dice: the sorted vector is a
deterministic |.|-ascending permutation of cf itself, so the whole
transform is sort_abs + one broadcast multiply + one add — 32 tokens,
one pool constant. The leading is q[0] = 1 + 0.075*s0 with |s0| <= 1
(the minimum-|.| coefficient can't beat the monic 1 by more than 1), so
the degree holds at 32 on every row. Only the fully symmetric delta=0
row t=(0.5,0.5) — which with the ro/io centering also carries an exact
origin root — has exact |cf| ties where sort order is legitimately free.

Parity is pinned by tests/test_giga_2878_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2878.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

DELTA_TEXT = "((0.05+0.9*t1)+(0-0.5)) + 1i*((0.05+0.9*t2)+(0-0.5))"


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
            f"poly = translate_roots(roots_ascii_literal(178), {DELTA_TEXT})",
            "poly",
            "poly = sort_abs(poly)",
            "poly = multiply(poly, 0.075)",
            "poly = add(pop, poly)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2878",
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
                "FATAL: giga_2878.coeff-program.json is stale; run "
                "scripts/gen_giga_2878_coeff_program.py"
            )
        print("giga_2878.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
