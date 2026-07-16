#!/usr/bin/env python3
"""Generate the portable giga_2911 Coeff Program document.

giga_2911 (run 2025-03-06; p11b2_v2/toline_q at snapshot a9393c5) is the
p11-family sawtooth pushed through a Cayley transform — and, uniquely in
the recreated set, it contains NO randomness at all: every stage is a
deterministic function of the sweep parameters. No remap argument, no
PRNG, no ensemble measurement — per-row exactness end to end.

    unit_circle:  t' = e^{2j pi t}                      (complex uniforms)
    p11b2_v2:     denom = t1'+t2'+3   (|denom| >= 1: the normalize branch
                                       is DEAD — |t1'+t2'| <= 2)
                  u  = 77 * linspace(0,1,11)^15 / denom
                  m  = int(4583*|t1'+t2'|) % 11 + 1     (integer regime 1..11)
                  cf = ((arange(11)+1) % m + 1) * exp(1j pi u)
    zfrm:         cumsum -> rev -> toline_q (andy=1.0):
                  roots -> 1j*(1+r)/(1-r) -> np.poly    (Cayley, disk->line)

The Cayley stage uses the mid-chain root-finder (roots_cm strip=exact),
vector ops for the Mobius algebra, and the expand_roots primitive
(np.poly bitwise, solver emission order). andy=1.0 keeps only the
transformed polynomial, matching toline_q's blend exactly.

Parity is pinned by tests/test_giga_2911_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2911.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

M_TEXT = ("floor(4583*abs(poly[0])) - 11*floor(floor(4583*abs(poly[0]))/11) + 1")


def power_vector() -> list[float]:
    """77 * linspace(0,1,11)^15 — numpy's exact doubles (linspace interior
    points carry step dust; np.power the same)."""
    import numpy as np

    return [float(x) for x in 7 * 11 * np.power(np.linspace(0, 1, 11), 15)]


def build_source_text() -> str:
    p_text = ", ".join(repr(x) for x in power_vector())
    return "\n".join(
        [
            # s = e^{2i pi t1} + e^{2i pi t2}; m = int(4583|s|) % 11 + 1
            "poly = fill(2, 0)",
            "poly[0] = exp(pi2i*t1) + exp(pi2i*t2)",
            f"poly[1] = {M_TEXT}",
            # sf+1 = (k+1) mod m + 1  (push; scratch stays readable)
            "scan(11, 0, (0+1) - poly[1]*floor((0+1)/poly[1]) + 1,"
            " (k+1) - poly[1]*floor((k+1)/poly[1]) + 1)",
            # uc = exp(1i pi * P/(s+3)); cf = (sf+1)*uc
            f"vector_literal({p_text})",
            "poly = divide(pop, poly[0] + 3)",
            "poly = multiply(poly, 3.141592653589793i)",
            "poly = exp(poly)",
            "poly = multiply(pop, poly)",
            # zfrm: cumsum, rev
            "poly = scan(poly_len, 0, poly[0], prev + poly[k])",
            "poly = rev(poly)",
            # toline_q (andy=1): roots -> 1i*(1+r)/(1-r) -> expand
            "poly = roots_cm(poly, lo, exact)",
            "poly = scan(10, 0, poly[0], poly[k])",
            "poly",
            "poly = multiply(poly, 0-1)",
            "poly = add(poly, 1)",
            "poly",
            "swap",
            "poly = add(pop, 1)",
            "poly = multiply(poly, 1i)",
            "poly = divide(poly, pop)",
            "poly = expand_roots(poly)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2911",
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
                "FATAL: giga_2911.coeff-program.json is stale; run "
                "scripts/gen_giga_2911_coeff_program.py"
            )
        print("giga_2911.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
