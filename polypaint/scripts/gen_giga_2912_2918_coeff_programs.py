#!/usr/bin/env python3
"""Generate giga_2912..giga_2918 Coeff Programs.

Seven python-era (pngs 2025-03-06/07) artworks whose zfrm chains all end in
a recursive_add* — the reference's SEQUENTIAL cross-row EMA:

    out_i = cf_i*(1-decay) + state*decay;  state = out_i
    (recursive_add_q then mixes: out_q = out*andy + cf*(1-andy))

The reference driver (pyroots.py) draws t=(U(0,1),U(0,1)) per row with an
np.random stream seeded from time+pid, so the chain runs over IID draws and
resets only at process start (~7M rows/process). The app's per-row programs
are stateless, so each chain is ported as the giga_2920 stateless analog:
the EMA expanded over INDEPENDENT pseudo-draws built from frac cascades of
the sweep's own (t1, t2), validated by 2-D root-cloud correlation against
the true sequential chain (numpy), with the chain's half-vs-half sampling
floor as the ceiling:

    giga  pipeline                                   analog       corr / floor
    2912  unit_circle p11b2_v2 cumsum,rev,r_add_q    J=1 ghost    0.957 / 0.933
    2913  unit_circle p11b2_v2 cumsum,rev,r_add_q    J=4 ladder   0.953 / 0.917
    2914  unit_circle poly_giga_12 uc,r_add          3 draws      0.919 / 0.877
    2915  unit_circle poly_giga_88 r_add             4 draws      0.944 / 0.941
    2916  unit_circle p11b2_v2 rev,roots,r_add       E + 2 draws  0.927 / 0.916
    2917  uc,sum_prod poly_727 rev,r_add             deterministic (5e-5 -> E)
    2918  uc three_balls r_add                       9 draws      0.725 / 0.618

Key physics (measured, scratchpad g2912_explore/g2913_ghost/g2916_design3):

- p11b2_v2 EXPLODES: u = 77*v^15/(t1+t2+3) is complex, so exp(i*pi*u) spans
  ~40 decades (P(max|cf|>1e6)=0.30, log-flat tail to ~1e42.7). The EMA drags
  a decaying ghost of each big row across later rows; with heavy-tailed
  scales the deep ladder terms are what matter, and the f32 transport knife
  (rows with |cf|>3.4e38, ~0.9%) removes the extreme ghosts in-app.
- 2916's root-space state at decay=0.9999 is PERMANENTLY heavy (~1e30): the
  root-magnitude tail is a ~log-flat power law with infinite mean, so the
  chain equilibrium is dominated by its biggest recent injections. The
  E-analog is realization-INVARIANT: two independent MC realizations of E
  give analog-vs-analog cloud corr 0.9995 (the cloud only sees the
  giant-slot-dominant SHAPE), so one realization is baked below.
- 2917's chain term is DETERMINISTIC: decay=5e-5 with a ~stable state means
  out = (1-5e-5)*cf + 5e-5*E exactly (state fluctuation contributes 2.5e-7
  relative — dropped). E baked from a 2e6-draw MC (seed 2917).
- 'solve' = np.roots (the app solver); 'safe' (2917/2918) never fires:
  sum|cf| measured in [587, 654] and [1.08, 78.5] vs the (1e-10, 1e10) window.

Analog-vs-reference simplifications (all measured, all documented):
- |exp(2*pi*i*u)| is taken as exactly 1 where the reference computes
  np.abs of the rounded exponential (delta ~5e-15 in the worst k*log slot).
- 2914 divides by the constant 100 (the provably-constant max|cf|); the
  reference divides by np.max(np.abs(cf)) = 100*(1 +/- 2e-16).
- Exact-boundary sweep rows (u or u+v exactly 0.5/1.5) can flip Arg sign;
  everywhere the angle enters as exp(i*angle) the result is identical.

Parity: tests/test_giga_2912_2918_coeff_programs.py (VM vs the analog
oracles below; the 2916 root trip runs on the Accelerate/LAPACK binary).
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[1]

WI = "6.283185307179586i"                 # 2*pi*i
PII = "3.141592653589793i"                # pi*i
TWO_PI = 6.283185307179586

# frac-cascade seeds (the 2920 pattern): each pseudo-draw pair uses two
# rows of this table — u' = frac(t1*m1+t2*m2+c), v' = frac(t1*m3+t2*m4+c').
SEEDS = [
    (7919.7717, 104729.31, 0.5), (3571.3331, 27644.437, 0.25),
    (1299.7091, 15485.863, 0.75), (6997.9337, 86028.121, 0.125),
    (5227.5511, 49979.687, 0.375), (2417.9977, 67867.967, 0.625),
    (9973.3181, 15991.879, 0.875), (4409.4157, 78901.117, 0.0625),
    (8887.5903, 24571.093, 0.5625), (6053.7411, 93077.671, 0.3125),
    (3163.9199, 56003.303, 0.8125), (7237.1129, 38747.539, 0.1875),
    (5441.8317, 61403.981, 0.6875), (2699.6003, 90679.349, 0.4375),
    (9199.4351, 33613.667, 0.9375), (4051.7093, 71993.227, 0.03125),
    (8623.9127, 47279.899, 0.53125), (5879.3141, 82763.531, 0.28125),
]


def _frac(expr: str) -> str:
    return f"(({expr})-floor({expr}))"


def _draw_pair(idx: int) -> tuple[str, str]:
    """The idx-th pseudo-draw (idx>=1; idx 0 is the row's own t1,t2)."""
    if idx == 0:
        return "t1", "t2"
    m1, m2, c1 = SEEDS[2 * (idx - 1)]
    m3, m4, c2 = SEEDS[2 * (idx - 1) + 1]
    return (_frac(f"t1*{m1!r}+t2*{m2!r}+{c1!r}"),
            _frac(f"t1*{m3!r}+t2*{m4!r}+{c2!r}"))


def _draw_pair_np(idx: int, u: float, v: float) -> tuple[float, float]:
    if idx == 0:
        return u, v
    m1, m2, c1 = SEEDS[2 * (idx - 1)]
    m3, m4, c2 = SEEDS[2 * (idx - 1) + 1]
    a = u * m1 + v * m2 + c1
    b = u * m3 + v * m4 + c2
    return a - math.floor(a), b - math.floor(b)


def _lit(values) -> str:
    """Spell a vector_literal argument list (complex-capable)."""
    parts = []
    for z in values:
        z = complex(z)
        if z.imag == 0:
            parts.append(_num(z.real))
        elif z.real == 0:
            parts.append(f"{_num_pos(z.imag)}i" if z.imag >= 0
                         else f"(0-{_num_pos(-z.imag)}i)")
        else:
            re = _num(z.real)
            if z.imag >= 0:
                parts.append(f"({re}+{_num_pos(z.imag)}i)")
            else:
                parts.append(f"({re}-{_num_pos(-z.imag)}i)")
    return ", ".join(parts)


def _num(x: float) -> str:
    return f"(0-{_num_pos(-x)})" if x < 0 else _num_pos(x)


def _num_pos(x: float) -> str:
    r = repr(float(x))
    if "e" in r:                      # grammar accepts sci literals (2898)
        return r
    return r


# ---------------------------------------------------------------------------
# p11b2_v2 (2912/2913/2916): cf_i = ((i+1)%m + 1) * exp(i*pi*77*(i/10)^15/den)
# with m = int(4583*|t1+t2|) % 11 + 1 and den = t1+t2+3 (|den|>=1 always, so
# the reference's normalize branch is dead; equality holds only at the exact
# u=v=0.5 grid point where the strict < still does not fire).
#
# Built as ONE pushless scan against a parked [S, m] pair (the 64-token
# scalar-expr cap rejects inlining the cascade-draw S into the mod formula).
# (i/10)^15 is spelled exp(15*log(i/10)) — the grammar has no pow — whose
# i=0 slot is exactly 0 via clog(0) = -inf, cexp(-inf) = 0 (probed bitwise
# against numpy).
# ---------------------------------------------------------------------------
P11_F = ("((k+1)-tos[1]*floor((k+1)/tos[1])+1)"
         "*exp(3.141592653589793i*77*exp(15*log(k/10))/(tos[0]+3))")


# Parked [S, m, 0...] pair built by a GATED scan (pokes compile chip-per-op
# and would blow the 256-chip cap at 5 evals; scan exprs ride the side
# table). Element 0 = S; element 1 reads prev (= S) through a floor gate
# that is 1 only at k==1; elements 2..10 are 0. Length 11 so the
# pop-discard add length-matches the main scan.
P11_M = ("(floor(2/(k+1)))*(floor(4583*abs(prev))"
         "-11*floor(floor(4583*abs(prev))/11)+1)")


def _p11b2_lines(U: str, V: str) -> list[str]:
    park_init = f"exp({WI}*({U}))+exp({WI}*({V}))"
    return [
        f"poly = scan(11, 0, {park_init}, {P11_M})",
        "poly",
        f"poly = scan(11, 0, {P11_F}, {P11_F})",
        "drop",                                     # discard the parked pair
    ]


def _p11b2_np(u: float, v: float) -> np.ndarray:
    S = np.exp(2j * np.pi * u) + np.exp(2j * np.pi * v)
    a = np.floor(4583 * np.abs(S))
    m = a - 11 * np.floor(a / 11) + 1
    k = np.arange(11, dtype=float)
    x15 = np.zeros(11)
    x15[1:] = np.exp(15 * np.log(k[1:] / 10))
    sf1 = (k + 1) - m * np.floor((k + 1) / m) + 1
    return sf1 * np.exp(1j * np.pi * 77 * x15 / (S + 3))


_CUMSUM_REV = [
    "poly",
    "poly = scan(11, 0, tos[0], prev + tos[k])",
    "drop",                                         # discard the parked cf
    "poly = rev(poly)",
]


def _cf2912_np(u: float, v: float) -> np.ndarray:
    return np.flip(np.cumsum(_p11b2_np(u, v)))


# ---------------------------------------------------------------------------
# weights (andy folded exactly)
# ---------------------------------------------------------------------------
def _weights_2912() -> list[float]:
    d, a = 1e-11, 0.9999
    return [1.0 - a * d, a * d]


def _weights_2913() -> list[float]:
    d, a = 1e-3, 0.999999
    w = [a * (1.0 - d) + (1.0 - a)]
    w += [a * (1.0 - d) * d**j for j in range(1, 4)]
    w.append(a * d**4)
    return w


W_2914 = [0.9, 0.09, 0.01]
W_2915 = [0.4 * 0.6**j for j in range(3)] + [0.6**3]   # J=3 + tail (chip cap)
W_2918 = [0.3 * 0.7**j for j in range(8)] + [0.7**8]
W_2916_B = 1e-4 * math.sqrt(1.0 / (1.0 - 0.9999**2))
W_2916_A = 1e-4
W_2917_CF = 1.0 - 5e-5

# E realizations (see module docstring). 2916: seed 2916, N=200000 over
# np.roots(rev(p11b2_v2)) — realization-arbitrary but cloud-equivalent.
E_2916 = [
    complex(1.5097303684058847e+28, 1.1049292265346052e+29),
    complex(3.118206446952097, -1750.6897525301708),
    complex(1.4826863137814748, 0.5507115999510209),
    complex(0.07296098135931596, 0.3665552071617456),
    complex(0.059533155743630745, 0.15543320794633564),
    complex(-0.1568887887329331, 0.010558268987581835),
    complex(-0.2558380392466701, -0.09153242074429933),
    complex(-0.14017706059257426, -0.12149157355774129),
    complex(-0.3427423549328569, -0.18422993652925065),
    complex(-0.2071282825143897, -0.1789822715899017),
]
# 2917: seed 2917, N=2000000 over rev(poly_727(z1+z2, z1*z2)) — convergent.
E_2917 = [
    complex(-23.096642580534997, -0.1554088276022002),
    complex(1.2277656482081907, 0.04408973333244868),
    complex(25.537192390043334, 0.0017821163758154355),
    complex(-7.9868854143301595, 0.060846840437819814),
    complex(1.1338467349184878, -0.007701823041257422),
    complex(6.086742663635923, -0.0006290539965778862),
    complex(-0.942627189183289, -0.011244520736460942),
    complex(0.16431286826155556, -0.0002085725864323182),
    complex(0.18573309009200084, 3.720104396554805e-05),
]

# ---------------------------------------------------------------------------
# 2914: poly_giga_12 -> uc.  Nonzero slots (index, coef, a, b) of c*t1^a*t2^b;
# uc(cf) = exp(2*pi*i*cf/100) with max|cf| == 100 (the |100*t1*t2| slot).
# ---------------------------------------------------------------------------
G12_SLOTS = [(0, 2.0, 0, 0), (2, 50.0, 3, 0), (4, 50.0, 0, 3),
             (6, -30.0, 2, 0), (8, -30.0, 0, 2), (10, 100.0, 1, 1),
             (12, 50.0, 2, 1), (14, 50.0, 1, 2), (19, -75.0, 3, 3),
             (20, 3.5, 0, 1), (24, -2.0, 1, 0)]
G12_AA = [0.0] * 25
G12_BB = [0.0] * 25
G12_CA = [0.0] * 25
for idx, c, a, b in G12_SLOTS:
    G12_AA[idx], G12_BB[idx], G12_CA[idx] = float(a), float(b), float(c)
UC_SCALE = repr(2 * math.pi / 100) + "i"


def _uc12_lines(U: str, V: str) -> list[str]:
    # push-literal-then-scan (the 2920 idiom): scan init/step expressions
    # ride the side table, so the cascade draws cost no chips.
    return [
        f"poly = vector_literal({_lit(G12_AA)})",
        "poly",
        f"poly = scan(25, 0, tos[0]*({WI}*({U})), tos[k]*({WI}*({U})))",
        "drop",
        "poly",
        f"poly = vector_literal({_lit(G12_BB)})",
        "poly",
        f"poly = scan(25, 0, tos[0]*({WI}*({V})), tos[k]*({WI}*({V})))",
        "drop",
        "poly = add(poly, pop)",
        "poly = exp(poly)",                          # M = t1^a * t2^b phases
        "poly",
        f"poly = vector_literal({_lit(G12_CA)})",
        "poly = multiply(poly, pop)",                # c * M
        f"poly = multiply(poly, {UC_SCALE})",
        "poly = exp(poly)",                          # exp(2*pi*i*cf/100)
    ]


def _uc12_np(u: float, v: float) -> np.ndarray:
    M = np.exp(2j * np.pi * (np.array(G12_AA) * u + np.array(G12_BB) * v))
    return np.exp((2j * np.pi / 100) * np.array(G12_CA) * M)


# ---------------------------------------------------------------------------
# 2915: poly_giga_88.  Slots 0..19 = (t1^p + t2^(p+1))/|t1-t2| (primes p);
# slots 20..50 constants (|t|==1 -> log(2)); slot 50 overwritten with
# exp(-i*Arg(t1)*Arg(t2)).
# ---------------------------------------------------------------------------
G88_P = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61,
         67, 71]
