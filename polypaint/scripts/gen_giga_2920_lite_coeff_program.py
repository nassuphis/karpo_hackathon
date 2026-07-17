#!/usr/bin/env python3
"""Generate the giga_2920_lite Coeff Program document.

giga_2920's cost is dominated by TWO degree-64 eigensolves per row
(both convex-mix root trips). This variant keeps branch A (the current
row's full mix -> roots_cm -> sort_abs) and replaces branch B — the
10%-weight previous-row analog — with the EXACTLY KNOWN phase-displaced
lattice points: constructed, not solved. One eigensolve per row instead
of two (~2x on the dominant cost), and the ensembles are visually
identical: lite-vs-full 2D cloud correlation 0.979 against a 0.993
half-vs-half sampling floor (3000 rows, measured at authoring).

Everything else follows recreate_giga_2920.md: the stack-parked parity
scans, the frac-cascade uniforms, the whites-as-exact-mix-roots
invariant, and the dust/pairing doctrine on the surviving eigensolve.
Parity is pinned by tests/test_giga_2920_lite_coeff_program.py.
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
    ]


def build_source_text() -> str:
    d_phase_a = f"0.5*exp(6.283185307179586i*({U[0]}))*(1+1i)"
    d_real_a = f"0.5*(({U[1]})-0.5)*(1+1i)"
    d_phase_b = f"0.5*exp(6.283185307179586i*({U[3]}))*(1+1i)"
    return "\n".join(
        # branch A: the current row's full mix — the one eigensolve
        _point_set(d_phase_a) + ["poly = expand_roots(poly)", "poly"]
        + _point_set(d_real_a) + ["poly = expand_roots(poly)"]
        + [
            f"poly = multiply(poly, scan(65, 0, {U[2]}, prev))",
            f"poly = add(poly, multiply(pop, scan(65, 0, 1-({U[2]}), prev)))",
            "poly = roots_cm(poly, lo, exact)",
            "poly = poly[0:64]",
            "poly = sort_abs(poly)",
            "poly = multiply(poly, 0.9)",
            "poly",
        ]
        # branch B: raw phase-displaced lattice points (constructed, not
        # solved) stand in for the previous row at 10% weight
        + _point_set(d_phase_b)
        + [
            "poly = sort_abs(poly)",
            "poly = multiply(poly, 0.1)",
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
        "name": "giga_2920_lite",
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    output = ROOT / "giga_2920_lite.coeff-program.json"
    rendered = json.dumps(build_payload(), indent=2, ensure_ascii=True) + "\n"
    if args.check:
        current = output.read_text(encoding="utf-8") if output.exists() else ""
        if current != rendered:
            raise SystemExit("FATAL: giga_2920_lite.coeff-program.json is stale; run scripts/gen_giga_2920_lite_coeff_program.py")
        print(f"{output.name}: OK")
    else:
        output.write_text(rendered, encoding="utf-8")
        print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
