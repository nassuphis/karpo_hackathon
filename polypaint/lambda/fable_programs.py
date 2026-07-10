"""fable-N: original parametric coefficient functions as Coeff Programs.

Unlike port_poly100_programs.py (which ports existing Python functions),
these are new families authored directly in the coeff program source
language, with a numpy reference implementation for the parity gate.

Usage:
  python lambda/fable_programs.py --show            # print sources
  python lambda/fable_programs.py --dry-run         # compile + parity only
  python lambda/fable_programs.py                   # + upload to S3
  python lambda/fable_programs.py --only fable-3 --force

Run against `const` with all-zero params: the base contributes nothing but
the degree; every program reads poly_len, so one program serves any degree.
"""
import argparse
import os
import sys

import numpy as np

LAMBDA_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, LAMBDA_DIR)

from coeff_program_source import compile_coeff_program_source
from port_poly100_programs import (
    PARITY_REL_TOL, build_payload, list_existing_keys, parity_check, upload,
)

N_PARITY = 36  # parity-gate coefficient count; sources are degree-agnostic


def _k(n):
    return np.arange(n, dtype=np.complex128)


# --- fable-1: phase braid ---------------------------------------------------
# Quadratic (Gauss-sum) phase: c_k = exp(i*(a*k^2/n + b*k)),
# a = angle(p1), b = angle(p2). Curved root lattices that shear as p1 turns.
FABLE_1 = """\
# fable-1: phase braid — c_k = exp(i*(angle(p1)*k^2/n + angle(p2)*k))
push_range(0, poly_len, 1)
dup
multiply(pop, pop)
linear((1i * angle(p1) / poly_len), 0)
poly = exp(pop)
push_range(0, poly_len, 1)
linear((1i * angle(p2)), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_1(p1, p2, n=N_PARITY):
    k = _k(n)
    return np.exp(1j * (np.angle(p1) * k * k / n + np.angle(p2) * k))


# --- fable-2: spiral comb ---------------------------------------------------
# Lacunary: only every 4th slot is live; live slots spiral with angle(p1)
# and swell/decay with |p2|. Root necklaces that pinch as p2 leaves the disk.
FABLE_2 = """\
# fable-2: spiral comb — c_k = [k%4 == (n-1)%4] * exp(k*(i*angle(p1) + log(abs(p2))/n))
# comb aligned to the LEADING slot (ascending coeffs): a comb that misses
# slot n-1 zeroes the leading coefficient and degenerates the solve
push_range(0, poly_len, 1)
linear(((1i * angle(p1)) + (log(abs(p2)) / poly_len)), 0)
poly = exp(pop)
fill(poly_len, 4)
push_range(0, poly_len, 1)
rem(pop, pop)
fill(poly_len, ((poly_len - 1) - (4 * floor((poly_len - 1) / 4))))
eq(pop, pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_2(p1, p2, n=N_PARITY):
    k = _k(n)
    live = (np.arange(n) % 4 == (n - 1) % 4).astype(np.complex128)
    return live * np.exp(k * (1j * np.angle(p1) + np.log(abs(p2)) / n))


# --- fable-3: palindrome pulse ----------------------------------------------
# Self-inversive-ish: w_k + conj(w_{n-1-k}) with w a travelling gaussian
# bump, center driven by angle(p2), phase twist by angle(p1). Roots hug the
# unit circle and part where the bump sits.
FABLE_3 = """\
# fable-3: palindrome pulse — w_k + conj(w_(n-1-k)),
# w_k = exp(-((k - mu)/(n/6))^2) * exp(i*k*angle(p1)),
# mu = (n-1) * (angle(p2)/tau + 0.5)
push_range(0, poly_len, 1)
linear((6.0 / poly_len), (-6.0 * ((poly_len - 1) * ((angle(p2) / tau) + 0.5)) / poly_len))
dup
multiply(pop, pop)
neg(pop)
exp(pop)
push_range(0, poly_len, 1)
linear((1i * angle(p1)), 0)
exp(pop)
poly = multiply(pop, pop)
push_range((poly_len - 1), -1, -1)
linear((6.0 / poly_len), (-6.0 * ((poly_len - 1) * ((angle(p2) / tau) + 0.5)) / poly_len))
dup
multiply(pop, pop)
neg(pop)
exp(pop)
push_range((poly_len - 1), -1, -1)
linear((-1i * angle(p1)), 0)
exp(pop)
multiply(pop, pop)
poly = add(poly, pop)
emit
"""


def ref_fable_3(p1, p2, n=N_PARITY):
    k = np.arange(n, dtype=np.float64)
    mu = (n - 1) * (np.angle(p2) / (2 * np.pi) + 0.5)
    sig = n / 6.0
    w = np.exp(-(((k - mu) / sig) ** 2)) * np.exp(1j * k * np.angle(p1))
    kr = k[::-1]
    wr = np.exp(-(((kr - mu) / sig) ** 2)) * np.exp(-1j * kr * np.angle(p1))
    return w + wr


# --- fable-4: scan cascade --------------------------------------------------
# Recurrence via scan: c_0 = 1, c_k = c_(k-1)*(0.92*p1) + cos(k*angle(p2)).
# Geometric memory + oscillating drive: logarithmic-spiral root clouds.
FABLE_4 = """\
# fable-4: scan cascade — c_0=1, c_k = c_(k-1)*0.92*p1 + cos(k*angle(p2))
scan(poly_len, 0, 1, (prev * (0.92 * p1)) + cos(k * angle(p2)))
poly = pop
emit
"""


def ref_fable_4(p1, p2, n=N_PARITY):
    out = np.zeros(n, dtype=np.complex128)
    out[0] = 1.0
    for k in range(1, n):
        out[k] = out[k - 1] * (0.92 * p1) + np.cos(k * np.angle(p2))
    return out


# --- fable-5: chebyshev tilt ------------------------------------------------
# Kac-like ring with correlated phases: c_k = cos(k*angle(p1)) +
# i*sin(k*angle(p2)). Near-unit-circle root rings that split as the two
# angles decouple.
FABLE_5 = """\
# fable-5: chebyshev tilt — c_k = cos(k*angle(p1)) + 1i*sin(k*angle(p2))
push_range(0, poly_len, 1)
linear(angle(p1), 0)
cos(pop)
push_range(0, poly_len, 1)
linear(angle(p2), 0)
sin(pop)
linear(1i, 0)
poly = add(pop, pop)
emit
"""


def ref_fable_5(p1, p2, n=N_PARITY):
    k = _k(n)
    return np.cos(k * np.angle(p1)) + 1j * np.sin(k * np.angle(p2))


# --- fable-6: weyl walk -----------------------------------------------------
# Partial sums of unit phases (Cornu-spiral walk): c_k = cumsum of
# exp(i*(angle(p1)*j^2/n + angle(p2)*j)), normalized by sqrt(n).
FABLE_6 = """\
# fable-6: weyl walk — c_k = (1/sqrt(n)) * sum_(j<=k) exp(i*(angle(p1)*j^2/n + angle(p2)*j))
push_range(0, poly_len, 1)
dup
multiply(pop, pop)
linear((1i * angle(p1) / poly_len), 0)
poly = exp(pop)
push_range(0, poly_len, 1)
linear((1i * angle(p2)), 0)
exp(pop)
poly = multiply(poly, pop)
push(poly)
cumsum()
poly = pop
poly = linear(poly, (1.0 / sqrt(poly_len)), 0)
emit
"""


def ref_fable_6(p1, p2, n=N_PARITY):
    k = _k(n)
    ph = np.exp(1j * (np.angle(p1) * k * k / n + np.angle(p2) * k))
    return np.cumsum(ph) / np.sqrt(n)


# --- fable-7: szegő teardrop -------------------------------------------------
# Truncated exponential: c_k = lam^k / k!, lam = 0.9*n*p1. Factorial decay
# beats the unit-circle law — roots trace the Szegő curve |z e^(1-z)| = 1,
# a teardrop that rotates and breathes with p1. Nothing circular about it.
FABLE_7 = """\
# fable-7: szego teardrop — c_k = lam^k/k!, lam = 0.9*poly_len*p1
scan(poly_len, 0, 1, prev * (((0.9 * poly_len) * p1) / k))
poly = pop
emit
"""


def ref_fable_7(p1, p2, n=N_PARITY):
    lam = 0.9 * n * p1
    out = np.zeros(n, dtype=np.complex128)
    out[0] = 1.0
    for k in range(1, n):
        out[k] = out[k - 1] * lam / k
    return out


# --- fable-8: theta spiral ----------------------------------------------------
# Partial theta: c_k = q^(k^2), |q| = 0.9..0.98 from |p2|, arg from p1.
# Super-geometric decay puts root j at radius ~ |q|^-(2j+1): a genuine
# logarithmic spiral, arms winding with angle(p1).
FABLE_8 = """\
# fable-8: theta spiral — c_k = q^(k^2), log|q| = -B/(n-1)^2 so the decay
# budget B (not |q|) is what's fixed: outermost root ~ e^(2B/(n-1)) stays
# ~20x at any degree AND the leading coeff e^(-B) stays f32-transportable
# (B capped at 60*1.15 = 69 < ln(1e30)). |p2| breathes B by +-15%,
# angle(p1) winds the arms.
scan(poly_len, 0, 1, prev * exp(((2 * k) - 1) * (((0 - ((0.85 + ((0.3 * abs(p2)) / (1 + abs(p2)))) * (0.5 * ((60 + (1.5 * (poly_len - 1))) - abs(60 - (1.5 * (poly_len - 1))))))) / ((poly_len - 1) * (poly_len - 1))) + ((1i * angle(p1)) / poly_len))))
poly = pop
emit
"""


def ref_fable_8(p1, p2, n=N_PARITY):
    breathe = 0.85 + 0.3 * abs(p2) / (1 + abs(p2))
    base = 0.5 * ((60 + 1.5 * (n - 1)) - abs(60 - 1.5 * (n - 1)))  # min(60, 1.5(n-1))
    logq = -(breathe * base) / ((n - 1) * (n - 1)) + 1j * np.angle(p1) / n
    out = np.zeros(n, dtype=np.complex128)
    out[0] = 1.0
    for k in range(1, n):
        out[k] = out[k - 1] * np.exp((2 * k - 1) * logq)
    return out


# --- fable-9: tan line ---------------------------------------------------------
# (1 + itz)^m + (1 - itz)^m with t = 0.9*p1, m = poly_len-1: binomial
# magnitudes, cos(pi*k/2) sign comb. Roots sit on a straight LINE through
# the origin (tan-spaced, dense center, sparse ends), rotating with p1.
FABLE_9 = """\
# fable-9: tan line — even/odd part of 2*(1+tz)^m, parity matched to the
# LEADING slot so the top coefficient stays alive at any degree:
# c_k = 2*C(m,k)*(0.9*p1)^k * [k%2 == (n-1)%2]  (exact 0/1 mask, no fuzz)
scan(poly_len, 0, 2, prev * ((0.9 * p1) * ((poly_len - k) / k)))
poly = pop
fill(poly_len, 2)
push_range(0, poly_len, 1)
rem(pop, pop)
fill(poly_len, ((poly_len - 1) - (2 * floor((poly_len - 1) / 2))))
eq(pop, pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_9(p1, p2, n=N_PARITY):
    t = 0.9 * p1
    out = np.zeros(n, dtype=np.complex128)
    out[0] = 2.0
    for k in range(1, n):
        out[k] = out[k - 1] * t * (n - k) / k
    live = (np.arange(n) % 2 == (n - 1) % 2).astype(np.complex128)
    return out * live


# --- fable-10: sine butterfly ---------------------------------------------------
# Truncated sine: c_k = (lam^k/k!) * sin(pi*k/2) — exactly the Taylor
# coefficients of sin(lam*z). Two conjugate Szegő teardrops joined at the
# origin: a butterfly, wings tipping with angle(p1).
FABLE_10 = """\
# fable-10: sine butterfly — truncated sin(lam*z) when the leading slot is
# odd, truncated cos(lam*z) when it is even (same two-teardrop butterfly;
# the comb parity must match slot n-1 or the leading coefficient dies):
# c_k = (lam^k/k!) * sin(pi*(k + 1 - p)/2) * [k%2 == p], p = (n-1)%2
scan(poly_len, 0, 1, prev * (((0.85 * poly_len) * p1) / k))
poly = pop
push_range(0, poly_len, 1)
linear((pi / 2), ((pi / 2) * (1 - ((poly_len - 1) - (2 * floor((poly_len - 1) / 2))))))
sin(pop)
poly = multiply(poly, pop)
fill(poly_len, 2)
push_range(0, poly_len, 1)
rem(pop, pop)
fill(poly_len, ((poly_len - 1) - (2 * floor((poly_len - 1) / 2))))
eq(pop, pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_10(p1, p2, n=N_PARITY):
    lam = 0.85 * n * p1
    out = np.zeros(n, dtype=np.complex128)
    out[0] = 1.0
    for k in range(1, n):
        out[k] = out[k - 1] * lam / k
    k = np.arange(n)
    par = (n - 1) % 2
    comb = np.sin(np.pi * (k + 1 - par) / 2)
    live = (k % 2 == par).astype(np.complex128)
    return out * comb * live


# --- fable-11: exp trio --------------------------------------------------------
# Interference of three truncated exponentials: c_k = (l1^k + l2^k + l3^k)/k!,
# l1 = 0.9n*p1, l2 = 0.9n*p2, l3 = 0.9n*p1*p2 (sum frequency). Zeros gather
# on the tram lines where two exponentials balance — a triangle network that
# SHEARS as the three lambdas move relative to each other. Nothing rigid:
# no z -> c*z substitution maps one frame to the next.
FABLE_11 = """\
# fable-11: exp trio — c_k = (l1^k + l2^k + l3^k)/k!,
# l1 = 0.9n*p1, l2 = 0.9n*p2, l3 = 0.9n*p1*p2
scan(poly_len, 0, 1, prev * (((0.9 * poly_len) * p1) / k))
poly = pop
scan(poly_len, 0, 1, prev * (((0.9 * poly_len) * p2) / k))
poly = add(poly, pop)
scan(poly_len, 0, 1, prev * (((0.9 * poly_len) * (p1 * p2)) / k))
poly = add(poly, pop)
emit
"""


def _exp_series(lam, n):
    out = np.zeros(n, dtype=np.complex128)
    out[0] = 1.0
    for k in range(1, n):
        out[k] = out[k - 1] * lam / k
    return out


def ref_fable_11(p1, p2, n=N_PARITY):
    return (_exp_series(0.9 * n * p1, n) + _exp_series(0.9 * n * p2, n)
            + _exp_series(0.9 * n * (p1 * p2), n))


# --- fable-12: chirped teardrop --------------------------------------------------
# fable-7 with a quadratic chirp: c_k = (lam^k/k!) * exp(i*angle(p2)*k^2/n).
# The k^2 phase lives in coefficient space, so rotating p1 is NO LONGER a
# rotation of z: the teardrop bends, dents and breathes instead of spinning
# rigidly; angle(p2) controls the warp.
FABLE_12 = """\
# fable-12: chirped teardrop — c_k = (lam^k/k!) * exp(1i*(angle(p1)+angle(p2))*k^2/n),
# lam = 0.9*poly_len*p1. The chirp tracks angle(p1) too: sweeping p1 warps
# the teardrop at the same rate it turns it — no rigid frames anywhere.
scan(poly_len, 0, 1, prev * (((0.9 * poly_len) * p1) / k))
poly = pop
push_range(0, poly_len, 1)
dup
multiply(pop, pop)
linear(((1i * (angle(p1) + angle(p2))) / poly_len), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_12(p1, p2, n=N_PARITY):
    k = np.arange(n, dtype=np.float64)
    return (_exp_series(0.9 * n * p1, n)
            * np.exp(1j * (np.angle(p1) + np.angle(p2)) * k * k / n))


# --- recurrence helpers (mirror the VM's scan seeding exactly) -----------------
def _ratio_series(n, c0, ratio):
    """c_0 = c0, c_k = c_(k-1) * ratio(k). Matches scan(len, 0, c0, step)."""
    out = np.zeros(n, dtype=np.complex128)
    out[0] = c0
    for k in range(1, n):
        out[k] = out[k - 1] * ratio(k)
    return out


def _two_term(n, c0, c1, step):
    """c_0=c0, c_1=c1, c_k = step(prev, prev2, k). Matches the 5-arg scan."""
    out = np.zeros(n, dtype=np.complex128)
    out[0], out[1] = c0, c1
    for k in range(2, n):
        out[k] = step(out[k - 1], out[k - 2], k)
    return out


# The hypergeometric families below are all one 3-token scan: a coefficient
# ratio c_k/c_(k-1) that is rational in k gives a different pFq, and each pFq
# has its own root geometry (teardrop, cardioid, real line, ...).

# --- fable-13: kummer confluent (1F1) -----------------------------------------
# c_k = c_(k-1) * (a+k-1)*x / ((b+k-1)*k), a=1+2*p1, b=1.5, x=0.8*n*p2.
# One numerator Pochhammer over one denominator: a warped teardrop that
# breathes with p1 (the a-parameter) and rotates with p2 (the argument).
FABLE_13 = """\
# fable-13: kummer 1F1 — c_k = c_(k-1)*((2*p1)+k)*x/((k-0.5)*k), x=0.8*poly_len*p2
scan(poly_len, 0, 1, prev * (((((2 * p1) + k) * ((0.8 * poly_len) * p2))) / (((k - 0.5) * k))))
poly = pop
emit
"""


def ref_fable_13(p1, p2, n=N_PARITY):
    x = 0.8 * n * p2
    return _ratio_series(n, 1.0, lambda k: ((2 * p1 + k) * x) / ((k - 0.5) * k))


# --- fable-14: gauss hypergeometric (2F1) -------------------------------------
# c_k = c_(k-1) * (a+k-1)(b+k-1)*x / ((c+k-1)*k), a=1+p2, b=0.5, c=1.5, x=0.75*p1.
# TWO numerator Pochhammers over one denominator: a branch-point family whose
# roots hug an arc from the |x|<1 radius, splitting as a and c decouple.
FABLE_14 = """\
# fable-14: gauss 2F1 — c_k = c_(k-1)*((p2+k)(k-0.5)*0.75*p1)/((k+0.5)*k)
scan(poly_len, 0, 1, prev * (((((p2 + k) * (k - 0.5)) * (0.75 * p1))) / (((k + 0.5) * k))))
poly = pop
emit
"""


def ref_fable_14(p1, p2, n=N_PARITY):
    x = 0.75 * p1
    return _ratio_series(n, 1.0, lambda k: ((p2 + k) * (k - 0.5) * x) / ((k + 0.5) * k))


# --- fable-15: fm sidebands ----------------------------------------------------
# c_k = 0.94^k * exp(i*A*sin(w*k)), A=3*angle(p1), w=angle(p2)+0.7. A frequency
# -modulated phase on a geometric envelope: the sin-of-phase sprays Bessel-
# function sidebands, so the roots gather into evenly-spaced clusters around a
# circle that shift as the modulation depth (p1) and rate (p2) change.
FABLE_15 = """\
# fable-15: fm sidebands — c_k = 0.94^k exp(i*3*angle(p1)*sin((angle(p2)+0.7)*k))
push_range(0, poly_len, 1)
linear((angle(p2) + 0.7), 0)
sin(pop)
linear((1i * (3 * angle(p1))), 0)
poly = exp(pop)
push_range(0, poly_len, 1)
linear((log(0.94)), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_15(p1, p2, n=N_PARITY):
    k = np.arange(n, dtype=np.float64)
    ph = np.exp(1j * (3 * np.angle(p1)) * np.sin((np.angle(p2) + 0.7) * k))
    return ph * np.exp(k * np.log(0.94))


# --- fable-16: catalan cardioid ------------------------------------------------
# c_k = c_(k-1) * 2(2k-1)/(k+1) * t, t=0.24*p1. The Catalan generating function
# has a square-root branch point at t=1/4; truncated, its roots crowd onto the
# cardioid-like cut, swinging around with p1.
FABLE_16 = """\
# fable-16: catalan — c_k = c_(k-1)*2(2k-1)/(k+1)*0.24*p1
scan(poly_len, 0, 1, prev * (((2 * ((2 * k) - 1)) / (k + 1)) * (0.24 * p1)))
poly = pop
emit
"""


def ref_fable_16(p1, p2, n=N_PARITY):
    return _ratio_series(n, 1.0, lambda k: (2 * (2 * k - 1) / (k + 1)) * (0.24 * p1))


# --- fable-17: laguerre line ---------------------------------------------------
# Coefficients of L_(n-1)(x/·) with an argument scale: c_k = c_(k-1)*(-(n-k))*x/k^2,
# x=0.7*n*p1. Laguerre roots are POSITIVE REAL — a straight comb of roots on a
# ray, rotating with arg(p1). As far from a circle as it gets.
FABLE_17 = """\
# fable-17: laguerre — c_k = c_(k-1)*(-(poly_len-k))*x/k^2, x=0.7*poly_len*p1
scan(poly_len, 0, 1, prev * ((((0 - (poly_len - k)) * ((0.7 * poly_len) * p1))) / ((k * k))))
poly = pop
emit
"""


def ref_fable_17(p1, p2, n=N_PARITY):
    x = 0.7 * n * p1
    return _ratio_series(n, 1.0, lambda k: (-(n - k) * x) / (k * k))


# --- fable-18: linear recurrence (fibonacci/lucas) ----------------------------
# c_0=1, c_1=p1, c_k = p1*c_(k-1) + p2*c_(k-2). The truncated 1/(1-p1 z-p2 z^2):
# roots cluster near the reciprocals of the characteristic roots and drift as
# p1,p2 move — a two-body root dance.
FABLE_18 = """\
# fable-18: linear recurrence — c_0=1, c_1=p1, c_k = p1*prev + p2*prev2
scan(poly_len, 0, 1, p1, ((p1 * prev) + (p2 * prev2)))
poly = pop
emit
"""


def ref_fable_18(p1, p2, n=N_PARITY):
    return _two_term(n, 1.0, p1, lambda a, b, k: p1 * a + p2 * b)


# --- fable-19: chebyshev-form recurrence --------------------------------------
# c_0=1, c_1=p1, c_k = 2*p1*c_(k-1) - p2*c_(k-2). The Chebyshev/Dickson three-term
# shape: roots ride near-circular arcs that pinch into lenses as p2 leaves 1.
FABLE_19 = """\
# fable-19: chebyshev recurrence — c_0=1, c_1=p1, c_k = 2*p1*prev - p2*prev2
scan(poly_len, 0, 1, p1, (((2 * p1) * prev) - (p2 * prev2)))
poly = pop
emit
"""


def ref_fable_19(p1, p2, n=N_PARITY):
    return _two_term(n, 1.0, p1, lambda a, b, k: 2 * p1 * a - p2 * b)


# --- fable-20: fresnel packet --------------------------------------------------
# A Gaussian window times a quadratic chirp: c_k = exp(-((k-mu)/sig)^2) *
# exp(i*(3*angle(p1))*k^2/n + i*angle(p2)*k). A localized wave packet whose
# zeros lie on curved Fresnel zones that bend with the chirp rate.
FABLE_20 = """\
# fable-20: fresnel packet — gaussian window (mu=(n-1)/2, sig=n/4) x quadratic chirp
push_range(0, poly_len, 1)
linear((4.0 / poly_len), ((-4.0 * ((poly_len - 1) / 2)) / poly_len))
dup
multiply(pop, pop)
neg(pop)
poly = exp(pop)
push_range(0, poly_len, 1)
dup
multiply(pop, pop)
linear(((1i * (3 * angle(p1))) / poly_len), 0)
exp(pop)
poly = multiply(poly, pop)
push_range(0, poly_len, 1)
linear((1i * angle(p2)), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_20(p1, p2, n=N_PARITY):
    k = np.arange(n, dtype=np.float64)
    mu = (n - 1) / 2.0
    sig = n / 4.0
    g = np.exp(-(((k - mu) / sig) ** 2))
    ph = np.exp(1j * (3 * np.angle(p1)) * k * k / n + 1j * np.angle(p2) * k)
    return g * ph


# --- fable-21: dirichlet strip -------------------------------------------------
# c_k = (k+1)^(-s), s = sigma + i*(6*angle(p1)), sigma=0.5..0.8 from |p2|. A
# truncated Dirichlet series: its zeros crowd toward a vertical strip that
# leans with the imaginary height — a nod to the zeta landscape.
FABLE_21 = """\
# fable-21: dirichlet strip — c_k = (k+1)^(-s), s = (0.5 + 0.3|p2|/(1+|p2|)) + i*6*angle(p1)
push_range(1, (poly_len + 1), 1)
log(pop)
linear((0 - ((0.5 + ((0.3 * abs(p2)) / (1 + abs(p2)))) + (1i * (6 * angle(p1))))), 0)
poly = exp(pop)
emit
"""


def ref_fable_21(p1, p2, n=N_PARITY):
    kk = np.arange(1, n + 1, dtype=np.float64)
    s = (0.5 + 0.3 * abs(p2) / (1 + abs(p2))) + 1j * (6 * np.angle(p1))
    return np.exp(-s * np.log(kk))


# --- fable-22: cubic chirp -----------------------------------------------------
# c_k = exp(i*(angle(p1)*k^3/n^2 + angle(p2)*k)). A cubic phase (vs. fable-1's
# quadratic) makes a three-fold twisted lattice that shears without ever being
# a rotation of z.
FABLE_22 = """\
# fable-22: cubic chirp — c_k = exp(i*(angle(p1)*k^3/n^2 + angle(p2)*k))
push_range(0, poly_len, 1)
dup
dup
multiply(pop, pop)
multiply(pop, pop)
linear(((1i * angle(p1)) / (poly_len * poly_len)), 0)
poly = exp(pop)
push_range(0, poly_len, 1)
linear((1i * angle(p2)), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_22(p1, p2, n=N_PARITY):
    k = np.arange(n, dtype=np.float64)
    return np.exp(1j * (np.angle(p1) * k * k * k / (n * n) + np.angle(p2) * k))


# --- fable-23: am triad --------------------------------------------------------
# c_k = (1 + 0.8*cos(w*k)) * (0.95 e^{i angle(p2)})^k, w=2*angle(p1). Amplitude
# modulation splits the spectrum into a carrier plus two sidebands, so the roots
# organize into three interleaved rings that beat against each other as the
# modulation frequency (p1) sweeps.
FABLE_23 = """\
# fable-23: am triad — c_k = (1 + 0.8 cos(2*angle(p1)*k)) (0.95 e^{i angle(p2)})^k
push_range(0, poly_len, 1)
linear((2 * angle(p1)), 0)
cos(pop)
linear(0.8, 1)
poly = pop
push_range(0, poly_len, 1)
linear(((1i * angle(p2)) + log(0.95)), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_23(p1, p2, n=N_PARITY):
    k = np.arange(n, dtype=np.float64)
    am = 1 + 0.8 * np.cos(2 * np.angle(p1) * k)
    dec = np.exp(k * (1j * np.angle(p2) + np.log(0.95)))
    return am * dec


# --- fable-24: two-spiral moire ------------------------------------------------
# c_k = q1^k + q2^k, q1=0.9 e^{i angle(p1)}, q2=0.82 e^{i angle(p2)}. Two geometric
# spirals of different radius interfere; their zeros fall on a moire lattice
# where q1^k = -q2^k, breathing as the two angles slide.
FABLE_24 = """\
# fable-24: two-spiral moire — c_k = (0.97 e^{i angle(p1)})^k + (0.55 e^{i angle(p2)})^k
push_range(0, poly_len, 1)
linear(((log(0.97)) + (1i * angle(p1))), 0)
poly = exp(pop)
push_range(0, poly_len, 1)
linear(((log(0.55)) + (1i * angle(p2))), 0)
exp(pop)
poly = add(poly, pop)
emit
"""


def ref_fable_24(p1, p2, n=N_PARITY):
    k = np.arange(n, dtype=np.float64)
    return np.exp(k * (np.log(0.97) + 1j * np.angle(p1))) + np.exp(k * (np.log(0.55) + 1j * np.angle(p2)))


# Degree-scaled log|q| for the theta families (q^(k^2) decay): a fixed |q|<1
# underflows the leading slot at high degree (0.93^(35^2) is below f32). Fix the
# DECAY BUDGET B instead so the leading ~ e^-B stays transportable and the
# outermost root radius holds at any degree — the trick fable-8 uses.
# log|q| = -min(52, 1.5(n-1)) / (n-1)^2   (min via the abs identity).
_DECAY_SRC = ("((0 - (0.5 * ((52 + (1.5 * (poly_len - 1))) - "
              "abs(52 - (1.5 * (poly_len - 1)))))) / ((poly_len - 1) * (poly_len - 1)))")


def _decay_logmag(n, cap=52.0):
    base = 0.5 * ((cap + 1.5 * (n - 1)) - abs(cap - 1.5 * (n - 1)))  # min(cap, 1.5(n-1))
    return -base / ((n - 1) * (n - 1))


# --- fable-25: rogers-ramanujan -----------------------------------------------
# c_k = q^(k^2) / (q;q)_k, |q| degree-scaled, arg=angle(p1)/n. Partition-theoretic:
# a theta numerator over a q-factorial denominator makes a spiral with widening
# gaps, the Rogers-Ramanujan signature.
FABLE_25 = f"""\
# fable-25: rogers-ramanujan — c_k = q^(k^2)/(q;q)_k, log|q| degree-scaled
scan(poly_len, 0, 1, prev * exp(((2 * k) - 1) * ({_DECAY_SRC} + ((1i * angle(p1)) / poly_len))))
poly = pop
scan(poly_len, 0, 1, prev / (1 - exp(k * ({_DECAY_SRC} + ((1i * angle(p1)) / poly_len)))))
poly = multiply(poly, pop)
emit
"""


def ref_fable_25(p1, p2, n=N_PARITY):
    logq = _decay_logmag(n) + 1j * np.angle(p1) / n
    theta = _ratio_series(n, 1.0, lambda k: np.exp((2 * k - 1) * logq))
    denom = _ratio_series(n, 1.0, lambda k: 1.0 / (1.0 - np.exp(k * logq)))
    return theta * denom


# --- fable-26: sheared theta ---------------------------------------------------
# c_k = q^(k^2) * r^k, q=0.93 e^{i angle(p1)/n}, r=e^{i angle(p2)}. A log-spiral
# (fable-8) given a linear phase tilt: the spiral shears sideways with p2 while
# it winds with p1.
FABLE_26 = f"""\
# fable-26: sheared theta — c_k = q^(k^2) e^{{i angle(p2) k}}, log|q| degree-scaled
scan(poly_len, 0, 1, prev * exp(((((2 * k) - 1) * ({_DECAY_SRC} + ((1i * angle(p1)) / poly_len))) + (1i * angle(p2)))))
poly = pop
emit
"""


def ref_fable_26(p1, p2, n=N_PARITY):
    logq = _decay_logmag(n) + 1j * np.angle(p1) / n
    logr = 1j * np.angle(p2)
    return _ratio_series(n, 1.0, lambda k: np.exp((2 * k - 1) * logq + logr))


# --- fable-27: heat-kernel spiral ---------------------------------------------
# c_k = exp(-(a - i*angle(p1)) k^2 / n + i*angle(p2) k), a=1.2. Gaussian magnitude
# DECAY (not a window) with a quadratic phase: an inward-winding spiral of roots
# that tightens as p1 turns.
FABLE_27 = """\
# fable-27: heat-kernel spiral — c_k = exp((-1.2 + i angle(p1)) k^2/n + i angle(p2) k)
push_range(0, poly_len, 1)
dup
multiply(pop, pop)
linear((((0 - 1.2) + (1i * angle(p1))) / poly_len), 0)
exp(pop)
poly = pop
push_range(0, poly_len, 1)
linear((1i * angle(p2)), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_27(p1, p2, n=N_PARITY):
    k = np.arange(n, dtype=np.float64)
    return np.exp((-1.2 + 1j * np.angle(p1)) * k * k / n) * np.exp(1j * np.angle(p2) * k)


# --- fable-28: generalized binomial cut ---------------------------------------
# c_k = C(alpha, k) = c_(k-1)*(alpha-k+1)/k, alpha = n*p1 (complex). The truncated
# (1+z)^alpha: for non-integer complex alpha the roots spiral around the unit
# circle with a branch-cut gap toward z=-1.
FABLE_28 = """\
# fable-28: binomial cut — c_k = C(alpha,k), alpha = poly_len*p1
scan(poly_len, 0, 1, prev * (((((poly_len * p1) - k) + 1)) / (k)))
poly = pop
emit
"""


def ref_fable_28(p1, p2, n=N_PARITY):
    alpha = n * p1
    return _ratio_series(n, 1.0, lambda k: (alpha - k + 1) / k)


# --- fable-29: log-phase (riemann-siegel flavor) ------------------------------
# c_k = exp(i*angle(p1)*(k+1)*log(k+1)/n) * decay. A k*log(k) phase (the
# Riemann-Siegel theta shape) gives quasi-periodic, never-repeating root
# spacing; the 0.98^k decay keeps it bounded.
FABLE_29 = """\
# fable-29: log-phase — c_k = exp(i*2*angle(p1)(k+1)log(k+1)/poly_len) (0.9 e^{i angle(p2)})^k
push_range(1, (poly_len + 1), 1)
dup
log(pop)
multiply(pop, pop)
linear(((1i * (2 * angle(p1))) / poly_len), 0)
poly = exp(pop)
push_range(0, poly_len, 1)
linear(((1i * angle(p2)) + log(0.9)), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_29(p1, p2, n=N_PARITY):
    B = np.arange(1, n + 1, dtype=np.float64)
    phase = np.exp(1j * (2 * np.angle(p1)) * B * np.log(B) / n)
    k = np.arange(n, dtype=np.float64)
    dec = np.exp(k * (1j * np.angle(p2) + np.log(0.9)))
    return phase * dec


# --- fable-30: lacunary theta comb --------------------------------------------
# c_k = [k%3 == (n-1)%3] * q^(k^2), q=0.95 e^{i angle(p1)/n}. Only every third
# coefficient survives (comb aligned to the leading slot), spiralling: a
# three-fold gapped log-spiral.
FABLE_30 = f"""\
# fable-30: lacunary theta comb — every 3rd slot, q^(k^2), log|q| degree-scaled
scan(poly_len, 0, 1, prev * exp(((2 * k) - 1) * ({_DECAY_SRC} + ((1i * angle(p1)) / poly_len))))
poly = pop
fill(poly_len, 3)
push_range(0, poly_len, 1)
rem(pop, pop)
fill(poly_len, ((poly_len - 1) - (3 * floor((poly_len - 1) / 3))))
eq(pop, pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_30(p1, p2, n=N_PARITY):
    logq = _decay_logmag(n) + 1j * np.angle(p1) / n
    theta = _ratio_series(n, 1.0, lambda k: np.exp((2 * k - 1) * logq))
    k = np.arange(n)
    live = (k % 3 == (n - 1) % 3).astype(np.complex128)
    return theta * live


# --- fable-31: fejer window x chirp -------------------------------------------
# c_k = (1 - |2k-(n-1)|/n) * exp(i angle(p1) k^2/n + i angle(p2) k). A triangular
# (Fejer) magnitude — dense in the middle, tapering to the ends — modulated by a
# chirp: roots on nested curved shells.
FABLE_31 = """\
# fable-31: fejer x chirp — triangular magnitude x quadratic chirp
push_range(0, poly_len, 1)
linear(2, (0 - (poly_len - 1)))
mod(pop)
linear((0 - (1.0 / poly_len)), 1)
poly = pop
push_range(0, poly_len, 1)
dup
multiply(pop, pop)
linear(((1i * angle(p1)) / poly_len), 0)
exp(pop)
poly = multiply(poly, pop)
push_range(0, poly_len, 1)
linear((1i * angle(p2)), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_31(p1, p2, n=N_PARITY):
    k = np.arange(n, dtype=np.float64)
    tri = 1.0 - np.abs(2 * k - (n - 1)) / n
    ph = np.exp(1j * np.angle(p1) * k * k / n) * np.exp(1j * np.angle(p2) * k)
    return tri * ph


# --- fable-32: exp/bessel interference ----------------------------------------
# c_k = (0.9n*p1)^k/k! + 0F1(0.5n*p2). Two decay families — factorial (exp) and
# double-factorial (Bessel) — summed: their zeros fight along the seam where the
# two envelopes cross, a new filament network.
FABLE_32 = """\
# fable-32: exp/bessel interference — c_k = (0.9 poly_len p1)^k/k! + bessel(0.5 poly_len p2)
scan(poly_len, 0, 1, prev * (((0.9 * poly_len) * p1) / k))
poly = pop
scan(poly_len, 0, 1, prev * (((0.5 * poly_len) * p2) / (k * k)))
poly = add(poly, pop)
emit
"""


def ref_fable_32(p1, p2, n=N_PARITY):
    e = _ratio_series(n, 1.0, lambda k: (0.9 * n * p1) / k)
    b = _ratio_series(n, 1.0, lambda k: (0.5 * n * p2) / (k * k))
    return e + b


# =============================================================================
# The p11 family: the interesting shapes come NOT from smooth decay (which pins
# roots to a circle) but from a SAWTOOTH magnitude (k mod 2m — periodic resets
# carve scallops/petals), a param-derived INTEGER regime m (the sweep jumps
# between discrete patterns), and a param-winding phase. Ported from poly.py's
# p11a1/p11b and generalized. Magnitudes are exact integers, so the leading
# slot is either 0 (degree drops) or >=1 — no tiny-nonzero spurious roots.
# =============================================================================

# m in 1..13 from |p1+p2|: floor(5|p1+p2|) mod 13, +1 (>=1 so 2m>0).
_M13 = ("(1 + ((floor(5 * abs(p1 + p2))) - "
        "((floor((floor(5 * abs(p1 + p2))) / 13)) * 13)))")


def _m13(p1, p2):
    a = np.floor(5 * np.abs(p1 + p2))
    return 1 + (a - np.floor(a / 13) * 13)


# --- fable-33: petal gear (port of p11a1) -------------------------------------
# c_k = (k mod 2m) * exp(i*pi*k/((poly_len-1)/poly_len*(m+3+p1+p2))). Sawtooth
# magnitude with a param-set tooth count m, wound by a param-dependent phase:
# a scalloped gear-ring with radial spikes, the pattern re-teething as m jumps.
FABLE_33 = f"""\
# fable-33: petal gear — sawtooth (k mod 2m) x param winding, m=1+floor(5|p1+p2|)%13
fill(poly_len, (2 * {_M13}))
push_range(0, poly_len, 1)
rem(pop, pop)
poly = pop
push_range(0, poly_len, 1)
linear(((1i * pi * poly_len) / ((poly_len - 1) * (({_M13} + 3) + (p1 + p2)))), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_33(p1, p2, n=N_PARITY):
    m = _m13(p1, p2)
    k = np.arange(n, dtype=np.float64)
    sf = k % (2 * m)
    coef = 1j * np.pi * n / ((n - 1) * (m + 3 + p1 + p2))
    return sf * np.exp(coef * k)


# --- fable-34: spiral petal (port of p11b) ------------------------------------
# c_k = (k mod 2m) * exp(i*pi*poly_len*v^2/(m+3+p1+p2)), v=k/(poly_len-1). The
# sawtooth petals of fable-33 wound by a QUADRATIC (accelerating) phase instead
# of linear: the teeth pull out into long sweeping spiral arms — a pinwheel.
FABLE_34 = f"""\
# fable-34: spiral petal — sawtooth (k mod 2m) x quadratic winding exp(i pi n v^2/denom)
fill(poly_len, (2 * {_M13}))
push_range(0, poly_len, 1)
rem(pop, pop)
poly = pop
push_range(0, poly_len, 1)
linear((1.0 / (poly_len - 1)), 0)
dup
multiply(pop, pop)
linear(((1i * pi * poly_len) / (({_M13} + 3) + (p1 + p2))), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_34(p1, p2, n=N_PARITY):
    m = _m13(p1, p2)
    k = np.arange(n, dtype=np.float64)
    sf = k % (2 * m)
    v = k / (n - 1)
    coef = 1j * np.pi * n / (m + 3 + p1 + p2)
    return sf * np.exp(coef * v * v)


# --- fable-35: mandala (port of p11b2_v1) -------------------------------------
# c_k = (1 + k mod 2m) * exp(i*pi*3*poly_len*v^6/(p1+p2+3)), v=k/(poly_len-1).
# An extreme nonlinear phase (v^6) crushes the winding to the outer
# coefficients, so the roots stack into concentric rings pierced by radial
# spokes — a bullseye mandala. The +1 keeps every magnitude >=1 (like
# p11b2_v1) so no interior/leading zero throws a spurious far root.
FABLE_35 = f"""\
# fable-35: mandala — (1 + k mod 2m) x exp(i*A*v^6), A = n*(1.5+angle(p1)+0.4 angle(p2)).
# PURELY-imaginary winding: |exp| = 1 always, so magnitude is entirely the
# bounded sawtooth and no leading-coefficient collapse can throw a far root.
fill(poly_len, (2 * {_M13}))
push_range(0, poly_len, 1)
rem(pop, pop)
linear(1, 1)
poly = pop
push_range(0, poly_len, 1)
linear((1.0 / (poly_len - 1)), 0)
dup
multiply(pop, pop)
dup
dup
multiply(pop, pop)
multiply(pop, pop)
linear((1i * (poly_len * ((1.5 + angle(p1)) + (0.4 * angle(p2))))), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_35(p1, p2, n=N_PARITY):
    m = _m13(p1, p2)
    k = np.arange(n, dtype=np.float64)
    sf = (k % (2 * m)) + 1
    v = k / (n - 1)
    coef = 1j * (n * (1.5 + np.angle(p1) + 0.4 * np.angle(p2)))
    return sf * np.exp(coef * (v ** 6))


# --- fable-36: triangle petals -------------------------------------------------
# c_k = (m - |k mod 2m - m|) * exp(i*pi*k/((poly_len-1)*(m+3+p1+p2))). A TRIANGLE
# wave magnitude (ramp up then down each period) instead of the sawtooth's hard
# reset: the petals are symmetric lobes rather than hooked teeth.
FABLE_36 = f"""\
# fable-36: triangle petals — magnitude m-|k mod 2m - m| x param winding
fill(poly_len, (2 * {_M13}))
push_range(0, poly_len, 1)
rem(pop, pop)
linear(1, (0 - {_M13}))
mod(pop)
linear((0 - 1), {_M13})
poly = pop
push_range(0, poly_len, 1)
linear(((1i * pi * poly_len) / ((poly_len - 1) * (({_M13} + 3) + (p1 + p2)))), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_36(p1, p2, n=N_PARITY):
    m = _m13(p1, p2)
    k = np.arange(n, dtype=np.float64)
    tri = m - np.abs((k % (2 * m)) - m)
    coef = 1j * np.pi * n / ((n - 1) * (m + 3 + p1 + p2))
    return tri * np.exp(coef * k)


# --- fable-37: beat petals -----------------------------------------------------
# c_k = (k mod 2m)*(k mod (m+5)) * exp(i*pi*k/((poly_len-1)*(m+3+p1+p2))). TWO
# sawtooths of coprime-ish periods multiplied: their beat carves a longer-period
# lattice of petals-within-petals.
FABLE_37 = f"""\
# fable-37: beat petals — (k mod 2m)(k mod (m+5)) x param winding
fill(poly_len, (2 * {_M13}))
push_range(0, poly_len, 1)
rem(pop, pop)
poly = pop
fill(poly_len, ({_M13} + 5))
push_range(0, poly_len, 1)
rem(pop, pop)
poly = multiply(poly, pop)
push_range(0, poly_len, 1)
linear(((1i * pi * poly_len) / ((poly_len - 1) * (({_M13} + 3) + (p1 + p2)))), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_37(p1, p2, n=N_PARITY):
    m = _m13(p1, p2)
    k = np.arange(n, dtype=np.float64)
    sf = (k % (2 * m)) * (k % (m + 5))
    coef = 1j * np.pi * n / ((n - 1) * (m + 3 + p1 + p2))
    return sf * np.exp(coef * k)


# --- fable-38: petal spiral (sawtooth on a log spiral) ------------------------
# c_k = (k mod 2m) * (0.93 e^{i angle(p1)})^k. The sawtooth petals ride an
# inward geometric spiral instead of a circle, so the whole scalloped ring
# coils toward the center — petals on a nautilus.
FABLE_38 = f"""\
# fable-38: petal spiral — sawtooth (k mod 2m) x (0.93 e^{{i angle(p1)}})^k
fill(poly_len, (2 * {_M13}))
push_range(0, poly_len, 1)
rem(pop, pop)
poly = pop
push_range(0, poly_len, 1)
linear(((log(0.93)) + (1i * angle(p1))), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_38(p1, p2, n=N_PARITY):
    m = _m13(p1, p2)
    k = np.arange(n, dtype=np.float64)
    sf = k % (2 * m)
    return sf * np.exp(k * (np.log(0.93) + 1j * np.angle(p1)))


# --- fable-39: cubic pinwheel --------------------------------------------------
# c_k = (k mod 2m) * exp(i*pi*poly_len*v^3/(m+3+p1+p2)), v=k/(poly_len-1). A
# CUBIC phase winds even harder than fable-34's quadratic: the spiral arms
# curl into a many-bladed pinwheel that tightens toward the rim.
FABLE_39 = f"""\
# fable-39: cubic pinwheel — sawtooth x cubic winding exp(i pi n v^3/denom)
fill(poly_len, (2 * {_M13}))
push_range(0, poly_len, 1)
rem(pop, pop)
poly = pop
push_range(0, poly_len, 1)
linear((1.0 / (poly_len - 1)), 0)
dup
dup
multiply(pop, pop)
multiply(pop, pop)
linear(((1i * pi * poly_len) / (({_M13} + 3) + (p1 + p2))), 0)
exp(pop)
poly = multiply(poly, pop)
emit
"""


def ref_fable_39(p1, p2, n=N_PARITY):
    m = _m13(p1, p2)
    k = np.arange(n, dtype=np.float64)
    sf = k % (2 * m)
    v = k / (n - 1)
    coef = 1j * np.pi * n / (m + 3 + p1 + p2)
    return sf * np.exp(coef * (v ** 3))


# =============================================================================
# Sparse pokes: instead of a dense formula for every coefficient, hand-place a
# few terms and leave the rest zero. Sparse (lacunary) polynomials have roots
# that are NOT near the unit circle — trinomials z^a + c z^b + d fan into two
# interleaved rosettes of a-b and b arcs. The base is all-zero, so poking a
# handful of slots (poly[i] = f(p1,p2)) is the whole program.
# =============================================================================

# --- fable-40: trinomial rosette ----------------------------------------------
# z^(poly_len-1) + (12i*p1 - 8)*z^m + 4*p2, m = 0.38*(poly_len-1). Three terms:
# the roots split into (poly_len-1-m) outer arcs and m inner arcs, a two-tier
# flower that rotates and resizes with the middle coefficient.
FABLE_40 = """\
# fable-40: trinomial rosette — z^(n-1) + (12i*p1-8) z^m + 4 p2, m=floor(0.38(n-1))
fill(poly_len, 0)
poly = pop
poly[0] = (4 * p2)
poly[(floor(0.38 * (poly_len - 1)))] = ((12i * p1) - 8)
poly[(poly_len - 1)] = 1
emit
"""


def ref_fable_40(p1, p2, n=N_PARITY):
    c = np.zeros(n, dtype=np.complex128)
    c[0] = 4 * p2
    c[int(np.floor(0.38 * (n - 1)))] = 12j * p1 - 8
    c[n - 1] = 1
    return c


# --- fable-41: pentanomial chord ----------------------------------------------
# Five hand-placed terms (poly[k] = f(p1,p2)), rest zero — a sparse "chord" of
# powers. Each term contributes its own rosette of roots; the five interleave
# into a lattice that morphs as the coefficients slide (the cf[10]=100i*t1-100
# idea, five notes at once).
FABLE_41 = """\
# fable-41: pentanomial chord — 5 poked slots, rest zero
fill(poly_len, 0)
poly = pop
poly[0] = (2 + (3 * p2))
poly[5] = ((6i * p1) - 3)
poly[12] = ((9 * p1) * p2)
poly[22] = ((0 - 5i) * p2)
poly[(poly_len - 1)] = (2 + p1)
emit
"""


def ref_fable_41(p1, p2, n=N_PARITY):
    c = np.zeros(n, dtype=np.complex128)
    c[0] = 2 + 3 * p2
    c[5] = 6j * p1 - 3
    c[12] = 9 * p1 * p2
    c[22] = -5j * p2
    c[n - 1] = 2 + p1
    return c


# --- fable-42: quadrinomial tiers ---------------------------------------------
# z^(poly_len-1) + a*z^(2(n-1)/3) + b*z^((n-1)/3) + c, exponents at even thirds.
# Four widely-spaced terms fan the roots into three nested tiers of arcs — a
# layered flower whose petals rotate independently with a and b.
FABLE_42 = """\
# fable-42: quadrinomial tiers — z^(n-1) + a z^(2(n-1)/3) + b z^((n-1)/3) + c
fill(poly_len, 0)
poly = pop
poly[0] = (3 * p2)
poly[(floor((poly_len - 1) / 3))] = ((8i * p1) - 4)
poly[(floor((2 * (poly_len - 1)) / 3))] = ((7 * p2) - 3i)
poly[(poly_len - 1)] = 1
emit
"""


def ref_fable_42(p1, p2, n=N_PARITY):
    c = np.zeros(n, dtype=np.complex128)
    c[0] = 3 * p2
    c[int(np.floor((n - 1) / 3))] = 8j * p1 - 4
    c[int(np.floor(2 * (n - 1) / 3))] = 7 * p2 - 3j
    c[n - 1] = 1
    return c


def _rev_blend(c, alpha):
    """VM's rev(poly, alpha): andy blend alpha*c[k] + (1-alpha)*c[n-1-k].
    alpha=1 keeps the original, alpha=0 is a full reversal (reciprocal roots)."""
    return alpha * c + (1 - alpha) * c[::-1]


# --- fable-43: clustered rosette (a local index + a triplet of pokes) ---------
# Poke a TRIPLET of neighbouring slots around a computed index r1 (via a local
# variable), a couple more near the top, then a faint reciprocal-root blend
# rev(poly, 1 - |p1|^2/100). The cluster makes each rosette petal split into a
# little fork; the rev whisper bends the whole figure toward self-inversive.
FABLE_43 = """\
# fable-43: clustered rosette — triplet pokes at r1-1,r1,r1+1 + rev blend
fill(poly_len, 0)
poly = pop
poly[0] = (0.4 * p2)
r1 = (floor(0.38 * (poly_len - 1)))
poly[r1 - 1] = ((12i * p2) + 8)
poly[r1] = ((12i * p1) - 8)
poly[r1 + 1] = ((12 * p2) + 8i)
poly[(poly_len - 2)] = ((1 * p1) + (100i * p2))
poly[(poly_len - 1)] = 1i
poly = rev(poly, (1 - ((abs(p1) * abs(p1)) * 0.01)))
emit
"""


def ref_fable_43(p1, p2, n=N_PARITY):
    c = np.zeros(n, dtype=np.complex128)
    c[0] = 0.4 * p2
    r1 = int(np.floor(0.38 * (n - 1)))
    c[r1 - 1] = 12j * p2 + 8
    c[r1] = 12j * p1 - 8
    c[r1 + 1] = 12 * p2 + 8j
    c[n - 2] = p1 + 100j * p2
    c[n - 1] = 1j
    return _rev_blend(c, 1 - (abs(p1) * abs(p1)) * 0.01)


# --- fable-44: twin clusters ---------------------------------------------------
# TWO triplet clusters at r1 = 0.25(n-1) and r2 = 0.7(n-1), each a little fork
# of param-dependent terms, plus anchored ends. Two rosette groups at different
# radii interleave — a double flower whose two tiers turn independently.
FABLE_44 = """\
# fable-44: twin clusters — triplets at r1=0.25(n-1) and r2=0.7(n-1)
fill(poly_len, 0)
poly = pop
poly[0] = (2 * p2)
r1 = (floor(0.25 * (poly_len - 1)))
r2 = (floor(0.7 * (poly_len - 1)))
poly[r1 - 1] = ((5i * p1) + 3)
poly[r1] = ((10 * p1) * p2)
poly[r1 + 1] = ((5 * p2) - 3i)
poly[r2 - 1] = ((7i * p2) - 2)
poly[r2] = ((8 * p1) + 4i)
poly[r2 + 1] = ((6i * p1) + 2)
poly[(poly_len - 1)] = (2 + p1)
emit
"""


def ref_fable_44(p1, p2, n=N_PARITY):
    c = np.zeros(n, dtype=np.complex128)
    c[0] = 2 * p2
    r1 = int(np.floor(0.25 * (n - 1)))
    r2 = int(np.floor(0.7 * (n - 1)))
    c[r1 - 1] = 5j * p1 + 3
    c[r1] = 10 * p1 * p2
    c[r1 + 1] = 5 * p2 - 3j
    c[r2 - 1] = 7j * p2 - 2
    c[r2] = 8 * p1 + 4j
    c[r2 + 1] = 6j * p1 + 2
    c[n - 1] = 2 + p1
    return c


FABLES = [
    ("fable-1", FABLE_1, ref_fable_1),
    ("fable-2", FABLE_2, ref_fable_2),
    ("fable-3", FABLE_3, ref_fable_3),
    ("fable-4", FABLE_4, ref_fable_4),
    ("fable-5", FABLE_5, ref_fable_5),
    ("fable-6", FABLE_6, ref_fable_6),
    ("fable-7", FABLE_7, ref_fable_7),
    ("fable-8", FABLE_8, ref_fable_8),
    ("fable-9", FABLE_9, ref_fable_9),
    ("fable-10", FABLE_10, ref_fable_10),
    ("fable-11", FABLE_11, ref_fable_11),
    ("fable-12", FABLE_12, ref_fable_12),
    ("fable-13", FABLE_13, ref_fable_13),
    ("fable-14", FABLE_14, ref_fable_14),
    ("fable-15", FABLE_15, ref_fable_15),
    ("fable-16", FABLE_16, ref_fable_16),
    ("fable-17", FABLE_17, ref_fable_17),
    ("fable-18", FABLE_18, ref_fable_18),
    ("fable-19", FABLE_19, ref_fable_19),
    ("fable-20", FABLE_20, ref_fable_20),
    ("fable-21", FABLE_21, ref_fable_21),
    ("fable-22", FABLE_22, ref_fable_22),
    ("fable-23", FABLE_23, ref_fable_23),
    ("fable-24", FABLE_24, ref_fable_24),
    ("fable-25", FABLE_25, ref_fable_25),
    ("fable-26", FABLE_26, ref_fable_26),
    ("fable-27", FABLE_27, ref_fable_27),
    ("fable-28", FABLE_28, ref_fable_28),
    ("fable-29", FABLE_29, ref_fable_29),
    ("fable-30", FABLE_30, ref_fable_30),
    ("fable-31", FABLE_31, ref_fable_31),
    ("fable-32", FABLE_32, ref_fable_32),
    ("fable-33", FABLE_33, ref_fable_33),
    ("fable-34", FABLE_34, ref_fable_34),
    ("fable-35", FABLE_35, ref_fable_35),
    ("fable-36", FABLE_36, ref_fable_36),
    ("fable-37", FABLE_37, ref_fable_37),
    ("fable-38", FABLE_38, ref_fable_38),
    ("fable-39", FABLE_39, ref_fable_39),
    ("fable-40", FABLE_40, ref_fable_40),
    ("fable-41", FABLE_41, ref_fable_41),
    ("fable-42", FABLE_42, ref_fable_42),
    ("fable-43", FABLE_43, ref_fable_43),
    ("fable-44", FABLE_44, ref_fable_44),
]


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--only", nargs="*", help="just these names")
    ap.add_argument("--dry-run", action="store_true", help="no S3 upload")
    ap.add_argument("--force", action="store_true", help="overwrite existing")
    ap.add_argument("--show", action="store_true", help="print sources")
    args = ap.parse_args()

    known = {n for n, _, _ in FABLES}
    if args.only:
        unknown = [n for n in args.only if n not in known]
        if unknown:
            sys.exit(f"unknown fable name(s): {', '.join(unknown)}; "
                     f"choose from: {', '.join(sorted(known))}")
    picks = [(n, s, r) for n, s, r in FABLES if not args.only or n in args.only]
    if not picks:
        sys.exit("no fables selected")
    if args.show:
        for name, source, _ in picks:
            print(f"===== {name} =====\n{source}")
        return

    existing = set() if args.dry_run else list_existing_keys()
    failures = []
    for name, source, ref in picks:
        try:
            compiled = compile_coeff_program_source(source)
        except Exception as exc:
            failures.append((name, f"compile: {exc}"))
            print(f"FAIL {name}: compile: {exc}")
            continue
        stats, err = parity_check(compiled, ref, N_PARITY)
        if err:
            failures.append((name, err))
            print(f"FAIL {name}: {err}")
            continue
        line = (f"OK   {name}: tokens={compiled['token_count']} "
                f"worst={stats['worst']:.2e} points={stats['points']} "
                f"chaotic={stats['chaotic']:.0%}")
        if args.dry_run:
            print(line + " (dry-run)")
            continue
        program = build_payload(name, source)
        key, status = upload(program, existing, force=args.force)
        print(line + f" -> {key} ({status})")
    if failures:
        sys.exit(f"{len(failures)} failed: {', '.join(n for n, _ in failures)}")
    print(f"all {len(picks)} passed parity (rel tol {PARITY_REL_TOL})")


if __name__ == "__main__":
    main()
