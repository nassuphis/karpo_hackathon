# Cubic / Quartic Solver Findings

This note documents the current issues in the analytic cubic and quartic solvers used by the `roots5` and `roots6` parameter transforms in [`lambda/sweep_cli.c`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c).

## Scope

Relevant code:

- [`_solve_cubic()`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2584)
- [`_solve_quartic()`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2620)
- [`pt_roots5()`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2699)
- [`pt_roots6()`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2714)

The transform setup in `pt_roots5()` and `pt_roots6()` is fine. The main problem is the shared solver core.

## Finding 1: Cubic Solver Is Wrong For Complex Coefficients

Severity: high

Current implementation:

- `_solve_cubic()` computes:
  - `u = cbrt(-q/2 + sqrt(delta))`
  - `v = cbrt(-q/2 - sqrt(delta))`
- This happens independently at:
  - [`_solve_cubic()` line 2607](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2607)
  - [`_solve_cubic()` line 2608](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2608)

Why this is wrong:

- For Cardano over complex numbers, `u` and `v` cannot be chosen independently from principal cube roots.
- They must satisfy:

```text
u * v = -p / 3
```

- If that relation is broken, the three reported values are often not roots of the original cubic.

Observed behavior:

- Randomized residual testing against `numpy.polyval()` showed the current implementation fails badly on generic complex cubics.
- In local checking:
  - current implementation: `2638 / 4000` random complex cubics had residuals greater than `1e-5`
  - fixed implementation using `v = -(p/3)/u`: `0 / 4000`

Implication:

- `roots5()` is currently unreliable whenever the cubic coefficients are genuinely complex, which they are.

## Finding 2: Quartic Solver Inherits The Cubic Bug

Severity: high

Current implementation:

- `_solve_quartic()` uses Ferrari reduction and solves the resolvent cubic at:
  - [`_solve_quartic()` line 2659](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2659)

Why this matters:

- If `_solve_cubic()` is wrong, the chosen resolvent root can be wrong.
- That corrupts the subsequent `s = sqrt(2m - p)` and both quadratics.

Observed behavior:

- Randomized residual testing against `numpy.polyval()` showed the current quartic path also fails badly on generic complex quartics.
- In local checking:
  - current implementation: `1349 / 2000` random complex quartics had residuals greater than `1e-5`
  - after fixing the cubic branch logic: `0 / 2000`

Implication:

- `roots6()` is also unreliable, even if the quartic reduction itself is mostly correct.

## Finding 3: Quartic Has An Extra Robustness Hole At `s ~= 0`

Severity: medium

Current implementation:

- `_solve_quartic()` computes:
  - `s = sqrt(2m - p)` at [`line 2669`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2669)
- If `|s|` is tiny, it tries another resolvent root at:
  - [`lines 2671-2674`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2671)
- But it still divides by `2*s` unconditionally at:
  - [`line 2676`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2676)

Why this is a bug:

- If the fallback root also gives `s ~= 0`, the division produces NaNs or huge garbage.
- This is separate from the main cubic bug.

Implication:

- Even after fixing the cubic branch selection, quartic still needs one more guard.

## What To Change

### 1. Fix `_solve_cubic()`

Keep the depressed-cubic setup exactly as it is:

- normalize to monic
- compute `p`
- compute `q`
- compute `delta`
- compute `sqrt(delta)`

Then change only the `u` / `v` selection logic.

Current code:

```c
_cbrt_c(-qr/2 + sdr, -qi/2 + sdi, &ur, &ui);
_cbrt_c(-qr/2 - sdr, -qi/2 - sdi, &vr, &vi);
```

Replace with:

1. Compute `u` from the first branch as now.
2. If `|u|` is not tiny, compute:

```text
v = -(p/3) / u
```

3. Only if `|u|` is tiny, fall back to:

```text
v = cbrt(-q/2 - sqrt(delta))
```

Concrete implementation shape:

```c
_cbrt_c(-qr/2 + sdr, -qi/2 + sdi, &ur, &ui);
double umag = ur*ur + ui*ui;
if (umag > 1e-24) {
    c_div(-pr/3, -pi_/3, ur, ui, &vr, &vi);
} else {
    _cbrt_c(-qr/2 - sdr, -qi/2 - sdi, &vr, &vi);
}
```

