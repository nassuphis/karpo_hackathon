# Autolevels Plan

## Goal

Add an `Autolevels` button to the `Color` family inside the `Render` tab.

When the user selects a saved color artifact and presses `Autolevels`:

1. a new immutable color artifact is created from the selected image
2. the new artifact is an autoleveled post-processed version of that image
3. the new artifact appears immediately above the source artifact in the Color catalog
4. the new artifact inherits the original render settings, so existing `Populate` behavior keeps working
5. the new artifact can be compared with the source via click and arrow-key navigation
6. `Download`, `Delete`, and `DeepZoom` continue to work with no special-case UI

This is a render post-process, not a new render mode and not a new Step Functions workflow.


## Product Behavior

### Where It Appears

- Only in `Render -> Color`
- Only when a saved color artifact row is selected
- Not shown in `BiLevel`, `Coeffs`, or `Palette`

### Button Placement

In the Color-family action row:

- `Generate`
- `Populate`
- `Autolevels`
- `Download`
- `Delete`
- `DeepZoom`

### What It Does

`Autolevels` opens a popup seeded with default parameters derived from the Preview-like pipeline described in [autolevels.py](/Users/nicknassuphis/karpo_hackathon/polypaint/autolevels.py).

Nothing is created until the user explicitly clicks `AutoLevel` inside that popup.

If the user presses `Esc` or clicks outside the popup:

- the popup closes
- no artifact is created

### Catalog Position

The new artifact must appear immediately above the clicked source artifact, not merely “newest first at the top”.

That behavior must survive refresh, reload, and later visits.

This means ordering must be part of the render-summary model, not just a one-time client splice.

### Scope of Deletion

Deleting the autoleveled artifact deletes only that derived artifact prefix:

- `renders/{job_id}/color/{artifact_id}/...`

It must not delete the source artifact or any result/compute assets.


## Recommended Architecture

## Shape

Use a single async Lambda, dispatched through the existing `dispatch` Lambda.

Do **not** use Step Functions for this feature.

Reason:

- this is one-image post-processing, not tiled fanout
- it already fits the existing immutable artifact model
- existing DDB task polling is enough
- lower implementation cost, fewer moving parts

### New Lambda

Add:

- [lambda/handler_autolevels.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_autolevels.py)

This handler will:

1. read the selected source image from S3
2. invoke a native autolevel binary
3. write the derived full image
4. write a preview PNG
5. write inherited + derived metadata onto the final image object
6. report DDB status updates so the existing Render observer can poll it

The handler should be thin. The image processing should live in a C program linked against libvips, matching the rest of this repo’s deployment model.

### Dispatch Target

Extend [lambda/handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py):

- add target: `autolevels`
- wire env var: `AUTOLEVELS_FUNCTION`

Frontend should dispatch one async job, exactly like current one-shot background jobs.


## Why This Belongs In Color Family

The output is still a normal image artifact. It should live in the same immutable color artifact family:

- source family: `color`
- derived family: `color`

That keeps these existing features working automatically:

- render catalog browsing
- `Populate`
- `Download`
- `Delete`
- `DeepZoom`

No new family tab is needed.


## Source Algorithm

Use [autolevels.py](/Users/nicknassuphis/karpo_hackathon/polypaint/autolevels.py) as the behavioral reference only.

It is not the deployment target.

The production implementation should be:

- a native C binary using libvips
- packaged exactly like other existing image binaries in this repo
- called by `handler_autolevels.py` via `subprocess.run(...)`

### Recommended Initial Preset

For v1, ship one fixed preset named:

- `preview_default_v1`

Use the current defaults from `AutoLevelsRGBConfig`:

- `bins=256`
- `clip_low=0.0`
- `clip_high=1.0`
- `peak_factor=0.0`
- `gamma=1.0`
- `auto_gamma="none"`
- `target=0.5`
- `sigmoid_strength=0.0`
- `sigmoid_mid=0.5`
- `vibrance=0.0`
- `pooled_rgb=None`
- `jpeg_subsample_mode="on"`
- `jpeg_optimize_coding=False`
- `jpeg_interlace=False`

