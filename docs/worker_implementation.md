# Fast Mode: Multi-Worker Bitmap Rendering

Technical reference for the fast-mode subsystem in PolyPaint (`index.html`).

---

## Architecture Overview

Fast mode renders polynomial root trajectories onto a high-resolution bitmap canvas (up to 15,000x15,000px) using parallel Web Workers. Each pass simulates 1.0 second of virtual animation time, distributed across N workers that each handle a contiguous block of steps.

```
Pass (100K steps, 4 workers):

  Worker 0: steps [0, 25K)       ──→  sparse pixels_0
  Worker 1: steps [25K, 50K)     ──→  sparse pixels_1
  Worker 2: steps [50K, 75K)     ──→  sparse pixels_2
  Worker 3: steps [75K, 100K)    ──→  sparse pixels_3
                                        │
                                Main thread composites
                                all 4 onto existing canvas
                                        │
                                   advancePass()
                                        │
                              ┌─────────┴──────────┐
                              │                     │
                         more passes       jiggle boundary
                              │                     │
                       dispatchPass()   reinitWorkersForJiggle()
                                           generateJiggleOffsets()
                                           initFastModeWorkers()
```

Workers are **persistent** within a jiggle cycle: created once at init, reused across passes (only roots + elapsed offset change per pass). Terminated and recreated at jiggle boundaries (via `reinitWorkersForJiggle()`), or terminated when user exits fast mode.

---

## Global State

### Bitmap Canvas

| Variable | Type | Description |
|----------|------|-------------|
| `bitmapActive` | bool | Canvas initialized and ready |
| `bitmapCtx` | CanvasRenderingContext2D | Bitmap canvas context |
| `bitmapRange` | number | Frozen bounding-box radius (from roots panel at init time) |
| `bitmapFrameCount` | number | Frames painted since last clear |
| `bitmapCanvasColor` | string | Background color hex, default `"#000000"` |
| `bitmapCoeffView` | bool | When true, fast mode plots coefficient positions instead of roots |

### Pass & Cycle Tracking

| Variable | Type | Description |
|----------|------|-------------|
| `FAST_PASS_SECONDS` | const 1.0 | Virtual seconds per pass |
| `fastModeActive` | bool | Fast mode currently running |
| `fastModeElapsedOffset` | number | Accumulated virtual seconds across passes within one cycle |
| `fastModePassCount` | number | Completed passes in current cycle (resets at boundary) |
| `fastModeTargetPasses` | number | Passes per jiggle cycle (= `jiggleInterval`, since each pass = 1s). 0 = unlimited. |
| `fastModeCycleStartTime` | number | `performance.now()` when current cycle started |
| `fastModeTotalSteps` | number | Steps per pass (from dropdown: 10 to 1M) |
| `fastModeStepIndex` | number | Current step within pass (legacy fallback only) |

### Multi-Worker State

| Variable | Type | Description |
|----------|------|-------------|
| `numWorkers` | number | Worker count, 1-16. Default: `min(hardwareConcurrency, 16)` |
| `fastModeWorkers` | Worker[] | Persistent worker instances |
| `fastModeWorkersComplete` | number | Workers that reported "done" for current pass |
| `fastModeWorkerPixels` | object[] | Collected sparse pixel buffers from workers |
| `fastModeWorkerRoots` | {re, im} | Final root positions from last-range worker (warm start) |
| `fastModeCompositeBreakdown` | object\|null | Timing breakdown of compositeWorkerPixels phases |
| `fastModeWorkerProgress` | number[] | Per-worker step counts for progress aggregation |
| `fastModePassStartTime` | number | `performance.now()` at pass start |
| `fastModeTimingHistory` | object[] | Array of `{passMs, stepsPerSec, workers, steps}` |
| `fastModeSharedData` | object | Serialized data shared across all workers |

### Precomputed Data

