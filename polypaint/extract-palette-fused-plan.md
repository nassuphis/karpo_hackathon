# Fused extract-palette via cached step scores

Status: not implemented.

## Why

Extract-palette on a fused color artifact today runs `palette_orchestrator`
→ `palette_workflow.asl.json.template`, which executes the full classic
palette pipeline (clip + hist map + hist merge + palette chunk fan-out +
palette finalize). It takes tens of seconds and uses the legacy 10-bin
`pixel_bins_render` + `raw2jpeg` path, so the produced palette can look
different from what a fused render with `save_associated_palette=true`
would have produced.

It shouldn't need any of that. The palette is just a colorization of
per-step score bytes that the raster already computed. If we cache those
bytes during the original render, extract-palette becomes a stream
transformation + one libvips encode pass.

## What has to be done

### 1. Emit `step_scores.raw` during fused renders

**`lambda/roots2pix_mt.c`**

- Add a per-thread byte buffer `stepScoresBuf` of size `stepsPerThread`.
- In the per-step main loop, after `solveBin` is computed, write it into
  the buffer at the thread's local step index.
- This write must happen exactly once per solve step, before any root
  viewport clipping, pixel dedup, or per-root emission logic. Do not
  gate it on whether any root survives for the main image. This is the
  same contract that fixed the fused associated-palette filament bug.
- Do not drop `solveBin == 0`. Zero is still a valid stored byte for the
  cached step-score stream and must preserve position.
- Add CLI flag `--step_scores_output=<path>`. Before exit, concatenate
  all threads' buffers in global step order and write the file.

**`lambda/handler_raster_mt.py`**

- Pass `--step_scores_output=/tmp/step_scores.bin` to the raster binary.
- Upload one file per raster section to
  `{fragment_prefix}{section_idx:04d}_step_scores.raw`.

**`lambda/handler_finalize_mt.py`**

- Download all per-section step_scores files in section order.
- Concatenate into a single `step_scores.raw`.
- Upload to
  `renders/{job_id}/color/{artifact_id}/step_scores.raw`.
- Thread `step_scores_key`, `step_count`, and `step_scores_grid_n` into
  `build_raw_sidecar`.
- Also surface those same fields into the color artifact overlay /
  inventory metadata. The frontend dispatch branch must be able to route
  from the color artifact summary alone, without rereading
  `greyscale.meta.json`.

### 2. Sidecar schema gets three new fields

**`lambda/raw_sidecar.py`**

- Bump `RAW_SIDECAR_VERSION = 3`.
- Accept v1, v2, and v3 in `validate_raw_sidecar`.
- On v3, require:
  - `step_scores_key: string`
  - `step_count: int > 0`
  - `step_scores_grid_n: int > 0`
- On v1/v2, these fields are absent; consumers must treat that as
  "fused-extract path not available."

`step_count` alone is not enough to unfold the pass-0 palette image.
The consumer also needs the fused palette grid size `N` that was used
when the original render computed the associated-palette mapping.

### 3. Tiny C binary that unfolds step scores → palette raw

**`lambda/step_scores_to_palette_raw.c`** (new, ~50 lines)

```c
int main(int argc, char **argv) {
    const char *in_path  = argv[1];
    const char *out_path = argv[2];
    long long N          = atoll(argv[3]);
    long long step_count = atoll(argv[4]);

    long long palette_pixels = N * N;
    if (step_count < palette_pixels) {
        fprintf(stderr, "step_scores shorter than pass0 palette grid\\n");
        return 1;
    }
    FILE *fin  = fopen(in_path, "rb");
    uint8_t *step_scores = malloc(step_count);
    fread(step_scores, 1, step_count, fin);
    fclose(fin);

    uint8_t *palette = calloc(palette_pixels, 1);
    for (long long s = 0; s < palette_pixels; s++) {
        uint8_t byte = step_scores[s];
        long long row = s / N;
        long long j   = s % N;
        long long col = (row & 1) ? (N - 1 - j) : j;
        palette[row * N + col] = byte;
    }

    FILE *fout = fopen(out_path, "wb");
    fwrite(palette, 1, palette_pixels, fout);
    fclose(fout);
    return 0;
}
```

No threading, no libraries, no libvips. One pass over memory.

