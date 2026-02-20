# Fast Mode: Multi-Worker Bitmap Rendering

Technical reference for the fast-mode subsystem in PolyPaint (`index.html`).

---

## Architecture Overview

Fast mode renders polynomial root trajectories onto a high-resolution bitmap canvas (up to 15,000x15,000px) using parallel Web Workers. Each pass simulates 1.0 second of virtual animation time, distributed across N workers that each handle a contiguous block of steps.

```
Pass (100K steps, 4 workers — balanced distribution):

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
                                           computeJiggleForStep()
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
| `fastModeTimingHistory` | object[] | Array of `{passMs, stepsPerSec, workers, steps, workerMs, composite}` |
| `fastModeSharedData` | object | Serialized data shared across all workers |

### Precomputed Data

| Variable | Type | Description |
|----------|------|-------------|
| `fastModeCurves` | Map\<int, curve\> | Hi-res curves for animated C-coefficients (N = stepsVal points) |
| `fastModeDCurves` | Map\<int, curve\> | Hi-res curves for animated D-node morph targets |
| `fastModeRootColors` | string[] | CSS color strings per root (legacy fallback and `paintBitmapFrameFast`) |

---

## Worker Code

Created by `createFastModeWorkerBlob()` (~line 9907) as an inline blob URL. Contains the following components:

### 1. Ehrlich-Aberth Solver (two-tier selection)

Two solver tiers are available, selectable via the **cfg** button in the bitmap toolbar. Workers try the WASM tier first and fall back automatically:

1. **WASM step loop** (`S_useWasmLoop`): The entire per-step loop (coefficient interpolation, D-curve advance, follow-C, morph blend, jiggle, solver, root matching, pixel painting) runs in a single WASM module (`step_loop.c`). The binary is base64-encoded as `WASM_STEP_LOOP_B64` (~line 1136) and sent via `wasmStepLoopB64` in the init message. `initWasmStepLoop(d)` (~line 10200) decodes, compiles, allocates WASM linear memory via `computeWasmLayout()`, and copies all persistent state into flat memory buffers. The `runStepLoop(stepStart, stepEnd, elapsedOffset)` export executes the full step range and returns the pixel count directly. Progress is reported via an imported `reportProgress` callback.

2. **JS solver** (`solveEA`): Pure JavaScript flat-array implementation (~line 9911). Used when WASM is disabled or unavailable.

| Parameter | JS Worker | WASM (step loop) | Main Thread |
|-----------|-----------|---------------------|-------------|
| Max iterations | 64 | 64 | 100 |
| Convergence threshold | 1e-16 (squared) | 1e-16 (squared) | 1e-12 (magnitude) |
| Leading-zero test | magnitude² < 1e-30 | magnitude² < 1e-30 | `Math.hypot` < 1e-15 |
| Hot-loop math | No `Math.hypot`, manual `d*d` | Native f64 ops | Uses `Math.hypot` |
| NaN rescue | In solver (isFinite) | Post-call JS (x !== x) | In solver |

All solvers operate on flat `Float64Array` buffers for cache efficiency. The WASM step loop allocates all buffers in WASM linear memory via `computeWasmLayout()` (~line 10145), which computes byte offsets for coefficients, roots, curves, palettes, follow-C indices, pixel output, and working arrays. Memory is grown to fit (`L.pages` pages).

**WASM step loop fallback rule**: If `S_idxProxColor` or `S_ratioColor` is true, the WASM step loop is forced off (~line 10410) and the JS step loop is used instead, because these color modes are not implemented in `step_loop.c`.

### 2. Root Matching (`matchRoots` / `hungarianMatch`)

Three strategies controlled by `S_matchStrategy`:

| Strategy | Function | Frequency | Complexity |
|----------|----------|-----------|------------|
| `"assign4"` (default) | `matchRoots()` greedy (~line 9974) | Every 4th step | O(n²) |
| `"assign1"` | `matchRoots()` greedy | Every step | O(n²) |
| `"hungarian1"` | `hungarianMatch()` Kuhn-Munkres (~line 9990) | Every step | O(n³) |

Matching is skipped entirely in uniform-color and proximity modes. In derivative mode, greedy matching runs every 4th step regardless of `S_matchStrategy`. In idx-prox mode, matching follows the user-selected `S_matchStrategy` (same as rainbow).

### 3. Derivative Sensitivity (`computeSens` / `rankNorm`)

For derivative color mode, workers compute per-root Jacobian sensitivity:

- `computeSens(coeffsRe, coeffsIm, nCoeffs, rootsRe, rootsIm, nRoots, selIndices)` (~line 10058): Evaluates p(z) and p'(z) via Horner's method for each root, computes sensitivity as `sum(|z|^(deg-k)) / |p'(z)|` for selected coefficient indices k.
- `rankNorm(raw, n)` (~line 10036): Rank-normalizes raw sensitivity values to [0, 1] range, mapping to the 16-entry derivative palette.

