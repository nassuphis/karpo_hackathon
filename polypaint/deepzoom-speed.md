# deepzoom-speed.md — making the mosaic walls (and DeepZoom) fast

Plan for fixing the slow load of the AllCol/AllPal mosaic walls and polishing the
DeepZoom tab. Everything below is grounded in measurements taken against the live
bucket on 2026-07-07; nothing is guessed.

The headline: the walls are slow because they fetch **every artifact's full
`preview.png`** — ~1.7 GB for AllCol — six requests at a time, with no HTTP
caching. The DeepZoom pyramids themselves are healthy (~18.5 MB total for a
whole export). The fix is (1) proper cache headers, (2) JPEG previews with a
**migration for all ~2,800 existing previews**, and (3) JPEG tiles for new
DeepZoom exports. Composite wall pyramids, thumbs, and CloudFront are sketched
as deferred follow-ups.

---

## 0. Measured baseline (2026-07-07)

### The walls

| Surface | Tiles | Format | Sizes (sampled via head-object, n=12/bucket) | Est. total transfer |
|---|---|---|---|---|
| AllCol | 1,651 | all PNG | 896 @ 512px: median 467 KB (156–689) · 755 @ 1024px: median 1,719 KB (616–2,797) | **~1.7 GB** |
| AllPal | 1,131 | all PNG | 1,128 @ 512px + 3 @ 500px: median 381 KB (153–722) | **~431 MB** |

Manifests measured: `renders/_index/color_mosaic/mosaic_20260706T200856Z_94d883da/all.json`
and `renders/_index/palette_mosaic/mosaic_20260706T200851Z_f7760460/all.json`.
The wall is a single-level OpenSeadragon grid (`minLevel: 0, maxLevel: 0`,
js/13-artifact-mosaics.js:424-444): every visible cell fetches that artifact's
full preview, even when the cell paints at ~20 px on screen.

Two data-quality problems ride along (verified by sampling live objects):

- **Mixed preview sizes, color family**: the 755 older color previews are
  1024px, the 896 newer ones 512px. The wall's cell size is
  `max(preview_width)` (js/13-artifact-mosaics.js:446-456), so the old 1024s
  set the grid size and carry 4× the pixels. Palette previews are already all
  ≤512 actual pixels (1,128 @ 512 + 3 @ 500 where full_n < 512).
- **Lying object metadata, legacy color previews**: old color `preview.png`
  objects carry x-amz-meta `width`/`height` of the FULL render (5000, 10000 —
  sampled live), not the preview. Newer previews carry no dim metadata;
  palette previews carry none; autolevels stamps its true 512. finalize_mt
  and recolor upload previews with no Metadata (handler_finalize_mt.py,
  color_recolor_raw.py) — but Phase 1 implementation found TWO producers
  still stamping full-image dims on previews (handler_resize_artifact.py and
  handler_png_export.py); both now stamp measured preview dims at source, so
  the backfill repair can't be re-polluted. It's also why
  `_mosaic_preview_dimensions` (handler_storage.py:3288) must do a ranged-GET
  PNG-header parse per artifact instead of trusting metadata — its comment
  documents exactly this.

### Three multipliers on top of the payload

1. **HTTP/1.1 only.** S3's REST endpoint doesn't negotiate h2 (verified with
   `curl --http2`), so the browser caps at 6 concurrent downloads per host.
   The `imageLoaderLimit: 16` at js/13-artifact-mosaics.js:411 can never be
   reached.
2. **No `Cache-Control` anywhere.** Live HEAD on a preview returns only
   ETag + Last-Modified. Browsers fall back to heuristic caching (~10% of
   object age), so recently rendered artifacts effectively re-download on
   every wall open.
3. **PNG for noisy art.** Fractal-detail content is pathological for lossless
   compression — 1.7 MB for a 1024px preview.

### DeepZoom tab is NOT the problem

Newest export (`deepzoom/compute_mr97mt4v/dz_1783349495`, 5120² source):
548 tiles, **18.5 MB total**, deepest level averages 34 KB/tile. A first
viewport touches ~1 MB. Minor drags only: same missing cache headers, the
6-connection cap, and the viewer being destroyed/recreated on every inventory
row click (js/12-deepzoom-boot.js:265-267).

### Bucket scope (for the migration/backfill design)

