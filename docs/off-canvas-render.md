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

### Original data flow (before off-canvas split)

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

The GPU allocation existed solely to display a CSS-downscaled preview and to serve as the source for `toBlob()` export.

### The insight

`ImageData` is CPU-only. We can keep the persistent buffer at any resolution without touching the GPU. The canvas only needs to be large enough for display — a 2000px canvas (16MB GPU) looks identical to a 10K canvas when CSS-scaled to 500px. For export, we encode directly from the CPU persistent buffer (BMP, JPEG, PNG, or TIFF) — no GPU involvement at all.

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

Export: exportPersistentBufferAs{BMP,JPEG,PNG,TIFF}(rgba, w, h, ...) from CPU buffer
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

### Export

All export formats use pure-CPU encoders that read from `bitmapPersistentBuffer.data`. No canvas, no GPU, no `toBlob()`. Each encoder takes `(rgba, width, height, filename)` parameters (JPEG also takes a quality parameter). See the Multi-Format Export section below for details.

The save button opens a popup (`bitmap-save-pop`) with format selection and a Download button. Encoding is deferred with `setTimeout` so the "Saving..." label renders before the blocking encode runs.

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

**25K warning**: The 2.5GB persistent buffer needs 16GB+ system RAM. `initBitmapCanvas()` wraps the allocation in try/catch with a user-friendly error alert.

## Implementation Details

All changes in `index.html`. All steps below have been implemented.

### State variables (~line 1086)

After `let bitmapPersistentBuffer = null;`:

```javascript
const BITMAP_DISPLAY_CAP = 2000;   // max display canvas resolution (px)
let bitmapComputeRes = 0;          // full computation resolution (from dropdown)
let bitmapDisplayRes = 0;          // display canvas resolution = min(computeRes, DISPLAY_CAP)
let bitmapDisplayBuffer = null;    // ImageData at display resolution (null when no split needed)
```

### Resolution dropdown

Options: 1000, 2000 (default), 5000, 8000, 10000, 15000, 25000.

### Steps dropdown (~line 895)

`bitmap-steps-select` controls the number of time steps per fast-mode pass. Options: 10, 100, 1K, 5K, 10K (default), 50K, 100K, 1M. The value is read in `enterFastMode()` and passed through `serializeFastModeData()` as `stepsVal`.

### Dropdown change listeners (~line 12435)

Both the steps dropdown and the resolution dropdown have `change` event listeners that automatically restart fast mode when changed during an active run:

```javascript
document.getElementById("bitmap-steps-select").addEventListener("change", function () {
    if (fastModeActive) { exitFastMode(); enterFastMode(); }
});
document.getElementById("bitmap-res-select").addEventListener("change", function () {
    if (fastModeActive) { exitFastMode(); enterFastMode(); }
});
```

This allows the user to adjust steps or resolution on the fly without manually pausing and restarting. The exit/re-enter cycle tears down existing workers, re-reads both dropdowns, re-serializes all fast-mode data, and spawns fresh workers at the new settings.

### `fillDisplayBuffer()` helper (~line 8822)

Same exponential `copyWithin` pattern as `fillPersistentBuffer()`, operates on `bitmapDisplayBuffer`.

### `initBitmapCanvas()` (~line 8927)

- `bitmapComputeRes = res` (from dropdown)
- `bitmapDisplayRes = Math.min(res, BITMAP_DISPLAY_CAP)`
- **Canvas** gets `bitmapDisplayRes` (small GPU footprint)
- **Persistent buffer**: `new ImageData(bitmapComputeRes, bitmapComputeRes)` — direct constructor, NOT `createImageData` (decoupled from canvas size). Wrapped in try/catch with user-friendly OOM alert.
- **Display buffer**: `new ImageData(bitmapDisplayRes, bitmapDisplayRes)` only when `bitmapComputeRes > BITMAP_DISPLAY_CAP`, otherwise null

### `serializeFastModeData()` (~line 9794) — MOST CRITICAL

```javascript
canvasW: bitmapComputeRes, canvasH: bitmapComputeRes,
```

Workers always receive the compute resolution, not the display canvas size.

### `enterFastMode()` resolution check (~line 9667)

`bitmapComputeRes !== wantRes` instead of `canvas.width !== wantRes`.

### Worker partitioning (~line 10250)

`initFastModeWorkers()` caps the number of spawned workers to avoid empty work ranges:

```javascript
const actualWorkers = Math.min(numWorkers, sd.stepsVal);
```

