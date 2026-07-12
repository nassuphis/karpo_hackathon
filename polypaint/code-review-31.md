# Code Review 31: Virtual Machine Performance

Date: 2026-07-12  
Reviewed HEAD: `32e01ff` (`CR30 follow-up: all 12 second-pass findings fixed`)  
Review mode: performance-focused code audit, production-shaped macrobenchmarks, direct C microbenchmarks, and controlled source A/B probes

## Verdict

There is substantial speed available, but it is not in one generic "make the interpreter faster" change. The four deployed program runtimes have different bottlenecks:

- **Coeff Program:** repeated argument preparation and excessive whole-vector copying dominate cheap operations. The production chunked path also performs one `pread` and one `pwrite` per coefficient row.
- **Param Program:** the evaluator lacks the load-time lowering already present in Coeff, and its threaded row scheduler collapses under excess workers because every row passes through one mutex and condition variable.
- **Solve Score:** the stack VM is already very cheap. Recomputing root features independently for each metric is the real cost, often by one to two orders of magnitude.
- **Root Transform:** dispatch is small, but constant transform setup such as `sin`, `cos`, defaults, and reciprocal variance is repeated for every solved row.

The best first changes are concrete and wire-neutral:

1. Pre-resolve static Coeff arguments and do not construct `CoeffResolvedArgs` for argument-free tokens. A controlled source A/B reduced the real 189-token `compute_mqlacwaq` program from `23.666 ms` to `20.900 ms`, an **11.7% end-to-end Coeff generation reduction**, with byte-identical output.
2. Batch the production Coeff chunk worker's reads and writes. At 65,536 degree-35 rows, the one-thread chunked path took `151.519 ms` versus `114.156 ms` for the buffered path, a **32.7% penalty** around otherwise comparable work.
3. Replace Param's per-row shared scheduler with static contiguous worker ranges and block output. Four workers took `4.431 ms`, but eight took `24.311 ms`: **5.49x slower than four workers** and slightly slower than one.
4. Cache Solve metric feature families per source and row. The Solve VM itself took `3.4-56.9 ns` per evaluation; individual metrics took `45.7-1,660 ns`. Optimizing VM dispatch first would address the wrong layer.
5. Replace Coeff stack vector copying with slot ownership and in-place destinations. Current typed unary/binary operations write to scratch and immediately copy the result into another 4 KiB vector slot.

Do **not** begin with computed goto, a JIT, `-ffast-math`, wider thread defaults, or broad algebraic fusion. Those approaches either target a smaller measured cost, threaten exact numerical behavior, or amplify the current scheduler and I/O defects.

## Findings Summary

Severity below means expected performance impact on real workloads, not correctness severity.

| ID | Severity | Finding |
|---|---|---|
| F0 | HIGH | The existing benchmark suite does not measure several deployed VM paths and compiles with different optimization flags from production. |
| F1 | HIGH | Coeff resolves and clears a roughly 160-byte argument frame for every token on every row, even when the token has no arguments. |
| F2 | HIGH | Production `coeffgen_chunked` performs one `pread` and one `pwrite` per row, creating 131,072 syscalls for a 256 x 256 chunk. |
| F3 | HIGH | The Coeff vector stack is a copy machine: source, scratch, stack, poly, and output buffers repeatedly copy the same vectors. |
| F4 | HIGH | Solve metrics independently recompute shared O(d), O(d log d), and O(d^2) root features; the Solve stack VM is not the bottleneck. |
| F5 | HIGH | Param's threaded scheduler serializes row acquisition and publication through one mutex and broadcasts on every row; eight workers regress catastrophically. |
| F6 | MEDIUM | Param parses `uses_legacy_fast_path` but never uses it, and evaluates raw expression triples rather than a lowered execution plan. |
| F7 | MEDIUM | Coeff's generic per-element unary/binary dispatch prevents clean kernel specialization and easy vectorization. |
| F8 | MEDIUM | Root Transform repeats constant setup and default decoding for every row instead of preparing an execution plan once. |
| F9 | MEDIUM | All four VMs lack a consistent internal execution-plan pass for flags, static operands, exact peepholes, and resource masks. |
| F10 | LOW | Current timing metadata is insufficient to distinguish VM, metric, I/O, scheduler, and output costs in production-shaped runs. |

## Scope: What Actually Executes Programs

This review covers the deployed native runtimes, not only the Python compilers:

| Program kind | Compiler / serializer | Deployed evaluator | Main call site |
|---|---|---|---|
| Param | `lambda/param_program_chain.py`, `lambda/param_program_source.py` | `paramEvalProgram` in `lambda/sweep_cli.c:7087` | `applyParamTransformProgram` at `lambda/sweep_cli.c:7320` |
| Coeff | `lambda/coeff_program_chain.py`, `lambda/coeff_program_source.py` | `evalCoeffProgram` in `lambda/sweep_cli.c:5553` | buffered and chunked coefficient generators |
| Root | `lambda/root_program_source.py`, `lambda/root_pipeline_programs.py` | `apply_root_xforms` in `lambda/root_xforms.h:340` | solve/raster paths including `root_xforms.h` |
| Solve Score | `lambda/solve_score_chain.py`, `lambda/solve_score_program_source.py` | metric extraction and `solve_score_eval_program_outputs_from_buffers` in `lambda/solve_score.h:1733` | solve-score streams and raster paths |

The benchmark and optimization boundary must include the work immediately around the interpreter. A faster opcode switch is irrelevant if each row still performs two syscalls, copies a coefficient vector four times, or computes the same pairwise root distances three times.

## Benchmark Methodology

### Host and build

