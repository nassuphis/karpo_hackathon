# Render Generate MT

Status: v1 failed.

The first `Generate-MT` implementation did not materially improve `Render -> Color`.
In practice it was flat or slower than the single-thread baseline.

The reason is simple:

- splitting a chunk into subfiles
- launching multiple `roots2pix` subprocesses
- merging worker `.pix` / `.pbx` files

added too much overhead and did not attack the real hot path cleanly.

That approach is now considered a dead end.

This document replaces the old plan.

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
- and likely batching multiple chunks per invocation

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
- and the testbed for native batched S3 I/O

## Scope

### In scope

- true native multithreading for solve-score prepass
- true native multithreading for raster
- explicit thread selection in the `Generate-MT` popup
- phase logging that reports thread counts
- end-of-run logging that reports elapsed seconds everywhere

### Out of scope

- multithreaded finalize
- multithreaded encode
- exact raster winner parity with single-thread output
- proximity-mode MT in the first native pass unless it is straightforward

## New Experiment Goal: Native S3 Hist Pipeline

The next serious experiment is:

- move solve-score `hist` chunk I/O out of Python
- move chunk download and histogramming into the native hot path
- batch multiple chunks per Lambda invocation
- measure how much wall time drops when startup, Python copy, and per-chunk
  overhead are reduced

This is attractive because:

- render is still too slow
- the same native I/O pattern can later be reused for raster and other steps
- it is a meaningful systems experiment, not just a micro-optimization

## What We Are Actually Optimizing

The current solve-score `hist` architecture is:

1. one Step Functions item per chunk
2. one Lambda invocation per chunk
3. Python `boto3` download per chunk
4. Python launches one native process per chunk
5. native process histograms one chunk
6. Lambda uploads one tiny JSON result

This means the system pays, per chunk:

- Lambda startup / handler overhead
- Python import / `boto3` / subprocess overhead
- one S3 object fetch
- one clip JSON fetch
- one native process launch

When there are hundreds of chunks, that overhead dominates.

The current bottleneck is not the score math.

The current bottleneck is:

- too many small chunk invocations
- too much Python in the hot path
- too much per-object overhead

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
- if a Lambda is put inside a VPC, S3 access can get worse unless the network
  path is configured carefully

So the main performance levers here are:

- fewer invocations
- larger per-invocation useful work
- native I/O
- appropriate Lambda memory sizing
- better overlap of download and compute

## Native S3 Hist Plan

### Goal

Build a new solve-score `hist` execution path where:

- one Lambda invocation handles multiple chunks
- one native process handles the batch
- native worker threads do the download + histogram work
- Python is only orchestration, not the hot path

### Core idea

Instead of:

- `1 chunk = 1 Lambda = 1 download = 1 native process`

do:

- `1 batch = N chunks = 1 Lambda = 1 native process`

Inside that one native process:

- threads pull chunk work from a queue
- each thread downloads a chunk directly
- each thread histograms that chunk directly
- each thread emits one per-chunk histogram result

That saves:

- Lambda startup overhead
- handler setup overhead
- repeated clip reads
- repeated process launch overhead

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

The handler creates presigned URLs for chunk objects, then the native binary
downloads them via HTTPS.

Pros:

- no AWS auth code in the binary
- easier to reuse across binaries
- easier to test locally
- compatible with ordinary HTTP range requests

Cons:

- one presign step in Python
- still need a robust native HTTP client

Recommendation:

- use presigned URLs plus native HTTP for the first serious experiment

Reason:

- it keeps AWS-specific auth out of the C code
- it is easier to reuse later in raster/finalize experiments
- it keeps the native binary focused on I/O + compute, not credential logic

### Packaging concern

This choice adds a real packaging question.

If the native binary uses `libcurl` for HTTPS, the deploy story must account for:

- `libcurl`
- TLS dependencies such as OpenSSL or the platform TLS stack
- ARM64 Lambda compatibility
- binary size / layer size
- whether the binary is:
  - dynamically linked against a dedicated layer
  - or statically linked as part of the build

This is not optional detail.

Before implementation, choose one explicit packaging model and test it in the
same deploy-style ARM64 runtime used elsewhere in this repo.

Preferred first check:

- prove that a tiny ARM64 Lambda-side binary using presigned HTTPS + `libcurl`
  can be built, packaged, invoked, and smoke-tested cleanly

