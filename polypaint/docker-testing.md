# Docker / Graviton-Native Testing

## Rule

All runtime-relevant tests for the Polypaint Lambda binaries must run against Linux ARM64 binaries in Docker.

Do not treat host-native binaries as valid substitutes.

In this repo that means:

- valid runtime binaries:
  - [`lambda/sweep`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep)
  - [`lambda/sweep_cm`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cm)
  - other Lambda binaries compiled for Linux ARM64

- invalid runtime substitutes:
  - [`lambda/sweep_test`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_test) if it is a host/macOS binary
  - any `Mach-O`
  - any x86_64 Linux binary when the deployed target is ARM64

The deployed runtime is:

- Linux
- ARM64
- Lambda-like filesystem/runtime assumptions

So the test runtime must match that.

## Why This Matters

Host-native testing can hide exactly the kinds of bugs that matter in Lambda:

- missing shared libraries
- wrong dynamic linker
- wrong architecture
- ABI mismatches
- LAPACK/OpenBLAS/Fortran runtime issues
- runtime behavior differences between host and Lambda-like environment

A test that passes on a macOS binary but is skipped or broken in Docker is not a valid runtime regression test.

## Required Testing Policy

### 1. Runtime Solver Tests Must Use Linux ARM64 Binaries

Any test that claims to validate:

- `sweep`
- `sweep_cm`
- binary output format
- AE vs CM comparison
- Lambda-packaged runtime behavior

must use:

- Linux ARM64 binaries
- inside Docker

This is mandatory.

### 2. Host Tests Are Secondary

Host-native tests may still exist for:

- fast development iteration
- pure math helpers
- parser utilities
- UI logic

But host tests must not be the authoritative runtime gate for Lambda binaries.

### 3. Docker Is The Source Of Truth For Binary Regression

For all compiled Lambda executables, the authoritative regression environment is:

- `public.ecr.aws/amazonlinux/amazonlinux:2023`
- `--platform linux/arm64`

with:

- the same mounted layer libs
- the same `LD_LIBRARY_PATH`
- the same binary artifacts that deploy uses

## Binary Classes

## Class A: Deploy Binaries

These are the binaries that matter most:

- `lambda/sweep`
- `lambda/sweep_cm`
- `lambda/roots2pix`
- `lambda/pixassemble`
- `lambda/bilevel_raster`
- `lambda/coeffs_bilevel_raster`
- `lambda/raw2jpeg`
- `lambda/dz_export`
- `lambda/png_export`
- `lambda/tiff_compat`

These should be tested in Docker if the test is about runtime correctness.

## Class B: Host Convenience Binaries

Examples:

- `lambda/sweep_test`
- other local convenience builds

These may be useful for quick iteration, but:

- they are not deployment artifacts
- they are not runtime truth
- they must not be used as the main oracle for Lambda regression

## Prohibited Patterns

These should be treated as incorrect:

### Pattern 1: AE vs CM Comparison Using `sweep_test`

Wrong:

```python
SWEEP_AE = os.path.join(LAMBDA_DIR, "sweep_test")
```

for a runtime regression test.

Reason:

- `sweep_test` may be a `Mach-O` host binary
- `sweep_cm` is Linux ARM64
- that comparison mixes two different runtimes

### Pattern 2: “Skipped Because Wrong Platform” Counts As Passing

Wrong:

- Docker runtime test reports AE comparison skipped
- overall test suite still treated as valid runtime coverage

Reason:

- that means the actual runtime comparison never happened

### Pattern 3: Host Binary Mounted Into Docker As Comparison Oracle

Wrong:

- mount a host-compiled Mach-O into Docker
- try to execute it there
- skip when it fails

Reason:

- this is not a real runtime validation

## Required Replacement

For AE vs CM runtime comparison:

- AE binary must be [`lambda/sweep`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep)
- CM binary must be [`lambda/sweep_cm`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cm)

Both must be:

- Linux ARM64
- built by the same build process used for deploy

## Canonical Runtime Environment

Use:

```bash
docker run --rm --platform linux/arm64 \
  -v "$ROOT/lambda:/src" \
  public.ecr.aws/amazonlinux/amazonlinux:2023
```

For libvips-linked binaries or LAPACK-linked binaries, also mount the relevant layer build:

```bash
-v "$ROOT/lambda/layer-build:/opt"
```

or:

```bash
-v "$ROOT/lambda/layer-build-lapack:/opt"
```

And set:

```bash
export LD_LIBRARY_PATH=/opt/lib
```

## Binary Validation Rule

Before any runtime test, explicitly validate the binary type.

Required commands:

```bash
file lambda/sweep
file lambda/sweep_cm
```

Expected:

- `ELF 64-bit`
- `ARM aarch64`

Unexpected:

- `Mach-O`
- `x86_64`

If a binary is not Linux ARM64, the test should fail immediately.

Do not silently skip.

## Required Test Layers

## Layer 1: Docker Binary Smoke Tests

These are the minimum required runtime tests.

### Sweep Smoke

Verify:

- `lambda/sweep` runs in Docker
- solves one tiny known polynomial

### Sweep-CM Smoke

Verify:

- `lambda/sweep_cm` runs in Docker
- LAPACK/OpenBLAS load correctly
- solves one tiny known polynomial

### AE vs CM Runtime Comparison

Verify:

- both binaries run in Docker
- both solve the same tiny coeff file
- both produce finite roots
- both produce small residuals

This should be a required predeploy test.

## Layer 2: Docker Batch Regression Tests

These can still be small, but should cover more cases:

- exact cubic
- exact quartic
- repeated root
- leading-zero case
- all-zero case
- complex quadratic

Run both:

- `sweep`
- `sweep_cm`

and compare:

- output size
- finiteness
- residuals

