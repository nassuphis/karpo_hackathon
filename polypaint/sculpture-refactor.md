# Sculpture refactor: artifact-sourced generation + off-thread viewer

Shipped state lives in `polynomial-sculptures.md`. This doc is the plan
for the next two waves, born from measured problems at hi-res
(384²/512²), plus the recorded backlog of speedup options.

## Problems (measured, not guessed)

**Generation.** The sculpture path grew out of the lores preview, so it
couples to live render state (solve-score mode, source radios, the
current rt text) and re-evaluates the solve-score for every run. At
512² that evaluation dominated: 110–188 s per run in the lambda
(CloudWatch), which first broke the synchronous path (API Gateway kills
responses at ~30 s; the browser saw dead requests while runs completed
invisibly, retries stacked more runs) and still costs 2–3 min as an
async job. The evaluation is redundant whenever the sculpture is "this
artwork, lifted to 3D" — the render already computed those scores.

**Viewer.** At 384² (degree ~45), `show=clu` froze the tab with no
feedback: 6.5 M segments. The build runs on the main thread —
k-means (17 k pts × 45 centers × 8 iters per column), greedy chains
at O(m²) per cluster, a median-NN cut also O(m²), × 384 columns ≈
**7–8 billion ops**, plus a 6.5 M argsort and ~200 MB buffer fills.
The paint lane IS the compute lane: the thread that would show
"clustering… 37%" (or un-grey the checkbox) is the one stuck in the
loop. Threads matching and nearest-ribbons share the disease at this
scale (~300 M ops each), just milder.

## Wave A — viewer: off-thread builds + linear-ish algorithms

**SHIPPED** (2026-07-25). As planned below, with these outcomes: all
three line primitives are now lazy worker-built topologies (xz-only,
cached per key — ribbons per connect order, threads/clu per z axis)
and the main thread only re-emits Y-aware buffers; boot is instant at
any size because nothing builds until its checkbox is on. The len%
sort became a 2048-bucket O(n) length sort (the comparator argsort was
itself seconds at 6.5M segments). clu got the sampled-fit k-means and
principal-axis chains. Progress lands in a `#hud-build` line; uncheck
cancels deterministically. Deviations from the plan: no worker path
for the thread matcher's repair phase specifically (the whole matcher
moved as-is), and the progress-text pin was dropped as inherently
flaky on fast machines — the cancel pin covers the protocol instead.
Details in polynomial-sculptures.md "Off-thread topology".

Viewer-only (`sculpture.html`); unblocks already-generated hi-res data.

1. **Inline Web Worker** (Blob URL, no new files): clu, thread
   matching, and the big segment sorts run off-thread on transferred
   position buffers; results return as index/segment arrays and the
   main thread only fills GPU buffers.
   - Progress protocol → HUD ("clu: column 120/384"), busy state on the
     checkbox, cancel on uncheck; UI (orbit, tours, sliders) stays live.
2. **Algorithm upgrades** (also *quality* wins):
   - k-means: fit centers on a ≤2 k-point sample per column, then one
     assignment pass over all points (~8× cheaper, same clusters).
   - Chains: replace greedy nearest-neighbor with **principal-axis
     ordering** — project each cluster's points onto its principal
     component (closed-form 2×2 eigen), sort, connect consecutive,
     cut at gaps > 2.5× the median gap. Trajectory arcs are near-1D,
     so this is O(m log m) AND removes greedy's residual path
     pathologies (backtrack chords at mid-cluster starts). Optionally
     keep greedy for small m (<500) where curvature folds a projection.
   - Same treatment applicable to per-solve nearest ribbons and the
     thread matcher's repair phase if profiling says so.
3. **Expected**: 384² clu ≈ a few seconds, in the background, with
   progress; 512² usable. Segment counts themselves (6.5 M lines) are
   fine for the GPU — the freeze was build cost, not draw cost.
4. Tests: worker round-trip determinism (same fixture → same segments
   as the sync reference), progress/cancel pins, and the existing clu
   fixture pins re-run against the worker path.

## Wave B — generation: saved color artifact is the ONLY source

**SHIPPED** (2026-07-25). As planned below. Outcomes: the lores
lambda gained an `artifact_sculpture` mode (self-contained function —
degree from calc.json, everything else from the artifact's metadata;
trivial m0 raster for the transform+dump only; stored step_scores
subsampled on the SAME `_logical_row_mapping` lattice the roots
materializer walks, de-serpentined row-major); `/start-sculpture-hires`
became `/start-sculpture-artifact` {job_id, artifact_id, n∈(384,512)}
with a SYNCHRONOUS step-scores head check ("re-render it" beats a dead
rail card); the tab's first line shows `source: <color id>` from the
Color family's selection; `source_artifact_id` provenance rides the
sculpture block, save payload, meta.json, and list rows. Deleted:
preview mode, `sculpture: true`, the solve-score-mode requirement, the
Sculpture-next-to-Preview flow, live-state coupling. One deviation:
the backend accepts view_n 2–512 (tiny test grids); the 384/512
restriction is enforced at the storage route + UI.

