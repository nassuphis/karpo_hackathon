# Plan: Morphing Feature — Blend Two Coefficient Systems

## Context
Add coefficient morphing: a second coefficient set D (morphTargetCoeffs) blends with the primary set C (coefficients). When enabled, the polynomial solved each frame uses `C[i]*(1-mu) + D[i]*mu` where `mu = 0.5 + 0.5*sin(2π*rate*elapsed)`. D starts as a copy of C, lives in the "D-Nodes" tab (with a companion "D-List" for trajectory editing), and can be edited independently (dragged). The morph enable/rate/mu controls live in the "Final" tab bar, which shows the blended coefficients sent to the solver. This creates rich interference patterns as roots respond to the blended perturbation.

## File Modified
- `index.html` (only file — all phases)

---

## Review Notes (issues found during code audit)

1. **morphMu=0 default**: Initial mu=0 means no blending when morph is disabled. When the user enables morph, mu is set to 0.5 for an immediate 50/50 blend. When morph is disabled, mu resets to 0.

2. **Scrub slider misses morph**: The scrub handler interpolates C along curves and calls `solveRootsThrottled()`, but doesn't update morphMu. **Fix**: morphMu update is now handled inside `advanceToElapsed()`, which the scrub slider calls.

3. **Morph panel scale sync**: When `autoScaleCoeffPanel()` (~line 2476) changes `panels.coeff.range`, the morph panel becomes stale. **Fix**: also update morph panel grid + positions if initialized.

4. **rebuild() must invalidate morph panel**: `rebuild()` (~line 12855) clears and rebuilds coeff/roots SVGs. Must also clear morph SVG layers and mark for lazy re-init.

5. **Swap C/D semantics**: Specified in step 1j — swap positions, recompute C curves from new homes, update D curves.

6. **Line number corrections**: Several approximate line numbers were wrong — all corrected below.

---

## Phase 1: Morph Tab + Interactive Morphing (D at fixed positions)

### 1a. Global State (~line 1120, before `solverType`)
```javascript
let morphTargetCoeffs = [];   // parallel to coefficients[], same {re,im,curve,...} structure
let morphEnabled = false;
let morphRate = 0.25;         // Hz (oscillation cycles/sec)
let morphMu = 0;              // current blend factor [0,1] — 0 when morph disabled
```
**Note**: morphMu defaults to 0. When the user enables morph, the enable handler sets `morphMu = 0.5` for an immediate 50/50 blend. When morph is disabled, mu resets to 0.

### 1b. initMorphTarget() (~line 3191, after `deleteCoefficient`)
Creates D array as deep copy of C positions with pathType="none":
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
Called from:
- `applyPattern()` (~line 5334)
- `addCoefficientAt()` (~line 3143)
- `deleteCoefficient()` (~line 3169)
- `applyLoadedState()` (~line 7677) — as fallback if `meta.morph` missing
- Init block

### 1c. HTML: Tab Buttons (line 620, in `#left-tab-bar`)
The left tab bar contains six tabs. The morph-related tabs are "D-Nodes" and "D-List":
```html
<button class="tab" data-ltab="coeffs">C-Nodes ...</button>
<button class="tab" data-ltab="list">C-List</button>
<button class="tab" data-ltab="morph">D-Nodes ...</button>
<button class="tab" data-ltab="dlist">D-List</button>
<button class="tab" data-ltab="jiggle">Jiggle ...</button>
<button class="tab" data-ltab="final">Final</button>
```
Note: `data-ltab="morph"` maps to the "D-Nodes" tab (interactive SVG panel for dragging D coefficients). "D-List" is the trajectory editor for D-nodes. "Final" shows blended coefficients and hosts the morph enable/rate/mu controls.

