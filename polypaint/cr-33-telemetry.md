# CR33 Telemetry: Production Baseline and Collection Plan

Date: 2026-07-13

## 1. Purpose

This document records the production telemetry review performed after the CR33 VM work and the CR34 colorization review. It answers two questions:

1. What is actually slow in the currently deployed compute and render pipelines?
2. What telemetry must be retained and collected before further optimization decisions are made?

The current telemetry is sufficient to identify the largest bottlenecks, but it is not yet a durable or fully truthful benchmark system. Several counters mix aggregate worker effort with elapsed wall time, useful DynamoDB rows are deleted by the next render, and deployed results do not carry a durable build identity.

The immediate conclusion is clear:

- Param generation is no longer the production bottleneck in the sampled compute run.
- Compute time is now dominated by solve work, coefficient generation, and S3 data movement.
- Render time is dominated by source retrieval/data movement, followed by final assembly and upload work.
- More Param VM micro-optimization is not currently justified without contrary telemetry from the newly added stage counters.

## 2. Measurement Rules

The timing terms in this document are deliberately distinct.

| Term | Meaning |
|---|---|
| Workflow wall time | End-to-end Step Functions elapsed time. This is user-visible latency. |
| State wall time | Elapsed time between a Step Functions state entering and exiting. |
| Lambda handler duration | CloudWatch `REPORT Duration`; elapsed time for one Lambda invocation. |
| Task-stage sum | Sum of a named stage over every parallel task. This measures aggregate work, not user-visible latency. |
| Worker-thread sum | Sum of timers recorded independently by overlapping native threads. This can exceed Lambda wall time and must not be labeled wall time. |
| Task maximum | Slowest invocation or stage. In a fully parallel wave this is often more relevant to wall time than the sum. |
| Billed duration | CloudWatch billed duration, including initialization where applicable. Used for cost comparisons. |

Parallel task-stage sums must never be described as CPU time or wall time. They are useful for determining how aggregate work is distributed, but they cannot be added to predict workflow latency without accounting for concurrency and overlap.

## 3. Evidence and Limitations

The baseline was reconstructed from three sources:

- Step Functions Standard execution history, including each Lambda task result.
- CloudWatch Lambda `REPORT` lines, joined to task results by Lambda request ID.
- DynamoDB task-status telemetry where rows still existed.

The raw Step Functions histories used during this review were downloaded to temporary files:

- `/private/tmp/cr33_compute_history.json`, approximately 2.4 MB.
- `/private/tmp/cr34_render_history.json`, approximately 491 KB.

These are intentionally not proposed as repository artifacts. Histories are large and may contain complete user programs and request payloads.

The measurements are production observations, not a controlled A/B benchmark. They establish bottleneck direction and expose telemetry defects. They do not by themselves prove the speedup of a particular code change.

## 4. Production Measurement Methodology

### 4.1 Collection boundary

The review used read-only AWS API calls against the deployed production account in `us-east-1`. It did not invoke a workflow, mutate an S3 object, update DynamoDB, or alter a Lambda configuration. The measured runs were normal production executions that had already completed.

Collection covered the complete execution histories rather than a sample of map iterations. The compute history contains all 49 map iterations and 63 Lambda tasks. The render history contains all 18 raster iterations plus its planning, status, clipping, and finalization tasks.

### 4.2 Discovering the executions

The relevant executions can be found with the Step Functions `list-executions` API. The state-machine ARNs for the measured workflows are:

```text
arn:aws:states:us-east-1:710848990594:stateMachine:polypaint-compute-workflow
arn:aws:states:us-east-1:710848990594:stateMachine:polypaint-render-workflow
```

Representative discovery commands are:

```bash
aws stepfunctions list-executions \
  --state-machine-arn arn:aws:states:us-east-1:710848990594:stateMachine:polypaint-compute-workflow \
  --status-filter SUCCEEDED \
  --max-results 20 \
  --region us-east-1
```

