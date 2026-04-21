# DeepZoom Viewport Readout

## Goal

When viewing a DeepZoom image in OpenSeadragon, expose the current visible
viewport in render-world coordinates.

That means:

- know which source render artifact the DeepZoom export came from
- know the source render viewport
- map the current OSD image-space viewport back to render-world coordinates

Scope requirement:

- this must work for every DeepZoom-capable image artifact whose image geometry
  derives from a render viewport
- that includes direct color/bilevel/coeff images and derived artifacts such as
  repalette, recolor, Color2Bilevel, TIFF/PNG bilevel exports, resize outputs,
  and any future image artifact that preserves the same viewport geometry

## Short Answer

Yes, this is feasible.

For a square render viewport, the mapping is trivial.

For a general explicit-bounds viewport, it is still easy: it is just a linear
map from image pixel coordinates to world coordinates.

The main work is not math. The main work is making sure the DeepZoom export/viewer
has access to the source render viewport metadata.

## Current State

### What DeepZoom metadata stores now

The DeepZoom export manifest currently stores:

- `job_id`
- `export_id`
- `source_key`
- `width`
- `height`
- DZI/share URLs

in [lambda/handler_deepzoom_export.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_deepzoom_export.py).

That is enough to identify the source artifact, but not enough to directly report
world coordinates in the viewer.

Both DeepZoom entry points funnel through the same manifest-writing code:

- [lambda/handler_deepzoom_export.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_deepzoom_export.py) — the main export path.
- [lambda/handler_deepzoom_from_raw.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_deepzoom_from_raw.py) — a thin wrapper that delegates to `handle_deepzoom_export_request` with `require_raw_sidecar=True`.

So a single metadata-shape change in `handle_deepzoom_export_request` covers both.

### What the viewer does now

The standalone DeepZoom viewer template in
[lambda/deepzoom_viewer_template.html](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/deepzoom_viewer_template.html)
only initializes OpenSeadragon with the tile source.

It does not:

- fetch source render metadata
- compute world-coordinate viewport bounds
- display a live viewport readout

### What OpenSeadragon can provide

OpenSeadragon can provide the current visible image rectangle at runtime.

That means the viewer can know:

- left image x
- right image x
- top image y
- bottom image y

once the image dimensions are known.

So the image-space half of the problem is already solved by OSD.

## The Core Mapping

Assume:

- source image width = `W`
- source image height = `H`
- render-world bounds:
  - `min_re`
  - `max_re`
  - `min_im`
  - `max_im`

Then image-space to world-space is:

```text
re = min_re + (x / W) * (max_re - min_re)
im = max_im - (y / H) * (max_im - min_im)
```

The `im` formula is flipped because image y increases downward while the render
plane increases upward.

For a visible image rectangle:

- `x0`
- `x1`
- `y0`
- `y1`

the visible render-world viewport is:

```text
view_min_re = min_re + (x0 / W) * (max_re - min_re)
view_max_re = min_re + (x1 / W) * (max_re - min_re)
view_max_im = max_im - (y0 / H) * (max_im - min_im)
view_min_im = max_im - (y1 / H) * (max_im - min_im)
```

That is the full feature mathematically.

## OSD Coordinate Space

