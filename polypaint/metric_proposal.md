# Metric Proposal for `solve_score.h`

This document proposes a concrete set of additional solve-score metrics that fit the current design of `solve_score.h`:

- exact string metric names
- enum-backed dispatch
- no new DSL grammar required
- scalar output only
- same "compute metric from one row" model as the current code

The bias here is toward metrics that add **new geometric information**, not just slightly different versions of `spread`, `area`, or `proximity`.

A practical theme runs through all of them:

- prefer `O(n)` or `O(n + B)` metrics when possible
- use fixed names rather than parameterized metrics for v1
- stay compatible with the current source model (`slv`, `cf`, and sometimes `pm`)
- keep "larger means more of the named thing" when possible

---

## Proposed implementation order

If you want the highest value per line of code, implement these first:

1. `mean_log_mod`
2. `sd_log_mod`
3. `inside_unit_fraction`
4. `unit_annulus_fraction_01`
5. `imag_axis_proximity`
6. `diagonal_proximity`
7. `angular_entropy_16`
8. `angular_order_2`
9. `angular_order_3`
10. `angular_order_4`
11. `conjugation_defect`
12. `antipodal_defect`
13. `radial_bimodality`
14. `sector_max_share_16`

Stretch goals:

15. `nn_gini`
16. `hull_compactness`
17. `hull_fill`

---

## Naming and compatibility notes

Your current parser only accepts exact metric names. So this proposal uses **fixed identifiers** instead of parameterized forms like `annulus_fraction(delta=0.1)`.

That is why this document proposes names such as:

- `unit_annulus_fraction_01`
- `angular_entropy_16`
- `sector_max_share_16`

where the suffix encodes the fixed parameter.

If later you want metric parameters in the DSL, these fixed-name metrics can be the first instances of a more general family.

---

## Recommended source support

For v1, I would keep these rules:

### Solve/Coeff only
These are meaningful for root clouds and should support:

- `SOLVE_SCORE_SOURCE_SOLVE`
- `SOLVE_SCORE_SOURCE_COEFF`

This should apply to:

- `mean_log_mod`
- `sd_log_mod`
- `inside_unit_fraction`
- `unit_annulus_fraction_01`
- `imag_axis_proximity`
- `diagonal_proximity`
- `angular_entropy_16`
- `angular_order_2`
- `angular_order_3`
- `angular_order_4`
- `conjugation_defect`
- `antipodal_defect`
- `radial_bimodality`
- `sector_max_share_16`
- `nn_gini`
- `hull_compactness`
- `hull_fill`

### Optional param support later
A subset can reasonably support `SOLVE_SCORE_SOURCE_PARAM` too if you want it later:

- `mean_log_mod`
- `sd_log_mod`
- `inside_unit_fraction`
- `unit_annulus_fraction_01`
- `angular_order_2`
- `angular_order_3`
- `angular_order_4`

But I would not do that in v1 unless there is a real use case.

---

## Shared helper functions worth adding

These will make several metrics cleaner.

### 1. `compute_moduli(...)`
```c
static void compute_moduli(const float *roots, int degree, double *mod_out) {
    for (int i = 0; i < degree; i++) {
        double re = roots[i * 2];
        double im = roots[i * 2 + 1];
        mod_out[i] = hypot(re, im);
    }
}
```

### 2. `compute_log_moduli(...)`
```c
static void compute_log_moduli(const float *roots, int degree, double *logmod_out) {
    for (int i = 0; i < degree; i++) {
        double re = roots[i * 2];
        double im = roots[i * 2 + 1];
        logmod_out[i] = log(hypot(re, im) + SOLVE_SCORE_EPS);
    }
}
```

Use natural log or `log10`, but be consistent. I would use `log` internally unless there is a reason to prefer `log10`.

### 3. `compute_angles(...)`
```c
static int compute_angles_skip_zero(const float *roots, int degree, double *angle_out) {
    int out = 0;
    for (int i = 0; i < degree; i++) {
        double re = roots[i * 2];
        double im = roots[i * 2 + 1];
        if (re == 0.0 && im == 0.0) continue;
        angle_out[out++] = wrapped_angle_0_2pi(re, im);
    }
    return out;
}
```

