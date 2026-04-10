Render Associated Palette Plan
==============================

Objective
---------

When a user runs `Render -> Color`, optionally keep the palette used for that image attached to the resulting Color artifact.

User-facing goals:
- add a checkbox in the Render Generate popups
- if checked for a `solve_score` Color render, create a real Palette artifact as part of the render flow
- if checked for a `saved_palette` Color render, do not regenerate anything; just record the dependency on the source Palette artifact
- when the user downloads `image + meta`, also download the associated palette image when one exists
- when the user makes a ColorSpread PDF, include the associated palette as a small square on the text page

This is about artifact association, not just convenience text in metadata.


Current State
-------------

Color render already computes:
- solve-score clip
- solve-score hist
- solve-score merge
- final solve-score bins JSON

That work lives in:
- [handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- [render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)

Palette artifact generation does more than that:
- it reuses the global solve-score bins JSON
- then computes per-chunk all-pass palette bins
- then assembles a real Palette artifact image and metadata

That work lives in:
- [handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)
- [handler_palette_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_finalize.py)
- [palette_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/palette_workflow.asl.json.template)

Important consequence:
- the existing Color render does not currently produce enough data to materialize a reusable Palette artifact for free
- but it already pays the expensive global solve-score clip/hist/merge cost
- so generating the Palette artifact from the same render is still cheaper than running a separate Palette job later


Recommended Behavior
--------------------

UI
--

Add a checkbox to the two Color render launch popups:
- `Render -> Color -> Generate`
- `Render -> Color -> Generate-MT`

Do not put this in the always-visible Render form.

Popup placement:
- in the single-thread `Generate` popup, place it under the existing solve-score histogram input choice area
- in the `Generate-MT` popup, place it with the execution/output controls, near the hist/raster/finalize options

Checkbox details:
- label: `Save Associated Palette`
- only shown when `_renderActiveFamily === 'color'`
- enabled only when the effective `color_mode` is `solve_score` or `saved_palette`
- hidden or disabled with explanatory text for `rainbow`, `constant`, `proximity`, and non-Color families
- default: unchecked

State handling:
- the single-thread popup state should carry `saveAssociatedPalette`
- the MT popup state should carry `saveAssociatedPalette`
- both `runRasterPipeline(...)` and `runRasterPipelineMT(...)` must include it in the render payload
- the plan Lambda should normalize it to a strict boolean
- Populate should not silently force it on; it is a run option, not a persistent artifact property

Related UI gap: `GenerateFromPalette`
------------------------------------

Current behavior:
- `Render -> Color -> GenerateFromPalette` selects a reusable palette artifact and dispatches a saved-palette Color render directly
- it does not expose the MT execution controls used by `Generate-MT`

Recommended change:
- after selecting the source palette artifact, open a second execution popup for `GenerateFromPalette`
- this popup should be the saved-palette analogue of `Generate-MT`

What it should include:
- raster threads
- raster input mode
- raster retries
- finalize workers
- `Save Associated Palette`

What it should not include:
- solve-score hist line
- solve-score hist threads
- solve-score hist input
- solve-score hist retries
- solve-score merge workers

Reason:
- `saved_palette` renders already have their bins from the selected palette artifact
- no solve-score clip/hist/merge work is run for this path
- the execution tuning here is only raster/finalize tuning

This should be explicit in the popup summary text:
- e.g. `saved-palette render • source bins=<palette_id> • hist unused`

Behavior by color mode:
- `solve_score`
  - if unchecked: current behavior
  - if checked: generate a real Palette artifact after the solve-score bins already computed by the render
- `saved_palette`
  - if unchecked: current behavior
  - if checked: do not generate a new palette artifact; just record the source palette dependency in the Color artifact metadata

This keeps semantics simple:
- `solve_score` creates a new associated palette artifact
- `saved_palette` records the already-existing palette artifact

Derived Color artifacts
-----------------------

Derived Color artifacts should preserve associated-palette lineage.

Specifically:
- if a Color artifact has an associated palette
- and the user creates a derived Color artifact from it
  - e.g. `Autolevels`
  - e.g. `Resize`
- then the derived Color artifact should inherit that same associated palette metadata

Rules:
- do not regenerate the palette
- do not clear the association
- copy the associated-palette fields forward to the derived artifact metadata
- preserve whether the association was:
  - `generated`
  - `dependency`
- preserve the palette image key/id/display metadata exactly

Implementation contract:
- `Autolevels`, `Resize`, and any other Color-to-Color derivative must inspect
  the source Color artifact metadata at creation time
- if the source artifact has:
  - `associated_palette_mode = generated`
  - or `associated_palette_mode = dependency`
  then the derived artifact must copy the full associated-palette field set
  unchanged
- if the source artifact has:
  - `associated_palette_mode = none`
  - or no associated-palette fields at all
  then the derived artifact must also have no association of its own
- the derivative operation must not run:
  - `palette_chunk`
  - `palette_finalize`
  - `ExtractPalette`
  - or any image-based palette inference
- this is a metadata inheritance step only

Operational meaning:
- the derived artifact keeps its own:
  - `artifact_id`
  - `image_key`
  - `preview_key`
  - `derived_from_artifact_id`
- but it points at the same associated palette artifact as its source
- the associated palette is therefore inherited by reference, not copied as a
  second palette artifact

Non-goal for v1:
- if a source Color artifact later gains an associated palette through
  `ExtractPalette`, existing already-created derived artifacts are not
  retroactively rewritten automatically
- if the user wants the derived artifact to carry that association too, they
  can run `ExtractPalette` on the derived artifact, and that operation should
  resolve the palette through lineage

Reason:
- these postprocess operations change presentation of the same rendered image
- they do not change the palette lineage of the image
- the user should still be able to:
  - download `image + meta + palette`
  - make a ColorSpread PDF with the palette square
  - understand which palette belongs to the derived image


Artifact Contract
-----------------

Add these metadata fields to Color artifacts:
- `associated_palette_enabled`: `true|false`
- `associated_palette_mode`: `generated|dependency|none`
- `associated_palette_id`
- `associated_palette_display_name`
- `associated_palette_image_key`
- `associated_palette_preview_key`
- `associated_palette_palette`
- `associated_palette_metric`
- `associated_palette_quantile`
- `associated_palette_omega`
- `associated_palette_omega_enabled`

Rules:
- for `solve_score` + checked
  - `associated_palette_mode = generated`
  - `associated_palette_id` points to the palette artifact created by the render flow
- for `saved_palette` + checked
  - `associated_palette_mode = dependency`
  - `associated_palette_id` points to the input source palette artifact
- otherwise
  - `associated_palette_mode = none`

Inheritance rule for derived Color artifacts:
- `autolevels`, `resize`, and similar Color-to-Color derived artifacts copy the full associated-palette field set from their source Color artifact
- they do not create a new associated palette of their own
- their metadata should continue to expose the inherited association through the same fields

Required downstream behavior for inherited associations:
- `image + meta` on the derived artifact downloads:
  - the derived image
  - the derived metadata JSON
  - the inherited associated palette image
- ColorSpread PDF on the derived artifact shows the inherited associated
  palette square
- render-summary / artifact inventory must surface inherited associations the
  same way as directly-generated associations
- UI must not distinguish between:
  - directly-associated palette
  - inherited-by-derivation palette
  except where provenance text explicitly mentions the source

These fields should be exposed in:
- Color artifact metadata on S3
- [handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py) render-summary entries
- the meta JSON created by the existing `image + meta` download path


Workflow Design
---------------

Case 1: `solve_score` Color render with checkbox checked
--------------------------------------------------------

Recommended shape:
1. Run the existing Color render flow unchanged through solve-score merge.
2. Reuse the render’s already-created:
   - `solve_score.clip_key`
   - `solve_score.bins_key`
3. Add an optional palette-association branch after those solve-score artifacts exist.
4. That branch runs only the palette-specific extra work:
   - `palette_chunk` map over the existing render chunk list
   - `palette_finalize`
5. After palette finalize succeeds:
   - update the Color artifact metadata to point at the generated palette artifact
   - refresh render status with the associated palette info

Important:
- do not rerun clip/hist/merge
- the palette branch should consume the same render plan values:
  - `metric`
  - `palette`
  - `solve_score_quantile`
  - `solve_score_omega`
  - `solve_score_omega_enabled`
  - `root_transforms`
  - `degree`
  - `N`
  - `times`

Case 2: `saved_palette` Color render with checkbox checked
----------------------------------------------------------

No extra workflow branch.

Just write association metadata onto the resulting Color artifact:
- `associated_palette_mode = dependency`
- `associated_palette_id = saved_palette_id`
- plus display/image fields copied from the source palette metadata

This is cheap and exact.


Where To Put The Extra Work
---------------------------

Recommended v1:
- keep this in the Render state machine
- do not spawn a completely separate palette orchestrator from the browser

Reason:
- the user asked for one render operation with an optional associated palette
- browser fanout is the wrong place for this
- the render run should own the lineage and status

Suggested render workflow addition:
- after Color finalize/encode completes, branch:
  - if `save_associated_palette` is false: finish
  - if `color_mode == saved_palette`: write dependency metadata and finish
  - if `color_mode == solve_score`: run `AssociatedPaletteChunkMap` + `AssociatedPaletteFinalize`

Why after encode instead of before:
- the color image remains the primary artifact
- the palette is a derived companion artifact
- a palette failure should not destroy the color image unless we explicitly choose all-or-nothing semantics

Recommended semantics:
- color artifact succeeds even if associated palette generation fails
- but the render log and metadata must show:
  - associated palette requested
  - associated palette failed
  - exact reason

That is the more pragmatic product behavior.


Failure Semantics
-----------------

Need explicit status fields:
- `associated_palette_requested`
- `associated_palette_status`: `not_requested|dependency|running|done|error`
- `associated_palette_error`

Why:
- palette generation is optional companion work
- it should not silently vanish
- the user needs to know whether the image is accompanied by a real generated palette or only by a dependency reference


Download Behavior
-----------------

Current `Render -> Download -> image + meta` does:
- image file
- `_meta.json`

Required change:
- if `associated_palette_mode != none` and the palette image exists:
  - also download the palette image

This applies equally to:
- original Color render artifacts
- derived Color artifacts such as `Autolevels` and `Resize` that inherited the associated palette metadata

Recommended filenames:
- color image: current behavior
- meta: current behavior
- palette image:
  - `<image-base>_palette.jpeg`

Save-to-directory mode:
- save all produced files into the selected directory

Browser-download mode:
- trigger palette image download after meta download and before/after main image

Meta JSON must include:
- all `associated_palette_*` fields
- enough info to reconstruct whether the palette was generated or inherited

Do not download a second palette meta JSON in v1.
One meta JSON for the color artifact is enough if it contains the palette association block.


PDF Artifact Behavior
---------------------

Current ColorSpread PDF:
- left page: text
- right page: selected Color image

Required change:
- if associated palette exists, place a small palette image on the left page under the text
- target size: `5 cm x 5 cm`

Recommended layout:
- keep the existing centered text block
- place the palette square below the last metadata line
- center it horizontally on the left page
- keep generous spacing so the page does not become cramped

Source palette selection rules:
- if `associated_palette_mode == generated`
  - use the generated palette artifact image
- if `associated_palette_mode == dependency`
  - use the source palette artifact image
- otherwise
  - current PDF behavior, no palette square

Needed code touch points:
- [handler_pdf_artifact.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_pdf_artifact.py)
- [spread_pdf.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/spread_pdf.py)


Storage / Inventory Effects
---------------------------

Palette family inventory:
- for `solve_score` + checked, the generated palette artifact appears naturally in the Palette family
- no special fake row should be synthesized

Color family inventory:
- include associated palette summary fields in the Color artifact entry
- useful for:
  - download bundling
  - PDF enrichment
  - future `GoPalette` behavior if wanted later

No special change needed for the Palette tab itself beyond:
- the generated palette artifact must be stored as a normal palette artifact
- switching to the Palette family should show it in inventory


Why This Is Cheaper Than A Separate Palette Run
-----------------------------------------------

Separate later Palette generation would redo:
- solve-score clip
- solve-score hist
- solve-score merge

Associated palette generation inside render reuses:
- the already-created global clip JSON
- the already-created global bins JSON

So the extra cost is only:
- per-chunk all-pass palette bin generation
- palette image assembly/finalize

That is the correct place to save work.


API / Contract Additions
------------------------

Render params:
- add `save_associated_palette: bool`

Popup wiring:
- `Generate` popup must serialize it along with `hist_input_mode`
- `Generate-MT` popup must serialize it along with:
  - `solve_score_threads`
  - `solve_score_hist_input_mode`
  - `solve_score_hist_retries`
  - `solve_score_merge_workers`
  - `raster_input_mode`
  - `raster_sectioned_retries`
  - `raster_mt_threads`
  - `finalize_workers`

Render plan output:
- add an `associated_palette` block, e.g.
  - `enabled`
  - `mode`
  - `palette_id`
  - `prefix`
  - `chunk_bins_prefix`
  - `chunk_meta_prefix`
  - `image_key`
  - `preview_key`
  - `meta_key`

Color artifact metadata:
- add the `associated_palette_*` fields listed above

Storage render-summary:
- expose these fields on Color artifact entries

PDF request path:
- no new user parameter needed
- PDF builder should derive palette inclusion from source artifact metadata


Testing Requirements
--------------------

Workflow tests:
- render workflow definition must verify the new optional associated-palette branch
- field-forwarding tests must pin:
  - metric
  - palette
  - quantile
  - omega
  - omega_enabled
  - root_transforms
  - N
  - times

Handler tests:
- render plan:
  - `solve_score` + checked produces associated palette plan block
  - `saved_palette` + checked produces dependency-only plan block
- associated palette chunk/finalize handlers:
  - reuse render clip/bins keys correctly
  - write normal palette artifact metadata
- storage:
  - Color artifact entries expose associated palette fields
- PDF:
  - ColorSpread includes palette image when association exists
  - ColorSpread omits it when none exists

Frontend tests:
- checkbox exists in both `Generate` and `Generate-MT` popups
- checkbox is correctly gated by effective Color mode
- `Generate` dispatch includes `save_associated_palette`
- `Generate-MT` dispatch includes `save_associated_palette`
- `GenerateFromPalette` opens an execution popup before dispatch
- that popup includes raster/finalize controls and `save_associated_palette`
- that popup omits hist controls
- `image + meta` download path requests palette file when available
- PDF popup path still works with associated palette metadata present

Live smoke after deploy:
1. `solve_score` Color render with checkbox on
2. verify Color artifact exists
3. verify Palette artifact exists in Palette family
4. verify Color artifact metadata points to the Palette artifact
5. download `image + meta` and confirm palette image is also downloaded
6. generate ColorSpread PDF and confirm the left page includes the palette square
7. `saved_palette` Color render with checkbox on
8. verify no new palette artifact is generated
9. verify Color artifact metadata points to the source palette artifact


Implementation Order
--------------------

1. Add render param and plan wiring for `save_associated_palette`
2. Add Color artifact metadata contract for associated palette fields
3. Implement `saved_palette` dependency-only path
4. Implement `solve_score` associated palette workflow branch reusing render clip/bins
5. Expose associated palette fields in storage/render-summary
6. Extend `image + meta` download to include palette image when present
7. Extend ColorSpread PDF to draw the palette square
8. Add tests
9. Run live smoke after deploy


Recommendation
--------------

This feature makes sense.

The right v1 is:
- optional checkbox
- real generated Palette artifact only for `solve_score`
- dependency-only metadata for `saved_palette`
- palette included in download bundle and PDF when available

That gives the user the pairing they want without wasting work or muddying artifact lineage.
