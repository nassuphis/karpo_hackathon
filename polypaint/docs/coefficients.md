# Coefficient Computation Pipeline

The coeffgen pipeline computes complex polynomial coefficients across a 2D parameter grid. Each grid point (i1, i2) produces one polynomial whose roots are later solved and rendered as pixels.

The pipeline is composable and has three stages:

```
[parameter transforms] → coefficient function → [coefficient transforms]
```

## Execution Flow

### Frontend → Lambda → C Binary

1. **Frontend** (`index.html`, `runCalculate()`) builds job specs and fans out across Lambda:
   - Splits the grid into `nStripes` horizontal bands (rows)
   - Each stripe: `{i1_start, i1_end, n1, n2, function, param_transforms, coeff_transforms, times}`
   - Up to 50 concurrent Lambda invocations via `asyncPool`

2. **Lambda handler** (`handler_coeffgen.py`) receives the job, passes the spec as JSON on stdin to the `sweep` binary, uploads the output `.bin` to S3.

3. **C binary** (`sweep_cli.c`, mode=`coeffgen`) iterates the grid, applies transforms, calls the coefficient function, writes binary output.

### Grid Iteration

```c
for (int pass = 0; pass < times; pass++) {
    // RNG re-seeded per (pass, i1_start) for unique dither
    for (int i1 = i1_start; i1 < i1_end; i1++) {
        double x1 = (double)i1 / (double)n1;
        for (int j = 0; j < n2; j++) {
            int i2 = (i1 & 1) ? (n2 - 1 - j) : j;   // alternating scan direction
            double x2 = (double)i2 / (double)n2;

            double z1r = x1, z1i = 0.0, z2r = x2, z2i = 0.0;
            // apply param transforms...
            // call coefficient function...
            // apply coeff transforms...
            // write to binary file
        }
    }
}
```

- Initial z1 and z2 are **real** values in [0, 1) with imaginary parts = 0
- **Alternating scan**: odd rows iterate j in reverse order (space-filling pattern for better solver warm-start)
- **times** parameter: repeats the full grid with different RNG seeds per pass, producing `times × stripeRows × n2` coefficient sets total

### Binary Output Format

Each grid step writes one polynomial's worth of coefficients:

```
Per step: nCoeffsOut × 2 × sizeof(float32) bytes
Layout:   [Re(c0), Im(c0), Re(c1), Im(c1), ..., Re(c_{n-1}), Im(c_{n-1})]
```

Total file size: `stripeRows × n2 × times × nCoeffsOut × 2 × 4` bytes.

Coefficients are stored **leading-first** (index 0 = highest power, index n-1 = constant term).

### Metadata Output

The binary prints JSON metadata to stdout:

```json
{
  "mode": "coeffgen",
  "function": "giga_5",
  "n_coeffs": 26,
  "degree": 25,
  "n1": 1000, "n2": 1000,
  "i1_start": 0, "i1_end": 100,
  "n_t": 100000,
  "data_bytes": 20800000,
  "elapsed_us": 1234567
}
```

---

## Stage 1: Parameter Transforms

Parameter transforms modify (z1, z2) before they reach the coefficient function. They're specified as an array-of-arrays in the pipeline spec: `[["unit_circle"], ["sdith", "3"], ...]`.

Applied sequentially via `dispatchPt()`. Each transform receives pointers to `(z1r, z1i, z2r, z2i)` and modifies them in place.

### Basic Transforms

| Name | Effect |
|------|--------|
| `none` | No-op |
| `unit_circle` | z1 = exp(2πi·x1), z2 = exp(2πi·x2) — maps [0,1) reals onto the unit circle |
| `square` | z1 = z1², z2 = z2² |
| `cube` | z1 = z1³, z2 = z2³ |
| `reciprocal` | z1 = 1/z1, z2 = 1/z2 (guarded, returns 0 if |z| < 1e-30) |
| `conjugate` | z1 = conj(z1), z2 = conj(z2) — negates imaginary parts |
| `swap` | Swap z1 ↔ z2 |
| `add_sub` | z1' = z1 + z2, z2' = z1 - z2 |
| `mul_div` | z1' = z1 · z2, z2' = z1 / z2 |
| `moebius` | z1 = 1/(z1+2), z2 = 1/(z2+2) — Möbius transformation |
| `shift1` | z1r += 1, z2r += 1 |
| `scale10` | Multiply both by 10 |
| `negate` | Negate all components |
| `exp` | z = exp(z) for both — complex exponential |

