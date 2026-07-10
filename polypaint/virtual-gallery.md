# Virtual Gallery: implementation design

Code-grounded revision, 2026-07-10. Second review pass folded in (all findings
verified against source): DeepZoom cache keyed per artifact not per job (§9.1);
memory-safe Load-original with a size guard vs `RENDER_MAX_PIX=32768` (§9);
"latest ready" vs displayed-manifest snapshot semantics (§3); version-stamped
vendored Three.js to avoid in-place overwrite (§1.4/§12.2); shared ordering
helper packaging made consistent with §12 (§5); room-size defaults reduced to
32 pieces / ~15 m (§2.1/§5.1); texture failure policy — timeouts, slot release,
backoff, negative cache (§7.1); code-unit (locale-independent) sort + explicit
seed (§5); streamed manifest size cap + exact share-path shape (§4.1);
toneMapped/dispose/OSD-teardown lifecycle details (§6/§10). Design addition:
saved galleries are curated exhibitions (Add-to-Gallery per tile → Create
Gallery), and the Create step generates/reuses a per-artifact DZI for every
piece and records its exact key (§9.1/§14.1) — the AllCol wall pyramid is not a
usable per-piece zoom source (one preview-resolution composite). Phasing
reordered: Phase 0 now curates from the DeepZoom tab (existing exports only) so
the full viewer + real DZI zoom can be tested with no generation orchestrator;
AllCol curation + generate-missing moves to Phase 2 (§15).
Third review pass (all citations verified): the DeepZoom-first pivot's storage
+ orchestration architecture is now specified — a minimal `/share-gallery`
write+enrich route (§3.1), server-side color filtering/enrichment because the
export `meta.json` lacks display fields (§3.1), a two-schema viewer contract
(`virtual_gallery` vs `artifact_mosaic`, per-type paths, §4), unique
deterministic export/task ids + bounded (2–4) generation orchestrator with
reuse-map/retries/resume/poll-exact-key (§14.1), vendored OpenSeadragon (§12),
ordering-helper scoped to Phase 2 (§5), backend/orchestrator tests (§16.4),
`image.{jpeg,png}` not assumed (F11), and the Phase-0 definition made
consistent across §0/§9/§15/§19.
Fourth review pass (all citations verified): corrected the false "handler
honors caller task_id" claim — it hardcodes `task_id` and must be changed
(§14.1); split View-Wall-as-Gallery (zoomless `/share-mosaic`, §3) from Create
Gallery (curated `/share-gallery` + DZI, §14.1); defined the immutable share
schema §13.1 (auto-layout, ordered pieces, `artifact_kind`, non-hardcoded
source) apart from the editable §13.2; `/share-gallery` now validates the export
against its `meta.json` + HEADs the DZI (§3.1); gave the Phase-2 build a real
Lambda/DDB orchestrator architecture (§14.1); flagged `/list-deepzoom`'s
swallowed-error false-miss (§14.1); enrichment reuses
`_render_family_entry_from_prefix`+`_mosaic_tile_from_entry` (§3.1); OSD deploy
fully specified (§12.2); `/share-gallery` vs `/create-gallery-share` split
(§14); curator `ordinal` preserved (§3.1); squareness validated not assumed;
Phase-0 checklist now builds+deploys the route (§15).
Fifth review pass (all citations verified) — Phase-0 blockers closed and Phase-2
orchestration made buildable: `/share-gallery` accepts zoomless pieces so a
partial-failure build can publish (§3.1); server assigns ordinals, viewer
re-validates every `deepzoom` on load and ignores absolute URLs (§3.1/§13.1);
task-id change now specified as an ATOMIC handler + Render-caller + orchestrator
migration (§14.1); single Lambda replaced by Step Functions (900 s cap,
`deploy.sh:32`) with a lease/heartbeat (not a permanent claim), worker
existing-output fast path / per-export lease for real idempotency, POST-only
status route, and internal share-builder instead of self-HTTP (§14.1); newest
valid export chosen on reuse (§3.1); schema `document_kind` discriminator + the
editable schema self-contained (§13); manifest-wins layout precedence (§4);
open alternatives marked deferred (§3/§5).
Sixth review pass (all citations verified): a supplied `export_id` is validated
EXACTLY (newest-valid is Phase-2 reuse only) and `meta.dzi_key` must equal the
canonical `deepzoom/<job>/<export>/image.dzi` (§3.1, Phase-0 blockers); the two
executor models are disambiguated — Step Functions SYNCHRONOUS Map (returns the
result, no polling) vs the async self-reinvoking coordinator (polls), never both
(§14.1); the immutable share schema gains `layout.mode:"auto"|"explicit"` so
`/create-gallery-share` preserves arrangement (§13.1); reuse is "newest VALID
wins" (§14.1); Phase-2 SFN deploy infra enumerated (ASL, state machine, scoped
role, `states:*` route perms, packaging tests) (§14.1); `/share-gallery` returns
`skipped[]` with reasons and assigns ordinals post-filter (§3.1); editable
schema example fixed to the `source.kind` enum + compact fields (§13.2).

## 0. Verdict

The feature is viable and useful, but the original draft overstated the
existing asset pipeline and understated the browser work.

The correct first version is:

- a standalone desktop viewer, `gallery.html`;
- curated from the **DeepZoom tab (existing exports only)** and handed off via a
  minimal server-side `/share-gallery` route (§3.1) — so the full viewer plus
  real DZI zoom is testable without any generation orchestrator;
- deterministic placement of the picked pieces;
- preview textures only inside the 3D scene;
- first-person and guided navigation modes;
- a 2D inspection overlay for the selected artifact;
- one minimal new backend route, `/share-gallery` (§3.1), to enrich the picks
  and write the manifest — the browser cannot write to the public bucket. Phase 0
  still avoids the DeepZoom *generation orchestrator* (that is Phase 2).

That is a solid MVP. It reuses existing DeepZoom exports and the color-artifact
resolver, without pretending every artifact has an export or that the browser
can write its own manifest.

A disposable visual spike could be a few hundred lines. A mergeable feature
with manifest validation, bounded texture loading, input cleanup, deployment,
and tests will not be. The distinction matters.

## 1. Corrections to the original proposal

### 1.1 DeepZoom is optional, not a universal second LOD tier

PolyPaint does not currently have a DeepZoom pyramid for every color artifact.
`lambda/handler_deepzoom_export.py` creates one only when an export is requested.
The export lives at:

```text
deepzoom/<job_id>/<export_id>/image.dzi
```

Its `meta.json` records `source_key`, `source_family`, and
`source_artifact_id`. `renders/<job_id>/deepzoom_latest.json` is only the latest
DeepZoom pointer for a job. It is not a complete per-artifact index and can
refer to a different artifact than the one selected in the gallery.

The AllCol wall pyramid is also not a substitute. It is one composite DZI for
the whole mosaic, not one DZI per artifact.

Consequences:

- The 3D wall must work completely without DZI data.
- Never derive or guess a DZI key from an artifact id.
- Never trust `deepzoom_latest.json` without checking both
  `source_artifact_id` and `source_key` against the selected tile.
- Do not call `/list-deepzoom` once per piece or on gallery startup. That is a
  full inventory operation and is the wrong lookup primitive.
- DZI inspection is an optional enhancement in a 2D overlay, not the scene's
  baseline LOD mechanism.

### 1.2 The reusable source is `/share-mosaic`

The current AllCol manifest already contains the fields needed by the MVP.
`lambda/handler_storage.py::_build_color_mosaic_manifest` emits an
`artifact_mosaic` document whose tiles include:

```text
job_id, artifact_id, created_at, function, degree, N, times,
preview_width, preview_height, key, image_key
```

Important field meanings:

- `key` is the preview object to display. It may be a migrated `preview.jpg`
  or a fallback `preview.png`; code must not reconstruct this path.
- `image_key` is the full artifact image.
- `preview_width` and `preview_height` are authoritative geometry metadata.
  Current migrated color previews are 512 by 512, but the viewer should still
  read these fields rather than hard-code that fact.

`POST /share-mosaic` copies the current manifest to a unique key under:

```text
renders/_shared_mosaic/<kind>/<share_id>/manifest.json
```

It returns both `manifest_url` and the current mosaic viewer's `share_url`.
The gallery should consume `manifest_url`. This avoids depending on a moving
or eventually pruned `_index` manifest.

The snapshot copies the manifest, not the image objects. It gives a stable
layout input, but it is not an archival package: deleting an underlying
artifact will still break that piece.

### 1.3 The standalone precedents are narrower than claimed

