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