### Parameterized Transforms

These accept numeric arguments from the pipeline spec:

| Name | Args | Effect |
|------|------|--------|
| `sdith` | d | Square dither: adds uniform noise ∈ [-w/2, w/2] to real parts, where w = 1/(d·gridN) |
| `radd` | v | Add v to real parts of z1 and z2 |
| `iadd` | v | Add v to imaginary parts of z1 and z2 |
| `add` | v | Add v to all four components |
| `cadd` | re, im | Add complex (re + im·i) to both z1 and z2 |
| `rscale` | v | Multiply real parts by v |
| `iscale` | v | Multiply imaginary parts by v |
| `scale` | v | Multiply all components by v |
| `rtheta` | gridN | Polar: z1 = z1r·exp(2πi·z2r), z2 = z2r·exp(2πi·z1r) |

### Algebraic "coeff" Transforms

These are parameter transforms that remap the input space in interesting ways:

| Name | Effect |
|------|--------|
| `coeff2` | z1' = z1+z2, z2' = z1·z2 |
| `coeff3` | z1 = 1/(z1+2), z2 = 1/(z2+2) — same as moebius |
| `coeff3a` | z1 = 1/(z1+1), z2 = 1/(z2+1) |
| `coeff4` | z1 = cos(z1), z2 = sin(z2) |
| `coeff5` | z1' = z1 + 1/z2, z2' = z2 + 1/z1 |
| `coeff5a` | z1' = z1 + 1/z1, z2' = z2 + 1/z2 |
| `coeff6` | z1 = (z1³+i)/(z1³-i), z2 = (z2³+i)/(z2³-i) |
| `coeff7` | z1 = (z1+sin(z1))/(z1+cos(z1)), z2 likewise |
| `coeff8` | z1 = (z1+sin(z2))/(z2+cos(z1)), z2 = (z2+sin(z1))/(z1+cos(z2)) |
| `coeff9` | z1 = (z1²+i·z2)/(z1²-i·z2), z2 = (z2²+i·z1)/(z2²-i·z1) |
| `coeff10` | z1 = (z1⁴-z2)/(z1⁴+z2), z2 = (z2⁴-z1)/(z2⁴+z1) |
| `coeff11` | z1 = log(z1⁴+2), z2 = log(z2⁴+2) |
| `coeff12` | z1 = 2z1⁴-3z2³+4z1²-5z2, z2 = 2z2⁴-3z1³+4z2²-5z1 |

### RNG for Dithering

The `sdith` transform uses xorshift64 PRNG:

```c
static uint64_t _rng_state = 0x123456789abcdef0ULL;
static inline uint64_t xorshift64(void) {
    uint64_t x = _rng_state;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    _rng_state = x;
    return x;
}
static inline double rng_uniform(void) {   // returns [0, 1)
    return (xorshift64() >> 11) * (1.0 / 9007199254740992.0);
}
```

Re-seeded at the start of each pass to ensure every (pass, stripe) combination produces unique dither:

```c
_rng_state = 0x123456789abcdef0ULL
            ^ ((uint64_t)pass * 2654435761ULL)
            ^ ((uint64_t)i1_start * 40503ULL);
```

---

## Stage 2: Coefficient Functions

Coefficient functions take the transformed (z1, z2) complex parameters and output a polynomial's coefficient vector.

### Function Signature

Two calling conventions exist:

```c
// Old-style: 2 real parameters (imaginary parts ignored)
typedef void (*CoeffFunc)(double x1, double x2,
                          double *cRe, double *cIm, int *nCoeffs);

// New-style: 2 complex parameters
typedef void (*CoeffFuncC)(double x1r, double x1i, double x2r, double x2i,
                           double *cRe, double *cIm, int *nCoeffs);
```

Old-style functions are wrapped via the `WRAP_OLD` macro to adapt them to the new signature. Lookup is via `lookupCoeffFuncC(name)` which does strcmp dispatch.

### Complex Arithmetic Helpers

All coefficient functions use these inline helpers (Cartesian form):

| Function | Signature | Operation |
|----------|-----------|-----------|
| `c_mul` | `(ar,ai, br,bi, &rr,&ri)` | Complex multiplication: (a·b) |
| `c_div` | `(ar,ai, br,bi, &rr,&ri)` | Complex division: a/b (returns 0 if \|b\|² < 1e-30) |
| `c_sin` | `(ar,ai, &rr,&ri)` | sin(a) = sin(ar)cosh(ai) + i·cos(ar)sinh(ai) |
| `c_cos` | `(ar,ai, &rr,&ri)` | cos(a) = cos(ar)cosh(ai) - i·sin(ar)sinh(ai) |
| `c_log` | `(ar,ai, &rr,&ri)` | log(a) = 0.5·log(\|a\|²) + i·atan2(ai,ar). Returns Re=-700 if \|a\|²=0 |
| `c_exp2` | `(r,i, &rr,&ri)` | exp(a) = exp(ar)·(cos(ai) + i·sin(ai)) |
| `c_abs` | `(r,i) → double` | \|a\| = sqrt(r²+i²) |
| `c_arg` | `(r,i) → double` | arg(a) = atan2(i, r) |
| `c_powr` | `(r,i, p, &rr,&ri)` | a^p for real exponent p: \|a\|^p · exp(i·p·arg(a)). Returns 0 if \|a\|² < 1e-60 |
| `c_powc` | `(ar,ai, br,bi, &rr,&ri)` | a^b for complex exponent: exp(b·log(a)) |

### Handwritten Coefficient Functions

Each function defines a polynomial family parameterized by (z1, z2). Coefficients are output in **leading-first** order (c[0] = highest power).

| Function | Degree | Coeffs | Input Transform | Description |
|----------|--------|--------|-----------------|-------------|
| `giga_1` | 24 | 25 | unit_circle | Sparse, reversed. t1=exp(2πi·x1), t2=exp(2πi·x2). Fixed terms at specific indices with t1²·t2 products. |
| `giga_5` | 25 | 26 | unit_circle | Sparse with complex trig terms. cf[6]=100i(t2³+t2²-t2-1), cf[8]=100i(t1³+t1²+t2-1). |
| `giga_19` | 89 | 90 | none (raw) | **Iterative**: cf[k]=i·sin(k·cf[k-1])/\|sin(...)\|. Uses raw x1,x2 (no unit circle). Ascending order, then reversed. |
| `giga_30` | 9 | 10 | unit_circle | Simple. cf[1]=150i·t2²+100·t1³, cf[5]=150\|t1+t2-2.5(1+i)\|. Reversed. |
| `giga_39` | 49 | 50 | unit_circle | Sparse fixed + trig/exponential: cf[25]=50(sin(t1)+i·cos(t2)). |
| `giga_40` | 29 | 30 | unit_circle | Sparse + trig. |
| `giga_42` | 49 | 50 | unit_circle | cf[11]=100i·exp(t1²+t2²), cf[44]=200sin(t1+t2)+i·cos(t1-t2). |
| `giga_43` | 39 | 40 | unit_circle | cf[19]=50(t1³-t2³), cf[34]=200·t1·t2·sin(t1+t2). |
| `giga_87` | 50 | 51 | unit_circle | Complex sparse with trig/exponential. |
| `giga_227` | 24 | 25 | unit_circle, coeff3 | **All-real** coefficients: power-based \|z1+z2\|^k, logarithmic, sqrt terms. |
| `giga_230` | 9 | 10 | unit_circle, coeff3 | Complex: sin³·cos², conditional branching on Im(z1). Safeguarded (NaN→0). |
| `giga_232` | 8 | 9 | unit_circle, coeff2 | Iterative per-coefficient: magnitude = \|z1\|^((j%3)+1) + \|z2\|^(8-j), angle from arg(z1/z2). Safeguarded. |
| `p7f` | 22 | 23 | none (raw) | Nested exponentials: tt1=exp(2πi·t1), ttt1=exp(2πi·tt1). Piecewise scale/freq based on t2. |
| `poly_110` | 70 | 71 | unit_circle, coeff5 | Prime-indexed: cf[k]=Re(z1)·prime[k%17]+Im(z2)·k². Symmetric fill from both ends. |

