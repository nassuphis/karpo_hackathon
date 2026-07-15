#!/usr/bin/env python3
"""Generate the portable giga_2877 Coeff Program document.

giga_2877 is the first family member whose zfrm runs a ROOT-FINDER inside
the coefficient function (snapshot 8e28adb, which also extends poly_letter
with the ro/io glyph offset this run sets to -0.5-0.5i):

    roots_p:  q = andy * append(np.roots(cf), 1) + cf      (andy = 100)

Two structural facts drive the recreation (full story in
recreate_giga_2877.md):

1. The root VALUES need no solver: cf is np.poly(glyph + delta), so
   np.roots(cf) is exactly the shifted glyph points up to eigenvalue dust
   (~1e-13, invisible at 100x). The program carries the raw points as a
   second pool constant and shifts them with the same delta expression.
2. The root ORDER — which root pairs with which coefficient slot — is
   LAPACK zgeev's QR emission order: measured chaotic (239 distinct
   permutations over 400 rows, no monotone rule) and build-dependent, so
   it is irreproducible by construction. The program uses the measured
   CLOSEST expressible rule: descending |root| (sort_abs + reverse scan),
   whose aggregate cloud correlates 0.934 with the LAPACK ensemble
   (fixed pool order: 0.65; random per-row: 0.77; ascending: 0.47).
   Rows where distinct roots tie in magnitude EXACTLY (only the fully
   symmetric delta=0 row t=(0.5,0.5)) have tie-order freedom in both
   pipelines.

The appended constant 1 slot is handled inside the combine map with the
guarded index k-32*floor(k/32) (reads tos[0] harmlessly at k=32) and the
selector floor(k/32), keeping every index in bounds; the combine is
commutative-exact against the formula.

Parity is pinned by tests/test_giga_2877_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2877.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

# scale(t, 0.9) + offset with ro = io = -0.5, association preserved:
# (0.05 + 0.9t) computed first, then + (-0.5), as letters.square does.
DELTA_TEXT = "((0.05+0.9*t1)+(0-0.5)) + 1i*((0.05+0.9*t2)+(0-0.5))"


def letter_roots() -> list[complex]:
    """Glyph b178's lit pixels via the checked-in table, read exactly as
    letters.get_letter_coordinates did: bit (x, y) -> (x-3.5, (7-y)-3.5)."""
    from cp437_font import FONT_ROWS

    coords = []
    for y, row in enumerate(FONT_ROWS[178]):
        for x in range(8):
            if row & (1 << (7 - x)):
                coords.append(complex(x - 3.5, (7 - y) - 3.5))
    return coords


def _point_literal(z: complex) -> str:
    text = f"{z.real:g}"
    text += f"+{z.imag:g}i" if z.imag >= 0 else f"{z.imag:g}i"
    return text


def build_source_text() -> str:
    points = letter_roots()
    if len(points) != 32:
        raise RuntimeError("b178 must contain exactly 32 lit pixels")
    points_text = ", ".join(_point_literal(p) for p in points)
    return "\n".join(
        [
            # rts = glyph points + delta (values of np.roots(cf), exactly)
            f"poly = vector_literal({points_text})",
            f"poly = add(poly, {DELTA_TEXT})",
            # documented pairing: descending |root| (reverse of sort_abs)
            "poly = sort_abs(poly)",
            "poly",
            "poly = scan(32, 0, tos[31], tos[31-k])",
            "drop",
            "poly",
            # cf = coefficients of the same shifted constellation
            f"poly = translate_roots(roots_ascii_literal(178), {DELTA_TEXT})",
            # q[k] = cf[k] + 100*r[k] for k<32; q[32] = cf[32] + 100*1
            "poly = scan(poly_len, 0, poly[0] + 100*tos[0],"
            " poly[k] + 100*(tos[k-32*floor(k/32)]*(1-floor(k/32))"
            " + floor(k/32)))",
            "drop",
            "emit",
        ]
    )


def build_cf_probe_source_text() -> str:
    """Test-only variant that emits the cf stage instead of q: identical
    prefix, with the combine map removed (the trailing drop then discards
    the sorted-roots vector). Used to recover the VM's own cf so the
    tie-row structural test does not conflate translate-vs-np.poly dust
    with pairing differences."""
    lines = build_source_text().split("\n")
    combine = [ln for ln in lines if "floor(k/32)" in ln]
    if len(combine) != 1:
        raise RuntimeError("expected exactly one combine map line")
    lines.remove(combine[0])
    return "\n".join(lines)


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2877",
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
                "FATAL: giga_2877.coeff-program.json is stale; run "
                "scripts/gen_giga_2877_coeff_program.py"
            )
        print("giga_2877.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
