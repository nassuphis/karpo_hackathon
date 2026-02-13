# Proof-Read Report: README.md

Line-by-line audit of README.md against the actual codebase (`index.html`, 10,439 lines). Each section is characterized as **accurate** (verified against code, no changes needed) or **was updated** (corrected during this review).

---

## Line 1: `# PolyPaint`
**Accurate.** Title matches the app.

## Line 3: `**[Try it live](...)**`
**Accurate.** URL points to correct GitHub Pages deployment.

## Line 5: `Interactive polynomial root visualizer...`
**Accurate.** Correctly describes the app's core function.

## Lines 7–11: What It Does — math description
**Accurate.** Polynomial notation `p(z) = cₙzⁿ + ... + c₀` is correct. Subscript = power of z. n+1 coefficients, n roots. Verified against the code's coefficient ordering (`coefficients[0]` = leading term = highest power).

## Line 13: Video embed
**Accurate.** GitHub user-attachments URL — cannot verify if video still exists but syntax is correct.

## Line 15: Left panel description
**Accurate.** Coefficient dragging updates roots instantly. Domain coloring shifts in real time. Verified: `renderRoots()` called after coefficient changes, domain coloring redraws on each animation frame.

## Line 16: Right panel description
**Accurate.** Root dragging calls `rootsToCoefficients()` (verified at ~line 4310). Dashed polyline during root drag — verified in `renderCoefficients()`. The product formula `(z − r₀)(z − r₁)···` is correct.

## Line 17: Multi-select
**Accurate.** Click toggles selection, marquee-select by dragging on empty canvas, group drag maintains relative positions. Verified via `selectedCoeffs` Set and drag handler code.

## Line 18: Animate
**Accurate.** Each coefficient has independent trajectory (pathType, radius, speed, angle, ccw). Trajectory editor is always visible. Play starts all animated coefficients. Verified against data model and `animLoop()` at line 3631.

## Line 19: Transform (Ops tools)
**Accurate.** Scale/Rotate/Translate tools with live preview. Target label green for coefficients, red for roots. Verified against ops tool builder functions (~line 5100+).

## Line 20: Sonify
**Accurate.** Three layers (B/M/V), sidebar buttons, config popovers. 25 computed stats (23 STAT_TYPES + 2 constants = 25). 14 audio parameters (6 Base + 4 Voice + 4 Melody). Per-route normalization (Fixed/RunMax) and EMA smoothing. All verified against code.

## Line 21: Sensitivity coloring
**Accurate.** Derivative mode colors by Jacobian sensitivity. Formula `∂rⱼ/∂cₖ = −rⱼⁿ⁻ᵏ / p'(rⱼ)` is the standard implicit differentiation result. Rank-based normalization verified. Coefficient picker sensitivity dots verified.

## Line 22: Stats dashboard
**Accurate.** 4×4 grid = 16 plots (verified: `STAT_CANVAS_COUNT` implied by 16 canvas elements). 23 time-series (STAT_TYPES array, line 955, counted 23). 5 phase-space plots (PHASE_PLOTS array, line 958, counted 5). 4 spectrum charts (SPECTRUM_PLOTS array, line 966: SpeedSpectrum, OdometerSpectrum, WindingSpectrum, TortuositySpectrum). 4000-frame ring buffer (verified: `STAT_HISTORY_LEN = 4000`). All stat names match STAT_TYPES entries. 6 sonification features listed match code.

## Line 23: Bitmap rendering
**Accurate.** Up to 15000×15000 px (verified: bitmap-res-select has 15000 option). Fast mode with parallel Web Workers (verified). Steps up to 1M (verified: bitmap-steps-select). 10 jiggle strategies (verified: CANVAS_BG_COLORS at line 9674 has 10 jiggle modes at line 929). "random, rotate, walk, spiral, lissajous" explicitly named — all correct. WASM solver option verified.

