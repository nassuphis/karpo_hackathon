# Code Review 34: Color Raster, Finalization, and Preview Performance

Date: 2026-07-13

Reviewed HEAD: `649a73c` (`CR33 F3: shared coeff vector kernels + direct poly->poly in-place paths`)

Review mode: production-path trace, native source audit, orchestration audit, local libvips macrobenchmarks, and disposable source prototypes

## Verdict

The palette lookup itself is not the reason Color rendering feels slow. The shipped path has four materially different costs that the UI currently groups under "rendering" or "colorization":

1. Solve-score feature extraction and program evaluation for every saved solve.
2. Projection of every root into image pixels, collision handling, and sparse-fragment construction.
3. S3 upload, download, and collision-resolving assembly of all raster fragments into dense raw pixels.
4. Equalization, palette mapping, full-image encoding, preview encoding, persistence, and optional associated-palette finalization.

The local encoder probes show that scalar palette mapping plus JPEG encoding is measured in tenths of a second at 4096x4096 on the review host. It is not a credible explanation for a minutes-long render. Heavy solve metrics, raster projection, section fan-out, fragment traffic, and final assembly are the likely large terms. CR33 addresses the solve-feature term. CR34 covers the raster and post-raster pipeline around it.

There are several concrete improvements:

1. Fix telemetry before tuning. Current raster timing adds worker durations together and presents the sum as `raster_us`; finalization records `encode_ms=0` unconditionally. Production status cannot currently tell CPU from S3 or mapping from encoding.
2. Make pixel-collision nondeterminism an explicit throughput contract. Within one raster Lambda, the first thread to set an atomic bit wins, so thread scheduling can change collided-pixel colors. This is acceptable when speed is preferable to byte reproducibility, but the allowed variation, safety invariants, cache behavior, and tests must say so.
3. Stop building associated palettes twice. Every raster section already emits dense step scores, but it also emits a second sparse associated-palette fragment. Finalize downloads and assembles both. The existing `step_scores_to_palette_raw` binary performs the exact serpentine remap needed to derive the palette from the retained step scores.
4. Shorten the serial finalization chain. Raw upload, step-score upload, main image upload, and associated-palette work are mostly independent but currently block one another.
5. Generate Color and Palette previews directly as JPEG. The current path encodes and uploads PNG, then a migration job downloads it and creates the JPEG that the walls actually prefer. The baked wall then creates another JPEG pyramid. Direct JPEG is a real cleanup and a large wall-bandwidth win, although only a small render-time win.
6. Make sectioning a throughput decision rather than an accidental memory side effect. `raster_workers` is only a Map concurrency ceiling; automatic sectioning creates the minimum number of sections required by a stale 4096 MB planning assumption even though the deployed raster Lambda is 10240 MB.
7. Benchmark projection and assembler changes against production-shaped fragments. Zero-rotation projection, bounds-before-cast, safe any-contender-wins atomics, preallocation, and removing one mutex acquisition per fragment record are all credible targets. Collision-free fixtures still require byte parity; deliberate-collision fixtures require valid-winner and memory-safety checks rather than identical hashes.

Do not start by replacing the 256-entry palette LUT, adding more Lambda workers blindly, JIT-compiling the color map, or rewriting libvips. The scalar LUT path is already compact. More sections can increase duplicate pixels and fragment traffic, and the current timings cannot show whether added concurrency helps the critical path.

## Scope

This review covers the active fused Color path and the related raw-recolor and wall-preview paths:

| Layer | Primary code |
|---|---|
| Render planning and sectioning | `lambda/handler_render_plan.py`, `lambda/logical_sections.py` |
| Raster worker | `lambda/handler_raster_mt.py`, `lambda/roots2pix_mt.c`, `lambda/solve_score.h` |
| Fragment assembly and final output | `lambda/handler_finalize_mt.py`, `lambda/assemble_greyscale.c`, `lambda/raw_score_render.py`, `lambda/score_raw_render.c` |
| Raw recolor / repalette | `lambda/color_recolor_raw.py` |
| Associated palette extraction | `lambda/step_scores_to_palette_raw.c`, `lambda/handler_extract_palette_from_step_scores.py` |
| Preview migration and wall consumption | `scripts/migrate_preview_jpg.py`, `lambda/handler_storage.py`, `lambda/handler_wall_pyramid.py`, `lambda/wall_dz.c`, `js/13-artifact-mosaics.js` |

Coeff, Param, and Solve Score VM internals are covered by CR31-CR33. This document references Solve Score only where its cost is paid inside the raster path.

The relevant Color pipeline source files were clean during this review. The worktree had unrelated user changes in deployment and Solve Score files plus untracked `code-review-33.md`; none were modified or reverted.

## Evidence Levels

Each finding is classified as one of:

- **Measured locally:** production source or a minimal disposable variant was timed on the review host.
- **Directly traced:** the active production call chain and data flow prove the duplicated or serialized work.
- **Candidate:** the hot operation is visible in source, but a production-shaped Graviton A/B is still required.

Local timings are not Lambda forecasts. They establish relative costs and eliminate implausible explanations.

## Current Production Flow

### 1. Plan and fan-out

`handler_render_plan.py` compiles the Solve Score program and calls `compute_safe_sectioning(...)`. In `logical_sections_auto`, the returned `computed_section_count` is the minimum count needed to fit the source rows inside a memory budget. The plan then builds exactly that many Map items.

The Step Functions `ColorRasterMap` has `MaxConcurrencyPath=$.plan.raster.workers`, but this is only a ceiling. Ten requested workers do not create ten items. If automatic sectioning creates one item, the Map invokes one raster Lambda.

