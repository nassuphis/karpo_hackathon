# Paths & Curve Indexing

How coefficients (C-nodes) and morph targets (D-nodes) move along their assigned trajectories during animation and fast mode, how the full-cycle jiggle interval is computed, and how trajectory editors work.

## Data Model

Each coefficient (C-node) and morph target (D-node) stores the following path-related fields:

| Field | Type | Description |
|-------|------|-------------|
| `pathType` | string | Path type key (e.g. `"none"`, `"circle"`, `"spiral-dither"`, `"follow-c"`) |
| `radius` | number | Path radius as percentage of `coeffExtent()` (1–100, default 25) |
| `speed` | number | Loops per second, stored as 0.001–1.0 (displayed as integers 1–1000 via `toUI: v * 1000`, `fromUI: v / 1000`) |
| `angle` | number | Rotation angle as fraction of a full turn (0–1.0, step 0.01) |
| `ccw` | boolean | Direction: `false` = clockwise (CW), `true` = counter-clockwise (CCW) |
| `extra` | object | Non-standard parameters keyed by name (e.g. `{ freqA: 3, freqB: 2 }` for Lissajous, `{ mult: 1.5, turns: 2 }` for spiral) |
| `curve` | array | The sampled closed curve — N complex points `[{re, im}, ...]` |
| `curveIndex` | number | Integer curve index last set during animation (floor of rawIdx) |

Standard keys (`radius`, `speed`, `angle`, `ccw`) are stored directly on the coefficient object. All other parameter keys are stored inside `extra`.

## Curve Representation

Each animated coefficient stores a **sampled closed curve** — an array of N complex points `curve[0], curve[1], ..., curve[N-1]` representing the trajectory in the complex plane. The first point `curve[0]` is the coefficient's **home position** (where it sits before animation starts).

The number of sample points N depends on the path type:
- **Standard paths** (circle, horizontal, vertical, random, lissajous, figure-8, cardioid, astroid, deltoid, rose, epitrochoid, hypotrochoid, butterfly, star, square, c-ellipse): N = 200 (interactive) or the Steps dropdown value (10K/50K/100K/1M) in fast mode
- **High-resolution paths** (hilbert, peano, sierpinski, spiral): N = 1500 (interactive) or Steps value in fast mode

Constants: `COEFF_TRAIL_SAMPLES = 200`, `COEFF_TRAIL_SAMPLES_HI = 1500`, `_HIRES_PATHS = new Set(["hilbert", "peano", "sierpinski", "spiral"])`.

The curve is always closed: walking from index 0 through N-1 and wrapping back to 0 traces exactly one complete loop.

## PATH_CATALOG and PATH_PARAMS

### PATH_PARAMS

Defines the parameter schema for each path type. Each entry is an array of parameter descriptors with `key`, `label`, `min`, `max`, `step`, `default`, and `fmt` fields. Standard parameter shortcuts:

- `_RSAD` = `[speed, radius, angle, ccw]` — most curve paths use this
- `_RSD` = `[speed, radius, ccw]` — horizontal, vertical (no angle)

Full listing:

| Path Type | Parameters |
|-----------|------------|
| `none` | (none) |
| `follow-c` | (none — D-only) |
| `circle` | S, R, A, CW/CCW |
| `horizontal` | S, R, CW/CCW |
| `vertical` | S, R, CW/CCW |
| `spiral` | S, R (multiplier 0–2x), T (turns 0.5–5), CW/CCW |
| `random` | S, sigma (0–10, as % of coeffExtent) |
| `lissajous` | S, R, A, CW/CCW, a (freq 1–8), b (freq 1–8) |
| `figure8` through `square` | S, R, A, CW/CCW |
| `hilbert`, `peano`, `sierpinski` | S, R, A, CW/CCW |
| `c-ellipse` | S, W (width 1–100%), CW/CCW |

### PATH_CATALOG

Single source of truth for all path `<select>` elements. Organized into groups:

1. **None** — top-level `"none"` option
2. **Follow C** — top-level `"follow-c"` option (D-only, `dOnly: true`)
3. **Basic** — circle, horizontal, vertical, spiral, Gaussian cloud (random)
4. **Curves** — lissajous, figure-8, cardioid, astroid, deltoid, rose, epitrochoid (Spirograph), hypotrochoid, butterfly, star (pentagram), square, c-ellipse
5. **Space-filling** — hilbert (Moore), peano, sierpinski

