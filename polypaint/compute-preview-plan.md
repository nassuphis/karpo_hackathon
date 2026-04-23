# Compute Preview Plan

## Objective

Add a **fast, ephemeral Compute preview** in the Compute tab so the user can:

- set a small `N-preview`
- click one preview action
- see a **grainy bi-level image** quickly
- iterate on:
  - param transforms
  - coefficient function
  - function params
  - coeff transforms

This preview is for **shape exploration before committing to a full compute**.

## Definition Of Done

From the Compute tab:

- there is a dedicated preview section
- the user can enter `N-preview`
- `times` is **not shown** for preview and is hardwired to `1`
- preview runs in **one synchronous Lambda call**
- preview produces a **black/white image** and shows it directly in the browser
- preview is **not saved** to S3, DynamoDB, Results, Render, Favorites, or DeepZoom
- preview reruns are cheap and disposable

## Recommended UX

### Placement

Add a new **Compute Preview** section:

- **below**:
  - param transforms
  - function
  - function params
  - coeff transforms
- **above**:
  - `Calculate-AE`
  - `Calculate-AE-MT`
  - `Calculate-CM`

This keeps preview tied to the pipeline definition and visually separate from durable compute.

### Controls

Recommended v1 controls:

- `N-preview`
- `Preview`

Recommended optional v1.1 control:

- `Solver`
  - `AE`
  - `AE-MT`
  - `CM`

### Recommendation On Solver Choice

There is one real design choice here.

Option A:

- one `Preview` button
- fixed preview solver, recommended `AE`

Pros:

- simplest UX
- exactly matches the “push a button and see a grainy image” goal

Cons:

- preview no longer matches `lores.bin` semantics when the later real compute uses `AE-MT` or `CM`

Option B:

- one `Preview` button
- plus a small `Solver` selector in the preview section

Pros:

- preview can match the chosen lores/full compute solver path
- still only one preview button

Cons:

- one more control

**Recommendation:** Option B.

Reason:

- current lores solve uses the same solver mode as the real compute workflow
- if preview is meant to stand in for lores experimentation, solver choice should be explicit
- this still preserves the “one button” experience

## Preview Image Behavior

### Output Type

Return a **PNG** to the browser, not a TIFF.

Semantics:

- visually bi-level
- black background
- white plotted roots
- browser displays it with `image-rendering: pixelated`

This is the right compromise:

- visually exactly what the user wants
- cheap to return synchronously
- no libvips required just to encode the preview image

### Viewport

Preview should **auto-fit** the computed roots.

Recommended policy:

- reuse current `compute_viewport_from_bin(...)` logic from [shared.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/shared.py)
- fixed preview viewport params in v1:
  - `quantile = 0`
  - `shim = 0.05`

Reason:

- Compute preview is exploratory
- there is no explicit render viewport in the Compute tab
- the user wants immediate whole-shape feedback, not exact final framing

### Resolution

Recommended v1 rule:

- preview raster size = `N-preview`

Reason:

- intuitive
- graininess directly tracks the control the user edits
- no second preview-size knob

Hard cap recommended:

- backend cap: `N-preview <= 1024` in v1

Reason:

- synchronous Lambda + API Gateway
- payload size stays comfortably bounded
- keeps preview feeling interactive

## Ephemeral Scope

Preview must be **fully disposable**.

It must **not** write:

- `calc.json`
- `params.bin`
- `coeffs_*.bin`
- `chunk_*.bin`
- `lores.bin`
- `preview.png`
- Results rows
- Render family artifacts
- Favorites rows
- DeepZoom exports
- DynamoDB task rows

Allowed temporary state:

- browser-only preview state
- Lambda `/tmp` files only

This means:

- rerunning preview replaces the browser image only
- page refresh loses the preview
- preview does not affect any durable pipeline state

## Proposed Backend Shape

### New Lambda

Add a new synchronous handler:

- [lambda/handler_compute_preview.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_preview.py)

New direct API route:

- `POST /compute-preview`

