# Architecture Notes

Architecture, conventions, performance insights, and debugging notes for the PolyPaint codebase.

---

## 1. Architecture Overview

### Single-File Design

The entire application lives in one HTML file (`index.html`, ~13,900 lines). CSS is embedded in a `<style>` block (lines 12–560), HTML body is lines 562–930, and all JavaScript is inline in a single `<script>` block (lines 931–13910). There is no build step — serve the file directly.

**Why it works**: Zero tooling overhead, instant deployment to GitHub Pages, no import/bundling issues. D3.js and image-encoding libraries are loaded from CDN.

**Why it hurts**: No modules means everything shares one scope. Finding a function means searching by name. Related code can be hundreds of lines apart. The file is too large for most editors' "go to definition" to work well.

### Section Map

| Section | Lines | What's There |
|---------|-------|-------------|
| CSS variables & layout | 12–560 | Colors, grid, popover, panel, animation styles |
| HTML body | 562–930 | Header (incl. play/scrub/home controls), panels, SVG containers, 15 popovers |
| Constants & config | 931–1210 | Audio params, stat types, color maps, bitmap color modes, palettes, fast mode state, morph state, WASM base64 blobs |
| Audio/sonification | 1210–1825 | FM synth, arpeggiator, encounters, routing |
| Rendering foundations | 1826–2125 | Grid drawing, panel setup, scales, sensitivity/derivative computation |
| Patterns & init coefficients | 2125–2425 | Coefficient patterns, root shape generation |
| Drag & selection | 2425–2900 | D3 drag, click/select logic, mutually exclusive C/D/root selection, ops target indicator |
| Anim-bar trajectory editor | 2534–2700 | Bar snapshots, preview/revert/commit for selection |
| Add/delete coefficients | 3097–3200 | Right-click canvas to add, context menu delete |
| D-node animation helpers | 3372–3470 | `allAnimatedDCoeffs()`, `advanceDNodesAlongCurves()`, `updateMorphPanelDDots()` |
| Play/Scrub/Home controls | 3719–3860 | `playBtn` click cycle (Play/Pause/Resume), `homeBtn`, scrub slider handler, `advanceToElapsed()`, `updateAnimSeconds()` |
| Animation loop | 4318–4445 | `startAnimation()`, `animLoop()`, `stopAnimation()` |
| Coefficient rendering | 4455–4500 | `renderCoefficients()` |
| Morph panel & rendering | 4500–4680 | Ghost layer, D dots, interp lines/markers, drag handler |
| Final tab | 4682–4770 | `renderFinalPanel()`, `updateMorphMarkers()` |
| Roots rendering | 4863–4965 | `renderRoots()` |
| Root solver | 5151–5350 | Ehrlich-Aberth, root matching, warm start |
| Color popover & toggling | 5599–5860 | `buildColorPop()` (animation), `toggleSound()` |
| Ops tools | 6075–6400 | Scale, rotate, translate, shape morph, pattern arrange (`PTRN_PARAMS`, `opCloseCallback`) — supports C/D/root targets |
| Recording & snapshots | 7358–7530 | Video capture, `recordTick()` |
| Save/Load | 7529–8425 | `buildStateMetadata()`, `saveState()`, `loadState()`, `applyLoadedState()` |
| Bitmap config & save popovers | 7807–8400 | `buildBitmapCfgPop()`, `buildBitmapSavePop()`, 6 bitmap color mode UI |
| Stats plotting | 9170–9660 | 16 time-series canvases, phase-space plots, spectrum plots |
| Off-canvas bitmap utilities | 9660–9780 | `fillPersistentBuffer()`, `fillDisplayBuffer()`, export functions (BMP/JPEG/PNG/TIFF) |
| Bitmap canvas init | 9780–9910 | `initBitmapCanvas()`, split compute/display setup |
| Web worker blob | 9907–10880 | Inline EA solver, Hungarian matcher, rankNorm, computeSens, WASM step loop init + layout, JS step loop, persistent worker state, init/run message handler |
| Fast mode entry/exit | 10881–11430 | `enterFastMode()`, `serializeFastModeData()`, `initFastModeWorkers()`, `dispatchPassToWorkers()`, `handleFastModeWorkerMessage()`, `compositeWorkerPixels()`, `advancePass()`, `reinitWorkersForJiggle()`, `exitFastMode()` |
| Tab switching | 11649–11710 | `switchTab()`, `switchLeftTab()` |
| C-List tab & transforms | 11915–12430 | Coefficient table, Transform dropdown (20 transforms), bulk operations |
| D-List tab | 12713–13180 | D-node table, D-list curve editor |
| Jiggle panel | 13593–13828 | `nearestPrime()`, `buildJigglePanel()` (12 modes, `-1`/`+1`/`P` step buttons) |
| Initialization | 13829–13910 | `rebuild()`, default coefficients, first render, event wiring |

### External Dependencies

All from CDN:
- **D3.js v7** — SVG scaling, axes, drag behavior, selections, d3 color interpolators for proximity palettes
- **html2canvas v1.4.1** — PNG export of panels
- **Pako v2.1.0** + **UPNG.js v2.1.0** — PNG encoding from RGBA data (Pako provides deflate)
- **jpeg-js v0.4.4** — JPEG encoding from RGBA data (global: `window['jpeg-js'].encode`)
- **UTIF.js v3.1.0** — TIFF encoding from RGBA data (uses `self.pako` if available)

### Canvas Architecture

Four distinct canvas systems coexist:

1. **SVG panels** (D3-managed) — Coefficient and root circles, trails, grid. Interactive (draggable).
2. **Domain canvas** (`#domain-canvas`) — Complex plane coloring overlay behind root SVG. Rendered at 50% resolution for performance.
3. **Bitmap canvas** (`#bitmap-canvas`) — High-res accumulation canvas (1K–25K px) for fast mode. Uses a **split compute/display** architecture: at resolutions above 2000px, the canvas is capped at 2000px (display) while a CPU-only `ImageData` persistent buffer holds the full compute resolution. Workers always compute in compute-space; `compositeWorkerPixels()` downsamples to the display buffer. Export supports JPEG/PNG/BMP/TIFF via pure-JS encoders directly from the CPU buffer — no GPU involvement. The save button opens a format popup with quality slider for JPEG. See [off-canvas-render.md](off-canvas-render.md).
4. **Stats canvases** — 16 small canvases in a 4x4 grid, each plotting a time-series, phase-space, or spectrum statistic.

### Source Files

| File | Purpose |
|------|---------|
| `index.html` | The entire application (~13,900 lines) |
| `step_loop.c` | Full worker step loop in C: EA solver, curve interpolation, root matching, pixel output |
| `solver.c` | Historical solver-only WASM source (no longer embedded, kept for reference) |
| `build-wasm.sh` | Compiles `step_loop.c` to WASM via Homebrew LLVM, produces `.wasm.b64` file |
| `step_loop.wasm` / `step_loop.wasm.b64` | Compiled full step loop WASM binary and base64-encoded version |

---

## 2. Data Model

### Coefficient Objects

Each coefficient is a plain object with these fields:

```
{ re, im, pathType, radius, speed, angle, ccw, extra, curve, curveIndex }
```

- `re`, `im` — Current complex position
- `pathType` — Animation type: `"none"`, `"follow-c"` (D-nodes only), `"circle"`, `"spiral"`, `"lissajous"`, etc.
- `radius` — Path radius (0–100, as % of panel extent)
- `speed` — Animation speed (internal 0–1.0 float, displayed as integer 0–1000 via `speed * 1000`, resolution 1/1000)
- `angle` — Starting phase (0–1 turns)
- `ccw` — Counter-clockwise flag
- `extra` — Path-specific params (object, varies by pathType)
- `curve` — Pre-computed array of `{re, im}` points (N samples of the closed path)
- `curveIndex` — Current integer index into `curve[]`

**Important**: `"none"` path type means a 1-point curve at the coefficient's home position. It is NOT null — always check `pathType`, never check `curve == null`. `"follow-c"` is a D-node-only path type that mirrors the corresponding C-node's current position — treated like `"none"` for curve generation (1-point curve), but workers track `S_dFollowC` indices to copy C positions into D each step.

### Morph System

Coefficient morphing blends two coefficient sets: primary C (`coefficients[]`) and target D (`morphTargetCoeffs[]`). When enabled, the solver receives `C[i]*(1-mu) + D[i]*mu` where `mu = 0.5 - 0.5*cos(2*pi*morphRate*elapsed)`.

Key state: `morphEnabled`, `morphRate` (Hz, 0.01–2.00), `morphMu` (0–1), `morphTargetCoeffs[]` (same structure as `coefficients[]`). D-nodes can be assigned paths via the D-List tab, making the morph target itself dynamic.

**Design insight**: Morph is a "global path" — it affects all coefficients equally through a single parameter. Unlike per-coefficient paths which are independent, morph creates correlated perturbation across the entire polynomial. The blending happens BEFORE the solver call, making it transparent to both JS and WASM solvers.

**Fast mode**: Workers receive D positions in the init message and compute mu per-step. The blending loop is inserted between curve interpolation and solver call in both the JS step loop and the WASM step loop.

**Continuous fast mode**: Fast mode runs continuously until stopped. `jiggleInterval` (0.1–100s, cfg popup) controls how often jiggle perturbations fire — not a cycle length. "init" button snapshots animation state + clears bitmap + resets elapsed. "start"/"pause" toggles computation. Stopping preserves elapsed; resuming continues. "clear" only clears pixels. Elapsed seconds shown as zero-padded counter. Changing the bitmap steps or resolution selects (`bitmap-steps-select`, `bitmap-res-select`) while fast mode is active triggers an automatic restart (`exitFastMode()` + `enterFastMode()`) via dedicated `change` event listeners.

### Root State

- `currentRoots[]` — Array of `{re, im}` objects, one per root
- `rootSensitivities` — `Float64Array` for derivative-based coloring
- `trailData[]` — Array of arrays: `[[{re,im}, ...], ...]`, one inner array per root
- `closeEncounters` — `Float64Array(n * 3)` per-root top-3 closest distances

### Selection

Three `Set` objects: `selectedCoeffs` (indices into `coefficients[]`), `selectedRoots` (indices into `currentRoots[]`), and `selectedMorphCoeffs` (indices into `morphTargetCoeffs[]`). Selections are **mutually exclusive** — selecting a C-node clears D-node and root selections, selecting a D-node clears C-node and root selections, and selecting a root clears both C and D selections. This is enforced by `toggleCoeffSelect()`, D-node click handler, and root click handler each calling `clearRootSelection()` / `clearMorphSelection()` / `clearCoeffSelection()` as appropriate. Lasso drag on each panel also clears the other two.

The ops mid-bar shows a colored indicator of the active selection target: green "C" for C-nodes, blue "D" for D-nodes, red "roots" for root selection, or gray "none" when nothing is selected (`updateTransformGroupVisibility()`). The ops group is disabled when no selection exists.

### C-List Tab

The C-List tab (`leftTab === "list"`) shows a tabular view of all coefficients with:
- **Selection buttons**: All, None, SameCurve, and a curve-type cycler (cycles path types, not individual nodes)
- **Transform dropdown** (20 transforms): Applies a one-shot transform to `selectedCoeffs`, then resets to "none". Includes PrimeSpeeds, SetAllSpeeds, RandomSpeed, RandomAngle, RandomRadius, LerpSpeed, LerpRadius, LerpAngle, RandomDirection, FlipAllDirections, ShuffleCurves, ShufflePositions, CircleLayout, RotatePositions, ScalePositions, JitterPositions, Conjugate, InvertPositions, SortByModulus, SortByArgument.
- **Param1/Param2 sliders**: Passive inputs that transforms read when executed. Param2 range is 1–1000.
- **Per-coefficient columns**: Index with color dot, position (re, im), speed (spd), radius (rad), curve length (pts), curve position (pos).

