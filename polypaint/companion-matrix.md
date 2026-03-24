# Companion Matrix Solve Path

## Goal

Add a second compute path in the Compute tab:

- existing button: `Calculate`
  - current Aberth-Ehrlich solver path
- new button: `Calculate-CM`
  - companion-matrix eigenvalue solve path

The two paths should share:

- the same parameter generation
- the same coefficient generation
- the same chunking
- the same storage layout
- the same render pipeline
- the same `calc.json` shape

They should differ only in the solve stage.

## Why This Is The Right Design

The current compute pipeline is already split into clean phases:

1. `param_gen`
2. `coeffgen_chunked`
3. `solve`
4. render / raster / finalize

That means companion-matrix support does not need a second full pipeline.

It only needs:

- a second solve Lambda
- a second solve binary
- a small UI branch that selects which solver target to dispatch

This is much cleaner than forking the whole compute stack.

## Current Pipeline

Relevant frontend flow:

- [`index.html`](./index.html)
  - phase 0 `param_gen`
  - phase 1 `coeffgen_chunked`
  - phase 2 lores solve
  - phase 3 hires solve

Relevant backend flow:

- [`lambda/handler_coeffgen.py`](./lambda/handler_coeffgen.py)
- [`lambda/handler_sweep.py`](./lambda/handler_sweep.py)
- [`lambda/handler_dispatch.py`](./lambda/handler_dispatch.py)
- [`lambda/sweep_cli.c`](./lambda/sweep_cli.c)

Right now:

- coeffgen produces one coeff file per chunk
- sweep solves one coeff chunk into one roots chunk
- render downstream only cares about the produced roots chunks

That makes the solve phase swappable.

## What `Calculate-CM` Should Mean

`Calculate-CM` should mean:

- run the same `param_gen`
- run the same `coeffgen_chunked`
- use a different lores solve
- use a different hires solve dispatch target
- write the same kind of output `.bin` root chunks
- save metadata indicating which solver was used

Suggested metadata addition in `calc.json`:

```json
{
  "solver": "aberth"
}
```

or

```json
{
  "solver": "companion_matrix"
}
```

Keep the rest of the metadata shape unchanged.

## Recommended Architecture

### Separate Lambda

Recommended new Lambda:

- `polypaint-sweep-cm`

Recommended new handler:

- [`lambda/handler_sweep_cm.py`](./lambda/handler_sweep_cm.py)

Recommended new binary:

- `lambda/sweep_cm`

Recommended dispatch target:

- `sweep_cm`

Reason:

- isolates heavy LAPACK dependencies from the current fast path
- keeps Aberth deploys simple
- lets you tune memory/timeouts independently
- avoids contaminating the current static `sweep` build

## Why Not Put CM Inside The Existing `sweep` Binary

You could add a second mode inside `sweep_cli.c`, but it is not the best first step.

Problems:

- current `sweep` is built static with `aarch64-linux-musl-gcc`
- LAPACK / OpenBLAS packaging is much easier in a separate dynamically linked binary
- debugging and deployment become harder if both solvers share the same executable

Recommended rule:

- Aberth stays in `sweep`
- companion-matrix lives in `sweep_cm`

## LAPACK / BLAS Requirement

Yes, the companion-matrix path needs an eigensolver library in the Lambda runtime.

Practical choices:

1. bundle OpenBLAS/LAPACK in the Lambda zip
2. provide OpenBLAS/LAPACK via a dedicated Lambda layer

Recommended choice:

- dedicated `lapack` / `openblas` layer

Reason:

- cleaner packaging
- reusable across future numerical Lambdas
- smaller function zip
- easier iteration on the solver binary alone

## Recommended Runtime Stack

### Binary

Write a small C binary that:

1. reads the coeff chunk file
2. for each polynomial:
   - builds the companion matrix
   - computes eigenvalues
   - writes roots as `float32 re/im` pairs to output
3. writes compute metadata JSON to stdout

### LAPACK Backend

Use LAPACK eigenvalue routines:

- complex double path:
  - `zgeev`

Do not write your own QR / Schur algorithm.

