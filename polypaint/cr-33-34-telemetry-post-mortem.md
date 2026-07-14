# CR33/CR34 Telemetry Implementation Post-Mortem

Date: 2026-07-13

Review target: current tree at `cfa501a`, with the telemetry implementation reviewed against the pre-telemetry baseline `f5b47af`.

## Verdict

The implementation establishes useful telemetry plumbing, but the generated reports are not yet trustworthy enough to drive production optimization decisions.

The strongest parts are the causal reconstruction of Step Functions task results, preservation of render task rows for later inspection, per-chunk compute timings, and native raster download worker/wall separation. The principal problem is report integrity: build identity is not deployed to the measured Lambda fleet, workload identity is discarded, CloudWatch joins fail open, metrics from different Lambda roles are mixed, and several timing names imply exclusivity that the measurements do not have.

This is not a claim that the product pipeline is broken. It is a claim that a successful telemetry run can currently produce a plausible-looking but materially misleading report. The collector must fail closed on incomplete attribution before CR33/CR34 speedup decisions are based on it.

## Findings

### F1 - HIGH - Build identity is not deployed to the measured Lambda fleet

`lambda/shared.py:35-46` reads `PP_GIT_SHA` and `PP_BUILD_ID`, and the compute, raster, and finalizer handlers copy `build_identity()` into task results. That producer-side code is sound only if the environment variables exist.

They do not exist on the relevant functions. `deploy.sh:451-454` sets the variables only in the book/PDF deployment path. The generated Lambda environment specifications for compute-fused, raster, and finalizer omit both variables, and `deploy_manifest.py --emit-bash` emits no `PP_GIT_SHA` or `PP_BUILD_ID` assignment for those functions.

Consequences:

- Production task rows normally contain no build identity.
- A report cannot prove which code produced the measurements.
- Reusing `$LATEST` allows a deployment during an execution to create a mixed-build sample.
- The acceptance criterion in `cr-33-telemetry.md` is not met despite the handler fields existing.

Required fix:

- Stamp `PP_GIT_SHA` and `PP_BUILD_ID` centrally for every deployed Lambda, preferably in the common deployment path rather than per-function declarations.
- Use the already computed deployment revision/build identifier, or a package content hash that changes for dirty builds.
- Add a deployment-contract test against generated function environments, not merely a source grep for `build_identity()`.
- Prefer immutable Lambda versions and aliases for benchmark runs. Environment labels alone cannot prevent a mid-run `$LATEST` replacement.

### F2 - HIGH - The collector drops the actual workload identity

Production `ExecutionStarted.input` stores the workload under `params`. The retained histories reviewed here have top-level keys such as `job_id`, `mode`, `params`, `run_id`, and `task_id`.

`scripts/capture_production_telemetry.py:529-539` looks for workload fields only at the top level. The resulting reports identify a job/run and sometimes the first task, but omit the values needed to compare like with like:

- Compute: `N`, degree, times, chunk count, coefficient count, function, solver, and program fingerprints.
- Render: pixel dimensions, quality, format, palette, metric, section count, worker count, and collision/non-finite policy.

This was reproduced against the retained compute and render histories. The reports completed successfully while lacking the workload signature.

Required fix:

- Normalize identity from the outer execution input, nested `params`, plan output, and task input.
- Define a required identity schema per workflow kind.
- Include all program fingerprints and the render controls that affect cost or output.
- Mark a report invalid when required fields are unavailable instead of silently emitting a sparse identity object.

### F3 - HIGH - CloudWatch attribution is fail-open and can undercount cost

`join_cloudwatch()` in `scripts/capture_production_telemetry.py:398-449` scans Lambda log groups and catches all exceptions with `continue`. No error list is retained. The report builder accepts any subset of joined request IDs, and a live run with no joins can be labeled as having no CloudWatch join as if it were an intentional offline run.

There is no validation that:

- Every successful task request ID joined exactly once.
- The joined function matches the task function.
- A log query failed, timed out, or was throttled.
- Function configuration was captured for every measured function.
- Failed/retried Lambda attempts contributed billed duration.

