# Testing

## Testing Strategy

PolyPaint uses four test layers. They are meant to catch different failure modes:

1. **Native/binary regression tests**
   - Catch math drift, file-format breakage, and runtime behavior in C binaries.
   - Examples: `test_sweep_smoke.py`, `test_param_dump.py`, `test_bilevel_raster.py`, `test_companion_matrix.py`.

2. **Python handler and workflow tests**
   - Catch Lambda contract bugs, storage/reporting mistakes, Step Functions payload drift, and packaging regressions.
   - Examples: `test_pipeline.py`, `test_render_plan.py`, `test_render_workflow_definition.py`, `test_deploy_packaging.py`.
   - This layer is also where metadata-contract drift must be caught.
     If an artifact moves fields from S3 object headers into sidecar JSON or
     `meta.json`, tests in this layer should prove that readers still work for:
     - legacy header-only artifacts
     - current header + sidecar/overlay artifacts

3. **Frontend VM tests**
   - Execute `index.html` JavaScript in a Node VM without a browser.
   - Catch missing functions, stale IDs, payload-shape regressions, popup logic mistakes, and UI contract drift quickly.
   - Main file: `test_frontend_js.sh`.

4. **Playwright browser tests**
   - Catch real DOM/browser regressions: buttons disappearing, popups not opening, selection/focus behavior, wiring between controls and dispatched payloads.
   - These are the tests that should fail when a visible UI element silently disappears.

The rule of thumb is:

- **VM tests** prove UI logic exists.
- **Playwright tests** prove the user can actually reach and use it in a browser.
- **Handler/workflow tests** prove the backend contract is still coherent.
- **Native tests** prove the math/runtime layer is still correct.

## Hard Rule: Rebuild Deploy Binaries Before Docker Tests

If a change touches C/C++ code or build flags for a binary that is shipped in
`lambda/`, do not run the Docker runtime gate against stale artifacts.

Required order:

1. rebuild the affected deploy binary or binaries in `lambda/`
2. then run `bash scripts/test-docker-runtime.sh`

Do not rely on the Docker test to tell you the mounted binary is stale. That is
operator error, not a meaningful regression.

Examples:

- change [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)
  - rebuild at least:
    - `lambda/sweep`
    - `lambda/sweep_coeffgen`
- change [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)
  - rebuild `lambda/solve_proximity_stats`
- change [lambda/solve_palette_chunk_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_chunk_mt.c)
  - rebuild `lambda/solve_palette_chunk_mt`

If the binary is built through the Docker/LAPACK/libvips path in
[deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh), use that
same path before the Docker gate so the runtime test exercises the current
artifact.

## Test Location

All tests live in `polypaint/tests/`.

