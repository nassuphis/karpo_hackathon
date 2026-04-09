# Compute PostCoeffgen Data Limit Plan

## Problem

A real compute run failed in AWS after coeffgen finished:

- execution: `compute_aberth_mt_run_1775736858691_vki47x`
- job: `compute_mnrfvl8i`
- start: `2026-04-09 15:14:19 +03:00`
- stop: `2026-04-09 15:15:16 +03:00`
- Step Functions error: `States.DataLimitExceeded`
- cause:
  - `The state/task 'PostCoeffgen' returned a result with a size exceeding the maximum number of bytes service limit.`

Observed UI symptom:

- log stopped at `Coeffgen 500/500...`
- top-level DDB row remained at phase `coeffgen`
- the browser did not surface the real Step Functions failure

This was not a coeffgen worker failure.

## What Actually Happened

The current compute workflow does this:

1. `CoeffgenMap` runs `500` coeffgen workers.
2. Each worker returns a result row into Step Functions state.
3. `PostCoeffgen` in [lambda/handler_compute_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_plan.py) builds:
   - `degree`
   - `n_coeffs`
   - `total_coeffs_size`
   - `lores`
   - `sweep_items`
4. `sweep_items` contains one full solve payload per chunk.
5. With `500` chunks, that returned JSON is too large for Step Functions.

So the workflow fails after coeffgen, before lores or solve begins.

## Root Cause

The failure is structural:

- too much per-chunk data is being carried inside Step Functions state
- `PostCoeffgen` returns a `500`-entry `sweep_items` array
- `CoeffgenMap` also carries more output fields than the next step truly needs

This is exactly the kind of state-payload bloat Step Functions punishes.

## Design Rule

Step Functions state must carry:

- compact control metadata
- not large per-chunk worklists if those can be derived from existing plan data

The workflow should pass references and scalars, not fully expanded task arrays.

## Fix Plan

### 1. Make `PostCoeffgen` compact

Change [lambda/handler_compute_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_plan.py):

- keep:
  - `degree`
  - `n_coeffs`
  - `total_coeffs_size`
  - `lores`
- remove from returned payload:
  - `sweep_items`

`PostCoeffgen` should return only the metadata needed to continue.

### 2. Stop materializing solve work items in state

Change [stepfunctions/compute_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/compute_workflow.asl.json.template):

- `SolveMap` should iterate over `$.plan.chunk_items`, not `$.post.sweep_items`

That means `build_plan` must ensure each `chunk_item` already contains everything solve needs except `n_coeffs`, which is only known after coeffgen.

Recommended `chunk_items` additions at plan-build time:

- `solve_task_id`
- `bin_key`

Then `SolveMap.ItemSelector` can combine:

- from `plan.chunk_items`:
  - `job_id`
  - `chunk_idx`
  - `step_count`
  - `coeffs_key`
  - `solve_task_id`
  - `bin_key`
- from `post`:
  - `n_coeffs`

No expanded `sweep_items` array is needed.

### 3. Trim coeffgen map output too

Change `CoeffgenMap` worker output to keep only what `PostCoeffgen` actually uses:

- `degree`
- `n_coeffs`
- `coeffs_size`
- optionally `chunk_idx` for diagnostics

Do not keep in Step Functions state:

- `job_id`
- `coeffs_key`
- `elapsed_us`

Those are already available elsewhere:

- detailed perf is in DDB worker rows
- object keys are in `plan.chunk_items`

### 4. Add an explicit visible phase after coeffgen

Right now the browser shows:

- `Coeffgen 500/500...`

while Step Functions is doing hidden work.

Add a top-level phase between `CoeffgenMap` and lores:

- `phase = post_coeffgen`
- `phase_label = Post coeffgen`

This makes the handoff visible and credible in the UI.

### 5. Improve compute failure visibility

Current bad behavior:

- top-level DDB row can stay at `coeffgen`
- browser appears stuck

Needed behavior:

- if execution fails after coeffgen and before next phase row, the browser should not sit forever on `Coeffgen 500/500...`

Plan:

- add a compute stale-state rule similar to render warning/stall handling
- and/or surface the top-level Step Functions failure path more reliably in the compute status row

Important note:

- the primary fix is to avoid `States.DataLimitExceeded`
- but stale-phase UI should still be handled better

## Clear Button

Render has a `Clear` button because render persists active-run browser state in local storage.

Compute does not currently persist an active run that way, so a compute `Clear` button is not required for correctness.

Optional later:

- add a compute-side `Clear` as a UI reset only
- but it is not the fix for this workflow bug

## Implementation Order

1. Make `PostCoeffgen` stop returning `sweep_items`.
2. Move solve map derivation to `plan.chunk_items + post.n_coeffs`.
3. Trim `CoeffgenMap` output fields.
4. Add explicit `Post coeffgen` phase reporting.
5. Tighten compute stale/error reporting in the browser.

## Tests Required

### Workflow definition

Update [tests/test_compute_workflow_definition.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_compute_workflow_definition.py):

- assert `SolveMap` no longer reads `$.post.sweep_items`
- assert `PostCoeffgen` phase exists if added

### Plan Lambda

Add/update tests for [lambda/handler_compute_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_plan.py):

- `handle_post_coeffgen()` returns compact metadata only
- no `sweep_items` in returned payload
- `build_plan()` chunk items contain the fields solve will need later

### Size-budget regression

Add a regression that simulates large chunk counts, e.g. `500`, and asserts:

- `post_coeffgen` output remains compact
- serialized response stays comfortably below Step Functions payload limits

### Frontend

Update [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh):

- compute log shows `Post coeffgen` if that phase is added
- compute does not appear indefinitely stuck at `Coeffgen x/x`

## Done Means

This issue is fixed when:

- a `500`-chunk compute run does not fail at `PostCoeffgen`
- Step Functions state no longer carries expanded solve worklists
- the UI either advances to the next phase or shows an explicit error
- `Coeffgen 500/500...` is no longer a misleading terminal-looking stuck state
