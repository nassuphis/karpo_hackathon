# Recreating `giga_2921`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2921/giga_2921.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2921` (or its `test16` data-stem) names.

## 1. Main conclusions

`giga_2921` is the series' first **physics fossil**: the artwork is not
a root cloud of the formula at all — it is the occupation measure of a
240-particle repulsion simulation that the formula merely *feeds*.

```text
poly:  poly_giga_1, degree 25, xfrm=uc, zfrm=recursive_addvr,
       solve=none, bins=10, decay=0.999, gravity=10, speed=2, phi=0.125
run:   roots=1e9, res=50000, samples=100000, procs=14, alpha=0.1
view:  (-5-5j) -> (5+5j)
```

Per row (zfrm.py:859, config values; damping=0.9 and stepsize=0.001 are
code defaults — the config's `speed`/`phi` are unused by this zfrm):

```python
T1, T2 = exp(2j*pi*t1), exp(2j*pi*t2)            # xfrm 'uc'
rts    = np.roots(poly_giga_1(T1, T2))           # 24 roots
# rows 0..9: store rts in bin i; velocities 10*exp(2j*pi*rand(24))
d      = sum(|root_matrix - rts[:,None]|, axis=0)
ci     = argmin(d)                               # closest of 10 bins
root_matrix[:,ci] = 0.999*root_matrix[:,ci] + 0.001*rts   # EMA pull
# all 240 stored points: inverse-square REPULSION, damped velocities
vel    = 0.9*vel + 10*forces/240 ; points += 0.001*vel
paint(root_matrix[:,ci])                         # solve='none'
```

1. **The rings are the repulsion equilibrium, not root shells.** The
   raw `poly_giga_1` torus roots live at radius 0.85–1.2 (99% < 1.85);
   the artwork's rings sit at 1.05 / 2.57 / 4.36. Re-simulating the
   true mechanics faithfully (150k rows) reproduces the ring *style* —
   concentric shells plus ballistic escape wisps — but at different
   radii (0.45/1.2/2.0), and scores **2D corr ≈ 0.0** against the
   artwork: the radii depend on run length, process forking (14
   independent chains), and the **unseeded** `np.random.rand` initial
   velocities. Pure run fossils, unknowable from the mechanism.
2. **The migration keeps the mechanism, fits the fossils.** Angles come
   from the exact formula roots (`roots_cm`, the reference's np.roots
   zgeev order); the radial structure is reconstructed from the artwork
   itself — measured rings (mass/mean/sd): 9.3%/1.048/0.128,
   35.7%/2.569/0.198, 54.0%/4.361/0.169, wisps 1.5% (dropped) —
   assigned per root by the frac-cascade PRNG (the 2883/2907 remap
   pattern) with a ±0.15 rad angular smear for the EMA blur. Measured
   against the reference small PNG: **radial corr 0.896, 2D log-blur
   corr 0.876** (the residual is beading — individual particle
   residences, RNG fossils by nature).
3. **Convention correction.** This wave empirically re-pinned the
   coefficient layout: the pipeline solver (sweep_cli.c `solveEA`)
   strips leading zeros from the FRONT and Horners from `cr[0]` —
   **slot 0 is the leading coefficient (numpy order) end-to-end**, and
   `roots_cm`/`expand_roots` agree. An earlier note claiming ascending
   (slot n−1 leading) was inverted; the first prototype poked that way
   and produced exactly reciprocal roots (parity 1e13 — the multiset of
   1/r matched to machine precision, which is the tell).

## 2. Reference artifacts

`giga_2921_sml.png` — 1000×1000, three concentric beaded rings, faint
transit filaments; sidecars: paramconstruct=`uc`, polyfun=
`poly_giga_1`, zfrm=`recursive_addvr`, solver=`none`, res=50000,
alpha=0.1, paramcount=1e9.

## 3. Polypaint Coeff Program

`giga_2921.coeff-program.json` (246 tokens, stack_max 8; generator
`scripts/gen_giga_2921_coeff_program.py`; parity suite
`tests/test_giga_2921_coeff_program.py`). Uses the register syntax and
expression-composable transforms shipped this week:

```text
r1 = exp(2πi·t1); r2 = exp(2πi·t2)          # exact xfrm 'uc'
poly = fill(25, 0); 11 pokes                # exact poly_giga_1, np layout
poly = roots_cm(poly, lo, exact)[0:24]      # np.roots order
r3, r4 = frac-cascade scans (ring choice, radial jitter)
r5 = select(lt(r3,.0934), ring1, select(lt(r3,.4501), ring2, ring3))
poly = unit(poly) · r5 · exp(0.3i·(w−½))    # measured fossils + smear
poly = expand_roots(poly)                   # monic; solver recovers points
emit
```

VM-vs-oracle parity: coefficients ≤ 5.1e-7 rel (f32 debug channel
floor), root round-trip ≤ 1.8e-7, monic slot exact. Ring masses pinned
at the measured fossils over a 400-row ensemble.

## 4. What is and is not reproducible

Reproducible (and pinned): formula, unit-circle params, np.roots
order, PRNG cascade, ring mass/radius/width statistics, angular
density. Not reproducible (documented fossils): the beading (which
particle rested where), the wisps (specific escape trajectories), and
any refinement of ring radii beyond the artwork measurement — the
mechanism cannot predict them without the lost RNG state and exact
run schedule. This is the same doctrine as 2877's pairing order and
outflow's division dust, at larger scale: here even the *geometry* is
a fossil, so the artwork itself is the only valid oracle for it.
