# Transpiler: transpile_poly.py

Converts Python polynomial coefficient functions from `poly100.py` into C code for the sweep binary.

## Invocation

```bash
cd polypaint/lambda
python3 transpile_poly.py > poly_generated.c
```

Generates:
- `poly_generated.c` — full function implementations
- `poly_generated_funcs.h` — function declarations (included by sweep_cli.c)
- `poly_generated_lookups.h` — strcmp dispatch entries for lookupCoeffFuncC

## How It Works

1. Parses each `poly_N(t1, t2)` function from `poly100.py` using Python's `ast` module
2. Walks the AST via the `PolyTranspiler` visitor class
3. Emits C code with split real/imaginary representation (separate variables for re/im)
4. Each complex number becomes a pair: `_vNr` (real) and `_vNi` (imaginary)

## Output Function Signature

```c
static void poly_N_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = N;
    for (int _i = 0; _i < N; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    // ... transpiled body ...
    // NaN guard
    for (int _i = 0; _i < N; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}
```

## Supported Python Constructs

### Expressions
- Constants: integers, floats, complex numbers
- Variables: `t1`, `t2` (inputs), `pi`, loop variables, locals
- Unary ops: negation
- Binary ops: `+`, `-`, `*`, `/`, `//`, `**`, `%`
- Function calls: numpy and math functions
- Subscripts: array indexing with bounds checking
- Ternary: `x if cond else y`

### Statements
- Single index assignment: `cf[i] = expr`
- Slice assignment: `cf[a:b] = expr` (with limitations, see below)
- Augmented assignment: `+=`, `-=`, `*=`
- Local variables: `x = expr`
- For loops: `for i in range(n)`, `for i in range(a, b)`, `for i in range(a, b, step)`
- If/elif/else

### Numpy Functions
- Trigonometric: `np.sin`, `np.cos`, `np.tan`
- Exponential/Log: `np.exp`, `np.log`, `np.sqrt`
- Magnitude/Phase: `np.abs`, `np.angle`
- Components: `np.real`, `np.imag`, `np.conj`
- Reduction: `np.sum(cf[a:b])`, `np.prod(cf[a:b])`
- Array creation: `np.zeros`, `np.arange`
- Misc: `np.sign`, `np.floor`, `np.ceil`, `np.clip`, `np.maximum`, `np.minimum`, `np.tanh`, `np.sinh`, `np.cosh`

### Math Functions
- `sin`, `cos`, `tan`, `exp`, `log`, `sqrt`, `tanh`, `sinh`, `cosh`, `atan2`, `fabs`, `floor`, `ceil`

## Known Limitations

### 1. Computed arrays in slice assignments (CRITICAL)

Two patterns silently emit **zeros** for all elements:

**Pattern A: `np.array([...])` with computed elements**
```python
cf[0:5] = np.array([1, t1, t1**2, t1**3, t1**4])
```

**Pattern B: List comprehensions**
```python
cf[0:35] = [np.real(f(t1, n)) - np.imag(f(t2, n)) for n in range(1, 36)]
```

The transpiler cannot evaluate these at transpile time because they contain expressions involving the function parameters (`t1`, `t2`). Both patterns produce all-zero coefficient slices, and the functions compile and run without error — the failure is completely silent.

**Affected functions (hand-written replacements exist in `poly_hand.h`):**
- poly_29 — `np.array([...])` slice assignments
- poly_33 — list comprehension slice assignments
- poly_55 — list comprehension slice assignments
- poly_58 — list comprehension slice assignments (also in stubbed list)

**Workaround:** Hand-write the function in C (see `poly_hand.h`) or rewrite the Python to use element-by-element assignment:
```python
# Instead of:
cf[0:5] = np.array([1, t1, t1**2, t1**3, t1**4])
# Use:
cf[0] = 1; cf[1] = t1; cf[2] = t1**2; cf[3] = t1**3; cf[4] = t1**4
```

### 2. Broken transpiled functions (visual pixel comparison results)

A batch visual test (50x50 grid, unit_circle → poly → rev → solve → render 500px) compared Python numpy reference images with C sweep images for all at-risk functions. The test renders roots as bilevel pixels and measures pixel set overlap. Functions with <60% overlap are confirmed broken.

**Confirmed broken (17 functions, need hand-writing):**

