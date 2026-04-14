# Solve Proximity Spec

Status: legacy archive. Historical implementation spec preserved for reference; the current user-facing mode is `solve_score`, and histogram artifacts are chunk-based (`chunk_*_hist.json`), not stripe-based.

This document is the implementation spec for a new Render color mode named `solve_proximity`.

The intended audience is an implementor who should not have to infer anything important.
If something here is not explicit enough, the implementor should treat that as a spec bug and fix the spec before coding.

## 1. Goal

Add a new Render color mode:

- UI label: `Solve proximity`
- internal mode name: `solve_proximity`

Definition:

- For each solve (one root vector of length `degree`), compute one scalar:
  - `d_solve = min_{i<j} |r_i - r_j|`
- All roots from that solve get the same color.
- Color is chosen from a discrete 10-bin palette.
- Bins are approximately equal-density across the full hires render, not per stripe and not per solve.

This is intentionally different from the existing `proximity` mode.

## 2. What Current `proximity` Actually Is

Current code in `lambda/roots2pix.c` is not solve-level.

Current behavior:

- file: `lambda/roots2pix.c`
- usage comment: lines `12-18`
- enum: line `268`
- parser: lines `294-306`
- implementation branch: lines `433-506`

Current `proximity` does this:

- for each root inside a solve, compute nearest-neighbor distance to another root in the same solve
- gather global min/max over the entire stripe
- linearly normalize each root’s nearest-neighbor distance against that stripe-global range
- color each root independently

That mode should remain available, but the UI label should become:

- `Root proximity`

Do not silently replace the old algorithm with the new one.
The feature here is a new mode, not a rewrite of the existing one.

## 3. Desired Behavior

`solve_proximity` should behave as follows:

1. For each solve, compute the minimum pairwise root separation.
2. Convert that to a scalar score suitable for binning.
3. Use lores solves to estimate a robust clip range.
4. Use hires solves to build a coarse histogram inside that clipped range.
5. Derive 10 approximately equal-density bins from the hires histogram.
6. Raster every root in a solve with the same palette color corresponding to that solve’s bin.

This is a near-discriminant / near-root-collision proxy.

It is not the actual discriminant.

## 4. Why This Design

We explicitly do **not** want the old stripe-local normalization problem.

We also explicitly do **not** want `t-digest` in v1.

Reasons:

- Only 10 output bins are needed.
- A 100-bin histogram is easy to implement and easy to merge.
- The codebase already has a stripe-oriented async pipeline; a histogram merge fits that model cleanly.
- Deterministic fixed-bin logic is easier to debug than an approximate digest.

## 5. High-Level Pipeline

When the user renders with `renderColorMode === 'solve_proximity'`, do this before normal raster:

1. `clip` phase:
   - read `lores.bin`
   - compute exact lores solve scores
   - compute robust clip bounds from lores quantiles
   - write `solve_proximity_clip.json`

2. `hist` phase:
   - one job per hires stripe
   - read one `stripe_{s}.bin`
   - compute solve scores
   - normalize/clamp using lores clip bounds
   - accumulate a 100-bin histogram
   - write `solve_proximity/stripe_{s}_hist.json`

3. `merge` phase:
   - sum stripe histograms
   - compute 10 equal-density bins
   - write `solve_proximity_bins.json`

4. Raster phase:
   - normal raster jobs run as today
   - each raster stripe receives the final solve-proximity bin spec
   - `roots2pix` colors each solve uniformly

This is intentionally a 3-pass precompute:

- lores clip pass
- hires histogram pass
- hires raster pass

## 6. Exact Metrics

### 6.1 Solve Metric

For one solve with roots `r_0 ... r_{degree-1}`:

- compute
  - `d2_min = min_{i<j} ((re_i - re_j)^2 + (im_i - im_j)^2)`

Do not compute both `(i,j)` and `(j,i)`.
Use `for (i = 0; i < degree; i++) for (j = i + 1; j < degree; j++)`.

### 6.2 Score Space

Do not bin raw distance directly.

Use:

- `score = -0.5 * log10(max(d2_min, EPS2))`

with:

- `EPS2 = 1e-300`

Why:

- avoids `sqrt`
- equivalent to `-log10(d_min)`
- spreads tiny separations across a more useful numeric range

Interpretation:

- larger `score` means roots are closer
- smaller `score` means roots are farther apart

### 6.3 Lores Clip Bounds

Use lores solves to estimate clip bounds.

The lores target is currently set in `index.html`:

- `TARGET_PREVIEW_ROOTS = 250000`
- file: `index.html`
- lines: `2538-2541`

Quantiles to use in v1:

- lower quantile: `0.001` (0.1%)
- upper quantile: `0.999` (99.9%)

Implementation rule:

- sort all lores solve scores ascending
- let `n = number of lores solves`
- `lo_idx = floor((n - 1) * 0.001)`
- `hi_idx = floor((n - 1) * 0.999)`
- `clip_lo = scores[lo_idx]`
- `clip_hi = scores[hi_idx]`

Fallback rules:

- if `n < 100`, use exact min/max instead
- if `hi_idx <= lo_idx`, use exact min/max instead
- if `clip_hi - clip_lo < 1e-12`, use exact min/max instead
- if exact min/max still has width `< 1e-12`, set:
  - `clip_lo = scores[0] - 0.5`
  - `clip_hi = scores[0] + 0.5`

