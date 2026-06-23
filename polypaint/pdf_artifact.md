# PDF Artifact

Status: implemented.

## Current Implementation Contract

The Render PDF artifact path creates an immutable `pdf` artifact from an existing saved Color artifact. The live backend is Lambda + Pillow + ReportLab:

`Render PDF ColorSpread UI -> handler_dispatch.py -> handler_pdf_artifact.py -> spread_pdf.py`

This path does not use R, knitr, LaTeX, `.Rnw`, or `scripts/compile_knitr.sh`. Those tools are only for local report documents under `reports/`.

V1 supports one PDF subtype:

- `pdf_kind = color_spread`
- source family: `color`
- output file: `renders/{job_id}/pdf/{artifact_id}/document.pdf`
- first spread, left page: provenance report
- first spread, right page: selected Color artifact image
- optional appendix spreads: full program source listings

Large source rasters are downsampled before ReportLab sees them. `spread_pdf.prepare_pdf_image(...)` inspects dimensions, enforces a deliberate Pillow decompression-bomb limit, optionally uses decoder `draft(...)`, resizes with LANCZOS, writes a prepared temporary image, and returns original/prepared dimensions.

Default preparation caps:

- main image: `PDF_IMAGE_MAX_PX=3600`
- palette/reference image: `PDF_PALETTE_MAX_PX=800`
- target DPI: `PDF_TARGET_DPI=300`
- prepared format: lossless PNG by default

`handler_pdf_artifact.py` reports `prepare_image` before `compose_pdf`, and the final metadata/status rows include:

- `source_width`, `source_height`
- `prepared_width`, `prepared_height`
- `image_resized`, `image_max_px`
- palette equivalents when an associated palette is present
- `page_count`

If a single-shot PDF job reaches the frontend hard-stale threshold without a terminal row, the UI offers `Abandon stalled PDF job`. That clears only the local active-run lock through `_clearActiveRun()`; it does not cancel server work or delete audit rows.

The report page answers: "what exactly produced this image?" It renders compute id, Color artifact id, function/degree/N/times/solver, color mode, interpretation, palette, output channels, viewport, and a large labeled palette swatch when an associated palette is available. Program/source details are not excerpted on the summary page; they flow to appendix spreads in Courier code blocks.

The visual treatment is the CR21 dark navy/signal red report style:

- `PAGE_BG = #1a1a2e`
- `ACCENT = #e94560`
- `TEXT = #f2f2f7`
- `MUTED = #9aa0b4`
- `PANEL_BG = #121829`
- `PANEL_BORDER = #2b3a5e`
- `CODE_TEXT = #e6e9f2`

Tests covering the current path:

- `tests/test_spread_pdf.py`: real image preparation and PDF smoke generation.
- `tests/test_pdf_artifact_handler.py`: status phases, prepared-image handoff, dimension metadata, and PDF metadata.
- `tests/test_frontend_js.sh`: stale PDF wording and local abandon action.

## Historical Design Notes

The remainder of this document is the original V1 design note. Where it conflicts with the current implementation contract above, the current contract wins.

This document describes a new fifth Render-family artifact type:

- `PDF`

V1 scope is intentionally narrow:

- only one PDF generation mode exists: `ColorSpread`
- input is one saved `Color` artifact
- output is one immutable PDF artifact, with appendix spreads when needed
- left page is a provenance report derived from artifact/job metadata
- right page is the selected prepared Color image

The PDF family is for downloadable/inspectable summary documents, not for DeepZoom, post-processing, or solve recomputation.

## Goal

Add a new `PDF` family beside:

- `Color`
- `BiLevel`
- `Coeffs`
- `Palette`

The user flow is:

1. Open `Render`
2. Switch to `PDF`
3. Click `ColorSpread`
4. Pick one saved `Color` artifact from a popup
5. Execute
6. A new immutable PDF artifact is created
7. The new artifact appears in the `PDF` catalog
8. The right-side viewer displays the PDF
9. `Download` downloads the `.pdf`

