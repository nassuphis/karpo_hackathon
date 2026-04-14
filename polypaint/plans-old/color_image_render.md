# Color Image Render Notes

Status: legacy archive. Historical optimization note preserved for reference; it uses older stripe-era naming, so read `stripe` as `chunk` when comparing to the current implementation.

This document summarizes practical suggestions for improving the **color** rendering path, based on the current pipeline in:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
- [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)
- [lambda/handler_encode.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_encode.py)
- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
- [lambda/pixassemble.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/pixassemble.c)

This is not a rewrite plan for bilevel. It is a set of suggestions for improving the current **color** render pipeline while preserving:

- RGB output
- rainbow / proximity / constant modes
- root matching behavior
- tile-based parallelism

## Current Color Path

The current color render path is:

1. `viewport`
2. `raster` per stripe
3. `finalize` per tile
4. `encode` once for the full image

The main shapes are:

- compute output: `stripe_XXXX.bin`
- raster output: sparse `pix_{stripe}_t{tile}.pix`
- finalize output: dense RGB `tile_XXXX.raw`
- encode input: one stitched full-image RGB raw file

This pipeline is logically sound. The biggest problems are implementation-level:

- whole-stripe reads
- whole-stripe in-memory rasterization
- full-image RGB stitching in encode
- lack of specialization by color mode

## Recommendations

### 1. Keep stripe-first for color

For color rendering, stripe-first is still the right base architecture.

Reason:

- each root is projected once
- matching logic is naturally step-local within a stripe
- proximity coloring is already implemented stripe-by-stripe
- total CPU and S3 read stay far lower than a tile-first “every tile scans all stripes” design

So the recommendation is:

- do not move color rendering to tile-first
- improve the stripe-first implementation instead

### 2. Remove whole-stripe buffering in `handler_raster.py`

Current issue:

- [handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py) does `Body.read()` and writes the whole stripe to `/tmp`
- then [roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c) reads the whole file and mallocs it again

This is wasteful and couples raster Lambda memory to solver stripe size.

Suggested improvement:

- stream stripe data from S3 to local storage without fully buffering it in Python
- then refactor `roots2pix` so it only loads what it really needs

This alone will make the color path more robust without changing the high-level architecture.

### 3. Split raster implementation by color mode

The current `roots2pix` binary supports three color modes:

- `constant`
- `rainbow`
- `proximity`

These modes do not have the same memory requirements.

#### `constant`

- simplest mode
- no matching
- can be streamed

#### `rainbow` + `match=none`

- can also be streamed
- no need to materialize the whole stripe

#### `rainbow` + `match=greedy|hungarian`

- still does not fundamentally require the whole stripe in memory
- it only needs the previous step for matching, not the entire file
- it can be processed step-by-step

#### `proximity`

- currently does a global min/max nearest-neighbor pass and then an emit pass
- this is the least stream-friendly mode
- but it still does not require the whole stripe in RAM if the file can be scanned twice

Recommendation:

- keep one public raster Lambda if you want
- but internally split `roots2pix` into execution paths optimized for these different modes

This is one of the best wins available for the color path.

### 4. Refactor `roots2pix` to be incremental where possible

The best version of the color path is:

- read one step of roots
- optionally match against the previous step
- project roots
- dedup at the tile-local pixel level
- emit sparse tile contributions

For `rainbow` and `constant`, the in-memory shape should be:

- current step
- previous step if matching is enabled
- per-tile output buffers
- per-tile dedup bitsets

not:

- the entire stripe file

For `proximity`, the likely shape is:

- first pass over the stripe file to determine global normalization bounds
- second pass to emit pixels

That is still much better than forcing the whole stripe into memory.

### 5. Keep tile-local dedup exactly as it is

The current dedup logic in [roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c) is good and should stay.

It already provides:

- one unique emitted pixel per tile-local coordinate per stripe
- strong reduction in sparse output size
- simple semantics

For color, the current “first hit wins within a stripe” behavior is a reasonable raster-stage policy.

Recommendation:

- keep the per-tile bitset dedup model
- do not regress this when refactoring streaming behavior

### 6. Keep finalize per tile, but reduce its working set if practical

Current finalize uses:

- `uint32_t[npix]` tile buffer
- then a second `rgb[npix * 3]` dense output buffer

in [pixassemble.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/pixassemble.c).

This is functional, but not minimal.

Possible improvement:

- assemble directly into a 3-byte RGB tile buffer
- avoid the extra `uint32_t[npix]` intermediate if overwrite semantics can be preserved cleanly

This is not as high priority as fixing raster and encode, but it is a real memory win.

### 7. The biggest remaining color bottleneck is encode

The encode Lambda is still the largest scaling issue for color renders.

Current behavior in [handler_encode.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_encode.py):

- download one row of tile `.raw` files into memory
- stitch a full-image RGB `encode_in.raw` on `/tmp`
- call `raw2jpeg`

This means color rendering still depends on:

- row-sized tile memory
- full-image RGB `/tmp`

That is why large renders run into hard scaling limits even after raster and finalize complete successfully.

### 8. Replace full-image RGB stitching with tile-aware encoding

The best long-term improvement for color is to stop building one giant RGB raw file before encode.

Possible directions:

#### Option A: tile-row streaming encoder

- read only one tile row at a time
- feed those rows directly to the encoder
- never materialize the whole image on `/tmp`

#### Option B: libvips tile-aware composition

- use libvips to compose from tile inputs directly
- avoid manually stitching one giant raw file in Python

#### Option C: color-specific encode binary

- replace the current `handler_encode.py` stitcher with a binary that can read tile raws directly and encode final JPEG/PNG incrementally

This is the single most important color-path improvement if very large color images matter.

### 9. Consider separate encode strategies for JPEG and PNG

JPEG and PNG do not have identical scaling behavior or product goals.

Possible split:

- JPEG path optimized for large photographic-style output
- PNG path optimized for lossless or sharper synthetic color output

That may justify separate encode implementations or different memory strategies.

At minimum:

- do not assume one encode strategy is optimal for both formats

### 10. Color should stay separate from bilevel

A major conclusion from the bilevel analysis still applies here:

- color and bilevel should be different backends

The color path needs:

- RGB values
- palette logic
- optional root matching
- proximity normalization

The bilevel path does not.

So the recommendation is:

- `Render` button -> color pipeline
- `BiLevel` button -> bilevel pipeline

Color should not be forced into bilevel-friendly structures, and bilevel should not inherit color’s RGB-heavy intermediates.

## Priority Order

If improving the color path incrementally, the suggested order is:

1. Stream stripe downloads in raster.
2. Refactor `roots2pix` to avoid whole-stripe malloc for `constant` and `rainbow`.
3. Rework `proximity` into a two-pass stream over the stripe file instead of whole-file buffering.
4. Reduce finalize memory if easy.
5. Replace encode’s full-image RGB stitch with a tile-aware encoder/composer.

## Bottom Line

The color pipeline does not need a radical architectural replacement.

The correct direction is:

- keep stripe-first
- keep sparse tile contributions
- keep per-tile finalize
- but make raster incremental and make encode tile-aware

That preserves the parts of the current design that are computationally efficient while removing the biggest memory and `/tmp` bottlenecks.
