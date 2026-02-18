# D-Node Path Animation System

## Overview

D-nodes (morph targets stored in `morphTargetCoeffs[]`) have the same full data structure as C-coefficients, including `pathType`, `radius`, `speed`, `angle`, `ccw`, `extra`, `curve`, and `curveIndex`. A dedicated "D-List" tab mirrors the C-List tab, allowing D-nodes to be assigned paths (circles, spirals, etc.) and animate along them. D-node animation is integrated into all five animation entry points (animLoop, play/start, scrub, home, fast mode) and is fully supported in both the JS worker blob and the WASM step loop.

**Files:** `index.html` (main app, ~13,900 lines), `step_loop.c` (WASM step loop)

---

## Design Decisions

- **Six left-panel tabs**: C-Nodes, C-List, D-Nodes (morph panel SVG, `data-ltab="morph"`), D-List (`data-ltab="dlist"`), Jiggle, Final. D-List mirrors C-List for path editing. D-Nodes is the morph visualization SVG panel.
- **D radius uses C's `coeffExtent()`** -- shared coordinate space. Morph blends C and D in the same unit system.
- **`bitmapCoeffView` unchanged** -- currently plots C coefficient positions only. Adding a C/D toggle is deferred.
- **No jiggle offsets for D-nodes** -- jiggle only applies to C-coefficients. D-nodes have no jiggle support.
- **"Follow C" path type**: D-only path type (`"follow-c"`) that mirrors the corresponding C-node position at every step. Listed in `PATH_CATALOG` with `dOnly: true`, has empty params in `PATH_PARAMS`. `buildPathSelect()` accepts a `dNode` parameter to include D-only options. Treated like `"none"` for curve generation (single-point curve at home position). During animation, `follow-c` nodes copy from `coefficients[i].re/im` (or `workCoeffsRe[i]/Im[i]` in fast mode) at each step.
- **Speed resolution**: Speed values use 1-1000 integer range internally (stored as `speed * 1000`). The `_P.speed` schema defines `min: 1, max: 1000`, with `toUI: v => Math.round(v * 1000)` and `fromUI: v => v / 1000`. All UI displays use `Math.round(ci.speed * 1000)`.
- **`findDPrimeSpeed()` range**: Searches up to delta 2000 for coprime speeds (vs `findPrimeSpeed()` which searches up to 1000 for C-nodes). Range 1-2000. Skips `none` and `follow-c` nodes.
- **D-List editor simplification**: Same as C-List -- no node cycler (prev/next arrows), no PS button in the bulk editor. Only the per-coefficient path picker popup (from clicking the path cell in the list) retains the PS button. The editor uses `dleRefIdx` (first selected D-node index) as the reference for control values. "Update Whole Selection" applies editor params to all selected D-nodes.

---

## D-Node Context Menu

Right-clicking a D-node in the morph panel SVG opens a context menu (`#dnode-ctx`). This mirrors the C-coefficient context menu with full D-node-specific handling:

- **State**: `ctxDNodeIdx`, `ctxDNodeSnapshot`, `ctxDNodeQuadrant`
- **`openDNodeCtx(event, dIdx)`**: Snapshots the D-node's path state, builds a path type select (cloned from `#anim-path` with "Follow C" option inserted after "none"), dynamic parameter controls, and an Accept button
- **Live preview**: `previewDNodeCtx()` applies current menu settings immediately to the D-node, regenerates the curve. For `"none"` or `"follow-c"` paths, sets a single-point curve at current position.
- **Revert on outside click**: `closeDNodeCtx(true)` restores the snapshot. Accept button nulls the snapshot before closing, committing changes.
- **Quadrant repositioning**: Menu can be moved to different quadrants around the D-node via a rotate button. Position is calculated relative to the morph panel SVG using `cxs()/cys()` coordinate transforms.

---

## D-List Tab

### HTML Structure

