# Escape-camera programs: mandel_camera, julia_camera

Born from an observation session (2026-07-17): the user's program

```
poly = scan(21, 0, -0.15, -1*prev*(1-prev)*t1*exp(2pi*101i*t2)+2)
```

painted a little Mandelbrot set on the real axis. Diagnosis: the scan's
coefficients are the orbit of a quadratic map (`x <- -λx(1-x)+2` is
conjugate to `y² + c` with `c = 3λ/2 - λ²/4`, seed near-critical), and
two mechanisms compose:

1. **The camera.** The smallest root of the emitted polynomial is an
   almost-affine function of the two constant-end coefficients,
   `z₁ ≈ -a₀/a₁` — a conformal map squeezing the ENTIRE swept
   parameter disk into a small blob. (Layout note: slot 0 is the
   LEADING coefficient in this pipeline, so "constant end" is the top
   slots; the accidental program's copy actually sits at the
   reciprocal location z ≈ 13.3.)
2. **The knife.** Rows whose orbit escapes grow doubly exponentially,
   overflow the f32 coefficient transport, and are skipped — punching
   the escape set out of the camera's image. What survives is the
   boundedness locus: a Mandelbrot (or Julia) silhouette.

The two shipped programs make the mechanism deliberate:

## mandel_camera (`mandel_camera.coeff-program.json`)

`c` sweeps the Mandelbrot bounding box [-2.1, 0.5] × [-1.3, 1.3]; the
scan runs the TRUE critical orbit `x <- x² + c` for 120 steps (a deep
escape-time level — the silhouette hugs the real boundary). Camera:
`a₀ = 3 + 0.5c`, `a₁ = 20` poked at slots 121/120, giving

```
z₁ ≈ -(3 + 0.5c)/20 = -0.15 - 0.025c
```

an affine, undistorted copy of M centered at -0.15, 0.065 wide.

## julia_camera (`julia_camera.coeff-program.json`)

The dual: `c` fixed at the Douady-rabbit component (-0.1226+0.7449i),
the SEED sweeps a radius-1.4 disk (t2 winds 101 times —
`634.6017... = 2π·101` — covering the disk densely). The cutout is the
filled Julia set of the rabbit through the same camera.

## Knobs

- The two poked constants place and size the copy: center ≈ `-α/a₁`
  for `a₀ = α + βc`, width = `|β|/a₁ ×` (parameter-box size). Large
  `a₁` keeps the copy affine (branch corrections ~ `|orbit|·|z₁|/a₁`,
  pinned at ≤5e-3 in the suite).
- Scan length sharpens the boundary (escape-time level): 120 shipped,
  ~250 possible within the vector cap.
- Every other root branch paints a further, increasingly distorted
  copy for free — including the reciprocal-world mirror. The bounded
  rows' remaining roots form the usual near-unit-circle cloud, so the
  camera blob coexists with ring structure exactly like the accidental
  original.
- Julia constants worth trying: 0.285+0.01i (near-parabolic),
  -0.8+0.156i (spirals), i (dendrite — zero-area filled set, so the
  cutout becomes a filament silhouette).

Verification (tests/test_camera_programs.py, predeploy-gated): VM
coefficient parity ≤1e-9 vs the exact python mirror, camera prediction
within 5e-3, escape probes proven to overflow within 120 steps.
