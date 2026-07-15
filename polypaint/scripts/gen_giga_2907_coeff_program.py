#!/usr/bin/env python3
"""Generate the portable giga_2907 Coeff Program document.

giga_2907 (run 2025-02-15; poly_chess4 at snapshot 8d554a2, the last
commit before the run) is the curve-blend chessboard — structurally
giga_2902's bimodal blend with the two constellations riding DIFFERENT
curves:

    t = np.random.rand()                                # ignores t1/t2
    to = 0.5 * exp(2j*pi*t)                             # outer: circle
    ti = 0.2 * ipolygon(t + 0.125)                      # inner: square path
    cfi = np.poly(chess32 + ti*(1+1j))                  # (X+ti) + 1j(Y+ti)
    cfo = np.poly(chess32 + to*(1+1j))
    a  = bimodal_skewed(1e-5)                           # ~uniform blend weight
    cf = cfo*a + cfi*(1-a)
    cf = sort_abs(cf)*0.01 + cf                         # zfrm sort_abs_p
    q  = cf[::-1]                                       # zfrm rev

Both randoms ignore the sweep parameters (the dict's bkr,bkr xfrm feeds
arguments poly_chess4 never reads — vestigial, like i/tt/speed/off), so
they remap directly: t := t1, bimodal's u := t2. The VM's bimodal
primitive (added for giga_2902) is bit-for-bit bimodal_skewed, and
blend = below*(1-t) + top*t matches cfo*a + cfi*(1-a) with CFI pushed
first. The square path's vertex selection is the new trick: the four
vertices (numpy's exact exp(2j*pi*k/4) doubles, dust included) sit in a
scratch-extended pool vector, the edge index/frac/next-index ride pokes,
and the delta expression gathers with DYNAMIC DOUBLE INDEXING
poly[poly[4]] — piecewise-linear interpolation in one expression.

Parity is pinned by tests/test_giga_2907_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2907.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

EDGE_TEXT = "floor(4*(t1+0.125)) - 4*floor(floor(4*(t1+0.125))/4)"
FRAC_TEXT = "4*(t1+0.125) - floor(4*(t1+0.125))"
EDGE1_TEXT = "(floor(4*(t1+0.125))+1) - 4*floor((floor(4*(t1+0.125))+1)/4)"
DELTA_I_TEXT = "0.2*((1-poly[5])*poly[poly[4]] + poly[5]*poly[poly[6]])*(1+1i)"
DELTA_O_TEXT = "(0.5*exp(pi2i*t1))*(1+1i)"


def chess_points() -> list[complex]:
    """poly_chess4's masked grid: indices arange(8)-3.5, parity
    (i+j) % 2 != 0, X[mask] + 1j*Y[mask] in meshgrid order."""
    import numpy as np

    idx = np.arange(8) - 3.5
    par = (np.indices((8, 8)).sum(axis=0)) % 2 != 0
    X, Y = np.meshgrid(idx, idx)
    return [complex(z.real, z.imag) for z in (X[par] + 1j * Y[par]).flatten()]


def square_vertices() -> list[complex]:
    """ipolygon's vertices for n=4, radius 1, offset 0 — numpy's exact
    exp(2j*pi*k/4) doubles, dust included (cos(pi/2) != 0)."""
    import numpy as np

    return [complex(v) for v in np.exp(1j * 2 * np.pi * np.arange(4) / 4)]


def _lit(z: complex, *, precise: bool = False) -> str:
    fmt = (lambda v: repr(float(v))) if precise else (lambda v: f"{v:g}")
    text = fmt(z.real)
    text += f"+{fmt(z.imag)}i" if z.imag >= 0 else f"{fmt(z.imag)}i"
    return text


def build_source_text() -> str:
    pts = chess_points()
    if len(pts) != 32:
        raise RuntimeError("the chessboard mask must contain exactly 32 points")
    pts_text = ", ".join(_lit(p) for p in pts)
    verts_text = ", ".join(_lit(v, precise=True) for v in square_vertices())
    return "\n".join(
        [
            # square-path vertex pool + edge/frac/next-edge scratch
            f"poly = vector_literal({verts_text}, 0, 0, 0)",
            f"poly[4] = {EDGE_TEXT}",
            f"poly[5] = {FRAC_TEXT}",
            f"poly[6] = {EDGE1_TEXT}",
            # cfi: chessboard + 0.2*ipolygon(t1+0.125)*(1+1i)
            f"poly = translate_roots(roots_literal({pts_text}), {DELTA_I_TEXT})",
            "poly",
            # cfo: chessboard + 0.5*circle(t1)*(1+1i)   (same pool, deduped)
            f"poly = translate_roots(roots_literal({pts_text}), {DELTA_O_TEXT})",
            "poly",
            # combine: cfo*a + cfi*(1-a), a = bimodal_skewed(1e-5) at u=t2
            "poly = blend(bimodal(t2, 0.00001))",
            # zfrm sort_abs_p (andy = 0.01), then rev
            "poly",
            "poly = sort_abs(poly)",
            "poly = multiply(poly, 0.01)",
            "poly = add(pop, poly)",
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
        "name": "giga_2907",
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
                "FATAL: giga_2907.coeff-program.json is stale; run "
                "scripts/gen_giga_2907_coeff_program.py"
            )
        print("giga_2907.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
