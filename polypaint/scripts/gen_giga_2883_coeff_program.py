#!/usr/bin/env python3
"""Generate the portable giga_2883 Coeff Program document.

giga_2883 (working tree at run time; ltlwd committed at 1230acc, the
rest at 8e28adb) mixes a random Littlewood polynomial with the glyph
constellation under a baker's-map xfrm:

    xfrm bkr:  t' = frac(2*(t mod 1)) + 0.5j*floor(2*(t mod 1))  per axis
    combine:   cf = ltlwd()*0.75 + poly_letter(t1', t2')*0.25
    zfrm rev:  q = cf[::-1]

with ltlwd = 33 iid random +-1 coefficients (np.random.choice — TRUE
RNG, independent of the uniforms), poly_letter on glyph b178 with
offset ro=0, io=-1, and n=33 matching the letter's coefficient count.

Two structural translations (measurements in recreate_giga_2883.md):

1. THE BAKER'S MAP makes the uniforms complex; through scale() and
   complex(a, b) = a + 1j*b the shift decomposes to
   Re = 0.05 + 0.9*frac(2u1) - 0.45*floor(2u2)
   Im = 0.45*floor(2u1) + (0.05 + 0.9*frac(2u2)) - 1
   — a 2x2 grid of shifted glyph copies. The four frac/floor parts ride
   the scratch-poke bridge into the translate delta.
2. THE RNG is replaced by the scan-PRNG idiom (no new primitive): a
   frac-cascade x_{k+1} = frac(x_k*9821.4959 + 0.211327) seeded from
   frac(t1*7919.7717 + t2*104729.31 + 0.5), thresholded to +-1 by the
   second binary digit. Measured: sign balance +0.005, intra-row lag-1
   correlation -0.003, and the aggregate root cloud is statistically
   indistinguishable from the true-RNG ensemble (log-density corr
   0.9271 vs same-ensemble noise floor 0.9087 at 24k rows). The PRNG
   arithmetic is bit-identical between the VM and the numpy oracle, so
   per-row parity carries only translate dust.

Parity is pinned by tests/test_giga_2883_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2883.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

PRNG_G = "9821.4959"
PRNG_C = "0.211327"
PRNG_SEED = ("((t1*7919.7717 + t2*104729.31 + 0.5)"
             " - floor(t1*7919.7717 + t2*104729.31 + 0.5))")
DELTA_TEXT = ("(0.05+0.9*poly[0]-0.45*poly[3])"
              " + 1i*(0.45*poly[1]+(0.05+0.9*poly[2])-1)")


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
    s0 = f"({PRNG_SEED}*{PRNG_G} + {PRNG_C}) - floor({PRNG_SEED}*{PRNG_G} + {PRNG_C})"
    sk = f"(prev*{PRNG_G} + {PRNG_C}) - floor(prev*{PRNG_G} + {PRNG_C})"
    return "\n".join(
        [
            # ltlwd: 33 pseudo-random +-1 coefficients (scan-PRNG idiom)
            f"poly = scan(33, 0, {s0}, {sk})",
            "poly = scan(33, 0, 1-2*floor(2*poly[0]), 1-2*floor(2*poly[k]))",
            "poly = multiply(poly, 0.75)",
            "poly",
            # bkr parts: frac/floor of the folded, doubled uniforms
            "poly = fill(4, 0)",
            "poly[0] = 2*(t1-floor(t1)) - floor(2*(t1-floor(t1)))",
            "poly[1] = floor(2*(t1-floor(t1)))",
            "poly[2] = 2*(t2-floor(t2)) - floor(2*(t2-floor(t2)))",
            "poly[3] = floor(2*(t2-floor(t2)))",
            # poly_letter on the baker-shifted glyph (ro=0, io=-1)
            f"poly = translate_roots(roots_ascii_literal(178), {DELTA_TEXT})",
            "poly = multiply(poly, 0.25)",
            # combine (andy = 0.75), then zfrm rev
            "poly = add(pop, poly)",
            "poly = rev(poly)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2883",
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
                "FATAL: giga_2883.coeff-program.json is stale; run "
                "scripts/gen_giga_2883_coeff_program.py"
            )
        print("giga_2883.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
