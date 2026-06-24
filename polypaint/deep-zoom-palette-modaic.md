# AllCol / AllPal Mosaic Implementation Plan

## Objective

Rename the existing `AllRenders` tab to `AllCol`, then add an `AllPal` tab that gives the same single-level OpenSeadragon wall for saved palette artifacts.

The important constraint is that this remains a manifest-of-existing-thumbnails feature. It must not build a stitched image or a deep-zoom pyramid. Refresh crawls S3, writes a JSON manifest, and the browser uses OpenSeadragon as a large pan/zoom contact sheet over existing preview images.

## Current State

### Existing Color Mosaic

The current color wall is implemented as:

- UI tab: `AllRenders` in `index.html`.
- DOM ids: `allrenders-*`.
- JS module: `js/13-allrenders.js`.
- API route: `/list-color-mosaic`.
- Storage handler entry points:
  - `handle_list_color_mosaic`
  - `_start_color_mosaic_refresh`
  - `_run_color_mosaic_worker`
  - `_build_color_mosaic_manifest`
- Status row:
  - `job_id = "__allrenders_mosaic__"`
  - `task_id = "color_mosaic_status"`
- Manifest prefix:
  - `renders/_index/color_mosaic/<refresh_id>/all.json`

The implementation is already async and should be reused:

- Refresh writes a DDB status row.
- Refresh self-invokes the storage Lambda with an internal build action.
- The tab polls status every two seconds.
- Worker writes progress fields:
  - `progress_stage`
  - `progress_message`
  - `progress_jobs_done`
  - `progress_jobs_total`
  - `progress_artifacts_done`
  - `progress_artifacts_total`
  - `progress_tiles`
  - `progress_last_job`
- Worker writes a public manifest URL when ready.
- Old refresh manifests are pruned.

Do not replace this with a synchronous API crawl.

The color mosaic has already been hardened and that baseline should be preserved during genericization:

- Transient frontend status-poll failures reschedule polling while a refresh is computing instead of stopping forever.
- Frontend logs are capped so the fallback log DOM does not grow without bound.
- Click mapping uses the active OpenSeadragon tile source's stored columns/tile size, not recomputed controls.
- Backend job-id and refresh-id parsing is centralized in helpers; do not add new depth-dependent `split("/")` parsing.
- Missing S3 objects are treated as expected skips, but non-missing S3 errors such as throttles/5xx propagate to the worker error path instead of silently degrading manifest metadata. Malformed optional `calc.json` is the exception: it should default compute metadata to `?`/`0` for that job because the wall does not depend on it.
- Color manifests already include `manifest_type`, `artifact_kind`, `sizes`, and `size_counts`; keep the palette manifest symmetric with that shape.
- Progress cadence is named (`MOSAIC_PROGRESS_JOB_INTERVAL`, `MOSAIC_PROGRESS_ARTIFACT_INTERVAL`) and progress writes refresh `updated_at_ms` as the worker heartbeat.

### Existing Palette Inventory

Palettes are not stored through `RENDER_FAMILY_SHAPES` the same way immutable color artifacts are. They already have separate inventory code:

- Palette artifacts live under:
  - `renders/<job_id>/palettes/<palette_id>/`
- Existing list route:
  - `/list-palettes`
- Existing storage helper:
  - `_list_saved_palettes(job_id)`
- Existing ordering helper:
  - `_order_palette_variants(variants)`
- Existing Render tab family:
  - `palette`
- Existing Render tab artifact selection:
  - `refreshRenderArtifacts(jobId, { selectFamily: "palette", selectArtifactId: paletteId })`

That means `AllPal` should plug into the palette inventory path, not force palettes into the color `RENDER_FAMILY_SHAPES` model.

## Design Decisions

### Naming

User-facing names:

- `AllRenders` becomes `AllCol`.
- New palette wall is `AllPal`.

Internal route names:

- Keep `/list-color-mosaic` for color. It already exists and is deployed/tested.
- Add `/list-palette-mosaic` for palettes.

Internal DDB key:

- Keep `job_id = "__allrenders_mosaic__"` for compatibility with the existing color status row.
- Split only by `task_id`:
  - `color_mosaic_status`
  - `palette_mosaic_status`

