# `roots_cm` Coefficient Transform Plan

Status: implemented.

## Goal

Add a new coefficient transform:

- `roots_cm(hi|lo)`

Its behavior:

- take the current coefficient vector
- compute its roots using the **companion-matrix** method
- treat those roots as the next coefficient vector
- keep output length constant the same way `roots(k, hi|lo)` does now:
  - `hi`: prepend a zero highest-order coefficient, then write the roots
  - `lo`: write the roots first, then append a zero constant coefficient

This should behave like a normal coefficient transform in the existing pipeline:

```text
[parameter transforms] -> coefficient function -> [coefficient transforms]
```

The important constraint is:

- do **not** special-case this in Python orchestration
- do **not** split the coeff-transform chain in the handler
- keep coefficient-transform semantics inside the native coeffgen binary

## What We Are Not Doing

We are **not** going to:

- make `handler_coeffgen.py` inspect the coeff-transform chain and run custom multi-stage logic
- bounce between `sweep` and `sweep_cm` in Python
- add a “fake coeff transform” implemented outside the coeff-transform engine

That would break the current separation of concerns and make the handler own transform semantics.

## Current State

### Native coeff transforms live in `sweep_cli.c`

Current coefficient transforms are implemented in:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)

That includes:

- plain transforms like `rev`, `conj`, `deriv`
- parametric transforms like:
  - `roots(k)`
  - `power(k)`
  - `invpower(k)`

### Companion-matrix solving already exists, but in a separate binary

The existing companion-matrix implementation is here:

- [lambda/sweep_cm.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cm.c)

That binary:

- reads coefficient records from a file
- builds a companion matrix
- uses LAPACK `zgeev`
- writes roots in the normal packed root format

It is already deployed separately for the `Calculate-CM` solve path.

### The coeffgen Lambda does not currently have LAPACK

Current coeffgen packaging:

- `polypaint-coeffgen` ships `handler_coeffgen.py` + `shared.py` + `sweep`
- no LAPACK/OpenBLAS layer is attached there now

Current build:

- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh) builds `lambda/sweep` as a static musl binary:
  - `aarch64-linux-musl-gcc -O3 -static -o lambda/sweep lambda/sweep_cli.c -lm`

So `roots_cm` cannot simply be dropped into the current shipped coeffgen binary without changing how coeffgen is built and packaged.

## Intended Design

### 1. Keep `roots_cm` as a native coeff transform in `sweep_cli.c`

`roots_cm` should be added to the coefficient-transform dispatch in:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)

Expected shape:

- `ct_roots_cm(double *cRe, double *cIm, int *nCoeffs, int padLo)`
- one placement parameter: `hi` or `lo`
- same output length behavior as `roots(k, hi|lo)`

That keeps the transform model coherent:

- the handler still just passes `coeff_transforms`
- the native coeffgen binary still owns transform semantics

### 2. Share the companion solver code instead of duplicating it

The companion-matrix logic should not be copy-pasted into two unrelated files.

Recommended refactor:

- extract the reusable companion-matrix solve core from:
  - [lambda/sweep_cm.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cm.c)
- into a shared native module, for example:
  - `lambda/companion_solver.h`
  - or `lambda/companion_solver.c` + `lambda/companion_solver.h`

That shared code should expose something like:

- `solve_companion_coeffs(...)`

Then:

- `sweep_cm.c` keeps using that shared implementation for the CM solve mode
- `sweep_cli.c` uses the same implementation for `ct_roots_cm`

This preserves one CM solver implementation instead of two near-copies.

### 3. Add LAPACK/OpenBLAS to the coeffgen Lambda, not to Python control flow

The correct architectural move is:

- add LAPACK/OpenBLAS availability to the **coeffgen Lambda**
- not to Python orchestration

That means:

- the coeffgen native binary used by `polypaint-coeffgen` must be LAPACK-linked
- the coeffgen Lambda package must get the LAPACK layer

### 4. Do not disturb the current static `sweep` binary used by the AE solve path

The current `sweep` binary is used in multiple places and is currently a static musl binary.

To minimize blast radius:

- keep the current static `lambda/sweep` build for the existing AE solve path
- build a **separate coeffgen-native binary** for the coeffgen Lambda

Recommended naming in the repo:

- keep:
  - `lambda/sweep`
- add:
  - `lambda/sweep_coeffgen`

