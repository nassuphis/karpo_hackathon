# Off-Screen Bitmap Rendering

## Motivation

At 10K resolution, the bitmap `<canvas>` element uses ~400MB of GPU memory (10,000 × 10,000 × 4 bytes). After running fast mode for an extended period, Chrome's GPU memory manager reclaims the canvas context — the canvas goes white and shows a stylized dead face icon (Chrome's canvas context loss indicator). This is not a bug in the application code; it's Chrome enforcing GPU memory limits.

The irony: the display container is CSS-scaled to ~500px, so **99.99% of the rendered pixels are never visible on screen**. The full resolution only matters at export time. We're burning 400MB of GPU memory to display a 500px preview.

**Target**: 15K and 25K resolution must work reliably.

## Analysis

### Where does the GPU memory come from?

Two data structures hold bitmap pixel data:

1. **`bitmapPersistentBuffer`** (ImageData) — a `Uint8ClampedArray` of size `W × H × 4` bytes. This is **CPU-only memory**. No GPU involvement. It exists so we can avoid calling `getImageData()` every pass (which was the original 14x speedup from the persistent buffer optimization).

2. **`<canvas id="bitmap-canvas">`** — the HTML canvas element. When a 2D context is obtained via `getContext("2d")`, the browser allocates a GPU-backed backing store at the canvas's native resolution. At 10K, this is ~400MB of GPU memory. This is the only GPU allocation.

The `putImageData()` call transfers the dirty rectangle from the CPU-side persistent buffer to the GPU-side canvas backing store. This is the bridge between the two.

### Why does it crash after "a while"?

The GPU memory allocation is **constant** — a 10K canvas uses 400MB whether 0 or 100 million pixels have been painted. The timing of the crash depends on:

- **Chrome's GPU memory budget** — typically 512MB–1GB shared across all tabs. A single 10K canvas consumes most of this budget.
- **Browser heuristics** — Chrome periodically evaluates whether to reclaim expensive canvas contexts. The longer you run, the more chances this check fires.
- **Other tabs/processes** competing for the same GPU memory pool.
- **macOS unified memory** — GPU allocations compete with system RAM.

### Current data flow

```
Workers compute pixels at canvas resolution (e.g., 10K)
    ↓
compositeWorkerPixels() writes to bitmapPersistentBuffer (CPU, 400MB)
    ↓
putImageData(persistentBuffer, dirty rect) → <canvas> (GPU, 400MB)
    ↓
Browser CSS-scales canvas → ~500px visible on screen
    ↓
Save: canvas.toBlob("image/png") → full-res PNG file
```

The GPU allocation exists solely to display a CSS-downscaled preview and to serve as the source for `toBlob()` export.

### The insight

`ImageData` is CPU-only. We can keep the persistent buffer at any resolution without touching the GPU. The canvas only needs to be large enough for display — a 2000px canvas (16MB GPU) looks identical to a 10K canvas when CSS-scaled to 500px. For export, we write directly from the CPU persistent buffer as BMP — no GPU involvement at all.

## Architecture

```
Workers compute pixels at COMPUTE resolution (e.g., 15K)
    ↓
compositeWorkerPixels() writes to:
    ├─ bitmapPersistentBuffer (ImageData, computeRes × computeRes, CPU-only)
    └─ bitmapDisplayBuffer (ImageData, displayRes × displayRes, CPU-only)
           ↓
       putImageData → <canvas> (displayRes × displayRes, ~16MB GPU)
           ↓
       CSS scales canvas to container size (~500px visible)

Export: exportPersistentBufferAsBMP() → 24-bit BMP directly from CPU buffer
```

**Split activates only when computeRes > DISPLAY_CAP (2000px).** At 1000px or 2000px, `bitmapDisplayBuffer` is null and behavior is identical to today — zero overhead.

### Pixel index contract

Workers always compute pixel indices in **compute-space**. The `paintIdx` values from workers are indices into the full `computeRes × computeRes` grid. The composite function:
1. Writes RGBA to `bitmapPersistentBuffer` at compute-space offset (unchanged)
2. Downsamples to display-space for `bitmapDisplayBuffer` and dirty-rect tracking

**Critical**: `serializeFastModeData()` must send `bitmapComputeRes` as `canvasW`/`canvasH` to workers, NOT `bitmapCtx.canvas.width` (which is now display-sized). Without this, workers would silently render at 2000px resolution.