### Dithered Variants

Every path type except `"none"`, `"follow-c"`, and `"random"` has an auto-generated **dithered variant** (e.g. `"circle-dither"`, `"hilbert-dither"`). At catalog build time, a `(dither)` entry is inserted after each base path in every group. The dithered variant inherits all parameters from its base path plus an additional **sigma** parameter (0–1.0, step 0.01, default 0.2, displayed as `sigma%`). When computing curves, the `_ditherSigmaPct` flag is stored on the curve array. During animation, if `_ditherSigmaPct` is set, each interpolated position is perturbed by a Gaussian random offset scaled by `sigma / 100 * coeffExtent()`. This creates a noisy/fuzzy version of the base trajectory. Backward compatibility: old saves with sigma values > 1 are divided by 10 on load.

`buildPathSelect(sel, noneLabel, dNode)` populates a `<select>` from `PATH_CATALOG`. The `dNode` flag controls inclusion of D-only options like "Follow C" — called with `true` for the D-List path selector, omitted for C-List and anim-bar selectors.

## Special Curve Types

### Cloud paths (random / Gaussian)

When `baseType === "random"`, `computeCurveN()` generates a Gaussian point cloud: N independent points sampled from a 2D Gaussian centered at the home position with standard deviation equal to the radius parameter (already scaled to absolute units). The curve array is flagged with `curve._isCloud = true`.

During animation, cloud curves use **snap indexing** (no interpolation): the coefficient jumps to the nearest integer index (`Math.floor(rawIdx) % N`), producing discrete jumps rather than smooth motion.

### Orbital paths (spiral, c-ellipse)

Curves flagged with `curve._isOrbital` store **absolute** positions in the complex plane rather than home-relative offsets.

- **Spiral**: Orbits around the origin (0+0i). The coefficient spirals from its current orbit radius R0 to a target radius R1 = R0 * mult (default 1.5x) over `turns` turns (default 2), holds at R1 for one full revolution, then spirals back. Three phases: outward spiral, revolution at target, return spiral. Total angular travel = `(2*turns + 1) * 2*pi`.
- **C-Ellipse**: Traces an ellipse whose vertices are the coefficient's home position and the origin (0+0i). The center is the midpoint, semi-major axis = half the distance from home to origin, semi-minor axis = width% of semi-major. The `width` parameter (1–100%, default 50%) controls the eccentricity.

Both are fully regenerated (not transformed) when radius or angle changes, because `transformCoeffCurve()` detects the `_isOrbital` flag and calls `computeCurve()` instead of applying scale/rotation transforms.

### Space-Filling Curves

Three space-filling curves implemented via L-system turtle graphics with caching:

- **Hilbert (Moore curve):** Closed variant of the Hilbert curve — 4 Hilbert sub-curves arranged in a loop. Order 4, 256 points.
- **Peano:** Classic Peano space-filling curve. Order 3, 729 points, out-and-back traversal for closure (1458 steps total).
- **Sierpinski arrowhead:** Fills a Sierpinski triangle. Order 5, 243 segments, out-and-back for closure (486 steps total).

All three generate uniform step sizes and are cached on first use.

## How the Curve Index Changes with Time

### Playback Controls

The Play, Scrub, and Home controls live in the **header bar** (not in the left tab bar). The scrub slider is **additive**: dragging it adds seconds on top of the current elapsed time (stored in `scrubBase`). On mousedown/touchstart it captures `scrubBase = animState.elapsedAtPause || 0`, then during input computes `elapsed = scrubBase + sliderValue/1000`. On release the slider resets to 0 (the elapsed time is already committed via `advanceToElapsed()`). The Play button cycles through Play -> Pause -> Resume states.

### Position Computation

During animation, time is tracked as `elapsed` — seconds since Play was pressed (real wall-clock time divided by 1000). Each coefficient has three relevant parameters:

- **speed** — how many full loops per second (slider displays 1–1000 as integers; internally 0.001–1.0)
- **ccw** — direction flag: false = clockwise, true = counter-clockwise
- **curve** — the N-point sampled trajectory

At each animation frame, the curve index for coefficient *i* is computed as:

```
t = elapsed * speed * direction        (direction = -1 if CCW, +1 if CW)
u = ((t mod 1) + 1) mod 1              (fractional position in [0, 1), always positive)
rawIdx = u * N                          (continuous index into the curve array)
```

For regular curves, the position is **linearly interpolated** between adjacent samples:

```
lo  = floor(rawIdx) mod N
hi  = (lo + 1) mod N
frac = rawIdx - floor(rawIdx)

re = curve[lo].re * (1 - frac) + curve[hi].re * frac
im = curve[lo].im * (1 - frac) + curve[hi].im * frac
```

For cloud curves (`curve._isCloud`), no interpolation — the coefficient snaps to the nearest integer index.

For dithered curves (`curve._ditherSigmaPct`), after interpolation the position is perturbed by a Gaussian random offset: `re += gaussRand() * (sigma / 100 * coeffExtent())`.

### advanceToElapsed(elapsed)

The main function that advances all C-nodes along their curves for a given elapsed time. For each coefficient with `pathType !== "none"`, it computes the interpolated position as above and updates `c.re`, `c.im`, and `c.curveIndex`. Then calls `advanceDNodesAlongCurves(elapsed)` for D-nodes.

### advanceDNodesAlongCurves(elapsed)

Advances all D-nodes (morph targets) along their curves. For each D-node:
- If `pathType === "follow-c"` and index < coefficients.length: copies the C-coefficient's current position directly (`d.re = coefficients[i].re`)
- If `pathType === "none"`: skips (D-node stays at home)
- Otherwise: same interpolation logic as C-nodes (linear interpolation or cloud snap, plus dither perturbation)

### What u = 0 means

When `u = 0`, `rawIdx = 0`, so the coefficient is at `curve[0]` — its home position. This happens at `elapsed = 0` (animation start) and every time `elapsed * speed` is an exact integer. Each coefficient returns home every `1/speed` seconds.

### Example

A coefficient with speed = 0.50 (displayed as 500) and N = 200:
- At elapsed = 0.0s -> t = 0.0, u = 0.0, rawIdx = 0 -> at curve[0] (home)
- At elapsed = 0.5s -> t = 0.25, u = 0.25, rawIdx = 50 -> interpolating near curve[50]
- At elapsed = 1.0s -> t = 0.50, u = 0.50, rawIdx = 100 -> halfway through the loop
- At elapsed = 2.0s -> t = 1.0, u = 0.0, rawIdx = 0 -> back at home (one full loop)

Period = 1/speed = 2.0 seconds.

## Trajectory Editors

There are five places where a coefficient's path can be edited:

### 1. Anim-bar (C-Nodes tab, preview/revert/commit pattern)

The anim-bar sits at the top of the C-Nodes tab. It shows the selection label, a path type dropdown (`#anim-path`), dynamically rebuilt parameter sliders/toggles (`#bar-dynamic`), and the **"Update Whole Selection"** button (`#sel2path-btn`).

**Preview/revert/commit pattern:**
- When any slider or toggle is adjusted, `previewBarToSelection()` is called immediately. On first interaction it takes a **snapshot** of all selected coefficients' path state (`barSnapshots = { idx: {pathType, radius, speed, angle, ccw, extra, curve, curveIndex}, ... }`), then applies the current bar settings as a live preview.
- Changing the path type dropdown also triggers `previewBarToSelection()`.
- If the user clicks away (deselects coefficients, changes tab, etc.), `revertBarPreview()` restores all coefficients from the snapshot and clears `barSnapshots`.
- Pressing Escape also reverts the preview.
- Clicking **"Update Whole Selection"** calls `commitBarPreview()` if a preview is active (discards the snapshot, keeping changes) or `applyBarToSelection()` if no preview is active.
- `commitBarPreview()` resets animation elapsed time to 0 and refreshes trails and the C-List.

**No PS button**: The PS (prime speed) button was removed from the anim-bar. Use the PrimeSpeeds transform in the C-List Transform dropdown instead, or the per-coefficient path picker popup.

