# AllCol / AllPal Mosaic Post-Mortem — code review of the implementation

**Date:** 2026-06-24 · **State reviewed:** working tree (uncommitted) on top of `4953856`, plus the genericization changes:
`handler_storage.py` (+426/−), new `js/13-artifact-mosaics.js` (521), deleted `js/13-allrenders.js`, `index.html`, `js/01-core-compute.js`, `tests/test_storage_handler.py` (+75), `tests/test_frontend_js.sh` (+139), `deploy_manifest.json` + `api_manifest.json` (+`/list-palette-mosaic`).
**Tests at review:** `test_storage_handler.py` 26 passed; `test_frontend_js.sh` OK ("artifact mosaic runtime checks: OK", 20 scripts).

Method: read the genericized backend (`_handle_list_mosaic`/`_run_mosaic_worker`/`_build_mosaic_manifest` by `kind`, palette scanner + builder) and the full new frontend controller; traced every cross-kind contract (DDB task split, routes, dispatch, click-through, size dropdown) to source; ran the gates.

## Verdict

**The genericization is done right and it ships.** The hard parts are shared, not duplicated: status helpers, refresh, worker, route handler, dispatcher, and prune are all `kind`-parameterized with thin `color`/`palette` wrappers; the frontend is one controller over per-kind state with no leaked `allrenders-` ids and separate viewers per wall. Every prior-round hardening survived (clean size aggregation, centralized key parsing, poll-retry, log cap, fail-visible S3 errors, `AWS_REGION`). The flaky bits below are **left-hanging tail risk**, not blockers — two worth fixing before this is "done," the rest are notes.

## Verified solid (so these don't get re-litigated)

- **Genericized, not duplicated, where it counts:** `_run_mosaic_worker("color"|"palette")` (one body, dispatch at handler:1281/1286), `_handle_list_mosaic(event, kind)` + `handle_list_*_mosaic` wrappers, `_build_mosaic_manifest(kind, …)` dispatcher, `_prune_mosaic_manifests(kind, …)`, `_mosaic_task_id/_mosaic_prefix/_mosaic_internal_action(kind)`.
- **Both routes go through `_handle_storage_route`** (handler:1373/1375) → structured 400/404/500 mapping. (The earlier divergence is fixed.)
- **Size aggregation is clean in *both* builders** — dict-from-integers (`size_counts[width] += 1`, then `sorted(size_counts)` / `{str(k): v}`), at 2990/3034 and 3142/3187. The old `split("x")`/`endswith` gymnastics are gone everywhere.
- **No `/list-palettes` regression:** the refactored `_list_saved_palettes` returns unordered, but every caller (`handle_list_palettes`:1968-1969, `handle_render_summary`:4229) applies `_order_palette_variants` itself, so ordering is preserved.
- **Frontend per-kind isolation:** `_mosaicState(kind)`/`_mosaicEl(kind, suffix)` (so DOM ids resolve to `allcol-*`/`allpal-*`), separate `state.viewer` per kind, generic click registered with the right kind closure (js/13:293), dynamic palette size dropdown with disable-when-single (`_syncMosaicSizeOptions`:235). `switchTab` cleanly routes `allcol`/`allpal` only (js/01:72-73).
- **`strict` split on the palette scanner:** `_palette_entry_from_prefix(strict=False)` for the lenient `/list-palettes` path, `strict=True` for the worker — missing `meta.json` (NoSuchKey) is a skip in both modes.

## Flaky bits left hanging (severity-ordered)

### F1 — MEDIUM · The two manifest builders are ~150-line near-twins (drift risk)
`_build_color_mosaic_manifest` (handler:2945-3041) and `_build_palette_mosaic_manifest` (3043-3193) share an essentially identical skeleton: job list → "scanning jobs" progress → flatten `(job_id, prefix)` → artifact pool → counts/`source_counts`/`size_counts` → two-key sort → manifest dict. Only `read_job`/`read_artifact` (which inventory helper, which entry fields) and a few manifest constants (`artifact_kind`, `dimension_filter`) actually differ. This is the one place the doc's #1 risk ("duplicate instead of genericize") landed: status/worker/route/handler were genericized by `kind`, the builders were copy-pasted. A change to the scan/progress/counting/sort logic (new skip counter, cadence tweak, sort change) must now be hand-mirrored in both, or they silently diverge. **Fix:** one `_build_mosaic_manifest_generic(kind, refresh_id, *, list_prefixes, read_entry, tile_extra, manifest_extra, progress_cb)` with per-kind callbacks; the two public builders become ~10-line configs.