This avoids losing the current color manifest/status state while still separating Color and Palette refreshes.

### Module Shape

Refactor the frontend from one hardcoded `AllRenders` controller into a generic artifact mosaic controller.

Optional file rename:

- `js/13-allrenders.js` -> `js/13-artifact-mosaics.js`

Part registration becomes:

```js
;(window.__ppParts = window.__ppParts || []).push('13-artifact-mosaics');
```

This rename is cleaner, but it is not required for functionality. If reducing churn is more important, keep `js/13-allrenders.js`, genericize it internally, and leave the script include/parts registration alone. The user-visible rename is the tab label and function names, not necessarily the physical file name.

Expose small wrappers:

```js
function loadAllCol(opts) { return _loadArtifactMosaic('color', opts); }
function refreshAllColMosaic() { return _refreshArtifactMosaic('color'); }
function homeAllCol() { return _homeArtifactMosaic('color'); }
function _allColRebuild() { return _rebuildArtifactMosaic('color'); }

function loadAllPal(opts) { return _loadArtifactMosaic('palette', opts); }
function refreshAllPalMosaic() { return _refreshArtifactMosaic('palette'); }
function homeAllPal() { return _homeArtifactMosaic('palette'); }
function _allPalRebuild() { return _rebuildArtifactMosaic('palette'); }
```

Do not copy/paste the existing `AllRenders` controller into a second file. The state, polling, logging, OSD setup, tile-source generation, and click handling are the same shape.

Use separate OpenSeadragon viewer instances for `AllCol`, `AllPal`, and `DeepZoom`. The simplest v1 lifecycle is to keep each mosaic viewer alive after it is first opened; OSD only keeps visible tiles resident, so this is acceptable for current scale. If memory becomes visible in browser testing, add an explicit tab-switch teardown for the inactive mosaic viewer, but do not share `_osdViewer` with DeepZoom.

### Size Semantics

`AllCol` size filter:

- Static choices:
  - `All`
  - `512`
  - `1024`
- Filter by actual `preview_width/preview_height`.
- Tile geometry must use the actual selected tile size. This is what fixed the old 1024 upper-left-quarter bug.
- In `All` mode, mixed 512 and 1024 color previews still use one uniform OpenSeadragon cell size. The current implementation uses the largest observed preview size, so 512 previews appear enlarged in 1024 cells. That is acceptable for `All`; exact-size filters are the crisp path.

`AllPal` size filter:

- Dynamic choices:
  - `All`
  - every distinct preview PNG size observed in the manifest
- This is a preview wall, not an actual full-palette wall. Compute `N` is irrelevant for the size dropdown.
- Filter by actual `preview_width/preview_height`.
- In practice, most palette previews are 512px. The Size control is therefore a preview-size diagnostic/filter, not the primary palette organization axis. If the manifest has only one observed preview size, disable the dropdown or leave it as `All` plus the single size; do not spend UI complexity on it.

Tile geometry is based on actual preview dimensions:

- Store `preview_width` and `preview_height` from the PNG header when available.
- For exact-size palette filters, use the selected preview size as tile geometry.
- If PNG dimensions are unavailable, fall back to `512` and increment `unknown_dimensions`.

This keeps `AllPal` honest: the wall displays thumbnails, so the filter groups thumbnails by thumbnail size.

The manifest should carry `metric` and `palette` because they are the more meaningful future palette grouping axes. Generic compute fields such as degree and `N` may still be present because the builder already reads `calc.json`; they can be used for sorting, but AllPal must not use `N` for size filtering or tile geometry.

## Backend Implementation Steps

### 1. Generalize Mosaic Status Helpers

Refactor the existing fixed color helpers so they accept `kind` or a config object.

Add config:

```python
MOSAIC_STATUS_JOB_ID = "__allrenders_mosaic__"
MOSAIC_TASK_IDS = {
    "color": "color_mosaic_status",
    "palette": "palette_mosaic_status",
}
MOSAIC_PREFIXES = {
    "color": "renders/_index/color_mosaic/",
    "palette": "renders/_index/palette_mosaic/",
}
MOSAIC_INTERNAL_ACTIONS = {
    "color": "build_color_mosaic",
    "palette": "build_palette_mosaic",
}
```

