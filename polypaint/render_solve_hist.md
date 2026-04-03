# Render Solve Histogram Debug Plan

This document specifies a **debug-only Solve score histogram action** in the Render tab.

Purpose:

- let the user inspect the current lores solve-score distribution
- make `Score clip q` debuggable without running a full render
- show the actual clip range and score distribution in the Render log
- answer questions like:
  - does `q=5%` actually narrow the clip range?
  - is the score distribution degenerate?
  - are `clip_lo` / `clip_hi` falling back to the full range?
  - is one metric effectively monochrome on this dataset?

This is **not** another render pipeline.

It must be:

- synchronous
- lores-only
- no S3 artifact writes
- no DynamoDB task rows
- no active render observer


## 1. UI

File:

- [`index.html`](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Current Solve score controls are here:

- metric dropdown at [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L367)
- `Score clip q:` slider at [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L382)

### 1.1 New button

Add a button immediately to the right of the `Score clip q:` slider row.

Exact label:

- `Histogram`

Suggested placement:

- same row as `Score clip q:`
- right of the slider
- compact button styling, not a primary action button

Example row shape:

- `Score clip q: 0.1% [slider] [Histogram]`

### 1.2 Visibility and enablement

The button must be visible all the time, but only enabled when all of these are true:

1. `renderColorMode === 'solve_score'`
2. `render-results-dir` is non-empty
3. config URLs are loaded so `lambdaPost()` can call Lambdas

If clicked while disabled state should have prevented it, the handler must still fail safely with a log message, not throw uncaught.

### 1.3 Button behavior

On click:

1. disable the button
2. change button label to `Histogram...`
3. append a start line to `render-log`
4. perform the summary call
5. append formatted results to `render-log`
6. restore button label to `Histogram`
7. re-enable the button

This action must **not**:

- start `_activeRenderRun`
- touch localStorage active render state
- start or stop the render observer
- call the render orchestrator
- call `/clean-render`
- change any render artifacts

### 1.4 Render status text

While the histogram is running:

- set `render-status` to `Computing solve histogram...`

When it completes successfully:

- restore `render-status` to whatever the render observer or UI would normally show
- do not leave a fake permanent status

If it fails:

- set `render-status` to `Solve histogram failed`
- log the actual error to `render-log`


## 2. Frontend Flow

File:

- [`index.html`](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

### 2.1 New helper

Add a dedicated function:

- `runSolveScoreHistogramDebug()`

Do **not** bury this inside `runRasterPipeline()`.

### 2.2 Calc metadata source

The histogram action needs:

- `degree`
- lores `.bin` key

Use the existing storage detail route:

- [`handle_detail()`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py#L514)

It already returns:

- `calc`

and current `calc.json` already contains:

- `degree` at [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2411)
- `lores.bin_key` at [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2415)

Required frontend loading rule:

1. if `_lastCalcMeta` matches the current `job_id`, use it
2. otherwise call:
   - `lambdaPost('storage', { job_id }, '/detail')`
3. require:
   - `detail.calc.degree`
   - `detail.calc.lores.bin_key`
4. if missing, log a clear error:
   - `Histogram failed: calc.json missing lores.bin_key or degree`

Do **not** add a new backend route just to fetch the same calc metadata.

### 2.3 Lambda call

Call the existing solve-score Lambda directly, synchronously:

- `lambdaPost('solve_proximity', body)`

Do **not** use the dispatch Lambda.

Do **not** create a new Lambda just for this feature.

Payload shape:

```json
{
  "phase": "summary",
  "job_id": "<job_id>",
  "degree": <degree>,
  "metric": "<current solve metric>",
  "solve_score_quantile": <current q as 0.001..0.05>,
  "lores_bin_key": "renders/<job_id>/lores.bin",
  "root_transforms": [ ... ]   // optional, current render UI transforms
}
```

Use the same current UI values as the render path:

- metric from `renderSolveMetric`
- quantile from `render-solve-score-quantile`
- root transforms from `_renderCommonParams().rootTransforms`

This is a debug action for the **current** Solve score setup, so it must honor current metric, q, and root transforms.


## 3. Backend Lambda

File:

- [`lambda/handler_solve_proximity.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

### 3.1 New phase

Add a new synchronous phase:

- `summary`

Routing rule:

- `phase == "summary"` -> `handle_summary(params)`

### 3.2 Summary behavior

`handle_summary()` must:

1. validate metric
2. validate quantile in `[0.001, 0.05]`
3. download the provided lores `.bin`
4. optionally write root transforms sidecar
5. invoke the binary in new `--mode=summary`
6. return parsed JSON directly via `ok_response(...)`

### 3.3 No side effects

`summary` must **not**:

- call `report_status()`
- write any S3 object
- write any DynamoDB row
- require `task_id`

This is a synchronous debug summary, not a tracked job.

### 3.4 Timing fields

The response should include:

- `dl_ms`
- `compute_ms`

These are useful to know if the debug action itself is becoming slow.


## 4. Binary

File:

- [`lambda/solve_proximity_stats.c`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)

### 4.1 New mode

Add a new mode:

- `--mode=summary`

This mode must:

1. compute all solve scores for the input lores file
2. sort them
3. compute requested summary stats
4. compute current q-based clip bounds using the same logic as clip mode
5. compute a compact histogram over the **full** score range
6. emit one JSON object to stdout

### 4.2 Do not change clip/hist behavior

Existing modes:

- `clip`
- `hist`

must remain byte-for-byte compatible in behavior.

Do not refactor them in a way that risks breaking the render path.

### 4.3 Required summary JSON fields

The `summary` mode output must include:

- `mode`
  - `"summary"`
- `metric`
- `n_solves`
- `degree`

#### Distribution extremes

- `min_score`
- `max_score`

#### Central tendency / spread

- `mean_score`
- `stddev_score`

#### Quantiles explicitly requested by the user

- `q05`
- `q10`
- `q25`
- `q50`
- `q75`
- `q90`
- `q95`

#### Current q-based clip

- `clip_quantile`
  - the requested `q`
- `clip_lo`
- `clip_hi`
- `full_range`
  - `max_score - min_score`
- `clip_range`
  - `clip_hi - clip_lo`

#### Clip occupancy

- `clip_below_count`
- `clip_inrange_count`
- `clip_above_count`
- `clip_below_frac`
- `clip_inrange_frac`
- `clip_above_frac`

#### Histogram

- `hist_bins`
  - fixed integer, see below
- `hist_full`
  - array of counts over full `[min_score, max_score]`

#### Debug/fallback visibility

- `clip_fallback`
  - boolean
- `clip_fallback_reason`
  - one of:
    - `null`
    - `"small_sample"`
    - `"degenerate_quantile_range"`
    - `"zero_full_range_expanded"`

### 4.4 Histogram bin count

Use:

- `hist_bins = 32`

Reason:

- enough shape to be informative
- still compact enough to print to the log

Do **not** use 100 bins in the log summary.

### 4.5 Quantile definition

Use the same quantile indexing style already used by clip mode:

- `idx = floor((nSolves - 1) * q)`

Do not introduce a second quantile convention here.

### 4.6 Fallback behavior visibility

If current q-based clip falls back to the full range because:

- `nSolves < 100`, or
- `hiIdx <= loIdx`, or
- `clipHi - clipLo < 1e-12`

the summary output must state that explicitly through:

- `clip_fallback`
- `clip_fallback_reason`

This is one of the main reasons the feature exists.


## 5. Stats To Log

The button exists for log output, so the log format must be explicit and useful.

File:

- [`index.html`](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

### 5.1 Required log block

The frontend must write a compact multi-line block to `render-log`.

Required content:

1. header
2. sample count
3. min / max / mean / stddev
4. requested quantiles
5. current q-based clip range
6. clip occupancy
7. fallback note if any
8. ASCII histogram

### 5.2 Exact required stats

The log must show:

- metric name
- `q` as a percentage
- `n_solves`
- degree
- `min_score`
- `max_score`
- `mean_score`
- `stddev_score`
- `q05`
- `q10`
- `q25`
- `q50`
- `q75`
- `q90`
- `q95`
- `clip_lo`
- `clip_hi`
- `clip_range`
- `full_range`
- `clip_below_count` and %
- `clip_inrange_count` and %
- `clip_above_count` and %
- `clip_fallback_reason` if not null
- `dl_ms`
- `compute_ms`

### 5.3 Recommended log format

Use exactly this shape, with numeric formatting at implementor discretion:

```text
Solve histogram: metric=proximity, q=5.0%, n=2500, degree=20
  min=-4.821  p05=-2.114  p10=-1.774  p25=-1.013  median=-0.441  p75=0.168  p90=0.641  p95=0.889  max=1.731
  mean=-0.382  std=0.714
  clip[q..1-q]: lo=-2.114  hi=0.889  width=3.003 / full=6.552
  clip occupancy: below=125 (5.0%)  in=2250 (90.0%)  above=125 (5.0%)
  fallback: none
  lores timings: download=18ms  compute=7ms
  histogram (32 bins, full range):
    00  1 |#
    01  3 |##
    02  7 |####
    ...
    31  2 |#
```

### 5.4 ASCII histogram rules

Use:

- one line per bin
- bin index
- raw count
- bar scaled to the maximum bin count

Do **not** try to render Unicode block art or canvas graphics in this change.

Simple ASCII is correct here.

### 5.5 Log target

Write to:

- `render-log`

Do not create a separate debug panel for this change.


## 6. No Hidden Reuse Of Render Artifacts

This debug action must recompute directly from lores and current UI inputs.

It must **not**:

- reuse `solve_scores/<metric>_clip.json`
- reuse `solve_scores/<metric>_bins.json`
- reuse previous histogram output

Reason:

- this is specifically to debug current metric + q + transform settings
- cached hires artifacts may reflect older settings


## 7. Exact Files To Change

### Frontend

- [`index.html`](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
  - add `Histogram` button beside `Score clip q`
  - add `runSolveScoreHistogramDebug()`
  - add formatter for returned summary JSON into `render-log`
  - reuse `_lastCalcMeta` or fetch `storage /detail`

### Lambda handler

- [`lambda/handler_solve_proximity.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
  - add `phase == "summary"`
  - implement `handle_summary()`

### Binary

- [`lambda/solve_proximity_stats.c`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)
  - add `--mode=summary`
  - compute quantiles/stats/histogram/fallback info

### Tests

- [`tests/test_solve_proximity_stats.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_stats.py)
- [`tests/test_solve_proximity_handler.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_handler.py)
- [`tests/test_frontend_js.sh`](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- [`tests/e2e/render-solve-score.spec.js`](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)


## 8. Tests

### 8.1 Binary tests

Add tests for `--mode=summary`:

1. returns all required top-level fields
2. `n_solves` matches input solve count
3. quantiles are monotone:
   - `min <= q05 <= q10 <= q25 <= q50 <= q75 <= q90 <= q95 <= max`
4. `clip_lo <= clip_hi`
5. `full_range >= clip_range >= 0`
6. `clip_below_count + clip_inrange_count + clip_above_count == n_solves`
7. histogram count sum equals `n_solves`
8. `q=0.05` produces narrower or equal `clip_range` than `q=0.001`
9. fallback flag/reason are emitted correctly on degenerate synthetic inputs

### 8.2 Handler tests

Add tests for `phase=summary`:

1. validates metric
2. validates quantile range
3. downloads provided lores key
4. passes root transforms to binary when present
5. returns parsed JSON directly
6. does not call `report_status()`
7. does not write S3 artifacts

### 8.3 Frontend harness tests

Add tests for:

1. button exists next to `Score clip q`
2. button disabled when color mode is not `solve_score`
3. button payload includes:
   - `phase: "summary"`
   - current metric
   - current `solve_score_quantile`
   - current `root_transforms`
   - lores key from `detail.calc.lores.bin_key`
   - `degree` from `detail.calc.degree`
4. success path appends required summary lines to `render-log`
5. failure path logs a readable error
6. action does not write `_activeRenderRun`
7. action does not call `dispatch`

### 8.4 Playwright

Add browser tests for:

1. button visible beside `Score clip q`
2. button disabled when mode is not `Solve score`
3. clicking button logs summary block with quantiles and clip range
4. changing slider value changes request payload `solve_score_quantile`


## 9. Anti-Cheat Rules

Reject the implementation if any of these are true.

1. The button dispatches the render orchestrator.
2. The button writes any S3 debug artifact.
3. The button writes any DDB status row.
4. The button ignores current root transforms.
5. The button ignores current metric.
6. The button ignores current `Score clip q`.
7. The implementation reuses old `solve_scores/*.json` instead of recomputing lores summary.
8. The implementation logs only `clip_lo/clip_hi` and omits the requested quantiles.
9. The implementation omits fallback visibility.
10. The implementation computes histogram from hires stripe histograms instead of directly from lores scores.


## 10. Manual Validation

After implementation:

1. Select a result dir with valid `calc.json`.
2. Choose `Solve score`.
3. Pick metric `Proximity`.
4. Set `Score clip q` to `0.1%`.
5. Click `Histogram`.
6. Confirm `render-log` shows:
   - metric
   - q
   - `n_solves`
   - quantiles
   - clip range
   - occupancy
   - ASCII histogram
7. Change `q` to `5.0%`.
8. Click `Histogram` again.
9. Confirm the log shows a different `clip_lo` / `clip_hi` or explicitly reports a fallback reason if unchanged.
10. Repeat with a different metric such as `Area` or `Crowding`.

