#!/usr/bin/env python3
"""Generate the giga_cf10p450 Coeff Program document.

giga_cf10p450 is xfrm 'unit_circle,coeff10' -> poly_450 -> zfrm rev
(poly.py:11298, xfrm.py:77/315), fully DETERMINISTIC in (t1, t2) — no
unseeded draws, so this is an exact port, not an RNG remap:

  z1 = e^(2*pi*i*t1), z2 = e^(2*pi*i*t2)              # unit_circle
  v1 = (z1^4 - z2)/(z1^4 + z2), v2 = (z2^4 - z1)/(z2^4 + z1)  # coeff10
  A1 = angle(v1), A2 = angle(v2), M = |v1| + |v2|
  base[j-1] = log(M+j)*(1+0.1j) * (cos(AP+PS) + i*sin(AP-PS)), j=1..35
      AP = sin(j*A1 + cos(j*A2)); PS = A1*cos(j*pi/35) - A2*sin(j*pi/35)
  base[35-k] = base[35-k]*conj(v1)^((k%3)+1) + conj(v2)^(k%4), k=1..5
  cf[r-1] = base[r-1] + cumprod(M+1..M+r)/(r+1)        # the ladder
  emit rev(cf) * 1e-30                                 # zfrm rev + RESCALE

The 1e-30 rescale is the migration's one addition: the cumulative
ladder reaches ~3e41 at the median row (heavy pole tail beyond), which
is fine in the reference's f64 pipeline but overflows this pipeline's
f32 coefficient transport (3.4e38) on 100% of rows — the reason the
user's semantically-correct UI chain (legacy unit_circle + coeff10
param transforms, poly_450 port, rev) renders NOTHING. Roots are
scale-invariant; after *1e-30 the smallest slots (~0.45 raw) sit at
4.5e-31 (comfortably normal f32) and only the ~1.85% most pole-adjacent
rows still overflow and get knifed.

The construction inverts the giga_2897 trap into a tool (see the
comment above U below): a self-carrying [P6 | payload] accumulator
whose gated arg-scans read the header through tos — the parked left
operand of multiply(poly, scan(...)) IS tos during the arg-scan. That
is what fits the two hard caps at once (256 chips total, pokes are
chip-per-op; 64 tokens per scalar expression). k is available in scan
init exprs (k=0), so one text serves init and step.

Parity is pinned by tests/test_giga_cf10p450_coeff_program.py; the
diagnosis lives in recreate_giga_cf10p450.md.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

E8 = "25.132741228718345"     # 8*pi   (z^4 on the unit circle)
E2 = "6.283185307179586"      # 2*pi
P35 = "0.08975979010256552"   # pi/35
SCALE = "1e-30"

V1 = f"(exp({E8}i*t1)-exp({E2}i*t2))/(exp({E8}i*t1)+exp({E2}i*t2))"
V2 = f"(exp({E8}i*t2)-exp({E2}i*t1))/(exp({E8}i*t2)+exp({E2}i*t1))"

# Two hard caps shape this program: 256 chips total (pokes compile
# chip-per-op, and each V1/V2 instance costs ~14 chips) and 64 TOKENS
# per scalar expression (the direct base spelling needs 90). The
# design that fits both turns the giga_2897 trap into the tool: in
# multiply(poly, scan(...)), the parked left operand IS tos during the
# arg-scan. So the accumulator CARRIES its own header — length 41 =
# [P6 | 35 payload slots] with P6 = [A1, A2, M, pi/35, conj v1,
# conj v2] — and every gated arg-scan reads the header from tos[0..5]
# while writing the payload. The gate u = floor((k+34)/40) is 0 on
# header slots, 1 on payload (j = k-5); pass-1 gates preserve the
# header through products, and the last factor of the second/fourth
# chains zeroes it so add(poly, pop) keeps a SINGLE header copy.
U = "floor((k+34)/40)"
J = "(k-5)"

# base row, exact identity cos(S+P)+i*sin(S-P) = cosP*e^{iS}-i*sinP*e^{-iS}
_S = f"sin({J}*tos[0]+cos({J}*tos[1]))"
_P = f"tos[0]*cos({J}*tos[3])-tos[1]*sin({J}*tos[3])"
G_EIS = f"(1-{U})+{U}*exp(1i*{_S})"
G_COSP = f"(1-{U})+{U}*cos({_P})"
G_MAG1 = f"(1-{U})+{U}*log(tos[2]+{J})*(1+0.1*{J})"
G_ENIS = f"(1-{U})+{U}*exp(0-1i*{_S})"
G_SINP = f"(1-{U})+{U}*sin({_P})"
G_MAGNI0 = f"{U}*(0-1i)*log(tos[2]+{J})*(1+0.1*{J})"
# the cumulative ladder: prev chains through the all-1 header region,
# so cum(j) = prod(M+1..M+j) starts clean at k=6; the divisor gate
# k-4+7*(1-u) avoids the k=4 zero divisor on header slots
G_CUM = f"(1-{U})+{U}*prev*(tos[2]+k-5)"
G_INV0 = f"{U}/(k-4+7*(1-{U}))"

P6_POKES = [
    "poly[0] = angle(tos[0])",
    "poly[1] = angle(tos[1])",
    "poly[2] = abs(tos[0])+abs(tos[1])",
    f"poly[3] = {P35}",
    "poly[4] = conj(tos[0])",
    "poly[5] = conj(tos[1])",
]
COPY_POKES = [f"poly[{i}] = tos[{i}]" for i in range(6)]


def build_source_text() -> str:
    return "\n".join(
        # Z = [v1, v2] (the two heavy pokes), consumed by the P6 header
        [
            "poly = fill(2, 0)",
            f"poly[0] = {V1}",
            f"poly[1] = {V2}",
            "poly",
        ]
        + ["poly = fill(41, 1)"] + P6_POKES + ["drop"]
        # T1 = MAG.cosP.e^{iS} on the payload, header preserved
        + [
            f"poly = multiply(poly, scan(41, 0, {G_EIS}, {G_EIS}))",
            f"poly = multiply(poly, scan(41, 0, {G_COSP}, {G_COSP}))",
            f"poly = multiply(poly, scan(41, 0, {G_MAG1}, {G_MAG1}))",
            "poly",
        ]
        # T2 chain ends in (-i*MAG) with a ZEROING gate, so the add
        # keeps one header copy: poly = [P6 | T1 - i*T2] = [P6 | base]
        + ["poly = fill(41, 1)"] + COPY_POKES + [
            f"poly = multiply(poly, scan(41, 0, {G_ENIS}, {G_ENIS}))",
            f"poly = multiply(poly, scan(41, 0, {G_SINP}, {G_SINP}))",
            f"poly = multiply(poly, scan(41, 0, {G_MAGNI0}, {G_MAGNI0}))",
            "poly = add(poly, pop)",
        ]
        # reference order: conj corrections on the BASE (self-slot
        # reads: header carries conj v1/v2 at slots 4/5), ladder after
        + [
            "poly[40] = poly[40]*poly[4]*poly[4]+poly[5]",
            "poly[39] = poly[39]*poly[4]*poly[4]*poly[4]+poly[5]*poly[5]",
            "poly[38] = poly[38]*poly[4]+poly[5]*poly[5]*poly[5]",
            "poly[37] = poly[37]*poly[4]*poly[4]+1",
            "poly[36] = poly[36]*poly[4]*poly[4]*poly[4]+poly[5]",
            "poly",
        ]
        # ladder D = cumprod(M+1..M+j)/(j+1), zero-header, added on
        + ["poly = fill(41, 1)"] + COPY_POKES + [
            f"poly = multiply(poly, scan(41, 0, {G_CUM}, {G_CUM}))",
            f"poly = multiply(poly, scan(41, 0, {G_INV0}, {G_INV0}))",
            "poly = add(poly, pop)",
        ]
        # strip the header; zfrm rev; the f32-transport rescue
        + [
            "poly = poly[6:41]",
            "poly = rev(poly)",
            f"poly = multiply(poly, {SCALE})",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_cf10p450",
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    output = ROOT / "giga_cf10p450.coeff-program.json"
    rendered = json.dumps(build_payload(), indent=2, ensure_ascii=True) + "\n"
    if args.check:
        current = output.read_text(encoding="utf-8") if output.exists() else ""
        if current != rendered:
            raise SystemExit("FATAL: giga_cf10p450.coeff-program.json is stale; run scripts/gen_giga_cf10p450_coeff_program.py")
        print(f"{output.name}: OK")
    else:
        output.write_text(rendered, encoding="utf-8")
        print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