### Per-pixel overhead in split mode

For each worker pixel, the composite function does one extra downscale + write:
- 1 modulo (`pixIdx % computeW`): ~1ns
- 1 integer divide (`(pixIdx / computeW) | 0`): ~1ns
- 2 multiply + truncate for display coordinates: ~2ns
- 1 multiply + add for display buffer offset: ~1ns
- 4 byte writes to display buffer: ~4ns
- **Total: ~9ns per pixel**

At 50K pixels per pass with a typical pass time of 150–300ms, this adds 0.45ms — a 0.15–0.3% overhead.

### Display quality

With nearest-neighbor downscaling (last-writer-wins), multiple compute pixels that map to the same display pixel overwrite each other. At 10K compute / 2000px display, each display pixel represents a 5×5 block. Since workers visit pixels stochastically (each time step produces different root positions), over hundreds of passes the display converges to a reasonable preview. The CSS display is ~500px, so there are 4 display pixels per visible pixel — effective oversampling makes nearest-neighbor artifacts invisible.

The full-resolution persistent buffer is always available for export — export quality is unaffected.

### BMP export

Export uses a pure-CPU BMP writer (`exportPersistentBufferAsBMP()`) that reads directly from `bitmapPersistentBuffer.data`. No canvas, no GPU, no `toBlob()`.

**Why BMP?**
- Simplest format to write — 54-byte header + raw BGR pixel rows
- No compression library needed (unlike PNG which requires deflate)
- Zero risk of GPU context issues during export
- macOS Preview opens BMP natively
- Chunked Blob construction (8MB chunks) avoids single massive allocation

**BMP format details:**
- 24-bit uncompressed (no alpha channel)
- 14-byte file header + 40-byte DIB header (BITMAPINFOHEADER)
- Rows stored bottom-up per BMP spec, each row padded to 4-byte boundary
- Byte order: BGR (not RGB)

Multi-format export (JPEG, PNG, BMP, TIFF) is now implemented — see below.

### Memory budget

| Resolution | Persistent Buffer | Display Canvas | BMP File Size |
|-----------|-------------------|----------------|---------------|
| 1000px | 4 MB (no split) | 4 MB GPU | 3 MB |
| 2000px | 16 MB (no split) | 16 MB GPU | 12 MB |
| 5000px | 100 MB | 16 MB GPU | 75 MB |
| 8000px | 256 MB | 16 MB GPU | 192 MB |
| 10000px | 400 MB | 16 MB GPU | 300 MB |
| 15000px | 900 MB | 16 MB GPU | 675 MB |
| 25000px | 2.5 GB | 16 MB GPU | 1.9 GB |

GPU savings: 384MB (10K), 884MB (15K), 2.48GB (25K). The 16MB display buffer added to CPU is negligible.

**25K warning**: The 2.5GB persistent buffer needs 16GB+ system RAM. Should add try/catch around `new ImageData(25000, 25000)` with user-friendly error message.

## Implementation Plan

All changes in `index.html`.

### Step 1: Add state variables (near line 925)

After `let bitmapPersistentBuffer = null;`:

```javascript
const BITMAP_DISPLAY_CAP = 2000;
let bitmapComputeRes = 0;
let bitmapDisplayRes = 0;
let bitmapDisplayBuffer = null;  // ImageData at display res (null when no split)
```

### Step 2: Add 8K and 25K to resolution dropdown (line 773)

Add `8000px` between 5000 and 10000, and `25000px` after 15000.

### Step 3: Add `fillDisplayBuffer()` helper (after `fillPersistentBuffer`, ~line 7862)

Same exponential `copyWithin` pattern, operates on `bitmapDisplayBuffer`.

### Step 4: Rewrite `initBitmapCanvas()` (lines 7864-7885)

- `bitmapComputeRes = res` (from dropdown)
- `bitmapDisplayRes = Math.min(res, BITMAP_DISPLAY_CAP)`
- **Canvas** gets `bitmapDisplayRes` (small GPU footprint)
- **Persistent buffer**: `new ImageData(bitmapComputeRes, bitmapComputeRes)` — direct constructor, NOT `createImageData` (decoupled from canvas size)
- **Display buffer**: `new ImageData(bitmapDisplayRes, bitmapDisplayRes)` only when split active, otherwise null

### Step 5: Fix `serializeFastModeData()` (line 8652) — MOST CRITICAL