```bash
aws stepfunctions list-executions \
  --state-machine-arn arn:aws:states:us-east-1:710848990594:stateMachine:polypaint-render-workflow \
  --status-filter SUCCEEDED \
  --max-results 20 \
  --region us-east-1
```

Execution names and inputs were checked rather than assuming that the most recent successful execution represented the desired workload. The selected compute and render ARNs are recorded in Sections 5.1 and 6.1.

### 4.3 Downloading complete Step Functions histories

For each selected ARN, both execution metadata and the full event history were retrieved:

```bash
aws stepfunctions describe-execution \
  --execution-arn "$EXECUTION_ARN" \
  --region us-east-1 \
  > /private/tmp/execution-description.json
```

```bash
aws stepfunctions get-execution-history \
  --execution-arn "$EXECUTION_ARN" \
  --include-execution-data \
  --region us-east-1 \
  > /private/tmp/execution-history.json
```

The AWS CLI paginates `get-execution-history` by default. A collector implemented with boto3 must follow every `nextToken` explicitly. A partial first page is unusable for map-stage totals and percentile calculations.

`--include-execution-data` is required. Without it, task inputs and outputs are omitted, which removes the stage timers, byte counters, chunk identities, and Lambda request IDs needed for this analysis.

The retained files used in this review were:

```text
/private/tmp/cr33_compute_history.json
/private/tmp/cr34_render_history.json
```

Raw histories were kept under `/private/tmp` because they contain full production inputs, including program source, and should not be committed casually.

### 4.4 Reconstructing task records

Each Step Functions history event has an event ID, timestamp, type, and causal `previousEventId`. Task records were reconstructed by following that causal chain rather than pairing events only by state name. Name-only pairing is unsafe when map iterations execute the same state concurrently.

For each successful Lambda task:

1. Follow `previousEventId` from `TaskSucceeded` to `TaskStarted` and then `TaskScheduled`.
2. Decode `taskScheduledEventDetails.parameters` to obtain the exact Lambda `FunctionName` and task input.
3. Decode `taskSucceededEventDetails.output`, which is a JSON string containing the optimized Lambda integration envelope.
4. Decode `Payload.body`, which is usually a second JSON string containing the handler's actual result.
5. Retain the map iteration/chunk identity so task metrics are not associated by array order.

A typical integration envelope has this shape:

```json
{
  "ExecutedVersion": "$LATEST",
  "SdkResponseMetadata": {
    "RequestId": "2122980d-e244-451a-a3aa-27374adf8bf8"
  },
  "Payload": {
    "statusCode": 200,
    "body": "{...handler result...}"
  }
}
```

The compute history identified 49 invocations of `polypaint-compute-fused-chunk`. The render history identified 18 invocations of `polypaint-raster-mt` and one invocation of `polypaint-finalize-mt`.

### 4.5 Deriving workflow and state wall time

Workflow wall time came directly from `startDate` and `stopDate` in `describe-execution` and was cross-checked against the `ExecutionStarted` and `ExecutionSucceeded` history timestamps.

State wall time was computed from the matching state-entered and state-exited timestamps. Map and parallel states were measured at their enclosing state boundary. Individual Lambda wall time was not inferred from those state boundaries; it came from CloudWatch `REPORT` records.

This distinction matters:

- The enclosing map wall includes scheduling, Lambda invocation, retries if present, response handling, and the slowest concurrency wave.
- A Lambda handler duration measures one invocation.
- Summing handler or stage durations across map iterations measures aggregate task work, not map wall.

No failed or retried task events occurred in the selected map samples. A general collector must report retry counts and must not silently count multiple attempts as distinct successful chunks.

### 4.6 Joining tasks to CloudWatch Lambda reports

The Lambda request ID was read from `SdkResponseMetadata.RequestId` in each task result. It is also available in the optimized integration response headers. That request ID was joined exactly to the matching CloudWatch `REPORT RequestId` record in the function's log group.

