## Favorites

### Goal

Add a new top-level tab called `Favorites` that shows a cross-job list of favorited `Color` artifacts.

This is **not** a new render artifact family and **not** a new backend artifact type. It is a persisted list of references to existing immutable `Color` artifacts.

V1 scope:

- from `Render -> Color`, user can mark the selected Color artifact as a favorite
- `Favorites` tab shows all saved favorite references across all jobs
- `Favorites` tab has:
  - a table like `Render -> Color`
  - an image preview on the right
  - `Delete`
  - `Download`
- `Download` offers exactly:
  - `Image + Meta`
  - `Select Dir…`

Non-goals for V1:

- no new Lambda
- no server-side global favorites index
- no support for favoriting non-Color artifacts

### UX

#### Render -> Color

Add a new button to the existing `Render -> Color` action row:

- `Favorite`

Behavior:

- only shown when active render family is `color`
- enabled when a Color artifact row is selected and that artifact is not already favorited
- if the selected artifact is already in favorites:
  - keep the button visible
  - disable it
  - label it `Favorited`
- on click:
  - add a reference to the selected Color artifact
  - log a green success line in `render-log`
  - do not mutate or copy the underlying artifact

#### Favorites Tab

Add a new top-level tab in the main tab bar:

- `Favorites`

The layout should mirror the existing Render artifact browser closely:

- left side: table of favorite Color artifacts
- right side: image preview
- top action row:
  - `Download ▼`
  - `Delete`

Download menu entries must be exactly:

- `Image + Meta`
- `Select Dir…`

Behavior:

- selecting a row updates the preview
- `Delete` removes the favorite reference only
- `Delete` must **not** delete the underlying Color artifact from S3
- `Download` downloads the original image plus generated metadata JSON for that Color artifact

### Persistence Model

Do **not** use `localStorage` as the source of truth.

Favorites must survive:

- page reload
- browser restart
- cache clear on the frontend
- moving between machines that use the same deployed app/backend

Use S3-backed persistence through the existing storage Lambda.

Recommended storage key:

- `polypaint/favorites/color_artifacts.json`

Recommended JSON shape:

```json
[
  {
    "job_id": "compute_...",
    "artifact_id": "color_run_...",
    "family": "color",
    "added_at": "2026-04-05T12:34:56Z",
    "display_name": "color_run_...",
    "image_key": "renders/compute_.../color/color_run_.../image.jpeg",
    "preview_key": "renders/compute_.../color/color_run_.../preview.png"
  }
]
```

Rules:

- dedupe by `(job_id, artifact_id)`
- keep newest-added first
- if user favorites an already-favorited artifact:
  - do nothing
  - log `already in favorites`

Because there is no user/auth model in the app right now, this is effectively a **global app-wide favorites list**.

That is acceptable for V1.

Optional frontend cache:

- a local in-memory cache is fine
- a localStorage mirror is fine as an optimization
- but S3 JSON must remain the source of truth

### Hydration Model

Favorites are references, not full artifact objects.

On app startup:

1. call storage `/list-favorites`
2. cache the returned refs in frontend memory as `_favoriteRefs`

When the `Favorites` tab is rendered:

1. read `_favoriteRefs`
2. group refs by `job_id`
3. for each distinct `job_id`, call existing storage `/render-summary`
4. read `summary.families.color`
5. find the artifact whose `artifact_id` matches the stored ref
6. build a hydrated favorites list for the UI

Do not refetch the favorites JSON on every tab switch by default.

Use this model instead:

- initial source-of-truth fetch on startup
- write-through save on add
- write-through save on delete
- explicit `Refresh` button on the Favorites tab to re-read S3 JSON if needed

Hydrated row object shape:

```js
{
  ...colorArtifactFromDetail,
  favorite_ref: { job_id, artifact_id, family, added_at },
  favorite_job_id: job_id,
  favorite_added_at: added_at
}
```

#### Missing / stale references

If a stored favorite reference no longer resolves:

- keep it in the table as a stale row
- summary column should show `missing artifact`
- preview should show empty state
- `Download` should be disabled
- `Delete` should still work

This is important. Silent auto-pruning hides what happened and makes the UI confusing.

### UI Structure

#### New top-level tab

In the main tab bar in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

- add `Favorites` button
- add `<div id="tab-favorites" class="tab-content">`

Recommended placement:

- after `Render`
- before `DeepZoom`

Update `switchTab(name)` so:

- `switchTab('favorites')` activates the tab
- it calls `loadFavoritesInventory()`

