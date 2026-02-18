# Fast Mode: Resolution vs Performance Analysis

Why does fast mode slow down at higher canvas resolutions when the polynomial computation is identical?

---

## Architecture: Data Flow Per Pass

```
Workers (parallel)                    Main Thread
────────────────                      ──────────────
Ehrlich-Aberth solver  ──┐
  × steps_per_worker     │
Root matching (every 4)  │──→ structured clone ──→ handleFastModeWorkerMessage
Sparse pixel gen         │    (paintIdx, R,G,B)         │
                         │                              ▼
                         │                     compositeWorkerPixels()
                         │                       1. write sparse pixels to   ← O(painted)
                         │                          persistent ImageData
                         │                       2. if split: downsample to  ← O(painted)
                         │                          display buffer
                         │                       3. dirty-rect putImageData  ← O(dirty_rect)
                         │                              │
                         └──────────────────────────────▼
                                                  Canvas updated
```

**Worker computation** (solver + root matching + pixel generation) does NOT depend on canvas resolution. Workers produce sparse index arrays sized to `steps × nRoots`, regardless of whether the canvas is 1K or 15K.

**Compositing** writes sparse pixels into a persistent `ImageData` buffer at **compute resolution** (no `getImageData` needed), then calls `putImageData` with a **dirty rect** covering only the painted region. When compute resolution exceeds `BITMAP_DISPLAY_CAP` (2000px), the canvas element stays at display resolution and a downsampled display buffer is used for `putImageData`.

---

## Off-Canvas Render Split

When the user selects a resolution above 2000px, the system splits into two layers:

| Layer | Resolution | Purpose | Storage |
|-------|-----------|---------|---------|
| Persistent buffer (`bitmapPersistentBuffer`) | `bitmapComputeRes` (from dropdown) | Full-fidelity pixel accumulation, export | CPU-only `ImageData` |
| Display buffer (`bitmapDisplayBuffer`) | `bitmapDisplayRes` = min(computeRes, 2000) | On-screen preview | CPU-only `ImageData` |
| Canvas element | `bitmapDisplayRes` | GPU-backed display | `<canvas>` |

When compute ≤ 2000, `bitmapDisplayBuffer` is null and the persistent buffer is used directly for both accumulation and display.

### Compositing with split

In `compositeWorkerPixels()`:
1. Sparse pixels are written into `bitmapPersistentBuffer` at compute resolution
2. If split is active, each pixel is also downsampled (`invScale = dW / cW`) into `bitmapDisplayBuffer`
3. Dirty rect is tracked in **display-space** coordinates
4. `putImageData` is called on whichever buffer matches the canvas: `bitmapDisplayBuffer` when split, `bitmapPersistentBuffer` otherwise

### Export at full resolution

Bitmap export reads directly from `bitmapPersistentBuffer` at compute resolution, bypassing the display canvas entirely. This means a 15K bitmap export produces a 15K image even though the on-screen canvas is only 2000px.

---

## Worker Partitioning

Steps are distributed evenly across workers in `dispatchPassToWorkers()`:

```javascript
const actualWorkers = Math.min(numWorkers, sd.stepsVal);
// ...
const base = Math.floor(stepsVal / nw);
const extra = stepsVal % nw;
for (let w = 0; w < nw; w++) {
    const count = base + (w < extra ? 1 : 0);
    // worker w processes steps [offset, offset + count)
}
```

Key detail: `actualWorkers = Math.min(numWorkers, stepsVal)` ensures we never spawn more workers than steps. At 10 steps with 16 configured workers, only 10 workers are created (1 step each). At 10K steps with 16 workers, each gets ~625 steps.

Each worker's `maxPaintsPerWorker` buffer is sized to `ceil(stepsVal / actualWorkers) * nRoots`, guaranteeing sufficient space for its pixel output.

---

## Dropdown Change Listeners

The steps and resolution dropdowns trigger a full restart of fast mode when changed mid-run:

```javascript
document.getElementById("bitmap-steps-select").addEventListener("change", function () {
    if (fastModeActive) { exitFastMode(); enterFastMode(); }
});
document.getElementById("bitmap-res-select").addEventListener("change", function () {
    if (fastModeActive) { exitFastMode(); enterFastMode(); }
});
```

`exitFastMode()` terminates all workers and clears state. `enterFastMode()` re-reads both dropdowns, reinitializes the bitmap canvas (rebuilding persistent/display buffers at the new resolution), recomputes curves at the new step count, and spawns fresh workers.

---

## Theoretical Analysis

### Memory per putImageData (dirty rect)

With the persistent buffer, only `putImageData` is called (no `getImageData`). The dirty rect limits the copy to the bounding box of painted pixels.

| Resolution | Full buffer | Dirty rect (10% area) | Dirty rect (1% area) |
|------------|------------|----------------------|---------------------|
| 1,000 | 4 MB | 0.4 MB | 0.04 MB |
| 2,000 | 16 MB | 1.6 MB | 0.16 MB |
| 5,000 | 100 MB | 10 MB | 1 MB |
| 10,000 | 400 MB | 40 MB | 4 MB |
| 15,000 | 900 MB | 90 MB | 9 MB |