Relevant production log groups included:

```text
/aws/lambda/polypaint-compute-fused-chunk
/aws/lambda/polypaint-raster-mt
/aws/lambda/polypaint-finalize-mt
```

The log query interval used the workflow start and stop timestamps with a small safety margin. A representative retrieval command is:

```bash
aws logs filter-log-events \
  --log-group-name /aws/lambda/polypaint-compute-fused-chunk \
  --start-time "$START_EPOCH_MS" \
  --end-time "$END_EPOCH_MS" \
  --filter-pattern '"REPORT RequestId:"' \
  --region us-east-1
```

Each `REPORT` line supplied:

- Handler duration.
- Billed duration.
- Configured memory.
- Maximum memory used.
- Initialization duration when the invocation was cold.

Cold starts were classified by the presence of `Init Duration` in that invocation's `REPORT` line. They were not inferred from latency alone.

Joining by exact request ID avoids timestamp heuristics. Timestamp-only matching is unsafe when dozens of invocations overlap in the same log group.

### 4.7 Reading transient DynamoDB telemetry

Where task-status rows still existed, the job partition was queried from the production jobs table and rows were matched by run/task identity. These rows supplied raster worker counters and finalizer counters not returned in the Step Functions payload.

DynamoDB was treated as a supplementary source, not the authoritative archive, because `CleanRender` currently deletes the job's status rows. Missing rows were not interpreted as zero-duration work.

This is why the compute stage totals could be recovered from Step Functions after rendering, while some render timing decomposition depended on rows captured before another cleanup.

### 4.8 Capturing deployment configuration

Function configuration was inspected with the Lambda `get-function-configuration` API to obtain:

- Memory size.
- Timeout.
- Architecture.
- Ephemeral storage.
- Layers.
- `CodeSha256`.
- `LastModified`.

Because the state machines invoked `$LATEST`, current configuration is reliable only when captured immediately after the execution. It is not a substitute for embedding a build ID or invoking immutable Lambda versions.

### 4.9 Aggregation and calculations

Handler result fields were aggregated only after all expected task records had been recovered and joined.

The calculations used were:

```text
task_stage_sum = sum(stage_time for every successful task)
task_stage_mean = task_stage_sum / successful_task_count
task_stage_max = max(stage_time for every successful task)
stage_share = task_stage_sum / sum(all mutually exclusive measured stage sums)
workflow_wall = execution_stop - execution_start
state_wall = state_exit - state_enter
```

Byte totals were summed from handler result fields, not estimated from timing. Cold-start counts came from CloudWatch. Memory maxima came from `Max Memory Used` in each `REPORT` line.

For the render path, `download_us` and `native_us` were not treated as wall time because source inspection showed that they sum overlapping worker-thread durations. Their impossible relationship to Lambda wall time provided an independent sanity check: a reported 63-71 seconds of download work cannot be elapsed time inside a 19-24 second invocation.

### 4.10 Validation and evidence labels

The reconstruction was checked with the following invariants:

- The compute map contained 49 successful iterations for 49 requested chunks.
- The raster map contained 18 successful iterations for 18 requested sections.
- Every measured optimized Lambda task had a decodable handler result.
- Every task included in Lambda duration statistics joined to exactly one CloudWatch request ID.
- Map wall time contained the observed invocation waves.
- Stage byte totals agreed with the object-family fields returned by the handlers.
- Aggregate worker counters were never presented as elapsed workflow time.

Claims in this document fall into three evidence classes:

| Class | Examples |
|---|---|
| Direct measurement | Step Functions timestamps, Lambda `REPORT` duration, handler stage counters, byte counters. |
| Derived measurement | Sums, means, maxima, shares, residual time, and cold-start counts calculated from direct measurements. |
| Inference | Raster download dominating wall time and the likely contents of the finalizer's unmeasured residual. |

