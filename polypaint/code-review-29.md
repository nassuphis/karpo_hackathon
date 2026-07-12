# Code Review 29: Gallery, DeepZoom, and Share Integrity

Date: 2026-07-12  
Reviewed HEAD: `322cdf3` (`Gallery: layout modes - maze / serpentine / exhibition`)  
Review mode: adversarial, findings-first, with focused runtime probes

## Verdict

The gallery implementation has a strong rendering and validation foundation, and the focused test suites are green. It is not yet concurrency-safe or identity-safe across all of the product paths it now exposes.

There are six confirmed findings:

| ID | Severity | Finding |
|---|---|---|
| F1 | HIGH | Concurrent DeepZoom exports for the same job share one task row and can share one S3 export prefix. |
| F2 | HIGH | The advertised three-way gallery merge is IDs-only; it loses concurrent edits and can leave Save permanently wedged. |
| F3 | HIGH | Mixed-family gallery identity is inconsistent between enrichment, deduplication, manifest normalization, and texture caching. |
| F4 | HIGH | Describe Selection performs a retrying external vision request synchronously inside the shared storage API Lambda. |
| F5 | MEDIUM | Gallery actions assume `viewer.html` exists although gallery admission never validates or records it. |
| F6 | LOW | A 40-bit, unconditionally written share ID can overwrite a supposedly immutable share. |

The first four should be fixed before treating the gallery as robust under normal multi-tab, multi-user, or repeated-click operation. F5 is a real broken-action path. F6 has low immediate likelihood but violates the immutable-share contract and is cheap to fix correctly.

## Scope Decision

Backward compatibility for the previous `gallery.html?manifest=...` URL shape is deliberately excluded from this review. The feature is still under development, and the product decision is that old gallery share URLs do not need to keep working. The current `?share=<id>`-only boot path is therefore not a finding.

This exception does not waive current data-contract failures. F5 is not a complaint that an old gallery URL changed. It is a mismatch between what the current gallery backend admits and what the current Gallery and standalone-viewer buttons claim is present for an admitted piece.

## Review Scope and Evidence

The review covered:

- `js/11-artifacts.js`
- `js/13-artifact-mosaics.js`
- `js/15-gallery.js`
- `gallery/app.js`
- `gallery/layout.js`
- `gallery/manifest.js`
- `gallery/texture-manager.js`
- `lambda/handler_dispatch.py`
- `lambda/handler_deepzoom_export.py`
- `lambda/handler_deepzoom_from_raw.py`
- `lambda/handler_storage.py`
- `lambda/book_describe.py`
- gallery, DeepZoom, deployment, frontend, and Playwright tests

The following existing gates pass on the reviewed HEAD:

```text
71 passed
  tests/test_gallery_backend.py
  tests/test_deepzoom_export_handler.py
  tests/test_deploy_packaging.py
  tests/test_frontend_parts_contract.py

Frontend fused render source checks: OK
Frontend generated editor help runtime checks: OK
Frontend results lazy preview runtime checks: OK
Frontend jobs rail runtime checks: OK
Frontend artifact mosaic runtime checks: OK
Frontend sequential load checks: OK (24 scripts)

42 passed
  tests/e2e/gallery-logic.spec.js
  tests/e2e/gallery-tab.spec.js
  tests/e2e/gallery-viewer-smoke.spec.js
```

Two focused executable probes were also run:

1. A frontend dirty-merge probe demonstrated loss of a remote name, layout, and title while adopting the remote revision.
2. A backend fake-S3 probe demonstrated both mixed-family rejection and mixed-family duplicate acceptance for the same `(job_id, artifact_id)`.

The passing suites do not contradict the findings. The relevant tests exercise happy-path serialization, concurrent additions only, and individually valid exports. They do not exercise overlapping DeepZoom runs, field-level gallery conflicts, or duplicate viewer texture identities.

## F1 - HIGH - Concurrent DeepZoom Exports Share Task and Export Identity

### Affected code

- `js/11-artifacts.js:1219-1287`
- `js/13-artifact-mosaics.js:1070-1079`
- `lambda/handler_dispatch.py:50-56`
- `lambda/handler_deepzoom_export.py:129-141`
- `lambda/handler_deepzoom_export.py:198-201`
- `lambda/handler_deepzoom_export.py:243-283`
- `lambda/handler_deepzoom_export.py:294-307`
- `lambda/handler_deepzoom_export.py:327-344`
- `lambda/handler_deepzoom_from_raw.py:11-17`
- `lambda/handler_storage.py:5376-5411`