Keep these existing generic helpers and use them everywhere:

```python
def _mosaic_job_id_from_prefix(prefix):
    ...

def _mosaic_refresh_id_from_manifest_key(key_or_prefix):
    ...
```

Do not introduce new ad-hoc key-depth parsing such as `split("/")[1]`, `split("/")[-2]`, or frontend `slice(-2, -1)`. The color wall already had bugs in that class; genericization is the chance to keep the parser centralized.

Update these helpers and entry-point bodies to accept `kind`:

- `_mosaic_status_item(kind, status)`
- `_mosaic_status_from_item(item)`
- `_read_mosaic_status(kind, consistent=True)`
- `_put_mosaic_status(kind, status, ...)`
- `_put_owned_mosaic_status(kind, status, refresh_id)`
- `_start_mosaic_refresh(kind)`
- `_ready_mosaic_status(kind, refresh_id, manifest, existing)`
- `_error_mosaic_status(kind, refresh_id, existing, exc)`
- `_progress_mosaic_status(kind, refresh_id, existing, ...)`
- `_prune_mosaic_manifests(kind, keep_refresh_ids=...)`
- `_mosaic_job_id_from_prefix(prefix)`
- `_mosaic_refresh_id_from_manifest_key(key_or_prefix)`
- `_handle_list_mosaic(event, kind)`
- `_run_mosaic_worker(kind, refresh_id)`

Keep wrappers for clarity:

```python
def _start_color_mosaic_refresh():
    return _start_mosaic_refresh("color")

def _start_palette_mosaic_refresh():
    return _start_mosaic_refresh("palette")

def handle_list_color_mosaic(event):
    return _handle_list_mosaic(event, "color")

def handle_list_palette_mosaic(event):
    return _handle_list_mosaic(event, "palette")
```

The existing color route must continue to return the same response shape, but it should now run through `_handle_list_mosaic(event, "color")`. Likewise, the existing internal color action should run through `_run_mosaic_worker("color", refresh_id)`. This is the real genericization checkpoint: color and palette must share the route body and worker body, with only thin public wrappers left.

Target shape:

```python
def _handle_list_mosaic(event, kind):
    params = parse_body(event)
    refresh = parse_boolish(params.get("refresh"), False)
    if refresh:
        return ok_response(_start_mosaic_refresh(kind))
    return ok_response(_read_mosaic_status(kind, consistent=True))


def _run_mosaic_worker(kind, refresh_id):
    existing = _read_mosaic_status(kind, consistent=True)

    def publish_progress(**kwargs):
        _put_owned_mosaic_status(
            kind,
            _progress_mosaic_status(kind, refresh_id, existing, **kwargs),
            refresh_id,
        )

    try:
        manifest = _build_mosaic_manifest(kind, refresh_id, progress_cb=publish_progress)
        s3.put_object(
            Bucket=BUCKET,
            Key=manifest["manifest_key"],
            Body=json.dumps(manifest, separators=(",", ":"), ensure_ascii=False).encode("utf-8"),
            ContentType="application/json",
            CacheControl="no-cache, max-age=0",
        )
        status = _ready_mosaic_status(kind, refresh_id, manifest, existing)
        _put_owned_mosaic_status(kind, status, refresh_id)
        previous_refresh_id = _mosaic_refresh_id_from_manifest_key(
            existing.get("last_ready_manifest_key") or existing.get("manifest_key") or ""
        )
        keep = {refresh_id}
        if previous_refresh_id:
            keep.add(previous_refresh_id)
        _prune_mosaic_manifests(kind, keep_refresh_ids=keep)
        return status
    except Exception as exc:
        error_status = _error_mosaic_status(kind, refresh_id, existing, exc)
        try:
            _put_owned_mosaic_status(kind, error_status, refresh_id)
        except ClientError as put_exc:
            if not _is_conditional_failure(put_exc):
                raise
        return error_status
```

The code above is still illustrative; preserve the current color worker's exact conditional-failure behavior when porting. The point is structural: one handler body and one worker body serve both kinds.

