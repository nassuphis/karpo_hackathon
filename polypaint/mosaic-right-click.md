# Mosaic Right-Click Menu Plan

## Objective

Add a right-click context menu to the `AllCol` and `AllPal` OpenSeadragon mosaics.

The menu should act on the tile under the cursor, using the same tile-coordinate mapping as the current left-click navigation. It must not require building a stitched image or a deep-zoom pyramid. It operates from the existing mosaic manifest tile data plus on-demand storage calls where needed.

Primary actions requested:

- `Go Render`
- `Go Compute`
- `Go Result`
- `Favorite`
- `Download`
- `Copy Link`

## Current State

`js/13-artifact-mosaics.js` already has the critical primitive:

```js
_tileFromMosaicClick(kind, event)
```

It maps an OpenSeadragon click event to the corresponding manifest tile using the active tile source's stored `cols` and `tileSize`. Left-click then calls `_artifactMosaicCanvasClick(kind, event)`, which opens the Render tab and selects the color/palette artifact.

The mosaic manifest tiles already include:

- `job_id`
- `artifact_id`
- `palette_id` for palettes
- `key` for the preview image
- `image_key`
- `created_at`
- `function`
- `degree`
- `N`
- `times`
- `preview_width`
- `preview_height`
- palette-only fields such as `metric`, `palette`, `render_reusable`, `data_layout`, `color_interpretation`, `score_output_channel_count`

The render/favorite/download primitives already exist elsewhere:

- Render selection: `refreshRenderArtifacts(jobId, { selectFamily, selectArtifactId })`
- Result selection: `_ensureResultsSelection(jobId)` and `selectResult(jobId)`
- Render job input fallback: `_setRenderResultsJob(jobId)`
- Favorite selected color artifact: `favoriteSelectedRenderArtifact()`, currently selection-dependent
- Browser download helpers: `downloadPresignedFile(...)`, `_downloadStorageKeyBlob(...)`, `_downloadBlob(...)`; `_dlMenuAction(...)` is selection-dependent
- Presigned download flow: `/presign`

## Implementation Rule

Right-click actions must not call button handlers that depend on global UI selection as their source of truth.

Avoid this pattern:

```js
await selectSomethingInAnotherTab(...);
await existingSelectedThingButtonHandler();
```

Use this pattern instead:

```js
await commandHelper({ jobId, artifactId, imageKey, previewKey });
```

Then existing buttons can become thin wrappers around the same command helper. This is the main guard against flaky cross-tab state bugs.

Do not call these from the context menu:

- `favoriteSelectedRenderArtifact()` — reads current Render selection
- `populateSelectedResult()` — reads current Results selection
- `_dlMenuAction(...)` — reads current Render selection and menu state

Extract command helpers from them where needed, then have both the old button and the new context menu call the extracted helper.

Acceptable exceptions:

- `Go Result` specifically means selecting a Results row.
- `Go Render` is a navigation action, but it should select Render directly by `job_id + artifact_id`; it should not require a Results row.
- `Go Compute` should not depend on Results-row selection; it should load `/detail` by `job_id`.

## Action Semantics

### Go Render

Opens the Render tab and selects the exact artifact.

For `AllCol`:

```js
refreshRenderArtifacts(tile.job_id, {
  selectFamily: 'color',
  selectArtifactId: tile.artifact_id,
})
```

For `AllPal`:

```js
refreshRenderArtifacts(tile.job_id, {
  selectFamily: 'palette',
  selectArtifactId: tile.palette_id || tile.artifact_id,
})
```

Implementation should not depend on `_ensureResultsSelection(job_id)`.

Use a direct Render navigation helper:

```js
async function _goMosaicTileRender(kind, tile) {
  const cfg = _mosaicConfig(kind);
  const artifactId = _mosaicArtifactId(kind, tile);
  if (!tile || !tile.job_id || !artifactId) throw new Error('Missing mosaic tile target');
  if (typeof _setRenderResultsJob === 'function') {
    _setRenderResultsJob(tile.job_id);
  } else {
    const el = document.getElementById('render-results-dir');
    if (el) el.value = tile.job_id;
  }
  switchTab('render');
  await refreshRenderArtifacts(tile.job_id, {
    selectFamily: cfg.family,
    selectArtifactId: artifactId,
  });
  const selected = typeof _renderSelectedArtifactEntry === 'function' ? _renderSelectedArtifactEntry() : null;
  const selectedId = selected && (selected.palette_id || selected.artifact_id);
  if (selectedId !== artifactId) {
    throw new Error(`${artifactId} was not found in Render ${cfg.family}`);
  }
}
```

