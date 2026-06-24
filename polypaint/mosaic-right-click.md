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

The render/favorite/download helpers already exist elsewhere:

- Render selection: `refreshRenderArtifacts(jobId, { selectFamily, selectArtifactId })`
- Result selection: `_ensureResultsSelection(jobId)` and `selectResult(jobId)`
- Render job input fallback: `_setRenderResultsJob(jobId)`
- Favorite selected color artifact: `favoriteSelectedRenderArtifact()`
- Browser download helpers: `downloadPresignedFile(...)`, `_downloadStorageKeyBlob(...)`, `_downloadBlob(...)`
- Presigned download flow: `/presign`

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

This should reuse the same robust fallback as left-click: try `_ensureResultsSelection(job_id)`, but if Results is stale or filtered, set `render-results-dir` directly and still open Render.

### Go Compute

Switches to the Compute tab and loads/populates the selected result's compute configuration.

This is not the same as selecting the Results row. It should be treated as a convenience action for "take me back to the calculation inputs for this job."

Implementation options:

- Preferred: call `_ensureResultsSelection(tile.job_id)`, then call the existing result-populate function used by the Results tab.
- If no single public helper exists, create one small wrapper such as `populateComputeFromJob(job_id)` that selects the result and invokes the existing Populate path.

If the result no longer exists in `/list`, this action should show a clear menu error: `Compute result not found in Results list`.

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

1. Open/select the artifact in Render using the same logic as `Go Render`.
2. Verify the Render selection actually landed on the same color artifact:

```js
const selected = _renderSelectedArtifactEntry();
if (!selected || selected.artifact_id !== tile.artifact_id) {
  throw new Error(`Cannot favorite ${tile.artifact_id}: Render did not select it`);
}
```

3. Call `favoriteSelectedRenderArtifact()`.

This reuses the existing favorite API and avoids creating a second favorites path that can drift.

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
await downloadPresignedFile('', filename, key);
```

This should use the existing `/presign` flow via `downloadPresignedFile`, rather than embedding presigned URLs in the mosaic manifest. The manifest should stay small and public-cacheable.

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

2. Extend `js/13-artifact-mosaics.js` state:

```js
contextTile: null,
contextKind: '',
contextOpen: false,
```

3. Register OpenSeadragon right-click handling in `_ensureMosaicViewer(kind)`.

OpenSeadragon event names to verify in implementation:

- Try `canvas-contextmenu` first.
- If unavailable, attach a native `contextmenu` listener to `viewer.canvas` or the viewer container.

For `canvas-contextmenu`, use the OSD event's `.position` and call `event.originalEvent.preventDefault()` when present.

For the native `contextmenu` fallback, `_tileFromMosaicClick(kind, event)` cannot be called directly. Native DOM events have `clientX/clientY`, not OSD's `.position`, and `_tileFromMosaicClick` currently rejects events without `.position`.

Required refactor:

```js
function _tileFromMosaicPixel(kind, pixelPoint) { ... }
function _tileFromMosaicClick(kind, event) {
  ...
  return _tileFromMosaicPixel(kind, event.position);
}
function _tileFromMosaicDomEvent(kind, domEvent) {
  const rect = viewerCanvasOrElement.getBoundingClientRect();
  return _tileFromMosaicPixel(kind, {
    x: domEvent.clientX - rect.left,
    y: domEvent.clientY - rect.top,
  });
}
```

This keeps OSD left-click, OSD right-click, and native right-click on one tile-mapping core.

4. Add helpers:

```js
function _mosaicArtifactId(kind, tile)
function _mosaicArtifactImageKey(tile)
function _mosaicExtensionFromKey(key)
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

Then left-click and right-click `Go Render` call the same function.

This avoids another copy of the exact regression that just happened around stale Results selection.

6. Implement `Go Result`.

Use `_ensureResultsSelection(job_id)` and `switchTab('results')`. Fail visibly if the Results row cannot be selected.

7. Implement `Go Compute`.

Reuse the existing result Populate path. If it is not exposed as a job-id function, extract a small public wrapper and use it from both the Results button and this menu.

8. Implement `Favorite`.

For v1, support only `kind === 'color'`.

Internally:

1. Call `_goMosaicTileRender('color', tile)`.
2. Confirm `_renderSelectedArtifactEntry()?.artifact_id === tile.artifact_id`.
3. Only then call `favoriteSelectedRenderArtifact()`.

If the selected artifact does not match, show a menu error and do not call the favorite helper. This prevents favoriting a previously selected Render artifact after a select-miss.

9. Implement `Download`.

Use `downloadPresignedFile('', filename, tile.image_key || tile.key)`.

10. Implement `Copy Link`, `Copy Job ID`, and `Copy Artifact ID`.

Centralize clipboard logic in one helper:

```js
async function _copyTextToClipboard(text)
```

11. Update frontend tests in `tests/test_frontend_js.sh`.

Required coverage:

- Right-click on a color tile opens the menu.
- Menu shows job id, artifact id, function, N, degree, preview dimensions.
- `Go Render` uses the same fallback as left-click when Results selection fails.
- `Go Result` calls `_ensureResultsSelection` and switches to Results.
- `Favorite` is enabled for color and disabled for palette.
- `Download` calls the existing presigned download helper with `image_key`, not preview key.
- `Copy Link` uses `image_key` and manifest base.
- Right-click blank space does not open a stale menu.

12. Run:

```bash
bash tests/test_frontend_js.sh
uv run python -m pytest tests/test_frontend_parts_contract.py -q
```

In this local sandbox, `uv` usually needs escalation per `deployment-checklist.md`; the command above is the repo-standard command, not a sandbox hint.

No backend tests are required unless the manifest schema is extended.

## Risks

1. OpenSeadragon right-click event naming may differ from expectation. Verify with the actual 4.1.1 API. `canvas-contextmenu` should carry an OSD `.position`; the native `contextmenu` fallback needs explicit `clientX/clientY` to viewer-pixel conversion.

2. Favorite is currently color-only. Do not pretend palette favorites exist unless the Favorites schema/tab are extended. Also verify the Render selection before calling `favoriteSelectedRenderArtifact()`, or a select-miss can favorite the previous artifact.

3. Download should use the artifact image key, not the preview tile key. Otherwise it downloads the small mosaic preview and feels broken.

4. Copy Link should be explicit about public S3 URL vs presigned URL. Use public object URL for v1.

5. `Go Compute` may need a small extraction if the current Populate logic is tied to the selected Results row and button state.

6. The menu must close on mosaic rebuild/refresh. Otherwise it can act on a stale tile after sort/filter/refresh changes.

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
