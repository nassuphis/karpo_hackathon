# Improve Logs And Liveness Plan

This document specifies the next render-status/logging cleanup.

Goal:

- make the Render tab distinguish `alive but slow` from `actually stalled`
- expose useful worker progress without scraping CloudWatch logs
- stop relying on the top-level render row timestamp as the primary liveness signal
- keep the browser as an observer only

This is not a generic logging rewrite for the whole app. This is specifically for:

- render workflow top-level status rows
- render worker status rows
- `/check-status`
- Render-tab observer logic in [`index.html`](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)


## 1. Current Problems

The current implementation has the right building blocks, but the wrong primary signal.

### 1.1 Top-level render rows only update at phase boundaries

[`lambda/handler_render_status.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_status.py) writes the top-level `queued` / `phase` / `done` / `error` row.

That row gets `updated_at_ms`, but only when Step Functions enters a new phase.

This means:

- a long `Map` phase can run for minutes with no top-level row update
- the browser can wrongly conclude the run is dead even while workers are actively running

### 1.2 Worker rows have statuses but no heartbeat timestamp surface

[`lambda/shared.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/shared.py) `report_status()` currently writes:

- `job_id`
- `task_id`
- `task_status`
- `ttl`
- optional `error_msg`
- optional `result_data`

It does **not** write a top-level `updated_at_ms` attribute.

So worker rows already contain useful status transitions, but `/check-status` cannot cheaply aggregate worker freshness.

### 1.3 `/check-status` returns counts, not liveness

[`lambda/handler_storage.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py) `handle_check_status()` currently returns:

- `done`
- `errors`
- `error_details`
- `stuck`
- `status_counts`
- `total`
- `expected`
- `complete`
- optional `results`
- optional `found_ids`

It does **not** return:

- latest worker update time
- latest done time
- latest nonterminal worker update time
- newest task/status
- stale age derived from worker timestamps

So the frontend cannot tell:

- `workers are still heartbeating`
- `workers are stuck but not errored`
- `workers never started`

### 1.4 Browser stale detection uses the wrong signal

[`index.html`](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) `_pollActiveRenderRun()` currently:

- polls the top-level row
- optionally polls worker rows using `subtask_prefix`
- then declares a stall if the top-level row is older than 5 minutes and secondary poll has `done == 0`

That is better than the previous version, but still wrong.

A legitimate long-running phase can have:

- old top-level row
- fresh worker activity
- `done == 0` because no item has finished yet

That should be treated as `alive`, not `stalled`.

### 1.5 The current `result_data["log"]` string is not the right fix

[`lambda/handler_render_status.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_status.py) writes a timestamped `result_data["log"]` string for top-level rows.

That is not enough because:

- it only changes at phase boundaries
- it is not the source of truth for worker liveness
- the browser currently does not consume it
- `_prev_log` is not threaded through the workflow, so it is not a durable server-side event log

Do not try to solve this by stuffing more strings into `result_data["log"]`.


## 2. Scope

This change covers:

1. worker heartbeat timestamps
2. richer `/check-status` aggregation
3. standardized render-phase progress fields
4. Render-tab observer logic
5. tests for alive-vs-dead distinction

This change does **not** cover:

- CloudWatch log ingestion
- server-side persistent append-only logs
- Step Functions execution-history parsing in the browser
- libvips native progress as a dependency for liveness


## 3. Required Outcome

After this change:

1. If a render phase is still producing worker status updates, the Render tab must show it as active even if the top-level row is old.
2. If worker updates stop for long enough, the Render tab must show a warning first, then a hard stall.
3. The UI must show more than `phase + done/expected`; it must also show a recency signal such as `last worker update 12s ago`.
4. `/check-status` must provide enough information for the browser to make this decision without fetching CloudWatch or Step Functions execution history.
5. Existing worker `report_status()` calls must remain; this change builds on them.


## 4. Non-Negotiables

1. Do **not** remove worker `report_status()` calls. They are now required for liveness.
2. Do **not** make the browser poll Step Functions `DescribeExecution`.
3. Do **not** scrape CloudWatch logs from the browser or from a Lambda.
4. Do **not** use only the top-level render row timestamp for stall detection.
5. Do **not** treat `done > 0` as the only proof of life.
6. Do **not** spam DDB with tight heartbeat loops. Heartbeats come from meaningful status transitions already present in workers.
7. Do **not** solve this by appending giant text blobs into `result_data["log"]`.
8. Do **not** make libvips progress hooks a prerequisite for this change.


