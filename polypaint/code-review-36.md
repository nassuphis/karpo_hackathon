# Code Review 36 — the Views rework

Date: 2026-07-27

Reviewed range: `ac9ca1e..9dceb56` (the corrective rework), with the whole
Views feature surface (`d155716..HEAD`) in scope.

- `ac9ca1e Views become associated artifacts: view_raster + modal + per-job list` — the build this review's author produced, rejected in use
- `a691f1e Refactor ViewRender artifact workflow` — the corrective redesign
- `9dceb56 Add isometric ViewRender projections` — isometric + shared projection header
- Review mode: adversarial static analysis of every touched layer, targeted
  reachability probes, Docker ARM64 runtime verification of both C binaries,
  and the complete predeploy gate.

## Verdict

The reworked architecture is correct and the current state is sound. Every
adversarial probe this review ran came back clean: the full predeploy gate
passes, the Docker ARM64 suite passes including the new isometric oracles on
the shipped binaries, and each suspicion investigated (multi-pass t
semantics, step-scores key scoping, completion→refresh loop, metadata
availability on catalog rows, delete scoping, job-switch races, detached
DOM feedback) turned out to be handled. No high-severity findings. What
remains is cleanup and doc truth: 2 medium, 5 low/info.

The most important content of this review is not the findings table — it is
the analysis of why the rejected build was wrong, recorded so the failure
mode is never repeated.

## Why ac9ca1e was wrong (the design failure, named precisely)

The corrected design makes the mistake easy to state: **ViewRender is one
changed pixel-mapping inside a renderer that already existed.** The correct
implementation is ~40 lines of plan/finalize routing (family = "views",
pix = N) on top of machinery that was already built, tested, and deployed —
d155716 had even already added the view flags to the raster and the plan.
Instead, ac9ca1e built a second renderer:

1. **A parallel pipeline instead of a parameter.** A bespoke lores path
   (lattice subsample ≤512², a new C tool, a new storage dispatch route, a
   new task follower) re-implemented raster + equalize + palette + persist —
   all of which the fused MT pipeline already did better. Duplication where
   a flag was sufficient.
2. **A quality ceiling baked into a permanent artifact.** Views were capped
   at 512²/2048px "fast sizes" when the natural resolution of a view is
   N×N — the parameter square itself (N = 2000–10000). The rework renders
   at exactly N×N through the full pipeline: the t axis has N distinct
   values, so N is not a choice, it is *the* resolution. Asking the user to
   pick a lattice and a pixel size (the ac9ca1e modal) was asking them to
   choose between wrong answers.
3. **A bespoke UI instead of the house convention.** Custom list rows with
   four per-row buttons, a custom `#views-list` pane, custom arrow handling —
   when the artifact panel already had a catalog + selected-row preview +
   toolbar convention every other family uses (the recorded ui-consistency
   rule). The rework deleted all of it and got arrows, preview, Download
   menu, Delete, and DeepZoom for free by normalizing view rows onto the
   shared render-artifact contract (`_viewAsRenderArtifact`).
4. **Provenance narrowed instead of inherited.** The lores path carried a
   subset of the source artifact's settings (palette, viewport, transforms)
   and silently dropped the rest (quality, format, normalize, background,
   interpretation fidelity via reconstruction fallbacks). The rework derives
   the *entire* render request from the source artifact's recorded
   provenance (`_viewRenderParamsFromArtifact`) and dispatches the same
   orchestrator as ColorRender-MT — WYSIWYG by construction.
5. **The foundation was already there and was abandoned.** d155716 wired
   view flags through roots2pix_mt, the plan, the ASL contracts, and the
   parity tests, then ac9ca1e ignored that path and built beside it. The
   rework's core is d155716's foundation plus routing.

The general lesson, stated once: when a feature is "the same computation
with one changed mapping," the implementation must be a parameter on the
existing path, not a new path. Every new path is a permanent tax — its own
bugs, tests, deploy wiring, and UI — and a quality fork the user has to
reason about.

## What the rework verifiably does (current architecture)

- **Dispatch**: ViewRender modal (projection front/rear/left/right/radial/
  isometric + vertical t1/t2 — nothing else) → `_dispatchRenderOrchestrator`
  with the full param set derived from the selected Color artifact; `pix` is
  forced to calc N both client-side and at plan time (`handler_render_plan`
  re-checks bounds 2 ≤ N ≤ 32768 and validates `source_color_artifact_id`).