| Variable | Type | Description |
|----------|------|-------------|
| `fastModeCurves` | Map\<int, curve\> | Hi-res curves for animated C-coefficients (N = stepsVal points) |
| `fastModeDCurves` | Map\<int, curve\> | Hi-res curves for animated D-node morph targets |
| `fastModeRootColors` | string[] | CSS color strings per root (legacy fallback and `paintBitmapFrameFast`) |

---

## Worker Code

Created by `createFastModeWorkerBlob()` as an inline blob URL. Contains three components:

### 1. Ehrlich-Aberth Solver (`solveEA` / `solveEA_wasm`)

Two solver implementations are available, selectable via the **cfg** button in the bitmap toolbar:

**JS solver** (`solveEA`): Flat-array implementation identical to the main-thread solver but tuned for throughput.

**WASM solver** (`solveEA_wasm`): Calls into a WebAssembly module compiled from C (`solver.c`). The WASM binary (~2KB) is base64-encoded and sent to workers during init. Eliminates JIT warmup, GC pauses, and leverages tighter register allocation for pure f64 arithmetic.

| Parameter | JS Worker | WASM Worker | Main Thread |
|-----------|-----------|-------------|-------------|
| Max iterations | 64 | 64 | 100 |
| Convergence threshold | 1e-16 (squared) | 1e-16 (squared) | 1e-12 (magnitude) |
| Leading-zero test | magnitude² < 1e-30 | magnitude² < 1e-30 | `Math.hypot` < 1e-15 |
| Hot-loop math | No `Math.hypot`, manual `d*d` | Native f64 ops | Uses `Math.hypot` |
| NaN rescue | In solver (isFinite) | Post-call JS (x !== x) | In solver |

Both solvers operate on flat `Float64Array` buffers for cache efficiency. The WASM solver copies data into/out of WASM linear memory (64KB) — negligible overhead relative to the O(n²) solver cost.

### 2. Root Matching (`matchRoots` / `hungarianMatch`)

Three strategies controlled by `S_matchStrategy`:

| Strategy | Function | Frequency | Complexity |
|----------|----------|-----------|------------|
| `"assign4"` (default) | `matchRoots()` greedy | Every 4th step | O(n²) |
| `"assign1"` | `matchRoots()` greedy | Every step | O(n²) |
| `"hungarian1"` | `hungarianMatch()` Kuhn-Munkres | Every step | O(n³) |

Matching is skipped entirely in uniform-color and proximity modes. In derivative mode, greedy matching runs every 4th step regardless of `S_matchStrategy`.

### 3. Message Handler (`self.onmessage`)

#### Persistent State (set once by "init")

```
S_coeffsRe, S_coeffsIm    Float64Array  Base coefficient values
S_nCoeffs, S_degree        int           Polynomial size
S_nRoots                   int           Root count (= degree)
S_colorsR/G/B              Uint8Array    Per-root RGB
S_W, S_H, S_range          int, number   Canvas dimensions and display range
S_curvesFlat               Float64Array  All curve points (re,im interleaved)
S_entries                  object[]      Animation entries [{idx, ccw, speed}]
S_offsets, S_lengths       int[]         Curve offset/length per entry
S_isCloud                  bool[]        Random-cloud flag per entry
S_useWasm                  bool          Use WASM solver (set from init message)
S_noColor                  bool          Uniform color mode
S_uniformR/G/B             int           Uniform color RGB
S_totalSteps, S_FPS        int, number   Steps per pass, seconds per pass
S_proxColor                bool          Proximity color mode
S_proxPalR/G/B             Uint8Array    16-entry proximity palette RGB
S_derivColor               bool          Derivative color mode
S_derivPalR/G/B            Uint8Array    16-entry derivative palette RGB
S_selIndices               int[]|null    Selected coefficient indices (for derivative mode)
S_morphEnabled             bool          Morph blending active
S_morphRate                number        Morph oscillation rate (Hz)
S_morphTargetRe/Im         Float64Array  Morph target coefficient values
S_matchStrategy            string        Root matching strategy ("assign4"|"assign1"|"hungarian1")
S_dCurvesFlat              Float64Array  D-node curve points (re,im interleaved)
S_dEntries                 object[]      D-node animation entries [{idx, ccw, speed}]
S_dOffsets, S_dLengths     int[]         D-curve offset/length per entry
S_dIsCloud                 bool[]        D-curve random-cloud flag per entry
S_jiggleRe, S_jiggleIm     Float64Array  Per-coefficient jiggle offsets (applied post-interpolation)
```

