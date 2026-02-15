# Timing Display Fix + Fast Mode Resolution Performance Analysis

## Context

Two issues:
1. **Timing popup bug**: The "Passes" row in the T-button timing popup shows raw `fastModeTargetPasses` without rounding (e.g. `11379/7.142857142857143`). The progress bar counter was already fixed with `fmtPassCount()` but the timing popup was missed.
2. **Resolution-dependent slowdown**: Fast mode is significantly slower at high resolutions (10K×10K) than low (1K×1K) for the same polynomial, steps, and workers. The user wants a thorough analysis with measured data to make data-driven optimization decisions.

## File
`/Users/nicknassuphis/karpo_hackathon/index.html`

## Part 1: Fix Timing Popup (trivial)

**Line 5949** in `updateTimingPopup()`:
```javascript
// Before:
lines.push(["Passes", nPasses + "/" + fastModeTargetPasses]);
// After:
lines.push(["Passes", nPasses + "/" + Math.round(fastModeTargetPasses * 100) / 100]);
```

## Part 2: Resolution Performance Analysis

### Root Cause (from code analysis)

`compositeWorkerPixels()` (line 7931) is called once per pass and does:
1. `getImageData(0, 0, w, h)` — copies **entire** canvas pixel buffer from GPU→CPU: **O(W×H)**
2. Pixel write loop — writes only painted pixels via sparse indices: **O(pixels_painted)**
3. `putImageData(existing, 0, 0)` — copies **entire** buffer back CPU→GPU: **O(W×H)**

Memory per getImageData/putImageData call:
| Resolution | Pixels | Buffer size | get+put per pass |
|------------|--------|-------------|------------------|
| 1K×1K | 1M | 4 MB | 8 MB |
| 2K×2K | 4M | 16 MB | 32 MB |
| 5K×5K | 25M | 100 MB | 200 MB |
| 10K×10K | 100M | 400 MB | 800 MB |
| 15K×15K | 225M | 900 MB | 1.8 GB |

That's **100× more data** moved per pass at 10K vs 1K. The solver work (Ehrlich-Aberth, root matching) and pixel generation in workers is identical — it doesn't depend on canvas size.

### Experiment Plan

Add instrumentation to `compositeWorkerPixels()` to break down the three phases, plus capture the worker computation time separately. Display the breakdown in the timing popup.

**Instrumented `compositeWorkerPixels()`:**
```javascript
function compositeWorkerPixels() {
    if (!bitmapCtx) return;
    const w = bitmapCtx.canvas.width, h = bitmapCtx.canvas.height;

    const t0 = performance.now();
    const existing = bitmapCtx.getImageData(0, 0, w, h);
    const t1 = performance.now();

    const out = existing.data;
    for (const result of fastModeWorkerPixels) {
        if (!result) continue;
        const idx = new Int32Array(result.paintIdx);
        const r = new Uint8Array(result.paintR);
        const g = new Uint8Array(result.paintG);
        const b = new Uint8Array(result.paintB);
        const count = result.paintCount;
        for (let i = 0; i < count; i++) {
            const off = idx[i] * 4;
            out[off] = r[i]; out[off + 1] = g[i];
            out[off + 2] = b[i]; out[off + 3] = 255;
        }
    }
    const t2 = performance.now();

    bitmapCtx.putImageData(existing, 0, 0);
    const t3 = performance.now();

    fastModeCompositeBreakdown = {
        getMs: t1 - t0,
        writeMs: t2 - t1,
        putMs: t3 - t2,
        totalMs: t3 - t0
    };
    fastModeWorkerPixels = [];
}
```

**New state variable:** `let fastModeCompositeBreakdown = null;`

**Record worker computation time:** Capture `performance.now()` when pass starts (already `fastModePassStartTime`) and when last worker message arrives (before `compositeWorkerPixels`). Add to timing record:
```javascript
// In handleFastModeWorkerMessage, when all workers complete:
const workerMs = performance.now() - fastModePassStartTime;
compositeWorkerPixels();
// workerMs = time workers ran; composite breakdown captured inside
```

**Update `recordPassTiming()`** to include breakdown:
```javascript
function recordPassTiming() {
    const ms = performance.now() - fastModePassStartTime;
    fastModeTimingHistory.push({
        passMs: ms,
        stepsPerSec: Math.round(fastModeTotalSteps / (ms / 1000)),
        workers: fastModeWorkers.length,
        steps: fastModeTotalSteps,
        composite: fastModeCompositeBreakdown  // NEW
    });
    updateTimingPopup();
}
```