### Current behavior

`runDeepZoomExport()` chooses the task identifier from the dispatch target:

```js
const dispatchTarget = useExactSource ? 'deepzoom_export' : ...;
const taskId = dispatchTarget;
```

That means every normal DeepZoom export for a given `job_id` uses the same DynamoDB key:

```text
(job_id, task_id) = (<job>, deepzoom_export)
```

The browser then deletes that shared row before dispatch:

```js
await lambdaPost('storage', { job_id: jobId, task_id: taskId }, '/delete-task');
```

The dispatched job contains neither `task_id` nor `export_id`:

```js
jobs: [{
    job_id: jobId,
    source_key: sourceKey,
}]
```

The exporter therefore uses two process-wide defaults:

```python
export_id = params.get("export_id", f"dz_{int(time.time())}")
...
return handle_deepzoom_export_request(..., task_id="deepzoom_export")
```

The status identity is fixed, and the S3 export identity has only one-second resolution when the caller omits it.

### Exact failure timeline

Consider two exports, A and B, launched for different artifacts in the same job:

1. A deletes `(job, deepzoom_export)`.
2. A dispatches and writes status `started` to `(job, deepzoom_export)`.
3. B deletes `(job, deepzoom_export)` while A is still running.
4. B dispatches and writes `started` to the same row.
5. A and B alternately overwrite `generating`, `uploading`, `done`, or `error` in that row.
6. Both browser pollers query `begins_with(task_id, "deepzoom_export")` and see the same row.
7. The first worker to write `done` makes both browser operations appear complete, even if the second worker is still running or will fail.

If A and B begin in the same second, both also default to the same export prefix:

```text
deepzoom/<job>/dz_<same_epoch_second>/
```

Both workers then upload `image.dzi`, pyramid tiles, `viewer.html`, and `meta.json` into the same prefix. The resulting object set can be a last-writer mixture from two different source images. `renders/<job>/deepzoom_latest.json` is also last-writer-wins, which is acceptable for a latest pointer only if the export prefixes themselves remain unique. They currently do not.

Even if the calls start in different seconds and get different S3 prefixes, the shared task row still causes false completion and error masking.

### Why this is reachable

The Render tab disables one button while its call is active, but that is not a system-level lock:

- The same job can be open in multiple browser tabs.
- AllCol and AllPal call `runDeepZoomExport(..., null, ...)` and intentionally do not await it.
- A user can dispatch different mosaic tiles from the same result while an earlier export is still running.
- The browser button state cannot serialize another client or another UI entry point.

The jobs rail does not solve this. It gives each card a unique local `railId`, but both cards poll the same backend task row.

### Impact

- A job card can report success for the wrong export.
- A failed export can be hidden by another export's `done` status.
- Two exports can overwrite the same immutable S3 prefix.
- `meta.json` can describe one source while some pyramid tiles came from another.
- Refresh can expose a corrupt or nondeterministic DeepZoom export.
- Debugging is misleading because the UI has distinct cards but the backend has one operation identity.

This is a data-integrity issue, not only a progress-display issue.

### Required fix

Generate one operation ID at the caller and thread it through every layer without replacement:

```js
const randomId = crypto.randomUUID().replace(/-/g, '');
const runId = Date.now().toString(36) + '_' + randomId;
const exportId = 'dz_' + runId;
const taskId = 'deepzoom_export_' + runId;

jobs: [{
    job_id: jobId,
    source_key: sourceKey,
    export_id: exportId,
    task_id: taskId,
}]
```

The Lambda wrappers must read the supplied `task_id` instead of hardcoding it:

```python
task_id = assert_safe_id(params["task_id"], "task_id")
return handle_deepzoom_export_request(
    params,
    require_raw_sidecar=False,
    task_id=task_id,
)
```

The same rule must apply to `deepzoom_from_raw`.

