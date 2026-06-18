# Palette Tab Refactor Plan

## Problem

The Render tab now supports five solve-score color interpretations:

```text
Scalar LUT
RGB
HSV
RGB LUT
HSV LUT
```

The standalone Palette tab still implements the older scalar-only palette
artifact path. This creates three user-visible problems:

- The Palette tab rejects explicit `emit` / `emit_norm` solve-score programs
  even though 3-channel RGB/HSV rendering is now a first-class workflow.
- The validation message mentions "Palette Generate", which is not visible
  UI terminology and is misleading when the user is thinking in terms of
  Scalar LUT / RGB / HSV / RGB LUT / HSV LUT.
- The Palette tab can queue duplicate equivalent palette calculations because
  palette artifact ids are timestamped, not deterministic.

The Palette tab should be brought up to the same output contract as the Render
tab. Palette artifacts are not inherently scalar; they are reusable pass-0
score/color artifacts. They should support all implemented color
interpretations.

## Current State

### Render Color Path

The modern ColorRender-MT path already supports both scalar and 3-channel
output:

- `index.html`: Render tab exposes `Scalar LUT`, `RGB`, `HSV`, `RGB LUT`,
  `HSV LUT`.
- `js/04-palette-color.js`: `_solveScoreColorCompatibility()` validates
  `Scalar LUT => 1 output` and `RGB/HSV/RGB LUT/HSV LUT => 3 outputs`.
- `lambda/handler_render_plan.py`: validates the selected interpretation via
  `validate_color_output_contract()`, stores `score_output_channel_count`,
  `color_interpretation`, `raw_channels`, and `raw_layout`.
- `lambda/handler_raster_mt.py` and `lambda/roots2pix_mt.c`: emit packed
  raw-score fragments. Scalar records are one byte; multi-output records are
  packed channel bytes.
- `lambda/handler_finalize_mt.py`: associated palettes use
  `_finalize_associated_palette()`, assemble packed raw bytes, and render via
  `raw_score_render`.
- `lambda/score_raw_render.c`: renders `scalar_lut`, `rgb`, `hsv`,
  `rgb_lut`, and `hsv_lut`.

This path is the reference contract.

### Standalone Palette Tab Path

The standalone Palette tab still uses the old scalar workflow:

- `index.html`: Palette tab has no color interpretation selector.
- `index.html`: `#btn-palette-create` is placed in the Stack row and labeled
  `Palette`, which is visually awkward and semantically vague.
- `js/04-palette-color.js`: `_solveScorePaletteCompatibility()` rejects
  `compiled.has_explicit_outputs`.
- `js/09-render-orchestration.js`: `runPaletteArtifact()` dispatches
  `palette_orchestrator` with metric, palette, quantile, omega, chain, and
  root transforms only. It does not send color interpretation or channel
  metadata.
- `js/10-status-results.js`: `runRenderPaletteArtifact()` repeats the same
  scalar-only validation from the Render artifact-family Palette button.
- `lambda/handler_palette_render_plan.py`: rejects
  `compiled_score.get("has_explicit_outputs")`.
- `stepfunctions/palette_workflow.asl.json.template`: forwards no color
  interpretation or channel contract to chunk/finalize.
- `lambda/handler_palette_chunk.py`: runs `solve_palette_chunk` or
  `solve_palette_chunk_mt`, uploads `score_key` and `palette_bins_key`.
- `lambda/solve_palette_chunk_mt.c`: writes `scores_out: float32[count]` and
  `bins_out: uint8[count]`. It evaluates only one scalar program result in the
  chunk path.
- `lambda/handler_palette_finalize.py`: allocates `bytearray(N*N)`, assembles
  one byte per pass-0 sample, renders with `palette_bins_render`, then encodes
  with `raw2jpeg`.
- `lambda/palette_bins_render.c`: renders scalar 0..9 palette bins only.

That path cannot correctly represent a 3-channel palette artifact today.

## Desired Semantics

Palette artifacts use the same solve-score output contract as ColorRender-MT:

```text
Scalar LUT:
  solve-score output count = 1
  data layout = one byte per sample
  render = byte -> optional equalization/LUT -> RGB

RGB:
  solve-score output count = 3
  data layout = packed RGB bytes per sample
  render = direct RGB

HSV:
  solve-score output count = 3
  data layout = packed HSV bytes per sample
  render = HSV -> RGB

RGB LUT:
  solve-score output count = 3
  data layout = packed lookup bytes per sample
  render = per-channel palette lookup as RGB

HSV LUT:
  solve-score output count = 3
  data layout = packed lookup bytes per sample
  render = per-channel palette lookup as HSV -> RGB
```

The Palette tab must validate against the selected interpretation:

```text
Scalar LUT expects exactly 1 output.
RGB expects exactly 3 outputs.
HSV expects exactly 3 outputs.
RGB LUT expects exactly 3 outputs.
HSV LUT expects exactly 3 outputs.
```

There is no valid mode where "leave three values on the solve-score stack"
means RGB/HSV. Multi-channel solve-score output must use explicit emits.

## Terminology

