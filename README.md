# PolyPaint

**[Try it live](https://nassuphis.github.io/karpo_hackathon/)**

Interactive polynomial root visualizer. Drag coefficients *or* roots in the complex plane and watch everything update in real time.

## What It Does

A degree-*n* polynomial p(z) = c₀zⁿ + c₁zⁿ⁻¹ + ··· + cₙ has *n*+1 complex coefficients and *n* complex roots. The relationship between them is rich, nonlinear, and often surprising — small changes to one coefficient can send roots flying across the plane, while other perturbations barely move them.

PolyPaint makes this relationship tangible. Two side-by-side complex-plane panels let you explore it from both directions:

- **Left panel (Coefficients):** Drag any coefficient dot and watch the roots respond instantly on the right. The domain coloring background shifts in real time, revealing how the polynomial's complex landscape reshapes.
- **Right panel (Roots):** Drag any root dot and the coefficients on the left update to match — the polynomial is reconstructed from its roots via (z − r₀)(z − r₁)···(z − rₙ₋₁).
- **Multi-select:** Click multiple coefficients (or roots) to select a group. Drag any member and the whole group moves together, maintaining their relative positions.
- **Animate:** Select one or more coefficients, switch to Loop mode, and watch their centroid orbit along a path (circle, figure-8, spiral, etc.) while the roots dance in response. Each coefficient maintains its offset from the group center.

Everything runs client-side in a single HTML file. No server, no build step, no dependencies to install.

## Quick Start

Open [`index.html`](index.html) in any modern browser. That's it.

Or visit the **[live demo](https://nassuphis.github.io/karpo_hackathon/)**.

## Architecture

```
Single HTML file (~1200 lines)
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

When enabled (on by default), the roots panel background is painted using [domain coloring](https://en.wikipedia.org/wiki/Domain_coloring). For each pixel at position z in the complex plane, the polynomial p(z) is evaluated and mapped to a color:

- **Hue** = arg(p(z)) — the phase of the polynomial's value. Roots appear as points where all colors converge (all phases meet at a zero).
- **Lightness** = 0.5 + 0.4·cos(2π·frac(log₂|p(z)|)) — contour lines at powers-of-2 modulus. Zeros appear as dark points.
- **Saturation** = 0.8 fixed.

The polynomial is evaluated via Horner's method. The canvas renders at half resolution with `devicePixelRatio` support and is CSS-scaled to full size, keeping it smooth at 60fps even at degree 30.

## Controls

| Control | Description |
|---------|-------------|
| **Degree slider** (3–30) | Number of polynomial roots. Reinitializes coefficients on change. |
| **Pattern dropdown** | Initial arrangement of coefficients or roots. 25 patterns in 3 categories. |
| **Spread slider** (0.2–2.5) | Scales the initial pattern size. |
| **Domain coloring** checkbox | Toggles the domain coloring background on the roots panel. |
| **Deselect all** button | Clears all coefficient and root selections. |

### Multi-Select and Group Drag

Click any coefficient or root dot to toggle it into the selection. Click again to deselect. Selected items glow to indicate membership. Drag any selected item and the entire group translates together — relative positions are preserved.

Clicking a coefficient clears any root selection and vice versa, so you work with one panel at a time.

### Coefficient Animation

Select one or more coefficients — a translucent control overlay appears on the coefficient panel:

- **Drag mode** (default): normal drag behavior (group drag if multiple selected)
- **Loop mode**: the group's centroid follows a pre-programmed path, each coefficient maintaining its offset
  - 6 paths: Circle, Horizontal, Vertical, Spiral, Figure-8, Random walk
  - Adjustable radius and speed
  - Play/Pause control

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
├── index.html            # Entire app (~1200 lines): CSS, JS, HTML all inline
└── README.md
```

## Technical Notes

### Edge Cases Handled

- **Leading coefficient at origin**: near-zero leading coefficients are stripped before solving
- **NaN/Inf roots**: filtered out before rendering
- **Window resize**: panels dynamically resize, solver re-runs
- **Degree change**: coefficients reinitialized, both panels reset

### Performance

- Root solving throttled via `requestAnimationFrame` (~60fps cap)
- Domain coloring rendered to half-resolution canvas, CSS-scaled with `devicePixelRatio` support
- No d3 transitions on dots — positions update instantly to avoid animation conflicts during rapid drag
- Warm-started Ehrlich-Aberth typically converges in 1–3 iterations during interactive drag
