# Favorites Tab Speedup

Reviewed commit: `5fe851ca2249ba52af36e750bdaa5ac5ffaca168`

Scope: Favorites tab activation, frontend caching and rendering, Favorites DynamoDB storage, `/render-summary`, S3 artifact discovery, mutation behavior, and relevant tests/deployment gates.

## Verification (traced against 5fe851c)

The load-bearing claims were traced to source and hold:

- **Root cause confirmed.** `switchTab('favorites')` calls `loadFavoritesInventory()` unconditionally (`js/01-core-compute.js:69`). It caches refs via `_favoriteRefsLoaded` but re-runs `_hydrateFavoriteArtifacts()` on *every* activation (`:575`), which issues one `/render-summary` per distinct job through `asyncPool(4, …)` (`:322-324`) and awaits them all before the first paint (`:330`). No in-flight/generation guard exists.
- **The per-job formula `31 + 4C + 3B + 2K + P + D` is exact** for a modern job. Re-derived: 13 top-level HEADs + `calc.json` GET + `deepzoom_latest.json` GET + 5 prefix listings + 11 legacy HEADs = 31 base; then color 3 HEAD + 1 overlay GET (`4C`), bilevel 3 HEAD (`3B`), coeffs 2 HEAD (`2K`), palette 1 meta GET (`P`), pdf 1 HEAD (`D`). The 11 legacy HEADs (`_legacy_render_variant`) re-HEAD keys already covered by the first 13 — `_head_artifact_keys` does not dedup across calls, so that duplication is real and eliminable.
- **`missing:true` conflation confirmed** at `js/01-core-compute.js:357` — any `/render-summary` failure sets `missing:true` with `missing_reason` = the transient error string.
- **Mutations reread + rehydrate confirmed.** `/add-favorite` and `/delete-favorite` both re-Query and return the whole partition (`handler_storage.py:1511-1513`, `:1521-1523`); the frontend then re-runs full hydration (`:381` add, `:738` delete).
- **Gate gap confirmed.** The only Favorites backend tests (`TestFavoritesStorage`) live in `tests/test_pipeline.py`, which is in `ALLOWED_UNGATED` — not run by predeploy. `tests/e2e/favorites-ui.spec.js` exists but exercises the *current* render-summary-based load (see Tests).

Two corrections to fold in below:

- DDB rows store `favorite_job_id` / `favorite_artifact_id` (plus a composite sort key `task_id = favorite#{job}#{artifact}` and a numeric `updated_at_ms`), **not** bare `job_id`/`artifact_id`. `image_key`, `preview_key`, `display_name` are optional (only present when the client sent them — and the client is trusted; see below).
- "1 DDB read" on first open is actually **two** DDB ops: a `__meta__` `GetItem` (`_favorite_store_initialized`) plus the partition `Query`. Proposal 5 is what collapses it to one.

One thing the current diagnosis understates: **`handle_add_favorite` performs no server-side validation at all** (`handler_storage.py:1507-1510`). It stores whatever `image_key`/`preview_key`/`display_name` the caller sends, unchecked. So the fast path today is *also* the unsafe path — Proposal 3's resolve-and-validate-on-Add is not only a speedup, it closes an identity-trust gap and is the natural place to apply `parse_render_key()` / `assert_render_identity()`.

## Recommendation

The delay is not primarily caused by reading the favorites list. The expensive step is `_hydrateFavoriteArtifacts()`: every time the Favorites tab is selected, it calls the full `/render-summary` route once for every distinct favorite job. `/render-summary` inventories every render family and every artifact for that job, although Favorites needs only one or a few known Color artifacts.

Implement this in two stages:

1. Make hydrated Favorites state cache-aware and idempotent. A repeat tab visit should render the in-memory list immediately and issue zero requests unless the user presses `Refresh` or the cache expires.
2. Stop using `/render-summary` for Favorites. Persist a compact, canonical display snapshot with each favorite and return it from `/list-favorites`. Hydrate old/incomplete rows by exact artifact key, never by scanning the whole job.

