# Palette Reuse

Status: proposed.

This document describes the agreed refactor to make saved Palette artifacts reusable in two ways:

1. `RePalette`
   - input: an existing Palette artifact
   - output: a new Palette artifact
   - behavior: reuse the existing bin assignment, apply a different palette, generate a new `image.jpeg` / `preview.png`, preserve the underlying numeric data

2. `GenerateFromPalette`
   - input: an existing Palette artifact
   - output: a new `Color` render artifact
   - behavior: reuse the existing per-solve bin assignment to color the roots directly, without recomputing solve-score clip / hist / merge or per-solve score math

The key design constraint is:

- the visible `Palette` image is pass `0` only
- the real `Color` render colors every solve across all passes

So if a palette artifact is going to be reusable for `Color`, it must persist all-pass per-solve bin data, not only pass `0`.

## Current State

Current implemented Palette workflow:

- [palette-render.md](/Users/nicknassuphis/karpo_hackathon/polypaint/palette-render.md)
- [lambda/handler_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_render_plan.py)
- [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
- [lambda/handler_palette_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_finalize.py)

Current semantics:

- full solve is used for clip / hist / merge
- chunk workers compute exact pass-0 scores and bins only
- finalize assembles:
  - `score_<metric>.bin`
  - `palette_bins.bin`
  - `image.jpeg`
  - `preview.png`
  - `meta.json`
- the persisted numeric sidecars are pass `0` only

That is enough to display Palette artifacts, but not enough to reproduce `Color` renders when `times > 1`, because `Color` is per solve, not per `(i,j)` cell.

## Agreed Semantics

### Palette Image

The Palette image remains:

- dense `N x N`
- one pixel per base parameter cell
- pass `0` only

This is the human-facing summary image.

### Reusable Numeric Data

For reuse in `GenerateFromPalette`, palette artifacts must persist:

- one score per solve, all passes
- one bin per solve, all passes

This data must stay chunk-local and solve-order-preserving.

### No False Reuse

Pass-0-only palette data must never be treated as sufficient for `GenerateFromPalette`.

That would fail as soon as:

- `times > 1`
- or dithered passes map the same `(i,j)` to different bins

## Storage Layout

### New Palette Artifact Layout

For new reusable palette artifacts, the durable layout becomes:

- `renders/{job_id}/palettes/{palette_id}/image.jpeg`
- `renders/{job_id}/palettes/{palette_id}/preview.png`
- `renders/{job_id}/palettes/{palette_id}/meta.json`
- `renders/{job_id}/palettes/{palette_id}/chunks/score_chunk_{idx:04d}.bin`
- `renders/{job_id}/palettes/{palette_id}/chunks/palette_bins_chunk_{idx:04d}.bin`
- `renders/{job_id}/palettes/{palette_id}/chunks/meta_chunk_{idx:04d}.json`

Note:

- there is intentionally no new monolithic all-pass `score_<metric>.bin`
- there is intentionally no new monolithic all-pass `palette_bins.bin`

Reason:

- that would duplicate data already present in the chunk-local all-pass files
- and it scales badly for large `N * N * times`

### Temporary Finalize Files

Finalize may still assemble temporary pass-0 buffers in `/tmp`:

- pass-0 score buffer for debugging if needed
- pass-0 bin buffer for image generation

But those pass-0 buffers are not persisted as separate durable artifacts.

### Compatibility

Old palette artifacts remain valid for viewing.

Compatibility rules:

- old artifacts with `palette_bins.bin` and no chunk-local all-pass data:
  - `RePalette`: allowed
  - `GenerateFromPalette`: disabled
- new artifacts with chunk-local all-pass data:
  - `RePalette`: allowed
  - `GenerateFromPalette`: allowed

## Metadata Contract

### New Required Fields In `meta.json`

Add these fields to palette artifact metadata:

- `data_layout`: `"chunk_all_pass_v1"`
- `render_reusable`: `true`
- `image_pass`: `0`
- `base_grid_solves`: `N * N`
- `total_solves`: `N * N * times`
- `pass_count`: `times`
- `chunk_scores_prefix`
- `chunk_bins_prefix`
- `chunk_meta_prefix`

Retain existing fields:

- `job_id`
- `palette_id`
- `created_at`
- `display_name`
- `metric`
- `palette`
- `solve_score_quantile`
- `solve_score_omega`
- `root_transforms`
- `degree`
- `N`
- `times`
- `clip_lo`
- `clip_hi`
- `cuts_norm`
- `clip_fallback`
- `clip_fallback_reason`
- `image_key`
- `preview_key`
- `file_size`

### New Optional Provenance Fields

For derived palette artifacts:

- `derived_from_palette_id`
- `derived_from_palette_created_at`
- `derived_from_palette_name`
- `derivation_kind`

Use:

- `derivation_kind = "repalette"`

For color artifacts produced from a palette artifact:

- `palette_source_id`
- `palette_source_created_at`
- `palette_source_display_name`
- `palette_source_metric`
- `palette_source_quantile`
- `palette_source_omega`

## Data Ordering Contract

The palette reuse design depends on the solve ordering being explicit.

Codify this contract:

- all chunk-local palette bins and scores are in exactly the same solve order as the corresponding root chunk
- chunk files remain one-to-one with render root chunks by `chunk_idx`
- within a chunk:
  - item `off` in `palette_bins_chunk_{idx}.bin`
  - corresponds to item `off` in the root chunk file

For pass-aware image assembly:

- `base_grid_solves = N * N`
- `pass_idx = global_solve_idx // base_grid_solves`
- `base_idx = global_solve_idx % base_grid_solves`
- pass `0` means `pass_idx == 0`

For the final dense Palette image, `base_idx` maps to row/col using the existing serpentine contract already used by [lambda/handler_palette_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_finalize.py).

## Palette Workflow Changes

### Plan

Update:

- [lambda/handler_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_render_plan.py)

Changes:

- keep existing chunk planning
- no UI-visible semantic change
- output prefixes should explicitly separate durable chunk data from temporary solve-score intermediates

Required outputs:

- `outputs.image_key`
- `outputs.preview_key`
- `outputs.meta_key`
- `outputs.chunk_scores_prefix`
- `outputs.chunk_bins_prefix`
- `outputs.chunk_meta_prefix`
- temporary:
  - `outputs.solve_score_prefix`
  - `outputs.solve_score_clip_key`
  - `outputs.solve_score_bins_key`

### Chunk Worker

Update:

- [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
- [lambda/solve_palette_chunk.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_chunk.c)

Required semantic change:

- compute scores and bins for the full chunk span, all passes
- do not truncate to pass `0`

Durable uploads per chunk:

- `chunks/score_chunk_{idx}.bin`
- `chunks/palette_bins_chunk_{idx}.bin`
- `chunks/meta_chunk_{idx}.json`

Chunk meta must record:

- `chunk_idx`
- `step_start`
- `step_count`
- `score_key`
- `palette_bins_key`
- `metric`
- `clip_lo`
- `clip_hi`
- `cuts_norm`
- `omega`
- `min_score`
- `max_score`

### Finalize

Update:

- [lambda/handler_palette_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_finalize.py)

Finalize must change from:

- persist pass-0 numeric sidecars

to:

- persist all-pass chunk-local numeric outputs
- generate the human-facing image from pass `0` only

Algorithm:

1. list durable chunk meta under `chunk_meta_prefix`
2. allocate a temporary pass-0 bin buffer of size `N * N`
3. iterate chunk metadata in `step_start` order
4. for each solve in each chunk:
   - `g = step_start + off`
   - if `g >= N * N`, skip it for image assembly
   - otherwise map to pass-0 dense image index via serpentine deshuffle
   - write that pass-0 bin into the temporary pass-0 image buffer
5. call [lambda/palette_bins_render](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_bins_render) on the temporary pass-0 bin buffer
6. generate `image.jpeg` and `preview.png`
7. write new `meta.json`
8. delete only temporary solve-score workflow scratch
9. do not delete durable chunk bins / scores / chunk meta

### `palette_bins_render`

No semantic change required in:

- [lambda/palette_bins_render.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_bins_render.c)

It still only needs:

- a pass-0 dense bin buffer
- a palette name

## `RePalette`

### User Flow

In the Palette family tab:

- select a saved palette artifact
- click `RePalette`
- popup opens
- popup shows the palette picker UI
- user chooses a new palette
- buttons:
  - `Exit`
  - `Execute`
- `Execute` creates a new palette artifact

### Semantics

`RePalette` must:

- not recompute solve scores
- not recompute solve-score bins
- reuse the existing bin assignment
- change only the final palette used to map bins to RGB

### New Artifact

The new palette artifact:

- gets a new `palette_id`
- gets a new `created_at`
- gets a new `palette`
- gets new `image.jpeg`
- gets new `preview.png`
- copies or reuses the existing numeric data according to the v1 strategy below

### V1 Storage Strategy

For v1, use the simple strategy agreed in discussion:

- copy the durable chunk-local score/bin/meta files under the new artifact prefix

Yes, this duplicates data. That is acceptable for v1 because:

- it keeps every palette artifact self-contained
- it avoids cross-artifact references
- delete semantics stay simple

So `RePalette` will:

1. read source `meta.json`
2. validate source belongs to the selected `job_id`
3. copy:
   - `chunks/score_chunk_*.bin`
   - `chunks/palette_bins_chunk_*.bin`
   - `chunks/meta_chunk_*.json`
4. render new pass-0 image from the copied or source chunk bin data
5. write new `meta.json`

### Legacy Compatibility

If the source artifact is legacy pass-0-only:

- `RePalette` is still allowed
- source `palette_bins.bin` may be used directly to render the new image
- the new artifact may remain legacy-style and non-reusable for `GenerateFromPalette`

This should be explicit in `meta.json`:

- `render_reusable = false`

### Backend Shape

Implement as a dedicated async Lambda:

- `lambda/handler_repalette.py`

Reason:

- no heavy numerical recomputation
- mostly S3 copy + image/preview regeneration + metadata write
- simpler than a new Step Functions workflow

Expected inputs:

- `job_id`
- `source_palette_id`
- `new_palette`
- `task_id`

Required runtime dependencies:

- [lambda/palette_bins_render](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_bins_render)
- [lambda/raw2jpeg](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/raw2jpeg)
- libvips layer for preview generation

### UI Placement

In the Palette family toolbar:

- `Generate`
- `RePalette`
- `Populate`

`RePalette` is enabled only when a palette artifact is selected.

### Ordering In The Palette Catalog

Derived repalette artifacts should appear directly above their source artifact, just like autolevel children appear above their source in the Color family.

That requires:

- `derived_from_palette_id`
- palette-family ordering logic in [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)

## `GenerateFromPalette`

### User Flow

In the Color family tab:

- click `GenerateFromPalette`
- popup shows reusable saved palette artifacts for the current result set
- user selects one
- buttons:
  - `Exit`
  - `Execute`
- `Execute` creates a new Color artifact

### Semantics

`GenerateFromPalette` must:

- not recompute solve-score clip
- not recompute solve-score hist
- not recompute solve-score merge
- not recompute per-solve solve-score metric in raster
- instead reuse persisted all-pass per-solve bins from the selected palette artifact

It still does:

- root download
- root transforms
- rotation
- viewport projection
- spatial bucketing
- match mode
- color encoding

### What Comes From The Selected Palette Artifact

Must come from the selected palette artifact, not from the current solve-score form fields:

- `palette`
- `metric`
- `solve_score_quantile`
- `solve_score_omega`
- `root_transforms`
- chunk-local bin files

The current Color form still controls:

- view / viewport
- pix
- format
- quality
- rotation
- match mode

### New Color Mode

Add a distinct internal mode:

- `color_mode = "saved_palette"`

Reason:

- provenance is clear in metadata
- handlers can branch cleanly
- UI can display that this artifact came from a saved palette, not a fresh solve-score calculation

### Render Plan Changes

Update:

- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)

Required behavior:

1. if `color_mode == "saved_palette"`:
   - load selected palette artifact metadata
   - validate:
     - same `job_id`
     - `render_reusable == true`
     - `data_layout == "chunk_all_pass_v1"`
     - `N`, `times`, `degree` compatible with current compute
2. inject into the plan:
   - `palette_source_id`
   - `palette_source_display_name`
   - `palette_source_palette`
   - `palette_source_chunk_bins_prefix`
   - `palette_source_root_transforms`
3. force plan root transforms to the source palette artifact’s `root_transforms`
4. disable solve-score prepass in the plan

### Raster Changes

Update:

- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)