## Layer 3: Optional Host Tests

Host tests may exist for speed, but they must be clearly labeled as:

- development convenience
- not deploy/runtime authority

If they use `sweep_test`, they should say so explicitly.

## Required Repo Changes

## 1. Stop Using `sweep_test` For CM Runtime Regression

Any CM regression test that is supposed to validate runtime behavior must stop using:

- `lambda/sweep_test`

and use:

- [`lambda/sweep`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep)

instead.

## 2. Add A Dedicated Docker Runtime Test Script

Add a script like:

- [`scripts/test-docker-runtime.sh`](./scripts/test-docker-runtime.sh)

or:

- [`scripts/test_docker_runtime.py`](./scripts/test_docker_runtime.py)

This should be the canonical runtime gate.

## 3. Make The Script Fail Hard On Wrong Binary Type

Do:

```bash
file /src/sweep | grep 'ELF 64-bit.*ARM aarch64'
file /src/sweep_cm | grep 'ELF 64-bit.*ARM aarch64'
```

If this fails:

- exit non-zero

Do not:

- skip
- warn only

## 4. Make AE vs CM Comparison Mandatory In Docker

If the test script cannot run AE in Docker:

- that is a failure

not a skip.

## Required Docker Test Script

Recommended script:

- [`scripts/test-docker-runtime.sh`](./scripts/test-docker-runtime.sh)

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

    file /src/sweep | grep "ELF 64-bit.*ARM aarch64"
    file /src/sweep_cm | grep "ELF 64-bit.*ARM aarch64"

    python3 - <<PY
import json, struct

def write_cf(path, coeffs):
    with open(path, "wb") as f:
        for c in coeffs:
            f.write(struct.pack("<ff", float(c), 0.0))

write_cf("/tmp/cubic.bin", [1, -6, 11, -6])

with open("/tmp/spec_ae.json", "w") as f:
    json.dump({
        "mode": "solve",
        "coeffs_file": "/tmp/cubic.bin",
        "n_coeffs": 4,
        "n2": 1,
        "i1_start": 0,
        "i1_end": 1,
        "match_roots": False
    }, f)

with open("/tmp/spec_cm.json", "w") as f:
    json.dump({
        "mode": "solve_cm",
        "coeffs_file": "/tmp/cubic.bin",
        "n_coeffs": 4,
        "n_steps": 1
    }, f)
PY

    /src/sweep /tmp/ae.bin < /tmp/spec_ae.json >/tmp/ae_meta.json
    /src/sweep_cm /tmp/cm.bin < /tmp/spec_cm.json >/tmp/cm_meta.json

    test -f /tmp/ae.bin
    test -f /tmp/cm.bin
    test "$(stat -c%s /tmp/ae.bin)" -eq 24
    test "$(stat -c%s /tmp/cm.bin)" -eq 24
  '
```

This is the minimum shape.

It should later be extended with residual checks, not just file-size checks.

## Recommended Python Docker Comparison Script

For deeper comparison, add:

- [`scripts/test_docker_ae_vs_cm.py`](./scripts/test_docker_ae_vs_cm.py)

Purpose:

- read both root files
- evaluate residuals
- fail if either solver produces bad output

Suggested core logic:

```python
import struct

def read_roots(path):
    with open(path, "rb") as f:
        data = f.read()
    vals = struct.unpack("<" + "f" * (len(data) // 4), data)
    return [complex(vals[i], vals[i+1]) for i in range(0, len(vals), 2)]

def polyval(cf, z):
    y = 0j
    for a in cf:
        y = y * z + a
    return y
```

Then assert:

- all roots finite
- residuals small for both AE and CM

## Testing Rules By Category

## Runtime Solver Tests

Must be Docker ARM64.

Examples:

- AE vs CM comparison
- CM linkage test
- output compatibility test

## Handler Tests

May be host-native Python tests if subprocesses are mocked.

Examples:

- `handler_sweep_cm.py` reports correct status
- uploads output correctly

These are logic tests, not runtime binary tests.

## Frontend Tests

May be host-native.

Examples:

- `Calculate-CM` chooses the right endpoint
- metadata writes `solver: companion_matrix`

These are not binary runtime tests.

## Acceptance Rules

A CM runtime test suite is acceptable only if:

1. AE binary used in Docker is [`lambda/sweep`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep)
2. CM binary used in Docker is [`lambda/sweep_cm`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cm)
3. both are verified as Linux ARM64 ELF binaries
4. no “wrong platform” skip is used to greenlight runtime coverage
5. deploy fails if the Docker runtime comparison fails

## Required Cleanup

The following distinction must be enforced in docs and code:

- `sweep_test`
  - host convenience binary
  - not deployment truth

- `sweep`
  - deployed AE solver binary
  - runtime truth

- `sweep_cm`
  - deployed CM solver binary
  - runtime truth

Any runtime doc or regression test that blurs those roles should be corrected.

## Checklist

### Phase 1: Fix Existing CM Tests

- change CM AE comparison tests to use [`lambda/sweep`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep)
- remove `sweep_test` from Docker runtime regression usage
- fail instead of skip if AE runtime comparison cannot run in Docker

### Phase 2: Add Docker Runtime Gate

- add `scripts/test-docker-runtime.sh`
- verify `file` output for both binaries
- run AE cubic solve
- run CM cubic solve
- verify output sizes

### Phase 3: Add Residual Comparison

- add Docker-side Python residual checker
- compare AE and CM on exact cubic and quartic fixtures
- fail on non-finite or high residuals

### Phase 4: Wire Into Deploy

- call Docker runtime test from [`deploy.sh`](./deploy.sh)
- fail deploy on any runtime mismatch

## Final Rule

If a test is about deployed binary behavior, it must be Graviton-native in Docker.

No exceptions.
