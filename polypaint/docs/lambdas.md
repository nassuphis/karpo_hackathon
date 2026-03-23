# Lambda Handlers

15 Lambdas, each with a Python handler and optional C binary.

## polypaint-coeffgen

**Handler:** `handler_coeffgen.py`
**Binary:** `sweep` (param_gen, coeffgen_chunked, or legacy coeffgen mode)
**Route:** POST /coeffgen
**Memory:** 1769 MB (1 vCPU), 10 GB /tmp

Three phases, routed by `phase` parameter:

### phase=param_gen

Generates the full unrolled parameter stream (`params.bin`). Single Lambda call (not fanned out). Applies param transforms and dither. Output streams directly to S3 via multipart upload — no `/tmp` materialization, supports arbitrarily large files.

**Input:**
- `job_id`, `N`, `times`
- `param_transforms` — array-of-arrays

**Process:**
1. Launches `sweep -` (stdout mode) with `mode: "param_gen"`
2. Pipes binary output to S3 multipart upload (8 MB parts)
3. Metadata returned on stderr

**Output:** `params_key`, `n_steps`, `data_bytes`

### phase=coeffgen_chunked

Reads a slice of `params.bin` from S3 via range read, generates coefficients for that chunk. Fanned out — one Lambda per chunk.

**Input:**
- `job_id`, `chunk_idx`, `params_key`
- `step_start`, `step_count` — contiguous range in params.bin
- `function`, `coeff_transforms`

**Process:**
1. Range-reads `step_count × 16` bytes from S3
2. Runs `sweep` in `coeffgen_chunked` mode
3. Uploads `coeffs_{chunk_idx:04d}.bin`

**Output:** `coeffs_key`, `coeffs_size`, `n_coeffs`, `degree`

### Default (legacy)

Old stripe-based coeffgen for backward compatibility with tests and scripts. Generates coefficients inline from grid parameters. Not used by the current frontend compute flow.

See [coefficients.md](coefficients.md) for details on the coefficient pipeline.

---

## polypaint-sweep

**Handler:** `handler_sweep.py`
**Binary:** `sweep` (solve mode)
**Route:** POST /sweep
**Memory:** 10240 MB (6 vCPUs)

Solves polynomial roots from a pre-computed coefficient file (one chunk). The solver reads coefficient records sequentially until EOF.

**Input:**
- `job_id`, `stripe_idx` — job and chunk identification
- `coeffs_key` — S3 key for coefficient file (required)
- `n_coeffs` — coefficient count
- `n_steps` — number of parameter steps in this chunk (preferred), or legacy `N`/`row_start`/`row_end`

**Status reporting:** Reports progress to DynamoDB via `report_status()` with task_id `sweep_{stripe_idx}`. Status progression: `started` -> `done` (or `error`). On success, `result_data` includes sweep metadata (`bin_size`, `compute_us`, `n_t`, `avg_iterations`).

**Output:** `s3_key`, `bin_size`, `n_t`, `degree`, `avg_iterations`, `n_procs`

**Invocation:** Dispatched asynchronously via the dispatch Lambda (fire-and-forget). The frontend polls `/check-status` with `task_prefix: 'sweep_'` every 3 seconds to track completion, collecting sweep metadata from the `results` array in the response.

Note: The legacy `handle_compute_only_stripe()` (grid mode, inline coefficient generation) was removed. All compute now flows through the chunked pipeline: `param_gen` → `coeffgen_chunked` → `solve`.

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
- `source` — `"roots"` (default) or `"coeffs"` for coeff bilevel viewport

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
| `/clean-render` | Delete render artifacts (.raw/.pix/.jpeg/.tif/.bits), preserve .bin |
| `/check-keys` | Poll S3 for expected file count (legacy) |
| `/check-status` | Poll DynamoDB task status (with optional `return_ids`) |
| `/detail` | Get job file count + viewport bounds |
| `/presign` | Generate presigned download URL |
| `/list-prefix` | List S3 keys under a prefix with optional suffix filter |
| `/head-keys` | Batch HEAD check: which of N specific S3 keys exist |

### /check-status — return_ids

When `return_ids: true` is passed, the response includes a `found_ids` array listing every `task_id` that has any DynamoDB record (done, error, or in-progress). This enables the frontend to compute exactly which task indices are missing (dispatched but never executed) and re-dispatch them.

