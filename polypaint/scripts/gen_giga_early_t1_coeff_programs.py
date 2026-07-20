#!/usr/bin/env python3
"""Generate the giga_1..giga_30 STATIC-FILL Coeff Programs (T1 tier).

The earliest reference artworks. Two eras with OPPOSITE coefficient
conventions (established by comparing giga_6 vs giga_29 — identical
generator text, different images):

- R era (Dec 2024 pngs; giga_1..26): ppol() vectors fed to R's
  base::polyroot, which reads ASCENDING (cf[1] = constant). Scripts
  that end `rev(cf)` (giga_1, giga_2) therefore already hand polyroot
  the reversed vector — their cf layout IS numpy-descending ("desc").
  Scripts returning cf directly are ascending ("asc"): the app builds
  cf[1..n] into poly[0..n-1] then `poly = rev(poly)`.
- python era (Jan 2025 pngs; giga_27..30): polys/giga.py defs solved
  with np.roots — numpy-DESCENDING direct ("np", 0-based keys).

Params for every one: param_space(len, x_01, x_circle) COMPOSES the
constructors over one sequence (workflow reduce), so BOTH t1 and t2
are unit-circle points e^(2 pi i t) — the legacy 'uc'. Monomials
t1^a t2^b therefore fold to exp(2 pi i (a t1 + b t2)).

giga_25's png is byte-identical to giga_22's (sha-verified) and its
def matches giga_22 exactly — kept as its own document for the id.
giga_21..23's saved `param_space(4000,x_01,,x_circle)` extra-comma
typo cannot run in R; the runs used x_01,x_circle like every sibling.

Parity: tests/test_giga_early_t1_coeff_programs.py (oracle = faithful
replication of each R/python def, descending output).
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

W = "6.283185307179586i"


def Z(a: int, b: int) -> str:
    """Monomial t1^a * t2^b on uc params: exp(2 pi i (a t1 + b t2))."""
    if a == 0 and b == 0:
        return "1"
    if a == 0:
        arg = "t2" if b == 1 else f"{b}*t2"
    elif b == 0:
        arg = "t1" if a == 1 else f"{a}*t1"
    else:
        left = "t1" if a == 1 else f"{a}*t1"
        right = "t2" if b == 1 else f"{b}*t2"
        arg = f"({left}+{right})"
    return f"exp({W}*{arg})"


Z1 = Z(1, 0)
Z2 = Z(0, 1)

# cf keys: R 1-based indices for desc/asc layouts, 0-based for np.
GIGAS = {
    "giga_1": {"n": 25, "layout": "desc", "cf": {
        1: f"30*{Z(2, 1)}",
        2: f"30*{Z(1, 2)}",
        3: f"40*{Z(3, 0)}",
        4: f"40*{Z(0, 3)}",
        5: f"-25*{Z(2, 0)}",
        6: f"-25*{Z(0, 2)}",
        7: f"10*{Z(1, 1)}",
        10: f"100*{Z(4, 4)}",
        12: f"-5*{Z1}",
        14: f"5*{Z2}",
        25: "-10",
    }},
    "giga_2": {"n": 25, "layout": "desc", "cf": {
        1: "-5",
        2: f"-10*{Z(1, 1)}",
        3: f"20*({Z1}-{Z2})",
        6: f"50*({Z(3, 0)}+{Z2})",
        8: f"-80*({Z(4, 0)}-{Z(0, 2)})",
        10: f"200*({Z(2, 0)}+{Z(0, 2)})",
        16: f"150*{Z(3, 5)}",
        20: f"-30*({Z(5, 0)}-{Z(0, 5)})",
        25: f"10*{Z(1, 3)}",
    }},
    "giga_3": {"n": 25, "layout": "asc", "cf": {
        1: "1",
        15: f"exp({Z1}-{Z2})",
        17: f"exp(1i*{Z1})",
        18: f"exp({Z1})",
        19: f"exp(-{Z1})",
        20: f"exp(-1i*{Z1})",
        24: f"exp(1i*{Z2})",
        25: "1+1i",
    }},
    "giga_4": {"n": 25, "layout": "asc", "cf": {
        1: "100",
        13: f"100*({Z(3, 0)}+{Z(2, 0)}+{Z1}-1)",
        15: f"100*({Z(3, 0)}-{Z(2, 0)}+{Z1}-1)",
        17: f"100*({Z(3, 0)}+{Z(2, 0)}-{Z1}-1)",
        21: "-10",
        25: f"exp(0.2i*{Z2})",
    }},
    "giga_5": {"n": 26, "layout": "asc", "cf": {
        1: "1", 5: "4", 13: "4", 20: "-9", 21: "-1.9", 25: "0.2",
        7: f"100i*({Z(0, 3)}+{Z(0, 2)}-{Z2}-1)",
        9: f"100i*({Z(3, 0)}+{Z(2, 0)}+{Z2}-1)",
        15: f"100i*({Z(0, 3)}-{Z(0, 2)}+{Z2}-1)",
    }},
    "giga_6": {"n": 10, "layout": "asc", "cf": {
        1: f"150*{Z(0, 3)}-150i*{Z(2, 0)}",
        5: f"100*({Z1}-{Z2})",
        10: "10i",
    }},
    "giga_12": {"n": 25, "layout": "asc", "cf": {
        1: "2",
        3: f"50*{Z(3, 0)}",
        5: f"50*{Z(0, 3)}",
        7: f"-30*{Z(2, 0)}",
        9: f"-30*{Z(0, 2)}",
        11: f"100*{Z(1, 1)}",
        13: f"50*{Z(2, 1)}",
        15: f"50*{Z(1, 2)}",
        20: f"-75*{Z(3, 3)}",
        21: f"3.5*{Z2}",
        25: f"-2*{Z1}",
    }},
    "giga_13": {"n": 25, "layout": "asc", "cf": {
        2: f"100*{Z(4, 0)}",
        4: f"100*{Z(0, 4)}",
        6: f"80*{Z(3, 1)}",
        8: f"80*{Z(1, 3)}",
        10: Z1,
        12: f"-{Z2}",
        14: f"5*{Z(2, 2)}",
        18: f"-0.5*{Z(5, 0)}",
        19: f"-0.5*{Z(0, 5)}",
        23: f"2.3*({Z(2, 0)}-{Z(0, 2)})",
        25: f"10*({Z(3, 0)}-{Z(0, 3)})",
    }},
    "giga_14": {"n": 25, "layout": "asc", "cf": {
        1: f"200*{Z(3, 2)}",
        5: f"200*{Z(2, 3)}",
        7: f"50*{Z(4, 0)}",
        9: f"50*{Z(0, 4)}",
        11: f"-100*{Z(3, 0)}",
        13: f"-100*{Z(0, 3)}",
        15: f"10*({Z(2, 0)}-{Z(0, 2)})",
        17: f"20*({Z1}-{Z2})",
        19: f"0.1*{Z(5, 0)}",
        21: f"0.1*{Z(0, 5)}",
        23: f"0.05*{Z(1, 1)}",
        25: "-10",
    }},
    "giga_15": {"n": 18, "layout": "asc", "cf": {
        2: f"200*({Z(5, 0)}+{Z(0, 5)})",
        4: f"100*({Z(4, 0)}-{Z(0, 4)})",
        6: f"80*{Z(6, 0)}",
        8: f"80*{Z(0, 6)}",
        10: f"2*{Z1}",
        12: f"-2*{Z2}",
        14: f"5*{Z(3, 3)}",
        18: "5",
    }},
    "giga_16": {"n": 22, "layout": "asc", "cf": {
        1: "-10",
        2: f"5*{Z2}",
        3: f"15*({Z1}-0.5*{Z2})",
        4: f"-20*{Z(1, 1)}",
        5: f"40*{Z(0, 3)}",
        7: f"-60*{Z(3, 0)}",
        9: f"80*({Z(2, 1)}-{Z(0, 2)})",
        13: f"250*({Z(5, 0)}-{Z(0, 3)})",
        18: f"200*{Z(4, 4)}",
        22: f"-30*({Z(6, 0)}+{Z2})",
    }},
    "giga_22": {"n": 26, "layout": "asc", "cf": {
        1: "100",
        13: f"100*({Z(3, 0)}+{Z(2, 0)}+{Z1}-1)",
        15: f"100*({Z(0, 3)}-{Z(0, 2)}+{Z2}-1)",
        17: f"100*({Z(0, 3)}+{Z(0, 2)}-{Z2}-1)",
        21: "-10",
        25: "0.2i",
    }},
    "giga_23": {"n": 26, "layout": "asc", "cf": {
        1: "1", 5: "4", 13: "4", 20: "-9", 21: "-1.9", 25: "0.2",
        7: f"100*({Z(0, 3)}+{Z(0, 2)}-{Z2}-1)",
        9: f"100*({Z(3, 0)}+{Z(2, 0)}+{Z2}-1)",
        15: f"100*({Z(0, 3)}-{Z(0, 2)}+{Z2}-1)",
    }},
    "giga_24": {"n": 26, "layout": "asc", "cf": {
        1: "1", 5: "4", 13: "4", 20: "-9", 21: "-1.9", 25: "0.2",
        7: f"100i*{Z(0, 3)}+100i*{Z(0, 2)}-100*{Z2}-100",
        9: f"100i*{Z(3, 0)}+100i*{Z(2, 0)}+100*{Z2}-100",
        15: f"100i*{Z(0, 3)}-100i*{Z(0, 2)}+100*{Z2}-100",
    }},
    "giga_25": {"n": 26, "layout": "asc", "cf": {
        1: "100",
        13: f"100*({Z(3, 0)}+{Z(2, 0)}+{Z1}-1)",
        15: f"100*({Z(0, 3)}-{Z(0, 2)}+{Z2}-1)",
        17: f"100*({Z(0, 3)}+{Z(0, 2)}-{Z2}-1)",
        21: "-10",
        25: "0.2i",
    }},
    "giga_26": {"n": 26, "layout": "asc", "cf": {
        1: "100",
        13: f"100*({Z(3, 0)}+{Z(2, 0)}+{Z1}-1)",
        15: f"100*({Z(0, 3)}-{Z(0, 2)}+{Z2}-1)",
        17: f"100*({Z(0, 3)}+{Z(0, 2)}-{Z2}-1)",
        21: "-10",
        25: "0.2",
    }},
    "giga_28": {"n": 6, "layout": "np", "cf": {
        0: f"100*{Z(0, 3)}+100i*{Z(3, 0)}",
        2: "150",
        5: "40i",
    }},
    "giga_29": {"n": 10, "layout": "np", "cf": {
        0: f"150*{Z(0, 3)}-150i*{Z(2, 0)}",
        4: f"100*({Z1}-{Z2})",
        9: "10i",
    }},
    "giga_30": {"n": 10, "layout": "np", "cf": {
        0: f"150i*{Z(0, 2)}+100*{Z(3, 0)}",
        4: f"150*abs({Z1}+{Z2}-2.5*(1i+1))",
        9: f"100i*{Z(3, 0)}+150*{Z(0, 2)}",
    }},
}


def build_source_text(name: str) -> str:
    spec = GIGAS[name]
    n, layout, cf = spec["n"], spec["layout"], spec["cf"]
    lines = [f"poly = fill({n}, 0)"]
    for key in sorted(cf):
        slot = key if layout == "np" else key - 1
        lines.append(f"poly[{slot}] = {cf[key]}")
    if layout == "asc":
        lines.append("poly = rev(poly)")
    lines.append("emit")
    return "\n".join(lines) + "\n"


def build_payload(name: str) -> dict:
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": name,
        "chain": [],
        "source_text": build_source_text(name),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    stale = []
    for name in GIGAS:
        output = ROOT / f"{name}.coeff-program.json"
        rendered = json.dumps(build_payload(name), indent=2, ensure_ascii=True) + "\n"
        if args.check:
            current = output.read_text(encoding="utf-8") if output.exists() else ""
            if current != rendered:
                stale.append(output.name)
            else:
                print(f"{output.name}: OK")
        else:
            output.write_text(rendered, encoding="utf-8")
            print(f"Wrote {output}")
    if stale:
        raise SystemExit(
            f"FATAL: stale documents {stale}; run scripts/gen_giga_early_t1_coeff_programs.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
