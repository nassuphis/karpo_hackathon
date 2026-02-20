# Test Results

**775 tests total: 774 passed, 1 skipped** | Runtime: ~12m | Headless Chromium on Apple Silicon

Run with: `python -m pytest tests/ -v`

---

## test_solver.py — Ehrlich-Aberth Solver Correctness (10 tests)

Tests `solveRootsEA()` — pure function, no global state needed.

| Test | Status | What it checks |
|------|--------|----------------|
| `TestDegree1::test_z_plus_1` | PASS | z + 1 = 0 → root at -1 |
| `TestDegree1::test_2z_minus_3` | PASS | 2z - 3 = 0 → root at 1.5 |
| `TestDegree2::test_z2_minus_1` | PASS | z² - 1 = 0 → roots at ±1 |
| `TestDegree2::test_z2_plus_1` | PASS | z² + 1 = 0 → roots at ±i |
| `TestDegree2::test_z2_minus_4z_plus_4` | PASS | z² - 4z + 4 = 0 → double root at 2 |
| `TestDegree3::test_cube_roots_of_unity` | PASS | z³ - 1 = 0 → three cube roots of unity |
| `TestHigherDegree::test_degree_5_roots_of_unity` | PASS | z⁵ - 1 = 0 → five 5th roots of unity |
| `TestWarmStart::test_warm_start_converges` | PASS | Warm start near roots converges quickly |
| `TestNaNResilience::test_leading_zeros` | PASS | Leading near-zero coefficients stripped correctly |
| `TestNaNResilience::test_always_returns_degree_roots` | PASS | z¹⁰ + 1: always returns exactly 10 finite roots |

---

## test_matching.py — Root Tracking (7 tests)

Tests `matchRootOrder()` — greedy nearest-neighbor matching to preserve root identity across frames.

| Test | Status | What it checks |
|------|--------|----------------|
| `TestIdentity::test_same_order` | PASS | Same roots in same order → unchanged |
| `TestPermuted::test_shuffled_back` | PASS | Shuffled roots → reordered to match original |
| `TestPermuted::test_reversed` | PASS | Reversed roots → reordered correctly |
| `TestCloseRoots::test_slight_movement` | PASS | Roots moved slightly → tracked to nearest old root |
| `TestEdgeCases::test_mismatched_lengths` | PASS | Different array sizes → returns newRoots unchanged |
| `TestEdgeCases::test_empty_old` | PASS | Empty oldRoots → returns newRoots unchanged |
| `TestEdgeCases::test_single_root` | PASS | Single root → matched correctly |

---

## test_curves.py — Animation Curve Generation (13 tests)

Tests `computeCurveN()` — generates animation paths for coefficients.

| Test | Status | What it checks |
|------|--------|----------------|
| `TestCirclePath::test_circle_starts_at_home` | PASS | First point is at home position |
| `TestCirclePath::test_circle_max_distance` | PASS | Max distance from home ≈ 2×radius |
| `TestCirclePath::test_circle_full_loop` | PASS | First and last points nearly coincide |
| `TestEllipsePath::test_ellipse_points` | PASS | c-ellipse points show eccentricity (varying distances) |
| `TestEllipsePath::test_ellipse_passes_through_home_and_origin` | PASS | Ellipse passes through both home and origin |
| `TestNonePath::test_none_single_point` | PASS | "none" path → single point at home |
| `TestPointCount::test_correct_count_circle[1,10,100,500]` | PASS ×4 | Circle returns exactly N points |
| `TestPointCount::test_correct_count_spiral[1,10,100]` | PASS ×3 | Spiral returns exactly N points |

---

## test_paths.py — Path Parametrics (32 tests)

Tests parametric path functions: circle, horizontal/vertical, lissajous, cardioid, figure-8, astroid, deltoid, rose, epitrochoid, hypotrochoid, butterfly, star, square, and space-filling curves (Moore, Peano, Sierpinski).

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestCirclePath` | 4 | t=0 on positive x-axis, quarter turn, full cycle return, respects center |
| `TestHorizontalVertical` | 2 | Horizontal stays on y, vertical stays on x |
| `TestLissajous` | 1 | Bounded by radius |
| `TestCardioid` | 1 | Passes through center |
| `TestParametricPaths::test_bounded` | 9 | figure8, astroid, deltoid, rose, epitrochoid, hypotrochoid, butterfly, star, square all stay bounded |
| `TestParametricPaths::test_periodic` | 7 | circle, figure8, astroid, deltoid, epitrochoid, star, square return to start |
| `TestParametricPaths::test_hypotrochoid_periodic_2` | 1 | Hypotrochoid period with k=2 |
| `TestSpaceFillingCurves` | 7 | Moore/Peano/Sierpinski: bounded, adjacent-connected, Moore wraps around |

---

## test_polynomial.py — Polynomial Operations (14 tests)

Tests `rootsToCoefficients()` (Vieta's formulas) and `rankNormalize()`.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestRootsToCoefficients` | 6 | Single root, two real roots, complex conjugates, roundtrip real/complex, degree-5 roundtrip |
| `TestRankNormalize` | 8 | Basic ordering, reversed, ties, single, all-same, non-finite replacement, empty, all-non-finite |