#### Favorites tab DOM

Use a dedicated panel, not a hacked sixth Render family.

Recommended ids:

- `favorites-count`
- `favorites-info`
- `favorites-preview`
- `favorites-log`
- `favorites-download-menu`
- `btn-favorites-refresh`
- `btn-favorites-download`
- `btn-favorites-delete`

The table/viewer layout should be visually aligned with the Render panel so it feels native.

### Frontend State

Add dedicated state in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

```js
let _favoriteRefs = [];
let _favoriteArtifacts = [];
let _favoriteSelectedIdx = -1;
let _favoriteCatalogScrollTop = 0;
let _favoriteSaveDirHandle = null;
let _favoriteRefsLoaded = false;
```

Do not overload `_renderArtifacts` for this.

Favorites are conceptually different:

- Render inventory = artifacts for one selected job
- Favorites = cross-job references

### Functions To Add

Add these helpers in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html).

#### Storage helpers

- `_isFavorite(jobId, artifactId)`
- `_favoriteRefKey(jobId, artifactId)`
- `_loadFavoriteRefs(force = false)`

#### Render-side action

- `favoriteSelectedRenderArtifact()`

This uses:

- current job id from `render-results-dir`
- current selected artifact from `_renderSelectedArtifactEntry()`

Checks:

- active family must be `color`
- selected artifact must exist
- job id must exist
- call storage `/add-favorite`
- update `_favoriteRefs` immediately after success
- mark `_favoriteRefsLoaded = true`

#### Favorites loading / rendering

- `loadFavoritesInventory()`
- `refreshFavoritesInventory()`
- `renderFavoritesPanel()`
- `_favoriteSelectedEntry()`
- `_favoriteSelect(idx)`
- `_favoriteRestoreScroll()`
- `_favoriteArtifactSummary(art)`
- `_favoriteArtifactFilename(art)`

#### Favorites download / delete

- `_toggleFavoritesDownloadMenu()`
- `_favoritesDlMenuAction(mode)`
- `deleteSelectedFavorite()`

These should call:

- storage `/list-favorites`
- storage `/add-favorite`
- storage `/delete-favorite`

Reuse existing helpers where possible:

- `_fmtSize(...)`
- `_buildArtifactMeta(jobId, art)`
- `downloadPresignedFile(...)`

### Changes To Existing Functions

#### `switchTab(name)`

In [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

- add:
  - `if (name === 'favorites') loadFavoritesInventory();`

Behavior:

- if `_favoriteRefsLoaded` is false, `loadFavoritesInventory()` should first call `_loadFavoriteRefs()`
- otherwise it should render from the cached refs without re-reading S3

#### `renderArtifactPanel(...)`

In the Color action row, add:

- `Favorite`

Recommended id:

- `btn-render-favorite`

Behavior:

- visible only when `_renderActiveFamily === 'color'`
- enabled only when selected artifact exists and is not already favorited
- if already favorited:
  - disabled
  - text becomes `Favorited`

Also update `_updateRenderActionButtons()` so the button state stays correct when:

- user changes row selection
- family changes
- render inventory refreshes
- favorites inventory refreshes

### Favorites Tab Controls

Add a `Refresh` button to the Favorites action row.

Button text:

- `Refresh`

Behavior:

- calls `refreshFavoritesInventory()`
- this forces `_loadFavoriteRefs(true)`
- then rerenders the hydrated table
- log a green success line in `favorites-log` when complete

### Storage Lambda Changes

Add routes in [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py):

- `POST /list-favorites`
- `POST /add-favorite`
- `POST /delete-favorite`

Recommended helper functions:

- `_favorites_key()`
- `_read_favorites()`
- `_write_favorites(items)`

Behavior:

#### `/list-favorites`

- read `polypaint/favorites/color_artifacts.json`
- if missing, return empty list
- response:

```json
{ "favorites": [...], "count": 12 }
```

#### `/add-favorite`

Input:

```json
{
  "job_id": "compute_...",
  "artifact_id": "color_run_...",
  "family": "color",
  "display_name": "color_run_...",
  "image_key": "...",
  "preview_key": "..."
}
```

Behavior:

- read current list
- dedupe by `(job_id, artifact_id)`
- prepend new item if absent
- write full file back
- return `added: true/false`

#### `/delete-favorite`

Input:

```json
{
  "job_id": "compute_...",
  "artifact_id": "color_run_..."
}
```

Behavior:

- remove matching ref only
- write full file back
- return `deleted: true/false`

Implementation note:

- S3 read-modify-write is acceptable for V1
- favorites volume is tiny
- concurrency risk is low
- do not overengineer this into DynamoDB for the first pass

### Download Behavior

Favorites download must mirror Render download behavior, but with a narrower menu.

Required menu entries:

- `Image + Meta`
- `Select Dir…`

Behavior:

- `Select Dir…` stores a directory handle in `_favoriteSaveDirHandle`
- `Image + Meta`:
  - if no directory selected:
    - browser-download image
    - browser-download generated meta JSON
  - if directory selected:
    - write both files to the selected folder

Generated metadata should come from existing `_buildArtifactMeta(jobId, art)`.

Important:

- for Favorites, `job_id` comes from `favorite_job_id`, not from `render-results-dir`

### Delete Behavior

`Delete` in Favorites means:

- call storage `/delete-favorite`
- refresh the Favorites tab
- do not call `/delete-render-artifact`

Confirmation text should make that explicit:

- `Remove favorite reference for color_run_... ?`

### Reuse vs Duplication

Do not try to jam Favorites into the existing Render-family machinery.

Recommended approach:

- keep a separate `renderFavoritesPanel()` implementation
- reuse small existing helpers only

Why:

- Favorites are cross-job
- Render assumes one `job_id`
- Render action row is family-specific and already crowded
- trying to fake Favorites as a Render family will create edge cases and hidden coupling

### Exact File Touch List

V1 should require:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py)
- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- a new storage test file or an existing storage suite under [tests](/Users/nicknassuphis/karpo_hackathon/polypaint/tests)
- optionally [docs/testing.md](/Users/nicknassuphis/karpo_hackathon/polypaint/docs/testing.md) if we want to document a Favorites smoke test