Without this, selecting 100 steps with 16 configured workers would spawn 16 workers but only 100 steps to distribute — 10 workers would receive zero steps (`stepEnd === stepStart`) and produce empty pixel buffers. The fix ensures at most `stepsVal` workers are created, so every worker gets at least one step.

`dispatchPassToWorkers()` (~line 10316) distributes steps evenly across workers using integer division with remainder:

```javascript
const base = Math.floor(stepsVal / nw);
const extra = stepsVal % nw;
// Worker w gets: base + (w < extra ? 1 : 0) steps
```

Each worker receives a contiguous `[stepStart, stepEnd)` range. The first `extra` workers each get one additional step. This guarantees all `stepsVal` steps are covered exactly once with no gaps or overlaps.

Worker paint buffers are pre-allocated at init time based on the per-worker step count:

```javascript
maxPaintsPerWorker: Math.ceil(sd.stepsVal / actualWorkers) * sd.nRoots,
```

Each step produces at most `nRoots` painted pixels, so `maxPaintsPerWorker` is the ceiling of steps-per-worker times `nRoots`. The worker blob uses this to size its `paintIdx`/`paintR`/`paintG`/`paintB` typed arrays, falling back to `totalSteps * nRoots` if not provided (backward compat).

### `compositeWorkerPixels()` (~line 10052)

- Writes to persistent buffer in compute-space (unchanged)
- When split active (`bitmapDisplayBuffer !== null`): downsamples each pixel to display buffer using `invScale = dW / cW`
- Tracks dirty rect in display-space
- `putImageData` uses display buffer (or persistent buffer when no split)

### `plotCoeffCurvesOnBitmap()` (~line 9587)

Uses `bitmapDisplayRes || bitmapCtx.canvas.width` for width/height (display-only function that paints to the display canvas).

### Export functions (~line 8837)

Four pure-CPU encoders, each taking `(rgba, width, height, filename)` (JPEG adds quality):
- `exportPersistentBufferAsBMP()` — 24-bit BMP, chunked Blob
- `exportPersistentBufferAsJPEG()` — lossy via jpeg-js
- `exportPersistentBufferAsPNG()` — lossless via UPNG.js + pako
- `exportPersistentBufferAsTIFF()` — uncompressed via UTIF.js

### Save popup handler (~line 7352)

Save popup with format dropdown, quality slider (JPEG only), and Download button. Reads from `bitmapPersistentBuffer.data` at `bitmapComputeRes`.

### Clear handler (~line 12113)

Clears canvas, persistent buffer, and display buffer (when split active via `if (bitmapDisplayBuffer) fillDisplayBuffer()`).

### `resetBitmap()` (~line 10374)

Nulls out: `bitmapPersistentBuffer`, `bitmapDisplayBuffer`, `bitmapComputeRes = 0`, `bitmapDisplayRes = 0`.

### Snap "bitmap" export (~line 6997)

Creates temp canvas at `bitmapComputeRes` dimensions, uses `ctx.putImageData(bitmapPersistentBuffer, 0, 0)` to copy full-resolution data, then `canvas.toBlob("image/png")` for the snap PNG download.

### Timing copy resolution (~line 7531)

`bitmapComputeRes || null` instead of `bitmapCtx.canvas.width`.

## Verified Behavior

1. **2000px and below**: No split path active, `bitmapDisplayBuffer` is null, behavior identical to original
2. **5000px**: Canvas 2000px, workers compute at 5000px, preview downsampled
3. **10000px**: No GPU context loss (canvas is only 2000px / 16MB GPU)
4. **15000px**: ~900MB RAM for persistent buffer, multi-format export works
5. **25000px**: Requires 16GB+ system RAM (2.5GB persistent buffer)
6. **Export**: All four formats encode correctly from persistent buffer
7. **Clear**: Both buffers reset to background color
8. **Snap bitmap**: Exports at compute resolution via temp canvas + `putImageData`
9. **Low step counts**: 100 steps with 16 workers spawns only 100 workers (one step each), no empty-range workers
10. **Dropdown hot-swap**: Changing steps or resolution dropdown during fast mode restarts cleanly (exit + re-enter)

## Risks and Mitigations

- **25K OOM**: `new ImageData(25000, 25000)` allocates 2.5GB. `initBitmapCanvas()` wraps this in try/catch with an alert suggesting lower resolution.
- **Large file sizes**: Mitigated by multi-format export — PNG is typically smallest for sparse bitmaps (e.g., 4.5MB vs JPEG 35MB at 15K).
- **Snap at compute-res**: Creates temp canvas at full compute resolution (momentary GPU use). Acceptable for one-shot export.
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