This keeps the first feature simple and reproducible while still exposing the parameters in the popup.

The popup should show these values prefilled. The user may edit them before pressing `AutoLevel`.

For v1, no preset selector is required. One visible parameter form is enough.


## Artifact Model

### Output Prefix

Write derived artifacts under the normal color-family prefix:

- `renders/{job_id}/color/{artifact_id}/image.jpeg` or `.png`
- `renders/{job_id}/color/{artifact_id}/preview.png`

Where `artifact_id` is a new immutable id, for example:

- `autolevels_1775..._<shorthash>`

### Metadata

The derived artifact must preserve the source render metadata so `Populate` works without special cases.

Metadata should include all existing color metadata fields that current Color artifacts already carry, plus these new fields:

- `derived_from_artifact_id`
- `derived_from_image_key`
- `postprocess_kind = "autolevels"`
- `postprocess_profile = "preview_default_v1"`
- `source_family = "color"`
- `quality`

The inherited metadata set should include at least:

- `artifact_id`
- `family=color`
- `created_at`
- `degree`
- `pix`
- `format`
- `quality`
- `tile_size`
- `view_mode`
- `quantile`
- `shim`
- `square_extent`
- `rotation`
- `root_transforms`
- `color_mode`
- `match_mode`
- `palette`
- `constant_color`
- `solve_metric`
- `solve_score_quantile`
- `solve_score_omega`

### Important Detail About `quality`

Current color artifacts do not reliably persist JPEG quality in metadata.

That should be fixed as part of this feature:

- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py) should add `quality` to color artifact metadata
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py) should expose it in render summary entries

For old artifacts without saved quality:

- use fallback `90`


## Catalog Ordering Requirement

## Problem

Current render-summary ordering is simple `created_at DESC`.

That is wrong for this feature because the user wants:

- source selected
- press `Autolevels`
- new artifact appears immediately above source
- arrow up/down compares them

### Required Ordering Rule

Introduce derivation-aware ordering for the `color` family:

1. build parent-child relationships from `derived_from_artifact_id`
2. top-level color artifacts remain ordered by `created_at DESC`
3. for each top-level artifact, emit its derived children immediately before the parent
4. if a child has children, use depth-first flattening, newest-first among siblings

This gives stable adjacency and still keeps the main list chronological at the group level.

### Example

Instead of:

- `new_autolevel`
- `other_recent`
- `source`

the order should be:

- `other_recent`
- `new_autolevel`
- `source`

if `new_autolevel` derives from `source`.

### Files To Change

- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
  - parse new derivation fields in `_render_artifact_entry()`
  - replace plain sort for color-family variants with derivation-aware flattening


## Native Implementation Plan

### New Binary

Add a new native image processor, for example:

- [lambda/autolevels_render.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/autolevels_render.c)

This binary should:

1. load the selected image from disk via libvips
2. implement the same algorithmic sequence described in [autolevels.py](/Users/nicknassuphis/karpo_hackathon/polypaint/autolevels.py)
3. write the processed output image
4. write a preview PNG
5. emit compact JSON metadata on stdout for the handler to parse

The Python file remains the reference for:

- pooled histogram logic
- endpoint selection
- gamma handling
- optional vibrance
- final pooled RGB stretch

It should not be imported at runtime.

## Lambda Implementation Plan

## Recommended Handler Contract

Input job payload:

```json
{
  "job_id": "compute_xxxxxxxx",
  "task_id": "autolevels_<runid>",
  "artifact_id": "autolevels_<newid>",
  "source_artifact_id": "color_run_123",
  "source_image_key": "renders/compute_xxxxxxxx/color/color_run_123/image.jpeg",
  "source_preview_key": "renders/compute_xxxxxxxx/color/color_run_123/preview.png",
  "source_format": "jpeg",
  "source_quality": 90,
  "metadata": { "... inherited color artifact metadata ..." }
}
```

### Handler Steps

In [lambda/handler_autolevels.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_autolevels.py):