#### Per-Step Computation (within "run" handler)

For each step in `[stepStart, stepEnd)`:

1. **Compute virtual time**: `elapsed = elapsedOffset + (step / totalSteps) * FAST_PASS_SECONDS`

2. **Interpolate animated coefficients**: For each entry in `S_entries`:
   ```
   t = elapsed * speed * direction
   u = ((t % 1) + 1) % 1          // normalized phase [0, 1)
   rawIdx = u * curveLength
   ```
   - Cloud curves: snap to nearest point (no interpolation)
   - Smooth curves: linear interpolation between adjacent points

3. **Advance D-node curves** (if morph enabled): Same interpolation as step 2 but for `S_dEntries`/`S_dCurvesFlat`, updating `morphRe`/`morphIm` arrays

4. **Morph blend** (if `S_morphEnabled`): Sinusoidal blend `mu = 0.5 + 0.5 * sin(2π * morphRate * elapsed)` between C-coefficients and morph target D-coefficients

5. **Apply jiggle offsets**: If `S_jiggleRe`/`S_jiggleIm` present, add per-coefficient offsets post-interpolation

6. **Solve**: Call `solveEA()` (JS) or `solveEA_wasm()` (WASM) based on `S_useWasm` flag

7. **Match roots** (colored mode, per `S_matchStrategy`): Reorder roots to track identity

8. **Paint pixel**: Map root position to canvas coordinates:
   ```
   ix = ((rootRe / range + 1) * 0.5 * W) | 0
   iy = ((1 - rootIm / range) * 0.5 * H) | 0
   ```
   Append to sparse pixel arrays if in bounds.

9. **Report progress** every 2000 steps.

---

## Sparse Pixel Format

Workers avoid allocating W×H×4 buffers (a 10K×10K canvas = 400MB per worker). Instead:

| Array | Type | Content |
|-------|------|---------|
| `paintIdx` | Int32Array | Canvas pixel indices (row-major: `y * W + x`) |
| `paintR` | Uint8Array | Red channel |
| `paintG` | Uint8Array | Green channel |
| `paintB` | Uint8Array | Blue channel |
| `paintCount` | int | Actual number of pixels (≤ allocated max) |

Max allocation: `(stepEnd - stepStart) * nRoots` entries. Transferred back to main thread as `Transferable` buffers (zero-copy).

Compositing on main thread writes sparse pixels directly into a **persistent `ImageData` buffer** (no `getImageData` needed). Only the dirty rectangle region is flushed to canvas via `putImageData`. Pixel order is arbitrary (no blending, last-write-wins within overlapping workers).

---

## Message Formats

### init (main → worker, once per cycle)

