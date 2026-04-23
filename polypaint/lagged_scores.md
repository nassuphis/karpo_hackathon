# Lagged Scores Plan

Status: rewritten feature plan. This version replaces the previous
"lagged source as a separate metric distribution" model with a buffer-based
lag model.

The core idea is:

```text
metrics:
  m0 = proximity(slv, q=0.4)

program:
  m0-0;m0-1;abs_diff
```

`m0-0` reads the current value of metric slot `m0`.
`m0-1` reads the previous-row value of the same metric slot.

Lag is encoded in the program token, not in the metric definition.

## Goal

Let solve-score programs compare a metric value at the current solve row with
the same metric value from the immediately previous solve row.

Primary motivating program:

```text
m0-0;m0-1;abs_diff
```

where:

```text
m0 = proximity(slv, q=0.4)
```

This represents the difference between current proximity and previous-row
proximity using the same metric slot, the same source family, and the same
clip/quantile.

## Pinned Decisions

1. Lagged metric references do not create independent metric slots.
2. Metric definitions are base slots only: metric name, base source family, and
   quantile.
3. Lag is encoded only in program metric references: `mN-0` for current and
   `mN-1` for previous.
4. `--score_sources=` remains base-source only. It accepts `slv`, `cf`, and
   `pm`. It does not accept `slv-1`, `cf-1`, or `pm-1`.
5. The canonical program string uses `mN-L` tokens, where `L` is the lag depth.
   V1 accepts only `0` and `1`.
6. Current unlagged references may be parsed from legacy `mN`, but canonical
   serialization writes `mN-0`.
7. Lagged refs share the exact same clip range as the base metric slot. They do
   not get separate quantiles.
8. Production fused solve-score render keeps the current lores clip calibration
   path. Lagged refs share the base metric slot's lores-derived clip range.
9. V1 adds no new stats pass. The production path remains lores clip
   calibration followed by final raster.
10. V1 remains fused-color-only. Non-fused solve-score consumers reject lagged
    programs early and clearly.
11. `step_scores.raw` is written only by the final image raster stage.
12. Higher lags are deliberately not implemented in v1, but the model extends
    naturally to a ring buffer.

## Semantic Model

### Base Metric Slots

The compiled chain contains a list of base metric slots:

```json
{
  "metrics": [
    {
      "slot": 0,
      "metric": "proximity",
      "source": "slv",
      "quantile": 0.004
    }
  ],
  "program_spec": "m0-0;m0-1;abs_diff"
}
```

Valid base sources are:

- `slv`
- `cf`
- `pm`

Invalid in the metric list:

- `slv-1`
- `cf-1`
- `pm-1`

Those lag suffixes belong in program references only.

### Program Metric References

Program metric references have this canonical form:

```text
m<slot>-<lag>
```

Examples:

```text
m0-0
m0-1
m12-0
m12-1
```

Meaning:

- `m0-0` = current value of metric slot 0
- `m0-1` = previous-row value of metric slot 0
- `m12-0` = current value of metric slot 12
- `m12-1` = previous-row value of metric slot 12

Rejected in v1:

- `m0-2`
- `m0--1`
- `m0-01`
- `m0-lag`
- `m-1`
- `m0 -1`
- `M0-1`

Legacy `m0` is accepted as `m0-0` when reading older artifacts or tests, but
new writers emit `m0-0`.

### Quantile Ownership

The quantile belongs to the base metric slot.

For example:

```text
m0 = proximity(cf, q=0.4)
program = m0-0;m0-1;abs_diff
```

Both `m0-0` and `m0-1` use the same clip range derived from
`proximity(cf, q=0.4)`.

If a user-facing editor still accepts a syntax such as:

```text
proximity(cf-1, q=0.9)
```

that lag suffix must be lowered to a program reference such as `m0-1`. The
`q=0.9` value is not a separate lag quantile. It is ignored when an existing
base slot already defines the metric's quantile.

To avoid fake UI:

- the chip editor should not present an independent editable quantile for a
  lagged reference
- if a lagged-only reference creates a new base slot, its displayed `q` may seed
  that base slot
- once the base slot exists, lagged references show inherited quantile state