| Function | Py pixels | C pixels | Overlap | Failure mode |
|----------|-----------|----------|---------|-------------|
| poly_2 | 25344 | 23208 | 59.5% | Sequential cf[k-2] dependency in loop |
| poly_9 | 17732 | 10869 | 52.7% | Post-loop reads of loop-set values |
| poly_42 | 6242 | 6058 | 38.4% | arange loop variable replaced with 0 in slice assignment |
| poly_44 | 11149 | 1753 | 15.7% | Sequential dependency across overlapping loops |
| poly_50 | 7841 | 9867 | 21.3% | Loop dependency |
| poly_54 | 7149 | 1119 | 1.8% | Slice modification after loop |
| poly_61 | 3625 | 1752 | 0.8% | Nearly total mismatch |
| poly_62 | 0 | 2705 | 0% | Python produces 0 nonzero coefficients, C produces garbage |
| poly_65 | 18823 | 13348 | 57.6% | Sequential dependency |
| poly_70 | 2413 | 3332 | 20.7% | Loop dependency |
| poly_73 | 6779 | 2306 | 34.0% | Multiple overlapping loops |
| poly_78 | 20857 | 2537 | 9.3% | Complex power mishandling |
| poly_81 | 0 | 15068 | 0% | Python produces empty, C hallucinates |
| poly_82 | 2655 | 1100 | 27.6% | Sequential dependency |
| poly_95 | 9932 | 4657 | 24.9% | Loop dependency |
| poly_96 | 204 | 1 | 0.5% | Nearly empty C output |
| poly_97 | 8660 | 3047 | 35.1% | Loop dependency |

**Confirmed OK (24 functions, transpiler handles correctly despite complex patterns):**

poly_1 (100%), poly_8 (100%), poly_10 (97.7%), poly_23 (99.9%), poly_24 (70.9%), poly_41 (99.2%), poly_43 (88.0%), poly_47 (71.5%), poly_48 (64.8%), poly_49 (100%), poly_53 (100%), poly_60 (82.2%), poly_64 (63.6%), poly_71 (63.2%), poly_76 (98.6%), poly_77 (97.1%), poly_83 (98.2%), poly_89 (76.2%), poly_90 (88.1%), poly_91 (99.8%), poly_92 (69.8%), poly_93 (62.4%), poly_98 (98.8%), poly_99 (100%)

Functions with 60-80% overlap are correct but lossy due to float32 coefficient storage at high degree (70). The root positions shift with small coefficient perturbations, but the overall structure matches.

**Key failure modes identified:**

1. **arange variable zeroed** (poly_42): `cf[0:35] = |t1| * sin(arange(1,36) * angle(t1))` — the transpiler replaces the arange loop variable with 0, producing `sin(0) = 0` for all elements. The beautiful accident was preserved as `poly_42_serendipity`.

2. **Overlapping loop modification** (poly_44, poly_45, poly_54): A second loop modifies array elements that depend on values set by the first loop. The transpiler loses the sequential ordering.

3. **Post-loop slice reads** (poly_9, poly_73): Lines after a loop read `cf[k]` values that were set by the loop. The transpiler may mis-order these.

4. **Computed reverse indexing** (poly_45): `cf[len(cf) - i]` uses a computed index the transpiler can't resolve correctly.

5. **Phantom coefficients** (poly_62, poly_81): The transpiler produces nonzero output where Python produces all zeros (exception-caught functions returning zeros).

### 3. Formerly stubbed functions (all now hand-written)

These 9 functions were originally too complex for auto-transpilation. All now have hand-written C implementations in `poly_hand.h`:

poly_21, poly_35, poly_37, poly_40, poly_46, poly_72, poly_74, poly_94, poly_100

### 4. Slice assignments with complex RHS patterns

Only simple patterns are supported for `cf[a:b] = expr`:
- `cf[a:b] = scalar * np.arange(start, end)` — works
- `cf[a:b] = expr1 * expr2` where one is arange — works
- `cf[a:b] = np.array([...])` with computed elements — **does not work** (emits zeros)
- `cf[a:b] = [expr for x in range(...)]` — **does not work** (emits zeros)

### 5. Real-only accuracy tests miss complex input bugs

The `test_poly_accuracy.py` tests use real-only inputs (`x1, x2` with no imaginary part). Functions that only fail with complex inputs (like poly_29 with `unit_circle`) pass the accuracy test despite being broken. The accuracy test should be extended to use complex inputs via `unit_circle` transform.

### 6. Silent failure mode

When the transpiler encounters an unsupported construct, it typically emits `0` rather than raising an error. This makes broken transpilations hard to detect — the function compiles and runs but produces wrong coefficients. The only way to catch this is coefficient comparison tests with complex inputs.

### 7. No complex literal arrays