Required behavior:

When `color_mode == "saved_palette"`:

1. raster downloads:
   - root chunk
   - matching palette bin chunk
2. raster passes the bin chunk file path into `roots2pix`
3. `roots2pix` reads one bin byte per solve
4. for solve offset `off`, use `bin[off]` directly
5. map that bin to RGB using the selected palette name
6. skip `compute_solve_metric_score()`
7. skip solve-score normalization and cut lookup

### New `roots2pix` CLI Surface

Extend `roots2pix` with a new path such as:

- `--color=saved_palette`
- `--solve_bins_file=/tmp/palette_bins_chunk.bin`
- `--palette=<palette_name>`

Validation:

- bin file length must equal the number of solves in the root chunk
- bin values must be in `0..9`

### Render Metadata

Color artifact metadata should record:

- `color_mode = "saved_palette"`
- `palette = <source palette name>`
- `palette_source_id`
- `palette_source_display_name`
- `palette_source_metric`
- `palette_source_quantile`
- `palette_source_omega`
- `root_transforms` copied from the selected palette artifact

### UI Placement

In the Color family toolbar:

- `Generate`
- `GenerateFromPalette`
- `Populate`

`GenerateFromPalette` is enabled only when at least one reusable palette artifact exists for the selected result set.

## Storage Handler Changes