## 5. Data Model Changes

### 5.1 Add top-level `updated_at_ms` to all worker rows

File:

- [`lambda/shared.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/shared.py)

Change:

- `report_status()` must always write a top-level DynamoDB numeric attribute:
  - `updated_at_ms`

Exact rule:

- compute `now_ms = int(time.time() * 1000)` inside `report_status()`
- include:
  - `"updated_at_ms": {"N": str(now_ms)}`

Do this for **every** worker row, regardless of status.

This must apply to:

- render workers
- bilevel workers
- solve-score workers
- preview/export workers that already use `report_status()`

### 5.2 Add top-level `updated_at_ms` to top-level render rows too

File:

- [`lambda/handler_render_status.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_status.py)

Change:

- `_put_row()` must also write a top-level DynamoDB numeric attribute:
  - `updated_at_ms`

Do not rely only on `result_data["updated_at_ms"]`.

Reason:

- `/check-status` should be able to aggregate timestamps without parsing every `result_data` blob


## 6. `/check-status` Response Upgrade

File:

- [`lambda/handler_storage.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)

### 6.1 Projection

Update `ProjectionExpression` to include:

- `updated_at_ms`

Keep existing fields:

- `task_id`
- `task_status`
- `error_msg`
- `result_data`

### 6.2 Required new response fields

`handle_check_status()` must continue returning current fields, and additionally return:

- `latest_update_ms`
  - max `updated_at_ms` across all matching rows
- `latest_done_ms`
  - max `updated_at_ms` among rows with `task_status == "done"`
- `latest_nonterminal_ms`
  - max `updated_at_ms` among rows with status not in `done,error`
- `stale_for_ms`
  - if `latest_update_ms` exists: `now_ms - latest_update_ms`
  - else `null`

### 6.3 Optional debugging field

`newest_task` is optional and is **not** required for liveness decisions.

If included, it should be:

- `newest_task`
  - object with:
    - `task_id`
    - `status`
    - `updated_at_ms`

This is a debugging convenience only.

Do not block the implementation on it.

### 6.4 Extend existing `stuck` entries

Each entry in `stuck` must include:

- `task_id`
- `status`
- `updated_at_ms`
- `age_ms`

`age_ms` is:

- `now_ms - updated_at_ms`

### 6.5 Optional `results`

Keep existing `results` behavior for callers that need worker `result_data`.

Do not remove it.

### 6.6 Do not add paginator scans outside the current query

This must remain a single DDB `query()` loop over the existing prefix.

Do not add:

- S3 probes
- Step Functions calls
- CloudWatch calls


## 7. Worker Progress Semantics

This change does **not** require a full worker-status renaming campaign.

However, render-related workers must have at least one meaningful intermediate `report_status()` between `started` and `done` when the worker does substantial work.

### 7.1 Minimum render worker status expectations

Keep or add meaningful statuses in these files:

- [`lambda/handler_raster.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_raster.py)
  - `started`
  - `bin_downloaded`
  - `rasterized`
  - `done`
- [`lambda/handler_finalize.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_finalize.py)
  - `started`
  - repeated `reading_*`
  - `assembled`
  - `done`
- [`lambda/handler_encode.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_encode.py)
  - `started`
  - `stitching`
  - `encoding`
  - `done`
- [`lambda/handler_bilevel.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_bilevel.py)
  - keep `started`
  - add one mid-phase status if a mode currently jumps straight to `done`
- [`lambda/handler_coeff_bilevel_stitch.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_coeff_bilevel_stitch.py)
  - `started`
  - `stitching`
  - `done`