#### Reversed Functions

Functions prefixed with `rev_` (e.g. `rev_giga_1`) are wrappers that reverse the coefficient order after calling the base function. This converts between ascending order (constant first) and leading-first order (highest power first).

### Auto-Generated Functions (poly_1 through poly_100)

100 functions auto-transpiled from Python by `transpile_poly.py`. Included via:
```c
#include "poly_generated_funcs.h"   // function definitions
#include "poly_generated_lookups.h" // strcmp dispatch entries
```

**Typical characteristics:**
- Degree 20–70 (varies per function)
- Accept complex (z1, z2) inputs
- Use loop constructs over coefficient indices
- Complex arithmetic: c_mul, c_div, c_sin, c_cos, c_log, c_exp, c_powr, c_powc
- All outputs NaN-guarded (isfinite check replaces bad values with 0)
- Bounds-checked array writes: `if (_idx >= 0 && _idx < n_coeffs)`

**10 functions are stubbed** (too complex for auto-transpilation, return all zeros):
poly_21, poly_35, poly_37, poly_40, poly_46, poly_58, poly_72, poly_74, poly_94, poly_100.

---

## Stage 3: Coefficient Transforms

Applied after the coefficient function returns. Specified as an array of names in the pipeline: `["safe", "rev"]`.

| Name | Effect |
|------|--------|
| `none` | No-op |
| `rev` | Reverse coefficient order: c[k] ↔ c[n-1-k] |
| `conj` | Complex conjugate all coefficients: Im(c[k]) = -Im(c[k]) |
| `normalize` | Divide all coefficients by c[0] so that c[0] = 1 |
| `deriv` | Polynomial derivative (leading-first): c[k] *= (n-1-k), degree decreases by 1 |
| `scale100` | Multiply all coefficients by 100 |
| `safe` | Replace NaN/Inf with 0 |
| `negate_odd` | Negate odd-indexed coefficients: c[1], c[3], c[5], ... |

---

## The Transpiler: transpile_poly.py

Converts Python coefficient functions from `poly100.py` into C code.

### Invocation

```bash
python3 transpile_poly.py > poly_generated.c
```

Generates both `poly_generated.c` (implementations) and `poly_generated_funcs.h` (declarations).

### How It Works

1. Parses each `poly_N(t1, t2)` function from `poly100.py` using Python's `ast` module
2. Walks the AST via a `PolyTranspiler` visitor class
3. Emits C code with complex split representation (separate real/imaginary variables)

### Output Function Signature

```c
static void poly_N_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = N;
    for (int _i = 0; _i < N; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    // ... transpiled body ...
    // NaN guard:
    for (int _i = 0; _i < N; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}
```

### Supported Python Constructs

