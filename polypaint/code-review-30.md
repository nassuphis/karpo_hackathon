# Code Review 30: Async Gallery, Cache Integrity, and Viewer Lifecycle

Date: 2026-07-12  
Reviewed HEAD: `6e35536` (`Gallery: Slow / Fast observer speed buttons (halve / double, clamped)`)  
Review mode: adversarial, findings-first, with focused runtime probes and current-HEAD regression suites

## Verdict

The far-wall rendering repair is correct and the new standalone layouts preserve their capacity and reachability invariants. The branch nevertheless has two high-severity Gallery Describe defects and a cluster of cache-consistency bugs that normal happy-path tests do not exercise.

There are sixteen confirmed findings:

| ID | Severity | Finding |
|---|---|---|
| F1 | HIGH | Async Gallery Describe has no dispatch-time ownership contract and can overwrite a newer human title. |
| F2 | HIGH | Describe's advertised 240-second budget does not bound the provider call, while the browser gives up at 180 seconds. |
| F3 | MEDIUM | A selected piece that disappears before the worker runs is reported as successfully titled. |
| F4 | MEDIUM | Results exposes a false `await`: selection can fail while the required refresh is already in flight. |
| F5 | MEDIUM | A superseded Favorites request mutates shared cache state before its generation check. |
| F6 | MEDIUM | A failed Favorites refresh clears valid cached content and is logged as successful. |
| F7 | MEDIUM | A confirmed missing Favorite is not persisted, so the stale snapshot resurrects on the next cached load. |
| F8 | MEDIUM | Duplicate Favorite creation returns a fabricated row rather than the authoritative stored row. |
| F9 | MEDIUM | Results catalog reconciliation parallelizes S3 reads but serializes every DDB write in the request path. |
| F10 | MEDIUM | Results catalog rows have no schema/source version and are trusted indefinitely. |
| F11 | MEDIUM | Forced Favorites hydration nests one thread pool per artifact inside an outer thread pool. |
| F12 | LOW | An empty DeepZoom inventory never becomes session-cached. |
| F13 | MEDIUM | The predeploy browser gate omits load-bearing Results, Favorites, DeepZoom, and Compute suites. |
| F14 | LOW | The wall-edge control still claims pixel units after the implementation changed to world units. |
| F15 | LOW | Photographic sky textures are outside both the texture budget and the explicit disposal path. |
| F16 | LOW | A stale speed-clamp timer can make the readout disagree with the actual observer speed. |

F1 and F2 need to be fixed before treating async Describe as concurrency-safe or operationally bounded. F4-F11 are not speculative style concerns: each is a concrete stale-state, false-success, or request-scaling failure. F12-F16 are smaller, but all have direct fixes and regression tests.

## Review Scope and Evidence

The review covered the current feature wave and its shared dependencies:

- `js/01-core-compute.js`
- `js/11-artifacts.js`
- `js/15-gallery.js`
- `gallery/app.js`
- `gallery/layout.js`
- `gallery/manifest.js`
- `gallery/texture-manager.js`
- `lambda/handler_storage.py`
- `lambda/book_describe.py`
- `lambda/shared.py`
- `deploy.sh`
- `scripts/predeploy_check.sh`
- backend, frontend-harness, and Playwright tests for Gallery, Favorites, Results, and DeepZoom

The following current-HEAD suites pass:

```text
81 passed
  tests/test_results_catalog.py
  tests/test_gallery_backend.py
  tests/test_favorites_storage.py

Frontend fused render source checks: OK
Frontend generated editor help runtime checks: OK
Frontend results lazy preview runtime checks: OK
Frontend jobs rail runtime checks: OK
Frontend scrub pad runtime checks: OK
Frontend fused render runtime checks: OK
Frontend sequential load checks: OK (24 scripts)
Frontend artifact mosaic runtime checks: OK
Frontend results refresh popup runtime checks: OK
Frontend coeff program modal filter/sort checks: OK
Frontend book row label checks: OK
Frontend stored-metadata escaping checks: OK
Frontend mosaic viewer URL validation checks: OK
Frontend lambdaPost mutation classifier checks: OK

92 passed
  tests/e2e/gallery-logic.spec.js
  tests/e2e/gallery-tab.spec.js
  tests/e2e/gallery-viewer-smoke.spec.js
  tests/e2e/favorites-ui.spec.js
  tests/e2e/results-ui.spec.js
  tests/e2e/deepzoom-inventory.spec.js
```

The green suites do not contradict the findings. They mostly serialize requests and return immediately resolved mocks. They do not reorder concurrent promises, simulate an async Lambda retry after a CAS conflict, make a provider call outlive the nominal worker deadline, or perform thousands of non-zero-latency DDB writes.

Three focused probes were also used:

1. An explicit Describe target absent from the current gallery produced `described=0`, `errors=[]`, followed by task status `done`.
2. A speed-clamp callback was scheduled at `8x`, followed immediately by a valid reduction to `4x`; the delayed callback changed the label back to `8x` while the actual speed remained `4x`.
3. Every standalone layout at 5%, 35%, and 100% coverage placed all 64 viewer-cap pieces; no capacity defect was found in the new sparse layouts.

