# Coefficient Path Calculation

This document gives the exact formula for the **final coefficient vector F(t)** — the complex polynomial coefficients fed to the Ehrlich-Aberth solver at each time step. Every root trajectory the app produces is determined entirely by this vector.

## Master Formula

```
F_i(t) = M( C_i(t), D_i(t), theta(t) ) + J_i(s) * [i in S]
```

where:

| Symbol | Meaning |
|--------|---------|
| `F_i(t)` | Final coefficient i at time t (complex). This is what the solver sees. |
| `C_i(t)` | C-node i position at time t, sampled from its precomputed curve |
| `D_i(t)` | D-node i position at time t, sampled from its precomputed curve |
| `M(c, d, theta)` | Morph interpolation function (identity when morph disabled) |
| `theta(t)` | Morph phase angle = 2 pi morphRate t |
| `J_i(s)` | Jiggle offset for coefficient i at jiggle step s (complex, additive) |
| `s` | Jiggle step = floor(t / jiggleInterval) |
| `S` | Set of selected coefficient indices (jiggle targets) |
| `[i in S]` | Iverson bracket: 1 if i is in the selection, 0 otherwise |

The order of operations is: **animate C -> animate D -> morph blend -> add jiggle -> solve**.

Jiggle is never applied to C or D individually. It is applied to the already-blended result.

When morph is disabled, M is the identity on C: `M(c, d, theta) = c`.

When jiggle is off (mode = "none" or `i not in S`), the jiggle term is zero.

---

## Node Parameters

The master formula involves two sets of animated nodes, each of length `n = d+1`:

- **C-nodes** (`coefficients[]`): primary coefficients. Always active.
- **D-nodes** (`morphTargetCoeffs[]`): morph targets. Active only when morph is enabled.

Both node types carry the same set of per-node animation parameters. D-nodes additionally support `tau_i = "follow-c"` (see D-Node Animation below).

### Per-node parameters

| Symbol | Meaning | Range | Default | Unit |
|--------|---------|-------|---------|------|
| `H_i` | Home position (= `curve_i[0]`) | complex | (from pattern) | complex plane |
| `gamma_i` | Precomputed closed curve | --- | `[H_i]` | N points in C |
| `tau_i` | Path type (curve shape) | see below | `"none"` | --- |
| `R_i` | Path radius | [1, 100] | 25 | % of E |
| `v_i` | Traversal speed | [0.001, 1.0] | 1.0 | cycles/s |
| `alpha_i` | Rotation of path shape | [0, 1) | 0 | turns |
| `delta_i` | Direction: -1 if CCW, +1 if CW | {-1, +1} | +1 | --- |
| `epsilon_i` | Extra parameters (path-specific) | varies | {} | --- |
| `N` | Curve sample count | {200, 1500} | 200 | points |

Here `E = coeffExtent`: the maximum pairwise distance among all coefficient home positions. The absolute radius used for curve generation is `R_abs = (R_i / 100) * E`.

The curve `gamma_i` is generated once from `(H_i, tau_i, R_abs, alpha_i, epsilon_i)` and recomputed whenever any parameter changes. When `tau_i = "none"`, the curve is the single point `[H_i]` (no animation).

### Path types

22 base path shapes in three groups. Each animated type also has a **dithered variant** (suffix `-dither`) that adds per-frame Gaussian noise.

| Group | Types |
|-------|-------|
| Non-animated | `none`, `follow-c`* |
| Basic | circle, horizontal, vertical, spiral**, random*** |
| Curves | lissajous, figure-8, cardioid, astroid, deltoid, rose, spirograph, hypotrochoid, butterfly, star, square, c-ellipse** |
| Space-filling | hilbert, peano, sierpinski |

\* D-node only: `D_i(t) = C_i(t)`.
\*\* Orbital paths: curve points are absolute positions (orbiting the origin), not offsets from `H_i`.
\*\*\* Cloud: pre-generated Gaussian point cloud; no interpolation between samples.

Curve sample count: N = 200 for most paths; N = 1500 for space-filling paths and spiral.

**Notable extra parameters (epsilon_i):**
`spiral`: multiplier m in [0,2] (default 1.5), turns T in [0.5, 5] (default 2).
`lissajous`: frequencies a in [1,8] (default 3), b in [1,8] (default 2).
`c-ellipse`: minor-axis width w in [1,100]% (default 50).
`random`: sigma in [0,10] (default 2).
Dithered variants: sigma_% in [0,1] (default 0.2).

---

## C_i(t): C-Node Animation

Each C-node `i` carries a precomputed curve `gamma_i` generated from the parameters in the preceding table. At each time step, `C_i(t)` is obtained by sampling this curve at a phase determined by the speed `v_i` and direction `delta_i`.

