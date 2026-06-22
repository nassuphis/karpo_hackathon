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

### 7. Cleanup

Remove dead imports/locals in `handler_pdf_artifact.py`. Do not remove the PDF ColorSpread popup unless the frontend is first changed to provide an equivalent live entry point.

## Immediate Mitigation

For the currently stuck browser session, the client-side active-run lock can be cleared locally. That does not cancel the Lambda/SFN execution, but if the Lambda has already died or timed out, it unblocks the UI.

The real fix is still backend downsampling plus stale-run recovery; otherwise the next 10k image can reproduce the same stuck state.