### 4. `fill_angle_histogram_16(...)`
```c
static void fill_angle_histogram_16(const double *angles, int n, int counts[16]) {
    for (int b = 0; b < 16; b++) counts[b] = 0;
    if (n <= 0) return;
    const double scale = 16.0 / (2.0 * M_PI);
    for (int i = 0; i < n; i++) {
        int bin = (int)(angles[i] * scale);
        if (bin < 0) bin = 0;
        if (bin > 15) bin = 15;
        counts[bin]++;
    }
}
```

### 5. `mean_abs_complex_match_error(...)`
Useful for `conjugation_defect` and `antipodal_defect`.
This can start as an `O(n^2)` nearest-target search.

---

## Concrete metrics

---

## 1. `mean_log_mod`

### What it measures
Whether the cloud lives mainly **inside** or **outside** the unit circle.

Unlike `dist_unit_circle`, this preserves the sign of radial bias.

- roots mostly inside the unit circle -> negative
- roots mostly outside the unit circle -> positive
- roots centered around radius 1 -> near zero

### Definition
For roots \(z_j\),
\[
\text{mean\_log\_mod} = \frac{1}{n} \sum_{j=1}^n \log(|z_j| + \varepsilon)
\]

### Output direction
Larger means roots are farther outside the unit circle on average.

### Complexity
`O(n)`

### Minimum roots
`1`

### Suggested source support
`slv`, `cf`

### Implementation sketch
```c
double sum = 0.0;
for (int i = 0; i < degree; i++) {
    double re = roots[i * 2];
    double im = roots[i * 2 + 1];
    sum += log(hypot(re, im) + SOLVE_SCORE_EPS);
}
return sum / degree;
```

### Edge cases
- Exact zero root contributes `log(EPS)`.
- That is fine as long as you accept that many exact zeros drive the metric strongly negative.

### Why it adds information
`dist_unit_circle` cannot distinguish radius `0.5` from radius `2.0` very well; this one does immediately.

---

## 2. `sd_log_mod`

### What it measures
How broad the radial distribution is on a multiplicative scale.

This is often more informative than a plain modulus standard deviation.

### Definition
\[
\text{sd\_log\_mod} = \operatorname{sd}(\log(|z_j| + \varepsilon))
\]

### Output direction
Larger means more multiplicative radial spread.

### Complexity
`O(n)`

### Minimum roots
`2`

### Suggested source support
`slv`, `cf`

### Implementation sketch
Use one pass for the mean and one pass for the variance, or reuse a temp buffer.

```c
double mean = 0.0;
for (int i = 0; i < degree; i++) {
    double lm = log(hypot(roots[i * 2], roots[i * 2 + 1]) + SOLVE_SCORE_EPS);
    mean += lm;
}
mean /= degree;

double ss = 0.0;
for (int i = 0; i < degree; i++) {
    double lm = log(hypot(roots[i * 2], roots[i * 2 + 1]) + SOLVE_SCORE_EPS);
    double d = lm - mean;
    ss += d * d;
}
return sqrt(ss / degree);
```

### Edge cases
- All roots at the same radius -> `0`
- Exact zeros are allowed and just map to very negative finite values

### Why it adds information
This catches inner/outer radial mixtures better than `spread`, especially when the geometry is basically radial.

---

## 3. `inside_unit_fraction`

### What it measures
The fraction of roots strictly inside the unit circle.

### Definition
\[
\text{inside\_unit\_fraction} = \frac{1}{n} \sum_j 1\{|z_j| < 1\}
\]

### Output direction
Larger means more mass inside the unit circle.

### Complexity
`O(n)`

### Minimum roots
`1`

### Suggested source support
`slv`, `cf`

### Implementation sketch
```c
int inside = 0;
for (int i = 0; i < degree; i++) {
    double r = hypot(roots[i * 2], roots[i * 2 + 1]);
    if (r < 1.0) inside++;
}
return (double)inside / (double)degree;
```

### Edge cases
- Roots exactly on the circle count as outside with the strict inequality.
- That is fine; just document it.

### Why it adds information
This is a brutally simple but very useful statistic. It often separates regimes immediately.

---

## 4. `unit_annulus_fraction_01`

### What it measures
The fraction of roots close to the unit circle, using a fixed band width of `0.1`.

### Definition
\[
\text{unit\_annulus\_fraction\_01} = \frac{1}{n} \sum_j 1\{ ||z_j| - 1| < 0.1 \}
\]

### Output direction
Larger means more roots hug the unit circle.

