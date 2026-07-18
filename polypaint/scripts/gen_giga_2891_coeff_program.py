#!/usr/bin/env python3
"""Generate the giga_2891 Coeff Program document.

giga_2891 is poly_chess2 PLAIN (polys/chess.py:48) — degree 64, xfrm
and zfrm both none, solve=solve: the memoryless chessboard mixer, i.e.
giga_2920 without the recursive_add3 EMA chain and without any root
trip. Per row (three unseeded np.random draws):

  d1 = 0.5*e^(2 pi i u)*(1+1i)      # rotating phase displacement
  d2 = 0.5*(v-0.5)*(1+1i)           # real diagonal shift
  cf1 = np.poly(lattice + parity*d1)   # black squares displaced
  cf2 = np.poly(lattice + parity*d2)
  cf  = cf2*a + cf1*(1-a)           # plain-uniform convex mix

White squares (parity 0) are EXACT common roots of both copies and of
every convex mix — and with no EMA pairing to drag them, the anchor is
per-row exact here (unlike 2920's distributional anchors). The halo is
the usual ill-conditioned mix-root dust. The three draws remap to
fresh frac-cascade uniforms of the sweep's own (t1, t2); the mix
orientation (cf2 gets a, cf1 gets 1-a) mirrors the reference exactly.

No solver in the program, so the plain no-LAPACK build runs it, and a
row costs just two np.poly expansions — the cheap sibling of 2920.
Construction idioms are 2920's verbatim: the parity mask parked on the
stack with statement-position scans (tos in expression-position scans
reads poly itself — the 2897 trap), constant-weight arg-scans (no tos:
safe), coordinate literals in layout2coord/meshgrid row-major order.

Parity is pinned by tests/test_giga_2891_coeff_program.py; the
diagnosis lives in recreate_giga_2891.md.
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

# fresh frac-cascade seeds (u -> phase, v -> real shift, a -> mix)
SEEDS = [
    (6089.6603, 53923.19, 0.2),
    (3251.8093, 71993.77, 0.8),
    (1847.0567, 46807.13, 0.4),
]


def _frac(expr: str) -> str:
    return f"({expr})-floor({expr})"


U = [_frac(f"t1*{m1}+t2*{m2}+{c}") for m1, m2, c in SEEDS]


def _point_set(delta_expr: str) -> list[str]:
    return [
        f"poly = vector_literal({PARITY_TEXT})",
        "poly",
        f"poly = scan(64, 0, tos[0]*({delta_expr}), tos[k]*({delta_expr}))",
        f"poly = add(poly, vector_literal({BASE_TEXT}))",
        "drop",
        "poly = expand_roots(poly)",
    ]


def build_source_text() -> str:
    # Each branch is weighted BEFORE parking, so the mix is a plain
    # add(poly, pop) of statement-position results. The tempting one-
    # liner add(poly, multiply(pop, scan(...))) MISCOMPUTES: add parks
    # its left operand as a stack temp and the nested pop grabs that
    # temp instead of the parked branch (VM-verified — the same trap
    # family as tos in expression-position scans).
    d_phase = f"0.5*exp(6.283185307179586i*({U[0]}))*(1+1i)"
    d_real = f"0.5*(({U[1]})-0.5)*(1+1i)"
    return "\n".join(
        _point_set(d_phase)                                   # cf1 (phase copy)
        + [
            f"poly = multiply(poly, scan(65, 0, 1-({U[2]}), prev))",
            "poly",
        ]
        + _point_set(d_real)                                  # cf2 (real copy)
        + [
            f"poly = multiply(poly, scan(65, 0, {U[2]}, prev))",
            "poly = add(poly, pop)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2891",
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    output = ROOT / "giga_2891.coeff-program.json"
    rendered = json.dumps(build_payload(), indent=2, ensure_ascii=True) + "\n"
    if args.check:
        current = output.read_text(encoding="utf-8") if output.exists() else ""
        if current != rendered:
            raise SystemExit("FATAL: giga_2891.coeff-program.json is stale; run scripts/gen_giga_2891_coeff_program.py")
        print(f"{output.name}: OK")
    else:
        output.write_text(rendered, encoding="utf-8")
        print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