OpenSeadragon's `viewer.viewport.getBounds()` returns bounds in **normalized
viewport coordinates** (where the image's long edge is 1.0), not image pixels.

To get image-pixel coordinates for the formulas above, convert first:

```js
const vpBounds = viewer.viewport.getBounds(true);
const imgRect = viewer.viewport.viewportToImageRectangle(vpBounds);
// imgRect.x, imgRect.y, imgRect.width, imgRect.height are in image pixels
const x0 = imgRect.x;
const x1 = imgRect.x + imgRect.width;
const y0 = imgRect.y;
const y1 = imgRect.y + imgRect.height;
```

Then apply the linear map to `min_re..max_re` and `min_im..max_im`.

Do not apply the linear map directly to `getBounds()`; its values are not in
pixels.

## Rotation

Render artifacts may carry a non-zero `rotation` (radians) applied to roots
before projection. When `rotation != 0`, the image-to-world map is not axis
aligned: a visible rectangle in image space corresponds to a rotated rectangle
in world space, so the four world-space corners are not simply
`(view_min_re, view_max_im)` etc.

Support rotation correctly:

Copy `rotation` into the DeepZoom manifest. In the readout, after computing the
visible image rectangle, map that rectangle into the rotated camera plane, then
invert the rotation around the render viewport center to recover a new explicit
viewport center in render-input coordinates. Keep the visible `span_re` and
`span_im` unchanged. The resulting `min_re/max_re/min_im/max_im` are exactly
the explicit viewport bounds that reproduce the current DeepZoom crop when the
same `rotation` is preserved in Render.

Displayed text should still show four numbers, but now they are the
rotation-aware render viewport bounds, not a fail-closed placeholder.

## Why Square Viewports Feel Trivial

If the original render viewport was square, for example:

```text
min_re = -2
max_re =  2
min_im = -2
max_im =  2
```

and the image is square:

```text
W = H = pix
```

then:

- x and y use the same world span
- x and y use the same pixels-per-world-unit
- the mapping is symmetric

So yes, in that case the mapping is especially intuitive.

But the non-square case is not fundamentally harder. It is still just:

- one linear map in x
- one linear map in y

The only difference is the spans differ.

## Why This Is Straightforward Now

Exact viewport bounds are already canonical render metadata for current shipped
artifacts:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

That means the DeepZoom feature no longer needs to infer the camera from
`view_mode`, `square_extent`, `quantile`, or `shim` for the common path.

Legacy artifacts and older DeepZoom exports still exist, but they are now the
exception rather than the main design target.

## Chosen Contract

Choose one contract and use it everywhere:

- in-app DeepZoom tab
- standalone `viewer.html`
- `GotoRender` handoff
- artifact writers for any DeepZoom-capable derived image

### Canonical choice: Option A everywhere

DeepZoom `meta.json` must copy the source render viewport and source identity
directly. Do not make the viewer or the in-app tab perform a second lookup just
to compute the viewport readout.

Add these fields to DeepZoom `meta.json`:

- `source_key`
  already present; keep it
- `source_artifact_id`
- `source_family`
- `viewport_min_re`
- `viewport_max_re`
- `viewport_min_im`
- `viewport_max_im`
- `source_rotation`

Do **not** add duplicate `source_width` / `source_height` fields. The manifest
already contains:

- `width`
- `height`

and those are the authoritative image dimensions for the linear map.

`source_family` means the source render artifact family as used by the Render
tab catalog and `source_key` layout:

- `color`
- `bilevel`
- `coeffs`
- `palette`

`pdf` is out of scope; the Render tab does not DeepZoom PDFs.

### No second lookup for the readout

With Option A chosen:

- the in-app DeepZoom tab uses the selected export row data directly
- the standalone viewer fetches sibling `meta.json`
- no `/render-summary` or source-artifact lookup is needed to compute visible
  world bounds

The existing source selection behavior in `GotoRender` may still use
`source_key`, `source_family`, and `source_artifact_id` to select the source
artifact in the Render tab. That is separate from the viewport readout math.

### Artifact-writer requirement

Do not solve derived-artifact support in the viewer.

Instead, require this invariant across the codebase:

- every image artifact that can be used as a DeepZoom source and whose image
  geometry is inherited from a viewport must carry canonical viewport metadata
  directly in its own image metadata

The required carried fields are:

- `min_re`
- `max_re`
- `min_im`
- `max_im`
- `rotation`

and, when applicable for Render UI restore/display:

- `view_mode`
- `quantile`
- `shim`
- `square_extent`

These values must be copied unchanged from the source artifact whenever the
derivation preserves the same world viewport.

This keeps the DeepZoom readout/export logic simple:

- DeepZoom export reads direct-source metadata only
- viewer reads DeepZoom manifest only
- no parent traversal
- no reconstruction heuristics

## Chosen Rotation Policy

Use the rotation-aware inverse mapping in v1.

If `source_rotation != 0`:

- the readout still computes visible viewport bounds
- the reported `min_re/max_re/min_im/max_im` are the explicit viewport values to
  use in Render while preserving the same `rotation`
- `GotoRender` forwards those bounds into explicit viewport mode
- `GotoRender` keeps the populated source artifact rotation unchanged

Displayed text example:

```text
Visible world viewport
min_re=-2.500000  max_re=0
min_im=0  max_im=2.500000
center=(-1.250000, 1.250000)
span=(2.500000 x 2.500000)
rotation=-1.570796 (preserved)
```

## UI Contract

### In-app DeepZoom tab

Add a plain-text readout block directly under `#deepzoom-viewer` and above the
DeepZoom action buttons in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html).

