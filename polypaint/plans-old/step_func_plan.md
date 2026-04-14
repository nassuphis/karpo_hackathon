# Step Functions Migration Plan

Status: legacy archive. Historical planning note preserved for reference; the Render workflow has already migrated to Step Functions, and newer docs use chunk-based naming plus immutable artifact-family catalogs.

## 1. Problem

The render orchestrator (`handler_render_orchestrator.py`) self-invokes via async
Lambda when approaching the 15-minute timeout. AWS recursive loop detection sees
`polypaint-render-orchestrator` → `polypaint-render-orchestrator` → ... and kills
the chain.

Current workarounds:
- `put-function-recursive-config AllowRecursiveInvocations` (fragile, opt-out of a safety feature)
- Higher Lambda timeout (already at 900s max)

Both are band-aids. The orchestrator is doing what Step Functions was designed for.

## 2. Current Architecture

### 2.1 Orchestrator flow (color mode)

```
Browser → dispatch Lambda → render orchestrator (async)
                                  │
                                  ├─ clean (sync call to storage)
                                  ├─ viewport (sync call to viewport)
                                  ├─ calc_meta (sync call to storage)
                                  │
                                  ├─ [if solve_score] solve_score_clip → poll DDB
                                  ├─ [if solve_score] solve_score_hist (batch) → poll DDB
                                  ├─ [if solve_score] solve_score_merge → poll DDB
                                  │
                                  ├─ raster (batch dispatch) → poll DDB → stall re-dispatch
                                  ├─ finalize (batch dispatch) → poll DDB
                                  ├─ encode (single dispatch) → poll DDB
                                  ├─ preview (single dispatch) → poll DDB
                                  └─ done → write DDB status
```

### 2.2 Bilevel mode

```
orchestrator → clean → viewport → calc_meta
             → bilevel_raster (batch) → poll
             → bilevel_merge (batch) → poll
             → bilevel_stitch (single) → poll
             → preview → poll → done
```

### 2.3 Coeff bilevel mode

Same as bilevel but dispatches `coeff_raster` instead of `bilevel_raster`.

### 2.4 What the orchestrator Lambda currently does

1. **Phase sequencing** — runs phases in order, each phase depends on the previous
2. **Batch dispatch** — sends N async Lambda invocations (wave dispatch, batch_size=50)
3. **DDB polling** — loops `sleep(3) + query DDB` waiting for all tasks to report done
4. **Stall re-dispatch** — if no progress for 45s, re-dispatches missing tasks (up to 2x)
5. **Checkpoint/self-reinvoke** — when `remaining_ms < 60s`, serializes state to JSON, invokes self
6. **Progress reporting** — writes phase/done/expected to DDB so browser can observe
7. **Error propagation** — if any worker errors, raises to mark the whole run as error

### 2.5 What the browser does

- Dispatches one orchestrator job via `lambdaPost('dispatch', { target: 'render_orchestrator', jobs: [payload] })`
- Saves `{ job_id, run_id, task_id }` to localStorage
- Polls DDB every 3s via `lambdaPost('storage', ..., '/check-status')`
- Reads `phase`, `done`, `expected` from the task row
- Calls `refreshRenderArtifacts()` when phase = done

## 3. Target Architecture

### 3.1 Overview

Replace the single orchestrator Lambda with an AWS Step Functions state machine.
Each phase becomes a state. Batch-dispatch-and-poll becomes a Map state or a
poll-loop sub-state-machine.