- Host: Apple M3 Max, 16 physical cores (12 performance, 4 efficiency), 128 GiB RAM, `arm64`.
- Compiler: Apple clang 17.0.0.
- Native build: `cc -O3 -pthread -I lambda lambda/sweep_cli.c -lm`.
- Production uses `aarch64-linux-musl-gcc -O3 -static -pthread` at `deploy.sh:859`; the local benchmark matches the optimization level but not the Linux/musl/Graviton platform.
- Current source: `32e01ff`. That commit did not change `sweep_cli.c`, `root_xforms.h`, or `solve_score.h` after the benchmark review began.

These numbers are suitable for **relative A/B decisions on this checkout**. They are not Lambda wall-time forecasts. Any accepted optimization must be repeated on the deployed Graviton architecture before changing production sizing or concurrency defaults.

### Timing rules

- Macrobenchmarks use the native command's internal `elapsed_us`, so process startup and JSON parsing are excluded while traversal, generation, and output work inside the mode remain included.
- Tables use the median of 11 runs unless stated otherwise. The harvested program table uses 15 runs.
- Direct Root/Solve C microbenchmarks use nine samples and report median nanoseconds per call.
- Files and binaries were warm in `/tmp` for repeated runs.
- Semantic-equivalence probes compare SHA-256 hashes of complete output files.
- Controlled source A/B results were interleaved across 31 runs to reduce drift. The experimental source edit was reverted after measurement.
- macOS provides no stable CPU-affinity/performance-governor control here. Very short absolute timings can move with core placement and thermals; medians and interleaved ratios, rather than one minimum, drive the conclusions.

The current native/oracle baseline was also checked after benchmarking:

```text
76 passed, 45 subtests passed

tests/test_param_program_native.py
tests/test_coeff_program_native.py
tests/test_root_program_source.py
tests/test_solve_score_native_parity.py
tests/test_program_m3_oracles.py
tests/test_coeff_wire_fingerprints.py
tests/test_whole_sweep_oracle.py
```

Those tests establish a green starting point. They do not prove that a future optimization is equivalent; every candidate must rerun them plus its new byte-invariance cases.

### Reproduction assets

HEAD contains:

- `tests/.cr31_vm_matrix.py`: Param, Coeff, and production chunked-path macrobenchmark matrix.
- `tests/.cr31_vm_microbench.c`: direct Root Transform, Solve VM, and Solve metric microbenchmarks.
- `tests/bench_vm.py`: the older CR18 harness.

The two CR31 files are useful evidence but are not yet a durable benchmark interface: they are hidden dotfiles, the Python matrix hardcodes `/tmp/polypaint_sweep_vm_o3`, and neither records host/compiler/git metadata. F0 describes the required cleanup.

The current probes can be reproduced with:

```sh
cc -O3 -pthread -I lambda lambda/sweep_cli.c -lm -o /tmp/polypaint_sweep_vm_o3
python3 tests/.cr31_vm_matrix.py
cc -O3 -I lambda tests/.cr31_vm_microbench.c -lm -o /tmp/cr31_vm_microbench
/tmp/cr31_vm_microbench
```

## Benchmark Results

### Param VM: equivalent programs have materially different execution cost

Workload: `256 x 256 x 4 = 262,144` generated parameter rows, one worker.

| Program | Tokens | Median | Delta vs no program |
|---|---:|---:|---:|
| No Param Program | 0 | `9.500 ms` | baseline |
| Identity push/emit | 4 | `10.945 ms` | `+15.2%` |
| Add/subtract pair | 8 | `12.608 ms` | `+32.7%` |
| `unit_circle` shortcut lowering | 6 | `19.977 ms` | `+110.3%` |
| Equivalent expression program | 10 | `22.990 ms` | `+142.0%` |

A separate semantic-equivalence probe compared three encodings of the same unit-circle mapping:

| Encoding | Tokens | Median | Relative to expression |
|---|---:|---:|---:|
| Structural shortcut | 6 | `20.370 ms` | `14.5%` faster |
| One native-registry token | 1 | `16.639 ms` | `30.1%` faster |
| Scalar-expression form | 10 | `23.814 ms` | baseline |

All three complete output files had the same SHA-256:

```text
fea270dc41c8e25f2bd745d9215c883d4160a66e2aa8873d68df1e7c8cddafcd
```

This does **not** justify rewriting stored chains or fingerprints. It demonstrates that a wire-neutral internal execution plan can collapse known token patterns after parsing.

### Param threading: four workers help, eight workers collapse

Same 10-token expression program and 262,144 rows:

| Workers | Median | Speedup vs one | Relative to four |
|---:|---:|---:|---:|
| 1 | `22.990 ms` | `1.00x` | `5.19x` slower |
| 2 | `9.218 ms` | `2.49x` | `2.08x` slower |
| 4 | `4.431 ms` | `5.19x` | best |
| 8 | `24.311 ms` | `0.95x` | `5.49x` slower |

The super-linear-looking four-worker result is host scheduling and mode-level timing, not a claim of greater-than-linear VM scaling. The actionable result is the repeatable eight-worker cliff. The current public limit is 64 (`lambda/compute_fused.py:8`, `lambda/handler_compute_plan.py:56`), while the fused plan defaults to four (`lambda/handler_compute_plan.py:248`). Increasing the default without replacing the scheduler would be harmful.

### Coeff VM: cheap operations are dominated by plumbing

Workload: 16,384 rows, degree 35, buffered `coeffgen` path.

