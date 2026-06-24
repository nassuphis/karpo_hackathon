<!-- LEGACY: historical AllRenders-only post-mortem; current work is the generic AllCol/AllPal artifact mosaic implementation. -->
# AllRenders Mosaic Post-Mortem — `502c9fa "Add AllRenders mosaic tab"`

**Date:** 2026-06-24 · **Commit:** `502c9fa` (1,193+/24−, 8 files) · **Plan:** `deep-zoom-mosaic.md` §21.
**Tests at review:** `test_storage_handler.py` 20 passed; `test_api_route_contracts.py` + `test_frontend_parts_contract.py` + `test_deploy_packaging.py` 33 passed; `tests/test_frontend_js.sh` OK (20 scripts, "Frontend AllRenders runtime checks: OK").

Method: read the full backend (`handler_storage.py` mosaic block 2460–3011 + dispatch 1266–1355), the new `js/13-allrenders.js` in full, the `index.html` diff, and traced every cross-module contract (DDB, IAM, preview-metadata, click-to-open) to source. HIGH/correctness claims re-verified by reading the producing code, not assumed.

## Verdict

**Shippable, and notably faithful to the plan.** Every must-fix from the design-review rounds was implemented correctly — this is the cleanest pass of the series. There is **one real correctness bug** (F1: preview-dimension capture mis-reads autolevels-derived previews, breaking the size filter for that subset) and a handful of LOW polish/robustness items. Nothing crashes; the wall, refresh, async status, filters, sort, columns, and click-to-open all work. F1 is the only finding that affects a user-facing feature's correctness.

## What went right (verified against the code)

Every contract I flagged across the prior reviews landed correctly:

- **Conditional `put_item`, not `update_item`** (the IAM gap). The storage role grants `PutItem/GetItem/DeleteItem/Query/BatchWriteItem` but **not** `UpdateItem` (`deploy.sh:1812/1938`); the implementation uses conditional `put_item` for every status transition (`_put_mosaic_status` 2633-2646, `_put_owned_mosaic_status` 2965-2974), so no `AccessDenied`. Exactly the recommended fix.
- **Self-invoke via env var, not `context`.** `_handle_storage_route(fn, event)` still drops `context` (200), and the refresh uses `os.environ.get("AWS_LAMBDA_FUNCTION_NAME", ...)` (2709) — no `AttributeError`. `import os` present (21).
- **`internal_action` handled before route dispatch** (`handler` 1267, before the `path.endswith` chain at 1273). The worker payload short-circuits cleanly.
- **Refresh-id ownership CAS on every write.** Acquire conditions on `attribute_not_exists OR state<>computing OR stale` (2692-2701); ready/error writes condition on `refresh_id = :id AND state = computing` (2968) — a stale worker's publish fails the condition and is swallowed (3000-3002). Strongly-consistent reads (`ConsistentRead=True`, 2628). Status row carries no tiles (manifest goes to S3), so no 400 KB DDB-item risk.
- **Manifest pruning implemented** (`_prune_color_mosaic_manifests` 2900-2918, `MOSAIC_KEEP_LAST=10`), called from the worker. Orphan-manifest accumulation is addressed.
- **Flattened single pool** (no nested executors): jobs→`work` list of `(job_id, prefix)` then one bounded pool over it (2820-2849, `MOSAIC_WORKERS=24`).
- **`presign=False` threaded** through `_list_render_family_variants`/`_render_family_entry_from_prefix`/`_head_artifact_keys`/`_load_color_artifact_overlay` (all signatures updated, `*, s3_client=None`); the render-summary wrapper preserves prior behavior and its tests still pass (20 green) — no Render-tab regression.
- **`renders/_index/` excluded from the job scan** (`_list_mosaic_job_ids` skips `job_id.startswith("_")`, 2761), so the index dir isn't scanned as a "job".
- **Frontend:** separate `_allRendersViewer` (not the DeepZoom `_osdViewer`), `imageLoaderLimit:16`, 1×1 transparent data-URI tile, idempotent `loadAllRenders` guard, controls wired (`onchange="_allRendersRebuild()"`), explicit viewer `height:68vh`, click-to-open uses the verified `_ensureResultsSelection`→`switchTab('render')`→`refreshRenderArtifacts(...selectFamily/selectArtifactId)` path, `__ppParts` registered and the part-consistency list updated.
- **Gating not regressed** (the recurring CR20/21 miss): tests live in already-gated `test_storage_handler.py` and `test_frontend_js.sh`; route is in both `deploy_manifest.json:139` and `api_manifest.json`; no new ungated test file.
- **Async retry self-mitigated:** the worker catches its own exceptions and writes `error` status rather than re-raising (2996-3003), so the Lambda invocation succeeds and AWS does not auto-retry a duplicate crawl (except the rare non-conditional DDB-put failure path).

