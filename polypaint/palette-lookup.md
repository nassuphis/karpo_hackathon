# Palette Lookup For Solve Score

Status: deferred design note.

The exact full-resolution Palette workflow now exists, but Color render does not yet reuse those saved palette bins directly.

This document records the next possible refactor after [palette-render.md](/Users/nicknassuphis/karpo_hackathon/polypaint/palette-render.md).

## Current State

Implemented today:

- the Palette workflow produces exact pass-0 full-resolution artifacts:
  - `score_<metric>.bin`
  - `palette_bins.bin`
  - `image.jpeg`
- the Palette tab and Render `Palette` family both generate those immutable palette artifacts

Not implemented today:

- Color render does not look up per-sample bins from saved `palette_bins.bin`
- `roots2pix` still computes solve-score color from solve-score metadata (`clip_lo`, `clip_hi`, `cuts_norm`) during rasterization

So the system already has an exact palette artifact family, but not yet a shared lookup-driven Color render.

## Desired Direction

The longer-term goal is still attractive:

1. compute one authoritative per-sample solve-score bin assignment
2. derive the parameter-space palette image from it
3. derive the root-space color render from it

That would make:

- the palette image
- the root image

two views of the same authoritative color assignment.

## Source Of Truth

If this refactor happens, the source of truth must be:

- `palette_bins.bin`

not:

- `image_palette.jpeg`

JPEG remains a human-facing artifact only.

## Why This Is Deferred

Even after the exact Palette workflow landed, the harder part remains:

- render workers need a reliable mapping from a solve sample to its authoritative palette-bin entry
- the current render path is chunk-based and still computes solve-score color locally inside `roots2pix`

So the exact palette artifacts are now available, but the render-side lookup contract is still a separate change.

## Likely Future Shape

If implemented later, the next step should be:

1. add a Render color mode that selects a saved palette artifact
2. freeze Color render to that palette’s:
   - `metric`
   - `palette`
   - `solve_score_quantile`
   - `root_transforms`
   - `clip_lo`
   - `clip_hi`
   - `cuts_norm`
3. later, if desired, move from frozen-metadata reuse to true `palette_bins.bin` lookup during rasterization

That gives a clean staged path:

- first: reuse saved palette metadata
- later: reuse saved palette bin files directly

## Non-Negotiable Rules

If this refactor is revived later:

- do not use JPEG as machine-readable input
- do not invent a second solve-score binning algorithm
- do not silently change `times` semantics
- do not make Color and Palette disagree for the same saved palette definition
