# Exact Viewport

## Goal

Add a third Render view mode:

- `auto`
- `square`
- `explicit`

`explicit` means the operator enters:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

and the render clips exactly to that viewport.

## Short Answer

Yes, the right architecture is to make **explicit bounds** the canonical viewport
contract and fold `auto` and `square` into it as thin translations.

That is cleaner than keeping three different viewport models alive in parallel.

But:

- this is **not** a pure UI patch
- the current runtime contract is still a **square isotropic camera**
- exact rectangular bounds require changing that runtime contract

So the design direction is correct, but the implementation is not just “add four
inputs and thread them through.”

## Current State

### UI

Render currently exposes only:

- `auto`
- `square`

in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html).

The browser currently sends:

- `view_mode`
- `quantile`
- `shim`
- `square_extent`

from `_renderCommonParams(...)`.

The new UI should follow the existing Render-tab view-row pattern, not invent a
new popup or secondary tuning surface.

### Planner

The planner computes a viewport in
[lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py):

- `square`:
  - center = `(0, 0)`
  - one isotropic `scale`
- everything else:
  - call the viewport Lambda
  - get quantile box
  - reduce to center + one isotropic `scale`

`_compute_viewport(...)` today returns:

- `center_re`
- `center_im`
- `scale`

not explicit bounds.

### Runtime

Raster handlers and native binaries consume:

- `width`
- `height`
- `center_re`
- `center_im`
- `scale`

This is a square/isotropic camera model.

That is the key constraint.

## Why Canonical Explicit Bounds Is Better

The canonical internal viewport should be:

```json
{
  "min_re": ...,
  "max_re": ...,
  "min_im": ...,
  "max_im": ...
}
```

Then:

- `square` is just a special case that computes those four numbers
- `auto` is just a special case that computes those four numbers
- `explicit` directly provides those four numbers

That gives one consistent viewport model for:

- planner
- metadata
- populate/restore
- storage summaries
- future tools like DeepZoom, viewport preview, and exports

This is the right direction.

## Why The Current Contract Is Not Enough

If the user asks for exact:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

then the runtime must preserve that rectangle exactly.

The current contract cannot do that in the general case, because it only has:

- one center
- one scale

One isotropic scale cannot represent an arbitrary exact rectangle on a fixed
square image unless:

- the world-space viewport aspect matches the pixel aspect, or
- you accept padding/cropping, or
- you accept anisotropic mapping but stop pretending the contract is one-scale

Today the main Render path is effectively square-first:

- one `pix`
- square grid assumptions in plan/raster

So an arbitrary explicit rectangle is not exact unless the runtime changes.

## Exact Means A Decision About Pixel Mapping

To make explicit bounds exact and non-distorting, one of these must be true:

1. The output image can be non-square.
2. The runtime supports different x/y scales.
3. The runtime pads inside a square image, which means the world bounds are exact
   only for the active image area, not the whole square frame.

If “exact viewport” means:

- the full rendered image covers exactly `[min_re,max_re] × [min_im,max_im]`
- with no padding
- with no stretch

then the runtime needs either:

- `min_re/max_re/min_im/max_im`, or
- `center_re/center_im/scale_x/scale_y`

not just `center + scale`.

## Chosen Direction

This design should assume:

- output images remain square
- output size remains `pix × pix`
- viewport bounds are exact
- x and y world-units-per-pixel may differ

That means:

- no aspect-derived output sizing
- no padding
- no cropping
- exact bounds still hold

The runtime therefore needs exact bounds or separate per-axis mapping, but not
derived image dimensions.

## Recommended Canonical Contract

Use this canonical viewport object inside the planner and metadata:

```json
{
  "mode": "explicit",
  "min_re": -1.25,
  "max_re": 0.75,
  "min_im": -0.40,
  "max_im": 0.90
}
```

For `square` and `auto`, the planner should still emit that same shape.

### UI Request Contract

Browser request params:

```json
{
  "view_mode": "auto|square|explicit",
  "quantile": 0.01,
  "shim": 0.07,
  "square_extent": 2.0,
  "min_re": null,
  "max_re": null,
  "min_im": null,
  "max_im": null
}
```

Rules:

- `auto` uses only `quantile` + `shim`
- `square` uses only `square_extent`
- `explicit` uses only `min_re/max_re/min_im/max_im`

Unused fields are ignored.

### Planner Canonicalization

`handler_render_plan.py` should canonicalize all three modes into:

```json
{
  "viewport": {
    "min_re": ...,
    "max_re": ...,
    "min_im": ...,
    "max_im": ...
  }
}
```