Inferences are stated as such. In particular, the current telemetry does not directly report raster download wall time, so the estimated 15-20 second range is not presented as a measured counter.

## 5. Production Compute Baseline

### 5.1 Execution identity

| Field | Value |
|---|---|
| Execution | `arn:aws:states:us-east-1:710848990594:execution:polypaint-compute-workflow:compute_aberth_mt_run_1783921106557_dhafq8` |
| Job | `compute_mriskfb1` |
| Run | `run_1783921106557_dhafq8` |
| Region | `us-east-1` |
| `N` | 10,000 |
| Chunks | 49 |
| Times | 1 |
| Function | `const` |
| Solver | `aberth_mt` |
| Degree | 34 |
| Coefficients | 35 |
| Fused threads | 4 |
| Reused params | 0 |
| Reused coeffs | 0 |
| Solve steps | 100,000,000 |

The run used `legacy(unit_circle,both,both)` as its Param program and a non-trivial Coeff program. The complete source is present in the raw history but is not duplicated here.

### 5.2 End-to-end timing

| Measurement | Time |
|---|---:|
| Workflow wall | 36.001 s |
| `FusedChunkMap` wall | 29.942 s |
| Fastest Lambda handler | 22.190 s |
| Slowest Lambda handler | 28.780 s |

All 49 map invocations were cold starts. The map had a concurrency ceiling of 50 and launched 49 distinct environments, so there was no warm second wave.

Each invocation used 10,240 MB configured memory. Peak observed memory was only 2,330-2,428 MB, but the allocation also controls available vCPU and network capacity. The low memory-use ratio is not evidence that the memory tier can safely be reduced.

### 5.3 Aggregate stage work

The following values are sums over all 49 parallel tasks.

| Stage | Aggregate task-stage sum | Mean per task | Maximum per task | Share of measured stage work |
|---|---:|---:|---:|---:|
| Param generation | 3.673 s | 0.075 s | 0.086 s | 0.29% |
| Coeff generation | 229.107 s | 4.676 s | 5.578 s | 18.00% |
| Solve | 520.200 s | 10.616 s | 12.243 s | 40.85% |
| Uploads | 520.356 s | 10.620 s | 10.837 s | 40.86% |
| Total | 1,273.336 s | 25.986 s | n/a | 100.00% |

Again, 1,273 seconds is not the workflow duration. It is the sum of work performed by 49 concurrent tasks.

The slowest measured task was chunk 33:

| Stage | Time |
|---|---:|
| Param generation | 0.083 s |
| Coeff generation | 5.329 s |
| Solve | 12.226 s |
| Uploads | 10.837 s |
| Measured total | 28.475 s |

### 5.4 Upload decomposition

| Object family | Aggregate upload time | Mean per task |
|---|---:|---:|
| Params | 18.646 s | 0.381 s |
| Coefficients | 224.253 s | 4.577 s |
| Roots | 277.457 s | 5.662 s |
| Total | 520.356 s | 10.620 s |

The run produced approximately:

| Object family | Bytes |
|---|---:|
| Params | 1.6 GB |
| Coefficients | 28.0 GB |
| Roots | 27.2 GB |
| Total | 56.8 GB |

These byte totals explain why upload work is comparable to solve work. This is not a small-object bookkeeping problem; it is a large data-movement path.

### 5.5 Compute conclusions

1. The CR33 Param work has achieved its intended production effect in this sample. Param generation is only 0.29% of measured aggregate stage work.
2. Solve is the largest compute stage, but uploads are effectively tied with it.
3. Coeff generation remains material at 18% and is worth profiling, especially for complex programs.
4. Further Param VM tuning should be deferred unless the deployed `stage_telemetry` counters contradict this result on representative workloads.
5. Overlapping independent uploads with subsequent computation is now a justified experiment. Param upload may overlap Coeff generation, and coefficient upload may overlap solve where data dependencies permit. Root upload remains tail work.
6. Overlap can increase S3 or network contention and must be benchmarked rather than assumed beneficial.
7. Memory-tier changes require controlled production tests because Lambda memory determines CPU and network allocation as well as capacity.

