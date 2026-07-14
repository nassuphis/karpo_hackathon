# Code Review 33: Param, Coeff, and Solve Score Performance

Date: 2026-07-13  
Reviewed HEAD: `4c1cdf5` (`Freshness check manifest hash overrides mtime`)  
Review mode: adversarial performance audit, production-path tracing, native macrobenchmarks, direct C microbenchmarks, and disposable source A/B prototypes

## Verdict

There is still substantial speed available after CR31 and CR32. The remaining work is not one interpreter trick. The three program systems now have three different dominant costs:

- **Param Program:** the deployed fused handler accidentally selects the old stdout scheduler even though it writes to a seekable local file. After that is fixed, static legacy arguments and transform constants are still decoded or recomputed for every point.
- **Coeff Program:** cheap vector operations still copy whole coefficient vectors through scratch buffers. Large source-authored scalar programs also expand into long typed-token streams that repeatedly dispatch and move one scalar at a time.
- **Solve Score:** the stack VM is already cheap. Root-feature extraction, repeated scans, full sorting for medians, and per-row planning dominate it by one to three orders of magnitude.

The highest-value changes are concrete and wire-neutral:

1. Pass the actual Param output path to `sweep_coeffgen` from the fused handler. The current `"-"` argument forces the legacy ordered-ring scheduler. The actual-file path reduced the same production-shaped Param request by **20.5% at two workers** and **14.5% at four workers**, with byte-identical output. At eight workers the old path became pathological.
2. Add a load-time Param legacy execution plan. A prepared `rect` prototype that resolves static arguments and precomputes invariant geometry reduced that transform by **26.5%**, while leaving the dynamic-argument path unchanged.
3. Add direct in-place Coeff kernels for owned `poly -> poly` unary, binary, and native operations. The prototype reduced one cheap degree-35 operation by **5-26%**, degree-128/256 add by about **26%**, and sixteen repeated negations by about **77-78%**.
4. Stop computing a centroid for Solve metrics that do not use it. This local change reduced representative extrema, modulus, and distance metrics by roughly **7-20%**.
5. Build Solve metric-family plans and compute related features in shared passes. Disposable exact-order prototypes reduced representative multi-metric families by **41-81%**.
6. Replace full `qsort` with a deterministic bounded selection algorithm where only a median is required. The axis-median family fell from about `967 ns` to `210 ns` at degree 35 in the prototype.
7. Fuse straight-line typed Coeff scalar subgraphs internally. The real 189-token `compute_mqlacwaq` source program was represented by an equivalent 14-operation prepared form and ran **30.6% faster**, with identical complete output.

These percentages are not additive. They apply to different stages and program shapes. The first job is to fix the production scheduler selection; otherwise deeper Param VM optimization is benchmarked behind an avoidable orchestration defect.

Do **not** start with computed goto, a JIT, `-ffast-math`, indiscriminate expression memoization, more worker threads, runtime last-value caches, or a broad workspace ownership rewrite. Controlled probes either showed no gain, regressed, or exposed unacceptable semantic risk.

## Findings Summary

Severity means expected performance impact on deployed workloads, not correctness severity.

| ID | Severity | Finding | Evidence |
|---|---|---|---|
| F1 | HIGH | The fused Lambda invokes Param generation with output path `"-"`, selecting the old ordered-ring scheduler despite writing stdout to a regular local file. | Production-shaped A/B: `-20.5%` at 2 workers, `-14.5%` at 4; byte-identical. |
| F2 | HIGH | Static Param legacy arguments and transform constants are resolved, validated, and often recomputed for every generated point. | Static-vs-dynamic overhead of `6.5-19.2 ns/point`; prepared `rect` prototype `-26.5%`. |
| F3 | HIGH | Direct Coeff selector operations copy owned vectors through scratch even when in-place mutation is semantically safe. | Direct-kernel prototype: `-5%` to `-28%` for one cheap op; `-77%` for 16 negations. |
| F4 | HIGH | `compute_solve_metric_score` computes a centroid before metrics that never consume it. | Local prototype: representative affected metrics `-7%` to `-20%`. |
| F5 | HIGH | Solve Score recomputes related O(d), O(d log d), and O(d^2) root features in separate metric calls. | Family prototypes: `-41%` to `-81%` at degree 35. |
| F6 | HIGH | Median-only Solve metrics fully sort their buffers with `qsort`. | Axis-median family: `967 ns` to `210 ns` with selection prototype. |
| F7 | MEDIUM | Source-authored Coeff scalar expressions expand into long typed stack/poke streams without an internal straight-line execution plan. | Harvested `compute_mqlacwaq`: `189` tokens to equivalent 14 prepared ops, `-30.6%`. |
| F8 | MEDIUM | Solve metric ownership, duplicate slots, source engagement, and family requirements are rediscovered on every row; feature storage is also provisioned per call. | Source audit; heap-to-stack alone was neutral, showing planning/fusion is the useful unit. |
| F9 | MEDIUM | Param has exact common stack patterns that can be recognized internally, and its point-at-a-time VM blocks vectorization. | Identity still costs `+15%`; native unit circle materially beats equivalent structural/expression forms. |
| F10 | MEDIUM | Coeff result ownership and the fused process pipeline retain avoidable copies, local-file rereads, and serial stage/upload ordering. | Source audit; requires end-to-end Lambda measurement before redesign. |
| F11 | LOW | Direct Solve evaluation rescans tokens to answer `uses_lag` even after lag preparation. | Source audit; relevant primarily to cheap metrics/programs. |
| F12 | LOW | Current durable benchmarks miss the production Param invocation seam, non-pair Solve bundles, high-degree Coeff direct operations, and harvested programs. | Benchmark inventory audit. |

## Scope

This review covers the deployed execution path for:

| Program kind | Python compile/serialization | Native execution | Production orchestration |
|---|---|---|---|
| Param | `lambda/param_program_chain.py`, `lambda/param_program_source.py` | `paramEvalProgram` in `lambda/sweep_cli.c:7323` | `handler_compute_chunk_fused.py::_run_param_gen_local` |
| Coeff | `lambda/coeff_program_chain.py`, `lambda/coeff_program_source.py` | `evalCoeffProgram` in `lambda/sweep_cli.c:5716` | fused chunk handler and `coeffgen_chunked` workers |
| Solve Score | `lambda/solve_score_chain.py`, `lambda/solve_score_program_source.py` | `lambda/solve_score.h` metric extraction and stack evaluation | solve-score streams and render/solve consumers |

Root Transform execution was deliberately excluded except where it shares orchestration. CR33 is about the three kinds requested. Python compile latency and browser-editor latency were inspected only where they affect runtime payload shape; the dominant deployed costs are native row evaluation and stage orchestration.

## Methodology and Limits

### Host and builds

- Host: Apple M3 Max, arm64.
- Compiler: Apple clang with `-O3` and the same broad optimization level as production.
- Production target: Linux/musl on AWS Graviton. Absolute local timings are not Lambda forecasts.
- Baseline native binary was built from unmodified HEAD before disposable probes.
- Source A/B variants were built separately and compared against the baseline binary.
- Production source files were restored after every probe.

### Measurement rules

- Macrobenchmarks report the native command's internal `elapsed_us` where available.
- Repeated tables use medians; most focused A/Bs use 15, 21, or 31 interleaved repetitions.
- Complete generated files were SHA-256 compared for production-shaped Param and Coeff probes.
- Solve feature prototypes preserved root traversal and accumulator order where claimed byte-compatible.
- Extremely short nanosecond timings are useful for relative ranking, not capacity planning.
- A candidate is not accepted merely because it is theoretically cheaper. Several intuitive changes were slower and are recorded later.

### Interpretation

There are three evidence levels in this review:

1. **Production-path measured:** the real current invocation or evaluator was compared to a minimally changed equivalent path.
2. **Disposable prototype measured:** source was temporarily changed or a focused native probe was built. These numbers establish opportunity, not merge readiness.
3. **Source-audit candidate:** the cost is visible in code, but no reliable end-to-end improvement has yet been demonstrated.

Every finding below states its evidence level. This distinction matters. A 30% isolated kernel win may be a 2% job win if that kernel is rare; a one-line orchestration fix may affect every fused chunk.

## Current Execution Architecture

### Fused chunk sequence

`lambda/handler_compute_chunk_fused.py` currently performs these stages serially:

1. Generate Param rows into a local file.
2. Upload the Param object.
3. Run Coeff generation, rereading that local Param file.
4. Upload the Coeff object.
5. Run the solver, rereading the local Coeff file.
6. Upload roots and report completion.

The local Param artifact already exists in full before Coeff generation starts. There is no actual Param-to-Coeff pipe. That fact makes F1 especially clear: using stdout mode buys no streaming behavior in this handler, but it selects a materially worse scheduler.

### Param native path

`runParamGen` in `lambda/sweep_cli.c:9442` chooses among three execution modes:

- serial generation for one worker;
- `runParamGenThreadedRange` for `outPath == "-"`, because stdout cannot seek;
- `runParamGenThreadedStatic` for a normal file path, using static row ranges and positioned writes.

Each point eventually reaches `computeParamGenRow` (`lambda/sweep_cli.c:8989`) and `paramEvalProgram` (`:7323`). `paramPreparePlan` (`:6874`) currently lowers scalar expressions and derives register-read masks, but does not prepare static legacy operands or transform constants.

### Coeff native path

`evalCoeffProgram` (`lambda/sweep_cli.c:5716`) copies input coefficients into owned `poly` workspace, executes tokens, and copies `poly` to the caller's output. The workspace is deliberately per worker, which solved prior sharing/race problems, but direct-selector handlers still route many owned-vector operations through scratch arrays:

- generic binary kernel: `coeffProgramApplyBinaryFn` (`:4576`);
- generic unary kernel: `coeffProgramApplyUnaryFn` (`:4635`);
- native transform direct path: `coeffProgramNativeTransformOp` (`:5048`).

Typed stack operations already have more specialized in-place paths. The next safe step is not a global ownership rewrite; it is to extend those exact kernels to the direct `poly -> poly` cases where ownership is known.

### Solve Score native path

The Solve stack VM is downstream of metric extraction. `solve_score_eval_metric_slots` (`lambda/solve_score.h:1797`) currently identifies duplicate metrics and pair-family opportunities per row, then calls `compute_solve_metric_score` (`:429`) for most other metrics. `SolveSourceFeatures` is about 16.6 KiB and pair-family caching is the only substantial shared feature plan.

This structure means a seven-token score program may cost tens of nanoseconds while its requested metrics cost hundreds or thousands. VM-dispatch work is therefore not the first target.

## Baseline Measurements

### Solve VM versus metrics

Current direct microbenchmark results:

| Operation | Median ns/evaluation |
|---|---:|
| Solve VM pass, 2 tokens | `2.960` |
| Solve VM arithmetic, 7 tokens | `12.256` |
| Solve VM transcendental, 7 tokens | `48.987` |
| Solve VM long program, 31 tokens | `55.561` |
| Metric `max_re` | `47.360` |
| Metric `proximity` | `329.540` |
| Metric `clusteriness` | `1095.310` |
| Metric `min_angular_separation` | `364.000` |
| Four-slot pair-family bundle | `2527.410` |

The important ratio is not a single absolute number. A cheap metric already costs roughly an entire long VM program; expensive feature extraction costs 6-20 times more. Computed-goto work on the score stack switch would attack the smaller term.

### Production-shaped Param and Coeff samples

Representative current macrobenchmarks:

| Case | Median elapsed |
|---|---:|
| Param baseline | `9.400 ms` |
| Param identity, 4 tokens | `11.172 ms` |
| Param arithmetic, 8 tokens | `12.773 ms` |
| Param native unit circle | `19.702 ms` |
| Param expression unit circle | `22.506 ms` |
| Coeff degree 35 baseline | `28.393 ms` |
| Coeff degree 35 one add | `31.007 ms` |
| Coeff degree 35 one negation | `31.028 ms` |
| Coeff degree 35 sixteen negations | `61.851 ms` |
| Harvested `compute_mqlacwaq` baseline | `4.608 ms` |
| Harvested `compute_mqlacwaq` with Coeff program | `19.578 ms` |
| Same fixture with Param and Coeff programs | `21.548 ms` |

Thread scaling on a representative chunked Param expression remains useful only when the static scheduler is selected:

| Workers | Median elapsed |
|---:|---:|
| 1 | `99.796 ms` |
| 2 | `51.734 ms` |
| 4 | `26.536 ms` |
| 8 | `13.761 ms` |

F1 explains why the fused handler does not necessarily receive that behavior.

## Detailed Findings

## F1 - Fused Param Generation Selects the Wrong Scheduler

**Severity:** HIGH  
**Evidence:** production-path measured  
**Location:** `lambda/handler_compute_chunk_fused.py:274-303`, `lambda/sweep_cli.c:9468-9553`

### Current behavior

The handler opens `output_path` itself and invokes:

```python
with open(output_path, "wb") as out:
    proc = subprocess.Popen(
        [SWEEP_COEFFGEN, "-"],
        stdout=out,
        ...
    )
```

From the operating system's perspective, `stdout` is a regular seekable file. The native binary cannot know that. It branches solely on the literal argument:

```c
int streamMode = (strcmp(outPath, "-") == 0);
...
if (threadsUsed <= 1) {
    ...
} else if (streamMode) {
    runParamGenThreadedRange(...);   /* ordered ring */
} else {
    runParamGenThreadedStatic(...);  /* static ranges + pwrite */
}
```

The CR31 scheduler improvement therefore exists in the native program but is bypassed by the main fused Lambda path.

### Measured impact

Exact handler-shaped request, 524,288 generated points, identical input JSON and output bytes:

| Workers | Current `"-"` path | Actual output-path argument | Delta |
|---:|---:|---:|---:|
| 2 | `18.495 ms` | `14.700 ms` | `-20.52%` |
| 4 | `9.510 ms` | `8.129 ms` | `-14.52%` |
| 8 | `21.110 ms` | `4.267 ms` | `-79.79%` |

The eight-worker result is not a promise of an 80% Lambda gain. It demonstrates that the ordered publication scheduler can collapse under contention while the static scheduler continues scaling. The 2- and 4-worker results are the safer production expectation until measured on Graviton.

### Required implementation

Call the binary with the real output path, matching `_run_coeffgen_local`:

```python
result = subprocess.run(
    [SWEEP_COEFFGEN, output_path],
    input=json.dumps(spec),
    capture_output=True,
    text=True,
    timeout=840,
)
meta = json.loads(result.stdout)
```

The metadata channel changes from stderr in stream mode to stdout in file mode. Error text remains stderr. Do not retain the outer Python file handle; let the native static writer own the file descriptor.

### Correctness and tests

- Pin that `_run_param_gen_local` passes `output_path`, not `"-"`.
- Assert metadata is decoded from stdout on success and stderr is used for failures.
- Run 1-, 2-, 4-, and 8-worker output parity against the current stream path for programs without random transforms.
- For dither/random transforms, preserve and test the current serial-versus-threaded request contract. Scheduler choice must not alter per-row RNG seeding.
- Include range requests with non-row-aligned `step_start` and `step_count`.
- Measure this exact call inside the deployed Graviton Lambda before changing default worker counts.

This should be the first implementation. It is small, high-confidence, and affects the actual deployed path.

## F2 - Param Needs a Real Load-Time Legacy Plan

**Severity:** HIGH  
**Evidence:** production evaluator audit plus disposable measured prototypes  
**Location:** `lambda/sweep_cli.c:6541-6659`, `:6874-6912`, `:7323-7557`

### Current behavior

`paramPreparePlan` prepares scalar expressions and register masks. Legacy tokens still perform work per point that is invariant for static arguments:

- copy argument values into temporary `ParamCx` arrays;
- resolve defaults;
- check whether each declared-real argument has an imaginary component;
- decode source/target behavior;
- recompute constants such as rectangle perimeter and rotation trigonometry.

Dynamic arguments legitimately need per-point evaluation. Static arguments do not.

### Static versus dynamic evidence

262,144 points, semantically equivalent static and dynamic forms:

| Program | Median | Dynamic overhead |
|---|---:|---:|
| No Param Program | about `10.141 ms` | - |
| `rtheta` static | about `20.428 ms` | - |
| `rtheta` dynamic equivalent | about `22.134 ms` | `+1.706 ms`, about `6.5 ns/point` |
| `rect` static | about `15.815 ms` | - |
| `rect` dynamic equivalent | about `20.843 ms` | `+5.028 ms`, about `19.2 ns/point` |

The difference is not entirely argument resolution, but it establishes the cost boundary a static plan can remove.

### Prepared `rect` prototype

A disposable plan classified a static `rect` token at load time, copied and validated its arguments once, and precomputed:

- half width and half height;
- perimeter;
- `cos(angle)` and `sin(angle)`;
- selector/target mode.

It then called a branch-free prepared helper for static rows while preserving the existing path for dynamic arguments.

| Case | Baseline | Prepared | Delta |
|---|---:|---:|---:|
| No Param Program | `8.927 ms` | `9.012 ms` | `+0.95%` noise |
| Static `rect` | `14.924 ms` | `10.967 ms` | `-26.51%` |
| Dynamic `rect` | `19.997 ms` | `20.068 ms` | `+0.36%` noise |

Outputs were byte-identical.

### Required design

Add a prepared record per legacy token, not a global memo cache:

```c
enum ParamArgMode {
    PARAM_ARGS_NONE,
    PARAM_ARGS_STATIC,
    PARAM_ARGS_DYNAMIC
};

typedef struct {
    enum ParamArgMode argMode;
    ParamCx staticArgs[MAX_PARAM_ARGS];
    unsigned srcMode;
    unsigned targetMode;
    unsigned preparedKind;
    union {
        struct { double hw, hh, perimeter, ca, sa; } rect;
        /* Add only measured transform-specific plans. */
    } u;
} ParamPreparedLegacy;
```

Preparation must be derived from the compiled token and registry-backed argument metadata. It must not create another defaults or arity table in C.

Implementation order:

1. Classify `NONE`, `STATIC`, and `DYNAMIC` arguments.
2. Resolve and validate static argument values once.
3. Predecode selectors and target placement.
4. Add `rect` as the first transform-specific prepared kind.
5. Benchmark `rply`, `star`, and `rrect` individually before adding them. Their repeated geometry/trigonometry suggests opportunity, but resemblance is not evidence.

### Rejected shortcut: runtime last-value memoization

A thread-local cache keyed by the last runtime argument values was slower:

- static `rect`: about **13.7% slower**;
- dynamic `rect`: about **11.9% slower**.

The per-point key comparisons and branch cost exceed the saved setup. Preparation must happen once at parse/load time, where staticness is already known.

### Correctness and tests

- Static and dynamic equivalent forms must produce byte-identical rows.
- Include real and complex-invalid arguments, omitted defaults, every selector, every target mode, `pop` sources, and stack targets.
- Preserve exact random/dither state progression; never classify stateful transforms into a pure prepared fast path without explicit RNG handling.
- Test negative zero, infinities, NaNs, and the current sanitize boundaries.
- Keep M3/wire/fingerprint payloads unchanged. This is an internal native execution plan only.

## F3 - Coeff Direct Selector Paths Still Copy Owned Vectors

**Severity:** HIGH  
**Evidence:** disposable source A/B, complete output parity  
**Location:** `lambda/sweep_cli.c:4576-4685`, `:5048-5162`, `:5716-5822`

### Current behavior

The evaluator owns its `poly` vector inside `CoeffProgramWorkspace`. Nevertheless, direct operations commonly compute into scratch and copy back:

```text
poly -> generic unary/binary/native helper -> scratch -> poly
```

For a degree-256 polynomial, each unnecessary pass moves multiple KiB. Cheap arithmetic is then memory traffic plus dispatch rather than arithmetic.

Typed-stack paths already contain specialized in-place operations for several cheap opcodes. Direct-selector paths do not consistently reuse them.

### Measured prototype

The prototype made only ownership-safe cases in-place:

- direct unary `poly -> poly`;
- direct binary `poly, poly/cf -> poly`;
- native `poly -> poly` with `andy == 0` where the native operation permits it;
- operation selection hoisted outside the element loop.

All tested finite complete-output hashes matched.

| Case | Baseline | Candidate | Delta |
|---|---:|---:|---:|
| Degree 35 add | `1.712 ms` | `1.566 ms` | `-8.53%` |
| Degree 128 add | `5.917 ms` | `4.375 ms` | `-26.06%` |
| Degree 256 add | `12.029 ms` | `8.865 ms` | `-26.30%` |
| Degree 35 negation | `1.668 ms` | `1.238 ms` | `-25.78%` |
| Degree 35 reverse | `1.340 ms` | `1.273 ms` | `-5.00%` |
| Degree 35 sine | `2.514 ms` | `2.430 ms` | `-3.34%` |

Repeated cheap operations amplify the result. Earlier controlled runs showed sixteen negations improving by about **77-78%** at degrees 35, 128, and 256.

### Required implementation

Create shared exact kernels rather than duplicating arithmetic in a new fast path:

```c
coeffUnaryKernel(fnIndex, dstRe, dstIm, srcRe, srcIm, n);
coeffBinaryKernel(fnIndex, dstRe, dstIm, aRe, aIm, bRe, bIm, n);
```

The kernels must own the existing nonfinite and error policy. Both typed-stack and direct-selector execution should call them. Then add explicit alias-safe entry cases:

- `dst == a` for add/subtract/multiply where the element is read before overwrite;
- `dst == src` for unary operations;
- immutable `cf` can be a source but never an in-place destination;
- length-changing native transforms need their own proven rules and must not be swept into the generic case.

Do not begin with a complete slot-ownership or copy-on-write rewrite. Direct `poly -> poly` cases capture a measured win with a much smaller semantic surface.

### Correctness and tests

- Compare full output bytes for every unary/binary opcode at degrees 0, 1, 35, 128, and 256.
- Include `poly/poly`, `poly/cf`, and the reverse source order where legal.
- Exercise aliasing explicitly.
- Pin overflow, underflow, NaN, infinity, signed zero, and divide-by-zero behavior.
- Test `andy == 0`, `andy == 1`, and intermediate blending separately. Initial native in-place work should remain limited to the proven `andy == 0` cases.
- Keep source serialization, execution specs, M3 oracles, and wire fingerprints unchanged.

## F4 - Solve Computes Centroids for Metrics That Do Not Use Them

**Severity:** HIGH  
**Evidence:** disposable local A/B  
**Location:** `lambda/solve_score.h:429-919`

### Current behavior

`compute_solve_metric_score` filters finite roots and computes the centroid before its metric switch. Several branches never read that centroid:

- distance from the unit circle;
- maximum/minimum real component;
- maximum/minimum imaginary component;
- minimum/maximum modulus;
- minimum angular separation.

For these metrics, the centroid pass is pure overhead.

### Measured impact

A conditional centroid prototype preserved existing metric bodies and accumulator order. Representative local reductions were:

| Metric family | Approximate reduction |
|---|---:|
| Distance from unit circle | `11.8%` |
| Minimum modulus | `7.5%` |
| Maximum modulus | `16.4%` |
| Real extrema | about `20%` |

Centroid-dependent metrics were effectively unchanged within local noise.

### Required implementation

Use a static predicate or switch grouping to compute centroid only for metrics that consume it. Do not infer this from metric names in multiple places. The eventual family plan in F5 should carry `needCentroid`; F4 can land first as a small direct fix and later be subsumed by that plan.