## Findings (severity-ordered)

### F1 — MEDIUM · HEAD-first preview dimensions mis-read autolevels-derived previews → wrong size filter
`_mosaic_preview_dimensions` (2733-2752) trusts `preview.png`'s `width`/`height` **user-metadata** before falling back to the PNG header. **Verified:** autolevels stamps the *source image* size onto the preview object — `img_meta["width"]=str(out_width)` then `preview_meta["width"]=image_meta["width"]`, uploaded via `ExtraArgs={"Metadata": preview_meta}` (`handler_autolevels.py:303,318,321-325`). `out_width` is the full autolevel output (hundreds–thousands of px), **not** the 512 preview. So for autolevels-derived color artifacts the HEAD branch returns e.g. `4000×4000`: the square check passes (still shown in **All**), but the tile is **excluded from the `512`/`1024` filters** (width matches neither) and pollutes `source_counts` with a bogus key.

**Scope (verified, narrower than first feared):** the two dominant producers are clean — `raw_score_render` previews carry **no** preview metadata (`raw_score_render.py`; upload sets none) and recolor/repalette previews also set **no** Metadata on the preview put (`color_recolor_raw.py:352`). Both therefore fall through to the authoritative PNG-header read and bucket correctly. Only **autolevels** poisons the metadata. The design doc's own bucket scan used PNG headers (saw only 512/1024), confirming headers are authoritative and the metadata is the unreliable source.

**Fix:** stop trusting the generic `width`/`height` metadata for preview sizing — always read the PNG header for preview dimensions (one ranged GET, refresh-only, already implemented and correct), or only honor a dedicated `preview_width`/`preview_height` key (which no producer writes). Add a test fixture: a `color/<id>/preview.png` whose S3 metadata says `width=4000` but whose 24-byte header says `512×512`, and assert the manifest records `512`.

### F2 — LOW/MEDIUM · `include_legacy` is dead; legacy color renders silently omitted *and* uncounted
`_build_color_mosaic_manifest(refresh_id, *, include_legacy=False)` (2807) never calls `_legacy_render_variant`; the only use of the flag is `if not include_legacy: counts["skipped_legacy"] = 0` (2863-2864), a no-op. So jobs whose only color output is the legacy root `image.jpeg`/`preview_color.png` (no `color/<id>/`) **never appear** on the "all renders" wall, and `skipped_legacy` is **always 0** — the status/manifest claim "no legacy skipped" even when legacy renders exist. Skipping legacy is an acceptable v1 choice (per the plan), but it should be *counted* for honesty, and the dead `include_legacy` parameter implies a capability that isn't wired. **Fix:** either count legacy artifacts into `skipped_legacy` during the scan, or drop the `include_legacy` parameter so it doesn't imply unimplemented behavior.

