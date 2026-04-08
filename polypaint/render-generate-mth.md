# Render Generate MT

Status: sectioned hist shipped; sectioned raster shipped; merge shipped;
finalize v1 shipped.

The first `Generate-MT` implementation did not materially improve `Render -> Color`.
In practice it was flat or slower than the single-thread baseline.

The reason is simple:

- splitting a chunk into subfiles
- launching multiple `roots2pix` subprocesses
- merging worker `.pix` / `.pbx` files

added too much overhead and did not attack the real hot path cleanly.

That approach is now considered a dead end.

This document replaces the old plan.

Since then, solve-score `hist` has been reworked into a native sectioned path
and is now live. Raster now also has a sectioned input path behind an explicit
`tmpfile | sectioned` selector. Both materially improved real render runs.
Solve-score `merge` is now also shipped with concurrent small-object fan-in.
`finalize` now has ordered concurrent fan-in with tunable worker count, and
render wall timing in the UI now comes from AWS-side timestamps instead of the
browser clock.

## Finding

What was tried:

- explicit `Generate-MT` button
- per-Lambda subprocess fan-out inside
  [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)
- worker-local `roots2pix` runs
- worker-local `.pix` / `.pbx` outputs
- merge back into the standard chunk/tile files

What happened:

- no meaningful speedup
- extra file slicing
- extra subprocess launch cost
- extra merge cost
- higher emitted-hit counts with no user-visible benefit
- overall result: not worth keeping as the real MT path

Conclusion:

- raster MT must be a true native shared-memory implementation
- solve-score prepass must also be truly multithreaded
- no multiprocessing hacks
- no subprocess fan-out inside the raster Lambda

## Status Update: stdin A/B Result

The solve-score `hist` `tmpfile` vs `stdin` benchmark was worth doing.

Result:

- `stdin` did not help
- `stdin` was slightly slower than `tmpfile`
- there was effectively zero useful savings

Why:

- the current `stdin` path is still full-buffered
- the C binary still reads the whole chunk before histogramming
- there is no real read/compute overlap
- `tmpfile` benefits from local page-cache behavior on the Lambda host
- `stdin` adds Python pipe overhead and backpressure

So the conclusion is:

- `stdin` as currently implemented is not the optimization
- the real next step is native S3 I/O in the hot path
- and, after that success, native raster I/O in the hot path

This document now treats `stdin` as an experiment that answered a useful
question, not as the main path forward.

## New Goal

`Generate-MT` should remain:

- an explicit button
- an explicit comparison path
- user-controlled thread count

But the backend must become:

1. native multithreaded solve-score prepass
2. native multithreaded raster
3. unified progress logging with thread counts for each phase

`Generate` remains:

- the single-thread baseline

`Generate-MT` becomes:

- the real threaded implementation
- and the testbed for native hot-path I/O

## Scope

### In scope

- true native multithreading for solve-score prepass
- true native multithreading for raster
- explicit thread selection in the `Generate-MT` popup
- phase logging that reports thread counts
- end-of-run logging that reports elapsed seconds everywhere

### Out of scope

- native multithreaded finalize hot-path rewrite
- multithreaded encode
- exact raster winner parity with single-thread output
- proximity-mode MT in the first native pass unless it is straightforward

## Implemented Single-Chunk Hist Pipeline

The solve-score `hist` experiment is now implemented:

- keep the existing `1 chunk -> 1 Lambda` contract
- move solve-score `hist` chunk I/O out of Python
- move chunk section download and histogramming into the native hot path
- split one chunk into solve-aligned sections
- let one native thread own one section end-to-end

This is now the preferred direction because it:

- matches the shipped workflow and status model
- avoids any Step Functions contract churn
- avoids changing the per-chunk histogram artifact shape
- attacks the real hot path directly

## What We Actually Learned

The useful result is no longer theoretical.

We now have:

- local laptop measurements
- AWS-side in-region benchmark Lambda measurements
- real end-to-end render runs on the shipped sectioned hist path

The local results were useful for inspection and correctness, but not for
download tuning. They were limited by the developer machine WAN path and Python
`boto3` behavior.

### AWS benchmark findings

Using the dedicated benchmark Lambda on `10240 MB`, with one invocation
processing one chunk at a time but sweeping internal chunk-worker concurrency:

- `compute_mmw2ilf7`
  - `max_chunks=8`
  - best tested concurrency: `2`