### 2. Raster each logical section

Each `handler_raster_mt` invocation:

1. Resolves root, coefficient, and parameter spans.
2. Writes presigned multispan manifests.
3. Runs `roots2pix_mt` with the requested native thread count.
4. Uploads a sparse main fragment.
5. Optionally uploads a sparse associated-palette fragment.
6. Uploads dense per-solve `step_scores.raw` data for one- or three-channel programs.

Inside `roots2pix_mt`, each solve is scored once. The resulting output byte or bytes are then reused for all roots in that solve. This part is already correctly fused.

The native work is approximately:

```text
S * (solve-feature cost + score-program cost + root-transform cost)
  + S * degree * projection/collision cost
```

where `S` is the number of solves in the section. Render pixel size mainly affects the global bitset, collision rate, final dense image, and encoding. It does not remove the per-solve feature and per-root projection work.

### 3. Finalize

`handler_finalize_mt` currently executes this critical path:

1. Download and assemble all main sparse fragments into dense raw pixels.
2. Write the histogram sidecar.
3. Serially download and concatenate every section's step-score object.
4. Upload the combined step-score object.
5. Render and encode the main image and PNG preview.
6. Upload the dense main raw object and its sidecar.
7. If requested, download and assemble all associated-palette sparse fragments.
8. Render and upload associated-palette raw, image, PNG preview, and metadata.
9. Upload the main image, preview, and overlay metadata.
10. Mark the task done.

Several independent network and CPU stages are therefore serialized.

### 4. Wall conversion

New Color and Palette artifacts still store `preview.png`. `scripts/migrate_preview_jpg.py` later downloads each PNG, normalizes it to at most 512 pixels, encodes `preview.jpg` at quality 92 with 4:4:4 chroma, uploads it, and records `preview_jpg_*` metadata.

The mosaic manifest prefers that JPEG sibling. The default wall view then uses a server-generated DeepZoom wall whose tiles are JPEG again. A non-default sort/filter falls back to direct per-artifact tile URLs.

There is no browser-side PNG-to-JPEG conversion. It is a separate migration, followed by a separate wall-pyramid encode.

## Local Encoder Measurements

### Method

- Host: Apple M3 Max, arm64.
- libvips: 8.17.1.
- Build: current `lambda/score_raw_render.c`, `-O3`.
- `VIPS_CONCURRENCY=6`, matching the approximate vCPU ceiling of the 10240 MB Lambda tier.
- Input: 4096x4096 scalar byte raster with high-entropy values, identity equalization LUT, Inferno palette.
- Reported values: warm wall-time medians from repeated runs.
- JPEG-preview result: disposable variant changing only the preview saver to quality 92, 4:4:4 JPEG.

| Output | Median wall time | Preview size |
|---|---:|---:|
| Full JPEG, no preview | `0.18 s` | n/a |
| Full JPEG + current PNG preview | `0.19 s` | `611 KB` |
| Full JPEG + direct JPEG preview | `0.18 s` | `147 KB` |
| Full PNG, no preview | `1.58 s` | n/a |

Interpretation:

- Direct JPEG preview saves only about 10 ms in this isolated encoder probe, but shrinks this 512px preview by 4.2x.
- Full PNG encoding was about 8.8x slower than full JPEG on the same high-entropy source.
- Content changes compression ratios substantially. The measured wall migration remains the better storage/network evidence: it estimated roughly 2.2 GB of PNG preview reads and a several-fold JPEG reduction across existing artifacts.

Three-channel 4096x4096 warm medians:

| Interpretation | Median wall time |
|---|---:|
| RGB, zero-background replacement | `0.11 s` |
| RGB, no zero-background replacement | `0.09 s` |
| RGB LUT | `0.12 s` |
| HSV LUT | `0.16 s` |
| HSV | `0.17 s` |

These are synthetic local numbers, but they establish an important bound: the final 256-entry palette lookup is not a minutes-long operation. If production spends minutes before a Color artifact appears, the investigation should start with Solve feature extraction, source downloads, root projection, fragment uploads, final assembly, and serialized S3 persistence.

### Materialization probe

The current scalar libvips graph has two sinks: full-image save and preview save. A disposable `vips_image_copy_memory` materialization reduced warm process CPU from roughly `0.26 s` to `0.21 s` with a PNG preview, but did not materially improve the `0.19 s` wall time. This confirms some repeated lazy-graph evaluation, but it is not a first-priority optimization and raises peak memory.

## Findings Summary

Severity is expected impact or correctness risk in the performance work, not a claim that every item dominates every render.