Do not leave a zero-width range.

### 6.4 Hires Histogram

Histogram bin count:

- `HIST_BINS = 100`

For each solve score:

1. normalize:
   - `u = (score - clip_lo) / (clip_hi - clip_lo)`
2. clamp:
   - if `u < 0`, use `0`
   - if `u > 1`, use `1`
3. histogram index:
   - `h = floor(u * HIST_BINS)`
   - if `h == HIST_BINS`, clamp to `HIST_BINS - 1`

### 6.5 Final Equal-Density Bins

Final palette bin count:

- `FINAL_BINS = 10`

Store 9 interior cut points in normalized `[0,1]` space:

- `cuts_norm[0..8]`

These represent boundaries between bins:

- bin 0: `u <= cuts_norm[0]`
- bin 1: `cuts_norm[0] < u <= cuts_norm[1]`
- ...
- bin 8: `cuts_norm[7] < u <= cuts_norm[8]`
- bin 9: `u > cuts_norm[8]`

Derive them from cumulative histogram counts.

For each target quantile `k/10` with `k = 1..9`:

1. find the first histogram bucket whose cumulative count reaches the target
2. interpolate inside that bucket:
   - `frac = (target_count - cum_before_bucket) / bucket_count`
   - `cut = (bucket_index + frac) / HIST_BINS`

Fallback if `bucket_count == 0`:

- use `cut = (bucket_index + 1) / HIST_BINS`

Clamp every cut to `[0,1]`.
Enforce monotonicity:

- if any cut is less than the previous cut, set it equal to the previous cut

## 7. Artifacts to Write

All artifacts live under `renders/{job_id}/`.

### 7.1 Clip Artifact

Key:

- `renders/{job_id}/solve_proximity_clip.json`

JSON shape:

```json
{
  "mode": "solve_proximity",
  "version": 1,
  "job_id": "compute_abc123",
  "score": "-0.5*log10(d2_min)",
  "eps2": 1e-300,
  "quantiles": [0.001, 0.999],
  "clip_lo": 2.74391,
  "clip_hi": 8.12055,
  "n_solves": 3571,
  "degree": 70,
  "lores_bin_key": "renders/compute_abc123/lores.bin",
  "root_transforms": [["unit_circle"]]
}
```

### 7.2 Per-Stripe Histogram Artifact

Key:

- `renders/{job_id}/solve_proximity/stripe_{stripe_idx}_hist.json`

JSON shape:

```json
{
  "mode": "solve_proximity",
  "version": 1,
  "job_id": "compute_abc123",
  "stripe_idx": 3,
  "hist_bins": 100,
  "clip_lo": 2.74391,
  "clip_hi": 8.12055,
  "n_solves": 500000,
  "hist": [12, 19, 44, 103, 201, 0, 0, 1]
}
```

`hist` must always be length 100.

### 7.3 Final Bin Spec

Key:

- `renders/{job_id}/solve_proximity_bins.json`

JSON shape:

```json
{
  "mode": "solve_proximity",
  "version": 1,
  "job_id": "compute_abc123",
  "hist_bins": 100,
  "final_bins": 10,
  "clip_lo": 2.74391,
  "clip_hi": 8.12055,
  "cuts_norm": [0.09, 0.18, 0.29, 0.40, 0.52, 0.63, 0.74, 0.85, 0.94],
  "n_solves_total": 50000000,
  "root_transforms": [["unit_circle"]]
}
```

## 8. New Lambda

Add a new async-dispatched Lambda:

- function name: `polypaint-solve-proximity`
- handler file: `lambda/handler_solve_proximity.py`
- dispatch target name: `solve_proximity`

This Lambda should support 3 phases:

- `clip`
- `hist`
- `merge`

No new API Gateway route is required.
Use the existing `/dispatch` and `/check-status` flow.

## 9. New Binary

Add a new static ARM64 helper binary:

- source: `lambda/solve_proximity_stats.c`
- output binary: `lambda/solve_proximity_stats`

Build command to add in `deploy.sh`:

```bash
aarch64-linux-musl-gcc -O3 -static -o lambda/solve_proximity_stats lambda/solve_proximity_stats.c -lm
```

This binary must include:

- `#include "root_xforms.h"`

Reason:

- solve proximity must be computed on the same transformed roots that the render uses
- root transforms already exist in C in `lambda/root_xforms.h`
- do not reimplement transform behavior differently in Python

## 10. File-by-File Implementation Checklist

This section is the concrete to-do list.

---

## 10A. `index.html`

### Current relevant locations

- color UI block: `index.html:347-375`
- color CSS: `index.html:70-101`
- color state/helpers: `index.html:773-819`
- render param helper: `index.html:1103-1125`
- render orchestration: `index.html:1128-1205`
- lores target and `calcMeta.lores.bin_key`: `index.html:2538-2541`, `index.html:2669-2708`

### Required UI changes

#### 1. Rename the existing label

At `index.html:359-362`:

- change visible text from `Proximity` to `Root proximity`
- keep internal mode name `proximity`

Do not rename the current backend mode string yet.
Only rename the UI label.

#### 2. Add a new row for solve proximity