If lowering from a legacy/free-form shape is ambiguous, the compiler should
reject clearly rather than guessing.

### Lowering Algorithm

Both the Python and JavaScript compilers must implement the same lowering rules.
The chip editor should normally avoid the ambiguous cases by representing lag as
a reference to an existing metric slot, not as a separate metric declaration.

Canonical lowering:

```text
base_slots = []

for each metric-like user expression in display order:
    metric, source, lag, quantile = parse(expression)

    if lag == 0:
        slot = find_or_create_exact_base_slot(metric, source, quantile)
        emit_ref(slot, 0)
        continue

    if lag != 1:
        reject("lag depth other than 0 or 1 is unsupported")

    base_source = source_without_lag(source)
    candidates = base_slots with same metric and base_source

    if len(candidates) == 1:
        slot = candidates[0]
        # The lagged expression's quantile is ignored. The base slot owns q.
    elif len(candidates) == 0:
        slot = create_base_slot(metric, base_source, quantile)
    else:
        reject("lagged metric reference is ambiguous; choose an explicit slot")

    emit_ref(slot, 1)
```

Pinned examples:

- `proximity(cf, q=0.4); proximity(cf-1, q=0.4); abs_diff` lowers to one base
  slot with `q=0.4` and program `m0-0;m0-1;abs_diff`.
- `proximity(cf, q=0.4); proximity(cf-1, q=0.5); abs_diff` also lowers to one
  base slot with `q=0.4`; the lagged `q=0.5` is ignored because the base slot is
  unambiguous.
- `proximity(cf-1, q=0.9)` with no current sibling creates one base
  `proximity(cf, q=0.9)` slot and emits `m0-1`.
- `proximity(cf, q=0.4); proximity(cf, q=0.5); proximity(cf-1, q=0.7)` rejects,
  because two base slots could own the lagged reference.

### Visual Semantics Of `abs_diff`

`m0-0;m0-1;abs_diff` compares normalized values from adjacent rows of the same
base metric. For smooth parameter-space changes, those adjacent values are often
highly correlated, so the difference concentrates near `0` with spikes at
discontinuities.

That means the motivating lag-difference program can look like a mostly dark
edge-detect image by design. V1 does not add output-level re-normalization.
Operators who want to brighten that output should use the existing autolevels or
post-processing path.

## Final Raster Runtime Algorithm

In final raster mode, each `roots2pix_mt` native worker allocates two metric
buffers:

```c
float current_metric_buffer[M];
float recent_metric_buffer[M];
```

`M` is the number of base metric slots in the compiled program.

### Worker Initialization

If the program contains no `mN-1` references, no lag setup is needed.

If the program contains at least one lagged reference:

1. The worker obtains one previous solve row before its first scored row.
2. It evaluates the base metric slots needed by lagged program refs on that
   previous row.
3. It stores those values into `recent_metric_buffer`.

For global solve row `0`, no previous row exists. The sentinel rule is applied
after row 0 current metrics are evaluated and before the row 0 program is
evaluated:

```text
recent_metric_buffer = current_metric_buffer for row 0
```

That makes `mN-1` equal `mN-0` at the global first row.

### Per-Row Loop

For each scored solve row `i`:

1. Decode current row data.
2. Apply the same root-transform pipeline used by current fused raster.
3. Evaluate every base metric slot into `current_metric_buffer`.
4. Evaluate the program:
   - `mN-0` reads `current_metric_buffer[N]`
   - `mN-1` reads `recent_metric_buffer[N]`
5. Convert the program result to the final 1..255 solve byte.
6. Store the solve byte for `step_scores.raw`.
7. Project roots, claim pixels, and emit sparse fragments exactly as today.
8. Copy `current_metric_buffer` to `recent_metric_buffer`.
9. Advance to row `i + 1`.

The lag cost is therefore:

- one extra prelude row per worker boundary when lag is used
- one `M`-float buffer copy per solve row
- no second metric evaluation for lagged refs inside the same row

### Normalized vs Raw Buffer Values

In the final raster pass, the metric buffers hold normalized metric values after
per-slot clip ranges have been applied.