Progress writes must remain throttled with the existing named constants, not magic inline `% 10` / `% 25` checks. The current color worker reports every batch of jobs/artifacts, not once per artifact. Preserve that cadence for palette; each progress event is a conditional DynamoDB `put_item`, so per-artifact progress would become write-noise during large refreshes. Each throttled progress write must also refresh `updated_at_ms`; that timestamp is the active-lock heartbeat that prevents another refresh from treating the worker as stale during a long crawl.

### 2. Keep the Existing Color Builder Working

Rename the implementation function if useful, but preserve behavior:

```python
def _build_color_mosaic_manifest(refresh_id, *, progress_cb=None):
    ...
```

or:

```python
def _build_mosaic_manifest(kind, refresh_id, *, progress_cb=None):
    if kind == "color":
        return _build_color_mosaic_manifest(refresh_id, progress_cb=progress_cb)
    if kind == "palette":
        return _build_palette_mosaic_manifest(refresh_id, progress_cb=progress_cb)
```

Regression gate: existing color mosaic tests must pass unchanged except for expected UI names changing from `AllRenders` to `AllCol`.

### 3. Add Palette Artifact Scanner

Do not call `_list_saved_palettes(job_id)` directly from the mosaic builder as-is, because it currently generates presigned URLs and uses the module-level `s3` client. The mosaic worker should not mint throwaway presigned URLs.

Add lower-level helpers:

```python
def _palette_artifact_prefixes(job_id, *, s3_client=None):
    ...

def _palette_entry_from_prefix(job_id, prefix, *, presign=True, s3_client=None):
    ...
```

Then refactor `_list_saved_palettes(job_id)` to use those helpers with `presign=True`.

The mosaic worker uses:

```python
_palette_entry_from_prefix(job_id, prefix, presign=False, s3_client=client)
```

Palette entry requirements:

- `palette_id`
- `artifact_id`
- `image_key`
- `preview_key`
- `created_at`
- `metric`
- `palette`
- `render_reusable`
- `data_layout`
- `solve_score_program_source_text`, if present in metadata
- `solve_score_program_spec`, if present
- `color_interpretation`, if present
- `score_output_channel_count`, if present
- parent/derived fields, if present

Missing `meta.json` means skip the palette artifact. Missing image means skip. Missing preview means skip unless the image itself is safe to use as the tile source; v1 should skip to match color wall behavior.

Some older palette `meta.json` files may not contain every field in the list above. Treat `metric`, `palette`, `render_reusable`, `data_layout`, and score/source fields as optional metadata, not hard validation requirements.

### 4. Build Palette Manifest

Add:

```python
def _build_palette_mosaic_manifest(refresh_id, *, progress_cb=None):
    ...
```

Use the same two-phase scan shape as color:

1. List job ids from `renders/`.
2. Read `calc.json` once per job with `_read_mosaic_calc_meta`.
3. List palette prefixes per job.
4. Flatten work items into `(job_id, prefix)`.
5. Read each palette artifact in one worker pool.
6. Read preview PNG dimensions with `_mosaic_preview_dimensions`.
7. Produce manifest JSON.

Do not use nested `ThreadPoolExecutor`s.

Each tile should include:

```json
{
  "key": "renders/<job>/palettes/<palette>/preview.png",
  "job_id": "compute_...",
  "artifact_id": "pal_...",
  "palette_id": "pal_...",
  "created_at": "...",
  "function": "...",
  "degree": 35,
  "N": 1024,
  "times": 1,
  "preview_width": 512,
  "preview_height": 512,
  "image_key": "renders/<job>/palettes/<palette>/image.jpeg",
  "metric": "...",
  "palette": "...",
  "render_reusable": true,
  "data_layout": "..."
}
```

Manifest should include:

```json
{
  "schema_version": 1,
  "manifest_type": "artifact_mosaic",
  "artifact_kind": "palette",
  "computed_at": "...",
  "base": "https://polypaint.s3.us-east-1.amazonaws.com/",
  "refresh_id": "...",
  "manifest_key": "renders/_index/palette_mosaic/<refresh_id>/all.json",
  "manifest_kind": "all",
  "dimension_filter": "preview-size",
  "count": 123,
  "sizes": [512],
  "size_counts": {"512": 123},
  "source_counts": {"512x512": 123},
  "skipped_missing_preview": 0,
  "skipped_missing_image": 0,
  "skipped_missing_meta": 0,
  "unknown_dimensions": 0,
  "tiles": []
}
```

