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
- Reduction: `np.sum(cf[a:b])`, `np.prod(cf[a:b])`, `np.sum([a, b])`, `np.prod([a, b])`
- Array creation: `np.zeros`, `np.arange`, `np.linspace`
- Misc: `np.sign`, `np.floor`, `np.ceil`, `np.clip`, `np.maximum`, `np.minimum`, `np.tanh`, `np.sinh`, `np.cosh`, `np.arctan2`, `np.conjugate`

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

Supported patterns for `cf[a:b] = expr`:
- `cf[a:b] = scalar * np.arange(start, end)` — works (element-wise loop)
- `cf[a:b] = expr1 * expr2` where one is arange — works
- `cf[a:b] = np.array([expr1, expr2, ...])` — works (unrolled element-by-element)
- `cf[a:b] = [expr for x in range(...)]` — works (transpiled list comprehension)
- `cf[a:b] = np.array([...]) * expr` — works (unrolled with binop per element)

Not supported:
- Dynamic slice bounds: `cf[:k] = expr` where `k` is a variable (emits warning)

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

**Batch test script:** `test_transpiler_all.py` in `polypaint/lambda/` tests all functions in a source file range. For each function:
1. **C pipeline:** `sweep_test` binary runs coeffgen → solve → reads binary float32 root positions → rasterizes to 1000×1000 boolean grid
2. **Python pipeline:** loads function via `ast.parse`+`exec` → evaluates on 100×100 parameter grid with `unit_circle` transform → `np.roots()` → rasterizes to same grid
3. **Compare:** computes `intersection / union` of the two boolean pixel sets

No PNG files are generated or compared — it's an in-memory pixel set overlap test. Run: `cd polypaint/lambda && uv run python test_transpiler_all.py`

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

## Updating the UI Dropdown

After transpiling a new batch and running visual tests, the coefficient function selector dropdown in `index.html` must be updated manually. The dropdown is a `<select id="render-function">` element (around line 195–811) containing one `<option>` per function.

### Current format

```html
<option value="poly_501">· poly_501 [100%] (degree 34)</option>   <!-- transpiled, passing -->
<option value="poly_535">· poly_535 [0%] (degree 34)</option>      <!-- transpiled, failing -->
<option value="poly_407">❌ poly_407 (degree 34)</option>           <!-- stubbed -->
```

- `·` prefix = transpiled C function exists in sweep binary
- `❌` prefix = stubbed (zero-output placeholder)
- `[N%]` = pixel overlap from visual comparison test
- `(degree N)` = polynomial degree (34 for poly_301+, 70 for poly_1–223)

### What to update after adding a new batch

1. Add `<option>` entries for the new functions before the `</select>` tag
2. Update overlap percentages for any re-transpiled functions whose results changed
3. Mark stubbed functions with `❌` and no overlap percentage

### Future improvement

This is currently a manual copy-paste process which is error-prone and tedious. A better approach would be:

1. Save test results to a JSON file (e.g. `polypaint/lambda/test_results.json`) keyed by function name, with overlap %, stub status, and degree
2. Write a script that reads the JSON and regenerates the `<option>` block
3. Optionally run the script as part of the transpile → compile → test → update pipeline

This would make the dropdown a derived artifact rather than hand-maintained state.

## Coverage Summary (poly_1–800)

| Source | Total | Transpiled OK | Hand-written | Stubbed | Broken | Pass Rate |
|--------|-------|---------------|--------------|---------|--------|-----------|
| poly100.py | 100 | ~68 | 32 | 0 | ~10 | ~90% (with hand) |
| poly200.py | 100 | ~40 | 30 | 2 | ~30 | ~70% (with hand) |
| poly300.py | 100 | ~23 (arange lowering) | 100 | 0 | ~12 | ~100% (all hand) |
| poly400.py | 100 | 73 | 0 | 10 | 17 | 73% |
| poly500.py | 100 | 79 | 0 | 7 | 14 | 79% |
| poly600.py | 100 | 84 | 0 | 0 | 16 | 84% |
| poly700.py | 100 | 89 | 0 | 0 | 11 | 89% |
| poly800.py | 100 | 61 | 0 | 2 | 37 | 61% |

**poly200.py**: Transpiler handles most loop-based functions. ~30 fail due to float32 dynamic range (>7 orders of magnitude in coefficients), not transpiler bugs.

**poly300.py**: Entirely vectorized `np.arange` style. Arange loop lowering (implemented) handles poly_201-223. The degree-34 functions (224-300) use `np.linspace`, `np.prod`, conditional branches — all hand-written.

**poly400.py/poly500.py**: Loop-based style. After Priority 1-6 improvements: 73-79% pass rate, 0 transpiler warnings. Remaining failures are float32 dynamic range, phantom coefficients, and sequential loop dependencies.

**poly600.py**: Same loop-based style. 84% pass rate. `ps.poly.get("n") or 35` pattern now handled (extracts default value). 7 warnings from dynamic slice bounds.

**poly700.py**: 89% pass rate — highest auto-transpile success. Mostly degree-8/9 functions. 1 warning (dynamic slice bound). Self-referencing variable fix (`angle_part = f(angle_part)`) discovered and implemented here.

**poly800.py**: 61% pass rate — lowest of the auto-transpiled batches. Contains exotic patterns the transpiler doesn't handle: `np.where`, `np.isnan`/`np.isinf`/`np.isfinite`, `np.fft.fft`, `np.linalg.det`/`inv`, `np.log10`, `np.median`, `np.arctan`, `np.log1p`, `math.prod`, `np.math.factorial`. 2 stubs (poly_755, poly_772) for vectorized linspace array operations (`csi ** 2` where `csi` is a linspace array). 51 warnings total. Mixed degrees (8-24).