That preserves current solve-score semantics:

1. compute raw base metric value
2. apply that slot's clip range
3. clamp to `[0, 1]`
4. compose using the RPN program

The lores clip pass evaluates raw base metric values and computes clip ranges.
It does not need to evaluate lagged program refs because v1 does not add
independent lag quantiles or output-level normalization.

## Production Pipeline

Production fused solve-score render keeps the existing two-stage shape.

### Stage 1: Lores Clip Calibration

The existing solve-score clip stage remains in place.

It computes clip ranges from lores artifacts exactly as today:

- `lores.bin` for `slv` metrics
- `lores_coeffs.bin` for `cf` metrics
- `lores_params.bin` for `pm` metrics

Lagged refs do not create separate clip work.

The clip stage sees only the base metric slots:

```text
metrics:
  m0 = proximity(slv, q=0.4)
program:
  m0-0;m0-1;abs_diff
```

The clip stage computes one clip range for `m0`. Both `m0-0` and `m0-1` use
that same range in final raster.

This stage:

- does not evaluate `mN-1`
- does not need lag buffers
- does not need prelude rows
- does not compute full-resolution stats

### Stage 2: Final Raster Map

The existing full-resolution raster map runs with the lores-derived clip
artifact.

This is where:

- `current_metric_buffer` and `recent_metric_buffer` are normalized values
- `mN-0` / `mN-1` program refs are evaluated
- sparse image fragments are written
- `step_scores.raw` fragments are written
- associated palette fragments are written when enabled

Finalize remains unchanged.

## Performance Compared To Current Production Render

### Current No-Lag Path

Current production fused solve-score render has two expensive phases:

1. Lores clip calibration.
2. Full-resolution raster.

The lores clip phase reads low-resolution solve artifacts and computes clip
ranges from that approximation. It is much cheaper than a full render pass, but
its distribution is not the same object as the final full-resolution solve
stream.

The full-resolution raster phase then:

- reads each logical solve section
- applies root transforms
- evaluates solve-score metrics
- clips and composes the program
- projects roots to pixels
- claims pixels
- writes sparse image fragments and `step_scores.raw`

### New Lag-Capable Path

The lag-capable path keeps the same two expensive phases:

1. Lores clip calibration.
2. Full-resolution raster.

The clip phase remains essentially the same cost as today. It computes one clip
range per base metric slot. Lagged refs do not add clip work because they reuse
the base metric slot's range.

The final raster phase adds only the buffer-based lag machinery.

The expected cost shape is:

```text
current total ≈ lores_clip + full_raster
new total     ≈ lores_clip + full_raster_with_metric_buffers
```

`full_raster_with_metric_buffers` is only slightly more expensive than current
full raster when lag is used.

Expected feature penalty: close to zero for practical render workloads.

Reason: lag does not add a new clip pass, does not add a new full-resolution
stats pass, and does not re-evaluate metrics for lagged refs. It only changes
where program refs read their already-computed normalized metric values.

### Incremental Cost Of Lag

The cost of actually using lag in final raster is small.

For lag-free programs, final raster evaluates base metric slots into
`current_metric_buffer` and reads only `mN-0` refs.

For lagged programs, final raster additionally:

- reads one prelude solve row at worker/section boundaries
- keeps `recent_metric_buffer[M]`
- copies `current_metric_buffer` into `recent_metric_buffer` once per solve row
- reads `mN-1` refs from the recent buffer during program evaluation

It does not re-evaluate the metric for the lagged ref. It also does not ask the
clip stage for another quantile.

The lag overhead is therefore O(M) memory and O(M) copy per solve row, where
`M` is the number of base metric slots. In practice this is tiny compared with
root decoding, root transforms, metric evaluation, pixel projection, and pixel
claiming.

### Cost Summary

Compared with the current no-lag production renderer:

- Final raster with lag is only slightly more expensive than final raster
  without lag.
- The expected lag-feature performance penalty is close to zero in normal use.
- Memory overhead in final raster is two `M`-float buffers per native worker.
- The number of metric slots stays base-only, so `m0-0;m0-1;abs_diff` still has
  one metric slot, not two.