1. `report_status(job_id, task_id, "started", result_data={...})`
2. download source image to `/tmp/source.<ext>`
3. invoke the native autolevel binary with the popup parameters
4. write output image to `/tmp/out.<ext>`
5. write preview PNG to `/tmp/preview.png`
6. upload output image with inherited + derived metadata
7. upload preview PNG
8. `report_status(job_id, task_id, "done", result_data={"family":"color","artifact_id":..., "image_key": ...})`

On failure:

- `report_status(job_id, task_id, "error", str(e))`

### Status Labels

Recommended internal phases for logs/debug:

- `started`
- `download`
- `process`
- `preview`
- `upload`
- `done`

Even if only `started/done/error` are required for the observer, logging finer subphases is useful.


## Dependency / Packaging Plan

No new Python image layer should be introduced for this feature.

The deployment model should match the rest of the repo:

- native binary
- existing libvips Lambda layer
- thin Python handler

### Files To Add / Change

- [lambda/autolevels_render.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/autolevels_render.c)
- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
  - compile the new binary
  - package `handler_autolevels.py`
  - package the new binary into the autolevels zip
  - create/update the new Lambda
  - attach the existing libvips layer
  - wire `AUTOLEVELS_FUNCTION` into `dispatch`


## Frontend Plan

### Button / Popup

In [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

- add `Autolevels` button in the Color-family action row
- only render the button for `_renderActiveFamily === 'color'`
- enable it only when a selected color artifact exists and has an `image_key`
- clicking the button opens a popup, it does not immediately dispatch work

### Popup Behavior

Reuse the existing overlay/dialog pattern already used by the TRI / PAL / LONG palette popups:

- overlay closes on outside click
- `Esc` closes
- explicit close button closes
- no artifact is created by opening/closing alone

Popup contents should include:

- the selected source artifact id
- a compact parameter form with the autolevel defaults prefilled
- `AutoLevel` button
- `Cancel` button

### Popup Parameters

For v1, expose the full parameter set already documented above:

- `bins`
- `clip_low`
- `clip_high`
- `peak_factor`
- `gamma`
- `auto_gamma`
- `target`
- `sigmoid_strength`
- `sigmoid_mid`
- `vibrance`
- `pooled_rgb`
- `quality`
- `jpeg_subsample_mode`
- `jpeg_optimize_coding`
- `jpeg_interlace`

### Click Behavior

Add function:

- `openAutolevelPopup()`
- `runAutolevelSelectedRenderArtifact()`

Behavior:

1. `openAutolevelPopup()` reads the selected color artifact and fills the popup form
2. nothing is dispatched yet
3. `runAutolevelSelectedRenderArtifact()` dispatches one async job to `dispatch` target `autolevels`
4. store active render run object so existing observer can poll it
5. log:
   - `Autolevels: dispatching ...`
   - `Autolevels complete: ...`
   - `Autolevels failed: ...`

### Completion Behavior

On completion:

- call `refreshRenderArtifacts(jobId, { selectFamily: 'color', selectArtifactId: newArtifactId })`

Because render summary now does derivation-aware ordering, the newly selected artifact will appear directly above its source row.

### Files To Modify

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)


## Interaction With Existing Populate

No separate `Populate` behavior is needed for autoleveled artifacts if metadata inheritance is done correctly.

Because the derived artifact stays in family `color` and carries the original render metadata:

- current `Color -> Populate` should restore the same render settings
- `Palette -> Populate` remains unchanged

This is one of the main reasons to model autolevels as a derived color artifact instead of a separate family.


## Preview Generation

The autolevel handler should generate its own preview PNG directly.

Do not call a second Lambda for preview.

Reason:

- the handler already has the processed image in memory/on disk
- generating the preview in the same place is cheaper and simpler

Preview target:

- `renders/{job_id}/color/{artifact_id}/preview.png`


## Suggested Artifact Id Strategy

Use ids that encode derivation and preserve immutability:

- `autolevels_<unix_ts>_<shorthash>`

Where the short hash may include:

- source artifact id
- preset/profile name

This is enough to avoid collisions and keeps artifacts readable in the catalog.


## Error Handling

### Better Messages

The handler should emit specific messages for:

- missing source image key
- unsupported source format
- processing failure inside autolevel pipeline
- upload failure
- binary missing from Lambda package
- native autolevel process exited nonzero

