**DeepZoom Plan**

This note describes how to add DeepZoom export and viewing for rendered images using OpenSeadragon.

The feature should be treated as a derived post-render artifact, not as part of the core render pipeline.

**Goal**

After a render completes, the user should be able to:

1. click a `DeepZoom` button in the Render results area
2. generate a DeepZoom export from the finished render
3. open that DeepZoom export in an OpenSeadragon viewer
4. revisit previously generated DeepZoom exports from a separate inventory UI

**What This Is**

This is:

- a post-render export
- a persistent derived artifact
- a separate viewing workflow

This is **not**:

- another render mode
- another variant of the core render pipeline
- something that should be embedded into the normal static preview image

**UI Shape**

There should be two UI entry points.

1. Render tab post-render action
- after a render completes, add a `DeepZoom` button in the same result/download area as:
  - `Download TIFF`
  - `Preview-Compatible TIFF`
  - `PNG`
- clicking it launches a DeepZoom export for that finished render

2. Separate `DeepZoom` tab
- lists all existing DeepZoom exports
- allows the user to click one and open it
- supports deletion if desired

This separation is important:

- the Render tab is where exports are created
- the DeepZoom tab is where saved DeepZoom artifacts are browsed and reopened

**Refactoring Render Workflow**

This is now a wider Render-tab change, not just a new DeepZoom export.

The Render tab has accumulated multiple persistent artifact types:

- base render preview
- base bilevel TIFF
- Preview-Compatible TIFF
- PNG export
- DeepZoom export

That means the old mental model is no longer sufficient:

- run a render
- inject a preview
- inject a couple of download buttons for that one fresh result

The current code still mostly follows that older pattern. In particular:

- [render-results-dir](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1161) is the selected results/job input
- [render-preview](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1267) is the current preview/action container
- [render-info](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1268) is summary text
- [render-log](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1269) is the operation log
- [_renderCommonParams()](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1779) gathers base inputs
- [runRasterPipeline()](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1804) runs color render
- [runBilevelPipeline()](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2181) runs bilevel render
- [runCoeffBilevelPipeline()](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2433) runs coeff render
- [runTiffCompat()](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2138) is a post-render export
- [runPngExport()](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2160) is a post-render export

The specific problem is that post-render artifact actions are still being created ad hoc inside the success path of the render functions, especially [runBilevelPipeline()](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L2410). That means the UI knows how to show those actions only for the “just finished render” case.

That is now the wrong abstraction.

The Render tab needs to become an artifact-management surface for the selected job, not only a render launcher.

The Render-tab model should now be:

- selected job id
- discovered render artifacts for that job
- available actions derived from the artifacts that already exist

This requires a new explicit refresh path.

Add a `Refresh` button immediately to the right of the `Results Dir` input in the Render tab.

That button should:

1. read the current job id from [render-results-dir](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1161)
2. inspect the stored artifacts for that job
3. rebuild the Render result area from those artifacts

This is now necessary because a job may already have:

- a bilevel TIFF from an earlier render
- a Preview-Compatible TIFF generated later
- a PNG generated later
- a DeepZoom export generated later

The user should not have to rerender just to discover or use those outputs.

So the Render tab must support both:

- creating artifacts
- rediscovering and managing artifacts that already exist

Recommended Render-tab responsibilities after refactor:

1. Launch render pipelines
- `Render`
- `BiLevel`
- `Coeffs`

2. Refresh artifact state for the selected job
- new `Refresh` button next to `Results Dir`

3. Show available outputs for the selected job
- preview image if present
- canonical bilevel TIFF if present
- Preview-Compatible TIFF if present
- PNG if present
- DeepZoom if present

4. Offer conversions only when their source artifact exists
- `Make Preview-Compatible TIFF` only if base bilevel TIFF exists and compat TIFF does not
- `Download Preview-Compatible TIFF` if compat TIFF already exists
- `PNG` only if base bilevel TIFF exists and PNG does not
- `Download PNG` if PNG already exists
- `DeepZoom` only if base bilevel TIFF exists and DeepZoom does not
- `Open DeepZoom` if DeepZoom already exists

This is the key behavior change:

- actions are driven by artifact existence
- not by whether the user just completed a render in the current browser session

So the Render result panel should no longer be assembled separately inside each pipeline success branch. It should be rendered by one reusable artifact-state renderer.

