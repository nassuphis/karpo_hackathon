# Bilevel PNG Render Review

Status: legacy archive. Historical review note preserved for reference; it analyzes an older stripe-named RGB/bilevel path. Current render storage is chunk-based and the current immutable family-catalog UI is described in [render_refactor.md](/Users/nicknassuphis/karpo_hackathon/polypaint/render_refactor.md) and [docs/s3results.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/s3results.md).

This document describes the exact path from saved root stripes to `image_bilevel.png`, with the memory and `/tmp` costs that matter if you want to render `50,000 x 50,000`.

## Short Answer

`50,000 x 50,000` looks possible in the current design, but only if:

- your saved root stripes are not extremely large,
- you keep the tile size at `4096` or `2048`,
- and you accept that the pipeline is still mostly RGB until the very last encode step.

`100,000 x 100,000` is not possible with the current encode Lambda configuration, because the stitched raw RGB file alone would be about `27.94 GiB`, while the deployed Lambdas only get `10 GiB` of `/tmp` (`deploy.sh:41`).

## Exact Bilevel Path

### 1. Frontend orchestration

The Render tab's bilevel path starts in `runBilevelPipeline()` (`index.html:2086-2347`).

It does four phases:

1. `clean-render`
2. `viewport`
3. `raster`
4. `finalize`
5. `encode` with `bilevel: true`

For bilevel specifically, the frontend forces:

- `format = png` (`index.html:2096-2099`)
- `color = constant`
- `constant_color = ffffff`
- `match = none`

So the image is rendered as white roots on black, then converted to a 1-bit PNG at the end.

### 2. Raster phase: `stripe_XXXX.bin` -> `pix_{stripe}_t{tile}.pix`

Each raster Lambda is `handler_raster.py` (`lambda/handler_raster.py:21-92`).

Stripe and tile are different axes:

```text
Saved root data:
  stripe_0000.bin
  stripe_0001.bin
  stripe_0002.bin
  ...
  stripe_{nStripes-1}.bin

Final image grid:
  tile_0000  tile_0001  tile_0002  ...
  tile_0013  tile_0014  tile_0015  ...
  ...

Raster output is the cross-product:
  pix_{stripe}_t{tile}.pix

Example with 3 stripes and 4 tiles:

  stripe_0000.bin -> pix_0000_t0000.pix  pix_0000_t0001.pix  pix_0000_t0002.pix  pix_0000_t0003.pix
  stripe_0001.bin -> pix_0001_t0000.pix  pix_0001_t0001.pix  pix_0001_t0002.pix  pix_0001_t0003.pix
  stripe_0002.bin -> pix_0002_t0000.pix  pix_0002_t0001.pix  pix_0002_t0002.pix  pix_0002_t0003.pix

Then finalize collapses by tile:

  tile 0000 <- pix_0000_t0000 + pix_0001_t0000 + pix_0002_t0000
  tile 0001 <- pix_0000_t0001 + pix_0001_t0001 + pix_0002_t0001
  tile 0002 <- pix_0000_t0002 + pix_0001_t0002 + pix_0002_t0002
  tile 0003 <- pix_0000_t0003 + pix_0001_t0003 + pix_0002_t0003

Output of finalize:
  tile_0000.raw
  tile_0001.raw
  tile_0002.raw
  tile_0003.raw
```

For each saved root stripe:

- it downloads `renders/{job_id}/stripe_{s}.bin` from S3 (`lambda/handler_raster.py:34-38`)
- it runs `roots2pix` with the viewport, output size, tile grid, rotation, and bilevel color settings (`lambda/handler_raster.py:42-64`)
- it uploads only the non-empty tile buckets as `.pix` files (`lambda/handler_raster.py:70-80`)

`roots2pix` does not build a full canvas. It:

- loads the full stripe's roots into memory (`lambda/roots2pix.c:379-382`)
- projects each root to a pixel
- deduplicates hits with a per-tile bitset (`lambda/roots2pix.c:395-404`, `lambda/roots2pix.c:451-458`, `lambda/roots2pix.c:499-507`, `lambda/roots2pix.c:552-559`)
- writes sparse entries `[pixel_idx:uint32, rgb:uint32]` into per-tile `.pix` files (`lambda/roots2pix.c:243-262`, `lambda/roots2pix.c:351-366`)

Important: even in bilevel mode, the `.pix` entries still carry a full RGB color value (`0xffffff`), not 1-bit data (`lambda/roots2pix.c:480-508`).

### 2a. How `roots2pix` de-duping actually works

The current raster binary already has an important bilevel-friendly idea inside it: a per-tile bitset used for pixel de-duplication.

#### Bitset layout

For each output tile, `roots2pix` allocates:

- one output buffer for sparse `.pix` entries
- one bitset with one bit per pixel in that tile

That happens here:

- tile-local dimensions are computed at `lambda/roots2pix.c:338-349`
- per-tile output buffers are allocated at `lambda/roots2pix.c:351-366`
- per-tile bitsets are allocated at `lambda/roots2pix.c:395-404`

The bitset size for one tile is:

- `ceil(tile_w * tile_h / 8)` bytes

Examples:

- `2048 x 2048` tile -> `524,288` bytes = `512 KiB`
- `4096 x 4096` tile -> `2,097,152` bytes = `2 MiB`
- `8192 x 8192` tile -> `8,388,608` bytes = `8 MiB`