That would be wasted effort and much less reliable.

## File-Level Plan

### Frontend

Change:

- [`index.html`](./index.html)

Add:

- a new `Calculate-CM` button
- a small shared compute runner with a solver mode argument

Suggested solver identifiers:

- `aberth`
- `companion_matrix`

### New Backend Files

Add:

- [`lambda/handler_sweep_cm.py`](./lambda/handler_sweep_cm.py)
- `lambda/sweep_cm.c`

Optional helper:

- `lambda/companion_solver.h`

### Existing Backend Files To Change

Change:

- [`lambda/handler_dispatch.py`](./lambda/handler_dispatch.py)
  - add target `sweep_cm`
- [`deploy.sh`](./deploy.sh)
  - compile/package/deploy new binary + handler
  - wire dispatch env var
- [`config.json`](./config.json) or generated frontend endpoint config
  - expose `sweep-cm` endpoint if endpoints are explicit

### Optional Tests

Add:

- [`tests/test_companion_matrix.py`](./tests/test_companion_matrix.py)
- extend [`lambda/test_pipeline.py`](./lambda/test_pipeline.py)

## UI Implementation

### Recommended Refactor

Right now `runCalculate()` owns the whole compute flow.

Refactor to:

```js
async function runCalculateWithSolver(solverMode) {
    // solverMode: 'aberth' | 'companion_matrix'
}

async function runCalculate() {
    return runCalculateWithSolver('aberth');
}

async function runCalculateCM() {
    return runCalculateWithSolver('companion_matrix');
}
```

Then branch only in the solve phases.

### Button Snippet

Suggested UI change:

```html
<button id="btn-calc" onclick="runCalculate()">Calculate</button>
<button id="btn-calc-cm" onclick="runCalculateCM()">Calculate-CM</button>
```

### Frontend Branching

For lores solve:

```js
const solveLambda = solverMode === 'companion_matrix' ? 'sweep-cm' : 'sweep';
```

For hires dispatch:

```js
const dispatchTarget = solverMode === 'companion_matrix' ? 'sweep_cm' : 'sweep';
```

Then use:

```js
const loresResult = await lambdaPost(solveLambda, {
    job_id: jobId,
    stripe_idx: 0,
    coeffs_key: loresCoeffResult.coeffs_key,
    n_coeffs: nCoeffs,
    n_steps: loresSteps,
    s3_key: loresKey,
});
```

and:

```js
const dispResult = await lambdaPost('dispatch', {
    target: dispatchTarget,
    jobs: batch,
    expected_keys: [],
});
```

### Metadata Snippet

Add to `calcMeta`:

```js
solver: solverMode,
```

Suggested final value:

- `'aberth'`
- `'companion_matrix'`

## Backend Handler Design

### `handler_sweep_cm.py`

This should match the structure of [`lambda/handler_sweep.py`](./lambda/handler_sweep.py).

It should:

1. parse the same input
2. download the coeff chunk from S3
3. run `sweep_cm`
4. upload the resulting root chunk
5. report status to DynamoDB
6. return the same result shape

That compatibility is important because:

- render code
- polling code
- metadata code

should not need to know which solver produced the chunk.

### Handler Skeleton

