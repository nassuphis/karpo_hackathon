# Palette Render Workflow

Status: implemented.

This document describes the current asynchronous Palette workflow.

## Goal

The Palette tab now computes a true full-resolution parameter-space palette artifact.

It is no longer a synchronous lores-expanded debug action.

Current behavior:

- clip range is estimated from lores
- histogram and merged cuts are computed from the full solve
- chunk workers compute exact pass-0 scores and bins on the full solve
- finalize assembles exact `N x N` sidecars and the final image
- the workflow is asynchronous and tracked through Step Functions + DynamoDB status rows

## User-Facing Behavior

Both of these entrypoints use the asynchronous palette workflow:

- the Palette tab `Palette` / `Generate` action
- the Render tab `Palette` family `Generate` action

The browser tracks the active palette run in `_activePaletteRun` and refreshes:

- the Palette inventory
- the Render `palette` family catalog

when the workflow completes.

## Inputs

Workflow input:

- `job_id`
- `metric`
- `palette`
- `solve_score_quantile`
- `root_transforms`

The workflow loads the rest from `calc.json`.

## Exact Semantics

The authoritative semantics are:

- full solve
- exact `N x N`
- pass `0` only
- same solve-score clip / hist / merge math used by solve-score render

The visible palette image is derived from exact full-resolution `palette_bins.bin`.
It is not a lores image expanded to `N x N`.

## Workflow Components

### Orchestrator

- [lambda/handler_palette_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_orchestrator.py)

Starts the Step Functions workflow and writes the top-level DDB run row.

### Plan

- [lambda/handler_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_render_plan.py)

Responsibilities:

- load `calc.json`
- validate metric / palette / `q`
- compute immutable `palette_id`
- build exact chunk items using `step_start` and `step_count`
- define output keys
- enforce the Step Functions plan size limit

### Full-Solve Solve-Score Phases

Reuses:

- [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

Phases:

1. `clip`
2. `hist`
3. `merge`

### Palette Chunk Worker

- [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
- binary: [lambda/solve_palette_chunk](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_chunk)

Each worker:

- downloads one full-solve chunk
- downloads merged solve-score bins metadata
- computes exact pass-0 scores and final bins for that chunk span
- uploads:
  - chunk score binary
  - chunk palette-bin binary
  - chunk metadata JSON

### Finalize

- [lambda/handler_palette_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_finalize.py)
- binary: [lambda/palette_bins_render](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_bins_render)

Finalize:

- reassembles exact full-resolution pass-0 data
- applies serpentine deshuffle to final row-major order
- writes:
  - `score_<metric>.bin`
  - `palette_bins.bin`
  - `image.jpeg`
  - `preview.png`
  - `meta.json`
- deletes only the workflow’s temporary chunk and solve-score intermediate objects

## Output Layout

Each successful run writes:

- `renders/{job_id}/palettes/{palette_id}/image.jpeg`
- `renders/{job_id}/palettes/{palette_id}/preview.png`
- `renders/{job_id}/palettes/{palette_id}/score_<metric>.bin`
- `renders/{job_id}/palettes/{palette_id}/palette_bins.bin`
- `renders/{job_id}/palettes/{palette_id}/meta.json`

Temporary workflow artifacts are written under:

- `renders/{job_id}/palettes/{palette_id}/chunks/`
- `renders/{job_id}/palettes/{palette_id}/solve_score/`

and removed after a successful finalize.

## Metadata

`meta.json` currently records:

- `job_id`
- `palette_id`
- `created_at`
- `display_name`
- `metric`
- `palette`
- `solve_score_quantile`
- `root_transforms`
- `degree`
- `N`
- `times`
- `using_pass`
- `clip_lo`
- `clip_hi`
- `cuts_norm`
- `clip_fallback`
- `clip_fallback_reason`
- `file_size`
- `image_key`
- `preview_key`
- `score_key`
- `palette_bins_key`

`using_pass` is `0` in the current implementation.

## State Machine

Workflow definition:

- [stepfunctions/palette_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/palette_workflow.asl.json.template)

High-level phases:

1. `Clean + plan`
2. `Solve score: clip`
3. `Solve score: hist`
4. `Solve score: merge`
5. `Palette: chunk`
6. `Palette: finalize`
7. `Done`

The state machine is wrapped in a top-level `Parallel` to catch `States.Runtime` parameter-resolution failures.

## Lifetime Rules

Palette runs are immutable.

Generating a new palette:

- creates a new `palette_id`
- does not delete previous palette runs
- does not delete render family artifacts

Explicit deletion uses:

- `POST /delete-palette`

## Tests

Current dedicated tests:

- [tests/test_palette_workflow_definition.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_workflow_definition.py)
- [tests/test_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_render_plan.py)
- [tests/test_palette_chunk_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_chunk_handler.py)
- [tests/test_palette_finalize_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_finalize_handler.py)
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

## Relationship To `handler_palette_debug.py`

[lambda/handler_palette_debug.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_debug.py) still exists, but it is no longer the main UI workflow for the Palette tab.

The authoritative Palette-tab behavior is the asynchronous workflow described here.
