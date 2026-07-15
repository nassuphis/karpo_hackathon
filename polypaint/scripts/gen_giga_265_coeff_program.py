#!/usr/bin/env python3
"""Generate the portable giga_265 Coeff Program document.

The program is the section-8.1 source from recreate_giga_265.md: the
historical bkr -> uc -> epow -> batman chain spelled in the current Coeff
Program language. The baker's map reduces exactly for real uniforms
(x % 1 == x - floor(x); the imaginary fold is zero), locals w1/w2 avoid the
reserved p1/p2 registers, and arange(9, 0, -1) makes zfrm.rev free. Parity
against the recovered historical chain is pinned by
tests/test_giga_265_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_265.coeff-program.json"

SOURCE_TEXT = """x1 = t1 - floor(t1)
b1 = (2*x1 - floor(2*x1)) + 1i*floor(2*x1)/2
w1 = exp(exp(pi2i*b1))
x2 = t2 - floor(t2)
b2 = (2*x2 - floor(2*x2)) + 1i*floor(2*x2)/2
w2 = exp(exp(pi2i*b2))
poly = arange(9, 0, -1)
poly = sin(poly)
poly = multiply(poly, angle(w1))
poly
poly = arange(9, 0, -1)
poly = cos(poly)
poly = multiply(poly, angle(w2))
poly = add(pop, poly)
poly = multiply(poly, 1i)
poly = exp(poly)
poly
poly = arange(9, 0, -1)
poly = add(poly, abs(w1)+abs(w2))
poly = log(poly)
poly
poly = arange(9, 0, -1)
poly = multiply(poly, poly)
poly = multiply(pop, poly)
poly = multiply(pop, poly)
emit"""


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_265",
        "chain": [],
        "source_text": SOURCE_TEXT,
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
                "FATAL: giga_265.coeff-program.json is stale; run "
                "scripts/gen_giga_265_coeff_program.py"
            )
        print("giga_265.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