### 2. Right-click context menu (C-node)

Right-clicking a C-coefficient dot on the SVG canvas opens a per-coefficient trajectory editor popup (`#coeff-ctx`). It shows:
- Title with coefficient subscript label and a reposition button (cycles through 4 quadrants: BR, BL, TL, TR)
- Path type dropdown (cloned from `#anim-path`)
- Dynamically built parameter sliders/toggles
- Accept and Delete buttons

All slider/toggle changes call `previewCtx()` for immediate live preview. Closing without clicking Accept reverts to the snapshot. Clicking Accept discards the snapshot and keeps changes.

### 3. Right-click context menu (D-node)

Right-clicking a D-node dot on the morph panel SVG opens `#dnode-ctx`. Same structure as the C-node context menu but for morph targets. The path dropdown includes the "Follow C" option (inserted after "None"). Accept button only (no Delete for D-nodes).

### 4. C-List path picker popup

Clicking the path button in a C-List row opens a per-coefficient path picker popup (`#path-pick-pop`). It provides:
- Title with coefficient subscript label
- Path dropdown (cloned from `#anim-path`)
- Dynamic parameter sliders/toggles with live preview
- Accept button

On open, a snapshot is taken. Closing without Accept reverts to the snapshot. All slider changes call `previewPP()` for immediate visual feedback.

### 5. D-List path picker popup

Clicking the path button in a D-List row opens a per-D-node path picker popup (`#dpath-pick-pop`). Same structure as the C-List path picker but for morph targets. The path dropdown includes the "Follow C" option.

### 6. C-List curve editor (bulk editor)

Below the C-List toolbar sits a compact curve editor (`#list-curve-editor`). When coefficients are selected, it shows:
- Path type dropdown (`#lce-path-sel`)
- Parameter sliders/toggles (`#lce-controls`)
- **"Update Whole Selection"** button (`#lce-update-sel`)

The editor syncs to the **first selected coefficient** (sorted by index, stored in `lceRefIdx`). Changing the dropdown rebuilds controls with that path's parameter schema. Unlike the anim-bar, sliders here do **not** auto-preview — the user must click "Update Whole Selection" to apply.

**No PS button or node cycler**: The PS button and prev/next node cycler arrows were removed from this editor. Use the PrimeSpeeds transform in the C-List Transform dropdown instead.

### 7. D-List curve editor (bulk editor)

Below the D-List toolbar sits a compact curve editor (`#dlist-curve-editor`). Mirrors the C-List curve editor but for morph targets (`selectedMorphCoeffs`, `dleRefIdx`, `#dle-path-sel`, `#dle-controls`, `#dle-update-sel`). The path dropdown includes the "Follow C" option (built via `buildPathSelect(sel, noneLabel, true)`).

**No PS button or node cycler**: Same simplification as the C-List editor.

### Transform Dropdowns

Both C-List and D-List tabs have a **Transform** dropdown (`#list-transform`, `#dlist-transform`) with the following actions, all operating on the current selection:

| Transform | Description |
|-----------|-------------|
| PrimeSpeeds | Find coprime speeds for each selected coefficient |
| Set All Speeds | Set all selected to Param1 speed |
| RandomSpeed | Random speed in [0, Param1] |
| RandomAngle | Random angle in [0, Param1/100] |
| RandomRadius | Random radius in [min(Param1,Param2), max(Param1,Param2)] |
| Lerp Speed | Linearly interpolate speeds from Param1 to Param2 across selection |
| Lerp Radius | Linearly interpolate radii from Param1 to Param2 |
| Lerp Angle | Linearly interpolate angles from Param1/100 to Param2/100 |
| RandomDirection | Random CW/CCW for each selected |
| FlipAllDirections | Toggle CW/CCW for all selected |
| ShuffleCurves | Fisher-Yates shuffle of path settings among selected |
| ShufflePositions | Fisher-Yates shuffle of positions among selected |
| CircleLayout | Arrange selected in a circle of radius Param1% |
| RotatePositions | Rotate positions around centroid by Param1/100 turns |
| ScalePositions | Scale positions from centroid (50=1x, 100=2x) |
| JitterPositions | Random position jitter by Param1% of coeffExtent |
| Conjugate | Negate imaginary part |
| InvertPositions | Complex inversion (z -> 1/z*) |
| SortByModulus | Reorder positions by modulus (ascending) |
| SortByArgument | Reorder positions by argument (ascending) |

