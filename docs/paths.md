# Paths & Curve Indexing

How coefficients move along their assigned trajectories during animation and fast mode, and how the full-cycle jiggle interval is computed.

## Curve Representation

Each animated coefficient stores a **sampled closed curve** — an array of N complex points `curve[0], curve[1], ..., curve[N−1]` representing the trajectory in the complex plane. The first point `curve[0]` is the coefficient's **home position** (where it sits before animation starts).

The number of sample points N depends on the path type:
- **Standard paths** (circle, horizontal, vertical, random, lissajous, figure-8, cardioid, astroid, deltoid, rose, epitrochoid, hypotrochoid, butterfly, star, square, c-ellipse): N = 200 (interactive) or the Steps dropdown value (10K/50K/100K/1M) in fast mode
- **High-resolution paths** (hilbert, peano, sierpinski, spiral): N = 1500 (interactive) or Steps value in fast mode

Path types are organized in `PATH_CATALOG` into groups: None, Follow C (D-only, `dOnly: true`), Basic (circle, horizontal, vertical, spiral, random/Gaussian cloud), Curves (lissajous, figure-8, cardioid, astroid, deltoid, rose, epitrochoid, hypotrochoid, butterfly, star, square, c-ellipse), and Space-filling (hilbert, peano, sierpinski). The "Follow C" path type is a D-node-only option: the D-node copies the position of its corresponding C-coefficient each frame instead of following its own trajectory. It has no parameters (`PATH_PARAMS["follow-c"] = []`). In fast mode, follow-c D-node indices are serialized as `dFollowCIndices` and each worker step copies the current C-coefficient position into the morph target array for those indices. `buildPathSelect(sel, noneLabel, dNode)` takes a `dNode` flag to include or exclude D-only options — called with `true` for the D-List path selector, omitted for C-List and anim-bar selectors.

### Dithered Variants

Every path type except "none", "follow-c", and "random" has an auto-generated **dithered variant** (e.g. "circle-dither", "hilbert-dither"). At catalog build time, a `(dither)` entry is inserted after each base path in every group. The dithered variant inherits all parameters from its base path plus an additional **sigma** (σ) parameter (0–10, step 0.5, default 2, displayed as `σ%`). When computing curves, the `_ditherSigmaPct` flag is stored on the curve array. During animation, if `_ditherSigmaPct` is set, each interpolated position is perturbed by a Gaussian random offset scaled by `σ / 100 × coeffExtent()`. This creates a noisy/fuzzy version of the base trajectory. The `PATH_PARAMS` auto-generation loop (`for (const key of Object.keys(PATH_PARAMS))`) appends `_SIGMA_PARAM` to the base schema for each dithered key.

The curve is always closed: walking from index 0 through N−1 and wrapping back to 0 traces exactly one complete loop.

## How the Curve Index Changes with Time

The Play, Scrub, and Home controls live in the **header bar** (not in the left tab bar). The scrub slider is **additive**: dragging it adds seconds on top of the current elapsed time (stored in `scrubBase`). On mousedown/touchstart it captures `scrubBase = animState.elapsedAtPause || 0`, then during input computes `elapsed = scrubBase + sliderValue/1000`. On release the slider resets to 0 (the elapsed time is already committed via `advanceToElapsed()`). The Play button cycles through Play → Pause → Resume states.

During animation, time is tracked as `elapsed` — seconds since Play was pressed (real wall-clock time divided by 1000). Each coefficient has three relevant parameters:

- **speed** — how many full loops per second (slider displays 1–1000 as integers; internally stored as 0.001–1.0 via `toUI: v * 1000`, `fromUI: v / 1000`)
- **ccw** — direction flag: false = clockwise, true = counter-clockwise
- **curve** — the N-point sampled trajectory

At each animation frame, the curve index for coefficient *i* is computed as:

```
t = elapsed × speed × direction        (direction = −1 if CCW, +1 if CW)
u = ((t mod 1) + 1) mod 1              (fractional position in [0, 1), always positive)
rawIdx = u × N                          (continuous index into the curve array)
```

