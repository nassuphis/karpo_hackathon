# Logical Lores Design Notes

## Status

This document captures the current understanding of "logical lores": a low
resolution view derived from the full-resolution solve artifacts, rather than a
separate low-resolution compute run.

The goal is not to remove the current lores path immediately. The goal is to
define a practical path that can eventually replace or supplement it for:

- fast render previews at arbitrary view resolutions;
- better solve-score range estimation from samples of the real full solve;
- fewer redundant param/coeff/solve computations;
- consistent previews that are derived from the same artifacts as final render.

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

For small and medium previews this should be much faster than recomputing params,
coeffs, and solves. For `view_N` close to `full_N`, the row-band method
degenerates toward reading the full artifact; at that point the normal full path
or a more native strided reader is preferable.

## Proposed V1 Implementation

Start with a Python materializer, not native strided raster support.

Inputs:

- full solve source manifest;
- `full_N`;
- `view_N`;
- `times`;
- `degree`;
- `n_coeffs`;
- requested source families: `slv`, `cf`, `pm`.

Outputs:

- `/tmp/logical_roots_${view_N}.bin`
- `/tmp/logical_coeffs_${view_N}.bin`
- `/tmp/logical_params_${view_N}.bin`

The materializer should:

1. Build the deterministic logical-to-hires row map.
2. Range-fetch selected source rows from S3 using existing manifest metadata.
3. Decimate columns locally.
4. Write compact contiguous artifacts in logical order.
5. Return metadata describing `view_N`, `full_N`, selected rows, bytes read, and
   compact output sizes.

Then existing binaries can consume the compact files:

- `solve_proximity_stats` for summaries/range estimation;
- `roots2pix_mt` for preview/raster, once local-file input is supported or a
  small manifest mode can point at local files.

This keeps lagged score semantics correct because the compact file is already in
logical order.

## Rasterizer Implications

A simple `source_grid_n` argument to `roots2pix_mt` is not enough. The rasterizer
currently assumes a contiguous logical stream from `step_start` and `step_count`.

Direct native logical-lores support would need:

- a 2D row/column mapping from logical view rows to source rows;
- row-band or strided source reads;
- mixed-family alignment for `slv`, `cf`, and `pm`;
- lag prelude handling in logical-view order;
- section/thread boundary handling for lagged refs.

That is doable, but it is more risk than needed for the first version. The safer
v1 is materialize-then-reuse-existing-pipeline.

## Score Range Estimation

Logical lores enables a better range-estimation workflow than current lores:

1. Materialize a `100x100` logical view.
2. Compute score quantiles such as `q05/q95`.
3. Materialize a `200x200` logical view.
4. Recompute quantiles.
5. Use the observed change to extrapolate a more realistic full-grid range.

For lagged-current differences, the range often shrinks as density increases.
The extrapolation should be treated as a heuristic, not a guarantee.

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
- avoids redundant solve computation;
- can support arbitrary preview/sample resolutions;
- gives range estimates that should track full-render behavior better;
- can be reused for multiple preview sizes if a larger logical view is cached.

## Caching Strategy

Logical views can be ephemeral at first.

For interactive use, if a larger logical view has already been materialized, a
smaller view can be derived by decimating the compact logical artifact instead of
re-reading S3.

Example:

- materialize `400x400`;
- derive `200x200` and `100x100` locally.

Persistent S3 caching can wait until the access pattern is clearer.

## Open Questions

- Exact non-divisible mapping policy: floor, nearest, or center-of-cell.
- Whether logical lores should eventually replace current lores entirely.
- Whether to expose logical preview resolution separately from current preview
  pixel size.
- Whether compact logical artifacts should be saved, cached temporarily, or kept
  strictly ephemeral.
- Whether native `file://` or local-manifest support should be added first for
  `roots2pix_mt`.
- How aggressive score-range extrapolation should be for v1.

## Recommended Next Steps

1. Implement a small logical-view materializer for `slv` only.
2. Add tests for exact divisible mapping and serpentine order.
3. Extend the materializer to `cf` and `pm`.
4. Run `solve_proximity_stats` summaries on compact logical views.
5. Add local-file support for preview raster input.
6. Compare current lores, logical `100x100`, logical `200x200`, and full render
   score-byte distributions on a few known jobs.