| ID | Severity | Finding | Evidence |
|---|---|---|---|
| F1 | HIGH | Raster and finalizer telemetry cannot identify the real bottleneck and includes misleading values. | Direct trace |
| F2 | HIGH design consequence | Pixel ownership is intentionally allowed to be nondeterministic at collisions; this changes reproducibility, cache, publication, and test contracts. | Direct trace |
| F3 | HIGH | Automatic sectioning is memory-minimum sectioning, not throughput tuning, and uses a stale memory model. | Direct trace + computed examples |
| F4 | HIGH | Associated palettes are emitted and assembled twice even though retained step scores contain the exact dense source. | Direct trace |
| F5 | HIGH | Independent finalization uploads and associated-palette work are serialized on the critical path. | Direct trace |
| F6 | MEDIUM | Color/Palette previews are encoded as PNG, later migrated to JPEG, then often encoded into a JPEG wall pyramid. | Measured + direct trace |
| F7 | MEDIUM | The root projection loop misses common no-rotation and nonnegative-cast fast paths and performs one atomic claim per in-bounds root. | Candidate |
| F8 | MEDIUM | `assemble_greyscale` takes a striped mutex for every fragment record and mixes network workers with contended merge workers. | Candidate |
| F9 | MEDIUM | Step-score concatenation performs serial S3 GETs and blocks rendering before uploading the combined object. | Direct trace |
| F10 | MEDIUM | Three-channel conversions are single-threaded loops with avoidable full-image allocations and copies. | Measured + direct trace |
| F11 | MEDIUM when selected | Full PNG is an expensive output mode for noisy art; the default JPEG path is already much faster. | Measured |
| F12 | LOW/MEDIUM | The preview sink can re-evaluate the lazy scalar graph; materialization reduces CPU but not local wall time. | Disposable prototype |
| F13 | LOW/MEDIUM | Raw recolor is architecturally good, but legacy histogram fallback and associated-palette handling can dominate it. | Direct trace |
| F14 | MEDIUM | There is no durable benchmark covering raster projection, fragment merge, final encode modes, or the full Lambda critical path. | Test/benchmark inventory |
| F15 | LOW | Subprocess timeouts are individually 600 seconds inside a 900-second Lambda, despite multiple sequential subprocesses. | Direct trace |

## F1 - Performance Telemetry Is Misleading

**Severity:** HIGH
**Evidence:** directly traced
**Locations:** `lambda/roots2pix_mt.c:277-559`, `lambda/handler_raster_mt.py:613-627`, `lambda/handler_finalize_mt.py:684-758`, `lambda/raw_score_render.py:82-129`

### Current behavior

Each raster thread records its own download and native duration. `roots2pix_mt` adds those durations across all workers and returns the sums as `download_us` and `native_us`.

The Python handler separately measures subprocess wall time as `native_wall_us`, but then discards it whenever the native JSON contains `native_us`, which it always does:

```python
native_wall_us = ...
perf["native_us"] += int(raster_meta.get("native_us", native_wall_us))
```

Four workers each spending one second in parallel can therefore report about four seconds of `native_us` for a roughly one-second wall stage. That is useful as aggregate worker occupancy, but it is not raster latency and must not be labeled or consumed as such.

Finalization has the inverse problem. `render_ms` includes raw loading, equalization mapping, palette mapping, full-image encoding, resize, and preview encoding. `encode_ms` is then set to zero unconditionally. The two status transitions `rendered_rgb_tiles` and `encoded` carry no separable measurements.

Raw recolor reports phase names but not download, histogram, render, copy, or upload durations.

### Required fix

Add wall and aggregate fields with explicit names:

- Raster handler: `subprocess_wall_us`, `worker_download_sum_us`, `worker_native_sum_us`.
- Native raster: record process-level `download_wall_us`, `compute_wall_us`, `fragment_write_us`, and total wall time. Per-worker sums can remain as utilization diagnostics.
- Finalizer: `assemble_download_ms`, `assemble_merge_ms`, `step_scores_download_ms`, `step_scores_upload_ms`, `raw_upload_ms`, `full_encode_ms`, `preview_encode_ms`, `image_upload_ms`, and associated-palette equivalents.
- `score_raw_render`: either return per-sink timings or expose separate benchmark modes. Because libvips is lazy, labels must describe evaluated sinks rather than pretending mapping and encoding are trivially separable.
- Persist one concise terminal timing object. Do not infer latency by subtracting unrelated DDB update timestamps.

This is Phase 0. Optimization without it is guesswork.

## F2 - Collision Nondeterminism Is an Explicit Throughput Contract

**Classification:** HIGH-consequence design decision, not a correctness bug by itself
**Evidence:** directly traced
**Location:** `lambda/roots2pix_mt.c:210-215`, `:397-552`; `lambda/assemble_greyscale.c:274-318`

### Current behavior

All worker threads share one pixel bitset. For each in-bounds root:

```c
old = __atomic_fetch_or(&pixelWords[wordIdx], bit, __ATOMIC_RELAXED);
if ((old & bit) == 0) emit(pixel, score);
```

If roots from different solves land on the same pixel, the thread that reaches the bit first owns that pixel. The score can differ by solve, so scheduling can alter the selected raw value.

Across separate raster sections, `assemble_greyscale` currently selects the lowest section ordinal. The shipped pipeline therefore has two mechanisms:

- Within one section: first thread to claim the pixel.
- Across sections: lowest section ordinal containing the pixel.

Changing native thread count, section boundaries, scheduling, or hot-loop speed can change a winner. Map worker concurrency alone does not currently redefine the cross-section winner because the assembler uses ordinals, but it may do so if assembly moves to a faster first-arrival design.

### Chosen policy: any valid contender may win

Byte-identical rerenders are not a product requirement at collided pixels when nondeterministic ownership provides a speed advantage. For a pixel with one or more valid projected roots, the published value may be the complete score value from any one of those contenders. The selected contender may differ between executions.

This does not mean that arbitrary race behavior is acceptable. The invariant is **memory-safe one-winner nondeterminism**:

- A winner must be a value produced by a root that validly projects to that pixel.
- Scalar and three-channel values must be written atomically as complete logical values; torn RGB values are corruption, not acceptable variation.
- Fragment records must remain structurally valid, bounded, and unambiguous.
- A background pixel must not acquire bytes that were never produced by a valid contender.
- Dense raw, histogram, equalization LUT, rendered image, sidecar, and metadata must all describe the same selected realization.
- Once successfully published, an artifact must be treated as immutable. A retry must not leave metadata and image objects from different realizations.

