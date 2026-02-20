# D-Node Path Animation System

## Overview

D-nodes (morph targets stored in `morphTargetCoeffs[]`) have the same full data structure as C-coefficients, including `pathType`, `radius`, `speed`, `angle`, `ccw`, `extra`, `curve`, and `curveIndex`. A dedicated "D-List" tab mirrors the C-List tab, allowing D-nodes to be assigned paths (circles, spirals, etc.) and animate along them. D-node animation is integrated into all five animation entry points (animLoop, play/start, scrub, home, fast mode) and is fully supported in both the JS worker blob and the WASM step loop.

**Files:** `index.html` (main app, ~15,000 lines), `step_loop.c` (WASM step loop)

---

## Design Decisions

- **Six left-panel tabs**: C-Nodes, C-List, D-Nodes (morph panel SVG, `data-ltab="morph"`), D-List (`data-ltab="dlist"`), Jiggle, Final. D-List mirrors C-List for path editing. D-Nodes is the morph visualization SVG panel.
- **Morph auto-activation** (v40): The morph checkbox has been removed. `morphEnabled` is always `true`. Morph auto-activates whenever any D-node exists. In `applyLoadedState()`, `morphEnabled = true` is set unconditionally (~line 8654). The system determines morph motion via `hasMorphMotion()` (~line 3514), which checks if any D-node position differs from its C-node by more than 1e-12 (skipping `follow-c` nodes).
- **C-D path interpolation** (v40): 4 path types between C and D positions: **line** (default), **circle**, **ellipse**, **figure-8**. Controlled by `morphPathType`, `morphPathCcw`, and `morphEllipseMinor` globals. Configured via the C-D Path popup (`#cdpath-pop`), opened by the morph-cdpath-btn button.
- **C-D path dither** (v40): 3 separate sigma controls (start/mid/end) with partition-of-unity envelopes. `morphDitherStartSigma` (0-0.01%), `morphDitherMidSigma` (0-0.1%), `morphDitherEndSigma` (0-0.01%). Envelopes: start = max(cos theta, 0)^2, mid = sin^2 theta, end = max(-cos theta, 0)^2.
- **D-node dither distribution** (v40): Per-D-entry (and per-C-entry) dither supports normal (Gaussian, default) or uniform distribution. Controlled by `ditherDist` in `extra`, serialized as `_ditherDist` on curves. `_DIST_PARAM` added to `PATH_PARAMS` for all dithered path variants. `_ditherRand(dist)` returns `(Math.random() - 0.5) * 2` for uniform or `_gaussRand()` for normal.
- **D radius uses C's `coeffExtent()`** -- shared coordinate space. Morph blends C and D in the same unit system.
- **`bitmapCoeffView` unchanged** -- currently plots C coefficient positions only. Adding a C/D toggle is deferred.
- **No jiggle offsets for D-nodes** -- jiggle only applies to C-coefficients. D-nodes have no jiggle support.
- **"Follow C" path type**: D-only path type (`"follow-c"`) that mirrors the corresponding C-node position at every step. Listed in `PATH_CATALOG` with `dOnly: true`, has empty params in `PATH_PARAMS`. `buildPathSelect()` accepts a `dNode` parameter to include D-only options. Treated like `"none"` for curve generation (single-point curve at home position). During animation, `follow-c` nodes copy from `coefficients[i].re/im` (or `workCoeffsRe[i]/Im[i]` in fast mode) at each step.
- **Speed resolution**: Speed values use 1-1000 integer range internally (stored as `speed * 1000`). The `_P.speed` schema defines `min: 1, max: 1000`, with `toUI: v => Math.round(v * 1000)` and `fromUI: v => v / 1000`. All UI displays use `Math.round(ci.speed * 1000)`.
- **`findDPrimeSpeed()` range**: Searches up to delta 2000 for coprime speeds (vs `findPrimeSpeed()` which searches up to 1000 for C-nodes). Range 1-2000. Skips `none` and `follow-c` nodes.
- **D-List editor simplification**: Same as C-List -- no node cycler (prev/next arrows), no PS button in the bulk editor. Only the per-coefficient path picker popup (from clicking the path cell in the list) retains the PS button. The editor uses `dleRefIdx` (first selected D-node index) as the reference for control values. "Update Whole Selection" applies editor params to all selected D-nodes.

---

