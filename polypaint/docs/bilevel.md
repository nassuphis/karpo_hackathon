# Bilevel Render Lambda

Dedicated Lambda for rendering polynomial root images as 1-bit black/white PNGs. Stripe-first architecture: each stripe is processed once, each root is projected once.

## Architecture

Three-phase stripe-first pipeline:

```
Phase 1 — Raster (one Lambda per stripe):
  Download stripe .bin → project roots → write per-tile .bits files to S3

Phase 2 — Merge (one Lambda per tile):
  Download stripe .bits files for tile → OR bitsets → write 1-bit tile PNG

Phase 3 — Stitch (single Lambda):
  Download tile PNGs → vips_arrayjoin → write final 1-bit PNG
```

### Why stripe-first

The alternative (tile-first: one Lambda per tile, each downloads all stripes) was implemented first and rejected. Problems:

1. **Duplicated work**: every stripe downloaded N_tiles times, every root projected N_tiles times
2. **Duplicated IO**: with 10 stripes and 169 tiles, that's 1690 S3 downloads instead of 10
3. **Slow**: each tile Lambda spent most of its time downloading stripes

Stripe-first processes each stripe once, projects each root once, then fans out per-tile bitsets (tiny: 2 MB each) for the merge phase. The merge phase is trivial (bitwise OR).

## Data flow

```
stripe_0.bin ──→ bits_s0000_t0000.bits  bits_s0000_t0001.bits  ...
stripe_1.bin ──→ bits_s0001_t0000.bits  bits_s0001_t0001.bits  ...
  ...

For each tile:
  bits_s0000_tN.bits + bits_s0001_tN.bits + ... ──OR──→ bilevel_tN.png

bilevel_t0000.png + bilevel_t0001.png + ... ──stitch──→ image_bilevel.png
```

## Files

| File | Purpose |
|------|---------|
| `bilevel_raster.c` | Static binary: one stripe → per-tile .bits files. No libvips. |
| `bilevel_merge.c` | Dynamic binary (libvips): merge .bits → tile PNG, or stitch tile PNGs → final PNG. |
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

Merge mode (OR bitsets → tile PNG):
```
bilevel_merge merge --tile_w=TW --tile_h=TH --output=tile.png
    bits1.bits bits2.bits ...
```

Stitch mode (join tile PNGs → final PNG):
```
bilevel_merge stitch --n_cols=C --n_rows=R --output=final.png
    tile0.png tile1.png ...
```

Build: Docker ARM64 with libvips (same as raw2jpeg).

## Memory budget

### Raster Lambda (per stripe)
- One stripe .bin in memory: 20–200 MB
- Per-tile bitsets: `nTiles × ceil(tileW × tileH / 8)` — 169 × 2 MB = 338 MB for 50K
- **Total: ~500 MB** — fits in 1769 MB

### Merge Lambda (per tile)
- N stripe .bits files: N × 2 MB — 20 MB for 10 stripes
- Image buffer for PNG conversion: 16 MB (4096×4096 × 1 byte)
- **Total: ~40 MB**

### Stitch Lambda (single)
- libvips opens tile PNGs lazily (demand-driven, not all in memory)
- Working set: a few rows of tiles at a time
- Output streams to disk

## Deploy

```bash
./deploy.sh create   # or update
```

Single Lambda function `polypaint-bilevel` handles all three phases:
- 1769 MB memory (1 vCPU)
- 10 GB `/tmp`
- libvips Lambda layer
- Async retry disabled

Two binaries in the zip: `bilevel_raster` (static) + `bilevel_merge` (dynamic/libvips).

## Frontend pipeline

`runBilevelPipeline()` dispatches three phases sequentially:
1. Dispatch `nStripes` bilevel Lambdas with `phase: "raster"`
2. Poll `bilevel_raster_*` tasks until all complete
3. Dispatch `nTiles` bilevel Lambdas with `phase: "merge"`
4. Poll `bilevel_merge_*` tasks until all complete
5. Dispatch 1 bilevel Lambda with `phase: "stitch"`
6. Poll `bilevel_stitch` task until complete
7. Get presigned URL for final PNG

## Scaling

| Image size | Raster bitsets (total) | Merge input/tile | Old RGB pipeline |
|-----------|----------------------|-----------------|-----------------|
| 4096×4096 | 2 MB | 20 MB (10 stripes) | 48 MB/tile |
| 50K×50K | ~338 MB | 20 MB/tile | ~7 GB stitched raw |
| 100K×100K | ~1.2 GB | 20 MB/tile | ~28 GB (impossible) |

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

No tests for `bilevel_merge` locally — it requires libvips which is only available in the Lambda Docker build environment.

## Design history

### v1: Tile-first (rejected)

The first implementation dispatched one Lambda per tile. Each tile Lambda downloaded ALL stripe .bin files, projected all roots, and wrote a 1-bit PNG. Problems:

1. **`/tmp` exhaustion**: downloading all stripes filled 10 GB `/tmp`. Fixed by processing one stripe at a time with `--bitset` accumulation, but this was a workaround for the wrong architecture.
2. **Duplicated IO**: 10 stripes × 169 tiles = 1690 S3 downloads instead of 10.
3. **Duplicated compute**: every root projected 169 times instead of once.
4. **Encode contract break**: tile PNGs were fed into the old raw-tile encoder which expected a 12-byte binary header.
5. **`dl_ms` undefined**: handler returned an undefined variable on success, causing NameError after reporting "done".

### v2: Stripe-first (current)

Rewritten to match the architecture recommended in `image_render.md`:

1. Raster phase processes each stripe once (one Lambda per stripe)
2. Each root is projected once and bucketed into the correct tile's bitset
3. Tiny .bits files (2 MB each) are uploaded to S3
4. Merge phase is trivial: OR the bitsets, write PNG
5. Stitch phase uses libvips `vips_arrayjoin` to join tile PNGs — never touches the old raw-tile encoder

This is the same structure as the existing color pipeline (raster → finalize → encode) but with 24× less data flowing through S3.