---

## test_shapes.py — Root Shape Patterns (45 tests)

Tests the 13 root shape generators (heart, circle, star, spiral, cross, diamond, grid, smiley, figure8, butterfly, trefoil, polygon, infinity).

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestShapeCount::test_correct_count` | 13 | Each shape returns exactly `degree` points |
| `TestShapeCount::test_count_large_degree` | 13 | Shapes scale to large degree (29) |
| `TestShapeFinite::test_all_finite` | 13 | All generated coordinates are finite |
| `TestCircleShape` | 2 | Points on unit circle, equally spaced |
| `TestStarShape` | 1 | Alternating inner/outer radii |
| `TestGridShape` | 1 | Checkerboard within expected spread |
| `TestDiamondShape` | 1 | L1 norm ≤ expected radius |
| `TestSpiralShape` | 1 | Increasing radius from center |

---

## test_colors.py — Color Functions (14 tests)

Tests HSL→RGB conversion, sensitivity coloring (derivative mode), and proximity coloring.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestHslToRgb` | 7 | Red, green, blue, white, black, gray, yellow |
| `TestSensitivityColor` | 4 | 0→blue, 0.5→white, 1→red, 0.25→mid-blue |
| `TestProximityColoring` | 3 | Equidistant roots, clustered/isolated, palette exists |

---

## test_stats.py — Statistics Functions (16 tests)

Tests `computeMinDist()`, `computeMaxDist()`, `computeMeanDist()`, and `percentileSorted()`.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestComputeMinDist` | 4 | Two roots, three roots, single root, coincident roots |
| `TestComputeMaxDist` | 3 | Two roots, three roots, single root |
| `TestComputeMeanDist` | 3 | Two roots, three equidistant, collinear |
| `TestPercentileSorted` | 6 | Median, min, max, interpolation, empty, single |

---

## test_utils.py — Utility Functions (29 tests)

Tests subscript/superscript formatting, path catalog consistency, prime speed finder, audio helpers (clamp, frac01, midiToHz), scale quantization, and range computation.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestSubscript` | 3 | Single digit, multi-digit, all digits 0–9 |
| `TestSuperscript` | 3 | Single digit, multi-digit, special digits (2,3) |
| `TestPathCatalog` | 4 | Catalog↔params consistency, no duplicates, has groups |
| `TestFindPrimeSpeed` | 3 | Coprime with empty, avoids common factor, returns integer |
| `TestAudioHelpers` | 9 | clamp (within/below/above), frac01 (pos/neg/zero), midiToHz (A4/A5/C4) |
| `TestQuantizeToScale` | 3 | On-scale, off-scale, octave wrapping |
| `TestComputeRange` | 4 | Basic, array format, non-finite ignored, empty |

---

## test_state.py — Save/Load Roundtrip (8 tests)

Tests `buildStateMetadata()` → `applyLoadedState()` for serialization fidelity.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestSaveLoadRoundtrip` | 7 | Basic roundtrip, coefficients, roots, jiggle params, panel ranges, color mode, uniform color |
| `TestLoadPartialState` | 1 | Minimal state (missing fields) loads without crash |

---

## test_morph.py — Coefficient Morphing (13 tests)

Tests `initMorphTarget()`, blending formula, save/load, degree sync, and fast mode serialization. Morph is always enabled (no enable/disable toggle).

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestMorphInit` | 3 | Target created on init, positions copied C→D, curves are 1-point |
| `TestMorphBlending` | 4 | mu=0 (no blend), mu=1 (full morph), mu=0.5 (midpoint), sinusoidal range [0,1] |
| `TestMorphSaveLoad` | 3 | Full roundtrip, missing morph fallback, load preserves mu |
| `TestMorphDegreeSync` | 1 | Degree change reinitializes D |
| `TestMorphFastMode` | 2 | Serialize includes morph when enabled, serialize always includes morph (morph always enabled) |

---

## test_jiggle.py — Jiggle Perturbation System (61 tests)