## F1 - HIGH - Async Describe Can Overwrite a Newer Human Title

### Affected code

- `js/15-gallery.js:406-424`
- `lambda/handler_storage.py:5280-5308`
- `lambda/handler_storage.py:5343-5368`
- `lambda/handler_storage.py:5395-5403`
- `deploy.sh:482-505`

### Current behavior

The browser dispatches only gallery and artifact identity:

```js
const resp = await lambdaPost('storage', {
    gallery_id: gid,
    overwrite: true,
    pieces: [{
        job_id: p.job_id,
        family: p.family || 'color',
        artifact_id: p.artifact_id,
    }],
}, '/describe-gallery', { idempotent: false });
```

It does not send:

- the gallery revision the user reviewed;
- the title observed at dispatch time;
- an operation owner/token that must still own the title field;
- a condition saying that the title may be changed only while it retains that base value.

The route copies those parameters into an asynchronous self-invoke. The worker later reads whichever gallery revision is current at execution time:

```python
doc, revision = _read_gallery_doc_with_etag(gallery_id)
...
p["title"] = _clean_gallery_title(title)
revision = _put_gallery_doc(doc, expected_revision=revision)
```

The CAS protects the complete document between the worker's read and write. It does **not** protect the user's dispatch-time intent. If another client changed the title before the worker read the document, the worker reads the new title and replaces it without a conflict because `overwrite=True`.

### Failure timeline A: edit lands before worker read

1. Browser A reads revision `R1`, whose selected piece title is `"Old"`.
2. Browser A clicks Describe. The request carries no `R1` and no base title.
3. Browser B saves the human title `"Chosen by curator"`, creating revision `R2`.
4. The async worker starts and reads `R2`.
5. Vision returns `"Night Lattice"`.
6. The worker conditionally writes against `R2`; the write succeeds.
7. The human title is silently replaced by `"Night Lattice"`.

The gallery-level CAS is green throughout this sequence. It protects only against changes after step 4.

### Failure timeline B: retry defeats a conflict

1. The worker reads `R1` and begins the vision call.
2. A user saves a title, creating `R2`.
3. The worker's write against `R1` conflicts.
4. `_describe_gallery_run` rethrows `GalleryConflictError`.
5. `_run_describe_gallery_worker` records task `error` and rethrows again.
6. A clean deployment has no event-invoke policy for the Storage Lambda in `configure_async_invoke_policies()`. Its self-invoked event therefore retains the platform's default async retry behavior unless someone changed live infrastructure manually.
7. The retry rereads `R2`, regenerates a title, and writes against `R2` successfully.
8. The task row can change from `error` to `done` after the browser has stopped polling it.

This is the same ownership class already handled correctly by Book Describe: generated content must merge only if the human-owned field still matches the base value from dispatch.

### Impact

- A human title can be overwritten after the Describe button was clicked.
- A UI-visible conflict/error can later turn into a successful write because of an invisible async retry.
- The vision provider can be charged twice for one click.
- The task row is not monotonic and does not communicate which attempt owns the result.

### Required fix

The dispatch payload must carry an immutable operation contract:

```json
{
  "gallery_id": "gallery_x",
  "expected_revision": "etag-r1",
  "pieces": [{
    "job_id": "compute_x",
    "family": "color",
    "artifact_id": "color_x",
    "base_title": "Old"
  }]
}
```

The worker should generate from that snapshot and, immediately before publish, reread the current document and merge only if the selected piece still exists and its title still equals `base_title`. A moved gallery revision is acceptable only when the target field itself is unchanged. A changed title is a human edit and must win.

The Storage Lambda's async invoke policy must also be explicit. Either set retries to zero or make the operation owner-conditioned and idempotent under retries. Doing only one of those is weaker than doing both.

### Acceptance tests

- Dispatch at `R1`, save a new title at `R2` before worker start, run the worker, and assert that the `R2` title survives.
- Dispatch at `R1`, make the first worker CAS conflict, replay the same event, and assert that the newer title still survives.
- Assert that a replay cannot move a task from terminal `error` to `done` unless it owns the same operation state.
- Assert that editing an unrelated gallery field while leaving the title unchanged still permits a field-level merge.

## F2 - HIGH - Describe's Time Budget Does Not Bound Its Work

### Affected code

- `lambda/handler_storage.py:4608`
- `lambda/handler_storage.py:5380-5395`
- `lambda/book_describe.py:144-190`
- `lambda/book_describe.py:213-220`
- `js/15-gallery.js:428-435`
- `lambda/shared.py:32-47`

### Current behavior

The Gallery worker declares:

```python
DESCRIBE_TIME_BUDGET_S = 240.0
```

but tests the deadline only before beginning each piece:

```python
if time.time() >= deadline:
    ...
title = _gallery_title_from_reply(_vision_call(model, api_key, img, prompt))
```