#### Pixel addressing

After a root is projected to global image coordinates `(px, py)`, `roots2pix` computes:

- which tile owns that pixel
- the pixel's local coordinates within that tile
- a tile-local linear pixel index

That logic is at:

- `lambda/roots2pix.c:444-449`
- `lambda/roots2pix.c:493-498`
- `lambda/roots2pix.c:546-551`

The tile-local index is:

- `pix_idx = local_y * tileW[tile_id] + local_x`

So the bitset is strictly tile-local, not image-global.

#### Dedup check

For each projected root, `roots2pix` computes:

- `byte_idx = pix_idx >> 3`
- `bit_mask = 1u << (pix_idx & 7)`

Then it checks whether that bit is already set.

That happens in all three color modes:

- proximity mode: `lambda/roots2pix.c:451-458`
- constant mode: `lambda/roots2pix.c:499-507`
- rainbow mode: `lambda/roots2pix.c:552-559`

If the bit is already set:

- the root is counted as deduped
- no `.pix` entry is emitted

If the bit is not set:

- the bit is flipped on
- one `.pix` entry is emitted for that pixel

#### Dedup semantics

The semantic rule in `roots2pix` is:

- first root to hit a pixel wins
- later roots that land on the same pixel are ignored

That is explicit in the code comments:

- `lambda/roots2pix.c:392-394`

This matters because it means raster output is already lossy with respect to multiplicity:

- the pipeline preserves whether a pixel was hit
- it does not preserve how many roots hit that pixel

For bilevel output, that is usually exactly what you want.

#### Why this is already close to a bilevel representation

In bilevel mode, every surviving emitted pixel has the same color:

- `constant_color = ffffff`

So after de-duplication, the `.pix` payload still stores `rgb = 0xffffff`, but the real information content is only:

- pixel was hit

That means the current bilevel raster path is effectively doing:

1. compute a tile-local occupancy bitset
2. convert set bits back into sparse RGB entries

So for bilevel renders, the current format is wasting space after dedup:

- the bitset already answers the bilevel question
- the `.pix` output expands that back into 8-byte sparse RGB records

#### What the dedup bitset saves today

Without the dedup bitset, every root hit would produce a `.pix` record.

With the bitset:

- each tile pixel is emitted at most once per stripe
- high-density regions collapse to one sparse record per pixel
- duplicate hits are tracked as `rootsDeduped`

The binary logs this here:

- `lambda/roots2pix.c:581-584`

And it reports:

- `roots_plotted`
- `roots_clipped`
- `total_entries`

at the end of rasterization (`lambda/roots2pix.c:613-620`).

#### Limitation of the current dedup scope

The dedup bitset only removes duplicates within one raster Lambda invocation, which means:

- within one stripe
- within one tile

It does not dedup across stripes.

So if the same output pixel is hit in:

- stripe 7 and stripe 8

then both stripes will still emit data for that pixel, and finalize will resolve the collision later.

For bilevel output, this is another reason a dedicated bilevel path can be simpler:

- stripe stage can emit tile-local bitsets
- finalize can just bitwise-OR those bitsets across stripes
- no RGB overwrite semantics are needed

#### Why this matters for your tile-centric idea

Your proposed “one Lambda per tile, stream all pixels, keep only the tile” approach is simple partly because the final in-memory structure is exactly this same kind of tile-local occupancy bitmap.

So the important observation is:

- the current code already proves the bitset model works
- it is already the internal dedup structure used in raster
- the main inefficiency is that bilevel renders convert the bitset back into RGB sparse records instead of keeping it as the primary artifact

### 3. Finalize phase: `.pix` -> `tile_XXXX.raw`

Each finalize Lambda is `handler_finalize.py` (`lambda/handler_finalize.py:26-109`).

For one tile:

- it streams every stripe's `.pix` for that tile from S3 into `pixassemble` via stdin (`lambda/handler_finalize.py:50-71`)
- `pixassemble` allocates a full tile buffer as `uint32_t[npix]` and then a second `unsigned char[npix * 3]` RGB output buffer (`lambda/pixassemble.c:48-57`, `lambda/pixassemble.c:76-91`)
- it writes one raw RGB tile file with a 12-byte header: `W, H, bands=3` (`lambda/pixassemble.c:93-110`)
- the handler uploads that tile as `renders/{job_id}/tile_{tile}.raw` (`lambda/handler_finalize.py:88-95`)

Important: this is still full RGB, not bilevel. The intermediate tile format is explicitly documented as raw RGB (`docs/s3results.md:46-58`).

### 4. Encode phase: `tile_XXXX.raw` -> `image_bilevel.png`

The encode Lambda is `handler_encode.py` (`lambda/handler_encode.py:20-112`).

It does this:

- opens `/tmp/encode_in.raw` for the full final image (`lambda/handler_encode.py:32-47`)
- writes one big raw RGB header: `width, height, 3` (`lambda/handler_encode.py:46-47`)
- walks the tile grid row by row (`lambda/handler_encode.py:48-65`)
- downloads each tile `.raw` into memory for that row (`lambda/handler_encode.py:51-56`)
- strips each tile header and writes the pixel rows into the full stitched raw file (`lambda/handler_encode.py:58-65`)
- runs `raw2jpeg` with `--bilevel` (`lambda/handler_encode.py:70-81`)

