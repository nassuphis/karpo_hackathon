# Bilevel Render Lambda

Dedicated Lambda for rendering polynomial root images as 1-bit black/white TIFFs. Stripe-first architecture: each stripe is processed once, each root is projected once.

## Output format

TIFF with CCITT Group 4 compression throughout the pipeline. No zlib anywhere.

PNG uses zlib internally (even through libvips), which has size limits and is very slow at high compression for large images. CCITT G4 is purpose-built for bilevel data: no zlib dependency, no size limits, better compression for 1-bit content. libvips writes CCITT G4 natively via `vips_tiffsave`.

## Architecture

Three-phase stripe-first pipeline:

```
Phase 1 — Raster (one Lambda per stripe):
  Download stripe .bin → project roots → write per-tile .bits files to S3

Phase 2 — Merge (one Lambda per tile):
  Download stripe .bits files for tile → OR bitsets → write 1-bit tile TIFF

Phase 3 — Stitch (single Lambda):
  Download tile TIFFs → vips_arrayjoin → write final 1-bit TIFF
```

### Why stripe-first

The alternative (tile-first: one Lambda per tile, each downloads all stripes) was implemented first and rejected. Problems:

1. **Duplicated work**: every stripe downloaded N_tiles times, every root projected N_tiles times
2. **Duplicated IO**: with 10 stripes and 169 tiles, that's 1690 S3 downloads instead of 10
3. **Slow**: each tile Lambda spent most of its time downloading stripes

Stripe-first processes each stripe once, projects each root once, then fans out per-tile bitsets (tiny: 2 MB each) for the merge phase. The merge phase is trivial (bitwise OR).

### Why not PNG

The stitch phase on a 50K×50K image timed out at 600s using `vips_pngsave` with compression=9. PNG format uses zlib deflate internally, which is both slow and has a 32-bit size constraint that makes very large images problematic. CCITT G4 has none of these issues.

## Data flow

```
stripe_0.bin ──→ bits_s0000_t0000.bits  bits_s0000_t0001.bits  ...
stripe_1.bin ──→ bits_s0001_t0000.bits  bits_s0001_t0001.bits  ...
  ...

For each tile:
  bits_s0000_tN.bits + bits_s0001_tN.bits + ... ──OR──→ bilevel_tN.tif

bilevel_t0000.tif + bilevel_t0001.tif + ... ──stitch──→ image_bilevel.tif
```

## Files

| File | Purpose |
|------|---------|
| `bilevel_raster.c` | Static binary: one stripe → per-tile .bits files. No libvips. |
| `bilevel_merge.c` | Dynamic binary (libvips): merge .bits → tile TIFF, or stitch tile TIFFs → final TIFF. |
| `handler_bilevel.py` | Lambda handler: routes phase=raster/merge/stitch to appropriate binary. |

## C binary interfaces

### bilevel_raster

```
bilevel_raster stripe.bin /tmp/bits
    --width=W --height=H --tile_size=TS
    --n_tile_cols=C --n_tile_rows=R
    --center_re=X --center_im=Y --scale=S --degree=D
    [--rotation=R]
```

Output: `{prefix}_t0000.bits`, `{prefix}_t0001.bits`, ... (only non-empty tiles)
Each `.bits` file: raw packed bitset, `ceil(tile_w * tile_h / 8)` bytes. No header.

Build: `aarch64-linux-musl-gcc -O3 -static -o bilevel_raster bilevel_raster.c -lm`

### bilevel_merge

Merge mode (OR bitsets → tile TIFF):
```
bilevel_merge merge --tile_w=TW --tile_h=TH --output=tile.tif
    bits1.bits bits2.bits ...
```

Stitch mode (join tile TIFFs → final TIFF):
```
bilevel_merge stitch --n_cols=C --n_rows=R --output=final.tif
    tile0.tif tile1.tif ...
```

Both modes output TIFF with CCITT G4, bitdepth=1 via `vips_tiffsave`.

Build: Docker ARM64 with libvips (same as raw2jpeg).

## Memory budget

### Raster Lambda (per stripe)
- One stripe .bin in memory: 20–200 MB
- Per-tile bitsets: `nTiles × ceil(tileW × tileH / 8)` — 169 × 2 MB = 338 MB for 50K
- **Total: ~500 MB** — fits in 1769 MB

### Merge Lambda (per tile)
- N stripe .bits files: N × 2 MB — 20 MB for 10 stripes
- Image buffer for TIFF conversion: 16 MB (4096×4096 × 1 byte)
- **Total: ~40 MB**

### Stitch Lambda (single)
- libvips opens tile TIFFs lazily (demand-driven, not all in memory)
- Working set: a few rows of tiles at a time
- Output streams to disk

## Deploy

```bash
./deploy.sh create   # or update
```

Two Lambda functions:

**polypaint-bilevel** — raster + merge phases:
- 1769 MB memory (1 vCPU), 10 GB `/tmp`, libvips layer
- Async retry: **2 attempts, 3600s max event age** (not 0 — see below)
- Two binaries: `bilevel_raster` (static) + `bilevel_merge` (dynamic/libvips)

**polypaint-bilevel-stitch** — stitch phase:
- 6144 MB memory (~4 vCPUs), 10 GB `/tmp`, libvips layer
- Async retry: 0 (single invocation, not fan-out)
- Binary: `bilevel_merge` (stitch mode)

The stitch phase is a separate Lambda because libvips is multithreaded and benefits from extra vCPUs. The raster/merge Lambdas run at 500+ concurrency and don't need 6 GB each.

## Frontend pipeline

