# Enhance GenerateFromPalette

## Goal

Extend `GenerateFromPalette` so it can:

1. reuse the persisted solve-bin assignments from a selected `Palette` artifact
2. but render with a different output palette than the one stored in that artifact

In this document, the selected output palette is called the **colorvector**.

So the new mental model is:

- selected palette artifact provides the **bin assignment**
- popup palette selectors provide the **colorvector**

The resulting Color render uses:

- exact same per-solve bins
- different palette lookup if the user chooses one

## What this means mathematically

Current `GenerateFromPalette` already avoids recomputing solve scores.

It uses:

- one persisted bin `0..9` per solve
- then looks that bin up through a 10-color LUT derived from the palette name

So if the persisted bins are unchanged, the user can choose any other output palette and still get a valid render:

- same `bin(p)` for every solve `p`
- different `RGB = LUT_new_palette[ bin(p) ]`

This is exactly the same bin reuse, only with a different final color lookup.

## Current Codepath Audit

### Frontend

Popup lives in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

- state:
  - `_generateFromPalettePopupState`
  - `_visibleGenerateFromPaletteCatalog()`
- UI render:
  - `_renderGenerateFromPalettePopup()`
- open:
  - `openGenerateFromPalettePopup()`
- execute:
  - `runGenerateFromPaletteSelected()`

Today the popup only:

- filters/selects reusable palette artifacts
- does **not** expose any palette selector UI

### Backend

Current dispatch path:

- `runGenerateFromPaletteSelected()` calls `_launchRenderOrchestrator('color', { color_mode: 'saved_palette', saved_palette_id: ... })`

Current plan behavior in [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py):

- when `color_mode == "saved_palette"`:
  - load source palette artifact metadata
  - validate reusable layout
  - force:
    - `palette = source_meta["palette"]`
    - `solve_metric = source_meta["metric"]`
    - `solve_score_quantile = source_meta["solve_score_quantile"]`
    - `solve_score_omega = source_meta["solve_score_omega"]`
    - `root_transforms = source_meta["root_transforms"]`

This is the one behavior that must change:

- keep metric / q / omega / root transforms from source palette artifact
- but do **not** forcibly reuse source artifact’s palette if the user selected a different output colorvector

### Raster/runtime

In [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py):

- saved-palette mode downloads `saved_palette_bins_key`
- passes `--color=saved_palette`
- passes `--palette=<current render palette>`

In [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c):

- `COLOR_SAVED_PALETTE` reads one bin per solve from `--solve_bins_file`
- then maps that bin through the currently selected palette LUT

This already supports the desired behavior.

No raster algorithm change is required.

## Desired UI

### Popup layout

`GenerateFromPalette` popup should have two sections:

1. artifact selection table
2. output colorvector selector row

The colorvector selector row should reuse the same selector language as Render:

- `PAL`
- `TRI`
- `LONG`

Use `PAL`, not `VAL`.

### Initial state

When the user highlights a saved palette artifact:

- the popup should seed the colorvector from that artifact’s stored palette
- if the artifact palette is built-in:
  - `PAL` active
- if `tri_*`:
  - `TRI` active with correct remembered tri name
- if `long_*`:
  - `LONG` active with correct remembered long name

The popup summary should explicitly separate:

- `bin source`
- `colorvector`

Example:

- `Selected bins: crowding q=5.0% w=4 from pal_...`
- `Colorvector: tri_redgold`

### User interactions

Inside the popup, the user can:

- pick the source palette artifact row
- then optionally change the colorvector using:
  - `PAL`
  - `TRI`
  - `LONG`

The popup should reuse the existing palette selector popups:

- built-in popup
- TRI popup
- LONG popup

These must be opened in a mode specific to `GenerateFromPalette`, not by piggybacking on `solve_score` or `repalette`.

## Frontend Implementation

### 1. Add popup-local palette mode state

In [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html), add a new palette-row mode:

- `generate_from_palette`

It needs parallel state like existing modes:

- current selected palette id
- remembered built-in palette
- remembered TRI palette name
- remembered LONG palette name

Add support in:

- `_paletteContainerId(mode)`
- `_modeUsesBuiltinPopup(mode)`
- `_builtinPaletteForMode(mode)`
- `_setRememberedBuiltinPalette(mode, ...)`
- `_triPaletteForMode(mode)`
- `_longPaletteForMode(mode)`
- `_setRememberedTriPalette(mode, ...)`
- `_setRememberedLongPalette(mode, ...)`
- `_currentPaletteForMode(mode)`
- `_popupModeLabel(mode)`

`_popupModeLabel('generate_from_palette')` should return:

- `GenerateFromPalette`

### 2. Add container in popup markup

In the popup markup block in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

add a palette selector area, e.g.

- `#palette-circles-generate-from-palette`

Place it below the artifact summary and above the `Exit` / `Execute` buttons.

### 3. Render the colorvector row

In `_renderGenerateFromPalettePopup()`:

1. determine selected source artifact
2. seed popup palette mode from the source artifact’s stored palette
   - only when opening or when switching highlighted source artifact
   - do not clobber user’s manual selection after they change the colorvector
3. render `_renderPaletteRow('generate_from_palette')`