- `compute_mnj8n3f1`
  - `max_chunks=8`
  - best tested concurrency: `2`
- `compute_mn4id70k`
  - `max_chunks=8`
  - `2` and `3` were effectively tied
  - `4` was worse
- `compute_mnlddojx`
  - `max_chunks=8`
  - best tested concurrency: `2`

What this actually proved:

- do **not** assume "more cores => more chunk workers"
- for the **multi-chunk benchmark Lambda**, internal chunk-worker concurrency had
  a sweet spot around `2`

Also:

- this phase remains overwhelmingly download-bound
- compute time is tiny relative to chunk download time

But this benchmark was not the final shipped path.

It measured:

- one benchmark Lambda
- multiple whole chunks processed inside that invocation

It did **not** measure:

- one real hist Lambda per chunk
- one chunk split into sections
- each thread owning one section end to end

So it was useful for direction, but not for setting the final sectioned-hist
thread default.

### Live render findings on the shipped sectioned path

Real render runs are the source of truth for the implementation that now exists.

Representative runs on the same render family showed:

- `tmpfile`, `threads=2`
  - hist wall `58.1s`
  - total render wall `382.9s`
- `sectioned`, `threads=2`
  - hist wall `50.4s`
  - total render wall `261.6s`
- `sectioned`, `threads=4`
  - hist wall `38.8s`
  - total render wall `250.2s`
- `sectioned`, `threads=6`
  - hist wall `34.4s`
  - total render wall `240.7s`

Practical conclusion from the shipped path:

- `sectioned` is materially faster than `tmpfile`
- on the shipped sectioned path, `6` beat `4`, and `4` beat `2`, in the tested
  runs
- summed `download_ms` rises with more section threads, but wall time falls
- therefore tuning must optimize for `wall`, not aggregate download time

So the current live conclusion is:

- parallel section download inside one chunk
- is the real win
- the old benchmark-Lambda `2 is best` result should **not** be reused blindly
  as the default for the shipped sectioned path
- keep the thread count user-selectable and keep measuring `1,2,4,6`

### Raster observations from the same runs

The same render series also showed better raster wall times as thread count
increased:

- raster `threads=2`: wall `63.7s`
- raster `threads=4`: wall `62.3s`
- raster `threads=6`: wall `57.9s`

That earlier observation was before the raster sectioned path shipped.

With the shipped raster sectioned path, representative live runs showed:

- `sectioned`, `threads=6`
  - raster wall `50.0s`
  - raster aggregate `dl 566.5s + native 30.2s + up 160.8s`
  - total render wall `219.5s`
  - finalize wall `100.4s`
  - solve-score merge wall `19.8s`
- `sectioned`, `threads=6`, `merge workers=16`
  - hist wall `58.6s`
  - raster wall `49.8s`
  - raster aggregate `dl 569.1s + native 31.8s + up 179.2s`
  - finalize wall `106.5s`
  - total render wall `200.8s`
- after the finalize worker rollout shipped, a new live run showed:
  - hist `tmpfile`, `threads=4`: wall `40.9s`
  - merge `workers=16`: wall `1.5s`
  - raster `tmpfile`, `threads=4`: wall `42.5s`
  - finalize `workers=16`: wall `8.8s`
  - total render wall `104.9s`
  - this run emitted a much smaller output set than the earlier dense sectioned
    runs:
    - emitted root hits `21.6M` instead of `54.8M`
    - tile files uploaded `502` instead of `1500`
  - so it proves the shipped finalize optimization works, but it is not a clean
    apples-to-apples replacement for the earlier `200.8s` dense-run number

Practical conclusion:

- sectioned raster is a real improvement over the earlier tmpfile raster path
- end-to-end render wall time dropped from the earlier `~382.9s` tmpfile/hist
  baseline to `219.5s` on the sectioned hist + sectioned raster path
- after merge fan-in shipped, end-to-end render wall dropped again to `200.8s`
- that is roughly a `48%` end-to-end reduction from the earlier `382.9s`
  baseline
- after finalize fan-in shipped with `16` workers, finalize fell from the
  earlier `~100s` range to single-digit wall time in the latest live run
- merge is now negligible in practice on these runs
- finalize is no longer the obvious dominant stage; the remaining bottleneck is
  now workload-dependent between hist and raster

## Key AWS Facts

