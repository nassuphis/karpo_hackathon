# Patterns

## Basic (5)

| Pattern | Description |
|---------|-------------|
| Circle | Evenly spaced on a circle |
| Real axis | Along the real axis |
| Imaginary axis | Along the imaginary axis |
| Grid | Square grid arrangement |
| Random | Uniformly random |

## Coefficient Patterns (8)

| Pattern | Description |
|---------|-------------|
| Spiral | Archimedean spiral |
| Star | Alternating inner/outer radii |
| Figure-8 | Bernoulli lemniscate |
| Conjugate pairs | Pairs straddling the real axis |
| Two clusters | Two separate clusters |
| Geometric decay | Alternating-sign geometric series |
| Rose curve | 3-petal rose r = cos(3theta) |
| Cardioid | Heart-shaped curve |

## Root Shapes (13)

These define the *roots* in a specific shape, then compute the coefficients by expanding the product. Dragging the resulting coefficients perturbs the roots away from the initial shape:

Heart, Circle, Star, Spiral, Cross, Diamond, Chessboard, Smiley, Figure-8, Butterfly, Trefoil, Polygon, Infinity

## Path Catalog

Animation path types are defined in `PATH_CATALOG`, the single source of truth for all path `<select>` elements. Each entry is either a top-level option (`{ value, label }`) or an `<optgroup>` (`{ group, items: [...] }`). Entries may carry a `dOnly: true` flag, indicating they appear only in D-node path selectors.

| Group | Paths |
|-------|-------|
| *(top-level)* | None |
| *(top-level, D-only)* | Follow C |
| Basic | Circle, Horizontal, Vertical, Spiral, Gaussian cloud |
| Curves | Lissajous, Figure-8, Cardioid, Astroid, Deltoid, Rose (3-petal), Spirograph, Hypotrochoid, Butterfly, Star (pentagram), Square, C-Ellipse |
| Space-filling | Hilbert (Moore), Peano, Sierpinski |

Dithered variants (`-dither` suffix) are auto-inserted after each base path in catalog groups. Random and Follow-C have no dithered variant.

### Follow-C (D-node only)

The **Follow C** path type is exclusive to D-nodes (`dOnly: true`). A D-node with this path copies the position of its corresponding C-coefficient each frame instead of following its own trajectory. It has no parameters (`PATH_PARAMS["follow-c"] = []`) and no curve of its own -- its curve is a single-point array at its current position. In fast-mode workers, Follow-C indices are serialized as `dFollowCIndices` and the worker copies `coeffsRe[i]`/`coeffsIm[i]` into the morph target arrays each step.

### buildPathSelect(sel, noneLabel, dNode)

Populates a `<select>` element from `PATH_CATALOG`. The optional `noneLabel` overrides the "None" text (e.g. "--" for list editors). The `dNode` flag (third parameter) includes D-only entries when `true`. Called three times at init:

- `buildPathSelect(document.getElementById("anim-path"))` -- anim-bar, no D-only
- `buildPathSelect(document.getElementById("lce-path-sel"), "--")` -- C-List editor, no D-only
- `buildPathSelect(document.getElementById("dle-path-sel"), "--", true)` -- D-List editor, includes Follow C

## Selection Model

Selection is **mutually exclusive** across three pools: C-coefficients (`selectedCoeffs`), D-nodes (`selectedMorphCoeffs`), and roots (`selectedRoots`). Clicking a node in one pool clears the other two. The three `Set` objects are managed by `clearCoeffSelection()`, `clearMorphSelection()`, `clearRootSelection()`, and `clearAllSelection()`.

### Ops Bar Indicator

The ops bar shows a colored target label (`#ops-target`) reflecting which pool is active:

| State | Label | Color |
|-------|-------|-------|
| No selection | `* none` | gray `#888` |
| C-coefficients selected | `* C` | green `#5ddb6a` |
| D-nodes selected | `* D` | blue `#4ea8de` |
| Roots selected | `* roots` | red `#e94560` |

When no selection exists, the ops group is disabled (`#ops-group.disabled`).

### Ops Tools on D-nodes

The mid-bar **Scale**, **Rotate**, and **Translate** tools operate on whichever pool is active. `snapshotSelection()` returns `{ which: "coeff"|"morph"|"roots", items }` depending on which `Set` is non-empty (checked in priority order: C, then D, then roots). `applyPreview()` dispatches to the correct data array: `coefficients[]` for "coeff", `morphTargetCoeffs[]` for "morph", `currentRoots[]` for "roots". After transforming D-nodes, curves with `pathType === "follow-c"` get a single-point curve at the new position; all other D-node curves are recomputed via `computeCurve()`.