Polling should target the exact task key. `handle_check_status()` currently uses `begins_with(task_id, :pfx)`, which is useful for multi-worker runs but is unnecessarily loose for a singleton DeepZoom operation. Either add an exact-task status route backed by `GetItem`, or add an explicit exact mode to `handle_check_status`. Merely passing a longer prefix is better than the current code but still leaves prefix semantics where exact identity is required.

The browser should retain `exportId` and `taskId` in the jobs-rail run object. Completion should be accepted only from that exact task. The worker's terminal `result_data` should include `export_id`, `source_key`, `source_family`, and `source_artifact_id`, allowing the caller to verify it completed the operation it launched.

Do not use an S3 existence check as the uniqueness mechanism. The export ID should be collision-resistant, and immutable export writes should use a create-only marker or otherwise fail if a supposedly new export prefix already exists.

### Acceptance tests

- Launch two exports for different artifacts in the same job and assert distinct `task_id` and `export_id` values reach the dispatcher.
- Interleave status events and prove A cannot complete from B's `done` row.
- Freeze time to one second, launch two exports, and assert their S3 prefixes are still different.
- Make A succeed and B fail; assert the two jobs-rail cards end in different states.
- Run the same tests for `deepzoom_export` and `deepzoom_from_raw`.
- Replace the current source-string assertion in `tests/test_frontend_js.sh` with a behavioral test of the dispatched payload and polling identity.

## F2 - HIGH - The Dirty Gallery Merge Loses Concurrent Edits and Can Wedge Save

### Affected code

- `js/15-gallery.js:11-30`
- `js/15-gallery.js:96-128`
- `js/15-gallery.js:256-301`
- `js/15-gallery.js:405-466`
- `lambda/handler_storage.py:4953-5013`
- `tests/e2e/gallery-tab.spec.js:191-230`

### Current behavior

The frontend calls `_galleryNotifyChanged()` a three-way merge, but it stores only a set of base piece identities:

```js
baseIds: new Set()
```

`_gallerySyncBase()` discards the actual base document:

```js
_galleryState.baseIds = new Set(gallery.pieces.map(_galleryPieceKey));
```

When a server update arrives while local edits are dirty, the merge performs exactly one operation: append pieces that did not exist in the base or local document.

```js
const added = gallery.pieces.filter((p) =>
    !baseIds.has(_galleryPieceKey(p)) && !localIds.has(_galleryPieceKey(p)));
local.pieces = local.pieces.concat(added);
_gallerySyncBase(gallery, revision);
```

It does not merge or detect changes to:

- gallery name
- curator titles on existing pieces
- piece removal
- piece order
- sky mode
- wall color
- wall coverage
- self tint
- edge-line thickness
- wall layout

It nevertheless adopts the latest server revision. A later Save is therefore authorized to overwrite the remote values that were ignored.

### Reproduced lost-update case

The following state was exercised directly against `js/15-gallery.js`:

```text
Base:
  name = Base
  layout = maze
  pieces = A(title=""), B(title="old B")

Local dirty state:
  name = Local name
  A.title = local A

Remote state handed to _galleryNotifyChanged:
  revision = r2
  name = Remote name
  layout = exhibition
  B.title = remote B
  added C
```

Actual result:

```json
{
  "revision": "r2",
  "name": "Local name",
  "layout": "maze",
  "titles": [
    ["A", "local A"],
    ["B", "old B"],
    ["C", ""]
  ]
}
```

The concurrent addition survives, but every other remote edit is silently discarded. Because revision `r2` was adopted, the next Save can overwrite those remote edits without a 409.

### Reproduced wedge class

The removal case is worse:

1. Base contains A and B.
2. The local client edits A and remains dirty.
3. The remote client removes B and commits revision `r2`.
4. A server update at `r2` reaches `_galleryNotifyChanged()`.
5. The IDs-only merge leaves local B in place and adopts `r2`.
6. The next Save sends A and B with `expected_revision=r2`.
7. The backend revision check passes because the revision is current.
8. `handle_save_gallery()` builds `stored_by_id` from the server document, where B no longer exists.
9. B fails validation as `unknown piece`, producing a 400 rather than a conflict.
10. The frontend treats a non-409 as retryable and keeps the same invalid local document.

Every subsequent Save repeats the same 400 until the user manually discards/reloads. The UI has accepted a revision for a state it cannot legally save.

### Why existing tests miss it

The two merge tests in `tests/e2e/gallery-tab.spec.js` cover this one shape:

```text
local edit + remote additions
```

That is exactly the only shape the implementation handles. They do not cover:

- remote modification of an existing title
- remote rename
- remote settings edit
- remote reorder
- remote removal
- both sides modifying the same field
- both sides reordering

The tests therefore validate the narrow append behavior while naming it a general three-way merge.

### Required fix

Store a full immutable base snapshot, not only `baseIds`:

```js
baseDoc: structuredClone(gallery)
```

Perform a real three-way merge over `base`, `local`, and `remote`.

For scalar fields such as `name` and each setting:

```text
local == base, remote changed  -> take remote
remote == base, local changed  -> take local
local == remote                -> take either
both changed differently       -> explicit conflict
```

For piece identity, use the same canonical tuple everywhere:

```text
(job_id, family, artifact_id)
```

For per-piece titles, apply the same field-level rule. Server-owned fields such as keys, dimensions, function metadata, and DeepZoom references must always come from the remote document, never from the local snapshot.

For additions and removals:

```text
added on one side only                         -> keep the addition
removed on one side, unchanged on the other   -> keep the removal
removed on one side, modified on the other    -> conflict
same identity added differently on both sides -> conflict unless equivalent
```

Order needs an explicit policy. A practical policy is:

- If only local order changed, preserve local order and place remote-only additions deterministically.
- If only remote order changed, use remote order and place local-only additions deterministically.
- If both changed order differently, surface a conflict instead of silently selecting one.

If any conflict is unresolved, do not adopt the remote revision as though the local document were based on it. Mark the gallery conflicted, disable ordinary Save, and offer an explicit resolution/reload path. A simpler safe alternative is to refuse automatic merging whenever any non-addition remote change is detected. That is less convenient but correct.

Longer term, the cleanest API is operation-based curation rather than whole-document replacement: rename, set-title, set-settings, move, remove, and add can each CAS-retry against the latest document. That is more work, but it removes most client-side document merge complexity.

### Acceptance tests

- Local A title edit plus remote B title edit preserves both.
- Local rename plus remote wall-layout edit preserves both.
- Both sides change the same title differently and produce an explicit conflict.
- Remote removal plus untouched local piece removes it cleanly.
- Remote removal plus local title edit produces a conflict, never a 400 Save loop.
- Concurrent local and remote reorders are either deterministically merged or explicitly rejected.
- Every setting added by the current layout work participates in the merge tests.
- After every successful merge, immediately saving the merged document succeeds against the adopted revision.

## F3 - HIGH - Mixed-Family Identity Is Internally Contradictory

### Affected code

- `lambda/handler_storage.py:4470-4555`
- `lambda/handler_storage.py:4584-4727`
- `lambda/handler_storage.py:5026-5065`
- `lambda/handler_storage.py:4730-4762`
- `gallery/manifest.js:69-132`
- `gallery/manifest.js:172-229`
- `gallery/manifest.js:232-277`
- `gallery/app.js:418-453`
- `gallery/app.js:622-650`
- `gallery/texture-manager.js:44-63`

### Current behavior

The gallery began as color-only, then gained a DZI fallback that can build a gallery piece from any canonical render family. The resulting implementation now has incompatible identity rules at different layers.

The backend deduplicates saved pieces by:

```text
(job_id, family, artifact_id)
```

The editable document and share snapshot carry `family` per piece. This implies that `family` is part of piece identity.

The standalone viewer, however, identifies textures by:

```js
_pieceId(piece) { return piece.job_id + '/' + piece.artifact_id; }
```

The texture manager stores desired, resident, inflight, retry, negative-cache, generation, and pin state in Maps keyed by that ID. Family is omitted.

At admission time, `_enrich_gallery_pick()` always tries to resolve a color artifact first. It consults the DeepZoom export's family only after that decision.

### Failure A: a valid non-color export is rejected when a color artifact has the same ID

The following current-HEAD backend probe was run:

```text
renders/jobX/color/same/... exists
deepzoom/jobX/dz_b/meta.json identifies renders/jobX/bilevel/same/image.tif
request add jobX/same/dz_b
```

Actual result:

```text
added = false
reason = export_identity_mismatch
```