### Add/Delete Coefficients

- **Right-click on empty canvas space** → `addCoefficientAt(re, im, event)` creates a new highest-power coefficient via `unshift()`, adjusts selection indices +1, clears trails, opens context menu on index 0.
- **Right-click on existing coefficient** → context menu with path editing. **Delete** button (red-styled) removes the coefficient, with a guard preventing deletion below degree 2 (3 coefficients minimum, `coefficients.length <= 3`).
- Both operations call `clearTrails()` and `solveRoots()` to keep the root display consistent.
- Minimum degree is 2 (enforced by `setDegree()` which clamps to `Math.max(2, ...)`; degree popover range is 2–30).

---

## 3. Core Algorithms

### Ehrlich-Aberth Solver

Simultaneous iterative root finder with cubic convergence. Key parameters:

| Parameter | Value | Notes |
|-----------|-------|-------|
| Max iterations | 100 (main), 64 (worker/WASM) | 64 gives ample margin with warm-starting |
| Tolerance | 1e-12 magnitude (main), 1e-16 squared (worker/WASM) | Worker uses tighter tolerance for accuracy |
| Leading-zero test | `Math.hypot` < 1e-15 (main), magnitude-squared < 1e-30 (worker/WASM) | Main uses Math.hypot; workers use manual |
| Hot loop optimization | No `Math.hypot` in worker/WASM | Workers use `re*re + im*im` directly |
| WASM option | Compiled C solver + full step loop | Selectable via cfg button in bitmap toolbar |

**Warm starting** is critical: reusing previous roots as initial guesses cuts iterations from ~20 to ~3-5 for small coefficient movements. This is what makes interactive dragging feel instant.

**NaN handling**: The solver always returns exactly `degree` roots. Non-finite results (which occur in numerically difficult regimes, especially with many rapidly-moving coefficients) fall back to warm-start values, then to unit-circle seeding. This is essential because the Aberth correction couples all roots — a single NaN propagates to every root within one iteration. The worker solver also actively re-seeds non-finite roots in the warm-start buffer after each call.

**Root matching** (`matchRootOrder`) uses greedy nearest-neighbor after each solve to preserve root identity across frames. Called every 4th step in colored mode because it's O(n^2).

### Animation Pipeline

```
animLoop()
  |-- Update coefficient positions along curves (elapsed time -> curveIndex)
  |-- Advance D-nodes along their paths (advanceDNodesAlongCurves)
  |-- Recompute jiggle offsets if step changed
  |-- Update morphMu (cosine oscillation) if morphEnabled
  |-- solveRootsThrottled()
  |     +-- requestAnimationFrame -> solveRoots()
  |         |-- Blend C with D if morphEnabled: coeffs[i] = C[i]*(1-mu) + D[i]*mu
  |         +-- renderRoots(newRoots)
  |             |-- Draw SVG circles
  |             |-- Update trail data
  |             |-- Compute stats
  |             +-- updateAudio()
  |-- Update morph panel visuals (ghosts, lines, markers) if morph tab visible
  |-- renderCoefficients() (animated nodes only)
  |-- renderDomainColoringThrottled()
  |-- recordTick()
  |-- updateListCoords() / updateDListCoords()
  |-- updateAnimSeconds(elapsed)
  +-- requestAnimationFrame(animLoop)  [next frame]
```

### Play/Scrub/Home Controls

These controls live in the **header bar** (not the left tab bar), inside `<span id="anim-controls">`:

- **Scrub slider** (`#scrub-slider`): Range 0–5000, adds virtual seconds when paused. Drives `advanceToElapsed()` which snaps all C and D nodes to their curve positions at the given elapsed time. Updates `animState.elapsedAtPause` to enable seamless resume from scrubbed position.
- **Play button** (`#play-btn`): Three-state cycle: Play (fresh start) -> Pause (preserves elapsed) -> Resume (continues from paused time). Text updates: "Play" / "Pause" / "Resume".
- **Elapsed counter** (`#anim-seconds`): Shows `0.00s` format, updated by `updateAnimSeconds()` every frame.
- **Home button** (`#home-btn`): Stops animation, resets all C and D nodes to their `curve[0]` positions, resets `animState.elapsedAtPause` and scrub slider to 0.

**Fast mode is separate**: when `fastModeActive`, `animLoop()` returns early. Workers drive the pipeline instead: `dispatchPassToWorkers()` -> workers run step loop (curve interpolation + morph blend + solve + pixel gen) -> `handleFastModeWorkerMessage()` -> `compositeWorkerPixels()` -> `advancePass()`. Fast mode runs continuously — `advancePass()` calls `reinitWorkersForJiggle()` every `jiggleInterval` passes (if jiggle enabled), which regenerates offsets and recomputes curves without resetting elapsed time.

---

## 4. Fast Mode & Web Workers

### Architecture

Workers are created as blob URLs from inline code (no separate `.js` file). Each worker receives the full polynomial on `init`, then gets `{stepStart, stepEnd}` ranges on each `run` message. Steps are distributed using balanced floor division: `base = Math.floor(stepsVal / nw)` with the remainder distributed one extra step to the first `stepsVal % nw` workers. The actual worker count is capped at `Math.min(numWorkers, stepsVal)` so that low step counts (e.g., 100 steps with 16 workers) don't create workers with zero steps.

**Two-tier solver selection per worker**: On init, each worker attempts (in order):
1. **WASM step loop** (`step_loop.wasm`) — full step loop in WASM: curve interpolation, morph blend, solve, root matching, pixel output all run in WASM. Falls back if unsupported color mode (idx-prox, ratio).
2. **Pure JS** — everything in JavaScript.

The selection is per-worker and transparent to the main thread — the `done` message format is identical regardless of which tier executed.