## Line 25: Client-side, single HTML file
**Accurate.** No server dependencies. All code inline plus 2 CDN deps.

## Lines 27–31: Quick Start
**Accurate.** Open index.html directly. Live demo URL correct.

## Lines 33–44: Architecture diagram
**Accurate.** "~10K lines" — actual is 10,439 (close enough). d3.js v7, html2canvas from CDN. Ehrlich-Aberth solver, Horner evaluator, Canvas 2D, Web Workers, Web Audio. All verified.

## Line 46: No server, no build tools
**Accurate.** Two CDN deps correctly listed. WASM base64-embedded.

## Lines 48–55: Deep Dives
**Was updated.** Added link to [Coefficient Paths & Jiggle](docs/paths.md) which was missing from the Deep Dives section. All other links verified — solver.md, worker_implementation.md, sonification.md, braids.md, patterns.md all exist and match descriptions.

## Lines 57–59: Interface intro
**Accurate.** Compact header, two side-by-side panels, mid-bar.

## Line 61: Header description
**Accurate.** Version number in title, clickable Degree (3–30), Pattern dropdown, Reset, Save/Load/Export buttons (7 export modes verified), C and T diagnostic buttons. All verified against HTML.

## Line 63: Mid-bar Ops description
**Accurate.** Scale (exponential 0.1×–10×), Rotate (±0.5 turns), Translate (±2 in each axis). Popover with live preview. Target label green/red. ⊕/✕ select buttons. Inv button reverses coefficient order. All verified against code (~line 5100+).

## Line 65: Left panel description
**Was updated.** Changed from "tabular view of all coefficients with per-row path editing, bulk speed/radius controls, and a curve navigator" to include Transform dropdown (20 operations), Param1/Param2 sliders, and specific column names (position, speed, radius, curve length, curve index). Verified: `#list-transform` select has 20 non-none options, `#list-param1` and `#list-param2` range inputs exist, `.cpick-radius` column added.

## Line 67: Trajectory editor
**Accurate.** Coefficient picker (☰), All/None buttons, Update Sel, path type dropdown with path-specific controls. R/S/A/CW-CCW for circle, S/σ for Gaussian, S/W/CW-CCW for C-Ellipse. PS button. Controls dim when no selection. All verified against PATH_PARAMS and UI code.

## Line 69: Roots toolbar
**Accurate.** Trails, color popover, domain coloring, Fit, +25%, All/None buttons. Verified against HTML toolbar.

## Lines 71–93: Control table
- **Degree (3–30)**: **Accurate.** `Math.max(3, Math.min(30, ...))` at line 4334.
- **Pattern (26, 3 categories)**: **Accurate.** Counted: 5 Basic + 8 Coeff + 13 Root = 26.
- **C/T buttons**: **Accurate.** Worker config and timing stats.
- **Scrub slider (0–5s)**: **Accurate.** Range input in tab bar.
- **Play/Home**: **Accurate.** Play starts animLoop, Home resets.
- **Coefficient picker**: **Accurate.** Scrollable, index color, sensitivity dots.
- **All/None**: **Accurate.**
- **Update Sel**: **Accurate.**
- **Path type (21, 3 groups)**: **Accurate.** 1 none + 5 Basic + 12 Curves + 3 Space-filling = 21.
- **Path-specific controls**: **Accurate.** PATH_PARAMS schema verified.
- **PS button**: **Accurate.** Coprime speed search verified.
- **Trails**: **Accurate.** Loop detection, auto-stop recording.
- **Root coloring (5 modes)**: **Accurate.** uniform, rainbow, derivative, iteration, proximity. Verified at line 4543.
- **Domain coloring toggle**: **Accurate.**
- **Fit/+25%**: **Accurate.**
- **4 right panel tabs**: **Accurate.** roots, stats, sound, bitmap.
- **Stats dropdowns**: **Accurate.** 23 + 5 + 4 = 32 options per dropdown.
- **Record (7 modes)**: **Accurate.** roots, coeffs, both, stats, sound, bitmap, full.
- **Bitmap tab**: **Accurate.** All buttons verified: start, save, clear, resolution (5 options), fastmode, ROOT/COEF, steps (8 options), jiggle (10 modes), D, bg (24 colors), prog, cfg.
- **B/M/V**: **Accurate.** Three instruments with config popovers.
- **Selection count**: **Accurate.** Green for coefficients, red for roots.