This is a derivative artifact flow, similar in spirit to `Autolevels`, `RePalette`, and `Color RePalette`, but the output family is `pdf`.

## V1 Behavior

V1 supports exactly one PDF subtype:

- `pdf_kind = "color_spread"`

It uses:

- source artifact family: `color`
- source file: the selected Color artifact `image.jpeg` or `image.png`
- source metadata: selected Color artifact object metadata
- job metadata: `renders/{job_id}/calc.json`

The produced PDF is:

- a first PDF page representing an open two-page spread, with optional appendix spreads
- size: `586 x 296 mm`
- left page: structured provenance report with large labeled palette swatch
- right page: the prepared source Color artifact image, cover-fit to one content page

V1 does not:

- support multi-page PDFs
- support arbitrary uploaded text
- support non-Color source families
- support DeepZoom from PDFs
- support PDF preview raster generation unless explicitly added

## UI Model

## Render Family Tabs

The Render family workspace becomes:

1. `Color`
2. `BiLevel`
3. `Coeffs`
4. `Palette`
5. `PDF`

The `PDF` family catalog behaves like the others:

- immutable artifacts
- scrollable list
- click row to select
- arrow up/down to move selection
- `Download`
- `Delete`

But `PDF` family does not support:

- `DeepZoom`
- `Populate`
- `Autolevels`
- `RePalette`
- `GenerateFromPalette`

## Primary Action

For `PDF` family, the primary action is:

- `ColorSpread`

Do not show a generic `Generate` button beside it and do not require the user to guess that `Generate` means “make a PDF”.

V1 `PDF` family action row should be:

- `ColorSpread`
- `Download`
- `Delete`

## ColorSpread Popup

Clicking `ColorSpread` opens a popup modeled after `GenerateFromPalette`:

- modal overlay
- title: `ColorSpread`
- filter input
- scrollable table of candidate `Color` artifacts
- `Exit`
- `Execute`

The popup should list saved `Color` artifacts from the current `render-summary` response.

Recommended table columns:

- `Created`
- `Dims`
- `Summary`

Selection rules:

- single selection only
- click row to select
- arrow up/down moves selection
- `Enter` executes
- `Esc` closes without side effects

Default selection:

- if a Color artifact is currently selected in the `Color` family, preselect that one
- otherwise preselect the first row

Popup summary line:

- show the source artifact id
- show the source render summary
- show the PDF subtype: `ColorSpread`

Example:

- `Source: color_repalette_1775abcd • solve:clusteriness q=5.0% w=1 inferno • output: PDF ColorSpread`

## PDF Viewer

The right-side viewer for `PDF` family must not use `<img>`.

Use an embedded PDF viewer:

- preferred: `<iframe src="...#toolbar=0&navpanes=0&view=FitH">`
- fallback: `<embed type="application/pdf">`
- final fallback: centered message plus download link

Viewer behavior:

- fit within the existing viewer pane
- black viewer background
- no auto-download on selection
- preserve the same stable panel height as the other families

Catalog rows still show metadata summary text, not thumbnails.

## Artifact Storage Contract

Add a new immutable family prefix:

- `renders/{job_id}/pdf/{artifact_id}/`

V1 stored files:

- `renders/{job_id}/pdf/{artifact_id}/document.pdf`

Optional later:

- `renders/{job_id}/pdf/{artifact_id}/preview.png`

V1 should work without `preview.png`.

The PDF object itself should carry S3 user metadata, matching the Color/BiLevel/Coeffs model rather than a separate `meta.json`.

Required object metadata on `document.pdf`:

- `family=pdf`
- `artifact_id`
- `created_at`
- `format=pdf`
- `pdf_kind=color_spread`
- `source_family=color`
- `source_artifact_id`
- `source_image_key`
- `source_display_name`
- `source_color_mode`
- `source_palette`
- `source_solve_metric`
- `source_solve_score_quantile`
- `source_solve_score_omega`
- `source_root_transforms`
- `page_count=1`
- `width_mm=586`
- `height_mm=296`

