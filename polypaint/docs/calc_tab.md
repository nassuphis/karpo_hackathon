# COMPUTE Tab

The COMPUTE tab generates polynomial coefficients and solves for roots across a 2D parameter grid.

## UI Controls

### Param Transforms
Dropdown `#pt-add` with 35+ options. Selected transforms appear as chips in `#pt-chips`. Applied left-to-right before the coefficient function. Some accept parameters (radd, iadd, scale, sdith, etc.).

Common transforms: `unit_circle`, `rtheta`, `square`, `reciprocal`, `sdith` (stochastic dither), `coeff2`-`coeff12`.

### Function
Dropdown `#render-function` with ~86 polynomial functions: `giga_*` (degree 9-25), `poly_1`-`poly_86` (mostly degree 70), `p7f` (degree 22), `poly_110`.

### Coeff Transforms
Dropdown `#ct-add` with 8 options: `rev`, `conj`, `normalize`, `deriv`, `scale100`, `safe`, `negate_odd`. Applied left-to-right after coefficient generation.

### Grid Parameters
| Control | Range | Default | Description |
|---------|-------|---------|-------------|
| N | 10-5000 | 500 | Grid dimension (N x N parameter points) |
| Stripes | 10-500 | 10 | Parallel subdivisions of the grid |
| Times | 1-100 | 1 | Coefficient repetitions (for dithered passes) |

### Quick Presets
- **Fast N=100** — quick test
- **Medium N=500** — default quality
- **Hi-res N=1K** — high density

### Calculate Button
Green button, calls `runCalculate()`. Disabled during computation.

## Computation Pipeline

### Phase 1: Hires Coeffgen

Fan-out `nStripes` parallel Lambda invocations (max 50 concurrent via `asyncPool`).

Each stripe processes rows `[i1_start, i1_end)` of the N x N grid:
```
stripe s: i1_start = s * rowsPerStripe
           i1_end = (s+1) * rowsPerStripe  (last stripe gets remainder)
```

Each Lambda receives:
- `job_id`, `stripe_idx`, `function`, `n1`, `n2`, `i1_start`, `i1_end`
- `param_transforms` (from chip chain), `coeff_transforms`, `times`

Returns: `{degree, n_coeffs, coeffs_key, coeffs_size}`

Output: `renders/{job_id}/coeffs_{stripe:04d}.bin`

Status: "Generating coefficients 3/50..."

### Phase 1b: Lores Coeffgen

Single Lambda call for a smaller preview grid:
```javascript
loresN = min(n, max(10, ceil(sqrt(250000 / degree))))
```
Targets ~250K roots for consistent preview quality regardless of degree.

Output: `renders/{job_id}/lores_coeffs.bin`

### Phase 2: Lores Solve

Single Lambda call — solves the lores grid for viewport computation and quick preview.

Output: `renders/{job_id}/lores.bin`

### Phase 3: Hires Solve

Fan-out `nStripes` parallel solve Lambdas (max 50 concurrent).

Each reads its coefficient file from Phase 1:
```
stripe s reads: renders/{job_id}/coeffs_{s:04d}.bin
```

Returns: `{stripe_idx, s3_key, bin_size, compute_us, n_t, avg_iterations}`

Output: `renders/{job_id}/stripe_{s}.bin`

Status: "Solving 15/50 stripes..."

### Phase 4: Save Metadata

Calls `/save-metadata` on the storage Lambda with full compute metadata.

Output: `renders/{job_id}/calc.json`

Also caches metadata in `_lastCalcMeta` for immediate use by the RENDER tab, and auto-populates the Results Dir field.

## Job ID

Generated at compute start:
```javascript
const jobId = 'compute_' + Date.now().toString(36);
```

Example: `compute_mmvtc0zf`

## Status Display

**Status line** (`#compute-status`): Single colored line showing current phase.

**Log** (`#compute-log`): Scrollable timestamped log (max 500 entries), shows:
```
[11:02:20] Compute: [unit_circle] poly_33 [rev] N=500, stripes=50...
[11:02:26]   coeffgen: 5.8s, degree=70, size=2.84GB (50 stripes)
[11:02:32]   lores solve: 0.4s
[11:05:40]   solve: 198s (sum) / 45s wall (50 stripes)
[11:05:40] Compute compute_mmvtc0zf: deg70 N=500, avg_iters=13.0
```

## Error Handling

- Entire `runCalculate()` wrapped in try/catch/finally
- Lambda calls retry up to 5 times with exponential backoff (for 503, 429, network errors)
- Other HTTP errors fail immediately
- On error: status turns red, error logged
- Finally: Calculate button always re-enabled

## No Resume

There is no partial resume. If a compute fails mid-way:
- Coefficient files already in S3 remain (not cleaned up)
- User must start a fresh compute (new job ID)
- The old partial job shows as "?" in the Results tab (no calc.json)