`raw2jpeg` then:

- loads the stitched raw RGB image with libvips (`lambda/raw2jpeg.c:83-91`)
- ORs the bands together and thresholds `> 0` if `--bilevel` is set (`lambda/raw2jpeg.c:93-123`)
- saves a 1-bit PNG with `bitdepth=1` (`lambda/raw2jpeg.c:138-146`)

This is the key fact for sizing: the pipeline does not become bilevel until the very last encode binary. Everything before that is sparse RGB or dense RGB.

## Where Memory Goes

### Raster Lambda

The raster Lambda's memory cost is roughly:

- one full stripe `.bin` in Python during S3 download (`lambda/handler_raster.py:34-38`)
- one full stripe `.bin` again inside `roots2pix` as `float *roots` (`lambda/roots2pix.c:379-382`)
- one dedup bitset for the entire output image: about `pixels / 8` bytes total across tiles (`lambda/roots2pix.c:395-404`)
- one `512 KiB` output buffer per tile (`lambda/roots2pix.c:34`, `lambda/roots2pix.c:360-365`)

For `50,000 x 50,000`, the dedup bitset alone is about `298 MiB`.

### Finalize Lambda

`pixassemble` uses about `7 bytes/pixel` of working memory per tile:

- `4 bytes/pixel` for `uint32_t tile[]` (`lambda/pixassemble.c:50-56`)
- `3 bytes/pixel` for the RGB output buffer (`lambda/pixassemble.c:76-91`)

That does not include the `.raw` file written to `/tmp`.

### Encode Lambda

The encode Lambda has two separate costs:

1. Row stitching memory
   - it holds one full row of tile `.raw` payloads in memory (`lambda/handler_encode.py:48-65`)
2. Full-image `/tmp`
   - it writes the entire stitched RGB image to `/tmp/encode_in.raw` before encoding (`lambda/handler_encode.py:32-65`)

The deployed limits are:

- `1769 MB` RAM for encode (`deploy.sh:33`)
- `10 GiB` `/tmp` for binary-processing Lambdas (`deploy.sh:41`)

## 50k Numbers

For `50,000 x 50,000`, the final image has `2.5e9` pixels.

- Final 1-bit bitmap size: about `298 MiB`
- Current stitched RGB raw size: about `6.98 GiB`

So the final PNG is relatively small, but the encode input is not.

### Tile size comparison

Assuming square tiles and the current tile size options:

| Tile size | Tile grid | Finalize jobs | Per-tile raw size | Finalize working set per tile | Encode row memory | Raster tile buffers |
|---|---:|---:|---:|---:|---:|---:|
| `2048` | `25 x 25` | `625` | `12.0 MiB` | `28 MiB` | `300 MiB` | `312.5 MiB` |
| `4096` | `13 x 13` | `169` | `48.0 MiB` | `112 MiB` | `624 MiB` | `84.5 MiB` |
| `8192` | `7 x 7` | `49` | `192.0 MiB` | `448 MiB` | `1344 MiB` | `24.5 MiB` |

Common to all of them at `50k`:

- raster dedup bitset: about `298 MiB`
- stitched encode input: about `6.98 GiB`

## Is 50k Feasible?

### Encode

Yes, `50k` is plausible for encode.

`/tmp/encode_in.raw` is about `6.98 GiB`, which fits under the current `10 GiB` `/tmp` limit. That leaves roughly `3 GiB` for the output PNG and general overhead.

The memory risk is tile-row stitching:

- `8192` tiles are risky because one row is already about `1.31 GiB` before Python overhead.
- `4096` looks reasonable.
- `2048` is safest for encode memory, but increases tile count and finalize fan-out.

### Finalize

Yes, `50k` is plausible for finalize.

With `4096` tiles, each finalize Lambda is only assembling one `4096 x 4096` tile, so the working set is about `112 MiB` plus process overhead and the `.raw` file on `/tmp`.

### Raster

Raster is the least predictable part.

It does not care much about the final PNG format, but it does care about:

- output pixel count, because of the dedup bitset
- tile count, because of per-tile buffers
- stripe `.bin` size, because the roots are loaded in full

For `50k` with `4096` tiles, the raster Lambda already spends about:

- `298 MiB` on the dedup bitset
- `84.5 MiB` on tile output buffers
- plus the stripe data itself

So feasibility depends heavily on the size of each saved `stripe_*.bin`.

If your stripe files are a few hundred MiB each, `50k` still looks realistic.
If your stripe files are pushing toward `1 GiB`, raster becomes risky.

## Practical Verdict

### Today

`50,000 x 50,000` is probably possible in the current pipeline if you use:

- tile size `4096` as the default first try
- tile size `2048` if encode memory becomes tight
- saved root stripes that are not enormous

### Not possible without redesign

The current design cannot scale arbitrarily because encode is still stitching a full RGB image on disk before converting to bilevel.

The hard limit from `/tmp` alone is around:

- `59,825 px` if you spend essentially all `10 GiB` on the raw RGB input
- more realistically `56k-58k px` once you leave headroom for the output PNG and runtime overhead

So `50k` is in range, but `100k` is not.

## If You Want 50k To Be Safe

The biggest improvements would be:

1. Make bilevel truly bilevel earlier.
   - Do not carry RGB through finalize and encode when the output is black/white.
   - Emit 1-bit or at least 1-byte monochrome tile data instead of RGB tile raws.

