# Remove Rotation Section In Render

This document describes how to remove the dedicated **Rotation** section from the Render tab safely.

## Main Finding

The current Rotation section is **not yet redundant** with `rotate_roots`.

Why:

- the old Render rotation rotates the viewport projection around the current viewport center
- `rotate_roots` rotates roots around the complex origin
- coeff render still uses the old rotation path, but does not use root transforms

So a straight UI deletion would silently change behavior.

## Current Behavior

### Old Rotation section

The Render tab currently exposes:

- slider `#render-rotation`
- direction select `#render-rotation-dir`

in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1183).

The common render params builder converts that into a radians value:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1791)

That `rotation` value is passed to:

- color root render at [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1861)
- bilevel root render at [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2199)
- coeff render at [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2454)

and then into:

- [handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py#L58)
- [handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py#L71)
- [handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py#L155)

Inside the raster binaries, the current rotation is applied after subtracting the viewport center:

- [roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c#L462)
- [bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_raster.c#L153)

So this is a **view-space rotation around `(center_re, center_im)`**.

### `rotate_roots`

The new root transform is implemented in:

- [root_xforms.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/root_xforms.h#L105)

It multiplies each root by `exp(i * 2*pi*turns)`.

So this is a **root-space rotation around the complex origin**.

That is only equivalent to the old Rotation section when:

- viewport center is exactly `0 + 0i`

For auto viewport or any off-center framing, they are different operations.

## Decision Needed First

Before removing the Rotation section, choose which semantic should survive.

There are three real options.

### Option 1: Keep the old semantic

Keep image/view rotation as the canonical feature.

Then:

- keep projection-time rotation in raster binaries
- remove `rotate_roots` if it is confusing
- or keep `rotate_roots` as a distinct advanced transform

This is the safest if you care about current behavior continuity.

### Option 2: Make `rotate_roots` the canonical feature

Then you are explicitly changing semantics from:

- rotate around viewport center

to:

- rotate around the origin

This is only acceptable if that behavior change is intentional.

If you choose this, old saved jobs may render differently under auto viewport.

### Option 3: Split them explicitly

Keep both, but name them honestly:

- `rotate_roots` = rotate around origin
- `rotate_view` or `rotation` = rotate around viewport center

This is the most technically clear design, but it does not remove the extra control.

## Recommendation

Do **not** remove the Rotation section until you resolve the semantic mismatch.

My recommendation is:

1. Decide whether origin-rotation is really what you want.
2. If yes, remove old view rotation only after replacing coeff render’s dependency and accepting the behavior change.
3. If no, keep the old Rotation section and treat `rotate_roots` as a separate root transform.

## Coeff Render Dependency

Even if you accept the semantic change for root renders, coeff render is a separate blocker.

The coeff bilevel path still uses the old `rotation` field:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2454)
- [handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py#L155)
- [coeffs_bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeffs_bilevel_raster.c#L69)

But coeff render does not use root transforms.

So if you remove the Rotation section outright, you must also decide:

- should coeff render lose rotation entirely?
- or should coeff render keep a separate view-rotation control?

## Safe Removal Plan

If you still want to remove the Rotation section, do it in this order.

### Phase 1: Resolve semantics

Pick one:

- keep view-space rotation
- switch to origin-space root rotation

Document the decision in:

- [root_transform.md](/Users/nicknassuphis/karpo_hackathon/polypaint/root_transform.md)
- render docs

If switching to `rotate_roots`, explicitly note:

- old auto-viewport renders may rotate differently because the center of rotation changed

### Phase 2: Decide coeff-render behavior

Choose one:

- remove rotation from coeff render too
- keep a coeff-only rotation control
- keep a generic view-rotation control that still applies to coeff render

This must be resolved before deleting the shared UI control.

### Phase 3: Normalize the root-transform UX

If `rotate_roots` is meant to replace the UI section, make it easy to use.

Recommended cleanup:

- expose it prominently in the root transform dropdown
- set its parameter label to `turns`
- ensure its sign convention matches the old UI expectation
- consider presets or chips for `0.25`, `0.5`, `0.75`, `1.0`

One more thing to check:

- [root_xforms.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/root_xforms.h#L105) says “Positive = CW”, but the math shown there is standard complex multiplication, which is CCW for positive angles. That should be corrected before using it as the canonical rotation control.

### Phase 4: Remove Render-tab Rotation UI

Delete from [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

- the entire `Rotation` color-box block
- `#render-rotation`
- `#render-rotation-val`
- `#render-rotation-dir`
- `#rotation-stops`

### Phase 5: Remove JS parameter plumbing

Update `_renderCommonParams()` in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1779) to stop returning:

- `rotation`

Then update callers:

- root render jobs
- bilevel root render jobs
- coeff render jobs

to stop sending `rotation: p.rotation`.

### Phase 6: Remove handler plumbing

Delete `rotation` pass-through from:

- [handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py#L58)
- [handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py#L71)
- [handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py#L155)

### Phase 7: Remove binary CLI support

Remove `--rotation` support from:

- [roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
- [bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_raster.c)
- [coeffs_bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/coeffs_bilevel_raster.c) if coeff render no longer supports it

Then remove the corresponding docs and usage strings.

### Phase 8: Update docs

At minimum update:

- [root_transform.md](/Users/nicknassuphis/karpo_hackathon/polypaint/root_transform.md)
- [docs/bilevel.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/bilevel.md)
- [docs/lambdas.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/lambdas.md)

to remove the old rotation flag from the render path if you actually delete it.

### Phase 9: Tests

Add or update tests for:

- `rotate_roots` sign convention
- equivalence or intentional non-equivalence versus old rotation
- absence of `rotation` in render payloads after cleanup
- coeff render behavior after the change

## Preferred Outcome

The best immediate next step is not removal. It is clarification.

Specifically:

1. decide whether you want origin rotation or viewport-center rotation
2. fix `rotate_roots` naming / sign semantics if needed
3. decide what coeff render should do
4. only then remove the old UI

Without that, deleting the Rotation section is likely to remove a feature that is still doing something distinct.