Recommended mirrored calc metadata:

- `function`
- `degree`
- `N`
- `times`

These should be copied from `calc.json` so the PDF artifact stays useful even if UI-side state changes later.

## Text Derivation

The left-page text should be generated from metadata, not lorem ipsum.

V1 text source:

- source Color artifact object metadata
- `renders/{job_id}/calc.json`

V1 formatting rule:

- title uses compute/artifact ids
- body is structured metadata plus a large labeled palette swatch; source details continue in appendix spreads
- filename line is the source image stem

Recommended title:

- use `function` when present
- include degree on the next logical phrase if it fits

Recommended body content, in this order:

- `job_id`
- `artifact_id`
- `function`
- `degree`
- `N`
- `times`
- `color_mode`
- `palette`
- `solve_metric`
- `q`
- `omega`
- `root_transforms`
- `created_at`

Do not dump raw JSON. Build a compact human-readable paragraph or short line sequence first, then let the same wrap logic center it.

## PDF Builder Refactor

[make_spread.py](/Users/nicknassuphis/karpo_hackathon/polypaint/make_spread.py) is currently a local CLI helper. That is not directly deployable as-is.

Refactor it into:

1. a reusable shared module
2. a thin CLI wrapper

Recommended structure:

- new shared module: [spread_pdf.py](/Users/nicknassuphis/karpo_hackathon/polypaint/spread_pdf.py)
- keep [make_spread.py](/Users/nicknassuphis/karpo_hackathon/polypaint/make_spread.py) as a CLI wrapper that imports the shared module

The current shared module exposes:

- `prepare_pdf_image(input_path, output_path, *, max_px, quality=90, image_format=None)`
- `build_color_spread_pdf(image_path, output_pdf_path, title, body=None, filename=None, meta=None, palette_image_path=None, report=None)`

The current implementation still reuses:

- content-page size constants from [make_book.py](/Users/nicknassuphis/karpo_hackathon/polypaint/make_book.py)
- `_draw_text_page(...)`
- `_draw_image_cover(...)`
- `_load_image_rgb(...)`

This avoids duplicating the book look.

## Backend Implementation

## New Lambda

Add a direct async derivation Lambda:

- [lambda/handler_pdf_artifact.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_pdf_artifact.py)

This should follow the same deployment style as:

- [lambda/handler_autolevels.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_autolevels.py)
- [lambda/handler_color_repalette.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_color_repalette.py)

It does not need Step Functions in V1.

Expected request payload:

- `job_id`
- `task_id`
- `artifact_id`
- `source_artifact_id`
- `source_image_key`

Optional request payload:

- `source_display_name`

Handler flow:

1. parse request
2. validate source object exists
3. verify source artifact is `family=color`
4. download the full source image to `/tmp`
5. load `renders/{job_id}/calc.json`
6. derive title/body text
7. call shared spread builder to create `/tmp/document.pdf`
8. upload `document.pdf` to the new PDF artifact prefix with metadata
9. report done with:
   - `family=pdf`
   - `artifact_id`
   - `image_key=document.pdf`

Suggested status phases:

- `started` / `ColorSpread`
- `load_source` / `Load source`
- `compose_pdf` / `Compose PDF`
- `upload` / `Upload PDF`
- `done` / `Done`

The returned result payload should match the existing render observer expectations:

- `job_id`
- `artifact_id`
- `family=pdf`
- `image_key`
- `format=pdf`

## Dispatch Wiring

Add a new dispatch target in [lambda/handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py):

- `pdf_artifact`

Environment variable:

- `PDF_ARTIFACT_FUNCTION`

Frontend should dispatch exactly one job for V1.

## Dependency Packaging

