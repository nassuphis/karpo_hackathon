#!/usr/bin/env python3
"""Generate the giga_259 Coeff Program document.

giga_259 is xfrm 'uc,coeff2' -> poly_373 -> zfrm rev, solve 'safe'
(poly.py poly_373, xfrm.py uc/coeff2, solve.py safe) — fully
DETERMINISTIC in (t1, t2): an exact port like giga_cf10p450, no RNG
remap.

  z1 = e^(2 pi i t1), z2 = e^(2 pi i t2)          # uc
  v1 = z1 + z2,  v2 = z1 * z2                     # coeff2 (symmetric pair)
  for j = 1..35:
    mag = log(|v1|+j) sin(j pi/7) + cos(j pi/11) Re(v2)
    ang = angle(v1) + angle(v2) j + sin(j pi/13)
    cf[j-1] = (mag + Im(v1) cos(j pi/5)) e^(i ang) + conj(v2) sin(j pi/17)
  emit rev(cf)

Notes from the diagnosis:
- poly_373's rec/imc linspace block is DEAD CODE (never used); omitted.
- solve='safe' is the reference's own knife (rows with sum|cf| outside
  (1e-10, 1e10) are dropped) — measured over the sweep it NEVER fires
  here (sum|cf| in [57, 83]), so no gate is ported.
- v2 = z1*z2 = e^(2 pi i (t1+t2)) exactly, so conj(v2) and Re(v2)
  inline without the parked header; only v1-derived quantities (and
  angle(v2)) ride the header.
- coefficients are O(1..7): no transport rescale needed.

Construction: the slot expression exceeds the 64-token scalar cap in
one piece, so the row builds as MAG-scan x PHASE-scan (each against
the parked header P = [angle(v1), angle(v2), |v1|, Re(v2), Im(v1)],
statement-position tos scans, header rebuilt per scan) plus a no-tos
arg-scan tail for conj(v2) sin(j pi/17).

Parity is pinned by tests/test_giga_259_coeff_program.py; the
diagnosis lives in recreate_giga_259.md.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

TWO_PI_I = "6.283185307179586i"
PI7 = "0.4487989505128276"
PI11 = "0.28559933214452665"
PI5 = "0.6283185307179586"
PI13 = "0.241660973353061"
PI17 = "0.18479956785822313"

V1 = f"(exp({TWO_PI_I}*t1)+exp({TWO_PI_I}*t2))"
V2 = f"exp({TWO_PI_I}*(t1+t2))"
V2C = f"exp(0-{TWO_PI_I}*(t1+t2))"

# P = [angle(v1), angle(v2), |v1|, Re(v2), Im(v1)] — rebuilt before each
# tos-referencing scan (pokes are chip-per-op; two rebuilds fit easily)
P_BUILD = [
    "poly = fill(5, 0)",
    f"poly[0] = angle({V1})",
    f"poly[1] = angle({V2})",
    f"poly[2] = abs({V1})",
    f"poly[3] = real({V2})",
    f"poly[4] = imag({V1})",
]

MAG = (f"log(tos[2]+k+1)*sin((k+1)*{PI7})"
       f"+cos((k+1)*{PI11})*tos[3]"
       f"+tos[4]*cos((k+1)*{PI5})")
PHASE = f"exp(1i*(tos[0]+tos[1]*(k+1)+sin((k+1)*{PI13})))"
TAIL = f"{V2C}*sin((k+1)*{PI17})"


def build_source_text() -> str:
    return "\n".join(
        P_BUILD
        + [
            "poly",
            f"poly = scan(35, 0, {MAG}, {MAG})",
            "drop",
            "poly",
        ]
        + P_BUILD
        + [
            "poly",
            f"poly = scan(35, 0, {PHASE}, {PHASE})",
            "drop",
            "poly = multiply(poly, pop)",
            f"poly = add(poly, scan(35, 0, {TAIL}, {TAIL}))",
            "poly = rev(poly)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_259",
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    output = ROOT / "giga_259.coeff-program.json"
    rendered = json.dumps(build_payload(), indent=2, ensure_ascii=True) + "\n"
    if args.check:
        current = output.read_text(encoding="utf-8") if output.exists() else ""
        if current != rendered:
            raise SystemExit("FATAL: giga_259.coeff-program.json is stale; run scripts/gen_giga_259_coeff_program.py")
        print(f"{output.name}: OK")
    else:
        output.write_text(rendered, encoding="utf-8")
        print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
