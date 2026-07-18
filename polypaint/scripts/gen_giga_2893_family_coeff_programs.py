#!/usr/bin/env python3
"""Generate the giga_2893/2894/2895 Coeff Program documents.

Three poly_chess4 runs (polys/chess.py). chess4 differs from chess2/3
structurally: the parity mask SELECTS WHICH CELLS EXIST — only cells
with (row+col+off) % mod != 0 enter the polynomial (no white-square
anchors at all), and BOTH copies displace that same cell set by two
shape-driven offsets coupled through ONE draw t (tt default):

  to = orad * oshape(t * ospeed)         # outer copy
  ti = irad * ishape(t * ispeed + phi)   # inner copy
  cf = np.poly(cells + to*(1+1i)) * b + np.poly(cells + ti*(1+1i)) * (1-b)
  b  = bimodal_skewed(a)                 # second draw

  run        N  deg  inner              outer                a     zfrm
  giga_2893  8  32   0.5*circle(t)      0.5*circle(t)        0.85  none
  giga_2894  8  32   0.3*circle(t+.25)  0.45*opolygon4(t)    0.95  p2_p@1e-18
  giga_2895  12 72   0.25*circle(t)     1.0*circle(0.01*t)   0.85  none

- giga_2893 is ALL DEFAULTS, where inner == outer EXACTLY (same t,
  same radius, same shape, phi=0) — the mix is a no-op and the run is
  a SINGLE displaced half-lattice: one branch, one draw, no bimodal.
- giga_2894's outer shape walks the SQUARE (opolygon, oplgn=4):
  piecewise-linear between the 4th-roots-of-unity vertices, spelled
  with floor gates and i^e = exp(i*pi/2*e). Exponent 1/(1-0.95) =
  19.999999999999982 (f64 truth, not 20). The p2_p zfrm perturbs the
  mix with (cf^2+cf+1)*ramp(1..33)*1e-18 — measured up to 1.4e-2
  relative shatter (the |cf|^2 ~ 1e28 amplification eats most of the
  tiny andy).
- giga_2895 at degree 72 reaches |cf| ~ 9.3e45 — OVER the f32
  transport ceiling (the giga_cf10p450 blank-image mechanism), so the
  program rescales by 1e-15 (roots invariant). Its outer circle moves
  at ospeed=0.01: an almost-frozen offset near (1+1i) against a live
  0.25-radius inner circle.

Construction: 2898-family per-branch weight parking (statement-
position tos scans only), mixes via add(poly, pop) (the nested-pop
one-liner miscomputes — giga_2891 trap log). Cell lists embedded as
coordinate literals in numpy boolean row-major order.

Parity pinned by tests/test_giga_2893_family_coeff_programs.py; the
diagnosis lives in recreate_giga_2893_family.md.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def _cells(n: int, mod: int = 2, off: int = 0) -> list[complex]:
    idx = [i - (n - 1) / 2 for i in range(n)]
    out = []
    for row in range(n):
        for col in range(n):
            if (row + col + off) % mod != 0:
                out.append(complex(idx[col], idx[row]))
    return out


def _lit(z: complex) -> str:
    re = f"{z.real:g}"
    if z.imag == 0:
        return re
    return f"{re}+{z.imag:g}i" if z.imag > 0 else f"{re}{z.imag:g}i"


CELLS8 = _cells(8)
CELLS12 = _cells(12)
CELLS8_TEXT = ", ".join(_lit(z) for z in CELLS8)
CELLS12_TEXT = ", ".join(_lit(z) for z in CELLS12)

TWO_PI_I = "6.283185307179586i"
HALF_PI_I = "1.5707963267948966i"

# fresh frac-cascade seed pairs per run: (t shape draw, u bimodal draw)
SEEDS = {
    "giga_2893": ((5701.9333, 61879.31, 0.15), None),
    "giga_2894": ((6301.7351, 54017.83, 0.45), (2909.4643, 69931.19, 0.75)),
    "giga_2895": ((7351.2287, 47143.67, 0.55), (3373.8629, 64613.41, 0.25)),
}


def _frac(expr: str) -> str:
    return f"({expr})-floor({expr})"


def _u(seed) -> str:
    m1, m2, c = seed
    return _frac(f"t1*{m1}+t2*{m2}+{c}")


def _a_mix(exponent: str) -> str:
    u = "tos[1]"
    return (
        f"(1-floor(2*{u}))*exp({exponent}*log(2*{u}))/2"
        f"+floor(2*{u})*(1-exp({exponent}*log(2-2*{u}))/2)"
    )


def _opolygon4(t: str) -> str:
    # piecewise-linear walk on the square's vertices i^e, e=floor(4t):
    # (1-f)*i^e + f*i^(e+1) with i^e = exp(i*pi/2*e)
    f = f"(4*{t}-floor(4*{t}))"
    e = f"floor(4*{t})"
    return (f"(1-{f})*exp({HALF_PI_I}*{e})"
            f"+{f}*exp({HALF_PI_I}*({e}+1))")


def _branch(cells_text: str, degree: int, t_seed, u_seed,
            d_expr: str, w_expr: str) -> list[str]:
    pair = f"poly = scan(2, 0, {_u(t_seed)}, {_u(u_seed)})"
    return [
        pair,
        "poly",
        f"poly = scan({degree + 1}, 0, {w_expr}, prev)",
        "poly",
        pair,
        "poly",
        f"poly = scan({degree}, 0, {d_expr}, prev)",
        f"poly = add(poly, vector_literal({cells_text}))",
        "drop",
        "poly = expand_roots(poly)",
        "poly = multiply(poly, pop)",
        "drop",
    ]


def _source_2893() -> str:
    t_seed = SEEDS["giga_2893"][0]
    d = f"0.5*exp({TWO_PI_I}*tos[0])*(1+1i)"
    return "\n".join([
        f"poly = scan(1, 0, {_u(t_seed)}, {_u(t_seed)})",
        "poly",
        f"poly = scan(32, 0, {d}, prev)",
        f"poly = add(poly, vector_literal({CELLS8_TEXT}))",
        "drop",
        "poly = expand_roots(poly)",
        "emit",
    ])


def _source_2894() -> str:
    t_seed, u_seed = SEEDS["giga_2894"]
    a = _a_mix("19.999999999999982")
    d_inner = f"0.3*exp({TWO_PI_I}*(tos[0]+0.25))*(1+1i)"
    d_outer = f"0.45*({_opolygon4('tos[0]')})*(1+1i)"
    return "\n".join(
        _branch(CELLS8_TEXT, 32, t_seed, u_seed, d_inner, f"1-({a})")
        + ["poly"]
        + _branch(CELLS8_TEXT, 32, t_seed, u_seed, d_outer, a)
        + [
            "poly = add(poly, pop)",
            # p2_p (andy=1e-18): (cf^2 + cf + 1) * ramp(1..33) * andy + cf
            "poly",
            "poly",
            "poly",
            "poly = multiply(poly, pop)",
            "poly = add(poly, pop)",
            "poly = add(poly, fill(33, 1))",
            "poly = multiply(poly, scan(33, 0, 1, prev+1))",
            "poly = multiply(poly, 1e-18)",
            "poly = add(poly, pop)",
            "emit",
        ]
    )


def _source_2895() -> str:
    t_seed, u_seed = SEEDS["giga_2895"]
    a = _a_mix("6.666666666666666")
    d_inner = f"0.25*exp({TWO_PI_I}*tos[0])*(1+1i)"
    d_outer = "exp(0.06283185307179587i*tos[0])*(1+1i)"
    return "\n".join(
        _branch(CELLS12_TEXT, 72, t_seed, u_seed, d_inner, f"1-({a})")
        + ["poly"]
        + _branch(CELLS12_TEXT, 72, t_seed, u_seed, d_outer, a)
        + [
            "poly = add(poly, pop)",
            # degree-72 coefficients reach ~9.3e45 — over the f32
            # transport ceiling; rescale (roots invariant)
            "poly = multiply(poly, 1e-15)",
            "emit",
        ]
    )


SOURCES = {
    "giga_2893": _source_2893,
    "giga_2894": _source_2894,
    "giga_2895": _source_2895,
}


def build_source_text(name: str) -> str:
    return SOURCES[name]()


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
    for name in SOURCES:
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
        raise SystemExit(f"FATAL: stale documents {stale}; run scripts/gen_giga_2893_family_coeff_programs.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