The call does not use the bounded interactive profile. `_vision_call(..., interactive=False)` delegates to a provider profile whose Gemini path permits:

```python
attempts = 8
timeout = 120.0
max_retry_wait = 90.0
```

One provider call can therefore run far beyond 240 seconds. The deadline is a loop-entry hint, not an execution deadline.

The nearby `targets = targets[:4]` comment still says `sync route — stay well inside the API gateway budget`, even though the implementation was moved to a self-invoked asynchronous worker specifically because it could not stay inside that budget. That stale comment is not the bug, but it accurately exposes that the timing contract was only partly migrated.

The browser imposes a different budget:

```js
if (performance.now() - t0 > 180000) {
    throw new Error('describe timed out — check the jobs rail later');
}
```

The client gives up at three minutes, one minute before the backend's nominal budget, and potentially many minutes before `_vision_call` exits.

### Exact failure modes

**Provider remains slow:** the browser marks the rail item failed at 180 seconds, but the worker continues and may write a title later.

**Lambda reaches its hard timeout:** process termination bypasses `_run_describe_gallery_worker`'s catch block, so no terminal status is written. The DDB row remains `started` until its 24-hour TTL.

**Async retry follows timeout:** the same expensive provider call can begin again with the same unowned payload.

**Browser message is misleading:** “check the jobs rail later” implies continuing observation, but the catch path stops polling. The existing rail card is not backed by a persistent background reconciler for this task.

### Why the existing time-budget test is insufficient

`test_describe_stops_at_time_budget` sets the budget to `-1` and proves only that the worker does not start a call after an already-expired deadline. It does not prove that an in-progress call respects the deadline.

### Required fix

Pass a real remaining-time budget into the provider layer. The provider call must derive per-attempt timeout, retry count, and maximum wait from the remaining deadline and must stop with enough margin to write terminal status.

A practical contract is:

```text
worker hard timeout       900s
operation deadline        e.g. 240s
status-write reserve      e.g. 10s
provider attempt timeout  min(configured timeout, remaining - reserve)
retry sleep               never beyond remaining - reserve
browser poll timeout      operation deadline + dispatch/status margin
```

The client should either poll through the bounded backend deadline or hand the task to a persistent jobs-rail poller. It must not label a still-running operation failed and then stop observing it.

### Acceptance tests

- Use a fake provider that blocks beyond the remaining deadline and assert that the worker exits before the configured operation deadline plus a small margin.
- Assert that every worker exit path writes exactly one terminal task state.
- Assert that the browser continues polling until the backend's documented maximum duration.
- Assert that Lambda timeout/retry configuration for `polypaint-storage` is explicitly deployed and tested.

## F3 - MEDIUM - A Missing Describe Target Is Reported as Success

### Affected code

- `lambda/handler_storage.py:5353-5368`
- `lambda/handler_storage.py:5327-5332`
- `js/15-gallery.js:438-449`

### Current behavior

For an explicit target list, the worker intersects requested identities with the current pieces:

```python
targets = [p for p in pieces if key3(p) in want]
...
if not targets:
    return {"gallery": doc, "revision": revision,
            "described": 0, "errors": []}
```

The wrapper interprets `described=0` plus an empty error list as success:

```python
if not result.get("described") and errors:
    report_status(..., "error", ...)
else:
    report_status(..., "done")
```

The browser then refetches, cannot find the piece, but still reports success:

```js
rail({ state: 'done', detail: titled && titled.title ? ... : 'titled' });
_galleryBtnFlash('btn-gallery-describe', '✓ Titled');
_galleryStatus('Titled: “' + ((titled && titled.title) || '?') + '”');
```

### Reproduced example

Given a gallery that no longer contains the requested `(job_id, family, artifact_id)`, the worker returns:

```text
described = 0
errors = []
task_status = done
```

The visible result is `Titled: “?”`, which is a false success rather than a useful concurrency error.

### Required fix

An explicitly requested identity matching zero current pieces must produce a coded terminal error such as `gallery_piece_missing`. The task result should store the requested identity and `described` count. The frontend should verify that the refetched target exists and has the returned title before showing a green state.

An empty target list is only a valid no-op when the caller intentionally requested “all untitled pieces” and none remain. Explicit selection and bulk no-op are different contracts.

### Acceptance tests

- Delete the selected piece after dispatch and assert terminal task `error`.
- Keep the piece but replace its identity and assert terminal `error`.
- Assert that the frontend never displays `✓ Titled` when the returned target cannot be found.

## F4 - MEDIUM - Results Refresh Is Not Actually Awaitable

### Affected code

- `js/01-core-compute.js:237-284`
- `js/01-core-compute.js:296-303`

### Current behavior

`loadResults()` uses a boolean as its concurrency primitive:

```js
async function loadResults(options = null) {
    if (_resultsLoading) return;
    ...
}
```

`_ensureResultsSelection()` assumes that awaiting it means the cache has reached a post-refresh state:

```js
if (!hasJob) await loadResults({ force: true });
const found = _resultsCache.some(r => r.job_id === jobId);
if (!found) throw new Error(`Result ${jobId} not found`);
```

When a refresh is already active, `loadResults()` returns an already-resolved promise containing `undefined`. The caller immediately checks the old cache.

### Failure timeline

1. Results cache does not contain `compute_new`.
2. The user starts Refresh; `/list` is in flight.
3. Before it resolves, Gallery, Favorites, AllCol, or AllPal invokes `_ensureResultsSelection('compute_new')`.
4. `_ensureResultsSelection` calls `await loadResults({force:true})`.
5. `_resultsLoading` is true, so `loadResults` returns immediately.
6. `_ensureResultsSelection` checks the still-old cache and throws `Result compute_new not found`.
7. The original request resolves milliseconds later and adds the job.

The same function also swallows refresh errors. A network error is converted into “Result X not found,” hiding the real cause from navigation callers.

### Required fix

Replace `_resultsLoading` as the ownership primitive with an actual `_resultsLoadPromise`. Equivalent callers should await the active promise. Force/rebuild requests need an explicit policy: either join the active request if it is sufficient or schedule one follow-up request after it settles.

The data loader should reject on request failure. UI entry points can catch and display the error; internal selection logic must not receive a false successful completion.

### Acceptance tests

- Hold `/list` unresolved, call `_ensureResultsSelection()` during it, then resolve with the target row and assert selection succeeds.
- Repeat with a rejected `/list` and assert the network error survives instead of becoming “not found.”
- Assert that two non-forced callers produce one request and both await the same promise.

## F5 - MEDIUM - Superseded Favorites Loads Mutate Shared State

### Affected code

- `js/01-core-compute.js:347-353`
- `js/01-core-compute.js:577-610`

### Current behavior

The request helper mutates global cache state before the caller's generation guard:

```js
async function _loadFavoriteRefs(force = false) {
    ...
    const resp = await lambdaPost(...);
    _favoriteRefs = Array.isArray(resp.favorites) ? resp.favorites : [];
    _favoriteRefsLoaded = true;
    return _favoriteRefs;
}
```

The generation check happens later:

```js
await _loadFavoriteRefs(force);
if (generation !== _favoriteLoadGeneration) return _favoriteArtifacts;
```

Therefore the guard protects rendering but not the authoritative cache mutation. The shared promise has a second race:

```js
_favoriteLoadPromise = (...).finally(() => {
    _favoriteLoadPromise = null;
});
```

Every completion clears the slot, even when the slot now belongs to a newer forced request.

### Exact stale-write timeline

1. Non-forced request A starts at generation 0.
2. Forced Refresh B increments generation to 1 and replaces `_favoriteLoadPromise`.
3. B returns first, stores fresh refs, rebuilds the panel, and displays them.
4. A returns later and overwrites `_favoriteRefs` with its older response.
5. A notices its generation is stale and skips rendering, but the cache is already stale.
6. A's `finally` sets `_favoriteLoadPromise = null`, regardless of whether B still owns that variable.
7. `_isFavorite()` now reads stale refs. A later tab load sees the row-signature mismatch, but `_loadFavoriteRefs(false)` trusts `_favoriteRefsLoaded` and returns the stale refs without a request, allowing the stale panel to be rebuilt.

### Required fix

Request functions must return data without touching shared state:

```js
async function _fetchFavoriteRefs(force) {
    const resp = await lambdaPost(...);
    return Array.isArray(resp.favorites) ? resp.favorites : [];
}
```

Only the current generation may commit `_favoriteRefs`, `_favoriteArtifacts`, signatures, selection, and DOM. Promise cleanup must be owner-checked:

```js
const promise = ...;
_favoriteLoadPromise = promise;
try {
    return await promise;
} finally {
    if (_favoriteLoadPromise === promise) _favoriteLoadPromise = null;
}
```

### Acceptance tests

- Start A, start forced B, resolve B, then resolve A; assert refs and panel remain B.
- Resolve A while B is still active and assert A cannot clear B's promise slot.
- Assert `_isFavorite()` sees only the winning generation.

## F6 - MEDIUM - Failed Favorites Refresh Clears Good Data and Logs Success

### Affected code

- `js/01-core-compute.js:599-606`
- `js/01-core-compute.js:613-624`
- `js/11-artifacts.js:1538-1547` as the existing correct stale-good pattern

### Current behavior

On a current-generation failure, Favorites does this:

```js
_favoriteArtifactsReady = false;
_favoriteArtifacts = [];
_favoriteSelectedIdx = -1;
_favoriteSelectedKey = '';
renderFavoritesPanel();
```

A transient API error is therefore rendered as an empty collection even when a valid list was visible immediately before Refresh.

The catch block does not rethrow or return an error result. Consequently this outer code always executes its success branch:

```js
await loadFavoritesInventory({ force: true });
log('Favorites refreshed', 'ok', 'favorites-log');
```

Its `catch` is unreachable for `/list-favorites` failures already swallowed by `loadFavoritesInventory()`.

