ExtractPalette Plan
===================

Objective
---------

Add an `ExtractPalette` action to `Render -> Color` so the user can select an
existing Color artifact and materialize the palette that belongs with it.

User-facing goals:
- select a Color artifact
- click `ExtractPalette`
- create or recover a real Palette artifact when possible
- associate that palette with the selected Color artifact
- make the palette visible in the Palette family and available for:
  - display beside the image
  - `image + meta + palette` download
  - ColorSpread PDF
  - later `saved_palette` renders when the extracted result is exact/reusable

This is about exact palette recovery from existing render lineage, not about
guessing a palette from the finished image pixels.


Core Decision
-------------

V1 should **not** infer the palette from the final image.

Reason:
- the final image is winner-per-pixel presentation data
- it loses the all-pass per-solve palette-bin data needed for a proper reusable
  palette artifact
- image-only inference would be noisy
- image-only inference would not produce the full metadata contract needed for
  later `saved_palette` renders

So the correct implementation is:
- reuse exact saved render metadata and compute chunks
- reuse any exact solve-score scratch that still exists
- rerun only the missing exact palette stages when necessary


What Exists Today
-----------------

Palette generation is already parallel.

Current palette workflow:
- [palette_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/palette_workflow.asl.json.template)
  - solve-score clip
  - solve-score hist `Map`
  - solve-score merge
  - palette-chunk `Map`
  - palette finalize

Chunk-parallel palette work:
- [handler_palette_chunk.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_chunk.py)

Palette assembly/finalize:
- [handler_palette_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_finalize.py)

Relevant Color render metadata/planning:
- [handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)

Important current fact:
- palette extraction can already reuse exact render metadata
- and if `renders/<job>/solve_scores/...` scratch still exists, it can skip
  clip/hist/merge

So `ExtractPalette` should reuse the existing palette pipeline, not invent a
second image-analysis pipeline.


Recommended User Flow
---------------------

UI placement:
- add `ExtractPalette` button in `Render -> Color`
- it applies to the currently selected Color artifact

Recommended behavior:
1. User selects a Color artifact.
2. User clicks `ExtractPalette`.
3. If the artifact already has an associated palette:
   - do not regenerate
   - log that the association already exists
   - optionally switch/select the Palette artifact
4. If the artifact is a `saved_palette` render without associated-palette fields:
   - do not regenerate
   - attach the source palette as a dependency association
5. If the artifact is a `solve_score` render without associated palette:
   - extract a real Palette artifact using exact compute/render lineage
   - associate that palette with the selected Color artifact

Noisy image inference should not be the default or fallback in v1.


Behavior By Source Artifact
---------------------------

Case 1: Color artifact already has `associated_palette_id`
----------------------------------------------------------

Behavior:
- treat `ExtractPalette` as already satisfied
- do not recompute anything
- log:
  - palette already exists
  - palette id
  - whether it is `generated` or `dependency`

Optional UX:
- switch to Palette family and select it


Case 2: Color artifact is `saved_palette`
-----------------------------------------

Behavior:
- do not regenerate anything
- use the stored source palette metadata:
  - `palette_source_id`
  - `palette_source_display_name`
  - `palette_source_palette`
  - `palette_source_metric`
  - `palette_source_quantile`
  - `palette_source_omega`
  - `palette_source_omega_enabled`
- write the normal associated-palette fields onto the selected Color artifact:
  - `associated_palette_mode = dependency`
  - `associated_palette_id = palette_source_id`
  - etc.

This is exact, cheap, and consistent with the associated-palette model already
planned for render-time generation.


Case 3: Color artifact is `solve_score`
---------------------------------------

Behavior:
- create a real Palette artifact
- use exact compute chunks and exact render metadata

Fast path:
- if matching solve-score scratch still exists:
  - `clip_key`
  - `bins_key`
  - optionally hist scratch
- skip solve-score clip/hist/merge
- run only:
  - `palette_chunk`
  - `palette_finalize`

