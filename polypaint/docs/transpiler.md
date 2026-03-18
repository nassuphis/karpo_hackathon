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

### 1. np.array([...]) in slice assignments (CRITICAL)

**Does not work:**
```python
cf[0:5] = np.array([1, t1, t1**2, t1**3, t1**4])
```

The transpiler emits **zeros** for all elements. It cannot evaluate `np.array([...])` with mixed constants and complex expressions at transpile time. This affects any function that uses `np.array([...])` to populate coefficient slices.

**Affected functions:** poly_29 (hand-written replacement exists), and likely others among the 100.

**Workaround:** Hand-write the function in C (see `poly_29_hand` in sweep_cli.c) or rewrite the Python to use a loop:
```python
# Instead of:
cf[0:5] = np.array([1, t1, t1**2, t1**3, t1**4])
# Use:
cf[0] = 1; cf[1] = t1; cf[2] = t1**2; cf[3] = t1**3; cf[4] = t1**4
```

### 2. Stubbed functions (10 functions)

These were too complex for auto-transpilation and produce all-zero coefficients:

poly_21, poly_35, poly_37, poly_40, poly_46, poly_58, poly_72, poly_74, poly_94, poly_100

They may use unsupported numpy functions (spherical harmonics, special functions), complex control flow, or runtime-dependent operations.

### 3. Slice assignments with complex RHS patterns

Only simple patterns are supported for `cf[a:b] = expr`:
- `cf[a:b] = scalar * np.arange(start, end)` — works
- `cf[a:b] = expr1 * expr2` where one is arange — works
- `cf[a:b] = np.array([...])` with computed elements — **does not work** (emits zeros)

### 4. Real-only accuracy tests miss complex input bugs

The `test_poly_accuracy.py` tests use real-only inputs (`x1, x2` with no imaginary part). Functions that only fail with complex inputs (like poly_29 with `unit_circle`) pass the accuracy test despite being broken. The accuracy test should be extended to use complex inputs via `unit_circle` transform.

### 5. Silent failure mode

When the transpiler encounters an unsupported construct, it typically emits `0` rather than raising an error. This makes broken transpilations hard to detect — the function compiles and runs but produces wrong coefficients. The only way to catch this is coefficient comparison tests with complex inputs.

### 6. No complex literal arrays

`np.array([1+2j, 3+4j])` is not handled. Complex literals in array constructors are not evaluated.

### 7. Transpile-time evaluation limitations

Constant arrays like `np.cumsum(np.arange(1, N))` are evaluated at code generation time and embedded as `static const double`. But this only works for expressions that can be fully evaluated without the function parameters. Any expression involving `t1` or `t2` in an array constructor context fails silently.

## Identifying Broken Functions

To check if a transpiled function is correct:

1. Pick a test point with complex inputs (e.g., `t1 = exp(πi/4)`, `t2 = exp(πi/3)`)
2. Run the Python function to get reference coefficients
3. Run the C function via sweep_test coeffgen to get C coefficients
4. Compare: if nonzero count differs significantly, the transpilation is broken

The `test_poly_accuracy.py` framework can be extended for this — just use `unit_circle` as a param transform instead of raw real inputs.

## Hand-Written Overrides

When a transpiled function is broken, write it by hand in sweep_cli.c and add a lookup override before the `#include "poly_generated_lookups.h"` line:

```c
if (strcmp(name, "poly_29") == 0)  return poly_29_hand;
```

This takes precedence over the generated lookup. Current hand-written overrides:
- `poly_29_hand` — replaces broken transpiled poly_29
