# Code Review 32: CR31 VM Speedup Implementation and Evidence Audit

Date: 2026-07-12

Reviewed range: `32e01ff..9ec68c2`

Implementation HEAD: `9056919` (`CR31 P4/P5: scoped in-place typed ops (F3), hoisted kernels (F7), perf counters (F10), post-mortem`)

Evidence HEAD: `9ec68c2` (`CR31: Graviton benchmark results`)

Review mode: adversarial source audit, commit-by-commit implementation trace, production-entry microbenchmarks, byte-level A/B probes, ThreadSanitizer, stack-usage inspection, benchmark-report audit, and deploy-binary freshness audit

## Verdict

CR31 contains real, useful speedups, but it is not a safe deployment closeout in its current form.

The positive result is substantial: the Coeff argument plan improves the harvested `compute_mqlacwaq` program, blocked I/O improves the multithreaded chunk path, the static Param scheduler removes the eight-worker collapse, and the Solve feature cache improves a deliberately dense same-source pair-metric bundle. The Graviton macrobenchmark values in `cr-31-graviton.md` match the local report files, and the 26 selected finite-workload outputs are byte-identical across the M3 and Graviton reports.

The negative result is more important for deployment:

1. The specialized Coeff kernels changed non-finite output semantics and can emit infinities where the previous evaluator emitted zero.
2. The Solve feature cache makes ordinary single-metric production evaluation approximately 5x to 7x slower and adds about 33 KiB of unconditional per-call clearing plus a large production stack-frame increase.
3. The single-worker chunk path never enters the new blocked-I/O code, so the documents' headline conclusion that batching was tested and proved irrelevant at t1 is false.
4. The new CPU cap can switch a request between two different Param dither seed policies, making output depend on the host's visible CPU count.
5. The new Param static scheduler and the changed Coeff worker contain unsynchronized reads of a concurrently written failure flag.
6. Several binaries used by the claimed final ARM64 Docker regression predate the source commits they were supposed to verify.

The campaign's benchmark and test suite did not catch these defects because its favorable paths and finite fixtures are too narrow. Passing 214 Python tests and producing equal bytes for the selected matrix are both useful facts; neither establishes equivalence for untested numerical policy, scheduler transitions, or production entry points.

Recommendation: do not deploy `9056919` as-is. Fix CR32-F1 and CR32-F2 first. Fix the determinism and race findings before treating the CPU cap and scheduler as production-safe. Then rebuild every deploy binary from the final tree and rerun the corrected benchmark matrix.

## Scope

The review covers all CR31 changes, not only the two result documents.

| Commit | Intended change | Main audited files |
|---|---|---|
| `7ad40be` | Benchmark harness and Coeff load-time argument plan | `scripts/bench_program_vms.py`, `lambda/sweep_cli.c` |
| `eba68ef` | Param lowered plans, Root prepared chains, Solve lag flags | `lambda/sweep_cli.c`, `lambda/root_xforms.h`, `lambda/solve_score.h` |
| `a73c136` | Blocked Coeff chunk I/O and Param static scheduler | `lambda/sweep_cli.c` |
| `5159951` | Solve shared feature cache | `lambda/solve_score.h`, `tests/native/vm_microbench.c` |
| `9056919` | Scoped in-place Coeff operations, fast kernels, counters, post-mortem | `lambda/sweep_cli.c`, `lambda/solve_score.h`, `cr-31-post-mortem.md` |
| `9ec68c2` | Graviton benchmark report | `cr-31-graviton.md` |

The review also checked the ignored benchmark reports under `reports/`, the current native binaries under `lambda/`, the predeploy test list, the Docker runtime regression, and the production call sites that include `solve_score.h` and `root_xforms.h`.

## Evidence Used

The conclusions below are based on the following evidence:

- Source comparison between the pre-campaign baseline `32e01ff` and implementation `9056919`.
- Existing full campaign gate: `214 passed`, one warning, and `63` native subtests.
- Re-execution of exact baseline/current Solve production-entry probes.
- Re-execution and inspection of the exact Coeff overflow A/B outputs.
- Re-execution and inspection of Param dither outputs at t1, t2, and t4.
- A 51-repetition interleaved M3 Param t4 A/B.
- ThreadSanitizer execution of the Param static worker failure path.
- Compiler stack-usage output for current production consumers.
- Inspection of all Graviton JSON reports and their metadata.
- File timestamps and commit timestamps for deploy binaries and changed headers.
- Static tracing of every new branch in `sweep_cli.c`, `solve_score.h`, and `root_xforms.h`.

No finding below depends only on an inferred performance theory. Where a runtime claim is made, the measured values or exact control-flow reason are included.

## Findings Summary