Some facts to pin down so we do not optimize the wrong thing:

- Lambda and S3 are already in the same region
  - this project is in `us-east-1`
  - there is no extra region change to make here
- there is no user-facing "faster Ethernet" setting for Lambda
- increasing Lambda memory can increase:
  - CPU
  - network throughput
  - general per-invocation performance
- the benchmark Lambda showed that chunk-level internal concurrency had a sweet
  spot around `2`
- the shipped sectioned hist path scaled differently and improved through `6`
  threads in the measured live runs

So the main performance levers here are:

- native I/O
- overlapping section download with section compute
- appropriate Lambda memory sizing
- path-specific concurrency tuning based on wall time

## Solve-Score Merge: Different Bottleneck

`merge` is not another large-chunk sectioned-read problem.

Current merge does:

- download `clip.json`
- download every per-chunk histogram artifact
- parse JSON
- sum 100-bin arrays
- derive 10 equal-density score thresholds ("cuts")
- upload one final bins JSON

So its shape is:

- many small S3 objects
- mostly request latency / fan-in overhead
- almost no compute

Representative live run:

- merge wall `19.8s`
- aggregate `dl 17.6s + compute 0.0s`

Current render shape here is `500` histogram artifacts, not `100`, so that
download time is plausible as pure small-object S3 GET latency plus JSON parse
overhead.

That means the direct hist/raster sectioned approach does **not** translate.

The right merge attack is:

- concurrent small-object fetch fan-in
- not sectioned range GET

First merge implementation target:

1. keep the same merge Lambda contract
2. keep the same per-chunk hist artifact keys
3. fetch hist JSONs concurrently with a bounded Python worker pool
4. sum them into one accumulator
5. derive cuts exactly as today
6. keep the final bins artifact unchanged

Implementation notes:

- use `ThreadPoolExecutor` first; this is an S3 fan-in problem, not a new native
  binary problem
- raise the boto3/botocore S3 client `max_pool_connections` to at least the
  worker count; otherwise worker counts above the default client pool size do
  not actually increase concurrent GET throughput
- benchmark bounded worker counts such as `8`, `16`, and `32`

Possible later follow-ons, only if needed:

- move chunk hist artifacts from JSON to a tiny binary format
- partial/tree merge if one merge Lambda becomes the limit

That first merge attack is now shipped and appears to have done what it needed
to do:

- merge is no longer the stage worth optimizing first
- finalize was the next fan-in/fan-out stage after merge, and its ordered
  worker-pool v1 is now also shipped

## Lessons From s5cmd

The useful lesson from `s5cmd` is the split between two different kinds of
parallelism:

- many-object worker concurrency
- per-object part/range concurrency

That maps directly to this renderer:

- solve-score `merge`
  - many small histogram artifacts
  - use bounded worker-pool fan-in
- `finalize`
  - many small raster artifacts
  - also a worker-pool problem, not a sectioned-read problem
- sectioned `hist`
  - one large chunk object
  - use per-object range concurrency
- sectioned `raster`
  - one large chunk object
  - use per-object range concurrency

So the plan should follow the same separation:

- many small objects -> worker pool
- few large objects -> ranged section concurrency

One concrete consequence:

- thread count alone is not enough on the many-object path
- the S3 client connection pool must scale with the worker count too
- otherwise merge/finalize will not realize the intended concurrency

## Native Single-Chunk Hist Design

### Goal

Build a new solve-score `hist` execution path where:

- one Lambda invocation still handles one chunk
- one native process handles that chunk
- native worker threads split the chunk into solve-aligned sections
- each worker thread downloads and histograms its own section
- Python is only orchestration, not the hot path

### Core idea

Instead of:

- Python downloads the whole chunk
- Python writes `/tmp`
- native code rereads `/tmp`
- native threads only parallelize the score math

do:

- Python prepares the request and invokes the native binary
- native code receives the chunk URL/size/degree/thread count
- native code splits the chunk into solve-aligned sections
- each thread:
  - downloads its own byte range
  - decodes solves in that section
  - computes a private histogram
- the process reduces per-thread histograms at the end

This keeps:

- `1 chunk -> 1 Lambda`
- one histogram artifact per chunk
- current Step Functions map shape
- current merge contract

while removing:

- Python chunk download from the hot path
- `/tmp` input write
- `/tmp` reread in the native process

## Native I/O Model