Then everything downstream should use `plan.viewport`, not `view_mode`-specific
logic.

## How Each Mode Translates

### Explicit

Direct mapping:

```text
min_re = user input
max_re = user input
min_im = user input
max_im = user input
```

Validation:

- all four required
- all finite
- `max_re > min_re`
- `max_im > min_im`

### Square

Current square mode is centered at the origin.

For `square_extent = ext`:

```text
min_re = -ext
max_re =  ext
min_im = -ext
max_im =  ext
```

If rotation/root transforms are later interpreted as changing the desired camera
frame rather than the roots, that needs to stay separate. Current code applies
rotation in raster, not in viewport construction.

### Auto

Current auto path already computes quantile bounds in the viewport Lambda:

- `q_re = [q_min_re, q_max_re]`
- `q_im = [q_min_im, q_max_im]`

So `auto` should become:

```text
range_re = (q_max_re - q_min_re) * (1 + shim)
range_im = (q_max_im - q_min_im) * (1 + shim)
center_re = (q_min_re + q_max_re) / 2
center_im = (q_min_im + q_max_im) / 2

min_re = center_re - range_re / 2
max_re = center_re + range_re / 2
min_im = center_im - range_im / 2
max_im = center_im + range_im / 2
```

That is the right thin translation.

## Recommended Runtime Change

For true exact bounds, the raster/native runtime should stop taking only:

- `center_re`
- `center_im`
- `scale`

and instead take either:

### Option A: Bounds

```text
--min_re=...
--max_re=...
--min_im=...
--max_im=...
```

This is the clearest API.

### Option B: Center + Separate Scales

```text
--center_re=...
--center_im=...
--scale_x=...
--scale_y=...
```

This is workable, but less direct than bounds.

Recommendation:

- use **bounds** in the planner and handler payloads
- native code may internally convert those into per-axis scale if that is easier

## Output Size Contract

For this feature, output should remain:

- `width = pix`
- `height = pix`

for all render families that currently render square outputs.

The exact viewport requirement applies to the world-space domain, not to the image
aspect ratio.

That means the runtime mapping becomes:

```text
x_scale = width  / (max_re - min_re)
y_scale = height / (max_im - min_im)
```

with:

- `width = pix`
- `height = pix`

This preserves:

- exact bounds
- square output

while allowing:

- anisotropic world-to-pixel scale

That is the intended behavior for this design.

## Metadata Changes

Artifact metadata should persist canonical bounds:

- `view_mode`
- `min_re`
- `max_re`
- `min_im`
- `max_im`

Legacy compatibility fields can remain for a migration window:

- `quantile`
- `shim`
- `square_extent`

but canonical restore should prefer the explicit bounds if present.

## Populate / Restore Changes

Current populate logic in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
restores only:

- `auto`
- `square`

It should become:

- if bounds exist:
  - restore `explicit`
  - fill the four fields
- else if `view_mode === square`:
  - restore `square`
- else:
  - restore `auto`

That keeps old artifacts working while making explicit viewport first-class.

## Storage / Summary Changes

Storage summaries currently expose view metadata like:

- `view_mode`
- `quantile`
- `shim`
- `square_extent`

They should also expose:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

for render inventory, populate, and future DeepZoom/viewer tooling.

## Implementation Shape

### Smallest Coherent Version

If the goal is correctness, not a half-step:

1. Add `explicit` mode and four UI inputs in Render.
2. Canonicalize all modes to bounds in `handler_render_plan.py`.
3. Change plan/runtime payloads from center+scale to exact bounds or x/y scales.
4. Keep output square (`width = height = pix`) and render anisotropic world-to-pixel mapping into it.
5. Persist canonical bounds in artifact metadata.
6. Restore canonical bounds on Populate.
7. Update tests.

### Half-Step To Avoid

Do not do this:

1. add explicit bounds UI
2. convert bounds back into one `center + scale`
3. keep square output
4. call it exact

That would only be exact for square world-space viewports.

## Assessment

### Architecturally

Yes, your proposed direction is correct:

- make explicit bounds the only real viewport contract
- make `auto` and `square` thin calculations on top of it

That is cleaner than the current setup.

### Implementation Effort

Not trivial, because exact bounds require changing the runtime contract away from
the current single-scale square camera.

So the honest answer is:

- **easy as an architectural model**
- **moderate-to-large as an implementation**

because the backend and native raster path need to stop pretending every viewport
is representable as `center + one scale`.