The current atomic bit claim satisfies the within-section one-winner safety property. It should not be replaced with unsynchronized check-then-write merely because exact reproducibility is unnecessary.

### Design consequences

1. **A fingerprint identifies semantics, not image bytes.** The same program and render inputs may produce more than one valid image hash. Output SHA equality cannot be used as the semantic cache contract for collision-bearing renders.
2. **The effect can extend beyond collided pixels.** Assembly builds the histogram from the selected dense raw values, and equalization uses that histogram. Different winners can slightly change the LUT and therefore alter colors at pixels that were not themselves contested.
3. **Caches store a realization.** The first successfully committed immutable artifact is one valid realization of the request. Concurrent attempts must not race to overwrite pieces of the same artifact key set. If retries reuse keys, publication needs an attempt/ownership guard or an atomic final pointer.
4. **Performance settings are allowed to alter the realization.** Thread count, section count, worker scheduling, projection fast paths, and assembly strategy may change pixels while remaining within contract. Such changes are not regressions solely because hashes differ.
5. **Tests need two classes of fixtures.** Collision-free fixtures require exact byte parity across thread and section settings. Deliberate-collision fixtures require every selected value to belong to the pixel's contender set, complete scalar/RGB writes, matching raw/histogram metadata, and no out-of-bounds or duplicate-record corruption. Repeated hashes may differ.
6. **Visual bias still matters.** First-arrival is allowed, but a faster implementation should be inspected for gross scheduling bias, banding, section seams, or systematic preference for one solve range. Nondeterministic does not mean visually unconstrained.
7. **Assembly can become simpler.** If benchmarks justify it, the current lowest-section-ordinal owner buffer and mutex policy may be replaced by a packed first-successful-CAS value. That trades reproducibility for less memory and synchronization while preserving one complete valid winner.

### Performance decision

Keep the existing first-claim behavior unless a faster safe contender-selection mechanism measures better. Also benchmark a deterministic alternative if it is cheap, but determinism is not a prerequisite and should not be purchased with material latency or memory cost.

## F3 - Sectioning Does Not Deliberately Use Raster Concurrency

**Severity:** HIGH
**Evidence:** directly traced and computed
**Locations:** `lambda/logical_sections.py:8-17`, `:121-176`; `lambda/handler_render_plan.py:520-550`; `stepfunctions/render_workflow.asl.json.template`

### Current behavior

`logical_sections_auto` chooses:

```text
ceil(total_source_bytes / memory_budget)
```

It does not consider requested `raster_workers` as a throughput target. `raster_workers=10` only caps concurrent Map invocations.

The planner also defaults `DEFAULT_RASTER_MEMORY_MB` to 4096. The deployed raster Lambda is 10240 MB, and the render-plan Lambda environment does not provide `RASTER_MT_MEMORY_MB`. The plan's reported `section_memory_mb` is therefore not the deployed worker memory.

For degree 35 with coefficient and parameter sources, four native threads, and one pass of `N*N` solves, the current formula gives:

| N | Sections using current 4096 MB model | Sections if 10240 MB were propagated |
|---:|---:|---:|
| 512 | 1 | 1 |
| 1024 | 1 | 1 |
| 2048 | 2 | 1 |
| 4096 | 7 | 3 |
| 8192 | 25 | 10 |

Times/passes multiply solve count and therefore section count.

The conservative 4096 MB assumption can accidentally produce more parallelism, but that is not a sound throughput policy. More sections also have costs:

- More Lambda cold starts and manifest setup.
- More source-range requests.
- One independent pixel bitset per section, so cross-section duplicate pixels increase.
- More fragment objects, uploads, downloads, and assembler collision work.

### Required fix

Separate two concepts:

1. `min_safe_sections`: memory safety floor based on the actual deployed worker configuration.
2. `target_sections`: throughput choice based on total solves, score-program cost class, requested worker ceiling, minimum useful work per Lambda, and measured fragment amplification.

Then benchmark a matrix on Graviton:

- Sections: `min_safe`, 2, 4, 6, 8, 10 where legal.
- Threads: 1, 2, 4, 6.
- Metrics: cheap, median/sort, pairwise, and mixed-source.
- Outputs: scalar and three-channel.
- Record end-to-end Map wall time, total Lambda GB-seconds, source bytes, fragment bytes, duplicate/collision counts, and final assembly time.

The optimal setting is not necessarily the fastest single raster Lambda or the largest fan-out.

## F4 - Associated Palette Work Duplicates Retained Step Scores

**Severity:** HIGH
**Evidence:** directly traced
**Locations:** `lambda/roots2pix_mt.c:488-515`; `lambda/handler_raster_mt.py:647-683`; `lambda/handler_finalize_mt.py:708-738`, `:336-417`; `lambda/step_scores_to_palette_raw.c`

### Current behavior

For every solve, `roots2pix_mt` stores output bytes in the dense step-score stream. During pass 0 it also maps the same bytes into an associated-palette sparse fragment:

```text
row = step / N
j   = step % N
col = odd(row) ? N - 1 - j : j
pixel = row * N + col
```

`step_scores_to_palette_raw.c` implements the same serpentine formula against the first `N*N` step-score entries.

Finalize already concatenates all step-score pieces before it calls `_finalize_associated_palette`. Nevertheless, associated-palette finalization downloads a second family of sparse fragments and runs the generic deterministic assembler again.

The duplicated path costs, per raster section:

- One extra local ByteVec and append per pass-0 solve.
- One extra S3 upload.
- One extra S3 object.
- One extra finalizer download.
- Generic sparse record parsing, per-record locking, dense allocation, histogram pass, and raw write.

