# New Solve-Score Metrics

Status: proposed.

## Motivation

The current 10 solve-score metrics are all **shape descriptors**: they measure internal geometry of the root cloud (nearest-neighbor distances, spread, anisotropy, etc.) but are invariant to where the cloud sits in the complex plane.

This works well when different parameter pairs produce root clouds with different internal structure. But for polynomials where the root cloud maintains a consistent shape while shifting or rotating across the parameter space, every solve gets the same score and the image becomes a single flat color.

Example: `compute_mnj3exwe` (degree 70) — the preview shows clear spatial structure (28K of 252K roots in viewport), but all 10 metrics produce degenerate histograms with 98%+ of solves in one bin. The roots clearly go to different places for different parameter pairs, but the cloud shape is constant.

**What's needed**: metrics that are sensitive to the *position* of the root cloud, not just its internal geometry. These would produce color variation whenever roots move, even if the cloud shape doesn't change.

## Proposed Metrics

### Category 1: Half-plane fractions

Measure what fraction of roots falls in a specific half or quadrant of the complex plane. These are maximally position-sensitive and produce smooth gradients as the root cloud drifts across a boundary.

#### `frac_upper`

Fraction of roots with positive imaginary part.

```
score = count(im[i] > 0) / degree
```

Range: [0, 1]. Score = 0.5 when the cloud is centered on the real axis.

#### `frac_right`

Fraction of roots with positive real part.

```
score = count(re[i] > 0) / degree
```

Range: [0, 1]. Score = 0.5 when the cloud is centered on the imaginary axis.

#### `frac_quadrant_ul`

Fraction of roots in the upper-left quadrant (re < 0, im > 0).

```
score = count(re[i] < 0 && im[i] > 0) / degree
```

Range: [0, 1]. Sensitive to diagonal drift.

#### `frac_quadrant_lr`

Fraction of roots in the lower-right quadrant (re > 0, im < 0).

```
score = count(re[i] > 0 && im[i] < 0) / degree
```

Complementary to `frac_quadrant_ul` — together they detect diagonal asymmetry.

### Category 2: Centroid position

Measure where the center of mass of the root cloud is. These directly track bulk position.

#### `centroid_re`

Real part of the root cloud centroid.

```
score = mean(re[i])
```

Range: unbounded, but in practice bounded by the coefficient magnitudes. Apply log-scale or normalize against viewport.

#### `centroid_im`

Imaginary part of the root cloud centroid.

```
score = mean(im[i])
```

Same properties as `centroid_re` but on the imaginary axis.

#### `centroid_dist`

Distance of the centroid from the origin.

```
score = sqrt(mean(re)^2 + mean(im)^2)
```

Range: [0, ∞). Rotationally symmetric — detects how far the cloud is from the origin regardless of direction.

#### `centroid_angle`

Angle of the centroid from the origin.

```
score = atan2(mean(im), mean(re)) / (2π)
```

Range: [0, 1) after normalization. Creates angular color bands radiating from the origin. Pairs naturally with `centroid_dist` for polar decomposition.

### Category 3: Distance from geometric shapes

Measure how close the root cloud is to a fixed geometric reference. These produce interesting visual patterns because the "distance to shape" varies smoothly and creates contour-like color bands.

#### `dist_unit_circle`

Mean distance of roots from the unit circle.

```
score = mean(| |z_i| - 1 |)
```

Range: [0, ∞). Score = 0 when all roots sit exactly on the unit circle. Many classical polynomials have roots near the unit circle, so this highlights deviations.

Use log scale: `score = log10(mean(| |z_i| - 1 |) + EPS)`

#### `dist_real_axis`

Mean distance of roots from the real axis.

```
score = mean(|im[i]|)
```

Range: [0, ∞). Score = 0 when all roots are real. Highlights how "complex" the root cloud is.

Use log scale: `score = log10(mean(|im_i|) + EPS)`