Insert a new `.color-row` immediately after the existing root-proximity row and before the constant row.

Required contents:

- `data-mode="solve_proximity"` dot
- visible label `Solve proximity`
- a dedicated palette circle container

Use a new container id:

- `palette-circles-solve-proximity`

Rename the old palette container to:

- `palette-circles-root-proximity`

Do not reuse one shared palette row.

#### 3. Increase vertical spacing between color rows

Current CSS:

- `index.html:99-101`
- `.color-row { ... margin-bottom: 6px; ... }`

Change `margin-bottom` from `6px` to `10px`.

That is the entire requested spacing change.
Do not invent more layout churn.

### Required state changes

Current state:

- `renderColorMode`
- `renderMatchMode`
- `renderPalette`

at `index.html:774-776`

Replace the single palette state with two separate palette states:

- `renderRootProximityPalette = 'inferno'`
- `renderSolveProximityPalette = 'inferno'`

Keep:

- `renderColorMode`
- `renderMatchMode`

Do not keep a single shared `renderPalette`.

### Required helper changes

Current helpers:

- `setColorMode()` at `index.html:789-795`
- `setPalette()` at `index.html:802-805`
- IIFE builder at `index.html:807-819`

Replace this with:

#### 1. Keep `setColorMode(mode)` as the active mode setter

No behavioral change besides supporting `solve_proximity`.

#### 2. Replace `setPalette(name)` with a mode-specific setter

Implement:

- `setPaletteForMode(mode, name)`

Behavior:

- if `mode === 'proximity'`, write `renderRootProximityPalette`
- if `mode === 'solve_proximity'`, write `renderSolveProximityPalette`
- then update only the circles inside that mode’s container

#### 3. Replace the one-off palette builder with a reusable builder

Implement:

- `buildPaletteCircles(containerId, mode, getCurrentPalette)`

It must:

- render all `PALETTE_DEFS`
- set `.active` only inside the given container
- on click:
  - call `setPaletteForMode(mode, paletteName)`
  - call `setColorMode(mode)`

This is important.
The current code at `index.html:816` forces `setColorMode('proximity')`.
That must be removed.

#### 4. Build two rows on load

Call the builder twice:

- once for `palette-circles-root-proximity` with mode `proximity`
- once for `palette-circles-solve-proximity` with mode `solve_proximity`

### Required render param changes

Current `_renderCommonParams()` at `index.html:1103-1125` does not return a palette.

Keep it that way if you want.
The active palette can be selected later inside `runRasterPipeline()`.

Add a helper near `_renderCommonParams()`:

```js
function _activeRenderPalette() {
    if (renderColorMode === 'proximity') return renderRootProximityPalette;
    if (renderColorMode === 'solve_proximity') return renderSolveProximityPalette;
    return null;
}
```

### Required calc metadata helper

Current render fallback in `runRasterPipeline()`:

- lines `1171-1177`

This is insufficient for solve proximity, because it only reconstructs:

- `n_stripes`
- `degree`

and does not provide:

- `lores.bin_key`
- `times`
- any other full `calc.json` metadata

Add a frontend helper:

```js
async function _loadCalcMetaForRender(jobId) { ... }
```

Required behavior:

1. if `_lastCalcMeta && _lastCalcMeta.job_id === jobId`, return it
2. else call:
   - `lambdaPost('storage', { job_id: jobId }, '/detail')`
3. require `detail.calc`
4. if missing, throw:
   - `Render metadata missing for ${jobId}; calc.json/lores.bin required for solve proximity`

Do not keep using `/list` for this mode.

### Required render orchestration changes

In `runRasterPipeline()`:

#### 1. Update `colorDesc`

Current code:

- `index.html:1142-1143`

Replace with a 4-way description:

- `constant/#hex`
- `proximity/<root palette>`
- `solve_proximity/<solve palette>`
- `rainbow/<match>`

#### 2. Replace the current `calcMeta` fallback

Current code:

- `index.html:1171-1177`

Replace with:

- `const calcMeta = await _loadCalcMetaForRender(p.jobId);`

Use `calcMeta.n_stripes` and `calcMeta.degree`.

#### 3. Add solve-proximity prepass before raster jobs

Between:

- viewport resolution
- raster job creation

insert:

```js
let solveProximityBinsKey = null;
if (renderColorMode === 'solve_proximity') {
    solveProximityBinsKey = await _ensureSolveProximityBins({
        jobId: p.jobId,
        calcMeta,
        rootTransforms: p.rootTransforms,
        degree: calcMeta.degree,
        nStripes: calcMeta.n_stripes,
        logTarget: 'render-log',
    });
}
```

#### 4. Add the bins key to every raster job

When building `rasterJobs` at `index.html:1181-1194`, add:

- `solve_proximity_bins_key: solveProximityBinsKey` only when color mode is `solve_proximity`

#### 5. Choose palette from active mode

Current raster payload uses:

- `palette: renderPalette`

Replace with:

- `palette: _activeRenderPalette() || undefined`

### Required new frontend helper

Add:

- `_ensureSolveProximityBins(...)`

Place it near other render orchestration helpers, not buried in unrelated preview code.

Required behavior:

#### Clip phase

1. delete stale task:
   - `/delete-task` with task_id `solve_proximity_clip`