| Program | Tokens | Median | Added ns/row vs baseline |
|---|---:|---:|---:|
| No Coeff Program | 0 | `28.287 ms` | baseline |
| `poly = rev(poly)` | 1 | `29.162 ms` | `53.4 ns` |
| `poly = neg(poly)` | 1 | `30.779 ms` | `152.1 ns` |
| `poly = add(poly, poly)` | 1 | `30.840 ms` | `155.8 ns` |
| `poly = sin(poly)` | 1 | `38.110 ms` | `599.5 ns` |
| `sin` then `rev` | 2 | `38.634 ms` | `631.8 ns` |
| 16 x `rev` | 16 | `38.090 ms` | `598.8 ns` total |
| 16 x `neg` | 16 | `61.836 ms` | `2,047.9 ns` total |

An argument-free no-op `emit` measured in the controlled matrix added about `21 ns/row`. That fixed cost exists before useful vector arithmetic. A degree-35 `neg` is not expensive mathematics, yet it adds about `152 ns/row`, showing that buffer movement and generic dispatch are substantial.

At degree 4, where vector math is small, the overhead is clearer:

| Program | Tokens | Median |
|---|---:|---:|
| No Coeff Program | 0 | `0.960 ms` |
| Representative scalar/vector program | 21 | `3.535 ms` |
| 16 scalar assignments | 80 | `7.581 ms` |

### Production chunked Coeff path: row-at-a-time I/O is expensive

Workload: 65,536 degree-35 rows. The buffered path is not identical to the chunked API, but coefficient generation and data volume are comparable enough to expose the syscall tax.

| Path | Workers | Program | Median |
|---|---:|---|---:|
| Buffered `coeffgen` | 1 | none | `114.156 ms` |
| `coeffgen_chunked` | 1 | none | `151.519 ms` |
| `coeffgen_chunked` | 2 | none | `118.482 ms` |
| `coeffgen_chunked` | 4 | none | `70.248 ms` |
| `coeffgen_chunked` | 8 | none | `93.196 ms` |
| Buffered `coeffgen` | 1 | `sin` | `153.200 ms` |
| `coeffgen_chunked` | 1 | `sin` | `190.303 ms` |
| `coeffgen_chunked` | 4 | `sin` | `79.578 ms` |

The one-worker chunked baseline is `32.7%` slower than buffered. Four workers recover that cost and parallelize computation, but eight regress by `32.7%` versus four. Thread count is therefore not a substitute for fixing I/O granularity.

### Harvested real Coeff program: `compute_mqlacwaq`

Source: `tests/fixtures/program-m3-oracle/harvested/compute_mqlacwaq.calc.json`.

The saved Coeff source compiles to:

```text
189 tokens
stack_max = 5
94 typed scalar pushes
68 typed binary operations
13 typed unary operations
13 typed poly pokes
1 native transform
47 tokens with arguments
0 dynamic expression references
```

At 16,384 rows, degree 34:

| Work | Median | Added cost |
|---|---:|---:|
| Coefficient function only | `4.743 ms` | baseline |
| Coefficient function + Coeff Program | `23.809 ms` | `19.066 ms` |
| Param Program + coefficient function + Coeff Program | `26.076 ms` | `21.333 ms` |

This is the most important representative case in the review. The Coeff Program adds about `1.164 us/row` on this host.

For scale only, a linear single-core-equivalent extrapolation to a `10,000 x 10,000` parameter square is approximately:

```text
coefficient function only        29 CPU-seconds
coefficient function + program  145 CPU-seconds
full Param + Coeff path         159 CPU-seconds
```

Those are not Lambda wall times; chunking and workers divide wall time, while platform and filesystem differences move the absolute values. They show why a 10% VM improvement matters at large `N`.

### Root Transform and Solve Score direct C costs

Degree 35, direct `-O3` C harness:

| Operation | Median per solved row |
|---|---:|
| Root copy-only harness baseline | `11.540 ns` |
| One rotate | `28.385 ns` |
| Three affine transforms | `42.955 ns` |
| One pull-to-unit-circle | `132.300 ns` |
| Sixteen affine transforms | `227.645 ns` |
| Solve pass-through VM, 2 tokens | `3.445 ns` |
| Solve arithmetic VM, 7 tokens | `12.571 ns` |
| Solve transcendental VM, 7 tokens | `51.103 ns` |
| Solve long VM, 31 tokens | `56.911 ns` |

Metric extraction on the same 35 roots:

| Metric | Median per call |
|---|---:|
| `max_re` | `45.740 ns` |
| `proximity` | `342.150 ns` |
| `min_angular_separation` | `349.540 ns` |
| `clusteriness` | `1,134.680 ns` |

An expanded exploratory metric run also measured approximately:

```text
crowding             1,660 ns
nn_variation           878 ns
shelliness              135 ns
outlierness             340 ns
mean_log_mod             170 ns
sd_log_mod               318 ns
angular_entropy          284 ns
sector_max_share         253 ns
angular_order_2          470 ns
```

Even a 31-token Solve program is cheaper than one simple `max_re` extraction and about 20x cheaper than `clusteriness`. Solve optimization must begin with shared feature extraction, not opcode dispatch.

## F0 - HIGH - The Existing Benchmark Suite Measures the Wrong Things

### Evidence

`tests/bench_vm.py` is useful as an old smoke benchmark, but it cannot validate the optimizations proposed here:

1. It builds through `tests/oracle_runner.py:28-41`, which uses `-O2`. Production uses `-O3` (`deploy.sh:859`).
2. Its `root_raster` case at `tests/bench_vm.py:191-204` does not include a Root Program or root transform. It benchmarks root solving, not the Root VM.
3. Its Solve Score case at `tests/bench_vm.py:117-138` calls Python `eval_solve_score`, not the deployed C evaluator in `solve_score.h`.
4. Coeff cases use buffered `coeffgen`; production fused work uses `coeffgen_chunked`, whose row I/O and threading behavior is materially different.
5. It has no empty-program baseline, no harvested real program, no output hash, no architecture/compiler/git metadata, and no per-phase attribution.
6. `reports/vm_bench_latest.json` can therefore look stable while the deployed Root VM, Solve VM, or chunked Coeff path regresses.