Tests `gaussRand()`, `jiggleTargetCoeffs()`, `computeJiggleCentroid()`, `computeJiggleForStep()` across all 11 modes, pure function behavior, caching, save/load, `nearestPrime()`, and step/period -1/+1/P button controls.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestJiggleHelpers` | 3 | gaussRand distribution (mean≈0, variance≈1), target coeffs = selected, centroid = center of mass |
| `TestJiggleModes::test_mode_produces_offsets` | 11 | Each of 11 modes (random, rotate, walk, scale-center, scale-centroid, circle, spiral-centroid, spiral-center, breathe, wobble, lissajous) produces offsets |
| `TestJiggleModes::test_mode_offsets_are_finite` | 11 | All offsets from each mode are finite |
| `TestJiggleModes::test_none_mode_no_offsets` | 1 | "none" mode produces no offsets |
| `TestJigglePureFunction` | 5 | Rotate/scale/breathe are pure functions of step (no accumulation), same step cached, jiggleStepFromElapsed floors correctly |
| `TestJiggleSaveLoad` | 1 | Mode + parameters survive save/load |
| `TestNearestPrime` | 5 | Prime returns itself, composite returns nearest, small values, result always prime, distance is minimal |
| `TestStepButtons::test_buttons_exist` | 7 | Each step/period mode (rotate, circle, spiral-centroid, wobble, lissajous, breathe, wobble) has -1, +1, P buttons |
| `TestStepButtons` (non-parametrized) | 17 | Rotate +1/-1/P/P-noop, circle +1/P, spiral +1, lissajous +1/P, min/max clamp, breathe period +1/P/min-clamp, slider sync, wobble dual rows, P button title |

---

## test_fastmode.py — Continuous Fast Mode (22 tests)

Tests `fmtPassCount()`, button labels, removed variables, `initBitmapCanvas()` resets, `serializeFastModeData()`, clear behavior, and toggle handler.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestFmtPassCount` | 5 | Zero elapsed, small, large, fractional rounds, padded width stable (7 chars) |
| `TestButtonLabels` | 4 | Init button says "init", fast button says "start" (disabled), no progress bar in DOM, pass count element exists hidden |
| `TestJiggleInterval` | 3 | Default=4, save/load roundtrip (wire key: targetSeconds), old snap compat |
| `TestFastModeVariablesRemoved` | 3 | `fastModeCumulativeSec`, `fastModeTargetSeconds`, `fastModeShowProgress` all undefined |
| `TestInitBitmapCanvasResets` | 3 | Init resets elapsed to 0, pass count to 0, worker roots to null |
| `TestSerializeFastModeData` | 2 | Basic structure (coeffs, roots, steps), jiggle offsets baked into non-animated coeffs |
| `TestClearButtonNoElapsedReset` | 1 | Clear preserves elapsed offset |
| `TestFastModeToggle` | 1 | Toggle handler doesn't reference removed variables |

---

## test_offcanvas.py — Off-Canvas Render, Image Export, Color & Derivative (76 tests)