G88_PP = [float(p) for p in G88_P] + [0.0] * 31
G88_PP1 = [float(p + 1) for p in G88_P] + [0.0] * 31
G88_MK = [1.0] * 20 + [0.0] * 31
G88_CB = [0.0] * 20 + [
    (math.cos(k * math.log(2)) + math.sin(k * math.log(2))) / k
    for k in range(20, 51)]
G88_CB[50] = 0.0            # the reference overwrites slot 50; added below


def _g88_lines(U: str, V: str) -> list[str]:
    Z1 = f"exp({WI}*({U}))"
    Z2 = f"exp({WI}*({V}))"
    DIVA = f"abs({Z1}-{Z2})"
    S50 = f"exp((0-1i)*angle({Z1})*angle({Z2}))"
    return [
        # park 1/|z1-z2| at slot 0 (the fused per-k expressions overflow the
        # 256-char arg cap; a parked scalar keeps every later arg tiny)
        f"poly = scan(51, 0, 1/({DIVA}), prev)",   # 1/|z1-z2| in EVERY slot
        "poly",
        f"poly = vector_literal({_lit(G88_PP)})",
        "poly",
        f"poly = scan(51, 0, tos[0]*({WI}*({U})), tos[k]*({WI}*({U})))",
        "drop",
        "poly = exp(poly)",
        "poly",
        f"poly = vector_literal({_lit(G88_PP1)})",
        "poly",
        f"poly = scan(51, 0, tos[0]*({WI}*({V})), tos[k]*({WI}*({V})))",
        "drop",
        "poly = exp(poly)",
        "poly = add(poly, pop)",              # body = t1^p + t2^(p+1)
        # NB: tos[j] inside a vector-op ARGUMENT aliases poly itself (only
        # scan init/step see the pushed stack top), so the division consumes
        # the parked constant vector elementwise instead.
        "poly = multiply(poly, pop)",          # / |z1-z2|
        "poly",
        f"poly = vector_literal({_lit(G88_MK)})",
        "poly = multiply(poly, pop)",          # keep slots 0..19
        "poly",
        f"poly = vector_literal({_lit(G88_CB)})",
        "poly = add(poly, pop)",               # k*log(2) constants
        "poly",
        f"poly = scan(51, 0, 0, (floor(k/50))*({S50}))",
        "poly = add(poly, pop)",               # slot 50 = exp(-i*Arg*Arg)
    ]