Add helper flag in popup state:

- `paletteSeededFromSourceId`

Behavior:

- if highlighted artifact id differs from `paletteSeededFromSourceId`, reseed from source artifact palette
- otherwise keep current user-edited colorvector

### 4. Execute behavior

In `runGenerateFromPaletteSelected()`:

current payload is:

- `color_mode = saved_palette`
- `saved_palette_id = <selected palette artifact id>`

Add:

- `palette = _currentPaletteForMode('generate_from_palette')`

This is the chosen output colorvector.

Important:

- keep `saved_palette_id`
- do **not** change source artifact metric/q/omega here
- only provide chosen output palette override

### 5. Popup summary text

Update popup summary to say something like:

- `Selected bins: ${display_name}`
- `Metric=${metric}, q=${q}, w=${omega}`
- `Colorvector=${_currentPaletteForMode('generate_from_palette')}`

This makes it visually obvious that source bins and output palette are separate.

## Backend Implementation

### 1. [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)

Revise the `color_mode == "saved_palette"` block.

Current logic wrongly does:

- `palette = source_meta.get("palette", palette)`
- `rp["palette"] = palette`

New rule:

#### Source palette artifact controls:

- `solve_metric`
- `solve_score_quantile`
- `solve_score_omega`
- `root_transforms`
- `chunk_bins_prefix`
- source display/provenance fields

#### User-selected output colorvector controls:

- `palette`

Implementation:

1. Read requested palette from incoming params first:
   - `requested_palette = rp.get("palette", "inferno")`
2. Validate source artifact as today
3. Validate requested palette is in `VALID_PALETTE_NAMES`
4. Keep:
   - `solve_metric = source_meta["metric"]`
   - `solve_score_quantile = source_meta["solve_score_quantile"]`
   - `solve_score_omega = source_meta["solve_score_omega"]`
   - `root_transforms = source_meta["root_transforms"]`
5. Set:
   - `palette = requested_palette`
   - `rp["palette"] = requested_palette`

Do not overwrite the chosen palette from the source artifact anymore.

### 2. Saved-palette provenance metadata

For Color outputs produced from `GenerateFromPalette`, keep existing source metadata and add one more distinction:

- `palette_source_palette = <source artifact palette>`
- `palette = <actual output colorvector>`

This matters because after enhancement they may differ.

Suggested metadata additions in Color artifact image metadata:

- `palette_source_id`
- `palette_source_display_name`
- `palette_source_metric`
- `palette_source_quantile`
- `palette_source_omega`
- `palette_source_palette`
- `palette_source_family = palette`

Current code already writes most of these, except `palette_source_palette`.

### 3. [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)

Expose:

- `palette_source_palette`

for Color artifact summary/detail.

This lets the UI later explain:

- source bins came from palette artifact `X`
- rendered with palette `Y`

### 4. Raster/runtime

No changes required in:

- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)

Reason:

- saved-bin render path already uses:
  - source bins from file
  - current palette from `--palette`

That is exactly the desired behavior.

## Artifact Semantics After Change

After enhancement:

### Source Palette artifact

Defines:

- solve-bin assignment
- source metric/q/omega/root transforms
- original palette artifact image

### Generated Color artifact

Defines:

- same source bin assignment
- same metric/q/omega/root transforms
- possibly different output colorvector

So two Color artifacts can share:

- identical bins

but differ in:

- `palette`

That is intentional.

## Tests

### Frontend

Extend [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh):

Add:

1. `GenerateFromPalette` popup renders `PAL`, `TRI`, `LONG`
2. source artifact with built-in palette seeds `PAL`
3. source artifact with `tri_*` seeds `TRI`
4. source artifact with `long_*` seeds `LONG`
5. changing popup palette selector changes dispatched `params.palette`
6. dispatched payload keeps `saved_palette_id`
7. dispatched payload uses chosen output colorvector, not necessarily source artifact palette

### Browser / E2E

Extend [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js):

Add a test that:

1. opens `GenerateFromPalette`
2. selects a source palette artifact
3. opens `PAL`/`TRI`/`LONG` inside the popup
4. chooses a different output colorvector
5. asserts popup summary updates

If dispatch is stubbed, also assert:

- `saved_palette_id` equals source palette artifact id
- `palette` equals chosen output palette

### Render plan

Extend [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py):

Cover:

1. `saved_palette` source still overrides metric/q/omega/root transforms from source artifact
2. requested output `palette` survives and is not overwritten by source artifact palette
3. invalid requested palette still fails validation

### Storage / summary

Extend [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py):

Cover:

- Color render-summary exposes both:
  - `palette`
  - `palette_source_palette`

## Implementation Order

1. add popup-local palette mode `generate_from_palette` in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
2. add selector row to popup UI
3. dispatch chosen output palette in `runGenerateFromPaletteSelected()`
4. change `handler_render_plan.py` saved-palette behavior so source artifact no longer overwrites requested output palette
5. expose `palette_source_palette` in stored metadata / render-summary
6. add tests

## Explicit Non-Goals

Not part of this enhancement:

- changing the persisted bin data itself
- recomputing solve scores
- changing `RePalette`
- adding score-sidecar persistence for Color artifacts
