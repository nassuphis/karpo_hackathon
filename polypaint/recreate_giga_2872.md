# Recreating `giga_2872`

## 0. Objective and safety boundary

Recreate the image represented by
`/Users/nicknassuphis/pyroots/giga_2872/giga_2872.png` using the saved state
in `/Users/nicknassuphis/pyroots/giga_2872/giga_2872_polyfun.txt`. The
reference tree — including `fonts/3dfx8x8.png` — is read-only. No new run may
use the `giga_2872` (or `giga_2869`) stem.

## 1. Main conclusions

1. **The roots are the lit pixels of an IBM PC (CP437) character.** The saved
   `poly_letter` with `ascii = 178` selects `b178` — the medium-shade block
   `▓` — whose 8x8 bitmap is a perfect 32-pixel checkerboard. Provenance
   chain: `fonts/3dfx8x8.png` (144x288, 1-bit; added 25 Jan 2025) was
   transcribed by `pngfont2pydict.py` into the `FONT` bit-row dictionary in
   `letters.py` (added 26-27 Jan); `get_letter_coordinates` maps each set bit
   to `complex(x - 3.5, (7 - y) - 3.5)` with `factor = 1`. The runtime reads
   the dictionary, not the PNG.
2. The producing snapshot is `500685f` (27 Jan 2025) — `letters.py` was
   created the day before and modified that day, the sidecar files are dated
   27 Jan, and the runner tree already has the split modules
   (`polyflow/polyview/polypng`), i.e. the same runner family as `giga_2902`.
3. Per sample, `letters.square` **translates** the fixed 32-root
   constellation by `complex(scale(t1, 0.9), scale(t2, 0.9))` with
   `scale(t, s) = (1-s)/2 + s*t` — a real x/y shift in `[0.05, 0.95]^2` —
   then `np.poly` expands the shifted roots. This is exactly the
   constant-pool + `translate_roots` decomposition Polypaint already has.
4. **`zfrm.andy1` explains the saved `degree = 31`.** It computes
   `p = argsort(|cf|)`, `csi = cumprod(p) % 33`, `cpi = cumsum(p) % 33`, and
   returns `cf[csi] - cf[cpi]`. Because `csi[0] == cpi[0] == p[0]`, the
   leading output element is **identically zero**, so the effective
   polynomial has 32 coefficients — degree 31. The saved
   `chunk = floor(2e9 / 31 / 14) = 4,608,294` confirms the era derived 31.
5. **The cumprod collapses.** The polynomial is monic (`|cf[0]| = 1`) and
   every other coefficient of this constellation is enormously larger, so
   `argsort[0] == 0` — measured on 5,000 sampled rows without exception.
   The cumulative product therefore hits index 0 immediately and
   `csi = [0, 0, ..., 0]`, collapsing `andy1` to `1 - cf[cpi]` exactly
   (with element 0 equal to 0). This matters doubly: for rows where
   `argsort[0] != 0` (never observed), NumPy's int64 `cumprod` **overflows
   and wraps** for 33 indices — semantics no binary64 evaluator can honestly
   replicate. The collapse regime is both the observed and the only
   faithfully reproducible behavior.
