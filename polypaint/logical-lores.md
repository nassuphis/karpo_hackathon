# Logical Lores Design Notes

## Status

This document captures the current understanding of "logical lores": a low
resolution view derived from the full-resolution solve artifacts, rather than a
separate low-resolution compute run.

The goal is not to remove the current lores path. At the current `lores_N=60`
size, physical lores is faster and simpler. Logical lores adds capabilities that
physical lores cannot provide:

- previews and summaries sampled from the same hires artifacts as final render;
- better solve-score range estimation from samples of the real full solve;
- arbitrary larger sample grids without recomputing a new solve;
- reusable/cached logical views when multiple resolutions or score programs are
  tested.

The headline justification is consistency with the final render and better
full-solve sampling. It is not a speed replacement for the existing small lores
compute.

## Current Pipeline

The compute pipeline currently produces separate lores artifacts:

- `lores_params.bin`
- `lores_coeffs.bin`
- `lores.bin`

Those are produced by a real additional solve pass at `lores_N`, not by
downsampling the hires artifacts. The lores param generation uses `N=lores_N`
but keeps `gridN=full_N`, so transform/dither sizing remains tied to the final
grid scale.

The hires solve does not require lores. Lores is currently useful for:

- solve-score clip/range estimation;
- histogram/debug summaries;
- render-preview colorization of existing lores artifacts;
- viewport/debug style inspection.

The current color render path is fused:

1. Run a lores solve-score clip prepass.
2. Raster hires logical sections with `roots2pix_mt`.
3. Compute solve scores during rasterization.
4. Emit raw score bytes as sparse fragments.
5. Finalize assembles raw bytes, builds the byte histogram, equalizes, and colorizes.

There is no separate solve-score histogram/merge stage in the shipped fused color
render path.

## Core Idea

A full solve with `full_N * full_N * times` rows contains a natural low
resolution subset. A `view_N * view_N * times` logical view can be obtained by
selecting rows from the full artifacts.

This is not equivalent to the current lores compute in every detail, because the
current lores run has its own row indices and RNG/dither sequence. But for render
preview and score-range estimation, a logical view has a major advantage: it is a
sample of the actual full solve that will be rendered.

The logical view should include all source families needed by the score program:

- `slv`: solved roots;
- `cf`: coefficient-derived roots;
- `pm`: parameter rows.

All families must be sampled with the same logical row mapping so mixed-source
score programs remain coherent.

The same logical solve index maps to a different byte stride per family:

```text
physical_solve_index = hires_index(logical pass, logical row, logical column)

slv row bytes = degree * 2 * sizeof(float)
cf  row bytes = n_coeffs * 2 * sizeof(float)
pm  row bytes = 2 * sizeof(double)

slv byte offset = physical_solve_index * slv row bytes
cf  byte offset = physical_solve_index * cf row bytes
pm  byte offset = physical_solve_index * pm row bytes
```

For chunked hires artifacts, `physical_solve_index` first maps to the chunk
whose `[step_start, step_start + step_count)` contains it. The byte offset within
that chunk is:

```text
local_row = physical_solve_index - chunk.step_start
chunk_byte_offset = local_row * family_row_bytes
```

For chunk-local params, use the chunk's `params_key` and the same `local_row`.
For any future global params object, use `physical_solve_index * pm row bytes`.

## Row Mapping

Rows are pass-major. Within a pass, rows are ordered by `i1`, and each `i1` row
uses serpentine `i2` order.

For the exact divisible case:

```text
ratio = full_N / view_N

logical pass p, logical row li, logical column lj

hi = li * ratio
i2_lo = (li is odd) ? (view_N - 1 - lj) : lj
i2_hi = i2_lo * ratio
hj = (hi is odd) ? (full_N - 1 - i2_hi) : i2_hi

hires_index =
    p * full_N * full_N
  + hi * full_N
  + hj
```

A flat stride of `(full_N / view_N)^2` is not correct in general, because it does
not preserve the 2D serpentine ordering.

For non-divisible `full_N / view_N`, use proportional integer mapping:

```text
hi = floor(li * full_N / view_N)
i2_hi = floor(i2_lo * full_N / view_N)
```