Left-click and right-click `Go Render` should both call this helper. If you still want the Results row highlighted as a courtesy, do it best-effort after or in parallel and never let it block Render navigation.

### Go Compute

Switches to the Compute tab and loads/populates the selected result's compute configuration.

This is not the same as selecting the Results row. It should be treated as a convenience action for "take me back to the calculation inputs for this job."

Implementation options:

- Preferred: fetch the result detail by job id and call the lower-level populate helper directly:

```js
async function populateComputeFromJob(jobId) {
  const detail = await _getResultDetail(jobId);
  _populateComputeFromDetail(jobId, detail || {});
  switchTab('compute');
}
```

This is more robust than going through `_ensureResultsSelection(job_id)`: `/detail` resolves by `job_id` from S3, so `Go Compute` still works when the job exists but is absent from the current `/list` cache/table.

If `/detail` fails because the job was genuinely deleted or is unreadable, show a clear menu error with the job id.

### Go Result

Switches to the Results tab and selects the source compute job.

Implementation:

```js
await _ensureResultsSelection(tile.job_id);
switchTab('results');
```

If `_ensureResultsSelection` fails, show an error in the context menu/log. Unlike `Go Render`, there is no useful direct fallback because this action specifically means "select the Results row."

### Favorite

Current favorites are color-artifact favorites. They store `(job_id, artifact_id)` references and are rendered through the Favorites tab.

Therefore v1 behavior should be:

- Enabled for `AllCol` color tiles.
- Disabled for `AllPal` palette tiles with a tooltip: `Favorites currently support Color artifacts only`.

Implementation:

Extract a parameterized helper from `favoriteSelectedRenderArtifact()`:

```js
async function _addColorFavorite(ref, options = {}) {
  const jobId = String(ref.jobId || ref.job_id || '').trim();
  const artifactId = String(ref.artifactId || ref.artifact_id || '').trim();
  if (!jobId || !artifactId) throw new Error('Favorite requires job_id and artifact_id');
  if (_isFavorite(jobId, artifactId)) return { already: true };
  const resp = await lambdaPost('storage', {
    job_id: jobId,
    artifact_id: artifactId,
    family: 'color',
    display_name: ref.displayName || ref.display_name || artifactId,
    image_key: ref.imageKey || ref.image_key || '',
    preview_key: ref.previewKey || ref.preview_key || '',
  }, '/add-favorite');
  _favoriteRefs = Array.isArray(resp.favorites) ? resp.favorites : _favoriteRefs;
  _favoriteRefsLoaded = true;
  if (document.getElementById('tab-favorites')?.classList.contains('active')) {
    await loadFavoritesInventory();
  }
  return { already: false };
}
```

Then:

- `favoriteSelectedRenderArtifact()` becomes a wrapper that reads `_renderSelectedArtifactEntry()` and calls `_addColorFavorite(...)`.
- The wrapper must preserve the current Render button behavior: disable/re-enable the button, keep the button text/already-favorited log behavior, and call `_updateRenderActionButtons()` after the favorite state changes. The refactor should change the source of truth, not regress the Render tab UI.
- The mosaic context menu calls `_addColorFavorite(...)` directly from the tile data:

```js
const result = await _addColorFavorite({
  jobId: tile.job_id,
  artifactId: tile.artifact_id,
  displayName: tile.display_name || tile.artifact_id,
  imageKey: tile.image_key || '',
  previewKey: tile.key || '',
});
```

The menu should surface both outcomes explicitly:

- `{ already: false }` -> `Favorited`.
- `{ already: true }` -> `Already in favorites`.

Do not navigate to Render just to favorite. That makes Favorite depend on unrelated tab-selection state and can favorite the previous Render selection if a select-miss occurs.

This still reuses the existing favorite API and state update logic, but the reusable unit is a command helper, not a selected-artifact UI handler.

Future option:

- Add palette favorites as a separate feature if wanted. That should update the favorites schema and Favorites tab deliberately; it should not be hidden inside this right-click work.

### Download

Downloads the artifact image, not the preview thumbnail.

Use `tile.image_key` when available. Fall back to `tile.key` only if the manifest lacks `image_key`.

Implementation:

```js
const key = tile.image_key || tile.key;
const artifactId = _mosaicArtifactId(kind, tile);
const ext = _mosaicExtensionFromKey(key) || 'png';
const filename = `${artifactId}.${ext}`;
await _downloadStorageObject({ key, filename });
```

