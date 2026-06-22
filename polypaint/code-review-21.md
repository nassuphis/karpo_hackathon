# Code Review 21 - PDF Artifact Generation

## Verdict

The Render-tab PDF artifact path is old and fragile, but the stuck job is not an Rnw/LaTeX failure. The deployed Render PDF path is:

`index.html` PDF ColorSpread popup -> `js/05-render-popups.js` -> `lambda/handler_dispatch.py` -> `lambda/handler_pdf_artifact.py` -> `lambda/spread_pdf.py`

That path uses Pillow and ReportLab. The Rnw/LaTeX path exists separately in `scripts/compile_knitr.sh` for local report documents such as `architecture-ui-critique-2.Rnw`; it is not invoked by the Render PDF artifact button.

The likely failure mode for the stuck `Compose PDF` job is full-resolution raster handling. A 10k x 10k image is decoded and embedded at full size even though the PDF only displays it at roughly one page. `pdf_artifact` is configured with 2048 MB memory, so a 300 MB RGB decode plus ReportLab/PDF overhead makes OOM genuinely plausible, not just slow execution. When that happens inside a single Lambda invocation, no terminal error row is guaranteed, so the frontend can keep seeing the last `compose_pdf` row and treat the render pipeline as still active.

## Findings

### 1. Blocker: `spread_pdf.py` embeds full-resolution images

`lambda/spread_pdf.py` opens the source image, converts it to RGB, and passes the full decoded raster to ReportLab. For a 10,000 x 10,000 image, RGB decode alone is about 300 MB before ReportLab/PDF encoding overhead.

The PDF draw size is about a page, not 10k pixels. At 300 DPI the useful embedded image size is roughly 3400-3500 px on the long edge. Anything beyond that increases memory, runtime, and PDF size without visible benefit.

Required fix: downsample before ReportLab sees the image. The main image should be prepared into a temporary JPEG/PNG capped by a target DPI-derived max dimension, probably defaulting to about `3600 px`. The palette/reference image can be capped much smaller, roughly `800 px`.

Also handle Pillow's decompression-bomb guard deliberately. Images above `PIL.Image.MAX_IMAGE_PIXELS` can raise `DecompressionBombError` before the code gets to resize. The PDF path should either set a safe explicit limit for this job shape or catch that error and return a clear input-size diagnostic.

### 2. Blocker: timeout/OOM can leave the UI stuck in `compose_pdf`

`handler_pdf_artifact.py` writes a `processing / compose_pdf` phase immediately before calling `build_color_spread_pdf(...)`. If the process hangs, times out, or is killed by Lambda, the Python exception handler may never write an error row.

The frontend has stale-run messaging for active render operations, but this path is a single blocking Lambda call rather than a worker loop that can keep emitting progress. The hard-stale branch explicitly does not auto-clear the active run. If no terminal row arrives, the local active render lock can remain in place and block other render operations until the user manually clears state or deletes the task.

Required fix: reduce the chance of timeout by downsampling, and add a frontend escape hatch for stale single-shot derived jobs. After hard stale, PDF/deepzoom-style single-action runs should offer `Abandon stalled run` and clear the local active-run lock. Do not globally auto-clear long compute workflows without mode awareness.

### 3. High: no image preflight or resource limit

The handler downloads the source artifact and immediately enters PDF composition. It should inspect image dimensions before full decode, record original dimensions, compute the target embedded dimensions, and report a `prepare_image` phase.

The output metadata/status should include:

- `source_width`
- `source_height`
- `prepared_width`
- `prepared_height`
- `image_resized`
- `image_max_px`

This makes failures diagnosable and prevents the current "Compose PDF, stale/no update" blind spot.

### 4. High: tests mock the builder and miss the real failure mode

`tests/test_pdf_artifact_handler.py` mocks `build_color_spread_pdf`, so it verifies status plumbing only. It does not exercise `spread_pdf.py`, image preparation, downsampling, large-image handling, or actual PDF creation.

Required tests:

- A `spread_pdf` unit test that verifies a large input is downsampled before embedding.
- A handler test that verifies `prepare_image`/dimension metadata is reported.
- A frontend test that verifies a stale PDF run can be abandoned and does not permanently block render actions.
- A packaging test update if image-preparation code moves into a new helper file.

### 5. Medium: docs and UI naming are stale

The live entry point is still the PDF ColorSpread popup; it is not dead code and must not be removed as part of this fix. The stale-docs problem is narrower: `pdf_artifact.md` and some UI text need to match the current user flow and the current ReportLab/Pillow backend.

