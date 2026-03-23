# Additional Dithering Modes

## Scope

This note covers the remaining older brush/dither transforms that are documented in [ops_xfrm.md](/Users/nicknassuphis/karpo_hackathon/polypaint/ops_xfrm.md) but are not yet implemented in the current UI/backend flow.

Already covered by the newer system:

- `sdith` for square dither
- `ddith` for disk dither
- `ndith` for normal dither

Still missing:

- `adth`  — annulus dither
- `scdth` — sector dither
- `ldth`  — line dither
- `crdth` — cross dither

## Goal

Implement these in the current two-parameter transform pipeline with the same targeting convention used elsewhere:

- `t1`
- `t2`
- `both`

Wire encoding:

- `0` => `t1`
- `1` => `t2`
- `2` => `both`

## Design Rule

Port the older transforms into the modern contract instead of copying the old API literally.

That means:

- do not expose `serp_len` in the UI
- do not expose raw selected-index-only legacy signatures
- do use the current target-selector style

Recommended new contract pattern:

- arg 1: target (`t1` / `t2` / `both`)
- remaining args: shape parameters and width

## Mapping From Old To New

Old documented pattern:

- `a[0]` = selected index
- `a[1]` = `serp_len`
- `a[2]` = `width`

with:

- `w = width / sqrt(max(1, serp_len))`

New project pattern:

- use current grid size scaling like the other modern dithers
- width should map to a directly understandable screen/grid scale

Recommendation:

- scale by `width / N`
- keep behavior aligned with existing `sdith` / `ddith` / `ndith`

Reason:

- this matches the current dither family better
- avoids reintroducing the opaque legacy `serp_len` parameter
- keeps all modern dithers expressed in comparable units

## Proposed UI Contracts

### `adth`

Annulus dither.

Suggested args:

- `target`
- `d`
- `inner`

Meaning:

- `d`: outer radius in grid units
- `inner`: inner radius fraction in `[0,1]`

Example:

- `adth(both, 1, 0.4)`

Behavior:

- sample uniformly by area in annulus from `inner * rmax` to `rmax`

### `scdth`

Sector dither.

Suggested args:

- `target`
- `d`
- `half_ap`
- `center`

Meaning:

- `d`: radius in grid units
- `half_ap`: half aperture as fraction of `pi`
- `center`: center angle in radians

Example:

- `scdth(t1, 1, 0.25, 0)`

Behavior:

- sample uniformly by area inside a circular sector

### `ldth`

Line dither.

Suggested args:

- `target`
- `d`
- `len`
- `angle`

Meaning:

- `d`: base half-length scale in grid units
- `len`: length fraction
- `angle`: line angle in radians

Example:

- `ldth(t2, 1, 1, 1.5708)`

Behavior:

- sample uniformly on a line segment through the origin

### `crdth`

Cross dither.

Suggested args:

- `target`
- `d`

Example:

- `crdth(both, 1)`

Behavior:

- choose horizontal or vertical arm with 50% probability
- sample uniformly along that arm

## Backend Implementation

File:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)

### Pattern

For each transform:

1. implement `pt_<name>_one(...)`
2. implement `pt_<name>(..., int n, ...)`
3. add dispatch branch in `dispatchPt(...)`

Follow the same pattern already used by:

- `pt_ddith`
- `pt_crd`
- `pt_hrt`

### Suggested Function Shapes

#### `adth`

- `pt_adth_one(double *xr, double *xi, double rmax, double inner_frac)`
- `pt_adth(..., int n, double d, double inner_frac, int gridN)`

Sampling:

- `u ~ U[0,1]`
- `theta ~ U[0, 2*pi)`
- `r = sqrt(r0*r0 + u*(r1*r1 - r0*r0))`

where:

- `r1 = d / N`
- `r0 = clamp(inner_frac, 0, 1) * r1`

#### `scdth`

