# Recreating `giga_263`

## 0. Objective and safety boundary

Recreate the image represented by:

```text
/Users/nicknassuphis/pyroots/giga_263/giga_263.png
```

using the saved state in:

```text
/Users/nicknassuphis/pyroots/giga_263/giga_263_polyfun.txt
```

The existing `/Users/nicknassuphis/pyroots` tree and all existing `giga_*`
directories are reference material only. Nothing in that tree should be
deleted, renamed, checked out, regenerated, or overwritten. All experiments and
renders must run in a disposable local clone, with outputs in a new scratch
directory. Never pass `giga_263` as the output stem of a new run.

## 1. Main conclusions

1. `giga_263_polyfun.txt` is the authoritative specification, and — unlike the
   `giga_2902` dict-only snapshot — it is **self-describing**: it embeds the
   actual source of every selected function (`xfrm.unit_circle`, `xfrm.coeff7`,
   `poly.batman`, `zfrm.rev`, `solve.safe`) alongside the effective options.
   Reconstruction does not depend on recovering those functions from history.
2. The producing runner is the 17 January 2025 monolith. The closest
   recoverable snapshot is commit `483801d` (2025-01-17, "more"), one commit
   before the 18 January "split into modules". The sidecar files
   (`_paramconstruct`, `_paramcount`, `_alpha`, `_resolution`, `_solver`) match
   that era's writer.
3. The saved square view bounds are authoritative. `polyparse.py:303
   estimate_root_range` derives per-axis `np.percentile(..., alpha)` bounds
   with `alpha = 0.01`, then **squares them** by taking the min/max across both
   axes (`polyparse.py:386`), which is why `min_re == min_im` and
   `max_re == max_im` in the saved options. A rerun re-estimates a new random
   view; the reconstruction must use the saved bounds bit-for-bit.
4. Two raster details differ from the later `giga_2902` pipeline and are part
   of the image:
   - roots with `|Im(root)| <= 1e-10` are **excluded** from binning
     (`pyroots.py`, worker inner loop);
   - there is **no imaginary-axis flip**, and out-of-view roots are **clamped
     to the border pixel** rather than dropped. The measured consequence: one
     partially lit border line (post-rotation right column, mean 8.9/255), the
     other three borders fully dark.
5. Both `solve.safe` guards are measured non-events for this family: across
   20,000 sampled rows (160,000 roots), the `sum(|cf|)` band gate
   (`1e-10..1e10`) never fired (max observed 771.6) and **zero** roots fell
   within `1e-10` of the real axis. The guards matter for robustness, not for
   the composition.
6. The original run was nondeterministic (`np.random.seed(os.getpid())` per
   worker, shared float64 histogram without a write contract). A rerun
   reproduces the construction and composition, not the PNG bytes.
7. Thumbnail naming is **literal** in this era — the opposite of the
   `giga_2902` contract: `_sml` is a plain downsample of the full image
   (measured mean absolute difference 0.0000) and `_sml_inv` is its inverse.
   `args.invert = False`, so the full image is dark-background with light
   marks.
8. **The polynomial family is already expressible in Polypaint with zero new
   primitives.** A candidate Coeff Program (section 17) compiles in the current
   language, runs on the native VM, and matches the embedded historical formula
   to a maximum relative error of `1.7e-16` at a probe row, with a clean
   fingerprint-preserving chain→source round trip.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_263.png` | 5000 x 5000, 8-bit grayscale | 1,597,829 bytes | `da81244924f578e56b8d1f53eb8675b99cb5a3e14ab35f63b6daa5f4ee09df27` |
| `giga_263_sml.png` | 1000 x 1000, 8-bit grayscale | 437,554 bytes | `6260ad056980fddbfcc27af17d4519dfbfffcbe41f03d4a08341251ff6a35d33` |
| `giga_263_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 439,409 bytes | `f6b8b9d250e9575da574f9324f9961adcb616bb3eb608694aace8689773b1c89` |

Measured polarity contract (this era names its thumbnails literally):

```text
giga_263.png          full image, dark background (mean 61.8)
giga_263_sml.png      1000px downsample, SAME polarity as full (mean 62.1;
                      |resize(full) - sml| mean abs diff = 0.0000)
giga_263_sml_inv.png  inverted thumbnail (mean 192.9)
```

Sidecars: `_alpha` = `0.01`, `_paramconstruct` = `unit_circle,coeff7 / batman /
rev / safe`, `_paramcount` = `30000000`, `_resolution` = `5000`,
`_solver` = `safe`.

## 3. Saved calculation

