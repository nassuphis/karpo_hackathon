# `ops_xfrm.py` Transform Reference

This document translates the transforms in [ops_xfrm.py](/Users/nicknassuphis/karpo_hackathon/polypaint/test-visual/prototypes/ops_xfrm.py) into the `t1, t2` style used in this project.

The source file uses the signature:

```python
op_name(z, a, state)
```

with:

- `z[0] = t1`
- `z[1] = t2`
- `a` = parameter array
- `state` = saved vectors for `save` / `get`

For this project, the important translation is:

- describe each transform in terms of `t1`, `t2`
- note which parameter slots in `a` matter
- preserve source behavior exactly unless you deliberately choose to simplify it

## Common Conventions

Many transforms use:

- `a[0]` = selected input index
  - `0` means apply to `t1`
  - `1` means apply to `t2`
  - any other value is a no-op

Many curve transforms use only the real part of the selected input:

- `u = Re(t1)` or `u = Re(t2)`
- `theta = 2*pi*u`

That means they are really using the selected parameter as a 1D curve parameter, not as a full complex point.

## Direct Value / Wiring Transforms

### `circ`

Selected-index circle map.

Parameters:

- `a[0]` = index `n`

Behavior:

- if `n == 0`: `t1 = exp(i * 2*pi*t1)`
- if `n == 1`: `t2 = exp(i * 2*pi*t2)`

### `rt`

Build a disk point from `rho = t1` and `theta = t2`, then write it into one selected output.

Parameters:

- `a[0]` = output index `n`

Behavior:

- `disk = t1 * exp(i * 2*pi*t2)`
- if `n == 0`: `t1 = disk`
- if `n == 1`: `t2 = disk`

### `tr`

Same as `rt`, but swaps which input is radius and which is angle.

Behavior:

- `disk = t2 * exp(i * 2*pi*t1)`
- if `n == 0`: `t1 = disk`
- if `n == 1`: `t2 = disk`

### `rttr`

Applies both disk maps at once.

Behavior:

- `t1' = t1 * exp(i * 2*pi*t2)`
- `t2' = t2 * exp(i * 2*pi*t1)`

### `dot`

Replace one selected parameter with a constant.

Parameters:

- `a[0]` = index `n`
- `a[1]` = replacement complex value

Behavior:

- if `n == 0`: `t1 = a[1]`
- if `n == 1`: `t2 = a[1]`

### `xim`

Discard everything except the real parts, then turn them into pure-imaginary values.

Behavior:

- `t1' = i * Re(t1)`
- `t2' = i * Re(t2)`

### `zz`

Both outputs become the same value.

Behavior:

- `z = t1 + i*t2`
- `t1' = z`
- `t2' = z`

### `zz1`

Behavior:

- `t1' = t1 + i*t2`
- `t2' = t1*t2 + i*(t1 + t2)`

### `zz2`

Behavior:

- `t1' = t1 + i*t2`
- `t2' = t1 - i*t2`

### `zz3`

Behavior:

- `t1' = t1 + i*t2`
- `t2' = t2 + i*t1`

### `pz`

Apply the same cubic polynomial separately to `t1` and `t2`.

Parameters:

- `a[0]`, `a[1]`, `a[2]`, `a[3]` = polynomial coefficients

Behavior:

- `p(x) = a0 + a1*x + a2*x^2 + a3*x^3`
- `t1' = p(t1)`
- `t2' = p(t2)`

## Discrete / Chaotic Map

### `bkr`

Baker’s map applied repeatedly to both parameters.

Parameters:

- `a[0]` = number of iterations

Behavior:

- treat each selected value `x+iy` by folding both coordinates mod 1
- map:
  - `x' = (2*x) mod 1`
  - `y' = (y + floor(2*x)) / 2`
- apply this to both `t1` and `t2`, `n` times

Note:

- the implementation mutates the original input array in place before returning it
- if you reimplement, decide whether you want to preserve that exact behavior or use a copied array consistently

## Single-Index Curve Transforms

These select one parameter, use its real part as a curve parameter, and replace only that selected parameter.

### `crd`

Cardioid.

Parameters:

- `a[0]` = index
- `a[1]` = `size`

Behavior for selected value `x`:

- `theta = 2*pi*Re(x)`
- `x' = size * (1 + cos(theta)) * exp(i*theta)`

### `hrt`

Heart curve.

Parameters:

- `a[0]` = index
- `a[1]` = `size`
- `a[2]` = rotation in turns

Behavior for selected value `x`:

- `u = Re(x)`
- `t = 2*pi*u + pi/2`
- `xh = 16*sin(t)^3`
- `yh = 13*cos(t) - 5*cos(2t) - 2*cos(3t) - cos(4t)`
- `heart = xh/40 + i*yh/40 + 0.1i`
- output = `exp(i*2*pi*turns) * size * heart`

### `spdl`

Spindle / superellipse-like shape.

Parameters:

- `a[0]` = index
- `a[1]` = `va`
- `a[2]` = `vb`
- `a[3]` = exponent control `vp`

Behavior:

- `theta = 2*pi*Re(x)`
- `x' = va * sign(cos(theta)) * |cos(theta)|^(2/vp)`
- `y' = vb * sign(sin(theta)) * |sin(theta)|^(2/vp)`

### `lmc`

Limacon.

Parameters:

- `a[0]` = index
- `a[1]` = `a`
- `a[2]` = `b`

Behavior:

- `theta = 2*pi*Re(x)`
- `r = a + b*cos(theta)`
- output = `r * exp(i*theta)`

### `rsc`

Rose curve.

Parameters:

- `a[0]` = index
- `a[1]` = amplitude
- `a[2]` = petal multiplier `k`

Behavior:

- `theta = 2*pi*Re(x)`
- `r = amplitude * cos(k*theta)`
- output = `r * exp(i*theta)`

### `lss`

Lissajous curve.

Parameters:

- `a[0]` = index
- `a[1]` = `A`
- `a[2]` = `B`
- `a[3]` = `a`
- `a[4]` = `b`
- `a[5]` = phase offset in turns

Behavior:

- `theta = 2*pi*Re(x)`
- `delta = pi * phase_turns`
- `X = A * sin(a*theta + delta)`
- `Y = B * sin(b*theta)`
- output = `X + iY`

### `ast`

Astroid.

Parameters:

- `a[0]` = index
- `a[1]` = scale

Behavior:

- `theta = 2*pi*Re(x)`
- `X = scale * cos(theta)^3`
- `Y = scale * sin(theta)^3`

### `asp`

Archimedean spiral.

Parameters:

- `a[0]` = index
- `a[1]` = `a`
- `a[2]` = `b`

Behavior:

- `theta = 2*pi*Re(x)`
- `r = a + b*theta`
- output = `r * exp(i*theta)`

### `lsp`

Logarithmic spiral.

Parameters:

- `a[0]` = index
- `a[1]` = `a`
- `a[2]` = `b`

Behavior:

- `theta = 2*pi*Re(x)`
- `r = a * exp(b*theta)`
- output = `r * exp(i*theta)`

### `dlt`

Deltoid.

Parameters:

- `a[0]` = index
- `a[1]` = scale `R`

Behavior:

- `theta = 2*pi*Re(x)`
- `X = R * (2*cos(theta) + cos(2*theta)) / 3`
- `Y = R * (2*sin(theta) - sin(2*theta)) / 3`

## Polygon / Shape Perimeter Transforms

### `rply`

Regular polygon perimeter.

Parameters:

- `a[0]` = index
- `a[1]` = number of sides
- `a[2]` = radius
- `a[3]` = rotation in turns

Behavior:

- use `t = Re(x) mod 1`
- interpret `t` as a position along the perimeter
- linearly interpolate between consecutive polygon vertices

### `star`

Simple star perimeter.

Parameters:

- `a[0]` = index
- `a[1]` = number of points
- `a[2]` = outer radius
- `a[3]` = inner radius ratio

Behavior:

- use alternating outer and inner vertices
- linearly interpolate along edges

### `rect`

Rectangle perimeter.

Parameters:

- `a[0]` = index
- `a[1]` = width
- `a[2]` = height
- `a[3]` = rotation in turns

Behavior:

- `t = Re(x) mod 1`
- walk the perimeter uniformly by arc length
- rotate final point

### `rrect`

Rounded rectangle via superellipse parameterization.

Parameters:

- `a[0]` = index
- `a[1]` = width
- `a[2]` = height
- `a[3]` = roundness `m`

Behavior:

- `theta = 2*pi*(Re(x) mod 1)`
- `X = (W/2) * sign(cos(theta)) * |cos(theta)|^(2/m)`
- `Y = (H/2) * sign(sin(theta)) * |sin(theta)|^(2/m)`