| Prefix / pattern | Objects |
|---|---|
| `renders/` total | 1,222,600 (dominated by fragments/sidecars — **never blanket-copy this prefix**) |
| `renders/*/color/*/preview.png` | **1,651** |
| palette previews (`renders/*/palette*/.../preview.png`) | ~1,131 in-manifest (1,230 keys match `palette.*preview`) |
| `deepzoom/` total | 983,705 across 245 exports (avg ~4,000 tiles each; the 548-tile sample in §0 is on the small side) |

---

## 1. Goals / non-goals

Goals:
- Wall opens in a few seconds cold, near-instant warm. No visual downgrade a
  human notices at wall/zoom scale.
- **Zero feature regression**: sort modes (job/function/degree/N/random reseed),
  size filter, click-through, context menus, share viewers all keep working.
- **Wire formats stay frozen — additive only.** `preview_key` keeps meaning the
  PNG everywhere; the JPEG arrives as a NEW sibling key + NEW meta field.
- **Existing renders migrate.** 1,651 color + ~1,131 palette previews (the
  "1600!!!" — actually ~2,800 including palettes) get JPEG siblings and cache
  headers without breaking anything mid-migration.
- Deploys stay user-only. Claude writes code/scripts; the user deploys and
  green-lights each migration run (dry-run first).

Non-goals (deferred, sketched in §7): composite wall DZI pyramid, thumbnail
layer with zoom-gating, CloudFront.

---

## 2. Design

### 2.1 Additive `preview_jpg_key` in artifact meta

Each artifact's overlay `meta.json` (the same overlay mechanism
`load_color_artifact_head` merges — proven by the book palette fix) gains an
optional field:

```json
{ "preview_jpg_key": "renders/{job}/color/{art}/preview.jpg" }
```

The JPEG lives NEXT TO the PNG. The PNG is never deleted or repointed — every
existing consumer (Results, Favorites, Book hydration, share viewers) keeps
working untouched.

### 2.2 Manifest builder prefers the JPEG

Single seam: `_mosaic_tile_from_entry` (lambda/handler_storage.py:3353) emits
`"key": preview_jpg_key or preview_key`. The entry comes from
`_render_family_entry_from_prefix` (call site handler_storage.py:3443-3450) —
verify it passes overlay fields through; if it whitelists, add the field.
`preview_width/height` logic is unchanged (dims are read from the PNG and the
JPEG is generated at identical dimensions). Palette side is symmetric in
`_build_palette_mosaic_manifest` (handler_storage.py:3517, seam at :3600).

Wall JS and `artifact_mosaic_viewer.html` treat `tiles[].key` as opaque —
**no frontend change needed**. Mid-migration is safe by construction: tiles
with a JPEG load fast, the rest still load their PNG.

### 2.3 Cache policy

| Class | Keys | Header |
|---|---|---|
| Immutable artifacts (id-scoped keys, written once) | `preview.png`, `preview.jpg`, `deepzoom/*/image.dzi`, `deepzoom/*/image_files/**`, `deepzoom/*/viewer.html` | `Cache-Control: public, max-age=31536000, immutable` |
| Mutable pointers/meta | `renders/{job}/deepzoom_latest.json`, artifact `meta.json` overlays (recolor/autolevels rewrite them), `calc.json` | untouched (default) |
| Mosaic manifests | `renders/_index/**` | already `no-cache, max-age=0` (handler_storage.py:3831) — correct, keep |

### 2.4 JPEG encoding spec + size normalization

Pillow: `quality=92, subsampling=0 (4:4:4), optimize=True`, and the JPEG is
**normalized to ≤512px** (Lanczos downscale, never upscale) for BOTH families.
That collapses the 1024/512 split: the 755 old 1024px color previews shrink
4× in pixels on top of the format win; palettes are already ≤512 so it's a
pure format conversion there. 4:4:4 protects hard edges in palette grids from
chroma smearing.

Consequences by design:
- The wall becomes a uniform 512 grid (`_mosaicTileSize` = max over tiles);
  the AllCol size-filter buckets collapse to one — data-driven UI, no JS
  change.
- Expected totals: AllCol ~1.7 GB → **~100–150 MB**, AllPal ~431 MB →
  **~60–80 MB**.
- `preview.png` originals keep their resolution untouched for every other
  consumer.

Render-first discipline applies: the migration's dry-run emits before/after
pairs for visual inspection **before** the full sweep (§4 runbook) — the
inspection now also judges 1024→512 on wall-typical zoom.