6. `xfrm = none` is the identity: the two uniforms feed `square` directly
   (no discarded draws in this era's `polyflow.sample`).
7. `solve = polyroot` is the compiled C++ extension. The recreation is
   solver-agnostic (same polynomial, any correct root finder); Polypaint's
   AE-MT or CM both qualify.
8. Raster era: `polyview.pixels` **clips** out-of-view roots (no border
   clamp), flips the imaginary axis, and the shared histogram is uint8 with
   the same wrap caveat as `giga_2902`. `png = none, rotate = 0`: the full
   image is dark-background occupancy with NO rotation and NO inversion.
9. Thumbnail naming is **literal** (measured): full mean 23.63 == `_sml`
   mean; `_sml_inv` and the full-size `giga_2872_inv.png` (written 12 Feb,
   a later publish artifact) are exact inverses (mean 231.37 = 255 - 23.63).
10. `data.stem = giga_2869` — the same stale stem as `giga_2902`: both
    results were renamed from one working stem after compute.
11. **Expressible in Polypaint today with zero language changes.** The
    candidate program (section 8) uses `roots_literal` + `translate_roots`
    for the constellation, the VM's permute-by-key `argsort` to extract the
    permutation as values, `scan` for the cumulative sum AND as an
    alias-free gather (`tos[poly[k]]` — scan pushes to the stack, never
    writing operands mid-loop), and `rem` for the modulus. Native parity vs
    the verbatim historical formula: worst `3.8e-12` relative over four
    probe rows.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2872.png` | 50000 x 50000, 8-bit grayscale | 132,360,565 bytes | `75dfd8def013a38189ea565c1c32641893a0cf9de79cbb384dbb1cd3d394c493` |
| `giga_2872_inv.png` | 50000 x 50000, 8-bit grayscale | 132,572,869 bytes | `6dc5577e3cb59583b16ad462cc5e4a0e97e98bba41b30e4ac4fe0559517ac64b` |
| `giga_2872_sml.png` | 1000 x 1000, 8-bit grayscale | 331,747 bytes | `2633b24a82e70c82a8c4792dfabb59799a44ca383301edf94f55060d0bfb6719` |
| `giga_2872_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 334,132 bytes | `36b0df3064cd54d3ccdf8ba83e54c1eb00cb01ca9360573316fdf7c956a456ab` |

Measured polarity (literal naming): full/`_sml` mean 23.63 (dark
background); `_inv`/`_sml_inv` mean 231.37 (inverted).

Sidecars: `_alpha` = `0.0001`, `_paramconstruct` = `none / poly_letter /
andy1 / polyroot`, `_paramcount` = `2000000000`, `_resolution` = `50000`,
`_solver` = `polyroot`.

## 3. Saved calculation

```python
poly = { 'ascii': 178, 'degree': 31, 'poly': 'poly_letter',
         'solve': 'polyroot', 'xfrm': 'none', 'zfrm': 'andy1' }
view = { 'alpha': 0.0001, 'margin': 0.075, 'res': 50000,
         'samples': 100000, 'subview': 'full',
         'view': (-339.69995503929636 - 480.70359996814324j,
                   365.84233452709515 + 224.83868959824815j) }
job  = { 'chunk': 4608294, 'procs': 14, 'roots': 2000000000 }
png  = { 'png': 'none', 'rotate': 0 }
data = { 'stem': 'giga_2869', ... }   # stale, as with giga_2902
```

## 4. Formula (all sources at `500685f`)

```text
poly.py:18789      poly_letter: cf = letters.square("b178", t1, t2, 0); np.poly(cf)
letters.py:2056    b178 bitmap (checkerboard, 32 set bits)
letters.py:2851    get_letter_coordinates: bit (x,y) -> complex(x-3.5, (7-y)-3.5)
letters.py:2872    scale(t, s) = ((1-s)/2 + s*t) * factor,  factor = 1
letters.py:2875    square: FONTXY[key] + complex(scale(t1,.9), scale(t2,.9))
zfrm.py:421        andy1: cf[cumprod(argsort|cf|) % n] - cf[cumsum(argsort|cf|) % n]
polyflow.py:14     sample: xfrm none -> poly_letter -> andy1 -> polyroot
polyview.py:98     pixels: clip to view, imaginary flip, (res-1) scaling
```

Per sample: 32 checkerboard roots shifted by the sample's x/y offset; monic
degree-32 expansion (`max |cf|` observed `~3.4e14`); `andy1` gathers by the
two cumulative index walks and differences them (leading element identically
0); the surviving degree-31 polynomial is solved; roots plot into the saved
view. Actual work: `4,608,294 x 14 = 64,516,116` samples x 31 roots =
`1,999,999,596` roots. Resources match `giga_2902` (2.5 GB uint8 shared
histogram at 50K; the sharded strategy from that document applies verbatim
if a historical rerun is wanted).

## 5. Reconstruction route

The `giga_2902` playbook applies with three substitutions: snapshot commit
`500685f`; the driver loads this stem's dictionaries and asserts
`degree == 31`, `chunk == 4,608,294`, the saved view bit-for-bit; and the
output pipeline is the identity (no rotation, no inversion) with literal
thumbnail naming. The `letters.py` FONT dictionary ships inside the
snapshot-era tree, so the disposable clone needs no extra data — the fonts
PNG is provenance, not a runtime input.

## 6. Validation targets

50000x50000 and 1000x1000 one-band grayscale; means near 23.6/231.4 with the
literal polarity contract; no lit border (this era clips); root count
1,999,999,596; the checkerboard-translation structure repeated across the
composition. Hash equality is not an acceptance criterion (clock/PID worker
seeds, uint8 histogram races).

## 7. Historical uncertainties

Worker seeds and the uint8-wrap race, as with `giga_2902`. The
`argsort[0] == 0` collapse is measured (5,000/5,000) rather than proven for
every float in `[0.05, 0.95]^2`; a hypothetical violating row would invoke
int64-wrapping cumprod semantics that no faithful recreation can honor —
the parity suite's oracle asserts the regime so any drift is loud.

## 8. Polypaint Coeff Program

### 8.1 Status and headline

**Zero language changes** — this is the program the pool machinery was built
for, plus one idiom: `scan` doubles as an alias-free **gather**, because it
evaluates per-element expressions (with dynamic indexed reads like
`tos[poly[k]]`) into scratch and pushes the result, never mutating its
operands mid-loop. Proven program (40 tokens, stack depth 6, one pool
constant, three scan expressions; native parity worst `3.8e-12` over four
probe rows; fingerprint-preserving round trip):

```text
poly = translate_roots(roots_literal(<32 b178 roots>), (0.05+0.9*t1) + 1i*(0.05+0.9*t2))
poly
poly = arange(0, 33)
poly = argsort(poly, peek)
poly
poly = scan(33, 0, tos[0], prev + tos[k])
poly = rem(poly, 33)
drop
poly = scan(33, 0, tos[poly[k]], tos[poly[k]])
drop
poly = multiply(poly, 0-1)
poly = add(poly, 1)
emit
```

Line by line: translate the pooled constellation by the row's x/y offset
(the constant pool holds the 33 expanded coefficients of the UNSHIFTED
checkerboard, computed exactly once); keep the coefficients on the stack;
`argsort(arange, peek)` extracts the sort permutation **as values**;
`scan` accumulates the cumulative sum; `rem` takes mod 33; drop the
permutation; the second `scan` is the gather `cf[cpi[k]]`; the final
`1 - gathered` is `andy1` in its (always-observed) collapsed form —
element 0 lands on exactly 0 without special-casing, and both solvers strip
the leading zero to solve the degree-31 polynomial, exactly as
`np.roots` did historically.

### 8.2 Fidelity notes (measured)

- Parity `3.8e-12` worst-case at `(0.137, 0.823)`, `(0.75, 0.31)`,
  `(0.0, 0.999)`, `(0.5, 0.5)` — the residual is translate-vs-`np.poly`
  expansion-order dust magnified by degree-32 binomials at the `1e14`
  coefficient scale, three orders below the float32 transport boundary.
- The float32 artifact boundary WILL perturb this ill-conditioned family
  visibly, exactly as `giga_2902` §17.8 documents for its degree-33 sibling;
  the same art-workflow acceptance applies.
- `argsort` tie-break: the VM's shell sort is stable-by-index on equal keys,
  matching NumPy's stable argsort; coefficient magnitudes of this family are
  generically distinct.
- Emit discipline: the stack must be EMPTY at `emit` (a leftover vector is
  popped into poly and silently replaces the result) — hence both `drop`
  statements.

### 8.3 Run settings

```text
base coefficient function = const
degree                    = 32          (program emits 33 coefficients;
                                         solvers strip the zero leading term
                                         -> 31 plotted roots per row)
solver                    = aberth_mt or companion_matrix
times                     = 1
Coeff Program             = giga_2872 (saved program)
viewport                  = re in [-339.69995503929636, 365.84233452709515]
                            im in [-480.70359996814324, 224.83868959824815]
render                    = no rotation, dark background (no invert)
N = 8031  ->  1,999,405,791 roots (two-billion-class run)
N =  803  ->     19,989,679 roots (5K-class validation)
```

### 8.4 Implementation

Mirrors the siblings: `scripts/gen_giga_2872_coeff_program.py` derives the
32 roots from the checked-in `b178` bit rows (the same transcription the
era's `letters.py` holds) and emits the section-8.1 source with `--check`;
`tests/test_giga_2872_coeff_program.py` pins freshness, shape, the round
trip, and four-point native parity against the verbatim `andy1` oracle
(including its `argsort[0] == 0` regime assertion); saved through
`/save-coeff-program` (id `giga-2872`); predeploy-gated.
