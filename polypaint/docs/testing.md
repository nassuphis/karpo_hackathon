# Testing

## Test Location

All tests live in `polypaint/tests/`. Three test files:

| File | What it tests | Requires |
|------|--------------|----------|
| `test_sweep_smoke.py` | sweep binary: coeffgen, solve, grid, dither uniqueness | `sweep_test` compiled |
| `test_poly_accuracy.py` | Transpiled C poly functions match Python originals | `sweep_test` compiled, numpy |
| `test_pipeline.py` | Lambda handlers: dispatch, storage, coeffgen, sweep, preview | Python mocks only |

## Running Tests

### Full suite (all three files)

```bash
cd polypaint
# Compile the sweep binary first
cd lambda && cc -O3 -o sweep_test sweep_cli.c -lm && cd ..
# Run all tests
uv run python -m pytest tests/ -v
```

### Individual test files

```bash
cd polypaint
uv run python -m pytest tests/test_sweep_smoke.py -v
uv run python -m pytest tests/test_poly_accuracy.py -v
uv run python -m pytest tests/test_pipeline.py -v
```

### Poly accuracy standalone (no pytest needed)

```bash
cd polypaint
uv run python tests/test_poly_accuracy.py
```

## What Each Test Covers

### test_sweep_smoke.py

Tests the compiled `sweep` binary end-to-end via subprocess:

- **TestCoeffgenSmoke**
  - `test_coeffgen_basic` — metadata and file size match for a basic coeffgen run
  - `test_coeffgen_with_times` — times=3 produces exactly 3x the data of times=1

- **TestSolveSmoke**
  - `test_solve_basic` — solver reads coefficients and produces correct output size
  - `test_solve_reads_all_times` — solver processes all 300 steps from times=3 coefficients (not just 100)
  - `test_solve_times_multiplier` — solve output for times=3 is exactly 3x the times=1 output

- **TestGridSmoke**
  - `test_grid_basic` — grid mode produces correct n_t and output size

- **TestDitherUniqueness**
  - `test_passes_differ` — pass 0 and pass 1 produce different coefficients (RNG seeded per pass)
  - `test_different_stripes_differ` — different i1_start values produce different dither

### test_poly_accuracy.py

Compares Python and C implementations of polynomial functions at 5 test points:

- `test_poly_16` — poly_16: 51 coefficients, iterative with primes
- `test_poly_17` — poly_17: 71 coefficients, slice assignments with arange

Tolerance: max error < 1e-4 (float32 vs float64 precision).

### test_pipeline.py

Unit tests with mocked AWS services (no real S3/DynamoDB/Lambda calls):

- **TestDispatchHandler** (5 tests) — fire-and-forget Lambda invocation, error handling
- **TestStorageList** (4 tests) — job listing, calc.json parsing, missing files, preview detection
- **TestStorageCheckKeys** — S3 key polling
- **TestStorageCleanRender** — render artifact cleanup, preserves .bin files
- **TestStoragePresign** (2 tests) — presigned URL generation with/without filename
- **TestShared** (5 tests) — parse_body, ok_response, viewport computation
- **TestReportStatus** (3 tests) — DynamoDB status tracking, TTL, error truncation
- **TestCheckStatus** (4 tests) — DynamoDB task polling, pagination, error details
- **TestCleanRenderDynamoDB** — DynamoDB cleanup on clean-render
- **TestCoeffgenHandler** (6 tests) — coeffgen subprocess, S3 upload, transforms, DDB status
- **TestSolveFromCoeffs** (5 tests) — solve routing, full file download, S3 key override, failure
- **TestPreviewHandler** (4 tests) — preview generation, custom size, empty lores, PNG validity

## Pre-Deploy Checklist

Before running `deploy.sh update`:

1. **Compile locally:** `cd lambda && cc -O3 -o sweep_test sweep_cli.c -lm`
2. **Run full test suite:** `cd polypaint && uv run python -m pytest tests/ -v`
3. **Cross-compile:** `cd lambda && aarch64-linux-musl-gcc -O3 -static -o sweep sweep_cli.c -lm`
4. **JS syntax check:** `deploy.sh` does this automatically
5. **Verify API Gateway routes:** Every storage endpoint used by the frontend must have a matching `ensure_route` in deploy.sh (see below)

## Post-Deploy Checklist

After deploying, verify:

1. **API Gateway routes match handler endpoints.** The storage Lambda handles multiple routes internally (via `path.endswith("/...")`), but each route must be explicitly registered in the API Gateway. If deploy.sh adds a new handler endpoint but forgets the `ensure_route`, the frontend gets "Failed to fetch" with no useful error. Known routes that must exist:
   - `POST /list` — job listing
   - `POST /delete` — job deletion
   - `POST /detail` — file count + viewport for selected job
   - `POST /save-metadata` — calc.json upload
   - `POST /clean-render` — render artifact cleanup
   - `POST /check-keys` — legacy S3 key polling
   - `POST /check-status` — DynamoDB task polling
   - `POST /presign` — presigned URL generation

   To check: `aws apigatewayv2 get-routes --api-id <API_ID> --region us-east-1`

2. **Lambda env vars.** Lambdas that write to DynamoDB need `JOBS_TABLE`. Dispatch Lambda needs function name env vars for all targets (`RASTER_FUNCTION`, `FINALIZE_FUNCTION`, `ENCODE_FUNCTION`, `SWEEP_FUNCTION`).

3. **Test a compute + preview + render** end-to-end on a fast function (e.g., giga_30, N=100) to verify the full pipeline works.

## When to Run What

| What changed | Tests to run |
|---|---|
| sweep_cli.c (any edit) | test_sweep_smoke + test_poly_accuracy |
| Coefficient functions or transpiler | test_poly_accuracy |
| RNG, dither, times loop | test_sweep_smoke (especially dither tests) |
| Lambda handlers (Python) | test_pipeline |
| handler_preview.py | test_pipeline (TestPreviewHandler) |
| handler_sweep.py | test_pipeline (TestSolveFromCoeffs) |
| handler_coeffgen.py | test_pipeline (TestCoeffgenHandler) |
| Before any deploy | All three test files |
| New poly function (hand-written or transpiled) | Visual comparison: render Python _py.png and C _c.png side by side, check overlap |

## Visual Comparison Procedure

When adding or fixing a poly function, always generate **both** the Python reference and C sweep renders and compare side by side:

1. **Render Python reference:** Run the Python function through `unit_circle → poly_N → rev → np.roots` at N=100, 1000px, extent=2.0. Save as `/tmp/poly_NNN_py.png`.
2. **Render C sweep:** Run `coeffgen → solve` through the sweep binary. Save as `/tmp/poly_NNN_c.png`.
3. **Open both side by side:** `open /tmp/poly_NNN_py.png /tmp/poly_NNN_c.png`
4. **Compute pixel overlap:** >60% = OK (float32 precision loss), <60% = broken transpilation, needs hand-writing.
5. **If Python produces 0 roots but C produces roots:** phantom — the Python function crashes (OOB, overflow, etc.) and the C bounds-checking/NaN-guard creates a different function. Accept as-is or exclude.