### 2.5 Legacy metadata repair (rides the same passes)

Fix the lying x-amz-meta on legacy color previews where we're already
touching every object:

- The Phase-1 header backfill copies previews in place with
  `MetadataDirective=REPLACE`; before each copy it reads the true dims via
  ranged-GET PNG-header parse (same trick as handler_storage.py:3288) and
  rewrites `width`/`height` to the measured values. One copy = headers +
  honest metadata.
- The Phase-2 migration uploads `preview.jpg` with correct object metadata
  and writes `preview_jpg_width`/`preview_jpg_height` into the overlay meta,
  so the manifest builder can skip its per-artifact ranged GET for migrated
  tiles (mosaic refresh gets faster as a side effect).
- Checklist before the repair run: grep the hydration/head paths
  (`_head_artifact_keys` user_meta consumers, Results/Render detail JS) for
  anything DISPLAYING preview `width` metadata — a display that today shows
  "5000" by accident would change to the honest 512. **Done (2026-07-07):
  nothing in js/ reads `user_meta` at all; handler_storage.py:4663-4667
  copies head user_meta dims into the head info dict but no UI displays it
  for previews. Repair is safe.**

### 2.6 New DeepZoom exports get JPEG tiles

`dz_export.c:45` suffix `".png"` → `".jpg[Q=90]"` (vips encodes per-tile;
DZI `Format` attribute follows the suffix automatically). Old exports keep
their PNG `.dzi`s and remain valid forever — no re-export needed at 18.5 MB
each; **no pyramid migration required**.

---

## 3. Phase 1 — Cache-Control (code + targeted backfill)

### Code (headers at creation)

- lambda/handler_deepzoom_export.py:199-201 (tiles + dzi), :220-225
  (viewer.html): add `CacheControl="public, max-age=31536000, immutable"`.
  Leave meta.json (:244-249) and deepzoom_latest.json (:251-256) alone.
- Preview producers add the same header on `preview.png` puts (and later
  `preview.jpg`): handler_finalize_mt.py (~:905 upload), color_recolor_raw.py
  (:328, :549), handler_autolevels.py (:210), handler_png_export.py (:100),
  palette producers (handler_palette_finalize.py, handler_palette_render_plan.py
  :773/:882/:1159, handler_extract_palette_from_step_scores.py:397).
  Mechanical, one kwarg per put; grep each file for the actual `put_object`/
  upload call rather than assuming.

### Backfill: `scripts/backfill_cache_headers.py`

Prior art: `scripts/repair_preview_metadata.py` already does the dims repair
for color previews (copy-in-place, IHDR-authoritative, dry-run default). The
backfill script follows its conventions but is deliberately self-contained
(stdlib + boto3, no repo imports) so a single file upload runs it in
CloudShell; it widens scope to palette previews + deepzoom and adds the
Cache-Control header in the same copy.

Copy-in-place (`CopyObject` with `MetadataDirective=REPLACE`, preserving
ContentType, adding CacheControl, and — for previews — repairing legacy
`width`/`height` metadata to the measured PNG-header dims per §2.5).
**Targeted patterns only** — `renders/` has 1.22M objects and only ~4k are
browser-fetched previews.

- Default scope: color previews (1,651) + palette previews (~1,230) +
  `deepzoom/*/image.dzi` + `deepzoom/*/viewer.html`. Seconds to minutes.
- `--deepzoom-tiles` flag: the ~980k tiles. Costs ~$5 (COPY = $0.005/1k) and
  ~1–2 h at a few hundred copies/s. **Optional/deferred**: old tiles already
  get generous heuristic caching (Last-Modified is old → ~10%-of-age freshness);
  headers-at-creation covers all new exports, which are the worst cached today.
- Flags: `--dry-run` (list + count only), `--limit N`, `--prefix`, resumable
  (skips objects whose CacheControl is already set — idempotent by definition).
- Caveats: copy-in-place refreshes Last-Modified (fine) and requires
  re-specifying ContentType explicitly per object (read from HEAD first;
  never let the CLI guess — `.dzi` would mis-guess).

### Tests / gates

- Unit-test the header constant + producer puts where suites exist (payload
  contract style: assert exact put_object kwargs).
- `uv run python -m pytest` affected suites; `bash scripts/predeploy_check.sh`;
  no JS change → no Playwright.
- Post-deploy verify: `curl -sI` on a fresh preview and a fresh tile shows the
  header.

