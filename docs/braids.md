# Root Braids and Monodromy

When you animate a coefficient along a closed loop, the roots don't just wiggle — they trace out a **braid**. This is a topological phenomenon with deep mathematical roots (pun intended).

The space of degree-*n* polynomials with distinct roots is topologically the [configuration space](https://en.wikipedia.org/wiki/Configuration_space_(mathematics)) of *n* unordered points in **C**. Its fundamental group is the [braid group](https://en.wikipedia.org/wiki/Braid_group) B_n. A closed loop in coefficient space induces a **monodromy permutation** on the roots — after one full cycle, root #3 might now occupy the position that root #7 had before.

This is not a solver artifact; it is a topological invariant of the loop. Different loops around different "holes" in coefficient space produce different permutations. The [cohomology](https://en.wikipedia.org/wiki/Cohomology) of the configuration space (computed by Arnol'd and Cohen) classifies these possibilities.

## What you see in PolyPaint

The trail patterns are visual braids. Root identity is preserved across frames by reordering each new solver output to best match the previous frame's root positions. This keeps trails tracking the same root continuously. When roots still swap indices (e.g. during fast near-collisions), jump detection (30% of visible range threshold) inserts a path break (SVG `M` move or canvas `moveTo`) rather than drawing a false connecting line.

### Trail rendering

Root trails are stored in `trailData[]` (line ~1003), an array of arrays where `trailData[i]` holds `{re, im}` points for root *i*. A parallel `finalTrailData[]` stores coefficient trails (blended C/D positions) for the Final tab. Both are capped at `MAX_TRAIL_POINTS = 4000` entries per root (line ~1005).

Trails are rendered three ways depending on which panel is active:

- **Roots SVG panel** (animation tab): `renderTrails()` (line ~5054) builds SVG `<path>` elements with `M`/`L` segments, colored per root via `rootColor()`.
- **Final SVG panel**: `renderFinalTrails()` (line ~4847) renders coefficient trails on the Final tab's SVG.
- **Bitmap canvas panel**: `drawRootsToCtx()` (line ~7282) renders trails as canvas strokes.

All three renderers use the same jump-detection logic: if the distance between consecutive trail points exceeds 30% of the visible range, a path break (`M` move) is inserted instead of a connecting line (`L`).

The Trails button in the roots toolbar header toggles trail recording on and off. Trails are cleared when animation starts (unless resuming) or when animation parameters change.

## Root matching — preserving identity

### Main-thread animation path

`matchRootOrder()` (line ~5398) is a greedy nearest-neighbor O(n^2) matcher. It reorders each new set of solver roots so that root *i* in the new frame is the one closest to root *i* in the previous frame. This is called:

- In `solveRoots()` (line ~5416): every interactive solve (dragging, manual updates).
- In the main-thread fast-mode stepping loop (line ~11738): called for every bitmap color mode except Uniform, which does not need identity tracking.

### Worker fast-mode path

Workers use typed-array versions of the matching functions. The strategy is configurable via `bitmapMatchStrategy` (line ~1014) for modes that need root identity:

- **Hungarian** (`hungarian1`): Kuhn-Munkres O(n^3) optimal assignment via `hungarianMatch()` (line ~10115), run every step. Most accurate but slowest.
- **Greedy x1** (`assign1`): Greedy nearest-neighbor via `matchRoots()` (line ~10099), run every step.
- **Greedy x4** (`assign4`): Greedy nearest-neighbor every 4th step. Default — balances accuracy and speed since O(n^2) matching is expensive at high frame rates.

The match strategy is stored persistently in worker state as `S_matchStrategy` (line ~10459) and is serialized via `matchStrategy` in the worker init data.

### Which bitmap color modes use matching

There are six bitmap color modes (`bitmapColorMode`, line ~1012):

| Mode | Key | Root matching | Notes |
|------|-----|--------------|-------|
| Uniform | `uniform` | None | All roots painted the same color; identity irrelevant. |
| Index Rainbow | `rainbow` | Configurable (`bitmapMatchStrategy`) | Root index determines color via rainbow palette. |
| Derivative | `derivative` | Hardcoded greedy every 4th step | Uses `computeSens()` for Jacobian sensitivity coloring. Not configurable. |
| Root Proximity | `proximity` | None | Color based on nearest-neighbor distance; identity irrelevant. |
| Idx x Prox | `idx-prox` | Configurable (`bitmapMatchStrategy`) | Combines rainbow hue with proximity brightness. |
| Min/Max Ratio | `ratio` | None | Color based on min/max inter-root distance ratio; identity irrelevant. |

### UI for match strategy

Match strategy chips (small labeled buttons: "Hungarian", "Greedy x1", "Greedy x4") appear in the bitmap config popup under two modes:

- **Index Rainbow**: chips at line ~8101, nested under the rainbow row.
- **Idx x Prox**: chips at line ~8180, nested under the idx-prox row.

Clicking a chip sets `bitmapMatchStrategy` and, if the current mode is different, switches to the corresponding color mode.

## Implementation details

### `matchRootOrder(newRoots, oldRoots)` — line ~5398

Main-thread greedy matcher operating on `{re, im}` object arrays. For each old root *i*, finds the closest unused new root *j* by squared Euclidean distance. Returns a reordered array.

### `matchRoots(newRe, newIm, oldRe, oldIm, n)` — line ~10099

Worker blob greedy matcher operating on parallel `Float64Array` typed arrays. Same algorithm as `matchRootOrder` but avoids object allocation. Modifies `newRe`/`newIm` in place.

### `hungarianMatch(newRe, newIm, oldRe, oldIm, n)` — line ~10115

Worker blob Kuhn-Munkres O(n^3) optimal assignment. Builds an n x n squared-distance cost matrix, applies the Hungarian algorithm with 1-indexed potentials, and reorders `newRe`/`newIm` in place. Produces the globally optimal assignment rather than the locally greedy one.

### Jump threshold

All trail renderers use the same formula:

```
jumpThresh = panelRange * 0.3
```

where `panelRange` is the visible extent of the roots or coefficient panel. A distance exceeding this between consecutive trail points is interpreted as a root-identity swap (the greedy matcher assigned the wrong root) rather than genuine root motion, and a path break is inserted.