- Lores clip calibration stays in place and should have roughly the same cost as
  today for the same base metric list.

This design intentionally keeps the current lores-derived quantile contract and
makes lag cheap by reading previous normalized metric values from a buffer.

## Clip Artifact Contract

The existing solve-score clip artifact remains the clip contract. It has one
entry per base metric slot:

```json
{
  "family": "solve_score",
  "version": 2,
  "chain_fingerprint": "sha256:...",
  "program": "m0-0;m0-1;abs_diff",
  "metrics": [
    {
      "slot": 0,
      "metric": "proximity",
      "source": "slv",
      "quantile": 0.004,
      "clip_lo": 0.0021,
      "clip_hi": 1.934
    }
  ]
}
```

Pinned properties:

- no `source` value contains a lag suffix
- there is no separate metric entry for `m0-1`
- clip ranges are per base slot, not deduped by metric name alone
- clip ranges are derived from lores artifacts, as today
- `chain_fingerprint` includes the lagged program refs, so `m0-0` and
  `m0-0;m0-1;abs_diff` cannot share stale clip artifacts by accident

Backward compatibility:

- New v2 clip artifacts should always include the canonical `program` field.
- Readers must tolerate older v2 clip artifacts that lack `program` when the
  fingerprint and metric entries still match.
- Missing `program` on an old artifact must not be treated as permission to
  ignore a fingerprint mismatch.

## Source And Final-Raster Prelude Contract

### Source Families

The base metric source family determines which input stream a metric uses:

- `slv` reads roots from the solve stream
- `cf` reads coefficient roots from the coefficient source manifest
- `pm` reads parameter rows from the parameter source manifest

Lag does not alter the source family. It only changes whether the program reads
the current or previous metric-buffer value.

### Prelude Rows

A one-row prelude is required only by the final raster stage when a program
contains lagged references.

The lores clip stage does not evaluate `mN-1` refs, so it does not need prelude
rows.

The planner derives per-family prelude requirements from lagged program refs:

```text
program ref mN-1
  -> metric slot N
  -> metric source family
  -> that family needs one prelude row
```

Examples:

```text
m0 = proximity(slv, q=0.4)
program = m0-0;m0-1;abs_diff
```

Requires one `slv` prelude row.

```text
m0 = spread(cf, q=0.4)
program = m0-1
```

Requires one `cf` prelude row.

```text
m0 = proximity(slv, q=0.4)
m1 = max_re(pm, q=0.4)
program = m0-0;m1-1;avg
```

Requires one `pm` prelude row. It does not require an `slv` prelude row because
`m0` is only read as current.

### Logical Section Boundary

If a logical section starts at global row `S > 0` and any required source family
needs lag, the section manifest includes row `S - 1` as a prelude row for that
family.

The prelude row is available to the worker but is not counted as a scored row.

### Native Thread Boundary

Threads do not share section buffers. Therefore every native thread that starts
at scored row `T` needs access to row `T - 1` for lag initialization.

Implementation rule:

- if `T == 0`, initialize lag with the row-0 sentinel
- otherwise, the thread's input read begins at `T - 1`
- the thread's scoring loop still begins at `T`
- prelude row bytes are excluded from final raster fragments and step scores

Boundary table:

| Section | Thread | Required initialization |
| --- | --- | --- |
| first section (`S == 0`) | first thread (`T == 0`) | evaluate row 0 once and copy current to recent |
| first section (`S == 0`) | later thread (`T > 0`) | read in-section row `T - 1` as prelude |
| later section (`S > 0`) | any thread | read global row `S + T - 1`; for `T == 0`, this is the section prelude row |

This applies independently to `slv`, `cf`, and `pm` streams when their metric
slots are read lagged.

## Chain Compiler Contract

### Python Compiler

`solve_score_chain.py` changes:

- metric sources remain `slv`, `cf`, `pm`
- lag suffixes are represented on program refs, not metric sources
- compiled `metrics[]` contains base slots only
- compiled `program_spec` emits `mN-0` and `mN-1`
- fingerprints include the full canonical `program_spec`
- helpers expose:
  - whether the program uses lag
  - max lag depth
  - lagged metric slots
  - per-family prelude requirements