Tests the split compute/display architecture, multi-format export, high-resolution support, decoupled bitmap/animation color modes, and derivative sensitivity coloring pipeline.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestStateVariables` | 4 | BITMAP_DISPLAY_CAP=2000, initial zeros/nulls for compute/display state |
| `TestResolutionDropdown` | 2 | All 7 options (1K–25K) present, default is 2000 |
| `TestInitNoSplit` | 2 | At 1000/2000px: no display buffer, canvas = compute res |
| `TestInitWithSplit` | 2 | At 5000px: canvas=2000, persistent=5000, display buffer=2000 |
| `TestSerializeComputeRes` | 2 | Workers get computeRes (not canvas.width) in canvasW/canvasH |
| `TestEnterFastModeResCheck` | 1 | Resolution check compares bitmapComputeRes, not canvas.width |
| `TestCompositeWorkerPixels` | 2 | No-split: writes to persistent buffer; split: writes to both + downsamples |
| `TestFillDisplayBuffer` | 1 | Fills entire display buffer with correct color |
| `TestResetBitmap` | 1 | Nulls all new state (persistent, display, computeRes, displayRes) |
| `TestClearWithSplit` | 1 | Clear resets both persistent and display buffers |
| `TestBMPExport` | 4 | Function exists, valid BMP header, BGR pixel order, bottom-up row order |
| `TestPlotCoeffCurvesDisplayRes` | 1 | Uses display resolution for coefficient view |
| `TestTimingCopyRes` | 1 | Shows compute resolution in timing data |
| `TestExportFormatState` | 4 | Default PNG, variable persistence, save/load roundtrip |
| `TestSavePopup` | 5 | Popup exists, initially closed, opens/closes, has format dropdown |
| `TestExportFunctions` | 4 | JPEG/PNG/TIFF/downloadBlob functions exist |
| `TestLibraryLoading` | 4 | pako, UPNG, jpeg-js, UTIF globals available |
| `TestJPEGExport` | 1 | Produces valid image/jpeg blob |
| `TestPNGExport` | 1 | Produces valid image/png blob |
| `TestTIFFExport` | 1 | Produces valid image/tiff blob |
| `TestBitmapColorMode` | 8 | Defaults (uniform/white), independence from animation color, save/load roundtrip, backward compat, serialization uses bitmapColorMode/bitmapUniformColor |
| `TestDerivativePalette` | 8 | DERIV_PALETTE 16 entries, blue→red endpoints, flat arrays, serialization flags, subset selection, symmetry, white midpoint |
| `TestRankNormalize` | 7 | Basic ranking, ties, Infinity handling, single element, empty/all-Infinity→null, [0,1] range |
| `TestComputeRootSensitivities` | 4 | No selection→null, valid array with selection, length matches roots, all-selected valid |
| `TestDerivativeSerializationNonDerivMode` | 3 | derivColor=false for rainbow, uniform, proximity modes |
| `TestAnimationColorPicker` | 2 | Only 3 modes (no proximity), 8 fixed swatches from ROOT_COLOR_SWATCHES |

---

## test_match_strategy.py — Root-Matching Strategies (16 tests)

Tests the Hungarian algorithm, greedy strategy options, serialization, save/load, UI chips, and worker blob contents.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestStateVariable` | 1 | Default bitmapMatchStrategy is "assign4" |
| `TestHungarianAlgorithm` | 5 | Identity, reversed roots, 2×2 optimal, valid permutation, single root |
| `TestSerializationStrategy` | 3 | matchStrategy in serialized data for assign4, hungarian1, assign1 |
| `TestSaveLoadRoundtrip` | 2 | Roundtrip preserves strategy, missing field defaults to assign4 |
| `TestUIChips` | 3 | 3 chips in popup, default highlighted, click updates variable |
| `TestWorkerBlobContents` | 2 | Worker blob contains hungarianMatch function and S_matchStrategy variable |

---

## test_dnode.py — D-Node Paths (32 tests)

Tests D-List tab HTML, `allAnimatedDCoeffs()`, `advanceDNodesAlongCurves()`, D-node save/load with paths, D-curve serialization for fast mode, backward compatibility, and jiggle immunity.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestDListTabHTML` | 6 | Tab button, content panel, toolbar elements, scroll container, path picker popup, tab switching |
| `TestDListRefresh` | 2 | Row creation with `cpick-row` class, d-prefix labels |
| `TestAllAnimatedDCoeffs` | 4 | Empty by default, returns animated indices, Set type, ignores "none" |
| `TestAdvanceDNodesAlongCurves` | 6 | None path unchanged, circle moves, elapsed=0 at start, ccw reversal, speed effect, multiple independent |
| `TestDNodeSaveLoad` | 5 | Path fields saved, circle roundtrip, none roundtrip, home vs pos, mixed paths |
| `TestDNodeBackwardCompat` | 4 | Old snap pos-only, default values, missing morph section, target length mismatch |
| `TestDCurveSerialization` | 3 | Empty when no animated, populated when animated, offsets/lengths consistency |
| `TestDNodeJiggleImmunity` | 1 | Jiggle offsets not applied to D-nodes in serialization |
| `TestExitFastModeCleanup` | 1 | fastModeDCurves nulled on exit |

---

## test_state_fields.py — Extended Save/Load (26 tests)

Tests save/load roundtrip for bitmap settings, solver type, selected coefficients, trail data, jiggle sub-fields, worker count, domain coloring, and backward compatibility for missing fields.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestBitmapFieldsRoundtrip` | 6 | Canvas color, color mode, uniform color, match strategy, export format, coeff view |
| `TestSolverTypeRoundtrip` | 1 | Solver type preserved |
| `TestSelectedCoeffsRoundtrip` | 2 | Non-empty and empty selected coefficients |
| `TestTrailDataRoundtrip` | 1 | Trail data arrays with points |
| `TestJiggleFieldsRoundtrip` | 4 | Scale step, period, amplitude, Lissajous freqs |
| `TestNumWorkersRoundtrip` | 1 | Worker count preserved |
| `TestBackwardCompatMissingFields` | 9 | Missing bitmap color mode, match strategy, export format, solver type, num workers, jiggle, selected coeffs, trail data, canvas color |
| `TestDomainColoringRoundtrip` | 2 | Domain coloring enabled, trails enabled |

