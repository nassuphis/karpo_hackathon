# Chunking Refactor

Status: legacy archive. Historical refactor note preserved for reference; remaining `stripe_*` mentions reflect migration-era terminology, not the preferred current naming.

## Problem

The current pipeline treats `stripes` as row bands only.

Today, coeffgen effectively does:

1. loop over `pass in 0..times-1`
2. loop over `i1` rows in one stripe
3. loop over `i2`
4. apply param transforms inline
5. produce coefficients inline

That creates an artificial limitation:

- `stripes > N` is awkward or broken
- small-`N`, high-`times` jobs cannot gain useful extra parallelism

Example:

- `N=10`
- `times=10000`
- `stripes=100`

This should be valid. There is no mathematical reason to cap stripes at `N`. The problem is the current work decomposition.

## Principle

Do not add a merge stage to preserve a flawed abstraction.

The flaw is:

- the system currently treats row stripes as the canonical unit of work

The fix is:

- make the canonical unit of work a linear chunk of fully materialized parameter inputs

## New Design

### Canonical Intermediate: `param.bin`

Add a cheap precompute stage that generates the full unrolled parameter stream for the logical traversal order.

Each record stores:

- `t1.re`
- `t1.im`
- `t2.re`
- `t2.im`

So one record is 4 floats = 16 bytes.

This file represents the full expanded parameter loop:

- size = `N * N * times`
- order must match the logical compute traversal exactly

Suggested file:

- `renders/{job_id}/params.bin`

### Canonical Work Unit: Chunk

From that point on, `stripes` should really mean:

- number of chunks

Each chunk is a contiguous range of step indices into `param.bin`:

- `step_start`
- `step_count`

Suggested chunk payload:

```json
{
  "job_id": "compute_xxx",
  "chunk_idx": 17,
  "step_start": 170000,
  "step_count": 10000,
  "N": 10,
  "times": 10000,
  "params_key": "renders/compute_xxx/params.bin",
  "function": "g39",
  "coeff_transforms": ["rev", ["roots", "6"]]
}
```

`coeff_transforms` can now mix plain string entries and parametric array
entries.

No row-range fields are needed for coeffgen anymore.

## Required Traversal Order

`param.bin` must be written in the exact same logical order the current coeffgen loop implies.

Recommended order:

1. pass-major
2. within each pass, `i1` ascending
3. within each row, serpentine `i2`

This matters because:

- coefficient output order must stay stable
- solve currently consumes coefficient vectors sequentially
- chunking only works cleanly if every chunk is a contiguous slice of one canonical order

## Why This Fixes The Real Problem

With `param.bin`:

- param transforms become a cheap one-time expansion stage
- coeffgen becomes a pure map:
  - read parameter tuples
  - run coefficient function
  - apply coefficient transforms
  - write coefficient tuples
- chunk count is independent of `N`

That means:

- `N=10, times=10000, stripes=100` works naturally
- no fake `stripes <= N` limit
- no need to split rows and passes separately in coeffgen
- no need to merge coefficient files back into row stripes

## Downstream Model

Downstream should not care about `N` as a sharding concept.

Downstream should care about:

- chunk count
- chunk ordering
- per-chunk artifact keys

`N` still matters for interpretation of the logical space, but not for how work is partitioned.

So:

- coeffgen should emit one coeff file per chunk
- solve should read one coeff file per chunk
- raster should rasterize one root file per chunk
- finalize should combine chunk outputs across `n_chunks`, not `n_stripes`

## Pipeline

### Stage 0: Param Generation

One Lambda:

- reads `N`
- reads `times`
- reads `param_transforms`
- generates `params.bin`
- uploads it to S3

This stage is intentionally not fanned out.

Reason:

- generating transformed `(t1, t2)` tuples is cheap
- the expensive stages are coeffgen and solve

### Stage 1: Chunked Coeffgen

Many Lambdas:

- each coeffgen worker range-reads its slice of `params.bin`
- computes coefficients for each tuple
- applies coefficient transforms
- writes `coeffs_chunk_{chunk_idx:04d}.bin`

### Stage 2: Chunked Solve

Many Lambdas:

- each solve worker reads one coeff chunk
- solves sequentially through that chunk
- writes `roots_chunk_{chunk_idx:04d}.bin`