---

## 4. Phase 2 — JPEG previews + THE MIGRATION (the big win)

### 4.1 Migration script: `scripts/migrate_preview_jpg.py`

Single-file script, no Lambda fleet: the job is network-bound (~2.2 GB down /
~0.3 GB up; a 512² JPEG encode is milliseconds) and the storage Lambda is pure
Python — no PIL (deploy.sh:1064-1080) — so a one-off sweep doesn't justify new
Lambda deps or fan-out plumbing.

**Runs in AWS CloudShell (us-east-1) by preference** — the user's home line is
slow, and in-region execution keeps every byte inside AWS (fast + zero
egress). CloudShell constraints shape the script:

- Dependencies: stdlib + boto3 (preinstalled there) + Pillow
  (`pip install pillow` is the only setup). No uv assumption.
- Auth comes from the console session — no credentials handling.
- Stream one preview at a time (download → convert → upload → discard):
  the 1 GB `$HOME` never fills, and the resume journal lives in `$HOME`
  (persists across CloudShell sessions per region).
- ~8 threads (1 vCPU: overlap network with Pillow decode); whole sweep
  ~5–15 min. Idle timeout (~20–30 min) and 12 h cap are irrelevant at that
  runtime.
- `--sample` has no display in CloudShell: it uploads before/after pairs to
  a scratch prefix (`_scratch/preview_migration_samples/`) and prints their
  public URLs — the bucket is public, so inspection is browser tabs (~1 MB
  each, fine on a slow line). Scratch prefix is deleted after sign-off.

The identical invocation works from the laptop too (boto3 + Pillow); only
the transfer time differs.

Per artifact prefix (color + palettes families):

1. Skip if `preview.jpg` already exists (idempotent, re-runnable as top-up).
2. GET `preview.png`, convert + normalize to ≤512px per §2.4, PUT
   `preview.jpg` with `ContentType=image/jpeg`, honest `width`/`height`
   object metadata, + immutable CacheControl.
3. Merge `preview_jpg_key` + `preview_jpg_width`/`preview_jpg_height` into
   the overlay `meta.json` (read-merge-write, same pattern as the book
   palette overlay fix — never clobber other fields).
4. Record progress to a local journal (`--resume` continues after interrupt).
   Log any preview whose actual dims are anomalous (>1024 — a full-res image
   masquerading as a preview) — the ≤512 rule normalizes it anyway.

Flags: `--dry-run` (count + bytes estimate), `--sample N` (convert N random
previews to a LOCAL directory as `before.png`/`after.jpg` pairs for visual
inspection — run this and LOOK before the full sweep), `--family color|palettes|all`,
`--limit`, `--verify` (HEAD every expected jpg + meta field, report gaps).

Rollback: delete the `preview.jpg` objects and/or drop the meta field — the
builder falls back to PNG. PNGs were never touched.

### 4.2 Builder preference + tests

The §2.2 seam, both families. Tests (payload-contract style, in the existing
storage-suite home under `tests/`):

- entry with `preview_jpg_key` → manifest tile `key` is the jpg and
  `preview_width/height` come from `preview_jpg_width/height` in the meta
  (no ranged GET); without → png key + header-parse dims, exactly as today.
- Mixed manifest (some migrated, some not) round-trips — pins the mid-migration
  safety claim. Migrated tiles report 512 → the wall grid converges to a
  uniform 512 as migration completes.

### 4.3 Go-forward story (new renders after the migration)

- Producers keep writing `preview.png` (the color one comes from the native
  `score_raw_render --preview ... --preview_max=512`, lambda/raw_score_render.py:114-115
  — changing that binary is NOT required for this plan).
- New artifacts simply appear as PNG tiles on the wall (correct, just heavier)
  until the next `migrate_preview_jpg.py --top-up` run — same script, only
  processes artifacts missing a jpg, takes seconds when nearly caught up.
- Optional later hardening (separate wave): teach `score_raw_render` a
  `--preview_jpg=` output (C change + rebuild + docker gate) and add Pillow
  conversion in the producers that already have imaging deps. Not blocking.

### Expected result

Cold AllCol open: ~1.7 GB → **~100–150 MB** (JPEG ~6× + the 755 old 1024px
previews normalized to 512; with Phase 1, warm opens ≈ 0 bytes). Cold AllPal:
~431 MB → **~60–80 MB**. Uniform 512 grid, honest metadata. Same look at wall
scale (verified via `--sample` inspection before the sweep).