New config service:

- `compute-preview`

### Why Not Step Functions / DDB / Dispatch

Do **not** run preview through:

- dispatch Lambda
- Step Functions
- DDB polling

Reason:

- preview is deliberately small
- the whole point is lower overhead and instant feedback
- orchestration cost would dominate the actual work

## Proposed Preview Pipeline

One synchronous Lambda call does:

1. validate request
2. generate coeffs for `N-preview * N-preview * 1`
3. solve roots in one local solver invocation
4. compute auto viewport from the solved roots
5. raster roots to a black/white preview buffer
6. encode PNG in memory
7. return the image bytes inline plus stats

### Input Payload

Preview request should include:

- `solver_mode`
- `N_preview`
- `function`
- `param_transforms`
- `coeff_transforms`
- `cfpv`

Nothing else is needed for v1.

### Internal Files

Use `/tmp` only:

- `/tmp/preview_coeffs.bin`
- `/tmp/preview_roots.bin`

No S3 staging.

### Why `/tmp` Instead Of Pure In-Memory Pipes

The current native tools are file-oriented:

- coeffgen writes to an output path
- solve reads a coeff path and writes a roots path

So the pragmatic v1 is:

- one Lambda
- no durable storage
- `/tmp` only

That is still operationally “ephemeral,” even if not literally pipe-only.

## Binary / Layer Consequences

This feature must support the current compute pipeline faithfully, including:

- param transforms
- function params
- coeff transforms
- `roots_cm`

That has consequences.

### Coeffgen Binary Choice

Do **not** use plain `sweep` for preview coeffgen.

Use:

- `sweep_coeffgen`

Reason:

- current compute coeffgen path already moved there
- `roots_cm` coeff transform requires LAPACK-backed coeffgen support
- preview must not silently diverge from real compute

### Solver Binary Choice

Use the same single-Lambda solver binaries already shipped:

- `AE` -> `sweep`
- `AE-MT` -> `sweep_mt`
- `CM` -> `sweep_cm`

### Required Layer

Attach the LAPACK layer to the preview Lambda.

Reason:

- `sweep_coeffgen`
- `CM`
- `roots_cm`

all depend on the LAPACK/OpenBLAS runtime path being available.

## Rendering Strategy

Recommended v1 rendering path:

- reuse the pure-Python preview raster logic from [handler_preview.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_preview.py)
- do **not** invoke render/finalize/encode
- do **not** invoke bilevel render workflows

Why:

- preview image is tiny
- no need for render fan-out
- no need for TIFF
- existing preview raster is already the right shape:
  - compute viewport from roots
  - plot white points
  - encode PNG

This should be a **single-pass in-process raster** after solve.

## Browser Response Shape

Recommended response body:

- `solver_mode`
- `N_preview`
- `degree`
- `n_roots_total`
- `n_roots_in_view`
- `coeffgen_ms`
- `solve_ms`
- `viewport_ms`
- `raster_ms`
- `total_ms`
- `image_width`
- `image_height`
- `image_png_base64`

### Why Inline Image Instead Of S3

Because preview is ephemeral.

Returning the PNG inline means:

- no S3 writes
- no presigned URLs
- no cleanup
- immediate display

Constraint:

- keep the PNG comfortably below API Gateway/Lambda sync payload limits

That is why `N-preview` needs a cap.

## UI State And Staleness

Preview must become **stale** whenever any compute-defining control changes:

- `N`
- `N-preview`
- param transforms
- function
- function params
- coeff transforms
- preview solver mode, if added

Recommended behavior:

- keep showing the old preview image
- mark it `stale`
- clear the stale marker only after a successful rerun

Reason:

- avoids blanking the preview too aggressively
- makes it obvious the image no longer matches the current pipeline

## Error Reporting

Preview errors must be explicit and contextual.

If preview fails, the UI should show:

- phase:
  - `coeffgen`
  - `solve`
  - `viewport`
  - `raster`
  - `encode`