Warm-start continuity is preserved within a chunk.
Cross-chunk continuity does not exist, but that is already true today across stripes.

### Stage 3: Chunked Raster

Many Lambdas:

- each raster worker reads one roots chunk
- emits tile-bucketed pix files tagged by chunk index

### Stage 4: Finalize / Merge Tiles

Finalize should read all chunk contributions for each tile:

- loop over `n_chunks`
- read `pix_{chunk_idx}_...`
- combine into final tiles/images

The current finalize logic already aggregates many independent shard outputs. It just needs to stop assuming those shards are row stripes.

## File Layout

Suggested S3 objects:

- `renders/{job_id}/params.bin`
- `renders/{job_id}/coeffs_chunk_0000.bin`
- `renders/{job_id}/coeffs_chunk_0001.bin`
- `renders/{job_id}/roots_chunk_0000.bin`
- `renders/{job_id}/roots_chunk_0001.bin`
- `renders/{job_id}/pix_c0000_t0000.pix`
- `renders/{job_id}/pix_c0001_t0000.pix`

Use chunk-centric naming consistently.

## Size Consideration

`param.bin` size is:

- `16 * N * N * times` bytes

Examples:

- `N=10, times=10000` -> about 16 MB
- `N=100, times=10000` -> about 1.6 GB
- `N=5000, times=100` -> about 40 GB

So this design is correct, but storage strategy matters.

Recommendation:

- do not rely on `/tmp` for the whole file
- generate `param.bin` as a streaming upload to S3
- coeffgen workers should use range reads from S3

## Backend Changes

### 1. New Param-Gen Lambda

Add a dedicated Lambda, for example:

- `handler_param_gen.py`

Responsibilities:

- generate the canonical `(t1, t2)` stream
- apply param transforms here, not in coeffgen
- preserve exact traversal order
- write `params.bin`
- return:
  - `params_key`
  - `n_steps`
  - maybe `record_bytes=16`

### 2. Coeffgen Handler

Update [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py) to accept:

- `params_key`
- `step_start`
- `step_count`
- `chunk_idx`

It should:

- range-read its param slice
- compute coefficient vectors from those params
- write one coeff chunk

Coeffgen should stop redoing param transforms itself.

### 3. Sweep / Coeffgen CLI

Update [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c) coeffgen mode so it can run from a parameter stream instead of regenerating the grid internally.

Two implementation options:

1. add a new mode
   - `coeffgen_from_params`
2. extend current coeffgen mode with:
   - `params_file`
   - `step_count`

Recommendation:

- use a new explicit mode

That keeps the old mode intact during migration.

### 4. Solve Handler

Update [lambda/handler_sweep.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_sweep.py) solve path to be chunk-native:

- one solve task per coeff chunk
- output `roots_chunk_XXXX.bin`

No row-band assumptions should remain in orchestration.

### 5. Raster / Finalize

Update:

- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
- [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)
- bilevel equivalents if needed

Required changes:

- rename/interpret shard index as `chunk_idx`
- finalize loops over `n_chunks`, not `n_stripes`
- pix naming should use chunk index

## Dither / RNG

In the new design, dither randomness belongs in param generation, because param generation is now the canonical source of transformed inputs.

That means:

- `ddith`
- `sdith`
- `ndith`

must be fully resolved during `params.bin` generation.

Required rule:

- for a fixed job spec, `params.bin` must be deterministic

So RNG seeding in param generation must depend on logical step identity, not execution chunking.

Good seed inputs:

- `pass`
- `i1`
- `i2`
- job-local fixed base seed

Bad seed inputs:

- chunk index alone
- local loop index inside a chunk

## Metadata

`calc.json` should move to chunk-centric metadata.

Suggested fields:

```json
{
  "job_id": "compute_xxx",
  "N": 10,
  "times": 10000,
  "n_steps": 1000000,
  "n_chunks": 100,
  "params_key": "renders/compute_xxx/params.bin",
  "coeffs_keys": [
    "renders/compute_xxx/coeffs_chunk_0000.bin"
  ],
  "root_keys": [
    "renders/compute_xxx/roots_chunk_0000.bin"
  ],
  "chunks": [
    {
      "chunk_idx": 0,
      "step_start": 0,
      "step_count": 10000,
      "coeffs_key": "renders/compute_xxx/coeffs_chunk_0000.bin",
      "root_key": "renders/compute_xxx/roots_chunk_0000.bin"
    }
  ]
}
```

