**Bilevel Rendering Bug**

This note is a plan for identifying the bilevel rendering corruption seen on `compute_mmz9yean`, where the final image shows repeated squares, copied regions, or tile-like artifacts.

There is one strong code-level suspect already:

- the current render cleanup path does not delete bilevel `.bits` and `.tif` artifacts

That means rerendering the same `job_id` can mix:

- newly generated stripe bitsets
- stale tile TIFFs or stale stripe bitsets from an older bilevel render of the same job

This would produce exactly the kind of “strange squares and copies” that were described.

**Most Likely Cause**

The bilevel pipeline currently writes these S3 artifacts:

- stripe tile bitsets:
  - `renders/{job_id}/bits_s####_t####.bits`
- merged tile TIFFs:
  - `renders/{job_id}/bilevel_t####.tif`
- final image:
  - `renders/{job_id}/image_bilevel.tif`

But the cleanup route in [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py#L264) only deletes:

- `.raw`
- `.jpeg`
- `.jpg`
- `.png`
- `.pix`

It does **not** delete:

- `.bits`
- `.tif`

Relevant code:

- render cleanup extension list in [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py#L271)
- bilevel raster writes `.bits` in [lambda/handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py#L92)
- bilevel merge writes tile TIFFs in [lambda/handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py#L216)
- coeff bilevel stitch reads tile TIFFs in [lambda/handler_coeff_bilevel_stitch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeff_bilevel_stitch.py#L34)

So the first hypothesis is:

- old bilevel `.bits` or `.tif` objects were left behind in S3 for the same job
- a later render reused some of them unintentionally
- merge/stitch produced a mixed-generation image

This is especially plausible when:

- rerendering the same compute job with different viewport, rotation, root transforms, or tile size
- some tiles are regenerated and others are silently reused from a previous render

**Why This Fits The Symptom**

Tile contamination produces artifacts that look like:

- repeated blocks
- copied quadrants
- tiles from an older render pasted into the new image
- patterns that are too sharp and rectangular to be a math bug in the roots

That is a much better fit for “all sorts of strange squares and copies” than a numerical root-solve problem.

**Immediate Checks**

Before building any new test harness, check these in S3 for `compute_mmz9yean`:

1. How many `bits_s####_t####.bits` files exist.
2. How many `bilevel_t####.tif` files exist.
3. Whether their timestamps span multiple renders of the same job.
4. Whether the current tile count matches the count implied by the active render settings.
5. Whether stale `coeff_*` artifacts also exist for the same job.

The key question is:

- are there bilevel artifacts for this job that were not regenerated in the latest render but were still present in S3?

**First Fix To Make**

Update `/clean-render` so it deletes all current render-stage artifacts, including:

- `.bits`
- `.tif`
- `.png`
- `.jpeg`
- `.jpg`
- `.raw`
- `.pix`

and possibly also delete by known prefixes:

- `bits_`
- `coeff_bits_`
- `bilevel_t`
- `coeff_t`
- `image_bilevel`

Using prefix-aware cleanup is safer than relying only on filename extensions.

**Other Plausible Bugs To Check**

If stale artifacts are not the issue, the next places to inspect are:

1. Tile indexing consistency
- raster uses `tile_id = row * n_tile_cols + col`
- merge uses the same numbering
- stitch expects row-major tile order

2. Edge-tile dimensions
- last row / last column tiles are smaller
- if merge or stitch assumes full `tileSize` on an edge tile, copied blocks can appear

3. Tile size / tile count drift across phases
- raster, merge, and stitch must agree on:
  - `tile_size`
  - `n_tile_cols`
  - `n_tile_rows`

4. Reuse of stale local `/tmp` files
- less likely here because local `/tmp` cleanup exists in the bilevel handlers
- still worth confirming for exception paths

5. Prefix collisions between root bilevel and coeff bilevel outputs
- root path uses `bits_*` and `bilevel_t*`
- coeff path uses `coeff_bits_*` and `coeff_t*`
- these look separated correctly, but should still be verified

**Debug Strategy**

The shortest path is:

1. Confirm or eliminate stale-artifact reuse.
2. If still broken, use a synthetic root-pattern render that has an obvious expected tile layout.

Do not start with real polynomial outputs. They are too hard to reason about visually.

**Synthetic Root Pattern Harness**

Yes, a special debug root dataset is the right next step.

Create a `compute_debug/` folder with a tiny synthetic job whose stripe files are written directly, bypassing the solver.

The debug job should use:

- small image size, for example `128 x 128` or `256 x 256`
- small tile size, for example `32`
- degree `1`
- a handful of stripes

That gives a simple grid like:

- `4 x 4` tiles

and makes misplacement immediately visible.

**Best Debug Patterns**

Use more than one.

Pattern 1: one unique marker per tile

- place exactly one white point at a unique position inside each tile
- expected result:
  - every tile contains exactly one marker
  - no marker repeats
  - no tile is blank

If tiles are copied or reordered, this will show it instantly.

Pattern 2: diagonal crossing tile boundaries

- place points along a continuous diagonal through the whole image
- expected result:
  - one smooth diagonal crossing many tiles

If tile origins or edge dimensions are wrong, the diagonal will jump at boundaries.

Pattern 3: stripe overlap OR test

- two stripes both set some of the same pixels in one tile
- expected result:
  - final tile is simple logical OR
  - no duplicates matter

This checks merge semantics independently of tile placement.

Pattern 4: sparse tile occupancy

- only every other tile gets points
- expected result:
  - empty tiles stay empty
  - non-empty tiles do not leak into neighbors

This is useful for catching stale tile TIFF reuse.

**Recommended Test Fixture Layout**

Add a folder like:

- `compute_debug/`

containing:

- one or more fixture manifests
- tiny synthetic stripe `.bin` files
- a note describing the expected rendered image

The simplest root stripe format for this harness is:

- degree `1`
- `float32` complex root pairs exactly matching the current root file format

That keeps the test aligned with the current raster binaries.

**What The Fixture Should Encode**

For each synthetic root step:

- choose an intended global pixel `(px, py)`
- convert that back to complex coordinates using the chosen viewport
- write that complex root into the stripe file

This is better than guessing complex coordinates visually.

That way the expected output is defined in image space first, which is what the renderer actually has to honor.

**Expected Outcome Of The Debug Harness**

With the synthetic pattern, each stage becomes easy to reason about:

- raster should produce only the expected `bits_s####_t####.bits`
- merge should produce one tile TIFF per expected tile
- stitch should place those tiles in row-major order with no duplication

If corruption still appears, the stage where it first appears will be obvious.

**Stage-By-Stage Isolation**

To isolate the bug, compare outputs at three levels:

1. Stripe bitsets
- inspect which tiles exist for each stripe
- confirm their sizes match the tile dimensions

2. Tile TIFFs
- inspect each `bilevel_t####.tif` individually
- see whether corruption is already present before stitch

3. Final image
- if tiles are clean but the final image is wrong, the bug is in stitch
- if tiles are already wrong, the bug is in raster or merge

**Suggested Work Order**

1. Fix cleanup so bilevel `.bits` and `.tif` are deleted.
2. Rerender the broken job with the same settings.
3. If the bug disappears, the issue was stale artifact reuse.
4. If not, create the synthetic `compute_debug` fixture set.
5. Run the synthetic render and inspect:
   - stripe bitsets
   - merged tile TIFFs
   - final stitched TIFF

**My Current Judgment**

The strongest current suspect is not math and not libvips.

It is stale artifact reuse caused by `/clean-render` not deleting the actual bilevel intermediate formats now in use.

That is the first thing I would fix and retest before building deeper instrumentation.