### Required fix

Promote the CR31 probes into a normal, non-hidden benchmark tool:

```text
scripts/bench_program_vms.py
tests/native/vm_microbench.c
reports/vm_bench_<platform>_<git>.json
```

The runner must:

- compile with the deployment optimization level;
- support buffered and chunked Param/Coeff paths;
- directly exercise Root and Solve C evaluators;
- include no-program baselines and `compute_mqlacwaq`;
- alternate baseline/candidate runs for A/B work;
- hash complete outputs;
- record git SHA, dirty state, compiler, flags, architecture, CPU count, and host label;
- report median, minimum, maximum, and preferably median absolute deviation;
- keep host-sensitive timing outside mandatory predeploy pass/fail;
- keep byte/parity/oracle tests in mandatory predeploy.

The current hidden scripts should either be renamed and integrated or removed after integration. Leaving a benchmark that depends on a manually compiled `/tmp` binary is brittle.

## F1 - HIGH - Coeff Rebuilds Static Arguments for Every Token and Row

### Evidence

`evalCoeffProgram` unconditionally declares and resolves arguments before dispatch:

```c
for (int k = 0; k < program->token_count; k++) {
    const CoeffProgramToken *tok = &program->tokens[k];
    CoeffResolvedArgs resolved;
    if (coeffResolveTokenArgs(ctx, tok, &resolved) != 0) return 1;
    switch (tok->op) {
        ...
    }
}
```

Affected code:

- `lambda/sweep_cli.c:4272-4300`
- `lambda/sweep_cli.c:5581-5585`

`CoeffResolvedArgs` contains two eight-double arrays plus metadata, typically 160 bytes after alignment on this host. `coeffResolveTokenArgs` clears the whole object with `memset`, checks counts, copies static arguments, checks expression references, and prepares the optional blend value.

That happens for `push`, `emit`, `dup`, `pop`, `swap`, typed unary/binary operations, and every other argument-free token. In `compute_mqlacwaq`, 142 of 189 tokens have no declared arguments, and none of the 189 tokens has a dynamic expression reference.

### Measured A/B

The controlled candidate skipped resolution only when a token had:

- `n_args == 0`;
- no dynamic optional blend expression;
- a zero static blend value;
- an opcode that does not consume resolved arguments.

Thirty-one interleaved runs produced:

```text
current median     23.666 ms
candidate median   20.900 ms
improvement            11.7%
output SHA         identical
```

This is an end-to-end `coeffgen` timing on the harvested program, not a synthetic opcode loop.

### Required fix

Create a load-time, C-internal token plan. It must not change JSON tokens, versions, source, fingerprints, or persisted execution specifications.

Each prepared token should carry:

```c
enum CoeffArgMode {
    COEFF_ARGS_NONE,
    COEFF_ARGS_STATIC,
    COEFF_ARGS_DYNAMIC
};
```

- `NONE`: dispatch without allocating or clearing `CoeffResolvedArgs`.
- `STATIC`: point directly at immutable token arguments, or copy only the exact number consumed. Do not clear eight unused lanes.
- `DYNAMIC`: run only referenced lowered expression plans into a small frame.

The loader can also precompute whether the optional blend is absent, constant, or dynamic.

### Acceptance target

- `compute_mqlacwaq` Coeff case: at least 10% faster than the current median on the same host/build.
- Argument-free one-token operations: no regression.
- Complete output hashes unchanged.
- `test_coeff_program_native.py`, `test_coeff_source_equivalence.py`, `test_program_m3_oracles.py`, `test_coeff_wire_fingerprints.py`, and `test_whole_sweep_oracle.py` remain green.

## F2 - HIGH - Chunked Coeff Generation Performs Two Syscalls per Row

### Evidence

`coeffGenWorkerMain` loops one step at a time:

- `pread(..., 16 bytes, ...)` at `lambda/sweep_cli.c:9593-9603`;
- generate and evaluate one coefficient row;
- convert one row to floats;
- `pwrite(..., outRowBytes, ...)` at `lambda/sweep_cli.c:9643-9655`.

For 65,536 rows, that is:

```text
65,536 pread calls
65,536 pwrite calls
131,072 total data syscalls
```

At degree 35, each read is only 16 bytes and each write is 280 bytes. The one-thread chunked path is 32.7% slower than the buffered path in the measured matrix.

### Required fix

Workers already own contiguous `[stepLo, stepHi)` ranges. Process blocks of 64-256 rows:

1. `pread` one contiguous parameter block.
2. Compute all rows in the block using the existing global step index for deterministic seeds.
3. Convert results into one contiguous output block.
4. `pwrite` the whole output block.
5. Loop on partial reads/writes and preserve exact error reporting with the first failed global row.

A 128-row block reduces 131,072 data syscalls to roughly 1,024 for this workload, a 128x reduction. It also makes I/O more sequential and reduces contention between workers.

Do not begin with `mmap`. Blocked `pread`/`pwrite` preserves the current explicit offset and failure model with less complexity.

### Acceptance target

- One-thread chunked baseline should close at least half of the current `37.363 ms` gap to buffered generation.
- Four-thread chunked baseline should improve without increasing eight-thread variance.
- Output bytes and row ordering must be identical for 1, 2, 4, and 8 workers.
- Include partial first/last chunk ranges, short-read injection, short-write injection, and deterministic random-chip cases.

## F3 - HIGH - Coeff Stack Semantics Copy Whole Vectors Repeatedly

