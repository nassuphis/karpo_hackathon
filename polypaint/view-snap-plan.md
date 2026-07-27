# ViewSnap: render a Sculpture camera pose at full ViewRender resolution

Status: implementation plan, 2026-07-27.

## 0. Verdict and hard decisions

This feature is feasible. The expensive infrastructure already exists:
ViewRender runs the normal ColorRender-MT workflow at the calculation's
full `N x N` parameter grid and stores the result as a first-class Views
artifact. ViewSnap should extend that path with one new projection,
`camera`, rather than create another renderer.

The product contract is:

1. Select a saved full Sculpture in the Render tab's Sculpture catalogue.
2. Move the live embedded viewer to the desired camera position.
3. Press `Snap` in the action row above the catalogue.
4. Capture the live viewer camera and point-cloud geometry settings.
5. Dispatch the normal full ColorRender-MT workflow using the Sculpture's
   exact source Color artifact.
6. Produce an `N x N` artifact in the existing Views family.

Hard decisions:

- Do not take a browser screenshot and upscale it.
- Do not render from the saved Sculpture's reduced 128/192/384/512 grid.
- Do not add a storage route, a lores Lambda mode, or a second render
  workflow.
- Add `view_projection="camera"` to the existing full ViewRender path.
- Camera ViewSnap v1 renders the solid point primitive with depth testing.
  It must not silently claim to reproduce ribbons, threads, clusters,
  baked splats, ghost blending, or cloud/additive rendering.
- The camera matrices, not yaw/pitch reconstruction, are authoritative.
- Output remains exactly the calculation's `N x N`, subject to the existing
  renderer limit and a calculated resource-admission check. Do not invent
  an arbitrary lower pixel cap.
- A camera projection is not acceptable without nearest-depth ownership.
  The current ViewRender "lowest section/first point wins" contract does
  not reproduce the solid Three.js viewer.
- Exact equal-depth collisions inside one raster section may use lock
  acquisition order in v1. Do not spend another dense `uint32` plane merely
  to make an art-output boundary case deterministic. Cross-section ties stay
  deterministic by processing fragment ordinals in ascending order.
- Catalogue previews use the current packaged `sculpture.html` with the saved
  row's public `meta.json` as input. Public share links continue using their
  frozen per-save `viewer.html`. Do not mutate old published Sculpture
  prefixes just to add Snap.

No API manifest change should be required. The frontend continues to call
the existing render orchestrator.

## 1. Current state, verified in code

### 1.1 Sculpture viewer

`sculpture.html` builds a Three.js `PerspectiveCamera(50, aspect, 0.01,
50)`, `OrbitControls`, and a `sculpt` object containing the point cloud and
optional topology.

The point coordinates before `sculpt.matrixWorld` are:

```text
cx   = (min_re + max_re) / 2
cy   = (min_im + max_im) / 2
side = max(max_re - min_re, max_im - min_im)

x = (root_re - cx) / side
y = selected_t - 0.5
z = -(root_im - cy) / side
```

`selected_t` is `t1` or `t2`. `sculpt.scale.y` applies the Height control.
The Slices control can quantize `selected_t` before `y` is constructed.
The z-range controls clip the resulting world-space height.

The live objects are already exposed through
`window.__sculptureViewer`, including `camera`, `controls`, `sculpt`, and
the point material. The saved `view` metadata does not contain the live
camera pose. `sanitize_sculpture_view()` currently preserves point size,
height, slices, visibility, style, glow, ordering, selected parameter
axis, z-range, and tour settings only.

Saved full viewers are copied into each `sculptures/{id}/viewer.html` and
served with immutable caching. Those copies are the stable public share
experience. They must not be the catalogue Snap implementation: old copies
do not contain the new protocol, and overwriting them would not update
already-cached copies for up to a year.

The catalogue can instead load the current app-packaged `sculpture.html`
with the selected row's public `meta.json` URL. That viewer can resolve the
saved sibling `roots.bin` and `palette.png`, apply the saved `view`, and
participate in the current Snap protocol without rewriting the saved
prefix.

### 1.2 Sculpture catalogue

The Render tab's Sculpture family uses the shared render-artifact panel.
The selected catalogue item is embedded in an iframe after a debounce.
The action row already contains SaveFull, SaveSplat, Refresh, Open,
Copy Link, and Delete.

The global Sculpture inventory contains `id`, `job_id`,
`source_artifact_id`, viewer URL, viewport, and saved-view metadata for
full saves. A full-resolution snap is possible only when:

- the selected row is a full Sculpture, not `kind="splatbake"`;
- `job_id` and `source_artifact_id` are present;
- the source Color artifact and the original calculation still exist;
- the source Color artifact carries the normal ViewRender provenance,
  including a valid viewport and solve-score source.

The command must resolve the source from the selected Sculpture's explicit
identity. It must not use whichever Color row happens to be selected in
global UI state.

### 1.3 ViewRender

The existing ViewRender flow is:

```text
js/11-artifacts.js
  _viewRenderParamsFromArtifact(...)
  _dispatchRenderOrchestrator("color", ...)

handler_render_orchestrator.py
  existing render state machine

handler_render_plan.py
  view_projection != "plan"
  family = "views"
  pix = calc N

workflow_contracts.py
  view flags enter every ColorRasterMap item

handler_raster_mt.py
  roots2pix_mt --view_projection=...

handler_finalize_mt.py
  normal fragment assembly, color mapping, image/preview/meta publication

handler_storage.py /list-views
  shared Views catalogue
```

The supported projections are currently `front`, `rear`, `left`, `right`,
`radial`, and `isometric`. `roots2pix_mt.c` changes only the root-to-pixel
mapping; scoring, root transforms, rotation, fragments, final palette
mapping, output publication, jobs-rail status, and Views integration are
shared with ColorRender-MT.

That is the correct extension point for ViewSnap.

### 1.4 The depth-contract gap

`roots2pix_mt` currently claims a pixel through a shared bitset. Within a
raster section, the first thread to claim the pixel wins. Across raster
sections, `assemble_greyscale` deterministically gives the pixel to the
lowest fragment ordinal.

That rule was consciously accepted for the existing architectural views,
but it is wrong for an arbitrary perspective camera. The solid Sculpture
viewer has depth testing and depth writes enabled: the nearest visible
point must win regardless of source section or worker timing.

Changing only the projection matrix would therefore produce the right
silhouette from the wrong occlusion order. ViewSnap must add a
depth-carrying fragment contract.

There are two additional existing constraints:

- `handler_finalize_mt.py` splits image metadata through the
  `COLOR_IMAGE_HEADER_KEYS` whitelist before enforcing the 2 KiB S3
  user-metadata limit. A camera payload is suitable for the Views
  `meta.json`, but must never be added to that image-header whitelist.
- `assemble_greyscale` currently downloads each complete fragment into
  memory before parsing it, with several fragment workers active. That is
  not safe for camera fragments approaching gigabytes. Camera mode needs a
  bounded streaming parser; the legacy path can remain unchanged.
- `RENDER_FINALIZE_MT_FRAGMENT_MANIFEST` sources `fragment_encoding` and
  `record_size_bytes` from `$.solve_score_clip.parsed.*` — the CLIP
  phase's solve-score contract, which describes only the existing `4 + C`
  records and knows nothing about a camera projection. Without a new
  authority, a camera raster writing `8 + C` records would be parsed by
  Finalize under the wrong contract. Section 7.1 defines that authority.

## 2. V1 product behavior

