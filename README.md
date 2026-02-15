# PolyPaint

**[Try it live](https://nassuphis.github.io/karpo_hackathon/)**

Interactive polynomial root visualizer. Drag coefficients *or* roots in the complex plane and watch everything update in real time.

## What It Does

A degree-*n* polynomial p(z) = cₙzⁿ + cₙ₋₁zⁿ⁻¹ + ··· + c₁z + c₀ has *n*+1 complex coefficients and *n* complex roots. The subscript is the power of *z*: cₙ is the leading coefficient, c₀ is the constant term. The relationship between them is rich, nonlinear, and often surprising — small changes to one coefficient can send roots flying across the plane, while other perturbations barely move them.

PolyPaint makes this relationship tangible. Two side-by-side complex-plane panels let you explore it from both directions.

<video controls loop muted playsinline width="720" src="https://github.com/user-attachments/assets/7ab7733b-4a6f-47b2-b8fe-a9f650cd9c22"></video>

- **Left panel (Coefficients):** Drag any coefficient dot and watch the roots respond instantly on the right. The domain coloring background shifts in real time, revealing how the polynomial's complex landscape reshapes.
- **Right panel (Roots):** Drag any root dot and the coefficients on the left update to match — the polynomial is reconstructed from its roots via (z − r₀)(z − r₁)···(z − rₙ₋₁). During root dragging (single or multi-selected group), a dashed polyline connects all coefficient dots on the left panel, visualizing the coefficient constellation as a chain.
- **Multi-select:** Click individual dots to toggle selection, or **marquee-select** by clicking and dragging on empty canvas to select all nodes inside the rectangle. Drag any selected item and the entire group moves together, maintaining relative positions.
- **Animate:** Each coefficient has its own trajectory — assign a curve (circle, figure-8, spiral, etc.) with independent radius, speed, and direction. Select coefficients and configure them via the trajectory editor in the Coefficients tab. Hit Play and all animated coefficients follow their trajectories simultaneously, creating rich interference patterns as the roots respond to the combined perturbation.
- **Transform:** Select coefficients or roots and use interactive gesture tools — **Scale** (vertical slider with exponential mapping), **Rotate** (horizontal slider in turns), and **Translate** (2D vector pad) — all with live preview as you drag. Ops work on both coefficient and root selections — the target label turns green for coefficients, red for roots.
- **Sonify:** Three independent sound layers — **Base** (FM drone), **Melody** (pentatonic arpeggiator), and **Voice** (close-encounter beeps) — each with its own sidebar button and config popover. Click any button to open a panel of tuning sliders (pitch, FM depth, volume, cutoff, etc.) that reshape the sound in real time. The **Sound** tab exposes a signal routing matrix: any of the 25 computed stats (speed, distance, angular momentum, etc.) can be patched into any of the 14 audio parameters across all three instruments, with per-route normalization (Fixed or adaptive RunMax) and EMA smoothing. See [Sonification](docs/sonification.md) for the full algorithm.
- **Sensitivity coloring:** Switch root coloring to **Derivative** mode to color each root by how sensitive it is to coefficient perturbation — blue (stable) through white to red (volatile). Uses the Jacobian ∂rⱼ/∂cₖ = −rⱼⁿ⁻ᵏ / p'(rⱼ) with rank-based normalization. The coefficient picker also shows per-coefficient sensitivity dots.
- **Stats dashboard:** The roots panel has a **Stats** tab with a 4×4 grid of 16 configurable plots, each selectable from 23 time-series stats, 5 phase-space plots, and 4 spectrum charts. Time-series include **Force/MinForce/MaxForce** (Jacobian sensitivity per root), **Speed/MinSpeed/MaxSpeed** (root displacements), **MinDist/MaxDist/MeanDist/ΔMeanDist/σDist** (pairwise distances), **Records** (closeness record-breaking events), **AngularMom** (signed rotational momentum), **σSpeed/EMASpeed** (speed statistics), **Odometer/CycleCount** (cumulative distance and cycle detection), and 6 **sonification features** — **MedianR** (median radius from centroid), **Spread** (r90−r10), **EMed/EHi** (50th/85th percentile speeds), **Coherence** (angular clustering), **Encounters** (per-root record-breaking close approaches) — all with one-pole EMA smoothing matching the audio pipeline. Phase plots show correlations (e.g. MaxForce v MaxSpeed, MeanDist v σDist). Spectrum charts: **SpeedSpectrum** (per-root bar chart with all-time peak dots), **OdometerSpectrum**, **WindingSpectrum** (signed cumulative angle), **TortuositySpectrum** (directness ratio). Data is collected every frame into a 4000-frame ring buffer.
- **Bitmap rendering:** The **Bitmap** tab accumulates root (or coefficient) positions as single-pixel stamps on a high-resolution canvas (up to 25000×25000 px). At resolutions above 2000px, an **off-canvas split** decouples computation from display — a CPU-only persistent buffer holds full-resolution pixel data while the display canvas is capped at 2000px (~16MB GPU), eliminating Chrome's GPU context loss at high resolutions. **Fast mode** distributes the solver across parallel Web Workers for continuous off-main-thread computation, with configurable steps per pass (up to 1M) and automatic full-cycle completion. Between cycles, **jiggle** perturbation modes (10 strategies including random, rotate, walk, spiral, lissajous, and more) can shift coefficient paths to explore nearby parameter space. Export supports **JPEG, PNG, BMP, and TIFF** formats via a save popup — all encoding is pure-JS directly from the CPU buffer, no GPU involvement, works at any resolution. An optional **WASM solver** (compiled from C) can replace the JavaScript solver for faster performance at high polynomial degrees.

Everything runs client-side in a single HTML file. No server, no build step, no dependencies to install.

## Quick Start

Open [`index.html`](index.html) in any modern browser. That's it.

Or visit the **[live demo](https://nassuphis.github.io/karpo_hackathon/)**.

## Architecture

```
Single HTML file (~10K lines)
├── d3.js v7 (CDN)          — SVG rendering, drag interactions
├── html2canvas (CDN)       — "Full" snapshot export of the whole UI
├── Ehrlich-Aberth solver    — polynomial root finding (JS + optional WASM)
├── Horner evaluator         — domain coloring + derivative computation
├── Canvas 2D API            — real-time domain coloring
├── Web Workers              — parallel fast-mode bitmap rendering
└── Web Audio API            — sonification of root motion
```

No server. No WebSocket. No build tools. The entire app is one self-contained HTML file with inline CSS and JavaScript plus two CDN deps: d3.js v7 and html2canvas (used only for Export → Full snapshots). The WASM solver binary is base64-encoded and embedded directly in the HTML.

## Deep Dives

- **[Root Finding: Ehrlich-Aberth Method & Domain Coloring](docs/solver.md)** — the simultaneous iterative solver with cubic convergence, warm-starting for interactive use, WASM compilation, and HSL domain coloring
- **[Fast Mode Workers](docs/worker_implementation.md)** — multi-worker architecture, sparse pixel format, WASM solver integration, performance characteristics
- **[Coefficient Paths & Jiggle](docs/paths.md)** — curve representation, 21 path types, fast-mode cycle sync, full-cycle auto-stop derivation, and 10 jiggle perturbation modes
- **[Sonification](docs/sonification.md)** — audio graph, feature extraction from root distributions, sound mapping formulas, instrument config popovers, signal routing matrix, and silence management
- **[Root Braids and Monodromy](docs/braids.md)** — why closed loops in coefficient space permute roots, and how trail rendering visualizes it
- **[Patterns & Trail Gallery](docs/patterns.md)** — the 26 initial patterns (basic, coefficient, root shapes) and annotated trail screenshots
- **[Off-Canvas Bitmap Rendering](docs/off-canvas-render.md)** — split compute/display architecture, BMP export, GPU memory management at 10K–25K
- **[Testing](docs/test-results.md)** — 302 Playwright tests covering solver, root tracking, paths, shapes, colors, stats, save/load, morph, jiggle, fast mode, off-canvas render, multi-format image export, and JS vs WASM benchmarks

## Interface

The UI is organized around a compact header bar and two side-by-side panels with a mid-bar between them.

**Header:** App title with version number, clickable **Degree** number (opens a slider popover, range 3–30), **Pattern** dropdown, ↺ **Reset** button, file/export buttons — 💾 **Save**, 📂 **Load**, ⬇ **Export** (opens a popup with 7 export modes: Both, Coeffs, Roots, Stats, Sound, Bitmap, Full) — and two diagnostic buttons: **C** (worker count config) and **T** (timing stats).

**Mid-bar — Ops** (between left and right panels; enabled when nodes are selected — buttons brighten from dim to full when a selection exists): ⇕ **Scale** (vertical slider, exponential 0.1×–10×), ⟲ **Rotate** (horizontal slider, ±0.5 turns), ✛ **Translate** (2D vector pad, ±2 in each axis). Each opens a transient popover with live preview — drag to scrub and apply the transform in real time. Closing the popover (click outside or press Escape) simply dismisses it; changes are already applied. A colored target label below Ops shows "· coeffs" (green) or "· roots" (red) to indicate what the operations will affect. Below the transform tools: ⊕ / ✕ buttons for quick select/deselect (selects roots if any root is selected, otherwise coefficients), and **Inv** to reverse coefficient order (reflects roots around the unit circle).

**Left panel** has two tabs: **Coefficients** (complex-plane visualization with drag interaction) and **List** (tabular view with per-coefficient columns for position, speed, radius, curve length, and curve index; a **Transform** dropdown with 20 bulk operations; and **Param1/Param2** sliders for transform parameters). The tab bar also contains a **scrub slider** (drag to manually move coefficients along their paths, 0–5 seconds), ▶ **Play** / ⏸ **Pause**, and **Home** (return to start positions).

**Trajectory editor** (in the Coefficients tab, below the tab bar): ☰ Coefficient picker, ⊕ All / ✕ None selection buttons, selection label, and **Update Sel** button on the first row. Path type dropdown and path-specific controls on the second row — the available sliders depend on the chosen path type (e.g. R/S/A/CW-CCW for circular paths, S/σ for Gaussian cloud, S/W/CW-CCW for C-Ellipse). A **PS** (Prime Speed) button appears when the path has a speed parameter. Controls dim when no coefficients are selected.

**Roots toolbar** (overlay on the roots canvas): **Trails** toggle, 🎨 Root coloring popover, ◐ Domain coloring toggle, **Fit** (auto-zoom to fit all roots and trails), **+25%** (zoom out), ⊕ All / ✕ None root selection buttons.

| Control | Description |
|---------|-------------|
| **Degree** (header) | Click the number to open a horizontal slider popover (3–30). Reinitializes on change. |
| **Pattern** dropdown | Initial arrangement of coefficients or roots. 26 patterns in 3 categories (Basic, Coefficient, Root shapes). |
| **C** / **T** (header) | **C** opens worker count config (1/2/4/8/16). **T** opens timing stats (steps/sec, pass times, composite breakdown). |
| **Scrub slider** (left tab bar) | Drag to manually advance coefficients along their paths (0–5 seconds). |
| **▶ Play** / **Home** (left tab bar) | Start/pause animation; Home returns all animated coefficients to their start positions and resets stats. |
| **☰** coefficient picker | Scrollable list with index color and sensitivity dots. Click a row to toggle selection. |
| **⊕ All** / **✕ None** | Select or deselect all coefficients. |
| **Update Sel** button | Applies all current editor settings to every selected coefficient. |
| **Path type** dropdown | Trajectory curve for selected coefficients: None, Circle, Figure-8, Hilbert, etc. (21 options in 3 groups). |
| **Path-specific controls** | Sliders and toggles that change based on the selected path type: Speed (S), Radius (R), Angle (A), CW/CCW toggle, and path-specific parameters like frequency, width, or sigma. |
| **PS** button | Prime Speed — sets the selected coefficient's speed to the nearest value coprime with all other animated coefficients' speeds, maximizing the full-cycle pass count. |
| **Trails** (roots toolbar) | Enables root trail recording. Loop detection stops collecting new trail points and auto-stops video recording after one full cycle. |
| **🎨** (roots toolbar) | Opens a root coloring popover with 5 modes: **Uniform** (single color — offers white and all coefficient colors), **Index Rainbow** (color by root index), **Derivative** (sensitivity coloring), **Iteration Count** (convergence speed, for bitmap), and **Root Proximity** (nearest-root distance, for bitmap). |
| **◐** (roots toolbar) | Toggle domain coloring on the roots canvas background. |
| **Fit** / **+25%** (roots toolbar) | Auto-zoom to fit all roots and trails, or zoom out by 25%. |
| **Roots / Stats / Sound / Bitmap** tabs | Right panel tab bar — switch between root visualization, stats dashboard, sound routing, and bitmap rendering. |
| **Stats** dropdowns | Each of the 16 stat plots has a dropdown: 23 time-series, 5 phase-space plots, and 4 spectrum charts. Sonification features (MedianR, Spread, EMed, EHi, Coherence, Encounters) mirror the audio pipeline with matching EMA smoothing. |
| **⏺** record (tab bar) | Records to WebM video. Mode selector: Roots, Coeffs, Both, Stats, Sound, Bitmap, or Full (both panels + info bar). "Both" is tab-aware — renders coefficients + the active right-side tab. Auto-stops on loop completion when Trails are enabled. |
| **Bitmap** tab | Accumulates root positions as single-pixel stamps on a canvas. Toolbar buttons (left to right): **init** snapshots animation state, clears bitmap, and resets elapsed to 0. **save** opens a format popup (JPEG/PNG/BMP/TIFF with quality slider for JPEG) and downloads the full-resolution image from the CPU buffer (no GPU involvement). **clear** resets the canvas pixels (elapsed unchanged). **Resolution** dropdown (1000/2000/5000/8000/10000/15000/25000 px) — above 2000px, the display canvas is capped at 2000px while computation runs at full resolution. **start**/**stop** toggles continuous fast mode (parallel Web Workers). Stopping preserves all state; resuming continues where it left off. **ROOT/COEF** toggles between plotting root or coefficient positions. **Steps** dropdown (10/100/1K/5K/10K/50K/100K/1M) sets solver steps per pass. **cfg** opens the config popup: solver engine (JS/WASM), jiggle perturbation (10 modes with mode-specific tuning, interval slider, select-all), and background color (24 preset colors). See [Paths](docs/paths.md) for jiggle formulas. A zero-padded elapsed seconds counter appears during computation. |
| **B / M / V** (Sound tab toolbar) | Toggle and configure the three sound layers: **B** Base (drone), **M** Melody (arpeggio), **V** Voice (beeps). Click to open config popover with on/off toggle + tuning sliders. See [Sonification](docs/sonification.md). |
| **Selection count** (panel headers) | Shows the number of selected items next to "Coefficients" (green) and "Roots" (red) panel titles. |

### Selection

- **Click** any dot to toggle it into the selection.
- **Marquee select:** Click and drag on empty canvas to draw a selection rectangle — all nodes inside are added to the selection.
- **⊕ All / ✕ None:** Quick select/deselect all coefficients from the trajectory editor bar, or all roots from the roots toolbar.
- Selected nodes pulse with a bright glow to clearly indicate membership.
- Clicking a coefficient clears any root selection and vice versa. Press **Escape** to close any open tool, or deselect all if no tool is open.
- The trajectory editor displays the last-selected coefficient's settings. Adjusting any slider or toggle immediately applies the change to all selected coefficients; clicking **Update Sel** does the same.

### Per-Coefficient Trajectories

Each coefficient stores its own trajectory settings: path type, radius, speed, angle, and direction. There are no shared "path groups" — every coefficient is independent.

**Workflow:**
1. Select one or more coefficients (click, marquee, or ⊕ All)
2. Choose a path type, adjust controls in the trajectory editor
3. Settings apply immediately to all selected coefficients (or click **Update Sel** explicitly)
4. Click **▶ Play** → all coefficients with a trajectory animate simultaneously

**21 path curves** (including None) in three groups:
- **Basic:** None, Circle, Horizontal, Vertical, Spiral, Gaussian cloud
- **Curves:** Lissajous, Figure-8, Cardioid, Astroid, Deltoid, Rose (3-petal), Spirograph, Hypotrochoid, Butterfly, Star (pentagram), Square, C-Ellipse
- **Space-filling:** Hilbert (Moore curve), Peano, Sierpinski arrowhead

Right-click any coefficient to open a context menu with trajectory settings for that individual coefficient — changes preview live. Click "Accept" to commit; press Escape or click outside to cancel and revert. The context menu also has a **Delete** button to remove the coefficient (down to degree 1; the header Degree slider initializes between 3–30).

- **Add/Delete coefficients:** Right-click on empty canvas space to add a new coefficient at that position (as the new highest-power term). Right-click an existing coefficient to edit or delete it.
- **Coefficient paths** are always visible on the left panel when a trajectory is assigned — the colored curve shows exactly where each coefficient will travel during animation.
- **Trails** toggle (roots toolbar): enables root trail recording on the right panel. Roots leave colored SVG path trails as they move. Loop detection stops collecting new trail points and auto-stops video recording after one full cycle. Jump detection breaks trails when consecutive points are far apart (>30% of visible range), avoiding visual artifacts from root-identity swaps.

## File Structure

```
karpo_hackathon/
├── index.html            # Entire app (~10K lines): CSS, JS, HTML, WASM all inline
├── solver.c              # WASM solver source (Ehrlich-Aberth in pure C)
├── build-wasm.sh         # Compile solver.c → solver.wasm → base64
├── solver.wasm           # Compiled WASM binary (~2KB)
├── docs/
│   ├── solver.md         # Ehrlich-Aberth method + domain coloring + WASM
│   ├── worker_implementation.md  # Fast mode workers + WASM integration
│   ├── paths.md          # Coefficient path curves, cycle sync, jiggle
│   ├── sonification.md   # Audio graph, feature extraction, sound mapping
│   ├── braids.md         # Root braids and monodromy
│   ├── patterns.md       # Pattern catalog + trail gallery
│   ├── off-canvas-render.md  # Split compute/display + BMP export design
│   ├── lessons.md        # Architecture notes + debugging war stories
│   ├── memory_timings.md # Persistent buffer optimization analysis
│   ├── wasm_investigation.md  # WASM solver design + benchmarks
│   └── test-results.md   # Playwright test results + JS/WASM benchmarks
├── tests/                # Playwright Python tests (302 tests, 16 files)
├── snaps/                # Snap captures (PNG + JSON metadata)
└── README.md
```

## Technical Notes

### Edge Cases Handled

- **Leading coefficient at origin**: near-zero leading coefficients are stripped before solving
- **NaN/Inf roots**: solver always returns exactly `degree` roots — non-finite results fall back to warm-start values, then to unit-circle seeds
- **Huge coefficients during root drag**: `rootsToCoefficients` can produce coefficients with magnitudes up to ~10¹¹ at high degree; grid step computation uses a dynamic 1-2-5 × 10ᵏ formula to keep ≤20 grid lines per axis at any scale, and `computeRange` caps overflow to prevent infinite loops
- **Window resize**: panels dynamically resize, solver re-runs
- **Degree change**: coefficients reinitialized, both panels reset

### Path Transform Model

Each coefficient with a trajectory stores a sampled curve (typically 200 points, more for space-filling/spiral paths). When radius or angle sliders change, the curve is transformed in place (scale/rotate around the home position) and the coefficient is snapped to the nearest point on the updated curve so it remains on-trajectory. This avoids regenerating the curve from scratch, which would cause visible jumps after play-pause.

### Space-Filling Curve Paths

Three space-filling curves are available as animation paths, all implemented via L-system turtle graphics with caching:

- **Hilbert (Moore curve):** Closed variant of the Hilbert curve — 4 Hilbert sub-curves arranged in a loop. Order 4, 256 points. Fills a square with uniform step sizes. L-system: `LFL+F+LFL`, `L → -RF+LFL+FR-`, `R → +LF-RFR-FL+`.
- **Peano:** Classic Peano space-filling curve. Order 3, 729 points, out-and-back traversal for closure (1458 steps total). L-system: `L`, `L → LFRFL-F-RFLFR+F+LFRFL`, `R → RFLFR+F+LFRFL-F-RFLFR`.
- **Sierpinski arrowhead:** Fills a Sierpinski triangle. Order 5, 243 segments, out-and-back for closure (486 steps total). L-system: `A → B-A-B`, `B → A+B+A` with 60-degree turns.

All three generate perfectly uniform step sizes and are cached on first use.

### Performance

- Root solving throttled via `requestAnimationFrame` (~60fps cap)
- Domain coloring rendered to half-resolution canvas, CSS-scaled with `devicePixelRatio` support
- No d3 transitions on dots — positions update instantly to avoid animation conflicts during rapid drag
- Warm-started Ehrlich-Aberth typically converges in 1–3 iterations during interactive drag
- **WASM solver** option for fast-mode workers: the Ehrlich-Aberth algorithm compiled from C to WebAssembly (~2KB binary, base64-embedded). Eliminates JIT warmup and GC pauses. Selectable via the **cfg** button in the bitmap toolbar.
- **Bitmap fast mode** runs the solver in parallel Web Workers — the entire hot loop (Ehrlich-Aberth solver, root matching, pixel painting) executes off the main thread continuously. Workers loop automatically across passes, posting periodic pixel snapshots back for display. Full-cycle auto-stop computes the LCM of all animated coefficient periods (via GCD of integer speeds) and terminates after exactly that many passes. When root coloring is Uniform, the O(n²) root-matching step is skipped for faster throughput. Falls back to a chunked main-thread loop if Workers are unavailable.