def _g88_np(u: float, v: float) -> np.ndarray:
    z1, z2 = np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)
    pp = np.array(G88_PP)
    pp1 = np.array(G88_PP1)
    body = (np.exp(2j * np.pi * pp * u) + np.exp(2j * np.pi * pp1 * v)) \
        / np.abs(z1 - z2)
    cf = body * np.array(G88_MK) + np.array(G88_CB)
    cf[50] = np.exp(-1j * np.angle(z1) * np.angle(z2))
    return cf


# ---------------------------------------------------------------------------
# 2917: poly_727 (n=9) on (v1, v2) = (z1+z2, z1*z2); rev; deterministic mix.
# ---------------------------------------------------------------------------
F_2917 = ("log(tos[2]+1+k)*k*k*"
          "exp(1i*(tos[0]*sin(k)+tos[1]*cos(k)))")


def _cf2917_np(u: float, v: float) -> np.ndarray:
    z1, z2 = np.exp(2j * np.pi * u), np.exp(2j * np.pi * v)
    a1 = np.angle(z1 + z2)
    a2 = np.angle(np.exp(2j * np.pi * (u + v)))
    m1 = np.abs(z1 + z2)
    k = np.arange(1, 10)
    cf = np.log(m1 + 1 + k) * k * k * np.exp(1j * (a1 * np.sin(k)
                                                   + a2 * np.cos(k)))
    return np.flip(cf)