2. dispatch one job:
   - `target: 'solve_proximity'`
   - job:
     - `phase: 'clip'`
     - `job_id`
     - `degree`
     - `lores_bin_key: calcMeta.lores.bin_key`
     - `root_transforms`
     - `out_key: renders/${jobId}/solve_proximity_clip.json`
     - `task_id: 'solve_proximity_clip'`
3. poll `/check-status`:
   - `task_prefix: 'solve_proximity_clip'`
   - `expected: 1`
4. on error, throw with the actual `error_details[0].error_msg`

#### Hist phase

1. build one job per stripe:
   - `phase: 'hist'`
   - `job_id`
   - `stripe_idx`
   - `bin_key: renders/${jobId}/stripe_${s}.bin`
   - `degree`
   - `clip_key: renders/${jobId}/solve_proximity_clip.json`
   - `hist_bins: 100`
   - `root_transforms`
   - `out_key: renders/${jobId}/solve_proximity/stripe_${s}_hist.json`
   - `task_id: solve_proximity_hist_${s}`
2. dispatch via existing `_bilevelDispatchAndPoll(...)`
3. use:
   - `taskPrefix: 'solve_proximity_hist_'`
   - `target: 'solve_proximity'`
   - `label: 'Solve proximity hist'`

#### Merge phase

1. delete stale task:
   - `/delete-task` with task_id `solve_proximity_merge`
2. dispatch one job:
   - `phase: 'merge'`
   - `job_id`
   - `n_stripes`
   - `hist_prefix: renders/${jobId}/solve_proximity/`
   - `clip_key: renders/${jobId}/solve_proximity_clip.json`
   - `out_key: renders/${jobId}/solve_proximity_bins.json`
   - `task_id: 'solve_proximity_merge'`
3. poll `/check-status`:
   - `task_prefix: 'solve_proximity_merge'`
   - `expected: 1`
4. return:
   - `renders/${jobId}/solve_proximity_bins.json`

#### Logging

Log all three stages into `render-log`:

- `Solve proximity: clip...`
- `Solve proximity: hist...`
- `Solve proximity: merge...`

Do not make the user guess where time is going.

---

## 10B. `lambda/handler_storage.py`

### Current relevant locations

- `handle_clean_render()` at `lambda/handler_storage.py:321-404`
- `handle_detail()` at `lambda/handler_storage.py:467-494`

### Required changes to `handle_detail()`

Current `/detail` only returns:

- `file_count`
- `q_re`
- `q_im`

That is not enough.

Extend `handle_detail()` so it also tries to load:

- `renders/{job_id}/calc.json`

and returns:

```json
{
  "job_id": "...",
  "file_count": 123,
  "q_re": [...],
  "q_im": [...],
  "calc": {
    "degree": 70,
    "n_stripes": 5000,
    "n_chunks": 5000,
    "times": 1,
    "lores": {
      "bin_key": "renders/.../lores.bin",
      "N": 60,
      "n_steps": 3600
    },
    "pipeline": {...}
  }
}
```

Minimum required fields inside `calc`:

- `degree`
- `n_stripes`
- `n_chunks`
- `times`
- `lores`
- `pipeline`

It is fine to return the whole `calc.json` object.
That is simpler and safer than inventing a second partial schema.

### Required changes to `handle_clean_render()`

Current code only deletes prefixed intermediates:

- `pix_`
- `raw_`
- `tile_`
- `bilevel_t`
- `coeff_t`

at `lambda/handler_storage.py:331-338`

Extend cleanup to also remove solve-proximity intermediates.

Required deletions:

#### Prefix-based

Add prefix:

- `solve_proximity/`

This clears:

- `renders/{job_id}/solve_proximity/stripe_*_hist.json`

#### Explicit top-level keys

Also delete if present:

- `renders/{job_id}/solve_proximity_clip.json`
- `renders/{job_id}/solve_proximity_bins.json`

Do not rely on prefix deletion for these, because they are top-level keys.

### Required tests for storage

Add tests in `tests/test_pipeline.py`:

1. `/detail` returns `calc.lores.bin_key` when `calc.json` exists
2. `/detail` still works if `calc.json` is missing
3. `clean_render` deletes `solve_proximity/stripe_0000_hist.json`
4. `clean_render` deletes `solve_proximity_clip.json`
5. `clean_render` deletes `solve_proximity_bins.json`
6. `clean_render` still preserves:
   - `calc.json`
   - `stripe_0.bin`
   - final images

---

## 10C. `lambda/handler_dispatch.py`

### Current relevant location

- `FUNCTIONS` map at `lambda/handler_dispatch.py:22-33`

### Required change

Add one new target:

```python
"solve_proximity": os.environ.get("SOLVE_PROXIMITY_FUNCTION", "polypaint-solve-proximity"),
```

Do not hardwire the function name without an env variable.

### Required tests

Add dispatch target tests in:

- `tests/test_pipeline.py`
- `tests/test_dispatch_resilience.py`

Required coverage:

1. `target='solve_proximity'` fires jobs
2. it invokes the correct function name
3. non-202 responses are surfaced in `non_202`

---

## 10D. `lambda/handler_solve_proximity.py` (new file)

Create this file.

This is the async-dispatched Lambda for the 3 precompute phases.

