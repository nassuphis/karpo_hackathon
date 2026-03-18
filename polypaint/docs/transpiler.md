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

### 2. Sequential coefficient dependencies in loops (SUBTLE)

When a loop body reads `cf[i-2]` to compute `cf[i-1]`, the transpiler *usually* handles this correctly — it generates C code that reads and writes in the same order as Python. **However**, the transpiler can produce wrong results when:

**Pattern A: Multiple loops modifying overlapping ranges**
```python
# Loop 1 sets cf[0:70]
for k in range(1, 72):
    cf[k-1] = some_expr(k, t1, t2)
# Loop 2 overwrites parts of cf using values set by loop 1
for i in range(2, 35):
    cf[i-1] = cf[i-2] * factor + constant       # forward dependency
    cf[len(cf) - i] = -cf[len(cf) - i + 1] * ...  # reverse dependency
```

The transpiler may mis-handle computed reverse indices like `cf[len(cf) - i]` or fail to preserve the sequential evaluation order when the same array is both read and written across loop iterations.

**Pattern B: Post-loop modifications reading loop-set values**
```python
for k in range(1, 72):
    cf[k-1] = expr(k, t1, t2)
cf[1] += np.sum(cf[0:2])      # reads cf[0], cf[1] set by loop
cf[4] += np.prod(cf[0:5])     # reads cf[0:5] set by loop
cf[12] = 3 * (t1**2 - t2**2)  # overwrites loop value
cf[13] = cf[12] + ...         # reads just-overwritten cf[12]
```

The transpiler may evaluate these in a different order than Python, especially when a line reads a value that was just overwritten by the preceding line.

**Confirmed broken:** poly_45 (3% pixel overlap with Python reference — completely wrong image).

**At-risk functions** (transpiled, contain sequential cf dependencies — may or may not be correct, need visual comparison testing):

poly_1, poly_2, poly_8, poly_9, poly_10, poly_23, poly_24, poly_41, poly_42, poly_43, poly_44, poly_47, poly_48, poly_49, poly_50, poly_53, poly_54, poly_60, poly_61, poly_62, poly_64, poly_65, poly_70, poly_71, poly_73, poly_76, poly_77, poly_78, poly_81, poly_82, poly_83, poly_89, poly_90, poly_91, poly_92, poly_93, poly_95, poly_96, poly_97, poly_98, poly_99

Not all of these are broken — many sequential patterns transpile correctly. The only way to confirm is to run a visual comparison test (Python vs C) for each function.

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

To check if a transpiled function is correct:

1. Pick a test point with complex inputs (e.g., `t1 = exp(πi/4)`, `t2 = exp(πi/3)`)
2. Run the Python function to get reference coefficients
3. Run the C function via sweep_test coeffgen to get C coefficients
4. Compare: if nonzero count differs significantly, the transpilation is broken

The `test_poly_accuracy.py` framework can be extended for this — just use `unit_circle` as a param transform instead of raw real inputs.

## Hand-Written Overrides

Hand-written C implementations live in `poly_hand.h` (included by sweep_cli.c). Each uses the same signature as transpiled functions. Add a lookup override before the `#include "poly_generated_lookups.h"` line in `lookupCoeffFuncC()`:

```c
if (strcmp(name, "poly_21") == 0)  return poly_21_hand;
if (strcmp(name, "poly_29") == 0)  return poly_29_hand;
if (strcmp(name, "poly_33") == 0)  return poly_33_hand;
if (strcmp(name, "poly_35") == 0)  return poly_35_hand;
if (strcmp(name, "poly_37") == 0)  return poly_37_hand;
if (strcmp(name, "poly_40") == 0)  return poly_40_hand;
if (strcmp(name, "poly_45") == 0)  return poly_45_hand;
if (strcmp(name, "poly_46") == 0)  return poly_46_hand;
if (strcmp(name, "poly_55") == 0)  return poly_55_hand;
if (strcmp(name, "poly_58") == 0)  return poly_58_hand;
if (strcmp(name, "poly_72") == 0)  return poly_72_hand;
if (strcmp(name, "poly_74") == 0)  return poly_74_hand;
if (strcmp(name, "poly_94") == 0)  return poly_94_hand;
if (strcmp(name, "poly_100") == 0) return poly_100_hand;
```

These take precedence over the generated lookup. Current hand-written overrides (14 functions):

| Function | Reason | Verification |
|----------|--------|-------------|
| poly_21 | Lagrange basis construction (complex indexing) | Visual check |
| poly_29 | np.array slice assignments | 99.97% pixel match N=500 |
| poly_33 | List comprehension slice assignments | 77.7% match N=100 (float32 ill-conditioning) |
| poly_35 | Originally stubbed (cos/sin powers) | Visual check |
| poly_37 | Post-loop modifications + fancy indexing | Visual check |
| poly_40 | Originally stubbed (angle/power) | Visual check |
| poly_45 | Sequential dependency in overlapping loops | 100% pixel match N=100 |
| poly_46 | Prime array + complex power construction | Visual check |
| poly_55 | List comprehension slice assignments | Visual check |
| poly_58 | List comprehension + 70! overflow | Visual check |
| poly_72 | Sort-based rewrite (runtime-dependent) | Visual check |
| poly_74 | Fractional complex powers | Visual check |
| poly_94 | Sparse power structure | Visual check |
| poly_100 | Iterative product with conj(iter) | Visual check |