There are two realistic ways to let native code fetch chunk data.

### Option A: native AWS S3 client

The binary directly talks to S3 using a native AWS client.

Pros:

- direct S3 access
- no presigned URL step

Cons:

- packaging is heavier
- auth/signing in native code is more complex

### Option B: presigned HTTPS URLs plus native HTTP

The handler creates a presigned URL for the chunk object, then the native
binary downloads byte ranges via HTTPS.

Pros:

- no AWS auth code in the binary
- easier to reuse across binaries
- easier to test locally
- compatible with ordinary HTTP range requests

Cons:

- one presign step in Python
- still need a robust native HTTP client

Recommendation:

- use presigned URLs plus native HTTP for the first real single-chunk sectioned implementation

Reason:

- it keeps AWS-specific auth out of the C code
- it is easier to reuse later in raster and other chunk readers
- it keeps the native binary focused on I/O + compute, not credential logic

### Packaging concern

This choice originally added a real packaging question.

If the native binary uses `libcurl` for HTTPS, the deploy story must account for:

- `libcurl`
- TLS dependencies such as OpenSSL or the platform TLS stack
- ARM64 Lambda compatibility
- binary size / layer size
- whether the binary is:
  - dynamically linked against a dedicated layer
  - or statically linked as part of the build

This is not optional detail.

For sectioned hist, this question is now resolved.

What was actually shipped:

- ARM64 Docker build using `libcurl`
- dynamic runtime packaging of the needed `libcurl` dependencies
- deploy wiring that places those runtime libraries alongside the binary
- local and deploy-side smoke coverage for the sectioned hist path

So raster should not reopen the packaging decision from scratch.

Raster should reuse the same model:

- presigned HTTPS
- `libcurl`
- the same deploy/runtime library packaging pattern already proven by sectioned
  hist

What still remains for raster is narrower:

- verify that `roots2pix_mt` links and runs cleanly with the same runtime libs
- add a raster-specific local Range-HTTP smoke test
- add deploy packaging checks for the raster binary if new runtime artifacts are
  introduced

In other words:

- hist solved the `libcurl` packaging question
- raster still needs its own binary/runtime validation
- but the transport/packaging approach itself is no longer speculative

## Threading Model for Hist

### First native hist version: one thread per section

Do **not** start with:

- one thread pool for load
- another thread pool for compute
- queues between them

That is too clever for v1.

Start with:

- one solve-aligned chunk section per worker
- one worker thread owns that section
- that same thread:
  - downloads the section
  - histograms the section
  - accumulates a private histogram

This is the simplest design that actually attacks the bottleneck.

### Why this is the right first step

- no Step Functions change
- no per-chunk batching contract change
- no second coordination layer between I/O and compute
- low memory pressure
- directly aligned with the AWS benchmark evidence

### Initial thread-count policy

Support explicit thread counts:

- `1`
- `2`
- `4`
- `6`

Historical first default:

- `2`

Current guidance:

- keep section thread count user-selectable
- benchmark `1`, `2`, `4`, `6` on real renders
- do not treat the older multi-chunk benchmark-Lambda `2 is best` result as the
  settled sectioned-hist default

## Solve-Aligned Sectioning

For a given degree:

- `solve_bytes = degree * 2 * sizeof(float)`

Any per-thread byte range must:

- start on a multiple of `solve_bytes`
- end on a multiple of `solve_bytes`

That guarantees:

- no partial solve decoding
- no cross-thread record overlap

So for a `1GB` chunk:

- compute total solves from `Content-Length`
- split by solve count, not arbitrary bytes
- assign contiguous solve ranges to threads

## Memory Model

This plan is intentionally conservative.

The goal is **not**:

- load the entire chunk into one giant shared buffer first

The goal is:

- each thread owns one section buffer
- each thread downloads only its own section
- each thread computes directly from that section buffer

Important clarification:

- `section_bytes ~= chunk_bytes / threads`
- total resident section input across all workers is therefore roughly:
  - `threads * section_bytes ~= chunk_bytes`

So resident input memory scales mainly with total chunk size, not linearly with
thread count.

So memory stays roughly:

- `chunk_bytes`
- plus small per-thread HTTP/read buffers
- plus fixed process overhead
- plus small histogram scratch

That is a better fit for Lambda than:

- whole-chunk duplicate buffers
- multi-chunk in-flight buffering

