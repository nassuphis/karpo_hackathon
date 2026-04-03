## Palette Debug Follow-Ups

These are the remaining cleanup items after the latest `palette_debug` implementation review.

### 1. Fix Button Enablement To Check Real Lores Availability

Current problem:
- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1816) sets `_lastCalcHasLores = !!(calc.exists && calc.degree)`.
- That is too weak for Palette / Histogram debug.
- The actual actions require lores metadata such as:
  - `calc.lores.N`
  - `calc.lores.bin_key`

Required change:
- Change the enablement predicate so Palette / Histogram only enable when the current calc actually has the lores inputs needed to run.
- Do not use `degree` as a proxy for lores availability.

Suggested rule:
- `_lastCalcHasLores = !!(calc.exists && calc.lores && calc.lores.N && calc.lores.bin_key)`

### 2. Reset Solve-Score Debug Button State When Job ID Changes

Current problem:
- `_lastCalcHasLores` is only updated from [renderArtifactPanel()](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1795).
- If the user edits [render-results-dir](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L292) manually, the previous calc-derived state can remain in effect.
- That means Palette / Histogram can stay enabled for a different job until the next artifact refresh.

Required change:
- Add an `input` or `change` listener on `render-results-dir`.
- When the job id changes manually:
  - clear or pessimistically reset `_lastCalcHasLores`
  - call `_updateSolveScoreButtons()`

Required behavior:
- Manual job-id changes must never leave Palette / Histogram enabled based on stale calc state from a previous job.

### 3. Add A Real Test For Palette Preview Generation

Current problem:
- Coverage now checks:
  - Palette button exists
  - Palette artifact row exists
  - `Download` and `DeepZoom` buttons exist
  - Palette preview toggle appears
- But there is still no test that actually drives on-demand `preview_palette.png` generation.

Required test:
- Add one frontend test that:
  - renders a summary with `palette_jpeg.exists = true`
  - sets `preview_palette_png.exists = false`
  - triggers Palette preview
  - verifies the preview-generation call uses:
    - `source_key = renders/{job_id}/image_palette.jpeg`
    - `preview_key = renders/{job_id}/preview_palette.png`

Also verify:
- it does not reuse `preview_color.png`
- it does not use the wrong source artifact

### Done Criteria

This mini-follow-up is complete only when:
- Palette / Histogram enablement requires real lores metadata
- manual job-id edits reset stale button state
- there is a real automated test for on-demand Palette preview generation
