# Multi-Threaded AE Solve Plan

This plan adds a new solver path:

- UI button: `AE-mt`
- Lambda name: `polypaint-sweep-mt`
- API route: `POST /sweep-mt`
- Dispatch target: `sweep_mt`
- Binary: `lambda/sweep_mt`

The goal is to keep the existing chunked compute pipeline unchanged except for the solve phase, and to parallelize Aberth-Ehrlich inside a single solve Lambda by splitting the chunk's coefficient records across a fixed worker pool.

## Current Fit

The current compute flow in [index.html](./index.html) is:

1. `param_gen`
2. `coeffgen_chunked`
3. lores solve via direct call to `/sweep` or `/sweep-cm`
4. hires solve via async dispatch target `sweep` or `sweep_cm`
5. poll DynamoDB task rows with task prefix `sweep_`

Relevant current files:

- [index.html](./index.html)
- [lambda/handler_sweep.py](./lambda/handler_sweep.py)
- [lambda/handler_dispatch.py](./lambda/handler_dispatch.py)
- [deploy.sh](./deploy.sh)
- [lambda/sweep_cli.c](./lambda/sweep_cli.c)

This means the clean integration is:

- keep `param_gen` unchanged
- keep `coeffgen_chunked` unchanged
- add a third solver mode alongside AE and CM
- reuse the same output `.bin` format
- reuse the same DynamoDB status reporting shape
- reuse the same chunk fanout architecture

## Why A Separate Lambda

Do not overload the existing `polypaint-sweep` Lambda.

Reasons:

- the single-threaded AE path should remain the stable baseline
- the multi-threaded path will want different memory sizing
- the multi-threaded path may need different binary build flags
- this makes rollout and regression comparison much easier
- if the MT solver regresses, fallback is trivial

So the intended split is:

- `polypaint-sweep` = existing single-threaded AE
- `polypaint-sweep-mt` = internal worker-pool AE
- `polypaint-sweep-cm` = companion matrix

## Runtime Target

The existing AE deployment target is already high-memory:

- [deploy.sh](./deploy.sh) sets `SWEEP_MEMORY=10240`

That is the right sizing direction for `AE-mt` too.

Recommendation:

- set `SWEEP_MT_MEMORY=10240`
- keep `Timeout=900`
- use `arm64`

Treat this as a compute-optimized Lambda:

- large memory
- large `/tmp`
- no image layers

## Solver Model

Each chunk already contains:

- one coefficient file
- `n_steps`
- `n_coeffs`

Each polynomial inside that coefficient file is independent for the current solve path because:

- [lambda/handler_sweep.py](./lambda/handler_sweep.py) uses `match_roots: False`

That means the correct parallel unit is:

- one coefficient record = one polynomial solve

Do not spawn one thread per polynomial.

Instead:

- detect available worker count once
- partition `n_steps` into contiguous step ranges
- assign one range to each worker
- each worker solves its own range sequentially
- each worker writes roots into preassigned offsets in the shared output buffer

This preserves:

- deterministic output order
- current binary format
- no per-root locking

## Binary Design

Add a new C binary:

- `lambda/sweep_mt.c`

Recommended implementation approach:

1. Copy the existing solve-only parts from [lambda/sweep_cli.c](./lambda/sweep_cli.c).
2. Strip everything not needed for solving from precomputed coefficients.
3. Build a worker-pool around the per-polynomial solve loop.
4. Keep the same JSON input contract as the current solve handler.
5. Keep the same output binary layout.

### Input Contract

Use the same solve input shape as current AE:

```json
{
  "mode": "solve_mt",
  "coeffs_file": "/tmp/coeffs.bin",
  "n_coeffs": 25,
  "n2": 100000,
  "i1_start": 0,
  "i1_end": 1,
  "match_roots": false
}
```

For simplicity, keep compatibility with the current handler model:

- `n_steps = n2 * (i1_end - i1_start)`

Or add direct `n_steps` support in the binary if preferred.

### Output Contract

Match current AE output exactly:

- raw interleaved float32 root pairs
- no header
- one polynomial after another
- each record emits `degree` roots

Metadata JSON should mirror the existing solve mode:

```json
{
  "mode": "solve_mt",
  "degree": 24,
  "n1": 1,
  "n2": 100000,
  "i1_start": 0,
  "i1_end": 1,
  "n_t": 100000,
  "compute_us": 1234567,
  "avg_iterations": 18.4,
  "n_threads": 6
}
```

Add `n_threads` for observability.

## Worker Pool Design

Use a fixed worker pool in C with `pthread`.

Recommended shape:

```c
typedef struct {
    const float *coeffs;
    float *roots;
    int n_coeffs;
    int degree;
    long step_start;
    long step_end;
    double iter_sum;
    long solved;
} SolveTask;
```

Each worker:

- reads coeffs for step `i` from:
  - `coeffs + i * n_coeffs * 2`
- solves that polynomial with the same AE routine used today
- writes roots to:
  - `roots + i * degree * 2`
- accumulates local iteration totals

After join:

- sum all worker `iter_sum`
- sum all worker `solved`
- compute weighted `avg_iterations`

### Preserve Order

Do not let workers append to a shared file.

Instead:

- allocate the full output root buffer in memory
- write each step's roots to its deterministic slot
- write the buffer once at the end

Offset formulas:

- coeff input offset:
  - `i * n_coeffs * 2`
- root output offset:
  - `i * degree * 2`

This is the simplest correct design.

## Thread Count

Detect thread count inside the binary.

Preferred order:

1. explicit env override:
   - `SWEEP_MT_THREADS`
2. otherwise CPU count:
   - `sysconf(_SC_NPROCESSORS_ONLN)`
3. clamp to sane bounds

Recommended policy:

- minimum: `1`
- maximum: detected cores
- never exceed `n_steps`

Example:

```c
long cores = sysconf(_SC_NPROCESSORS_ONLN);
int n_threads = (int)(cores > 0 ? cores : 1);
const char *ov = getenv("SWEEP_MT_THREADS");
if (ov && *ov) n_threads = atoi(ov);
if (n_threads < 1) n_threads = 1;
if ((long)n_threads > n_steps) n_threads = (int)n_steps;
```

## Memory Model

For each chunk:

- input coeff file already exists in `/tmp`
- current handler downloads it whole

MT solve should:

1. read the whole coefficient file into RAM
2. allocate one output buffer for all roots
3. run workers over that memory
4. write final output file

Memory estimate:

- coeff buffer:
  - `n_steps * n_coeffs * 2 * 4`
- root buffer:
  - `n_steps * degree * 2 * 4`

Since `degree = n_coeffs - 1`, total working set is roughly:

- `~ 2 * n_steps * n_coeffs * 2 * 4`

You should keep the existing chunk-size preflight in the UI and may want a second stricter preflight for `AE-mt` because it holds both coeffs and roots in memory at once.

## Handler Design

Add:

- `lambda/handler_sweep_mt.py`

Model it after:

- [lambda/handler_sweep.py](./lambda/handler_sweep.py)

Same contract:

- input: `job_id`, `chunk_idx`, `coeffs_key`, `n_coeffs`, `n_steps`, optional `s3_key`
- output: same result_data shape

Differences:

- binary path points to `sweep_mt`
- metadata can include `n_threads`

Recommended result_data:

```python
result_data = {
    "stripe_idx": stripe_idx,
    "chunk_idx": stripe_idx,
    "s3_key": s3_key,
    "bin_size": bin_size,
    "compute_us": compute_us,
    "n_t": compute_meta["n_t"],
    "degree": compute_meta["degree"],
    "avg_iterations": compute_meta["avg_iterations"],
    "n_threads": compute_meta.get("n_threads", 1),
}
```

Keep both `stripe_idx` and `chunk_idx` for compatibility.

## Dispatch Integration

Update:

- [lambda/handler_dispatch.py](./lambda/handler_dispatch.py)

Add:

```python
"sweep_mt": os.environ.get("SWEEP_MT_FUNCTION", "polypaint-sweep-mt"),
```

The browser should use:

- `target: 'sweep_mt'`

for hires fanout when `solverMode === 'aberth_mt'`.

## Frontend Integration

Update [index.html](./index.html).

### Button

Add a third compute button near the existing ones:

```html
<button class="btn-primary" id="btn-calculate-ae-mt" onclick="runCalculateAEMT()" style="background:#3b7">AE-mt</button>
```

### Solver Mode Entry Point

Add:

```js
async function runCalculateAEMT() { return runCalculateWithSolver('aberth_mt'); }
```

### Button Binding

Extend button selection in `runCalculateWithSolver()`:

```js
const btn = document.getElementById(
  solverMode === 'companion_matrix' ? 'btn-calculate-cm' :
  solverMode === 'aberth_mt' ? 'btn-calculate-ae-mt' :
  'btn-calculate'
);
```

### Lores Solve

For lores direct solve:

```js
const loresSolveLambda =
  solverMode === 'companion_matrix' ? 'sweep-cm' :
  solverMode === 'aberth_mt' ? 'sweep-mt' :
  'sweep';
```

### Hires Dispatch

For hires async dispatch:

```js
const solverTarget =
  solverMode === 'companion_matrix' ? 'sweep_cm' :
  solverMode === 'aberth_mt' ? 'sweep_mt' :
  'sweep';
```

### Metadata

Store solver mode explicitly:

```js
solver: solverMode,
```

This already exists and should just receive `aberth_mt`.

## Deploy Integration

Update [deploy.sh](./deploy.sh).

### Variables

Add:

```bash
SWEEP_MT_NAME="polypaint-sweep-mt"
SWEEP_MT_MEMORY=10240
```

### Build

Compile:

```bash
aarch64-linux-musl-gcc -O3 -pthread -static -o lambda/sweep_mt lambda/sweep_mt.c -lm
```

Notes:

- use `-pthread`
- static build like current AE is preferable
- avoid dragging in LAPACK or other large runtime dependencies

### Package

Create a dedicated package:

- `handler_sweep_mt.py`
- `shared.py`
- `sweep_mt`

### Lambda Create/Update

Add `create_lambda` / `update_lambda` entries mirroring `polypaint-sweep`.

Recommended env:

```bash
"BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE"
```

Optional env:

```bash
"BUCKET=$BUCKET,JOBS_TABLE=$JOBS_TABLE,SWEEP_MT_THREADS=6"
```

I recommend leaving thread count auto-detected by default and only using the env var for benchmarking.

### API Gateway

Add route:

- `POST /sweep-mt`

Update generated `config.json` with:

```json
"sweep-mt": ".../sweep-mt"
```

## Performance Guidance

Performance is the whole point here.

Expected behavior:

- better throughput on large chunks
- little benefit on tiny chunks
- best gains when:
  - degree is moderate/high
  - chunk has many steps
  - Lambda has multiple vCPUs

Do not assume linear scaling.

Benchmark dimensions:

- degree
- n_steps per chunk
- number of threads
- average iterations
- total wall clock

Measure:

- AE single-thread
- AE-mt with auto thread count
- CM

## Test Strategy

### 1. Binary Correctness

Add:

- `tests/test_ae_mt.py`

Core checks:

- same output size as AE
- all roots finite
- residuals below threshold
- AE and AE-mt root sets agree within tolerance
- metadata includes `n_threads >= 1`

Fixture set:

- exact cubic
- exact quartic
- repeated root
- complex roots
- random stable coefficient sets

### 2. Handler Test

Add:

- `lambda/test_pipeline_mt.py` or extend [lambda/test_pipeline.py](./lambda/test_pipeline.py)

Checks:

- `handler_sweep_mt.py` uploads expected S3 key
- reports DynamoDB `done`
- propagates subprocess failures correctly
- returns `chunk_idx`

### 3. Docker Runtime Test

Extend:

- [scripts/test-docker-runtime.sh](./scripts/test-docker-runtime.sh)

Add:

- build/validate `lambda/sweep_mt`
- run AE vs AE-mt on the same coefficient fixtures
- compare:
  - root count
  - finiteness
  - residual thresholds
  - timing

### 4. UI Smoke Test

Manual:

- run a small job with `Calculate`
- run the same job with `AE-mt`
- compare:
  - `total_roots`
  - preview image
  - render output
  - wall time

## Implementation Checklist

### Phase 1: Binary

- create `lambda/sweep_mt.c`
- copy the current solve-from-coefficients AE path only
- add `pthread` worker pool
- add thread auto-detection
- add deterministic output buffer writes
- emit JSON metadata with `n_threads`
- benchmark locally on representative coefficient files

### Phase 2: Handler

- add `lambda/handler_sweep_mt.py`
- mirror `handler_sweep.py`
- return both `stripe_idx` and `chunk_idx`
- include `n_threads` in result_data

### Phase 3: Frontend

- add `AE-mt` button to Compute tab
- add `runCalculateAEMT()`
- extend `runCalculateWithSolver()` for:
  - button binding
  - lores direct solve endpoint
  - hires dispatch target
  - status labeling if desired

### Phase 4: Dispatch / Config

- add `sweep_mt` target in `handler_dispatch.py`
- add `/sweep-mt` config endpoint generation
- add route creation in deploy

### Phase 5: Deploy

- add `SWEEP_MT_NAME`
- add `SWEEP_MT_MEMORY=10240`
- compile `sweep_mt`
- package Lambda zip
- create/update Lambda
- wire API Gateway route

### Phase 6: Tests

- add binary correctness tests
- add handler tests
- extend Docker runtime script
- verify AE vs AE-mt on ARM64 runtime

## Acceptance Criteria

The feature is ready when all of these are true:

- `AE-mt` button appears and completes full compute jobs
- lores solve works through `/sweep-mt`
- hires solve fanout works through dispatch target `sweep_mt`
- output root format is identical to current AE
- render pipeline accepts AE-mt jobs with no downstream changes
- Docker runtime regression passes on ARM64
- AE-mt is measurably faster than single-thread AE on large chunks
- no correctness regressions against AE on selected fixture sets

## Recommended First Cut

Keep the first version narrow:

- solve-from-coefficients only
- `match_roots = false` only
- fixed contiguous range partitioning
- no work stealing
- no per-thread file IO
- one shared output buffer

That will fit the current codebase best and give the highest chance of a fast, correct first implementation.