There is no repository-root `viewer.html`. The real standalone precedents are:

- `artifact_mosaic_viewer.html`;
- `flipbook.html`;
- the generated DeepZoom viewer based on
  `lambda/deepzoom_viewer_template.html`.

They establish the stable root-page and share-link pattern, but not a ready 3D
runtime.

### 1.4 Three.js is not currently vendored

The only current `vendor/` runtime is `page-flip.browser.js`. Three.js and
`PointerLockControls` are absent.

Modern `PointerLockControls` is distributed as an ES module. The implementation
must pin and vendor a compatible Three.js/module pair. Do not mix a current
control module with an unrelated global `THREE` build and do not leave the
version floating on a CDN.

Recommended packaging (version-stamped directory — see §12.2 point 6):

```text
vendor/three-r{NNN}/three.module.js
vendor/three-r{NNN}/addons/controls/PointerLockControls.js
vendor/three-r{NNN}/LICENSE
```

Use an import map in `gallery.html` and a `type="module"` gallery entry point.
No bundler is required, but this is not a classic-script integration.

### 1.5 Frustum culling is not asset management

Three.js frustum culling can avoid drawing off-screen meshes. It does not stop
`TextureLoader` from fetching and decoding every URL it is given. Explicit
load scheduling and eviction are required.

A 512 by 512 RGBA texture is roughly 1 MiB of decoded GPU data before mipmaps
and implementation overhead. Loading 1,500 previews can therefore consume well
over a gigabyte even though the compressed files are much smaller.

The MVP must cap pieces and resident textures. Multi-room virtualization is a
later scaling feature, not something to defer while loading the entire corpus
into one room.

## 2. Product scope and decisions

### 2.1 MVP decisions

| Question | Decision |
|---|---|
| Artifact family | AllCol only |
| Source | DeepZoom tab (existing exports) → small gallery manifest; AllCol + generate deferred to Phase 2 |
| Default piece count | 32 (one row → ~15 m room; see §5.1) |
| Hard MVP piece cap | 64 (one row → ~27 m; use two rows above this) |
| Scene texture | Manifest preview `tile.key` only |
| Full image | Explicit load inside inspection overlay |
| DeepZoom | Optional, exact-identity match only |
| Navigation | Pointer-lock plus guided artwork navigation |
| Persistence | None in MVP |
| Main-app tab | Not in MVP |
| Target | Desktop WebGL; explicit mobile fallback |

The limits are starting values, not sacred constants. Benchmark them on the
actual deployment before raising them.

### 2.2 Non-goals for the MVP

- Loading every AllCol artifact into one WebGL scene.
- In-scene DeepZoom tile projection.
- Interactive wall editing.
- Saved galleries or a Gallery management tab.
- Multiplayer or concurrent visitors.
- Physics, a general collision engine, or arbitrary 3D models.
- Mobile joystick controls.
- Archival shares that survive deletion of source artifacts.

## 3. View Wall as Gallery (zoomless AllCol snapshot)

> There are **two distinct features**, and this section is the simpler one:
> - **View Wall as Gallery** (this §3): open the *whole* current AllCol wall as
>   a 3D room, zoomless, straight from an `artifact_mosaic` snapshot. No
>   curation, no DZI, reuses `/share-mosaic`.
> - **Create Gallery** (§14.1 + §3.1 `/share-gallery`): a *curated* exhibition of
>   picked pieces, each with an exact per-piece DZI. This is the primary product;
>   Phase 0 builds it from DeepZoom-tab picks (existing exports), Phase 2 from
>   AllCol picks (generating misses).
>
> The origin, popup, and manifest-validation rules below apply to both. Do not
> conflate them: View Wall opens an `artifact_mosaic`; Create Gallery opens a
> `virtual_gallery`.

For **View Wall as Gallery**, add a `Gallery` action beside the existing AllCol
`Share` action.

Do not duplicate the POST logic in another button handler. Extract a command
helper from `js/13-artifact-mosaics.js`:

```javascript
async function _createArtifactMosaicSnapshot({kind, size, sort, cols}) {
    return lambdaPost('storage', {kind, size, sort, cols}, '/share-mosaic');
}
```

Then:

- `_shareArtifactMosaic(kind)` gathers UI state, calls the helper, and opens
  the existing `artifact_mosaic_viewer.html` URL.
- `_openArtifactGallery(kind)` gathers the same explicit state, calls the
  helper, and opens `gallery.html` with the returned `manifest_url`.

The gallery URL should be explicit and reproducible:

```text
gallery.html?manifest=<encoded-manifest-url>&size=all&sort=date&seed=1&limit=32
```

Build the gallery URL from the returned manifest origin, not from the current
app page:

```javascript
const manifestUrl = new URL(result.manifest_url);
const galleryUrl = new URL('/gallery.html', manifestUrl.origin);
```

The main app may be open through the S3 website endpoint while the returned
manifest uses the HTTPS REST endpoint. Deriving `gallery.html` from
`location.origin` would therefore turn an intended same-origin load into a
cross-origin one.

Because snapshot creation is asynchronous, open a blank target window
synchronously inside the button click before awaiting the POST. Navigate that
window after the snapshot returns, close it on failure, and retain the
copy-link fallback. Otherwise normal popup blocking will make Gallery feel
randomly broken even though the snapshot succeeded.

Rules:

- `kind` must be `color` for the MVP.
- `size` and `sort` use the existing mosaic semantics.
- `seed` is required for deterministic random ordering. Do not read a hidden
  global random seed inside the standalone viewer.
- `limit` is clamped to `1..64` (the hard cap), defaulting to 32.
- Mosaic `cols` has no useful meaning in a room and is not forwarded.
- Popup blocking follows the existing Share behavior: if `window.open` fails,
  copy the gallery URL and report that clearly.

View Wall as Gallery reuses the existing `/share-mosaic` route (no new route) —
it opens an `artifact_mosaic`, zoomless. The curated Create Gallery flow (§14.1)
is the one that needs the new `/share-gallery` route (§3.1) and DZI.

**"Latest ready" vs "what you see."** `/share-mosaic` does not snapshot the
manifest the client is displaying — `_mosaic_manifest_key_for_share()` always
selects the server's latest ready manifest (`lambda/handler_storage.py:4345`;
this is a pre-existing property shared with the Share button). If a mosaic
refresh completes between the moment the wall renders and the moment Gallery is
clicked, the gallery silently opens different content. Resolve this one of two
ways: (a) accept and document Gallery as "the latest ready mosaic," not "exactly
what is on screen"; or (b) send the displayed `manifest_key` and add an
`expected_manifest_key` check to `/share-mosaic` (a change to the existing
route — still no new route — that returns a conflict when the ready manifest has
moved). **Deferred decision** (does not block primary Phase 0 — this is the
View-Wall feature): resolve it when View Wall is implemented, not before.

**Refresh precondition.** `/share-mosaic` raises "No ready {kind} mosaic
manifest to share; refresh the mosaic first" when nothing is ready
(`handler_storage.py:4351`). The Gallery launcher inherits this exactly like the
Share button, which simply surfaces the error (`js/13-artifact-mosaics.js:295`):
catch it and show "refresh the mosaic first" rather than opening a broken
window.

### 3.1 The `/share-gallery` route (the real handoff — both phases)

The browser cannot write the gallery document: the bucket grants anonymous
`s3:GetObject` only, and there is no `/save-gallery`/`/share-gallery` route yet.
`/share-mosaic` cannot substitute — it ignores caller-supplied pieces and copies
the *latest mosaic*. So Phase 0 needs a minimal new backend route, `POST
/share-gallery`, that:

1. Accepts a bounded, de-duplicated list of picks: `[{job_id, artifact_id,
   export_id?}]`, each id `assert_safe_id`-validated, capped at the piece limit.
   `export_id` is **optional at the route** so the route can publish a
   partial-failure build: a piece with an `export_id` is validated + zoomable, a
   piece without one is a valid **zoomless** piece (`deepzoom: null`, degrades to
   the 512 preview). Reject duplicate `(job_id, artifact_id)`. Order is the
   request-array order — the server assigns normalized `ordinal = 0..n-1` and
   does NOT trust caller ordinals (no duplicate/negative/huge values). *Phase 0
   additionally requires every pick to carry an `export_id`, enforced at
   pick/submit time — not by the route.*
2. Filters to canonical **color** artifacts (the DeepZoom inventory spans all
   families and legacy sources — reject non-color/legacy).