For regular curves, the position is **linearly interpolated** between adjacent samples:

```
lo  = floor(rawIdx) mod N
hi  = (lo + 1) mod N
frac = rawIdx − floor(rawIdx)

re = curve[lo].re × (1 − frac) + curve[hi].re × frac
im = curve[lo].im × (1 − frac) + curve[hi].im × frac
```

For random/cloud curves (Gaussian point cloud, flagged with `curve._isCloud`), no interpolation is done — the coefficient snaps to the nearest integer index.

### Orbital paths (spiral, c-ellipse)

Curves flagged with `curve._isOrbital` store **absolute** positions in the complex plane rather than home-relative offsets. The spiral path orbits around the origin (0+0i), spiraling out to a target radius and back. The c-ellipse path traces an ellipse whose vertices are the coefficient's home position and the origin, with a configurable width parameter. Both are regenerated (not transformed) when radius or angle changes.

### What u = 0 means

When `u = 0`, `rawIdx = 0`, so the coefficient is at `curve[0]` — its home position. This happens at `elapsed = 0` (animation start) and every time `elapsed × speed` is an exact integer. That is, each coefficient returns home every `1/speed` seconds.

### Example

A coefficient with speed = 0.50 (displayed as 500) and N = 200:
- At elapsed = 0.0s → t = 0.0, u = 0.0, rawIdx = 0 → at curve[0] (home)
- At elapsed = 0.5s → t = 0.25, u = 0.25, rawIdx = 50 → interpolating near curve[50]
- At elapsed = 1.0s → t = 0.50, u = 0.50, rawIdx = 100 → halfway through the loop
- At elapsed = 2.0s → t = 1.0, u = 0.0, rawIdx = 0 → back at home (one full loop)

Period = 1/speed = 2.0 seconds.

## Fast Mode Passes

In fast mode, time doesn't come from the wall clock. Instead, the Worker loops through the curve in discrete steps:

```
for step = 0 to totalSteps−1:
    elapsed = elapsedOffset + (step / totalSteps) × FAST_PASS_SECONDS
```

where `FAST_PASS_SECONDS = 1.0` second and `totalSteps` is the Steps dropdown value (e.g. 100,000). One complete pass simulates exactly 1.0 second of animation time. After each pass completes, `elapsedOffset` increments by 1.0 and the next pass begins.

So pass 0 covers elapsed ∈ [0, 1), pass 1 covers [1, 2), pass 2 covers [2, 3), etc.

At each step, every animated coefficient's position is computed from `elapsed` using the same formula as interactive mode (with linear interpolation into the high-resolution curve), then the polynomial is solved and roots are painted onto the bitmap canvas as single pixels.

## Full-Cycle Target (Jiggle Interval)

The **full cycle** is the smallest amount of time after which **every animated coefficient simultaneously returns to its home position** (curve index 0). Fast mode does **not** auto-stop — it runs indefinitely until manually paused via the bitmap "pause" button. The full-cycle value is used only by the **jiggle system**: the GCD button in the jiggle popup computes the cycle length and sets `jiggleInterval` so that jiggle perturbations are applied at exact cycle boundaries.

The former `computeFullCyclePasses()` function has been removed. The GCD computation now lives inline in the jiggle popup's GCD button handler.

### Derivation

Coefficient *i* returns home whenever `elapsed × speed_i` is an integer. Its period is:

```
T_i = 1 / speed_i    seconds
```

For **all** coefficients to be home simultaneously, the elapsed time must be a common multiple of every coefficient's period. The earliest such time is the **least common multiple (LCM)** of all periods:

```
T_cycle = LCM(T_1, T_2, ..., T_k)
```

Since each pass covers 1.0 second, the number of passes for a full cycle is `T_cycle / 1.0 = T_cycle`.

### Integer arithmetic

The speed slider displays integers 1–1000 (internally stored as `s_i / 1000`), so every speed can be written as `s_i / 1000` where `s_i` is a positive integer (e.g. speed 500 → s_i = 500, speed 7 → s_i = 7). Then:

```
T_i = 1 / (s_i / 1000) = 1000 / s_i    seconds
```

