# Better Error Messages Spec

This document specifies a systematic upgrade of user-visible error messages across Compute, Render, Palette, DeepZoom, Preview, and storage-backed artifact management.

The current system often surfaces one of these low-value messages:

- `Internal Server Error`
- `NoSuchKey`
- `Render failed`
- `Palette failed`
- `dispatch failed`
- raw Step Functions `Cause` blobs
- raw binary stderr with no phase/context

That is not enough. The user needs to know:

1. what phase failed
2. what exact artifact/key/input was missing or invalid
3. whether the failure is retryable
4. whether the failure is a legacy-compatibility mismatch
5. what concrete next action is sensible

This spec covers the current failure surfaces and prescribes better messages for each.

## 1. Goal

Every user-visible failure should become:

- phase-specific
- artifact-specific
- actionable
- stable enough to test

The browser log should answer:

- `what failed?`
- `where did it fail?`
- `what object/key/input was involved?`
- `is this a bad current run, a stale old job, or an infrastructure problem?`

## 2. Non-Goals

This spec does not require:

- changing the control flow of the workflows
- hiding raw technical detail
- adding a second error-reporting backend
- changing successful status behavior

This is an error-reporting refactor, not a workflow refactor.

## 3. Core Design Rules

### 3.1 Every raised runtime error must include phase context

Do not raise:

- `NoSuchKey`
- `raw2jpeg failed`
- `dispatch failed`

Raise:

- `Solve score hist failed: missing root chunk renders/{job_id}/chunk_{idx}.bin`
- `Encode failed: missing tile raw renders/{job_id}/tile_0007.raw`
- `DeepZoom export failed: missing source image renders/{job_id}/color/{artifact_id}/image.jpeg`

### 3.2 Missing S3 object errors must include the exact key

For every `s3.get_object()` failure that reaches the user, the message must include:

- the exact key requested
- the operation/phase that wanted it

### 3.3 Legacy-compatibility mismatches must be identified explicitly

Old jobs still exist.

If current code expects:

- `chunk_{idx}.bin`

and only old-style artifacts likely exist:

- `stripe_{idx}.bin`

the message must say so directly.

Example:

```text
Solve score hist failed: missing root chunk renders/compute_x/chunk_0.bin
legacy hint: older jobs may store hires roots as renders/compute_x/stripe_0.bin
```

### 3.4 Worker errors must survive Step Functions wrapping

The top-level Render/Palette status row must not collapse specific worker failures into:

- `Internal Server Error`
- `Lambda.Unknown`
- `States.TaskFailed`

The specific worker message must survive through:

- worker Lambda
- Step Functions `Cause`
- `handler_render_status.py`
- browser log/status line

### 3.5 User-facing messages need two levels

Each surfaced error should have:

1. short status text
2. detailed log text

Example:

- status line:
  - `Render error: missing root chunk`
- log line:
  - `Render failed: Raster failed: missing root chunk renders/compute_x/chunk_17.bin`

The status line must stay compact.
The log line should be detailed.

### 3.6 Do not “sanitize away” the real cause

Do not paper over root causes with:

- `failed to parse JSON`
- `unexpected response`
- `invalid output`

when the real issue is:

- missing S3 key
- invalid palette id
- stale legacy layout
- bins metric mismatch
- coverage mismatch
- timeout
- binary stderr

## 4. Message Shape

Where a handler raises `RuntimeError`, use this structure:

```text
<Phase> failed: <specific cause>
```

Examples:

- `Render plan failed: calc.json missing degree for compute_x`
- `Solve score clip failed: missing lores roots renders/compute_x/lores.bin`
- `Raster failed: missing solve-score bins renders/compute_x/solve_scores/anisotropy_bins.json`
- `Palette finalize failed: expected 16000000 pass-0 samples, assembled 15996412`

Where useful, append one hint line:

```text
legacy hint: older jobs may use stripe_*.bin instead of chunk_*.bin
```

or:

```text
hint: regenerate this artifact for the selected job
```

## 5. Failure Taxonomy And Exact Touch Points

## 5.1 Browser request/launch failures

### 5.1.1 Lambda URL/config failures

Current source:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
  - `lambdaUrl(...)`
  - `lambdaPost(...)`

Current weak messages:

- `Lambda URL not configured`
- `HTTP 500`
- `<name>: failed after 5 retries`

Required improvements:

- include route for storage helper calls
- include whether the failure was:
  - config missing
  - HTTP non-200
  - retry exhaustion

Desired messages:

- `storage /render-summary HTTP 500: <body>`
- `dispatch /dispatch-render failed after 5 retries`
- `Lambda URL not configured for "solve_proximity" (config.json missing or stale)`

## 5.1.2 Async dispatch rejection

Current source:

- [lambda/handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py)
- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Current weak messages:

- `Orchestrator dispatch failed`
- `Sweep dispatch failed`
- `dispatch failed: fired X/Y`

Required improvements:

- include target name
- include `non_202` details if present
- include first failed task hint

Desired messages:

- `Render dispatch failed: render_orchestrator fired 0/1`
- `DeepZoom dispatch rejected: polypaint-deepzoom-export returned 429`
- `Sweep dispatch failed: polypaint-sweep fired 47/50`

Implementation sites:

- [lambda/handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py)
- browser launchers in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

## 5.2 Render plan failures

Current source:

- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)

Important failure classes:

1. `calc.json` missing
2. viewport call failed
3. invalid palette
4. invalid quantile
5. plan too large
6. legacy compute metadata incompatible with current rerender path

Desired messages:

- `Render plan failed: calc.json missing for compute_x`
- `Render plan failed: viewport computation failed for compute_x`
- `Render plan failed: invalid palette tri_bogus`
- `Render plan failed: solve_score_quantile must be in [0.001, 0.05], got 0.5`
- `Render plan failed: state payload 260743 bytes exceeds 204800-byte safety limit`
- `Render plan failed: legacy job compute_x has n_stripes but no chunk metadata needed for current workflow`

Implementation sites:

- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- optionally [lambda/handler_render_status.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_status.py) for preserving the top-level message

## 5.3 Solve-score prepass failures

Current source:

- [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

### 5.3.1 Clip phase

Failure classes:

1. missing lores root file
2. invalid metric
3. invalid quantile
4. binary stderr / invalid JSON output

Desired messages:

- `Solve score clip failed: missing lores roots renders/compute_x/lores.bin`
- `Solve score clip failed: invalid metric anisotropyy`
- `Solve score clip failed: solve_score_quantile must be in [0.001, 0.05], got 0.5`
- `Solve score clip failed: solve_proximity_stats produced invalid JSON`

### 5.3.2 Hist phase

This is where the old-job compatibility problem currently shows up.

Failure classes:

1. missing hires root chunk
2. clip artifact missing
3. old job likely uses `stripe_*.bin`
4. binary failure

Desired messages:

- `Solve score hist failed: missing root chunk renders/compute_x/chunk_0.bin`
- `Solve score hist failed: missing clip artifact renders/compute_x/solve_scores/anisotropy_clip.json`
- `Solve score hist failed: missing root chunk renders/compute_x/chunk_0.bin\nlegacy hint: older jobs may use renders/compute_x/stripe_0.bin instead`
- `Solve score hist failed: solve_proximity_stats hist failed: <stderr>`

### 5.3.3 Merge phase

Failure classes:

1. missing one or more histogram shards
2. metric mismatch
3. quantile mismatch
4. malformed histogram artifact

Desired messages:

- `Solve score merge failed: missing histogram shard renders/compute_x/solve_scores/anisotropy/chunk_17_hist.json`
- `Solve score merge failed: histogram metric mismatch expected anisotropy got crowding`
- `Solve score merge failed: histogram quantile mismatch expected 0.05 got 0.001`
- `Solve score merge failed: chunk_3_hist.json missing hist array`

Implementation site:

- [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

## 5.4 Color raster failures

Current source:

- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)

Failure classes:

1. missing root chunk
2. missing solve-score bins artifact
3. bins family mismatch
4. bins metric mismatch
5. bins quantile mismatch
6. `roots2pix` stderr

Desired messages:

- `Raster failed: missing root chunk renders/compute_x/chunk_7.bin`
- `Raster failed: missing solve-score bins renders/compute_x/solve_scores/anisotropy_bins.json`
- `Raster failed: bins artifact family mismatch expected solve_score got palette`
- `Raster failed: bins metric mismatch expected anisotropy got crowding`
- `Raster failed: bins quantile mismatch expected 0.05 got 0.001`
- `Raster failed: roots2pix failed: <stderr>`

Legacy compatibility message:

- `Raster failed: missing root chunk renders/compute_x/chunk_7.bin\nlegacy hint: older jobs may use renders/compute_x/stripe_7.bin instead`

Implementation site:

- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)