3. Enriches each pick server-side by EXACT key — reusing
   `_render_family_entry_from_prefix` + `_mosaic_tile_from_entry` with a
   per-job `calc.json` cache (the favorites resolver alone lacks the calc
   fields). Resolves the artifact's real `preview_key`/`image_key`
   (`image.{jpeg,png}` / `preview.{jpg,png}` — never assume `.jpeg`),
   `preview_width/height`, and `function/degree/N/times/created_at`. No client
   path reconstruction (§1.2). A genuinely missing artifact is recorded in a
   `skipped[]` list (`{job_id, artifact_id, reason}`) and returned to the caller
   — a curated gallery must not go silently incomplete on a bare count; a
   transient S3 failure fails the whole request (do not silently drop). Final
   `ordinal`s (`0..n-1`) are assigned to the SURVIVING pieces after filtering.
4. For each pick WITH an `export_id`, **validates that EXACT export** — it does
   not silently substitute a different one. Load
   `deepzoom/<job_id>/<export_id>/meta.json` and require `job_id`, `export_id`,
   `source_family == "color"`, `source_artifact_id == artifact_id`,
   `source_key == the resolved image_key`, and that `meta.dzi_key` equals
   **exactly** `deepzoom/<job_id>/<export_id>/image.dzi` (constructed, not read
   as a free string — malformed metadata must not point the share at an
   unrelated object); then `HEAD` that `dzi_key`. Reject an export that belongs
   to another artifact or whose DZI is absent (same identity-trust discipline as
   CR28 F12). Choosing among *multiple* existing exports (newest-valid) is a
   Phase-2 reuse-map concern (§14.1), NOT this route's job — here the caller
   named the export, so this route validates that one.
5. Writes a `virtual_gallery` manifest (§13.1 schema) to an immutable share key
   under `renders/_shared_mosaic/gallery/<share_id>/manifest.json` (kept under
   the already-public, viewer-trusted prefix so §4.1 same-origin rules hold) and
   returns its `manifest_url`.

`/share-gallery` has exactly ONE meaning: create an immutable share from an
explicit validated pick list. Snapshotting a *saved* gallery revision is a
different operation with a different name (§14, `/create-gallery-share`). Both
Phase 0 (DeepZoom-tab picks) and Phase 2 (AllCol picks, after the §14.1
orchestrator has produced each `export_id`) feed this same route.

## 4. Source manifest contract

The viewer accepts **two** document types and normalizes them to one internal
`GallerySceneSpec` (a list of pieces, each with `job_id`, `artifact_id`,
`preview_key`, `image_key`, dims, display fields, and optional exact
`deepzoom`):

- **`virtual_gallery`** (the primary path — written by `/share-gallery`, §3.1)
  at `renders/_shared_mosaic/gallery/<share_id>/manifest.json`. This is what both
  Phase 0 (DeepZoom picks) and Phase 2 (AllCol curation) produce, and it is the
  only one carrying per-piece `deepzoom` references.
- **`artifact_mosaic`** (the AllCol wall manifest, `preview`-only, no DZI) at
  `renders/_shared_mosaic/color/<share_id>/manifest.json` — accepted as a
  zoom-less fallback source.

Validation (§4.1) is per-type: the accepted path prefix and `manifest_type` must
agree (`.../gallery/...` ⇒ `virtual_gallery`; `.../color/...` ⇒ `artifact_mosaic`).
Do not accept an `artifact_mosaic` type at a gallery path or vice-versa.

**Layout-option precedence.** For a `virtual_gallery` share the MANIFEST wins:
its stored `layout.seed` and curator `ordinal` order are authoritative, and URL
`seed`/`size`/`sort`/`limit` params must NOT silently reorder or truncate the
exhibition (it is an immutable curated set). URL layout params apply only to the
`artifact_mosaic` View-Wall source, which has no stored order.

`artifact_mosaic` top-level fields:

```json
{
  "schema_version": 1,
  "manifest_type": "artifact_mosaic",
  "artifact_kind": "color",
  "base": "https://polypaint.s3.us-east-1.amazonaws.com/",
  "manifest_key": "renders/_shared_mosaic/color/.../manifest.json",
  "tiles": []
}
```

Required tile fields:

```json
{
  "job_id": "compute_...",
  "artifact_id": "color_...",
  "key": "renders/<job>/color/<artifact>/preview.jpg",
  "image_key": "renders/<job>/color/<artifact>/image.jpeg",
  "preview_width": 512,
  "preview_height": 512,
  "created_at": "2026-...",
  "function": "...",
  "degree": 12,
  "N": 2048,
  "times": 1
}
```

The viewer must not trust `count`, inferred paths, or display metadata. It
validates the actual `tiles` array and constructs URLs only from validated keys
plus a trusted base.

### 4.1 Input validation

Before creating WebGL resources:

1. Parse `manifest` with `new URL(raw, location.href)`.
2. Require HTTPS in production.
3. Require `manifestUrl.origin === location.origin` for the deployed viewer.
4. Require the EXACT path shape for one of the two accepted types —
   `/renders/_shared_mosaic/gallery/<share_id>/manifest.json` (⇒ expect
   `virtual_gallery`) or `/renders/_shared_mosaic/color/<share_id>/manifest.json`
   (⇒ expect `artifact_mosaic`) — one `share_id` segment matching the id charset,
   no extra segments, not merely a prefix/suffix test (which would accept
   `.../color/a/b/c/manifest.json`). The path type and the document
   `manifest_type` must agree.
5. Fetch the document with `cache: 'no-store'`.
6. Enforce the size cap (initially 8 MiB) BEFORE buffering: reject on a
   `Content-Length` over the cap, then read the body as a stream with a running
   byte counter and abort once it exceeds the cap. `response.text()` buffers the
   whole payload first, so it cannot protect against an oversized manifest.
   `JSON.parse` only the bytes that passed the cap.
7. Require the top-level type, schema version, artifact kind, and array shape.
8. Reject more than `MAX_MANIFEST_TILES = 20_000` input rows before applying
   the display limit.
9. Validate IDs with the JavaScript equivalent of
   `[A-Za-z0-9_-]{1,64}`.
10. Require preview and image keys to resolve to the same declared job and
    color artifact. Reject `..`, backslashes, braces, query strings, and
    cross-job/cross-artifact paths.
11. Require finite positive preview dimensions. A malformed row is skipped and
    counted; it must not poison the whole scene unless the manifest itself is
    structurally invalid.
12. Clamp every numeric query option. Reject `NaN`, infinities, negative limits,
    and extreme room dimensions.

Ignore `manifest.base` when constructing asset URLs. After key validation,
resolve every object against the already trusted `location.origin`. This is
safer than validating one URL and then concatenating a second caller-supplied
base.

All titles and metadata are untrusted. Render them with `textContent`, not
`innerHTML`.

## 5. Deterministic tile selection and layout

> Scope: this size/sort selection applies to the **AllCol source (Phase 2)**,
> where the gallery is derived from the whole mosaic. **Phase 0** curates
> explicit DeepZoom picks, so it needs only a stable order of the already-chosen
> pieces (Phase 0 step 5), not this filtering — the shared-helper packaging
> decision below is therefore a Phase 2 deliverable.

The viewer applies the selected size and sort before the piece cap. Reuse the
standalone mosaic viewer's semantics, but move the shared pure logic into a
tested helper rather than copy-pasting a third divergent implementation.

This extraction must have one dependency-free API that takes
`(tiles, {size, sort, seed})`; it must not read DOM controls or module-global
state. The main AllCol wall, `artifact_mosaic_viewer.html`, and the gallery
should all call it.

But "share one helper" is not implementable from the §12 file/deploy plan as
written: the three call sites load in three DIFFERENT contexts — the app as
build-versioned classic `js/*`, `artifact_mosaic_viewer.html` as an unstamped
standalone HTML, and the gallery as an ES module. §12 currently neither names a
shared ordering asset nor stamps the mosaic viewer to load a build-versioned
copy of it. Choose one explicitly and reflect it in §12:

- **True sharing** — publish the helper as a build-versioned asset, convert it
  to a form all three can load (e.g. a classic script exposing a global plus a
  thin ESM re-export), and add stamping so `artifact_mosaic_viewer.html` points
  at the versioned key (it is currently deployed unstamped).
- **Independent implementations** — accept three copies but gate them with
  explicit golden parity fixtures, and do NOT claim in the doc that they are
  shared.

Do not describe a shared helper while shipping three silently-divergent copies.

Required sort modes:

- `date`;
- `job`;
- `function`;
- `degree`;
- `N`;
- seeded `random`.

