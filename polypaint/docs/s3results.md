# S3 Results Storage

Current storage overview for compute, render, palette, preview, and DeepZoom artifacts.

## Bucket & Table

- S3 bucket: `polypaint`
- DynamoDB status table: `polypaint-jobs`

## Top-Level Layout

Most job-scoped data lives under:

- `renders/{job_id}/`

DeepZoom exports live under:

- `deepzoom/{job_id}/`

## Compute Artifacts

### Coefficients

- `renders/{job_id}/coeffs_{chunk:04d}.bin`
- `renders/{job_id}/lores_coeffs.bin`

### Roots

- `renders/{job_id}/chunk_{chunk_idx}.bin`
- `renders/{job_id}/lores.bin`

The compute side is chunk-based.
Some older docs and compatibility code still mention `stripe_*`, but current root outputs are chunk-named.

### Metadata

- `renders/{job_id}/calc.json`
- `renders/{job_id}/view.json`

Current `calc.json` is chunk-oriented and commonly contains:

- `N`
- `degree`
- `times`
- `n_chunks`
- `coeffs_keys`
- `lores`
- `chunks`

## Render Intermediates

### Color render

- `renders/{job_id}/color/{artifact_id}/fragments/section_{section:04d}.frag`
- `renders/{job_id}/color/{artifact_id}/greyscale.raw`
- `renders/{job_id}/solve_scores/...`
- `renders/{job_id}/solve_proximity/...`

### BiLevel render

- `renders/{job_id}/bilevel_section_{section:04d}.frag`

### Coeffs bilevel render

- `renders/{job_id}/coeff_bilevel_section_{section:04d}.frag`

These are workflow intermediates, not user-facing artifacts.

## Immutable Render Artifact Families

### Color

- `renders/{job_id}/color/{artifact_id}/image.jpeg`
- `renders/{job_id}/color/{artifact_id}/image.png`
- `renders/{job_id}/color/{artifact_id}/preview.png`

### BiLevel

- `renders/{job_id}/bilevel/{artifact_id}/image.tif`
- `renders/{job_id}/bilevel/{artifact_id}/preview.png`

### Coeffs

- `renders/{job_id}/coeffs/{artifact_id}/image.tif`
- `renders/{job_id}/coeffs/{artifact_id}/preview.png`

### Palette

- `renders/{job_id}/palettes/{palette_id}/image.jpeg`
- `renders/{job_id}/palettes/{palette_id}/preview.png`
- `renders/{job_id}/palettes/{palette_id}/score_<metric>.bin`
- `renders/{job_id}/palettes/{palette_id}/palette_bins.bin`
- `renders/{job_id}/palettes/{palette_id}/meta.json`

## Legacy Render Artifacts

Older top-level artifacts may still exist directly under `renders/{job_id}/`, for example:

- `image.jpeg`
- `image.png`
- `image_bilevel.tif`
- `image_coeffs_bilevel.tif`
- `image_palette.jpeg`
- old preview keys

The storage layer still discovers these and surfaces them as synthetic `legacy_*` variants for compatibility.

## Preview Artifacts

Root preview uses:

- `renders/{job_id}/preview.png`
- `renders/{job_id}/preview_stats.json`

These are separate from immutable Render-family previews.

## DeepZoom

DeepZoom exports write:

- `deepzoom/{job_id}/{export_id}/image.dzi`
- `deepzoom/{job_id}/{export_id}/image_files/...`
- `deepzoom/{job_id}/{export_id}/viewer.html`
- `deepzoom/{job_id}/{export_id}/meta.json`

Job-scoped pointer:

- `renders/{job_id}/deepzoom_latest.json`

## Discovery APIs

Current important storage routes:

- `/list`
  - compute results table
- `/detail`
  - selected result details
- `/render-summary`
  - Render family inventories for one job
- `/list-palettes`
  - saved palette variants for one job
- `/list-deepzoom`
  - DeepZoom export inventory

## Lifecycle Notes

- compute outputs are reusable job data
- render family artifacts are immutable and append-only
- palette artifacts are immutable and append-only
- deletion is explicit per selected artifact
- `clean-render` now clears family-scoped intermediates, not immutable outputs