### Evidence

`CoeffProgramWorkspace` is about 274 KiB per worker (`lambda/sweep_cli.c:3652-3659`). Its 64 vector slots each reserve two 256-double planes. The current ownership model does not exploit those preallocated slots efficiently:

- `coeff_stack_push` copies the complete vector into a new slot (`lambda/sweep_cli.c:3680-3693`).
- source selectors copy vectors into scratch (`lambda/sweep_cli.c:4450-4474`).
- targets copy scratch to poly or copy scratch again into a stack slot (`lambda/sweep_cli.c:4477-4485`).
- typed binary pops two slots, writes scratch, then copies scratch into another slot (`lambda/sweep_cli.c:4704-4745`).
- typed unary pops one slot, writes scratch, then copies scratch into another slot (`lambda/sweep_cli.c:4748-4770`).
- `swap` can perform three complete vector copies (`lambda/sweep_cli.c:5225-5255`).
- every program starts with `cf -> poly` (`lambda/sweep_cli.c:5569`) and ends with `poly -> out` (`lambda/sweep_cli.c:5645`). In chunked generation, `out` is often the same coefficient arrays that supplied `cf`, so alias-aware ownership could avoid at least one boundary copy.

For degree 35, one complex vector is 560 bytes. At the maximum degree 256, it is 4,096 bytes. A simple typed unary currently reads one vector, writes scratch, then reads scratch and writes a stack slot before the result can be emitted to poly.

The benchmark shape supports this diagnosis:

- an empty `emit` still costs about `21 ns/row`;
- degree-35 `neg` costs about `152 ns/row` over baseline;
- degree-35 `sin` costs about `600 ns/row`, where transcendental math finally dominates more of the operation;
- `swap` is metadata semantically but physical data movement operationally.

### Required fix

Represent stack order separately from vector storage:

```c
typedef struct {
    uint16_t slot;
    uint16_t len;
    uint8_t type;
} CoeffStackRef;
```

Use a free-slot list and these rules:

- `swap` exchanges two `CoeffStackRef` records only.
- typed unary writes back into the popped input slot when the value has no other owner.
- typed binary reuses the left or right popped slot as destination when safe.
- `push(poly)` and `push(cf)` copy only when mutation or lifetime requires ownership; immutable references can be tagged read-only.
- `emit` can transfer slot ownership to poly rather than copy.
- direct `poly -> poly` transforms should run in place.
- preserve an original vector only when an operation actually needs `cf`, `original`, or optional blend-back semantics.

This is the highest-risk performance change because stack aliasing, `dup`, `peek`, `cf`, `poly`, blend, and scan establish real ownership constraints. Implement it after F1 and F2, with a debug build that tracks slot owners and rejects use-after-free/double ownership.

### Acceptance target

- Zero vector copies for `swap`.
- At most one vector write for an in-place typed unary on an unaliased temporary.
- At most one destination vector write for an unaliased typed binary.
- No output or diagnostic changes in all Coeff native/parity/oracle tests.
- Benchmark separately at degrees 4, 35, 128, and 256; expected gains grow with degree and token count.

## F4 - HIGH - Solve Recomputes Shared Root Features per Metric Slot

### Evidence

`solve_score_eval_metric_slots` loops over metric slots and calls `compute_solve_metric_score` independently (`lambda/solve_score.h:1606-1617`). Each call begins by checking/filtering finite roots (`lambda/solve_score.h:428-465`) and then computes only one requested metric.

Related metrics repeat the same expensive work:

- `proximity` and `crowding` each run an O(d^2) pair loop (`lambda/solve_score.h:467-502`).
- `clusteriness` and `nn_variation` each compute nearest-neighbor scores (`lambda/solve_score.h:504-533`).
- `mean_log_mod` and `sd_log_mod` each compute log radii; `sd_log_mod` computes them twice (`lambda/solve_score.h:552-581`).
- angular entropy and sector share each build the same 16-bin histogram (`lambda/solve_score.h:643-674`).
- angular-order and minimum-separation metrics independently derive angles/trigonometric values.
- centroid metrics share one centroid within a single call, but a second metric slot invokes the function again and recomputes it.
- shelliness and outlierness each recompute centroid radii (`lambda/solve_score.h:845-873`).
- every distinct clipping quantile can create a separate metric slot even when the raw metric/source feature is the same.

The direct benchmark establishes the priority: a 31-token Solve VM took `56.9 ns`, while `clusteriness` took `1,134.7 ns` and `crowding` about `1,660 ns`.

### Required fix

Build one lazy feature cache per row and source (`solve`, `coeff`, `param`):

```c
typedef struct {
    uint64_t valid_mask;
    int finite_degree;
    const float *finite_roots;
    double centroid_re, centroid_im;
    double log_mod[...];
    double radii[...];
    double nearest_neighbor[...];
    int angle_histogram_16[16];
    /* scalar reductions and pairwise accumulators */
} SolveMetricFeatures;
```

Prepare a program-level feature mask when parsing the Solve Program. Then:

1. Filter finite roots once per source and row.
2. Run one pairwise pass when any pair metric is requested. It can accumulate minimum distance, crowding sum, and each root's nearest-neighbor distance together.
3. Compute centroid and radii once for all centroid/radius/covariance metrics.
4. Compute log modulus once and derive mean/variance with a numerically stable accumulator.
5. Compute angles/histogram/order sums in one angular pass where exact arithmetic order permits it.
6. Normalize/clamp each metric slot from the cached raw scalar afterward.

The cache key is source plus row data, not clipping quantile. Quantiles change normalization, not raw feature extraction.

### Numerical constraint

