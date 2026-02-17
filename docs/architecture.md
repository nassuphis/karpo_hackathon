# Architecture Notes

Architecture, conventions, performance insights, and debugging notes for the PolyPaint codebase.

---

## 1. Architecture Overview

### Single-File Design

The entire application lives in one HTML file (`index.html`, ~12,500 lines). CSS is embedded in a `<style>` block (lines 12–560), HTML body is lines 562–922, and all JavaScript is inline in a single `<script>` block (lines 923–12481). There is no build step — serve the file directly.

**Why it works**: Zero tooling overhead, instant deployment to GitHub Pages, no import/bundling issues. D3.js and html2canvas are loaded from CDN.

**Why it hurts**: No modules means everything shares one scope. Finding a function means searching by name. Related code can be hundreds of lines apart. The file is too large for most editors' "go to definition" to work well.

### Section Map

| Section | Lines | What's There |
|---------|-------|-------------|
| CSS variables & layout | 12–560 | Colors, grid, popover, panel, animation styles |
| HTML body | 562–922 | Header (incl. animation controls), panels, SVG containers, 13 popovers |
| Constants & config | 925–1130 | Audio params, stat types, color maps, defaults |
| Audio/sonification | 1200–1800 | FM synth, arpeggiator, encounters, routing |
| Coefficient data model | 987–1000 | State arrays, selection sets, trail data |
| Rendering functions | 1830–5050 | SVG circles, trails, domain coloring, grid |
| Add/delete coefficients | 3097–3170 | Right-click canvas to add, context menu delete |
| D-node animation helpers | 3372–3410 | `allAnimatedDCoeffs()`, `advanceDNodesAlongCurves()`, `updateMorphPanelDDots()` |
| Morph panel & rendering | 4410–4680 | Ghost layer, D dots, interp lines/markers, drag handler |
| Animation loop | 4265–4360 | `animLoop()`, path interpolation, morph mu, throttled solve |
| Root solver | 5057–5250 | Ehrlich-Aberth, root matching, warm start |
| Event handlers | 5250–5950 | Mouse, keyboard, tab switch, popover toggle |
| Ops tools | 6047–6230 | Scale, rotate, add — popover builders |
| Recording & snapshots | 6350–7530 | Video capture, save/load JSON, PNG export |
| Stats plotting | 7767–8720 | 16 time-series canvases, stat computation |
| Bitmap & fast mode | 8720–10000 | Persistent buffer, worker coordination, compositing |
| Web worker blob | 8954–9550 | Inline EA solver for workers |
| List tab & transforms | 10575–11390 | Coefficient table, Transform dropdown, bulk operations |
| D-List tab | 11391–12000 | D-node table, D-list curve editor |
| Jiggle panel | 12233–12400 | Jiggle configuration panel |
| Initialization | 12400–12481 | Default coefficients, first render, event wiring |

### External Dependencies

All from CDN:
- **D3.js v7** — SVG scaling, axes, drag behavior, selections
- **html2canvas v1.4.1** — PNG export of panels
- **Pako v2.1.0** + **UPNG.js v2.1.0** — PNG encoding from RGBA data (Pako provides deflate)
- **jpeg-js v0.4.4** — JPEG encoding from RGBA data (global: `window['jpeg-js'].encode`)
- **UTIF.js v3.1.0** — TIFF encoding from RGBA data (uses `self.pako` if available)

### Canvas Architecture

Four distinct canvas systems coexist:

1. **SVG panels** (D3-managed) — Coefficient and root circles, trails, grid. Interactive (draggable).
2. **Domain canvas** (`#domain-canvas`) — Complex plane coloring overlay behind root SVG. Rendered at 50% resolution for performance.
3. **Bitmap canvas** (`#bitmap-canvas`) — High-res accumulation canvas (1K–25K px) for fast mode. Uses a **split compute/display** architecture: at resolutions above 2000px, the canvas is capped at 2000px (display) while a CPU-only `ImageData` persistent buffer holds the full compute resolution. Workers always compute in compute-space; `compositeWorkerPixels()` downsamples to the display buffer. Export supports JPEG/PNG/BMP/TIFF via pure-JS encoders directly from the CPU buffer — no GPU involvement. The save button opens a format popup with quality slider for JPEG. See [off-canvas-render.md](off-canvas-render.md).
4. **Stats canvases** — 16 small canvases in a 4x4 grid, each plotting a time-series statistic.

---

## 2. Data Model

### Coefficient Objects

Each coefficient is a plain object with these fields:

```
{ re, im, pathType, radius, speed, angle, ccw, extra, curve, curveIndex }
```

- `re`, `im` — Current complex position
- `pathType` — Animation type: `"none"`, `"circle"`, `"spiral"`, `"lissajous"`, etc.
- `radius` — Path radius (0–100, as % of panel extent)
- `speed` — Animation speed multiplier
- `angle` — Starting phase (0–1 turns)
- `ccw` — Counter-clockwise flag
- `extra` — Path-specific params (object, varies by pathType)
- `curve` — Pre-computed array of `{re, im}` points (N samples of the closed path)
- `curveIndex` — Current integer index into `curve[]`

**Important**: `"none"` path type means a 1-point curve at the coefficient's home position. It is NOT null — always check `pathType`, never check `curve == null`.

### Morph System

Coefficient morphing blends two coefficient sets: primary C (`coefficients[]`) and target D (`morphTargetCoeffs[]`). When enabled, the solver receives `C[i]*(1-mu) + D[i]*mu` where `mu = 0.5 + 0.5*sin(2π*morphRate*elapsed)`.

Key state: `morphEnabled`, `morphRate` (Hz, 0.01–2.00), `morphMu` (0–1), `morphTargetCoeffs[]` (same structure as `coefficients[]`). D-nodes can be assigned paths via the D-List tab, making the morph target itself dynamic.

**Design insight**: Morph is a "global path" — it affects all coefficients equally through a single parameter. Unlike per-coefficient paths which are independent, morph creates correlated perturbation across the entire polynomial. The blending happens BEFORE the solver call, making it transparent to both JS and WASM solvers.

**Fast mode**: Workers receive D positions in the init message and compute mu per-step. The blending loop is 7 lines inserted between curve interpolation and solver call. No WASM changes needed.

