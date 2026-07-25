# Polynomial sculptures

The sweep is parametrized by (t1, t2) ∈ [0,1]², and each (t1, t2) solve
yields N roots on the complex plane. Lift every root set to height z = t2
and the flat root cloud becomes a 3D shape in a cube: x, y span the root
extent (the render viewport), z runs 0→1. The 2D art is literally this
shape's shadow — the sweep marginalizes t2 away when it scatters all roots
onto one plane.

Geometry: a horizontal slice at height z is the root set swept over t1
only — N root trajectories (curves), so the sculpture is generically a
bundle of surface sheets, pinched where roots collide (discriminant
crossings) and shattered where programs step (chessboard mixers). Smooth
programs give continuous sheets; KNIFE/escape programs punch volumes out.

## v1 (HISTORY): lores point cloud, solve-score colors

> Superseded by "Artifact-sourced generation" below (Wave B): the
> preview-coupled sourcing, the `sculpture: true` flag, and the
> Sculpture-next-to-Preview button are GONE. The upload keys, dump
> semantics, palette-PNG conventions, and viewer described here remain
> accurate.

**Data** rode the Render-tab lores preview (`render-lores-preview` +
`sculpture: true`), so all three source modes worked unchanged:

- **lores** (physical): the existing `renders/{job}/lores.bin` is reused
  as-is — no upload.
- **logical**: the server-side subsample of the FULL solve (this is the
  "subsample of a full render" path — no extra machinery).
- **recompute**: fresh solve at the chosen size/solver.

The handler uploads what the viewer needs (public bucket, no presigning):

- roots: `renders/{job}/sculpture_roots.bin` (f32 interleaved `[re,im]`,
  serpentine step order). These are the raster's TRANSFORMED dump
  (`roots2pix_mt --xformed_roots_output`): the root-transform script (rt —
  rotate_roots/CAMERA/KNIFE/…) and the viewport rotation applied by the
  exact C code path that plots, recorded before the finite/viewport clip.
  The raw transport artifact is never uploaded — it ignores both, so a
  sculpture built from it diverges from the plot (user-caught on
  escape-camera pieces). No `lores.bin` reuse in any mode.
- colors: `renders/{job}/sculpture_palette.png` — the per-step palette
  image (grid_n × grid_n, de-serpentined to (row, col), the job's actual
  palette + equalization applied by `render_score_raw`). One solve = one
  pixel = one color, exactly matching the app's Palette tab. PNG is
  lossless, so per-step colors are exact.