### JS Compiler

The frontend compiler mirrors the Python compiler:

- accepts user-facing lag references
- lowers them to base metric slots plus `mN-1` refs
- serializes canonical program refs
- round-trips saved solve-score programs without losing lag refs

The UI must not expose a fake independent quantile for a lagged ref. A lagged
chip should show that its quantile is inherited from the base slot.

### C Parser

`solve_score.h` changes:

- `parse_solve_score_source_csv` continues to accept only `slv`, `cf`, `pm`
- `parse_solve_score_program_spec` accepts `mN`, `mN-0`, and `mN-1`
- canonical tests expect writers to emit `mN-0` for current refs
- `SolveScoreProgramToken` stores both:
  - metric slot
  - lag depth

V1 rejects any lag depth other than `0` or `1`.

## Evaluator Contract

The evaluator should be split so program composition can run from buffers.

Recommended shape:

```c
int solve_score_eval_metric_slots(
    const float *roots, int degree,
    const float *coeffRoots, int coeffDegree,
    const float *paramValues, int paramDegree,
    const SolveScoreProgram *program,
    const double *clipLo,
    const double *clipHi,
    float *outMetricBuffer,
    char *err, size_t errCap);

int solve_score_eval_program_from_buffers(
    const float *currentMetricBuffer,
    const float *recentMetricBuffer,
    const SolveScoreProgram *program,
    double *out,
    char *err, size_t errCap);
```

Capacity and value contracts:

- `outMetricBuffer` must have space for `program->metricCount` floats.
- `currentMetricBuffer` and `recentMetricBuffer` contain normalized `[0, 1]`
  metric values after per-slot clip application.
- `solve_score_eval_program_from_buffers` must treat both buffers as read-only.

The current convenience wrapper can remain for lag-free direct callers, but a
lagged program must fail if no recent buffer is supplied.

No silent fallback is allowed. These are errors:

- program contains `mN-1` but caller passes no recent buffer
- `mN-1` references a slot outside the metric list
- source CSV contains `slv-1`, `cf-1`, or `pm-1`
- program contains `mN-2` in v1

## Planner And Workflow Contract

### Plan Fields

`plan.solve_score` includes:

- `chain`
- `clip_key`
- `uses_lag`
- `max_lag`

`plan.raster` includes per-family prelude flags/counts:

- `prelude_rows`
- `score_coeff_prelude_rows`
- `score_param_prelude_rows`

All values are integers. V1 values are `0` or `1`.

### ASL States

The render workflow keeps the existing color solve-score clip task:

- existing `ColorClipPhase`
- existing `ColorClipTask`
- existing `ColorRasterPhase`
- existing `ColorRasterMap`
- existing finalize states

The ASL must be generated/checked against `lambda/workflow_contracts.py`. The
payload contract maps must be added before implementation, not reconstructed by
memory.

Color clip payload remains lores-based and must thread:

- `job_id`
- `degree`
- `solve_score_chain`
- `solve_score_threads`
- `lores_bin_key`
- `lores_coeffs_key`
- `lores_params_key`
- `n_coeffs`
- `root_transforms`
- `out_key`

Final raster payload must thread:

- `solve_score_clip_key`
- `fragment_prefix`
- `prelude_rows`
- `score_coeff_prelude_rows`
- `score_param_prelude_rows`
- associated palette fragment fields

Finalize continues to consume:

- sparse fragments
- `step_scores.raw`
- clip slots from the lores-derived clip artifact

## File-Level Plan

### `lambda/solve_score_chain.py`

- lower lagged refs into `mN-1` program refs
- keep `metrics[]` base-only
- reject ambiguous legacy lowering
- include lag refs in fingerprints
- expose prelude helper functions

### `index.html`

- mirror compiler lowering
- show lagged refs in the chip editor without fake independent quantile state
- saved solve-score modal round-trips lagged programs
- Populate restores lagged programs
- render summaries display lag refs clearly

### `lambda/solve_score.h`

- parse `mN-0` / `mN-1`
- store lag depth on push-metric tokens
- add buffer-based program evaluation
- keep source CSV base-only
- reject lag misuse loudly

