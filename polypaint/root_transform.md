# Root Transforms

This document describes the implementation plan for a new **root transform** layer.

Root transforms operate on solved roots:

- after parameter transforms
- after the coefficient function
- after coefficient transforms
- after the solver
- before rasterization maps roots to pixels

So the full conceptual pipeline becomes:

1. parameter transforms
2. coefficient function
3. coefficient transforms
4. solver
5. root transforms
6. rasterizer

The critical design decision is:

- **root transforms are a Render-stage feature**

not:

- a Compute-stage feature
- a saved-data mutation step
- a new persisted intermediate

They are applied **in flight** while tiles are being created.

## Goal

Add a render-time transform pipeline that:

- takes one vector of roots for each solved sample
- returns a new vector of roots of the same length
- supports chaining
- supports parameters per transform
- is exposed in the Render UI as chips, like the existing transform rows

Initial transforms:

- `rotate_roots(turns_half_pi)`
- `pull_unit_circle(sigma, alpha)`

## What This Feature Is

Root transforms are optional geometry filters on already-computed roots.

They let the user:

- compute roots once
- render the same job multiple different ways
- avoid recomputing coefficients or solving again

That means the canonical solved root files remain unchanged:

- `chunk_XXXX.bin` stays canonical

The transform layer only changes what gets projected into tiles during a particular render request.

## What This Feature Is Not

It is not:

- another Compute-tab pipeline row
- another saved-down root artifact
- another S3 intermediate format
- something that affects coeff render or param debug

So there should be no:

- `root_transforms` inside `calc.json`
- transformed `chunk_XXXX.bin` files
- transform-specific saved root copies

At most, root transforms may appear in transient render request payloads and optional render logs.

## Scope

Root transforms apply only to render modes that consume solved roots:

- color render via [handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
- root-based bilevel render via [handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py)

They do **not** apply to:

- coeff render, which plots coefficient points directly
- param debug, which plots transformed parameter points directly

## UI Plan

The root transform UI belongs in the **Render** tab, not the Compute tab.

Reason:

- the transforms affect only how saved roots are rendered
- they do not change what Compute produces
- the same computed job should support multiple render pipelines

### Placement

Add a new row in the Render tab near the other render controls.

Suggested Render-tab order:

1. view / viewport
2. rotation
3. color controls
4. root transforms
5. render buttons

That keeps it visually clear that root transforms are part of the render pipeline, alongside:

- viewport
- image rotation
- color mode
- bilevel vs color selection

### Interaction model

Reuse the chip pattern already used for:

- parameter transforms
- coefficient transforms

Suggested state name:

- `_rtChain`

UI elements:

- add-transform dropdown
- chip list
- per-chip editable params
- delete chip
- preserve left-to-right execution order

Suggested initial chip labels:

- `rotate_roots(0.5)`
- `pull_unit_circle(0.75,1.0)`

## Render Contract

Root transforms should be sent only as part of render dispatch payloads.

Suggested field:

```json
"root_transforms": [
  ["rotate_roots", "0.5"],
  ["pull_unit_circle", "0.75", "1.0"]
]
```

This field should be included in:

- `runRasterPipeline()`
- `runBilevelPipeline()`

and excluded from:

- compute job creation
- coeff render
- param debug

## No Saved Transform Data

This feature should not create any new saved root data.

Specifically:

- do not save transformed roots back to S3
- do not write new transformed root stripe files
- do not add transformed roots to `calc.json`

The only saved render outputs should remain the normal render artifacts:

- color `.pix` / `.raw` / final image outputs
- bilevel `.bits` / tile TIFF / final TIFF outputs

The root transforms are applied on the fly, during rasterization, and then discarded.

## In-Flight Execution Model

The correct model is:

1. read one solved root vector from the canonical stripe file
2. copy roots into a working buffer
3. apply the root transform chain to the working buffer
4. rasterize the transformed roots into the current tile output
5. move on to the next sample

This happens entirely inside the raster step while tiles are being created.

That is the right place because:

- raster already has the roots in memory
- no extra persistence step is needed
- root transforms are pure geometry operations
- render-time experimentation stays cheap

## Where To Implement It

Implement root transforms in C inside the raster binaries, not in Python.

Reason:

- the transform acts per root vector, at raster time
- the performance-sensitive code already lives in C
- both root-based render paths need the same behavior
- Python would create unnecessary duplication and data marshaling

So the design should be:

- Python handlers parse `root_transforms`
- handlers pass the transform chain into the raster binary
- raster binary applies transforms in memory before projection

## Shared Implementation

Do **not** duplicate the transform code across:

- [roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
- [bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_raster.c)

Instead, create a shared implementation, for example:

- `lambda/root_xforms.h`
- `lambda/root_xforms.c`

with:

- transform-chain parsing
- transform dispatch
- helper math routines

Both raster binaries should call the same root-transform engine.

## Data Model

Each solved sample currently contains one vector of roots:

- `roots[0..degree-1]`

For now, root transforms should preserve vector length:

- input: `Complex[]`
- output: `Complex[]`
- same number of roots in and out

That keeps:

- matching
- coloring
- dedup
- raster projection

simple and compatible with the current pipeline.

## Transform Semantics

### `rotate_roots`

Source behavior:

```python
theta = np.pi * a[0].real
rotated = z * np.exp(1j * theta)
```

Interpretation:

- parameter: half-turn count measured in units of `pi`
- actual angle:
  - `theta = pi * turns`

Effect:

- multiply every root by the same complex rotation factor

Vector form:

- `out[k] = roots[k] * exp(i * pi * turns)`

This is a rigid rotation around the origin.

### `pull_unit_circle`

Source behavior:

```python
sigma = a[0].real or 0.75
alpha = a[1].real or 1.0

r = abs(root)
theta = arg(root)
d = r - 1
rprime = r - alpha * d * exp(-(d / sigma) ** 2)
out = rprime * exp(i * theta)
```

Interpretation:

- radial deformation relative to the unit circle
- roots near radius `1` are pulled toward radius `1`
- distant roots are affected less because of the Gaussian falloff

Parameters:

- `sigma`
- `alpha`

Per-root formula:

- `r = |z|`
- `theta = arg(z)`
- `d = r - 1`
- `r' = r - alpha * d * exp(-(d / sigma)^2)`
- output = `r' * exp(i*theta)`

Edge case:

- if `sigma == 0`, clamp it to a small epsilon before dividing

## Execution Order

Root transforms run in chip order.

For one solved sample:

1. read roots from stripe
2. apply root transform 1
3. apply root transform 2
4. ...
5. rasterize transformed roots

So:

```json
[
  ["rotate_roots", "0.5"],
  ["pull_unit_circle", "0.75", "1.0"]
]
```

means:

- rotate first
- then pull toward the unit circle

## Color Render Placement

For color render, root transforms must happen before:

- root matching
- color assignment
- pixel projection

Reason:

- matching depends on root geometry
- if transforms change the root geometry, matching should operate on transformed roots

So inside [roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c), the per-sample order should be:

1. decode root vector
2. copy into working buffer
3. apply root transforms
4. perform matching/color logic on transformed roots
5. project to pixels
6. emit `.pix`

## Bilevel Render Placement

For root-based bilevel render, root transforms should happen:

1. decode root vector
2. copy into working buffer
3. apply root transforms
4. project transformed roots
5. set tile bits

That belongs in [bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_raster.c).

## Frontend Work

### Add render-side state

Add:

- root transform catalog
- render-tab root transform chain state
- chip add/remove/edit helpers

Suggested state:

- `_rtChain`

### Pass through render payloads

Include `root_transforms` in:

- `runRasterPipeline()`
- `runBilevelPipeline()`

Do not include them in:

- `runCoeffBilevelPipeline()`
- `runParamDebug()`
- `runCalculate()`

### Optional display

It is useful to show root transforms in:

- render log
- status text

But they should remain render metadata only, not compute metadata.

## Lambda Handler Work

### `handler_raster.py`

Add:

- `root_transforms = params.get("root_transforms", [])`

Pass the chain into `roots2pix`.

### `handler_bilevel.py`

In `phase=raster`, also accept:

- `root_transforms`

and pass them into `bilevel_raster`.

Do not route them through:

- `coeff_raster`
- `merge`
- `stitch`

They are needed only when canonical root stripes are being rasterized.

## Binary Interface

Recommended binary interface:

```text
--root_xforms=/tmp/root_xforms.json
```

Suggested file contents:

```json
[
  ["rotate_roots", "0.5"],
  ["pull_unit_circle", "0.75", "1.0"]
]
```

Using a JSON sidecar is better than squeezing nested arrays into shell flags.

## C Implementation Sketch

Shared type:

```c
typedef struct {
    char name[64];
    double args[8];
    int n_args;
} RootXformEntry;
```

Shared API:

```c
int parse_root_xform_chain(const char *json, RootXformEntry *entries, int max_count);
int apply_root_xform_chain(
    const RootXformEntry *entries, int n_entries,
    float *roots_re, float *roots_im, int degree);
```

Per-sample flow:

```c
decode one sample's roots
copy roots into working buffers
apply_root_xform_chain(...)
continue with raster projection
```

The key point is that the transformed roots exist only in the working buffer.

## Testing Plan

### 1. Pure transform tests

New file:

- `tests/test_root_transforms.py`

Cover:

- `rotate_roots(0)` = identity
- `rotate_roots(1)` = multiply by `-1`
- `rotate_roots(0.5)` = quarter-turn
- `pull_unit_circle` preserves angle
- `pull_unit_circle` moves radii near `1` toward `1`
- chain order is respected

### 2. Raster integration tests

Extend raster tests with cases where the same canonical root input is rendered:

- with no root transforms
- with `rotate_roots`
- with `pull_unit_circle`

and verify changed pixel output.

This is the most important integration proof, because root transforms are supposed to be applied in flight during tile creation.

### 3. Frontend / handler tests

Extend mocked tests to verify:

- `root_transforms` is sent only by render payloads
- `handler_raster.py` passes it through
- `handler_bilevel.py` passes it through only in `phase=raster`
- coeff render and param debug do not include it

## Rollout Plan

### Phase 1

Implement:

- Render-tab chip UI
- payload plumbing
- shared C parser / dispatcher
- `rotate_roots`
- `pull_unit_circle`
- bilevel root render integration

This gets the feature working quickly in the simpler render path.

### Phase 2

Add color render integration via `roots2pix`.

This is slightly more delicate because matching and color assignment must see transformed roots.

### Phase 3

Add more root transforms.

Once the shared root-transform engine exists, incremental additions should be cheap.

## Final Recommendation

The feature should be framed as:

- **Render-time root geometry transforms**

The right model is:

- Compute produces canonical roots and saves them once
- Render chooses an optional root transform pipeline
- Raster applies that pipeline in memory while creating tiles
- No transformed roots are saved back to S3

That is the clean separation for this feature, and it keeps experimentation fast without contaminating compute artifacts.