Avoid exposing implementation terms that are not visible in the UI.

Use these labels:

- `Palette tab`: the top-level tab named Palette.
- `Color tab`: the Render artifact family named Color.
- `Scalar LUT`, `RGB`, `HSV`, `RGB LUT`, `HSV LUT`: color interpretation
  modes.
- `Generate Artifact`: the Palette tab action that builds a reusable palette
  artifact.
- `ColorRender-MT`: the Render tab color artifact action.

Do not use `Palette Generate` in user-facing validation messages.

## Duplicate Detection

Current `_palette_variant_id()` (`lambda/handler_palette_render_plan.py:150`)
uses `int(time.time() * 1000)`, so two identical requests produce two
different artifacts.

Replace this with deterministic ids. This is a correctness change, not a
cosmetic one — two traps below make the naive "just drop the timestamp" fix
wrong.

### Trap 1: omega is silently ignored today

`_palette_variant_id(chain, metric, q, omega, omega_enabled, palette,
root_transforms)` **accepts `omega`/`omega_enabled` and then never uses
them** — the returned string is built only from `score_id`
(`solve_score_chain_id`, which folds in chain + quantile), `metric`,
`palette`, and a root-transforms hash, plus the timestamp. The timestamp is
currently the only thing keeping two different-omega requests apart. If you
make the id deterministic by removing the timestamp **without** adding omega,
two renders differing only in omega collide on one id — and rule 4 below
(`fail loudly on mismatch`) then *blocks the user* on a spurious collision,
or silently serves the wrong artifact. The fingerprint must add omega +
interpretation + channel count explicitly; it is not a one-line deletion.

### Trap 2: two entrypoints mint ids

`handler()` has two mutually exclusive entrypoints (gated on whether
`artifact_id` is present): the `mode="palette"` direct-generation branch and
`_build_extract_plan(...)` (extract-from-Color-artifact). Both currently call
`_palette_variant_id`. If the deterministic id is added to only one,
tab-generated and extract-generated artifacts get different ids for identical
content and never dedup against each other. **The deterministic id must come
from one shared function called by both entrypoints.**

