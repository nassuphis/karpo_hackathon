ExtractPalette MT Plan
======================

Objective
---------

Make exact palette extraction use the same class of optimizations that were
worth the pain in Render-MT:

- native multithreaded work inside each Lambda
- sectioned solve-aligned input instead of mandatory full tmpfile download
- explicit retry handling for sectioned reads
- better phase-level perf reporting
- higher useful inter-Lambda concurrency where it is safe

The target is not only `ExtractPalette`. The real expensive stage is the shared
exact `palette_chunk` pipeline used by:

- standalone Palette generation
- `Render -> Color -> Save Associated Palette`
- `Render -> Color -> ExtractPalette`

So the MT work should land in the shared exact palette-chunk engine and then be
used by all of those flows.


Observed Problem
----------------

The current `ExtractPalette` popup is misleading.

Today it exposes:

- solve-score hist threads
- solve-score hist input mode
- solve-score hist retries
- solve-score merge workers

Those knobs only matter when extraction takes the `generate_full` path:

- solve-score clip
- solve-score hist
- solve-score merge
- palette chunk
- palette finalize

But many post-hoc extractions take the `generate_reuse` path:

- reuse saved solve-score scratch
- skip clip/hist/merge
- run only palette chunk + finalize + attach

On that reuse path, the currently exposed thread knob does not speed up the
main cost at all. It is effectively dead UI.

That is exactly what happened in the user run that took about five minutes:

- `500` chunk workers
- `PaletteChunkMap MaxConcurrency = 10`
- current `solve_palette_chunk` is single-threaded
- current `solve_palette_chunk` always downloads the whole chunk to `/tmp`
- no sectioned input
- no per-chunk MT
- no chunk-stage retry knob


Current Cost Model
------------------

For an extract run with `n_chunks = 500`:

Reuse path:

1. orchestrator
2. build plan
3. palette chunk phase marker
4. `500` `palette_chunk` workers
5. palette finalize phase marker
6. palette finalize
7. attach phase marker
8. attach palette
9. report done

That is:

- `508` Lambda invocations inside the workflow
- plus `1` orchestrator Lambda
- total `509`

Full path:

1. orchestrator
2. build plan
3. solve-score clip phase marker
4. solve-score clip
5. solve-score hist phase marker
6. `500` hist workers
7. solve-score merge phase marker
8. solve-score merge
9. palette chunk phase marker
10. `500` palette chunk workers
11. palette finalize phase marker
12. palette finalize
13. attach phase marker
14. attach palette
15. report done

That is:

- `1013` workflow Lambdas
- plus `1` orchestrator
- total `1014`

So yes, exact post-hoc extraction is expensive today.


Important Clarification About Render-MT
---------------------------------------

Chunk-heavy Render-MT phases are not running with `50` live workers today.

Current render workflow:

- solve-score hist map: `MaxConcurrency = 10`
- associated palette chunk map: `MaxConcurrency = 10`
- raster map: `MaxConcurrency = 10`
- finalize map: `MaxConcurrency = 32`

So the big win in Render-MT was not “50 live workers everywhere”.
It was:

- per-Lambda native MT
- sectioned input for the right phases
- explicit retries
- good perf visibility

That is the pattern to copy here.


Core Decision
-------------

Do not build an `ExtractPalette`-only optimization path.

Instead:

1. build a shared multithreaded exact palette-chunk engine
2. use it in:
   - palette workflow
   - render associated-palette workflow branch
3. expose knobs in `ExtractPalette` because that is the user-facing place where
   the performance pain is obvious

This avoids creating a second palette-generation system.


What Should Stay Single-Lambda
------------------------------

Keep these phases single-Lambda:

- solve-score clip
- solve-score merge
- palette finalize
- attach palette

Reason:

- clip is not the main cost
- merge is not a sectioned-read problem; it is a small reducer
- finalize assembles one final palette artifact; it is not a chunk fan-out phase
- attach is tiny metadata work

The real optimization target is `palette_chunk`, with secondary attention to
hist only when full prepass is needed.


What Must Become MT
-------------------

The shared exact palette chunk worker must gain:

- native worker threads
- `tmpfile | sectioned` input modes
- explicit sectioned retries
- detailed perf JSON
- better context on failure

Recommended implementation:

- new native binary:
  - `lambda/solve_palette_chunk_mt.c`
- new handler:
  - `lambda/handler_palette_chunk_mt.py`

Do not mutate the current single-threaded `solve_palette_chunk` into an
unrecognizable hybrid. Keep the old stable path available during rollout.


Why MT Is Safe Here
-------------------

This is a much better MT target than AE solve.

For palette chunk:

- each solve is independent
- score computation per solve is independent
- final palette bin assignment is independent once clip/cuts are fixed
- there is no cross-solve dedupe
- there is no chunk-wide iterative coupling

