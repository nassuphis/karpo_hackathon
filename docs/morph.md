# Morphing: Blend Two Coefficient Systems

## Overview

The morph system lets a second coefficient set **D** (morph targets, stored in `morphTargetCoeffs[]`) blend with the primary set **C** (`coefficients[]`). When enabled, the polynomial solved each frame uses blended coefficients:

```
blended[i] = C[i] * (1 - mu) + D[i] * mu
```

where `mu` oscillates between 0 and 1 via:

```
mu = 0.5 - 0.5 * cos(2 * pi * morphRate * elapsed)
```

D starts as a copy of C positions, lives in the "D-Nodes" tab (with a companion "D-List" for trajectory editing), and can be edited independently (dragged, given paths). The morph enable/rate/mu controls live in the "Final" tab bar, which shows the blended coefficients sent to the solver. This creates rich interference patterns as roots respond to the blended perturbation.

**File modified:** `index.html` (single-file app, ~13,912 lines)

---

## Global State (~line 1129)

```javascript
let morphTargetCoeffs = [];   // parallel to coefficients[], same {re,im,curve,...} structure
let morphEnabled = false;
let morphRate = 0.25;         // Hz (oscillation cycles/sec)
let morphMu = 0;              // current blend factor [0,1] — 0 when morph disabled
```

- `morphMu` defaults to 0. When the user enables morph, the handler sets `morphMu = 0.5` for an immediate 50/50 blend. When morph is disabled, mu resets to 0.
- The morph mu formula uses **cosine** (not sine): `0.5 - 0.5 * Math.cos(2 * Math.PI * morphRate * elapsed)`. This starts at 0 when elapsed=0 (unlike sine which starts at 0.5), providing a clean ramp-up from pure C to the first blend.

---

## HTML Structure

### Tab Buttons (~line 620, in `#left-tab-bar`)

Six tabs in the left panel:

```html
<button class="tab active" data-ltab="coeffs">C-Nodes ...</button>
<button class="tab" data-ltab="list">C-List</button>
<button class="tab" data-ltab="morph">D-Nodes ...</button>
<button class="tab" data-ltab="dlist">D-List</button>
<button class="tab" data-ltab="jiggle">Jiggle ...</button>
<button class="tab" data-ltab="final">Final</button>
```

- `data-ltab="morph"` maps to the "D-Nodes" tab (interactive SVG panel for dragging D coefficients)
- `data-ltab="dlist"` is the trajectory editor for D-nodes
- `data-ltab="final"` shows blended coefficients and hosts the morph enable/rate/mu controls

### D-Nodes Tab Content (~line 717)

```html
<div id="morph-content" class="tab-content">
    <div id="morph-bar" style="...">
        <button id="morph-copy-btn" class="bar-sel-btn">Copy C->D</button>
        <button id="morph-swap-btn" class="bar-sel-btn">Swap C<->D</button>
    </div>
    <div id="morph-container" style="flex:1;min-height:0;position:relative;">
        <svg id="morph-panel"></svg>
    </div>
</div>
```

### Final Tab Content (~line 801)

```html
<div id="final-content" class="tab-content">
    <div id="final-bar" style="...">
        <label><input type="checkbox" id="morph-enable"> Morph</label>
        <label>Rate</label>
        <input id="morph-rate" type="range" min="1" max="200" value="25" step="1">
        <span id="morph-rate-val">0.25 Hz</span>
        <span id="morph-mu-val">mu=0.50</span>
    </div>
    <div id="final-container" style="...">
        <svg id="final-panel"></svg>
    </div>
</div>
```

Rate slider: integer 1-200, displayed as value/100 Hz (0.01-2.00 Hz).

---

## Core Functions

### initMorphTarget() (~line 3201)

Creates D array as a deep copy of C positions with `pathType="none"`:

```javascript
function initMorphTarget() {
    morphTargetCoeffs = coefficients.map(c => ({
        re: c.re, im: c.im,
        pathType: "none", radius: 25, speed: 1, angle: 0, ccw: false, extra: {},
        curve: [{ re: c.re, im: c.im }], curveIndex: 0
    }));
    selectedMorphCoeffs.clear();
}
```