### 4. Gaussian Dither (`wGaussRand`)

`wGaussRand()` (~line 10345) generates standard-normal random samples via the Box-Muller transform. Used when `ditherSigma > 0` to perturb coefficient positions after curve interpolation.

### 5. Message Handler (`self.onmessage`)

#### Persistent State (set once by "init")

```
S_coeffsRe, S_coeffsIm    Float64Array  Base coefficient values
S_nCoeffs, S_degree        int           Polynomial size
S_nRoots                   int           Root count (= degree)
S_colorsR/G/B              Uint8Array    Per-root RGB
S_W, S_H, S_range          int, number   Canvas dimensions and display range
S_curvesFlat               Float64Array  All curve points (re,im interleaved)
S_entries                  object[]      Animation entries [{idx, ccw, speed, ditherSigma}]
S_offsets, S_lengths       int[]         Curve offset/length per entry
S_isCloud                  bool[]        Random-cloud flag per entry
S_useWasmLoop              bool          Use WASM step loop (preferred when WASM enabled)
S_noColor                  bool          Uniform color mode
S_uniformR/G/B             int           Uniform color RGB
S_totalSteps, S_FPS        int, number   Steps per pass, seconds per pass
S_proxColor                bool          Proximity color mode
S_proxPalR/G/B             Uint8Array    16-entry proximity palette RGB
S_derivColor               bool          Derivative color mode
S_derivPalR/G/B            Uint8Array    16-entry derivative palette RGB
S_selIndices               int[]|null    Selected coefficient indices (for derivative mode)
S_idxProxColor             bool          Index × Proximity color mode (JS-only)
S_idxProxGamma             number        Gamma correction for Idx×Prox brightness
S_ratioColor               bool          Min/Max Ratio color mode (JS-only)
S_ratioGamma               number        Gamma correction for Min/Max Ratio
S_morphEnabled             bool          Morph blending active
S_morphRate                number        Morph oscillation rate (Hz)
S_morphTargetRe/Im         Float64Array  Morph target coefficient values
S_matchStrategy            string        Root matching strategy ("assign4"|"assign1"|"hungarian1")
S_dCurvesFlat              Float64Array  D-node curve points (re,im interleaved)
S_dEntries                 object[]      D-node animation entries [{idx, ccw, speed, ditherSigma}]
S_dOffsets, S_dLengths     int[]         D-curve offset/length per entry
S_dIsCloud                 bool[]        D-curve random-cloud flag per entry
S_dFollowC                 int[]         D-node indices with "follow-c" pathType (copy from C-node)
S_jiggleRe, S_jiggleIm     Float64Array  Per-coefficient jiggle offsets (applied post-interpolation)
wasmLoopExports            object        WASM step loop exports (if S_useWasmLoop)
wasmLoopMemory             WebAssembly.Memory  WASM linear memory for step loop
wasmLoopLayout             object        Byte-offset layout computed by computeWasmLayout()
wasmLoopNRoots             int           Root count cached for WASM loop
wasmLoopNCoeffs            int           Coefficient count cached for WASM loop
wasmLoopWorkerId           int           Current worker ID cached for WASM progress callback
wasmLoopTotalRunSteps      int           Total steps in current run cached for WASM progress callback
```

Solver tier selection during init (~line 10398):
1. If `useWasm` and `wasmStepLoopB64` present: try `initWasmStepLoop(d)` -> set `S_useWasmLoop = true`
2. Otherwise: pure JS (`S_useWasmLoop = false`)
3. Post-init check: if `S_useWasmLoop` and (`S_idxProxColor` or `S_ratioColor`), force `S_useWasmLoop = false` (these modes are JS-only)

#### "run" Handler: WASM Step Loop Path (~line 10432)

When `S_useWasmLoop && wasmLoopExports` is true:

1. Copy warm-start roots into WASM memory at `L.pRR`/`L.pRI` offsets
2. Set `wasmLoopWorkerId` and `wasmLoopTotalRunSteps` for progress callback
3. Call `wasmLoopExports.runStepLoop(stepStart, stepEnd, elapsedOffset)` — returns pixel count `pc`
4. If WASM throws, post an error message back and disable `S_useWasmLoop` for future runs
5. Slice output arrays from WASM memory: `paintIdx` (Int32), `paintR/G/B` (Uint8), final roots
6. Post "done" message with sparse pixels (transferred) and final roots (cloned)

The WASM step loop handles all per-step computation natively:
- Coefficient interpolation along C-curves (smooth linear interp or cloud snap)
- Dither (Gaussian noise via imported `cos`/`sin`/`log`)
- D-curve advance for animated morph targets
- Follow-C D-node copying
- Morph blend (cosine oscillation)
- Jiggle offset application
- Ehrlich-Aberth solver
- Root matching (greedy or Hungarian, per configured strategy and color mode)
- Pixel coordinate mapping and sparse pixel output
- Progress reporting via imported `reportProgress` callback (every 2000 steps)

