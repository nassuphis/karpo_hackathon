# Fused Compute Chunk Pipeline Plan

Status
------

Not implemented.

This plan covers a compute-pipeline refactor:

- add a tiny dedicated `degree_probe` step first
- derive `degree` / `n_coeffs` before any full-res dispatch
- use that probe result to size `lores` and validate hires chunking
- keep the current compute pipeline intact as `classic_chunk_pipeline`
- add a second independent `fused_chunk_pipeline`
- replace the current full-res three-stage chunk pipeline in the fused branch
  - `param_gen`
  - `coeffgen_chunked`
  - `solve`
  with one fused per-chunk worker that runs all three stages locally


Objective
---------

Reduce compute latency and orchestration overhead without changing the external
artifact contract.

Hard invariant:

- for new jobs, `fused_chunk_pipeline` must emit the same external compute
  contract as `classic_chunk_pipeline`
- render, palette, solve-score, preview, and any other downstream consumer
  must be able to consume fused outputs without branching on execution method
- any fused-only information is additive metadata only

This must be introduced as an A/B-comparable rollout:

- the current compute pipeline and the fused pipeline both remain selectable
  from the UI while the fused path is validated
- the user can run the same compute job through both methods and compare wall
  time, per-phase timings, and output contract

The user-facing control should be simple:

- add a `Fused` checkbox to `Compute -> Calculate-AE-MT`
- unchecked:
  - run `classic_chunk_pipeline`
- checked:
  - run `fused_chunk_pipeline`

The backend/storage field should still remain explicit:

- `execution_method = classic_chunk_pipeline`
- `execution_method = fused_chunk_pipeline`

The current full-res pipeline fans out by chunk three separate times, with S3
handoff between stages. That creates avoidable cost in:

- Lambda startup count
- Step Functions transitions
- S3 reads between stages
- repeated local materialization of data that was just produced by the
  previous step

The target fused shape is:

1. run a tiny `degree_probe`
2. use that probe to discover `degree` / `n_coeffs`
3. compute a safe minimum hires chunk count from solver mode, Lambda memory,
   fused worker thread count, and row sizes
4. size and run `lores`
5. fan out one fused hires worker per chunk
6. each fused worker does:
   - param gen
   - coeffgen
   - solve
   - uploads stage outputs
   - frees previous stage memory / local files before continuing
7. finalize `calc.json` as today


Short Answer
------------

Yes, this architecture is coherent.

The correct order for the fused path is:

1. `degree_probe param_gen`
2. `degree_probe coeffgen`
3. extract `degree` / `n_coeffs`
4. compute safe hires chunking
5. choose `lores N`
6. `lores param_gen`
7. `lores coeffgen`
8. `lores solve`
9. dispatch fused hires chunk workers
10. finalize metadata

That works because the probe, `lores`, and `hires` are expected to produce the
same coefficient shape:

- same `degree`
- same `n_coeffs`

The important design choices are:

- v1 fuses the worker at the Lambda orchestration level while still reusing
  the existing native binaries
- do not start with a new combined native binary
- do not require all three stage outputs to remain in RAM at the same time
- do not mutate the classic branch while fused is being introduced

The win comes mainly from removing cross-Lambda S3 handoff, not from doing
everything in one in-memory buffer.

For v1, the fused path is still a Python-orchestrated Lambda worker that
reuses the existing native binaries in sequence. This is intentional. The
storage contract stays stable; only execution shape changes.


Current State
-------------

Today the compute workflow is:

1. full-res `ParamGenMap`
2. full-res `CoeffgenMap`
3. `post_coeffgen`
4. lores param gen
5. lores coeffgen
6. lores solve
7. full-res `SolveMap`
8. finalize metadata

References:

- [stepfunctions/compute_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/compute_workflow.asl.json.template)
- [lambda/handler_compute_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_plan.py)
- [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py)
- [lambda/handler_sweep.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_sweep.py)

For `C` full-res chunks, hires alone currently uses roughly:

- `C` param-gen Lambdas
- `C` coeffgen Lambdas
- `C` solve Lambdas

plus the lores path and metadata/finalize steps.

