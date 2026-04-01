# Root Computation and Storage

## Overview

Roots are computed by the `sweep` binary (`sweep_cli.c`). There are two modes:

1. **Grid mode** (`mode=grid`): generates coefficients from a polynomial function inline and solves at each grid point
2. **Solve mode** (`mode=solve`): reads pre-computed coefficients from a `.bin` file and solves each set

Both use the Ehrlich-Aberth iterative solver.

## Ehrlich-Aberth Solver

**Function:** `solveEA(cr, ci, n, rRe, rIm, degree)`

**Algorithm:** Simultaneous root-finding for all `degree` roots in parallel. Each iteration:

1. **Horner evaluation**: Compute p(z_i) and p'(z_i) simultaneously for each root z_i
2. **Newton step**: w_i = p(z_i) / p'(z_i)
3. **Aberth correction**: s_i = sum over j != i of 1/(z_i - z_j) — repulsion from other roots
4. **Update**: z_i -= w_i / (1 - w_i * s_i)
5. **Convergence**: stop when max |correction|^2 < TOL2 across all roots

**Constants:**
- `MAX_ITER = 64` — maximum iterations per solve
- `TOL2 = 1e-16` — squared convergence tolerance

**Warm-starting:** Previous step's roots initialize the next solve. Initial guess for the first step is roots on a perturbed unit circle:
```
angle = 2*pi*k/degree + 0.3
radius = 1.0 + 0.1*k/degree
```

**Degenerate cases:**
- |p'(z)| < 1e-60: skip this root's update (don't divide by zero)
- |z_i - z_j| < 1e-60: skip this pair in Aberth correction

## Special Cases

| Degree | Method |
|--------|--------|
| 0 | All roots = 0 (constant polynomial, no roots) |
| 1 | Analytical: z = -b/a via complex division |
| >= 2 | Ehrlich-Aberth iterative solver |

**Leading zero stripping:** Before solving, coefficients with magnitude < 1e-30 are skipped from the leading end. This reduces the effective degree. Output still contains `degree` root slots (extras are zero).

## Root Matching

**Function:** `matchRoots(newRe, newIm, oldRe, oldIm, n)`

Greedy nearest-neighbor matching: for each old root, find the closest unmatched new root. This preserves root identity across consecutive parameter steps, giving smooth root paths.

**Complexity:** O(n^2) per step. Expensive for high-degree polynomials.

**When applied:** Only when `match_roots=true` and `totalSteps > 1`. In solve-from-coefficients mode, matching is enabled by default.

## Solve-From-Coefficients Mode

Used by the coeffgen+solve pipeline where coefficients are pre-computed.

1. Open the coefficients file
2. Read `nCoeffs * 2` float32 values per step (re/im pairs)
3. Convert float32 → double
4. Strip leading zeros, determine effective degree
5. Solve (special case or Ehrlich-Aberth)
6. Match roots against previous step (if enabled)
7. Write `degree * 2` float32 values per step to output
8. **Loop until EOF** — reads all data in the file regardless of grid dimensions

The solver doesn't need to know `times` or grid dimensions. It reads until `fread` returns short.

## Grid Mode

Used for direct polynomial evaluation + solve in a single step.

1. Look up coefficient function by name
2. Probe at (0,0) to determine degree
3. For each grid point (i1, i2):
   - Compute x1 = i1/n1, x2 = i2/n2
   - Alternating scan: odd rows reverse j direction (space-filling)
   - Call coefficient function
   - Strip leading zeros, solve
   - Match roots (if enabled)
   - Write float32 root pairs

## Binary Output Format (.bin root files)

```
Per step (repeated for n_t steps):
  float32 root[0].re
  float32 root[0].im
  float32 root[1].re
  float32 root[1].im
  ...
  float32 root[degree-1].re
  float32 root[degree-1].im
```

- **Stride per step:** `degree * 2 * 4` bytes
- **Total file size:** `n_t * degree * 2 * 4` bytes
- **Byte order:** little-endian (native)
- **Root order:** if matching enabled, roots are ordered to track identity across steps

## Metadata (stdout JSON)

```json
{
  "mode": "solve",
  "degree": 9,
  "n_t": 300,
  "stride": 18,
  "matched": true,
  "data_bytes": 21600,
  "elapsed_us": 507,
  "avg_iterations": 5.18
}
```

- `stride`: floats per step (degree * 2)
- `avg_iterations`: mean Ehrlich-Aberth iterations across all steps
- `n_t`: total steps processed (matches coefficients file size, not grid dimensions)

## Lambda Invocation

The sweep Lambda (`polypaint-sweep`) is dispatched asynchronously via the dispatch Lambda (fire-and-forget, `InvocationType=Event`), the same pattern used for raster/finalize/encode. The frontend sends sweep jobs to `/dispatch-render` with `target: 'sweep'`, then polls `/check-status` with `task_prefix: 'sweep_'` every 3 seconds.

The sweep Lambda reports its progress to DynamoDB via `report_status()` with task_id `sweep_{chunk_idx}`. On completion, it stores sweep metadata (`bin_size`, `compute_us`, `n_t`, `avg_iterations`) as `result_data` in the DynamoDB item. For compatibility the payload still includes `stripe_idx`, but the current compute pipeline is chunk-based and writes `renders/{job_id}/chunk_{chunk_idx}.bin`.

See [dynamodb.md](dynamodb.md) for the full status tracking protocol.
