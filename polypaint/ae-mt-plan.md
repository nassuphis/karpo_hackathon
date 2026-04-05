# AE-MT Plan (Current State)

This plan updates the stale AE multithreading notes to match the codebase that ships today.

The goal is still the same:

- add a third Compute solver mode for Aberth-Ehrlich with intra-Lambda CPU parallelism
- keep the current chunked compute pipeline unchanged everywhere else
- preserve the existing downstream root `.bin` contract so Render, Palette, DeepZoom, and storage do not care which solver produced the roots

The current codebase already uses concurrency in several places:

- [lambda/handler_dispatch.py](./lambda/handler_dispatch.py) uses a `ThreadPoolExecutor` for async invoke fanout
- libvips-backed binaries benefit from vCPU scaling in `encode`, `bilevel_stitch`, `deepzoom_export`, `render_preview`, `autolevels`, and TIFF/PNG export
- [lambda/handler_repalette.py](./lambda/handler_repalette.py) already parallelizes S3 copy work

But the main AE solve path is still single-threaded:

- [lambda/handler_sweep.py](./lambda/handler_sweep.py)
- [lambda/sweep_cli.c](./lambda/sweep_cli.c)

That is the missing CPU-bound piece this document targets.

## Scope

This plan is only for Compute-tab AE solving.

In scope:

- a new `Calculate-AE-MT` button in Compute
- a new `polypaint-sweep-mt` Lambda
- a new `sweep_mt` dispatch target
- a new `/sweep-mt` direct route for lores solve
- a new native solve binary `lambda/sweep_mt`

Out of scope for this document:

- implementing multithreaded `param_gen`
- implementing multithreaded `coeffgen_chunked`
- multithreading `roots2pix`
- multithreading `solve_proximity_stats`
- changing Step Functions render/palette workflows
- replacing the existing single-thread AE baseline

Those can come later, but they are not this first cut.

## Current Compute Flow

The current frontend compute flow lives in [index.html](./index.html), inside `runCalculateWithSolver(...)`.

Today it has two solver modes:

- `aberth` via `Calculate-AE`
- `companion_matrix` via `Calculate-CM`

The current pipeline is:

1. `param_gen` via [lambda/handler_coeffgen.py](./lambda/handler_coeffgen.py)
2. `coeffgen_chunked` via the same Lambda
3. lores solve via direct API call to `/sweep` or `/sweep-cm`
4. hires solve via dispatch target `sweep` or `sweep_cm`
5. browser polling `/check-status` with `task_prefix: "sweep_"`
6. metadata save to `calc.json` with `solver: solverMode`

Relevant current files:

- [index.html](./index.html)
- [lambda/handler_sweep.py](./lambda/handler_sweep.py)
- [lambda/handler_sweep_cm.py](./lambda/handler_sweep_cm.py)
- [lambda/handler_dispatch.py](./lambda/handler_dispatch.py)
- [deploy.sh](./deploy.sh)
- [lambda/sweep_cli.c](./lambda/sweep_cli.c)
- [tests/test_pipeline.py](./tests/test_pipeline.py)
- [tests/test_dispatch_resilience.py](./tests/test_dispatch_resilience.py)
- [tests/docker_runtime_regression.py](./tests/docker_runtime_regression.py)
- [scripts/test-docker-runtime.sh](./scripts/test-docker-runtime.sh)

## Important Current-State Correction

The old plan assumed:

- "`match_roots: false` means each coefficient record is fully independent"

That was only true because of a real bug.

In [lambda/sweep_cli.c](./lambda/sweep_cli.c), the solve loops were zeroing the entire root array before every step, which accidentally killed the intended warm-start chain. That bug has now been fixed:

- single-thread AE again preserves the previous step's solved roots in the active solver slots
- only the trailing-zero root slots are force-set to `0`
- the shipped Compute path still sends `match_roots: false`, so the greedy reorder remains off

That restores the real intended AE semantics:

- there is again a genuine cross-step warm-start chain in single-thread AE
- AE-MT cannot be exact across the whole chunk when `n_threads > 1`
- the correct v1 model is block-parallel with warm starts preserved only within each worker block
- `avg_iterations` will be higher than single-thread AE as thread count increases

This also restores the original complexity ranking:

- `param_gen` and `coeffgen_chunked` are still cleaner MT follow-ons
- AE solve is harder because of the warm-start chain across steps
- the semantic non-goal for v1 remains `match_roots=true`

## Recommended Solver Model

Use a separate Lambda and a separate binary:

- Lambda: `polypaint-sweep-mt`
- handler: `lambda/handler_sweep_mt.py`
- binary: `lambda/sweep_mt`
- frontend solver token: `aberth_mt`
- dispatch target: `sweep_mt`
- API route: `POST /sweep-mt`

Do **not** overload the existing `polypaint-sweep`.

Reasons:

- single-thread AE remains the exact reference path
- AE-MT has different performance characteristics and slightly different warm-start semantics at block boundaries
- rollout and rollback are trivial
- comparison against AE and CM stays easy

Resulting solver split:

- `polypaint-sweep` = single-thread AE baseline
- `polypaint-sweep-mt` = block-parallel AE with local warm starts
- `polypaint-sweep-cm` = companion-matrix eigensolve

## Binary Design

Add a new dedicated solve-only binary:

- `lambda/sweep_mt.c`

Do **not** fork the full [lambda/sweep_cli.c](./lambda/sweep_cli.c) with all coeffgen, transform, and catalog code.

The new file should only carry the solve-side subset:

- `solveEA(...)`
- JSON helpers needed for solve mode
- the degree-1 / degenerate handling from `runSolveFromCoeffs(...)`
- file IO for coefficient chunk in, roots chunk out
- block worker pool

If a small refactor is acceptable, the cleaner version is:

- extract the shared solve-only helpers into `lambda/sweep_solve_core.h`
- include that from both `sweep_cli.c` and `sweep_mt.c`

But the minimal acceptable first cut is:

- `sweep_mt.c` as a trimmed solve-only translation unit copied from the current solve subset

## V1 Parallelization Strategy

### What v1 should do

Partition the chunk's step range into contiguous thread-local blocks:

- worker 0: `[0, a)`
- worker 1: `[a, b)`
- worker 2: `[b, c)`
- ...

Each worker:

- reads the coefficient records for its block
- solves the block sequentially
- warm-starts from the previous step within that block
- cold-starts the first step in the block from the same perturbed-circle seed used by single-thread AE
- writes roots into deterministic slots in the shared output buffer

This gives:

- no locks on the output path
- deterministic file layout
- substantial throughput gain on large chunks
- bounded implementation complexity

### What v1 will not preserve exactly

It will not preserve the single-thread AE warm-start chain across the entire chunk.

At each block boundary, the first step in the block starts from the standard perturbed-circle seed:

```c
double ang = 2.0 * M_PI * k / degree + 0.3;
double r = 1.0 + 0.1 * k / degree;
rootRe[k] = r * cos(ang);
rootIm[k] = r * sin(ang);
```

So the first-cut AE-MT contract is:

- same output shape
- same degree
- same root count
- same residual tolerance
- deterministic output layout
- exact parity with single-thread AE when `n_threads=1`
- possible root-order and iteration-count drift across worker-block boundaries when `n_threads>1`

That tradeoff is acceptable because:

- the baseline AE path remains available
- CM remains available
- most downstream consumers care about the solved root set and residual quality, not exact iteration history

### Explicit v1 non-goals

Do not attempt these in the first implementation:

- exact chunk-wide parity with single-thread AE at `n_threads>1`
- `match_roots=true`
- work stealing
- per-step dynamic scheduling
- shared append-only output
- per-thread file writes followed by merge

Keep it simple:

- contiguous blocks
- one shared output buffer
- one final write

The exactness statement for v1 is:

- exact when `n_threads=1`
- approximate but deterministic when `n_threads>1`
- explicitly unsupported for `match_roots=true`

## Input and Output Contract

### Input

Mirror the current solve-from-coefficients contract as closely as possible.

Preferred input:

```json
{
  "mode": "solve_mt",
  "coeffs_file": "/tmp/coeffs.bin",
  "n_coeffs": 25,
  "n_steps": 100000
}
```

Accept the legacy shape too, because [lambda/handler_sweep.py](./lambda/handler_sweep.py) and [lambda/handler_sweep_cm.py](./lambda/handler_sweep_cm.py) still derive `n_steps` from older fields when needed:

```json
{
  "mode": "solve_mt",
  "coeffs_file": "/tmp/coeffs.bin",
  "n_coeffs": 25,
  "n2": 100000,
  "i1_start": 0,
  "i1_end": 1
}
```

Do not support `match_roots=true` in v1.

If the spec requests it:

- either force it to `false`
- or return a clear error

I recommend explicit rejection, because silent fallback would hide an important semantic change.

### Output

Match current root `.bin` layout exactly:

- raw interleaved float32 root pairs
- no header
- one polynomial solve after another
- `degree` roots per solve

Metadata JSON should look like current AE/CM output plus `n_threads`:

```json
{
  "mode": "solve_mt",
  "degree": 24,
  "n_t": 100000,
  "data_bytes": 19200000,
  "elapsed_us": 1234567,
  "avg_iterations": 18.4,
  "n_threads": 6
}
```

Keep `avg_iterations`; it is already consumed by the current Compute UI summary.

## Thread Count Policy

Detect thread count inside the binary.

Recommended order:

1. explicit env override: `SWEEP_MT_THREADS`
2. `sysconf(_SC_NPROCESSORS_ONLN)`
3. clamp to:
   - minimum `1`
   - maximum `n_steps`

Recommended default:

- auto-detect
- no hardcoded thread count in the browser or handler

Optional env for benchmarking:

```text
SWEEP_MT_THREADS=6
```

## Memory Model

Current [lambda/handler_sweep.py](./lambda/handler_sweep.py) downloads the entire coefficient chunk before invoking the binary.
AE-MT can keep that model.

Working set for one chunk is roughly:

- coefficient buffer:
  - `n_steps * n_coeffs * 2 * 4`
- root output buffer:
  - `n_steps * degree * 2 * 4`

Plus:

- per-thread scratch root arrays
- per-thread temporary coefficient doubles

The binary should also have an explicit runtime sanity check before allocation.

At minimum:

- estimate coefficient bytes
- estimate root-output bytes
- estimate thread scratch bytes
- fail early with a readable error if the total would exceed a conservative bound

Do not rely on `malloc` failure alone.

That means AE-MT should keep the same high-memory sizing class as current sweep:

- `SWEEP_MT_MEMORY=10240`
- `Timeout=900`
- `arm64`
- large `/tmp`

The existing Compute-tab chunk-size preflight should remain in place.

But add a stricter AE-MT note:

- AE-MT holds both input coefficients and full output roots in memory at once
- so its safe chunk size is lower than pure streaming coeffgen

## Handler Design

Add:

- `lambda/handler_sweep_mt.py`

Model it after [lambda/handler_sweep_cm.py](./lambda/handler_sweep_cm.py), not the stale assumptions in the old doc.

Current compatibility rules to preserve:

- input fields: `job_id`, `chunk_idx`, `coeffs_key`, `n_coeffs`, `n_steps`, optional `s3_key`
- task id format stays `sweep_{chunk_idx}`
- result payload keeps both `chunk_idx` and `stripe_idx`
- frontend continues polling `task_prefix: "sweep_"`

Recommended result payload:

```python
result_data = {
    "chunk_idx": chunk_idx,
    "stripe_idx": chunk_idx,
    "s3_key": s3_key,
    "bin_size": bin_size,
    "compute_us": compute_us,
    "n_t": compute_meta["n_t"],
    "degree": compute_meta["degree"],
    "avg_iterations": compute_meta["avg_iterations"],
    "n_threads": compute_meta.get("n_threads", 1),
}
```

And the HTTP response can keep the legacy compatibility field:

For AE-MT, do **not** blindly preserve the stale `n_procs: 1` field from the current AE/CM handlers.

Current state:

- [lambda/handler_sweep.py](./lambda/handler_sweep.py) returns `n_procs: 1`
- [lambda/handler_sweep_cm.py](./lambda/handler_sweep_cm.py) returns `n_procs: 1`
- nothing in the frontend reads it
- it only survives in tests/docs as legacy baggage

So AE-MT should do one of these:

1. preferred: omit `n_procs` entirely and return `n_threads`
2. compatibility fallback: set `n_procs` equal to `n_threads`

Do **not** return `n_procs: 1` from a multithreaded solver.

Do not rename the DynamoDB task prefix in v1.
`handler_sweep_cm.py` already proved that the current UI can support multiple solver backends while still using `sweep_{chunk_idx}`.