| ID | Severity | Finding |
|---|---|---|
| CR32-F1 | CRITICAL | Coeff fast vector kernels skip the evaluator's non-finite clamp and change output bytes. |
| CR32-F2 | HIGH | Solve feature caching regresses common single-metric production evaluation by about 5x to 7x and materially enlarges stack frames. |
| CR32-F3 | HIGH | Chunked t1 never uses blocked I/O; the principal t1 benchmark conclusion is based on a path that did not execute the optimization. |
| CR32-F4 | HIGH | The CPU-count cap can change Param dither output by selecting a different serial/threaded seed policy. |
| CR32-F5 | MEDIUM | Param and Coeff worker failure flags have C data races. |
| CR32-F6 | MEDIUM | Param's static scheduler writes one row per `pwrite`; the default four-worker path regresses materially on M3 and the documents misclassify it as noise. |
| CR32-F7 | MEDIUM | The claimed final Docker runtime regression used stale binaries for several changed Root, Solve, and Coeff paths. |
| CR32-F8 | MEDIUM | The microbenchmark bypasses the ordinary Solve cache path and the Root prepared path, so its flat rows do not establish no regression. |
| CR32-F9 | MEDIUM | Benchmark provenance is insufficient to reconstruct the exact binaries behind the published numbers. |
| CR32-F10 | LOW | `PP_VM_PERF` reports incomplete syscall counts for Param and single-worker Coeff runs. |
| CR32-F11 | MEDIUM | The Graviton report overextends EC2 results into untested Lambda memory-size and CPU-cap conclusions. |
| CR32-F12 | LOW | Root prepared chains are plausible by inspection but have no direct prepared-path regression test or current production-shaped measurement. |
| CR32-F13 | LOW | The harvested-program narrative attributes gains to fast vector kernels that its scalar-heavy chain does not execute. |
| CR32-F14 | LOW | The benchmark CLI cannot run `--cases micro` as advertised by its filtering model. |

## CR32-F1 - Coeff Fast Kernels Violate the Non-Finite Policy

Severity: CRITICAL

### Evidence

The generic vector operations end with the established policy:

```c
if (!isfinite(*rr)) *rr = 0.0;
if (!isfinite(*ri)) *ri = 0.0;
```

That policy is at `lambda/sweep_cli.c:4624-4625` for binary operations and `lambda/sweep_cli.c:4677-4678` for unary operations.

The CR31 F7 fast paths at `lambda/sweep_cli.c:4818-4832` perform vector-vector add, subtract, and multiply directly. The CR31 fast unary paths at `lambda/sweep_cli.c:4874-4881` perform `neg`, `conj`, `real`, and `imag` directly. None applies the final non-finite clamp.

The implementation comment at `lambda/sweep_cli.c:4871` says each fast kernel's arithmetic matches the generic ladder exactly. It matches the central arithmetic expression, but not the operation's complete semantics. The clamp is part of the operation.

### Reproduction

The verified test chain creates a degree-two constant vector with values near `DBL_MAX`, duplicates the vector through the typed stack, adds it to itself, and writes it back to `poly`:

```text
push_const(2, 1e308)
emit
_typed_push_vector(poly)
_typed_push_vector(poly)
_typed_binary(add)
_typed_set_poly
```

Baseline `32e01ff` output as float32:

```text
0.0  0.0  0.0  0.0
SHA-256 374708fff7719dd5979ec875d56cd2286f6d3cf7ec317a3b25632aab28ec37bb
```

Current `9056919` output:

```text
inf  0.0  inf  0.0
SHA-256 512bf535db00964e9340dc35dd702282fe0b977200c59d8b3d213e1711cc8bb1
```

This is not harmless cache fragmentation or a benchmark-only discrepancy. It is a changed program result.

### Why the gates missed it

The property fuzz explicitly avoids non-finite expected values at `tests/test_coeff_vm_property_fuzz.py:114`. The benchmark matrix uses ordinary finite coefficient functions and parameter ranges. Byte equality over those fixtures never enters this policy boundary.

### Impact

- Existing programs can now emit infinities where the historical evaluator emitted zero.
- Downstream float conversion, solver behavior, and artifact hashes can change.
- The post-mortem's output-preservation claim is false outside the selected finite fixtures.
- The Graviton report's statement that the campaign "regresses none" is false.

### Required fix

Apply exactly the same component-wise clamp after every specialized binary and unary operation. Do not invent a different threshold or sanitize policy. The fast path must implement the full generic operation, not just its arithmetic core.

### Required tests

1. Add direct native parity tests for overflow in fast add, subtract, and multiply.
2. Add non-finite-input tests for fast `neg`, `conj`, `real`, and `imag`.
3. Compare each fast path against an intentionally forced generic path.
4. Include signed zero, infinity, NaN, subnormal, and overflow values.
5. Keep complete output-file hashes in the A/B benchmark matrix.

## CR32-F2 - Solve Feature Cache Regresses the Common Path

Severity: HIGH

### Evidence

`SolveSourceFeatures` at `lambda/solve_score.h:1642-1657` embeds:

- `stackRoots[2048]`, or 8192 bytes;
- `s1Stack[1024]`, or 8192 bytes;
- memo arrays and scalar state.

`solve_score_eval_metric_slots` creates two instances on every call at `lambda/solve_score.h:1776`, one for solve roots and one for coefficient roots. `solve_features_init` then clears both entire objects with `memset` at `lambda/solve_score.h:1659-1660`, even if the program has one scalar O(d) metric and uses only one source.