Expected steady-state behavior:

| Action | Current | Target |
|---|---:|---:|
| Reopen Favorites in one session | `J` `/render-summary` calls | 0 requests |
| First open with complete snapshots | 1 DDB read plus `J` summaries | 1 DDB query, 0 S3 calls |
| Explicit Refresh | 1 DDB read plus `J` summaries | 1 DDB query plus exact checks for `F` favorites |
| Add/Delete while Favorites is open | Mutation, DDB reread, full rehydration | Mutation only, local cache patch |

`F` is the number of favorites and `J` is the number of distinct jobs represented by those favorites.

## Current Path

### Frontend

`switchTab('favorites')` in `js/01-core-compute.js` calls `loadFavoritesInventory()` on every tab activation.

`loadFavoritesInventory()` does cache `_favoriteRefs`, but it does not cache their hydrated artifact rows:

1. `_loadFavoriteRefs()` calls `/list-favorites` only when `_favoriteRefsLoaded` is false.
2. `_hydrateFavoriteArtifacts()` still runs on every activation.
3. References are grouped by `job_id`.
4. One `/render-summary` request is issued per distinct job through `asyncPool(4, ...)`.
5. The function waits for all job summaries before rendering anything.
6. `renderFavoritesPanel()` then rebuilds the complete table and viewer with `innerHTML`.

The concurrency limit of four bounds browser request pressure, but it also means `J` jobs complete in at least `ceil(J / 4)` waves. More importantly, each request is doing far more backend work than Favorites needs.

Rapid repeated tab selection is also not deduplicated. There is no shared in-flight promise or request generation, so overlapping loads can repeat work and an older completion can overwrite newer state.

### Backend Favorites Read

`/list-favorites` is relatively cheap:

- `_favorite_store_initialized()` performs one DynamoDB `GetItem` on the `__meta__` marker.
- `_read_favorites_from_ddb()` performs a paginated DynamoDB `Query` on `job_id = favorites#color` (no `ProjectionExpression`, so every attribute is returned).
- Rows already contain `favorite_job_id`, `favorite_artifact_id`, `added_at`, `updated_at_ms`, and (when the client supplied them) `display_name`, `image_key`, `preview_key`. The sort key is `task_id = favorite#{job}#{artifact}`, and Add dedups on it with a conditional put (`attribute_not_exists`).

The extra initialization `GetItem` can be removed later, but it is not the dominant delay.

### Backend Render Summary Amplification

For each distinct favorite job, `handle_render_summary()` performs a complete Render-tab refresh:

- 13 top-level legacy `HEAD` requests.
- One `GET` for `calc.json`.
- One `GET` for `deepzoom_latest.json`.
- Five prefix listings: Color, BiLevel, Coeffs, Palette, and PDF.
- 11 more legacy `HEAD` requests, duplicating legacy keys already checked in the first batch.
- Per immutable Color artifact: three `HEAD` requests plus one overlay `GET` attempt.
- Per BiLevel artifact: three `HEAD` requests.
- Per Coeffs artifact: two `HEAD` requests.
- Per Palette artifact: one metadata `GET`.
- Per PDF artifact: one `HEAD` request.
- Presigned URLs for every discovered artifact, even though Favorites displays only the selected favorite preview.

The approximate S3 request count per favorite job is therefore:

```text
31 + 4*C + 3*B + 2*K + P + D
```

`C`, `B`, `K`, `P`, and `D` are the job's Color, BiLevel, Coeffs, Palette, and PDF artifact counts. This excludes SDK retries and response transfer size.

**Worked example.** A favorites set of `F = 24` across `J = 12` distinct jobs, each job carrying ~5 Color + ~2 Palette artifacts, costs per tab open ≈ `12 × (31 + 4·5 + 2) = 12 × 53 ≈ 640` S3 operations — to display 24 known Color previews. The target path is **one DDB query and zero S3 operations**. The waste is ~640:0, and it recurs on every tab entry and after every mutation.

Two clarifications on what the formula does and does not count:

- **Presigns are local, not S3 round-trips.** `_head_artifact_keys(presign=True)` and `_palette_entry_from_prefix` call `generate_presigned_url`, which is a pure client-side signing operation — it does not appear in the request count above. It still hurts: it bloats the response, burns CPU per row, and every URL carries a one-hour expiry (the cache-poisoning problem in "Required Behavior"). So presigning every discovered artifact is wasteful even though it is not a network op — reason enough to prefer stable public URLs.
- **The 11 legacy HEADs are pure duplication.** They re-HEAD `image.jpeg`, `image.png`, `image_bilevel.tif`, etc. — the exact keys already HEADed in the first batch of 13 — because `_head_artifact_keys` holds no cross-call cache. For a modern job with no legacy top-level objects, all 24 of those HEADs (13 + 11) return 404 and exist only for backward compatibility.

The family inventories are evaluated sequentially in `handle_render_summary()`. Individual artifact checks use nested thread pools, which reduces some wall time but adds thread and connection overhead. Browser concurrency cannot remove this backend amplification.

### Mutations Repeat The Same Work

The same expensive hydration is repeated after mutations:

- `/add-favorite` writes one DDB item, rereads the whole favorites partition, and the active Favorites tab calls `loadFavoritesInventory()` again.
- `/delete-favorite` deletes one DDB item, rereads the whole favorites partition, then the frontend calls `_hydrateFavoriteArtifacts()` for every remaining favorite.

Both operations already know the exact local change. Neither needs a complete reread and rehydration to update the current browser.

## Required Behavior To Preserve

- Favorites remain a global backend-backed list, not a `localStorage` feature.
- Dedupe remains `(job_id, artifact_id)`.
- Newest-added-first ordering remains stable.
- A deleted artifact remains visible as a stale favorite until the user removes the reference.
- A transient S3/API error must not be mislabeled as `missing artifact`; only an authoritative not-found result means missing. This is already a solved problem in the codebase: use `shared.is_missing_s3_error()` (only 404/NoSuchKey is absent) and `shared.s3_error_reason()` (missing / throttled / server_error / access_denied / transport) for the classification, rather than inventing a favorites-local rule. Note that `_head_artifact_keys` was updated (CR28 F13) to already return `error: true` + `error_reason` for a non-missing per-key failure instead of a bare `exists: false`, so the exact-key fallback in Proposal 3 gets the missing-vs-error distinction for free.
- `GoRender`, Download, Add to Book, and Delete keep using the favorite's own job and artifact identity.
- Color image identity must be canonicalized with `shared.parse_render_key()` / `assert_render_identity()`, not trusted from caller strings.
- The current bucket is public-read, so stable public URLs can be derived from canonical keys. If that deployment assumption changes, URL signing must become lazy rather than returning presigned URLs for every row.
- Existing presigned URLs expire after one hour. A cache that retains current `/render-summary` URLs must either refresh before expiry or replace them with stable public URLs.

## Proposal 1: Make Tab Entry Cache-Aware

Priority: highest. Risk: low. Backend changes: none.

Add explicit Favorites load state rather than treating `_favoriteRefsLoaded` as if it also meant artifacts were hydrated.

Suggested state:

```js
let _favoriteArtifactsReady = false;
let _favoriteArtifactsSignature = '';
let _favoriteArtifactsLoadedAt = 0;
let _favoriteLoadPromise = null;
let _favoriteLoadGeneration = 0;
```

Add a stable signature over the reference fields that affect the panel:

```text
job_id, artifact_id, added_at, image_key, preview_key, display_name
```

Change `loadFavoritesInventory(options = {})`:

1. Read `force` from `options`.
2. If refs are loaded, artifact cache is ready, and the refs signature matches, render immediately and return without hydration.
3. If an equivalent load is already running, return `_favoriteLoadPromise` instead of starting another.
4. On a forced refresh, increment `_favoriteLoadGeneration` and clear the ready marker.
5. Commit asynchronous results only if their generation is still current.
6. Set `_favoriteArtifactsReady` only after a successful hydration for the current signature.