#### "run" Handler: JS Step Loop Fallback (~line 10472)

When the WASM step loop is not active, the JS fallback executes. For each step in `[stepStart, stepEnd)`:

1. **Compute virtual time**: `elapsed = elapsedOffset + (step / totalSteps) * FAST_PASS_SECONDS`

2. **Reset base coefficients**: If jiggle is active, reset `coeffsRe`/`coeffsIm` to `S_coeffsRe`/`S_coeffsIm` so non-animated coefficients get clean jiggle application each step.

3. **Interpolate animated C-coefficients**: For each entry in `S_entries`:
   ```
   t = elapsed * speed * direction
   u = ((t % 1) + 1) % 1          // normalized phase [0, 1)
   rawIdx = u * curveLength
   ```
   - Cloud curves: snap to nearest point (no interpolation)
   - Smooth curves: linear interpolation between adjacent points
   - If `ditherSigma > 0`: add Gaussian noise `wGaussRand() * ditherSigma` to both re/im after interpolation

4. **Advance D-node curves** (if morph enabled): Same interpolation as step 3 but for `S_dEntries`/`S_dCurvesFlat`, updating `morphRe`/`morphIm` arrays. D-entries also support `ditherSigma`.

5. **Follow-C D-nodes**: For each index in `S_dFollowC`, copy the current C-node position (`coeffsRe[fci]`/`coeffsIm[fci]`) into `morphRe[fci]`/`morphIm[fci]`. Applied after D-curve advance, before morph blend — so follow-c D-nodes track the (possibly animated) C-coefficient position at each step.

6. **Morph blend** (if `S_morphEnabled`): Cosine blend `mu = 0.5 - 0.5 * cos(2pi * morphRate * elapsed)` between C-coefficients and morph target D-coefficients. Starts at mu=0 (pure C) and oscillates to mu=1 (pure D).

7. **Apply jiggle offsets**: If `S_jiggleRe`/`S_jiggleIm` present, add per-coefficient offsets post-interpolation.

8. **Solve**: Call `solveEA()` (JS).

9. **Color-mode-specific painting**: Branch based on active color mode (see "Root Color Modes" section below). Each branch handles root matching, color computation, and sparse pixel output.

10. **Report progress** every 2000 steps via `postMessage({type: 'progress', ...})`.

---

## WASM Step Loop Memory Layout

`computeWasmLayout(nc, nr, maxP, nE, nDE, nFC, nSI, tCP, tDP, heapBase)` (~line 10145) computes byte offsets for all buffers in WASM linear memory. The layout starts at `__heap_base` (read from WASM exports, default 65536) and aligns all allocations to 8 bytes.

| Abbreviation | Contents | Size |
|-------------|----------|------|
| `cfgI` | Integer config (65 Int32 values: dimensions, counts, mode flags, uniform RGB, RNG seeds, all buffer offsets) | 260 bytes |
| `cfgD` | Float64 config (range, FAST_PASS_SECONDS, morphRate) | 24 bytes |
| `cRe`, `cIm` | Base coefficient values | nc * 8 each |
| `clR`, `clG`, `clB` | Per-root color RGB | nr each |
| `jRe`, `jIm` | Jiggle offsets | nc * 8 each |
| `mTR`, `mTI` | Morph target coefficients | nc * 8 each |
| `ppR/G/B` | Proximity palette (16 entries) | 16 each |
| `dpR/G/B` | Derivative palette (16 entries) | 16 each |
| `sI` | Selected coefficient indices | nSI * 4 |
| `fCI` | Follow-C D-node indices | nFC * 4 |
| `eIdx`, `eSpd`, `eCcw`, `eDth` | C-curve entry metadata (index, speed, ccw, dither) | nE * (4+8+4+8) |
| `cOff`, `cLen`, `cCld` | C-curve offsets, lengths, cloud flags | nE * 4 each |
| `dIdx`, `dSpd`, `dCcw`, `dDth` | D-curve entry metadata | nDE * (4+8+4+8) |
| `dOff`, `dLen`, `dCld` | D-curve offsets, lengths, cloud flags | nDE * 4 each |
| `cvF` | C-curve flat data (re,im interleaved) | tCP * 16 |
| `dcF` | D-curve flat data (re,im interleaved) | tDP * 16 |
| `wCR`, `wCI` | Working coefficient copies (mutated during step) | nc * 8 each |
| `tRe`, `tIm` | Temporary root arrays (solver output) | nr * 8 each |
| `mWR`, `mWI` | Working morph target copies | nc * 8 each |
| `pRR`, `pRI` | Persistent roots (warm-start, updated each step) | nr * 8 each |
| `piO` | Pixel index output | maxP * 4 |
| `prO`, `pgO`, `pbO` | Pixel RGB output | maxP each |