## Pattern Arrange Tool (Mid-bar)

The **Pattern** tool (`#ptrn-tool-btn`) in the mid-bar arranges selected positions onto a geometric pattern. It operates on whichever node kind is active (C-coefficients, D-nodes, or roots).

### PATTERN_LIST (21 patterns)

Circle, Square, Triangle, Pentagon, Hexagon, Diamond, Star, Ellipse, Infinity, Spiral, Grid, Line, Wave, Cross, Heart, Lissajous, Rose, 2 Circles, 2 Squares, Ring, Scatter.

### PTRN_PARAMS (per-pattern parameters)

13 of the 21 patterns have adjustable parameters defined in `PTRN_PARAMS`. Each entry is an array of parameter descriptors:

| Pattern | Parameters | Defaults |
|---------|------------|----------|
| Star | Inner R (0.1--0.9) | 0.38 |
| Ellipse | Aspect (0.1--3.0) | 0.5 |
| Infinity | Amp (0.1--1.0) | 0.5 |
| Spiral | Turns (0.5--10) | 3 |
| Grid | Cols (1--n) | sqrt(n) |
| Line | Angle (0--360°) | 0 |
| Wave | Cycles (0.5--5.0), Amp (0.1--1.0) | 1, 0.5 |
| Cross | Width (0.05--0.5) | 0.25 |
| Lissajous | Freq A (1--8), Freq B (1--8) | 3, 2 |
| Rose | Petals (1--12) | 3 |
| 2 Circles | Distance (0--2.0) | 1.0 |
| 2 Squares | Distance (0--2.0) | 1.0 |
| Ring | Inner R (0.1--0.95) | 0.5 |

Patterns without entries in `PTRN_PARAMS` (Circle, Square, Triangle, Pentagon, Hexagon, Diamond, Heart, Scatter) use fixed geometry.

### Pattern Tool UI (Accept/Revert)

Unlike the drag-to-apply Scale/Rotate/Translate tools, the Pattern tool uses an **Accept/Revert** workflow:

1. A dropdown selects the pattern. Per-pattern sliders (from `PTRN_PARAMS`) appear below.
2. Changing the pattern or adjusting sliders calls `refresh()` → `applyPreview(snap, ...)` for live preview.
3. Switching patterns resets `opts = {}` and rebuilds controls with new defaults.
4. Click **Accept** to commit (sets `accepted = true`, calls `closeOpTool()`).
5. Closing without Accept (outside click, Escape, or switching tools) triggers `opCloseCallback` which reverts all positions to the original snapshot.

The blend/percentage slider was removed — patterns now apply at full strength immediately.

### `patternPositions(pattern, n, cRe, cIm, R, opts)`

Generates `n` positions for the given pattern, centered at `(cRe, cIm)` with bounding radius `R`. The `opts` object passes per-pattern parameters (e.g. `{ starInner: 0.38 }` for Star, `{ cols: 5 }` for Grid). Returns `[{re, im}, ...]`.

Most patterns use `distributeOnPath(n, verts, closed)` — an arc-length-based point distributor that places `n` points equidistantly along a polyline (closed or open).

## Parameter Ranges

### Speed

Speed is stored internally as a float (0.001--1.0) and displayed/edited as an integer (1--1000) via `toUI: v => Math.round(v * 1000)` and `fromUI: v => v / 1000`. Resolution is **1/1000** of a full loop per second. The speed slider in C-List and D-List selection bars has `min="1" max="1000" step="1"`.

### Param2

The param2 slider (used by LerpSpeed, LerpRadius, LerpAngle transforms) has range **1--1000** with step 1, in both C-List (`#list-sel-param2`) and D-List (`#dlist-sel-param2`).

### Jiggle Controls

| Control | Range | Step | Notes |
|---------|-------|------|-------|
| sigma | 0--10 | 0.01 | `jiggleSigma`; read as `jiggleSigma / 10` (fraction of `coeffExtent()`). Backward compat: old 0--100 values are divided by 10 on load. |
| Steps (rotate/circle/spiral/wobble) | 10--5000 | 1 | `jiggleAngleSteps` or `jiggleCircleSteps`; number of jiggle steps for one full revolution. Replaces the old theta (turns) control. |
| Interval | 0.1--100s | 0.1 | `jiggleInterval`; seconds between jiggle perturbations. |
| Lissajous period | 10--5000 | 1 | `jigglePeriod`; measured in **steps** (not cycles). Slider with `min="10" max="5000"`. |
| Amplitude | 1--50 | 1 | `jiggleAmplitude`; % of `coeffExtent()` or centroid distance. |
| FreqX / FreqY | 1--10 | 1 | Lissajous frequency multipliers. |

