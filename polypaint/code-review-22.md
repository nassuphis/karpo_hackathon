# Core Review 22: Lazy Color Render Count In Results Sidebar

## Feature Description

Add a selected-result `Color renders` count to the Results sidebar. The count should show how many usable color render artifacts exist for the selected compute result.

Do not add a `Render#` column to the Results table. The table is already visually dense, and counting color artifacts for every result during `/list` would add one or more S3 calls per saved job. With many saved jobs, that turns a fast metadata refresh into a background inventory crawl.

The target behavior:

- Results refresh stays fast and continues to load only compute metadata.
- Selecting a row shows `Color renders: ...` in the sidebar.
- The count is fetched lazily for the selected `job_id` only.
- The count updates to a number when available, `0` when there are no color renders, and `?` on count failure.
- The count is cached in the in-memory Results row so reselecting the same result does not repeat the S3 count.
- The table layout is tightened, especially numeric columns, and the sidebar is widened.

## Current State

### Results List

`lambda/handler_storage.py::handle_list` lists compute results by:

- Listing top-level prefixes under `renders/` with `Delimiter="/"`.
- Reading `renders/<job_id>/calc.json` per job in a parallel pool.
- Returning only table metadata: `job_id`, `function`, `degree`, `N`, `n1`, `n_chunks`, `times`, `total_size`, `total_roots`.

Recent hardening already skips prefixes that do not have `calc.json`, so preview-only or otherwise incomplete prefixes do not appear as broken `?` rows.

Important consequence: adding color render counts directly to `/list` would add an extra render-inventory operation for every job on every refresh. That is the wrong default for a large saved-job set.

### Color Artifact Storage

Color artifacts are stored under:

```text
renders/<job_id>/color/<artifact_id>/image.jpeg
renders/<job_id>/color/<artifact_id>/image.png
renders/<job_id>/color/<artifact_id>/preview.png
renders/<job_id>/color/<artifact_id>/meta.json
```

The render inventory path already knows how to list color variants:

- `RENDER_FAMILY_DIRS["color"] == "color"`.
- `RENDER_FAMILY_SHAPES["color"]["image_candidates"] == ["image.jpeg", "image.png"]`.
- `_list_render_family_variants(job_id, "color")` lists `renders/<job_id>/color/` with `Delimiter="/"`, then verifies image/preview candidates.
- `handle_render_summary` returns all render families and appends a legacy color variant from root-level `renders/<job_id>/image.jpeg` / `image.png` when present.

### Results UI

The Results tab layout is inline HTML in `index.html`.

Current table columns:

```text
Job ID | Function | Deg | N | Times | Size
```

Current sidebar width is fixed at `200px`, and it shows metadata such as version, transforms, degree, chunks, times, roots, solver, preview stats, viewport, and the preview image.

`js/01-core-compute.js` owns:

- `loadResults()`: calls `lambdaPost("storage", ..., "/list")`.
- `renderResultsTable()`: renders the table from `_resultsCache`.
- `selectResult(jobId)`: selects a row, populates sidebar placeholders, fetches `/detail`, and calls `_applyDetail`.
- `_applyDetail(...)`: applies `/detail` metadata and starts lazy preview generation if needed.

The right place for the new count is the sidebar, not the table.

## Design Decision

Use a selected-result lazy count endpoint:

```text
POST /render-count
body: { "job_id": "compute_..." }
```

Return:

```json
{
  "job_id": "compute_...",
  "family": "color",
  "color_artifact_count": 3,
  "legacy_color_artifact_count": 1,
  "color_render_count": 4,
  "count_us": 12345
}
```

Semantics:

- `color_artifact_count` counts immutable color artifacts under `renders/<job_id>/color/<artifact_id>/` that have at least one valid image candidate (`image.jpeg` or `image.png`).
- `legacy_color_artifact_count` is `1` if the legacy root-level color image exists, otherwise `0`.
- `color_render_count` is the total shown to the user: immutable count plus legacy count.
- The UI label should use `color_render_count` and display it as `Color renders`.