So:

- `threads=1` can remain exact to current behavior
- `threads>1` can also remain exact, not approximate, if we partition solves
  into contiguous ranges and write outputs by fixed index

This is unlike AE-MT, where thread boundaries change iteration behavior.


Input Strategy
--------------

The MT chunk worker should support:

1. `tmpfile`
   - current behavior
   - download whole chunk to `/tmp`
   - simple fallback

2. `sectioned`
   - presigned URL + object size
   - native range GET
   - split by solve-aligned contiguous sections
   - explicit bounded retries with backoff

The sectioned math should follow the same lessons as Render-MT:

- split only on whole-solve boundaries
- do not cut the chunk at arbitrary bytes
- each thread gets a contiguous solve range
- each thread reads only its own byte range


Worker Output Contract
----------------------

Keep the palette artifact contract unchanged.

The MT chunk worker should still emit:

- `score_key`
- `palette_bins_key`
- `meta_key`

Keep this stable for:

- existing palette reuse
- repalette
- storage summary
- downstream tooling

But improve the worker status payload to include:

- `download_us` or `dl_ms`
- `native_us` or `compute_ms`
- `upload_us`
- `threads`
- `input_mode`
- `retries`
- `n_samples`
- `chunk_idx`
- `source_size`

This should mirror what Render-MT exposes.


Workflow Changes
----------------

Palette workflow:

- replace `PaletteChunkWorker` target with the new MT-capable handler
- or route through a small Choice that selects single vs MT engine

Preferred:

- one handler that supports both:
  - `threads=1`, `input_mode=tmpfile`
  - `threads>1`, `input_mode=sectioned`

That keeps the workflow simpler.

Shared adoption:

- `stepfunctions/palette_workflow.asl.json.template`
- `stepfunctions/render_workflow.asl.json.template`
  - associated-palette chunk branch should use the same MT-capable palette chunk
    worker, not keep an older slow path


Concurrency Strategy
--------------------

There are two independent concurrency axes:

1. threads inside one chunk Lambda
2. concurrent chunk Lambdas in the `Map`

Both matter.

### Current

- chunk workers: `MaxConcurrency = 10`
- per-chunk palette worker threads: `1`

### Proposed

Expose and use:

- `palette_chunk_threads`
- `palette_chunk_input_mode`
- `palette_chunk_retries`
- `palette_chunk_workers`

Meaning:

- `palette_chunk_threads`
  - native threads inside one Lambda
- `palette_chunk_input_mode`
  - `tmpfile | sectioned`
- `palette_chunk_retries`
  - sectioned retry count
- `palette_chunk_workers`
  - live chunk Lambdas in parallel

### Important caution

Do not blindly copy a large number from compute.

Aggregate pressure matters:

- `live chunk workers * threads per worker`

If we do:

- `workers=50`
- `threads=6`

then sectioned mode means roughly `300` simultaneous range-read streams.
We already learned from solve-score hist that this can trigger transient
`500/503` failures.

So the plan should be:

1. implement the knob
2. start with a safer default
   - e.g. `workers=16` or `32`
3. benchmark `16 / 32 / 50`
4. only promote a larger default if live runs justify it

### Specific recommendation

- full-path hist map:
  - keep current conservative concurrency at first
  - because we already know sectioned hist can become brittle under too much
    aggregate concurrency
- palette chunk map:
  - this is the real place to raise concurrency
  - benchmark and likely move above `10`


Popup / UI Changes
------------------

The current `ExtractPalette` popup should be split into two logical sections.

Section A: Solve-score prepass

- `Hist threads`
- `Hist input`
- `Hist retries`
- `Merge workers`

These apply only when extraction needs `generate_full`.

Section B: Exact palette chunk generation

- `Chunk threads`
- `Chunk input`
- `Chunk retries`
- `Chunk workers`

These apply whenever extraction needs generation:

- `generate_reuse`
- `generate_full`

Attach/no-op paths:

- if the plan resolves to `done` or `attach`
- all generation knobs are ignored
- the popup/help text should say that explicitly

This fixes the current “fake thread knob” problem.


Plan Payload Changes
--------------------

Add to `handler_palette_render_plan.py`:

- `palette_chunk_threads`
- `palette_chunk_input_mode`
- `palette_chunk_retries`
- `palette_chunk_workers`

These should be validated the same way Render-MT validates:

- thread range
- input-mode enum
- retry range
- worker range

They should land in:

- `plan.params`
- workflow item selectors
- worker payload


Handler Changes
---------------

Add:

- `lambda/handler_palette_chunk_mt.py`

Responsibilities:

- validate MT chunk params
- decide `tmpfile` vs `sectioned`
- enforce memory / `/tmp` guardrails
- generate presigned URL for sectioned input
- invoke native MT chunk binary
- upload outputs
- report rich perf and error context