For any pair-family metric, `solve_features_pair_pass` at `lambda/solve_score.h:1698-1728` unconditionally:

- initializes every nearest-neighbor slot;
- computes the global minimum distance;
- calls `log10` for the crowding sum on every pair;
- updates both nearest-neighbor minima for every pair;
- calls `log10` for every nearest-neighbor result.

A proximity-only program needs only the minimum distance. It now pays for crowding and both nearest-neighbor metrics as well.

### Production-entry A/B

The benchmark below calls `solve_score_eval_metric_slots`, which is the changed production entry. It does not call the old raw metric helper directly.

| One-slot program | Baseline `32e01ff` | Current `9056919` | Regression |
|---|---:|---:|---:|
| `max_re` | 46.954 ns | 246.754 ns | 5.26x slower |
| `proximity` | 329.050 ns | 2199.620 ns | 6.68x slower |

The output sink is identical in both runs, so this is a pure performance regression rather than changed arithmetic.

The dense five-slot pair bundle genuinely improves:

| Bundle | Baseline | Current | Result |
|---|---:|---:|---:|
| M3 pair bundle | 4337.6 ns | 2658.9 ns | 1.63x faster |
| Graviton pair bundle | 12588.8 ns | 8644.4 ns | 1.46x faster |

The implementation optimizes one dense bundle by imposing its full preparation cost on every ordinary program.

### Stack impact

Compiler stack-usage output reports approximately 75,120 bytes for `solve_palette_chunk_mt.c:compute_scores_for_roots` after CR31. The feature objects also combine with the existing stack scratch inside `compute_solve_metric_score` for non-pair metrics. This is not an immediate overflow on normal pthread stacks, but it is a large and unnecessary per-call/per-frame expansion in hot production code.

### Why the benchmark missed it

`tests/native/vm_microbench.c:195-211` measures `metric_max_re`, `metric_proximity`, `metric_clusteriness`, and `metric_min_angular` by calling `compute_solve_metric_score` directly. That bypasses `solve_score_eval_metric_slots` and therefore bypasses every byte of the new cache.

Only `metric_bundle_pair4` calls the changed entry. The table's flat ordinary metric rows are not evidence about the optimized production path.

### Impact

- The default and common one-metric solve-score shapes can become several times slower.
- O(d) metrics pay two large clears and cache plumbing that provide no reuse.
- A one-slot proximity program performs crowding and nearest-neighbor work it never requested.
- Stack pressure rises in every consumer that includes the header and invokes the production evaluator.

### Required fix

Build a requirement plan before evaluating metrics.

1. Scan slots once and derive per-source feature masks: minimum distance, crowding, nearest-neighbor vector, raw duplicate reuse, and finite filtering.
2. Use the original direct path when a source has one unique metric and no reusable preparation.
3. Make the pair traversal conditional: do not compute `log10` crowding or NN data unless requested.
4. Do not clear 16 KiB inline arrays merely to initialize scalar state.
5. Move large scratch storage to a reusable worker-owned workspace or allocate it lazily only when the requested feature requires it.
6. Preserve the original accumulation order separately for every requested metric.

### Required tests and benchmarks

Add production-entry cases for:

- one O(d) metric such as `max_re`;
- one proximity metric;
- one crowding metric;
- one nearest-neighbor metric;
- two duplicate slots of one metric;
- the current dense pair bundle;
- solve and coefficient sources separately;
- mixed sources;
- finite and filtered non-finite roots;
- degree 35, 128, 256, and 1024.

The acceptance rule should be: dense bundles retain a material win and no one-slot production case regresses by more than ordinary measurement noise.

## CR32-F3 - The Chunked t1 Experiment Did Not Execute Blocked I/O

Severity: HIGH

### Evidence

The new blocked helpers are `coeffGenReadBlock` and `coeffGenWriteBlock` at `lambda/sweep_cli.c:9928-9951`. They are used by `coeffGenThreadWorkerMain`, which is entered only when `threadsUsed > 1`.

The one-worker branch begins at `lambda/sweep_cli.c:10195`. It retains the old per-row calls:

```c
pread(... sizeof(params) ...);   /* lambda/sweep_cli.c:10215 */
pwrite(... outRowBytes ...);     /* lambda/sweep_cli.c:10261 */
```

Therefore:

- baseline t1 uses one read and one write per row;
- candidate t1 uses one read and one write per row;
- the t1 A/B does not test blocked I/O at all.

### Incorrect document conclusions

`cr-31-post-mortem.md:73-81` describes blocked I/O as if it applies to the chunk path generally and uses t1 to conclude that the one-worker gap is not syscall cost.

`cr-31-graviton.md:39-45` is more explicit: it says `131,072 -> about 1,024` syscalls did not move t1 on either platform and declares the syscall theory dead. That transition never occurred in the t1 candidate.

The original CR31 acceptance target specifically called for batching the one-worker production path. That target remains unimplemented, not disproved.

### Impact

- The main negative result in the post-mortem is invalid.
- The residual-work list assigns t1 to per-row VM work without isolating I/O.
- The campaign leaves two implementations of the same chunk loop, guaranteeing future drift.
- Single-worker Lambda configurations receive none of F2.

