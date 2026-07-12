# Results List Refresh

Status: Phase 1 shipped. Phase 2 still planned.

## Problem

`Results` refresh currently takes around `4.9s`.

That is not catastrophic, but it is slow enough to feel like a compute step
instead of a simple list/refresh action.

The relevant current path is:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
  - `loadResults()`
  - calls `POST /list`
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
  - `handle_list()`
  - lists `renders/` prefixes
  - then fetches one `calc.json` per job in parallel

So the current refresh shape is:

- one S3 prefix listing
- then many small S3 `get_object(calc.json)` calls
- one Python fan-in response

This is a many-small-object latency problem.

## Lessons From s5cmd

The useful `s5cmd` lesson is not "use `s5cmd` inside Lambda".

The useful lesson is:

- many small objects -> bounded worker-pool concurrency
- few large objects -> ranged section concurrency

`Results -> Refresh` is the first category.

So the right optimization pattern is:

- improve concurrent small-object fetch
- not sectioned reads
- not a native binary

## Why Not Use s5cmd Directly

Using `s5cmd` itself inside the Lambda would add:

- another packaged runtime dependency
- subprocess launch overhead
- output parsing complexity
- a more brittle deploy/runtime story

For this request/response metadata path, that is the wrong trade.

The right move is to apply the same concurrency principles directly in Python
using the S3 client.

## Current Backend Diagnosis

The current [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
implementation already uses a thread pool:

- `ThreadPoolExecutor(max_workers=20)`

But that alone is not enough.

If the S3 client still uses the default botocore connection pool size, the code
can become connection-pool limited before it becomes worker limited.

So the likely current problem is:

- worker count says `20`
- real HTTP connection pool is smaller
- effective concurrency is lower than intended

This is exactly the same issue already identified for solve-score `merge`.

## Approach

### Phase 1: Cheap win

Keep the current `/list` contract and improve only its fan-in behavior.

Shipped in [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
and [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html).

`handle_list()` now:

- uses a dedicated S3 client for the list path
- sets a larger botocore `max_pool_connections`
- keeps a bounded worker pool for `calc.json` reads
- returns timing breakdown so the UI can log what happened

The Results tab now has a scrolling `results-log` area and logs:

- refresh start
- refresh success with total/list breakdown
- worker and pool settings
- refresh failure

The `Refresh` button now opens a small popup with:

- `Workers`

That value is sent to `/list` as `list_workers`, so tuning happens on the AWS
side instead of by measuring browser wall time.

Suggested first sweep:

- workers: `8`, `16`, `32`
- `max_pool_connections`: match or slightly exceed worker count

Success criterion:

- materially lower `list_us`
- no contract change
- same response shape to the frontend

### Phase 2: True fix

Stop rebuilding the full results list from S3 object metadata on every refresh.

Instead, maintain a compact results catalog:

- one JSON artifact in S3, or
- one DynamoDB table

The catalog should contain exactly what the table needs:

- `job_id`
- `function`
- `degree`
- `N`
- `n_chunks`
- `times`
- `total_size`
- `total_roots`
- any lightweight display metadata needed by the Results table

Then `POST /list` becomes:

- one read of the catalog
- optional lightweight filtering/sorting
- return immediately

This is the path most likely to make refresh feel instant.

### Phase 3: Lazy detail only

Keep the `Results` table refresh separate from heavier job-specific reads.

The current code already does this reasonably well:

- table refresh loads the list
- job detail loads on selection

That separation should stay.

If the catalog exists, `loadResults()` should remain a fast list-only path, and
all richer metadata should stay behind:

- `POST /detail`

## Recommended Design

### Short term

Phase 1 is now the baseline:

- dedicated list S3 client
- tuned `max_pool_connections`
- bounded worker pool
- visible timing/logging in the Results tab
- popup control for worker-count tuning

The next question is measurement:

- is this fast enough in practice
- or does the product still need the catalog/index path

### Medium term

Implement Phase 2:

- maintain a results index
- make `/list` read the index instead of scanning/fetching all `calc.json`

That is the real path to a refresh that feels like a list operation instead of a
mini compute job.

**SHIPPED (2026-07-12, with the favorites-speedup wave).** The catalog is a
DynamoDB partition (`results#catalog`, one row per job, reserved-prefix
guarded) and `/list` is now catalog + reconcile in `handle_list()`:

- membership truth = the cheap `renders/` prefix listing (unchanged)
- table fields = one paginated DDB Query
- `calc.json` is read ONLY for jobs the catalog has never seen, then cached
  forever (calc.json is written once, at compute completion — `finalize_metadata`
  / `/save-metadata`, and the latter upserts its row in lockstep)
- rows whose prefix vanished are pruned; `/delete` drops its row up front
- calc-less prefixes are re-probed only within a 24h mid-compute window;
  older ones are trusted as junk (`no_calc` rows)
- transient calc-read errors are surfaced in the response but NEVER cached
- `rebuild: true` (Refresh popup checkbox) re-reads everything — the escape
  hatch after manual S3 surgery

No pipeline hooks were needed: the reconcile self-heals, so a missed writer
can only delay a row by one `/list`, never lose it. Steady state is
~2-4 network ops total instead of `1 + n_jobs`. The frontend also became
session-cached (favorites-speedup idea 1): Results/DeepZoom tab re-entry is
zero requests; compute completion invalidates, popup Run / Refresh force.
Response gains `catalog_read_us` / `catalog_hits` / `catalog_misses` /
`catalog_pruned`, logged in the results-log line. Gated tests:
`tests/test_results_catalog.py`.

## What To Measure

For the existing `/list` path:

- total `list_us`
- number of jobs listed
- worker count
- S3 client `max_pool_connections`
- optionally:
  - prefix list time
  - aggregate calc fetch time

For the indexed path:

- total `list_us`
- index object size
- number of jobs returned

## Bottom Line

The `s5cmd` lesson applies.

But the answer is not:

- add `s5cmd` to Lambda

The answer is:

1. tune many-small-object concurrency correctly in Python now
2. then replace repeated metadata fan-in with a maintained results catalog

If the goal is "feels instant", the catalog/index is the real end state.
