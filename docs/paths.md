# Paths & Curve Indexing

How coefficients move along their assigned trajectories during animation and fast mode, and how the full-cycle auto-stop target is computed.

## Curve Representation

Each animated coefficient stores a **sampled closed curve** — an array of N complex points `curve[0], curve[1], ..., curve[N−1]` representing the trajectory in the complex plane. The first point `curve[0]` is the coefficient's **home position** (where it sits before animation starts).

The number of sample points N depends on the path type:
- **Standard paths** (circle, figure-8, cardioid, etc.): N = 200 (interactive) or the Steps dropdown value (10K/50K/100K/1M) in fast mode
- **High-resolution paths** (hilbert, peano, sierpinski, spiral): N = 1500 (interactive) or Steps value in fast mode

The curve is always closed: walking from index 0 through N−1 and wrapping back to 0 traces exactly one complete loop.

## How the Curve Index Changes with Time

During animation, time is tracked as `elapsed` — seconds since Play was pressed (real wall-clock time divided by 1000). Each coefficient has three relevant parameters:

- **speed** — how many full loops per second (slider displays 1–100 as integers; internally stored as 0.01–1.0)
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

For random/cloud curves (Gaussian point cloud), no interpolation is done — the coefficient snaps to the nearest integer index.

### What u = 0 means

When `u = 0`, `rawIdx = 0`, so the coefficient is at `curve[0]` — its home position. This happens at `elapsed = 0` (animation start) and every time `elapsed × speed` is an exact integer. That is, each coefficient returns home every `1/speed` seconds.

### Example

A coefficient with speed = 0.50 and N = 200:
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

## Full-Cycle Target (Auto-Stop)

The **full cycle** is the smallest amount of time after which **every animated coefficient simultaneously returns to its home position** (curve index 0). Fast mode auto-stops after exactly this many passes.

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

The speed slider displays integers 1–100 (internally stored as `s_i / 100`), so every speed can be written as `s_i / 100` where `s_i` is a positive integer (e.g. speed 50 → s_i = 50, speed 7 → s_i = 7). Then:

```
T_i = 1 / (s_i / 100) = 100 / s_i    seconds
```

The LCM of fractions `100/s_1, 100/s_2, ...` equals `100 / GCD(s_1, s_2, ..., s_k)`:

```
passes = T_cycle = 100 / GCD(s_1, s_2, ..., s_k)
```

This is computed by `computeFullCyclePasses()`.

### Examples

| Coefficients | Speeds | Integer s_i | GCD | Passes | Meaning |
|-------------|--------|-------------|-----|--------|---------|
| 1 coeff | 0.50 | 50 | 50 | 2 | loops twice in 2s |
| 1 coeff | 0.10 | 10 | 10 | 10 | loops once in 10s |
| 2 coeffs | 0.50, 0.10 | 50, 10 | 10 | 10 | fast one loops 5×, slow one loops 1× |
| 2 coeffs | 0.30, 0.20 | 30, 20 | 10 | 10 | first loops 3×, second loops 2× |
| 2 coeffs | 0.07, 0.03 | 7, 3 | 1 | 100 | first loops 7×, second loops 3× |
| 3 coeffs | 1.00, 0.50, 0.25 | 100, 50, 25 | 25 | 4 | loops 4×, 2×, 1× respectively |
| 1 coeff | 1.00 | 100 | 100 | 1 | single pass = one full loop |

### Edge cases

- **No animated coefficients** (all paths are "none"): `computeFullCyclePasses()` returns 0, meaning unlimited — fast mode runs until manually stopped via "imode".
- **Speed = 0**: coefficients with zero speed are skipped in the GCD computation (they never move, so they're always "home").
- **Single speed**: GCD = s, passes = 100/s. E.g. speed 0.01 → 10,000 passes.
- **Coprime speeds**: e.g. 7 and 3 → GCD(7, 3) = 1 → 100 passes. This is the worst case for small speeds.

### Prime Speed (PS) Button

The **PS** button in the trajectory editor finds the nearest integer speed (1–100) that is **coprime** with all other animated coefficients' speeds and also **different** from all of them. This maximizes the full-cycle pass count (`100 / GCD = 100 / 1 = 100` when all speeds are pairwise coprime), ensuring the densest possible bitmap coverage.

The search starts at the current speed and radiates outward (±1, ±2, ...) until it finds a valid candidate. Since 1 is coprime with everything, it always terminates.

### Verification

After exactly `passes` passes, every coefficient's elapsed time is `passes` seconds. For coefficient *i*:

```
elapsed × speed_i = passes × (s_i / 100) = (100 / GCD) × (s_i / 100) = s_i / GCD
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

## List Tab Columns

The List tab shows a table with per-coefficient data:

| Column | Content | Updates |
|--------|---------|---------|
| **Index** | Color dot + subscript label (c₀, c₁, ...) | Static |
| **Position** | Complex coordinates (re, im) | Every frame during animation |
| **spd** | Speed value (1–100 display) | On path change |
| **rad** | Path radius (0–100) or "—" if pathType is "none" | On path change |
| **pts** | `curve.length` — number of sample points in the trajectory. For interactive mode this is 200 or 1500; unrelated to the Steps dropdown (which only applies to fast mode via `computeCurveN`). | On path change |
| **pos** | `nearestCurveIndex(c)` — brute-force search for which curve sample point is closest to the coefficient's current position. Sweeps 0 → N−1 → 0 during animation. | Every frame during animation |
