# Code Review: Rendering Lambdas

Status: historical review snapshot. Some findings still reference pre-refactor route usage and older render artifact behavior. Use [docs/lambdas.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/lambdas.md) for the current handler/reference view.

## Findings

1. High: `handler_dispatch` reports rejected async invokes as "fired", so the render pipeline can wait forever on jobs AWS already refused.
   - `_invoke_one()` treats any non-`202` Lambda invoke response as a problem at `lambda/handler_dispatch.py:42-47`.
   - But `handler()` still increments `fired` for those non-`202` cases at `lambda/handler_dispatch.py:73-79`.
   - The frontend/render orchestration uses `fired == batch.length` as the success condition, so a batch with `429`/`5xx` invoke responses can still be treated as successfully dispatched. That leaves the browser polling DynamoDB for raster/finalize/encode tasks that were never accepted by Lambda.

2. ~~High~~ **Fixed**: rerender cleanup previously left stale DynamoDB task rows behind.
   - `clean-render` now retries `batch_write_item()` up to 4 attempts with exponential backoff, checks `UnprocessedItems` on each attempt, and returns `ddb_errors` in the response instead of silently swallowing failures (`lambda/handler_storage.py:349-360`).
   - The risk of stale rows breaking progress polling remains in principle (task IDs are still deterministic and reused), but the retry logic makes partial cleanup failures unlikely in practice.

3. High: `handler_encode` does not scale to the image sizes the UI permits; it can exceed both Lambda memory and the configured `/tmp` budget.
   - The handler builds a full stitched raw image at `/tmp/encode_in.raw` at `lambda/handler_encode.py:32-65`.
   - It also reads every tile in the current tile row fully into memory before writing that row at `lambda/handler_encode.py:48-65`.
   - The deployed Lambda only gets `1769 MB` RAM and `10240 MB` of ephemeral storage in `deploy.sh:33` and `deploy.sh:41`.
   - That means large multi-tile renders can fail in two ways:
     - Wide images can OOM during row stitching because one row may contain many full tile payloads in memory at once.
     - Large images can run out of `/tmp` because the stitched raw file is `width * height * 3 + 12` bytes before encoding even starts.

4. Medium: `handler_raster` downloads the entire stripe blob into Python memory before writing it to disk, which creates an avoidable memory cliff on large renders.
   - The code does `obj["Body"].read()` and then writes the whole result to `/tmp/stripe.bin` at `lambda/handler_raster.py:34-38`.
   - Stripe result files can be large for high-`N`, high-degree jobs, so this duplicates the blob in memory before `roots2pix` even starts.
   - The rest of the render pipeline is explicitly written to stream large files; this handler is the outlier and is the most likely place to OOM first on large stripe inputs.

## Testing Gap

- There are no handler-level tests for `handler_raster.py`, `handler_finalize.py`, `handler_encode.py`, or `handler_viewport.py`. These are the main render handlers — their failure modes are contract and scale bugs, not syntax bugs.
- Dispatch and storage behavior is now well-covered: `tests/test_pipeline.py` tests dispatch, storage list/check-keys/clean-render/presign, shared utilities, coeffgen, sweep, and preview. `tests/test_dispatch_resilience.py` adds 28 tests covering `return_ids`, `/head-keys`, bilevel dispatch targets, missing-task detection logic, and wave dispatch calculation.
- The bilevel pipeline has binary-level tests (`test_bilevel_raster.py`, `test_bilevel_stitch.py`) and export tests (`test_tiff_compat.py`, `test_png_export.py`), plus the deploy.sh Docker smoke tests that exercise dz_export, png_export, and tiff_compat against the actual layer.