- Tab button: `<button class="tab" data-ltab="dlist">D-List</button>` after D-Nodes button
- Full `#dlist-content` panel mirroring `#list-content` with `dlist-`/`dle-` prefixes
- `dpath-pick-pop` popup element for per-coefficient path picker
- Tab switching: `dlist` in `leftTabContents`, calls `refreshDCoeffList()` and `refreshDListCurveEditor()`

### Core Functions

All C-List functions mirrored for `morphTargetCoeffs[]` and `selectedMorphCoeffs`:

- **Path picker popup**: `openDPathPickPop(dIdx, anchorEl)`, `closeDPathPickPop(revert)` with live preview, PS button, and "Follow C" option. Snapshots D-node state; reverts on cancel, commits on accept.
- **Core list**: `refreshDCoeffList()`, `updateDListCoords()`, `updateDListPathCols()`. Rows show label (d0, d1, ...), path type button, speed, radius, curve length, curve index, coordinates. "Follow C" nodes show "Follow C" text for path; speed/radius columns show "-" for `none` and `follow-c` types.
- **Curve type cycler**: `buildDCurveCycleTypes()`, `updateDCurveCycleLabel()`, `selectByDCurveType(type)`. Cycles through distinct path types present across all D-nodes; clicking selects all D-nodes of that type.
- **Curve editor**: `refreshDListCurveEditor()`, `buildDleControls(pathType)`, `dleReadParams()`, `dleApplyToCoeff(di, params)`. Uses `dleRefIdx` (first selected D-node, sorted by index) as reference for control values. `dleApplyToCoeff()` treats `follow-c` like `"none"` for curve generation (single-point curve at current position).
- **"Update Whole Selection"** button (`#dle-update-sel`): Reads params from the editor, calls `dleApplyToCoeff()` for each selected D-node, then refreshes the list and updates morph panel if visible.
- **Prime speed**: `findDPrimeSpeed(currentIntSpeed, excludeSet)` -- finds nearest coprime integer speed among D-nodes. Searches up to delta 2000, range 1-2000. Skips `none` and `follow-c` nodes.
- **Select all / Deselect all**: Toolbar buttons add/remove all indices from `selectedMorphCoeffs`.

### Transforms

The D-List transform dropdown (`#dlist-transform`) provides 20 transforms targeting `morphTargetCoeffs[]` + `selectedMorphCoeffs`. Uses execute-then-reset pattern (fires on change, resets value to "none"). All transforms skip `follow-c` and `none` nodes where path parameters are involved (filter `pt !== "none" && pt !== "follow-c"` before applying speed, angle, direction, etc.).

Transforms: PrimeSpeeds, SetAllSpeeds, RandomSpeed, RandomAngle, RandomRadius, LerpSpeed, LerpRadius, LerpAngle, RandomDirection, FlipAllDirections, ShuffleCurves, ShufflePositions, CircleLayout, RotatePositions, ScalePositions, JitterPositions, Conjugate, InvertPositions, SortByModulus, SortByArgument.

After any D-List transform: calls `solveRootsThrottled()` if `morphEnabled`, `renderMorphPanel()` if on D-Nodes tab, refreshes D-List if on D-List tab.

### Ops Tools (Scale/Rotate/Translate)

Ops tools work on D-nodes when `selectedMorphCoeffs` is non-empty. `snapshotSelection()` returns `{ which: "morph", items }`, and `applyPreview()` handles the morph case: updates D positions, regenerates curves (follow-c nodes get single-point curve at current position like `"none"`).

---

## D-Node Animation -- All 5 Entry Points

### Helper Functions

- **`allAnimatedDCoeffs()`** (~line 3428): Returns Set of D-node indices with `pathType !== "none"` and `pathType !== "follow-c"`. Follow-c nodes are not considered "animated" for curve interpolation purposes (they mirror C-nodes instead).
- **`advanceDNodesAlongCurves(elapsed)`** (~line 3437): Iterates all D-nodes. For `follow-c` nodes, copies `coefficients[i].re/im` directly. For other animated nodes, interpolates position along their curve using `elapsed * speed * direction`, with cloud/smooth interpolation and optional dither.
- **`updateMorphPanelDDots()`** (~line 3467): Updates D-dot circle positions, label positions, and morph interpolation line endpoints in the morph panel SVG.