If that turns into dependency pain, revisit:

- whether native HTTP is still worth it for the first pass
- or whether a cheaper batching experiment should come first

## Threading Model for Hist

### First native hist version: thread-per-chunk work queue

Do **not** start with per-chunk section sharding.

Start with:

- one chunk descriptor per work item
- one shared queue
- worker threads pop chunk jobs
- each worker:
  - downloads one whole chunk
  - histograms it
  - writes one in-memory result record
  - moves to the next chunk

This is the simplest useful design and already removes a lot of overhead.

### Why this is the right first step

- much simpler than intra-chunk range sharding
- preserves the existing per-chunk histogram artifact shape
- makes batching easy
- good reuse potential for other pipeline steps
- probably enough to show whether native I/O batching is worth continuing

### Second native hist version: section sharding inside one chunk

If chunk objects are individually large enough that one chunk is still a
bottleneck, then add a second layer:

- split one chunk into solve-aligned byte ranges
- assign those ranges to multiple threads
- each thread downloads its own HTTP range
- each thread computes a private histogram for its range
- reduce the per-thread histograms at the end

This is more complex and should be a v2 optimization, not the first pass.

## Solve-Aligned Chunk Sectioning

For a given degree:

- `solve_bytes = degree * 2 * sizeof(float)`

Any per-thread byte range must:

- start on a multiple of `solve_bytes`
- end on a multiple of `solve_bytes`

That guarantees:

- no partial solve decoding
- no cross-thread record overlap

So the user’s idea is valid, but it should be treated as:

- a second-phase optimization for very large chunks

not the very first native batching pass.

## Batch Shape

### Important point

The goal is not to load the entire batch into memory before doing anything.

The goal is:

- bounded in-memory concurrency
- download + compute overlap

### Batch descriptor

Each hist Lambda should receive a batch like:

- `batch_id`
- `chunks[]`
  - `chunk_idx`
  - `bin_key`
  - `byte_size`
  - `hist_out_key`

### In-memory execution model

Within one invocation:

- maintain a bounded number of in-flight chunk downloads
- as soon as one chunk is fully available to a worker, histogram it
- upload results after the native phase completes

### Memory budgeting

The safe limit should be computed from:

- Lambda memory
- fixed Python/native overhead
- per-chunk resident bytes
- thread-local scratch
- safety margin

For the first pass, keep it simple:

- process chunks one at a time per worker
- bounded worker count
- no attempt to hold dozens of full chunks in RAM simultaneously

That means memory stays roughly:

- `threads * chunk_bytes`
- plus fixed overhead

not:

- `batch_count * chunk_bytes`

### Practical rule

Use a budget like:

- `max_resident_chunk_bytes <= 35% to 45% of Lambda memory`

and derive:

- worker count
- in-flight chunk count
- max batch size

from that.

## Workflow / Step Functions Plan

### Current

- `ColorSolveScoreHistMap` iterates `chunk_items`
- one item per chunk

### New

Change the plan to emit:

- `hist_chunk_batches`

Each batch contains:

- a small list of chunk descriptors

Then `ColorSolveScoreHistMap` iterates batches, not chunks.

Each batch Lambda:

- downloads/handles multiple chunks
- produces multiple histogram JSON artifacts

This keeps:

- the merge contract the same
- one histogram artifact per original chunk

while reducing:

- number of Lambda invocations
- number of Step Functions transitions

## Cheap Staging Experiment Before Native HTTP

Before committing to native HTTP, there is a cheaper test that isolates the
main hypothesis:

- does batching multiple chunks into one Lambda invocation help materially on
  its own?

### Stage 0 experiment

Implement a temporary batched hist path that still uses the current Python
download model:

- one hist Lambda invocation receives `N` chunks
- Python processes them sequentially in one invocation
- for each chunk:
  - download to tmpfile
  - run the existing one-chunk binary
  - collect/upload that chunk histogram

This experiment is intentionally boring.

It does **not** try to fix Python hot-path overhead completely.

It only answers:

- how much of the current slowdown comes from
  - per-invocation startup
  - per-invocation setup
  - repeated clip fetch / process launch / Step Functions overhead

### Why this stage matters

If this stage already gives a meaningful win, then:

- the batching hypothesis is real
- native HTTP is justified as the next step

