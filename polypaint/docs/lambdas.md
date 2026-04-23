# Lambda Handlers

This file focuses on the current compute, render, storage, export, and workflow-entry Lambdas.
The exact deployed inventory changes over time; [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh) is the source of truth for the full current set.

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
- `job_id`, `chunk_idx` — job and chunk identification
- `coeffs_key` — S3 key for coefficient file (required)
- `n_coeffs` — coefficient count
- `n_steps` — number of parameter steps in this chunk (preferred), or legacy `N`/`row_start`/`row_end`

**Status reporting:** Reports progress to DynamoDB via `report_status()` with task_id `sweep_{chunk_idx}`. Status progression: `started` -> `done` (or `error`). For compatibility, result payloads still include both `chunk_idx` and `stripe_idx` aliases. On success, `result_data` includes sweep metadata (`bin_size`, `compute_us`, `n_t`, `avg_iterations`).

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

Converts one logical section of root data into one sparse global raw-score
fragment.

**Input:**
- `job_id`, `section_idx`, `solve_source_manifest` — root data location
- `pix` — square output size
- `min_re`, `max_re`, `min_im`, `max_im` — exact viewport bounds
- `degree` — polynomial degree
- `solve_score_chain`, `solve_score_clip_key` — score program and clip artifact
- `fragment_prefix` — output prefix for `section_{idx}.frag`

**Process:**
1. Reads the requested logical section through the source manifest
2. Runs `roots2pix_mt` which transforms each root (re, im) → pixel (px, py):
   ```
   px = floor((re - min_re) * pix / (max_re - min_re))
   py = floor((max_im - im) * pix / (max_im - min_im))
   ```
3. Deduplicates with one full-image shared bitset
4. Writes one sparse fragment: 5-byte entries `[uint32 global_pixel_idx, uint8 score]`
5. Uploads `fragments/section_{idx}.frag`

**Output:** `fragment_files_uploaded`, `fragment_bytes_uploaded`, `roots_plotted`, `roots_clipped`, `raster_us`

### Color Raster Contract

The active Color render path is solve-score-program based. The frontend sends a
solve-score chain, the planner compiles it, and `roots2pix_mt` receives the
compiled program contract:

- `--score_metrics=...`
- `--score_clip_los=...`
- `--score_clip_his=...`
- `--score_program=...`
- optional source manifests for coefficient or parameter-backed metric slots

Lambda: `polypaint-solve-proximity` (handler: `handler_solve_proximity.py`)
Binary: `solve_proximity_stats` (clip + hist modes)
Shared header: `solve_score.h` (metric implementations used by both binaries)
Artifacts: solve-score clip JSON plus sparse raw-score section fragments

**Metrics:**

| Metric | Score formula | Interpretation |
|--------|--------------|----------------|
| proximity | `-0.5 * log10(min d²)` | Near-collision (min pairwise distance) |
| crowding | `mean(-0.5 * log10(d²))` over all pairs | Global clustering |
| spread | `0.5 * log10(RMS radius²)` | Cloud size from centroid |
| anisotropy | `log10(λ_max / λ_min)` of covariance | Elongation (line vs circle) |
| area | `0.5 * log10(λ_max × λ_min)` of covariance | 2D cloud area |
| clusteriness | `s1_max - median(s1)` where s1 = NN scores | One exceptional local collision vs typical |
| shelliness | `-log10(σ_ρ / (μ_ρ + ε) + ε)` | Thin-shell / ring detector |
| outlierness | `log10((ρ_max + ε) / (ρ_median + ε))` | Radial outlier strength |
| nn_variation | `stddev(s1)` where s1 = NN scores | Heterogeneity of local spacing |
| real_axis_proximity | `-log10(median(\|im\|) + ε)` | Closeness to real axis |

Legacy `color=solve_proximity` is accepted and coerced to `solve_score` with `metric=proximity`.

### Root Matching

| Mode | Complexity | Notes |
|------|-----------|-------|
| none | O(1) | No tracking, roots in solver order |
| greedy | O(n^2) | Nearest-neighbor matching |
| hungarian | O(n^3) | Optimal bipartite matching, auto-downgrades to greedy if degree > 40 |

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
| `/clean-render` | Delete family-scoped render intermediates, preserve immutable family artifacts |
| `/check-keys` | Poll S3 for expected file count (legacy) |
| `/check-status` | Poll DynamoDB task status (with optional `return_ids`) |
| `/detail` | Get job file count + viewport bounds |
| `/render-summary` | Get Render family inventories, calc summary, latest DeepZoom pointer |
| `/list-palettes` | List saved palette variants for one job |
| `/delete-palette` | Delete one saved palette variant |
| `/delete-render-artifact` | Delete one immutable Color/BiLevel/Coeffs artifact variant |
| `/list-deepzoom` | List DeepZoom exports server-side |
| `/delete-prefix` | Delete all S3 objects under a safe prefix (currently used for DeepZoom cleanup) |
| `/presign` | Generate presigned download URL |
| `/list-prefix` | List S3 keys under a prefix with optional suffix filter |
| `/head-keys` | Batch HEAD check: which of N specific S3 keys exist |