```python
import json
import os
import subprocess
import time

import boto3

from shared import BUCKET, parse_body, ok_response, report_status

s3 = boto3.client("s3")
SWEEP_CM = os.path.join(os.path.dirname(__file__), "sweep_cm")


def handler(event, context):
    params = parse_body(event)
    return handle_solve_from_coeffs_cm(params)


def handle_solve_from_coeffs_cm(params):
    job_id = params["job_id"]
    stripe_idx = params["stripe_idx"]
    coeffs_key = params["coeffs_key"]
    n_coeffs = params["n_coeffs"]
    n_steps = params["n_steps"]
    task_id = f"sweep_{stripe_idx}"

    try:
        report_status(job_id, task_id, "started")
        t0 = time.time()

        resp = s3.get_object(Bucket=BUCKET, Key=coeffs_key)
        coeffs_data = resp["Body"].read()

        coeffs_file = "/tmp/coeffs_chunk.bin"
        with open(coeffs_file, "wb") as f:
            f.write(coeffs_data)

        bin_path = "/tmp/roots_chunk.bin"
        spec = {
            "mode": "solve_cm",
            "coeffs_file": coeffs_file,
            "n_coeffs": n_coeffs,
            "n_steps": n_steps,
        }
        result = subprocess.run(
            [SWEEP_CM, bin_path],
            input=json.dumps(spec),
            capture_output=True,
            text=True,
            timeout=840,
        )
        if result.returncode != 0:
            raise RuntimeError(f"solve_cm failed: {result.stderr.strip()}")

        meta = json.loads(result.stdout)
        s3_key = params.get("s3_key", f"renders/{job_id}/stripe_{stripe_idx}.bin")
        bin_size = os.path.getsize(bin_path)
        with open(bin_path, "rb") as f:
            s3.upload_fileobj(f, BUCKET, s3_key)

        result_data = {
            "stripe_idx": stripe_idx,
            "s3_key": s3_key,
            "bin_size": bin_size,
            "compute_us": int((time.time() - t0) * 1e6),
            "n_t": meta["n_t"],
            "degree": meta["degree"],
            "avg_iterations": meta.get("avg_iterations", 0),
        }
        report_status(job_id, task_id, "done", result_data=result_data)
        return ok_response({**result_data, "n_procs": 1})

    except Exception as e:
        report_status(job_id, task_id, "error", str(e))
        raise
```

Important:

- keep `task_id = f"sweep_{stripe_idx}"` if you want zero downstream changes
- or use `sweepcm_{stripe_idx}` if you want explicit solver-aware status rows

Recommended first version:

- keep `sweep_{stripe_idx}`

because each compute job uses exactly one solver path.

## `sweep_cm.c` Design

### Input Contract

Suggested JSON input:

```json
{
  "mode": "solve_cm",
  "coeffs_file": "/tmp/coeffs_chunk.bin",
  "n_coeffs": 71,
  "n_steps": 25000
}
```

### Output Contract

Match current solve output:

- binary file written to the output path argument
- metadata JSON to stdout

Suggested metadata:

```json
{
  "mode": "solve_cm",
  "n_t": 25000,
  "degree": 70,
  "avg_iterations": 0
}
```

`avg_iterations` can be `0` or omitted.

If you omit it, update the caller.

Best compatibility:

- include it as `0`

## Companion Matrix Algorithm

### Coefficient Interpretation

If the coefficient vector is:

```text
a0 x^n + a1 x^(n-1) + ... + an
```

then:

1. trim leading zeros
2. if degree becomes:
   - `0`: no roots
   - `1`: direct linear solve
3. normalize to monic
4. build companion matrix
5. compute eigenvalues with LAPACK

### Companion Matrix Form

For monic polynomial:

```text
x^n + b1 x^(n-1) + b2 x^(n-2) + ... + bn
```

use:

```text
[ -b1  -b2  -b3  ... -bn ]
[  1    0    0   ...  0  ]
[  0    1    0   ...  0  ]
[  ...                   ]
[  0    0    0   ...  1 0]
```

Eigenvalues are the roots.

### LAPACK Routine

Use:

- `zgeev`

for complex double general matrix eigenvalues.

You do not need left or right eigenvectors.

Set:

- `JOBVL = 'N'`
- `JOBVR = 'N'`

## C Snippet For `zgeev`

Illustrative snippet:

```c
#include <complex.h>

extern void zgeev_(char *jobvl, char *jobvr, int *n,
                   double _Complex *a, int *lda,
                   double _Complex *w,
                   double _Complex *vl, int *ldvl,
                   double _Complex *vr, int *ldvr,
                   double _Complex *work, int *lwork,
                   double *rwork, int *info);
```

Workspace query pattern:

```c
char jobvl = 'N', jobvr = 'N';
int lda = n, ldvl = 1, ldvr = 1, info, lwork = -1;
double _Complex wkopt;
double *rwork = malloc(sizeof(double) * 2 * n);

zgeev_(&jobvl, &jobvr, &n,
       A, &lda,
       W,
       NULL, &ldvl,
       NULL, &ldvr,
       &wkopt, &lwork,
       rwork, &info);

lwork = (int)creal(wkopt);
double _Complex *work = malloc(sizeof(double _Complex) * lwork);

zgeev_(&jobvl, &jobvr, &n,
       A, &lda,
       W,
       NULL, &ldvl,
       NULL, &ldvr,
       work, &lwork,
       rwork, &info);
```

Then `W[i]` are the roots.

## Practical Numerical Guards

The CM solver should not try to be a symbolic CAS.

It should do a few pragmatic things:

### 1. Trim Leading Zero Coefficients

If top coefficients are zero or tiny:

- reduce degree before building the matrix

Suggested threshold:

```c
1e-15
```

relative to coefficient max magnitude.

### 2. Normalize Polynomial

Always divide by leading coefficient.

### 3. Handle Small Degrees Directly

Do not call LAPACK for:

- degree 0
- degree 1

Optional:

- degree 2 via analytic quadratic

Not required, but cheap.

### 4. Filter Non-Finite Outputs

If eigenvalues come back non-finite:

- zero them
- or skip them

For pipeline compatibility, zeroing is simpler.

## Packaging Strategy

## Recommended Build Layout

Add a dedicated build section in [`deploy.sh`](./deploy.sh):

1. build an ARM64 dynamic binary in Docker on Amazon Linux 2023
2. link against OpenBLAS/LAPACK from a dedicated layer build directory
3. package:
   - `handler_sweep_cm.py`
   - `shared.py`
   - `sweep_cm`

### Why Not Static Here

Your current `sweep` is built with:

```bash
aarch64-linux-musl-gcc -O3 -static -o lambda/sweep lambda/sweep_cli.c -lm
```

That is good for a pure math binary.

For LAPACK/OpenBLAS:

- dynamic linking is the practical route
- especially on Lambda ARM64

## Recommended New Layer

Suggested layer contents:

- `libopenblas.so`
- `liblapack.so` or LAPACK symbols provided by OpenBLAS build
- required Fortran runtime libs:
  - `libgfortran.so`
  - `libquadmath.so`
  - anything else `ldd` reports

Suggested env:

```bash
LD_LIBRARY_PATH=/opt/lib
```

This is the same pattern you already use for libvips-linked binaries.

## Build Snippet

Illustrative `deploy.sh` pattern:

```bash
CM_LAYER_BUILD="$SCRIPT_DIR/lambda/layer-build-lapack"
docker run --rm --platform linux/arm64 \
    -v "$SCRIPT_DIR/lambda:/src" \
    -v "$CM_LAYER_BUILD:/opt" \
    public.ecr.aws/amazonlinux/amazonlinux:2023 \
    bash -c '
        set -euo pipefail
        dnf install -y gcc 2>&1 | tail -1
        gcc -O3 -o /src/sweep_cm /src/sweep_cm.c \
            -L/opt/lib -llapack -lopenblas -lm \
            -Wl,-rpath,/opt/lib
        echo "  sweep_cm compiled: $(file /src/sweep_cm)"
        export LD_LIBRARY_PATH=/opt/lib
        /src/sweep_cm /tmp/out.bin < /src/testdata/cm_smoke.json
    '
```

This should be refined to match the actual library names in your layer.

## Deploy Changes

### New Names In `deploy.sh`

Add:

```bash
SWEEP_CM_NAME="polypaint-sweep-cm"
SWEEP_CM_MEMORY=4096
```

Maybe start with:

- `4096` or `6144` MB

Companion-matrix eigensolves will likely need more memory and CPU than Aberth.

### New Packaging Block

Package:

- `handler_sweep_cm.py`
- `shared.py`
- `sweep_cm`

### New Lambda Create/Update

Mirror the existing sweep Lambda creation, but attach the LAPACK layer instead of the libvips layer.

### Dispatch Env

Add to dispatch Lambda env:

```bash
SWEEP_CM_FUNCTION=$SWEEP_CM_NAME
```

