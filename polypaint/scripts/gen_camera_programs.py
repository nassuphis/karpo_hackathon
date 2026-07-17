#!/usr/bin/env python3
"""Generate the escape-camera Coeff Program documents.

Born from the giga-observation session (2026-07-17): a scan whose
coefficients are a quadratic-map orbit paints a little Mandelbrot set,
because (a) the smallest root of the emitted polynomial is an
almost-affine function of the low-order coefficients — a conformal
"camera" aimed at the parameter plane — and (b) rows whose orbit
escapes overflow the f32 coefficient transport and are skipped,
punching the escape set out of the camera's image. These two programs
make that mechanism deliberate:

- mandel_camera — c sweeps the Mandelbrot bounding box
  [-2.1, 0.5] x [-1.3, 1.3]; the scan runs the true critical orbit
  x <- x^2 + c for 120 steps (a deep escape-time level, so the
  silhouette hugs the real boundary). Camera pokes at the CONSTANT end
  of the vector (slot 0 is the leading coefficient in this pipeline):
  a0 = 3 + 0.5c, a1 = 20, so the smallest root is
  z1 ~ -(3 + 0.5c)/20 = -0.15 - 0.025c — an affine, undistorted copy
  of M, centered at -0.15, 0.065 wide. The large a1 is what keeps the
  copy affine: branch corrections scale like orbit/a1.
- julia_camera — the dual: c is FIXED at the Douady-rabbit component
  (-0.1226 + 0.7449i) and the SEED sweeps a radius-1.4 disk
  (t2 winds 101 times, 634.60...i = 2*pi*101*i, so the disk is covered
  densely). The escape cutout is the filled Julia set of the rabbit,
  painted through the same camera.

Knobs (documented in docs/camera-programs.md): the two poked constants
place and size the copy; the scan length sharpens the boundary (up to
~250); every other root branch paints a further, more distorted copy
for free — including the reciprocal-world mirror.

Parity is pinned by tests/test_camera_programs.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

C_EXPR = "(t1*2.6-2.1)+1i*(t2*2.6-1.3)"


def build_source_text_mandel() -> str:
    return "\n".join(
        [
            f"r1 = {C_EXPR}",
            "poly = scan(120, 0, r1, prev*prev+r1)",
            "poly",
            "poly = fill(122, 0)",
            "poly[0:120] = pop",
            "poly[120] = 20",
            f"poly[121] = 3+0.5*({C_EXPR})",
            "emit",
        ]
    )


def build_source_text_julia() -> str:
    return "\n".join(
        [
            "r1 = 1.4*t1*exp(634.6017010557239i*t2)",
            "poly = scan(120, 0, r1, prev*prev-0.1226+0.7449i)",
            "poly",
            "poly = fill(122, 0)",
            "poly[0:120] = pop",
            "poly[120] = 20",
            "poly[121] = 3+0.5*r1",
            "emit",
        ]
    )


PROGRAMS = {
    "mandel_camera": build_source_text_mandel,
    "julia_camera": build_source_text_julia,
}


def build_payload(name: str) -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": name,
        "chain": [],
        "source_text": PROGRAMS[name](),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    for name in PROGRAMS:
        output = ROOT / f"{name}.coeff-program.json"
        rendered = json.dumps(build_payload(name), indent=2, ensure_ascii=True) + "\n"
        if args.check:
            current = output.read_text(encoding="utf-8") if output.exists() else ""
            if current != rendered:
                raise SystemExit(f"FATAL: {output.name} is stale; run scripts/gen_camera_programs.py")
            print(f"{output.name}: OK")
        else:
            output.write_text(rendered, encoding="utf-8")
            print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
