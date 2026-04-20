# DeepZoom Viewport Readout

## Goal

When viewing a DeepZoom image in OpenSeadragon, expose the current visible
viewport in render-world coordinates.

That means:

- know which source render artifact the DeepZoom export came from
- know the source render viewport
- map the current OSD image-space viewport back to render-world coordinates

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

Render artifacts may carry a non-zero `rotation` (degrees) applied to roots
before projection. When `rotation != 0`, the image-to-world map is not axis
aligned: a visible rectangle in image space corresponds to a rotated rectangle
in world space, so the four world-space corners are not simply
`(view_min_re, view_max_im)` etc.

Choose one path for this feature:

### Option R-A: Support rotation correctly

Copy `rotation` into the DeepZoom manifest. In the readout, after computing the
visible image rectangle, invert the rotation around the render viewport center
before reporting world coordinates. The readout still shows four numbers, but
they represent the axis-aligned bounding box of the rotated visible region — or
two shown corners plus a note that the visible region is tilted.

### Option R-B: Disable the readout when rotation != 0

Simpler. Still include `rotation` in the manifest, but when non-zero the readout
displays:

```text
Rotated render (rotation=15°)
Visible world viewport unavailable with rotation
```

and the Send-To-Render affordance is hidden.

### Recommendation

Start with Option R-B. It is honest, cheap, and covers the common case (most
renders are rotation=0). Option R-A can be added later if users ask for it.
Either way, `rotation` must be added to the manifest so the viewer can tell.

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

## Why This Gets Cleaner With Exact Bounds

With the planned exact viewport contract, render artifacts will store canonical:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

That makes the DeepZoom readout feature straightforward.

Without canonical bounds, the viewer must reconstruct viewport state from older
fields like:

- `view_mode`
- `square_extent`
- `quantile`
- `shim`

which is possible for legacy support, but more annoying.

## Recommended Metadata Contract

To make this feature clean, DeepZoom `meta.json` should include either:

### Option A: Copy source viewport directly

Add these fields to DeepZoom manifest:

- `source_job_id`
- `source_artifact_id`
- `source_family`
- `viewport_min_re`
- `viewport_max_re`
- `viewport_min_im`
- `viewport_max_im`
- `source_width`
- `source_height`
- `source_rotation` (degrees, for rotation handling; see Rotation section)

This is the simplest runtime contract for the viewer.

### Option B: Keep only source reference, fetch source artifact meta

DeepZoom manifest keeps:

- `source_key`
- `source_job_id`
- `source_artifact_id`

Then the viewer or app fetches the source render artifact metadata separately.

This avoids duplicating viewport metadata, but requires another lookup.

Recommendation:

- for the in-app DeepZoom tab: Option B is acceptable
- for the standalone `viewer.html`: Option A is better

The standalone viewer should not depend on extra app APIs if it can avoid it.

## Proposed Feature Variants

### Variant 1: In-app readout only

In the main app’s DeepZoom tab:

1. User selects a DeepZoom export.
2. App already has the export `meta.json`.
3. App resolves the source artifact metadata.
4. App listens to OSD viewport changes.
5. App shows current visible world bounds.

Display:

- `min_re`
- `max_re`
- `min_im`
- `max_im`
- maybe center and span too

This is the cheapest useful feature.

### Variant 2: Standalone viewer readout

Extend `viewer.html` so it also shows:

- source artifact id
- current visible world bounds

This requires the viewer HTML to receive or fetch viewport metadata.

### Variant 3: “Send To Render”

A `GotoRender` button already exists in the in-app DeepZoom tab (see
`_dzGotoSelectedRender` in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html))
that switches to the Render tab with the source artifact selected but does
**not** currently forward viewport bounds.

Variant 3 = extend that existing button to also forward:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

into the Render tab's explicit viewport fields, flipping the view selector to
`explicit`.

Only enabled when the DeepZoom manifest contains viewport bounds **and** (per
the Rotation section) the source artifact has `rotation == 0` under Option R-B.
Under Option R-A, bounds can be forwarded as the axis-aligned bbox of the
rotated visible region; document what the user will get.