Transforms use the execute-then-reset pattern: the dropdown fires on change, executes, then resets to "none".

### Curve Type Cycler

Both C-List and D-List toolbars have a **curve type cycler** — prev/next arrow buttons and a label showing the current curve type. `buildCurveCycleTypes()` collects all unique `pathType` values across coefficients. The "Same Curve" button selects all coefficients with the displayed curve type. Clicking the arrows cycles through the sorted list of curve types.

## Curve Building

`computeCurve(homeRe, homeIm, pathType, radius, angle, extra)` is the main entry point for building a curve at interactive resolution (200 or 1500 points). It delegates to `computeCurveN(...)` which can also be called with an arbitrary N (used for fast mode hi-res curves).

The flow:
1. **Random**: Generate N Gaussian points around home, flag `_isCloud = true`
2. **Spiral**: Build orbital spiral around origin, flag `_isOrbital = true`
3. **C-Ellipse**: Build orbital ellipse between home and origin, flag `_isOrbital = true`
4. **All others**: Call `animPathFn(pathType, t, origin, radius, extra)` for t in [0, 1), compute offsets from home, apply angle rotation, store as absolute positions

For dithered variants, `_ditherSigmaPct` is stored on the curve array.

`transformCoeffCurve(c, oldRadius, oldAngle)` efficiently updates a curve when only radius or angle changes (without full recomputation). For cloud and orbital paths it regenerates instead. For regular paths it applies scale and rotation transforms to the existing curve points, then snaps the coefficient to the nearest curve point.

## Fast Mode Passes

In fast mode, time doesn't come from the wall clock. Instead, the Worker loops through the curve in discrete steps:

```
for step = 0 to totalSteps-1:
    elapsed = elapsedOffset + (step / totalSteps) * FAST_PASS_SECONDS
```

where `FAST_PASS_SECONDS = 1.0` second and `totalSteps` is the Steps dropdown value (e.g. 100,000). One complete pass simulates exactly 1.0 second of animation time. After each pass completes, `elapsedOffset` increments by 1.0 and the next pass begins.

Pass 0 covers elapsed in [0, 1), pass 1 covers [1, 2), pass 2 covers [2, 3), etc.

### Fast Mode Curve Serialization

`serializeFastModeData(animated, stepsVal, nRoots)` packages all data workers need:
- Hi-res curves (recomputed at `stepsVal` resolution) serialized as flat `Float64Array` with offset/length metadata
- Per-curve flags: `curveIsCloud` array for cloud snap behavior
- Animation entries with `idx`, `ccw`, `speed`, `ditherSigma`
- D-node curves similarly serialized as `dAnimEntries`, `dCurvesFlat`, etc.
- `dFollowCIndices` — list of D-node indices with `pathType === "follow-c"` (workers copy C-position into morph target for these)
- Jiggle offset arrays (`jiggleRe`, `jiggleIm`)

Workers use this data to advance coefficients along curves at each step, identical to the interactive-mode formula but at much higher resolution.

## Save/Load Serialization

### Save (`buildStateMetadata()`)

Each coefficient is serialized as:
```json
{
    "pos": [re, im],
    "home": [curve[0].re, curve[0].im],
    "pathType": "circle",
    "radius": 25,
    "speed": 0.5,
    "angle": 0.33,
    "ccw": false,
    "extra": { "freqA": 3 }
}
```

D-nodes are serialized identically inside `morph.target[]`.

### Load (`loadState()`)

On load:
1. Rebuild coefficient objects with defaults for missing fields (`pathType || "none"`, `radius ?? 25`, `speed ?? 1`, `angle ?? 0`, `ccw ?? false`, `extra || {}`)
2. Set panel ranges first (so `coeffExtent()` works correctly for curve building)
3. Regenerate curves: for each coefficient, if `pathType === "none"` set curve to `[{re, im}]`, otherwise call `computeCurve()` with the saved parameters
4. For D-nodes: similarly restore from `morph.target[]` with backward-compatible defaults. D-nodes with `pathType !== "none" && pathType !== "follow-c"` get their curves regenerated. The home position for D-nodes with paths comes from `d.home || d.pos`; for D-nodes without paths it comes from `d.pos`.
5. Set `curveIndex = 0` for all coefficients and D-nodes

