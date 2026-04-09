# Render Architecture

## Observation 1

Assumption:

- if the final image fits in memory, then this direct raster architecture is
  considered feasible from a Lambda memory/time perspective

Assume:

- the final image always fits in memory
- raster input is already deduped
- raster input is stored as one S3 object of fixed-size records:
  - `x: int32`
  - `y: int32`
  - `palette_index: uint8`

Then a single large Lambda can, in principle:

- allocate one shared in-memory bitmap initialized to background `255`
- section-read the S3 object in parallel
- parse fixed-size records
- write `palette_index` into `bitmap[y][x]`

If the input is truly deduped, this is a valid direct raster architecture and
removes the need for later pixel merge/finalize for that stage.

Main caveats:

- section boundaries must align to whole records
- total input size may still be too large for one Lambda timeout even if the
  image fits in memory
- if dedupe is not global, concurrent writes need an explicit winner rule

## Observation 2

Current shipped color render pipeline:

- raster does **not** write final dense tiles directly
- raster writes sparse per-chunk-per-tile hit streams
- finalize later assembles those sparse streams into dense tile buffers

Current sparse raster artifact:

- `renders/<job_id>/pix_chunk_<chunk_idx>_t<tile_idx>.pix`

Format:

- packed 8-byte entries
- `local_pixel_idx : uint32`
- `rgb : uint32`
- no header
- entries concatenated back-to-back

Important:

- `local_pixel_idx` is tile-local, not global `(x, y)`
- `rgb` is packed `0xRRGGBB`

For solve-score / repalette-capable runs there is also:

- `renders/<job_id>/pixbin_chunk_<chunk_idx>_t<tile_idx>.pbx`

Format:

- packed 8-byte entries
- `local_pixel_idx : uint32`
- `bin : uint32`

Note:

- this is convenience-oriented, not compact
- `bin` only needs one byte in principle
- current `.pbx` keeps the same 8-byte two-`uint32` record shape as `.pix`
- that simplifies emission and assembly code, but is wasteful on disk and over
  S3

Tile shape:

- the image is divided into a 2D grid of tiles
- tiles are usually square
- rightmost / bottom edge tiles may be smaller rectangles

Finalize shape:

- Step Functions launches one finalize Lambda per tile
- each finalize Lambda owns one output tile
- each finalize Lambda downloads all chunk files for that tile
- chunk files are prefetched concurrently but replayed in chunk order

Assembly semantics:

- `pixassemble` consumes `.pix`
- `pixbinassemble` consumes `.pbx`
- both use `last wins` overwrite for duplicate `local_pixel_idx`
- so chunk replay order matters in finalize

What finalize produces:

- one dense tile raw:
  - `renders/<job_id>/tile_<tile_idx>.raw`
- optionally one dense tile pixel-bin buffer

Then the later encode step stitches the dense tile raws into the final image.

## Observation 3

Current root / solve storage is chunked.

Main solve objects:

- `renders/<job_id>/chunk_<chunk_idx>.bin`

There is also a separate lores root object used by the solve-score clip pass:

- `calc.json -> lores.bin_key`

Current `.bin` chunk format:

- no header
- raw interleaved float32 root coordinates
- one solve is:
  - `degree` complex roots
  - stored as `re, im, re, im, ...`

So one solve occupies:

- `solve_bytes = degree * 2 * 4`

That means a chunk is simply:

- solve 0 bytes
- solve 1 bytes
- solve 2 bytes
- ...

with no per-solve header and no per-chunk header.

Important consequence:

- section boundaries for native sectioned read must align to whole solves
- not arbitrary bytes

Current render pipeline therefore starts from:

- chunked solve vectors in `.bin`
- not sparse pixel records
- raster computes projection/color from roots and emits sparse `.pix` / `.pbx`
  later

## Observation 4

Conceptually, the rasterizer turns the chunked solve table into a stream of
roots:

- input rows are solves
- each solve contains `degree` roots
- raster loops over solves
- then loops over roots within each solve
- each root is projected to pixel space and clipped

So conceptually it behaves like:

- one root per row after an implicit pivot/flatten

But this is not materialized as a separate table in memory.

Current behavior is:

- read one solve record
- decide any solve-level color/bin state needed for that solve
- iterate its roots directly
- project each root
- clip out-of-bounds roots
- dedupe / claim at the pixel level
- emit sparse `.pix` / `.pbx` hits only for surviving local pixel claims

Mode detail:

- `solve_score` / `saved_palette`
  - solve-aware color/bin state exists
- `constant`
  - color is trivial
- `rainbow`
  - color is root-index-aware, not solve-score-aware

## Observation 5

Proposed bilevel architecture, restated cleanly:

1. By chunk:
   - scan solve rows
   - flatten to roots on the fly
   - project roots to pixel coordinates
   - clip out-of-bounds roots
   - dispatch surviving pixels into one of `N` tile-owned sparse buffers
   - write one sparse file per `(chunk, tile)` that had hits

2. By tile:
   - one Lambda reads all sparse files for that tile
   - dedupes them globally for that tile
   - produces one deduped sparse tile representation

3. Final encode:
   - one encode Lambda reads the deduped tile representations
   - multiple workers place pixels into one shared final image buffer
   - no contention remains because dedupe already happened
   - final image is encoded and uploaded

This is close to the current bilevel pipeline, but not the same.

Current shipped bilevel pipeline:

1. By chunk:
   - scan solve rows
   - flatten to roots on the fly
   - project roots to pixel coordinates
   - clip out-of-bounds roots
   - dispatch by tile
   - but instead of sparse coordinate vectors, set bits in per-tile dense bitsets
   - upload non-empty `bits_chunk_<chunk>_t<tile>.bits`

2. By tile:
   - one Lambda reads all `.bits` files for that tile
   - merges them by bitwise OR
   - outputs one bilevel tile TIFF

3. Stitch:
   - a later stitch step reads tile TIFFs and writes the final bilevel image

So the major difference is representation:

- proposed path:
  - sparse coordinate streams / sparse final tile sets
- current path:
  - dense tile-local bitsets from the first raster stage onward

Why current bilevel is simpler:

- bilevel has only occupancy, not color
- dedupe semantics are just OR / set-membership
- bitset OR is extremely cheap

Why the proposed sparse path may still matter:

- if tiles are very sparse, sparse coordinate files may be much smaller than
  full tile bitsets or TIFF fan-in
- it may enable a simpler final one-Lambda encode path if the full final image
  fits in memory

Why the proposed sparse path may be worse:

- for dense tiles, sparse coordinate streams can be much larger than a bitset
- storing final global `(x, y)` coordinates is usually unnecessary; tile id plus
  tile-local pixel index is enough
- bilevel already has ideal merge semantics for bitsets (`OR`), so a sparse
  dedupe stage replaces a very cheap operation with a potentially more expensive
  one unless sparsity is high enough to justify it

Current stitch note:

- current bilevel stitch is done with `libtiff` / `libvips`
- that means it is not constrained by needing one giant final image buffer in
  Python memory
- it is therefore much less memory-constrained than a naive “allocate the whole
  final image and fill it directly” approach
- but it is not memory-free:
  - tiles still need to be loaded and processed
  - Lambda memory still bounds the process
