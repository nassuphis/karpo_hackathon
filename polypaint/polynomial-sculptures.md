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

## v1 (shipped): lores point cloud, solve-score colors

**Data** rides the Render-tab lores preview (`render-lores-preview` +
`sculpture: true`), so all three source modes work unchanged:

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
- **Style: solid (default) / ghost.** Solid renders opaque with depth
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
- **Tours** (▶/■ next to the tour selector): two parametric camera
  paths — **orbit**, an exterior helical turntable whose height breathes
  past the whole stack, and **weave**, an interior corkscrew with a
  breathing radius sweeping the full height (the close-up detail pass).
  Both follow the height-SCALED geometry so squashed plate stacks get a
  skimming pass. Any manual interaction (canvas pointerdown, a movement
  key, entering fly) stops the tour and re-anchors the orbit target
  ahead of the camera; starting a tour exits pointer lock. Deterministic
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

**Failure modes** are readable messages (no params, fetch failure,
truncated bin, nothing inside the viewport, no WebGL).

## Saved sculptures (the Sculpture tab)

A sixth family tab in the Render artifact panel (after PDF): a create
block plus the global list, DeepZoom-style. **Create** runs the lores
solve with the current Solve-score settings (`sculpture_save: true` +
`sculpture_title`) and publishes a self-contained prefix:

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

## Sizes

lores/logical/recompute previews run ≤256²; 256² × degree 30 ≈ 2M points
≈ 16MB f32 — comfortable for a fetch + a Points draw. The full-solve
transport artifact (500²+) stays out of scope for the browser until it's
subsampled (which "logical" already does server-side).

## Tests

- `tests/test_render_lores_preview_handler.py`: sculpture uploads the
  transformed dump + palette PNG (exact bytes, content types, no-cache
  pinned); non-square grid → friendly error; the no-flag path stays
  upload-free and passes no `--xformed_roots_output`.
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
  plate).
- `tests/e2e/render-solve-score.spec.js`: Sculpture button posts
  `sculpture: true`, opens exactly one window, fragment carries all data
  params; the plain Preview payload stays sculpture-free.

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