**Data flow per pass**:
```
Workers (parallel)                    Main Thread
--------------------                  ---------------
Step loop (WASM or JS):
  Interpolate C-curves  ---|
  Interpolate D-curves     |
  Follow-C copy            |
  Morph blend              |
  Apply jiggle offsets     |--> structured clone --> handleFastModeWorkerMessage
  EA solve                 |    (paintIdx, R,G,B)         |
  NaN rescue               |                              v
  Root matching            |                     compositeWorkerPixels()
  Color-mode pixel gen     |                       1. Write sparse pixels to persistent buffer
  x steps_per_worker       |                       2. Track dirty rect
                           |                       3. putImageData(dirty region only)
                           +-------------------------------v
                                                     Canvas updated
```

### Worker Blob Structure

The worker blob (~line 9907) contains all code inlined as a template string:

1. **JS EA solver** (`solveEA`) — flat-array Ehrlich-Aberth, 64 iterations, 1e-16 tolerance
2. **Greedy root matching** (`matchRoots`) — O(n^2) nearest-neighbor
3. **Hungarian matching** (`hungarianMatch`) — Kuhn-Munkres O(n^3), optimal assignment
4. **Derivative sensitivity** (`rankNorm`, `computeSens`) — Jacobian sensitivity + rank normalization
5. **WASM step loop init** (`computeWasmLayout`, `initWasmStepLoop`) — compiles step_loop.wasm with imported memory, computes flat memory layout for all data sections, writes config + data into WASM memory
7. **WASM step loop run path** — copies warm-start roots into WASM memory, calls `runStepLoop()`, reads sparse pixel output + final roots
8. **JS step loop fallback** — full step loop: curve interpolation, D-curve interpolation, follow-C copy, morph blend, jiggle offsets, solve, NaN rescue, color-mode-dependent root matching + pixel output
9. **Persistent state** (`S_*` variables) — set by `init`, reused across `run` calls
10. **`onmessage` handler** — dispatches `init` (parse + store) and `run` (execute step loop, post results)

### WASM Step Loop (`step_loop.c`)

The full step loop was ported to C and compiled to WASM (~817 lines). This eliminates per-step JS-to-WASM boundary crossing — the entire inner loop runs in WASM with only a single call per `run` message.

**What `step_loop.c` contains**:
- EA solver (ported from `solver.c`)
- Greedy root matching
- Hungarian root matching (capped at degree 32 for stack safety)
- Derivative sensitivity computation (`computeSens`, `rankNorm`)
- Curve interpolation (C-curves and D-curves, with cloud/interpolation modes)
- Follow-C copy
- Morph blend (cosine mu formula)
- Jiggle offset application
- NaN rescue (unit-circle re-seeding)
- All four color modes: Uniform (0), Index Rainbow (1), Proximity (2), Derivative (3)
- Progress reporting via imported JS function
- xorshift128 PRNG for dither (avoids importing `Math.random`)

**Memory layout**: WASM uses imported memory. The JS side computes a flat layout (`computeWasmLayout`) with 65 int32 config values and 3 float64 config values, followed by all data arrays (coefficients, colors, jiggle, morph targets, palettes, curve data, working arrays, output buffers). Memory grows dynamically based on layout requirements. The shadow stack is 64KB (stack-first linker flag).

**Unsupported modes**: The WASM step loop does not implement Idx x Prox or Min/Max Ratio color modes. When these are selected, `S_useWasmLoop` is forced false and the JS step loop runs instead.

**Build workflow**: `./build-wasm.sh` compiles `step_loop.c` -> `step_loop.wasm` using Homebrew LLVM. The `.wasm.b64` file is then pasted into `WASM_STEP_LOOP_B64` in `index.html`.

### Sparse Pixel Format

Workers send `{paintIdx: Int32Array, paintR/G/B: Uint8Array, paintCount}`. This is vastly more efficient than sending W x H x 4 RGBA buffers — a pass painting 29,000 pixels sends ~130KB vs 400MB at 10K resolution.

### Persistent Buffer Optimization

The biggest performance win in the codebase. Before: `getImageData` + `putImageData` on the full canvas every pass. After: persistent `ImageData` buffer + dirty-rect `putImageData`.

Results at 10K resolution: **14.4x speedup** (125ms -> 8.7ms per pass). See [memory_timings.md](memory_timings.md) for full analysis.

### Why Not SharedArrayBuffer?

Requires `Cross-Origin-Opener-Policy` and `Cross-Origin-Embedder-Policy` headers. GitHub Pages doesn't support custom headers. Structured clone of sparse data is fast enough.

### Why Not OffscreenCanvas?

Good for single worker, but compositing from multiple workers into one canvas requires either:
- Layered `OffscreenCanvas` per worker (memory explosion at high res), or
- Single shared `OffscreenCanvas` with synchronization (complex)

Deferred as not worth the complexity given sparse pixels work well.

---

## 5. Color Modes

### Animation Color Modes (`rootColorMode`)

Controls SVG animation root dot coloring. Three modes:
- **Uniform** — all roots same color (from `uniformRootColor`, 8-color swatch picker)
- **Index Rainbow** — `d3.interpolateRainbow(i / n)`, one hue per root
- **Derivative** — Jacobian sensitivity coloring via `computeRootSensitivities()` -> `sensitivityColor()`, blue (stable) -> white -> red (volatile)

### Bitmap Color Modes (`bitmapColorMode`)

Controls fast-mode bitmap pixel coloring. Six modes:
- **Uniform** — all pixels same color (from `bitmapUniformColor`)
- **Index Rainbow** — per-root rainbow with configurable matching strategy
- **Derivative** — Jacobian sensitivity via `computeSens()` + `rankNorm()` in worker, 16-entry `DERIV_PALETTE` (blue -> white -> red)
- **Root Proximity** — nearest-neighbor distance mapped to 16-entry proximity palette (8 d3 palette options: Inferno, Viridis, Magma, Plasma, Turbo, Cividis, Warm, Cool)
- **Idx x Prox** — Index Rainbow hue with proximity-based brightness, configurable gamma (0.1-1.0). JS-only (not in WASM step loop).
- **Min/Max Ratio** — distance ratio coloring with configurable gamma. JS-only (not in WASM step loop).

