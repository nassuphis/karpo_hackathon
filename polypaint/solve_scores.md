# Solve Scores Spec

Status: partially historical spec. The solve-score system is implemented, but some examples below still reflect earlier migration-stage terminology and older UI references. Current runtime storage is chunk-based and current Render UI behavior is documented in [render_refactor.md](/Users/nicknassuphis/karpo_hackathon/polypaint/render_refactor.md) and [docs/s3results.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/s3results.md).

This document is the implementation spec for generalizing the existing solve-level colorization pipeline into a reusable `solve_score` family.

The intended implementor is not allowed to infer missing requirements.
If something here seems ambiguous, treat that as a spec bug and fix the spec before coding.

This spec is deliberately rigid because the current codebase already has a working `solve_proximity` prepass pipeline, and the easiest way to fake progress here is to rename labels without actually wiring the metric through the backend.

## 1. Goal

Replace the current one-off `Solve proximity` render mode with a generalized solve-level color mode:

- UI label: `Solve score`
- internal color mode emitted by the frontend: `solve_score`
- parameter: dropdown selecting which solve-level score to use

The same 3-phase prepass model must be reused:

1. lores clip
2. hires histogram
3. histogram merge -> equal-density bins
4. hires raster using one color per solve

All roots from one solve still get the same color.

## 2. Scope

Implement the following solve metrics in v1:

1. `proximity`
2. `crowding`
3. `spread`
4. `anisotropy`
5. `area`

Do not implement more than these in v1.

Explicitly deferred:

- `orientation`
- `offcenter`
- `shelliness`
- `unit_circle_proximity`

Those metrics need different color semantics or a different palette model and are out of scope here.

## 3. Non-Negotiable Design Decisions

### 3.1 Reuse the existing prepass machinery

Do not create one top-level color mode per metric.

Do not add:

- `solve_crowding`
- `solve_spread`
- `solve_anisotropy`
- `solve_area`

as separate color dots.

The correct UI is one row:

- `Solve score`
- one dropdown
- one palette circle group

### 3.2 Do not build on dead browser code

Current file:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Current dead helper:

- `_ensureSolveProximityBins()` around lines 1129-1201

That helper is no longer on the active render path.
The active path is the backend render orchestrator launched by `_launchRenderOrchestrator()` around lines 1263-1309.

Do not extend the dead helper.
Delete it and delete tests that only exercise it.

### 3.3 Keep infrastructure names stable in v1

To minimize deploy risk, do not rename the deployed Lambda/package names in v1.

Keep these as-is:

- `polypaint-solve-proximity`
- [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
- [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)

These become implementation details for the generalized solve-score pipeline.

User-facing names, task ids, artifacts, and frontend state should become generic.

## 4. Exact User-Facing Behavior

### 4.1 Color rows

Current color rows are in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) around lines 348-371.

Required final rows:

1. `Rainbow`
2. `Root proximity`
3. `Solve score`
4. `Constant`

The `Solve score` row must contain:

- the color dot
- the label text `Solve score`
- a dropdown with the exact options listed in section 4.2
- one palette circle container

Do not leave the old `Solve proximity` row in the UI.

### 4.2 Dropdown options

Add a `<select>` with id:

- `render-solve-score`

Exact option values and labels:

- `proximity` / `Proximity`
- `crowding` / `Crowding`
- `spread` / `Spread`
- `anisotropy` / `Anisotropy`
- `area` / `Area`

Default:

- `proximity`

### 4.3 Palette behavior

There is exactly one solve-score palette, independent of:

- rainbow matching chips
- root proximity palette
- constant color

So the JS state must have a dedicated solve-score palette variable.

Selecting a solve-score palette circle must:

1. set the solve-score palette
2. activate `solve_score` mode
3. not modify the root-proximity palette

### 4.4 Backward compatibility

For one rollout only, the backend must accept legacy `solve_proximity` requests.

Interpret:

- `color_mode == "solve_proximity"`

as:

- `color_mode = "solve_score"`
- `solve_metric = "proximity"`

The frontend must not emit `solve_proximity` anymore.

This alias exists only to avoid mixed-version breakage during deployment.

## 5. Solve Metrics

All metrics operate on one solve, meaning one root vector of length `degree`.

Use transformed roots if root transforms are active.

### 5.1 Common definitions

For one solve with roots `r_i = (re_i, im_i)`:

- `dx_i = re_i - mean_re`
- `dy_i = im_i - mean_im`
- `d2_ij = (re_i - re_j)^2 + (im_i - im_j)^2`

Constants:

- `EPS2 = 1e-300`

### 5.2 `proximity`

Definition:

- `d2_min = min_{i<j} d2_ij`
- `score = -0.5 * log10(max(d2_min, EPS2))`

Interpretation:

- larger score = closer root collision

This is the existing solve-level metric and must remain bit-for-bit equivalent to current `solve_proximity`.

### 5.3 `crowding`

Definition:

- `M = degree * (degree - 1) / 2`
- `score = (1/M) * sum_{i<j} (-0.5 * log10(max(d2_ij, EPS2)))`

Interpretation:

- larger score = more globally crowded solve

This differs from `proximity` because one close pair is not enough; many pair distances contribute.

### 5.4 `spread`

Definition:

- `mean_re = average(re_i)`
- `mean_im = average(im_i)`
- `r2_mean = (1/degree) * sum_i (dx_i^2 + dy_i^2)`
- `score = 0.5 * log10(max(r2_mean, EPS2))`

Interpretation:

- larger score = larger root cloud

### 5.5 `anisotropy`

Definition:

- `Sxx = (1/degree) * sum_i dx_i^2`
- `Syy = (1/degree) * sum_i dy_i^2`
- `Sxy = (1/degree) * sum_i dx_i * dy_i`
- `trace = Sxx + Syy`
- `det = Sxx * Syy - Sxy * Sxy`
- `disc = sqrt(max(trace * trace - 4 * det, 0))`
- `lambda_max = 0.5 * (trace + disc)`
- `lambda_min = 0.5 * (trace - disc)`
- `score = log10((lambda_max + EPS2) / (lambda_min + EPS2))`

Interpretation:

- larger score = more line-like / elongated solve

### 5.6 `area`

Use the same covariance quantities as `anisotropy`.

Definition:

- `score = 0.5 * log10(max(lambda_max * lambda_min, EPS2))`

Interpretation:

- larger score = larger 2D footprint

This is intentionally different from `spread`:

- `spread` measures overall RMS radius
- `area` penalizes line-like clouds that have large extent in only one direction

## 6. Shared Implementation Rule

Do not duplicate the metric formulas separately in:

- [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)
- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)

Create a new shared header:

- [lambda/solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)

This header must contain:

1. the metric enum
2. metric-name parser
3. metric-name serializer
4. score computation helpers used by both binaries

Required API shape:

```c
enum SolveMetric {
    SOLVE_METRIC_PROXIMITY = 0,
    SOLVE_METRIC_CROWDING = 1,
    SOLVE_METRIC_SPREAD = 2,
    SOLVE_METRIC_ANISOTROPY = 3,
    SOLVE_METRIC_AREA = 4,
};

int parse_solve_metric(const char *s, enum SolveMetric *out);
const char *solve_metric_name(enum SolveMetric m);
double compute_solve_metric_score(const float *roots, int degree, enum SolveMetric metric);
```

If root transforms are needed, either:

- transform first and call the shared score helper on transformed roots

or:

- add a second helper in the same header

Do not maintain two independent metric implementations.

## 7. Artifacts

Replace one-off `solve_proximity*` runtime artifacts with metric-specific solve-score artifacts.

### 7.1 Keys

All artifacts live under:

- `renders/{job_id}/solve_scores/`

Exact keys:

- clip:
  - `renders/{job_id}/solve_scores/{metric}_clip.json`
- per-stripe histogram:
  - `renders/{job_id}/solve_scores/{metric}/stripe_{s}_hist.json`
- merged bins:
  - `renders/{job_id}/solve_scores/{metric}_bins.json`

Examples:

- `renders/compute_x/solve_scores/proximity_clip.json`
- `renders/compute_x/solve_scores/proximity/stripe_0003_hist.json`
- `renders/compute_x/solve_scores/proximity_bins.json`
- `renders/compute_x/solve_scores/anisotropy_bins.json`

### 7.2 Clip artifact JSON

Required fields:

```json
{
  "family": "solve_score",
  "version": 1,
  "job_id": "compute_abc123",
  "metric": "crowding",
  "clip_lo": 0.0,
  "clip_hi": 1.0,
  "n_solves": 3571,
  "degree": 70,
  "lores_bin_key": "renders/compute_abc123/lores.bin",
  "root_transforms": []
}
```

Do not keep `"mode": "solve_proximity"` in these new artifacts.

### 7.3 Hist artifact JSON

Required fields:

```json
{
  "family": "solve_score",
  "version": 1,
  "job_id": "compute_abc123",
  "metric": "crowding",
  "stripe_idx": 3,
  "hist_bins": 100,
  "clip_lo": 0.0,
  "clip_hi": 1.0,
  "n_solves": 100000,
  "hist": [0, 12, 91, "..."]
}
```

### 7.4 Bins artifact JSON

Required fields:

```json
{
  "family": "solve_score",
  "version": 1,
  "job_id": "compute_abc123",
  "metric": "crowding",
  "hist_bins": 100,
  "final_bins": 10,
  "clip_lo": 0.0,
  "clip_hi": 1.0,
  "cuts_norm": [0.12, 0.21, 0.31, 0.44, 0.55, 0.63, 0.74, 0.87, 0.94],
  "n_solves_total": 125000000,
  "root_transforms": []
}
```

## 8. File-by-File Implementation Plan

## 8.1 [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

### UI markup

Current solve row is around lines 365-367.

Required changes:

1. Replace the `Solve proximity` row label with `Solve score`.
2. Change the color-dot mode value from `solve_proximity` to `solve_score`.
3. Add the dropdown:
   - `id="render-solve-score"`
4. Replace palette container id:
   - from `palette-circles-solve-proximity`
   - to `palette-circles-solve-score`
5. Keep exactly one solve-score palette row.

### JS state

Current variables are around lines 779-782.

Replace:

- `renderSolveProximityPalette`

with:

- `renderSolveScorePalette`

Add:

- `let renderSolveMetric = 'proximity';`

### State helpers

Current palette helpers are around lines 809-838.

Required changes:

1. `setPaletteForMode(mode, name)`
   - must support `solve_score`
   - must no longer mention `solve_proximity`
2. `_activeRenderPalette()`
   - if `renderColorMode === 'solve_score'`, return `renderSolveScorePalette`
3. `buildPaletteCircles(...)`
   - rebuild solve-score palette circles for `solve_score`
4. add helper:
   - `function setSolveMetric(name) { ... }`
   - validate against exact allowed values
   - set `renderSolveMetric`
   - activate `solve_score` mode

### Render launch payload

Current orchestrator dispatch is in `_launchRenderOrchestrator()` around lines 1263-1309.

Required payload fields:

- `color_mode: renderColorMode`
- `solve_metric: renderColorMode === 'solve_score' ? renderSolveMetric : undefined`

Do not omit `solve_metric` when `color_mode === 'solve_score'`.

### Remove dead helper

Delete:

- `_ensureSolveProximityBins()`

That code is dead and must not survive this refactor.

Also remove any dead comments or logs that imply the browser runs clip/hist/merge itself.

## 8.2 [lambda/handler_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py)

Current solve-proximity prepass lives around lines 293-349.

Required changes:

1. Accept:
   - `color_mode == "solve_score"`
   - legacy alias `color_mode == "solve_proximity"` -> coerce to `solve_score` + `metric="proximity"`
2. Determine:
   - `solve_metric = rp.get("solve_metric", "proximity")`
3. Rename orchestrator phases to generic names:
   - `solve_score_check`
   - `solve_score_clip`
   - `solve_score_hist`
   - `solve_score_merge`
4. Phase labels must include the chosen metric in human-readable form:
   - `Solve score (Crowding): clip`
   - `Solve score (Anisotropy): hist`
5. Use the new artifact keys in section 7.
6. Clip dispatch payload must include:
   - `metric`
7. Hist dispatch payloads must include:
   - `metric`
8. Merge dispatch payload must include:
   - `metric`
9. Raster jobs must include:
   - `color: "solve_score"`
   - `solve_metric`
   - `solve_score_bins_key`

Do not continue using:

- `solve_proximity_bins_key`

except as a temporary alias read-path in `handler_raster.py`.

## 8.3 [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

Keep the filename and deployed Lambda name in v1.

Required changes:

1. `clip`, `hist`, and `merge` phases must require `metric`.
2. Validate `metric` against the exact allowed set.
3. Pass `--metric=<metric>` to the binary in clip and hist phases.
4. Clip artifact must write:
   - `family: "solve_score"`
   - `metric`
5. Hist artifact must write:
   - `family: "solve_score"`
   - `metric`
6. Merge phase must:
   - validate clip artifact `family == "solve_score"`
   - validate clip artifact `metric == requested metric`
   - validate every stripe histogram `metric == requested metric`
   - reject mixed metrics with a hard failure
7. Output bins artifact must write:
   - `family: "solve_score"`
   - `metric`

Do not silently accept mismatched metric artifacts.

## 8.4 [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)

Keep the filename in v1.

Required changes:

1. Add CLI arg:
   - `--metric=proximity|crowding|spread|anisotropy|area`
2. Default metric:
   - `proximity`
3. Parse metric via `solve_score.h`
4. Replace hardcoded solve-proximity score computation with shared metric computation
5. Emit `"metric": "<name>"` in clip JSON
6. Emit `"metric": "<name>"` in hist JSON
7. Invalid metric must fail with nonzero exit and a readable stderr message

Do not leave the usage string mentioning only solve proximity.

## 8.5 [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)

Current solve-proximity branch is around lines 535-585.

Required changes:

1. Add new color mode name:
   - `solve_score`
2. Keep one-release alias:
   - `solve_proximity` -> same internal solve-score branch with `metric=proximity`
3. Add CLI arg:
   - `--solve_metric=<name>`
4. Replace `--solve_prox_*` args with generic names:
   - `--solve_score_clip_lo`
   - `--solve_score_clip_hi`
   - `--solve_score_cuts`
5. For one release, accept the old arg names as aliases too.
6. The solve-score branch must compute the chosen metric using shared helpers from `solve_score.h`.
7. The palette mapping stays the same:
   - 10 bins
   - one palette sample per bin midpoint
8. Output JSON must include:
   - `"solve_score": true`
   - `"solve_metric": "<name>"`
   - `"palette": "..."`

Do not keep the branch hardcoded to proximity while only changing labels.

## 8.6 [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)

Current solve-proximity arg hookup is around lines 73-83.

Required changes:

1. If `color == "solve_score"`:
   - require `solve_score_bins_key`
2. For one release only:
   - if `color == "solve_proximity"`, accept `solve_proximity_bins_key` and coerce to `solve_score` + `metric=proximity`
3. Download bins JSON and validate:
   - `family == "solve_score"`
   - `metric` present
   - `cuts_norm.length == 9`
4. If request payload includes `solve_metric`, verify it matches bins JSON `metric`
5. Pass to `roots2pix`:
   - `--color=solve_score`
   - `--solve_metric=<metric>`
   - `--solve_score_clip_lo=...`
   - `--solve_score_clip_hi=...`
   - `--solve_score_cuts=...`

Fail fast on mismatch. Do not silently trust stale bins for a different metric.

## 8.7 [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)

Current clean-render logic deletes `solve_proximity/` and two top-level `solve_proximity*.json` artifacts.

Required changes:

1. Delete new solve-score intermediates:
   - prefix `solve_scores/`
2. Continue deleting legacy solve-proximity keys for one release:
   - `solve_proximity/`
   - `solve_proximity_clip.json`
   - `solve_proximity_bins.json`

This is mandatory so rerenders do not pick up stale bins from another metric.

## 8.8 [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)

Infrastructure names stay the same in v1.

Required changes:

1. Build comment strings should no longer describe this Lambda as proximity-only.
2. Packaging remains the same.
3. Docker runtime regression section must run metric-aware tests.

No API route changes are needed for this feature.

## 8.9 Docs

Update:

- [docs/lambdas.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/lambdas.md)

Required changes:

1. Render color mode list must include `solve_score`
2. `solve_proximity` must be documented only as a temporary alias
3. Solve-score prepass docs must describe the metric dropdown and metric-specific artifacts

Add a superseded note to:

- [solve_proximity.md](/Users/nicknassuphis/karpo_hackathon/polypaint/solve_proximity.md)

The note must say:

- `solve_proximity.md` is historical input for the original metric only
- `solve_scores.md` is the current implementation spec

## 9. Required Tests

## 9.1 Binary tests

File:

- [tests/test_solve_proximity_stats.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_stats.py)

These tests must become metric-aware. Do not leave proximity-only tests and claim the other metrics are “covered by the shared helper”.

Add at least:

1. `proximity` reference test
   - existing style, keep
2. `crowding` ranking test
   - two solves with same `d_min` but different overall pair spacing
   - assert higher crowding score for the more globally clustered solve
3. `spread` ranking test
   - same centroid, different scale
   - assert larger spread score for larger cloud
4. `anisotropy` ranking test
   - one isotropic-ish solve and one line-like solve
   - assert line-like score is larger
5. `area` ranking test
   - one small 2D cloud and one large 2D cloud
   - assert large-cloud area score is larger
6. `hist` test for a non-proximity metric
   - e.g. `spread`
   - assert counts sum correctly and JSON includes `"metric": "spread"`
7. invalid metric fails
   - nonzero return code
8. root-transform metric test
   - pick one metric and prove root transforms affect score

## 9.2 Merge-handler tests