```javascript
{
    type: "init",
    coeffsRe: ArrayBuffer,        // Float64Array, base coefficient real parts
    coeffsIm: ArrayBuffer,        // Float64Array, base coefficient imag parts
    nCoeffs: int,
    degree: int,
    nRoots: int,
    animEntries: [{idx, ccw, speed}, ...],
    curvesFlat: ArrayBuffer,       // Float64Array, all curve points re,im interleaved
    curveOffsets: [int, ...],      // start index in curvesFlat for each curve
    curveLengths: [int, ...],      // point count per curve
    curveIsCloud: [bool, ...],
    colorsR: ArrayBuffer,          // Uint8Array, one per root
    colorsG: ArrayBuffer,
    colorsB: ArrayBuffer,
    canvasW: int,
    canvasH: int,
    bitmapRange: number,
    noColor: bool,
    uniformR: int, uniformG: int, uniformB: int,
    proxColor: bool,               // proximity color mode
    proxPalR: ArrayBuffer,         // Uint8Array(16), proximity palette R
    proxPalG: ArrayBuffer,
    proxPalB: ArrayBuffer,
    derivColor: bool,              // derivative color mode
    derivPalR: ArrayBuffer,        // Uint8Array(16), derivative palette R
    derivPalG: ArrayBuffer,
    derivPalB: ArrayBuffer,
    selectedCoeffIndices: [int, ...],  // which coefficients for derivative sensitivity
    totalSteps: int,
    FAST_PASS_SECONDS: number,     // always 1.0
    useWasm: bool,                 // use WASM solver
    wasmB64: string|null,          // base64-encoded WASM binary (if useWasm)
    morphEnabled: bool,            // morph blending active
    morphRate: number,             // morph oscillation rate (Hz)
    morphTargetRe: ArrayBuffer,    // Float64Array, morph target real parts (or null)
    morphTargetIm: ArrayBuffer,    // Float64Array, morph target imag parts (or null)
    matchStrategy: string,         // "assign4"|"assign1"|"hungarian1"
    dAnimEntries: [{idx, ccw, speed}, ...],  // D-node animation entries
    dCurvesFlat: ArrayBuffer,      // Float64Array, D-node curve points
    dCurveOffsets: [int, ...],
    dCurveLengths: [int, ...],
    dCurveIsCloud: [bool, ...],
    jiggleRe: ArrayBuffer,         // Float64Array, per-coefficient jiggle offsets (or null)
    jiggleIm: ArrayBuffer,
}
```

Buffers are sent via `.buffer.slice(0)` (structured clone of copies). Workers store these as persistent state.

### run (main → worker, once per pass)

```javascript
{
    type: "run",
    workerId: int,                 // 0 to numWorkers-1
    stepStart: int,                // first step for this worker
    stepEnd: int,                  // exclusive upper bound
    elapsedOffset: number,         // accumulated virtual time from prior passes
    rootsRe: ArrayBuffer,          // Float64Array, warm-start root positions
    rootsIm: ArrayBuffer,
}
```

All workers receive the same root positions. Worker 0 has perfect warm-start continuity; other workers converge within 1-2 extra solver iterations on their first step.

### progress (worker → main, every 2000 steps)

```javascript
{
    type: "progress",
    workerId: int,
    step: int,                     // steps completed by this worker so far
    total: int,                    // total steps assigned to this worker
}
```

### done (worker → main, end of pass)

```javascript
{
    type: "done",
    workerId: int,
    paintIdx: ArrayBuffer,         // Int32Array, pixel indices
    paintR: ArrayBuffer,           // Uint8Array
    paintG: ArrayBuffer,
    paintB: ArrayBuffer,
    paintCount: int,               // actual pixel count
    rootsRe: ArrayBuffer,          // Float64Array, final root positions
    rootsIm: ArrayBuffer,
}
```

`paintIdx`, `paintR`, `paintG`, `paintB` are transferred (zero-copy). Root buffers are cloned (small). Only the last-range worker's roots are kept for the next pass's warm start.

---

## Lifecycle

### enterFastMode()

1. Stop interactive animation
2. Ensure bitmap canvas exists (init if needed, match resolution dropdown)
3. Read steps from `bitmap-steps-select` dropdown
4. **Precompute hi-res curves** for all animated coefficients:
   - `allAnimatedCoeffs()` returns Set of indices where `pathType !== "none"`
   - Snap coefficients to home position (`curve[0]`) temporarily for `coeffExtent()` calculation
   - For each animated coeff: `computeCurveN(home, pathType, absRadius, angle, extra, stepsVal)` (jiggle offsets are NOT baked into curves; they are applied post-interpolation in workers)
   - Store in `fastModeCurves` Map
