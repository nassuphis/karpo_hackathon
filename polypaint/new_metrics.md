# New Solve-Score Metrics

Status: revised implementation spec.

## Motivation

The current solve-score set in [solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h) is dominated by internal-shape descriptors:

- pairwise spacing
- nearest-neighbor structure
- covariance shape
- radial spread around the centroid

That works when different parameter pairs change the internal geometry of the root cloud.

It fails when the cloud keeps roughly the same shape but moves through the complex plane. In that case:

- the plotted roots clearly move
- the current metrics stay almost constant
- the histogram collapses into one or two bins
- the render becomes flat color

Example: `compute_mnj3exwe` (degree 70). The root image has obvious spatial structure, but shape-only metrics produce degenerate solve-score histograms.

What is missing is a set of **position-sensitive** solve metrics.

## Design Constraints

These metrics must fit the existing pipeline:

- score is computed per solve in [solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)
- clip/hist/summary are handled by [solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)
- raster and palette chunking then normalize to `u`, apply `omega`, and bin into 10 colors
- metrics must remain viewport-invariant
- metrics should be `O(degree)` per solve

Important consequence:

- metrics should be **linear scalar values**
- circular quantities are risky because the clip/hist pipeline is not circular-aware

Because of that, `centroid_angle` is not recommended for the first implementation batch.

## Recommended First Batch

Implement these 5 first:

1. `centroid_re`
2. `centroid_im`
3. `centroid_dist`
4. `dist_unit_circle`
5. `asymmetry_re`

Why this batch:

- it directly fixes the “rigid cloud moves but shape stays constant” problem
- it avoids circular seam issues
- it avoids stepwise quantization as the first experiment
- all 5 are cheap and easy to reason about

## Deferred Metrics

These are still reasonable, but should not be first:

- `frac_upper`
- `frac_right`
- `frac_quadrant_ul`
- `frac_quadrant_lr`
- `centroid_angle`
- `dist_real_axis`
- `dist_imag_axis`
- `dist_origin`
- `dist_triangle`
- `dist_square`
- `asymmetry_im`

Reasons for deferral:

- `frac_*` metrics are piecewise-constant at the root level and can quantize visibly for low-degree solves
- `centroid_angle` is circular and will create seam artifacts in the current linear clip/hist pipeline
- `dist_triangle` and `dist_square` are fine ideas, but they are more code for less immediate value than the first batch

## Metric Definitions

### 1. `centroid_re`

Real part of the root-cloud centroid.

```text
score = mean(re[i])
```

Properties:

- signed
- position-sensitive
- odd under left-right reflection
- no log transform

This is the cleanest detector for horizontal drift.

### 2. `centroid_im`

Imaginary part of the root-cloud centroid.

```text
score = mean(im[i])
```

Properties:

- signed
- position-sensitive
- odd under top-bottom reflection
- no log transform

This is the cleanest detector for vertical drift.

### 3. `centroid_dist`

Distance of the centroid from the origin.

```text
score = sqrt(mean(re)^2 + mean(im)^2)
```

Properties:

- nonnegative
- rotation-invariant
- useful when the cloud moves on circular or radial trajectories

Transform:

```text
score = log10(centroid_dist + SOLVE_SCORE_EPS)
```

### 4. `dist_unit_circle`

Mean distance of roots from the unit circle.

```text
score = mean(abs(abs(z_i) - 1))
```

Properties:

- nonnegative
- highlights deviation from classical unit-circle structure
- still position-sensitive in many practical cases because rigid translations change `|z|`

Transform:

```text
score = log10(mean(abs(abs(z_i) - 1)) + SOLVE_SCORE_EPS)
```

### 5. `asymmetry_re`

Left-right imbalance of the cloud.

```text
score = abs(mean(re[i])) / (mean(abs(re[i])) + SOLVE_SCORE_EPS)
```

Properties:

- bounded near `[0, 1]`
- zero for clouds symmetric about the imaginary axis
- high when most roots sit on one side

No log transform.

## Explicitly Rejected For First Pass

### `centroid_angle`

Do not add this in the first pass.

Reason:

- `atan2` is circular
- the current clip/hist pipeline treats metrics as linear scalars
- values near `-pi` and `+pi` are close geometrically but far numerically
- this will create seam artifacts and unstable clip ranges

If added later, it should be paired with circular-aware handling, not dropped into the current scalar pipeline unchanged.

### Viewport normalization

Do not normalize centroid metrics against the viewport.

Reason:

- solve scores are explicitly intended to be viewport-invariant
- viewport-based normalization would violate that contract

## Degree Handling

The current score function begins with:

```c
if (degree < 2) return 0.0;
```

in [solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h).

