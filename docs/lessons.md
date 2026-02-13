# Lessons Learned: PolyPaint Codebase

Everything discovered while working on this codebase — architecture, conventions, pitfalls, performance insights, and debugging war stories.

---

## 1. Architecture Overview

### Single-File Design

The entire application lives in one HTML file (`index.html`, ~10,400 lines). CSS is embedded in a `<style>` block (lines 8–513), HTML body is lines 515–787, and all JavaScript is inline in a single `<script>` block (lines 804–10439). There is no build step — serve the file directly.

**Why it works**: Zero tooling overhead, instant deployment to GitHub Pages, no import/bundling issues. D3.js and html2canvas are loaded from CDN.

**Why it hurts**: No modules means everything shares one scope. Finding a function means searching by name. Related code can be hundreds of lines apart. The file is too large for most editors' "go to definition" to work well.

### Section Map

| Section | Lines | What's There |
|---------|-------|-------------|
| CSS variables & layout | 8–513 | Colors, grid, popover, panel, animation styles |
| HTML body | 515–787 | Header, panels, SVG containers, 12+ popovers |
| Constants & config | 825–1100 | Audio params, stat types, color maps, defaults |
| Audio/sonification | 1014–1580 | FM synth, arpeggiator, encounters, routing |
| Panel setup & scales | 830–900 | D3 scale initialization, grid drawing |
| Coefficient data model | 868–880 | State arrays, selection sets, trail data |
| Rendering functions | 1621–3960 | SVG circles, trails, domain coloring, grid |
| Add/delete coefficients | 2659–2710 | Right-click canvas to add, context menu delete |
| Animation loop | 3631–3750 | `animLoop()`, path interpolation, throttled solve |
| Root solver | 4121–4310 | Ehrlich-Aberth, root matching, warm start |
| Event handlers | 4430–5400 | Mouse, keyboard, tab switch, popover toggle |
| Ops tools | 5100–5350 | Scale, rotate, translate — popover builders |
| Recording & snapshots | 5650–6310 | Video capture, save/load JSON, PNG export |
| Stats plotting | 7050–7430 | 16 time-series canvases, stat computation |
| Bitmap & fast mode | 7430–8500 | Persistent buffer, worker coordination, compositing |
| Web worker blob | 7582–8100 | Inline EA solver for workers |
| List tab & transforms | 9000–9400 | Coefficient table, Transform dropdown, bulk operations |
| Initialization | 10400–10439 | Default coefficients, first render, event wiring |

### External Dependencies

Only two, both from CDN:
- **D3.js v7** — SVG scaling, axes, drag behavior, selections
- **html2canvas v1.4.1** — PNG export of panels

### Canvas Architecture

Four distinct canvas systems coexist:

1. **SVG panels** (D3-managed) — Coefficient and root circles, trails, grid. Interactive (draggable).
2. **Domain canvas** (`#domain-canvas`) — Complex plane coloring overlay behind root SVG. Rendered at 50% resolution for performance.
3. **Bitmap canvas** (`#bitmap-canvas`) — High-res accumulation canvas (1K–15K px) for fast mode. Uses persistent `ImageData` buffer.
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

### Root State

- `currentRoots[]` — Array of `{re, im}` objects, one per root
- `rootSensitivities` — `Float64Array` for derivative-based coloring
- `trailData[]` — Array of arrays: `[[{re,im}, ...], ...]`, one inner array per root
- `closeEncounters` — `Float64Array(n * 3)` per-root top-3 closest distances

### Selection

Two `Set` objects: `selectedCoeffs` (indices into `coefficients[]`) and `selectedRoots` (indices into `currentRoots[]`). Most operations check `.size > 0` before enabling UI.

### List Tab