### F3 — LOW · Re-opens the viewer (resets pan/zoom) on every tab visit
`switchTab('allrenders')` → `loadAllRenders()` → on `ready` with a matching `refresh_id`, the idempotency guard correctly avoids **re-fetching** the manifest (`_loadAllRendersManifestForStatus` 221-224) but still calls `_allRendersRebuild()` → `viewer.open(source)` (194). `viewer.open` resets the viewport to home, so leaving and returning to the tab loses the user's pan/zoom position. The plan called for "only resize/reopen when the container or controls changed." **Fix:** skip `viewer.open` when the manifest, filter, sort, and column layout are unchanged (track the last-rendered signature; reopen only on change).

### F4 — LOW · Click maps through a recomputed `cols` instead of the rendered source's `cols`
`_allRendersTileSource` stores `_allRendersCols` on the source (js/13:173) but the click handler ignores it and recomputes `cols = _allRendersRequestedCols(_allRendersTiles.length)` (205). It happens to match today because every control `onchange` rebuilds before a click can use a stale grid. It's fragile: any future path that changes the grid without an immediate rebuild (e.g., a window-resize relayout, or switching to `oninput`) would make clicks select the wrong tile. **Fix:** read the active source's stored `_allRendersCols` in `_allRendersCanvasClick`.

### F5 — LOW · A failed self-invoke leaves status stuck `computing` for the full stale window
In `_start_color_mosaic_refresh`, the CAS `put_item` commits `computing` *before* `boto3.client("lambda").invoke(...)` (2708). If the invoke throws (throttling, transient), the status is `computing` with no worker, and the UI polls until `MOSAIC_STATUS_STALE_MS` (30 min) lets a new refresh take over. **Fix:** wrap the invoke in try/except and, on failure, write an `error` status (owned by this `refresh_id`) so the UI recovers immediately.

### F6 — LOW · Region hard-coded
`MOSAIC_BASE_URL = f"https://{BUCKET}.s3.us-east-1.amazonaws.com/"` (100) and the frontend fallback base (js/13:43) hard-code `us-east-1`. It matches `deploy.sh REGION` today, so it works, but it silently breaks if the bucket region ever changes. **Fix (optional):** derive from `AWS_REGION` server-side; keep the literal only as the frontend fallback.

### Notes (no action required)
- The prune `keep_refresh_ids` expression (2992) computes `{refresh_id, <refresh_id again>}` = `{refresh_id}` (the `last_ready_manifest_key` equals the just-written key); harmless because `_prune` independently keeps the `MOSAIC_KEEP_LAST` most-recent by lexical (≈chronological) refresh id. Convoluted but correct.
- `source_counts["unknown"]` and `counts["unknown_dimensions"]` both increment for unknown-dim tiles (2853-2855) — intentional double-bookkeeping, not a bug.
- Manifest is fetched whole client-side; ~1,483 tiles ≈ a few hundred KB — fine. Linear growth + the single-level "fit-all loads every tile" ceiling remain the known scaling limits (documented in §21.4).

## Test / gating status

- Backend mosaic behavior is covered in **gated** `test_storage_handler.py` (20 green). Route presence is enforced by the gated `api_manifest.py --check`/`deploy_manifest.py --check` contract path and `test_api_route_contracts.py` (33 green incl. packaging + parts contract). Frontend tab/registration/runtime asserted by gated `test_frontend_js.sh`.
- **Gap:** no test asserts the F1 case (preview whose S3 metadata dims differ from its PNG-header dims), so the bug is unguarded. Add it with the fix.

## Recommended fix order

1. **F1** — drop HEAD-metadata trust for preview sizing; read the PNG header always. Add the metadata-vs-header regression test. (Only finding that affects feature correctness.)
2. **F2** — count `skipped_legacy` (or remove the dead `include_legacy`).
3. **F5** — reset to `error` on a failed self-invoke.
4. **F3 / F4** — preserve viewport across tab visits; map clicks through the rendered source's `cols`.
5. **F6** — optional region de-hard-coding.

F1 is the must-fix before relying on the size filter; everything else is polish. The async/DDB/IAM/self-invoke core — the part most likely to be subtly wrong — is correct.