**Continuous fast mode**: Fast mode runs continuously until stopped. `jiggleInterval` (1–100s, cfg popup) controls how often jiggle perturbations fire — not a cycle length. "init" button snapshots animation state + clears bitmap + resets elapsed. "start"/"stop" toggles computation. Stopping preserves elapsed; resuming continues. "clear" only clears pixels. Elapsed seconds shown as zero-padded counter.

### Root State

- `currentRoots[]` — Array of `{re, im}` objects, one per root
- `rootSensitivities` — `Float64Array` for derivative-based coloring
- `trailData[]` — Array of arrays: `[[{re,im}, ...], ...]`, one inner array per root
- `closeEncounters` — `Float64Array(n * 3)` per-root top-3 closest distances

### Selection

Two `Set` objects: `selectedCoeffs` (indices into `coefficients[]`) and `selectedRoots` (indices into `currentRoots[]`). Most operations check `.size > 0` before enabling UI.

### C-List Tab

The C-List tab (`leftTab === "list"`) shows a tabular view of all coefficients with:
- **Selection buttons**: All, None, SameCurve, and a curve-type cycler (cycles path types, not individual nodes)
- **Transform dropdown** (20 transforms): Applies a one-shot transform to `selectedCoeffs`, then resets to "none". Includes PrimeSpeeds, SetAllSpeeds, RandomSpeed, RandomAngle, RandomRadius, LerpSpeed, LerpRadius, LerpAngle, RandomDirection, FlipAllDirections, ShuffleCurves, ShufflePositions, CircleLayout, RotatePositions, ScalePositions, JitterPositions, Conjugate, InvertPositions, SortByModulus, SortByArgument.
- **Param1/Param2 sliders**: Passive inputs that transforms read when executed.
- **Per-coefficient columns**: Index with color dot, position (re, im), speed (spd), radius (rad), curve length (pts), curve position (pos).

### Add/Delete Coefficients

- **Right-click on empty canvas space** → `addCoefficientAt(re, im, event)` creates a new highest-power coefficient via `unshift()`, adjusts selection indices +1, clears trails, opens context menu on index 0.
- **Right-click on existing coefficient** → context menu with path editing. **Delete** button (red-styled) removes the coefficient, with a guard preventing deletion below degree 1 (2 coefficients minimum).
- Both operations call `clearTrails()` and `solveRoots()` to keep the root display consistent.

---

## 3. Core Algorithms

### Ehrlich-Aberth Solver

Simultaneous iterative root finder with cubic convergence. Key parameters:

| Parameter | Value | Notes |
|-----------|-------|-------|
| Max iterations | 100 (main), 64 (worker/WASM) | 64 gives ample margin with warm-starting |
| Tolerance | 1e-12 magnitude (main), 1e-16 squared (worker/WASM) | Worker uses tighter tolerance for accuracy |
| Leading-zero test | `Math.hypot` < 1e-15 (main), magnitude² < 1e-30 (worker/WASM) | Main uses Math.hypot; workers use manual |
| Hot loop optimization | No `Math.hypot` in worker/WASM | Workers use `re*re + im*im` directly |
| WASM option | Compiled C solver | Selectable via cfg button in bitmap toolbar |

**Warm starting** is critical: reusing previous roots as initial guesses cuts iterations from ~20 to ~3-5 for small coefficient movements. This is what makes interactive dragging feel instant.

**NaN handling**: The solver always returns exactly `degree` roots. Non-finite results (which occur in numerically difficult regimes, especially with many rapidly-moving coefficients) fall back to warm-start values, then to unit-circle seeding. This is essential because the Aberth correction couples all roots — a single NaN propagates to every root within one iteration. The worker solver also actively re-seeds non-finite roots in the warm-start buffer after each call.

**Root matching** (`matchRootOrder`) uses greedy nearest-neighbor after each solve to preserve root identity across frames. Called every 4th step in colored mode because it's O(n^2).

### Animation Pipeline

```
animLoop()
  ├─ Update coefficient positions along curves (elapsed time → curveIndex)
  ├─ Update morphMu (sinusoidal oscillation) if morphEnabled
  ├─ solveRootsThrottled()
  │   └─ requestAnimationFrame → solveRoots()
  │       ├─ Blend C with D if morphEnabled: coeffs[i] = C[i]*(1-mu) + D[i]*mu
  │       └─ renderRoots(newRoots)
  │           ├─ Draw SVG circles
  │           ├─ Update trail data
  │           ├─ Compute stats
  │           └─ updateAudio()
  ├─ Update morph panel visuals (ghosts, lines, markers) if morph tab visible
  ├─ renderCoefficients()
  ├─ renderCoeffTrails()
  └─ requestAnimationFrame(animLoop)  [next frame]
```

**Fast mode is separate**: when `fastModeActive`, `animLoop()` returns early (line 4267). Workers drive the pipeline instead: `dispatchPassToWorkers()` → workers interpolate curves + blend morph + solve → `handleFastModeWorkerMessage()` → `compositeWorkerPixels()` → `advancePass()`. Fast mode runs continuously — `advancePass()` calls `reinitWorkersForJiggle()` every `jiggleInterval` passes (if jiggle enabled), which regenerates offsets and recomputes curves without resetting elapsed time.

---

## 4. Fast Mode & Web Workers

### Architecture

Workers are created as blob URLs from inline code (no separate `.js` file). Each worker receives the full polynomial on `init`, then gets `{stepStart, stepEnd}` ranges on each `run` message.

**Data flow per pass**:
```
Workers (parallel)                    Main Thread
────────────────                      ──────────────
Ehrlich-Aberth solver  ──┐
  × steps_per_worker     │
Root matching (every 4)  │──→ structured clone ──→ handleFastModeWorkerMessage
Sparse pixel gen         │    (paintIdx, R,G,B)         │
                         │                              ▼
                         │                     compositeWorkerPixels()
                         │                       1. Write sparse pixels to persistent buffer
                         │                       2. Track dirty rect
                         │                       3. putImageData(dirty region only)
                         └──────────────────────────────▼
                                                  Canvas updated
```

### Sparse Pixel Format

Workers send `{paintIdx: Int32Array, paintR/G/B: Uint8Array, paintCount}`. This is vastly more efficient than sending `W×H×4` RGBA buffers — a pass painting 29,000 pixels sends ~130KB vs 400MB at 10K resolution.

### Persistent Buffer Optimization

The biggest performance win in the codebase. Before: `getImageData` + `putImageData` on the full canvas every pass. After: persistent `ImageData` buffer + dirty-rect `putImageData`.

