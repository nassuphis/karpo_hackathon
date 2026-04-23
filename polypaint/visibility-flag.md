# Solve Score Visibility Flag Plan

Status: design note only.

## Objective

Add a solve-score mode flag:

- `Visible only`

Meaning:

- exclude solves with zero visible roots from solve-score clipping
- exclude them from solve-score histogram / cut generation
- exclude them from raster emission
- exclude them from associated-palette generation

This is not a raster-only tweak.

If the flag only affects raster emission, off-screen solves still shape the clip
window and the 10 solve-score bins, which is exactly the distortion the flag is
meant to fix.

## Why This Exists

Current behavior:

1. compute one solve-score value per solve from the whole solve vector
2. map that value to one of 10 bins
3. only then test each root against the viewport and emit visible hits

So a solve with no visible roots:

- still participates in clip bounds
- still participates in histogram / cut generation
- still occupies part of the 0..9 solve-score range
- but contributes no visible pixel

That can waste bin resolution.

Example failure shape:

- visible solves use only the lower few bins
- higher bins are mostly consumed by off-screen solves

`Visible only` should make the solve-score distribution reflect the visible
subset instead.

## Core Rule

`Visible only` is a solve-score mode flag, not a score-program token.

It does not change the numeric score program.

It changes the set of solves considered eligible for:

- clip calculation
- histogram / merge / cut generation
- raster emission
- associated-palette palette-bin generation

The score program still returns a normalized scalar in `[0,1]`.

Background / no-hit remains a raster/bin outcome, not a score value.

## Eligibility Definition

A solve is `visible` if at least one root projects inside the current viewport.

Use the same projection math as raster:

- center
- scale
- rotation
- output width / height

Specifically:

- apply the same root transforms already used for solve-score/raster
- project each root using the current render viewport
- if any projected root lands inside `[0, W) x [0, H)`, the solve is visible

This must use the same visibility contract in:

- solve-score clip
- solve-score hist / merge
- raster / raster_mt
- associated palette chunk generation

If these diverge, the image and saved palette will drift.

## User-Facing UI

Add a solve-score mode checkbox:

- label: `Visible only`

Placement:

- near other solve-score-wide controls, not inside the chip list

Reason:

- this is a mode flag on eligibility semantics
- it is not a metric
- it is not a unary score transform
- it should not appear in the RPN program

## Metadata / Payload Model

Add a boolean solve-score field:

- `solve_score_visible_only`

Thread it through:

- frontend dispatch payloads
- render/palette plans
- scratch solve-score clip/bins metadata
- color artifact metadata
- associated palette metadata
- saved palette metadata where solve-score provenance is stored

Recommended persistence rule:

- default = `false`
- omit or false means current behavior

## Runtime Semantics

### 1. Clip phase

Current:

- evaluate raw score for every solve
- compute clip bounds from all solves

With `Visible only`:

- first compute visibility eligibility per solve
- only eligible solves contribute to clip bounds

If zero solves are visible:

- fail explicitly with a useful message, not a fake clip range

Example error:

- `Visible-only solve score found 0 visible solves for the current viewport`

### 2. Histogram / merge / bins phase

Current:

- all solves contribute to the histogram / quantile cuts

With `Visible only`:

- only eligible solves contribute

This is the key step that actually improves bin usage.

Without this, a raster-only flag would still leave off-screen solves occupying
the solve-score range.

### 3. Raster / raster_mt

Current:

- every solve gets a score/bin
- only visible roots emit pixels

With `Visible only`:

- solve may still be scored internally if convenient
- but if the solve has zero visible roots, it must emit nothing
- no pixel-bin entry should be written for that solve

This keeps the dense image semantics unchanged:

- visible palette bins: `0..9`
- background / no plotted root: `255`

### 4. Associated palette generation

This must follow the same visible-only rule as the image render.

Otherwise the image can look correct while the associated palette drifts.

For generated associated palettes:

- only visible solves contribute to palette chunk scores / bins

For extracted / reused palette flows:

- preserve the original solve-score visible-only metadata

## Histogram Debug Behavior

The debug view must make the flag legible.

Add visibility counters:

- total solves
- visible solves
- invisible solves
- visible percentage

If `Visible only` is on:

- raw and final histograms should be computed over the visible subset
- the log should say that explicitly

Suggested log lines:

- `visibility mode: all solves`
- or
- `visibility mode: visible only`
- `visible solves: 1842/7225 (25.5%)`

If desired, a second diagnostic block can still report all-solve stats, but the
main histogram used for binning should match the actual active mode.

## Implementation Shape

### Frontend

In `index.html`:

- add checkbox state to solve-score dispatch payloads
- include it in histogram debug requests
- show it in solve-score request logs
- restore it during Populate where solve-score metadata is restored

### Compiler / metadata helpers

In `lambda/solve_score_chain.py` or adjacent metadata helpers:

- treat `visible_only` as a solve-score mode field
- do not encode it into the score program string
- include it in metadata emit/read helpers

### Clip / hist handlers

In `lambda/handler_solve_proximity.py` and native helpers:

- pass the visibility flag plus viewport parameters into clip/hist evaluation
- compute eligibility from the same projection contract as raster

### Native binaries

Likely touch:

- `lambda/solve_proximity_stats.c`
- `lambda/solve_proximity_hist_sectioned.c`
- `lambda/roots2pix.c`
- `lambda/roots2pix_mt.c`
- `lambda/solve_palette_chunk.c`
- `lambda/solve_palette_chunk_mt.c`

The safest implementation is:

- one shared helper in `solve_score.h` or a sibling header:
  - `solve_has_visible_root(...)`

That helper should take:

- root row
- degree
- center
- scale
- rotation
- width
- height

and return:

- `1` if any root is visible
- `0` otherwise

That avoids re-implementing viewport eligibility in five places.

## Edge Cases

### Zero visible solves

Must fail clearly in clip/hist generation.

Do not silently fall back to:

- all solves
- fake `0..1`
- or all-background output without explanation

### Partial visibility

Normal case:

- some roots in a solve are visible
- some are not

That solve is eligible.

Only solves with zero visible roots are excluded.

### Saved palette render

Saved-palette render already consumes persisted bins.

Visible-only behavior should be determined by the bins artifact provenance:

- if the saved palette was generated with `visible_only=true`, it should keep
  that semantics
- do not reinterpret old saved bins under a new visibility flag at render time

### Lores debug vs full render

Histogram debug operates on lores data.

That means `Visible only` in histogram debug is:

- viewport-aware
- but still lores-solve-based, not full chunk data

This is acceptable as long as the UI says:

- `lores-only debug`

The full render path must still compute visibility on full chunk data.

## Recommended Rollout

### Phase 1: histogram/debug support

- add `Visible only` checkbox
- plumb it into solve-score summary / clip / hist debug
- show visibility counters in the log

Goal:

- make the behavior legible before touching full render

### Phase 2: full render solve-score path

- apply visible-only eligibility in clip/hist/merge/raster
- ensure rendered image bins reflect visible-only distribution

### Phase 3: associated palette and palette workflows

- generated associated palettes use the same visibility rule
- extracted/reused palette metadata preserves the flag

## Tests Required

### Handler / plan tests

- render plan preserves `solve_score_visible_only`
- palette render plan preserves `solve_score_visible_only`
- metadata emit/read round-trips the flag

### Frontend tests

- checkbox exists
- dispatch payload includes the flag
- Populate restores the flag
- histogram debug log mentions the visibility mode

### Native runtime tests

Need exact-path Docker tests for:

1. clip/hist with a fixture where:
   - all solves score validly
   - only a subset has visible roots
   - visible-only changes the clip/hist result measurably

2. raster / raster_mt where:
   - a solve with no visible roots gets no emitted pixel-bin entries

3. associated palette chunk path where:
   - visible-only changes palette chunk bins

### Invariant test

For the same solve-score setup:

- image render
- saved pixel bins
- associated palette

must all agree on visibility eligibility.

## Non-Goals

This plan does **not** include:

- a chip/token named `visible` inside the RPN program
- changing the meaning of `255`
- scoring only visible roots within a solve

That last point is different.

`Visible only` means:

- include or exclude entire solves based on whether they have any visible root

It does **not** mean:

- recompute metrics on only the visible subset of roots within each solve

That would be a separate feature with different semantics.

## Recommendation

Implement `Visible only` as a solve-score mode checkbox with full-pipeline
semantics.

Do not implement it as:

- a raster-only flag
- or a fake numeric score-program token

The correct behavior requires clip, hist/binning, raster, and associated palette
to stay on the same eligibility set.
