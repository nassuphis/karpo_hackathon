# Companion Matrix Regression Tests

## Goal

Make the companion-matrix solver path a permanent, testable part of the app.

The tests should answer four questions:

1. Does the CM solver run at all in the packaged runtime?
2. Does it solve known polynomials correctly?
3. Does it produce output in the exact format the existing pipeline expects?
4. How does it compare to the existing Aberth-Ehrlich solver on the same input?

This should be permanent regression coverage, not a one-off bring-up exercise.

## Testing Strategy

Split the tests into two layers:

### Layer 1: Fast Functional Regression Tests

Run in normal local/CI test flow.

Purpose:

- verify solver correctness on small fixtures
- verify output format
- verify handler behavior
- compare AE vs CM on tiny inputs

These should be:

- deterministic
- quick
- not dependent on Docker
- not dependent on deployed AWS infrastructure

### Layer 2: Docker Runtime Regression Tests

Run in a Lambda-like ARM64 Docker environment before deploy.

Purpose:

- verify `sweep_cm` loads LAPACK/OpenBLAS from the layer correctly
- verify the packaged runtime behaves like the local development expectation
- catch missing shared libs, ABI mismatches, and packaging mistakes

These should be:

- required before deploy
- small and focused
- not huge visual sweeps

## What To Test Permanently

### A. Exact Small Polynomial Fixtures

The CM solver should be tested against known-root polynomials:

- linear
- quadratic
- cubic
- quartic

Example fixtures:

- `x - 3`
- `x^2 - 5x + 6`
- `x^3 - 6x^2 + 11x - 6`
- `x^4 - 10x^3 + 35x^2 - 50x + 24`

Expected roots:

- `3`
- `2, 3`
- `1, 2, 3`
- `1, 2, 3, 4`

These should be validated by residual, not by output order.

### B. Degenerate / Robustness Cases

These are mandatory because CM implementations often fail here:

- all-zero polynomial
- leading-zero polynomial
- nearly-leading-zero polynomial
- repeated-root polynomial
- constant polynomial

Example cases:

- `[0, 0, 0, 0]`
- `[0, 1, -3, 2]`
- `[1, -2, 1]` for repeated root `1,1`
- `[5]` or constant-only record if representable at this layer

### C. AE vs CM Comparison Fixtures

Feed the same coefficient file to both:

- `sweep`
- `sweep_cm`

Compare:

- root count
- non-finite root count
- polynomial residuals
- rough root-set similarity

Do not compare by raw root ordering.

### D. Handler Contract Tests

The CM handler should be tested for:

- same input shape as `handler_sweep.py`
- same output shape as `handler_sweep.py`
- correct DynamoDB status reporting
- correct S3 upload behavior

### E. Runtime Linkage

Before deploy, verify:

- `sweep_cm` starts
- LAPACK/OpenBLAS symbols resolve
- a known solve works inside the Docker runtime

This is the runtime gate that catches packaging breakage.

## What The Tests Should Not Do

Do not make these the primary regression checks:

- large visual renders
- full app end-to-end AWS runs
- exact root-order equality
- giant randomized sweeps on every commit

Those are too slow or too brittle for the core solver suite.

## Comparison Oracles

## 1. Residual-Based Correctness

For each root `r`, evaluate:

```python
def polyval(cf, z):
    y = 0j
    for a in cf:
        y = y * z + a
    return y
```

The main solver correctness criterion should be:

- `abs(polyval(cf, r)) < tol`

Suggested tolerances:

- exact small fixtures: `1e-9` to `1e-7`
- AE/CM comparison fixtures: `1e-6` or similar

Use the same tolerance family across the suite.

## 2. Finiteness

Assert:

- no NaNs
- no infinities

This should be an explicit check.

## 3. Root Count

Assert:

- the output record length matches the expected binary format
- the number of roots written matches the polynomial degree expected by the pipeline

The app downstream expects:

- one coefficient vector of length `n_coeffs`
- one root vector of length `degree = n_coeffs - 1`
- output stored as interleaved `float32 re/im`

## 4. Root-Set Agreement

For AE vs CM comparison, use one of:

- nearest-neighbor matching
- sorting by `(re, im)`
- sorting by modulus, then angle

But only after checking residuals.

Residuals matter more than order.

## Recommended Test Files

### 1. Python-Level Math Tests

Add:

- [`tests/test_companion_matrix.py`](./tests/test_companion_matrix.py)

Purpose:

- fixture generation
- subprocess calls to `lambda/sweep_cm`
- residual checks
- AE vs CM comparison on small coefficient files

### 2. Handler Tests

Extend:

- [`lambda/test_pipeline.py`](./lambda/test_pipeline.py)

Add:

- handler tests for [`lambda/handler_sweep_cm.py`](./lambda/handler_sweep_cm.py)

### 3. Docker Runtime Test Script

Add:

- [`scripts/test-cm-runtime.sh`](./scripts/test-cm-runtime.sh)

