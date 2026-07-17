#!/usr/bin/env python3
"""Generate the giga_2897 Coeff Program document.

giga_2897 is poly_chess5 over the rloc13 layout (degree 37, a=0.75,
phi=0.5, rho/speed/shape defaults 0.33/1.0/circle; xfrm/zfrm none,
solve=solve): two 37-point letterform lattices — source S and target T
cells of the rloc13 ASCII grid — displaced along the (1+1i) diagonal by
antipodal phases of one circle point (delta and, at phi=0.5, exactly
-delta), each expanded with np.poly, then convex-mixed with the
bimodal_skewed(0.75) weight. The sweep inputs are ignored by the
reference (two unseeded np.random draws per row), so the standard RNG
remap applies: two frac-cascade uniforms of (t1, t2) stand in for the
draws. No root trip inside the program — the reference paints
np.roots(mix), which is exactly what the pipeline solver does with the
emitted coefficients.

Deployed-compiler constraints honored (no registers, no expression-
position transforms): constant vectors are built with the scan side-
table idiom, the uniforms pair is parked on the stack and read back
through tos[k] inside scan expressions, and the bimodal weight is
spelled branchlessly with floor() gates and repeated multiplication
(no ^ operator).

Parity is pinned by tests/test_giga_2897_coeff_program.py; the full
diagnosis lives in recreate_giga_2897.md.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# rloc13 coordinates, computed once (at authoring) from the read-only
# reference layout string (pyroots/poly.py:19578) through an exact
# replica of polylayout.layout2coord: expandtabs(4), dedent, strip
# newlines, row-major S/T masks on a centered 14x25 grid with the top
# row at the highest y (the leading "_" row and two blank rows shift
# the vertical center; one trailing-spaces row widens the grid to 25).
# 37 source cells + 37 target cells -> degree 37, matching the saved
# run metadata. Embedded as literals rather than ASCII art so the
# figure cannot drift by a miscounted row.
S_POINTS = [
    (8+2.5j), (-5+1.5j), (8+1.5j), (-5+0.5j), (8+0.5j), (-5-0.5j),
    (8-0.5j), (-5-1.5j), (8-1.5j), (-5-2.5j), (8-2.5j), (-5-3.5j),
    (8-3.5j), (-5-4.5j), (8-4.5j), (8-5.5j), (-12-6.5j), (-11-6.5j),
    (-10-6.5j), (-9-6.5j), (-8-6.5j), (-7-6.5j), (-6-6.5j), (-5-6.5j),
    (-4-6.5j), (-3-6.5j), (-2-6.5j), (-1-6.5j), (0-6.5j), (1-6.5j),
    (2-6.5j), (3-6.5j), (4-6.5j), (5-6.5j), (6-6.5j), (7-6.5j), (8-6.5j),
]

T_POINTS = [
    (-12+3.5j), (-11+3.5j), (-10+3.5j), (-9+3.5j), (-8+3.5j), (-7+3.5j),
    (-6+3.5j), (-5+3.5j), (-4+3.5j), (-3+3.5j), (-2+3.5j), (-1+3.5j),
    (0+3.5j), (1+3.5j), (2+3.5j), (3+3.5j), (4+3.5j), (5+3.5j), (6+3.5j),
    (7+3.5j), (8+3.5j), (-12+2.5j), (-12+1.5j), (1+1.5j), (-12+0.5j),
    (1+0.5j), (-12-0.5j), (1-0.5j), (-12-1.5j), (1-1.5j), (-12-2.5j),
    (1-2.5j), (-12-3.5j), (1-3.5j), (-12-4.5j), (1-4.5j), (-12-5.5j),
]


def _lit(z: complex) -> str:
    re = f"{z.real:g}"
    if z.imag == 0:
        return re
    return f"{re}+{z.imag:g}i" if z.imag > 0 else f"{re}{z.imag:g}i"


S_TEXT = ", ".join(_lit(z) for z in S_POINTS)
T_TEXT = ", ".join(_lit(z) for z in T_POINTS)

# frac-cascade uniforms of the sweep's own (t1, t2) replacing the two
# unseeded reference draws: U1 -> the circle parameter t, U2 -> the
# bimodal_skewed mix draw.
SEEDS = [
    (5477.4409, 49807.87, 0.375),
    (2833.9973, 68111.53, 0.625),
]


def _frac(expr: str) -> str:
    return f"({expr})-floor({expr})"


U = [_frac(f"t1*{m1}+t2*{m2}+{c}") for m1, m2, c in SEEDS]

TWO_PI_I = "6.283185307179586i"

# displacement per copy: rho*circle(.) times (1+1i); the target phase
# sits at t+phi with phi=0.5, speed=1.
D_SRC = f"0.33*exp({TWO_PI_I}*tos[0])*(1+1i)"
D_TGT = f"0.33*exp({TWO_PI_I}*(tos[0]+0.5))*(1+1i)"

# bimodal_skewed(0.75) branchlessly: for u < 0.5 the weight is
# (2u)^4/2, else 1-(2(1-u))^4/2; floor(2u) is the branch gate and the
# 4th powers are spelled as repeated products (no ^ in the grammar).
_W = "(2*tos[1])"
_V = "(2-2*tos[1])"
A_MIX = (
    f"(1-floor(2*tos[1]))*{_W}*{_W}*{_W}*{_W}/2"
    f"+floor(2*tos[1])*(1-{_V}*{_V}*{_V}*{_V}/2)"
)


def _branch(points_text: str, d_expr: str, w_expr: str) -> list[str]:
    # TRAP (verified in the VM): in expression position the compiler
    # parks the left operand as a stack temp before evaluating the
    # right, so a tos[k] inside an arg-position scan reads POLY ITSELF,
    # not the parked uniforms. Every tos-referencing scan below is
    # therefore STATEMENT-position, run while the uniforms pair is the
    # top of stack; weights are parked and applied via multiply(pop),
    # which does read the true pre-statement stack (also verified).
    return [
        f"poly = scan(2, 0, {U[0]}, {U[1]})",
        "poly",
        f"poly = scan(38, 0, {w_expr}, prev)",
        "poly",
        f"poly = scan(2, 0, {U[0]}, {U[1]})",
        "poly",
        f"poly = scan(37, 0, {d_expr}, prev)",
        f"poly = add(poly, vector_literal({points_text}))",
        "poly = expand_roots(poly)",
        "drop",
        "poly = multiply(poly, pop)",
        "drop",
    ]


def build_source_text() -> str:
    return "\n".join(
        # source copy: S lattice + delta*(1+1i), expanded, x (1-a)
        _branch(S_TEXT, D_SRC, f"1-({A_MIX})")
        + ["poly"]
        # target copy: T lattice + the antipodal phase, expanded, x a
        + _branch(T_TEXT, D_TGT, A_MIX)
        + ["poly = add(poly, pop)", "emit"]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2897",
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    output = ROOT / "giga_2897.coeff-program.json"
    rendered = json.dumps(build_payload(), indent=2, ensure_ascii=True) + "\n"
    if args.check:
        current = output.read_text(encoding="utf-8") if output.exists() else ""
        if current != rendered:
            raise SystemExit("FATAL: giga_2897.coeff-program.json is stale; run scripts/gen_giga_2897_coeff_program.py")
        print(f"{output.name}: OK")
    else:
        output.write_text(rendered, encoding="utf-8")
        print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