No new Lambda should be necessary for V1.

### Implementation Order

1. Add `Favorites` tab shell and `switchTab('favorites')` hook.
2. Add storage routes in `handler_storage.py`.
3. Add startup favorites load into `_favoriteRefs`.
4. Add `Favorite` button to `Render -> Color`.
5. Add `loadFavoritesInventory()` hydration using cached refs plus existing `/render-summary`.
6. Add `renderFavoritesPanel()` with table + preview.
7. Add Favorites `Refresh`.
8. Add Favorites `Download` menu with:
   - `Image + Meta`
   - `Select Dir…`
9. Add Favorites `Delete`.
10. Add stale-reference UI handling.
11. Tighten tests.

### Tests

Add frontend coverage in [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh).

Add backend coverage for [lambda/handler_storage.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_storage.py).

Minimum required checks:

- main tab bar contains `Favorites`
- startup path loads favorite refs once into frontend state
- `switchTab('favorites')` calls the Favorites loader
- `Render -> Color` action row contains `Favorite`
- selecting a Color artifact enables `Favorite`
- clicking `Favorite` posts the correct storage payload
- already-favorited selected artifact shows `Favorited`
- Favorites panel renders table rows from hydrated refs
- Favorites action row contains `Refresh`
- `Refresh` forces a `/list-favorites` reload
- Favorites preview shows selected image
- Favorites `Delete` removes only the stored favorite ref
- Favorites `Download` menu contains exactly:
  - `Image + Meta`
  - `Select Dir…`
- Favorites download uses the favorited artifact’s own `job_id`
- stale favorite reference renders as `missing artifact`
- stale favorite disables `Download` but allows `Delete`

Backend checks:

- `/list-favorites` returns empty list when file missing
- `/add-favorite` dedupes correctly
- `/delete-favorite` removes one entry only
- stored file round-trips valid JSON

### Manual Smoke Checklist

Before calling this done:

1. Favorite a Color artifact from one job.
2. Favorite a Color artifact from a different job.
3. Open `Favorites` and verify both rows appear together.
4. Click each row and verify preview changes.
5. Download `Image + Meta`.
6. Set `Select Dir…`, then save again.
7. Delete one favorite and verify the underlying Render artifact still exists.
8. Delete the underlying Color artifact from Render, then verify Favorites shows a stale row and lets you remove it.

### Acceptance Criteria

The feature is done when:

- `Favorite` exists in `Render -> Color`
- `Favorites` exists as a top-level tab
- Favorites shows a cross-job list of favorited Color artifacts
- Preview works
- `Delete` removes the reference only
- `Download` offers exactly `Image + Meta` and `Select Dir…`
- no backend changes were needed for the first version
