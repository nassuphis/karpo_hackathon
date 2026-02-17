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
                         │                       1. getImageData(0,0,W,H)  ← O(W×H)
                         │                       2. write sparse pixels    ← O(painted)
                         │                       3. putImageData(0,0,W,H)  ← O(W×H)
                         │                              │
                         └──────────────────────────────▼
                                                  Canvas updated
```

**Worker computation** (solver + root matching + pixel generation) does NOT depend on canvas resolution. Workers produce sparse index arrays sized to `steps × nRoots`, regardless of whether the canvas is 1K or 15K.

**Compositing** calls `getImageData` and `putImageData` on the **entire** canvas, even though only a small fraction of pixels are modified. These are the resolution-dependent bottleneck.

---

## Theoretical Analysis

### Memory per getImageData/putImageData

Each call copies a `Uint8ClampedArray` of size `W × H × 4` bytes between CPU and GPU memory.

| Resolution | Pixels | Buffer Size | get + put per pass |
|------------|--------|-------------|-------------------|
| 1,000 | 1M | 4 MB | 8 MB |
| 2,000 | 4M | 16 MB | 32 MB |
| 5,000 | 25M | 100 MB | 200 MB |
| 10,000 | 100M | 400 MB | 800 MB |
| 15,000 | 225M | 900 MB | 1.8 GB |

Going from 1K to 10K is **100x more data** transferred per pass. Going from 1K to 15K is **225x**.

### What scales with resolution

| Operation | Cost | Depends on resolution? |
|-----------|------|----------------------|
| Ehrlich-Aberth solver | O(degree² × iters × steps) | No |
| Root matching | O(degree² × steps/4) | No |
| Sparse pixel generation (workers) | O(steps × degree) | No (bounds check only) |
| Structured clone transfer | O(steps × degree × 13 bytes) | No |
| `getImageData(0,0,W,H)` | O(W × H × 4) | **Yes** |
| Pixel write loop | O(painted_pixels) | No |
| `putImageData(0,0,W,H)` | O(W × H × 4) | **Yes** |

---

## Measured Data

### Test Setup

- Polynomial: degree 29 (29 roots × 1000 steps = 29,000 pixels/pass)
- Workers: 16
- Steps/pass: 1,000
- Jiggle: active (produces periodic spike passes at cycle boundaries)
- Machine: Apple Silicon Mac

### Results (steady-state medians, excluding jiggle spike passes)

| Resolution | Total pass | Workers | getImageData | Pixel writes | putImageData | Composite | Comp % |
|------------|-----------|---------|-------------|-------------|-------------|-----------|--------|
| 1,000 | **4.0 ms** | 2.9 ms | 0.8 ms | 0.1 ms | 0.2 ms | **1.1 ms** | **28%** |
| 2,000 | **7.9 ms** | 3.6 ms | 3.6 ms | 0.1 ms | 0.6 ms | **4.3 ms** | **54%** |
| 5,000 | **34.0 ms** | 3.3 ms | 22.4 ms | 1.4 ms | 5.6 ms | **30.5 ms** | **90%** |
| 10,000 | **125.0 ms** | 8.4 ms | 89.4 ms | 1.6 ms | 24.4 ms | **116.4 ms** | **93%** |

### Scaling Ratios (relative to 1K)

| Metric | 2K (4x px) | 5K (25x px) | 10K (100x px) |
|--------|-----------|------------|--------------|
| `getImageData` | 4.5x | 28x | **112x** |
| `putImageData` | 3x | 28x | **122x** |
| Composite total | 3.9x | 27.7x | **106x** |
| Worker computation | 1.2x | 1.1x | 2.9x |
| Total pass | 2.0x | 8.5x | **31x** |

### Key Findings

1. **`getImageData` is the dominant bottleneck**: 89ms at 10K, consuming 72% of the total pass time by itself. It copies the entire 400MB canvas buffer from GPU → CPU memory every single pass.

2. **`putImageData` is second**: 24ms at 10K (20% of pass time). Also copies the full 400MB buffer back CPU → GPU.

3. **Composite scales exactly as theory predicts**: ~100x for 100x more pixels. The measured ratios (112x for get, 122x for put) match the O(W×H) cost model.

4. **Worker computation is nearly resolution-independent**: 2.9ms → 3.6ms → 3.3ms → 8.4ms. The small increase at 10K is likely from memory pressure or GC, not from resolution-dependent work.

5. **At 5K+, over 90% of pass time is wasted on full-canvas memcpy** for a pass that only paints 29,000 pixels (0.03–0.12% of the canvas).

6. **The crossover point is ~2K**: below 2K, workers and composite are roughly equal; above 2K, composite dominates and grows linearly with pixel count.

---

## Optimization Options

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

### Option 3: Persistent Buffer + Dirty Rect putImageData (recommended)

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

## Recommendation

**Option 3 (Persistent Buffer + Dirty Rect putImageData)** is the clear winner:

1. **Eliminates getImageData entirely** (the 89ms bottleneck — 72% of pass time at 10K)
2. **Reduces putImageData** to dirty region only (potentially 10-100x smaller than full canvas)
3. **No worker changes** — workers already produce sparse pixel data; dirty rect is computed from pixel indices on main thread
4. **Moderate complexity** — main changes are in `initBitmapCanvas()` (create persistent ImageData), `compositeWorkerPixels()` (write to persistent buffer, track dirty rect, partial putImageData), and clear button (fill buffer with bg color)
5. **Memory cost is acceptable** — 400MB for a 10K persistent buffer is the same allocation that `getImageData` was creating and discarding every pass

Expected improvement:
- **1K**: 4.0ms → ~3.0ms (1.3x, already fast)
- **5K**: 34ms → ~5-10ms (**3-7x speedup**)
- **10K**: 125ms → ~10-30ms (**4-12x speedup** depending on root spread)

The bottleneck would shift back to worker computation, which is the correct behavior — resolution should not significantly affect pass time.

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

---

## Appendix: Code Locations

| Component | File | Lines |
|-----------|------|-------|
| `compositeWorkerPixels()` | index.html | ~9946 |
| `fillPersistentBuffer()` | index.html | ~8722 |
| `initBitmapCanvas()` | index.html | ~8842 |
| `recordPassTiming()` | index.html | ~10030 |
| `updateTimingPopup()` | index.html | ~7394 |
| Worker pixel generation | index.html (blob) | ~9273-9450 |
| `bitmapPersistentBuffer` state | index.html | ~1084 |