### Required implementation

Use the already-concatenated local `step_scores.raw` as the associated-palette source:

1. Package `step_scores_to_palette_raw` in the finalize Lambda.
2. Extend it to emit the 256-bin channel-0 histogram and background/nonzero counts in the same pass, avoiding a Python rescan.
3. Feed its dense output into the existing `render_score_raw` and sidecar code.
4. In a transition build, optionally generate both old and new palette raw files and SHA-256 compare them on test fixtures and selected development renders.
5. After parity is proven, remove `associated_palette_fragment_prefix` from `roots2pix_mt`, raster uploads, workflow payloads, and cleanup.

This preserves retained step scores for later ExtractPalette while deleting the redundant sparse representation.

## F5 - Finalization Serializes Independent Work

**Severity:** HIGH
**Evidence:** directly traced
**Locations:** `lambda/handler_finalize_mt.py:684-923`; `lambda/color_recolor_raw.py:590-739`

### Current behavior

The main render does not begin until step scores have been serially concatenated and uploaded. The dense raw object is uploaded only after main rendering. The main image and preview are not uploaded until the optional associated palette has been fully assembled, rendered, and uploaded.

Large `put_object` calls use one request each. Raw can be tens or hundreds of megabytes; images can also be large. These transfers sit directly on the user-visible critical path.

### Safe overlap plan

After main assembly completes, the local raw file is immutable. A bounded executor can overlap:

- Main raw multipart upload with main render.
- Step-score upload with local associated-palette conversion.
- Main image/preview upload with associated-palette render/upload.
- Small JSON/metadata writes after their dependencies are available.

Rules:

- Keep concurrency bounded, initially two or three independent transfers. The same-region S3 path is fast, but excessive multipart concurrency across many simultaneous renders can throttle.
- Use managed multipart upload above a measured threshold; keep simple `put_object` for small objects.
- Join every future and verify all required writes before publishing overlay metadata or `done` status.
- Preserve cleanup and fail-closed behavior. A failed background upload must fail the render, not leave a metadata record pointing at a missing object.
- Measure Lambda wall time and GB-seconds. Parallel work can reduce wall time while increasing CPU/network contention.

The same pattern applies to `color_recolor_raw`, where server-side raw/step-score copies, image encoding, associated-palette rendering, and final image upload are also mostly serial.

## F6 - Artifact Previews Should Be JPEG at Creation

**Severity:** MEDIUM for render latency, HIGH for wall bandwidth
**Evidence:** measured locally and directly traced
**Locations:** `lambda/score_raw_render.c:508-523`; `lambda/handler_finalize_mt.py:680-682`, `:905-913`; `scripts/migrate_preview_jpg.py`; `lambda/handler_storage.py:3172-3193`, `:4065-4091`; `lambda/wall_dz.c:153-181`

### Current behavior

New Color and Palette producers create `preview.png`. A separate migration creates `preview.jpg`, records `preview_jpg_key`, `preview_jpg_width`, and `preview_jpg_height`, and leaves the PNG in place. Mosaic manifests prefer the JPEG metadata path. The default baked wall then creates JPEG DeepZoom tiles.

This is deliberately compatible but inefficient for new artifacts:

```text
RGB -> PNG encode -> S3 PNG -> migration GET/decode
    -> JPEG encode -> S3 JPEG -> wall GET/decode -> JPEG pyramid encode
```

### Correct go-forward contract

For new **Color and Palette artifact previews**:

- Write `preview.jpg` directly at 512px maximum.
- Match the accepted migration quality: quality 92, 4:4:4 chroma, no upscaling.
- Set `ContentType=image/jpeg`, immutable Cache-Control, and honest width/height metadata.
- Set the artifact's canonical `preview_key` to the JPEG.
- Do not also generate PNG for new artifacts; doing both preserves the cost this change is meant to remove.

Compatibility reads should prefer JPEG and fall back to PNG:

- `RENDER_FAMILY_SHAPES["color"].preview_candidates` must include `preview.jpg` before `preview.png`.
- Exact gallery resolution currently requires a candidate from that list before it reads JPEG overlay metadata; it must accept JPEG-only new artifacts.
- Palette producers and metadata must emit the actual JPEG `preview_key`.
- Mosaic dimension reading must be format-aware. The current fallback parses only a PNG IHDR. New JPEGs should carry trusted producer metadata, with a JPEG SOF parser or image probe as repair fallback.
- Existing `preview_jpg_*` fields remain a compatibility bridge for migrated artifacts.

Do not upload JPEG bytes under a `.png` key. Do not change root job-result previews, bilevel previews, or coefficient previews in the same patch. Bilevel and line-art previews benefit from lossless PNG; this finding is for noisy Color/Palette artwork.

### Producer inventory

At minimum, inspect and update:

- `handler_finalize_mt.py`, including associated palettes.
- `color_recolor_raw.py`, including associated palettes.
- `handler_palette_finalize.py`.
- `handler_extract_palette_from_step_scores.py`.
- `handler_repalette.py`.
- Color post-process producers such as resize/autolevels where they create a fresh artifact preview.

The old migration remains useful for legacy repair and verification but should no longer be the normal go-forward producer.

## F7 - Raster Projection Has Untaken Common Fast Paths

**Severity:** MEDIUM candidate
**Evidence:** source audit
**Location:** `lambda/roots2pix_mt.c:518-552`

For every root, the projection loop performs center subtraction, full rotation arithmetic, viewport scaling, two `isfinite` calls, two `floor` calls, bounds checks, an atomic pixel claim, and a dynamic ByteVec append.

