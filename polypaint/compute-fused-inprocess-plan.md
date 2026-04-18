# Compute fused path: in-process fusion of param_gen + coeffgen + solve

Status: not implemented.

## Scope

The existing fused compute pipeline
([compute-fused-chunk-plan.md](compute-fused-chunk-plan.md))
collapsed three separate Lambda invocations (param_gen, coeffgen, solve) into
one Lambda per chunk. Inside that single Lambda, however, the three stages
still run as three separate subprocesses of the same binary
(`sweep_coeffgen` twice + `sweep_mt`/`sweep_cm`/`sweep` once), each writing
its output to `/tmp` and the next stage reading it back. Every spawn pays
fork + exec + ELF load + thread-pool init (~150–300 ms), and every
stage-to-stage handoff pays a `/tmp` write + `/tmp` read of 1–50 MB.

This plan collapses those three subprocesses into **one subprocess** that
runs all three stages back-to-back, holding the intermediate buffers in RAM
across stage boundaries. P, C, and R are still written to `/tmp` and
uploaded to S3 — the render pipeline's multispan reader needs all three —
but inter-stage data flow is RAM-to-RAM, and the Python handler overlaps
uploads with subsequent stage compute.

This plan does **not** cover:

- Warm-start across adjacent chunks (separate plan).
- Skipping the params/coeffs S3 uploads (they persist for downstream).
- Changes to the outer fused_chunk_pipeline workflow / ASL.
- Changes to the classic (non-fused) compute path.

## Expected payoff

- **Subprocess startup saved:** likely ~2 × 150–300 ms of billed per-chunk
  overhead, depending on cold/warm process state and binary load cost.
- **Inter-stage `/tmp` read/write skipped:** likely 10–50 MB of I/O per chunk.
- **Parallelized uploads:** P + C + R upload in parallel instead of serially.
  This is a secondary gain in Phase 3 and a larger gain only once Phase 4
  overlaps P/C upload with later compute.
- **Main expected win:** remove two process boundaries and the P→C / C→R
  local-file reread path. This should improve the fused path materially, but
  the gain is bounded if solve dominates chunk time.
- **Billed time saved per chunk:** treat 8–20 % as the realistic first-cut
  target for production-sized chunks; anything above that should be measured
  and demonstrated, not assumed.
- **Memory claim:** the existing sizer in `compute_fused.py` is the starting
  model for peak co-resident buffers, but the implementation must verify real
  peak RSS and `/tmp` usage during rollout before claiming parity with the
  current path.

## Measurement-first gate

Before Phase 1 lands, collect a baseline from the current
[lambda/handler_compute_chunk_fused.py](lambda/handler_compute_chunk_fused.py)
telemetry on a fixed corpus of representative chunks.

Use at least:

- one small chunk where handler/process overhead is a large fraction of total
  time
- one medium chunk from the common case
- one large solve-heavy chunk
- at least two solver modes if both `aberth_mt` and `companion_matrix` are
  used in practice

Record, per chunk:

- Lambda billed duration / task wall time
- `param_gen_us`
- `coeffgen_us`
- `solve_us`
- `upload_params_us`
- `upload_coeffs_us`
- `upload_roots_us`
- output byte sizes for P, C, R

The acceptance standard for this plan is not "the code got simpler" or
"fewer subprocesses exist." The acceptance standard is:

- byte-identical P, C, and R versus the current path
- lower p50 and p90 chunk wall time on the fixed corpus
- no increase in OOM, `/tmp` exhaustion, or timeout rate

If the measured gain on the fixed corpus is negligible, stop after the
refactor/parity work and do not continue into more invasive rollout steps.

### Collection method

Do not make this baseline a manual CloudWatch or DDB reading exercise.
Add a small harness script:

- `scripts/compute_fused_baseline.py`

The script's job is:

- run the fixed corpus through the current fused path
- collect the handler-emitted timing fields
- compute p50 / p90 per configuration
- write a machine-readable artifact such as
  `tests/fixtures/compute_fused_baseline.json`

The promotion comparison for Rollout 2 should read this fixture and compare
the new path against it mechanically. No rollout decision should depend on
someone eyeballing logs.

## Current state

Files in scope:

- [lambda/handler_compute_chunk_fused.py](lambda/handler_compute_chunk_fused.py)
  - `handle_fused_chunk` orchestrates three subprocess calls at
    L107–115 (`_run_param_gen_local`), L158–166 (`_run_coeffgen_local`),
    L194–201 (`_run_solve_local`).
  - Each stage writes to a `/tmp` path and uploads to S3.
  - Uploads run synchronously and block the next stage.
  - `_s3_size_matches` HEAD checks at L92–100, L135–144 support resume.
