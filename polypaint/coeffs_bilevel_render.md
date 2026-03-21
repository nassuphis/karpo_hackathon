# Coeffs Bilevel Render

This document describes a new render mode for plotting coefficient trajectories directly as bilevel images.

It is not a root render. It does not solve for roots. It renders the complex numbers already present in `coeffs_XXXX.bin` and `lores_coeffs.bin`.

## Goal

Add a new `Coeff` render mode that:

- always renders bilevel
- takes coefficient files as input
- computes viewport from `lores_coeffs.bin`
- projects coefficient values directly to pixels
- reuses the existing bilevel merge and stitch phases

The output should be a bilevel TIFF, same as the existing bilevel root pipeline.

## Why This Is Simpler Than Root Rendering

The current bilevel root pipeline is:

1. solve roots into `stripe_XXXX.bin`
2. raster each stripe into per-tile `.bits`
3. merge per tile
4. stitch final TIFF

For coefficient trajectories, step 1 is unnecessary because the coefficient files already contain complex values.

So the coefficient pipeline can be:

1. coeffgen writes `coeffs_XXXX.bin` and `lores_coeffs.bin`
2. compute viewport from `lores_coeffs.bin`
3. raster each coeff stripe into per-tile `.bits`
4. reuse existing bilevel merge
5. reuse existing bilevel stitch

This is structurally closer to the existing bilevel path than to the full compute path.

## Data Shape

`coeffs_XXXX.bin` contains complex coefficient values per parameter step.

Compared to roots:

- roots file: `degree` complex numbers per step
- coeffs file: `n_coeffs` complex numbers per step

The important point is that both are just packed complex samples in binary form.

For bilevel plotting, the raster stage only needs:

- how many complex numbers there are per step
- how to iterate them
- how to project them into image space

No root matching, root identity, or color logic is needed.

## Proposed Pipeline

### Phase 0: Viewport From `lores_coeffs.bin`

Add a coefficient viewport path parallel to the current root viewport path.

Input:

- `renders/{job_id}/lores_coeffs.bin`

Output:

- `center_re`
- `center_im`
- `scale_ref`
- `n_points`
- optional quantile-trimmed bounds

This can be implemented in either of two ways:

1. Extend the existing viewport Lambda with a mode switch like `source: "roots" | "coeffs"`.
2. Add a separate coeff viewport Lambda.

The simpler first implementation is probably extending the existing viewport Lambda.

The viewport math is the same as roots:

- interpret the file as complex points
- compute range / center / scale
- optionally apply quantile trimming

The only difference is record width:

- roots use `degree`
- coeffs use `n_coeffs`

### Phase 1: Raster Coeff Stripes

Add a new coeff bilevel raster phase:

- one Lambda per coeff stripe
- reads `coeffs_{stripe_idx:04d}.bin`
- projects all coefficient values into pixels
- emits per-tile `.bits`

Output shape should match the current bilevel merge contract:

- `renders/{job_id}/coeff_bits_s{stripe}_t{tile}.bits`

or reuse the same naming convention if the mode is fully isolated per job:

- `renders/{job_id}/bits_s{stripe}_t{tile}.bits`

The raster algorithm is the same as the current bilevel root raster:

1. read one complex point
2. transform into `(px, py)`
3. compute owning tile
4. compute tile-local bit index
5. dedup with tile bitset
6. set the bit

The only semantic difference is that the points come from coefficients, not roots.

### Phase 2: Merge

Reuse the existing bilevel merge phase unchanged.

The merge Lambda does not care whether the `.bits` files came from:

- roots
- coefficients

It only ORs tile-local bitsets into one tile TIFF.

### Phase 3: Stitch

Reuse the existing dedicated bilevel stitch Lambda unchanged.

It also does not care whether the tile TIFFs came from:

- root trajectories
- coefficient trajectories

It only joins tile TIFFs into the final bilevel TIFF.

## Recommended Implementation Strategy

There are two viable approaches.

### Option A: New Coeff Raster Binary

Add a new binary, for example:

- `coeffs_bilevel_raster.c`

This is the fastest path to implementation.

It can be a near-clone of `bilevel_raster.c`, with these differences:

- input file is `coeffs_XXXX.bin`
- use `n_coeffs` instead of `degree`
- output key naming reflects coeff mode if desired

Pros:

- low risk
- easy to reason about
- minimal impact on the existing root pipeline

Cons:

- duplicates most of `bilevel_raster.c`

### Option B: Generalize `bilevel_raster.c`

Refactor the current raster binary into a generic complex-points rasterizer.

For example:

- pass `--points_per_step=N`
- optionally pass `--input_kind=roots|coeffs`

Then use the same binary for:

- root bilevel raster
- coeff bilevel raster

Pros:

- less code duplication
- cleaner long-term architecture

Cons:

- touches working code
- more refactor risk

Recommendation:

- start with Option A
- refactor to Option B later only if both modes stabilize

## Frontend Changes

Add a new button in the Render tab:

- `Coeffs`

The frontend should dispatch a distinct backend path:

- `Render` -> color root pipeline
- `BiLevel` -> bilevel root pipeline
- `Param` -> bilevel coeff pipeline

The `Coeffs` mode should:

1. clean old coeff-render artifacts
2. compute viewport from `lores_coeffs.bin`
3. dispatch coeff raster stripes
4. poll coeff raster tasks
5. dispatch existing bilevel merge tiles
6. poll merge tasks
7. dispatch existing bilevel stitch
8. present final TIFF

This should be a separate function in `index.html`, parallel to `runBilevelPipeline()`.

## Preview

A coeff preview path is also straightforward.

Current preview logic uses low-resolution roots for:

- preview image
- viewport estimation

For coefficients, add:

- `coeffs_preview.png` or `coeffs_preview.tif`

This can be generated from `lores_coeffs.bin` using the same point-to-pixel logic as the full coeff raster path.

That gives:

- quick visual confirmation of coefficient trajectories
- viewport based on actual coeff data
- a coherent preview for the new mode

## Naming

Keep coeff render artifacts separate from root render artifacts.

Suggested keys:

- `renders/{job_id}/coeff_bits_s{stripe}_t{tile}.bits`
- `renders/{job_id}/coeff_t{tile}.tif`
- `renders/{job_id}/image_coeffs_bilevel.tif`
- `renders/{job_id}/coeffs_preview.png`

This avoids collisions with:

- root bilevel artifacts
- color artifacts

## Status Keys

Use distinct DynamoDB task prefixes so polling is unambiguous.

Suggested:

- `coeff_bilevel_raster_{stripe}`
- `coeff_bilevel_merge_{tile}`
- `coeff_bilevel_stitch`

Even if merge/stitch code is shared, separate task ids make the frontend simpler and prevent collisions with the root bilevel pipeline.

## Memory / Performance Expectations

This path should be cheaper than root bilevel rendering for the same stripe count because it removes solve entirely.

The main costs are:

- coeff viewport over `lores_coeffs.bin`
- coeff raster over `coeffs_XXXX.bin`
- existing merge
- existing stitch

The coeff raster memory profile should be similar to the current bilevel raster:

- one coeff stripe input
- one set of tile bitsets

The main variable is point count:

- coeffs have `n_coeffs` complex values per step
- roots have `degree` complex values per step

Since `n_coeffs = degree + 1`, coefficient rendering is slightly denser, but not qualitatively different.

## Open Questions

1. Should coeff viewport default to square/manual only, or support quantile-trimmed auto viewport like roots?
2. Should coeff artifacts reuse the generic `bits_s...` naming or use coeff-specific names?
3. Should coeff preview be TIFF for consistency or PNG for browser friendliness?
4. Should coeff raster get its own Lambda, or should the existing bilevel Lambda route `phase=coeff_raster`?

## Recommended First Cut

The simplest useful implementation is:

1. Add `Coeff` button in the Render tab.
2. Extend viewport Lambda to read `lores_coeffs.bin`.
3. Add `coeffs_bilevel_raster.c`.
4. Add coeff raster handling in a new or existing bilevel Lambda.
5. Reuse the current bilevel merge Lambda.
6. Reuse the current dedicated bilevel stitch Lambda.
7. Output `image_coeffs_bilevel.tif`.

This keeps the change focused:

- new viewport source
- new raster source
- existing back half of the pipeline unchanged

That is the right implementation order.