## Recommended Next Step

If implementing this, do not start from the UI.

Start by deciding and documenting the runtime contract:

1. canonical viewport bounds
2. output size behavior for non-square bounds
3. handler/native argv shape

Once that is fixed, the UI work is straightforward.

## Implementation Plan By File

This is the concrete cut for the bounds-first version.

### 1. [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

#### Add UI mode

Extend the Render view selector from:

- `auto`
- `square`

to:

- `auto`
- `square`
- `explicit`

Add four numeric inputs:

- `render-min-re`
- `render-max-re`
- `render-min-im`
- `render-max-im`

Recommended labels:

- `Min Re`
- `Max Re`
- `Min Im`
- `Max Im`

Do not label this `LLUR`. The codebase and metadata should use the explicit axis
names.

UI-guide compliance requirements:

- keep this in the existing Render view block using the current `.view-row` style
- numeric inputs stay compact and monospace
- labels remain explicit and short
- any stretch/aspect readout must be plain text or helper text, not a disabled
  input
- do not hide exact bounds behind a popup; this is a primary viewport control
- do not add fake controls for derived values

#### Browser request shape

Update `_renderCommonParams(...)` to emit:

- `minRe`
- `maxRe`
- `minIm`
- `maxIm`

and continue emitting:

- `viewMode`
- `quantile`
- `shim`
- `squareExtent`

`_launchFusedRenderOrchestrator(...)` and `_launchNonColorRenderOrchestrator(...)`
should forward the new explicit bounds fields in `orchPayload.params`.

The dispatched orchestrator payload should use snake_case field names:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

#### Mode behavior

`selectViewMode(...)` should:

- highlight `explicit`
- keep `auto` slider changes selecting `auto`
- keep `square` extent changes selecting `square`
- switch to `explicit` when any explicit bounds field is focused/edited

Mode switching rules should be explicit:

- switching to `auto` does not erase explicit bounds fields
- switching to `square` does not erase explicit bounds fields
- switching to `explicit` does not rewrite `quantile`, `shim`, or `square_extent`
- Populate overwrites the active form with the artifact’s saved viewport state
- manual user edits after Populate are local form state only until dispatch

If the explicit bounds aspect is not `1:1`, the UI should show a clear note that:

- output remains square
- the domain will be stretched on one axis
- the x/y stretch factor is `((max_re - min_re) / (max_im - min_im))`

This can be:

- a help-text note
- a live stretch-factor readout
- or both

Label it unambiguously as world aspect, for example:

- `World aspect Δre:Δim = 2.0 : 1.0`

Do not label it with a bare `stretch factor` number.

#### Populate / restore

Update the artifact populate path so it restores:

1. explicit bounds if present
2. else square mode
3. else auto mode

This currently only knows `auto` and `square`.

#### UI summary / validation

Add browser-side validation:

- all four explicit fields must be finite
- `max_re > min_re`
- `max_im > min_im`

Reject invalid explicit requests before dispatch.

Explicit-mode validation/error rules:

- invalid explicit bounds block dispatch
- the error should surface in existing Render status/log surfaces
- invalid explicit bounds must not silently fall back to `auto`
- empty explicit fields must not be coerced to zero

#### Optional helper

Add one helper that normalizes current UI view state to canonical bounds in the
browser for summaries and future preview tooling:

```js
_currentRenderViewportBounds()
```

That helper is not authoritative for final planning, but it keeps the UI logic
clear.

#### UI IDs and source guards

The implementation should pin these exact IDs in frontend source tests:

- `view-row-explicit`
- `render-min-re`
- `render-max-re`
- `render-min-im`
- `render-max-im`

and assert the render dispatch path includes:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

### 2. [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)

This is the main pivot point.

#### Canonical viewport object

Replace the planner’s current output shape:

```json
{
  "center_re": ...,
  "center_im": ...,
  "scale": ...
}
```

with canonical bounds:

```json
{
  "min_re": ...,
  "max_re": ...,
  "min_im": ...,
  "max_im": ...
}
```

The old center/scale shape should not remain the planner’s source of truth.

#### Canonicalization rules

`_compute_viewport(...)` should:

- `square` -> compute square bounds
- `auto` -> call viewport Lambda, expand quantile bounds with shim, return bounds
- `explicit` -> validate and return user bounds directly

The planner should reject unknown `view_mode` values explicitly.

#### Grid sizing

Keep square output:

- `width = pix`
- `height = pix`

Then compute:

- `n_tile_cols = ceil(width / tile_size)`
- `n_tile_rows = ceil(height / tile_size)`

This avoids the separate aspect-driven output-size refactor.

#### Plan payload

`plan.grid` should become:

- `width`
- `height`
- `pix`
- `tile_size`
- `n_tile_cols`
- `n_tile_rows`

Downstream runtime may still have `width == height == pix`, but it must stop
assuming the world-space viewport is representable by one isotropic scale.

#### Metadata

Artifact metadata should add:

- `min_re`
- `max_re`
- `min_im`
- `max_im`

and continue storing:

- `view_mode`

Legacy fields may remain for compatibility:

- `quantile`
- `shim`
- `square_extent`

but canonical restore should prefer bounds.

Metadata write rules:

- always write bounds for new renders, even for `auto` and `square`
- continue writing legacy compatibility fields during the migration window
- do not write conflicting bounds and legacy values for the same new artifact

#### Digesting

`_plan_params_digest(...)` should digest canonical bounds, not center+scale. This
keeps cache/identity stable when the same bounds are reached from different UI
modes.

### 3. [stepfunctions/render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)

This file currently threads:

- `center_re`
- `center_im`
- `scale`

into raster tasks.

It must be updated to thread:

- `min_re`
- `max_re`
- `min_im`
- `max_im`
- `width`
- `height`

for all render families that raster directly from solves:

- fused color
- bilevel
- coeff_bilevel

Any task payload that still assumes:

- `width = $.plan.grid.pix`
- `height = $.plan.grid.pix`

must be changed to use the new explicit dimensions.

### 4. [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)

#### Handler contract

Stop requiring:

- `center_re`
- `center_im`
- `scale`

Start requiring:

- `min_re`
- `max_re`
- `min_im`
- `max_im`
- `width`
- `height`

#### Native argv

Replace current native argv camera parameters with bounds argv:

```text
--min_re=...
--max_re=...
--min_im=...
--max_im=...
```

Retain:

- `width`
- `height`
- `tile_size`
- tile grid
- rotation

#### Validation

Validate bounds before subprocess launch:

- finite
- strict ordering

### 5. [lambda/handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py)

This file has the same camera contract problem as color raster.

Update all active raster paths to consume:

- bounds
- width
- height

instead of center+scale.

This affects:

- section raster
- coeff raster
- merge/finalize assumptions where they still treat image dimensions as square

Finalize is mostly dimension-driven already, but every place that assumes `pix`
implies both width and height needs to switch to real width/height.

### 6. Native Raster Binaries

Files likely affected:

- [lambda/roots2pix_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix_mt.c)
- [lambda/bilevel_section_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_section_raster.c)
- [lambda/bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_raster.c)
- [lambda/coeffs_bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeffs_bilevel_raster.c)

#### Required contract change

Replace the square-camera projection logic:

- subtract center
- multiply by one scale

with exact per-axis projection from bounds:

```text
u = (re - min_re) / (max_re - min_re)
v = (im - min_im) / (max_im - min_im)

px = floor(u * width)
py = floor((1 - v) * height)
```

or equivalent numerically stable logic.

Rotation remains a separate transform on roots before projection.

#### Important rule

Do not internally re-square the viewport.

The whole point of the bounds contract is that the world rectangle is exact.

Do not derive padding or crop from aspect mismatch.

The raster should instead map:

- `min_re..max_re` across `0..width`
- `min_im..max_im` across `0..height`

even when the output image remains square.

### 7. [lambda/handler_viewport.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_viewport.py)

This Lambda currently returns:

- `center_re`
- `center_im`
- `scale_ref`
- `q_re`
- `q_im`

It should shift to returning quantile bounds directly as the primary output:

- `q_re`
- `q_im`

Optionally it can keep:

- `center_re`
- `center_im`
- `scale_ref`

for a compatibility window, but the planner should stop depending on them.

The planner should derive explicit auto bounds from:

- `q_re`
- `q_im`
- `shim`

### 8. [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)

Render artifact summaries should expose:

- `view_mode`
- `min_re`
- `max_re`
- `min_im`
- `max_im`
- `width`
- `height`

where available.

This is needed for:

- Populate
- Render inventory summaries
- any future viewer tooling

The `handle_detail(...)` path already reads `view.json`; if `view.json` remains in
use, it should remain a diagnostic/helper artifact only.

Decision:

- do not make `view.json` canonical
- do not rely on it for exact viewport replay
- canonical viewport state must live in plan payloads and artifact metadata

### 9. [lambda/shared.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/shared.py)