Required fix: update `pdf_artifact.md` to describe the live PDF ColorSpread popup and any Color-tab entry point that opens it. If the product decision is to move PDF generation fully into the Color tab later, that should be a separate UI refactor, not part of the resource-failure fix.

### 6. Medium: stale-status wording is misleading for this job shape

The stale branch currently uses worker-oriented language (`no worker update`) for a single Lambda PDF composition job. There is no worker loop during `build_color_spread_pdf`; it is one blocking call. The status should say something like:

`PDF compose has not updated for 5+ min`

That is more accurate and points at the correct failure surface.

### 7. Nits

- `handler_pdf_artifact.py` has dead or stale locals/imports such as `parse_boolish` and `pipeline_parts`.
- A `filename` value is computed but the builder call still uses artifact metadata for display; check whether that variable is obsolete.
- The deployment config gives `pdf_artifact` the default long Lambda timeout. That is acceptable only if downsampling is added; otherwise it just makes stuck UI state last longer.

### 8. Product gap: the text page is stale and underuses current provenance data

The current left page is a centered title plus a few short metadata lines. That matched the older render pipeline, but it no longer reflects what now makes up a generated image. The compute pipeline stores richer param/coeff program information in `calc.json`, and render artifacts store color/score/root-program provenance in artifact metadata.

The PDF should become a concise provenance report:

- The compute result set, e.g. `compute_xxxxx`.
- The selected color render artifact, e.g. `color_run_xxxxx`.
- The coefficient function name and, when available, a source snippet or catalog summary.
- Param program source as a code block.
- Coeff program source as a code block.
- Root program source and solve-score program source when the color render used them.
- Viewport, render mode, palette/color interpretation, degree, `N`, `times`, and solver settings.

This should be presented as a designed summary, not a dump. Use clear section headers, compact metadata rows, and monospace code blocks for programs. The goal is: "what exactly produced this image?" in a form that is aesthetically pleasing enough to sit beside the artwork.

## Rnw / LaTeX Path

`scripts/compile_knitr.sh` compiles `.Rnw`, `.Rtex`, and `.tex` files using `Rscript` and `latexmk`, writing outputs under `reports/pdf/`. That is a local document-generation path.

I found no Render PDF route that invokes:

- `compile_knitr.sh`
- `Rscript`
- `latexmk`
- `.Rnw`

So if the stuck job came from the Render Color-tab PDF action, it is not failing inside knitr or LaTeX. It is failing in the Lambda/Pillow/ReportLab path.

Large images can still be a LaTeX problem in general, but that is not the current Render artifact implementation.

## Implementation Plan

### 1. Add image preparation in `lambda/spread_pdf.py`

Add a helper that opens the image lazily, reads dimensions, downsamples, and writes a prepared temporary image:

```python
def prepare_pdf_image(input_path, output_path, *, max_px, quality=90):
    ...
    return {
        "source_width": source_w,
        "source_height": source_h,
        "prepared_width": prepared_w,
        "prepared_height": prepared_h,
        "resized": resized,
        "prepared_path": output_path,
    }
```

Implementation notes:

- Use `Image.open(...)` first to inspect dimensions.
- Use `draft("RGB", target_size)` for JPEGs when possible. This is a JPEG optimization only; PNG and other formats still need normal resize via `thumbnail`.
- Use `thumbnail(..., Image.Resampling.LANCZOS)` to cap the long edge.
- Convert to RGB after resizing where possible.
- Save the prepared main image as a lossless PNG in `/tmp` by default. This avoids visible JPEG artifacts on fractal art while still cutting memory by reducing pixel dimensions first. Add an explicit `PDF_IMAGE_FORMAT=jpeg` option later only if PDF file size becomes the real bottleneck.
- Do the same for the palette/reference image with a much smaller cap.
- Set or catch Pillow's decompression-bomb limit deliberately so oversized inputs fail with a clear diagnostic instead of crashing unpredictably before resize.

Integration point: `handler_pdf_artifact.py` should call `prepare_pdf_image` for both the main source image and the optional palette image. It should report the returned dimensions during `prepare_image`, pass the returned prepared paths into `build_color_spread_pdf`, and `build_color_spread_pdf` should stop doing full-resolution decode/downsample work itself.

### 2. Derive caps from print size

Default caps:

- Main image: `PDF_IMAGE_MAX_PX=3600`
- Palette/reference image: `PDF_PALETTE_MAX_PX=800`

The main cap should be derived from the draw box and target DPI when practical:

`ceil(max(draw_width_in, draw_height_in) * PDF_TARGET_DPI)`

Then clamp to the configured max.

### 3. Add status phases and metadata

In `handler_pdf_artifact.py`, add:

- `prepare_image` before calling the builder.
- `compose_pdf` only after prepared images are ready.
- `upload` after PDF exists.

Include dimension/downsample metadata in status rows and the final artifact metadata.

S3 metadata is string-only. Route numeric fields through the existing `_stringify_meta` helper before writing them to artifact metadata.

### 4. Add stale-run recovery in the frontend

For single-shot derived render actions such as PDF artifact generation, hard stale should not permanently block the render tab.

Implement a mode-aware action:

- Show `Abandon stalled PDF job` after hard stale.
- Clear the local active render run by calling the existing `_clearActiveRun()` helper, which also removes `localStorage['polypaint_active_render_run']`.
- Keep the server row intact for audit/debug.

Use the existing constants unless there is a deliberate UX reason to add a PDF-specific threshold:

- `RENDER_WARN_STALE_MS = 5 min` for warning text.
- `RENDER_HARD_STALE_MS = 15 min` for the abandon action.

Avoid silently clearing long-running multi-phase compute jobs.

### 5. Add tests

Add or update:

- `tests/test_spread_pdf.py`: real downsample/PDF smoke test.
- `tests/test_pdf_artifact_handler.py`: status phases and dimension metadata.
- frontend JS test: stale PDF run exposes an abandon action and clears the active lock.
- packaging test if new files are introduced.

### 6. Update docs

Update `pdf_artifact.md`:

- The PDF ColorSpread popup is the current live entry point.
- Any Color-tab PDF button that opens it should be documented as a launcher, not a separate backend path.
- Backend is Lambda + Pillow + ReportLab.
- Rnw/LaTeX is not part of Render PDF artifacts.
- Large images are downsampled before embedding.
- Stale single-shot PDF jobs can be abandoned locally.

### 7. Refresh the report content and layout

Replace the current `_build_spread_meta` + centered-line rendering with a structured report model. The handler should build a `report` dict from `calc.json` plus color artifact metadata, and `spread_pdf.py` should render that model.

Recommended report structure:

```python
report = {
    "title": title,
    "compute": {
        "job_id": job_id,
        "function": calc["pipeline"]["function"],
        "degree": calc["degree"],
        "n_coeffs": calc["n_coeffs"],
        "N": calc["N"],
        "times": calc["times"],
        "solver": calc["solver"],
    },
    "color_artifact": {
        "artifact_id": source_artifact_id,
        "color_mode": src_meta.get("color_mode"),
        "palette": src_meta.get("palette"),
        "color_interpretation": src_meta.get("color_interpretation"),
        "score_output_channel_count": src_meta.get("score_output_channel_count"),
        "viewport": {
            "view_mode": src_meta.get("view_mode"),
            "quantile": src_meta.get("quantile"),
            "shim": src_meta.get("shim"),
            "square_extent": src_meta.get("square_extent"),
        },
    },
    "programs": [
        {"label": "Param Program", "language": "poly-param", "source": "..."},
        {"label": "Coeff Program", "language": "poly-coeff", "source": "..."},
        {"label": "Root Program", "language": "poly-root", "source": "..."},
        {"label": "Solve Score Program", "language": "poly-score", "source": "..."},
        {"label": "Coefficient Function", "language": "python", "source": "..."},
    ],
}
```

Data sources:

- Compute result set: `job_id` and `renders/{job_id}/calc.json`.
- Color render shown: `source_artifact_id` and the selected artifact metadata.
- Param program: `calc["pipeline"]["param_program_source_text"]`; fallback to `param_program_display` or chain display if the source text is absent.
- Coeff program: `calc["pipeline"]["coeff_program_source_text"]`; fallback to `coeff_program_display` or chain display if the source text is absent.
- Coefficient function: `calc["pipeline"]["function"]` is always available. Source text is not always stored in metadata. Add a resolver that can return source for known Python/catalog functions when cheap and safe; otherwise show the function name plus catalog summary/degree instead of pretending source is available.
- Root program: `src_meta["root_program_source_text"]` when present.
- Solve-score program: `src_meta["solve_score_program_source_text"]` or `src_meta["score_source_text"]` when present.