- **Plan**: `family = "views"`, `artifact_id = view_{run_id}`, prefix
  `renders/{job}/views/{id}/`; view metadata (projection, vertical,
  lattice_n, source_artifact_id, image/preview keys) rides `artifact_meta`.
- **Raster**: `roots2pix_mt` projects per root — elevations put one rotated
  root coordinate against t, radial puts `hypot(re,im)·W/rmax` (rmax =
  farthest viewport corner from origin), isometric uses the shared
  `view_projection.h` (viewport-normalized (Re,Im,t) onto 30° axes) that
  `view_raster.c` also compiles — one header, two consumers, cross-pinned by
  oracles. Out-of-viewport roots clip before projection for every non-plan
  mode. Multi-pass steps wrap `(step_start + p) % N²`, so later passes
  overlay pass-0 t space instead of clipping (verified in source).
- **Finalize**: for family "views" the stringified full metadata is written
  as `{prefix}/meta.json` (no color overlay), and `family`/`artifact_id`
  ride the result so the frontend completion hook can do a targeted
  `_viewsEnsureInventory(true, {selectViewId})` — the finished view appears
  selected without a manual refresh (verified: js/10-status-results.js
  family === 'views' branch).
- **Storage**: `/list-views` normalizes rows onto the render-artifact shape
  (width/height/content_type/preview_url/viewer_url) with backward compat
  for pre-rework meta.json rows; `/delete-render-artifact` handles views by
  prefix delete; `/start-view-render` and its handler are gone from code,
  routes, and manifests.
- **Tests**: plan (`test_view_plan_*`), ASL definition (contract-injected
  selector), raster parity (hand-computed pixel sets incl. isometric and
  section `view_step_start` offsets), finalize (views meta.json + family in
  result), storage (list normalization), frontend text pins
  (test_frontend_js.sh asserts the derivation functions exist AND that the
  lattice/pix selectors and `/start-view-render` do NOT), a full e2e
  (exact dispatch payload from a rich artifact row, shared catalog/arrows/
  GoColor/Delete, color-family purity), and Docker ARM64 runtime oracles
  for both binaries.

## Findings Summary

| ID | Severity | Finding |
|---|---|---|
| CR36-F1 | MEDIUM | The lores `view_render` mode + `view_raster` tool are orphaned product surface — built, bundled, and gate-tested with zero reachable callers |
| CR36-F2 | MEDIUM | Docs describe the replaced architecture as shipped (sculpture-refactor.md §7, project memory) |
| CR36-F3 | LOW | The ASL template's view fields are dead text — the renderer overwrites the ItemSelector from workflow_contracts.py |
| CR36-F4 | LOW | `handle_list_views` treats every meta read failure as an absent row (CR28-F13 taxonomy: transient ≠ absent) |
| CR36-F5 | LOW | Isometric occlusion is first-claim order, not depth order |
| CR36-F6 | LOW | `/delete-prefix` still carries the views widening alongside `/delete-render-artifact` — two delete surfaces for one artifact type |
| CR36-F7 | INFO | A views-sourced DeepZoom exercises `deepzoom_from_raw` on a views raw with no integration test |
| CR36-F8 | INFO | ViewRender cost equals a full N×N MT render — deliberate; the F1 decision is where a future fast path would come from |

## Findings

### CR36-F1 (MEDIUM) — orphaned lores view path

`handler_render_lores_preview._run_view_render` (and the `view_raster`
binary it shells out to) has no caller: `/start-view-render` was deleted
from storage routes and manifests, and no frontend or dispatch code sends a
`view_render` payload (verified by grep across js/, index.html, lambda/).
Yet the surface is fully alive everywhere else: musl-built in deploy.sh,
copied into the lores bundle (~180 KB static binary), tracked by
check_binary_freshness, and tested by 7 oracle tests + 2 handler tests + a
Docker runtime smoke — all of which the gate runs forever. 9dceb56 even
updated it (isometric added), so intent is ambiguous.

Recommendation: make the decision explicit. Either (a) keep it as the
documented donor for a future fast in-modal preview (a 128² view in ~2 s
next to the full-pipeline render) — in which case one comment at the mode
saying exactly that; or (b) delete the mode, the tool, the tests, and the
deploy wiring in one commit. What should not persist is an untriaged
second renderer — that is the exact failure mode this rework corrected.