```javascript
canvasW: bitmapComputeRes, canvasH: bitmapComputeRes,
```

### Step 6: Fix `enterFastMode()` resolution check (line 8466)

`bitmapComputeRes !== wantRes` instead of `canvas.width !== wantRes`

### Step 7: Rewrite `compositeWorkerPixels()` (lines 8782-8837)

- Write to persistent buffer in compute-space (unchanged)
- When split active: downsample each pixel to display buffer
- Track dirty rect in display-space
- `putImageData` uses display buffer (or persistent buffer when no split)

### Step 8: Fix `plotCoeffCurvesOnBitmap()` (line 8379)

Use `bitmapDisplayRes` for width/height (display-only function).

### Step 9: Add `exportPersistentBufferAsBMP()` function

24-bit BMP from RGBA data with chunked Blob construction.

### Step 10: Replace bitmap save handler (lines 10062-10072)

BMP export from persistent buffer. Filename `.bmp`.

### Step 11: Fix clear handler (lines 10074-10082)

Also clear display buffer when split active.

### Step 12: Fix `resetBitmap()` (lines 9064-9079)

Null out: `bitmapPersistentBuffer`, `bitmapDisplayBuffer`, `bitmapComputeRes`, `bitmapDisplayRes`.

### Step 13: Fix snap "bitmap" export (lines 6164-6190)

Use `bitmapComputeRes` for dimensions, `putImageData(bitmapPersistentBuffer)` instead of `drawImage`.

### Step 14: Fix timing copy resolution (line 6625)

`bitmapComputeRes || null` instead of `bitmapCtx.canvas.width`.

## Verification

1. **2000px**: No split path active, behavior identical to current
2. **5000px**: Canvas 2000px, workers compute at 5000px, preview downsampled
3. **10000px**: No GPU context loss (canvas is only 2000px)
4. **15000px**: ~900MB RAM for persistent buffer, BMP export works
5. **25000px**: Test on 16GB+ machine
6. **BMP export**: Opens correctly in macOS Preview at full resolution
7. **Clear**: Both buffers reset to background color
8. **Snap bitmap**: Exports at compute resolution

## Risks and Mitigations

- **25K OOM**: `new ImageData(25000, 25000)` allocates 2.5GB. Wrap in try/catch, show user error suggesting lower resolution.
- **BMP file sizes**: 675MB at 15K, 1.9GB at 25K. Mitigated by multi-format export (see below).
- **Snap at compute-res**: Creates temp canvas at full compute resolution (momentary GPU use). Acceptable for one-shot export. Could fall back to BMP for snaps too if needed.
- **Display quality**: Nearest-neighbor downsampling is fine — CSS already downscales to ~500px, and the persistent buffer preserves full resolution for export.

## Multi-Format Export

The save button opens a popup with format selection (JPEG, PNG, BMP, TIFF). All encoding is pure-JS from `bitmapPersistentBuffer.data` — no canvas/GPU involvement.

### Libraries (loaded via CDN `<script>` tags)

| Format | Library | Global | Notes |
|--------|---------|--------|-------|
| BMP | Built-in | — | `exportPersistentBufferAsBMP()`, 24-bit uncompressed |
| JPEG | [jpeg-js](https://github.com/jpeg-js/jpeg-js) | `window['jpeg-js'].encode` | Lossy, quality slider (50-100), default 92 |
| PNG | [UPNG.js](https://github.com/photopea/UPNG.js) + [Pako](https://github.com/nicknassuphis/pako) | `window.UPNG` + `window.pako` | Lossless deflate |
| TIFF | [UTIF.js](https://github.com/photopea/UTIF.js) | `window.UTIF` | Uncompressed RGBA |

### File Sizes at 15K

| Format | Size | Encoding Time |
|--------|------|---------------|
| BMP | ~675 MB | ~2-5s |
| JPEG (q=92) | ~5-35 MB | ~10-30s |
| PNG | ~4-50 MB | ~30-120s |
| TIFF | ~200-675 MB | ~5-15s |

**Note**: PNG is often the smallest format for PolyPaint bitmaps because they are very sparse (99%+ single background color). PNG's deflate compression handles uniform regions far better than JPEG's DCT blocks. At 15K, a typical sparse render: PNG ~4.5MB vs JPEG ~35MB. JPEG is the default export format. Format preference persists in save/load state (`bitmapExportFormat`).
