# Testing

## Test Location

All tests live in `polypaint/tests/`.

| File | What it tests | Requires |
|------|--------------|----------|
| `test_sweep_smoke.py` | sweep binary: coeffgen, solve, grid, dither uniqueness | `sweep_test` compiled |
| `test_poly_accuracy.py` | Transpiled C poly functions match Python originals | `sweep_test` compiled, numpy |
| `test_pipeline.py` | Lambda handlers: dispatch, storage, coeffgen, sweep, preview | Python mocks only |
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
```

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
- **TestDitherUniqueness** — pass 0 vs pass 1 differ, different stripes differ

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

1. **Compile locally:** `cd lambda && cc -O3 -o sweep_test sweep_cli.c -lm`
2. **Compile bilevel:** `cc -O3 -o bilevel_raster_local bilevel_raster.c -lm`
3. **Run fast suite:** `uv run python -m pytest tests/test_sweep_smoke.py tests/test_pipeline.py tests/test_dither.py tests/test_param_dump.py tests/test_bilevel_raster.py -v`
4. **Cross-compile:** `aarch64-linux-musl-gcc -O3 -static -o sweep sweep_cli.c -lm`
5. **JS syntax check:** `deploy.sh` does this automatically

## When to Run What

| What changed | Tests to run |
|---|---|
| sweep_cli.c | test_sweep_smoke + test_poly_accuracy + test_param_dump + test_dither |
| Param transforms | test_param_dump + test_dither |
| Coefficient functions / transpiler | test_poly_accuracy + visual comparisons |
| bilevel_raster.c | test_bilevel_raster |
| bilevel_merge.c | test_bilevel_stitch |
| Lambda handlers (Python) | test_pipeline |
| Before any deploy | Fast suite (all non-visual tests) |