Do not add a second button; extend the existing one.

## Recommended UI

For the in-app DeepZoom tab:

- add a small readout block under or above the viewer
- render it as plain text / monospace technical readout
- do not use disabled inputs

Suggested display:

```text
Visible world viewport
min_re=-0.812500  max_re=-0.437500
min_im= 0.125000  max_im= 0.500000
center=(-0.625000, 0.312500)
span=(0.375000 × 0.375000)
```

This matches the UI style guide:

- technical
- explicit
- no false affordance

## Implementation Plan

### 1. DeepZoom metadata

Either:

- copy source viewport bounds into DeepZoom `meta.json`

or:

- add enough source identifiers to fetch the source artifact metadata reliably

If exact viewport lands first, copying canonical bounds is preferable.

The change lands in one place:

- [lambda/handler_deepzoom_export.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_deepzoom_export.py) — in the `manifest = {...}` block around `handle_deepzoom_export_request`.

The from-raw variant at [lambda/handler_deepzoom_from_raw.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_deepzoom_from_raw.py)
delegates into the same function, so the new fields propagate automatically.
Also include `source_rotation` here so the viewer can honor the rotation
policy chosen in the Rotation section.

### 2. In-app DeepZoom tab

In [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

- when selecting a DeepZoom row, resolve source render metadata
- store:
  - image width/height
  - world bounds
- after `OpenSeadragon(...)`, subscribe to viewport updates
- compute visible world bounds
- render them into a plain-text readout

Likely OSD hooks:

- `open`
- `animation`
- `pan`
- `zoom`

and/or a debounced update path.

Convert OSD viewport coords to image pixels with
`viewer.viewport.viewportToImageRectangle(...)` before applying the linear map
in the Core Mapping section. See "OSD Coordinate Space" above.

### 3. Standalone viewer

If standalone support is wanted:

- extend [deepzoom_viewer_template.html](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/deepzoom_viewer_template.html)
- add a small viewport HUD
- inject source bounds into template or fetch a sidecar JSON

### 4. Render handoff

Once the readout exists, extend the existing `GotoRender` button
(`_dzGotoSelectedRender` in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html))
to also forward the current visible world bounds:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

into the Render tab's explicit viewport controls, flipping the view selector
to `explicit`. Do not add a second button.

Disabled (or forwards only the source artifact selection) when `source_rotation != 0`
under Option R-B.

## Legacy Compatibility

If the source artifact is legacy and does not store canonical bounds:

- if it is `square`, reconstruct from `square_extent`
- if it is `auto`, reconstruct from legacy viewport fields only if enough data exists
- otherwise, show:
  - `Viewport unavailable for this artifact`

Do not fake exact world bounds if the source metadata is insufficient.

## Tests

### App / browser tests

Add tests for:

- DeepZoom selection loads viewport readout when source bounds are available
- panning/zooming updates the readout
- source-less / bounds-less legacy rows show unavailable state
- `GotoRender` still works

### Math tests

Unit-test the mapping helper with:

- square viewport
- wide viewport
- tall viewport
- asymmetric viewport
- top/bottom y inversion
- OSD viewport-coord → image-pixel conversion (golden values against a fixture `viewportToImageRectangle` return)

### Rotation tests

- `rotation == 0` renders normally, readout shows bounds
- `rotation != 0`: behavior matches the chosen option (R-A: rotated-bbox bounds; R-B: "unavailable" and Send-To-Render disabled)
- manifest always includes `source_rotation`

### Contract tests

If DeepZoom manifest starts copying viewport bounds:

- assert manifest contains:
  - source identifiers
  - viewport bounds

## Recommendation

Do this in two steps:

1. implement exact viewport bounds as canonical render metadata
2. add DeepZoom viewport readout on top of that

That gives the cleanest contract.

If you want it before exact viewport lands, it is still possible, but the code
will need more legacy reconstruction logic and the result will be less robust.