### `lambda/logical_sections.py`

- support per-family prelude rows
- prepend one logical row when needed
- keep scored row counts unchanged
- expose enough metadata for ASL payloads and tests

### `lambda/roots2pix_mt.c`

- allocate `current_metric_buffer` and `recent_metric_buffer`
- seed recent buffer from prelude row or row-0 sentinel
- evaluate base metrics once per scored row
- evaluate program from current/recent buffers
- continue writing sparse fragments and `step_scores.raw` in raster mode

### `lambda/handler_raster_mt.py`

- prepare final raster manifests with the required prelude rows
- keep final raster upload behavior unchanged except for prelude fields

### `lambda/solve_proximity_stats.c`

- keep the existing lores clip-calibration role
- compute clip ranges for base metric slots only
- preserve canonical lagged `program_spec` in the clip artifact
- do not create lagged metric entries

### `lambda/handler_solve_proximity.py`

- keep the existing render-workflow clip phase
- invoke lores clip calibration as today
- upload the standard clip artifact
- keep unrelated histogram/debug behavior clearly separate

### `lambda/handler_render_plan.py`

- compute lag/prelude requirements from the compiled chain
- keep `clip_key` as the final raster clip contract
- reject lagged programs outside fused color render

### `lambda/workflow_contracts.py`

- keep the color clip payload contract lores-based
- update final raster payload contract with prelude fields

### `stepfunctions/render_workflow.asl.json.template`

- keep `ColorClipTask`
- thread all fields defined in `workflow_contracts.py`
- keep final raster/finalize paths deterministic

### Non-Fused Consumers

Reject lag early in:

- palette render planner
- palette chunk handler
- direct C CLI paths that cannot supply recent buffers

The error should say lagged solve-score refs are supported only by fused color
render in v1.

### Atomic Deploy Boundary

These files form one contract boundary and must ship together:

- `lambda/solve_score.h`
- `lambda/roots2pix_mt.c`
- `lambda/handler_raster_mt.py`
- `lambda/handler_render_plan.py`
- `lambda/solve_score_chain.py`
- `lambda/solve_proximity_stats.c`
- `lambda/workflow_contracts.py`
- `stepfunctions/render_workflow.asl.json.template`
- `index.html`

Do not stage parser/compiler support separately from the ASL and handler payload
changes. A lagged program must not reach a raster Lambda that cannot provide
recent buffers and prelude rows.

## Tests

### Compiler Tests

- Python compiler lowers current refs to `mN-0`
- Python compiler lowers lag refs to `mN-1`
- JS compiler matches Python canonical output
- legacy `mN` parses as `mN-0`
- `mN-2` rejects
- `slv-1` / `cf-1` / `pm-1` never appear in compiled `metrics[].source`
- fingerprints differ between `m0-0` and `m0-0;m0-1;abs_diff`
- lagged refs share the base slot quantile
- ambiguous legacy quantile/source lowering rejects clearly
- `proximity(cf, q=0.4)` plus `proximity(cf-1, q=0.5)` lowers to one base slot
  with `q=0.4`
- lagged-only `proximity(cf-1, q=0.9)` creates one base `proximity(cf, q=0.9)`
  slot
- two current base slots with the same metric/source but different quantiles make
  a free-form lagged ref ambiguous

### C Parser And Evaluator Tests

- source CSV accepts only `slv`, `cf`, `pm`
- source CSV rejects `slv-1`, `cf-1`, `pm-1`
- program parser accepts `m0`, `m0-0`, `m0-1`
- program parser canonical behavior treats `m0` as current
- program parser rejects malformed lag tokens
- buffer evaluator reads current buffer for `mN-0`
- buffer evaluator reads recent buffer for `mN-1`
- buffer evaluator does not mutate current or recent buffers
- lagged program without recent buffer fails clearly
- lag-free programs remain byte-identical

### Runtime Boundary Tests

- global row 0 uses `recent = current`
- worker starting at row `T > 0` reads row `T - 1`
- logical section starting at `S > 0` includes required prelude rows
- prelude rows are excluded from fragments
- prelude rows are excluded from `step_scores.raw`
- per-family prelude is requested only for lagged metric slots that use that
  family

