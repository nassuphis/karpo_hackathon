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

The reworked architecture is correct: ViewRender belongs in the existing MT
renderer as a changed projection, not in a parallel rendering pipeline. The
current implementation is not completely sound, however. A product-shape
probe at the actual `pix=N` resolution exposes an elevation off-by-one that
the existing `grid_n=4`, `pix=8` oracles cannot see, and the original
DeepZoom conclusion traced a raw-sidecar path that the frontend never
selects.

The full predeploy gate and Docker ARM64 suite still pass. That is useful
evidence for packaging, contracts, and the cases they exercise, but it is not
evidence against the missed `pix=N` geometry case. No high-severity
architectural finding remains. Open work is 3 medium, 4 low, and 1 info;
the former documentation finding is already resolved.

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
   the output-affecting render parameters from the source artifact's
   recorded provenance (`_viewRenderParamsFromArtifact`) and dispatches the
   same orchestrator as ColorRender-MT. Operational knobs such as thread and
   worker counts use the current standard defaults rather than pretending to
   be inherited provenance.
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
  with the output-affecting parameters derived from the selected Color
  artifact and standard current execution knobs; `pix` is forced to calc N
  both client-side and at plan time (`handler_render_plan` re-checks bounds
  2 ≤ N ≤ 32768 and validates `source_color_artifact_id`).
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
  overlay pass-0 t space instead of clipping. The step identity is correct,
  but the elevation pixel formula is not at `pix=N`: see CR36-F9.
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
  for both binaries. The elevation oracles use `grid_n=4`, `pix=8`; they do
  not pin the shipped `pix=N` geometry and therefore miss CR36-F9.

## Findings Summary

| ID | Severity | Finding |
|---|---|---|
| CR36-F9 | MEDIUM | At the shipped `pix=N`, elevation modes collapse the first two t rows and leave the top output row empty |
| CR36-F1 | MEDIUM | The lores `view_render` mode + `view_raster` tool have no first-party caller but remain externally callable, built, bundled, and gate-tested |
| CR36-F7 | MEDIUM | Views use ordinary image DeepZoom; the claimed raw integration is false and the frontend's `deepzoom_from_raw` branch cannot complete |
| CR36-F2 | MEDIUM (RESOLVED) | Docs described the replaced architecture as shipped; §7 was corrected with this review |
| CR36-F3 | LOW | The ASL template's entire ColorRasterMap ItemSelector is dead text — the renderer replaces it from workflow_contracts.py |
| CR36-F4 | LOW | `handle_list_views` treats every meta read failure as an absent row (CR28-F13 taxonomy: transient ≠ absent) |
| CR36-F5 | LOW | Isometric occlusion is first-claim order, not depth order, and the winning root can depend on worker scheduling |
| CR36-F6 | LOW | `/delete-prefix` still carries the views widening alongside `/delete-render-artifact` — two delete surfaces for one artifact type |
| CR36-F8 | INFO | ViewRender cost equals a full N×N MT render — deliberate; the F1 decision is where a future fast path would come from |

## Findings

### CR36-F9 (MEDIUM) — `pix=N` elevation mapping loses one t row

The full ViewRender product forces `pix` to calc `N`, but the elevation
mapping computes:

```text
t  = row / N          (or col / N)
py = floor((1 - t) * N)
if py == N and t == 0: py = N - 1
```

That clamp makes the first two t samples share the bottom output row. At
`N=4`, t indices 0,1,2,3 map to pixel rows 3,3,2,1; row 0 is never used.
The same formula is present in `roots2pix_mt.c` and `view_raster.c`.
Front/rear/left/right/radial all use it. Isometric uses the separate shared
projection and does not pass through this exact branch.

The hand-computed parity and ARM64 fixtures use `grid_n=4`, `pix=8`, where
the samples map to 7,6,4,2 and remain distinct. That is why every gate is
green while the actual `pix=N` product case is wrong.

