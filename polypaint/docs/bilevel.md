# Bilevel Render Lambda

Dedicated Lambda for rendering root or coefficient occupancy as 1-bit black/white TIFFs. The active render path is sparse-section first: each logical section emits global pixel-hit fragments, then one finalize Lambda assembles and encodes the image.

## Output format

TIFF with CCITT Group 4 compression throughout the pipeline. No zlib anywhere.

PNG uses zlib internally (even through libvips), which has size limits and is very slow at high compression for large images. CCITT G4 is purpose-built for bilevel data: no zlib dependency, no size limits, better compression for 1-bit content. libvips writes CCITT G4 natively via `vips_tiffsave`.

## Architecture

Two-phase sparse pipeline:

```
Phase 1 — Section raster (one Lambda per logical section):
  Download root/coeff byte spans → project points → write sparse .frag to S3

Phase 2 — Finalize (single Lambda):
  Presign .frag files → assemble_greyscale raw → raw_to_bilevel TIFF + preview
```

### Why sparse sections

The active path uses logical source sections and global pixel indexes:

1. Each source row is downloaded once by its logical section.
2. Each root/coefficient is projected once.
3. Fragments use global pixel indexes, so no image-region assembly topology is needed.
4. Final assembly is shared with color sparse fragments.

### Why not PNG

The stitch phase on a 50K×50K image timed out at 600s using `vips_pngsave` with compression=9. PNG format uses zlib deflate internally, which is both slow and has a 32-bit size constraint that makes very large images problematic. CCITT G4 has none of these issues.

## Data flow

```
root/coeff spans for section 0 ──→ bilevel_section_0000.frag
root/coeff spans for section 1 ──→ bilevel_section_0001.frag
  ...

section_*.frag ──assemble_greyscale──→ greyscale.raw ──raw_to_bilevel──→ image.tif + preview.png
```

## Files

| File | Purpose |
|------|---------|
| `bilevel_section_raster.c` | Static binary: one logical root section → sparse occupancy fragment. |
| `coeffs_bilevel_raster.c` | Static binary: one logical coefficient section → sparse occupancy fragment. |
| `assemble_greyscale.c` | Assembles sparse `[u32le pixel_idx][u8 score]` fragments into dense raw. |
| `raw_to_bilevel.c` | Encodes dense raw occupancy into TIFF + preview. |
| `handler_bilevel.py` | Lambda handler: routes section raster, coeff raster, finalize, and Color2Bilevel phases. |

## C binary interfaces

### bilevel_section_raster

```
bilevel_section_raster section.bin out.frag
    --pix=N
    --min_re=A --max_re=B --min_im=C --max_im=D
    --degree=D
    [--rotation=R]
```

Output fragment records: `[u32le global_pixel_idx][u8 score=1]`.

### coeffs_bilevel_raster

```
coeffs_bilevel_raster coeffs.bin out.frag
    --pix=N
    --min_re=A --max_re=B --min_im=C --max_im=D
    --n_coeffs=C
    [--rotation=R]
```

Output fragment records: `[u32le global_pixel_idx][u8 score=1]`.

## Memory budget

### Section raster Lambda
- One logical root/coeff section in `/tmp`
- One full-image claim bitset: `ceil(pix * pix / 8)` bytes
- Sparse hit vector: `4 bytes * unique_hit_count`

### Finalize Lambda
- Dense raw output: `pix * pix` bytes
- Fragment download buffers are bounded by worker count
- TIFF encode uses libvips via `raw_to_bilevel`

## Deploy

```bash
./deploy.sh create   # or update
```

One Lambda function:

**polypaint-bilevel** — sparse raster + finalize phases:
- 1769 MB memory (1 vCPU), 10 GB `/tmp`, libvips layer
- Async retry: **2 attempts, 3600s max event age** (not 0 — see below)
- Binaries: `bilevel_section_raster`, `coeffs_bilevel_raster`, `assemble_greyscale`, `raw_to_bilevel`

## Frontend pipeline

Render is Step Functions driven:

1. `BilevelRasterMap` or `CoeffRasterMap` fans out logical section raster tasks.
2. `BilevelFinalizeTask` or `CoeffFinalizeTask` assembles sparse fragments and encodes TIFF + preview.
3. `/render-summary` refreshes the Render family catalog.

## Known limits

- **Full-image claim bitset**: section raster needs `ceil(pix * pix / 8)` bytes for dedupe.
- **Sparse fragment density**: fragments cost 5 bytes per unique occupied pixel. Very dense coefficient renders can create large fragments.
- **Finalize raw size**: final assembly writes `pix * pix` raw bytes before TIFF encoding.

## Tests

- `tests/test_bilevel_handler.py` covers handler section raster and finalize contracts.
- `tests/test_exact_viewport_parity.py` covers native projection math for root and coeff sparse rasterizers.
- `tests/docker_runtime_regression.py` covers deployed ARM64 binaries inside the Lambda-like Docker runtime.

## Design history

### v1: Region-first (rejected)

The first implementation split the target image into output regions and
reprocessed root data repeatedly. That architecture duplicated IO and compute,
made `/tmp` pressure worse, and created a brittle stitching/encoding boundary.

### v2: Stripe-first with PNG (stitch timeout)

Rewrote to stripe-first architecture but still used PNG output (`vips_pngsave`). The stitch phase timed out at 600s on a 50K image — `vips_pngsave` with compression=9 on 2.5 billion 1-bit pixels is too slow because PNG uses zlib internally.

### v3: Stripe-first with TIFF CCITT G4 (current)

Switched all output from PNG to TIFF with CCITT Group 4 compression. No zlib anywhere in the pipeline. CCITT G4 is purpose-built for bilevel data, has no size limits, and compresses faster with better ratios than PNG for 1-bit content.

## Post-render exports

After the bilevel TIFF is produced, the artifact panel offers on-demand conversions:

| Export | Lambda | Purpose |
|--------|--------|---------|
| Preview-Compatible TIFF | polypaint-tiff-compat | Converts TIFF layout for macOS Preview compatibility. |
| PNG | polypaint-png-export | 1-bit PNG via libvips. Smaller than TIFF for web use. |
| DeepZoom | polypaint-deepzoom-export | OpenSeadragon tile pyramid for zoomable web viewing. |

These are triggered by buttons in the artifact panel, not part of the render pipeline. Each runs a separate Lambda, uploads the result to S3, and refreshes the artifact panel.

## Warm container /tmp bug

**Symptom:** bilevel render produced corrupted output from stale warm-container intermediate files.

This stale `/tmp/*.bits` class is removed from the active sparse section path because each worker writes exactly one deterministic fragment path.