Effective options (from the snapshot's `options` dict):

```python
xfrm    = ['unit_circle', 'coeff7']
poly    = 'batman'            # n = 9 default -> degree 8
zfrm    = ['rev']
solve   = 'safe'
roots   = 30_000_000
procs   = 14
chunk   = 267_857             # floor(30e6 / 8 / 14)
res     = 5000
samples = 100_000             # view-estimation pass
alpha   = 0.01
margin  = 0.0
rotate  = 90
invert  = False
min_re  = min_im = -1.1210708654699295
max_re  = max_im =  1.1221681211669654
degree  = 8
```

`parquet/blurr/combine/subtract/import` are all `False` and inactive.

## 4. How the program hangs together

Producing path (all in the 189-line `pyroots.py` at `483801d`):

```text
pyroots.py main
  -> polyparse.poly_setup()        options + polyfun snapshot writing
  -> make_hist                     shared float64 histogram, Pool(14)
  -> chunk_worker                  per-sample: xfrm chain -> poly -> zfrm
                                   chain -> solve -> bin (with exclusions)
  -> hist[hist>0] = 1; *255        threshold to uint8 occupancy
  -> make_png                      vips rotate(90), no invert, write full,
                                   downsample thumb, inverted thumb
```

Reference lines at `483801d`:

```text
pyroots.py:15    chunk_worker (xfrm/zfrm functools.reduce chains)
pyroots.py:44    np.random.seed(os.getpid())
pyroots.py:60    per-root binning: int scale, border clamp, |Im|>1e-10 gate
pyroots.py:95    make_png (rotate, optional invert, thumbnails)
pyroots.py:157   make_hist (shared memory, threshold)
polyparse.py:217 polyfun snapshot writer
polyparse.py:303 estimate_root_range (percentile alpha)
polyparse.py:386 square-union of per-axis bounds
```

## 5. What the calculation computes

### 5.1 Parameter transforms (chained)

Each sample draws two uniforms and folds them through the `xfrm` chain:

```text
u1 = exp(2*pi*i*t1)          u2 = exp(2*pi*i*t2)          (unit_circle)
p1 = (u1 + sin(u1)) / (u1 + cos(u1))                       (coeff7)
p2 = (u2 + sin(u2)) / (u2 + cos(u2))
```

`coeff7` receives **complex** arguments (the unit-circle points), so `sin` and
`cos` are the complex functions. The denominators `u + cos(u)` have isolated
zeros; a float sample never lands on one in practice, and `solve.safe` guards
the residual non-finite rows.

### 5.2 Coefficients: `batman(p1, p2, n=9)`

```text
k     = 1..9
mag_k = log(|p1| + |p2| + k) * k^2
ang_k = angle(p1)*sin(k) + angle(p2)*cos(k)
cf_k  = mag_k * exp(i*ang_k)
```

`zfrm.rev` flips the vector; `np.roots` treats index 0 as the leading
coefficient, so the effective leading coefficient is the `k = 9` term
(magnitude `81*log(...)`, never zero) and the polynomial has degree 8.

The coefficient scale is tame: measured `max sum(|cf|) = 771.6` over 20,000
samples. Unlike the degree-33 `giga_2902` family (coefficients to `1.35e22`),
this family is comfortably inside binary32 range and degree-8 root recovery is
well-conditioned — the float32 artifact boundary in Polypaint is a non-issue
here.

### 5.3 Solve and rasterize

`solve.safe` wraps `np.roots` with guards (degree < 2, non-finite
coefficients, `sum|cf|` outside `1e-10..1e10`, non-finite roots → 8 zero
roots). Measured: no guard fires in 20,000 samples.

Binning, per root (`pyroots.py:60`):

```text
x = int((re - min_re) / range_re * 5000)     clamped to [0, 4999]
y = int((im - min_im) / range_im * 5000)     clamped to [0, 4999]
bin only if |im| > 1e-10
```

Three properties to preserve or consciously deviate from:

1. **No imaginary flip** (y indexes im ascending; +im is the bottom row of the
   pre-rotation buffer).
2. **Border clamping**: out-of-view roots pile onto edge pixels. With the
   0.01-percentile view this lights part of one border (measured: one border
   line at mean 8.9, the others dark) — a genuine feature of the reference.
3. **Near-real exclusion**: measured zero occurrences for this family; it can
   be ignored without visible effect.

The histogram is thresholded (`hist > 0 -> 255`), so the full image is a
binary occupancy bitmap, rotated 90 degrees with pyvips `rotate(90)` and NOT
inverted.

## 6. Counts and resource implications

```text
267,857 samples/worker x 14 workers = 3,749,998 samples
x 8 roots                           = 29,999,984 roots (16 short of nominal)
histogram: 5000 * 5000 * 8 bytes    = 200 MB shared float64 (+25 MB uint8)
```

This is a light run: minutes on a laptop, no sharding or checkpoint concerns.
The 14-process layout is a convenience, not a resource necessity.

## 7. Why a naive current-tree invocation is insufficient

1. `poly_setup()` re-estimates the view; the saved square bounds would be
   replaced by new random-sample percentiles.
2. The current tree has evolved past the January runner (module split on
   18 January 2025, `polys/` split later); function lookup and the PNG
   pipeline have both changed.
3. The snapshot embeds its own function sources, so the faithful route
   executes those, not whatever the current tree resolves for the same names.

## 8. Isolated environment

Same discipline as `giga_2902`:

```bash
git clone --no-hardlinks /Users/nicknassuphis/pyroots /private/tmp/pyroots-giga-263
git -C /private/tmp/pyroots-giga-263 checkout --detach 483801d
```

Use the existing `.venv` read-only, `PYTHONDONTWRITEBYTECODE=1`, caches under
the scratch directory, `pip freeze` recorded into the scratch run record.

## 9. Phase A: protect and inspect the reference

1. Recompute the three SHA-256 hashes against section 2.
2. Confirm `git status --short` in the reference repository is empty.
3. Copy `giga_263_polyfun.txt` into the clone; drivers read the copy only.
4. Choose a unique scratch output stem; fail if it exists.

## 10. Phase B: fixed-view reconstruction driver

The snapshot embeds everything, so the driver is small:

1. Execute the scratch copy of `giga_263_polyfun.txt` as a namespace; take
   `options` and the four classes from it. Do not resolve any function from
   the repository tree.
2. Override only the output stem (and optionally `procs`).
3. Assert before allocating anything:

   ```text
   degree == 8        chunk == 267,857      roots == 30,000,000
   res == 5000        min_re == min_im      max_re == max_im
   saved bounds match section 3 bit-for-bit
   ```

4. Reimplement the 60-line worker loop verbatim (chained xfrm reduce, batman,
   rev, safe, int-scale binning with border clamp and the `|im| > 1e-10`
   gate), with a recorded deterministic seed per worker instead of
   `os.getpid()`.
5. Threshold, rotate 90 with pyvips, do NOT invert, write the full PNG.
6. Thumbnails per section 12.

A single run is fine (200 MB histogram); shards are unnecessary at this size.
A 1000px smoke run with `roots = 300_000` validates the pipeline first.

## 11. Phase C: thumbnails

Literal naming for this stem (measured, section 2):

```text
<stem>_sml.png      resize(full, 1000/5000)   same polarity as full
<stem>_sml_inv.png  invert(<stem>_sml.png)
```

Use libvips resize, as the reference did.

## 12. Validation

Structural: 5000x5000 and 1000x1000 one-band 8-bit grayscale; full-image mean
near 61.8 (binary occupancy at this root count is statistically stable);
`_sml` matches `resize(full)` exactly; `_sml_inv` is its inverse; exactly one
partially lit border line (the clamp fingerprint); actual root count recorded
as 29,999,984.

Visual, against `giga_263.png` (dark background): overall silhouette and
symmetry, empty center (measured: the central 200px region is fully dark),
lobe positions, the lit border segment's location. Hash equality is not an
acceptance criterion (PID seeds were never recorded).

## 13. Historical uncertainties that do not block reconstruction

- Per-worker PID seeds were not stored (composition is stable at 30M roots).
- No dependency lock exists; NumPy/BLAS builds are unprovable from the
  repository. Nothing else is uncertain: the snapshot embeds the math.

## 14. Polypaint Coeff Program implementation plan

### 14.0 Status and headline

**No language work is required.** Everything `giga_263` needs shipped with the
`giga_2902` wave and its follow-ups: complex scalar expressions with locals,
`arange(start, stop, step)`, vector unaries (`log`, `exp`, `sin`, `cos`,
`abs`, `angle`), vector binaries with scalar broadcast, and the stack
(`poly` / `pop`). The candidate program below **compiles today, ran on the
native VM, and matched the embedded historical formula to `1.7e-16` maximum
relative error** at probe row `(t1, t2) = (0.137, 0.823)`, with a
fingerprint-preserving chain→source round trip and zero fallback warnings.

### 14.1 Canonical program

`zfrm.rev` costs nothing: building the index vector with `arange(9, 0, -1)`
produces every elementwise term already in reversed (leading-first) order.

```text
u1 = exp(pi2i*t1)
u2 = exp(pi2i*t2)
v1 = (u1+sin(u1))/(u1+cos(u1))
v2 = (u2+sin(u2))/(u2+cos(u2))
poly = arange(9, 0, -1)
poly = sin(poly)
poly = multiply(poly, angle(v1))
poly
poly = arange(9, 0, -1)
poly = cos(poly)
poly = multiply(poly, angle(v2))
poly = add(pop, poly)
poly = multiply(poly, 1i)
poly = exp(poly)
poly
poly = arange(9, 0, -1)
poly = add(poly, abs(v1)+abs(v2))
poly = log(poly)
poly
poly = arange(9, 0, -1)
poly = multiply(poly, poly)
poly = multiply(pop, poly)
poly = multiply(pop, poly)
emit
```

122 tokens, stack depth 4, no constant pool, no scalar-expression pool.
Locals are scalar-only substitutions (a vector cannot be named), which is why
the `k` vector is re-issued by `arange` at each use — four extra tokens,
nothing at run time. The `u1/u2/v1/v2` locals keep the source readable; the
compiler substitutes them into the typed scalar streams.

### 14.2 Fidelity notes (all measured)

- **Parity**: native VM row vs the snapshot's own `batman`/`coeff7` formula:
  max relative error `1.66e-16` (machine epsilon).
- **Conditioning**: `max sum|cf| = 771.6` over 20k samples — binary32
  transport is unconditionally safe for this family; degree-8 recovery is
  well-conditioned. None of `giga_2902`'s §17.8 caveats apply.
- **`solve.safe` gates**: never fire (0 of 20,000 rows). The app's existing
  solver guards are a superset of what this family needs.
- **Near-real exclusion**: 0 of 160,000 roots within `1e-10` of the real
  axis; Polypaint plotting all roots is visually indistinguishable.
- **Border clamp**: Polypaint drops out-of-view roots instead of clamping, so
  the reference's single lit border segment will NOT appear. This is the one
  known, deliberate composition difference.
- **Sampling**: the app's deterministic `(t1, t2)` grid replaces the
  historical worker RNG — same two-dimensional distribution, reproducible.

### 14.3 Run settings

```text
base coefficient function = const
degree                    = 8
solver                    = aberth_mt or companion_matrix
times                     = 1
Coeff Program             = giga_263 (saved program)
viewport                  = re,im in [-1.1210708654699295, 1.1221681211669654]
render                    = rotate 90, dark background (no invert)
```

Root-count mapping at degree 8 (`N^2 * 8` roots):

```text
N = 1936  ->  29,993,888 roots   (the 30M original; res 5000 is native scale)
N =  612  ->   2,996,352 roots   (fast validation pass)
```

Viewport, rotation, and polarity are render settings, not program semantics.

### 14.4 Implementation steps

1. `scripts/gen_giga_263_coeff_program.py`: emit the section-14.1 source as
   `giga_263.coeff-program.json` in the ordinary portable shape
   (`{version, program_kind, name, chain: [], source_text}`), with `--check`
   staleness support — the same contract as the giga_2902 generator, minus
   any expansion work (there are no constants to precompute).
2. Extend `tests/test_giga_2902_coeff_program.py` (or add a sibling) with:
   the generated document compiling; a native `compute_debug` row matching an
   independent NumPy oracle that implements the snapshot formula verbatim
   (the oracle must not share the program's code path); the chain→source
   round trip; and the save/list/fetch storage contract if a saved-program
   test slot is wanted.
3. Save through the existing `/save-coeff-program` route (name `giga_263`,
   id `giga-263`). No registry, Starter, preset, or deploy-asset additions.
4. Validate end-to-end per §17.11 Phase-6 discipline: compute-debug rows →
   small-N scatter → `N = 612` occupancy comparison against a downsampled
   reference → `N = 1936` at 5K with the section 14.3 viewport and rotation.

### 14.5 Acceptance criteria

1. Fixed-row binary64 coefficients match the snapshot formula (`<= 1e-12`
   relative; measured headroom is four orders better).
2. The saved program is ordinary in every way: normal compile, normal
   coeffgen artifact, both solvers accepted, no app special-casing.
3. The 5K app render at `N = 1936` reproduces the reference composition,
   viewport, orientation, and polarity, modulo the documented border-clamp
   difference and pixel-level sampling noise.
4. Every new artifact (generator output, tests) is predeploy-gated.
