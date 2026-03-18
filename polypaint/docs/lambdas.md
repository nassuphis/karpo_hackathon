# Lambda Handlers

9 Lambdas, each with a Python handler and optional C binary.

## polypaint-coeffgen

**Handler:** `handler_coeffgen.py`
**Binary:** `sweep` (coeffgen mode)
**Route:** POST /coeffgen
**Memory:** 1769 MB (1 vCPU)

Generates polynomial coefficients for one horizontal stripe of the parameter grid.

**Input:**
- `job_id`, `stripe_idx` — job and stripe identification
- `function` — coefficient function name (e.g. "giga_30", "poly_17")
- `param_transforms` — array-of-arrays, e.g. `[["unit_circle"], ["sdith", "3"]]`
- `coeff_transforms` — array of names, e.g. `["safe", "rev"]`
- `n1`, `n2` — grid dimensions
- `i1_start`, `i1_end` — row range for this stripe
- `times` — repeat count for dithering (default 1)
- `s3_key` — optional override for S3 upload path

**Process:**
1. Builds JSON spec, pipes to `sweep` binary on stdin
2. Binary writes coefficient `.bin` to `/tmp`
3. Validates output file size matches `data_bytes` from metadata
4. Uploads to S3: `renders/{job_id}/coeffs_{stripe_idx:04d}.bin`
5. Reports status to DynamoDB (started/done/error)

**Output:** `coeffs_key`, `coeffs_size`, `n_coeffs`, `degree`, `elapsed_us`

See [coefficients.md](coefficients.md) for details on the coefficient pipeline.

---

## polypaint-sweep

**Handler:** `handler_sweep.py`
**Binary:** `sweep` (solve or grid mode)
**Route:** POST /sweep
**Memory:** 10240 MB (6 vCPUs)

Solves polynomial roots for one stripe. Two modes:

### Solve mode (when `coeffs_key` present)
Downloads pre-computed coefficients from S3, runs `sweep` in solve mode. The solver reads all coefficient sets from the file (supporting `times > 1`).

### Grid mode (legacy, when no `coeffs_key`)
Evaluates a coefficient function and solves inline. Spawns multiple `sweep` subprocesses to use all vCPUs. Merges sub-outputs into single `.bin`.

**Input:**
- `job_id`, `stripe_idx`, `i1_start`, `i1_end`, `n1`, `n2`
- `coeffs_key` — S3 key for coefficient file (solve mode)
- `n_coeffs` — coefficient count (solve mode)
- `function` — polynomial function name (grid mode)

**Status reporting:** Reports progress to DynamoDB via `report_status()` with task_id `sweep_{stripe_idx}`. Status progression: `started` -> `done` (or `error`). On success, `result_data` includes sweep metadata (`bin_size`, `compute_us`, `n_t`, `avg_iterations`).

**Output:** `s3_key`, `bin_size`, `n_t`, `degree`, `avg_iterations`, `n_procs`

**Invocation:** Dispatched asynchronously via the dispatch Lambda (fire-and-forget). The frontend polls `/check-status` with `task_prefix: 'sweep_'` every 3 seconds to track completion, collecting sweep metadata from the `results` array in the response.

See [roots.md](roots.md) for details on the solver.

---

## polypaint-viewport

**Handler:** `handler_viewport.py`
**Route:** POST /viewport
**Memory:** 512 MB

Computes viewport bounds (center, scale) from lores root data.

**Input:**
- `job_id`
- `quantile` — outlier fraction to trim (default 0.0 = use all roots)
- `shim` — padding multiplier (default 0.05 = 5% margin)

**Process:**
1. Reads `calc.json` to find `lores.bin` key
2. Downloads `lores.bin`
3. Sorts all root re/im coordinates, applies quantile + shim
4. Computes center and scale for a 4096px reference size
5. Saves `view.json` to S3

**Output:** `center_re`, `center_im`, `scale_ref`, `n_roots`, `q_re`, `q_im`

---

## polypaint-raster

**Handler:** `handler_raster.py`
**Binary:** `roots2pix`
**Route:** POST /raster (dispatched async)
**Memory:** 1769 MB (1 vCPU)

Converts one stripe of root data into tile-bucketed sparse pixel files.

**Input:**
- `job_id`, `stripe_idx`, `bin_key` — root data location
- `width`, `height` — full canvas size
- `tile_size`, `n_tile_cols`, `n_tile_rows` — tile grid
- `center_re`, `center_im`, `scale` — viewport
- `degree` — polynomial degree
- `color` — "rainbow", "proximity", or "constant"
- `match` — root matching: "none", "greedy", or "hungarian"
- `palette` — for proximity mode: "inferno", "viridis", etc.
- `constant_color` — hex RGB for constant mode