This binary reconstructs the pass-0 `N x N` serpentine palette grid
only. If the source render had `times > 1`, later passes are
intentionally ignored, matching the current associated-palette
semantics.

### 4. New handler for fused extract-palette

**`lambda/handler_extract_palette_from_step_scores.py`** (new, ~150
lines)

Flow:

1. Parse payload: `job_id`, `source_artifact_id`, `new_palette`.
2. Load source artifact head → source metadata.
3. Load source `greyscale.meta.json` via `validate_raw_sidecar`.
   Require v3 with non-empty `step_scores_key`, `step_count > 0`, and
   `step_scores_grid_n > 0`.
4. Download `step_scores.raw` to `/tmp/extract_step_scores.raw`.
5. Subprocess to `step_scores_to_palette_raw` →
   `/tmp/palette.greyscale.raw`.
6. Compute 256-bin histogram of the palette raw
   (`histogram_from_raw_path`).
7. Build equalization LUT (`write_equalization_lut`).
8. Call `render_score_raw` with the new palette →
   `palette.jpeg` + `palette_preview.png`.
9. Allocate `palette_id = pal_{source_artifact_id}`. This is the same
   deterministic associated-palette ID contract used by fused
   `save_associated_palette=true`; it refreshes / attaches the source
   color artifact's associated palette rather than creating an
   unbounded palette-variant history for ExtractPalette. Upload:
   - `renders/{job_id}/palettes/{palette_id}/image.jpeg`
   - `renders/{job_id}/palettes/{palette_id}/preview.png`
   - `renders/{job_id}/palettes/{palette_id}/greyscale.raw`
   - `renders/{job_id}/palettes/{palette_id}/greyscale.meta.json`
     (v3 sidecar, `artifact_family="palette"`, histogram cached,
     inherits `chain_fingerprint`, `score_chain`, `score_program`,
     `clip_slots`, `plan_params_digest` from source)
   - `renders/{job_id}/palettes/{palette_id}/meta.json`
10. Patch source color artifact meta with `associated_palette_*`
    fields (same pattern as
    `color_recolor_raw._apply_associated_palette_metadata`).
11. Return palette artifact info.

Imports: `raw_score_render`, `raw_sidecar`, `color_artifact_meta`,
`shared`. No delegation to any classic handler.

### 5. Dispatch wiring

**`lambda/handler_dispatch.py`**

- Add `"extract_palette_fused": os.environ.get("EXTRACT_PALETTE_FUSED_FUNCTION", "polypaint-extract-palette-fused")`.

**`deploy.sh`**

- Package `handler_extract_palette_from_step_scores.py`,
  `step_scores_to_palette_raw` binary, `score_raw_render` binary,
  `raw_score_render.py`, `raw_sidecar.py`, shared modules.
- Create Lambda `polypaint-extract-palette-fused`.

### 6. UI dispatch

**`index.html::runExtractPaletteArtifact`**

Branch on the source artifact's surfaced metadata:

- If the artifact exposes `step_scores_key` and `step_scores_grid_n`
  (v3 sidecar, surfaced in overlay/inventory) → dispatch to target
  `extract_palette_fused` with a small payload (`job_id`,
  `source_artifact_id`, `new_palette`).
- Otherwise (v1/v2 sidecar, or no sidecar at all) → existing dispatch to
  `palette_orchestrator`.

Detection must not depend on the frontend rereading `greyscale.meta.json`
from S3. The color artifact's `meta.json` overlay / inventory row should
surface `step_scores_key`, `step_scores_grid_n`, and `step_count` when
the underlying sidecar is v3. If it doesn't, add them when the overlay
is written by FinalizeMT.

New fused renders get the fast path. Existing v1/v2 artifacts continue
to route to `palette_orchestrator` until they are rerendered.

### 7. Tests

- `tests/test_raster_mt.py` — assert step_scores fragment is emitted
  with the expected size and byte pattern for a deterministic fixture,
  and that capture is not suppressed by root clipping or main-image
  dedup. Pin this with two explicit fixtures:
  - some steps whose roots all land outside the viewport → the
    corresponding `step_scores.raw[step_idx]` bytes still equal the
    computed `solveBin`, not zero
  - some steps whose roots collide on the same main-image pixel →
    every `step_scores.raw[step_idx]` still equals its own computed
    `solveBin`, not just the dedup winner