```
Browser → dispatch Lambda → Step Functions (start execution)
                                  │
                                  ├─ CleanState (Lambda)
                                  ├─ ViewportState (Lambda)
                                  ├─ CalcMetaState (Lambda)
                                  │
                                  ├─ [Choice] SolveScoreCheck
                                  │     ├─ SolveClipState (Lambda)
                                  │     ├─ SolveHistState (Map → Lambda)
                                  │     │     └─ PollHistCompletion (poll loop)
                                  │     └─ SolveMergeState (Lambda)
                                  │
                                  ├─ RasterDispatchState (Lambda — dispatches batch)
                                  ├─ RasterPollState (Lambda — polls DDB, loops via Wait)
                                  │
                                  ├─ FinalizeDispatchState (Lambda)
                                  ├─ FinalizePollState (Lambda — polls, loops)
                                  │
                                  ├─ EncodeState (Lambda)
                                  ├─ EncodePollState (Lambda — polls, loops)
                                  │
                                  ├─ PreviewState (Lambda)
                                  └─ DoneState (Lambda — writes final DDB status)
```

### 3.2 Key design decisions

**A. Each phase = one state (not one Lambda)**

The existing worker Lambdas (raster, finalize, encode, bilevel, etc.) stay unchanged.
Only the orchestrator is replaced. Each phase becomes a Step Functions state that
either invokes a worker directly or dispatches a batch and polls.

**B. Poll loops use Wait + Lambda, not callback**

The existing workers report completion to DDB and don't know about Step Functions.
Changing all workers to use `SendTaskSuccess` would be a much larger migration.
Instead, the poll states use a `Wait(3s) → Lambda(check DDB) → Choice(done? error?)` loop.

This is the same pattern as the current `_poll_completion()` but Step Functions owns
the loop instead of a single long-running Lambda.

**C. Batch dispatch stays async Lambda invocation**

Raster, finalize, hist all dispatch N async Lambdas. This doesn't change.
The dispatch Lambda sends the batch, then the poll state waits for DDB completion.

**D. Stall re-dispatch moves into the poll Lambda**

The poll Lambda checks for stalled tasks (no progress for 45s) and re-dispatches,
same as current `_poll_completion()` logic.

**E. Progress reporting stays in DDB**

Each state writes progress to DDB via `report_status()`. The browser keeps polling
DDB unchanged. Step Functions execution ARN is stored in the DDB row for debugging.

**F. Browser dispatch changes minimally**

Instead of `lambdaPost('dispatch', { target: 'render_orchestrator', jobs: [payload] })`,
the dispatch handler calls `stepfunctions.start_execution()`. The response returns
an execution ARN. The browser still polls DDB — no change to the observer.

## 4. State Machine Definitions

### 4.1 Color mode state machine

```json
{
  "StartAt": "Clean",
  "States": {
    "Clean": {
      "Type": "Task",
      "Resource": "arn:aws:lambda:...:polypaint-sfn-phase",
      "Parameters": { "phase": "clean", "params.$": "$" },
      "Next": "Viewport"
    },
    "Viewport": {
      "Type": "Task",
      "Parameters": { "phase": "viewport", "params.$": "$" },
      "Next": "CalcMeta"
    },
    "CalcMeta": {
      "Type": "Task",
      "Parameters": { "phase": "calc_meta", "params.$": "$" },
      "Next": "SolveScoreCheck"
    },
    "SolveScoreCheck": {
      "Type": "Choice",
      "Choices": [{
        "Variable": "$.color_mode",
        "StringEquals": "solve_score",
        "Next": "SolveClip"
      }],
      "Default": "RasterDispatch"
    },
    "SolveClip": {
      "Type": "Task",
      "Parameters": { "phase": "solve_clip", "params.$": "$" },
      "Next": "SolveHistDispatch"
    },
    "SolveHistDispatch": {
      "Type": "Task",
      "Parameters": { "phase": "solve_hist_dispatch", "params.$": "$" },
      "Next": "SolveHistPoll"
    },
    "SolveHistPoll": {
      "Type": "Task",
      "Parameters": { "phase": "poll", "params.$": "$" },
      "Next": "SolveHistPollCheck",
      "Retry": []
    },
    "SolveHistPollCheck": {
      "Type": "Choice",
      "Choices": [{
        "Variable": "$.poll_complete",
        "BooleanEquals": true,
        "Next": "SolveMerge"
      }],
      "Default": "SolveHistPollWait"
    },
    "SolveHistPollWait": {
      "Type": "Wait",
      "Seconds": 3,
      "Next": "SolveHistPoll"
    },
    "SolveMerge": {
      "Type": "Task",
      "Parameters": { "phase": "solve_merge", "params.$": "$" },
      "Next": "RasterDispatch"
    },
    "RasterDispatch": { "...": "same pattern: dispatch → poll → wait → check" },
    "FinalizeDispatch": { "...": "same pattern" },
    "EncodeDispatch": { "...": "same pattern" },
    "PreviewDispatch": { "...": "same pattern" },
    "Done": {
      "Type": "Task",
      "Parameters": { "phase": "done", "params.$": "$" },
      "End": true
    }
  }
}
```

