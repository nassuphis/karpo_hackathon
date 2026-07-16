#!/usr/bin/env python3
"""Generate the portable giga_2910 Coeff Program document.

giga_2910 (run 2025-03-03; kabala1/tohalf at snapshot 8264c9c) is the
kabbalistic walk: the word 'isminiadami' becomes a gematria vector
kv = [9,100,40,9,50,9,1,4,1,40,9], and each sample advances a GLOBAL
unit-modulus state:

    cf <- (cf + kv*t1 - 1j*flip(kv)*t2) / |cf + ...|   (elementwise)
    q  = tohalf(cf):  roots -> (r - e^{i pi/2})/(e^{-i pi/2} - r) -> np.poly

The image is the walk's OCCUPATION MEASURE — its stationary
distribution (the drive magnitudes kv*t are >> 1, so the chain mixes in
a few steps). Polypaint rows are independent, so the program samples
the stationary distribution per row with a K=23 BURN-IN: a frac-cascade
PRNG supplies 22 drive pairs, the row's own (t1, t2) is the FINAL step
(preserving the reference's per-sample correlation between drive and
emitted polynomial), and one 253-long scan runs all 11 independent
component chains back to back with floor-mask resets at component
boundaries. Measured: burn-in cloud vs the true sequential walk at the
sampling noise floor (corr 0.824 vs floor 0.817 at 12k rows).

tohalf runs the mid-chain root-finder (roots_cm strip=exact — the same
zgeev order as the reference's np.roots), a Mobius map in vector ops
(the e^{+-i pi/2} constants are numpy's exact doubles, dust included),
and the expand_roots primitive added for this wave: np.poly's
convolution bitwise, in the solver's emission order.

Parity is pinned by tests/test_giga_2910_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2910.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

WORD = "isminiadami"
KABALA_VALUES = {
    'A': 1, 'B': 2, 'C': 3, 'D': 4, 'E': 5, 'F': 6, 'G': 7, 'H': 8, 'I': 9,
    'J': 10, 'K': 20, 'L': 30, 'M': 40, 'N': 50, 'O': 60, 'P': 70, 'Q': 80,
    'R': 90, 'S': 100, 'T': 200, 'U': 300, 'V': 400, 'W': 500, 'X': 600,
    'Y': 700, 'Z': 800,
}
K_STEPS = 23

CASCADE_FIRST = (
    "((t1*7919.7717 + t2*104729.31 + 0.5) - floor(t1*7919.7717 + t2*104729.31 + 0.5))"
    " + 1i*((t1*104729.31 + t2*7919.7717 + 0.25) - floor(t1*104729.31 + t2*7919.7717 + 0.25))"
)
CASCADE_STEP = (
    "((real(prev)*9821.4959 + 0.211327) - floor(real(prev)*9821.4959 + 0.211327))"
    " + 1i*((imag(prev)*9821.4959 + 0.531327) - floor(imag(prev)*9821.4959 + 0.531327))"
)
CONCAT_STEP = (
    "tos[(k - 23*floor(k/23))*(1-floor(k/23))]*(1-floor(k/23))"
    " + poly[(k-23)*floor(k/23)]*floor(k/23)"
)
Z_FIRST = "tos[23]*real(tos[0]) - 1i*tos[33]*imag(tos[0])"
Z_STEP = (
    "tos[23+floor(k/23)]*real(tos[k - 23*floor(k/23)])"
    " - 1i*tos[33-floor(k/23)]*imag(tos[k - 23*floor(k/23)])"
)
CHAIN_FIRST = "(1 + tos[0])/abs(1 + tos[0])"
CHAIN_STEP = (
    "(prev + floor((23-(k - 23*floor(k/23)))/23)*(1-prev) + tos[k])"
    "/abs(prev + floor((23-(k - 23*floor(k/23)))/23)*(1-prev) + tos[k])"
)


def kabalistic_vector() -> list[int]:
    return [KABALA_VALUES.get(ch, 0) for ch in WORD.upper()]


def mobius_constants() -> tuple[complex, complex]:
    """theta = 0.25: e^{+i pi/2} and e^{-i pi/2} as numpy's exact doubles."""
    import numpy as np

    return (complex(np.exp(1j * 2 * np.pi * 0.25)),
            complex(np.exp(-1j * 2 * np.pi * 0.25)))


def _clit(z: complex) -> str:
    text = repr(float(z.real))
    text += f"+{repr(float(z.imag))}i" if z.imag >= 0 else f"{repr(float(z.imag))}i"
    return text


def build_source_text() -> str:
    kv = kabalistic_vector()
    if len(kv) != 11 or kv != [9, 100, 40, 9, 50, 9, 1, 4, 1, 40, 9]:
        raise RuntimeError("isminiadami must map to the pinned gematria vector")
    a1, a2 = mobius_constants()
    return "\n".join(
        [
            # PRNG drive pairs; the LAST step is the row's own uniforms
            f"poly = scan(23, 0, {CASCADE_FIRST}, {CASCADE_STEP})",
            "poly[22] = t1 + 1i*t2",
            "poly",
            # concat [T(23), kv(11)] so one stacked vector serves the chain
            f"poly = vector_literal({', '.join(str(x) for x in kv)})",
            f"poly = scan(34, 0, tos[0], {CONCAT_STEP})",
            "poly",
            "swap",
            "drop",
            # drives per (component, step): z = v_j*re(T_s) - 1i*w_j*im(T_s)
            f"poly = scan(253, 0, {Z_FIRST}, {Z_STEP})",
            "drop",
            "poly",
            # the walk: 11 component chains back to back, floor-mask resets
            f"poly = scan(253, 0, {CHAIN_FIRST}, {CHAIN_STEP})",
            "drop",
            # component finals -> the stationary state (11 unit-modulus coeffs)
            "poly = scan(11, 0, poly[22], poly[23*k+22])",
            # tohalf: roots -> Mobius -> expand
            "poly = roots_cm(poly, lo, exact)",
            "poly = scan(10, 0, poly[0], poly[k])",
            "poly",
            "poly = multiply(poly, 0-1)",
            f"poly = add(poly, {_clit(a2)})",
            "poly",
            "swap",
            f"poly = subtract(pop, {_clit(a1)})",
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
        "name": "giga_2910",
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
                "FATAL: giga_2910.coeff-program.json is stale; run "
                "scripts/gen_giga_2910_coeff_program.py"
            )
        print("giga_2910.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