- [`lambda/handler_solve_proximity.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_solve_proximity.py)
  - keep existing `started` / `bin_downloaded` / `computed` / `merged` / `done`

### 7.2 Do not over-instrument

Do not add tight loop status writes like:

- once per pixel
- once per root
- once per tile upload

The goal is:

- meaningful stage transitions
- not high-frequency heartbeat spam


## 8. Frontend Observer Rules

File:

- [`index.html`](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Function:

- `_pollActiveRenderRun()`

### 8.1 Keep the two-poll model

The observer must continue doing:

1. top-level `/check-status` on the render-run task id
2. secondary `/check-status` on `subtask_prefix` when available

Do not collapse back to a single poll.

### 8.2 Replace current stale logic

Current logic:

- top-level row stale > 5 min
- and not (`subtask_prefix && done > 0`)
- => mark stalled

This must be replaced.

### 8.3 New liveness rules

When `rd.subtask_prefix` exists:

1. use secondary poll freshness as the primary signal
2. if `subcheck.latest_update_ms` is recent, treat the phase as alive even if top-level row is old
3. only warn/fail based on worker freshness, not top-level row freshness

### 8.3.1 Optional fallback during mixed rollouts

If you need one-release compatibility with older worker rows that do not yet have `updated_at_ms`, you may add a browser-side fallback:

- remember the previous subtask `done` count
- if `done` increases across polls, treat the phase as alive

If this fallback is implemented:

- it must be explicitly temporary
- it must not replace `latest_update_ms` as the primary signal
- it must be removed after the rollout window if it is no longer needed

When `rd.subtask_prefix` does **not** exist:

1. fall back to top-level `rd.updated_at_ms`
2. use the no-subtask thresholds below

### 8.4 New thresholds

Use these exact constants in the browser:

- `RENDER_NO_ROW_STALE_MS = 120000`
- `RENDER_WARN_STALE_MS = 300000`
- `RENDER_HARD_STALE_MS = 900000`

Interpretation:

- no top-level row for >2 min after launch:
  - same as today; treat as stale abandoned run and clear local active run
- worker/top-level freshness >5 min:
  - warning state only
- worker/top-level freshness >15 min:
  - hard stall state

### 8.5 Warning vs hard stall behavior

At warning threshold:

- do **not** clear active run
- do **not** stop observer
- show warning text such as:
  - `Raster 0/10 · no worker update for 5+ min`
- log one warning line to `render-log`

At hard stall threshold:

- show error text such as:
  - `Render stalled (no worker update for 15+ min)`
- keep active run record
- keep observer running
- do **not** silently clear the run

Reason:

- the user may want to keep watching
- the run may still recover
- automatic clearing destroys useful debugging context

### 8.6 Status text format

When subtask data exists, render status should be:

- `<phase_label> <done>/<expected>`

and, if freshness is available:

- `<phase_label> <done>/<expected> · last update <N>s ago`

Do not include raw epoch timestamps in the UI.

### 8.7 Render-log rules

The browser `render-log` must log:

- phase transitions
- warning threshold crossed
- hard stall threshold crossed
- recovery after a warning/stall
- terminal error
- terminal done

It must **not** log every 3-second poll iteration.


## 9. Top-Level Status Lambda

File:

- [`lambda/handler_render_status.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_status.py)

### 9.1 Keep top-level phase logs lightweight

Do not try to make the status Lambda accumulate a full workflow transcript in DDB.

Acceptable:

- keep a single phase log entry
- keep a bounded small log tail

Not acceptable:

- ever-growing `result_data["log"]` strings carried through Step Functions state

### 9.2 Recommended minimal change

For this iteration:

- keep current `phase_label`
- keep `updated_at_ms`
- keep `subtask_prefix`
- keep `expected`
- keep readable `error_msg`

No requirement to redesign top-level phase log storage in this change.

The actual liveness improvement comes from worker freshness, not from top-level log strings.


## 10. Libvips Progress

There is evidence that libvips supports progress hooks and the CLI has `--vips-progress`.

That is **not** the primary deliverable of this change.

### 10.1 Explicitly out of scope for v1

Do not block this change on:

- wiring native libvips callbacks
- parsing `vips` CLI stdout/stderr
- inventing percentage bars for every worker

### 10.2 Optional phase 2 only

If later desired, libvips progress should only be added to long single-worker phases such as:

- DeepZoom export
- preview generation
- TIFF/PNG conversion
- encode, if it is meaningfully long in practice

If implemented later:

- use proper API callbacks where available
- do not scrape CLI text as the primary mechanism


## 11. Exact Implementation Order

Implement in this order.

### 11.1 Shared heartbeat timestamps

1. Update [`lambda/shared.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/shared.py) `report_status()` to write top-level `updated_at_ms`.
2. Update [`lambda/handler_render_status.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_status.py) `_put_row()` to write top-level `updated_at_ms`.

### 11.2 Storage aggregation

3. Update [`lambda/handler_storage.py`](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py) `handle_check_status()` to:
   - project `updated_at_ms`
   - compute latest freshness fields
   - extend `stuck` entries with age/timestamp
   - optionally expose `newest_task`

### 11.3 Worker heartbeat completeness

4. Audit render-related worker Lambdas for at least one meaningful mid-phase `report_status()` call.
5. Add missing mid-phase statuses only where a worker currently jumps from `started` to `done`.

