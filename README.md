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
- **Sonify:** Three independent sound layers — **Base** (theremin drone), **Melody** (pentatonic arpeggiator), and **Voice** (close-encounter beeps) — each with its own sidebar button and config popover. Click any button to open a panel of tuning sliders (pitch, brightness, volume, rate, etc.) that reshape the sound in real time. See [Sonification](#sonification) for the full algorithm.

Everything runs client-side in a single HTML file. No server, no build step, no dependencies to install.

## Quick Start

Open [`index.html`](index.html) in any modern browser. That's it.

Or visit the **[live demo](https://nassuphis.github.io/karpo_hackathon/)**.

## Architecture

```
Single HTML file (~3500 lines)
├── d3.js v7 (CDN)          — SVG rendering, drag interactions
├── Ehrlich-Aberth solver    — polynomial root finding in pure JS
├── Horner evaluator         — domain coloring + derivative computation
├── Canvas 2D API            — real-time domain coloring
└── Web Audio API            — sonification of root motion
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

When enabled (off by default, toggle via the ◐ sidebar button), the roots panel background is painted using [domain coloring](https://en.wikipedia.org/wiki/Domain_coloring). For each pixel at position z in the complex plane, the polynomial p(z) is evaluated and mapped to a color:

- **Hue** = arg(p(z)) — the phase of the polynomial's value. Roots appear as points where all colors converge (all phases meet at a zero).
- **Lightness** = 0.5 + 0.4·cos(2π·frac(log₂|p(z)|)) — contour lines at powers-of-2 modulus. Zeros appear as dark points.
- **Saturation** = 0.8 fixed.

The polynomial is evaluated via Horner's method. The canvas renders at half resolution with `devicePixelRatio` support and is CSS-scaled to full size, keeping it smooth at 60fps even at degree 30.

## Sonification

Three independent instrument layers — **B** (Base), **M** (Melody), **V** (Voice) — can be toggled individually via sidebar buttons. Each button opens a config popover with an on/off toggle and tuning sliders that modify the sound in real time. Sound is generated whenever roots update — both during animation playback and during interactive drag of coefficients or roots.

### Audio Graph

```
[osc1: sine 110Hz] ──┐                                                    ┌──► speakers
                      ├──► [gainNode] ──► [lowpass filter] ──► [masterGain]┤
[osc2: triangle ~111Hz] ──┘                                                └──► [mediaDest] ──► recording

[beepOsc: sine] ──► [beepGain] ──► [masterGain]

[arpOsc: triangle] ──► [arpGain] ──► [arpFilter: lowpass] ──► [masterGain]

[lfo: sine 1.5–7.5Hz] ──► [lfoGain] ──► osc1.frequency + osc2.frequency
```

Two main oscillators (sine + slightly detuned triangle at ×1.012) produce a thick, beating theremin tone. They pass through a gain stage, then a lowpass filter whose cutoff tracks the root constellation's spread and kinetic energy, then a master gain node that acts as the definitive gate. A separate beep oscillator, gated by its own gain envelope, handles close encounter events. An arpeggiator oscillator (triangle wave) cycles through roots at 24 notes/sec, mapping each root's angle to a pentatonic pitch and radius to an octave, with pluck-style envelopes gated by per-root velocity. An LFO provides vibrato on both main oscillators, with both rate and depth modulated by the root distribution.

### Instrument Config Popovers

Each instrument button (**B**, **M**, **V**) opens a popover with an on/off toggle and tuning sliders. All parameters take effect immediately — drag a slider during animation and hear the change in real time.

**Base** (6 sliders):

| Slider | Range | Default | Controls |
|--------|-------|---------|----------|
| Pitch | 55–440 Hz | 110 Hz | Center frequency of the drone |
| Range | 0.5–4.0 oct | 2.0 | How many octaves the pitch swings with root spread |
| Detune | 1.000–1.050 | 1.012 | Frequency ratio between the two oscillators (beating) |
| Bright | 50–1000 Hz | 250 Hz | Filter cutoff floor (higher = brighter at rest) |
| Volume | 0.05–0.50 | 0.22 | Gain swing driven by kinetic energy |
| Vibrato | 0–25 Hz | 10 Hz | LFO depth driven by angular coherence |

**Melody** (5 sliders):

| Slider | Range | Default | Controls |
|--------|-------|---------|----------|
| Rate | 2–60 /s | 24 /s | Arpeggiator step speed (notes per second) |
| Volume | 0.02–0.30 | 0.12 | Peak note gain |
| Attack | 1–20 ms | 4 ms | Pluck attack time |
| Decay | 10–200 ms | 64 ms | Pluck decay time |
| Bright | 200–4000 Hz | 1200 Hz | Filter cutoff floor |

**Voice** (5 sliders):

| Slider | Range | Default | Controls |
|--------|-------|---------|----------|
| Memory | 1.0001–1.010 | 1.001 | Record decay rate (higher = records expire faster, more beeps) |
| Cooldown | 10–500 ms | 80 ms | Minimum gap between beeps |
| Volume | 0.02–0.30 | 0.12 | Beep peak gain |
| Attack | 1–20 ms | 5 ms | Beep attack time |
| Decay | 10–300 ms | 80 ms | Beep ring-down time |

### Feature Extraction

Six features are extracted from `currentRoots` each frame, using distribution statistics rather than simple averages. This ensures the sound responds to the *shape* of the root constellation, not just its center of mass.

| Feature | Formula | Maps to |
|---------|---------|---------|
| **Median radius** (`r50`) | 50th percentile of distances from centroid | Oscillator pitch |
| **Spread** (`r90 − r10`) | 90th minus 10th percentile of radii | Filter cutoff (brightness) |
| **Energy med** (`E_med`) | 50th percentile of per-root velocities | Filter cutoff boost, LFO speed |
| **Energy hi** (`E_hi`) | 85th percentile of per-root velocities | Gain (loudness) |
| **Angular coherence** (`R`) | circular mean resultant length of root angles | LFO depth (vibrato) |
| **Close encounters** | per-root top-3 closest distances ever seen | Beep on record-breaking approach |

**Radius distribution:** Each root's distance from the centroid is computed, sorted, and sampled at the 10th, 50th, and 90th percentiles using linear interpolation. The spread `r90 − r10` captures how "inflated" or "collapsed" the constellation is, independent of where its center sits.

**Dual energy:** Per-root velocities (Euclidean frame-to-frame displacement) are sorted and sampled at two percentiles. The 50th percentile (`E_med`) provides a stable, noise-resistant measure; the 85th percentile (`E_hi`) captures when *some* roots go wild even if the majority are calm. This split prevents the median from flattening dynamics.

**Angular coherence:** Instead of mean angular velocity (which cancels when roots rotate in opposite directions), we compute the [circular mean resultant length](https://en.wikipedia.org/wiki/Directional_statistics#Mean_resultant_length): `R = |mean(e^{iθ})|`. R = 1 when all roots cluster at the same angle (a "clump"); R ≈ 0 when angles are uniformly distributed (a ring). This gives structural information even when roots aren't moving.

All features are smoothed with a one-pole exponential filter before mapping to audio parameters:

```
smoothed += α × (raw − smoothed),  α = 0.1
```

This prevents audible discontinuities from frame-to-frame noise while remaining responsive enough to track real motion.

### Sound Mapping

**Pitch (theremin):**
```
frequency = 110 × 2^((r50_norm − 0.5) × 2.0)
```
where `r50_norm = clamp(r50 / panel_range, 0, 1)`. The median radius maps to ±1 octave around A2 (110 Hz). When the root constellation expands, pitch rises; when it contracts, pitch falls. This is more stable than centroid-based pitch because the median radius resists centroid drift. The second oscillator tracks at `frequency × 1.012` for a slow beating effect.

**Filter cutoff (brightness):**
```
cutoff = 250 + 3500 × spread_norm + 1500 × E_med_norm
```
Clamped to 150–8000 Hz, Q = 2. The constellation's spread opens the filter (wider cloud = brighter tone), and median kinetic energy adds further brightness. When roots are tightly clustered and still, only the fundamental comes through; when they're spread out and active, upper harmonics emerge.

**Gain (loudness):**
```
gain = 0.03 + 0.22 × E_hi_smoothed
```
where `E_hi_norm = clamp(E_hi / (range × 0.05), 0, 1)`. The 85th-percentile energy drives loudness — when even a few roots are moving fast, you hear it. The small floor (0.03) ensures the drone doesn't completely vanish during slow structural rearrangements. The smoothing filter provides a natural fade-out when motion stops, and the watchdog timer (see Silence Management) handles the final fade to true silence.

**Vibrato (coherence):**
```
lfo_depth = 2 + 10 × R_smoothed  Hz
lfo_rate  = 1.5 + 6.0 × E_med_norm  Hz
```
The LFO modulates both oscillators' frequencies. When roots cluster angularly (high R), vibrato depth increases up to 12 Hz — the sound "wobbles" as the clump moves. When roots form a balanced ring (low R), vibrato settles to a gentle 2 Hz baseline. Additionally, the LFO *rate* itself increases with median energy: calm scenes get slow vibrato (1.5 Hz), active scenes get faster pulsing (up to 7.5 Hz).

**Close encounter beeps (novelty-based):**

Instead of a fixed or adaptive threshold, each root tracks its own **top 3 closest distances** ever observed to any neighbor. A beep fires only when a root **beats one of its own records** — a genuinely unusual close approach for that specific root.

Each root's encounter table is seeded with current distances on the first frame (no startup burst). On subsequent frames:
1. Each root finds its 3 closest neighbors via partial sort
2. If any distance is smaller than the root's worst record, the record is replaced
3. The root with the most dramatic improvement fires a beep at its own **pentatonic pitch** (root 0 = C4, root 1 = D4, ..., ascending through octaves via the scale [C, D, E, G, A])
4. All records slowly **decay** (`×1.001` per frame, ~6%/sec at 60fps), so old records gradually become beatable again

```
pitch = midiToHz(60 + pentatonic[i % 5] + 12 × floor(i / 5))
peak  = 0.05 + 0.12 × clamp(improvement / (range × 0.05), 0, 1)
envelope: 0.0001 → peak (5ms attack) → 0.0001 (80ms decay)
cooldown: 80ms between beeps
```

This approach is inherently adaptive: tight configurations set low records early, so only truly exceptional approaches trigger; loose configurations keep records high, so moderate approaches still register. No threshold constants to tune.

### Silence Management

A **watchdog timer** runs via `setInterval` (100ms) while sound is enabled. If `updateAudio()` has not been called for 150ms (i.e., no root updates from animation or drag), the watchdog fades `masterGain` to zero over ~150ms. Using `setInterval` instead of `requestAnimationFrame` ensures the watchdog keeps running even when the browser throttles animation in background tabs.

A **visibilitychange** listener immediately ramps `masterGain` to zero (20ms time constant) when the page becomes hidden, preventing orphaned audio when the user switches tabs.

Additionally, `resetAudioState()` is called on: animation stop, Home button, pattern change, degree change, and sound toggle off. It zeroes all smoothing accumulators and ramps `masterGain` to zero, ensuring clean silence in all state transitions.

## Root Braids and Monodromy

When you animate a coefficient along a closed loop, the roots don't just wiggle — they trace out a **braid**. This is a topological phenomenon with deep mathematical roots (pun intended).

The space of degree-*n* polynomials with distinct roots is topologically the [configuration space](https://en.wikipedia.org/wiki/Configuration_space_(mathematics)) of *n* unordered points in **C**. Its fundamental group is the [braid group](https://en.wikipedia.org/wiki/Braid_group) B_n. A closed loop in coefficient space induces a **monodromy permutation** on the roots — after one full cycle, root #3 might now occupy the position that root #7 had before.

This is not a solver artifact; it is a topological invariant of the loop. Different loops around different "holes" in coefficient space produce different permutations. The [cohomology](https://en.wikipedia.org/wiki/Cohomology) of the configuration space (computed by Arnol'd and Cohen) classifies these possibilities.

**What you see in PolyPaint:** the trail patterns are visual braids. Root identity is preserved across frames using a greedy nearest-neighbor assignment (`matchRootOrder`), which reorders each new solver output to best match the previous frame's root positions. This keeps trails tracking the same root continuously. When roots still swap indices (e.g. during fast near-collisions), jump detection breaks the path rather than drawing a false connecting line.

## Interface

The UI is organized around a left sidebar with three groups and a compact header:

**Header:** App title, clickable **Degree** label (click to open slider, range 3–30), and **Pattern** dropdown.

**Sidebar — View:** ◐ Domain coloring toggle, 🎨 Root coloring toggle, **B** Base / **M** Melody / **V** Voice sound toggles (each opens a config popover).

**Sidebar — Tools:** ✕ Deselect all, ⬇ Export snapshot.

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
| **⏺** record (roots header) | Records to WebM video. Mode selector: Roots, Coefficients, or Both (side-by-side). Auto-stops on loop completion. |
| **⌂ Home** button | Returns all animated coefficients to their start positions (curve[0]) — resets the animation clock without changing path shapes. |
| **B / M / V** sound buttons | Toggle and configure the three sound layers. Click to open config popover with on/off toggle + tuning sliders. See [Sonification](#sonification). |
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

<p align="center">
  <img src="snaps/polypaint-2026-02-08T18-50-52.png" width="90%" alt="Degree-5 circle pattern — Peano space-filling path, single coefficient">
</p>

**Degree 5, circle pattern, Peano space-filling path** — A single coefficient (c₁) tracing a Peano curve at radius 0.75, speed 0.1. On the left, the coefficient's path reveals the Peano curve's characteristic zigzag pattern — dense, space-filling, covering the full square around the coefficient. On the right, each of the 5 roots responds by tracing its own miniature space-filling shape: the Peano structure propagates through the polynomial, producing self-similar fractal-like trails at each root position. The slow speed (0.1) allows the trails to accumulate cleanly, showing the full one-cycle braid. A striking demonstration of how a single space-filling perturbation on one coefficient induces fractal geometry across all roots.

<p align="center">
  <img src="snaps/polypaint-2026-02-08T21-58-35.png" width="90%" alt="Degree-20 two-clusters — dual Hilbert paths, all coefficients animated">
</p>

**Degree 20, two-clusters pattern, dual Hilbert (Moore curve) paths** — All 21 coefficients animated across two Hilbert paths at radius 0.65, speed 0.1, with complementary angles (0.75 and 0.25). One path drives 10 coefficients from one cluster, the other drives 11 from the second cluster. On the left, the overlapping Hilbert curves form a dense, layered maze — the two angle rotations tilt the space-filling grids against each other, creating a moiré-like interference pattern. On the right, the 20 root trails each trace their own miniature Hilbert-like fractal, arranged in a large ring. Every root responds to the combined space-filling motion of both coefficient groups, producing intricate self-similar shapes at each root position — some tightly wound, others more open depending on proximity to the coefficient clusters.

<p align="center">
  <img src="snaps/polypaint-2026-02-09T07-33-41.png" width="90%" alt="Degree-28 diamond roots — dual circle paths, web-like trail mesh">
</p>

**Degree 28, diamond root shape, dual circle paths at different speeds** — Two coefficients animated on independent circles: c₄ at radius 1.2, speed 0.6 (CW, angle 0.5) and c₀ at radius 1.2, speed 0.1 (CCW, angle 1.0). The roots were initialized in a diamond arrangement. On the left, the two coefficient paths — one large offset circle (c₀ near the bottom-left) and one near the cluster at center — show the asymmetric perturbation. On the right, the 28 roots maintain a large ring with the diamond's characteristic spacing, but the trails weave an intricate web of fine mesh-like filaments connecting neighboring roots. The speed ratio (6:1) between the two paths creates a dense Lissajous-like interference: the fast path drives rapid oscillations while the slow path modulates the overall envelope, producing a cage-like lattice structure around the ring.

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
├── index.html            # Entire app (~3500 lines): CSS, JS, HTML all inline
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