- [lambda/sweep_cli.c](lambda/sweep_cli.c) (compiled as `sweep`, `sweep_coeffgen`)
  - Mode dispatch at L6100–6134.
  - `runParamGen` at L6124 handles the `param_gen` mode.
  - `runCoeffGenChunked` at L6129 handles the `coeffgen_chunked` mode.
  - `runSolveFromCoeffs` at L6114 handles the `solve` mode.
  - Each of these reads a JSON spec, optionally reads input files from disk,
    allocates output buffers, writes them to the single `outPath`
    CLI argument, and emits a stdout JSON meta.
- [lambda/sweep_mt.c](lambda/sweep_mt.c) — MT Aberth-Ehrlich solver,
  currently invoked as a separate binary in `solver_mode="aberth_mt"`.
- [lambda/sweep_cm.c](lambda/sweep_cm.c) — companion-matrix solver,
  separate binary for `solver_mode="companion_matrix"`.

Current flow for one chunk:

```
handler spawns subprocess 1: sweep_coeffgen (mode=param_gen)
  spec via stdin; writes P to /tmp/fused_params_N.bin; meta on stderr
handler uploads P to S3 (blocking)
handler spawns subprocess 2: sweep_coeffgen (mode=coeffgen_chunked)
  spec via stdin; reads P from /tmp; writes C to /tmp/fused_coeffs_N.bin
handler uploads C to S3 (blocking)
handler removes /tmp/fused_params_N.bin
handler spawns subprocess 3: sweep_mt/sweep_cm/sweep (mode=solve)
  spec via stdin; reads C from /tmp; writes R to /tmp/fused_roots_N.bin
handler uploads R to S3 (blocking)
handler removes /tmp/fused_coeffs_N.bin, /tmp/fused_roots_N.bin
handler returns status
```

## Target state

One subprocess per chunk. The solver modes (`aberth_mt`, `companion_matrix`,
classic `aberth`) are selectable inside the fused binary — `sweep_cli` grows
a small dispatch over the three solver back-ends so the fused path doesn't
need to choose between three binaries. Uploads are parallelized via a
Python `ThreadPoolExecutor` and (optionally in Phase 4) overlapped with
subsequent stage compute via a small stage-signal protocol on the
subprocess's stderr.

```
handler spawns ONE subprocess: sweep_cli (mode=fused)
  spec via stdin; produces three /tmp files in order:
    1. /tmp/fused_params_N.bin       → stderr: {"stage":"params","bytes":...}
    2. /tmp/fused_coeffs_N.bin       → stderr: {"stage":"coeffs","bytes":...}
    3. /tmp/fused_roots_N.bin        → stderr: {"stage":"roots","bytes":...}
  final stderr emits a consolidated meta JSON summing param_gen_us,
  coeffgen_us, solve_us, avg_iterations, degree, n_t, skipped_overflow, etc.

handler thread reads subprocess stderr line by line:
  - on "stage":"params"  → submit P upload to executor
  - on "stage":"coeffs"  → submit C upload to executor
  - on "stage":"roots"   → submit R upload to executor
handler waits for subprocess to exit
handler waits for all three upload futures
handler returns status
```

## Memory and `/tmp` contract

For one chunk with `step_count = N`, `degree = d`, `n_coeffs = k`:

| buffer | size             | typical example (N=100k, d=7, k=8) |
|--------|------------------|------------------------------------|
| P      | `N × 16` bytes   | 1.6 MB                             |
| C      | `N × k × 8` bytes| 6.4 MB                             |
| R      | `N × d × 8` bytes| 5.6 MB                             |

In the fused binary, the lifetime rule is:

- **P**: allocated in param_gen, needed by coeffgen. Freed after coeffgen
  finishes reading it, at which point P already exists on `/tmp` and its
  S3 upload may or may not have started.
- **C**: allocated in coeffgen, needed by solve. Freed after solve
  finishes reading it, at which point C already exists on `/tmp`.
- **R**: allocated in solve, written to `/tmp`, freed before process exit.

Peak in-RAM data = `max(P + C during coeffgen, C + R during solve)`. This
matches the existing `compute_fused.py::peak_memory_step_bytes` formula
(L105–109), so the current plan-side sizer already budgets for it
correctly. No change to the sizer is required.

`/tmp` peak remains `P + C + R` while all three files are present (before
upload completion). Identical to today.

## Phase 1 — Refactor `sweep_cli.c` to expose in-memory compute cores