### Tests

- Native parity for every metric, not only affected metrics.
- Degree zero, one, and normal degree.
- Partially nonfinite roots and all-nonfinite roots.
- Byte comparison where score output is serialized.
- Verify no metric accidentally used an uninitialized centroid after regrouping.

## F5 - Solve Metrics Need Shared Feature-Family Passes

**Severity:** HIGH  
**Evidence:** source audit plus disposable C microbench prototypes  
**Location:** `lambda/solve_score.h:429-919`, `:1639-1912`

### Current behavior

The current evaluator has useful pair-family sharing and duplicate-slot reuse. Most non-pair metrics still fall through independently to `compute_solve_metric_score`. A score requesting several related metrics repeatedly:

- scans and filters the same roots;
- recomputes modulus, logarithm, angle, or centroid;
- allocates/fills related temporary buffers;
- sorts or traverses the same values again.

This is the dominant Solve Score opportunity.

### Measured family prototypes

Degree-35 roots, current production entry points versus family-specific fused passes:

| Feature family | Metrics in probe | Current | Fused | Reduction |
|---|---:|---:|---:|---:|
| Radial | 7 | `1060.86 ns` | `257.45 ns` | `75.7%` |
| Angular histogram | 2 | `533.39 ns` | `278.77 ns` | `47.7%` |
| Angular orders | 3 | `1397.24 ns` | `816.84 ns` | `41.5%` |
| Centroid/covariance | 9 | `992.71 ns` | `403.49 ns` | `59.4%` |
| Real/imag extrema | 4 | `196.05 ns` | `38.28 ns` | `80.5%` |
| Axis medians | 3 | `967.38 ns` | `894.63 ns` | `7.5%` before selection work |

These are family-bundle results. A program requesting only one metric may see little benefit and must retain a low-overhead single-metric path.

### Safe family definitions

The following sharing is straightforward if operation order within each metric is preserved:

- **Radial:** compute modulus and log modulus once per root; accumulate inside-circle, annulus, distance, min/max modulus, and mean/SD inputs.
- **Angular histogram:** one binning pass produces entropy and maximum-share inputs.
- **Angular orders:** one `atan2` per root can feed requested orders. Keep separate `sin`/`cos` calls for each order if byte identity matters; a recurrence changes rounding.
- **Centroid/covariance:** centroid once, then a shared second pass for covariance/radius families while preserving each accumulator order.
- **Extrema:** max/min real and imaginary in one pass.
- **Axis buffers:** fill real, imaginary, and derived axis buffers in one root pass; selection then dominates.

### Required plan

At program preparation time, derive:

- engaged sources;
- metric owner for duplicate slots;
- required feature-family mask per source;
- requested outputs inside each family;
- pair-family requirements such as crowding, nearest-neighbor, and proximity;
- whether a single-metric direct path is cheaper.

At row evaluation time, run each engaged family once and scatter results to metric slots. The VM itself remains unchanged.

### Pair-family refinement

`solve_features_pair_pass` currently computes minimum squared pair distance even when a requested pair-family bundle does not need proximity. Carry a `needProximity` flag in the plan and skip that reduction when absent.

### Correctness traps

- Do not change summation order merely to combine formulas.
- Do not replace trigonometric calls with recurrence identities without an explicit numerical-contract decision.
- Preserve finite filtering exactly.
- Preserve duplicate metric behavior and source-specific roots.
- Param-source metrics use different data and cannot blindly share root-family buffers.
- Lagged sources require worker-local state and cannot share current-row features across time.

## F6 - Median-Only Metrics Should Not Fully Sort

**Severity:** HIGH  
**Evidence:** disposable C microbench prototype  
**Location:** `lambda/solve_score.h:314-319` and median call sites

### Current behavior

`median_inplace` calls `qsort`, paying O(d log d) and fully ordering a buffer when only one or two middle values are required. This affects:

- real/imaginary/radial medians;
- axis-median family;
- outlierness and clusteriness helpers that consume a median.

It does not replace sorting where the full order is semantically required, such as minimum angular separation in its current implementation.

### Measured opportunity

For the degree-35 three-axis family:

| Implementation | Median |
|---|---:|
| Current separate paths | `967.38 ns` |
| Shared fill + `qsort` | `894.63 ns` |
| Shared fill + quickselect prototype | `209.76 ns` |

The simple quickselect probe establishes the opportunity but is not production-ready. A poor pivot can degrade to O(d^2), which is unacceptable on adversarial or ordered inputs.

### Required implementation

Implement deterministic introselect/nth-element behavior:

- deterministic pivot strategy;
- depth or progress bound;
- fallback to `qsort` or heap selection;
- exact even-length median rule matching current behavior;
- no undefined comparator behavior for unusual floating values.

For even `d`, select both middle order statistics or select the upper middle and find the maximum of the lower partition, then apply the current arithmetic order.

### Tests

- Compare against sorted reference across random arrays and degrees 0 through the supported maximum.
- Include sorted, reverse-sorted, all-equal, duplicate-heavy, alternating, and pivot-adversarial arrays.
- Include signed zeros and finite extreme magnitudes.
- Nonfinite values should be filtered before selection exactly as today.
- Benchmark degrees representative of production, not only degree 35.

## F7 - Coeff Scalar Programs Need Internal Straight-Line Fusion

**Severity:** MEDIUM  
**Evidence:** harvested real program and measured equivalent prototype  
**Location:** typed scalar execution in `lambda/sweep_cli.c`

### Current behavior

The source-authored Coeff compiler correctly lowers expressions to typed stack operations. A large scalar assignment performs many native dispatches:

```text
push scalar -> push scalar -> binary op -> ... -> typed poke
```

The real `compute_mqlacwaq` program contains 13 `poly[i] = expr` assignments followed by `poly = rev(poly)`. Its current compiled shape contains:

- 189 total tokens;
- 94 typed scalar pushes;
- 68 typed binary operations;
- 13 typed unary operations;
- 13 typed pokes;
- 1 native transform;
- maximum stack depth 5;
- no dynamic scalar expression references.

### Measured equivalent

A disposable equivalent representation used 13 prepared scalar-expression pokes plus reverse:

| Form | Operations | Median for 16,384 degree-34 rows |
|---|---:|---:|
| Current typed stream | 189 tokens | `19.168 ms` |
| Prepared straight-line scalar form | 14 operations | `13.298 ms` |
| Improvement | - | `30.62%` |

The full output SHA-256 matched.

### Correct implementation boundary

Do not change source compilation, execution specs, serialized chains, or fingerprints. Build an internal C preparation pass that recognizes straight-line typed scalar subgraphs ending in a poke and emits a compact scalar RPN/superinstruction plan.

The plan must execute the **typed scalar semantics**, not reuse the existing general expression evaluator blindly. The two paths differ in some nonfinite and divide behavior. Reusing the wrong evaluator would trade speed for silent semantic drift.

Safe initial shape:

- only scalar-stack operations with no control/state effects;
- one final typed poke;
- no reads from mutable `tos`/scan state unless dependency tracking proves order;
- preserve opcode order inside the fused region;
- use the same primitive arithmetic helpers as the typed VM;
- fall back to the normal token loop on any unsupported opcode.

### Why this ranks below direct vector kernels

The harvested program demonstrates a real workload, but the fixture corpus is small: most other sampled programs are short native-transform programs. Direct vector kernels help common cheap operations with less machinery. Add production token-shape telemetry before broad scalar fusion.

## F8 - Solve Planning and Workspace Are Still Per-Row Work

**Severity:** MEDIUM  
**Evidence:** source audit and negative isolated-allocation probe  
**Location:** `lambda/solve_score.h:1797-2005`

### Current behavior

`solve_score_eval_metric_slots` scans metric slots each row to determine:

- duplicate metrics;
- pair-family owners;
- engaged root sources;
- feature needs.

It also provisions feature structures around each evaluation. `SolveSourceFeatures` is roughly 16.6 KiB, so casually placing multiple instances on every call stack is not a clean fix.

### What the probe showed

Moving feature storage from heap to stack by itself changed pair/nearest-neighbor cases by only about `0-2%`. Allocation removal is not the main opportunity. Shared feature passes and prepared requirements are.

### Required design

Extend `SolveScoreProgram` with immutable prepared metadata and add a worker-local `SolveMetricWorkspace`:

```c
typedef struct {
    unsigned engagedSources;
    unsigned familyMask[SOLVE_SOURCE_COUNT];
    unsigned pairNeeds[SOLVE_SOURCE_COUNT];
    int metricOwner[MAX_SOLVE_METRICS];
    int usesLag;
} SolveMetricPlan;

typedef struct {
    /* Reusable buffers sized for the maximum supported degree. */
    double scratchA[MAX_DEGREE];
    double scratchB[MAX_DEGREE];
    double scratchC[MAX_DEGREE];
    SolveSourceFeatures source[SOLVE_SOURCE_COUNT];
} SolveMetricWorkspace;
```

The exact storage layout should be sized after stack and worker-count analysis. The existing lag stream is a natural owner because it is already worker-local. Direct no-lag callers need either a caller-owned workspace or a bounded local fast path.

Keep the one-slot direct path. Preparing a large workspace to return one `max_re` score would be a regression.

## F9 - Param Has Exact Peephole and Batch Opportunities

**Severity:** MEDIUM  
**Evidence:** current benchmark and compiler vectorization report  
**Location:** `lambda/sweep_cli.c:7323-7557`

### Exact internal peepholes

An identity Param program still adds about 15% over the no-program baseline. Three semantically equivalent unit-circle forms have materially different costs:

| Form | Approximate median |
|---|---:|
| Native registry token | `16.64 ms` |
| Structural shortcut | `20.37 ms` |
| Expression form | `23.81 ms` |

This supports an internal preparation pass that recognizes exact canonical patterns such as:

- identity push/emit sequences;
- targeted unary assignment patterns;
- structural sequences exactly equivalent to one native transform.

The plan must not rewrite source, chain, execution spec, or fingerprint. It chooses an evaluator only after compilation.

Stateful/random transforms are excluded unless the fast path explicitly preserves the same per-row and per-lane RNG sequence. Algebraic equivalence is not sufficient; sanitize and nonfinite boundaries must also match.

### Batch VM

Compiler vectorization reports show:

- several Coeff outer arithmetic kernels vectorize;
- Param's point/token loop does not;
- lowered Coeff scalar-expression loops do not.

A longer-term Param design can evaluate a small block of points with token-outer, lane-inner execution and structure-of-arrays registers. That amortizes token dispatch and exposes arithmetic lanes to NEON.

This is high effort because:

- stack depth and dynamic expressions become lane vectors;
- selectors and target modes can diverge;
- random/dither sequencing is part of output behavior;
- sanitization must occur at the same semantic boundaries.

Do it only after F1 and F2, and begin with pure deterministic opcodes under byte-parity tests.

## F10 - Coeff Ownership and Fused Pipeline Have Architectural Headroom

**Severity:** MEDIUM  
**Evidence:** source audit; end-to-end gain unmeasured

### Final Coeff copy

`evalCoeffProgram` copies caller input into workspace `poly` and copies final `poly` back out. The caller often immediately serializes or converts that output. A result-view API could let the caller consume workspace `poly` directly and remove the final copy.

An even more aggressive no-`cf`-reference mode could operate directly on caller-owned output and remove both boundary copies, but only after a dependency plan proves the program never reads immutable original `cf`. Alias semantics make this materially riskier than F3.

### Combined Param and Coeff native stage

The fused Python handler still launches separate native processes and writes/reads a local Param file between them. A combined native mode could:

1. statically schedule rows once;
2. evaluate Param values;
3. feed them directly into Coeff generation;
4. write both required artifacts without rereading Param data.

