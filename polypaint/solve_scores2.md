# Solve Scores Phase 2 Spec

Status: historical draft superseded by the implemented solve-score pipeline and newer docs such as [solve_scores.md](/Users/nicknassuphis/karpo_hackathon/polypaint/solve_scores.md) and [render_refactor.md](/Users/nicknassuphis/karpo_hackathon/polypaint/render_refactor.md).

This document is the phase-2 extension spec for the existing solve-score system implemented from [solve_scores.md](/Users/nicknassuphis/karpo_hackathon/polypaint/solve_scores.md).

Do not treat this as a brainstorming note.
This is an implementation checklist for extending the current `solve_score` machinery with additional metrics of the same class:

- one scalar per solve
- permutation-invariant in root order
- compatible with the existing lores-clip / hires-hist / merge / raster pipeline
- one color for all roots in a solve

The intended implementor is not allowed to "simplify" the metrics into approximate lookalikes without updating this spec first.

## 1. Goal

Extend the current solve-score family with five new metrics:

1. `clusteriness`
2. `shelliness`
3. `outlierness`
4. `nn_variation`
5. `real_axis_proximity`

Do this by extending the existing generic `solve_score` pipeline.

Do not add new top-level color modes.

The final UI remains:

1. `Rainbow`
2. `Root proximity`
3. `Solve score`
4. `Constant`

Only the solve-score dropdown grows.

## 2. Scope

This phase extends the current v1 metric set:

- `proximity`
- `crowding`
- `spread`
- `anisotropy`
- `area`

with the five new metrics listed above.

The full allowed solve-score metric set after this work must be exactly:

1. `proximity`
2. `crowding`
3. `spread`
4. `anisotropy`
5. `area`
6. `clusteriness`
7. `shelliness`
8. `outlierness`
9. `nn_variation`
10. `real_axis_proximity`

Do not add more than these in this phase.

Explicitly out of scope:

- `orientation`
- `centroid_radius`
- `unit_circle_proximity`
- `real_root_count`
- any metric requiring cross-solve root matching
- any cyclic-hue metric

## 3. Non-Negotiable Design Rules

### 3.1 Reuse the existing generic solve-score pipeline

Do not add:

- `solve_clusteriness`
- `solve_shelliness`
- `solve_outlierness`
- `solve_nn_variation`
- `solve_real_axis_proximity`

as separate color modes, separate radio options, or separate palette rows.

The correct model is still:

- `color_mode = "solve_score"`
- `solve_metric = <one of the 10 values>`

### 3.2 Keep the runtime artifact family unchanged

The artifact family remains:

- `family: "solve_score"`

The artifact key layout remains:

- `renders/{job_id}/solve_scores/{metric}_clip.json`
- `renders/{job_id}/solve_scores/{metric}/stripe_{s}_hist.json`
- `renders/{job_id}/solve_scores/{metric}_bins.json`

Do not invent a second artifact family.

### 3.3 Do not fork the metric implementations

The single source of truth remains:

- [lambda/solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)

Do not implement the new metrics once in:

- [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)

and again differently in:

- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)

All metric formulas must live in the shared header and be called from both binaries.

### 3.4 No approximation shortcuts in phase 2

Do not:

- replace medians with means
- replace nearest-neighbor arrays with the global minimum
- replace standard deviation with max-min spread
- replace `real_axis_proximity` with centroid imaginary part
- map any new metric name to an existing metric implementation

If a metric uses:

- nearest-neighbor distances
- radii from centroid
- medians
- standard deviation

then compute those things honestly.

Degree is small enough that exact per-solve computation is acceptable.

### 3.5 Preserve the current backend compatibility alias only

Legacy alias support remains:

- `color_mode == "solve_proximity"` coerces to:
  - `color_mode = "solve_score"`
  - `solve_metric = "proximity"`

Do not add legacy aliases for the new metrics.

## 4. Exact User-Facing Behavior

## 4.1 Solve-score dropdown

Current dropdown is at [index.html:367](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L367).

It currently has 5 options.

Required final option values and labels, in this exact order:

1. `proximity` / `Proximity`
2. `crowding` / `Crowding`
3. `spread` / `Spread`
4. `anisotropy` / `Anisotropy`
5. `area` / `Area`
6. `clusteriness` / `Clusteriness`
7. `shelliness` / `Shelliness`
8. `outlierness` / `Outlierness`
9. `nn_variation` / `NN variation`
10. `real_axis_proximity` / `Real-axis proximity`

Default remains:

- `proximity`