2. Remove the monolithic `encode_in.raw`.
   - The current encode step is the main `/tmp` limiter.
   - A tiled PNG writer or streaming row encoder would remove the `~7 GiB` raw-file requirement at `50k`.

3. Stream stripe downloads in `handler_raster.py`.
   - The current `obj["Body"].read()` is avoidable and makes large stripes riskier than they need to be.

4. Check real stripe sizes before starting.
   - The best predictor for `50k` success is the actual `calc.json` `stripes[].bin_size` values, not just the requested output pixel size.

## Implementation Plan

This plan is for adding a dedicated bilevel-only render path. It does not change the existing color pipeline; it adds a parallel path optimized for black/white output.

## Design Considerations

This section captures the main architectural conclusions from the design discussion.

### 1. Stripe-to-tile translation is unavoidable

The root solver emits data in stripes because stripes are convenient for compute.
The renderer wants 2D tiles because tiles are convenient for image generation.

So some translation layer from:

- solver partitioning: stripes

to:

- render partitioning: tiles

is always required.

That means the pair:

- `stripe_id`
- `tile_id`

is not itself a design mistake. Once roots are solved in stripes and rendered in tiles, some `stripe x tile` relationship has to exist somewhere.

### 2. The real problem is not `stripe x tile`, it is how it is implemented

The current design becomes awkward because of the specific choices it makes:

- raster reads a whole stripe into memory
- raster outputs RGB-oriented sparse tile fragments even for bilevel
- finalize expands those fragments into dense RGB tiles
- encode stitches dense RGB tiles into one huge RGB raw image before converting to bilevel

So the issue is not that stripe and tile both exist. The issue is that the current path carries the wrong data shape and memory shape through that boundary.

### 3. Tiles are true 2D, non-overlapping image partitions

Render tiles are square-ish 2D rectangles, not full-width strips.

For a given global image pixel `(px, py)`:

- exactly one tile owns that pixel
- no other tile contains it

Ownership is computed by:

- `tile_col = px / tileSize`
- `tile_row = py / tileSize`
- `tile_id = tile_row * nTileCols + tile_col`

Then inside that tile:

- `local_x = px - tile_col * tileSize`
- `local_y = py - tile_row * tileSize`
- `pix_idx = local_y * tile_w + local_x`

So a global pixel is unique to one tile. The non-uniqueness comes only from multiple stripes contributing to the same pixel of that tile.

### 4. Raster tile fragments are sparse; finalized tiles are dense

Current raster output:

- `pix_{stripe}_t{tile}.pix`

is sparse.

Each entry stores:

- `pixel_idx:uint32`
- `rgb:uint32`

So only hit pixels appear.

Current finalized output:

- `tile_{tile}.raw`

is dense.

It stores every pixel in the tile as RGB.

This is why the current bilevel path is wasteful:

- the raster stage already has the information needed for a bitmap
- but the pipeline later expands it into full dense RGB tiles
- and only at the very end converts that back into 1-bit semantics

### 5. `roots2pix` already uses a bilevel-friendly internal model

`roots2pix` already allocates a per-tile bitset and uses it to deduplicate pixel hits.

Semantics:

- first root to hit a tile-local pixel wins
- later hits to the same pixel in the same stripe are skipped

That means the current raster stage already computes a tile-local occupancy bitmap internally.

For bilevel renders, that is very close to the final information content you actually want:

- pixel hit or not hit

The inefficiency is that the current code converts that bitset back into sparse RGB records instead of keeping the bitmap as the primary bilevel artifact.

### 6. Raster should not inherit solver memory shape

This is the deepest architectural flaw in the current implementation.

Today the raster stage assumes that:

- one solver stripe

is also:

- one in-memory raster work unit

That is wrong.

Solver chunking is a compute concern. Raster memory shape should be chosen based on render concerns:

- tile size
- output size
- Lambda memory
- output format

So while the solver may still store roots in stripes, the raster layer should consume them as a stream or as tile-oriented work, not as mandatory full-memory blobs.

### 7. A whole stripe does not need to be read into memory for bilevel raster

For bilevel raster, the stripe can be processed as a stream:

1. read root pairs sequentially
2. project root to `(px, py)`
3. map to `tile_id`
4. compute tile-local `pix_idx`
5. set a bit or emit a tile-local hit

That does not require materializing the whole stripe in memory.

The current whole-stripe reads are an implementation choice, not a fundamental requirement of bilevel rendering.

### 8. S3 range reads help streaming, but not tile selectivity by themselves

S3 can read byte ranges from an object.

That helps if you want to:

- stream large stripe objects incrementally
- avoid whole-object reads

But range reads do not by themselves solve tile selection, because current `stripe_*.bin` files are stored in solver order, not spatial tile order.

So with the current stripe layout:

- range reads are useful for chunked streaming
- range reads do not make “read only tile 42 from stripe 7” possible in any simple way

### 9. There are two clean extremes: stripe-first and tile-first

#### Stripe-first

- one raster worker per stripe
- each root is projected once
- worker emits contributions partitioned by tile

Pros:

- lower total CPU
- lower total S3 read
- closer to current compute output

Cons:

- requires later per-tile merge
- can become messy if stripes leak too far into render architecture

#### Tile-first

- one raster worker per tile
- each tile worker scans all stripes
- worker keeps only hits inside its tile