## C-D Path Interpolation (v40)

The C-D path defines how the morph interpolation point travels between C-node and D-node positions. This is distinct from D-node path animation (which moves D-nodes along their own curves).

### morphInterpPoint() (~line 1139)

```js
function morphInterpPoint(cRe, cIm, dRe, dIm, theta, pathType, ccw, minorPct)
```

Computes the interpolated position between C and D along the configured path type:

- **line**: Linear interpolation using `mu = 0.5 - 0.5 * cos(theta)`. Returns `C * (1-mu) + D * mu`.
- **circle**: Traces a semicircular arc from C to D. Uses a local frame (u = unit C->D, v = perpendicular). `lx = -semi * cos(theta)`, `ly = sign * semi * sin(theta)`.
- **ellipse**: Like circle but with `ly = sign * (minorPct * semi) * sin(theta)`. The `morphEllipseMinor` parameter (10-100%) controls the minor axis ratio.
- **figure-8**: `lx = -semi * cos(theta)`, `ly = sign * (semi * 0.5) * sin(2 * theta)`. Double-frequency perpendicular motion creates the figure-8 shape.

All non-line paths share the same local coordinate frame: midpoint = `(C+D)/2`, semi-major = `|C-D|/2`, with CW/CCW direction toggle.

### State Variables (~line 1130)

```js
let morphPathType = "line";   // "line" | "circle" | "ellipse" | "figure8"
let morphPathCcw = false;     // CW by default; applies to circle, ellipse, figure8
let morphEllipseMinor = 0.5;  // minor axis as fraction of major (0.1-1.0), ellipse only
```

### C-D Path Popup (`buildCDPathPop()`, ~line 11841)

Opened by `#morph-cdpath-btn`. Contains:

- **Path type select**: line, circle, ellipse, figure-8
- **Rate slider**: 0-100 maps to 0.0000-0.0100 Hz
- **CW/CCW toggle**: Hidden for line path type
- **Minor axis slider**: 10-100%, shown only for ellipse
- **Start/Mid/End dither sigma sliders** (see C-D Path Dither below)
- **Accept button**: Commits changes; Escape/outside-click reverts all values to snapshot

---

## C-D Path Dither (v40)

Three separate dither sigma controls add noise along the C-D interpolation path, with partition-of-unity envelopes that weight the noise differently at different phases of the morph cycle.

### State Variables (~line 1133)

```js
let morphDitherStartSigma = 0; // 0-0.01 (% of coeffExtent), at C/start position
let morphDitherMidSigma = 0;   // 0-0.1 (% of coeffExtent), at midpoint
let morphDitherEndSigma = 0;   // 0-0.01 (% of coeffExtent), at D/end position
```

### Envelope Functions

The three sigma values are blended using partition-of-unity envelopes based on the morph angle `theta = 2 * PI * morphRate * elapsed`:

- **Start envelope** (at C): `cosT > 0 ? cosT^2 : 0` -- peaks when theta = 0 (at C position)
- **Mid envelope**: `sin^2(theta)` -- peaks when theta = PI/2 (at midpoint)
- **End envelope** (at D): `cosT < 0 ? cosT^2 : 0` -- peaks when theta = PI (at D position)

Combined sigma: `mds = startAbs * startEnv + midAbs * sin^2(theta) + endAbs * endEnv`

When `mds > 0`, uniform noise `(Math.random() - 0.5) * 2 * mds` is added to each coefficient's Re and Im.

### Serialization

- **Fast mode data**: Converted to absolute values (`% / 100 * coeffExtent()`) as `morphDitherStartAbs`, `morphDitherMidAbs`, `morphDitherEndAbs`
- **Save/load**: Stored as `cdDitherStartSigma`, `cdDitherMidSigma`, `cdDitherEndSigma` in the morph section. Backward compat: falls back to `cdDitherSigma` for mid sigma, and start defaults to end sigma value.

---

## D-Node Dither Distribution (v40)

Per-entry dither for both C and D nodes supports two distribution types:

### `_DIST_PARAM` (~line 2590)

```js
const _DIST_PARAM = { key: "ditherDist", type: "select", label: "Dist",
    options: [{ value: "normal", label: "Normal" }, { value: "uniform", label: "Uniform" }],
    default: "normal" };
```

Added to all dithered path variants via `PATH_PARAMS[key + "-dither"]` auto-generation (~line 2628).