## 5.5 Finalize failures

Current source:

- [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)

Failure classes:

1. all `.pix` inputs missing
2. some missing `.pix` files causing an empty tile
3. `pixassemble` failure
4. upload failure

Desired messages:

- `Finalize failed: no pix inputs found for tile 17 across 50 chunks`
- `Finalize failed: pixassemble failed for tile 17 (rc=1): <stderr>`
- `Finalize failed: upload failed for renders/compute_x/tile_0017.raw`

Important distinction:

Missing some chunk inputs is not always fatal if the tile legitimately had no hits.
The spec should only force an error when:

- no `.pix` inputs were found for a tile that was expected to exist, or
- the downstream assembler fails

Implementation site:

- [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)

## 5.6 Encode failures

Current source:

- [lambda/handler_encode.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_encode.py)

Failure classes:

1. missing tile raw
2. malformed raw header
3. `raw2jpeg` failure
4. upload failure
5. `/tmp` exhaustion or OOM symptoms

Desired messages:

- `Encode failed: missing tile raw renders/compute_x/tile_0007.raw`
- `Encode failed: malformed raw tile renders/compute_x/tile_0007.raw`
- `Encode failed: raw2jpeg failed: <stderr>`
- `Encode failed: upload failed for renders/compute_x/color/color_run_x/image.jpeg`
- `Encode failed: temporary file assembly exceeded Lambda /tmp or memory budget`

Implementation site:

- [lambda/handler_encode.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_encode.py)

## 5.7 BiLevel failures

Current source:

- [lambda/handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py)
- [lambda/handler_coeff_bilevel_stitch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeff_bilevel_stitch.py)

### 5.7.1 BiLevel raster

Failure classes:

1. missing root chunk
2. legacy root naming mismatch
3. `bilevel_raster` stderr

Desired messages:

- `BiLevel raster failed: missing root chunk renders/compute_x/chunk_3.bin`
- `BiLevel raster failed: missing root chunk renders/compute_x/chunk_3.bin\nlegacy hint: older jobs may use renders/compute_x/stripe_3.bin instead`
- `BiLevel raster failed: bilevel_raster failed: <stderr>`

### 5.7.2 Coeffs raster

Failure classes:

1. missing coefficient chunk
2. `coeffs_bilevel_raster` stderr

Desired messages:

- `Coeffs raster failed: missing coefficient chunk renders/compute_x/coeffs_0003.bin`
- `Coeffs raster failed: coeffs_bilevel_raster failed: <stderr>`

### 5.7.3 Merge

Failure classes:

1. no bitset inputs for tile
2. `bilevel_merge merge` failure

Desired messages:

- `BiLevel merge failed: no bitset inputs found for tile 17`
- `BiLevel merge failed: bilevel_merge merge failed: <stderr>`

### 5.7.4 Stitch

Failure classes:

1. missing tile TIFF
2. `bilevel_merge stitch` failure
3. preview generation failure

Desired messages:

- `BiLevel stitch failed: missing tile TIFF renders/compute_x/bilevel_t0017.tif`
- `BiLevel stitch failed: bilevel_merge stitch failed: <stderr>`
- `BiLevel stitch failed: preview generation failed: <stderr>`

Implementation sites:

- [lambda/handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py)
- [lambda/handler_coeff_bilevel_stitch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeff_bilevel_stitch.py)

## 5.8 Palette workflow failures

Current sources:

- [lambda/handler_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_render_plan.py)
- [lambda/handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
- [lambda/handler_palette_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_finalize.py)
- [lambda/handler_palette_debug.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_debug.py)

### 5.8.1 Palette plan

Failure classes:

1. invalid metric
2. invalid palette
3. invalid quantile
4. `calc.json` missing degree/N/lores bin
5. chunk metadata missing
6. plan too large

Desired messages:

- `Palette plan failed: invalid palette tri_bogus`
- `Palette plan failed: calc.json missing degree, N, or lores.bin_key for compute_x`
- `Palette plan failed: calc.json missing chunk metadata for compute_x`
- `Palette plan failed: state payload exceeds 200KB safety limit`

### 5.8.2 Palette chunk

Failure classes:

1. missing full-solve chunk
2. missing bins artifact
3. bins family/metric/quantile mismatch
4. binary stderr

Desired messages:

- `Palette chunk failed: missing root chunk renders/compute_x/chunk_12.bin`
- `Palette chunk failed: missing solve-score bins renders/compute_x/palettes/pal_x/solve_score/anisotropy_bins.json`
- `Palette chunk failed: bins metric mismatch expected anisotropy got crowding`
- `Palette chunk failed: solve_palette_chunk failed: <stderr>`

### 5.8.3 Palette finalize

Failure classes:

1. no chunk metadata
2. chunk score/bin length mismatch
3. out-of-bounds chunk coverage
4. incomplete pass-0 coverage
5. missing solve-score clip/bins side artifacts
6. `palette_bins_render` failure
7. `raw2jpeg` failure
8. preview generation failure

Desired messages:

- `Palette finalize failed: no chunk metadata found under renders/compute_x/palettes/pal_x/chunks/`
- `Palette finalize failed: chunk 3 score length 998 != 1000`
- `Palette finalize failed: chunk 3 writes out of pass-0 bounds at 16000000`
- `Palette finalize failed: assembled 15996000 pass-0 samples, expected 16000000`
- `Palette finalize failed: missing clip artifact renders/compute_x/palettes/pal_x/solve_score/anisotropy_clip.json`
- `Palette finalize failed: palette_bins_render failed: <stderr>`
- `Palette finalize failed: raw2jpeg failed: <stderr>`
- `Palette finalize failed: preview generation failed: <stderr>`

### 5.8.4 Legacy palette-debug path

Failure classes:

1. missing lores bin
2. invalid palette
3. binary stderr
4. preview generation failure

Desired messages:

- `Palette debug failed: missing lores roots renders/compute_x/lores.bin`
- `Palette debug failed: invalid palette tri_bogus`
- `Palette debug failed: solve_palette_debug failed: <stderr>`
- `Palette debug failed: preview generation failed: <stderr>`

## 5.9 DeepZoom failures

Current source:

- [lambda/handler_deepzoom_export.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_deepzoom_export.py)

Failure classes:

1. missing source image/TIFF
2. unsupported source format
3. `dz_export` stderr
4. upload failure
5. manifest/write failure

Desired messages:

- `DeepZoom export failed: missing source artifact renders/compute_x/bilevel/bilevel_run_y/image.tif`
- `DeepZoom export failed: unsupported source format for renders/compute_x/color/color_run_y/image.png`
- `DeepZoom export failed: dz_export failed: <stderr>`
- `DeepZoom export failed: upload failed for deepzoom/compute_x/dz_.../image.dzi`
- `DeepZoom export failed: could not write manifest deepzoom/compute_x/dz_.../meta.json`

Implementation site:

- [lambda/handler_deepzoom_export.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_deepzoom_export.py)

## 5.10 Storage and artifact-management failures

Current source:

- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)

Failure classes:

1. `/detail` calc/view missing
2. `/render-summary` list failure
3. delete partial failure
4. DDB cleanup partial failure
5. stale `preview_stats.json`

Desired messages:

- `Result detail failed: calc.json missing for compute_x`
- `Render summary failed: unable to list color artifacts for compute_x`
- `Delete failed: removed 14 objects, 2 deletions failed`
- `Cleanup warning: 7 DynamoDB task rows remained after retries`

Important note:

The browser should distinguish:

- hard failure
- partial cleanup warning

Not every storage problem should become a red fatal status.

## 5.11 Status extraction failures

Current source:

- [lambda/handler_render_status.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_status.py)

Current weak point:

- `_extract_error_message()` is generic and loses structured context

Required improvement:

If Step Functions `Cause` contains:

- `errorMessage`
- `task context`
- `Payload`

then preserve:

- worker phase
- specific key
- worker index (`chunk_idx`, `tile_idx`)

The status row should store the already-extracted readable message, not only a raw envelope.