---

# Coding Patterns

## Popup Pattern (`.ops-pop`)

All popups and popovers share the `.ops-pop` CSS class. The pattern is:

**CSS**: Hidden by default (`display: none`), shown when `.open` is added (`display: flex`). Positioned with `position: fixed; z-index: 200;` and given the standard dark glassmorphism treatment (semi-transparent background, backdrop blur, subtle border, drop shadow).

```css
.ops-pop { display: none; position: fixed; z-index: 200; ... }
.ops-pop.open { display: flex; flex-direction: column; align-items: center; gap: 6px; }
```

**Positioning**: Always done via `getBoundingClientRect()` on the triggering button, then setting `style.left` and `style.top` on the popup element:

```js
const r = btnEl.getBoundingClientRect();
pop.style.left = r.left + "px";        // align left edge to button
pop.style.top = (r.bottom + 6) + "px"; // below button with 6px gap
```

Some popups align to the right edge of the button instead: `pop.style.left = (r.right + 8) + "px"`.

**Toggle lifecycle**:
1. `openXxxPop()` -- clears popup content (`innerHTML = ""`), rebuilds DOM, adds `.open`, positions.
2. `closeXxxPop()` -- removes `.open`, optionally reverts state if the popup supports preview.
3. Dismissal via outside click or Escape key.

**Instances**: ops-pop (scale/rotate/translate), coeff-ctx, dnode-ctx, snap-pop, bitmap-cfg-pop, bitmap-save-pop, timing-pop, color-pop, coeff-pick-pop, audio-pop, degree-pop, path-pick-pop, dpath-pick-pop.

## Outside-Click Dismissal

A single `mousedown` listener on `document` handles closing all open popups. Each popup has its own guard block:

```js
document.addEventListener("mousedown", e => {
    if (barSnapshots) {
        if (!animBar.contains(e.target)) revertBarPreview();
    }
    if (activeToolBtn) {
        if (!opsPop.contains(e.target) && !(e.target.id && e.target.id.endsWith("-tool-btn")))
            closeOpTool();
    }
    if (ctxCoeffIdx >= 0) {
        if (!ctxEl.contains(e.target)) closeCoeffCtx(true);  // revert
    }
    // ... same pattern for each popup type
});
```

The check ensures clicks *inside* the popup do not close it. For popups with preview/revert semantics (context menus, path pickers, anim-bar), outside click triggers a **revert**.

## Escape Key Handling

A `keydown` listener on `document` handles Escape with a priority chain -- the first open popup wins:

```js
if (e.key === "Escape") {
    if (barSnapshots) { revertBarPreview(); return; }
    if (ctxCoeffIdx >= 0) { closeCoeffCtx(true); return; }
    if (ctxDNodeIdx >= 0) { closeDNodeCtx(true); return; }
    if (activeToolBtn) { closeOpTool(); return; }
    // ... more popup types ...
    clearAllSelection();  // fallback: deselect everything
}
```

## Preview/Revert/Commit Pattern

Used wherever UI controls offer live preview that can be cancelled. The pattern has three functions and a snapshot variable:

### Anim-Bar (C-coefficients)

```
barSnapshots = null   -- null means no active preview
previewBarToSelection()  -- snapshot on first call, apply bar params to selected coefficients
revertBarPreview()       -- restore from snapshot, set barSnapshots = null
commitBarPreview()       -- discard snapshot (keep applied changes), set barSnapshots = null
```

**Trigger flow**:
- Path dropdown `change` or slider `input` calls `previewBarToSelection()`.
- "Update Whole Selection" button calls `commitBarPreview()` if a snapshot exists, else `applyBarToSelection()`.
- Outside click on anything outside `#anim-bar` calls `revertBarPreview()`.
- `updateAnimBar()` (called when selection changes) calls `revertBarPreview()` first.

### Context Menus (C-coeff and D-node)

```
ctxSnapshot = null   -- snapshot for reverting on cancel
closeCoeffCtx(revert)    -- if revert=true and ctxSnapshot exists, restore original values
previewCtx()             -- applies current menu settings immediately (no separate snapshot call)
```

