# Test Results

**430 tests total: 429 passed, 1 skipped** | Runtime: ~6m | Headless Chromium on Apple Silicon

Run with: `python -m pytest tests/ -v`

---

## test_solver.py — Ehrlich-Aberth Solver Correctness (11 tests)

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

## test_morph.py — Coefficient Morphing (15 tests)

Tests `initMorphTarget()`, blending formula, enable/disable state, save/load, degree sync, and fast mode serialization.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestMorphInit` | 3 | Target created on init, positions copied C→D, curves are 1-point |
| `TestMorphBlending` | 4 | mu=0 (no blend), mu=1 (full morph), mu=0.5 (midpoint), sinusoidal range [0,1] |
| `TestMorphDisabled` | 2 | Disabled → mu stays 0, enabling sets mu=0.5 |
| `TestMorphSaveLoad` | 3 | Full roundtrip, missing morph fallback, disabled forces mu=0 |
| `TestMorphDegreeSync` | 1 | Degree change reinitializes D |
| `TestMorphFastMode` | 2 | Serialize includes morph when enabled, omits when disabled |

---

## test_jiggle.py — Jiggle Perturbation System (26 tests)

Tests `gaussRand()`, `jiggleTargetCoeffs()`, `computeJiggleCentroid()`, `generateJiggleOffsets()` across all 10 modes, cumulative state, and save/load.

| Test Group | Count | What it checks |
|------------|-------|----------------|
| `TestJiggleHelpers` | 3 | gaussRand distribution (mean≈0, variance≈1), target coeffs = selected, centroid = center of mass |
| `TestJiggleModes::test_mode_produces_offsets` | 9 | Each of 9 modes (random, rotate, walk, scale, circle, spiral, breathe, wobble, lissajous) produces offsets |
| `TestJiggleModes::test_mode_offsets_are_finite` | 9 | All offsets from each mode are finite |
| `TestJiggleModes::test_none_mode_no_offsets` | 1 | "none" mode produces no offsets |
| `TestJiggleCumulative` | 3 | Rotate accumulates angle (θ×2π per call), scale accumulates (×(1+step/100)), trigger count increments |
| `TestJiggleSaveLoad` | 1 | Mode + parameters survive save/load |

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

## test_offcanvas.py — Off-Canvas Render, Image Export, Color & Derivative (80 tests)

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
| `TestExportFormatState` | 4 | Default JPEG, variable persistence, save/load roundtrip |
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

## test_dnode.py — D-Node Paths (33 tests)

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
| `TestDListTransform` | 1 | Transform dropdown exists with options |

---

## test_state_fields.py — Extended Save/Load (28 tests)

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

## test_animation.py — Animation Entry Points (16 tests)

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

## test_integration.py — End-to-End (3 tests)

| Test | Status | What it checks |
|------|--------|----------------|
| `TestLoadSnap::test_load_bug1` | PASS | Load bug1.json snap → degree matches (22) |
| `TestSolverDeterminism::test_repeated_solve_identical` | PASS | Same polynomial solved 10× → bit-identical results |
| `TestFastModePixels::test_fast_mode_produces_pixels` | SKIP | Requires bitmap tab active in headless Chromium |

---

## test_benchmark.py — JS vs WASM Performance (4 tests)

Tests correctness and speed of the WASM Ehrlich-Aberth solver (`solver.c` → `solver.wasm`).

| Test | Status | What it checks |
|------|--------|----------------|
| `test_benchmark_js` | PASS | JS solver completes at all degrees without hanging |
| `test_benchmark_wasm` | PASS | WASM solver completes at all degrees without hanging |
| `test_benchmark_comparison` | PASS | Side-by-side JS vs WASM timing comparison |
| `test_wasm_correctness` | PASS | WASM finds same 5th roots of unity as JS |

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
| Morph | test_morph.py | 15 | Init, blending, enable/disable, save/load, degree sync, fast mode |
| Jiggle | test_jiggle.py | 26 | Helpers, all 10 modes, cumulative state, save/load |
| Fast mode | test_fastmode.py | 22 | Formatting, buttons, removed vars, resets, serialization, clear, toggle |
| Off-canvas & export | test_offcanvas.py | 76 | Split compute/display, BMP/JPEG/PNG/TIFF export, library loading, save popup, format state, bitmap/animation color decoupling, derivative palette, rank normalization, root sensitivities |
| Match strategies | test_match_strategy.py | 16 | Hungarian algorithm, greedy strategies, serialization, save/load, UI chips, worker blob |
| D-node paths | test_dnode.py | 33 | D-List tab, allAnimatedDCoeffs, advanceDNodesAlongCurves, save/load with paths, D-curve serialization, backward compat, jiggle immunity |
| Extended state | test_state_fields.py | 28 | Bitmap settings, solver type, selected coeffs, trails, jiggle sub-fields, worker count, backward compat for missing fields |
| Animation | test_animation.py | 16 | Start/stop/home/scrub, play guard with D-nodes, allAnimatedCoeffs |
| Integration | test_integration.py | 3 | Snap loading, determinism, fast mode pixels |
| Benchmark | test_benchmark.py | 4 | JS vs WASM: correctness + performance |
| **Total** | **20 files** | **425** | |

---

## Architecture

All tests use **Playwright Python** running headless Chromium. Tests call functions in the actual `index.html` page via `page.evaluate()` — no code extraction or duplication.

```
tests/
  conftest.py           — Fixtures: HTTP server, browser, page, wasm_b64
  test_solver.py        — solveRootsEA correctness (pure function)
  test_matching.py      — matchRootOrder correctness (pure function)
  test_curves.py        — computeCurveN correctness (semi-pure)
  test_paths.py         — Parametric path functions (21 path types)
  test_polynomial.py    — rootsToCoefficients, rankNormalize
  test_shapes.py        — Root shape generators (13 shapes)
  test_colors.py        — HSL→RGB, sensitivity, proximity coloring
  test_stats.py         — Distance stats, percentiles
  test_utils.py         — Formatting, path catalog, audio helpers
  test_state.py         — Save/load roundtrip, backward compat
  test_morph.py         — Coefficient morphing system
  test_jiggle.py        — Jiggle perturbation (10 modes)
  test_fastmode.py      — Continuous fast mode, buttons, serialization
  test_offcanvas.py     — Off-canvas render split, multi-format export, high-res
  test_match_strategy.py — Root-matching strategies (Hungarian, greedy, UI)
  test_dnode.py         — D-node paths (D-List tab, animation, save/load, fast mode)
  test_state_fields.py  — Extended save/load roundtrip (bitmap, jiggle, backward compat)
  test_animation.py     — Animation entry points (start, stop, home, scrub, play guard)
  test_integration.py   — Snap loading, determinism, fast mode
  test_benchmark.py     — JS vs WASM speed + correctness
```

**Dependencies**: `uv pip install playwright pytest && playwright install chromium`

**WASM build**: `./build-wasm.sh` (requires `brew install lld`)