5. **Precompute hi-res D-curves** for animated morph target coefficients (`allAnimatedDCoeffs()`), stored in `fastModeDCurves` Map
6. Precompute root colors (`rootColorRGB`)
7. Set `fastModeActive = true`, reset counters
8. Set `fastModeTargetPasses = jiggleInterval`
9. Show progress bar / pass counter if enabled
10. **If coefficient view**: plot curves directly via `plotCoeffCurvesOnBitmap()` (no workers needed)
11. **If no Worker support**: start legacy fallback (`setTimeout(fastModeChunkLegacy, 0)`)
12. **Otherwise**: serialize data → `initFastModeWorkers()` → first `dispatchPassToWorkers()`

### initFastModeWorkers()

For each of `min(numWorkers, stepsVal)` workers:
1. Create blob URL via `createFastModeWorkerBlob()`
2. `new Worker(blobUrl)`, then `URL.revokeObjectURL`
3. Set `onmessage` and `onerror` handlers
4. Send "init" message with all static data (buffer copies)
5. Push to `fastModeWorkers` array

Then immediately call `dispatchPassToWorkers()`.

### dispatchPassToWorkers()

1. Calculate `stepsPerWorker = ceil(stepsVal / numWorkers)`
2. Reset completion counter and pixel/progress arrays
3. Record `fastModePassStartTime`
4. For each worker: send "run" message with `{stepStart, stepEnd, elapsedOffset, rootsRe, rootsIm}`

### Pass completion (all workers done)

1. `compositeWorkerPixels()` — merge sparse pixels onto canvas
2. `recordPassTiming()` — push `{passMs, stepsPerSec, workers, steps}` to history
3. `advancePass()`:
   - `fastModeElapsedOffset += 1.0`, increment pass counter
   - Update pass counter UI
   - **Jiggle boundary check**: if `jiggleMode !== "none"` and `passCount >= targetPasses`:
     - `reinitWorkersForJiggle()`: terminate workers → `generateJiggleOffsets()` → re-serialize data → `initFastModeWorkers()` (elapsed offset continues, no exit/re-enter)
   - Otherwise: update warm-start roots in shared data → `dispatchPassToWorkers()`

### exitFastMode()

1. Proportionally advance elapsed offset for mid-pass interruption
2. Terminate all workers, clear arrays
3. Clear legacy fallback timer if set
4. Restore `currentRoots` from last worker snapshot (or legacy fallback roots)
5. Reset UI (remove `fast-mode` class, set button text to "cont", hide progress)
6. Call `renderCoefficients()` and `renderCoeffTrails()` to re-sync interactive display

---

## Cycle Management

### Jiggle Interval

`fastModeTargetPasses` is set directly from `jiggleInterval` (a user-configurable value, 1-100 seconds, shown in the cfg popup). Since each pass is 1.0 virtual seconds, the number of passes per jiggle cycle equals the interval in seconds.

The **PrimeSpeeds** transform (in the C-List/D-List Transform dropdown) sets speeds coprime to all others, forcing GCD=1 and maximum cycle diversity. The PS button is also available in per-coefficient path picker popups.

### Jiggle Integration

Jiggle offsets are applied **post-interpolation** in workers (not baked into precomputed curves). At each step, after coefficient interpolation and morph blending, `S_jiggleRe[i]`/`S_jiggleIm[i]` are added to each coefficient.

At jiggle boundary (`passCount >= targetPasses` and `jiggleMode !== "none"`):
1. `reinitWorkersForJiggle()` terminates current workers
2. `generateJiggleOffsets()` computes new perturbation offsets
3. Re-serialize data (new jiggle offsets included) → `initFastModeWorkers()` starts fresh workers
4. Elapsed offset continues (no reset), warm-start roots are preserved

No curve recomputation is needed since jiggle is applied post-interpolation.

---

## Data Serialization

`serializeFastModeData(animated, stepsVal, nRoots)` produces the shared data object:

### Coefficient Arrays
- `coeffsRe`, `coeffsIm`: Float64Array of current coefficient positions (base values, no jiggle applied)
- Jiggle offsets are sent separately as `jiggleRe`/`jiggleIm` and applied post-interpolation in workers

### Curve Data (flattened)
All animated curves concatenated into one `Float64Array`:
```
curvesFlat: [re0_p0, im0_p0, re0_p1, im0_p1, ..., re1_p0, im1_p0, ...]
```
With metadata arrays:
- `animEntries`: `[{idx: coeffIndex, ccw: bool, speed: number}, ...]`
- `curveOffsets`: start index (in points, not floats) for each curve
- `curveLengths`: point count per curve
- `curveIsCloud`: random-cloud flag per curve

Workers index into `curvesFlat` as: `base = offsets[a] * 2; curvesFlat[base + k*2]` for real part of point k.

### Root Warm Start
- Uses `fastModeWorkerRoots` from previous pass if available
- Falls back to `currentRoots` array for first pass

### Color Arrays
- `colorsR`, `colorsG`, `colorsB`: Uint8Array, one entry per root
- Computed from `d3.interpolateRainbow(i / nRoots)` (rainbow) or `bitmapUniformColor` (uniform)
- `noColor` flag: if true, worker uses `uniformR/G/B` for all roots
- `proxColor` flag + `proxPalR/G/B`: proximity palette (Uint8Array(16) each)
- `derivColor` flag + `derivPalR/G/B`: derivative palette (Uint8Array(16) each)
- `selectedCoeffIndices`: which coefficient indices to use for derivative sensitivity

### Morph & D-Curve Data
- `morphEnabled`, `morphRate`: morph blending configuration
- `morphTargetRe`, `morphTargetIm`: Float64Array of morph target positions
- D-curves serialized identically to C-curves: `dAnimEntries`, `dCurvesFlat`, `dCurveOffsets`, `dCurveLengths`, `dCurveIsCloud`

### Root Matching
- `matchStrategy`: `"assign4"` (default), `"assign1"`, or `"hungarian1"`

### Jiggle Offsets
- `jiggleRe`, `jiggleIm`: Float64Array, one entry per coefficient. Applied post-interpolation in workers. Zero for non-jiggled coefficients.

---

## UI Controls

### Bitmap Toolbar

| Control | ID | Function |
|---------|----|----------|
| Start/Restart | `bitmap-start-btn` | `initBitmapCanvas()`, enables other buttons |
| Save | `bitmap-save-btn` | PNG export with timestamp filename |
| Clear | `bitmap-clear-btn` | Fill canvas with bg color, reset `fastModeElapsedOffset` |
| Resolution | `bitmap-res-select` | 1000, 2000, 5000, 10000, 15000 px |
| Fast/imode | `bitmap-fast-btn` | Toggle `enterFastMode()` / `exitFastMode()` |
| ROOT / COEF | `bitmap-coeff-btn` | Toggle coefficient view (plot coefficients instead of roots) |
| Steps | `bitmap-steps-select` | 10, 100, 1K, 5K, 10K, 50K, 100K, 1M steps per pass |
| Jiggle | `jiggle-btn` | Opens jiggle configuration popup |
| Diagnostics | `jiggle-diag-btn` | Opens jiggle diagnostics popup (mode, offsets, positions) |
| Background | `bitmap-bg-btn` | Color picker for canvas background |
| Progress | `bitmap-progress-toggle` | Toggle progress bar visibility |

During fast mode, all controls except Fast/imode and R\|C are disabled (CSS `pointer-events: none`).

### Config Popup (C button)

Worker count selection: buttons for 1, 2, 4, 8, 16. Shows `navigator.hardwareConcurrency` as reference. Changing worker count clears timing history.

### Timing Popup (T button)

Displays after each pass:
- Last pass duration (seconds)
- Steps/sec throughput
- Worker count and steps/pass
- Pass progress (current/target)
- Cumulative time
- Average steps/sec across all passes