Then in [`lambda/handler_dispatch.py`](./lambda/handler_dispatch.py):

```python
"sweep_cm": os.environ.get("SWEEP_CM_FUNCTION", "polypaint-sweep-cm"),
```

## Frontend Compute Refactor

### Recommended Shared Runner

Implement:

```js
async function runCalculateWithSolver(solverMode) {
    // existing runCalculate body
}
```

Then:

```js
async function runCalculate() {
    return runCalculateWithSolver('aberth');
}

async function runCalculateCM() {
    return runCalculateWithSolver('companion_matrix');
}
```

### Solve Branch Points

Only branch at:

- lores solve Lambda name
- hires dispatch target
- saved metadata solver field
- UI log text

Everything else should remain shared.

## Logging Recommendations

Use explicit logs so users know which solver ran.

Suggested compute log line:

```js
log(`Compute-${solverMode === 'companion_matrix' ? 'CM' : 'AE'}: ${pipelineStr} N=${n}, chunks=${nChunks}${times > 1 ? ', times=' + times : ''}...`);
```

Suggested solve log lines:

- `lores solve (CM): ...`
- `solve dispatch (CM): ...`
- `solve (CM): ...`

This will make side-by-side comparisons much easier.

## Output Compatibility Requirements

The CM path should write root chunks in the exact same binary format:

- interleaved `float32`
- `[re, im, re, im, ...]`

That keeps:

- viewport
- preview
- render
- bilevel
- export

unchanged.

## Testing Strategy

## 1. Unit Tests For Companion Matrix Builder

Add a narrow unit test around the CM solver helper:

- monic cubic
- non-monic cubic
- leading-zero trimming
- linear fallback

Compare roots via residuals, not ordering.

### Residual Helper

Use:

```python
def polyval(cf, z):
    y = 0j
    for a in cf:
        y = y * z + a
    return y
```

Accept:

- `max(abs(polyval(cf, r))) < tol`

## 2. Small End-To-End Binary Test

Create a small coeff file with a few known polynomials:

- quadratic with known roots
- cubic with known roots
- quartic with known roots

Run `sweep_cm` locally and assert:

- output file size is correct
- roots satisfy the polynomial

## 3. Lambda Handler Test

Add a handler test mirroring the existing sweep tests:

- mock S3 download/upload
- mock subprocess
- assert `report_status(..., result_data=...)`

## 4. UI/Dispatch Test

Add a frontend-level smoke test or at least a checklist ensuring:

- `Calculate` still uses Aberth
- `Calculate-CM` uses `sweep-cm`
- metadata contains `solver`

## 5. Visual Comparison Test

Use the same:

- parameter stream
- coeff files

Then solve both ways and compare:

- rendered previews
- viewport bounds
- residual distributions
- runtime

This is the real value of the feature.

## Comparison Metrics

For each chunk or job, compare:

- wall time
- max residual
- median residual
- count of non-finite roots
- visual overlap
- viewport consistency

Suggested metadata additions for debug mode:

```json
{
  "solver": "companion_matrix",
  "solver_stats": {
    "nonfinite_roots": 0,
    "max_residual": 1.2e-10
  }
}
```

Optional, not required for first implementation.

## Performance Warning

This path is for comparison and investigation first, not necessarily production scale.

Companion-matrix eigensolves are likely:

- slower
- heavier on memory
- worse on some very large degrees

So first implementation should assume:

- maybe smaller chunk size
- maybe higher memory
- maybe no auto-selection in production

Good defaults:

- keep standard `Calculate` as default
- expose `Calculate-CM` as explicit alternative

## Recommended First Scope

Do not try to replace Aberth.

Implement:

1. lores solve with CM
2. hires solve with CM
3. identical outputs/metadata shape
4. a visible solver label in results

Do not implement initially:

- hybrid solver selection
- fallback between solvers
- per-degree auto-routing
- CM as default

## Risks

### 1. Packaging Complexity

OpenBLAS/LAPACK on Lambda ARM64 is the main deployment risk.

Mitigation:

- use a dedicated layer
- test locally in Docker before deploy
- include an actual eigensolve smoke test in deploy

