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

## Jiggle — Stochastic Path Perturbation

The **jiggle** button on the Bitmap tab opens a popup for applying random per-coefficient offsets to trajectory home positions during fast mode. This produces stochastically perturbed bitmaps — each cycle paints a slightly different version of the root pattern, building up density over many cycles.

### How it works

Each animated coefficient gets an independent random offset `{δre, δim}` drawn from `N(0, σ)` (Box-Muller transform). The σ parameter is an integer 0–100 representing a percentage of `coeffExtent()` (the max pairwise distance between coefficients at their home positions):

```
σ_absolute = (jiggleSigma / 100) × coeffExtent()
δre = gaussRand() × σ_absolute
δim = gaussRand() × σ_absolute
```

This uses the same reference space as the radius slider (`absR = (c.radius / 100) × coeffExtent()`).

When fast mode generates hi-res curves, the offset is added to the coefficient's home position:

```
homeRe += δre
homeIm += δim
```

The curve shape and parameters (path type, radius, angle, speed) are unchanged — only the center point shifts.

### Controls

- **σ** (0–100): Standard deviation as % of coefficient extent. 0 = no perturbation, 10 = moderate, 100 = extreme.
- **Generate**: Creates new random offsets for all animated coefficients.
- **Clear**: Removes all offsets (reverts to exact trajectories).
- **OnTarget**: When checked, automatically regenerates offsets each time a full cycle completes — fast mode exits, generates new offsets, and re-enters fast mode seamlessly. This builds up stochastic density over many cycles without manual intervention.

### Typical σ values

| σ | Effect |
|---|--------|
| 1 | ~1% of coefficient spread — subtle thickening of bitmap paths |
| 10 | ~10% — visible smearing, paths overlap |
| 50 | ~50% — dramatic perturbation, root patterns significantly altered |
| 100 | ~100% — extreme perturbation |

## The "pos" Column in the List Tab

The List tab shows a **pos** column for each coefficient. This displays `nearestCurveIndex(c)` — a brute-force search that finds which curve sample point is closest to the coefficient's current position. During animation it updates every frame, showing the coefficient's approximate index as it sweeps through 0 → N−1 → 0. When paused, it shows where on the curve the coefficient currently sits.

## The "pts" Column

The **pts** column shows `curve.length` — the number of sample points in the coefficient's trajectory. For interactive mode this is 200 or 1500; it has no direct relationship to the Steps dropdown (which only applies to fast mode curve generation via `computeCurveN`).