The List tab (`leftTab === "list"`) shows a tabular view of all coefficients with:
- **Selection buttons**: All, None, SameCurve, and a curve-type cycler
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
| Max iterations | 100 (main), 64 (worker/WASM) | Worker uses 64 for iteration color mode range |
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
  ├─ solveRootsThrottled()
  │   └─ requestAnimationFrame → solveRoots()
  │       └─ renderRoots(newRoots)
  │           ├─ Draw SVG circles
  │           ├─ Update trail data
  │           ├─ Compute stats
  │           └─ updateAudio()
  ├─ renderCoefficients()
  ├─ renderCoeffTrails()
  └─ requestAnimationFrame(animLoop)  [next frame]
```

**Fast mode is separate**: when `fastModeActive`, `animLoop()` returns early (line 3472). Workers drive the pipeline instead: `runFastModePass()` → workers solve → `handleFastModeWorkerMessage()` → `compositeWorkerPixels()` → `advancePass()`.

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

Toggle via `.open` class. A single global `document.addEventListener("click", ...)` handler (around line 5065) closes any open popover when clicking outside.

### Tab System

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

13. **`Set` iteration order is insertion order, not sorted order** — `[...selectedCoeffs]` returns indices in the order they were added to the Set, not numerically sorted. When applying transforms like LerpRadius that need deterministic index ordering (first selected → last selected), always sort: `[...selectedCoeffs].sort((a, b) => a - b)`.

14. **Coefficient array index = polynomial term power mapping** — `coefficients[0]` is the leading (highest power) term, `coefficients[n-1]` is the constant term. Subscript display is `c${n-1-i}` where `n = coefficients.length`. Adding a new coefficient via `unshift()` makes it the new highest power; adding via `push()` makes it the new constant term. Selection indices must be adjusted when coefficients are added/removed (all indices shift by ±1).

15. **Root trails must be cleared on coefficient add/delete** — when the polynomial's degree changes, old root trails from the previous polynomial are meaningless and visually misleading. `clearTrails()` must be called in both `addCoefficientAt()` and `deleteCoefficient()`.

16. **Transform dropdown pattern: execute-then-reset** — The List tab's Transform dropdown fires on `change`, applies the selected transform to `selectedCoeffs`, calls `solveRootsThrottled()` to update roots, then resets the dropdown back to `"none"`. This prevents accidental double-application and keeps the UI clean.

---

## 11. Conventions to Follow

### Adding a New Popover

1. Add HTML in the body (near the other popovers, around line 770+)
2. Use class `ops-pop` for consistent styling
3. Position via `getBoundingClientRect()` on the trigger button
4. Toggle `.open` class to show/hide
5. Add close logic in the global click handler (~line 5065)

### Adding New State to Save/Load

1. Add serialization in `saveState()` (~line 6281)
2. Add deserialization in `loadState()` (~line 6305) with a default fallback for old snapshots
3. Test with old snapshot files to verify backward compatibility

### Adding a New Statistic

1. Add to `STAT_TYPES` array (line ~955)
2. Add a color to `STAT_COLORS` map
3. Compute in the stats update section of `renderRoots()` or `updateStats()`
4. It automatically becomes available as an audio route source

### Adding a New Audio Route Target

1. Add a slot to the relevant instrument's routes array (`baseRoutes`, `melodyRoutes`, or `voiceRoutes`)
2. Add the parameter application in `updateAudio()` inside the appropriate instrument section
3. The routing UI auto-generates from the routes array

### Performance-Sensitive Code

- **No `Math.hypot` in hot loops** — use `re*re + im*im` directly
- **Avoid creating objects in tight loops** — reuse arrays, use flat typed arrays
- **Throttle expensive operations** — root matching every 4th step, domain coloring once per rAF
- **Profile before optimizing** — the timing popup (T button) shows per-pass breakdown

---

## 12. Testing & Debugging

### Test Suite

Automated tests exist in `tests/` using Playwright Python (headless Chromium). 38 tests covering solver correctness, root matching, curve generation, integration, and JS vs WASM benchmarks. See [test-results.md](test-results.md) for details.

Manual testing remains important for:
- Dragging coefficients and roots
- Playing animations with various path types
- Toggling fast mode at different resolutions
- Enabling/disabling sonification instruments
- Save → load round-trip verification
- Checking the timing popup for performance regressions

### Debugging Tools Built In

1. **Timing popup** (T button): Shows per-pass breakdown — worker time, composite time, getImageData, putImageData, pixels painted
2. **Config popup** (C button): Runtime tuning of worker count, steps per pass
3. **Stats tab**: 16 real-time plots of root constellation features
4. **Console logging**: Various `console.log` statements for worker lifecycle events (can be noisy)

### Common Debugging Scenarios

**"Fast mode is slow"**: Check the timing popup. If composite % is high, the bottleneck is canvas operations (check resolution). If worker % is high, the bottleneck is the solver (check degree, iterations).

**"No sound"**: Check that `audioNodes` is not null (the uiPing/initAudio ordering bug). Check `audioCtx.state` — might be "suspended" (needs user gesture). Check `masterGain.gain.value` — might be zero from watchdog.

**"Trails look wrong"**: Check root matching — if `matchRootOrder` is skipped or roots teleport, trails will have discontinuities. The jump threshold determines when a root is considered to have "teleported" vs moved continuously.

**"Save/load lost my setting"**: The new field probably isn't in `loadState()` yet. Add it with a default fallback.

**"Fast mode shows sparse dots / almost nothing"**: Check `currentRoots.length` vs `coefficients.length - 1` (degree). If they differ, the solver is dropping roots. Also check the worker's warm-start buffer size matches the expected degree. A dimension mismatch causes NaN poisoning in the EA solver — one NaN root kills all roots within one iteration due to the coupled Aberth sum. The fix is ensuring the solver always returns exactly `degree` roots, substituting fallback values for non-finite results.

---

## 13. File Index

### Documentation

| File | Topic | Lines |
|------|-------|-------|
| [solver.md](solver.md) | Ehrlich-Aberth algorithm, warm start, Horner eval, WASM | ~109 |
| [paths.md](paths.md) | Curve representation, path types, cycle sync, jiggle | ~237 |
| [paths_nn.md](paths_nn.md) | Path notes (draft) | ~25 |
| [sonification.md](sonification.md) | Audio graph, feature extraction, sound mapping, routing | ~233 |
| [braids.md](braids.md) | Root monodromy, topological permutations | ~10 |
| [patterns.md](patterns.md) | Coefficient/root patterns, gallery snapshots | ~99 |
| [worker_implementation.md](worker_implementation.md) | Fast mode protocol, worker lifecycle, data format | ~546 |
| [worker_speed_issues.md](worker_speed_issues.md) | Timing bug fix, resolution scaling analysis | ~189 |
| [memory_timings.md](memory_timings.md) | Persistent buffer optimization: analysis + results | ~219 |
| [wasm_investigation.md](wasm_investigation.md) | WASM solver design, build workflow, benchmarks | ~251 |
| [test-results.md](test-results.md) | Playwright test results + JS/WASM benchmarks | ~122 |
| [lessons.md](lessons.md) | This file | — |

### Key Code Locations

| Component | Approx Lines |
|-----------|-------------|
| `initAudio()` | ~1208 |
| `updateAudio()` | ~1272 |
| `uiPing()` / `uiBuzz()` | ~1173 |
| `addCoefficientAt()` | ~2659 |
| `deleteCoefficient()` | ~2684 |
| `animLoop()` | ~3631 |
| `renderRoots()` | ~3831 |
| `solveRootsEA()` | ~4121 |
| `matchRootOrder()` | ~4272 |
| `toggleSound()` | ~4803 |
| `saveState()` | ~6281 |
| `loadState()` | ~6305 |
| `fillPersistentBuffer()` | ~7437 |
| `initBitmapCanvas()` | ~7452 |
| `createFastModeWorkerBlob()` | ~7582 |
| `compositeWorkerPixels()` | ~8392 |
| List tab transforms | ~9004 |