That means the same chunk is processed by three different Lambdas with two S3
handoffs between them.


Core Rules
----------

No hidden unsafe chunk sizing.

If a fused chunk worker can exceed Lambda memory or `/tmp`, the planner must
compute and expose:

- the minimum safe hires chunk count
- the expected per-chunk params size
- the expected per-chunk coeffs size
- the expected per-chunk roots size
- the assumed memory budget
- the solver mode and fused worker thread count used in the calculation

The UI must not allow a chunk count below the computed minimum.

Rollout rules:

- do not remove the old compute path before the new one can be selected and
  measured side by side from the UI
- do not quietly mutate the classic path while bringing up fused
- `classic_chunk_pipeline` remains the current workflow shape
- `fused_chunk_pipeline` is added as a second independent branch


Proposed Architecture
---------------------

## 1. Degree probe first

Add a tiny dedicated probe ahead of everything else.

Goal:

- determine `degree`
- determine `n_coeffs`
- do it cheaply enough that the UI can use the same logic for validation

Recommended shape:

- separate small Lambda route, conceptually similar to compute preview
- use a fixed tiny grid such as `N=5`, `times=1`
- run only:
  - param gen
  - coeffgen
- no solve required

Reason:

- the UI needs `degree` / `n_coeffs` before it can validate minimum hires
  chunks for the fused path
- `lores N` itself should depend on `degree`, so `lores` cannot be the first
  thing if the sizing model is to stay honest

Probe outputs:

- `degree`
- `n_coeffs`
- `probe_N`
- `probe_step_count`
- `param_gen_us`
- `coeffgen_us`
- optional:
  - `params_size`
  - `coeffs_size`

Probe robustness rule:

- the probe must validate shape stability across more than one sample point
- minimum acceptable check:
  - run two small probe samples at distinct grid positions
  - require identical `degree` and `n_coeffs`
- if probe samples disagree, reject the fused path for that job and surface:
  - `probe_stable = false`
  - the conflicting shape values

Probe / planner handshake:

- add a dedicated probe action before fused planning
- the probe returns:
  - `probe_degree`
  - `probe_n_coeffs`
  - `probe_signature`
  - `probe_stable`
- the fused plan request consumes those exact fields
- the server must revalidate the signature and rerun the probe if the request
  omitted probe data or supplied stale probe data

The probe result should be cached in compute metadata for the run so the popup
and orchestrator use the same discovered values.

## 2. Lores after probe

Run `lores` only after the degree probe is known.

Flow:

1. choose `lores N` from the probed `degree`
2. lores param gen
3. lores coeffgen
4. lores solve

This replaces the current need to run full-res coeffgen before lores sizing,
while still keeping lores honest to the real polynomial degree.

For v1, lores sizing should reuse the current formula exactly:

- `lores_n = min(N, max(5, ceil(sqrt(TARGET_PREVIEW_ROOTS / max(1, degree * times)))))`

That keeps lores comparable to the current classic pipeline while only moving
the discovery point earlier.

## 3. Safe hires chunk sizing

After the degree probe completes, the planner computes a safe minimum hires
chunk count.

Inputs:

- solver mode
  - `aberth`
  - `aberth_mt`
  - `companion_matrix`
- Lambda memory for the fused worker
- fused worker thread count
- `N`
- `times`
- `degree`
- `n_coeffs`

Outputs:

- `min_safe_chunks`
- representative per-chunk:
  - params bytes
  - coeff bytes
  - roots bytes
- estimated peak bytes
- sizing reason / budget breakdown
- limiting floor:
  - `memory`
  - `/tmp`
  - `timeout`

The planner must reject any requested `n_chunks < min_safe_chunks`.

The safe count calculation must be based on the worst stage inside the fused
worker, not on an average across stages.

Rule:

- `min_safe_chunks = max(memory_floor_chunks, tmp_floor_chunks, timeout_floor_chunks)`

## 4. Fused hires chunk worker

Each full-res work item becomes one fused chunk Lambda.

Worker contract:

- input:
  - `chunk_idx`
  - `step_start`
  - `step_count`
  - param transform chain
  - coeff transform chain
  - function
  - solver mode
  - fused worker thread count
  - output keys:
    - params
    - coeffs
    - roots
- output:
  - one chunk result record

Thread model:

- one Lambda owns one not-yet-generated chunk
- inside that Lambda, each stage runs to completion before the next starts
- each stage is locally multithreaded
- all stage threads write into shared output buffers/files for that stage
- once a stage completes:
  - upload its artifact
  - emit timing/byte logs
  - free memory and/or unlink local files no longer needed
  - proceed to the next stage

Stage threading:

- param gen uses `fused_threads`
- coeffgen uses `fused_threads`
- solve uses `fused_threads`

The fused sizing model must use this exact `fused_threads` value.

For v1, `fused_threads` is one knob that applies to all three stages inside the
fused Lambda.

The classic path keeps its existing stage-specific knobs:

- `param_gen_threads`
- `coeffgen_threads`
- `lores_param_gen_threads`
- `lores_coeffgen_threads`

Those classic knobs are not reused as hidden defaults for the fused path.
Fused gets its own explicit control.

The fused worker result must be backward-compatible with the classic solve
result that [lambda/handler_compute_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_plan.py)
already consumes in `finalize_metadata`.

Required top-level fields:

- `chunk_idx`
- `s3_key`
- `bin_size`
- `compute_us`
- `n_t`
- `degree`
- `avg_iterations`

Required additional fused fields:

- `params_key`
- `params_size`
- `params_step_start`
- `params_step_count`
- `coeffs_key`
- `coeffs_size`
- `param_gen_us`
- `coeffgen_us`
- `solve_us`
- `upload_params_us`
- `upload_coeffs_us`
- `upload_roots_us`
- `param_gen_threads`
- `coeffgen_threads`
- `fused_threads`
- `peak_estimated_bytes`
- `execution_method`

Recommended persisted metadata fields:

- `execution_method`
- `fused_threads`
- `auto_hires_chunks`
- `probe_degree`
- `probe_n_coeffs`
- `probe_signature`
- `min_safe_chunks`
- `safe_chunk_limit_reason`

Rule:

- the classic fields stay byte-for-byte compatible in meaning
- the fused fields are additive only
- `finalize_metadata` should be able to consume fused results with no lossy
  translation step
- `calc.json` for fused jobs should remain identical in shape to classic jobs,
  with optional additive metadata only

Execution inside one Lambda:

1. generate params for this chunk
2. upload params chunk
3. generate coeffs from the local params artifact
4. upload coeffs chunk
5. release params local artifact
6. solve roots from the local coeffs artifact
7. upload roots chunk
8. release coeffs local artifact
9. emit one combined chunk result record

The key points are:

- stage outputs still get persisted
- cross-stage reuse happens locally inside one Lambda
- downstream render/palette/debug consumers continue to see the same
  `params_0000.bin`, `coeffs_0000.bin`, and `chunk_0.bin` contract


Why This Works
--------------

This design preserves the existing data model while changing execution shape.

What stays stable:

- `calc.json` still lists:
  - chunk root objects
  - chunk coeff objects
  - chunk param objects
- lores artifacts remain:
  - `lores_params.bin`
  - `lores_coeffs.bin`
  - `lores.bin`
- render, palette, solve-score, and preview code keep using the same artifact
  names and metadata fields

So this is an execution refactor, not a storage-contract rewrite.


Why Not Start With One Giant Native Binary
------------------------------------------

That is a reasonable end state, but it is the wrong first step.

The smaller, safer first version is:

- one fused Python Lambda worker
- reuse existing native subprocesses in sequence
- reuse current chunk artifact names
- reuse current metadata shapes
- keep stage handoff local instead of through S3

This gets most of the speedup while changing much less.

The combined native binary can come later if profiling shows subprocess
boundary cost is still significant.


Memory Model
------------

The safe chunk count must be computed from a conservative peak-memory model.

Minimum inputs:

- params bytes per step:
  - `16`
- coeff bytes per step:
  - `n_coeffs * 2 * 4`
- roots bytes per step:
  - `degree * 2 * 4`
