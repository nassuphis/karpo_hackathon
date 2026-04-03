# Rendering Artifact Lifetime Rules

Status: updated to the current immutable-artifact implementation.

This document defines the current lifetime rules for render-related artifacts.

## Core Rule

There are four independent user-facing artifact families:

1. `Color`
2. `BiLevel`
3. `Coeffs`
4. `Palette`

All four families are append-only by default.

Creating a new artifact:

- adds a new immutable artifact variant
- does not delete prior artifacts in that family
- does not delete artifacts in other families

Deleting an artifact is explicit and selection-scoped.

## Current Storage Ownership

### Color

Immutable outputs:

- `renders/{job_id}/color/{artifact_id}/image.jpeg` or `image.png`
- `renders/{job_id}/color/{artifact_id}/preview.png`

Intermediates owned by color cleanup:

- `pix_*`
- `raw_*`
- `tile_*`
- `solve_proximity/`
- `solve_scores/`
- `solve_proximity_clip.json`
- `solve_proximity_bins.json`

### BiLevel

Immutable outputs:

- `renders/{job_id}/bilevel/{artifact_id}/image.tif`
- `renders/{job_id}/bilevel/{artifact_id}/preview.png`

Intermediates owned by bilevel cleanup:

- `bilevel_t*`

### Coeffs

Immutable outputs:

- `renders/{job_id}/coeffs/{artifact_id}/image.tif`
- `renders/{job_id}/coeffs/{artifact_id}/preview.png`

Intermediates owned by coeff cleanup:

- `coeff_t*`

### Palette

Immutable outputs:

- `renders/{job_id}/palettes/{palette_id}/image.jpeg`
- `renders/{job_id}/palettes/{palette_id}/preview.png`
- `renders/{job_id}/palettes/{palette_id}/score_<metric>.bin`
- `renders/{job_id}/palettes/{palette_id}/palette_bins.bin`
- `renders/{job_id}/palettes/{palette_id}/meta.json`

Temporary workflow-only artifacts:

- `renders/{job_id}/palettes/{palette_id}/chunks/...`
- `renders/{job_id}/palettes/{palette_id}/solve_score/...`

These workflow temporaries are deleted by palette finalize after success.

## Cleanup Contract

`POST /clean-render` is now family-scoped intermediate cleanup only.

Current implementation:

- `pipeline=color`
  - deletes color intermediates
  - deletes DynamoDB status rows for the job
- `pipeline=bilevel`
  - deletes bilevel intermediates
  - deletes DynamoDB status rows for the job
- `pipeline=coeff_bilevel`
  - deletes coeff intermediates
  - deletes DynamoDB status rows for the job
- `pipeline=palette`
  - currently has no S3 intermediate prefixes to clear through `clean-render`

It does not delete immutable family artifacts.

It also does not delete previews or same-family siblings for immutable family variants, because the immutable family prefixes make stale sibling shadowing largely a legacy-only issue.

## Explicit Deletion

User-facing deletion is no longer tied to regeneration.

Current delete routes:

- `POST /delete-render-artifact`
  - deletes one selected immutable artifact variant in:
    - `color`
    - `bilevel`
    - `coeffs`
  - also handles synthetic legacy entries for those families
- `POST /delete-palette`
  - deletes one selected palette variant under:
    - `renders/{job_id}/palettes/{palette_id}/`

No route should delete an entire family because a new artifact was generated.

## Legacy Compatibility

Older top-level artifacts may still exist:

- `renders/{job_id}/image.jpeg`
- `renders/{job_id}/image.png`
- `renders/{job_id}/image_bilevel.tif`
- `renders/{job_id}/image_coeffs_bilevel.tif`
- `renders/{job_id}/image_palette.jpeg`
- old preview keys such as `preview_color.png`, `preview_bilevel.png`, `preview_coeffs.png`, `preview_palette.png`

`render-summary` surfaces these as synthetic `legacy_*` artifacts so they remain visible and explicitly deletable.

They are compatibility artifacts, not the current canonical storage layout.

## UI Contract

The Render tab no longer shows four fixed rows.

It now shows four family catalogs.
Lifetime behavior must therefore satisfy:

- family catalogs persist across new generations
- generating one family does not remove entries from another family catalog
- deleting acts on the selected row only

## Acceptance Criteria

The current artifact-lifetime model is correct only if all of these remain true:

1. generating `Color` preserves all saved `BiLevel`, `Coeffs`, and `Palette` artifacts
2. generating `BiLevel` preserves all saved `Color`, `Coeffs`, and `Palette` artifacts
3. generating `Coeffs` preserves all saved `Color`, `BiLevel`, and `Palette` artifacts
4. generating `Palette` preserves all saved `Color`, `BiLevel`, and `Coeffs` artifacts
5. `clean-render` deletes only family-owned intermediates
6. deletion is explicit and selection-scoped
7. legacy top-level artifacts remain discoverable until explicitly deleted

## Related Files

- [render_refactor.md](/Users/nicknassuphis/karpo_hackathon/polypaint/render_refactor.md)
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
