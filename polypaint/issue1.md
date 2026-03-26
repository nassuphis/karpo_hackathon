# Issue 1: SFN Runtime Errors Bypass State-Level Catch

## What Happened

1. User launched a color render with solve_score enabled
2. Starter Lambda wrote `task_status = queued` to DDB row
3. Starter Lambda called `StartExecution` successfully
4. SFN entered `ColorSolveScoreClipTask`
5. During **parameter resolution** (before the Lambda was invoked), ASL tried to resolve `$.plan.params.root_transforms`
6. That JSONPath didn't exist because the browser didn't send `root_transforms`
7. SFN raised `States.Runtime` error
8. The execution **failed at the top level** — the state's `Catch` block did not fire
9. `ReportError` never ran
10. DDB row stayed at `task_status = solve_score_clip`, `phase_label = "Solve score: clip"` forever
11. Browser observer polled DDB, saw a non-done, non-error row, and displayed "Solve score: clip" indefinitely
12. On page reload, observer restored the active run from localStorage and resumed polling the same stale row
13. On "Refresh" artifacts, the code restored the previous status text ("Solve score: clip") instead of clearing it

## Why the Catch Didn't Fire

ASL state-level `Catch` handles errors that occur **during task execution** (Lambda failures, timeouts, etc.). But `States.Runtime` errors from **parameter resolution** — JSONPath evaluation that happens before the task is dispatched — are not caught by the state's own `Catch` block. They propagate directly to the execution level and terminate it.

From the execution history:

```
2026-03-27T01:34:42.704  ExecutionFailed
  error=States.Runtime
  cause=An error occurred while executing the state 'ColorSolveScoreClipTask'
        (entered at the event id #23). The JSONPath '$.plan.params.root_transforms'
        specified for the field 'root_transforms.$' could not be found in the input.
```

There is no `TaskFailed` or `LambdaFunctionFailed` event. The task was never started.

## The Real Problem

The issue is not specifically `root_transforms`. That was fixed by adding a default in the plan Lambda. The real problem is structural:

**Any missing or mistyped JSONPath in any state's `Parameters` block will kill the entire execution with no error reported to DDB.**

This will happen again the next time:
- A new field is added to a worker payload but the plan Lambda doesn't include it
- A field name is misspelled in the ASL template
- An optional field (like `root_transforms`, `solve_score_bins_key`) is conditionally absent

The current ASL has dozens of `"field.$": "$.plan.something"` references across 40+ states. Each one is a potential `States.Runtime` crash that bypasses all Catch blocks.

## What Needs to Be Fixed

### 1. Structural: Catch States.Runtime at the workflow level

Wrap the entire workflow body in a top-level `Parallel` state (single branch) with a `Catch` that handles `States.ALL` including `States.Runtime`. Route to `ReportError` so the DDB row always gets an error status written.

Without this, any parameter resolution failure in any state silently kills the run.

### 2. Defensive: Plan Lambda must guarantee all JSONPaths exist

Every field referenced by `".$"` in the ASL template must be present in the plan output — no exceptions, no conditional fields. If a value is optional, the plan Lambda must emit it with a default (empty list, null-equivalent, zero).

This was partially done for `root_transforms` but needs to be systematic: audit every `".$"` reference in the template against the plan output shape.

### 3. Browser: Observer must handle silent execution failure

If the DDB row exists but is stuck in a non-terminal state (`task_status` is not `done` or `error`) for longer than a threshold, the observer should:
- Check whether the SFN execution is still running (optional, requires new endpoint)
- Or assume failure after a timeout and show an error to the user

The 2-minute stale-run detection added as a workaround handles the case where the row doesn't exist, but not the case where the row exists and is stuck at a phase that will never complete.

## Affected States

Every state in the ASL that uses `"field.$": "$.plan...."` in its `Parameters` block is vulnerable. This includes all worker invocations in all Map states and all single Task states — essentially the entire workflow after `BuildPlan`.