Curves are never serialized directly — they are always regenerated from parameters on load.

## Full-Cycle Target (Jiggle Interval)

The **full cycle** is the smallest amount of time after which **every animated coefficient simultaneously returns to its home position** (curve index 0). Fast mode does **not** auto-stop — it runs indefinitely until manually paused via the bitmap "pause" button. The full-cycle value is used only by the **jiggle system**: the GCD button in the jiggle popup computes the cycle length and sets `jiggleInterval` so that jiggle perturbations are applied at exact cycle boundaries.

### Derivation

Coefficient *i* returns home whenever `elapsed * speed_i` is an integer. Its period is:

```
T_i = 1 / speed_i    seconds
```

For **all** coefficients to be home simultaneously, the elapsed time must be a common multiple of every coefficient's period. The earliest such time is the **least common multiple (LCM)** of all periods:

```
T_cycle = LCM(T_1, T_2, ..., T_k)
```

Since each pass covers 1.0 second, the number of passes for a full cycle is `T_cycle / 1.0 = T_cycle`.

### Integer arithmetic

The speed slider displays integers 1–1000 (internally stored as `s_i / 1000`), so every speed can be written as `s_i / 1000` where `s_i` is a positive integer (e.g. speed 500 -> s_i = 500, speed 7 -> s_i = 7). Then:

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
| 2 coeffs | 0.500, 0.100 | 500, 100 | 100 | 10 | fast one loops 5x, slow one loops 1x |
| 2 coeffs | 0.300, 0.200 | 300, 200 | 100 | 10 | first loops 3x, second loops 2x |
| 2 coeffs | 0.007, 0.003 | 7, 3 | 1 | 1000 | first loops 7x, second loops 3x |
| 3 coeffs | 1.000, 0.500, 0.250 | 1000, 500, 250 | 250 | 4 | loops 4x, 2x, 1x respectively |
| 1 coeff | 1.000 | 1000 | 1000 | 1 | single pass = one full loop |

### Edge cases