Results at 10K resolution: **14.4x speedup** (125ms → 8.7ms per pass). See [memory_timings.md](memory_timings.md) for full analysis.

### Why Not SharedArrayBuffer?

Requires `Cross-Origin-Opener-Policy` and `Cross-Origin-Embedder-Policy` headers. GitHub Pages doesn't support custom headers. Structured clone of sparse data is fast enough.

### Why Not OffscreenCanvas?

Good for single worker, but compositing from multiple workers into one canvas requires either:
- Layered `OffscreenCanvas` per worker (memory explosion at high res), or
- Single shared `OffscreenCanvas` with synchronization (complex)

Deferred as not worth the complexity given sparse pixels work well.

### WASM Solver Integration

The Ehrlich-Aberth solver was ported to C (`solver.c`) and compiled to WebAssembly via Homebrew LLVM + lld. The ~2KB WASM binary is base64-encoded and embedded in `index.html`.

**Key design decisions**:
- **Only workers use WASM** — main thread solver stays JS (single call per frame, marshalling overhead not worth it)
- **No stdlib, no malloc, no math.h** — pure f64 arithmetic. NaN check via `x != x` (IEEE 754)
- **NaN rescue stays in JS** — cos/sin for unit-circle re-seeding is a cold path, not worth the WASM complexity
- **WASM memory: 64KB (1 page)** — data region ~8.25KB + 32KB shadow stack. No growth needed
- **Data copied in/out** — not zero-copy, but copy overhead is negligible vs O(n² × iters) solver cost
- **Each worker compiles independently** — avoids sharing compiled modules across workers (~1ms init cost)
- **Selectable via cfg button** — users can compare JS vs WASM performance using the timing popup

**Build workflow**: `./build-wasm.sh` compiles `solver.c` → `solver.wasm` → `solver.wasm.b64`. Paste the base64 into `WASM_SOLVER_B64` in `index.html`.

---

## 5. Sonification System

### Three Instruments

| Instrument | Synthesis | Triggered By |
|-----------|-----------|-------------|
| **Base** (B) | FM synthesis: sine carrier + sine modulator | Continuous — root constellation shape |
| **Melody** (M) | Triangle wave arpeggiator, pentatonic scale | Per-frame — top-N fastest roots |
| **Voice** (V) | Sine beep with attack/decay envelope | Event — record-breaking close encounters |

### Audio Graph

```
[modulator: sine] ──► [modGain] ──► carrier.frequency
                                                          ┌──► speakers
[carrier: sine 110Hz] ──► [gainNode] ──► [lowpass] ──► [masterGain]┤
                                                          └──► [mediaDest] ──► recording

[beepOsc: sine] ──► [beepGain] ──► [masterGain]

[arpOsc: triangle] ──► [arpGain] ──► [arpFilter: lowpass] ──► [masterGain]

[lfo: sine 1.5–7.5Hz] ──► [lfoGain] ──► carrier.frequency
```

### Signal Routing

Each instrument has configurable routes: `{source, target, alpha, normMode}`. Sources are any of the 23+ computed statistics. Targets are audio parameters (pitch, gain, filter cutoff, etc.). Each route has independent EMA smoothing.

**Key insight**: Voice and Melody routes use `×2 scaling` — disconnected routes sit at `smoothed = 0.5`, giving ×1.0 (no change). Connecting a source that swings 0–1 gives 0×–2× modulation range.

### Feature Extraction

Six primary features drive the sound:

| Feature | Derived From | Drives |
|---------|-------------|--------|
| Median radius (r50) | 50th percentile of root distances from centroid | Carrier pitch |
| Spread (r90 − r10) | 90th minus 10th percentile | Filter cutoff (brightness) |
| Energy median (E_med) | 50th percentile of root velocities | Filter boost, LFO speed |
| Energy high (E_hi) | 85th percentile of root velocities | Gain (loudness), FM depth |
| Angular coherence (R) | Circular mean resultant length | Vibrato depth |
| Close encounters | Per-root top-3 closest distances ever | Beep triggers |

### Silence Management

Three mechanisms prevent orphaned audio:
1. **Watchdog timer** (100ms `setInterval`): fades masterGain to zero if `updateAudio()` hasn't been called in 150ms
2. **Visibility listener**: ramps to zero on tab hide
3. **`resetAudioState()`**: called on stop, home, pattern change, degree change, sound toggle off

---

## 6. UI Patterns & Conventions

### Popover System

All popovers (config, timing, audio, ops tools) use the `.ops-pop` CSS class. Positioning:

```javascript
const r = btnEl.getBoundingClientRect();
pop.style.left = r.left + "px";
pop.style.top = (r.bottom + 6) + "px";
```

Toggle via `.open` class. A global `document.addEventListener("mousedown", ...)` handler (around line 6267) closes open popovers when clicking outside.

### Tab System

Left panel has 6 tabs: C-Nodes, C-List, D-Nodes, D-List, Jiggle, Final. Switching calls `switchLeftTab(name)` which updates CSS classes and triggers content-specific refresh (e.g., `refreshCoeffList()` for C-List, `refreshDCoeffList()` for D-List, `renderMorphPanel()` for D-Nodes, `buildJigglePanel()` for Jiggle, `renderFinalPanel()` for Final).

Right panel has 4 tabs: Roots, Stats, Sound, Bitmap. Switching calls `switchTab(name)` which updates CSS classes and triggers resize/redraw for the newly visible tab.

### Save/Load

`saveState()` serializes everything to JSON: coefficients (with curves), trails, selections, audio config, routes, jiggle params, color mode, etc. `loadState()` parses JSON and restores all fields plus reconstructs UI state.

**When adding new state**: add it to both `saveState()` and `loadState()`. Provide defaults in `loadState()` for backward compatibility with old snapshots.

---

## 7. Bugs Found & Fixed

### The Sonification Silence Bug

**Symptom**: Enabling Base, Melody, or Voice during animation produces complete silence.

**Root cause**: `uiPing()` and `uiBuzz()` (UI feedback sounds for button clicks) create a bare `AudioContext` without building the sonification audio graph:

```javascript
function uiPing(freq = 880, dur = 0.08) {
    const ctx = audioCtx || new AudioContext();
    if (!audioCtx) audioCtx = ctx;  // Sets global audioCtx!
    // ... creates standalone oscillator, never sets audioNodes
}
```

When `toggleSound()` later calls `initAudio()`, the guard `if (audioCtx) return;` fires because `uiPing` already created the context. `audioNodes` stays `null`. `updateAudio()` checks `!audioNodes` and returns — silence.

