# Recreating `giga_265`

## 0. Objective and safety boundary

Recreate the image represented by
`/Users/nicknassuphis/pyroots/giga_265/giga_265.png` using the saved state in
`/Users/nicknassuphis/pyroots/giga_265/giga_265_polyfun.txt`. The reference
tree is read-only: nothing in `/Users/nicknassuphis/pyroots` may be deleted,
renamed, checked out, regenerated, or overwritten, and no new run may use the
`giga_265` stem.

## 1. Main conclusions

1. `giga_265` is the sibling of `giga_263`: same 17 January 2025 snapshot
   format, same runner (the pre-module-split monolith), same `batman`
   coefficients, `rev`, `safe`, resolution 5000, 30M roots at degree 8,
   rotate 90, no invert, literal thumbnail naming. Only the parameter
   transform chain differs: `['bkr', 'uc', 'epow']` instead of
   `['unit_circle', 'coeff7']`.
2. **This snapshot is not fully self-describing.** The embedded `xfrm` class
   methods delegate to helpers `bkr1`, `uc1`, `epow1` that the snapshot does
   NOT define. They are recovered from `xfrm.py` at commit `483801d`
   (`uc1` :309, `epow1` :422, `bkr1` :438) — and `git log -S "def bkr1"`
   shows `bkr1` was **introduced in that very commit**, pinning `483801d` as
   the producing snapshot rather than merely the nearest one.
3. The recovered chain, for a real uniform `t in [0,1)`:

   ```text
   bkr1:  t < 0.5  ->  b = 2t            (real)
          t >= 0.5 ->  b = (2t-1) + 0.5i
   uc1:   u = exp(2*pi*i*b)   ->  |u| = 1        for the first branch
                                  |u| = e^(-pi)  (~0.0432) for the second
   epow1: p = exp(u)
   ```

   Half of all samples therefore ride the unit circle and half ride a tiny
   circle of radius `e^(-pi)` before the final exponential — a two-regime
   parameter family that the image's structure reflects.
4. `alpha = 0.0` (unlike 263's `0.01`): the estimation pass takes the exact
   min/max of its 100,000-sample roots, square-unioned as before. Full-run
   roots that exceed the sample extremes clamp onto the border; measured:
   one lit border line (post-rotation right column, mean 5.87), the other
   three fully dark.
5. Both `solve.safe` guards and the `|Im| <= 1e-10` near-real exclusion are
   measured non-events: 0 guard rows and 0 excluded roots in 20,000 sampled
   rows (160,000 roots); `max sum|cf| = 717.3` — binary32-trivial.
6. Thumbnail naming is literal (measured): `_sml` equals `resize(full)` with
   0.0000 mean absolute difference; `_sml_inv` is its inverse.
7. **Expressible in Polypaint today with zero language changes.** The
   candidate program (section 8) compiles, runs on the native VM, and matches
   the recovered historical chain to a worst-case relative error of
   `1.5e-16` across four probe rows covering both baker branches and the
   exact branch edges `t = 0` and `t = 0.5`.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_265.png` | 5000 x 5000, 8-bit grayscale | 1,424,371 bytes | `ddeac3d7ac0388a3198c771a3a18a0b03ebc03da9a7c5c14fcb6f9e6592eedf9` |
| `giga_265_sml.png` | 1000 x 1000, 8-bit grayscale | 431,110 bytes | `a36cf1299ac7d3e749f62126d2edb13a5d321f3fd5439d88e56949b4426354aa` |
| `giga_265_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 433,004 bytes | `323b045ead728eb3efaf7f5f96e6960c5c44fa2479469e37382db94baa833086` |

Measured polarity: full mean 44.39 (dark background), `_sml` 44.73 (same
polarity, exact downsample), `_sml_inv` 210.27 (inverse).

Sidecars: `_alpha` = `0.0`, `_paramconstruct` = `bkr,uc,epow / batman / rev /
safe`, `_paramcount` = `30000000`, `_resolution` = `5000`, `_solver` = `safe`.

## 3. Saved calculation

```python
xfrm    = ['bkr', 'uc', 'epow']
poly    = 'batman'            # n = 9 default -> degree 8
zfrm    = ['rev']
solve   = 'safe'
roots   = 30_000_000
procs   = 14
chunk   = 267_857
res     = 5000
samples = 100_000
alpha   = 0.0
margin  = 0.0
rotate  = 90
invert  = False
min_re  = min_im = -1.0827928389190722
max_re  = max_im =  1.0830723405839713
degree  = 8
```

## 4. Producing pipeline

Identical to `recreate_giga_263.md` section 4 (the same `pyroots.py` at
`483801d`): chained `functools.reduce` transforms, `batman`, `rev`, `safe`,
int-scale binning with border clamp and the `|Im| > 1e-10` gate, shared
float64 histogram thresholded to occupancy, pyvips `rotate(90)`, no invert,
literal thumbnails. See that document for line references, resource numbers
(200 MB histogram; minutes, no sharding), the isolated-clone procedure, and
the fixed-view driver design — all of it applies verbatim with this stem's
options and the recovered `bkr1/uc1/epow1` sources added to the driver.

Actual work: `267,857 x 14 x 8 = 29,999,984` roots.

## 5. Formula (recovered chain + embedded snapshot)

Per sample, uniforms `t1, t2`:

```text
b  = bkr1(t)  = (2*(t%1))%1 + i*((t%1 applied to Im, i.e. 0) + floor(2*(t%1)))/2
u  = exp(2*pi*i*b)
p  = exp(u)
k  = 1..9
mag_k = log(|p1| + |p2| + k) * k^2
ang_k = angle(p1)*sin(k) + angle(p2)*cos(k)
cf    = reversed( mag * exp(i*ang) )        # rev -> leading = k=9 term
roots = safe(np.roots(cf))                  # guards measured never firing
```

## 6. Validation targets

Structural: geometries and polarity per section 2; exactly one partially lit
border line; root count 29,999,984; empty-center check against the reference.
Visual: silhouette, lobe positions, the two-regime density structure, the lit
border segment's position. Hash equality is not an acceptance criterion (PID
seeds unrecorded).

