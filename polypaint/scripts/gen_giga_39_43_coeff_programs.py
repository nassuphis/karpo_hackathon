#!/usr/bin/env python3
"""Generate giga_39..giga_43 Coeff Programs.

Five python-era static slot-fill polynomials (polys/giga.py, identical
in poly.py). The driver is polybook.sh:

    -x unit_circle  -z rev  -s safe

so t1, t2 are unit-circle points (unit_circle: e^(2 pi i t)), the
coefficient vector is REVERSED before solving (zfrm rev = np.flip), and
'safe' is the reference's knife (sum|cf| outside (1e-10, 1e10), or
NaN/inf, -> zeros).

Unlike the earlier T1 wave (giga_1..30), these apply np.sin / np.exp /
powers DIRECTLY to the unit-circle points, so the monomial->exp folding
does NOT apply. Each slot is written with U1 = e^(2 pi i t1) and
U2 = e^(2 pi i t2) as the actual complex arguments, e.g. np.sin(t1)
becomes sin(U1) and t1**2 becomes U1**2 (= e^(2 pi i * 2 t1)).

'safe' NEVER fires here: measured sum|cf| across 500 draws each stays
O(44..1382) (never near 1e-10 or 1e10) and no coefficient is NaN/inf —
so no gate is ported (the giga_259 precedent). numpy-descending direct;
the program builds poly[k] = cf[k] in the def's own index order, then
`poly = rev(poly)` mirrors the zfrm.

Parity: tests/test_giga_39_43_coeff_programs.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

WI = "6.283185307179586i"
U1 = f"exp({WI}*t1)"
U2 = f"exp({WI}*t2)"

# name -> (length, {index: expression}). Expressions use U1/U2 (the
# unit-circle points) exactly as the numpy def uses t1/t2.
GIGAS = {
    "giga_39": (50, {
        0: "1", 9: "2", 19: "-3", 29: "4", 39: "-5", 49: "6",
        14: f"100*({U1}**2+{U2}**2)",
        24: f"50*(sin({U1})+1i*cos({U2}))",
        34: f"200*({U1}*{U2})+1i*({U1}**3-{U2}**3)",
        44: f"exp(1i*({U1}+{U2}))+exp(-1i*({U1}-{U2}))",
    }),
    "giga_40": (35, {
        0: "1", 6: "-2", 14: "3", 19: "-4", 26: "5", 34: "-6",
        11: f"50i*sin({U1}**2-{U2}**2)",
        17: f"100*(cos({U1})+1i*sin({U2}))",
        24: f"50*({U1}**3-{U2}**3+1i*{U1}*{U2})",
        29: f"200*exp(1i*{U1})+50*exp(-1i*{U2})",
    }),
    "giga_41": (60, {
        0: "1", 9: "-5", 29: "10", 49: "-20",
        19: f"100*exp({U1}+{U2})",
        39: f"50*({U1}**2*{U2}+1i*{U2}**2)",
        54: f"exp(1i*{U1})*exp(-1i*{U2})+50*{U1}**3",
        59: f"300*sin({U1}+{U2})+1i*cos({U1}-{U2})",
    }),
    "giga_42": (50, {
        0: "1", 7: "-3", 15: "3", 31: "-1", 39: "2",
        11: f"100i*exp({U1}**2+{U2}**2)",
        19: f"50*({U1}**3+{U2}**3)",
        24: f"exp(1i*({U1}-{U2}))+10*{U1}**2",
        44: f"200*sin({U1}+{U2})+1i*cos({U1}-{U2})",
    }),
    "giga_43": (40, {
        0: "1", 4: "-5", 14: "10", 29: "-20",
        19: f"100i*({U1}**3-{U2}**3)",
        9: f"50*({U1}**2*{U2}+1i*{U2}**2)",
        24: f"exp(1i*{U1})+exp(-1i*{U2})",
        34: f"200*{U1}*{U2}*sin({U1}+{U2})",
    }),
}


def build_source_text(name: str) -> str:
    n, cf = GIGAS[name]
    lines = [f"poly = fill({n}, 0)"]
    for idx in sorted(cf):
        lines.append(f"poly[{idx}] = {cf[idx]}")
    lines.append("poly = rev(poly)")   # zfrm rev
    lines.append("emit")
    return "\n".join(lines) + "\n"


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
    for name in GIGAS:
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
            f"FATAL: stale documents {stale}; run scripts/gen_giga_39_43_coeff_programs.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
