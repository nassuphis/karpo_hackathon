#!/usr/bin/env python3
"""Generate the portable giga_2872 Coeff Program document.

The roots are the 32 lit pixels of CP437 character 178 (the medium-shade
block) from the era's letters.py FONT dictionary — itself transcribed from
fonts/3dfx8x8.png by pngfont2pydict.py. The program translates that pooled
constellation per row and applies zfrm.andy1 in its always-observed collapsed
form (see recreate_giga_2872.md sections 1.4-1.5 and 8). Parity against the
verbatim historical formula is pinned by tests/test_giga_2872_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2872.coeff-program.json"

# letters.py at 500685f, key 'b178' — a perfect checkerboard.
B178 = [
    0b10101010,
    0b01010101,
    0b10101010,
    0b01010101,
    0b10101010,
    0b01010101,
    0b10101010,
    0b01010101,
]


def letter_roots() -> list[complex]:
    """get_letter_coordinates at 500685f: bit (x, y) -> (x-3.5, (7-y)-3.5),
    factor = 1, row-major scan with bit 7 leftmost."""
    coords = []
    for y, row in enumerate(B178):
        for x in range(8):
            if row & (1 << (7 - x)):
                coords.append(complex(x - 3.5, (7 - y) - 3.5))
    return coords


def _number(value: float) -> str:
    normalized = 0.0 if value == 0.0 else float(value)
    return format(normalized, ".17g")


def _complex(value: complex) -> str:
    real = 0.0 if value.real == 0.0 else value.real
    imag = 0.0 if value.imag == 0.0 else value.imag
    if imag == 0.0:
        return _number(real)
    if real == 0.0:
        return f"{_number(imag)}i"
    sign = "+" if imag >= 0.0 else ""
    return f"{_number(real)}{sign}{_number(imag)}i"


def build_source_text() -> str:
    roots = letter_roots()
    if len(roots) != 32:
        raise RuntimeError("b178 must contain exactly 32 lit pixels")
    rows = ["roots_literal("]
    rows.extend(f"    {_complex(value)}," for value in roots[:-1])
    rows.append(f"    {_complex(roots[-1])}")
    rows.append(")")
    roots_call = "\n".join(rows)
    return "\n".join(
        [
            f"poly = translate_roots({roots_call}, (0.05+0.9*t1) + 1i*(0.05+0.9*t2))",
            "poly",
            "poly = arange(0, 33)",
            "poly = argsort(poly, peek)",
            "poly",
            "poly = scan(33, 0, tos[0], prev + tos[k])",
            "poly = rem(poly, 33)",
            "drop",
            "poly = scan(33, 0, tos[poly[k]], tos[poly[k]])",
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