Recommendation: map the discrete t index to the full pixel range without a
special-case collision — for example, with `t=k/N`, use
`H - 1 - floor(t * H)` (equivalently `H - 1 - k` when `H=N`). Add exact
front/t1 and front/t2 ownership oracles with `pix == grid_n`, in the host
binary test and Docker ARM64 runtime suite.

### CR36-F1 (MEDIUM) — orphaned but externally callable lores view path

`handler_render_lores_preview._run_view_render` (and the `view_raster`
binary it shells out to) has no first-party caller: `/start-view-render`
was deleted from storage routes and manifests, and no frontend or dispatch
code sends a `view_render` payload. It is not literally unreachable,
however. The Lambda remains behind the public `/render-lores-preview`
route, and its handler still dispatches any request carrying
`view_render` into `_run_view_render`. A direct caller can therefore still
create the old low-resolution row shape under the current `views` family.

The surface is fully alive everywhere else: musl-built in deploy.sh, copied
into the lores bundle (~180 KB static binary), tracked by
check_binary_freshness, and tested by 7 oracle tests + 2 handler tests + a
Docker runtime smoke — all of which the gate runs forever. 9dceb56 even
updated it (isometric added), so it is active maintenance burden, not merely
historical source.

Recommendation: delete the `view_render` mode, `view_raster` tool, tests,
manifest entry, and deploy wiring in one commit. Version control is the
donor if a fast preview is designed later. Keeping a second renderer
because it might become useful conflicts with the central lesson of this
review; a future preview should be introduced as an explicit product path
with an owner and contract.

### CR36-F2 (MEDIUM, RESOLVED) — documentation described the replaced design

Before dac1fea, `sculpture-refactor.md` §7 presented the lores view path
(128–512 lattice, `/start-view-render`, view_raster as the product
renderer) as shipped architecture. That would have misled the next session.
The same commit rewrote §7 to the full-pipeline design. This is retained in
the review as historical audit evidence, not open work.

### CR36-F3 (LOW) — the template's whole ColorRasterMap selector is dead text

`workflow_template_render.py:_apply_render_workflow_contracts` replaces the
whole `ColorRasterMap.ItemSelector` with
`deepcopy(RENDER_COLOR_RASTER_ITEM_SELECTOR)` at render time. The
`view_projection`/`view_vertical`/`view_grid_n` fields added to the JSON
template are therefore only three instances of a broader duplication: none
of that template selector is authoritative in the deployed ASL.

Recommendation: establish one structural authority for the whole selector,
not a field-by-field cleanup. Replace the template copy with an explicit
generation placeholder/minimal sentinel and have the renderer plus tests
enforce that `workflow_contracts.RENDER_COLOR_RASTER_ITEM_SELECTOR` is the
only deployed definition.

### CR36-F4 (LOW) — list-views swallows transient errors as absence

`read_meta` returns `None` on any exception, so an S3 throttle or 5xx
silently shrinks the views list (the sculpture listing shares this
pre-existing pattern). CR28-F13 doctrine: only a genuine 404 is absence.
Low severity because the listing is refreshable and non-destructive.

Recommendation: skip only confirmed missing metadata. Retry transient
reads, then either surface a request error or return an explicit
`metadata_error_count` and log the affected keys. Do not silently translate
a transient failure into an authoritative shorter inventory.

### CR36-F5 (LOW) — isometric ownership has no depth rule and can vary by schedule

Per-pixel ownership is first successful claim, not depth order. Inside one
`roots2pix_mt` section, workers race through an atomic
`__atomic_fetch_or`; the modification is safe, but which thread claims a
colliding pixel first is scheduler-dependent. Across completed sections,
merge ordering adds another ownership layer. For elevations this is the
established convention; for isometric — a projection of a genuinely 3D
point set — a visually conventional result would require an explicit depth
rule.

