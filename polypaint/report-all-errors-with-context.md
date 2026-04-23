# Report All Errors With Context

Status: plan.

## Problem

PolyPaint still has too many failure paths that collapse into a useless final
string:

- raw native `stderr`
- `RuntimeError(str(e))`
- `Render failed: ...`
- `Favorites refresh failed: ...`

That loses the facts needed to debug the run:

- what object failed
- which job/task/phase failed
- which chunk/section/range failed
- which mode/input/thread/worker settings were active
- whether the failure came from S3, subprocess launch, native code, HTTP, DDB,
  or frontend request handling

This document is the rule for fixing that everywhere.

## Concrete Observed Failure

This is not hypothetical. A real shipped render failed with:

```text
Render failed: solve_proximity_hist_sectioned failed for s3://polypaint/renders/compute_mnqahs0d/chunk_60.bin (clip=s3://polypaint/renders/compute_mnqahs0d/solve_scores/clusteriness_clip.json, job=compute_mnqahs0d, task=render_run_1775668030789_9ezdox_solve_score_hist_60, chunk=60, input=sectioned, size=28000000, threads=6, metric=clusteriness): range GET failed for bytes 4667040-9334079: The requested URL returned error: 500
```

What this proved:

- the old error surface was inadequate
- after context was added, we could immediately see:
  - exact source object
  - exact clip artifact
  - exact job/task/chunk
  - exact input mode and thread count
  - exact byte range that failed

What the context also proved:

- this was not malformed section math
- the failing byte range was a valid solve-aligned section for that chunk
- the actual failure was a server-side HTTP `500` on a valid range GET
- current sectioned hist treats that as fatal

This is the standard to meet:

- errors must make this level of diagnosis possible from the UI log alone
- without manual code inspection just to understand what failed

## Goal

Every operator-visible error in PolyPaint must answer:

1. What failed?
2. Where did it fail?
3. Which inputs were involved?
4. Which execution settings were active?
5. What low-level error actually came back?

Not just in CloudWatch. In the actual UI log and status path.

## Hard Rule

No layer is allowed to throw away context if it already knows it.

If a layer knows:

- `job_id`
- `task_id`
- `phase`
- `source_key`
- `chunk_idx`
- `threads`
- `workers`
- `input_mode`
- `size`

then the layer must preserve that information when reporting the error.

## Error Envelope

Every async worker error should be representable as:

- `message`
  - short human-readable summary
- `job_id`
- `task_id`
- `phase`
- `phase_label`
- `subsystem`
  - `hist`, `raster`, `merge`, `finalize`, `results`, `favorites`, etc.
- `source_bucket`
- `source_key`
- `source_size`
- `clip_key`
- `out_key`
- `chunk_idx`
- `section_idx`
- `byte_range`
- `threads`
- `workers`
- `input_mode`
- `metric`
- `color_mode`
- `artifact_id`
- `command`
  - binary name only, not full giant argv unless needed
- `returncode`
- `http_status`
- `stderr_summary`
- `updated_at_ms`

Not every field applies everywhere. Missing fields are fine. Missing known
fields are not.

## Layer Rules

### 1. Native binaries

Current problem:

- many native paths only emit a plain `fprintf(stderr, "...")`
- handlers then forward only the text

Plan:

- keep one concise human stderr line
- add enough native detail to that line to identify the failing unit of work
- especially for range/S3/sectioned paths, include:
  - byte range
  - section index if known
  - HTTP status if known
  - curl/lib-level error text

Preferred direction:

- standardize native fatal errors as:
  - one readable line
  - optionally one machine-readable JSON line on stderr later

First targets:

- `solve_proximity_hist_sectioned.c`
- `roots2pix_mt.c`
- finalize helper binaries if they have opaque errors

### 2. Lambda handlers

Current problem:

- handlers often know the full context
- then throw `RuntimeError(f\"binary failed: {stderr}\")`
- losing object keys and execution settings

Rule:

- each handler builds a `progress`/context dict at start
- that dict is enriched as inputs are discovered
- any raised error must include the relevant context already known in the
  handler

Required handler context when applicable:

- `job_id`, `task_id`, `phase`
- input object key(s)
- output key(s)
- chunk/section index
- mode/metric/input mode
- thread/worker counts
- sizes

Rule for `except` blocks:

- always call `report_status(..., "error", str(e), result_data=progress)`
- `progress` must already contain the useful context

### 3. `report_status` / DDB rows

Current problem:

- some failures have `error_msg`
- some useful context only exists in `result_data`
- some UI paths only read `error_msg`

Rule:

- error rows must store both:
  - `error_msg`
  - `result_data`

- `result_data` on error rows must preserve the current context envelope as far
  as it is known

This is the source of truth for async task failures.

### 4. `/check-status`

Current problem:

- error aggregation has historically returned too little detail

Rule:

- `error_details[]` must include:
  - `task_id`
  - `error_msg`
  - parsed `result_data` when available

This is what lets the browser log:

- the actual failure message
- plus the context block under it

### 5. Browser/UI logging

Current problem:

- many UI paths still log just `e.message`
- useful backend context may exist but not be shown

Rule:

- every operator log should show:
  - one headline error line
  - one or more indented context lines when context exists

Example:

```text
Render failed: solve_proximity_hist_sectioned failed: range GET failed for bytes 9334080-14000559: The requested URL returned error: 503
  context: task=solve_prox_hist_333 · phase=hist · chunk=333 · metric=centroid_re · input=sectioned · threads=6 · source=s3://polypaint/renders/compute_x/chunk_333.bin · size=18666480 · clip=renders/compute_x/solve_scores/clip.json
```

Rules:

- do not hide the backend error behind a vague frontend rephrase
- do not rely on browser wall time for backend failure timing
- keep status text short, but logs rich

## Required Coverage By Surface

### Render

Must report:

- orchestrator dispatch failures
- phase worker failures
- binary failures
- range-read failures
- merge/finalize failures

With context:

- `job_id`
- `task_id`
- `phase`
- source keys
- chunk/section index
- mode
- thread/worker counts

### Results

Must report:

- refresh request failure
- backend list failure
- degraded metadata fan-in

With context:

- worker count
- pool size
- timing breakdown
- degraded/error count

### Favorites

Must report:

- DDB read/write failures
- migration failures
- malformed favorite rows

With context:

- favorite ref
- operation
- backing store

### Palette / DeepZoom / Preview / Export

Must report:

- source artifact ids
- source object keys
- palette ids / image keys
- output keys when relevant

## Implementation Order

### Phase 1: Standardize render failures

- render async workers
- `report_status(..., "error", ..., result_data=progress)`
- `/check-status` preserves `result_data`
- render UI logs context lines

This is the highest-priority path because it is where long-running failures are
most expensive and most opaque.

### Phase 2: Standardize all subprocess-backed handlers

Apply the same pattern to:

- palette
- preview
- export
- coeffgen
- sweep
- bilevel

### Phase 3: Standardize request/response UI actions

For direct UI actions:

- log backend endpoint name
- log key parameters
- log backend error body when available
- keep the operator-visible line readable

### Phase 4: Native stderr convention

Introduce a shared convention for native binaries:

- concise human failure line
- optional structured error line

This is desirable, but it comes after the handler/UI plumbing because the
plumbing is what decides whether the user sees the context at all.

## Testing Requirements

Every new error-context rollout must include regression tests.

At minimum:

- handler test:
  - failing subprocess/native path includes source object and execution context
- storage test:
  - `/check-status` returns `result_data` for errors
- frontend test:
  - UI logs headline error plus context line

No error-path change should ship with only happy-path tests.

## Non-Goals

This plan is not:

- a retry plan
- a rollback plan
- a monitoring/alerting plan
- a CloudWatch dashboard plan

Those may matter later. This document is only about making failures visible and
diagnosable in the actual product workflow.

## Definition of Done

This effort is done when:

- any render failure in the UI immediately identifies the failing task/object
- async task errors preserve context from worker to DDB to browser
- direct UI request failures identify the operation and relevant object/id
- new handlers follow one explicit error-context pattern instead of inventing
  their own

Until then, “error handling exists” is not considered good enough.