The LCM of fractions `1000/s_1, 1000/s_2, ...` equals `1000 / GCD(s_1, s_2, ..., s_k)`:

```
passes = T_cycle = 1000 / GCD(s_1, s_2, ..., s_k)
```

This is computed by the GCD button in the jiggle popup, which sets `jiggleInterval` to this value (clamped to 0.1–100).

### Examples

| Coefficients | Speeds | Integer s_i | GCD | Passes | Meaning |
|-------------|--------|-------------|-----|--------|---------|
| 1 coeff | 0.500 | 500 | 500 | 2 | loops twice in 2s |
| 1 coeff | 0.100 | 100 | 100 | 10 | loops once in 10s |
| 2 coeffs | 0.500, 0.100 | 500, 100 | 100 | 10 | fast one loops 5×, slow one loops 1× |
| 2 coeffs | 0.300, 0.200 | 300, 200 | 100 | 10 | first loops 3×, second loops 2× |
| 2 coeffs | 0.007, 0.003 | 7, 3 | 1 | 1000 | first loops 7×, second loops 3× |
| 3 coeffs | 1.000, 0.500, 0.250 | 1000, 500, 250 | 250 | 4 | loops 4×, 2×, 1× respectively |
| 1 coeff | 1.000 | 1000 | 1000 | 1 | single pass = one full loop |

### Edge cases