or

- [`scripts/test_cm_runtime.py`](./scripts/test_cm_runtime.py)

Purpose:

- run the compiled binary inside ARM64 Docker
- verify linked libs
- run one or two known solves

## Suggested Fast Test Cases

## Test 1: Exact Cubic

Polynomial:

```text
x^3 - 6x^2 + 11x - 6
```

Coefficients:

```python
[1, -6, 11, -6]
```

Checks:

- 3 finite roots
- residuals small
- roots close to `1, 2, 3`

## Test 2: Exact Quartic

Polynomial:

```text
(x-1)(x-2)(x-3)(x-4)
```

Coefficients:

```python
[1, -10, 35, -50, 24]
```

Checks:

- 4 finite roots
- residuals small
- roots close to `1, 2, 3, 4`

## Test 3: Repeated Root

Polynomial:

```text
(x-1)^2 = x^2 - 2x + 1
```

Checks:

- 2 finite roots
- residuals small
- not necessarily exact duplicate ordering, but both near `1`

## Test 4: Leading Zero Trim

Polynomial input record:

```python
[0, 1, -3, 2]
```

This should reduce to:

```text
x^2 - 3x + 2
```

Checks:

- no crash
- roots near `1, 2`

## Test 5: All-Zero Polynomial

Input:

```python
[0, 0, 0, 0]
```

Checks:

- no crash
- output finite
- output zero-filled or otherwise safe for the pipeline

## Test 6: AE vs CM On Same Cubic Batch

Build a tiny coeff file with several simple polynomials:

- cubic
- quartic
- repeated root
- one complex-root quadratic

Run both solvers and compare:

- output sizes
- finiteness
- residual distributions

## Recommended Python Test Helpers

### Coeff File Writer

```python
import struct

def write_coeff_file(path, polys):
    with open(path, "wb") as f:
        for cf in polys:
            for c in cf:
                f.write(struct.pack("<ff", float(c.real), float(c.imag)))
```

### Root File Reader

```python
import struct

def read_root_file(path):
    out = []
    with open(path, "rb") as f:
        data = f.read()
    vals = struct.unpack("<" + "f" * (len(data) // 4), data)
    for i in range(0, len(vals), 2):
        out.append(complex(vals[i], vals[i + 1]))
    return out
```

### Residual Helper

```python
def polyval(cf, z):
    y = 0j
    for a in cf:
        y = y * z + a
    return y

def max_residual(cf, roots):
    return max(abs(polyval(cf, r)) for r in roots) if roots else 0.0
```

### Root Matching Helper

```python
def sort_roots(roots):
    return sorted(roots, key=lambda z: (round(z.real, 8), round(z.imag, 8)))
```

Use this only for human-readable comparison, not as the primary oracle.

## Suggested `tests/test_companion_matrix.py`

Example structure:

```python
import json
import os
import struct
import subprocess
import tempfile

ROOT = os.path.dirname(os.path.dirname(__file__))
SWEEP = os.path.join(ROOT, "lambda", "sweep")
SWEEP_CM = os.path.join(ROOT, "lambda", "sweep_cm")


def write_coeff_file(path, polys):
    with open(path, "wb") as f:
        for cf in polys:
            for c in cf:
                f.write(struct.pack("<ff", float(c.real), float(c.imag)))


def read_roots(path):
    with open(path, "rb") as f:
        data = f.read()
    vals = struct.unpack("<" + "f" * (len(data) // 4), data)
    return [complex(vals[i], vals[i + 1]) for i in range(0, len(vals), 2)]


def polyval(cf, z):
    y = 0j
    for a in cf:
        y = y * z + a
    return y


def run_cm(coeffs, n_coeffs):
    with tempfile.TemporaryDirectory() as td:
        coeff_path = os.path.join(td, "cf.bin")
        out_path = os.path.join(td, "roots.bin")
        write_coeff_file(coeff_path, [coeffs])
        spec = {
            "mode": "solve_cm",
            "coeffs_file": coeff_path,
            "n_coeffs": n_coeffs,
            "n_steps": 1,
        }
        subprocess.run(
            [SWEEP_CM, out_path],
            input=json.dumps(spec),
            text=True,
            check=True,
            capture_output=True,
        )
        return read_roots(out_path)


def test_exact_cubic():
    cf = [1+0j, -6+0j, 11+0j, -6+0j]
    roots = run_cm(cf, 4)
    roots = roots[:3]
    assert all(abs(polyval(cf, r)) < 1e-5 for r in roots)
```

This is only a starting shape.

## Suggested Handler Test

In [`lambda/test_pipeline.py`](./lambda/test_pipeline.py), add:

- `TestSweepCMHandler`

Checks:

- handler downloads coeff chunk
- subprocess is invoked with `sweep_cm`
- output is uploaded
- DynamoDB `report_status(..., "done", result_data=...)` shape matches `handler_sweep.py`

Mock:

- S3 get/upload
- `subprocess.run`

## Docker Runtime Test

## Purpose

Catch failures like:

- missing `liblapack.so`
- missing `libopenblas.so`
- missing `libgfortran.so`
- wrong architecture
- wrong ABI
- `zgeev_` unresolved

## Suggested Script

Add:

- [`scripts/test-cm-runtime.sh`](./scripts/test-cm-runtime.sh)

Suggested contents:

```bash
#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LAPACK_BUILD="$ROOT/lambda/layer-build-lapack"

docker run --rm --platform linux/arm64 \
  -v "$ROOT/lambda:/src" \
  -v "$LAPACK_BUILD:/opt" \
  public.ecr.aws/amazonlinux/amazonlinux:2023 \
  bash -c '
    set -euo pipefail
    export LD_LIBRARY_PATH=/opt/lib
    ldd /src/sweep_cm
    python3 - <<PY
import struct
data = b""
for c in [1, -6, 11, -6]:
    data += struct.pack("<ff", float(c), 0.0)
with open("/tmp/test_cf.bin", "wb") as f:
    f.write(data)
PY
    echo "{\"mode\":\"solve_cm\",\"coeffs_file\":\"/tmp/test_cf.bin\",\"n_coeffs\":4,\"n_steps\":1}" | /src/sweep_cm /tmp/out.bin
    test -f /tmp/out.bin
    test "$(stat -c%s /tmp/out.bin)" -eq 24
  '
```

Adjust `stat` syntax if needed for the host environment.

## Deploy Gate Recommendation

Make the Docker runtime test a required predeploy gate.

Meaning:

- if runtime linkage fails, deploy fails
- if the cubic smoke solve fails, deploy fails

This should live next to the existing runtime smoke tests already present in [`deploy.sh`](./deploy.sh).

## AE vs CM Comparison Regression

Add one permanent small-batch comparison test.

### Goal

Not to prove they are identical.

Goal:

- prove both are sane on the same coefficients
- catch catastrophic divergence
- catch output format mismatches

### Suggested Batch

Use 4 to 8 tiny polynomials:

- exact cubic
- exact quartic
- repeated-root quadratic
- one complex quadratic such as `x^2 + 1`
- one trimmed-leading-zero case

### Comparison Checks

For both AE and CM:

- all roots finite
- max residual below threshold

Optional:

- compare median residual
- compare nearest-neighbor distance between root sets

If CM residuals go off a cliff after a code change, that is a real regression.

## Binary Format Regression

Add an explicit output-size check:

If input has:

- `n_steps`
- `n_coeffs`

then output size must be:

```text
n_steps * (n_coeffs - 1) * 2 * 4
```

This should be a permanent regression check.

It ensures compatibility with:

- preview
- render
- raster
- finalize

## Recommended Thresholds

Suggested first thresholds:

- exact fixtures:
  - `max residual < 1e-6`
- comparison batch:
  - `max residual < 1e-5`
- all roots finite:
  - required

If needed, loosen carefully based on observed numeric behavior.

Do not make the thresholds so loose they stop catching real regressions.

## CI / Workflow Recommendation

### Every Normal Test Run

Run:

- Python fixture tests
- handler tests

### Before Deploy

Run:

- Docker runtime smoke test
- one AE vs CM comparison smoke test

### Optional Nightly

Run:

- a slightly larger AE vs CM batch
- maybe one tiny visual comparison

## Checklist

### Phase 1: Fast Regression Tests

- add [`tests/test_companion_matrix.py`](./tests/test_companion_matrix.py)
- add exact cubic test
- add exact quartic test
- add repeated-root test
- add leading-zero test
- add all-zero test
- add output-size test
- add AE vs CM batch test

### Phase 2: Handler Tests

- extend [`lambda/test_pipeline.py`](./lambda/test_pipeline.py)
- add `handler_sweep_cm.py` success test
- add `handler_sweep_cm.py` subprocess-failure test
- add status/result_data shape assertion

### Phase 3: Docker Runtime Test

- add [`scripts/test-cm-runtime.sh`](./scripts/test-cm-runtime.sh)
- verify `ldd` resolves all runtime libs
- verify one exact cubic solve
- verify output file size

### Phase 4: Deploy Gate

- call the runtime script from [`deploy.sh`](./deploy.sh)
- fail deploy if it fails

## Acceptance Criteria

The CM regression suite is good enough when:

1. exact fixture tests pass locally
2. handler tests pass locally
3. Docker runtime smoke test passes
4. one AE vs CM comparison batch passes
5. deploy fails automatically if the Docker CM runtime test fails

## Final Recommendation

Yes, these should be permanent regression tests.

The right shape is:

- fast math/format tests in the normal suite
- Docker runtime compatibility test as a predeploy gate

That gives you real protection against:

- solver logic regressions
- binary format regressions
- packaging/runtime regressions

which are exactly the risks in a separate companion-matrix Lambda path.