Goal: each of the three existing modes keeps its current public entry
point (for the non-fused path, for test harnesses, for any external
caller), but delegates its actual compute to a new function that operates
on in-memory buffers. The fused mode will then chain those cores without
the I/O wrappers.

### Step 1.1 — Extract `computeParamGen`

Current function at roughly `runParamGen` in `sweep_cli.c` (around L5425 based
on the `"mode":"param_gen"` meta string).

- Read the existing `runParamGen(const char *buf, const char *outPath)`.
- Split it into:
  - `int computeParamGen(const ParamGenSpec *spec, double **outParams, size_t *outSize, ParamGenMeta *meta)` — allocates an output buffer via `malloc`, fills it, returns 0 on success. Caller owns `outParams` and must `free` it.
  - `runParamGen(const char *buf, const char *outPath)` — parses spec from JSON buf, calls `computeParamGen`, writes the buffer to `outPath` via `fwrite`, emits the existing meta JSON on stdout/stderr.
- Introduce `ParamGenSpec` struct in a new header `sweep_compute.h`:
  ```c
  typedef struct {
      int n1, n2;
      int times;
      int step_start, step_count;
      int n_threads;
      const char *param_transforms_json;  // raw JSON slice
  } ParamGenSpec;
  ```
  Plus matching `ParamGenMeta` for timing / threads info.

### Step 1.2 — Extract `computeCoeffGen`

Current function: `runCoeffGenChunked` at L6129 dispatch, body defined
earlier.

- Split into:
  - `int computeCoeffGen(const CoeffGenSpec *spec, const double *paramsBuf, size_t paramsSize, double **outCoeffs, size_t *outSize, CoeffGenMeta *meta)` — no file I/O, no JSON parsing.
  - `runCoeffGenChunked(const char *buf, const char *outPath)` — parses spec, reads `params_file` to a buffer, calls `computeCoeffGen`, writes out.
- `CoeffGenSpec` carries `function`, `coeff_transforms_json`, `cfpv_json`,
  `step_start`, `step_count`, `n_threads`. No `params_file` — that's an
  I/O-wrapper concern.

### Step 1.3 — Extract `computeSolve` and choose solver dispatch strategy

Solver lives across three source files (`sweep_cli.c` has a classic Aberth
path, `sweep_mt.c` has multithreaded Aberth, `sweep_cm.c` has companion
matrix). The current fused handler picks the binary by `solver_mode`.

- Decide the dispatch surface: either
  - (a) Link all three solver implementations into `sweep_cli` directly and
    pick via a runtime switch, or
  - (b) Keep three binaries and have `sweep_cli --fused` call the
    appropriate solver as a helper subprocess.
- Preferred end state: (a). It eliminates the last subprocess boundary and is
  the only version that fully realizes the plan title.
- Implementation rule: do not let solver inlining block the rest of the work.
  If linking `sweep_mt.c` and `sweep_cm.c` into `sweep_cli` becomes messy
  because of binary size, symbol collisions, duplicated `main` scaffolding, or
  thread-pool lifecycle problems, stop and measure before forcing it through.
  The plan should only continue with (a) once the build/link surface is shown
  to be tractable.
- Extract:
  - `int computeSolve(const SolveSpec *spec, const double *coeffsBuf, size_t coeffsSize, double **outRoots, size_t *outSize, SolveMeta *meta)` — dispatches internally on `spec->solver_mode`.
  - Keep `runSolveFromCoeffs(const char *buf, const char *outPath)` as the existing file-oriented entry point for the classic path.
- Link `sweep_mt.c` and `sweep_cm.c` as `.o` files into the `sweep_cli`
  build (deploy.sh / Makefile change). Their `main` functions stay as
  standalone binaries for the classic path; their solver kernels get
  exposed via header declarations like:
  ```c
  int aberth_mt_solve_buffer(const double *coeffs, size_t coeffs_bytes,
                             int n_coeffs, int n_steps, int n_threads,
                             double **out_roots, size_t *out_bytes,
                             SolveMeta *meta);
  int companion_matrix_solve_buffer(const double *coeffs, size_t coeffs_bytes,
                                    int n_coeffs, int n_steps,
                                    double **out_roots, size_t *out_bytes,
                                    SolveMeta *meta);
  ```
  Both declared in `sweep_compute.h`. The respective `.c` files grow
  external-linkage functions that wrap their existing internal solvers,
  taking a raw buffer + spec instead of a file path.

### Step 1.4 — Unit tests for the compute cores

