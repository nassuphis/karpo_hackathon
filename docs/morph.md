# Morphing: Blend Two Coefficient Systems

## Overview

The morph system lets a second coefficient set **D** (morph targets, stored in `morphTargetCoeffs[]`) blend with the primary set **C** (`coefficients[]`). Morph is **always enabled** (the checkbox was removed in v40). When any D-node differs from its corresponding C-node, the polynomial solved each frame uses interpolated coefficients computed along a configurable **C-D path**.

The default path type is **line** (linear interpolation):

```
blended[i] = C[i] * (1 - mu) + D[i] * mu
```

where `mu = 0.5 - 0.5 * cos(theta)` and `theta = 2 * pi * morphRate * elapsed`.

Three non-linear path types are also available: **circle**, **ellipse**, and **figure-8**. These trace parametric curves through a local coordinate frame centered on the midpoint of each C-D segment. A **dither** system adds per-step noise with three separate envelope controls (start, mid, end).

D starts as a copy of C positions, lives in the "D-Nodes" tab (with a companion "D-List" for trajectory editing), and can be edited independently (dragged, given paths). The morph mu display lives in the "Final" tab bar, which shows the blended coefficients sent to the solver.

**File modified:** `index.html` (single-file app, ~15,000 lines)

---

## Global State (~line 1125)

```javascript
let morphTargetCoeffs = [];   // parallel to coefficients[], same {re,im,curve,...} structure
let morphEnabled = true;      // always true (checkbox removed in v40)
let morphRate = 0.01;         // Hz (oscillation cycles/sec), range 0–0.01
let morphMu = 0;              // current blend factor [0,1]
let morphPathType = "line";   // "line" | "circle" | "ellipse" | "figure8"
let morphPathCcw = false;     // CW by default; applies to circle, ellipse, figure8
let morphEllipseMinor = 0.5;  // minor axis as fraction of major (0.1–1.0), ellipse only
let morphDitherStartSigma = 0; // 0–0.01 (% of coeffExtent), dither at C/start (max(cosθ,0)² envelope)
let morphDitherMidSigma = 0;   // 0–0.1 (% of coeffExtent), dither at midpoint (sin²θ envelope)
let morphDitherEndSigma = 0;   // 0–0.01 (% of coeffExtent), dither at D/end (max(-cosθ,0)² envelope)
let morphTheta = 0;           // current morph phase angle = 2π * morphRate * elapsed
```

- `morphEnabled` is always `true`. The enable checkbox was removed; morph auto-activates whenever D-nodes exist. `applyLoadedState()` forces `morphEnabled = true`.
- `morphRate` defaults to 0.01 Hz (range 0–0.01 Hz), configured via the C-D Path popup.
- `morphMu` is derived from `morphTheta` via the cosine formula. It starts at 0 when elapsed=0.

---

## C-D Path Interpolation (~line 1139)

### morphInterpPoint(cRe, cIm, dRe, dIm, theta, pathType, ccw, minorPct)

Computes the interpolated position between a C-node and D-node along the configured morph path. `theta = 2*pi*morphRate*elapsed` is the raw angle in radians.

```javascript
function morphInterpPoint(cRe, cIm, dRe, dIm, theta, pathType, ccw, minorPct) {
    const dx = dRe - cRe, dy = dIm - cIm;
    const len = Math.sqrt(dx * dx + dy * dy);
    if (len < 1e-15) return { re: cRe, im: cIm };
    if (pathType === "line") {
        const mu = 0.5 - 0.5 * Math.cos(theta);
        return { re: cRe * (1 - mu) + dRe * mu, im: cIm * (1 - mu) + dIm * mu };
    }
    // Local frame: u = unit C→D, v = perpendicular (90° CCW)
    const ux = dx / len, uy = dy / len;
    const vx = -uy, vy = ux;
    const midRe = (cRe + dRe) * 0.5, midIm = (cIm + dIm) * 0.5;
    const semi = len * 0.5;
    const sign = ccw ? 1 : -1;
    let lx, ly;
    if (pathType === "circle") {
        lx = -semi * Math.cos(theta);
        ly = sign * semi * Math.sin(theta);
    } else if (pathType === "ellipse") {
        const semi_b = minorPct * semi;
        lx = -semi * Math.cos(theta);
        ly = sign * semi_b * Math.sin(theta);
    } else { // figure8
        lx = -semi * Math.cos(theta);
        ly = sign * (semi * 0.5) * Math.sin(2 * theta);
    }
    return { re: midRe + lx * ux + ly * vx, im: midIm + lx * uy + ly * vy };
}
```