### 1d. HTML: D-Nodes Tab Content (~line 717)
The "D-Nodes" tab (`morph-content`) has a simple toolbar with Copy/Swap buttons and the interactive SVG panel:
```html
<div id="morph-content" class="tab-content">
    <div id="morph-bar" style="...">
        <button id="morph-copy-btn" class="bar-sel-btn" style="font-size:8px;">Copy C→D</button>
        <button id="morph-swap-btn" class="bar-sel-btn" style="font-size:8px;">Swap C↔D</button>
    </div>
    <div id="morph-container" style="flex:1;min-height:0;position:relative;">
        <svg id="morph-panel"></svg>
    </div>
</div>
```

The morph enable/rate/mu controls live in the **Final tab** bar (`final-bar`, ~line 801):
```html
<div id="final-content" class="tab-content">
    <div id="final-bar" style="...">
        <label style="font-size:10px;...">
            <input type="checkbox" id="morph-enable"> Morph
        </label>
        <label style="font-size:9px;color:var(--muted);">Rate</label>
        <input id="morph-rate" type="range" min="1" max="200" value="25" step="1" style="width:60px;...">
        <span id="morph-rate-val" style="...">0.25 Hz</span>
        <span id="morph-mu-val" style="...">μ=0.50</span>
    </div>
    <div id="final-container" style="flex:1;min-height:0;position:relative;overflow:hidden;">
        <svg id="final-panel" style="width:100%;height:100%;"></svg>
    </div>
</div>
```
Rate slider: integer 1–200 → display as value/100 Hz (0.01–2.00 Hz).

### 1e. Tab Registration (~line 10763)
`leftTabContents` (~line 10763):
```javascript
const leftTabContents = {
    coeffs: document.getElementById("coeffs-content"),
    list: document.getElementById("list-content"),
    morph: document.getElementById("morph-content"),
    dlist: document.getElementById("dlist-content"),
    jiggle: document.getElementById("jiggle-content"),
    final: document.getElementById("final-content")
};
```
`switchLeftTab()` (~line 10772) handles all tabs:
```javascript
if (tabName === "list") { refreshCoeffList(); refreshListCurveEditor(); }
if (tabName === "dlist") { refreshDCoeffList(); refreshDListCurveEditor(); }
if (tabName === "morph") renderMorphPanel();
if (tabName === "jiggle") buildJigglePanel();
if (tabName === "final") renderFinalPanel();
```

### 1f. Morph SVG Panel Setup (~line 1914, after roots panel setup)
Declare variables (lazy-initialized):
```javascript
let morphSvg = null;
let morphGhostLayer = null;
let morphLayer = null;
let morphPanelInited = false;
```

`initMorphPanel()` function (place near `renderMorphPanel`):
- `morphSvg = d3.select("#morph-panel")`
- Calls `setupPanel(morphSvg, panels.coeff)` — **shares `panels.coeff`** directly, same coordinate range
- Creates ghost + draggable layers
- Scale sync: `cxs()`/`cys()` from `panels.coeff` work because both SVGs have identical pixel dimensions (same panel container, `flex:1`)

**Scale sync mechanism**: `autoScaleCoeffPanel()` (~line 2476) already updates `panels.coeff.range` and calls `initPanelScales(panels.coeff)`. Since the morph panel shares `panels.coeff` scales, coordinates are automatically in sync. But grid lines and dot positions need updating:
```javascript
// In autoScaleCoeffPanel(), after initPanelScales(panels.coeff):
if (morphPanelInited) {
    drawGrid(morphSvg, panels.coeff);
    renderMorphPanel();
}
```

**rebuild() handling**: In `rebuild()` (~line 12855), after existing SVG teardown:
```javascript
if (morphPanelInited) {
    morphSvg.selectAll("*").remove();
    morphPanelInited = false;  // force lazy re-init on next tab switch
}
```

### 1g. renderMorphPanel() (~line 4559)
- Lazy-inits panel on first call: `if (!morphPanelInited) initMorphPanel();`
- Ghost layer: C coefficients as faint colored circles (opacity 0.25), labels c₀..cₙ
- D layer: full-color draggable circles with labels d₀..dₙ
- Both use `cxs()` and `cys()` from `panels.coeff` for coordinate mapping
- Colors: D[i] uses same color as C[i] via `coeffColor(i, n)` (~line 1958)
- Drag handler: updates `d.re`, `d.im`, `d.curve=[{re,im}]`; calls `solveRootsThrottled()` if morphEnabled
- Update mu display: `document.getElementById("morph-mu-val").textContent = "μ=" + morphMu.toFixed(2);`