Pros:

- very clean tile-centric render abstraction
- no explicit finalize phase
- easy per-worker memory model

Cons:

- every stripe is reread for every tile
- every root is reprojected for every tile
- much higher aggregate IO and CPU

### 10. Reality wins: stripe-first is the practical choice

After considering both models, the pragmatic conclusion is:

- keep stripe-first
- stop doing stripe-first in an RGB-heavy, memory-heavy way

That means the best practical direction is:

1. keep one raster pass per stripe
2. project each root once
3. emit tile-native bilevel intermediates
4. merge per tile with bitwise OR
5. encode final bilevel PNG without a giant RGB raw file

This preserves the computational efficiency of stripe-first while fixing the worst current implementation problems.

### 11. The right abstraction for bilevel is OR over tile contributions

For bilevel output, the final tile semantics are simple:

- if any stripe hits pixel `(x, y)` in the tile, that pixel is white
- otherwise it is black

So the correct mental model is:

- final tile bitmap = OR of all stripe contributions for that tile

This is why bilevel should not be modeled as an RGB overwrite problem. It should be modeled as bitmap OR.

### 12. `pixel_idx` is a 32-bit linear tile-local index

The current raster format stores `pixel_idx` as one flat `uint32`, not packed `x/y` coordinates.

The formula is:

- `pix_idx = local_y * tile_w + local_x`

Because it is 32-bit, the main representational limit is:

- `tile_w * tile_h <= 2^32`

For square tiles, that puts the absolute edge around:

- `65536 x 65536`

So the practical tile ceiling is about `64K x 64K`, not because the code packs two 16-bit coordinates, but because it uses one 32-bit linear index.

### 13. Shared mutable state is possible, but only with the right primitive

It is tempting to want one shared bitmap that many Lambdas can update concurrently.

That is viable only if the backend supports atomic bit operations or if ownership is partitioned so that no two Lambdas touch the same byte.

Important nuance:

- losing a race on a shared byte is not “just losing a duplicate”
- two different pixels may live in the same byte
- ordinary read-modify-write can therefore lose a real pixel, not just redundant work

So a shared bitmap backend must provide one of:

- atomic bit updates
- atomic bitmap OR / merge
- disjoint byte-range ownership

This rules out naive shared files or naive object rewrites as a safe general solution.

### 14. ElastiCache is a credible shared-bitmap option; MemoryDB is not

If you want a shared mutable bitmap in AWS, the most credible service is:

- `ElastiCache` with Valkey/Redis bitmaps

Why:

- Redis bitmaps are just strings with bit operations
- ElastiCache supports `SETBIT`, `GETBIT`, `BITOP`, `BITCOUNT`, etc.
- those are the kinds of operations needed for concurrent tile bitmap updates and merges

By contrast:

- `MemoryDB` is not a fit here
- AWS MemoryDB documentation lists bitmap commands like `SETBIT`, `GETBIT`, `BITOP`, `BITCOUNT` as unsupported

So for this use case:

- ElastiCache: viable
- MemoryDB: not viable

### 15. One bitmap per tile is better than one bitmap for the whole image

If using ElastiCache, per-tile bitmaps are the natural design.

Why:

- removes the single-key resolution ceiling as a practical concern
- keeps memory per key small and predictable
- maps directly to the render unit you already care about
- avoids a giant global hot key

Typical tile bitmap sizes:

- `2048 x 2048` -> `512 KiB`
- `4096 x 4096` -> `2 MiB`
- `8192 x 8192` -> `8 MiB`

At `50,000 x 50,000`, one whole-image bitmap is still only about `312.5 MiB`, so it would fit within Redis string limits in principle. But per-tile keys are still the better operational design.

### 16. The right ElastiCache merge shape is bitmap-chunk OR, not per-pixel `SETBIT`

If using Redis-backed tile bitmaps, the practical merge shape is:

1. a Lambda builds a local tile bitmap for one `(stripe, tile)` contribution
2. that contribution is ORed into the shared tile bitmap
3. the final encoder later reads tile bitmaps

What not to do:

- one `SETBIT` per hit pixel

That would create excessive network command volume and turn Redis into a per-pixel RPC service.

What to prefer:

- one local packed tile bitmap per contribution
- one bitmap OR merge per `(stripe, tile)`

In Redis terms, that means the useful operations are:

- store the contribution as a temporary key or buffer
- merge with `BITOP OR`

The important point is that ElastiCache only makes sense if merges happen in chunks, not pixel-by-pixel over the network.

### 17. ElastiCache removes the current RGB scaling wall, but not all limits

Per-tile shared bitmaps in ElastiCache remove several current bottlenecks:

- no dense RGB finalize artifacts are required
- no giant full-image RGB `encode_in.raw` is required
- the system no longer depends on one monolithic full-image intermediate

But there are still real limits:

- total cache memory
- command rate and merge volume
- Lambda-to-cache networking overhead
- final image encoding strategy

So ElastiCache does not make the problem free. It just changes the shape of the bottleneck from:

- giant RGB intermediates on `/tmp`

to:

- distributed bitmap merge and final encode

### 18. A sorted pixel-ID pipeline is also viable for bilevel

There is another clean bilevel-native strategy:

1. project each root to a pixel
2. encode the pixel as an integer ID
3. sort pixel IDs
4. unique adjacent equal IDs
5. materialize the final bitmap or tiles from the deduplicated IDs

