# D-List Tab with Path Animation for D-Nodes

## Status: IMPLEMENTED

All phases complete. D-nodes (morph targets) can now be assigned paths and animate along them.

## Context
D-nodes (morph targets in `morphTargetCoeffs[]`) have the full coefficient data structure including `pathType`, `radius`, `speed`, `angle`, `ccw`, `extra`, `curve`, `curveIndex`. Previously all initialized to `pathType: "none"`. Now a "D-List" tab mirrors the C-List tab so D-nodes can be assigned paths (circles, spirals, etc.) and animate.

**File:** `index.html` (single file, all changes here)

### Design Decisions
- **Separate C-List and D-List tabs** (not a single tab with C/D toggle). Clearer UX.
- **D radius uses C's `coeffExtent()`** — shared coordinate space. Morph blends C↔D in the same unit system.
- **`bitmapCoeffView` unchanged** — currently plots C coefficient positions only. Adding a C/D toggle is deferred.
- **No jiggle offsets for D-nodes** — jiggle only applies to C-coefficients.

---

## Phase 1: D-List Tab HTML + Tab Switching ✓

- Tab button: `<button class="tab" data-ltab="dlist">D-List</button>` after C-List button
- Full `#dlist-content` panel mirroring `#list-content` with `dlist-`/`dle-` prefixes
- `dpath-pick-pop` popup element
- Tab switching: `dlist` in `leftTabContents`, calls `refreshDCoeffList()` and `refreshDListCurveEditor()`
- `dle-path-sel` initialized with `buildPathSelect()`

---

## Phase 2: D-List JavaScript Functions ✓

All C-List functions mirrored for `morphTargetCoeffs[]` and `selectedMorphCoeffs`:

- **Path picker popup**: `openDPathPickPop()`, `closeDPathPickPop()` with live preview and PS button
- **Core list**: `refreshDCoeffList()`, `updateDListCoords()`, `updateDListPathCols()`
- **Curve cycling**: `buildDCurveCycleTypes()`, `updateDCurveCycleLabel()`, `selectByDCurveType()`
- **Curve editor**: `refreshDListCurveEditor()`, `buildDleControls()`, `dleReadParams()`, `dleApplyToCoeff()`, `dleSyncToCurrentCoeff()`
- **Transform dropdown**: All 22 transforms targeting `morphTargetCoeffs` + `selectedMorphCoeffs`
- After D transforms: calls `solveRootsThrottled()` if `morphEnabled`, `renderMorphPanel()` if on morph tab

---

## Phase 3: D-Node Animation — All 5 Entry Points ✓

Helper functions:
- `allAnimatedDCoeffs()` — returns Set of D-node indices with `pathType !== "none"`
- `advanceDNodesAlongCurves(elapsed)` — advances all animated D-nodes along their curves
- `updateMorphPanelDDots()` — updates D-dot positions, labels, and interp line endpoints in morph panel SVG

### Entry Points:
1. **`animLoop()`** — `advanceDNodesAlongCurves(elapsed)` after C-coefficient loop, `updateMorphPanelDDots()` in morph panel block, `updateDListCoords()` at end
2. **`startAnimation()`** — snap animated D-nodes to `curve[0]` on fresh start; allow animation if animated D-nodes exist (not just C)
3. **Scrub slider** — `advanceDNodesAlongCurves(elapsed)` after C-loop, `updateMorphPanelDDots()`, `updateDListCoords()`
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
- `morphTargetRe/Im` still contains home positions for non-animated D-nodes

### Worker blob
- **Persistent state**: `S_dCurvesFlat`, `S_dEntries`, `S_dOffsets`, `S_dLengths`, `S_dIsCloud`
- **Init handler**: Parses D-curve data from init message
- **Run handler**: Pre-allocates `morphRe`/`morphIm` as copies of `S_morphTargetRe`/`S_morphTargetIm`. Each step:
  1. Advance C-curves → overwrite `coeffsRe`/`coeffsIm`
  2. Advance D-curves → overwrite animated D indices in `morphRe`/`morphIm`
  3. Morph blend: `coeffs = C*(1-mu) + morphRe/Im*mu`

### Legacy fallback (fastModeChunkLegacy)
- Advances D-nodes along `fastModeDCurves` each step
- Morph blending creates `coeffsToSolve` with animated C+D positions

### Cleanup
- `exitFastMode()`: nulls `fastModeDCurves`
- `reinitWorkersForJiggle()`: recomputes `fastModeDCurves`
- Legacy jiggle cycle: recomputes `fastModeDCurves`

---

## Phase 5: Save/Load ✓

### buildStateMetadata()
D-node serialization extended from `{ pos }` to `{ pos, home, pathType, radius, speed, angle, ccw, extra }`.

### applyLoadedState()
Restores path fields with backward compat: `d.pathType || "none"`, `d.radius ?? 25`, etc. Regenerates curves via `computeCurve()` for non-"none" paths.

**Backward compat**: Old saves with only `{pos}` load as `pathType: "none"`, `home = pos`, single-point curve.

---

## Out of Scope (deferred)
- **`bitmapCoeffView` D toggle**: Currently plots C coefficient positions only
- **Ops tools (Scale/Rotate/Translate) for D-nodes**: Heavy, deferred
- **`coeffExtent` override**: D uses C's extent; per-set option deferred
- **Morph panel trail rendering**: Cosmetic, deferred

---

## Verification
1. Open D-List tab → see all D-nodes with labels d₀, d₁, ... and "none" paths
2. Select D-nodes, assign circle/spiral paths → verify curves generated, path columns update
3. Enable morph + Play → D-nodes animate along paths, morph blending produces moving target
4. Scrub slider → D-nodes move along paths in sync with C
5. Home button → D-nodes reset to curve[0]
6. Fast mode (bitmap) with animated D-nodes → verify pixels accumulate correctly
7. Save/Load roundtrip → D-node paths preserved
8. Backward compat: load old snap → D-nodes default to "none" paths
9. Run existing test suite: `python -m pytest tests/ -v`