Use this exact id:

- `deepzoom-viewport-readout`

Render it as:

- monospace
- compact technical text
- no disabled inputs
- no fake controls

Suggested default content:

```text
Visible world viewport unavailable
```

Suggested populated content:

```text
Visible world viewport
min_re=-0.812500  max_re=-0.437500
min_im= 0.125000  max_im= 0.500000
center=(-0.625000, 0.312500)
span=(0.375000 × 0.375000)
```

### Standalone viewer

Add a small HUD block in [lambda/deepzoom_viewer_template.html](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/deepzoom_viewer_template.html).

Use this exact id:

- `viewer-viewport-readout`

It follows the same formatting and unavailable states as the in-app readout.

## Update Strategy

Do not leave this vague. Use one update path:

- register handlers for:
  - `open`
  - `animation`
  - `animation-finish`
  - `resize`
- every handler calls `_scheduleDeepZoomViewportReadout()`
- `_scheduleDeepZoomViewportReadout()` uses a single `requestAnimationFrame`
  guard so only one readout recompute runs per frame

Do not recompute directly inside every OSD callback.

This gives:

- smooth updates while panning/zooming
- no event flood
- one deterministic code path

## Numeric Formatting

Use adaptive formatting for all reported world coordinates.

Define one helper for both in-app and standalone viewers:

```text
if value == 0 -> "0"
if 1e-4 <= abs(value) < 1e6 -> fixed with 6 decimals
otherwise -> scientific notation with 6 decimals
```

Examples:

- `0.125000`
- `-3.500000`
- `1.234567e-8`

Apply the same helper to:

- `min_re`
- `max_re`
- `min_im`
- `max_im`
- center
- span

## Feature Variants

### Variant 1: In-app readout

This is in scope.

Flow:

1. User selects a DeepZoom export.
2. App reads `viewport_*`, `width`, `height`, and `source_rotation` directly
   from the selected export row.
3. App updates `#deepzoom-viewport-readout` from OSD viewport state.

### Variant 2: Standalone viewer readout

This is in scope.

Flow:

1. `viewer.html` fetches sibling `meta.json`.
2. Viewer reads `viewport_*`, `width`, `height`, and `source_rotation`.
3. Viewer updates `#viewer-viewport-readout` from OSD viewport state.

### Variant 3: Extend existing `GotoRender`

This is in scope.

Do not add a second button.

Extend existing `_dzGotoSelectedRender` in
[index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) so that:

- if the current visible viewport is available:
  - it forwards the **current visible**:
    - `min_re`
    - `max_re`
    - `min_im`
    - `max_im`
  - into the Render tab explicit viewport controls
  - and flips the Render tab view selector to `explicit`
- regardless of rotation:
  - it still selects the source render artifact first
  - it preserves the populated source artifact rotation and root transforms
- otherwise:
  - it keeps the current behavior
  - selects the source render artifact only
  - forwards no viewport

## Implementation Plan