### `_ditherRand(dist)` (~line 4295)

```js
function _ditherRand(dist) {
    return dist === "uniform" ? (Math.random() - 0.5) * 2 : _gaussRand();
}
```

- **normal** (default): Box-Muller Gaussian random
- **uniform**: Uniform random in [-1, 1]

### Worker `wDitherRand(dist)` (~line 10443)

```js
function wDitherRand(dist) {
    return dist ? (Math.random() - 0.5) * 2 : wGaussRand();
}
```

Worker uses integer encoding: `dist = 0` for normal, `dist = 1` for uniform. Serialized from `curve._ditherDist === "uniform" ? 1 : 0` in `serializeFastModeData()`.

### Storage

- Stored in `extra.ditherDist` on the coefficient data model
- Copied to `curve._ditherDist` during curve computation
- Serialized as integer `ditherDist` (0 or 1) in `animEntries` / `dAnimEntries` for fast mode

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

After any D-List transform: calls `solveRootsThrottled()` (morph is always enabled), `renderMorphPanel()` if on D-Nodes tab, refreshes D-List if on D-List tab.

### Ops Tools (Scale/Rotate/Translate)

Ops tools work on D-nodes when `selectedMorphCoeffs` is non-empty. `snapshotSelection()` returns `{ which: "morph", items }`, and `applyPreview()` handles the morph case: updates D positions, regenerates curves (follow-c nodes get single-point curve at current position like `"none"`).

---

## D-Node Animation -- All 5 Entry Points

### Helper Functions

- **`allAnimatedDCoeffs()`** (~line 3504): Returns Set of D-node indices with `pathType !== "none"` and `pathType !== "follow-c"`. Follow-c nodes are not considered "animated" for curve interpolation purposes (they mirror C-nodes instead).
- **`hasMorphMotion()`** (~line 3514): Returns true when any D-node position differs from its C-node by more than 1e-12. Skips follow-c nodes. Used to determine if morph blending will produce visible motion.
- **`advanceDNodesAlongCurves(elapsed)`** (~line 3525): Iterates all D-nodes. For `follow-c` nodes, copies `coefficients[i].re/im` directly. For other animated nodes, interpolates position along their curve using `elapsed * speed * direction`, with cloud/smooth interpolation and optional dither (supporting both normal and uniform distributions via `_ditherRand(curve._ditherDist)`).
- **`updateMorphPanelDDots()`** (~line 3556): Updates D-dot circle positions, label positions, and morph interpolation line endpoints in the morph panel SVG.

### Entry Points

1. **`animLoop()`** (~line 3815): After C-coefficient curve advancement, calls `advanceDNodesAlongCurves(elapsed)`. If morph panel is visible (`leftTab === "morph"`), calls `updateMorphPanelDDots()` and `updateMorphMarkers()`. At end, calls `updateDListCoords()`.

2. **Play button** (~line 3719): Allows animation if animated D-nodes exist (not just C): `if (allAnimatedCoeffs().size === 0 && allAnimatedDCoeffs().size === 0 && !morphEnabled) { uiBuzz(); return; }`. Since `morphEnabled` is always true (v40), this condition simplifies to allowing play whenever there are any animated coefficients.

3. **Scrub slider** (~line 3861): Additive scrubber in the header bar (`#anim-controls`). Calls `advanceToElapsed(elapsed)` (~line 3790) which advances both C-coefficients along their curves and D-nodes via `advanceDNodesAlongCurves(elapsed)`. Updates morph panel visuals including `updateMorphPanelDDots()`. Also calls `updateDListCoords()`.

4. **Home button** (~line 3739): Resets all D-nodes to `curve[0]`: `d.re = d.curve[0].re; d.im = d.curve[0].im; d.curveIndex = 0`. Calls `updateMorphPanelDDots()` and `updateMorphMarkers()` if morph panel is visible.

5. **Fast mode workers** -- see Fast Mode section below.

---

## Fast Mode -- D-Node Animation

### State

- `let fastModeDCurves = null;` (~line 1101) -- `Map<dNodeIndex, hi-res curve>` for animated D-nodes.

### enterFastMode() (~line 10928)

Computes `fastModeDCurves` for animated D-nodes (those from `allAnimatedDCoeffs()`) using `computeCurveN()` with `extentAtHome`, same as C-curve computation. Uses the D-node's home position (`d.curve[0].re/im`), not the current animated position.