### Required fix

Route t1 through the same block engine, or factor one range worker that can execute inline without creating a pthread. The serial and threaded cases should differ only in work partitioning, not in I/O semantics.

### Required benchmark

Use syscall counters that include the actual direct calls and establish:

| Case | Required evidence |
|---|---|
| baseline t1 | about two syscalls per row |
| fixed candidate t1 | about two syscalls per block |
| candidate t2/t4/t8 | same block policy per worker |
| partial first/tail range | exact byte equality and correct offsets |
| injected short I/O | retry and first-failed-row behavior |

Only after that A/B can the review conclude whether t1 is I/O-bound.

## CR32-F4 - CPU Cap Can Change Param Dither Results

Severity: HIGH

### Evidence

Threaded and range evaluation use `computeParamGenRow`, which seeds each row at `lambda/sweep_cli.c:8955`.

The full serial path seeds once per pass at `lambda/sweep_cli.c:9067-9070` and consumes one evolving RNG stream across rows.

CR31 caps `threadsUsed` with `sysconf(_SC_NPROCESSORS_ONLN)` at `lambda/sweep_cli.c:9434-9438`. If a full non-range request asks for multiple workers but the host reports one online CPU, the same request crosses into `runParamGenSerial` and changes seed policy.

Verified `sdith` output hashes:

```text
t1  5b424e88d55dc56e66bf1163f4967e7afc749d567ff9dec504063a42dfa9fd5a
t2  c2d54eb715035c9db1ede2bba0f1e8515f68d5d234fdcf209e78333d92b364ce
t4  c2d54eb715035c9db1ede2bba0f1e8515f68d5d234fdcf209e78333d92b364ce
```

The t2/t4 equality is good. The t1 split means CPU visibility can be a reproducibility input despite the source comment saying the cap is "never a reproducibility input."

Range requests are narrower: `runParamGenRangeSerial` calls `computeParamGenRow`, so a range request capped to one retains row seeding. The correctness risk is specifically the full non-range path. That still includes real CLI/API uses and must not depend on host CPU topology.

### Impact

- Identical full requests can produce different parameter streams on hosts with different visible CPU counts.
- Artifacts and cache identities do not encode that host-dependent choice.
- Small-memory Lambda behavior cannot be inferred from an eight-core EC2 run where the cap did not bind.

### Required fix

Choose one canonical seed policy and use it in every serial, range, streaming, and static-scheduler path. The simplest implementation is for full serial generation to call the same row function used by all other paths.

Because changing the historical t1 dither stream changes output, document the compatibility decision explicitly. If old t1 bytes must remain reproducible, version the seed policy rather than silently changing it.

### Required tests

1. Assert t1/t2/t4/t8 byte identity for every dither transform under the chosen policy.
2. Exercise full and range requests.
3. Simulate a CPU cap below the requested thread count.
4. Verify output metadata records any intentionally versioned seed policy.

## CR32-F5 - Worker Failure Flags Have Data Races

Severity: MEDIUM

### Evidence

`ParamGenStaticCtx.failed` is a plain `int` at `lambda/sweep_cli.c:9248`.

`paramGenStaticSetError` writes it while holding `errMutex` at `lambda/sweep_cli.c:9258-9264`. Worker loops read it without holding that mutex at `lambda/sweep_cli.c:9276` and `lambda/sweep_cli.c:9304`.

ThreadSanitizer confirms a write/read race on an injected dynamic-expression failure. The mutex protects the error string and first-writer selection, but it does not make unlocked reads of `failed` legal C.

The Coeff worker context has the same pattern: `coeffGenSetThreadError` writes under the context mutex, while worker loops read `ctx->failed` at `lambda/sweep_cli.c:9978`, `10023`, and `10040` without the lock. That pattern predates part of CR31, but the campaign expanded and relies on the affected worker path.

### Impact

- Behavior is undefined by the C memory model.
- Workers may continue after failure or fail to observe cancellation promptly.
- Sanitizer-clean concurrency cannot be claimed.

### Required fix

Use `_Atomic int failed` with relaxed loads and a store coordinated with the existing mutex. Keep the error message under the mutex. Read the final flag after `pthread_join`, which already supplies completion ordering.

Add a TSan test that forces one worker to fail while others are active for both Param and Coeff paths.

## CR32-F6 - Param Static Scheduler Regresses the Default M3 Path

Severity: MEDIUM

### Evidence

Each static worker allocates one row and calls `pwrite` once per row at `lambda/sweep_cli.c:9292-9303`. The original CR31 plan called for static ranges and block output; only the static ranges were implemented.

The fused compute plan defaults to four threads at `lambda/handler_compute_plan.py:248`.

A 51-repetition interleaved A/B on M3 gives:

```text
base median       4649 us
candidate median  5368 us
delta             +15.47%
base MAD           126 us
candidate MAD        39 us
bytes              identical
```

A separate 31-repetition sweep gives:

| Threads | Delta |
|---:|---:|
| 2 | +2.35% |
| 4 | +14.90% |
| 8 | -78.81% |

The eight-worker cliff is genuinely fixed. The four-worker default is genuinely slower on M3.

Graviton gives t2 `-8.1%`, t4 `-5.9%`, and t8 `-47.5%`. That means the implementation is platform-sensitive. It does not turn the M3 regression into scheduler noise.

### Incorrect document claim

`cr-31-post-mortem.md:18`, `87-88`, and `152` call the +9.6% result "within MAD." The measured gap is several times the baseline MAD and far larger than the candidate MAD. The wording is mathematically false.

`cr-31-graviton.md:47-50` calls the M3 result host scheduling noise. The repeated interleaved result supports a platform-specific regression, not random noise.

### Required fix

Batch contiguous rows per worker into a reusable output block before each `pwrite`. This should preserve the static ownership model while reducing the M3 syscall penalty.

Retain separate platform results in the documentation. Do not average away opposite-sign behavior.

## CR32-F7 - Final Docker Regression Used Stale Binaries

Severity: MEDIUM

### Evidence

The post-mortem says the ARM64 Docker runtime regression passed against freshly cross-compiled deploy binaries at `cr-31-post-mortem.md:29-30`, and repeats that all deploy binaries were rebuilt at lines 130-134.

Local timestamps contradict that statement:

| Binary | Timestamp | Relevant source commit |
|---|---|---|
| `lambda/roots2pix_mt` | 18:39:58 | Root P1 committed 19:32; Solve P3 committed 19:43 |
| `lambda/solve_proximity_hist_sectioned` | 18:40:37 | Root P1 19:32; Solve P3 19:43 |
| `lambda/solve_palette_chunk_mt` | 18:40:57 | Root P1 19:32; Solve P3 19:43 |
| `lambda/sweep_coeffgen` | 18:41:54 | Coeff F1 committed 19:23; P2/P4 followed |
| `lambda/sweep_cm` | 18:41:54 | Coeff F1 committed 19:23; P2/P4 followed |

Some static binaries such as `lambda/sweep`, `lambda/sweep_mt`, and `lambda/solve_proximity_stats` were rebuilt later. The assertion that all deploy binaries were current is still false.

`tests/docker_runtime_regression.py` executes the existing binaries. It does not compile them. A green Docker run against stale files proves packaging/runtime compatibility for those files, not the final source.

### Impact

- Current Root prepared plans were not exercised in the affected deploy binaries.
- Current Solve feature-cache behavior and stack growth were not exercised in the affected binaries.
- A later real deployment rebuild would introduce code that the claimed final runtime regression never ran.

### Required fix

1. Rebuild every deploy binary from one clean final commit.
2. Record each binary's SHA-256 and source commit.
3. Run Docker regression only after the build step.
4. Make the regression fail if a generated binary is older than any source/header in its declared dependency set.
5. Add a build manifest to the deployment package.

## CR32-F8 - Microbenchmark Coverage Is Path-Selective

Severity: MEDIUM

### Solve blind spot

The ordinary metric rows call `compute_solve_metric_score` directly. They cannot measure CR31 F4. Only the dense pair bundle enters `solve_score_eval_metric_slots`.

Therefore `cr-31-graviton.md:60-63` and `105-116` cannot use the flat ordinary rows to claim all non-bundle Solve behavior is flat.

### Root blind spot

The Root benchmark constructs `RootXformEntry` objects directly at `tests/native/vm_microbench.c:215-225`. Their `prep_fn` remains zero, so `apply_root_xforms` deliberately takes the old fallback path. The benchmark cannot measure F8.

That limitation is acknowledged in the documents, but the post-mortem still marks Root as a successful structural result without a current production-shaped A/B. No test invokes `rt_prepare_chain` directly, and Docker fixtures found in the relevant runtime tests use empty `root_transforms` arrays.

### Numerical-policy blind spot

The macro matrix contains ordinary finite values. It does not cover overflow/non-finite Coeff operations, dither seed transitions, cancellation, short I/O, or high-degree Solve stack behavior.

### Required fix

The benchmark should include the public production entry for every optimization:

- parsed Root file -> prepared chain -> apply;
- one-slot and many-slot Solve production evaluator;
- one-worker and multiworker blocked Coeff I/O;
- full and range Param generation with dither;
- fast Coeff operations at numerical policy boundaries.

## CR32-F9 - Benchmark Reports Do Not Pin Their Binaries

Severity: MEDIUM

### Graviton reports

Every Graviton report has:

```json
"git_dirty": false,
"git_sha": ""
```

The reports therefore do not independently establish that base was `32e01ff` and candidate was `9056919`. The Markdown says so, but the machine-readable evidence does not.

`reports/vm_bench_graviton_after-cr31.json` contains `"micro": null`. The published Graviton micro values occur only in Markdown and are not preserved with compiler/source metadata.

### M3 reports

The baseline and after reports both record `git_dirty: true`. The after report records SHA `5159951`, not final implementation `9056919`. A dirty flag does not identify which later source edits were present.

### Repository retention