Added to solve the 449/500 raster stall — see [Dispatch Resilience](#dispatch-resilience).

### /head-keys

Takes `{keys: ["renders/j/image_bilevel.tif", ...]}`, returns `{exists: ["renders/j/image_bilevel.tif"]}` — only keys that exist. Uses parallel HEAD requests (ThreadPoolExecutor, up to 20 workers).

Added because the previous artifact discovery used `/list-prefix` with `max_keys: 500`, which gets swamped by `stripe_*.bin` and `coeffs_*.bin` files on large jobs — the S3 listing is prefix-ordered, so the first 500 keys are all compute artifacts and the actual render outputs (`image_bilevel.tif`, `image_bilevel_preview.png`) never appear. The artifact panel showed nothing even after a successful render.

---

## polypaint-dispatch

**Handler:** `handler_dispatch.py`
**Route:** POST /dispatch-render
**Memory:** 1769 MB

Asynchronous fan-out: invokes target Lambdas in parallel using a 50-thread pool. Fire-and-forget (`InvocationType=Event`).

**Input:**
- `target` — "sweep", "raster", "finalize", "encode", "bilevel", or "bilevel_stitch"
- `jobs` — array of job specs to invoke

**Output:** `fired`, `total`, `errors`, `non_202`

### Non-202 tracking

If Lambda returns a status code other than 202 (e.g. 429 throttle), it is still counted as `fired` (the event may still execute) but logged in the `non_202` array. The dispatch Lambda has no way to know whether an async invocation will actually run — it only knows that Lambda accepted the event.

---

## polypaint-bilevel

**Handler:** `handler_bilevel.py`
**Binary:** `bilevel_raster` (static) + `bilevel_merge` (dynamic/libvips)
**Route:** POST /bilevel (dispatched async via dispatch Lambda)
**Memory:** 1769 MB + libvips layer, 10 GB /tmp
**Async retry:** 2 attempts, 3600s max event age

Single Lambda function handling two phases, routed by `phase` parameter:

### phase=raster

One Lambda per stripe. Downloads stripe `.bin`, runs `bilevel_raster` to project roots to per-tile packed bitsets, uploads non-empty `.bits` files to S3.

Cleans stale `/tmp/*.bits` files before each run (warm container reuse fix — previous invocation's leftover files would be uploaded as current stripe's data).

### phase=coeff_raster

Same as raster but for coefficient bilevel rendering. Uses `coeffs_bilevel_raster` binary. Different task prefix (`coeff_bilevel_raster_`) and S3 naming (`coeff_bits_s*_t*.bits`).

### phase=merge

One Lambda per tile. Downloads all stripe `.bits` files for one tile, runs `bilevel_merge merge` to OR bitsets into a single tile TIFF (1-bit CCITT G4). Supports configurable `bits_prefix` and `tile_prefix` for coeff vs root naming.

See [bilevel.md](bilevel.md) for architecture details.

---

## polypaint-bilevel-stitch

**Handler:** `handler_bilevel_stitch.py`
**Binary:** `bilevel_merge` (stitch mode)
**Route:** dispatched async via dispatch Lambda (target: "bilevel_stitch")
**Memory:** 6144 MB (~4 vCPUs for libvips multithreading), 10 GB /tmp

Separate Lambda from bilevel raster/merge for independent memory sizing. libvips is multithreaded and uses the extra vCPUs.

**Process:**
1. Downloads all tile TIFFs from S3
2. Runs `bilevel_merge stitch` with `--width`, `--height`, `--tile_size` for exact output dimensions
3. Produces tiled BigTIFF (or strip-based for small images) with CCITT G4
4. Generates 1024px preview PNG via libvips resize
5. Uploads final TIFF + preview PNG to S3

**Output:** `out_key`, `preview_key`, `file_size`, `dl_ms`, `stitch_ms`

### Why separate from polypaint-bilevel

The stitch phase needs more memory than raster/merge because libvips opens all tile TIFFs concurrently and uses multiple threads for encoding. Raster/merge are embarrassingly parallel (one Lambda per stripe/tile) and only need 1 vCPU. Making stitch a separate Lambda avoids over-provisioning the 500+ raster/merge invocations at 6144 MB each.

---

## polypaint-tiff-compat

**Handler:** `handler_tiff_compat.py`
**Binary:** `tiff_compat` (dynamic, libvips + libtiff)
**Route:** POST /tiff-compat
**Memory:** 4096 MB + libvips layer, 10 GB /tmp

Converts tiled bilevel TIFF to strip-based TIFF for macOS Preview compatibility. Preview.app cannot open tiled TIFFs.

**Process:**
1. Downloads source TIFF from S3
2. Runs `tiff_compat` — streaming conversion via VipsRegion (one tile row at a time, not full image in memory)
3. Auto-selects classic TIFF vs BigTIFF based on image dimensions
4. Uploads `*_compat.tif` to S3

**Binary implementation:** Uses `VIPS_ACCESS_SEQUENTIAL` to read input row-by-row via VipsRegion. Packs each scanline to 1-bit. Writes via libtiff directly (not vips_tiffsave) to produce strip-based layout.

**Why 4096 MB:** The scanline buffer for a 50K-wide image is small (6 KB per row), but libvips internal caching and libtiff overhead need headroom. Earlier attempts at 1769 MB worked for small images but OOMed on 50K.

---

## polypaint-png-export

**Handler:** `handler_png_export.py`
**Binary:** `png_export` (dynamic, libvips)
**Route:** POST /png-export
**Memory:** 4096 MB + libvips layer, 10 GB /tmp

Converts bilevel TIFF to 1-bit PNG.

**Process:**
1. Downloads source TIFF from S3
2. Runs `png_export` — sequential access via libvips, writes PNG with `bitdepth=1`
3. Uploads `*_bilevel.png` to S3

**Binary implementation:** `vips_image_new_from_file` with `VIPS_ACCESS_SEQUENTIAL`, then `vips_pngsave` with bitdepth=1.

---

## polypaint-deepzoom-export

**Handler:** `handler_deepzoom_export.py`
**Binary:** `dz_export` (dynamic, libvips with libarchive)
**Route:** POST /deepzoom-export
**Memory:** 4096 MB + libvips layer, 10 GB /tmp

Generates OpenSeadragon-compatible DeepZoom tile pyramid from TIFF.

**Process:**
1. Downloads source TIFF from S3
2. Runs `dz_export` — `vips_dzsave` with DZ layout, 256px PNG tiles, overlap=0
3. Parallel-uploads .dzi + all tile files to S3 (ThreadPoolExecutor, 50 workers)
4. Writes `meta.json` manifest with dzi_url, dimensions, timestamps
5. Public access via S3 bucket policy on `deepzoom/` prefix (no per-object ACL)

**Binary implementation:** `vips_dzsave(img, outBase, "layout", VIPS_FOREIGN_DZ_LAYOUT_DZ, "suffix", ".png", "tile-size", 256, "overlap", 0, NULL)`

**Critical dependency:** `vips_dzsave` requires **libarchive** (not libgsf as in older libvips versions). libvips 8.16.1 switched the dzsave backend from libgsf to libarchive. The layer must be built with `libarchive-devel` installed. See [libvips Layer](#libvips-layer) for the full dependency chain.

---

## polypaint-param-debug

**Handler:** `handler_param_debug.py`
**Binary:** `sweep` (param_dump mode) + `bilevel_merge`
**Route:** POST /param-debug
**Memory:** 1769 MB + libvips layer

Renders transformed parameter pairs as bilevel TIFF for visual debugging of parameter transforms.

**Process:**
1. Runs `sweep` in `param_dump` mode — outputs raw float32 transformed parameter pairs
2. Python rasters parameter points to a packed bitset (in-process, no C binary needed)
3. Runs `bilevel_merge merge` to convert bitset → TIFF with CCITT G4
4. Generates preview PNG via `bilevel_merge` with `--preview` flag
5. Uploads TIFF + preview to `debug/{job_id}/`

Uses the same transform code path as the real coeffgen pipeline — no Python reimplementation of transforms.

---

## Dispatch Resilience

### The problem: 449/500 raster stall

A bilevel render of 500 stripes dispatched all 500 async Lambda invocations successfully (all returned HTTP 202). But only 449 ever executed. The remaining 51 never wrote any DynamoDB status — not even "started". The render stalled for 10 minutes then timed out.

### Root cause

Three factors combined:

1. **Account concurrency limit: 1000.** 500 bilevel Lambdas at 1769 MB each, plus other polypaint Lambdas running concurrently (dispatch, storage, etc.), hit the concurrency ceiling.

2. **MaximumRetryAttempts: 0.** The bilevel Lambda's async invoke config had retries disabled. When Lambda throttles an async invocation (can't start it due to concurrency limits), it normally retries with backoff. With 0 retries, throttled events are silently discarded.

3. **MaximumEventAgeInSeconds: 300.** Events older than 5 minutes are dropped. Combined with 0 retries, any throttled invocation was permanently lost.

The dispatch Lambda returned `fired: 500` because all 500 got HTTP 202 (accepted into Lambda's internal queue). But 51 were never dequeued.

### Evidence

- `check-status` returned `statuses={"done":449}` — only "done", no "started", no "error"
- The 51 missing tasks never wrote a single DynamoDB record
- `handler_bilevel.py` calls `report_status(job_id, task_id, "started")` as its first action — so the functions were never invoked at all
- All 10 dispatch batches completed with 500/500 fired, 0 errors, 0 non_202
- First poll at 14.9s already had 449 done, then zero progress for 10 minutes

### Three-layer fix

**Layer 1: AWS retry config (belt)**

```bash
aws lambda put-function-event-invoke-config \
  --function-name polypaint-bilevel \
  --maximum-retry-attempts 2 \
  --maximum-event-age-in-seconds 3600
```

Throttled invocations now get 2 retries with exponential backoff instead of being silently dropped. Codified in `deploy.sh` — the bilevel Lambda gets different retry settings from other async Lambdas (which stay at 0 retries to prevent retry storms on actual errors).

Other Lambdas (raster, finalize, bilevel-stitch) keep `--maximum-retry-attempts 0 --maximum-event-age-in-seconds 300` because their failure modes are different — an error in pixassemble should not retry, it should fail fast.

**Layer 2: Wave dispatch (suspenders)**

`_bilevelDispatchAndPoll()` in `index.html` replaces the old "dispatch all, then poll" pattern. It caps in-flight invocations at `MAX_INFLIGHT = 200`:

```
cursor = 0, lastPollDone = 0
while cursor < total:
    inFlight = cursor - lastPollDone
    canSend = min(MAX_INFLIGHT - inFlight, total - cursor)
    dispatch jobs[cursor..cursor+canSend]
    cursor += canSend
    if more to send: poll, update lastPollDone
```

With 200 max in-flight and 1000 account concurrency, there's 800 slots remaining for other Lambdas. The old approach dispatched all 500 at once.

**Layer 3: Missing-task re-dispatch (safety net)**

After 45 seconds of no progress (`GRACE_MS`), the poll loop requests `return_ids: true` from `/check-status`. The response includes every `task_id` found in DynamoDB. The frontend computes the missing set:

```javascript
const foundSet = new Set(check.found_ids);
const missingIndices = [];
for (let i = 0; i < expected; i++) {
    if (!foundSet.has(`${taskPrefix}${i}`)) missingIndices.push(i);
}
```

If any are missing, it re-dispatches only those jobs (max 2 re-dispatch rounds). The log shows exact indices:

```
⚠ BiLevel raster: 51 missing [449,450,451,...499], re-dispatching (attempt 1/2)
```

### Idempotency

Re-dispatch can cause a task to run twice if the original was merely delayed (not dropped). This is safe because:
- Raster: writes the same `.bits` files to the same S3 keys (deterministic)
- Merge: ORs the same bitsets, overwrites the same tile TIFF
- DynamoDB: `report_status` is a `put_item` (upsert), not conditional

### Call sites

The `_bilevelDispatchAndPoll` helper replaced all 4 dispatch+poll blocks:
1. Root bilevel raster (`bilevel_raster_` prefix)
2. Root bilevel merge (`bilevel_merge_` prefix)
3. Coeff bilevel raster (`coeff_bilevel_raster_` prefix)
4. Coeff bilevel merge (`coeff_bilevel_merge_` prefix)

Stitch is a single Lambda (not fan-out), so it doesn't need wave dispatch.

### Tests

`tests/test_dispatch_resilience.py` — 28 tests:

| Class | Tests | What it covers |
|-------|-------|----------------|
| `TestCheckStatusReturnIds` | 5 | `return_ids` flag: default off, explicit true/false, 449/500 gap detection, pagination |
| `TestHeadKeys` | 6 | `/head-keys` endpoint: all exist, none, mixed, empty, single, parallel execution |
| `TestDispatchBilevelTarget` | 4 | dispatch bilevel/stitch targets, non-202 tracking, 200-job batch |
| `TestStorageRouting` | 2 | `/head-keys` route wired, unknown route 400 |
| `TestMissingTaskDetection` | 6 | set-diff logic: none missing, all missing, contiguous 449/500 gap, scattered, different prefixes |
| `TestWaveDispatchLogic` | 5 | wave simulation: single wave, multi-wave, throttled, exact inflight, full inter-wave completion |

---

## Artifact Discovery

### The problem: render succeeds but panel is empty

After a successful bilevel render, the artifact panel showed no preview image and no download buttons. The render log showed success with timing.

### Root cause

`discoverArtifacts()` used `/list-prefix` with `max_keys: 500` to enumerate all objects under `renders/{jobId}/`. For a large job with 500 stripes and 500 coefficient files, the first 500 S3 keys (alphabetically ordered) are all `coeffs_0000.bin`, `coeffs_0001.bin`, ..., `stripe_0.bin`, `stripe_1.bin`, etc. The actual render outputs (`image_bilevel.tif`, `image_bilevel_preview.png`) never appear in the listing.

The code then filtered the presign list against this incomplete set:
```javascript
const existing = keysToCheck.filter(k => existingKeys.has(k));
```
Result: empty. No presigns generated. No buttons rendered.

### Fix

Replaced `/list-prefix` with `/head-keys`. The frontend sends the 8 exact artifact keys it cares about, and the backend does parallel HEAD requests to check existence. O(8) HEAD requests instead of O(500+) listing, and always finds the render outputs regardless of how many compute artifacts exist.

### Second bug: status wipe

`refreshRenderArtifacts()` set `render-status` text to `''` on success, wiping the timing summary that `runBilevelPipeline()` had just written. Fixed by saving/restoring the prior status text.

---

## libvips Layer

**Layer ARN:** `arn:aws:lambda:us-east-1:710848990594:layer:polypaint-libvips:9`
**Build script:** `lambda/build-libvips-layer.sh`
**Size:** ~18 MB zipped

### What's in it

libvips 8.16.1 built from source with these format support libraries:

| Library | Purpose | Required by |
|---------|---------|-------------|
| libjpeg-turbo | JPEG load/save (`jpegsave`) | encode (raw2jpeg) |
| libpng | PNG load/save (`pngsave`) | png_export, dz_export tiles |
| libtiff | TIFF load/save (`tiffsave`, CCITT G4) | bilevel_merge, tiff_compat |
| libarchive | DeepZoom pyramid save (`dzsave`) | dz_export |

All shared library dependencies are bundled in the layer zip (not relying on Lambda runtime):
glib-2.0, gobject-2.0, gio-2.0, gmodule-2.0, expat, ffi, pcre2-8, z, mount, selinux, blkid, jpeg, png16, tiff, webp, jbig, xml2, bz2, archive.

### Build options

```
meson setup --buildtype=release -Dmodules=disabled -Dintrospection=disabled
  -Dtiff=enabled
  -Dheif=disabled -Dpoppler=disabled -Drsvg=disabled -Dopenexr=disabled
  -Dfits=disabled -Dimagequant=disabled -Dcgif=disabled -Dspng=disabled
  -Dwebp=disabled -Dpdfium=disabled -Dnifti=disabled -Dopenslide=disabled
  -Dexif=disabled -Dlcms=disabled
```

libarchive is auto-detected by meson (not a meson option). It must be available via `libarchive-devel` at configure time for `dzsave` to be compiled in. Meson silently disables dzsave if libarchive is missing.

### Layer version history

| Version | Change | What broke |
|---------|--------|------------|
| v7 | Built with `-Dtiff=disabled` | `tiffsave` not found — bilevel pipeline couldn't write TIFF |
| v8 | Rebuilt with `-Dtiff=enabled`, bundled libtiff+libwebp+libjbig | Fixed TIFF. But `dzsave` still missing — no libarchive |
| v9 | Added `libarchive-devel` to build deps, bundled libarchive.so | Fixed DeepZoom. All 4 ops confirmed: tiffsave, pngsave, jpegsave, dzsave |

### The dzsave saga

libvips 8.16.1 switched the backend for `vips_dzsave` from **libgsf** to **libarchive**. Older documentation and Stack Overflow answers say dzsave needs libgsf, which led to building libgsf 1.14.52 from source (not available in Amazon Linux 2023 repos). After libgsf was built, installed, and bundled, dzsave was still missing.

The actual meson configure output revealed the truth:
```
image pyramid save with libarchive  : NO
```

Not libgsf. libarchive. Available as `libarchive-devel` directly from the Amazon Linux 2023 repos — no source build needed.

### Pre-deploy smoke tests

`deploy.sh` now runs runtime smoke tests inside Docker after compiling binaries, before packaging any Lambda zips:

1. **Operation probe** — a compiled C program calls `vips_type_find("VipsForeignSave", op)` for tiffsave, pngsave, and dzsave. Hard fail if any returns 0.

2. **Test TIFF creation** — creates an 8×8 TIFF via `vips_black` + `vips_tiffsave`. Verifies libvips can actually write files, not just register operations.

3. **dz_export smoke** — runs the actual dz_export binary on the test TIFF. Asserts `.dzi` file and at least 1 PNG tile exist. This is the exact binary that ships to Lambda.

4. **png_export smoke** — runs png_export on the test TIFF. Asserts output PNG exists.

5. **tiff_compat smoke** — runs tiff_compat on the test TIFF. Asserts output TIFF exists.

All tests run inside the same Docker container that compiled the binaries, using the same `/opt/lib` from the layer build. If any test fails, deploy aborts before uploading anything.

```
--- Runtime smoke tests ---
  OK: tiffsave
  OK: pngsave
  OK: dzsave
  test TIFF written
  dz_export: .dzi + 4 tiles OK
  png_export OK
  tiff_compat OK
--- All smoke tests passed ---
```

This catches the exact failure mode that caused dzsave to be missing: "binary compiles against libvips headers, but the runtime library doesn't have the operation registered." Compilation succeeds because `vips_dzsave` is declared in the header — it's a function call, not a compile-time feature check. The function exists, but at runtime it fails because the dzsave operation was never registered in the type system (because libarchive wasn't present at libvips build time).

---

## Deploy

### deploy.sh

Single script handles everything: JS syntax check, binary compilation (static + Docker ARM64), runtime smoke tests, Lambda packaging, code/config updates, API Gateway routes, S3 upload.

```bash
./deploy.sh create   # First-time: create all Lambdas + API Gateway
./deploy.sh update   # Update code + config for all existing Lambdas
```

### Async invoke config (in deploy.sh)

| Lambda | MaxRetryAttempts | MaxEventAge | Rationale |
|--------|-----------------|-------------|-----------|
| polypaint-raster | 0 | 300s | Errors should fail fast, not retry |
| polypaint-finalize | 0 | 300s | Same — OOM or bad input won't fix on retry |
| polypaint-bilevel | **2** | **3600s** | Concurrency throttle drops need retry |
| polypaint-bilevel-stitch | 0 | 300s | Single invocation, not fan-out |

The bilevel Lambda is the only one with retries enabled because it's the only one dispatched at fan-out scale (up to 500 concurrent). The others are either single invocations or have different failure modes where retry would be harmful.

### Lambda count: 15

| Lambda | Handler | Binary | Memory | Layer |
|--------|---------|--------|--------|-------|
| polypaint-sweep | handler_sweep.py | sweep | 10240 MB | — |
| polypaint-coeffgen | handler_coeffgen.py | sweep | 1769 MB | — |
| polypaint-raster | handler_raster.py | roots2pix | 1769 MB | — |
| polypaint-finalize | handler_finalize.py | pixassemble | 1769 MB | — |
| polypaint-encode | handler_encode.py | raw2jpeg | 1769 MB | libvips |
| polypaint-viewport | handler_viewport.py | — | 512 MB | — |
| polypaint-storage | handler_storage.py | — | 512 MB | — |
| polypaint-dispatch | handler_dispatch.py | — | 1769 MB | — |
| polypaint-preview | handler_preview.py | — | 1024 MB | — |
| polypaint-bilevel | handler_bilevel.py | bilevel_raster + bilevel_merge | 1769 MB | libvips |
| polypaint-bilevel-stitch | handler_bilevel_stitch.py | bilevel_merge | 6144 MB | libvips |
| polypaint-param-debug | handler_param_debug.py | sweep + bilevel_merge | 1769 MB | libvips |
| polypaint-tiff-compat | handler_tiff_compat.py | tiff_compat | 4096 MB | libvips |
| polypaint-png-export | handler_png_export.py | png_export | 4096 MB | libvips |
| polypaint-deepzoom-export | handler_deepzoom_export.py | dz_export | 4096 MB | libvips |