- **Expressions**: constants, variables, unary/binary ops (+, -, *, /, //, **, %), ternary conditionals
- **Assignments**: `cf[i] = expr`, `cf[a:b] = expr` (slice), `x = expr` (locals), augmented (`+=`, `-=`, `*=`)
- **Control flow**: `for i in range(...)`, `if/elif/else`
- **Numpy functions**: sin, cos, tan, exp, log, sqrt, abs, angle, real, imag, conj, sum, prod, sign, floor, ceil, clip, maximum, minimum, tanh, sinh, cosh, zeros, arange, array
- **Math functions**: sin, cos, tan, exp, log, sqrt, tanh, sinh, cosh, atan2, fabs, floor, ceil

### Key Design Decisions

- **CVar class**: Each complex number is a pair of C variable names (`_v3r`, `_v3i`), auto-named with incrementing counter
- **Bounds checking**: Every `cf[idx]` write is guarded: `{ int _idx = expr; if (_idx >= 0 && _idx < n_coeffs) { ... } }`
- **Transpile-time evaluation**: Constant arrays like `np.cumsum(np.arange(1, N))` are evaluated once at code generation, embedded as `static const double`
- **NaN guard**: Final isfinite sweep on all coefficients
- **Stubbing**: Functions too complex for auto-transpilation emit a zero-coefficient stub with `(void)` casts to suppress unused-parameter warnings

---

## Constants

```c
#define MAX_DEGREE   255     // Maximum polynomial degree
#define MAX_COEFFS   256     // Maximum coefficient count (degree + 1)
#define MAX_CHAIN    16      // Maximum transforms in either chain
#define MAX_PT_ARGS  4       // Maximum numeric arguments per param transform
#define MAX_ANIM     64
#define MAX_ITER     64      // Ehrlich-Aberth solver iterations
#define TOL2         1e-16   // Solver convergence tolerance (squared)
#define BUF_SIZE     (1024 * 256)   // 256 KB I/O buffer
```

---

## Pipeline Examples

### Example 1: giga_5 with unit_circle

```
PT chain:  [["unit_circle"]]
Function:  giga_5
CT chain:  []

Grid point (i1=250, i2=500), n=1000:
  x1 = 0.250, x2 = 0.500
  z1 = (0.25, 0.0), z2 = (0.50, 0.0)
    ↓ unit_circle
  z1 = exp(πi/2) = (0, 1) = i
  z2 = exp(πi) = (-1, 0) = -1
    ↓ giga_5(z1, z2)
  → 26 complex coefficients (leading-first)
    ↓ (no coeff transforms)
  Output: 26 × 2 × 4 = 208 bytes of float32
```

### Example 2: Dithered giga_232

```
PT chain:  [["unit_circle"], ["coeff2"], ["sdith", "3"]]
Function:  giga_232
CT chain:  ["safe"]

  z1 = x1, z2 = x2
    ↓ unit_circle: z1 = exp(2πi·x1), z2 = exp(2πi·x2)
    ↓ coeff2:      z1' = z1+z2, z2' = z1·z2
    ↓ sdith(3):    z1r += uniform[-w/2, w/2], z2r += uniform[-w/2, w/2]
                   where w = 1/(3·n1)
    ↓ giga_232(z1, z2) → 9 complex coefficients
    ↓ safe: replace NaN/Inf with 0
  Output: 9 × 2 × 4 = 72 bytes of float32
```

### Example 3: times=30 dithered run

With times=30 and sdith in the PT chain, the coeffgen produces 30× as many coefficient sets. The RNG is re-seeded each pass, so each pass explores a different random offset from each grid point. The solver reads all coefficient sets from the binary file (reading until EOF), solves each polynomial, and renders all roots — producing a denser image.

---

## Related Documentation

- [roots.md](roots.md) — how roots are computed from coefficients (Ehrlich-Aberth solver, binary format, async dispatch)
- [lambdas.md](lambdas.md) — all Lambda handlers including coeffgen and sweep
- [dynamodb.md](dynamodb.md) — DynamoDB status tracking (coeffgen, sweep, raster, finalize, encode)
- [testing.md](testing.md) — test suite and pre-deploy checklist

**Note:** After coeffgen completes, the solve phase (sweep Lambda) is dispatched asynchronously via the dispatch Lambda, not invoked synchronously. The frontend polls DynamoDB for sweep completion. See [roots.md](roots.md) for details.