### 2.1 Action row

Add a `Snap` button beside Open/Copy Link/Delete in the Sculpture action
row.

The button is enabled only when all of the following are true:

- a full Sculpture row is selected;
- its embedded iframe has announced the ViewSnap protocol as ready;
- the ready message belongs to the selected Sculpture and current iframe
  generation;
- the row has `job_id` and `source_artifact_id`;
- `_activeRenderRun` is false;
- no snapshot request is already in flight.

Disabled states need informative `title` text:

- `Select a full Sculpture first`
- `Baked splat snapshots are not supported in v1`
- `Viewer is still loading`
- `Source Color artifact is unavailable`
- `Wait for the active render to finish`

The button lifecycle is:

```text
Snap -> Capturing... -> Dispatching... -> checkmark Snap
                              |
                              +-> explicit error in render-log
```

The operation stays on the Sculpture tab. The normal jobs rail reports
the full render. On completion, the existing Views completion hook
refreshes and selects the new result.

### 2.2 What v1 reproduces

V1 reproduces:

- the exact current perspective camera pose;
- a square center-crop of the current camera view;
- selected `t1` or `t2` vertical axis;
- Height;
- Slices;
- z-range;
- solid points;
- point-size attenuation;
- the source Color artifact's root transforms, rotation, score program,
  palette, output format, quality, background, and viewport;
- pass 0 only, exactly the solves the viewer displays (later passes are
  never plotted in camera mode);
- nearest-depth occlusion;
- the source artifact's palette and score mapping, equalized over the
  SNAP OUTPUT like every other render — deliberately NOT a replay of the
  viewer's exact color transfer (see section 9.5a for the disclaimer and
  the recorded v2 path to exact parity).

V1 does not reproduce:

- ribbons;
- threads;
- cluster lines;
- voxel-baked splats;
- cube/frame lines;
- ghost alpha blending;
- cloud/additive density;
- tours as motion.

The cube frame and the z-range slab guides are navigation CHROME, not
data: the viewer always creates them (`sculpture.html` builds the frame
unconditionally as the spatial reference), so treating them as
unsupported primitives would disable Snap permanently. They are
deliberately omitted from the output and never block a snap.

If any unsupported DATA primitive is visible (ribbons, threads, cluster
lines, splats), or the style is not `solid`, or `show.points` is FALSE
(the viewer permits hiding the points layer — snapping points the user
was not seeing is the same silent-surprise class), the viewer must not
silently dispatch a different-looking image. The Snap response should carry a
structured unsupported-state list and the parent should refuse with a
concise explanation. A later version can deliberately add those render
contracts.

### 2.3 Full saves only

`kind="splatbake"` uses a different self-contained WebGL renderer and
different geometry (voxel-reduced anisotropic splats). Projecting the
original roots from its camera would not reproduce the selected baked
image. Snap is therefore disabled for baked splats in v1.

## 3. Snapshot messaging protocol

Do not access `iframe.contentWindow.__sculptureViewer` directly. It works
only when app and saved viewer happen to share an exact origin, and fails
under the S3 website endpoint, REST endpoint, or a future CDN/custom
domain. Use a versioned `postMessage` protocol.

### 3.1 Handshake

Viewer to parent, after the viewer and its source data are ready:

```json
{
  "type": "polypaint-sculpture-ready",
  "protocol_version": 1,
  "sculpture_id": "scu_..."
}
```

Parent to viewer:

```json
{
  "type": "polypaint-sculpture-snapshot-request",
  "protocol_version": 1,
  "request_id": "snap_...",
  "sculpture_id": "scu_...",
  "target_aspect": 1
}
```

Viewer to parent:

```json
{
  "type": "polypaint-sculpture-snapshot-response",
  "protocol_version": 1,
  "request_id": "snap_...",
  "sculpture_id": "scu_...",
  "snapshot": { "...": "schema in section 4" }
}
```

### 3.2 Identity and origin checks

The parent must verify all of:

- `event.source === currentIframe.contentWindow`;
- `event.origin === new URL(currentIframe.src).origin`;
- exact protocol version;
- exact request id;
- exact selected Sculpture id;
- the iframe generation has not changed since the request;
- the selected row is still the same row.

The catalogue iframe is the current packaged viewer, not the row's frozen
public `viewer_url`, so origin validation must use the iframe URL actually
loaded. The viewer should answer only the requesting window and use
`event.origin` as the response target. No message may carry or select an
S3 key. The parent supplies authoritative `job_id`,
`source_artifact_id`, and Sculpture id from the selected inventory row.

Use a bounded timeout. A stale response after row selection changes must
be ignored rather than dispatched against the successor row.

## 4. Canonical camera snapshot schema

Use one canonical matrix representation. Do not send position,
quaternion, target, and matrices as competing authorities.

The authoritative payload is:

```json
{
  "version": 1,
  "projection": "perspective",
  "matrix_layout": "column_major",
  "model_view_matrix": [16 finite numbers],
  "projection_matrix": [16 finite numbers],
  "vertical": "t2",
  "slices": 0,
  "effective_tlo": 0.0,
  "effective_thi": 1.0,
  "point_world_size": 0.004,
  "point_scale": 1.0,
  "point_min_fraction": 0.0009765625,
  "point_max_fraction": 1.0,
  "style": "solid",
  "show": {
    "points": true,
    "ribbons": false,
    "threads": false,
    "clu": false,
    "splats": false
  },
  "frame": {
    "aspect": 1.0,
    "crop": "center_square"
  },
  "debug": {
    "camera_position": [3 finite numbers],
    "camera_target": [3 finite numbers],
    "vertical_fov_degrees": 50.0,
    "near": 0.01,
    "far": 50.0,
    "height": 0.1,
    "control_zlo": 0.0,
    "control_zhi": 1.0,
    "point_control": 10
  }
}
```

`model_view_matrix` must already include `sculpt.matrixWorld`, including
the current Height scale:

```text
model_view = squareCamera.matrixWorldInverse * sculpt.matrixWorld
```

The backend reconstructs the unscaled local point `(x, y, z, 1)` and
applies this matrix exactly once. It must not apply Height a second time.

The `debug` block is metadata only. Backend projection must never be
reconstructed from it.

`effective_tlo` and `effective_thi` are authoritative filtering bounds,
not debug copies of the range-control values. The viewer's clipping planes
include a world-space `0.005` outward margin. For control values `lo`,
`hi`, and `sy = sculpt.scale.y`, capture:

```text
if sy > 0:
    effective_tlo = max(0, lo - 0.005 / sy)
    effective_thi = min(1, hi + 0.005 / sy)
else:
    effective_tlo = 0
    effective_thi = 1
```

At Height zero every point has world-space `y=0`, and both viewer clipping
planes retain it; the full effective t-window is therefore the exact parity
case. The backend must not reconstruct these bounds from `height` or the
debug control values.

Before capture:

```text
camera.updateMatrixWorld(true)
sculpt.updateMatrixWorld(true)
```

Reject NaN, infinity, wrong lengths, unexpected keys, wrong enums,
`effective_tlo > effective_thi`, bounds outside `[0,1]`, or a non-square
target aspect at both the frontend and render plan.

`point_scale` and the clamp fractions are CAPTURED, never reconstructed:

```text
crop_side          = min(buffer_width, buffer_height)
point_scale        = 0.5 * buffer_height / crop_side
point_min_fraction = ALIASED_POINT_SIZE_RANGE[0] / crop_side
point_max_fraction = ALIASED_POINT_SIZE_RANGE[1] / crop_side
```