Extract a command-style helper from `downloadPresignedFile(...)`:

```js
async function _downloadStorageObject({ key, filename, fallbackUrl = '' }) {
  if (key) {
    const result = await lambdaPost('storage', { key, filename }, '/presign');
    window.location.href = result.url;
    return;
  }
  if (!fallbackUrl) throw new Error('Download requires a storage key or URL');
  const a = document.createElement('a');
  a.href = fallbackUrl;
  a.download = filename;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
}
```

Then `downloadPresignedFile(...)` becomes a UI wrapper around `_downloadStorageObject(...)` that handles button text/disabled state and alerts. The context menu calls `_downloadStorageObject(...)` directly and reports errors in the menu.

This still uses the existing `/presign` flow rather than embedding presigned URLs in the mosaic manifest. The manifest stays small and public-cacheable.

For palettes, download `image.jpeg` unless the tile has a different `image_key`.

Optional sub-actions for later:

- `Download Preview`
- `Download Image + Meta`

Do not add these until the simple action is working.

### Copy Link

Copies a stable public S3 object link to the artifact image.

Use `tile.image_key || tile.key` and the manifest `base` URL:

```js
const url = _mosaicPublicUrl(kind, key);
await navigator.clipboard.writeText(url);
```

Fallback if `navigator.clipboard` is unavailable:

- Create a temporary input, select it, `document.execCommand('copy')`.
- If that fails, show the URL text in the context menu so the user can manually copy.

Question to keep explicit:

- Copying a public S3 object link is stable but not a forced download.
- Copying a presigned URL is more private/controlled but expires.

For this app, use the stable public S3 link because existing previews/images are publicly loaded by the browser already.

## Menu Metadata

The context menu should show a compact tile header above the actions.

Recommended fields:

- Artifact id: `artifact_id` or `palette_id`
- Job id: `job_id`
- Function: `function`
- Created: formatted `created_at`
- Preview: `preview_width x preview_height`
- Compute N: `N`
- Degree: `degree`
- Times: `times`

For `AllCol`, also show when present:

- Color mode / interpretation if available in the tile later
- Associated palette id if added to the manifest later

For `AllPal`, also show:

- Metric: `metric`
- Palette: `palette`
- Interpretation: `color_interpretation`
- Channels: `score_output_channel_count`
- Reusable: `render_reusable`
- Layout: `data_layout`

Do not make the right-click menu fetch `/render-summary` just to populate extra metadata. It should open immediately. If more fields are wanted, add them to the async mosaic manifest builder.

## Suggested Extra Actions

Useful additions that fit naturally:

- `Copy Job ID`
- `Copy Artifact ID`
- `Copy Preview Link`
- `Open Image in New Tab`
- `Open Preview in New Tab`
- `Go Palette` from an `AllCol` tile if the color tile later carries `associated_palette_id`
- `Go Color` from an `AllPal` tile if the palette tile carries a linked color id

Recommended v1 extras:

- Add `Copy Job ID`
- Add `Copy Artifact ID`

These are cheap and useful for debugging.

Leave linked `Go Palette` / `Go Color` for later unless the manifest already carries reliable linkage for both walls.

## UI Design

Add one floating context menu element to `index.html`, for both mosaics:

```html
<div id="artifact-mosaic-context-menu" class="artifact-mosaic-menu" hidden></div>
```

Recommended behavior:

- Right-click on a tile opens the menu near the pointer.
- Right-click on blank mosaic space closes the menu.
- Left-click outside closes the menu.
- Escape closes the menu.
- Scrolling/refreshing/rebuilding the mosaic closes the menu.
- The menu should stay within viewport bounds.
- Menu action buttons should disable while an async action is running.
- Action failures should be visible in the menu and logged to the relevant mosaic log.

Recommended visual style:

- Dark panel matching the Render artifact tables.
- Header in monospace for ids.
- Metadata in small two-column rows.
- Primary action `Go Render` at top.
- Copy/download actions grouped below.

## Implementation Steps

1. Add context-menu markup and CSS in `index.html`.

2. Add one shared context-menu state in `js/13-artifact-mosaics.js`.

Do not put `contextTile/contextKind` inside each per-kind mosaic state. There is one DOM menu and only one menu can be open at a time. Use one global context state:

```js
const _artifactMosaicContext = {
  open: false,
  kind: '',
  tile: null,
  tileKey: '',
  x: 0,
  y: 0,
  busy: false,
  error: '',
};
```