If this stage gives little or no win, then:

- the main bottleneck is probably not invocation count alone
- and native HTTP may not be worth the added packaging complexity

### Success threshold for Stage 0

A rough threshold:

- if Python-batched sequential hist cuts wall time enough to be clearly visible
  on the same render, proceed to native HTTP
- if it barely moves, stop and profile before adding `libcurl`

## Handler shape

Add a dedicated batched hist handler path inside
[lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
or split it into a new handler if that keeps the code cleaner.

Preferred first shape:

- keep the same handler
- add a new phase such as:
  - `hist_batch`

The handler would:

1. load the clip artifact once
2. presign all chunk keys in the batch
3. invoke one native binary for the whole batch
4. upload the resulting per-chunk hist JSONs
5. report batched perf

## Native hist binary shape

Recommended first step:

- add a new binary instead of overloading the current CLI too much

Suggested file:

- `lambda/solve_proximity_hist_batch.c`

Reason:

- it keeps the current tested one-chunk CLI stable
- avoids turning `solve_proximity_stats.c` into an awkward multi-mode transport tool
- makes experiment rollback easy

### Native batch input

Pass one small JSON manifest path or JSON string describing:

- metric
- clip bounds
- omega settings
- degree
- threads
- chunk descriptors:
  - `chunk_idx`
  - `url`
  - `size`

### Native batch output

Emit one JSON summary to stdout containing:

- `threads`
- `chunks_processed`
- `bytes_downloaded`
- `download_ms`
- `compute_ms`
- per-chunk histogram payloads in memory-friendly compact form

The Python handler then uploads those per-chunk hist JSON objects to S3.

## Native HTTP implementation

### First recommendation

Use an HTTP client that supports:

- regular GET
- range GET
- connection reuse
- concurrent transfers

Examples:

- `libcurl` with multi interface

Why:

- mature
- straightforward for presigned HTTPS
- supports the exact concurrent range/read patterns we care about

### First version behavior

Do not overreach.

For v1 native hist batching:

- one whole-object GET per chunk
- one chunk owned by one worker at a time
- connection reuse across the batch

Only add range GET section sharding after that baseline is measured.

## How This Reuses for Other Stages

This is the most important systems payoff.

If native presigned-URL I/O works for hist, the same pattern can be reused for:

- raster chunk input
- palette chunk input
- maybe finalize input reads

So hist is a useful proving ground for:

- native HTTP in Lambda
- batched work per invocation
- shared thread pools
- measured download/compute overlap

## Logging Requirements

This experiment must report enough detail to be credible.

### Hist progress logs

For batched hist:

- `threads=<N>`
- `batch_chunks=<K>`
- `bytes=<...>`
- `input=native_http_batch`

Example:

```text
Solve score: hist 24/125 batches · wall=18.6s aggregate=download 41.2s + compute 33.7s · threads=8 · batch_chunks=4 · input=native_http_batch
```

### Final hist perf fields

Per batch result data should include:

- `threads`
- `chunks_processed`
- `bytes_downloaded`
- `download_ms`
- `compute_ms`
- `upload_ms`
- `input_mode=native_http_batch`

Optional but strongly recommended:

- `download_mb_per_s`
- `avg_chunk_download_ms`
- `avg_chunk_compute_ms`

## Benchmark Plan

Benchmark these in order:

1. current baseline
   - one chunk per Lambda
   - Python download
   - tmpfile input

2. current stdin experiment
   - one chunk per Lambda
   - Python download
   - stdin input

3. native hist batch v1
   - multiple chunks per Lambda
   - native whole-object GET
   - one chunk per worker at a time

4. native hist batch v2, only if needed
   - intra-chunk range sharding

## Success Criteria

The experiment is worth continuing if native hist batching gives:

- clear wall-time reduction on the same render
- lower total chunk overhead per solve-score hist phase
- enough gain to justify reusing the model in raster

Reasonable target:

- at least `2x` faster hist wall time

If it cannot beat the current tmpfile baseline convincingly, stop there.

## Implementation Order

1. update the render plan to support hist batches
2. add a cheap Stage 0 batched hist path:
   - multiple chunks per Lambda
   - current Python download model
   - current one-chunk native binary reused sequentially
3. add detailed logging for Stage 0 and benchmark it against the current
   tmpfile baseline
4. if Stage 0 shows a real win, choose and validate the native HTTP packaging
   model
5. add the native batch hist binary
6. add presign plumbing in the handler
7. replace Stage 0 with native batched hist
8. benchmark against the current tmpfile baseline
9. only then decide whether to extend the native I/O model to raster

## Immediate A/B: solve-score hist input mode

Before broader architecture changes, there is one small, self-contained
optimization worth measuring directly:

- remove the `/tmp` write/read round-trip from the solve-score `hist` phase

Today the path is:

1. Python downloads a chunk from S3 to `/tmp/solve_prox_input.bin`
2. Python launches `solve_proximity_stats /tmp/solve_prox_input.bin ...`
3. the C binary opens that file and reads the whole chunk back into RAM
4. pthread workers compute on the in-memory buffer

That means:

- one S3 download per hist Lambda
- one local disk write per hist Lambda
- one local disk read per hist Lambda

The threads themselves do not download anything.

### Goal

Add an explicit benchmark toggle for solve-score hist input mode:

- `tmp file`
- `stdin`

The user must be able to run both modes from the UI and compare logs.

### Why make it user-selectable

This is a performance experiment.

Do not silently switch the codepath.

We want:

- same compute job
- same render settings
- same thread count
- one changed variable: hist input mode

That means the mode must be:

- visible in the popup
- visible in progress logs
- visible in final perf summaries

## UI plan for hist input mode

### Generate

`Render -> Color -> Generate` should only open a popup when:

- `color_mode === solve_score`

For all other color modes:

- `Generate` should keep its current direct behavior

When `color_mode === solve_score`, `Generate` opens a small popup with:

- `Histogram input`
  - `tmp file`
  - `stdin`
- short summary of current baseline thread usage:
  - `solve score threads=1`
  - `raster threads=1`

This popup is a baseline A/B chooser, not a full MT control surface.

### Generate-MT

Keep the existing `Generate-MT` popup, but add one more control:

- `Histogram input`
  - `tmp file`
  - `stdin`

So the MT popup becomes:

- `Solve score threads`
- `Raster threads`
- `Histogram input`

### Logging

All solve-score hist progress logs must include:

- `threads=<N>`
- `input=tmpfile` or `input=stdin`

Examples:

- `Solve score: hist 324/500 · wall=111.9s aggregate=dl 1001.0s + compute 19.3s · threads=7 · input=tmpfile`
- `Solve score: hist 324/500 · wall=104.2s aggregate=dl 812.4s + compute 19.0s · threads=7 · input=stdin`

Final phase/perf summaries should also include the input mode.

## Backend parameter plan

Add a new render parameter:

- `solve_score_hist_input_mode`

Allowed values:

- `tmpfile`
- `stdin`

Default:

- `tmpfile`

Reason:

- baseline remains the current shipped behavior
- experiment is explicit

### Parameter plumbing

Thread this parameter through:

- `index.html`
- `lambda/handler_render_plan.py`
- `stepfunctions/render_workflow.asl.json.template`
- `lambda/handler_solve_proximity.py`

It only matters for solve-score phases, especially `hist`.

It may be passed through `clip` and `summary` too for consistency, but
the primary benchmark target is `hist`.

## Native/C implementation plan

### Current limitation

[lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)
currently requires a seekable file path:

- reads `argv[1]`
- `fopen()`
- `fseek()/ftell()`
- `fread()`

So a pipe cannot work with the current code.

### New input contract

Teach the binary to accept:

- normal file path
- `-` meaning `stdin`
- optional `--input_size=BYTES`

### `stdin` mode behavior

Do not implement stdin mode as an unbounded `realloc` growth loop unless
there is no size information.

Preferred behavior:

1. the handler gets the S3 object `Content-Length`
2. the handler passes `--input_size=BYTES`
3. when `argv[1] == "-"`, the binary:
   - `malloc()`s exactly once
   - reads stdin into that preallocated buffer
   - validates that the expected byte count was fully read

Fallback behavior:

- if `--input_size` is absent, use a growable buffer path

So the primary path is:

1. preallocate from `--input_size`
2. read stdin fully into RAM
3. validate total byte count
4. compute `nSolves` from `degree`
5. run the existing threaded compute on that buffer

Important:

- keep the file-path mode unchanged
- do not fork a second binary
- keep one codepath after the buffer is in memory

### Why this is safe

This does not change:

- metric math
- histogram math
- thread partitioning
- JSON output schema

It only changes how the input chunk gets into RAM.

## Python/Lambda implementation plan

### `tmpfile` mode

Keep the current behavior:

1. download S3 object to `/tmp/solve_prox_input.bin`
2. launch the binary with that path

### `stdin` mode

In [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py):

1. call `s3.get_object(...)`
2. read `Content-Length`
2. start `subprocess.Popen(...)` with:
   - `stdin=subprocess.PIPE`
   - `stdout=subprocess.PIPE`
   - `stderr=subprocess.PIPE`
3. launch the binary as:
   - `solve_proximity_stats - --input_size=<Content-Length> --mode=hist ...`
4. stream `Body.iter_chunks(...)` into child stdin
5. close stdin
6. collect stdout/stderr

This removes:

- the `/tmp` input file write
- the C-side reopen of that file
- the C-side reread from disk

It does **not** remove:

- one S3 download per hist Lambda
- the shared clip JSON fetch

## Expected outcome

This is an incremental optimization, not a full architectural fix.

It should reduce:

- per-worker local I/O
- per-worker `dl_ms`
- maybe phase wall time

It will not solve the bigger issue that:

- hist is still one Lambda per chunk
- and therefore still strongly S3/object-overhead bound

So success criteria are modest:

- measurable reduction in aggregate hist `dl`
- measurable reduction in hist wall time on the same job/settings
- no correctness drift

## Tests

### Native / unit

Add tests for `solve_proximity_stats`:

- file-path input still works
- stdin input produces identical JSON output for the same chunk
- both `clip` and `hist` keep existing behavior

### Lambda handler

Add handler tests for:

- `solve_score_hist_input_mode=tmpfile`
- `solve_score_hist_input_mode=stdin`
- invalid mode rejected

The handler tests should assert:

- tmpfile mode uses `_download(...)`
- stdin mode uses child stdin streaming
- returned progress/result payload includes the input mode

### Frontend

Extend [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh):

- `Generate` opens popup only in solve-score mode
- popup includes hist input selector
- `Generate-MT` popup includes hist input selector
- orchestrator payload includes `solve_score_hist_input_mode`
- log lines include `input=...`

## Implementation order

1. add binary stdin support
2. add handler support and progress metadata
3. add popup control in `Generate-MT`
4. add conditional popup for `Generate` in solve-score mode
5. add logging
6. add regression tests
7. benchmark `tmpfile` vs `stdin`

## Decision rule

After benchmarking:

- if `stdin` gives a clear win, make it the default
- but only after the benchmark path has been validated in the UI and logs

Until then:

- keep `tmpfile` as the default baseline
- keep the input mode explicit

## Where the Time Actually Goes

For `Render -> Color -> Generate` in `solve_score` mode there are two expensive areas:

1. solve-score prepass
   - `clip`
   - `hist`
   - `merge`

2. raster
   - per-root projection
   - viewport clipping
   - pixel claim
   - color/bin emission

The subprocess raster version only partially attacked (2), and attacked it badly.

The proper next version must handle:

- true threading inside [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)
- true threading inside a new
  [lambda/roots2pix_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix_mt.c)

## Revised Architecture

## 1. Solve-score prepass: native threads

Do not create subprocess workers for solve-score phases.

Instead:

- extend [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)
  with `--threads=N`
- keep one process
- split solve ranges across pthread workers
- use shared input buffer
- use per-thread accumulators / output slices
- reduce once at the end

### Clip phase

For `clip`:

1. download the lores `.bin` once
2. split solves into contiguous ranges
3. each thread computes solve scores for its range into a shared `double scores[n]`
4. after join, one thread computes:
   - min
   - max
   - quantiles
   - clip range

This is the cleanest way to get exact clip statistics without per-thread approximation.

### Hist phase

For `hist`:

1. download one chunk `.bin` once
2. split solves into contiguous ranges
3. each thread:
   - computes solve scores for its range
   - applies clip normalization / omega
   - increments a private histogram
4. reduce the private histograms into the final histogram

### Merge phase

`merge` is not the big CPU problem.

Leave it single-threaded initially unless profiling says otherwise.

It should still report:

- `threads=1`

for consistency.

### Implementation note

Do not create a second `solve_proximity_stats_mt` binary unless needed.

Preferred shape:

- one binary
- one codepath
- `--threads=1` for baseline
- `--threads=N` for MT

That keeps behavior drift smaller.

## 2. Raster: true native threads

The subprocess fan-out in
[lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)
must be removed or fully replaced.

The new path is:

- one chunk download
- one process
- true native threads inside `roots2pix_mt`
- one set of final `.pix` / `.pbx` outputs

### New binary

Add:

- [lambda/roots2pix_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix_mt.c)

Also extract shared logic from
[lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
into a common header, for example:

- `lambda/roots2pix_core.h`

so single-thread and MT do not diverge unnecessarily.

### Required execution model

For one chunk:

1. read the chunk `.bin` once
2. read saved-palette bins once if needed
3. split solve indices across threads
4. each thread processes its solve range in place
5. threads write into shared tile state
6. after join, one thread serializes the final `.pix` / `.pbx` files

### Shared raster state

Use shared per-tile state in memory:

- occupancy bitset as `uint64_t[]`
- packed RGB buffer per pixel
- optional pixel-bin buffer per pixel when `emit_pixel_bins` is enabled

Suggested shape:

- `tile_occ_words`
- `tile_rgb`
- `tile_bin`

### Solve-score thread-safety prerequisite

For `color_mode=solve_score`, raster workers will call the solve-score
metric path concurrently.

That requires [lambda/solve_score.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_score.h)
to remain thread-safe under concurrent calls.

The current expectation is:

- no shared mutable global scratch state
- only per-call stack buffers or local heap allocations

This should be treated as an explicit implementation check, not an
implicit assumption.

### Pixel claim semantics

Speed matters more than repeatability.

We do **not** need exact parity with single-thread winner order.

But we **do** need:

- no undefined behavior
- no torn writes
- the final color must be the color of a real root that hit that pixel

So:

- do not use racy plain writes
- do use atomic bit claims on the occupancy word

Recommended rule:

1. compute pixel index
2. atomically `fetch_or` the bit
3. if this thread set the bit first:
   - write packed RGB
   - write pixel bin if enabled
4. otherwise:
   - skip

That gives:

- one winner per pixel
- winner is always a real root
- no per-thread `.pix` streams
- no merge pass

### Serialization

After all worker threads finish:

- scan the final tile state once
- emit exactly one `.pix` record per occupied pixel
- emit exactly one `.pbx` byte per occupied pixel when enabled

This keeps:

- finalize unchanged
- fast Color RePalette unchanged

## 3. The Lambda path

`Generate-MT` should still use:

- [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)

but that handler must become a thin wrapper over the native MT binary.

It should no longer:

- slice chunk files into worker subfiles
- create per-worker saved-bin files
- launch multiple `roots2pix` subprocesses
- concatenate worker outputs

Instead it should:

1. download input once
2. invoke `roots2pix_mt`
3. upload final chunk/tile outputs
4. report perf

So the handler name can stay, but the implementation must be replaced.

## UI / UX

`Render -> Color` keeps:

- `Generate`
- `Generate-MT`

`Generate-MT` popup must become more explicit.

### Popup fields

For now it should include:

- `Solve-score threads`
- `Raster threads`

Defaults:

- if color mode is `solve_score`: both default to the same value
- otherwise only raster threads matter, but the popup can still show both

If we want a simpler v1 UI, allow:

- one `Threads` field

and use it for both solve-score and raster.

But the backend/logging must still report per-phase thread counts separately.

### Supported modes for native MT

First native MT target set:

- `solve_score`
- `saved_palette`
- `constant`
- `rainbow` with `match=none`

Stay single-thread for now:

- `proximity`
- `rainbow` + `match=greedy`
- `rainbow` + `match=hungarian`

## Logging requirements

This is mandatory.

The old logs were not good enough.

### Rule

Every timed phase must report:

- wall seconds
- summed worker seconds where applicable
- thread count

### Solve-score phase logs

For:

- `Solve score: clip`
- `Solve score: hist`
- `Solve score: merge`

log:

- `threads=<N>`

Examples:

```text
Solve score: clip 1/1 · wall=3.2s · total=download 0.4s + compute 2.8s · threads=8
Solve score: hist 24/100 · wall=18.6s · total=download 3.7s + compute 41.9s · threads=8
Solve score: merge 1/1 · wall=0.9s · total=download 0.2s + compute 0.7s · threads=1
```

### Raster logs

The current compact acronyms are not good enough.

Keep the new multi-line block and make it authoritative.

Required fields:

- engine
- threads
- chunks
- wall time
- download time
- native raster time
- upload time
- emitted root hits
- clipped roots
- tile files uploaded
- pixel-bin files uploaded

### Completion logs

All Color-family operations must end with elapsed seconds:

- `Render complete: ... (12.3s)`
- `Color RePalette complete: ... (2.1s)`
- `Autolevels complete` still goes through the generic render completion log, so it must include elapsed seconds too
- `GenerateFromPalette` ends as a normal color render, so it must include elapsed seconds too

## Perf fields

## Raster perf fields

Both raster handlers should report:

- `engine`
- `threads`
- `download_us`
- `native_us`
- `upload_us`
- `roots_plotted`
- `roots_clipped`
- `tiles_uploaded`
- `pixel_bin_tiles_uploaded`

Optional deeper fields from the native JSON are strongly recommended:

- `transform_us`
- `score_us`
- `project_us`
- `claim_us`
- `serialize_us`

Those will matter once `roots2pix_mt` exists, because we will want to know
whether the time is really in:

- score/bin lookup
- projection
- atomic claim
- serialization

## Solve-score perf fields

For `handler_solve_proximity.py`, each phase result should report:

- `threads`
- `dl_ms`
- `compute_ms`
- `upload_ms` where applicable

This should come from the binary result if possible, otherwise from the handler.

## Concrete file changes

### New files

- [lambda/roots2pix_mt.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix_mt.c)
- `lambda/roots2pix_core.h`
- [tests/test_roots2pix_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_roots2pix_mt.py)

### Existing files to change

- [lambda/handler_raster_mt.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster_mt.py)
  - replace subprocess fan-out with one native MT invocation
- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
  - extract shared core
- [lambda/solve_proximity_stats.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_proximity_stats.c)
  - add native threading
- [lambda/handler_solve_proximity.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
  - pass thread count through
  - report threads in result data
- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
  - plan fields for solve-score threads and raster threads
- [stepfunctions/render_workflow.asl.json.template](/Users/nicknassuphis/karpo_hackathon/polypaint/stepfunctions/render_workflow.asl.json.template)
  - thread fields threaded through the solve-score and raster phases
- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
  - `Generate-MT` popup fields
  - phase logging with thread counts
  - render completion logs with elapsed seconds
- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
  - compile `roots2pix_mt`
  - keep `-pthread`
- [docs/testing.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/testing.md)

## Lambda sizing

Keep a separate MT raster Lambda.

That part of the earlier plan was still right.

But tune it for the native implementation, not the subprocess hack.

Initial guidance:

- `RASTER_MT_MEMORY >= 4096`
- benchmark `threads=2`, `4`, `6`, `8`

The chosen default should come from actual measurements on the native code, not
from the previous subprocess version.

## Tests

## New required tests

### Solve-score MT tests

Add tests that verify:

- `--threads=1` and `--threads=N` produce the same clip outputs
- `--threads=1` and `--threads=N` produce the same hist outputs
- per-phase result data includes `threads`

### Raster MT tests

Add tests that verify:

- `roots2pix_mt` produces valid `.pix` output
- `roots2pix_mt` produces valid `.pbx` output when enabled
- `threads=1` and `threads=N` both work
- emitted colors always come from a real root hit
- no duplicate final `.pix` entries for one pixel in one chunk/tile output

Do **not** require exact winner parity with single-thread output.

### Frontend tests

Extend the JS harness so it asserts:

- `Generate-MT` popup exposes thread settings
- solve-score phase logs include `threads=`
- raster perf block includes `threads=`
- all Color-family completion logs include elapsed seconds

## Recommendation

Do not spend more time tuning the subprocess version.

Treat it as discarded work.

The next real implementation should be:

1. native threads inside `solve_proximity_stats`
2. native threads inside `roots2pix_mt`
3. one-process MT raster handler
4. explicit per-phase thread telemetry
5. benchmark against the single-thread baseline

That is the first version that has a real chance of making `Generate-MT`
meaningfully faster.
