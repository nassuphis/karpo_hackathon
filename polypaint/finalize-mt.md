# Finalize Data-Movement And Local MT Plan

## Current State

`Render -> Color -> Generate-MT` finalize is parallel across output tiles, but the expensive path is mostly data movement and many-object gather, not native pixel assembly.

- Step Functions runs `ColorFinalizeMap` over tile items with `MaxConcurrency=32`.
- Legacy RGB finalize downloads one `.pix` fragment per raster chunk for one tile.
- Pixel-bin capable renders can already use `pixel_bins_drive_rgb=true`, where raster skips `.pix`, emits sparse `.pbx`, and finalize renders RGB from assembled dense bins.
- That sparse-bin path removes duplicate RGB bytes, but still reads one `.pbx` fragment per compute chunk per tile.
- Python `finalize_workers` only controls ordered S3 prefetch concurrency.
- Native `pixassemble` is single-threaded.
- Sparse pixel-bin assembly via `pixbinassemble` is also single-threaded.
- `FINALIZE_MEMORY` is currently the 1-vCPU tier, but simply raising memory will not make `pixassemble` use more cores.

## Goal

Reduce saved-palette render latency by attacking the biggest measured costs first:

- eliminate duplicated RGB/bin streams
- reduce S3 object count in the finalize gather
- keep final dense pixel-bin tiles as the canonical repalette artifact
- use local Lambda cores only where CPU time is actually material

Keep the external render artifact contract stable where possible:

- same final `tile_XXXX.raw` keys
- same optional dense pixel-bin tile keys
- same Step Functions tile-map shape
- same UI `finalize_workers` meaning for S3 fetch concurrency unless a separate native thread control is added

## Semantics

The current assembler has "last write wins" behavior for repeated writes to the same pixel.

For this project, deterministic tie-breaking is not required. If multiple entries target the same pixel and there is no meaningful winner, any racing winner is acceptable. That allows a much simpler local MT implementation.

Non-goal:

- Do not spend CPU preserving exact old overwrite ordering unless a later visual regression shows it matters.

Still required:

- output must be a valid raw tile
- out-of-range pixel indices must be ignored
- background remains black for `.raw`
- dense pixel bins still use `255` as background

## Phase 0: Logging And Diagnosis

Before changing the native algorithm, finalize logs should expose enough data to identify the bottleneck.

The UI should show:

- tile size
- tile pixel count / raw byte size
- tile fragment count
- S3 prefetch workers
- access mode, e.g. `s3_prefetch_stdin`
- native assembler mode, e.g. `pixassemble_stdin`
- `.pix` files read / total
- `.pix` bytes read
- missing `.pix` files
- `.pbx` files and bytes when pixel bins are enabled
- dense bin group files and bytes when grouped bin mode is enabled
- read time
- native assemble time
- upload time
- pixel-bin assembly time

This tells us whether the slow part is S3 read, native assembly, pixel-bin assembly, or upload.

Interpretation rule:

- `wall` is elapsed phase time.
- `total=read/assemble/upload/pbx` is summed across tile workers, so it can exceed `wall`.
- Do not optimize native assembly unless `assemble_ms` or `pixel_bins_ms` is actually material.

Example from the first improved log:

- finalize wall: `10.8s`
- aggregate `.pix` read: `14.9s`
- aggregate native RGB assemble: `0.7s`
- aggregate raw upload: `24.7s`
- aggregate pixel-bin pass: `18.7s`
- aggregate `.pix` input: about `1.8GB`
- aggregate `.pbx` input: about `1.8GB`

For that run, native `pixassemble` is not the bottleneck. Local MT `pixassemble` would not materially improve the observed finalize wall. The pressure is mostly S3 transfer/object overhead and the second pixel-bin pass.

Example after the bin-first patch:

- raster skipped all `.pix` upload: `Tile files uploaded: 0`, `skipped .pix uploads: 400`
- raster still uploaded `400` sparse pixel-bin files
- sparse `.pbx` bytes uploaded: `1779.9 MB`
- finalize still read `400/450` sparse bin fragments
- finalize wall: `6.2s`
- aggregate pixel-bin pass: `18.5s`
- aggregate RGB-from-bin render: `2.5s`
- encode wall: `11.7s`

The diagnostic grouping estimate for that same run was decisive:

- current sparse: `400` files / `1779.9 MB`
- `group=5`: `80` dense files / `922.5 MB` (`0.52x` current sparse bytes)
- `group=10`: `40` dense files / `461.3 MB` (`0.26x` current sparse bytes)
- `group=25`: `16` dense files / `184.5 MB` (`0.10x` current sparse bytes)