`reports/` is gitignored. A clean checkout containing the two review documents does not contain the reports they cite.

### Harness limitation

In compare mode, host metadata describes the checkout running the harness, not either supplied binary. The harness records neither binary SHA-256 nor build command per binary.

### Required fix

Each report must include:

- base binary SHA-256;
- candidate binary SHA-256;
- source commit/tree hash for each binary;
- exact compile command and compiler identity;
- benchmark source SHA-256;
- fixture SHA-256;
- whether the tree was dirty, plus a patch hash if it was;
- architecture and libc;
- the microbenchmark data in the same retained report.

Commit compact final reports or publish them as immutable build artifacts referenced by digest.

## CR32-F10 - Perf Counters Are Incomplete

Severity: LOW

### Evidence

`pp_perf_pread_calls` and `pp_perf_pwrite_calls` increment only inside `coeffGenReadBlock` and `coeffGenWriteBlock` at `lambda/sweep_cli.c:9931-9945`.

They do not count:

- t1 direct `pread` at `lambda/sweep_cli.c:10215`;
- t1 direct `pwrite` at `lambda/sweep_cli.c:10261`;
- Param static-scheduler `pwrite` at `lambda/sweep_cli.c:9297`;
- Param serial `fwrite` calls.

Yet `pp_perf_report` is emitted for Param at `lambda/sweep_cli.c:9490` and for all chunked Coeff runs at `lambda/sweep_cli.c:10345`.

A t1 chunked run or Param static run can therefore report zero I/O calls while performing substantial I/O. The post-mortem description at lines 125-128 says the counters count preads/pwrites per chunked/Param run; that is not true.

### Required fix

Wrap every relevant syscall/output operation in the same instrumentation layer, or rename the fields to `blocked_pread_calls` and `blocked_pwrite_calls` so they cannot be mistaken for totals. Add tests for expected counts in t1, t2, and Param modes.

## CR32-F11 - EC2 Graviton Results Are Not a Lambda Sizing Proof

Severity: MEDIUM

### What was measured

The Graviton run used a c7g.2xlarge with eight physical Graviton3 cores, Amazon Linux 2023, glibc 2.34, and gcc 11.5.

That is useful architecture-specific evidence. It is not the deployed Lambda environment:

- production binaries are built statically with musl;
- Lambda CPU allocation and cgroup visibility differ from a dedicated eight-core EC2 instance;
- the CPU cap did not bind in the experiment;
- no low-memory Lambda allocation was measured;
- no cold-start, package, `/tmp`, or noisy-neighbor effect was measured.

### Overclaim

`cr-31-graviton.md:130-140` concludes that the campaign helps at every Lambda memory size and regresses none. The table has no data for every memory size, the cap did not bind, single-metric Solve was not measured, and Coeff non-finite behavior regressed.

The report can conclude that the selected finite macro cases improve on that EC2 Graviton host. It cannot close Lambda sizing or correctness.

### Required evidence

Run a small Lambda matrix at representative memory allocations with the final static binaries. Record effective CPUs, actual `threadsUsed`, output hashes, and billed duration. Treat those measurements as sizing evidence, not the EC2 table.

## CR32-F12 - Root Prepared Path Has No Direct Gate

Severity: LOW

### Source assessment

The prepared implementations in `lambda/root_xforms.h:350-435` appear arithmetic-equivalent to the original functions by direct trace:

- rotate hoists `2*pi*turns`, `cos`, and `sin`;
- pull transforms hoist clamped sigma and `1/(sigma*sigma)`;
- add, multiply, and Mobius hoist defaulted arguments;
- the inner float stores remain in the same positions.

This is the right shape for a preparation pass.

### Missing proof

- `rg` finds no test reference to `rt_prepare_chain` or `prep_fn`.
- The microbenchmark uses hand-built unprepared entries.
- Relevant Docker fixtures use empty Root chains.
- Several consumer binaries were stale when the final Docker test ran.

### Required test

For every Root transform and every supported arity/default form:

1. Apply an unprepared hand-built entry.
2. Serialize/parse the same entry through `parse_root_xform_file`.
3. Assert byte-identical float32 roots.
4. Cover zero, negative sigma, omitted defaults, poles, NaN propagation, and a full 16-entry chain.

Add a production-shaped benchmark that parses once and applies many rows.

## CR32-F13 - Harvested Gain Is Misattributed to Vector Kernels

Severity: LOW

The harvested `compute_mqlacwaq` Coeff chain contains:

```text
94 _typed_push_scalar
68 _typed_binary
13 _typed_unary
13 _typed_poke_poly
 1 _native_transform
```

It contains no `_typed_push_vector`. Its typed binary and unary work is scalar stack work. The F7 vector-vector loops at `lambda/sweep_cli.c:4818` and vector unary loops at `4874` are not entered by those scalar operations.

The F1-only measurement was about `-15.9%`; the final M3 result was `-16.5%`. The extra fraction is too small to attribute confidently and cannot demonstrate the vector kernels because the harvested chain does not execute them.

The documents should credit F1 as the harvested-program win. F3/F7 need separate typed-vector fixtures.