Recommended implementation shape:

1. Add a small Render-artifact loader
- input: `job_id`
- output: normalized discovered artifact state

2. Add a small Render-artifact UI renderer
- input: normalized artifact state
- output: updates [render-preview](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1267) and [render-info](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1268)

3. After any successful render or export
- call the artifact loader
- rerender the result panel from discovered state

4. The new `Refresh` button
- calls the same artifact loader + renderer without rerunning anything

This gives one consistent code path for:

- immediate post-render update
- revisiting an older job
- post-export state changes
- DeepZoom creation from an existing bilevel TIFF

Artifact discovery should inspect the current render prefix and the DeepZoom prefix.

At minimum the refresh logic should detect:

- `renders/{job_id}/image_bilevel.tif`
- `renders/{job_id}/image_bilevel_preview.png`
- `renders/{job_id}/image_bilevel_compat.tif`
- `renders/{job_id}/image_bilevel.png`
- DeepZoom manifest / `.dzi` under `deepzoom/{job_id}/...`

This refactor is now necessary because the Render tab is no longer managing one output. It is managing a family of persistent derived artifacts, and the UI needs to reflect that explicitly.

**Export Model**

The DeepZoom export should take the finished render artifact as input, preferably:

- the final TIFF

Then generate:

- a `.dzi` descriptor
- a tile pyramid directory

This should be done with libvips `dzsave`.

So the flow is:

1. normal render completes
2. user clicks `DeepZoom`
3. dedicated export Lambda downloads the source TIFF
4. Lambda runs DeepZoom export
5. Lambda uploads the `.dzi` and tile pyramid
6. Lambda writes a small manifest entry

**Backend Shape**

Use a dedicated DeepZoom export Lambda.

Reason:

- separate timeout
- separate `/tmp`
- separate task status
- separate failure mode

Recommended handler:

- `handler_deepzoom_export.py`

Recommended implementation:

- use libvips directly
- likely via CLI or pyvips

There is no reason to overload the bilevel stitch Lambda with this behavior.

This needs to remain a separate Lambda even if the implementation could technically be folded into another image-processing function. The separation is intentional:

- cleaner responsibility boundaries
- separate deployment unit
- separate resource tuning
- simpler failure analysis
- no contamination of the main render or stitch paths

**Storage Layout**

DeepZoom artifacts should live in the existing `polypaint` bucket under a separate prefix, not under `renders/{job_id}/` and not in a separate bucket.

Recommended layout:

- `deepzoom/{job_id}/{export_id}/image.dzi`
- `deepzoom/{job_id}/{export_id}/image_files/...`
- `deepzoom/{job_id}/{export_id}/meta.json`

This keeps the distinction clear:

- bucket: `polypaint`
- `renders/` = canonical render artifacts
- `deepzoom/` = derived viewer artifacts

This needs to be explicit:

- same bucket
- different prefix

Do not implement this as:

- a second S3 bucket
- or DeepZoom files mixed into `renders/{job_id}/`

**Manifest**

Each DeepZoom export should write a small manifest file:

- `deepzoom/{job_id}/{export_id}/meta.json`

Suggested fields:

- `job_id`
- `export_id`
- `created_at`
- `source_key`
- `dzi_key`
- `tile_prefix`
- `width`
- `height`
- `format`
- maybe `kind` such as:
  - `bilevel`
  - `coeff_bilevel`
  - later `color`

This manifest is what the inventory UI should read.

**Inventory**

Yes, keep an inventory of existing DeepZoom exports.

The simplest persistent inventory is:

- list `deepzoom/` prefixes from S3
- read each `meta.json`

This is probably enough for the first version.

Later, if listing becomes slow, a secondary metadata index could be added, but S3 manifests are the simplest place to start.

**Viewer**

Use OpenSeadragon with the `.dzi` descriptor URL.

Recommended UX:

- clicking `Open` in the DeepZoom tab opens a dedicated viewer panel or page
- that viewer initializes OpenSeadragon against the `.dzi` URL

Do not try to stuff this into the existing render preview element.

DeepZoom wants its own space and interaction model:

- pan
- zoom
- navigator
- fullscreen if desired

**Important Access Constraint**

DeepZoom is not one file. It is:

- one descriptor
- many tile files

So you should not try to use a one-off presigned URL only for the `.dzi` and assume the tiles will magically work.