### `eclps`

Ellipse.

Parameters:

- `a[0]` = index
- `a[1]` = x radius
- `a[2]` = y radius
- `a[3]` = rotation in turns

Behavior:

- `theta = 2*pi*(Re(x) mod 1)`
- output = rotated `rx*cos(theta) + i*ry*sin(theta)`

## General Parametric Shape Families

These all operate on one selected index and use a larger parameter block.

### `supe`

Superellipse / Lamé curve with extra deformation knobs.

Parameters:

- `a[0]` = index
- `a[1]` = `A` x half-axis
- `a[2]` = `B` y half-axis
- `a[3]` = roundness `m`
- `a[4]` = rotation in turns
- `a[5]` = theta multiplier
- `a[6]` = theta offset in turns
- `a[7]` = x stretch
- `a[8]` = y stretch
- `a[9]` = skew (`x += skew*y`)
- `a[10]` = translate x
- `a[11]` = translate y

Behavior:

- `theta = 2*pi*(theta_offset + theta_mul*(Re(x) mod 1))`
- base Lamé curve:
  - `px = sign(cos(theta)) * |cos(theta)|^(2/m)`
  - `py = sign(sin(theta)) * |sin(theta)|^(2/m)`
- then scale, skew, rotate, translate

### `supf`

Superformula.

Parameters:

- `a[0]` = index
- `a[1]` = `m`
- `a[2]` = `n1`
- `a[3]` = `n2`
- `a[4]` = `n3`
- `a[5]` = `a_r`
- `a[6]` = `b_r`
- `a[7]` = overall scale
- `a[8]` = rotation in turns
- `a[9]` = theta offset in turns
- `a[10]` = theta multiplier
- `a[11]` = y stretch

Behavior:

- standard superformula radius
- then convert polar to Cartesian
- apply y anisotropy and rotation

### `ecld`

Epicycloid.

Parameters:

- `a[0]` = index
- `a[1]` = fixed/base radius `R`
- `a[2]` = rolling radius `r`
- `a[3]` = overall scale
- `a[4]` = rotation in turns
- `a[5]` = theta multiplier
- `a[6]` = theta offset in turns
- `a[7]` = x stretch
- `a[8]` = y stretch
- `a[9]` = skew
- `a[10]` = translate x
- `a[11]` = translate y

Behavior:

- `theta = 2*pi*(th0 + km*(Re(x) mod 1))`
- `k = (R + r) / r`
- `X = (R + r) cos(theta) - r cos(k theta)`
- `Y = (R + r) sin(theta) - r sin(k theta)`
- then deform, rotate, scale, translate

### `hcld`

Hypocycloid.

Same parameter layout as `ecld`, but:

- `k = (R - r) / r`
- `X = (R - r) cos(theta) + r cos(k theta)`
- `Y = (R - r) sin(theta) - r sin(k theta)`

### `trch`

Trochoid.

Parameters:

- `a[0]` = index
- `a[1]` = rolling radius `R`
- `a[2]` = pen offset `d`
- `a[3]` = overall scale
- `a[4]` = rotation in turns
- `a[5]` = theta multiplier
- `a[6]` = theta offset in turns
- `a[7]` = x stretch
- `a[8]` = y stretch
- `a[9]` = skew
- `a[10]` = translate x
- `a[11]` = translate y

Behavior:

- `X = R*(theta - sin(theta)) + d*cos(theta)`
- `Y = R*(1 - cos(theta)) + d*sin(theta)`
- then deform, rotate, scale, translate

### `lmn`

Lemniscate.

Parameters:

- `a[0]` = index
- `a[1]` = `A`
- `a[2]` = overall scale
- `a[3]` = rotation in turns
- `a[4]` = theta multiplier
- `a[5]` = theta offset in turns
- `a[6]` = x stretch
- `a[7]` = y stretch
- `a[8]` = skew
- `a[9]` = translate x
- `a[10]` = translate y

Behavior:

- `val = 2*A^2*cos(2*theta)`
- `r = sqrt(val)` if `val > 0`, else `0`
- convert to Cartesian
- then deform, rotate, scale, translate

### `cssn`

Cassini oval.

Parameters:

- `a[0]` = index
- `a[1]` = focus half-distance `C`
- `a[2]` = Cassini parameter `B`
- `a[3]` = overall scale
- `a[4]` = rotation in turns
- `a[5]` = theta multiplier
- `a[6]` = theta offset in turns
- `a[7]` = x stretch
- `a[8]` = y stretch
- `a[9]` = skew
- `a[10]` = translate x
- `a[11]` = translate y

Behavior:

- solve the quadratic in `u = r^2`
- take the nonnegative branch
- `r = sqrt(u)`
- convert to Cartesian
- then deform, rotate, scale, translate

## Stateful / Structural Transforms

These are probably less relevant to the current two-parameter UI, but they exist in the source.

### `save`

- `a[0]` = state slot index
- saves a copy of the current full vector `z` into `state[i]`
- returns the input unchanged

### `get`

- `a[0]` = output index in current `z`
- `a[1]` = saved state slot
- `a[2]` = element index inside saved vector
- loads one saved complex value into one current slot

### `snip`

- `a[0]` = start
- `a[1]` = end
- returns a sliced vector `z[start:end]`

This changes vector length, so it does not fit cleanly into a fixed `(t1, t2)` transform system.

### `xtnd`

- `a[0]` = source index
- appends a copy of `z[n]` to the vector

Also changes vector length.

### `copy`

- `a[0]` = source index
- `a[1]` = destination index
- copies one current vector slot into another

## Brush / Dither Transforms

These are older brush-like dither transforms. They are not the same contract as your current `sdith` / `ddith` / `ndith`, but they are worth documenting.

Common pattern:

- `a[0]` = selected index
- `a[1]` = `serp_len`
- `a[2]` = width

with scale factor:

- `w = width / sqrt(max(1, serp_len))`

### `sdth`

Square-ish complex dither around one selected parameter.

Behavior:

- independent uniform real and imaginary offsets in `[-w/2, +w/2]`
- add `dx + i*dy` to selected parameter

### `cdth`

Disk dither around one selected parameter.

Behavior:

- radius sampled uniformly in `[0, w]`
- angle uniform in `[0, 2*pi)`
- note: this is not area-uniform disk sampling, because radius is sampled linearly, not via `sqrt(u)`

### `ndth`

Normal dither around one selected parameter.

Parameters:

- `a[3]` = sigma multiplier

Behavior:

- sample `dx`, `dy` from a normal distribution
- add `sigma * (dx + i*dy)`

### `adth`

Annulus dither.

Parameters:

- `a[3]` = inner radius fraction

Behavior:

- sample uniformly by area in an annulus from `inner*w` to `w`

### `scdth`

Sector dither.

Parameters:

- `a[3]` = half aperture, expressed as a fraction of `pi`
- `a[4]` = center angle in radians

Behavior:

- sample uniformly by area in a circular sector

Note:

- the code names the args a little confusingly:
  - `cang = a[4]`
  - `halfap = pi * clamp(a[3], 0, 1)`

### `ldth`

Line dither.

Parameters:

- `a[3]` = length fraction
- `a[4]` = angle in radians

Behavior:

- sample one random position on a line segment through the center
- add that offset to the selected parameter

### `crdth`

Cross dither.

Behavior:

- with 50% probability choose horizontal arm, otherwise vertical
- sample uniformly along that arm
- add the resulting offset to the selected parameter

## Names Worth Carrying Over

If you implement these in the current project, the highest-value transforms from this file are probably:

- `zz`, `zz1`, `zz2`, `zz3`
- `crd`, `hrt`
- `spdl`, `lmc`, `rsc`, `lss`, `ast`, `asp`, `lsp`, `dlt`
- `rply`, `star`, `rect`, `rrect`, `eclps`
- `supe`, `supf`, `ecld`, `hcld`, `trch`, `lmn`, `cssn`

The vector-length-changing/stateful ones:

- `save`, `get`, `snip`, `xtnd`

are a worse fit for the current fixed two-parameter transform system.

## Implementation Advice

For this project, a clean porting rule is:

1. For transforms that naturally map `(t1, t2) -> (t1', t2')`, implement them directly.
2. For single-index curve transforms, expose the first parameter as the selected index `0` or `1`.
3. Compute both outputs from the original inputs, not partially updated ones.
4. Preserve the source formulas exactly first; only simplify names after the port works.

If you want, I can next turn this into a smaller shortlist of “transforms worth adding first” with concrete `dispatchPt()` implementation sketches. 
