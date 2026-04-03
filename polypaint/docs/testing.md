# Testing

## Test Location

All tests live in `polypaint/tests/`.

| File | What it tests | Requires |
|------|--------------|----------|
| `test_sweep_smoke.py` | sweep binary: coeffgen, solve, grid, dither uniqueness | `sweep_test` compiled |
| `test_poly_accuracy.py` | Transpiled C poly functions match Python originals | `sweep_test` compiled, numpy |
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
| `test_solve_proximity_stats.py` | solve_proximity_stats binary: 10 metrics, summary mode, quantile clipping | Docker ARM64 |
| `test_solve_palette_debug.py` | solve_palette_debug binary: serpentine, expansion, palette selection, quantile | Docker ARM64 |
| `test_palette_debug_handler.py` | Palette debug Lambda handler: validation, S3 upload, stale preview deletion | Python mocks only |
| `test_render_workflow_definition.py` | Step Functions ASL template: required states, Map concurrency, retry policies | JSON parsing only |
| `test_render_orchestrator.py` | Render orchestrator starter Lambda: validation, DDB write, SFN StartExecution | Python mocks only |
| `test_render_plan.py` | Render plan Lambda: viewport, tile/chunk computation, output keys, size limit | Python mocks only |
| `test_render_status.py` | Render status Lambda: phase reporting, error extraction, updated_at_ms | Python mocks only |
| `test_palette_workflow_definition.py` | Palette workflow ASL template: required states, structure | JSON parsing only |
| `test_palette_render_plan.py` | Palette render plan Lambda | Python mocks only |
| `test_palette_chunk_handler.py` | Palette chunk worker Lambda | Python mocks only |
| `test_palette_finalize_handler.py` | Palette finalize Lambda | Python mocks only |
| `test_coeff_catalog_consistency.py` | Hand-written coeff-function overrides stay wired through source catalog, generated lookup, and generated JS catalog | Python only |
| `test_giga62_hand.py` | giga_62 hand-written function accuracy | `sweep_test` compiled |
| `test_poly645_hand.py` | `poly_645` hand-written coeff function matches Python reference and stays off the broken transpiled path | `sweep_test` compiled |
| `test_poly795_hand.py` | `poly_795` hand-written coeff function matches Python reference, including slice rewrites and both `np.where` branches | `sweep_test` compiled |
| `test_frontend_js.sh` | Frontend JS execution: UI logic, TRI palette popup/swatches, dispatch, Render family catalogs, Palette workflow UI, DeepZoom inventory | Node.js (vm module) |
| `e2e/deepzoom-inventory.spec.js` | DeepZoom inventory: load, sort, select, arrow keys, share links | Playwright browser |
| `e2e/render-refresh.spec.js` | Render tab refresh: summary call, artifact panel, info line | Playwright browser |
| `e2e/render-solve-score.spec.js` | Solve score UI: metrics, quantile, TRI palette behavior, dispatch payloads, family catalogs, palette family behavior | Playwright browser |

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
uv run python -m pytest tests/test_sweep_smoke.py tests/test_poly_accuracy.py tests/test_pipeline.py tests/test_bilevel_raster.py tests/test_bilevel_stitch.py tests/test_dither.py tests/test_param_dump.py -v
```

### Individual test files

```bash
uv run python tests/test_dither.py
uv run python tests/test_param_dump.py
uv run python tests/test_bilevel_raster.py
uv run python tests/test_bilevel_stitch.py
uv run python -m pytest tests/test_coeff_catalog_consistency.py tests/test_poly645_hand.py tests/test_poly795_hand.py -q
```

### Hand override workflow

When replacing a broken transpiled coeff function with a hand implementation in `poly_hand.h`, all of these must be updated together:

1. Add or update the hand function in `lambda/poly_hand.h`.
2. Change the source catalog entry in `lambda/coeff_func_catalog.json`:
   - `c_symbol` must point at `<name>_hand`
   - `kind` must be `"hand"`
   - `source` must be `"poly_hand.h"`
3. Update `lambda/coeff_func_metrics.json` if the frontend agreement badge should change.
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
7. Run the loose-end checks after regenerating:
   ```bash
   cd polypaint
   uv run python -m pytest tests/test_coeff_catalog_consistency.py tests/test_poly645_hand.py tests/test_poly795_hand.py -q
   ```

Do not stop after editing `poly_hand.h`. If the catalog or generated lookup is left stale, the runtime and UI will still use and label the function as transpiled.

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
- **TestStorageCheckKeys/CleanRender/Presign** — S3 operations
- **TestShared** — parse_body, ok_response, viewport computation
- **TestReportStatus/CheckStatus** — DynamoDB status tracking
- **TestCoeffgenHandler** — coeffgen subprocess, S3 upload, transforms
- **TestSolveFromCoeffs** — solve routing, full file download
- **TestPreviewHandler** — preview generation, PNG validity

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
| solve_palette_debug.c | test_solve_palette_debug (Docker ARM64) |
| autolevels_render.c / handler_autolevels.py | test_autolevels_handler + test_autolevels_render_native + test_frontend_js.sh |
| Palette handlers | test_palette_debug_handler + test_palette_chunk_handler + test_palette_finalize_handler |
| Step Functions ASL | test_render_workflow_definition + test_palette_workflow_definition |
| Render orchestrator/plan/status | test_render_orchestrator + test_render_plan + test_render_status |
| tiff_compat.c | test_tiff_compat |
| png_export.c | test_png_export |
| Frontend JS logic | test_frontend_js.sh |
| Frontend UI (browser) | e2e/*.spec.js (Playwright) |
| Before any deploy | Fast suite + Docker ARM64 tests + frontend JS + e2e |