New test file `tests/test_sweep_compute_cores.py` that, for each of the
three cores:

- Builds a small deterministic input in Python.
- Runs the existing file-oriented mode (e.g. `sweep_cli` in
  `mode=param_gen`) end-to-end and captures the byte output.
- Runs the new in-memory core through a tiny C-binding test harness
  (`tests/compute_core_probe.c` compiled on the fly — or a subprocess
  driver `sweep_compute_probe` that accepts a spec, runs the core, writes
  the buffer).
- Asserts byte-identical output between the file-oriented and in-memory
  paths.

This is critical: the fused path MUST produce byte-identical P, C, R to
the existing path for any given input.

## Phase 2 — Add `fused` mode to `sweep_cli.c`

Goal: one binary invocation runs all three stages.

### Step 2.1 — Add the dispatch branch

In `sweep_cli.c` at L6134 (end of the mode-dispatch block), add:

```c
if (strcmp(mode, "fused") == 0) {
    int rc = runFused(buf, outPath);
    free(buf);
    return rc;
}
```

### Step 2.2 — Implement `runFused`

New function `static int runFused(const char *buf, const char *outPath)` in
`sweep_cli.c`. `outPath` here is a `/tmp` prefix; the fused mode writes
three files:

- `{outPath}.params.bin`
- `{outPath}.coeffs.bin`
- `{outPath}.roots.bin`

(Alternatively, take three explicit output paths from the spec — see
"Output path convention" below.)

Pseudocode:

```c
static int runFused(const char *buf, const char *outPath) {
    ParamGenSpec pg_spec; /* parse fields from buf */
    CoeffGenSpec cg_spec;
    SolveSpec sv_spec;

    /* Parse the fused spec. The fused spec contains every field from
       all three stages plus three output paths (or one prefix). */

    double *P_buf = NULL; size_t P_size = 0; ParamGenMeta pg_meta = {0};
    if (computeParamGen(&pg_spec, &P_buf, &P_size, &pg_meta) != 0) {
        return 1;
    }
    if (write_file_bytes(P_path, P_buf, P_size) != 0) {
        free(P_buf);
        return 1;
    }
    emit_stage_marker("params", P_size, &pg_meta);

    double *C_buf = NULL; size_t C_size = 0; CoeffGenMeta cg_meta = {0};
    if (computeCoeffGen(&cg_spec, P_buf, P_size, &C_buf, &C_size, &cg_meta) != 0) {
        free(P_buf);
        return 1;
    }
    free(P_buf); P_buf = NULL;

    if (write_file_bytes(C_path, C_buf, C_size) != 0) {
        free(C_buf);
        return 1;
    }
    emit_stage_marker("coeffs", C_size, &cg_meta);

    double *R_buf = NULL; size_t R_size = 0; SolveMeta sv_meta = {0};
    if (computeSolve(&sv_spec, C_buf, C_size, &R_buf, &R_size, &sv_meta) != 0) {
        free(C_buf);
        return 1;
    }
    free(C_buf); C_buf = NULL;

    if (write_file_bytes(R_path, R_buf, R_size) != 0) {
        free(R_buf);
        return 1;
    }
    emit_stage_marker("roots", R_size, &sv_meta);
    free(R_buf);

    /* emit consolidated meta JSON on stdout */
    emit_fused_meta(&pg_meta, &cg_meta, &sv_meta);
    return 0;
}
```

Key details:

- **`emit_stage_marker`**: when present, prints one line of JSON to `stderr`,
  e.g. `{"protocol_version":1,"stage":"params","bytes":1600000,"elapsed_us":12345}\n`.
  The Python handler reads these as they happen and fires upload threads.
  This is a Phase 4 feature and may be omitted in the minimal first cut.
  Rollout 2 must tolerate the absence of stage markers; Rollout 3 must only
  react to markers whose `protocol_version` it explicitly supports.
- **`emit_fused_meta`**: prints one final JSON to `stdout` with all three
  stages' timings, degree, n_t, avg_iterations, skipped_overflow.
- **Memory discipline**: `free(P_buf)` immediately after coeffgen completes;
  `free(C_buf)` immediately after solve completes. Each `free` keeps peak
  RAM at `max(P+C, C+R)`.
- **Error handling**: on any failure, free currently held buffers, emit an
  error line on stderr, return non-zero. Do NOT leave partial output files
  behind — the handler's `finally` block cleans up `/tmp` files, but the
  fused binary should still try to `unlink` any partial output before
  returning to avoid mixing a partial file with a success status if the
  handler's cleanup is racy.

### Step 2.3 — Output path convention