`initWasmStepLoop(d)` (~line 10200):
1. Decodes the base64 WASM binary and compiles the module
2. Instantiates with a small initial memory (2 pages) plus env imports: `{memory, cos, sin, log, reportProgress}`
3. Reads `__heap_base` from WASM exports to determine where free memory begins
4. Calls `computeWasmLayout()` to determine total memory needed
5. Grows memory if needed (`wasmLoopMemory.grow()`)
6. Writes integer config (`cfgI32[0..64]`): nc, nr, W, H, totalSteps, colorMode (0=uniform, 1=rainbow, 2=proximity, 3=derivative), matchStrategy (0=assign4, 1=assign1, 2=hungarian), morphEnabled, entry counts, follow-C count, selIdx count, hasJiggle, uniform RGB, 4 RNG seed values, and all 45 buffer byte-offsets
7. Writes float config (`cfgF64[0..2]`): bitmapRange, FAST_PASS_SECONDS, morphRate
8. Copies all data arrays into their computed memory locations
9. Calls `wasmLoopExports.init(L.cfgI, L.cfgD)` to initialize WASM internal state

---

## Sparse Pixel Format

Workers avoid allocating W*H*4 buffers (a 10K*10K canvas = 400MB per worker). Instead:

| Array | Type | Content |
|-------|------|---------|
| `paintIdx` | Int32Array | Canvas pixel indices (row-major: `y * W + x`) |
| `paintR` | Uint8Array | Red channel |
| `paintG` | Uint8Array | Green channel |
| `paintB` | Uint8Array | Blue channel |
| `paintCount` | int | Actual number of pixels (<= allocated max) |

Max allocation: `(stepEnd - stepStart) * nRoots` entries. Transferred back to main thread as `Transferable` buffers (zero-copy).

Compositing on main thread writes sparse pixels directly into a **persistent `ImageData` buffer** (no `getImageData` needed). Only the dirty rectangle region is flushed to canvas via `putImageData`. Pixel order is arbitrary (no blending, last-write-wins within overlapping workers).

---

## Message Formats

### init (main -> worker, once per cycle)

```javascript
{
    type: "init",
    coeffsRe: ArrayBuffer,        // Float64Array, base coefficient real parts
    coeffsIm: ArrayBuffer,        // Float64Array, base coefficient imag parts
    nCoeffs: int,
    degree: int,
    nRoots: int,
    animEntries: [{idx, ccw, speed, ditherSigma}, ...],
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
    idxProxColor: bool,            // index x proximity color mode (JS step loop only)
    idxProxGamma: number,          // gamma for idx-prox brightness
    ratioColor: bool,              // min/max ratio color mode (JS step loop only)
    ratioGamma: number,            // gamma for ratio mode
    selectedCoeffIndices: [int, ...],  // which coefficients for derivative sensitivity
    totalSteps: int,
    FAST_PASS_SECONDS: number,     // always 1.0
    useWasm: bool,                 // use WASM solver
    wasmStepLoopB64: string|null,  // base64-encoded WASM step loop binary (if useWasm)
    morphEnabled: bool,            // morph blending active
    morphRate: number,             // morph oscillation rate (Hz)
    morphTargetRe: ArrayBuffer,    // Float64Array, morph target real parts (or null)
    morphTargetIm: ArrayBuffer,    // Float64Array, morph target imag parts (or null)
    matchStrategy: string,         // "assign4"|"assign1"|"hungarian1"
    dAnimEntries: [{idx, ccw, speed, ditherSigma}, ...],  // D-node animation entries
    dCurvesFlat: ArrayBuffer,      // Float64Array, D-node curve points
    dCurveOffsets: [int, ...],
    dCurveLengths: [int, ...],
    dCurveIsCloud: [bool, ...],
    dFollowCIndices: [int, ...],   // D-node indices with "follow-c" pathType
    jiggleRe: ArrayBuffer,         // Float64Array, per-coefficient jiggle offsets (or null)
    jiggleIm: ArrayBuffer,
    totalCPts: int,                // total C-curve points (for WASM layout)
    totalDPts: int,                // total D-curve points (for WASM layout)
    maxPaintsPerWorker: int,       // ceil(stepsVal / actualWorkers) * nRoots (for WASM pixel buffer sizing)
}
```

Buffers are sent via `.buffer.slice(0)` (structured clone of copies). Workers store these as persistent state.

### run (main -> worker, once per pass)

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

### progress (worker -> main, every 2000 steps)

```javascript
{
    type: "progress",
    workerId: int,
    step: int,                     // steps completed by this worker so far
    total: int,                    // total steps assigned to this worker
}
```

In the WASM step loop path, progress is reported via the imported `reportProgress(step)` function, which calls `self.postMessage` with the same format. The `wasmLoopWorkerId` and `wasmLoopTotalRunSteps` variables are set before each `runStepLoop` call so the callback can construct the message.

### error (worker -> main, on WASM failure)