This content-identity dedup is *separate from* and does not collide with the
existing reuse logic in `_build_extract_plan` (`extract.action` values
`attach` / `generate_reuse` / `done`): those are lineage/association reuse
("does this Color artifact already carry an associated palette / reusable
solve-score scratch?"), not content-hash dedup. The two can coexist; only the
id function is shared surface.

### Fingerprint Inputs

The palette artifact identity must include, hashed explicitly (do not assume
an existing id such as `score_id` already folds these in — `score_id`
captures chain + quantile only, and the current variant id drops omega):

- `job_id`
- compiled solve-score fingerprint, computed with
  `compiled_solve_score_fingerprint(compiled_score)` from
  `lambda/solve_score_chain.py` after `compile_solve_score_chain_or_legacy()`
  returns. `compile_solve_score_chain_or_legacy()` does **not** return a
  `chain_fingerprint` field directly.
- public solve-score chain
- `metric`
- `quantile`
- `omega` and `omega_enabled` (the currently-dropped inputs)
- selected `color_interpretation`
- output channel count
- output channel metadata relevant to clipping/normalization
- selected palette name only when it affects rendering:
  `scalar_lut`, `rgb_lut`, and `hsv_lut`. For direct `rgb` and `hsv`, palette
  does not affect `score_raw_render`, so canonicalize the palette component to
  `"none"` in the fingerprint/id payload to avoid duplicate identical artifacts
  when only the selected colormap changes.
- root transforms canonical JSON
- any future render-affecting palette settings

### Id Shape

Use a stable hash:

```text
pal_<metric_slug>_<interpretation>_<hash12>
```

Example:

```text
pal_proximity_hsv_lut_a1b2c3d4e5f6
```

Do not include timestamps in the id. Store `created_at` in metadata only.

The hash payload must include an explicit scheme/version tag, for example:

```json
{
  "scheme": "palette_variant_id_v2",
  "job_id": "...",
  "solve_score_fingerprint": "sha256:...",
  "color_interpretation": "hsv_lut",
  "output_channel_count": 3,
  "palette": "inferno"
}
```

Do not infer version from field presence. Future changes to data layout,
normalization, or render-affecting options must bump the scheme string.

### Existing Artifact Reuse

In `lambda/handler_palette_render_plan.py`:

1. Compute the deterministic id (via the one shared function — Trap 2) before
   the expensive chunk/finalize fan-out. The solve-score compile at
   `handler_palette_render_plan.py:614` is cheap; compute its fingerprint with
   `compiled_solve_score_fingerprint(compiled_score)`. The id check goes after
   compile, before fan-out.
2. Check `renders/{job_id}/palettes/{palette_id}/meta.json`.
3. Store one exact `palette_variant_fingerprint` (or
   `content_fingerprint`) field in metadata. If an existing artifact has the
   same fingerprint and required artifact keys exist, return a no-op plan.
4. If metadata exists but does not match, fail loudly. A deterministic id
   collision should not silently overwrite. Scope this hard-fail strictly to
   `palettes/{id}/meta.json`; do not let it wrap the existing
   `_scratch_matches` solve-score-scratch logic, which deliberately falls
   through to regen on mismatch.

Required artifact-key checks:

```text
all modes:
  image_key exists
  preview_key exists

scalar-bin path:
  scalar bin/chunk metadata required by current RePalette flow exists

raw multi-channel path:
  raw_key exists
  raw_meta_key exists
```

The reuse check must compare the exact stored fingerprint string, not
reconstruct equality from a loose set of metadata fields.

Reuse the existing action the workflow already handles:

```text
extract.action = "done"
extract.reason = "palette_artifact_already_exists"
```

The Step Functions workflow already routes `extract.action == "done"` to
`ReportDone` (`palette_workflow.asl.json.template`, `ExtractActionChoice`), so
no new ASL state is needed.

Frontend duplicate prevention remains useful but is not sufficient. The
backend must be authoritative.

## UI Changes

### Palette Tab Layout

Files:

- `index.html`
- `js/04-palette-color.js`
- `js/05-render-popups.js`
- `js/09-render-orchestration.js`
- `js/12-deepzoom-boot.js` if palette swatch bootstrapping changes
- relevant frontend tests

Changes:

1. Move `#btn-palette-create` out of the Stack row.
2. Rename it from `Palette` to `Generate Artifact`.
3. Put it in a toolbar row near `Refresh`, or in a dedicated action row below
   the color interpretation selector.
4. Keep palette swatches in their own row. The Generate button must not share
   the swatch row or overlap it.
5. Add the same color interpretation selector used in Render:

```text
Scalar LUT
RGB
HSV
RGB LUT
HSV LUT
```

6. Show palette swatches only when the interpretation uses a palette:

```text
Scalar LUT
RGB LUT
HSV LUT
```

7. Hide or visually de-emphasize palette swatches for direct `RGB` and `HSV`.
8. The Stack row should only show the stack preview/status, not the Generate
   action.

### Palette Tab State

Add a global state variable:

```javascript
let paletteTabColorInterpretation = 'scalar_lut';
```

Add helpers mirroring Render:

```javascript
function _selectedPaletteColorInterpretation()
function _setPaletteColorInterpretation(value)
function _syncPaletteColorInterpretationUi()
```

Do not share DOM radio names with Render. Use:

```text
name="palette-color-interpretation"
```

### Validation

Replace `_solveScorePaletteCompatibility(compiled)` with an interpretation-aware
helper:

```javascript
function _solveScorePaletteCompatibility(compiled, interpretation)
```

Internally it should call the same count rules as `_solveScoreColorCompatibility`.

Expected messages:

```text
Scalar LUT requires 1 solve-score output; got 3.
RGB requires 3 solve-score outputs; got 1.
HSV requires 3 solve-score outputs; got 1.
RGB LUT requires 3 solve-score outputs; got 1.
HSV LUT requires 3 solve-score outputs; got 1.
```

No message should mention `Palette Generate`.

### Render Tab Palette Family

The Render tab has artifact-family tabs: `Color`, `BiLevel`, `Coeffs`,
`Palette`, `PDF`. The `Palette` family action currently triggers the same
standalone scalar palette workflow.

Decision:

- Either keep the Render artifact-family `Palette` button as a shortcut to the
  same refactored standalone Palette artifact workflow, including the selected
  Render color interpretation.
- Or remove/disable that shortcut and direct users to the top-level Palette tab.

Recommended v1:

- Keep it as a shortcut.
- Use the Render tab's selected color interpretation.
- Validate with the same helper.
- Pass `color_interpretation` through to the palette orchestrator.

Files:

- `js/10-status-results.js`
- `js/04-palette-color.js`
- `tests/e2e/render-solve-score.spec.js`

Update the stale test:

```text
palette family generate rejects explicit output score programs in the frontend
```

It should become:

```text
palette family generate accepts RGB/HSV compatible explicit-output programs
```

and still reject 3-output programs when interpretation is `Scalar LUT`.

### All Palette Launch Surfaces

There are three frontend paths into palette artifact generation and they must
all agree on interpretation/channel behavior:

```text
Palette tab:
  js/09-render-orchestration.js
  runPaletteArtifact()

Render artifact-family Palette shortcut:
  js/10-status-results.js
  runRenderPaletteArtifact()

ExtractPalette popup from a selected Color artifact:
  js/05-render-popups.js
  runExtractPaletteFromPopup()
```

The first two launch direct palette generation. They must pass
`color_interpretation` to `palette_orchestrator`.

The ExtractPalette popup has two routes:

```text
source artifact has solve-score lineage needing scalar recompute:
  target = palette_orchestrator

source artifact has fused raw/step-score sidecar:
  target = extract_palette_fused
```

Keep this split explicit. Do not route a multi-channel raw Color artifact
through the scalar palette-orchestrator extraction branch. If the selected
source artifact is multi-channel and extractable, use the fused extraction
route that already carries raw sidecar/channel metadata.

## Backend Contract

### Plan Payload

`lambda/handler_palette_render_plan.py` should parse:

```json
{
  "params": {
    "color_interpretation": "scalar_lut|rgb|hsv|rgb_lut|hsv_lut",
    "score_output_interpretation": "optional backward alias",
    "solve_score_chain": "...",
    "palette": "inferno",
    "root_transforms": []
  }
}
```

Normalize via:

```python
from color_render_contract import (
    normalize_color_interpretation,
    validate_color_output_contract,
)
```

Use `validate_color_output_contract()` after compiling the solve-score chain.

Deployment packaging requirement: the Palette Render Plan Lambda zip must
include `lambda/color_render_contract.py`, because this helper is not currently
part of the scalar-only plan package.

`color_interpretation` is canonical. `score_output_interpretation` is accepted
only as a backward alias. If both are present and normalize to different modes,
the planner must reject the request instead of choosing one silently.

Plan fields to add:

```json
{
  "params": {
    "color_interpretation": "hsv_lut"
  },
  "solve_score": {
    "output_channel_count": 3,
    "output_channels": [...],
    "chain_fingerprint": "sha256:..."
  },
  "outputs": {
    "raw_key": ".../greyscale.raw",
    "raw_meta_key": ".../greyscale.meta.json",
    "raw_layout": "u8_packed_channels_row_major",
    "raw_channels": 3
  }
}
```

For scalar mode, split implicit and explicit-output behavior:

```text
implicit scalar solve-score output:
  keep existing scalar chunk-bin artifacts for reusable RePalette support

explicit one-output solve-score program:
  use the raw explicit-output path, not the old scalar-bin path
```

This preserves the existing fast scalar path while keeping `emit` /
`emit_norm` semantics consistent with ColorRender-MT. An explicit one-output
program is still valid for `Scalar LUT`, but it is not the same storage layout
as the legacy implicit scalar score.

For 3-channel mode, store packed raw artifacts:

```text
raw_layout = u8_packed_channels_row_major
raw_channels = 3
data_layout = palette_raw_pass0_v1
```

### Workflow

File:

- `stepfunctions/palette_workflow.asl.json.template`

Forward the new fields to chunk and finalize:

```json
"color_interpretation.$": "$.plan.params.color_interpretation",
"score_output_channel_count.$": "$.plan.solve_score.output_channel_count",
"score_output_channels.$": "$.plan.solve_score.output_channels",
"solve_score_chain_fingerprint.$": "$.plan.solve_score.chain_fingerprint",
"raw_key.$": "$.plan.outputs.raw_key",
"raw_meta_key.$": "$.plan.outputs.raw_meta_key",
"raw_layout.$": "$.plan.outputs.raw_layout",
"raw_channels.$": "$.plan.outputs.raw_channels"
```

The workflow shape can remain the same:

```text
plan -> optional solve-score clip/hist/merge -> palette chunks -> finalize
```

But chunk/finalize behavior changes based on channel count.

### Attach Metadata

File:

- `lambda/handler_attach_palette_to_color.py`
- `lambda/color_artifact_meta.py`
- `stepfunctions/palette_workflow.asl.json.template`

When a palette workflow attaches a generated/dependency palette to a Color
artifact, the attach payload must preserve the new palette metadata:

`handler_palette_render_plan.py` must populate these fields on `plan.attach`
when `attach.enabled` is true:

```json
{
  "attach": {
    "color_interpretation": "hsv_lut",
    "raw_key": ".../greyscale.raw",
    "raw_meta_key": ".../greyscale.meta.json",
    "meta_key": ".../meta.json"
  }
}
```

```json
"associated_palette_color_interpretation.$": "$.plan.attach.color_interpretation",
"associated_palette_raw_key.$": "$.plan.attach.raw_key",
"associated_palette_raw_meta_key.$": "$.plan.attach.raw_meta_key",
"associated_palette_meta_key.$": "$.plan.attach.meta_key"
```

`color_artifact_meta.associated_palette_metadata()` already supports
`associated_palette_color_interpretation`, and
`inherit_associated_palette_metadata()` preserves raw-key fields if present.
`handler_attach_palette_to_color.py` must be extended to accept/pass these
fields; otherwise multi-channel associated palette metadata is lost when the
palette is attached to a Color artifact.

## Native Chunk Refactor

Files:

- `lambda/solve_palette_chunk_mt.c`
- `lambda/solve_palette_chunk.c`
- `lambda/handler_palette_chunk.py`
- tests for chunk handler and native parity

### Recommendation

Make `solve_palette_chunk_mt` authoritative for all new multi-channel Palette
tab work. Do not add channel awareness to the legacy `solve_palette_chunk`.

The "keep or retire legacy" question is mostly settled by the facts: the
legacy binary is selected only when
`(not uses_lag) and threads == 1 and input_mode == "tmpfile"`
(`handler_palette_chunk.py:458`), but the render plan defaults
`palette_chunk_input_mode="sectioned"` and `threads=4`, and forces
`sectioned` whenever lag is used. So under the plan-driven workflow the
legacy binary is not selected by default — it is reachable only by a
non-default payload. Leave the binary and its source untouched for scalar
legacy compatibility, but treat the MT binary as the only palette path that
needs multi-channel work.

Implementation requirement: if `score_output_channel_count != 1` or the
compiled solve-score program has explicit outputs, `handler_palette_chunk.py`
must force the MT binary regardless of requested `palette_chunk_threads` or
`palette_chunk_input_mode`. A user-provided `threads=1,input_mode=tmpfile`
must not route a multi-channel or explicit-output palette job through
`solve_palette_chunk`.

### Required Native Behavior

Add output-channel awareness to `solve_palette_chunk_mt`:

```text
--score_output_channel_count=N
--score_output_clip_los=csv
--score_output_clip_his=csv
--color_interpretation=...
```

Use `solve_score_program_output_count()` to verify the request.

For explicit-output programs, evaluate all outputs:

```c
solve_score_eval_program_outputs_from_buffers(...)
```

Do not call the scalar helper:

```c
solve_score_eval_program_from_buffers(...)
```

For each solve row:

```text
Scalar LUT implicit output:
  scores_out[i] = float score
  bins_out[i] = scalar byte/bin

Explicit output count N:
  scores_out[i*N + ch] = float output[ch]       # optional but useful
  bins_out[i*N + ch] = uint8 output_byte[ch]
```

For explicit output count `N`, the handler/native contract must either always
write `scores_out` as `float32[step_count * N]` and record
`score_channels=N`, or not upload scores for that mode. Do not leave this
optional per call site. Recommended v1: always write it because it gives
debugging and future reuse a stable artifact.

Here `i` is the solve-row index in **solve order** (dense), not a grid
position. Keep the chunk emitting in solve order; the serpentine deshuffle to
`(row, col)` destination happens at assembly time — see Sample Ordering under
Finalize Refactor. Do not bake grid coordinates into the chunk output.

For `emit_norm` channels, normalize with per-channel clip bounds from the
lores summary. For `emit` channels, clamp directly to `[0,1]`.

Use the same byte conversion convention as `roots2pix_mt.c`:

```text
explicit channels: round(clamp01(value) * 255)
implicit scalar legacy: 1 + round(clamp01(value) * 254)
```

This preserves the existing "0 means background" convention for implicit scalar
mode while allowing explicit channels to use the full 0..255 byte range.

### Handler Changes

`lambda/handler_palette_chunk.py` should:

- read `score_output_channel_count`
- pass per-channel clip lows/highs to native
- upload multi-channel `palette_bins_key` with expected size
  `step_count * channel_count`
- write chunk metadata:

```json
{
  "score_output_channel_count": 3,
  "raw_channels": 3,
  "raw_layout": "u8_packed_channels_row_major",
  "palette_bins_key": "...",
  "score_key": "...",
  "score_record_count": step_count,
  "palette_bins_bytes": step_count * 3
}
```

For scalar mode, preserve current metadata and file size expectations.

## Finalize Refactor

Files:

- `lambda/handler_palette_finalize.py`
- `lambda/raw_score_render.py`
- `lambda/raw_sidecar.py`
- `lambda/color_render_contract.py`
- `lambda/score_raw_render.c` if new rendering behavior is needed
- tests for finalize

### Scalar Path

Keep the existing implicit scalar path:

```text
assemble uint8[N*N] scalar bins
palette_bins_render
raw2jpeg
data_layout = chunk_all_pass_v1
render_reusable = true
```

This keeps existing RePalette behavior and avoids rewriting the scalar fast
path unnecessarily.

Explicit one-output `Scalar LUT` programs should follow the raw explicit-output
path, not this implicit scalar-bin path. That keeps `emit` and `emit_norm`
normalization consistent with the Render tab and avoids silently changing the
meaning of explicit solve-score outputs.

### Multi-Channel Path

Add a channel-aware finalizer path:

```text
assemble uint8[N*N*channels] packed raw (see Sample Ordering below)
render_score_raw(... channels=3, interpretation=rgb/hsv/rgb_lut/hsv_lut ...)
upload greyscale.raw
upload greyscale.meta.json sidecar
upload image.jpeg + preview.png
data_layout = palette_raw_pass0_v1
render_reusable = false initially
```

#### Sample Ordering (correctness-critical)

The pass-0 parameter grid is written in a **serpentine / boustrophedon**
order: for global solve index `s`, `row = s // N`, `j = s % N`, and
`col = j` on even rows but `col = N-1-j` on **odd** rows. Three different
deshuffle mechanisms exist in the tree and they are not interchangeable:

- `handler_palette_finalize.py` `_copy_pass0_chunk_rows` — dense byte-stream
  deshuffle, 1 channel (the current scalar path).
- `step_scores_to_palette_raw.c` — dense deshuffle with `--channels`, but
  wired only into the Extract-from-step-scores handler.
- `_finalize_associated_palette` / `assemble_greyscale` — sparse
  `(pixel_idx, bytes)` records where the serpentine map is already baked into
  `pixel_idx` by the raster writer; finalize does no ordering math.

Pick ONE and match it exactly. Recommended, lowest-risk: extend the existing
scalar `_copy_pass0_chunk_rows` to N channels — keep `solve_palette_chunk_mt`
emitting dense packed bytes in solve order, and apply the same even/odd-row
reversal with stride = channel count during assembly. This reuses proven,
in-file serpentine math rather than introducing the sparse-fragment contract.
(The associated-palette reference cited above is the model for the *render*
step and the *sidecar metadata shape* — `score_raw_render` + `build_raw_sidecar`
— NOT for the assembly mechanism; do not conflate them.)

Getting the odd-row reversal wrong produces an image that is horizontally
mirrored on alternating rows — invisible to any test that only checks byte
counts. A finalize test must compare assembled output ordering against a
known-good render, not just its length.

`render_reusable=false` is acceptable for v1 because current RePalette support
is scalar-bin based. A later stage can make 3-channel raw palette artifacts
repalette/reinterpretation capable.

Metadata for multi-channel palette artifacts:

```json
{
  "family": "palette",
  "artifact_family": "palette",
  "raw_channels": 3,
  "raw_layout": "u8_packed_channels_row_major",
  "color_interpretation": "hsv_lut",
  "score_output_interpretation": "hsv_lut",
  "raw_key": ".../greyscale.raw",
  "raw_meta_key": ".../greyscale.meta.json",
  "render_reusable": false,
  "data_layout": "palette_raw_pass0_v1"
}
```

Use `build_raw_sidecar()` from `lambda/raw_sidecar.py` if possible, matching
the associated-palette path in `handler_finalize_mt.py`.

Deployment packaging requirement:

```text
deploy.sh:
  Palette Finalize zip must include:
    handler_palette_finalize.py
    shared.py
    color_artifact_meta.py
    color_render_contract.py
    solve_score_chain.py
    raw_sidecar.py
    raw_score_render.py
    palette_bins_render
    raw2jpeg
    score_raw_render
```

The current scalar-only package does not include the raw-sidecar helpers or
`score_raw_render`; multi-channel finalize will fail in Lambda unless the
package is updated.

## Extraction And Reuse

Existing Color artifact extraction already handles 3-channel step scores in:

- `lambda/handler_extract_palette_from_step_scores.py`

Do not break that path. It is already closer to the desired multi-channel
palette model than the standalone Palette tab.

For v1:

- Standalone scalar Palette artifacts remain reusable via chunk bins.
- Standalone 3-channel Palette artifacts are viewable/downloadable raw-color
  artifacts but not scalar-bin reusable.
- Associated palettes from ColorRender-MT remain supported.

Future v2:

- Make 3-channel palette artifacts reusable by storing packed raw sidecars and
  supporting reinterpretation/repalette through `color_recolor_raw.py`.

## Frontend Inventory And Display

Files:

- `lambda/handler_storage.py`
- `js/11-artifacts.js`
- `js/04-palette-color.js`
- `js/09-render-orchestration.js`

Storage inventory should expose for palette entries:

```json
{
  "color_interpretation": "hsv_lut",
  "score_output_interpretation": "hsv_lut",
  "raw_channels": 3,
  "raw_layout": "u8_packed_channels_row_major",
  "render_reusable": false
}
```

Concrete backend hook:

```text
lambda/handler_storage.py:
  _list_saved_palettes(job_id)
```

`_list_saved_palettes()` currently normalizes image/preview keys, palette-bin
keys, `render_reusable`, and `data_layout`. Extend that palette-family reader
to normalize and expose the same channel fields that the Color-family reader
already exposes:

```text
color_interpretation
score_output_interpretation
score_output_channel_count
raw_channels
raw_layout
raw_key
raw_meta_key
```

Do not rely on raw `meta.json` pass-through. UI code should receive consistent
typed fields for Palette artifacts the same way it receives them for Color
artifacts.

UI summaries should show:

```text
HSV LUT · 3ch · inferno · score program display
```

For scalar entries:

```text
Scalar LUT · reusable bins · inferno · score program display
Scalar LUT · raw explicit output · inferno · score program display
```

The scalar summary must surface the storage/reuse distinction introduced by
the implicit-vs-explicit split:

```text
implicit scalar score:
  render_reusable = true
  label includes "reusable bins"
  scalar RePalette enabled

explicit one-output Scalar LUT:
  render_reusable = false
  label includes "raw explicit output"
  scalar RePalette disabled unless raw-sidecar repalette is added later
```

Disable scalar-only `RePalette` for 3-channel standalone Palette artifacts and
for explicit one-output raw Scalar LUT artifacts unless/until the raw-sidecar
repalette path is explicitly supported.

Concrete frontend gate:

```text
js/04-palette-color.js:
  _canRepaletteArtifact(art)

js/05-render-popups.js:
  popup summary already reads art.raw_channels and art.render_reusable
```

Update `_canRepaletteArtifact(art)` to require scalar reusable bins:

```javascript
function _canRepaletteArtifact(art) {
  if (!art || !art.palette_id) return false;
  if (Number(art.raw_channels || art.score_output_channel_count || 1) > 1) return false;
  if (art.render_reusable === false) return false;
  return !!(art.palette_bins_key || art.chunk_bins_prefix);
}
```

Backend gate:

```text
lambda/handler_repalette.py
```

Frontend button disabling is not sufficient. `handler_repalette.py` must also
reject source palette artifacts that are not scalar reusable-bin artifacts:

```text
raw_channels > 1 -> reject
score_output_channel_count > 1 -> reject
render_reusable is false -> reject unless the legacy pass-0 scalar-bin path is
  deliberately still supported for old artifacts
missing palette_bins_key/chunk_bins_prefix -> reject with a clear message
```

This prevents direct API calls from trying to route raw RGB/HSV palette
artifacts through `palette_bins_render`.

## Tests

### Frontend Tests

Files:

- `tests/test_frontend_js.sh`
- `tests/e2e/render-solve-score.spec.js`
- `tests/e2e/palette-ui.spec.js`

Required coverage:

- Palette tab has color interpretation selector.
- Palette tab Generate Artifact button is not inside the Stack row.
- Scalar LUT rejects 3 explicit outputs.
- RGB/HSV/RGB LUT/HSV LUT reject scalar-only programs.
- RGB/HSV/RGB LUT/HSV LUT accept exactly 3 explicit outputs.
- Render artifact-family Palette shortcut uses selected Render interpretation.
- ExtractPalette popup does not route multi-channel raw Color artifacts through
  the scalar palette-orchestrator extraction branch.
- Palette tab and Render shortcut both include `color_interpretation` in the
  palette-orchestrator payload.
- User-facing messages do not contain `Palette Generate`.

### Python Plan Tests

Files:

- `tests/test_palette_render_plan.py`
- `tests/test_palette_workflow_definition.py`

Required coverage:

- Plan accepts 3-channel explicit-output score program for `rgb`.
- Plan accepts 3-channel explicit-output score program for `hsv_lut`.
- Plan rejects 3-output program for `scalar_lut`.
- Plan rejects scalar program for `rgb/hsv/rgb_lut/hsv_lut`.
- Plan includes `color_interpretation`, `output_channel_count`,
  `output_channels`, `chain_fingerprint`, `raw_key`, `raw_meta_key`,
  `raw_layout`, and `raw_channels`.
- Deterministic palette id is stable across identical inputs.
- Discrimination (the cases dedup bugs actually live in — id collisions, not
  misses): id DIFFERS when only `color_interpretation` differs; when only
  `omega` differs (the currently-dropped input — guard against regression);
  when only the channel count differs; when only the palette or a root
  transform differs.
- The `mode="palette"` and `_build_extract_plan` entrypoints produce the SAME
  id for the same content (shared id function — Trap 2).
- Existing matching artifact returns `done` / no-op plan.
- Metadata mismatch on deterministic id raises an error.
- ASL forwards all new fields to chunk/finalize.
- ASL forwards associated-palette interpretation/raw metadata to
  `handler_attach_palette_to_color.py`.
- `handler_attach_palette_to_color.py` preserves
  `associated_palette_color_interpretation`, `associated_palette_raw_key`,
  `associated_palette_raw_meta_key`, and `associated_palette_meta_key`.
- `handler_storage.py` exposes normalized channel/raw fields for Palette-family
  inventory entries, not just Color-family inventory entries.
- `handler_repalette.py` rejects raw/multi-channel/non-reusable palette
  artifacts server-side.

### Native And Handler Tests

Files:

- `tests/test_palette_chunk_handler.py`
- `tests/test_palette_finalize_handler.py`
- `tests/test_palette_chunk_mt_parity.py`
- Docker/native regression tests

Required coverage:

- `solve_palette_chunk_mt` scalar path still writes `step_count` bytes.
- `solve_palette_chunk_mt` 3-channel path writes `step_count * 3` bytes.
- `emit_norm` channel uses per-channel clip bounds.
- `emit` channel uses identity clamp `[0,1]`.
- `handler_palette_chunk.py` records channel metadata and validates output
  sizes.
- `handler_palette_finalize.py` scalar path still calls `palette_bins_render`.
- `handler_palette_finalize.py` 3-channel path calls `score_raw_render` with
  `--channels=3` and the selected interpretation.
- Final metadata includes raw sidecar keys and channel fields.

### Integration Tests

Add at least one end-to-end-ish mocked test for:

```text
Palette tab + HSV LUT + 3 emit_norm outputs -> palette plan -> chunk -> finalize metadata
```

Add duplicate-detection tests — both the stable case and the discriminating
cases (a dedup scheme fails by colliding distinct content, not by missing
identical content):

```text
same job + same program + same interpretation + same palette -> same palette_id
same everything except interpretation                         -> different palette_id
same everything except omega                                  -> different palette_id
same everything except palette                                -> different palette_id
```

## Rollout Stages

### Stage 0: UI Terminology And Layout Cleanup

Safe, no backend contract change.

- Rename button to `Generate Artifact`.
- Move it out of the Stack row.
- Replace `Palette Generate` wording with visible UI terminology.
- Keep scalar-only validation for now, but make the message say:

```text
Palette tab currently supports Scalar LUT only. RGB/HSV palette artifacts are not enabled yet.
```

Exit criteria:

- No overlapping Palette button/swatch layout.
- No user-facing `Palette Generate` string.

### Stage 1: Shared Interpretation Selector And Validation

Frontend only, but still disabled for unsupported backend until Stage 2.

- Add Palette tab color interpretation selector.
- Add interpretation-aware validation.
- Keep 3-channel Generate disabled with a clear "backend not enabled" message
  until native support lands, or land Stage 1 and Stage 2 together.

Recommended: land Stage 1 and Stage 2 together to avoid exposing a control that
cannot run.

### Stage 2: Backend Plan Contract And Deterministic Ids

- Add `color_interpretation` to palette planner.
- Use `validate_color_output_contract()`.
- Make palette ids deterministic.
- Add already-existing no-op detection.
- Update ASL field forwarding.

Exit criteria:

- Plan tests cover scalar and 3-channel validation.
- Duplicate equivalent requests produce the same `palette_id`.

### Stage 3: Native Chunk Multi-Channel Output

- Update `solve_palette_chunk_mt.c`.
- Update `handler_palette_chunk.py`.
- Prefer MT binary for all new palette chunk execution.
- Keep scalar output unchanged.

Exit criteria:

- Native tests prove output byte counts and channel values.
- Existing scalar palette chunk tests still pass.

### Stage 4: Finalize Multi-Channel Artifacts

- Add scalar vs multi-channel finalize branch.
- Multi-channel branch assembles packed raw and renders via `score_raw_render`.
- Upload raw sidecar and metadata.

Exit criteria:

- Scalar finalize tests still pass.
- 3-channel finalize tests pass for `rgb`, `hsv`, `rgb_lut`, `hsv_lut`.

### Stage 5: Inventory, RePalette Rules, And UX Polish

- Show channel/interpretation in palette inventory.
- Disable scalar-only RePalette where inappropriate.
- Ensure Populate carries interpretation back into Render/Palette settings.
- Ensure active-run guard and duplicate no-op messages are clear.

Exit criteria:

- UI displays 3-channel palette artifacts accurately.
- Duplicate click either no-ops or reports existing artifact instead of queuing
  another equivalent workflow.

### Stage 6: Deployment Gates

Required before deploy:

- Python tests for plan/chunk/finalize/storage.
- Frontend grep/e2e tests.
- Native rebuild for `solve_palette_chunk_mt`.
- Docker/native runtime parity for scalar and 3-channel palette chunks.
- Palette workflow ASL definition tests.
- Deployment checklist section for palette native binary rebuild.
- Deploy packaging tests prove Palette Render Plan includes
  `color_render_contract.py`.
- Deploy packaging tests prove Palette Finalize includes `raw_score_render.py`,
  `raw_sidecar.py`, `color_render_contract.py`, and the `score_raw_render`
  binary in addition to the existing scalar `palette_bins_render` / `raw2jpeg`.

## Non-Goals For V1

- Do not unify Palette and Color workflows completely.
- Do not remove existing scalar Palette artifacts.
- Do not make old scalar RePalette support work for 3-channel raw artifacts in
  the same change.
- Do not change solve-score program semantics.
- Do not allow implicit 3-output stack results. Multi-channel output still
  requires explicit emit chips.
- Do not silently reinterpret scalar palette bins as RGB/HSV channels.
- Do not refactor the lores/debug-only `handler_palette_debug.py` /
  `solve_palette_debug` path as part of this work. It is not the orchestrated
  Palette tab artifact workflow. If its `persistent` mode remains exposed
  anywhere, either disable that mode or port it to the same deterministic-id
  contract; do not leave a second user-visible timestamped palette-artifact
  creator.

## Risks

### Old And New Palette Artifacts Have Different Data Layouts

Scalar standalone Palette artifacts use scalar bins and are reusable for
RePalette. New 3-channel standalone artifacts should initially be raw packed
artifacts. Metadata must make this obvious.

### Finalize Branch Complexity

`handler_palette_finalize.py` will have two render branches. Keep the scalar
branch unchanged and add the multi-channel branch separately. Do not try to
force scalar bins through the raw packed path in the same refactor.

### Duplicate Id Collisions

Deterministic ids require strict metadata verification before reuse. If the id
exists with mismatched metadata, fail rather than overwrite.

### User Mental Model

The user sees "Palette" both as a top-level tab and as a colormap choice. The
UI must distinguish:

- color interpretation: `Scalar LUT`, `RGB`, `HSV`, `RGB LUT`, `HSV LUT`
- palette/colormap: `inferno`, `viridis`, etc.
- artifact family: `Palette`

## Implementation Priority

Recommended order:

1. Stage 0 terminology/layout cleanup.
2. Stage 2 deterministic id and backend contract.
3. Stage 3 native chunk multi-channel output.
4. Stage 4 finalize multi-channel artifact rendering.
5. Stage 1 UI selector enabled once backend is ready.
6. Stage 5 inventory polish.
7. Stage 6 full deploy gate.

The most important correctness work is Stages 2-4. The most important UX work
is Stage 0 plus adding the Palette tab interpretation selector.