### Entry Points

1. **`animLoop()`** (~line 3815): After C-coefficient curve advancement, calls `advanceDNodesAlongCurves(elapsed)`. If morph panel is visible (`leftTab === "morph"`), calls `updateMorphPanelDDots()` and `updateMorphMarkers()`. At end, calls `updateDListCoords()`.

2. **Play button** (~line 3719): Allows animation if animated D-nodes exist (not just C): `if (allAnimatedCoeffs().size === 0 && allAnimatedDCoeffs().size === 0 && !morphEnabled) { uiBuzz(); return; }`. This means the user can animate with only D-node paths assigned.

3. **Scrub slider** (~line 3861): Additive scrubber in the header bar (`#anim-controls`). Calls `advanceToElapsed(elapsed)` (~line 3790) which advances both C-coefficients along their curves and D-nodes via `advanceDNodesAlongCurves(elapsed)`. Updates morph panel visuals including `updateMorphPanelDDots()`. Also calls `updateDListCoords()`.

4. **Home button** (~line 3739): Resets all D-nodes to `curve[0]`: `d.re = d.curve[0].re; d.im = d.curve[0].im; d.curveIndex = 0`. Calls `updateMorphPanelDDots()` and `updateMorphMarkers()` if morph panel is visible.

5. **Fast mode workers** -- see Fast Mode section below.

---

## Fast Mode -- D-Node Animation

### State

- `let fastModeDCurves = null;` (~line 1105) -- `Map<dNodeIndex, hi-res curve>` for animated D-nodes.

### enterFastMode() (~line 10928)

Computes `fastModeDCurves` for animated D-nodes (those from `allAnimatedDCoeffs()`) using `computeCurveN()` with `extentAtHome`, same as C-curve computation. Uses the D-node's home position (`d.curve[0].re/im`), not the current animated position.

### serializeFastModeData() (~line 11010)

Serializes D-curve data into the shared data object sent to workers:

- **`dAnimEntries`**: Array of `{ idx, ccw, speed, ditherSigma }` for each animated D-node
- **`dCurvesFlat`** (Float64Array): All D-curve points flattened as [re, im, re, im, ...]
- **`dCurveOffsets`**, **`dCurveLengths`**, **`dCurveIsCloud`**: Parallel metadata arrays
- **`dFollowCIndices`**: Array of D-node indices with `pathType === "follow-c"`, collected via `morphTargetCoeffs.reduce()`
- **`morphTargetRe/Im`**: Float64Array of current D-node positions (home positions for non-animated nodes)
- **`totalDPts`**: Total number of D-curve points across all animated D-nodes

### Worker Blob (JS)

**Persistent state** (~line 10364):
- `S_dCurvesFlat`, `S_dEntries`, `S_dOffsets`, `S_dLengths`, `S_dIsCloud`, `S_dFollowC`

**Init handler** (~line 10418): Parses D-curve data from init message, including `S_dFollowC = d.dFollowCIndices || []`.

**Run handler** (~line 10499): Pre-allocates `morphRe`/`morphIm` as copies of `S_morphTargetRe`/`S_morphTargetIm` (not mutating persistent state). Each step:

1. Advance C-curves: overwrite `coeffsRe`/`coeffsIm` for animated C-coefficients
2. Advance D-curves (~line 10555): overwrite animated D indices in `morphRe`/`morphIm`. Same interpolation logic as C-curves (cloud vs smooth, dither support).
3. Follow-C (~line 10584): copy `coeffsRe[fci]`/`coeffsIm[fci]` to `morphRe[fci]`/`morphIm[fci]` for each index in `dFollowC`
4. Morph blend (~line 10592): `coeffs = C * (1-mu) + morphRe/Im * mu` using cosine morph rate
5. Apply jiggle offsets post-interpolation (jiggle only applies to C-coefficients, but is added to the blended result)

