#!/usr/bin/env python3
"""Generate the giga_2877 pairing-variant Coeff Program documents.

giga_2877's ring texture is decided by the PAIRING — which root of
np.roots(cf) lands on which coefficient slot in
q = 100*(roots ++ [1]) + cf. The reference used LAPACK zgeev's emission
order (irreproducible; see recreate_giga_2877.md). The main recreation
(giga_2877) uses descending |root|, the measured-closest rule (cloud
corr 0.934). These two variants keep everything else identical and swap
only the pairing stage — three artworks from one formula:

- giga_2877_v2 — POOL ORDER (no sort at all): each row pairs the same
  way, so the ring collapses into smooth coherent petals.
  Cloud corr vs the LAPACK ensemble: 0.655.
- giga_2877_v3 — CHAOTIC KEY: argsort of frac(1e5 * |root|), a
  deterministic hash-like key that re-dices the permutation row to row.
  The ensemble washes the shards into a diffuse fuzz halo.
  Cloud corr vs a true random-per-row ensemble: 0.956 (noise floor).

Parity for both is pinned by tests/test_giga_2877_variants.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

DELTA_TEXT = "((0.05+0.9*t1)+(0-0.5)) + 1i*((0.05+0.9*t2)+(0-0.5))"
COMBINE_TEXT = (
    "poly = scan(poly_len, 0, poly[0] + 100*tos[0],"
    " poly[k] + 100*(tos[k-32*floor(k/32)]*(1-floor(k/32))"
    " + floor(k/32)))"
)
KEY_TEXT = "100000*abs(tos[{i}]) - floor(100000*abs(tos[{i}]))"


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


def _point_literal(z: complex) -> str:
    text = f"{z.real:g}"
    text += f"+{z.imag:g}i" if z.imag >= 0 else f"{z.imag:g}i"
    return text


def _points_text() -> str:
    points = letter_roots()
    if len(points) != 32:
        raise RuntimeError("b178 must contain exactly 32 lit pixels")
    return ", ".join(_point_literal(p) for p in points)


def build_source_text_v2() -> str:
    return "\n".join(
        [
            f"poly = vector_literal({_points_text()})",
            f"poly = add(poly, {DELTA_TEXT})",
            # pool-order pairing: no sort stage at all
            "poly",
            f"poly = translate_roots(roots_ascii_literal(178), {DELTA_TEXT})",
            COMBINE_TEXT,
            "drop",
            "emit",
        ]
    )


def build_source_text_v3() -> str:
    return "\n".join(
        [
            f"poly = vector_literal({_points_text()})",
            f"poly = add(poly, {DELTA_TEXT})",
            "poly",
            # chaotic pairing: permute by the hash-like key frac(1e5*|r|)
            f"poly = scan(32, 0, {KEY_TEXT.format(i='0')},"
            f" {KEY_TEXT.format(i='k')})",
            "poly = argsort(pop, poly)",
            "poly",
            f"poly = translate_roots(roots_ascii_literal(178), {DELTA_TEXT})",
            COMBINE_TEXT,
            "drop",
            "emit",
        ]
    )


VARIANTS = {
    "giga_2877_v2": build_source_text_v2,
    "giga_2877_v3": build_source_text_v3,
}


def build_payload(name: str) -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": name,
        "chain": [],
        "source_text": VARIANTS[name](),
    }


def _render(payload: dict) -> str:
    return json.dumps(payload, indent=2, ensure_ascii=True) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    for name in VARIANTS:
        output = ROOT / f"{name}.coeff-program.json"
        rendered = _render(build_payload(name))
        if args.check:
            current = output.read_text(encoding="utf-8") if output.exists() else ""
            if current != rendered:
                raise SystemExit(
                    f"FATAL: {output.name} is stale; run "
                    "scripts/gen_giga_2877_variants.py"
                )
            print(f"{output.name}: OK")
        else:
            output.write_text(rendered, encoding="utf-8")
            print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