### Candidates

1. **No-rotation kernel.** Rotation defaults to zero, but the loop still executes the generic rotation formula. Select a no-rotation loop once outside the hot path.
2. **Bounds-before-cast.** After checking `0 <= pxf < W` and `0 <= pyf < H`, C truncation equals floor. This removes two `floor` calls for in-range coordinates while preserving negative/boundary behavior.
3. **Pre-size output vectors.** Start from a bounded estimate such as `min(local_solves*degree, total_pixels) * record_size`, with a memory cap. Avoid repeated growth from 4096 bytes on dense sections.
4. **Cheaper safe ownership.** F2 does not require a deterministic merge. Benchmark the current atomic first-claim against pixel-range sharding or another memory-safe any-contender-wins design only if collision counters show the atomic is material. Do not replace one atomic with larger fragments or extra passes without measuring the full pipeline.
5. **Skip unused transforms.** `prepare_step` already returns the original AoS roots when no root transforms exist. Preserve this zero-copy path.

Benchmark the projection-only term with precomputed score bytes so Solve feature work does not hide the delta. Test default rotation, nonzero rotation, sparse viewport, dense collisions, degree 3/35/128/256, and one/three output channels.

## F8 - Fragment Assembly Locks Every Record

**Severity:** MEDIUM candidate
**Evidence:** source audit
**Location:** `lambda/assemble_greyscale.c:274-318`, `:341-348`, `:494-610`

Each download worker immediately parses its fragment and takes one of 64 striped pthread mutexes for every record. Network concurrency and merge concurrency are coupled, and dense fragments contend heavily on only 64 locks.

The lock is not removable without replacement. It protects both owner and pixel bytes and currently implements lowest-fragment-ordinal wins. F2 permits changing that winner rule, but it does not permit torn values or unsynchronized writes.

### Better designs to benchmark

- Packed any-winner values: use an occupancy bit plus the complete scalar/RGB value in one atomic CAS. This can remove the ordinal owner buffer and striped mutexes when first successful arrival is accepted.
- Packed owner+value atomics: preserve lowest ordinal with CAS only if measurements show the deterministic policy is effectively free.
- Parallel download followed by ordered merge. This removes locks but can retain too much fragment memory if all downloads complete before merge.
- A bounded producer/consumer pipeline: parallel download into a small queue, then either ordered merge or safe first-arrival CAS according to the measured design.
- Pixel-range sharding: merge records into disjoint output ranges, then concatenate, avoiding shared locks at the cost of another partitioning pass.

Also emit counters for total records, winning records, overwritten records, duplicate ratio, download wall time, and merge wall time. Without collision density, a worker-count benchmark is uninterpretable.

The final dense buffer is written and then scanned again for the histogram. A redesigned merge can compute histogram/background counts during the final owner/value extraction pass.

## F9 - Step-Score Concatenation Is Serial

**Severity:** MEDIUM
**Evidence:** directly traced
**Location:** `lambda/handler_finalize_mt.py:221-236`, `:708-738`

`_concat_step_scores` performs one blocking `get_object` after another and streams each object into the destination. This preserves order but leaves available network concurrency unused. The combined object is then uploaded synchronously before main rendering starts.

Options:

- Parallel GET each known section object into a predetermined local offset with `pwrite`.
- Download to per-section temporary files concurrently, validate each length, then concatenate in ordinal order.
- Add a small native presigned-URL concatenator with bounded parallel downloads and ordered writes.

The output must remain exact section order. Validate each section size from plan/raster metadata instead of accepting only a final aggregate byte count.

This work becomes more valuable if F4 uses local step scores for associated-palette generation.

## F10 - Three-Channel Conversion Is Single-Threaded and Copy-Heavy

**Severity:** MEDIUM at large dimensions
**Evidence:** measured locally and directly traced
**Locations:** `lambda/score_raw_render.c` functions `copy_rgb_raw_to_image`, `convert_hsv_raw_to_image`, and `convert_palette_component_lut_raw_to_image`

The three-channel paths allocate full input/output buffers and run scalar C loops:

- RGB with zero-background replacement loads into a heap buffer, mutates it, then `vips_image_new_from_memory_copy` copies it again.
- HSV allocates input and RGB output, converts every pixel with floating-point sector math, then copies the RGB buffer into libvips-owned memory.
- RGB LUT and HSV LUT similarly allocate coordinate and RGB buffers and copy into libvips.

At 4096x4096 this remained sub-0.2 seconds locally, but work and memory scale with pixel count. A 10000x10000 three-channel image is 300 MB per full buffer; multiple copies materially affect the 1769 MB recolor Lambda.

Candidates:

- Parallelize row ranges with a fixed worker count for HSV/LUT conversion.
- Transfer buffer ownership to a VipsImage with an attached free callback instead of copying into a second full buffer.
- Use `vips_rawload` and vectorized libvips operations for RGB zero-background replacement where exact semantics can be pinned.
- Precompute byte-to-RGB/HSV tables as already done, but keep conversion integer where branch/rounding parity allows.

Gate with decoded RGB byte equality before JPEG encoding, not just visual inspection.

## F11 - Full PNG Output Is an Explicit Expensive Mode

**Severity:** MEDIUM when selected
**Evidence:** measured locally
**Location:** `lambda/score_raw_render.c:371-386`; render-format UI and plan default

The default Color format is JPEG, which is appropriate. If PNG is selected, `vips_pngsave(..., compression=6)` can dominate final encoding on noisy artwork. The local high-entropy 4096x4096 case took `1.58 s` for PNG versus `0.18 s` for JPEG.