## Frontend Integration

Update [index.html](./index.html).

### Buttons

The current buttons are:

- `Calculate-AE`
- `Calculate-CM`

Add a third:

```html
<button class="btn-primary" id="btn-calculate-mt" onclick="runCalculateAEMT()" style="background:#287">Calculate-AE-MT</button>
```

### Solver entry point

Add:

```js
async function runCalculateAEMT() { return runCalculateWithSolver('aberth_mt'); }
```

### Button binding

Update `runCalculateWithSolver(...)` so button selection becomes:

```js
const btn = document.getElementById(
  solverMode === 'companion_matrix' ? 'btn-calculate-cm' :
  solverMode === 'aberth_mt' ? 'btn-calculate-mt' :
  'btn-calculate'
);
```

### Lores direct solve

Current code chooses between `/sweep` and `/sweep-cm`.
Extend it to:

```js
const loresSolveLambda =
  solverMode === 'companion_matrix' ? 'sweep-cm' :
  solverMode === 'aberth_mt' ? 'sweep-mt' :
  'sweep';
```

### Hires async dispatch

Current code chooses between `sweep` and `sweep_cm`.
Extend it to:

```js
const solverTarget =
  solverMode === 'companion_matrix' ? 'sweep_cm' :
  solverMode === 'aberth_mt' ? 'sweep_mt' :
  'sweep';
```

### Metadata persistence

This already exists:

```js
solver: solverMode,
```

That should simply start receiving:

- `"aberth_mt"`

### Populate flow

Current `_populateComputeFromDetail(...)` only distinguishes:

- `companion_matrix`
- everything else

That needs to be updated so restored status text is accurate for:

- `aberth`
- `aberth_mt`
- `companion_matrix`

## Dispatch Integration

Update [lambda/handler_dispatch.py](./lambda/handler_dispatch.py):

```python
"sweep_mt": os.environ.get("SWEEP_MT_FUNCTION", "polypaint-sweep-mt"),
```

The dispatch Lambda does not need any other behavior change.

## Deploy Integration

Update [deploy.sh](./deploy.sh).

### Variables

Add:

```bash
SWEEP_MT_NAME="polypaint-sweep-mt"
SWEEP_MT_MEMORY=10240
```

### Build

Compile the new binary:

```bash
aarch64-linux-musl-gcc -O3 -pthread -static -o lambda/sweep_mt lambda/sweep_mt.c -lm
```

Local test build:

```bash
cc -O3 -pthread -o lambda/sweep_mt_test lambda/sweep_mt.c -lm
```

### Packaging

Create a dedicated Lambda zip containing:

- `lambda/handler_sweep_mt.py`
- `lambda/shared.py`
- `lambda/sweep_mt`

### Lambda create/update

Add `create_lambda` and `update_lambda` entries mirroring `polypaint-sweep`.

Recommended env:

```bash
"BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"
```

Optional benchmark override:

```bash
"BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,SWEEP_MT_THREADS=6"
```

### Dispatch env

Update the dispatch Lambda environment so it receives:

```bash
SWEEP_MT_FUNCTION=$SWEEP_MT_NAME
```

### API Gateway

Add:

- `POST /sweep-mt`

and add it to generated `config.json`:

```json
"sweep-mt": ".../sweep-mt"
```

### Doc/comment cleanup

While touching deploy, fix stale wording that still says:

- `polypaint-sweep — multi-process root solver`

The current AE path is single-threaded today.

## Tests

### 1. Local binary tests

Add:

- `tests/test_ae_mt.py`

Checks:

- same output size as AE
- all roots finite
- residuals below threshold
- `n_threads >= 1`
- block-boundary resets do not produce garbage

Do **not** require exact bitwise equality with AE.

Instead compare:

- root count
- finiteness
- max residual
- rough root-set agreement on stable fixtures

### 2. Handler tests

Extend [tests/test_pipeline.py](./tests/test_pipeline.py):

- mirror the existing `handler_sweep.py` / `handler_sweep_cm.py` tests
- add `handler_sweep_mt.py` upload/status/error-path coverage
- assert `n_threads` is preserved in `result_data`

### 3. Dispatch tests

Extend:

- [tests/test_dispatch_resilience.py](./tests/test_dispatch_resilience.py)

or the dispatch section in [tests/test_pipeline.py](./tests/test_pipeline.py)