The non-color export is valid and self-consistent. It is rejected because `_enrich_gallery_pick()` finds `renders/jobX/color/same/` first, enters the color path, and `_validate_gallery_export()` correctly refuses a `bilevel` export for that color tile. The wrong family was selected before validation began.

This makes admission depend on whether an unrelated color artifact happens to reuse the same artifact ID.

### Failure B: two families with the same job and artifact ID are accepted but share one texture ID

The following current-HEAD backend probe was also run:

```text
deepzoom/jobX/dz_b -> renders/jobX/bilevel/same/image.tif
deepzoom/jobX/dz_c -> renders/jobX/coeffs/same/image.tif
```

Both adds succeed. The resulting pieces are:

```text
(jobX, bilevel, same, dz_b)
(jobX, coeffs,  same, dz_c)
```

That is consistent with the backend's `(job, family, artifact)` deduplication rule. In the viewer, both become:

```text
jobX/same
```

During `_scheduleTextures()`, both desired rows enter `GalleryTextureManager.setDesired()`. Its Map keeps only one value for that key. Both art meshes then call `tm.get("jobX/same")` and bind the same resident texture. The wall can therefore show one family's preview twice, depending on ordering and load timing.

Focus and inspection pins also collide. Focusing one piece pins the shared ID; inspecting or unfocusing the other adjusts the same refcount. Negative cache and retry state likewise leak between the two pieces.

### Additional contract inconsistencies

The same partial migration is visible in the manifest layer:

- Share manifests are hardcoded as `artifact_kind: "color"` even when pieces are mixed-family.
- `normalizeManifest()` rejects any top-level kind other than `color`, then returns `artifactKind: "color"` regardless of per-piece families.
- `normalizeGalleryPiece()` accepts a `family`, but an absent family normalizes to `""` while backend identity defaults it to `"color"`.
- `validateRenderKey()` accepts any family in `image_key` but does not require it to equal the row's declared `family`.
- Preview validation still carries color-only naming (`validateColorKey`) and is saved only by the special DZI-preview fallback for non-color pieces.

None of these alone necessarily crashes the viewer, but together they show that the product has no single authoritative identity contract.

### Impact

- Valid non-color exports are rejected based on unrelated color-key presence.
- Two accepted pieces can display the same image even though their manifests name different previews.
- Texture load errors and retries can be attributed to the wrong piece.
- Focus and inspection pin accounting can affect the wrong piece.
- A manifest can declare one family while linking an original from another family with the same IDs.
- Future code reading `artifact_kind` receives false metadata.

The wrong-image outcome makes this HIGH severity.

### Required fix

Define and use two explicit identities:

```text
render piece identity = (render_job_id, family, artifact_id)
export identity       = (export_job_id, export_id)
```

Admission should resolve export identity before choosing an enrichment path:

1. Read the named export metadata once.
2. Derive family from a validated canonical `source_key` and/or explicit `source_family`.
3. Reject metadata when those two family claims disagree.
4. If family is `color`, use the rich color overlay path.
5. Otherwise, use the DZI-derived piece path directly.
6. Never probe color first when the export already identifies another family.

If callers already know the family, include it in `/add-to-gallery` and verify it against export metadata. Do not trust it as a substitute for checking the export.

The frontend and viewer must use the same piece key as the backend:

```js
function galleryPieceId(piece) {
    return `${piece.job_id}/${piece.family || 'color'}/${piece.artifact_id}`;
}
```

Use that helper for:

- editable-gallery selection
- deduplication
- texture-manager IDs
- focus and inspection pins
- deterministic tie-breakers
- Describe target identity

Manifest normalization should default a missing family to `color`, validate that a canonical `image_key` family matches the declared family, and expose a truthful top-level kind. The top-level field can be `mixed`, or it can be removed in the next schema version if per-piece family is authoritative. Keeping `artifact_kind: color` while knowingly carrying non-color pieces is not acceptable.

### Acceptance tests

- A bilevel export remains addable when a color artifact with the same job/artifact ID exists.
- A mismatched export family and source-key family is rejected.
- Bilevel and coeffs pieces with the same job/artifact ID produce distinct viewer IDs and distinct textures.
- Focus, pin, retry, and negative-cache state remain independent for those two pieces.
- A row declaring `family=bilevel` with an `image_key` under `coeffs` is rejected or degraded explicitly.
- The share's top-level kind accurately represents a mixed-family piece set.
- Color-only galleries continue to normalize and render unchanged.