### 11.4 Browser liveness logic

6. Update [`index.html`](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) `_pollActiveRenderRun()` to:
   - read `subcheck.latest_update_ms`
   - compute worker freshness
   - distinguish warning vs hard stall
   - stop clearing the active run on stall
   - log warning/recovery transitions once

### 11.5 Optional UI detail

7. If needed, add a small helper to format recency strings like:
   - `12s`
   - `3m`
   - `14m`

Do not add a big UI redesign.


## 12. Tests

These tests are required.

### 12.1 `tests/test_pipeline.py`

Add/extend tests for:

1. `shared.report_status()` writes top-level `updated_at_ms`
2. `handler_render_status._put_row()` writes top-level `updated_at_ms`
3. `handle_check_status()` returns:
   - `latest_update_ms`
   - `latest_done_ms`
   - `latest_nonterminal_ms`
   - `newest_task`
   - `stale_for_ms`
4. `stuck` entries include:
   - `task_id`
   - `status`
   - `updated_at_ms`
   - `age_ms`
5. mixed statuses aggregate correctly:
   - one `done`
   - one `started`
   - one `error`
6. missing `updated_at_ms` rows do not crash aggregation

### 12.2 `tests/test_render_status.py`

Add/extend tests for:

1. `queued` row includes top-level `updated_at_ms`
2. `phase` row includes top-level `updated_at_ms`
3. `done` row includes top-level `updated_at_ms`
4. `error` row includes top-level `updated_at_ms`

### 12.3 `tests/test_frontend_js.sh`

Add explicit observer tests for:

1. top-level stale + fresh subtask heartbeat => **not** stalled
2. top-level stale + no subtask heartbeat for >5 min => warning text
3. top-level stale + no subtask heartbeat for >15 min => hard stall text
4. warning/hard stall does **not** clear local active run
5. no top-level row for >2 min still clears abandoned run
6. no-subtask phase uses top-level `updated_at_ms`
7. recovery from stale warning to fresh heartbeat clears warning state

### 12.4 Playwright

Add one real browser test file or extend an existing render observer spec to verify:

1. mocked in-progress subtask run shows `done/expected`
2. stale top row + fresh worker heartbeat does not show stalled
3. stale worker heartbeat shows warning/hard stall text

This can use intercepted `/check-status` responses. It does not need real worker execution.


## 13. Manual Validation

After deploy, validate all of these.

### 13.1 Healthy long phase

1. Start a large color render.
2. Wait until it enters raster or finalize.
3. Confirm Render tab shows:
   - phase name
   - `done/expected`
   - recency text or equivalent freshness indicator
4. Confirm it does **not** show stalled while worker rows are still updating.

### 13.2 Simulated stale workers

Using a controlled test or temporary instrumentation:

1. force worker heartbeat to stop without producing terminal error
2. confirm warning appears after 5 minutes
3. confirm hard stall appears after 15 minutes
4. confirm active run is **not** auto-cleared

### 13.3 Recovery

1. after warning state, resume worker updates
2. confirm UI returns to active phase text
3. confirm `render-log` records recovery once

### 13.4 Error path

1. force a worker failure
2. confirm `/check-status` returns `error_details`
3. confirm browser shows terminal error immediately, not warning/stall


## 14. Anti-Cheat Checklist

Reject the implementation if any of these are true.

1. `report_status()` still does not write top-level `updated_at_ms`.
2. `handle_check_status()` still cannot report latest worker freshness.
3. Browser still decides staleness from top-level row timestamp alone.
4. Browser still uses `done > 0` as the only sign of life.
5. Browser clears active run immediately on warning/hard stall.
6. Implementation adds Step Functions polling to the browser.
7. Implementation adds CloudWatch log scraping anywhere in the request path.
8. Implementation claims “improved logs” but only changes `result_data["log"]`.
9. Implementation adds dozens/hundreds of heartbeat writes inside tight loops.
10. Tests only check new field presence, not the stale/alive decision logic.
11. Playwright/frontend tests do not cover the fresh-worker-vs-stale-top-row case.
12. Any change removes existing worker `report_status()` calls.


## 15. Nice-To-Haves After This

These are explicitly later work, not part of the core change:

1. standardized structured `result_data.progress` payloads
2. bounded top-level log tail for phase history
3. libvips/native progress hooks for long single-worker steps
4. render-status UI secondary line showing newest worker task/status
5. per-phase expected-duration heuristics