The final point matters for cost: task result extraction currently starts from successful task bodies. A failed Lambda attempt that Step Functions retries can incur billed GB-seconds without appearing in the joined successful-request set.

Required fix:

- Derive the exact log-group set from task function names instead of scanning every `polypaint*` group.
- Preserve and report CloudWatch API/query errors.
- Validate expected request IDs against joined request IDs and reject missing, duplicate, or wrong-function joins.
- Distinguish `source=offline` from a failed live join.
- Report failed-attempt billing as incomplete until Step Functions failure/retry request IDs are also reconstructed.
- Add mocked live-join tests, including pagination, throttling, missing groups, duplicate IDs, and partial results.

### F4 - HIGH - Metrics from different functions and roles are aggregated together

`classify_and_aggregate()` processes all successful task bodies into one metric namespace. `tasks_by_function` is only a count; it is not used to scope metric distributions. CloudWatch handler durations and billed durations are likewise summarized across functions.

The problem is visible in a retained compute history:

- The execution contains 63 successful task results but 49 fused compute workers.
- `coeffgen_us.n` and `compute_us.n` are 50, not 49, because another task role exposes fields with the same names.
- `elapsed_us.n` combines unrelated task types.
- A workflow-wide handler percentile combines status, planning, worker, and finalize Lambdas.

Even when contamination is numerically small, the report no longer answers the optimization question. A p95 across heterogeneous functions is not a fused-worker p95, and summing same-named fields from different roles is unsafe.

Required fix:

- Attach Step Functions state, logical role, function name, request ID, and attempt to every task result.
- Aggregate primarily by workflow kind plus role/function.
- Emit workflow rollups only for metrics explicitly declared additive across those roles.
- Make the CR33 compute baseline filter specifically to the fused compute workers.
- Keep raster, finalizer, planner, status, and low-resolution/probe distributions separate.

### F5 - HIGH - Raster wall-clock labels overlap and overstate what they measure

In `lambda/handler_raster_mt.py:624-644`, `native_wall_us` times the complete native subprocess call. That interval includes process startup, S3 download, native compute, fragment serialization/file writes, and process output handling. The C process separately reports `download_wall_us`.

Therefore `native_wall_us` is not a native-compute-only wall time and overlaps `download_wall_us`. A consumer can easily double-count the download or attribute it to the VM/render kernel.

`handler_wall_us` has a second naming problem. Its timer starts at `lambda/handler_raster_mt.py:534`, after some request parsing and setup, and stops around line 704 before final status writes, response construction, and cleanup. It is a pipeline-region timer, not complete Lambda handler wall time.

Required fix:

- Rename the Python subprocess measurement to `subprocess_wall_us`.
- Have `roots2pix_mt` expose the earliest native-worker start and latest native-worker end as a true `native_wall_us`.
- Keep `native_worker_us` as worker-time sum and document that it is not wall time.
- Rename the current handler timer to `pipeline_wall_us`, or start a complete timer at handler entry and stop it in a `finally` block.
- Continue using CloudWatch REPORT duration as the authoritative billed/handler duration.

### F6 - HIGH - Mixed-build executions are silently attributed to the first build

`classify_and_aggregate()` uses `setdefault` for identity fields. If two task results carry different `build_id`, `git_sha`, or architecture values, the first value wins and no warning is emitted.

An adversarial two-task probe with build IDs `A` and `B` produced a report identity of `A` with no conflict. Once F1 is repaired, this becomes a direct benchmark-integrity bug rather than a dormant one.

Required fix:

- Collect the distinct values for every identity field by function and role.
- Require exactly one build ID, revision, architecture, and memory setting for each comparison group.
- Fail report validation on disagreement.
- Record Lambda version/alias and function configuration alongside the application build identity.

### F7 - MEDIUM - Finalizer telemetry is still partial and contains a fake zero

`lambda/handler_finalize_mt.py` adds useful measurements, but several values do not match the phase names required by the telemetry plan:

- `assemble_ms` includes presigning and assembly rather than reporting them separately.
- `step_scores_fetch_concat_ms` combines download and concatenation.
- `render_ms` and `render_encode_ms` are aliases of the same interval.
- `upload_ms` is a parent total that overlaps raw, associated-palette, image, preview, and metadata uploads.
- There is no complete handler wall timer.
- The associated-palette result still emits `encode_ms: 0` around lines 558 and 587.