### 0. Artifact-writer propagation

Before or alongside the DeepZoom readout work, audit every DeepZoom-capable
derived image writer and make it preserve canonical viewport metadata when the
derived image keeps the same geometry.

At minimum this audit must cover:

- [lambda/handler_color_repalette.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_color_repalette.py)
- [lambda/color_recolor_raw.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/color_recolor_raw.py)
- [lambda/handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py)
  Color2Bilevel / from-raw path
- [lambda/handler_tiff_compat.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_tiff_compat.py)
- [lambda/handler_png_export.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_png_export.py)
- [lambda/handler_resize_artifact.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_resize_artifact.py)

Rule:

- if the derived image preserves the same world viewport, copy the canonical
  viewport metadata unchanged
- if a derivation genuinely changes world geometry in the future, it must write
  new correct canonical bounds explicitly

For this feature, missing canonical bounds on a current viewport-preserving
derived artifact is a writer bug to fix, not an acceptable steady state.

### 1. DeepZoom metadata

Update the `manifest = {...}` block in
[lambda/handler_deepzoom_export.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_deepzoom_export.py)
to copy viewport and source metadata from the selected source artifact.

Read the source artifact metadata via the family-appropriate artifact metadata
helper or existing render-artifact metadata path already used by the Render
catalog. Do not parse viewport information out of `source_key`.

Because [lambda/handler_deepzoom_from_raw.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_deepzoom_from_raw.py)
delegates into the same function, the metadata shape change covers both export
paths automatically.

### 2. In-app DeepZoom tab