### Environment

Needs:

- `BUCKET`
- `JOBS_TABLE`

No layer required.
No libvips required.

### Binary used

Use:

- `solve_proximity_stats`

from the same package directory.

### Phase routing

At top of handler:

```python
phase = params["phase"]
if phase == "clip": ...
elif phase == "hist": ...
elif phase == "merge": ...
else: raise RuntimeError(...)
```

### Common status behavior

Use `report_status()` from `lambda/shared.py`.

Status names:

- `started`
- `bin_downloaded`
- `computed`
- `merged`
- `done`
- `error`

Task IDs are provided by the frontend job payload.
Do not invent different task ids server-side.

### `/tmp` hygiene

Before work begins for each phase:

- delete any stale phase-specific temp files this handler owns

Use a private namespace only.

Recommended:

- `/tmp/solve_prox_input.bin`
- `/tmp/solve_prox_root_xforms.json`
- `/tmp/solve_prox_clip.json`
- `/tmp/solve_prox_hist.json`

Do not wipe all of `/tmp`.

### Phase: `clip`

Input payload:

- `phase`
- `job_id`
- `degree`
- `lores_bin_key`
- `root_transforms`
- `out_key`
- `task_id`

Steps:

1. report `started`
2. download `lores_bin_key` to `/tmp/solve_prox_input.bin`
3. if `root_transforms` present:
   - write `/tmp/solve_prox_root_xforms.json`
4. run binary:

```bash
solve_proximity_stats /tmp/solve_prox_input.bin \
  --mode=clip \
  --degree=<degree> \
  --quantile_lo=0.001 \
  --quantile_hi=0.999 \
  [--root_xforms=/tmp/solve_prox_root_xforms.json]
```

5. parse stdout JSON
6. upload it to `out_key`
7. report `done` with result_data containing:
   - `out_key`
   - `n_solves`
   - `clip_lo`
   - `clip_hi`

### Phase: `hist`

Input payload:

- `phase`
- `job_id`
- `stripe_idx`
- `bin_key`
- `degree`
- `clip_key`
- `hist_bins`
- `root_transforms`
- `out_key`
- `task_id`

Steps:

1. report `started`
2. download stripe `.bin`
3. download `clip_key`
4. write transforms sidecar if present
5. run binary:

```bash
solve_proximity_stats /tmp/solve_prox_input.bin \
  --mode=hist \
  --degree=<degree> \
  --clip_lo=<clip_lo> \
  --clip_hi=<clip_hi> \
  --hist_bins=100 \
  [--root_xforms=/tmp/solve_prox_root_xforms.json]
```

6. parse stdout JSON
7. upload to `out_key`
8. report `done` with result_data containing:
   - `stripe_idx`
   - `out_key`
   - `n_solves`

### Phase: `merge`

Input payload:

- `phase`
- `job_id`
- `n_stripes`
- `hist_prefix`
- `clip_key`
- `out_key`
- `task_id`

Steps:

1. report `started`
2. download `clip_key`
3. load all expected stripe hist JSONs:
   - keys:
     - `renders/{job_id}/solve_proximity/stripe_{s}_hist.json`
   - for `s in [0, n_stripes)`
4. fail if any histogram key is missing
5. sum the 100-bin histograms elementwise
6. derive `cuts_norm`
7. upload final JSON to `out_key`
8. report `done` with result_data containing:
   - `out_key`
   - `n_solves_total`
   - `cuts_norm`

### Error handling

On any exception:

- `report_status(job_id, task_id, "error", str(e), result_data=progress_context)`
- re-raise

Mirror the style of:

- `lambda/handler_render_preview.py:75-168`

That handler is the correct example for async status reporting plus temp cleanup.

### Required tests

Add `tests/test_solve_proximity_handler.py`.

Cover at minimum:

1. `clip` phase:
   - downloads lores
   - runs subprocess with `--mode=clip`
   - uploads JSON
   - reports done
2. `hist` phase:
   - downloads stripe and clip
   - passes `--clip_lo`, `--clip_hi`, `--hist_bins=100`
   - uploads JSON
3. `merge` phase:
   - loads all expected hist files
   - merges counts correctly
   - writes 9 cuts
4. error path:
   - reports `error`
   - includes progress context

---

## 10E. `lambda/solve_proximity_stats.c` (new file)

Create this file.

This file is the authoritative numeric implementation for solve-proximity scoring.

### Responsibilities

Support 2 modes:

- `--mode=clip`
- `--mode=hist`

### Input format

Same raw `.bin` format as `roots2pix`:

- one solve = `degree` roots
- each root = `(re, im)` float32 pair
- therefore:
  - `stride = degree * 2`
  - `nSolves = fileSize / (stride * sizeof(float))`

### Shared transform behavior

This binary must reuse the existing root transform code:

- include `root_xforms.h`
- parse `--root_xforms=...`
- apply transforms exactly like `lambda/roots2pix.c:396-414`

That is not optional.

### CLI

Required usage:

```text
solve_proximity_stats stripe.bin --mode=clip --degree=D [--quantile_lo=0.001] [--quantile_hi=0.999] [--root_xforms=...]
solve_proximity_stats stripe.bin --mode=hist --degree=D --clip_lo=X --clip_hi=Y --hist_bins=100 [--root_xforms=...]
```