`switchTab()` should call `void loadFavoritesInventory()` so the fire-and-forget intent is explicit.

`refreshFavoritesInventory()` should be the only normal path that passes `{force: true, reloadRefs: true}`.

Cache lifetime:

- If rows use stable public URLs, a session-long cache is safe for display.
- If rows retain one-hour presigned URLs, use a maximum age around 50 minutes and refresh URLs without discarding visible cached rows.
- Underlying deletion is discovered by explicit Refresh or image-load failure. Favorite artifact identity and image bytes are otherwise immutable.
- **Cross-device staleness is accepted, by design.** A session-long cache means a favorite added or removed on *another* browser/device will not appear until this session issues an explicit Refresh (or reloads). That is the correct trade for a global-but-rarely-mutated list: DDB stays the source of truth, and the only cost of the cache is that a second device's change is not pushed live. State it so it is a decision, not a surprise.

Expected result: reopening Favorites becomes immediate and network-free.

## Proposal 2: Render References Immediately

Priority: high. Risk: low/medium. Backend changes: none for the first version.

The current DDB rows already contain enough data to show a useful table and preview before metadata hydration completes. Build provisional rows directly from `_favoriteRefs`:

```js
{
    family: 'color',
    artifact_id: ref.artifact_id,
    favorite_job_id: ref.job_id,
    favorite_added_at: ref.added_at,
    display_name: ref.display_name || ref.artifact_id,
    image_key: ref.image_key || '',
    preview_key: ref.preview_key || '',
    image_url: publicStorageUrl(ref.image_key),
    preview_url: publicStorageUrl(ref.preview_key),
    viewer_url: publicStorageUrl(ref.preview_key || ref.image_key),
    hydration_state: 'pending'
}
```

Do not mark provisional rows as missing. The current fallback sets `missing: true` after any summary failure, which conflates transient failure with deletion.

First-open sequence:

1. Fetch `/list-favorites` once.
2. Build provisional rows.
3. Render the table and selected preview immediately.
4. Hydrate the selected row first.
5. Hydrate remaining rows in the background with bounded concurrency.
6. Update rows progressively and report `Details 8/24` rather than blocking the whole panel on the slowest job.

This makes time-to-first-content roughly one storage request even before the backend is optimized.

Do not rebuild the whole table after every background row. Store rows by stable key and patch the affected table cells. A single final render is acceptable for small lists, but per-result full `innerHTML` replacement will cause avoidable image reloads and scroll churn.

## Proposal 3: Replace Full-Job Hydration With Exact Favorite Snapshots

Priority: high. Risk: medium. This is the main backend fix.

Do not add another Lambda. Extend the existing storage Lambda and preferably keep the existing `/list-favorites` route.

### Compact DDB Snapshot

Persist a compact display snapshot in each favorite row when it is added:

```text
snapshot_version
favorite_job_id
favorite_artifact_id
added_at
display_name
image_key
preview_key
artifact_created_at
width
height
file_size
content_type
format
color_mode
palette
color_interpretation
postprocess_kind
derivation_kind
summary_text or the small fields needed to construct it
```

Do not persist the complete Render artifact object. It is large, contains fields unrelated to Favorites, and some overlay provenance can change later. The snapshot is for list display only; actions requiring complete current metadata can fetch it lazily.

`handle_add_favorite()` should resolve and validate the artifact server-side:

1. Validate `job_id` and `artifact_id`.
2. Resolve the canonical Color prefix or validate canonical client hints with `parse_render_key()` and `assert_render_identity()`.
3. `HEAD` only the exact image and preview candidates for that artifact.
4. Read only that artifact's overlay metadata if needed for the compact summary.
5. Store canonical keys and compact fields in DDB.
6. Ignore or reject mismatched caller-provided keys.

This moves a small exact lookup to the one-time Favorite action instead of repeating complete job scans on every tab visit.

`handle_list_favorites()` then becomes one DDB query that returns panel-ready rows. No S3 inventory is required for rows with a complete snapshot.