The snapshot is taken once when the context menu opens (`showCoeffCtxMenu`). Every slider `input` and dropdown `change` inside the menu calls `previewCtx()`. Clicking outside the menu calls `closeCoeffCtx(true)` (revert). The "Accept" button calls `closeCoeffCtx(false)` (commit, keeping changes).

The D-node context menu follows the same pattern with `ctxDNodeSnapshot`, `closeDNodeCtx(revert)`, and `previewDNodeCtx()`.

### Path Picker Popups (C-List and D-List)

```
pathPickSnapshot = null
openPathPickPop(coeffIdx, anchorEl)  -- snapshots current state
closePathPickPop(revert)             -- if revert=true, restores from snapshot
previewPP()                          -- live preview from popup controls
```

Same pattern for D-List: `dpathPickSnapshot`, `openDPathPickPop()`, `closeDPathPickPop(revert)`, `previewDPP()`.

### Transform Tools (Scale/Rotate/Translate)

```
opSnapshot = null   -- { which: "coeff"|"morph"|"roots", items: [{idx, re, im}, ...] }
openOpTool(btnEl, buildFn)  -- snapshots selection, builds popup
closeOpTool()               -- reverts via opCloseCallback if set, clears state
applyPreview(snap, fn)      -- applies transform function to snapshotted items
```

For drag-based tools (Scale, Rotate, Translate), live preview happens during drag (mousemove calls `applyPreview` repeatedly). The snapshot stores original positions so each drag frame recomputes from the original, not cumulatively. Releasing the mouse commits the transform.

For Accept-based tools (Pattern), `opCloseCallback` is set to a function that reverts positions if not accepted. The callback fires in `closeOpTool()` before clearing state. The Pattern tool sets `accepted = true` on Accept click, so the callback becomes a no-op; on any other close path (outside click, Escape), the callback restores positions from `opSnapshot`.

## Transform Dropdown Execute-Then-Reset

The C-List and D-List transform dropdowns (`#list-transform`, `#dlist-transform`) use an execute-then-reset pattern: the dropdown fires on `change`, immediately resets its value to `"none"`, then executes the selected action. This means the dropdown always shows the placeholder and never "stays" on a selected action.

```js
document.getElementById("list-transform").addEventListener("change", function() {
    const action = this.value;
    this.value = "none";           // reset immediately
    if (action === "none") return;
    if (selectedCoeffs.size === 0) { uiBuzz(); return; }
    // ... execute action (PrimeSpeeds, SetAllSpeeds, RandomSpeed, etc.)
});
```

The same pattern is used for D-List transforms on `#dlist-transform`.

## Save/Load Pattern

### buildStateMetadata()

Serializes the entire application state into a plain JSON-serializable object. Called by both `saveState()` and `snapDownload()`. Fields include:

- `degree`, `pattern`, `coefficients[]` (pos, home, pathType, radius, speed, angle, ccw, extra)
- `roots[]`, `panels` (coeff/roots ranges), `trailData`
- Color modes: `rootColorMode`, `uniformRootColor`, `bitmapColorMode`, `bitmapUniformColor`, `bitmapCanvasColor`
- Bitmap config: `bitmapMatchStrategy`, `bitmapProxPalette`, `bitmapIdxProxGamma`, `bitmapExportFormat`
- Jiggle state: mode, sigma, angleSteps, scaleStep, period, amplitude, lissFreqX/Y, circleSteps
- Morph state: `morph.enabled`, `morph.rate`, `morph.mu`, `morph.target[]` (same fields as coefficients)
- `numWorkers`, `solverType`, `targetSeconds`, `bitmapCoeffView`

### saveState() / loadState()

Both use the File System Access API (`showSaveFilePicker` / `showOpenFilePicker`) when available, with fallback to `<a>` download / `<input type="file">` for browsers that lack the API.

### applyLoadedState(meta)

Restores state in a strict order:
1. Stop animation, clear everything (trails, roots, audio, selection)
2. Set degree + pattern (without triggering `applyPattern`)
3. Rebuild `coefficients[]` from saved data
4. Restore panel ranges (must happen before `computeCurve` so `coeffExtent()` works)
5. Regenerate curves for all coefficients
6. Restore morph state, jiggle state, color modes, etc.

**Backward compatibility**: uses `??` and `||` defaults for all fields so older saves missing newer fields load cleanly. Example: `saved.radius ?? 25`, `meta.morph?.enabled`, `bitmapMatchStrategy || "assign4"`.

## Worker Init/Run Message Pattern

Fast-mode workers use a two-phase message protocol with persistent workers:

### Phase 1: `init` message (sent once per fast-mode session)

