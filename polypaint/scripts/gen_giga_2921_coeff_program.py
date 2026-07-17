#!/usr/bin/env python3
"""Generate the giga_2921 Coeff Program document.

giga_2921 (poly_giga_1 on unit-circle params, zfrm=recursive_addvr,
solve=none) paints a 240-particle physics run: per row, np.roots of the
formula EMA-pull the nearest of 10 stored root vectors while all 240
stored points repel under inverse-square forces and damped velocities;
the pulled bin's points are painted directly. The three concentric
rings in the artwork are the REPULSION EQUILIBRIUM's shells — their
radii depend on run length and the unseeded np.random initial
velocities (a faithful re-simulation at a different scale/RNG scores
2D corr ~0.0 against the artwork; see recreate_giga_2921.md) — pure
run fossils, unknowable from the mechanism.

The migration keeps what is mechanism (exact formula, exact unit-circle
params, root ANGLES from roots_cm in the reference's np.roots order)
and reconstructs the fossil radial structure from the artwork itself
(measured rings: mass/mean/sd = 9.3%/1.048/0.128, 35.7%/2.569/0.198,
54.0%/4.361/0.169), assigned per root by the frac-cascade PRNG (the
2883/2907 remap pattern) with an EMA-smear angular jitter. Measured:
radial corr 0.896, 2D log-blur corr 0.876 vs the reference small PNG
(the residual is the beading — individual particle residences).

Uses the register syntax (r1..r5) and expression-composable transforms.
Parity is pinned by tests/test_giga_2921_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

FRAC = lambda e: f"{e}-floor({e})"  # noqa: E731 — the language has no frac()
U1 = FRAC("t1*7919.7717+t2*104729.31+0.5")
V1 = FRAC("t1*3571.3331+t2*27644.437+0.25")
W1 = FRAC("t1*1299.7091+t2*15485.863+0.75")
STEP = FRAC("prev*9821.4959+0.211327")


def build_source_text() -> str:
    return "\n".join(
        [
            # exact xfrm 'uc': T = exp(2*pi*i*t)
            "r1 = exp(6.283185307179586i*t1)",
            "r2 = exp(6.283185307179586i*t2)",
            # exact poly_giga_1, numpy layout (slot 0 = leading)
            "poly = fill(25, 0)",
            "poly[0] = 30*r1*r1*r2",
            "poly[1] = 30*r1*r2*r2",
            "poly[2] = 40*r1**3",
            "poly[3] = 40*r2**3",
            "poly[4] = 0-25*r1*r1",
            "poly[5] = 0-25*r2*r2",
            "poly[6] = 10*r1*r2",
            "poly[9] = 100*r1**4*r2**4",
            "poly[11] = 0-5*r1",
            "poly[13] = 5*r2",
            "poly[24] = 0-10",
            # np.roots order (zgeev), 24 roots in slots 0..23
            "poly = roots_cm(poly, lo, exact)",
            "poly = poly[0:24]",
            # frac-cascade PRNG: ring choice / radial jitter / angular smear
            f"r3 = scan(24, 0, {U1}, {STEP})",
            f"r4 = scan(24, 0, {V1}, {STEP})",
            # measured ring fossils: masses .0934/.3567/.5399 (cumulative
            # .0934/.4501), centers 1.048/2.569/4.361, uniform widths
            # sd*sqrt(12) = .443/.686/.585
            "r5 = select(lt(r3, 0.0934), add(1.048, multiply(subtract(r4, 0.5), 0.443)),"
            " select(lt(r3, 0.4501), add(2.569, multiply(subtract(r4, 0.5), 0.686)),"
            " add(4.361, multiply(subtract(r4, 0.5), 0.585))))",
            "poly = multiply(divide(poly, abs(poly)), r5)",
            f"poly = multiply(poly, exp(multiply(subtract(scan(24, 0, {W1}, {STEP}), 0.5), 0.3i)))",
            # monic re-expansion; the pipeline's solver recovers the points
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
        "name": "giga_2921",
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    output = ROOT / "giga_2921.coeff-program.json"
    rendered = json.dumps(build_payload(), indent=2, ensure_ascii=True) + "\n"
    if args.check:
        current = output.read_text(encoding="utf-8") if output.exists() else ""
        if current != rendered:
            raise SystemExit("FATAL: giga_2921.coeff-program.json is stale; run scripts/gen_giga_2921_coeff_program.py")
        print(f"{output.name}: OK")
    else:
        output.write_text(rendered, encoding="utf-8")
        print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
