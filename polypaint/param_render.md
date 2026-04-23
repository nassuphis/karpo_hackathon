# Param Render

This document describes a debug render mode for visualizing the param transform pipeline itself.

It is not a root render.
It is not a coefficient render.
It is a direct render of the transformed parameter pairs.

## Goal

Add a debug-only `Param` action near the `Calculate` button that:

- runs only the param transform pipeline
- treats the transformed `(z1, z2)` values as complex points
- renders them as a bilevel image
- produces a downloadable image for local magnified inspection

This is intended to help understand what the selected param transforms are doing before coefficient generation or root solving.

## What It Renders

The input grid is real:

- `x1`
- `x2`

The param transform pipeline maps those real grid coordinates into transformed complex parameters.

So for each grid step, the renderable output is:

- one complex value for parameter 1
- one complex value for parameter 2

That means the debug image is just a plot of the transformed parameter pairs in the complex plane.

Depending on how you want to visualize it, there are two natural modes:

1. Plot both transformed parameters into one image.
2. Plot one image per transformed parameter.

For debugging, the second option is usually clearer.

Recommended outputs:

- `param1_debug.tif`
- `param2_debug.tif`

Or if you prefer one combined output:

- `param_debug.tif`

## Why This Should Not Live In Render

This mode is conceptually tied to the Calculate tab, not the Render tab.

Reasons:

- it does not depend on solved roots
- it does not depend on coefficient files
- it is a diagnostic for param transforms
- the user wants it while shaping the compute setup, not after rendering results

So the right UI is probably:

- keep `Render`, `BiLevel`, `Coeff` in the Render tab
- add `Param` next to `Calculate` in the Compute tab

That keeps the mental model clean:

- Compute tab debug tools stay near the transform controls
- Render tab remains about final output images

## Proposed Backend Shape

This should be a dedicated Lambda path, separate from root and coeff rendering.

Suggested flow:

1. build the real `(x1, x2)` grid
2. apply the selected param transforms
3. emit transformed complex values
4. compute viewport from those values
5. raster to bilevel image
6. return downloadable artifact

Since this is a debug tool, the simplest architecture is better than maximum reuse.

## Simplest Implementation

Use one dedicated Lambda that does everything:

1. generate transformed parameter points
2. compute bounds / viewport
3. raster them directly
4. write one bilevel TIFF
5. upload and return a presigned URL

That avoids:

- stripe fanout
- merge phase
- stitch phase
- extra DynamoDB orchestration

This is appropriate because:

- the debug image does not need the full giant-render architecture
- it is for inspection, not for the main production render pipeline
- the output can be moderate resolution

## Recommended Output Size

For debugging, keep this moderate:

- `2048x2048`
- `4096x4096`

That is enough to inspect transform behavior when zoomed locally, while keeping the Lambda simple and fast.

If you later want very large param debug renders, you can promote it into a sectioned pipeline, but that should not be the first implementation.

## Data Model

The render source is not:

- roots
- coefficients

It is:

- transformed parameter values

So the Lambda only needs:

- `n1`, `n2`
- selected param transforms
- any transform parameters
- requested debug image size

For each grid point:

1. compute the raw parameter pair from `(x1, x2)`
2. apply the selected param transforms
3. emit the resulting complex values

Then raster them as points.

## Viewport

Viewport can be computed inline during the same Lambda invocation.

Because this is a debug mode, the simplest approach is:

1. first pass over generated transformed points:
   - compute min/max or quantile-trimmed bounds
2. second pass:
   - raster them

If the point count is small enough, you can also keep them in memory and avoid regenerating.

Because this is only for debug use, a simple two-pass design is acceptable.

## Suggested Binary / Lambda Design

### Option A: One Python Lambda

Implement the whole debug render in Python.

Pros:

- fastest to build
- easy to experiment with
- fine for moderate image sizes

Cons:

- slower if point counts get large

This is probably the right first version.

### Option B: Small C helper

If the transform-point count is large enough that Python feels slow, add a small C raster helper later.

That should not be the first step.

## Output Format

Use bilevel TIFF, same as the new bilevel pipeline.

Reasons:

- easy to inspect locally
- good for large sparse point plots
- avoids PNG compression bottlenecks
- consistent with the debug/analysis use case

Suggested output key:

- `renders/{job_id}/param_debug.tif`

Or, if split by parameter:

- `renders/{job_id}/param1_debug.tif`
- `renders/{job_id}/param2_debug.tif`

## UI Proposal

Add a button near `Calculate`:

- `Param`

Behavior:

1. read the current param transform selector state
2. launch the param debug Lambda
3. poll for completion if async, or wait directly if sync
4. show a download link

This should not alter:

- computed coeff files
- solved root files
- render tab state

It is a standalone inspection tool.

## Sync vs Async

For the first implementation, this can likely be synchronous.

Reason:

- no stripe fanout
- no large merge
- no final stitch
- debug image sizes should stay moderate

So a single API call returning a presigned URL is probably enough.

If later it grows into a larger pipeline, it can be moved to the async dispatch + polling model.

## Recommended First Cut

1. Add a `Param` button next to `Calculate`.
2. Create a dedicated debug Lambda.
3. Generate transformed parameter points directly from the selected param transforms.
4. Compute viewport inline.
5. Raster to a single bilevel TIFF.
6. Return a downloadable artifact.

Keep it intentionally simple.

This is a debug visualization tool, not a production render path.

## Optional Nice-To-Haves

- separate output for parameter 1 and parameter 2
- overlay both parameters in different colors later, if you ever want a color debug mode
- configurable debug resolution
- optional square/manual viewport override
- optional point count / clipping stats in the response

## Recommendation

Do not fold this into the existing Render tab pipeline.

Treat it as a Calculate-tab debug tool with its own dedicated Lambda and a small, direct implementation.

That is the cleanest fit for what you want to inspect.