This can retain the existing S3 object contract while removing process startup and local reread. It is an architectural candidate, not the first fix. F1 captures much of the immediate Param scheduling gain with far less risk.

### Upload overlap

The handler can potentially overlap:

- Param upload with local Coeff generation;
- Coeff upload with local solve.

The local files already exist and downstream work is local. Uploads must be joined before success, and failures must not delete files still being read or uploaded. On small chunks, thread and network setup may erase the gain. Measure in Lambda before adding concurrency.

## F11 - Solve Lag Use Is Rescanned

**Severity:** LOW  
**Evidence:** source audit  
**Location:** `lambda/solve_score.h:1568-1576`, `:2242-2243`

`solve_score_program_uses_lag` scans the token program. Lag streams cache `usesLag`, but direct `solve_score_eval_program_with_sources` calls the scanning helper per evaluation before metric work.

Store a prepared `usesLag` bit in the immutable program plan, with a safe fallback for hand-built test structs. This matters only when the requested metrics are cheap; it is lost in expensive feature extraction otherwise.

## F12 - Benchmark Coverage Does Not Match the Remaining Hot Paths

**Severity:** LOW as a direct speed issue, HIGH as an optimization-safety issue

`scripts/bench_program_vms.py` and `tests/native/vm_microbench.c` are useful, but the durable suite currently misses the seams that produced the strongest CR33 findings.

Add cases for:

- fused handler `"-"` versus actual output-path Param invocation;
- static and dynamic Param legacy forms;
- prepared `rect` and future prepared transforms;
- direct-selector Coeff operations at degrees 35, 128, and 256;
- repeated cheap Coeff operations;
- the source-authoritative harvested `compute_mqlacwaq` shape;
- non-pair Solve metric families;
- median selection across degrees and adversarial input orders;
- complete fused-stage wall timing, including process startup and uploads.

Timing tests should be advisory benchmark jobs, not brittle predeploy pass/fail thresholds. Correctness parity belongs in predeploy; speed regression thresholds need controlled Graviton runners.

## Changes That Did Not Help

These probes are important because they prevent the implementation from drifting toward plausible but unproductive work.

| Candidate | Result | Decision |
|---|---|---|
| Param thread-local last-argument cache | Static `rect` about `13.7%` slower; dynamic about `11.9%` slower | Reject. Prepare static values once at load time. |
| Param precomputed coordinate lookup | Deltas roughly `-0.4%` to `+0.7%` | Reject as standalone. Arithmetic is already cheap. |
| Param `sanitize_after` flag | Flat to slower; arithmetic about `4.3%` slower | Reject as standalone. Predictable finite checks are cheap and semantics are delicate. |
| Coeff reserve-write slots for `fill`/`blend` | Mostly within noise, with some high-degree regressions | Do not prioritize producer-slot rewrite. |
| Skip Coeff source-param preparation when no Coeff program | `0%` to about `0.4%` | Do not land standalone complexity. |
| Solve feature heap-to-stack only | About `0-2%` | Allocation alone is not the bottleneck. Build the family plan/workspace together. |
| Generic runtime expression memoization | Unsafe for mutable poly/tos/scan dependencies; no representative evidence | Reject broad memoization. |
| Computed goto for Solve VM | VM costs `3-56 ns`; metrics cost `47-1095+ ns` | Wrong first target. |
| More default threads | Old stream scheduler collapses; oversubscription and Lambda CPU allocation vary | Fix path selection and measure first. |
| `-ffast-math` or `-Ofast` | Changes finite, NaN, signed-zero, branch, and transcendental behavior | Prohibited without a product-level numerical-contract change. |
| Full slot ownership/COW rewrite | Large semantic and aliasing surface before direct fast paths are exhausted | Defer. |

## Prioritized Implementation Sequence

The order below deliberately puts small production wins and safety infrastructure before large evaluator rewrites.

### Phase 0 - Make the remaining paths measurable

1. Add the durable benchmark cases listed in F12.
2. Record host, compiler, git SHA, degree, row count, token histogram, thread count, invocation mode, and output hash.
3. Capture deployed Graviton baselines for Param generation, Coeff generation, metric extraction, and full fused chunk wall time.
4. Do not gate predeploy on wall-clock thresholds.

### Phase 1 - Fix fused Param path selection

1. Pass `output_path` to `SWEEP_COEFFGEN`.
2. Read metadata from stdout and errors from stderr.
3. Add handler invocation-contract tests.
4. Run byte parity for worker counts and range boundaries.
5. Deploy and measure before changing `fused_threads` defaults.

Expected scope: every multithreaded fused Param chunk.

### Phase 2 - Land the two small native wins

1. Gate centroid computation by metric need (F4).
2. Add Param static argument classification and preparation scaffolding (first half of F2).
3. Keep transform-specific preparation disabled until parity tests cover the generic scaffolding.

Expected scope: affected simple Solve metrics and all static Param legacy calls.

### Phase 3 - Prepare `rect`, then measured Param transforms

1. Add the exact prepared `rect` plan.
2. Rerun static/dynamic parity and Graviton A/B.
3. Profile `rply`, `star`, and `rrect` separately.
4. Add only transforms whose end-to-end result remains meaningful.

Expected scope: static legacy transform programs, with no source or wire changes.

### Phase 4 - Add shared Coeff kernels and direct in-place paths

1. Extract exact unary/binary kernels from current typed semantics.
2. Move existing typed operations onto the shared kernels without changing output.
3. Enable direct `poly -> poly` unary and binary paths.
4. Add narrowly proven native-transform cases.
5. Run all native, nonfinite, source-equivalence, M3, wire-fingerprint, and whole-sweep gates.

Expected scope: cheap direct Coeff operations, especially high degree and repeated operations.

### Phase 5 - Build the Solve metric plan and family passes

