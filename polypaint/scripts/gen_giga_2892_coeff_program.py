#!/usr/bin/env python3
"""Generate the giga_2892 Coeff Program document.

giga_2892 is poly_chess3 (polys/chess.py) — degree 64, xfrm/zfrm none,
solve=solve: the COUPLED chessboard mixer. Two twists on poly_chess2
(giga_2891):

  t0 = rand()                              # ONE draw drives BOTH copies
  d1 = 0.5*e^(2 pi i t0)*(1+1i)            # phase displacement
  d2 = 0.5*(t0-0.5)*(1+1i)                 # real shift — same t0!
  a  = bimodal_skewed(0.85)                # heavy bimodal mix
  cf = np.poly(lat+par*d2)*a + np.poly(lat+par*d1)*(1-a)

The coupling makes the two copies breathe in lockstep (the phase
angle and the real shift always agree through t0), and the a=0.85
bimodal (exponent 1/(1-0.85) = 6.666666666666666, non-integer -> the
exp(E*log(.)) floor-gate spelling, 2898-family doctrine) keeps most
rows near a pure copy. Two unseeded draws -> two fresh frac cascades.
White squares are per-row exact anchors as in 2891.

Construction: 2891's parity-mask scans for the point sets + the
2898-family per-branch weight parking (the weight scan reads the
parked uniform through tos in STATEMENT position; branches mix with
the proven add(poly, pop) — never the nested-pop one-liner, which
miscomputes; see recreate_giga_2891.md trap log).

Parity is pinned by tests/test_giga_2892_coeff_program.py; the
diagnosis lives in recreate_giga_2892.md.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def _lattice():
    idx = [i - 3.5 for i in range(8)]
    base = []
    parity = []
    for row in range(8):
        for col in range(8):
            base.append(complex(idx[col], idx[row]))
            parity.append((row + col) % 2)
    return base, parity


def _lit(z: complex) -> str:
    re = f"{z.real:g}"
    if z.imag == 0:
        return re
    return f"{re}+{z.imag:g}i" if z.imag > 0 else f"{re}{z.imag:g}i"


BASE, PARITY = _lattice()
BASE_TEXT = ", ".join(_lit(z) for z in BASE)
PARITY_TEXT = ", ".join(str(p) for p in PARITY)

# fresh frac-cascade seeds: t0 (shared displacement draw), u (bimodal)
SEEDS = [
    (7043.9231, 58111.97, 0.35),
    (2593.4053, 66301.49, 0.65),
]

# bimodal_skewed(0.85) exponent, exact f64 of the reference's 1/(1-a)
EXPONENT = "6.666666666666666"


def _frac(expr: str) -> str:
    return f"({expr})-floor({expr})"


U = [_frac(f"t1*{m1}+t2*{m2}+{c}") for m1, m2, c in SEEDS]


def _a_mix(u: str) -> str:
    e = EXPONENT
    return (
        f"(1-floor(2*{u}))*exp({e}*log(2*{u}))/2"
        f"+floor(2*{u})*(1-exp({e}*log(2-2*{u}))/2)"
    )


def _branch(delta_expr: str, weight_expr: str) -> list[str]:
    # weight vector first (uniform parked, statement-position tos scan),
    # then the parity-masked displaced lattice, expanded and weighted
    return [
        f"poly = scan(1, 0, {U[1]}, {U[1]})",
        "poly",
        f"poly = scan(65, 0, {weight_expr}, prev)",
        "poly",
        f"poly = vector_literal({PARITY_TEXT})",
        "poly",
        f"poly = scan(64, 0, tos[0]*({delta_expr}), tos[k]*({delta_expr}))",
        f"poly = add(poly, vector_literal({BASE_TEXT}))",
        "drop",
        "poly = expand_roots(poly)",
        "poly = multiply(poly, pop)",
        "drop",
    ]


def build_source_text() -> str:
    a = _a_mix("tos[0]")
    d_phase = f"0.5*exp(6.283185307179586i*({U[0]}))*(1+1i)"
    d_real = f"0.5*(({U[0]})-0.5)*(1+1i)"
    return "\n".join(
        _branch(d_phase, f"1-({a})")                     # cf1 (phase) x (1-a)
        + ["poly"]
        + _branch(d_real, a)                             # cf2 (real)  x a
        + ["poly = add(poly, pop)", "emit"]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2892",
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    output = ROOT / "giga_2892.coeff-program.json"
    rendered = json.dumps(build_payload(), indent=2, ensure_ascii=True) + "\n"
    if args.check:
        current = output.read_text(encoding="utf-8") if output.exists() else ""
        if current != rendered:
            raise SystemExit("FATAL: giga_2892.coeff-program.json is stale; run scripts/gen_giga_2892_coeff_program.py")
        print(f"{output.name}: OK")
    else:
        output.write_text(rendered, encoding="utf-8")
        print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