### Complexity
`O(n)`

### Minimum roots
`1`

### Suggested source support
`slv`, `cf`

### Implementation sketch
```c
int in_band = 0;
for (int i = 0; i < degree; i++) {
    double r = hypot(roots[i * 2], roots[i * 2 + 1]);
    if (fabs(r - 1.0) < 0.1) in_band++;
}
return (double)in_band / (double)degree;
```

### Edge cases
- Strict `< 0.1` is fine.
- If you want exact boundary inclusion, use `<=`.

### Why it adds information
This differs from `dist_unit_circle`. A cloud with half the points exactly on the circle and half far away is very different from one where every point is moderately close.

---

## 5. `imag_axis_proximity`

### What it measures
How close the cloud lies to the imaginary axis.

This is the natural counterpart of `real_axis_proximity`.

### Definition
\[
\text{imag\_axis\_proximity} = -\log_{10}(\operatorname{median}(|\Re z_j|) + \varepsilon)
\]

### Output direction
Larger means the cloud lies closer to the imaginary axis.

### Complexity
`O(n log n)` if you use sort for the median

### Minimum roots
`1`

### Suggested source support
`slv`, `cf`

### Implementation sketch
Exactly parallel to `real_axis_proximity`, replacing `|Im|` with `|Re|`.

### Why it adds information
Some root clouds organize vertically, and your current metric list does not see that directly.

---

## 6. `diagonal_proximity`

### What it measures
How tightly roots align with the diagonals `|Re| = |Im|`.

This catches "X-shaped" structure.

### Definition
\[
\text{diagonal\_proximity} =
-\log_{10}(\operatorname{median}(|\,|\Re z_j| - |\Im z_j|\,|) + \varepsilon)
\]

### Output direction
Larger means roots sit closer to the diagonal lines.

### Complexity
`O(n log n)` if using median via sort

### Minimum roots
`1`

### Suggested source support
`slv`, `cf`

### Implementation sketch
```c
for (int i = 0; i < degree; i++) {
    double re = fabs(roots[i * 2]);
    double im = fabs(roots[i * 2 + 1]);
    buf[i] = fabs(re - im);
}
double med = median_inplace(buf, degree);
return -log10(med + SOLVE_SCORE_EPS);
```

### Why it adds information
This gives you a simple detector for diagonal symmetry or arm structure that covariance alone often misses.

---

## 7. `angular_entropy_16`

### What it measures
How evenly the roots occupy angle space around the origin.

### Definition
Ignore exact zero roots. Compute angles in `[0, 2π)`, bin them into `16` equal sectors, and let `p_b` be the normalized bin masses.

Use normalized Shannon entropy:
\[
H = -\sum_{b=1}^{16} p_b \log(p_b)
\]
\[
\text{angular\_entropy\_16} = \frac{H}{\log(16)}
\]

### Output direction
- near `0`: roots occupy only a few angular sectors
- near `1`: roots are spread roughly uniformly in angle

### Complexity
`O(n + B)` with `B = 16`

### Minimum roots
`2` nonzero roots is a sensible minimum

### Suggested source support
`slv`, `cf`

### Implementation sketch
```c
int counts[16];
fill_angle_histogram_16(angles, angle_count, counts);
double H = 0.0;
for (int b = 0; b < 16; b++) {
    if (counts[b] == 0) continue;
    double p = (double)counts[b] / (double)angle_count;
    H -= p * log(p);
}
return H / log(16.0);
```

### Edge cases
- If there are no nonzero roots, return `0.0`.
- If there is only one nonzero root, returning `0.0` is also reasonable.

### Why it adds information
A ring with four spikes and a ring with uniform occupancy can have similar area and shelliness but very different angular entropy.

---

## 8. `angular_order_2`

### What it measures
Strength of latent 2-fold angular symmetry.

### Definition
Ignoring exact zeros, let `theta_j` be the wrapped angles.
\[
\text{angular\_order\_2} =
\left| \frac{1}{n} \sum_j e^{2 i \theta_j} \right|
\]

Equivalent real form:
\[
\left( \left(\frac{1}{n}\sum_j \cos(2\theta_j)\right)^2 +
       \left(\frac{1}{n}\sum_j \sin(2\theta_j)\right)^2 \right)^{1/2}
\]

### Output direction
Larger means stronger 2-fold symmetry: opposite lobes, line-like, or antipodal structure.