The main thread creates workers via `createFastModeWorkerBlob()` (inline Blob URL) and sends a single `init` message containing all static data: coefficient arrays, curve data, color palettes, WASM binaries (base64-encoded), morph targets, jiggle offsets, etc. Workers store this data in module-scoped `S_*` variables that persist across runs.

```js
worker.postMessage({
    type: "init",
    coeffsRe: ..., coeffsIm: ..., nCoeffs, degree, nRoots,
    animEntries, curvesFlat, curveOffsets, curveLengths, curveIsCloud,
    colorsR, colorsG, colorsB,
    canvasW, canvasH, bitmapRange,
    // color mode flags
    noColor, uniformR/G/B, proxColor, proxPalR/G/B, derivColor, derivPalR/G/B,
    // WASM binaries
    useWasm, wasmB64, wasmStepLoopB64,
    // morph, D-curves, jiggle, match strategy...
});
```

### Phase 2: `run` message (sent once per pass, per worker)

Only the data that changes between passes -- root positions and step range:

```js
worker.postMessage({
    type: "run",
    workerId: w,
    stepStart, stepEnd,
    elapsedOffset,
    rootsRe: ..., rootsIm: ...
});
```

### Worker response messages

Workers send three message types back to the main thread:

| Type | Purpose |
|------|---------|
| `progress` | Periodic step count update for progress bar (`{ workerId, step, total }`) |
| `error` | WASM runtime error (`{ workerId, error }`) |
| `done` | Sparse pixel data + final root positions (`{ workerId, paintIdx, paintR/G/B, paintCount, rootsRe, rootsIm }`) |

The `done` message uses transferable buffers for zero-copy transfer:
```js
self.postMessage({ type: 'done', ... }, [paintIdx.buffer, paintR.buffer, paintG.buffer, paintB.buffer]);
```

### Worker creation: inline Blob

Workers are created from inline code via `createFastModeWorkerBlob()`, which constructs a string containing the entire worker script (solver, step loop, WASM support), wraps it in a `Blob`, and returns an Object URL:

```js
function createFastModeWorkerBlob() {
    const code = `'use strict'; ... entire worker script as template literal ...`;
    return URL.createObjectURL(new Blob([code], { type: "application/javascript" }));
}
```

The URL is revoked immediately after `new Worker(blobUrl)` returns.

## WASM Fallback Pattern

The worker `init` handler implements a three-tier fallback for the solver engine:

```
1. Try WASM step loop (entire step loop in C/WASM, maximum performance)
2. Fall back to WASM solver-only (JS step loop, WASM for each EA solve call)
3. Fall back to pure JavaScript (everything in JS)
```

Code structure:

```js
S_useWasm = false;
S_useWasmLoop = false;
if (d.useWasm) {
    if (d.wasmStepLoopB64) {
        try { initWasmStepLoop(d); S_useWasmLoop = true; }
        catch(e) { S_useWasmLoop = false; }
    }
    if (!S_useWasmLoop && d.wasmB64) {
        try { initWasm(d.wasmB64, d.nCoeffs, d.nRoots); S_useWasm = true; }
        catch(e) { S_useWasm = false; }
    }
}
// Force JS for modes not supported by WASM step_loop.c
if (S_useWasmLoop && (S_idxProxColor || S_ratioColor)) S_useWasmLoop = false;
```

The WASM binaries are base64-encoded constants (`WASM_SOLVER_B64`, `WASM_STEP_LOOP_B64`) embedded directly in the HTML file. Workers decode them via `atob()` and instantiate with `new WebAssembly.Module()` / `new WebAssembly.Instance()`.

**Runtime fallback**: If the WASM step loop throws during `runStepLoop()`, the worker falls back to pure JS for that run and disables WASM for subsequent runs:

```js
try {
    pc = wasmLoopExports.runStepLoop(stepStart, stepEnd, elapsedOff);
} catch(wasmErr) {
    self.postMessage({type: 'error', ...});
    S_useWasmLoop = false;
    return;
}
```

The main-thread `solverType` variable (`"js"` or `"wasm"`) controls whether WASM binaries are sent to workers at all. It is user-selectable in the bitmap config popup.

## Persistent Buffer Pattern

The bitmap rendering system uses a split-resolution architecture to handle high-resolution computation without GPU memory limits:

### Two-buffer split

```
bitmapComputeRes   -- full computation resolution (from dropdown, e.g. 5000, 10000, 15000)
bitmapDisplayRes   -- display canvas resolution = min(computeRes, BITMAP_DISPLAY_CAP)
BITMAP_DISPLAY_CAP = 2000  -- max display canvas size (px)
```

