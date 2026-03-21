# Code Review: Rendering Lambdas

## Findings

1. High: `handler_dispatch` reports rejected async invokes as "fired", so the render pipeline can wait forever on jobs AWS already refused.
   - `_invoke_one()` treats any non-`202` Lambda invoke response as a problem at `lambda/handler_dispatch.py:42-47`.
   - But `handler()` still increments `fired` for those non-`202` cases at `lambda/handler_dispatch.py:73-79`.
   - The frontend/render orchestration uses `fired == batch.length` as the success condition, so a batch with `429`/`5xx` invoke responses can still be treated as successfully dispatched. That leaves the browser polling DynamoDB for raster/finalize/encode tasks that were never accepted by Lambda.

2. High: rerender cleanup can leave stale DynamoDB task rows behind, which breaks progress polling because task IDs are reused for the same `job_id`.
   - `clean-render` tries to delete all status rows for the job at `lambda/handler_storage.py:290-319`, but it never checks `UnprocessedItems` from `batch_write_item()` and never retries partial failures.
   - It also swallows cleanup exceptions entirely at `lambda/handler_storage.py:320-321`.
   - The render Lambdas reuse deterministic task IDs on every rerender of the same job: `raster_{stripe}` in `lambda/handler_raster.py:29`, `tile_{tile}` in `lambda/handler_finalize.py:33`, and `encode` in `lambda/handler_encode.py:26`.
   - If any old rows survive cleanup, `/check-status` can report stale `done` results for the new render and let the client advance before the new raster/finalize/encode work has actually completed.

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

- I did not find handler-level tests for `handler_raster.py`, `handler_finalize.py`, `handler_encode.py`, or `handler_viewport.py`. The documented Lambda unit coverage in [docs/testing.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/testing.md#L72) lists dispatch, storage, shared utilities, coeffgen, sweep, and preview, but not the main render handlers.
- That gap matters here because the failure modes above are mostly contract and scale bugs, not syntax bugs.