Error context should include:

- `chunk_idx`
- `input_mode`
- `threads`
- `retries`
- `source=s3://...`
- `size=...`
- exact range on sectioned failure when possible


Native Binary Changes
---------------------

Add:

- `lambda/solve_palette_chunk_mt.c`

Behavior:

- same exact palette-chunk math as current `solve_palette_chunk`
- thread over solve ranges
- support `--threads`
- support `--input_mode=tmpfile|sectioned`
- support `--url`
- support `--input_size`
- support `--retries`
- support root transforms

Implementation guidance:

- reuse the same solve-scoring helpers already used by `solve_palette_chunk`
- reuse sectioned-download patterns from:
  - `lambda/solve_proximity_hist_sectioned.c`
  - `lambda/roots2pix_mt.c`
- keep stdout JSON compact and machine-readable

Exactness rule:

- output `scores_out` and `bins_out` must match the current worker exactly
  at `threads=1`
- and should remain exact at `threads>1`


Logging / Observability
-----------------------

Palette observer should report perf with the same level of detail as Render-MT.

Examples:

Hist:

```text
Solve score: hist 86/500 · wall=18.6s total=dl 41.2s + compute 33.7s · threads=4 · input=sectioned · retries=2
```

Palette chunk:

```text
Palette: chunk 317/500 · wall=74.1s total=dl 182.0s + native 95.6s + up 21.3s · threads=6 · input=sectioned · retries=2 · workers=32
```

Finalize:

```text
Palette: finalize · wall=2.7s total=assemble 1.4s + render 0.5s + encode 0.6s · size 5.2MB
```

Dispatch line should also be explicit:

```text
ExtractPalette-MT: dispatching with hist threads=4, hist input=sectioned, hist retries=2, merge workers=16, chunk threads=6, chunk input=sectioned, chunk retries=2, chunk workers=32...
```

The point is:

- user sees whether the expensive phase is using the requested knobs
- user can tell whether the run is on the reuse path or the full path


What Not To Change
------------------

Do not change these in v1:

- palette artifact meaning
- palette metadata contract
- image-based palette inference
- finalize output layout
- repalette expectations

This is a performance/engine upgrade, not a semantic redesign.


Tests Required
--------------

### Native

- exact parity with current `solve_palette_chunk` at `threads=1`
- exact parity at `threads>1`
- solve-aligned section splitting tests
- sectioned retry tests

### Handler

- MT handler validates all new params
- tmpfile path works
- sectioned path works
- rich error context is preserved
- perf fields are written to `result_data`

### Workflow

- palette workflow forwards MT chunk params
- render associated-palette workflow forwards MT chunk params too
- map concurrency setting is pinned

### Frontend

- popup shows both prepass and chunk sections
- dispatch payload contains both prepass and chunk knobs
- progress log prints detailed perf lines
- ignored-control messaging for attach/no-op path

### Deploy / Runtime

- package includes new handler + binary
- docker runtime smoke checks:
  - binary exists
  - `ldd` shows no missing shared libs
  - sectioned mode can run in the Lambda-like container


Rollout Plan
------------

Phase 1

- document exact current cost and fake knobs
- add popup fields for chunk-stage knobs
- add plan/workflow payload fields

Phase 2

- implement `solve_palette_chunk_mt.c`
- implement `handler_palette_chunk_mt.py`
- keep single-thread tmpfile path as compatibility fallback

Phase 3

- switch palette workflow to MT-capable chunk worker
- switch render associated-palette chunk branch to the same worker

Phase 4

- add detailed palette perf reporting
- benchmark:
  - `workers=10/16/32/50`
  - `threads=1/2/4/6`
  - `tmpfile vs sectioned`

Phase 5

- choose defaults from live runs
- update docs/UI help with the measured defaults


Success Criteria
----------------

The change is successful when all of these are true:

1. `ExtractPalette` reuse-path runs visibly faster than today on large
   `500`-chunk jobs.
2. The chunk-stage thread knob actually affects the expensive phase.
3. Sectioned mode is exact and operationally stable with retries.
4. The same MT chunk engine is reused for render-associated-palette generation.
5. Logs make it obvious:
   - which path was taken
   - what knobs were used
   - where the time went


Bottom Line
-----------

Yes: the current `ExtractPalette` knob is partly bullshit today.

It only speeds up the solve-score prepass, and on the common `generate_reuse`
path that prepass is skipped, so the main cost is still:

- single-threaded
- whole-chunk tmpfile
- only `10` live chunk workers

The fix is to make the shared exact `palette_chunk` phase truly MT, sectioned,
retry-aware, and observable, then use that everywhere exact palette generation
happens.
