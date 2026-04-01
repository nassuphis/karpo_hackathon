# Replacing `n1` / `n2` With `N`

Status: historical migration note. Current compute metadata is already centered on `N` plus chunk-based planning; examples here may still show older `stripe_idx`/`n_stripes` payloads.

This document updates the earlier refactor note against the current codebase.

The core problem has not changed:

- the UI exposes one grid size, `N`
- the backend contracts still mostly speak in `n1`, `n2`, `i1_start`, `i1_end`
- the newer coeff and param-debug paths inherited the same naming

So the code still leaks rectangular-grid machinery into places that are square-grid-only in product terms.

## Recommendation

Do the refactor in two layers:

1. Make `N` the public contract everywhere above the C binary boundary.
2. Keep internal rectangular support in the binaries unless and until there is a good reason to remove it.

That is still the right split. The repo has grown, but the architecture answer is the same.

## Current State

There are now three distinct categories of code:

### 1. Public/UI code that should use `N`

These paths are square-grid-only from the user’s point of view:

- Compute tab
- Results list / metadata display
- root render pipeline
- coeff render pipeline
- param debug pipeline

For these, `n1` / `n2` are just internal leakage.

### 2. Adapter code that should translate `N` to binary inputs

These handlers sit at the boundary and are the right place to keep compatibility logic:

- `lambda/handler_coeffgen.py`
- `lambda/handler_sweep.py`
- `lambda/handler_param_debug.py`

These should accept `N` publicly, then synthesize `n1 = N`, `n2 = N` for the binaries.

### 3. Internal binaries that still genuinely use rectangular semantics

These are not just using bad names. They really do operate on two dimensions:

- `lambda/sweep_cli.c`
- `lambda/lores_viewport.c`
- `lambda/param_gen.c`

In those binaries:

- `x1 = i1 / n1`
- `x2 = i2 / n2`
- stripe height is based on `i1_start` / `i1_end`
- solver row width is `n2`

That means removing `n1` / `n2` inside C is a separate and deeper refactor. It is not required to clean up the public API.

## Where The Leak Still Exists

### 1. Results UI is inconsistent today

The table header already says `N` in `index.html:1091`, but the row renderer still displays `r.n1` in `index.html:1315`.

So the user already sees the new label backed by the old field.

This is exactly the sort of half-refactor that should be finished cleanly.

### 2. Compute orchestration still fans out `n1` / `n2`

`runCalculate()` still duplicates the square grid into:

- `n1: n, n2: n` for coeffgen jobs at `index.html:2691`
- `n1: loresN, n2: loresN` for lores coeffgen at `index.html:2721`
- `n1: loresN, n2: loresN` for lores solve at `index.html:2736`
- `n1: n, n2: n` for hires solve at `index.html:2755`

And the saved `calc.json` metadata still stores:

- `n1`, `n2` at `index.html:2824`
- `lores.n1`, `lores.n2` at `index.html:2833`

So the main compute path is still entirely on the old contract.

### 3. Param debug inherited the same leak

The newer param-debug path now uses the real `param_gen` C pipeline, which is good, but the request contract is still:

- `n1: n, n2: n` in `index.html:2621`

and the Lambda still parses:

- `n1` / `n2` in `lambda/handler_param_debug.py:102`
- then forwards those to `param_gen` in `lambda/handler_param_debug.py:114`
- and returns `n1`, `n2` in the response at `lambda/handler_param_debug.py:165`

So even the new debug-only path is already carrying the old naming.

### 4. Coeffgen handler still requires `n1` / `n2`

`lambda/handler_coeffgen.py` builds the sweep spec with:

- `n1: params["n1"]`
- `n2: params["n2"]`

at `lambda/handler_coeffgen.py:43-44`.

This is the right place to add compatibility translation.

### 5. Sweep handler still requires `n1` / `n2`

Grid mode still forwards:

- `n1`, `n2` at `lambda/handler_sweep.py:51`
- `n1`, `n2` at `lambda/handler_sweep.py:78`

Solve-from-coeffs is slightly different:

- it only actually needs `n2` for the solve spec at `lambda/handler_sweep.py:184`
- but the public caller still sends both `n1` and `n2`

That is a useful clue:

- public API should use `N`
- internal solver adapter can derive the one width field it actually needs

### 6. Storage/listing still exposes old metadata

`lambda/handler_storage.py` still reads:

- `calc["n1"]` at `lambda/handler_storage.py:80`
- `calc["n2"]` at `lambda/handler_storage.py:81`

and the results table consumes that old shape.

This is also related to the coeff render bug you recently hit: summary metadata is thinner and more brittle than the full compute metadata. The more this path stays on legacy field names, the easier it is for callers to miss required fields.

### 7. C binaries still parse `n1` / `n2`

The current binaries still genuinely parse and use rectangular inputs:

- `lambda/sweep_cli.c`
- `lambda/lores_viewport.c`
- `lambda/param_gen.c`

Examples:

- `sweep_cli.c` parses `n1` / `n2` around `lambda/sweep_cli.c:2523-2529`, `2662-2670`, and `2806-2821`
- `lores_viewport.c` parses `n1` / `n2` at `lambda/lores_viewport.c:673-679`
- `param_gen.c` parses `n1` / `n2` at `lambda/param_gen.c:365-366`