- **No animated coefficients** (all paths are "none"): The GCD button has no speeds to compute, so `jiggleInterval` is unchanged. Fast mode runs until manually paused via the bitmap "pause" button.
- **Speed = 0**: coefficients with zero speed are skipped in the GCD computation (they never move, so they're always "home").
- **Single speed**: GCD = s, passes = 1000/s. E.g. speed 0.001 (s=1) → 1,000 passes (clamped to 100 by the jiggle interval slider).
- **Coprime speeds**: e.g. 7 and 3 → GCD(7, 3) = 1 → 1000 passes. This is the worst case for small speeds.

### Prime Speed (PS) Button

The **PS** button finds the nearest integer speed (1–1000) that is **coprime** with all other animated coefficients' speeds and also **different** from all of them. This maximizes the full-cycle pass count (`1000 / GCD = 1000 / 1 = 1000` when all speeds are pairwise coprime), ensuring the densest possible bitmap coverage.

`findPrimeSpeed()` searches outward from the current speed (±1, ±2, ...) up to ±1000, clamped to [1, 1000]. `findDPrimeSpeed()` searches the same way but with an expanded range up to ±2000, clamped to [1, 2000], and skips D-nodes with `pathType === "follow-c"`. Since 1 is coprime with everything, both always terminate.

**Note:** The PS button was removed from the trajectory editor (anim-bar), C-List curve editor, and D-List curve editor. It is still available in per-coefficient path picker popups (click the path cell in a C-List or D-List row to open the popup). The PrimeSpeeds transform in the C-List/D-List Transform dropdown is also still available for bulk prime-speed assignment.

### Verification

After exactly `passes` passes, every coefficient's elapsed time is `passes` seconds. For coefficient *i*:

```
elapsed × speed_i = passes × (s_i / 1000) = (1000 / GCD) × (s_i / 1000) = s_i / GCD
```

Since GCD divides every s_i by definition, `s_i / GCD` is always an integer. Therefore `u = 0` and `rawIdx = 0` — every coefficient is at `curve[0]`, its home position. The bitmap contains every root position from one complete cycle with no overlap or gap.

## Jiggle — Path Perturbation Between Cycles

The **jiggle** button on the Bitmap tab opens a popup for perturbing coefficient trajectory home positions between fast-mode cycles. All modes produce the same `Map<coeffIdx, {re, im}>` additive offsets consumed by `enterFastMode()` — only the generation strategy differs.

### Architecture

When fast mode generates hi-res curves, each animated coefficient's home position is shifted by its jiggle offset:

```
homeRe += offset.re
homeIm += offset.im
```

The curve shape and parameters (path type, radius, angle, speed) are unchanged — only the center point shifts. Offsets are recomputed between cycles (via Generate or OnTarget auto-trigger).

### Modes

The popup has a **Mode** dropdown with 10 options:

#### None (default)
No perturbation. Generate is a no-op. Use this when you want jiggle disabled without having to remember parameter values.

#### Random
Fresh Gaussian offsets each trigger. Each trigger replaces all offsets independently.

- **Control**: σ (0–100) — standard deviation as % of `coeffExtent()`
- **Formula**: `offset = { re: gaussRand() × σ_abs, im: gaussRand() × σ_abs }` where `σ_abs = (σ / 100) × coeffExtent()`
- Uses Box-Muller transform for proper Gaussian distribution
- Same reference space as the radius slider (`absR = (c.radius / 100) × coeffExtent()`)

| σ | Effect |
|---|--------|
| 1 | ~1% of coefficient spread — subtle thickening of bitmap paths |
| 10 | ~10% — visible smearing, paths overlap |
| 50 | ~50% — dramatic perturbation, root patterns significantly altered |
| 100 | ~100% — extreme perturbation |

#### Rotate
Cumulative rotation of all animated coefficient home positions around their centroid by a fixed angle step.

- **Control**: θ (0.001–0.500 turns), displayed with degree equivalent
- **State**: `jiggleCumulativeAngle` — accumulated angle in radians, incremented by `θ × 2π` each trigger
- **Formula**:
  ```
  centroid = mean of all animated coefficients' home positions
  for each coefficient:
    dx, dy = home − centroid
    offset.re = dx×cos(angle) − dy×sin(angle) − dx
    offset.im = dx×sin(angle) + dy×cos(angle) − dy
  ```
- Deterministic — produces a systematic angular sweep through coefficient space
- With OnTarget, each cycle rotates further, painting a rosette of overlapping root patterns

#### Walk
Cumulative random walk — each trigger adds a small random step to the current offsets instead of replacing them.

- **Control**: σ (0–100) — step size as % of `coeffExtent()`
- **Formula**: `offset.re += gaussRand() × σ_abs`, `offset.im += gaussRand() × σ_abs`
- If no offsets exist yet, creates fresh offsets like Random mode
- Produces spatially correlated drift — nearby cycles paint nearby perturbations
- With OnTarget, the bitmap accumulates a Brownian-motion exploration of coefficient space

#### Scale
Cumulative scaling of all animated coefficient home positions around their centroid.

- **Control**: step (1–50) — percent scale per trigger
- **State**: `jiggleCumulativeScale` — accumulated scale factor, multiplied by `(1 + step/100)` each trigger
- **Formula**:
  ```
  centroid = mean of all animated coefficients' home positions
  for each coefficient:
    dx, dy = home − centroid
    offset = { re: dx × (scale − 1), im: dy × (scale − 1) }
  ```
- With OnTarget, coefficients spread further apart each cycle, producing radial density patterns

#### Circle
Cumulative rotation of all coefficient positions around the **origin** (not the centroid). Each coefficient's home position is multiplied by e^(i·θ), effectively rotating the entire polynomial.

- **Control**: θ (0.001–0.500 turns)
- **State**: `jiggleCumulativeAngle` — accumulated angle, incremented by `θ × 2π` each trigger
- **Formula**:
  ```
  cos_a, sin_a = cos/sin(cumAngle)
  offset.re = hre × (cos_a − 1) − him × sin_a
  offset.im = hre × sin_a + him × (cos_a − 1)
  ```
- Unlike Rotate (which rotates around the centroid), Circle rotates around the origin — this changes the polynomial's coefficient magnitudes, not just their relative arrangement

#### Spiral
Combined cumulative rotation + scaling around the centroid. Combines the effects of Rotate and Scale.

- **Controls**: θ (turns) + step (% scale per trigger)
- **State**: `jiggleCumulativeAngle` + `jiggleCumulativeScale`
- **Formula**: Applies both scale factor and rotation matrix to `(home − centroid)` offsets
- With OnTarget, produces a logarithmic spiral exploration of coefficient space

#### Breathe
Sinusoidal scaling from the centroid — coefficients expand and contract periodically.

- **Controls**: Amplitude (0–100, as % of centroid distance), Period (triggers per oscillation)
- **Formula**: `scale = 1 + (amplitude/100) × sin(2π × t / period)` applied to centroid-relative offsets
- Produces a pulsing/breathing effect. After one full period, offsets return near zero.

#### Wobble
Sinusoidal rotation around the centroid — coefficients oscillate angularly.

- **Controls**: θ (max rotation in turns), Period (triggers per oscillation)
- **Formula**: `angle = θ × 2π × sin(2π × t / period)` applied as rotation around centroid
- Produces a rocking/wobbling effect. The rotation swings back and forth within ±θ turns.

#### Lissajous
Uniform translation along a Lissajous figure — all coefficients shift by the same offset.

- **Controls**: Amplitude (0–100, as % of `coeffExtent()`), Period, FreqX, FreqY
- **Formula**:
  ```
  dx = amp × sin(2π × freqX × t / period)
  dy = amp × sin(2π × freqY × t / period)
  ```
  All coefficients get the same `{re: dx, im: dy}` offset.
- Default frequencies 1:2 produce a classic figure-8 scan through coefficient space.

### Common Controls

- **Generate**: Computes new offsets using the current mode. Plays a ping on success, a buzz when mode is None or no animated coefficients exist.
- **Clear**: Removes all offsets and resets cumulative state (`jiggleCumulativeAngle = 0`, `jiggleCumulativeScale = 1.0`).
- **OnTarget**: When checked, automatically calls Generate each time a full cycle completes — fast mode exits, generates new offsets, and re-enters fast mode seamlessly.

### Persistence

Mode, σ, θ, and scale step are saved/loaded with the project state. Cumulative state (angle, scale factor) and active offsets are transient — they reset on load.

## Space-Filling Curve Paths

Three space-filling curves are available as animation paths, all implemented via L-system turtle graphics with caching:

- **Hilbert (Moore curve):** Closed variant of the Hilbert curve -- 4 Hilbert sub-curves arranged in a loop. Order 4, 256 points. Fills a square with uniform step sizes. L-system: `LFL+F+LFL`, `L -> -RF+LFL+FR-`, `R -> +LF-RFR-FL+`.
- **Peano:** Classic Peano space-filling curve. Order 3, 729 points, out-and-back traversal for closure (1458 steps total). L-system: `L`, `L -> LFRFL-F-RFLFR+F+LFRFL`, `R -> RFLFR+F+LFRFL-F-RFLFR`.
- **Sierpinski arrowhead:** Fills a Sierpinski triangle. Order 5, 243 segments, out-and-back for closure (486 steps total). L-system: `A -> B-A-B`, `B -> A+B+A` with 60-degree turns.

All three generate perfectly uniform step sizes and are cached on first use.

## C-List Tab Columns

The C-List tab shows a row per coefficient with the following elements (built by `refreshCoeffList()`):

| Element | Content | Updates |
|---------|---------|---------|
| **Checkbox** | Selection toggle (`.cpick-cb`) | On click |
| **Color dot** | Coefficient color from `coeffColor(i, n)` | Static |
| **Sensitivity dot** | Derivative sensitivity color from `sensitivityColor(coeffSens[i])` | On rebuild |
| **Label** | Subscript label (c₀, c₁, ...) where subscript = degree − index | Static |
| **Power** | Monomial term (1, z, z², ...) | Static |
| **Path** | Button showing path type name or "—" for none; click opens path picker popup | On path change |
| **Speed** | Speed value (1–1000 display) or "—" if none | On path change |
| **Radius** | Path radius (0–100) or "—" if none | On path change |
| **Pts** | `curve.length` — sample points in the trajectory (200 or 1500 for interactive; unrelated to the fast-mode Steps dropdown) | On path change |
| **Pos** | `c.curveIndex` — the integer curve index last set during animation (floor of rawIdx). Sweeps 0 → N−1 → 0 during animation. | Every frame via `updateListCoords()` |
| **Coords** | Complex coordinates (re ± im·i) | Every frame via `updateListCoords()` |