That is acceptable for pairwise and covariance metrics, but it is unnecessarily restrictive for centroid-based metrics.

Implementation requirement:

- change the early-return logic to be metric-specific

Recommended rule:

- pairwise / NN / covariance metrics may still require `degree >= 2`
- centroid and asymmetry metrics should work for `degree >= 1`
- if `degree <= 0`, still return `0.0`

## Exact Implementation Plan

### 1. `lambda/solve_score.h`

Add enum values:

- `SOLVE_METRIC_CENTROID_RE`
- `SOLVE_METRIC_CENTROID_IM`
- `SOLVE_METRIC_CENTROID_DIST`
- `SOLVE_METRIC_DIST_UNIT_CIRCLE`
- `SOLVE_METRIC_ASYMMETRY_RE`

Add parser support in `parse_solve_metric(...)`:

- `"centroid_re"`
- `"centroid_im"`
- `"centroid_dist"`
- `"dist_unit_circle"`
- `"asymmetry_re"`

Add serializer support in `solve_metric_name(...)`.

Refactor `compute_solve_metric_score(...)`:

- replace the blanket `if (degree < 2) return 0.0;`
- allow centroid/asymmetry metrics for `degree >= 1`
- keep the non-finite root guard exactly as-is

Add the new metric branches after centroid computation:

- `centroid_re`: return `mean_re`
- `centroid_im`: return `mean_im`
- `centroid_dist`: return `log10(hypot(mean_re, mean_im) + SOLVE_SCORE_EPS)`
- `dist_unit_circle`: compute mean `fabs(hypot(re, im) - 1.0)` and return `log10(mean + SOLVE_SCORE_EPS)`
- `asymmetry_re`: compute `abs(mean_re) / (mean(abs(re)) + SOLVE_SCORE_EPS)`

Use `hypot(...)` instead of manual `sqrt(x*x + y*y)` where appropriate.

### 2. `lambda/solve_proximity_stats.c`

Update:

- file header metric list
- usage string
- invalid-metric error string

No behavioral change beyond accepting the new names, because all actual score math is delegated to [solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h).

### 3. `lambda/handler_solve_proximity.py`

Extend `VALID_METRICS` with:

- `centroid_re`
- `centroid_im`
- `centroid_dist`
- `dist_unit_circle`
- `asymmetry_re`

### 4. `lambda/handler_palette_render_plan.py`

Also extend `VALID_METRICS` there.

This file is easy to miss, but it independently validates palette metric names.

### 5. `index.html`

Update both solve-score metric dropdowns:

- Render tab solve-score selector
- Palette tab metric selector

Also update the JS-side allowed-metric arrays near the existing metric lists:

- the arrays currently containing `clusteriness`, `shelliness`, etc.

Do not stop at the HTML `<option>` tags only.

## Testing Plan

### 1. `tests/test_solve_proximity_stats.py`

Add ranking tests for the first batch.

Recommended tests:

- `centroid_re`
  - left-shifted cloud vs right-shifted cloud
  - assert right-shifted score > left-shifted score

- `centroid_im`
  - lower cloud vs upper cloud
  - assert upper score > lower score

- `centroid_dist`
  - cloud near origin vs translated outward
  - assert farther cloud has larger score

- `dist_unit_circle`
  - roots exactly on the unit circle vs roots translated off it
  - assert on-circle score is smaller than off-circle score

- `asymmetry_re`
  - symmetric cloud around imaginary axis vs one-sided cloud
  - assert one-sided cloud has larger score

Also add:

- hist smoke for one of the new metrics
- summary smoke for one signed metric (`centroid_re`) and one logged metric (`dist_unit_circle`)
- invalid-metric rejection updated to mention the new names only if the test depends on the full error string

### 2. Frontend tests

Update:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)

Coverage needed:

- selecting each new metric updates dispatch payload
- Render solve-score selector accepts the new values
- Palette metric selector accepts the new values

### 3. Handler tests

Update validation coverage in:

- [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)
- [tests/test_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_render_plan.py)
- [tests/test_solve_proximity_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_handler.py)

At minimum:

- one new metric should pass through render-plan validation
- one new metric should pass through palette-plan validation
- solve-proximity merge path should accept a new metric name

## Nice-to-Have Second Batch

Once the first batch is working, the next additions I would do are:

1. `dist_real_axis`
2. `dist_imag_axis`
3. `dist_origin`
4. `asymmetry_im`
5. one `frac_*` metric, probably `frac_upper`

This keeps the second pass simple while still broadening the space meaningfully.

## Non-Goals

- cross-solve metrics
- metrics that depend on parameter values rather than roots
- viewport-dependent normalization
- circular-metric handling in the first pass
- per-root coloring metrics