Slow path:
- if matching solve-score scratch is missing
- rerun the exact solve-score prepass:
  - clip
  - hist
  - merge
- then run:
  - `palette_chunk`
  - `palette_finalize`

This still produces an exact reusable palette artifact.


Case 4: Derived Color artifact (`Autolevels`, `Resize`, similar)
----------------------------------------------------------------

Preferred behavior:
- if associated-palette metadata is already present on the derived artifact:
  - treat as already satisfied
- if the derived artifact does not have associated-palette metadata but the
  lineage source does:
  - attach the inherited association to the derived artifact
- if neither derived artifact nor source has association metadata, but the
  underlying lineage is `solve_score`:
  - extract from the lineage source data, not from the final resized/autoleveled
    pixels

Lineage resolution rule:
- walk upward from the selected Color artifact through
  `derived_from_artifact_id`
- the first artifact that already has a valid associated-palette attachment
  wins
- if none have association metadata, the first artifact in that lineage that
  is a `saved_palette` render provides the dependency source
- if none are `saved_palette`, the first artifact in that lineage that is a
  `solve_score` render provides the extraction source
- never use the final postprocessed pixels as the palette source of truth in v1

Reason:
- postprocess Color artifacts change presentation, not palette lineage

V1 simplification:
- attach/update the selected artifact only
- do not retroactively rewrite every sibling or ancestor artifact

Resulting user-visible behavior:
- `ExtractPalette` on a resized/autoleveled artifact should usually be cheap
  when its source already has association metadata
- in that case it becomes an attach/inherit operation on the selected derived
  artifact, not a new palette computation
- if the lineage has no associated palette yet but does have reusable
  `solve_score` scratch, extraction should reuse that exact scratch


Data Reuse Strategy
-------------------

The extraction logic should reuse exact data in this order:

1. Existing associated-palette metadata on the selected artifact
2. Existing source palette dependency metadata for `saved_palette`
3. Existing exact solve-score scratch under `renders/<job>/solve_scores/...`
4. Existing compute chunks + `calc.json` + Color artifact metadata
5. Never image-only palette inference in v1

This keeps the operation exact and as cheap as the retained data allows.


What Must Be Reconstructed
--------------------------

To make a proper reusable Palette artifact, `ExtractPalette` needs:
- `job_id`
- source Color `artifact_id`
- `degree`
- `N`
- `times`
- exact `root_transforms`
- palette name
- solve metric
- solve-score quantile
- solve-score omega
- solve-score omega-enabled
- compute chunk list with:
  - `chunk_idx`
  - `bin_key`
  - ideally `step_start`
  - ideally `step_count`

These inputs should come from:
- Color artifact metadata
- `calc.json`
- chunk metadata in compute outputs


New Workflow Shape
------------------

Recommended v1:
- a new async workflow dedicated to extraction from a selected Color artifact
- browser dispatches one `extract_palette_orchestrator`
- browser polls status like other artifact jobs

Suggested phases:
1. `plan`
2. `resolve_source`
3. `reuse_or_attach`
4. if needed:
   - `solve_score_clip`
   - `solve_score_hist`
   - `solve_score_merge`
   - `palette_chunk`
   - `palette_finalize`
5. `associate_color_artifact`

Why a dedicated workflow:
- clearer status/logging
- extraction has different semantics from plain Palette tab generation
- easier to express fast-path vs slow-path branching


Association Rules
-----------------

When extraction succeeds, update the selected Color artifact metadata with:
- `associated_palette_mode`
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
- extracted exact palette from `solve_score`:
  - `associated_palette_mode = generated`
- reused source palette from `saved_palette`:
  - `associated_palette_mode = dependency`

The produced Palette artifact itself should be a normal entry in the Palette
family inventory, not a hidden helper object.


Performance / Parallelism
-------------------------

Yes, this should be parallel the same way palette generation already is.

Important clarification:
- palette generation is already chunk-parallel today via Step Functions `Map`
- `ExtractPalette` should reuse that same parallel chunk/finalize machinery

