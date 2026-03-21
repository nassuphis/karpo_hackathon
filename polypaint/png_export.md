**PNG Export Plan**

This note describes how to add a bilevel PNG export feature.

The goal is:

- keep the main bilevel render pipeline unchanged
- keep TIFF as the primary large-image artifact
- add PNG as an optional post-render export

This should follow the exact same workflow as the existing `Preview-Compatible TIFF` export.

Concretely, the existing TIFF compatibility feature is already structured the right way:

- UI action in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- dedicated Lambda handler in [handler_tiff_compat.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_tiff_compat.py)
- dedicated conversion binary in [tiff_compat.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/tiff_compat.c)
- dedicated deploy packaging in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)

The PNG export should copy that structure exactly.

**Scope**

The feature is:

- post-render
- opt-in
- attached to an already completed bilevel render result

It is **not**:

- another main render mode
- another top-level render button next to `BiLevel`
- part of the core large-image stitch path

**UI Placement**

This belongs in the Render tab, in the result/download area that appears after a successful bilevel render.

The flow should be:

1. user presses `BiLevel`
2. normal bilevel render completes
3. UI shows:
   - preview PNG
   - `Download TIFF`
   - `Preview-Compatible TIFF`
4. add one more button in that exact same result area:
   - `PNG`

This button should only appear:

- after a successful bilevel render
- for that specific completed render result

So the placement should exactly mirror the existing `Preview-Compatible TIFF` workflow, not the main render controls.

**Backend Shape**

Use a dedicated conversion Lambda.

Reason:

- separate timeout
- separate memory budget
- separate `/tmp` budget
- separate task status
- separate failure mode

The input should be:

- source tiled TIFF key
- destination PNG key

The output should be:

- PNG key
- presigned URL
- file size
- conversion timing

**Implementation Strategy**

The implementation should follow the existing compatibility-export mechanics:

1. download the finished bilevel TIFF
2. open it with `libvips`
3. save it as PNG with:
   - bilevel / 1-bit output if supported by the chosen code path
4. upload the PNG

This should be implemented as a dedicated binary or a small handler subprocess wrapper, the same way the TIFF compatibility export is implemented.

The intended shape is explicitly:

- `runPngExport(...)` in the frontend
- `handler_png_export.py` as the Lambda handler
- `png_export.c` or equivalent conversion binary
- separate packaging and deployment in `deploy.sh`

**Why Post-Render**

The core bilevel render path is optimized for:

- exact-size tiled TIFF
- large-image scale

PNG export is different:

- it is a convenience format
- it is broadly viewable
- but it is not the right primary artifact for very large renders

So it should remain a second-stage conversion, not part of the main bilevel render itself.

**Recommended Artifact Names**

Starting from:

- `image_bilevel.tif`

write:

- `image_bilevel.png`

This keeps the naming simple and predictable.

If needed, the UI can also keep:

- `image_bilevel_compat.tif`

for the Preview-friendly TIFF export.

**Lambda Contract**

Recommended input:

- `job_id`
- `source_key`
- `png_key` optional override

Recommended output:

- `png_key`
- `url`
- `file_size`
- `convert_ms`

Recommended task id:

- `png_export`

**Suggested UI Wiring**

Mirror the current `Preview-Compatible TIFF` button flow:

- add a `runPngExport(jobId, sourceKey, pix)` function in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- disable the button while conversion is running
- call the new API route
- when ready:
  - either replace the button with `Download PNG`
  - or trigger the download immediately and leave a download link behind

Best behavior:

- keep the button mechanics the same as the existing TIFF compatibility button
- use the state progression:
  - `PNG`
  - `Converting...`
  - `Download PNG`

**Technical Choice**

Use `libvips` for this export.

Reason:

- PNG writing is already in its wheelhouse
- the source is already a TIFF
- this is an export/conversion task, not the primary stitch path

So unlike the main TIFF stitcher, there is no reason to avoid `libvips` here.

**Important Caveat**

PNG export should remain optional because for very large images:

- PNG writing can be slow
- the result may still be huge in pixel dimensions even if compressed size is moderate
- browser/viewer behavior may be worse than with TIFF previews

So the policy should be:

- allow PNG export
- but do not make it the default output format for bilevel render

**Large-Image Guidance**

For moderate images like `10k x 10k`, PNG export is very reasonable.

For much larger images, especially near `100k x 100k`, PNG export may still be possible, but it should be treated as a best-effort optional export, not something the main pipeline depends on.

If needed, the UI can warn above a threshold, for example:

- `PNG export may be slow for very large images`

But that warning belongs to the export button, not the main `BiLevel` render path.

**Testing**

Add a dedicated test file, for example:

- `tests/test_png_export.py`

Suggested cases:

1. Convert a small bilevel TIFF to PNG
- verify dimensions
- verify black/white pixel placement

2. Convert an edge-tile remainder image
- confirm output size is exact

3. Missing input file
- verify clean failure

4. If possible, verify the PNG is truly bilevel / 1-bit rather than silently promoted to grayscale

Also add handler-level tests for:

- S3 download
- subprocess invocation
- upload of the PNG
- presigned URL generation

**Implementation Order**

1. Add the new conversion binary or small converter wrapper.
2. Add `handler_png_export.py`.
3. Package and deploy a dedicated Lambda.
4. Add an API route.
5. Add the post-render button in the Render result area.
6. Add tests for the converter and handler.

**Recommendation**

The `Preview-Compatible TIFF` button mechanics are the correct model and should be copied directly.

Add one more post-render button in the Render result/download area:

- `PNG`

This button should:

- appear after a successful bilevel render
- dispatch a dedicated PNG conversion Lambda
- convert the finished bilevel TIFF to PNG
- then become `Download PNG`

This is the workflow to implement.

Do not add PNG as another main render mode.

In other words:

- the TIFF compatibility feature already demonstrates the exact mechanics
- PNG should be implemented as another sibling post-render export feature with its own Lambda