using the DRAWING BUFFER dimensions (the renderer applies
`setPixelRatio(min(devicePixelRatio, 2))`, so CSS pixels are the wrong
basis) and the WebGL implementation's actual point-size clamp range.

Validation matches the REAL viewer ranges — zero-sized footprints must
not slip through and zero out `A_max`, silently bypassing footprint
admission while the raster still does substantial work:

- `0.0004 <= point_world_size <= 0.016` (the point control is 1..40
  at `0.0004` per step in `sculpture.html`);
- `point_scale >= 0.5` (crop side = min of the buffer dimensions, so
  `0.5 * height / crop` cannot be smaller);
- `0 < point_min_fraction <= point_max_fraction`, all finite (WebGL
  guarantees a positive minimum point size).

Pin rejection tests for each bound, and assert every ACCEPTED payload
yields `A_max >= 1`.

Finite floats are not enough: the backend derives the near plane from the
projection matrix and prices admission with it, so both validators must
also check STRUCTURE (column-major indices):

- projection matrix has the Three.js perspective form: `m[11] = -1`,
  `m[15] = 0`, `m[0] > 0`, `m[5] > 0`, zero in the non-perspective
  off-diagonal slots;
- unit aspect: `m[0] == m[5]` within a small epsilon (the square clone);
- derived `near = m[14] / (m[10] - 1)` and `far = m[14] / (m[10] + 1)`
  are finite with `0 < near < far`;
- the model-view matrix is affine: bottom row exactly `[0, 0, 0, 1]`.

A matrix that fails structure is rejected before any resource estimate
uses it.

## 5. Square framing

The embedded viewer is not guaranteed to be square. Reusing its current
projection matrix and then rendering `N x N` would change composition.

The target is the center square visible inside the current canvas:

- Landscape canvas: preserve the current vertical FOV and crop the sides.
- Portrait canvas: preserve the current horizontal FOV and crop top/bottom.

For current aspect `a` and vertical FOV `v`:

```text
if a >= 1:
    square_vertical_fov = v
else:
    horizontal_fov = 2 * atan(tan(v / 2) * a)
    square_vertical_fov = horizontal_fov
```

Clone the live camera, set aspect to `1`, assign the calculated square
vertical FOV, update its projection matrix, and derive the snapshot
matrices from the clone. Do not mutate the live navigation camera.

The viewer should display a center-square framing guide while embedded in
the app so the user knows what Snap will include. The guide must match the
formula above and must not intercept pointer events.

## 6. Exact native projection

Add a camera projection helper, preferably in a focused header such as
`lambda/view_camera_projection.h`; do not bury matrix math among CLI
parsing in `roots2pix_mt.c`.

For each transformed and viewport-rotated root:

```text
local.x = (re - viewport_center_re) / viewport_side
local.y = quantized_or_raw_selected_t - 0.5
local.z = -(im - viewport_center_im) / viewport_side
local.w = 1

view = model_view_matrix * local
clip = projection_matrix * view
```

Reject:

- source roots outside the saved source viewport, matching existing Views;
- quantized-or-raw selected `t` outside
  `[effective_tlo, effective_thi]`;
- non-finite values;
- `clip.w <= 0`;
- points outside clip space;
- points outside the camera near/far range.

Map NDC to the output square with one documented edge policy. Pin NDC
center, corners, and exact boundary behavior in tests.

Camera depth is `-view.z` and must be finite and positive.

### 6.1 Parameter-axis and slice parity

Use the same serpentine-to-grid identity as both the current viewer and
ViewRender:

```text
row = global_step / N
j = global_step % N
col = (row is odd) ? N - 1 - j : j
t1 = row / N
t2 = col / N
```

If `slices > 0`, mirror `sculpture.html` exactly:

```text
level = min(slices - 1, floor(t * slices))
t = (slices > 1) ? level / (slices - 1) : 0.5
```

Do not replace this with `N - 1` denominators simply because they look
more symmetrical; parity with the live viewer is the contract.

Pass semantics are part of the same contract. The viewer displays pass 0
only (`passSteps = N * N` in `sculpture.html`), while the native view
path folds every later pass onto pass-0 t space (`(step_start + p) %
N*N`). Folding is correct for the fixed architectural views but would
render geometry the user is NOT looking at in a camera snap. Camera mode
therefore rasterizes ONLY solves with `global_step < N*N` and counts the
skipped remainder in telemetry. Scoring and the step-scores sidecar are
unchanged — the standard full `grid x times` sidecar is still produced;
only PLOTTING is pass-0-restricted. Pin a `times > 1` fixture proving a
pass-1 root is absent from the camera output and present in a fixed-view
output.

The ordering is also part of the contract:

1. derive raw `t1` or `t2`;
2. apply slice quantization;
3. compare the resulting value with the captured effective t-window;
4. construct local `y = t - 0.5`.

Pin a fixture with `slices > 0`, a small nonzero Height, and a plate exactly
inside the viewer's `0.005` clipping margin.

### 6.2 Point footprint

One projected root per output pixel would preserve the camera direction
but not the current solid point appearance. Mirror Three.js point-size
attenuation for `PointsMaterial` using the captured
`point_world_size = point_control * 0.0004`.

Derive the pixel footprint from camera-space depth using the CAPTURED
scale and clamp values — the naive "buffer height cancels" form is wrong
in portrait (the center-square crop side is the buffer WIDTH there, so
the height/crop ratio survives) and ignores WebGL's
implementation-dependent point-size clamp entirely:

```text
frame_fraction = clamp(point_world_size * point_scale / depth,
                       point_min_fraction, point_max_fraction)
native_point_side_pixels = N * frame_fraction
```

`point_scale` is `0.5 * buffer_height / crop_side` captured in the
viewer (1/2 exactly only in landscape), and the clamp fractions are the
GPU's `ALIASED_POINT_SIZE_RANGE` normalized by the crop side — a huge
near point clamps at the max, a distant point clamps at the min, and
both clamps are part of what the user is LOOKING at. Rasterization and
admission use this same expression. Round the side length with one
pinned edge convention, clip the covered square to the output, and
update every covered pixel through the depth rule. Pin against
browser-generated oracles for LANDSCAPE, PORTRAIT, a minimum-clamped
far point, and a maximum-clamped near point — not just depth sweeps.

V1 may use the solid square point footprint used by `PointsMaterial`.
Antialiasing differences at the boundary are acceptable, but the
footprint extent and depth ownership are not.

## 7. Depth-aware fragment contract

This is the central backend change.

### 7.1 New encoding

Keep existing fixed ViewRender and ColorRender encodings unchanged. Add a
camera-only encoding:

```text
u32le_f32depth_u8_channels_v1

[pixel_idx:uint32 little-endian]
[camera_depth:float32 little-endian]
[channel_byte:uint8] * channels
```

Record size is `8 + channels`.

Add the encoding to the fragment manifest validator. A camera projection
must require this encoding; a non-camera projection must reject it.

The encoding has ONE producer-to-consumer authority: the render plan
emits a `fragment_contract` block — `{encoding, record_size_bytes,
channels}` — chosen at plan time from the projection. Both the raster
item selector and the Finalize fragment manifest read it from
`$.plan.fragment_contract.*`; the Finalize manifest must STOP sourcing
encoding and record size from `$.solve_score_clip.parsed.*` for camera
runs (the clip contract cannot express a camera record). The native
assembler receives the selected encoding explicitly on its command line
and hard-fails on a record-size mismatch. One authority, two readers,
zero inference.