The existing color manifest already has `manifest_type`, `artifact_kind`, `sizes`, and `size_counts`. Palette should emit the same high-level fields so the generic frontend can read one manifest shape for both kinds.

Do not derive `sizes` / `size_counts` by parsing `source_counts` strings. Aggregate size counts directly from integer `preview_width` / `preview_height` while processing tiles:

```python
if width and width == height:
    size_counts[width] = size_counts.get(width, 0) + 1
```

Then emit:

```python
"sizes": sorted(size_counts),
"size_counts": {str(k): v for k, v in sorted(size_counts.items())},
```

Do not include `0` or unknown dimensions in `sizes`. If preview dimensions cannot be read, keep the tile in `All`, increment `unknown_dimensions`, and exclude it from exact-size filters.

Do not silently swallow transient S3 failures while building this manifest. Missing `meta.json`, missing image, and missing preview are expected artifact states and should increment skip counters. Other `ClientError`s or preview/meta decode failures should either be retried deliberately or fail the worker into `state:"error"` so the status UI shows the problem. Malformed `calc.json` should not fail the worker; default that job's compute metadata and continue.

Sort manifest tiles by:

1. `created_at` descending
2. `job_id`
3. `palette_id`

### 5. Route Internal Worker Actions Through the Generic Worker

Update the storage Lambda internal-action branch so both color and palette use `_run_mosaic_worker`:

```python
if event.get("internal_action") == "build_color_mosaic":
    return _run_mosaic_worker("color", event.get("refresh_id"))
if event.get("internal_action") == "build_palette_mosaic":
    return _run_mosaic_worker("palette", event.get("refresh_id"))
```

Keep the old color action name for deployed/in-flight compatibility.

### 6. Add Palette API Route

Add the palette wrapper:

```python
def handle_list_palette_mosaic(event):
    return _handle_list_mosaic(event, "palette")
```

Also convert the existing color wrapper, if not already done:

```python
def handle_list_color_mosaic(event):
    return _handle_list_mosaic(event, "color")
```

Route dispatch:

```python
elif path.endswith("/list-palette-mosaic"):
    return _handle_storage_route(handle_list_palette_mosaic, event)
```

This must use `_handle_storage_route`, matching `/list-color-mosaic` and the other storage routes. Otherwise `ValueError`, `ClientError`, and unexpected exceptions bypass the normal structured 400/404/500 response mapping.

Update `deploy_manifest.json` storage routes:

- add `/list-palette-mosaic`

Regenerate `api_manifest.json` with the repo Python environment.

## Frontend Implementation Steps

### 1. Rename the Tab

In `index.html`, replace:

```html
<div class="tab-btn" onclick="switchTab('allrenders')">AllRenders</div>
```

with:

```html
<div class="tab-btn" onclick="switchTab('allcol')">AllCol</div>
<div class="tab-btn" onclick="switchTab('allpal')">AllPal</div>
```

This must use `AllCol`/`allcol` and `AllPal`/`allpal` exactly. `switchTab` marks the active tab by comparing `button.textContent.toLowerCase()` with the tab name, so labels with spaces would break active-state styling.

### 2. Rename the Color Panel

Rename the current panel:

- `tab-allrenders` -> `tab-allcol`
- `allrenders-*` ids -> `allcol-*`
- `refreshAllRendersMosaic()` -> `refreshAllColMosaic()`
- `homeAllRenders()` -> `homeAllCol()`
- `_allRendersRebuild()` -> `_allColRebuild()`

Visible strings:

- `AllRenders ready` -> `AllCol ready`
- `Computing AllRenders mosaic` -> `Computing AllCol mosaic`
- `AllRenders load failed` -> `AllCol load failed`

Regression gate: after this rename and before any palette backend work, run the existing color mosaic frontend tests and verify `/list-color-mosaic` still loads and opens the wall. The rename touches deployed working code, so catch stale `allrenders-*` references before adding `AllPal`.

### 3. Add Palette Panel

Add a sibling panel:

```html
<div id="tab-allpal" class="tab-content">
  <div style="display:flex; gap:8px; align-items:center; flex-wrap:wrap; margin-bottom:8px">
    <button class="btn-secondary btn-inline" id="btn-allpal-refresh" onclick="refreshAllPalMosaic()" style="padding:4px 12px; font-size:12px">Refresh</button>
    <button class="btn-secondary btn-inline" id="btn-allpal-home" onclick="homeAllPal()" style="padding:4px 12px; font-size:12px">Home</button>
    <label style="font-size:11px; color:#aaa">Size</label>
    <select id="allpal-size-filter" onchange="_allPalRebuild()" style="width:90px">
      <option value="all">All</option>
    </select>
    <label style="font-size:11px; color:#aaa">Sort</label>
    <select id="allpal-sort-mode" onchange="_allPalRebuild()" style="width:120px">
      <option value="date">Date</option>
      <option value="job">Job</option>
      <option value="function">Function</option>
      <option value="degree">Degree</option>
      <option value="N">N</option>
      <option value="random">Random</option>
    </select>
    <label style="font-size:11px; color:#aaa">Cols</label>
    <input id="allpal-cols" type="number" min="1" step="1" placeholder="auto" onchange="_allPalRebuild()" style="width:80px">
    <span id="allpal-summary" style="font-size:11px; color:#888"></span>
  </div>
  <div id="allpal-status" class="status" style="margin-bottom:6px">Refresh to build the palette wall.</div>
  <div id="allpal-viewer" style="width:100%; height:68vh; background:#050505; border:1px solid #333; border-radius:4px; display:block"></div>
  <div id="allpal-log" style="margin-top:8px; font-family:monospace; font-size:11px"></div>
</div>
```

### 4. Update `switchTab`

In `js/01-core-compute.js`:

```js
if (name === 'allcol') loadAllCol();
if (name === 'allpal') loadAllPal();
```

Remove the old `allrenders` branch once the panel is renamed.

### 5. Generic Mosaic Controller

Replace hardcoded `AllRenders` state with per-kind state.

Config:

```js
const ARTIFACT_MOSAICS = {
    color: {
        tabName: 'allcol',
        label: 'AllCol',
        statusPath: '/list-color-mosaic',
        family: 'color',
        sizeMode: 'fixed-preview',
        fixedSizes: ['512', '1024'],
        selectArtifactId(tile) { return tile.artifact_id; },
    },
    palette: {
        tabName: 'allpal',
        label: 'AllPal',
        statusPath: '/list-palette-mosaic',
        family: 'palette',
        sizeMode: 'manifest-palette-size',
        fixedSizes: [],
        selectArtifactId(tile) { return tile.palette_id || tile.artifact_id; },
    },
};
```

State:

```js
const _artifactMosaicState = {
    color: { viewer:null, status:null, manifest:null, tiles:[], pollTimer:null, loading:false, randomSeed:1, lastRenderSignature:'', activeTileSource:null, lastLogSignature:'' },
    palette: { viewer:null, status:null, manifest:null, tiles:[], pollTimer:null, loading:false, randomSeed:1, lastRenderSignature:'', activeTileSource:null, lastLogSignature:'' },
};
```

Every helper should take `kind`:

- `_mosaicEl(kind, suffix)`
- `_setMosaicStatus(kind, text, cls)`
- `_setMosaicRefreshBusy(kind, busy)`
- `_mosaicProgressText(kind, status)`
- `_logMosaic(kind, message, cls, signature)`
- `_fetchMosaicStatus(kind)`
- `_fetchMosaicManifest(url)`
- `_selectedMosaicSize(kind)`
- `_mosaicSortMode(kind)`
- `_mosaicRequestedCols(kind, count)`
- `_mosaicFilteredSortedTiles(kind)`
- `_ensureMosaicViewer(kind)`
- `_mosaicTileSource(kind, tiles)`
- `_mosaicTileSize(kind, tiles)`
- `_tileFromMosaicClick(kind, event)`
- `_rebuildArtifactMosaic(kind)`
- `_loadArtifactMosaic(kind, opts)`
- `_refreshArtifactMosaic(kind)`
- `_homeArtifactMosaic(kind)`

