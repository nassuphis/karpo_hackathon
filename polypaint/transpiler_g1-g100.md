# Remaining `g1`-`g100` Failures After Stub Hardening

Current state after the latest transpiler fixes:

- `53 PASS`
- `8 genuine FAIL`
- `38 STUB`

The 8 non-stub failures are:

- `g9`
- `g10`
- `g43`
- `g44`
- `g63`
- `g72`
- `g76`
- `g89`

This doc focuses on what those failures likely are and which ones look worth fixing in the transpiler.

## Overall Read

These 8 are not all the same kind of failure.

- `g43`, `g44` look like a specific missing lowering for fancy indexed assignment with constant index arrays.
- `g63`, `g72`, `g76` look like a specific type-inference bug where complex temporaries are being collapsed to real scalars.
- `g9` looks like a missing lowering for whole-array expressions built from `np.linspace(...)` arrays.
- `g89` looks like a math-lowering issue around complex exponents on a real base.
- `g10` is the main likely numeric-sensitivity candidate once the `nCoeffs` fix is applied and the batch is regenerated.

So this is a good place to stop widening the test surface and instead do one or two targeted transpiler improvements.

## Function Notes

### `g9`

Source shape:

- builds `rec = np.linspace(...)`
- builds `imc = np.linspace(...)`
- computes `cf = 100j * imc**9 + 100 * rec**9`

Why it still fails:

- this is a whole-array expression from array temporaries, not scalar coefficient assignment
- the old generated C path built the `rec[]` and `imc[]` arrays but never lowered the final vector expression into a loop that writes `cRe[k]` / `cIm[k]`

Likely fix:

- add lowering for `cf = <array expr>` when the RHS is an elementwise expression over same-length array temporaries
- specifically support:
  - array variable references
  - elementwise `+`, `-`, `*`
  - scalar complex multiply
  - elementwise power by real constant

Priority:

- medium

### `g10`

Source shape:

- all-scalar loop
- no fancy indexing
- no `np.roots`
- no list comprehensions
- no array temporaries other than `cf`

Why it is probably different from the others:

- structurally, this is the kind of function the transpiler should already handle
- after the `nCoeffs` bug is fixed and regenerated, any remaining mismatch is more likely to be:
  - coefficient dynamic-range / root sensitivity
  - accumulated numeric drift in complex exp/trig

Likely fix:

- do not guess
- compare Python vs C coefficient vectors at a few fixed `(t1, t2)` points before changing the transpiler
- if coeffs are close but images drift, this is mostly numeric / solver sensitivity
- if coeffs diverge materially, inspect `np.exp`, `np.sin`, and mixed real/complex arithmetic lowering

Priority:

- low to medium

### `g43`

Source shape:

- `i = np.array([0, 4, 14, 29], dtype=np.intp)`
- `cf[i] = np.array([1, -5, 10, -20], dtype=np.complex128)`

Likely current failure mode:

- fancy indexed assignment with a constant integer index array is not being lowered correctly
- earlier generated output showed the index array being emitted as a static C array, but then cast like a scalar index instead of iterating over it

Likely fix:

- detect the pattern:
  - integer constant index array on lhs
  - constant scalar/array RHS of matching length
- lower it to an explicit store loop:

```c
static const int idxs[] = {0,4,14,29};
static const double vals_r[] = {1,-5,10,-20};
static const double vals_i[] = {0,0,0,0};
for (int j = 0; j < 4; ++j) {
    int idx = idxs[j];
    cRe[idx] = vals_r[j];
    cIm[idx] = vals_i[j];
}
```

Priority:

- high

### `g44`

Source shape:

- same structural issue as `g43`
- constant integer index array on lhs
- constant coefficient array on rhs

Likely fix:

- same fix as `g43`

Priority:

- high

### `g63`

Source shape:

- loop computes a complex `numerator`
- loop computes a real `denominator`
- then does `cf[i] = numerator / denominator`
- later assignments read `.real`, `.imag`, `np.angle`, `np.abs`, etc.