Precedence trap, verified in code: `handler_finalize_mt` resolves
`pair_encoding OR fragment_encoding` — the PAIR field wins — and
validates it against the two legacy encodings only. Rewiring
`fragment_encoding` alone therefore changes nothing: a camera manifest
still carrying the clip-sourced `pair_encoding` is rejected (or worse,
misparsed) before the new field is ever read. `pair_encoding`,
`fragment_encoding`, `channels`, and `record_size_bytes` must ALL be
populated from `fragment_contract` — the competing solve-score-sourced
fields are removed, not merely outranked — and the validator's accepted
set gains the camera encoding.

### 7.2 Section-local ownership

Inside each `roots2pix_mt` invocation, the current claim bitset is
insufficient. Camera mode needs a nearest-depth value and corresponding
score bytes per occupied output pixel.

Use one shared section-local depth/score store across native threads:

- initialize depth to infinity;
- convert each candidate depth to a validated positive FLOAT32 exactly
  once, and use that same f32 value for the section-local comparison AND
  the serialized record — projection math runs in doubles, and comparing
  doubles locally while serializing floats lets two locally distinct
  depths collapse to an equal cross-section depth and flip the winner.
  Pin a fixture where two doubles collapse to one float32;
- compare/update depth and score under striped locks or an equivalent
  race-free primitive;
- mark occupancy in the existing claim BITSET only — do NOT keep a
  uint32 occupied-index list (worst case that is another `4 * P` = 4 GiB
  at `N = 32768`, unaccounted);
- later nearer candidates update the stored depth and score in place;
- after workers join, enumerate the bitset's set bits in pixel order and
  STREAM each record directly to the fragment file.

Camera mode must not retain fragment records in memory: the existing
per-worker `fragmentByteVec` accumulation is explicitly prohibited for
the camera encoding — the depth/score planes already hold the state, and
the fragment is produced once, sequentially, at emit time.

Pass-1-only sections get an explicit cheap path. Every section still
SCORES its steps for the sidecar, but a section whose step range has no
overlap with `[0, N*N)` plots nothing in camera mode: define
`pass0_overlap` per section, allocate the depth/score/occupancy planes
ONLY when it is nonzero, restrict the footprint-work estimate to
`pass0_overlap` roots, and emit a valid EMPTY camera fragment alongside
the normal step-score file.

Do not allocate a dense source-ordinal plane. It costs another `4 * P`
bytes solely for the exact-equal-depth boundary case. In v1, a strictly
nearer finite depth replaces the winner; an exactly equal float depth keeps
whichever thread acquired the stripe first. This limited nondeterminism is
accepted and must be documented and tested as such.

### 7.3 Cross-section ownership

Extend `assemble_greyscale` to recognize the depth encoding. For every
pixel, retain the smallest positive finite depth and its score bytes,
independent of fragment download order or worker scheduling.

The camera path must process fragment ordinals in ascending order and
replace only on strictly smaller depth. Exact cross-section ties therefore
retain the lower fragment ordinal without another owner plane. The legacy
owner-ordinal path remains unchanged for existing fragment encodings.

Camera fragments must be parsed incrementally from the HTTP response body.
Keep only a small carry buffer for a record split across callbacks; never
materialize a complete camera fragment in Finalize memory or `/tmp`.
Process one fragment stream at a time for the initial implementation. Any
future overlapped downloader must preserve bounded memory and the explicit
tie contract.

Reject malformed records, non-finite depth, non-positive depth,
out-of-range pixels, channel mismatch, and record-size mismatch.

### 7.4 Resource accounting

Depth has a real cost. For `P = N*N`, `C = output channels`,
`R_s = candidate roots in section s`, and `A_max = conservative maximum
covered pixels per accepted point`, calculate and report at plan time:

```text
section depth bytes      = 4 * P
section score bytes      = C * P
section occupancy bytes  = ceil(P / 8)
section fragment records <= min(P, R_s * A_max)
section fragment bytes   <= (8 + C) * min(P, R_s * A_max)
absolute fragment bytes  <= (8 + C) * P
final depth bytes        = 4 * P
final score bytes        = C * P
final raw /tmp bytes      = C * P
total fragment bytes     = sum(section fragment bytes)
plus encoded-output and native-reader overhead
```

For the estimate, do NOT price every point at the clip-near plane: with
the viewer's `near = 0.01`, `N = 1000` and `point_world_size = 0.004`
that prices a 200x200 footprint for points that actually render at
roughly 1x1 — safe but unusable, rejecting ordinary snaps. Use the
affine DEPTH INTERVAL of the local bounding prism instead: transform its
eight corners (`x, z` in `[-0.5, 0.5]`, `y` in
`[effective_tlo - 0.5, effective_thi - 0.5]`) by the validated
model-view matrix. Depth is affine in the corners, so the prism's depth
extrema ARE corner depths — and the camera being inside or outside is
irrelevant (a camera OUTSIDE the prism can still have the near plane
slice it, with all corners far while interior points graze near):

```text
depths     = { -view.z over the 8 prism corners }
frustum cull: transform the 8 corners to CLIP space; if all 8 violate
            any ONE of the six half-spaces — explicitly, all eight
            satisfy the SAME inequality among `x < -w`, `x > w`,
            `y < -w`, `y > w`, `z < -w`, `z > w` — REJECT at plan time
            as "empty camera frustum: nothing visible from this pose".
            Reject non-finite derived view/clip coordinates outright,
            and pin corner-on-boundary behavior to the native edge
            policy so admission and rasterization agree at the exact
            frustum surface. Near-only detection misses a prism
            panned off-screen or beyond the far plane, and zero
            footprint does NOT mean zero cost — the run would still
            score every step for the sidecar, allocate and upload the
            full C*P raw, and encode a blank image. A snap of nothing
            is a user error deserving a message, not a full-price blank
            render. Admission tests include an off-left pose and a
            beyond-far pose, not just behind-camera.
elif min(depths) >= near:   pricing_depth = min(depths)
else:                       the prism straddles the near plane ->
                            pricing_depth = near
max_point_side = ceil(N * clamp(point_world_size * point_scale
                                 / pricing_depth,
                                 point_min_fraction, point_max_fraction))
A_max = min(P, max_point_side * max_point_side)
```

Admission and rasterization use the SAME footprint law — including the
captured clamp; admission prices a conservative UPPER BOUND on it (the
bounding-prism depth makes `A_max` an over-estimate by construction,
never an under-estimate).

Clamp all integer arithmetic before multiplication and reject overflow.
The absolute `(8 + C) * P` per-fragment bound remains the admission
backstop even when the occupancy estimate is smaller.

Raster-MT and Finalize-MT are currently 10,240 MiB Lambdas with 10 GiB
ephemeral storage. Admission must use an explicit safety budget tied to
those deployed resources and pinned against `deploy_manifest.json`.

Sectioning must know about the camera planes: `compute_safe_sectioning`
budgets ONLY source-row bytes today. For camera runs it must subtract
the fixed per-section camera allocations (depth + score + occupancy
planes and the step-score buffer) from the source-row budget before
choosing section sizes, or a section that fits its rows can still blow
the Lambda on the planes.

The two stages have different constraints:

- Raster currently writes the complete section fragment to
  `/tmp/fused_fragment.frag` before `upload_fileobj`. The per-fragment
  estimate must fit the Raster `/tmp` budget with named safety headroom.