---

## test_animation.py — Animation Entry Points (17 tests)

Tests animation state management, play guard (C and D animated nodes), home button reset for C and D nodes, scrub slider advancement, and allAnimatedCoeffs.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestAnimationState` | 4 | Initial not playing, play button exists, home button exists, scrub slider exists |
| `TestStartAnimationGuard` | 3 | No animated = no start, animated C allows start, animated D allows start |
| `TestHomeButton` | 3 | Resets C to curve start, resets D to curve start, resets curveIndex |
| `TestScrubSlider` | 4 | Advances C, advances D, zero returns to start, label updates |
| `TestStopAnimation` | 1 | Sets playing to false |
| `TestAllAnimatedCoeffs` | 2 | Empty when no paths, returns animated indices |

---

## test_anim_bar.py — Trajectory Editor Simplification (29 tests)

Tests anim-bar preview/revert/commit pattern, PS button removal, Update Whole Selection naming, C-List and D-List curve editor button/cycler removal, and editor functionality.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestAnimBarButtons` | 2 | Update button text, no PS button in bar |
| `TestAnimBarPreview` | 3 | Snapshot creation, path change during preview, curve preservation in snapshot |
| `TestAnimBarRevert` | 4 | Path restoration, snapshot clearing, selection change revert, noop when no preview |
| `TestAnimBarCommit` | 2 | Keep new path on commit, Update button commits preview |
| `TestCListEditorButtons` | 7 | No PS, no Update This, no prev/next cycler, no coeff name label, has Update Whole Selection, has path dropdown |
| `TestDListEditorButtons` | 7 | No PS, no Update This, no prev/next cycler, no coeff name label, has Update Whole Selection, has path dropdown |
| `TestCListEditorFunction` | 2 | Update Whole Selection applies to all selected, editor shows first selected path |
| `TestDListEditorFunction` | 2 | Update Whole Selection applies to all selected D-nodes, editor shows first selected path |

---

## test_final_tab.py — Final Tab (20 tests)

Tests Final tab DOM, rendering, trail data accumulation, trail SVG paths, and integration with solveRoots and animation.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestFinalTabExists` | 4 | Tab button, content div, panel SVG, tab switching |
| `TestFinalPanelRendering` | 4 | Dot count, labels, C-node positions without morph, blended positions with morph |
| `TestFinalTrailData` | 4 | Array declared, empty initially, accumulates during animation, cleared on clearTrails |
| `TestFinalTrailRendering` | 3 | Correct panel key (panels.coeff), renders trail paths, paths have valid d attribute |
| `TestSolveRootsWithFinalTab` | 3 | solveRoots succeeds with Final tab, roots valid, pendingSolve resets |
| `TestAnimationWithFinalTrails` | 2 | Roots move during animation, final dots update during solve |

---

## test_dnode_ctx.py — D-Node Context Menu (16 tests)

Tests D-node right-click context menu: popup existence, open/close, snapshot/revert, accept button, path changes, and morph panel integration.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestDNodeCtxPopupExists` | 2 | Popup div exists, initially hidden |
| `TestOpenDNodeCtx` | 5 | Sets ctxDNodeIdx, shows popup, creates path select, d-prefix title, snapshots state |
| `TestCloseDNodeCtx` | 3 | Revert restores path, no-revert keeps changes, resets idx to -1 |
| `TestDNodeCtxAccept` | 3 | Accept button exists, no Delete button, no PS button |
| `TestDNodeCtxPathChange` | 2 | Change path via select, revert undoes path change |
| `TestDNodeCtxOnMorphPanel` | 1 | Right-click morph dot opens dnode-ctx |

---

## test_wasm_step_loop.py — WASM Step Loop (26 tests)

