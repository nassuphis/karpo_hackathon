#!/usr/bin/env python3
"""Generate the portable giga_2902 Coeff Program document."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from textwrap import dedent


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_2902.coeff-program.json"

RJAIL3 = """
TTTTTTTTTTT
TSSSSSSSSST
TSSS   SSST
TS   S   ST      T
TSSS   SSST
TSSSSSSSSST
TTTTTTTTTTT
"""


def _layout_roots(layout: str) -> tuple[list[complex], list[complex]]:
    lines = dedent(layout).strip("\n").expandtabs(4).splitlines()
    width = max(len(line) for line in lines)
    rows = len(lines)
    source: list[complex] = []
    target: list[complex] = []
    for row, line in enumerate(lines):
        padded = line.ljust(width)
        y = (rows - 1) / 2.0 - row
        for column, marker in enumerate(padded):
            x = column - (width - 1) / 2.0
            if marker == "S":
                source.append(complex(x, y))
            elif marker == "T":
                target.append(complex(x, y))
    return source, target


def _number(value: float) -> str:
    normalized = 0.0 if value == 0.0 else float(value)
    return format(normalized, ".17g")


def _complex(value: complex) -> str:
    # 'i' is the app's house-style imaginary suffix (pi2i, decompiled '1.0i',
    # every Help snippet) and the only one the scrub pad originally targeted;
    # the parser accepts [ijIJ] but generated source should scrub cleanly.
    real = 0.0 if value.real == 0.0 else value.real
    imag = 0.0 if value.imag == 0.0 else value.imag
    if imag == 0.0:
        return _number(real)
    if real == 0.0:
        return f"{_number(imag)}i"
    sign = "+" if imag >= 0.0 else ""
    return f"{_number(real)}{sign}{_number(imag)}i"


def _roots_source(values: list[complex]) -> str:
    rows = ["roots_literal("]
    rows.extend(f"    {_complex(value)}," for value in values[:-1])
    rows.append(f"    {_complex(values[-1])}")
    rows.append(")")
    return "\n".join(rows)


def build_source_text() -> str:
    # The program carries the rjail3 ROOT LAYOUT itself; the Coeff Program
    # compiler expands each monic product exactly once (Fraction arithmetic,
    # coeff_program_chain.expand_monic_roots) into the constant pool. The
    # compiled pool — and therefore the fingerprint — is bit-identical to the
    # earlier vector_literal spelling that shipped expanded coefficients.
    source_roots, target_roots = _layout_roots(RJAIL3)
    if len(source_roots) != 33 or len(target_roots) != 33:
        raise RuntimeError("rjail3 must contain exactly 33 source and 33 target roots")
    return "\n".join(
        [
            _roots_source(source_roots),
            _roots_source(target_roots),
            "poly = blend(bimodal(t2, 0.7))",
            "poly = translate_roots(poly, (1+i)*0.1*exp(pi2i*t1))",
            "poly",
            "poly = sort_abs(poly)",
            "poly = multiply(poly, 0.01)",
            "poly = add(pop, poly)",
            "emit",
        ]
    )


def build_payload() -> dict:
    # This is deliberately the same portable document accepted by the normal
    # Coeff Programs Upload UI and /save-coeff-program endpoint. It is not an
    # application preset, registry entry, or special execution mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_2902",
        "chain": [],
        "source_text": build_source_text(),
    }


def source_provenance() -> dict:
    normalized_layout = dedent(RJAIL3).strip("\n") + "\n"
    return {
        "source_layout": "rjail3",
        "source_layout_sha256": hashlib.sha256(
            normalized_layout.encode("utf-8")
        ).hexdigest(),
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
                "FATAL: giga_2902.coeff-program.json is stale; run "
                "scripts/gen_giga_2902_coeff_program.py"
            )
        print("giga_2902.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