Workers receive flags (`noColor`, `proxColor`, `derivColor`, `idxProxColor`, `ratioColor`) derived from `bitmapColorMode` — they never read the mode string directly.

### Root-Matching Strategies

For Index Rainbow bitmap mode, `bitmapMatchStrategy` offers three options:
- **Greedy x4** (`assign4`) — O(n^2) greedy matching every 4th step (default)
- **Greedy x1** (`assign1`) — O(n^2) greedy matching every step
- **Hungarian x1** (`hungarian1`) — O(n^3) Kuhn-Munkres optimal matching every step

The Hungarian algorithm is implemented in both the JS worker blob and `step_loop.c` (capped at degree 32 in WASM for stack safety, falling back to greedy above that).

---

## 6. Sonification System

### Three Instruments

| Instrument | Synthesis | Triggered By |
|-----------|-----------|-------------|
| **Base** (B) | FM synthesis: sine carrier + sine modulator | Continuous — root constellation shape |
| **Melody** (M) | Triangle wave arpeggiator, pentatonic scale | Per-frame — top-N fastest roots |
| **Voice** (V) | Sine beep with attack/decay envelope | Event — record-breaking close encounters |

### Audio Graph

```
[modulator: sine] --> [modGain] --> carrier.frequency
                                                          +---> speakers
[carrier: sine 110Hz] --> [gainNode] --> [lowpass] --> [masterGain]+
                                                          +---> [mediaDest] --> recording

[beepOsc: sine] --> [beepGain] --> [masterGain]

[arpOsc: triangle] --> [arpGain] --> [arpFilter: lowpass] --> [masterGain]

[lfo: sine 1.5-7.5Hz] --> [lfoGain] --> carrier.frequency
```

### Signal Routing

Each instrument has configurable routes: `{source, target, alpha, normMode}`. Sources are any of the 23+ computed statistics (plus Const0, Const1). Targets are audio parameters (pitch, gain, filter cutoff, etc.). Each route has independent EMA smoothing.

**Key insight**: Voice and Melody routes use x2 scaling — disconnected routes sit at `smoothed = 0.5`, giving x1.0 (no change). Connecting a source that swings 0-1 gives 0x-2x modulation range.

### Silence Management

Three mechanisms prevent orphaned audio:
1. **Watchdog timer** (100ms `setInterval`): fades masterGain to zero if `updateAudio()` hasn't been called in 150ms
2. **Visibility listener**: ramps to zero on tab hide
3. **`resetAudioState()`**: called on stop, home, pattern change, degree change, sound toggle off

---

## 7. UI Patterns & Conventions

### Popover System

All popovers (config, timing, audio, ops tools, context menus) use the `.ops-pop` CSS class. Positioning:

```javascript
const r = btnEl.getBoundingClientRect();
pop.style.left = r.left + "px";
pop.style.top = (r.bottom + 6) + "px";
```

Toggle via `.open` class. A global `document.addEventListener("mousedown", ...)` handler closes open popovers when clicking outside.

### Tab System

Left panel has 6 tabs: C-Nodes, C-List, D-Nodes, D-List, Jiggle, Final. Switching calls `switchLeftTab(name)` which updates CSS classes and triggers content-specific refresh (e.g., `refreshCoeffList()` for C-List, `refreshDCoeffList()` for D-List, `renderMorphPanel()` for D-Nodes, `buildJigglePanel()` for Jiggle, `renderFinalPanel()` for Final).

Right panel has 4 tabs: Roots, Stats, Sound, Bitmap. Switching calls `switchTab(name)` which updates CSS classes and triggers resize/redraw for the newly visible tab.

### Save/Load

`saveState()` serializes everything to JSON via `buildStateMetadata()`: coefficients (with curves), trails, selections, audio config, routes, jiggle params, color modes, bitmap settings, morph state (including D-node paths), solver type, worker count, etc. `loadState()` calls `applyLoadedState()` which parses JSON and restores all fields plus reconstructs UI state.

**When adding new state**: add it to both `buildStateMetadata()` (~line 7529) and `applyLoadedState()` (~line 8474). Provide defaults in `applyLoadedState()` for backward compatibility with old snapshots. If the state affects fast mode workers, also add it to `serializeFastModeData()` and the worker init message handler.

---

## 8. Bugs Found & Fixed

### The Sonification Silence Bug

**Symptom**: Enabling Base, Melody, or Voice during animation produces complete silence.

**Root cause**: `uiPing()` and `uiBuzz()` (UI feedback sounds for button clicks) create a bare `AudioContext` without building the sonification audio graph. When `toggleSound()` later calls `initAudio()`, the guard `if (audioCtx) return;` fires because `uiPing` already created the context. `audioNodes` stays `null`. `updateAudio()` checks `!audioNodes` and returns — silence.

**Fix**: Changed the guard from `if (audioCtx) return;` to `if (audioNodes) return;`.

### The getImageData Performance Cliff

**Symptom**: Fast mode grinds to a halt above 2K resolution.

**Root cause**: `compositeWorkerPixels()` called `getImageData`/`putImageData` on the *entire* canvas every pass, even though only ~29,000 pixels were modified. At 10K, this copies 800MB of data per pass.

**Fix**: Persistent `ImageData` buffer (eliminates getImageData entirely) + dirty-rect putImageData (only flushes the region that changed).

### The NaN Root Poisoning Bug (Fast Mode "Sparse Dots")

**Symptom**: Certain polynomial configurations produce almost nothing in fast mode bitmap — just ~22 scattered dots per pass. Interactive animation works perfectly.

