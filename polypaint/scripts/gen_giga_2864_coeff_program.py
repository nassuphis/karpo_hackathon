#!/usr/bin/env python3
"""Generate the portable giga_2864 Coeff Program document.

giga_2864 (run 2025-01-24; poly_creative10/zz at snapshot 500685f) is a
solver-noise artwork, and its formula collapses to almost nothing:

    zz:  t1' = t2' = t1 + 1j*t2      (BOTH arguments the same z)
    poly_creative10 ("geometric algebra product terms"):
        dot   = a.b   with a == b  ->  |z|^2
        wedge = a^b   with a == b  ->  x*y - y*x == +0.0 EXACTLY (IEEE)
        cf[k] = gp^(k+1), signs alternated
    =>  cf[k] = (-r)^(k+1),  r = t1^2 + t2^2

A geometric sequence. The polynomial is -r*(x^71 - (-r)^71)/(x + r):
its TRUE roots are a perfect circle of radius r with one point deleted.
Sweeping (t1, t2) draws each of the 70 fixed angles radially — a clean
starburst of 70 rays, nothing else. That is what Aberth-Ehrlich renders
(measured: true-roots cloud corr 0.735 vs the reference, BELOW the
0.866 same-ensemble floor — visibly different, i.e. "boring").

The artwork lives in the SOLVER: the coefficients span r^1..r^71, and
the companion-matrix eigensolve's structured error on that scaling
paints the ornate ringed orb at the center. The reference used
np.roots (f64 zgeev); polypaint's companion_matrix path (f32 transport
+ f64 zgeev + f32 roots) reproduces the ensemble at corr 0.9273 vs the
0.8665 floor — same starburst, same orb. RENDER WITH companion_matrix;
aberth_mt gives the rays alone.

The program is ONE scan — three tokens, the shortest of the series:
the geometric chain c_0 = -r, c_k = c_{k-1} * (-r). Coefficient parity
vs the verbatim (complex-pow) reference spelling: 2.7e-15 per-slot
relative, including the r^71 tinies.

Parity is pinned by tests/test_giga_2864_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2864.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))


def build_source_text() -> str:
    return "\n".join(
        [
            "poly = scan(71, 0, 0-(t1*t1+t2*t2), prev*(0-(t1*t1+t2*t2)))",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2864",
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
                "FATAL: giga_2864.coeff-program.json is stale; run "
                "scripts/gen_giga_2864_coeff_program.py"
            )
        print("giga_2864.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