The zero is especially dangerous because it looks measured. Unknown or unavailable timing must be omitted/null, not represented as zero.

Required fix:

- Remove the associated-palette fake zero.
- Split presign, fragment download, assembly, score download, concatenation, rendering, encoding, and upload where those phases are independently actionable.
- Mark parent totals and aliases explicitly in the metric schema.
- Add the same split for the associated-palette path, or label its total as intentionally opaque.

### F8 - MEDIUM - The percentile implementation can report p95 below p50

The collector selects p95 using `int(0.95 * (n - 1))`. For two values `[1, 2]`, this yields `p50=1.5` and `p95=1`. A retained report contains the same impossible ordering for a two-sample field: p95 is below p50.

This is not just presentation. Small task groups are common for finalizers, planners, and probes, so the broken edge case appears in normal production reports.

Required fix:

- Use a documented quantile rule, such as nearest rank `ceil(p*n)-1`, or a standard interpolation.
- Special-case singleton samples.
- Add tests for sample sizes 1, 2, 3, and a representative worker count.
- Assert the invariant `min <= p50 <= p95 <= max`.

### F9 - MEDIUM - Solve-plan telemetry is emitted but cannot reach the report

`lambda/solve_score.h:1111+` prints `pp_solve_plan` only when `getenv("PP_PLAN_TELEMETRY")` is non-null. The deployment does not set that variable, and the collector parses CloudWatch REPORT records rather than these application log records.

As a result, the implementation exists in native code but is not production telemetry. In addition, setting `PP_PLAN_TELEMETRY=0` still enables it because presence, not a true value, is tested.

Required fix:

- Prefer returning the selected plan summary in the task result so it remains causally attached to the worker.
- If logs are retained as the transport, deploy an explicit boolean flag, parse the plan records, and join them by request ID.
- Treat only a defined true value as enabled.
- Test the full producer-to-report path, not only native stderr behavior.

### F10 - MEDIUM - The metric schema does not encode aggregation or overlap semantics

The collector classifies fields mainly by suffix. That is insufficient for the current payloads:

- Gauges/invariants such as `lambda_memory_mb`, online CPU count, token counts, and average iteration counts are reported with meaningless sums.
- Aliases such as `compute_us`/`solve_us`, `roots_size`/`bin_size`, and `render_ms`/`render_encode_ms` appear independent.
- Parent totals such as `upload_ms` overlap their child timers.
- Worker-time sums and wall-clock spans can appear beside each other without a scope distinction.

Required fix:

- Define each metric with `unit`, `aggregation`, `scope`, and overlap metadata.
- Support at least `distribution`, `additive`, and `invariant` aggregation modes.
- Represent aliases with `alias_of`, and parent/child overlap explicitly.
- Reject invariant disagreement instead of averaging or summing it.
- Do not offer a cross-metric total unless the schema proves the fields exclusive and additive.

### F11 - MEDIUM - Reconstruction validation remains fail-open

The causal reconstruction is a substantial improvement, but its completeness checks are incomplete:

- Missing request IDs, function names, or task bodies do not necessarily fail the run.
- Expected chunk validation runs only when the reconstructed chunk set is non-empty; complete extraction failure can bypass the comparison.
- There is no equivalent expected-section validation for raster work.
- Lambda integration `statusCode` values are not comprehensively validated.
- CloudWatch and function-configuration coverage are outside validation.
- `state_walls` is accepted by validation but is not used to prove state coverage.

Required fix:

- Define required fields and expected map cardinality by workflow kind.
- Compare planned chunks/sections with reconstructed task identities even when zero tasks were reconstructed.
- Validate status codes, request IDs, function names, task bodies, state coverage, CloudWatch coverage, and configuration snapshots.
- Emit a machine-readable validation section and exit non-zero for an invalid live report.

### F12 - MEDIUM - Producer contracts and live collection are not adequately gated

The new collector tests cover causal reconstruction and replay of retained histories, which is valuable. They do not cover several load-bearing production contracts:

- Raster handler tests do not assert the new timing fields or their propagation into status rows.
- Finalizer tests do not catch the associated-palette `encode_ms: 0` or timing aliases.
- No generated-deployment test proves build identity reaches the measured functions.
- No live CloudWatch join test proves exact request coverage or error handling.
- No test rejects mixed builds, workload-identity loss, cross-function metric contamination, or invalid percentile ordering.
- The retained histories predate some of the new producer fields, so successful replay does not prove current producer/collector compatibility.

Required fix:

- Add response and DDB-row contract tests for compute, raster, and finalizer producers.
- Add a generated-deployment environment test.
- Add mocked end-to-end live collector tests.
- Add a current-format synthetic history containing all required fields and multiple roles/functions.
- Gate these tests in `scripts/predeploy_check.sh`.

### F13 - MEDIUM - Required CR33/CR34 producer fields are still missing

The implementation does not yet capture all fields needed to evaluate the optimization proposals:

- Compute exposes Param and Coeff outer/native timings, but Solve has only the outer `solve_us`; the native solve elapsed value is not reported separately.
- Compute does not expose a complete handler wall.
- Raster omits several input/behavior controls needed for CR34 comparisons, including input bytes, collision/non-finite outcomes, section concurrency, and effective output policy.
- Native raster tracks values such as deduplicated roots for stderr diagnostics but does not include all of them in the machine-readable result.
- Finalizer lacks the fully separated phase set described in `cr-33-telemetry.md`.

Required fix:

- Turn the CR33/CR34 measurement table into an executable producer schema.
- Require every run to state which fields are available, unavailable, or not applicable.
- Preserve native and wrapper timings as separately named fields.
- Carry all output-affecting policies into workload identity so non-deterministic/quality tradeoffs can be compared honestly.

## Correctly Implemented Parts

Several changes are sound and should be retained:

- Step Functions task results are reconstructed causally rather than by guessing from timestamps.
- State enter/exit pairing and task-attempt extraction work on the retained real histories.
- Compute stage telemetry is emitted per chunk rather than per row, keeping native stdout bounded.
- Native raster download telemetry correctly distinguishes worker-time sum from wall-clock span.
- `CleanRender` no longer deletes task telemetry immediately. Render task IDs are run-scoped, status checks use the run/task prefix, and the existing TTL bounds retention.
- Native output byte-neutrality and parity checks remain green.
- Unknown timing suffixes fail collector classification rather than silently becoming timings.
- Raw Step Functions histories remain available for reprocessing instead of being replaced by only the derived report.

## Verification Performed

The following targeted suites passed on the reviewed tree:

```text
175 passed
tests/test_production_telemetry_collector.py
tests/test_compute_chunk_fused.py
tests/test_pipeline.py
tests/test_coeffgen_param_gen.py
tests/test_solve_score_native_parity.py
tests/test_deploy_packaging.py

18 passed
tests/test_finalize_mt_handler.py
tests/test_raster_mt.py

19 passed, 4 subtests passed
tests/test_raster_mt_parity.py
tests/test_exact_viewport_parity.py
```

The collector was also run offline against the retained compute and render execution histories. Those runs established the workload-identity loss, cross-function aggregation, and small-sample percentile defects described above.

The green tests do not contradict the findings. Most findings are missing-contract problems: the current tests accept partial attribution and overlapping semantics rather than asserting that a production report is complete and comparable.

The full predeploy gate was not rerun as part of this review.

## Remediation Order

1. Deploy build identity to the entire measured fleet and reject mixed identities.
2. Preserve complete workload identity and aggregate by logical role/function.
3. Make CloudWatch joining and reconstruction completeness fail closed.
4. Correct raster timing names/boundaries and remove finalizer fake/alias timings.
5. Replace percentile calculation and introduce an explicit metric schema.
6. Connect solve-plan telemetry to the report and complete the CR33/CR34 producer field set.
7. Add producer, deployment, live-join, and report-integrity gates to predeploy.

Until items 1-4 are complete, use current reports for exploratory debugging only. Do not use their aggregate percentiles, cost totals, or build-to-build comparisons as production evidence.