| File | What it tests | Requires |
|------|--------------|----------|
| `test_sweep_smoke.py` | sweep binary: coeffgen, solve, grid, dither uniqueness | `sweep_test` compiled |
| `test_ae_mt.py` | AE-MT native regression: `sweep_mt` matches single-thread AE exactly at `n_threads=1`, restores the single-thread warm-start benefit, and rejects `match_roots=true` | local `cc` toolchain |
| `test_poly_accuracy.py` | Transpiled C poly functions match Python originals | `sweep_test` compiled, numpy |
| `test_poly164_hand.py` | Dedicated parity regression for the `poly_164` hand override | `sweep_test` compiled, numpy |
| `test_pipeline.py` | Lambda handlers: dispatch, storage, coeffgen, sweep (solve-only), preview | Python mocks only |
| `test_chunking.py` | Chunked pipeline: param_gen, coeffgen_chunked, chunk planner, chunks>N | `sweep_test` compiled |
| `test_bilevel_raster.py` | bilevel_raster: byte-exact bitset comparison vs Python | `bilevel_raster_local` + `sweep_test` |
| `test_bilevel_stitch.py` | bilevel_merge: merge (OR bitsets → TIFF) and stitch (join tiles) | `bilevel_merge_local` (needs libvips) |
| `test_dither.py` | sdith, ddith, ndith: bounds, scaling, isotropy, statistics | `sweep_test` compiled |
| `test_param_dump.py` | param_dump mode: identity, unit_circle, rtheta, swap, chains | `sweep_test` compiled |
| `test_visual_1_100.py` | Visual comparison poly_1-100 (C vs Python pixel overlap) | `sweep_test` compiled, numpy |
| `test_visual_101_200.py` | Visual comparison poly_101-200 | `sweep_test` compiled, numpy |
| `test_visual_201_300.py` | Visual comparison poly_201-300 | `sweep_test` compiled, numpy |
| `test_visual_301_500.py` | Visual comparison poly_301-500 | `sweep_test` compiled, numpy |
| `test_visual_501_600.py` | Visual comparison poly_501-600 | `sweep_test` compiled, numpy |
| `test_visual_601_700.py` | Visual comparison poly_601-700 | `sweep_test` compiled, numpy |
| `test_visual_701_800.py` | Visual comparison poly_701-800 | `sweep_test` compiled, numpy |
| `test_visual_801_821.py` | Visual comparison poly_801-821 | `sweep_test` compiled, numpy |
| `test_tiff_compat.py` | tiff_compat: tiled→strip TIFF conversion | `tiff_compat_local` (needs libvips+libtiff) |
| `test_png_export.py` | png_export: TIFF→1-bit PNG conversion | `png_export_local` (needs libvips) |
| `test_dispatch_resilience.py` | Dispatch resilience: return_ids, head-keys, wave dispatch, missing-task detection | Python mocks only |
| `test_tri_palette_generation.py` | TRI palette generator + generated catalog consistency | Python only |
| `test_companion_matrix.py` | sweep_cm binary: exact roots, degenerate cases, overflow handling, AE vs CM comparison | Docker ARM64 + LAPACK |
| `test_solve_proximity_stats.py` | solve_proximity_stats binary: solve-score metrics, summary/hist modes, score-program parsing, mixed-source (`slv`/`cf`/`pm`) clipping, quantile clipping | Docker ARM64 |
| `test_solve_palette_debug.py` | solve_palette_debug binary: serpentine, expansion, palette selection, quantile | Docker ARM64 |
| `test_palette_debug_handler.py` | Palette debug Lambda handler: validation, S3 upload, stale preview deletion | Python mocks only |
| `test_render_workflow_definition.py` | Step Functions ASL template: required states, Map concurrency, retry policies | JSON parsing only |
| `test_render_orchestrator.py` | Render orchestrator starter Lambda: validation, DDB write, SFN StartExecution | Python mocks only |
| `test_render_plan.py` | Render plan Lambda: viewport, tile/chunk computation, output keys, size limit | Python mocks only |
| `test_raster_mt.py` | MT raster Lambda: solve-range splitting, worker output merge, saved-palette bin slicing, perf/result_data contract | Python mocks only |
| `test_render_status.py` | Render status Lambda: phase reporting, error extraction, updated_at_ms | Python mocks only |
| `test_palette_workflow_definition.py` | Palette workflow ASL template: required states, structure | JSON parsing only |
| `test_palette_render_plan.py` | Palette render plan Lambda | Python mocks only |
| `test_palette_chunk_handler.py` | Palette chunk worker Lambda | Python mocks only |
| `test_palette_finalize_handler.py` | Palette finalize Lambda | Python mocks only |
| `test_coeff_catalog_consistency.py` | Hand-written coeff-function overrides stay wired through source catalog, generated lookup, and generated JS catalog | Python only |
| `test_giga62_hand.py` | giga_62 hand-written function accuracy | `sweep_test` compiled |
| `test_poly645_hand.py` | `poly_645` hand-written coeff function matches Python reference and stays off the broken transpiled path | `sweep_test` compiled |
| `test_poly795_hand.py` | `poly_795` hand-written coeff function matches Python reference, including slice rewrites and both `np.where` branches | `sweep_test` compiled |
| `test_low_agreement_hand.py` | Batch parity coverage for the repaired low-agreement coeff funcs promoted from transpiled to hand; the current authoritative function list lives in `CASES` inside the test file | `sweep_test` compiled, numpy |
| `test_deploy_packaging.py` | Lambda zip contents, local helper/sidecar packaging, executable chmod coverage, and deploy-script regressions such as the PDF layer builder entrypoint/tooling contract | Python only |
| `test_api_route_contracts.py` | Manifest-backed API contract checks: tracked `api_manifest.json` must match the current tree, frontend service/route usage must match deploy wiring, and dispatch/solver mappings must stay aligned | Python only |
| `test_pdf_artifact_handler.py` | PDF artifact Lambda: Color source validation, metadata-derived spread composition, PDF upload contract | Python mocks only |
| `test_frontend_js.sh` | Frontend JS execution: UI logic, TRI palette popup/swatches, dispatch, Render family catalogs, Palette workflow UI, DeepZoom inventory, render perf logging | Node.js (vm module) |
| `e2e/compute-ui.spec.js` | Compute tab: preview controls, function picker, compute-preview dispatch/render path | Playwright browser |
| `e2e/results-ui.spec.js` | Results tab: refresh popup, filtering, selection, populate/render actions | Playwright browser |
| `e2e/favorites-ui.spec.js` | Favorites tab: inventory load, GoRender, download menu, delete | Playwright browser |
| `e2e/palette-ui.spec.js` | Palette tab: inventory load, create dispatch, download/delete, keyboard navigation | Playwright browser |
| `e2e/deepzoom-inventory.spec.js` | DeepZoom inventory: load, sort, select, arrow keys, share links | Playwright browser |
| `e2e/render-refresh.spec.js` | Render tab refresh: summary call, artifact panel, info line | Playwright browser |
| `e2e/render-solve-score.spec.js` | Solve score UI: metrics, quantile, TRI palette behavior, dispatch payloads, family catalogs, palette family behavior | Playwright browser |

