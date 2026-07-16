#!/usr/bin/env python3
"""Generate the portable giga_0164 Coeff Program document.

giga_0164 is from the EARLY options-format era, whose polyfun.txt inlines
the complete verbatim source — no repository archaeology needed:

    z01:      t1' = t1 + 1j*t2,  t2' = t2 + 1j*t1     (mirrored pair)
    poly_445: 35 coefficients, per-j trig tables (k = (5j+2)%12+1):
              cf[j] = log(|t1'|+j)*|sin(j pi/10)|
                      * (t1*sin(j pi/k) + t2*cos(j pi/(k+1))
                         + 1j*(t2*cos(j pi/k) - t1*sin(j pi/(k+1))))
                      * (cos(A_j) + 1j*sin(A_j)),
              A_j = angle(t1')*cos(j pi/8) + angle(t2')*sin(j pi/9)
    p8:       (z^8+...+z+1) * arange(1..35)   — the native power_series
              transform IS this (its C already scales by index+1)
    invp3:    s = z^3+z^2+z+1; 1/s where |s|>1 else 1  — native invpower
    roots:    np.roots feedback (its sum<1e-10 guard NEVER fires:
              0/5000 rows; max |cf| per row is 0.2..1.0)

The seven per-j trig tables ride pool constants (numpy's exact doubles);
the row scalars |t1'|, angle(t1'), angle(t2') are scalar expressions.
The roots feedback is roots_cm(hi, exact) — |cf| reaches 3.6e-25, so the
exact-strip mode is load-bearing again, and the zgeev emission order
matches the reference's np.roots (the vector IS the emitted polynomial,
so order is position-significant). The hi pad's leading zero strips in
the solver, reproducing the reference's degree-33 solve of the 34-root
vector.

SOURCE TRAP recorded: `power(poly, 8)` lowers to the TYPED elementwise
power (z^8); the series transform's source spelling is
`power_series(poly, 8)` (its registry name is shadowed).

Parity is pinned by tests/test_giga_0164_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_0164.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))


def trig_tables():
    """The seven per-j constant tables, numpy-verbatim."""
    import numpy as np

    J = np.arange(1, 36)
    K = (J * 5 + 2) % 12 + 1
    return {
        "S1": np.sin(J * np.pi / K),
        "C2": np.cos(J * np.pi / (K + 1)),
        "C1": np.cos(J * np.pi / K),
        "S2": np.sin(J * np.pi / (K + 1)),
        "S10": np.abs(np.sin(J * np.pi / 10)),
        "C8": np.cos(J * np.pi / 8),
        "S9": np.sin(J * np.pi / 9),
    }


def _vlit(a) -> str:
    return ", ".join(repr(float(x)) for x in a)


def build_source_text() -> str:
    t = trig_tables()
    if any(len(v) != 35 for v in t.values()):
        raise RuntimeError("trig tables must have 35 entries")
    return "\n".join(
        [
            # R = t1*S1 + t2*C2
            f"poly = vector_literal({_vlit(t['C2'])})",
            "poly = multiply(poly, t2)",
            "poly",
            f"poly = vector_literal({_vlit(t['S1'])})",
            "poly = multiply(poly, t1)",
            "poly = add(poly, pop)",
            "poly",
            # I = t2*C1 - t1*S2;  RI = R + 1i*I
            f"poly = vector_literal({_vlit(t['S2'])})",
            "poly = multiply(poly, t1)",
            "poly",
            f"poly = vector_literal({_vlit(t['C1'])})",
            "poly = multiply(poly, t2)",
            "poly = subtract(poly, pop)",
            "poly = multiply(poly, 1i)",
            "poly = add(poly, pop)",
            "poly",
            # A = angle(t1')*C8 + angle(t2')*S9;  E = cos(A) + 1i*sin(A)
            f"poly = vector_literal({_vlit(t['S9'])})",
            "poly = multiply(poly, angle(t2 + 1i*t1))",
            "poly",
            f"poly = vector_literal({_vlit(t['C8'])})",
            "poly = multiply(poly, angle(t1 + 1i*t2))",
            "poly = add(poly, pop)",
            "poly",
            "poly = cos(poly)",
            "poly",
            "swap",
            "poly = sin(pop)",
            "poly = multiply(poly, 1i)",
            "poly = add(poly, pop)",
            "poly",
            # M = log(|t1'| + j) * S10
            "poly = arange(1, 36)",
            "poly = add(poly, abs(t1 + 1i*t2))",
            "poly = log(poly)",
            "poly",
            f"poly = vector_literal({_vlit(t['S10'])})",
            "poly = multiply(poly, pop)",
            # cf = (M*RI)*E, then the zfrm chain
            "swap",
            "poly = multiply(poly, pop)",
            "poly = multiply(poly, pop)",
            "poly = power_series(poly, 8)",
            "poly = invpower(poly, 3)",
            "poly = roots_cm(poly, hi, exact)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_0164",
        "chain": [],
        "source_text": build_source_text(),
    }


def _render(payload: dict) -> str:
    return json.dumps(payload, indent=2, ensure_ascii=True) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    rendered = _render(build_payload())
    if args.check:
        current = OUTPUT.read_text(encoding="utf-8") if OUTPUT.exists() else ""
        if current != rendered:
            raise SystemExit(
                "FATAL: giga_0164.coeff-program.json is stale; run "
                "scripts/gen_giga_0164_coeff_program.py"
            )
        print("giga_0164.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
