# Chips For Solve Scores Plan

Status: plan only.

## Objective

Replace the current Solve score UI:

- metric dropdown
- omega checkbox
- omega slider

with the same chip-style input model already used for coefficient transforms.

The new model should support:

- one or more solve-score chips in a visible ordered chain
- a final optional transfer chip instead of the current omega checkbox
- future metric/transform parameters without another UI redesign

This change applies to both:

- `Render -> Color -> Solve score`
- `Palette -> Solve Score Palette`

## Important Correctness Note

The current backend does **not** implement a literal `sin(w*u*a)` final step.

Current code in:

- [solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)

implements:

- identity when omega is off
- `0.5 * (cos(omega * 2π * u) + 1)` when omega is on

So v1 should preserve current output exactly.

That means:

- do **not** silently rename the existing behavior to `sin(...)` if the math is still cosine-based
- the final chip in v1 should map to the current implementation exactly
- if a literal sine-family transfer is wanted later, that is a separate behavior change

## Current State

### Render tab

Solve-score controls currently live in:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Current state variables:

- `renderSolveMetric`
- render quantile slider
- render omega checkbox + slider

Current payload emission:

- `_renderCommonParams()`
- `_launchRenderOrchestrator()`
- `runSolveScoreHistogramDebug()`
- `runRenderPaletteArtifact()`

### Palette tab

Palette solve-score controls currently live in:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Current state variables:

- `paletteTabMetric`
- palette quantile slider
- palette omega checkbox + slider

Current payload emission:

- `runPaletteArtifact()`

### Backend contract

Current backend plan/build path is scalar, not chain-based.

It expects:

- `solve_metric`
- `solve_score_quantile`
- `solve_score_omega`
- `solve_score_omega_enabled`

Main files:

- [handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- [handler_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_render_plan.py)
- [handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
- [render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)
- [palette_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/palette_workflow.asl.json.template)

### Artifact metadata

Current artifact metadata stores scalar solve-score fields, and downstream UI reads them.

Examples:

- `solve_metric`
- `solve_score_quantile`
- `solve_score_omega`
- `solve_score_omega_enabled`
- `palette_source_metric`
- `palette_source_quantile`
- `palette_source_omega`
- `associated_palette_metric`
- `associated_palette_quantile`
- `associated_palette_omega`

Main readers/writers:

- [handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
- [handler_pdf_artifact.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_pdf_artifact.py)
- [color_artifact_meta.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/color_artifact_meta.py)

## Core Decision

Do this in two layers.

### Layer 1: chip UI + metadata model

Add a chip-based solve-score chain in the frontend and store that chain in metadata.

### Layer 2: compile to the current scalar backend contract

Do **not** change the native solve-score pipeline in v1.

Instead:

- compile the chip chain down to the existing scalar fields
- keep Step Functions and native binaries working exactly as they do now

This gives the new UI and persistence model without changing rendered output or expanding backend scope too early.

## Proposed V1 Chain Grammar

### Render solve-score chain

Add a dedicated chain for render solve-score mode, for example:

- `_renderScoreChain`

### Palette solve-score chain

Add a dedicated chain for palette generation, for example:

- `_paletteScoreChain`

### Catalog

Add a dedicated solve-score chip catalog, for example:

- `_ssCatalog`

Initial entries:

- all current metrics as chips
  - `proximity`
  - `crowding`
  - `spread`
  - `anisotropy`
  - `area`
  - `clusteriness`
  - `shelliness`
  - `outlierness`
  - `nn_variation`
  - `real_axis_proximity`
  - `centroid_re`
  - `centroid_im`
  - `centroid_dist`
  - `dist_unit_circle`
  - `asymmetry_re`
- one final transfer chip representing the current omega behavior
  - recommended honest name: `omega_cosine`
  - parameter: `omega`

### V1 validity rules

V1 should be intentionally strict:

- exactly one metric chip is required
- the metric chip must be first
- at most one final transfer chip is allowed
- the transfer chip must be last
- no other solve-score transforms are allowed yet

This keeps the chip model real while still compiling cleanly to the current scalar API.

### V1 add-time enforcement

Do not rely only on dispatch-time rejection.

The add-chip UI should enforce the grammar as the user builds the chain:

- once a metric chip exists, metric chips disappear or become disabled in the add dropdown
- once a transfer chip exists, transfer chips disappear or become disabled in the add dropdown
- if the chain is empty, only metric chips are addable
- if the chain already has one metric and no transfer, only the transfer chip is addable

Dispatch-time validation still stays in place, but the UI should not invite invalid chains in the first place.

### V1 meaning

- no transfer chip present:
  - `solve_score_omega_enabled = false`
  - `solve_score_omega = 1`
- transfer chip present:
  - `solve_score_omega_enabled = true`
  - `solve_score_omega = <chip param>`

Quantile stays separate in v1:

- keep the current quantile slider
- do **not** make quantile a chip yet

That keeps the first migration small.

## Frontend Design

### 1. Reuse the existing chip renderer

Current chip infrastructure already exists in:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Relevant functions:

- `addChip()`
- `removeChip()`
- `updateChipParam()`
- `_catalogForChain()`
- `_chainForWhich()`
- `_renderChips()`
- `_serializeNamedChain()`

The solve-score plan should reuse that system, not invent a second chip framework.

### 2. Add new chip namespaces

Extend the chip router to support two new namespaces, for example:

- `ss`
- `palette-ss`

That means:

- `_catalogForChain('ss') -> _ssCatalog`
- `_catalogForChain('palette-ss') -> _ssCatalog`
- `_chainForWhich('ss') -> _renderScoreChain`
- `_chainForWhich('palette-ss') -> _paletteScoreChain`

### 3. Replace the current metric dropdown and omega controls

In both Render and Palette tabs:

- remove the solve-score metric `<select>`
- remove the omega checkbox
- remove the omega slider
- replace them with:
  - a chip container
  - an add-chip dropdown

The palette swatch row stays.

The quantile slider stays.

### 4. Add solve-score-specific chip rendering

The current renderer has coeff-transform special cases in `_renderRtChipHtml()`.

That should be generalized into something like:

- `_renderNamedChipHtml(item, i, which, catalog)`

and then extended for solve-score chips:

- metric chips render as simple named chips
- `omega_cosine` renders with one input and a compact label

Example chip display:

- `clusteriness`
- `omega_cosine(3)`
- preferred compact form: `ω-cos(3)`

The full formula:

- `g(u)=0.5*(cos(ω*2π*u)+1)`

should live in a tooltip or info affordance, not in the chip body.

## Compile Step: Chain To Legacy Scalar Fields

Add a compiler helper in the frontend, for example:

- `_compileSolveScoreChain(chain)`

Return shape:

- `metric`
- `omega`
- `omega_enabled`
- `display`
- `errors`

This helper should:

- validate the v1 grammar
- reject invalid chains before dispatch
- derive the legacy scalar backend fields

This same logic should also exist in Python.

Compiled output must be used everywhere that currently talks to scalar solve-score code, including:

- `_launchRenderOrchestrator()`
- `runPaletteArtifact()`
- `runRenderPaletteArtifact()`
- `runSolveScoreHistogramDebug()`

The histogram debug path is easy to miss because it is not part of the main render dispatch, but it still feeds scalar-native solve-score code and must compile the chain first.

## Server-Side Validation And Compilation

Frontend-only validation is not enough.

Add matching compilation/validation helpers to:

- [handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- [handler_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_render_plan.py)

Recommended behavior:

- if `solve_score_chain` is present:
  - validate it
  - compile it to:
    - `solve_metric`
    - `solve_score_omega`
    - `solve_score_omega_enabled`
- if `solve_score_chain` is absent:
  - continue to accept the legacy scalar fields

This gives a clean migration path and avoids trusting JS as the only source of truth.

### Compiler parity requirement

The JS compiler and Python compiler must stay equivalent on v1 chain semantics.

Add a shared fixture, for example:

- `tests/solve_score_chain_vectors.json`

with cases like:

- metric only
- metric + `omega_cosine`
- invalid second metric
- invalid transfer-first
- invalid duplicate transfer

Then test both:

- JS `_compileSolveScoreChain()`
- Python solve-score chain compiler helper

against the same chain-to-scalar expectations.

## Metadata Plan

### Canonical new fields

Add canonical chain metadata fields:

- `solve_score_chain`
- `palette_source_score_chain`
- `associated_palette_score_chain`

These should store the serialized chip chain, for example as compact JSON text.

### Single metadata helper

Do not hand-roll chain/scalar emission at every write site.

Add one helper that takes:

- a chain
- quantile
- an optional prefix such as `""`, `palette_source_`, or `associated_palette_`

and emits both:

- canonical chain metadata
- compiled scalar mirror fields

For example, a helper like:

- `emit_solve_score_metadata(chain, quantile, prefix="")`

should be responsible for writing:

- `solve_score_chain`, `palette_source_score_chain`, or `associated_palette_score_chain`
- `solve_metric`, `palette_source_metric`, or `associated_palette_metric`
- `solve_score_quantile`, `palette_source_quantile`, or `associated_palette_quantile`
- `solve_score_omega`, `palette_source_omega`, or `associated_palette_omega`
- `solve_score_omega_enabled`, `palette_source_omega_enabled`, or `associated_palette_omega_enabled`

This is the safest way to avoid N slightly different implementations across render, palette, associated-palette, storage overlays, and inherited metadata paths.

### Keep existing scalar fields

Do **not** remove the old scalar fields in v1.

Continue writing:

- `solve_metric`
- `solve_score_quantile`
- `solve_score_omega`
- `solve_score_omega_enabled`

and the palette-source / associated-palette scalar mirrors.

Reason:

- current workflows already use them
- summaries and PDF text already read them
- old artifacts and old UI populate paths still depend on them

### Populate behavior

When populating UI from an existing artifact:

1. if chain metadata exists:
   - load the chain directly
2. else:
   - reconstruct a chain from legacy scalar fields

Legacy reconstruction rule:

- metric chip from `solve_metric`
- add `omega_cosine(omega)` chip only if `solve_score_omega_enabled != false`

### Derived/inherited metadata

Anything that already propagates solve-score scalar metadata should also propagate the chain:

- saved palettes
- associated palettes
- autolevels / resize descendants that inherit palette lineage

That propagation should use the same metadata helper, not bespoke field-by-field copies.

## Summary And Display Plan

Artifact summaries should continue to be short.

Do not dump the full serialized chain into the table.

Preferred display rule:

- derive a concise label from the compiled chain
- keep the current short style

Examples:

- `solve:clusteriness q=0.1% w=3`
- `saved:clusteriness q=0.1% w=3`

If the transfer chip is absent:

- show `w=off`

The artifact table does not need to expose the full chain in v1.

### Display-name compatibility rule

For identical inputs, the new chain-derived display name must produce the same output string the repo already emits today.

That matters because existing downstream code already depends on the current scalar-derived format, including:

- `_associated_palette_display_name()` in [handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- storage/inventory summary fields in [handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
- PDF artifact labeling in [handler_pdf_artifact.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_pdf_artifact.py)
- score labels in [_score_or_color_label()](/Users/nicknassuphis/karpo_hackathon/polypaint/make_polypaint_book.py#L575) inside [make_polypaint_book.py](/Users/nicknassuphis/karpo_hackathon/polypaint/make_polypaint_book.py)

So v1 should treat the compiled scalar label as the source of truth for display formatting, not invent a new chain-specific string format.

## Implementation Steps

### Phase 1: UI and state

1. Add `_ssCatalog`
2. Add `_renderScoreChain` and `_paletteScoreChain`
3. Extend the chip helpers to support `ss` and `palette-ss`
4. Replace the metric dropdown + omega controls in both tabs
5. Add add-time enforcement to the add dropdown so invalid chains are not constructible from the normal UI
6. Add `_compileSolveScoreChain()` in JS
7. Make histogram debug compile the chain before dispatch
8. Update:
   - `_renderCommonParams()`
   - `runPaletteArtifact()`
   - `runSolveScoreHistogramDebug()`
   - `runRenderPaletteArtifact()`
   - `_launchRenderOrchestrator()`

### Phase 2: server compatibility

1. Add Python-side chain validation/compiler
2. Add a shared JS/Python compiler parity fixture and test both compilers against it
3. Accept optional `solve_score_chain` in render/palette plan handlers
4. Derive legacy scalar fields from the chain server-side
5. Keep workflows unchanged

### Phase 3: metadata and populate

1. Add the single solve-score metadata emission helper
2. Write chain metadata alongside legacy scalar fields through that helper
3. Update storage/inventory parsing
4. Update populate-from-artifact logic to prefer the chain
5. Update summary rendering to use chain-derived labels when present, while preserving the existing display string format

### Phase 4: downstream polish

1. Update palette/source/associated-palette metadata copying
2. Update PDF text generation if needed
3. Update logs and histogram text to use the chain-derived transfer label

## Tests

### Frontend

Add/extend tests in:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

Coverage:

- solve-score chip containers exist in render and palette tabs
- metric dropdown and omega checkbox/slider are gone
- once one metric chip exists, metric chips are no longer addable
- once the transfer chip exists, transfer chips are no longer addable
- metric chip serializes correctly
- transfer chip serializes correctly
- invalid chains refuse dispatch
- render payload includes:
  - `solve_score_chain`
  - legacy compiled scalar fields
- palette payload includes:
  - `solve_score_chain`
  - legacy compiled scalar fields
- populate from artifact restores chips from:
  - canonical chain metadata
  - legacy scalar fallback

### Render plan tests

Add/extend:

- [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)

Coverage:

- valid chain compiles to expected scalar fields
- Python compiler matches the shared chain-to-scalar fixture
- absent transfer chip yields `omega_enabled=false`
- invalid chain is rejected
- legacy scalar-only payload still works
- chain metadata is emitted into outputs metadata
- display name matches the existing scalar-derived format for equivalent inputs

### Palette plan tests

Add/extend:

- [tests/test_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_render_plan.py)

Coverage:

- same as render plan, but for palette artifacts and extraction/reuse paths
- prefixed palette-source / associated-palette metadata emission stays in sync with the canonical chain

### Storage / pipeline tests

Add targeted coverage where needed for:

- summary label generation
- populate behavior
- inherited palette metadata carrying the chain

Likely files:

- [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)

### Docker / native

V1 should not require native solve-score pipeline changes.

So no new Docker runtime gate should be necessary beyond the existing required gate for the overall native repo state.

If phase 2 later changes native binaries to consume score chains directly, Docker becomes mandatory again.

## Risks

### 1. Naming mismatch

If the chip says `sin(...)` but the backend is still cosine, the UI will be lying.

Do not do that.

### 2. UI ambiguity

If chips are allowed in arbitrary order before the backend supports real chaining, the UI will promise more than the system can do.

So v1 must enforce a narrow grammar.

### 3. Metadata sprawl

There are already:

- source solve-score fields
- palette-source fields
- associated-palette fields

Adding chain metadata must be done systematically, not ad hoc.

### 4. Backward compatibility

Old artifacts will not have chain metadata.

Populate and summary code must keep the legacy reconstruction path.

## Recommended V1 Scope

Do this now:

- chip UI for solve-score selection
- optional final transfer chip replacing omega checkbox/slider
- chain metadata
- server-side chain compilation into the existing scalar fields
- summary/populate compatibility

Do **not** do this yet:

- arbitrary score-transform chaining in native binaries
- multiple metric chips
- mid-chain score transforms
- changing the current transfer math from cosine to something else

That is the right cut if the goal is:

- better UI now
- room for future parameterized score transforms
- no accidental change to render outputs
