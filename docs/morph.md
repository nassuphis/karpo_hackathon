# Plan: Morphing Feature — Blend Two Coefficient Systems

## Context
Add coefficient morphing: a second coefficient set D (morphTargetCoeffs) blends with the primary set C (coefficients). When enabled, the polynomial solved each frame uses `C[i]*(1-mu) + D[i]*mu` where `mu = 0.5 + 0.5*sin(2π*rate*elapsed)`. D starts as a copy of C, lives in a new "Morph" tab, and can be edited independently (dragged). This creates rich interference patterns as roots respond to the blended perturbation.

## File Modified
- `index.html` (only file — all phases)

---

## Review Notes (issues found during code audit)

1. **morphMu=0 usability bug**: Initial mu=0 means enabling morph without playing shows NO blending — D drags have no effect until Play is pressed. **Fix**: set `morphMu = 0.5` on enable, so blending is immediately visible.

2. **Scrub slider misses morph**: The scrub handler (line 3141) interpolates C along curves and calls `solveRootsThrottled()`, but doesn't update morphMu. **Fix**: add morphMu update in the scrub handler too.

3. **Morph panel scale sync**: When `autoScaleCoeffPanel()` (line 2246) changes `panels.coeff.range`, the morph panel becomes stale. **Fix**: also update morph panel grid + positions if initialized.

4. **rebuild() must invalidate morph panel**: `rebuild()` (line 10362) clears and rebuilds coeff/roots SVGs. Must also clear morph SVG layers and mark for lazy re-init.

5. **Swap C/D semantics**: Specified in step 1j — swap positions, recompute C curves from new homes, update D curves.

6. **Line number corrections**: Several approximate line numbers were wrong — all corrected below.

---

## Phase 1: Morph Tab + Interactive Morphing (D at fixed positions)

### 1a. Global State (~line 939, after `fastModeShowProgress`)
Insert after line 939 (`let fastModeShowProgress = true;`), before the `solverType` line:
```javascript
let morphTargetCoeffs = [];   // parallel to coefficients[], same {re,im,curve,...} structure
let morphEnabled = false;
let morphRate = 0.25;         // Hz (oscillation cycles/sec)
let morphMu = 0.5;            // current blend factor [0,1] — start at 0.5 so enabling morph immediately shows effect
```
**Note**: morphMu defaults to 0.5 (not 0) so that enabling morph without playing immediately shows a 50/50 blend.

### 1b. initMorphTarget() (after `deleteCoefficient`, ~line 2704)
Place new function after `deleteCoefficient()` ends (line 2703). Creates D array as deep copy of C positions with pathType="none":
```javascript
function initMorphTarget() {
    morphTargetCoeffs = coefficients.map(c => ({
        re: c.re, im: c.im,
        pathType: "none", radius: 25, speed: 1, angle: 0, ccw: false, extra: {},
        curve: [{ re: c.re, im: c.im }], curveIndex: 0
    }));
}
```
Called from:
- `applyPattern()` (line 4326) — after `initCoefficients(currentDegree)` (line 4326)
- `addCoefficientAt()` (line 2659) — after state adjustments, before `renderCoefficients()` (~line 2677)
- `deleteCoefficient()` (line 2684) — after state adjustments, before `renderCoefficients()` (~line 2699)
- `applyLoadedState()` (line 6329) — after curve regeneration (line 6382), as fallback if `meta.morph` missing
- Init block (line 10409) — after `initCoefficients(currentDegree)`

### 1c. HTML: Morph Tab Button (line 575, in `#left-tab-bar`)
Insert before the `<span class="tab-bar-controls">` on line 576:
```html
<button class="tab" data-ltab="morph">Morph</button>
```