`np.array([1+2j, 3+4j])` is not handled. Complex literals in array constructors are not evaluated.

### 8. Transpile-time evaluation limitations

Constant arrays like `np.cumsum(np.arange(1, N))` are evaluated at code generation time and embedded as `static const double`. But this only works for expressions that can be fully evaluated without the function parameters. Any expression involving `t1` or `t2` in an array constructor context fails silently.

## Identifying Broken Functions

**Single-point coefficient comparison is unreliable.** Float32 storage causes large relative errors at individual points even when the function is visually correct. poly_1 showed 197% relative error at one test point but produces pixel-identical images.

**Visual pixel comparison is the reliable test.** Render a 50x50 (or 100x100) grid through the full pipeline (unit_circle → poly → rev → solve → plot) in both Python and C, then compare lit pixel sets:

- **>60% overlap** → transpiler is correct (differences are float32 precision loss)
- **<60% overlap** → transpiler is broken, needs hand-writing
- **0% overlap with one side empty** → phantom coefficients or total failure

The batch visual test script is inline in the conversation history. To test a single function:

1. Create `test_poly_N.py` with the Python function, run it to get `/tmp/polyN_roots.png`
2. Run C sweep: `coeffgen → solve → render` to get `/tmp/polyN_sweep.png`
3. Compare pixel sets: `open /tmp/polyN_roots.png /tmp/polyN_sweep.png`

## Hand-Written Overrides

Hand-written C implementations replace broken transpiled versions. Files:

| File | Functions | Source |
|------|-----------|--------|
| `poly_hand.h` | poly_2, 9, 21, 29, 33, 35, 37, 40, 42, 42_serendipity, 44, 45, 46, 50, 54, 55, 58, 61, 62, 65, 70, 72, 73, 74, 78, 81, 82, 94, 95, 96, 97, 100 | poly100.py |
| `poly_hand_batch2.h` | poly_134, 135, 142, 146, 150 | poly200.py |
| `poly_hand_batch3.h` | poly_152, 153, 157, 164, 167 | poly200.py |
| `poly_hand_batch4.h` | poly_171, 179, 180, 184, 187 | poly200.py |
| `poly_hand_batch5.h` | poly_188, 189, 190, 191, 192 | poly200.py |
| `poly_hand_batch6.h` | poly_193, 194, 195, 196, 197 | poly200.py |
| `poly_hand_batch7.h` | poly_198, 199, 123, 161, 103 | poly200.py |
| `poly_hand_300a.h` | poly_201–220 | poly300.py |
| `poly_hand_300b.h` | poly_221–240 | poly300.py |
| `poly_hand_300c.h` | poly_241–260 | poly300.py |
| `poly_hand_300d.h` | poly_261–280 | poly300.py |
| `poly_hand_300e.h` | poly_281–300 | poly300.py |

Add lookup overrides in `lookupCoeffFuncC()` before the `#include "poly_generated_lookups.h"` line. Hand-written lookups take priority over transpiled ones.

## Multi-file Transpilation

The transpiler supports multiple source files via `transpile_file()`:

```python
from transpile_poly import transpile_file
c_code, header, lookups = transpile_file('poly200.py', stub_funcs={'poly_103'}, skip_funcs={'poly_110'}, label='poly200.py')
```

- `stub_funcs`: emit zero-output stubs for functions too complex to transpile
- `skip_funcs`: completely omit (e.g., poly_110 already defined in poly100)
- Output files: `poly_generated_200.c`, `poly_generated_200_funcs.h`, `poly_generated_200_lookups.h`

## Coverage Summary (poly_1–500)

| Source | Total | Transpiled OK | Hand-written | Stubbed | Broken |
|--------|-------|---------------|--------------|---------|--------|
| poly100.py | 100 | ~68 | 32 | 0 | ~10 |
| poly200.py | 100 | ~40 | 30 | 2 | ~30 |
| poly300.py | 100 | ~23 (arange lowering) | 100 | 0 | ~12 |
| poly400.py | 100 | ~47 | 0 | 8 | ~45 |
| poly500.py | 100 | ~59 | 0 | 7 | ~34 |

**poly200.py**: Transpiler handles most loop-based functions. ~30 fail due to float32 dynamic range (>7 orders of magnitude in coefficients), not transpiler bugs.

**poly300.py**: Entirely vectorized `np.arange` style. Arange loop lowering (implemented) handles poly_201-223. The degree-34 functions (224-300) use `np.linspace`, `np.prod`, conditional branches — all hand-written.