Called from: `applyPattern()`, `addCoefficientAt()`, `deleteCoefficient()`, `applyLoadedState()` (as fallback if `meta.morph` missing), and the init block. Degree changes reinitialize D via this function (D positions are lost).

### solveRoots() (~line 5308)

When morph is enabled, blends C and D before solving:

```javascript
function solveRoots() {
    let coeffsToSolve = coefficients;
    if (morphEnabled && morphTargetCoeffs.length === coefficients.length) {
        coeffsToSolve = coefficients.map((c, i) => ({
            re: c.re * (1 - morphMu) + morphTargetCoeffs[i].re * morphMu,
            im: c.im * (1 - morphMu) + morphTargetCoeffs[i].im * morphMu
        }));
    }
    // ... jiggle offsets, then solve
}
```

### animLoop() (~line 4353)

During animation, after advancing C-nodes and D-nodes along curves:

```javascript
if (morphEnabled) {
    morphMu = 0.5 - 0.5 * Math.cos(2 * Math.PI * morphRate * elapsed);
    const muEl = document.getElementById("morph-mu-val");
    if (muEl) muEl.textContent = "mu=" + morphMu.toFixed(2);
}
```

The morph panel visuals (ghost C positions, interp lines, interp markers) are also updated if visible.

### advanceToElapsed() (~line 3790)

Called by the scrub slider. Advances C-nodes along curves, then calls `advanceDNodesAlongCurves(elapsed)`, then updates morphMu:

```javascript
advanceDNodesAlongCurves(elapsed);
if (morphEnabled) {
    morphMu = 0.5 - 0.5 * Math.cos(2 * Math.PI * morphRate * elapsed);
}
```

### Home button handler (~line 3740)

Resets all coefficients to curve[0], resets D-nodes to curve[0], sets `morphMu = 0`, and re-renders.

---

## D-Node Path System

D-nodes support full trajectory paths (circle, ellipse, figure8, spiral, cloud, etc.) identically to C-nodes, plus one D-only path type.

### allAnimatedDCoeffs() (~line 3428)

Returns a Set of D-node indices that have animation paths (excludes "none" and "follow-c"):

```javascript
function allAnimatedDCoeffs() {
    const s = new Set();
    for (let i = 0; i < morphTargetCoeffs.length; i++) {
        const pt = morphTargetCoeffs[i].pathType;
        if (pt !== "none" && pt !== "follow-c") s.add(i);
    }
    return s;
}
```

### advanceDNodesAlongCurves(elapsed) (~line 3437)

Advances all D-nodes along their curves each frame:

```javascript
function advanceDNodesAlongCurves(elapsed) {
    for (let i = 0; i < morphTargetCoeffs.length; i++) {
        const d = morphTargetCoeffs[i];
        if (d.pathType === "follow-c" && i < coefficients.length) {
            d.re = coefficients[i].re; d.im = coefficients[i].im; continue;
        }
        if (d.pathType === "none") continue;
        // ... standard curve interpolation (same as C-nodes)
    }
}
```

### Follow-C Path Type

A D-only path type (`dOnly: true` in `PATH_CATALOG`, ~line 2600) where D[i] mirrors C[i]'s current position each frame:

- `PATH_PARAMS["follow-c"]` is `[]` (no parameters)
- Treated like "none" for `hasPath` checks: `allAnimatedDCoeffs()` skips it, save/load `hasPath` excludes it, curve regeneration skips it
- In `advanceDNodesAlongCurves()`, follow-c D-nodes copy from C: `d.re = coefficients[i].re; d.im = coefficients[i].im`
- In `applyPreview()` for morph snaps, follow-c is treated like none: `d.curve = [{re: d.re, im: d.im}]`

---

## Morph Panel SVG (D-Nodes Tab)

### Panel Variables (~line 1924)

```javascript
let morphSvg = null;
let morphGhostLayer = null;
let morphInterpLayer = null;
let morphLayer = null;
let morphPanelInited = false;
```

### initMorphPanel() (~line 1931)

Lazy-initialized on first tab switch to D-Nodes:

```javascript
function initMorphPanel() {
    morphSvg = d3.select("#morph-panel");
    setupPanel(morphSvg, panels.coeff);  // shares panels.coeff directly
    morphGhostLayer = morphSvg.append("g");
    morphInterpLayer = morphSvg.append("g");
    morphLayer = morphSvg.append("g");
    morphPanelInited = true;
}
```