You need a strategy for serving the whole DeepZoom directory.

Options:

1. Public or CloudFront-backed prefix
- simplest viewer behavior
- best fit for OpenSeadragon

2. Signed prefix or proxy route
- more secure
- more plumbing

3. Per-tile presigning
- not recommended
- too much per-request overhead

The practical recommendation is:

- serve DeepZoom exports from a prefix that OpenSeadragon can fetch directly

If the bucket is not public, use a proxy or CDN strategy rather than presigning each tile on the fly.

**Recommended First Scope**

First version should support:

- bilevel render → DeepZoom export
- inventory listing
- viewer open

Do not expand the first implementation to every render type immediately.

Start with the output you already know is useful:

- bilevel TIFF → DeepZoom

**Button Workflow**

The button behavior should mirror the existing post-render export buttons:

- initial state: `DeepZoom`
- while running: `Creating...`
- on success:
  - `Open DeepZoom`
  - or:
  - leave `DeepZoom` and show a separate `Open` link

Best behavior:

- after creation, the button becomes `Open DeepZoom`
- and the export also appears in the separate DeepZoom tab inventory

This should happen through the same artifact-refresh path described above:

- when no DeepZoom export exists for the selected job, show `DeepZoom`
- when one already exists, show `Open DeepZoom`
- after creation completes, refresh artifact state and rerender the panel
- do not patch the result area with another one-off DOM branch

**Suggested API / Lambda Contract**

Input:

- `job_id`
- `source_key`
- optional `export_id`

Output:

- `export_id`
- `dzi_key`
- maybe `viewer_url` if you build one

Task id:

- `deepzoom_export`

**Large-Image Considerations**

DeepZoom is actually a good fit for very large renders.

Reasons:

- it is tile-pyramid based by design
- it is meant for zoomable viewing
- it avoids trying to show one gigantic raster directly in the browser

So unlike PNG export, this is not just a convenience format. It is a proper viewing format for large images.

That makes this feature especially worthwhile.

**Potential Implementation Choices**

For the export Lambda, likely options are:

1. `vips dzsave`
- straightforward
- likely easiest if the libvips CLI in the layer supports the needed operation

2. pyvips
- also fine
- may be easier if CLI packaging is awkward

3. custom binary
- probably unnecessary unless you need very specific behavior

Recommendation:

- use libvips / pyvips, not a custom DeepZoom generator

**Testing**

Add tests at two levels.

1. Export generator tests
- source TIFF → `.dzi` + tile pyramid
- verify descriptor exists
- verify at least some tile files exist
- verify metadata matches source size

2. Inventory / manifest tests
- list multiple exports
- parse `meta.json`
- ensure sort and open behavior works

If possible, also add a manual smoke test:

- create DeepZoom from a known image
- open in OpenSeadragon
- verify tiles load across several zoom levels

**Implementation Order**

1. Refactor the Render tab around artifact discovery and artifact-state rendering
2. Add the `Refresh` button next to `Results Dir`
3. Implement Render artifact loading for existing render/export outputs
4. Implement Render result-panel rerendering from discovered artifact state
5. Add `handler_deepzoom_export.py`
6. Implement libvips-based DeepZoom generation
7. Upload `.dzi`, tiles, and `meta.json` under `deepzoom/`
8. Wire `DeepZoom` / `Open DeepZoom` into the refactored Render result panel
9. Add a `DeepZoom` tab that lists saved exports
10. Add an OpenSeadragon viewer panel/page
11. Add delete support if needed

**Recommendation**

Build this as:

- a post-render `DeepZoom` export button in the Render result area
- a wider Render-tab refactor so existing render artifacts can be rediscovered and managed via `Refresh`
- a dedicated DeepZoom export Lambda
- persistent DeepZoom artifacts in the existing `polypaint` bucket under a separate `deepzoom/` prefix
- a separate `DeepZoom` tab for inventory and reopening exports
- an OpenSeadragon viewer for viewing selected exports

The workflow should be explicit:

- Render tab: trigger renders, trigger exports, refresh artifact state, and operate on the selected job’s existing artifacts
- DeepZoom tab: browse the inventory of previously generated DeepZoom exports and reopen them

That is the right architecture for large-image interactive viewing and for the broader Render-tab artifact model the app now needs.
