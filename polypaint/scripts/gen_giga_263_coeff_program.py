#!/usr/bin/env python3
"""Generate the portable giga_263 Coeff Program document.

The program is the section-14.1 source from recreate_giga_263.md: the
historical batman/coeff7 formula spelled directly in the current Coeff
Program language. There are no constants to precompute — arange(9, 0, -1)
builds the index vector pre-reversed (zfrm.rev), and everything else is
per-row scalar/vector arithmetic. Parity against the snapshot's own formula
is pinned by tests/test_giga_263_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_263.coeff-program.json"

SOURCE_TEXT = """u1 = exp(pi2i*t1)
u2 = exp(pi2i*t2)
v1 = (u1+sin(u1))/(u1+cos(u1))
v2 = (u2+sin(u2))/(u2+cos(u2))
poly = arange(9, 0, -1)
poly = sin(poly)
poly = multiply(poly, angle(v1))
poly
poly = arange(9, 0, -1)
poly = cos(poly)
poly = multiply(poly, angle(v2))
poly = add(pop, poly)
poly = multiply(poly, 1i)
poly = exp(poly)
poly
poly = arange(9, 0, -1)
poly = add(poly, abs(v1)+abs(v2))
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
        "name": "giga_263",
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
                "FATAL: giga_263.coeff-program.json is stale; run "
                "scripts/gen_giga_263_coeff_program.py"
            )
        print("giga_263.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
