# Recreating `giga_2864`

## 0. Objective and safety boundary

Recreate `/Users/nicknassuphis/pyroots/giga_2864/giga_2864.png` from its
saved state. The reference tree is read-only; no new run may use the
`giga_2864` stem.

## 1. Main conclusions

`giga_2864` (run 2025-01-24; `poly_creative10`/`zz` at snapshot
`500685f`) is the second solver-noise artwork of the collection — and
the most extreme formula collapse:

```text
chain: zz -> poly_creative10 -> none -> safe
run:   roots=2e9, res=50000, alpha=0.0, procs=14, chunk=2040816,
       degree=70, ROTATE=270
view:  (-1.374206211752728 - 1.376189164773158j) ->
       ( 1.3781721177935862 + 1.3761891647731561j)
```

### 1.1 The redundancy (the user called it)

`zz` sends BOTH arguments to the same `z = t1 + 1j*t2`. The
"geometric algebra product terms" then compute `dot(a, b)` and
`wedge(a, b)` **with a == b**: the wedge `x*y - y*x` is EXACTLY `+0.0`
in IEEE (the same product subtracted from itself), and the dot is
`|z|^2 = t1^2 + t2^2 =: r`. The alternating-sign power ladder
therefore collapses to a geometric sequence:

```text
cf[k] = (-r)^(k+1),   k = 0..70
```

Everything else in the function is decoration around a one-liner.

### 1.2 The true roots are boring — and that is measurable

The polynomial is the geometric series
`-r * (x^71 - (-r)^71)/(x + r)`: its TRUE roots are the 71st roots of
`(-r)^71` minus the point `x = -r` — a perfect circle of radius r with
one deletion, at 70 FIXED angles. Sweeping (t1, t2) moves only the
radius, so each angle draws a straight radial line: the exact-artwork
is a clean 70-ray starburst and nothing more. Aberth-Ehrlich converges
to the true roots, so that is what it renders. Measured: the
true-roots cloud correlates 0.735 with the reference — BELOW the 0.866
same-ensemble noise floor. Visibly different; "boring" quantified.

### 1.3 The orb is the eigensolve's painting

The coefficients span `r^1..r^71` — a dynamic range of `r^70`. On that
scaling the companion-matrix eigensolve (`safe` = np.roots = f64 zgeev)
makes STRUCTURED errors: the computed roots scatter off the true
circle in ringed, layered, moire-textured patterns — the ornate orb
that fills the image's center, wrapped in the ray starburst that the
truth provides. The artwork is the error field of Francis QR on
ill-scaled companion matrices, swept over a parameter plane.

polypaint's companion_matrix path (f32 coefficient transport + f64
zgeev + f32 root output) was simulated end to end: ensemble cloud corr
**0.9273** vs the reference, above the 0.8665 floor — same starburst,
same orb, same texture class. The f32 stages do not disturb the
structure at ensemble level.

## 2. Reference artifacts

| File | Geometry | Size | SHA-256 |
|---|---:|---:|---|
| `giga_2864.png` | 50000 x 50000, 8-bit grayscale | 41,925,129 bytes | `51c15064f682cfd7452f748029ada65c6b401bdffa7fea7630c381d43fea4ff0` |
| `giga_2864_inv.png` | 50000 x 50000, 8-bit grayscale | 42,088,968 bytes | `43608565dadfae4633b87d988b5dfb2b64d912aa9dc157188fa82b62f17298b0` |
| `giga_2864_sml.png` | 1000 x 1000, 8-bit grayscale | 154,742 bytes | `9d9df9a242079b238f6c6ea1c3c3bf43bd699488acec6431172ec61697b8bf3f` |
| `giga_2864_sml_inv.png` | 1000 x 1000, 8-bit grayscale | 157,388 bytes | `441a099b2e89e003b220adc5b53c52d8b8329cdd1c775daca358b84d40fe766a` |

Measured polarity: `_sml` mean 21.56 (dark background), `_sml_inv`
233.44; full-size mid-band ~72.3 / inv ~182.7. This stem has a
full-size `_inv`.

Sidecars: `_paramconstruct` = `zz / poly_creative10 / none / safe`,
`_paramcount` = `2000000000`, `_resolution` = `50000`, `_solver` =
`safe`, `_alpha` = `0.0`.

## 3. Polypaint Coeff Program

Proven program — THREE TOKENS, the shortest of the series
(fingerprint-preserving round trip;
`fb83ee3e430346434ddf00b57fb77b9b95f617c8`):

```text
poly = scan(71, 0, 0-(t1*t1+t2*t2), prev*(0-(t1*t1+t2*t2)))
emit
```

One prev-chained scan builds the geometric sequence. Coefficient
parity vs the verbatim complex-power reference spelling: worst
`2.7e-15` per-slot RELATIVE over 45 rows, including the `r^71` tinies
(the two power paths — repeated multiplication vs numpy's cpow —
differ only in dust). The suite also pins the wedge == +0.0 collapse
and the true-roots circle identity.

Run settings — **the solver choice IS the artwork**:

```text
base coefficient function = const
degree                    = 70
solver                    = companion_matrix   <- REQUIRED for the orb
                            (aberth_mt renders the true circle: the
                             boring 70-ray starburst alone)
viewport                  = re in [-1.374206211752728, 1.3781721177935862]
                            im in [-1.376189164773158, 1.3761891647731561]
render                    = ROTATE 270, dark background (no invert)
N = 5345  ->  1,999,725,750 roots (the reference's 2e9 scale)
N =  534  ->     19,960,920 roots (5K-class validation)
```

Saved through `/save-coeff-program` as id `giga-2864`, predeploy-gated.
No deploy needed. Scrub knob: the 71 (scan length) sets the ray count
and the conditioning depth — more slots, wilder orb.