Tie-break every mode by stable identity `(job_id, artifact_id)`. The same URL
and manifest must always produce the same selected pieces and placement — on
every browser and locale. The current mosaic comparators use `localeCompare()`
(`js/13-artifact-mosaics.js:379-400`), which is locale-dependent and therefore
NOT cross-browser deterministic; the shared helper must use an explicit
code-unit comparison (`a < b ? -1 : a > b ? 1 : 0`, or `localeCompare` pinned to
a fixed locale + options). Likewise pass the exact `state.randomSeed` from the
launching app into the gallery URL — do not let the standalone viewer invent its
own seed.

### 5.1 MVP room generation

Use a rectangular room with four artwork walls:

- height: 3.6 m;
- artwork center: 1.65 m;
- nominal artwork width: 1.1 m;
- horizontal gap: 0.45 m;
- wall-end margin: 1.5 m;
- preview plane offset from wall: 0.025 m to prevent z-fighting;
- camera eye height: 1.65 m;
- camera collision radius: 0.30 m.

Distribute pieces round-robin across north, east, south, and west walls. Compute
room width and depth from the maximum pieces assigned to either pair of walls,
rather than squeezing or overlapping a fixed room.

**Watch the resulting room size.** With one row and these defaults, a wall
holding `p` pieces is `p·1.1 + (p−1)·0.45 + 2·1.5` m wide. 64 pieces → 16/wall →
a **27.35 m** square room; 96 → 24/wall → **39.75 m**. Those are cavernous and
defeat the "one intimate room" experiment the MVP is meant to test. Keep the
room human-scaled by doing one of:

- default to **32 pieces** (8/wall → ~14.95 m square) — the recommended MVP
  value (see §2.1);
- hang **two vertical rows** per wall (halving pieces-per-wall for the same
  count, e.g. 64 pieces → 8/wall → ~15 m); or
- introduce **multiple compact rooms** earlier (Phase 3) instead of one large
  hall.

Do not raise the piece cap before measuring how a room of that physical size
actually feels to walk.

For each piece:

```text
aspect = preview_width / preview_height
width_m = nominal artwork width
height_m = width_m / aspect
```

Reject or clamp dimensions that would put a piece outside the wall's vertical
bounds. The layout helper returns semantic wall placement, not arbitrary magic
world coordinates:

```json
{
  "wall": "north",
  "center_offset_m": -2.325,
  "center_y_m": 1.65,
  "width_m": 1.1,
  "height_m": 1.1
}
```

A single wall-to-world transform owns the north/south/east/west position and
rotation rules. Do not duplicate four ad hoc coordinate formulas in the render
loop and click code.

### 5.2 Geometry ownership

- Share one unit `PlaneGeometry` across artwork meshes and scale each mesh.
- Share frame geometry where dimensions permit; otherwise cache by quantized
  aspect ratio.
- Keep the artwork meshes in one explicit array for raycasting.
- Keep room collision bounds independent from visual wall meshes.
- Offset art and frame surfaces consistently to avoid z-fighting.

## 6. Rendering and color fidelity

Use Three.js with:

- `Scene`;
- `PerspectiveCamera`;
- `WebGLRenderer`;
- `PointerLockControls` for first-person look;
- `Raycaster` for crosshair selection.

Artwork must use `MeshBasicMaterial` with `toneMapped = false`, so neither
scene lighting nor the renderer's tone mapping shifts the rendered artifact
colors. Walls, floor, ceiling, and frames may use lit materials.

Set the renderer and textures to the correct sRGB color space for the pinned
Three.js version. Also:

- cap device pixel ratio, initially `Math.min(devicePixelRatio, 2)`;
- use a restrained antialias setting;
- avoid shadows in the MVP;
- set texture anisotropy to a bounded value derived from renderer capability;
- use a neutral placeholder material until a preview is resident.

The visual direction should be quiet and deliberate: warm-white walls, a dark
matte floor, restrained frames, and a minimal HUD. The art, not game-like chrome
or dramatic colored lighting, is the visual focus.

## 7. Texture manager: required, not optional polish

Do not create one `TextureLoader.load()` call per manifest tile at startup.
Implement a single `GalleryTextureManager` with explicit ownership.

Initial limits:

```text
max_in_flight = 4
max_resident_textures = 48
max_estimated_texture_bytes = 128 MiB
```

Required behavior:

1. At a throttled cadence, rank pieces by visibility, crosshair focus, and
   distance.
2. Queue only high-priority missing previews.
3. Load with a cancellable fetch path (`fetch` plus `AbortController`) rather
   than an uncancellable fire-and-forget call.
4. Decode into an image/bitmap and create the Three.js texture only if the
   request generation is still current.
5. Close stale bitmaps and dispose stale textures immediately.
6. Maintain an LRU of resident preview textures.
7. On eviction, restore the placeholder, call `texture.dispose()`, and release
   the decoded image resource where the browser API permits it.
8. Never evict the focused piece while its inspection UI is open.
9. Pause scheduling when `document.hidden` is true.
10. Expose counters in a debug HUD: queued, in flight, resident, estimated
    bytes, and `renderer.info.memory.textures`.

Frustum membership alone is insufficient. A near piece just outside the view
should remain warm briefly, while a far piece that happened to cross the
frustum should not displace the focused art.

Full artifact images do not enter this cache. They can exceed GPU texture
limits and are loaded only into the 2D inspection layer after an explicit user
action.

### 7.1 Failure handling (required, not implied)

The in-flight cap protects nothing if a stuck fetch can hold a slot forever, and
a missing preview must not be re-requested every scheduling pass:

1. Every preview fetch has a timeout (`AbortController` + a deadline). A timed-out
   or aborted load releases its slot.
2. Release the slot in a `finally`, on success, error, timeout, and cancellation
   alike — never only on the success path.
3. Transient failures (timeout, 5xx, network) get bounded retry with backoff and
   a small cap; after the cap, treat as failed for a cooldown window.
4. A permanent failure (404) goes into a NEGATIVE cache: show the labelled
   missing-art placeholder and do not requeue it. Clear the negative cache only
   on an explicit refresh.
5. A failed load must not starve visible pieces: failures free their slot
   immediately so the scheduler can advance to the next priority piece.

## 8. Navigation and interaction

### 8.1 Pointer-lock mode

- Click the viewport to lock the pointer.
- WASD moves on the floor plane.
- Mouse controls yaw and pitch with a bounded pitch.
- Movement is delta-time based and clamped after long frame stalls.
- Camera position is clamped to room bounds using its collision radius.
- `Escape` releases pointer lock and exposes controls.
- Clear all held-key state on `blur`, `visibilitychange`, and pointer-lock exit
  so a lost key-up event cannot leave the camera moving forever.

Pointer lock and mouse selection compete for the same click. While locked, use
a center crosshair and `Enter` or primary click to inspect the focused piece.
While unlocked, a click may lock the pointer or select through a clearly
labelled UI control; do not overload one event ambiguously.

### 8.2 Guided mode

Guided navigation is an MVP accessibility and usability path, not a distant
mobile feature.

- `Previous` and `Next` move to a safe viewing pose in front of each piece.
- An artwork index/search panel can select a piece directly.
- Camera transitions are short and cancellable.
- Respect `prefers-reduced-motion`; use an immediate jump when requested.
- `Home` returns to the spawn pose.

On touch/mobile, show the artwork index and guided mode if the tested browser
can support it. Otherwise show a clear unsupported message with a link back to
the 2D shared mosaic. Do not present a broken WASD-only canvas.

### 8.3 Piece focus

Raycast only the artwork plane meshes. Focus state should:

- highlight the frame, not recolor the image;
- show title/job/artifact in the HUD;
- identify the exact tile by stable `(job_id, artifact_id)`;
- remain valid after asynchronous texture completion;
- clear safely if a piece is removed during a future edit mode.

## 9. Inspection overlay

Selecting a piece opens a full-window 2D overlay. The scene pauses expensive
work while the overlay is active.

Initial overlay contents:

- preview image;
- job id and artifact id;
- function, degree, N, times, and creation date;
- `Load original` button using the validated `image_key`, guarded by a size
  check (see below);
- copyable job/artifact reference and source mosaic link;
- close control and keyboard focus trap.

Loading the original is explicit because full images can be large. It must also
be memory-safe: renders reach `RENDER_MAX_PIX = 32768` per side
(`js/09-render-orchestration.js:7`), and a square RGBA decode at that limit is
~32768 × 32768 × 4 ≈ **4 GiB** of browser memory — enough to crash the tab.
Before any inline decode:

1. `HEAD` the `image_key` and read `Content-Length` (and dimensions from object
   metadata when present).
2. Enforce an inline threshold on both byte size and, when known, pixel count.
3. Above the threshold, do NOT decode inline. Offer instead: exact-match DZI
   (if discovered), a plain download, or open-in-a-new-tab — never a raw
   `new Image()`/`createImageBitmap` on an unbounded object.

Report loading/error state, release the decoded image when the overlay closes,
and allow cancellation of an in-flight original load.

There is no current standalone-to-Render deep-link contract. Do not fake one
with `window.opener`, hidden global selection, or an unimplemented query string.
An `Open Render` action can be added later only after `index.html` owns and
tests an explicit `{tab, job_id, family, artifact_id}` startup contract.

### 9.1 DeepZoom: build-time generation (primary) vs discovery (fallback)

There are two ways a piece gets an exact `dzi_key`, and both record it in the
gallery manifest — the viewer never discovers at inspection time:

- **Curate-from-existing (Phase 0).** Pieces are picked from the DeepZoom tab, so
  each already has an export; `/share-gallery` (§3.1) records its `dzi_key`. No
  generation.
- **Generate-or-reuse (Phase 2).** For AllCol-curated pieces, the Create Gallery
  build reuses an existing export or generates the missing one, then records the
  `dzi_key` (§14.1).

The `deepzoom_latest.json` per-job pointer (below) is a **discovery fallback**,
used only for a piece that reaches the viewer with no recorded `dzi_key`; the
built manifests above never rely on it.