### 1d. HTML: Morph Tab Content (between lines 671–672)
Insert between `</div>` closing `#list-content` (line 671) and `</div>` closing `#left-tab-panes` (line 672):
```html
<div id="morph-content" class="tab-content">
    <div id="morph-bar" style="padding:4px 8px;display:flex;align-items:center;gap:8px;flex-shrink:0;border-bottom:1px solid var(--stroke);background:rgba(10,12,30,0.45);">
        <label style="font-size:10px;display:flex;align-items:center;gap:3px;cursor:pointer;">
            <input type="checkbox" id="morph-enable"> Morph
        </label>
        <label style="font-size:9px;color:var(--muted);">Rate</label>
        <input id="morph-rate" type="range" min="1" max="200" value="25" step="1" style="width:60px;accent-color:var(--accent);">
        <span id="morph-rate-val" style="font-size:9px;color:var(--muted);min-width:36px;">0.25 Hz</span>
        <span id="morph-mu-val" style="font-size:9px;color:var(--accent);min-width:30px;">μ=0.50</span>
        <span style="flex:1;"></span>
        <button id="morph-copy-btn" class="bar-sel-btn" style="font-size:8px;">Copy C→D</button>
        <button id="morph-swap-btn" class="bar-sel-btn" style="font-size:8px;">Swap C↔D</button>
    </div>
    <div id="morph-container" style="flex:1;min-height:0;position:relative;">
        <svg id="morph-panel"></svg>
    </div>
</div>
```
Rate slider: integer 1–200 → display as value/100 Hz (0.01–2.00 Hz).

### 1e. Tab Registration (lines 8706–8718)
Add to `leftTabContents` (line 8706–8709):
```javascript
morph: document.getElementById("morph-content")
```
Extend `switchLeftTab()` (line 8711–8717):
```javascript
if (tabName === "morph") renderMorphPanel();
```

### 1f. Morph SVG Panel Setup (~line 1685, after roots panel setup)
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

**Scale sync mechanism**: `autoScaleCoeffPanel()` (line 2246) already updates `panels.coeff.range` and calls `initPanelScales(panels.coeff)`. Since the morph panel shares `panels.coeff` scales, coordinates are automatically in sync. But grid lines and dot positions need updating:
```javascript
// Add to autoScaleCoeffPanel(), after line 2258:
if (morphPanelInited) {
    drawGrid(morphSvg, panels.coeff);
    renderMorphPanel();
}
```

**rebuild() handling**: In `rebuild()` (line 10362), add after existing SVG teardown:
```javascript
if (morphPanelInited) {
    morphSvg.selectAll("*").remove();
    morphPanelInited = false;  // force lazy re-init on next tab switch
}
```

### 1g. renderMorphPanel() (~line 3735, after subscript function)
- Lazy-inits panel on first call: `if (!morphPanelInited) initMorphPanel();`
- Ghost layer: C coefficients as faint colored circles (opacity 0.25), labels c₀..cₙ
- D layer: full-color draggable circles with labels d₀..dₙ
- Both use `cxs()` and `cys()` from `panels.coeff` for coordinate mapping
- Colors: D[i] uses same color as C[i] via `coeffColor(i, n)` (line 1727)
- Drag handler: updates `d.re`, `d.im`, `d.curve=[{re,im}]`; calls `solveRootsThrottled()` if morphEnabled
- Update mu display: `document.getElementById("morph-mu-val").textContent = "μ=" + morphMu.toFixed(2);`

### 1h. Modify solveRoots() (line 4290)
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

### 1i. Modify animLoop() (line 3631)
After C coefficient interpolation loop ends (line 3656), before SVG updates (line 3658):
```javascript
if (morphEnabled) {
    morphMu = 0.5 + 0.5 * Math.sin(2 * Math.PI * morphRate * elapsed);
    const muEl = document.getElementById("morph-mu-val");
    if (muEl) muEl.textContent = "μ=" + morphMu.toFixed(2);
}
```

### 1i-extra. Modify scrub slider handler (line 3141)
After the coefficient interpolation loop (line 3167), before `solveRootsThrottled()` (line 3176):
```javascript
if (morphEnabled) {
    morphMu = 0.5 + 0.5 * Math.sin(2 * Math.PI * morphRate * elapsed);
    const muEl = document.getElementById("morph-mu-val");
    if (muEl) muEl.textContent = "μ=" + morphMu.toFixed(2);
}
```
This ensures scrubbing also drives the morph oscillation.

### 1j. Morph Controls Event Handlers (~line 8719, after leftTabButtons listener)
- **Enable checkbox**: toggle `morphEnabled`; when enabling, set `morphMu = 0.5` (immediate 50/50 blend); call `solveRoots()` + update mu display
- **Rate slider**: `morphRate = this.value / 100`; update display span
- **Copy C→D**: call `initMorphTarget()`; re-render morph panel; call `solveRoots()` if morphEnabled
- **Swap C/D**: For each i, swap `(C[i].re, C[i].im)` with `(D[i].re, D[i].im)`. Then:
  - For D[i]: update `D[i].curve = [{ re: D[i].re, im: D[i].im }]` (pathType stays "none")
  - For C[i]: if `pathType === "none"`, update `C[i].curve = [{ re: C[i].re, im: C[i].im }]`. If C[i] has a trajectory, recompute its curve from the new home position via `computeCurve(...)`.
  - Re-render: `renderCoefficients()`, `renderCoeffTrails()`, `renderMorphPanel()`, `solveRoots()`