- `pt_scdth_one(double *xr, double *xi, double rmax, double half_ap_frac, double center)`
- `pt_scdth(..., int n, double d, double half_ap_frac, double center, int gridN)`

Sampling:

- `theta = center + uniform(-half_ap, +half_ap)`
- `half_ap = pi * clamp(half_ap_frac, 0, 1)`
- radial part should be area-uniform:
  - `r = sqrt(u) * rmax`

#### `ldth`

- `pt_ldth_one(double *xr, double *xi, double half_len, double len_frac, double angle)`
- `pt_ldth(..., int n, double d, double len_frac, double angle, int gridN)`

Sampling:

- `L = (d / N) * len_frac`
- `t ~ U[-L, +L]`
- offset = `t * exp(i * angle)`

#### `crdth`

- `pt_crdth_one(double *xr, double *xi, double half_len)`
- `pt_crdth(..., int n, double d, int gridN)`

Sampling:

- `L = d / N`
- pick horizontal or vertical arm with 50% probability
- sample `t ~ U[-L, +L]`

## UI Implementation

File:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

### `_ptCatalog`

Add entries for:

- `adth`
- `scdth`
- `ldth`
- `crdth`

Suggested catalog entries:

```js
adth:  { params: [{ph:'t1|t2|both', def:'both', target:true}, {ph:'d', def:'1'}, {ph:'inner', def:'0.4'}] },
scdth: { params: [{ph:'t1|t2|both', def:'both', target:true}, {ph:'d', def:'1'}, {ph:'half_ap', def:'0.25'}, {ph:'center', def:'0'}] },
ldth:  { params: [{ph:'t1|t2|both', def:'both', target:true}, {ph:'d', def:'1'}, {ph:'len', def:'1'}, {ph:'angle', def:'0'}] },
crdth: { params: [{ph:'t1|t2|both', def:'both', target:true}, {ph:'d', def:'1'}] },
```

### Dropdown Labels

Add labels for:

- `adth (annulus dither)`
- `scdth (sector dither)`
- `ldth (line dither)`
- `crdth (cross dither)`

## Testing

### File

- [tests/test_dither.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_dither.py)

### Add Tests For `adth`

- bounded by outer radius
- excludes inner radius statistically
- target `t1` only leaves `t2` unchanged

### Add Tests For `scdth`

- bounded by radius
- angular samples stay within sector bounds
- isotropy is not expected globally, but angular concentration should match the sector

### Add Tests For `ldth`

- all samples lie on the specified line within tolerance
- bounded by requested segment length
- target selection works

### Add Tests For `crdth`

- samples lie on one axis or the other
- arm choice is roughly balanced
- bounded by requested arm length

## Edge Rules

Use the same safety behavior as the modern dither functions:

- if `d <= 0`, default to `1`
- clamp fractions where needed:
  - `inner` to `[0,1]`
  - `half_ap` to `[0,1]`
- if a parameter is invalid, prefer safe clamping over undefined behavior

## Implementation Order

Recommended order:

1. `adth`
2. `ldth`
3. `crdth`
4. `scdth`

Reason:

- `adth` is the most straightforward extension of `ddith`
- `ldth` and `crdth` are simple 1D support shapes
- `scdth` has the most parameter-shape interaction

## Checklist

- add `_ptCatalog` entries
- add dropdown labels
- implement `pt_adth_one` / `pt_adth`
- implement `pt_scdth_one` / `pt_scdth`
- implement `pt_ldth_one` / `pt_ldth`
- implement `pt_crdth_one` / `pt_crdth`
- add `dispatchPt()` branches
- add tests in `tests/test_dither.py`
- run `uv run python -m pytest tests/test_dither.py -q`
- spot-check one UI flow per new dither

## Recommendation

Do not port the old `serp_len` argument literally.

Treat these as modern dither modes in the same family as:

- `sdith`
- `ddith`
- `ndith`

That keeps the UI coherent and avoids dragging legacy scaling semantics into the current system.