Likely current failure mode:

- complex temporaries are being inferred as real scalars in some local-variable cases
- this is the smoking-gun pattern:
  - Python: `numerator = (t1 * (i+1) + t2**((i+1)/2))`
  - generated C was previously emitting something like `double numerator = _add9r;`
- that drops the imaginary part before the divide

Likely fix:

- local type inference must preserve complex locals
- if RHS is complex-valued, declare paired locals instead of a single `double`
- this same fix should help more than one function

Priority:

- high

### `g72`

Source shape:

- recurrence
- `v = sin(k * cf[k-1] + angle(t2**k)) + cos(k * abs(t1))`
- normalize via `v / (abs(v) + 1e-10)`

Likely current failure mode:

- same complex-local collapse as `g63`
- `v` is complex in Python
- if transpiled as real-only before normalization, the direction information is wrong

Likely fix:

- same complex local inference fix as `g63`

Priority:

- high

### `g76`

Source shape:

- recurrence
- `v = (t1+t2)**(k+1) + sin(k * cf[k-1]) + log(abs(k*t1)) - log(abs((k+1)*t2))`
- normalize via `v / abs(v)`

Likely current failure mode:

- same as `g72`
- `v` is complex, but if it gets collapsed to a real temp before normalization, the phase is destroyed

Likely fix:

- same complex local inference fix as `g63` / `g72`

Priority:

- high

### `g89`

Source shape:

- scalar-from-first-input function
- now correctly interpreted as:
  - `t1 = z[0].real`
  - `t2 = z[0].imag`
- key difficult term:
  - `t1**(51-1j)`

Why it still fails:

- after the scalar-from-first-input fix, this is no longer the old semantics bug
- the suspicious part is the complex exponent applied to a real base
- if the transpiler lowers this through a helper intended mainly for real exponents, it will drift or miscompute

Likely fix:

- inspect how complex exponentiation is lowered when:
  - base is real scalar
  - exponent is complex scalar
- if needed, route this through the full complex-power path instead of a real-exponent fast path

Fallback:

- if only this one remains after the other fixes, hand-write it

Priority:

- medium

## Recommended Order

1. Fix constant fancy-index assignment lowering.

Targets:

- `g43`
- `g44`

Expected gain:

- likely turns both into passes

2. Fix complex local-variable inference.

Targets:

- `g63`
- `g72`
- `g76`

Expected gain:

- likely turns most or all of this cluster into passes

3. Add elementwise lowering for array-expression assignment from `np.linspace` arrays.

Target:

- `g9`

4. Inspect complex-exponent lowering.

Target:

- `g89`

5. Re-evaluate `g10` last.

Reason:

- it is the least obviously structurally broken of the remaining set

## Suggested Regression Tests

Add targeted transpiler regression cases for:

- constant fancy indexed assignment:
  - `idx = np.array([0, 4, 14, 29]); cf[idx] = np.array([1, -5, 10, -20])`
- complex local temp:
  - `v = (t1+t2) + 1j * t1`
- normalization of complex local:
  - `cf[k] = v / np.abs(v)`
- whole-array expression assignment:
  - `arr1 = np.linspace(...); arr2 = np.linspace(...); cf = arr1**2 + 1j*arr2**3`
- complex exponent on real base:
  - `cf[0] = t1**(51-1j)` where `t1` is a real scalar extracted from `z[0].real`

## Expected Best-Case Outcome

If the fixes above land cleanly, the realistic next-pass target is:

- `g43`, `g44`, `g63`, `g72`, `g76` → likely pass
- `g9` → likely pass
- `g89` → maybe pass after math lowering fix
- `g10` → may still remain as a numeric-sensitivity case

That would take the batch from:

- `53 PASS / 8 FAIL / 38 STUB`

to something closer to:

- `59-60 PASS / 1-2 FAIL / 38 STUB`

without broadening the transpiler much beyond clearly useful patterns.
