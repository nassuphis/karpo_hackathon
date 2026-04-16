# Fused Compute Chunk Pipeline Plan

Status
------

Not implemented.

This plan covers a compute-pipeline refactor:

- move `lores` to the front as the probe step
- derive `degree` / `n_coeffs` before full-res dispatch
- replace the current full-res three-stage chunk pipeline
  - `param_gen`
  - `coeffgen_chunked`
  - `solve`
  with one fused per-chunk worker that runs all three stages locally


Objective
---------

Reduce compute latency and orchestration overhead without changing the external
artifact contract.

This must be introduced as an A/B-comparable rollout:

- the current compute pipeline and the fused pipeline both need to remain
  selectable from the UI while the fused path is being validated
- the user needs to be able to run the same compute job through both methods
  and compare wall time, per-phase timings, and output contract

The current full-res pipeline fans out by chunk three separate times, with S3
handoff between stages. That creates avoidable cost in:

- Lambda startup count
- Step Functions transitions
- S3 reads between stages
- repeated local materialization of data that was just produced by the
  previous step

The target shape is:

1. run `lores` first
2. use `lores coeffgen` metadata to discover `degree` / `n_coeffs`
3. compute a safe minimum hires chunk count from solver mode, Lambda memory,
   threads, and row sizes
4. fan out one fused hires worker per chunk
5. each fused worker does:
   - param gen
   - coeffgen
   - solve
   - uploads stage outputs
   - frees previous stage data before continuing
6. finalize `calc.json` as today


Short Answer
------------

Yes, this architecture is coherent.

The correct order is:

1. `lores param_gen`
2. `lores coeffgen`
3. `lores solve`
4. extract `degree` / `n_coeffs`
5. compute safe hires chunking
6. dispatch fused hires chunk workers
7. finalize metadata

That works because `lores` and `hires` are expected to produce the same
coefficient shape:

- same `degree`
- same `n_coeffs`

The important design choice is:

- v1 should fuse the worker at the Lambda orchestration level while still
  reusing the existing native binaries
- do not start with a new combined native binary
- do not require all three stage outputs to remain in RAM at the same time

The win comes mainly from removing cross-Lambda S3 handoff, not from doing
everything in one in-memory buffer.


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


Core Rule
---------

No hidden unsafe chunk sizing.

If a fused chunk worker can exceed Lambda memory or `/tmp`, the planner must
compute and expose:

- the minimum safe hires chunk count
- the expected per-chunk params size
- the expected per-chunk coeffs size
- the expected per-chunk roots size
- the assumed memory budget
- the solver mode and thread count used in the calculation

The UI must not allow a chunk count below the computed minimum.

Rollout rule:

- do not remove the old compute path before the new one can be selected and
  measured side by side from the UI


Proposed Architecture
---------------------

## 1. Lores-first probe

Move lores ahead of full-res fan-out.

Flow:

1. lores param gen
2. lores coeffgen
3. lores solve
4. read lores coeffgen metadata
5. derive:
   - `degree`
   - `n_coeffs`

This replaces the current need to run full-res coeffgen before lores sizing.

## 2. Safe hires chunk sizing

After lores coeffgen completes, the planner computes a safe minimum hires chunk
count.

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

The planner must reject any requested `n_chunks < min_safe_chunks`.

## 3. Fused hires chunk worker

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
  - thread counts
  - output keys:
    - params
    - coeffs
    - roots
- output:
  - params metadata
  - coeff metadata
  - solve metadata

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

The key point is:

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


UI Changes
----------

`Compute -> Calculate-AE-MT` should gain a job-size / sizing section similar in
spirit to the render-side MT popup.

It should also expose an explicit execution-method selector.

Recommended values:

- `classic_chunk_pipeline`
- `fused_chunk_pipeline`

Meaning:

- `classic_chunk_pipeline`
  - existing full-res execution shape
  - `param_gen -> coeffgen -> solve`
  - separate Step Functions phases and Lambdas
- `fused_chunk_pipeline`
  - lores-first probe
  - one fused hires worker per chunk

It should show:

- selected execution method
- total steps
- requested hires chunk count
- minimum safe hires chunk count
- params bytes per full chunk
- coeff bytes per full chunk
- roots bytes per full chunk
- estimated peak bytes per fused chunk
- fused worker Lambda memory
- solver mode
- thread count used in sizing

Rules:

- the execution method must be visible in the popup, persisted, and restored by
  results populate
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


Workflow Changes
----------------

## New order

Replace the current sequence:

- full-res param gen
- full-res coeffgen
- post coeffgen
- lores param gen
- lores coeffgen
- lores solve
- full-res solve

with:

- lores param gen
- lores coeffgen
- lores solve
- post lores coeffgen probe
- full-res fused chunk map
- finalize metadata

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

That is acceptable for v1 if:

- retries stay chunk-local
- chunk sizes are safe
- stage timings are logged clearly

Possible later improvement:

- checkpoint reuse inside the fused worker
- if params or coeffs already exist and validate, skip rebuilding them on retry

That is useful, but not required for the first rollout.


Phased Rollout
--------------

## Phase 1: Lores-first probe + sizing UI

Implement:

- move lores before hires fan-out
- derive `degree` / `n_coeffs` from lores coeffgen
- add safe hires chunk sizing calculation
- add compute execution-method selector to the UI
- persist and restore that selection
- expose safe chunk count in the compute popup
- reject unsafe manual chunk counts

Do not yet fuse full-res execution.

This phase proves:

- sizing logic
- UI behavior
- execution-method persistence
- A/B comparison setup

## Phase 2: Fused hires chunk Lambda using existing binaries

Implement:

- new fused hires worker phase
- local sequential execution:
  - param gen
  - coeffgen
  - solve
- same S3 output contract as today
- same `calc.json` output fields as today

Remove:

- full-res `ParamGenMap`
- full-res `CoeffgenMap`
- full-res `SolveMap`

Replace them with:

- one `FusedChunkMap`

Keep lores separate and first.

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
- optional checkpoint reuse on retry

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

For new jobs, the artifact names and metadata fields should remain compatible
enough that render/palette code does not need a second parallel contract.


Testing Plan
------------

## Planner tests

Add tests for:

- lores-first plan order
- safe-min chunk calculation
- unsafe manual chunk rejection
- auto chunk selection
- solver-specific sizing branches

Targets:

- [lambda/handler_compute_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_plan.py)
- corresponding workflow-definition tests

## Worker tests

Add tests for:

- fused worker produces:
  - params chunk
  - coeffs chunk
  - roots chunk
- metadata matches current contract
- stage-local cleanup happens
- retry of a failed fused chunk is safe

## End-to-end tests

Add coverage for:

- UI can select both classic and fused compute methods
- selected method persists into results metadata and populate
- `Calculate-AE-MT` popup reflects safe min chunks
- unsafe smaller chunk count is rejected
- successful fused compute still produces a normal `calc.json`
- render / solve-score / palette paths still consume the output unchanged
- same compute job can be run through both methods for timing comparison


Checklist
---------

- [ ] Add compute execution-method selector to the UI:
      `classic_chunk_pipeline` vs `fused_chunk_pipeline`.
- [ ] Persist and restore the selected execution method.
- [ ] Show the selected execution method in compute logs and result metadata.
- [ ] Move lores ahead of full-res fan-out.
- [ ] Add lores coeff probe result handling for `degree` / `n_coeffs`.
- [ ] Add safe hires chunk count calculation.
- [ ] Expose safe min chunk count in `Calculate-AE-MT`.
- [ ] Add `auto hires chunks` mode.
- [ ] Reject unsafe manual chunk counts.
- [ ] Add fused hires chunk worker phase.
- [ ] Keep the classic full-res pipeline available in parallel during rollout.
- [ ] Reuse current paramgen / coeffgen / solve binaries locally in the fused
      worker.
- [ ] Keep current S3 artifact naming for params / coeffs / roots.
- [ ] Keep current `calc.json` fields.
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

The clean execution shape is:

- `lores` first
- use lores to discover output shape
- compute safe hires chunking
- one fused hires worker per chunk

That keeps the artifact contract stable while removing a large amount of
orchestration and S3 handoff overhead.