Prefer `n_chunks` over `n_stripes` in new code.

## UI

The label `Stripes` is now misleading.

Recommended UI change:

- rename `Stripes` to `Chunks`

If you keep the old label for continuity, it should only be cosmetic. Internally:

- treat it as requested chunk count

## Testing

### Core Invariants

1. `param.bin` generation is deterministic
2. chunk ranges fully cover `0 .. n_steps`
3. no overlaps
4. no gaps
5. coeffgen output order is identical to canonical step order
6. solve output order is identical to coeff chunk order

### High-Value Cases

- `N=10, times=1, chunks=100`
- `N=10, times=10000, chunks=100`
- `N=20, times=100, chunks=200`
- `N=100, times=100, chunks=200`
- `N=500, times=1, chunks=10`

### Regression

For cases where the old pipeline is still available:

- old monolithic coeffgen output vs new param-stream coeffgen output
- final root outputs should match for the same logical traversal

## Rollout

### Step 1

Introduce `param.bin` generation as a separate stage.

### Step 2

Add chunk planner:

- `n_steps = N * N * times`
- chunk by contiguous step ranges

### Step 3

Add coeffgen-from-params mode.

### Step 4

Convert solve orchestration to chunk-centric coeff files.

### Step 5

Convert raster/finalize to `n_chunks`.

### Step 6

Rename metadata and UI from stripes to chunks where practical.

### Step 7

Remove the `stripes <= N` workaround entirely.

## Recommendation

The right long-term model is not:

- split rows
- split passes
- merge back into fake row stripes

The right long-term model is:

- generate the full transformed parameter stream once
- chunk that stream linearly
- make all downstream stages chunk-native

That removes the false row-band constraint instead of encoding it deeper.

## Implementation Checklist

### Phase 0: Terminology

- introduce `n_chunks` in new code paths
- stop using `n_stripes` as the conceptual model for new work
- decide whether UI keeps the old label temporarily

### Phase 1: Param Stream Generator

- add a new Lambda for param generation
- generate `(t1r, t1i, t2r, t2i)` records in canonical order
- apply param transforms here
- move dither randomness here
- stream-upload `params.bin` to S3
- return:
  - `params_key`
  - `n_steps`
  - `record_bytes`

### Phase 2: Chunk Planner

- add a chunk planner in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- input:
  - `N`
  - `times`
  - requested chunk count
- compute:
  - `n_steps = N * N * times`
  - contiguous `(step_start, step_count)` ranges
- skip zero-size chunks
- add tests for:
  - no overlaps
  - no gaps
  - exact coverage of all steps

### Phase 3: Coeffgen From Params

- update [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py) to read:
  - `params_key`
  - `step_start`
  - `step_count`
  - `chunk_idx`
- add coeffgen CLI support for parameter-stream input
- write:
  - `coeffs_chunk_{chunk_idx:04d}.bin`
- make coeffgen purely a coefficient-stage worker

### Phase 4: Solve From Chunked Coeffs

- dispatch one solve job per coeff chunk
- output:
  - `roots_chunk_{chunk_idx:04d}.bin`
- make orchestration and status polling use `n_chunks`

### Phase 5: Raster / Finalize Refactor

- update raster to use `chunk_idx`
- rename pix outputs to chunk-centric keys
- update finalize to aggregate over `n_chunks`
- remove row-stripe assumptions from finalize and bilevel finalize paths

### Phase 6: Metadata

- update `calc.json` to store:
  - `n_chunks`
  - `n_steps`
  - `params_key`
  - chunk metadata
- keep compatibility fields only where needed during migration

### Phase 7: Verification

- deterministic `params.bin` for same job spec
- chunk planner correctness
- coeffgen chunk output shape correctness
- solve chunk output correctness
- raster/finalize completeness across all chunks
- high-times benchmark:
  - `N=10, times=10000, chunks=100`

### Phase 8: Cleanup

- remove UI cap tied to `N`
- remove old row-band chunking code
- rename user-facing `stripes` terminology if desired

## Minimum Acceptable End State

- user can request `chunks > N`
- no empty-range failure mode
- `N=10, times=10000, chunks=100` is a normal supported job
- downstream stages operate on chunk count, not row-band count