Combining loops can alter floating-point summation order. Existing byte-exact oracles and metric parity tests are the authority. Preserve each metric's current accumulation order unless a deliberate numeric migration is approved. It is still possible to share distance calculation while maintaining separate accumulators in their original pair order.

### Acceptance target

- A program requesting `proximity`, `crowding`, `clusteriness`, and `nn_variation` from the same source should perform one pair traversal, not four.
- A program requesting both angular histogram metrics should build one histogram.
- A program requesting mean and standard deviation of log modulus should call `hypot`/`log` once per root, not three times.
- Direct multi-metric benchmark should improve by at least 1.5x for a representative mixed program before merge.
- `test_solve_score_native_parity.py`, Solve source equivalence, and whole-sweep oracle remain exact.

## F5 - HIGH - Param's Threaded Row Scheduler Serializes Every Row

### Evidence

`paramGenWorkerMain` (`lambda/sweep_cli.c:8773-8820`) does this for every row:

1. acquire one shared mutex;
2. find/reserve the ring slot for the global next row;
3. release the mutex;
4. compute one row;
5. reacquire the mutex;
6. mark the slot ready;
7. `pthread_cond_broadcast` to all waiters.

The writer thread then acquires the same mutex and waits for every row in strict order (`lambda/sweep_cli.c:8970` onward). The slot ring has only `2 * nThreads` rows (`lambda/sweep_cli.c:8903`). At eight workers, broadcasts and lock handoffs dominate the small row workload.

The benchmark is decisive: eight workers are 5.49x slower than four and slower than one.

### Required fix

Use static contiguous row ranges, matching the simpler partition already used by Coeff workers:

- worker `i` owns a deterministic `[rowLo, rowHi)` range;
- each worker fills a block buffer;
- use offset writes (`pwrite`) for complete blocks, or give each worker a private temporary output and concatenate after join;
- seed random behavior from `(pass, global row, column/token)` rather than shared execution order;
- publish only failure state atomically; no mutex/condition operation on successful rows.

If strict streaming order is required, use a bounded queue of multi-row blocks and targeted signaling, not a broadcast for every row.

Cap effective worker count by:

```text
min(requested_threads, online_cpus, row_count, useful_parallelism_cap)
```

Do not expose host CPU detection as a reproducibility input to fingerprints. It is an execution-plan choice only.

### Acceptance target

- Eight workers must not be more than 30% slower than the best lower thread count on the benchmark host; ideally it should be no worse than four after CPU capping.
- One-, two-, four-, and eight-worker outputs must be byte-identical.
- Random/dither programs need explicit thread-count invariance tests.
- Partial row ranges and failed-worker cleanup must remain correct.

## F6 - MEDIUM - Param Has No Real Load-Time Execution Plan

### Evidence

The Python compiler emits `uses_legacy_fast_path` (`lambda/param_program_chain.py:1438`). The C parser stores it (`lambda/sweep_cli.c:6462`, `6705-6706`). No evaluator reads it. The name claims an optimization that does not exist.

Param scalar expressions remain arrays of double triples (`ParamScalarExpr` at `lambda/sweep_cli.c:6454-6465`) and are decoded on every evaluation in `paramEvalScalarExpr` (`lambda/sweep_cli.c:6792-6866`). Coeff already lowers expression triples into compact typed tokens once at load time (`CoeffLoweredExprPlan` at `lambda/sweep_cli.c:3558-3578`).

`paramEvalProgram` also:

- zeros all eight complex registers on every row whether used or not (`lambda/sweep_cli.c:7093-7096`);
- sanitizes the top stack value after every token (`lambda/sweep_cli.c:7311`), including tokens that cannot create a non-finite value;
- interprets structural shortcut sequences one token at a time even when they have an exact one-token native-registry equivalent.

The equivalent unit-circle benchmark shows a 30.1% gap between expression form and one native token, with identical output bytes.

### Required fix

Prepare a Param execution plan once after JSON parsing:

- lower expression triples into compact structs, as Coeff does;
- compute `register_read_mask` and `register_write_mask`; initialize only registers whose initial zero value can be observed;
- classify tokens by whether they can produce non-finite values and sanitize only at those boundaries plus final outputs;
- fuse exact known patterns into internal superinstructions, such as `push Px -> unary -> emit Px` for both registers;
- route eligible structural shortcuts to the native-registry operation internally;
- remove or rename `uses_legacy_fast_path` unless it is made a real checked plan property.

Stored tokens, source, display, fingerprints, and M3 execution specifications remain unchanged. The plan is derived after parsing and discarded after the process exits.

### Acceptance target

- Equivalent six-token/native one-token mappings remain byte-identical.
- The shortcut benchmark should approach the one-token runtime without changing compiler output.
- Dynamic expressions, registers, stack selectors, and random/dither legacy transforms remain on conservative paths.
- Param native, M3 oracle, source-equivalence, and whole-sweep tests remain green.

## F7 - MEDIUM - Coeff Element Kernels Dispatch Inside the Vector Loop

### Evidence

The unary loop calls `coeffProgramApplyUnaryFn(tok->fn_index, ...)` for every element (`lambda/sweep_cli.c:4628-4638`). Binary loops do the same through `coeffProgramApplyBinaryFn` (`lambda/sweep_cli.c:4615-4623`, `4736-4743`). The helper then selects the operation through a long `if/else` ladder.

The operation is constant for the whole vector, but its selection remains inside the element loop. This makes autovectorization and operation-specific simplification harder, especially for cheap `neg`, `conj`, `real`, `imag`, `add`, and `subtract` kernels.