**Fix**: Changed the guard from `if (audioCtx) return;` to `if (audioNodes) return;`:

```javascript
function initAudio() {
    if (audioNodes) return;  // Only skip if full graph exists
    if (!audioCtx) audioCtx = new AudioContext();
    // ... build full audio graph, set audioNodes = {...}
}
```

**Lesson**: When multiple code paths can create a shared resource (AudioContext), guard on the *derived state* (audioNodes), not the resource itself.

### The getImageData Performance Cliff

**Symptom**: Fast mode grinds to a halt above 2K resolution, even though worker computation is resolution-independent.

**Root cause**: `compositeWorkerPixels()` called `getImageData(0, 0, W, H)` and `putImageData(0, 0, W, H)` on the *entire* canvas every pass, even though only ~29,000 pixels (0.03% of a 10K canvas) were modified. At 10K, this copies 800MB of data per pass (400MB each way).

**Fix**: Persistent `ImageData` buffer (eliminates getImageData entirely) + dirty-rect putImageData (only flushes the region that changed).

**Lesson**: Always check whether full-canvas operations are necessary when only sparse updates are being made. The cost of `getImageData`/`putImageData` scales with canvas area, not with the number of pixels you care about.

### Fast Mode Video Recording

**Symptom**: Starting video recording then entering fast mode freezes the recorded video.

**Root cause**: `animLoop()` returns early when `fastModeActive` is true (line 3472), so `recordTick()` never fires. The bitmap canvas IS being updated by workers, but nobody captures frames.

**Fix**: Added `recordTick()` call in `handleFastModeWorkerMessage()` right after `compositeWorkerPixels()`. The function already has an early return when not recording, so zero cost in the common case.

**Lesson**: When bypassing the normal rendering loop (as fast mode does), check what other side effects that loop was responsible for.

### The NaN Root Poisoning Bug (Fast Mode "Sparse Dots")

**Symptom**: Certain polynomial configurations produce almost nothing in fast mode bitmap — just ~22 scattered dots per pass instead of dense pixel coverage. Interactive animation of the same polynomial works perfectly. Configs with many animated coefficients (e.g., all 23 on circle paths) consistently fail; configs with fewer animated coefficients (e.g., 10 of 30) work fine.

**Root cause**: `solveRootsEA()` **filters out non-finite roots** before returning:

```javascript
// THE BUG — old code
const result = [];
for (let i = 0; i < degree; i++) {
    if (isFinite(roots[i][0]) && isFinite(roots[i][1])) {
        result.push(r);  // Drops non-finite roots!
    }
}
return result;  // result.length <= degree
```

This causes a cascade failure in fast mode:

1. EA solver produces NaN/Inf for some roots (common with many animated coefficients pushing the solver into numerically nasty regimes)
2. Those roots get silently dropped → `currentRoots.length < degree`
3. Fast mode sets `nRoots = currentRoots.length` (too small)
4. Worker's `solveEA` computes `degree = nCoeffs - 1` (correct), but reads warm-start from a buffer of size `nRoots` → `warmRe[i]` for `i >= nRoots` returns `undefined` → Float64Array stores `NaN`
5. In the Aberth sum, every root depends on every other root: `Σ 1/(z_i - z_j)`. A single NaN root poisons the sum for ALL other roots within ONE iteration
6. After solver: all roots are NaN → all pixel coordinates are NaN → `NaN | 0 = 0` → everything maps to pixel (0,0) → almost nothing visible

**Why some configs work and others don't**: Configurations with all coefficients animated on circles (bug1.json) push the solver into regimes where NaN/Inf roots are more likely — the polynomial changes dramatically each step. Configs with many static coefficients (parallel_spirals.json) are more numerically stable, so the solver rarely produces non-finite roots and the filter has nothing to drop.

**Fix (two parts)**:

1. **Main thread `solveRootsEA`** — always return exactly `degree` roots. Non-finite roots fall back to warm-start values, then to unit-circle seeding:
```javascript
const result = new Array(degree);
for (let i = 0; i < degree; i++) {
    if (isFinite(roots[i][0]) && isFinite(roots[i][1])) {
        re = roots[i][0]; im = roots[i][1];
    } else if (warmStart && warmStart[i] && isFinite(warmStart[i].re) && isFinite(warmStart[i].im)) {
        re = warmStart[i].re; im = warmStart[i].im;
    } else {
        re = Math.cos(angle); im = Math.sin(angle);  // unit circle seed
    }
    result[i] = { re, im };
}
```

2. **Worker `solveEA`** — rescue non-finite roots after each solve call. Instead of only skipping the warm-start write for NaN roots, actively re-seed them on the unit circle so the next solver call has valid input.

**Lessons**:
- **Never silently change array length** in a function that feeds into a pipeline. Consumers assume `result.length === degree`. If you must handle invalid values, keep the array the same size and substitute fallback values.
- **NaN is viral in coupled numerical systems**. In Ehrlich-Aberth, a single NaN root corrupts all other roots in one iteration because of the `Σ 1/(z_i - z_j)` coupling term. Any NaN-producing bug is catastrophic, not graceful degradation.
- **Dimension mismatches between typed arrays produce silent NaN**. Reading `Float64Array[outOfBounds]` returns `undefined`, which silently becomes `NaN` when written to another Float64Array. No error, no warning — just corrupted computation.
- **Test with worst-case configurations**. The bug was invisible with "nice" polynomials (few animated coefficients, gentle speeds). It only appeared with high-degree polynomials where ALL coefficients were animated, stressing the solver's numerical limits.

---

## 8. Performance Insights

### What's Expensive

| Operation | Cost | Scales With |
|-----------|------|-------------|
| Ehrlich-Aberth solver | O(degree^2 × iters) per step | Polynomial degree |
| Root matching | O(degree^2) per call | Polynomial degree |
| `getImageData` (eliminated) | O(W × H × 4) | Canvas area |
| `putImageData` (dirty rect) | O(dirty_W × dirty_H × 4) | Root spread |
| Structured clone transfer | O(steps × degree × 13 bytes) | Steps per pass |
| Stats computation | O(degree × stats_count) | Polynomial degree |
| Domain coloring | O(pixels × degree × iters) | Canvas resolution × degree |

### Measured Performance (16 workers, degree 29, 1000 steps/pass)

