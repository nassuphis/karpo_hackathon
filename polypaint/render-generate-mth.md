# Render Generate MT

Status: implemented v1.

This document defines the concrete v1 plan for making:

- `Render -> Color -> Generate`

faster by multithreading the per-chunk raster worker.

The shipped UX is:

- `Generate`
  - explicit single-thread baseline
- `Generate-MT`
  - opens a small popup
  - user chooses `threads`
  - popup summary states the current eligible color mode and requested worker count
  - explicit multithreaded comparison path

This implementation is intentionally optimized for:

- speed
- acceptable non-determinism

not for bit-exact parity with the current single-thread raster path.

## Decision

For the new MT Color raster path:

- we do **not** care which colliding root wins a pixel
- we **do** care that the final color is the color of some root that hit that pixel
- we **do not** require deterministic chunk-local `.pix` ordering
- we **do not** require exact parity with `roots2pix`

That means:

- no deterministic block merge
- no per-thread sparse stream merge
- no attempt to preserve current “first hit wins within chunk” behavior

The shipped v1 design is:

- split one chunk into contiguous solve ranges
- run the existing [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
  binary on each subrange in parallel subprocesses
- merge the worker-local `.pix` / `.pbx` files back into the standard
  chunk/tile outputs

This keeps finalize, encode, repalette, and file formats unchanged.

## Current Architecture

The current Color render flow is:

1. [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
   - builds the render plan

2. [stepfunctions/render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)
   - `ColorRasterMap`
   - `ColorFinalizeMap`
   - `ColorEncodeTask`
   - `ColorPreviewTask`

3. [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
   - downloads one chunk `.bin`
   - runs [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
   - uploads chunk/tile `.pix` files

4. [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)
   - assembles one output tile via [lambda/pixassemble.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/pixassemble.c)

The workflow already has coarse-grained parallelism:

- chunk raster map:
  - `MaxConcurrency: 10`
- tile finalize map:
  - `MaxConcurrency: 32`

So the missing speedup is inside each raster worker, and v1 gets it by running
multiple `roots2pix` processes inside one Lambda.

## Scope

## In scope for v1

- a new multithreaded raster binary
- a new raster MT Lambda
- plan/workflow routing to that Lambda
- solve-score focused optimization
- saved-palette / constant / rainbow-no-match support
- optional proximity support if still straightforward after the above

## Out of scope for v1

- multithreaded finalize
- multithreaded encode
- `rainbow` with `match=greedy`
- `rainbow` with `match=hungarian`
- exact parity with current single-thread raster output

## Why Solve Score Is the Best First Target

Among current Color modes, `solve_score` is the strongest MT candidate because per solve it does:

1. solve metric evaluation via [lambda/solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)
2. normalization / `omega`
3. bin selection
4. projection of all roots in the solve
5. pixel claim and color write

So it has more CPU per solve than:

- `saved_palette`
- `constant`

and does not have the sequential solve-to-solve dependency of rainbow matching.

## The Real Constraint

The hard part is not “who wins.”
We have relaxed that.

The hard part is:

- keeping the existing `.pix` / `.pbx` contracts
- not breaking finalize or fast Color RePalette
- getting real parallelism without rewriting `roots2pix` first

The v1 choice is therefore:

- parallel subprocesses, not shared-memory C threading

That gives immediate speedup with much less risk.

## V1 Design

## New Lambda

Add:

- [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)

This handler mirrors [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py):

- same input shape
- same output shape
- same status phases

The native executable stays:

- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)

## MT execution model

For one chunk:

1. download the chunk `.bin`
2. split it into contiguous solve-range subfiles
3. for `saved_palette`, split the saved bin file into matching per-worker slices
4. run `roots2pix` in parallel on each worker slice
5. concatenate worker-local tile outputs into the normal final:
   - `pix_tXXXX.pix`
   - `pixbin_tXXXX.pbx`
6. upload those standard outputs

This deliberately allows the worker merge order to decide the winner for
cross-worker pixel collisions.

That is acceptable for this path because:

- speed matters more than repeatability
- the winning color is still the color of a real root
- finalize already uses `last wins`

## Supported modes in v1

### Must support first

- `solve_score`
- `saved_palette`
- `constant`
- `rainbow` with `match=none`

### Must stay on single-thread path

- `rainbow` + `match=greedy`
- `rainbow` + `match=hungarian`
- `proximity`

Reason:

- they maintain solve-to-solve color assignment continuity through `colorMap`
- that state is inherently sequential
- `proximity` would also change semantics if chunk-local min/max were computed
  per worker instead of once per whole chunk

## How each supported mode maps into the MT design

### Solve score

Per solve:

1. optionally apply root transforms into thread-local arrays
2. compute solve score
3. compute `u`
4. apply `omega`
5. assign the solve bin
6. compute `rgb` from that bin once
7. project all roots in the solve inside worker-local `roots2pix`

### Saved palette

Per solve:

1. load saved bin for solve `p`
2. compute `rgb` from that bin once
3. project all roots in the solve inside worker-local `roots2pix`

If `emit_pixel_bins` is enabled, the merged `.pbx` files continue to feed the
existing fast Color RePalette path.

### Constant

Per solve:

1. precompute one packed RGB
2. project all roots inside worker-local `roots2pix`

### Rainbow with `match=none`

Per solve:

1. root color is just root index -> palette color
2. project each root inside worker-local `roots2pix`

No sequential cross-solve state is needed.

## Recommended implementation sequence

Implement in this order:

1. add [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)
2. keep [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c) unchanged
3. add plan/workflow routing
4. add perf reporting on both raster handlers
5. benchmark memory tier / thread count / map concurrency

## Routing rules

## Plan-time routing

In [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py), add:

- `plan.raster.requested_engine`
- `plan.raster.engine`
- `plan.raster.function_name`
- `plan.raster.eligible`
- `plan.raster.reason`

Eligibility rules for v1:

- `mode == "color"`
- and one of:
  - `color_mode == "solve_score"`
  - `color_mode == "saved_palette"`
  - `color_mode == "constant"`
  - `color_mode == "rainbow" and match_mode == "none"`
  - optionally `color_mode == "proximity"` once implemented

Otherwise route to the current raster Lambda.

The key rule is:

- the normal `Generate` button sends `raster_engine=single`
- the separate `Generate-MT` button sends `raster_engine=mt`
- plan-time routing only selects MT when both:
  - `requested_engine == "mt"`
  - the current Color mode is eligible

Also require:

- if `emit_pixel_bins` is true, MT routing is only allowed for modes that actually
  populate `tileBin` correctly
- otherwise fall back to the current single-thread raster Lambda

## State machine routing

In [stepfunctions/render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template):

- stop hardcoding the raster Lambda ARN in `ColorRasterMap`
- use the function from the plan:
  - `$.plan.raster.function_name`

Do not create a second render workflow.

Keep one workflow and route inside it.

## Lambda sizing

The current raster Lambda uses:

- `RASTER_MEMORY=1769`

That is not the right tier for MT.

Add in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh):

- `RASTER_MT_NAME="polypaint-raster-mt"`
- `RASTER_MT_MEMORY=<higher tier>`

Implemented starting point:

- `RASTER_MT_MEMORY=4096`

Also add env:

- `RASTER_MT_THREADS`

Implemented starting point:

- `RASTER_MT_THREADS=2`

Do not start at 4+ threads without measuring.

Because Step Functions already runs many chunk Lambdas in parallel, too many threads per Lambda can lose overall throughput.

## Step Functions concurrency retuning

Do not change this immediately.

Current:

- `ColorRasterMap.MaxConcurrency = 10`

After MT raster is working, benchmark:

1. single-thread raster @ current concurrency
2. MT raster @ same concurrency
3. MT raster @ slightly reduced concurrency

Only then decide whether:

- `MaxConcurrency` should stay `10`
- or drop to `6..8`

This is a benchmark question, not a design assumption.

## Concrete file changes

### New files

- [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)
- [tests/test_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_mt.py)

### Existing files to modify

- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
  - MT eligibility and routing
- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
  - comparable single-thread perf reporting
- [stepfunctions/render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)
  - dynamic raster function selection
- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
  - build/package/deploy MT raster Lambda
- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
  - log raster perf summary on Color completion
- [docs/testing.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/testing.md)

## Native output contract

`handler_raster_mt.py` preserves the current external file contract:

- `renders/{job}/pix_chunk_{chunk}_t{tile}.pix`
- `renders/{job}/pixbin_chunk_{chunk}_t{tile}.pbx` when `emit_pixel_bins` is enabled

Each worker subprocess still returns the normal `roots2pix` JSON:

- `roots_plotted`
- `roots_clipped`

The MT handler aggregates those and reports them in its own final result.

## Required comparison logging

This is required for v1.

The current single-thread path must log comparable metrics too.
Otherwise the MT numbers are meaningless.

### Rule

Both raster handlers must report the same perf schema:

- existing:
  - [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
- new:
  - [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)

The only expected difference is:

- single-thread path reports `threads=1`
- MT path reports `threads=<actual>`

### Exact fields to publish per raster subtask

Each raster worker should store these in `result_data` when it reports `done`:

- `engine`
  - `"single"` or `"mt"`
- `threads`
  - integer
- `download_us`
- `native_us`
- `upload_us`
- `roots_plotted`
- `roots_clipped`
- `tiles_uploaded`
- if available from native JSON:
  - `transform_us`
  - `score_us`
  - `project_us`
  - `claim_us`
  - `serialize_us`

For the existing handler, `native_us` is just the current wall time around `roots2pix`.

The shipped UI now also logs per-phase progress with:
- `wall=<seconds>`
- `total=<summed worker seconds>`

for:
- `Solve score: clip`
- `Solve score: hist`
- `Solve score: merge`
- `Raster`
- `Finalize`
- `Encode`

### Transport path

Do not invent a new API for this.

Use the existing status plumbing:

- [lambda/shared.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/shared.py)
  - `report_status(..., result_data=...)`
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
  - `/check-status` already returns parsed `results`

So the implementation should be:

1. raster worker finishes
2. raster worker calls `report_status(job_id, task_id, "done", result_data=perf_dict)`
3. frontend raster-phase poll already calling `/check-status` receives those `results`
4. frontend aggregates and logs them on Color completion

### Frontend log requirement

At the end of the raster phase for `Render -> Color -> Generate`, the frontend should log one compact summary line to the render log.

Required fields:

- engine
- threads
- chunk count
- aggregate download time
- aggregate native time
- aggregate upload time
- total roots plotted
- total roots clipped

Recommended form:

```text
Raster perf: engine=single threads=1 chunks=100 wall=17.5s dl=2.1s native=13.4s up=2.0s plotted=12345678 clipped=234567 tiles=400
```

For MT:

```text
Raster perf: engine=mt threads=4 chunks=100 wall=8.2s dl=2.0s native=3.8s up=1.9s plotted=12340211 clipped=234901 tiles=400 pbx=400
```

### Important note on aggregation

Because raster subtasks run in parallel across chunks:

- per-subtask times should be aggregated as sums for CPU-stage comparison
- the current UI log uses those summed worker timings

## Tests

## Required routing tests

Add plan tests that assert:

- `solve_score` -> MT raster function
- `saved_palette` -> MT raster function
- `constant` -> MT raster function
- `rainbow + match=none` -> MT raster function
- `proximity` -> single-thread raster function
- `rainbow + greedy/hungarian` -> single-thread raster function

## Required handler tests

Add handler tests that assert:

1. worker-local `.pix` / `.pbx` files are merged into the standard final keys
2. `saved_palette` worker slices use the correct subset of the saved bin file
3. done `result_data` reports:
   - `engine`
   - `threads`
   - `download_us`
   - `native_us`
   - `upload_us`
   - `roots_plotted`
   - `roots_clipped`
4. existing single-thread handler reports the same perf schema with `threads=1`

This is covered by:

- [tests/test_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_mt.py)
- [tests/test_raster_pixel_bins.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_pixel_bins.py)
- [tests/test_raster_saved_palette.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_saved_palette.py)
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

## Explicit non-goals

This plan does **not** try to:

- preserve winner identity
- preserve exact `.pix` ordering
- preserve exact chunk-local collision winners vs single-thread `roots2pix`

What it must preserve is only:

- valid output files
- valid colors
- compatibility with the existing finalize/encode path

## Recommendation

The shipped v1 implementation is:

1. parallel `roots2pix` subprocesses inside [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)
2. separate `polypaint-raster-mt` Lambda at a higher memory tier
3. plan/workflow routing only for eligible Color modes
4. comparable perf reporting on both raster handlers
5. UI log aggregation so single-thread and MT can be compared directly

That is the fastest low-risk path that keeps the rest of the render pipeline unchanged.