### Initial hard guard

For the first native sectioned implementation, make the binary or handler reject
chunk sizes above a conservative budget instead of trying to soldier on.

Recommended initial guard:

- `max_chunk_bytes <= 0.5 * lambda_memory_bytes`

This is intentionally conservative.

It leaves room for:

- Python runtime
- native process overhead
- `libcurl` / TLS buffers
- per-thread section buffers
- root-transform scratch
- output JSON / bookkeeping

Example:

- `10240 MB` Lambda
- initial supported chunk size limit: about `5 GB`

That is already far above the current benchmarked chunk sizes, which were on
the order of:

- `~61 MB`
- `~27 MB`
- `~134 MB`

So the first version has ample headroom while still having a real safety stop.

### Thread-count implication

For this design, thread count still matters for:

- HTTP connection pressure
- per-thread temporary buffers
- total scratch

but it does **not** multiply the full input buffer size.

That is one more reason the live thread-count sweep should remain:

- `1`
- `2`
- `4`
- `6`

## Workflow / Step Functions Plan

### Current

- `ColorSolveScoreHistMap` iterates `chunk_items`
- one item per chunk
- one `solve_proximity` Lambda invocation per chunk

### Keep this

Do **not** change the render workflow contract for the first pass.

That means:

- `ColorSolveScoreHistMap` stays chunk-based
- merge stays exactly the same
- status/progress semantics stay the same

Only the internals of the hist Lambda change.

## Handler Shape