### Mode: `clip`

Algorithm:

1. read full root buffer
2. apply root transforms in-place if present
3. for each solve:
   - compute `d2_min`
   - compute `score = -0.5 * log10(max(d2_min, 1e-300))`
   - append to score array
4. sort score array ascending
5. compute clip bounds with fallback rules from section 6.3
6. print JSON to stdout

Output JSON keys:

- `mode`
- `n_solves`
- `degree`
- `score`
- `clip_lo`
- `clip_hi`
- `min_score`
- `max_score`

### Mode: `hist`

Algorithm:

1. read full root buffer
2. apply root transforms in-place if present
3. initialize 100-bin histogram
4. for each solve:
   - compute `d2_min`
   - compute `score`
   - normalize/clamp to `u`
   - increment histogram bucket
5. print JSON to stdout

Output JSON keys:

- `mode`
- `n_solves`
- `degree`
- `hist_bins`
- `clip_lo`
- `clip_hi`
- `hist`

### Required performance rules

Do not do unnecessary work.

Specifically:

- use `i < j`, not `i != j`
- do not compute `sqrt`
- do not allocate per-solve heap objects
- do not parse JSON inside the inner loops

### Required failure handling

Reject:

- invalid degree
- empty file
- invalid clip range in `hist` mode
- malformed root transform file

Write human-readable errors to stderr.

### Required tests

Add `tests/test_solve_proximity_stats.py`.

This test file should run on host Python and validate the algorithmic contract at a small scale.

Required cases:

1. exact clip on a tiny synthetic file with known solves
2. exact histogram counts with known clip bounds
3. root transform passthrough smoke test if feasible
4. degenerate identical-root solve does not crash

Keep the synthetic files tiny and exact.

---

## 10F. `lambda/handler_raster.py`

### Current relevant location

- command construction: `lambda/handler_raster.py:47-71`

### Required change

When `params["color"] === "solve_proximity"`:

1. require `params["solve_proximity_bins_key"]`
2. download that JSON to `/tmp/solve_proximity_bins.json`
3. parse:
   - `clip_lo`
   - `clip_hi`
   - `cuts_norm`
4. pass them to `roots2pix` via CLI args

Required new CLI args:

- `--solve_prox_clip_lo=<float>`
- `--solve_prox_clip_hi=<float>`
- `--solve_prox_cuts=<comma-separated 9 floats>`

Do not make `roots2pix` parse JSON.
Do the JSON parsing in Python and pass primitive CLI args.

### Required cleanup

Delete:

- downloaded stripe bin
- root transform sidecar if written
- solve proximity bins temp file if written

### Required tests

Add/extend tests for `handler_raster.py` in `tests/test_pipeline.py` or a dedicated test file:

1. when color is `solve_proximity`, handler downloads the bins JSON
2. it passes all 3 new CLI args to `roots2pix`
3. when color is not `solve_proximity`, it does not pass them

---

## 10G. `lambda/roots2pix.c`

### Current relevant locations

- usage comment: `lambda/roots2pix.c:12-18`
- enum: `lambda/roots2pix.c:268`
- usage string: `lambda/roots2pix.c:273-278`
- arg parse: `lambda/roots2pix.c:294-306`
- proximity implementation: `lambda/roots2pix.c:433-506`
- dedup comment: `lambda/roots2pix.c:419-421`
- output JSON: `lambda/roots2pix.c:640-650`

### Required changes

#### 1. Extend usage text

Update both top comment and runtime usage text to include:

- `solve_proximity`
- `--solve_prox_clip_lo`
- `--solve_prox_clip_hi`
- `--solve_prox_cuts`

#### 2. Extend enum

Add:

```c
COLOR_SOLVE_PROXIMITY = 3
```

Do not renumber existing modes in a way that breaks anything else.
Appending is fine.

#### 3. Parse new color string

Extend:

- `if (strcmp(colorStr, "solve_proximity") == 0) ...`

#### 4. Parse new CLI args

Add:

- `solveProxClipLo`
- `solveProxClipHi`
- `solveProxCutsCsv`

Parse the cuts CSV into an array:

- `double solveProxCuts[9]`

Reject malformed CSV.

#### 5. Add new branch

Add a new branch before constant/rainbow:

```c
else if (colorMode == COLOR_SOLVE_PROXIMITY) { ... }
```

#### 6. Inside solve-proximity branch

For each solve `p`:

1. compute one `d2_min` using `i < j`
2. compute one `score = -0.5 * log10(max(d2_min, 1e-300))`
3. normalize using `solveProxClipLo`, `solveProxClipHi`
4. clamp to `[0,1]`
5. convert normalized value to final bin using `solveProxCuts`
6. choose one RGB for the solve
7. emit every root in the solve with that same RGB

#### 7. Precompute the 10 palette colors once

At branch entry:

- precompute RGB for bins `0..9`
- use palette centers:
  - `t = (bin + 0.5) / 10.0`

Example:

```c
RGB solvePal[10];
for (int b = 0; b < 10; b++) {
    paletteRGB(proxPal, (b + 0.5) / 10.0, &solvePal[b].r, &solvePal[b].g, &solvePal[b].b);
}
```

Do not call `paletteRGB()` for every root when the solve color is constant.

