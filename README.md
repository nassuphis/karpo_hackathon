# PolyPaint

**[Try it live](https://nassuphis.github.io/karpo_hackathon/)**

Interactive polynomial root visualizer. Drag coefficients *or* roots in the complex plane and watch everything update in real time.

## What It Does

A degree-*n* polynomial p(z) = c₀zⁿ + c₁zⁿ⁻¹ + ··· + cₙ has *n*+1 complex coefficients and *n* complex roots. The relationship between them is rich, nonlinear, and often surprising — small changes to one coefficient can send roots flying across the plane, while other perturbations barely move them.

PolyPaint makes this relationship tangible. Two side-by-side complex-plane panels let you explore it from both directions:

- **Left panel (Coefficients):** Drag any coefficient dot and watch the roots respond instantly on the right. The domain coloring background shifts in real time, revealing how the polynomial's complex landscape reshapes.
- **Right panel (Roots):** Drag any root dot and the coefficients on the left update to match — the polynomial is reconstructed from its roots via (z − r₀)(z − r₁)···(z − rₙ₋₁).
- **Multi-select:** Click multiple coefficients (or roots) to select a group. Drag any member and the whole group moves together, maintaining their relative positions.
- **Animate:** Define multiple simultaneous animation paths — each path drives a different subset of coefficients along its own curve (circle, figure-8, spiral, etc.) with independent radius, speed, and direction. Hit Play and all paths activate at once, creating rich interference patterns as the roots respond to the combined perturbation.
- **Transform:** Select coefficients or roots and apply Scale, Add (complex), or Rotate operations to the group.

Everything runs client-side in a single HTML file. No server, no build step, no dependencies to install.

## Quick Start

Open [`index.html`](index.html) in any modern browser. That's it.

Or visit the **[live demo](https://nassuphis.github.io/karpo_hackathon/)**.

## Architecture

```
Single HTML file (~2000 lines)
├── d3.js v7 (CDN)          — SVG rendering, drag interactions
├── Ehrlich-Aberth solver    — polynomial root finding in pure JS
├── Horner evaluator         — domain coloring + derivative computation
└── Canvas 2D API            — real-time domain coloring
```

No server. No WebSocket. No build tools. The entire app is one self-contained HTML file with inline CSS and JavaScript plus a single CDN dependency (d3.js).

## Root Finding: Ehrlich-Aberth Method

The core computational engine is a pure JavaScript implementation of the [Ehrlich-Aberth method](https://en.wikipedia.org/wiki/Aberth_method) — a simultaneous iterative root-finding algorithm with cubic convergence.

### How It Works

Given a degree-*n* polynomial with coefficients c₀, ..., cₙ, the algorithm maintains *n* root approximations z₁, ..., zₙ and refines them all simultaneously:

1. **Evaluate** p(zᵢ) and p'(zᵢ) at each current root estimate using [Horner's method](https://en.wikipedia.org/wiki/Horner%27s_method) — this computes both the polynomial value and its derivative in a single O(n) pass per root.

2. **Newton step:** Compute the Newton correction wᵢ = p(zᵢ) / p'(zᵢ).

3. **Aberth correction:** Instead of applying the Newton step directly (which would be plain Newton's method and only converge quadratically), apply the Aberth deflation:

   ```
   zᵢ ← zᵢ − wᵢ / (1 − wᵢ · Σⱼ≠ᵢ 1/(zᵢ − zⱼ))
   ```

   The sum term accounts for the other roots, effectively deflating the polynomial so each root estimate repels from the others. This is what gives the method its cubic convergence and prevents multiple estimates from collapsing onto the same root.

4. **Converge** when max |correction| < 10⁻¹².

### Why It's Fast for Interactive Use

The key insight is **warm-starting**: when the user drags a coefficient slightly, the roots barely move. The previous frame's root positions are an excellent initial guess, so the solver converges in **1–3 iterations** during interactive drag (versus 15–30 iterations from a cold start). At degree 30, each iteration is O(n²) for the Aberth sums, making the total cost negligible compared to the domain coloring render.

### Implementation Details

The solver (`solveRootsEA` in `index.html`) uses:

- **Flat arrays** `[re, im]` for root storage during iteration (avoids object allocation overhead in the hot loop)
- **`Float64Array`** for coefficient storage (cache-friendly, typed-array fast paths in V8/SpiderMonkey)
- **Simultaneous Horner evaluation** of p(z) and p'(z) — the derivative is accumulated alongside the polynomial value with zero extra cost:
  ```
  dp = dp · z + p      // derivative accumulator
  p  = p · z + cₖ      // polynomial accumulator
  ```
- **Guard clauses** for degenerate cases: near-zero derivative (skip update), near-coincident roots (skip Aberth term), leading zero coefficients (strip before solving), degree 1 (direct formula)
- **Radius heuristic** for cold-start initialization: initial guesses are spread on a circle of radius (|cₙ|/|c₀|)^(1/n), derived from the polynomial's coefficient ratio, with an angular offset of 0.37 radians to break symmetry

### Bidirectional Editing

Dragging roots works in the opposite direction: the polynomial is reconstructed from its roots by expanding the product (z − r₀)(z − r₁)···(z − rₙ₋₁) using sequential polynomial multiplication — O(n²) complex multiplications. The resulting coefficients are rendered on the left panel and the domain coloring updates accordingly.

## Domain Coloring

When enabled (off by default, toggle via pill button), the roots panel background is painted using [domain coloring](https://en.wikipedia.org/wiki/Domain_coloring). For each pixel at position z in the complex plane, the polynomial p(z) is evaluated and mapped to a color:

- **Hue** = arg(p(z)) — the phase of the polynomial's value. Roots appear as points where all colors converge (all phases meet at a zero).
- **Lightness** = 0.5 + 0.4·cos(2π·frac(log₂|p(z)|)) — contour lines at powers-of-2 modulus. Zeros appear as dark points.
- **Saturation** = 0.8 fixed.

The polynomial is evaluated via Horner's method. The canvas renders at half resolution with `devicePixelRatio` support and is CSS-scaled to full size, keeping it smooth at 60fps even at degree 30.

## Root Braids and Monodromy

When you animate a coefficient along a closed loop, the roots don't just wiggle — they trace out a **braid**. This is a topological phenomenon with deep mathematical roots (pun intended).

The space of degree-*n* polynomials with distinct roots is topologically the [configuration space](https://en.wikipedia.org/wiki/Configuration_space_(mathematics)) of *n* unordered points in **C**. Its fundamental group is the [braid group](https://en.wikipedia.org/wiki/Braid_group) B_n. A closed loop in coefficient space induces a **monodromy permutation** on the roots — after one full cycle, root #3 might now occupy the position that root #7 had before.

This is not a solver artifact; it is a topological invariant of the loop. Different loops around different "holes" in coefficient space produce different permutations. The [cohomology](https://en.wikipedia.org/wiki/Cohomology) of the configuration space (computed by Arnol'd and Cohen) classifies these possibilities.

**What you see in PolyPaint:** the trail patterns are visual braids. When roots swap indices mid-trail, the jump detection breaks the path rather than drawing a false connecting line. A future improvement could track roots by continuity (solving a frame-to-frame assignment problem) rather than by array index, which would eliminate index swaps entirely and reveal the true braid structure.

## Controls

| Control | Description |
|---------|-------------|
| **Domain coloring** pill button | Toggles the domain coloring background on the roots panel (off by default). |
| **Color roots** pill button | Toggles rainbow coloring on roots by array index (on by default). |
| **Deselect all** button | Clears all coefficient and root selections. |
| **Select all coeffs** button | Selects all coefficients for group operations. |
| **Degree** number input (3–30) | Number of polynomial roots. Reinitializes coefficients on change. |
| **Pattern dropdown** | Initial arrangement of coefficients or roots. 26 patterns in 3 categories. |
| **Scale** action button | Multiplies all selected elements by a real number. |
| **Add** action button | Adds a complex number (re + im·i) to all selected elements. |
| **Rotate** action button | Rotates all selected elements by multiplying with exp(2πi·rev). |
| **Sel→Path** button | Captures the current coefficient selection into a new animation path. |
| **◀ ▶** path navigation | Cycle through defined paths to view/edit each one's settings. |
| **A (Angle)** slider | Rotates the path shape around the coefficient (0–1 → 0–360°). |
| **CW / CCW** toggle | Sets clockwise or counter-clockwise direction for the current path. |
| **×** delete button | Removes the currently viewed path. |
| **Snap** button | Exports a PNG screenshot and JSON metadata file for the current state. |

### Multi-Select and Group Drag

Click any coefficient or root dot to toggle it into the selection. Click again to deselect. Selected items glow to indicate membership. Drag any selected item and the entire group translates together — relative positions are preserved.

Clicking a coefficient clears any root selection and vice versa, so you work with one panel at a time. Press **Escape** to deselect all.

### Multi-Path Animation

The animation system supports **multiple simultaneous paths**, each driving a different subset of coefficients along its own curve with independent settings.

**Workflow:**
1. Select coefficients → click **Sel→Path** → a new path is created and the selection clears
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

### Trail Gallery

<p align="center">
  <img src="snaps/polypaint-2026-02-08T12-25-27.png" width="90%" alt="Degree-24 grid pattern — circle path, one complete loop">
</p>

**Degree 24, grid pattern, circle path (one full loop)** — 17 of 25 coefficients orbiting on a circle (radius 0.75, speed 0.5). Left panel shows the coefficient trails — each selected coefficient traces its own circle, creating an overlapping ring pattern from the grid arrangement. Right panel shows the resulting root braids: small loops and cusps where roots nearly collide, large sweeping arcs where roots respond dramatically to the perturbation. The loop detection fired after one complete cycle, stopping trail accumulation automatically.

<p align="center">
  <img src="snaps/polypaint-2026-02-08T12-29-26.png" width="90%" alt="Degree-24 grid pattern — figure-8 path, one complete loop">
</p>

**Degree 24, grid pattern, figure-8 path** — Same 17 coefficients, now following a figure-8 (lemniscate) at radius 1.5. The coefficient trails on the left form a dense weave of overlapping figure-8s. On the right, every root inherits the double-loop character — small figure-8 knots appear throughout, with some roots tracing tight local loops while others sweep wide arcs. The larger radius amplifies the perturbation, pushing roots further and producing more dramatic braiding than the circle path above.

<p align="center">
  <img src="snaps/polypaint-2026-02-08T12-48-07.png" width="90%" alt="Degree-30 two-clusters pattern — square path">
</p>

**Degree 30, two-clusters pattern, square path** — 11 of 31 coefficients (from one cluster) following a square path at radius 1.5, speed 0.4. The coefficient trails on the left show nested rectangles — each selected coefficient traces its own square, offset by its position within the cluster. The 30 roots on the right arrange in a large ring with emerging trail segments showing the early stages of the braid. The two-cluster pattern concentrates coefficients into two groups, creating an asymmetric perturbation that pushes some roots into tight local orbits while others track the ring's perimeter.

<p align="center">
  <img src="snaps/polypaint-2026-02-08T12-53-02.png" width="90%" alt="Degree-30 chessboard roots — circle path, 5 coefficients">
</p>

**Degree 30, chessboard root shape, circle path** — Only 5 of 31 coefficients selected, orbiting on a large circle (radius 2.0, speed 0.5). The roots were initialized in a chessboard grid pattern (a root shape, so the coefficients were computed from the grid). On the left, the 5 selected coefficients trace circles of varying sizes depending on their magnitude. On the right, most roots stay clustered near their grid positions while one outlier root swings through a wide arc — a striking demonstration of how perturbing a few high-order coefficients can leave most roots nearly fixed while sending one root on a long excursion.

<p align="center">
  <img src="snaps/polypaint-2026-02-08T13-00-17.png" width="90%" alt="Degree-30 diamond roots — circle path, single coefficient">
</p>

**Degree 30, diamond root shape, circle path** — A single coefficient (c₈) orbiting on a large circle (radius 2.0, speed 0.4). The roots were initialized in a diamond arrangement. On the left, the lone selected coefficient traces one clean circle while the remaining coefficients sit near the origin. On the right, the 30 roots maintain their diamond shape but each traces a smooth rounded-square orbit — the diamond's corners soften into curves as the perturbation sweeps around. The minimal input (one coefficient, one circle) produces a surprisingly coherent collective response: every root moves in concert, preserving the diamond's symmetry while the trail reveals the underlying geometry of the perturbation.

<p align="center">
  <img src="snaps/polypaint-2026-02-08T13-37-45.png" width="90%" alt="Degree-5 multi-path — 6 paths, each coefficient on its own circle">
</p>

**Degree 5, circle pattern, 6 simultaneous paths (multi-path demo)** — Every coefficient (c₀ through c₅) assigned to its own animation path, all circles but with different configurations: c₀ on a large circle (radius 1.4, speed 0.6, CCW), c₄ barely moving (radius 0.5, speed 0.1, CCW), and the rest at radius 0.5, speed 1.0 with alternating CW/CCW directions. On the left, six circles of varying size show each coefficient's individual orbit. On the right, the 5 roots trace complex entangled loops — the interference between six independent perturbations at different frequencies and directions creates an intricate braid that no single-path animation could produce. The loop detection fired after one complete cycle, confirming the combined motion is periodic.

<p align="center">
  <img src="snaps/polypaint-2026-02-08T18-01-14.png" width="90%" alt="Degree-29 grid pattern — 10 paths with mixed shapes and angles">
</p>

**Degree 29, grid pattern, 10 simultaneous paths with mixed shapes and angles** — 10 of 30 coefficients each on their own path with diverse configurations: circles (c₀ at radius 1.4, c₁₃ at 0.5, c₁₇ at 1.0), a star (c₁₄ at radius 0.15), an astroid (c₁₅ at 0.3), and four horizontal oscillations (c₂₀–c₂₃ at varying speeds 0.2–0.8) plus c₅ horizontal at angle 0.96. Each path uses a different angle rotation, tilting the shapes relative to each other. On the left, the always-visible coefficient paths show the full variety — circles, a star, an astroid, and angled horizontal lines radiating from their coefficients across the grid. On the right, the 29 root trails form a dense braid around a large ring, with tight cusps where roots nearly collide and sweeping arcs from the combined interference of all 10 perturbations at different frequencies and orientations.

<p align="center">
  <img src="snaps/polypaint-2026-02-08T18-08-36.png" width="90%" alt="Degree-29 grid pattern — 11 paths, mostly angled horizontals, maximum chaos">
</p>

**Degree 29, grid pattern, 11 paths with mass horizontal perturbation** — Building on the previous snap, now with nearly every coefficient animated. Two group paths dominate: 12 coefficients (c₁–c₄, c₉–c₁₁, c₁₆–c₂₀) on a single horizontal at speed 0.7 with angle 0.73, and 5 coefficients (c₂₄–c₂₈) on a faster horizontal at speed 1.2 with angle 0.29. The remaining 4 solo paths keep their circles, star, and astroid from before. On the left, the angled horizontal paths form a striking fan of parallel lines across the grid — the angle slider tilts each group's oscillation axis differently. On the right, the root trails explode into tangled loops and whorls: with so many coefficients oscillating at different speeds and angles, the roots are pushed far from equilibrium, producing a chaotic braid where almost every root interacts with its neighbors.

## Patterns

### Basic (5)

| Pattern | Description |
|---------|-------------|
| Circle | Evenly spaced on a circle |
| Real axis | Along the real axis |
| Imaginary axis | Along the imaginary axis |
| Grid | Square grid arrangement |
| Random | Uniformly random |

### Coefficient Patterns (8)

| Pattern | Description |
|---------|-------------|
| Spiral | Archimedean spiral |
| Star | Alternating inner/outer radii |
| Figure-8 | Bernoulli lemniscate |
| Conjugate pairs | Pairs straddling the real axis |
| Two clusters | Two separate clusters |
| Geometric decay | Alternating-sign geometric series |
| Rose curve | 3-petal rose r = cos(3θ) |
| Cardioid | Heart-shaped curve |

### Root Shapes (13)

These define the *roots* in a specific shape, then compute the coefficients by expanding the product. Dragging the resulting coefficients perturbs the roots away from the initial shape:

Heart, Circle, Star, Spiral, Cross, Diamond, Chessboard, Smiley, Figure-8, Butterfly, Trefoil, Polygon, Infinity

## File Structure

```
karpo_hackathon/
├── index.html            # Entire app (~2000 lines): CSS, JS, HTML all inline
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