### F2 — MEDIUM · A single corrupt JSON file fails the *entire* refresh
The B3 "fail-visible" policy is now doubled and, for per-file decode errors, too coarse:
- `_read_mosaic_calc_meta` (2796-2809) raises on anything that isn't `_is_missing_s3_error` — so a present-but-malformed `calc.json` (`JSONDecodeError`) propagates to the worker's top-level handler and flips the **whole** mosaic to `state:"error"`. This hits **both** walls (it runs in each `read_job`).
- `_palette_entry_from_prefix(strict=True)` (2058-2070) does the same for a corrupt palette `meta.json` → the **entire AllPal refresh** errors.

For a wall of thousands of artifacts, one truncated/old-format JSON blanks everything. `calc.json`'s `function/degree/N` are *informational sort labels*; even a palette's `meta.json` could degrade one tile rather than nuke the wall. Fail-loud on transient/systemic S3 errors (5xx/throttle) is right; a per-file *decode* error should increment a `skipped_bad_meta` counter and continue. **Fix:** in both readers, treat `json.JSONDecodeError`/`ValueError` as a per-artifact skip (count it), reserve `raise` for non-missing `ClientError`.

### F3 — LOW · Palette tiles have no `created_at` fallback (color does)
Color tiles get `created_at = meta.get("created_at") or image_info.get("modified_at")` (S3 LastModified fallback, via `_render_artifact_entry`). The palette path (`_palette_entry_from_prefix`) sets `created_at` only from `meta.json`; `_mosaic_tile_from_entry` then reads `entry.get("created_at", "")`. So a palette whose `meta.json` lacks `created_at` gets `""` and clumps at the end of the default **Date** sort, where the equivalent color artifact would fall back to its S3 mtime. **Fix:** thread the preview/image `LastModified` from the builder's HEAD results into the palette tile's `created_at` fallback.

### F4 — LOW · Palette preview is HEAD-checked *and* range-GET'd (two round-trips)
The palette builder HEADs `[image_key, preview_key]` for existence (js… handler:3113) and then `_mosaic_tile_from_entry` range-GETs the same `preview_key` for dimensions (2816). Two requests on the preview per tile. Tolerable, but at N previews it doubles the preview request count; fold the dimension read into the existence pass or skip the redundant HEAD on the preview.

### F5 — LOW · The `AllRenders` rename shims are alive only because a test wasn't migrated
`js/13-artifact-mosaics.js:514-519` keeps `loadAllRenders`/`refreshAllRendersMosaic`/`homeAllRenders`/`_allRendersRebuild`/`_allRendersCanvasClick` as aliases "for stale callers during the rename." No live code uses them — `switchTab` calls `loadAllCol`/`loadAllPal` — **except** `tests/test_frontend_js.sh:1722`, which still calls `_allRendersCanvasClick(...)`. So the shims exist purely to keep an un-migrated test green, and "temporary" shims that a test depends on never get removed. **Fix:** migrate the test to `_artifactMosaicCanvasClick('color', …)` and delete the five aliases.

### F6 / F7 — LOW · Pre-existing color cruft now inherited
- **Status-row duplication:** `_ready_mosaic_status` still copies `count`/`source_counts`/`skipped_*` from the manifest into the DDB row — two sources of truth that can drift; the row was meant to be a small pointer. Both walls carry it.
- **Extra per-job HEAD:** the color `read_job` runs `_legacy_render_variant(...)` for every job solely to count `skipped_legacy` (a number surfaced nowhere prominent) — N extra round-trips per refresh. (Palette has no legacy concept, correctly.)

## Test status

- Backend palette behavior is covered in **gated** `test_storage_handler.py` (26 passed), and the frontend harness exercises the generic controller ("artifact mosaic runtime checks: OK"). Route is in both `deploy_manifest.json` and `api_manifest.json`.
- **Gaps:** no test asserts F2 (a corrupt-but-present `calc.json`/`meta.json` fails the refresh — and after the fix, that it *skips-and-counts* instead). No test for F3 (palette date fallback). The F5 test still pins the old name.

## Recommended order

1. **F2** — make per-file decode errors skip-and-count, not refresh-fatal (both readers). Add the corrupt-JSON test.
2. **F1** — collapse the two builders into one generic skeleton + per-kind callbacks before more divergence accrues.
3. **F5** — migrate the click test to the generic name; delete the rename shims.
4. **F3 / F4 / F6 / F7** — date fallback, drop the redundant preview HEAD, trim status duplication / the diagnostic legacy HEAD as cleanup.

F1 and F2 are the two that matter; everything else is housekeeping. The feature itself — genericized backend, palette wall, click-through, dynamic size dropdown — is correct and green.