3. Register OpenSeadragon right-click handling in `_ensureMosaicViewer(kind)`.

OpenSeadragon event names to verify in implementation:

- Try `canvas-contextmenu` first.
- If unavailable, attach a native `contextmenu` listener to the same viewer element used for coordinate conversion.

For `canvas-contextmenu`, use the OSD event's `.position` and call `event.originalEvent.preventDefault()` when present.

For the native `contextmenu` fallback, `_tileFromMosaicClick(kind, event)` cannot be called directly. Native DOM events have `clientX/clientY`, not OSD's `.position`, and `_tileFromMosaicClick` currently rejects events without `.position`.

The native fallback must also pass a real `OpenSeadragon.Point`, not a plain `{x, y}` object. `viewport.pointFromPixel(...)` expects OSD point semantics; a plain object can fail inside OSD viewport math.

Required refactor:

```js
function _tileFromMosaicPixel(kind, pixelPoint) {
  const state = _mosaicState(kind);
  const source = state.activeTileSource;
  if (!state.viewer || !source || !Array.isArray(state.tiles)) return null;
  const viewportPoint = state.viewer.viewport.pointFromPixel(pixelPoint);
  const imagePoint = state.viewer.viewport.viewportToImageCoordinates(viewportPoint);
  const tileSize = Number(source._mosaicTileSize);
  const cols = Number(source._mosaicCols);
  if (!Number.isFinite(tileSize) || tileSize <= 0 || !Number.isFinite(cols) || cols <= 0) return null;
  const x = Math.floor(imagePoint.x / tileSize);
  const y = Math.floor(imagePoint.y / tileSize);
  if (x < 0 || y < 0 || x >= cols) return null;
  return state.tiles[y * cols + x] || null;
}
function _tileFromMosaicClick(kind, event) {
  ...
  return _tileFromMosaicPixel(kind, event.position);
}
function _tileFromMosaicDomEvent(kind, domEvent) {
  const el = _mosaicEl(kind, 'viewer'); // or state.viewer.element
  const rect = el.getBoundingClientRect();
  return _tileFromMosaicPixel(
    kind,
    new OpenSeadragon.Point(
      domEvent.clientX - rect.left,
      domEvent.clientY - rect.top,
    ),
  );
}
```

Attach the native listener to that same `el`. Do not listen on one element and compute the rect from another, or every right-click can be offset by border/padding differences.

This keeps OSD left-click, OSD right-click, and native right-click on one tile-mapping core.

When opening a context menu, store a tile identity snapshot such as:

```js
_artifactMosaicContext.tileKey = `${tile.job_id}|${_mosaicArtifactId(kind, tile)}|${state.manifest?.refresh_id || ''}`;
```

Before running an action, verify the menu is still open for the same `kind` and `_artifactMosaicContext.tileKey`. This is an extra guard against stale actions after refresh/sort/filter.

4. Add helpers:

```js
function _mosaicArtifactId(kind, tile)
function _mosaicExtensionFromKey(key)
function _addColorFavorite(ref, options = {})
function _downloadStorageObject({ key, filename, fallbackUrl = '' })
function _openMosaicContextMenu(kind, tile, domEvent)
function _closeMosaicContextMenu()
function _renderMosaicContextMenu()
function _runMosaicContextAction(action)
```

Use the existing `_mosaicPublicUrl(kind, key)` for URLs instead of adding a duplicate public-url helper.

5. Refactor the current left-click navigation into a reusable helper:

```js
async function _goMosaicTileRender(kind, tile)
```

This helper sets `render-results-dir` directly, switches to Render, calls `refreshRenderArtifacts(...)`, and verifies the selected Render artifact id. It must not call `_ensureResultsSelection(...)`.

Then left-click and right-click `Go Render` call the same function.

This avoids another copy of the exact regression that just happened around stale Results selection.

6. Close stale menus on mosaic rebuild/refresh.

Call `_closeMosaicContextMenu()` at the top of `_rebuildArtifactMosaic(kind)`, because that function swaps `state.tiles` and `state.activeTileSource` under any captured tile reference.

Also close the menu when starting a manual refresh and when stopping/polling a refresh after status changes. A context menu must never act on a tile from a previous sort/filter/manifest.

7. Implement `Go Result`.

Use `_ensureResultsSelection(job_id)` and `switchTab('results')`. Fail visibly if the Results row cannot be selected.

8. Implement `Go Compute`.