### 4.2 Bilevel and coeff_bilevel

Separate state machine definitions (or a single parameterized one with Choice states).
Same dispatch→poll→wait pattern but different worker targets and phase names.

## 5. New Lambda Functions

### 5.1 `polypaint-sfn-phase` (new, replaces orchestrator)

Single Lambda that handles all phase types based on input `phase` field.
Each invocation is short-lived (seconds, not minutes). No self-reinvoke.

Phases:

| Phase | Action | Duration |
|-------|--------|----------|
| `clean` | Sync call to storage `/clean-render` | <1s |
| `viewport` | Sync call to viewport Lambda | <2s |
| `calc_meta` | Sync call to storage `/detail` | <1s |
| `solve_clip` | Dispatch 1 clip Lambda, poll DDB once | <5s |
| `solve_hist_dispatch` | Dispatch N hist Lambdas | <5s |
| `solve_merge` | Dispatch 1 merge Lambda, poll DDB once | <5s |
| `raster_dispatch` | Dispatch N raster Lambdas (wave) | <10s |
| `finalize_dispatch` | Dispatch N finalize Lambdas | <5s |
| `encode_dispatch` | Dispatch 1 encode Lambda | <1s |
| `preview_dispatch` | Dispatch 1 preview Lambda | <1s |
| `poll` | Query DDB, check completion, re-dispatch stalls | <2s |
| `done` | Write final status to DDB | <1s |

Each invocation returns state for the next state to consume (job_id, clip_key,
bins_key, n_tiles, etc.). Step Functions passes this between states.

### 5.2 Existing Lambdas — unchanged

All worker Lambdas (raster, finalize, encode, bilevel, solve_proximity, etc.)
remain exactly as they are. They still:
- Report status to DDB via `report_status()`
- Don't know about Step Functions
- Are invoked asynchronously

## 6. Infrastructure Changes

### 6.1 IAM

New role or policy additions:
- `states:StartExecution` on the state machine ARN
- `states:DescribeExecution` for debugging
- The state machine execution role needs `lambda:InvokeFunction` on `polypaint-sfn-phase`
- The phase Lambda needs the same permissions as the current orchestrator (S3, DDB, Lambda invoke)

### 6.2 deploy.sh changes

1. **Remove** `polypaint-render-orchestrator` Lambda creation/update
2. **Add** `polypaint-sfn-phase` Lambda creation (same package, different handler)
3. **Add** state machine definition JSON files (one per mode, or one parameterized)
4. **Add** `aws stepfunctions create-state-machine` / `update-state-machine`
5. **Add** state machine execution role creation
6. **Update** dispatch handler to call `stepfunctions.start_execution()` instead of `lambda.invoke()`

### 6.3 API Gateway / dispatch handler

The dispatch handler (`handler_dispatch.py`) currently invokes the orchestrator Lambda:

```python
if target == "render_orchestrator":
    lambda_client.invoke(FunctionName=..., InvocationType="Event", ...)
```

Replace with:

```python
if target == "render_orchestrator":
    sfn_client.start_execution(
        stateMachineArn=STATE_MACHINE_ARN,
        name=f"render_{run_id}",
        input=json.dumps(payload),
    )
```

### 6.4 DDB — no change

The DDB table and its schema stay exactly the same. Progress rows, status tracking,
check-status queries — all unchanged.