In [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

- add `#deepzoom-viewport-readout`
- store the selected export's viewport metadata in JS state
- update the readout through the single RAF-throttled path
- convert OSD viewport coordinates to image pixels with
  `viewer.viewport.viewportToImageRectangle(...)`
- then apply the linear map from the Core Mapping section

### 3. Standalone viewer

In [lambda/deepzoom_viewer_template.html](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/deepzoom_viewer_template.html):

- add `#viewer-viewport-readout`
- fetch sibling `meta.json` via relative path (`fetch('meta.json')`)
- compute the readout with the same mapping and formatting rules

### 4. Render handoff

Extend existing `_dzGotoSelectedRender` to reuse the current readout state.

Do not recompute bounds separately for handoff; use the same already-computed
visible viewport values that drive the readout.

## Legacy and Backfill Policy

### Old DeepZoom exports

Do not backfill old DeepZoom exports.

Do not self-heal by fetching source artifact metadata on demand.

If an older DeepZoom export does not already contain copied viewport metadata:

- show `Visible world viewport unavailable`
- keep `GotoRender` source-selection behavior only

To gain viewport readout, the operator must create a new DeepZoom export.

### Legacy render artifacts

For this feature, do **not** reconstruct viewport from legacy render metadata.

Do not infer bounds from:

- `square_extent`
- `quantile`
- `shim`

If the direct source artifact does not expose canonical:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

then the DeepZoom manifest must omit `viewport_*`, and the viewer/readout must
show unavailable.

### Derived artifacts

For v1, do **not** walk the parent chain in the viewer or DeepZoom export.

Policy:

- viewport-preserving derived image artifacts must already carry canonical
  viewport fields directly in their own metadata
- DeepZoom export copies from the direct source artifact only
- if a direct source artifact is an older pre-fix derived artifact and lacks
  canonical bounds, fail closed and show unavailable
- for newly written derived artifacts in supported families, missing canonical
  bounds is a bug

This keeps the viewer/export contract honest without blocking support for
viewport-bearing derived images going forward.

## Non-goals

- parent-chain traversal for derived artifacts
- backfilling old DeepZoom exports
- adding a second DeepZoom render-handoff button
- making PDF artifacts DeepZoom-capable
- storing named/saved DeepZoom view presets

## Tests

### Backend contract tests

Add or update tests in:

- [tests/test_deepzoom_export_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deepzoom_export_handler.py)
- [tests/test_deepzoom_from_raw.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deepzoom_from_raw.py)
- [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)

Required cases:

- DeepZoom manifest includes:
  - `source_key`
  - `source_artifact_id`
  - `source_family`
  - `viewport_min_re`
  - `viewport_max_re`
  - `viewport_min_im`
  - `viewport_max_im`
  - `source_rotation`
- copied viewport fields match the source artifact metadata exactly
- if the source artifact lacks canonical bounds, manifest omits `viewport_*`
- `deepzoom_latest.json` preserves the same copied viewport fields

### Derived-artifact propagation tests

Add or update tests in:

- [tests/test_color_repalette_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_color_repalette_handler.py)
- [tests/test_recolor_from_raw.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_recolor_from_raw.py)
- [tests/test_bilevel_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_bilevel_handler.py)
- [tests/test_tiff_compat_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_tiff_compat_handler.py)
- [tests/test_png_export_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_png_export_handler.py)
- [tests/test_resize_artifact_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_resize_artifact_handler.py)

Required cases:

- derived image metadata preserves:
  - `min_re`
  - `max_re`
  - `min_im`
  - `max_im`
  - `rotation`
- viewport-preserving derivations also preserve:
  - `view_mode`
  - `quantile`
  - `shim`
  - `square_extent`
- DeepZoom export from those derived artifacts copies the same canonical
  viewport fields into `meta.json`

### Math tests

Add a focused helper/unit test file for viewport mapping, for example:

- [tests/test_deepzoom_viewport_math.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deepzoom_viewport_math.py)

Required cases:

- square viewport
- wide viewport
- tall viewport
- asymmetric viewport
- rotated viewport crop remaps back into explicit render bounds correctly
- full-image rotated viewport preserves the original explicit bounds
- y-axis inversion
- adaptive formatting
- OSD image-rectangle input to world-bounds output

### Browser tests

Add or extend browser tests in:

- [tests/e2e/deepzoom-inventory.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/deepzoom-inventory.spec.js)

Required cases:

- selecting a row with copied viewport metadata shows populated readout
- readout updates as the mocked OSD viewport changes
- row without `viewport_*` shows unavailable state
- row with `source_rotation != 0` shows a populated rotation-aware readout
- `GotoRender` still switches to the correct source artifact
- `GotoRender` forwards visible bounds when readout state is available,
  including rotated exports

### Standalone viewer tests

If the standalone viewer HUD is implemented in this cut, add a focused test for:

- fetching sibling `meta.json`
- rendering populated readout
- rotation-aware readout state

## Acceptance Criteria

The feature is done when all of the following are true:

1. New DeepZoom exports copy direct-source viewport bounds and source identity
   into `meta.json`.
2. Every DeepZoom-capable viewport-preserving derived image artifact writes
   canonical viewport metadata directly into its own image metadata.
3. The in-app DeepZoom tab shows a live readout in
   `#deepzoom-viewport-readout` for exports with copied viewport metadata.
4. The standalone `viewer.html` shows the same readout in
   `#viewer-viewport-readout`.
5. The readout uses the exact linear image-pixel to world-bounds map from this
   document.
6. Rotated exports (`source_rotation != 0`) use the documented inverse-rotation
   mapping and show the corresponding explicit viewport bounds.
7. `GotoRender` forwards the current visible viewport into Render explicit mode
   whenever readout state is available, including rotated exports.
8. Old exports or truly legacy artifacts without canonical bounds fail closed
   with unavailable state.
9. RePalette, recolor, bilevel, and other viewport-preserving derived images
   can produce DeepZoom exports with working viewport readout.
10. All new/updated tests pass.

## Recommendation

Implement this as one coherent DeepZoom metadata + viewer + handoff feature.

Do not split the contract between in-app and standalone viewers.
Do not add a source-artifact lookup layer just to compute the readout.
Do not try to be clever with legacy reconstruction in v1.
