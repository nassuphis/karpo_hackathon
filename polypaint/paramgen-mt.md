# ParamGen / CoeffGen MT + Chunked Params Plan

Status: Phase 1 through Phase 3 are implemented for the compute/render/palette
paths. Full-res param generation is now Step Functions fan-out by chunk and
writes chunk-local `params_0000.bin` style objects. `lores_params.bin` remains
global. Legacy global full-res `params.bin` metadata is still supported as a
fallback for old jobs, but new compute jobs do not assemble a redundant global
full-res params object.

This plan covers two related questions:

1. `Compute -> Calculate-AE-MT` should expose native threading controls in a
   popup, similar to `Render -> Color -> Generate-MT`.
2. `param_gen` is currently a single global `params.bin` producer. We need to
   decide whether to:
   - keep that global artifact and make the native C path internally
     multithreaded, or
   - move toward chunked param artifacts, similar to coeff chunks, without
     wasting time assembling a redundant full-res `params.bin`.

The core requirement is strict:

- do not break existing param-source solve-score chips
- do not break existing render/palette consumers that rely on `params_key`
- do not strand old `calc.json` readers
- do not add a full-res `params.bin` assembly step just to preserve an
  implementation detail that can be hidden behind an adaptor

## Short Answer

Yes, the UI should get a popup for `Calculate-AE-MT`.

Yes, `param_gen` is a good candidate for native internal multithreading.

Yes, `coeffgen_chunked` should also get local native multithreading inside
each chunk worker. The compute fan-out already exists, so there is no reason
to leave 3 local cores idle inside a large coeffgen Lambda.

For large full-res runs, the right medium-term target is:

- keep `lores_params.bin` global
- move full-res params to chunked `params_0000.bin` style artifacts
- do not spend time assembling a redundant full-res `params.bin`
- hide the storage shape behind helper/adaptor code in the plan/handler layer

The rollout order was:

1. add UI controls for param-gen and coeffgen MT
2. add internal MT to the native `param_gen` C path
3. add internal MT to the native `coeffgen_chunked` C path
4. add chunked full-res params artifacts and metadata
5. teach coeffgen/render/palette/solve-score consumers to use an adaptor that
   resolves the correct param source per chunk
6. keep `lores_params.bin` global for debug/clip paths
7. preserve old-job fallback for legacy global `params.bin` readers, but do
   not require new jobs to materialize a full global params object

## Current State

### UI

`Calculate-AE-MT` now opens a popup with a compact compute-stage table:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
  - `compute-mt-popup-overlay`
  - `compute-mt-param-gen-threads`
  - `compute-mt-coeffgen-threads`
  - `compute-mt-lores-param-gen-threads`
  - `compute-mt-lores-coeffgen-threads`

The popup dispatches:

- `param_gen_threads`
- `coeffgen_threads`
- `lores_param_gen_threads`
- `lores_coeffgen_threads`

Defaults are currently:

- full param gen: `4`
- full coeffgen: `4`
- lores param gen: `1`
- lores coeffgen: `1`

### Workflow

The compute workflow currently has:

- `ParamGenMap` fan-out over `plan.chunk_items`
- chunked coeffgen fan-out
- one lores param gen
- one lores coeffgen
- thread counts passed through all four stages

References:

- [stepfunctions/compute_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/compute_workflow.asl.json.template)
- [lambda/handler_compute_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_plan.py)

### Execution path

Full-res `param_gen` today is:

- one Lambda invocation per compute chunk
- thin Python wrapper
- one C subprocess
- local native pthread workers when `n_threads > 1`
- one chunk-local S3 object output per chunk:
  - `renders/<job>/params_0000.bin`
  - `renders/<job>/params_0001.bin`
  - ...
- native `param_gen` accepts `step_start` and `step_count`

References:

- [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py)
- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)

`coeffgen_chunked` today is:

- one Lambda invocation per chunk
- thin Python wrapper
- one C subprocess
- local native pthread workers when `n_threads > 1`
- sectioned local param reads with `pread`
- fixed-offset coefficient writes with `pwrite`

References:

- [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py)
- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)

### Storage layout today

Full compute path:

- chunked params objects:
  - `renders/<job>/params_0000.bin`
  - `renders/<job>/params_0001.bin`
  - ...
- chunked coeff objects:
  - `renders/<job>/coeffs_0000.bin`
  - `renders/<job>/coeffs_0001.bin`
  - ...