Note: resolutions above `BITMAP_DISPLAY_CAP` (2000) only put the **display buffer** (max 16 MB), not the full compute buffer. The persistent buffer at compute resolution is CPU-only and never sent to the GPU via `putImageData`.

### What scales with resolution

| Operation | Cost | Depends on resolution? |
|-----------|------|----------------------|
| Ehrlich-Aberth solver | O(degree² × iters × steps) | No |
| Root matching | O(degree² × steps/4) | No |
| Sparse pixel generation (workers) | O(steps × degree) | No (bounds check only) |
| Structured clone transfer | O(steps × degree × 13 bytes) | No |
| Pixel write to persistent buffer | O(painted_pixels) | No |
| Downsample to display buffer (split only) | O(painted_pixels) | No |
| `putImageData` (dirty rect) | O(dirty_W × dirty_H × 4) | Partially (dirty rect size) |

---

## Measured Data

### Test Setup

- Polynomial: degree 29 (29 roots × 1000 steps = 29,000 pixels/pass)
- Workers: 16
- Steps/pass: 1,000
- Jiggle: active (produces periodic spike passes at cycle boundaries)
- Machine: Apple Silicon Mac

### Pre-Optimization Results (with getImageData, steady-state medians)

| Resolution | Total pass | Workers | getImageData | Pixel writes | putImageData | Composite | Comp % |
|------------|-----------|---------|-------------|-------------|-------------|-----------|--------|
| 1,000 | **4.0 ms** | 2.9 ms | 0.8 ms | 0.1 ms | 0.2 ms | **1.1 ms** | **28%** |
| 2,000 | **7.9 ms** | 3.6 ms | 3.6 ms | 0.1 ms | 0.6 ms | **4.3 ms** | **54%** |
| 5,000 | **34.0 ms** | 3.3 ms | 22.4 ms | 1.4 ms | 5.6 ms | **30.5 ms** | **90%** |
| 10,000 | **125.0 ms** | 8.4 ms | 89.4 ms | 1.6 ms | 24.4 ms | **116.4 ms** | **93%** |

### Scaling Ratios (relative to 1K, pre-optimization)

| Metric | 2K (4x px) | 5K (25x px) | 10K (100x px) |
|--------|-----------|------------|--------------|
| `getImageData` | 4.5x | 28x | **112x** |
| `putImageData` | 3x | 28x | **122x** |
| Composite total | 3.9x | 27.7x | **106x** |
| Worker computation | 1.2x | 1.1x | 2.9x |
| Total pass | 2.0x | 8.5x | **31x** |

### Key Pre-Optimization Findings

1. **`getImageData` was the dominant bottleneck**: 89ms at 10K, consuming 72% of the total pass time by itself. It copied the entire 400MB canvas buffer from GPU → CPU memory every single pass.

2. **`putImageData` was second**: 24ms at 10K (20% of pass time). Also copied the full 400MB buffer back CPU → GPU.

3. **Composite scaled exactly as theory predicted**: ~100x for 100x more pixels. The measured ratios (112x for get, 122x for put) match the O(W×H) cost model.

4. **Worker computation was nearly resolution-independent**: 2.9ms → 3.6ms → 3.3ms → 8.4ms. The small increase at 10K was likely from memory pressure or GC, not from resolution-dependent work.

5. **At 5K+, over 90% of pass time was wasted on full-canvas memcpy** for a pass that only paints 29,000 pixels (0.03–0.12% of the canvas).

6. **The crossover point is ~2K**: below 2K, workers and composite are roughly equal; above 2K, composite dominates and grows linearly with pixel count.

---

## Optimization Options (Historical)

These options were evaluated before implementing the solution. Option 3 was selected and implemented.

### Option 1: Persistent JS Pixel Buffer (eliminates getImageData)

**Idea**: Keep a permanent `ImageData` object in JavaScript. Write sparse pixels directly into its `.data` array. Call only `putImageData` to flush to canvas (skip `getImageData` entirely).

**Projected savings at 10K**: Eliminates 89ms getImageData. Pass drops from 125ms → ~36ms (**3.5x speedup**).

**Cost**: `putImageData` still O(W×H) at 24ms. Buffer uses W×H×4 bytes of JS heap (400MB at 10K).

**Complexity**: Low. Create `ImageData` once at `initBitmapCanvas()`. Clear = fill `.data` with bg color bytes. No worker changes needed.

### Option 2: Dirty Rectangle Tracking (reduces get/put region)

**Idea**: Track the bounding box of painted pixels across all workers. Call `getImageData`/`putImageData` only for the dirty region.

**Projected savings at 10K**: Depends on root spread. If roots cluster in 10% of canvas area, composite drops ~10x.

**Cost**: Workers need to send min/max pixel coordinates alongside sparse data. Main thread computes union of dirty rects.

**Complexity**: Medium. Good for clustered roots, minimal win for spread-out roots.

