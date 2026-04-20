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

Once the visible world bounds are known, the viewer can offer:

- `Use This View In Render`

That would populate explicit viewport bounds back into the Render tab.

This becomes especially valuable once exact viewport is implemented.

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

### 3. Standalone viewer

If standalone support is wanted:

- extend [deepzoom_viewer_template.html](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/deepzoom_viewer_template.html)
- add a small viewport HUD
- inject source bounds into template or fetch a sidecar JSON

### 4. Render handoff

Once the readout exists, optionally add:

- `Use Visible View`

which sends:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

to the Render tab explicit viewport controls.

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