User decision: remove preview from the loop completely. The sculpture
derives from the **currently selected color artifact**; the tab's first
line shows `source: <color artifact id>`. Sizes 384²/512² only; one
code path; always async on the jobs rail.

**Feasibility is verified** — everything needed is already recorded per
color artifact (finalize + raster metadata):

- `step_scores_key`, `step_scores_grid_n`, `step_count` — per-solve
  scores at FULL grid, next to the artifact raw;
- `root_transforms` — the compiled rt chain that made that image
  (WYSIWYG from provenance, not live state);
- viewport `min_re/max_re/min_im/max_im` + `rotation`
  (VIEWPORT_METADATA_KEYS);
- palette, interpretation, clips, channels, score program text;
- the job's transport roots (calc + chunk manifests).

Precedent: `handler_extract_palette_from_step_scores` already derives
products from an artifact's stored scores + viewport metadata.

**The job becomes almost pure I/O** (~20–40 s expected):

1. Logical-materialize the transport roots at the 384/512 lattice
   (range GETs, no solving).
2. Apply the ARTIFACT's rt chain + rotation → u16 dump over the
   artifact's viewport. Dump production choice: first version runs
   roots2pix with a trivial one-token score (zero new C); a lean
   transform-only path is the later cleanup.
3. Subsample the artifact's stored `step_scores.raw` at the same
   lattice (byte picking — NO score evaluation) → palette PNG via
   `render_score_raw` with the artifact's palette + equalization.
4. Upload; DDB row lifecycle exactly as today (report_status →
   /check-status → jobs rail).

**Deleted concepts**: preview-sized sculptures, the solve-score-mode
requirement, live-settings coupling, the sync path, the
`render-sculpture-n` "preview" option. `source_artifact_id` becomes
provenance in sculpture meta and the saved list.

**Constraints**: artifacts predating step-scores storage lack
`step_scores_key` → clean error ("re-render to sculpture this piece").
Multi-pass (times>1) artifacts: step_scores cover grid×passes; the
sculpture keeps its pass-0 convention.

## Backlog (recorded, not scheduled)

- **Lambda knobs**: lores lambda 4 GB/4 raster threads → 10 GB/8–12
  threads (2–3× the raster stage for a config change). Mostly moot
  once Wave B removes scoring, but cheap if ever needed.
- **Stripe fan-out** (the render chunk pattern: per-stripe workers +
  report_status rows + /check-status expected=K + rail x/y progress).
  Likely unnecessary after Wave B; keep as the canonical split if a
  future mode re-introduces heavy compute.
- **Progressive stride-lattice tiles**: emit the dump coarse→fine so
  the viewer opens on ~15% coverage while the rest streams. Perceived
  latency, pairs with fan-out.
- **Content-hash result reuse**: **SHIPPED** (2026-07-25) — the
  artifact job hashes its full output-shaping signature into
  `renders/{job}/sculpture_cache/{sha1[:16]}/`; repeat Generates serve
  in seconds via server-side copies into the fixed keys. Repalettes
  hash differently (pinned); cache writes are best-effort.
- **Tour recording** (roadmap #5): **SHIPPED** (2026-07-25) — ● in the
  viewer's tour row records the canvas via MediaRecorder to a
  downloading mp4/webm, fully client-side; e2e-pinned headless.
- **WebGPU**: still only justified for compute-driven splatting/OIT at
  15 M+ points AFTER cloud style + hi-res data are routine, and only
  if WebGL wheezes.

## Sequencing

1. Wave A (viewer worker + algorithms) — DONE.
2. Wave B (artifact-only source) — DONE.
3. Wave C (content-hash reuse + tour recording) — DONE.
4. Motion LOD (2026-07-25, user-hit "3d paths kind of slow" at 5M
   segments) — DONE: stride-decimated index while the camera moves,
   k=ceil(segs/1.5M)x adaptive boost, full geometry at rest. The
   WebGPU question was answered with arithmetic: the frame is
   vertex+raster bound on the same silicon — draw less, not port.
5. Remaining backlog (stripe fan-out, progressive tiles, lambda knobs,
   WebGPU) stays gated on demonstrated need: they exist to fix a
   slowness Wave B was built to remove — the first deployed artifact
   run's timing decides.
