# Logical Sectioning + Job Size Plan

Status
------

Superseded by [native-multispan-reader-plan.md](/Users/nicknassuphis/karpo_hackathon/polypaint/native-multispan-reader-plan.md).

Note:

- the original recommendation here was a Python tmpfile stitch adaptor as the
  first real rollout target
- that is no longer the desired end state
- the target architecture for logical render sections is now documented in
  [native-multispan-reader-plan.md](/Users/nicknassuphis/karpo_hackathon/polypaint/native-multispan-reader-plan.md)

Current state after the recent Generate-MT work:

- the popup now exposes the associated-palette execution knobs
  - `palette_chunk_threads`
  - `palette_chunk_input_mode`
  - `palette_chunk_retries`
  - `palette_chunk_workers`
- those knobs are persisted in `render_execution` and restored by
  `Render -> Color -> Populate`
- but the actual per-Lambda unit of work for associated-palette generation is
  still hard-wired to the compute chunk layout

That last part is the real bug. The UI now exposes concurrency knobs, but it
still does not expose the actual task size.


Objective
---------

Fix two related problems.

1. `Generate-MT` should show the user what job size they are about to run:
   - compute chunk count
   - representative root / coeff / param chunk sizes
   - warnings when the currently selected execution mode is obviously unsafe
2. Render-side chunk-heavy phases must stop using compute chunking as the
   hidden work-size knob.

The required execution model is:

- concurrency is one knob
- task size is a separate knob
- both are explicit
- neither is silently inferred from the other


Short Answer
------------

Yes, the right abstraction is a logical-section adaptor over the full solve
index space.

The worker contract should become:

- `get me section 31/64`

not:

- `process physical chunk 31`

That adaptor must know how to map one logical solve range onto the underlying:

- root chunk files
- coeff chunk files
- param chunk files

and present a contiguous local input to the worker.


Current Problem
---------------

Today the associated-palette render path still maps directly over
`plan.chunk_items`:

- [handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- [render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)

Specifically:

- `ColorAssociatedPaletteMap` iterates `$.plan.chunk_items`
- each item forwards one physical:
  - `bin_key`
  - `coeffs_key`
  - `params_key`
  - `bin_size`
  - `step_start`
  - `step_count`

And the worker consumes one full physical chunk:

- [handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)

So:

- `palette_chunk_workers` changes only `MaxConcurrency`
- `palette_chunk_threads` only splits that one chunk inside the Lambda
- neither changes the per-Lambda input size

That is why this failed:

- `sectioned palette chunk input too large for current Lambda memory:
  952000000 bytes > safe limit 927465472 bytes`

The bad coupling is:

- compute chunking was chosen for solve / coeffgen throughput
- then palette extraction inherited that chunking as its work-unit size

Those are different concerns and should not be linked.


Core Rule
---------

No hidden task sizing.

If a render phase can fail because its work unit is too large, the user must be
able to see and control:

- how large the physical compute chunks are
- how large the logical render/palette sections are
- how many workers are processing those sections in parallel

`workers` is not a task-size knob and must never be presented as one.


User-Facing Goals
-----------------

`Generate-MT` should expose a `Job Size` section that reports:

- compute chunk count
- representative root chunk size
- representative coeff chunk size
- representative param chunk size
- lores sizes when available
- current pixel-bin fragment mode and group size
- current palette chunk input mode
- current hist input mode
- warnings for sectioned paths that exceed a safe size threshold

Important UI rule:

- use the first full-size physical chunk for the representative size
- do not use a tail chunk if it is smaller
- if there is no obvious full-size chunk, use the largest chunk and label it as
  such

The popup should also expose explicit section-count controls for chunk-heavy
render-side phases:

- solve-score hist partitioning mode
- solve-score hist sections
- solve-score hist auto sizing
- associated-palette partitioning mode
- associated-palette sections
- associated-palette auto sizing

Those values must be persisted in `render_execution` and restored by
`Populate`, just like the other Generate-MT controls.

Recommended visible modes:

- `physical_chunks`
- `logical_sections`
- `logical_sections_auto`

For rollout safety, the initial default can stay `physical_chunks`, but it must
be shown explicitly in the popup, logs, and persisted execution settings.

`logical_sections_auto` means:

- the user selects the execution mode
- the planner computes a safe section count automatically from:
  - Lambda memory size
  - required data sources
  - thread count
  - total solve count
  - conservative overhead assumptions

The user should still be able to override it manually with
`logical_sections`.


Scope
-----

This plan is for render-side heavy consumers of full solve-aligned data.

Immediate target:

- `Render -> Color -> Generate-MT`
  - associated palette path first

Second target:

- solve-score histogram / render prepass

Possible later adopters:

- standalone `ExtractPalette`
- standalone Palette generation

This is not a compute chunking redesign.


Non-Goals
---------

Not in v1:

- changing compute chunk sizes at render time
- rebuilding the compute pipeline around logical sections
- changing solve-score semantics
- changing chip syntax for solve / coeff / param metrics
- deleting legacy chunk-based code before the logical path is proven


Current Code Boundaries
-----------------------

Relevant current control points:

- popup + persisted MT execution settings
  - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- render plan normalization and associated-palette map inputs
  - [handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- associated-palette chunk worker
  - [handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
- render workflow map over `plan.chunk_items`
  - [render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)
- param chunk-source adaptor
  - [param_source.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/param_source.py)

That existing param adaptor is useful precedent:

- user-visible syntax stayed stable
- plan/runtime absorbed the storage-layout change

Logical sectioning should follow the same principle.


Recommended Model
-----------------

Introduce a logical-section manifest over the global solve index space.

Canonical section identity:

- `section_idx`
- `section_count`

Canonical solve range:

- `solve_start`
- `solve_count`

Canonical source spans:

- `root_spans`
- `coeff_spans`
- `param_spans`

Each span record should identify:

- source key
- source byte start
- source byte length
- local solve start
- local solve count

The render worker should not need to know or care whether the section:

- fits inside one physical chunk
- straddles two chunk files
- needs coeff spans but not param spans
- needs param spans but not coeff spans

That is the adaptor’s job.


Auto-Sizing Model
-----------------

Yes, section count can be derived automatically.

But the correct function is not just:

- `section_count = total_size / max_section_size`

because the memory budget is not only the raw input bytes.

The planner needs a conservative memory model:

- fixed per-invocation overhead
- native process overhead
- per-thread scratch / read buffers
- required source bytes for the logical section
- safety margin

For a given phase, the planner should estimate:

- `available_bytes = lambda_memory_bytes * usable_fraction`
- `thread_overhead_bytes = threads * per_thread_budget`
- `source_budget_bytes = available_bytes - fixed_overhead_bytes - thread_overhead_bytes`

Then compute the maximum safe solve count per section from the required row
widths:

- roots row bytes
- coeff row bytes when `cf` is used
- param row bytes when `pm` is used

Conceptually:

- `section_row_bytes = roots_row_bytes + coeff_row_bytes? + params_row_bytes?`
- `max_solves_per_section = floor(source_budget_bytes / section_row_bytes)`
- `section_count = ceil(total_solves / max_solves_per_section)`

Important detail:

- for UI display, `representative chunk size * chunk count` is a fine hard
  upper ceiling estimate
- for actual auto sizing, use solve counts and row widths when available
- if only chunk-size metadata exists, fall back to the ceiling estimate and add
  extra margin

This should never target the true limit. It should target a safe budget below
it.

Recommended planner constants:

- `usable_fraction < 0.5` for current sectioned-native paths
- explicit per-thread overhead budget
- explicit fixed native overhead budget
- minimum section count of `1`

The point of auto mode is not perfect packing. The point is:

- no OOM
- no sectioned-size hard-limit surprise
- still allow manual override for deliberate tuning


Phase-Specific Auto Sizing
--------------------------

Auto sizing must be phase-aware.

Associated palette:

- source requirements come from the solve-score program
  - `slv` only
  - `slv + cf`
  - `slv + pm`
  - `slv + cf + pm`
- threads come from `palette_chunk_threads`
- Lambda memory comes from the palette chunk function configuration

Solve-score hist:

- same source-aware sizing
- threads come from `solve_score_threads`
- Lambda memory comes from the solve histogram worker configuration

This is why auto sizing belongs in the plan layer, not in the popup alone.


Why This Is Better Than Chunk-Count Heuristics
----------------------------------------------

The user intuition is right:

- total data size is approximately `chunk_size * chunk_count`

and that is a valid hard ceiling for display.

But chunk-count heuristics are not enough for actual safe execution because:

- some jobs have stub/tail chunks
- mixed-source programs pull more than one row store
- thread count changes the memory budget
- different phases may use different Lambda memory sizes

So:

- use chunk sizes for diagnostics
- use solve-count + row-width math for the actual auto-sizing decision


Recommended UI Contract
-----------------------

For each sectioned heavy phase, expose:

- partitioning mode
  - `physical_chunks`
  - `logical_sections`
  - `logical_sections_auto`
- section count
  - editable only in manual logical mode
- auto-sizing summary
  - estimated section count
  - estimated max solves per section
  - estimated source bytes per section
  - Lambda memory and safety budget used

If auto mode is selected, the UI should still show the computed result, not
hide it.

The safety bound must be shown right next to the manual choice.

If the UI exposes `section count`:

- show `min safe sections = N`
- do not allow the user to pick a value below that minimum
- clamp or reject invalid lower values immediately in the control

If the UI ever exposes `section size` instead:

- show `max safe solves/section = M`
- do not allow the user to pick a value above that maximum

This direction matters:

- higher `section_count` means smaller tasks and is safer
- lower `section_count` means larger tasks and is riskier

So for the current plan, where the primary control is `section count`, the UI
constraint is:

- `section_count >= min_safe_sections`

not:

- `section_count <= some_max_for_safety`

The user should also be able to select the computed safe/manual value directly,
for example:

- `Use minimum safe`
- `Use auto`

without having to retype it.

Example summary lines:

- `Associated palette: auto -> 64 sections (~390625 solves/section, budget 640 MB)`
- `Solve hist: auto -> 48 sections (~520833 solves/section, budget 768 MB)`

If metadata is insufficient for a reliable estimate:

- show `auto unavailable`
- explain why
- require explicit manual mode


Why Solve-Aligned Sections
--------------------------

The section partition must be defined in solve units, not raw bytes.

Reason:

- roots, coeffs, and params are all solve-aligned row stores
- row widths differ across sources
- byte partitioning would desynchronize the sources

So the correct order is:

1. partition the global solve index range
2. map that solve range onto each required physical source
3. materialize contiguous local files for the worker


V1 Adaptor Strategy
-------------------

Use a Python-side stitching adaptor first.

Recommended new shared helper:

- `lambda/logical_sections.py`

Responsibilities:

1. build section manifests from `chunk_items`
2. map one logical solve range to one or more physical source spans
3. download or range-read those spans
4. stitch them into contiguous local temp files for native consumers

This is intentionally a plan/runtime-layer solution first.

Do not start with a native “multi-span remote reader” redesign unless the
Python-side adaptor proves too slow. The first goal is to decouple task size
from compute chunk size safely.


Phase 0: Job Size Reporting
---------------------------

Add a `Job Size` section to `Generate-MT`.

Display:

- `compute chunks = N`
- `root chunk ≈ X MB`
- `coeff chunk ≈ Y MB`
- `param chunk ≈ Z MB`
- `lores root = ...`
- `lores coeff = ...`
- `lores params = ...`
- `hist input = tmpfile | sectioned`
- `palette input = tmpfile | sectioned`
- `palette sections = ...`
- `hist sections = ...`

Warnings:

- if `palette_chunk_input_mode = sectioned` and representative root chunk size
  exceeds the palette Lambda’s sectioned safe limit, show a warning
- likewise for solve-score hist
- if section counts are still using physical chunks, say so explicitly

Implementation notes:

- derive sizes from `calc.json` / `chunk_items`
- prefer the first full-size chunk over a tail chunk
- if size metadata is missing, report `unknown` rather than inventing certainty

Persisted state:

- add section-count fields to `render_execution`
- restore them in `Populate`


Phase 1: Associated-Palette Logical Sections
--------------------------------------------

This is the first runtime phase to decouple.

New plan fields:

- `associated_palette.section_mode`
- `associated_palette.section_count`
- `associated_palette.section_count_auto`
- `associated_palette.section_budget_bytes`
- `associated_palette.section_items`

`section_items` replaces `chunk_items` for the associated-palette map.

Workflow change:

- `ColorAssociatedPaletteMap.ItemsPath`
  - from `$.plan.chunk_items`
  - to `$.plan.associated_palette.section_items`

Each section item should carry:

- `section_idx`
- `section_count`
- `solve_start`
- `solve_count`
- `root_spans`
- `coeff_spans`
- `param_spans`
- enough metadata to preserve current metric / score-program behavior

Worker change:

- `handler_palette_chunk.py` should accept a logical section item
- materialize local stitched inputs for the sources actually needed
- invoke the existing native palette chunk binary on the stitched local files

Important compatibility rule:

- if `section_mode = physical_chunks`, the plan must preserve current behavior
  exactly
- if `section_mode = logical_sections`, `section_count` defines the actual work
  partition

This gives a real A/B path.


Phase 2: Solve-Score Hist Uses the Same Layer
---------------------------------------------

The same coupling exists in the solve-score hist map.

Once the section adaptor works for associated palette, use the same model for:

- solve-score histogram workers

New plan fields:

- `solve_score.section_mode`
- `solve_score.section_count`
- `solve_score.section_count_auto`
- `solve_score.section_budget_bytes`
- `solve_score.section_items`

This phase matters because hist can hit the same memory ceiling for large root
chunks even when palette extraction is disabled.

Using one shared logical-section builder prevents the two maps from drifting.


Phase 3: Standalone Palette / ExtractPalette
--------------------------------------------

After render uses the section layer successfully, move the same design into:

- palette workflow
- `ExtractPalette`

The exact palette-chunk engine is shared, so it should not end up with one
sectioning model in render and a different one in palette extraction.


Section Count vs Workers
------------------------

These knobs must remain separate.

`section_count`:

- controls per-Lambda work size
- more sections means smaller tasks
- fewer sections means larger tasks

`workers`:

- controls how many section tasks run concurrently
- affects throughput and pressure on S3 / Lambda concurrency
- does not change task size

This distinction must be visible in both:

- UI text
- logs


Recommended UI Table Additions
------------------------------

In the current Generate-MT table, add explicit rows for:

- `Solve hist partitioning`
- `Solve hist sections`
- `Solve hist auto summary`
- `Associated palette partitioning`
- `Associated palette sections`
- `Associated palette auto summary`

Keep the existing rows for:

- hist threads
- hist input mode
- hist retries
- merge workers
- raster input mode
- raster retries
- raster threads
- bin fragment mode / group size
- finalize workers
- palette chunk threads
- palette chunk input mode
- palette chunk retries
- palette chunk workers

No hidden defaults.

Defaults are fine, but they must be visible and editable.


Recommended Logging
-------------------

Dispatch log lines should include:

- physical chunk count
- logical section counts
- representative section size estimate
- whether section count was:
  - manual
  - or auto-derived
- the budget basis:
  - Lambda memory
  - threads
  - usable budget
- whether the current run is:
  - `physical_chunks`
  - or `logical_sections`

Per-phase progress logs should include:

- `sections_done / total_sections`
- wall time
- total download time
- total compute time
- workers
- threads
- input mode

For associated palette specifically:

- log both physical chunk count and logical section count
- the point is to make it obvious when one is no longer equal to the other


Data-Source Rules
-----------------

Logical sections must support whichever solve-score program sources are used:

- `slv`
- `cf`
- `pm`

This means:

- roots-only programs should not pay to fetch coeff or param spans
- coeff-source programs should fetch stitched coeff spans
- param-source programs should fetch stitched param spans
- mixed-source programs should fetch exactly the required combination

The existing source-aware solve-score compiler does not need new syntax for
this. The runtime layer already knows which sources are needed.


Exactness and Semantics
-----------------------

Logical sectioning must not change score semantics.

It only changes:

- how physical input files are fetched
- how work is partitioned across Lambdas

It must not change:

- solve-score clip bounds
- score normalization
- palette bin assignment
- associated palette image semantics

The clip / merge / finalize math must remain exactly the same as today.


Persistence
-----------

Persist these new Generate-MT controls in `render_execution`:

- `solve_score_section_mode`
- `solve_score_section_count`
- `solve_score_section_count_auto`
- `palette_section_mode`
- `palette_section_count`
- `palette_section_count_auto`

`Populate` should restore them exactly, the same way it now restores:

- hist threads
- raster threads
- finalize workers
- palette chunk threads
- palette chunk input mode
- palette chunk retries
- palette chunk workers


Testing Plan
------------

Frontend:

- popup summary shows the new `Job Size` section
- representative chunk sizes render correctly
- section-count controls persist in popup state
- auto-sizing summaries render correctly
- manual section-count controls show the safe bound inline
- manual section-count controls reject unsafe values in the correct direction
- `Populate` restores them from artifact `render_execution`

Plan-layer:

- render plan builds stable section manifests from `chunk_items`
- physical-chunk compatibility mode still works
- auto sizing computes stable section counts from memory budget inputs
- mixed-source section manifests include only required spans

Workflow:

- render workflow maps over `section_items` for associated palette
- later, solve-score hist workflow maps over `section_items`

Worker:

- palette chunk worker can stitch one logical section from:
  - one physical chunk
  - multiple adjacent chunks
- sectioned and tmpfile modes both work through the adaptor
- oversized physical chunks no longer force oversized palette tasks when
  section count is increased

Safety:

- auto mode never emits `0` sections
- auto mode clamps to at least one solve per section
- auto mode falls back cleanly when required sizing metadata is missing
- auto mode keeps a conservative budget margin so a config drift does not turn
  into an OOM

Regression:

- same job, same score program, same associated palette result for:
  - legacy physical chunk mode
  - logical section mode with equivalent coverage


Checklist
---------

- [ ] Add `Job Size` panel to `Generate-MT`
- [ ] Surface representative root / coeff / param chunk sizes
- [ ] Surface sectioned safety warnings in the popup
- [ ] Add explicit `Solve hist sections` control
- [ ] Add explicit `Solve hist auto mode`
- [ ] Add explicit `Associated palette sections` control
- [ ] Add explicit `Associated palette auto mode`
- [ ] Show the safe bound inline next to each manual section-count control
- [ ] Prevent manual section counts below the computed safe minimum
- [ ] Persist new section-count controls in `render_execution`
- [ ] Restore them in `Populate`
- [ ] Add conservative auto-sizing budget model
- [ ] Make auto sizing phase-aware and source-aware
- [ ] Add shared logical-section manifest builder
- [ ] Add shared source-span stitching helper
- [ ] Change associated-palette render map to iterate section items
- [ ] Update palette chunk worker to consume logical sections
- [ ] Keep a compatibility path for physical chunk mode
- [ ] Reuse the same logical-section layer for solve-score hist
- [ ] Extend standalone Palette / ExtractPalette to the same model


Recommended Rollout Order
-------------------------

1. Phase 0: expose job size and section-count controls in the UI
2. Phase 1: implement logical sections for associated palette only
3. Phase 2: move solve-score hist to the same section layer
4. Phase 3: converge standalone Palette / ExtractPalette onto the same model
5. Only then consider deeper native multi-span read optimizations

That order gets the dangerous hidden coupling out first, while keeping the
native binaries and scoring semantics stable during the transition.