```javascript
{
    type: "error",
    workerId: int,
    error: string,                 // stringified error from WASM
}
```

Sent when `runStepLoop()` throws. The worker disables `S_useWasmLoop` so subsequent runs fall back to JS.

### done (worker -> main, end of pass)

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

### enterFastMode() (~line 10881)

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
12. **Otherwise**: serialize data -> `initFastModeWorkers()` -> first `dispatchPassToWorkers()`

### initFastModeWorkers() (~line 11148)

Worker count is capped: `actualWorkers = Math.min(numWorkers, stepsVal)`. This prevents creating more workers than steps (e.g., 16 workers for 100 steps would leave most workers with 0 steps). For each of `actualWorkers` workers:
1. Create blob URL via `createFastModeWorkerBlob()`
2. `new Worker(blobUrl)`, then `URL.revokeObjectURL`
3. Set `onmessage` (`handleFastModeWorkerMessage`) and `onerror` handlers
4. Send "init" message with all static data (buffer copies), including:
   - `useWasm: solverType === "wasm"` — WASM is only used when the user has selected wasm in the cfg popup
   - `wasmStepLoopB64: WASM_STEP_LOOP_B64` — base64 binary sent per init
   - `maxPaintsPerWorker = ceil(stepsVal / actualWorkers) * nRoots` for WASM pixel buffer sizing
5. Push to `fastModeWorkers` array

Then immediately call `dispatchPassToWorkers()`.

### dispatchPassToWorkers() (~line 11218)

1. Calculate balanced step distribution: `base = floor(stepsVal / nw)`, `extra = stepsVal % nw` (where `nw = fastModeWorkers.length`, the actual worker count). Each worker gets `base + (w < extra ? 1 : 0)` steps, using a running offset for `stepStart`/`stepEnd`.
2. Reset completion counter and pixel/progress arrays
3. Record `fastModePassStartTime`
4. For each worker: send "run" message with `{stepStart, stepEnd, elapsedOffset, rootsRe, rootsIm}`

### Pass completion (all workers done) (~line 11275)

1. `compositeWorkerPixels()` (~line 11286) — merge sparse pixels onto persistent `ImageData` buffer, flush dirty rect
2. `recordTick()` — update frame counter
3. `recordPassTiming(workerMs)` (~line 11370) — push `{passMs, stepsPerSec, workers, steps, workerMs, composite}` to history
4. `advancePass()` (~line 11383):
   - `fastModeElapsedOffset += 1.0`, increment pass counter
   - Update pass counter UI
   - **Jiggle boundary check**: if `jiggleMode !== "none"` and `passCount >= targetPasses`:
     - `reinitWorkersForJiggle()` (~line 11407): terminate workers -> `computeJiggleForStep()` -> re-serialize data -> `initFastModeWorkers()` (elapsed offset continues, no exit/re-enter)
   - Otherwise: update warm-start roots in shared data -> `dispatchPassToWorkers()`

### exitFastMode() (~line 11428)

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

The **PrimeSpeeds** transform (in the C-List/D-List Transform dropdown) sets speeds coprime to all others, forcing GCD=1 and maximum cycle diversity. `findPrimeSpeed()` (~line 10777) operates on integer speed values (internal speed * 1000, range 1-1000) and finds the nearest coprime integer, then divides back by 1000 for the internal speed. The PS button is also available in per-coefficient path picker popups.

### Jiggle Integration

Jiggle offsets are applied **post-interpolation** in workers (not baked into precomputed curves). At each step, after coefficient interpolation and morph blending, `S_jiggleRe[i]`/`S_jiggleIm[i]` are added to each coefficient.

At jiggle boundary (`passCount >= targetPasses` and `jiggleMode !== "none"`):
1. `reinitWorkersForJiggle()` terminates current workers
2. `computeJiggleForStep()` computes new perturbation offsets
3. Re-serialize data (new jiggle offsets included) -> `initFastModeWorkers()` starts fresh workers
4. Elapsed offset continues (no reset), warm-start roots are preserved

No curve recomputation is needed since jiggle is applied post-interpolation.

---

## Data Serialization

`serializeFastModeData(animated, stepsVal, nRoots)` (~line 11010) produces the shared data object:

### Coefficient Arrays
- `coeffsRe`, `coeffsIm`: Float64Array of current coefficient positions (base values, no jiggle applied)
- Jiggle offsets are sent separately as `jiggleRe`/`jiggleIm` and applied post-interpolation in workers

### Curve Data (flattened)
All animated curves concatenated into one `Float64Array`:
```
curvesFlat: [re0_p0, im0_p0, re0_p1, im0_p1, ..., re1_p0, im1_p0, ...]
```
With metadata arrays:
- `animEntries`: `[{idx: coeffIndex, ccw: bool, speed: number, ditherSigma: number}, ...]`
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
- `idxProxColor` flag + `idxProxGamma`: index * proximity mode with gamma correction
- `ratioColor` flag + `ratioGamma`: min/max ratio mode with gamma correction
- `selectedCoeffIndices`: which coefficient indices to use for derivative sensitivity