## Implemented Transpiler Improvements

### Priority 1–6: ALL IMPLEMENTED

All six prioritized improvements have been implemented and tested:

| # | Feature | Implementation |
|---|---------|---------------|
| 1 | `np.linspace` | `_is_np_linspace()`, `_extract_linspace_args()` — emits `double name[n]; for` loop. Handles positional and `num=` keyword arg. Resolves constant expressions like `degree + 1` via `_eval_const_expr()`. |
| 2 | `np.prod`/`np.sum` with list args | `_extract_list_or_array_elts()` — unrolls `np.prod([a, b])` to `c_mul(a, b)`, `np.sum([a, b])` to `a + b`. |
| 3 | Conditional variable scoping | `_predeclare_if_vars()` — scans if/elif/else branches, pre-declares variables as `double name = 0;` before the if. |
| 4 | `max()`/`min()`/`complex()` builtins | `max(a,b)` → `fmax()`, `min(a,b)` → `fmin()`, `complex(real=a, imag=b)` → CVar. |
| 5 | `np.arctan2` | Aliased to `atan2()`. Also added `np.conjugate` support. |
| 6 | Dynamic slice bounds | `np.sum(cf[:j])` emits bounded loop with `(int)` cast and `< nCoeffs` guard. |

**Actual results vs estimates:**
- poly400: estimated ~75 → actual **73** (close)
- poly500: estimated ~80 → actual **79** (close)
- poly600 (new): **84** (exceeded the ~75-80% prediction)

### Additional fixes discovered during poly600–800 transpilation

**Index cast bug (compile error, poly600):** Local variables like `n`, `idx`, `index` are declared as `double` but used as array subscripts in `cf[n - k]` patterns. The transpiler now tracks `_loop_vars` (declared as `int` in for-loops) separately from other locals, and casts non-loop variables to `(int)` when used as array indices.

**Linspace `num=` keyword (poly600):** `np.linspace(t1.real, t2.real, num=n)` with keyword argument. `_extract_linspace_args()` now checks `node.keywords` for `num=`.

**Constant expression evaluation (poly600):** `np.linspace(..., num=degree + 1)` requires evaluating `degree + 1` at transpile time. Added `_eval_const_expr()` which recursively evaluates `+`, `-`, `*`, `//` on constants and resolved locals.

**General `_resolve_const_name` (poly600):** Now resolves any variable from `_const_locals`, not just `"n"`.

**Runtime config defaults (poly600):** `n = ps.poly.get("n") or 35` — the const pre-scan now extracts the default from `X or <constant>` BoolOp patterns. Will help ~20 functions in poly900.

**Self-referencing variable pre-declaration (poly700):** `angle_part = sin(x) + cos(angle_part)` — the RHS references the variable before its declaration. Added `_name_in_expr()` to detect this and pre-declare as `double name = 0;`.

### Still unimplemented

**Vectorized linspace array operations:** `csi = np.linspace(...); cf[1:10] = t1 * csi ** 2` — the transpiler creates `csi` as a `double[]` array but then tries to use it as a scalar in expressions. Requires element-wise loop lowering for array variables in expressions. Causes compile errors (poly_755, poly_772 stubbed).

**Exotic numpy functions (poly800+):** `np.where`, `np.isnan`, `np.isinf`, `np.isfinite`, `np.fft.fft`, `np.linalg.det`/`inv`, `np.log10`, `np.median`, `np.arctan`, `np.log1p`, `math.prod`, `np.math.factorial`. These emit warnings and produce zeros. 51 warnings in poly800.

**Whole-array operations:** Patterns like `cf *= np.exp(1j * np.angle(cf))` need post-processing loop detection.

**Float32 dynamic range guard:** Optional normalization pass to keep coefficient magnitudes within float32 range. Would help ~30 poly200 functions and ~10 poly300 functions.

**Dynamic slice assignment:** `cf[:k] = expr` where `k` is a variable (not `np.sum` context) still emits a warning.

## Remaining Failure Modes (poly400–800)

Analysis of 94 failed functions across poly400-800 shows these categories:

### 1. Float32 dynamic range / precision loss (~20 functions)
Functions produce structurally similar but shifted root positions. Overlap 30-55%. The transpiled C is correct but float32 coefficient storage amplifies small differences at high degree. Example: poly_301 (48%), poly_408 (45%), poly_749 (52%).

### 2. Phantom coefficients (~15 functions)
C produces nonzero output where Python produces all zeros (exception-caught functions returning zeros), or vice versa. Overlap 0%. Example: poly_307, poly_504, poly_742, poly_782, poly_789, poly_793.

### 3. Unsupported numpy functions (~15 functions, mostly poly800)
Functions using `np.where`, `np.isnan`, `np.fft.fft`, `np.linalg`, etc. produce zeros for the unsupported calls, leading to structural mismatch. Example: poly_745 (5%), poly_762 (2%), poly_774 (6%).

### 4. Sequential loop dependencies (~10 functions)
A second loop reads `cf[k]` values set by the first loop. The transpiler emits correct individual operations but loses ordering dependencies. Example: poly_341 (33%), poly_525 (32%).

### 5. Structural mismatch (~15 functions)
Complete or near-complete mismatch from fundamental transpilation errors. More common in poly800 due to complex patterns. Example: poly_317 (0%), poly_766 (18%), poly_776 (10%), poly_800 (3%).

### 6. Near-miss (50-60% overlap, ~10 functions)
Visually similar but below the 60% threshold. Example: poly_489 (60%), poly_573 (52%), poly_637 (57%), poly_749 (52%).