Those should not drive the public API design.

## What Should Change

### Public contract

The public square-grid contract should be:

- `N`
- `row_start`
- `row_end`

That applies to:

- compute dispatch
- lores compute
- coeff render launch
- param debug launch
- saved `calc.json`
- `/list` result summaries

Suggested compute payload shape:

```json
{
  "job_id": "compute_xxx",
  "stripe_idx": 7,
  "function": "poly_33",
  "N": 500,
  "row_start": 350,
  "row_end": 400,
  "times": 1
}
```

For solve-from-coeffs, the public contract should still stay square:

```json
{
  "job_id": "compute_xxx",
  "stripe_idx": 7,
  "coeffs_key": "renders/compute_xxx/coeffs_0007.bin",
  "n_coeffs": 71,
  "N": 500,
  "row_start": 350,
  "row_end": 400
}
```

The handler can then derive:

- `n2 = N`
- `i1_start = row_start`
- `i1_end = row_end`

### Metadata contract

`calc.json` should stop storing:

- `n1`
- `n2`

and instead store:

- `N`

For the lores section:

- `lores.N`

Suggested shape:

```json
{
  "job_id": "compute_xxx",
  "function": "poly_33",
  "N": 500,
  "n_stripes": 50,
  "times": 1,
  "degree": 70,
  "n_coeffs": 71,
  "lores": {
    "N": 60,
    "bin_key": "renders/compute_xxx/lores.bin"
  }
}
```

### Param debug contract

The param-debug public request should also use `N`, even if `param_gen.c` continues to take `n1` / `n2` internally.

That means:

- frontend sends `N`
- handler accepts `N`
- handler translates to `n1 = N`, `n2 = N`
- handler response returns `N`, not `n1` / `n2`

This is worth doing because param debug is now a real feature, not a throwaway script.

## What Should Not Change Yet

Do not rush to remove `n1` / `n2` from:

- `lambda/sweep_cli.c`
- `lambda/lores_viewport.c`
- `lambda/param_gen.c`

There are still legitimate internal reasons for those binaries to think in two dimensions:

- rectangular sampling math
- row-partitioned stripes
- per-row coefficient / solve layouts

The public API cleanup does not depend on changing that.

## Concrete Refactor Plan

### Phase 1: Frontend and metadata cleanup

Update `index.html` so the public payloads use:

- `N`
- `row_start`
- `row_end`

instead of:

- `n1`
- `n2`
- `i1_start`
- `i1_end`

Touch points:

- `runParamDebug()` at `index.html:2620`
- coeffgen job assembly at `index.html:2686`
- lores coeffgen request at `index.html:2716`
- lores solve request at `index.html:2732`
- hires solve request at `index.html:2751`
- `calcMeta` save at `index.html:2816`

Also finish the Results UI refactor:

- keep the `N` header
- stop rendering `r.n1`
- render `r.N`

### Phase 2: Handler compatibility layer

Teach handlers to accept both contracts temporarily:

- if `N` is present, synthesize `n1 = N`, `n2 = N`
- if `row_start` / `row_end` are present, synthesize `i1_start` / `i1_end`
- otherwise fall back to legacy fields

Handlers to change:

- `lambda/handler_coeffgen.py`
- `lambda/handler_sweep.py`
- `lambda/handler_param_debug.py`

This should be done before removing legacy frontend fields, so old jobs and tests keep working during the transition.

### Phase 3: `calc.json` and storage listing

Save new jobs with:

- `N`
- `lores.N`

and make `lambda/handler_storage.py` read:

- `calc["N"]` first
- fall back to `calc["n1"]` for old jobs

That gives backward compatibility without keeping the old schema as primary.

### Phase 4: Tests and docs

Update the Python test suite and docs so public examples use `N`.

This matters because a lot of the stale `n1` / `n2` language is now in:

- `docs/calc_tab.md`
- `docs/lambdas.md`
- `docs/s3results.md`
- `docs/coefficients.md`
- `docs/roots.md`
- many `tests/test_pipeline.py` and `tests/test_sweep_smoke.py` payloads

Some tests should keep rectangular cases, but they should be explicitly internal-engine tests, not the default public examples.

## Compatibility Strategy

The safe transition is:

1. accept both new and old handler payloads
2. write new metadata as `N`
3. read old metadata as fallback
4. migrate frontend to only emit `N`
5. update docs and most tests
6. only then consider whether the C binaries deserve a deeper rename

This avoids breaking:

- old saved jobs in S3
- existing tests
- any ad hoc scripts still posting legacy payloads

## Final Recommendation

The doc needs one important update from the earlier version:

- `n1` / `n2` are no longer just a compute-path problem
- they now leak into coeff render, param debug, results listing, and metadata too

So the cleanup is more valuable now than before.

But the scope should still stay disciplined:

- make `N` the only public square-grid concept
- keep `n1` / `n2` as internal binary fields for now

That gives you the simplification you want without forcing an unnecessary rewrite of the engine internals.
