# Render Step Functions Migration Spec

Status: legacy archive. Historical migration spec preserved for reference; Render now uses Step Functions. Some examples still use legacy `stripe_*` / `stripe_items` terminology; current runtime planning is chunk-based and the current UI model is documented in [render_refactor.md](/Users/nicknassuphis/karpo_hackathon/polypaint/render_refactor.md).

This document is the migration spec for replacing the current self-reinvoking render orchestrator Lambda with an AWS Step Functions Standard workflow.

This spec is intentionally rigid.
The current implementation already caused AWS Lambda recursive-loop auto-remediation.
The implementor is not allowed to "mostly" migrate it while leaving recursive self-invocation, browser-owned phase transitions, or hidden async polling logic in place.

If anything here seems ambiguous, fix the spec before coding.

## 1. Goal

Replace the current render orchestration architecture:

- browser dispatches one `render_orchestrator` Lambda
- that Lambda owns the full pipeline
- when it is near timeout, it checkpoints and asynchronously invokes itself again

with this architecture:

- browser still launches one `render_orchestrator` target
- that target now starts one **Step Functions Standard** execution
- the workflow owns all phase transitions
- there is **no Lambda self-invocation**
- browser is a status observer only

The new design must preserve:

- the current worker Lambdas
- the current render outputs and S3 key layout
- the current top-level DDB run row (`task_id = render_run_<mode>_<run_id>`)
- the current frontend launch flow as much as possible

## 2. Why This Migration Is Required

Current self-reinvoke logic is in:

- [lambda/handler_render_orchestrator.py:98](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py#L98)
- [lambda/handler_render_orchestrator.py:119](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py#L119)

That implementation invokes:

- `SELF_FUNCTION`

which is:

- [lambda/handler_render_orchestrator.py:37](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py#L37)

This creates a chain:

- `polypaint-render-orchestrator -> polypaint-render-orchestrator -> ...`

AWS Lambda now detects recursive loops composed only of Lambda functions and will terminate them by default.

This is not an acceptable steady-state design.

The migration must eliminate:

1. Lambda self-invocation
2. recursive loop configuration hacks
3. timeout-threshold checkpoint logic

## 3. Required End State

After the migration:

1. There is **no** `lambda_client.invoke(FunctionName=SELF_FUNCTION, ...)` anywhere in the render path.
2. There is **no** need to configure `Allow` recursive loop behavior on any Lambda.
3. Browser launch still performs one dispatch to `target: 'render_orchestrator'`.
4. The `render_orchestrator` Lambda becomes a **starter Lambda** only.
5. Step Functions Standard owns the workflow execution.
6. Worker Lambdas are invoked directly from the state machine.
7. Browser observer polls DDB status only.
8. Background tab / page reload do not stop the backend pipeline.

## 4. Non-Negotiable Design Decisions

### 4.1 Use Step Functions Standard, not Express

Use:

- **Standard** workflow

Do not use:

- Express workflow

Reasons:

1. render runs can be long-lived
2. there are multiple phase boundaries and waits
3. execution history matters for debugging
4. background-safe orchestration is the whole point

### 4.2 Keep the browser launch target stable

Current frontend launch uses:

- [index.html:1252](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1252)

with:

- `target: 'render_orchestrator'`

Keep that target name.

Do not force the frontend to learn a new launch target just to adopt Step Functions.

### 4.3 Keep the existing function name `polypaint-render-orchestrator`

The easiest safe migration is:

- keep the deployed Lambda function name
- replace its code with a thin starter Lambda

So:

- `RENDER_ORCHESTRATOR_NAME="polypaint-render-orchestrator"`

remains valid in:

- [deploy.sh:58](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh#L58)

but the function stops being a recursive orchestrator.

### 4.4 Keep `handler_dispatch.py` generic

The dispatch Lambda must stay a generic target router.

For render launches it must continue to:

- accept `target: 'render_orchestrator'`
- resolve that target to the deployed starter Lambda
- asynchronously invoke the starter Lambda

It must **not**:

- call `states:StartExecution` directly for render launches
- own Step Functions execution-name generation
- write the top-level render DDB row
- absorb render-specific startup logic that belongs in the starter Lambda

Reasons:

1. render launch semantics belong with render code, not the generic dispatcher
2. starter behavior must remain unit-testable in isolation
3. initial DDB row creation must stay co-located with workflow start
4. the browser launch surface stays stable without baking Step Functions details into generic dispatch code

### 4.5 Do not "wrap" the old orchestrator in Step Functions

This is explicitly forbidden.

The state machine must **not**:

- invoke the old long-running orchestrator Lambda as one big task
- invoke the orchestrator Lambda again at later phases
- use Step Functions only as a thin shell around the recursive orchestrator

The point is to remove the recursive orchestrator design, not disguise it.

### 4.6 Do not replace the orchestrator with one generic "phase Lambda"

This is also explicitly forbidden.

The migration must **not** introduce a catch-all Lambda such as:

- `polypaint-sfn-phase`
- `phase=<clean|viewport|calc_meta|dispatch|poll|done>`

that still centralizes orchestration logic in imperative Python.

Forbidden pattern:

- Step Functions calls one generic phase Lambda
- that Lambda switches on `phase`
- that Lambda dispatches async workers
- that Lambda polls DDB
- that Lambda performs stall redispatch

That is still the same homegrown orchestrator, just sliced into smaller invocations.

Required replacement:

- starter Lambda
- plan Lambda
- status Lambda
- direct worker invoke states and `Map` states in ASL

### 4.7 Do not use Step Functions to async-dispatch and then poll DDB from Lambda loops

The state machine must invoke worker Lambdas directly with synchronous task states and `Map` states.

Do not recreate:

- async invoke
- poll loop
- timeout checkpoint

inside a new helper Lambda.

That would be the same design with a different wrapper.

### 4.8 Preview generation stays out of the workflow

This migration is for the main render pipelines only.

The color workflow ends at:

- encode done
- top-level DDB row update

It does **not** automatically generate preview PNGs as part of the Step Functions workflow.

Preview generation remains:

- browser-triggered
- on-demand
- separate from the main render state machine

Reasons:

1. current preview generation is already an independent async feature
2. previews are optional and should not lengthen or complicate the main workflow
3. the migration target is orchestration correctness, not preview feature redesign

Do not add:

- `ColorPreviewPhase`
- `ColorPreviewTask`

unless the spec is explicitly revised later.

## 5. Concrete Architecture

Create these runtime components:

1. **Starter Lambda**
   - file: [lambda/handler_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py)
   - deployed function name unchanged:
     - `polypaint-render-orchestrator`
   - new responsibility:
     - validate launch request
     - write initial top-level DDB row
     - start Step Functions execution
     - return immediately

2. **Plan Lambda**
   - new file:
     - `lambda/handler_render_plan.py`
   - new function name:
     - `polypaint-render-plan`
   - responsibility:
     - load metadata
     - compute viewport if needed
     - normalize params
     - compute stripe/tile plans
     - return compact workflow plan JSON

3. **Status Lambda**
   - new file:
     - `lambda/handler_render_status.py`
   - new function name:
     - `polypaint-render-status`
   - responsibility:
     - update the top-level render run row in DDB
     - set phase labels / expected counts / subtask prefixes / error / done

4. **State machine**
   - new file:
     - `stepfunctions/render_workflow.asl.json.template`
   - new state machine name:
     - `polypaint-render-workflow`

Existing worker Lambdas remain:

- `polypaint-raster`
- `polypaint-finalize`
- `polypaint-encode`
- `polypaint-viewport`
- `polypaint-storage`
- `polypaint-bilevel`
- `polypaint-bilevel-stitch`
- `polypaint-solve-proximity`

Worker `report_status()` behavior stays in place.

Do not remove worker DDB status writes during this migration.

Reason:

- Step Functions becomes the workflow control plane
- but browser secondary `/check-status` reads still rely on worker rows for live counts and debugging

## 6. Starter Lambda Design

## 6.1 File and function

Reuse:

- [lambda/handler_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py)

but replace its internal logic.

It is no longer allowed to contain:

- `run_color(...)`
- `run_bilevel(...)`
- `run_coeff_bilevel(...)`
- `_SelfReinvoke`
- `_check_timeout`
- `_poll_completion`
- worker-dispatch loops

The file becomes a starter only.

## 6.2 Inputs

Input contract remains the same as current browser launch:

```json
{
  "job_id": "compute_x",
  "run_id": "run_abc",
  "mode": "color",
  "params": { ... }
}
```

## 6.3 Responsibilities

The starter Lambda must:

1. parse launch payload
2. validate `mode`
3. compute:
   - `task_id = render_run_<mode>_<run_id>`
4. generate a Step Functions execution name
   - exact required format:
     - `render_<mode>_<run_id>`
5. call `StartExecution`
6. write the initial DDB row with:
   - `job_id`
   - `task_id`
   - `task_status = queued`
   - `phase = queued`
   - `phase_label = Queued`
   - `mode`
   - `run_id`
   - `execution_arn`
   - `started_at_ms`
   - `updated_at_ms`
7. return:
   - `execution_arn`
   - `task_id`
   - `run_id`

## 6.4 What it must not do

The starter Lambda must not:

1. poll DDB
2. dispatch worker jobs
3. call itself
4. perform phase transitions
5. own checkpoint logic

## 6.5 Environment variables

Required env for the starter Lambda:

- `BUCKET`
- `JOBS_TABLE`
- `RENDER_STATE_MACHINE_ARN`

It no longer needs all worker Lambda names.

That is deliberate.
It should not know how to orchestrate workers anymore.

## 7. Plan Lambda Design

## 7.1 Purpose

The state machine needs compact but sufficient execution data for `Map` states.

Do **not** try to build all tile math and per-mode job arrays directly in Amazon States Language.
That invites a fragile mess.

The plan Lambda is the single place that prepares:

- viewport
- calc metadata subset
- stripe plan
- tile plan
- output keys
- solve-score plan

## 7.2 File and function

Create:

- `lambda/handler_render_plan.py`

Deploy as:

- `polypaint-render-plan`

## 7.3 Inputs

Input:

```json
{
  "job_id": "compute_x",
  "run_id": "run_abc",
  "task_id": "render_run_color_run_abc",
  "mode": "color",
  "params": { ... }
}
```

## 7.4 Responsibilities

The plan Lambda must:

1. load calc metadata
2. validate required fields
3. compute viewport:
   - square mode directly
   - auto mode by invoking `polypaint-viewport`
4. compute:
   - `n_stripes`
   - `degree`
   - `pix`
   - `tile_size`
   - `n_tile_cols`
   - `n_tile_rows`
   - `n_tiles`
5. normalize solve-score parameters:
   - legacy `solve_proximity` alias
   - `solve_metric`
   - `solve_score_quantile`
6. produce compact arrays for later Map states

## 7.5 Output shape

Required output object:

```json
{
  "job_id": "compute_x",
  "run_id": "run_abc",
  "task_id": "render_run_color_run_abc",
  "mode": "color",
  "params": { ...normalized... },
  "viewport": {
    "center_re": 0.0,
    "center_im": 0.0,
    "scale": 512.0
  },
  "calc": {
    "degree": 70,
    "n_stripes": 10,
    "lores_bin_key": "renders/compute_x/lores.bin",
    "coeffs_keys": [...],
    "n_coeffs": 71
  },
  "grid": {
    "pix": 8192,
    "tile_size": 2048,
    "n_tile_cols": 4,
    "n_tile_rows": 4,
    "n_tiles": 16,
    "tile_keys": [
      "renders/compute_x/tile_0000.raw",
      "renders/compute_x/tile_0001.raw"
    ]
  },
  "stripe_items": [
    {"stripe_idx": 0},
    {"stripe_idx": 1}
  ],
  "tile_items": [
    {"tile_idx": 0, "tile_row": 0, "tile_col": 0, "tile_w": 2048, "tile_h": 2048}
  ],
  "solve_score": {
    "enabled": true,
    "metric": "clusteriness",
    "quantile": 0.01,
    "clip_key": "renders/compute_x/solve_scores/clusteriness_clip.json",
    "hist_prefix": "renders/compute_x/solve_scores/clusteriness/",
    "bins_key": "renders/compute_x/solve_scores/clusteriness_bins.json"
  },
  "outputs": {
    "image_key": "renders/compute_x/image.jpeg",
    "bilevel_key": "renders/compute_x/image_bilevel.tif",
    "coeff_bilevel_key": "renders/compute_x/image_coeffs_bilevel.tif"
  }
}
```

The plan Lambda must precompute `grid.tile_keys`.

Do not try to synthesize `tile_0000.raw` style strings inside ASL.
That formatting belongs in the plan Lambda, not the state machine definition.

Not every mode uses every field, but keep one stable shape.

## 7.6 Compactness requirement

The plan output must stay well below the Step Functions 256 KB state limit.

Therefore:

1. `stripe_items` may contain only compact objects
2. `tile_items` may contain only compact objects
3. do not emit full worker payloads per item
4. do not copy the full `calc.json` into state

Fail fast if the serialized plan exceeds `200 KB`.

Do not silently start a workflow that will exceed the Step Functions state payload limit later.

## 8. Status Lambda Design

## 8.1 File and function

Create:

- `lambda/handler_render_status.py`

Deploy as:

- `polypaint-render-status`

## 8.2 Purpose

This Lambda owns top-level render run row updates in DDB.

It replaces the current role of:

- [handler_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py)

for top-level progress reporting.

## 8.3 Input actions

Support these actions:

1. `queued`
2. `phase`
3. `done`
4. `error`

### `queued`

Writes initial row after `StartExecution`.

### `phase`

Updates:

- `phase`
- `phase_label`
- `expected`
- `subtask_prefix`
- `updated_at_ms`
- optional extra context

### `done`

Writes:

- `task_status = done`
- `phase = done`
- `phase_label = Done`
- optional `image_key`

### `error`

Writes:

- `task_status = error`
- `phase = error`
- `phase_label = Error`
- `error_msg`

For `error`, the status Lambda must extract a human-readable message from the Step Functions error envelope.

Required behavior:

1. read top-level `Error` and `Cause` if present
2. if `Cause` is JSON, parse it
3. if parsed JSON contains:
   - `errorMessage`
   - or nested Lambda `Payload.errorMessage`
   - or nested `Cause`
   extract the most specific human-readable message available
4. write that readable message to DDB as `error_msg`

Do not write only opaque strings such as:

- `States.TaskFailed`
- `Lambda.Unknown`

when a more specific worker error message is available.

## 8.4 Top-level DDB row contract

The row for:

- `task_id = render_run_<mode>_<run_id>`

must continue to be the browser’s primary observation point.

Required fields in `result_data`:

- `job_id`
- `run_id`
- `mode`
- `phase`
- `phase_label`
- `execution_arn`
- `started_at_ms`
- `updated_at_ms`

Optional but strongly recommended:

- `expected`
- `subtask_prefix`
- `image_key`

## 9. Browser Responsibilities After Migration

Browser launch remains in:

- [index.html:1240](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1240)

## 9.1 What stays the same

The browser still:

1. gathers params
2. dispatches one `render_orchestrator` job
3. saves active-run record in localStorage
4. polls the top-level DDB run row

If starter returns `execution_arn`, the browser should also store it in the active-run record.

This is optional for correctness but strongly recommended for debugging.

## 9.2 What changes

The browser must stop assuming the top-level row contains live `done` counts for every phase.

Instead:

1. poll the top-level run row as today
2. if the row contains:
   - `subtask_prefix`
   - `expected`
3. issue a second `/check-status` read-only query for that subtask prefix
4. derive `done/expected` from worker rows

This is acceptable because the browser is only observing, not orchestrating.

If the browser is background-throttled, UI counts may lag.
That is fine.
The pipeline keeps progressing because Step Functions owns it.

### 9.2.1 Exact frontend change

Update:

- [index.html:_pollActiveRenderRun()](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1347)

Required behavior:

1. first call:
   - `lambdaPost('storage', { job_id, task_prefix: run.task_id, expected: 1 }, '/check-status')`
2. read the first result object:
   - `const rd = check.results?.[0] || {}`
3. read phase text from:
   - `rd.phase_label || rd.phase || 'working'`
4. if `rd.subtask_prefix` and `rd.expected` are present, make a second call:
   - `lambdaPost('storage', { job_id, task_prefix: rd.subtask_prefix, expected: rd.expected }, '/check-status')`
5. derive live counts from the second response:
   - `done = subcheck.done || 0`
   - `expected = subcheck.expected || rd.expected || 0`
6. if no `subtask_prefix` is present:
   - show phase text only
7. completion detection remains based on the top-level run row:
   - `check.complete` or `rd.phase === 'done'`

Do not change `/check-status` response shape for this migration.
Reuse the existing response contract from storage:

- `done`
- `errors`
- `error_details`
- `expected`
- `complete`
- optional `results`

## 9.3 Frontend restrictions

The browser must still not:

1. dispatch raster/finalize/encode/stitch directly
2. own phase transitions
3. own timeout recovery

## 10. State Machine Definition

## 10.1 File and name

Create:

- `stepfunctions/render_workflow.asl.json.template`

Deploy as state machine:

- `polypaint-render-workflow`

Use a template file with placeholders for Lambda ARNs and concurrency constants.

Do not hardwire ARNs directly into checked-in JSON.

## 10.2 Required high-level structure

The workflow must look like this:

1. `ReportQueuedPhase`
2. `CleanRender`
3. `BuildPlan`
4. `ModeChoice`
5. one of:
   - `ColorPipeline`
   - `BilevelPipeline`
   - `CoeffBilevelPipeline`
6. `ReportDone`
7. `Succeed`

There must be a top-level `Catch` path to:

1. `ReportError`
2. `Fail`

## 10.2.1 State data flow rule

The workflow must use one consistent `ResultPath` convention.

Required rule:

1. starter execution input remains at the top level
2. `BuildPlan` writes its output to:
   - `$.plan`
3. all status/report states use:
   - `ResultPath: null`
4. all worker Task and Map states use:
   - `ResultPath: null`
   unless a specific later state explicitly needs their returned value
5. later states must read render plan data from:
   - `$.plan`
6. no state may overwrite the whole execution input with a worker result array or one-off payload

This is the default required pattern because:

- plan data must survive across all later phases
- most worker outputs are deterministic in S3 and do not need to remain in state
- it avoids accidental loss of `grid`, `tile_items`, `tile_keys`, and solve-score metadata

Do not improvise mixed `ResultPath` usage across states without a documented reason.

## 10.3 Color pipeline states

Required color states:

1. `ColorSolveScoreChoice`
2. if solve-score enabled:
   - `ColorSolveScoreClipPhase`
   - `ColorSolveScoreClipTask`
   - `ColorSolveScoreHistPhase`
   - `ColorSolveScoreHistMap`
   - `ColorSolveScoreMergePhase`
   - `ColorSolveScoreMergeTask`
3. `ColorRasterPhase`
4. `ColorRasterMap`
5. `ColorFinalizePhase`
6. `ColorFinalizeMap`
7. `ColorEncodePhase`
8. `ColorEncodeTask`

## 10.4 Bilevel pipeline states

Required bilevel states:

1. `BilevelRasterPhase`
2. `BilevelRasterMap`
3. `BilevelMergePhase`
4. `BilevelMergeMap`
5. `BilevelStitchPhase`
6. `BilevelStitchTask`

## 10.5 Coeff bilevel pipeline states

Required coeff states:

1. `CoeffRasterPhase`
2. `CoeffRasterMap`
3. `CoeffMergePhase`
4. `CoeffMergeMap`
5. `CoeffStitchPhase`
6. `CoeffStitchTask`

## 10.6 Worker invocation rule

Every worker state must use:

- synchronous Lambda invoke integration

Use:

- `arn:aws:states:::lambda:invoke`

Do not use async invoke + DDB poll loops inside helper Lambdas.

## 10.7 Map state concurrency

Use static `MaxConcurrency`.

Required initial values:

- stripe maps:
  - `10`
- tile maps:
  - `32`

This includes:

- solve-score hist map
- color raster map
- color finalize map
- bilevel raster map
- bilevel merge map
- coeff raster map
- coeff merge map

Do not leave concurrency unbounded.
Do not set required Map states to `MaxConcurrency: 1`.

## 10.8 Map failure policy

Map states must **not** retry the whole Map on business-logic worker failure.

Required behavior:

1. individual Lambda invoke states inside the Map may use the normal transient invoke retry policy
2. if a worker ultimately fails with a business-logic error, the Map fails
3. top-level workflow `Catch` handles that by reporting error and failing the execution
4. partial outputs already written by successful items are acceptable
5. a user rerun starts from `clean-render`, which removes stale intermediates before work begins again

Do not add:

- whole-Map retry on item failure
- custom per-item catch-and-continue logic that marks a render successful with missing stripes or tiles

For this migration, correctness is:

- all required items succeed
- otherwise the workflow fails cleanly

## 10.9 Retry policy

Every Lambda invoke state must have a retry block for transient Lambda invocation failures.

Required retry on:

- `Lambda.ServiceException`
- `Lambda.AWSLambdaException`
- `Lambda.SdkClientException`

Use:

- `IntervalSeconds = 2`
- `BackoffRate = 2.0`
- `MaxAttempts = 3`

Do not retry business-logic failures indefinitely.

## 11. How Payloads Are Built in Map States

Do not precompute full payloads for every worker item in the plan Lambda.

Use compact plan data plus `ItemSelector` in Map states.

### 11.1 Stripe-based maps

For stripe maps, `stripe_items` entries only need:

- `stripe_idx`

The Map state builds payloads by combining:

- top-level plan fields
- `$.Map.Item.Value.stripe_idx`

### 11.2 Tile-based maps

For tile maps, `tile_items` entries must include:

- `tile_idx`
- `tile_row`
- `tile_col`
- `tile_w`
- `tile_h`

This avoids trying to do division/mod arithmetic inside ASL.

## 12. Deploy Changes

## 12.1 Keep deploying the starter Lambda

Keep:

- `RENDER_ORCHESTRATOR_NAME="polypaint-render-orchestrator"`

But its zip contents change to the starter implementation only.

It must no longer ship the old recursive orchestration logic.

Do not remove `polypaint-render-orchestrator` from deploy and route around it from `handler_dispatch.py`.

That would be the wrong seam.

## 12.2 Add new Lambda packages

Deploy new functions:

1. `polypaint-render-plan`
2. `polypaint-render-status`

## 12.3 Add Step Functions deployment

`deploy.sh` must:

1. render the ASL template with actual Lambda ARNs
2. create or update:
   - `polypaint-render-workflow`
3. create or update a dedicated Step Functions IAM role

`deploy.sh` must **not**:

- delete the `polypaint-render-orchestrator` starter Lambda in favor of a generic phase Lambda
- add a render-specific `StartExecution` path inside `handler_dispatch.py`
- add recursive-loop `Allow` as a "temporary" fix and quietly leave it in place

## 12.4 Required new deploy variables

Add:

- `RENDER_STATE_MACHINE_NAME="polypaint-render-workflow"`
- `RENDER_PLAN_NAME="polypaint-render-plan"`
- `RENDER_STATUS_NAME="polypaint-render-status"`

Starter Lambda env must include:

- `RENDER_STATE_MACHINE_ARN`
- `JOBS_TABLE`
- `BUCKET`

Plan Lambda env must include:

- `BUCKET`
- `VIEWPORT_FUNCTION`
- `STORAGE_FUNCTION`

Status Lambda env must include:

- `JOBS_TABLE`

## 12.5 Remove recursion-related behavior

Do not add:

- `aws lambda put-function-recursion-config --recursive-loop Allow`

to `deploy.sh`.

That would be the wrong "fix".

The whole point of this migration is to avoid needing recursive Lambda allowance.

## 13. Tests

## 13.1 Replace orchestrator Lambda tests

Current file:

- [tests/test_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_orchestrator.py)

This file currently tests the monolithic recursive orchestrator.

After migration, replace it with starter-Lambda tests:

1. starter validates `mode`
2. starter calls `StartExecution` exactly once
3. starter writes queued DDB row
4. starter returns `execution_arn`
5. starter does **not** invoke itself
6. dispatch handler still invokes the starter Lambda for `target='render_orchestrator'`
7. dispatch handler does **not** call Step Functions directly
8. starter does **not** call `DescribeExecution` or block waiting for workflow completion

## 13.2 Add plan Lambda tests

New file:

- `tests/test_render_plan.py`

Required coverage:

1. color plan with square viewport
2. color plan with auto viewport
3. solve-score plan includes compact solve-score fields
4. bilevel tile plan shape
5. coeff bilevel uses `coeffs_keys` from metadata
6. plan output stays compact / rejects oversize
7. plan Lambda does **not** dispatch workers or poll DDB for completion

## 13.3 Add status Lambda tests

New file:

- `tests/test_render_status.py`

Required coverage:

1. queued row write
2. phase update with `subtask_prefix` and `expected`
3. done update with `image_key`
4. error update with `error_msg`
5. error action extracts readable worker errors from Step Functions `Error` / `Cause`
6. status Lambda does **not** dispatch workers or poll DDB for completion

## 13.4 Add state machine definition tests

New file:

- `tests/test_render_workflow_definition.py`

Required coverage:

1. ASL template parses as valid JSON after rendering
2. required state names exist
3. required `Map` states exist
4. no state targets `polypaint-render-orchestrator`
5. no state uses recursive starter invocation
6. retry policy exists on Lambda invoke states
7. no state targets a generic catch-all phase Lambda for dispatch/poll orchestration
8. `BuildPlan` writes to `$.plan`
9. report states use `ResultPath: null`
10. worker states do not overwrite top-level plan data
11. required Map states set the specified `MaxConcurrency` values
12. worker states target real worker Lambdas, not intermediary dispatch wrappers

## 13.5 Frontend harness tests

Update:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

Required additions:

1. launch still dispatches one `render_orchestrator` target
2. no direct worker dispatch from browser launch
3. observer can handle top row containing:
   - `phase_label`
   - `subtask_prefix`
   - `expected`
4. observer uses secondary `/check-status` read for subtask counts

## 13.6 Existing worker tests

Keep and reuse existing worker suites:

- raster
- finalize
- encode
- bilevel
- solve-score

Do not rewrite working worker logic merely because orchestration is changing.
Do not remove worker `report_status()` calls as part of "cleanup".

## 13.7 Manual validation

Manual validation is mandatory.

Required scenarios:

1. color render, leave browser tab in background longer than the old Lambda threshold
2. bilevel render, leave browser tab in background
3. coeff bilevel render, leave browser tab in background
4. reload page mid-run and verify observer resumes
5. confirm no AWS Health recursive-loop notification
6. confirm `RecursiveInvocationsDropped` does not increase for render workflow

## 14. Acceptance Criteria

The migration is complete only when all of these are true:

1. no self-invocation remains in the render path
2. `polypaint-render-orchestrator` is a starter only
3. Step Functions Standard owns phase transitions
4. worker Lambdas are invoked directly from the workflow
5. browser launch target remains `render_orchestrator`
6. browser is not responsible for dispatching later phases
7. top-level run row in DDB is still the main browser observation point
8. background-tab rendering continues without recursion config hacks
9. page reload resumes observation correctly
10. deploy does not set recursive-loop `Allow`
11. `handler_dispatch.py` still routes render launch through the starter Lambda, not Step Functions directly
12. the workflow does not route dispatch/poll orchestration through a generic phase multiplexer Lambda

## 15. Anti-Cheat Review Checklist

Reject the patch if any of these are true:

1. `handler_render_orchestrator.py` still contains `_SelfReinvoke`
2. `handler_render_orchestrator.py` still calls `lambda_client.invoke(FunctionName=SELF_FUNCTION, ...)`
3. Step Functions invokes `polypaint-render-orchestrator` after startup
4. Step Functions only wraps the old monolithic orchestrator Lambda
5. browser still dispatches raster/finalize/encode/stitch directly
6. `deploy.sh` adds recursive-loop `Allow` instead of removing recursion
7. the state machine uses async invoke + poll-loop helper Lambdas instead of direct worker tasks
8. no top-level DDB row updates exist for browser observation
9. worker job arrays are shoved wholesale into state input without compactness checks
10. manual validation does not include background-tab and reload cases
11. `handler_dispatch.py` calls `StartExecution` directly for `target='render_orchestrator'`
12. a generic `polypaint-sfn-phase` style Lambda owns dispatch/poll/stall logic
13. `handler_render_plan.py` dispatches workers, polls DDB, or performs phase transitions
14. `handler_render_status.py` dispatches workers, polls DDB, or performs stall recovery
15. starter Lambda calls `DescribeExecution` or waits for workflow completion
16. worker Lambdas invoke later-phase workers or starter/status Lambdas directly
17. required `Map` states use `MaxConcurrency: 1` or route items through intermediary dispatch Lambdas
18. `BuildPlan` does not write plan data to `$.plan`, or later states overwrite that plan data
19. ASL definition is inlined in `deploy.sh` instead of living in the template file
