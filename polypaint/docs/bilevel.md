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