Shares `panels.coeff` scales directly with the C-Nodes panel.

### renderMorphPanel() (~line 4576)

- Ghost layer: C coefficients as faint circles (opacity 0.25), labels c0..cn
- Interp layer: lines from C[i] to D[i], and small markers at the blended position `C[i]*(1-mu) + D[i]*mu`
- D layer: full-color draggable circles with labels d0..dn
- Both use `cxs()` and `cys()` from `panels.coeff` for coordinate mapping
- Colors: D[i] uses same color as C[i] via `coeffColor(i, n)`

### renderFinalPanel() (~line 4682)

Shows the actual blended coefficients sent to the solver:

```javascript
const doBlend = morphEnabled && nD === n;
for (let i = 0; i < n; i++) {
    if (doBlend) {
        re = c.re * (1 - morphMu) + morphTargetCoeffs[i].re * morphMu;
        im = c.im * (1 - morphMu) + morphTargetCoeffs[i].im * morphMu;
    } else {
        re = c.re; im = c.im;
    }
}
```

### Scale Sync

`autoScaleCoeffPanel()` (~line 2511) updates the morph panel grid when `panels.coeff.range` changes:

```javascript
if (morphPanelInited) {
    drawGrid(morphSvg, panels.coeff);
    // dot positions update on next renderMorphPanel() call
}
```

### rebuild() Handling

In `rebuild()`, after existing SVG teardown: clears morph SVG layers and sets `morphPanelInited = false` to force lazy re-init on next tab switch.

---

## Morph Control Event Handlers (~line 11689)

- **Enable checkbox** (~line 11690): toggle `morphEnabled`; when enabling set `morphMu = 0.5` (immediate 50/50); when disabling set `morphMu = 0`; call `solveRoots()` + update mu display
- **Rate slider** (~line 11698): `morphRate = this.value / 100`; update display span
- **Copy C->D** (~line 11702): call `initMorphTarget()`; re-render morph panel; call `solveRoots()` if morphEnabled
- **Swap C<->D** (~line 11707): For each i, swap `(C[i].re, C[i].im)` with `(D[i].re, D[i].im)`. Then:
  - For D[i]: update `D[i].curve = [{ re: D[i].re, im: D[i].im }]`
  - For C[i]: if `pathType === "none"`, update curve to single point. Otherwise recompute via `computeCurve()`.
  - Re-render coefficients, trails, morph panel, solve roots

---

## Mutually Exclusive Selection (C/D/Roots)

### Selection State (~line 1002)

Three independent `Set` objects:

```javascript
const selectedCoeffs = new Set();       // indices into coefficients[]
const selectedRoots = new Set();        // indices into currentRoots[]
const selectedMorphCoeffs = new Set();  // indices into morphTargetCoeffs[]
```

Selecting nodes in one category automatically clears the other two.

### Mutual Exclusion in Click Handlers

- **C-node click** (`toggleCoeffSelect()`): clears root and morph selections
- **D-node click** (morph drag end): clears coeff and root selections
- **Root click** (root drag end): clears coeff and morph selections

### Mutual Exclusion in Marquee Selection

`installMarqueeSelection(svgSel, getCirclesSel, which)` clears the other two categories at the start of a marquee drag.

### Target Indicator

`updateTransformGroupVisibility()` shows which category is active via `#ops-target`:
- "C" (green) if C-nodes selected
- "D" (blue) if D-nodes selected
- "roots" (red) if roots selected
- "none" (gray) if nothing selected

---

## D-Nodes as Ops Mid-Bar Targets

The three ops tools (Scale, Rotate, Translate) work on D-nodes when D-nodes are the active selection.

### snapshotSelection()

Priority: C-nodes > D-nodes > roots. The `which: "morph"` branch captures D-node positions from `morphTargetCoeffs[idx]`.

### applyPreview() Morph Branch

When `snap.which === "morph"`, applies transform to each selected D-node, then rebuilds curves:
- follow-c and none get single-point curves
- Other path types get full `computeCurve()` regeneration
- Updates morph panel visuals, D-list, and triggers solve if morph enabled

---

## Save/Load

### buildStateMetadata() (~line 7567)