**Canvas**: Gets `bitmapDisplayRes` dimensions (small GPU footprint):
```js
canvas.width = bitmapDisplayRes;
canvas.height = bitmapDisplayRes;
```

**Persistent buffer**: An `ImageData` object at full `bitmapComputeRes` (CPU-only, no GPU):
```js
bitmapPersistentBuffer = new ImageData(bitmapComputeRes, bitmapComputeRes);
```

**Display buffer**: Only allocated when `bitmapComputeRes > BITMAP_DISPLAY_CAP` (i.e. when split is active):
```js
if (bitmapComputeRes > BITMAP_DISPLAY_CAP) {
    bitmapDisplayBuffer = new ImageData(bitmapDisplayRes, bitmapDisplayRes);
}
```

### Composite pipeline

`compositeWorkerPixels()` merges sparse pixel data from all workers:

1. Each worker's `done` message provides: `Int32Array` of pixel indices, `Uint8Array` R/G/B channels, and a `paintCount`.
2. Pixels are written to the persistent buffer at compute resolution.
3. If split is active, each pixel is also downsampled to the display buffer (`invScale = dW / cW`).
4. A dirty rect is tracked in display-space (`minX, minY, maxX, maxY`).
5. Only the dirty rect is `putImageData`'d to the canvas:
   ```js
   bitmapCtx.putImageData(buf, 0, 0, minX, minY, maxX - minX + 1, maxY - minY + 1);
   ```

This eliminates `getImageData` entirely. The persistent buffer accumulates across passes; only changed pixels are updated. The dirty-rect `putImageData` provides approximately a 14x speedup compared to full-canvas writes at 10K resolution.

### Export at full resolution

Bitmap export reads from `bitmapPersistentBuffer` (full compute resolution), not from the display canvas. This allows exporting at 10K or 15K resolution even though the canvas is capped at 2000px.

## Sparse Pixel Output

Workers do not send full `W x H x 4` canvas buffers. Instead, each worker sends:

- `Int32Array paintIdx` -- flat pixel indices into the compute-resolution buffer
- `Uint8Array paintR, paintG, paintB` -- per-pixel color channels
- `int paintCount` -- number of valid entries

This is dramatically smaller than full buffers. For a 10K x 10K render with 1M steps and degree 5, a worker sends ~5M pixel entries (~25 MB) vs. 400 MB for a full RGBA buffer.

## State Management

### Global mutable state

Application state lives in top-level `let` variables. There is no framework, store, or state management library. Key state variables include:

- `coefficients[]` -- array of coefficient objects `{ re, im, curve, curveIndex, pathType, radius, speed, angle, ccw, extra }`
- `morphTargetCoeffs[]` -- parallel array for D-nodes, same structure
- `currentRoots[]` -- solved root positions `{ re, im }`
- `selectedCoeffs`, `selectedMorphCoeffs`, `selectedRoots` -- `Set<number>` for selection
- `panels.coeff`, `panels.roots` -- panel configuration (range, scales)
- `bitmapPersistentBuffer`, `bitmapCtx` -- bitmap rendering state
- `fastModeWorkers[]`, `fastModeActive`, `fastModeSharedData` -- worker state

### Coefficient data model

Each coefficient object carries both position and animation metadata:

```js
{
    re: number, im: number,          // current position
    curve: [{re, im}, ...],          // precomputed path points
    curveIndex: number,              // current position along curve
    pathType: string,                // "none", "circle", "lissajous", etc.
    radius: number,                  // path radius (0-200, % of coeffExtent)
    speed: number,                   // 0.001-1.0, loops per second
    angle: number,                   // rotation angle (0-1, fraction of 2pi)
    ccw: boolean,                    // counter-clockwise flag
    extra: {}                        // path-type-specific params (e.g. {freqX:3, freqY:2} for Lissajous)
}
```

- `coefficients[0]` = leading (highest power). New coeff via `unshift()` = new highest power.
- `"none"` path = 1-point curve at home position (not null).
- `Set` iteration order is insertion order -- sort `[...selectedCoeffs]` for deterministic index order.

### D-node data model

`morphTargetCoeffs[]` is a parallel array to `coefficients[]` with the same structure. D-nodes have no jiggle (jiggle only applies to C-coefficients). D-nodes support an additional path type `"follow-c"` not available to C-coefficients.

## Event Handling Conventions

### Interactive feedback

