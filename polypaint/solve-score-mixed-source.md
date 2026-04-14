# Mixed-Source Solve Score Design Note

Status:

- Phase 1 landed:
  - source-aware metric chips (`slv` / `cf`)
  - mixed-source histogram debug on `lores.bin` + `lores_coeffs.bin`
- Full render/palette mixed-source runtime is still intentionally blocked.

This note extends the current solve-score chip/RPN system to support metrics
from more than one underlying vector source.

Current examples:

- `spread(slv,q=5%)`
- `spread(cf,q=5%)`
- `avg`

Meaning:

- evaluate one metric on the solve vector
- evaluate one metric on the coefficient vector
- clip and normalize each independently to `[0,1]`
- combine them in the existing score program

## Short Answer

Yes, this is feasible.

The important reason it is feasible is:

- solve bins and coeff bins are both solve-aligned row stores
- each row corresponds to one solve step
- the combine stage already expects normalized per-solve scalars

So the hard part is not the combination.

The hard part is:

- adding a second metric-evaluation pipeline over coefficient vectors
- then feeding those clipped metric slots into the same program evaluator

## Core Rule

No metric should ever be combined in raw units.

That is already true for solve-only programs and must stay true for mixed
source programs.

The correct order is:

1. evaluate raw metric per slot
2. compute slot-specific clip bounds
3. map each slot to `[0,1]`
4. combine normalized slots via `avg`, `max`, `mul`, `weighted_sum`, etc.

So for:

- `spread(slv,q=5%)`
- `spread(cf,q=5%)`
- `avg`

the runtime should do:

- raw solve spread -> clip solve spread -> normalized solve spread
- raw coeff spread -> clip coeff spread -> normalized coeff spread
- `avg(normalized_solve_spread, normalized_coeff_spread)`

This is not optional.

If raw values are combined directly, mixed-source programs become meaningless.

## Canonical Model

Each metric slot should carry:

- `source`
  - `slv`
  - `cf`
- `metric`
- `quantile`

Example canonical chain:

- `spread(slv,q=5%)`
- `spread(cf,q=5%)`
- `avg`
- `omega_cosine(3)`

Internally this should still compile to the same postfix/RPN program model.

The only extension is that metric slots are no longer just:

- `m0`
- `m1`

They are metric slot records with source-aware metadata.

## User-Facing Syntax

Recommended v1 syntax:

- `spread(slv,q=5%)`
- `clusteriness(slv,q=3%)`
- `spread(cf,q=5%)`

This is better than inventing separate metric names like:

- `spread_solve`
- `spread_coeff`

because:

- the metric identity stays stable
- source becomes one explicit parameter
- future sources can fit the same model

## What "cf" Means

`cf` should mean:

- the complex coefficient vector for that solve step

not:

- the coefficient index axis as a geometric x-coordinate
- not any separately rasterized artifact

The coefficient vector is treated as a complex point cloud, same as roots are.

That means many existing solve metrics can be reused conceptually:

- `spread`
- `crowding`
- `anisotropy`
- `area`
- `centroid_re`
- `centroid_im`
- `centroid_dist`
- `asymmetry_re`

Some metrics may need judgment:

- `dist_unit_circle` still makes sense on coefficients
- nearest-neighbor style metrics still make sense on coefficients
- "root meaning" is different, but the geometry is still well-defined

So the note assumes:

- coefficient metrics are geometric metrics over the coefficient cloud

not a different semantic family.

## Why This Is Structurally Possible

What already exists:

- coefficient chunks are persisted:
  - [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py)
  - keys like `renders/<job>/coeffs_0000.bin`
  - plus `lores_coeffs.bin`
- solve-score programs already support:
  - multiple metric slots
  - combination ops
  - unary transforms
- render and palette consumers already consume per-slot normalized score inputs

So the missing layer is not storage or program structure.

The missing layer is:

- coefficient metric evaluation + coefficient clip prepass

## Current Limitation

Today the shared scoring implementation in
[lambda/solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)
only knows how to evaluate metrics on root vectors.

Current consumers:

- [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)
- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
- [lambda/roots2pix_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix_mt.c)
- [lambda/solve_palette_chunk.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_chunk.c)
- [lambda/solve_palette_chunk_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_chunk_mt.c)

Those all read root bins, not coefficient bins.