**Curve sampling:**

```
phi_i  = t * speed_i * dir_i           (cycles, fractional)
u_i    = ((phi_i mod 1) + 1) mod 1     (normalized to [0, 1))
rawIdx = u_i * N
```

where `dir_i = -1` if CCW, `+1` if CW (note: the code uses `c.ccw ? -1 : 1`).

For regular curves (not clouds):
```
lo   = floor(rawIdx) mod N
hi   = (lo + 1) mod N
frac = rawIdx - floor(rawIdx)

C_i(t) = curve_i[lo] * (1 - frac) + curve_i[hi] * frac
```

This is linear interpolation between adjacent curve samples, wrapping at the boundary.

For cloud curves (random path type, `_isCloud` flag):
```
C_i(t) = curve_i[ floor(rawIdx) mod N ]
```

No interpolation — snaps to the nearest precomputed point.

**Dither term:** If the curve has `_ditherSigmaPct > 0` (circle-dither, spiral-dither, etc.), Gaussian noise is added after sampling:

```
sigma_abs = ditherSigmaPct / 100 * coeffExtent
C_i(t) += N(0, sigma_abs) + i * N(0, sigma_abs)
```

where `N(0, sigma)` is a Gaussian random variable (Box-Muller) and `coeffExtent` is the maximum pairwise distance between any two coefficients.

**When pathType = "none":** `C_i(t) = H_i` (the coefficient stays at its home position, no curve sampling).

**Curve sample counts:** N = 200 for basic paths, N = 1500 for space-filling paths (hilbert, peano, sierpinski, spiral).

---

## D_i(t): D-Node Animation

D-nodes use the identical curve-sampling formula as C-nodes, with their own independent `pathType`, `speed`, `radius`, `angle`, `ccw`, and precomputed `curve` array.

**Special case — Follow C:** When `D_i.pathType = "follow-c"`:
```
D_i(t) = C_i(t)
```
The D-node copies the already-animated C-node position. This is computed after C-node advancement, so it tracks the current animated position.

**When pathType = "none":** `D_i(t)` stays at the D-node's home position (which may differ from C's home position if the user has dragged it).

---

## M(c, d, theta): Morph Interpolation

When morph is **disabled**: `M(c, d, theta) = c`  (D-nodes are ignored).

When morph is **enabled**, the function `morphInterpPoint` computes the blended position. The behavior depends on the **C-D path type**:

### Shared setup

```
delta     = d - c                          (complex vector from C to D)
len       = |delta|
mid       = (c + d) / 2                   (midpoint)
semi      = len / 2                        (half-distance)
u         = delta / len                    (unit vector C -> D)
v         = i * u                          (perpendicular, 90 deg CCW rotation of u)
sign      = +1 if CCW, -1 if CW
```

When `len < 1e-15` (C and D coincide), all path types return `c`.

### Line (default)

```
mu = (1 - cos(theta)) / 2

M(c, d, theta) = c * (1 - mu) + d * mu
```

This is a raised-cosine oscillation. mu oscillates smoothly between 0 (at C) and 1 (at D). Period = 1/morphRate seconds. The path traces a straight line segment between C and D.

### Circle

The C-D segment is the diameter of a circle. The interpolated point travels along the circle.

```
lx = -semi * cos(theta)
ly = sign * semi * sin(theta)

M = mid + lx * u + ly * v
```

At theta=0: position = C. At theta=pi: position = D. Full revolution = 2*pi.

### Ellipse

Same as circle, but with a shorter semi-minor axis:

```
semi_b = minorPct * semi        (minorPct in [0.1, 1.0])

lx = -semi * cos(theta)
ly = sign * semi_b * sin(theta)

M = mid + lx * u + ly * v
```

The major axis lies along the C-D line. When minorPct = 1.0, this reduces to circle.

### Figure-8

A Lissajous 1:2 curve that crosses itself at the midpoint:

```
lx = -semi * cos(theta)
ly = sign * (semi / 2) * sin(2 * theta)

M = mid + lx * u + ly * v
```

At theta=0: at C. At theta=pi/2: crosses midpoint. At theta=pi: at D. At theta=3*pi/2: crosses midpoint again. The two lobes are symmetric about the midpoint.

### Morph phase and display mu

```
theta(t) = 2 * pi * morphRate * t
morphMu  = (1 - cos(theta)) / 2           (for display only in non-line modes)
```

`morphRate` is in Hz (default 0.01, range 0 to 0.1). For the line path, `morphMu` is the actual interpolation parameter. For circle/ellipse/figure-8, `morphMu` is only used for the UI readout — the actual position is computed parametrically from theta.

---

## J_i(s): Jiggle Offsets

Jiggle produces per-coefficient additive complex offsets. The offsets are:

- Computed from **home positions** `H_i = curve_i[0]`, NOT from the current animated position `C_i(t)`
- Piecewise-constant in time: they change only at jiggle step boundaries (every `jiggleInterval` seconds)
- Applied only to selected coefficients (the selection set `S`)
- Applied AFTER morph blending

```
s = floor(t / jiggleInterval)
J_i(s) = 0   if i not in S, or jiggleMode = "none"
```

**Notation** (in addition to per-node symbols from the Node Parameters table):

| Symbol | Meaning | Used by |
|--------|---------|---------|
| `cen` | Centroid = (1/\|S\|) * sum of H_i for i in S | rotate, scale-cen., spiral-cen., breathe, wobble |
| `sigma` | `(jiggleSigma / 10) * E` | random, walk |
| `g` | `(scaleStep / 100) * E` | scale, spiral |
| `N_angle` | Angle steps per full rotation | rotate, spiral, wobble |
| `N_circle` | Steps per full rotation (about origin) | circle |
| `A` | Amplitude parameter (0-100) | breathe, lissajous |
| `P` | Period in jiggle steps | breathe, wobble, lissajous |
| `freqX, freqY` | Frequency multipliers | lissajous |
| `R(alpha)` | Rotation operator by angle alpha | all rotation modes |

### Mode formulas

**random:**
```
J_i(s) = N(0, sigma) + i * N(0, sigma)
```
Independent Gaussian noise, freshly sampled each step.

**walk:**
```
J_i(s) = J_i(s-1) + N(0, sigma) + i * N(0, sigma)
J_i(0) = 0
```
Cumulative random walk. Only jiggle mode with memory.

**rotate:**
```
alpha = 2 * pi * s / angleSteps
J_i(s) = R(alpha) * (H_i - cen) - (H_i - cen)
```
where R(alpha) is rotation by alpha. Rigid rotation of the selected coefficients about their centroid.

**circle:**
```
alpha = 2 * pi * s / circleSteps
J_i(s) = R(alpha) * H_i - H_i
```
Rotation of each coefficient about the origin (0,0).

**scale-center:**
```
grow = (scaleStep / 100) * E
J_i(s) = (H_i / |H_i|) * grow * s
```
Radial growth from origin. Linear in step number.

**scale-centroid:**
```
grow = (scaleStep / 100) * E
d_i = H_i - cen
J_i(s) = (d_i / |d_i|) * grow * s
```
Radial growth from centroid.

**spiral-centroid:**
```
alpha = 2 * pi * s / angleSteps
grow = (scaleStep / 100) * E
d_i = H_i - cen
r = 1 + grow * s / |d_i|
J_i(s) = r * R(alpha) * d_i - d_i
```
Combined rotation + radial scaling from centroid.

**spiral-center:**
```
alpha = 2 * pi * s / angleSteps
grow = (scaleStep / 100) * E
r = 1 + grow * s / |H_i|
J_i(s) = r * R(alpha) * H_i - H_i
```
Combined rotation + radial scaling from origin.

**breathe:**
```
scale = 1 + (amplitude / 100) * sin(2 * pi * s / period)
d_i = H_i - cen
J_i(s) = d_i * (scale - 1)
```
Sinusoidal radial pulsation from centroid.

**wobble:**
```
alpha = (2 * pi / angleSteps) * sin(2 * pi * s / period)
d_i = H_i - cen
J_i(s) = R(alpha) * d_i - d_i
```
Sinusoidal angular oscillation about centroid.

**lissajous:**
```
amp = (amplitude / 100) * E
dx = amp * sin(2 * pi * freqX * s / period + pi/2)
dy = amp * sin(2 * pi * freqY * s / period)
J_i(s) = dx + i * dy
```
Uniform translation of all selected coefficients along a Lissajous curve. Same offset for every coefficient.

---

## Complete Pipeline (Pseudocode)

```
At each time step t:

1. FOR each coefficient i:
     IF C_i has a path (pathType != "none"):
       C_i(t) = sample curve_i at phase (t * speed_i * dir_i)
       IF dither enabled: C_i(t) += Gaussian noise
     ELSE:
       C_i(t) = H_i (home position)

2. FOR each D-node i:
     IF D_i.pathType == "follow-c":
       D_i(t) = C_i(t)
     ELIF D_i has a path (pathType != "none"):
       D_i(t) = sample D_curve_i at phase (t * D_speed_i * D_dir_i)
       IF dither enabled: D_i(t) += Gaussian noise
     ELSE:
       D_i(t) = D_home_i

3. IF morph enabled:
     theta = 2 * pi * morphRate * t
     FOR each coefficient i:
       B_i = morphInterpPoint(C_i(t), D_i(t), theta, pathType, ccw, minorPct)
   ELSE:
     B_i = C_i(t)

4. Compute jiggle step s = floor(t / jiggleInterval)
   Compute J_i(s) for each i in selection S

5. Final:
     F_i(t) = B_i + J_i(s) * [i in S]

6. Feed F(t) to Ehrlich-Aberth solver -> roots
```

