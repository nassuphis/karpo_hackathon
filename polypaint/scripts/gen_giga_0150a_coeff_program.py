#!/usr/bin/env python3
"""Generate the giga_0150a Coeff Program.

giga_0150a (png 2025; dict-format state files) is the RAW-parameter band
polynomial `p7f`: xfrm none (t1, t2 are the raw sweep uniforms), zfrm none,
solve 'safe', degree 22 (23 slots; the config's n=6 is unused by p7f).

Per row (verbatim reference):

    tt1  = exp(2*pi*i*t1)                       # unit-circle point
    ttt1 = exp(2*pi*i*tt1)                      # COMPLEX exponent
    v    = linspace(Re(tt1), Re(ttt1), 23)      # REAL grid, span up to e^2pi
    b    = the t2 DECILE selects one of 10 bands:
           amp  = [10*t1, 100, 599, 443, 293, 541, 379, 233, 173, 257][b]
           mult = [  11,   17,  83, 179, 127, 103, 283,   3,   5,  23][b]
           trig = cos for band 2, sin otherwise
    f    = amp * exp(i * trig(mult * 2*pi * v))
    f[22] += 211 * exp(2*pi*i * t2/7)

The port is branchless: b = floor(10*t2) reproduces the decile comparisons
exactly (bands are [0.1*m, 0.1*(m+1)) with <= / < endpoints), and the
band lookup is a one-hot FLOOR-gate vector dotted against a constant
amp/mult table via the scan pipeline:

    COMB literal [amps(0..9) | mults(10..19) | 0,0,0]
    SELV  = COMB (.) one-hot gate  (gate = floor(1/(1+|floor(10*t2)-idx|)))
    CUM   = running sum            -> CUM[9] = amp_b, CUM[19]-CUM[9] = mult_b
    BRIDGE= [X0, Xstep, ampfactor, iscos]   (X = mult*2*pi*v pre-folded)
    F     = ampfactor * exp(i*((1-iscos)*sin(X_k) + iscos*cos(X_k)))
    + the slot-22 addition via a gated scan

(each stage a scan against the previous stage parked on the stack — scan
init/step expressions ride the side table; the 2912-2918 trap log applies:
`drop` discards, tos[] only inside scan expressions).

Analog-vs-reference deltas (documented, all ~1e-10 or below):
- np.linspace forces its endpoint (y[22] = Re(ttt1) exactly); the scan
  computes start + step*22 (ulp-level, amplified by mult*2*pi*|v| to
  ~2e-10 relative on slot 22 of the highest band).
- X is accumulated as (M*2pi*rt) + (M*2pi*step)*k instead of the
  reference's (M*2pi)*(rt + step*k): association dust ~1e-16*|X|.
- Band boundaries: floor(10*t2) vs the reference's decile comparisons can
  disagree only when 10*t2 rounds across an integer (ulp-wide slivers of
  the sweep, measure-zero in the reference's random draws).

'safe' never fires: sum|f| is bounded by 23*599 + 211 and below by ~211
(the slot-22 addition keeps tiny band-0 rows above the knife); measured
over 20000 draws in the test.

Parity: tests/test_giga_0150a_coeff_program.py.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[1]

NAME = "giga_0150a"
WI = "6.283185307179586i"
TWO_PI = 6.283185307179586

AMPS = [10.0, 100.0, 599.0, 443.0, 293.0, 541.0, 379.0, 233.0, 173.0, 257.0]
MULTS = [11.0, 17.0, 83.0, 179.0, 127.0, 103.0, 283.0, 3.0, 5.0, 23.0]
COMB = AMPS + MULTS + [0.0, 0.0, 0.0]

# one-hot band gate for table index (k - 10*floor(k/10)) vs floor(10*t2),
# with the band-0 amplitude's t1 factor folded in at slot 0 only
GATE = ("floor(1/(1+abs(floor(10*t2)-(k-10*floor(k/10)))))"
        "*(1+floor(1/(1+k))*(t1-1))")

RT = f"real(exp({WI}*t1))"
RTTT = f"real(exp({WI}*exp({WI}*t1)))"
M2P = "(tos[19]-tos[9])*6.283185307179586"

# BRIDGE stage A (reads CUM): [X0, Xstep, amp, junk...]. Quadratic one-hot
# gates keep the 64-token scalar-expr cap; slots >= 3 are don't-care junk
# (the F scan reads tos[0..3] of stage B only).
BRIDGE_A_INIT = f"{M2P}*{RT}"
BRIDGE_A_STEP = (f"(2-k)*k*(({M2P}*{RTTT}-prev)/22)"
                 f"+(k-1)*tos[9]")
# BRIDGE stage B (reads stage A): [X0, Xstep, amp, iscos, junk...].
# iscos = [band==2] spelled floor(amp/599) — 599 is the unique max amp.
# quadratic one-hots over k in {1,2,3} (each vanishes at the other two;
# k>=4 slots are don't-care junk)
BRIDGE_B_STEP = ("((k-2)*(k-3)/2)*tos[1]"
                 "+((k-1)*(3-k))*tos[2]"
                 "+((k-1)*(k-2)/2)*floor(tos[2]/599)")
FE = ("tos[2]*exp(1i*((1-tos[3])*sin(tos[0]+tos[1]*k)"
      "+tos[3]*cos(tos[0]+tos[1]*k)))")
SLOT22 = ("(floor(k/22))*211"
          "*exp(6.283185307179586i*(1/7)*t2)")


def _lit(values) -> str:
    parts = []
    for x in values:
        r = repr(float(x))
        parts.append(r)
    return ", ".join(parts)


def build_source_text() -> str:
    lines = [
        f"poly = vector_literal({_lit(COMB)})",
        "poly",
        f"poly = scan(23, 0, tos[0]*({GATE}), tos[k]*({GATE}))",
        "drop",
        "poly",
        "poly = scan(23, 0, tos[0], prev + tos[k])",
        "drop",
        "poly",
        f"poly = scan(23, 0, {BRIDGE_A_INIT}, {BRIDGE_A_STEP})",
        "drop",
        "poly",
        f"poly = scan(23, 0, tos[0], {BRIDGE_B_STEP})",
        "drop",
        "poly",
        f"poly = scan(23, 0, {FE}, {FE})",
        "drop",
        "poly",
        f"poly = scan(23, 0, 0, {SLOT22})",
        "poly = add(poly, pop)",
        "emit",
    ]
    return "\n".join(lines) + "\n"


def oracle_row(u: float, v: float) -> np.ndarray:
    """Exact mirror of the emitted program (t1=u, t2=v raw)."""
    t1, t2 = float(u), float(v)
    b = int(np.floor(10 * t2))
    amp = AMPS[b]
    mult = MULTS[b]
    m2p = mult * TWO_PI
    rt = float(np.real(np.exp(2j * np.pi * t1)))
    rttt = float(np.real(np.exp(2j * np.pi * np.exp(2j * np.pi * t1))))
    x0 = m2p * rt
    xstep = (m2p * rttt - x0) / 22.0
    ampfactor = amp * (1.0 + (1.0 if b == 0 else 0.0) * (t1 - 1.0))
    iscos = 1.0 if b == 2 else 0.0
    k = np.arange(23, dtype=float)
    X = x0 + xstep * k
    f = ampfactor * np.exp(1j * ((1 - iscos) * np.sin(X) + iscos * np.cos(X)))
    f = f.astype(complex)
    f[22] += 211 * np.exp(2j * np.pi * (1.0 / 7.0) * t2)
    return f


def reference_row(u: float, v: float) -> np.ndarray:
    """The verbatim p7f (numpy reference) for the documented-delta check."""
    t1, t2 = float(u), float(v)
    pi2 = 2 * np.pi
    n = 23
    tt1 = np.exp(1j * 2 * np.pi * t1)
    ttt1 = np.exp(1j * 2 * np.pi * tt1)
    v_ = np.linspace(np.real(tt1), np.real(ttt1), n)
    if t2 < 0.1:
        f = 10 * t1 * np.exp(1j * np.sin(11 * pi2 * v_))
    elif t2 < 0.2:
        f = 100 * np.exp(1j * np.sin(17 * pi2 * v_))
    elif t2 < 0.3:
        f = 599 * np.exp(1j * np.cos(83 * pi2 * v_))
    elif t2 < 0.4:
        f = 443 * np.exp(1j * np.sin(179 * pi2 * v_))
    elif t2 < 0.5:
        f = 293 * np.exp(1j * np.sin(127 * pi2 * v_))
    elif t2 < 0.6:
        f = 541 * np.exp(1j * np.sin(103 * pi2 * v_))
    elif t2 < 0.7:
        f = 379 * np.exp(1j * np.sin(283 * pi2 * v_))
    elif t2 < 0.8:
        f = 233 * np.exp(1j * np.sin(3 * pi2 * v_))
    elif t2 < 0.9:
        f = 173 * np.exp(1j * np.sin(5 * pi2 * v_))
    else:
        f = 257 * np.exp(1j * np.sin(23 * pi2 * v_))
    f = f.astype(complex)
    f[n - 1] += 211 * np.exp(1j * pi2 * (1 / 7) * t2)
    return f


def build_document() -> dict:
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": NAME,
        "chain": [],
        "source_text": build_source_text(),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    path = ROOT / f"{NAME}.coeff-program.json"
    serialized = json.dumps(build_document(), indent=2) + "\n"
    if args.check:
        if not path.exists() or path.read_text() != serialized:
            print(f"STALE {path}")
            return 1
        print(f"OK {path}")
        return 0
    path.write_text(serialized)
    print(f"wrote {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