| Resolution | Total Pass | Workers | Composite | Comp % |
|------------|-----------|---------|-----------|--------|
| 1K | 4.0 ms | 2.9 ms | 1.1 ms | 28% |
| 2K | 7.9 ms | 3.6 ms | 4.3 ms | 54% |
| 5K | 34.0 ms | 3.3 ms | 30.5 ms | 90% |
| 10K (before fix) | 125 ms | 8.4 ms | 116.4 ms | 93% |
| 10K (after fix) | 8.7 ms | 3.2 ms | 5.2 ms | 60% |

**Key takeaway**: After the persistent buffer fix, the bottleneck shifted back to worker computation — resolution no longer dominates pass time, which is the correct behavior.

### Worker Scaling

- 1 worker: ~2.15M steps/s
- 16 workers: ~15M steps/s (~7x, not 16x due to coordination overhead and diminishing returns)
- Worker count is capped at `navigator.hardwareConcurrency` or 16, whichever is lower

### What Doesn't Scale

- **matchRoots**: O(n^2) greedy matching. Called every 4th step in colored mode to amortize cost. At degree 100+, this becomes the bottleneck.
- **Domain coloring**: Evaluates the polynomial at every pixel. Throttled to one render per rAF, at 50% resolution.
- **Stats computation**: Linear in degree but runs every frame. Not a bottleneck until very high degree.

---

## 9. Coordinate Systems

### Complex Plane ↔ Screen

D3 linear scales map between the complex plane and SVG coordinates:

```javascript
xScale: [range.min, range.max] → [MARGIN, width - MARGIN]
yScale: [range.max, range.min] → [MARGIN, height - MARGIN]  // inverted: SVG y increases downward
```

Default range is [-2, 2]. Zoom/pan modifies `range.min` and `range.max`.

### Bitmap Coordinate System

The bitmap canvas freezes the viewport at activation time (`bitmapRange`). Workers receive this frozen range and map root positions to pixel coordinates independently. The main thread composites sparse pixel indices directly into the persistent buffer.

Pixel index = `y * W + x` (row-major). RGB channels sent separately (not interleaved RGBA).

---

## 10. Things That Surprised Me

1. **Jiggle applies to non-animated coefficients too** — not just the ones on paths. This was intentional but non-obvious.

2. **`"none"` path type creates a 1-point curve** at the home position, rather than being null. This simplifies the animation loop (no null checks).

3. **The watchdog timer uses `setInterval`, not `requestAnimationFrame`** — because rAF stops firing in background tabs, but the watchdog needs to detect silence even when the tab is hidden.

4. **Workers are persistent** — created once on fast mode init, reused for every pass. Only the step range changes per pass. This avoids worker startup latency.

5. **Root matching frequency matters enormously** — calling it every step vs every 4th step is the difference between smooth operation and stuttering at high degree.

6. **putImageData with dirty rect has a sixth argument** — `putImageData(imageData, dx, dy, dirtyX, dirtyY, dirtyW, dirtyH)`. The first three position the image; the last four specify which sub-rectangle to actually write. Not well documented.

7. **GC pressure from getImageData was slowing workers** — eliminating the per-pass 400MB `ImageData` allocation (from getImageData) reduced worker time from 8.4ms to 3.2ms, even though workers don't touch that allocation. The GC pauses were affecting the whole page.

8. **The FM synthesis modulation depth tracks kinetic energy** — at rest, the base instrument is a pure sine wave. As roots move faster, the FM modulation index increases, producing progressively richer harmonics. This is a nice sonification design: you literally hear the dynamics.

9. **Close encounter detection is per-root, not global** — each root tracks its own top-3 closest approach records. A beep fires only when a root beats its own record. This makes the system self-calibrating: tight configurations set low records early, so only exceptional approaches trigger.

10. **The structured clone overhead for worker communication is negligible** — at 29,000 pixels per pass, the transfer is ~130KB. The real cost was always in the canvas operations, not the data transfer.

11. **A single NaN root kills the entire Ehrlich-Aberth solver in one iteration** — because the Aberth correction sums `1/(z_i - z_j)` over all roots. If any `z_j` is NaN, the sum becomes NaN, the correction becomes NaN, and `z_i -= NaN` makes `z_i` NaN. After one iteration, all roots are NaN. This is fundamentally different from, say, Newton's method where one bad root wouldn't affect the others.

12. **Float64Array out-of-bounds reads return `undefined`, not an error** — and `undefined` silently converts to `NaN` when stored in another Float64Array. This makes dimension-mismatch bugs extremely hard to find: no exception, no console warning, just corrupted numbers propagating through computation. The `solveRootsEA` filter bug went undetected because it only triggered with specific polynomial configurations.

13. **Cycle detection is fundamentally broken for discrete systems** — the GCD-based `computeFullCyclePasses()` assumed periodic coefficient paths with rational speed ratios. Adding morph oscillation on top breaks this: even though C positions and mu return to start, the roots DON'T (because the blended polynomial traces a different path through coefficient space each cycle due to floating-point accumulation). More broadly, with finite precision and discrete steps, you can never reliably detect when roots have "returned to start." The honest solution is a user-settable target seconds value. Removed `computeFullCyclePasses()` entirely.

14. **Trail loop detection has the same fundamental flaw** — `trailComplete` checked if all roots returned within 1% of start positions after `TRAIL_MIN_POINTS` frames. With morph oscillation at 2 Hz, one cycle = 0.5s = 30 frames = exactly `TRAIL_MIN_POINTS`, so loop detection triggered immediately and killed trail collection. But even without morph, it was unreliable: floating-point drift, root identity swaps, and discrete sampling all create false positives/negatives. Removed entirely — trails now collect up to `MAX_TRAIL_POINTS` and stop only when the user disables them.

15. **Worker integration for new features is simpler than expected** — adding morph to fast mode workers required only: (a) serialize D positions as Float64Arrays in `serializeFastModeData()`, (b) pass them in the worker init message, (c) a 7-line blending loop after curve interpolation. No worker architecture changes, no WASM changes, no protocol changes. The key: morph blending happens in JS BEFORE the solver call, making it transparent to both JS and WASM solvers.

16. **SVG drag handlers must update ALL coupled visual elements** — when D dots are dragged in the morph panel, the interpolation lines (C→D) and markers (blended position) must also update. The initial implementation only updated the dot and its label, causing stale connecting lines. Each interactive element needs to consider what other elements depend on its position.

