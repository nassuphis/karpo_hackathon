# New Parameter Transform Implementation Plan

## Scope

Add the following parameter transforms, based on the definitions already documented in [ops_xfrm.md](/Users/nicknassuphis/karpo_hackathon/polypaint/ops_xfrm.md):

- `spdl`
- `lmc`
- `rsc`
- `lss`
- `ast`
- `asp`
- `lsp`
- `dlt`
- `rply`
- `star`
- `rect`
- `rrect`
- `lmn`
- `cssn`

The project already has the pattern established by `crd` and `hrt`:

- UI accepts symbolic target selectors
- wire format stays numeric
- solver supports `0 => t1`, `1 => t2`, `2 => both`

This plan extends that pattern consistently.

## Goals

1. Keep UI input readable.
2. Keep solver payload format simple and numeric.
3. Share the same target-selector behavior across all new single-index transforms.
4. Implement the lowest-risk transforms first.
5. Add backend correctness tests as each group lands.

## Shared Contract

For every new single-index transform:

- arg 1 in UI accepts:
  - `t1`
  - `t2`
  - `both`
- UI serializes to:
  - `0`
  - `1`
  - `2`
- solver semantics:
  - `0` => transform only `t1`
  - `1` => transform only `t2`
  - `2` => transform both independently

For `both`, each parameter is transformed from its own current value.

## Files To Touch

### UI

