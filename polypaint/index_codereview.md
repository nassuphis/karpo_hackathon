# Code Review: `index.html`

## Findings

1. High: `times` is dropped from the lores coeffgen request, so previews and auto-viewporting are wrong for repeated runs.
   - `runCalculate()` passes `times` to the main coeffgen stripes at `index.html:2385-2393`, but the lores coeffgen call at `index.html:2415-2423` omits it.
   - The backend defaults missing `times` to `1` in `lambda/handler_coeffgen.py:47`, and `tests/test_sweep_smoke.py:60-87` shows that changing `times` materially changes the generated coefficient volume.
   - Because preview generation and viewport fitting are driven from `lores.bin`, any run with `Times > 1` can show a preview and render framing for a different polynomial than the one actually solved.

2. High: `render-stripes` is never clamped to `N`, so small grids can dispatch hundreds of empty jobs.
   - `nStripes` is accepted directly at `index.html:2354`, then `rowsPer = Math.floor(n / nStripes)` is computed at `index.html:2374`.
   - When `n < nStripes`, the first `nStripes - 1` coeffgen/sweep jobs are built with `i1_start === i1_end` in `index.html:2382-2393` and `index.html:2447-2456`.
   - The UI allows values like `N=10, Stripes=500`, which turns into 499 no-op Lambda invocations and one real stripe. That wastes capacity and makes progress reporting misleading. Clamp `nStripes` to `n` before building jobs.

3. Medium: `previewResult()` is racy and can attach a completed preview to the wrong selection.
   - The request is started with the current `_selectedJobId` at `index.html:1420-1422`, but after the `await` the code looks up and updates the cache/UI using the current global `_selectedJobId` again at `index.html:1427-1447`.
   - If the user clicks a different row before the preview Lambda returns, the old response can overwrite the newly selected row's cached preview URL and viewport fields.
   - Capture the job id in a local variable before the request and ignore stale responses unless that id is still selected.

4. Medium: chip parameter rendering is an injection sink because raw user input is written into `innerHTML`.
   - `_renderChips()` rebuilds the chip list with `innerHTML` at `index.html:1694-1704`.
   - The parameter value is interpolated directly into `value="${val}"` at `index.html:1698`, and that value comes from a free-form text input.
   - Entering quotes or markup in a chip parameter can break the DOM and execute HTML/JS in-page. Build the nodes with DOM APIs or escape attribute values before inserting them.