**Update `updateTimingPopup()`** to display breakdown after existing rows:
```javascript
if (last.composite) {
    const c = last.composite;
    lines.push(["─ getImageData", c.getMs.toFixed(1) + "ms"]);
    lines.push(["─ pixel writes", c.writeMs.toFixed(1) + "ms"]);
    lines.push(["─ putImageData", c.putMs.toFixed(1) + "ms"]);
    lines.push(["─ composite", c.totalMs.toFixed(1) + "ms"]);
}
```

### Analysis Document

Create `/Users/nicknassuphis/karpo_hackathon/docs/memory_timings.md` with:

1. **Problem statement** — same steps/roots, resolution affects speed
2. **Architecture diagram** — data flow: workers → structured clone → compositeWorkerPixels → canvas
3. **Theoretical analysis** — memory table above, O(W×H) vs O(pixels) operations
4. **Measured data** — table from running instrumented code at each resolution (1K, 2K, 5K, 10K, 15K) with same polynomial/steps/workers. Columns: resolution, total pass time, worker time, getImageData, pixel writes, putImageData, composite total
5. **Breakdown analysis** — what % of total time is composite at each resolution
6. **Optimization options** with pros/cons:
   - **Dirty rect tracking**: Track bounding box of painted pixels, only get/putImageData for that region. Pro: simple, huge win when roots cluster. Con: minimal win when roots spread across full canvas.
   - **Direct canvas draws**: Replace getImageData+putImageData with `fillRect()` per pixel. Pro: no full-buffer copy. Con: thousands of draw calls have overhead; need to benchmark.
   - **Accumulate into JS buffer**: Keep a persistent `Uint8ClampedArray(W×H×4)` in JS, write pixels there, putImageData once at end. Pro: eliminates getImageData. Con: still O(W×H) putImageData; doubles memory.
   - **Partial putImageData via dirty rect**: Same persistent buffer but `putImageData(imgData, 0, 0, dirtyX, dirtyY, dirtyW, dirtyH)`. Pro: reduces put cost to dirty region. Con: still need full buffer in memory.
   - **WebGL rendering**: Upload sparse pixels as texture updates. Pro: GPU-native, very fast. Con: major rewrite.
7. **Recommendation** — based on measured data

### Copy Button in Timing Popup

Add a small "Copy" button at the bottom of the timing popup that copies all timing history as JSON to clipboard:
```javascript
// In updateTimingPopup(), after the grid:
const copyBtn = document.createElement("button");
copyBtn.textContent = "Copy";
copyBtn.style.cssText = "margin-top:6px;font-size:9px;padding:2px 8px;cursor:pointer;background:var(--bg);color:var(--accent);border:1px solid var(--accent-border);border-radius:4px;";
copyBtn.addEventListener("click", function() {
    const data = {
        resolution: bitmapCtx ? bitmapCtx.canvas.width : null,
        workers: fastModeWorkers.length,
        passes: fastModeTimingHistory.map(h => ({
            passMs: h.passMs,
            stepsPerSec: h.stepsPerSec,
            steps: h.steps,
            composite: h.composite || null
        }))
    };
    navigator.clipboard.writeText(JSON.stringify(data, null, 2));
    copyBtn.textContent = "Copied!";
    setTimeout(() => copyBtn.textContent = "Copy", 1000);
});
```

### Data Collection Procedure

After adding instrumentation:
1. Load a polynomial (e.g., degree 5 with circle paths)
2. Set workers to 4, steps to 100K
3. For each resolution (1K, 2K, 5K, 10K, 15K): start bitmap → run 3+ passes → click Copy in timing popup → paste to me
4. I'll compile the data into `docs/memory_timings.md`

## Implementation Order

1. Fix timing popup rounding (line 5949) — 1 line
2. Add `fastModeCompositeBreakdown` state variable
3. Instrument `compositeWorkerPixels()` with performance.now() breakdown
4. Capture worker computation time in `handleFastModeWorkerMessage`
5. Update `recordPassTiming()` to include composite breakdown
6. Update `updateTimingPopup()` to display breakdown rows
7. Create `docs/memory_timings.md` with theoretical analysis + placeholder tables for measured data
8. Run experiments at 1K/2K/5K/10K/15K (user opens app, runs passes, records numbers)
9. Fill in measured data and complete analysis document

## Verification

1. Open timing popup → "Passes" row shows rounded target (e.g., `3/7.14` not `3/7.142857...`)
2. Run fast mode at 2K → timing popup shows composite breakdown (getImageData/writes/putImageData ms)
3. Compare 1K vs 10K at same steps → composite time dominates at 10K
4. `docs/memory_timings.md` contains coherent analysis with data
