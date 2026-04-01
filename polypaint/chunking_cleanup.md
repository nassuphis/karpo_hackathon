# Chunking Cleanup

Status: historical cleanup plan. The active compute and render paths are now chunk-based, but this document preserves the earlier rename plan and compatibility concerns.

## Purpose

After the chunking refactor, some code is:

- truly unused by the current app flow
- still needed for compatibility with tests and helper scripts
- still active, but named around the old stripe abstraction

This note separates those cases so cleanup can happen intentionally.

## Safe To Delete

### `handle_compute_only_stripe()` in `handler_sweep.py`

File:

- [lambda/handler_sweep.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_sweep.py)

Why it looks deletable:

- current browser compute flow no longer dispatches raw grid compute jobs through `sweep`
- the active compute path is:
  - `param_gen`
  - `coeffgen_chunked`
  - `solve_from_coeffs`
- current UI calls `sweep` only with `coeffs_key`, which routes into `handle_solve_from_coeffs()`

Before deleting:

- confirm there are no external API callers still using the old `compute-only-stripe` route
- confirm deploy/docs do not still advertise that route

Recommended action:

- remove `handle_compute_only_stripe()`
- simplify handler dispatch to solve-only if no external callers remain

## Compatibility-Only Code

These are not dead yet. They are legacy paths still used by tests, scripts, or migration safety.

### Legacy coeffgen handler path

File:

- [lambda/handler_coeffgen.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeffgen.py)

Function:

- `handle_legacy_coeffgen()`

Why it stays for now:

- many local tests and helper scripts still call old coeffgen directly
- examples:
  - [tests/test_sweep_smoke.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_sweep_smoke.py)
  - [tests/test_poly_solver.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_poly_solver.py)
  - multiple visual comparison scripts under [lambda](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda)

Recommended action:

- keep for now
- mark clearly as legacy
- remove only after tests and scripts migrate to `param_gen` + `coeffgen_chunked`

### Legacy `coeffgen` mode in `sweep_cli.c`

File:

- [lambda/sweep_cli.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/sweep_cli.c)

Function:

- `runCoeffGen()`

Why it stays for now:

- same reason as legacy coeffgen handler
- local test/tools ecosystem still depends on it

Recommended action:

- keep during transition
- remove only after the compatibility surface is migrated

## Active But Rename-Later

These are still live, but the names are now misleading after the chunk refactor.

### `stripe_idx`

Files:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [lambda/handler_sweep.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_sweep.py)
- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
- [lambda/handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py)

Status:

- active
- effectively means chunk index in the new pipeline

Recommended action:

- rename to `chunk_idx` once downstream handlers and metadata are migrated together

### `n_stripes`

Files:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)
- [lambda/handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py)

Status:

- active
- currently used as “number of chunks” by compatibility

Recommended action:

- rename to `n_chunks` once finalize/bilevel paths are updated coherently

### `stripe_*.bin` / `pix_{stripe}_...`

Files:

- [lambda/handler_sweep.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_sweep.py)
- [lambda/handler_raster.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
- [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)
- [lambda/handler_bilevel.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py)

Status:

- active naming
- semantically chunk-based now

Recommended action:

- rename outputs to chunk-based keys in one coordinated pass
- do not partially rename only one stage

## Docs To Update

### `handler_sweep.py` module docstring

File:

- [lambda/handler_sweep.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_sweep.py)

Problem:

- still describes the handler as a single `compute-only-stripe` route
- no longer matches the real dominant usage

Recommended action:

- rewrite the docstring to describe:
  - solve-from-coeffs as primary path
  - any remaining legacy path explicitly as legacy

## Cleanup Order

### First

- update stale docs
- mark legacy functions with comments

### Second

- migrate downstream naming from stripe to chunk

### Third

- migrate remaining tests/scripts off legacy coeffgen

### Last

- delete:
  - `handle_compute_only_stripe()`
  - legacy coeffgen handler path
  - legacy `runCoeffGen()`

## Recommendation

Do not delete the legacy coeffgen path yet.

Do delete or at least deprecate the old compute-only sweep path once you confirm there are no external callers.

The bigger cleanup win is not deletion first. It is renaming the still-live compatibility layer so the code stops lying about chunks being stripes.