## 5.12 Frontend observer failures

Current source:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Failure classes:

1. run vanished / no DDB row appears
2. worker poll error swallowed silently
3. generic `Refresh failed`
4. generic `Palette failed`
5. generic `Solve histogram failed`

Required improvements:

- when a run disappears after launch, show:
  - `Render failed: run row never appeared in DynamoDB`
- when polling fails repeatedly, show:
  - `Render status poll failing: storage /check-status HTTP 500`
- for histogram/palette actions:
  - status line should include the specific first-line cause, not only `failed`

Examples:

- `Solve histogram failed: missing root chunk`
- `Palette failed: bins metric mismatch`
- `Refresh failed: storage /render-summary HTTP 500`

## 6. Shared Helpers To Add

## 6.1 Shared S3 missing-key formatter

Add a small helper in [lambda/shared.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/shared.py), or a dedicated helper module:

```python
def missing_s3_key_message(phase, key, legacy_hint=None):
    ...
```

This avoids a hundred one-off `NoSuchKey` messages.

## 6.2 Shared legacy root-key hint helper

Given:

- `renders/{job_id}/chunk_{idx}.bin`

derive:

- `renders/{job_id}/stripe_{idx}.bin`

for hint text only.

This helper must never silently switch behavior.
It is for messaging, not fallback.

## 6.3 Shared binary-failure formatter

When subprocesses fail, format:

- binary name
- phase
- return code
- trimmed stderr

Example:

```text
Raster failed: roots2pix rc=1: invalid --solve_score_cuts
```

## 7. Browser Messaging Rules

## 7.1 Status line

Keep compact.

Good:

- `Render error: missing root chunk`
- `Palette error: invalid palette`
- `DeepZoom error: missing source image`

Bad:

- full 500-character traceback

## 7.2 Log line

Include full detail.

Good:

- `Render failed: Raster failed: missing root chunk renders/compute_x/chunk_17.bin`

## 7.3 Alerts/confirm-style actions

For delete failures and similar one-off actions, include:

- operation
- selected artifact id
- first concrete cause

Example:

- `Delete failed for color_177503...: S3 delete returned 2 errors`

## 8. Test Requirements

## 8.1 Handler unit tests

Add or extend tests for:

- [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)
- [tests/test_solve_proximity_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_handler.py)
- [tests/test_palette_chunk_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_chunk_handler.py)
- [tests/test_palette_finalize_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_finalize_handler.py)
- new handler tests for:
  - [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
  - [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)
  - [lambda/handler_encode.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_encode.py)
  - [lambda/handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py)
  - [lambda/handler_coeff_bilevel_stitch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeff_bilevel_stitch.py)
  - [lambda/handler_deepzoom_export.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_deepzoom_export.py)

Each test should assert the exact message text or an exact stable substring.

## 8.2 Frontend JS tests

Extend [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh) to verify:

1. histogram error surfaces the specific cause, not only `Solve histogram failed`
2. render observer surfaces the worker error message
3. palette observer surfaces the worker error message
4. refresh failures include route context

## 8.3 E2E tests

At least a few browser tests should simulate mocked error payloads and assert:

- status line text
- log text

Examples:

- missing legacy root chunk on old job
- solve-score bins mismatch
- DeepZoom missing source artifact

## 9. Rollout Order

Recommended implementation order:

1. `handler_render_status.py` message extraction preservation
2. S3 missing-key formatting in:
   - `handler_solve_proximity.py`
   - `handler_raster.py`
   - `handler_bilevel.py`
   - `handler_coeff_bilevel_stitch.py`
   - `handler_palette_chunk.py`
   - `handler_deepzoom_export.py`
3. binary stderr formatting
4. browser status/log wording cleanup
5. tests

## 10. Minimum Acceptable Outcome

After this refactor, the specific example that motivated the work:

```text
Render failed: An error occurred (NoSuchKey) when calling the GetObject operation: The specified key does not exist.
```

must instead look like:

```text
Solve score hist failed: missing root chunk renders/compute_mn5xlizz/chunk_0.bin
legacy hint: older jobs may use renders/compute_mn5xlizz/stripe_0.bin instead
```

If the system still collapses that into generic `NoSuchKey`, this spec has not been implemented correctly.