### 4.2 Palette behavior

No UI change here.

The solve-score palette remains one palette row at:

- [index.html:374](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L374)

Changing the metric dropdown must not:

- change the active palette
- change the root-proximity palette
- change the constant color

### 4.3 Render launch payload

No shape change except wider allowed metric values.

The orchestrator launch in [index.html:1220](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1220) must continue emitting:

- `color_mode`
- `solve_metric`

when solve-score mode is active.

## 5. Exact Metric Definitions

All metrics operate on one solve, meaning one root vector of length `degree`.

Use transformed roots if root transforms are active.

Definitions below must be implemented literally.

### 5.1 Common definitions

For one solve with roots `r_i = (re_i, im_i)`:

- `d2_ij = (re_i - re_j)^2 + (im_i - im_j)^2`
- `d1sq_i = min_{j != i} d2_ij`
- `s1_i = -0.5 * log10(max(d1sq_i, EPS2))`
- `mean_re = average(re_i)`
- `mean_im = average(im_i)`
- `dx_i = re_i - mean_re`
- `dy_i = im_i - mean_im`
- `rho_i = sqrt(dx_i^2 + dy_i^2)`
- `abs_im_i = abs(im_i)`

Constants:

- `EPS2 = 1e-300`
- `EPS = 1e-150`

When the formula below uses:

- median
- mean
- standard deviation

it refers to the ordinary exact finite-sample version over the solve’s roots.

### 5.2 `clusteriness`

Purpose:

- highlight solves where one or two roots are much more crowded than the rest
- distinct from `proximity`, which only tracks the single closest pair
- distinct from `crowding`, which averages all pair crowding

Definition:

1. compute `s1_i` for every root
2. let `s1_med = median_i(s1_i)`
3. let `s1_max = max_i(s1_i)`
4. `score = s1_max - s1_med`

Interpretation:

- larger score = one exceptional local collision relative to typical local spacing

Expected behavior:

- a uniformly dense solve has low `clusteriness`
- a solve with one very tight pair and otherwise ordinary spacing has high `clusteriness`

### 5.3 `shelliness`

Purpose:

- highlight ring-like solves whose roots sit at similar radius from the centroid

Definition:

1. compute centroid `(mean_re, mean_im)`
2. compute `rho_i` for every root
3. let `rho_mean = mean_i(rho_i)`
4. let `rho_std = std_i(rho_i)`
5. `score = -log10(rho_std / (rho_mean + EPS) + EPS)`

Interpretation:

- larger score = thinner shell / tighter ring around the centroid

Expected behavior:

- roots evenly distributed around one radius -> high
- roots filling center + edge radii -> low

### 5.4 `outlierness`

Purpose:

- highlight solves where one root or a small subset sits far away from the rest

Definition:

1. compute centroid `(mean_re, mean_im)`
2. compute `rho_i` for every root
3. let `rho_med = median_i(rho_i)`
4. let `rho_max = max_i(rho_i)`
5. `score = log10((rho_max + EPS) / (rho_med + EPS))`

Interpretation:

- larger score = stronger radial outlier relative to the typical radius

Expected behavior:

- compact cloud with one peeled-away root -> high
- uniform ring or uniform blob -> low

### 5.5 `nn_variation`

Purpose:

- measure heterogeneity of local spacing within a solve
- distinct from `clusteriness` because it cares about overall variation, not just the single worst local crowding value

Definition:

1. compute `s1_i` for every root
2. let `s1_mean = mean_i(s1_i)`
3. let `score = std_i(s1_i)`

Interpretation:

- larger score = stronger mixture of crowded and sparse local neighborhoods

Expected behavior:

- evenly spaced solve -> low
- solve with both tight local clusters and isolated roots -> high

### 5.6 `real_axis_proximity`

Purpose:

- highlight solves whose roots lie near the real axis

Definition:

1. compute `abs_im_i = abs(im_i)` for every root
2. let `im_med = median_i(abs_im_i)`
3. `score = -log10(im_med + EPS)`

Interpretation:

- larger score = solve is closer to the real axis

Expected behavior:

- nearly-real roots -> high
- roots with large imaginary parts -> low

## 6. Shared Helper Requirements

Current shared helper is:

- [lambda/solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)

Current enum/parser only covers 5 metrics around:

- [solve_score.h:23](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h#L23)
- [solve_score.h:31](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h#L31)

Required changes:

1. extend `enum SolveMetric` with the 5 new metrics
2. extend `parse_solve_metric(...)`
3. extend `solve_metric_name(...)`
4. extend `compute_solve_metric_score(...)`

### 6.1 Required helper structure

The file should stop being one long monolithic metric `if` chain once these new metrics are added.

Required internal helper breakdown:

1. nearest-neighbor helper
2. centroid helper
3. exact median helper
4. standard-deviation helper
5. covariance helper (retain existing one for anisotropy/area)

The goal is not beauty.
The goal is to prevent the implementor from duplicating error-prone little formulas in each branch.

### 6.2 Required helper APIs inside the header

These can be `static` internal helpers.

Required helpers:

```c
static void compute_nearest_neighbor_scores(const float *roots, int degree, double *s1_out);
static void compute_centroid(const float *roots, int degree, double *mean_re, double *mean_im);
static void compute_radii_from_centroid(const float *roots, int degree, double mean_re, double mean_im, double *rho_out);
static double median_inplace(double *values, int n);
static double mean_of(const double *values, int n);
static double stddev_of(const double *values, int n, double mean);
```

Implementor may add more helpers, but may not remove the single public API:

```c
double compute_solve_metric_score(const float *roots, int degree, enum SolveMetric metric);
```

### 6.3 Exact median rule

Median must be exact.

For odd `n`:

- middle sorted element

For even `n`:

- arithmetic mean of the two middle sorted elements

Do not use:

- lower median
- upper median
- approximate selection
- mean in place of median

Degree is small enough that an `O(n log n)` sort per solve is acceptable.

### 6.4 Numerical rules

1. All log arguments must be clamped away from zero.
2. `shelliness` must use `rho_std / (rho_mean + EPS) + EPS` exactly as specified.
3. `outlierness` and `real_axis_proximity` must use actual radii / actual absolute imaginary parts, not squared substitutes.
4. `nn_variation` and `clusteriness` must be computed from the nearest-neighbor score vector `s1_i`, not from raw distances and not from pairwise averages.

## 7. File-by-File Implementation Plan

## 7.1 [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Current solve-score dropdown is at:

- [index.html:367](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L367)

Current validation list is at:

- [index.html:832](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L832)

Required changes:

1. add the 5 new `<option>` entries in the exact order from section 4.1
2. extend `const valid = [...]` in `setSolveMetric(name)` to the exact 10 values
3. do not rename:
   - `render-solve-score`
   - `renderSolveMetric`
   - `renderSolveScorePalette`
4. do not add new palette rows
5. do not add new color-dot modes

### 7.1.1 Required frontend invariants

These must remain true after the change:

- selecting any new solve metric activates `solve_score`
- changing solve metric does not alter the active palette
- changing root-proximity palette does not alter solve-score palette
- orchestrator launch includes the exact selected `solve_metric`

## 7.2 [lambda/solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)

This is the primary implementation file for this phase.

Required work:

1. extend enum
2. extend parser / serializer
3. add exact helper functions from section 6
4. implement the 5 new metrics

### 7.2.1 Required implementation order inside the file

Implement in this order:

1. constants
2. enum
3. parser
4. serializer
5. low-level math helpers
6. `compute_solve_metric_score(...)`

This is not about style.
This is to keep the file reviewable and stop the implementor from hiding metric-specific hacks in random branches.

### 7.2.2 Anti-cheat checks for review

During review, reject the patch if any of these are true:

- `clusteriness` does not mention median
- `shelliness` does not compute both mean and standard deviation of radii
- `outlierness` does not compute a median radius
- `nn_variation` is implemented as max-min instead of stddev
- `real_axis_proximity` uses centroid imaginary part or mean imaginary part instead of median absolute imaginary coordinate

## 7.3 [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)

Current metric parsing already exists around:

- [solve_proximity_stats.c:102](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c#L102)

Required changes:

1. no new mode names
2. continue to accept `--metric=<name>`
3. after extending `solve_score.h`, clip/hist modes must accept all 10 metrics
4. no metric-specific code branches should be added in this file

This file must continue to be a caller of the shared header, not a second implementation site.

## 7.4 [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)

Current solve-score metric parsing is around:

- [roots2pix.c:328](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c#L328)

Required changes:

1. no new color modes
2. continue parsing `--solve_metric`
3. after extending `solve_score.h`, raster must accept all 10 metrics
4. invalid metric must continue to fail hard
5. no metric logic duplication here

The solve-score raster path must continue to call the shared helper on already-transformed roots.

## 7.5 [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

Current metric validation is internal to this file.

Required changes:

1. extend the allowed metric set to all 10 names
2. do not leave an outdated 5-metric allowlist
3. do not create metric-specific special cases in the handler
4. artifact family remains:
   - `solve_score`

## 7.6 [lambda/handler_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py)

Current solve-score phases are around:

- [handler_render_orchestrator.py:304](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py#L304)

Required changes:

1. no new phases
2. no new artifact families
3. accept the 5 new metric names unchanged
4. continue putting the metric into:
   - clip payload
   - hist payload
   - merge payload
   - raster payload

Optional but recommended:

- improve phase labels for long names:
  - `Real-axis proximity`
  - `NN variation`

This is optional formatting only.
It must not change payload field names.

## 7.7 [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)

Current bins-family validation is around:

- [handler_raster.py:75](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py#L75)

Required changes:

1. no special-case behavior for the new metrics
2. continue requiring:
   - `family == "solve_score"`
   - bins metric matches requested metric
3. do not add legacy aliases for any new metric names

## 7.8 [docs/lambdas.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/lambdas.md)

Current solve-score docs are around:

- [docs/lambdas.md:144](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/lambdas.md#L144)

Required changes:

1. extend the solve-score metric list from 5 metrics to 10
2. add one concise formula/interpretation line for each new metric
3. document that these are still solve-level scalar metrics using the same 3-phase prepass
4. do not rename Lambda/package names in docs

## 7.9 [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

Current solve-score frontend harness section is around:

- line 877 onward

Required changes:

1. update the solve-score dropdown expectation from 5 options to 10
2. add at least 3 new frontend checks:
   - selecting `clusteriness` updates `renderSolveMetric`
   - selecting `real_axis_proximity` updates `renderSolveMetric`
   - orchestrator launch carries one of the new metrics unchanged
3. keep the existing palette-independence checks

Do not replace real assertions with string search.
The harness must still execute the app code.

## 7.10 [tests/test_solve_proximity_stats.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_stats.py)

This file must be extended heavily.

Required new tests:

1. `test_clusteriness_ranking`
2. `test_shelliness_ranking`
3. `test_outlierness_ranking`
4. `test_nn_variation_ranking`
5. `test_real_axis_proximity_ranking`
6. `test_hist_clusteriness`
7. `test_invalid_new_metric_rejected_by_roots2pix_or_binary` is not needed here if invalid metric coverage already exists, but the existing invalid-metric test must still work with the extended parser

### 7.10.1 Exact ranking fixtures

Use fixtures that make the expected ordering obvious.

#### `clusteriness`

Use two degree-4 solves:

- uniform:
  - `(0,0), (1,0), (2,0), (3,0)`
- one-pair:
  - `(0,0), (0.001,0), (1,0), (2,0)`

Expected:

- `clusteriness(one-pair) > clusteriness(uniform)`

#### `shelliness`

Use two degree-5 solves:

- shell:
  - `(1,0), (0,1), (-1,0), (0,-1), (sqrt(0.5), sqrt(0.5))`
- filled:
  - `(1,0), (0,1), (-1,0), (0,-1), (0,0)`

Expected:

- `shelliness(shell) > shelliness(filled)`

#### `outlierness`

Use two degree-5 solves:

- compact:
  - `(-1,0), (1,0), (0,1), (0,-1), (0.5,0.5)`
- outlier:
  - `(-1,0), (1,0), (0,1), (0,-1), (100,0)`

Expected:

- `outlierness(outlier) > outlierness(compact)`

#### `nn_variation`

Use two degree-4 solves:

- uniform:
  - `(0,0), (1,0), (2,0), (3,0)`
- mixed:
  - `(0,0), (0.001,0), (1,0), (10,0)`

Expected:

- `nn_variation(mixed) > nn_variation(uniform)`

#### `real_axis_proximity`

Use two degree-4 solves:

- near-real:
  - `(0,0.001), (1,-0.001), (2,0.002), (3,-0.002)`
- off-axis:
  - `(0,10), (1,-10), (2,12), (3,-12)`

Expected:

- `real_axis_proximity(near-real) > real_axis_proximity(off-axis)`

### 7.10.2 Required test style

For each new metric ranking test:

1. run each solve alone
2. assert the intended solve has the larger score
3. assert `result["metric"]` matches the metric under test

Do not use only combined min/max aggregate tests.

## 7.11 [tests/test_solve_proximity_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_handler.py)

Required changes:

1. extend metric-validation coverage to at least one new metric in each phase:
   - clip
   - hist
   - merge
2. add one metric-mismatch test using a new metric name
3. add one merge-artifact-family test with a new metric name

Do not leave this file only testing `proximity`, `crowding`, `spread`, `anisotropy`, `area`.

## 7.12 [tests/test_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_orchestrator.py)

Required changes:

1. add one solve-score prepass contract test using a new metric, not one of the original five
2. add one raster payload contract test using a new metric, verifying:
   - `color == "solve_score"`
   - `solve_metric == "<new metric>"`
   - `solve_score_bins_key` contains the metric in its path

Required exact choices:

- use `clusteriness` for one test
- use `real_axis_proximity` for one test

This avoids the implementor "proving" the old 5 metrics still work while forgetting the new ones in one payload path.

## 7.13 [tests/docker_runtime_regression.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/docker_runtime_regression.py)

Current Docker runtime smoke already covers:

- `proximity`
- `crowding`
- `spread`
- roots2pix solve-score smoke

Required additions:

1. clip smoke for `clusteriness`
2. clip smoke for `real_axis_proximity`
3. roots2pix solve-score smoke for one new metric:
   - `--solve_metric=clusteriness`

Do not change the existing `proximity` smoke to the new metric.
Add coverage; do not swap coverage.

## 7.14 [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)

Current browser tests cover the 5-option dropdown and selected metric dispatch.

Required changes:

1. update the dropdown expectation from 5 options to the exact 10 labels in section 4.1
2. add one real browser test that selects `clusteriness` and verifies the orchestrator payload uses it
3. add one real browser test that selects `real_axis_proximity` and verifies the orchestrator payload uses it

Do not just rename the existing spread test.
Add the new coverage.

## 8. Performance Expectations

These new metrics are not all equally expensive.

Expected relative cost classes:

- cheap:
  - `real_axis_proximity`
- medium:
  - `shelliness`
  - `outlierness`
- expensive:
  - `clusteriness`
  - `nn_variation`

because `clusteriness` and `nn_variation` require the full nearest-neighbor score vector.

### 8.1 Required implementation rule for NN-based metrics

If `clusteriness` and `nn_variation` both need `s1_i`, compute the nearest-neighbor vector once per solve for that branch and derive both summary values from it.

Do not:

- recompute nearest-neighbor distances twice in one metric branch
- compute them once for `clusteriness` and again for `nn_variation` inside the same helper path if a shared helper can avoid it

### 8.2 Required benchmark

Before calling the feature complete, run one benchmark on a representative stripe and report:

1. `proximity` runtime
2. `clusteriness` runtime
3. `nn_variation` runtime

This benchmark must use the real deploy-build binary path, not a mocked microbenchmark.

It does not need to block shipping, but it must be reported honestly.

## 9. Acceptance Criteria

This phase is not done until all of the following are true:

1. the solve-score dropdown exposes exactly 10 options in the correct order
2. the frontend emits the new metric names unchanged
3. the shared header parses and serializes all 10 metrics
4. clip/hist/merge artifacts work for the new metrics without any artifact-family change
5. raster accepts the new metrics and fails on invalid metrics
6. Docker binary tests pass for the new metrics
7. Docker runtime regression includes at least one new-metric roots2pix smoke
8. frontend harness passes
9. Playwright solve-score suite is updated to 10-option behavior and new-metric dispatch assertions
10. docs list the full 10-metric solve-score family

## 10. Anti-Cheat Review Checklist

Reject the patch if any of these are true:

1. a new metric name appears in the dropdown but not in `solve_score.h`
2. a new metric name is parsed in Python handlers but not in the C parser
3. the implementation uses means where the spec requires medians
4. `nn_variation` is implemented using pairwise distances instead of nearest-neighbor scores
5. `real_axis_proximity` uses centroid or RMS imaginary part instead of median absolute imaginary part
6. the new tests only assert "it runs" and not the intended ranking direction
7. the Playwright file still expects only 5 options
8. the Docker runtime smoke still only exercises the original metrics
9. the implementor adds separate top-level color modes for the new scores
10. the implementor hides an approximation behind a comment instead of implementing the actual metric

## 11. Manual Validation

After tests pass, manually render at least one image with:

1. `clusteriness`
2. `shelliness`
3. `outlierness`
4. `nn_variation`
5. `real_axis_proximity`

For each one verify:

1. render launches normally
2. solve-score prepass phases run with the correct metric label
3. the final image is not identical to `proximity`
4. the palette is fully used, not collapsed to one color
5. switching between solve metrics does not reset the solve-score palette

If any two of the new metrics produce visibly identical images across several renders, assume a wiring bug first and investigate before declaring success.