**Root cause**: `solveRootsEA()` filtered out non-finite roots before returning, causing `currentRoots.length < degree`. Workers read warm-start from a buffer of size `nRoots` -> `warmRe[i]` for `i >= nRoots` returns `undefined` -> Float64Array stores `NaN`. In the Aberth sum, every root depends on every other root: a single NaN root poisons ALL other roots within ONE iteration.

**Fix (two parts)**:
1. **Main thread `solveRootsEA`** — always return exactly `degree` roots. Non-finite roots fall back to warm-start values, then to unit-circle seeding.
2. **Worker `solveEA`** — rescue non-finite roots after each solve call by re-seeding on the unit circle.

### Fast Mode Video Recording

**Symptom**: Starting video recording then entering fast mode freezes the recorded video.

**Root cause**: `animLoop()` returns early when `fastModeActive` is true, so `recordTick()` never fires.

**Fix**: Added `recordTick()` call in `handleFastModeWorkerMessage()` right after `compositeWorkerPixels()`.

---

## 9. Performance Insights

### What's Expensive

| Operation | Cost | Scales With |
|-----------|------|-------------|
| Ehrlich-Aberth solver | O(degree^2 x iters) per step | Polynomial degree |
| Root matching | O(degree^2) per call (greedy), O(degree^3) (Hungarian) | Polynomial degree |
| `getImageData` (eliminated) | O(W x H x 4) | Canvas area |
| `putImageData` (dirty rect) | O(dirty_W x dirty_H x 4) | Root spread |
| Structured clone transfer | O(steps x degree x 13 bytes) | Steps per pass |
| Stats computation | O(degree x stats_count) | Polynomial degree |
| Domain coloring | O(pixels x degree x iters) | Canvas resolution x degree |

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

- **matchRoots**: O(n^2) greedy matching (O(n^3) Hungarian). Called every 4th step in colored mode to amortize cost. At degree 100+, this becomes the bottleneck.
- **Domain coloring**: Evaluates the polynomial at every pixel. Throttled to one render per rAF, at 50% resolution.
- **Stats computation**: Linear in degree but runs every frame. Not a bottleneck until very high degree.

---

## 10. Coordinate Systems

### Complex Plane <-> Screen

D3 linear scales map between the complex plane and SVG coordinates:

```javascript
xScale: [range.min, range.max] -> [MARGIN, width - MARGIN]
yScale: [range.max, range.min] -> [MARGIN, height - MARGIN]  // inverted: SVG y increases downward
```

Default range is [-3, 3]. Zoom/pan modifies `range.min` and `range.max`.

### Bitmap Coordinate System

The bitmap canvas freezes the viewport at activation time (`bitmapRange`). Workers receive this frozen range and map root positions to pixel coordinates independently. The main thread composites sparse pixel indices directly into the persistent buffer.

Pixel index = `y * W + x` (row-major). RGB channels sent separately (not interleaved RGBA).

---

## 11. Things That Surprised Me

1. **Jiggle applies to non-animated coefficients too** — not just the ones on paths. This was intentional but non-obvious.

2. **`"none"` path type creates a 1-point curve** at the home position, rather than being null. This simplifies the animation loop (no null checks).

3. **The watchdog timer uses `setInterval`, not `requestAnimationFrame`** — because rAF stops firing in background tabs, but the watchdog needs to detect silence even when the tab is hidden.

4. **Workers are persistent** — created once on fast mode init, reused for every pass. Only the step range changes per pass. This avoids worker startup latency.

5. **Root matching frequency matters enormously** — calling it every step vs every 4th step is the difference between smooth operation and stuttering at high degree.

6. **putImageData with dirty rect has a sixth argument** — `putImageData(imageData, dx, dy, dirtyX, dirtyY, dirtyW, dirtyH)`. The first three position the image; the last four specify which sub-rectangle to actually write. Not well documented.

7. **GC pressure from getImageData was slowing workers** — eliminating the per-pass 400MB `ImageData` allocation reduced worker time from 8.4ms to 3.2ms. The GC pauses were affecting the whole page.

8. **The FM synthesis modulation depth tracks kinetic energy** — at rest, the base instrument is a pure sine wave. As roots move faster, the FM modulation index increases, producing progressively richer harmonics.

9. **Close encounter detection is per-root, not global** — each root tracks its own top-3 closest approach records. A beep fires only when a root beats its own record.

10. **A single NaN root kills the entire Ehrlich-Aberth solver in one iteration** — because the Aberth correction sums `1/(z_i - z_j)` over all roots. If any `z_j` is NaN, the sum becomes NaN for all roots after one iteration.

11. **Float64Array out-of-bounds reads return `undefined`, not an error** — and `undefined` silently converts to `NaN` when stored in another Float64Array. This makes dimension-mismatch bugs extremely hard to find.

12. **Worker integration for new features is simpler than expected** — adding morph to fast mode workers required only: (a) serialize D positions in `serializeFastModeData()`, (b) pass in worker init message, (c) a blending loop after curve interpolation. No worker architecture changes needed.

13. **Mirror features by mirroring ALL entry points, not just the obvious ones** — when adding D-node path animation, the obvious entry point was `animLoop()`. But there are 5 places that advance/reset coefficient positions: `animLoop()`, `startAnimation()` (fresh-start snap), scrub slider handler, home button handler, and fast mode workers. Missing any one creates a sync bug.

14. **`Set` iteration order is insertion order, not sorted order** — `[...selectedCoeffs]` returns indices in the order they were added to the Set, not numerically sorted. Always sort for deterministic index ordering.

15. **Coefficient array index = polynomial term power mapping** — `coefficients[0]` is the leading (highest power) term, `coefficients[n-1]` is the constant term. Adding via `unshift()` makes it the new highest power. Selection indices must be adjusted when coefficients are added/removed.

16. **GPU memory is the real constraint for large canvases, not CPU memory** — a 10K canvas allocates ~400MB GPU memory. The fix: decouple compute resolution from display resolution. The persistent buffer can be 25K (CPU-only) while the canvas stays at 2000px.