`_tileFromMosaicClick(kind, event)` must use the active tile source's stored column count and tile size. Do not recompute columns from the current controls during click handling; a rebuild or filter change can otherwise make click-to-tile mapping disagree with the source OSD is currently displaying.

### 6. Dynamic Palette Size Dropdown

When loading an `AllPal` manifest, populate `#allpal-size-filter` from `manifest.sizes`.

Rules:

- Preserve the current selected value if still present.
- Always include `All`.
- Add numeric sizes sorted ascending.
- Use labels like `512`, `1024`, `2048`.
- If no sizes are present, leave only `All`.
- Exclude null/zero/unknown sizes from the dropdown. Unknown-size palettes stay visible in `All` only.

Color keeps static `512` and `1024`.

### 7. Filtering and Tile Size Rules

Color:

```js
if (kind === 'color' && size !== 'all') {
    const n = Number(size);
    tiles = tiles.filter(t => Number(t.preview_width) === n && Number(t.preview_height) === n);
}
```

Palette:

```js
if (kind === 'palette' && size !== 'all') {
    const n = Number(size);
    tiles = tiles.filter(t => Number(t.preview_width) === n && Number(t.preview_height) === n);
}
```

Tile size:

```js
function _mosaicTileSize(kind, tiles) {
    const size = _selectedMosaicSize(kind);
    const previewSizes = tiles
        .map(t => Number(t.preview_width))
        .filter(n => Number.isFinite(n) && n > 0);
    if (previewSizes.length) return Math.max(...previewSizes);
    if (kind === 'palette' && size !== 'all') return Number(size) || 512;
    if (kind === 'color' && (size === '512' || size === '1024')) return Number(size);
    return 512;
}
```

This avoids both bad cases:

- A 1024 image rendered with `tileSize=512` only shows the upper-left quarter.
- Palette filters use the preview size itself, so compute `N` cannot leak into tile geometry.

### 8. Click Behavior

The click handler should be generic and use the per-kind config:

```js
async function _artifactMosaicCanvasClick(kind, event) {
    const cfg = ARTIFACT_MOSAICS[kind];
    const state = _artifactMosaicState[kind];
    const tile = _tileFromMosaicClick(kind, event);
    if (!cfg || !state || !tile || !tile.job_id) return;
    const artifactId = cfg.selectArtifactId(tile);
    if (!artifactId) return;
    await _ensureResultsSelection(tile.job_id);
    switchTab('render');
    await refreshRenderArtifacts(tile.job_id, {
        selectFamily: cfg.family,
        selectArtifactId: artifactId,
    });
}
```

For `AllCol`, `cfg.family` is `color`. For `AllPal`, `cfg.family` is `palette`, so clicking a palette moves to Render, opens the Palette subtab, and selects the specific palette artifact. Do not maintain separate copy/pasted click handlers.

### 9. Script Registration

If the file is renamed:

- Replace script include:
  - `js/13-allrenders.js`
  - with `js/13-artifact-mosaics.js`
- Update the expected parts list in `index.html`:
  - `13-allrenders`
  - to `13-artifact-mosaics`
- Update `tests/test_frontend_parts_contract.py` expectations if needed.
- Update `tests/test_frontend_js.sh` to load the new file.

## Test Plan

### Backend Tests

Add/extend storage handler tests:

1. Existing color mosaic route still works.
2. `/list-color-mosaic` still returns the existing ready/computing/error status shape.
3. `/list-palette-mosaic` with `{refresh:true}` creates `palette_mosaic_status`, not `color_mosaic_status`.
4. Palette worker writes manifest under:
   - `renders/_index/palette_mosaic/<refresh_id>/all.json`
5. Palette manifest includes `sizes` and `size_counts` from actual preview PNG dimensions.
6. Palette manifest tile includes `preview_width`, `preview_height`, `palette_id`, `job_id`, `image_key`, `key`, and informational compute fields such as `N`.
7. Missing `meta.json` skips a palette and increments `skipped_missing_meta`.
8. Missing preview skips a palette and increments `skipped_missing_preview`.
9. Missing/unreadable preview dimensions do not create a literal `0` dropdown entry; tile remains in `All`, increments `unknown_dimensions`, and is excluded from exact-size filters.
10. Manifest pruning is kind-scoped: pruning palette manifests must not delete color manifests.
11. Existing render-summary and list-palettes tests still pass.