`uiBuzz()` provides haptic/visual feedback for invalid actions (e.g., clicking "Update" with no selection, trying to enter fast mode when already active).

### Throttled updates

Expensive operations like root solving and domain coloring use throttled wrappers (`solveRootsThrottled()`, `renderDomainColoringThrottled()`) so rapid UI interactions (slider dragging, coefficient dragging) do not cause frame drops.

### Animation loop

The animation system uses `requestAnimationFrame` with elapsed-time tracking. `animState` tracks play/pause state. The scrubber slider and seconds counter are updated each frame. Play/Pause/Resume follows a cycle controlled by `playBtn.textContent`.

---

## Trail Gallery

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T12-25-27.png" width="90%" alt="Degree-24 grid pattern -- circle path, one complete loop">
</p>

**Degree 24, grid pattern, circle path (one full loop)** -- 17 of 25 coefficients orbiting on a circle (radius 0.75, speed 0.5). Left panel shows the coefficient trails -- each selected coefficient traces its own circle, creating an overlapping ring pattern from the grid arrangement. Right panel shows the resulting root braids: small loops and cusps where roots nearly collide, large sweeping arcs where roots respond dramatically to the perturbation. The loop detection fired after one complete cycle, stopping trail accumulation automatically.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T12-29-26.png" width="90%" alt="Degree-24 grid pattern -- figure-8 path, one complete loop">
</p>

**Degree 24, grid pattern, figure-8 path** -- Same 17 coefficients, now following a figure-8 (lemniscate) at radius 1.5. The coefficient trails on the left form a dense weave of overlapping figure-8s. On the right, every root inherits the double-loop character -- small figure-8 knots appear throughout, with some roots tracing tight local loops while others sweep wide arcs. The larger radius amplifies the perturbation, pushing roots further and producing more dramatic braiding than the circle path above.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T12-48-07.png" width="90%" alt="Degree-30 two-clusters pattern -- square path">
</p>

**Degree 30, two-clusters pattern, square path** -- 11 of 31 coefficients (from one cluster) following a square path at radius 1.5, speed 0.4. The coefficient trails on the left show nested rectangles -- each selected coefficient traces its own square, offset by its position within the cluster. The 30 roots on the right arrange in a large ring with emerging trail segments showing the early stages of the braid. The two-cluster pattern concentrates coefficients into two groups, creating an asymmetric perturbation that pushes some roots into tight local orbits while others track the ring's perimeter.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T12-53-02.png" width="90%" alt="Degree-30 chessboard roots -- circle path, 5 coefficients">
</p>

**Degree 30, chessboard root shape, circle path** -- Only 5 of 31 coefficients selected, orbiting on a large circle (radius 2.0, speed 0.5). The roots were initialized in a chessboard grid pattern (a root shape, so the coefficients were computed from the grid). On the left, the 5 selected coefficients trace circles of varying sizes depending on their magnitude. On the right, most roots stay clustered near their grid positions while one outlier root swings through a wide arc -- a striking demonstration of how perturbing a few high-order coefficients can leave most roots nearly fixed while sending one root on a long excursion.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T13-00-17.png" width="90%" alt="Degree-30 diamond roots -- circle path, single coefficient">
</p>

**Degree 30, diamond root shape, circle path** -- A single coefficient (c8) orbiting on a large circle (radius 2.0, speed 0.4). The roots were initialized in a diamond arrangement. On the left, the lone selected coefficient traces one clean circle while the remaining coefficients sit near the origin. On the right, the 30 roots maintain their diamond shape but each traces a smooth rounded-square orbit -- the diamond's corners soften into curves as the perturbation sweeps around. The minimal input (one coefficient, one circle) produces a surprisingly coherent collective response: every root moves in concert, preserving the diamond's symmetry while the trail reveals the underlying geometry of the perturbation.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T13-37-45.png" width="90%" alt="Degree-5 -- 6 independent circle trajectories">
</p>

**Degree 5, circle pattern, 6 independent trajectories** -- Every coefficient (c0 through c5) assigned its own circle trajectory with different configurations: c0 on a large circle (radius 1.4, speed 0.6, CCW), c4 barely moving (radius 0.5, speed 0.1, CCW), and the rest at radius 0.5, speed 1.0 with alternating CW/CCW directions. On the left, six circles of varying size show each coefficient's individual orbit. On the right, the 5 roots trace complex entangled loops -- the interference between six independent perturbations at different frequencies and directions creates an intricate braid that no single-trajectory animation could produce. The loop detection fired after one complete cycle, confirming the combined motion is periodic.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T18-01-14.png" width="90%" alt="Degree-29 grid pattern -- 10 trajectories with mixed shapes and angles">
</p>

