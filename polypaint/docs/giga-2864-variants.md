# giga_2864 iteration variants

giga_2864's simplified core is a geometric orbit — 71 coefficients from
iterating `x <- c*x` with `c = -(t1²+t2²)` swept by the parameters:

```
poly = scan(71, 0, 0-(t1*t1+t2*t2), prev*(0-(t1*t1+t2*t2)))
emit
```

These seven variants keep that shape (a 1D map iterated into the
coefficient slots, parameter-driven) and swap the iteration. All were
prototyped through the native VM before shipping (sweep_test
compute_debug, 25-probe grid over (t1,t2)); parity against pure-python
oracles is pinned by tests/test_giga_2864_variants.py — the four maps
built from IEEE-exact ops (`* + - floor abs`) match the VM **bitwise**,
spiral/sine/phases at ≤1e-9 (measured 6e-16 for spiral: complex-multiply
rounding grouping; sine/exp are libm calls).

| program | map | sweep behavior |
|---|---|---|
| `giga_2864_logistic` | `x ← r·x·(1−x)`, r ∈ [3,4] | walks the bifurcation cascade: period-2/4/8 plateaus (near-sparse coefficients → rosettes), chaos, the period-3 window at r≈3.83 cutting a clean band. Probe: distinct coefficient magnitudes swing 13↔71 — visible regime seams |
| `giga_2864_henon` | `x ← 1−a·x²+0.3·prev2`, a ∈ [1,1.4] | strange attractor: never periodic, both signs, root radius 0.3–4.7 — permanent chaos deforming continuously |
| `giga_2864_sine` | `x ← r·sin(πx)`, r ∈ [0.7,1] | the logistic's smooth conjugate; same cascade, differently curved seams |
| `giga_2864_spiral` | `z ← c·z`, complex `c = −(t1²+t2²)−i·t1·t2` | the original with a complex ratio: log-spiral decay, roots wind into a spiral, conjugate symmetry breaks. Inherits the base program's origin-corner degree collapse (tiny \|c\|⁷¹ underflows f32) |
| `giga_2864_bbaker` | β-baker `x ← frac(βx)`, β ∈ [1,1.98] | the baker's stretch-and-cut with a non-dyadic stretch: smooth melt from a 71st-roots-of-unity dot lattice (β≈1, near-equal coefficients) into full chaos |
| `giga_2864_tent` | `x ← 2r·(0.5−\|x−0.5\|)`, r ∈ [1,1.98] | the FOLDED baker: band-splitting regimes instead of periodic windows |
| `giga_2864_phases` | β-baker on angles, `coef = exp(2πi·x)` | every coefficient ON the unit circle: Erdős–Turán pins roots to a tight ring (probe \|z\|max 1.15–1.76) with baker-chaotic phase filigree |

## The doubling-map trap

The literal baker stretch `x ← frac(2x)` is a binary bit-shift: every
IEEE double is a dyadic rational, so the orbit lands on exactly 0
within ≤53 steps and every later coefficient is zero — probe-proven
**dead leading slot on 25/25 rows** (degree collapse). No program ships
with a power-of-2 stretch; β and the `+0.211327` offset in `phases`
keep orbits off the dyadic lattice.

Also: the source language has no `frac` — it is spelled `x - floor(x)`.

## Rejected variants

- **Alternating-sign mask** on any of these: multiplying coefficient k
  by (−1)^k is exactly `p(z) → p(−z)` — the identical image rotated
  180°. Any per-k geometric mask is a rigid transform; not a variant.
- **Complex-r logistic** (`r = 3.2+0.7·t1+0.6i·t2`): orbits decay and
  the leading slot dies at f32 on 22/25 probe rows.

## Rendering notes

The chaotic regions are where the solver-brush error fields bloom
(docs/solver-brushes.md): CM/JT paint the chaos bands differently from
the periodic plateaus. NEWT on chaotic coefficients is a basin fractal
of chaos. All variants are degree 70 — comfortably inside every
solver's range — and feed the custom hex-stop palettes like any run.
