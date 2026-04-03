# Solve Score Quantile Spec

This document is the implementation spec for adding a user-controlled clip quantile to the existing `solve_score` color pipeline.

This is a separate concept from the existing render viewport quantile.

The intended implementor is not allowed to reuse the existing viewport quantile control, payload field, or semantics.
If this distinction is blurred in code review, reject the patch.

## 1. Goal

Add a dedicated solve-score clipping parameter:

- UI concept: `solve score clip q`
- user range: `0.1%` to `5.0%`
- meaning: clip solve-score values to the quantile range `[q, 1 - q]` before histogram equalization

This exists because some solve-score renders are effectively monochrome even with 10 bins, which strongly suggests the current clip range is too wide for some score distributions.

The current hardcoded behavior is:

- `quantile_lo = 0.001`
- `quantile_hi = 0.999`

That is equivalent to:

- `q = 0.1%`

The new feature exposes that `q` to the user.

## 2. Non-Negotiable Distinction

This feature has **nothing to do with the existing render viewport quantile**.

Current viewport quantile:

- UI slider in [index.html:300](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L300)
- payload field in [index.html:1224](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1224)
- used by viewport logic in:
  - [lambda/handler_viewport.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_viewport.py)
  - [lambda/shared.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/shared.py)

That quantile controls:

- framing / cropping
- viewport bounds

The new solve-score quantile controls:

- score clipping before histogram equalization
- solve-score color distribution only

These must be separate in:

1. UI control
2. DOM ids
3. JS state / params
4. orchestrator payload
5. backend handler params
6. artifact JSON
7. test names

Do not reuse the generic name:

- `quantile`

for this feature.

The exact new name must be:

- `solve_score_quantile`

## 3. Exact User-Facing Behavior

## 3.1 UI placement

Current solve-score row is at:

- [index.html:364](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L364)

Add a **separate row directly below the existing Solve score row**.

Do not cram the new slider into the same line as:

- color dot
- label
- metric dropdown
- palette circles

That row is already busy and becomes unreadable if the slider is jammed in.

### 3.1.1 Required new row

Add a new row immediately after the solve-score row and before the `Constant` row.

Label text:

- `Score clip q:`

This label is mandatory so the user does not confuse it with the existing viewport `Quantile:` slider.

### 3.1.2 Required DOM ids

Exact ids:

- slider:
  - `render-solve-score-quantile`
- display span:
  - `render-solve-score-quantile-val`

Do not reuse:

- `render-quantile`
- `render-quantile-val`

## 3.2 Slider range and display

Exact slider config:

- `type="range"`
- `min="0.1"`
- `max="5.0"`
- `step="0.1"`
- default `value="0.1"`

The value displayed to the user is percent.

Examples:

- slider value `0.1` means `q = 0.1% = 0.001`
- slider value `1.0` means `q = 1.0% = 0.01`
- slider value `5.0` means `q = 5.0% = 0.05`

The text span must always show one decimal place.

Examples:

- `0.1%`
- `1.0%`
- `5.0%`

## 3.3 Default behavior

Default solve-score clip quantile is:

- `0.1%`

That preserves current behavior exactly.

If the user never touches the slider, the resulting render must be equivalent to the current implementation.

## 3.4 Mode behavior

The slider belongs to solve-score only.

It must not affect:

- `rainbow`
- `proximity` (root proximity)
- `constant`
- bilevel render
- coeff bilevel render
- DeepZoom
- preview generation
- viewport framing

It is acceptable for the slider to remain visible while another color mode is selected, but it must only be included in payloads when:

- `renderColorMode === 'solve_score'`

## 4. Exact Semantics

Let the user-selected slider value be:

- `q_pct` in percent, between `0.1` and `5.0`

Convert it to fractional form:

- `q = q_pct / 100`

Then solve-score clip uses:

- `quantile_lo = q`
- `quantile_hi = 1 - q`

Examples:

- `q_pct = 0.1` -> `[0.001, 0.999]`
- `q_pct = 1.0` -> `[0.01, 0.99]`
- `q_pct = 5.0` -> `[0.05, 0.95]`

This applies only to the **clip phase** of the solve-score prepass.

Histogram and merge phases consume the resulting clip bounds as usual.

## 5. File-by-File Implementation Plan

## 5.1 [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

### 5.1.1 Markup

Current solve-score row:

- [index.html:364](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L364)

Add a new row directly after it.

Required structure:

```html
<div class="color-row">
  <span class="color-row-label">Score clip q:</span>
  <span id="render-solve-score-quantile-val">0.1%</span>
  <input type="range" id="render-solve-score-quantile" min="0.1" max="5.0" step="0.1" value="0.1" ...>
</div>
```

Use the existing render-tab styling patterns.
Do not invent a completely different control style.

### 5.1.2 Label update wiring

The slider must update:

- `render-solve-score-quantile-val`

on `input`.

The display update must be immediate.

### 5.1.3 Common render params

Current common render params are built in:

- [index.html:1153](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1153)

Add:

- `solveScoreQuantile`

to the object returned by `_renderCommonParams()`.

Exact value:

```js
solveScoreQuantile: parseFloat(document.getElementById('render-solve-score-quantile').value) / 100
```

Do not name this field:

- `quantile`
- `score_quantile`
- `clip_q`

The exact internal common-param key must be:

- `solveScoreQuantile`

### 5.1.4 Orchestrator launch payload

Current orchestrator launch is in:

- [index.html:1210](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1210)

Required payload behavior:

When:

- `renderColorMode === 'solve_score'`

include:

- `solve_score_quantile: p.solveScoreQuantile`

When not in solve-score mode:

- do not include `solve_score_quantile`

Do not overload the existing viewport field:

- `quantile`

These two fields must coexist independently in the payload.

## 5.2 [lambda/handler_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py)

Current solve-score phases are around:

- [handler_render_orchestrator.py:304](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py#L304)

### 5.2.1 Read and validate q

Read:

- `solve_score_quantile = rp.get("solve_score_quantile", 0.001)`

Validation rules:

- must be numeric
- must be finite
- must satisfy `0.001 <= q <= 0.05`

If invalid:

- fail fast with a clear error

Do not silently clamp in the orchestrator.
The user should get an explicit failure if the frontend or a mixed-version caller sends nonsense.

### 5.2.2 Clip dispatch payload

Current clip dispatch payload is at:

- [handler_render_orchestrator.py:320](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py#L320)

Add:

- `solve_score_quantile: solve_score_quantile`

### 5.2.3 Hist dispatch payload

Current hist payloads are built at:

- [handler_render_orchestrator.py:340](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py#L340)

Add:

- `solve_score_quantile: solve_score_quantile`

This is mainly for artifact validation and debugging consistency.

### 5.2.4 Merge dispatch payload

Current merge payload is built at:

- [handler_render_orchestrator.py:359](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py#L359)

Add:

- `solve_score_quantile: solve_score_quantile`

### 5.2.5 Raster payload

Current raster payload writes solve-score fields at:

- [handler_render_orchestrator.py:399](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py#L399)

Add:

- `solve_score_quantile: solve_score_quantile`

This is required so raster can validate the bins artifact matches the requested q.

### 5.2.6 Human-readable progress labels

Current phase labels are:

- `Solve score (Metric): clip`
- `Solve score (Metric): hist`
- `Solve score (Metric): merge`

Required update:

include q in the phase label.

Example:

- `Solve score (Clusteriness, q=1.0%): clip`

This is not optional.
Without it, CloudWatch / UI progress gives no clue which q produced the current bins.

## 5.3 [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

Current clip handler hardcodes:

- [handler_solve_proximity.py:95](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py#L95)

### 5.3.1 Clip phase

Required changes:

1. read:
   - `solve_score_quantile = params.get("solve_score_quantile", 0.001)`
2. validate:
   - numeric
   - finite
   - `0.001 <= q <= 0.05`
3. invoke binary with:
   - `--quantile_lo=<q>`
   - `--quantile_hi=<1-q>`

Do not hardcode:

- `0.001`
- `0.999`

in the handler after this change.

### 5.3.2 Artifact JSON

The clip artifact must include:

- `clip_quantile`

Exact example:

```json
{
  "family": "solve_score",
  "version": 1,
  "job_id": "compute_abc",
  "metric": "clusteriness",
  "clip_quantile": 0.01,
  "clip_lo": 0.42,
  "clip_hi": 1.87,
  ...
}
```

### 5.3.3 Hist phase

Hist handler must read:

- `solve_score_quantile`

and include it in the hist artifact JSON as:

- `clip_quantile`

Exact purpose:

- validation in merge
- debugging
- artifact integrity checks

### 5.3.4 Merge phase

Merge handler must:

1. read `solve_score_quantile` from params
2. validate `clip_data["clip_quantile"] == q`
3. validate every hist artifact `clip_quantile == q`
4. write `clip_quantile` into the merged bins artifact

If any artifact has the wrong q:

- fail fast

Do not silently merge mixed-q histograms.

## 5.4 [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)

Current bins validation is at:

- [handler_raster.py:75](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py#L75)

Required changes:

When raster is in solve-score mode:

1. read requested `solve_score_quantile`
2. require bins artifact:
   - `family == "solve_score"`
   - `metric` matches request
   - `clip_quantile` matches request

If bins artifact is missing `clip_quantile`:

- fail fast

Do not default it.

That is exactly how stale or mixed artifacts hide.

## 5.5 [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)

Current clip mode already supports:

- `--quantile_lo`
- `--quantile_hi`

at:

- [solve_proximity_stats.c:149](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c#L149)

No algorithmic change is required in the binary itself for this feature.

However:

1. keep quantile parsing exactly as generic clip controls
2. do not hardcode solve-score quantile defaults in the binary
3. the handler is the correct place to set the UI-selected q

Optional cleanup:

- update help comments so they no longer imply the only expected values are `0.001` / `0.999`

## 5.6 Artifact JSON Requirements

### 5.6.1 Clip artifact

Must contain:

```json
{
  "family": "solve_score",
  "version": 1,
  "job_id": "compute_x",
  "metric": "spread",
  "clip_quantile": 0.01,
  "clip_lo": -0.2,
  "clip_hi": 0.7,
  "n_solves": 3571,
  "degree": 70,
  "lores_bin_key": "renders/compute_x/lores.bin",
  "root_transforms": []
}
```

### 5.6.2 Hist artifact

Must contain:

```json
{
  "family": "solve_score",
  "version": 1,
  "job_id": "compute_x",
  "metric": "spread",
  "clip_quantile": 0.01,
  "stripe_idx": 3,
  "hist_bins": 100,
  "clip_lo": -0.2,
  "clip_hi": 0.7,
  "n_solves": 100000,
  "hist": [...]
}
```

### 5.6.3 Bins artifact

Must contain:

```json
{
  "family": "solve_score",
  "version": 1,
  "job_id": "compute_x",
  "metric": "spread",
  "clip_quantile": 0.01,
  "hist_bins": 100,
  "final_bins": 10,
  "clip_lo": -0.2,
  "clip_hi": 0.7,
  "cuts_norm": [...],
  "n_solves_total": 125000000,
  "root_transforms": []
}
```

## 6. What Must Not Happen

Reject the implementation if any of these occur:

1. the new slider reuses `render-quantile`
2. the payload field is named just `quantile`
3. the solve-score quantile changes viewport framing
4. the viewport quantile changes solve-score clipping
5. the handler still hardcodes `0.001` / `0.999`
6. artifacts do not store `clip_quantile`
7. raster does not validate bins q against request q
8. merge allows mixed-q hist artifacts

## 7. Required Tests

## 7.1 Frontend harness

File:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

Add at least these tests:

1. `render-solve-score-quantile` exists with default `0.1`
2. changing the slider updates `render-solve-score-quantile-val`
3. orchestrator payload includes:
   - `solve_score_quantile`
   - only when `color_mode == 'solve_score'`
4. existing viewport `quantile` and new `solve_score_quantile` can differ simultaneously in payload
5. changing the viewport quantile does not change the solve-score quantile slider value

Do not replace these with DOM-string checks only.
The harness must execute app code and inspect the actual launch payload.

## 7.2 Orchestrator tests

File:

- [tests/test_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_orchestrator.py)

Add at least:

1. solve-score clip payload contains `solve_score_quantile`
2. solve-score hist payload contains `solve_score_quantile`
3. solve-score merge payload contains `solve_score_quantile`
4. raster payload contains `solve_score_quantile`
5. invalid q below `0.001` fails
6. invalid q above `0.05` fails

Use at least one non-default q in these tests, e.g.:

- `0.01`

Otherwise the tests do not prove the new field is really used.

## 7.3 Handler tests

File:

- [tests/test_solve_proximity_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_handler.py)

Add at least:

1. clip subprocess command includes:
   - `--quantile_lo=<q>`
   - `--quantile_hi=<1-q>`
2. clip artifact stores `clip_quantile`
3. hist artifact stores `clip_quantile`
4. merge artifact stores `clip_quantile`
5. merge rejects clip/hist q mismatch

The tests must inspect the actual command args and artifact bodies.

## 7.4 Binary tests

File:

- [tests/test_solve_proximity_stats.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_stats.py)

Add at least:

1. `test_clip_quantile_narrows_range`
   - create enough solves with a broad score distribution
   - compare `q = 0.001` vs `q = 0.05`
   - assert the `0.05` clip range is strictly narrower
2. `test_clip_quantile_preserves_order`
   - verify changing q changes clip bounds, not metric identity
3. one invalid quantile order test at binary level if the binary already meaningfully rejects bad lo/hi

Do not fake this with comments.
The test must call the real Docker ARM64 binary.

## 7.5 Docker runtime regression

File:

- [tests/docker_runtime_regression.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/docker_runtime_regression.py)

Add at least one non-default-q clip smoke:

- `--metric=proximity`, `q=0.05`

and print the resulting clip bounds.

The point is not deep coverage.
The point is to prove the deploy binary accepts non-default quantile inputs under Docker.

## 7.6 Playwright

File:

- [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)

Add at least:

1. solve-score quantile slider is present with `0.1%` default text
2. changing the slider updates the visible text
3. render dispatch payload contains `solve_score_quantile`
4. viewport `quantile` control and solve-score quantile control can hold different values simultaneously

These must be real browser tests, not source inspection.

## 8. Acceptance Criteria

This feature is done only when:

1. the user can set solve-score clip q from `0.1%` to `5.0%`
2. the UI clearly distinguishes it from viewport quantile
3. orchestrator sends `solve_score_quantile` separately from viewport `quantile`
4. clip phase uses `[q, 1-q]`
5. artifacts record `clip_quantile`
6. merge rejects mixed-q artifacts
7. raster rejects bins artifacts with the wrong q
8. frontend harness passes
9. orchestrator tests pass
10. handler tests pass
11. Docker binary tests pass
12. Docker runtime regression passes
13. Playwright solve-score tests pass

## 9. Manual Validation

After implementation, manually validate on a case that currently looks monochrome.

Required manual checks:

1. render with `q = 0.1%`
2. render with `q = 1.0%`
3. render with `q = 5.0%`
4. verify the solve-score image uses more of the palette as q increases
5. verify viewport framing does not change when only `solve_score_quantile` changes
6. verify changing viewport `Quantile:` does not change solve-score clipping unless the solve-score slider is also changed

If increasing q does not change the image on a previously monochrome case, assume wiring is broken until proven otherwise.