## UI Coverage Matrix

The UI is not tested by one monolithic suite. Coverage is split deliberately:

| Area | VM coverage (`test_frontend_js.sh`) | Playwright coverage | Current intent |
|---|---|---|---|
| Compute | Yes | `e2e/compute-ui.spec.js` | Preview controls, function picker, main calculate affordances must stay visible and wired |
| Results | Yes | `e2e/results-ui.spec.js` | Refresh popup, filtering, selection, populate/render actions must survive refactors |
| Favorites | Yes | `e2e/favorites-ui.spec.js` | Inventory, GoRender, download menu, delete must survive refactors |
| Palette | Yes | `e2e/palette-ui.spec.js` | Inventory, create dispatch, download/delete, keyboard navigation must survive refactors |
| Render | Yes | `e2e/render-refresh.spec.js`, `e2e/render-solve-score.spec.js` | Family tabs, solve-score controls, render popups, dispatch payloads, and selected-artifact actions are pinned hardest here |
| DeepZoom | Yes | `e2e/deepzoom-inventory.spec.js` | Inventory, row selection, share links, keyboard navigation |

This does **not** mean every cosmetic detail is browser-tested. It means:

- visible controls that matter operationally should exist in Playwright
- payload-shape and popup logic should also exist in `test_frontend_js.sh`
- if a feature adds a new tab control and there is no Playwright assertion for it, coverage is incomplete

## Running Tests

### Compile binaries first

```bash
cd polypaint/lambda
cc -O3 -o sweep_test sweep_cli.c -lm
cc -O3 -o bilevel_raster_local bilevel_raster.c -lm
cc -O3 -o bilevel_merge_local bilevel_merge.c $(pkg-config --cflags --libs vips) -lm
```

### Fast suite (no visual comparisons)

```bash
cd polypaint
uv run python -m pytest tests/test_sweep_smoke.py tests/test_ae_mt.py tests/test_poly_accuracy.py tests/test_pipeline.py tests/test_bilevel_raster.py tests/test_bilevel_stitch.py tests/test_dither.py tests/test_param_dump.py -v
```

### Individual test files

```bash
uv run python tests/test_dither.py
uv run python tests/test_param_dump.py
uv run python tests/test_bilevel_raster.py
uv run python tests/test_bilevel_stitch.py
uv run python -m pytest tests/test_ae_mt.py -q
uv run python -m pytest tests/test_poly164_hand.py tests/test_coeff_catalog_consistency.py -q
uv run python -m pytest tests/test_deploy_packaging.py tests/test_pdf_artifact_handler.py -q
uv run python -m pytest tests/test_api_route_contracts.py tests/test_deploy_packaging.py -q
python3 api_manifest.py --write
python3 api_manifest.py --check
bash scripts/predeploy_check.sh
uv run python lambda/gen_parity_results.py
uv run python -m pytest tests/test_coeff_parity_results.py tests/test_coeff_catalog_consistency.py -q
uv run python -m pytest tests/test_coeff_catalog_consistency.py tests/test_poly645_hand.py tests/test_poly795_hand.py tests/test_low_agreement_hand.py -q
```

### Frontend JS harness

Run this whenever `index.html` behavior changes, even if the browser UI "looks fine":

```bash
bash tests/test_frontend_js.sh
```

This is the fast guard for:

- missing DOM ids
- stale dispatch payloads
- popup initialization breakage
- contract-warning logging
- inventory/action wiring

### Playwright browser suite

Run the browser suite whenever user-facing tab behavior changes:

```bash
npx playwright test tests/e2e
```

Targeted runs are fine while iterating:

```bash
npx playwright test tests/e2e/compute-ui.spec.js
npx playwright test tests/e2e/results-ui.spec.js
npx playwright test tests/e2e/favorites-ui.spec.js
npx playwright test tests/e2e/palette-ui.spec.js
npx playwright test tests/e2e/render-refresh.spec.js
npx playwright test tests/e2e/render-solve-score.spec.js
npx playwright test tests/e2e/deepzoom-inventory.spec.js
```

Use Playwright when the question is:

- "does the popup actually open?"
- "did the button disappear?"
- "does clicking this still dispatch the right thing?"
- "does keyboard selection still work?"

### Deploy-time layer/build checks

When adding or changing any Lambda layer or deploy-time build script, do not stop at unit tests. Run both:

```bash
uv run python -m pytest tests/test_deploy_packaging.py -q
```

and the real build command for the affected layer. For the PDF layer:

```bash
bash lambda/build-pdf-python-layer.sh
```

The packaging test is permanent and checks the specific PDF-layer regression points:

- the Lambda Python base image entrypoint is overridden with `--entrypoint /bin/bash`
- the build script uses Python zipping/cleanup instead of assuming `find` or `zip` exist in the Lambda base image
- the PDF artifact Lambda is wired into `deploy.sh`

