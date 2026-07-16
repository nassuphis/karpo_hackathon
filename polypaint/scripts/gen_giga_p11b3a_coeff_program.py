#!/usr/bin/env python3
"""Generate the portable giga_p11b3a Coeff Program document.

giga_p11b3a (run 2025-03-14; p11b3/uc_old at snapshot 01627b4) — "balls
with holes", per the source's own comment:

    unit_circle:  t' = e^{2j pi t}
    p11b3:        t = t1'+t2';  a = |t|/2;  m = int(251a) % 37
                  v = (arange(11)+1)/(t+4)
                  v[int(7a*11)%11]   = (p1+1)/(t + 2|t| + 1 + m)
                  v[int(619a*11)%11] = (p2+1)/(t + 2|t| + 1 + m//2)
                  cf = exp(1j pi v)
    uc_old x3:    cf = exp(2j pi * cf/sum|cf|)     (guard sum<1e-15: DEAD,
                                                    0/5000 rows)
    roots:        np.roots feedback

Dict ghosts: n=6 and i=11110 are read by nothing in this chain.

Two spellings worth recording:
- POKE INDEXES MUST BE LITERAL: the reference's dynamic-position pokes
  become masked-update scans, with the exact integer-equality mask
  floor((101-(k-p)^2)/101) (1 iff k == p for |k-p| <= 10). The p1/p2
  positions and poke VALUES are parked in a stack-side 5-vector
  [a, val1, val2, p1, p2] so every expression stays under the 256-char
  cap; the two updates run in the reference's order (p2 overwrites p1
  on collision, exactly as numpy's sequential assignment does).
- uc_old's sum normalization is the cumsum-of-abs idiom: push cf, take
  |cf|, cumsum in-poly, then divide(pop, poly[10]) — the scalar chip
  reads the sorted... the SUMMED register before the pop.

The roots feedback is roots_cm(hi, exact); the leading pad-zero strips
in the solver, reproducing the reference's degree-9 solve of the
10-root vector. 245 tokens — the closest program yet to the 256 cap.

Parity is pinned by tests/test_giga_p11b3a_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_p11b3a.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

T_TEXT = "(exp(pi2i*t1) + exp(pi2i*t2))"
M0_TEXT = "floor(poly[0]*251) - 37*floor(floor(poly[0]*251)/37)"
EQ1 = "floor((101-(k-tos[3])*(k-tos[3]))/101)"
EQ1_0 = "floor((101-(0-tos[3])*(0-tos[3]))/101)"
EQ2 = "floor((101-(k-tos[4])*(k-tos[4]))/101)"
EQ2_0 = "floor((101-(0-tos[4])*(0-tos[4]))/101)"

UC_OLD_BLOCK = [
    "poly",
    "poly = abs(poly)",
    "poly = scan(11, 0, poly[0], prev + poly[k])",
    "poly = divide(pop, poly[10])",
    "poly = multiply(poly, 6.283185307179586i)",
    "poly = exp(poly)",
]


def build_source_text() -> str:
    lines = [
        # parked scalars: [a, val1, val2, p1, p2]
        "poly = fill(5, 0)",
        "poly[0] = abs(exp(pi2i*t1) + exp(pi2i*t2))/2",
        "poly[3] = floor(7*poly[0]*11) - 11*floor(floor(7*poly[0]*11)/11)",
        "poly[4] = floor(619*poly[0]*11) - 11*floor(floor(619*poly[0]*11)/11)",
        f"poly[1] = (poly[3]+1)/({T_TEXT} + abs({T_TEXT})*2 + 1 + {M0_TEXT})",
        f"poly[2] = (poly[4]+1)/({T_TEXT} + abs({T_TEXT})*2 + 1 + floor(({M0_TEXT})/2))",
        "poly",
        # v = (arange(11)+1)/(t+4), then the two masked updates in order
        "poly = arange(1, 12)",
        f"poly = divide(poly, {T_TEXT} + 4)",
        f"poly = scan(11, 0, poly[0]*(1-{EQ1_0}) + tos[1]*{EQ1_0},"
        f" poly[k]*(1-{EQ1}) + tos[1]*{EQ1})",
        f"poly = scan(11, 0, poly[0]*(1-{EQ2_0}) + tos[2]*{EQ2_0},"
        f" poly[k]*(1-{EQ2}) + tos[2]*{EQ2})",
        "drop",
        # cf = exp(1i pi v)
        "poly = multiply(poly, 3.141592653589793i)",
        "poly = exp(poly)",
    ]
    for _ in range(3):
        lines.extend(UC_OLD_BLOCK)
    lines.extend([
        "poly = roots_cm(poly, hi, exact)",
        "emit",
    ])
    return "\n".join(lines)


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_p11b3a",
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
                "FATAL: giga_p11b3a.coeff-program.json is stale; run "
                "scripts/gen_giga_p11b3a_coeff_program.py"
            )
        print("giga_p11b3a.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