## F4 - HIGH - Describe Selection Runs Retrying Vision I/O in the Storage API Lambda

### Affected code

- `js/15-gallery.js:364-395`
- `lambda/handler_storage.py:1581-1584`
- `lambda/handler_storage.py:5087-5151`
- `lambda/book_describe.py:141-174`
- `lambda/book_describe.py:197-245`
- `deploy.sh:32`
- `deploy.sh:1731-1787`
- `deploy_manifest.json:84-96`
- `js/14-book.js:754-852` as the existing correct async precedent

### Current behavior

`galleryDescribeSelection()` waits synchronously for `/describe-gallery`:

```js
const resp = await lambdaPost('storage', ..., '/describe-gallery');
```

The route executes `handle_describe_gallery()` directly in `polypaint-storage`. That function downloads the preview, downscales it, calls the external vision provider, updates the gallery, and only then returns the API response.

The vision helper is explicitly designed for long-running batch work:

```python
def _gemini_call(..., attempts=8, ...):
    with urllib.request.urlopen(req, timeout=120) as resp:
        ...
    wait = min(Retry-After or delay, 90.0)
    time.sleep(wait)
```

One provider attempt can block for 120 seconds. Retry waits can be as long as 90 seconds each. Eight attempts plus backoff can approach or exceed the storage Lambda's 900-second timeout. The HTTP API integration is created synchronously in `deploy.sh`, with no custom long-running job protocol.

The comment `targets = targets[:4]  # stay well inside the API gateway budget` is false. Reducing the number of pieces does not bound the first external request below the gateway budget. The frontend currently sends one piece, but one request is enough to exceed it.

### Failure timeline

1. The browser POSTs `/describe-gallery` through API Gateway.
2. Storage consumes one of its five reserved concurrency slots.
3. The vision provider stalls, returns 429, or returns a retryable 5xx.
4. `_gemini_call()` waits or retries beyond the interactive HTTP integration window.
5. The browser receives a gateway timeout or generic request failure.
6. The storage Lambda can continue running after the client has lost the response.
7. It may still write the title successfully.
8. The user sees failure and can retry, launching another expensive request against uncertain state.

Because the storage Lambda has reserved concurrency `5`, five slow Describe calls can consume every storage slot. That blocks unrelated operations routed to the same function, including Results, favorites, gallery fetch/save, DeepZoom inventory, books, and status polling.

### Why the current tests miss it

Backend tests patch `_vision_call()` to return or throw immediately. Frontend tests replace `lambdaPost()` with an in-memory function. No test exercises a slow provider, retry delay, gateway timeout, or storage concurrency exhaustion.

### Existing correct pattern

The Book tab already solves this exact problem in `js/14-book.js`:

1. Generate a unique run and task ID.
2. Dispatch `op: describe` asynchronously to `book_pdf`.
3. Put a card on the jobs rail.
4. Poll the task status every three seconds.
5. Reload the saved document on completion.

Gallery Describe should follow that design instead of importing the batch vision engine into the storage request path.

### Required fix

Move all vision I/O out of `handler_storage.py`.

One practical implementation is to extend the existing `book_pdf` worker with `op: describe_gallery`, since that worker already packages `book_describe.py`, has S3/DynamoDB access, and is reached through the async dispatcher. A dedicated smaller Gallery Describe Lambda is also valid if the 4096 MB book container is too expensive for one thumbnail.

The frontend flow should be:

```text
Save pending gallery edits
Generate unique gallery-describe task ID
Dispatch worker and return immediately
Show jobs-rail/progress state
Poll exact task status
Reload gallery on done
Surface conflict/error without pretending the title failed if it was saved
```

The worker payload should include:

```text
gallery_id
expected_revision
job_id
family
artifact_id
overwrite
task_id
```

The expected revision matters. The worker must not overwrite a curator title or settings from a newer gallery revision. A conservative policy is to fail with a conflict if the gallery moved before the worker writes. A more permissive policy may CAS-retry only if the target piece still exists and its title has not changed since dispatch. Either policy is valid; silent overwrite is not.

The storage route may remain as a fast start/status endpoint if useful, but it must not call `_vision_call()`.

### Acceptance tests

