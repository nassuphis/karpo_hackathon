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

## C_i(t): C-Node Animation

Each coefficient has a **home position** `H_i = curve_i[0]` and a precomputed closed curve `curve_i` of N sample points in the complex plane. The curve is computed once from `(homeRe, homeIm, pathType, radius, angle, extra)` and stored as an array of `{re, im}` points.

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

**Notation used below:**

| Symbol | Meaning |
|--------|---------|
| `H_i` | Home position of coefficient i: `curve_i[0]` |
| `cen` | Centroid of home positions of selected coefficients |
| `E` | coeffExtent = max pairwise distance between any two coefficients |
| `sigma` | `(jiggleSigma / 10) * E` (absolute amplitude for random/walk) |

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

## Worker (Fast Mode) Differences

In fast mode, the same pipeline runs inside Web Workers with these differences:

- **Jiggle is static per pass.** The main thread computes `J_i(s)` once and serializes it as a `Float64Array`. Workers apply the same offset for every step within a pass. At jiggle interval boundaries, the main thread recomputes offsets and reinitializes workers.

- **Non-line morph falls back to JS.** When `morphPathType != "line"` and WASM step loop would otherwise be used, the code falls back to the JavaScript step loop (the WASM step loop only implements linear morph).

- **Elapsed time per step.** Workers compute elapsed as:
  ```
  elapsed = elapsedOffset + (step / totalSteps) * FAST_PASS_SECONDS
  ```
  where `FAST_PASS_SECONDS` is the simulated time per pass (typically 1 second).

The mathematical result is identical to the main-thread path.

---

## Key Invariants

1. **Jiggle never mutates C or D.** It only affects the final blend F.
2. **Jiggle uses home positions.** All jiggle offsets (except walk) are deterministic functions of the step number and the home positions `H_i = curve_i[0]`, not the current animated positions.
3. **Morph uses live positions.** The morph function receives the already-animated `C_i(t)` and `D_i(t)`, not home positions.
4. **No feedback.** F(t) depends only on the current state of C(t), D(t), and jiggle step s. It does not depend on F at any previous time step (except jiggle walk mode, which accumulates).
5. **coefficients[i].re/im is mutated in place** by curve sampling. The `coefficients` array always holds the current animated C-node positions after step 1.