### Morph & D-Curve Data
- `morphEnabled`, `morphRate`: morph blending configuration
- `morphTargetRe`, `morphTargetIm`: Float64Array of morph target positions
- D-curves serialized identically to C-curves: `dAnimEntries`, `dCurvesFlat`, `dCurveOffsets`, `dCurveLengths`, `dCurveIsCloud`
- `dFollowCIndices`: array of D-node indices where `pathType === "follow-c"`. Built by `morphTargetCoeffs.reduce()` in `serializeFastModeData()`. Workers use this to copy the current C-node position into the morph target at each step.
- `totalCPts`, `totalDPts`: total curve point counts for C-curves and D-curves respectively. Used by `computeWasmLayout()` to size WASM memory buffers.
- `ditherSigma` in each animation entry: absolute dither radius (converted from `_ditherSigmaPct / 100 * coeffExtent()`). Workers add Gaussian noise scaled by this value after interpolation.

### Root Matching
- `matchStrategy`: `"assign4"` (default), `"assign1"`, or `"hungarian1"`

### Jiggle Offsets
- `jiggleRe`, `jiggleIm`: Float64Array, one entry per coefficient. Applied post-interpolation in workers. Zero for non-jiggled coefficients.

---

## Root Color Modes

Six bitmap color modes are available. The mode is stored in `bitmapColorMode` (~line 1016) and serialized as boolean flags in the init message:

| Mode | Flag(s) | Root matching | WASM step loop | Worker behavior |
|------|---------|---------------|----------------|-----------------|
| `"uniform"` | `noColor = true` | None | Yes | All roots painted with `uniformR/G/B`. |
| `"rainbow"` | `noColor = false`, all other flags false | Per `matchStrategy` | Yes | Each root uses `colorsR[i]/G[i]/B[i]` from d3 rainbow. |
| `"derivative"` | `derivColor = true` | Greedy every 4th step | Yes | Workers compute Jacobian sensitivity via `computeSens()` + `rankNorm()`. Color from 16-entry blue-white-red derivative palette. |
| `"proximity"` | `proxColor = true` | None | Yes | Workers compute min pairwise distance per root with adaptive running max normalization. Color from selectable 16-entry sequential palette (8 options: Inferno, Viridis, Magma, Plasma, Turbo, Cividis, Warm, Cool). |
| `"idx-prox"` | `idxProxColor = true` | Per `matchStrategy` | **No** (JS only) | Combines per-root rainbow color with proximity-based brightness. `brightness = pow(min(1, dist/maxDist), gamma)`. Root color is `colorsRGB[i] * brightness`. |
| `"ratio"` | `ratioColor = true` | None | **No** (JS only) | Computes `minDist/maxDist` ratio per root in single O(n^2) pass. Color from proximity palette via `pow(ratio, gamma)`. |

Uniform and proximity modes are fastest: no per-root color lookup, no root matching overhead. The idx-prox and ratio modes are JS-only — the WASM step loop is forced off when these are active.

### Proximity Adaptive Normalization

Both proximity and idx-prox modes use a running maximum for normalization:
- `proxRunMax` starts at 1 and is updated each step: `if (minDist > proxRunMax) proxRunMax = minDist`
- Slow decay: `proxRunMax *= 0.999` each step
- This provides adaptive normalization without needing a global pass over all distances

### Ratio Mode

Ratio mode computes both min and max pairwise distances per root in a single O(n^2) pass per step:
- `ratio = pow(minDist / maxDist, ratioGamma)`
- Maps to the proximity palette via `palIdx = (ratio * 15 + 0.5) | 0`
- No root matching needed (position-based, not identity-based)

---

## UI Controls

### Bitmap Toolbar

| Control | ID | Function |
|---------|----|----------|
| Start/Restart | `bitmap-start-btn` | `initBitmapCanvas()`, enables other buttons |
| Save | `bitmap-save-btn` | PNG export with timestamp filename |
| Clear | `bitmap-clear-btn` | Fill canvas with bg color, reset `fastModeElapsedOffset` |
| Resolution | `bitmap-res-select` | 1000, 2000, 5000, 8000, 10000, 15000, 25000 px |
| Fast/imode | `bitmap-fast-btn` | Toggle `enterFastMode()` / `exitFastMode()` |
| ROOT / COEF | `bitmap-coeff-btn` | Toggle coefficient view (plot coefficients instead of roots) |
| Steps | `bitmap-steps-select` | 10, 100, 1K, 5K, 10K, 50K, 100K, 1M steps per pass |
| Jiggle | `jiggle-btn` | Opens jiggle configuration popup |
| Diagnostics | `jiggle-diag-btn` | Opens jiggle diagnostics popup (mode, offsets, positions) |
| Background | `bitmap-bg-btn` | Color picker for canvas background |
| Progress | `bitmap-progress-toggle` | Toggle progress bar visibility |