### Path Types

| Path Type | Parametric (local frame) | Notes |
|-----------|-------------------------|-------|
| **line** | `mu = 0.5 - 0.5*cos(θ)`, linear interpolation C→D | Default. Oscillates between C (θ=0) and D (θ=π) |
| **circle** | `lx = -semi*cos(θ)`, `ly = sign*semi*sin(θ)` | Full circle through C and D. CW/CCW toggle controls direction |
| **ellipse** | `lx = -semi*cos(θ)`, `ly = sign*(minor*semi)*sin(θ)` | Ellipse with configurable minor axis (0.1–1.0 of major). At 1.0, identical to circle |
| **figure-8** | `lx = -semi*cos(θ)`, `ly = sign*(semi/2)*sin(2θ)` | Figure-eight with crossover at midpoint. Uses `sin(2θ)` for double-frequency perpendicular oscillation |

### Local Coordinate Frame (non-line paths)

For circle, ellipse, and figure-8, interpolation uses a local coordinate frame:

1. **Origin**: midpoint of C-D segment `(midRe, midIm)`
2. **u-axis**: unit vector along C→D direction `(ux, uy) = (dx/len, dy/len)`
3. **v-axis**: perpendicular unit vector (90° CCW) `(vx, vy) = (-uy, ux)`
4. **semi**: half the C-D distance `len/2`
5. **sign**: `+1` for CCW, `-1` for CW

The local coordinates `(lx, ly)` are converted to world coordinates: `re = midRe + lx*ux + ly*vx`, `im = midIm + lx*uy + ly*vy`.

At `θ=0`, `lx = -semi` maps to the C position. At `θ=π`, `lx = +semi` maps to the D position.

---

## C-D Path Dither

Three separate dither sigma controls add uniform noise to the blended coefficients. Each has an envelope function that peaks at different points along the C-D path cycle:

| Control | Envelope | Peak Position | Range |
|---------|----------|---------------|-------|
| **Start σ** | `max(cos θ, 0)²` | θ=0 (C position) | 0–0.01% of coeffExtent |
| **Mid σ** | `sin²(θ)` | θ=π/2, 3π/2 (midpoints) | 0–0.1% of coeffExtent |
| **End σ** | `max(-cos θ, 0)²` | θ=π (D position) | 0–0.01% of coeffExtent |

These three envelopes form a **partition of unity** — they sum to 1 at every θ. The combined dither magnitude at each step is:

```javascript
const ds = startSigma/100 * ext * startEnv + midSigma/100 * ext * sin²θ + endSigma/100 * ext * endEnv;
```

When `ds > 0`, each blended coefficient gets uniform random offsets `±ds` in both Re and Im.

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
- `data-ltab="final"` shows blended coefficients and the mu display

### D-Nodes Tab Content (~line 717)

```html
<div id="morph-content" class="tab-content">
    <div id="morph-bar" style="...">
        <button id="morph-copy-btn" class="bar-sel-btn">Copy C→D</button>
        <button id="morph-swap-btn" class="bar-sel-btn">Swap C↔D</button>
        <button id="morph-cdpath-btn" class="bar-sel-btn">C-D Path</button>
    </div>
    <div id="morph-container" style="flex:1;min-height:0;position:relative;">
        <svg id="morph-panel"></svg>
    </div>
</div>
```

The "C-D Path" button opens the `buildCDPathPop()` popup for configuring path type, rate, direction, and dither.

### C-D Path Popup (`#cdpath-pop`, ~line 925)

```html
<div id="cdpath-pop" class="ops-pop" style="min-width:180px"></div>
```

Uses the standard `.ops-pop` CSS class, positioned below the C-D Path button via `getBoundingClientRect()`.

### Final Tab Content (~line 802)

```html
<div id="final-content" class="tab-content">
    <div id="final-bar" style="...">
        <span id="morph-mu-val" style="...">μ=0.50</span>
    </div>
    <div id="final-container" style="...">
        <svg id="final-panel"></svg>
    </div>
</div>
```

The Final bar contains only the mu display (no checkbox or rate slider — those moved to the C-D Path popup).

---

## C-D Path Popup: buildCDPathPop() (~line 11841)

