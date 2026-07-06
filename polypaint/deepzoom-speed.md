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
| `deepzoom/` total | 983,705 (hundreds of exports × ~500–1,800 tiles) |

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

### 2.4 JPEG encoding spec

Pillow: `quality=92, subsampling=0 (4:4:4), optimize=True`, dimensions
unchanged. 4:4:4 protects hard edges in palette grids from chroma smearing.
Expected shrink 5–8× on this content (→ AllCol ~1.7 GB → **~220–300 MB**,
AllPal ~431 MB → **~60–90 MB**). Render-first discipline applies: the
migration's dry-run emits before/after pairs for visual inspection **before**
the full sweep (§4 runbook).

### 2.5 New DeepZoom exports get JPEG tiles

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

Copy-in-place (`CopyObject` with `MetadataDirective=REPLACE`, preserving
ContentType, adding CacheControl). **Targeted patterns only** — `renders/` has
1.22M objects and only ~4k are browser-fetched previews.

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

Local script (uv + Pillow + boto3). The storage Lambda is pure Python — no
PIL (deploy.sh:1064-1080) — and a one-off sweep doesn't justify new Lambda
deps. ~2.2 GB down / ~0.3 GB up, 16 threads, ~10–20 min end to end.

Per artifact prefix (color + palettes families):

1. Skip if `preview.jpg` already exists (idempotent, re-runnable as top-up).
2. GET `preview.png`, convert per §2.4, PUT `preview.jpg` with
   `ContentType=image/jpeg` + immutable CacheControl.
3. Merge `preview_jpg_key` into the overlay `meta.json` (read-merge-write,
   same pattern as the book palette overlay fix — never clobber other fields).
4. Record progress to a local journal (`--resume` continues after interrupt).

Flags: `--dry-run` (count + bytes estimate), `--sample N` (convert N random
previews to a LOCAL directory as `before.png`/`after.jpg` pairs for visual
inspection — run this and LOOK before the full sweep), `--family color|palettes|all`,
`--limit`, `--verify` (HEAD every expected jpg + meta field, report gaps).

Rollback: delete the `preview.jpg` objects and/or drop the meta field — the
builder falls back to PNG. PNGs were never touched.

### 4.2 Builder preference + tests

The §2.2 seam, both families. Tests (payload-contract style, in the existing
storage-suite home under `tests/`):

- entry with `preview_jpg_key` → manifest tile `key` is the jpg; without → png.
- `preview_width/height` unchanged in both cases.
- Mixed manifest (some migrated, some not) round-trips — pins the mid-migration
  safety claim.

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

Cold AllCol open: ~1.7 GB → **~250 MB** (with Phase 1, warm opens ≈ 0 bytes).
Cold AllPal: ~431 MB → **~75 MB**. Same look at wall scale (verified via
`--sample` inspection before the sweep).

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
2. Phase 2 builder + script → user deploys → `migrate_preview_jpg.py --sample 12`
   → **visual inspection** → `--dry-run` → full run → `--verify` → hit wall
   Refresh (manifest rebuild picks up jpg keys) → measure: DevTools network
   total for a cold AllCol open, before vs after (expect ~7×).
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

- **JPEG on hard palette edges**: mitigated by 4:4:4 + q92 + mandatory
  `--sample` inspection; per-key `--skip` escape hatch keeps stubborn
  artifacts on PNG (builder falls back per-artifact).
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
