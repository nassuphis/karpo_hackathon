#!/usr/bin/env python3
"""Generate the giga_2898 family Coeff Program documents.

Five sibling migrations of poly_chess5 with phi=0 and a coefficient-
space perturbation zfrm (the reference's *_p family: f(cf)*andy + cf):

  run        rloc    deg  rho  andy   zfrm (reference)          native
  giga_2898  rjail2  21   0.1  1e-06  sort_angles_keep_moduli_p sort_angle_keep_mod
  giga_2899  rjail2  21   0.1  0.01   sort_abs_p                sort_abs
  giga_2900  rjail2  21   0.1  1e-09  symmetrize_p              rev + add
  giga_2903  rp3     25   0.1  10.0   sort_abs_p                sort_abs
  giga_2904  rjail6  8    0.2  0.1    sort_abs_p                sort_abs

Shared mechanism (polys/chess.py:279, like giga_2897 but phi=0): both
letterform copies displaced IDENTICALLY by rho*circle(t)*(1+1i) (phi=0,
speed=1 make the two draws the same expression), np.poly both, convex
mix with bimodal_skewed(0.7) — exponent 1/(1-0.7) = 3.333333333333333,
non-integer, so the weight is spelled exp(E*log(.)) — then the
perturbation blend in COEFFICIENT space with the run's andy dial
(1e-09 barely dusts the small slots; 10.0 lets the transform dominate).
The two unseeded reference draws remap to per-run frac-cascade
uniforms of (t1, t2). No root trip in-program (solve=solve); all five
coefficient scales (1e6..1e28) sit inside f32 transport.

Statement-position scans only (the giga_2897 trap: tos[k] inside an
expression-position scan reads poly itself); zfrms are statement-form
native transforms, long deployed.

Parity is pinned by tests/test_giga_2898_family_coeff_programs.py; the
diagnosis lives in recreate_giga_2898_family.md.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# Layout coordinates computed once (at authoring) from the read-only
# reference layout strings (pyroots/poly.py: rjail2 19615, rp3 19653,
# rjail6 19671) through a verbatim replica of polylayout.layout2coord —
# row-major order preserved (np.poly convolves in element order), grid
# centering includes every blank/padded row. Cell counts match the
# recorded degrees: 21/25/8.
LAYOUTS = {
    "rjail2": (
      [
        (-10.5+2j), (-8.5+2j), (-6.5+2j), (-4.5+2j), (-2.5+2j), (-0.5+2j),
        (1.5+2j), (-10.5+1j), (1.5+1j), (-10.5+0j), (-3.5+0j), (1.5+0j),
        (-10.5-1j), (1.5-1j), (-10.5-2j), (-8.5-2j), (-6.5-2j), (-4.5-2j),
        (-2.5-2j), (-0.5-2j), (1.5-2j),
      ],
      [
        (-9.5+2j), (-7.5+2j), (-5.5+2j), (-3.5+2j), (-1.5+2j), (0.5+2j),
        (2.5+2j), (-9.5+1j), (2.5+1j), (-9.5+0j), (2.5+0j), (10.5+0j),
        (-9.5-1j), (2.5-1j), (-9.5-2j), (-7.5-2j), (-5.5-2j), (-3.5-2j),
        (-1.5-2j), (0.5-2j), (2.5-2j),
      ],
    ),
    "rp3": (
      [
        (-12+3j), (-10+3j), (-9+3j), (-8+3j), (-13+2j), (-11+2j), (-9+2j),
        (-12+1j), (-10+1j), (-12+0j), (-10+0j), (6+0j), (7+0j), (8+0j),
        (9+0j), (10+0j), (-12-1j), (-10-1j), (-13-2j), (-11-2j), (-9-2j),
        (-12-3j), (-10-3j), (-9-3j), (-8-3j),
      ],
      [
        (-13+3j), (-11+3j), (-12+2j), (-10+2j), (-16+1j), (-15+1j),
        (-14+1j), (-13+1j), (-11+1j), (-16+0j), (-15+0j), (-14+0j),
        (-13+0j), (-11+0j), (0+0j), (16+0j), (-16-1j), (-15-1j), (-14-1j),
        (-13-1j), (-11-1j), (-12-2j), (-10-2j), (-13-3j), (-11-3j),
      ],
    ),
    "rjail6": (
      [
        (-5+3j), (-5+2j), (-5+1j), (-5+0j), (-4+0j), (-5-1j), (-5-2j),
        (-5-3j),
      ],
      [
        (-4+3j), (-4+2j), (-4+1j), (-3+0j), (5+0j), (-4-1j), (-4-2j),
        (-4-3j),
      ],
    ),
}


# (name, layout, rho literal, andy literal, zfrm kind, (seed1, seed2))
RUNS = [
    ("giga_2898", "rjail2", "0.1", "1e-06", "angles",
     ((6521.4271, 51893.62, 0.15), (3163.9241, 72031.94, 0.85))),
    ("giga_2899", "rjail2", "0.1", "0.01", "sortabs",
     ((7213.5527, 46337.29, 0.45), (2377.8101, 63211.41, 0.05))),
    ("giga_2900", "rjail2", "0.1", "1e-09", "symm",
     ((5851.7057, 54499.86, 0.55), (3697.2251, 69847.13, 0.95))),
    ("giga_2903", "rp3", "0.1", "10.0", "sortabs",
     ((6763.3181, 48611.52, 0.35), (2749.6907, 66293.77, 0.65))),
    ("giga_2904", "rjail6", "0.2", "0.1", "sortabs",
     ((7489.9843, 44293.18, 0.25), (2087.5949, 71411.08, 0.75))),
]

# bimodal_skewed(0.7) exponent, exact f64 of the reference's 1/(1-a)
EXPONENT = "3.333333333333333"


def _lit(z: complex) -> str:
    re = f"{z.real:g}"
    if z.imag == 0:
        return re
    return f"{re}+{z.imag:g}i" if z.imag > 0 else f"{re}{z.imag:g}i"


def _frac(expr: str) -> str:
    return f"({expr})-floor({expr})"


def _uniforms(seeds):
    return [_frac(f"t1*{m1}+t2*{m2}+{c}") for m1, m2, c in seeds]


def _a_mix() -> str:
    e = EXPONENT
    return (
        f"(1-floor(2*tos[1]))*exp({e}*log(2*tos[1]))/2"
        f"+floor(2*tos[1])*(1-exp({e}*log(2-2*tos[1]))/2)"
    )


def _branch(u, points, rho: str, w_expr: str) -> list[str]:
    # Statement-position scans with the uniforms pair parked on top —
    # tos[k] in an expression-position scan would read poly itself
    # (the giga_2897 trap, VM-verified).
    n = len(points)
    pts = ", ".join(_lit(z) for z in points)
    d = f"{rho}*exp(6.283185307179586i*tos[0])*(1+1i)"
    return [
        f"poly = scan(2, 0, {u[0]}, {u[1]})",
        "poly",
        f"poly = scan({n + 1}, 0, {w_expr}, prev)",
        "poly",
        f"poly = scan(2, 0, {u[0]}, {u[1]})",
        "poly",
        f"poly = scan({n}, 0, {d}, prev)",
        f"poly = add(poly, vector_literal({pts}))",
        "poly = expand_roots(poly)",
        "drop",
        "poly = multiply(poly, pop)",
        "drop",
    ]


def _zfrm_p(kind: str, andy: str) -> list[str]:
    # the reference *_p blend: f(mix)*andy + mix, in coefficient space
    if kind == "symm":
        f = ["poly", "poly = rev(poly)", "poly = add(poly, pop)"]
    elif kind == "sortabs":
        f = ["poly = sort_abs(poly)"]
    else:
        f = ["poly = sort_angle_keep_mod(poly)"]
    return ["poly"] + f + [f"poly = multiply(poly, {andy})", "poly = add(poly, pop)"]


def build_source_text(name: str) -> str:
    spec = next(r for r in RUNS if r[0] == name)
    _, layout, rho, andy, kind, seeds = spec
    u = _uniforms(seeds)
    S, T = LAYOUTS[layout]
    a = _a_mix()
    return "\n".join(
        _branch(u, S, rho, f"1-({a})")
        + ["poly"]
        + _branch(u, T, rho, a)
        + ["poly = add(poly, pop)"]
        + _zfrm_p(kind, andy)
        + ["emit"]
    )


def build_payload(name: str) -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": name,
        "chain": [],
        "source_text": build_source_text(name),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    stale = []
    for name, *_ in RUNS:
        output = ROOT / f"{name}.coeff-program.json"
        rendered = json.dumps(build_payload(name), indent=2, ensure_ascii=True) + "\n"
        if args.check:
            current = output.read_text(encoding="utf-8") if output.exists() else ""
            if current != rendered:
                stale.append(output.name)
            else:
                print(f"{output.name}: OK")
        else:
            output.write_text(rendered, encoding="utf-8")
            print(f"Wrote {output}")
    if stale:
        raise SystemExit(f"FATAL: stale documents {stale}; run scripts/gen_giga_2898_family_coeff_programs.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