- **No animated coefficients** (all paths are "none"): The GCD button has no speeds to compute, so `jiggleInterval` is unchanged. Fast mode runs until manually paused.
- **Speed = 0**: coefficients with zero speed are skipped in the GCD computation (they never move, so they're always "home").
- **Single speed**: GCD = s, passes = 1000/s. E.g. speed 0.001 (s=1) -> 1,000 passes (clamped to 100 by the jiggle interval slider).
- **Coprime speeds**: e.g. 7 and 3 -> GCD(7, 3) = 1 -> 1000 passes. This is the worst case for small speeds.

## Prime Speed (PS)

The **PS** (prime speed) feature finds the nearest integer speed (1–1000 for C-nodes, 1–2000 for D-nodes) that is **coprime** with all other animated coefficients' speeds and also **different** from all of them. This maximizes the full-cycle pass count (`1000 / GCD = 1000 / 1 = 1000` when all speeds are pairwise coprime), ensuring the densest possible bitmap coverage.

- `findPrimeSpeed(currentIntSpeed, excludeSet)` — searches outward from the current speed (+/-1, +/-2, ...) up to +/-1000, clamped to [1, 1000]. Checks only C-coefficients.
- `findDPrimeSpeed(currentIntSpeed, excludeSet)` — searches the same way but with range up to +/-2000, clamped to [1, 2000]. Checks only D-nodes, skipping those with `pathType === "follow-c"`. Since 1 is coprime with everything, both always terminate.

**Where PS is available:**
- **Removed from**: anim-bar, C-List curve editor, D-List curve editor
- **Still available in**: per-coefficient path picker popups (click the path cell in a C-List or D-List row), and as the **PrimeSpeeds** transform in the C-List/D-List Transform dropdown (bulk prime-speed assignment)

### Verification

After exactly `passes` passes, every coefficient's elapsed time is `passes` seconds. For coefficient *i*:

```
elapsed * speed_i = passes * (s_i / 1000) = (1000 / GCD) * (s_i / 1000) = s_i / GCD
```

Since GCD divides every s_i by definition, `s_i / GCD` is always an integer. Therefore `u = 0` and `rawIdx = 0` — every coefficient is at `curve[0]`, its home position.

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
No perturbation. Generate is a no-op.

#### Random
Fresh Gaussian offsets each trigger. Each trigger replaces all offsets independently.
- **Control**: sigma (0–100) — standard deviation as % of `coeffExtent()`

#### Rotate
Cumulative rotation of all animated coefficient home positions around their centroid by a fixed angle step.
- **Control**: theta (0.001–0.500 turns)
- **State**: `jiggleCumulativeAngle`

#### Walk
Cumulative random walk — each trigger adds a small random step to current offsets.
- **Control**: sigma (0–100)

#### Scale
Cumulative scaling of all animated coefficient home positions around their centroid.
- **Control**: step (1–50) percent scale per trigger
- **State**: `jiggleCumulativeScale`

#### Circle
Cumulative rotation around the **origin** (not centroid). Each coefficient's home position is multiplied by e^(i*theta).
- **Control**: theta (0.001–0.500 turns)

#### Spiral
Combined cumulative rotation + scaling around the centroid.
- **Controls**: theta + step

#### Breathe
Sinusoidal scaling from centroid — coefficients expand and contract periodically.
- **Controls**: Amplitude (0–100%), Period (triggers per oscillation)

#### Wobble
Sinusoidal rotation around centroid — coefficients oscillate angularly.
- **Controls**: theta (max rotation in turns), Period

#### Lissajous
Uniform translation along a Lissajous figure — all coefficients shift by the same offset.
- **Controls**: Amplitude (0–100%), Period, FreqX, FreqY

### Common Controls

- **Generate**: Computes new offsets using the current mode.
- **Clear**: Removes all offsets and resets cumulative state.
- **OnTarget**: When checked, automatically calls Generate each time a full cycle completes.

### Persistence

Mode, sigma, theta, and scale step are saved/loaded with the project state. Cumulative state (angle, scale factor) and active offsets are transient — they reset on load.

## C-List Tab Columns

The C-List tab shows a row per coefficient with the following elements (built by `refreshCoeffList()`):

| Element | Content | Updates |
|---------|---------|---------|
| **Checkbox** | Selection toggle (`.cpick-cb`) | On click |
| **Color dot** | Coefficient color from `coeffColor(i, n)` | Static |
| **Sensitivity dot** | Derivative sensitivity color from `sensitivityColor(coeffSens[i])` | On rebuild |
| **Label** | Subscript label (c_0, c_1, ...) where subscript = degree - index | Static |
| **Power** | Monomial term (1, z, z^2, ...) | Static |
| **Path** | Button showing path type name or "-" for none; click opens path picker popup | On path change |
| **Speed** | Speed value (1–1000 display) or "-" if none | On path change |
| **Radius** | Path radius (0–100) or "-" if none | On path change |
| **Pts** | `curve.length` — sample points in the trajectory (200 or 1500 for interactive; unrelated to the fast-mode Steps dropdown) | On path change |
| **Pos** | `c.curveIndex` — the integer curve index last set during animation. Sweeps 0 -> N-1 -> 0 during animation. | Every frame via `updateListCoords()` |
| **Coords** | Complex coordinates (re +/- im*i) | Every frame via `updateListCoords()` |

## D-List Tab Columns

The D-List tab (`refreshDCoeffList()`) shows a row per morph target with similar elements:

| Element | Content |
|---------|---------|
| **Checkbox** | Selection toggle for `selectedMorphCoeffs` |
| **Color dot** | `coeffColor(i, n)` |
| **Sensitivity dot** | Gray (#555) placeholder |
| **Label** | `d0, d1, ...` |
| **Path** | Button showing path name, "Follow C", or "-"; click opens D-path picker popup |
| **Speed** | Speed value or "-" |
| **Radius** | Radius or "-" |
| **Pts** | Curve sample count |
| **Pos** | Curve index |
| **Coords** | Complex coordinates |