# ---------------------------------------------------------------------------
# 2918: three_balls: exp(i*pi*[1,2,4,8]/(z1+z2+4)).
# ---------------------------------------------------------------------------
def _tb_lines(U: str, V: str) -> list[str]:
    # Park s = z1+z2+4 first (the fused expression breaks the 256-char arg
    # cap), then one scan: V_k = 1,2,4,8 spelled floor(exp(k*ln2)+0.5) —
    # exact integers — and cf_k = exp(i*pi*V_k/s).
    F = f"exp({PII}*floor(exp(k*0.6931471805599453)+0.5)/tos[0])"
    return [
        f"poly = scan(4, 0, exp({WI}*({U}))+exp({WI}*({V}))+4, 0)",
        "poly",
        f"poly = scan(4, 0, {F}, {F})",
        "drop",
    ]


def _tb_np(u: float, v: float) -> np.ndarray:
    s = np.exp(2j * np.pi * u) + np.exp(2j * np.pi * v) + 4
    return np.exp(1j * np.pi * np.array([1, 2, 4, 8]) / s)


# ---------------------------------------------------------------------------
# mix scaffolding: acc = sum_j w_j * eval_j  (evals build poly in place)
# ---------------------------------------------------------------------------
def _mixed(evals: list[list[str]], weights: list[float]) -> list[str]:
    lines: list[str] = []
    for j, (ev, w) in enumerate(zip(evals, weights)):
        lines += ev
        lines.append(f"poly = multiply(poly, {_num(w)})")
        if j > 0:
            lines.append("poly = add(poly, pop)")
        if j < len(evals) - 1:
            lines.append("poly")
    return lines