### CR36-F2 (MEDIUM) — documentation describes the replaced design

`sculpture-refactor.md` §7 presents the lores view path (128–512 lattice,
`/start-view-render`, view_raster as the product renderer) as the shipped
design; the project memory recorded the same. Both now describe deleted
architecture and would mislead the next session. Fixed alongside this
review: §7 rewritten to the full-pipeline design, memory rewritten.

### CR36-F3 (LOW) — dead view fields in the ASL template

a691f1e added `view_projection`/`view_vertical`/`view_grid_n` to
`ColorRasterMap.ItemSelector` in `render_workflow.asl.json.template`, but
`workflow_template_render.py:73` replaces that whole selector with
`deepcopy(RENDER_COLOR_RASTER_ITEM_SELECTOR)` at render time — the deployed
ASL never reads the template's copy (this is also why d155716's
contracts-only change was already correct). Duplicated contract text that
cannot drift-fail invites false confidence in template edits. Either drop
the duplicated selector fields from the template or mark the block
`"__replaced_by": "workflow_contracts.RENDER_COLOR_RASTER_ITEM_SELECTOR"`.

### CR36-F4 (LOW) — list-views swallows transient errors as absence

`read_meta` returns `None` on any exception, so an S3 throttle or 5xx
silently shrinks the views list (the sculpture listing shares this
pre-existing pattern). CR28-F13 doctrine: only a genuine 404 is absence.
Low severity because the listing is refreshable and non-destructive, but
worth aligning both listings with `is_missing_s3_error` when next touched.

### CR36-F5 (LOW) — isometric occlusion is claim-order, not depth-order

Per-pixel ownership everywhere is "first claim in step order" (and across
raster sections, merge order). For elevations that is the established
convention; for isometric — a projection of a genuinely 3D point set — the
visually correct rule would be painter's order by iso depth. Current output
is deterministic but occlusion is arbitrary. Fine as v1 doctrine; recorded
so a future "why does the front look wrong" report has its answer waiting.

### CR36-F6 (LOW) — two delete surfaces for views

`_VIEW_PREFIX` still widens `/delete-prefix` to
`renders/{job}/views/{id}/` (from ac9ca1e) while the UI now deletes through
`/delete-render-artifact`. Harmless (both are single-item scoped), but one
artifact type with two delete APIs is drift waiting to diverge — drop the
`/delete-prefix` widening unless something still calls it.

### CR36-F7 (INFO) — views DeepZoom integration untested

`deepZoomSelectedRenderArtifact` hands the views row's `raw_key`/
`raw_meta_key` to the deepzoom-from-raw machinery; the raw layout is
identical to color raws so it should work, and the e2e stubs the export at
the seam. No test drives a views raw through the real deepzoom handler.
One targeted handler test would close it.

### CR36-F8 (INFO) — cost model is deliberate

A ViewRender is a full MT render at N×N (N=2000 ≈ the usual sectioned
render times; N=10000 is a real render's cost). This is the point of the
rework — full pipeline, full quality, one code path. If a cheap preview is
ever wanted inside the modal, the F1 decision (keep vs delete the lores
path) is where it would come from; do not build a third renderer.

## Verification performed

- Full predeploy gate: PASS (unpiped exit; pytest suites, both Playwright
  suites, contract checks).
- Docker ARM64 runtime regression: PASS, including
  `roots2pix_mt` front/radial/isometric pixel oracles and `view_raster`
  front+isometric ownership oracles on the shipped static binaries.
- Reachability probes: `/start-view-render` absent from routes/manifests/js;
  lores `view_render` unreachable (F1); completion→refresh loop traced
  through finalize result → js/10 family branch → targeted inventory
  refresh with selection.
- Semantic probes (all clean): multi-pass viewT wraps into pass-0 t space;
  views step_scores/raws live under the artifact prefix (no job-level
  clobber); render-summary color entries carry every field
  `_viewRenderParamsFromArtifact` reads (viewport, source texts with
  reconstruction fallbacks, quantile, quality, normalize, background,
  interpretation); `_viewsEnsureInventory` guards mid-flight job switches;
  isometric geometry maps the unit cube into [0, pix−1]² with no
  out-of-range py; delete scoping is single-artifact.
