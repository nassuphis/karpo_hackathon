# Fix Debug Histogram

This document corrects the current `Histogram` debug output for `Solve score`.

The current implementation is misleading:

- it prints a **32-bin full-range histogram**
- but the renderer does **not** color from that view
- the renderer colors from **10 final equal-density bins**
- those 10 bins are derived **after clipping** and **after 100-bin intermediate aggregation**

So the current debug log answers the wrong question.

The histogram debug output must be changed to show the **actual 10 bins used for coloring**.


## 1. Goal

When the user presses `Histogram`, the Render log must make it easy to answer:

- what score range is being clipped in?
- how many lores solves fall below clip, inside clip, and above clip?
- what are the **10 final color bins actually used**?
- how many solves land in each of those 10 bins?
- where are the cut boundaries in score units?
- are there pathological outliers or saturation walls distorting the picture?

This is a **debugging view of the real coloring pipeline**, not a generic stats dump.


## 2. Primary Requirement

The default histogram output must show the **10 final bins actually used by solve-score coloring**.

It must **not** default to:

- 32 full-range bins
- 100 intermediate bins
- any other bin count unrelated to the final colorization

If any secondary histogram view is retained, it must be clearly labeled as auxiliary and shown **after** the 10-bin view.


## 3. What “Actual 10 Bins” Means

The debug path must mirror the real solve-score binning logic:

1. compute lores solve scores
2. compute `clip_lo` / `clip_hi` from `q`
3. build the same **100-bin clipped histogram** used by the real solve-score pipeline
4. derive the same **10 equal-density cuts**
5. map the lores solves into those final 10 bins
6. report those final 10 bins in the log

The authoritative logic to mirror is here:

- intermediate histogram + merge logic in [handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py#L288)

The current misleading summary implementation that must be replaced is here:

- 32-bin full-range summary in [solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c#L246)


## 4. Backend Change

Files:

- [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)
- [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)

### 4.1 Summary mode must emit colorization-relevant data

`--mode=summary` must return JSON containing at least:

- `mode`
- `metric`
- `n_solves`
- `degree`
- `min_score`
- `max_score`
- `mean_score`
- `stddev_score`
- `q05`
- `q10`
- `q25`
- `q50`
- `q75`
- `q90`
- `q95`
- `clip_quantile`
- `clip_lo`
- `clip_hi`
- `full_range`
- `clip_range`
- `clip_below_count`
- `clip_inrange_count`
- `clip_above_count`
- `clip_below_frac`
- `clip_inrange_frac`
- `clip_above_frac`
- `clip_fallback`
- `clip_fallback_reason`

And, critically, the new fields:

- `intermediate_hist_bins`
  - must be `100`
- `final_bins`
  - must be `10`
- `cuts_norm`
  - 9 normalized cut positions in `[0,1]`
- `cuts_score`
  - 9 score-space cut values derived from `clip_lo + cuts_norm[i] * (clip_hi - clip_lo)`
- `final_bin_counts`
  - array of length 10
- `final_bin_fracs`
  - array of length 10

### 4.2 Remove the misleading primary histogram

The current summary JSON field:

- `hist_full`

must no longer be the primary thing shown in the log.

Preferred approach:

- remove `hist_full` entirely from summary mode

Acceptable fallback:

- keep it only as an explicitly auxiliary field, renamed to something like:
  - `full_range_hist_aux`

But it must no longer drive the default Render-log output.

### 4.3 Add outlier/saturation diagnostics

To explain bizarre cases like “everything in bin 0 plus a few values at 150”, summary mode must also emit:

- `min_score_count`
  - number of solves exactly equal to `min_score`
- `max_score_count`
  - number of solves exactly equal to `max_score`
- `clip_lo_count`
  - number of solves exactly equal to `clip_lo`
- `clip_hi_count`
  - number of solves exactly equal to `clip_hi`

Optional but useful:

- `n_unique_scores`

This is especially important for `proximity`, where a numerical wall can occur from:

- [solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h#L31)


## 5. Final Bin Assignment Rule

The summary mode must define final bin membership exactly the same way the render path does.

Required rule:

- scores below `clip_lo` count as `below clip`, not as bin `0`
- scores above `clip_hi` count as `above clip`, not as bin `9`
- `final_bin_counts` must count only solves **inside** the clipped range
- the 10 displayed bin counts must therefore sum to `clip_inrange_count`

This matters because the user needs to distinguish:

- “the first color bin has most of the in-range mass”

from:

- “almost everything got clipped below range”

Those are different failure modes.


## 6. Frontend Log Format

File:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

### 6.1 Fixed-spacing requirement

The histogram block must render in a fixed-width style.

Required:

- `render-log` must explicitly use a fixed-width font
- histogram rows must preserve column alignment

Do not rely on page-global font defaults.

Preferred implementation:

- add explicit CSS for `#render-log`:
  - `font-family: monospace`
  - `white-space: pre-wrap`

Better implementation:

- add a dedicated helper like `logBlock(lines, target)` that appends a `<pre>` block

Do not build a multi-column ASCII table from ordinary proportional text nodes.

### 6.2 Required log structure

The log block must be compact, readable, and in this order:

1. header
2. quantiles/stat summary
3. clip summary
4. final 10-bin color table
5. outlier/saturation summary
6. timing summary

### 6.3 Required header block

Example:

```text
Solve histogram: metric=proximity, q=0.1%, n=3600, degree=70
  lores-only debug; mirrors actual 10 solve-score color bins
```

### 6.4 Required quantile/stat block

Example:

```text
  min=1.070  p05=1.120  p10=1.148  p25=1.220  p50=1.335  p75=1.433  p90=1.546  p95=1.634  max=150.000
  mean=3.815  std=19.032
```

### 6.5 Required clip block

Example:

```text
  clip[q..1-q]: lo=1.075  hi=150.000  width=148.925 / full=148.930
  occupancy: below=3 (0.1%)  in=3597 (99.9%)  above=0 (0.0%)
  fallback: none
```

If there is a fallback reason, print it explicitly:

```text
  fallback: degenerate_quantile_range
```

### 6.6 Required 10-bin table

This is the core of the feature.

The log must show a 10-row table for the real final color bins.

Required columns:

- final bin index `0..9`
- score interval
- count
- fraction of in-range solves
- ASCII bar

Example:

```text
  final color bins (10, actual solve-score bins):
    b0  [1.075 .. 1.121)   361  10.0%  |##########
    b1  [1.121 .. 1.148)   360  10.0%  |##########
    b2  [1.148 .. 1.190)   360  10.0%  |##########
    b3  [1.190 .. 1.238)   359  10.0%  |##########
    b4  [1.238 .. 1.292)   360  10.0%  |##########
    b5  [1.292 .. 1.357)   359  10.0%  |##########
    b6  [1.357 .. 1.433)   360  10.0%  |##########
    b7  [1.433 .. 1.546)   359  10.0%  |##########
    b8  [1.546 .. 1.634)   360  10.0%  |##########
    b9  [1.634 .. 150.000] 360  10.0%  |##########
```

Formatting rules:

- use aligned fixed-width columns
- use `[` `..` `)` for bins `0..8`
- use `[` `..` `]` for the last bin
- bars should be scaled to the largest displayed bin
- use plain ASCII only

### 6.7 Optional color cue

If desired, each row may include the palette-bin number only:

- `b0` through `b9`

Do not attempt colored terminal text in the log.

Do not add emoji or Unicode blocks.

### 6.8 Required outlier/saturation block

Example:

```text
  extremes: min_count=1  max_count=60  clip_lo_count=0  clip_hi_count=60
```

If `max_score_count` or `clip_hi_count` is nontrivial, this makes the “wall at 150” obvious.

### 6.9 Timing block

Example:

```text
  lores timings: download=141ms  compute=28ms
```


## 7. Nice ASCII Art Requirement

The 10-bin table must look intentional, not like random log spam.

Required:

- aligned columns
- fixed-width font
- one compact table
- bars scaled consistently

Bar style:

- use `#`
- empty bars may be blank after `|`
- maximum width 24 characters

Do not:

- print 10 unrelated free-form sentences
- print JSON into the log
- print 32 bins by default
- prepend variable-width labels that ruin alignment


## 8. Optional Secondary Diagnostic

If you want one extra view, the only acceptable secondary histogram is:

- a small histogram over the **clipped range**

Example:

- 20 bins across `[clip_lo, clip_hi]`

This can help show whether the clipped range itself is concentrated near one edge.

But it must be clearly labeled:

```text
  auxiliary clipped-range shape (20 bins):
```

And it must appear **after** the 10-bin table.

Do not show the current 32-bin full-range histogram first.


## 9. Frontend Text Changes

Current misleading line:

- `histogram (${s.hist_bins} bins, full range):`

in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html#L1409)

must be removed or replaced.

New wording must explicitly say:

- `final color bins (10, actual solve-score bins):`


## 10. Tests

### 10.1 Binary tests

File:

- [tests/test_solve_proximity_stats.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_stats.py)

Add tests that validate summary mode now returns:

- `final_bins == 10`
- `intermediate_hist_bins == 100`
- `len(cuts_norm) == 9`
- `len(cuts_score) == 9`
- `len(final_bin_counts) == 10`
- `sum(final_bin_counts) == clip_inrange_count`
- `final_bin_counts` are consistent with the same cut derivation logic

Add one regression case matching the user-visible pathological shape:

- most solves near `min_score`
- a small number pegged at `max_score`

Required assertion:

- summary must make the outlier wall obvious via:
  - `max_score_count`
  - `clip_hi_count`
- and must still emit 10 final bins, not 32 full-range bins

### 10.2 Frontend harness tests

File:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

Add tests that:

1. mock summary payload with `final_bin_counts`
2. run `runSolveScoreHistogramDebug()`
3. assert the log contains:
   - `final color bins (10, actual solve-score bins):`
   - 10 `b0..b9` rows
   - aligned-looking rows
   - no `32 bins, full range`

Also add one test where:

- `max_score_count > 0`

and assert the outlier summary line appears.


## 11. Anti-Cheat Rules

Reject the implementation if any of these happen:

1. It still logs the 32-bin full-range histogram as the main view.
2. It prints “10 bins” but those bins are not the real final solve-score bins.
3. It computes exact deciles directly from sorted scores while the real renderer still uses a different 100-bin approximation path.
4. It silently folds below-clip or above-clip solves into bin `0` or bin `9`.
5. It prints JSON blobs into the log instead of a readable table.
6. It relies on proportional-font rendering for ASCII alignment.
7. It omits `max_score_count` / `clip_hi_count`, leaving saturation walls invisible.


## 12. Acceptance Criteria

This fix is complete only when:

1. Pressing `Histogram` shows the **actual 10 solve-score color bins**.
2. The log clearly distinguishes:
   - below clip
   - in range
   - above clip
3. The log clearly shows if a hard outlier wall is dominating the score range.
4. The log block is readable in fixed-width form.
5. No default output line refers to `32 bins, full range`.