## CR32-F14 - `--cases micro` Is Rejected Before Micro Runs

Severity: LOW

`scripts/bench_program_vms.py:253-256` filters only macro cases and exits if none match. `micro` is not in that dictionary. The later micro branch at lines 268-270 is therefore unreachable for `--cases micro`.

Verified behavior:

```text
python3 scripts/bench_program_vms.py --cases micro --reps 1
no cases match 'micro'
```

Treat micro as a selectable case before the empty-selection check, or add a dedicated `--micro-only` option. Parenthesize the line-268 boolean expression while editing it; its current precedence is needlessly difficult to audit.

## Document Accuracy Audit

### `cr-31-post-mortem.md`

The following claims need correction:

| Location | Current claim | Correct state |
|---|---|---|
| lines 18, 87-88, 152 | Param t4 regression is within MAD | Repeated interleaved A/B shows a material M3 regression. |
| lines 24-30 | Tests plus fresh Docker binaries establish final correctness | Tests passed, but several Docker binaries were stale and edge policies were untested. |
| lines 73-81 | F2 blocked chunk I/O includes t1 | t1 remains the old per-row implementation. |
| lines 93-99 | Shared pair pass is a general production win | Dense bundle wins; one-slot programs regress 5x to 7x. |
| lines 101-123 | Fast operations preserve exact bytes | Fast non-finite policy differs from generic behavior. |
| lines 125-128 | Counters count Param/chunked I/O | They count only blocked helper calls. |
| lines 130-134 | All deploy binaries were rebuilt | Several binaries predate the relevant commits. |
| residual work | Lists only future optimizations | It omits current correctness, regression, race, and evidence blockers. |

### `cr-31-graviton.md`

The following claims need correction:

| Location | Current claim | Correct state |
|---|---|---|
| lines 39-45 | t1 tested 131,072 -> about 1,024 syscalls | Candidate t1 still performs per-row I/O. |
| lines 47-50, 161-163 | M3 t2/t4 regression was noise | It is repeatable platform-dependent behavior. |
| lines 60-63, 105-116 | Other Solve metric rows are flat | Those rows bypass the changed production path. |
| lines 65-71 | 26 hashes validate the oracle suite broadly | They validate those 26 finite cases across two arm64 hosts. |
| lines 118-128 | EC2 result represents deployment sizing | It is useful Graviton evidence, not Lambda sizing evidence. |
| lines 130-140 | Every memory size improves and none regress | Untested and contradicted by other findings. |
| lines 143-157 | Results are reproducible from named reports | Reports omit source/binary hashes; micro data is absent. |

## Why the Existing Gates Stayed Green

The green suite is not fraudulent; it is incomplete in exactly the dimensions CR31 changed.

| Defect | Why existing tests miss it |
|---|---|
| Coeff clamp regression | Finite fixtures and fuzz filtering of non-finite expectations. |
| Solve single-metric slowdown | Correctness tests do not time; micro ordinary rows bypass cache. |
| t1 blocking omission | No syscall-count assertion and no requirement that t1 enter block helpers. |
| Dither host dependence | No t1/t2 equality contract or simulated CPU cap. |
| Failure-flag races | No TSan gate. |
| Root prepared path | Tests validate Python/source behavior, not parsed native prepared entries. |
| Stale deploy binaries | Docker test consumes files without checking source freshness. |
| Report provenance | Reports are advisory, ignored, and not schema-validated. |

The byte matrix is also intentionally finite and narrow. Equal bytes prove equivalence only for the programs and values that produced those files.

## Verified Strengths

The adversarial findings do not erase the parts that are sound.

### Coeff argument plan

`coeffPrepareArgPlan` correctly leaves malformed or expression-backed arguments on the dynamic path, uses a zero static frame for truly argument-free tokens, and copies literal real/imaginary arguments into prepared frames. No behavior defect was found in F1 itself. The harvested-program speedup is real on both measured hosts.

### Param lowered expression plan

The Param plan replays the same expression operations in the same order and computes a conservative register-read mask. All current parsed programs pass through preparation. No output mismatch was found in ordinary or partial-range probes.

### Solve lag flags

Parsed programs receive per-slot lag flags; hand-built structs retain the old scan fallback because `lagPrepared == 0`. This is a sound compatibility pattern.

### Multithreaded blocked Coeff I/O

The block helpers correctly resume short transfers, use worker-owned contiguous ranges, and preserved bytes for tested normal, partial-head, partial-tail, and non-row-aligned ranges. Graviton t2/t4/t8 improvements are real.

### Scoped Coeff slot reuse

The circular-stack reasoning for reusing a popped destination slot is coherent: pushes copy values, duplicate copies, and pop parks the head at the reusable slot. The discovered defect is the missing operation clamp, not an ownership alias in the tested paths.

### Graviton macro results

The values in the Markdown match the ignored JSON reports. All 26 candidate macro output hashes match the corresponding M3 candidate hashes. The selected harvested, chunked multithreaded, and Param Graviton gains are valid results for that host and those fixtures.

The following headline measurements remain valid as measured, subject to their stated path scope:

| Graviton case | Baseline | Candidate | Valid conclusion |
|---|---:|---:|---|
| `mqlacwaq_coeff` | 46.489 ms | 31.880 ms | The prepared Coeff argument plan materially helps this harvested program. |
| `mqlacwaq_param_coeff` | 49.217 ms | 35.184 ms | The combined harvested payload is materially faster. |
| `mqlacwaq_baseline` | 2.184 ms | 2.166 ms | The no-program control is effectively flat. |
| `chunked35_t2` | 213.210 ms | 180.703 ms | Blocked multithreaded I/O improves t2 on this host. |
| `chunked35_t4` | 107.971 ms | 91.099 ms | Blocked multithreaded I/O improves t4 on this host. |
| `chunked35_t8` | 55.837 ms | 46.043 ms | Blocked multithreaded I/O improves t8 on this host. |
| `param_expr_t4` | 5.680 ms | 5.346 ms | The static scheduler improves this t4 case on Graviton, despite regressing it on M3. |
| `param_expr_t8` | 5.811 ms | 3.051 ms | The static scheduler removes the Graviton t8 plateau. |

The t1 measurements are also numerically real (`402.393 -> 400.756 ms` on Graviton and `151.210 -> 152.826 ms` on M3), but they compare two per-row-I/O paths and therefore say nothing about the missing t1 blocked-I/O optimization.

## Required Remediation Order

### Phase 0 - Restore correctness

1. Restore the generic non-finite clamp in every Coeff fast kernel.
2. Add numerical-policy parity tests before making any further VM optimization.
3. Remove the Param/host seed-policy split or explicitly version it.
4. Make worker cancellation flags atomic and run TSan.

### Phase 1 - Remove the Solve common-path regression

1. Add per-source metric requirement masks.
2. Bypass feature caching when no reuse exists.
3. Compute only requested pair features.
4. Remove unconditional large-object clearing.
5. Move large scratch to reusable/lazy storage.
6. Add one-slot production-entry benchmarks.

### Phase 2 - Finish the I/O and scheduler work that CR31 claimed

1. Route Coeff t1 through blocked I/O.
2. Batch Param static output writes.
3. Instrument all actual I/O paths.
4. Re-run M3 and Graviton A/Bs with syscall counts.

### Phase 3 - Make evidence reproducible

1. Fix micro-only selection.
2. Benchmark prepared Root and production Solve paths.
3. Rebuild every deploy binary from one clean commit.
4. Record binary and source hashes.
5. Run Docker against those exact binaries.
6. Retain reports by digest.
7. Run a small real-Lambda memory/concurrency matrix before sizing conclusions.

## Mandatory Regression Matrix

### Correctness

```text
tests/test_param_program_native.py
tests/test_param_program_chain.py
tests/test_param_program_source.py
tests/test_program_m3_oracles.py
tests/test_coeff_program_native.py
tests/test_coeff_program_chain.py
tests/test_coeff_source_equivalence.py
tests/test_coeff_wire_fingerprints.py
tests/test_coeff_vm_property_fuzz.py
tests/test_root_program_source.py
tests/test_root_transform_registry.py
tests/test_solve_score_native_parity.py
tests/test_solve_score_source_equivalence.py
tests/test_whole_sweep_oracle.py
```

Add new gates for:

- Coeff fast/generic non-finite parity;
- Param dither invariance across effective thread counts;
- Root prepared/unprepared byte parity;
- Solve one-slot and bundle production-entry parity;
- partial blocked t1/tN ranges;
- injected short read/write behavior;
- Param and Coeff TSan failure paths;
- deploy-binary freshness and build manifests.

### Performance

Performance remains advisory, but every report must include:

| Area | Required cases |
|---|---|
| Coeff buffered | finite scalar, typed vector, non-finite policy fixture, harvested program |
| Coeff chunked | t1/t2/t4/t8, plain and transcendental, syscall totals |
| Param | full and range t1/t2/t4/t8, finite expression and dither |
| Root | parsed prepared chain at degree 35/128/256 |
| Solve | one O(d), one proximity, one NN, duplicate slot, dense bundle, mixed sources |
| Deployment | final static binaries on M3, EC2 Graviton, and representative Lambda sizes |

## Deployment Decision

Current decision: **NO-GO**.

The minimum conditions to change that decision are:

1. CR32-F1 has a byte-level regression test and fix.
2. CR32-F2 no longer regresses one-slot production programs.
3. CR32-F4 has one documented canonical seed policy.
4. CR32-F5 is TSan-clean.
5. Coeff t1 is either actually blocked and measured or honestly left as an unimplemented optimization.
6. Every deploy binary is rebuilt from the final source and the Docker runtime test runs those hashes.
7. The post-mortem and Graviton report are corrected so their conclusions match the paths measured.

After those conditions, the campaign should retain its real wins: F1 Coeff preparation, multithreaded Coeff blocking, Param static range ownership, Solve duplicate/dense-family reuse, Root constant preparation, and lag-flag preparation. The goal is not to discard CR31. It is to stop narrow favorable measurements from certifying behavior that the implementation and tests did not cover.