Option A (prefix): `sweep_cli fused_output_prefix` → binary writes
`{prefix}.params.bin`, `{prefix}.coeffs.bin`, `{prefix}.roots.bin`.
Simple, but slightly awkward for the handler which wants to pass exact
paths.

Option B (three explicit paths in the spec JSON):
```json
{
  "mode": "fused",
  "out_params_path": "/tmp/fused_params_42.bin",
  "out_coeffs_path": "/tmp/fused_coeffs_42.bin",
  "out_roots_path": "/tmp/fused_roots_42.bin",
  ...
}
```
With the `outPath` CLI arg ignored (or reused as a fallback). Cleaner.

**Pick Option B.** Aligns better with the existing handler, which already
knows exact paths.

### Step 2.4 — Solver dispatch inside the fused mode

`SolveSpec` carries `solver_mode` ∈ {`aberth`, `aberth_mt`, `companion_matrix`}.
Inside `computeSolve`, dispatch:

```c
if (spec->solver_mode == SOLVER_ABERTH_MT) {
    return aberth_mt_solve_buffer(coeffs, coeffs_size, n_coeffs,
                                  n_steps, spec->n_threads,
                                  out_roots, out_size, meta);
}
if (spec->solver_mode == SOLVER_COMPANION_MATRIX) {
    return companion_matrix_solve_buffer(coeffs, coeffs_size, n_coeffs,
                                         n_steps, out_roots, out_size,
                                         meta);
}
return aberth_solve_buffer(coeffs, coeffs_size, n_coeffs, n_steps,
                           out_roots, out_size, meta);
```

The `aberth_solve_buffer` / `aberth_mt_solve_buffer` /
`companion_matrix_solve_buffer` functions are the in-memory versions of
what `sweep`, `sweep_mt`, `sweep_cm` currently do on files. Implementation
is mostly copying the existing code with the top-level file-reading
replaced by a pointer into the passed `coeffs` buffer.

### Step 2.5 — Deploy / build changes

In [deploy.sh](deploy.sh), find where `sweep_cli`, `sweep_mt`, `sweep_cm`
are compiled. Add `sweep_mt.o` and `sweep_cm.o` as additional objects
linked into the `sweep_cli` (and `sweep_coeffgen` symlink/copy) binary.
Also expose the new `sweep_compute.h` header.

`sweep`, `sweep_mt`, `sweep_cm` keep compiling as their own standalone
binaries for backwards compatibility (classic path and tests).

The fused handler will invoke `sweep_cli` directly in `mode=fused`, not
`sweep_coeffgen` / `sweep_mt` / `sweep_cm`.

## Phase 3 — Rewrite `handler_compute_chunk_fused.py`

Goal: one subprocess call + parallel uploads.

### Step 3.1 — Single subprocess call

Replace `_run_param_gen_local` + `_run_coeffgen_local` + `_run_solve_local`
with a single function `_run_fused(...)` that invokes `sweep_cli` in
`mode=fused`:

```python
def _run_fused(
    *,
    params_path,
    coeffs_path,
    roots_path,
    n, times, step_start, step_count,
    n_threads,
    param_transforms,
    function_name, coeff_transforms, cfpv,
    solver_mode, n_coeffs, degree,
):
    spec = {
        "mode": "fused",
        "out_params_path": params_path,
        "out_coeffs_path": coeffs_path,
        "out_roots_path": roots_path,

        "n1": n, "n2": n, "times": times,
        "step_start": step_start, "step_count": step_count,
        "n_threads": n_threads,
        "param_transforms": list(param_transforms or []),

        "function": function_name,
        "coeff_transforms": list(coeff_transforms or []),
        "cfpv": list(cfpv or []),

        "solver_mode": solver_mode,
        "n_coeffs": n_coeffs,
        "degree": degree,
    }
    proc = subprocess.Popen(
        [SWEEP_CLI, "-"],   # outPath arg is ignored; paths are in spec
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    proc.stdin.write(json.dumps(spec))
    proc.stdin.close()
    stdout, stderr = proc.communicate(timeout=840)
    if proc.returncode != 0:
        raise RuntimeError(f"fused compute failed: {stderr.strip()}")
    return json.loads(stdout)
```

### Step 3.2 — Parallel uploads after subprocess exits

In `handle_fused_chunk`, after `_run_fused` returns:

```python
with ThreadPoolExecutor(max_workers=3) as pool:
    futures = []
    if not reused_params:
        futures.append(pool.submit(
            _upload_file, params_path, params_key,
            metadata=_stage_metadata(stage="params", ...),
        ))
    if not reused_coeffs:
        futures.append(pool.submit(
            _upload_file, coeffs_path, coeffs_key,
            metadata=_stage_metadata(stage="coeffs", ...),
        ))
    futures.append(pool.submit(_upload_file, roots_path, bin_key))
    for future in futures:
        future.result()  # propagates exceptions
```

Three uploads in parallel instead of three in series. On a Lambda network
link this is bounded by throughput, so this is not the primary win of the
plan. Treat it as a secondary reduction in the non-solve portion of chunk
wall time, especially for C and R.

### Step 3.3 — Resume support unchanged

The `_s3_size_matches` HEAD checks at L92 and L135 stay. If either P or
C is already in S3 (resume), set `reused_params` or `reused_coeffs`
accordingly. In the fused spec, add two optional flags:

```json
{
  "reuse_params": true,
  "reuse_coeffs": false,
  "params_file": "/tmp/fused_params_42.bin",   // only when reuse_params=true
  "coeffs_file": "/tmp/fused_coeffs_42.bin"    // only when reuse_coeffs=true
}
```

Binary behavior on resume:

- If `reuse_params=true`, binary reads P from `params_file` instead of
  running `computeParamGen`. Skips the stage marker for params.
- If `reuse_coeffs=true`, binary reads C from `coeffs_file` instead of
  running `computeCoeffGen`. Skips the stage marker for coeffs.
- Solve always runs.

The Python handler downloads P or C from S3 before invoking the binary if
it's resuming that stage. The binary sees a local file just like today.

### Step 3.4 — Metadata and result shape preserved

The binary's `emit_fused_meta` JSON must contain every field the current
handler assembles into `result_data` at L222–249. Specifically:

- `param_gen_us`, `coeffgen_us`, `solve_us`
- `n_t`, `degree`, `avg_iterations`
- `threads` (param gen), `threads` (coeffgen), `n_threads` (solve)
- `skipped_overflow` (solve-only, optional)

The handler threads these into `result_data` unchanged, so the downstream
consumers (`handler_compute_status`, plan finalization) see the same shape.

### Step 3.5 — Error handling

If the binary fails at any stage, it emits `{"error": "..."}` on stderr
and returns non-zero. The handler catches, reports status `"error"`, and
the `finally` block cleans up `/tmp`.

If the S3 upload fails for one of the three files after the binary
succeeded: all three `/tmp` files exist and the other uploads may have
succeeded. Current behavior is "upload fails → exception → handler error
path". With parallel uploads, one failure propagates via `future.result()`
after the other two may have completed. That's fine — the chunk retry
just re-uploads whatever hadn't landed. The HEAD-based resume check picks
this up on retry.

## Phase 4 — Streaming stage completion markers (optional second cut)

Goal: start each upload the moment its file is ready, not after the
subprocess exits. This hides most of the P and C upload time behind
the next stage's compute.

### Step 4.1 — Binary emits stage markers on stderr

In `runFused`, after each `write_file_bytes`, emit a JSON line on stderr:

```
{"protocol_version":1,"stage":"params","path":"/tmp/fused_params_42.bin","bytes":1600000,"elapsed_us":12345}
{"protocol_version":1,"stage":"coeffs","path":"/tmp/fused_coeffs_42.bin","bytes":6400000,"elapsed_us":45678}
{"protocol_version":1,"stage":"roots","path":"/tmp/fused_roots_42.bin","bytes":5600000,"elapsed_us":234567}
```

The final consolidated meta still goes on stdout at process exit.

### Step 4.2 — Handler reads stderr as it happens

Replace `proc.communicate(timeout=...)` with a streaming read loop:

```python
def _run_fused_streaming(proc, s3_uploader, params_key, coeffs_key, roots_key):
    upload_futures = {}
    pool = ThreadPoolExecutor(max_workers=3)
    try:
        while True:
            line = proc.stderr.readline()
            if not line:
                break
            line = line.strip()
            if not line:
                continue
            try:
                event = json.loads(line)
            except ValueError:
                continue
            stage = event.get("stage")
            if stage == "params":
                upload_futures["params"] = pool.submit(
                    s3_uploader, event["path"], params_key, ...
                )
            elif stage == "coeffs":
                upload_futures["coeffs"] = pool.submit(
                    s3_uploader, event["path"], coeffs_key, ...
                )
            elif stage == "roots":
                upload_futures["roots"] = pool.submit(
                    s3_uploader, event["path"], roots_key
                )
        proc.wait(timeout=60)
        stdout = proc.stdout.read()
    finally:
        for fut in upload_futures.values():
            fut.result()
        pool.shutdown(wait=True)
    if proc.returncode != 0:
        raise RuntimeError("fused compute failed")
    return json.loads(stdout)
```