This matches the Render tab inventory better than a raw prefix count, because incomplete artifact folders do not count.

## Implementation Steps

### 1. Backend: Reuse Canonical Render Inventory Helpers

Modify `lambda/handler_storage.py`.

Do not add a parallel counting implementation. Reuse the existing canonical inventory helpers that already drive the Render tab:

- `_list_render_family_variants(job_id, "color")`
- `_legacy_render_variant(job_id, "color")`

Reason:

- This guarantees the sidebar count matches exactly what the Render tab can display.
- It inherits `_list_render_family_variants`' existing `ThreadPoolExecutor(max_workers<=20)` fan-out for per-artifact HEAD checks.
- It avoids drift if image candidates or artifact validity rules change later.

Implementation note:

- `_list_render_family_variants` currently presigns image/preview URLs because render inventory needs them. For one selected job this is acceptable. If this becomes measurable later, add an optional `presign=False` parameter to `_list_render_family_variants` and thread it into `_head_artifact_keys`; do not fork the validity logic.
- `_order_color_variants` is not needed for counting. It reorders variants for display and does not filter them, so `len(_list_render_family_variants(...))` is sufficient.

### 2. Backend: Add `/render-count`

Add:

```python
def handle_render_count(event):
    params = parse_body(event)
    job_id = str(params.get("job_id") or "").strip()
    if not job_id:
        raise ValueError("render-count requires job_id")

    t0 = time.time()
    immutable = len(_list_render_family_variants(job_id, "color"))
    legacy = 1 if _legacy_render_variant(job_id, "color") else 0
    return ok_response({
        "job_id": job_id,
        "family": "color",
        "color_artifact_count": immutable,
        "legacy_color_artifact_count": legacy,
        "color_render_count": immutable + legacy,
        "count_us": int((time.time() - t0) * 1e6),
    })
```

Wire it in the storage dispatch chain:

```python
elif path.endswith("/render-count"):
    return _handle_storage_route(handle_render_count, event)
```

Place it near `/render-summary`, because it is part of render artifact inventory.

### 3. API Manifest

Update `deploy_manifest.json` storage routes:

```json
"/render-count"
```

Regenerate the checked-in API manifest:

```bash
uv run python api_manifest.py --write
```

Run route contract tests after the manifest update.

### 4. Frontend Layout: Tighten Table, Widen Sidebar

Modify `index.html` Results tab.

Do not add a new table column.

Change the right sidebar from `width:200px` to a wider fixed width, suggested `260px` or `280px`.

Tighten numeric columns in the table. Prefer CSS classes or a `colgroup` instead of repeating more inline styles. A robust shape:

```html
<table class="results-table">
  <colgroup>
    <col class="results-col-job">
    <col class="results-col-function">
    <col class="results-col-deg">
    <col class="results-col-n">
    <col class="results-col-times">
    <col class="results-col-size">
  </colgroup>
  ...
</table>
```

Suggested widths:

- `Job ID`: `9rem`.
- `Function`: auto/flex remaining width.
- `Deg`: `3.5rem`.
- `N`: `5rem`.
- `Times`: `4rem`.
- `Size`: `5rem`.

If changing to CSS classes is too broad for this pass, keep the table structure and set explicit `width` on the `Deg`, `N`, `Times`, and `Size` `<th>` cells. The key is that `Deg` should not consume function-name/sidebar space.

Add a sidebar row near the existing compute metadata:

```html
<div><span style="color:#555">color renders:</span> <span id="res-color-renders">-</span></div>
```

Put it near `roots` / `solver`, not in the preview stats block. It describes the selected result, not the preview image.

### 5. Frontend State: Lazy Selected-Result Count

Modify `js/01-core-compute.js`.

Add state:

```javascript
const _resultColorRenderCountInFlight = new Map();
const RESULTS_COLOR_RENDER_COUNT_DELAY_MS = 350;
let _resultColorRenderCountTimer = null;
let _resultColorRenderCountJobId = null;
```