Actions:

- Keep JPEG as the default.
- Make the UI describe PNG as slower and larger/variable rather than implying it is a free format switch.
- If PNG speed matters, benchmark compression levels 1, 3, and 6 plus filter settings on real artifacts and publish the size/time tradeoff.
- Do not silently lower compression or change output bytes without an explicit product decision.

Even the measured PNG cost is seconds, not minutes. It can explain a slow final tail but not a very long raster phase.

## F12 - Full Image and Preview Are Two Lazy Sinks

**Severity:** LOW/MEDIUM
**Evidence:** disposable prototype
**Location:** `lambda/score_raw_render.c:477-523`

The scalar graph is lazy:

```text
rawload -> equalization maplut -> palette maplut -> rgb
```

Saving the full image evaluates one sink. Resizing and saving the preview can evaluate upstream work again. A disposable materialization reduced CPU in the local PNG-preview case but did not improve warm wall time.

Do not add unconditional full RGB materialization without production measurement. It adds roughly `3*pix*pix` bytes of resident memory. Better options include:

- Materialize only above a measured repeated-work threshold and when memory permits.
- Generate the preview from the already-written JPEG using shrink-on-load.
- Add a dedicated shared-sink implementation if libvips supports it without full materialization.

Direct JPEG preview from F6 should be implemented first; it is simpler and benefits storage/network regardless of this result.

## F13 - Raw Recolor Is Good, but Its Slow Cases Need Isolation

**Severity:** LOW/MEDIUM
**Evidence:** directly traced
**Location:** `lambda/color_recolor_raw.py`, `lambda/raw_score_render.py:14-53`

The fast repalette architecture is correct: reuse dense raw bytes and the stored histogram, then run only equalization/palette mapping/encoding. It avoids roots and Solve Score entirely.

Remaining slow paths:

- Legacy sidecars without a histogram trigger a pure-Python byte-by-byte scan of the entire raw file.
- Associated palettes download and encode another raw image serially.
- Raw and step-score server-side copies, image upload, preview upload, and metadata writes are serialized.
- The handler does not persist detailed elapsed timings, so a slow legacy histogram scan looks like generic "rendering."

Keep histogram-bearing sidecars mandatory for new artifacts. For legacy first-use repair, use a native histogram helper or extend `score_raw_render` to report the histogram rather than scanning hundreds of megabytes in Python. Apply F5's bounded overlap after adding timings.

## F14 - No Durable Color-Pipeline Performance Harness

**Severity:** MEDIUM process risk
**Evidence:** benchmark inventory

Existing tests are strong on payload contracts and small native correctness fixtures, but there is no durable benchmark for:

- Projection-only `roots2pix_mt` cost and scaling.
- Section count versus threads versus Map workers.
- Fragment size amplification and collision rates.
- `assemble_greyscale` download versus merge cost.
- Scalar/RGB/HSV/LUT full encode and preview formats.
- Finalizer S3 overlap and multipart thresholds.
- End-to-end deployed Lambda wall time and GB-seconds.

Create `scripts/bench_color_pipeline.py` or a small family of focused tools. It should produce machine-readable JSON with host/build identity, source hashes, dimensions, channels, metric/program fixture, sections, threads, bytes, timings, collision counts, and output hashes. Hashes prove exact parity only for collision-free fixtures; for collision-bearing fixtures they identify one measured realization.

Performance tests need not run in every predeploy gate. Contract/parity fixtures must be gated; expensive benchmarks should be reproducible and run before/after performance commits on both local arm64 and deployed Graviton.

## F15 - Sequential 600-Second Subprocess Budgets Do Not Fit One Lambda

**Severity:** LOW robustness risk
**Evidence:** directly traced
**Locations:** `lambda/handler_finalize_mt.py:202`; `lambda/raw_score_render.py:114`; deployed timeout configuration

`assemble_greyscale` and `score_raw_render` each have a 600-second subprocess timeout. Associated-palette finalization can invoke another assembly and render. The Lambda timeout is 900 seconds. These independent maxima cannot all fit, and a hard Lambda termination may prevent terminal status and cleanup.

Pass a shared deadline/remaining budget through finalization. Reserve enough time for status publication and cleanup. This does not make successful renders faster, but it makes pathological slowness diagnosable instead of ending as an unexplained timeout.

## Recommended Implementation Order

### Phase 0 - Truthful measurements and an explicit collision contract

1. Add real wall-stage timings and keep worker sums under explicit names.
2. Replace the fake `encode_ms=0` fields.
3. Add collision/fragment amplification counters.
4. Codify F2's memory-safe any-valid-contender winner policy and immutable-publication rule.
5. Add durable projection, assembly, and encoder fixtures.

Do not tune threads or section count before this phase is green.

### Phase 1 - Delete proven duplicate work

1. Derive generated associated palettes from local concatenated step scores.
2. Validate old/new palette raw SHA-256 parity.
3. Remove associated-palette sparse fragment emission and S3 traffic.
4. Generate Color/Palette `preview.jpg` directly and dual-read old PNG artifacts.

These changes simplify the system rather than merely moving work.

### Phase 2 - Shorten the finalizer critical path

1. Parallelize ordered step-score downloads.
2. Overlap main raw upload with render.
3. Overlap main artifact uploads with associated-palette work.
4. Benchmark managed multipart thresholds.
5. Join and verify all outputs before metadata publication.

### Phase 3 - Tune the distributed shape

1. Propagate the real raster memory contract to the planner.
2. Keep a memory safety floor separate from a throughput target.
3. Benchmark section/thread/worker matrices on representative metric families.
4. Optimize for end-to-end latency and GB-seconds, not one native counter.