D-node path fields are fully serialized:

```javascript
morph: {
    enabled: morphEnabled,
    rate: morphRate,
    mu: morphMu,
    target: morphTargetCoeffs.map(d => ({
        pos: [d.re, d.im],
        home: [d.curve[0].re, d.curve[0].im],
        pathType: d.pathType, radius: d.radius, speed: d.speed,
        angle: d.angle, ccw: d.ccw, extra: d.extra || {}
    }))
}
```

### applyLoadedState() (~line 8532)

Restores morph state including D-node trajectories. Key details:

```javascript
if (meta.morph) {
    morphEnabled = !!meta.morph.enabled;
    morphRate = meta.morph.rate ?? 0.25;
    morphMu = morphEnabled ? (meta.morph.mu ?? 0.5) : 0;
    if (meta.morph.target && meta.morph.target.length === coefficients.length) {
        morphTargetCoeffs = meta.morph.target.map(d => {
            const hasPath = d.pathType && d.pathType !== "none" && d.pathType !== "follow-c";
            const home = hasPath ? (d.home || d.pos) : d.pos;
            return {
                re: home[0], im: home[1],
                pathType: d.pathType || "none",
                radius: d.radius ?? 25, speed: d.speed ?? 1,
                angle: d.angle ?? 0, ccw: d.ccw ?? false,
                extra: d.extra || {},
                curve: [{ re: home[0], im: home[1] }], curveIndex: 0
            };
        });
        // Regenerate curves for D-nodes with paths (skip "follow-c" and "none")
        for (const d of morphTargetCoeffs) {
            if (d.pathType !== "none" && d.pathType !== "follow-c") {
                d.curve = computeCurve(d.curve[0].re, d.curve[0].im, d.pathType,
                    d.radius / 100 * coeffExtent(), d.angle, d.extra);
            }
            d.curveIndex = 0;
        }
    } else {
        initMorphTarget();
    }
    // Update UI elements (checkbox, rate slider, mu display)
} else {
    initMorphTarget();  // backward compat: old snaps without morph data
}
```

Backward compatibility: `morphMu` is forced to 0 when morph is disabled. Old snaps without `meta.morph` get a fresh `initMorphTarget()`.

---

## Fast Mode: Worker Morph Blend (JS Worker Blob)

Workers receive morph state via `serializeFastModeData()` (~line 11133):

```javascript
morphEnabled: morphEnabled && morphTargetCoeffs.length === nCoeffs,
morphRate: morphRate,
morphTargetRe: morphEnabled ? Float64Array.from(morphTargetCoeffs, d => d.re) : null,
morphTargetIm: morphEnabled ? Float64Array.from(morphTargetCoeffs, d => d.im) : null,
dFollowCIndices: morphEnabled ? morphTargetCoeffs.reduce((a, d, i) => {
    if (d.pathType === "follow-c") a.push(i); return a;
}, []) : [],
```

Also serialized: D-curve animation entries (`dAnimEntries`, `dCurvesFlat`, `dCurveOffsets`, `dCurveLengths`, `dCurveIsCloud`).

### Worker Persistent State (~line 10362)

```javascript
var S_morphEnabled = false, S_morphRate = 0;
var S_morphTargetRe = null, S_morphTargetIm = null;
var S_dCurvesFlat = null, S_dEntries = null, S_dOffsets = null, S_dLengths = null, S_dIsCloud = null;
var S_dFollowC = null;
```

### Worker Run Logic (per step, ~line 10480)

Each step, the worker:

1. **Pre-allocates morph target copies** (~line 10502): `morphRe = new Float64Array(S_morphTargetRe)` (copies base D positions, not mutating persistent state)
2. **Interpolates C-curves** (~line 10528): updates `coeffsRe`/`coeffsIm` for animated C-nodes
3. **Interpolates D-curves** (~line 10554): updates `morphRe`/`morphIm` for animated D-nodes along their curves
4. **Follow-C** (~line 10583): for each follow-c index `fci`, copies `coeffsRe[fci]` into `morphRe[fci]`
5. **Morph blend** (~line 10592):