Do not choose the largest byte win first. `group=25` would reduce 50 chunks to only 2 raster group Lambdas, likely underusing raster fanout. The first implementation target is `group=5` for a 50-chunk / 10-concurrency render: it preserves 10-way raster fanout while cutting finalize objects from about `400` to `80` and cutting `.pbx` transfer by about half.

Optimization order should therefore be data-driven:

- If read/upload dominates, tune S3 prefetch workers, reduce fragment count, or reduce bytes emitted.
- If pixel-bin time dominates, reduce sparse `.pbx` object count first with grouped dense bin tiles; only optimize `pixbinassemble` if fallback sparse mode remains important.
- If native RGB assemble dominates, then implement local MT `pixassemble`.

## Phase 1: Grouped Dense Bin Tiles For Saved-Palette-Capable Renders

For solve-score / saved-palette-capable color renders, color is a pure function of a palette-bin index:

- `0..254` are palette bins
- `255` is background / empty
- RGB is derivable from `bin + palette`

The first bin-first patch removes the duplicated `.pix` stream by using sparse `.pbx` fragments as the RGB source. That is useful, but it still preserves the bad gather shape:

- one sparse `.pbx` object per compute chunk per tile
- finalize still reads `chunks * tiles` objects
- chunk count is tuned for compute fanout, not finalize efficiency

The stronger design is to make raster emit grouped dense grayscale bin tiles.

New saved-palette-capable path:

1. Render plan creates raster groups that each cover multiple compute chunks.
2. Each raster group Lambda processes its assigned chunks.
3. The group Lambda accumulates one dense `u8` bin tile buffer per image tile.
4. The group Lambda uploads one dense bin-tile fragment per touched tile, not one sparse fragment per chunk per tile.
5. Finalize merges dense group bin-tile fragments into one final dense bin tile per image tile.
6. Color is applied at the end by `bin -> palette RGB` lookup.
7. Encode stitches raw RGB tiles as today.
8. The final dense bin tile is uploaded for saved-palette / repalette reuse.

This separates compute granularity from finalize gather granularity:

- `n_chunks` remains tuned for coefficient/root compute and raster load balancing.
- `raster_bin_group_count` or `raster_bin_group_size` is tuned for S3 object count and finalize gather cost.
- For example, 50 compute chunks can become 10 bin groups, so finalize reads 10 bin fragments per tile instead of 50.

First implementation target:

- use `raster_bin_group_size=5` for the current 50-chunk, `ColorRasterMap MaxConcurrency=10` shape
- produce 10 raster group Lambdas
- preserve current raster parallelism
- expect about `80` dense `.u8` group fragments for the measured 8 touched tiles
- expect about `922.5 MB` dense group bytes for the measured render, down from `1779.9 MB` sparse `.pbx`
- keep `group=10` and `group=25` as later tuning options only after measuring raster wall time

Expected win from the measured run:

- remove about `1.8GB` of `.pix` raster upload
- remove about `1.8GB` of `.pix` finalize download
- remove hundreds to thousands of `.pix` objects
- reduce sparse pixel-bin object count by grouping chunks before upload
- replace sparse `.pbx` gather with small dense `u8` tile-layer merge
- keep final dense bin tiles, because they are the reusable saved-palette data

This is more valuable than local MT `pixassemble` for the observed workload because native RGB assembly is already sub-second while duplicated data movement and many-object gather are large.

Implementation notes:

- Keep `pixel_bins_drive_rgb=true` as the render-plan signal that color is derived from bins.
- Add a grouped dense-bin mode, e.g. `pixel_bin_fragment_mode=dense_grouped`.
- Add `plan.raster_items` separate from `plan.chunk_items`:
  - normal modes can keep `raster_items == chunk_items`
  - dense grouped bin mode uses one raster item per chunk group
  - each raster item carries the chunk list/range and `group_idx`
- Update `ColorRasterMap` to iterate `$.plan.raster_items` instead of `$.plan.chunk_items`.
- Keep compute, solve-score histogram, and associated-palette chunk maps on `chunk_items`.
- Add group controls to the render plan / UI:
  - `raster_bin_group_size`
  - or `raster_bin_group_count`
  - plus the existing raster worker/thread controls
- Dense group fragment format should be headerless `u8`:
  - exactly `tile_w * tile_h` bytes
  - `255` means empty/background
  - `0..254` means palette bin
  - dimensions come from the render plan item
- Suggested key shape:
  - `renders/{job_id}/pixbin_group_{group_idx:04d}_t{tile_idx:04d}.u8`
  - keep the old `pixbin_chunk_....pbx` path as compatibility fallback