The accepted v1 caveat is therefore stronger than "deterministic but
arbitrary": ownership is arbitrary and may vary between equivalent runs.
That is acceptable only as an explicit art/performance tradeoff. Record it
in the UI/help or projection documentation so a future rerender difference
is not mistaken for corruption.

### CR36-F6 (LOW) — two delete surfaces for views

`_VIEW_PREFIX` still widens `/delete-prefix` to
`renders/{job}/views/{id}/` (from ac9ca1e) while the UI now deletes through
`/delete-render-artifact`. Harmless (both are single-item scoped), but one
artifact type with two delete APIs is drift waiting to diverge — drop the
`/delete-prefix` widening unless something still calls it.

### CR36-F7 (MEDIUM) — the claimed raw DeepZoom path does not execute

`deepZoomSelectedRenderArtifact` passes `image_key`, `raw_key`, and
`raw_meta_key` to `runDeepZoomExport`, but that helper defines
`useExactSource` from the non-empty image key and unconditionally selects
`deepzoom_export`. The raw keys are discarded from the dispatched job.
Views therefore use the ordinary image-based `dz_export` path. That path is
generic and should work; no views raw is read or transformed.

The alternate frontend branch is internally contradictory: it selects
`deepzoom_from_raw` only when `sourceKey` is empty, then sends that empty
`source_key`; `handle_deepzoom_export_request` rejects an empty source key.
Moreover, the backend's `require_raw_sidecar=True` checks that raw keys
exist and share the artifact prefix, but still downloads and exports the
image. It never consumes either raw object.

Recommendation: first decide the contract. If DeepZoom is intentionally
always image-based, simplify the frontend and remove/rename the dead
`deepzoom_from_raw` surface. If raw-sidecar generation is intended, make
the handler actually consume `raw_key`/`raw_meta_key` and add an end-to-end
test. For Views specifically, add a test that pins the current truthful
behavior: selected `views/.../image.{jpeg,png}` dispatches
`deepzoom_export`.

### CR36-F8 (INFO) — cost model is deliberate

A ViewRender is a full MT render at N×N (N=2000 ≈ the usual sectioned
render times; N=10000 is a real render's cost). This is the point of the
rework — full pipeline, full quality, one code path. If a cheap preview is
ever wanted inside the modal, design it deliberately against measured need
and reuse the shared projection contract; do not preserve or build another
permanent renderer merely as a possible donor.

## Verification performed

- Full predeploy gate: PASS (unpiped exit; pytest suites, both Playwright
  suites, contract checks).
- Docker ARM64 runtime regression: PASS, including
  `roots2pix_mt` front/radial/isometric pixel oracles and `view_raster`
  front+isometric ownership oracles on the shipped static binaries. Those
  elevation fixtures use `grid_n=4`, `pix=8` and do not cover CR36-F9.
- Reachability probes: `/start-view-render` absent from routes/manifests/js;
  no first-party `view_render` caller found, but the mode remains directly
  callable through `/render-lores-preview` (F1); completion→refresh loop
  traced through finalize result → js/10 family branch → targeted
  inventory refresh with selection.
- DeepZoom call-flow probe: a selected view's non-empty `image_key` forces
  `deepzoom_export`; `raw_key` and `raw_meta_key` do not reach the worker.
  The source-empty `deepzoom_from_raw` branch is rejected by its backend
  contract (F7).
- Semantic probes: multi-pass viewT wraps into pass-0 t space;
  views step_scores/raws live under the artifact prefix (no job-level
  clobber); render-summary color entries carry every field
  `_viewRenderParamsFromArtifact` reads (viewport, source texts with
  reconstruction fallbacks, quantile, quality, normalize, background,
  interpretation); `_viewsEnsureInventory` guards mid-flight job switches;
  isometric geometry maps the unit cube into [0, pix−1]² with no
  out-of-range py; delete scoping is single-artifact. Elevation geometry at
  the product's `pix=N` fails the distinct-row invariant (F9).