### Existing Rows And Exact Fallback

Old favorite rows do not have the snapshot fields. Hydrate those rows by exact identity, not `/render-summary`:

1. Query the Favorites partition once.
2. Partition rows into complete snapshots and legacy/incomplete rows.
3. Flatten image and preview candidate keys for incomplete rows.
4. Call `_head_artifact_keys(..., presign=False)` once over the flat key set.
5. Load required Color overlays in one bounded flat worker pool.
6. Build only the requested Color entries.
7. Optionally backfill the DDB rows with `snapshot_version` after successful resolution.

Do not call `list_objects_v2`. Favorite identity is already known, so a prefix inventory is unnecessary.

Avoid nested executors. Use one bounded pool over favorites or one batch HEAD pool plus one overlay pool. Reuse the storage client's configured connection pool.

### Refresh Semantics

Use the existing `/list-favorites` route with an optional body flag rather than adding route churn:

```json
{"refresh": true}
```

Normal list:

- Return DDB snapshots immediately.
- No S3 calls for complete rows.

Forced refresh:

- Validate exact canonical image/preview keys.
- Refresh compact metadata where appropriate.
- Mark true 404s as missing while preserving the favorite row.
- Return an `error` state for transient S3 failures rather than `missing`.
- Return timing and counts for observability.

Suggested response diagnostics:

```json
{
  "count": 24,
  "snapshot_hits": 22,
  "snapshot_backfills": 2,
  "missing": 1,
  "errors": 0,
  "ddb_us": 12000,
  "hydrate_us": 85000
}
```

Expected result: first open is one API call and one DDB query; explicit Refresh scales with favorite count, not with every artifact in every source job.

## Proposal 4: Make Add/Delete Write Through Locally

Priority: high. Risk: low after Proposal 1.

The mutation endpoints should return the affected row, not require a full partition reread:

```json
{"added": true, "favorite": {...}}
```

```json
{"deleted": true, "job_id": "...", "artifact_id": "..."}
```

Frontend behavior after Add:

- Insert the returned favorite at the front of `_favoriteRefs`.
- Insert its compact snapshot into `_favoriteArtifacts`.
- Recompute the signature.
- Keep `_favoriteArtifactsReady = true`.
- Do not call `loadFavoritesInventory()`.

Frontend behavior after Delete:

- Remove the stable key from `_favoriteRefs` and `_favoriteArtifacts`.
- Preserve the nearest selection.
- Recompute the signature.
- Do not call `_hydrateFavoriteArtifacts()`.

Backend behavior:

- `/add-favorite` should not query and return the whole Favorites partition after a successful conditional put.
- `/delete-favorite` should not query and return the whole partition after deleting one row.
- Keep compatibility temporarily by accepting either the old `favorites` response or the new single-row response in the frontend.

This improves mutation latency and prevents Add/Delete from re-triggering the original tab-load problem.

### Interaction with the `lambdaPost` retry classifier (CR28 F6)

`lambdaPost` now suppresses retries on ambiguous network failures for endpoints it classifies as mutations, via `_lambdaEndpointIsMutation` — a path match on `/(save|delete|cleanup|migrate)/`. Two consequences the single-row responses must respect:

- **`/delete-favorite` matches `/delete`** → treated as a mutation → not retried on an ambiguous failure. Fine.
- **`/add-favorite` does not match** any of those tokens → treated as idempotent → *will* be retried on an ambiguous network failure. This happens to be safe only because Add is a conditional put (`attribute_not_exists`): a retried Add is a no-op that returns `added: false`. So the new single-row write-through must treat `added: false` as **"already present — ensure the row is in the local list,"** not as a failure. Do not assume `added: true`.

If you would rather the classifier be honest, either rename to `/save-favorite` or extend the mutation regex to include `favorite`/`add`; but given the conditional put, documenting the idempotent-retry contract is sufficient and lower-risk.

## Proposal 5: Remove The Extra DDB Initialization Read

Priority: medium/low. Risk: low with migration tests.