### User-visible example

1. Favorites displays 100 valid rows.
2. Refresh receives a transient 503.
3. The table becomes empty and selection disappears.
4. The status says refresh failed.
5. The log simultaneously says `Favorites refreshed` in success styling.

DeepZoom already follows the correct rule: retain the live table and report “Refresh failed ... showing cached list.” Favorites should use the same contract.

### Required fix

Preserve `_favoriteRefs`, `_favoriteArtifacts`, selection, and scroll on refresh failure. Mark the list stale in the status line. Return `{ok:false,error}` or rethrow so `refreshFavoritesInventory()` logs the real outcome.

Only a successful authoritative empty response may clear the list.

### Acceptance tests

- Load two favorites, reject a forced refresh, and assert both rows and selection remain.
- Assert the log contains `Favorites refresh failed` and never `Favorites refreshed` for that operation.
- Resolve a forced refresh with `favorites: []` and assert the panel clears normally.

## F7 - MEDIUM - Missing Favorite State Is Not Persisted

### Affected code

- `lambda/handler_storage.py:1635-1642`
- `lambda/handler_storage.py:1665-1704`
- `tests/test_favorites_storage.py:191-211`

### Current behavior

Snapshot rows are trusted without S3 work on a normal list. A forced refresh correctly re-resolves all rows and can classify one as `missing`:

```python
if state == "ready" and snap is not None:
    backfills.append(...)
elif state == "missing":
    diag["missing"] += 1
```

Only `ready` results are persisted. The old ready snapshot remains unchanged in DDB.

### Resurrection timeline

1. Favorite A has a valid ready snapshot.
2. Its underlying artifact is deleted.
3. Forced Refresh HEADs the exact keys and returns A as `missing`.
4. No DDB update removes or invalidates A's old snapshot.
5. The user reloads the app or opens a fresh browser session.
6. Normal `/list-favorites` sees the current snapshot version and performs zero S3 checks.
7. A returns as `ready` again, with stale image and preview keys.

The current test proves only step 3. It does not call a subsequent non-refresh list and therefore misses the resurrection.

### Required fix

Persist authoritative missing state. Options include:

- removing the snapshot and storing `hydration_state=missing` with a checked timestamp;
- storing a versioned missing snapshot that the normal path decodes as missing;
- deleting the Favorite automatically, only if that product behavior is explicitly desired.

Transient/throttle/access errors must never persist as missing. The existing `missing` versus `error` distinction should remain.

### Acceptance tests

- Seed a ready snapshot, delete the artifact, force refresh, then perform a normal list; assert it remains missing.
- Repeat with `SlowDown` and assert the old ready snapshot is preserved rather than converted to missing.

## F8 - MEDIUM - Duplicate Favorite Returns a Fabricated Row

### Affected code

- `lambda/handler_storage.py:1308-1336`
- `lambda/handler_storage.py:1736-1748`
- `js/01-core-compute.js:377-393`
- `tests/test_favorites_storage.py:257-263`

### Current behavior

The conditional put returns only a boolean:

```python
if code == "ConditionalCheckFailedException":
    return False
```

On that miss, `handle_add_favorite()` still returns a panel row made from the newly constructed, unstored `entry`:

```python
entry = {
    "added_at": now,
    "display_name": caller_value,
    ...
}
added = _put_favorite_entry(entry, snapshot=snapshot)
return ok_response({
    "added": added,
    "favorite": _favorite_panel_row(entry, snapshot, "ready", ""),
})
```

The real DDB row keeps its original `added_at` and `display_name`. The response claims current time and the latest caller-provided display name.

The frontend write-through logic trusts the response and prepends/replaces its local row. An already-favorited artifact can therefore appear newly added, move to the top, and temporarily change its display name until a full refresh.

### Required fix

On conditional failure, read the existing row and return that authoritative row. Since duplicates are exceptional and scoped to one known key, one `GetItem` is sufficient. Alternatively, return identity plus `added:false` and make the frontend leave an existing/unknown cache untouched.

### Acceptance tests

- Add a favorite at time T1 with display name A.
- Add it again at T2 with display name B.
- Assert the duplicate response contains T1 and A, exactly matching DDB.
- Assert frontend write-through does not reorder or rename the row after `added:false`.

## F9 - MEDIUM - Results Catalog Writes Are Serial in the API Request

### Affected code

- `lambda/handler_storage.py:2885-2891`
- `lambda/handler_storage.py:2893-2908`
- `lambda/handler_storage.py:2919-2925`
- `lambda/handler_storage.py:2937-2945`

### Current behavior

The expensive S3 `calc.json` reads are correctly parallelized. The result fold then performs one synchronous `PutItem` per reconciled job:

```python
for entry in fresh:
    ...
    _results_catalog_put(_results_catalog_item(entry))
```

Pruning similarly performs one synchronous `DeleteItem` at a time:

```python
for jid in list(rows):
    if jid not in present:
        _results_catalog_delete(jid)
```

