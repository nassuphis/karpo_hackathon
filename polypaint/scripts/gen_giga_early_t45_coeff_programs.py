#!/usr/bin/env python3
"""Generate giga_18 / giga_19 / giga_20 / giga_21 Coeff Programs
(T4 recurrences + T5 conditional tier of the early wave).

giga_19/20 (deg 89): params are x_01 ONLY (param_space(4000, x_01)) —
t1, t2 are the RAW [0,1] reals, no circle map. Chain:
    cf[1] = t1 - t2            (giga_19)   /   t1 + 1i t2   (giga_20)
    cf[k] = 1i * v/|v|,  v = sin(k cf[k-1]) + cos(k t1)
The R guard (finite && |v| > 1e-10, else t1+t2) is NOT ported: |v|
stays generically far from 0 (the zero set is a curve, grid spacing
2.5e-4, threshold 1e-10) — same class as giga_259's never-firing
'safe' knife. Ascending -> rev.

giga_21 (deg 49): uc params (the saved script's
param_space(4000,x_01,,x_circle) extra-comma typo cannot run in R;
every sibling uses x_01,x_circle). cf[1] = t1 + t2, chain
    v = sin(((k+3) %% 10) cf[k-1]) + cos(((k+1) %% 10) t1), cf[k] = v/|v|
Multipliers via x - 10 floor(x/10). Ascending -> rev.

giga_18 (deg 24): angle-conditional branches on theta_i = Arg(t_i),
computed as 2 pi (t - floor(t + 0.5)) — exact for the app grid except
the u = 0.5 gridline (Arg gives +pi, the formula -pi; only the
(theta1+theta2)^2 terms can tell). Strict x>0 gates become
1 - floor((2-x)/2), EXACT for the bounded gate arguments including
x = 0 (the t1 = t2 diagonal IS on-grid; a division-based sign would
NaN there). Header [theta1, theta2, base, secondary, toggle,
complex_scale, another_scale, sign_flip] parked on the stack;
R %% is floor-based so sign_flip works for negative floors too.
Ascending -> rev.

Parity: tests/test_giga_early_t45_coeff_programs.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

W = "6.283185307179586"
WI = "6.283185307179586i"
Z1 = f"exp({WI}*t1)"
Z2 = f"exp({WI}*t2)"


def Z(a: int, b: int) -> str:
    if a == 0:
        arg = "t2" if b == 1 else f"{b}*t2"
    elif b == 0:
        arg = "t1" if a == 1 else f"{a}*t1"
    else:
        left = "t1" if a == 1 else f"{a}*t1"
        right = "t2" if b == 1 else f"{b}*t2"
        arg = f"({left}+{right})"
    return f"exp({WI}*{arg})"


def _chain(first: str, unit: str) -> list[str]:
    v = "(sin((k+1)*prev)+cos((k+1)*t1))"
    return [
        f"poly = scan(90, 0, {first}, {unit}{v}/abs{v})",
        "poly = rev(poly)",
        "emit",
    ]


def _giga_19() -> list[str]:
    return _chain("t1-t2", "1i*")


def _giga_20() -> list[str]:
    return _chain("t1+1i*t2", "1i*")


def _giga_21() -> list[str]:
    # multipliers (R (k+3)%%10 and (k+1)%%10 at app slot k) packed as
    # one complex vector literal (real=sin mult, imag=cos mult): the
    # floor-based inline form is 70 scalar tokens (cap 64)
    packed = []
    for k in range(50):
        m1 = (k + 4) % 10
        m2 = (k + 2) % 10
        packed.append(f"{m1}+{m2}i" if m2 else f"{m1}")
    v = f"(sin(real(tos[k])*prev)+cos(imag(tos[k])*{Z1}))"
    return [
        f"poly = vector_literal({', '.join(packed)})",
        "poly",
        f"poly = scan(50, 0, {Z1}+{Z2}, {v}/abs{v})",
        "drop",
        "poly = rev(poly)",
        "emit",
    ]


# strict x>0 indicator, exact for x in (-2, 2] including x == 0
def _gt0(x: str) -> str:
    return f"(1-floor((2-({x}))/2))"


def _giga_18() -> list[str]:
    th1 = f"{W}*(t1-floor(t1+0.5))"
    th2 = f"{W}*(t2-floor(t2+0.5))"
    # gates on c = cos(th1+th2) (via tos), bounded in [-1, 1]
    c = "cos(tos[0]+tos[1])"
    ga = _gt0(f"{c}-0.5")
    gb = _gt0(f"-0.5-{c}")
    tog_pos = _gt0("sin(tos[0]-tos[1])")
    return [
        # stage 1: angles
        "poly = fill(2, 0)",
        f"poly[0] = {th1}",
        f"poly[1] = {th2}",
        "poly",
        # stage 2: scales (reads angles via tos)
        "poly = fill(8, 0)",
        "poly[0] = tos[0]",
        "poly[1] = tos[1]",
        "poly[2] = 1000*exp(0.5*sin(10*tos[0]-7*tos[1]))",
        "poly[3] = 500*cos(12*tos[0]+15*tos[1])**3",
        f"poly[4] = {tog_pos}*2000*sin(5*tos[0])*cos(3*tos[1])"
        f"-(1-{tog_pos})*2000*cos(4*tos[0])*sin(2*tos[1])",
        "poly[5] = 300*sin(sin(3*tos[0]+4*tos[1]))**2",
        "poly[6] = 100*exp(sin(tos[0])*cos(tos[1]))",
        "poly[7] = 1-2*(floor(3*(tos[0]+tos[1]))"
        "-2*floor(floor(3*(tos[0]+tos[1]))/2))",
        "poly",                                   # stack: angles, scales
        # the polynomial (ascending slots; tos = scales header)
        "poly = fill(25, 0)",
        "poly[0] = -5",
        f"poly[1] = ({Z(0, 3)}-{Z1})*200*sin(3*tos[0])*sin(tos[1])",
        f"poly[2] = -5*({Z(2, 0)}-{Z(0, 2)})*10*cos(5*tos[1])",
        f"poly[4] = tos[7]*50*({Z(3, 0)}+{Z2})*sin(2*tos[0]-tos[1])",
        f"poly[6] = -tos[6]*{Z(2, 2)}",
        f"poly[8] = ({Z(5, 0)}-{Z2})*tos[5]",
        f"poly[12] = (1-{ga}-{gb})*({Z(4, 7)}-{Z(5, 0)})*tos[2]*tos[3]",
        f"poly[15] = 500*({Z(6, 0)}-{Z(0, 3)})"
        "*sin((tos[0]+tos[1])**2)*cos((tos[0]-tos[1])**2)",
        f"poly[18] = {gb}*({Z(10, 0)}-{Z(0, 10)})*tos[3]*tos[4]",
        f"poly[20] = {ga}*{Z(7, 9)}*tos[2]*tos[4]",
        f"poly[22] = -10*({Z(9, 0)}+{Z(0, 9)})*sin(7*tos[0]-8*tos[1])**3",
        f"poly[24] = ({Z(4, 0)}-{Z(0, 4)})*100*cos(sin(tos[0])*tos[1])"
        "*exp(cos(2*tos[0]-3*tos[1]))",
        "drop",
        "drop",
        "poly = rev(poly)",
        "emit",
    ]


BUILDERS = {
    "giga_18": _giga_18,
    "giga_19": _giga_19,
    "giga_20": _giga_20,
    "giga_21": _giga_21,
}


def build_source_text(name: str) -> str:
    return "\n".join(BUILDERS[name]()) + "\n"


def build_payload(name: str) -> dict:
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
    for name in BUILDERS:
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
        raise SystemExit(
            f"FATAL: stale documents {stale}; run scripts/gen_giga_early_t45_coeff_programs.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