Tests the WASM step loop module: loading, memory layout, solver correctness, color modes, fast mode end-to-end, WASM vs JS comparison, worker integration, progress reporting, and edge cases.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestWasmModuleLoading` | 3 | Module instantiation, __heap_base exported, heap_base above stack |
| `TestMemoryLayout` | 4 | Starts at heap_base, no BSS overlap, 8-byte alignment, no section overlap |
| `TestWasmStepLoopInit` | 1 | Init reads config, produces correct pixel count and uniform color |
| `TestWasmSolverCorrectness` | 3 | Roots of unity degree 3, degree 5, z^2+1 roots match known values |
| `TestWasmFastModeEndToEnd` | 2 | WASM fast mode produces pixels, JS fast mode produces pixels (control) |
| `TestWasmVsJsComparison` | 1 | WASM and JS produce similar pixel counts for same polynomial |
| `TestWasmColorModes` | 3 | Uniform color output, index-rainbow per-root colors, uniform vs rainbow differ |
| `TestWorkerWasmIntegration` | 3 | WASM_STEP_LOOP_B64 present, serialized data has curve counts, error message handled |
| `TestWasmProgressReporting` | 1 | Progress callback fires during large step count |
| `TestWasmEdgeCases` | 4 | Degree-2 polynomial, zero steps, large step count, pixel indices within canvas |
| `TestWasmVsJsSolverDirect` | 1 | WASM and JS solvers find same roots for z^3 - 1 |

---

## test_shape_morph.py — Shape Morph Tool (30 tests)

Tests the shape morph tool: `rayPolyHit()` ray-polygon intersection, `shapeTargets()` boundary projection for 4 shape types (box, triangle, pentagon, infinity), and morph interpolation at slider positions 0, 0.25, 0.5, and 1.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestRayPolyHit` | 6 | Rightward ray hits triangle, upward ray hits triangle top, ray hits pentagon at correct distance, all 12 directions hit triangle, all 20 directions hit pentagon, rightward ray hits box |
| `TestShapeTargets::test_all_targets_non_null` | 4 | Box/tri/pent/inf: every item gets a valid (non-fallback) target |
| `TestShapeTargets` (specific shapes) | 4 | Box targets on bbox edges, tri targets on triangle edges, pent targets on pentagon edges, inf targets at R*(0.2+0.8*cos²θ) distance |
| `TestShapeMorphInterpolation::test_slider_zero` | 4 | Box/tri/inf/pent: u=0 collapses all points to centroid |
| `TestShapeMorphInterpolation::test_slider_half` | 4 | Box/tri/inf/pent: u=0.5 returns points to original positions |
| `TestShapeMorphInterpolation::test_slider_one` | 4 | Box/tri/inf/pent: u=1 places points at shape boundary targets |
| `TestShapeMorphInterpolation::test_slider_quarter` | 4 | Box/tri/inf/pent: u=0.25 places points halfway between centroid and original |

---

## test_pattern_arrange.py — Pattern Arrange Tool (193 tests)