- Starting Describe returns promptly without invoking vision in the storage Lambda.
- The dispatched task ID is unique and is the exact row the UI polls.
- A provider call delayed beyond 30 seconds does not hold an API Gateway request open.
- A 429/retry sequence updates jobs-rail progress and does not consume storage concurrency.
- A gallery edit landing before the worker save produces a defined conflict outcome.
- A completed title is reloaded and shown even if the original browser tab was hidden or reopened.
- Multiple gallery describes do not block `/list`, `/fetch-gallery`, or `/check-status`.

## F5 - MEDIUM - Go DeepZoom and Copy Link Assume an Unvalidated `viewer.html`

### Affected code

- `lambda/handler_storage.py:4514-4555`
- `lambda/handler_storage.py:4604-4682`
- `js/15-gallery.js:352-362`
- `gallery/app.js:819-834`
- `tests/test_gallery_backend.py:118-133`
- `tests/test_gallery_backend.py:183-198`

### Current behavior

Gallery admission validates:

- DeepZoom `meta.json`
- `image.dzi`
- the DZI descriptor
- a usable preview pyramid tile

It never checks:

```text
deepzoom/<export_job_id>/<export_id>/viewer.html
```

The stored `deepzoom` reference does not carry a validated viewer key or a capability flag.

Both current UI actions nevertheless construct that object path unconditionally:

```js
const url = _publicStorageUrl(
    `deepzoom/${p.export_job_id || p.job_id}/${p.deepzoom.export_id}/viewer.html`
);
```

The standalone gallery's Copy Link action does the same.

This is already demonstrated by the backend test fixture: `_seed_export()` creates `meta.json`, `image.dzi`, and one preview tile, but no `viewer.html`. `test_add_enriches_appends_with_deepzoom()` accepts the piece. Therefore the tested admission contract is strictly weaker than the UI action contract.

### User-visible result

The gallery itself can render the preview and inline OpenSeadragon view because those use the validated DZI. Pressing Go DeepZoom opens an unverified URL and immediately flashes `Opened`, even when the object is absent and the new tab returns 404. Copy Link similarly reports success for a dead URL.

This is not limited to historical data. Any partially deleted or manually repaired export can satisfy the current admission checks while lacking `viewer.html`.

### Required fix

Choose one explicit product contract.

Strict contract:

- A gallery piece with DeepZoom must have `viewer.html`.
- Admission HEADs the canonical viewer key.
- Missing viewer rejects the add as `export_viewer_absent`.

Capability contract:

- DZI is sufficient for gallery admission and inline zoom.
- Admission separately HEADs `viewer.html` and stores a validated optional `viewer_key`.
- Go DeepZoom and Copy Link are enabled only when that capability exists.
- Missing viewer leaves inline zoom working and gives a truthful message.

The capability contract is more flexible and matches the existing "has a DZI => curatable" rule. The manifest validator must reconstruct the viewer URL from validated IDs/key shape, just as it reconstructs the DZI URL. It must never trust a free absolute URL from the manifest.

Do not infer capability merely from `export_id`. The missing object is the failure.

### Acceptance tests

- An export with DZI but no viewer remains inline-zoomable under the capability contract.
- Go DeepZoom is disabled or reports `standalone viewer unavailable`; it never flashes success or opens a known-dead URL.
- An export with a valid viewer opens the exact owner-scoped key.
- Cross-job export ownership remains correct.
- Copy Link uses the same validated capability as Go DeepZoom.

## F6 - LOW - Short Share IDs Can Overwrite Immutable Shares

### Affected code

- `lambda/handler_storage.py:4730-4770`
- `tests/test_gallery_backend.py:344-365`

### Current behavior

Gallery shares use ten hexadecimal UUID characters:

```python
share_id = uuid.uuid4().hex[:10]
```

That is a 40-bit namespace:

```text
16^10 = 1,099,511,627,776 possible IDs
```

The object is then written with an unconditional `put_object()`. There is no `IfNoneMatch="*"`, no collision check, and no retry.

The comment calls this collision-safe, but truncation changes the relevant probability from UUID-scale to birthday-bound scale. Approximate collision probabilities are:

```text
10,000 shares:   0.0045%
100,000 shares:  0.45%
1,000,000 shares: about 36.5%
```

