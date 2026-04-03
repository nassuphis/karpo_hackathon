# UI Regression Tests

Status: updated for the current family-catalog UI.

This document defines the UI regression coverage that should protect the current Render, Palette, and DeepZoom surfaces.

The old preview-button / fixed-row model is no longer current.

## Current UI Shape To Protect

### Render

The Render tab now has:

- family tabs:
  - `Color`
  - `BiLevel`
  - `Coeffs`
  - `Palette`
- per-family scrollable catalogs
- a selected-artifact viewer
- selection-scoped actions:
  - `Download`
  - `Delete`
  - `DeepZoom`
- family-scoped `Generate`

### Palette

The Palette tab now has:

- its own job selector and refresh
- full workflow generation through `palette_orchestrator`
- a selected-palette canvas/viewer
- a scrollable inventory for the selected job
- selection-scoped actions:
  - `Download`
  - `Delete`

### DeepZoom

The DeepZoom tab now has:

- server-side inventory loading through `/list-deepzoom`
- per-row `Share` links when `share_url` exists
- standalone viewer pages

## Main Regression Classes To Prevent

1. one family tab or catalog silently disappearing when another family is added
2. selected-artifact actions acting on the wrong item
3. refresh collapsing back to “latest only” behavior
4. palette runs completing without refreshing both Palette and Render inventories
5. share links existing in metadata but not rendering in the DeepZoom inventory

## Required Test Layers

### Fast JS Harness

Primary file:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

This suite must prove:

- Render family tabs render from `render-summary`
- family switching changes catalog and viewer
- empty families disable selection-scoped actions
- `Generate` dispatches the correct workflow for the active family
- async palette completion refreshes Palette inventory and Render `palette` family
- DeepZoom inventory uses `/list-deepzoom`
- share links render with `rel="noopener noreferrer"`

### Browser Tests

Primary files:

- [tests/e2e/render-refresh.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-refresh.spec.js)
- [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)
- [tests/e2e/deepzoom-inventory.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/deepzoom-inventory.spec.js)

These suites must verify:

- real DOM family tabs and counts
- selection highlight and action enablement
- solve-score controls in Render
- DeepZoom `Open` share link rendering

### Backend Contract Tests

Primary files:

- [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)
- [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)
- [tests/test_render_status.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_status.py)
- [tests/test_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_render_plan.py)
- [tests/test_palette_chunk_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_chunk_handler.py)
- [tests/test_palette_finalize_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_finalize_handler.py)

These suites must verify:

- immutable artifact listing
- delete routes
- palette workflow output contract
- render-summary schema
- deepzoom share-link metadata

## Minimum Render Coverage Matrix

### Family Coexistence

The tests must cover:

1. Color only
2. BiLevel only
3. Coeffs only
4. Palette only
5. all four families present at once

For each case, assert:

- correct family counts
- correct selected row
- correct viewer target
- correct button enablement

### Selection Semantics

For each family:

- clicking a row selects that exact artifact
- `Download` uses the selected artifact
- `Delete` deletes the selected artifact only
- `DeepZoom` uses the selected artifact only

### Refresh Semantics

Render refresh must prove:

- one frontend call to `/render-summary`
- no browser waterfall through `/head-keys`, `/list-prefix`, or `/presign`
- legacy root-level artifacts still appear as synthetic `legacy_*` items when present

## Palette Workflow Coverage

The tests must prove:

1. Palette generation is asynchronous
2. `_activePaletteRun` is set on dispatch
3. completion refreshes the Palette inventory
4. completion refreshes Render with:
   - `selectFamily: 'palette'`
   - the completed `palette_id`
5. newest palette becomes selected unless an explicit selection override is provided

## DeepZoom Share-Link Coverage

The tests must prove:

1. `viewer.html` is written before metadata is advertised
2. `meta.json` contains `share_url`
3. `deepzoom_latest.json` contains `share_url`
4. DeepZoom inventory renders an `Open` link when `share_url` exists
5. `Open` links use:
   - `target="_blank"`
   - `rel="noopener noreferrer"`

## Historical Note

The old Coeffs-preview disappearance bug is still relevant historically, but it should now be represented in tests as a family-catalog coexistence problem, not as a preview-button coexistence problem.

The current UI no longer has preview-mode buttons to protect.
