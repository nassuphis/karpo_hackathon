# PolyPaint

**[Try it live](https://nassuphis.github.io/karpo_hackathon/)**

Interactive polynomial root visualizer. Drag coefficients *or* roots in the complex plane and watch everything update in real time.

## What It Does

A degree-*n* polynomial p(z) = c₀zⁿ + c₁zⁿ⁻¹ + ··· + cₙ has *n*+1 complex coefficients and *n* complex roots. The relationship between them is rich, nonlinear, and often surprising — small changes to one coefficient can send roots flying across the plane, while other perturbations barely move them.

PolyPaint makes this relationship tangible. Two side-by-side complex-plane panels let you explore it from both directions.

<video controls loop muted playsinline width="720" src="https://github.com/user-attachments/assets/7ab7733b-4a6f-47b2-b8fe-a9f650cd9c22"></video>

- **Left panel (Coefficients):** Drag any coefficient dot and watch the roots respond instantly on the right. The domain coloring background shifts in real time, revealing how the polynomial's complex landscape reshapes.
- **Right panel (Roots):** Drag any root dot and the coefficients on the left update to match — the polynomial is reconstructed from its roots via (z − r₀)(z − r₁)···(z − rₙ₋₁).
- **Multi-select:** Click individual dots to toggle selection, or **marquee-select** by clicking and dragging on empty canvas to select all nodes inside the rectangle. Drag any selected item and the entire group moves together, maintaining relative positions.
- **Animate:** Define multiple simultaneous animation paths — each path drives a different subset of coefficients along its own curve (circle, figure-8, spiral, etc.) with independent radius, speed, and direction. Hit Play and all paths activate at once, creating rich interference patterns as the roots respond to the combined perturbation.
- **Transform:** Select coefficients or roots and use interactive gesture tools — **Scale** (vertical slider with exponential mapping), **Rotate** (horizontal slider in turns), and **Translate** (2D vector pad) — all with live preview as you drag. Ops work on both coefficient and root selections — the target label turns green for coefficients, red for roots.
- **Sonify:** Three independent sound layers — **Base** (FM drone), **Melody** (pentatonic arpeggiator), and **Voice** (close-encounter beeps) — each with its own sidebar button and config popover. Click any button to open a panel of tuning sliders (pitch, FM depth, volume, cutoff, etc.) that reshape the sound in real time. The **Sound** tab exposes a signal routing matrix: any of the 25 computed stats (speed, distance, angular momentum, etc.) can be patched into any of the 14 audio parameters across all three instruments, with per-route normalization (Fixed or adaptive RunMax) and EMA smoothing. See [Sonification](docs/sonification.md) for the full algorithm.
- **Sensitivity coloring:** Switch root coloring to **Derivative** mode to color each root by how sensitive it is to coefficient perturbation — blue (stable) through white to red (volatile). Uses the Jacobian ∂rⱼ/∂cₖ = −rⱼⁿ⁻ᵏ / p'(rⱼ) with rank-based normalization. The coefficient picker also shows per-coefficient sensitivity dots.
- **Stats dashboard:** The roots panel has a **Stats** tab with a 4×4 grid of 16 configurable plots, each selectable from 23 time-series stats, 5 phase-space plots, and 4 spectrum charts. Time-series include **Force/MinForce/MaxForce** (Jacobian sensitivity per root), **Speed/MinSpeed/MaxSpeed** (root displacements), **MinDist/MaxDist/MeanDist/ΔMeanDist/σDist** (pairwise distances), **Records** (closeness record-breaking events), **AngularMom** (signed rotational momentum), **σSpeed/EMASpeed** (speed statistics), **Odometer/CycleCount** (cumulative distance and cycle detection), and 6 **sonification features** — **MedianR** (median radius from centroid), **Spread** (r90−r10), **EMed/EHi** (50th/85th percentile speeds), **Coherence** (angular clustering), **Encounters** (per-root record-breaking close approaches) — all with one-pole EMA smoothing matching the audio pipeline. Phase plots show correlations (e.g. MaxForce v MaxSpeed, MeanDist v σDist). Spectrum charts: **SpeedSpectrum** (per-root bar chart with all-time peak dots), **OdometerSpectrum**, **WindingSpectrum** (signed cumulative angle), **TortuositySpectrum** (directness ratio). Data is collected every frame into a 4000-frame ring buffer.

Everything runs client-side in a single HTML file. No server, no build step, no dependencies to install.

## Quick Start

Open [`index.html`](index.html) in any modern browser. That's it.

Or visit the **[live demo](https://nassuphis.github.io/karpo_hackathon/)**.

## Architecture

```
Single HTML file (~5800 lines)
├── d3.js v7 (CDN)          — SVG rendering, drag interactions
├── Ehrlich-Aberth solver    — polynomial root finding in pure JS
├── Horner evaluator         — domain coloring + derivative computation
├── Canvas 2D API            — real-time domain coloring
└── Web Audio API            — sonification of root motion
```

No server. No WebSocket. No build tools. The entire app is one self-contained HTML file with inline CSS and JavaScript plus a single CDN dependency (d3.js).

## Deep Dives

- **[Root Finding: Ehrlich-Aberth Method & Domain Coloring](docs/solver.md)** — the simultaneous iterative solver with cubic convergence, warm-starting for interactive use, and HSL domain coloring
- **[Sonification](docs/sonification.md)** — audio graph, feature extraction from root distributions, sound mapping formulas, instrument config popovers, signal routing matrix, and silence management
- **[Root Braids and Monodromy](docs/braids.md)** — why closed loops in coefficient space permute roots, and how trail rendering visualizes it
- **[Patterns & Trail Gallery](docs/patterns.md)** — the 26 initial patterns (basic, coefficient, root shapes) and annotated trail screenshots

## Interface

The UI is organized around a left sidebar with three groups and a compact header:

**Header:** App title, clickable **Degree** label (click to open slider, range 3–30), and **Pattern** dropdown.

**Sidebar — View:** ◐ Domain coloring toggle, 🎨 Root coloring toggle, **B** Base / **M** Melody / **V** Voice sound toggles (each opens a config popover).

**Sidebar — Tools:** ✕ Deselect all, ⊕ Select all coefficients, ☰ Coefficient picker (scrollable list with index color + sensitivity dots), ⬇ Export snapshot.

**Sidebar — Ops** (enabled when nodes are selected — buttons brighten from dim to full when a selection exists): ⇕ **Scale** (vertical slider, exponential 0.1×–10×), ⟲ **Rotate** (horizontal slider, ±0.5 turns), ✛ **Translate** (2D vector pad, ±2 in each axis). Each opens a transient popover with live preview — drag to scrub, click outside or press Escape to commit and close. A colored target label below Ops shows "· coeffs" (green) or "· roots" (red) to indicate what the operations will affect.

| Control | Description |
|---------|-------------|
| **Degree** (header) | Click the number → horizontal slider popover (3–30). Reinitializes on change. |
| **Pattern** dropdown | Initial arrangement of coefficients or roots. 26 patterns in 3 categories. |
| **⊕** (coeff panel header) | Select all coefficients for group operations. |
| **Sel→Path** button | Captures the current coefficient selection into a new animation path. |
| **◀ ▶** path navigation | Cycle through paths — auto-selects the viewed path's coefficients. |
| **A (Angle)** slider | Rotates the path shape around the coefficient (0–1 → 0–360°). |
| **CW / CCW** toggle | Sets clockwise or counter-clockwise direction for the current path. |
| **×** delete button | Removes the currently viewed path. |
| **Roots / Stats / Sound** tabs | Roots panel tab bar — switch between root visualization, stats dashboard, and sound routing. |
| **Stats** dropdowns | Each of the 16 stat plots has a dropdown: 23 time-series, 5 phase-space plots, and 4 spectrum charts. Sonification features (MedianR, Spread, EMed, EHi, Coherence, Encounters) mirror the audio pipeline with matching EMA smoothing. |
| **⏺** record (tab bar) | Records to WebM video. Mode selector: Roots, Coefficients, or Both (side-by-side). Auto-stops on loop completion. |
| **⌂ Home** button | Returns all animated coefficients to their start positions (curve[0]) — resets the animation clock, clears stats data, without changing path shapes. |
| **B / M / V** sound buttons | Toggle and configure the three sound layers. Click to open config popover with on/off toggle + tuning sliders. See [Sonification](docs/sonification.md). |
| **Selection count** (panel headers) | Shows the number of selected items next to "Coefficients" (green) and "Roots" (red) panel titles. |

### Selection

- **Click** any dot to toggle it into the selection.
- **Marquee select:** Click and drag on empty canvas to draw a selection rectangle — all nodes inside are added to the selection.
- **Path navigation** auto-selects: cycling ◀/▶ through paths overrides the selection with the current path's coefficients.
- Selected nodes pulse with a bright glow to clearly indicate membership.
- Clicking a coefficient clears any root selection and vice versa. Press **Escape** to close any open tool, or deselect all if no tool is open.

### Multi-Path Animation

The animation system supports **multiple simultaneous paths**, each driving a different subset of coefficients along its own curve with independent settings.

**Workflow:**
1. Select coefficients → click **Sel→Path** → a new path is created (selection persists for further operations)
2. Select more coefficients → click **Sel→Path** again → a second path is created
3. Use **◀ ▶** to navigate between paths and adjust each one's curve, radius, speed, and direction
4. Click **Play** → all paths animate simultaneously

Each path has:
- **20 path curves** (including None) in three groups:
  - **Basic:** None, Circle, Horizontal, Vertical, Spiral, Random walk
  - **Curves:** Lissajous (3:2), Figure-8, Cardioid, Astroid, Deltoid, Rose (3-petal), Spirograph, Hypotrochoid, Butterfly, Star (pentagram), Square
  - **Space-filling:** Hilbert (Moore curve), Peano, Sierpinski arrowhead
- **Radius**, **Speed**, and **Angle** sliders (independent per path)
- **CW/CCW** direction toggle

When a coefficient is assigned to a new path, it is automatically removed from any existing path. Empty paths are auto-deleted.

- **Coefficient paths** are always visible on the left panel when a path is assigned — the colored curve shows exactly where each coefficient will travel during animation.
- **Trails** toggle: enables root trail recording on the right panel. Roots leave colored SVG path trails as they move. Loop detection auto-stops recording after one full cycle. Jump detection breaks trails at root-index swaps to avoid artifacts.

## File Structure

```
karpo_hackathon/
├── index.html            # Entire app (~5800 lines): CSS, JS, HTML all inline
├── docs/
│   ├── solver.md         # Ehrlich-Aberth method + domain coloring
│   ├── sonification.md   # Audio graph, feature extraction, sound mapping
│   ├── braids.md         # Root braids and monodromy
│   └── patterns.md       # Pattern catalog + trail gallery
├── snaps/                # Snap captures (PNG + JSON metadata)
└── README.md
```

## Technical Notes

### Edge Cases Handled

- **Leading coefficient at origin**: near-zero leading coefficients are stripped before solving
- **NaN/Inf roots**: filtered out before rendering
- **Window resize**: panels dynamically resize, solver re-runs
- **Degree change**: coefficients reinitialized, both panels reset

### Path Transform Model

Each coefficient assigned to an animation path stores 200 absolute curve points. When radius or angle sliders change, the existing points are transformed in place — radius scales around the coefficient's position, angle rotates around it. The coefficient stays fixed and the path reshapes smoothly around it. This avoids regenerating the curve from scratch, which would cause visible jumps after play-pause.

### Space-Filling Curve Paths

Three space-filling curves are available as animation paths, all implemented via L-system turtle graphics with caching:

- **Hilbert (Moore curve):** Closed variant of the Hilbert curve — 4 Hilbert sub-curves arranged in a loop. Order 4, 256 points. Fills a square with uniform step sizes. L-system: `LFL+F+LFL`, `L → -RF+LFL+FR-`, `R → +LF-RFR-FL+`.
- **Peano:** Classic Peano space-filling curve. Order 3, 729 points. Not naturally closed — uses out-and-back traversal for closure. L-system: `L`, `L → LFRFL-F-RFLFR+F+LFRFL`, `R → RFLFR+F+LFRFL-F-RFLFR`.
- **Sierpinski arrowhead:** Fills a Sierpinski triangle. Order 5, 243 segments. Also out-and-back. L-system: `A → B-A-B`, `B → A+B+A` with 60-degree turns.

All three generate perfectly uniform step sizes and are cached on first use.

### Performance

- Root solving throttled via `requestAnimationFrame` (~60fps cap)
- Domain coloring rendered to half-resolution canvas, CSS-scaled with `devicePixelRatio` support
- No d3 transitions on dots — positions update instantly to avoid animation conflicts during rapid drag
- Warm-started Ehrlich-Aberth typically converges in 1–3 iterations during interactive drag