### 2. Cold Start / Binary Loading

Dynamic numerical libs will increase cold start and package complexity.

Mitigation:

- isolate them in a separate Lambda
- do not burden the existing sweep path

### 3. Runtime Cost

Companion matrix solve may be too slow for your highest-degree/highest-step jobs.

Mitigation:

- keep explicit button
- compare on selected jobs first

## Docker Smoke Test Recommendation

Before deploy, require a runtime smoke test that:

1. loads linked libs correctly
2. runs one known cubic or quartic
3. checks residuals

Example idea:

```bash
echo '{"mode":"solve_cm","coeffs_file":"/tmp/cf.bin","n_coeffs":4,"n_steps":1}' | /src/sweep_cm /tmp/out.bin
```

Then verify roots for:

```text
x^3 - 6x^2 + 11x - 6
```

which has roots:

- `1`
- `2`
- `3`

## Implementation Checklist

### Phase 1: UI

- add `Calculate-CM` button in [`index.html`](./index.html)
- refactor compute code into `runCalculateWithSolver(solverMode)`
- keep `runCalculate()` as the Aberth wrapper
- add `runCalculateCM()`
- branch lores solve Lambda name
- branch hires dispatch target
- write `solver` into `calcMeta`
- add solver label to logs/results if desired

### Phase 2: New Lambda

- add [`lambda/handler_sweep_cm.py`](./lambda/handler_sweep_cm.py)
- mirror the input/output contract of [`lambda/handler_sweep.py`](./lambda/handler_sweep.py)
- keep status reporting compatible
- keep root chunk format identical

### Phase 3: Solver Binary

- add `lambda/sweep_cm.c`
- parse `solve_cm` spec
- read coeff file sequentially
- trim leading zeros
- build companion matrix
- call `zgeev`
- write roots to output `.bin`
- emit metadata JSON

### Phase 4: Packaging

- create LAPACK/OpenBLAS ARM64 layer build
- compile `sweep_cm` dynamically against that layer
- add Docker smoke test
- package `polypaint-sweep-cm`
- deploy Lambda

### Phase 5: Dispatch

- add `sweep_cm` target to [`lambda/handler_dispatch.py`](./lambda/handler_dispatch.py)
- wire `SWEEP_CM_FUNCTION` env var in [`deploy.sh`](./deploy.sh)

### Phase 6: Tests

- add unit tests for companion matrix root extraction
- add end-to-end binary smoke tests
- add handler test for `handler_sweep_cm.py`
- add UI path smoke checklist
- compare AE vs CM on selected jobs

## Test Checklist

### Functional

- `Calculate` still works unchanged
- `Calculate-CM` completes a small job
- lores preview works from CM root output
- hires render works from CM root output
- metadata saves `solver: companion_matrix`

### Numerical

- cubic known roots pass
- quartic known roots pass
- non-monic polynomial passes
- leading-zero polynomial passes
- repeated-root polynomial does not explode

### Deployment

- Lambda can load all shared libs
- local Docker smoke test passes
- deployed Lambda completes one live small job

### Comparison

- same coeff chunk solved by AE and CM
- compare residuals
- compare wall times
- compare visual output

## Good First Demo Job

Start with:

- moderate degree
- small `N`
- small `times`
- low chunk count

Example:

- `N=50`
- `times=1`
- `chunks=10`
- degree around `20-40`

Do not start with:

- `N=10000`
- huge chunk count
- highest-degree functions

## Recommended Acceptance Criteria

The feature is ready when:

1. `Calculate-CM` runs end-to-end on a small job
2. outputs render through the existing render pipeline unchanged
3. metadata clearly records the solver
4. deploy has a real smoke test for the new binary
5. `Calculate` remains untouched and stable

## Final Recommendation

Build `Calculate-CM` as:

- same compute pipeline
- separate solve Lambda
- separate binary
- dedicated LAPACK/OpenBLAS layer

That is the cleanest, lowest-risk version.

It gives you a true apples-to-apples comparison:

- same params
- same coeffs
- same rasterization
- different root solver

which is exactly what you want.