Layout and format (concrete):

This is the visual contract. The page is 293 x 296 mm (about 831 x 839 pt); type and spacing are scaled for that size. Use only ReportLab built-in fonts -- Lambda has no Arial Unicode, and program source / artifact ids are ASCII, so `Helvetica` and `Courier` are safe. Right page: unchanged (full-bleed cover-fit image).

Palette (the Render-tab visual language -- dark navy + signal red, not pure black):

| token | hex | use |
| --- | --- | --- |
| `PAGE_BG` | `#1a1a2e` | page fill |
| `ACCENT` | `#e94560` | section headers, title rule, color-id line |
| `TEXT` | `#f2f2f7` | values, title |
| `MUTED` | `#9aa0b4` | labels, captions, truncation note |
| `PANEL_BG` | `#121829` | code-block inset fill |
| `PANEL_BORDER` | `#2b3a5e` | code-block border, header rule |
| `CODE_TEXT` | `#e6e9f2` | monospace code |

Geometry (origin bottom-left): lay the column inside the **trim**, not the gross page. The left page bleeds 3 mm on its outer edges, so use `CONTENT_NET = 290 mm` (not `PAGE_W = 293 mm`) and offset the origin past the bleed: column `x = BLEED_3 + MARGIN_L`. Margins L/R 24 mm, top 26 mm, bottom 22 mm (bottom always reserved). `CONTENT_W = CONTENT_NET - MARGIN_L - MARGIN_R = 242 mm`, single left-aligned column (stop centering every line). All vertical budgeting is off `CONTENT_NET`, not `PAGE_H`, so text never drifts into the bleed.

Type scale:

| element | font | size / leading | color |
| --- | --- | --- | --- |
| Title line 1 (`compute_xxxxx`) | Helvetica-Bold | 30 / 34 | TEXT |
| Title line 2 (`color_run_xxxxx`) | Helvetica-Bold | 19 / 24 | ACCENT |
| Section header (UPPERCASE, charSpace 1.0) | Helvetica-Bold | 15 | ACCENT |
| KV label (UPPERCASE) | Helvetica-Bold | 10 / 17 | MUTED |
| KV value | Helvetica | 12 / 17 | TEXT |
| Code | Courier | 10.5 / 13 | CODE_TEXT |
| Caption / truncation | Helvetica-Oblique | 9 / 12 | MUTED |

Components and rhythm:

- Title: two stacked lines (never wrap one long id onto two), then a 0.8 pt ACCENT rule across `CONTENT_W`; 9 mm gap below.
- KV grid: stacked rows, label column 46 mm + value, row pitch 17 pt. Order: Function, Degree, N, times, Solver, Color mode, Interpretation, Palette, Output channels, Viewport. Always rendered in full; 11 mm gap below.
- Section header: uppercase ACCENT + a 0.5 pt PANEL_BORDER rule under it; 4 mm to its body; 9 mm between sections.
- Code block: `roundRect` radius 2 mm, `PANEL_BG` fill + 0.75 pt PANEL_BORDER, padding 6 mm horizontal / 5 mm vertical. Courier 10.5 pt gives about 103 chars/line (inner about 230 mm; Courier advance = 0.6 x size). Wrap continuations with a 2-space hanging indent; preserve indentation; never shrink below 9 pt.
- Absent program: a single MUTED caption (e.g. `-- no root program --`). For a fallback, label it (`chain display (source text not stored)`) before the text. Never dump raw JSON.

Pagination (the non-ugliness contract -- do not cram everything onto one page):

The summary page cannot hold 4-5 full program sources. Title + a ~10-row KV grid leaves only ~31 code lines on the whole page, and per-section header/padding/gap overhead (~82 pt each) consumes nearly all of it -- so cramming all programs onto the left page is exactly the overcrowding to avoid. Split across pages instead:

1. **Spread 1, left page = summary.** Title block + the full KV grid + one *short excerpt* per present program: the first `EXCERPT_LINES = 6` lines in a code panel, with a caption `... full source on appendix p.N` when the source is longer. This budget is fixed (the KV grid is never truncated; excerpts are 6 lines each), so the summary page can never overcrowd. Right page = full image (unchanged).
2. **Appendix spreads = full source.** Render each present program's complete source under its section header, flowing in a two-column layout across the spread (left half, then right half) and onto additional appendix spreads as needed (`c.showPage()` per spread). Number the appendix spreads and back-reference them from the summary excerpts.
3. Appendix code paginates freely -- no per-section truncation. Apply only a sane global guard (`CODE_MAX_LINES_TOTAL`, e.g. 2000) so a pathological input cannot generate hundreds of pages; if hit, stop with a labeled caption. Wrap/indent rules match the summary panels.

