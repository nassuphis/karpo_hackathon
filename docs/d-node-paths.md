# D-List Tab with Path Animation for D-Nodes

## Status: IMPLEMENTED

All phases complete. D-nodes (morph targets) can now be assigned paths and animate along them.

## Context
D-nodes (morph targets in `morphTargetCoeffs[]`) have the full coefficient data structure including `pathType`, `radius`, `speed`, `angle`, `ccw`, `extra`, `curve`, `curveIndex`. Previously all initialized to `pathType: "none"`. Now a "D-List" tab mirrors the C-List tab so D-nodes can be assigned paths (circles, spirals, etc.) and animate.

**Files:** `index.html` (main app), `step_loop.c` (WASM step loop)

### Design Decisions
- **Six left-panel tabs**: C-Nodes, C-List, D-Nodes (morph panel, `data-ltab="morph"`), D-List (`data-ltab="dlist"`), Jiggle, Final. The D-List tab mirrors C-List for path editing. D-Nodes is the morph visualization SVG panel.
- **D radius uses C's `coeffExtent()`** — shared coordinate space. Morph blends C↔D in the same unit system.
- **`bitmapCoeffView` unchanged** — currently plots C coefficient positions only. Adding a C/D toggle is deferred.
- **No jiggle offsets for D-nodes** — jiggle only applies to C-coefficients.
- **"Follow C" path type**: D-only path type (`"follow-c"`) that mirrors the corresponding C-node position at every step. Listed in `PATH_CATALOG` with `dOnly: true`, has empty params in `PATH_PARAMS`. `buildPathSelect()` accepts a `dNode` parameter to include D-only options. Treated like `"none"` for curve generation (single-point curve at home). See Phase 3 and Phase 4 for animation details.
- **Speed resolution**: Speed values use 1–1000 integer range internally (stored as `speed * 1000`). The `_P.speed` schema defines `min: 1, max: 1000`, with `toUI: v => Math.round(v * 1000)` and `fromUI: v => v / 1000`. All UI displays use `Math.round(ci.speed * 1000)`.
- **`findDPrimeSpeed()` range**: Searches up to 2000 for coprime speeds (vs `findPrimeSpeed()` which searches up to 1000 for C-nodes).

---

## Phase 1: D-List Tab HTML + Tab Switching ✓

- Tab button: `<button class="tab" data-ltab="dlist">D-List</button>` after D-Nodes button
- Full `#dlist-content` panel mirroring `#list-content` with `dlist-`/`dle-` prefixes
- `dpath-pick-pop` popup element
- Tab switching: `dlist` in `leftTabContents`, calls `refreshDCoeffList()` and `refreshDListCurveEditor()`
- `dle-path-sel` initialized with `buildPathSelect(sel, noneLabel, dNode)` — `dNode=true` includes the "Follow C" option

---

## Phase 2: D-List JavaScript Functions ✓

All C-List functions mirrored for `morphTargetCoeffs[]` and `selectedMorphCoeffs`:

- **Path picker popup**: `openDPathPickPop()`, `closeDPathPickPop()` with live preview and PS button (PS remains in per-coefficient path picker popups only)
- **Core list**: `refreshDCoeffList()`, `updateDListCoords()`, `updateDListPathCols()`. D-List UI shows "Follow C" text for follow-c nodes; speed/radius columns show "—" for `none` and `follow-c` types
- **Curve cycling**: `buildDCurveCycleTypes()`, `updateDCurveCycleLabel()`, `selectByDCurveType()`
- **Curve editor**: `refreshDListCurveEditor()`, `buildDleControls()`, `dleReadParams()`, `dleApplyToCoeff()`. Uses `dleRefIdx` (first selected D-node) as reference for control values. Node cycler (prev/next) and PS button removed; only **Update Whole Selection** remains. `dleApplyToCoeff()` treats `follow-c` like `"none"` for curve generation (single-point curve at home).
- **Prime speed**: `findDPrimeSpeed(currentIntSpeed, excludeSet)` — finds nearest coprime integer speed among D-nodes. Searches up to delta 2000, range 1–2000. Skips `none` and `follow-c` nodes. Analogous to `findPrimeSpeed()` for C-nodes (which searches up to 1000).
- **Transform dropdown**: All 20 transforms targeting `morphTargetCoeffs` + `selectedMorphCoeffs`. All D-List transforms skip `follow-c` nodes (filter `pt !== "none" && pt !== "follow-c"` before applying speed, angle, direction, etc.)
- **Ops tools (Scale/Rotate/Translate)**: Work on D-nodes when `selectedMorphCoeffs` is non-empty. `snapshotSelection()` returns `{ which: "morph", items }`, `applyPreview()` handles the morph case: updates D positions, regenerates curves (follow-c nodes get single-point curve at current position like `"none"`)
- After D transforms: calls `solveRootsThrottled()` if `morphEnabled`, `renderMorphPanel()` if on D-Nodes tab (`leftTab === "morph"`)

---

## Phase 3: D-Node Animation — All 5 Entry Points ✓

Helper functions:
- `allAnimatedDCoeffs()` — returns Set of D-node indices with `pathType !== "none"` and `pathType !== "follow-c"` (follow-c nodes are not considered "animated" for curve purposes)
- `advanceDNodesAlongCurves(elapsed)` — advances all animated D-nodes along their curves. For `follow-c` nodes, copies the corresponding C-node position (`d.re = coefficients[i].re`, `d.im = coefficients[i].im`) before processing other path types
- `updateMorphPanelDDots()` — updates D-dot positions, labels, and interp line endpoints in morph panel SVG