The immediate deployment may never approach those counts, which is why this is LOW rather than HIGH. The correctness problem is still real: a collision does not fail. It overwrites an existing immutable share.

The consequences are unusually bad because the object is served with:

```text
Cache-Control: public, max-age=31536000, immutable
```

After an overwrite, some clients can retain the old gallery for a year while new clients see the replacement at the same supposedly immutable URL.

### Required fix

Use both sufficient entropy and an atomic create-only write:

- Generate at least 96 bits, preferably 128 bits, encoded as URL-safe base64 or full UUID hex.
- Write `manifest.json` with `IfNoneMatch="*"`.
- On precondition failure, generate a new ID and retry a bounded number of times.

A 128-bit base64url token is only 22 characters, still short enough for a share link. Entropy reduces probability; conditional creation provides the actual no-overwrite guarantee.

The test fake already implements `IfNoneMatch`, so this is straightforward to pin. The existing `len(share_id) <= 12` assertion is a product-shape assertion, not a safety proof; it must be updated if the identifier moves to a safe 96-bit or 128-bit encoding.

### Acceptance tests

- Assert share creation uses `IfNoneMatch="*"`.
- Force the ID generator to collide once and then succeed; assert the old manifest is untouched and the second ID is returned.
- Force repeated collisions and assert a bounded explicit failure, not overwrite.
- Keep the immutable cache header only after create-only semantics are established.

## Test-Gap Summary

The current tests are substantial and useful, but the missing cases align exactly with the findings:

| Area | Current coverage | Missing load-bearing case |
|---|---|---|
| DeepZoom export | Single export, packaging, status happy path | Two overlapping exports for one job with interleaved status |
| Gallery merge | Local edit plus remote additions | Remote edit/removal/reorder/settings and true conflicts |
| Mixed families | Individual DZI fallback accepted | Same IDs across families and viewer texture identity |
| Describe | Immediate mocked success/failure | Slow/retrying provider and API/storage isolation |
| Viewer link | URL construction | Backend-admitted export with missing `viewer.html` |
| Share ID | Short length and immutable header | Collision plus create-only enforcement |

The focused suites should remain in predeploy. The fixes need behavioral tests added to those same gated files; source-string assertions are not enough for operation identity or conflict behavior.

## Review of the New Layout Commits

The three commits after `b937ffb` were included in this pass:

- `e733bc9` edge accents
- `cd8f6a4` configurable edge thickness
- `322cdf3` maze/serpentine/exhibition layouts

No separate confirmed layout defect was found. The shared grid-to-layout path, collision representation, manifest setting validation, backend setting normalization, and focused browser tests are coherent. The focused 42-test gallery Playwright suite passes on current HEAD.

The layout additions do increase the blast radius of F2 because `wall_edge_px` and `wall_layout` are now concurrent-editable fields that the IDs-only merge can silently overwrite. That is accounted for in F2 rather than filed as a layout bug.

## Recommended Fix Order

1. Fix F1 so every DeepZoom operation has unique, end-to-end task and export identity.
2. Fix F3 so the gallery has one family-aware identity contract before more non-color pieces are admitted.
3. Fix F2 with a real base document and explicit conflict policy before relying on collaborative/multi-tab curation.
4. Fix F4 by moving vision work to the existing async dispatch/poll pattern.
5. Fix F5 by making standalone-viewer availability an admitted capability rather than an assumption.
6. Fix F6 with a create-only, high-entropy share key.

F1 and F3 protect artifact correctness. F2 protects curator data. F4 protects service availability. F5 and F6 then close the remaining action and immutability contract gaps.

## Definition of Done

This review is closed only when all of the following are true:

- Two overlapping DeepZoom exports for one job cannot share status or an S3 prefix.
- A gallery merge cannot silently lose a remote field change.
- A remote piece removal cannot produce a repeatable 400 Save loop.
- Piece identity includes family consistently from storage through the texture cache.
- A non-color export is not rejected merely because a color artifact reuses its ID.
- Gallery vision requests return asynchronously and cannot occupy storage Lambda slots during provider retries.
- Go DeepZoom and Copy Link use a validated standalone-viewer capability.
- Gallery share creation is atomic and cannot overwrite an immutable share.
- The focused backend, frontend, and 42-test gallery Playwright suites remain green with the new adversarial cases added.
