# Lagged Scores Plan

Status: revised feature plan. One architectural choice remains open: whether
the final image is produced by a second full evaluator pass, or by composing
from cached raw slot scores after native stats merge.

This revision assumes the real target use case is composed programs such as:

```text
abs_diff(proximity(slv), proximity(slv-1))
```

Because that output depends on full-resolution adjacency, the production fused
solve-score render path stops using lores-based clip calibration.

## Goal

Let a solve-score program reference the previous solve's data as a new source,
so the chain DSL can compare the current solve to the immediately previous solve
in serpentine index order.

New source tags:

- `slv-1` — previous solve's roots
- `cf-1` — previous solve's coefficients
- `pm-1` — previous solve's params

Lag magnitude is fixed at 1 for v1. No arbitrary lag-N.

### Primary motivating programs

- `abs_diff(proximity(slv), proximity(slv-1))`
- `avg(proximity(slv), proximity(slv-1))`
- `weighted_sum(0.8, spread(slv), 0.2, spread(slv-1))`

These are the programs the implementation must optimize for. The feature is not
being added merely to render an isolated `proximity(slv-1)` slot.

### Semantic definition (pinned)

`proximity(q=0.5, slv-1)` means: evaluate the same single-solve proximity metric
against the previous solve's roots. The lag source is an index shift, not a new
pairwise metric. At pixel `i`, the metric sees solve `i - 1`'s data.

The same applies to `cf-1` and `pm-1`: they mean "use the previous solve's
coeffs/params as the metric input for this slot."

In serpentine order, that makes most lag-1 references physically local within a
row. Row flips and pass boundaries remain index-based rather than spatially
adjacent. That discontinuity is accepted for v1.

### Grammar (pinned)

Accepted lag tokens are exactly the three lowercase literals matched by
`^(slv|cf|pm)-1$`:

- `slv-1`
- `cf-1`
- `pm-1`

Rejected with a clear error:

- whitespace variants: `slv -1`, ` slv-1`
- case variants: `SLV-1`, `Slv-1`
- other lag values: `slv-2`, `slv-01`, `slv-10`
- malformed suffixes: `slv-`, `slv1`, `slv-1x`

The same grammar applies to:

- the Python chain compiler
- the JS chain compiler
- the C `--score_sources=` CSV parser
- stored solve-score program JSON
- clip-artifact `metrics[].source`

## Pinned Decisions

1. Production fused solve-score render no longer uses lores clip calibration.
2. There is no planner-side aliasing or dedupe between `slv` and `slv-1`.
3. Production fused solve-score render uses full-resolution statistics computed
   from the actual render solve stream. Lores is not used for clip
   calibration.
4. Normalization semantics stay as they are today: each metric slot is clipped
   and normalized first, then the RPN stack machine combines those normalized
   slot values.
5. Lag remains fused-color-only in v1. Other solve-score consumers reject lag
   early and loudly.
6. V1 does not add output-level normalization. Correlated lag-pair programs such
   as `abs_diff(proximity(slv), proximity(slv-1))` therefore render as mostly
   dark edge-detect-style images by design under the existing
   slot-normalize-then-compose semantics.
7. `step_scores.raw` is written only by the stage that produces the final image.
   The stats pass never writes reusable step-score output.
8. Digest parsing, digest merge, and quantile extraction stay in C. Python only
   orchestrates section artifact download, binary invocation, and final upload.
9. V1 keeps t-digest-style summaries rather than switching to a top-K extreme
    sample scheme. Top-K remains an explicit alternative, not the chosen format.
10. Two production architectures remain under consideration:
    Option A reruns the full evaluator after stats merge; Option B writes raw
    slot-score cache fragments in pass 1 and performs a cheap compose-from-cache
    pass after stats merge.

## Why Lores Is Out

Lores is not merely "a bit approximate" for the target feature. It is the wrong
object.

For an isolated slot like `proximity(slv-1)`, one can argue about whether the
lagged marginal distribution looks similar to the non-lagged one. That argument
is irrelevant once the operator writes the program they actually want:

```text
abs_diff(proximity(slv), proximity(slv-1))
```