Store the count on each result row:

```javascript
r.color_render_count
r.color_artifact_count
r.legacy_color_artifact_count
r._colorRenderCountError
```

Add helpers:

```javascript
function _cancelPendingColorRenderCount(jobId = null) {
    if (!_resultColorRenderCountTimer) return;
    if (jobId != null && _resultColorRenderCountJobId !== jobId) return;
    clearTimeout(_resultColorRenderCountTimer);
    _resultColorRenderCountTimer = null;
    _resultColorRenderCountJobId = null;
}

function _setSelectedColorRenderCount(jobId, value) {
    if (_selectedJobId !== jobId) return;
    const el = document.getElementById('res-color-renders');
    if (el) el.textContent = value;
}

function _cachedColorRenderCountLabel(r) {
    if (!r) return '-';
    if (r._colorRenderCountError) return '?';
    if (r.color_render_count != null) return String(r.color_render_count);
    return '...';
}
```

Add an immediate loader:

```javascript
function _loadSelectedColorRenderCountNow(jobId) {
    const r = _resultsCache.find(row => row.job_id === jobId);
    if (!r) return;
    if (r.color_render_count != null || r._colorRenderCountError) {
        _setSelectedColorRenderCount(jobId, _cachedColorRenderCountLabel(r));
        return;
    }
    if (_resultColorRenderCountInFlight.has(jobId)) return;

    _setSelectedColorRenderCount(jobId, '...');
    const promise = lambdaPost('storage', { job_id: jobId }, '/render-count')
        .then(resp => {
            r.color_render_count = Number(resp.color_render_count || 0);
            r.color_artifact_count = Number(resp.color_artifact_count || 0);
            r.legacy_color_artifact_count = Number(resp.legacy_color_artifact_count || 0);
            r._colorRenderCountError = false;
            _setSelectedColorRenderCount(jobId, String(r.color_render_count));
        })
        .catch(() => {
            r._colorRenderCountError = true;
            _setSelectedColorRenderCount(jobId, '?');
        })
        .finally(() => {
            _resultColorRenderCountInFlight.delete(jobId);
        });
    _resultColorRenderCountInFlight.set(jobId, promise);
}
```

Add a debounced entrypoint:

```javascript
function _scheduleSelectedColorRenderCount(jobId) {
    _cancelPendingColorRenderCount();
    const r = _resultsCache.find(row => row.job_id === jobId);
    _setSelectedColorRenderCount(jobId, _cachedColorRenderCountLabel(r));
    if (!r || r.color_render_count != null || r._colorRenderCountError) return;
    if (_resultColorRenderCountInFlight.has(jobId)) return;

    _resultColorRenderCountJobId = jobId;
    _resultColorRenderCountTimer = setTimeout(() => {
        _resultColorRenderCountTimer = null;
        _resultColorRenderCountJobId = null;
        if (_selectedJobId !== jobId) return;
        _loadSelectedColorRenderCountNow(jobId);
    }, RESULTS_COLOR_RENDER_COUNT_DELAY_MS);
}
```

Call from `selectResult(jobId)`:

```javascript
_cancelPendingColorRenderCount();
document.getElementById('res-color-renders').textContent = '-';
...
const r = _resultsCache.find(r => r.job_id === jobId);
document.getElementById('res-color-renders').textContent = _cachedColorRenderCountLabel(r);
_scheduleSelectedColorRenderCount(jobId);
```

This mirrors the existing lazy preview debounce. It prevents arrow-key navigation from firing `/render-count` for every row briefly visited.

Guard every UI update with `_selectedJobId === jobId`, same as preview/detail code.

Do not block `/detail` rendering on this count. The count is a sidecar.

### 6. Refresh / Cache Semantics

When `loadResults()` replaces `_resultsCache`, old count cache disappears. That is fine.

When a render job creates a new color artifact, the selected result's cached count may be stale. Two options:

- Simple v1: `Refresh Results` clears cache and count refetches on selection.
- Slightly better: after render inventory refresh or color render completion, if `render-results-dir` matches a selected result, clear that row's `color_render_count` and call `_scheduleSelectedColorRenderCount(jobId)`.

Implement simple v1 first unless the stale count is annoying in practice.

### 7. Tests

Backend tests in `tests/test_storage_handler.py`:

- Add fake S3 objects:

```text
renders/job/color/color_a/image.jpeg
renders/job/color/color_b/image.png
renders/job/color/incomplete/preview.png
renders/job/image.jpeg
```

- Assert `/render-count` returns:

```json
{
  "color_artifact_count": 2,
  "legacy_color_artifact_count": 1,
  "color_render_count": 3
}
```

- Assert incomplete prefixes without `image.jpeg`/`image.png` are not counted.
- Assert missing `job_id` returns `400` through `_handle_storage_route`.

Route/manifest tests:

- `tests/test_deploy_packaging.py::test_storage_handler_routes_are_published_by_deploy` should pass after adding `/render-count` to `deploy_manifest.json`.
- `tests/test_api_route_contracts.py` should pass after regenerating `api_manifest.json`.
- `tests/test_storage_handler.py` is already in `scripts/predeploy_check.sh`, so the backend count assertions are predeploy-gated once added there.
- `deploy_manifest.py --check` and `api_manifest.py --check` are already part of predeploy; they will catch a route/manifest mismatch after `/render-count` is added.

Test fixture note:

- `_FakeS3` must support both `paginate(..., Delimiter="/")` returning `CommonPrefixes` and `head_object`, because the canonical inventory helpers use both. The current fake already has `head_object`; preserve that when extending the fixture data.

Frontend tests in `tests/test_frontend_js.sh`:

- Static assertions:
  - `id="res-color-renders"`.
  - `lambdaPost('storage', { job_id: jobId }, '/render-count')`.
  - `_resultColorRenderCountInFlight`.
  - `RESULTS_COLOR_RENDER_COUNT_DELAY_MS`.
  - `_cancelPendingColorRenderCount`.
  - `if (_selectedJobId !== jobId) return;` in the timer and setter paths.
- Runtime harness:
  - Selecting a result sets `Color renders` to `...`.
  - The route is not called until the debounce timer fires.
  - Selection change before the debounce fires cancels the pending route call.
  - The route is called once after the timer fires.
  - Response updates the selected sidebar to `3`.
  - Selecting away before response prevents stale UI update.
  - Reselecting a cached result does not issue a second route call.

### 8. Validation Commands

Use targeted commands first:

```bash
uv run python -m pytest tests/test_storage_handler.py tests/test_deploy_packaging.py tests/test_api_route_contracts.py -q
bash tests/test_frontend_js.sh
uv run python -m py_compile lambda/handler_storage.py
git diff --check
```

Before deployment, run the normal predeploy checklist command outside the known sandbox restriction path.

## Risks And Non-Goals

### Risks

- Counting reuses the canonical render inventory helper, so a selected job with many color artifacts does the same parallel HEAD checks the Render tab already does. This is acceptable for selected-only lazy loading and keeps the count in parity with the inventory.
- The count fetch must remain debounced/cancelable; otherwise arrow-key navigation can fire a storage request for every row visited.
- Count may be temporarily stale after a new render completes. Refreshing Results clears it. A later enhancement can invalidate the selected row after render completion.
- Legacy color counting may include old root-level renders. This is intentional if the label is `Color renders` and should match what the Render tab can show.

### Non-Goals

- Do not add a `Render#` table column in this pass.
- Do not count render artifacts during `/list`.
- Do not count bilevel, coeff, palette, or PDF artifacts.
- Do not recursively scan all keys under every `renders/<job>/` prefix.

## Verdict

Implement sidebar-only lazy color render counts. It provides the information needed without widening the table or making Results refresh scale with the number of saved jobs times render artifacts.