Fastness expectation:
- best case: attach existing association or attach saved-palette dependency
  - essentially instant
- middle case: solve-score scratch still exists
  - skip clip/hist/merge
  - run only `palette_chunk` + `palette_finalize`
- worst exact case: rerun solve-score clip/hist/merge + palette work

This is still better than inventing a noisy image-analysis path that cannot be
reused reliably.


Status / Logging
----------------

Need explicit logs that say which path was taken:
- `ExtractPalette: already associated`
- `ExtractPalette: attached source palette dependency`
- `ExtractPalette: reusing existing solve-score bins`
- `ExtractPalette: rerunning solve-score clip/hist/merge`
- `ExtractPalette: chunk X/Y`
- `ExtractPalette: finalize`

Failure logs must include:
- selected artifact id
- source color mode
- metric
- palette
- quantile
- omega
- whether scratch reuse was attempted
- exact missing prerequisite when it fails


Storage / Inventory Effects
---------------------------

Palette family:
- extracted palettes appear as normal Palette artifacts

Color family:
- selected artifact entry gets updated associated-palette fields

Downloads / PDF:
- once associated-palette fields are present, existing planned behavior applies:
  - `image + meta + palette`
  - ColorSpread palette square


Why Final-Image Inference Is Out Of Scope
-----------------------------------------

Possible future feature:
- `InferPaletteFromImage`

But that is not `ExtractPalette`.

Why not:
- final image only shows winner pixels
- it cannot recover the true all-pass palette-bin data
- it cannot reliably recreate a palette artifact suitable for `saved_palette`
  rendering
- it would mix together postprocess effects with palette lineage

So image inference is a different, approximate feature and should not be used
to define exact artifact lineage.


API / Contract Additions
------------------------

New action:
- `ExtractPalette` from selected Color artifact

New request payload should include at least:
- `job_id`
- `artifact_id`

Planner must resolve:
- source artifact metadata
- calc metadata
- whether association already exists
- whether source color mode is `saved_palette` or `solve_score`
- whether solve-score scratch is still present


Testing Requirements
--------------------

Planner tests:
- existing associated palette -> no-op / attach path
- `saved_palette` source -> dependency attach path
- `solve_score` with scratch present -> skip clip/hist/merge
- `solve_score` without scratch -> full exact rerun path
- derived Color artifact with inherited palette -> no-op attach path

Workflow tests:
- extraction workflow branches correctly between:
  - attach existing
  - attach dependency
  - exact fast-path extraction
  - exact slow-path extraction

Handler tests:
- Color artifact metadata is updated correctly
- produced Palette artifact is normal/reusable
- failures include source artifact context

Frontend tests:
- `ExtractPalette` button exists in `Render -> Color`
- button is disabled when no Color artifact is selected
- dispatch payload uses selected `job_id` + `artifact_id`
- polling/logging show extract-specific phases

Live smoke after deploy:
1. old `solve_score` Color artifact with no associated palette -> extract exact palette
2. old `saved_palette` Color artifact -> attach dependency only
3. Color artifact that already has associated palette -> no-op path
4. verify resulting Color metadata, Palette inventory, and download/PDF behavior


Implementation Order
--------------------

1. Add planner doc/contract for extraction source resolution
2. Add `ExtractPalette` UI action in `Render -> Color`
3. Add extraction planner/orchestrator
4. Implement:
   - existing-association no-op path
   - `saved_palette` dependency attach path
5. Implement exact `solve_score` extraction fast path using existing scratch
6. Implement exact `solve_score` slow path rerunning missing solve-score stages
7. Update Color metadata association
8. Add tests
9. Run live smoke after deploy


Recommendation
--------------

This feature makes sense.

The right v1 is:
- exact-data extraction only
- no image-only inference
- reuse existing palette machinery
- attach existing/dependency palettes immediately when possible
- rerun only the missing exact stages when necessary

That gives the user a real palette artifact they can present beside the image
without muddying lineage or degrading the palette into a noisy approximation.
