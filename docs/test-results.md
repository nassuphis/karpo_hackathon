# Test Results

**259 tests total: 258 passed, 1 skipped** | Runtime: ~3m 25s | Headless Chromium on Apple Silicon

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
| `TestWarmStart::test_warm_start_converges` | PASS | Warm start near roots converges in <10 iterations |
| `TestNaNResilience::test_leading_zeros` | PASS | Leading near-zero coefficients stripped correctly |
| `TestNaNResilience::test_always_returns_degree_roots` | PASS | z¹⁰ + 1: always returns exactly 10 finite roots |
| `TestIterationCounting::test_iter_counts_populated` | PASS | iterCounts array filled with values > 0 |

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
| Solver | test_solver.py | 11 | Ehrlich-Aberth: degree 1-10, warm start, NaN resilience, iteration counts |
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
| Integration | test_integration.py | 3 | Snap loading, determinism, fast mode pixels |
| Benchmark | test_benchmark.py | 4 | JS vs WASM: correctness + performance |
| **Total** | **15 files** | **259** | |

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
  test_integration.py   — Snap loading, determinism, fast mode
  test_benchmark.py     — JS vs WASM speed + correctness
```

**Dependencies**: `uv pip install playwright pytest && playwright install chromium`

**WASM build**: `./build-wasm.sh` (requires `brew install lld`)