### /check-status — return_ids

When `return_ids: true` is passed, the response includes a `found_ids` array listing every `task_id` that has any DynamoDB record (done, error, or in-progress). This enables the frontend to compute exactly which task indices are missing (dispatched but never executed) and re-dispatch them.

Added to solve the 449/500 raster stall — see [Dispatch Resilience](#dispatch-resilience).

### /render-summary

Current Render refresh route.

Returns:

- `schema_version`
- `calc`
- `families`
- `deepzoom_latest`

The Render tab now uses this one route instead of browser-side discovery fanout.

### /head-keys

Still available as a general storage helper.

It no longer drives the Render refresh path.
That responsibility moved to `/render-summary`.

---

## polypaint-dispatch

**Handler:** `handler_dispatch.py`
**Route:** POST /dispatch-render
**Memory:** 1769 MB

Asynchronous fan-out: invokes target Lambdas in parallel using a 50-thread pool. Fire-and-forget (`InvocationType=Event`).

**Input:**
- `target` — one of the targets published in `api_manifest.json` / `handler_dispatch.FUNCTIONS`
- `jobs` — array of job specs to invoke

**Output:** `fired`, `total`, `errors`, `non_202`

### Non-202 tracking

If Lambda returns a status code other than 202 (e.g. 429 throttle), it is still counted as `fired` (the event may still execute) but logged in the `non_202` array. The dispatch Lambda has no way to know whether an async invocation will actually run — it only knows that Lambda accepted the event.

---

## polypaint-bilevel

**Handler:** `handler_bilevel.py`
**Binary:** `bilevel_section_raster`, `coeffs_bilevel_raster`, `assemble_greyscale`, `raw_to_bilevel`
**Route:** POST /bilevel (dispatched async via dispatch Lambda)
**Memory:** 1769 MB + libvips layer, 10 GB /tmp
**Async retry:** 2 attempts, 3600s max event age

Single Lambda function handling sparse-fragment bilevel phases, routed by `phase` parameter:

### phase=section_raster

One Lambda per logical root section. Downloads the root byte spans, runs `bilevel_section_raster`, and uploads one sparse fragment containing `[u32le global_pixel_idx][u8 score=1]` records.

### phase=coeff_raster

One Lambda per logical coefficient section. Downloads coefficient byte spans, runs `coeffs_bilevel_raster`, and uploads the same sparse global-index fragment format.

### phase=finalize

Presigns sparse fragments, runs `assemble_greyscale` into a dense raw occupancy image, then runs `raw_to_bilevel` to write the TIFF and preview.

### phase=from_raw_color

Thresholds a fused Color raw sidecar into a bilevel TIFF.

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
3. Runs `bilevel_merge assemble` to convert the bitset → TIFF with CCITT G4
4. Generates preview PNG in the same `bilevel_merge assemble` invocation with `--preview`
5. Uploads TIFF + preview to `debug/{job_id}/`

Uses the same transform code path as the real coeffgen pipeline — no Python reimplementation of transforms.

---

## Dispatch Resilience

### The problem: 449/500 raster stall

A bilevel render of 500 chunks dispatched all 500 async Lambda invocations successfully (all returned HTTP 202). But only 449 ever executed. The remaining 51 never wrote any DynamoDB status — not even "started". The render stalled for 10 minutes then timed out.

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
- Section raster: writes the same sparse `.frag` file to the same S3 key (deterministic)
- Finalize: rewrites the same final artifact keys
- DynamoDB: `report_status` is a `put_item` (upsert), not conditional

### Call sites

Render bilevel and coeff-bilevel fan-out are Step Functions Map states now. The old browser-side `_bilevelDispatchAndPoll` path is not the active render path.

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

### Current model

Artifact discovery for the Render tab is now centered on:

- `POST /render-summary`

The frontend no longer builds the panel by probing individual artifact keys.

### Current shape

`/render-summary` returns:

- `schema_version`
- `calc`
- `families`
- `deepzoom_latest`

`families` contains immutable artifact inventories for:

- `color`
- `bilevel`
- `coeffs`
- `palette`

Legacy top-level render outputs are surfaced as synthetic `legacy_*` entries so old artifacts remain visible and deletable.

### Why this replaced the older approach

The older browser-side discovery flows:

- did too much network fanout
- depended on exact key probing and earlier on prefix listing
- did not fit the immutable family-catalog UI

The current model moves discovery to the storage Lambda and gives the browser a single summary response.

### Related UI behavior

`refreshRenderArtifacts()` now rebuilds:

- family tabs
- family catalogs
- selected-artifact viewer

from `render-summary`, and preserves selection where possible.

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

The bilevel Lambda is the only one with retries enabled because it's the only one dispatched at fan-out scale (up to 500 concurrent). The others are either single invocations or have different failure modes where retry would be harmful.

### Inventory note

This document is not a frozen count of every deployed Lambda.
Recent workflow work added dedicated starter/status/plan helpers for Render and Palette.
For the exact current inventory, names, memory settings, and attached layers, use:

- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