### Phase 4 - Native hot loops

1. No-rotation and bounds-before-cast projection kernels.
2. Replace the current atomic claim only with a faster memory-safe winner mechanism.
3. Replace per-record assembler mutexes with a measured packed-CAS, ordered-merge, or sharded design; exact winner ordering is optional.
4. Parallel/zero-copy three-channel conversion.
5. Revisit scalar graph materialization only if production timings justify it.

## Verification Gates

### Raster correctness

- Collision-free fixtures at 1/2/4/6 threads and section counts 1/2/4/8 produce byte-identical fragments and dense raw.
- A deliberate-collision oracle enumerates the valid scalar or complete RGB contenders for each pixel; every observed winner belongs to that set.
- Repeated deliberate-collision runs may have different hashes, but every run has valid record framing, bounds, occupancy, and complete channel values.
- Recomputed histogram/background counts from each realized dense raw exactly match its sidecar and metadata.
- Thread and section tuning produces no section seams, torn RGB values, invalid winners, or systematic visual bias outside an agreed tolerance.
- Scalar and three-channel fragments preserve exact record format.
- Lagged Solve Score programs preserve source prelude behavior at section boundaries.
- Root-transform and no-transform outputs remain equivalent to baseline where semantics are unchanged.

### Associated palette

- Existing sparse-fragment assembly and step-score conversion produce byte-identical palette raw for scalar and three-channel fixtures.
- Serpentine odd/even rows are explicitly pinned.
- `times > 1` uses only pass 0 for the palette while preserving the full retained step-score object.
- Histogram, background count, sidecar, image metadata, and click-through identity remain unchanged.

### Preview JPEG

- New Color and Palette artifacts appear in Render, Results detail, AllCol/AllPal, Gallery, Book, and shared viewers with no PNG sibling.
- Existing PNG-only and migrated PNG+JPEG artifacts continue to resolve.
- Dimensions are correct without a PNG IHDR read.
- Quality 92, 4:4:4 JPEG is visually inspected on hard palette boundaries, smooth gradients, and high-frequency fractal detail.
- Bilevel and coefficient preview contracts stay PNG.

### Finalizer concurrency

- Any failed future prevents `done` and leaves no metadata pointing at a missing required object.
- Retries remain idempotent.
- Concurrent attempts cannot publish image, raw, histogram, or metadata objects from different collision realizations under one completed artifact identity.
- Once an artifact is complete, retries do not mutate its published realization unless the product explicitly requests a new render artifact.
- Raw, step-score, image, preview, overlay, and associated-palette keys are all present before terminal success.
- Bounded concurrency is tested under simultaneous renders for S3 throttling.

### Existing suites and packaging

At minimum, run the affected gated suites plus native Docker regression:

```bash
uv run python -m pytest \
  tests/test_render_plan.py \
  tests/test_render_workflow_definition.py \
  tests/test_finalize_mt_handler.py \
  tests/test_recolor_from_raw.py \
  tests/test_storage_handler.py \
  tests/test_migrate_preview_jpg.py \
  tests/test_assemble_greyscale.py \
  tests/test_extract_palette_from_step_scores.py \
  tests/test_deploy_packaging.py -q

bash tests/test_frontend_js.sh
bash scripts/test-docker-runtime.sh
bash scripts/predeploy_check.sh
```

Any C edit requires rebuilding the ARM64 binary, updating `lambda/binary_manifest.json`, and proving source-hash freshness before deployment.

## What Not To Do

- Do not add more Map workers without increasing section items and measuring downstream fragment amplification.
- Do not reject a valid speedup solely because deliberate-collision image hashes change; exact parity is required only where the contender set is singular.
- Do not call torn writes, invalid contenders, mixed-attempt artifacts, section seams, or severe scheduling bias "acceptable nondeterminism."
- Do not optimize the 256-entry palette LUT before measuring it separately; current evidence says it is small.
- Do not generate both PNG and JPEG previews for new artifacts if render speed/storage reduction is the goal.
- Do not upload JPEG bytes under `preview.png`.
- Do not remove assembler synchronization without a replacement memory-safe one-winner mechanism.
- Do not call aggregate worker CPU time "wall time."
- Do not use `-ffast-math` in projection, HSV, or Solve Score code without a full non-finite and boundary policy review.
- Do not move all finalization into one larger monolithic in-memory operation merely to avoid `/tmp`; local raw persistence is useful for upload, retry diagnosis, and bounded memory.
- Do not assume a JIT helps this stage. Solve feature algorithms, root projection, random pixel ownership, S3 traffic, and JPEG/PNG sinks dominate more than simple bytecode dispatch.

## Expected Outcome

The safe near-term result is not a claimed universal percentage. It is a pipeline where the expensive term is observable, thread-race variation is an explicit and bounded throughput policy, associated palettes do not duplicate an already-retained data stream, new wall previews do not require PNG-to-JPEG repair, and independent S3 work no longer blocks serially.

After those changes, Graviton measurements can answer the two questions that matter:

1. For each metric/program family, how many sections and threads minimize end-to-end latency without excessive fragment amplification?
2. For each image size/channel/format, is the remaining tail CPU encoding, S3 persistence, or final fragment merge?

Until F1 is fixed and F2's safety/publication contract is gated, added parallelism can make timings look better while hiding corruption, cross-attempt mixtures, visual bias, or downstream assembly cost behind the word "nondeterministic." The highest-confidence immediate implementation work is F4, F5, and F6 after the Phase 0 gates.