**Process:**
1. Downloads `.bin` from S3
2. Runs `roots2pix` which transforms each root (re, im) → pixel (px, py):
   ```
   px = halfW + (re - center_re) * scale
   py = halfH - (im - center_im) * scale
   ```
3. Buckets pixels into tiles, deduplicates with per-tile bitset
4. Writes `.pix` files: 8-byte entries `[uint32 local_pixel_idx, uint32 rgb]`
5. Uploads non-empty `.pix` files to S3

**Output:** `tiles_uploaded`, `roots_plotted`, `roots_clipped`, `raster_us`

### Color Modes

| Mode | Algorithm |
|------|-----------|
| rainbow | Fixed HSL gradient per root index |
| proximity | Nearest-neighbor distance → palette interpolation (2-pass) |
| constant | Single hex color for all roots |

### Root Matching

| Mode | Complexity | Notes |
|------|-----------|-------|
| none | O(1) | No tracking, roots in solver order |
| greedy | O(n^2) | Nearest-neighbor matching |
| hungarian | O(n^3) | Optimal bipartite matching, auto-downgrades to greedy if degree > 40 |

---

## polypaint-finalize

**Handler:** `handler_finalize.py`
**Binary:** `pixassemble`
**Route:** POST /finalize (dispatched async)
**Memory:** 1769 MB (1 vCPU)

Assembles all stripe `.pix` files for one tile into a single `.raw` tile image.

**Input:**
- `job_id`, `tile_idx` — which tile
- `n_stripes` — number of raster stripes
- `tile_w`, `tile_h` — tile dimensions

**Process:**
1. For each stripe, downloads `pix_{stripe:04d}_t{tile:04d}.pix` from S3
2. Pipes all `.pix` data to `pixassemble` via stdin
3. `pixassemble` reads 8-byte entries, builds RGB buffer (last-wins overwrite)
4. Writes `.raw` file: 12-byte header (W, H, bands=3) + RGB pixel data
5. Uploads to S3

**Output:** `raw_key`, `raw_size`, `pix_files`

---

## polypaint-encode

**Handler:** `handler_encode.py`
**Binary:** `raw2jpeg` (linked against libvips)
**Route:** POST /encode-upload (dispatched async)
**Memory:** 1769 MB + libvips layer

Stitches tile grid into final image and encodes JPEG or PNG.

**Input:**
- `out_key` — S3 output path
- `width`, `height` — full canvas
- `tile_grid` — `{n_cols, n_rows, tile_keys: [S3 keys]}`
- `format` — "jpeg" or "png"
- `quality` — JPEG quality (1-100, default 90)
- `bilevel` — optional 1-bit B&W mode

**Process:**
1. Downloads tiles row-by-row (memory-efficient)
2. Stitches into full `.raw` with 12-byte header
3. Runs `raw2jpeg` with libvips for JPEG/PNG encoding
4. Uploads encoded image, returns presigned URL

**Output:** `out_key`, `file_size`, `image_url`

---

## polypaint-preview

**Handler:** `handler_preview.py`
**Route:** POST /preview
**Memory:** 1024 MB

All-in-one preview: viewport + rasterize + encode in a single Lambda call. Pure Python, no binaries.

**Input:**
- `job_id`
- `quantile`, `shim` — viewport params
- `preview_size` — output dimension (default 256)

**Process:**
1. Downloads `lores.bin`, computes viewport
2. Plots roots as white pixels on black grayscale buffer
3. Encodes grayscale PNG (pure Python, zlib)
4. Uploads `preview.png`, returns presigned URL

**Output:** `image_url`, `n_roots`, `degree`, `q_re`, `q_im`, `png_size`

---

## polypaint-storage

**Handler:** `handler_storage.py`
**Route:** Multiple routes on same Lambda
**Memory:** 512 MB

S3 and DynamoDB management. Routes:

| Route | Purpose |
|-------|---------|
| `/list` | List all computed jobs (reads calc.json per job) |
| `/delete` | Delete all data for a job |
| `/save-metadata` | Save calc.json |
| `/cleanup` | Delete specific S3 keys |
| `/clean-render` | Delete render artifacts (.raw/.pix/.jpeg), preserve .bin |
| `/check-keys` | Poll S3 for expected file count (legacy) |
| `/check-status` | Poll DynamoDB task status |
| `/detail` | Get job file count + viewport bounds |
| `/presign` | Generate presigned download URL |

---

## polypaint-dispatch

**Handler:** `handler_dispatch.py`
**Route:** POST /dispatch-render
**Memory:** 1769 MB

Asynchronous fan-out: invokes target Lambdas in parallel using a 50-thread pool. Fire-and-forget (InvocationType=Event).

**Input:**
- `target` — "sweep", "raster", "finalize", or "encode"
- `jobs` — array of job specs to invoke

**Output:** `fired`, `total`, `errors`