Constants (drop into `spread_pdf.py`):

```python
from reportlab.lib.colors import HexColor
from reportlab.lib.units import mm

PAGE_BG = HexColor('#1a1a2e'); ACCENT = HexColor('#e94560'); TEXT = HexColor('#f2f2f7')
MUTED = HexColor('#9aa0b4'); PANEL_BG = HexColor('#121829'); PANEL_BORDER = HexColor('#2b3a5e')
CODE_TEXT = HexColor('#e6e9f2')

MARGIN_L = MARGIN_R = 24 * mm
MARGIN_TOP = 26 * mm
MARGIN_BOTTOM = 22 * mm
# Lay out inside the TRIM (CONTENT_NET), offset past the 3 mm bleed -- not the gross page.
ORIGIN_X = BLEED_3 + MARGIN_L
CONTENT_W = CONTENT_NET - MARGIN_L - MARGIN_R          # 242 mm (not PAGE_W - margins)
CONTENT_H = CONTENT_NET - MARGIN_TOP - MARGIN_BOTTOM   # vertical budget off CONTENT_NET, not PAGE_H

F_TITLE = ('Helvetica-Bold', 30); F_TITLE2 = ('Helvetica-Bold', 19); F_H = ('Helvetica-Bold', 15)
F_LABEL = ('Helvetica-Bold', 10); F_VALUE = ('Helvetica', 12)
F_CODE = ('Courier', 10.5); F_CAP = ('Helvetica-Oblique', 9)

KV_PITCH = 17; CODE_LEADING = 13
EXCERPT_LINES = 6              # per-program teaser lines on the summary page
CODE_MAX_LINES_TOTAL = 2000   # global guard across appendix pages
SECTION_GAP = 9 * mm; CODE_PAD_X = 6 * mm; CODE_PAD_Y = 5 * mm
TITLE_RULE_W = 0.8; HEADER_RULE_W = 0.5; PANEL_BORDER_W = 0.75
```

ReportLab notes: colors via `HexColor`; code panel via `c.roundRect(x, y, w, h, 2*mm, fill=1, stroke=1)`; there is no auto-wrap, so wrap manually with `chars = floor((CONTENT_W - 2*CODE_PAD_X) / (0.6 * 10.5))` (about 103). Header letter-spacing: `c.setCharSpace(1.0)` then reset to 0 -- and if that fights surrounding text state, use a `beginText()` text object (`t.setCharSpace(...)` / `t.textLine(...)`) instead of `drawString`. **Sanitize before drawing:** built-in Helvetica/Courier are Latin-1 only and choke on unexpected Unicode (e.g. a coefficient-function source with non-ASCII), so coerce code and labels to a safe set (`s.encode('latin-1', 'replace').decode('latin-1')`) or substitute a marker. Do not depend on Arial Unicode in Lambda; render with Helvetica.

Implementation notes:

- Add `build_pdf_report_model(job_id, calc, src_meta, source_artifact_id)` in `handler_pdf_artifact.py` or a small helper module.
- Extend `build_color_spread_pdf(..., report=None)` so the old `meta` path can be retired after tests are updated.
- Add small drawing helpers in `spread_pdf.py`: `_draw_section_header`, `_draw_kv_grid`, `_draw_code_block`, `_wrap_monospace_line`.
- Use `Courier` for code snippets; keep the existing Unicode-capable body fallback for human labels.
- Include report content in the PDF artifact metadata only as compact summary fields, not full code blobs. Full code should come from `calc.json` / artifact metadata or be rendered into the PDF only.
- Add tests that assert param/coeff source text and artifact IDs appear in the generated PDF text stream. Use a small image so the test stays fast.

### 8. Cleanup

Remove dead imports/locals in `handler_pdf_artifact.py`. Do not remove the PDF ColorSpread popup unless the frontend is first changed to provide an equivalent live entry point.

## Immediate Mitigation

For the currently stuck browser session, the client-side active-run lock can be cleared locally. That does not cancel the Lambda/SFN execution, but if the Lambda has already died or timed out, it unblocks the UI.

The real fix is still backend downsampling plus stale-run recovery; otherwise the next 10k image can reproduce the same stuck state.