- `tests/test_finalize_mt_handler.py` — assert per-section step_scores
  are concatenated, uploaded to the color artifact prefix, and the v3
  sidecar plus color overlay carry `step_scores_key`, `step_count`, and
  `step_scores_grid_n`.
- `tests/test_raw_sidecar.py` — v1/v2/v3 round-trip validation.
- `tests/test_extract_palette_from_step_scores.py` (new):
  - Source with v3 sidecar + step_scores.raw → extract-palette produces
    the same palette raw bytes as a fresh fused render's
    `save_associated_palette=true` output, and uses the same
    `render_score_raw` path for final image generation.
  - Source with v2 sidecar → handler raises cleanly (caller should have
    routed to classic).
  - Only one subprocess call to `step_scores_to_palette_raw`, one to
    `score_raw_render`. No solve-read binaries. No fan-out.
- `tests/test_frontend_js.sh` — extract-palette dispatches to
  `extract_palette_fused` when the artifact advertises
  `step_scores_key` + `step_scores_grid_n`, else to
  `palette_orchestrator`.
- `tests/test_deploy_packaging.py` — new handler + binary are packaged,
  new Lambda ARN is created.

## What this does NOT change

- Classic `palette_workflow.asl.json.template`,
  `handler_palette_orchestrator.py`, `handler_palette_chunk.py`,
  `handler_palette_finalize.py` stay in place for v1/v2 artifacts.
  Those retire separately when classic color renders retire.
- This does not retroactively speed up or correct older fused artifacts
  that were rendered before `step_scores.raw` / sidecar v3 existed.
- No new Step Functions workflow.
- No changes to repalette, deepzoom, or any other existing handler.
- No changes to the main color render path beyond the new
  `step_scores.raw` artifact.

## Sizing note

`step_scores.raw` is `step_count × 1 byte`. For a 10K render
(`step_count ≈ 100 M`) that's 100 MB. For a 5K render it's 25 MB. Large
but not absurd — stored in the artifact prefix, cleaned when the
artifact is deleted. If the footprint becomes an operational concern,
add zstd compression behind a sidecar `step_scores_encoding` field.
Not needed for the first cut.

## Output parity

Given the same solve data, the new extract-palette path produces a
palette greyscale byte-identical to what FinalizeMT's
`_finalize_associated_palette` would have produced with
`save_associated_palette=true`. The encode path is the same
(`render_score_raw`), so the final palette image should match the same
rendering semantics and visual output. Exact JPEG byte identity is not
the contract.

## Summary of changes

| file / area                                                 | change                                                   |
|-------------------------------------------------------------|----------------------------------------------------------|
| `lambda/roots2pix_mt.c`                                     | per-thread step-scores buffer; `--step_scores_output=`   |
| `lambda/handler_raster_mt.py`                               | upload per-section step_scores fragment                  |
| `lambda/handler_finalize_mt.py`                             | concat per-section fragments; thread fields into sidecar |
| `lambda/raw_sidecar.py`                                     | bump to v3; `step_scores_key` + `step_count` + `step_scores_grid_n` fields |
| `lambda/step_scores_to_palette_raw.c` (new)                 | ~50-line C binary, step → serpentine → palette raw       |
| `lambda/handler_extract_palette_from_step_scores.py` (new)  | standalone handler, ~150 lines                           |
| `lambda/handler_dispatch.py`                                | new target `extract_palette_fused`                       |
| `deploy.sh`                                                 | package binary + handler; new Lambda                     |
| `index.html::runExtractPaletteArtifact`                     | dispatch branch on surfaced `step_scores_key` / `step_scores_grid_n` |
| `tests/test_raster_mt.py`                                   | step_scores emission                                     |
| `tests/test_finalize_mt_handler.py`                         | step_scores concat + v3 sidecar                          |
| `tests/test_raw_sidecar.py`                                 | v1/v2/v3 round-trip                                      |
| `tests/test_extract_palette_from_step_scores.py` (new)      | parity + no-fallback assertions                          |
| `tests/test_frontend_js.sh`                                 | dispatch branch assertion                                |
| `tests/test_deploy_packaging.py`                            | new Lambda + binary packaged                             |