If the real layer build is not run after changing the script, the deploy path is not considered verified.

The same rule applies to deploy binaries:

- if `lambda/<binary>` is what the Docker gate mounts, rebuild `lambda/<binary>`
  before the gate
- do not run the gate first and then rebuild only after it fails on stale output

Shared-helper rule:

- if a deployed handler starts importing a new repo-local helper such as
  `calc_chunks.py`, `logical_sections.py`, or any other `lambda/*.py` module,
  `tests/test_deploy_packaging.py` must be run and the corresponding bundle in
  `deploy.sh` must be updated in the same change
- do not assume direct local handler tests prove packaging is correct

Python-runner rule:

- for local repo checks, prefer `uv run python` when `uv` is available
- the repo venv is the fallback:
  - `./.venv/bin/python`
  - `../.venv/bin/python`
- in this environment, assume `uv` may need escalation because the shared cache
  often sits outside the sandbox
- do not burn a first attempt rediscovering that failure mode
- if the `uv` command matters, run it with escalation instead of changing the
  interpreter preference

Metadata-contract rule:

- if a feature changes where artifact metadata lives, tests must cover both the
  write path and every affected read path

API-manifest rule:

- if a change touches frontend service calls or route wiring in:
  - [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
  - [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
  - [handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
  - [handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py)
- then regenerate the tracked manifest in the same change:
  - `python3 api_manifest.py --write`
- `python3 api_manifest.py --check` is a gate, not the regeneration step
- examples:
  - image headers stay small and bounded
  - bulky fields move to sidecar JSON / `meta.json`
  - readers use the merged metadata loader instead of raw
    `head_object(...).Metadata`

### Predeploy contract gate

Before `deploy.sh update` touches AWS, run:

```bash
bash scripts/predeploy_check.sh
```

This gate checks:

- tracked `api_manifest.json` still matches the current tree
- API contracts in `tests/test_api_route_contracts.py`
- deploy/package contracts in `tests/test_deploy_packaging.py`
- frontend action/contract coverage in `tests/test_frontend_js.sh`

`deploy.sh update` now runs this gate automatically before Lambda layer builds, binary compiles, or AWS updates.

### Post-deploy reality check

When deployed behavior looks stale or contradictory, do not guess from logs
alone. Verify the deployed state directly:

```bash
./deploy.sh show-build
```

Use this after `deploy.sh update` whenever the question is:

- "did the frontend/config really update?"
- "did the critical Lambda bundle really update?"
- "did the workflow definition really update?"

`show-build` is the check that separates real runtime bugs from deploy drift.

### Hand override workflow

When replacing a broken transpiled coeff function with a hand implementation in `poly_hand.h`, all of these must be updated together:

1. Add or update the hand function in `lambda/poly_hand.h`.
2. Change the source catalog entry in `lambda/coeff_func_catalog.json`:
   - `c_symbol` must point at `<name>_hand`
   - `kind` must be `"hand"`
   - `source` must be `"poly_hand.h"`
3. If the function has parity coverage, regenerate the parity overlay instead of hand-editing the frontend agreement badge:
   ```bash
   cd polypaint
   ../.venv/bin/python lambda/gen_parity_results.py
   ```
   This rewrites `lambda/coeff_func_parity.json`, which is the source of truth for parity-tested agreement badges.
4. Regenerate the derived artifacts:
   ```bash
   cd polypaint/lambda
   python3 gen_catalog.py
   ```
   This rewrites:
   - `lambda/coeff_func_lookup.h`
   - `coeff_func_catalog_js.js`
5. Rebuild binaries that compile against `coeff_func_lookup.h`:
   ```bash
   cc -O3 -o sweep_test sweep_cli.c -lm
   ```
   And for deploy, `deploy.sh update` will rebuild the shipped `sweep` binary.
6. Add or update a parity regression for the specific function if the transpiled implementation was wrong.
   Prefer adding the new function to `tests/test_low_agreement_hand.py` once there are multiple low-agreement fixes in flight, so the backlog stays in one place.
   If the coeff function is highly sensitive to tiny parameter drift, compute the Python reference from the exact transform doubles instead of `param_dump`'s float32 serialization.
   For numerically explosive functions that serialize through float32, compare the stable finite prefix and then assert the tail only diverges where float32 overflow becomes unavoidable.
   For chaotic unit-magnitude recurrences like transformed `poly_26`, compare the stable prefix plus any direct overwrite slots, then assert the later normalized-recursion slots still lie on the unit circle.
7. Run the loose-end checks after regenerating:
   ```bash
   cd polypaint
   ../.venv/bin/python lambda/gen_parity_results.py
   uv run python -m pytest tests/test_coeff_parity_results.py tests/test_coeff_catalog_consistency.py tests/test_poly645_hand.py tests/test_poly795_hand.py tests/test_low_agreement_hand.py -q
   ```

Do not stop after editing `poly_hand.h`. If the catalog or generated lookup is left stale, the runtime and UI will still use and label the function as transpiled.
Do not hand-edit `agreement_pct` for functions covered by parity suites; regenerate `lambda/coeff_func_parity.json` and then rebuild the JS catalog.
The parity/catalog consistency tests will also self-regenerate `lambda/sweep_test`, `lambda/coeff_func_parity.json`, and `coeff_func_catalog_js.js` if needed, so a clean checkout can verify the path without manual pre-generation.

### Low-agreement repair workflow

When cleaning up the low-agreement backlog, use this order instead of sorting by agreement badge alone:

1. Prioritize transpiled functions whose generated C contains explicit `WARNING: unhandled ...` markers.
2. Check whether the Python source itself is broken before blaming the transpiler.
   Examples from the current backlog:
   - `poly_667`: fixed fallback coeff-count bug (`except -> zeros(9)`)
   - `poly_504`: fixed source bug (`range(... np.floor(...) ...)`)
   - `poly_101`: fixed off-by-one recurrence bound (`range(1, 35)`)
   - `poly_106`: fixed off-by-one tail rewrite (`range(15, 71)`)
   - `poly_121`: fixed invalid `np.sum(t1**2, t2**2)` to scalar addition
   - `poly_149`: fixed invalid `np.sum(np.real(t1), np.imag(t2))` to scalar addition
   - `poly_742`: fixed source bug (`np.math.factorial` -> `math.factorial`)
   - `poly_760`: fixed source bug (`np.fft.ifft(...)` instead of invalid `fft(..., inverse=True)`)
   - `poly_812`: fixed source bug (loop bound off by one)
3. Only after the Python reference is trustworthy, add the hand override in `poly_hand.h`.
4. Update `coeff_func_catalog.json` and `coeff_func_metrics.json`.
5. Regenerate `coeff_func_lookup.h` and `coeff_func_catalog_js.js`.
6. Rebuild `sweep_test`.
7. Run:
   ```bash
   ../.venv/bin/python -m pytest -q tests/test_low_agreement_hand.py tests/test_coeff_catalog_consistency.py
   ```
   This avoids local `uv` cache-permission issues if they occur.
8. If a promoted function only disagrees after values exceed float32 range, keep the hand route and encode that as an explicit overflow-tail contract in `tests/test_low_agreement_hand.py` instead of weakening the whole suite.

Current state after the completed repair pass:

- there are `0` remaining `transpiled` `poly_*` functions with `agreement_pct <= 30`
- transformed `poly_809`, `poly_811`, and `poly_818` use exact transform doubles in the parity harness
- `poly_39`, `poly_86`, `poly_102`, `poly_324`, and `poly_450` use float32-overflow-tail contracts
- `poly900.py` parity imports rely on lightweight `polylayout` stubs in the test harness

### Docker ARM64 tests (binary tests that need LAPACK or ARM64 runtime)

These tests exercise ARM64 binaries that can't run natively on macOS. Run them in Docker:

```bash
# Recompile sweep_cm with latest C source + run all companion matrix tests
docker run --rm --platform linux/arm64 \
    -v "$(pwd):/work" \
    -v "$(pwd)/lambda/layer-build-lapack:/opt" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc python3 python3-pip 2>&1 | tail -1
        pip3 install --root-user-action=ignore pytest 2>&1 | tail -1
        export LD_LIBRARY_PATH=/opt/lib
        gcc -O3 -o /work/lambda/sweep_cm /work/lambda/sweep_cm.c \
            -L/opt/lib -llapack -lopenblas -lm -Wl,-rpath,/opt/lib
        cd /work && python3 -m pytest tests/test_companion_matrix.py -v
    '
```

Tests that require Docker ARM64:

| File | Binary | Layer needed |
|------|--------|-------------|
| `test_companion_matrix.py` | `sweep_cm` | LAPACK (`layer-build-lapack`) |
| `test_solve_proximity_stats.py` | `solve_proximity_stats` | None (static) |
| `test_solve_palette_debug.py` | `solve_palette_debug` | None (static) |

These tests skip automatically on non-ARM hosts with a message pointing to Docker.

The deploy-style ARM64 smoke suite in `scripts/test-docker-runtime.sh` now covers:

- `sweep`
- `sweep_mt`
- `sweep_cm`
- `solve_proximity_stats`
- `roots2pix`

### Native solve-score program changes

If you change any of these:

- `lambda/solve_score.h`
- `lambda/solve_proximity_stats.c`
- `lambda/solve_proximity_hist_sectioned.c`
- `lambda/roots2pix.c`
- `lambda/roots2pix_mt.c`
- `lambda/solve_palette_chunk.c`
- `lambda/solve_palette_chunk_mt.c`
- score-program compilation or serialization in `index.html` or `lambda/solve_score_chain.py`

do **not** treat handler tests as sufficient. The common failure mode here is that Python wiring passes while the deployed native binary rejects or misinterprets the exact CLI/runtime shape.

Minimum expectation:

1. Add or update a regression in `tests/test_solve_proximity_stats.py` and/or `tests/docker_runtime_regression.py` for the exact program shape that changed.
2. Run the focused Docker-backed binary suite:
   ```bash
   uv run python -m pytest -q tests/test_solve_proximity_stats.py
   ```
3. Run the deploy-style runtime gate:
   ```bash
   bash scripts/test-docker-runtime.sh
   ```

The regression should match the real execution shape as closely as possible:

- the exact binary mode: `summary`, `hist`, `tmpfile`, or `sectioned`
- the exact source mix: `slv`, `cf`, `pm`, or mixed-source
- the exact operator shape: unary, binary, chained transfer, `omega_cosine`, `sawtooth`, etc.
- any legacy/v2 contract boundary, for example:
  - program mode must not require legacy `clip_lo/clip_hi`
  - param/coeff-source programs must require the matching sidecar file and row-alignment metadata

When building Docker command strings for these tests, quote `--score_program` with `shlex.quote(...)`. Unquoted semicolons will be interpreted by the shell and invalidate the test.

### Visual comparison tests (slow, ~5 min per batch)

```bash
uv run python tests/test_visual_1_100.py
uv run python tests/test_visual_301_500.py
# etc.
```

## What Each Test Covers

### test_sweep_smoke.py

Tests the compiled `sweep` binary end-to-end via subprocess:

- **TestCoeffgenSmoke** — metadata and file size for basic coeffgen, times=3 produces 3x data
- **TestSolveSmoke** — solver reads coefficients, correct output size, times multiplier
- **TestGridSmoke** — grid mode produces correct n_t and output size
- **TestDitherUniqueness** — pass 0 vs pass 1 differ, different chunks differ

### test_poly_accuracy.py

Compares Python and C implementations of polynomial functions at 5 test points. Tolerance: max error < 1e-4 (float32 vs float64).

### test_pipeline.py

Unit tests with mocked AWS services (no real S3/DynamoDB/Lambda calls):

- **TestDispatchHandler** — fire-and-forget Lambda invocation, error handling
- **TestStorageList** — job listing, calc.json parsing, missing files
- **TestRenderSummary** — immutable render-family catalogs, including PDF family entries
- **TestStorageCheckKeys/CleanRender/Presign** — S3 operations
- **TestShared** — parse_body, ok_response, viewport computation
- **TestReportStatus/CheckStatus** — DynamoDB status tracking
- **TestCoeffgenHandler** — coeffgen subprocess, S3 upload, transforms
- **TestSolveFromCoeffs** — solve routing, full file download
- **TestPreviewHandler** — preview generation, PNG validity
- **TestPdfArtifactHandler** — PDF spread derivation from saved Color artifacts

### test_deploy_packaging.py

Deploy-time contract coverage:

- packaged handlers include all local Python/module sidecars they import
- packaged local binaries have matching `chmod +x` in `deploy.sh`
- known regression paths stay covered:
  - palette-name generated modules
  - `solve_proximity_stats`
  - `roots2pix`
  - `autolevels_render`
  - `pixbinassemble`
  - `pixel_bins_render`
  - PDF artifact packaging and the PDF layer build-script contract
  - storage handler routes must be published by `deploy.sh`

### test_api_route_contracts.py

Frontend/API contract coverage:

- every `lambdaPost('storage', ..., '/...')` path used in `index.html` must exist in `handler_storage.py`
- every frontend storage path must also be published by `deploy.sh`
- every frontend service name used by `lambdaPost(...)` must exist in the generated `config.json` template

### test_bilevel_raster.py

Verifies bilevel_raster C binary against Python reference:

- **test_basic** — poly_1, 50×50 grid, 2×2 tiles, bitsets match byte-for-byte
- **test_rotation** — quarter turn, bitsets match
- **test_empty_tiles** — offset viewport, no .bits output
- **test_multiple_functions** — poly_1, poly_4, poly_49

### test_bilevel_stitch.py

Tests bilevel_merge binary (needs libvips):

- **test_merge_basic** — OR two bitsets → correct pixels in TIFF
- **test_merge_empty** — no input → all-black tile
- **test_stitch_2x2** — 4 tiles with distinct patterns, quadrant placement
- **test_stitch_3x2** — non-square grid, correct dimensions
- **test_stitch_missing_tile** — fails cleanly on missing input

### test_dither.py

Statistical validation of all three dither transforms:

- **test_identity** — empty chain = zero offset
- **test_sdith_bounded** — all offsets within ±d/(2N) per component
- **test_sdith_scaling** — d=2 has 2x spread of d=1
- **test_ddith_bounded** — radial offsets within d/N (disk)
- **test_ddith_isotropic** — re/im spread ratio ~1.0
- **test_ndith_gaussian** — measured sigma matches d/N
- **test_ndith_unbounded** — confirms Gaussian tails exceed d/N

### test_param_dump.py

Tests sweep param_dump mode (same code path as coeffgen transforms):

- **test_identity** — empty chain = exact grid positions
- **test_unit_circle** — exp(2πix) matches expected
- **test_rtheta** — pow(x,p)*exp(2πiy) with parameter p
- **test_swap** — z1↔z2
- **test_chain** — unit_circle → square composed correctly
- **test_t1radd/t2iadd** — per-parameter offsets
- **test_output_size** — file size = n²×16 bytes

### Visual comparison tests

Compare C sweep pipeline vs Python reference via pixel overlap:

- Generate roots from both pipelines on 100×100 grid
- Rasterize to 1000×1000 boolean image
- Compute intersection/union overlap
- ≥60% = PASS, <60% = FAIL (transpiler bug or float32 drift)

## Pre-Deploy Checklist

Before running `deploy.sh update`:

1. **Compile locally:**
   ```bash
   cd lambda
   cc -O3 -o sweep_test sweep_cli.c -lm
   cc -O3 -o bilevel_raster_local bilevel_raster.c -lm
   cc -O3 -o bilevel_merge_local bilevel_merge.c $(pkg-config --cflags --libs vips) -lm
   ```
2. **Run full fast suite:**
   ```bash
   uv run python -m pytest tests/test_sweep_smoke.py tests/test_poly_accuracy.py tests/test_pipeline.py tests/test_bilevel_raster.py tests/test_bilevel_stitch.py tests/test_dither.py tests/test_param_dump.py -v
   ```
3. **If you changed coeff-function wiring or hand overrides, run the catalog consistency slice:**
   ```bash
   uv run python -m pytest tests/test_coeff_catalog_consistency.py tests/test_poly645_hand.py -q
   ```
4. **Cross-compile:** `aarch64-linux-musl-gcc -O3 -static -o sweep sweep_cli.c -lm`
5. **JS syntax check:** `deploy.sh` does this automatically

If the change touches solve-score native/runtime behavior, also run:

```bash
uv run python -m pytest -q tests/test_solve_proximity_stats.py
bash scripts/test-docker-runtime.sh
```

The second command is mandatory for anything that changes the deployed binary path.

### test_chunking.py

Tests the chunked coefficient pipeline (param_gen → coeffgen_chunked):

- **test_param_gen_size** — output is exactly N×N×times×16 bytes for multiple (N, times) combos
- **test_param_gen_deterministic** — identical output for same inputs (including dither RNG)
- **test_coeffgen_chunked_matches_monolithic** — concatenated chunk output matches old monolithic coeffgen within float32 quantization (~1e-5)
- **test_chunk_planner_coverage** — no gaps, no overlaps, full step coverage for various (n_steps, n_chunks)
- **test_chunks_greater_than_n** — N=5, chunks=50 processes all 2500 steps (the original motivation)
- **test_param_gen_with_dither** — different passes produce different dither values

### test_dispatch_resilience.py

Tests for the dispatch resilience fixes (28 tests, pure mocks, no binaries):

- **TestCheckStatusReturnIds** — `return_ids` flag: default off, explicit true/false, 449/500 gap detection with 51 missing tasks, paginated queries
- **TestHeadKeys** — `/head-keys` endpoint: all exist, none exist, mixed existence, empty list, single key, parallel HEAD execution
- **TestDispatchBilevelTarget** — dispatch handler fires bilevel/stitch targets, non-202 status tracked, 200-job parallel batch
- **TestStorageRouting** — `/head-keys` route wired to handler, unknown route returns 400
- **TestMissingTaskDetection** — set-diff logic replicated from JS: no missing, all missing, contiguous 51-task gap (the real failure), scattered gaps, merge prefix, coeff prefix
- **TestWaveDispatchLogic** — wave dispatch simulation: small batch (single wave), large batch (multiple waves), throttled (MAX_INFLIGHT respected), exact inflight, full inter-wave completion

### test_tri_palette_generation.py

Tests the generated TRI palette catalogs and interpolation output:

- alias collapse / canonical name selection
- 15-stop midpoint behavior
- unknown color-name rejection
- JS/Python generated catalogs stay in sync
- generated gradient CSS includes all 15 stops

### test_companion_matrix.py

Tests the `sweep_cm` companion-matrix solver binary (Docker ARM64 + LAPACK):

- **test_exact_cubic/quartic** — known roots verified by residual
- **test_repeated_root** — (x-1)² handled correctly
- **test_leading_zero** — leading zero coefficients trimmed
- **test_all_zero** — all-zero polynomial produces finite output
- **test_complex_quadratic** — x²+1 gives ±i
- **test_output_size** — file size matches n_steps × degree × 8 bytes
- **test_linear** — degree-1 fast path
- **test_ae_vs_cm_comparison** — both solvers agree on same input
- **test_inf_coefficients** — inf leading coeff: skipped, zero roots, valid JSON with skipped_overflow
- **test_near_overflow_coefficients** — normalized coeff overflow caught before LAPACK
- **test_mixed_batch_overflow** — batch with normal + overflowed polys: all produce output, normal ones solve correctly

### test_solve_proximity_stats.py

Tests the `solve_proximity_stats` binary (Docker ARM64):

- 10 solve-score metrics, summary mode, quantile clipping, palette validation

### test_solve_palette_debug.py

Tests the `solve_palette_debug` binary (Docker ARM64):

- Serpentine placement, nearest-neighbor expansion, palette selection, quantile clipping

### test_palette_debug_handler.py

Tests the palette debug Lambda handler (Python mocks):

- Payload validation, quantile range, S3 upload key, stale preview deletion, no report_status

### test_render_workflow_definition.py

Tests the Step Functions ASL template (JSON parsing only):

- Required states exist, Map concurrency, retry policies, preview tasks, Parallel wrapper catch

### test_frontend_js.sh

Frontend JS execution tests (Node.js vm module, no browser):

- Catalog loading, TRI/LONG/built-in palette popups, dropdown population, CFPV rows, pipeline dispatch, wave dispatch, Render family catalogs, DeepZoom inventory, solve histogram, palette workflow, Palette tab, Autolevels popup dispatch/defaults/revert/logging

### e2e/ Playwright tests

Browser-based end-to-end tests:

- **compute-ui.spec.js** — Compute Preview controls, function picker, calculate affordances, preview payload/render path
- **results-ui.spec.js** — Results refresh popup, filtering, row selection, populate/render handoff
- **favorites-ui.spec.js** — favorites inventory load, GoRender, download menu, delete
- **palette-ui.spec.js** — palette inventory load, palette-create dispatch, download/delete, keyboard navigation
- **deepzoom-inventory.spec.js** — inventory load/sort, row selection, arrow keys, share links, question mark for old exports
- **render-refresh.spec.js** — single render-summary call, family tabs, selected-artifact actions, info line
- **render-solve-score.spec.js** — metric selection, quantile dispatch, TRI palette popup/right-click behavior, solve-score controls, palette family behavior

### test_tiff_compat.py

Tests tiff_compat binary (needs libvips+libtiff):

- Converts tiled TIFF to strip-based
- Verifies output dimensions and pixel placement match input

### test_png_export.py

Tests png_export binary (needs libvips):

- Converts bilevel TIFF to 1-bit PNG
- Verifies output dimensions

### test_autolevels_handler.py

Tests the autolevel post-process Lambda handler (Python mocks):

- parameter sanitization and stage-toggle defaults
- background metadata fallback and threshold clamping
- derived artifact upload metadata
- autolevel debug payload emission

### test_autolevels_render_native.py

Tests the native autolevel renderer (libvips):

- background exclusion on/off behavior
- per-channel threshold effect
- all-background fallback

## When to Run What

| What changed | Tests to run |
|---|---|
| sweep_cli.c | test_sweep_smoke + test_poly_accuracy + test_param_dump + test_dither |
| Param transforms | test_param_dump + test_dither |
| Coefficient functions / transpiler | test_poly_accuracy + visual comparisons |
| bilevel_raster.c | test_bilevel_raster |
| bilevel_merge.c | test_bilevel_stitch |
| Lambda handlers (Python) | test_pipeline + test_dispatch_resilience + test_chunking |
| Chunking / param_gen / coeffgen_chunked | test_chunking |
| TRI palette generator / catalogs | test_tri_palette_generation |
| Dispatch / storage / check-status | test_dispatch_resilience |
| sweep_cm.c | test_companion_matrix (Docker ARM64) |
| solve_proximity_stats.c | test_solve_proximity_stats (Docker ARM64) |
| `index.html` UI logic only | `test_frontend_js.sh` |
| `index.html` visible user flows / popups / tab actions | `npx playwright test tests/e2e` |
| New tab/button/popup in UI | `test_frontend_js.sh` + at least one Playwright spec that clicks it |
| solve_palette_debug.c | test_solve_palette_debug (Docker ARM64) |
| autolevels_render.c / handler_autolevels.py | test_autolevels_handler + test_autolevels_render_native + test_frontend_js.sh |
| handler_raster.py / handler_raster_mt.py | test_raster_pixel_bins + test_raster_saved_palette + test_raster_mt + test_frontend_js.sh |
| Palette handlers | test_palette_debug_handler + test_palette_chunk_handler + test_palette_finalize_handler |
| Step Functions ASL | test_render_workflow_definition + test_palette_workflow_definition |
| Render orchestrator/plan/status | test_render_orchestrator + test_render_plan + test_render_status |
| tiff_compat.c | test_tiff_compat |
| png_export.c | test_png_export |
| Frontend JS logic | test_frontend_js.sh |
| Frontend UI (browser) | e2e/*.spec.js (Playwright) |
| Before any deploy | Fast suite + Docker ARM64 tests + frontend JS + e2e |