### Complexity
`O(n)`

### Minimum roots
`2` nonzero roots

### Suggested source support
`slv`, `cf`

### Implementation sketch
```c
double c = 0.0, s = 0.0;
for (int i = 0; i < degree; i++) {
    double re = roots[i * 2];
    double im = roots[i * 2 + 1];
    if (re == 0.0 && im == 0.0) continue;
    double th = wrapped_angle_0_2pi(re, im);
    c += cos(2.0 * th);
    s += sin(2.0 * th);
}
return hypot(c, s) / nonzero_count;
```

### Why it adds information
This is a very strong detector of "two-lobed" or axis-like angular organization.

---

## 9. `angular_order_3`

Same as `angular_order_2`, but with `3 * theta`.

### Definition
\[
\text{angular\_order\_3} =
\left| \frac{1}{n} \sum_j e^{3 i \theta_j} \right|
\]

### Output direction
Larger means stronger 3-fold symmetry.

### Complexity
`O(n)`

### Minimum roots
`2` nonzero roots

### Suggested source support
`slv`, `cf`

### Why it adds information
This picks up triangular / three-arm structure that your current metrics do not see.

---

## 10. `angular_order_4`

Same as above, but with `4 * theta`.

### Definition
\[
\text{angular\_order\_4} =
\left| \frac{1}{n} \sum_j e^{4 i \theta_j} \right|
\]

### Output direction
Larger means stronger 4-fold symmetry.

### Complexity
`O(n)`

### Minimum roots
`2` nonzero roots

### Suggested source support
`slv`, `cf`

### Why it adds information
This is a clean detector of cross-like or square-like angular structure.

---

## 11. `conjugation_defect`

### What it measures
How far the cloud is from being invariant under complex conjugation.

This is especially useful when the underlying polynomial often has nearly real coefficients or nearly conjugate-paired roots.

### Definition
For each root \(z_i\), compute the distance to the nearest conjugate of another root:
\[
d_i = \min_j |z_i - \overline{z_j}|
\]
Then define
\[
\text{conjugation\_defect} = -\log_{10}\left( \frac{1}{n} \sum_i d_i + \varepsilon \right)
\]

### Output direction
Larger means the cloud is closer to exact conjugation symmetry.

### Complexity
Naive implementation: `O(n^2)`

### Minimum roots
`2`

### Suggested source support
`slv`, `cf`

### Implementation sketch
```c
double sum_min = 0.0;
for (int i = 0; i < degree; i++) {
    double zi_re = roots[i * 2];
    double zi_im = roots[i * 2 + 1];
    double best = 1e300;
    for (int j = 0; j < degree; j++) {
        double zj_re = roots[j * 2];
        double zj_im = -roots[j * 2 + 1]; /* conjugate target */
        double dr = zi_re - zj_re;
        double di = zi_im - zj_im;
        double d = hypot(dr, di);
        if (d < best) best = d;
    }
    sum_min += best;
}
return -log10(sum_min / degree + SOLVE_SCORE_EPS);
```

### Important note
Allowing `j == i` is okay. A root on the real axis is its own conjugate, which is mathematically correct and useful.

### Why it adds information
None of your current metrics directly detect conjugation symmetry.

---

## 12. `antipodal_defect`

### What it measures
How close the cloud is to symmetry under `z -> -z`.

### Definition
For each root \(z_i\), compute
\[
d_i = \min_j |z_i + z_j|
\]
Then
\[
\text{antipodal\_defect} = -\log_{10}\left( \frac{1}{n} \sum_i d_i + \varepsilon \right)
\]

### Output direction
Larger means stronger antipodal symmetry.

### Complexity
Naive implementation: `O(n^2)`

### Minimum roots
`2`

### Suggested source support
`slv`, `cf`

### Implementation sketch
Exactly parallel to `conjugation_defect`, but target `-z_j`.

### Why it adds information
This catches opposite-pair structure around the origin that covariance and spread do not directly capture.

---

## 13. `radial_bimodality`

### What it measures
Whether the radial distribution looks more like a single band or an inner/outer split.

### Definition
Use log-radii:
\[
x_j = \log(|z_j| + \varepsilon)
\]

Then compute:
\[
\text{radial\_bimodality} =
\frac{q_{0.9}(x) - q_{0.5}(x)}
     {q_{0.5}(x) - q_{0.1}(x) + \varepsilon}
\]