Changing the **Steps** or **Resolution** dropdowns while fast mode is active triggers an automatic restart (`exitFastMode(); enterFastMode()`), re-serializing data and re-creating workers with the new parameters.

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

Scaling is sub-linear due to: structured clone overhead and main-thread compositing. Step distribution is balanced (`floor` + remainder allocation), so worker load imbalance is at most 1 step.

### Bottlenecks

1. **Ehrlich-Aberth solver**: O(n^2 * iters) per step, where n = degree. Dominates at degree > 10. WASM solver reduces this by eliminating JIT warmup, GC pauses, and leveraging tighter f64 codegen — biggest gains at high degree.
2. **Root matching**: O(n^2) per call (greedy) or O(n^3) (Hungarian). Frequency depends on `matchStrategy`: every 4th step (default), every step, or Hungarian every step. Skipped in uniform, proximity, and ratio modes.
3. **Compositing**: Persistent `ImageData` buffer with dirty-rect `putImageData`. After the persistent buffer optimization, compositing is fast (~5ms at 10K) and no longer the bottleneck. See [memory_timings.md](memory_timings.md).

### Memory

Per worker:
- Sparse pixel arrays: `(stepEnd - stepStart) * nRoots * 13 bytes` (Int32 index + 3 * Uint8 color)
- Coefficient/root arrays: `nCoeffs * 16 + nRoots * 16 bytes`
- Curve data: shared via structured clone, ~`totalCurvePoints * 16 bytes`
- WASM step loop (if active): linear memory sized by `computeWasmLayout()` (includes all the above plus working arrays, palettes, pixel output buffers)

No per-worker W*H*4 canvas buffer. A 10K*10K canvas with 4 workers and 100K steps would use ~37MB per worker for pixel arrays at degree 29 (worst case), vs 400MB for a full canvas buffer.

### Tradeoffs

| Decision | Rationale |
|----------|-----------|
| Persistent workers (init once) | Avoids re-sending ~KB of curve data per pass |
| Sparse pixels over full buffers | 10K*10K canvas = 400MB per buffer. Sparse format is 13 bytes per painted pixel. |
| 64 solver iterations (not 100) | Warm start from previous step makes convergence fast. 64 iterations gives ample margin for high-degree polynomials. |
| Root matching every 4th step (default) | O(n^2) matching costs ~25% of solver time at degree 20. Every-4th is a good tradeoff for color accuracy vs speed. Users can switch to every-step or Hungarian (O(n^3)) for better accuracy at higher cost. |
| No SharedArrayBuffer | Requires COOP/COEP headers. GitHub Pages doesn't set them. Structured clone is fast enough for the small per-pass data (roots + step range). |
| No OffscreenCanvas | Useful for single worker but complex for multi-worker merging. Sparse pixel approach is simpler and equally fast. |
| All workers get same warm-start roots | Workers 1-N start with roots from time 0 (not their actual time). EA converges in 1-2 extra iters on first step. Negligible cost vs complexity of chaining roots across workers. |
| WASM solver optional (not default) | JS solver is already fast and requires no compilation toolchain. WASM provides marginal gains at low degree but significant gains at degree 50+. Users can toggle via cfg button. |
| WASM b64 sent per init (not shared) | Each worker decodes and compiles independently (~1ms). Avoids complexity of sharing compiled modules across workers. Only happens once per fast-mode session. |
| WASM step loop (full pipeline) | Moving the entire step loop into WASM eliminates JS-to-WASM boundary crossing per step. The step loop WASM module handles interpolation, D-curves, follow-C, morph, jiggle, solver, matching, and pixel output natively. Two-tier fallback (step loop -> JS) ensures graceful degradation. |
| `maxPaintsPerWorker` per-worker sizing | WASM step loop pre-allocates pixel output buffers in linear memory. `ceil(stepsVal / actualWorkers) * nRoots` gives the exact upper bound for each worker's pixel count, avoiding over-allocation when workers are capped below `numWorkers`. |
| Idx-prox and ratio modes JS-only | These color modes are not implemented in `step_loop.c`. The WASM step loop is forced off when they are active, falling back to pure JS. |

---

## Legacy Fallback

For browsers without `Worker` support. Runs on main thread via `setTimeout(fastModeChunkLegacy, 0)` (~line 11487).

Chunk size: `max(10, floor(200 / max(3, degree)))` steps per setTimeout callback. This keeps each chunk under ~16ms to avoid blocking the UI event loop.

Uses the same animation interpolation and solver logic but calls `paintBitmapFrameFast()` (~line 9829, canvas `fillRect` 1x1 pixel writes) instead of sparse pixel buffers. Follow-c D-nodes are handled per step: for each D-node with `pathType === "follow-c"`, its position is copied from the corresponding C-coefficient (`coefficients[fi]`).