Add a dedicated native sectioned hist path inside
[lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
or split it into a sibling handler if that keeps the code cleaner.

Preferred first shape:

- keep the same handler
- add a new hist input/engine mode for native sectioned download

The handler would:

1. load the clip artifact once
2. presign the chunk object
3. invoke one native binary for that one chunk
4. receive one histogram JSON result
5. upload the per-chunk histogram artifact
6. report detailed perf including thread count

Python should **not**:

- download the chunk
- write `/tmp` input
- split chunk sections itself

## Native Hist Binary Shape

Recommended first step:

- add a dedicated native binary instead of overloading the current CLI too much

Suggested file:

- `lambda/solve_proximity_hist_sectioned.c`

Reason:

- keeps the current tested one-chunk CLI stable
- makes rollback easy
- lets the new binary focus on:
  - presigned URL input
  - range GET
  - section partitioning
  - histogram reduction

### Native input

Pass:

- metric
- clip bounds
- omega settings
- degree
- thread count
- chunk size
- presigned chunk URL
- optional root transforms path

### Native output

Emit one JSON summary to stdout containing:

- `threads`
- `bytes_downloaded`
- `download_ms`
- `compute_ms`
- `n_solves`
- `hist`

Optionally include:

- per-thread section stats

## Native HTTP Implementation

### First recommendation

Use an HTTP client that supports:

- regular GET
- range GET
- connection reuse

Examples:

- `libcurl` with multi interface

Why:

- mature
- straightforward for presigned HTTPS
- supports exactly the concurrent range/read pattern we care about

### First version behavior

Do not overreach.

For v1 native sectioned hist:

- one chunk object
- one presigned URL
- `N` range GETs
- `N` worker threads
- one local histogram per thread
- one final reduce

No streaming-for-huge-chunks logic yet.

If streaming is needed later, add it as a follow-on optimization.

## How This Reuses for Other Stages

If native presigned-URL range I/O works for hist, the same pattern can later be
reused for:

- raster chunk input
- other read-heavy chunked phases

So hist remains a useful proving ground for:

- native HTTP in Lambda
- in-process section parallelism
- measured download/compute overlap

## Logging Requirements

This experiment must report enough detail to be credible.

### Hist progress logs

For sectioned single-chunk hist:

- `threads=<N>`
- `bytes=<...>`
- `input=native_http_sectioned`
- `sections=<N>`

Example:

```text
Solve score: hist 24/500 · wall=18.6s aggregate=download 41.2s + compute 33.7s · threads=2 · sections=2 · input=native_http_sectioned
```

### Final hist perf fields

Per chunk result data should include:

- `threads`
- `sections`
- `bytes_downloaded`
- `download_ms`
- `compute_ms`
- `input_mode=native_http_sectioned`

Optional but strongly recommended:

- `download_mb_per_s`
- `avg_section_download_ms`
- `avg_section_compute_ms`

## Hist, Raster, Merge, And Finalize Outcome

The solve-score and raster experiments have now answered the important question.

Done:

1. `tmpfile` baseline
2. `stdin` A/B
3. AWS-side benchmark Lambda
4. shipped native sectioned hist path
5. live render tuning on `2`, `4`, `6`
6. shipped native sectioned raster path
7. live render validation of sectioned raster
8. shipped concurrent solve-score merge fan-in
9. shipped concurrent finalize fan-in with tunable worker count

Conclusion:

- keep sectioned hist
- keep `tmpfile` only as the fallback/baseline
- keep section thread count selectable
- keep sectioned raster
- keep raster `tmpfile` as the fallback/baseline
- keep merge workers tunable
- keep finalize workers tunable
- the current remaining bottleneck depends on the render family and output
  density; there is no single universally dominant stage now
- use same-family / same-density runs when comparing `tmpfile` vs `sectioned`
  or thread-count changes

## Success Criteria

For the hist work, the experiment is already worth continuing because it gave:

- clear wall-time reduction on the same render
- a reusable pattern for native sectioned input
- enough gain to justify reusing the model in raster

The current live numbers indicate:

- merge has already fallen enough that it is no longer the first thing to fix
- finalize v1 succeeded: on the latest live run it fell to `8.8s` wall with
  `16` workers
- the old `~100s` finalize timings are now pre-finalize-optimization history,
  not the current shipped state
- end-to-end comparisons still need workload discipline, because sparse runs can
  look dramatically better than dense runs even with the same code

For the next optimization round, the target should be:

- keep validating on the same render family before comparing modes
- focus on whichever stage is actually dominant for that workload
- keep the current tile/raw output contract unless a deeper redesign is chosen

## Chunk Count Is A Design Choice

The current `500`-chunk render shape is an implementation choice, not a law of
the problem.

Today we do:

- solve emits many chunk objects
- hist runs per chunk
- merge combines per-chunk hist artifacts into one global cut artifact
- raster runs per chunk
- finalize reads all per-chunk raster artifacts and assembles the final image

That shape is good for:

- retry isolation
- simple Step Functions map fan-out
- bounded per-Lambda memory

But it also creates the two remaining fan-in bottlenecks:

- solve-score `merge`
- render `finalize`

### Larger structural option

A future redesign could intentionally reduce chunk count, or even collapse to
one very large solve object after solve, then raster it with sectioned streaming
reads.

That would mean:

- solve emits one large object, or a small number of large objects
- raster threads range-GET solve-aligned sections from that object
- solve data is streamed section by section, not fully materialized in memory
- shared image/tile state is kept in memory
- raster emits final image artifacts directly

If that path works, it could remove most or all of:

- per-chunk raster artifact fan-in
- finalize as a separate stage

### What "no merge / no finalize" really means

For non-solve-score color modes:

- yes, in principle a direct streamed raster path could eliminate the current
  finalize stage entirely
- raster would write the final image/tile outputs directly instead of emitting
  per-chunk intermediates

For `solve_score` color mode:

- finalize could still disappear under that model
- but merge only disappears if global cuts are no longer produced from
  per-chunk hist artifacts

That requires one of:

1. accept lores-only cuts and skip full hist/merge
2. compute the full global histogram inside the same monolithic or few-object
   streamed worker path, then derive cuts there
3. do a two-pass streamed design over the large solve object:
   - pass 1: histogram / cut derivation
   - pass 2: raster with final cuts

So "one huge streamed chunk" can remove finalize cleanly, but it only removes
solve-score merge if the cut-derivation architecture also changes.

### Why this is attractive

- the solve data can be much larger than memory, while the final image/tile
  state still fits in memory
- native sectioned range GET already proved useful for hist and raster
- it attacks the current fan-in stages at the source instead of optimizing
  around them

### Why this is not the immediate next step

- one huge-object raster worker has larger timeout and retry blast radius
- image/tile state must stay bounded and predictable
- direct-final-output raster must replace today's finalize contract cleanly
- solve-score cut derivation must be redesigned if merge is to disappear

So the current plan now is:

1. treat merge and finalize v1 as shipped
2. keep measuring heavy and light render families separately
3. keep the "few large streamed objects" architecture as the next deeper
   redesign if fan-in/fan-out stages become dominant again

## Finalize: Shipped V1 Outcome

`finalize` is not another sectioned-read problem.

Current [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)
does this for each tile:

- loop over all chunk-local `.pix` files in chunk order
- `get_object` each one sequentially
- stream them into `pixassemble`
- then loop over all `.pbx` files sequentially
- stream them into `pixbinassemble`
- upload the final tile raw and final dense pixel-bin tile

So finalize is a combined problem:

- many small-object S3 GET latency
- many small-object S3 PUT latency
- object-count fan-in/fan-out

### Important correctness constraint

[lambda/pixassemble.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/pixassemble.c)
is explicitly:

- "last wins"

That means finalize cannot simply fetch chunk files concurrently and write them
to `pixassemble` stdin in arbitrary completion order.

If chunk ordering changes, winner semantics can change.

So the first finalize optimization must preserve logical chunk order even if the
downloads happen in parallel.

### First finalize attack

Keep the current contract:

- one finalize Lambda per tile
- same tile outputs
- same `pixassemble` / `pixbinassemble` tools

But change the internal read path to:

1. dedicated finalize S3 client with higher `max_pool_connections`
2. bounded worker-pool fetch for per-chunk `.pix` objects
3. bounded worker-pool fetch for per-chunk `.pbx` objects
4. preserve chunk order when feeding bytes into the assembler stdin

Recommended implementation shape:

- submit chunk fetches ahead of time with a bounded prefetch window
- store completed chunk bodies in a small in-memory reorder buffer keyed by
  `chunk_idx`
- only write to `pixassemble` / `pixbinassemble` when the next required chunk in
  order is available

This gives:

- overlap of S3 request latency
- preserved "last wins" semantics
- no first-pass workflow change

This is now shipped.

Representative live results:

- before finalize fan-in optimization:
  - finalize wall was in the `~100s` range on the sectioned render runs above
- after finalize fan-in optimization with `16` workers:
  - finalize wall `8.8s`
  - finalize aggregate `read 11.7s + assemble 0.2s + up 9.5s + pbx 8.0s`
  - total render wall `104.9s`

UI requirement for this phase:

- expose `Finalize workers` as a tunable worker-count control in the
  `Generate-MT` popup
- this is now a dedicated `Finalize` row in the MT table:
  - `Hist`: threads, input
  - `Raster`: threads, input
  - `Merge`: workers
  - `Finalize`: workers
- treat it like `Merge workers`, not like hist/raster thread count
- do not add finalize-specific input-mode controls in the first pass

### What not to do first

Do not start by:

- rewriting finalize in native C
- changing tile output contracts
- changing chunk-local artifact layout
- writing inputs to `/tmp` just to sort them again

Those may become useful later, but they are not the first attack.

### First-pass measurement goal

For the current render family, first-pass finalize optimization should try to
reduce:

- `read_ms` first

without regressing:

- `assemble_ms`
- `upload_ms`

### Likely later follow-ons

If the order-preserving prefetch attack is not enough, the next finalize
follow-ons are structural:

- reduce the number of intermediate raster objects emitted per tile
- group chunk-local raster outputs into fewer larger finalize inputs
- or bypass finalize entirely under the deeper "few large streamed objects"
  redesign

## Implementation Order

1. keep the current sectioned hist path stable
2. keep the current sectioned raster path stable
3. keep merge and finalize v1 stable
4. keep measuring same-family renders before changing defaults
5. tune hist/raster/finalize controls only from real wall-time data
6. if fan-in/fan-out becomes dominant again, benchmark whether a deeper
   streamed-object redesign is worth the complexity
7. only then decide whether finalize needs a larger structural redesign

## Appendix: Completed stdin A/B

The old `tmpfile` vs `stdin` experiment is no longer part of the main plan.

Outcome:

- `stdin` did not help
- `stdin` was slightly slower than `tmpfile`
- it did not provide the read/compute overlap needed to move wall time

Reason:

- the implementation was still full-buffered
- the binary still consumed the full chunk before histogramming
- so the real bottleneck remained chunk download

Historical note only:

- keep the existing `stdin` support as a completed experiment
- do not invest further in that path for the main solve-score MT work
- orchestrator payload includes `solve_score_hist_input_mode`
- log lines include `input=...`

## Current Runtime Architecture

### Hist

The shipped solve-score `hist` path is now:

- one chunk per Lambda
- presigned URL handed to a native sectioned binary
- solve-aligned sections
- one thread owns one section end-to-end
- one per-chunk histogram artifact emitted

The older `tmpfile` path remains only as:

- fallback
- baseline for comparison

### Raster

The shipped `Generate-MT` raster path is now:

- one chunk per Lambda
- native `roots2pix_mt`
- selectable raster input:
  - `tmpfile`
  - `sectioned`
- one thread owns one solve-aligned section end-to-end
- shared tile/image state in memory
- atomic pixel claim semantics
- one final chunk raster output set per Lambda

The important point is:

- raster sectioning is now real and shipped
- future work should optimize around this path, not the discarded subprocess
  fan-out version

### Merge

`merge` remains:

- one Lambda
- many small histogram artifact downloads
- almost no compute

So merge is:

- not a native sectioned-read problem
- not a new C-binary problem
- a Python S3 fan-in problem

### Finalize

`finalize` remains:

- many small raster artifact downloads
- many small output uploads
- significant object-count fan-in/fan-out

So finalize is still structurally a small-object fan-in/fan-out stage, but the
shipped ordered-prefetch worker pool means it is no longer automatically the
largest wall-time stage on every render family.

## Current UI / Logging Expectations

`Generate`:

- remains the single-render baseline
- in `solve_score` mode, the hist input selector is still useful as an explicit
  A/B between:
  - `tmpfile`
  - `sectioned`

`Generate-MT`:

- remains the threaded path
- should keep explicit selectors while benchmarking
- popup layout should stay table-like and compact:
  - columns: `Threads`, `Input`, `Workers`
  - rows: `Hist`, `Raster`, `Merge`, `Finalize`
- stage-specific controls:
  - `Hist`: threads + input
  - `Raster`: threads + input
  - `Merge`: workers
  - `Finalize`: workers
- cells that do not apply to a stage should render as disabled/placeholder cells,
  not as extra free-form controls

Logs should continue to make the active path obvious:

- hist logs include `threads=` and `input=`
- raster logs include `threads=` and `input=`
- solve-score merge phase logs currently show `threads=<merge_workers>` because
  they reuse the solve-score phase perf formatter
- popup summaries and `Render-MT` dispatch logs show `merge workers=`
- `Render-MT` dispatch logs include `finalize workers=`
- finalize backend status rows carry `workers`, but the current browser finalize
  perf summary shows `read + assemble + up + pbx`, not `workers`
- completion logs include elapsed wall time

## Timing Semantics

The current UI no longer trusts browser wall-clock timing for render duration.

What is shipped now:

- top-level render elapsed time is derived from AWS-side timestamps:
  - `run_started_at_ms`
  - `updated_at_ms`
- phase wall time is derived from:
  - phase `started_at_ms`
  - latest worker/server timestamp seen for that phase
- browser `Date.now()` is only a fallback when server timestamps are missing

This matters because hidden/background browser tabs can throttle timers.
Current render `wall=` numbers are therefore intended to reflect workflow-side
timing, not tab visibility artifacts.

## Relevant Files

Core implementation files:

- [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
- [lambda/solve_proximity_hist_sectioned.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_hist_sectioned.c)
- [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)
- [lambda/roots2pix_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix_mt.c)
- [lambda/handler_finalize.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)
- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- [lambda/handler_render_orchestrator.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_orchestrator.py)
- [lambda/handler_render_status.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_status.py)
- [stepfunctions/render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)
- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)

Key verification files:

- [tests/test_solve_proximity_hist_sectioned.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_solve_proximity_hist_sectioned.py)
- [tests/test_roots2pix_mt_sectioned.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_roots2pix_mt_sectioned.py)
- [tests/test_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_raster_mt.py)
- [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)
- [tests/test_render_workflow_definition.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_workflow_definition.py)
- [tests/test_deploy_packaging.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_deploy_packaging.py)
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

## Recommendation

The plan is now:

1. keep the shipped sectioned hist path stable
2. keep the shipped sectioned raster path stable
3. keep the shipped concurrent solve-score merge path stable
4. implement ordered concurrent fan-in for `finalize`
5. benchmark finalize worker counts together with S3 client
   `max_pool_connections`
6. only then decide whether the deeper "few large streamed objects" redesign is
   justified

That sequencing matches both the measured timings and the `s5cmd` lesson:

- many small objects -> worker-pool concurrency
- few large objects -> ranged section concurrency