This is a quantile asymmetry statistic rather than a formal bimodality test, which is good: it is simple and robust.

### Output direction
- near `1`: roughly symmetric radial spread
- much greater than `1`: long upper radial tail / likely outer secondary band
- much less than `1`: long inner tail / likely inner secondary band

### Complexity
`O(n log n)` if implemented via sorting

### Minimum roots
`3` or more

### Suggested source support
`slv`, `cf`

### Implementation sketch
- compute log-radii into a buffer
- sort
- extract 10%, 50%, 90% quantiles
- form ratio

### Why it adds information
`shelliness` likes a thin ring. This metric separates "one ring" from "inner ring + outer ring" much better.

---

## 14. `sector_max_share_16`

### What it measures
How concentrated the angular mass is in the most populated direction bin.

### Definition
Using the same 16-bin angle histogram as `angular_entropy_16`,
\[
\text{sector\_max\_share\_16} = \max_b p_b
\]

### Output direction
Larger means a greater share of the cloud sits in one angular sector.

### Complexity
`O(n + B)`

### Minimum roots
`1` nonzero root

### Suggested source support
`slv`, `cf`

### Why it adds information
This is a simple "dominant direction" statistic. It often highlights directional bursts more starkly than entropy.

---

## 15. `nn_gini` (stretch goal)

### What it measures
Inequality of nearest-neighbor distances or nearest-neighbor closeness scores.

### Recommendation
Use **nearest-neighbor distance**, not the transformed `-0.5 * log10(d^2)` score, because Gini is easier to reason about on the original positive quantity.

Let `r_i` be the nearest-neighbor distance for root `i`.

### Definition
\[
\text{nn\_gini} =
\frac{\sum_i \sum_j |r_i - r_j|}
     {2 n \sum_i r_i + \varepsilon}
\]

### Output direction
Larger means local spacing is more unequal across the cloud.

### Complexity
- nearest-neighbor computation: `O(n^2)` naive
- Gini from sorted values: `O(n log n)` after NN distances are known

### Why it adds information
This is a more inequality-sensitive version of `nn_variation`.

---

## 16. `hull_compactness` (stretch goal)

### What it measures
Whether the overall occupied shape is compact / disk-like versus spiky / filamentary.

### Definition
Let `A` be convex hull area and `P` its perimeter. Then
\[
\text{hull\_compactness} = \frac{4 \pi A}{P^2 + \varepsilon}
\]

### Output direction
Larger means more compact.

### Complexity
Depends on convex hull algorithm, typically `O(n log n)`.

### Minimum roots
`3`

### Suggested source support
`slv`, `cf`

### Why it adds information
This sees hull geometry directly rather than through covariance.

### Implementation note
Only do this if you are willing to add a convex hull helper; otherwise skip for now.

---

## 17. `hull_fill` (stretch goal)

### What it measures
How filled-in the hull is versus how much the points only trace a boundary.

### Definition
One reasonable version is:
\[
\text{hull\_fill} =
\frac{\sqrt{\lambda_{\max} \lambda_{\min}}}
     {A_{\text{hull}} + \varepsilon}
\]

You may want a different scale factor, because covariance-area and hull area have different raw units. The key idea is ratio-of-interior-mass proxy to hull area.

### Output direction
Larger means the hull is more interior-filled.

### Complexity
Needs covariance plus convex hull.

### Implementation note
This is informative, but it is the least "drop-in" metric in this document.

---

## Enum / parser / serializer changes

For the core set, add enum values after the existing metrics. Example:

```c
SOLVE_METRIC_MEAN_LOG_MOD = 30,
SOLVE_METRIC_SD_LOG_MOD = 31,
SOLVE_METRIC_INSIDE_UNIT_FRACTION = 32,
SOLVE_METRIC_UNIT_ANNULUS_FRACTION_01 = 33,
SOLVE_METRIC_IMAG_AXIS_PROXIMITY = 34,
SOLVE_METRIC_DIAGONAL_PROXIMITY = 35,
SOLVE_METRIC_ANGULAR_ENTROPY_16 = 36,
SOLVE_METRIC_ANGULAR_ORDER_2 = 37,
SOLVE_METRIC_ANGULAR_ORDER_3 = 38,
SOLVE_METRIC_ANGULAR_ORDER_4 = 39,
SOLVE_METRIC_CONJUGATION_DEFECT = 40,
SOLVE_METRIC_ANTIPODAL_DEFECT = 41,
SOLVE_METRIC_RADIAL_BIMODALITY = 42,
SOLVE_METRIC_SECTOR_MAX_SHARE_16 = 43,
```