## 6. Production Render Baseline

### 6.1 Execution identity

| Field | Value |
|---|---|
| Execution | `arn:aws:states:us-east-1:710848990594:execution:polypaint-render-workflow:render_color_run_1783926589128_xkrbx4` |
| Job | `compute_mriskfb1` |
| Run | `run_1783926589128_xkrbx4` |
| Resolution | 5,000 px |
| Format | JPEG |
| JPEG quality | 90 |
| Solve score | `centroid_re`, q=0.1% |
| Palette | Long palette, Washington family |
| Raster threads | 4 |
| Raster workers | 10 |
| Raster sections | 18 |
| Finalize workers | 16 |
| Associated palette | Enabled |

### 6.2 Workflow timing

| State | Wall time |
|---|---:|
| Complete render workflow | 69.858 s |
| `CleanRender` | 1.472 s |
| `BuildPlan` | 3.945 s |
| Clip task | 1.615 s |
| `ColorRasterMap` | 45.080 s |
| Final phase status call | 1.663 s |
| `ColorAssembleEncodeTask` | 15.605 s |
| Final done-status write | 0.141 s |

The 18 raster sections ran with concurrency 10, producing two waves. The first 10 invocations were cold; the remaining 8 reused environments.

Raster Lambda handler durations ranged from 19.149 to 23.748 seconds. Each raster invocation used 10,240 MB configured memory and peaked at only 1,227-1,523 MB. As with compute, this does not justify reducing memory without measuring the accompanying loss of vCPU/network capacity.

### 6.3 Raster telemetry interpretation

The current raster counters are easy to misread:

- `native_us` is the sum of four overlapping worker-thread native durations, not native wall time.
- `download_us` is also summed across worker threads. Per-invocation values of roughly 63-71 seconds cannot be Lambda wall time because the complete handlers ran in roughly 19-24 seconds.
- `upload_us` is closer to an elapsed stage measurement, at roughly 0.7-1.05 seconds per invocation, but its exact scope still needs to be documented and normalized.

Across all 18 sections, reported `native_us` totaled 46.002 seconds, with a mean of 2.556 seconds and a range of 2.011-3.044 seconds per section. This aggregate worker metric is small relative to the raster Lambda wall durations.

The Lambda wall times and thread-summed download counters together show that source retrieval is the dominant raster cost. The precise download wall time is not currently recorded, but it is plausibly on the order of 15-20 seconds per invocation, not 63-71 seconds.

Raster output traffic was approximately:

| Output | Bytes |
|---|---:|
| Main sparse fragments | 212.4 MB |
| Associated-palette fragments | 500.0 MB |
| Step-score fragments | 100.0 MB |
| Total | 812.4 MB |

The 500 MB associated-palette path is the largest avoidable output family. CR34's proposal to derive the associated palette from retained step-score data should be evaluated before attempting small arithmetic optimizations in the native color loop.

### 6.4 Finalizer timing

| Measurement | Time |
|---|---:|
| Step Functions task wall | 15.605 s |
| Lambda handler duration | 14.452 s |
| Lambda billed duration | 14.967 s |
| Cold initialization | 0.515 s |
| Peak memory | 826 MB of 10,240 MB |

Current finalizer telemetry reports:

| Counter | Time |
|---|---:|
| `assemble_ms` | 1.842 s |
| `render_ms` | 0.503 s |
| `encode_ms` | 0.000 s |
| `upload_ms` | 8.025 s |
| Reported component total | 10.370 s |
| Unexplained handler time | approximately 4.082 s |

`encode_ms=0` is not a measurement. The code currently hardcodes it.

`upload_ms` is also too broad. It spans raw upload, associated-palette assembly/render/upload, and main image/preview uploads. It therefore cannot identify which output path is expensive.