- Finalize holds one final depth plane and score plane, streams fragments,
  and writes the dense raw output plus encoded artifacts. It must not
  download complete camera fragments to memory or `/tmp`.
- The final score plane IS the dense raw output: one `C * P` allocation
  serves both roles, written out after the merge. `4P + C*P` is
  7.0 GiB at `N=32768`, `C=3` (7P bytes = 7.52e9 B) — inside the
  10,240 MiB Lambda only if nobody allocates the score plane and the raw
  buffer separately. State this in the implementation and pin peak
  memory in the benchmark.

Allocation identities are not a peak model. Admission must price each
PHASE's peak RSS and `/tmp` explicitly:

```text
Raster peak RSS   = section source buffers (logical materialization)
                  + 4P depth + C*P score + P/8 occupancy
                  + C * section_steps worker step-score bytes (the
                    sections still SCORE every step for the sidecar)
                  + native reader + streaming emit buffer
Raster /tmp       = fragment file <= (8 + C) * occupied
                  + /tmp/step_scores.bin (C * section_steps) — BOTH
                    files coexist until the handler's sequential uploads
                    complete  (hard: their SUM fits the 10 GiB budget
                    with named headroom; 11 * N^2 at N=32768 is
                    11.0 GiB and REJECTS on the fragment alone)
Finalize peak RSS = 4P depth + C*P score/raw (one allocation)
                  + bounded stream carry buffer
Finalize /tmp     = C*P raw + step-scores sidecar (C * P * times —
                    Finalize writes it whenever N/times are present)
                  + encoded image + preview
Uploads           = any object over the 5 GiB single-PUT ceiling must go
                    multipart or the plan rejects; the step-scores
                    sidecar crosses it first at large N*times.
```

Output bytes are still not the whole cost — rasterization WORK is. Every
footprint candidate is processed before deduplication, so the true work
is approximately `sum(roots_s * footprint_area_s)`; with screen-space
point size this grows toward `N^4` for a near camera on a dense cloud.
Admission therefore also computes:

```text
estimated_footprint_updates = sum over sections of R_s * A_max
```

