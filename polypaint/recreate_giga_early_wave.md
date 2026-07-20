# Recreating `giga_1` .. `giga_30` (the early wave)

## 0. Objective and safety boundary

Recreate the 30 earliest reference artworks from their saved state in
`/Users/nicknassuphis/pyroots/giga_N/`. The reference tree is
read-only; no new run may use the `giga_*` stems.

Uploaded 2026-07-20, ids `giga-1` .. `giga-30`, none overwriting.

## 1. The two eras and their OPPOSITE coefficient conventions

The load-bearing discovery of this wave. `poly_giga_29` in
`polys/giga.py` is textually identical to `giga_6.R`'s ppol, yet
`giga_29.png` (Jan 2025) differs from `giga_6.png` (Dec 2024) — the
same coefficient vector fed to two solvers with opposite conventions:

- **R era (Dec 2024 pngs: giga_1..26).** `ppol(t1,t2)` vectors fed to
  `base::polyroot`, which reads ASCENDING (`cf[1]` = constant).
  Scripts ending `rev(cf)` (giga_1, giga_2) hand polyroot the reversed
  vector, so their cf layout is numpy-descending already; every other
  script is ascending and the app program ends `poly = rev(poly)`.
- **python era (Jan 2025 pngs: giga_27..30).** `polys/giga.py` defs
  solved with `np.roots` — numpy-DESCENDING direct, no reverse.

giga_24..26 have no surviving R scripts (only Dec-era pngs); their
`polys/giga.py` defs are index-shifted transliterations in the same
style as the surviving scripts (verified against giga_22/23), so they
take the R-era ascending convention.

## 2. Params

Every artwork: `param_space(len, x_01, x_circle)` COMPOSES the
constructors over ONE sequence (`reduce`), so BOTH t1 and t2 are
unit-circle points `e^(2 pi i t)` — the legacy `uc`. Monomials
`t1^a t2^b` fold to `exp(2 pi i (a t1 + b t2))`.

Exceptions:
- **giga_19, giga_20**: `param_space(4000, x_01)` — t1, t2 are the
  RAW [0,1] reals. No circle map.
- **giga_21..23** save `param_space(4000,x_01,,x_circle)` — the
  extra-comma typo cannot evaluate in R (`list(...)` with a missing
  argument errors), so the actual runs used `x_01,x_circle` like
  every sibling. Assumed and documented.
- **giga_17** (python era): xfrm `uc,sum_prod` — poly_727 receives
  v1 = z1+z2, v2 = z1 z2.

## 3. Per-giga notes

| giga | tier | notes |
|---|---|---|
| 1, 2 | T1 | rev(cf) scripts -> descending direct |
| 3-6, 12-16, 22-26 | T1 | ascending -> `poly = rev(poly)`; giga_5/24 have mixed 100i/100 coefficient patterns; giga_25 is the byte-identical giga_22 duplicate (sha-verified png), kept as its own id |
| 28-30 | T1 | python era, descending direct; giga_30 uses `abs(t1+t2-2.5(1+i))` |
| 7, 9 | T2 | deg-29/19 linspace(Re t1..Re t2 / Im..Im) trig/power vectors; giga_9's `**9` blows the 64-token scalar cap -> elementwise square-and-multiply on the vector (p2=p*p, p4, p8, p9=p8*pop) |
| 10 | T2 | 120-term 1-based k-formula, header [re1,im1,re2,im2] parked on the stack; three closed-form slot overrides (cf[30]+=1000i etc.) recomputed inline — no read-back needed |
| 11 | T2 | m = trunc(5|t1+t2|)+1 (the R %%17 is a no-op, arg <= 10); (k-1) mod m via floor; m parked on the stack |
| 8 | T3 | chained cubic polyroot solves feed coefficient slots. R polyroot = cpoly (Jenkins-Traub): the app's `roots_jt` reproduced R's root ORDER exactly on every Rscript probe (including near-magnitude-tie cases where a magnitude sort fails). Test oracle = Rscript-generated fixtures |
| 27 | T3 | np.roots cubics -> `roots_cm` (same LAPACK companion eigensolve, order verified per-slot in the parity test against numpy); local parity via the Accelerate-linked test binary |
| 19, 20 | T4 | deg-89 chains cf[k] = 1i unit(sin(k cf[k-1]) + cos(k t1)) over RAW params. CHAOTIC: measured ~10x error growth per element (complex sin derivatives ~cosh k), so any 1-ulp libm difference decorrelates the tail — in the app AND between two builds of the reference. Pinned: chain head (first 8 elements at 1e-11) + unit-magnitude invariant. The R guard (|v|>1e-10, else t1+t2) is not ported (zero-set is a curve vs 2.5e-4 grid spacing and 1e-10 threshold) |
| 21 | T4 | deg-49 chain with ((k+3)%%10)/((k+1)%%10) multipliers packed as one complex vector literal (real/imag parts) — the floor-based inline form is 70 scalar tokens (cap 64) |
| 18 | T5 | angle-conditional branches. theta = Arg(e^(2 pi i t)) = 2 pi (t - floor(t+0.5)) (exact except the u=0.5 gridline, where only the (theta1+theta2)^2 terms can tell). Strict x>0 gates = 1 - floor((2-x)/2), exact for the bounded args INCLUDING x=0 — the t1=t2 diagonal is on-grid and a division-based sign would NaN there. Two-stage parked header (angles, then scales) |
| 17 | python-era state | uc,sum_prod -> poly_727 (n=9) -> rev + recursive_add EMA (decay 5e-05) -> safe. EMA NOT ported: measured stateless-vs-EMA root moves median 4.7e-5 / p95 1.0e-4 / max 2.4e-4 vs an 8e-5 pixel — sub-pixel dust, and the reference's own EMA state is per-process chunk-order noise under procs=14. Safe knife never fires (sum|cf| in [587, 654]) |

## 4. Trap log (this wave)

1. **`**n` powers expand hard**: `x**9` in a scan element exceeded the
   64-token scalar cap even against a parked header. Vector
   square-and-multiply is the idiom.
2. **cpoly order is not a magnitude sort**: at near-magnitude ties
   R's deflation order broke a sorted-oracle; the app's roots_jt
   matched R exactly — fixtures from Rscript are the right oracle.
3. **Chaotic chains cap parity**: sin(k*prev) recurrences amplify
   1-ulp differences ~10x per element. Chain-head + invariants is the
   honest contract (2864-dust class).
4. **Gates must survive on-grid zeros**: sin(theta1-theta2) = 0 on
   the t1=t2 diagonal; 1-floor((2-x)/2) is exact there, x/|x| NaNs.

## 5. Verification summary (measured at authoring)

- T1 (19 programs): coeff rel parity < 1e-12 at 5 probes each, root
  multisets < 1e-8.
- T2 (4): same bounds; giga_10/11 include the parked-header paths.
- T3: giga_8 vs Rscript fixtures < 1e-6 rel (JT-vs-cpoly ULP class);
  giga_27 vs numpy < 1e-6 rel on the Accelerate LAPACK binary.
- T4/T5: giga_18 < 1e-10 rel incl. the on-diagonal probe; chains
  match heads at 1e-11 with unit-magnitude tails.

Suites: tests/test_giga_early_t1_coeff_programs.py (+_t2/_t3/_t45),
tests/test_giga_17_coeff_program.py — all predeploy-gated.