- solver-specific scratch factor
- fixed process overhead
- per-thread overhead
- `/tmp` overhead if local file staging is used
- safety margin

This must be solver-specific.

The budget formula should not pretend all solvers have the same scratch cost.

At minimum:

- `aberth`
- `aberth_mt`
- `companion_matrix`

need separate sizing constants.

The first version can use conservative hard-coded solver multipliers. Those can
be tightened later from measurements.

`/tmp` budget is a first-class constraint, not a footnote.

Peak local disk use should be estimated as:

- `max(params_file + coeffs_file, coeffs_file + roots_file)`

because params are released before solve begins, but coeffs overlap with both
adjacent stages.

The safe chunk count must satisfy both:

- memory budget
- `/tmp` budget

and the stricter one wins.


Timeout Model
-------------

The fused worker runs three stages sequentially, so timeout is also a hard
floor on safe chunk sizing.

The planner must estimate per-chunk fused wall time from:

- param-gen wall time
- coeffgen wall time
- solve wall time
- upload time for params / coeffs / roots
- fixed Lambda/process overhead
- safety margin

Rules:

- if estimated fused chunk wall time can exceed the selected Lambda timeout,
  increase the minimum safe chunk count
- if the fused method still cannot fit within timeout after reaching practical
  chunking limits, reject fused for that job and direct the user to classic
- timeout must be reported as a possible limiting floor in the sizing UI


UI Changes
----------

`Compute -> Calculate-AE-MT` should gain a job-size / sizing section similar in
spirit to the render-side MT popup.

It should expose a user-facing `Fused` checkbox, not a verbose method dropdown.

Meaning:

- `Fused = off`
  - run `classic_chunk_pipeline`
  - existing current compute workflow, unchanged
  - `param_gen -> coeffgen -> solve`
  - separate Step Functions phases and Lambdas
- `Fused = on`
  - run `fused_chunk_pipeline`
  - degree probe first
  - one fused hires worker per chunk

It should show:

- selected execution method
- discovered `degree`
- discovered `n_coeffs`
- total steps
- requested hires chunk count
- minimum safe hires chunk count
- params bytes per full chunk
- coeff bytes per full chunk
- roots bytes per full chunk
- estimated peak bytes per fused chunk
- fused worker Lambda memory
- fused worker `/tmp` budget
- fused worker timeout
- solver mode
- fused worker thread count used in sizing
- safe chunk limiting floor

Rules:

- the `Fused` checkbox state must be visible in the popup, persisted, and
  restored by results populate
- `classic_chunk_pipeline` and `fused_chunk_pipeline` must not share hidden
  sequencing changes
- the chunk count control must display the safe minimum next to it
- the user must be able to select the safe minimum directly
- the user must not be allowed to choose a smaller chunk count
- if `classic_chunk_pipeline` is selected, still show the fused safe minimum as
  sizing guidance, but do not validate the classic path against fused-only
  limits
- if `fused_chunk_pipeline` is selected, enforce the fused safe minimum as a
  hard rule

Optional mode:

- `auto hires chunks`

If enabled:

- planner sets `n_chunks = min_safe_chunks`

If manual:

- planner validates `n_chunks >= min_safe_chunks`

Persisted execution config should use explicit field names:

- `execution_method`
- `fused`
- `fused_threads`
- `auto_hires_chunks`
- `probe_degree`
- `probe_n_coeffs`
- `probe_signature`


Workflow Changes
----------------

## Classic branch

Do not change the classic branch while fused is being introduced.

The current classic sequence stays:

- full-res param gen
- full-res coeffgen
- post coeffgen
- lores param gen
- lores coeffgen
- lores solve
- full-res solve

This is the A/B baseline.

## Fused branch

The fused branch is a fresh sequence:

- degree probe param gen
- degree probe coeffgen
- fused sizing / lores sizing
- lores param gen
- lores coeffgen
- lores solve
- fused hires chunk map
- finalize metadata

The fused branch must normalize its probe result into the same metadata shape
the classic branch currently gets from `post_coeffgen`.

Required normalized `post` contract before `finalize_metadata`:

- `degree`
- `n_coeffs`
- `total_coeffs_size`
- `lores`
  - `N`
  - `n_steps`
  - `params_key`
  - `coeffs_key`
  - `bin_key`
  - `param_gen_threads`
  - `coeffgen_threads`
  - `param_task_id`
  - `coeff_task_id`
  - `solve_task_id`

Allowed additive fields:

- `execution_method`
- `probe_degree`
- `probe_n_coeffs`
- `probe_signature`
- `fused_threads`
- sizing / timing diagnostics

The implementation may call this object `post_probe`, `probe_metadata`, or
similar internally, but before `finalize_metadata` it must be normalized to
the same nested shape the classic branch currently gets from `post_coeffgen`.
`finalize_metadata` should not need a separate fused-only schema branch.

The fused branch should expose its own knob:

- `fused_threads`

That value controls local multithreading for param gen, coeffgen, and solve
inside each fused chunk Lambda.

## New worker phase

Add a new full-res phase, for example:

- `compute_chunk_fused`

That worker owns:

- one chunk
- all three hires stages for that chunk

Status reporting should expose both:

- combined wall time
- per-stage timings inside the worker

For example:

- `param_gen_us`
- `coeffgen_us`
- `solve_us`
- `upload_params_us`
- `upload_coeffs_us`
- `upload_roots_us`


Failure / Retry Semantics
-------------------------

This is the main tradeoff.

Today:

- solve can fail independently of paramgen and coeffgen

In a fused worker:

- a retry may rerun the whole chunk

That is acceptable only if retries stay chunk-local and artifact reuse is
cheap.

Phase-2 requirement:

- before each stage, check whether the required prior-stage artifact already
  exists and validates
- if params already exist and validate, skip local param gen
- if coeffs already exist and validate, skip local coeffgen
- if roots exist and validate, the chunk can short-circuit as already complete

This is basic checkpoint reuse, not an optional later enhancement.


Phased Rollout
--------------

## Phase 1: Degree probe + sizing UI

Implement:

- add degree probe step
- derive `degree` / `n_coeffs` from the degree probe
- add safe hires chunk sizing calculation
- add compute execution-method selector to the UI
- persist and restore that selection
- expose safe chunk count in the compute popup
- reject unsafe manual chunk counts

Do not yet fuse full-res execution.
Do not mutate the classic branch.

This phase proves:

- degree probe contract
- sizing logic
- UI behavior
- execution-method persistence
- A/B comparison setup

## Phase 2: Fused hires chunk Lambda using existing binaries

Implement:

- separate fused workflow branch
- new fused hires worker phase
- local sequential execution:
  - param gen
  - coeffgen
  - solve
- same S3 output contract as today
- same `calc.json` output fields as today
- basic checkpoint reuse for params / coeffs / roots on retry

Keep lores separate and first in the fused branch.

Important rollout constraint:

- do not delete the classic full-res path in this phase
- keep both workflow branches available behind the UI selector
- only consider removing the classic path after real timing comparisons show
  the fused path is faster and stable

## Phase 3: Logging and resume hardening

Implement:

- per-stage timing logs
- per-stage byte counts
- explicit peak-size estimates in status rows
- stricter checkpoint validation / reuse hardening if Phase 2 basic reuse needs
  more guardrails

## Phase 4: Native combined chunk pipeline

Optional, only if profiling justifies it.

Implement:

- one native binary for:
  - param gen
  - coeffgen
  - solve
- shared thread pool
- shared buffers
- fewer subprocess boundaries


Compatibility Requirements
--------------------------

Must not break:

- existing render consumers of chunked root artifacts
- existing coeff debug / preview tooling
- existing param-source solve-score chips
- existing lores debug paths
- old jobs with old `calc.json`

For new jobs, fused output must be externally identical to classic output for
all currently supported downstream consumers.

That means:

- same `calc.json` top-level shape
- same `lores` object shape inside `calc.json`
- same chunk ordering and indexing
- same artifact naming:
  - `params_0000.bin`
  - `coeffs_0000.bin`
  - `chunk_0.bin`
  - `lores_params.bin`
  - `lores_coeffs.bin`
  - `lores.bin`