Fake S3 requirements:

- `get_paginator("list_objects_v2")` must support `Delimiter="/"`.
- `get_object(..., Range="bytes=0-32")` must return a PNG header for preview dimension tests.
- `head_object` must exist for image/preview validation.

### Frontend Tests

Extend `tests/test_frontend_js.sh`:

1. `AllCol` loads `/list-color-mosaic`, opens an OSD tile source, and logs `AllCol`.
2. `AllCol` exact `1024` filter uses `tileSize=1024`.
3. `AllCol` transient status-poll failure while computing reschedules polling and leaves the refresh button busy.
4. `AllCol` click mapping uses the active tile source's stored columns/tile size.
5. `AllPal` loads `/list-palette-mosaic`.
6. `AllPal` populates the size dropdown from `manifest.sizes`.
7. `AllPal` exact-size filter filters by `preview_width/preview_height`.
8. `AllPal` with only one observed preview size disables the size dropdown or leaves it as an inert `All`/single-size control.
9. `AllPal` `N` sort uses copied compute metadata but does not affect size filtering or tile geometry.
10. `AllPal` tile source uses actual preview dimensions.
11. `AllPal` click calls `refreshRenderArtifacts(jobId, { selectFamily:"palette", selectArtifactId: paletteId })`.
12. Switching tabs does not destroy the DeepZoom viewer or the other mosaic viewer.
13. Status polling/progress text works independently for color and palette.

Update frontend parts contract:

- The new script must be in the sequential load list.
- The new part name must appear in `window.__ppParts`.

### API/Deploy Tests

Update:

- `deploy_manifest.json`
- `api_manifest.json`

Predeploy should catch:

- `/list-palette-mosaic` missing from route manifest.
- New JS file missing from frontend asset list.
- Storage handler imports missing from deploy bundle.

## Implementation Order

0. Confirm the hardened color baseline remains green: transient poll retry, centralized key parsing, previous-manifest prune keep, fail-visible S3 errors, symmetric color manifest fields.
1. Rename visible `AllRenders` tab and DOM ids to `AllCol`.
2. Refactor `js/13-allrenders.js` into a generic artifact mosaic controller. Rename the file only if the extra include/parts churn is worth it.
3. Keep color behavior passing with only name changes. This is a hard checkpoint before palette work.
4. Generalize backend mosaic status helpers, `_handle_list_mosaic`, and `_run_mosaic_worker` by `kind`.
5. Move `/list-color-mosaic` and `build_color_mosaic` onto the generic backend path and keep them passing before adding palette behavior.
6. Add low-level palette prefix/entry helpers and refactor `_list_saved_palettes` to use them.
7. Add `_build_palette_mosaic_manifest`.
8. Add palette internal worker action.
9. Add `/list-palette-mosaic` route and manifest entries.
10. Add `AllPal` panel and JS config.
11. Add dynamic palette size dropdown.
12. Add palette click-through selection.
13. Add backend tests.
14. Add frontend tests.
15. Run the targeted storage/frontend tests.
16. Run predeploy before deployment.

## Expected Risk

This is straightforward because the hard part already exists in the color mosaic:

- async refresh
- DDB status
- progress polling
- manifest writing
- OSD single-level tile source
- click-through into Render tab

The real risks are narrow:

1. Accidentally duplicating the color mosaic instead of genericizing it.
2. Letting compute `N` leak into AllPal size filtering or tile geometry even though the wall displays preview PNGs.
3. Calling `_list_saved_palettes` directly and generating thousands of unused presigned URLs.
4. Sharing one OSD viewer between `AllCol`, `AllPal`, and `DeepZoom`.
5. Forgetting route/script registration, leaving the feature present locally but absent in deploy.
6. Letting the UI rename break the already-shipped color wall because a stale `allrenders-*` reference was missed.
7. Reintroducing brittle key-depth parsing or poll-terminal behavior that was already fixed in the color baseline.

If the implementation follows the steps above, this should be a low-to-medium risk feature, not a redesign.