Update:

- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)

Required changes:

1. palette listing must expose:
   - `render_reusable`
   - `data_layout`
   - `chunk_scores_prefix`
   - `chunk_bins_prefix`
   - `derived_from_palette_id`
2. palette ordering must support derived children above their source
3. delete must delete full copied `chunks/` data for repalette artifacts
4. legacy palette artifacts must still list cleanly

## Frontend Changes

Update:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

### Palette Family

Add:

- `RePalette` button

Popup behavior:

- same visual language as existing palette selector UI
- shows `PAL`, `TRI`, `LONG`
- selected new palette shown in popup
- `Exit`
- `Execute`

On success:

- refresh Palette inventory
- auto-select the new palette artifact

### Color Family

Add:

- `GenerateFromPalette` button

Popup behavior:

- scrollable table of reusable palette artifacts for the current job
- columns:
  - created
  - metric
  - q
  - omega
  - palette
  - preview strip if useful
- `Exit`
- `Execute`

On success:

- refresh Color family catalog
- auto-select the new Color artifact

### Compatibility UI

If the selected palette artifact is legacy pass-0-only:

- `RePalette`: enabled
- `GenerateFromPalette`: disabled
- message:
  - `This palette artifact has image-only pass-0 bins and cannot drive Color reuse. Regenerate the palette to make it reusable.`