### Option 3: Persistent Buffer + Dirty Rect putImageData (implemented)

**Idea**: Combine options 1 and 2. Keep persistent `ImageData` buffer, use `putImageData(imgData, 0, 0, dirtyX, dirtyY, dirtyW, dirtyH)` to only flush the dirty region.

**Projected savings at 10K**: Eliminates 89ms getImageData entirely. Reduces putImageData from 24ms to proportional to dirty rect size. If roots use 10% of canvas: pass drops from 125ms → ~11ms (**11x speedup**).

**Cost**: Persistent W×H×4 buffer in JS heap (400MB at 10K). Workers send bounding boxes.

**Complexity**: Medium. Best theoretical performance of the non-WebGL options.

### Option 4: Canvas fillRect per pixel (no buffer at all)

**Idea**: Instead of getImageData/putImageData, call `ctx.fillRect(x, y, 1, 1)` for each painted pixel.

**Projected savings at 10K**: No full-canvas memcpy. 29K fillRect calls per pass.

**Cost**: Canvas draw calls have per-call overhead (~1-5μs each). 29K calls × 3μs = ~87ms — possibly comparable to current approach. Would need benchmarking.

**Complexity**: Low code change, but performance is uncertain and likely worse at high pixel counts.

### Option 5: WebGL Rendering

**Idea**: Use WebGL to render the bitmap. Upload sparse pixel data as texture updates via `texSubImage2D`.

**Projected savings at 10K**: Everything stays on GPU. No CPU↔GPU round-trip for full canvas.

**Cost**: Major architectural change. WebGL setup, shaders, framebuffer management, PNG export changes.

**Complexity**: High. Would need to rewrite bitmap rendering subsystem.

---

## Post-Optimization Results (Option 3 Implemented)

### 10K Before vs After (steady-state medians, excluding jiggle spikes)

| Metric | Before | After | Speedup |
|--------|--------|-------|---------|
| **Total pass** | 125 ms | **8.7 ms** | **14.4x** |
| `getImageData` | 89.4 ms | **0 ms** | eliminated |
| `putImageData` | 24.4 ms | **5.0 ms** | **4.9x** |
| Composite total | 116.4 ms | **5.2 ms** | **22.4x** |
| Workers | 8.4 ms | **3.2 ms** | 2.6x |

### Key Observations

1. **getImageData eliminated**: 0ms across all passes. The persistent buffer completely removes the GPU→CPU full-canvas copy.

2. **putImageData reduced ~5x**: Dirty rect limits the CPU→GPU copy to ~20% of canvas area for this polynomial's root spread.

3. **Workers got faster**: 8.4ms → 3.2ms. Eliminating the per-pass 400MB `ImageData` allocation reduced GC pressure.

4. **Bottleneck shifted to workers**: Composite is now 5.2ms vs workers at 3.2ms — they're roughly equal, which is the correct behavior. Resolution no longer dominates pass time.

5. **Occasional putImageData spikes**: One pass showed putMs of 23.7ms (full-canvas-like), likely from a pass where roots spread across the entire canvas. This is expected and matches the theory — dirty rect only helps when roots cluster.

6. **14.4x total speedup exceeded the 4-12x projection** due to the combined effect of eliminating getImageData + dirty rect + reduced GC pressure on workers.

### Off-Canvas Render Split Impact

For resolutions above 2000px, the off-canvas split provides an additional benefit: `putImageData` always operates on the **display buffer** (max 2000×2000 = 16MB), never the full compute buffer. This means:

- At 5K compute: `putImageData` flushes at most 16MB (display buffer) instead of 100MB
- At 10K compute: `putImageData` flushes at most 16MB instead of 400MB
- At 15K compute: `putImageData` flushes at most 16MB instead of 900MB

The compute-resolution persistent buffer stays in JS heap (CPU-only) and is never sent to the GPU during compositing. It is only used for full-resolution export.

---

## Appendix: Code Locations

| Component | File | Lines |
|-----------|------|-------|
| `compositeWorkerPixels()` | index.html | ~10384 |
| `fillPersistentBuffer()` | index.html | ~8853 |
| `fillDisplayBuffer()` | index.html | ~8868 |
| `initBitmapCanvas()` | index.html | ~8973 |
| `serializeFastModeData()` | index.html | ~10116 |
| `initFastModeWorkers()` | index.html | ~10250 |
| `dispatchPassToWorkers()` | index.html | ~10316 |
| `recordPassTiming()` | index.html | ~10468 |
| `updateTimingPopup()` | index.html | ~7517 |
| Worker pixel generation | index.html (blob) | ~9273–9450 |
| `bitmapPersistentBuffer` state | index.html | ~1085 |
| `BITMAP_DISPLAY_CAP` constant | index.html | ~1086 |
| `bitmapComputeRes` / `bitmapDisplayRes` | index.html | ~1087–1088 |
| `bitmapDisplayBuffer` state | index.html | ~1089 |
| Steps/resolution dropdown listeners | index.html | ~12435–12441 |
| `resetBitmap()` | index.html | ~10714 |