def build_source_text(name: str) -> str:
    if name == "giga_2912":
        w = _weights_2912()
        evals = [_p11b2_lines(*_draw_pair(j)) + _CUMSUM_REV for j in range(2)]
        lines = _mixed(evals, w)
    elif name == "giga_2913":
        w = _weights_2913()
        evals = [_p11b2_lines(*_draw_pair(j)) + _CUMSUM_REV for j in range(5)]
        lines = _mixed(evals, w)
    elif name == "giga_2914":
        evals = [_uc12_lines(*_draw_pair(j)) for j in range(3)]
        lines = _mixed(evals, W_2914)
    elif name == "giga_2915":
        evals = [_g88_lines(*_draw_pair(j)) for j in range(4)]
        lines = _mixed(evals, W_2915)
    elif name == "giga_2916":
        trip = ["poly = rev(poly)",
                "poly = roots_cm(poly, lo, exact)",
                "poly = poly[0:10]"]
        evals = [_p11b2_lines(*_draw_pair(j)) + trip for j in range(2)]
        lines = _mixed(evals, [W_2916_A, W_2916_B])
        wE = 1.0 - W_2916_A - W_2916_B
        lines += ["poly",
                  f"poly = vector_literal({_lit([wE * z for z in E_2916])})",
                  "poly = add(poly, pop)"]
    elif name == "giga_2917":
        # parked [Arg v1, Arg v2, |v1|, 0...] via a gated scan (chip-cheap)
        park0 = f"angle(exp({WI}*t1)+exp({WI}*t2))"
        parkstep = (f"(floor(2/(k+1)))*angle(exp({WI}*(t1+t2)))"
                    f"+(floor(3/(k+1))-floor(2/(k+1)))"
                    f"*abs(exp({WI}*t1)+exp({WI}*t2))")
        lines = [
            f"poly = scan(9, 0, {park0}, {parkstep})",
            "poly",
            f"poly = scan(9, 1, {F_2917}, {F_2917})",
            "drop",
            "poly = rev(poly)",
            f"poly = multiply(poly, {_num(W_2917_CF)})",
            "poly",
            f"poly = vector_literal({_lit([5e-5 * z for z in E_2917])})",
            "poly = add(poly, pop)",
        ]
    elif name == "giga_2918":
        evals = [_tb_lines(*_draw_pair(j)) for j in range(9)]
        lines = _mixed(evals, W_2918)
    else:
        raise KeyError(name)
    return "\n".join(lines + ["emit"]) + "\n"