### serializeFastModeData() (~line 11010)

Serializes D-curve data into the shared data object sent to workers:

- **`dAnimEntries`**: Array of `{ idx, ccw, speed, ditherSigma, ditherDist }` for each animated D-node. `ditherDist` is encoded as integer: `curve._ditherDist === "uniform" ? 1 : 0`.
- **`dCurvesFlat`** (Float64Array): All D-curve points flattened as [re, im, re, im, ...]
- **`dCurveOffsets`**, **`dCurveLengths`**, **`dCurveIsCloud`**: Parallel metadata arrays
- **`dFollowCIndices`**: Array of D-node indices with `pathType === "follow-c"`, collected via `morphTargetCoeffs.reduce()`
- **`morphTargetRe/Im`**: Float64Array of current D-node positions (home positions for non-animated nodes)
- **`morphPathType`**, **`morphPathCcw`**, **`morphEllipseMinor`**: C-D path interpolation parameters
- **`morphDitherStartAbs`**, **`morphDitherMidAbs`**, **`morphDitherEndAbs`**: C-D path dither sigma values converted to absolute units
- **`totalDPts`**: Total number of D-curve points across all animated D-nodes

### Worker Blob (JS)

**Persistent state** (~line 10447):
- `S_dCurvesFlat`, `S_dEntries`, `S_dOffsets`, `S_dLengths`, `S_dIsCloud`, `S_dFollowC`
- `S_morphPathType`, `S_morphPathCcw`, `S_morphEllipseMinor` (C-D path interpolation)
- `S_morphDitherStartAbs`, `S_morphDitherMidAbs`, `S_morphDitherEndAbs` (C-D path dither)

**Worker-side morph interpolation** -- `morphInterpW()` (~line 10462):

Mirrors main-thread `morphInterpPoint()`. Supports all 4 path types (line, circle, ellipse, figure-8). Uses `S_morphPathType`, `S_morphPathCcw`, `S_morphEllipseMinor` for path geometry.

**Init handler** (~line 10492): Parses D-curve data from init message, including `S_dFollowC = d.dFollowCIndices || []`. Also parses morph path type and C-D dither parameters.

**Run handler** (~line 10499): Pre-allocates `morphRe`/`morphIm` as copies of `S_morphTargetRe`/`S_morphTargetIm` (not mutating persistent state). Each step:

1. Advance C-curves: overwrite `coeffsRe`/`coeffsIm` for animated C-coefficients. Dither uses `wDitherRand(entries[a].ditherDist)` for normal/uniform distribution.
2. Advance D-curves (~line 10676): overwrite animated D indices in `morphRe`/`morphIm`. Same interpolation logic as C-curves (cloud vs smooth, dither via `wDitherRand(dEntries[da].ditherDist)`).
3. Follow-C (~line 10705): copy `coeffsRe[fci]`/`coeffsIm[fci]` to `morphRe[fci]`/`morphIm[fci]` for each index in `dFollowC`.
4. Morph blend (~line 10714): Calls `morphInterpW()` for each coefficient, replacing `coeffsRe/Im` with the interpolated C-D position. Supports all 4 path types.
5. C-D path dither (~line 10720): If any dither sigma > 0, computes partition-of-unity envelopes and adds uniform noise proportional to the combined sigma.
6. Apply jiggle offsets post-interpolation (jiggle only applies to C-coefficients, but is added to the blended result).

### WASM Step Loop (`step_loop.c`)

The WASM step loop performs D-curve interpolation and morph blending with all 4 C-D path types entirely in C, avoiding any JS boundary crossing per step.

**Memory layout** -- `computeWasmLayout()` (~line 10145) allocates dedicated regions for D-curve data:

| Region | Description |
|--------|-------------|
| `L.dIdx` | Int32Array -- D-entry coefficient indices |
| `L.dSpd` | Float64Array -- D-entry speeds |
| `L.dCcw` | Int32Array -- D-entry direction flags |
| `L.dDth` | Float64Array -- D-entry dither sigma values |
| `L.dDd` | Int32Array -- D-entry dither distribution (0=normal, 1=uniform) |
| `L.dOff` | Int32Array -- D-curve offsets into flat array |
| `L.dLen` | Int32Array -- D-curve lengths |
| `L.dCld` | Int32Array -- D-curve isCloud flags |
| `L.dcF` | Float64Array -- D-curves flat (re,im pairs) |
| `L.fCI` | Int32Array -- Follow-C index list |
| `L.mWR`/`L.mWI` | Float64Array -- Morph working arrays |
| `L.eDd` | Int32Array -- C-entry dither distribution (0=normal, 1=uniform) |