This occurs inside the `/list` request. The first catalog build or `rebuild=true` can therefore execute thousands of sequential network round trips after the parallel S3 phase has completed.

### Scaling example

With 2,000 jobs and only 10 ms average DDB write latency:

```text
2,000 * 10 ms = 20 seconds
```

That excludes prefix listing, DDB query, S3 reads, JSON work, and response transfer. At 15 ms, DDB writes alone consume 30 seconds. The existing fake-DDB tests have effectively zero latency and one or two rows, so they cannot expose this boundary.

The response reports `prefix_list_us`, `catalog_read_us`, and `calc_fetch_us`, but no catalog-write/prune timing. A slow request therefore appears to have unexplained time outside the displayed breakdown.

### Required fix

Use `BatchWriteItem` in bounded chunks, retry `UnprocessedItems`, and keep cache writes best-effort. The deployed policy already includes batch-write capability. A bounded writer pool is a second-best option if preserving independent writes is important.

Report at least:

```text
catalog_write_us
catalog_writes_attempted
catalog_writes_failed
catalog_prune_us
```

Longer term, canonical compute-finalization writers can make reconciliation an exception rather than the normal first-build path, but the list endpoint still needs a bounded repair mechanism.

### Acceptance tests

- Rebuild 2,000 fake rows with injected 10 ms write latency and assert elapsed time is bounded by batching rather than N times latency.
- Exercise `UnprocessedItems` and assert bounded retries.
- Assert timing fields explain the complete catalog portion of the request.

## F10 - MEDIUM - Results Catalog Has No Schema or Source Version

### Affected code

- `lambda/handler_storage.py:2714-2729`
- `lambda/handler_storage.py:2740-2766`
- `lambda/handler_storage.py:2810-2820`
- `lambda/handler_storage.py:2852-2859`
- `lambda/handler_storage.py:5891-5907`

### Current behavior

Catalog items contain derived fields and `cached_at_ms`, but no schema version and no identity for the `calc.json` object from which they were derived:

```python
item = {
    "job_id": ...,
    "task_id": ...,
    "result_job_id": ...,
    "cached_at_ms": ...,
}
```

For a normal row, `needs_read()` returns false forever:

```python
if item is None or rebuild:
    return True
if "no_calc" in item:
    ...
return False
```

The `handle_save_metadata` and delete hooks correctly update/remove catalog rows for those API paths. That is a positive. It does not solve two remaining invalidation classes:

1. A future code release changes `_results_entry_fields()` or adds a column; every existing row remains in the old implicit schema and missing values decode as zero.
2. Direct S3 repair or administrative replacement of `calc.json` leaves the catalog stale until someone knows to request `rebuild=true`.

### Impact

- Schema migrations silently display zeros or old derivations.
- New Results fields cannot safely rely on automatic self-healing.
- The cache claims to reconcile membership but cannot reconcile changed content.
- Operators need hidden knowledge of the rebuild escape hatch after manual repair.

### Required fix

Add `catalog_schema_version` to each row and make version mismatch imply `needs_read=True`. Increment it whenever `_results_entry_fields()` or the stored representation changes.

For content freshness, store the source `calc.json` ETag/version when reading it. Whether `/list` should HEAD every job is a performance decision; it does not need to. The ETag still lets explicit repair tools and writer hooks update atomically and gives diagnostics a way to prove which source was cached.

### Acceptance tests

- Seed a version-N row, run with version N+1, and assert it is automatically rebuilt.
- Seed an old row lacking the version and assert it is treated as stale, not current.
- Assert save-metadata and delete hooks continue to update the versioned catalog.

## F11 - MEDIUM - Forced Favorites Hydration Nests Thread Pools

### Affected code

- `lambda/handler_storage.py:1365-1383`
- `lambda/handler_storage.py:1665-1679`
- `lambda/handler_storage.py:6133-6180`

### Current behavior

Forced refresh creates an outer executor with up to 16 workers. Each worker resolves one favorite and calls `_head_artifact_keys()`. That helper creates a new executor for the candidate image and preview keys.

The shape is therefore:

```text
outer pool: up to 16 favorite workers
  favorite 1 -> create inner HEAD pool
  favorite 2 -> create inner HEAD pool
  ...
  favorite 16 -> create inner HEAD pool
```

This is not simply 16 bounded workers. It is 16 concurrently constructed executors, each with several threads, plus their creation and shutdown overhead. The S3 client connection pool is shared, so excess threads can spend their time waiting for the same bounded socket pool.

The steady snapshot-hit path avoids this work, but explicit Refresh and legacy backfill are exactly the paths where latency feedback matters.

### Required fix

Flatten the work into `(favorite_identity, candidate_key)` tasks and run one bounded executor, then group HEAD results by favorite. Overlay reads can run in the same bounded pool after the image candidate is known, or in a second bounded phase.

If `_head_artifact_keys` must retain internal parallelism for other callers, add an option to execute serially inside an already-parallel caller or accept a shared executor.

### Acceptance tests

