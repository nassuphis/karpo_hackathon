# Recreating `giga_2892`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2892/giga_2892.png` from
its saved state. The reference tree is read-only; no new run may use
the `giga_2892` (or its recorded `giga_2869` data-stem) names.

## 1. Main conclusions

`giga_2892` is **poly_chess3** — the COUPLED chessboard mixer, sitting
between giga_2891 (chess2: independent draws, uniform mix) and the
2898 family (chess5: layouts + bimodal mixes).

```text
poly:  poly_chess3 (N=8, degree 64), xfrm none, zfrm none, solve=solve
run:   roots=2e9, res=50000, samples=100000, procs=14, alpha=0.001
view:  (-6.555-6.552j) -> (6.533+6.536j)
```

Per row (polys/chess.py, TWO unseeded draws):

```python
t0 = rand()                            # ONE draw for BOTH copies
d1 = 0.5*exp(2j*pi*t0)*(1+1j)          # phase displacement
d2 = 0.5*(t0-0.5)*(1+1j)               # real shift — same t0
cf1 = np.poly(lattice + parity*d1)
cf2 = np.poly(lattice + parity*d2)
cf  = cf2*a + cf1*(1-a),  a = bimodal_skewed(0.85)
```

1. **The coupling is the signature**: chess2 drew the phase angle and
   the real shift independently; chess3 derives both from one t0, so
   the two black-square copies breathe in lockstep — the phase copy's
   angle always agrees with the real copy's offset. One fewer degree
   of freedom than 2891, one more correlation in the paint.
2. **a = bimodal_skewed(0.85)** — exponent 1/(1-0.85) =
   6.666666666666666, the heaviest bimodal in the family so far: rows
   are almost always a near-pure copy (probe weights: 0.0000, 0.0003,
   0.9955, 0.0000, 0.0018), with rare genuine mixes carrying the halo.
   Spelled exp(E*log(.)) with floor gates (2898-family doctrine).
3. **White squares are per-row exact anchors** (parity-0 points are
   common roots of every copy-combination); measured 8.4e-4 np.roots
   conditioning dust.
4. **RNG remap**: two fresh frac-cascade uniforms of (t1, t2).
   No solver in-program; two np.poly expansions per row.

## 2. Trap log (this wave)

Nothing new fired. The giga_2891 nested-pop miscompute is designed
out (branches weighted in statement position, mixed with
add(poly, pop)) and pinned out of the source by the suite.

## 3. Polypaint Coeff Program

`giga_2892.coeff-program.json` (41 tokens, stack_max 6; generator
`scripts/gen_giga_2892_coeff_program.py`; suite
`tests/test_giga_2892_coeff_program.py`, predeploy-gated, plain
no-LAPACK build). Per branch: park the bimodal uniform, build the
weight vector with a statement-position tos scan, park it; parity
mask parked, displacement scan (delta inlines the SHARED t0 cascade),
+ lattice, expand_roots, multiply(pop).

Verification (measured): coefficient relative parity 5.9e-11, leading
slot exactly 1, whites <= 8.4e-4, root multiset vs oracle median
5.1e-10 / max 1.8e-3.

Uploaded 2026-07-18: id `giga-2892`.