File:

- [tests/test_solve_proximity_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_handler.py)

Add:

1. merge output preserves requested metric
2. merge rejects clip artifact with wrong metric
3. merge rejects stripe histogram with wrong metric
4. merge output artifact has `family == "solve_score"`
5. merge output artifact has `cuts_norm` length 9

These tests must execute `handle_merge()`. Do not replace them with source inspection.

## 9.3 Orchestrator tests

File:

- [tests/test_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_orchestrator.py)

Add or replace with:

1. `solve_score` color render runs clip -> hist -> merge before raster
2. clip payload contains `metric`
3. hist payloads contain `metric`
4. merge payload contains `metric`
5. raster payload contains:
   - `color == "solve_score"`
   - `solve_metric == selected metric`
   - `solve_score_bins_key`
6. legacy alias test:
   - input `color_mode == "solve_proximity"`
   - orchestrator coerces to proximity metric

Do not settle for phase-name-only tests. Payload contract assertions are required.

## 9.4 Frontend harness tests

File:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

Required tests:

1. UI contains `Solve score` row and dropdown
2. dropdown options are exactly:
   - proximity
   - crowding
   - spread
   - anisotropy
   - area
3. selecting a solve-score palette activates `solve_score`
4. solve-score palette is independent of root-proximity palette
5. changing dropdown updates `renderSolveMetric`
6. orchestrator dispatch in solve-score mode contains:
   - `color_mode: "solve_score"`
   - `solve_metric: "<selected>"`
7. there is no remaining test coverage for dead `_ensureSolveProximityBins()`

Do not keep passing tests that exercise deleted browser-side prepass code.

## 9.5 Playwright tests

Current file:

- [tests/e2e/render-solve-proximity.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-proximity.spec.js)

Rename it to:

- [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)

Required browser tests:

1. Render tab shows `Root proximity` and `Solve score`
2. Solve-score dropdown is visible with exact option labels
3. selecting a solve-score palette activates solve-score mode
4. selecting a dropdown value updates app state
5. render dispatch payload contains selected `solve_metric`
6. switching root-proximity palette does not change solve-score palette
7. switching solve-score palette does not change root-proximity palette

These are UI tests. They do not replace backend metric tests.

## 9.6 Docker runtime regression

File:

- [tests/docker_runtime_regression.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/docker_runtime_regression.py)

Add:

1. `solve_proximity_stats --metric=proximity` smoke
2. `solve_proximity_stats --metric=crowding` smoke
3. `solve_proximity_stats --metric=spread` smoke
4. one `roots2pix --color=solve_score --solve_metric=proximity ...` smoke

The `roots2pix` smoke can be tiny.
It only needs to prove the generic solve-score branch is wired and the binary accepts the new args.

## 10. Manual Validation Checklist

After deploy:

1. Open Render tab
2. Confirm `Solve score` row exists
3. Confirm dropdown options exactly match spec
4. Render with each metric on the same result directory:
   - Proximity
   - Crowding
   - Spread
   - Anisotropy
   - Area
5. Confirm each run completes
6. Confirm the images differ in meaningful ways
7. Confirm switching metrics without changing palette still changes output
8. Confirm switching palette without changing metric changes output colors only
9. Confirm rerender cleanup removes stale score artifacts
10. Confirm render refresh still works and is fast

## 11. Anti-Cheat Rules

The implementation is wrong if any of the following are true:

1. There is still a top-level `Solve proximity` row instead of a generic `Solve score` row
2. There is one UI row per solve metric
3. The frontend still uses or tests `_ensureSolveProximityBins()`
4. `crowding`, `spread`, `anisotropy`, or `area` all secretly map to the proximity score
5. `roots2pix.c` and `solve_proximity_stats.c` each implement the formulas separately
6. Bins artifacts do not record which metric they belong to
7. Raster accepts bins for one metric while rendering another
8. Tests only assert labels or phase names without inspecting payload contracts or score behavior
9. Legacy `solve_proximity` alias becomes the primary UI path
10. Clean-render leaves stale `solve_scores/` artifacts behind

## 12. Acceptance Criteria

This feature is complete only when all of the following are true:

1. The Render tab emits `color_mode: "solve_score"` with a real `solve_metric`
2. The orchestrator performs metric-aware clip/hist/merge prepass
3. The raster worker consumes metric-aware bins
4. All five metrics produce distinct renders
5. All required test suites are updated and pass
6. Dead browser-side solve-proximity prepass code is removed
7. `solve_proximity` exists only as a temporary backend compatibility alias