- Instrument executor construction during a 100-row forced refresh and assert a bounded fixed count, not one inner pool per row.
- Assert maximum simultaneous HEAD calls respects the configured worker limit.
- Preserve the existing exact-key and no-listing guarantees.

## F12 - LOW - Empty DeepZoom Inventory Is Never Cached

### Affected code

- `js/11-artifacts.js:1484-1496`
- `js/11-artifacts.js:1511-1533`

### Current behavior

The non-empty path sets:

```js
_dzInventoryLoaded = true;
```

The empty path clears the viewer, renders “No DeepZoom exports yet,” and returns before setting that flag.

Consequently every re-entry to an empty DeepZoom tab calls `/list-deepzoom` again. Empty is a valid loaded result, not an uninitialized state.

### Required fix

Set `_dzInventoryLoaded = true` on every successful response, including zero exports. An explicit Refresh or a successful new export remains responsible for invalidating/patching the cache.

### Acceptance test

Return `exports: []`, enter the tab twice, and assert exactly one `/list-deepzoom` request.

## F13 - MEDIUM - Browser Gate Coverage Does Not Match the Changed Product Surface

### Affected code

- `scripts/predeploy_check.sh:134-142`
- `tests/test_predeploy_gate_completeness.py:129-153`
- `tests/e2e/results-ui.spec.js`
- `tests/e2e/favorites-ui.spec.js`
- `tests/e2e/deepzoom-inventory.spec.js`
- `tests/e2e/compute-ui.spec.js`

### Current behavior

Predeploy runs only Gallery Playwright suites:

```sh
npx playwright test \
    tests/e2e/gallery-logic.spec.js \
    tests/e2e/gallery-tab.spec.js \
    tests/e2e/gallery-viewer-smoke.spec.js \
    tests/e2e/gallery-texture-manager.spec.js \
    tests/e2e/gallery-curation.spec.js
```

The feature wave also changed Results session caching/catalog behavior and DeepZoom inventory caching, while Favorites shares the same navigation and stale-good-cache contracts. Their Playwright suites are not in the deployment gate.

The gate-completeness test cannot catch this because its parser and filesystem scan consider only `tests/test_*.py`. It has no equivalent accounting for `tests/e2e/*.spec.js`.

### Why this matters

All 92 focused browser tests pass when run manually. A deployment can still skip the Results, Favorites, and DeepZoom subset entirely. Passing predeploy therefore does not mean the changed browser contracts ran.

### Required fix

Maintain an explicit required E2E allowlist and add a meta-test that accounts for every `tests/e2e/*.spec.js` as either gated or deliberately excluded with a reason. At minimum, gate:

```text
gallery-logic.spec.js
gallery-tab.spec.js
gallery-viewer-smoke.spec.js
gallery-texture-manager.spec.js
gallery-curation.spec.js
favorites-ui.spec.js
results-ui.spec.js
deepzoom-inventory.spec.js
compute-ui.spec.js
```

If total runtime becomes unacceptable, split fast contract E2E from slower visual/integration E2E, but do not silently omit the suites.

## F14 - LOW - Wall Edge Control Claims the Wrong Units

### Affected code

- `index.html:2292-2293`
- `gallery/app.js:314-345`
- `gallery/manifest.js:34,47-48`
- `lambda/handler_storage.py:5019-5028`

### Current behavior

The UI says:

```text
Thickness ... in pixels
Edge lines [1] px
```

The field is named `wall_edge_px`. The flicker fix correctly changed rendering to:

```js
linewidth: widthPx * 0.004,
worldUnits: true,
```

The configured number is now a world-space edge-weight multiplier, not a screen-pixel width. Its apparent pixel size varies with perspective and distance; that variance is the feature that fixed the far-wall shimmer.

### Required fix

Rename the product setting and schema to world-space semantics, for example `wall_edge_weight` or `wall_edge_width_m`. If a schema migration is undesirable during development, at least correct the label, tooltip, variable names, and stale “real pixel width” comment.

The rendering behavior itself should remain world-unit based.

## F15 - LOW - Photographic Sky Textures Bypass Budget and Disposal

### Affected code

- `gallery/app.js:249-252`
- `gallery/app.js:401-409`
- `gallery/app.js:1020-1050`
- `gallery/texture-manager.js:18-24`
- `gallery/texture-manager.js:225-251`

### Current behavior

The artwork texture manager enforces 48 resident textures and 128 MiB. A photographic sky is loaded independently:

```js
const tex = new THREE.TextureLoader().load(...);
this.scene.background = tex;
```

It is not registered with the texture manager and is not stored in a dedicated field. `destroy()` disposes `_skyMats`, floor texture, labels, walls, and the renderer, but never explicitly disposes `scene.background`.

The current photographic sky files are 4096x2048. An uncompressed RGBA GPU allocation is approximately:

```text
4096 * 2048 * 4 = 33,554,432 bytes (32 MiB)
```

That is material relative to the advertised 128 MiB art-texture budget, yet it is absent from `tm.stats()` and the debug readout.