**The AllCol wall pyramid is NOT a per-piece zoom source.** `wall_dz`
arrayjoins the tiles into ONE composite image and dzsaves that — so (a) an
individual artifact is only a cell inside the big image, with no clean standalone
boundaries, and (b) the composite is built from the **512px previews**
(`handle_build_wall_pyramid` downloads each tile's preview `key`), so zooming
into it yields no more detail than the wall plane already shows. Real zoom
requires a per-artifact DZI built from the full image (`image.jpeg` OR
`image.png` — color artifacts support both; use the exact resolved `image_key`,
never an assumed extension).

For the discovery fallback, for an inspected piece only, the viewer may fetch:

```text
renders/<job_id>/deepzoom_latest.json
```

Offer DeepZoom only when all of these match:

- pointer `source_artifact_id === tile.artifact_id`;
- pointer `source_key === tile.image_key`;
- `dzi_url` is same-origin HTTPS under `/deepzoom/<job_id>/`;
- the pointer has finite positive width and height.

Cache the raw `deepzoom_latest.json` POINTER per job for the session, then
evaluate the match per artifact. Do NOT cache a per-job boolean decision: the
pointer is per-artifact (`source_artifact_id`), so if the latest pointer names
artifact B, inspecting artifact A first would record a job-level "miss" and then
wrongly suppress DZI for B. Either cache the pointer object keyed by `job_id`
(and re-run the match for each inspected artifact), or cache the boolean
decision keyed by the full `(job_id, artifact_id, image_key)` triple. A mismatch
means "no known DeepZoom for THIS piece," not an error and not permission to use
the job's latest export for a different artifact.

Use an OpenSeadragon overlay for DZI. Do not implement DZI tiles as dynamic
Three.js wall textures in this feature. That would require a separate tiled
projection/cache system and offers little benefit over the proven 2D viewer.

Because DZI inspection ships in Phase 0, OpenSeadragon is vendored as a pinned,
version-stamped dependency (§12.1) — not loaded from cdnjs like the current
`artifact_mosaic_viewer.html`. Decided, not optional.

An older matching export may exist even when the latest pointer does not match.
Discovering it efficiently requires a real `(job_id, artifact_id)` DeepZoom
index or a saved exact DZI reference. Do not hide this limitation with a global
inventory scan.

## 10. Lifecycle and failure handling

One owner object should manage the viewer lifecycle. `destroy()` must:

- cancel `requestAnimationFrame`;
- abort outstanding preview/original fetches;
- exit pointer lock if owned;
- call `PointerLockControls.dispose()`;
- remove keyboard, mouse, resize, visibility, and WebGL listeners;
- dispose every resident texture and unique material;
- dispose shared geometries exactly once;
- destroy any OpenSeadragon inspection viewer;
- release the loaded original image (revoke object URLs / drop the decoded
  bitmap) when the inspection overlay closes;
- dispose the renderer;
- clear overlay state and DOM references.

Handle:

- WebGL unavailable: show a readable fallback and the source mosaic link;
- `webglcontextlost`: prevent default, stop rendering, and report the state;
- `webglcontextrestored`: rebuild owned GPU resources or reload deliberately;
- individual preview 404: retain a labelled missing-art placeholder;
- manifest failure: show the specific validation/fetch error before creating
  the scene;
- resize: update renderer size and camera aspect without reallocating textures.

## 11. CORS and origin model

`deploy.sh` configures public reads but does not install S3 bucket CORS. API
Gateway CORS is unrelated to image texture fetches.

The production gallery link should use the same REST S3 origin as its manifest
and images, for example:

```text
https://polypaint.s3.us-east-1.amazonaws.com/gallery.html
```

With the page, manifest, and image keys on that same origin, S3 CORS is not
required. Keep asset URLs relative/same-origin where possible.

Local development or a future different-origin frontend will need an explicit
S3 `GET`/`HEAD` CORS rule. Do not add a broad wildcard rule pre-emptively and do
not assume API Gateway's response headers solve it.

## 12. Deployment integration

### 12.1 Files

Recommended mergeable structure:

```text
gallery.html
gallery/app.js
gallery/manifest.js
gallery/layout.js
gallery/texture-manager.js
vendor/three-r{NNN}/three.module.js
vendor/three-r{NNN}/addons/controls/PointerLockControls.js
vendor/three-r{NNN}/LICENSE
vendor/openseadragon-r{NNN}/openseadragon.min.js
vendor/openseadragon-r{NNN}/images/...
vendor/openseadragon-r{NNN}/LICENSE
```

OpenSeadragon is a **Phase 0** dependency (DZI inspection ships in Phase 0), so
vendor a pinned build under a version-stamped directory — do not add a second
floating cdnjs dependency while deliberately vendoring Three.js. The existing
`artifact_mosaic_viewer.html` loads OSD from cdnjs; the durable gallery share
must not.

Keep parsing/layout/cache policy in importable modules. Avoid a thousand-line
inline script in `gallery.html` merely because the older standalone viewers are
single files.

### 12.2 Build-versioned modules

`deploy.sh` currently build-versions `js/*` and generated `*_js.js`, while only
`index.html` is stamped to point at those keys. A standalone gallery module
will not work merely by adding an unmodified script tag.

Required deploy changes:

1. Add `gallery.html`, every `gallery/*.js` module, and the pinned Three.js files
   to `frontend_asset_keys()`.
2. Map `gallery/*.js` to `assets/${BUILD_ID}/gallery/...` in
   `deployed_asset_key()`.
3. Add a `stamped_gallery_html()` step that rewrites the module entry point to
   its build-versioned key.
4. Upload all gallery modules and vendor files before uploading the stamped
   root `gallery.html`.
5. Make `verify_frontend_assets()` compare deployed `gallery.html` against the
   stamped copy, not the source shell.
6. Pin Three.js under a VERSION-STAMPED path, e.g. `vendor/three-r{NNN}/...`,
   and bump the directory on every upgrade. A stable `vendor/three/...` key is
   NOT safe: `deployed_asset_key()` build-versions only `js/*` and generated
   vocab files (`deploy.sh:203`), so a stable vendor key is overwritten IN PLACE
   on an upgrade — the exact mixed-deploy race the build-versioning exists to
   prevent (a stamped gallery.html referencing new modules while the vendor blob
   is mid-overwrite, or a cached page pointing at changed bytes). Alternatively,
   add both Three.js files to the build-versioned set. Include the license.
7. Vendor OpenSeadragon fully (Phase 0 dependency): add its script AND **every
   control image** to `frontend_asset_keys()`; extend
   `frontend_asset_content_type()` to serve the `.png` control images correctly
   (the current asset set is HTML/JS/JSON); set the viewer's `prefixUrl` to the
   exact local `vendor/openseadragon-r{NNN}/images/` path; and have
   `verify_frontend_assets()` check every OSD object, not just the script.

Alternatively, generalize the existing index stamping code into a helper for
root HTML shells. Do not solve this by serving mutable application modules from
an unversioned key and reintroducing mixed-deploy races.

## 13. Gallery schemas

There are TWO `virtual_gallery` shapes with **incompatible** layouts, so they
must not share `manifest_type` + `schema_version` alone: add a
`document_kind: "share" | "editable"` discriminator (or use distinct
`manifest_type`s). The viewer selects its loader on `document_kind`. A share is
auto-layout + ordinals; an editable gallery carries explicit rooms/placements,
and `/create-gallery-share` snapshotting one MUST preserve that placement (never
downgrade a curated arrangement to the auto-layout share shape).

### 13.1 Immutable share schema (what `/share-gallery` writes — Phase 0/2)

This is the document the viewer actually loads. `layout.mode` is either:

- **`"auto"`** — placement derived from `layout.seed` (no per-piece
  coordinates), pieces ordered by curator `ordinal`. This is what `/share-gallery`
  writes for Phase 0/2 picks.
- **`"explicit"`** — the immutable snapshot of a *saved, arranged* gallery
  (`/create-gallery-share`, §14). Here `layout` also carries the frozen
  `rooms[]` and each piece a `placement` (`room_id`, `wall`, `center_offset_m`,
  `center_y_m`, `width_m`, orientation, frame). This is REQUIRED so sharing an
  edited gallery preserves its arrangement instead of silently re-auto-laying it.

Pieces are ordered by `ordinal`; `source.kind` reflects where the picks came
from; and it carries a top-level `artifact_kind` (which §4.1 validation
requires).

```json
{
  "schema_version": 1,
  "manifest_type": "virtual_gallery",
  "document_kind": "share",
  "artifact_kind": "color",
  "created_at": "2026-...",
  "source": { "kind": "deepzoom_selection", "share_id": "share_..." },
  "layout": { "mode": "auto", "seed": 1 },
  "pieces": [
    { "ordinal": 0, "job_id": "compute_...", "artifact_id": "color_...",
      "preview_key": "renders/.../preview.jpg", "image_key": "renders/.../image.jpeg",
      "preview_width": 512, "preview_height": 512,
      "function": "...", "degree": 12, "N": 2048, "times": 1, "created_at": "...",
      "deepzoom": { "export_id": "...", "dzi_key": "deepzoom/<job>/<export>/image.dzi",
                    "source_key": "renders/.../image.jpeg", "source_artifact_id": "color_..." } }
  ]
}
```

`source.kind` is `deepzoom_selection` (Phase 0 DeepZoom-tab picks) or
`allcol_selection` (Phase 2 AllCol picks) — never a hardcoded `shared_mosaic`.
`deepzoom` is validated on write (§3.1 step 4) AND re-validated by the viewer on
load — the viewer must not trust the manifest blindly. For each present
`deepzoom`, require: `export_id` matches the safe-id charset; `dzi_key` equals
**exactly** `deepzoom/<job_id>/<export_id>/image.dzi` (constructed from the
validated ids, not read as a free string); `source_key === piece.image_key`; and
`source_artifact_id === piece.artifact_id`. Ignore any absolute DZI URL stored in
the document — build the OpenSeadragon tile-source URL from the validated
`dzi_key` against the trusted origin only. A piece failing any check is treated
as zoomless.

### 13.2 Editable saved schema (later — the Gallery tab)

When interactive curation and per-piece arrangement arrive, a saved gallery adds
mutable metadata and EXPLICIT placement, rather than the original ambiguous
`wall/x/y/scale/rot` rows. It uses the SAME `source.kind` enum as the share
schema (a saved gallery built from a DeepZoom selection has
`kind: "deepzoom_selection"`, not a hardcoded `shared_mosaic`) and carries the
same compact per-piece display fields (`function/degree/N/times/…`) so it stays
self-contained:

```json
{
  "schema_version": 1,
  "manifest_type": "virtual_gallery",
  "document_kind": "editable",
  "artifact_kind": "color",
  "gallery_id": "gallery_...",
  "name": "Selected works",
  "created_at": "2026-...",
  "updated_at": "2026-...",
  "source": {
    "kind": "deepzoom_selection",
    "share_id": "share_..."
  },
  "settings": {
    "spawn_room_id": "room_1",
    "navigation": "guided",
    "style": "warm_white"
  },
  "rooms": [
    {"room_id": "room_1", "width_m": 14, "height_m": 3.6, "depth_m": 14}
  ],
  "pieces": [
    {
      "piece_id": "piece_1",
      "room_id": "room_1",
      "job_id": "compute_...",
      "artifact_id": "color_...",
      "preview_key": "renders/.../preview.jpg",
      "image_key": "renders/.../image.jpeg",
      "preview_width": 512,
      "preview_height": 512,
      "function": "...", "degree": 12, "N": 2048, "times": 1, "created_at": "...",
      "title": "...",
      "placement": {
        "wall": "north",
        "center_offset_m": -2.325,
        "center_y_m": 1.65,
        "width_m": 1.1,
        "frame_style": "thin_black"
      },
      "deepzoom": null
    }
  ]
}
```

Every piece carries the same compact display fields as the share schema
(`function/degree/N/times/created_at`) plus its explicit `placement`; `title` is
the only optional field.

`deepzoom` is populated by the Create Gallery build step (§14.1) — reused if an
export already exists, generated if not (color renders are square in practice,
but the build validates `width == height` and fails soft rather than assuming
it), `null` only if the export otherwise fails. When present it must contain an
exact `export_id`, `dzi_key`, `source_key`, and `source_artifact_id`; validate
the identity on load (the same
`source_artifact_id === artifact_id` and `source_key === image_key` check as the
discovery fallback).

Do not persist arbitrary absolute URLs, raw world matrices, `NaN`, infinities,
or unconstrained dimensions.

## 14. Persistence and sharing: later phase

### 14.1 Exhibition curation and build flow

A saved gallery is an *exhibition* — a deliberately curated set, not an
auto-selected AllCol subset. Two-step UX:

1. **Add to Gallery** — a per-tile action on the AllCol wall (alongside the
   existing Favorite / Add-to-Book tile actions in `js/13-artifact-mosaics.js`).
   It appends `(job_id, artifact_id)` + the tile's compact display fields to a
   draft exhibition list. Curation is browsing the wall and adding pieces.
2. **Create Gallery** — finalizes the draft: for each piece, ensure a
   per-artifact DeepZoom exists, record its exact key, write the gallery
   manifest, and open `gallery.html`.

The Create step's DZI pass is a **bounded build orchestrator**, not a browser
loop — the naive contract is unsafe:

- **Reuse via an exact map.** Call `/list-deepzoom` once and build a
  `(job_id, artifact_id, source_key) → export` map (the inventory carries
  `source_artifact_id`/`source_key`; `deepzoom_latest.json` only knows the latest
  per job, so it cannot find an older sibling export). A piece with a map hit is
  reused; only the misses are generated. **Caveat:** `handle_list_deepzoom`
  currently swallows a failed `meta.json` read and drops that export
  (`handler_storage.py:5420`), so a transient S3 error looks like "not exported"
  and triggers an expensive duplicate. Before relying on it for reuse, make
  `/list-deepzoom` surface per-export error counts/reasons (the CR28 F13
  taxonomy) and retry, or add a durable per-artifact DeepZoom index. Key the
  reuse map on the full `(job_id, artifact_id, source_key)` triple, not just
  `(job_id, artifact_id)`. When a key has **multiple** exports (`/list-deepzoom`
  is newest-first), the rule is **newest VALID wins** — newest whose meta
  identity matches AND whose DZI `HEAD` succeeds — never "whichever the dict
  iteration kept last."
- **Deterministic, collision-free IDs (requires an ATOMIC handler + caller
  migration).** `handle_deepzoom_export_request` reads `export_id` from params
  but `task_id` is hardcoded to `"deepzoom_export"` (`handler_deepzoom_export
  .py:344`). Two same-job exports collide in the S3 prefix (same-second default
  `export_id`) AND the DDB status row (identical `task_id`). Fix the handler to
  derive `task_id = f"deepzoom_export_{export_id}"` (or read+`assert_safe_id` a
  caller id) — but this **breaks the existing Render DeepZoom button** unless it
  migrates in the same change: today `js/11-artifacts.js` sends no `export_id`,
  fixes `task_id = "deepzoom_export"`, deletes that one row, and polls the broad
  `deepzoom_export` prefix with `expected: 1`. Once unique rows accumulate, that
  `expected: 1` can match a stale completed row. The migrated caller must
  generate the `export_id`, derive the exact `task_id`, send both, delete that
  exact row, and poll that exact `meta.json` key. Ship handler + Render caller +
  orchestrator together.
- **Workers are NOT computationally idempotent from a deterministic id alone.**
  A unique id prevents key collisions, but the handler still downloads and
  regenerates immediately after reporting `started` — it does not check for an
  existing valid `meta.json` or take an export-level claim. A retried async
  `Event` invoke therefore re-runs the same 8 GiB job and races the upload. Add a
  validated existing-output fast path (skip if a good `meta.json` already exists)
  and/or a per-export lease before the handler regenerates.
- **Completion depends on the executor (they are two DIFFERENT models — pick
  one, §Concrete architecture).** With the preferred Step Functions model the
  Map invokes the DeepZoom Lambda SYNCHRONOUSLY and receives its result
  (`handle_deepzoom_export_request` returns `{export_id, dzi_url, …}`) — no
  dispatcher, no polling. Only the fallback coordinator uses the async dispatcher
  (which returns acceptance counts, not `dzi_key`s — CR28 F7) and therefore polls
  each export's exact `deepzoom/<job>/<export_id>/meta.json`. Do NOT combine
  sync-invoke with async-poll: that gives ambiguous completion/error semantics.
- **Bounded concurrency.** The DeepZoom Lambda is 8192 MiB
  (`deploy_manifest.json:332`); firing 32 misses at once is a ~256 GiB burst.
  Limit to 2–4 concurrent, like the existing render/palette orchestrators.
- **Real orchestration state.** Persist per-item progress; idempotent retries on
  transient failure; resume a partially-built gallery; cancellation; publish the
  gallery manifest only after a terminal state, recording `deepzoom: null` for
  any piece that ultimately failed (it degrades to the 512 preview, does not
  block the build). Renders are square in practice, but validate it — the
  enrichment already HEADs the image, so check `width == height` and fail that
  piece soft rather than assuming it across every family (`_mosaic_tile_from
  _entry` itself has a `non_square` skip).
- Persist each resolved `dzi_key` (+ `export_id`, `source_key`,
  `source_artifact_id`, and the exact `image_key`/`preview_key`) in
  `pieces[].deepzoom` (§13.1) via `/share-gallery` (§3.1), so the viewer never
  re-discovers at inspection time.

**Concrete architecture.** A single Lambda CANNOT own this build: Lambda caps at
`TIMEOUT=900` s (`deploy.sh:32`), while 32 exports at 60–120 s each with 2–4
concurrency run ~15–32 minutes. Use one of:

- **Step Functions (preferred)** — model on the existing
  `polypaint-{render,compute,palette}-workflow` ASL machines. A Map state with
  `MaxConcurrency: 2–4` **synchronously** invokes the DeepZoom Lambda per miss
  and consumes its returned `{export_id, dzi_key}`; native `Retry`/`Catch` handle
  transient failure and record `deepzoom: null` on a final catch; execution state
  is owned by the machine (no manual lease, no polling). This resolves timeout,
  concurrency, ownership, and resume in one model.
- **Self-reinvoking coordinator Lambda (fallback only)** — dispatches a bounded
  round ASYNCHRONOUSLY, polls exact `meta.json` keys, persists progress, and
  re-invokes itself for the next round so no invocation exceeds 900 s. Needs the
  explicit lease/heartbeat below. Use only if Step Functions is undesirable.

State + recovery (whichever executor):

- **State**: one DDB partition per build, `job_id = gallery_build#<build_id>`, a
  `__meta__` row (`queued|building|ready|error`, counts) and one row per piece
  (`pending|generating|done|failed`, `export_id`, `dzi_key`).
- **Lease, not a permanent claim**: a bare `attribute_not_exists` claim strands
  the build forever if the owner dies mid-flight. Use an owner token + heartbeat
  + lease expiry, with a conditional stale-takeover — OR let Step Functions own
  execution state and skip the manual claim entirely.
- **Concurrency**: ≤ 2–4 exports in flight (not a 32-wide 256 GiB burst).
- **Routes** (POST only — the API is `POST`/`OPTIONS`, `deploy.sh:1697`):
  `POST /create-gallery` (start → `build_id`), `POST /gallery-build-status`
  (poll, like `/check-status`), optional `POST /cancel-gallery-build`.
- **Terminal publication is internal, not a self-HTTP call.** A Lambda must not
  POST its own public API. Extract a pure/internal share-builder (the
  manifest-write core of §3.1) and call it directly, or invoke storage with an
  `internal_action` (the pattern `handler_storage` already uses for
  `build_color_mosaic`). Publish only after every piece is `done|failed`, then
  flip `__meta__` to `ready`.

**Phase 2 deploy infrastructure (Step Functions path)** — enumerate it as
explicit deploy steps + tests, modelled on the existing
`stepfunctions/*_workflow.asl.json.template` machines (`deploy.sh:334`), not
inferred:

- a `stepfunctions/gallery_build.asl.json.template` ASL definition;
- a `polypaint-gallery-workflow` state machine created/updated at deploy (name +
  ARN wired like `RENDER_STATE_MACHINE_ARN`), with idempotent update/teardown;
- an execution role scoped to `lambda:InvokeFunction` on the DeepZoom function
  only;
- storage-route IAM permission for `states:StartExecution` +
  `states:DescribeExecution` (so `/create-gallery` starts and
  `/gallery-build-status` reads it);
- `deploy_manifest.json` entries + a `tests/test_deploy_packaging.py` assertion
  and predeploy membership for all of the above.

Do NOT reuse the AllCol wall pyramid as the zoom source (§9.1): it is one
preview-resolution composite with no clean per-artifact boundaries.

For a truly archival share, the same Create/Save step is where you would also
copy each piece's preview, full image, and DZI tiles under the gallery's own
share prefix — otherwise the share still breaks when a source artifact is
deleted (see below).

A first-class Gallery tab requires more than `/save-gallery` and
`/fetch-gallery`. It needs the full saved-object lifecycle:

- `/list-galleries`;
- `/fetch-gallery`;
- `/save-gallery`;
- `/delete-gallery`;
- `/create-gallery-share` for an immutable snapshot of a *saved* gallery
  revision — a DISTINCT route from `/share-gallery` (§3.1), which creates a share
  directly from an explicit pick list. Do not overload one route with both
  meanings; if they must share a handler, gate on a strict mutually-exclusive
  mode field.

Suggested mutable key:

```text
polypaint/galleries/<gallery_id>/gallery.json
```

Suggested immutable share key:

```text
polypaint/gallery-shares/<gallery_id>/<share_id>/manifest.json
```

Apply `assert_safe_id` semantics to gallery and share ids. Validate every key
and numeric field server-side, not only in JavaScript.

`/save-gallery` should return the S3 ETag as an opaque revision and accept
`expected_revision` using `IfMatch`, following the Book save path. A share must
pin a specific saved revision rather than opening a mutable `?g=<id>` document.

The current stack is intentionally public-read and unauthenticated for writes.
That means saved gallery mutation/deletion is not private. State this explicitly
before exposing management routes in a shareable product.

Again, an immutable layout snapshot still references mutable/deletable source
objects. A truly archival gallery would need to copy or retain every referenced
preview, image, and DZI export under the share prefix. That is a separate cost
and lifecycle decision.

Current `/share-mosaic` snapshots also have no automatic retention policy. A
Gallery launch creates another snapshot unless the frontend deliberately reuses
an identical in-session result. Measure that growth before making Gallery the
default browsing action; do not prune links that users reasonably expect to
remain shareable without first defining an expiry contract.

## 15. Implementation phases

### Phase 0: code-grounded visual MVP

**Curate from the DeepZoom tab (existing exports only).** The first source is
NOT the AllCol wall — it is an "Add to Gallery" action on the existing DeepZoom
inventory (`tab-deepzoom` / `/list-deepzoom`), where every listed export already
carries `job_id`, `export_id`, `source_artifact_id`, `source_key`,
`dzi_key`/`dzi_url`, and dimensions. This lets Phase 0 build and test the ENTIRE
viewer — 3D wall, texture manager, navigation, raycast focus, and *real DZI zoom
inspection* on the existing `dzi_key` — with no generation pipeline, no AllCol
curation, and no Create-Gallery batch. It also exercises the exact per-piece
data model (`dzi_key` per piece) that the full flow (§14.1) will produce. Defer
AllCol curation + generate-missing to Phase 2.

1. Pin and vendor Three.js + `PointerLockControls` + OpenSeadragon (§12.1);
   extend the deploy asset set, content types, and verify for all three.
2. Add the standalone gallery shell/modules and deploy stamping (§12.2).
3. **Implement and deploy the `/share-gallery` route** (§3.1): add it to the
   storage handler router, run `api_manifest.py --write`, wire it in
   `deploy_manifest.json`, add packaging checks, and add the route + its tests to
   `scripts/predeploy_check.sh` — it is not optional plumbing, Phase 0 cannot open
   a gallery without it.
4. Implement same-origin, per-type gallery-manifest validation (§4/§4.1).
5. Add a DeepZoom-tab "Add to Gallery" action → draft list (preserving add
   order); on open, POST the picks to `/share-gallery`, then open
   `gallery.html?manifest=<returned url>`. `/share-gallery` does the color
   filtering, exact-key enrichment, export validation, and manifest write
   server-side (§3.1) — the client does not resolve or reconstruct any key.
6. Order pieces by curator `ordinal` (identity as tie-break); the picks are the
   selection, so the mosaic size/sort filtering (§5) is NOT needed here.
7. Implement deterministic auto-layout (≤32 pieces; §2.1/§5.1 room-size).
8. Implement room rendering with placeholder artwork materials.
9. Implement the bounded preview texture manager.
10. Implement pointer-lock and guided navigation.
11. Implement raycast focus, the inspection overlay, and OpenSeadragon DZI on
    each piece's recorded `dzi_key` (real zoom from day one — no full-image
    inline decode needed).