The current list path performs `GetItem(__meta__)` and then queries the same partition. Query once instead:

1. Query the Favorites partition.
2. Detect the `__meta__` row in the returned items.
3. If present, return the favorite rows.
4. If absent, run the legacy S3 migration path once, then query or return the migrated rows.

Add a `ProjectionExpression` containing only fields needed by Favorites list display. This is a small optimization compared with removing `/render-summary`, but it makes the target one-round-trip DDB path real.

## Proposal 6: Reduce DOM Work After Network Work Is Fixed

Priority: low unless Favorites contains hundreds of rows.

`renderFavoritesPanel()` rebuilds the complete table and viewer on every selection. `_favoriteSelectKey()` therefore performs O(F) string generation and replaces the preview image element for a one-row selection change.

Improve this only after request fan-out is fixed:

- Build table rows once per inventory revision.
- On selection, remove/add the selected class on two rows.
- Update only the viewer and action buttons.
- Use event delegation instead of one inline handler per row.
- Add simple windowing only if measured row counts justify it.

This will improve keyboard navigation and large lists, but it is unlikely to explain the current tab-entry delay.

## What Not To Do

### Do Not Just Increase `asyncPool(4)`

Raising browser concurrency may shorten a small refresh on a fast network, but it multiplies concurrent Lambda executions and S3 thread pools while preserving the wrong unit of work. It is load amplification, not an architectural speedup.

### Do Not Add API Gateway Caching To `/render-summary`

It would cache a large, short-lived response containing one-hour presigned URLs and unrelated render families. Invalidation after artifact creation/deletion would be difficult, and Favorites would still transfer far more data than needed.

### Do Not Make `localStorage` Authoritative

An in-memory or localStorage mirror is acceptable for instant paint, but DDB must remain the source of truth across browsers and machines.

### Do Not Build A Global S3 Manifest Yet

A materialized Favorites manifest would add consistency and concurrent-write problems. The DDB partition is already the correct compact index. Use it directly.

### Do Not Presign Every Favorite On Every List

The bucket is public-read today. Use stable public URLs from canonical keys. If the bucket becomes private, presign only the selected preview and action-time download, with a small URL cache.

## Implementation Order

### Phase 0: Measure The Baseline

Add frontend timing around:

- favorites ref fetch
- hydration
- first render
- total load

Add backend timing to the optimized list response. Record `F`, `J`, request count, and p50/p95 load time from a representative favorites set.

### Phase 1: Instant Repeat Visits

Files:

- `js/01-core-compute.js`
- `js/04-palette-color.js` only if the shared state location is retained there
- `tests/test_frontend_js.sh`
- `tests/e2e/favorites-ui.spec.js`

Steps:

1. Add artifact-ready, signature, timestamp, promise, and generation state.
2. Make `loadFavoritesInventory()` idempotent.
3. Render valid cached artifacts before any optional revalidation.
4. Make explicit Refresh the force path.
5. Deduplicate overlapping loads.
6. Prevent stale completions from overwriting newer state.
7. Patch Add/Delete locally.

Checkpoint: entering Favorites twice must produce no second `/render-summary` or `/list-favorites` request.

### Phase 2: Immediate First Paint

Steps:

1. Build provisional rows from favorite refs.
2. Derive stable public preview/image URLs from canonical keys.
3. Render before hydration.
4. Hydrate selected-first and then progressively.
5. Distinguish `pending`, `ready`, `missing`, and `error`.

Checkpoint: a deliberately delayed hydration response must not delay table or selected preview visibility.

### Phase 3: Compact Backend Snapshots

Files:

- `lambda/handler_storage.py`
- `lambda/shared.py` only if a small canonical Color resolution helper belongs there
- `tests/test_storage_handler.py` or a new gated `tests/test_favorites_storage.py`
- `scripts/predeploy_check.sh` if a new test file is created

Steps:

1. Define `snapshot_version` and compact DDB fields.
2. Canonicalize and resolve the exact artifact on Add.
3. Return panel-ready snapshots from `/list-favorites`.
4. Add exact-key fallback for old rows.
5. Add `refresh:true` validation semantics.
6. Remove full-partition responses from Add/Delete.
7. Keep temporary frontend compatibility with old responses.

Checkpoint: the Favorites backend tests must assert that normal list performs no S3 `list_objects_v2` and never calls `handle_render_summary()`.

### Phase 4: Remove Legacy Hydration

Steps:

1. Backfill existing favorite rows.
2. Remove `_hydrateFavoriteArtifacts()` job-summary fan-out.
3. Remove transitional summary caches.
4. Retain exact selected-artifact fetch only where an action needs full current metadata.
5. Consider the DOM selection optimization after measuring the new path.

## Tests And Gates

Frontend tests:

- First tab entry loads refs once.
- Second tab entry performs zero storage calls.
- Two simultaneous loads share one promise.
- A forced Refresh reloads refs and exact metadata.
- Cached rows render immediately while refresh is pending.
- An older request cannot overwrite a newer generation.
- Add inserts one row without full hydration.
- Delete removes one row without full hydration.
- Presigned URL expiry or stable public URL behavior is covered.
- Transient hydration errors render `error`, not `missing artifact`.
- A true 404 renders the existing stale-reference row.

Backend tests:

- Complete snapshot list uses one DDB query and zero S3 calls.
- Existing incomplete rows use exact keys and no prefix listing.
- Add rejects mismatched job/artifact/image identity.
- Add stores canonical image and preview keys.
- Forced refresh marks only genuine 404s missing.
- Forced refresh surfaces throttling/5xx separately.
- DDB pagination remains correct.
- Add/Delete do not reread the whole partition.
- Snapshot backfill is idempotent.

Performance contract test with fakes:

- 100 favorites across 100 jobs must not trigger 100 `/render-summary` calls.
- Normal snapshot list must issue zero S3 operations.
- Legacy fallback must be O(F) exact operations and issue zero `list_objects_v2` calls.

Existing tests that will move under you (update, do not just append):

- `tests/e2e/favorites-ui.spec.js` currently drives the render-summary-based load (`"favorites tab loads artifacts, auto-selects one, and enables actions"`, GoRender, download menu, delete). Proposals 1–4 change the network shape (no `/render-summary`, provisional-then-hydrated rows, local write-through on delete), so these titles' mocked routes and timing assumptions must be revised in lockstep — otherwise the spec passes against mocks that no longer reflect the code.
- `TestFavoritesStorage` in `tests/test_pipeline.py` (5 tests: key-prefix, empty-list, add-prepend, add-dedupe, delete-removes-matching) is the only backend coverage, and it asserts the *old* full-partition responses. The single-row response (Proposal 4) will require updating these, not just adding new ones.

Gate note:

`tests/test_pipeline.py` is in `ALLOWED_UNGATED` (`tests/test_predeploy_gate_completeness.py:89`) and is **not** run by `scripts/predeploy_check.sh`, so the current Favorites backend tests never gate a deploy. New load-bearing Favorites tests should live in the already-gated `tests/test_storage_handler.py`, or in a dedicated `tests/test_favorites_storage.py` **explicitly added to `predeploy_check.sh`** (the gate-completeness meta-test will otherwise fail, per CR28 F20). The frontend harness (`test_frontend_js.sh`) is already gated; the Playwright spec is useful but, being ungated and mock-based, is not a substitute for a predeploy runtime assertion that the list path issues zero `list_objects_v2` and never calls `handle_render_summary`.

## Final Target

The final Favorites tab should behave as a compact indexed view, not as a cross-job Render refresh:

- one DDB query on first open
- zero requests on repeat opens
- immediate rows and preview
- explicit exact-key Refresh
- no full-job artifact inventories
- no unrelated render-family work
- no eager presigning of every artifact
- local write-through updates after Add/Delete
- stale references preserved and classified correctly

The frontend cache is the fastest low-risk fix. The compact DDB snapshot is the durable fix that makes first load scale with favorites rather than with every artifact saved under every favorite job.