### Clip Tests

- lores clip emits one clip entry per base metric slot
- lores clip emits no separate lagged metric entries
- lores clip preserves canonical lagged `program_spec`
- lores clip does not request final-raster prelude rows
- clip artifact validation rejects stale chain fingerprints
- clip artifact validation rejects slot/source/quantile drift
- new lagged clip artifacts include `program`
- old lag-free v2 clip artifacts without `program` remain readable when the
  fingerprint and metric entries match

### End-To-End Synthetic Tests

- `m0-1` at row `i > 0` equals `m0-0` at row `i - 1`
- row 0 sentinel makes `abs_diff(m0-0, m0-1) == 0`
- `m0-0;m0-1;abs_diff` matches a small Python reference
- correlated adjacent-row values produce a low-valued `abs_diff` output unless
  post-processed
- root transforms are applied before metric evaluation for both current and
  prelude rows
- associated palette output remains aligned with final solve bytes
- `step_scores.raw` from final raster remains usable by repalette/recolor paths

### Workflow Contract Tests

- ASL keeps `ColorClipPhase` and `ColorClipTask`
- ASL color clip payload remains lores-based
- ASL color clip payload equals `workflow_contracts.py`
- ASL final raster payload includes prelude fields
- render plan emits prelude counts derived from lagged program refs
- render plan still provides lores keys needed by clip calibration

### UI Tests

- chip editor can create a lagged ref
- lagged ref displays inherited quantile, not an independent fake quantile
- solve-score modal save/load round-trips lagged programs
- Populate restores lagged programs
- render summary displays lagged program refs
- render request payload does not add fake lag quantile controls

### Rejection Tests

- palette planner rejects lagged programs early
- palette chunk handler rejects lagged programs defensively
- direct C CLI misuse rejects lagged program without recent buffer

### Deployment / Packaging Tests

- changed C binaries are packaged with handlers that invoke them
- changed Python helper dependencies are packaged with handlers that import them
- predeploy contract gate includes ASL/workflow contract checks
- Docker runtime regression covers the new parser/evaluator behavior

## Future Higher Lags

Higher lags are not part of v1, but the design is intentionally compatible.

The extension is:

```text
m0-0  current
m0-1  previous row
m0-2  two rows back
...
```

Runtime changes for lag `N`:

- replace the two buffers with a ring buffer of `N + 1` metric buffers
- fetch `N` prelude rows at worker/section boundaries
- parse and validate `mK-N`
- keep clip calibration unchanged because all lag depths still share the base metric slot
  quantile

This is straightforward, but v1 stays at lag 1 to keep boundary handling and
tests tight.

## Non-Goals

- arbitrary lag depth in v1
- output-level normalization
- pairwise current-vs-previous root metrics
- separate quantiles for lagged refs
- raw slot-score cache / compose-from-cache architecture
- replacing production lores clip calibration
- palette, bilevel, coeff-bilevel, or non-fused lag support
- preview latency redesign

## Acceptance Criteria

1. Compiled metric lists contain only base source families.
2. Lag appears only in program refs such as `m0-1`.
3. Canonical writers emit `mN-0` for current metric refs.
4. Lagged refs share the base metric slot's quantile and clip range.
5. `--score_sources=` rejects lag suffixes.
6. `roots2pix_mt` uses current/recent metric buffers for final raster.
7. Global row 0 uses the current-row sentinel for lagged refs.
8. Final raster worker and section starts read the needed prelude rows.
9. Final-raster prelude rows do not affect image fragments or
   `step_scores.raw`; the lores clip stage does not request them.
10. Lores clip calibration emits only base metric slots.
11. Production fused render keeps the existing lores clip calibration path.
12. No new full-resolution stats or native stats-merge stage is introduced.
13. Non-fused consumers reject lagged programs early.
14. Saved programs, Populate, and solve-score modal Load all round-trip lagged
    refs without drift.
15. All workflow contract changes are reflected in `workflow_contracts.py` and
    the ASL template.
16. All new and existing tests pass, including the predeploy contract gate and
    Docker runtime regression.
