# Recreating the `giga_2893` family (2893, 2894, 2895)

## 0. Objective and safety boundary

Recreate three poly_chess4 artworks from their saved states under
`/Users/nicknassuphis/pyroots/giga_289x/`. The reference tree is
read-only; no new run may use the `giga_289x` (or the shared
`giga_2869` data-stem) names.

## 1. Main conclusions

All three are **poly_chess4** — structurally different from chess2/3:
the parity mask now **selects which cells exist**. Only cells with
`(row+col+off) % mod != 0` (the odd half of the board) enter the
polynomial at all, so there are NO white-square anchors — every root
moves. Both copies displace that same cell set by shape-path offsets
coupled through ONE draw t, then a bimodal mix:

```python
to = orad * oshape(t * ospeed)          # outer copy
ti = irad * ishape(t * ispeed + phi)    # inner copy
cf = np.poly(cells + to*(1+1j)) * b + np.poly(cells + ti*(1+1j)) * (1-b)
b  = bimodal_skewed(a)                  # second draw
```

```text
run        N  deg  inner              outer               a     zfrm
giga_2893  8  32   0.5*circle(t)      0.5*circle(t)       0.85  none
giga_2894  8  32   0.3*circle(t+.25)  0.45*opolygon4(t)   0.95  p2_p@1e-18
giga_2895  12 72   0.25*circle(t)     1.0*circle(0.01*t)  0.85  none
```

- **giga_2893 is all defaults — and the defaults make inner == outer
  EXACTLY** (same t, same 0.5 radius, same circle, phi=0), so the
  bimodal mix is a no-op and the run is a single displaced
  half-lattice. The program is one branch, one draw, 12 tokens; the
  reference's b draw is dead code and is not remapped.
- **giga_2894** walks the outer copy around the SQUARE (opolygon with
  oplgn=4: piecewise-linear between the 4th-roots-of-unity vertices)
  against a 0.3-radius inner circle at phi=0.25. a=0.95's exponent is
  1/(1-0.95) = 19.999999999999982 (f64 truth, not 20). The p2_p zfrm
  adds (cf^2+cf+1)*ramp(1..33)*1e-18: the tiny andy is amplified by
  |cf|^2 ~ 1e28 into a measured 1.4e-2 relative shatter.
- **giga_2895** at degree 72 pits an almost-frozen outer circle
  (ospeed=0.01 — the offset barely leaves (1+1i)) against a live
  0.25-radius inner circle. Its coefficients reach ~9.3e45, OVER the
  f32 transport ceiling (the giga_cf10p450 blank-image mechanism), so
  the program rescales by 1e-15 — roots invariant.

RNG remap: one (2893) or two fresh frac-cascade uniforms per run.
No solver in any program; two np.poly expansions per row (one for
2893).

## 2. Trap log (this wave)

Nothing new fired. The square-path shape compiles cleanly with floor
gates and i^e = exp(i*pi/2*e) (e = floor(4t) is exact on [0,1)); the
2891 nested-pop miscompute stays designed out and pinned out of all
three sources.

## 2b. The giga_2895 dust investigation (user-reported)

The user saw "rounding errors in the roots" in the AE64 hires calc
that the AE64 preview does not show. Fully diagnosed (2026-07-18):

1. **Preview vs calc is pure resolution unmasking.** Grid (preview)
   and chunked (pipeline) AE64 produce statistically identical roots
   (measured on 2500 rows: same medians, same failing-row positions —
   the warm-chain structure difference is irrelevant). The dust's p99
   is ~2.8e-2: about ONE preview pixel, but ~90 pixels at res 50000.
   It is present in both renders; only the calc resolves it.
2. **The dust is solver-universal and authentic.** The affected rows
   are the b ~ 0 rows: a whisper of the far outer copy perturbing the
   inner lattice. That whisper times the astronomical root-condition
   of a degree-72 lattice polynomial legitimately throws roots O(1)
   distances (the halo mechanism itself), and stretches coefficients
   across ~45 decades — at which range EVERY f64 coefficient-based
   solver carries a ~1e-2 error floor on the most sensitive roots.
   Verified: AE at 64/128/256/1000 iterations, fresh-seed AE, JT, and
   np.roots (QR) all land ~1e-2..3e-2 apart on those rows; Newton-
   polishing each against the exact PRODUCT FORM moves QR max 1.75e-2
   and AE max 1.95e-2 (medians ~1e-9), and the two polished sets
   agree to exactly zero. The reference pyroots pipeline (np.roots on
   the same f64 coefficients) contains the same dust class — it is
   part of the artwork, not a pipeline defect.
3. **One real AE64-specific artifact**: ~0.5% of rows return NaN
   (Aberth internal collision) and drop out of the paint entirely,
   where JT64/CM64 would paint them. A fresh-seed retry rescues those
   rows (verified 2e-5); a retry-on-NaN hardening in fusedSolveRow is
   the one code change this investigation motivates.

Solver guidance for 2895: AE64 is fine (fastest, no dustier than any
alternative); CM64/JT64 avoid the NaN-dropped rows at higher cost.
No solver choice removes the ~1e-2 floor — it is the price of a
45-decade polynomial, and the reference paid it too.

## 3. Polypaint Coeff Programs

One generator emits all three
(`scripts/gen_giga_2893_family_coeff_programs.py`; 12/61/45 tokens;
suite `tests/test_giga_2893_family_coeff_programs.py`,
predeploy-gated, plain no-LAPACK build). Dual-branch runs use the
2898-family per-branch weight parking; mixes are add(poly, pop).

Verification (measured, pins with headroom): coefficient relative
parity 5.8e-14 / 6.5e-14 / 1.9e-12; root multisets max 1.9e-10 /
8.2e-10 / 5.8e-4 (deg-72 conditioning dust); 2895's rescale pinned as
the transport rescue (scaled fits f32, unscaled would overflow).

Uploaded 2026-07-18: ids `giga-2893`, `giga-2894`, `giga-2895`.