### 6.5 Frontend — minimal change

The browser dispatch call stays the same (`lambdaPost('dispatch', ...)`).
The observer polling stays the same (polls DDB via `/check-status`).

The only possible change: the dispatch response could include the Step Functions
execution ARN for debugging. Optional.

## 7. Migration Strategy

### 7.1 Phase 1: Parallel deployment (both paths live)

1. Deploy the Step Functions state machine and `polypaint-sfn-phase` Lambda
2. Keep the existing orchestrator Lambda alive
3. Add a feature flag (`use_step_functions: true/false`) to the dispatch handler
4. Test Step Functions path end-to-end with a flag flip
5. The old recursive-loop-detection workaround stays as fallback

### 7.2 Phase 2: Cut over

1. Default the feature flag to `true`
2. Monitor for a few renders
3. Remove the flag and the old orchestrator Lambda
4. Remove the recursive loop detection override

### 7.3 Phase 3: Cleanup

1. Delete `handler_render_orchestrator.py`
2. Delete the orchestrator Lambda from deploy.sh
3. Remove checkpoint/self-reinvoke logic from all test fixtures
4. Update tests to verify Step Functions state transitions instead

## 8. What Gets Simpler

| Current | Step Functions |
|---------|---------------|
| Homegrown checkpoint serialization | Step Functions owns state between phases |
| Self-reinvoke + recursive loop detection bypass | Native state transitions, no recursion |
| 15-min wall clock for entire pipeline | Each phase gets its own 15 min |
| Stall detection in poll loop inside Lambda | Stall detection in a short poll Lambda, Step Functions retries the wait |
| Error propagation via DDB | Step Functions Catch/Retry + DDB (for browser) |
| One monolithic 700-line orchestrator | Small phase Lambda + declarative state machine |

## 9. What Gets Harder

| Current | Step Functions |
|---------|---------------|
| One Lambda, one deployment unit | State machine JSON + phase Lambda + IAM role |
| Debugging: read one CloudWatch log stream | Debugging: Step Functions console + CloudWatch |
| Cost: 1 Lambda invocation (self-reinvokes are cheap) | Cost: state transitions billed per transition (~$0.025/1000) |
| Local testing: mock one Lambda | Local testing: mock Step Functions or use stepfunctions-local |

## 10. Cost Estimate

A typical color render has ~15 states × 5 poll iterations = ~75 state transitions.
At $0.025 per 1000 transitions, that's $0.002 per render. Negligible.

The phase Lambda runs ~15 short invocations instead of 1 long one. Lambda cost
is comparable (billed per 1ms, same total compute).

## 11. Risks

1. **State machine JSON is verbose** — ASL (Amazon States Language) is not concise. The
   color mode state machine alone will be ~200 lines of JSON. Bilevel adds another ~150.
   Consider using CDK or SAM for readability.

2. **Poll loop latency** — Step Functions Wait states have ~1s overhead per transition.
   A 3s wait becomes ~4s. For a 10-stripe raster with 30s completion time, this adds
   maybe 10s total. Acceptable.

3. **DDB race conditions** — same as current. A worker can finish between the poll query
   and the next wait. Not a correctness issue (next poll catches it).

4. **State size limit** — Step Functions input/output is limited to 256KB per state.
   The current progress dict with `_last_dispatched_jobs` for 100+ raster jobs could
   exceed this. Solution: store large payloads in S3, pass only keys through states.

5. **Execution history limit** — Step Functions Standard has 25,000 history events per
   execution. With 75 transitions per render this is fine, but Express workflows
   (cheaper, no history) could be considered if cost matters.

## 12. Immediate Action

Before starting this migration, apply the one-line fix to unblock renders now:

```bash
aws lambda put-function-recursive-config \
  --function-name polypaint-render-orchestrator \
  --recursive-loop AllowRecursiveInvocations \
  --region us-east-1
```

Add this to `deploy.sh` after the orchestrator creation/update blocks so it
persists across deploys. The Step Functions migration is a separate project.