### 1h. Modify solveRoots() (~line 5292)
When `morphEnabled && morphTargetCoeffs.length === coefficients.length`:
```javascript
function solveRoots() {
    let coeffsToSolve = coefficients;
    if (morphEnabled && morphTargetCoeffs.length === coefficients.length) {
        coeffsToSolve = coefficients.map((c, i) => ({
            re: c.re * (1 - morphMu) + morphTargetCoeffs[i].re * morphMu,
            im: c.im * (1 - morphMu) + morphTargetCoeffs[i].im * morphMu
        }));
    }
    let roots = solveRootsEA(coeffsToSolve, currentRoots);
    roots = matchRootOrder(roots, currentRoots);
    renderRoots(roots);
}
```

### 1i. Modify animLoop() (~line 4337)
After C coefficient interpolation and D-node advancement, before SVG updates:
```javascript
if (morphEnabled) {
    morphMu = 0.5 + 0.5 * Math.sin(2 * Math.PI * morphRate * elapsed);
    const muEl = document.getElementById("morph-mu-val");
    if (muEl) muEl.textContent = "μ=" + morphMu.toFixed(2);
}
```

### 1i-extra. Scrub slider morph handling
The scrub slider (~line 3721) lives in the header bar and calls `advanceToElapsed(elapsed)` (~line 3780) which handles morphMu update internally. The slider uses an additive model: it adds seconds to the current elapsed time when paused. `advanceToElapsed()` also advances C-nodes along curves, D-nodes along curves (via `advanceDNodesAlongCurves(elapsed)`), updates morph panel visuals, and calls `solveRootsThrottled()`.

### 1j. Morph Controls Event Handlers (~line 10785, after leftTabButtons listener)
- **Enable checkbox**: toggle `morphEnabled`; when enabling, set `morphMu = 0.5` (immediate 50/50 blend); when disabling, set `morphMu = 0`; call `solveRoots()` + update mu display
- **Rate slider**: `morphRate = this.value / 100`; update display span
- **Copy C→D**: call `initMorphTarget()`; re-render morph panel; call `solveRoots()` if morphEnabled
- **Swap C/D**: For each i, swap `(C[i].re, C[i].im)` with `(D[i].re, D[i].im)`. Then:
  - For D[i]: update `D[i].curve = [{ re: D[i].re, im: D[i].im }]` (pathType stays "none")
  - For C[i]: if `pathType === "none"`, update `C[i].curve = [{ re: C[i].re, im: C[i].im }]`. If C[i] has a trajectory, recompute its curve from the new home position via `computeCurve(...)`.
  - Re-render: `renderCoefficients()`, `renderCoeffTrails()`, `renderMorphPanel()`, `solveRoots()`

### 1k. Save/Load
**buildStateMetadata()** (~line 6870): D-node path fields are fully serialized (not just positions):
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