A synthetic degree-35 `neg` microkernel measured approximately `20.0 ns` through a deliberately generic non-inlined dispatcher versus `3.5 ns` for a specialized loop, a 5.7x kernel-only upper bound. That is **not** an expected whole-program gain: current Coeff results also pay stack and copy costs from F3.

### Required fix

Select once outside the loop:

```c
switch (tok->fn_index) {
case COEFF_VEC_NEG:
    for (...) { out_re[i] = -in_re[i]; out_im[i] = -in_im[i]; }
    break;
case COEFF_VEC_CONJ:
    ...
}
```

Specialize cheap arithmetic first. Keep shared scalar helpers for complex/transcendental operations where code duplication would be large and the math dominates.

Use compiler vectorization reports and assembly inspection as supporting evidence, but accept changes only on end-to-end Coeff benchmarks after F3 removes redundant copies.

## F8 - MEDIUM - Root Transform Repeats Constant Setup per Row

### Evidence

`apply_root_xforms` (`lambda/root_xforms.h:340-377`) resolves `fn_index`, applies default arguments, and dispatches every transform for every solved row. The transform arguments are constant for the job.

Examples:

- `rt_rotate_roots` recomputes `theta`, `cos(theta)`, and `sin(theta)` on every row (`lambda/root_xforms.h:214-221`).
- pull transforms recompute `1 / sigma^2` on every row (`lambda/root_xforms.h:226-238`, `324-335`).
- defaults and name fallback are selected repeatedly in `apply_root_xforms`.

A controlled rotate predecode probe moved `sin/cos` to plan construction and reduced the rotate harness from about `20.3 ns` to `15.3 ns`, roughly **25%** in that interleaved run. The standalone current run was noisier (`28.385 ns` including harness copy), but confirms that one rotate is small enough for constant setup to matter.

### Required fix

Parse `RootXformEntry` into an internal prepared entry:

```c
typedef struct {
    uint8_t op;
    double a, b, c, d;
    double sin_theta, cos_theta;
    double inv_sigma2;
} PreparedRootXform;
```

Resolve names, defaults, and derived constants once. Keep the transform loop and float stores in the same order.

Do not fuse adjacent affine transforms initially. Each current transform writes float32 roots before the next transform. Algebraic fusion would keep intermediate precision and can change output bits, poles, and downstream solve metrics. Fusion is a separate migration requiring exact-oracle evidence or an explicit numerical policy change.

## F9 - MEDIUM - Internal Plan Metadata and Exact Peepholes Are Missing

The four evaluators repeatedly rediscover facts that are fixed for one loaded program:

- Param register use, dynamic-argument use, and fast-path eligibility;
- Coeff static/dynamic argument class, source usage, output ownership, and operation class;
- Root derived constants;
- Solve lag use and lag source by repeated token scans (`lambda/solve_score.h:1549-1578`), plus metric feature families.

Create per-kind prepared plans with shared design rules, not one forced common struct. The language semantics differ; the lifecycle does not:

```text
persisted wire program
    -> validate/parse
    -> prepare immutable internal execution plan
    -> evaluate plan for many rows
```

Safe plan-time peepholes include only identities proven exact under current storage precision and selectors. The benchmark shows that 16 `rev` operations and 16 `neg` operations execute in full. Adjacent `rev(poly)` pairs are candidates because reversal is a permutation; `neg(neg(poly))` needs signed-zero and non-finite parity tests before cancellation. Any optional blend, stack observation, `cf` reference, `peek`, `dup`, scan, or dynamic argument blocks local cancellation.

Prepared-plan optimization must never rewrite persisted source, token arrays, fingerprints, display text, or saved artifacts.

## F10 - LOW - Performance Attribution Is Too Coarse

Most native modes report one `elapsed_us`. That cannot distinguish:

- parameter generation;
- coefficient function execution;
- Param VM;
- Coeff VM;
- root solve;
- Root Transform;
- Solve metric extraction;
- Solve stack VM;
- input/output syscalls.

Add opt-in diagnostic counters for benchmark/debug builds:

```text
rows
tokens_by_kind
dynamic_arg_evals
vector_bytes_copied
metric_feature_passes
pairwise_distance_pairs
pread_calls / pwrite_calls
mutex_waits / condition_wakeups
phase elapsed_us
```

Counters must be disabled in normal production builds or sampled because atomic/per-token accounting can itself distort the hot path. A compile-time `PP_VM_PERF` mode is preferable to unconditional instrumentation.

## Optimizations Not Recommended First

### Computed goto

It may reduce opcode dispatch in Param/Coeff/Solve, but measured Solve dispatch is already tiny, and Coeff/Param have larger argument, copy, expression, I/O, and synchronization costs. Revisit only after F1-F6 with an A/B benchmark.

### JIT compilation

The deployment is a static musl binary in Lambda. A JIT adds executable-memory policy, cold-start compilation, cache, security, and architecture complexity. Prepared plans plus specialized kernels capture most low-risk benefits first.

### `-ffast-math`

Reject it. The code relies on NaN/pole propagation, finite checks, signed zero, exact branch behavior, and byte/parity oracles. Fast-math can invalidate all of those assumptions.

`-fno-math-errno` may be evaluated separately because the hot code does not intentionally consume `errno`, but it is lower priority than structural fixes and still requires exact output checks.

### More workers by default

Current data disproves this. Four beats eight in both Param and chunked Coeff on the review host. Fix row scheduling and block I/O, then tune on the deployed Lambda CPU allocation.

### Broad algebraic fusion

Coeff and Root operations often store intermediate results at defined precision or expose stack/source state. Fusion can change rounding and observability. Restrict early peepholes to exact local identities with output-hash proof.

### SIMD before ownership cleanup