---

## Fast Mode (Bitmap Workers)

In bitmap rendering, the pipeline from the Master Formula runs in parallel Web Workers. This section defines the execution model.

### Definitions

| Symbol | Meaning | Value / range |
|--------|---------|---------------|
| `DT_pass` | Virtual time per pass | 1.0 s (constant) |
| `K` | Total steps per pass | user-selected: 10 to 1,000,000 |
| `W` | Number of workers | min(hardware threads, 16) |
| `t_off` | Elapsed offset (accumulated) | starts at 0, +DT_pass each pass |

### What is a pass

A **pass** is one complete execution of `K` solver steps distributed across `W` workers. Each pass advances virtual time by exactly `DT_pass = 1.0` s. This is not wall-clock time; it is the simulated elapsed time used to advance coefficient animations along their curves.

**Step distribution.** Steps are divided evenly across workers. Let `b = floor(K / W)` and `r = K mod W`. Worker `w` (w = 0, ..., W-1) executes steps `[start_w, end_w)` where the first `r` workers receive `b+1` steps and the remaining `W-r` workers receive `b` steps.

**Elapsed time per step.** Within a pass, step `k` (0 <= k < K) runs at virtual time:
```
t(k) = t_off + (k / K) * DT_pass
```
So within a single pass, `t` ramps linearly from `t_off` to `t_off + DT_pass`.

**Per-step pipeline.** Each step executes the full pipeline from the Complete Pipeline section (animate C, animate D, morph, jiggle, solve), then maps each root to a canvas pixel and emits it as a sparse (x, y, r, g, b) entry.

### Pass lifecycle

1. **Init.** Main thread serializes all static data (curves, colors, morph config, jiggle offsets, WASM binaries) and sends it to each worker once.
2. **Run.** Main thread sends a `"run"` message to each worker with its step range `[start_w, end_w)`, the current `t_off`, and warm-start root positions.
3. **Compute.** Workers execute their steps in parallel. Each worker accumulates sparse pixel arrays (no full-canvas buffers).
4. **Done.** Each worker transfers its pixel arrays (`Int32Array` indices + `Uint8Array` RGB) to the main thread via zero-copy buffer transfer.
5. **Composite.** Main thread merges all workers' pixels into a persistent `ImageData` buffer and calls `putImageData` on the dirty rectangle only.
6. **Advance.** `t_off += DT_pass`. Root positions from the highest-step worker become the warm start for the next pass. Go to step 2.

Workers are **persistent**: they are created once at init and reused across passes (only a new `"run"` message is sent each pass).

### Jiggle boundary

Jiggle offsets are **static within a pass**: the main thread computes `J_i(s)` once and bakes it into the worker init data. Workers apply the same offset for every step in the pass.

A **jiggle boundary** occurs every `jiggleInterval` virtual seconds (default 4 s = 4 passes). When the pass counter reaches the jiggle interval:

1. Compute the new jiggle step `s = floor(t_off / jiggleInterval)`.
2. Compute `J_i(s)` for all `i in S`.
3. **Terminate all workers** and recreate them with the updated offsets (full reinit, because jiggle offsets are init-time data).

### Solver and WASM

Workers use a 2-tier execution strategy:

1. **WASM step loop**: entire per-step pipeline compiled to WASM (~16.5 KB). Supports all morph C-D path types (line, circle, ellipse, figure-8) natively.
2. **Pure JS**: fallback when WASM is unavailable.

Worker solver parameters differ from the main thread:

| | Main thread | Workers |
|---|---|---|
| MAX_ITER | 100 | 64 |
| TOL | 1e-12 (magnitude) | 1e-16 (squared) |

The mathematical pipeline is identical to the main-thread path. The output differs only in solver precision and iteration count.

---

## Key Invariants

1. **Jiggle never mutates C or D.** It only affects the final blend F.
2. **Jiggle uses home positions.** All jiggle offsets (except walk) are deterministic functions of the step number and the home positions `H_i = curve_i[0]`, not the current animated positions.
3. **Morph uses live positions.** The morph function receives the already-animated `C_i(t)` and `D_i(t)`, not home positions.
4. **No feedback.** F(t) depends only on the current state of C(t), D(t), and jiggle step s. It does not depend on F at any previous time step (except jiggle walk mode, which accumulates).
5. **coefficients[i].re/im is mutated in place** by curve sampling. The `coefficients` array always holds the current animated C-node positions after step 1.