But inside the coeffgen Lambda package, it can still be copied in as:

- `sweep`

so [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py) does not need semantic changes.

This preserves separation while keeping deployment practical.

## Packaging Plan

### Build products

Keep current build:

- `lambda/sweep`
  - static
  - no LAPACK
  - used by existing AE solve Lambda path

Add new build:

- `lambda/sweep_coeffgen`
  - dynamic
  - linked against LAPACK/OpenBLAS
  - contains the same coeffgen modes as `sweep_cli.c`, plus `roots_cm`

Recommended build style:

- build in Docker/ARM64, similar to `sweep_cm`
- link against `/opt/lib` or an equivalent packaged runtime path

### Coeffgen Lambda package

Change coeffgen packaging in [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh):

- copy `lambda/sweep_coeffgen` into the coeffgen zip as `sweep`
- attach `LAPACK_LAYER` to `polypaint-coeffgen`

That way:

- the handler still invokes `sweep`
- but the coeffgen Lambda gets the LAPACK-backed build

### Sweep solve Lambda package

No change intended:

- keep using the existing static `lambda/sweep`
- do **not** add LAPACK to the main AE solve Lambda unless a separate need appears

## UI Plan

Add one new coefficient-transform option to the Compute tab:

- `roots_cm`

Properties:

- one `hi|lo` pad-mode arg
- array wire format:
  - `["roots_cm", "hi"]`
  - `["roots_cm", "lo"]`

This should appear beside the existing coeff transforms in:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

No popup or special UI treatment is needed.

## Behavior Plan

`roots_cm` should match the current `roots(k, hi|lo)` output shape:

- if input coeff vector has length `n`
- output coeff vector also has length `n`
- output is either:
  - `hi`: `0` followed by the computed roots
  - `lo`: computed roots followed by `0`
  - padded with zeros if needed

Important edge cases:

- all-zero polynomial
- leading zero coefficients
- trailing zero coefficients
- degree `0` / degree `1`
- overflow / non-finite normalized coefficients

The behavior should mirror the existing CM solver as closely as practical.

## Testing Plan

### 1. Frontend regression

Add a JS test in:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

Expected assertions:

- `roots_cm` appears in the coeff-transform dropdown
- it renders as a coeff chip with one `hi|lo` input
- it serializes as `["roots_cm", "hi"]` or `["roots_cm", "lo"]`

### 2. Native coeff-transform smoke

Add a roots-cm-specific native smoke test.

Because host `sweep_test` is currently built without LAPACK, do **not** force the whole normal host test flow to depend on LAPACK.

Recommended approach:

- add a dedicated CM-backed coeffgen smoke test path
- run it only when the LAPACK-backed coeffgen binary is available

The smoke case should verify that:

- `roots_cm` on a known coefficient family produces the same roots-as-coefficients result as the shared companion solver

### 3. Chunking parity

Add/extend chunking tests so:

- direct `coeffgen`
- and `param_gen -> coeffgen_chunked`

stay equivalent when `coeff_transforms` includes:

- `"roots_cm"`

### 4. Packaging/deploy test

Add deploy-side assertions that:

- coeffgen Lambda zip contains the LAPACK-backed binary
- coeffgen Lambda is attached to `LAPACK_LAYER`
- the main AE solve Lambda remains on the current static build unless intentionally changed

## Why This Is The Right Shape

This keeps the architecture clean:

- Python handler stays thin
- coeff-transform semantics remain native
- CM logic is shared, not duplicated ad hoc
- LAPACK is attached where the CM capability is actually needed
- existing AE solve deployment stays stable

It also avoids the bad alternative:

- no Python-level splitting of the coeff-transform chain
- no handler-owned interpretation of `roots_cm`
- no cross-binary semantic hacks

## Files Expected To Change

Native:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)
- [lambda/sweep_cm.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cm.c)
- new shared companion solver file(s)

Frontend:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Deploy:

- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)

Tests:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- roots-cm native smoke test
- chunking parity tests
- deploy packaging tests if needed

## Definition Of Done

`roots_cm` is done when:

- it appears in the Compute coeff-transform UI
- it runs as a native coeff transform, not a Python handler special case
- the coeffgen Lambda has LAPACK available
- direct and chunked coeffgen both support it
- it passes frontend, native smoke, and chunking parity tests
- no unrelated AE solve packaging path is destabilized
