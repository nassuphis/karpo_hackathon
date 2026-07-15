#!/usr/bin/env python3
"""Generate the portable giga_2880 Coeff Program document.

giga_2880 (snapshot 8e28adb) is the deepest chain in the recreation
series — five zfrm stages on a counter-driven constellation:

    nopoly_crazy1 -> poly -> sort_moduli_keep_angles_p(0.1)
                  -> invuc -> normalize -> roots_p(0.1)

Three structural translations, all measured (see recreate_giga_2880.md):

1. THE COUNTER. nopoly_crazy1 increments a global i per sample and
   derives a glyph offset ((i%71)/5) * exp(2j*pi*(i%101)/3) — 71 radii x
   101 phase steps (only 3 distinct angles since the phase reduces mod
   3), a 7171-cycle by CRT, independent of the uniforms. Polypaint rows
   have no cross-sample state, so the program splits each uniform into
   coarse+fine parts: m = floor(71*t1) mod 71 picks the radius and
   frac(71*t1) becomes the jitter (same for n = floor(101*t2) mod 101).
   The joint law (m, n, jitter1, jitter2) is identical to the
   reference's (offset-cycle x uniforms); aggregate clouds agree at the
   sampling noise floor (corr 0.924 vs floor 0.876 at 20k rows).
2. THE CHAIN. poly = translate_roots (glyph b2 + delta via the poke
   bridge), sort_moduli_keep_angles = the native sort_mod_keep_angle
   transform, invuc/normalize spelled with sort_abs + divide (max|cf| =
   abs of the last sorted element). Front-chain parity vs verbatim
   numpy: 2.7e-14.
3. THE ROOT-FIND. roots_p solves the TRANSFORMED cf (roots unknown at
   compile time) with np.roots and mixes them back: q = 0.1*(roots ++
   [1]) + cf. The VM's roots_cm runs the same companion-matrix zgeev —
   but its legacy leading-strip (|cf| < max*3.16e-8) deletes the tiny
   leading coefficients invuc+normalize produce (mean 16 slots!), whose
   GIANT roots are load-bearing (image corr collapses to 0.51). This
   wave added strip=exact (np.roots semantics: only exactly-zero leads
   stripped); with it the image is exact: the f32 root cast is invisible
   (corr 1.0000) and the zgeev emission ORDER — which pairs roots to
   slots — matches numpy's on both Accelerate-new (local) and netlib
   LAPACK 3.10 (the lambda layer; ARM64 docker gate, worst 1.9e-7).

Parity is pinned by tests/test_giga_2880_coeff_program.py; the deployed
LAPACK lineage by scripts/test-roots-cm-strip-docker.sh.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2880.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

M_TEXT = "floor(71*t1) - 71*floor(floor(71*t1)/71)"
N_TEXT = "floor(101*t2) - 101*floor(floor(101*t2)/101)"
# scale(0.1*t, 0.9) = 0.05 + 0.09*t with the remapped jitter, plus the
# discrete offset (m/5)*exp(2j*pi*n/3) read back from the scratch pokes.
DELTA_TEXT = (
    "(0.05+0.09*(71*t1-floor(71*t1))) + 1i*(0.05+0.09*(101*t2-floor(101*t2)))"
    " + (poly[0]/5)*exp(1i*6.283185307179586*(poly[1]/3))"
)


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
            # counter remap: scratch pokes carry the discrete offset index
            "poly = fill(2, 0)",
            f"poly[0] = {M_TEXT}",
            f"poly[1] = {N_TEXT}",
            # zfrm stage 1: poly (roots -> coefficients)
            f"poly = translate_roots(roots_ascii_literal(2), {DELTA_TEXT})",
            # zfrm stage 2: sort_moduli_keep_angles_p, andy = 0.1
            "poly",
            "poly = sort_mod_keep_angle(poly)",
            "poly = multiply(poly, 0.1)",
            "poly = add(pop, poly)",
            # zfrm stage 3: invuc  (cf / exp(2j*pi*cf/max|cf|))
            "poly",
            "poly = sort_abs(poly)",
            "poly = divide(peek, abs(poly[32]))",
            "poly = multiply(poly, 6.283185307179586i)",
            "poly = exp(poly)",
            "poly = divide(pop, poly)",
            # zfrm stage 4: normalize  (cf / max|cf|)
            "poly",
            "poly = sort_abs(poly)",
            "poly = divide(pop, abs(poly[32]))",
            # zfrm stage 5: roots_p, andy = 0.1 (np.roots semantics via
            # strip=exact; pad lo leaves slot 32 for the appended 1)
            "poly",
            "poly = roots_cm(poly, lo, exact)",
            "poly[32] = 1",
            "poly = multiply(poly, 0.1)",
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
        "name": "giga_2880",
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
                "FATAL: giga_2880.coeff-program.json is stale; run "
                "scripts/gen_giga_2880_coeff_program.py"
            )
        print("giga_2880.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