The exact rounding policy can be chosen later. The important property is that the
view remains stable, deterministic, monotone, and shared by all source families.

## Lag Semantics

Lagged score programs should operate on the logical view stream, not on adjacent
physical hires rows.

That means if the compact logical view is materialized as:

```text
logical row 0
logical row 1
logical row 2
...
```

then `m0-1` naturally means the previous logical sample. This matches what a
preview user expects from a lower-resolution solve view.

If instead the rasterizer reads physical hires rows directly with strides, lag
support becomes more complicated. It must preserve previous logical samples
across section/thread boundaries. That is a strong reason to materialize a
compact logical view first in v1.

## Performance Model

Point-by-point S3 range reads are the wrong shape. A `view_N * view_N` sample
would require thousands or millions of small reads.

The practical shape is row-band fetching:

1. Select `view_N` source rows per pass.
2. Fetch the full contiguous hires row for each selected source row.
3. Decimate columns locally.
4. Write compact logical artifacts to `/tmp`.

This uses approximately `view_N * times` range reads per artifact family, before
coalescing. It reads roughly:

```text
view_N / full_N
```

of each full artifact when full rows are fetched.

Approximate root bytes:

```text
full roots bytes = times * full_N * full_N * degree * 2 * sizeof(float)
logical row-band bytes = times * view_N * full_N * degree * 2 * sizeof(float)
compact logical roots bytes = times * view_N * view_N * degree * 2 * sizeof(float)
```

Coeff and param artifacts follow the same pattern with their own row widths.

For the current small `60x60` physical lores case, this is probably slower than
recomputing params, coeffs, and solves. For larger logical sample grids, the
tradeoff can flip because row-band materialization scales mostly with `view_N`
while recomputation scales with `view_N^2`. For `view_N` close to `full_N`, the
row-band method degenerates toward reading the full artifact; at that point the
normal full path or a more native strided reader is preferable.

## Implemented Preview V1

Inputs:

- `calc.json` chunk metadata;
- `full_N`;
- `view_N`;
- `times`;
- `degree`;
- `n_coeffs`;
- requested source families: `slv`, `cf`, `pm`.

Outputs:

- `/tmp/render_lores_preview_roots.bin`
- `/tmp/render_lores_preview_coeffs.bin`
- `/tmp/render_lores_preview_params.bin`

The materializer should:

1. Build the deterministic logical-to-hires row map.
2. Range-fetch selected source rows from S3 using existing manifest metadata.
3. Decimate columns locally.
4. Write compact contiguous artifacts in logical order.
5. Return metadata describing `view_N`, `full_N`, selected rows, bytes read, and
   compact output sizes.

Then existing binaries can consume the compact files:

- `solve_proximity_stats` for summaries/range estimation.
- `roots2pix_mt` for Render / Output / Preview through a local `file://`
  multispan manifest.

The compact file establishes the correct logical order for lagged refs, but it
does not by itself solve every consumer's boundary handling. Any threaded or
sectioned consumer still needs the normal lag prelude contract so the first row
of a section can see the previous logical row.

Preview v1 accepts that limitation. It is a test surface for visual comparison,
not a promise that lagged programs match full render exactly at thread
boundaries.

## Caller And Caching Model

Do not start with a separate frontend-visible materialization API. V1 is an
internal helper used by the existing Render / Output / Preview endpoint when the
`Logical` checkbox is enabled.

Recommended staging:

1. Inline ephemeral materialization inside `handler_render_lores_preview`.
   The handler materializes into `/tmp`, runs the same score summary and raster
   path, returns an inline PNG, and writes no new S3 artifacts.
2. Reuse the same helper inside the solve-summary/histogram path for
   score-range estimation.
3. Add persistent S3 caching only if repeated use justifies it. If added, cache
   keys for compact logical artifacts should include `(job_id, view_N, families,
   mapping_version)`. Cached summary results would additionally include the
   score-program signature.

The in-process `/tmp` cache can be keyed by `(job_id, view_N, families_tuple)`.
Evict by total compact bytes and age. A safe first rule is no cache persistence
across Lambda invocations beyond normal warm-start reuse; if the next request
does not find the exact key and intact file sizes, rematerialize.

V1 should enforce a compact-output budget before fetching. Compute:

```text
compact_bytes =
    view_N * view_N * times *
    sum(row_bytes for requested families)
```

Reject or downshift when `compact_bytes` plus row buffers exceeds the configured
`/tmp` budget. This is better than a hard `view_N` cap because degree and
requested families dominate size.

The current preview UI and handler use a conservative `view_N <= 256` cap as the
first safety rail. A byte-budget guard is still the better long-term rule.

## Rasterizer Implications

A simple `source_grid_n` argument to `roots2pix_mt` is not enough. The rasterizer
currently assumes a contiguous logical stream from `step_start` and `step_count`.

Direct native logical-lores support would need:

- a 2D row/column mapping from logical view rows to source rows;
- row-band or strided source reads;
- mixed-family alignment for `slv`, `cf`, and `pm`;
- lag prelude handling in logical-view order;
- section/thread boundary handling for lagged refs.

That is doable, but it is more risk than needed for the first version.

Render preview v1 uses compact local artifacts rather than native strided reads.
The native prerequisite for that path is:

- `roots2pix_mt` supports `file://` manifest sources through
  `multispan_reader`.

The remaining correctness gap is lagged solve-score refs at section/thread
boundaries. Until that is fixed, logical preview should be described as a visual
test surface, not as guaranteed byte-parity with full render.

## Score Range Estimation

Logical lores enables a better range-estimation workflow than current physical
lores because the samples are taken from the actual hires artifacts. A concrete
v1 model should be simple, logged, and conservative.

1. Materialize a `100x100` logical view.
2. Compute score quantiles such as `q05/q95`.
3. Materialize a `200x200` logical view.
4. Recompute quantiles.
5. Extrapolate q05 and q95 independently toward `full_N`.

Use an inverse-resolution endpoint model:

```text
t(N) = 1/N - 1/full_N

given endpoint e1 at N1 and e2 at N2:

slope = (e1 - e2) / (t(N1) - t(N2))
e_full = e2 - slope * t(N2)
```

Run that for both `q05` and `q95`. If the extrapolated range is invalid,
non-finite, or implausibly narrow, fall back to the largest measured logical
range. Otherwise apply a safety widening before using it:

```text
width_delta = abs((q95_N2 - q05_N2) - (q95_N1 - q05_N1))
lo = q05_full - safety * width_delta
hi = q95_full + safety * width_delta
```

Start with `safety=0.5` or `1.0`; tune only after measuring real jobs.

For lagged-current differences, the range often shrinks as density increases.
The extrapolation should be treated as a heuristic, not a guarantee. Every
summary should log `N1`, `N2`, measured q05/q95, extrapolated q05/q95, safety,
and fallback reason if a fallback is used.

A conservative v1 policy:

- use q05/q95, not raw min/max;
- compare two or three logical resolutions;
- widen the selected range rather than overfitting;
- log the input resolutions and measured ranges so the estimate is debuggable.

## Relationship To Existing Lores

Existing lores should remain as a fallback while logical lores is introduced.

Current lores advantages:

- already implemented;
- very fast for small `lores_N`;
- already integrated with clip summaries and render preview.

Logical lores advantages:

- samples the actual full solve;
- can support arbitrary preview/sample resolutions;
- gives range estimates that should track full-render behavior better;
- can be reused for multiple preview sizes if a larger logical view is cached.

Logical lores and physical lores should coexist. Physical lores remains the
default small, fast debug/clip artifact. Logical lores adds a sampled-from-hires
view for larger and more consistent previews, plus future range-estimation work.

## Observed Lores Timing

Run `compute_moe6vfc4` provides a useful concrete datapoint:

- function: `poly_146`
- solver: `aberth_mt`
- full grid: `N=5000`, `times=1`
- degree: `70`, `n_coeffs=71`
- lores grid: `lores_N=60`, `3600` rows
- full artifacts: roots about `14.0GB`, coeffs about `14.2GB`, params about
  `0.4GB`
- lores artifacts: roots about `2.0MB`, coeffs about `2.0MB`, params about
  `0.1MB`

Observed UI log:

```text
Lores param gen complete · total=gen 0.3s · threads=1 · 3600 steps · 0.1MB
Lores coeffgen complete · total=gen 0.2s · size 2.0MB · threads=1 · degree=70
Lores solve (AE-MT) complete · total=solve 0.2s · bin 2.0MB · avg_iters=4.3
```

Summed stage time is roughly `0.7s`. Even allowing for orchestration, upload,
and log ordering, the physical `60x60` lores path is not a bottleneck for this
degree-70 run.

This is important for logical lores: replacing current fixed `60x60` lores with
a logical view is probably not a speed win by itself.

For `N=5000`, `view_N=60`, row-band logical materialization would read roughly:

```text
roots:  14.0GB * 60 / 5000 ~= 168MB
coeffs: 14.2GB * 60 / 5000 ~= 170MB
params:  0.4GB * 60 / 5000 ~=   5MB
total for slv+cf+pm ~= 343MB read from S3
```

The compact logical outputs would still only be about the same size as physical
lores: roughly `2MB` roots, `2MB` coeffs, and `0.1MB` params. The cost is not
CPU; it is S3 range reads and local decimation.

Estimated first-cut Python materializer cost for this run:

- roots only (`slv`): maybe around `1-3s` if row range reads are parallelized;
- roots + coeffs + params (`slv+cf+pm`): maybe around `2-6s` with parallel row
  reads;
- serial range reads would likely be worse and should not be used.

These numbers are estimates, not benchmarks. The conclusion is stable enough:
for the existing `60x60` lores, physical lores is faster and simpler. Logical
lores becomes interesting for different reasons:

- sampling the actual hires solve for score-range estimation;
- producing arbitrary preview/sample grids without recomputing solve artifacts;
- reusing cached logical views;
- larger logical views, where recomputing a fresh lores solve scales with
  `view_N^2` while row-band materialization scales mostly with `view_N`.

For the same `N=5000` degree-70 run, approximate row-band read volume for all
three families is:

```text
view_N=100  -> ~0.57GB read
view_N=200  -> ~1.14GB read
view_N=1000 -> ~5.72GB read
```

The physical recompute path scales with row count. Scaling the observed `60x60`
lores stage time naively by rows gives:

```text
100x100  -> ~2s
200x200  -> ~8s
1000x1000 -> ~190s
```

That linear extrapolation is rough, but it frames the likely break-even:
logical lores is probably not worth it for `60x60`, ambiguous around low
hundreds depending on S3 throughput and parallelism, and much more compelling for
large preview/sample grids.

## Caching Strategy

Logical views can be ephemeral at first.

For interactive use, if a larger logical view has already been materialized, a
smaller view can be derived from the compact logical artifact instead of
re-reading S3. This must still use the 2D logical row mapping; a flat stride is
not correct.

Example:

- materialize `400x400`;
- derive `200x200` and `100x100` by treating the `400x400` compact artifact as
  the source grid and applying the same proportional row/column mapping.

Persistent S3 caching can wait until the access pattern is clearer.

## Open Questions

- Exact non-divisible mapping policy: floor, nearest, or center-of-cell.
- Whether to expose logical preview resolution separately from current preview
  pixel size.
- Whether compact logical artifacts should be saved, cached temporarily, or kept
  strictly ephemeral.
- How aggressive score-range extrapolation should be for v1.
- Whether summary materialization should live directly in
  `handler_solve_proximity.py` or in a small shared helper imported by both
  summary and preview handlers.

## Recommended Next Steps

1. Extend the benchmark table: current physical lores timing and
   estimated/read logical materialization cost for several jobs
   across degree, `N`, solver, and requested source families.
2. Use Render / Output / Preview with `Logical` enabled to compare physical
   lores, logical `60x60`, and logical `100x100` on representative jobs.
3. Add more tests for exact divisible mapping, non-divisible mapping, serpentine
   order, and chunk byte-offset lookup.
4. Add runtime coverage for `cf` and `pm` logical preview paths with shared
   logical index mapping.
5. Run `solve_proximity_stats` summaries on compact logical views and log the
   two-resolution extrapolation fields.
6. Wire lag prelude handling for logical preview if visual mismatch on lagged
   programs becomes a real problem.
7. Compare current lores, logical `100x100`, logical `200x200`, and full render
   score-byte distributions on a few known jobs.