```javascript
if (S_morphEnabled) {
    var mu = 0.5 - 0.5 * Math.cos(2 * Math.PI * S_morphRate * elapsed);
    var omu = 1 - mu;
    for (var m = 0; m < nCoeffs; m++) {
        coeffsRe[m] = coeffsRe[m] * omu + morphRe[m] * mu;
        coeffsIm[m] = coeffsIm[m] * omu + morphIm[m] * mu;
    }
}
```

6. **Jiggle offsets** (~line 10601): applied after morph blend
7. **Solve** (~line 10609): EA solver on the blended+jiggled coefficients

### Fast Mode Main-Thread Preview (~line 11500)

Between worker passes, the main thread also steps coefficients for UI preview. This code independently:
- Advances animated C-nodes on hi-res curves
- Advances animated D-nodes on their curves
- Copies Follow-C D-nodes from C positions
- Computes morph blend with the same cosine formula (~line 11565)
- Applies jiggle offsets
- Solves and renders for visual preview

---

## WASM Step Loop: Morph Blend in step_loop.c

The WASM step loop (`/Users/nicknassuphis/karpo_hackathon/step_loop.c`) implements the full per-step pipeline in C for maximum performance.

### Config Layout

Morph-related config entries:

| Config Index | Name | Type | Description |
|---|---|---|---|
| `CI_MORPH_ENABLED` (7) | int | Whether morph blending is active |
| `CI_N_DENTRIES` (9) | int | Number of animated D-curve entries |
| `CI_N_FOLLOWC` (10) | int | Number of follow-c D-node indices |
| `CD_MORPH_RATE` (2) | float64 | Morph oscillation rate in Hz |
| `CI_OFF_MORPH_TGT_RE` (27) | int (offset) | Byte offset to morph target Re array |
| `CI_OFF_MORPH_TGT_IM` (28) | int (offset) | Byte offset to morph target Im array |
| `CI_OFF_MORPH_WORK_RE` (57) | int (offset) | Byte offset to morph working Re array |
| `CI_OFF_MORPH_WORK_IM` (58) | int (offset) | Byte offset to morph working Im array |
| `CI_OFF_FOLLOWC_IDX` (36) | int (offset) | Byte offset to follow-c index array |

D-curve entries use offsets 44-52 (parallel arrays for idx, speed, ccw, dither, offsets, lengths, isCloud, flat data).

### Per-Step Pipeline in runStepLoop()

The C code mirrors the JS worker blob step-by-step:

```c
/* 2. Interpolate C-curves */
for (int a = 0; a < nEntries; a++) { /* ... standard curve interpolation ... */ }

/* 4. Interpolate D-curves */
if (nDEntries > 0 && dCurvesFlat) {
    for (int da = 0; da < nDEntries; da++) {
        /* ... same curve interpolation, writing to morphWorkRe/morphWorkIm ... */
    }
}

/* 5. Follow-C: D-nodes that mirror C-node position */
if (nFollowC > 0) {
    for (int fc = 0; fc < nFollowC; fc++) {
        int fci = followCIdx[fc];
        morphWorkRe[fci] = workCoeffsRe[fci];
        morphWorkIm[fci] = workCoeffsIm[fci];
    }
}

/* 6. Morph blend */
if (morphEnabled) {
    double mu = 0.5 - 0.5 * js_cos(2.0 * PI * morphRate * elapsed);
    double omu = 1.0 - mu;
    for (int m = 0; m < nc; m++) {
        workCoeffsRe[m] = workCoeffsRe[m] * omu + morphWorkRe[m] * mu;
        workCoeffsIm[m] = workCoeffsIm[m] * omu + morphWorkIm[m] * mu;
    }
}

/* 7. Apply jiggle offsets */
/* 8. Solve */
```

The WASM step loop uses `morphWorkRe`/`morphWorkIm` as pre-allocated working arrays (separate from the base `morphTargetRe`/`morphTargetIm`), so D-node base positions are preserved across steps while animated D-nodes are interpolated into the working copies each step.

### WASM Memory Layout

`computeWasmLayout()` (~line 10232) allocates space for all morph arrays:
- `morphTargetRe`/`morphTargetIm`: base D positions (nc doubles each)
- `morphWorkRe`/`morphWorkIm`: working copies for per-step interpolation (nc doubles each)
- `followCIdx`: array of follow-c D-node indices (nFollowC ints)
- D-curve flat data, entry arrays, offsets, lengths, isCloud arrays