That output depends on the joint distribution of adjacent solves. At lores
density, adjacent solves are much farther apart in parameter space than they are
in the final render. The difference image is therefore calibrated against the
wrong adjacency scale.

So the production fused solve-score render path gives up on lores completely:

- no `lores.bin`
- no `lores_coeffs.bin`
- no `lores_params.bin`
- no `solve_proximity_stats --mode=clip` in the render workflow
- no render-plan dependence on lores scratch to support mixed-source chains

This decision applies to the production fused solve-score render path. Unrelated
preview tooling is a separate feature and out of scope here.

## Current State

### Existing production path

Today fused solve-score render has two stages:

1. A lores clip prepass driven by [handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
2. A full-resolution raster driven by [handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py) and [roots2pix_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix_mt.c)

That split is what this plan replaces.

### Source tags live in `--score_sources`, not in `program_spec`

Lag tags belong in:

- [solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h) CSV parsing
- [solve_score_chain.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score_chain.py) compilation
- JS chain parsing in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

`program_spec` stays as the RPN opcode string.

### Shared evaluator hazard

The score-program evaluator in [solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h) is shared by:

- fused raster
- palette chunk binaries
- any other direct program-eval caller

If a lag slot silently falls back to current data when no prev pointer is
supplied, non-fused consumers will produce wrong output without noticing. The
evaluator must therefore hard-fail when a lagged source is referenced but the
caller did not supply the corresponding prev pointer.

### Full-res raster topology still matters

Inside [roots2pix_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix_mt.c), each native thread downloads its own section byte range through the multispan manifest. Threads do not share a section buffer. Lag therefore has to be supplied by:

- a logical-section prelude row in the manifest
- a per-thread one-row overlap inside the already-grown logical section

### Root transforms still apply before scoring

`slv-1` must refer to the previous solve after the same `prepare_step(...)`
transform pipeline as the current solve. The previous roots are not fed raw into
the evaluator.

## New Production Pipeline

The fused solve-score render path gains a shared full-resolution stats core,
then branches into one of two candidate final-image architectures.

### Stage 1: full-resolution score-stats map

Each logical raster section is processed once in `score_stats` mode.

The section worker:

- reads the full-resolution solve stream for that section
- applies the same root transforms as the real raster path
- evaluates every metric slot against the correct current or lagged source
- excludes warmup/prelude rows from the stats
- emits a compact per-slot quantile sketch artifact for that section
- optionally, under Option B, emits a raw slot-score cache fragment for that
  section

This pass computes no final pixels.
This pass also does not write `step_scores.raw`.

### Stage 2: score-stats merge

A single merge task reads every section stats artifact, invokes a native merge
path, and writes the standard solve-score clip artifact used by the render path.

The clip artifact remains slot-indexed. Slots are not deduped merely because
they happen to share the same metric name with different source tags.

### Option A: reraster after stats merge

This is the simpler implementation.

#### Stage 3A: final raster map

The existing full-resolution raster pass runs again, this time with the merged
clip ranges from stage 2.

Each slot is clipped and normalized using full-resolution statistics gathered
from the actual solve stream of the render. Then the existing RPN program runs.

#### Stage 4A: finalize

Finalize remains unchanged. It consumes the sectioned raw output and associated
step-scores exactly as today.

### Option B: cache raw slot scores, then compose from cache

This is the more efficient implementation.

#### Stage 3B: compose-from-cache map

Instead of re-reading roots and re-running the evaluator, each section reads the
raw slot-score cache fragment written in stage 1.

That section worker:

- reads cached raw slot scores for each solve and slot
- applies `clip_lo` / `clip_hi` per slot using the merged clip artifact
- runs the existing RPN stack machine
- writes the final grayscale output and final `step_scores.raw` fragment

No solve stream, no root transforms, and no lag resolution are repeated in this
stage.

#### Stage 4B: finalize

Finalize remains unchanged. It consumes the sectioned raw output and associated
step-scores exactly as today.

### `step_scores.raw` lifecycle (pinned across both options)

`step_scores.raw` remains a final-image-stage product.

Reason:

- downstream consumers such as repalette / recolor / extract-palette expect the
  final rendered score field, not pre-normalization slot data
- stage 1 exists to gather full-resolution slot statistics, and under Option B
  may also emit raw slot-score cache fragments
- the stage that actually produces the final image writes the same reusable
  `step_scores.raw` family that downstream
  tooling already knows how to consume

## Stats Contract

### Slot-level normalization remains the semantic contract

This plan does not change the meaning of solve-score programs.

Current semantics are:

1. compute a raw score per metric slot
2. clip and normalize each slot independently
3. run the RPN stack machine on those normalized slot values

Because that contract stays in place, the new stats pass only needs exact or
near-exact full-resolution slot distributions. It does not need a sketch of the
final composed program output.

If a future feature adds `normalize(expr, q)` or output-level normalization,
that is a separate design and not part of this plan.

### Consequence for correlated lag pairs (pinned)

The target expression

```text
abs_diff(proximity(slv), proximity(slv-1))
```

still renders as a mostly-dark image under this contract.

Why:

- both slots are individually normalized correctly from full-resolution stats
- adjacent solves at full render density are highly correlated
- the composed value `|clip(p_i) - clip(p_{i-1})|` is therefore near zero for
  most pixels and spikes mainly at discontinuities

This is not a calibration bug. It is the expected result of preserving the
existing slot-normalize-then-compose semantics.

V1 explicitly accepts this outcome. Operators who want a brighter visualization
of that signal can use downstream post-processing such as autolevels. A future
feature may add output-level normalization, but this plan does not.

### Section stats artifact format (pinned)

Each section writes one JSON artifact under a dedicated stats prefix:

```json
{
  "family": "solve_score_stats",
  "version": 1,
  "chain_fingerprint": "sha256:...",
  "section_idx": 7,
  "solve_start": 65536,
  "solve_count": 16384,
  "metrics": [
    {
      "slot": 0,
      "metric": "proximity",
      "source": "slv",
      "quantile": 0.001,
      "count": 16384,
      "min_score": 0.0021,
      "max_score": 1.934,
      "digest_format": "tdigest-f64le-v1",
      "digest_b64": "AAAB..."
    },
    {
      "slot": 1,
      "metric": "proximity",
      "source": "slv-1",
      "quantile": 0.001,
      "count": 16384,
      "min_score": 0.0017,
      "max_score": 1.887,
      "digest_format": "tdigest-f64le-v1",
      "digest_b64": "AAAB..."
    }
  ]
}
```

Pinned properties:

- summaries are per slot, not deduped by metric name
- summaries cover scored solves only; warmup/prelude rows are excluded
- `chain_fingerprint` is copied through so stale summaries cannot merge into a
  different program
- `source` is preserved exactly, including lag suffix

`tdigest` is chosen because the merge is associative enough for a multi-section
workflow and keeps artifacts small. The on-wire representation is a base64
encoding of a binary float64 digest payload, not raw JSON float centroids. This
avoids throwing away precision for very tight distributions while keeping the
artifact portable. A section summary should stay comfortably small even with
many slots.

Merge of this format happens only in native code. Python never reconstructs or
re-compresses the digest.

### Raw slot cache artifact format (Option B only)

If Option B is chosen, stage 1 also writes one raw slot-score fragment per
section.

Pinned shape:

- binary float32
- solve-major
- fixed slot count per solve
- scored solves only; warmup/prelude rows excluded

Conceptually:

```text
slot_scores_f32[solve_idx][slot_idx]
```

This artifact is not a user-facing contract. It is an internal render scratch
format used only to avoid a second full evaluator pass.

## Two-Level Lag Availability

Lagged slots need valid previous-solve data in both passes:

- the full-resolution score-stats pass
- the final full-resolution raster pass

The same overlap model is used in both.

### Logical-section level

When a source family uses lag and `solve_start > 0`, the corresponding manifest
gets one extra leading logical row prepended. The prelude row is the upstream
solve at `solve_start - 1`.

The global-first section clamps prelude to zero because no upstream solve exists.

### Native-thread level

Each native thread reads one row earlier than its first scored solve within that
logical section. No cross-thread reads are introduced.

### Per-thread formula

Let:

- `S` = section global `solve_start`
- `P` = section `prelude_rows` for that source family, either 0 or 1
- `thread_start` = global first scored solve handled by this native thread
- `thread_width` = number of scored solves handled by this thread
- `row_bytes` = source-family stride in bytes for one solve row

Then:

```text
prev_logical_offset = (thread_start - 1 - S + P) * row_bytes
```

Cases:

| Section | Thread | Behavior |
|---|---|---|
| first section, first thread | global solve 0 | sentinel: `prev = current` |
| first section, later threads | same logical section | previous row already in thread range |
| later section, any thread | non-first section | previous row comes from manifest prelude or earlier row in section |

The invariant is simple: at evaluator call time, every lagged slot sees a valid
prev pointer. It is either:

- the true previous solve
- or, only for global solve 0, the sentinel `prev = current`

### Pass and row-flip boundaries

Lag is index-based, not spatial-boundary-aware. At serpentine row flips and pass
boundaries, the previous solve may be physically distant. That remains accepted
behavior for v1.

## Cost And UX Impact

### Option A cost

Option A is not free. Stage 1 repeats most of the expensive work of stage 3A:

- manifest reads
- per-solve decode
- root transforms
- solve-score evaluation

It skips pixel writes, palette matching, and final raw assembly, so the total
wall time is expected to be roughly 1.8x the current production fused
solve-score render path rather than a full 2x.

That slowdown is an explicit tradeoff for correct full-resolution slot
statistics.

### Option B cost

Option B still has two stages after planning, but only one expensive evaluator
pass.

Expected cost shape:

- one full solve/evaluator pass in stage 1
- one cheap native merge
- one cheaper compose-from-cache pass that reads raw slot scores, applies clip,
  and runs the stack machine

This should be materially cheaper than Option A because it avoids repeating:

- solve-stream reads
- root transforms
- lag prev resolution
- metric evaluation

It does, however, introduce a new internal raw slot-score scratch artifact and
the code to compose from it.

### Preview consequence for both options

Operators will feel this cost immediately. Production fused solve-score render
gets slower the moment this lands.

This plan does not solve preview latency. Interactive preview needs a separate
follow-up track if the product wants fast experimentation after lores is removed
from the production path.

### Alternative not chosen for v1

An alternative architecture would evaluate the solve stream only once, writing
to scratch while simultaneously maintaining quantile sketches, then normalize in
a cheap second read. That would cut the cost to roughly one evaluator pass plus
a lightweight normalization pass.

That is effectively what Option B does, except with explicit raw slot-score
cache fragments rather than trying to normalize a fully composed final image in
place. The unresolved choice is whether that extra scratch artifact is worth the
complexity relative to Option A's simpler rerastering design.

Another alternative is per-slot top-K / bottom-K extreme samples instead of
t-digest summaries. That would simplify merge and make the artifact plain JSON,
but it hard-codes the summary toward extreme-edge quantiles and gives up the
more general quantile behavior the current solve-score contract already implies.
V1 keeps the digest approach.

## Contract Changes

### Chain DSL

Source grammar extends with:

- `slv-1`
- `cf-1`
- `pm-1`

Examples:

```text
proximity(q=0.1%, slv-1)
avg(proximity(q=0.1%, slv), proximity(q=0.1%, slv-1))
abs_diff(spread(q=0.5%, cf), spread(q=0.5%, cf-1))
```

### Source CSV

`--score_sources=<csv>` accepts the six-source vocabulary:

- `slv`
- `slv-1`
- `cf`
- `cf-1`
- `pm`
- `pm-1`

`program_spec` stays unchanged.

### Evaluator signature

[solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)
gains prev-pointer inputs:

```c
static double solve_score_eval_program_with_sources(
    const float *roots, int degree,
    const float *coeffRoots, int coeffDegree,
    const float *paramValues, int paramDegree,
    const float *prevRoots, int prevDegree,
    const float *prevCoeffRoots, int prevCoeffDegree,
    const float *prevParamValues, int prevParamDegree,
    const SolveScoreProgram *program);
```

Lagged slots route to the corresponding prev pointers.

### Hard-fail behavior (pinned)

If a lagged source is referenced and the relevant prev pointer is `NULL`, the
evaluator writes a clear stderr message and aborts. No silent fallback to the
current source is allowed.

This is the safety rail that protects palette chunk binaries and any other
non-fused callers.

### Native argv

The raster binary gains:

- `--score_stats_output=<path>` — emit section stats artifact and do not render pixels
- `--prelude_rows=<N>`
- `--score_coeff_prelude_rows=<N>`
- `--score_param_prelude_rows=<N>`

`N` is 0 or 1 in v1.

There is no render-workflow `solve_proximity_stats --mode=clip` contract anymore.

The stats merge is handled by extending `solve_proximity_stats` with a native
merge mode rather than implementing digest merge in Python:

- `solve_proximity_stats --mode=stats_merge --sections_manifest=<path> --out=<path>`

That mode:

- parses section stats artifacts
- merges per-slot digests
- extracts the requested quantiles
- emits the standard clip artifact JSON

### Manifest logical layout

[logical_sections.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/logical_sections.py)
gains per-family `prelude_rows`. When nonzero:

- `logical_size` grows by one row
- downstream spans shift right by one row
- a prepended span covers the upstream row

The multispan reader itself does not change.

### Plan

`plan.solve_score` gains:

- `stats_prefix`
- `clip_key`

`plan.raster` keeps or gains:

- `prelude_rows`
- `score_coeff_prelude_rows`
- `score_param_prelude_rows`

Planner behavior changes:

- fused solve-score render no longer depends on lores scratch for clipping
- mixed-source fused solve-score render no longer requires `lores.coeffs_key`
  or `lores.params_key`
- lag usage is derived directly from the compiled chain

### ASL

The render workflow replaces the lores clip task with:

- `ColorSolveScoreStatsMap`
- `ColorSolveScoreStatsMergeTask`

`ColorRasterMap` remains the final pixel-producing pass.

The lag chain itself still threads through `$.plan.solve_score.chain`. The new
states additionally thread the per-family prelude fields and stats prefix.

### Frontend

Required:

- JS chain compiler accepts lag tokens
- JS serializer round-trips lag tokens
- populate and solve-score modal Load restore lagged programs correctly

Required cleanup:

- render-dispatch controls that exist only for the old lores clip stage are
  removed from the production render UI and request payload

Optional:

- chip picker grows explicit lag entries

## Implementation Plan By File

### 1. `lambda/solve_score_chain.py`

- extend allowed sources with `slv-1`, `cf-1`, `pm-1`
- preserve lag suffix in compiled `metrics[i].source`
- add helpers such as `any_lag_source(compiled)` and per-family lag usage
- keep `program_spec` unchanged
- ensure fingerprints differ when only the source tag differs

### 2. `lambda/solve_score.h`

- add `*_LAG1` source enum variants
- extend CSV parsing
- extend `solve_metric_supports_source`
- extend evaluator signature with prev pointers
- abort on lagged-source access with NULL prev pointer

### 3. `lambda/logical_sections.py`

- add per-family `prelude_rows`
- prepend one logical warmup row when needed
- clamp to zero for the global-first section

### 4. `lambda/roots2pix_mt.c`

- add `--score_stats_output`
- in stats mode, emit per-slot t-digest summaries instead of pixels
- under Option B, also emit raw slot-score cache fragments
- in raster mode, keep current output behavior
- implement lag prev routing in both modes
- compute transformed previous step via the same `prepare_step(...)` pipeline
- validate prelude flags when lagged sources are present

### 5. `lambda/handler_raster_mt.py`

- add a `score_stats` phase alongside the current render phase
- build the same manifests as the render pass, including preludes
- invoke `roots2pix_mt` in stats mode
- upload section stats artifacts under `plan.solve_score.stats_prefix`
- under Option B, upload raw slot-score cache fragments too

### 6. `lambda/solve_proximity_stats.c` and `lambda/handler_solve_proximity.py`

- remove render-workflow dependence on `solve_proximity_stats --mode=clip`
- keep histogram / debug functionality separate
- add `solve_proximity_stats --mode=stats_merge`
- in that native mode, parse section stats artifacts, merge per-slot digests,
  extract quantiles, and emit the standard clip artifact JSON
- keep [handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
  as thin orchestration only: download section artifacts, invoke the binary,
  read JSON output, upload clip artifact

If the Python file name remains unchanged, add a file-head comment stating that
its render-workflow role is now orchestration around native full-resolution
stats merge rather than lores clip computation.

### 7. `lambda/handler_render_plan.py`

- stop requiring lores scratch for fused solve-score clipping
- derive per-family prelude flags from the compiled chain
- emit `stats_prefix`
- keep `clip_key` as the downstream clip artifact contract

### 8. `lambda/workflow_contracts.py` and `stepfunctions/render_workflow.asl.json.template`

- replace `ColorSolveScoreClipTask` with stats-map + stats-merge
- thread `stats_prefix`, `clip_key`, and prelude fields
- branch after stats merge:
  - Option A: final raster map
  - Option B: compose-from-cache map
- ensure whichever final-image stage is chosen still receives the standard clip
  artifact

### 8A. `lambda/compose_from_slot_scores.c` or `roots2pix_mt.c` compose mode (Option B only)

- read raw slot-score cache fragments
- apply slot clip ranges
- run the existing RPN stack machine
- emit final grayscale fragments and final `step_scores.raw`

### 9. `index.html`

- accept lag tokens in `_compileSolveScoreChain`
- preserve them in `_serializeSolveScoreChain`
- show them in render summaries
- remove obsolete lores-clip render controls from the render dispatch path

### 10. `lambda/handler_palette_render_plan.py` and `lambda/handler_palette_chunk.py`

- reject lagged sources early with a clear fused-only error

This rejection belongs in the planner as well as the chunk worker path. The
operator should be told immediately, not after work has already been planned.

### 11. `lambda/handler_storage.py`

No behavioral change required beyond the compiler update. Save/load of
solve-score programs should round-trip lag tokens automatically.

## Implementation Order

1. Compiler and evaluator contract: `solve_score_chain.py` + `solve_score.h`
2. Full-resolution section overlap: `logical_sections.py` + `roots2pix_mt.c`
3. Stats pass in `handler_raster_mt.py`
4. Native stats merge in `solve_proximity_stats.c`, orchestrated by
   `handler_solve_proximity.py`
5. Choose final-image architecture:
   - Option A: reraster with the merged clip artifact
   - Option B: compose from raw slot-score cache fragments
6. Planner + ASL rewrite
7. Frontend parser/serializer + render UI cleanup
8. Early rejection in non-fused paths
9. Tests

The critical atomic boundary is the shared evaluator signature. All C callers
must update together. In practice the atomic deploy unit also includes the
Python handlers and ASL states that start emitting lagged `--score_sources`
payloads to the updated binaries. Old handlers plus new binaries, or new
handlers plus old binaries, are both invalid deploy states.

## Tests

### Compiler and parser

- Python compiler accepts `slv-1`, `cf-1`, `pm-1`
- JS compiler accepts and round-trips them
- C CSV parser accepts the six-token vocabulary
- malformed lag tokens are rejected everywhere

### Evaluator safety

- direct CLI of a non-fused caller with `--score_sources=slv-1` aborts with the
  expected stderr
- lag-free programs still behave exactly as before

### Section overlap

- manifest prelude grows logical size and shifts offsets correctly
- first section clamps prelude to zero
- native-thread byte ranges include the needed warmup row

### Score-stats pass

- stats mode emits one summary per section
- warmup rows are excluded from the summary counts
- lagged stats match a Python reference on a tiny synthetic input
- transformed-prev behavior matches a Python reference when rotation and root
  transforms are enabled
- under Option B, raw slot-score cache fragments have the expected binary shape
  and exclude warmup rows

### Stats merge

- native `stats_merge` yields per-slot clip ranges within a tight epsilon of an
  exact Python quantile calculation on a small test corpus
- slots are merged by slot index and source tag, not deduped by metric name
- stale `chain_fingerprint` summaries are rejected
- Python orchestration never parses the digest payload itself

### End-to-end raster semantics

- `proximity(slv-1)` at solve `i > 0` matches `proximity(slv)` at solve `i - 1`
- solve 0 obeys sentinel `prev = current`
- `avg(proximity(slv), proximity(slv-1))` matches the mean of two control runs
- `abs_diff(proximity(slv), proximity(slv-1))` produces the expected output on a
  synthetic solve stream with known adjacent differences

### Option A specific

- final reraster pass produces the same result as a direct single-pass control
  run when fed the same merged clip artifact

### Option B specific

- compose-from-cache produces pixel-identical output to Option A on the same
  synthetic corpus
- compose-from-cache does not re-read roots or re-run metric evaluation

### Planner and workflow

- fused solve-score render plan no longer requires lores clip scratch
- mixed-source fused render no longer requires lores coeff/param scratch for
  clipping
- ASL contains stats-map + stats-merge and no longer routes production render
  through the old lores clip task

### Storage and UI round-trip

- save/load of lagged solve-score programs is byte-identical
- Populate restores lagged programs
- solve-score modal Load restores lagged programs
- render request payload no longer includes obsolete lores-clip controls

### `step_scores.raw`

- stage 1 never writes `step_scores.raw`
- the final-image stage writes the reusable final `step_scores.raw`
- repalette / recolor / extract-palette continue to read stage-3 output exactly
  as they do today

### Rejection elsewhere

- palette render plan rejects lagged programs early
- palette chunk handler rejects lagged programs defensively

## Non-goals

- arbitrary lag-N
- changing solve-score semantics from slot-normalize-then-compose to
  output-normalize
- palette, bilevel, coeff-bilevel, or other non-fused lag support
- row-boundary or pass-boundary sentinel behavior beyond global solve 0
- redesigning unrelated preview tools in the same change
- inventing new pairwise metrics; the feature only adds lagged sources

## Acceptance Criteria

1. `slv-1`, `cf-1`, and `pm-1` are accepted everywhere they should be, and all
   malformed lag tokens are rejected clearly.
2. There is no planner-side or merge-side aliasing between `slv` and `slv-1`.
3. Production fused solve-score render no longer invokes the old lores clip
   prepass.
4. Fused solve-score render no longer requires lores coeff/param scratch to
   support mixed-source chains.
5. Full-resolution section stats are computed from the actual render solve
   stream.
6. Section stats preserve slot index, source tag, quantile, and chain
   fingerprint.
7. Lag-free programs remain byte-identical in output.
8. Lagged slots use the true previous solve, except for global solve 0 which
   uses the sentinel `prev = current`.
9. Previous-solve data is transformed through the same root-transform pipeline
   as current-solve data before scoring.
10. `abs_diff(proximity(slv), proximity(slv-1))` renders correctly on a known
    synthetic case and is not normalized from lores-derived ranges.
11. The doc and tests explicitly treat that `abs_diff` output as a mostly-dark
    correlated signal under v1 semantics, not as a richly spread output image.
12. `step_scores.raw` is written only by the final-image stage and downstream
    reuse paths continue to work unchanged.
13. Non-fused solve-score callers reject lagged programs early, and direct CLI
    misuse still aborts defensively in the evaluator.
14. Obsolete lores-clip render controls are removed from the production render
    path so the UI does not advertise dead tuning knobs.
15. Digest parsing, merge, and quantile extraction happen in native code; Python
    orchestration does not implement t-digest logic.
16. If Option A is chosen, the reraster path is explicitly accepted as the
    simpler but slower architecture.
17. If Option B is chosen, compose-from-cache is explicitly accepted as the
    more complex but more efficient architecture, and it produces output
    identical to Option A on the test corpus.
18. Saved solve-score programs, Populate, and modal Load all round-trip lagged
    chains without drift.
19. All updated and new tests pass.