**Config slots** used by D-nodes and morph:
- `cfgI32[9]` (`CI_N_DENTRIES`): Number of D-curve entries
- `cfgI32[10]` (`CI_N_FOLLOWC`): Number of follow-c indices
- `cfgI32[44-50]`: Byte offsets for D-curve entry arrays (`CI_OFF_DENTRY_IDX` through `CI_OFF_DCURVE_ISCLOUD`)
- `cfgI32[52]` (`CI_OFF_DCURVES_FLAT`): Byte offset for D-curves flat data
- `cfgI32[65]`: Morph path type index (0=line, 1=circle, 2=ellipse, 3=figure8)
- `cfgI32[66]`: Morph path CW/CCW flag
- `cfgI32[67]`: Byte offset for C-entry dither distribution array
- `cfgI32[68]`: Byte offset for D-entry dither distribution array
- `cfgF64[3]`: morphEllipseMinor
- `cfgF64[4-6]`: morphDitherStartAbs, morphDitherMidAbs, morphDitherEndAbs

**`initWasmStepLoop(d)`** (~line 10200):
- Reads `dEntries = d.dAnimEntries || []`, `followC = d.dFollowCIndices || []`
- Calls `computeWasmLayout()` with `nDE = dEntries.length`, `nFC = followC.length`, `tDP = d.totalDPts`
- Writes D-entry parallel arrays: `dI[i]`, `dS[i]`, `dC2[i]`, `dD[i]`, `dDd[i]` (dither distribution), plus `dO[i]`/`dL[i]`/`dCl[i]` for curve metadata
- Writes morph path config: path type index, CW/CCW, ellipse minor ratio, and C-D dither sigma values into config arrays
- Writes follow-c indices: `new Int32Array(buf, L.fCI, followC.length).set(followC)`
- Writes D-curves flat data: `new Float64Array(buf, L.dcF, tDP * 2).set(new Float64Array(d.dCurvesFlat))`
- Initializes morph working arrays from morph target base values

**`step_loop.c` -- per-step D-node and morph handling**:

The C code in `runStepLoop()` processes D-nodes and morph in stages per step:

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
        // Dither: adds noise if dEntryDither[da] > 0
        //   (distribution type from dEntryDitherDist)
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

**Step 6 -- Morph blend with C-D path interpolation** (~line 666 in step_loop.c):

The WASM step loop supports all 4 C-D path types (line, circle, ellipse, figure-8). The morph path type index (0-3) is read from the config array once during init. Per step:

```c
if (morphEnabled) {
    double theta = 2 * PI * morphRate * elapsed;
    // For "line" (pathType=0):
    //   mu = 0.5 - 0.5 * cos(theta);
    //   workCoeffs = C * (1-mu) + morphWork * mu;
    // For "circle"/"ellipse"/"figure8" (pathType=1/2/3):
    //   Computes local frame (u, v) from C-D vector
    //   Applies parametric path equation in local coordinates
    //   Transforms back to global coordinates
    // C-D path dither:
    //   Computes partition-of-unity envelopes (start/mid/end)
    //   Combined sigma = startAbs*startEnv + midAbs*sin^2 + endAbs*endEnv
    //   Adds uniform noise if sigma > 0
}
```

The data pointers (`dEntryIdx`, `dEntrySpeed`, `dEntryCcw`, `dEntryDither`, `dEntryDitherDist`, `dCurveOffsets`, `dCurveLengths`, `dCurveIsCloud`, `dCurvesFlat`, `followCIdx`) and counts (`nDEntries`, `nFollowC`) are set once in `init()` from the config array, then used in every step without any JS boundary crossing.

### Legacy Fallback (`fastModeChunkLegacy`)

For browsers without Worker support (~line 11487):
- Advances D-nodes along `fastModeDCurves` each step, same interpolation as C-curves
- Follow-C D-nodes: copies current C-node position directly from `coefficients[fi]`
- Morph blending creates `coeffsToSolve` with animated C+D positions
- Supports dither distribution (normal/uniform) via `_ditherRand(curve._ditherDist)`