The response `sculpture` block carries `roots_url`, `palette_url`,
`grid_n`, `degree`, `step_count`, `pass_count`, `viewport`, `roots_bytes`.
Both uploads set `Cache-Control: no-cache` and both URLs carry a per-run
`?v=<ms>` stamp — fixed keys plus browser heuristic caching served STALE
palettes/roots across runs (user-visible as "the sculpture isn't using my
selected solve score").

**Frontend**: the `Sculpture` button next to Preview (`js/10`,
`runRenderLoresSculpture()` → `runRenderLoresPreview({sculpture:true})`).
The popup window opens synchronously in the click task (popup-blocker
safe) and is pointed at the viewer once the links exist; blocked popups
fall back to a URL line in the render log. Busy + lingering ✓/✗ feedback
on the button.

**Viewer**: `sculpture.html` — standalone, fully inline (no module
stamping in deploy.sh; stable key), vendored three.js r160 +
OrbitControls (newly vendored `addons/controls/OrbitControls.js`). Boot
parses hash params (`r`, `p`, `n`, `d`, `s`, `x0/x1/y0/y1`, `t`), fetches
roots + palette PNG, reconstructs each step's (row, col) with the
serpentine rule `col = (row & 1) ? gridN-1-j : j` (mirroring
`roots2pix_mt.c`), and builds one THREE.Points cloud:

- X = Re, Z = −Im (right-handed view), Y (up) = t2 − ½; cube side 1,
  wireframe frame; xy normalized by the isotropic viewport.
- **Style: solid (default) / ghost / cloud.** **Cloud** is the density
  aesthetic in 3D: points render as soft gaussian splats accumulated
  ADDITIVELY into a half-float buffer, then tone-mapped
  1−exp(−x·exposure) — stacked sheets glow, caustics bloom, isolated
  points stay faint, exactly how the 2D pipeline treats density. Lines
  go additive in this style; the **glow** slider drives the exposure;
  the z-window clips inside the splat shader (ShaderMaterial clipping
  chunks); size feeds the splat radius; style and glow ride saved
  views. Pinned with a real readback: splats visible through the
  two-pass pipeline, bounded output, additive lines, plane count, and
  the material swap restoring byte-exact solid. Solid renders opaque with depth
  writes — occlusion is correct from every angle. The original material
  had `depthWrite: false` (translucent point-cloud habit), so pixel
  ownership followed DRAW order, not distance: rotating the sliced
  sculpture showed bottom-plate points painted over the top plate
  (user-caught). Ghost keeps the translucent veil as an explicit choice
  whose ordering artifacts are inherent (no per-point sorting). The
  smoke spec pins occlusion behaviorally with a zero-parallax setup:
  camera axis straight down through one root spot, center pixels must
  be top-plate colored (mutation-tested against depthWrite:false; note
  perspective parallax makes "look from above, count bottom pixels"
  scenarios flaky — sprite scale uses h/2 while projection uses the
  focal length, so cover margins mislead).
- Exact colors: `THREE.ColorManagement.enabled = false` + renderer
  `LinearSRGBColorSpace` so the palette PNG's sRGB bytes pass through to
  the screen unconverted — the default pipeline treats vertex colors as
  linear and washes them out vs the 2D preview. The smoke spec renders a
  frame and asserts byte-exact palette triples via `gl.readPixels`
  (mutation-tested: reverting the flags fails the pin).
- Pass 0 only (`gridN²` steps) — matches the palette image's semantics
  for multi-pass jobs.
- Non-finite roots and roots outside the xy viewport are dropped and
  counted (`clipped` in the HUD).
- **Tours** (▶/■ + a ½×/1×/2×/4× speed selector that scales the tour
  clock and rides saved views as `tourSpeed`): three parametric camera
  paths — **orbit**, **wave** (the orbit's circle plus a vertical sine
  spanning the scaled stack, 3 bobs per lap = a closed crown;
  phase-matched to START at the play-time height and rise first, per
  user spec "start at my current height, continue to top then down" —
  the galactic-disk vertical-oscillation motion), **grand** (a stately
  4-lap cycle at the play-time radius: a full circle at the TOP plateau,
  a smoothstep descent lap, a full circle at the BOTTOM plateau, an
  ascent lap, always watching the center; plateaus default to +1.5/−0.5
  and adopt the play-time height — mirrored through the cube center —
  when starting beyond them, with the cycle beginning at the NEARER
  plateau so you start circling at your own level; eased entry), and
  **weave** (interior corkscrew with a breathing
  radius sweeping the full height — the close-up detail pass). Tours
  ADOPT the camera pose at play time (user: "orbit too close — take my
  current distance; I navigate to where the orbit starts"): orbit
  circles at exactly the play-time radius, height, and azimuth — zero
  jump by construction — and weave starts at the play-time azimuth and
  smoothstep-eases in from the current pose over ~1.2s before locking
  onto its path.
  Both follow the height-SCALED geometry so squashed plate stacks get a
  skimming pass. Any manual interaction (canvas pointerdown, a movement
  key, entering fly) stops the tour and hands the ORBIT PIVOT back at
  the tour's own look point — the cube middle for orbit (user-caught:
  the fly-exit "0.9 ahead" anchor moved the rotation center to the
  viewpoint, so post-tour drags orbited the camera and felt reversed).
  Fly exit keeps the ahead-anchor (first-person semantics). Starting a
  tour exits pointer lock. Deterministic
  in t — the smoke spec drives the tick and pins the parametrization,
  radii (exterior 1.35 vs interior <1), mode-switch reset, and the
  interaction-stops handoffs.
- Controls: orbit/damping for outside-in, plus **fly mode** (user:
  orbit alone was "terrible") — double-click the canvas to pointer-lock,
  then mouse look + WASD along the TRUE look direction (not walk-plane),
  Q/E down/up, Shift 3× sprint, Esc back to orbit; the orbit target
  re-anchors 0.8 ahead of the flown pose so the handoff doesn't snap.
  Keys are ignored while a panel input/select has focus. The HUD hint
  line tracks the active mode. (PointerLockControls was already
  vendored for gallery.html; movement is hand-rolled because PLC's
  moveForward is walk-plane.) e2e uses a `flight.forceActive` escape
  hatch since headless CI has no OS pointer lock — it pins dblclick →
  requestPointerLock, orbit handoff on lock/unlock, look-direction
  displacement, sprint 3×, Q/E vertical, and input-focus key guarding.
  Point-size slider (default 10); **height slider** (default 0.1) — 0
  flattens the sculpture onto its base plane, showing the shadow ≙ the
  2D art.
- **Threads** (show checkbox; per user spec): root trajectories along
  t2 — each solve matched to the NEXT t2 column in its t1 row by
  mutual-nearest on complex-plane (xz) distance with greedy repair for
  the leftovers (an earth-mover/assignment problem; exact JV/Sinkhorn
  skipped deliberately — many tiny hard LAPs with a warm-start
  near-identity prior, and root collisions are genuinely ambiguous so
  optimality buys nothing visually). Matching ignores Y, computed
  lazily once per session, cached. **Per slice: threads NEVER bridge
  plates** — a t2 pair draws only when both columns land on the same
  plate; slices off connects the whole chain unbroken. Clipped roots
  simply drop out (KNIFE punch-outs dissolve threads at the boundary).
  The show control is now three checkboxes (pts/rib/thr; default pts).
- **z axis: t2 / t1** (user ask): the transpose — height becomes t1 and
  the whole vertical semantics follow: slices bin by the active axis,
  and THREADS transpose their adjacency (t2 mode matches adjacent
  columns within a t1 row; t1 mode matches adjacent rows within a t2
  column) with a separate lazily-computed matching cache per axis.
  Ribbons are per-solve and unaffected. Travels with saved views
  (`meta.view.zaxis`). Pinned: step (row 1, col 3) flips Y +0.25 ↔
  −0.25; the threads fixture (col-dependent roots) makes t1-threads
  purely vertical (dx=dz=0, dy=0.25 × 24 segments).
- **z-window** (user ask: "two sliders, lo and hi, restrict the z
  extent shown — good for exploration"; refined to "same line, one
  slider, two knobs"): a single-track dual-thumb range (two overlaid
  inputs, thumb-only pointer-events, a fill bar marking the span, and a
  coincident-thumb z-index rescue so stacked knobs stay grabbable —
  drag-clamped so lo ≤ hi always) drive two world-space clipping planes on
  points/ribbons/threads — fragment-level slab, no rebuild, segments
  cut exactly at the plane; the cube frame stays as reference. Plane
  constants track the height scale live and carry a 0.005 OUTWARD
  margin: geometry spans y = ±0.5·scale inclusive and a plane at
  exactly that float culls boundary plates to rounding (caught by the
  occlusion pin going dark). Travels with saved views (zlo/zhi,
  sanitizer orders lo ≤ hi). Behavioral pin: clipping the top plate
  makes the bottom plate visible from straight above, and [0,1]
  restores it.
- **len% quantile** (0–100 slider; user: "clip thr and rib paths above
  the q-th quantile — so I can see which parts have the large jumps"):
  draws only the shortest q% of ribbon AND thread segments. Segments
  emit LENGTH-SORTED, so the slider is a pure `setDrawRange` — O(1)
  scrubbing, no rebuild, at any size. Scrubbing down is a persistence
  filtration on the connection graph: the longest links (forced
  bridges, matching failures at collisions) vanish last/first
  respectively, so the jumpy regions announce themselves. Travels with
  saved views (capture → sanitizer → meta.view.lenq). NOTE the sort
  changes buffer order, not the drawn set — pins that read "the first
  segment" must account for it.
- **Clu** (4th show primitive — user-corrected taxonomy: rib connects
  roots within ONE solve, clu connects across a whole z-group, so it is
  a different object, not a ribbon order; the order select is labeled
  "rib connect" accordingly, and the panel runs show → rib connect →
  len% with tour LAST): per z-column, k-means the whole slice (k =
  degree, deterministic strided init + 8 Lloyd iterations), chain each
  cluster greedily from its FARTHEST point (mid-curve starts guarantee
  a backtrack chord), 2.5× median-NN cut per cluster. Clusters
  approximate the root-trajectory arcs, so clu draws the slice curves —
  long flowing lines with per-point color gradients, flat per column.
  Lazy-built on first check, rebuilt on slices/z changes, rides
  solid/ghost, len%, the z-window planes, and the saved-view pipeline
  (show.clu). Pinned: two drifting arcs separate perfectly (zero
  cross-arc bridges), and the per-solve ribbons remain a separate,
  untouched primitive.
- **Slices** (off/2..32): bin t2 onto S discrete plates spanning the
  cube (S=11 → z = 0, 0.1, …, 1.0). Every solve sits at one t2, so
  whole ribbons land intact on their plate; the sculpture becomes an
  exploded shadow — a stack of translucent plates whose sum is the 2D
  artwork. Viewer-only: Y quantized in place from the retained per-point
  t2, ribbons rebuilt. Levels are floor(t2·S) clamped, plates at
  level/(S−1) so the stack spans the full cube height.
- **Ribbons** (per user spec): one polyline per solve — each row of the
  root table (columns = degree) connected at that row's z, monochrome by
  construction since the solve-score is per solve. `connect` selector
  picks the within-row order — roots have no intrinsic order, the table
  order is a solver artifact:
  - **nearest** (default; user: "root should ribbon to nearest"): greedy
    nearest-neighbor chain, O(k²) per solve — start at the root farthest
    from the centroid, hop to the nearest unvisited root, OPEN path —
    then **cluster cut**: segments longer than 2.5× the solve's median
    nearest-neighbor distance are dropped, one short strand per cluster.
    The cut is the load-bearing part (user-caught "ribbon jumping",
    verified on compute_mryrv8zk lores data): the manifold is smooth
    (matched movement ≈1.2% of root spacing per step) and the chain is
    stable along t2 (99.4% identical edges), but the constellations
    split into 2–4 separated clusters (conjugate bands + outliers) in
    100% of sampled solves, so ANY full path is forced to bridge them
    with a ~3.3×-spacing chord — 2-opt measured useless (3.27→3.13)
    because the chords are topology, not greedy error. Ribboning "to
    nearest" therefore means refusing the bridge, not optimizing it.
  - **angle**: tour each row's constellation by angle about its
    centroid, CLOSED for 3+ roots. Solver-independent, coherent, stable
    between neighboring solves.
  - **file order**: the solver's own row order, OPEN-ended. For the
    warm-started Aberth lores solve this order is a fossil of the chain;
    for CM/JT rows it is essentially arbitrary (torn sheets expected).
  One `LineSegments` draw call for all ribbons; rows with clipped roots
  just tour the survivors. Plain WebGL lines are 1px — real thickness
  would use the already-vendored fat-line addons.
- `window.__sculptureViewer` exposes scene/points/ribbons/sculpt for e2e.

**Menu toggle** (user: phone screens): a ☰ button (top-right) hides or
shows the control panel. Shares open with the menu OFF (clean phone
view — the recipient taps ☰ for controls); the in-app ephemeral
preview opens with it ON. Mode comes from the boot config (`saved`
flag), not from the captured view — the creator's panel state never
overrides the share-clean default.

**Failure modes** are readable messages (no params, fetch failure,
truncated bin, nothing inside the viewport, no WebGL).

## Saved sculptures (the Sculpture tab)

A sixth family tab in the Render artifact panel (after PDF), scoped to
the CURRENT job like every other family (the server list is global;
the client filters — mixing jobs into a per-job panel was a user-caught
design bug). The flow is snapshot-save, ONE window ever (the original
Create both re-solved and opened a second window — "atrocious", and the
re-solve could silently diverge from the tuned viewer):

1. **Sculpture** button → the one ephemeral viewer. Tune it; start a
   tour if the share should autoplay. Viewers are static pages — no
   server process; close tabs freely.
2. Sculpture tab → **Save**: a storage-side S3 COPY of exactly the
   ephemeral objects the viewer is showing (`/save-sculpture`; source
   keys derived from job_id server-side, never client-supplied; the
   head-checked size must match grid×degree or it demands a fresh run)
   plus the live view capture. No re-solve, no new window — a row
   appears. Saved sculptures are immutable snapshots; Save again for a
   new version.

The durable prefix:

```
sculptures/{scu_<ms-base36>}/
  viewer.html      frozen copy of sculpture.html (ships in the lores zip)
  meta.json        title, job, grid_n, degree, viewport, palette, created_at
  roots.bin        transformed dump (copied — never a reference into renders/)
  palette.png
```

The share URL is `…/sculptures/{id}/viewer.html`; opened bare, the
viewer reads its sibling `meta.json` and resolves data relatively
(hash-param mode still serves the ephemeral button). **Prepared
flythroughs (user ask)**: Create snapshots the LAST viewer window the
app opened (same-origin read of its live controls) — point, height,
slices, show, style, connect, and whether a tour is PLAYING and which —
whitelist-sanitized server-side into `meta.view`; the frozen viewer
applies it on boot and autoplays the captured tour, so shares open
exactly as tuned. The create block shows a live hint of what will be
captured (or "viewer defaults" when no viewer is open). Workflow:
press Sculpture, tune, start the tour you want, press Create. The import map is
ROOT-ANCHORED (`/vendor/…`) because the frozen copy lives one level
deep — relative `./vendor` would 404 (caught by the saved-mode smoke
test). Data objects are immutable-cached (unique ids); deletes go
through the narrowed `/delete-prefix` (now also `sculptures/<id>/`);
`/list-sculptures` is one delimiter pass + parallel meta reads. The
list is session-cached; the family registers in `_renderArtifacts` so
the active-family reset never bounces off it (the rebuild resets
unknown families to color).

## Artifact-sourced generation (Wave B of sculpture-refactor.md)

**The saved color artifact is the ONLY source** (user: "remove preview
from the loop completely; the data only comes from a saved hires color
artifact, the one currently selected; only 384 and 512"). The
Sculpture tab's first line shows `source: <color artifact id>` — the
Color family's current selection; Generate is disabled without one.

Everything derives from the artifact's recorded provenance, never
from live render state:

- geometry: the transport roots, logical-materialized at the 384²/512²
  lattice (range GETs, no solving), transformed by the ARTIFACT's
  compiled `root_transforms` chain + `rotation` over the ARTIFACT's
  viewport in the fused raster (trivial m0 score — the raster pass
  exists only to apply transforms and dump u16 positions);
- colors: the artifact's STORED `step_scores.raw` (solve order, u8),
  subsampled at EXACTLY the same lattice (`_logical_row_mapping`,
  shared with the materializer), de-serpentined row-major, equalized,
  and rendered through the artifact's palette/interpretation/
  background — **zero score evaluation**;
- provenance: `source_artifact_id` rides the sculpture block,
  `_lastSculptureData`, the save payload, `meta.json`, and the saved
  list row (`src …`).

The job is nearly pure I/O (~30s-class vs the old 110–188s runs whose
solve-score evaluation API Gateway could never wait for). It is ALWAYS
async on the common task infra: `/start-sculpture-artifact`
{job_id, artifact_id, n} head-checks the artifact **synchronously** —
artifacts predating step-scores storage get an immediate "re-render it
to sculpture it" instead of a dead rail card — then registers
`sculpture_artifact_<ms>` and async-invokes the lores lambda's
`artifact_sculpture` mode, which owns the row lifecycle (running →
done with the sculpture block, or error). The app polls
`/check-status` every 3s and the run rides the jobs rail; the Generate
button shows elapsed and a lingering ✓/✗.

Deleted concepts (the win): the preview-sized mode, the
solve-score-mode requirement, live-settings coupling, the sync path,
and the `preview` option in the size select. The u16 dump contract is
unchanged: 0..65534 spans the viewport per axis, (65535,65535) is the
sentinel, and the viewer dequantizes as before.

Grid-integrity guards: the artifact's `step_scores_grid_n` must equal
the transport grid (else scores and roots would describe different
solves), and the requested size must not exceed it.

## Sizes

lores/logical/recompute previews run ≤256²; 256² × degree 30 ≈ 2M points
≈ 16MB f32 — comfortable for a fetch + a Points draw. The full-solve
transport artifact (500²+) stays out of scope for the browser until it's
subsampled (which "logical" already does server-side).

## Off-thread topology (Wave A of sculpture-refactor.md)

At 384² × degree ~45, the synchronous line-primitive builds froze the
tab (clu ≈ 7–8 billion ops; the compute lane IS the paint lane, so not
even a busy hint could render). The viewer now computes ALL topology in
an inline Web Worker (Blob URL, no extra file):

- **Worker owns**: per-solve ribbon chains (nearest/angle/file —
  semantics unchanged, verbatim ports), adjacent-solve thread matching
  (mutual-nearest + greedy repair, per z axis), and clu clustering.
  Topology depends only on the xz layout, never on Y, so the worker
  holds one immutable copy of the positions (transferred at boot) and
  results cache per key (ribbons: order; threads/clu: z axis) — slices,
  height, z-window, and z-axis changes never re-run it.
- **Main thread owns**: buffer emission. `emitSortedSegments` reads
  live Y, applies the threads slice gate, and orders segments with a
  2048-bucket length sort — O(n), replacing the comparator argsort that
  itself took seconds at 6.5M segments; the len% drawRange semantics
  are unchanged at slider resolution.
- **clu algorithm upgrade**: centers fit on a ≤2048-point strided
  sample then one assignment pass over all points (~8× cheaper, same
  clusters), and chains order each cluster along its PRINCIPAL AXIS
  (closed-form 2×2 eigenvector, O(m log m)) with gaps cut at 2.5× the
  median consecutive gap — no more greedy O(m²), and no greedy
  backtrack chords.
- **Laziness**: nothing builds until its show checkbox is on (boot at
  any size is instant); jobs run in ~40 ms slices with progress posted
  to a `#hud-build` line ("building clu 120/384 columns"); unchecking
  cancels mid-build (main side resolves null synchronously, the worker
  drops the job between slices). `__sculptureViewer.pendingBuilds`
  exposes the outstanding refresh count — the e2e suite waits on it.

## Tests

- `tests/test_render_lores_preview_handler.py`: artifact mode pins —
  the raster runs with the ARTIFACT's rotation/viewport/xforms (the
  xforms file is read back inside the subprocess fake), stored
  solve-order scores subsample to the exact de-serpentined palette raw
  (`[0,1,2,3,7,6,5,4,…]` full-grid; `[0,2,8,10]` for view 2 over grid
  4 — the logical-lattice identity), no clip/summary subprocess ever
  runs, uploads + running→done row lifecycle, missing step_scores →
  "re-render" error row, view>grid and grid-mismatch guards.
- `tests/test_raster_mt_parity.py`: compiles the real C locally and pins
  the dump against rotate_roots(0.25) + a 0.5-rad viewport rotation in
  closed form, multi-worker (threads=2), including an out-of-viewport
  root the plot clips but the dump must keep.
- `tests/e2e/sculpture-viewer-smoke.spec.js` (SwiftShader, gallery-smoke
  pattern): module graph + no-params message; truncated-bin message; real
  scene build with serpentine z pin (step 4 → col 3 → Y=0.25), exact
  per-step color pin, clipped=0, shadow flatten. Ribbon pins use a
  degree-3 triangle whose angular tour (C,A,B closed) differs from file
  order (A,B,C open) — degree-2 rows are DEGENERATE for order pins (both
  orders draw the same segment; which vertex leads hinges on IEEE zero
  signs in atan2). Threads pins use a fixture whose ODD columns store
  the two roots file-order-swapped, so slot-identity matching is
  provably wrong at every even→odd boundary (mutation-tested); slices=2
  drops the plate-1→2 bridge (24 → 16 segments, all flat on their
  plate). Worker-era additions: line topology is LAZY (ribbons geometry
  is EMPTY until rib is checked — pinned), every build-triggering
  control change awaits `pendingBuilds === 0` before reading geometry,
  and the cancel pin unchecks clu in the same tick as an uncached t1
  build request (the worker round trip can't complete synchronously, so
  cancel deterministically wins; the old emission must survive and the
  re-check must rebuild).
- `tests/e2e/render-solve-score.spec.js`: the tab shows
  `source: <color id>` from the seeded inventory rebuild, sizes are
  exactly [384, 512], Generate dispatches the EXACT payload
  {job_id, artifact_id, n} (no live render state), never calls
  render-lores-preview, rides the rail to complete, and the no-artifact
  state disables Generate; the save payload pins `source_artifact_id`.
- `tests/test_storage_handler.py`: /start-sculpture-artifact registers
  the task and invokes the artifact mode with the exact payload,
  fails FAST (sync 400 "re-render") without step_scores, rejects bad
  ids/sizes and sizes beyond the solve grid; save persists
  `source_artifact_id` into meta.

## Future (not in v1)

- **Threads**: root-identity polylines along t2 (across solves — distinct
  from the per-solve ribbons above). The full-render transport keeps
  `matchRoots` identity along the serpentine chain, but the lores paths
  solve with `match_roots: False` — a threads mode needs either a
  match_roots lores variant or client-side matching. Fat-line addons are
  already vendored.
- **z = t1 transpose toggle** (cheap, viewer-side reindex).
- **Density voxels** (3D histogram — the true 3D analog of the density
  art) and **marching-cubes mesh export** (STL/GLB → printable sculpture).
- **All-passes view** for times>1 jobs (extra sheets at the same z).
