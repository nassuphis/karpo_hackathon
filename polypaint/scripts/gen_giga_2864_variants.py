#!/usr/bin/env python3
"""Generate the giga_2864 iteration-variant Coeff Program documents.

giga_2864's simplified core is a geometric orbit written as a scan:

    poly = scan(71, 0, c, prev*c)      with c = -(t1^2+t2^2)

These variants keep the shape — 71 coefficients produced by iterating a
1D map whose parameter is driven by the sweep — and swap the iteration.
Each was prototyped against the native VM (sweep_test compute_debug,
25-probe grid) before being committed; see docs/giga-2864-variants.md
for the per-variant probe stats and the doubling-map trap.

- giga_2864_logistic — x <- r*x*(1-x), r = 3+(t1^2+t2^2)/2 in [3,4].
  The sweep walks the bifurcation cascade: period-2/4/8 plateaus
  (near-sparse coefficients -> rosettes), chaos, and the period-3
  window at r~3.83 cutting a clean band through it.
- giga_2864_henon — x <- 1 - a*x^2 + 0.3*prev2 (two-init scan),
  a = 1+0.2*(t1^2+t2^2) in [1,1.4]. Strange-attractor coefficients:
  never periodic, both signs, root radius swings 0.3..4.7.
- giga_2864_sine — x <- r*sin(pi*x), r in [0.7,1]. The logistic's
  smooth conjugate; same cascade, differently curved seams.
- giga_2864_spiral — the original geometric orbit with a COMPLEX
  ratio c = -(t1^2+t2^2) - i*t1*t2: log-spiral coefficient decay,
  roots wind into a spiral and conjugate symmetry breaks. Inherits
  the base program's origin-corner degree collapse (|c|^71 underflow).
- giga_2864_bbaker — beta-baker (Renyi map) x <- frac(beta*x),
  beta = 1+0.49*(t1^2+t2^2) in [1,1.98]. The baker's stretch-and-cut
  with a non-dyadic stretch (the literal x2 baker collapses to exact
  zero in <=53 float steps — probe-proven dead on all rows). Smooth
  melt: roots-of-unity lattice at beta~1 dissolving into chaos.
- giga_2864_tent — the FOLDED baker x <- r*(0.5-|x-0.5|)*2,
  r/2 in [0.5,~1]: band-splitting regimes instead of periodic windows.
- giga_2864_phases — beta-baker on ANGLES, coefficients exp(2*pi*i*x)
  all on the unit circle: Erdos-Turan pins the roots to a tight ring
  (probe |z|max 1.15..1.76) with baker-chaotic phase filigree.

frac(x) is spelled x - floor(x): the source language has no frac.
Parity for every variant is pinned by tests/test_giga_2864_variants.py
against pure-python oracles (math module = the same libm the VM links).
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

R2 = "(t1*t1+t2*t2)"
PI_TEXT = "3.141592653589793"
TWO_PI_I = "6.283185307179586i"

BBAKER_SEED = "0.5+0.37*t1-floor(0.5+0.37*t1)"
BBAKER_STEP = f"(1+0.49*{R2})*prev-floor((1+0.49*{R2})*prev)"
PHASES_SEED = BBAKER_SEED
PHASES_STEP = (
    f"(1.3+0.3*{R2})*prev+0.211327"
    f"-floor((1.3+0.3*{R2})*prev+0.211327)"
)


def build_source_text_logistic() -> str:
    return "\n".join(
        [
            f"poly = scan(71, 0, 0.5, (3+{R2}/2)*prev*(1-prev))",
            "emit",
        ]
    )


def build_source_text_henon() -> str:
    return "\n".join(
        [
            f"poly = scan(71, 0, 0.1, 0.1, 1-(1+0.2*{R2})*prev*prev+0.3*prev2)",
            "emit",
        ]
    )


def build_source_text_sine() -> str:
    return "\n".join(
        [
            f"poly = scan(71, 0, 0.5, (0.7+0.15*{R2})*sin({PI_TEXT}*prev))",
            "emit",
        ]
    )


def build_source_text_spiral() -> str:
    ratio = f"0-{R2}-1i*t1*t2"
    return "\n".join(
        [
            f"poly = scan(71, 0, {ratio}, prev*({ratio}))",
            "emit",
        ]
    )


def build_source_text_bbaker() -> str:
    return "\n".join(
        [
            f"poly = scan(71, 0, {BBAKER_SEED}, {BBAKER_STEP})",
            "emit",
        ]
    )


def build_source_text_tent() -> str:
    return "\n".join(
        [
            f"poly = scan(71, 0, 0.37, (1+0.49*{R2})*(0.5-abs(prev-0.5))*2)",
            "emit",
        ]
    )


def build_source_text_phases() -> str:
    return "\n".join(
        [
            f"poly = scan(71, 0, {PHASES_SEED}, {PHASES_STEP})",
            f"poly = exp(multiply(poly, {TWO_PI_I}))",
            "emit",
        ]
    )


VARIANTS = {
    "giga_2864_logistic": build_source_text_logistic,
    "giga_2864_henon": build_source_text_henon,
    "giga_2864_sine": build_source_text_sine,
    "giga_2864_spiral": build_source_text_spiral,
    "giga_2864_bbaker": build_source_text_bbaker,
    "giga_2864_tent": build_source_text_tent,
    "giga_2864_phases": build_source_text_phases,
}


def build_payload(name: str) -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": name,
        "chain": [],
        "source_text": VARIANTS[name](),
    }


def _render(payload: dict) -> str:
    return json.dumps(payload, indent=2, ensure_ascii=True) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    for name in VARIANTS:
        output = ROOT / f"{name}.coeff-program.json"
        rendered = _render(build_payload(name))
        if args.check:
            current = output.read_text(encoding="utf-8") if output.exists() else ""
            if current != rendered:
                raise SystemExit(
                    f"FATAL: {output.name} is stale; run "
                    "scripts/gen_giga_2864_variants.py"
                )
            print(f"{output.name}: OK")
        else:
            output.write_text(rendered, encoding="utf-8")
            print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
