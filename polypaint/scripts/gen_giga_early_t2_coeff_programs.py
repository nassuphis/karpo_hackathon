#!/usr/bin/env python3
"""Generate giga_7 / giga_9 / giga_10 / giga_11 Coeff Programs (T2:
vector-formula tier of the early wave).

All four are R-era (base::polyroot, ASCENDING cf -> `poly = rev(poly)`
at the end) over uc params (t1 = e^(2 pi i u), t2 = e^(2 pi i v)).
R loops/sequences are 1-based; the app scan element k IS the 0-based
slot index (element 0 comes from the scan's `first` argument), so an
R formula F_R(k) lands as first = F_R(1), element = F_R(k+1), and a
1-based R linspace seq(a, b, length.out=n)[k] becomes a + (b-a)*k/(n-1)
directly in app-k.

giga_7  (deg 29): cf[k] = exp(1i sin(10 pi imc[k])) + exp(1i cos(10 pi rec[k]))
         rec = linspace(Re t1, Re t2, 30), imc = linspace(Im t1, Im t2, 30)
giga_9  (deg 19): cf[k] = 100i imc[k]^9 + 100 rec[k]^9, 20 points
giga_10 (deg 119): 120-term k-formula in re1/im1/re2/im2 with three
         closed-form slot overrides (cf[30]+=1000i, cf[60]-=500,
         cf[90]+=250 e^(1i t1 t2)); header [re1, im1, re2, im2] parked
         on the stack (the 64-token scalar cap), dropped before emit.
giga_11 (deg 39): m = trunc(5|t1+t2|)+1 (the R %%17 is a no-op: the
         argument never exceeds 10); cf[k] = ((k-1) mod m) *
         exp(1i pi k / (m + t1 + t2)); m parked on the stack; mod via
         x - m*floor(x/m).

Parity: tests/test_giga_early_t2_coeff_programs.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

W = "6.283185307179586"      # 2 pi (real)
WI = "6.283185307179586i"    # 2 pi i
TEN_PI = "31.41592653589793"
PI_I = "3.141592653589793i"

RE1 = f"cos({W}*t1)"
IM1 = f"sin({W}*t1)"
RE2 = f"cos({W}*t2)"
IM2 = f"sin({W}*t2)"
Z1 = f"exp({WI}*t1)"
Z2 = f"exp({WI}*t2)"


def _linspace(a: str, b: str, n: int, k: str) -> str:
    return f"({a}+({b}-{a})*{k}/{n - 1})"


def _giga_7() -> list[str]:
    rec = _linspace(RE1, RE2, 30, "k")
    imc = _linspace(IM1, IM2, 30, "k")
    first = f"exp(1i*sin({TEN_PI}*{IM1}))+exp(1i*cos({TEN_PI}*{RE1}))"
    elem = f"exp(1i*sin({TEN_PI}*{imc}))+exp(1i*cos({TEN_PI}*{rec}))"
    return [
        f"poly = scan(30, 0, {first}, {elem})",
        "poly = rev(poly)",
        "emit",
    ]


def _giga_9() -> list[str]:
    # x**9 expands past the 64-token scalar cap even with a parked
    # header -> elementwise ninth power via square-and-multiply on the
    # vector: p2=p*p, p4=p2*p2, p8=p4*p4, p9=p8*p (pushed copy)
    rec = _linspace(RE1, RE2, 20, "k")
    imc = _linspace(IM1, IM2, 20, "k")
    ninth = [
        "poly",
        "poly = multiply(poly, poly)",
        "poly = multiply(poly, poly)",
        "poly = multiply(poly, poly)",
        "poly = multiply(poly, pop)",
    ]
    return (
        [f"poly = scan(20, 0, {RE1}, {rec})"]
        + ninth
        + ["poly = multiply(poly, 100)", "poly",
           f"poly = scan(20, 0, {IM1}, {imc})"]
        + ninth
        + ["poly = multiply(poly, 100i)",
           "poly = add(poly, pop)",
           "poly = rev(poly)",
           "emit"]
    )


def _giga_10_term(k: str) -> str:
    # R formula at 1-based k, header parked as [re1, im1, re2, im2]
    return (f"(100*(tos[0]+tos[3])*(({k})/10)**2)*exp(1i*(tos[2]*({k})/20))"
            f"+(50*(tos[1]-tos[2])*sin(({k})*0.1*tos[3]))"
            f"*exp(-1i*({k})*0.05*tos[0])")


def _giga_10() -> list[str]:
    return [
        "poly = fill(4, 0)",
        f"poly[0] = {RE1}",
        f"poly[1] = {IM1}",
        f"poly[2] = {RE2}",
        f"poly[3] = {IM2}",
        "poly",
        f"poly = scan(120, 0, {_giga_10_term('1')}, {_giga_10_term('k+1')})",
        f"poly[29] = {_giga_10_term('30')}+1000i",
        f"poly[59] = {_giga_10_term('60')}-500",
        f"poly[89] = {_giga_10_term('90')}+250*exp(1i*exp({WI}*(t1+t2)))",
        "drop",
        "poly = rev(poly)",
        "emit",
    ]


def _giga_11() -> list[str]:
    denom = f"(tos[0]+{Z1}+{Z2})"
    elem = f"(k-tos[0]*floor(k/tos[0]))*exp({PI_I}*(k+1)/{denom})"
    return [
        "poly = fill(1, 0)",
        f"poly[0] = floor(5*abs({Z1}+{Z2}))+1",
        "poly",
        f"poly = scan(40, 0, 0, {elem})",
        "drop",
        "poly = rev(poly)",
        "emit",
    ]


BUILDERS = {
    "giga_7": _giga_7,
    "giga_9": _giga_9,
    "giga_10": _giga_10,
    "giga_11": _giga_11,
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
            f"FATAL: stale documents {stale}; run scripts/gen_giga_early_t2_coeff_programs.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