Current Lambda packaging does not include `reportlab` or `Pillow`.

The spec must therefore include a Python PDF dependency package.

Recommended approach:

- add a new Lambda layer containing:
  - `reportlab`
  - `Pillow`

This is cleaner than copying a large Python site-packages tree into one function zip.

Deploy work:

- new build script for a Python PDF layer
- publish the layer in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- attach that layer to the new PDF Lambda

Do not rely on local `uv` or repo venv contents existing in Lambda.

## Storage / Render Summary Changes

Add `pdf` to [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py):

- `RENDER_FAMILY_DIRS["pdf"] = "pdf"`
- `RENDER_FAMILY_SHAPES["pdf"] = { "image_candidates": ["document.pdf"], "preview_candidates": [] ... }`

Update `_render_artifact_entry(...)` to parse PDF-specific metadata:

- `pdf_kind`
- `source_family`
- `source_artifact_id`
- `source_display_name`
- `page_count`

Update `handle_render_summary(...)` so:

- `families.pdf` is returned
- ordering is newest-first

Delete semantics:

- `POST /delete-render-artifact` should work unchanged once `pdf` is in the family maps

There is no `legacy_pdf`.

## Frontend Changes

## Family State

Add `pdf` everywhere the family arrays are hard-coded in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

- family labels
- selected-artifact state
- family tab buttons
- catalog scroll restoration
- render-summary family normalization

Affected areas include:

- `_renderFamilyLabel(...)`
- `_renderArtifacts`
- `_renderSelectedArtifact`
- family tab rendering in `renderArtifactPanel(...)`
- family loops currently enumerating only `color`, `bilevel`, `coeffs`, `palette`

## PDF Summary Text

Extend `_renderArtifactSummary(...)` to produce a readable PDF row summary.

Recommended format:

- `ColorSpread · ${source_display_name || source_artifact_id}`

If source solve-score metadata is mirrored onto the PDF object, optionally include:

- `solve:${metric} q=... w=... ${palette}`

## PDF Viewer Branch

In `renderArtifactPanel(...)`, replace the current unconditional `<img>` viewer logic with:

- image viewer for image families
- PDF iframe/embed viewer for `pdf`

A simple rule is:

- if `art.format === "pdf"` or `art.content_type === "application/pdf"`, render the PDF viewer branch

## PDF Action Buttons

When active family is `pdf`:

- show `ColorSpread`
- show `Download`
- show `Delete`
- hide or disable:
  - `DeepZoom`
  - `Populate`
  - `Autolevels`
  - `RePalette`
  - `GenerateFromPalette`

## ColorSpread Popup

Add a new popup block in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html), parallel to:

- `GenerateFromPalette`
- `RePalette`
- `Color RePalette`

Suggested ids:

- `pdf-colorspread-popup-overlay`
- `pdf-colorspread-popup-title`
- `pdf-colorspread-popup-summary`
- `pdf-colorspread-popup-filter`
- `pdf-colorspread-popup-table`
- `pdf-colorspread-popup-run`

New JS state:

- `_pdfColorSpreadPopupState = { open: false, filter: '', highlightIdx: 0, sourceArtifactId: '' }`

New helper functions:

- `_visibleColorSpreadSourceCatalog()`
- `_closePdfColorSpreadPopup()`
- `_renderPdfColorSpreadPopup()`
- `openPdfColorSpreadPopup()`
- `_applyPdfColorSpreadFilter(text)`
- `runPdfColorSpreadSelected()`
- `_initPdfColorSpreadPopup()`

Candidate source list:

- use `families.color`
- filter out entries with missing `image_key`

Dispatch should log to `render-log` like:

- `PDF ColorSpread: dispatching color_... -> pdf_...`

Completion handling:

- active render run mode should be `pdf`
- observer completion should refresh Render artifacts
- auto-select the new `pdf` artifact in `families.pdf`

## Artifact Layout Example

Example prefix:

- `renders/compute_abcd1234/pdf/pdf_1776000000_abc123/`

Files:

- `document.pdf`

Object metadata on `document.pdf`:

- `family=pdf`
- `artifact_id=pdf_1776000000_abc123`
- `pdf_kind=color_spread`
- `source_family=color`
- `source_artifact_id=color_run_1775...`
- `source_image_key=renders/compute_abcd1234/color/color_run_1775.../image.jpeg`

## Download Behavior

Existing [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) `downloadSelectedRenderArtifact()` should continue to work once:

- `art.image_url` points to `document.pdf`
- `_renderArtifactFilename(...)` uses the `pdf` extension

No PDF-specific download route is needed.

## Suggested Text Template

V1 should use deterministic generated text.

Recommended title:

- `${function} — degree ${degree}`

Recommended body lines before wrapping:

- `Job ${job_id}`
- `Artifact ${source_artifact_id}`
- `N ${N} · times ${times}`
- `Mode ${color_mode}`
- `Palette ${palette}`
- `Metric ${solve_metric}`
- `q ${q_percent}% · omega ${omega}`
- `Transforms ${rt_summary}`
- `Created ${created_at}`

Then join those into a single body string for `_draw_text_page(...)`.

## Tests

Add backend tests:

- [tests/test_pdf_artifact_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pdf_artifact_handler.py)

This should verify:

- source must be `family=color`
- handler downloads the source image
- calc metadata is loaded
- PDF upload key is `renders/{job_id}/pdf/{artifact_id}/document.pdf`
- metadata fields are present
- result payload returns `family=pdf`

Add storage summary tests:

- extend [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)

Verify:

- `families.pdf` is returned
- `document.pdf` is recognized
- ordering works newest-first
- delete route removes PDF prefix only

Add deploy packaging tests:

- extend [tests/test_deploy_packaging.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deploy_packaging.py)

Verify:

- PDF Lambda zip includes:
  - handler
  - shared spread builder module
- deploy attaches the Python PDF layer

Add frontend harness tests:

- extend [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

Verify:

- `PDF` family tab appears
- `ColorSpread` button is present only for `PDF`
- popup lists Color artifacts
- popup filtering works
- dispatch payload includes:
  - `job_id`
  - `artifact_id`
  - `source_artifact_id`
  - `source_image_key`
- PDF family selection uses iframe/embed viewer branch
- `Download` remains enabled for selected PDF artifacts

Optional browser e2e:

- add one Playwright case covering:
  - switch to `PDF`
  - open `ColorSpread`
  - choose a Color source
  - execute
  - observer refresh selects the new PDF row

## Files To Modify

Shared PDF builder:

- [make_spread.py](/Users/nicknassuphis/karpo_hackathon/polypaint/make_spread.py)
- new [spread_pdf.py](/Users/nicknassuphis/karpo_hackathon/polypaint/spread_pdf.py)

Frontend:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Storage / summary:

- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)

Dispatch / new Lambda:

- [lambda/handler_dispatch.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_dispatch.py)
- new [lambda/handler_pdf_artifact.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_pdf_artifact.py)

Deploy:

- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- new layer build script for Python PDF dependencies

Tests:

- new [tests/test_pdf_artifact_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pdf_artifact_handler.py)
- [tests/test_pipeline.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_pipeline.py)
- [tests/test_deploy_packaging.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deploy_packaging.py)
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

## Recommended V1 Decisions

- make `PDF` a fifth immutable Render family
- use `ColorSpread` as the family-specific primary action
- use a direct async Lambda, not Step Functions
- refactor [make_spread.py](/Users/nicknassuphis/karpo_hackathon/polypaint/make_spread.py) into a shared builder
- store only `document.pdf` in V1
- use PDF embed viewing in the Render pane
- derive text entirely from artifact metadata + `calc.json`

That gives a minimal but coherent first PDF artifact flow without inventing a full book pipeline yet.