Why this fixes it:

- It enforces the required Cardano relation `u*v = -p/3`.
- The cube-root branch ambiguity is no longer free to drift.

### 2. Keep The Existing Root Reconstruction

After the `u` / `v` fix, keep:

```text
y0 = u + v
y1 = omega*u + omega^2*v
y2 = omega^2*u + omega*v
xk = yk - A/3
```

The reconstruction using `omega` / `omega^2` is fine.

### 3. Add A Final `s` Guard In `_solve_quartic()`

After the fallback root selection:

```c
double smag = sr*sr + si*si;
if (smag < 1e-60) {
    return 0; /* or degrade to a safe fallback */
}
```

Place this immediately before:

```c
c_div(-qr, -qi, 2*sr, 2*si, &tr, &ti);
```

Recommended behavior:

- simplest safe option: return `0`
- caller already handles `n <= 0` by zeroing outputs in `_roots_minmax()`

This is better than emitting NaNs.

## Optional Improvement: Use Residuals To Choose The Best Resolvent Root

Current quartic code chooses the resolvent root with largest magnitude:

- [`_solve_quartic()` lines 2661-2666](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c#L2661)

That heuristic is not obviously justified.

Better option:

1. For each candidate resolvent root:
   - build `s`
   - build the two quadratics
   - produce up to four candidate roots
2. Evaluate polynomial residuals for those roots
3. Keep the candidate set with the smallest max residual

This is not required to fix the major bug.
Do the `u*v = -p/3` fix first.

## What Not To Change

These parts look correct:

- `pt_roots5()` coefficient construction:
  - `a = cos(100*t1)`
  - `b = i*t1`
  - `c = i*t2`
  - `d = sin(100*t2)`
- `pt_roots6()` coefficient construction:
  - `a = sin(5*t1)`
  - `b = i*t1`
  - `c = (t1-t2)^3 + (t1+t2)^2 + t1*t2 + 1`
  - `d = i*t2`
  - `e = sin(t2)`
- `_roots_minmax()` magnitude selection

So the fix belongs in the solver internals, not the transform wrappers.

## Verification Plan

### 1. Add Deterministic Unit Checks

Add small tests for:

- cubic with real coefficients and known roots
- cubic with complex coefficients and known roots
- quartic with real coefficients and known roots
- quartic with complex coefficients and known roots

For each:

- solve analytically with `_solve_cubic()` / `_solve_quartic()`
- evaluate the original polynomial at each returned root
- assert max residual is below a tight threshold, e.g. `1e-8` or `1e-10`

### 2. Add Randomized Residual Tests

For randomized complex coefficients:

- cubic:
  - generate random `a,b,c,d` with `|a|` bounded away from zero
  - solve analytically
  - verify residuals

- quartic:
  - same for `a,b,c,d,e`

This is the fastest way to catch branch-choice bugs.

### 3. Cross-check `roots5` / `roots6`

For random `t1`, `t2`:

- build coefficients in Python
- compare min/max-by-magnitude roots from:
  - analytic solver
  - `numpy.roots`

They do not need identical ordering of all roots, only agreement on:

- smallest magnitude root
- largest magnitude root

### 4. Check Degenerate Fallbacks

Explicitly test:

- cubic with `a ~= 0` reducing to quadratic
- quartic with `a ~= 0` reducing to cubic
- quartic biquadratic case `q ~= 0`
- quartic `s ~= 0` path

## Recommended Fix Order

1. Fix `_solve_cubic()` by enforcing `v = -(p/3)/u`
2. Add cubic residual tests
3. Add the final `s` guard in `_solve_quartic()`
4. Add quartic residual tests
5. Only then decide whether resolvent-root selection needs refinement

## Bottom Line

The main defect is not in `roots5()` or `roots6()`.

It is in `_solve_cubic()`:

- independent principal cube roots are invalid for Cardano over complex coefficients

That one bug propagates directly into the quartic solver.

Fixing the cubic branch relation is the primary repair.
