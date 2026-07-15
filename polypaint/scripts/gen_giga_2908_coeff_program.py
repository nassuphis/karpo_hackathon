#!/usr/bin/env python3
"""Generate the portable giga_2908 Coeff Program document.

giga_2908 (run 2025-02-16; poly_chess4 at snapshot 2e061ed, which
teaches it the tt selector) is the spindle chessboard with a rotated
solver — and a chain whose composition COLLAPSES:

    xfrm bkr,t1_plus_t2:  t1' = t2' = s := bkr(t1) + bkr(t2)   (complex)
    tt = 't1pmt2':        tt1 = t1'+t2' = 2s,  tt2 = t1'-t2' = 0 EXACTLY
    to = 5.0 * spindle(2s)            # outer rides a superellipse
    ti = 0.2 * spindle(0.1)           # inner is a CONSTANT (tt2 = 0)
    cfi/cfo = np.poly(chess32 + shift*(1+1j))
    a  = bimodal_skewed(0.001)        # true RNG -> scan-PRNG remap
    cf = cfo*a + cfi*(1-a)
    cf = sort_abs(cf)*0.01 + cf; cf = cf[::-1]
    solve_rot45: roots * e^(-i pi/4)  # baked as coeff phase ramp

Notes measured/verified (full story in recreate_giga_2908.md):
- spindle's t is COMPLEX on 3/4 of rows (im(s) != 0): complex cosines
  grow like cosh, pushing the pre-rev constellation to ~1e5-1e7 — which
  the rev inversion (roots -> 1/roots) then maps to a SUB-PIXEL cluster
  at the origin (~1e-6..1e-9 vs pixel ~2e-5). np.sign's complex
  semantics changed in numpy 2.0 (sign(re) -> z/|z|); the program uses
  z/|z| (spelled C/abs(C)), and the choice only moves that origin dot
  at invisible scales. On visible rows the argument is real and both
  definitions coincide (+-1 exactly).
- The 4/3 power spells exp((2/1.5)*log|C|) — scalar ** is integer-only.
  Sub-ulp vs np's pow; measured parity 1.3e-11 including it.
- solve_rot45 rotates the SOLVED roots; algebraically q(z) = p(z*w^-1)
  has exactly the rotated roots, so the program multiplies cf[k] by
  exp(i pi/4 (32-k)) in a final map scan. Root-level equivalence is
  pinned in the test suite.
- bimodal's uniform is the only t-independent input; the giga_2883
  frac-cascade (identical constants) supplies it. Aggregate clouds:
  corr 0.69 vs same-ensemble floor 0.58 at 8k rows — indistinguishable.

Parity is pinned by tests/test_giga_2908_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2908.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

PHI4 = "0.7853981633974483"   # pi/4
S_TEXT = ("(2*(t1-floor(t1)) - floor(2*(t1-floor(t1)))"
          " + 2*(t2-floor(t2)) - floor(2*(t2-floor(t2))))"
          " + 0.5i*(floor(2*(t1-floor(t1))) + floor(2*(t2-floor(t2))))")
_U_INNER = ("((t1*7919.7717 + t2*104729.31 + 0.5)"
            " - floor(t1*7919.7717 + t2*104729.31 + 0.5))"
            "*9821.4959 + 0.211327")
U_TEXT = f"({_U_INNER}) - floor({_U_INNER})"


def chess_points() -> list[complex]:
    """poly_chess4's masked grid (n=8, mod=2, off=0): indices
    arange(8)-3.5, parity (i+j) % 2 != 0, meshgrid order."""
    import numpy as np

    idx = np.arange(8) - 3.5
    par = (np.indices((8, 8)).sum(axis=0)) % 2 != 0
    X, Y = np.meshgrid(idx, idx)
    return [complex(z.real, z.imag) for z in (X[par] + 1j * Y[par]).flatten()]


def inner_delta() -> complex:
    """0.2 * spindle(0.1) * (1+1j) — the constant inner shift, computed
    with the verbatim numpy formula (sign/abs/pow on real arguments)."""
    import numpy as np

    t = 0.0 * 1.0 + 0.1
    theta = 2 * np.pi * t
    x = 0.5 * np.sign(np.cos(theta)) * np.abs(np.cos(theta)) ** (2 / 1.5)
    y = 0.2 * np.sign(np.sin(theta)) * np.abs(np.sin(theta)) ** (2 / 1.5)
    return complex(0.2 * (x + 1j * y) * (1 + 1j))


def _lit(z: complex, *, precise: bool = True) -> str:
    fmt = (lambda v: repr(float(v))) if precise else (lambda v: f"{v:g}")
    text = fmt(z.real)
    text += f"+{fmt(z.imag)}i" if z.imag >= 0 else f"{fmt(z.imag)}i"
    return text


def build_source_text() -> str:
    pts = chess_points()
    if len(pts) != 32:
        raise RuntimeError("the chessboard mask must contain exactly 32 points")
    pts_text = ", ".join(_lit(p, precise=False) for p in pts)
    return "\n".join(
        [
            # scratch: s = bkr(t1)+bkr(t2), then the spindle coordinates
            "poly = fill(3, 0)",
            f"poly[0] = {S_TEXT}",
            "poly[1] = cos(6.283185307179586*(2*poly[0]))",
            "poly[2] = sin(6.283185307179586*(2*poly[0]))",
            "poly[1] = 0.5*(poly[1]/abs(poly[1]))"
            "*exp(1.3333333333333333*log(abs(poly[1])))",
            "poly[2] = 0.2*(poly[2]/abs(poly[2]))"
            "*exp(1.3333333333333333*log(abs(poly[2])))",
            # cfo: chessboard + 5*spindle(2s)*(1+1i)
            f"poly = translate_roots(roots_literal({pts_text}),"
            " (5*(poly[1] + 1i*poly[2]))*(1+1i))",
            "poly",
            # cfi: constant inner shift (tt2 = 0)
            f"poly = translate_roots(roots_literal({pts_text}), {_lit(inner_delta())})",
            "poly",
            "swap",
            # combine: cfo*a + cfi*(1-a), a = bimodal_skewed(0.001)
            f"poly = blend(bimodal({U_TEXT}, 0.001))",
            # zfrm sort_abs_p (andy=0.01), rev, then solve_rot45's ramp
            "poly",
            "poly = sort_abs(poly)",
            "poly = multiply(poly, 0.01)",
            "poly = add(pop, poly)",
            "poly = rev(poly)",
            f"poly = scan(poly_len, 0, poly[0]*exp(1i*{PHI4}*32),"
            f" poly[k]*exp(1i*{PHI4}*(32-k)))",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2908",
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
                "FATAL: giga_2908.coeff-program.json is stale; run "
                "scripts/gen_giga_2908_coeff_program.py"
            )
        print("giga_2908.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