#### 8. Keep dedup behavior unchanged

Do not change the current first-hit-wins pixel dedup behavior.

That behavior is documented at:

- `lambda/roots2pix.c:419-421`

This feature changes color assignment, not raster semantics.

#### 9. Output metadata

In the final JSON emitted by `roots2pix`, add solve proximity metadata when active:

- `"palette":"<name>"`
- `"solve_proximity":true`

No need to print clip/cut arrays here.

### Required test notes

The easiest trustworthy validation is to build a tiny synthetic stripe and assert:

- all roots in one solve produce the same RGB
- two solves with different `d2_min` can land in different bins
- `cuts_norm` boundary behavior is correct

This belongs either in a dedicated unit test around a host-buildable helper or in the Docker runtime regression section.

---

## 10H. `deploy.sh`

### Current relevant locations

- function names / memory constants: `deploy.sh:18-66`
- compile section: `deploy.sh:234-239` and surrounding binary build block
- package section: `deploy.sh:484-588`
- create path: `deploy.sh:903-949`
- update path: `deploy.sh:1006-1052`

### Required changes

#### 1. Add function name + memory constant

At the top with other names:

- `SOLVE_PROXIMITY_NAME="polypaint-solve-proximity"`
- `SOLVE_PROXIMITY_MEMORY=1769`

Use `BINARY_TMP` for ephemeral storage.

#### 2. Compile the new binary

In the binary compile section, add:

```bash
echo "  solve_proximity_stats (static, ARM64)..."
aarch64-linux-musl-gcc -O3 -static -o lambda/solve_proximity_stats lambda/solve_proximity_stats.c -lm
```

#### 3. Add a package section

Create `/tmp/polypaint-solve-proximity.zip` containing:

- `lambda/handler_solve_proximity.py`
- `lambda/shared.py`
- `lambda/solve_proximity_stats`

Mark the binary executable.

#### 4. Create path

Add a `create_lambda` call for:

- `polypaint-solve-proximity`
- handler: `handler_solve_proximity.handler`
- no layer
- env: `BUCKET`, `JOBS_TABLE`
- tmp: `$BINARY_TMP`

#### 5. Update path

Add an `update_lambda` call for the new function.

#### 6. Dispatch env

Extend the dispatch Lambda env string in both create and update sections with:

- `SOLVE_PROXIMITY_FUNCTION=$SOLVE_PROXIMITY_NAME`

#### 7. Async invoke config

Add `"$SOLVE_PROXIMITY_NAME"` to the no-retry async invoke config loop.

It should match raster/finalize/render_preview behavior:

- `--maximum-retry-attempts 0`
- `--maximum-event-age-in-seconds 300`

### Required runtime regression test

In `scripts/test-docker-runtime.sh`, add a section for the new binary.

It must run inside the ARM64 Docker environment, not on host macOS.

Required smoke:

1. verify `/src/solve_proximity_stats` exists and is ARM64 ELF
2. create a tiny synthetic `.bin`
3. run `--mode=clip`
4. run `--mode=hist`
5. assert valid JSON and expected histogram counts

This is mandatory.
Do not add a new binary without a deploy/runtime smoke.

---

## 10I. `docs/lambdas.md`

### Current relevant location

- raster section begins at `docs/lambdas.md:105`

### Required updates

#### 1. Raster color modes

Update the raster input docs:

- `color` can be `"rainbow"`, `"proximity"`, `"solve_proximity"`, or `"constant"`

Update the color mode table:

- `proximity` = root-level nearest-neighbor distance, stripe-normalized
- `solve_proximity` = solve-level min pair distance, equal-density binned

#### 2. Add new lambda section

Document:

- `polypaint-solve-proximity`
- handler file
- binary
- phases: clip / hist / merge
- output artifacts

---

## 11. Exact Frontend Job Payloads

These payload shapes should be copied exactly.

### 11.1 Clip job

```json
{
  "phase": "clip",
  "job_id": "compute_abc123",
  "degree": 70,
  "lores_bin_key": "renders/compute_abc123/lores.bin",
  "root_transforms": [["unit_circle"]],
  "out_key": "renders/compute_abc123/solve_proximity_clip.json",
  "task_id": "solve_proximity_clip"
}
```

### 11.2 Hist job

```json
{
  "phase": "hist",
  "job_id": "compute_abc123",
  "stripe_idx": 42,
  "bin_key": "renders/compute_abc123/stripe_42.bin",
  "degree": 70,
  "clip_key": "renders/compute_abc123/solve_proximity_clip.json",
  "hist_bins": 100,
  "root_transforms": [["unit_circle"]],
  "out_key": "renders/compute_abc123/solve_proximity/stripe_42_hist.json",
  "task_id": "solve_proximity_hist_42"
}
```

### 11.3 Merge job

```json
{
  "phase": "merge",
  "job_id": "compute_abc123",
  "n_stripes": 5000,
  "hist_prefix": "renders/compute_abc123/solve_proximity/",
  "clip_key": "renders/compute_abc123/solve_proximity_clip.json",
  "out_key": "renders/compute_abc123/solve_proximity_bins.json",
  "task_id": "solve_proximity_merge"
}
```

### 11.4 Raster job extension

For normal raster jobs, add this only when `color === 'solve_proximity'`:

```json
{
  "solve_proximity_bins_key": "renders/compute_abc123/solve_proximity_bins.json"
}
```

## 12. Anti-Goals

Do **not** do any of the following in this feature:

- do not replace existing `proximity`
- do not make `solve_proximity` per-stripe normalized
- do not make `solve_proximity` per-solve normalized
- do not use one shared palette row for root and solve proximity
- do not parse JSON inside `roots2pix`
- do not use `t-digest` in v1
- do not add support to bilevel render modes
- do not change first-hit-wins dedup behavior
- do not require a brand new API Gateway route

## 13. Testing Checklist

Implementation is not done until all of this exists.

### 13.1 Frontend harness

File:

- `tests/test_frontend_js.sh`

Add tests for:

1. UI contains both `Root proximity` and `Solve proximity`
2. there are two palette containers
3. clicking a solve-proximity palette circle:
   - activates `renderColorMode === 'solve_proximity'`
   - updates only `renderSolveProximityPalette`
   - does not overwrite `renderRootProximityPalette`
4. `runRasterPipeline()` in solve-proximity mode:
   - dispatches clip
   - dispatches hist jobs
   - dispatches merge
   - includes `solve_proximity_bins_key` in raster jobs
   - passes the solve palette, not the root palette
5. fallback to `_loadCalcMetaForRender()` works when `_lastCalcMeta` is absent

Do not fake success without asserting the actual payloads.

### 13.2 Python unit tests

Files:

- `tests/test_pipeline.py`
- `tests/test_dispatch_resilience.py`
- `tests/test_solve_proximity_handler.py`
- `tests/test_solve_proximity_stats.py`

Required assertions:

- dispatch target exists
- storage detail returns calc metadata
- clean-render removes solve-proximity intermediates
- handler phases invoke subprocess correctly
- merge computes 9 cuts
- stats binary clip/hist math is correct on tiny exact fixtures

### 13.3 Docker runtime regression

File:

- `scripts/test-docker-runtime.sh`

Must verify:

1. `solve_proximity_stats` exists in `/src`
2. binary is ARM64 ELF
3. `--mode=clip` runs
4. `--mode=hist` runs
5. histogram JSON is sane

### 13.4 Playwright

Add:

- `tests/e2e/render-solve-proximity.spec.js`

Required browser checks:

1. Render tab shows both `Root proximity` and `Solve proximity`
2. both rows have independent palette circles
3. clicking solve-proximity palette circles leaves root-proximity selection untouched
4. rows have increased vertical spacing
5. selecting solve proximity triggers the correct render dispatch chain with mocked backend

This does not need real S3.
It does need real DOM behavior.

## 14. Manual Test Checklist

Do this manually after deploy.

### Case A: Fresh job, no reload

1. compute a new job
2. go to Render
3. choose `Solve proximity`
4. choose a non-default solve palette
5. render
6. confirm:
   - clip phase logs appear
   - hist phase logs appear
   - merge phase logs appear
   - render completes
   - image shows multiple palette bins, not near-monochrome collapse

### Case B: Existing job after page reload

1. reload page
2. select an old job from Results
3. go to Render
4. choose `Solve proximity`
5. render
6. confirm:
   - frontend successfully loads `calc.lores.bin_key` via `/detail`
   - no dependency on `_lastCalcMeta`

### Case C: Root transforms

1. add a visible root transform chain
2. render in `Solve proximity`
3. compare with no transform
4. confirm color distribution changes consistently with transformed geometry

This is the sanity check that the stats binary applied `root_xforms.h` correctly.

### Case D: Cleanup

1. render once in `Solve proximity`
2. inspect S3 keys under `renders/{job}/`
3. re-render with a different root transform chain
4. confirm old:
   - `solve_proximity_clip.json`
   - `solve_proximity_bins.json`
   - `solve_proximity/stripe_*_hist.json`
   were removed by `/clean-render`

## 15. Acceptance Criteria

This feature is complete only if all of the following are true:

1. Render UI has:
   - `Rainbow`
   - `Root proximity`
   - `Solve proximity`
   - `Constant`

2. Root and solve proximity have independent palette selections.

3. `solve_proximity` computes one scalar per solve and one color per solve.

4. Bins are derived from a global hires histogram, not stripe-local min/max.

5. Existing `proximity` still works exactly as before, except for the label rename.

6. Old jobs after reload can still render in solve-proximity mode.

7. Cleanup removes solve-proximity intermediate artifacts.

8. Dispatch/deploy/runtime tests include the new lambda and new binary.

9. No part of the implementation depends on hand-wavy "it should probably work" logic.

## 16. Implementation Order

If the implementor wants the safest order, do it in this sequence:

1. `solve_proximity_stats.c`
2. Docker/runtime smoke for that binary
3. `handler_solve_proximity.py`
4. dispatch target + deploy packaging
5. storage `/detail` + cleanup support
6. frontend UI rows + palette state split
7. frontend orchestration for clip/hist/merge
8. `handler_raster.py` bins download / CLI passing
9. `roots2pix.c` new color mode
10. frontend harness + Python tests + Playwright
11. manual render test on a real job

Do not start with `roots2pix.c` and hope the orchestration can be bolted on later.
That is how this turns into a half-wired mess.