`runBilevelPipeline()` dispatches three phases sequentially, using `_bilevelDispatchAndPoll()` for the fan-out phases:

1. **Raster**: wave-dispatch `nStripes` bilevel Lambdas (`phase: "raster"`, MAX_INFLIGHT=200)
2. Poll `bilevel_raster_*` tasks; after 45s stall, query `return_ids`, re-dispatch missing (max 2 rounds)
3. **Merge**: wave-dispatch `nTiles` bilevel Lambdas (`phase: "merge"`, MAX_INFLIGHT=200)
4. Poll `bilevel_merge_*` tasks with same stall/re-dispatch logic
5. **Stitch**: dispatch 1 bilevel-stitch Lambda
6. Poll `bilevel_stitch` task until complete
7. Discover artifacts via `/head-keys`, display preview + download buttons

See [lambdas.md — Dispatch Resilience](lambdas.md#dispatch-resilience) for why wave dispatch and re-dispatch were added.

## Scaling

| Image size | Raster bitsets (total) | Merge input/tile | Old RGB pipeline |
|-----------|----------------------|-----------------|-----------------|
| 4096×4096 | 2 MB | 20 MB (10 stripes) | 48 MB/tile |
| 50K×50K | ~338 MB | 20 MB/tile | ~7 GB stitched raw |
| 100K×100K | ~1.2 GB | 20 MB/tile | ~28 GB (impossible) |

## Known limits

- **MAX_TILES=4096**: the raster binary supports up to a 64×64 tile grid. At 4096px tiles that's 262K×262K max. At 2048px tiles it's 131K×131K. Sufficient for the 100K target.
- **Whole-stripe malloc**: the raster binary loads the full stripe .bin into memory. This is the correct tradeoff for stripe-first (one stripe per Lambda), but memory is bounded by solver stripe sizing.
- **No merge/stitch local tests**: `bilevel_merge` requires libvips from the Lambda Docker build. Only `bilevel_raster` is tested locally.

## Tests

`polypaint/tests/test_bilevel_raster.py` — verifies the C binary against a Python reference implementation:

- **test_basic**: poly_1, 50×50 grid, 1000×1000 image, 2×2 tiles. Bitsets match byte-for-byte. Plotted/clipped/deduped counts match.
- **test_rotation**: quarter turn (0.25 turns), bitsets match.
- **test_empty_tiles**: offset viewport so all roots clip. Verifies no .bits files written.
- **test_multiple_functions**: poly_1, poly_4, poly_49 all produce byte-identical bitsets to Python.

Run: `cd polypaint/tests && uv run python test_bilevel_raster.py`

Requires `bilevel_raster_local` (natively compiled) in `polypaint/lambda/`:
```bash
cd polypaint/lambda && cc -O3 -o bilevel_raster_local bilevel_raster.c -lm
```

## Design history

### v1: Tile-first (rejected)

The first implementation dispatched one Lambda per tile. Each tile Lambda downloaded ALL stripe .bin files, projected all roots, and wrote a 1-bit PNG. Problems:

1. **`/tmp` exhaustion**: downloading all stripes filled 10 GB `/tmp`. Fixed by processing one stripe at a time with `--bitset` accumulation, but this was a workaround for the wrong architecture.
2. **Duplicated IO**: 10 stripes × 169 tiles = 1690 S3 downloads instead of 10.
3. **Duplicated compute**: every root projected 169 times instead of once.
4. **Encode contract break**: tile PNGs were fed into the old raw-tile encoder which expected a 12-byte binary header.
5. **`dl_ms` undefined**: handler returned an undefined variable on success, causing NameError after reporting "done".

### v2: Stripe-first with PNG (stitch timeout)

Rewrote to stripe-first architecture but still used PNG output (`vips_pngsave`). The stitch phase timed out at 600s on a 50K image — `vips_pngsave` with compression=9 on 2.5 billion 1-bit pixels is too slow because PNG uses zlib internally.

### v3: Stripe-first with TIFF CCITT G4 (current)

Switched all output from PNG to TIFF with CCITT Group 4 compression. No zlib anywhere in the pipeline. CCITT G4 is purpose-built for bilevel data, has no size limits, and compresses faster with better ratios than PNG for 1-bit content.

## Post-render exports

After the bilevel TIFF is produced, the artifact panel offers on-demand conversions:

| Export | Lambda | Purpose |
|--------|--------|---------|
| Preview-Compatible TIFF | polypaint-tiff-compat | macOS Preview can't open tiled TIFFs. Converts to strip-based layout. |
| PNG | polypaint-png-export | 1-bit PNG via libvips. Smaller than TIFF for web use. |
| DeepZoom | polypaint-deepzoom-export | OpenSeadragon tile pyramid for zoomable web viewing. |

These are triggered by buttons in the artifact panel, not part of the render pipeline. Each runs a separate Lambda, uploads the result to S3, and refreshes the artifact panel.

## Warm container /tmp bug

**Symptom:** bilevel render produced corrupted output — tiles contained data from a previous render.

**Root cause:** Lambda reuses containers. The `/tmp` directory persists between invocations. The raster phase writes `.bits` files to `/tmp/bits_t0000.bits`, etc. If a warm container runs a different stripe than last time, the old `.bits` files are still there. The upload loop iterates over all expected tile indices and uploads whatever `.bits` file exists at that path — including stale ones from the previous invocation.

**Fix:** `handler_bilevel.py` now globs and removes all stale `.bits` files before each raster run:
```python
import glob
for stale in glob.glob("/tmp/bits_t*.bits"):
    os.remove(stale)
```

Same pattern for coeff raster (`/tmp/coeff_bits_t*.bits`).