17. **Animation guards accumulate** — when adding new animation sources (morph), every guard that checks `allAnimatedCoeffs().size === 0` must be relaxed with `&& !morphEnabled`. Both `startAnimation()` and the Play button handler had this guard. Easy to miss the second one.

18. **`Set` iteration order is insertion order, not sorted order** — `[...selectedCoeffs]` returns indices in the order they were added to the Set, not numerically sorted. When applying transforms like LerpRadius that need deterministic index ordering (first selected → last selected), always sort: `[...selectedCoeffs].sort((a, b) => a - b)`.

19. **Coefficient array index = polynomial term power mapping** — `coefficients[0]` is the leading (highest power) term, `coefficients[n-1]` is the constant term. Subscript display is `c${n-1-i}` where `n = coefficients.length`. Adding a new coefficient via `unshift()` makes it the new highest power; adding via `push()` makes it the new constant term. Selection indices must be adjusted when coefficients are added/removed (all indices shift by ±1).

20. **Root trails must be cleared on coefficient add/delete** — when the polynomial's degree changes, old root trails from the previous polynomial are meaningless and visually misleading. `clearTrails()` must be called in both `addCoefficientAt()` and `deleteCoefficient()`.

21. **Transform dropdown pattern: execute-then-reset** — The List tab's Transform dropdown fires on `change`, applies the selected transform to `selectedCoeffs`, calls `solveRootsThrottled()` to update roots, then resets the dropdown back to `"none"`. This prevents accidental double-application and keeps the UI clean.

22. **Extensions must preserve original behavior when disabled** — `morphMu` defaulted to `0.5` because the plan optimized for "enable morph → immediately see 50/50 blend." But this meant that on startup (morph disabled), the morph panel displayed the interpolated marker at the midpoint between C and D instead of on C. The invariant: when a feature is OFF, all its state variables must reflect "as if the feature doesn't exist." For morph: `morphMu = 0` when disabled (marker on C, no blending). `morphMu = 0.5` only when explicitly enabled. Default values should match the disabled state, not the enabled state.

23. **"Cycle complete → restart" is the wrong model for open-ended exploration** — the original fast mode ran fixed-length cycles: compute N passes, exit, jiggle, re-enter from elapsed=0. This replayed the same morph phase every cycle. The fix: make fast mode continuous — elapsed ticks forever, jiggle fires periodically as a perturbation (not a cycle boundary). Stop/resume preserves all state. The only reset is explicit "init". Separate concerns: "init" = snapshot + clear + reset elapsed; "start/stop" = toggle computation; "clear" = clear pixels only. Each button does exactly one thing.

24. **GPU memory is the real constraint for large canvases, not CPU memory** — a 10K `<canvas>` allocates ~400MB of GPU memory (browser backing store) even though `ImageData` buffers are CPU-only. Chrome reclaims GPU contexts under memory pressure, causing the canvas to go white. The fix: decouple compute resolution from display resolution. The persistent buffer (`new ImageData(W, H)`) can be 15K or 25K (900MB–2.5GB CPU) because it never touches the GPU. The canvas only needs to be large enough for display (~2000px, 16MB GPU). Export writes BMP directly from CPU buffer — no GPU path at all. Key invariant: `serializeFastModeData()` must send `bitmapComputeRes` to workers, not `bitmapCtx.canvas.width` (which is now display-sized). The `new ImageData()` constructor creates a standalone buffer; `bitmapCtx.createImageData()` also works but `new ImageData()` makes the decoupling explicit.

25. **Decouple display-only controls from computation controls** — `rootColorMode` was a single variable controlling both SVG animation root dots AND bitmap worker pixel coloring. When the animation picker offered modes that only apply to bitmap rendering, users could select them and see no effect on the animation. The fix: separate `rootColorMode` (animation, 3 modes) from `bitmapColorMode` (bitmap, 4 modes) with independent uniform color variables. Workers receive flags (`noColor`, `proxColor`, `derivColor`) derived from `bitmapColorMode` — they never read the mode string directly, so no worker code changes were needed. The animation picker's swatches were simplified from a dynamic per-coefficient list to 8 fixed colors (`ROOT_COLOR_SWATCHES`), shared between both pickers. Backward compat: old snaps without `bitmapColorMode` fall back to mirroring `rootColorMode`.

26. **Progress bars add complexity without proportional value** — the fast mode progress bar required a `fastModeShowProgress` variable, a toggle button, save/load support, conditional blocks in 6+ functions, and two HTML elements. All to show a bar that fills and resets every ~1 second. A zero-padded elapsed seconds counter (`000042s`) conveys more useful information (total computation time) with zero complexity. When the visual feedback to complexity ratio is low, remove the feature entirely rather than maintaining it.

27. **Feature flags must cover all code paths, not just the obvious ones** — derivative coloring worked perfectly for SVG animation (`computeRootSensitivities()` → `sensitivityColor()`), but the bitmap pipeline was completely separate. When `bitmapColorMode === "derivative"` was selected, none of the worker flags (`noColor`, `proxColor`) were true, so workers silently fell through to rainbow (per-root index) coloring. The fix required a complete parallel implementation: `DERIV_PALETTE` (16-entry blue→white→red), `rankNorm()` + `computeSens()` functions in the worker blob, a `derivColor` flag in serialization, and new branches in `paintBitmapFrameFast()`. The lesson: when adding a new color mode, trace every code path that reads the mode — fast-mode painters, worker run loops, and serialization. A "mode" that only works in one pipeline is a bug, not a feature.

28. **Mirror features by mirroring ALL entry points, not just the obvious ones** — when adding D-node (morph target) path animation, the obvious entry point was `animLoop()`. But there are 5 places that advance/reset coefficient positions: `animLoop()`, `startAnimation()` (fresh-start snap), scrub slider handler, home button handler, and fast mode workers. Missing any one creates a sync bug: e.g., scrubbing would move C but freeze D, or home would reset C but leave D mid-path. The approach: enumerate ALL code paths that touch coefficient positions (search for `curve[0]`, `curveIndex`, `advanceDNodesAlongCurves`), then add D-node handling to each. For workers, D-curves must be pre-computed in `enterFastMode()`, serialized in `serializeFastModeData()`, parsed in the worker init handler, and advanced in the inner loop — each step must handle D independently from C. Pre-allocate `morphRe`/`morphIm` as copies of static D positions outside the step loop, then overwrite animated indices each step (avoids per-step full-array copy while keeping non-animated D positions correct).