and rejects when it exceeds a budget calibrated from measured native
throughput against the 600-second native subprocess limit (the raster
handler's hard `timeout=600`), with named headroom. That limit spans
the WHOLE subprocess — startup, manifest downloads, root transforms,
solve-score execution, projection, and serialization — not only
footprint writes, so a constant from one friendly program is not a safe
budget. v1 COMMITS to one policy: a SINGLE conservative
effective-throughput constant (baseline-plus-footprint accounting is
the recorded v2 refinement, not an open fork). The constant is
calibrated from a MATRIX, not a run: scalar and RGB outputs, expensive
root and solve-score programs, source preludes, `A ~= 1`, and a
collision-heavy footprint — take the MINIMUM measured throughput,
subtract named startup headroom, apply the named deration. The staged
calibration in this section and Milestone 4 measure exactly that
matrix and record every cell's result alongside the chosen constant.

Calibration bootstrap (the deadlock is otherwise real: admission needs
the constant, and the benchmark that measures it must pass admission),
STAGED because local ARM64 throughput is not a defensible bound for
Lambda:

1. Seed a PROVISIONAL updates-per-second constant from the local/Docker
   ARM64 benchmark in Milestone 2, derated by a named factor and marked
   provisional in the plan output.
2. The provisional constant may admit ONLY a deliberately small
   production calibration run — its estimated updates capped at a named
   small fraction of the provisional budget — never representative
   full-size work.
3. Replace the constant with conservatively derated PRODUCTION
   telemetry from that run, recording both values and the deration in
   this document.
4. Only the production-derived constant admits representative full-size
   runs.

There is NO admission override: a payload flag is not "internal" merely
because the UI does not send it, and a case the budget rejects is
itself a result (it was genuinely at risk of the 600-second cap) —
record it as such. If a bypass ever becomes unavoidable it must be a
deployment-level environment gate or a separate unrouted alias, never a
request field.
- `total_fragment_bytes` is primarily a transfer and wall-time constraint,
  not resident memory once streaming is implemented. Report it and enforce
  a benchmark-derived wall-time policy rather than pretending it is free.

For scale: a fully occupied RGB camera fragment is `11 * N * N` bytes. At
`N=32768` that is exactly 11 GiB, already larger than the Raster Lambda's
10 GiB `/tmp` before safety headroom. The general ViewRender maximum is
therefore not automatically feasible for every camera render. Rejecting
such a request from calculated bytes is correct; silently lowering N or
adding an unexplained megapixel cap is not.

Do not add an unexplained megapixel constant. If a requested `N` cannot
fit, the plan must either:

- fit/mmap the dense planes and fragment within the declared memory and
  ephemeral budgets; or
- reject with the calculated byte requirement and named limiting
  resource.

The existing `N <= 32768` renderer bound remains an outer bound, not a
promise that every camera payload fits. ViewSnap must not silently lower N
or use the saved viewer lattice.

## 8. Frontend implementation

### 8.1 `sculpture.html`

Add:

- protocol version constant;
- a saved-meta bootstrap input such as
  `#meta=<encoded-public-meta-json-url>&embed=1`;
- ready message after `window.__sculptureViewer` is fully initialized;
- snapshot request listener;
- square framing guide;
- snapshot schema builder;
- finite-number and supported-state validation;
- exact matrix capture;
- response scoped to request source/origin.

Retain `cfg` identity from `meta.json` (`id`, `job_id`,
`source_artifact_id`, and `kind`) instead of dropping it during
`bootConfig()`. These values are useful for response diagnostics, but the
parent inventory remains authoritative.

For the saved-meta bootstrap:

- fetch the selected public `meta.json` with `cache: "no-store"`;
- resolve `roots_key` and `palette_key` relative to that metadata object's
  directory, not relative to the app's `sculpture.html`;
- require public HTTP(S) URLs and reject malformed configuration;
- apply the saved `view` exactly as the frozen viewer does.
- use `embed=1` to expose the controls in the catalogue; frozen public
  shares retain their current clean initial presentation.

### 8.2 `js/11-artifacts.js`

Add one identity-scoped controller, not loose globals:

```text
_sculptureSnapState = {
    selectedId,
    iframeGeneration,
    ready,
    pendingRequestId,
    pendingTimer
}
```

Required helpers:

- `_sculptureSnapFrame()` returns the currently rendered iframe and exact
  selected row.
- `_sculptureCatalogueViewerUrl(meta)` builds the current packaged viewer
  URL with `_publicStorageUrl(meta.prefix + "meta.json")` as input. It does
  not return `meta.viewer_url`.
- `_sculptureSnapReceiveMessage(event)` validates source, origin, version,
  request id, row id, and generation.
- `_requestSculptureSnapshot(meta, iframe)` returns a bounded Promise.
- `_resolveViewSnapSource(meta)` fetches `/render-summary` for
  `meta.job_id` and finds the exact `meta.source_artifact_id`.
- `_viewSnapParamsFromArtifact({jobId, calc, artifact}, snapshot,
  sculptureMeta)` extends the existing `_viewRenderParamsFromArtifact`
  contract without reading current Color selection — the resolved calc
  travels IN the signature so the implementation cannot fall back to
  `_viewRenderGridN`'s global-summary lookup.
- `_dispatchViewSnap(meta, snapshot)` is a data-parameterized command.

On selection change, iframe replacement, family change, deletion, or
refresh:

- increment generation;
- clear readiness;
- reject/cancel any pending snapshot request;
- disable Snap;
- ignore late responses.

Do not implement Snap as a button handler that reads `_selectedJobId` or
the current Color row halfway through an async sequence.

The existing `Open` and `Copy Link` actions continue using the saved
row's frozen public `viewer_url`. Only the in-app catalogue iframe uses the
current packaged viewer.

### 8.3 Source resolution

`_resolveViewSnapSource` should:

1. use the selected Sculpture's `job_id`;
2. call the existing `/render-summary`;
3. locate the exact Color artifact by `source_artifact_id`;
4. reject missing, unreadable, or ambiguous matches;
5. return `{jobId, calc, artifact}` — the summary's OWN calc, not a
   global;
6. derive ViewRender provenance with N taken EXPLICITLY from that
   returned `calc.N`;
7. add the camera snapshot and source Sculpture id.

`_viewRenderGridN` is NOT usable here: it reads
`window._lastRenderSummary.calc` before the artifact — the currently
loaded Results job — so a Snap of Sculpture job B while job A is on
screen would render at job A's N. `_viewSnapParamsFromArtifact` takes
the resolved calc as an argument and never consults the global summary.
Pin it: an E2E case with current job A and Sculpture source job B
having DIFFERENT N asserts the dispatch carries B's N.

This allows Snap to work even when another Color row is selected, and
prevents the classic stale-selection regression.

## 9. Render workflow plumbing

### 9.1 `handler_render_plan.py`

Add `camera` to the View projection allowlist.

Add a default `view_camera={}` so Step Functions JSONPaths always exist.
Rules:

- `view_projection="camera"` requires a valid version-1 camera object.
- Fixed projections reject a non-empty camera object.
- Camera mode still requires `source_color_artifact_id`.
- Camera mode remains family `views` and forces `pix=calc.N`.
- Camera mode plots pass 0 only (`global_step < N*N`), matching the
  viewer; the step-scores sidecar keeps its standard full semantics.
- `view_camera.vertical` is the ONLY vertical authority in camera mode:
  the plan derives the workflow's normalized `view_vertical` from it
  server-side (the raster handler keeps consuming `view_vertical`
  unchanged), and a request that carries BOTH with different values is
  rejected — two authorities rendering t1 while metadata says t2 is the
  exact mismatch class this plan exists to prevent.
  IMPLEMENTATION TRAP: the plan's defaults loop fills
  `view_vertical="t2"` for EVERY request before validation
  (`fused_params[key] = rp.get(key, default)`), so presence must be
  captured FIRST — `"view_vertical" in rp` — or an omitted value
  falsely conflicts with `view_camera.vertical="t1"`. Pin four cases:
  omitted (derived, no conflict), matching, mismatching (rejected), and
  fixed-view requests untouched.
- `plan_params_digest` gains canonical projection identity: the digest
  today covers viewport/pix/transforms/score settings but no projection,
  vertical, or camera — two different camera outputs (or a camera and a
  fixed view) could advertise the same raw-sidecar digest. Include
  `view_projection`, `view_vertical`, and a canonical hash of the
  camera's EXECUTION SUBSET whenever the projection is not `plan`: the
  matrices, vertical, slices, effective t-window, point size/scale/clamp
  fractions, and frame — serialized with sorted keys and the codebase's
  frozen `canonical_number_g17` policy (`.17g` round-trip formatting
  plus signed-zero folding, exactly as `program_source_core` documents
  for fingerprint hashing), and EXCLUDING `debug` (it is declared
  non-authoritative; hashing it would give equivalent renders different
  sidecar identities). Digest tests: debug-only changes do not move the
  digest; adjacent binary64 values do; `-0.0` hashes equal to `0.0`.
- Validate matrices, enums, effective t-window, style, and point size.
- Calculate camera depth-buffer resource requirements.
- Store compact canonical camera JSON and `source_sculpture_id` in the
  Views `meta.json`.
- Keep camera JSON out of S3 image user-metadata. It must not be added to
  `COLOR_IMAGE_HEADER_KEYS`; assert the image PUT metadata remains below
  `S3_USER_METADATA_LIMIT_BYTES`.

Do not let the frontend send an alternate viewport or N through the
camera payload.

### 9.2 `workflow_contracts.py`

Thread the validated camera object and the fragment contract through the
authoritative contracts:

```text
RENDER_COLOR_RASTER_ITEM_SELECTOR:
  "view_camera.$": "$.plan.params.view_camera"
  "fragment_contract.$": "$.plan.fragment_contract"

RENDER_FINALIZE_MT_FRAGMENT_MANIFEST:
  "pair_encoding.$": "$.plan.fragment_contract.encoding"
  "fragment_encoding.$": "$.plan.fragment_contract.encoding"
  "channels.$": "$.plan.fragment_contract.channels"
  "record_size_bytes.$": "$.plan.fragment_contract.record_size_bytes"
```

Both encoding aliases come from the ONE contract (finalize gives
`pair_encoding` precedence — leaving it on the clip contract would
override the rewired field), and `channels` moves with them.

The plan emits `fragment_contract` for EVERY run (ASL `.$` selectors
hard-fail on absent fields): existing projections emit their current
encoding values, so legacy behavior is bit-identical while the authority
moves to one place.

Update workflow-definition tests. Do not hand-edit only the generated
state-machine template selector; `workflow_contracts.py` is authoritative.

### 9.3 `handler_raster_mt.py`

Validate the camera object again at the Lambda boundary, and validate
`fragment_contract` there too, BEFORE invoking native code: the encoding
must match the projection kind (camera <-> the depth encoding, fixed
projections <-> the legacy encodings), `channels` must equal the
solve-score channel count, and `record_size_bytes` must equal the
encoding's computed size. A mismatch is a hard request failure, not a
native-side surprise.

Convert its two matrices and scalar fields into explicit native argv
values. Since `subprocess.run` already receives an argv array, comma-
separated finite matrix values are acceptable and do not require shell
quoting. Keep parsing centralized; do not add 32 unrelated top-level
request fields.

Pass:

```text
--view_projection=camera
--view_model_view=<16 comma-separated doubles>
--view_projection_matrix=<16 comma-separated doubles>
--view_vertical=t1|t2
--view_slices=<int>
--view_effective_tlo=<double>
--view_effective_thi=<double>
--view_point_world_size=<double>
--view_point_scale=<double>
--view_point_min_fraction=<double>
--view_point_max_fraction=<double>
```

The three captured point-scale values MUST reach the native CLI —
without them the renderer cannot reproduce portrait scaling or GPU
clamping, and admission would price work the raster cannot perform.
Pin the exact handler-to-native propagation: a raster handler test
asserts all three flags appear in the argv with the schema's values.

### 9.4 Native and finalizer

Modify:

- `lambda/roots2pix_mt.c`
- `lambda/view_camera_projection.h` — projection math, included ONLY by
  `roots2pix_mt`
- `lambda/camera_fragment_format.h` — the record layout, included by
  BOTH `roots2pix_mt` and `assemble_greyscale`
- `lambda/assemble_greyscale.c`
- `lambda/handler_finalize_mt.py`

Freshness-pin each binary only to headers it actually includes: the
projection header staling `assemble_greyscale` (or the record header
missing from either binary's inputs) is false coupling that either
forces pointless rebuilds or, worse, ships a stale assembler after a
record change. Rebuild through the normal Docker ARM64 path.

### 9.5a Color parity contract

This is a contract decision, not an implementation nit — and the v1
decision is the LEAN one: camera Finalize keeps the NORMAL
projected-output equalization, and the product honestly disclaims exact
viewer-color parity (the Views summary and modal hint say colors are
equalized over the snap's own output).

Why the tempting alternative is rejected for v1, in full:

- "Equalize over full-N pass-0 scores" is NOT the viewer's population:
  Sculpture generation deterministically SUBSAMPLES to the saved
  `grid_n` lattice before histogramming
  (`_subsample_step_scores_pass0` -> `histogram_from_raw_path_channel0`
  over `view_n^2`). Full-N similarity to that subsample is an
  assumption that structured or chaotic score fields can break — close
  is not exact, and the plan must not claim otherwise.
- It silently breaks recolor/repalette consistency: the raw sidecar
  records the PROJECTED-OUTPUT histogram (`build_raw_sidecar` in
  Finalize), and recolor consumes exactly that sidecar histogram
  (`color_recolor_raw._histogram_for_sidecar`). Rendering with one
  histogram while the sidecar advertises another means the first
  repalette CHANGES the artifact's transfer function.
- Scope note: this applies to scalar-LUT output only; direct
  three-channel modes never use the equalization LUT and are unaffected.

Exact parity is the recorded v2 extension, not a v1 compromise:
reproduce the saved `grid_n` subsample's histogram, carry an explicit
`equalization_histogram` + source contract in the sidecar, and extend
recolor to honor it — the render and every later repalette then share
one declared transfer function. Until that contract exists end to end,
the lean path is the only one that is both honest and
infrastructure-neutral.

Fixture for v1: a heavy-occlusion synthetic pins that the camera LUT
follows the projected-output histogram (identical to fixed-view
behavior), and the disclaimer text is asserted in the UI.

### 9.5 Metadata and Views display

Persist:

- `projection: "camera"`
- `source_artifact_id`
- `source_sculpture_id`
- `vertical`
- `camera_snapshot_version`
- compact camera snapshot JSON
- `view_style: "solid_points"`
- normal `lattice_n`, image, preview, palette, score, and render metadata

Update the Views summary to say, for example:

```text
camera snap | t2 | sculpture scu_... | color color_...
```

The existing Download, Delete, arrows, GoColor, DeepZoom, and completion
selection should continue to work without camera-specific copies.

## 10. Catalogue compatibility without migration

Do not overwrite saved `sculptures/{id}/viewer.html` objects.

They are immutable-cached public share artifacts. A previous visitor may
retain an old copy for a year regardless of an S3 overwrite, and public
shares do not need Snap. Mutating every published prefix would add an
operational migration without improving the in-app feature.

Use two deliberate viewer roles:

```text
Public Open / Copy Link
    -> saved sculptures/{id}/viewer.html
    -> frozen share experience

In-app catalogue preview
    -> current app-packaged sculpture.html
    -> selected public meta.json URL
    -> current controls and ViewSnap protocol
```

Required compatibility checks:

1. A pre-feature full Sculpture row loads through the packaged viewer.
2. Its metadata-relative `roots.bin` and `palette.png` resolve correctly.
3. Its saved `view` controls are applied.
4. It announces the current Snap protocol.
5. Open and Copy Link still use the untouched frozen share URL.
6. `kind="splatbake"` remains on its existing viewer and cannot Snap.

This deliberately favors a current, fixed catalogue preview over exact
reproduction of the frozen viewer implementation. The saved data and view
settings remain authoritative; only the viewer code is current. Do not use
same-origin frame introspection as a fallback.

## 11. Failure behavior

Every failure must be explicit and non-destructive:

- viewer not ready: button remains disabled;
- snapshot timeout: no dispatch;
- stale response: ignored;
- unsupported visual state: no dispatch, list unsupported controls;
- source Color artifact missing: no dispatch;
- source job/calc missing: no dispatch;
- active render: Snap disabled; the orchestrator's 409 remains the
  fail-closed backend guard;
- invalid camera schema: plan-time failure before raster fan-out;
- resource estimate too large: calculated error before fan-out;
- malformed native matrix/depth record: task fails loudly;
- partial Views inventory refresh: preserve existing partial/error
  semantics.

No failure should mutate the saved Sculpture.

## 12. Tests

### 12.1 Viewer Playwright

Extend `tests/e2e/sculpture-viewer-smoke.spec.js`:

- ready message is emitted only after viewer initialization;
- request returns two 16-element finite matrices;
- changing camera changes the matrix;
- changing Height changes model-view but does not mutate camera;
- t1/t2, slices, z-range, and point size are captured;
- z-range capture includes the exact `0.005 / height` effective margin;
- Height zero captures the full effective t-window;
- slice quantization occurs before effective-window filtering;
- landscape and portrait square-crop matrices match hand calculations;
- unsupported style/topology is reported;
- request id and Sculpture id round-trip exactly;
- malformed/wrong-version messages are ignored.

### 12.2 App E2E

Extend `tests/e2e/render-solve-score.spec.js`:

- Snap disabled until exact iframe ready;
- selected full row enables Snap;
- baked splat row keeps it disabled;
- active render keeps Snap disabled;
- a pre-feature full row loads through the packaged viewer while Open and
  Copy Link retain its frozen share URL;
- stale ready/snapshot messages from a replaced iframe do nothing;
- source is resolved by Sculpture `job_id + source_artifact_id`, not
  current Color selection;
- missing source gives an error and zero dispatches;
- exact dispatch payload carries `view_projection="camera"`,
  `view_camera`, source Color id, source Sculpture id, and `pix=N`;
- button feedback survives panel rerender;
- completion refreshes/selects the resulting Views row.

Add frontend source/runtime pins to `tests/test_frontend_js.sh` only for
load-bearing contracts that cannot be better asserted behaviorally.

### 12.3 Plan/workflow tests

Extend:

- `tests/test_render_plan.py`
- render workflow-definition tests
- `tests/test_raster_mt.py`
- `tests/test_finalize_mt_handler.py`
- `tests/test_storage_handler.py`

Pin:

- valid camera plan;
- invalid/missing/non-finite matrices;
- fixed projection plus camera payload rejection;
- camera projection without source identity rejection;
- `pix` remains calc N;
- ASL selector carries the camera object;
- the fragment contract is the ONLY encoding authority: `pair_encoding`,
  `fragment_encoding`, `channels`, and `record_size_bytes` all resolve
  from `$.plan.fragment_contract.*`, NO encoding field remains sourced
  from `$.solve_score_clip.parsed.*`, and both raster and finalize
  reject a contract inconsistent with the projection or the score
  channel count;
- metadata survives to the Views row;
- camera JSON is present in the Views `meta.json`, absent from the image
  PUT's user-metadata, and the image metadata stays below the 2 KiB limit;
- admission names Raster memory, Raster `/tmp`, Finalize memory, or
  estimated transfer/wall time when rejecting a camera render.

### 12.4 Native parity

Extend:

- `tests/test_raster_mt_parity.py`
- `tests/test_assemble_greyscale.py`
- `tests/docker_runtime_regression.py`

Use hand-computed fixtures for:

- center point;
- left/right/top/bottom clip-space mapping;
- point behind camera;
- near/far clipping;
- t1 and t2;
- slices;
- effective t-range, including a sliced boundary plate at small Height;
- two roots on one pixel at different depths in one native section;
- nearer root in a higher-ordinal fragment beating farther root in a
  lower-ordinal fragment;
- accepted section-local equal-depth lock-order behavior;
- deterministic cross-section equal-depth fragment-ordinal behavior;
- RGB/multi-channel score bytes following the winning depth;
- malformed/non-finite depth rejection;
- a streamed fragment split in the middle of every record field;
- a large synthetic stream proving no complete-fragment allocation;
- ARM64 binary parity.

Add a browser-vs-native fixture: capture a known Three.js camera and a
small deterministic point set, then assert native pixel centers and point
footprints against the viewer oracle.

### 12.5 Packaging/gating

Update:

- binary freshness dependencies;
- deploy packaging assertions for the new header;
- Docker runtime invocation list if a new test entry point is added.

The principal existing tests are already predeploy-gated:

- render plan;
- raster handler;
- finalize;
- storage;
- assembler;
- frontend harness;
- `render-solve-score.spec.js`;
- Docker runtime regressions through deploy.

Run the full predeploy gate and Docker ARM64 runtime before deployment.

## 13. Telemetry and benchmarks

Add camera-specific native/finalize counters:

- candidate roots;
- roots behind camera;
- clip-space rejects;
- z-range rejects;
- projected points;
- footprint pixel candidates;
- section-local depth replacements;
- occupied camera pixels;
- depth-fragment bytes;
- maximum single fragment bytes;
- estimated versus actual fragment occupancy;
- total camera fragment transfer bytes;
- final cross-section depth replacements;
- depth-buffer bytes;
- peak camera-stream carry-buffer bytes;
- Raster `/tmp` high-water estimate and actual fragment size;
- native camera projection microseconds;
- final depth-merge microseconds.

Benchmark:

1. a small deterministic oracle;
2. a collision-heavy synthetic sculpture;
3. at least one real production Sculpture at its actual N;
4. the same source as fixed isometric ViewRender for cost comparison.

Record wall time, Lambda max memory, `/tmp` use, fragment bytes, and billed
GB-seconds. Do not guess a lower N limit from a laptop benchmark.

The depth prototype has a go/no-go gate before frontend integration:

- run one collision-heavy synthetic case;
- run one representative production Sculpture at its actual N;
- confirm the native raster fits the 600-second SUBPROCESS limit
  (`handler_raster_mt` runs the binary with `timeout=600` — that, not
  the 900-second Lambda envelope, is the raster ceiling);
- confirm sequential Finalize streaming fits its 900-second envelope AND
  its manifest URLs: `build_native_manifest_urls` presigns for 900
  seconds by default, so a long merge must regenerate URLs or request an
  expiry sized to the measured merge time before it starts.

The pass/fail thresholds are DEFINED AND RECORDED when the benchmark runs
— wall-time headroom against the 900-second envelope, peak memory against
the deployed allocation, fragment bytes against the calculated admission
estimate — and written into this document before any frontend work
starts. The gate must not become a judgment call made under sunk-cost
pressure after the native work exists; a number chosen in advance of the
frontend is the whole point. (Consistent with this plan's own rule: no
invented constants — the benchmark supplies them, then they bind.)

If representative production fragments cannot finish inside the recorded
thresholds, stop the v1 implementation. Do not grow it into a second
spatial-shuffle or tile-routing workflow under the name of a small Snap
feature. That would be a separately designed v2.

## 14. Implementation order

### Milestone 0: freeze contracts

- Add snapshot-schema validation tests.
- Add camera matrix and square-crop math tests.
- Pin effective t-window and Height-zero behavior.
- Pin memory, `/tmp`, and fragment-byte admission formulae.
- Add depth-fragment assembler tests before changing the producer.

### Milestone 1: viewer protocol

- Implement ready/request/response in `sculpture.html`.
- Add the public saved-meta bootstrap for catalogue previews.
- Implement square framing and supported-state checks.
- Land Playwright coverage.
- Do not expose the app Snap button yet.

### Milestone 2: depth-aware backend (local/native correctness only)

- Add camera matrix projection.
- Add section-local depth ownership.
- Add depth fragment encoding.
- Add final cross-section depth merge.
- Add host and Docker ARM64 parity tests.
- Benchmark LOCALLY for correctness and rough throughput only — the
  production go/no-go cannot run yet: it needs Milestone 3's plumbing to
  invoke the deployed workflow at all.

Existing projections must remain byte-identical under their existing
tests.

### Milestone 3: workflow plumbing

- Thread `view_camera` and `fragment_contract` through plan,
  authoritative workflow contracts, raster handler, finalizer metadata,
  and Views listing.
- Keep mode `color`, family `views`, and output `N x N`.
- Run full render workflow tests.

### Milestone 4: internal production go/no-go

- Invoke the deployed workflow directly (no UI), in calibration order:
  first the deliberately SMALL production calibration run admitted under
  the provisional constant, then — with the production-derived constant
  recorded — the synthetic and the representative full-size Sculpture.
- Record the thresholds defined in section 13 into this document.
- Record the provisional constant, the production telemetry, the
  deration factor, and the final constant.
- STOP here if the recorded thresholds fail — frontend work does not
  start on a backend that cannot meet them.

### Milestone 5: app integration

- Add identity-scoped iframe controller and Snap button.
- Load full catalogue rows through the current packaged viewer while
  preserving frozen Open/Copy Link URLs.
- Resolve exact source Color metadata.
- Dispatch through the existing render orchestrator.
- Add E2E stale-selection and exact-payload tests.

### Milestone 6: production verification

- Deploy.
- Run one modest-N and one representative high-N snap.
- Inspect camera telemetry, memory, fragment size, output framing, color,
  and depth correctness.
- Verify a pre-feature full save works in the catalogue without modifying
  its S3 prefix.
- Only then decide whether a separately scoped v2 needs a different
  fragment/shuffle architecture.

## 15. Acceptance criteria

The feature is complete when:

- a selected full Sculpture can be navigated and snapped from the action
  row;
- the target square is visible before dispatch;
- the result uses the original calculation's N, not the saved viewer grid;
- the image camera framing matches the square guide;
- nearer solid points occlude farther points across native threads and
  raster sections;
- source Color provenance and source Sculpture identity are recorded;
- camera JSON exists only in the Views sidecar metadata and cannot exhaust
  the S3 image-header budget;
- the result appears as a normal Views artifact;
- Download, Delete, arrows, GoColor, DeepZoom, and completion selection
  work unchanged;
- stale iframe messages cannot dispatch against a successor selection;
- unsupported viewer styles fail explicitly rather than silently changing
  appearance;
- pre-feature full Sculpture catalogue entries work through the packaged
  viewer without modifying their saved prefixes or frozen share links;
- Raster fragment `/tmp`, dense-plane memory, and total-fragment transfer
  are calculated and reported before dispatch;
- Finalize parses camera fragments with bounded streaming memory;
- all existing fixed ViewRender and ColorRender outputs retain their
  current contracts;
- focused tests, frontend harness, Playwright, Docker ARM64 runtime, and
  the full predeploy gate are green.

## 16. Future extensions, not v1

- cloud/additive camera renders with float RGB accumulation and tone map;
- ghost alpha blending with an explicit order-independent-transparency
  policy;
- ribbons, threads, and cluster topology rendered natively;
- baked-splat camera snapshots using the splat renderer's own geometry;
- cube/frame overlays;
- rectangular output preserving the live iframe aspect;
- named camera bookmarks and multiple snaps from one Sculpture;
- optional retirement of per-save viewer copies for newly created public
  shares, designed as a separate compatibility change.