### Entry Points:
1. **`animLoop()`** — `advanceDNodesAlongCurves(elapsed)` after C-coefficient loop, `updateMorphPanelDDots()` in morph panel block, `updateDListCoords()` at end
2. **`startAnimation()`** — snap animated D-nodes to `curve[0]` on fresh start; allow animation if animated D-nodes exist (not just C)
3. **Scrub slider** — additive scrubber in the header bar (`#anim-controls`). Adds seconds to `elapsedAtPause` while paused; resets to 0 on release. Calls `advanceToElapsed(elapsed)` which advances both C-coefficients and D-nodes via `advanceDNodesAlongCurves(elapsed)`, plus `updateMorphPanelDDots()`, `updateDListCoords()`
4. **Home button** — reset all D-nodes to `curve[0]`, `updateMorphPanelDDots()`, `updateMorphMarkers()`
5. **Fast mode workers** — see Phase 4

---

## Phase 4: D-Node Animation — Fast Mode Workers ✓

### State
- `let fastModeDCurves = null;` — Map<dNodeIndex, hi-res curve>

### enterFastMode()
- Computes `fastModeDCurves` for animated D-nodes using `computeCurveN()` with `extentAtHome`

### serializeFastModeData()
- Serializes `dAnimEntries`, `dCurvesFlat` (Float64Array), `dCurveOffsets`, `dCurveLengths`, `dCurveIsCloud`
- Serializes `dFollowCIndices`: array of D-node indices with `pathType === "follow-c"` (collected via `morphTargetCoeffs.reduce()`)
- `morphTargetRe/Im` still contains home positions for non-animated D-nodes

### Worker blob
- **Persistent state**: `S_dCurvesFlat`, `S_dEntries`, `S_dOffsets`, `S_dLengths`, `S_dIsCloud`, `S_dFollowC`
- **Init handler**: Parses D-curve data from init message, including `S_dFollowC = d.dFollowCIndices || []`
- **Run handler**: Pre-allocates `morphRe`/`morphIm` as copies of `S_morphTargetRe`/`S_morphTargetIm`. Each step:
  1. Advance C-curves → overwrite `coeffsRe`/`coeffsIm`
  2. Advance D-curves → overwrite animated D indices in `morphRe`/`morphIm`
  3. Follow-C: copy `coeffsRe[fci]`/`coeffsIm[fci]` → `morphRe[fci]`/`morphIm[fci]` for each index in `dFollowC`
  4. Morph blend: `coeffs = C*(1-mu) + morphRe/Im*mu`

### WASM step loop (initWasmStepLoop / step_loop.c)
- `computeWasmLayout()` includes `nFC` (follow-c count) parameter, allocates `L.fCI` region for Int32Array of follow-c indices
- `initWasmStepLoop()` writes `followC.length` to `cfgI32[10]` and copies indices into `L.fCI`
- `step_loop.c`: `followCIdx` pointer from `CI_OFF_FOLLOWC_IDX` (config slot 36), `nFollowC` from `CI_N_FOLLOWC` (config slot 10)
- Step 5 in the per-step loop: copies `workCoeffsRe[fci]`/`workCoeffsIm[fci]` to `morphWorkRe[fci]`/`morphWorkIm[fci]` for each follow-c index, after D-curve advancement and before morph blend

### Legacy fallback (fastModeChunkLegacy)
- Advances D-nodes along `fastModeDCurves` each step
- Follow-C D-nodes: copies current C-node position (`morphTargetCoeffs[fi].re = coefficients[fi].re`) for each `follow-c` node
- Morph blending creates `coeffsToSolve` with animated C+D positions

### Cleanup
- `exitFastMode()`: nulls `fastModeDCurves`
- `reinitWorkersForJiggle()`: recomputes `fastModeDCurves`
- Legacy jiggle cycle: recomputes `fastModeDCurves`

---

## Phase 5: Save/Load ✓

### buildStateMetadata()
D-node serialization extended from `{ pos }` to `{ pos, home, pathType, radius, speed, angle, ccw, extra }`. `follow-c` nodes are serialized with `pathType: "follow-c"`.

### applyLoadedState()
Restores path fields with backward compat: `d.pathType || "none"`, `d.radius ?? 25`, etc. Regenerates curves via `computeCurve()` for non-`"none"` and non-`"follow-c"` paths. `follow-c` nodes get a single-point curve at home position (same as `"none"`).

**Backward compat**: Old saves with only `{pos}` load as `pathType: "none"`, `home = pos`, single-point curve.

---

## Out of Scope (deferred)
- **`bitmapCoeffView` D toggle**: Currently plots C coefficient positions only
- **`coeffExtent` override**: D uses C's extent; per-set option deferred
- **Morph panel trail rendering**: Cosmetic, deferred

---

## Verification
1. Open D-List tab → see all D-nodes with labels d₀, d₁, ... and "none" paths
2. Select D-nodes, assign circle/spiral paths → verify curves generated, path columns update
3. Enable morph + Play → D-nodes animate along paths, morph blending produces moving target
4. Scrub slider (header bar) → D-nodes move along paths in sync with C
5. Home button → D-nodes reset to curve[0]
6. Fast mode (bitmap) with animated D-nodes → verify pixels accumulate correctly
7. Save/Load roundtrip → D-node paths preserved (including follow-c)
8. Backward compat: load old snap → D-nodes default to "none" paths
9. Assign "Follow C" to a D-node → verify it mirrors the C-node position during animation and in fast mode
10. Verify D-List transforms skip follow-c nodes (e.g., PrimeSpeeds, SetAllSpeeds)
11. Run existing test suite: `python -m pytest tests/ -v`