This discards all unnecessary structure for bilevel:

- root identity
- color
- matching

and keeps only:

- occupied pixel IDs

### 19. 64-bit pixel IDs are a good fit for that sorted strategy

Using `uint64` pixel IDs is sensible if dedup is done by sorting.

Typical encodings:

- global linear ID: `pixel_id = py * width + px`
- packed coordinates: `(py << 32) | px`

Advantages:

- no dependency on 32-bit tile-local limits
- no need to carry tile-local indexing at this stage
- simpler external merge and dedup story
- easy to repartition into tiles after sort if needed

This is a strong design for:

- external merge sort
- offline batch render pipelines
- GPU or high-memory compute environments

### 20. Sorted pixel IDs are elegant, but less naturally matched to Lambda

The weakness of the sorted-ID approach is not the algorithm. It is the infrastructure fit.

Sorting large volumes of pixel IDs tends to want:

- large memory
- fast local disk
- external sort support
- or GPU / high-throughput batch compute

That is a better fit for:

- batch systems
- external sort pipelines
- dedicated compute nodes

than for:

- ordinary AWS Lambda

So for this repository's current environment:

- sorted pixel IDs are viable in principle
- but they are not the most natural Lambda-native design

### 21. Practical decision: prefer bitmap-family solutions in Lambda

There are two broad bilevel strategy families:

#### Bitmap-family

- set bits in tile-local bitmaps
- dedup happens by bit semantics
- merge is bitmap OR

Best fit for:

- Lambda
- tile-based rendering
- incremental streaming

#### Sort-family

- emit projected pixel IDs
- dedup happens via sort/unique

Best fit for:

- batch systems
- external sort pipelines
- high-memory or GPU workflows

For the current AWS Lambda architecture, the practical recommendation remains:

- prefer bitmap-family solutions

Within that family, the most practical options are:

1. stripe-first bilevel tile bitsets stored in S3 and OR-merged later
2. stripe-first bilevel tile bitsets merged into ElastiCache tile bitmaps

### 22. Final practical ranking

If optimizing for deployment inside the current Lambda-based system:

1. Stripe-first, tile-local bilevel bitsets, S3 or direct merge
   - best practical fit
2. Stripe-first with ElastiCache per-tile shared bitmaps
   - credible, but adds stateful infrastructure
3. Tile-first, each tile scans all stripes
   - conceptually clean, but duplicates too much work
4. Sorted `uint64` pixel IDs
   - elegant, but better suited to batch/external-sort infrastructure than Lambda

### 23. If you really have billions of pixel IDs, sorting becomes an external-sort problem

If a bilevel path emits a couple of billion `uint64` pixel IDs, the practical way to sort them is not an in-Lambda or in-memory sort. It is an external sort.

Rough scale:

- `1e9` pixel IDs -> about `8 GiB` raw
- `2e9` pixel IDs -> about `16 GiB` raw

And the working set for a normal in-memory sort is usually significantly larger than the raw input.

So the practical approach is:

1. emit binary `uint64` files, not text
2. sort as many IDs as fit comfortably in RAM
3. write sorted runs back to disk
4. do a k-way merge of those sorted runs
5. dedup inline during the merge

That gives:

- exact dedup
- no need to hold the full dataset in memory
- a clean path to either tile materialization or final bitmap generation

### 24. External sort pipeline for `uint64` pixel IDs

The standard pipeline is:

1. **Emit IDs**
   - write packed `uint64` values to binary files
   - one file per worker/stripe/chunk

2. **Chunk sort**
   - read one chunk that fits RAM
   - sort it in memory
   - write one sorted run file

3. **K-way merge**
   - merge all sorted runs in numeric order
   - while merging, keep only unique values

4. **Materialize output**
   - convert deduplicated IDs into:
     - tile-local bitmaps
     - tile files
     - or final image rows

Inline dedup during merge is simple:

- keep `prev_id`
- if `curr_id == prev_id`, skip it
- otherwise emit it and update `prev_id`

### 25. Why sort-family is elegant but not Lambda-native

The sorted-ID approach is algorithmically clean, but the infrastructure fit is different from the bitmap-family designs.

Sorting billions of IDs wants:

- substantial RAM
- fast local disk or NVMe scratch
- or a distributed batch system

That is a better fit for:

- EC2
- AWS Batch
- EMR / Spark
- GPU-capable or storage-heavy machines

than for:

- ordinary AWS Lambda

So the practical conclusion is:

- if the system stays Lambda-centric, bitmap-family approaches are more natural
- if the system moves toward batch or external-sort infrastructure, sorted `uint64` pixel IDs become a very attractive bilevel pipeline

### Goal

Make bilevel renders scale by removing the current RGB intermediates:

- no RGB `.pix` payloads for bilevel,
- no RGB `tile_XXXX.raw` for bilevel,
- no full-image `/tmp/encode_in.raw` for bilevel,
- no final conversion from RGB to 1-bit as the last step.

### Architectural Observation

The current render pipeline is more coupled to the root solver than it should be.

Today the model is:

- compute partitions roots by stripe because solving is stripe-friendly
- raster keeps that same stripe partitioning
- finalize has to undo it by merging across stripes for each tile

That means the renderer is not operating on a pure image-oriented model. It is operating on a compute-oriented storage layout.