---

## Performance Characteristics

### Throughput

| Workers | Steps/sec (degree ~5) |
|---------|----------------------|
| 1 | ~2.15M |
| 4 | ~8M |
| 16 | ~15M |

Scaling is sub-linear due to: structured clone overhead, main-thread compositing, and uneven step distribution when `stepsVal` is not divisible by worker count.

### Bottlenecks

1. **Ehrlich-Aberth solver**: O(n² × iters) per step, where n = degree. Dominates at degree > 10. WASM solver reduces this by eliminating JIT warmup, GC pauses, and leveraging tighter f64 codegen — biggest gains at high degree.
2. **Root matching**: O(n²) per call (greedy) or O(n³) (Hungarian). Frequency depends on `matchStrategy`: every 4th step (default), every step, or Hungarian every step. Skipped in uniform and proximity modes.
3. **Compositing**: Persistent `ImageData` buffer with dirty-rect `putImageData`. After the persistent buffer optimization, compositing is fast (~5ms at 10K) and no longer the bottleneck. See [memory_timings.md](memory_timings.md).

### Memory

Per worker:
- Sparse pixel arrays: `(stepEnd - stepStart) * nRoots * 13 bytes` (Int32 index + 3 × Uint8 color)
- Coefficient/root arrays: `nCoeffs * 16 + nRoots * 16 bytes`
- Curve data: shared via structured clone, ~`totalCurvePoints * 16 bytes`

No per-worker W×H×4 canvas buffer. A 10K×10K canvas with 4 workers and 100K steps would use ~37MB per worker for pixel arrays at degree 29 (worst case), vs 400MB for a full canvas buffer.

### Tradeoffs

| Decision | Rationale |
|----------|-----------|
| Persistent workers (init once) | Avoids re-sending ~KB of curve data per pass |
| Sparse pixels over full buffers | 10K×10K canvas = 400MB per buffer. Sparse format is 13 bytes per painted pixel. |
| 64 solver iterations (not 100) | Warm start from previous step makes convergence fast. 64 iterations gives ample margin for high-degree polynomials. |
| Root matching every 4th step (default) | O(n²) matching costs ~25% of solver time at degree 20. Every-4th is a good tradeoff for color accuracy vs speed. Users can switch to every-step or Hungarian (O(n³)) for better accuracy at higher cost. |
| No SharedArrayBuffer | Requires COOP/COEP headers. GitHub Pages doesn't set them. Structured clone is fast enough for the small per-pass data (roots + step range). |
| No OffscreenCanvas | Useful for single worker but complex for multi-worker merging. Sparse pixel approach is simpler and equally fast. |
| All workers get same warm-start roots | Workers 1-N start with roots from time 0 (not their actual time). EA converges in 1-2 extra iters on first step. Negligible cost vs complexity of chaining roots across workers. |
| WASM solver optional (not default) | JS solver is already fast and requires no compilation toolchain. WASM provides marginal gains at low degree but significant gains at degree 50+. Users can toggle via cfg button. |
| WASM b64 sent per init (not shared) | Each worker decodes and compiles independently (~1ms). Avoids complexity of sharing compiled modules across workers. Only happens once per fast-mode session. |

---

## Legacy Fallback

For browsers without `Worker` support. Runs on main thread via `setTimeout(fastModeChunkLegacy, 0)`.

Chunk size: `max(10, floor(200 / max(3, degree)))` steps per setTimeout callback. This keeps each chunk under ~16ms to avoid blocking the UI event loop.

Uses the same animation interpolation and solver logic but calls `paintBitmapFrameFast()` (canvas `fillRect` 1x1 pixel writes) instead of sparse pixel buffers.

Jiggle boundary logic mirrors worker mode: when `jiggleMode !== "none"` and `passCount >= targetPasses`, reset pass count and call `generateJiggleOffsets()`. No curve recomputation needed since jiggle is applied post-interpolation. Unlike worker mode, the legacy fallback does not exit/re-enter fast mode at jiggle boundaries.

