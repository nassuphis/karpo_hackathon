# Lores Preview Transform-Scale Bug

## Summary

The Results preview is not just a low-resolution version of the full job.

For parameter transforms that scale by `1 / N`, the lores preview changes the transform itself, not only the sampling density.

That means the preview can show a materially different shape from the final render, even when:

- the same job is selected
- root transforms are null
- viewport settings match

This is especially visible for dither/spread transforms such as:

- `sdith`
- `ddith`
- `ndith`
- `adth`
- `ldth`
- `crdth`

## Root Cause

The compute pipeline currently uses a reduced `loresN` to build the preview parameter stream:

- hires param generation uses `N: n` in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3083)
- lores param generation uses `N: loresN` in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3137)

But several parameter transforms use `gridN` directly as part of their geometric scale.

Example:

- `ldth` in [sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2506)

```c
double half_len = (d / (gridN > 0 ? gridN : 1)) * len_frac;
```

So changing `N` changes:

1. how densely the parameter space is sampled
2. the actual size of the transform perturbation

Those should not be coupled for preview purposes.

## Concrete Example

Pipeline:

- `unit_circle`
- `ldth(both, 0.75, 40, 1)`

Job metadata:

- degree `24`
- `times = 500`
- full `N = 100`

Current lores sizing logic in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L3133) can drive preview to:

- `loresN = 5`

Then `ldth` scales as:

- hires:
  - `half_len = 0.75 / 100 * 40 = 0.3`
- lores preview:
  - `half_len = 0.75 / 5 * 40 = 6.0`

That is a 20x change in transform amplitude.

So the preview is not previewing the same parameter cloud at lower fidelity.
It is previewing a different transform regime.

## Why This Matters

For structured dither transforms, especially line/disk/annulus types, the visual output depends heavily on the perturbation scale.

So the current lores preview can:

- look stable and “correct”
- resemble an older render
- yet disagree strongly with the current hires render

This is not necessarily a bilevel raster bug or a viewport bug.

## What Is Not The Bug

The obvious viewport projection math appears aligned:

- preview plotting in [handler_preview.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_preview.py#L83)
- bilevel raster plotting in [bilevel_raster.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/bilevel_raster.c#L154)

So this issue is upstream of rasterization.

## Architectural Problem

`N` is currently serving two different roles:

1. sampling-grid resolution
2. transform scale reference

For preview, those should be separable.

The preview should be allowed to use a smaller sampling grid while preserving the full job's transform scale semantics.

## Possible Fix Directions

### Option 1. Separate Preview Sampling N from Transform Scale N

Add a second parameter to param generation / transform dispatch:

- `sampleN`
- `transformN`

Then:

- lores preview uses `sampleN = loresN`
- but uses `transformN = full N`

This is the cleanest semantic fix.

### Option 2. Add a “scale reference N” Override Only for Preview

Keep the main pipeline unchanged, but let preview param generation pass:

- `N = loresN` for traversal
- `transform_scale_N = full N` for dither transforms

This is a narrower version of option 1.

### Option 3. Accept That Lores Preview Is Only a Viewport Aid

Do not try to make it shape-faithful.
Document clearly that preview is only:

- for rough viewport estimation
- not for geometric fidelity

This is the least invasive option, but it does not solve the mismatch.

## Recommended Direction

Option 1 is the most correct:

- separate sampling density from transform scale

That keeps preview cheap without changing the geometry implied by the chosen parameter transforms.

## Immediate Practical Conclusion

For jobs using transforms like `ldth(both, 0.75, 40, 1)`, the current Results preview should not be trusted as a faithful preview of the final render.

It is currently a different parameterization, not just a coarser one.