## 7. Historical uncertainties

Per-worker PID seeds unrecorded; no dependency lock. The helper recovery in
conclusion 2 is evidence-pinned to `483801d` and is not an uncertainty.

## 8. Polypaint Coeff Program

### 8.1 Status and headline

**Zero language changes.** Proven program (compiles at 158 tokens, stack
depth 9, no pools; native-VM parity `1.5e-16` worst-case over four probe rows
covering both baker branches and the exact edges; fingerprint-preserving
chain→source round trip):

```text
x1 = t1 - floor(t1)
b1 = (2*x1 - floor(2*x1)) + 1i*floor(2*x1)/2
w1 = exp(exp(pi2i*b1))
x2 = t2 - floor(t2)
b2 = (2*x2 - floor(2*x2)) + 1i*floor(2*x2)/2
w2 = exp(exp(pi2i*b2))
poly = arange(9, 0, -1)
poly = sin(poly)
poly = multiply(poly, angle(w1))
poly
poly = arange(9, 0, -1)
poly = cos(poly)
poly = multiply(poly, angle(w2))
poly = add(pop, poly)
poly = multiply(poly, 1i)
poly = exp(poly)
poly
poly = arange(9, 0, -1)
poly = add(poly, abs(w1)+abs(w2))
poly = log(poly)
poly
poly = arange(9, 0, -1)
poly = multiply(poly, poly)
poly = multiply(pop, poly)
poly = multiply(pop, poly)
emit
```

The baker's map spells exactly with scalar locals: `x % 1` is `x - floor(x)`,
and the snapshot's `(y_fold + shift)/2` reduces to `floor(2*x1)/2` because
the inputs are real uniforms (`Im = 0`). The locals are named `w1/w2` because
`p1/p2` are reserved parameter registers. The batman body is byte-identical
to the `giga_263` program.

### 8.2 Fidelity notes (measured)

- Parity `1.5e-16` at `(0.137, 0.823)`, `(0.75, 0.31)`, `(0.5, 0.999)`,
  `(0.0, 0.499)`.
- `max sum|cf| = 717.3` over 20k rows; f32 transport unconditionally safe.
- `solve.safe` band gate and near-real exclusion: 0 events in 20k rows.
- The border-clamp difference documented for `giga_263` applies here too:
  Polypaint drops out-of-view roots, so the reference's lit border segment
  will not appear.

### 8.3 Run settings

```text
base coefficient function = const
degree                    = 8
solver                    = aberth_mt or companion_matrix
times                     = 1
Coeff Program             = giga_265 (saved program)
viewport                  = re,im in [-1.0827928389190722, 1.0830723405839713]
render                    = rotate 90, dark background (no invert)
N = 1936  ->  29,993,888 roots (native-scale run)
N =  612  ->   2,996,352 roots (fast validation)
```

### 8.4 Implementation

Mirrors `giga_263`: `scripts/gen_giga_265_coeff_program.py` emits the section
8.1 source as the ordinary portable document with `--check`;
`tests/test_giga_265_coeff_program.py` pins freshness, shape, the round trip,
and four-point native parity against the recovered chain implemented verbatim
and independently; saved through `/save-coeff-program` (id `giga-265`) with
no app special-casing; predeploy-gated.