## Lines 95–102: Selection
**Accurate.** Click toggle, marquee, All/None, bright glow, cross-panel clear, Escape, Update Sel behavior. All verified.

## Lines 104–123: Per-Coefficient Trajectories
- **Independent trajectories**: **Accurate.**
- **Workflow (4 steps)**: **Accurate.**
- **21 path curves in 3 groups**: **Accurate.** Listed paths match PATH_CATALOG exactly.
- **Right-click context menu**: **Accurate.** Context menu opens with path editing, Accept/cancel.
- **Delete button**: **Was updated.** Added "The context menu also has a Delete button to remove the coefficient (minimum degree 1)." Verified: `deleteCoefficient()` at line 2684, guard `coefficients.length <= 2`.
- **Add/Delete coefficients**: **Was updated.** Added bullet point about right-click add (unshift) and delete. Verified: `addCoefficientAt()` at line 2659, SVG contextmenu handler at line 1697.
- **Coefficient paths visible**: **Accurate.** `renderCoeffTrails()` draws curves.
- **Trails toggle**: **Accurate.** Loop detection, jump detection (>30% range threshold).

## Lines 125–147: File Structure
**Was updated.** Added:
- `paths.md` — was missing from file tree
- `memory_timings.md` — new doc from buffer optimization work
- `wasm_investigation.md` — new doc from WASM integration
- `test-results.md` — new doc from test suite
- `tests/` directory — Playwright Python test suite (38 tests)

All listed files verified to exist via `ls` and `glob`.

## Lines 149–157: Edge Cases
- **Leading coefficient at origin**: **Accurate.** `Math.hypot(...) < 1e-15` stripping at line 4129.
- **NaN/Inf roots**: **Was updated.** Changed from "filtered out before rendering" to "solver always returns exactly `degree` roots — non-finite results fall back to warm-start values, then to unit-circle seeds." This matches the fix documented in lessons.md (the NaN root poisoning bug fix).
- **Huge coefficients**: **Accurate.** Dynamic 1-2-5 grid formula, `computeRange` overflow cap.
- **Window resize**: **Accurate.** Resize handler at line 10400.
- **Degree change**: **Accurate.** Reinitializes coefficients.

## Lines 159–161: Path Transform Model
**Accurate.** 200 points standard, more for spiral/space-filling. Scale/rotate in place, snap to nearest. Verified in curve transform code.

## Lines 163–171: Space-Filling Curve Paths
**Accurate.** Hilbert (Moore variant, order 4, 256 points), Peano (order 3, 729 points, 1458 steps), Sierpinski (order 5, 243 segments, 486 steps). L-system rules match code. All cached on first use.

## Lines 173–180: Performance
- **rAF throttle**: **Accurate.** `solveRootsThrottled()` uses rAF.
- **Half-resolution domain coloring**: **Accurate.** `devicePixelRatio` support verified.
- **No d3 transitions**: **Accurate.** Instant position updates.
- **Warm-start 1–3 iterations**: **Accurate.** Documented in solver.md.
- **WASM solver**: **Accurate.** ~2KB binary, base64-embedded, cfg button.
- **Bitmap fast mode**: **Accurate.** Parallel workers, automatic pass looping, full-cycle auto-stop via LCM/GCD, uniform mode skips O(n²) matching, fallback to main-thread loop.

---

## Summary of Other Docs

