# Render Refactor

Status: implemented.

This document describes the current Render-tab model after the immutable-artifact refactor.

## Current Model

The Render tab is no longer a fixed four-row panel.

It is now a family workspace with four tabs:

1. `Color`
2. `BiLevel`
3. `Coeffs`
4. `Palette`

Each family tab has:

- a `Generate` action
- a scrollable catalog of saved artifacts for that family
- a selected-artifact viewer
- selection-scoped actions:
  - `Download`
  - `Delete`
  - `DeepZoom`

Arrow up/down moves selection inside the active family catalog.

## Artifact Model

Render artifacts are immutable variants.

Current storage layout:

- Color:
  - `renders/{job_id}/color/{artifact_id}/image.jpeg`
  - `renders/{job_id}/color/{artifact_id}/image.png`
  - `renders/{job_id}/color/{artifact_id}/preview.png`
- BiLevel:
  - `renders/{job_id}/bilevel/{artifact_id}/image.tif`
  - `renders/{job_id}/bilevel/{artifact_id}/preview.png`
- Coeffs:
  - `renders/{job_id}/coeffs/{artifact_id}/image.tif`
  - `renders/{job_id}/coeffs/{artifact_id}/preview.png`
- Palette:
  - `renders/{job_id}/palettes/{palette_id}/image.jpeg`
  - `renders/{job_id}/palettes/{palette_id}/preview.png`
  - `renders/{job_id}/palettes/{palette_id}/score_<metric>.bin`
  - `renders/{job_id}/palettes/{palette_id}/palette_bins.bin`
  - `renders/{job_id}/palettes/{palette_id}/meta.json`

Color, BiLevel, and Coeffs store artifact metadata in S3 object metadata on the image object.
Palette stores artifact metadata in `meta.json`.

## Refresh Contract

Render refresh is driven by one frontend call:

- `POST /render-summary`

The response shape is `schema_version: 2` and contains:

- `calc`
- `families`
- `deepzoom_latest`

`families` contains four arrays:

- `families.color`
- `families.bilevel`
- `families.coeffs`
- `families.palette`

For backward compatibility, `render-summary` still surfaces old top-level artifacts as synthetic legacy entries:

- `legacy_color`
- `legacy_bilevel`
- `legacy_coeffs`
- `legacy_palette`

The UI shows these legacy artifacts in the same family catalogs until they are explicitly deleted.

## Generate Semantics

`Generate` acts on the active family only.

- `Color` starts the render workflow in color mode
- `BiLevel` starts the render workflow in bilevel mode
- `Coeffs` starts the render workflow in coeff-bilevel mode
- `Palette` starts the asynchronous palette workflow

Generating a new artifact:

- appends a new immutable artifact variant
- does not overwrite prior artifacts in that family
- does not delete other families

## Delete Semantics

Deletion is explicit and selection-scoped.

- Color, BiLevel, and Coeffs use:
  - `POST /delete-render-artifact`
- Palette uses:
  - `POST /delete-palette`

Deleting one selected artifact must not affect any sibling artifact in the same family or any other family.

## DeepZoom Semantics

DeepZoom export is selection-scoped.

The `DeepZoom` button acts on the currently selected artifact only.
It is no longer attached to a fixed family row.

The latest DeepZoom pointer still appears in `render-summary` as `deepzoom_latest`, but the full DeepZoom inventory lives in the DeepZoom tab.

## Family-Specific Notes

### Color

Color artifacts may be JPEG or PNG.
Solve-score metadata is stored on the image object when relevant:

- `color_mode`
- `palette`
- `solve_metric`
- `solve_score_quantile`

### BiLevel

BiLevel immutable artifacts use `image.tif` + `preview.png` inside the artifact prefix.
Older top-level TIFF/preview outputs are still discoverable as `legacy_bilevel`.

### Coeffs

Coeffs immutable artifacts use `image.tif` + `preview.png` inside the artifact prefix.
Older top-level TIFF/preview outputs are still discoverable as `legacy_coeffs`.

### Palette

Palette artifacts are full workflow outputs, not just debug images.
Both the Palette tab and the Render tab `Palette` family dispatch the asynchronous palette workflow.

## What Was Removed

The following older UI model is no longer current:

- four fixed artifact rows
- one current artifact per family
- preview-mode buttons for `Color`, `Bilevel`, `Coeffs`, `Palette`
- a shared preview area switching among one current preview per family

## Related Files

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
- [lambda/handler_render_status.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_status.py)
- [stepfunctions/render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)

## Current Open Follow-On Work

This refactor did not decide how Color render should reuse saved Palette artifacts for solve-score rendering.
That remains a separate design problem, tracked in [palette-lookup.md](/Users/nicknassuphis/karpo_hackathon/polypaint/palette-lookup.md).
