#!/usr/bin/env python3
"""Generate the giga_2920 Coeff Program document.

giga_2920 = poly_chess2 through recursive_add3 (decay 0.1), solved. The
formula IGNORES its sweep inputs: three unseeded np.random draws per
row build two 8x8 chessboard root sets — black squares displaced by a
random PHASE offset 0.5*e^{2pi i u}*(1+i) in one copy and a random REAL
shift 0.5*(v-0.5)*(1+i) in the other — expanded with np.poly and mixed
with a random convex weight. recursive_add3 then EMAs the sorted roots
10% toward the PREVIOUS row's output (a one-row memory: influence of
row k-j decays as 0.1^j).

Migration (the 2883/2907 remap + 2910 burn-in doctrines):
- the three uniforms come from independent frac cascades of the sweep's
  own (t1, t2); a SECOND triple supplies a fresh draw standing in for
  the previous row (numpy analog vs the true sequential chain: 2D cloud
  corr 0.975 — the EMA memory is one row deep);
- white squares are EXACT common roots of both copies and every convex
  mix — the artwork's sharp lattice anchors, pinned per row;
- the mix polynomial's outer/halo roots are ill-conditioned (condition
  ~1e8-1e10): expand_roots' accumulation-order dust (~1e-10 of scale vs
  np.poly) and sort_abs tie-pairing on the 4-fold-degenerate lattice
  radii repaint them per row — the same noise-as-paint class as
  giga_2864/outflow, and the reference's own np dust is equally
  arbitrary. Ensemble-verified: VM cloud vs oracle cloud at the
  sampling floor (0.91 vs 0.95 disjoint-half floor at 400 rows).

Program idioms: the parity mask rides the stack while scans build the
displacement vectors from side-table expressions (the 2877 tos[k]
pattern keeps the token count at 86 — naive scalar embedding hit the
256-chip cap); mixes are constant-vector scans; write-once registers
avoided so the DEPLOYED compiler accepts the upload.

Parity is pinned by tests/test_giga_2920_coeff_program.py.
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

SEEDS = [
    (7919.7717, 104729.31, 0.5),
    (3571.3331, 27644.437, 0.25),
    (1299.7091, 15485.863, 0.75),
    (6997.9337, 86028.121, 0.125),
    (5227.5511, 49979.687, 0.375),
    (2417.9977, 67867.967, 0.625),
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


def _branch(phase_u: str, real_u: str, mix_u: str) -> list[str]:
    d_phase = f"0.5*exp(6.283185307179586i*({phase_u}))*(1+1i)"
    d_real = f"0.5*(({real_u})-0.5)*(1+1i)"
    return (
        _point_set(d_phase)
        + ["poly"]
        + _point_set(d_real)
        + [
            f"poly = multiply(poly, scan(65, 0, {mix_u}, prev))",
            f"poly = add(poly, multiply(pop, scan(65, 0, 1-({mix_u}), prev)))",
            "poly = roots_cm(poly, lo, exact)",
            "poly = poly[0:64]",
            "poly = sort_abs(poly)",
        ]
    )


def build_source_text() -> str:
    return "\n".join(
        _branch(U[3], U[4], U[5])                       # previous-row analog
        + ["poly = multiply(poly, 0.1)", "poly"]
        + _branch(U[0], U[1], U[2])                     # current row
        + [
            "poly = multiply(poly, 0.9)",
            "poly = add(poly, pop)",
            "poly = expand_roots(poly)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2920",
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    output = ROOT / "giga_2920.coeff-program.json"
    rendered = json.dumps(build_payload(), indent=2, ensure_ascii=True) + "\n"
    if args.check:
        current = output.read_text(encoding="utf-8") if output.exists() else ""
        if current != rendered:
            raise SystemExit("FATAL: giga_2920.coeff-program.json is stale; run scripts/gen_giga_2920_coeff_program.py")
        print(f"{output.name}: OK")
    else:
        output.write_text(rendered, encoding="utf-8")
        print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
