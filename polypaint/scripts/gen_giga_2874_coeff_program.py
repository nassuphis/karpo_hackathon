#!/usr/bin/env python3
"""Generate the portable giga_2874 Coeff Program document.

giga_2874 = the giga_2872 chain with two new elements, both verified against
the pinned pyroots snapshot 500685f (see recreate_giga_2874.md):

1. xfrm = jump: jt = t + count(levels > t) * jmp with levels = [0.1..0.9]
   and jmp = -0.05, applied to both uniforms BEFORE the letter translation.
   The counts are computed with real vector comparisons (gt) so exact
   grid-hits-a-level rows keep numpy's strict-> semantics; a floor()
   spelling would diverge there (floor(10*0.3) == 2 in doubles).
2. Full andy1, both gathers: cf[csi] - cf[cpi]. The jump's negative deltas
   break the monic-collapse shortcut used by giga_2872/2873 (51/20000
   random rows have argsort(|cf|)[0] != 0), so csi = cumprod(argsort) % n
   must be computed for real — INCLUDING numpy's int64 wraparound, which
   the cumprod hits by element ~13. The scan tracks the running product in
   two exact 32-bit limbs (prev = lo + 1i*hi, both < 2^32), and the decode
   maps limbs to the signed-int64 residue: (u - 2^64*[hi >= 2^31]) mod n
   with 2^32 mod n and 2^64 mod n spelled as poly_len expressions so the
   block stays glyph-agnostic. Verified integer-exact against numpy on
   20,000 rows including all collapse-violating ones.

The roots are the 36 lit pixels of font glyph b16 (CP437 sheet cell 15,
mirrored per the historical transcription quirks) via roots_ascii_literal.
Parity is pinned by tests/test_giga_2874_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2874.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

LEVELS_TEXT = "0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9"

# int64 cumprod step in two 32-bit limbs: prev = lo + 1i*hi.
# lo*s < 2^38 and hi*s + carry < 2^38 + 2^6 stay exact in doubles.
LIMB_STEP = (
    "(real(prev)*poly[k] - 4294967296*floor(real(prev)*poly[k]/4294967296))"
    " + 1i*(imag(prev)*poly[k] + floor(real(prev)*poly[k]/4294967296)"
    " - 4294967296*floor((imag(prev)*poly[k]"
    " + floor(real(prev)*poly[k]/4294967296))/4294967296))"
)

# 2^32 mod n and -(2^64 mod n), n = poly_len (dynamic so the glyph code
# stays scrubbable). 2^64 mod n = (2^32 mod n)^2 mod n.
R32 = "4294967296 - poly_len*floor(4294967296/poly_len)"
R64_NEG = (
    "0 - ((4294967296 - poly_len*floor(4294967296/poly_len))"
    "*(4294967296 - poly_len*floor(4294967296/poly_len))"
    " - poly_len*floor((4294967296 - poly_len*floor(4294967296/poly_len))"
    "*(4294967296 - poly_len*floor(4294967296/poly_len))/poly_len))"
)


def _map(expr_k: str) -> str:
    """Elementwise map over the top-of-stack vector via the scan idiom."""
    return (
        f"poly = scan(poly_len, 0, {expr_k.replace('K', '0')},"
        f" {expr_k.replace('K', 'k')})"
    )


def letter_roots() -> list[complex]:
    """Glyph b16's lit pixels via the checked-in table, read exactly as
    letters.get_letter_coordinates did: bit (x, y) -> (x-3.5, (7-y)-3.5)."""
    from cp437_font import FONT_ROWS

    coords = []
    for y, row in enumerate(FONT_ROWS[16]):
        for x in range(8):
            if row & (1 << (7 - x)):
                coords.append(complex(x - 3.5, (7 - y) - 3.5))
    return coords


def build_source_text() -> str:
    if len(letter_roots()) != 36:
        raise RuntimeError("b16 must contain exactly 36 lit pixels")
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
            "poly = translate_roots(roots_ascii_literal(16),"
            " (0.05+0.9*(t1+(0-0.05)*poly[0]))"
            " + 1i*(0.05+0.9*(t2+(0-0.05)*poly[8])))",
            # --- andy1, csi gather: cumprod(argsort) % n, int64 wrap ---
            "poly",
            "poly = arange(0, poly_len)",
            "poly = argsort(poly, peek)",
            f"poly = scan(poly_len, 0, poly[0], {LIMB_STEP})",
            "poly",
            _map("floor(imag(tos[K])/2147483648)"),
            f"poly = multiply(poly, {R64_NEG})",
            "poly",
            "swap",
            _map("imag(tos[K]) - poly_len*floor(imag(tos[K])/poly_len)"),
            f"poly = multiply(poly, {R32})",
            "poly",
            "swap",
            _map("real(tos[K]) - poly_len*floor(real(tos[K])/poly_len)"),
            "drop",
            "poly = add(pop, poly)",
            "poly = add(pop, poly)",
            "poly = rem(poly, poly_len)",
            "poly = add(poly, poly_len)",
            "poly = rem(poly, poly_len)",
            "poly = scan(poly_len, 0, tos[poly[k]], tos[poly[k]])",
            # --- andy1, cpi gather: cumsum(argsort) % n ----------------
            "poly",
            "swap",
            "poly = arange(0, poly_len)",
            "poly = argsort(poly, peek)",
            "poly = scan(poly_len, 0, poly[0], prev + poly[k])",
            "poly = rem(poly, poly_len)",
            "poly = scan(poly_len, 0, tos[poly[k]], tos[poly[k]])",
            "drop",
            "poly = subtract(pop, poly)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2874",
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
                "FATAL: giga_2874.coeff-program.json is stale; run "
                "scripts/gen_giga_2874_coeff_program.py"
            )
        print("giga_2874.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