### Cleanup

- `exitFastMode()` (~line 11470): nulls `fastModeDCurves`
- `reinitWorkersForJiggle()`: recomputes `fastModeDCurves`
- Legacy jiggle cycle: recomputes `fastModeDCurves`

---

## Save/Load

### buildStateMetadata() (~line 7567)

D-node serialization: `morphTargetCoeffs.map(d => ({ pos, home, pathType, radius, speed, angle, ccw, extra }))`. The `home` field stores `d.curve[0].re/im` (the curve start point, which is the pre-animation home position). The `pos` field stores `d.re/im` (current animated position). `follow-c` nodes are serialized with `pathType: "follow-c"`.

Morph config serialization includes (v40):
- `cdPathType`: morphPathType
- `cdCcw`: morphPathCcw
- `cdEllipseMinor`: morphEllipseMinor
- `cdDitherStartSigma`: morphDitherStartSigma
- `cdDitherMidSigma`: morphDitherMidSigma
- `cdDitherEndSigma`: morphDitherEndSigma

### applyLoadedState() (~line 8532)

Restores path fields with backward compat defaults: `d.pathType || "none"`, `d.radius ?? 25`, `d.speed ?? 1`, `d.angle ?? 0`, `d.ccw ?? false`, `d.extra || {}`. For nodes with paths (`pathType !== "none"` and `pathType !== "follow-c"`), the home position is used (`d.home || d.pos`); for `"none"` and `"follow-c"`, position is used directly. Curves are regenerated via `computeCurve()` for non-`"none"` and non-`"follow-c"` paths. `follow-c` and `"none"` nodes get a single-point curve at home position.

`morphEnabled = true` is set unconditionally (checkbox removed in v40).

Morph config restoration (v40):
- `morphPathType = meta.morph.cdPathType || "line"`
- `morphPathCcw = !!meta.morph.cdCcw`
- `morphEllipseMinor = meta.morph.cdEllipseMinor ?? 0.5`
- `morphDitherMidSigma = meta.morph.cdDitherMidSigma ?? meta.morph.cdDitherSigma ?? 0` (backward compat with single sigma)
- `morphDitherEndSigma = meta.morph.cdDitherEndSigma ?? 0`
- `morphDitherStartSigma = meta.morph.cdDitherStartSigma ?? morphDitherEndSigma` (defaults to end sigma)

**Backward compat**: Old saves with only `{pos}` load as `pathType: "none"`, `home = pos`, single-point curve. Old saves without C-D path fields default to line path, no dither.

---

## Out of Scope (deferred)

- **`bitmapCoeffView` D toggle**: Currently plots C coefficient positions only
- **`coeffExtent` override**: D uses C's extent; per-set option deferred
- **Morph panel trail rendering**: Cosmetic, deferred

---

## Verification

1. Open D-List tab -- see all D-nodes with labels d0, d1, ... and "none" paths
2. Select D-nodes, assign circle/spiral paths -- verify curves generated, path columns update
3. Play -- D-nodes animate along paths, morph blending produces moving target (morph is always active, no checkbox needed)
4. Scrub slider (header bar) -- D-nodes move along paths in sync with C
5. Home button -- D-nodes reset to curve[0]
6. Fast mode (bitmap) with animated D-nodes -- verify pixels accumulate correctly
7. Save/Load roundtrip -- D-node paths preserved (including follow-c), C-D path type and dither settings restored
8. Backward compat: load old snap -- D-nodes default to "none" paths, morph defaults to line path
9. Assign "Follow C" to a D-node -- verify it mirrors the C-node position during animation and in fast mode
10. Verify D-List transforms skip follow-c nodes (e.g., PrimeSpeeds, SetAllSpeeds)
11. Right-click D-node in morph panel -- context menu opens with path controls, live preview, accept/revert
12. WASM step loop with animated D-nodes -- verify D-curves interpolated in C, no JS boundary crossing
13. C-D path types: test line, circle, ellipse, figure-8 in both JS workers and WASM step loop
14. C-D path dither: verify start/mid/end sigma controls produce noise at expected morph phases
15. Dither distribution: assign dithered paths to D-nodes, verify normal vs uniform distribution in both main thread and fast mode
16. Run existing test suite: `python -m pytest tests/ -v`