Specialized SIMD kernels can make arithmetic faster while leaving two or three complete vector copies around each operation. Remove copies first, then optimize kernels. Transcendental vector math may eventually benefit from a vetted vector library, but that would be a numerical dependency change, not a simple compiler flag.

## Implementation Plan

### Phase 0 - Make the measurements trustworthy

1. Promote the CR31 harnesses to explicit non-hidden benchmark sources.
2. Compile with the same optimization level as deployment.
3. Add production `coeffgen_chunked`, direct Root, and direct Solve cases.
4. Add empty baselines, `compute_mqlacwaq`, thread sweeps, and output hashes.
5. Record platform metadata and preserve a Graviton baseline.
6. Keep timing advisory; keep correctness/parity/oracle tests mandatory.

### Phase 1 - Low-risk prepared metadata

1. Implement Coeff argument modes from F1.
2. Lower Param expressions once at load time.
3. Predecode Root defaults and constants.
4. Precompute Solve lag/source/feature masks.
5. Remove or make real the dead `uses_legacy_fast_path` field.

Expected result: immediate 10%+ improvement on the harvested Coeff program, 15-25% on simple Root transforms, and smaller Param/Solve setup wins.

### Phase 2 - Fix throughput plumbing

1. Batch Coeff chunk reads/writes in 64-256-row blocks.
2. Replace Param's shared row scheduler with static ranges and block output.
3. Cap effective threads by online CPUs and work size.
4. Benchmark 1/2/4/8 workers on Lambda, not only the development host.

Expected result: remove much of the 32.7% one-thread chunked penalty and eliminate the eight-worker Param cliff.

### Phase 3 - Cache Solve features

1. Add source-scoped `SolveMetricFeatures`.
2. Share finite-root filtering.
3. Share pairwise, nearest-neighbor, centroid/radius, log-modulus, and angular passes.
4. Preserve metric accumulation order and exact parity.
5. Add direct multi-metric benchmark cases.

Expected result: large gains for solve-score programs using related metrics; little change for one trivial metric, which is acceptable.

### Phase 4 - Redesign Coeff vector ownership

1. Add stack slot references/free-list ownership in debug mode.
2. Make `swap` metadata-only.
3. Reuse popped slots for typed unary/binary destinations.
4. Transfer ownership on `emit` where safe.
5. Avoid boundary copies when input/output aliasing is valid.
6. Extend to `dup`, `peek`, blend, scan, slice, and native transforms only after ownership invariants are tested.

Expected result: strongest gains for high-degree, long, cheap-arithmetic programs.

### Phase 5 - Specialized kernels and exact superinstructions

1. Move cheap unary/binary operation selection outside vector loops.
2. Add Param exact superinstructions in the prepared plan.
3. Add narrowly proven Coeff peepholes.
4. Re-run compiler vectorization reports and architecture-specific benchmarks.

Do this last because F1-F4 change the cost profile; optimize the remaining measured bottleneck, not today's secondary symptom.

## Required Correctness Gates

Every optimization in this document is intended to be wire-neutral and output-preserving. At minimum run:

```text
tests/test_param_program_native.py
tests/test_param_program_chain.py
tests/test_param_program_source.py
tests/test_program_m3_oracles.py
tests/test_coeff_program_native.py
tests/test_coeff_program_chain.py
tests/test_coeff_source_equivalence.py
tests/test_coeff_wire_fingerprints.py
tests/test_root_program_source.py
tests/test_root_transform_registry.py
tests/test_solve_score_native_parity.py
tests/test_solve_score_source_equivalence.py
tests/test_whole_sweep_oracle.py
```

Add byte-level invariance cases for:

- all benchmark programs at 1/2/4/8 workers;
- signed zero, NaN/poles, and subnormal values;
- random/dither tokens across thread counts;
- Coeff `dup`, `swap`, `peek`, blend, scan, slice, `cf`, and `poly` aliasing;
- Root float32 intermediate rounding;
- Solve metric combinations sharing a feature cache;
- partial chunk ranges and injected short I/O.

## Performance Acceptance Matrix

Record a before/after report on the same host and on the deployed Graviton class.

| Case | Current local median | Initial target |
|---|---:|---:|
| Harvested `compute_mqlacwaq`, Coeff Program | `23.809 ms` | `<= 21.4 ms` after F1 |
| Chunked degree-35, 65,536 rows, 1 worker | `151.519 ms` | `<= 132.8 ms` after batching |
| Chunked degree-35, 65,536 rows, 4 workers | `70.248 ms` | no regression; improve if I/O-bound |
| Param expression, 262,144 rows, 4 workers | `4.431 ms` | no regression |
| Param expression, 262,144 rows, 8 workers | `24.311 ms` | `<= 1.3x` best lower worker count |
| Root rotate, degree 35 | `28.385 ns` standalone run | 15-25% A/B gain from predecode |
| Solve 31-token VM | `56.911 ns` | not a priority; no regression |
| Solve same-source related metric bundle | add Phase-0 baseline | `>= 1.5x` after feature cache |

Targets are minimum useful outcomes, not promises. If a phase misses its target, inspect counters and revert rather than retaining complexity on the assumption that it should have helped.

## Final Recommendation

Implement F0, F1, F2, and the Root/Solve prepared metadata first. They are well-bounded, measurable, and do not require changing persisted programs. Then replace Param scheduling and add Solve feature caching. Treat the Coeff ownership redesign as a dedicated change with debug invariants and exhaustive oracle coverage.

The key architectural correction is simple: **parse once, prepare once, evaluate many rows with no repeated static work**. The current code parses once but still resolves static arguments, defaults, feature requirements, vector ownership, and row coordination inside hot loops. Removing that repeated work will produce larger and safer gains than changing the opcode switch.
