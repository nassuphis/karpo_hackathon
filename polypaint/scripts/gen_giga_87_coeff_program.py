#!/usr/bin/env python3
"""Generate the giga_87 Coeff Program.

giga_87 is a python-era (png 2025-01-11) DENSE 51-coefficient slot-fill
poly (poly.py / polys/giga.py `poly_giga_87`, 0-indexed — the R version
in poly.csv is a buggy 1-indexed transliteration that changes the
per-index arithmetic, so the python def is authoritative for the render).
The driver is the same as the giga_39..43 wave:

    -x unit_circle  -z rev  -s safe

so t1, t2 are unit-circle points (unit_circle: e^(2 pi i t)), the
coefficient vector is REVERSED before solving (zfrm rev = np.flip), and
'safe' is the reference's knife (sum|cf| outside (1e-10, 1e10), or
NaN/inf, -> zeros).

Unlike the sparse 39..43 fills, giga_87 is a DENSE k-formula:

    cf[0] = t1 + t2
    cf[1] = 1 + t1*t2 + log(|t1 + t2| + 1)
    cf[2] = t1 + t2 + log(|1 - t1*t2| + 1)
    for i in 3..50: cf[i] = i*t1 + (51-i)*t2 + log(|t1 - i*t2| + 1)
    # then five SEQUENTIALLY-DEPENDENT slot overrides (order matters):
    cf[10] = cf[0] + cf[9]  - sin(t1)
    cf[20] = cf[30] + cf[40] - cos(t2)   # reads the LOOP cf[30], cf[40]
    cf[30] = cf[20] + cf[40] + sin(t1)   # reads the NEW  cf[20]
    cf[40] = cf[30] + cf[20] - cos(t2)   # reads the NEW  cf[30], cf[20]
    cf[50] = cf[40] + cf[20] + sin(t2)   # reads the NEW  cf[40], cf[20]

The k-formula is a DIRECT map (no cf[i-1] dependency), so it compiles to
a single `scan(51, 0, F, F)` whose init and step are the SAME expression F
(the scan's `prev` is unused; every element k -> F(k)). scan fills all 51
slots incl. 0/1/2, which the statements below override — identical to the
numpy def where cf[0..2] are set before the loop and cf[10/20/30/40/50]
are set after it. The five overrides read `poly[i]` in statement order, so
the exact sequential dependency of the numpy def is preserved.

'safe' NEVER fires: sum|cf| across 500 random draws stays O(1341..3310)
(never near 1e-10 or 1e10) and every coefficient is finite (abs()+1 >= 1
so log(...) is always finite), so no gate is ported (the giga_259
precedent). numpy-descending direct; the program builds poly[k] = cf[k] in
the def's own index order, then `poly = rev(poly)` mirrors the zfrm.

Parity: tests/test_giga_87_coeff_program.py (machine-floor: coeff rel
<= 5e-18, root multiset <= 6e-15 at the probes).
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

NAME = "giga_87"
WI = "6.283185307179586i"
U1 = f"exp({WI}*t1)"                       # the unit_circle point e^(2 pi i t1)
U2 = f"exp({WI}*t2)"
# the general per-index coefficient F(k) = k*U1 + (51-k)*U2 + log(|U1 - k*U2| + 1)
F = f"k*{U1}+(51-k)*{U2}+log(abs({U1}-k*{U2})+1)"


def build_source_text() -> str:
    lines = [
        # DIRECT k-map: init == step == F, so element k -> F(k) (prev unused)
        f"poly = scan(51, 0, {F}, {F})",
        # cf[0..2] special values (numpy sets these before the loop)
        f"poly[0] = {U1}+{U2}",
        f"poly[1] = 1+{U1}*{U2}+log(abs({U1}+{U2})+1)",
        f"poly[2] = {U1}+{U2}+log(abs(1-{U1}*{U2})+1)",
        # the five sequentially-dependent overrides, in the numpy def's order
        f"poly[10] = poly[0]+poly[9]-sin({U1})",
        f"poly[20] = poly[30]+poly[40]-cos({U2})",
        f"poly[30] = poly[20]+poly[40]+sin({U1})",
        f"poly[40] = poly[30]+poly[20]-cos({U2})",
        f"poly[50] = poly[40]+poly[20]+sin({U2})",
        "poly = rev(poly)",   # zfrm rev
        "emit",
    ]
    return "\n".join(lines) + "\n"


def build_document() -> dict:
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": NAME,
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="verify the on-disk JSON matches this generator")
    args = parser.parse_args()

    path = ROOT / f"{NAME}.coeff-program.json"
    document = build_document()
    serialized = json.dumps(document, indent=2) + "\n"

    if args.check:
        if not path.exists():
            print(f"MISSING {path}")
            return 1
        if path.read_text() != serialized:
            print(f"STALE {path} — re-run scripts/gen_giga_87_coeff_program.py")
            return 1
        print(f"OK {path}")
        return 0

    path.write_text(serialized)
    print(f"wrote {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