**applyLoadedState()** (~line 7677): restores morph state including D-node trajectories. When `morphMu` is restored, it is set to 0 if morph is disabled:
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
        // Regenerate curves for D-nodes with paths (skip "follow-c" — no own curve)
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
    // ... update UI elements ...
} else {
    initMorphTarget();
}
```

### 1l. CSS (~line 304, after existing coeff/root styles)
```css
circle.morph-ghost { pointer-events: none; opacity: 0.25; }
circle.morph-coeff { cursor: grab; stroke-width: 1.5; }
circle.morph-coeff.dragging { cursor: grabbing; stroke-width: 2.5; }
.morph-label { font-size: 10px; fill: #aaa; pointer-events: none; }
.morph-ghost-label { font-size: 9px; fill: #555; pointer-events: none; }
```

### 1m. Degree sync
Degree changes (pattern/slider/add/delete) all reinitialize D via `initMorphTarget()`. D positions are lost — this is intentional for simplicity.

---

**Phase 1 verification:**
1. Click D-Nodes tab → see D dots (identical to C initially) + faint C ghosts + grid
2. Enable morph checkbox (in Final tab) → roots immediately shift to 50/50 blend (mu=0.5)
3. Drag a D dot → roots shift in real time (blended polynomial updates)
4. Play → mu oscillates sinusoidally; roots morph between C and D configurations
5. Scrub → mu oscillates with scrub position; C moves along curves AND morph blends
6. Copy C→D resets D to current C positions; Swap exchanges C↔D positions (C curves recomputed from new homes)
7. Save/load preserves all morph state including D positions
8. Degree change reinitializes D
9. Window resize → morph panel rebuilds correctly on next tab switch

---

## Phase 2 (D Trajectories) + Phase 3 (Fast Mode Morphing) — Implemented

Both phases are now implemented:

- **Phase 2 (D Trajectories)**: D-nodes support full trajectory paths (circle, ellipse, figure8, spiral, cloud, etc.) via the "D-List" tab (`data-ltab="dlist"`, ~line 624). Functions include `allAnimatedDCoeffs()`, `advanceDNodesAlongCurves(elapsed)` (~line 3780 in `advanceToElapsed()`), and the D-List trajectory editor (~line 11809+). D-node path fields are saved/loaded with backward compatibility.
- **"Follow C" path type**: D-only path (`dOnly: true` in `PATH_CATALOG`, ~line 2590) where D[i] mirrors C[i]'s current position each frame. `PATH_PARAMS["follow-c"]` is `[]` (no parameters). Treated like "none" for `hasPath` checks: `allAnimatedDCoeffs()` skips it (`pt !== "none" && pt !== "follow-c"`), save/load `hasPath` excludes it, curve regeneration skips it. In `advanceDNodesAlongCurves()`, follow-c copies from C: `d.re = coefficients[i].re; d.im = coefficients[i].im`. `applyPreview()` for morph snaps treats follow-c like none: `d.curve = [{re: d.re, im: d.im}]`.
- **Phase 3 (Fast Mode Morphing)**: Workers receive morph state via `serializeFastModeData()`. Fast mode D-curve serialization, worker blob D-curve advancement per step, and pre-allocated `morphRe`/`morphIm` copies in the worker morph blend (avoiding mutation of persistent state). `dFollowCIndices` is serialized in `serializeFastModeData()` (~line 10241) and stored as `S_dFollowC` in the worker blob. During each step, the worker copies `coeffsRe[fci]`/`coeffsIm[fci]` into `morphRe[fci]`/`morphIm[fci]` for each follow-c index.

---

## Phase 4: Mutually Exclusive Selection (C/D/Roots)

Selecting nodes in one category (C-nodes, D-nodes, or roots) automatically clears the selection in the other two categories. This ensures exactly one category is active at a time, which the ops mid-bar uses to determine its target.

### Selection state (~line 993)
Three independent `Set` objects track selections:
```javascript
const selectedCoeffs = new Set();       // indices into coefficients[]
const selectedRoots = new Set();        // indices into currentRoots[]
const selectedMorphCoeffs = new Set();  // indices into morphTargetCoeffs[]
```

### Clear helpers (~line 2748)
```javascript
function clearCoeffSelection()  // clears selectedCoeffs, updates visual + anim bar
function clearRootSelection()   // clears selectedRoots, updates visual
function clearMorphSelection()  // clears selectedMorphCoeffs, updates visual
function clearAllSelection()    // calls all three
```

### Mutual exclusion in click handlers
- **C-node click** (`toggleCoeffSelect()`, ~line 2728): calls `clearRootSelection()` and `clearMorphSelection()` before toggling C-node selection
- **D-node click** (morph drag end, ~line 4546): calls `clearCoeffSelection()` and `clearRootSelection()` before toggling D-node selection
- **Root click** (root drag end, ~line 4819): calls `clearCoeffSelection()` and `clearMorphSelection()` before toggling root selection

### Mutual exclusion in marquee selection
`installMarqueeSelection(svgSel, getCirclesSel, which)` (~line 2833) clears the other two categories at the start of a marquee drag:
```javascript
if (which === "coeff") { clearRootSelection(); clearMorphSelection(); }
else if (which === "roots") { clearCoeffSelection(); clearMorphSelection(); }
else { clearCoeffSelection(); clearRootSelection(); }  // morph
```

### Target indicator
`updateTransformGroupVisibility()` (~line 2774) enables/disables the ops group and shows which category is active via the `#ops-target` span:
```javascript
if (!hasSel) { t.textContent = "· none"; t.style.color = "#888"; }
else if (hasCoeff) { t.textContent = "· C"; t.style.color = "#5ddb6a"; }
else if (hasMorph) { t.textContent = "· D"; t.style.color = "#4ea8de"; }
else { t.textContent = "· roots"; t.style.color = "#e94560"; }
```

### Context-aware All/None mid-bar buttons (~line 5472)
The All (select-all) and None (deselect-all) buttons in the mid-bar respect the current selection context:
- If D-nodes are selected → All/None operates on D-nodes
- Else if roots are selected → All/None operates on roots
- Else (default) → All/None operates on C-nodes (clearing D-nodes and roots)

---

## Phase 5: D-Nodes as Ops Mid-Bar Targets

The three ops tools (Scale, Rotate, Translate) in the mid-bar now work on D-nodes when D-nodes are the active selection. This is achieved through `snapshotSelection()` and `applyPreview()`, which both gained a `"morph"` branch.

### snapshotSelection() (~line 6059)
Creates a snapshot of the current selection for live-preview ops tools. Priority: C-nodes > D-nodes > roots.
```javascript
function snapshotSelection() {
    const items = [];
    if (selectedCoeffs.size > 0) {
        for (const idx of selectedCoeffs) { ... }
        return { which: "coeff", items };
    } else if (selectedMorphCoeffs.size > 0) {
        for (const idx of selectedMorphCoeffs) { ... }
        return { which: "morph", items };
    } else if (selectedRoots.size > 0) {
        for (const idx of selectedRoots) { ... }
        return { which: "roots", items };
    }
    return null;
}
```
The `which: "morph"` branch captures D-node positions from `morphTargetCoeffs[idx]`.

### applyPreview() morph branch (~line 6129)
When `snap.which === "morph"`, the transform function is applied to each selected D-node. After applying positions, curves are recomputed for all D-nodes (follow-c and none get single-point curves; other path types get full `computeCurve()` regeneration):
```javascript
} else if (snap.which === "morph") {
    for (const s of snap.items) {
        const d = morphTargetCoeffs[s.idx];
        const r = fn(s.re, s.im);
        d.re = r.re; d.im = r.im;
    }
    for (const d of morphTargetCoeffs) {
        if (d.pathType === "none" || d.pathType === "follow-c") {
            d.curve = [{ re: d.re, im: d.im }];
        } else {
            d.curve = computeCurve(...);
        }
        d.curveIndex = 0;
    }
    if (leftTab === "morph") renderMorphPanel();
    updateMorphSelectionVisual();
    if (leftTab === "dlist") { refreshDCoeffList(); refreshDListCurveEditor(); }
    if (morphEnabled) solveRootsThrottled();
}
```

### Workflow
1. Switch to D-Nodes tab, click or marquee-select D-nodes (C-nodes and roots are auto-deselected)
2. Mid-bar ops target indicator shows "D" in blue
3. Click Scale/Rotate/Translate tool — popover appears with live preview on D-nodes
4. Drag slider/pad — D-nodes move in real time; if morph is enabled, solver updates immediately

---

## Touchpoints Summary

| Location | Change |
|----------|--------|
| CSS (~line 198) | dnode-ctx styles (context menu for D-node trajectory editing) |
| CSS (~line 304) | morph-ghost, morph-coeff, morph-label styles |
| HTML tab bar (~line 620) | "D-Nodes" tab (`data-ltab="morph"`), "D-List" tab (`data-ltab="dlist"`), "Final" tab (`data-ltab="final"`) |
| HTML morph-content (~line 717) | D-Nodes SVG panel with Copy/Swap toolbar |
| HTML final-content (~line 801) | Final panel with morph enable/rate/mu controls |
| HTML mid-bar (~line 817) | Ops group with target indicator, Scale/Rotate/Translate buttons, All/None buttons |
| Selection state (~line 993) | `selectedCoeffs`, `selectedRoots`, `selectedMorphCoeffs` Sets |
| Global state (~line 1120) | morphTargetCoeffs, morphEnabled, morphRate, morphMu |
| Panel vars (~line 1914) | morphSvg, morphGhostLayer, morphLayer, morphPanelInited |
| autoScaleCoeffPanel() (~line 2476) | Sync morph panel grid + positions on coeff range change |
| toggleCoeffSelect() (~line 2728) | Mutually exclusive: clears D-node and root selections |
| clearCoeffSelection() / clearRootSelection() / clearMorphSelection() (~line 2748) | Per-category clear helpers |
| clearAllSelection() (~line 2768) | Calls all three clear helpers |
| updateTransformGroupVisibility() (~line 2774) | Ops target indicator: "C" / "D" / "roots" / "none" |
| installMarqueeSelection() (~line 2833) | Mutually exclusive clearing at marquee start per `which` param |
| openDNodeCtx() (~line 2960) | Right-click D-node context menu with live trajectory preview |
| PATH_PARAMS / PATH_CATALOG (~line 2548) | `"follow-c": []` (no params), `dOnly: true` in catalog (~line 2590) |
| addCoefficientAt() (~line 3143) | Call initMorphTarget() after state adjustments |
| deleteCoefficient() (~line 3169) | Call initMorphTarget() after state adjustments |
| initMorphTarget() (~line 3191) | Create D from C, clear selectedMorphCoeffs |
| allAnimatedDCoeffs() (~line 3418) | Excludes "follow-c" from animated set |
| advanceDNodesAlongCurves() (~line 3427) | Follow-c D-nodes copy from C[i] position |
| advanceToElapsed() (~line 3780) | Update morphMu, advance D-nodes along curves, update morph panel |
| animLoop() (~line 4337) | Mu update + display when morphEnabled |
| D-node click (morph drag end, ~line 4546) | Mutually exclusive: clears C-node and root selections |
| renderMorphPanel() (~line 4559) | Ghost C + draggable D on morph SVG |
| renderFinalPanel() (~line 4666) | Final panel showing blended coefficients |
| Root click (root drag end, ~line 4819) | Mutually exclusive: clears C-node and D-node selections |
| mid-all-btn / mid-none-btn (~line 5472) | Context-aware: operates on D-nodes if D-nodes selected |
| solveRoots() (~line 5292) | Blend coefficients when morphEnabled |
| applyPattern() (~line 5334) | Call initMorphTarget() |
| snapshotSelection() (~line 6059) | `"morph"` branch captures D-node positions for ops tools |
| applyPreview() (~line 6083) | `"morph"` branch applies transform to D-nodes, rebuilds curves |
| buildStateMetadata() (~line 6870) | Serialize morph state (including D-node path fields) |
| applyLoadedState() (~line 7677) | Restore morph state (including D-node trajectories, follow-c hasPath exclusion) |
| Worker blob (~line 9381) | `S_dFollowC` state, copies C→D per step for follow-c indices |
| serializeFastModeData() (~line 10116) | `dFollowCIndices` array for follow-c D-nodes (~line 10241) |
| leftTabContents (~line 10763) | morph, dlist, final entries |
| switchLeftTab() (~line 10772) | Handle morph, dlist, final tabs |
| D-List functions (~line 11809) | D-node trajectory editor (follow-c skipped in bulk ops) |
| rebuild() (~line 12855) | Invalidate morph panel (set morphPanelInited=false) |