Tests the pattern arrange tool: `distributeOnPath()` helper, `patternPositions()` for all 21 patterns, per-pattern geometry validation, blend interpolation, centroid preservation, large-N scaling, parameterized options, and UI popup behavior.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestDistributeOnPath` | 6 | Closed triangle 3 pts, closed square 4 pts, closed square 8 pts with midpoints, open path endpoints, single point, equal spacing on hexagon |
| `TestPatternPositionsCount` | 63 | All 21 patterns: returns n points (n=10), returns 1 point at center, returns 2 points |
| `TestPatternPositionsFinite` | 21 | All 21 patterns: every point is finite |
| `TestCircle` | 2 | All points at correct radius, equally spaced angles |
| `TestSquare` | 1 | Points on square perimeter |
| `TestTriangle` | 1 | Points within radius R |
| `TestPentagon` | 1 | Points within radius R |
| `TestHexagon` | 1 | 6 points at regular hexagon vertices |
| `TestDiamond` | 1 | First point at top vertex |
| `TestStar` | 1 | Alternating inner/outer radii |
| `TestEllipse` | 1 | Points satisfy ellipse equation |
| `TestInfinity` | 1 | Points on both sides of x-axis |
| `TestSpiral` | 2 | Starts near center/ends near R, roughly increasing radius |
| `TestGrid` | 2 | Points within bounding box, 9 points form 3x3 grid |
| `TestLine` | 2 | Constant imaginary part, spans cRe-R to cRe+R |
| `TestWave` | 2 | Horizontal span correct, y-values vary |
| `TestCross` | 2 | Within bounds, points in all 4 cardinal directions |
| `TestHeart` | 1 | Wider at top than bottom |
| `TestLissajous` | 2 | Within radius, crosses near origin |
| `TestRose` | 2 | Within radius, has petals (passes through origin) |
| `TestTwoCircles` | 1 | Points split into left/right groups |
| `TestTwoSquares` | 1 | Points form two separate groups |
| `TestRing` | 1 | Points at two distinct radii |
| `TestScatter` | 2 | Within radius, fills area at various distances |
| `TestPatternBlend` | 13 | 5 patterns: u=0 gives original, 5 patterns: u=1 gives target, 3 patterns: u=0.5 is midpoint |
| `TestPatternCentroid` | 12 | 12 symmetric patterns: output centroid near input centroid |
| `TestPatternUIExists` | 2 | Ptrn button exists, PATTERN_LIST has 21 entries |
| `TestPatternLargeN` | 21 | All 21 patterns: handle 50 points without error |
| `TestPatternOpts` | 15 | Star inner radius, ellipse aspect, infinity amplitude, spiral turns, grid cols, line angle, wave cycles/amplitude, cross arm width, lissajous freq, rose petals, 2-circles/2-squares distance, ring inner ratio, default opts match |
| `TestPatternToolUI` | 8 | PTRN_PARAMS has 13 entries, accept keeps changes, close reverts, no morph slider, has accept button, select dropdown with 21 options, star shows Inner R, circle no controls, lissajous shows 2 freq sliders, pattern switch resets opts |

---

## test_integration.py — End-to-End (3 tests)

| Test | Status | What it checks |
|------|--------|----------------|
| `TestLoadSnap::test_load_bug1` | PASS | Load bug1.json snap → degree matches (22) |
| `TestSolverDeterminism::test_repeated_solve_identical` | PASS | Same polynomial solved 10× → bit-identical results |
| `TestFastModePixels::test_fast_mode_produces_pixels` | SKIP | Requires bitmap tab active in headless Chromium |

---

## test_benchmark.py — JS vs WASM Performance (7 tests)

Tests correctness and speed of the WASM step loop (`step_loop.c` → `step_loop.wasm`) and full step loop (solver + pixel output) benchmarks.

| Test | Status | What it checks |
|------|--------|----------------|
| `test_benchmark_js` | PASS | JS solver completes at all degrees without hanging |
| `test_benchmark_wasm` | PASS | WASM solver completes at all degrees without hanging |
| `test_benchmark_comparison` | PASS | Side-by-side JS vs WASM timing comparison |
| `test_wasm_correctness` | PASS | WASM finds same 5th roots of unity as JS |
| `test_step_loop_benchmark_js` | PASS | JS step loop (solver + pixel gen) at various degrees/step counts |
| `test_step_loop_benchmark_wasm` | PASS | WASM step loop at various degrees/step counts |
| `test_step_loop_benchmark_comparison` | PASS | Side-by-side JS vs WASM step loop speedup comparison |

### Benchmark Results (2026-02-13)

Headless Chromium, Apple Silicon Mac. Each degree run includes 100-200 JIT/WASM warmup calls before timing.

| Degree | Calls | JS us/call | WASM us/call | JS/WASM |
|--------|-------|-----------|-------------|---------|
| 5 | 10,000 | 0.31 | 0.23 | **1.35x** |
| 10 | 10,000 | 1.56 | 0.32 | **4.88x** |
| 20 | 10,000 | 2.55 | 1.26 | **2.02x** |
| 50 | 2,000 | 11.20 | 9.25 | **1.21x** |
| 100 | 500 | 42.20 | 38.00 | **1.11x** |

**Analysis**:
- At low degrees (5-20), WASM wins by 1.3-5x — likely JIT warmup overhead and allocation cost
- At high degrees (50-100), WASM advantage shrinks to ~1.1-1.2x — V8's JIT fully optimizes the hot loop
- For the real workload (degree 29 in fast mode with 16 workers), expect ~1.5-2x improvement, plus elimination of GC jitter

---

## Coverage Summary

| Area | Test File | Tests | What's covered |
|------|-----------|-------|----------------|
| Solver | test_solver.py | 10 | Ehrlich-Aberth: degree 1-10, warm start, NaN resilience |
| Root tracking | test_matching.py | 7 | Greedy matching: identity, permutations, close roots, edge cases |
| Curves | test_curves.py | 13 | Circle, ellipse, none, point counts |
| Paths | test_paths.py | 32 | 21 path types: parametric bounds, periodicity, space-filling connectivity |
| Polynomials | test_polynomial.py | 14 | Vieta's formulas, rank normalization |
| Shapes | test_shapes.py | 45 | 13 root patterns: counts, scaling, finiteness, geometry |
| Colors | test_colors.py | 14 | HSL→RGB, sensitivity, proximity |
| Statistics | test_stats.py | 16 | Min/max/mean distance, percentiles |
| Utilities | test_utils.py | 29 | Formatting, path catalog, prime speeds, audio helpers, ranges |
| Save/load | test_state.py | 8 | Roundtrip serialization, partial state |
| Morph | test_morph.py | 13 | Init, blending, save/load, degree sync, fast mode (morph always enabled) |
| Jiggle | test_jiggle.py | 61 | Helpers, all 11 modes, pure function behavior, caching, save/load, nearestPrime, step/period buttons |
| Fast mode | test_fastmode.py | 22 | Formatting, buttons, removed vars, resets, serialization, clear, toggle |
| Off-canvas & export | test_offcanvas.py | 76 | Split compute/display, BMP/JPEG/PNG/TIFF export, library loading, save popup, format state, bitmap/animation color decoupling, derivative palette, rank normalization, root sensitivities |
| Match strategies | test_match_strategy.py | 16 | Hungarian algorithm, greedy strategies, serialization, save/load, UI chips, worker blob |
| D-node paths | test_dnode.py | 32 | D-List tab, allAnimatedDCoeffs, advanceDNodesAlongCurves, save/load with paths, D-curve serialization, backward compat, jiggle immunity |
| Extended state | test_state_fields.py | 26 | Bitmap settings, solver type, selected coeffs, trails, jiggle sub-fields, worker count, backward compat for missing fields |
| Animation | test_animation.py | 17 | Start/stop/home/scrub, play guard with D-nodes, allAnimatedCoeffs |
| Trajectory editors | test_anim_bar.py | 29 | Preview/revert/commit, PS removal, cycler removal, Update Whole Selection, C-List/D-List editors |
| Final tab | test_final_tab.py | 20 | Tab DOM, rendering, trail data, trail SVG, solveRoots integration, animation |
| D-node ctx menu | test_dnode_ctx.py | 16 | Popup, open/close, snapshot/revert, accept, path changes, morph panel |
| WASM step loop | test_wasm_step_loop.py | 26 | Module loading, memory layout, solver correctness, color modes, fast mode E2E, JS comparison, progress, edge cases |
| Shape morph | test_shape_morph.py | 30 | Ray-polygon intersection, shape boundary targets (box/tri/pent/inf), morph slider interpolation |
| Pattern arrange | test_pattern_arrange.py | 193 | 21 patterns: distributeOnPath, count/finite/centroid, per-shape geometry, blend interpolation, large-N, opts, UI popup |
| Integration | test_integration.py | 3 | Snap loading, determinism, fast mode pixels |
| Benchmark | test_benchmark.py | 7 | JS vs WASM: solver correctness + performance, step loop benchmarks |
| **Total** | **26 files** | **775** | |

---

## Architecture

All tests use **Playwright Python** running headless Chromium. Tests call functions in the actual `index.html` page via `page.evaluate()` — no code extraction or duplication.

```
tests/
  conftest.py              — Fixtures: HTTP server, browser, page, wasm_b64
  test_solver.py           — solveRootsEA correctness (pure function)
  test_matching.py         — matchRootOrder correctness (pure function)
  test_curves.py           — computeCurveN correctness (semi-pure)
  test_paths.py            — Parametric path functions (21 path types)
  test_polynomial.py       — rootsToCoefficients, rankNormalize
  test_shapes.py           — Root shape generators (13 shapes)
  test_colors.py           — HSL→RGB, sensitivity, proximity coloring
  test_stats.py            — Distance stats, percentiles
  test_utils.py            — Formatting, path catalog, audio helpers
  test_state.py            — Save/load roundtrip, backward compat
  test_morph.py            — Coefficient morphing system
  test_jiggle.py           — Jiggle perturbation (11 modes), nearestPrime, step/period buttons
  test_fastmode.py         — Continuous fast mode, buttons, serialization
  test_offcanvas.py        — Off-canvas render split, multi-format export, high-res
  test_match_strategy.py   — Root-matching strategies (Hungarian, greedy, UI)
  test_dnode.py            — D-node paths (D-List tab, animation, save/load, fast mode)
  test_state_fields.py     — Extended save/load roundtrip (bitmap, jiggle, backward compat)
  test_animation.py        — Animation entry points (start, stop, home, scrub, play guard)
  test_anim_bar.py         — Trajectory editor simplification (preview/revert, cycler/PS removal)
  test_final_tab.py        — Final tab (rendering, trails, solveRoots integration)
  test_dnode_ctx.py        — D-node right-click context menu (popup, path editing, morph panel)
  test_wasm_step_loop.py   — WASM step loop (module loading, memory layout, solver, color modes, E2E)
  test_shape_morph.py      — Shape morph tool (rayPolyHit, shapeTargets, slider interpolation)
  test_pattern_arrange.py  — Pattern arrange tool (21 patterns, distributeOnPath, blend, opts, UI)
  test_integration.py      — Snap loading, determinism, fast mode
  test_benchmark.py        — JS vs WASM speed + correctness, step loop benchmarks
```

**Dependencies**: `uv pip install playwright pytest && playwright install chromium`

**WASM build**: `./build-wasm.sh` (requires `brew install lld`)
