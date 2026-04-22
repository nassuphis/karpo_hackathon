**Bilevel Sizing Bug**

This note records the investigation into the bilevel sizing bug where images whose dimensions are not exact multiples of `tileSize` acquire an empty border on the right and bottom.

Symptom:

- request a square region with exact extent
- expected image area is otherwise correct
- but there is an extra empty band on the lower/right side
- the bug disappears when `pix % tileSize == 0`

That strongly points to the stitch step, not raster math.

**Root Cause**

The bilevel pipeline correctly computes smaller edge tiles during merge:

- merge jobs compute per-tile widths and heights in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2270)
- edge tiles can be smaller than `tileSize`

But stitch then joins the tile TIFFs with:

- `vips_arrayjoin(..., "across", nCols, ...)`

in [lambda/bilevel_merge.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_merge.c#L192)

`arrayjoin` is the wrong primitive here. It treats the input as a regular grid of cells sized from the largest images in each row/column. Since the interior tiles are full `tileSize x tileSize`, the smaller rightmost and bottom tiles get padded to full cell size. That inflates the final stitched image to:

- `nCols * tileSize`
- `nRows * tileSize`

instead of the requested:

- `width = pix`
- `height = pix`

This exactly matches the observed symptom:

- correct content area
- extra empty border on the right/bottom

**Why This Is The Stitch Phase**

Raster and merge already know the correct edge sizes:

- raster is given the exact full image `width` and `height`
- merge receives `tile_w` and `tile_h` per tile

So the tiles themselves can be correct while the final assembly is wrong.

The bug appears only when the requested image size is not a multiple of `tileSize`, which is the signature of:

- correct edge-tile generation
- incorrect regular-grid final placement

**Current Gap In Tests**

The existing stitch tests do not exercise variable tile sizes:

- [tests/test_bilevel_stitch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_bilevel_stitch.py)

They test:

- `2x2` equal-size tiles
- `3x2` equal-size tiles

They do **not** test:

- rightmost column narrower than interior tiles
- bottom row shorter than interior tiles
- final output dimensions smaller than `nCols * tileSize` or `nRows * tileSize`

So this bug could pass the current suite.

**Large-Image Correct Fix**

Do not use `arrayjoin` for the final bilevel stitch.

And for the target use case, do not replace it with a giant in-memory canvas either.

The requirement here is not just correctness for odd image sizes. It is a stitch path that will still work for images on the order of:

- `100000 x 100000`

So the correct fix is:

1. pass the exact final image `width` and `height` into the stitch job
2. pass `tile_size` as well
3. write the final image as an exact-size tiled `BigTIFF`
4. place each tile at its exact tile slot in the TIFF
5. never materialize the whole final image as one giant joined canvas

This keeps both correctness and scale.

The exact placement math is still:

- `tile_col = t % n_tile_cols`
- `tile_row = t / n_tile_cols`
- `x = tile_col * tile_size`
- `y = tile_row * tile_size`

But instead of inserting into a huge joined image abstraction, the stitcher should write each tile directly into the final TIFF at that slot.

**Why Not Just Crop After `arrayjoin`**

Cropping the final stitched image down to the requested width/height may appear to work as a quick fix, but it is not the right design.

Reasons:

- it relies on `arrayjoin` padding only on the right and bottom
- it assumes alignment behavior that should not be the contract
- it keeps the wrong assembly primitive in place
- it does nothing to make the stitch path scale for huge outputs
- it hides rather than fixes the placement model

If you need an emergency patch, cropping is acceptable temporarily.

But it should not be the final architecture.

**Frontend / Handler Changes**

The stitch job currently sends:

- `job_id`
- `n_tile_cols`
- `n_tile_rows`
- `out_key`

from [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2314)

It should also send:

- `width`
- `height`
- `tile_size`

Then:

- [lambda/handler_coeff_bilevel_stitch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeff_bilevel_stitch.py)
  should pass those through to `bilevel_merge stitch`

And:

- [lambda/bilevel_merge.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_merge.c)
  should switch from `arrayjoin` to exact insertion.

**Implementation Plan**

1. Extend the stitch job payload with:
   - `width`
   - `height`
   - `tile_size`

2. Update `handler_coeff_bilevel_stitch.py` to pass:
   - `--width=...`
   - `--height=...`
   - `--tile_size=...`

3. Rewrite `do_stitch()` in `bilevel_merge.c`:
   - stop using `vips_arrayjoin`
   - write an exact-size tiled `BigTIFF`
   - place each tile at exact `(x, y)` tile offsets
   - support remainder tiles naturally

4. Generate preview as a separate step from the final TIFF.

5. Add tests for remainder tiles.

**Test To Add**

Add a stitch test with:

- final width `10`
- final height `10`
- `tileSize = 4`
- `nCols = 3`
- `nRows = 3`

This produces edge tiles of sizes:

- widths: `4, 4, 2`
- heights: `4, 4, 2`

Give each tile a distinct marker and assert:

- final image size is exactly `10 x 10`
- no extra border exists
- each marker lands in the correct global coordinates

That test should fail with the current `arrayjoin` implementation and pass with exact insertion.

**Preview Strategy**

The preview PNG should not drive the stitch design.

The right split is:

1. write the final exact-size `image_bilevel.tif`
2. open that TIFF with `libvips`
3. generate a downsampled `image_bilevel_preview.png`

This is a good fit for `libvips` because:

- it is reading, not assembling
- it is downsampling aggressively
- it keeps PNG work small

So:

- TIFF is the real artifact
- PNG is only the preview

**Memory Requirement**

The fix needs to be large-image-safe.

For `100000 x 100000`, a full 1-bit image is already about:

- `1.25 GB` raw

So any approach that conceptually builds the whole final image as one canvas before writing it is the wrong architecture.

The stitch path should therefore be bounded by:

- one tile at a time
- plus writer overhead

not:

- full-image bitmap
- full-image libvips canvas

This is why the recommended solution is a tiled `BigTIFF` writer rather than:

- `arrayjoin`
- `arrayjoin + crop`
- or exact-placement on a giant canvas

**Speed**

For small images, `arrayjoin` may appear simpler or faster.

For very large images, the tiled `BigTIFF` path is the right speed tradeoff because it:

- scales
- avoids catastrophic memory behavior
- avoids building giant intermediate images

The dominant work for huge outputs will be:

1. reading merged tile data
2. writing final TIFF tiles
3. generating the preview PNG

The preview should remain relatively cheap if it is aggressively downsampled.

So the large-image recommendation is:

- stitch to tiled `BigTIFF`
- then use `libvips` to generate the preview from that TIFF

**Implementation Shape**

The preferred large-image pipeline is:

1. stripe-first raster
2. per-tile merge
3. stitch by writing exact-size tiled `BigTIFF`
4. generate preview PNG from the finished TIFF

If possible, a later optimization is:

- keep merged data as packed tile bitsets rather than intermediate tile TIFFs

Then stitch can write the final TIFF directly from those tile bitsets.

**Library / Deployment Note**

The recommendation is **not** to bundle another copy of the TIFF runtime library into the function payload if the current Lambda layer already provides it.

The current deployed image-processing layer already carries the relevant runtime pieces:

- `libvips`
- `libtiff`
- `libwebp`
- `libjbig`

So if the stitcher is rewritten to use direct `libtiff`, the likely missing part is not the runtime `.so`. The likely missing part is build support:

- `tiffio.h` / `tiff.h`
- compile-time link setup

So the practical distinction is:

- runtime support: already present in the layer
- direct `libtiff` development headers: may need to be added to the build environment

That means the likely work item is:

- extend the build environment so `bilevel_merge.c` can compile against direct `libtiff`

not:

- stuff `libtiff.so` into the function zip again

If the current layer ever turns out not to contain the right runtime library version, then the layer should be updated. But the first assumption should be:

- keep shared runtime libraries in the layer
- keep the function payload for code and binaries only

**Penalty Of Using Only `libvips`**

There is no general penalty in `libvips` itself.

The problem is using the wrong `libvips` primitive for the job.

Bad fit:

- `vips_arrayjoin`

Why:

- it models a regular full-cell grid
- it pads smaller edge tiles
- it does not express the exact-size tile-placement contract needed here
- it is the wrong abstraction for guaranteed large-image-safe stitch

Potentially fine uses of `libvips`:

- reading the finished TIFF
- generating the preview PNG
- downsampling the final image

So the recommended split remains:

- use direct tiled `BigTIFF` writing for the final artifact
- use `libvips` afterward to generate the preview from that TIFF

This gives the clearest large-image behavior and keeps `libvips` in the part of the pipeline where it is strongest.

**My Judgment**

This bug is not in root projection and not in tile generation.

It is in the final stitch assembly primitive.

The current code generates variable-size edge tiles correctly, then stitches them with a function that assumes a regular full-cell grid. For the real target of `100k x 100k` renders, the fix should not be a cosmetic crop or a giant in-memory canvas. It should be a true large-image stitcher that writes an exact-size tiled `BigTIFF` and then derives the PNG preview from that finished TIFF.