**Degree 29, grid pattern, 10 independent trajectories with mixed shapes and angles** -- 10 of 30 coefficients each with their own trajectory: circles (c0 at radius 1.4, c13 at 0.5, c17 at 1.0), a star (c14 at radius 0.15), an astroid (c15 at 0.3), and four horizontal oscillations (c20-c23 at varying speeds 0.2-0.8) plus c5 horizontal at angle 0.96. Each trajectory uses a different angle rotation, tilting the shapes relative to each other. On the left, the always-visible coefficient paths show the full variety -- circles, a star, an astroid, and angled horizontal lines radiating from their coefficients across the grid. On the right, the 29 root trails form a dense braid around a large ring, with tight cusps where roots nearly collide and sweeping arcs from the combined interference of all 10 perturbations at different frequencies and orientations.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T18-08-36.png" width="90%" alt="Degree-29 grid pattern -- mass horizontal perturbation, maximum chaos">
</p>

**Degree 29, grid pattern, mass horizontal perturbation** -- Building on the previous snap, now with nearly every coefficient animated. Two groups dominate: 12 coefficients (c1-c4, c9-c11, c16-c20) on horizontal trajectories at speed 0.7 with angle 0.73, and 5 coefficients (c24-c28) on faster horizontals at speed 1.2 with angle 0.29. The remaining 4 coefficients keep their circles, star, and astroid from before. On the left, the angled horizontal trajectories form a striking fan of parallel lines across the grid -- the angle slider tilts each group's oscillation axis differently. On the right, the root trails explode into tangled loops and whorls: with so many coefficients oscillating at different speeds and angles, the roots are pushed far from equilibrium, producing a chaotic braid where almost every root interacts with its neighbors.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T18-50-52.png" width="90%" alt="Degree-5 circle pattern -- Peano space-filling path, single coefficient">
</p>

**Degree 5, circle pattern, Peano space-filling path** -- A single coefficient (c1) tracing a Peano curve at radius 0.75, speed 0.1. On the left, the coefficient's path reveals the Peano curve's characteristic zigzag pattern -- dense, space-filling, covering the full square around the coefficient. On the right, each of the 5 roots responds by tracing its own miniature space-filling shape: the Peano structure propagates through the polynomial, producing self-similar fractal-like trails at each root position. The slow speed (0.1) allows the trails to accumulate cleanly, showing the full one-cycle braid. A striking demonstration of how a single space-filling perturbation on one coefficient induces fractal geometry across all roots.

<p align="center">
  <img src="../snaps/polypaint-2026-02-08T21-58-35.png" width="90%" alt="Degree-20 two-clusters -- dual Hilbert trajectories, all coefficients animated">
</p>

**Degree 20, two-clusters pattern, dual Hilbert (Moore curve) trajectories** -- All 21 coefficients animated on Hilbert trajectories at radius 0.65, speed 0.1, with complementary angles (0.75 and 0.25). One group of 10 coefficients from one cluster, the other 11 from the second cluster. On the left, the overlapping Hilbert curves form a dense, layered maze -- the two angle rotations tilt the space-filling grids against each other, creating a moire-like interference pattern. On the right, the 20 root trails each trace their own miniature Hilbert-like fractal, arranged in a large ring. Every root responds to the combined space-filling motion of both coefficient groups, producing intricate self-similar shapes at each root position -- some tightly wound, others more open depending on proximity to the coefficient clusters.

<p align="center">
  <img src="../snaps/polypaint-2026-02-09T07-33-41.png" width="90%" alt="Degree-28 diamond roots -- dual circle trajectories, web-like trail mesh">
</p>

**Degree 28, diamond root shape, dual circle trajectories at different speeds** -- Two coefficients animated on independent circles: c4 at radius 1.2, speed 0.6 (CW, angle 0.5) and c0 at radius 1.2, speed 0.1 (CCW, angle 1.0). The roots were initialized in a diamond arrangement. On the left, the two coefficient trajectories -- one large offset circle (c0 near the bottom-left) and one near the cluster at center -- show the asymmetric perturbation. On the right, the 28 roots maintain a large ring with the diamond's characteristic spacing, but the trails weave an intricate web of fine mesh-like filaments connecting neighboring roots. The speed ratio (6:1) between the two trajectories creates a dense Lissajous-like interference: the fast trajectory drives rapid oscillations while the slow one modulates the overall envelope, producing a cage-like lattice structure around the ring.
