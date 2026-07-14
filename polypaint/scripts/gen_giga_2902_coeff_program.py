#!/usr/bin/env python3
"""Generate the portable giga_2902 Coeff Program document."""
from __future__ import annotations

import argparse
from fractions import Fraction
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


def _coefficients_from_roots(roots: list[complex]) -> list[complex]:
    # This is a development-time expansion. Production loads the resulting
    # immutable coefficient vectors from the Coeff Program constant pool.
    coefficients = [(Fraction(1), Fraction(0))]
    for root in roots:
        root_re = Fraction(root.real)
        root_im = Fraction(root.imag)
        expanded = [
            (Fraction(0), Fraction(0))
            for _ in range(len(coefficients) + 1)
        ]
        for index, (coefficient_re, coefficient_im) in enumerate(coefficients):
            current_re, current_im = expanded[index]
            expanded[index] = (
                current_re + coefficient_re,
                current_im + coefficient_im,
            )
            product_re = coefficient_re * root_re - coefficient_im * root_im
            product_im = coefficient_re * root_im + coefficient_im * root_re
            next_re, next_im = expanded[index + 1]
            expanded[index + 1] = (
                next_re - product_re,
                next_im - product_im,
            )
        coefficients = expanded
    return [complex(float(real), float(imag)) for real, imag in coefficients]


def _number(value: float) -> str:
    normalized = 0.0 if value == 0.0 else float(value)
    return format(normalized, ".17g")


def _complex(value: complex) -> str:
    real = 0.0 if value.real == 0.0 else value.real
    imag = 0.0 if value.imag == 0.0 else value.imag
    if imag == 0.0:
        return _number(real)
    if real == 0.0:
        return f"{_number(imag)}j"
    sign = "+" if imag >= 0.0 else ""
    return f"{_number(real)}{sign}{_number(imag)}j"


def _vector_source(values: list[complex]) -> str:
    rows = ["vector_literal("]
    rows.extend(f"    {_complex(value)}," for value in values[:-1])
    rows.append(f"    {_complex(values[-1])}")
    rows.append(")")
    return "\n".join(rows)


def build_source_text() -> str:
    source_roots, target_roots = _layout_roots(RJAIL3)
    if len(source_roots) != 33 or len(target_roots) != 33:
        raise RuntimeError("rjail3 must contain exactly 33 source and 33 target roots")
    source_coefficients = _coefficients_from_roots(source_roots)
    target_coefficients = _coefficients_from_roots(target_roots)
    return "\n".join(
        [
            _vector_source(source_coefficients),
            _vector_source(target_coefficients),
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
