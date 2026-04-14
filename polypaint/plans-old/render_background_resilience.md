# Render Background Resilience Spec

Status: legacy archive. Historical design note preserved for reference; background-safe rendering is now owned by Step Functions plus DynamoDB status rows, and this file still contains migration-stage payload examples with `n_stripes` and similar legacy names.

This document is the implementation spec for making Render progress resilient to browser background-tab throttling and page suspension.

The intended audience is an implementor who should not have to infer anything important.
If something here is ambiguous, treat that as a spec bug and fix the spec before coding.

This spec is intentionally strict because the current render pipeline is browser-orchestrated, which means "switch away and come back later" can stall the pipeline itself, not just the UI.

## 1. Problem Statement

Current Render, BiLevel, and Coeffs pipelines are orchestrated in the browser.

Key entrypoints:

- color render:
  - [runRasterPipeline()]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1234 )
- bilevel render:
  - [runBilevelPipeline()]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2167 )
- coeffs bilevel render:
  - [runCoeffBilevelPipeline()]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2317 )

Current orchestration uses browser-side polling loops with `setTimeout()`:

- solve proximity clip poll:
  - [index.html#L1152]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1152 )
- solve proximity merge poll:
  - [index.html#L1196]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1196 )
- finalize poll:
  - [index.html#L1372]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1372 )
- encode poll:
  - [index.html#L1446]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1446 )
- bilevel stitch poll:
  - [index.html#L2275]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2275 )
- coeff stitch poll:
  - [index.html#L2437]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2437 )
- wave dispatch/poll helper:
  - [_bilevelDispatchAndPoll()]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2062 )

This has two separate failure modes:

1. If the user only leaves the app's internal Render tab:
   - the JS still runs
   - the UI is just hidden

2. If the browser tab/window is backgrounded or suspended:
   - browser timers can be throttled or suspended
   - the JS poll loops stop or slow dramatically
   - because the browser is also responsible for dispatching later phases, the backend pipeline can stall at phase boundaries

This is the core design bug.

## 2. What Must Change

The browser must stop being the orchestrator.

After this refactor:

- the browser starts a render run
- the backend owns phase transitions
- the browser only observes status
- if the browser tab is backgrounded, the render continues
- when the user returns, the browser reconnects to the current run and updates the UI

This is not optional.

Do not "fix" this by:

- only adding `visibilitychange`
- only adding `focus` handlers
- only moving polling into a Web Worker
- only making the poll interval longer

Those approaches do not solve the fundamental problem that the browser currently owns phase transitions.

## 3. Goals

This refactor must achieve all of these:

1. Render continues if the browser tab is backgrounded.
2. Render continues if the user switches to another browser tab/window.
3. The user can return later and see current progress.
4. Page reload should not destroy the active run state.
5. The browser must not be responsible for dispatching finalize/encode/stitch after the initial launch.
6. Existing backend worker Lambdas may remain mostly unchanged, but orchestration must move off the browser.

## 4. Non-Goals

This spec does not require:

- streaming logs
- WebSockets
- Server-Sent Events
- Step Functions
- changing the actual raster/finalize/encode math
- simultaneous multiple active runs on the same job and mode

Single active run per `(job_id, mode)` is acceptable in v1.

## 4.1 Failed Approaches That Will Not Work

The following ideas are explicitly rejected.

They may sound cheaper, but they do not fix the real architecture problem.

### Failed Approach A: "Just decouple dispatch from poll timing"

This proposal usually means:

- keep browser orchestration
- modify [_bilevelDispatchAndPoll()]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2062 )
- dispatch waves more eagerly
- treat polling as "status only"

Why this fails:

1. It only touches one helper used for within-phase wave dispatch.
2. The browser still owns inter-phase transitions.
3. Those later transitions are where the pipeline can still stall if the browser is backgrounded.

Concrete current examples:

- color render still requires browser code to dispatch finalize at [index.html#L1346]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1346 )
- color render still requires browser code to dispatch encode at [index.html#L1434]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1434 )
- bilevel render still requires browser code to dispatch stitch at [index.html#L2267]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2267 )
- coeff render still requires browser code to dispatch stitch at [index.html#L2429]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2429 )
- solve-proximity prepass still requires browser-owned clip/merge polling at [index.html#L1152]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1152 ) and [index.html#L1196]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1196 )

So even if wave dispatch becomes less poll-sensitive, the pipeline still stalls at the next browser-owned phase boundary.

This is not an acceptable final fix.

### Failed Approach B: "Use a Web Worker for polling"

This proposal usually means:

- keep browser orchestration
- move polling to a Web Worker
- assume worker timers are not meaningfully throttled in background tabs

Why this is not acceptable as the core fix:

1. It still leaves orchestration in the browser.
2. It still does not survive reload/crash/browser restart correctly.
3. It relies on browser timer behavior as a correctness assumption.
4. That assumption is not strong enough across browsers.

Important point:

- a Worker may help in some environments
- a Worker is not a durable orchestration boundary

Even if worker timers behave better than main-thread timers in some browsers, this remains the wrong ownership model for a multi-phase backend job.

### Failed Approach C: "Make polling less frequent / add visibility handlers"

This includes:

- longer poll intervals
- `visibilitychange` only
- `focus` only
- "refresh status when the tab becomes visible again"

These may improve the UI, but they do not change who owns phase transitions.

They are useful only after backend orchestration exists.

### Summary

The rejected approaches all share the same flaw:

- they try to make browser orchestration less fragile
- instead of removing browser orchestration as the source of truth

That is why this spec requires a backend orchestrator Lambda.

## 5. Root Cause in Current Code

Current browser-owned phase transitions:

- color render:
  - browser dispatches raster at [index.html#L1316]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1316 )
  - browser waits for raster completion
  - browser dispatches finalize at [index.html#L1346]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1346 )
  - browser waits for finalize completion
  - browser dispatches encode at [index.html#L1434]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1434 )

- bilevel render:
  - browser dispatches raster
  - browser dispatches merge
  - browser dispatches stitch

- coeffs render:
  - browser dispatches coeff raster
  - browser dispatches merge
  - browser dispatches stitch

As long as that remains true, background suspension can stall the pipeline even though already-dispatched Lambdas continue running.

## 6. Required End State

The required architecture is:

1. Browser launches one orchestration job.
2. A new backend orchestrator Lambda owns the full pipeline.
3. The orchestrator polls DDB and dispatches later phases.
4. The browser polls only the orchestrator task row.
5. Browser visibility changes only affect UI refresh cadence, not pipeline progress.

## 7. New Lambda

Add a new Lambda:

- file:
  - `lambda/handler_render_orchestrator.py`
- function name:
  - `polypaint-render-orchestrator`

This Lambda is the only owner of phase transitions for:

- color render
- bilevel render
- coeffs bilevel render

It must support:

- `mode = "color"`
- `mode = "bilevel"`
- `mode = "coeff_bilevel"`

## 8. Dispatch Wiring

Update [lambda/handler_dispatch.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py#L25 ) to add a new target:

- `"render_orchestrator"`

Use an env var:

- `RENDER_ORCHESTRATOR_FUNCTION`

Do not hardwire the literal function name without env support.

Update [deploy.sh]( /Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh ) to:

- package the new Lambda
- create/update it
- pass `RENDER_ORCHESTRATOR_FUNCTION` into dispatch Lambda env

## 9. Browser Responsibilities After Refactor

The browser may do only these things:

1. gather user-selected parameters
2. launch one orchestrator job
3. poll orchestrator status
4. restore observation of an active run after visibility change or page reload
5. render status/log UI

The browser must not:

- dispatch raster phases directly
- dispatch finalize directly
- dispatch encode directly
- dispatch stitch directly
- decide when a phase is "complete enough" to start the next one

## 10. Active Run Model

### 10.1 Run Identity

Every launched run must have a unique `run_id`.

Format:

- `run_{unix_ms}_{6 random base36 chars}`

Example:

- `run_1774512345678_ab12cd`

### 10.2 Orchestrator Task ID

The orchestrator task id must be:

- `render_run_{mode}_{run_id}`

Examples:

- `render_run_color_run_1774512345678_ab12cd`
- `render_run_bilevel_run_1774512345678_ab12cd`

### 10.3 Browser-Persisted Run Record

Store the active run record in `localStorage`.

Key:

- `polypaint_active_render_run`

Value shape:

```json
{
  "job_id": "compute_abc123",
  "mode": "color",
  "run_id": "run_1774512345678_ab12cd",
  "task_id": "render_run_color_run_1774512345678_ab12cd",
  "started_at_ms": 1774512345678
}
```

There is only one active run record in v1.

When a new render is launched, overwrite the old record.

## 11. Backend Orchestration Strategy

The orchestrator must be resumable and must not assume one Lambda invocation can stay alive for the whole render.

### 11.1 Checkpoint Model

The orchestrator writes its current high-level state to its own DDB task row using [report_status()]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/shared.py#L22 ).

Use `result_data` to store:

```json
{
  "job_id": "compute_abc123",
  "run_id": "run_1774512345678_ab12cd",
  "mode": "color",
  "phase": "raster_poll",
  "phase_label": "Raster",
  "started_at_ms": 1774512345678,
  "updated_at_ms": 1774512356789,
  "n_stripes": 40,
  "n_tiles": 16,
  "done": 12,
  "expected": 40,
  "status_counts": {"done": 12, "started": 28},
  "image_key": "renders/compute_abc123/image.jpeg"
}
```

This row is the source of truth for the UI.

### 11.2 Self-Resume Rule

If the orchestrator is close to Lambda timeout, it must:

1. persist current phase state to its DDB task row
2. asynchronously invoke itself with the same payload
3. return successfully

Use `context.get_remaining_time_in_millis()`.

Threshold:

- if remaining time `< 60000 ms`, checkpoint and self-reinvoke

Do not let the orchestrator die at Lambda timeout.

### 11.3 Poll Sleep Rule

Inside the orchestrator, polling sleep may still use `time.sleep(3)`.

That is acceptable because the orchestrator is backend-side, not browser-side.

Poll interval:

- `3 seconds`

## 12. Subtask Identity

To make resumable orchestration safe, worker subtask ids must include `run_id`.

This is mandatory.

Current fixed task ids like:

- `raster_{stripe_idx}` in [lambda/handler_raster.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py#L29 )
- `tile_{tile_idx}` in [lambda/handler_finalize.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py#L33 )
- `encode` in [lambda/handler_encode.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_encode.py#L26 )

are not sufficient for resumable orchestration across restarts and reruns.

### 12.1 Required Change

Each worker Lambda must accept an optional `task_id`.

If provided:

- use it exactly

If not provided:

- preserve old default behavior

This applies to:

- `lambda/handler_raster.py`
- `lambda/handler_finalize.py`
- `lambda/handler_encode.py`
- `lambda/handler_bilevel.py`
- `lambda/handler_bilevel_stitch.py`
- `lambda/handler_solve_proximity.py` if its orchestrated phases also need unique IDs

### 12.2 Required Task ID Formats

Color mode:

- raster stripe:
  - `render_{run_id}_raster_{stripe_idx}`
- finalize tile:
  - `render_{run_id}_tile_{tile_idx}`
- encode:
  - `render_{run_id}_encode`
- solve proximity clip:
  - `render_{run_id}_solve_proximity_clip`
- solve proximity hist stripe:
  - `render_{run_id}_solve_proximity_hist_{stripe_idx}`
- solve proximity merge:
  - `render_{run_id}_solve_proximity_merge`

Bilevel mode:

- raster stripe:
  - `render_{run_id}_bilevel_raster_{stripe_idx}`
- merge tile:
  - `render_{run_id}_bilevel_merge_{tile_idx}`
- stitch:
  - `render_{run_id}_bilevel_stitch`

Coeff bilevel mode:

- raster stripe:
  - `render_{run_id}_coeff_bilevel_raster_{stripe_idx}`
- merge tile:
  - `render_{run_id}_coeff_bilevel_merge_{tile_idx}`
- stitch:
  - `render_{run_id}_coeff_bilevel_stitch`

## 13. Orchestrator Input Shape

Input payload from browser to orchestrator:

```json
{
  "job_id": "compute_abc123",
  "run_id": "run_1774512345678_ab12cd",
  "mode": "color",
  "params": {
    "pix": 8192,
    "fmt": "jpeg",
    "quality": 95,
    "view_mode": "quantile",
    "quantile": 0.0,
    "shim": 0.05,
    "square_extent": 2.0,
    "tile_size": 2048,
    "rotation": 0.0,
    "constant_color": "ffffff",
    "color_mode": "rainbow",
    "match_mode": "order",
    "palette": "inferno",
    "root_transforms": []
  }
}
```

For `solve_proximity`, the params must also include:

- `color_mode = "solve_proximity"`
- `palette = renderSolveProximityPalette`

Do not make the orchestrator read DOM-specific names.
Normalize them in the browser before launch.

## 14. Orchestrator High-Level Phases

### 14.1 Color Mode

The orchestrator phases for `mode=color` are:

1. `clean`
2. `viewport`
3. `calc_meta`
4. optional `solve_proximity_clip`
5. optional `solve_proximity_hist`
6. optional `solve_proximity_merge`
7. `raster_dispatch`
8. `raster_poll`
9. `finalize_dispatch`
10. `finalize_poll`
11. `encode_dispatch`
12. `encode_poll`
13. `cleanup`
14. `done`

### 14.2 Bilevel Mode

Phases:

1. `clean`
2. `viewport`
3. `calc_meta`
4. `bilevel_raster_dispatch`
5. `bilevel_raster_poll`
6. `bilevel_merge_dispatch`
7. `bilevel_merge_poll`
8. `bilevel_stitch_dispatch`
9. `bilevel_stitch_poll`
10. `done`

### 14.3 Coeff Bilevel Mode

Phases:

1. `clean`
2. `viewport`
3. `calc_meta`
4. `coeff_raster_dispatch`
5. `coeff_raster_poll`
6. `coeff_merge_dispatch`
7. `coeff_merge_poll`
8. `coeff_stitch_dispatch`
9. `coeff_stitch_poll`
10. `done`

## 15. Browser Refactor

### 15.1 Replace Browser-Orchestrated Pipelines

Refactor:

- [runRasterPipeline()]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1234 )
- [runBilevelPipeline()]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2167 )
- [runCoeffBilevelPipeline()]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2317 )

to do only:

1. gather params
2. generate `run_id`
3. store active run record in `localStorage`
4. dispatch one `render_orchestrator` job
5. start or resume UI polling for the orchestrator row

The browser must not dispatch internal worker phases anymore.

### 15.2 New Browser Poller

Add a dedicated lightweight UI poller.

Suggested function names:

- `startActiveRenderObserver()`
- `stopActiveRenderObserver()`
- `resumeActiveRenderObserver()`
- `_pollActiveRenderRun()`

Behavior:

- poll orchestrator task row every 3 seconds while the document is visible
- when `document.visibilityState === "hidden"`, stop UI polling
- when document becomes visible again, immediately refresh once and restart polling

This is safe because backend orchestration continues independently.

### 15.3 Visibility and Reload Hooks

Add:

- `document.addEventListener('visibilitychange', ...)`
- `window.addEventListener('focus', ...)`
- startup hook after app init to restore `localStorage` active run record

Behavior:

- if an active run record exists, reattach observer
- if orchestrator status is terminal (`done` or `error`), clear localStorage record

## 16. UI Status Contract

The orchestrator must write enough `result_data` for the browser to display useful progress.

Required fields:

```json
{
  "job_id": "compute_abc123",
  "run_id": "run_1774512345678_ab12cd",
  "mode": "color",
  "phase": "finalize_poll",
  "phase_label": "Finalize",
  "done": 12,
  "expected": 16,
  "status_counts": {"done": 12, "started": 4},
  "image_key": "renders/compute_abc123/image.jpeg",
  "n_stripes": 40,
  "n_tiles": 16
}
```

Optional fields:

- `warning`
- `stall_ms`
- `non_202`
- `raster_ms`
- `finalize_ms`
- `encode_ms`
- `merge_ms`
- `stitch_ms`

## 17. Completion Behavior

When orchestrator reaches terminal success:

1. it writes `task_status = "done"` for its own task row
2. it includes final output key in `result_data`
3. browser observer:
   - updates `render-status`
   - calls `refreshRenderArtifacts(job_id)` once
   - clears the active run record from `localStorage`

When orchestrator reaches terminal error:

1. it writes `task_status = "error"`
2. it includes a truncated human-readable error message
3. browser observer:
   - shows error in `render-status`
   - clears active run record

## 18. Files To Change

Primary files:

- [index.html]( /Users/nicknassuphis/karpo_hackathon/polypaint/index.html )
- [lambda/handler_dispatch.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py )
- [lambda/handler_render_orchestrator.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py ) new
- [lambda/handler_raster.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py )
- [lambda/handler_finalize.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py )
- [lambda/handler_encode.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_encode.py )
- [lambda/handler_bilevel.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py )
- [lambda/handler_bilevel_stitch.py]( /Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel_stitch.py )
- [deploy.sh]( /Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh )
- [tests/test_frontend_js.sh]( /Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh )

Recommended backend test files:

- `tests/test_render_orchestrator.py` new
- `tests/test_dispatch_resilience.py`

Optional docs:

- [docs/lambdas.md]( /Users/nicknassuphis/karpo_hackathon/polypaint/docs/lambdas.md )

## 19. Explicit Anti-Cheat Rules

These are not suggestions.

The implementor must not:

- keep browser-owned phase transitions and merely add visibility handlers
- keep `_bilevelDispatchAndPoll()` as the engine for Render/BiLevel/Coeffs
- move polling into a Worker and claim the problem is solved
- rely on the tab staying foregrounded
- preserve fixed worker task ids that collide across reruns
- make the browser dispatch finalize/encode/stitch after the initial launch

If any of that remains true, the refactor is incomplete.

## 20. Required Tests

### 20.1 Backend Orchestrator Tests

Add `tests/test_render_orchestrator.py`.

Required tests:

1. `test_color_orchestrator_dispatches_raster_then_finalize_then_encode`
   - mock downstream Lambdas and DDB status responses
   - assert phase order

2. `test_color_orchestrator_solve_proximity_prepass_runs_before_raster`
   - color mode with `solve_proximity`
   - assert clip → hist → merge → raster

3. `test_bilevel_orchestrator_dispatches_raster_merge_stitch`
   - assert correct phase order

4. `test_coeff_orchestrator_dispatches_raster_merge_stitch`
   - assert correct phase order

5. `test_orchestrator_checkpoints_and_self_reinvokes_when_time_low`
   - fake `context.get_remaining_time_in_millis()`
   - assert self invoke happens

6. `test_orchestrator_writes_status_row_each_phase`
   - assert `report_status()` calls include `phase`

7. `test_worker_task_ids_include_run_id`
   - assert dispatched jobs carry the exact task ids from section 12.2

8. `test_orchestrator_error_sets_terminal_error_status`
   - downstream error should mark orchestrator row `error`

### 20.2 Worker Compatibility Tests

Add or update tests to prove worker Lambdas accept explicit `task_id`.

Required tests:

- raster handler uses custom `task_id` if provided
- finalize handler uses custom `task_id` if provided
- encode handler uses custom `task_id` if provided
- bilevel handlers use custom `task_id` if provided
- old default behavior still works if `task_id` is omitted

### 20.3 Frontend Harness Tests

Add tests in [tests/test_frontend_js.sh]( /Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh ).

Required tests:

1. `runRasterPipeline dispatches one render_orchestrator job`
2. `runBilevelPipeline dispatches one render_orchestrator job`
3. `runCoeffBilevelPipeline dispatches one render_orchestrator job`
4. `browser no longer dispatches raster/finalize/encode/stitch directly`
5. `active run record written to localStorage`
6. `visibilitychange visible resumes observer`
7. `observer refreshes render artifacts on terminal success`
8. `observer clears localStorage on terminal success`
9. `observer clears localStorage on terminal error`

### 20.4 Search-Based Validation

Run:

```bash
rg -n "_bilevelDispatchAndPoll\\(|target: 'raster'|target: 'finalize'|target: 'encode'|target: 'bilevel_stitch'|task_prefix: 'tile_'|task_prefix: 'encode'|task_prefix: 'bilevel_stitch'|task_prefix: 'coeff_bilevel_stitch'" index.html
```

Expected:

- these may remain in helper/test code
- they must not remain in the primary launch path for Render/BiLevel/Coeffs

## 21. Manual Validation Checklist

Run these manual checks after implementation:

1. Start a color render, then background the browser tab for 2 minutes.
   - On returning, render has progressed or finished.
   - It did not stall waiting for browser-side finalize or encode dispatch.

2. Start a bilevel render, then switch browser tabs.
   - On returning, stitch has progressed or finished.

3. Start a render, then reload the page.
   - App restores active run observer.
   - Progress resumes in UI without restarting the render.

4. Start a render, then switch internal app tabs only.
   - Status resumes immediately when returning to Render tab.

5. Start a render and watch CloudWatch / DDB.
   - One orchestrator task row updates over time.
   - Worker rows include `run_id` in task ids.

## 22. Suggested Implementation Order

Do the work in this order:

1. add optional `task_id` support to worker Lambdas
2. add new orchestrator Lambda
3. add dispatch/deploy wiring
4. add backend orchestrator tests
5. refactor browser launch paths to single orchestrator dispatch
6. add localStorage run restore + visibility/focus observer
7. add frontend harness tests
8. run manual background-tab validation

Do not start with frontend-only hacks.

## 23. Acceptance Criteria

This refactor is complete only if all of these are true:

1. Browser launch path dispatches exactly one orchestrator job.
2. Browser no longer dispatches downstream render phases directly.
3. Backgrounding the browser tab does not stall the backend render pipeline.
4. Reloading the page does not orphan the run from the UI.
5. Worker task ids include `run_id`.
6. Backend orchestrator tests pass.
7. Frontend harness observer tests pass.