17. **Decouple display-only controls from computation controls** — `rootColorMode` was a single variable controlling both SVG animation and bitmap rendering. The fix: separate `rootColorMode` (animation, 3 modes) from `bitmapColorMode` (bitmap, 6 modes) with independent uniform color variables.

18. **Feature flags must cover all code paths** — derivative coloring worked for SVG but the bitmap pipeline was completely separate. When `bitmapColorMode === "derivative"` was selected, workers silently fell through to rainbow coloring. The fix required a complete parallel implementation in both the JS worker blob and the WASM step loop.

19. **The WASM step loop eliminates JS-to-WASM boundary crossing per step** — the original WASM integration called `solveEA` per-step from JS, paying marshalling overhead each time. The full step loop (`step_loop.c`) moves curve interpolation, morph blend, jiggle, solve, match, and pixel output into a single WASM call per run message. The JS side only needs to copy warm-start roots in and read sparse pixels out.

20. **WASM memory layout must be computed at runtime** — because array sizes depend on degree, number of animated coefficients, step count, etc. The JS-side `computeWasmLayout()` function calculates byte offsets for 40+ arrays and writes them into a config region that WASM reads via `init()`. Memory grows dynamically to fit.

21. **WASM and JS step loops must produce identical results** — the WASM loop is the fast path but the JS loop is the fallback for unsupported color modes. Both must implement the same step sequence: reset coeffs -> interpolate C-curves -> interpolate D-curves -> follow-C -> morph blend -> apply jiggle -> solve -> NaN rescue -> color-mode processing -> pixel output.

---

## 12. Conventions to Follow

### Adding a New Popover

1. Add HTML in the body (near the other popovers, around line 917+)
2. Use class `ops-pop` for consistent styling
3. Position via `getBoundingClientRect()` on the trigger button
4. Toggle `.open` class to show/hide
5. Add close logic in the global mousedown handler

### Adding New State to Save/Load

1. Add serialization in `buildStateMetadata()` (~line 7529)
2. Add deserialization in `applyLoadedState()` (~line 8474) with a default fallback for old snapshots
3. Test with old snapshot files to verify backward compatibility
4. If the state affects fast mode workers, also add it to `serializeFastModeData()` and the worker init message

### Adding a New Statistic

1. Add to `STAT_TYPES` array (line ~1150)
2. Add a color to `STAT_COLORS` map
3. Compute in the stats update section of `renderRoots()` or `updateStats()`
4. It automatically becomes available as an audio route source

### Adding a New Audio Route Target

1. Add a slot to the relevant instrument's routes array (`baseRoutes`, `melodyRoutes`, or `voiceRoutes`)
2. Add the parameter application in `updateAudio()` inside the appropriate instrument section
3. The routing UI auto-generates from the routes array

### Adding a New Bitmap Color Mode

1. Add the mode string to `bitmapColorMode` declaration (line ~1016)
2. Add a flag in `serializeFastModeData()` (e.g., `myNewColor: bitmapColorMode === "my-new"`)
3. Add the flag to the worker init handler (`S_myNewColor = !!d.myNewColor`)
4. Implement the pixel output logic in the JS step loop's color-mode switch
5. If possible, implement in `step_loop.c` (add a new `colorMode` value); if not, force JS fallback by setting `S_useWasmLoop = false` when the flag is true
6. Add the mode to `buildBitmapCfgPop()` UI with appropriate sub-options
7. Add to save/load with backward-compatible default

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

## 13. Testing & Debugging

### Test Suite

Automated tests exist in `tests/` using Playwright Python (headless Chromium). 777 tests across 26 files covering solver correctness, root matching, curve generation, path parametrics, shapes, polynomial operations, state save/load, stats, colors, utilities, morph system, jiggle perturbation (10 modes), continuous fast mode, off-canvas render split, multi-format image export, bitmap/animation color decoupling, derivative bitmap coloring, root-matching strategies (Hungarian algorithm, serialization, UI chips), D-node paths (D-List tab, animation helpers, D-curve serialization, backward compat), D-node context menu (open/close, path editing, revert), extended save/load fields, animation controls (play/pause/resume, home, scrub with D-nodes), trajectory editor simplification (preview/revert/commit, PS button removal, node cycler removal), Final tab (rendering, morph blending, trail data), WASM step loop (init, run, color modes, morph, matching strategies), shape morph ops, pattern arrange ops, integration tests, and JS vs WASM benchmarks. See [test-results.md](test-results.md) for details.

Manual testing remains important for:
- Dragging coefficients and roots
- Playing animations with various path types
- Toggling fast mode at different resolutions
- Enabling/disabling sonification instruments
- Save -> load round-trip verification
- Checking the timing popup for performance regressions

### Debugging Tools Built In

1. **Timing popup** (T button in header): Shows per-pass breakdown — worker time, composite time, getImageData, putImageData, pixels painted
2. **Config popup** (cfg button in bitmap toolbar): Runtime tuning of worker count, steps per pass, solver type (JS/WASM)
3. **Stats tab**: 16 real-time plots of root constellation features (time-series, phase-space, and spectrum)
4. **Console logging**: Various `console.log` statements for worker lifecycle events (can be noisy)

### Common Debugging Scenarios

**"Fast mode is slow"**: Check the timing popup. If composite % is high, the bottleneck is canvas operations (check resolution). If worker % is high, the bottleneck is the solver (check degree, iterations).

**"No sound"**: Check that `audioNodes` is not null (the uiPing/initAudio ordering bug). Check `audioCtx.state` — might be "suspended" (needs user gesture). Check `masterGain.gain.value` — might be zero from watchdog.

**"Trails look wrong"**: Check root matching — if `matchRootOrder` is skipped or roots teleport, trails will have discontinuities.

**"Save/load lost my setting"**: The new field probably isn't in `applyLoadedState()` yet. Add it with a default fallback.