Recommended examples:

- `Autolevels failed: missing source image renders/job/color/x/image.jpeg`
- `Autolevels failed: autolevels_render binary missing from package`
- `Autolevels failed: could not decode source image`

Frontend log text should say `Autolevels`, not generic `Render`.


## Storage / Summary Changes

### Needed Summary Fields

`/render-summary` entries for color artifacts should expose:

- `quality`
- `derived_from_artifact_id`
- `postprocess_kind`
- `postprocess_profile`

These fields support:

- inherited-populate correctness
- UI labels if needed later
- derivation-aware ordering

### Optional UI Summary Label

In [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html), `_renderArtifactSummary()` may append a short marker for derived autolevel artifacts, for example:

- `solve:anisotropy q=2.5% w=3 tri_redgold · autolevels`

This is optional but recommended.


## Exact Files Likely To Change

### New Files

- [autolevels.md](/Users/nicknassuphis/karpo_hackathon/polypaint/autolevels.md)
- [lambda/handler_autolevels.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_autolevels.py)
- [lambda/autolevels_render.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/autolevels_render.c)

### Existing Files

- [autolevels.py](/Users/nicknassuphis/karpo_hackathon/polypaint/autolevels.py)
  - reference only
  - no runtime integration required
- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [lambda/handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py)
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
  - add `quality` to color metadata for future renders
- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)
- [tests/test_deploy_packaging.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deploy_packaging.py)
- likely one e2e file:
  - [tests/e2e/render-refresh.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-refresh.spec.js)


## Test Plan

### Backend Unit Tests

Add handler tests for:

- autolevel success path
  - downloads selected source image
  - invokes native binary with the popup parameter set
  - uploads derived image + preview
  - preserves inherited render metadata
  - adds autolevel derivation metadata
  - reports `done` with `family=color` and `artifact_id`

- missing source image
  - clear error message
  - writes DDB `error`

- missing binary / subprocess failure
  - clear error message

### Storage Tests

Add tests that `/render-summary` for color family:

- returns derived metadata fields
- orders derived autolevel artifacts immediately above their parent
- keeps unrelated top-level artifacts in descending created order

### Frontend JS Tests

Add tests in [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh) for:

- `Autolevels` button visible only in Color family
- button disabled when no color selection exists
- click opens popup with selected artifact id and populated defaults
- `Esc` closes popup without dispatch
- outside click closes popup without dispatch
- dispatch payload contains selected source artifact info
- dispatch payload includes edited popup parameters
- completion refresh selects new artifact
- catalog order shows new derived artifact directly above source
- arrow-up / arrow-down can compare derived vs source without losing adjacency

### E2E

Add one browser test covering:

1. select a color artifact
2. click `Autolevels`
3. wait for refreshed catalog
4. verify new artifact row appears above source row
5. verify clicking / arrowing between the two swaps the viewer image

### Deploy Packaging Tests

Extend [tests/test_deploy_packaging.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deploy_packaging.py) so it fails if:

- `handler_autolevels.py` is not bundled
- `autolevels_render` is not bundled
- the autolevels Lambda is not attached to the libvips layer


## Non-Goals For V1

- no user-tunable autolevel parameters in the UI
- no separate artifact family
- no Step Functions workflow
- no autolevels for bilevel/coeff/palette artifacts
- no attempt to reprocess raw render intermediates; operate on the saved final image only


## Recommended Execution Order

1. add metadata support for `quality` on new color artifacts
2. implement derivation-aware ordering in render-summary
3. add `handler_autolevels.py`
4. package Python image deps in deployment
5. add dispatch target
6. add frontend button + observer integration
7. add tests


## Final Recommendation

This feature is doable and fits the current architecture well if it is implemented as:

- native C/libvips image postprocess
- thin async Python handler
- popup-configured action, not one-click immediate dispatch

Everything else is straightforward if the new artifact is treated as:

- family `color`
- immutable
- derived from a selected source artifact
- ordered immediately above its parent in render-summary

That gives you the UX you want without adding a new family or a new workflow type.