- solver mode
- `N-preview`
- function name
- coeff transform chain
- temp-file size estimates when relevant

Examples:

- `Compute preview failed at solve: AE-MT, N-preview=800, degree=71, estimated roots tmp=363 MB`
- `Compute preview failed at coeffgen: roots_cm requires LAPACK-backed coeffgen`

No bare strings like:

- `failed`
- `Internal Server Error`

## Guardrails

### Hard Input Limits

Recommended v1 backend limits:

- `N-preview <= 1024`
- `times = 1` always
- reject if temp-file estimate is too large

### Temp Size Estimates

Approximate temp usage:

- coeffs tmp:
  - `N_preview^2 * n_coeffs * 8`
- roots tmp:
  - `N_preview^2 * degree * 8`

Since `degree` and `n_coeffs` are only known after coeffgen, enforce in two stages:

1. preflight on `N-preview`
2. exact guard after coeffgen returns metadata

### Timeout Budget

Because this is synchronous:

- target wall time should be single-digit seconds
- absolute preview budget should stay well under API Gateway timeout

If preview is estimated to be too big:

- fail fast
- tell the user to lower `N-preview`

## Recommended UI Layout

Recommended preview block:

- row 1:
  - `N-preview`
  - optional `Solver`
  - `Preview` button
- row 2:
  - preview status summary
- row 3:
  - preview image panel

Image panel behavior:

- fixed viewer box
- black background
- `<img>` with `image-rendering: pixelated`
- shows placeholder before first run:
  - `No compute preview yet`

## Implementation Order

### Phase 1: Core Preview

- add new preview section in Compute tab
- add `N-preview`
- add preview image box
- add synchronous preview Lambda + API route
- use:
  - `sweep_coeffgen`
  - selected solver binary
  - pure-Python PNG raster path
- return inline PNG

### Phase 2: Robustness

- stale-preview invalidation
- detailed errors by phase
- guardrails for oversize preview runs
- solver selector, if not included in Phase 1

### Phase 3: Polish

- small metadata line under image:
  - solver
  - degree
  - total roots
  - elapsed
- optional “rerun last preview” keyboard shortcut

## Files Expected To Change

Frontend:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Backend:

- [lambda/handler_compute_preview.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_compute_preview.py)
- [lambda/shared.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/shared.py) if preview helpers are moved there

Deploy / routing:

- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- [api_manifest.json](/Users/nicknassuphis/karpo_hackathon/polypaint/api_manifest.json)

Tests:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- new handler tests for compute preview
- [tests/test_deploy_packaging.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deploy_packaging.py)
- [tests/test_api_route_contracts.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_api_route_contracts.py)
- [tests/docker_runtime_regression.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/docker_runtime_regression.py)

## Main Risks

### 1. Preview Diverges From Real Compute

Risk:

- preview coeffgen or solver path differs from the real compute path

Mitigation:

- use `sweep_coeffgen`
- use the same solver binaries
- keep serialization identical to `runCalculateWithSolver(...)`

### 2. Preview Feels Slow

Risk:

- synchronous preview exceeds the “instant feedback” goal

Mitigation:

- keep `N-preview` capped
- use one Lambda, no orchestration
- times fixed to `1`
- no S3 writes

### 3. Preview Pollutes Durable Pipeline State

Risk:

- stray objects or rows appear in Results/Render

Mitigation:

- no S3 output
- no DDB task rows
- no job_id allocation

### 4. Payload Too Large

Risk:

- inline PNG response gets too large

Mitigation:

- cap `N-preview`
- return only one image

## Recommended v1 Decision Summary

Build:

- a **new direct synchronous compute-preview Lambda**
- with:
  - `N-preview`
  - one `Preview` button
  - recommended explicit preview solver selector
- using:
  - `sweep_coeffgen`
  - existing solver binaries
  - pure-Python preview PNG raster
- storing:
  - nothing durable
- returning:
  - inline PNG + timing/stats

That is the shortest path to the actual user goal:

- press button
- see grainy image
- iterate fast