`compute_viewport_from_bin(...)` can remain the quantile-finding helper, but it
should no longer be treated as returning the final canonical render camera.

Recommended change:

- keep `q_re`
- keep `q_im`
- treat those as the authoritative auto-mode source

Any helpers that currently assume square camera math should be split from the
quantile-bounds helper.

## Implementation Order

The safest order is:

1. Canonical planner/runtime contract
   - bounds
   - fixed square width/height
2. Native raster updates
3. ASL payload updates
4. Metadata/storage updates
5. UI explicit mode
6. Populate/restore
7. Tests

Do not start from the UI.

If the runtime contract is not changed first, the implementation will drift into
fake exactness.

## Test Plan

### Backend unit tests

Update and expand:

- [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)

Add:

- explicit mode returns canonical bounds unchanged
- square mode translates to symmetric bounds
- auto mode translates `q_re/q_im + shim` to bounds
- plan grid keeps square `width == height == pix`
- planner no longer reduces non-square bounds to one scale

### Workflow contract tests

Update:

- render workflow contract tests to assert ASL now threads bounds and width/height

Required assertions:

- no remaining `center_re`
- no remaining `center_im`
- no remaining `scale`

in raster task payloads on the active render workflow paths

### Native parity / projection tests

Update raster tests so they assert exact clipping/projection under non-square
bounds.

Important cases:

- wide viewport
- tall viewport
- asymmetric viewport around zero
- negative-edge clipping
- exact square-output mapping with different x/y scale
- parity: `square(ext=2.5)` render is byte-identical pre-refactor vs post-refactor

Concrete file targets:

- [tests/test_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_mt.py)
- [tests/test_raster_mt_parity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_mt_parity.py)
- [tests/test_bilevel_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_bilevel_raster.py)

Required parity cases:

- fused color square viewport parity
- bilevel square viewport parity on the active raster path
- coeff_bilevel square viewport parity on the active raster path

### Frontend tests

Update:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

Add source guards for:

- explicit mode row
- four explicit inputs
- dispatch payload includes bounds
- populate/restore prefers bounds

Add browser coverage in the active Render UI spec for:

- switch to explicit
- enter bounds
- dispatch payload contains exact numbers
- populate restores explicit mode and values
- invalid explicit bounds block dispatch and surface an error
- switching modes does not erase inactive mode values

Concrete file targets:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)

### Storage / metadata tests

Update/add tests so render artifact summaries expose bounds and Populate prefers
them over legacy fields.

Concrete file targets:

- [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)

Required cases:

- new artifact summary includes `min_re/max_re/min_im/max_im`
- legacy artifact with only `square_extent` still restores correctly
- new artifact with both legacy and bounds fields restores from bounds

### Predeploy gate

Before considering the feature complete, rerun:

- [scripts/predeploy_check.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/scripts/predeploy_check.sh)

This is required in addition to targeted/browser/native tests.

## Compatibility Notes

### Existing artifacts

Old artifacts will only have:

- `view_mode`
- `quantile`
- `shim`
- `square_extent`

That is fine. Populate/restore should:

1. prefer bounds if present
2. else restore old square mode
3. else restore old auto mode

### Existing view.json

`view.json` should remain a diagnostic artifact only.

The canonical render contract should live in:

- planner payload
- ASL task payload
- artifact metadata

## Non-Goals

These are explicitly out of scope for this change:

- aspect-preserving letterbox output
- Compute-tab preview viewport refactor
- Palette-tab viewport changes
- named or saved viewport presets

## Acceptance Criteria

This feature is complete only when all of these are true:

1. `explicit` mode exists in Render.
2. The browser sends exact bounds.
3. The planner canonicalizes all view modes to bounds.
4. Raster handlers and native binaries consume exact bounds.
5. Non-square explicit bounds render on square output without padding or cropping.
6. Output remains square `pix × pix`.
7. Artifact metadata stores bounds.
8. Populate restores explicit bounds when present.
9. Auto and square still work, but only as translations into bounds.
10. All tests pass.
11. Pre-refactor equivalent cases replay byte-identically where the math is unchanged.
12. The predeploy contract gate passes.

## Regression Gate

The most important correctness gate is parity on equivalent square viewports.

For example:

- old path: `view_mode=square`, `square_extent=2.5`
- new path: bounds-derived square viewport with the same domain

must produce byte-identical output.

That parity test should be required before rollout, because it catches:

- translation mistakes
- off-by-one projection drift
- changed rounding behavior
- accidental crop/pad behavior
