# Test Results

**38 tests total: 37 passed, 1 skipped** | Runtime: ~30s | Headless Chromium on Apple Silicon

Run with: `pytest tests/ -v -s`

---

## test_solver.py — Ehrlich-Aberth Solver Correctness (10 tests)

Tests `solveRootsEA()` (line 4005) — pure function, no global state needed.

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

## test_matching.py — Root Tracking (6 tests)

Tests `matchRootOrder()` (line 4156) — pure function, greedy nearest-neighbor matching.

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

## test_curves.py — Animation Curve Generation (10 tests)

Tests `computeCurveN()` (line 3396) — generates animation paths for coefficients.

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

## test_integration.py — End-to-End (3 tests)

| Test | Status | What it checks |
|------|--------|----------------|
| `TestLoadSnap::test_load_bug1` | PASS | Load bug1.json snap → degree matches (22) |
| `TestSolverDeterminism::test_repeated_solve_identical` | PASS | Same polynomial solved 10× → bit-identical results |
| `TestFastModePixels::test_fast_mode_produces_pixels` | SKIP | `enterFastMode` needs bitmap tab active in headless |

**Skip reason**: The fast mode pixel test requires the bitmap tab to be visible and workers initialized. In headless Chromium with the default page state, `enterFastMode()` may not find the bitmap canvas ready. This test works when the page is in bitmap tab state.

---

## test_benchmark.py — JS vs WASM Performance (4 tests)

Tests both correctness and speed of the WASM Ehrlich-Aberth solver (`solver.c` → `solver.wasm`).

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
- At low degrees (5-20), WASM wins by 1.3-5x — likely JIT warmup overhead and allocation cost (JS creates `new Float64Array` each call, WASM reuses linear memory)
- At high degrees (50-100), WASM advantage shrinks to ~1.1-1.2x — V8's JIT fully optimizes the hot loop, so both converge to similar throughput
- The degree-10 outlier (4.88x) suggests the JS JIT hasn't fully optimized at that call count / loop size
- For the real workload (degree 29 in fast mode with 16 workers), expect ~1.5-2x improvement, plus elimination of GC jitter

---

## Architecture

All tests use **Playwright Python** (via `uv`) running headless Chromium. Tests call functions in the actual `index.html` page via `page.evaluate()` — no code extraction or duplication needed.

```
tests/
  conftest.py          — Fixtures: HTTP server, browser, page, wasm_b64
  test_solver.py       — solveRootsEA correctness (pure function)
  test_matching.py     — matchRootOrder correctness (pure function)
  test_curves.py       — computeCurveN correctness (semi-pure)
  test_integration.py  — Snap loading, determinism, fast mode
  test_benchmark.py    — JS vs WASM speed + correctness
```

**Dependencies**: `uv pip install playwright pytest && playwright install chromium`

**WASM build**: `./build-wasm.sh` (requires `brew install lld`)
