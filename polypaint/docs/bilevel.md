# Bilevel Render Lambda

Dedicated Lambda for rendering polynomial root images as 1-bit black/white PNGs. Designed for large renders (50K–100K pixels) where the old RGB pipeline hits memory and `/tmp` limits.

## Why a separate Lambda

The color render pipeline (raster → finalize → encode) carries RGB data throughout and only converts to 1-bit at the very end. For bilevel output this is wasteful:

- raster emits 8-byte entries per pixel (pixel_idx + RGB) when only 1 bit is needed
- finalize allocates a full RGB tile buffer (48 MB for 4096×4096) per tile
- encode stitches all tiles into one giant raw file on `/tmp` then converts

At 50K×50K the stitched raw file is ~7 GB. At 100K×100K it's ~28 GB — well beyond Lambda's 10 GB `/tmp` limit. The bilevel Lambda avoids this entirely by working with bitsets throughout.

## Architecture

One Lambda invocation per tile. Each processes all stripes for that tile:

```
For each tile:
  For each stripe .bin:
    download → project roots → set bits in tile bitset → delete .bin
  Write 1-bit PNG from bitset via libvips
  Upload tile PNG to S3
```

No intermediate RGB files. No finalize phase. No giant stitched raw.

### Memory budget per Lambda

- Tile bitset: `ceil(tile_w * tile_h / 8)` bytes — 2 MB for 4096×4096
- One stripe .bin: typically 20–200 MB
- Image buffer for PNG conversion: `tile_w * tile_h` bytes — 16 MB for 4096×4096
- **Total: ~220 MB peak** — well within the 1769 MB Lambda memory

### `/tmp` budget per Lambda

Only one stripe and one bitset file on disk at any time:

- `/tmp/current_stripe.bin`: one stripe (~200 MB)
- `/tmp/bilevel_N.bits`: accumulated bitset (~2 MB)
- `/tmp/tile.png`: final output (typically 50–200 KB)
- **Total: ~200 MB** — fits easily in 10 GB `/tmp`

## The `/tmp` exhaustion bug

The initial implementation downloaded ALL stripe .bin files to `/tmp` before processing. With 10 stripes at 200 MB each, that's 2 GB per Lambda. With Lambda execution environment reuse and concurrent invocations sharing `/tmp`, this exhausted the 10 GB budget immediately.

**Error:** `[Errno 28] No space left on device` on all 169 bilevel tiles.

**Fix:** Process stripes one at a time. The C binary gained two flags:
- `--bitset=path`: load/save a persistent bitset file between invocations
- `--no-png=1`: skip PNG output during accumulation passes

The handler now loops: for each stripe, download the .bin, run bilevel with `--no-png=1 --bitset=path`, delete the .bin. On the final stripe, it drops `--no-png` so the PNG is written.

## Files

| File | Purpose |
|------|---------|
| `lambda/bilevel.c` | C binary: root projection → tile bitset → 1-bit PNG via libvips |
| `lambda/handler_bilevel.py` | Lambda handler: stripe-by-stripe download, run binary, upload result |
| `deploy.sh` | Compilation (Docker ARM64 + libvips), packaging, Lambda create/update |

## C binary interface

```
bilevel --full_w=W --full_h=H --tile_col=C --tile_row=R
        --tile_w=TW --tile_h=TH --tile_size=TS
        --center_re=X --center_im=Y --scale=S --degree=D
        [--rotation=R] [--bitset=path] [--no-png=1]
        --output=tile.png
        stripe.bin
```

- Positional args are `.bin` file paths (processed sequentially, freed after each)
- `--bitset=path`: load existing bitset before processing, save after. Enables incremental accumulation across multiple invocations.
- `--no-png=1`: skip PNG output (used during accumulation passes)
- Output: JSON metadata to stdout, tile PNG to `--output` path

## Frontend pipeline

`runBilevelPipeline()` in `index.html`:

1. **Viewport** — compute center/scale (same as color pipeline)
2. **Bilevel tiles** — dispatch one `bilevel` Lambda per tile via `target: 'bilevel'`
3. **Encode stitch** — existing encode Lambda joins tile PNGs into final image

The old 3-phase pipeline (raster + finalize + encode) is replaced by 2 phases (bilevel + encode). The raster and finalize Lambdas are not involved.

## Scaling

| Image size | Bitset per tile | RGB tile (old) | Reduction |
|-----------|----------------|----------------|-----------|
| 4096×4096 tile | 2 MB | 48 MB | 24× |
| 50K×50K total | ~300 MB | ~7 GB | 24× |
| 100K×100K total | ~1.2 GB | ~28 GB | 24× |

The current encode stitch step still writes the final image to `/tmp`, so the absolute limit is around 60K for now (stitched 1-bit data fits in 10 GB). A streaming bilevel PNG encoder would remove that limit entirely.

## Deploy

The bilevel Lambda is compiled in the same Docker container as raw2jpeg (needs libvips):

```bash
./deploy.sh create   # or update
```

It gets:
- 1769 MB memory (1 vCPU)
- 10 GB `/tmp`
- libvips Lambda layer
- Async retry disabled (fire-and-forget via dispatch)

The dispatch Lambda's env vars include `BILEVEL_FUNCTION=polypaint-bilevel`.