So mixed-source support requires a new source-aware metric evaluator.

## Recommended Runtime Shape

### Option A: source-aware shared metric header

Add a new shared layer that evaluates metrics over generic complex vectors:

- vector source = `slv`
- vector source = `cf`

Conceptually:

- `compute_vector_metric_score(values, n, metric, source_kind)`

Then:

- solve paths feed root rows
- coeff paths feed coefficient rows

Pros:

- most coherent long-term design
- avoids duplicating geometric metric formulas
- keeps one metric implementation per metric

Cons:

- larger refactor now

### Option B: keep solve metric code, add parallel coeff metric code

Implement a separate coefficient metric evaluator and keep solve metrics
separate.

Pros:

- smaller incremental patch
- easier to land first

Cons:

- high drift risk
- same metric ends up implemented twice

Recommendation:

- Option A is the right architecture
- Option B is acceptable only if used as a short-lived bridge

## Program Compilation Model

The chain compiler should compile metric slots to source-aware slot records.

Example compiled chain:

- `spread(slv,q=5%)`
- `spread(cf,q=5%)`
- `avg`

Compiled metadata:

- slot 0
  - source=`slv`
  - metric=`spread`
  - quantile=`0.05`
- slot 1
  - source=`cf`
  - metric=`spread`
  - quantile=`0.05`
- program=`m0;m1;avg`

Important:

- the postfix program itself can stay the same
- source belongs to slot metadata, not to the op token syntax

That keeps the evaluator simple.

## Clip Artifacts

Each slot needs its own clip result.

For mixed-source chains that means the clip prepass must emit per-slot records
like:

- slot
- source
- metric
- quantile
- clip_lo
- clip_hi

Exactly the same rule as today, just with `source` added.

The renderer should never assume:

- same source for every slot
- same clip range for every slot

## Debug-First Rollout

Recommended phases:

### Phase 1: histogram debug only

Support mixed-source chains in histogram debug first.

Reason:

- easiest way to validate semantics
- easiest way to inspect slot-level behavior
- no palette/render pipeline risk yet

Required inputs:

- `lores.bin` for solve metrics
- `lores_coeffs.bin` for coeff metrics

This phase should surface in the debug log:

- compiled program
- slot list with source/metric/q
- per-slot clip ranges

### Phase 2: render solve-score path

Support mixed-source chains in:

- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
- [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)

This requires mixed-source bins artifacts to be produced during the solve-score
prepass.

### Phase 3: palette/associated-palette paths

Support the same mixed-source programs in:

- [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
- associated-palette extraction paths

## UX Recommendation

Do not expose source as a separate global toggle.

Put source on the metric chip itself.

That is the honest model.

Good:

- `spread(slv,q=5%)`
- `spread(cf,q=5%)`

Bad:

- one global "score source" selector

because mixed-source programs need slot-local source choice.

## Good First Examples

Useful first mixed-source programs:

- `spread(slv,q=5%) spread(cf,q=5%) avg`
- `spread(slv,q=5%) spread(cf,q=5%) abs_diff`
- `clusteriness(slv,q=3%) anisotropy(cf,q=2%) weighted_sum(0.7,0.3)`
- `spread(cf,q=5%) sawtooth(2)`

Note on your example:

- `spread(slv,q=5%) spread(cf,q=5%) avg`

is valid because both slots are normalized to `[0,1]` before `avg`.

That is exactly the intended model.

## Open Questions

1. Which metrics are officially supported on `cf` in v1?

My recommendation:

- start with the obvious geometric ones
- do not promise every existing solve metric on coefficients on day one

2. Should mixed-source debug require both lores artifacts to exist?

Yes.

If a chain mentions `cf`, histogram debug should fail clearly when
`lores_coeffs.bin` is missing.

3. Should source participate in artifact identity?

Yes.

Two chains that differ only by source are different scoring programs and must
produce different program ids / artifact ids.

## Recommendation

Build this as:

1. source-aware metric chips
2. per-slot source-specific clip bounds
3. same normalized `[0,1]` combine model as current solve-only programs
4. histogram debug first
5. real render/palette support second

So the direct answer is:

- yes, mixed `slv` + `cf` score programs are a good extension
- yes, they are practical because row alignment already exists
- and yes, per-slot clipping to `[0,1]` is mandatory before any combine op