Note: the existing `real_axis_proximity` metric uses the *median* of `|im|`, not the mean, and applies a different log transform. This mean-based variant may have better spread for certain distributions.

#### `dist_imag_axis`

Mean distance of roots from the imaginary axis.

```
score = mean(|re[i]|)
```

Symmetric counterpart to `dist_real_axis`.

#### `dist_origin`

Mean distance of roots from the origin.

```
score = mean(|z_i|)
```

Range: [0, ∞). Measures the overall scale of the root cloud. Different from `spread` (which measures variance of radii from the centroid, not from the origin).

Use log scale: `score = log10(mean(|z_i|) + EPS)`

#### `dist_triangle`

Mean distance of roots from the boundary of an equilateral triangle inscribed in the unit circle (vertices at the cube roots of unity).

```
For each root z_i:
  d_i = min distance from z_i to the three triangle edges
score = mean(d_i)
```

This creates hexagonal-symmetry color patterns. The triangle geometry interacts with the natural symmetries of many polynomial root clouds.

Use log scale for better spread.

#### `dist_square`

Mean distance of roots from the boundary of the unit square [-1,1] × [-1,1].

```
For each root z_i:
  d_i = min(|re - 1|, |re + 1|, |im - 1|, |im + 1|) if inside
      = distance to nearest edge if outside
score = mean(d_i)
```

Creates rectilinear color contours. Interesting for polynomials with roots that spread out differently along real vs imaginary axes.

### Category 4: Asymmetry measures

Measure how asymmetric the root distribution is along specific axes. These detect rotational or reflective structure.

#### `asymmetry_re`

Imbalance of the root cloud across the imaginary axis.

```
score = |mean(re[i])| / (mean(|re[i]|) + EPS)
```

Range: [0, 1]. Score = 0 for a root cloud perfectly symmetric about the imaginary axis. Score → 1 when all roots are on one side.

#### `asymmetry_im`

Imbalance across the real axis.

```
score = |mean(im[i])| / (mean(|im[i]|) + EPS)
```

Same logic, orthogonal axis.

## Implementation Notes

### Computation cost

All proposed metrics are O(degree) per solve — same as or cheaper than the existing metrics (which include O(degree²) nearest-neighbor computation). The shape-distance metrics require a few extra operations per root but nothing expensive.

### Log scale convention

For unbounded metrics (centroid_dist, dist_* family), apply `log10(score + EPS)` to match the existing convention. The EPS value should be `SOLVE_SCORE_EPS` (currently 1e-150) for consistency.

For fraction metrics (frac_* family), the raw [0,1] range is already bounded and may not need log transformation. However, if the distribution is still concentrated (e.g. fraction is always near 0.5), a logit transform `log10(f / (1-f) + EPS)` could spread the middle.

### Where to add

- `lambda/solve_score.h` — add new metric enum values and computation in `compute_solve_metric_score()`
- `lambda/solve_proximity_stats.c` — add metric name validation
- `lambda/handler_solve_proximity.py` — add to `VALID_METRICS`
- `lambda/handler_render_plan.py` — no change needed (metrics are strings, validation is in solve_proximity)
- `index.html` — add to the solve-score metric dropdown

### Recommended first batch

Start with these 5 — they cover position, shape-distance, and asymmetry with minimal code:

1. `frac_upper` — simplest possible, guaranteed to vary for any non-symmetric polynomial
2. `centroid_dist` — direct position tracking
3. `centroid_angle` — angular decomposition, creates radial color patterns
4. `dist_unit_circle` — classical reference shape
5. `asymmetry_re` — symmetry-breaking detector

These 5 add ~50 lines to `solve_score.h` and would immediately fix the flat-color problem for polynomials like `compute_mnj3exwe`.

## Non-Goals

- Metrics that require cross-solve comparison (e.g. "how different is this solve from the average solve")
- Metrics that depend on the parameter values (x1, x2) rather than the roots
- Viewport-dependent metrics (scores must be viewport-invariant)
- Per-root metrics (these are per-solve aggregate scores)
