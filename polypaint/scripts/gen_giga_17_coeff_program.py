#!/usr/bin/env python3
"""Generate the giga_17 Coeff Program.

giga_17 is the python-era member of the early wave: xfrm 'uc,sum_prod'
-> poly_727 (n=9, degree 8) -> zfrm 'rev,recursive_add' (decay 5e-05),
solve 'safe', view sq2.0 rotate 90.

  z1 = e^(2 pi i u), z2 = e^(2 pi i v)             # uc
  v1 = z1 + z2, v2 = z1 z2 = e^(2 pi i (u+v))      # sum_prod
  for k = 1..9:
    mag = log(|v1| + |v2| + k) * k^2               # |v2| = 1
    ang = angle(v1) sin(k) + angle(v2) cos(k)
    cf[k-1] = mag e^(i ang)
  emit rev(cf)

Diagnosis notes:
- poly_727's rec/imc linspace block is dead code (omitted).
- recursive_add is a decay=5e-05 EMA across ROW ORDER (global rcf,
  per-process under procs=14, so the reference's own EMA content is
  chunk-order noise). NOT ported: measured stateless-vs-one-EMA-step
  root moves median 4.7e-5 / p95 1.0e-4 / max 2.4e-4 against an
  8e-5 pixel at res 50000 — sub-pixel dust (2920-lite precedent,
  with a 2000x smaller decay).
- solve 'safe' knife (sum|cf| outside (1e-10, 1e10) -> zeros): sum|cf|
  measured in [587, 654] over 200 draws — never fires; not ported.
- angle(v2) inlines as 2 pi ((u+v) - floor(u+v+0.5)) (np.angle wrap,
  exact off the half-line); angle(v1)/|v1| ride the parked header.

Parity: tests/test_giga_17_coeff_program.py.
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


def build_source_text() -> str:
    first = "log(tos[1]+2)*exp(1i*(tos[0]*sin(1)+tos[2]*cos(1)))"
    elem = ("log(tos[1]+1+(k+1))*(k+1)**2"
            "*exp(1i*(tos[0]*sin(k+1)+tos[2]*cos(k+1)))")
    lines = [
        "poly = fill(3, 0)",
        f"poly[0] = angle({Z1}+{Z2})",
        f"poly[1] = abs({Z1}+{Z2})",
        f"poly[2] = {W}*((t1+t2)-floor(t1+t2+0.5))",
        "poly",
        f"poly = scan(9, 0, {first}, {elem})",
        "drop",
        "poly = rev(poly)",
        "emit",
    ]
    return "\n".join(lines) + "\n"


def build_payload() -> dict:
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_17",
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    output = ROOT / "giga_17.coeff-program.json"
    rendered = json.dumps(build_payload(), indent=2, ensure_ascii=True) + "\n"
    if args.check:
        current = output.read_text(encoding="utf-8") if output.exists() else ""
        if current != rendered:
            raise SystemExit(
                "FATAL: giga_17.coeff-program.json is stale; run "
                "scripts/gen_giga_17_coeff_program.py")
        print(f"{output.name}: OK")
    else:
        output.write_text(rendered, encoding="utf-8")
        print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
