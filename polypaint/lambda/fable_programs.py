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
]


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--only", nargs="*", help="just these names")
    ap.add_argument("--dry-run", action="store_true", help="no S3 upload")
    ap.add_argument("--force", action="store_true", help="overwrite existing")
    ap.add_argument("--show", action="store_true", help="print sources")
    args = ap.parse_args()

    picks = [(n, s, r) for n, s, r in FABLES if not args.only or n in args.only]
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