## Deployment

### New / Updated Bundles

Update [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh) for:

- new `handler_repalette.py`
- packaging `palette_bins_render`
- packaging `raw2jpeg`
- libvips layer attachment for preview generation
- any new `roots2pix` CLI support in the color raster bundle

### No Migration Script Required

No destructive migration is required.

Old artifacts remain:

- viewable
- downloadable
- repalette-able

But only new artifacts with `render_reusable = true` participate in `GenerateFromPalette`.

## File-Level Implementation Checklist

### Backend

- [lambda/handler_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_render_plan.py)
  - output durable chunk prefixes
- [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
  - write all-pass chunk data
- [lambda/solve_palette_chunk.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_chunk.c)
  - stop truncating to pass `0`
- [lambda/handler_palette_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_finalize.py)
  - assemble image from pass `0` only
  - stop persisting monolithic pass-0 sidecars
  - preserve durable chunk data
- [lambda/handler_repalette.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_repalette.py)
  - new
- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
  - add `saved_palette` mode
- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
  - download matching palette bin chunk
- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
  - consume precomputed bin bytes
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
  - expose reuse metadata
  - derived ordering
  - legacy compatibility

### Workflow / Routing

- [stepfunctions/palette_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/palette_workflow.asl.json.template)
  - no structural change required beyond output key fields if the current shape is retained
- [stepfunctions/render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)
  - support `saved_palette` mode by passing palette source chunk bin key into raster
- dispatch wiring in the existing API / function environment for `handler_repalette.py`

### Frontend

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
  - `RePalette` popup
  - `GenerateFromPalette` popup
  - enable/disable rules
  - observer refresh
  - auto-selection of new artifacts

## Testing

### Palette Workflow

Add / update:

- [tests/test_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_render_plan.py)
  - plan outputs new durable chunk prefixes
- [tests/test_palette_chunk_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_chunk_handler.py)
  - chunk outputs cover full `step_count`, not pass `0` only
- [tests/test_palette_finalize_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_finalize_handler.py)
  - finalize assembles pass-0 image while leaving durable chunk data intact
  - no persisted monolithic sidecars for new artifacts

### RePalette

Add:

- `tests/test_repalette_handler.py`

Cases:

- new palette artifact copies chunk data and writes new image/preview/meta
- legacy source artifact repalettes successfully without becoming render-reusable
- metadata provenance fields are correct

### GenerateFromPalette

Add:

- [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)
  - `saved_palette` mode validation
- `tests/test_raster_saved_palette.py`
  - raster downloads chunk bin file
  - bypasses solve-score bins JSON
- `roots2pix` compile / behavior test
  - bin file length mismatch fails
  - valid bin file colors roots without solve-score computation

### Storage / UI

Add:

- [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)
  - palette list exposes `render_reusable`
  - palette derived ordering
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
  - RePalette popup behavior
  - GenerateFromPalette popup behavior
  - legacy palette disables GenerateFromPalette
  - successful runs refresh the correct catalogs and auto-select the new rows

### Packaging

Update:

- [tests/test_deploy_packaging.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deploy_packaging.py)

Must catch:

- `handler_repalette.py` bundle includes required binaries
- render raster bundle still includes updated `roots2pix`

## Non-Goals For This Change

- no cross-artifact dedup layer for chunk bins/scores
- no new palette-side direct DeepZoom path
- no attempt to backfill old palette artifacts into reusable all-pass form

Those can come later if the reuse features prove valuable.