12. Add lifecycle cleanup, failure UI, and tests.

Exit criterion: a gallery assembled from existing DeepZoom exports opens
reproducibly, gives real zoom on every piece, and stays within the texture
limits — proving the experience before any generation pipeline exists.

### Phase 1: inspection refinement

(DZI inspection via each piece's recorded `dzi_key` and the OpenSeadragon
overlay already land in Phase 0, since Phase 0 curates from existing exports.)

1. Artwork index/search and richer metadata.
2. Benchmark and tune texture/piece limits against a real gallery.
3. The `deepzoom_latest.json` discovery fallback — only needed for a piece with
   no recorded `dzi_key` (e.g. a future ephemeral AllCol launch); the built
   exhibitions in §14.1 don't rely on it.
4. Optional AllPal adapter only after deciding how palettes should be framed
   and described.

### Phase 2: curation and persistence

1. Define and validate the saved gallery schema.
2. Add the AllCol per-tile "Add to Gallery" action and a draft exhibition list.
3. Add "Create Gallery": ensure/reuse a per-artifact DZI for each piece
   (dispatch `deepzoom-export` for misses), record exact `dzi_key`s, save (§14.1).
4. Add Gallery tab for selecting and arranging pieces.
5. Add list/fetch/save/delete with ETag CAS.
6. Add immutable share snapshots.
7. Add interactive wall placement with snapping and overlap checks.

