#!/usr/bin/env python3
"""Generate giga_8 and giga_27 Coeff Programs (T3: roots-inside-the-
coefficients tier of the early wave).

giga_8 (R era, ascending -> rev, deg 34): two chained cubic solves
whose ROOTS become coefficient slots.
    roots1 = rep(polyroot(c(t1^3, -50 t2, 100 t1, 10i)), 4) = [r1,r2,r3,r1]
    roots2 = rep(polyroot(c(1, r1, -Re r2, Im r3)), 4)      = [s1,s2,s3,s1]
    cf[1:4] = roots1; cf[10:13] = roots2
    cf[20] = 50 t1 t2 + Re(s1); cf[30] = e^(1i t1) + 50 t2^3
    cf[35] = 200 e^(1i t1^3) - e^(-1i t2^2)
R's polyroot is cpoly (Jenkins-Traub, smallest-root-first). MEASURED
at authoring: the app's roots_jt returns the SAME roots in the SAME
order as R polyroot on giga_8's cubics (6 Rscript probes, exact) — so
roots_jt is faithful INCLUDING order. np.roots came out reversed.

giga_27 (python era, numpy-descending direct, deg 11): polys/giga.py
    cf[0:3] = -100i; cf[[4,5,6]] = 100*np.roots([t1, t2, t1, 1])
    cf[[11,10,9]] = 100*np.roots([t2, t1, t2, 10i])
np.roots = LAPACK companion order -> roots_cm (same eigensolve).
Needs the LAPACK build (cloud has it; local parity uses the
Accelerate-linked test binary).

Both cubics keep |leading| = 1 on uc params — no degree drop; the
strip mode is `exact` so a synthetic zero lead would still behave.
Parity: tests/test_giga_early_t3_coeff_programs.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

WI = "6.283185307179586i"
Z1 = f"exp({WI}*t1)"
Z2 = f"exp({WI}*t2)"


def _giga_8() -> list[str]:
    return [
        # cubic 1, descending [10i, 100 t1, -50 t2, t1^3]
        "poly = fill(4, 0)",
        "poly[0] = 10i",
        f"poly[1] = 100*{Z1}",
        f"poly[2] = -50*{Z2}",
        f"poly[3] = exp({WI}*3*t1)",
        "poly = roots_jt(poly, lo, exact)",
        "poly = poly[0:3]",
        "poly",                                     # push R1
        # cubic 2, descending [Im r3, -Re r2, r1, 1]
        "poly = fill(4, 0)",
        "poly[0] = imag(tos[2])",
        "poly[1] = -real(tos[1])",
        "poly[2] = tos[0]",
        "poly[3] = 1",
        "poly = roots_jt(poly, lo, exact)",
        "poly = poly[0:3]",
        "poly",                                     # push R2 (stack: R1 R2)
        "poly = fill(35, 0)",
        "poly[9] = tos[0]",
        "poly[10] = tos[1]",
        "poly[11] = tos[2]",
        "poly[12] = tos[0]",
        f"poly[19] = 50*exp({WI}*(t1+t2))+real(tos[0])",
        "drop",                                     # stack: R1
        "poly[0] = tos[0]",
        "poly[1] = tos[1]",
        "poly[2] = tos[2]",
        "poly[3] = tos[0]",
        "drop",
        f"poly[29] = exp(1i*{Z1})+50*exp({WI}*3*t2)",
        f"poly[34] = 200*exp(1i*exp({WI}*3*t1))-exp(-1i*exp({WI}*2*t2))",
        "poly = rev(poly)",
        "emit",
    ]


def _giga_27() -> list[str]:
    return [
        # cubic 1, np-descending [t1, t2, t1, 1]
        "poly = fill(4, 0)",
        f"poly[0] = {Z1}",
        f"poly[1] = {Z2}",
        f"poly[2] = {Z1}",
        "poly[3] = 1",
        "poly = roots_cm(poly, lo, exact)",
        "poly = poly[0:3]",
        "poly = multiply(poly, 100)",
        "poly",                                     # push M (mid roots)
        # cubic 2, np-descending [t2, t1, t2, 10i]
        "poly = fill(4, 0)",
        f"poly[0] = {Z2}",
        f"poly[1] = {Z1}",
        f"poly[2] = {Z2}",
        "poly[3] = 10i",
        "poly = roots_cm(poly, lo, exact)",
        "poly = poly[0:3]",
        "poly = multiply(poly, 100)",
        "poly",                                     # push E (stack: M E)
        "poly = fill(12, 0)",
        "poly[0] = -100i",
        "poly[1] = -100i",
        "poly[2] = -100i",
        "poly[11] = tos[0]",
        "poly[10] = tos[1]",
        "poly[9] = tos[2]",
        "drop",                                     # stack: M
        "poly[4] = tos[0]",
        "poly[5] = tos[1]",
        "poly[6] = tos[2]",
        "drop",
        "emit",
    ]


BUILDERS = {
    "giga_8": _giga_8,
    "giga_27": _giga_27,
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
            f"FATAL: stale documents {stale}; run scripts/gen_giga_early_t3_coeff_programs.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