Renderer/context teardown usually releases it at page exit, so this is not an immediate leak on a one-shot load. It is still an ownership gap for viewer rebuilds, context restoration, and future live sky switching.

### Required fix

Store the texture as `_skyTexture`, dispose the previous one before replacement, and dispose it in `destroy()`. Account for fixed scene textures in either the same memory budget or an explicit separate scene-resource budget shown in diagnostics.

### Acceptance tests

- Build with a photographic sky, destroy, and assert the texture's `dispose()` runs once.
- Replace one photographic sky with another and assert the first is disposed.

## F16 - LOW - Speed Clamp Timer Can Overwrite a Later Speed Label

### Affected code

- `gallery/app.js:785-797`
- `gallery/app.js:1020-1050`
- `tests/e2e/gallery-viewer-smoke.spec.js:153-189`

### Current behavior

The clamp branch schedules a delayed cleanup:

```js
if (clamped) {
    el.textContent = label + (factor > 1 ? ' max' : ' min');
    clearTimeout(this._speedFlashT);
    this._speedFlashT = setTimeout(() => {
        el.textContent = label;
    }, 900);
} else {
    el.textContent = label;
}
```

Only another clamp clears the pending timer. A legitimate speed change does not.

### Reproduced timeline

```text
actual speed = 8x
click Fast  -> label "8x max", timer captures "8x"
click Slow  -> actual speed = 4x, label "4x"
900 ms      -> old timer writes label "8x"

final state:
  actual _speedMult = 4
  visible label      = 8x
```

The control still moves at 4x; only the readout lies. The existing test reaches a clamp but never changes direction before the timer fires. Its final “wired into both paths” assertion checks function source text with `.toString().includes(...)`, which is brittle and does not execute movement.

### Required fix

Clear and null `_speedFlashT` at the start of every `_speedAdjust()`, not only in the clamp branch. The callback should verify it still owns the current timer/state before writing. `destroy()` should clear the timer as normal lifecycle hygiene.

### Acceptance tests

- Clamp at max, immediately click Slow, advance fake time past 900 ms, and assert both state and label are `4x`.
- Repeat from min followed by Fast.
- Replace source-string wiring checks with behavioral movement/tour displacement checks under 1x and 2x.

## Confirmed Non-Findings

### Far-wall flicker repair is technically sound

The repair does three correct things together:

- `mergeWallRuns()` removes overlapping adjacent wall boxes and construction seams.
- wall accents use world-space widths, so distant edges shrink rather than remaining a dense fixed-pixel lattice;
- edge material tests depth but does not write it, with a small offset to avoid face-coincident depth seesaw.

The user confirmed that the visible jitter is gone. No regression was found in maze collision because collision remains grid-based rather than mesh-based.

### Standalone layouts retain viewer capacity

An executable probe built 64-piece layouts for `standalone`, `standalone2`, and `standalone4` at 5%, 35%, and 100% coverage. Every case placed all 64 pieces. The largest sparse grids remained within the 48-cell cap, and the existing browser tests cover reachability, detached panels, no perimeter, spacing ratios, and margins.

### Results writer hooks exist

`handle_save_metadata` upserts the catalog and `handle_delete` removes its row. F10 does not claim those hooks are absent. The remaining issue is schema/content invalidation outside those specific paths.

## Recommended Fix Order

1. Fix F1 and F2 together: dispatch-time ownership, field-level merge, real provider deadline, terminal task semantics, and explicit Storage async retry policy.
2. Fix F3 while changing the Describe task-result contract.
3. Fix F4-F6 as one frontend promise/cache discipline pass: loaders return data, promises are owner-checked, and stale-good UI survives failures.
4. Fix F7 and F8 as one Favorites backend consistency pass.
5. Fix F9-F11 as the catalog/hydration scalability pass, adding real latency and concurrency tests.
6. Gate the relevant browser suites under F13 before relying on those fixes during deployment.
7. Land F12 and F14-F16 as small, independently testable cleanup commits.

## Definition of Done

- Describe never overwrites a title changed after dispatch, including after event replay.
- Describe has one documented end-to-end deadline that bounds provider calls, Lambda work, client polling, and terminal status publication.
- Explicitly missing Describe targets fail rather than report `Titled: "?"`.
- Results and Favorites expose awaitable, owner-checked load promises.
- Superseded requests cannot mutate current caches or clear another request's promise slot.
- Refresh failures retain stale-good content and are logged as failures.
- Favorite missing/duplicate responses reflect authoritative persisted state.
- Results rebuild does not execute N sequential DDB writes, and its write cost is observable.
- Results catalog rows carry a schema version and automatically rebuild after a schema bump.
- Forced Favorites refresh uses one bounded concurrency budget.
- Empty DeepZoom inventory is a valid cached result.
- All load-bearing E2E suites are predeploy-gated or explicitly excluded with a reviewed reason.
- Wall-edge UI terminology matches world-unit rendering.
- Scene background textures have explicit ownership and disposal.
- Speed feedback timers cannot make the label disagree with `_speedMult`.