Conceptually, a cleaner model is:

1. compute roots in whatever partitioning is best for solving
2. expose a job-level root stream to rendering
3. let rendering partition by tiles because tiling is what image generation actually cares about

In that model:

- the number of stripes is a compute concern
- the number of tiles is a render concern
- raster code should not have to care how the solver partitioned work internally

The current pipeline leaks compute partitioning into rasterization:

- raster output is `pix_{stripe}_t{tile}.pix`
- finalize exists largely because raster is `stripe x tile` instead of just `tile`

This is not necessarily wrong, but it is more baroque than it needs to be.

The architectural direction to prefer for a redesigned bilevel path is:

- hide stripes behind a job-level render abstraction
- treat stripes as a storage/streaming implementation detail
- partition raster work by tiles, not by solver partitions

That does not require physically concatenating all root stripes into one huge file. It only means the render layer should think in terms of:

- “all roots for job X”

rather than:

- “stripe 7 of job X”

This distinction matters because it keeps compute optimizations from dictating render architecture.

### Architectural Risk: Raster Inherits Solver Memory Shape

There is a second, more serious coupling problem beyond just file layout: the raster stage currently inherits the solver's memory shape.

Today the raster path assumes that one solver stripe is a valid in-memory raster work unit.

That happens in two layers:

- `handler_raster.py` downloads the entire stripe object with `Body.read()` before writing it to disk (`lambda/handler_raster.py:34-38`)
- `roots2pix` then opens that stripe file and mallocs the entire file into memory again (`lambda/roots2pix.c:369-382`)

So a raster invocation is effectively defined as:

- “take one whole solver stripe, load it fully, then rasterize it”

This is a bad assumption because it means:

- solver stripe size is chosen for compute reasons
- raster memory sizing is then forced to accept the same chunk size
- render scalability is constrained by solver output chunking

In other words:

- the solver decides how roots are chunked
- the raster Lambda is forced to treat those chunks as in-memory processing units

That is exactly the wrong dependency direction.

The render layer should be free to choose its own memory model based on render concerns:

- tile size
- output pixel count
- desired concurrency
- available memory per render Lambda

It should not be forced to size memory around:

- whatever stripe size happened to be convenient for the root solver

This is why the current design feels wrong in practice:

- compute partitioning leaks into raster orchestration
- and solver chunk size leaks into raster memory requirements

Those are separate problems, and the second one is the more dangerous one for large renders.

The right architectural principle is:

- compute may store roots in stripes
- but raster should consume roots as a stream, not as mandatory full-memory stripe blobs

Minimum improvement:

- stream stripe data from S3 in `handler_raster.py`
- make the raster binary process incrementally rather than loading the full stripe file

Better redesign:

- stop defining raster work in terms of “one solver stripe per raster Lambda”
- define raster work in terms of tile-oriented or row-oriented render units
- treat stripes as a storage detail behind a job-level root stream

Until that changes, raster Lambda sizing will remain partly dictated by solver Lambda output sizing, which is an architectural flaw rather than just an implementation detail.

### Target Design

The bilevel-only pipeline should become:

1. viewport
2. bilevel raster
3. bilevel tile merge
4. bilevel PNG encode

Data shape by phase:

- root stripes: unchanged, still `stripe_XXXX.bin`
- bilevel raster output: tile-local bitsets or compact pixel-index lists
- bilevel merged tile: packed 1-bit tile buffer, not RGB raw
- final encode: PNG written directly from bilevel rows/tiles

### Phase 1: Define The Bilevel Contracts

Add an explicit bilevel contract before changing code.

Decide and document:

- raster output format for bilevel tiles
- merge input/output format
- final encoder input format
- tile ordering and edge-tile sizing rules
- exact pixel semantics: first-hit wins vs last-hit wins

Recommended contract:

- Raster output:
  - one file per stripe per tile
  - payload is packed 1-bit tile data, or a sorted list of `uint32 pixel_idx`
- Merge output:
  - one file per tile
  - payload is packed 1-bit tile bitmap with a small header
- Encode input:
  - list of merged bilevel tile files
  - encoder writes final PNG row-by-row without building a giant RGB image

Why:

- packed 1-bit tile data is the end-state you actually want
- it avoids carrying `rgb:uint32` through the pipeline
- it makes memory sizing deterministic

### Phase 2: Add A Dedicated Bilevel Raster Binary

Do not bend the current RGB `roots2pix` format further. Add a separate bilevel raster path.

Recommended work:

- create a new binary, for example `roots2mask`
- keep the same projection math as `roots2pix`
- keep the same tile grid logic
- output bilevel tile data instead of `[pixel_idx, rgb]`

Recommended output format:

- header:
  - `tile_w`
  - `tile_h`
- payload:
  - packed bitset, `ceil(tile_w * tile_h / 8)` bytes

Alternative:

- sparse sorted `pixel_idx` lists if the root density is low enough that sparse wins materially over packed 1-bit tiles

Recommendation:

- prefer packed bitsets first
- they simplify merge and encode dramatically
- at `4096 x 4096`, one full tile bitset is only `2 MiB`

### Phase 3: Add A Dedicated Bilevel Merge Lambda

Do not reuse `pixassemble` for bilevel.

Add:

- `handler_finalize_bilevel.py`
- a small merge binary, for example `maskassemble`

Behavior:

- for one tile, stream all stripe-level bilevel tile payloads
- OR them into a single packed bitset buffer
- write one merged bilevel tile object back to S3

Memory model:

- one packed tile bitmap in memory
- no `uint32[npix]`
- no `rgb[npix * 3]`

Expected per-tile memory at `4096 x 4096`:

- about `2 MiB` for the tile bitmap

That is dramatically smaller than the current `~112 MiB` working set in RGB finalize.

### Phase 4: Add A Direct Bilevel PNG Encoder

This is the key step. The current limit is the monolithic stitched RGB raw file.

Add:

- `handler_encode_bilevel.py`
- either:
  - a new C encoder that writes PNG directly from bilevel rows, or
  - a Python encoder only if it can stream rows and stay memory-safe

Required behavior:

- read merged bilevel tiles from S3 row-by-row or tile-row-by-tile-row
- assemble output scanlines directly into PNG
- never write a full-image RGB raw file to `/tmp`
- ideally never write a full-image uncompressed bilevel file either

Preferred implementation shape:

- C encoder using zlib
- write PNG scanlines incrementally
- for each output row:
  - gather the contributing tile row fragments
  - emit one packed bilevel row
  - feed it into the PNG stream

Why:

- this removes the current `~6.98 GiB` raw-file requirement at `50k`
- it moves the scaling limit from “full RGB image must fit on disk” to “one row or tile-row must fit in memory”

### Phase 5: Frontend And Dispatch Wiring

Once the backend pieces exist, wire the Render tab to use the new path only for bilevel.

Changes to plan for:

- keep `runBilevelPipeline()` as the orchestration entrypoint
- change its dispatched targets from:
  - `raster` -> `finalize` -> `encode`
- to dedicated bilevel targets, for example:
  - `raster_bilevel` -> `finalize_bilevel` -> `encode_bilevel`

Also update:

- `deploy.sh` packaging
- Lambda creation/update
- environment variables for dispatch
- API routes if you expose any new direct endpoints

### Phase 6: Rollout Strategy

Roll this out behind a separate path, not by replacing the RGB path in-place.

Order:

1. Add bilevel raster binary and tests
2. Add bilevel finalize binary/Lambda and tests
3. Add bilevel encode Lambda and tests
4. Wire frontend bilevel button to the new path
5. Keep old RGB bilevel path available briefly for comparison
6. Remove old bilevel-through-RGB path only after validation

### Phase 7: Validation

Before switching over, validate both correctness and scaling.

Correctness checks:

- compare output pixels against current bilevel renders for known jobs
- verify edge tiles and non-square last tiles
- verify rotation behavior matches the old path
- verify square viewport and auto viewport both match

Scale checks:

- `4096`, `8192`, `16384`, `32768`, `50000`
- tile sizes `2048` and `4096`
- collect:
  - Lambda max memory used
  - `/tmp` usage
  - number of S3 objects written
  - total wall time per stage

### Phase 8: Optional Follow-Ups

Once the dedicated bilevel path works, then consider second-order improvements:

1. Stream stripe downloads in raster.
   - `handler_raster.py` still does a full `Body.read()`.

2. Add a preflight capacity estimator.
   - predict whether a requested render will fit memory and `/tmp` before dispatch.

3. Consider tiled final outputs.
   - if you want to go well beyond `50k`, storing a single giant PNG may stop being the best product format.

### Concrete Recommendation

If the goal is specifically “make `50k x 50k` reliable”, the highest-value sequence is:

1. Build a bilevel tile format that is packed 1-bit, not RGB.
2. Replace RGB finalize with bitset OR merge.
3. Replace `encode_in.raw` with a direct bilevel PNG writer.

That is the minimum set of changes that removes the current scaling wall.

## To Implement

The recommended implementation direction is:

- keep the existing color render path for color output
- add a bilevel-specific render path for bilevel output
- make that bilevel path stay bitmap-native from raster through final encode

At the app level, the Render tab should branch immediately based on which button is pressed:

- `Render` button -> color backend pipeline
- `BiLevel` button -> bilevel backend pipeline

So the frontend should not send both buttons into one shared render pipeline with a late mode switch. It should dispatch to two genuinely different render backends.

In concrete terms, the recommended bilevel architecture is:

1. raster each stripe once
2. emit bilevel tile-local bitsets, not RGB sparse records
3. OR those bitsets across stripes per tile
4. have a bilevel-specific encode Lambda read the final tile bitsets and write the PNG directly

So yes: the main recommendation is to have a bilevel-specific Lambda that does not do color work and does not build a giant RGB intermediate. Its job should be:

- take the final per-tile bitsets
- assemble PNG rows or tile rows
- write `image_bilevel.png` directly

Why this is the right recommendation:

- it matches the true semantics of bilevel output
- it removes the current RGB-heavy memory and `/tmp` costs
- it preserves the efficient stripe-first compute model
- it avoids pushing the whole problem into ElastiCache or external sorting unless you later decide that is worth the complexity

What this recommendation does **not** require:

- no color matching logic
- no RGB tile `.raw` files
- no full-image `encode_in.raw`
- no dependence on the current `raw2jpeg --bilevel` conversion path

If this path is implemented cleanly, the color renderer and the bilevel renderer become two separate products:

- color renderer: optimized for RGB output
- bilevel renderer: optimized for occupancy bitmaps and direct PNG generation