---

## Coefficient Animation in Workers

### Curve Precomputation

In `enterFastMode()`, for each coefficient with `pathType !== "none"`:
1. Home position = `curve[0]` (first point of interactive-mode curve)
2. `computeCurveN(home, pathType, absRadius, angle, extra, stepsVal)` generates N points (jiggle offsets are NOT baked in; they are applied post-interpolation in workers)
3. Stored in `fastModeCurves` Map

D-node curves are precomputed identically and stored in `fastModeDCurves` Map.

The curve resolution matches `stepsVal` (steps per pass), so each step maps to approximately one curve point. With speed < 1.0, the coefficient traverses only a fraction of the curve per pass.

### Worker Interpolation

At each step, for each animated entry:
```
elapsed = elapsedOffset + (step / totalSteps) * 1.0
t = elapsed * speed * direction
u = ((t % 1) + 1) % 1
rawIdx = u * curveLength
```

Linear interpolation between `curvesFlat[base + lo*2]` and `curvesFlat[base + hi*2]` (and +1 for imaginary parts). Cloud curves snap to nearest point without interpolation.

The formula is mathematically identical to the interactive-mode `animLoop()`.

---

## Root Color Modes

| Mode | Worker behavior |
|------|----------------|
| `"uniform"` | `noColor = true`. All roots painted with `uniformR/G/B`. No root matching needed. |
| `"rainbow"` | `noColor = false`. Each root uses `colorsR[i]/G[i]/B[i]`. Root matching every 4th step. |
| `"derivative"` | `derivColor = true`. Workers compute Jacobian sensitivity via `computeSens()` + `rankNorm()`. Color from 16-entry blue-white-red derivative palette. |
| `"proximity"` | `proxColor = true`. Workers compute min pairwise distance per root. Color from selectable 16-entry sequential palette (8 options: Inferno, Viridis, Magma, Plasma, Turbo, Cividis, Warm, Cool). |

Uniform mode is faster: no per-root color lookup, no root matching overhead.

---

## Coefficient View Mode

When `bitmapCoeffView` is true (toggled via ROOT/COEF button), fast mode plots **coefficient positions** on the bitmap canvas instead of solving for roots. This visualizes the actual coefficient curves and jiggle perturbations.

### How It Works

1. Curves are precomputed by `enterFastMode()` identically to normal mode (jiggle offsets are NOT baked in)
2. `plotCoeffCurvesOnBitmap(animated, stepsVal)` iterates all steps:
   - **Animated coefficients**: read position from `fastModeCurves.get(idx)[step % curveLength]`, then add jiggle offset if present
   - **Non-animated coefficients**: fixed at base position, then add jiggle offset if present
3. Each coefficient plotted as a **3×3 pixel square** using `coeffColor(i, n)` rainbow palette
4. Uses `ImageData` pixel writes for speed (no canvas arc calls)
5. `bitmapRange` is set to `panels.coeff.range` (coefficient panel viewport) instead of root panel range

### No Workers Needed

Coefficient plotting is pure coordinate projection — no Ehrlich-Aberth solver, no root matching. Runs entirely on the main thread. For 1M steps × 30 coefficients = 30M pixel writes, this completes in well under a second.

### Jiggle Cycling

When jiggle is active, `coeffJiggleCycle` runs via `setTimeout(fn, 0)`:
1. At jiggle boundary: `generateJiggleOffsets()` — compute new perturbation offsets (no curve recomputation needed since jiggle is applied post-interpolation)
2. Plot new curve points via `plotCoeffCurvesOnBitmap()` (accumulating on existing canvas, jiggle offsets applied per-step)
3. Schedule next cycle

The timer is stored in `fastModeTimerId`, so `exitFastMode()` cleanly stops the cycle.

### Interactive Root Suppression

When `bitmapCoeffView` is true, bitmap rendering only plots coefficient positions — root painting is suppressed.