Lores path:

- global lores params object:
  - `renders/<job>/lores_params.bin`

Coeffgen reads chunk-local params when chunk metadata has `params_key`.
Legacy jobs without per-chunk params metadata still fall back to global
`params.bin` range reads:

- [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py)
  - `handle_coeffgen_chunked()`

Render/palette/solve-score param-source consumers use the same per-chunk
source tuple:

- `params_key`
- `params_step_start`
- `params_step_count`

Legacy fallback maps that tuple to:

- global `params_key`
- `params_step_start = step_start`
- `params_step_count = step_count`

References:

- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
- [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)
- [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
- [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

The shared storage adaptor is:

- [lambda/param_source.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/param_source.py)

### Implemented Checklist

- [x] `Calculate-AE-MT` popup exists.
- [x] Popup exposes thread counts for full param gen, full coeffgen, lores
  param gen, and lores coeffgen.
- [x] Compute plan validates and stores all four thread counts.
- [x] Compute Step Function forwards thread counts to param-gen and coeffgen
  tasks.
- [x] `handler_coeffgen.py` forwards `n_threads` to native `param_gen`.
- [x] `handler_coeffgen.py` forwards `n_threads` to native
  `coeffgen_chunked`.
- [x] Native `param_gen` supports pthread workers.
- [x] Native `coeffgen_chunked` supports pthread workers with sectioned
  local file reads/writes.
- [x] Status/logging reports thread counts and param-gen upload progress.
- [x] Full-res param fan-out.
- [x] `params_0000.bin` / `params_0001.bin` chunk objects.
- [x] Param-source adaptor helpers for global-vs-chunked layout.
- [x] Per-chunk `params_key`, `params_step_start`, and
  `params_step_count` metadata.
- [x] Coeffgen uses chunk-local params when present.
- [x] Render MT and single-thread raster paths use chunk-local params when
  present.
- [x] Solve histogram and associated-palette / palette-chunk paths use
  chunk-local params when present.
- [x] `lores_params.bin` remains global.
- [x] Legacy global `params.bin` fallback remains available for old
  `calc.json` files.

## Important Constraint

Param-source solve-score chips already exist and work:

- `t1_re`
- `t1_im`
- `t1_abs`
- `t1_phase`
- `t2_re`
- `t2_im`
- `t2_abs`
- `t2_phase`

Those compile to `pm` metric slots and already depend on:

- `lores_params_key` for clip/summary
- `params_key + params_step_start + params_step_count` for full
  hist/render/palette chunks

References:

- [lambda/solve_score_chain.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score_chain.py)
- [lambda/solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)

If we change param storage, these chips must continue to work without any user
visible syntax change.

That is non-negotiable.

The chips are not the place to encode storage layout. The plan/runtime layer
must adapt:

- legacy job:
  - global `params.bin` + `step_start`/`step_count`
- new job:
  - chunk-local `params_key` (for example `params_0007.bin`) with local
    offsets, or a direct whole-object read

User-visible chip syntax stays the same either way.

## Architectural Options

### Option A: keep global `params.bin`, add native MT inside `param_gen`

Shape:

- keep writing:
  - `renders/<job>/params.bin`
  - `renders/<job>/lores_params.bin`
- add `param_gen_threads` to the workflow payload
- teach the C `param_gen` path to split the step range across pthread workers
- preserve the same external artifact contract

Pros:

- smallest blast radius
- existing coeffgen range-read path keeps working
- existing solve-score `pm` readers keep working
- no `calc.json` schema churn beyond optional perf metadata

Cons:

- full params output is still one large object
- coeffgen workers still do range reads from the global params file
- does not reduce S3 object count or allow per-chunk params reuse by key

### Option B: chunked full-res params, global lores params, adaptor-based reads

Shape:

- full-res write:
  - `renders/<job>/params_0000.bin`
  - `renders/<job>/params_0001.bin`
  - ...
- lores write:
  - `renders/<job>/lores_params.bin`
- chunk metadata carries a chunk-local `params_key`
- plan/runtime helpers resolve param reads from either:
  - chunk-local `params_key`, or
  - legacy global `params.bin` + `step_start`/`step_count`
- no full-res `params.bin` assembly step for new jobs

Pros:

- removes the slowest redundant artifact and its assembly cost
- coeffgen chunk workers can read their own param object directly
- render/palette/solve-score chunk workers can read matching param chunks
  directly
- keeps lores debug/clip simple by leaving lores params global
- preserves chip syntax by hiding layout in helper code

Cons:

- more metadata to keep consistent
- all param-source consumers need to move onto the adaptor/helper path
- old jobs still need fallback support

### Option C: fan-out chunked params and also assemble a full global `params.bin`

Do not do this.

Why:

- it spends time and money writing data we do not need if the adaptor is done
  correctly
- it preserves the most expensive legacy artifact for no functional gain
- it adds an extra assembly stage on the critical compute path
- lores already gives us the one global params object we actually want to keep

This option is the wrong compromise: most of the migration complexity, plus an
avoidable assembly cost.

## Recommendation

Implemented rollout:

### Phase 1: Implemented

- add the `Calculate-AE-MT` popup
- add internal MT to native `param_gen`
- add internal MT to native `coeffgen_chunked`
- initially keep global params layout unchanged

Runtime shape after Phase 1 was:

- full param gen is still a single Lambda
- full param gen writes one global `params.bin`
- lores param gen writes one global `lores_params.bin`
- coeffgen still fans out by chunk
- each coeffgen chunk Lambda can use local native threads
- logs expose thread counts and param-gen byte/step progress

### Phase 2: Implemented

- full-res params moved to chunked artifacts
- keep only `lores_params.bin` global
- add an adaptor/helper layer so chunk consumers no longer assume a global
  `params.bin`
- do not assemble a redundant full-res `params.bin` just to preserve an old
  storage shape

### Phase 3: Implemented

- migrate all full-res param consumers to chunk-local params reads:
  - coeffgen
  - solve-score hist chunks
  - raster / raster-MT param-source score paths
  - palette chunk / associated palette paths
- keep legacy fallback for old jobs that still expose global `params.bin`

This keeps the performance improvement while avoiding a full-res params
assembly step.

## Adaptor Requirement

The storage-shape decision must be hidden behind shared helpers.

Do not make each call site open-code:

- "if chunked use params_0007.bin else range-read params.bin"

The shared abstraction is implemented in:

- [lambda/param_source.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/param_source.py)

That helper should return enough information for the caller to stay simple:

- storage mode: `global` or `chunked`
- source key
- local step offset
- step count

Expected behavior:

- legacy global layout:
  - `source_key = params.bin`
  - `local_step_start = original step_start`
  - `step_count = original step_count`
- new chunked layout:
  - `source_key = params_0007.bin`
  - `local_step_start = 0`
  - `step_count = chunk step_count`

This keeps chips and runtime logic focused on score semantics rather than S3
layout trivia.

## UI Plan

Replace the direct `Calculate-AE-MT` action with a popup similar in spirit to
the render MT popup.

### Objective

Make the compute MT path explicit and configurable, instead of hard-coding all
param-gen choices behind one button.

The popup should follow the same broad presentation pattern as
`Render -> Color -> Generate-MT`:

- a compact table
- rows are pipeline stages
- columns are stage-appropriate controls

But it must not copy the render popup mechanically. The controls have to match
the compute pipeline, not the render pipeline.

### Table shape

Recommended first-pass table:

- row: `Param gen`
- row: `Coeffgen`
- row: `Lores param gen`
- row: `Lores coeffgen`

Recommended initial columns:

- `Threads`

Possible later columns, only if they become real compute-stage choices:

- `Workers`
- `Access mode`

Do not expose a column just because render has one. Every column in the compute
popup must correspond to a real compute-stage runtime choice.

### First popup fields

Status: implemented.

Phase 1 should keep this minimal but complete for the native compute hot path:

- `Param gen` row → `Threads`
- `Coeffgen` row → `Threads`
- `Lores param gen` row → `Threads`
- `Lores coeffgen` row → `Threads`
- optional read-only summary text:
  - estimated total steps
  - current `N`
  - current `times`

Recommended defaults:

- full param gen threads: `4`
- lores param gen threads: `1` or `same as full`
- full coeffgen threads: `4`
- lores coeffgen threads: `1` or `same as full`

Do not overload the first popup with:

- memory tier selection
- chunk size
- retries
- render-style tmpfile/sectioned controls unless compute genuinely exposes two
  user-meaningful runtime modes
- S3 layout mode
- experimental toggles

Those are deploy-time or architecture-time concerns, not good first-user
controls.

### Payload

Status: implemented.

`runCalculateAEMT()` should no longer call `runCalculateWithSolver('aberth_mt')`
blindly.

Instead:

- open a popup
- capture MT settings
- dispatch compute with:
  - `param_gen_threads`
  - `lores_param_gen_threads`
  - `coeffgen_threads`
  - `lores_coeffgen_threads`

The backend also validates all four fields in `handler_compute_plan.py`.

## Native Param-Gen MT Plan

Status: implemented for both legacy global params and ranged chunk-local
params.

### Core observation

Each param row is fixed width:

- `4 x float32`
- `16 bytes`

Each step is independent after its base coordinates are known.

That means the work is structurally parallelizable by step range.

### Implemented threading model

Use fixed disjoint step ranges:

- worker 0: `[start0, end0)`
- worker 1: `[start1, end1)`
- ...

Each worker computes rows for its block only.

Current write model:

- worker-local output slots
- coordinator writes completed slots to stdout in order
- Python streams stdout directly to S3 multipart upload

This preserves the global artifact contract while allowing the expensive row
generation loop to use local Lambda cores.

## Native CoeffGen MT Plan

Status: implemented for chunk-local coeffgen workers.

### Core observation

Each coeffgen worker already operates on one chunk-local step range.

Within that chunk:

- each step is independent
- coefficient generation is embarrassingly parallel
- coeff transforms are applied per solve, not across solves

So the correct next move is local native threading inside each
`coeffgen_chunked` worker, not just relying on Step Functions fan-out.

### Threading model

Use the same broad strategy as the MT render binaries:

- split the local chunk into contiguous per-thread sections
- each thread owns one section of solve rows
- no work stealing, no shared per-step state

This should be implemented as sectioned local work, not one giant monolithic
buffer.

### Local I/O model

Status: implemented.

The native coeffgen worker should use sectioned local reads/writes:

- each thread reads only its assigned param rows from the local params slice
- each thread writes only its assigned coefficient rows into the final output
  file at fixed offsets

Concretely, the desired shape is:

- local params slice already exists as a local file in the worker
- thread `i` gets `[step_lo_i, step_hi_i)`
- input read path:
  - sectioned local file access for just that step range
- output write path:
  - fixed-offset sectioned writes into the shared output file

This avoids:

- a huge all-steps in-memory output buffer
- an extra concat/assembly pass
- thread contention on a single serial writer

Preferred implementation primitives:

- `pread`/`pwrite`, or
- equivalent fixed-offset sectioned file access

The current native implementation uses `pread` for each thread's param rows
and `pwrite` for fixed-offset coefficient rows.

### Non-determinism

Exact dither repeatability is not a design constraint here.

If coefficient transforms or upstream param transforms introduce randomized
variation, the MT implementation does not need to preserve byte-identical
single-thread ordering across runs.

The real requirement is:

- valid output
- stable metadata
- no crashes
- no cross-thread corruption

### Payload / metadata

Status: implemented.

The compute workflow should thread these knobs explicitly:

- `coeffgen_threads`
- `lores_coeffgen_threads`

And coeffgen task result metadata should report:

- `threads`
- `elapsed_us`

So the compute UI can surface the knob that was actually used.

### Scope boundary

This is still local threading inside each coeffgen Lambda.

It is not:

- a new Step Functions coeffgen layout
- a change to chunk boundaries
- a replacement for the existing coeffgen fan-out

The fan-out remains exactly as it is; each coeffgen worker just stops being
single-threaded internally.

Do not use:

- multiple threads writing unsynchronized to shared `stdout`

That is brittle and needlessly hard to reason about.

### Dithering / RNG

You said deterministic dithering is not required.

That relaxes the design substantially, but not completely.

What still matters:

- shared global RNG state is not thread-safe
- per-thread RNG state must be independent

So the plan should be:

- remove shared `_rng_state` from the threaded path
- give each worker its own RNG state
- accept that stochastic param transforms are no longer bit-for-bit identical
  to the old single-threaded path

That is acceptable if we document it explicitly.

### Metadata

Status: partially implemented.

Add perf metadata for observability:

- `threads`
- `elapsed_us`

Potential future additions:

- `block_count`
- native `compute_us`
- native `write_us`

Do not remove current fields like:

- `elapsed_us`
- `n_steps`
- `data_bytes`

Existing status aggregation should keep working.

## Should Params Be Chunked Like Coeffs?

Eventually, maybe.

Immediately, not as a replacement.

### Why chunked params are attractive

Coeffgen is already chunked:

- each coeff worker processes a `step_start` / `step_count` slice
- each coeff worker currently range-reads the matching bytes from the global
  `params.bin`

So yes, there is a coherent alternative:

- `param_gen` could emit chunked param objects aligned with `chunk_items`
- each coeff worker could read its dedicated param object directly

That is a valid design.

### Why it should not be phase 1

Because params are now used by more than coeffgen.

They are also used by:

- solve-score clip/hist debug on lores params
- render solve-score chunk workers
- palette chunk workers
- any `pm` solve-score program path

If we convert storage layout too early, we increase the risk of breaking:

- `t1_re(pm,...)`
- `t1_im(pm,...)`
- `t1_abs(pm,...)`
- `t1_phase(pm,...)`
- `t2_*`

That is exactly the class of breakage we want to avoid.

### Safe chunking rule

If we add chunked params, do not require a new full-res global `params.bin`
assembly step.

New jobs should be allowed to use:

- chunk-local full-res params:
  - `params_0000.bin`
  - `params_0001.bin`
  - ...
- canonical global lores params:
  - `lores_params.bin`

Old jobs should continue to work through fallback support for:

- legacy global full-res params:
  - `params.bin`

The compatibility layer is the adaptor, not a redundant assembly phase.
Materializing a full global `params.bin` for new chunked-param jobs is only
acceptable as a temporary migration/debug switch, not as the steady-state
design.

## Required Metadata Changes For Chunked Params

If chunked params are introduced, add per-chunk metadata:

- `chunk_items[].params_key`
- `chunk_items[].params_bin_size`
- `calc.param_storage_mode`, for example `global` or `chunked`

Keep existing fields:

- `calc.params_key`
- `calc.lores.params_key`
- `chunk_items[].step_start`
- `chunk_items[].step_count`

For old jobs:

- `calc.params_key` remains the fallback source.

For new chunked jobs:

- `calc.params_key` may be empty, absent, or explicitly marked legacy-only
  depending on the final schema.
- consumers should not dereference it directly without the adaptor.

Do not replace consumer logic in place with open-coded storage checks.
Add the helper first, then move call sites onto it.

## Consumer Migration Rules

### Coeffgen chunk workers

First migration target.

Current:

- range-read from global `params.bin`

Future:

- if `chunk params_key` exists, download that object directly
- otherwise fall back to the current global `params_key` + range read
- this should go through the shared adaptor, not bespoke code inside the
  coeffgen handler

### Solve-score param render/palette workers

Do not switch them first.

Current behavior is correct and already tested against:

- `params_key`
- `step_start`
- `step_count`

Migration rule:

- preserve current global path
- later optionally let workers use `chunk_items[].params_key`
- only after exact regression tests exist for:
  - render single-thread
  - render MT
  - associated palette
  - palette extract
  - solve histogram / clip

### Lores solve-score paths

Keep:

- `lores_params.bin`

Do not invent chunked lores param layout.

The lores clip/debug path is simpler and already stable as one object.

## Solve-Score Compatibility Requirements

This change must not alter solve-score chip syntax or semantics.

That means:

- `pm` remains the param metric source family
- `t1_*` / `t2_*` chip names do not change
- score programs do not gain new syntax just because param storage changed

The only acceptable backend change is storage indirection.

In other words:

- solve-score compilation stays the same
- solve-score evaluation must continue to receive the right param rows

Any chunked-param migration that leaks into chip syntax is a design failure.

## Suggested Phase Breakdown

## Phase 1: UI + internal MT, no storage change

Status: implemented.

Deliverables:

- compute MT popup in `index.html`
- payload fields:
  - `param_gen_threads`
  - `lores_param_gen_threads`
  - `coeffgen_threads`
  - `lores_coeffgen_threads`
- `handler_coeffgen.py` forwards thread counts into native `param_gen`
- native `sweep_cli.c` gets a threaded `param_gen` implementation
- native `sweep_cli.c` gets a threaded `coeffgen_chunked` implementation
- output artifacts unchanged:
  - `params.bin`
  - `lores_params.bin`

Success criteria:

- no changes needed in coeffgen consumers
- no changes needed in solve-score param chips
- compute time for param-gen meaningfully drops on larger jobs

## Phase 2: optional chunked params fan-out

Status: implemented.

Deliverables:

- `param_gen` fan-out writes chunk-local params
- `chunk_items[].params_key` metadata
- coeffgen chunk workers prefer chunk-local params
- shared param-source adaptor exists before consumers migrate

Success criteria:

- coeffgen no longer depends on global range reads when chunk params exist
- old jobs and old calc metadata still work
- new jobs do not spend time assembling a redundant full-res `params.bin`

## Phase 3: careful consumer adoption

Status: implemented.

Deliverables:

- optional render/palette worker support for chunk-local params
- global fallback preserved

Success criteria:

- param-source solve-score chips continue to work identically
- no regressions in histogram debug, render, palette, or associated palette

## Test Plan

### UI / plan tests

Status: implemented for Phase 1.

Add tests for:

- popup opens from `Calculate-AE-MT`
- thread fields are clamped/sanitized
- payload includes `param_gen_threads`
- payload includes `lores_param_gen_threads`
- payload includes `coeffgen_threads`
- payload includes `lores_coeffgen_threads`

Targets:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- Playwright only if the popup interaction is non-trivial

### Workflow / plan tests

Status: implemented for Phase 1 thread propagation.

Add/update:

- [tests/test_compute_workflow_definition.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_compute_workflow_definition.py)
- [tests/test_compute_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_compute_plan.py)

Verify:

- full param-gen task receives thread controls
- lores param-gen task receives thread controls
- no existing compute fields regress

### Native param-gen tests

Status: implemented for handler/native contract and runtime smoke coverage.

Add native tests for:

- output size correctness
- row count correctness
- no truncation/corruption under multiple thread counts
- no overlap or gap in row ordering
- same file length and traversal shape as single-thread path

Do not require bitwise equality when stochastic param transforms are present.

Do require exact equality for deterministic transform sets.

### Coeffgen compatibility tests

Status: implemented for chunked param generation and handler/workflow
contracts.

Coverage includes:

- deterministic ranged `param_gen` output equals the corresponding full-stream
  byte slice
- ranged single-thread and ranged multi-thread output match
- `coeffgen_chunked` receives per-chunk params source metadata

Targets:

- [tests/test_chunking.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_chunking.py)
- new dedicated param-chunk layout tests if needed

### Solve-score regression tests

Mandatory:

- existing `pm` chip tests stay green
- render single-thread param-source cases
- render MT param-source cases
- associated palette param-source cases
- solve histogram / clip param-source cases

Targets:

- [tests/test_solve_proximity_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_handler.py)
- [tests/test_solve_proximity_stats.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_stats.py)
- [tests/test_raster_pixel_bins.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_pixel_bins.py)
- [tests/test_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_mt.py)
- [tests/test_palette_chunk_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_chunk_handler.py)
- [tests/docker_runtime_regression.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/docker_runtime_regression.py)

### Runtime gate

If the native binary changes:

- `bash scripts/test-docker-runtime.sh` is mandatory

Current rule: if `lambda/sweep_cli.c`, `lambda/roots2pix*.c`,
`lambda/pixbinassemble.c`, or any other deployed native source changes, rebuild
the corresponding deploy binary before running the Docker/runtime gate. Do not
wait for the test to fail because the packaged binary is stale.

## Rollback / Safety

The safest rollback path is:

- keep single-thread `param_gen` code path available
- keep global `params.bin` contract available
- gate chunked params behind additive metadata, not destructive replacement

If MT `param_gen` regresses:

- drop thread count to `1`

If chunked params regress:

- consumers fall back to:
  - `params_key`
  - `step_start`
  - `step_count`

That rollback posture is exactly why phase 1 should not replace the global
params artifact.

## Recommendation Summary

Current recommendation:

1. keep using chunk-local full-res params for new compute jobs
2. keep `lores_params.bin` global
3. keep the legacy global `params.bin` fallback for old jobs
4. preserve all existing param-source solve-score contracts

Do not add:

- a full-res params assembly step
- solve-score chip syntax that exposes storage layout
- rewrite solve-score param chips to know about storage layout
- let storage refactors leak into chip syntax

If chunked params are added later, make them additive and optional.

That is the path that improves performance without breaking solve-score.