### WASM Step Loop (`step_loop.c`)

The WASM step loop performs D-curve interpolation entirely in C, avoiding any JS boundary crossing per step.

**Memory layout** -- `computeWasmLayout()` (~line 10145) allocates dedicated regions for D-curve data:

| Region | Description |
|--------|-------------|
| `L.dIdx` | Int32Array -- D-entry coefficient indices |
| `L.dSpd` | Float64Array -- D-entry speeds |
| `L.dCcw` | Int32Array -- D-entry direction flags |
| `L.dDth` | Float64Array -- D-entry dither sigma values |
| `L.dOff` | Int32Array -- D-curve offsets into flat array |
| `L.dLen` | Int32Array -- D-curve lengths |
| `L.dCld` | Int32Array -- D-curve isCloud flags |
| `L.dcF` | Float64Array -- D-curves flat (re,im pairs) |
| `L.fCI` | Int32Array -- Follow-C index list |
| `L.mWR`/`L.mWI` | Float64Array -- Morph working arrays |

**Config slots** used by D-nodes:
- `cfgI32[9]` (`CI_N_DENTRIES`): Number of D-curve entries
- `cfgI32[10]` (`CI_N_FOLLOWC`): Number of follow-c indices
- `cfgI32[44-50]`: Byte offsets for D-curve entry arrays (`CI_OFF_DENTRY_IDX` through `CI_OFF_DCURVE_ISCLOUD`)
- `cfgI32[52]` (`CI_OFF_DCURVES_FLAT`): Byte offset for D-curves flat data

**`initWasmStepLoop(d)`** (~line 10200):
- Reads `dEntries = d.dAnimEntries || []`, `followC = d.dFollowCIndices || []`
- Calls `computeWasmLayout()` with `nDE = dEntries.length`, `nFC = followC.length`, `tDP = d.totalDPts`
- Writes D-entry parallel arrays: `dI[i] = dEntries[i].idx`, `dS[i] = dEntries[i].speed`, `dC2[i] = dEntries[i].ccw ? 1 : 0`, `dD[i] = dEntries[i].ditherSigma || 0`, plus `dO[i]`/`dL[i]`/`dCl[i]` for curve metadata
- Writes follow-c indices: `new Int32Array(buf, L.fCI, followC.length).set(followC)`
- Writes D-curves flat data: `new Float64Array(buf, L.dcF, tDP * 2).set(new Float64Array(d.dCurvesFlat))`
- Initializes morph working arrays from morph target base values

**`step_loop.c` -- per-step D-node handling**:

The C code in `runStepLoop()` processes D-nodes in three stages per step:

**Step 4 -- Interpolate D-curves** (~line 624 in step_loop.c):
```c
if (nDEntries > 0 && dCurvesFlat) {
    for (int da = 0; da < nDEntries; da++) {
        int dIdx = dEntryIdx[da];
        double dDir = dEntryCcw[da] ? -1.0 : 1.0;
        double dT = elapsed * dEntrySpeed[da] * dDir;
        double dU = frac01(dT);
        int dN = dCurveLengths[da];
        double dRawIdx = dU * dN;
        int dBase = dCurveOffsets[da] * 2;
        // Cloud: snap to nearest point
        // Smooth: linear interpolation between adjacent points
        // Writes to morphWorkRe[dIdx] / morphWorkIm[dIdx]
        // Dither: adds Gaussian noise if dEntryDither[da] > 0
    }
}
```

**Step 5 -- Follow-C** (~line 657 in step_loop.c):
```c
if (nFollowC > 0) {
    for (int fc = 0; fc < nFollowC; fc++) {
        int fci = followCIdx[fc];
        morphWorkRe[fci] = workCoeffsRe[fci];
        morphWorkIm[fci] = workCoeffsIm[fci];
    }
}
```
This copies the current (post-C-interpolation) C-coefficient position into the morph working array for follow-c D-nodes.