The popup is built dynamically each time the "C-D Path" button is clicked. It snapshots all current values for revert on Escape/outside-click. Controls:

1. **Path type dropdown**: line (default), circle, ellipse, figure-8
2. **Rate slider**: 0–100 integer → 0.0000–0.0100 Hz (`morphRate = value / 10000`)
3. **CW/CCW toggle button**: hidden for line path; toggles `morphPathCcw`
4. **Minor axis slider**: shown only for ellipse; 10–100 integer → 0.10–1.00 (`morphEllipseMinor = value / 100`)
5. **Start σ slider**: 0–100 → 0.0000–0.0100% (`morphDitherStartSigma = value / 10000`)
6. **Mid σ slider**: 0–100 → 0.000–0.100% (`morphDitherMidSigma = value / 1000`)
7. **End σ slider**: 0–100 → 0.0000–0.0100% (`morphDitherEndSigma = value / 10000`)
8. **Accept button**: commits changes and closes popup

Accept/Revert workflow: closing without Accept (via Escape or outside-click) reverts all values to the snapshot.

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

### solveRoots() (~line 5416)

Uses `morphInterpPoint()` for C-D blending along the configured path type:

```javascript
function solveRoots() {
    let coeffsToSolve = coefficients;
    if (morphEnabled && morphTargetCoeffs.length === coefficients.length) {
        coeffsToSolve = coefficients.map((c, i) =>
            morphInterpPoint(c.re, c.im, morphTargetCoeffs[i].re, morphTargetCoeffs[i].im,
                             morphTheta, morphPathType, morphPathCcw, morphEllipseMinor));
    }
    // ... jiggle offsets, then solve
}
```

### animLoop() (~line 3910)

During animation, after advancing C-nodes and D-nodes along curves:

```javascript
if (morphEnabled) {
    morphTheta = 2 * Math.PI * morphRate * elapsed;
    morphMu = 0.5 - 0.5 * Math.cos(morphTheta);
    const muEl = document.getElementById("morph-mu-val");
    if (muEl) muEl.textContent = "μ=" + morphMu.toFixed(2);
}
```

Both `morphTheta` and `morphMu` are updated. The morph panel visuals (ghost C positions, interp lines, interp markers computed via `morphInterpPoint()`) are also updated if visible.

### advanceToElapsed() (~line 3910 / ~line 4496)

Called by the scrub slider. Advances C-nodes along curves, then calls `advanceDNodesAlongCurves(elapsed)`, then updates morphTheta and morphMu:

```javascript
advanceDNodesAlongCurves(elapsed);
if (morphEnabled) {
    morphTheta = 2 * Math.PI * morphRate * elapsed;
    morphMu = 0.5 - 0.5 * Math.cos(morphTheta);
}
```

### Home button handler (~line 3844)

Resets all coefficients to curve[0], resets D-nodes to curve[0], sets `morphMu = 0; morphTheta = 0`, and re-renders.

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

### Panel Variables (~line 1960)

```javascript
let morphSvg = null;
let morphGhostLayer = null;
let morphInterpLayer = null;
let morphLayer = null;
let morphPanelInited = false;
```

### initMorphPanel() (~line 1968)

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

### renderMorphPanel() (~line 4720)

- Ghost layer: C coefficients as faint circles (opacity 0.25), labels c0..cn
- Interp layer: lines from C[i] to D[i], and small markers at the interpolated position computed by `morphInterpPoint()`
- D layer: full-color draggable circles with labels d0..dn
- Both use `cxs()` and `cys()` from `panels.coeff` for coordinate mapping
- Colors: D[i] uses same color as C[i] via `coeffColor(i, n)`
- Interp markers use `morphInterpPoint()` with current `morphTheta`, `morphPathType`, `morphPathCcw`, and `morphEllipseMinor`

### renderFinalPanel() (~line 5025)

Shows the actual blended coefficients sent to the solver, computed via `morphInterpPoint()`:

```javascript
const doBlend = morphEnabled && nD === nC;
const mp = doBlend ? morphInterpPoint(c.re, c.im, morphTargetCoeffs[i].re, morphTargetCoeffs[i].im,
                                       morphTheta, morphPathType, morphPathCcw, morphEllipseMinor) : c;
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

## Morph Control Event Handlers (~line 12016)

- **Copy C->D** (~line 12017): call `initMorphTarget()`; re-render morph panel; call `solveRoots()` if morphEnabled
- **Swap C<->D** (~line 12022): For each i, swap `(C[i].re, C[i].im)` with `(D[i].re, D[i].im)`. Then:
  - For D[i]: update `D[i].curve = [{ re: D[i].re, im: D[i].im }]`
  - For C[i]: if `pathType === "none"`, update curve to single point. Otherwise recompute via `computeCurve()`.
  - Re-render coefficients, trails, morph panel, solve roots
- **C-D Path button** (~line 12016): opens `buildCDPathPop()` popup

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

### buildStateMetadata() (~line 7682)

D-node path fields and C-D path settings are fully serialized:

```javascript
morph: {
    enabled: morphEnabled,
    rate: morphRate,
    mu: morphMu,
    cdPathType: morphPathType,
    cdCcw: morphPathCcw,
    cdEllipseMinor: morphEllipseMinor,
    cdDitherStartSigma: morphDitherStartSigma,
    cdDitherMidSigma: morphDitherMidSigma,
    cdDitherEndSigma: morphDitherEndSigma,
    target: morphTargetCoeffs.map(d => ({
        pos: [d.re, d.im],
        home: [d.curve[0].re, d.curve[0].im],
        pathType: d.pathType, radius: d.radius, speed: d.speed,
        angle: d.angle, ccw: d.ccw, extra: d.extra || {}
    }))
}
```

### applyLoadedState() (~line 8653)

Restores morph state. Key details:

```javascript
morphEnabled = true; // always enabled (checkbox removed)
if (meta.morph) {
    morphRate = meta.morph.rate ?? 0.01;
    morphMu = meta.morph.mu ?? 0.5;
    morphPathType = meta.morph.cdPathType || "line";
    morphPathCcw = !!meta.morph.cdCcw;
    morphEllipseMinor = meta.morph.cdEllipseMinor ?? 0.5;
    morphDitherMidSigma = meta.morph.cdDitherMidSigma ?? meta.morph.cdDitherSigma ?? 0;
    morphDitherEndSigma = meta.morph.cdDitherEndSigma ?? 0;
    morphDitherStartSigma = meta.morph.cdDitherStartSigma ?? morphDitherEndSigma;
    morphTheta = 0;
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
} else {
    initMorphTarget();  // backward compat: old snaps without morph data
}
```

Backward compatibility: `cdDitherSigma` (old single-dither field) maps to `cdDitherMidSigma`. `morphTheta` is reset to 0 on load. Old snaps without `meta.morph` get a fresh `initMorphTarget()`.

---

## Fast Mode: Worker Morph Blend (JS Worker Blob)

Workers receive morph state via `serializeFastModeData()` (~line 11260):

```javascript
morphEnabled: morphEnabled && morphTargetCoeffs.length === nCoeffs,
morphRate: morphRate,
morphPathType: morphPathType,
morphPathCcw: morphPathCcw,
morphEllipseMinor: morphEllipseMinor,
morphDitherStartAbs: morphDitherStartSigma > 0 ? (morphDitherStartSigma / 100 * coeffExtent()) : 0,
morphDitherMidAbs: morphDitherMidSigma > 0 ? (morphDitherMidSigma / 100 * coeffExtent()) : 0,
morphDitherEndAbs: morphDitherEndSigma > 0 ? (morphDitherEndSigma / 100 * coeffExtent()) : 0,
morphTargetRe: morphEnabled ? Float64Array.from(morphTargetCoeffs, d => d.re) : null,
morphTargetIm: morphEnabled ? Float64Array.from(morphTargetCoeffs, d => d.im) : null,
dFollowCIndices: morphEnabled ? morphTargetCoeffs.reduce((a, d, i) => {
    if (d.pathType === "follow-c") a.push(i); return a;
}, []) : [],
```

Also serialized: D-curve animation entries (`dAnimEntries`, `dCurvesFlat`, `dCurveOffsets`, `dCurveLengths`, `dCurveIsCloud`).

### Worker Persistent State (~line 10457)

```javascript
var S_morphEnabled = false, S_morphRate = 0, S_morphTargetRe = null, S_morphTargetIm = null;
var S_morphPathType = "line", S_morphPathCcw = false, S_morphEllipseMinor = 0.5;
var S_morphDitherStartAbs = 0, S_morphDitherMidAbs = 0, S_morphDitherEndAbs = 0;
```

### Worker-Side morphInterpW() (~line 10461)

Mirrors the main-thread `morphInterpPoint()` but returns an array `[re, im]` instead of an object. Supports all 4 path types using the same local-frame math:

```javascript
function morphInterpW(cRe, cIm, dRe, dIm, theta) {
    // Uses S_morphPathType, S_morphPathCcw, S_morphEllipseMinor
    // Same parametric math as morphInterpPoint()
    // Returns [re, im] for performance
}
```

### Worker Run Logic (per step, ~line 10714)

Each step, the worker:

1. **Pre-allocates morph target copies**: `morphRe = new Float64Array(S_morphTargetRe)` (copies base D positions)
2. **Interpolates C-curves**: updates `coeffsRe`/`coeffsIm` for animated C-nodes
3. **Interpolates D-curves**: updates `morphRe`/`morphIm` for animated D-nodes along their curves
4. **Follow-C**: for each follow-c index, copies `coeffsRe[fci]` into `morphRe[fci]`
5. **Morph interpolation via morphInterpW()**: for each coefficient, computes interpolated position along the C-D path:

```javascript
if (S_morphEnabled) {
    var theta = 2 * Math.PI * S_morphRate * elapsed;
    for (var m = 0; m < nCoeffs; m++) {
        var mp = morphInterpW(coeffsRe[m], coeffsIm[m], morphRe[m], morphIm[m], theta);
        coeffsRe[m] = mp[0]; coeffsIm[m] = mp[1];
    }
}
```

6. **C-D path dither**: applies the three-envelope dither system:

```javascript
if (S_morphDitherStartAbs > 0 || S_morphDitherMidAbs > 0 || S_morphDitherEndAbs > 0) {
    var sinT = Math.sin(theta), cosT = Math.cos(theta);
    var startEnv = cosT > 0 ? cosT * cosT : 0;
    var endEnv = cosT < 0 ? cosT * cosT : 0;
    var mds = S_morphDitherStartAbs * startEnv + S_morphDitherMidAbs * sinT * sinT + S_morphDitherEndAbs * endEnv;
    if (mds > 0) for (var md = 0; md < nCoeffs; md++) {
        coeffsRe[md] += (Math.random() - 0.5) * 2 * mds;
        coeffsIm[md] += (Math.random() - 0.5) * 2 * mds;
    }
}
```

7. **Jiggle offsets**: applied after morph blend
8. **Solve**: EA solver on the blended+jiggled coefficients

### Fast Mode Main-Thread Preview (~line 11703)

Between worker passes, the main thread also steps coefficients for UI preview. This code independently:
- Advances animated C-nodes on hi-res curves
- Advances animated D-nodes on their curves
- Copies Follow-C D-nodes from C positions
- Computes morph interpolation via `morphInterpPoint()` along the configured path type
- Applies C-D path dither with the same three-envelope formula
- Applies jiggle offsets
- Solves and renders for visual preview

---

## WASM Step Loop: Morph in step_loop.c

The WASM step loop (`/Users/nicknassuphis/karpo_hackathon/step_loop.c`) implements the full per-step pipeline in C for maximum performance, including all 4 C-D path types and dither.

### Config Layout

Morph-related config entries:

| Config Index | Name | Type | Description |
|---|---|---|---|
| `cfgI32[7]` | morphEnabled | int | Whether morph blending is active |
| `cfgI32[9]` | nDEntries | int | Number of animated D-curve entries |
| `cfgI32[10]` | nFollowC | int | Number of follow-c D-node indices |
| `cfgI32[65]` | morphPathType | int | 0=line, 1=circle, 2=ellipse, 3=figure8 |
| `cfgI32[66]` | morphPathCcw | int | CW/CCW toggle |
| `cfgF64[0]` | bitmapRange | float64 | |
| `cfgF64[1]` | FAST_PASS_SECONDS | float64 | |
| `cfgF64[2]` | morphRate | float64 | Morph oscillation rate in Hz |
| `cfgF64[3]` | morphEllipseMinor | float64 | Ellipse minor axis fraction |
| `cfgF64[4]` | morphDitherStartAbs | float64 | Absolute start dither magnitude |
| `cfgF64[5]` | morphDitherMidAbs | float64 | Absolute mid dither magnitude |
| `cfgF64[6]` | morphDitherEndAbs | float64 | Absolute end dither magnitude |

D-curve entries use config offsets 44-52 (parallel arrays for idx, speed, ccw, dither, offsets, lengths, isCloud, flat data).

### WASM Path Type Encoding

Path type is encoded as an integer in `cfgI32[65]`:

```javascript
cfgI32[65] = ["line","circle","ellipse","figure8"].indexOf(d.morphPathType || "line");
```

### Per-Step Pipeline in runStepLoop()

The C code mirrors the JS worker blob step-by-step, including the full `morphInterpPoint()` equivalent for all 4 path types:

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

/* 6. Morph interpolation along C-D path (all 4 path types in C) */
if (morphEnabled) {
    double theta = 2.0 * PI * morphRate * elapsed;
    // For each coefficient: compute local frame, apply parametric curve
    // based on morphPathType (0=line, 1=circle, 2=ellipse, 3=figure8)
}

/* 7. C-D path dither (3 envelopes) */
/* 8. Apply jiggle offsets */
/* 9. Solve */
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
| HTML morph-content (~line 717) | D-Nodes SVG panel with Copy/Swap/C-D Path toolbar |
| HTML cdpath-pop (~line 925) | C-D Path popup container (`.ops-pop` class) |
| HTML final-content (~line 802) | Final panel with mu display only (no checkbox/rate) |
| Selection state (~line 1002) | `selectedMorphCoeffs` Set |
| Global state (~line 1125) | morphTargetCoeffs, morphEnabled, morphRate, morphMu, morphPathType, morphPathCcw, morphEllipseMinor, morphDitherStartSigma, morphDitherMidSigma, morphDitherEndSigma, morphTheta |
| morphInterpPoint() (~line 1139) | C-D path interpolation: 4 path types (line, circle, ellipse, figure-8) |
| Panel vars (~line 1960) | morphSvg, morphGhostLayer, morphInterpLayer, morphLayer, morphPanelInited |
| autoScaleCoeffPanel() (~line 2511) | Sync morph panel grid on coeff range change |
| PATH_CATALOG (~line 2600) | "follow-c" entry with `dOnly: true` |
| initMorphTarget() (~line 3201) | Create D from C, clear selectedMorphCoeffs |
| allAnimatedDCoeffs() (~line 3428) | Excludes "follow-c" from animated set |
| advanceDNodesAlongCurves() (~line 3437) | Follow-c copies from C; others interpolate along curves |
| updateMorphPanelDDots() (~line 3467) | Update D dot positions in morph SVG |
| Home button (~line 3844) | Reset D-nodes to curve[0], set morphMu=0, morphTheta=0 |
| animLoop() (~line 3910) | Update morphTheta/morphMu, advance D-nodes, update morph panel visuals |
| advanceToElapsed() (~line 4496) | Advance D-nodes, update morphTheta/morphMu |
| renderMorphPanel() (~line 4720) | Ghost C + draggable D + interp lines/markers (via morphInterpPoint) |
| renderFinalPanel() (~line 5025) | Final panel showing blended coefficients (via morphInterpPoint) |
| solveRoots() (~line 5416) | Blend coefficients via morphInterpPoint when morphEnabled |
| buildStateMetadata() (~line 7682) | Serialize morph state including C-D path settings and D-node paths |
| applyLoadedState() (~line 8653) | Restore morph state; morphEnabled always true; backward compat for old dither format |
| WASM config (~line 10330) | cfgI32[65]=pathType, cfgI32[66]=ccw, cfgF64[2-6]=rate/minor/dither |
| Worker state (~line 10457) | S_morphPathType, S_morphPathCcw, S_morphEllipseMinor, S_morphDitherStart/Mid/EndAbs |
| Worker morphInterpW() (~line 10461) | Worker-side C-D path interpolation (all 4 path types) |
| Worker run (~line 10714) | morphInterpW per coefficient, then 3-envelope dither, then jiggle, then solve |
| step_loop.c | Full WASM step loop: all 4 path types + 3-envelope dither in C |
| WASM layout (~line 10232) | computeWasmLayout allocates morph arrays |
| serializeFastModeData() (~line 11260) | Serialize morph + C-D path + dither + D-curve + follow-c data for workers |
| Fast mode preview (~line 11703) | Main-thread D-node advancement + morphInterpPoint + dither between passes |
| buildCDPathPop() (~line 11841) | C-D Path popup: path type, rate, CW/CCW, minor axis, 3 dither sliders, accept/revert |
| Morph control handlers (~line 12016) | Copy, Swap, C-D Path button event handlers |
| D-List functions (~line 12536) | D-node trajectory editor with follow-c support |
