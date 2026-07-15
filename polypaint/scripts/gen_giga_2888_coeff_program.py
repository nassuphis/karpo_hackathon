#!/usr/bin/env python3
"""Generate the portable giga_2888 Coeff Program document.

giga_2888 (run 2025-01-31; poly_chess1 committed at 1230acc, unchanged
through fb854a4) is the circling chessboard:

    N=7 grid; t = uniform(-2pi, 2pi); u = uniform(-2pi, 2pi)
    curve  = grid + (sin t + 1j cos t) - (4+4j)     # 49 roots on a ride
    cf1    = np.poly(curve + 0.1*cos u)             # real jitter shift
    cf3    = cf1 + 0.0001*(curve ++ [10j])          # roots BLENDED into coeffs
    cf     = cf3 + 1e-23*(cf3^2 + cf3 + 1)          # elementwise quadratic,
                                                    # only alive where |cf3|>~3e11

Both uniforms IGNORE t1/t2 entirely, so they remap directly onto the
sweep parameters in numpy's own low + (high-low)*r form:
t = -2pi + 4pi*t1, u = -2pi + 4pi*t2 — exact in distribution and
arithmetic. The append rides the giga_2877 guarded-map idiom
(k - 49*floor(k/49) reads tos[0] harmlessly at k=49; floor(k/49)
selects the 10j), and the quadratic is four vector ops. Two pool
constants: the raw 49 grid points (for the blend) and their monic
expansion via roots_literal (for the translate).

Parity is pinned by tests/test_giga_2888_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2888.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

W2 = "12.566370614359172"    # high - low = 4*pi, as np.random.uniform computes
LO = "-6.283185307179586"    # low = -2*pi


def grid_points() -> list[complex]:
    """The 7x7 integer grid centered on the origin, in poly_chess1's
    construction order: x = tile(1..7) (column fast), y = repeat(1..7)
    (row slow), then - (N+1)/2 - 1j*(N+1)/2."""
    return [complex(c - 4, r - 4) for r in range(1, 8) for c in range(1, 8)]


def _lit(z: complex) -> str:
    text = f"{z.real:g}"
    text += f"+{z.imag:g}i" if z.imag >= 0 else f"{z.imag:g}i"
    return text


def build_source_text() -> str:
    pts = grid_points()
    if len(pts) != 49:
        raise RuntimeError("the chessboard must contain exactly 49 points")
    pts_text = ", ".join(_lit(p) for p in pts)
    t = f"({LO} + {W2}*t1)"
    u = f"({LO} + {W2}*t2)"
    d_curve = f"sin({t}) + 1i*cos({t})"
    d_roots = f"sin({t}) + 0.1*cos({u}) + 1i*cos({t})"
    return "\n".join(
        [
            # curve = grid + circle shift (the blend vector, pre-jitter)
            f"poly = vector_literal({pts_text})",
            f"poly = add(poly, {d_curve})",
            "poly",
            # cf1 = np.poly(curve + 0.1*cos u)
            f"poly = translate_roots(roots_literal({pts_text}), {d_roots})",
            # cf3 = cf1 + 0.0001*(curve ++ [10j])  — guarded-map append
            "poly = scan(50, 0, poly[0] + 0.0001*tos[0],"
            " poly[k] + 0.0001*(tos[k-49*floor(k/49)]*(1-floor(k/49))"
            " + 10i*floor(k/49)))",
            "drop",
            # cf = cf3 + 1e-23*(cf3^2 + cf3 + 1)
            "poly",
            "poly = multiply(poly, poly)",
            "poly = add(poly, peek)",
            "poly = add(poly, 1)",
            "poly = multiply(poly, 0.00000000000000000000001)",
            "poly = add(pop, poly)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2888",
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
                "FATAL: giga_2888.coeff-program.json is stale; run "
                "scripts/gen_giga_2888_coeff_program.py"
            )
        print("giga_2888.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
