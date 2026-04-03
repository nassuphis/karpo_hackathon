# Color RePalette From Persisted Pixel-Bin Maps

## Goal

Add a **fast** `RePalette` action for `Render -> Color` that recolors an existing
solve-score Color artifact **without**:

- recomputing solve scores
- recomputing solve bins
- rereading root chunks during recolor
- rerunning root-to-pixel rasterization

This is **not** the same feature as `GenerateFromPalette`.

- `GenerateFromPalette` reuses **per-solve bins** and rerenders roots.
- `Color RePalette` should reuse the **already rasterized visible winner per pixel**
  and only change the final palette lookup.

## Why The Previous Extract/Reuse Design Is Wrong For This Goal

The old idea in the previous version of this document was:

- persist per-solve bins on Color artifacts
- derive a Palette artifact from Color
- use `GenerateFromPalette`

That works for reuse, but it does **not** solve the real performance goal.

The expensive step for “recolor this exact Color image quickly” is not only
solve-to-bin. It is also:

- root projection into pixels
- per-pixel conflict resolution
- final visible winner selection

If we rerender roots, we still pay the raster cost.

For fast Color RePalette, the correct reusable artifact is:

- **one byte per output pixel**
- `0..9` = winning palette-bin index at that pixel
- `255` = no root / background

Then recolor is trivial:

1. load the pixel-bin map
2. map `0..9` through a new palette LUT
3. map `255` to background
4. write new image + preview

No roots involved.

## Current Pipeline Audit

### 1. Where solve-score bins exist today

In [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c):

- `COLOR_SOLVE_SCORE` computes one bin per solve
- `COLOR_SAVED_PALETTE` reads one saved bin per solve

That happens around:

- solve-score path: [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c#L645)
- saved-palette path: [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c#L697)

### 2. What raster does today

In [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py):

- one worker downloads one root chunk
- runs `roots2pix`
- uploads sparse per-tile `.pix` files:
  - `renders/{job}/pix_chunk_{chunk}_t{tile}.pix`

These `.pix` files are **intermediate sparse RGB entries**, not final tiles.

### 3. Where final visible winners are chosen

The final visible tile image is produced in:

- [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)
- [lambda/pixassemble.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/pixassemble.c)

Important current behavior:

- `roots2pix` does **chunk-local first-hit dedup**
- `pixassemble` does **last-wins overwrite across chunk streams**

This is explicit in [lambda/pixassemble.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/pixassemble.c#L5):

- “Uses `last wins` overwrite”

And in [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py#L53):

- chunk files are streamed into `pixassemble` in increasing `chunk_idx` order

So the final visible pixel winner is:

- first winner **within a chunk**
- highest streamed chunk winner **across chunks**

Any reusable Color RePalette artifact must capture **that final visible result**,
not only per-solve bins.

### 4. What a Color artifact persists today

Current Color artifacts persist:

- `image.jpeg` / `image.png`
- `preview.png`
- metadata

They do **not** persist:

- per-solve scores
- per-solve bins
- final per-pixel bin identity

## Design Decision

Persist **final per-pixel palette-bin maps** for eligible Color artifacts.

Do **not** persist per-solve bins for this feature.

Do **not** route Color RePalette through Palette artifacts.

The new contract is:

- normal solve-score Color render still builds the image exactly as today
- while doing so, it also persists a tile-aligned final pixel-bin sidecar
- a new `Color RePalette` workflow recolors from those tile binmaps only

## Eligibility

Only these Color artifacts are repalette-capable:

- `color_mode = solve_score`
- `color_mode = saved_palette`

Not eligible:

- `rainbow`
- `proximity`
- `constant`
- any postprocessed derivative such as `autolevels`

Reason:

- the reusable artifact is a 10-bin palette index per pixel
- those modes either do not use 10 bins or alter pixels after colorization

## Persisted Artifact Contract

### New per-tile sidecars

For eligible Color artifacts, persist:

- `renders/{job_id}/color/{artifact_id}/pixel_bins/tile_{tile_idx}.bin`

Format:

- headerless `uint8`
- exactly `tile_w * tile_h` bytes
- row-major pixel order
- values:
  - `0..9` = winning palette-bin index
  - `255` = empty/background pixel

This is intentionally **pixel-space**, not solve-space.

### New Color metadata fields

Persist on the image object metadata and expose in `render-summary`:

- `repalette_capable = true|false`
- `pixel_bins_prefix = renders/{job_id}/color/{artifact_id}/pixel_bins/tile_`
- `pixel_bins_empty = 255`
- `pixel_bins_layout = tile_u8_v1`
- `background_color = 000000` (already persisted today for Color)

For ineligible artifacts:

- `repalette_capable = false`
- other fields empty

## Backend Implementation

### 1. [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)

Add optional sparse bin-output support for palette-based color modes.

New CLI flag:

- `--pixel_bin_prefix=/tmp/pixbin`

Behavior:

- only honored for:
  - `COLOR_SOLVE_SCORE`
  - `COLOR_SAVED_PALETTE`
- ignored or rejected for other color modes

New temp outputs:

- `/tmp/pixbin_t0000.pbx`
- `/tmp/pixbin_t0001.pbx`
- etc.

Recommended sparse entry format:

- 8 bytes per entry:
  - `local_pixel_idx:uint32`
  - `bin:uint32`

Why use a separate sparse stream instead of changing `.pix`:

- keeps existing `.pix` RGB format untouched
- avoids breaking `pixassemble`
- keeps the feature isolated to solve-score-like modes

Implementation detail:

- in `COLOR_SOLVE_SCORE`, when a pixel is accepted, emit both:
  - RGB sparse entry via existing `emit_pixel(...)`
  - bin sparse entry via new `emit_pixel_bin(...)`
- in `COLOR_SAVED_PALETTE`, do the same using the loaded bin

Do **not** emit sparse bin entries for rejected/deduped pixels.

### 2. [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)

Add optional outputs for repalette-capable renders.

New plan input:

- `pixel_bin_prefix`

Behavior:

- when present and color mode is `solve_score` or `saved_palette`:
  - pass `--pixel_bin_prefix=/tmp/pixbin` to `roots2pix`
  - upload each non-empty sparse bin file to:
    - `renders/{job}/pixbin_chunk_{chunk}_t{tile}.pbx`

Keep this as an intermediate workflow artifact, analogous to `pix_chunk_*`.

Cleanup:

- include `/tmp/pixbin_t*.pbx`

### 3. New binary: [lambda/pixbinassemble.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/pixbinassemble.c)

Purpose:

- merge sparse per-chunk pixel-bin entries into a final tile-aligned `uint8` binmap

Input:

- stdin stream of packed entries:
  - `local_pixel_idx:uint32`
  - `bin:uint32`

Arguments:

- `--tile_w=...`
- `--tile_h=...`
- `--output=/tmp/tile.bin`
- `--empty=255`

Behavior:

- initialize tile buffer to `255`
- for each entry:
  - `tile[pix_idx] = (uint8_t)bin`
- overwrite on every later entry

This must mirror [lambda/pixassemble.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/pixassemble.c):

- same chunk order
- same last-wins semantics

That is what guarantees exact reproduction of the visible image.

### 4. [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)

Extend finalization for eligible Color runs.

New inputs:

- `repalette_capable`
- `pixel_bins_out_key`

Behavior:

1. existing behavior remains:
   - merge sparse RGB `.pix` files into `tile_{tile}.raw`
2. new behavior for repalette-capable runs:
   - stream sparse `pixbin_chunk_{chunk}_t{tile}.pbx` files into `pixbinassemble`
   - upload resulting tile binmap to:
     - `renders/{job_id}/color/{artifact_id}/pixel_bins/tile_{tile_idx}.bin`

Important:

- this must happen in the same tile-finalize pass
- do **not** try to derive the final binmap from the final JPEG/PNG
- do **not** derive it earlier in raster; earlier data does not know the final cross-chunk winner

Cleanup:

- intermediate `renders/{job}/pixbin_chunk_*` should remain workflow scratch only

### 5. [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)

For `mode == color`:

- if:
  - `color_mode == solve_score` or
  - `color_mode == saved_palette`
  - and `postprocess_kind` is empty

then set:

- `outputs.metadata.repalette_capable = "true"`
- `outputs.metadata.pixel_bins_prefix = artifact_prefix + "pixel_bins/tile_"`
- `outputs.metadata.pixel_bins_empty = "255"`
- `outputs.metadata.pixel_bins_layout = "tile_u8_v1"`

otherwise:

- `repalette_capable = "false"`
- other fields empty

Also extend the per-tile finalize plan items with:

- `pixel_bins_out_key = artifact_prefix + "pixel_bins/tile_{tile_idx}.bin"`

### 6. [stepfunctions/render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)

Two changes:

1. Raster map items

Pass:

- `pixel_bin_prefix` or equivalent enable flag so raster uploads sparse `pixbin_chunk_*`

2. Finalize map items

Pass:

- `repalette_capable.$ = $.plan.outputs.repalette_capable`
- `pixel_bins_out_key.$ = States.ArrayGetItem($.plan.grid.pixel_bin_tile_keys, $$.Map.Item.Value.tile_idx)`

Add to plan generation:

- `grid.pixel_bin_tile_keys = ["...tile_0000.bin", "...tile_0001.bin", ...]`

### 7. [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)

Expose new Color summary fields:

- `repalette_capable`
- `pixel_bins_prefix`
- `pixel_bins_empty`
- `pixel_bins_layout`

These should appear in `render-summary` entries for Color artifacts.

This drives:

- button enable/disable in the Render UI
- provenance display if desired later

### 8. New binary: [lambda/pixel_bins_render.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/pixel_bins_render.c)

Purpose:

- recolor a final tile binmap into an RGB tile raw image

Input:

- `tile.bin` from the persisted Color artifact

Arguments:

- `--tile_w=...`
- `--tile_h=...`
- `--palette=<name>`
- `--background_color=<rrggbb>`
- `--empty=255`
- output raw path

Behavior:

- values `0..9`: map through the chosen palette LUT using the same 10-bin logic as solve-score coloring
- value `255`: emit background color
- write `raw2jpeg`-compatible 12-byte header + RGB pixels

This binary is the fast recolor engine.

### 9. New Lambda: [lambda/handler_color_repalette_tile.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_color_repalette_tile.py)

One tile per invocation.

Input:

- `job_id`
- `task_id`
- `source_pixel_bins_key`
- `tile_w`
- `tile_h`
- `palette`
- `background_color`
- `out_key`

Behavior:

1. download the persisted tile binmap
2. run `pixel_bins_render`
3. upload the resulting tile raw to the new artifact prefix

This Lambda is cheap compared to normal raster.

### 10. New workflow plan/orchestrator

Add:

- [lambda/handler_color_repalette_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_color_repalette_plan.py)
- [lambda/handler_color_repalette_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_color_repalette_orchestrator.py)
- [stepfunctions/color_repalette_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/color_repalette_workflow.asl.json.template)

#### Plan lambda responsibilities

Input:

- `job_id`
- `source_artifact_id`
- `new_palette`

Validation:

- source family = `color`
- `repalette_capable = true`
- `color_mode in {solve_score, saved_palette}`
- no postprocess artifact
- source tile binmaps exist

Plan output:

- new immutable Color artifact id/prefix
- copy most source render metadata unchanged:
  - `view_mode`
  - `quantile`
  - `shim`
  - `square_extent`
  - `rotation`
  - `match_mode`
  - `root_transforms`
  - `pix`
  - `format`
  - `quality`
  - `solve_metric`
  - `solve_score_quantile`
  - `solve_score_omega`
- set:
  - `palette = new_palette`
  - `derived_from_artifact_id = source_artifact_id`
  - `derivation_kind = color_repalette`
  - `repalette_capable = true`
  - `pixel_bins_prefix` copied to new artifact only if you want recolor chains to remain possible

Recommendation:

- copy the tile binmaps forward to the new artifact prefix
- this allows repeated fast recolors without referring back to the original artifact

#### Workflow shape

1. `Plan`
2. `Tile recolor phase`
3. `Tile recolor Map`
4. `Encode`
5. `Preview`
6. `Done`

This is much lighter than the full render workflow:

- no raster map
- no finalize map
- no solve-score prepass

### 11. [lambda/handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py)

Add new dispatch target:

- `color_repalette`

Payload:

- `job_id`
- `task_id`
- `source_artifact_id`
- `new_palette`

### 12. [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)

Add build/package/deploy wiring for:

- `pixbinassemble`
- `pixel_bins_render`
- `handler_color_repalette_plan.py`
- `handler_color_repalette_orchestrator.py`
- `handler_color_repalette_tile.py`
- new Step Functions definition

Also package:

- `palette_names.py`
- `tri_palette_names_generated.py`
- `long_palette_names_generated.py`

for the new recolor plan worker if it validates palettes.

## Frontend Implementation

### 1. [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Add `RePalette` button in `Render -> Color`.

Show only when:

- active family = `color`
- selected artifact exists
- `art.repalette_capable === true`
- `art.postprocess_kind` is empty

Popup behavior:

- same visual pattern as palette-family `RePalette`
- show `PAL`, `TRI`, `LONG` buttons
- seed from selected Color artifact’s `palette`
- summary should say something like:
  - `Source: color_...`
  - `mode=solve_score`
  - `q=...`
  - `w=...`
  - `current palette=...`
  - `fast recolor from persisted pixel bins`
- buttons:
  - `Exit`
  - `Execute`

Dispatch:

- target = `color_repalette`
- send:
  - `job_id`
  - `source_artifact_id`
  - `new_palette`

Completion behavior:

- same as other render-family actions
- refresh Render Color inventory
- select the new artifact
- place it directly above the source artifact if ordering prefers newest-derived-first

### 2. Render catalog ordering

In the Color family ordering logic in [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py):

- derived recolors should sort directly above their source when possible

This is the same UX goal as Autolevels:

- click source
- repalette
- new variant appears adjacent for comparison

## Exact Semantics

### What Color RePalette guarantees

It guarantees:

- exactly the same rasterized visible structure
- exactly the same winner per pixel
- only the final 10-bin color lookup changes

### What it does not guarantee

It does **not**:

- change view
- change rotation
- change root transforms
- change solve metric
- change `q`
- change `omega`
- change conflict resolution
- change dither/pass behavior

Those are all frozen by the persisted pixel-bin map.

## Storage Tradeoff

For one rendered image of size `pix x pix`, the persisted binmap costs:

- `pix * pix` bytes total across all tiles

Examples:

- `2000 x 2000` -> about `4 MB`
- `4000 x 4000` -> about `16 MB`
- `8000 x 8000` -> about `64 MB`

This is much smaller than:

- persisting per-solve bins for high-`times` jobs
- and much more useful for fast recolor

## Compatibility

Old Color artifacts:

- have no pixel-bin maps
- are not repalette-capable

UI behavior:

- `RePalette` button disabled or hidden
- tooltip/status should say:
  - `This Color artifact predates persisted pixel bins and cannot be recolored directly. Regenerate it to enable fast RePalette.`

## Tests To Add

### 1. Native tests

Add:

- [tests/test_pixbinassemble.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pixbinassemble.py)
- [tests/test_pixel_bins_render.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pixel_bins_render.py)

Coverage:

- overwrite semantics in `pixbinassemble` match `pixassemble`
- empty pixels remain `255`
- palette LUT mapping is correct
- background color path is correct

### 2. Raster/finalize tests

Add:

- [tests/test_raster_pixel_bins.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_pixel_bins.py)
- [tests/test_finalize_pixel_bins.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_finalize_pixel_bins.py)

Coverage:

- solve-score raster emits sparse bin files
- saved-palette raster emits sparse bin files
- finalize uploads tile binmaps only for eligible runs
- finalize binmap uses last-wins semantics matching the visible RGB output

### 3. Plan/storage tests

Add:

- [tests/test_color_repalette_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_color_repalette_plan.py)
- extend [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)
- extend [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)

Coverage:

- eligible Color artifacts get repalette metadata
- ineligible ones do not
- render-summary exposes the new fields
- color repalette plan preserves source settings and changes only `palette`

### 4. Frontend tests

Extend [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh):

- `RePalette` button appears for eligible Color artifacts
- hidden/disabled for ineligible/legacy/autolevel artifacts
- popup shows `PAL` / `TRI` / `LONG`
- selected palette is seeded from the source artifact
- dispatch target is `color_repalette`
- completion refreshes Color inventory and selects the new artifact

Optional e2e:

- [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)

### 5. Deploy packaging tests

Extend [tests/test_deploy_packaging.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deploy_packaging.py):

- `handler_finalize.py` bundle includes `pixbinassemble`
- `handler_color_repalette_tile.py` bundle includes `pixel_bins_render`
- new recolor plan/orchestrator bundles include required Python helpers

## What Not To Do

Do not:

- derive recolor from JPEG/PNG pixels
- store a lossy recolor source
- reuse per-solve bins for this feature
- implement Color RePalette by rerunning full raster
- build the pixel-bin artifact before final tile merge

All of those miss the actual goal.

## Summary

For fast solve-score Color RePalette, the correct reusable data is:

- **final pixel-space winner bins**

not:

- solve-space bins

The exact implementation is:

1. emit sparse per-pixel bin entries during solve-score/saved-palette raster
2. merge them at tile finalize with the same last-wins semantics as RGB tile assembly
3. persist the final tile-aligned `uint8` binmaps on eligible Color artifacts
4. add a lightweight recolor workflow that maps those binmaps through a new palette LUT

That gives you:

- exact same image geometry and visible structure
- very fast recolor
- no solve/bin/raster recomputation