### solver.md
**Accurate.** All parameters match code (MAX_ITER=100 main, 64 worker; TOL=1e-12 main, 1e-16² worker; Math.hypot < 1e-15 leading-zero test). WASM section correct. Memory layout matches. Build workflow correct.

### paths.md
**Was updated.** Added 5 missing jiggle modes (Circle, Spiral, Breathe, Wobble, Lissajous — only None/Random/Rotate/Walk/Scale were documented). Updated mode count from 5 to 10. Added radius (rad) column to List Tab Columns section. Replaced separate pos/pts sections with unified table.

### sonification.md
**Accurate.** Audio graph, 3 instruments (B/M/V), 6 features, sound mapping formulas, silence management, signal routing — all verified against code. Slider ranges and defaults match code.

### braids.md
**Accurate.** Brief topological explanation of root monodromy. `matchRootOrder` greedy nearest-neighbor assignment documented.

### patterns.md
**Accurate.** 5 Basic + 8 Coeff + 13 Root = 26 total. Root shape names match code options. Trail gallery images reference valid snap files.

### worker_implementation.md
**Was updated.** Fixed compositing description (was: "reads existing canvas ImageData, overwrites, puts back"; now: "persistent ImageData buffer, dirty-rect putImageData"). Fixed bottleneck #3 (was: "Single-threaded getImageData/putImageData cycle"; now: references persistent buffer optimization).

### lessons.md
**Was updated.** Major corrections:
- Fixed file line count (9,750 → 10,400)
- Fixed Section Map line numbers (all shifted due to file growth)
- Fixed solver parameters (MAX_ITER was listed as 40, corrected to 100; TOL was listed as 1e-8, corrected to 1e-12)
- Added 4 new "Things That Surprised Me" entries (#13–16: Set iteration order, coefficient indexing, trail clearing, Transform dropdown pattern)
- Updated "No Test Suite" section to reference the Playwright test suite
- Updated File Index with correct line numbers and added missing doc files
- Added new sections: List Tab, Add/Delete Coefficients
- Updated Key Code Locations with current line numbers

### memory_timings.md
**Accurate.** Pre/post optimization measurements, theoretical analysis, recommendation all match the persistent buffer implementation.

### wasm_investigation.md
**Accurate.** Status: INTEGRATED. Architecture, memory layout, build workflow, benchmarks all correct.

### test-results.md
**Accurate.** 38 tests (37 passed, 1 skipped). Test descriptions match test files in `tests/`.

---

## Changes Made During This Review

| File | Change Type | Description |
|------|------------|-------------|
| README.md | Updated | Left panel description: added Transform dropdown, Param1/Param2, column names |
| README.md | Updated | Added Deep Dives link to paths.md |
| README.md | Updated | Added add/delete coefficient feature description |
| README.md | Updated | Added Delete button to context menu description |
| README.md | Updated | File structure: added paths.md, memory_timings.md, wasm_investigation.md, test-results.md, tests/ |
| README.md | Updated | NaN/Inf handling: changed from "filtered out" to fallback description |
| lessons.md | Updated | File size 9,750 → 10,400; Section Map line numbers |
| lessons.md | Updated | Solver MAX_ITER 40→100, TOL 1e-8→1e-12 |
| lessons.md | Updated | Added List Tab, Add/Delete sections |
| lessons.md | Updated | Added 4 new "surprised me" entries (#13–16) |
| lessons.md | Updated | Fixed Key Code Locations line numbers |
| lessons.md | Updated | Updated File Index with new docs |
| lessons.md | Updated | Changed "No Test Suite" → references Playwright tests |
| paths.md | Updated | Jiggle modes: 5→10 (added Circle, Spiral, Breathe, Wobble, Lissajous) |
| paths.md | Updated | List Tab Columns: replaced pos/pts sections with unified table including rad column |
| worker_implementation.md | Updated | Compositing: persistent buffer instead of getImageData |
| worker_implementation.md | Updated | Bottleneck #3: references memory_timings.md |
