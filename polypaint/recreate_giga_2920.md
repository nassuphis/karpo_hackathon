# Recreating `giga_2920`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2920/giga_2920.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2920` (or its `test16` data-stem) names.

## 1. Main conclusions

`giga_2920` is the **chessboard mixer**: the formula ignores its sweep
inputs entirely — three unseeded `np.random` draws per row do all the
work — and a one-row-deep EMA chain smears consecutive rows together.

```text
poly:  poly_chess2 (N=8, degree 64), xfrm=uc (irrelevant — inputs unused),
       zfrm=recursive_add3, decay=0.1, solve=solve
run:   roots=5e8, res=50000, samples=100000, procs=14, alpha=1e-05
view:  (-9.150-9.171j) -> (9.094+9.073j), rotate 90
```

Per row (chess.py:48, zfrm.py:1005):

```python
u, v, a = np.random.rand(3)                     # unseeded — fossils
d1 = 0.5*exp(2j*pi*u)*(1+1j)                    # rotating phase offset
d2 = 0.5*(v-0.5)*(1+1j)                         # real diagonal shift
cf1 = np.poly(lattice + parity*d1)              # black squares displaced
cf2 = np.poly(lattice + parity*d2)
cf  = cf2*a + cf1*(1-a)                         # random convex mix
# recursive_add3: EMA in sorted-root space, one-row memory
painted = 0.9*sort_abs(roots(cf)) + 0.1*sort_abs(roots(prev_output))
```

1. **White squares are the sharp anchors** — they are *exact common
   roots* of both copies (parity=0 squares are never displaced) and
   hence of every convex mix. The EMA's sorted pairing can drag a
   white's partner from across the lattice (0.1 × lattice diameter ≈
   0.99), so per-row anchoring is NOT invariant — the crisp lattice in
   the artwork is distributional (pooled probe cloud: median anchor
   distance 0.000, worst 0.316).
2. **The halo is noise paint.** The convex mix of two `np.poly`
   expansions has ill-conditioned outer roots (condition 1e8–1e10 at
   coefficient scale 1e10): accumulation-order dust — the VM's
   `expand_roots` vs `np.poly`, or numpy against itself in a different
   run — repaints them per row. Multiset diffs VM-vs-oracle: median
   0.23 (sort_abs tie-pairing on the 4-fold-degenerate lattice radii),
   halo tail up to ~13. Same doctrine as giga_2864/outflow: the
   reference's own dust is equally arbitrary; the ensemble is the
   artwork.
3. **The EMA memory is one row deep** (influence of row k−j decays as
   0.1^j), so a stateless per-row analog with a SECOND independent
   draw standing in for the previous row is nearly exact: numpy analog
   vs the true sequential chain, 2D cloud correlation **0.975**. Both
   the analog and the faithful chain score the same (~0.60) against
   the reference PNG — the ceiling is the 5e8-root halo accumulation
   and frame/tone mapping, not the dynamics.
4. **RNG remap** (the 2883/2907 pattern): six independent frac-cascade
   uniforms of the sweep's own (t1, t2) replace the three unseeded
   draws (×2 for the previous-row analog). VM ensemble vs oracle
   ensemble on identical rows: 0.909 vs the 0.951 disjoint-half
   sampling floor at 400 rows.

## 2. Trap log (this wave)

- **Scalar math in vector-op args compiles chip-per-operation**: the
  naive program hit the 256-chip cap at 268. The 2877 idiom — park the
  parity mask on the stack, build displacement vectors with
  `scan(64, 0, tos[0]*(δ), tos[k]*(δ))` side-table expressions, mix
  with constant-vector scans — brought it to **86 tokens**.
- **frac() everywhere**: passing the raw (unfracced) cascade seeds into
  the displacement expressions put the real-shifted copy at radius
  ~1e5 (32 roots flung out). The phase copy survives (periodicity);
  the real copy does not.
- `expand_roots` is np.poly up to accumulation-order dust (~1e-10 of
  coefficient scale) at 64 roots — NOT bitwise (2910's 10-root case
  was). Fine for well-conditioned roots; the halo amplifies it, which
  is the paint.

## 2b. The lite variant

`giga_2920_lite` (uploaded 2026-07-17, fingerprint 974e871b) halves the
dominant cost: branch B — the 10%-weight previous-row analog — uses the
EXACTLY KNOWN phase-displaced lattice points instead of a second solved
mix, so each row runs ONE degree-64 eigensolve instead of two. Measured
lite-vs-full 2D cloud correlation: 0.979 against a 0.993 half-vs-half
sampling floor — visually identical ensembles.

## 3. Polypaint Coeff Program

`giga_2920.coeff-program.json` (86 tokens, stack_max 5; generator
`scripts/gen_giga_2920_coeff_program.py`; suite
`tests/test_giga_2920_coeff_program.py`, predeploy-gated). Structure:

```text
branch B (previous-row analog, uniforms u4..u6):
  parity → stack; scan(64) builds parity·δ_phase; + lattice; expand_roots
  same for δ_real; convex mix via constant-vector scans
  roots_cm(lo, exact)[0:64]; sort_abs        → ×0.1, push
branch A (current row, uniforms u1..u3): same → ×0.9
add; expand_roots; emit                       (monic, degree 64)
```

Verification tiers: whites are mix-roots at ≤1e-3 (np.roots dust,
measured 2.1e-4); pooled-cloud anchors median <0.01 / max <0.6; per-row
multiset vs oracle median <0.6 / halo tail <25; monic slot exact; halo
bounded (max|z| < 50, ≤8 escapees). The deployed compiler accepts the
program as-is (no rebinding, statement-form transforms).
