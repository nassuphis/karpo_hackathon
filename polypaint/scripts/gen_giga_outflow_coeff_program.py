#!/usr/bin/env python3
"""Generate the portable giga_outflow Coeff Program document.

giga_outflow (run 2025-03-14; poly_483/coeff6 at snapshot 01627b4):

    unit_circle: z = e^{2j pi t}
    coeff6:      t' = (z^3 + 1j)/(z^3 - 1j)      per axis (Mobius of cube)
    poly_483:    35 slots, j = 1..35:
                 part1 = re(t1')^j          * sin(j*angle(t2'))
                 part2 = im(t2')^(35-j)     * cos(j*|t1'|)
                 part3 = log(|t1'|+|t2'|+j)
                 part4 = (re(t1')+j)*(im(t2')+j)*log(|t1'|+1)
                 cf[j-1] = (part1*part2 + part3*part4)
                           * exp(1j*(angle(t1')*sin j + angle(t2')*cos j
                                     + log(|t1'|+1)/j))
    zfrm rev, solve safe.

THE FOSSIL FINDING: coeff6 maps the unit circle onto the IMAGINARY
axis — mathematically re(t') == 0. The reference's part1 is therefore
built from the ROUNDING DUST of numpy's complex division (|re| ~ 1e-16,
sign erratic), amplified by im(t2')^(35-j) into visible structure. The
VM's division (scaled-naive c_div_full) produces dust of the same scale
and character but different values; measured aggregate clouds in the
saved view: corr 0.863 vs same-ensemble noise floor 0.712 at 15k rows —
ensemble-indistinguishable, the giga_2877/2883 argument once more.

Program shape: the fused-scan idiom — the [t1', t2'] pair parks in a
stack-side 2-vector and THREE scans (angle phases, the dust-free
part3*part4, the dust-amplified part1*part2) read it via tos, with a
swap dance keeping the park on top. Integer powers spell
exp(n*log(x)) (scalar ** is integer-literal-only); for negative real
bases that carries ~1e-16 imaginary dust, absorbed by the same
ensemble argument. 95 tokens, no pool constants.

Parity: the dust-free stages match an exact-semantics oracle at
8.9e-16; the full chain is dust-dominated on some rows by construction
(no external oracle can pin the park's own exp/log dust) — see
tests/test_giga_outflow_coeff_program.py for the two-tier design.

Known edge: coeff6 has poles at t in {1/12 + k/3} exactly (z^3 = i);
the VM errors such rows (scalar division by zero) while the reference
propagated inf into its except-guard. Grid N should avoid placing
t-samples exactly on twelfths.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "giga_outflow.coeff-program.json"

LAMBDA_DIR = ROOT / "lambda"
import sys

sys.path.insert(0, str(LAMBDA_DIR))

_E1 = "exp(pi2i*t1)"
_E2 = "exp(pi2i*t2)"
T1P_TEXT = f"(({_E1}*{_E1}*{_E1}) + 1i)/(({_E1}*{_E1}*{_E1}) - 1i)"
T2P_TEXT = f"(({_E2}*{_E2}*{_E2}) + 1i)/(({_E2}*{_E2}*{_E2}) - 1i)"


def _f12(kk: str) -> str:
    return (f"exp(({kk}+1)*log(real(tos[0])))*sin(({kk}+1)*angle(tos[1]))"
            f"*exp((34-{kk})*log(imag(tos[1])))*cos(({kk}+1)*abs(tos[0]))")


def _f34(kk: str) -> str:
    return (f"log(abs(tos[0]) + abs(tos[1]) + ({kk}+1))"
            f"*((real(tos[0])+({kk}+1))*(imag(tos[1])+({kk}+1))*log(abs(tos[0])+1))")


def _aa(kk: str) -> str:
    return (f"angle(tos[0])*sin({kk}+1) + angle(tos[1])*cos({kk}+1)"
            f" + log(abs(tos[0])+1)/({kk}+1)")


def build_source_text() -> str:
    return "\n".join(
        [
            "poly = fill(2, 0)",
            f"poly[0] = {T1P_TEXT}",
            f"poly[1] = {T2P_TEXT}",
            "poly",
            f"poly = scan(35, 0, {_aa('0')}, {_aa('k')})",
            "poly = multiply(poly, 1i)",
            "poly = exp(poly)",
            "poly",
            "swap",
            f"poly = scan(35, 0, {_f34('0')}, {_f34('k')})",
            "poly",
            "swap",
            f"poly = scan(35, 0, {_f12('0')}, {_f12('k')})",
            "drop",
            "poly = add(poly, pop)",
            "poly = multiply(poly, pop)",
            "poly = rev(poly)",
            "emit",
        ]
    )


def build_structural_probe_source_text() -> str:
    """Test-only: the dust-free stages alone (F34 * E) — everything except
    the dust-amplified part1*part2 term. Pins the park, the scans, and the
    stack discipline at 1e-9-class parity."""
    return "\n".join(
        [
            "poly = fill(2, 0)",
            f"poly[0] = {T1P_TEXT}",
            f"poly[1] = {T2P_TEXT}",
            "poly",
            f"poly = scan(35, 0, {_aa('0')}, {_aa('k')})",
            "poly = multiply(poly, 1i)",
            "poly = exp(poly)",
            "poly",
            "swap",
            f"poly = scan(35, 0, {_f34('0')}, {_f34('k')})",
            "swap",
            "poly = multiply(poly, pop)",
            "drop",
            "emit",
        ]
    )


def build_payload() -> dict:
    # The ordinary portable document accepted by the Coeff Programs Upload UI
    # and /save-coeff-program — no preset, registry entry, or special mode.
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": "giga_outflow",
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
                "FATAL: giga_outflow.coeff-program.json is stale; run "
                "scripts/gen_giga_outflow_coeff_program.py"
            )
        print("giga_outflow.coeff-program.json: OK")
        return 0
    OUTPUT.write_text(rendered, encoding="utf-8")
    print(f"Wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