File: [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Work:

- add transform entries to `_ptCatalog`
- mark first arg as `target: true`
- define placeholders/defaults for each new transform
- reuse existing target normalization/serialization path

### Solver

File: [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)

Work:

- add one helper per transform, following the `*_one` plus dispatch wrapper pattern used by `crd` and `hrt`
- extend `dispatchPt(...)` with each new transform
- keep numeric argument parsing unchanged

### Tests

File: [tests/test_param_dump.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_param_dump.py)

Work:

- add Python reference implementations
- test `t1`, `t2`, and `both` behavior
- verify unchanged parameter remains unchanged for single-target cases

### Docs

Files:

- [ops_xfrm.md](/Users/nicknassuphis/karpo_hackathon/polypaint/ops_xfrm.md)
- [ui_chip_inputs.md](/Users/nicknassuphis/karpo_hackathon/polypaint/ui_chip_inputs.md)

Work:

- confirm the new transforms follow the same target selector convention
- document any implementation-specific clamping/defaults

## Rollout By Phase

### Phase 1: Simple Polar / Curve Transforms

Implement first:

- `spdl`
- `lmc`
- `rsc`
- `lss`
- `ast`
- `asp`
- `lsp`
- `dlt`

Why first:

- all are direct single-parameter curve mappings
- no perimeter traversal
- no vertex interpolation
- no larger deformation stack

Implementation pattern:

- write `pt_<name>_one(...)`
- write `pt_<name>(..., int n, ...)` wrapper with `0/1/2`
- add dispatch branch in `dispatchPt(...)`

Recommended test coverage per transform:

- one `t1` case
- one `t2` case
- one `both` case

### Phase 2: Perimeter / Shape Walkers

Implement next:

- `rply`
- `star`
- `rect`
- `rrect`

Why second:

- these need perimeter traversal or vertex interpolation
- slightly more implementation detail
- still localized and easy to reason about

Special considerations:

#### `rply`

- needs regular polygon vertex generation
- should use `t = Re(x) mod 1`
- should walk edges uniformly by perimeter fraction
- should apply optional rotation at the end

#### `star`

- alternating outer and inner vertices
- should use edge interpolation, not polar interpolation
- decide whether point count is clamped to a minimum like `>= 3`

#### `rect`

- should walk perimeter uniformly by arc length
- side lengths matter
- rotation should be applied to final Cartesian point

#### `rrect`

- can be implemented as a superellipse-style parameterization
- this is simpler than true rounded-corner arc stitching
- follow the documented behavior exactly from [ops_xfrm.md](/Users/nicknassuphis/karpo_hackathon/polypaint/ops_xfrm.md#L400)

Recommended test coverage:

- corners / edge transitions for `rect`
- vertex interpolation sanity for `rply` and `star`
- roundness behavior for `rrect`

### Phase 3: General Parametric Families

Implement last:

- `lmn`
- `cssn`

Why last:

- larger parameter blocks
- more numerically sensitive
- more likely to need clamping or branch handling

Special considerations:

#### `lmn`

- branch only when `val = 2*A^2*cos(2*theta)` is positive
- otherwise radius becomes `0`
- then apply scale / rotate / deform stack

#### `cssn`

- solve quadratic in `u = r^2`
- pick nonnegative branch
- handle negative discriminant safely
- then convert polar to Cartesian and apply deformations

These should be treated as the first transforms where small numeric edge cases are expected.

## Shared Helper Recommendation

To reduce duplication in `lambda/sweep_cli.c`, add a small helper pattern for selected-index transforms:

- one helper that applies a one-point transform to one selected `(xr, xi)`
- one wrapper that dispatches to `t1`, `t2`, or both

For example:

- `pt_spdl_one(...)`
- `pt_spdl(..., int n, ...)`

This matches the existing `crd` / `hrt` style and keeps implementation predictable.

## Suggested Parameter Defaults For UI

These do not need to match any historic defaults exactly, but should be visually useful.

- `spdl`: `target=t1`, `va=0.5`, `vb=0.2`, `vp=1.5`
- `lmc`: `target=t1`, `a=0.3`, `b=0.5`
- `rsc`: `target=t1`, `amp=0.5`, `k=2`
- `lss`: `target=t1`, `A=0.5`, `B=0.5`, `a=3`, `b=2`, `phase=0.5`
- `ast`: `target=t1`, `scale=1`
- `asp`: `target=t1`, `a=0`, `b=0.1`
- `lsp`: `target=t1`, `a=0.1`, `b=0.15`
- `dlt`: `target=t1`, `R=1`
- `rply`: `target=t1`, `sides=5`, `radius=1`, `turns=0`
- `star`: `target=t1`, `points=5`, `outer=1`, `inner_ratio=0.5`
- `rect`: `target=t1`, `width=2`, `height=1`, `turns=0`
- `rrect`: `target=t1`, `width=2`, `height=1`, `m=4`
- `lmn`: `target=t1`, `A=1`, `scale=1`, `turns=0`, `theta_mul=1`, `theta_off=0`, `xstretch=1`, `ystretch=1`, `skew=0`, `tx=0`, `ty=0`
- `cssn`: `target=t1`, `C=0.5`, `B=1`, `scale=1`, `turns=0`, `theta_mul=1`, `theta_off=0`, `xstretch=1`, `ystretch=1`, `skew=0`, `tx=0`, `ty=0`

## Validation / Clamping Rules

Add lightweight solver-side sanity handling where shape parameters can explode or degenerate:

- polygon sides / star points: clamp to minimum valid integer
- `vp`, `m`, or any roundness exponent: avoid zero or negative values if the formula divides by them
- width / height / radius / scale: allow negative only if you intentionally want reflection; otherwise clamp or document
- `cssn` discriminant and `lmn` radicand: clamp invalid branches to zero output

Do not silently reinterpret malformed UI target strings in the solver. Keep symbolic parsing in the UI only.

## Testing Plan

### Unit Strategy

For each transform:

1. Add a Python reference implementation in [tests/test_param_dump.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_param_dump.py).
2. Use `param_dump` mode to inspect transformed `t1`, `t2`.
3. Test:
   - target `0`
   - target `1`
   - target `2`

### Numerical Tolerance

Use:

- `1e-5` for most Cartesian comparisons

For polygon/perimeter transforms:

- exact geometry may need slightly looser tolerances at edge boundaries if `mod 1` handling is involved

### Regression Checks

After each phase:

- rerun [tests/test_param_dump.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_param_dump.py)
- rerun any sweep smoke tests that cover `param_transforms`

## Implementation Order Recommendation

Recommended exact order:

1. `spdl`
2. `lmc`
3. `rsc`
4. `lss`
5. `ast`
6. `asp`
7. `lsp`
8. `dlt`
9. `rply`
10. `star`
11. `rect`
12. `rrect`
13. `lmn`
14. `cssn`

This order starts with low-risk direct formulas and leaves the more numerically sensitive generalized families for last.

## Deliverables

When complete, the implementation should provide:

- UI chips for all listed transforms
- symbolic target input in the UI
- numeric wire serialization
- `0/1/2` solver behavior
- backend tests for each transform
- updated docs reflecting supported chip inputs and defaults

## Recommendation

Do this as three PR-sized chunks rather than one large patch:

1. simple curve transforms
2. perimeter transforms
3. generalized family transforms

That keeps review manageable and makes numerical regressions easier to isolate.

## Phase 1 Checklist

This is the concrete implementation checklist for the first rollout group:

- add `_ptCatalog` entries in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) for:
  - `spdl`
  - `lmc`
  - `rsc`
  - `lss`
  - `ast`
  - `asp`
  - `lsp`
  - `dlt`
- mark the first parameter of each as `target: true`
- set practical default UI params for each chip
- verify chip placeholders match the documented parameter order
- reuse `_serializeParamTransforms()` so UI symbolic targets normalize to numeric wire values automatically

- implement solver helpers in [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c):
  - `pt_spdl_one(...)`
  - `pt_lmc_one(...)`
  - `pt_rsc_one(...)`
  - `pt_lss_one(...)`
  - `pt_ast_one(...)`
  - `pt_asp_one(...)`
  - `pt_lsp_one(...)`
  - `pt_dlt_one(...)`
- implement the `0/1/2` wrappers for each transform
- add `dispatchPt(...)` branches for each transform
- clamp or guard any denominator / exponent parameter that can go invalid

- add Python reference helpers in [tests/test_param_dump.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_param_dump.py) for:
  - spindle
  - limacon
  - rose curve
  - Lissajous
  - astroid
  - Archimedean spiral
  - logarithmic spiral
  - deltoid
- add three tests per transform:
  - target `t1`
  - target `t2`
  - target `both`
- verify single-target tests leave the other parameter unchanged
- run:
  - `uv run python tests/test_param_dump.py`

- do one manual UI sanity pass:
  - add each chip in the UI
  - confirm `t1`, `t2`, and `both` are accepted
  - confirm invalid target text is rejected
  - confirm the request payload shows numeric wire values for target argument

- update docs after phase 1 lands:
  - note supported chip names
  - note parameter order
  - note that target selection is symbolic in UI and numeric on the wire