The approximately 4.1-second unmeasured bucket principally contains serial step-score download/concatenation/upload work and handler overhead. It must be instrumented before deciding how to parallelize the finalizer.

### 6.5 Render conclusions

1. Raster source retrieval and data movement dominate the sampled render, not native Solve Score evaluation or palette lookup.
2. The current worker-thread timers are useful counters but are mislabeled for latency analysis. Wall and worker-sum measurements must both be retained with explicit names.
3. Associated-palette sparse fragments add approximately 500 MB to the sampled raster output and are the clearest architectural removal target.
4. The finalizer contains about 4.1 seconds of unmeasured work and an 8-second counter that combines unrelated operations.
5. Section count and concurrency should be swept together. Eighteen sections at concurrency 10 necessarily creates a second wave; fewer larger sections may reduce orchestration and repeated-download overhead, while too few sections may reduce parallelism.
6. JPEG previews are appropriate where PNG losslessness is not required, but changing output encoding will not remove the dominant source-download cost.
7. CR34's relaxed nondeterminism constraint permits faster collision handling, but telemetry indicates that collision arithmetic is not the first production bottleneck in this sample.

## 7. Why Current Telemetry Disappears

`handle_clean_render` in `lambda/handler_storage.py` clears every DynamoDB status entry in the selected job partition. This includes compute task telemetry that otherwise has a 24-hour TTL.

Consequences:

- Starting a render destroys the detailed compute telemetry for the job.
- Starting another render destroys the previous render's detailed task rows.
- DynamoDB cannot currently serve as a durable benchmark archive.
- Telemetry must be exported immediately after execution or reconstructed from Step Functions history.

Step Functions Standard history saved this review because each optimized Lambda task result includes both its JSON payload and the Lambda request ID. The request ID can be joined exactly to the corresponding CloudWatch `REPORT` line.

The retention policy should be changed so `CleanRender` removes only data that must be invalidated. Run IDs are unique and status rows already expire through TTL. Broad partition deletion discards useful evidence without providing a correctness benefit.

## 8. Build Attribution Gap

The current state machines invoke `$LATEST`, and task results do not carry a Git revision, build ID, or immutable Lambda version.

This creates a serious benchmark ambiguity:

- An execution can be measured after another deployment has changed `$LATEST`.
- Querying current Lambda configuration then returns the new `CodeSha256`, not necessarily the code that produced the old run.
- A performance report can therefore be attributed to the wrong source revision.

At least one of the following must be implemented:

1. Include a build ID and Git revision in every task result and status row.
2. Publish immutable Lambda versions and have state machines invoke a version or alias tied to the build.
3. As a temporary measure, capture each function's `CodeSha256` and `LastModified` immediately after every benchmark execution.

Option 2 is the strongest deployment contract. Option 1 is still useful because it makes application-level telemetry self-describing.

## 9. Required Instrumentation Changes

### 9.1 Compute worker

The current worktree adds a `stage_telemetry` block to `lambda/handler_compute_chunk_fused.py`. It includes scheduler/native counters, token counts, fused-region counts, CPU count, memory, architecture, and root size.

That code was not present in the production execution measured above. It must be tested and deployed before it can be used as evidence.

The compute result should retain:

- Handler wall time.
- Param wall and native time.
- Coeff wall and native time.
- Solve wall and native time.
- Upload wall time by object family.
- Bytes by object family.
- Reuse flags.
- Scheduler/backend selection.
- Prepared and executed token counts.
- Lambda memory, architecture, CPU count, request ID, and build ID.

### 9.2 Raster worker

`lambda/handler_raster_mt.py` already measures `native_wall_us` around the subprocess but discards it in favor of `raster_meta["native_us"]`. The complete timing set should be returned in the Step Functions result, not only written to transient DynamoDB rows.

Required fields:

- `handler_wall_us`.
- `download_wall_us`.
- `download_worker_us` for the summed thread counter.
- `native_wall_us`.
- `native_worker_us` for the summed native counter.
- Clip/preparation wall time.
- Upload wall time.
- Input and output bytes by family.
- Fragment, sample, collision, and non-finite counters.
- Threads, sections, worker concurrency, memory, architecture, request ID, and build ID.

Names must encode whether a counter is elapsed wall time or accumulated worker time.

### 9.3 Finalizer

`lambda/handler_finalize_mt.py` needs separate wall counters for:

- Presign and manifest preparation.
- Main-fragment download and assembly.
- Step-score fragment download.
- Step-score concatenation.
- Step-score upload.
- LUT construction.
- Color map/render.
- Image encode.
- Raw output upload.
- Associated-palette assembly/render/encode/upload.
- Main image upload.
- Preview upload.
- Complete handler wall.

The fake `encode_ms=0` field should be replaced by a real timer or removed. The broad `upload_ms` field should be split rather than retained as an apparently precise but operationally ambiguous number.

### 9.4 Retention

The preferred order is:

1. Preserve run-scoped DynamoDB rows until TTL.
2. Return all essential telemetry in Step Functions task results.
3. Export normalized summaries immediately after production runs.

DynamoDB is useful for live UI progress. Step Functions history is useful for recovery. Neither should be the only long-term benchmark store.

## 10. Production Telemetry Collector

A repository script should make collection repeatable:

`scripts/capture_production_telemetry.py`

Suggested command shapes:

```bash
uv run python scripts/capture_production_telemetry.py \
  --execution-arn arn:aws:states:us-east-1:ACCOUNT:execution:STATE_MACHINE:EXECUTION
```

```bash
uv run python scripts/capture_production_telemetry.py \
  --kind compute \
  --run-id run_1783921106557_dhafq8
```

The collector should:

1. Call `describe-execution` and paginate `get-execution-history`.
2. Parse state-entered/state-exited events into state wall times.
3. Parse every `TaskSucceeded` payload, including the nested JSON `Payload.body`.
4. Extract the Lambda request ID from `SdkResponseMetadata.RequestId` or the response headers.
5. Query CloudWatch over the exact execution interval and join each `REPORT` line by request ID.
6. Capture duration, billed duration, initialization, configured memory, peak memory, and timeout.
7. Capture function `CodeSha256`, `LastModified`, architecture, ephemeral storage, and layers immediately.
8. Optionally query live DynamoDB task rows when they still exist.
9. Emit one normalized JSON report and one concise Markdown summary.
10. Keep raw execution history outside the repository unless explicitly requested.

Recommended output location:

```text
reports/production/<date>-<kind>-<run-id>.json
reports/production/<date>-<kind>-<run-id>.md
```

The normalized report must carry these metric families separately:

- `workflow_wall_ms`
- `state_wall_ms`
- `task_handler_ms`
- `task_stage_wall_ms`
- `task_worker_sum_ms`
- `task_stage_sum_ms`
- `task_stage_max_ms`
- task p50 and p95
- cold-start count
- billed GB-seconds
- max-memory distribution
- input/output bytes
- fragment and collision counters

The report generator should reject or visibly flag a metric whose timing class is unknown.

## 11. Workload Identity

Performance data without workload identity is not comparable. Every report should include:

### Compute identity

- Job ID and run ID.
- `N`, times, chunk count, degree, and coefficient count.
- Function and solver.
- Param, Coeff, Root, and Solve Score program fingerprints.
- Threads and scheduler/backend choices.
- Reuse flags.
- Output bytes by family.

### Render identity

- Job ID and run ID.
- Pixel dimensions and output format/quality.
- Solve Score program fingerprint and selected metric.
- Palette/LUT identity.
- Sections, workers, threads, and finalize workers.
- Associated-palette setting.
- Input/output bytes by family.
- Collision mode and relevant counters.

### Deployment identity