Jiggle boundary logic mirrors worker mode: when `jiggleMode !== "none"` and `passCount >= targetPasses`, reset pass count and call `computeJiggleForStep()`. No curve recomputation needed since jiggle is applied post-interpolation. Unlike worker mode, the legacy fallback does not exit/re-enter fast mode at jiggle boundaries.

---

## Coefficient Animation in Workers

### Curve Precomputation

In `enterFastMode()`, for each coefficient with `pathType !== "none"`:
1. Home position = `curve[0]` (first point of interactive-mode curve)
2. `computeCurveN(home, pathType, absRadius, angle, extra, stepsVal)` generates N points (jiggle offsets are NOT baked in; they are applied post-interpolation in workers)
3. Stored in `fastModeCurves` Map

D-node curves are precomputed identically and stored in `fastModeDCurves` Map.

The curve resolution matches `stepsVal` (steps per pass), so each step maps to approximately one curve point. With speed < 1.0, the coefficient traverses only a fraction of the curve per pass.

### Speed Resolution

Speed values have 1/1000 resolution. The UI slider ranges from 1 to 1000 (integer), mapped to internal speed values via `toUI: v => Math.round(v * 1000)` and `fromUI: v => v / 1000`. So internal speed 1.0 = slider value 1000, and the minimum speed 0.001 = slider value 1. Display shows the raw integer slider value (e.g., "500" for speed 0.5).

`findPrimeSpeed()` operates on integer speeds (multiplied by 1000) when computing coprimality for the PrimeSpeeds transform, searching the range [1, 1000].

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

### Dither

When `ditherSigma > 0` for an animation entry, Gaussian noise is added after interpolation:
```
coeffsRe[idx] += wGaussRand() * ditherSigma
coeffsIm[idx] += wGaussRand() * ditherSigma
```
`ditherSigma` is an absolute value converted from percentage at serialization time: `_ditherSigmaPct / 100 * coeffExtent()`. Both C-curve and D-curve entries support dither independently.

In the WASM step loop, dither uses the same Box-Muller transform with `cos`/`sin`/`log` imported from JS.

### Morph & D-Node Handling in Workers

When morph is enabled, workers maintain separate `morphRe`/`morphIm` arrays (pre-allocated copies of `S_morphTargetRe`/`S_morphTargetIm`). Each step:

1. **D-curve advance**: Animated D-nodes are interpolated along their D-curves (same algorithm as C-curves), updating `morphRe[dIdx]`/`morphIm[dIdx]`.
2. **Follow-C**: D-nodes with `follow-c` path type have their morph target position overwritten with the current C-coefficient position: `morphRe[fci] = coeffsRe[fci]`.
3. **Morph blend**: All coefficients are blended: `coeff = coeff * (1 - mu) + morphTarget * mu` where `mu = 0.5 - 0.5 * cos(2*pi * morphRate * elapsed)`.

This ordering ensures follow-C D-nodes track the animated C-coefficient position (not the base position), and the blend is applied after all target positions are finalized.

---

## Coefficient View Mode

When `bitmapCoeffView` is true (toggled via ROOT/COEF button), fast mode plots **coefficient positions** on the bitmap canvas instead of solving for roots. This visualizes the actual coefficient curves and jiggle perturbations.

### How It Works

1. Curves are precomputed by `enterFastMode()` identically to normal mode (jiggle offsets are NOT baked in)
2. `plotCoeffCurvesOnBitmap(animated, stepsVal)` (~line 10801) iterates all steps:
   - **Animated coefficients**: read position from `fastModeCurves.get(idx)[step % curveLength]`, then add jiggle offset if present
   - **Non-animated coefficients**: fixed at base position, then add jiggle offset if present
3. Each coefficient plotted as a **3x3 pixel square** using `coeffColor(i, n)` rainbow palette
4. Uses `ImageData` pixel writes for speed (no canvas arc calls)
5. `bitmapRange` is set to `panels.coeff.range` (coefficient panel viewport) instead of root panel range

### No Workers Needed

Coefficient plotting is pure coordinate projection — no Ehrlich-Aberth solver, no root matching. Runs entirely on the main thread. For 1M steps * 30 coefficients = 30M pixel writes, this completes in well under a second.

### Jiggle Cycling

When jiggle is active, `coeffJiggleCycle` runs via `setTimeout(fn, 0)`:
1. At jiggle boundary: `computeJiggleForStep()` — compute new perturbation offsets (no curve recomputation needed since jiggle is applied post-interpolation)
2. Plot new curve points via `plotCoeffCurvesOnBitmap()` (accumulating on existing canvas, jiggle offsets applied per-step)
3. Schedule next cycle

The timer is stored in `fastModeTimerId`, so `exitFastMode()` cleanly stops the cycle.

### Interactive Root Suppression

When `bitmapCoeffView` is true, bitmap rendering only plots coefficient positions — root painting is suppressed.