# ---------------------------------------------------------------------------
# analog oracles (exact mirrors of the emitted programs; used by the tests)
# ---------------------------------------------------------------------------
def oracle_row(name: str, u: float, v: float) -> np.ndarray:
    if name == "giga_2912":
        w = _weights_2912()
        return sum(wj * _cf2912_np(*_draw_pair_np(j, u, v))
                   for j, wj in enumerate(w))
    if name == "giga_2913":
        w = _weights_2913()
        return sum(wj * _cf2912_np(*_draw_pair_np(j, u, v))
                   for j, wj in enumerate(w))
    if name == "giga_2914":
        return sum(wj * _uc12_np(*_draw_pair_np(j, u, v))
                   for j, wj in enumerate(W_2914))
    if name == "giga_2915":
        return sum(wj * _g88_np(*_draw_pair_np(j, u, v))
                   for j, wj in enumerate(W_2915))
    if name == "giga_2916":
        rts = []
        for j in range(2):
            uj, vj = _draw_pair_np(j, u, v)
            rts.append(np.roots(np.flip(_p11b2_np(uj, vj))))
        wE = 1.0 - W_2916_A - W_2916_B
        return (W_2916_A * rts[0] + W_2916_B * rts[1]
                + wE * np.array(E_2916))
    if name == "giga_2917":
        return (W_2917_CF * _cf2917_np(u, v)
                + 5e-5 * np.array(E_2917))
    if name == "giga_2918":
        return sum(wj * _tb_np(*_draw_pair_np(j, u, v))
                   for j, wj in enumerate(W_2918))
    raise KeyError(name)


NAMES = [f"giga_{n}" for n in range(2912, 2919)]


def build_document(name: str) -> dict:
    return {
        "version": 1,
        "program_kind": "coeff_program",
        "name": name,
        "chain": [],
        "source_text": build_source_text(name),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    rc = 0
    for name in NAMES:
        path = ROOT / f"{name}.coeff-program.json"
        serialized = json.dumps(build_document(name), indent=2) + "\n"
        if args.check:
            if not path.exists() or path.read_text() != serialized:
                print(f"STALE {path}")
                rc = 1
            else:
                print(f"OK {path}")
        else:
            path.write_text(serialized)
            print(f"wrote {path}")
    return rc


if __name__ == "__main__":
    raise SystemExit(main())
