# Recreating `giga_2897`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2897/giga_2897.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2897` (or its recorded `giga_2869` data-stem) names.

## 1. Main conclusions

`giga_2897` is the **displaced letterform mixer** — and by migration
standards a simple one: no recursion, no cross-row state, no in-program
root trip.

```text
poly:  poly_chess5, rloc=rloc13, degree 37, a=0.75, phi=0.5
       (defaults: rho=0.33, speed=1.0, shape=circle), xfrm/zfrm none,
       solve=solve
run:   roots=2e9, res=50000, samples=100000, procs=14, alpha=0.005
view:  (-33.883-32.786j) -> (28.894+29.990j), rotate 0
```

Per row (polys/chess.py:279):

```python
t = np.random.rand()                    # unseeded — fossil draw 1
d1 = 0.33 * exp(2j*pi*t) * (1+1j)       # diagonal displacement
d2 = 0.33 * exp(2j*pi*(t+0.5)) * (1+1j) # the ANTIPODAL phase (= -d1)
scfs = np.poly(S + d1)                  # 37 source cells of rloc13
tcfs = np.poly(T + d2)                  # 37 target cells
a = bimodal_skewed(0.75)                # unseeded — fossil draw 2
cf = tcfs*a + scfs*(1-a)                # convex mix, painted by solve
```

1. **The lattice is a letterform.** `rloc13` (pyroots/poly.py:19578)
   parses through `layout2coord` to a 14x25 centered grid — a leading
   `_` row, two blank rows, and one trailing-spaces row all shift the
   centering, so the figure is asymmetric: x in [-12, 8], S cells
   y in [-6.5, 2.5], T cells y in [-5.5, 3.5]. 37 S + 37 T = the
   recorded degree 37. The saved view center (-2.5, -1.4) is exactly
   the lattice center — the artwork is the frame-and-pillars figure
   drawn twice.
2. **The two copies breathe in antiphase.** phi=0.5 with speed=1 makes
   the target displacement the exact negation of the source one: both
   letterforms slide along the (1+1i) diagonal on a rho=0.33 circle,
   always in opposite directions.
3. **The mix weight is bimodal.** `bimodal_skewed(0.75)` pushes u
   through (2u)^4/2 (or its mirror), so most rows sit near a pure
   copy — crisp letterform paint — with a heavy-ish tail of genuine
   mixes whose ill-conditioned outer roots throw the halo that fills
   the +/-30 view (cloud max |z| ~90; the view crops it).
4. **RNG remap** (2883/2907/2920 pattern): the sweep inputs are ignored
   by the reference, so two frac-cascade uniforms of (t1, t2) stand in
   for the draws: U1 -> circle parameter, U2 -> bimodal draw.
5. **No solver in the program.** solve=solve means the artwork is
   np.roots of the mix — precisely what the pipeline does with emitted
   coefficients. Coefficient scale ~1.7e34 max stays inside f32
   transport (3.4e38): no rescale needed, roots unchanged either way.

## 2. Trap log (this wave)

- **tos[k] in expression-position scans reads POLY, not the parked
  vector** (verified in the VM with probe programs): the compiler
  parks the LEFT operand as a stack temp before evaluating the right,
  so `add(poly, scan(.., tos[0], ..))` sees poly's own elements
  through tos. Every tos-referencing scan must be STATEMENT-position,
  run while the uniforms pair is top-of-stack. `multiply(poly, pop)` /
  `add(poly, pop)` are safe — pop reads the true pre-statement stack
  (also verified).
- Vector-times-vector `multiply` is strictly elementwise with exact
  length match (a mismatched toy fails with "vector binary length
  mismatch") — weight vectors are scanned at length 38 to match the
  expanded branch.
- The bimodal weight needs no `^` operator: floor(2u) gates the two
  branches and the 4th powers are repeated products — pure-python
  mirror grouping gives f64-exact parity (2864 lesson).
- ASCII-art lattices don't survive hand transcription: the layout is
  embedded as coordinate literals computed once from the reference
  through a verbatim layout2coord replica (row-major order included,
  since np.poly convolves in element order).

## 3. Polypaint Coeff Program

`giga_2897.coeff-program.json` (41 tokens, stack_max 6; generator
`scripts/gen_giga_2897_coeff_program.py`; suite
`tests/test_giga_2897_coeff_program.py`, predeploy-gated, plain
no-LAPACK build). Per branch:

```text
park uniforms; scan weight vector (38); park
park uniforms; scan displacement vector (37, statement-position tos)
+ lattice literal; expand_roots; multiply(pop)      -> weighted copy
branches mixed with add(poly, pop); emit             (monic, degree 37)
```

Verification (measured, pins 10-100x above): coefficient relative
parity <= 7e-16 (f64 end-to-end, leading slot exactly 1), per-row root
multiset vs oracle median <= 4e-8, halo tail <= 8.4e-4, cloud bounded.
The deployed compiler accepts the dialect (statement transforms, scan
side-tables, no registers).

Uploaded 2026-07-17: id `giga-2897`.
