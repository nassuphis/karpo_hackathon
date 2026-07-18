# Recreating `giga_2891`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2891/giga_2891.png` from
its saved state. The reference tree is read-only; no new run may use
the `giga_2891` (or its recorded `giga_2869` data-stem) names.

## 1. Main conclusions

`giga_2891` is **poly_chess2 plain** — the memoryless chessboard
mixer: giga_2920's per-row core without the recursive_add3 EMA chain
and without any root trip.

```text
poly:  poly_chess2 (N=8, degree 64), xfrm none, zfrm none, solve=solve
run:   roots=2e9, res=50000, samples=100000, procs=14, alpha=0.001
view:  (-7.897-7.894j) -> (7.740+7.743j)   (the full lattice view)
```

Per row (polys/chess.py:48, three unseeded draws):

```python
d1 = 0.5*exp(2j*pi*u)*(1+1j)          # rotating phase displacement
d2 = 0.5*(v-0.5)*(1+1j)               # real diagonal shift
cf1 = np.poly(lattice + parity*d1)    # black squares displaced
cf2 = np.poly(lattice + parity*d2)
cf  = cf2*a + cf1*(1-a)               # PLAIN-uniform convex mix
```

1. **White squares are per-row EXACT anchors** — parity-0 points are
   common roots of both copies and every convex mix, and with no EMA
   pairing to drag partners (2920's distributional smearing), each row
   nails all 32 whites to np.roots conditioning dust (measured
   2.9e-4 at the deg-64/1e10 coefficient scale).
2. **The halo** is the usual ill-conditioned mix-root dust between the
   displaced black-square copies.
3. **RNG remap**: the three unseeded draws become three fresh
   frac-cascade uniforms of (t1, t2); mix orientation preserved
   (cf2 gets a, cf1 gets 1-a).
4. **No solver in the program** — two np.poly expansions per row, so
   the cheap sibling of 2920 (no eigensolve at all).

## 2. Trap log (this wave) — a NEW miscompute trap

The tempting mix one-liner

```
poly = add(poly, multiply(pop, scan(65, 0, 1-(a), prev)))
```

**silently miscomputes**: `add` parks its left operand (cf2·a) as a
stack temp before evaluating the right argument, and the nested `pop`
grabs that temp instead of the parked cf1 — yielding
`cf1 + cf2·a·(1-a)` instead of the convex mix (VM-verified with
literal vectors: [1,2],[10,20],a=0.25 → [2.875, 5.75], i.e.
cf1 + cf2·a·(1-a); intended [3.25, 6.5]). Same family as the 2897
tos-in-expression-position trap: expression-position stack reads see
the statement's own temp. Tells: the leading slot becomes
1 + a·(1-a) ≠ 1, and coefficient parity fails while white anchors
still pass (ANY combination of the copies keeps the whites — which is
how the identical line shipped inside giga_2920/giga_2920_lite
without their root-tier tests noticing; see §4).

Safe idiom (used here): weight each branch in STATEMENT position
before parking, then mix with the proven `add(poly, pop)`.

## 3. Polypaint Coeff Program

`giga_2891.coeff-program.json` (33 tokens, stack_max 4; generator
`scripts/gen_giga_2891_coeff_program.py`; suite
`tests/test_giga_2891_coeff_program.py`, predeploy-gated, plain
no-LAPACK build):

```text
parity mask parked; scan builds parity*d_phase; + lattice;
expand_roots; x(1-a)  -> park
same for d_real;      x a
add(poly, pop); emit                        (monic, degree 64)
```

Verification (measured): coefficient relative parity 5.1e-11 (the
deg-64 expand_roots accumulation floor), leading slot exactly 1,
white anchors <= 2.9e-4 per row, root multiset vs oracle median
1.4e-9 / max 3.0e-4.

## 4. Fallout: giga_2920 and giga_2920_lite carry the broken line

Both shipped 2920 programs use the miscomputing one-liner for their
convex mixes, so they actually compute `cf1 + cf2·a·(1-a)` — a valid
chessboard-family combination (whites anchor regardless, halo
similar, which is why the loose distributional tiers passed), but not
the documented reference mix. Repairing them changes their emitted
polynomials and hence their artworks — left untouched pending an
explicit decision.

Uploaded 2026-07-18: id `giga-2891`.