### Phase 3: scale

1. Multiple generated rooms/wings.
2. Activate only the current and adjacent rooms.
3. Dispose inactive room materials/textures.
4. Add a compact map and room/artwork jump list.
5. Consider preview atlases or instancing only after profiling proves the
   current mesh/material model is the bottleneck.

## 16. Tests and gates

### 16.1 Pure unit tests

- Trusted manifest URL accepts only the expected same-origin share prefix.
- Manifest byte, row, id, key, dimension, and type limits are enforced.
- Cross-job/cross-artifact keys are rejected.
- Every sort mode has deterministic tie-breaking.
- Random ordering is stable for a seed and changes for a different seed.
- Limit clamping happens after filtering/sorting.
- Auto-layout is deterministic, in bounds, and non-overlapping.
- Wall-to-world transforms face inward on all four walls.
- Manifest byte cap is enforced from `Content-Length`/stream, not after fully
  buffering; an extra-segment share path is rejected.
- Sort comparators are code-unit stable and identical across locales.
- Texture queue respects in-flight and resident limits.
- A timed-out/aborted preview fetch releases its slot; a 404 preview is
  negative-cached and never requeued.
- A stale async completion cannot attach to an evicted/destroyed piece.
- Eviction and destroy dispose each texture exactly once.
- DeepZoom pointer mismatch never enables DZI, and a per-artifact miss does not
  suppress DZI for a sibling artifact whose pointer matches.
- `Load original` refuses an inline decode above the size threshold and offers
  DZI/download/open-in-tab instead.

### 16.2 Browser tests

- The Gallery action creates one snapshot and opens the gallery URL.
- The page shows a useful validation error for a malicious manifest URL.
- WebGL-unavailable fallback links to the 2D mosaic.
- Pointer-lock exit, blur, and hidden-page transitions clear movement keys.
- Guided Previous/Next focuses the expected stable artifact.
- Original-image request is not made until the explicit action.
- Closing inspection aborts an in-flight original-image load.
- Destroy removes listeners and leaves no active animation frame.

### 16.3 Deployment tests

- `tests/test_deploy_packaging.py` pins `gallery.html`, gallery modules, Three.js
  assets, stamping, upload order, and verification behavior.
- The deployed smoke test fetches the stamped gallery shell and every imported
  module.
- Add the new unit, frontend, and deployment tests explicitly to
  `scripts/predeploy_check.sh`; do not rely on their mere presence under
  `tests/`.
- Vendored OpenSeadragon (Phase 0) is pinned, version-stamped, and packaged like
  Three.js; the deploy verify covers it.

### 16.4 Backend route and build-orchestrator tests

The DeepZoom-first pivot adds backend surface that needs gated coverage:

- `/share-gallery` writes a `virtual_gallery` manifest and rejects: non-color /
  legacy picks, unsafe ids, an over-cap list, and caller-reconstructed keys.
- Enrichment resolves the exact `preview_key`/`image_key` (`.jpeg` *or* `.png`)
  and display fields by exact key — no path reconstruction, no `/render-summary`
  fan-out.
- The viewer accepts a `virtual_gallery` only at the `/gallery/` path and an
  `artifact_mosaic` only at the `/color/` path; a type/path mismatch is rejected.
- Build orchestrator (Phase 2): two artifacts of the same job get **unique**
  `export_id` + `task_id` (no S3-prefix or DDB-status collision); concurrency is
  capped at 2–4; transient failures retry idempotently; a partial build resumes;
  the gallery manifest publishes only after a terminal state; a permanently
  failed piece records `deepzoom: null`.
- Exact reuse: one `/list-deepzoom` pass builds the
  `(job_id, artifact_id, source_key)`→export map and a pre-existing sibling
  export is reused, not regenerated; a transient `meta.json` read error does NOT
  read as "not exported" (no false duplicate).
- Completion is detected by polling the exact `meta.json` key, not the
  dispatcher's acceptance count.

## 17. Risks and mitigations

| Risk | Mitigation |
|---|---|
| GPU memory growth | Hard piece cap, bounded texture LRU, explicit disposal, debug counters |
| Network burst | Priority queue, four in-flight loads, no startup fan-out |
| Wrong DeepZoom art | Exact artifact and source-key match; otherwise no DZI |
| Mixed deployment | Build-versioned gallery modules and stamped shell |
| Untrusted share URL | Same-origin and exact-prefix validation before fetch |
| Source artifact deleted | Missing-art placeholder; document non-archival shares |
| Pointer-lock discomfort | Guided mode, artwork index, reduced-motion support |
| Lost input event | Clear keys on blur, visibility, and pointer-lock exit |
| WebGL context loss | Bounded resources, context-loss UI, deterministic rebuild/reload |
| Mobile incompatibility | Guided fallback or explicit return-to-mosaic message |

## 18. Remaining product decisions

These are real choices, but none blocks Phase 0:

- Final default and hard piece limits after profiling.
- Frame styles and whether the selected frame is persisted per piece.
- Whether AllPal belongs in the same viewer or a separately styled gallery.
- Whether a shared gallery is allowed to break when source artifacts are
  deleted or must become an archival copy.
- Whether to add a real per-artifact DeepZoom index for older matching exports.
- Whether guided mode, not pointer lock, should be the default entry mode.

## 19. Recommended next step

Implement Phase 0 from a DeepZoom-tab curation via `/share-gallery` (§3.1), but
treat it as a bounded 3D viewer rather than a new universal asset browser. The
first proof should answer three questions with measured data:

1. Does walking and guided hopping make the corpus meaningfully easier or more
   enjoyable to browse than the 2D wall?
2. How many resident 512 previews can the target browser sustain without frame
   instability or context loss?
3. Is the 2D inspection overlay sufficient, or does exact-match DZI discovery
   materially improve the experience?

If those answers are positive, the saved Gallery tab and multi-room work are
well-justified follow-ons. If not, the experiment remains isolated and does not
burden the storage model.