**"Morph enabled but Play does nothing"**: Both `startAnimation()` and the Play button handler guard with `allAnimatedCoeffs().size === 0`. When morph is the only animation source (no paths), these guards must be relaxed with `&& !morphEnabled`.

**"Fast mode ignores morph"**: Check that `morphEnabled` is true and `morphTargetCoeffs.length === coefficients.length` — both conditions must hold for `serializeFastModeData()` to include morph data.

**"Fast mode shows sparse dots / almost nothing"**: Check `currentRoots.length` vs `coefficients.length - 1` (degree). If they differ, the solver is dropping roots. Also check the worker's warm-start buffer size matches the expected degree.

**"WASM step loop not activating"**: Check the bitmap color mode — idx-prox and ratio modes force JS fallback. Also check that `solverType === "wasm"` in the cfg popup. The worker tries WASM step loop first, then falls back to pure JS.

---

## 14. File Index

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
| [tutorial.md](tutorial.md) | Tutorial and screenshots | — |
| [architecture.md](architecture.md) | This file | — |
| [archive/](archive/) | Archived docs: proof-read.md, worker_speed_issues.md | — |

### Key Code Locations

| Component | Approx Lines |
|-----------|-------------|
| `uiPing()` / `uiBuzz()` | ~1369 |
| `initAudio()` | ~1433 |
| `updateAudio()` | ~1497 |
| `drawGrid()` | ~1842 |
| `computeRootSensitivities()` (main thread) | ~2081 |
| `toggleCoeffSelect()` / `clearAllSelection()` | ~2728 / ~2768 |
| `barSnapshots` / `previewBarToSelection()` | ~2678 / ~3563 |
| `revertBarPreview()` / `commitBarPreview()` | ~3614 / ~3635 |
| `addCoefficientAt()` | ~3143 |
| `deleteCoefficient()` | ~3169 |
| `allAnimatedDCoeffs()` | ~3418 |
| `advanceDNodesAlongCurves()` | ~3427 |
| `playBtn` click handler | ~3719 |
| `homeBtn` click handler | ~3739 |
| `scrubSlider` input handler | ~3787 |
| `advanceToElapsed()` | ~3790 |
| `updateAnimSeconds()` | ~3850 |
| `startAnimation()` | ~4318 |
| `animLoop()` | ~4353 |
| `renderCoefficients()` | ~4455 |
| `morphDrag` handler | ~4503 |
| `renderMorphPanel()` | ~4559 |
| `renderFinalPanel()` | ~4682 |
| `updateMorphMarkers()` | ~4748 |
| `renderRoots()` | ~4863 |
| `renderCoeffTrails()` | ~4969 |
| `renderDomainColoring()` | ~5055 |
| `solveRootsEA()` | ~5151 |
| `matchRootOrder()` | ~5290 |
| `buildColorPop()` (animation) | ~5599 |
| `toggleSound()` | ~5851 |
| `snapshotSelection()` / `applyPreview()` | ~6075 / ~6083 |
| `opCloseCallback` / `closeOpTool()` | ~6172 / ~6175 |
| `openOpTool()` / `buildScaleTool()` | ~6188 |
| `PTRN_PARAMS` / `patternPositions()` | ~6672 / ~6431 |
| `buildPatternTool()` | ~6704 |
| `recordTick()` | ~7358 |
| `buildStateMetadata()` | ~7529 |
| `buildBitmapCfgPop()` (bitmap cfg + color modes) | ~7807 |
| `buildBitmapSavePop()` | ~8199 |
| `saveState()` | ~8426 |
| `loadState()` / `applyLoadedState()` | ~8450 / ~8474 |
| `drawAllStatsPlots()` | ~9653 |
| `fillPersistentBuffer()` | ~9660 |
| `fillDisplayBuffer()` | ~9675 |
| `exportPersistentBufferAsBMP()` | ~9690 |
| `exportPersistentBufferAs{JPEG,PNG,TIFF}()` | ~9754 |
| `initBitmapCanvas()` | ~9780 |
| `plotCoeffCurvesOnBitmap()` | ~10801 |
| `createFastModeWorkerBlob()` | ~9907 |
| `solveEA()` (worker blob JS) | ~9911 |
| `hungarianMatch()` (worker blob) | ~9990 |
| `rankNorm()` + `computeSens()` (worker blob) | ~10036 |
| `computeWasmLayout()` (worker) | ~10145 |
| `initWasmStepLoop()` (worker) | ~10200 |
| Worker `onmessage` handler | ~10368 |
| WASM step loop run path | ~10432 |
| JS step loop fallback | ~10472 |
| `compositeWorkerPixels()` | ~11286 |
| `serializeFastModeData()` | ~11010 |
| `initFastModeWorkers()` | ~11148 |
| `dispatchPassToWorkers()` | ~11218 |
| `advancePass()` | ~11383 |
| `reinitWorkersForJiggle()` | ~11407 |
| `enterFastMode()` | ~10881 |
| `exitFastMode()` | ~11428 |
| `switchTab()` | ~11649 |
| `switchLeftTab()` | ~11676 |
| `refreshCoeffList()` | ~11915 |
| `refreshDCoeffList()` | ~12713 |
| `nearestPrime()` | ~13596 |
| `buildJigglePanel()` | ~13650 |
| `rebuild()` | ~13829 |
| `bitmapColorMode` / `bitmapUniformColor` | ~1016 |
| `bitmapMatchStrategy` | ~1018 |
| `bitmapIdxProxGamma` / `bitmapRatioGamma` | ~1020 |
| `ROOT_COLOR_SWATCHES` | ~1023 |
| `PROX_PALETTE_CATALOG` | ~1034 |
| `DERIV_PALETTE` / `DERIV_PAL_R/G/B` | ~1067 |
| `WASM_STEP_LOOP_B64` | ~1135 |
| `fastModeDCurves` | ~1105 |