### 1k. Save/Load
**buildStateMetadata()** (line 5816): add to returned object:
```javascript
morph: {
    enabled: morphEnabled,
    rate: morphRate,
    mu: morphMu,
    target: morphTargetCoeffs.map(d => ({ pos: [d.re, d.im] }))
}
```
Phase 1 D coefficients have no trajectories — only positions saved. Forward-compatible for Phase 2.

**applyLoadedState()** (line 6329): add after curve regeneration (after line 6382):
```javascript
if (meta.morph) {
    morphEnabled = !!meta.morph.enabled;
    morphRate = meta.morph.rate ?? 0.25;
    morphMu = meta.morph.mu ?? 0.5;
    if (meta.morph.target && meta.morph.target.length === coefficients.length) {
        morphTargetCoeffs = meta.morph.target.map(d => ({
            re: d.pos[0], im: d.pos[1],
            pathType: "none", radius: 25, speed: 1, angle: 0, ccw: false, extra: {},
            curve: [{ re: d.pos[0], im: d.pos[1] }], curveIndex: 0
        }));
    } else {
        initMorphTarget();
    }
    document.getElementById("morph-enable").checked = morphEnabled;
    document.getElementById("morph-rate").value = Math.round(morphRate * 100);
    document.getElementById("morph-rate-val").textContent = morphRate.toFixed(2) + " Hz";
    document.getElementById("morph-mu-val").textContent = "μ=" + morphMu.toFixed(2);
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
1. Click Morph tab → see D dots (identical to C initially) + faint C ghosts + grid
2. Enable morph checkbox → roots immediately shift to 50/50 blend (mu=0.5)
3. Drag a D dot → roots shift in real time (blended polynomial updates)
4. Play → mu oscillates sinusoidally; roots morph between C and D configurations
5. Scrub → mu oscillates with scrub position; C moves along curves AND morph blends
6. Copy C→D resets D to current C positions; Swap exchanges C↔D positions (C curves recomputed from new homes)
7. Save/load preserves all morph state including D positions
8. Degree change reinitializes D
9. Window resize → morph panel rebuilds correctly on next tab switch

---

## Future: Phase 2 (D Trajectories) + Phase 3 (Fast Mode Morphing)
Deferred. Phase 2 adds trajectory support for D coefficients (path types, animation). Phase 3 adds worker-based morphing for hires bitmap output. Both build on the Phase 1 foundation without changing its API.

---

## Touchpoints Summary

| Location | Change |
|----------|--------|
| CSS (~line 304) | morph-ghost, morph-coeff, morph-label styles |
| HTML tab bar (line 575) | Morph tab button |
| HTML tab panes (between 671–672) | morph-content div with toolbar + SVG |
| Global state (~line 939) | morphTargetCoeffs, morphEnabled, morphRate, morphMu |
| Panel vars (~line 1685) | morphSvg, morphGhostLayer, morphLayer, morphPanelInited |
| autoScaleCoeffPanel() (line 2246) | Sync morph panel grid + positions on coeff range change |
| initMorphTarget() (~line 2704) | New function to create D from C |
| addCoefficientAt (line 2659) | Call initMorphTarget() after state adjustments |
| deleteCoefficient (line 2684) | Call initMorphTarget() after state adjustments |
| scrub slider handler (line 3141) | Update morphMu when morphEnabled |
| renderMorphPanel() (~line 3735) | New: ghost C + draggable D on morph SVG |
| animLoop() (line 3631) | Mu update + display when morphEnabled |
| solveRoots() (line 4290) | Blend coefficients when morphEnabled |
| applyPattern() (line 4326) | Call initMorphTarget() |
| buildStateMetadata() (line 5816) | Serialize morph state |
| applyLoadedState() (line 6329) | Restore morph state |
| leftTabContents (line 8706) | Add morph entry |
| switchLeftTab() (line 8711) | Handle morph tab |
| Event listeners (~line 8719) | Morph control handlers |
| rebuild() (line 10362) | Invalidate morph panel (set morphPanelInited=false) |
| Init block (line 10409) | Call initMorphTarget() |