- same field semantics for everything render/palette/solve-score currently
  reads

Only additive metadata may differ, for example:

- `execution_method`
- `fused_threads`
- probe metadata
- sizing metadata
- per-stage timing metadata

Render and other consumers must not branch on `execution_method` to interpret
new-job fused outputs.


Testing Plan
------------

## Planner tests

Add tests for:

- degree-probe-first fused plan order
- probe stability / disagreement rejection
- safe-min chunk calculation
- unsafe manual chunk rejection
- auto chunk selection
- solver-specific sizing branches
- `/tmp`-limited safe-min chunk calculation
- timeout-limited safe-min chunk calculation
- classic branch remains unchanged

Targets:

- [lambda/handler_compute_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_plan.py)
- corresponding workflow-definition tests

## Worker tests

Add tests for:

- fused worker produces:
  - params chunk
  - coeffs chunk
  - roots chunk
- fused worker result includes the classic solve fields unchanged
- metadata matches current contract
- stage-local cleanup happens
- retry of a failed fused chunk is safe
- retry can reuse already uploaded params / coeffs when valid
- per-stage thread counts are forwarded and logged
- `fused_threads` is forwarded consistently to all three fused stages

## End-to-end tests

Add coverage for:

- UI can select both classic and fused compute methods
- selected method persists into results metadata and populate
- probe result is surfaced in the popup
- `Calculate-AE-MT` popup reflects safe min chunks
- unsafe smaller chunk count is rejected
- successful fused compute still produces a normal `calc.json`
- render / solve-score / palette paths still consume the output unchanged
- same compute job can be run through both methods for timing comparison


Checklist
---------

- [ ] Add compute execution-method selector to the UI:
      a `Fused` checkbox mapping to
      `classic_chunk_pipeline` vs `fused_chunk_pipeline`.
- [ ] Persist and restore the `Fused` checkbox state.
- [ ] Show the selected execution method in compute logs and result metadata.
- [ ] Add dedicated `degree_probe` step.
- [ ] Validate probe stability across more than one sample point.
- [ ] Surface `degree` / `n_coeffs` from the probe in the popup.
- [ ] Keep the classic workflow unchanged while fused is introduced.
- [ ] Add fused branch sequencing independent of classic.
- [ ] Add safe hires chunk count calculation.
- [ ] Include memory, `/tmp`, and timeout floors in the safe chunk
      calculation.
- [ ] Expose safe min chunk count in `Calculate-AE-MT`.
- [ ] Add `auto hires chunks` mode.
- [ ] Reject unsafe manual chunk counts.
- [ ] Add explicit `fused_threads` control to the UI.
- [ ] Use `fused_threads` consistently across paramgen, coeffgen, and solve in
      the fused worker.
- [ ] Add fused hires chunk worker phase.
- [ ] Keep the classic full-res pipeline available in parallel during rollout.
- [ ] Reuse current paramgen / coeffgen / solve binaries locally in the fused
      worker.
- [ ] Add basic checkpoint reuse for params / coeffs / roots in fused-worker
      retries.
- [ ] Keep current S3 artifact naming for params / coeffs / roots.
- [ ] Keep current `calc.json` fields.
- [ ] Keep fused new-job outputs externally identical to classic outputs for
      render/palette/solve-score consumption.
- [ ] Keep the classic solve-result fields identical in the fused worker
      result contract.
- [ ] Add per-stage timing and byte logging.
- [ ] Add planner / worker / end-to-end tests.
- [ ] Benchmark the same compute job through both UI-selectable methods before
      considering removal of the classic path.


Non-Goals
---------

Not part of this plan:

- changing render-side chunk grouping
- changing solve-score logical sectioning
- deleting legacy compute artifacts for old jobs
- switching render consumers to a new storage contract
- forcing a new all-native combined binary in phase 1


Bottom Line
-----------

This refactor is worth doing.

The clean fused execution shape is:

- degree probe first
- use the probe to discover output shape
- compute safe hires chunking
- one fused hires worker per chunk

That keeps the artifact contract stable while removing a large amount of
orchestration and S3 handoff overhead.