**poly400.py/poly500.py**: Loop-based style transpiles well. ~50-60% pass rate. Failures from: `np.linspace` (can't evaluate at transpile time), `np.prod([t1,t2])` (list args), `max()`/`min()` builtins, conditional variable scoping, `np.arctan2`.

## Transpiler Improvement Opportunities — Prioritized

### Priority 1: `np.linspace` support (HIGHEST IMPACT)

**122 warnings** across poly400+poly500 from `could not evaluate` — almost all are `np.linspace(np.real(t1), np.real(t2), n)`. These create local arrays `rec` and `imc` used throughout the function.

```python
rec = np.linspace(np.real(t1), np.real(t2), n)  # n=35
imc = np.linspace(np.imag(t1), np.imag(t2), n)
```

**Proposed fix:** Detect this pattern and emit inline computation:
```c
double rec[35], imc[35];
for (int _li = 0; _li < 35; _li++) {
    rec[_li] = x1r + (x2r - x1r) * _li / 34.0;
    imc[_li] = x1i + (x2i - x1i) * _li / 34.0;
}
```

This alone would fix ~60% of poly400/poly500 failures since most broken functions use `rec[j-1]` and `imc[j-1]` in their coefficient loops.

### Priority 2: `np.prod` and `np.sum` with list arguments

```python
np.prod(np.array([np.real(t1), np.imag(t2)]))  # = real(t1) * imag(t2)
np.sum(np.array([np.abs(t1), np.abs(t2)]))     # = abs(t1) + abs(t2)
```

These are small constant-length lists of expressions. Emit unrolled multiplication/addition:
```c
double _prod = x1r * x2i;  // np.prod([real(t1), imag(t2)])
double _sum = c_abs(x1r,x1i) + c_abs(x2r,x2i);  // np.sum([abs(t1), abs(t2)])
```

### Priority 3: Conditional variable scoping (COMPILE ERRORS)

15 functions across poly400+poly500 fail to compile because variables first assigned inside `if/elif` branches aren't declared at function scope:

```python
if j % 3 == 0:
    mag = expr1
elif j % 3 == 1:
    mag = expr2
else:
    mag = expr3
cf[j-1] = mag * np.exp(1j * angle)  # ERROR: 'mag' undeclared
```

**Fix:** When processing an `if/elif/else` chain, scan all branches for first assignments to new variables. Pre-declare them before the `if`:
```c
double mag = 0;  // pre-declare
if (j % 3 == 0) { mag = ...; }
else if (j % 3 == 1) { mag = ...; }
else { mag = ...; }
```

### Priority 4: Python builtins `max()`, `min()`, `complex()`

```python
max(np.abs(t1), np.abs(t2))  → fmax(c_abs(x1r,x1i), c_abs(x2r,x2i))
min(j, 6)                     → fmin((double)j, 6.0)
complex(real=a, imag=b)        → (a, b)  as CVar
```

Simple 1:1 mappings. Currently emit warnings.

### Priority 5: `np.arctan2`

```python
np.arctan2(np.imag(t1), np.real(t1))  → atan2(x1i, x1r)
```

Already have `math.atan2` support — just need to add `np.arctan2` as an alias.

### Priority 6: Dynamic slice bounds

```python
cf[:k] = ...      # Slice(upper=Name('k'))
np.sum(cf[:j])    # sum of first j elements
```

Currently emits a warning. Could emit a bounded loop:
```c
for (int _si = 0; _si < k && _si < nCoeffs; _si++) { ... }
```

### Implemented (from poly300 session)

### 9. Vectorized arange expressions — IMPLEMENTED

Arange loop lowering now works for `j = np.arange(N); cf = f(j)`. Verified identical output to hand-written code for poly_201, 202, 210, 220. Handles poly_201-223 automatically.

### 10. Whole-array operations

Still unimplemented. Patterns like `cf *= np.exp(1j * np.angle(cf))` and `mod_cf = (71 - np.arange(1, 72)) * np.abs(cf)` need post-processing loop detection.

### 11. Float32 dynamic range guard

Still unimplemented. Optional normalization pass to keep coefficient magnitudes within float32 range. Would help ~30 poly200 functions and ~10 poly300 functions.

## Estimated Impact of Improvements

If all Priority 1-5 fixes were implemented:
- **poly400**: ~47 → ~75 pass (linspace + prod + scoping + builtins)
- **poly500**: ~59 → ~80 pass (same)
- **Future files**: Expect similar loop-based style, ~75-80% auto-transpile rate

Priority 1 (linspace) alone would recover ~30 functions across poly400+poly500.