- Git revision/build ID.
- Lambda function/version or `CodeSha256`.
- Architecture.
- Configured memory and ephemeral storage.
- Relevant feature flags and environment variables.

## 12. Controlled Production Benchmark Matrix

Collect at least three runs per point; five is preferable when variance is visible. Report median and p95 rather than selecting the fastest run.

### 12.1 Compute matrix

Use identical inputs across compared builds and fresh job/output keys so reuse is zero.

Recommended workload classes:

| Class | Purpose |
|---|---|
| Simple Param + simple Coeff | Establish orchestration and I/O floor. |
| Complex Param + simple Coeff | Isolate Param VM changes. |
| Simple Param + complex Coeff | Isolate Coeff VM changes. |
| Complex Param + complex Coeff | Representative combined workload. |
| High-degree/solver-heavy | Measure solver scaling independently of VM improvements. |

Record cold and warm behavior separately. A 49-way first-wave cold burst is not directly comparable with a partially warm execution.

### 12.2 Render matrix

Use the same saved result for every render comparison.

Run at least these controlled pairs:

| Comparison | What it isolates |
|---|---|
| Associated palette off vs on | Duplicate fragment and finalizer cost. |
| Cheap metric vs centroid/radial/median/pair bundle | Solve Score complexity. |
| Fixed sections with worker count sweep | Concurrency and wave behavior. |
| Fixed workers with section-count sweep | Per-section overhead and source retrieval duplication. |
| Fixed sections/workers with thread sweep | Native and download thread scaling. |
| Memory tier sweep | CPU/network allocation versus billed GB-seconds. |

Keep resolution, JPEG quality, palette, collision policy, and source result constant unless one of those is the explicit variable.

Non-deterministic color ownership is acceptable under the CR34 constraint where output validity is preserved, but benchmark comparisons must still use the same semantic mode and report collision counters.

## 13. Immediate Optimization Order

Based on the measured production runs, work should proceed in this order:

1. Land durable, correctly named telemetry and the collector.
2. Preserve status rows until TTL or export them before `CleanRender` removes them.
3. Deploy and validate the current compute `stage_telemetry` addition.
4. Test safe compute overlap: Param upload with Coeff generation, then coefficient upload with solve.
5. Profile and reduce raster source retrieval and repeated large-object reads.
6. Remove associated-palette sparse fragment production where the palette can be derived from retained step scores.
7. Instrument and parallelize the finalizer's currently unmeasured step-score path.
8. Sweep section count, worker concurrency, threads, and memory using billed GB-seconds as well as latency.
9. Continue Coeff and solver optimization where stage telemetry shows meaningful wall-time contribution.
10. Do not spend another optimization cycle on Param generation unless representative deployed telemetry shows a regression or a workload where it is material.

## 14. Acceptance Criteria

The production telemetry work is complete when:

- Every optimized Lambda result contains a build identity and request ID.
- Every important stage reports elapsed wall time with unambiguous names.
- Summed worker counters remain available but are explicitly labeled as worker sums.
- Compute, raster, and finalizer task results retain enough data to reconstruct a report after DynamoDB cleanup.
- `CleanRender` no longer erases unrelated run telemetry before TTL.
- The collector joins Step Functions and CloudWatch by request ID without heuristic timestamp matching.
- Reports record workload and deployment identity.
- Reports show wall, task sum, maximum, p50, p95, cold starts, bytes, memory, and billed GB-seconds.
- A controlled repeat of the compute and render baselines can be compared across two immutable builds.

## 15. Bottom Line

Production evidence already changes the optimization priority. Param generation has fallen to a negligible share in the sampled compute run. Compute is now a solver, Coeff, and data-movement problem. Rendering is primarily a source-retrieval/data-movement problem, with an avoidable associated-palette fragment path and under-instrumented finalization behind it.

The next step is not another speculative micro-optimization. It is to make these measurements durable and truthful, then run controlled production comparisons against the largest measured costs.
