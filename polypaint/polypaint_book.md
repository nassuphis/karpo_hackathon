# PolyPaint Book Generation Plan

Status: proposed.

## Goal

Adapt `make_book.py` (WhiteWall coffee table book generator) to work with polypaint render artifacts downloaded via the app's Image+Meta download.

The current `make_book.py` is a generic book tool from the repo root. The polypaint version should:

1. Auto-discover image+meta pairs in `snaps/`
2. Build text pages from the `_meta.json` compute/render metadata instead of manual config entries
3. Use the same WhiteWall-compliant PDF dimensions and layout

## Current State

### Snaps directory convention

The app's Download menu saves pairs:
- `{job}_{artifact_id}.jpeg` — the render image
- `{job}_{artifact_id}_meta.json` — structured metadata

Example:
- `compute_mnk2ptxy_color_run_1775291797683_56tfl2.jpeg`
- `compute_mnk2ptxy_color_run_1775291797683_56tfl2_meta.json`

### Meta.json structure

Each `_meta.json` contains:
- `compute.function` — e.g. `poly_150`
- `compute.param_transforms` — e.g. `unit_circle, coeff7`
- `compute.coeff_transforms` — e.g. `rev`
- `compute.degree` — e.g. `70`
- `compute.N` — e.g. `1000`
- `compute.times` — e.g. `1`
- `compute.solver` — e.g. `aberth`
- `color_mode` — e.g. `solve_score`
- `palette` — e.g. `tri_spectral_autumn`
- `solve_metric` — e.g. `crowding`
- `solve_score_quantile` — e.g. `0.05`
- `solve_score_omega` — e.g. `1`
- `pix` — e.g. `5000`
- `rotation` — radians
- `artifact_id` — e.g. `color_run_1775291797683_56tfl2`
- `job_id` — e.g. `compute_mnk2ptxy`
- `view_mode`, `square_extent`, `quantile`, `shim`

### What the text page should show

Same layout as the PDF spread (`spread_pdf.py`):

1. **Title**: `"Title"` (literal placeholder)
2. **Pipeline line**: `[unit_circle, coeff7] poly_150 [rev] N=1000, times=1`
3. **Color mode line**: `SOLVE SCORE: crowding q=5.0% w=1 tri_spectral_autumn`
4. **Degree line**: `Degree: 70`
5. **Artifact ID** (grey, Courier): `color_run_1775291797683_56tfl2`

## Changes

### 1. Rename

- `make_book.py` → `make_polypaint_book.py`

### 2. New config file

- `polypaint_book_config.json`

Minimal shape:
```json
{
  "title": "PolyPaint",
  "subtitle": "Polynomial Root Visualizations",
  "description": "",
  "author": "",
  "snaps_dir": "snaps",
  "cover_image": null,
  "back_image": null
}
```

No `pages` array — pages are auto-discovered from `snaps/`.

### 3. Auto-discovery

Scan `snaps_dir` for `*.jpeg` files that have a matching `*_meta.json`. Sort by filename (which includes timestamp, so chronological). Each pair becomes one spread.

### 4. Text page generation

Replace the manual `title`/`text` fields with structured metadata read from `_meta.json`. Use the same `_build_spread_meta`-style logic from `handler_pdf_artifact.py` / `spread_pdf.py`, but running locally from the JSON file.

### 5. Book layout

Keep the existing WhiteWall layout exactly:
- Page 1: blank recto
- Each spread: verso (text from meta) + recto (image)
- Pad to multiple of 4
- Cover: front image + spine + back panel

### 6. Cover image

If `cover_image` is not set in config, use the first snap image.

## Files

### New
- `make_polypaint_book.py` — adapted book generator
- `polypaint_book_config.json` — minimal config

### Deleted
- `make_book.py` — renamed to `make_polypaint_book.py`

### Unchanged
- `spread_pdf.py` — shared spread builder (Lambda + local)
- `make_spread.py` — single-spread preview tool