The WASM `init()` function reads all config offsets and sets up data pointers. `runStepLoop()` performs the full step loop without crossing the JS/WASM boundary per step.

---

## CSS (~line 348)

```css
circle.morph-ghost { pointer-events: none; opacity: 0.25; }
circle.morph-coeff { cursor: grab; stroke-width: 1.5; }
circle.morph-coeff.dragging { cursor: grabbing; stroke-width: 2.5; }
.morph-label { font-size: 10px; fill: #aaa; pointer-events: none; }
.morph-ghost-label { font-size: 9px; fill: #555; pointer-events: none; }
line.morph-interp-line { pointer-events: none; }
circle.morph-interp-marker { pointer-events: none; }
circle.coeff.selected, circle.root.selected, circle.morph-coeff.selected { /* shared selection style */ }
```

---

## D-List Tab (~line 12536)

The D-List tab mirrors C-List for `morphTargetCoeffs[]` / `selectedMorphCoeffs`. Functions include:
- `refreshDCoeffList()` (~line 12695): rebuilds the D coefficient table
- `refreshDListCurveEditor()`: trajectory editor for selected D-node
- Path picker popups include "Follow C" option (inserted after "none")
- Bulk operations skip follow-c D-nodes (filter: `pt !== "none" && pt !== "follow-c"`)

---

## Touchpoints Summary

| Location | Description |
|----------|-------------|
| CSS (~line 348) | morph-ghost, morph-coeff, morph-label, interp-line, interp-marker styles |
| HTML tab bar (~line 620) | "D-Nodes", "D-List", "Final" tabs |
| HTML morph-content (~line 717) | D-Nodes SVG panel with Copy/Swap toolbar |
| HTML final-content (~line 801) | Final panel with morph enable/rate/mu controls |
| Selection state (~line 1002) | `selectedMorphCoeffs` Set |
| Global state (~line 1129) | morphTargetCoeffs, morphEnabled, morphRate, morphMu |
| Panel vars (~line 1924) | morphSvg, morphGhostLayer, morphInterpLayer, morphLayer, morphPanelInited |
| autoScaleCoeffPanel() (~line 2511) | Sync morph panel grid on coeff range change |
| PATH_CATALOG (~line 2600) | "follow-c" entry with `dOnly: true` |
| initMorphTarget() (~line 3201) | Create D from C, clear selectedMorphCoeffs |
| allAnimatedDCoeffs() (~line 3428) | Excludes "follow-c" from animated set |
| advanceDNodesAlongCurves() (~line 3437) | Follow-c copies from C; others interpolate along curves |
| updateMorphPanelDDots() (~line 3467) | Update D dot positions in morph SVG |
| Home button (~line 3740) | Reset D-nodes to curve[0], set morphMu=0 |
| advanceToElapsed() (~line 3790) | Advance D-nodes, update morphMu |
| animLoop() (~line 4353) | Advance D-nodes, update morphMu, update morph panel visuals |
| renderMorphPanel() (~line 4576) | Ghost C + draggable D + interp lines/markers on morph SVG |
| renderFinalPanel() (~line 4682) | Final panel showing blended coefficients |
| solveRoots() (~line 5308) | Blend coefficients when morphEnabled |
| buildStateMetadata() (~line 7567) | Serialize morph state including D-node path fields |
| applyLoadedState() (~line 8532) | Restore morph state with backward compat |
| Worker blob (~line 10362) | S_morphEnabled, S_morphRate, S_morphTargetRe/Im, S_dFollowC |
| Worker run (~line 10480) | Pre-allocated morphRe/Im, D-curve interpolation, follow-c copy, morph blend |
| step_loop.c | Full WASM step loop: D-curve interpolation, follow-c, morph blend in C |
| WASM layout (~line 10232) | computeWasmLayout allocates morph arrays |
| serializeFastModeData() (~line 11133) | Serialize morph + D-curve + follow-c data for workers |
| Fast mode preview (~line 11500) | Main-thread D-node advancement + morph blend between passes |
| Morph control handlers (~line 11689) | Enable, rate, copy, swap event handlers |
| D-List functions (~line 12536) | D-node trajectory editor with follow-c support |