---

## 5. Phase 3 — JPEG tiles for new DeepZoom exports + viewer polish

- dz_export.c:45: `".png"` → `".jpg[Q=90]"`. New pyramids shrink ~3–5×.
- handler_deepzoom_export.py:192: content-type map gains `.jpg`/`.jpeg` →
  `image/jpeg` (currently png-or-octet-stream).
- deploy.sh:947-959 smoke test asserts "tile PNGs" — update to accept either
  extension, asserting count > 0 of the NEW extension so a silent format
  regression fails the deploy.
- JS polish (small, one Playwright-gated commit):
  - Reuse the DeepZoom viewer across inventory clicks: keep the OSD instance,
    call `open(dziUrl)` instead of destroy/recreate
    (js/12-deepzoom-boot.js:261-288).
  - Walls + DeepZoom: `placeholderFillStyle: '#121829'`, `blendTime: 0` —
    perceived-speed only, matches app panel color.

---

## 6. Verification, gates, and who runs what

Per repo rules: every commit runs `bash scripts/predeploy_check.sh` (no such
thing as a docs-only change); JS changes add the Playwright gate; pytest via
`uv run python -m pytest`.

Sequencing (each its own commit/push, user deploys between):

1. Phase 1 code → user deploys → Claude/user runs
   `backfill_cache_headers.py --dry-run`, then live (default scope).
2. Phase 2 builder + script → user deploys → in CloudShell (us-east-1):
   `migrate_preview_jpg.py --sample 12` → **visual inspection via the printed
   sample URLs** → `--dry-run` → `--limit 50` canary → full run → `--verify`
   → hit wall Refresh (manifest rebuild picks up jpg keys) → measure:
   DevTools network total for a cold AllCol open, before vs after
   (expect ~10×).
3. Phase 3 → user deploys → new export → confirm jpg tiles + headers via
   `curl -sI`.

Migration runs write derived artifacts to S3; per the deploy rule they happen
only on the user's explicit go (Claude may run them when told).

---

## 7. Deferred options (in rough value order)

1. **Composite wall pyramid** (the meditationsincolor experience): mosaic
   refresh composites previews via vips `arrayjoin` (1,651 × 512px ≈ 21k×21k)
   through the existing `dz_export` → the wall becomes a real DZI; zoomed-out
   open is a few hundred KB and sharpens progressively. Trade-off: bakes ONE
   order — re-sort/filter needs a server re-composite (~1 min) or falls back
   to the per-preview grid. Bake `created_at` (the default view) first.
2. **Thumb layer + zoom-gating**: ~256px `thumb.jpg` (~25 KB) per artifact;
   wall grid uses thumbs (~40 MB total), full previews load only past a zoom
   threshold (swap/overlay tiled image). Keeps all client-side sorting.
3. **CloudFront** in front of the bucket: h2 (kills the 6-connection cap),
   edge latency, faster share links for friends; also the only fix that helps
   without shrinking bytes. deploy.sh distribution + base-URL swap
   (js/13-artifact-mosaics.js:163-167 already centralizes the base).

## 8. Risks

- **JPEG on hard palette edges + 1024→512 downscale**: mitigated by 4:4:4 +
  q92 + Lanczos + mandatory `--sample` inspection at wall-typical zoom;
  per-key `--skip` escape hatch keeps stubborn artifacts on PNG (builder
  falls back per-artifact). Full-res detail always remains one click away
  (image.jpeg / DeepZoom).
- **Metadata repair changing an accidental display**: a consumer that today
  shows "5000" from preview user_meta would start showing 512 — §2.5 mandates
  a consumer grep before the repair run.
- **Overlay meta races**: recolor/autolevels rewrite artifact meta; migration
  does read-merge-write and should run while no render jobs are active. A lost
  `preview_jpg_key` is self-healing via `--top-up`.
- **Copy-in-place surprises**: refreshed Last-Modified changes nothing
  functional (immutable header now governs); ETag preserved for single-part
  objects.
- **Browser memory on the wall**: 1,651 decoded previews — unchanged from
  today (JPEG decodes to the same RGBA); OSD's default tile cache cap applies.
- **Costs**: ~2,800 GET+PUT + optional ~1M COPY ≈ $5–6 one-off; +~330 MB
  storage for JPEG siblings. Negligible.