The upload for P starts while coeffgen and solve are still running. For
typical sizes the upload completes during coeffgen, and C's upload starts
right when coeffgen finishes, running alongside solve. R's upload is the
only one that's strictly after its stage finishes.

### Step 4.3 — Measured billed-time cut

Illustrative, not contractual, per chunk, at N=100k, d=7, k=8, solve=~2s:

| metric                          | current | fused Phase 3 | fused Phase 4 |
|---------------------------------|---------|---------------|---------------|
| subprocess startup              | ~600 ms | ~200 ms       | ~200 ms       |
| inter-stage /tmp read/write     | ~50 ms  | ~0 ms         | ~0 ms         |
| P upload (serialized)           | ~80 ms  | ~80 ms (par)  | hidden        |
| C upload (serialized)           | ~200 ms | ~200 ms (par) | hidden        |
| R upload                        | ~300 ms | ~300 ms (par) | ~300 ms       |
| total overhead above compute    | ~1.23 s | ~0.78 s       | ~0.5 s        |

Do not use this table as a go/no-go justification by itself. Promotion must
use the fixed-corpus telemetry gate above.

Phase 4 is optional; Phase 3 captures most of the win with far less
orchestration code.

## Testing strategy

### Unit tests

- `tests/test_sweep_compute_cores.py` (from Step 1.4): byte-identity
  between file-oriented and in-memory modes of param_gen, coeffgen, solve.
  Run with several sizes (N=1, N=100, N=10_000) and several solver modes.
- `tests/test_sweep_cli_fused_mode.py`: run `sweep_cli` in `mode=fused`
  against a small input. Assert that:
  - The three output files exist and have expected sizes
  (P = N×16, C = N×k×8, R = N×d×8).
  - The consolidated stdout meta JSON has the expected shape.
  - Stage markers appear on stderr in order: params, coeffs, roots.
  - The output bytes match what the three legacy modes would produce for
    the same inputs (run the legacy modes and byte-diff).

### Integration tests

- `tests/test_handler_compute_chunk_fused.py` (existing or new): run the
  handler end-to-end against a mocked S3. Assert:
  - Exactly one subprocess is invoked (not three).
  - All three S3 puts happen.
  - `_s3_size_matches` resume path still works (download + skip stage).
  - Error at each stage propagates correctly.

### Docker ARM64 test

Per project convention
([feedback_docker_arm64_tests.md](memory/feedback_docker_arm64_tests.md)),
the new `sweep_cli` binary must be exercised in the ARM64 Docker harness
against a realistic spec. Fused mode output byte-compared against the
legacy sequential mode output. Both must match bit-for-bit.

### Parity harness

New `tests/test_compute_fused_inprocess_parity.py`: for a fixed seed,
run the classic three-subprocess path and the new fused single-subprocess
path, byte-diff each of P, C, R. MUST be identical.

Rules:

- run the byte-identity parity with a fixed PRNG seed and `n_threads=1`
- keep thread count, transforms, and solver mode fixed across both paths
- treat this as the bit-for-bit gate

Do **not** require bit-identity at `n_threads>1` unless the underlying
implementation is shown to be scheduling-deterministic. For multithreaded
runs, add a separate regression check that verifies:

- output sizes are correct
- metadata shape is unchanged
- numeric results are equivalent within an explicitly stated tolerance

Gate the Phase 3 → production rollout on the deterministic (`n_threads=1`)
bit-identity harness passing.

## Rollout

Three independently shippable phases:

### Rollout 1: Phase 1 alone (refactor, no behavior change)

- Extract `computeParamGen`, `computeCoeffGen`, `computeSolve`.
- Keep `runParamGen`, `runCoeffGenChunked`, `runSolveFromCoeffs` as thin
  I/O wrappers over the new cores.
- All existing binaries and paths behave identically. Legacy parity tests
  pass unchanged.
- Unit tests on the cores as gate.
- Exit criterion:
  - core parity tests pass
  - no production path flips
  - no measured performance claim is made yet
  - no deploy-size regression greater than 5% in `sweep_cli` or its Lambda
    package versus the pre-refactor baseline

### Rollout 2: Phase 2 + Phase 3 (native fused mode + single-subprocess handler)