Add `populateComputeFromJob(jobId)` as a direct `/detail` wrapper around `_populateComputeFromDetail(jobId, detail)`. Do not route this through Results-row selection.

9. Implement `Favorite`.

For v1, support only `kind === 'color'`.

Internally:

1. Extract `_addColorFavorite(ref, options = {})` from `favoriteSelectedRenderArtifact()`.
2. Make `favoriteSelectedRenderArtifact()` a wrapper around `_addColorFavorite(...)`.
3. Keep the wrapper's existing Render button disable/text/update behavior, including `_updateRenderActionButtons()`.
4. Make the context menu call `_addColorFavorite(...)` directly from tile data and report `Favorited` versus `Already in favorites` based on the returned `{already}` flag.

Do not call `_goMosaicTileRender(...)` as part of Favorite. Favorite is a data operation, not navigation, and should not depend on Render's selected artifact state.

10. Implement `Download`.

Extract `_downloadStorageObject({ key, filename, fallbackUrl })` from `downloadPresignedFile(...)`. Make `downloadPresignedFile(...)` a wrapper around it, and make the context menu call `_downloadStorageObject(...)` directly.

11. Implement `Copy Link`, `Copy Job ID`, and `Copy Artifact ID`.

Centralize clipboard logic in one helper:

```js
async function _copyTextToClipboard(text)
```

12. Update frontend tests in `tests/test_frontend_js.sh`.

Required coverage:

- Right-click on a color tile opens the menu.
- Native `contextmenu` fallback constructs an `OpenSeadragon.Point`, uses the same viewer element for listener and rect, and maps to the same tile as OSD `canvas-contextmenu`.
- Menu shows job id, artifact id, function, N, degree, preview dimensions.
- `Go Render` sets the Render job directly, does not call `_ensureResultsSelection(...)`, and verifies the selected Render artifact id.
- `Go Result` calls `_ensureResultsSelection` and switches to Results.
- `Go Compute` calls `/detail` by job id and does not require the job to be in the Results list.
- `Favorite` is enabled for color and disabled for palette.
- `Favorite` calls `_addColorFavorite(...)` from tile data and does not call `_goMosaicTileRender(...)` or `favoriteSelectedRenderArtifact()`.
- `Download` calls `_downloadStorageObject(...)` with `image_key`, not preview key, and does not call `_dlMenuAction(...)`.
- `Copy Link` uses `image_key` and manifest base.
- Right-click blank space does not open a stale menu.
- Rebuild/refresh closes the menu and stale `contextTileKey` prevents actions from running against an old tile.

13. Run:

```bash
bash tests/test_frontend_js.sh
uv run python -m pytest tests/test_frontend_parts_contract.py -q
```

In this local sandbox, `uv` usually needs escalation per `deployment-checklist.md`; the command above is the repo-standard command, not a sandbox hint.

No backend tests are required unless the manifest schema is extended.

## Risks

1. OpenSeadragon right-click event naming may differ from expectation. Verify with the actual 4.1.1 API. `canvas-contextmenu` should carry an OSD `.position`; the native `contextmenu` fallback needs explicit `clientX/clientY` to viewer-pixel conversion.

2. Favorite is currently color-only. Do not pretend palette favorites exist unless the Favorites schema/tab are extended. The context menu must call `_addColorFavorite(...)` directly from tile data; do not navigate to Render and call `favoriteSelectedRenderArtifact()`, because a select-miss can favorite the previous artifact.

3. Download should use the artifact image key, not the preview tile key. Otherwise it downloads the small mosaic preview and feels broken.

4. Download must not call `_dlMenuAction(...)`; that helper reads current Render selection and download-menu DOM state.

5. Copy Link should be explicit about public S3 URL vs presigned URL. Use public object URL for v1.

6. The menu must close on mosaic rebuild/refresh. Otherwise it can act on a stale tile after sort/filter/refresh changes. Also verify `contextTileKey` before executing actions.

7. `Go Render` should not require the Results list. If the Results list is empty or stale, a mosaic tile still has enough data to open Render directly by `job_id + artifact_id`.

## Recommendation

Implement v1 with:

- `Go Render`
- `Go Result`
- `Go Compute`
- `Favorite` for `AllCol` only
- `Download`
- `Copy Link`
- `Copy Job ID`
- `Copy Artifact ID`

Show metadata:

- artifact id
- job id
- function
- created date
- preview size
- compute `N`
- degree
- times
- palette metric/palette/interp/channel info for `AllPal`

This is enough to make the mosaic useful as a navigation and triage surface without turning it into a second Render inventory UI.