Stretch goals can be added later.

Then update:

- `parse_solve_metric(...)`
- `solve_metric_name(...)`
- `solve_metric_min_roots(...)`
- `solve_metric_supports_source(...)`
- `compute_solve_metric_score(...)`

---

## Suggested minimum-root rules

I would use:

- `1`: `mean_log_mod`, `inside_unit_fraction`, `unit_annulus_fraction_01`, `imag_axis_proximity`, `diagonal_proximity`, `sector_max_share_16`
- `2`: `sd_log_mod`, `angular_entropy_16`, `angular_order_2`, `angular_order_3`, `angular_order_4`, `conjugation_defect`, `antipodal_defect`, `nn_gini`
- `3`: `radial_bimodality`, `hull_compactness`, `hull_fill`

---

## Test cases to add

For each metric, add small synthetic root clouds where the expected behavior is obvious.

### A. Unit circle
Roots exactly on the unit circle, evenly spaced.

Expected:
- `mean_log_mod` near 0
- `sd_log_mod` near 0
- `inside_unit_fraction` = 0 or close depending on exact radii
- `unit_annulus_fraction_01` = 1
- `angular_entropy_16` high if many roots
- `angular_order_k` depends on the chosen root count

### B. Tiny inner cloud
All roots near 0.1 radius.

Expected:
- `mean_log_mod` strongly negative
- `inside_unit_fraction` = 1
- `unit_annulus_fraction_01` near 0

### C. Two antipodal points
Roots at `+1` and `-1`.

Expected:
- `angular_order_2` high
- `antipodal_defect` high
- `angular_entropy_16` low-ish
- `sector_max_share_16` moderate

### D. Conjugate pair
Roots at `a+bi` and `a-bi`.

Expected:
- `conjugation_defect` high

### E. Four diagonal points
Roots at `(±1, ±1)` scaled appropriately.

Expected:
- `diagonal_proximity` high
- `angular_order_4` high

### F. Two radial bands
Half the roots near radius `0.5`, half near `2.0`.

Expected:
- `sd_log_mod` elevated
- `radial_bimodality` far from 1

---

## Correlation notes vs existing metrics

This matters because some candidates are genuinely new and some are partly redundant.

### Strongly new information
These add genuinely new structure:

- `inside_unit_fraction`
- `imag_axis_proximity`
- `diagonal_proximity`
- `angular_entropy_16`
- `angular_order_2`
- `angular_order_3`
- `angular_order_4`
- `conjugation_defect`
- `antipodal_defect`
- `radial_bimodality`
- `sector_max_share_16`

### Some overlap but still useful
These overlap somewhat with existing metrics but are still valuable:

- `mean_log_mod` overlaps partly with `dist_unit_circle` and `centroid_dist`, but is much better at inside/outside bias
- `sd_log_mod` overlaps partly with `spread`, but is more specifically radial and multiplicative
- `nn_gini` overlaps with `nn_variation`
- `hull_compactness` overlaps partly with `anisotropy` and `area`, but at the hull level

---

## Final recommendation

If you want a clean, implementation-friendly v1, add exactly these 14:

1. `mean_log_mod`
2. `sd_log_mod`
3. `inside_unit_fraction`
4. `unit_annulus_fraction_01`
5. `imag_axis_proximity`
6. `diagonal_proximity`
7. `angular_entropy_16`
8. `angular_order_2`
9. `angular_order_3`
10. `angular_order_4`
11. `conjugation_defect`
12. `antipodal_defect`
13. `radial_bimodality`
14. `sector_max_share_16`

Then only add:

15. `nn_gini`
16. `hull_compactness`
17. `hull_fill`

if you find you still need more structure after trying the first set.

The biggest bang-for-buck subset is probably:

- `mean_log_mod`
- `inside_unit_fraction`
- `imag_axis_proximity`
- `diagonal_proximity`
- `angular_entropy_16`
- `angular_order_2`
- `angular_order_3`
- `angular_order_4`
- `conjugation_defect`
- `antipodal_defect`

That set is diverse, informative, and still fairly straightforward to implement.