29. **Expose algorithm choices as user-selectable strategies** — root matching for Index Rainbow mode was hardcoded to greedy nearest-neighbor every 4th step. This is a quality/speed tradeoff the user should control. The fix: add `bitmapMatchStrategy` with three options — Hungarian (O(n³), optimal, every step), Greedy×1 (O(n²), every step), Greedy×4 (O(n²), every 4th step, default). The Hungarian algorithm (Kuhn-Munkres) was added to the worker blob using `var`-only ES5-style syntax. The strategy is serialized as a simple string field, dispatched in the worker's rainbow branch via `S_matchStrategy`, and persisted in save/load with backward-compatible default. UI: small labeled chips under the "Index Rainbow" toggle in the bitmap cfg popup, following the same sub-option pattern as uniform color swatches. Key: the strategy only affects the rainbow branch — derivative mode keeps its own fixed matching logic.

30. **Duplicating HTML without duplicating CSS leaves invisible broken styling** — when mirroring the C-List curve editor HTML for the D-List tab, the inline styles on containers, buttons, and spans were copied correctly, but the CSS rules for dynamically-populated child elements were not. The C-List editor's `#lce-controls` had 6 CSS rules (flex layout, label font-size/color, range width, value display, direction button styling, active state) that gave its dynamically-built sliders and labels their compact 9px appearance. The D-List's `#dle-controls` div had identical HTML structure but zero CSS rules — so its labels, sliders, and direction buttons rendered with browser defaults (larger fonts, no flex layout, wrong colors). The fix: duplicate all `#lce-controls` rules as `#dle-controls` rules. The lesson: when cloning a UI component, search the stylesheet for every selector that targets the original's ID or class — inline styles only cover the static HTML, not the dynamically-generated children that CSS rules style.

---

## 11. Conventions to Follow

### Adding a New Popover

1. Add HTML in the body (near the other popovers, around line 909+)
2. Use class `ops-pop` for consistent styling
3. Position via `getBoundingClientRect()` on the trigger button
4. Toggle `.open` class to show/hide
5. Add close logic in the global mousedown handler (~line 6267)

### Adding New State to Save/Load

1. Add serialization in `buildStateMetadata()` (~line 6747)
2. Add deserialization in `applyLoadedState()` (~line 7554) with a default fallback for old snapshots
3. Test with old snapshot files to verify backward compatibility
4. If the state affects fast mode workers, also add it to `serializeFastModeData()` and the worker init message

### Adding a New Statistic

1. Add to `STAT_TYPES` array (line ~1138)
2. Add a color to `STAT_COLORS` map
3. Compute in the stats update section of `renderRoots()` or `updateStats()`
4. It automatically becomes available as an audio route source

### Adding a New Audio Route Target

1. Add a slot to the relevant instrument's routes array (`baseRoutes`, `melodyRoutes`, or `voiceRoutes`)
2. Add the parameter application in `updateAudio()` inside the appropriate instrument section
3. The routing UI auto-generates from the routes array

### Adding Higher Resolutions / Off-Canvas Changes

The bitmap system uses a **split compute/display** model (see [off-canvas-render.md](off-canvas-render.md)). Key invariants:

1. `bitmapComputeRes` is the source of truth for computation size — never use `bitmapCtx.canvas.width` for anything that workers or the persistent buffer depend on
2. `bitmapDisplayRes = Math.min(bitmapComputeRes, BITMAP_DISPLAY_CAP)` — the canvas and display buffer use this
3. `bitmapDisplayBuffer` is `null` when no split is needed (compute <= 2000px) — check this before using it
4. Worker pixel indices (`paintIdx`) are always in compute-space — the composite function handles downsampling
5. Export goes through format-specific functions (`exportPersistentBufferAs{BMP,JPEG,PNG,TIFF}()`) which read directly from the CPU persistent buffer — no GPU path. The save button opens a popup with format selection.

### Performance-Sensitive Code

- **No `Math.hypot` in hot loops** — use `re*re + im*im` directly
- **Avoid creating objects in tight loops** — reuse arrays, use flat typed arrays
- **Throttle expensive operations** — root matching configurable (Hungarian every step, greedy every step, or greedy every 4th step), domain coloring once per rAF
- **Profile before optimizing** — the timing popup (T button) shows per-pass breakdown

---

## 12. Testing & Debugging

### Test Suite

Automated tests exist in `tests/` using Playwright Python (headless Chromium). 492 tests across 23 files covering solver correctness, root matching, curve generation, path parametrics, shapes, polynomial operations, state save/load, stats, colors, utilities, morph system, jiggle perturbation (10 modes), continuous fast mode, off-canvas render split, multi-format image export, bitmap/animation color decoupling, derivative bitmap coloring, root-matching strategies (Hungarian algorithm, serialization, UI chips), D-node paths (D-List tab, animation helpers, D-curve serialization, backward compat), D-node context menu (open/close, path editing, revert), extended save/load fields, animation controls (play/pause/resume, home, scrub with D-nodes), trajectory editor simplification (preview/revert/commit, PS button removal, node cycler removal), Final tab (rendering, morph blending, trail data), integration, and JS vs WASM benchmarks. See [test-results.md](test-results.md) for details.

Manual testing remains important for:
- Dragging coefficients and roots
- Playing animations with various path types
- Toggling fast mode at different resolutions
- Enabling/disabling sonification instruments
- Save → load round-trip verification
- Checking the timing popup for performance regressions

### Debugging Tools Built In

1. **Timing popup** (T button in header): Shows per-pass breakdown — worker time, composite time, getImageData, putImageData, pixels painted
2. **Config popup** (cfg button in bitmap toolbar): Runtime tuning of worker count, steps per pass
3. **Stats tab**: 16 real-time plots of root constellation features
4. **Console logging**: Various `console.log` statements for worker lifecycle events (can be noisy)

### Common Debugging Scenarios

**"Fast mode is slow"**: Check the timing popup. If composite % is high, the bottleneck is canvas operations (check resolution). If worker % is high, the bottleneck is the solver (check degree, iterations).

**"No sound"**: Check that `audioNodes` is not null (the uiPing/initAudio ordering bug). Check `audioCtx.state` — might be "suspended" (needs user gesture). Check `masterGain.gain.value` — might be zero from watchdog.

**"Trails look wrong"**: Check root matching — if `matchRootOrder` is skipped or roots teleport, trails will have discontinuities. The jump threshold determines when a root is considered to have "teleported" vs moved continuously.