- Raster group Lambda memory cost is modest:
  - one byte per tile pixel
  - for 9 tiles at about 2.8M pixels each, buffers are about 25 MB total
- Raster group Lambda should initially process chunks sequentially inside the group:
  - download/run one chunk
  - consume its sparse local `.pbx`
  - merge into the group dense tile buffers
  - delete local chunk scratch
  - continue to the next chunk
  - upload final dense group tile buffers at the end
- This avoids rewriting `roots2pix_mt` first. Native direct dense-bin output can come later if local sparse-to-dense group merge becomes material.
- Within a group, repeated writes to the same pixel can use current nondeterministic / last-writer semantics.
- Across groups, finalize merges by writing any non-`255` incoming bin over the current value.
- Because deterministic dithering/tie-breaking is not required, do not add expensive ordering machinery.
- Finalize should produce one final dense bin tile and then generate `.raw` by mapping:
  - `0..254` to the active output palette colors
  - `255` to background black
- First implementation can keep using the existing native `pixel_bins_render` lookup path.
- Later optimization can switch colorization/encode to libvips `maplut`:
  - input is the final single-channel `u8` bin image
  - LUT is a 256-entry RGB image
  - entry `255` maps to background
  - repalette becomes a cheap LUT reapply against saved dense bin tiles
- Keep the old `.pix + .pbx` path as a compatibility fallback until the new path is proven.
- This should apply to solve-score color and saved-palette color modes where pixel bins are semantically available.
- It should not apply to arbitrary RGB modes that do not have a bin representation.

Failure handling:

- If `pixel_bins_drive_rgb=true` and any required palette/bin metadata is missing, fail clearly instead of falling back silently.
- If dense grouped bin mode is requested, finalize must require the expected group count or group keys.
- Missing dense group fragments should be reported as `missing_group_bin_tiles`.
- A dense group fragment with byte size not equal to `tile_w * tile_h` must fail with a clear size error.
- The output image and saved dense pixel bins must come from the same dense bin tile, so image and saved palette cannot drift.

Tests:

- render plan sets `pixel_bins_drive_rgb=true` for solve-score / saved-palette-capable MT color renders.
- render plan emits `raster_items` grouped by `raster_bin_group_size=5` without changing `chunk_items`.
- workflow uses `plan.raster_items` for `ColorRasterMap` and still uses `chunk_items` for solve-score / palette chunk stages.
- raster handler passes native `--skip_pix_output=1` and skips `.pix` upload when `pixel_bins_drive_rgb=true`.
- raster grouped dense-bin mode emits `.u8` group fragments with exact `tile_w * tile_h` size.
- raster grouped dense-bin mode processes multiple chunk inputs in one invocation and uploads one dense group fragment per touched tile.
- finalize merges multiple dense group fragments into one final dense bin tile.
- finalize rejects dense group fragments with the wrong byte size.
- finalize can render `.raw` from dense pixel-bin tile plus palette.
- dense pixel-bin output remains available for saved-palette and repalette.
- frontend logs show the path, e.g. `rgb_from_dense_bins group_size=5 groups=10`.
- Docker runtime regression covers dense group bins -> final dense bins -> RGB raw output.

## Phase 2: Libvips Palette LUT Colorization

Once final dense bin tiles are the canonical saved-palette artifact, colorization should become a derived operation.

Initial implementation may keep the current `pixel_bins_render` native helper because it is simple and already tested.

The follow-up target is libvips LUT colorization:

1. Treat the final dense bin tile as a single-band `uchar` image.
2. Build a 256-entry RGB LUT for the selected palette.
3. Use libvips mapping to turn bin indices into RGB.
4. Encode the resulting RGB image using the same encode path as today.

Why this matters:

- libvips is streamed and locally multi-threaded.
- palette application is independent of roots/coefficient data.
- repalette can skip raster and finalize merge entirely when dense bin tiles already exist.
- saved palettes become "bin image + palette LUT" instead of a baked RGB image plus sidecar bins.

Non-goals:

- Do not use libvips for conflict resolution unless the native dense-bin merge proves slow.
- Do not build a complex vips graph for stack merging before measuring; a tiny native `u8` layer combiner is simpler and likely faster.

## Phase 3: Native `pixassemble` Threads

Add `--threads=N` to `pixassemble`.

Recommended implementation:

- Python finalize still downloads fragments.
- Python writes all `.pix` payloads to a local temp input file instead of piping directly to stdin.
- Native `pixassemble` gets `--input=/tmp/tile.pixstream --threads=N`.
- Before native execution, validate that the input file size is a multiple of 8 bytes because each record is `[pixel_idx:uint32, rgb:uint32]`.
- Native workers split the input file into aligned 8-byte entry ranges.
- Each worker scans its section and writes directly into the shared tile buffer.
- Do not use locks or ordering fences for duplicate-pixel writes.
- Naturally aligned 32-bit stores on the target ARM64 Lambda platform are sufficient for this use case; race winner is acceptable.
- Race outcome is acceptable for duplicate pixel hits.

Why use a temp input file:

- threads can seek to fixed offsets
- no giant in-memory concatenation
- avoids forcing a single stdin reader bottleneck
- easier to test section splitting

Keep stdin mode for backward compatibility and small tests.

Temp-file storage requirement:

- The current stdin path uses essentially no disk for the input stream.
- The temp-file path can require gigabytes of `/tmp` for large tiles.
- Lambda defaults to 512 MB ephemeral storage unless configured higher.
- Do not enable temp-file MT finalize without configuring enough ephemeral storage.
- Runtime should check available `/tmp` space before writing the temp input.
- If there is not enough space, either fail with a clear `insufficient /tmp` error or fall back to stdin single-thread mode.

Rough sizing per tile:

- `.pix` input bytes
- plus raw output bytes
- plus headroom for process overhead
- if pixel bins are enabled, `.pbx` can be processed after `.pix` to avoid holding both input temp files at once

## Phase 4: Native `pixbinassemble` Threads

Add the same `--threads=N` / `--input=...` approach to `pixbinassemble`.

This matters because solve-score and saved-palette renders often emit dense pixel-bin artifacts for repalette and palette saving.

Semantics:

- background is `255`
- repeated pixel-bin writes can use nondeterministic race winner
- invalid bin values should be handled the same way as today
- validate that the `.pbx` temp input size is also a multiple of 8 bytes because each record is `[pixel_idx:uint32, bin:uint32]`

This phase is lower priority if grouped dense bin fragments replace sparse `.pbx` for saved-palette-capable renders. It remains useful for fallback compatibility and non-grouped paths.

## Phase 5: UI Controls

Do not overload `finalize_workers`.

Use separate controls:

- `Finalize S3 workers`: existing `finalize_workers`
- `Finalize native threads`: new `finalize_threads`
- `Raster bin group size/count`: new control for saved-palette-capable renders

Reason:

- S3 prefetch concurrency and native CPU threads are different resources
- optimal values may differ
- overloading one number makes logs and tuning misleading
- compute chunk fanout and finalize bin-fragment grouping are also different resources

Defaults:

- S3 workers: keep current default `16`
- native threads: default to raster threads for MT renders, otherwise `1`
- bin group size: default to `ceil(n_chunks / raster_map_max_concurrency)` for bin-driven color renders
- measured first default: `5` for `50` chunks and `ColorRasterMap MaxConcurrency=10`
- expose override later, but do not make users tune this before the automatic default is proven

## Phase 6: Lambda Sizing

Only after native threads exist, raise `FINALIZE_MEMORY`.

Lambda CPU is memory-proportional. The approximate current rule of thumb in this repo is:

- `1769 MB` is about `1 vCPU`
- `4096 MB` is about `2.3 vCPU`
- `10240 MB` is about `6 vCPU`

Do not assume `4096 MB` gives 4 real cores.

Suggested first test:

- `FINALIZE_MEMORY=4096`
- `finalize_threads=2`

Suggested second test if native assembly is really CPU-bound:

- `FINALIZE_MEMORY` around `7168 MB`
- `finalize_threads=4`

Then compare:

- read time
- assemble time
- pixel-bin time
- upload time
- total wall time
- cost

Do not jump to high memory blindly. If read/upload dominates, more CPU will not fix the bottleneck.

## Tests

Required tests:

- unit test that finalize status payload includes access mode, workers, tile size, fragment counts, and byte counts
- frontend test that render logs display finalize workers, access mode, tile size, fragments, bytes, and timings
- native input alignment test rejects `.pix` / `.pbx` files whose byte size is not divisible by 8
- handler test for insufficient `/tmp` space produces a clear error or controlled stdin fallback
- native `pixassemble` test comparing single-thread and multi-thread outputs for non-overlapping entries
- native duplicate-pixel test that validates output is one of the candidate colors, not a fixed deterministic winner
- native `pixbinassemble` equivalent tests
- Docker runtime test for ARM64 deploy binaries after rebuilding `pixassemble` / `pixbinassemble`

Checklist rule:

- if `pixassemble.c` or `pixbinassemble.c` changes, rebuild the deployed ARM64 binaries before running `bash scripts/test-docker-runtime.sh`