**Step 6 -- Morph blend** (~line 666 in step_loop.c):
```c
if (morphEnabled) {
    double mu = 0.5 - 0.5 * cos(2*PI*morphRate*elapsed);
    for (int m = 0; m < nc; m++) {
        workCoeffsRe[m] = workCoeffsRe[m] * (1-mu) + morphWorkRe[m] * mu;
        workCoeffsIm[m] = workCoeffsIm[m] * (1-mu) + morphWorkIm[m] * mu;
    }
}
```

The data pointers (`dEntryIdx`, `dEntrySpeed`, `dEntryCcw`, `dEntryDither`, `dCurveOffsets`, `dCurveLengths`, `dCurveIsCloud`, `dCurvesFlat`, `followCIdx`) and counts (`nDEntries`, `nFollowC`) are set once in `init()` from the config array, then used in every step without any JS boundary crossing.

### Legacy Fallback (`fastModeChunkLegacy`)

For browsers without Worker support (~line 11487):
- Advances D-nodes along `fastModeDCurves` each step, same interpolation as C-curves
- Follow-C D-nodes: copies current C-node position directly from `coefficients[fi]`
- Morph blending creates `coeffsToSolve` with animated C+D positions

### Cleanup

- `exitFastMode()` (~line 11470): nulls `fastModeDCurves`
- `reinitWorkersForJiggle()`: recomputes `fastModeDCurves`
- Legacy jiggle cycle: recomputes `fastModeDCurves`

---

## Save/Load

### buildStateMetadata() (~line 7567)

D-node serialization: `morphTargetCoeffs.map(d => ({ pos, home, pathType, radius, speed, angle, ccw, extra }))`. The `home` field stores `d.curve[0].re/im` (the curve start point, which is the pre-animation home position). The `pos` field stores `d.re/im` (current animated position). `follow-c` nodes are serialized with `pathType: "follow-c"`.

### applyLoadedState() (~line 8532)

Restores path fields with backward compat defaults: `d.pathType || "none"`, `d.radius ?? 25`, `d.speed ?? 1`, `d.angle ?? 0`, `d.ccw ?? false`, `d.extra || {}`. For nodes with paths (`pathType !== "none"` and `pathType !== "follow-c"`), the home position is used (`d.home || d.pos`); for `"none"` and `"follow-c"`, position is used directly. Curves are regenerated via `computeCurve()` for non-`"none"` and non-`"follow-c"` paths. `follow-c` and `"none"` nodes get a single-point curve at home position.

**Backward compat**: Old saves with only `{pos}` load as `pathType: "none"`, `home = pos`, single-point curve.

---

## Out of Scope (deferred)

- **`bitmapCoeffView` D toggle**: Currently plots C coefficient positions only
- **`coeffExtent` override**: D uses C's extent; per-set option deferred
- **Morph panel trail rendering**: Cosmetic, deferred

---

## Verification

1. Open D-List tab -- see all D-nodes with labels d0, d1, ... and "none" paths
2. Select D-nodes, assign circle/spiral paths -- verify curves generated, path columns update
3. Enable morph + Play -- D-nodes animate along paths, morph blending produces moving target
4. Scrub slider (header bar) -- D-nodes move along paths in sync with C
5. Home button -- D-nodes reset to curve[0]
6. Fast mode (bitmap) with animated D-nodes -- verify pixels accumulate correctly
7. Save/Load roundtrip -- D-node paths preserved (including follow-c)
8. Backward compat: load old snap -- D-nodes default to "none" paths
9. Assign "Follow C" to a D-node -- verify it mirrors the C-node position during animation and in fast mode
10. Verify D-List transforms skip follow-c nodes (e.g., PrimeSpeeds, SetAllSpeeds)
11. Right-click D-node in morph panel -- context menu opens with path controls, live preview, accept/revert
12. WASM step loop with animated D-nodes -- verify D-curves interpolated in C, no JS boundary crossing
13. Run existing test suite: `python -m pytest tests/ -v`