Add:

- `target: "sweep_mt"` is accepted and fired

### 4. Deploy packaging tests

Extend:

- [tests/test_deploy_packaging.py](./tests/test_deploy_packaging.py)

Add:

- `handler_sweep_mt.py` packages `sweep_mt`

### 5. Frontend JS tests

Extend:

- [tests/test_frontend_js.sh](./tests/test_frontend_js.sh)

Checks:

- `Calculate-AE-MT` button exists
- `runCalculateWithSolver('aberth_mt')` uses:
  - `sweep-mt` for lores
  - `sweep_mt` for hires dispatch
- `_populateComputeFromDetail(...)` reports the right solver label for `aberth_mt`

### 6. Docker ARM64 runtime test

Extend:

- [scripts/test-docker-runtime.sh](./scripts/test-docker-runtime.sh)
- [tests/docker_runtime_regression.py](./tests/docker_runtime_regression.py)

Add:

- compile and mount `/src/sweep_mt`
- run AE vs AE-MT on the same coefficient fixtures
- compare:
  - output size
  - finiteness
  - residual thresholds
  - timing

This is the closest local approximation to deployed behavior and should be treated as required before rollout.

### 7. Docs after implementation

After AE-MT lands, update:

- [docs/lambdas.md](./docs/lambdas.md)
- [docs/calc_tab.md](./docs/calc_tab.md)
- [docs/roots.md](./docs/roots.md)
- [docs/testing.md](./docs/testing.md)

## Acceptance Criteria

AE-MT is ready when all of these are true:

- `Calculate-AE-MT` appears in Compute
- lores solve works through `/sweep-mt`
- hires solve fanout works through dispatch target `sweep_mt`
- output `.bin` layout is unchanged
- Render, Palette, and downstream artifact flows accept AE-MT jobs with no special cases
- local handler tests pass
- frontend JS tests pass
- deploy packaging tests pass
- Docker ARM64 runtime regression passes
- `n_threads=1` AE-MT output matches single-thread AE on the parity fixture
- repeated-polynomial regression shows single-thread AE benefiting from the restored warm-start chain relative to multi-thread block cold starts
- AE-MT is measurably faster than single-thread AE on large chunks
- AE remains available as the exact baseline

## Follow-on Candidates (Not This PR)

After AE-MT, the next CPU-bound candidates worth evaluating are:

1. `param_gen`
   - embarrassingly parallel by step
   - current blocker is output strategy, not math
   - [lambda/handler_coeffgen.py](./lambda/handler_coeffgen.py) streams `param_gen` stdout directly to S3 multipart upload, so MT needs deterministic chunk buffering or ordered writer threads
   - dither transforms (`sdith`, `ddith`, `ndith`) already reseed per pass; an MT version should make RNG stateless on `(pass, i1, i2)` or pre-partition ranges so output is identical regardless of thread count
2. `coeffgen_chunked`
   - easier than AE-MT
   - each params record independently produces one coeff record
   - same caveat on dither/RNG reproducibility if coeffgen is ever rejoined with inline param generation
   - current [lambda/sweep_cli.c](./lambda/sweep_cli.c) implementation reads the params slice sequentially from file, but the actual coeff function calls are independent and can be block-parallelized with a shared output buffer
3. `solve_proximity_stats`
   - clip/hist phases are solve-level loops over root records
   - good candidate for block-parallel score computation
4. `roots2pix`
   - only worth it if solve-score/palette reuse is still leaving raster CPU-bound
   - more complicated because of tile bucketing and duplicate-pixel resolution
5. `pixassemble`
   - likely lower value
   - current workload is more memory/write-order sensitive than compute-heavy

So the recommended order is:

1. AE-MT for immediate solve-wall-clock reduction
2. `coeffgen_chunked` MT
3. `param_gen` MT
4. solve-score stats MT
5. raster MT only if profiling proves it matters

## Recommended First Cut

Keep the first implementation narrow:

- solve-from-coefficients only
- `match_roots=false` only
- block-parallel contiguous ranges
- local warm starts within each block
- shared output buffer
- explicit `n_threads` metadata
- exact parity at `n_threads=1`
- explicit rejection of `match_roots=true`

That matches the current codebase best, keeps rollout risk understandable, and preserves the existing AE baseline for exact comparisons.