- `sweep_cli` grows `mode=fused`.
- `handler_compute_chunk_fused.py` switches to a single subprocess invocation.
- Parallel uploads via `ThreadPoolExecutor`.
- Behind a plan flag, e.g. `render_execution.compute_exec_mode`:
  - `"subprocess_per_stage"` (current default) — unchanged behavior
  - `"inprocess_fused"` — new path
- Parity harness gates the flip to `"inprocess_fused"` as the default.
- Promotion gate:
  - byte-identical P, C, R on the parity corpus
  - measured p50 and p90 chunk wall time improvement on the fixed telemetry
    corpus
  - no regression in OOM, `/tmp`, or timeout failures during canary
- If these gates are not met, leave `"subprocess_per_stage"` as default and
  do not proceed to broader rollout.

### Rollout 3: Phase 4 (streaming stage markers)

- Optional performance improvement on top of Rollout 2.
- No contract change. No rollback flag needed — this is purely
  binary-side marker emission + handler-side streaming read.
- Can be merged any time after Rollout 2 lands.
- Only do this if Rollout 2 has already shown a real gain. Do not stack the
  extra stderr/upload orchestration complexity on top of an unproven base
  implementation.

## Risks and open questions

- **Solver inlining bloat.** Linking `sweep_mt` and `sweep_cm` into
  `sweep_cli` grows the binary. Current sweep_cli is already large; adding
  two more solver back-ends may push over some practical size threshold
  (Lambda deploy zip, cold-start load time). Worth measuring before
  committing to inlining. If it matters, keep (b) from Step 1.3 — call
  the solver as a helper subprocess from within the fused path while the
  in-memory param/coeff work is validated. That loses the last
  subprocess-spawn saving and is not the final target, but it is a valid
  stop-point if the all-in-one binary turns out to be high-risk.
- **Thread pool lifecycle.** `sweep_mt` initializes a worker thread pool.
  If the fused path runs the solver after coeffgen, and coeffgen also used
  a thread pool, both pools must coexist or be torn down cleanly between
  stages. Investigate whether they share the same pthread pool or need
  separate ones.
- **Memory on extreme chunks.** The sizer is conservative but not
  infallible. If a user explicitly overrides `fused_threads` or
  `n_chunks` below the safe floor, the fused mode might OOM where the
  sequential path (via `/tmp`) would not. Enforce the `min_safe_chunks`
  floor inside the fused binary: if `step_count * peak_step_bytes`
  exceeds the memory budget, refuse to run and emit an error directing
  the user to re-plan with more chunks.
- **Over-claiming the performance win.** The handler already exposes stage
  timings and upload timings today. If the final implementation does not beat
  the current path on the fixed corpus, the plan must stop without trying to
  justify itself by architecture alone.
- **`/tmp` exhaustion vs RAM exhaustion.** Same calculation as above but
  for `/tmp`. The existing `tmp_floor_chunks` term in the sizer handles
  this. Fused mode doesn't change `/tmp` peak (same three files).
- **Error observability.** Today, a failure in coeffgen is a stderr
  message from the `sweep_coeffgen` subprocess. With fused, the same
  message comes from the `sweep_cli` subprocess. The handler's error path
  should still capture and report it the same way. Verify that the
  existing error-logging path works for the new binary.
- **Warm-start future.** This plan sets up the structure (one in-process
  solver call per chunk) that makes cross-chunk warm-start easier to add
  later. Worth noting in the plan that a follow-up will use the last
  step's roots as seeds for the next chunk's first step.

## Summary of changes

| file / area                                     | change                                                   |
|-------------------------------------------------|----------------------------------------------------------|
| `lambda/sweep_cli.c`                            | new `runFused` + `mode="fused"` dispatch; in-memory cores |
| `lambda/sweep_mt.c`, `lambda/sweep_cm.c`        | expose `*_solve_buffer` entry points                      |
| `lambda/sweep_compute.h` (new)                  | shared specs, metas, entry-point declarations             |
| `lambda/handler_compute_chunk_fused.py`         | one subprocess; `ThreadPoolExecutor`; optional streaming markers |
| `deploy.sh` / build                             | link solver objects into `sweep_cli`                      |
| `scripts/compute_fused_baseline.py` (new)       | fixed-corpus baseline collection and p50/p90 dump         |
| `tests/test_sweep_compute_cores.py` (new)       | core parity                                               |
| `tests/test_sweep_cli_fused_mode.py` (new)      | fused-mode output integrity                               |
| `tests/test_compute_fused_inprocess_parity.py` (new) | sequential vs fused bit-identity                     |
| `tests/test_handler_compute_chunk_fused.py`     | integration: single subprocess + parallel uploads         |