**"Save/load lost my setting"**: The new field probably isn't in `loadState()` yet. Add it with a default fallback.

**"Morph enabled but Play does nothing"**: Both `startAnimation()` and the Play button handler guard with `allAnimatedCoeffs().size === 0`. When morph is the only animation source (no paths), these guards must be relaxed with `&& !morphEnabled`.

**"Morph panel lines don't track during drag"**: The D-dot drag handler must update interpolation lines (x2/y2) and markers (blended position) in addition to the dot and label. Check the `morphDrag` handler.

**"Fast mode ignores morph"**: Check that `morphEnabled` is true and `morphTargetCoeffs.length === coefficients.length` — both conditions must hold for `serializeFastModeData()` to include morph data. Also verify the worker init message includes `morphTargetRe`/`morphTargetIm`.

**"Fast mode shows sparse dots / almost nothing"**: Check `currentRoots.length` vs `coefficients.length - 1` (degree). If they differ, the solver is dropping roots. Also check the worker's warm-start buffer size matches the expected degree. A dimension mismatch causes NaN poisoning in the EA solver — one NaN root kills all roots within one iteration due to the coupled Aberth sum. The fix is ensuring the solver always returns exactly `degree` roots, substituting fallback values for non-finite results.

---

## 13. File Index

### Documentation

| File | Topic | Lines |
|------|-------|-------|
| [solver.md](solver.md) | Ehrlich-Aberth algorithm, warm start, Horner eval, WASM | ~108 |
| [paths.md](paths.md) | Curve representation, path types, cycle sync, jiggle | ~315 |
| [sonification.md](sonification.md) | Audio graph, feature extraction, sound mapping, routing | ~251 |
| [braids.md](braids.md) | Root monodromy, topological permutations | ~17 |
| [patterns.md](patterns.md) | Coefficient/root patterns, gallery snapshots | ~98 |
| [worker_implementation.md](worker_implementation.md) | Fast mode protocol, worker lifecycle, data format | ~603 |
| [memory_timings.md](memory_timings.md) | Persistent buffer optimization: analysis + results | ~218 |
| [wasm_investigation.md](wasm_investigation.md) | WASM solver design, build workflow, benchmarks | ~240 |
| [off-canvas-render.md](off-canvas-render.md) | Split compute/display architecture, BMP export, GPU memory | ~240 |
| [d-node-paths.md](d-node-paths.md) | D-List tab, D-node path animation, fast mode workers, save/load | ~119 |
| [test-results.md](test-results.md) | Playwright test results + JS/WASM benchmarks | ~458 |
| [morph.md](morph.md) | Morph feature design: Phase 1 (static D), Phase 2/3 (deferred) | ~317 |
| [interface.md](interface.md) | Complete UI control reference | ~424 |
| [architecture.md](architecture.md) | This file | — |
| [archive/](archive/) | Archived docs: proof-read.md, worker_speed_issues.md | — |

### Key Code Locations

| Component | Approx Lines |
|-----------|-------------|
| `uiPing()` / `uiBuzz()` | ~1357 |
| `initAudio()` | ~1421 |
| `updateAudio()` | ~1485 |
| `drawGrid()` | ~1830 |
| `computeRootSensitivities()` (main thread) | ~2069 |
| `barSnapshots` / `previewBarToSelection()` | ~2641 / ~3509 |
| `revertBarPreview()` / `commitBarPreview()` | ~3560 / ~3581 |
| `addCoefficientAt()` | ~3097 |
| `deleteCoefficient()` | ~3123 |
| `allAnimatedDCoeffs()` | ~3372 |
| `advanceDNodesAlongCurves()` | ~3380 |
| `updateMorphPanelDDots()` | ~3403 |
| `animLoop()` | ~4265 |
| `renderCoefficients()` | ~4364 |
| `morphDrag` handler | ~4412 |
| `renderMorphPanel()` | ~4482 |
| `renderFinalPanel()` | ~4589 |
| `updateMorphMarkers()` | ~4671 |
| `advanceToElapsed()` | ~3724 |
| `updateAnimSeconds()` | ~3780 |
| `renderRoots()` | ~4769 |
| `renderCoeffTrails()` | ~4875 |
| `renderDomainColoring()` | ~4961 |
| `solveRootsEA()` | ~5057 |
| `matchRootOrder()` | ~5196 |
| `buildColorPop()` (animation) | ~5484 |
| `toggleSound()` | ~5736 |
| `openOpTool()` / `buildScaleTool()` / `buildRotateTool()` | ~6047 / ~6070 / ~6118 |
| Global mousedown close handler | ~6267 |
| `recordTick()` | ~6578 |
| `buildStateMetadata()` | ~6747 |
| `buildBitmapCfgPop()` (bitmap cfg + root color) | ~7024 |
| `buildBitmapSavePop()` | ~7279 |
| `saveState()` | ~7506 |
| `loadState()` / `applyLoadedState()` | ~7530 / ~7554 |
| `resizeStatsCanvases()` | ~7790 |
| `drawStatsPlot()` | ~8346 |
| `drawAllStatsPlots()` | ~8715 |
| `fillPersistentBuffer()` | ~8722 |
| `fillDisplayBuffer()` | ~8737 |
| `exportPersistentBufferAsBMP()` | ~8752 |
| `exportPersistentBufferAs{JPEG,PNG,TIFF}()` | ~8816 |
| `initBitmapCanvas()` | ~8842 |
| `createFastModeWorkerBlob()` | ~8954 |
| `hungarianMatch()` (worker blob) | ~9037 |
| `rankNorm()` + `computeSens()` (worker blob) | ~9083 |
| `compositeWorkerPixels()` | ~9946 |
| `bitmapColorMode` / `bitmapUniformColor` | ~1008 |
| `bitmapMatchStrategy` | ~1010 |
| `ROOT_COLOR_SWATCHES` | ~1013 |
| `DERIV_PALETTE` / `DERIV_PAL_R/G/B` | ~1057 |
| `fastModeDCurves` | ~1094 |
| `refreshCoeffList()` | ~10575 |
| `lceRefIdx` | ~11084 |
| `refreshListCurveEditor()` | ~11086 |
| `refreshDCoeffList()` | ~11391 |
| `dleRefIdx` | ~11873 |
| `refreshDListCurveEditor()` | ~11875 |
| `buildJigglePanel()` | ~12233 |