1. Prepare duplicate owners, engaged sources, lag use, and family masks once.
2. Introduce a worker-owned reusable workspace.
3. Land extrema and radial families first; they have simple exact-order sharing and large measured wins.
4. Add angular histogram and centroid/covariance families.
5. Add angular-order sharing without recurrence shortcuts.
6. Refine pair-family `needProximity`.

Expected scope: score programs requesting multiple related metrics.

### Phase 6 - Replace median full sorts

1. Implement deterministic bounded introselect with fallback.
2. Prove exact parity against the current sorted median.
3. Apply to median-only call sites.
4. Keep full sort where full order is required.

Expected scope: median, axis, outlierness, and clusteriness metrics.

### Phase 7 - Add Coeff straight-line scalar superinstructions

1. Instrument token shapes in deployed jobs.
2. Define typed-scalar primitive helpers shared by the normal and prepared evaluators.
3. Recognize only straight-line scalar subgraphs ending in poke.
4. Keep automatic fallback for unsupported/stateful operations.
5. Pin the harvested `compute_mqlacwaq` source and complete output as a regression fixture.

Expected scope: large source-authored scalar-assignment programs.

### Phase 8 - Reconsider architectural changes

Only after the measured local work lands:

- result-view Coeff API;
- combined native Param+Coeff mode;
- upload/compute overlap;
- block/lane Param VM;
- architecture-specific compiler tuning or PGO.

These require production end-to-end measurements and broader failure-path design.

## Required Correctness Gates

Native speed changes must preserve more than a final scalar on a friendly finite input. At minimum run:

- `tests/test_param_program_native.py`
- `tests/test_coeff_program_native.py`
- `tests/test_solve_score_native_parity.py`
- `tests/test_program_m3_oracles.py`
- `tests/test_coeff_wire_fingerprints.py`
- `tests/test_whole_sweep_oracle.py`
- fused handler tests
- source/chain equivalence tests
- thread/range parity tests
- ThreadSanitizer coverage for worker-owned workspaces and schedulers

Add candidate-specific corpora for:

- NaN, infinity, overflow, underflow, signed zero, and division by zero;
- static and dynamic Param arguments;
- every selector and target mode;
- Coeff source/destination alias combinations;
- degree boundaries and maximum stack depth;
- duplicate and lagged Solve metrics;
- all-finite, partially nonfinite, and all-nonfinite roots;
- odd/even medians and adversarial selection input;
- deterministic random/dither sequences across worker counts.

The optimization contract is:

1. Source, chain, execution spec, and fingerprints remain unchanged unless a separate migration explicitly says otherwise.
2. Finite complete outputs remain byte-identical for internal execution-plan changes.
3. Existing nonfinite/error behavior remains identical, not merely "reasonable."
4. Worker count and scheduler selection do not silently change random output contracts.

## Production Telemetry Needed

Local profiles identify mechanisms; production telemetry determines priority. Add low-volume structured timing fields for:

- Param scheduler mode: serial, ordered stream, or static file;
- rows, passes, worker count, and online CPU count;
- Param token count and static/dynamic legacy token counts;
- Coeff degree, token histogram, direct versus typed operation counts, and bytes copied if cheap to count;
- Solve metric count, source count, duplicate count, and feature-family mask;
- stage CPU/wall time, upload time, and local file sizes;
- process startup time versus native `elapsed_us`;
- Lambda memory size and architecture.

Do not emit per-row logs. Aggregate once per stage or task. The point is to answer whether a candidate affects common production programs, not to create another hot path.

## Build-Level Opportunities

Production currently uses a generic AArch64 `-O3` build. NEON is part of the AArch64 baseline, and clang already vectorizes several simple Coeff loops. Build tuning is therefore a secondary lever.

Potential later experiments:

- target the exact deployed Graviton generation with an appropriate `-mcpu` only after confirming runtime compatibility;
- profile-guided optimization using a representative corpus;
- inspect generated assembly for the new operation-selected kernels;
- consider function-level layout rather than global fast-math.

Do not use `-ffast-math`. Do not accept a build only because a microbenchmark improves; compare complete output bytes and the whole-sweep oracle on the deployment architecture.

The single large `sweep_cli.c` translation unit already gives the optimizer broad visibility, so conventional LTO is unlikely to be the first meaningful win.

## Expected Impact by Workload

| Workload shape | Most relevant changes |
|---|---|
| Any multithreaded fused Param chunk | F1 immediately; F2 for static legacy programs |
| Static geometry-heavy Param transforms | F2 prepared transform constants |
| Identity/small deterministic Param stack programs | F9 exact internal peepholes |
| High-degree Coeff with cheap unary/binary operations | F3 direct in-place kernels |
| Large scalar-assignment Coeff source | F7 prepared straight-line scalar plan |
| One cheap Solve metric | F4, F11; keep direct path |
| Several related Solve metrics | F5 family plan, then F6 for median families |
| Pair/nearest-neighbor Solve bundle | F5 `needProximity` refinement; existing pair cache remains useful |
| Full fused job dominated by I/O | F1 first, then F10 only after stage telemetry |

## Final Recommendation

The next performance work should not be advertised as a general VM rewrite. It should be a sequence of measured, internal execution-plan improvements:

1. **Fix the fused Param output-path argument.** This is a deployed orchestration bug that bypasses already-written fast code.
2. **Prepare static Param legacy operations and land the measured `rect` specialization.** Avoid runtime memoization.
3. **Add exact shared Coeff kernels and direct owned-vector fast paths.** This is the clearest remaining Coeff memory-traffic win.
4. **Move Solve optimization above the VM:** conditional centroid, prepared family masks, fused feature passes, and bounded median selection.
5. **Use hidden superinstructions only for proven real shapes**, starting with the harvested Coeff scalar program and exact Param patterns.
6. **Defer architectural fusion and build tuning** until production telemetry shows the local evaluator work no longer dominates.

The central lesson from the measurements is consistent: dispatch is visible, but repeated setup, repeated scans, full-vector copies, sorting, and the wrong scheduler are larger. Optimize those first, while keeping source, wire, fingerprints, random sequencing, and numerical behavior fixed.
