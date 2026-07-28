# SnapRender: hi-res camera reprojection

STATUS: this describes the SHIPPED system (2026-07-28). It supersedes every
earlier draft of this document. In particular, these earlier-draft ideas were
DELIBERATELY DROPPED during implementation and must not be resurrected from
old copies, reviews, or memory:

- excluding baked splat viewers from Snap (both viewer kinds snap);
- point-footprint expansion and its admission pricing (one root = one pixel);
- timing calibration (artifact, matrix, modes, throughput constants —
  admission is deterministic-resources-only; duration is telemetry);
- style/show/point-size eligibility rules (a camera is a camera);
- rerunning the score VM (stored score bytes are reprojected).

## 1. Product contract

`SnapRender` is a ViewRender operation. The Sculpture viewer is only a camera
controller — and EVERY saved viewer kind is one: the full sculpture viewer
AND the baked splat viewer both implement the same snapshot protocol
(ready announcement + snapshot request/response). Snapping a baked view is
a primary use case, not an exception: the light baked page is where a
vantage gets found, and SnapRender renders the SOURCE artifact's full-res
root cloud from that vantage. Baked pages are frozen per save, so protocol
capability rides the template at bake time; pages baked before the
protocol existed cannot answer and simply leave SnapRender disabled.

The viewer contributes:

- the model-view matrix;
- the perspective projection matrix;
- the selected vertical parameter axis (`t1` or `t2`);
- the active slice count and effective t-range;
- the square output-frame contract.

It does not contribute a WebGL drawing primitive. Point size, splat size,
style, opacity, ribbons, threads, clusters, and browser/GPU point limits have
no meaning in the hi-res renderer.

The output is an ordinary `views` artifact at the source calculation's
`N x N` resolution.

## 2. Raster semantics

For every saved solve step and every root in that step:

1. Read the root row from the existing calculation chunks.
2. Apply the source Color artifact's saved root transforms.
3. Recover the step's already-computed score byte or bytes from the source
   Color artifact's immutable `step_scores.raw`.
4. Recover `t1`/`t2` from the global serpentine step index. Later passes use
   the same parameter lattice through `global_step % (N*N)`.
5. Map the normalized root and selected t coordinate through the captured
   camera matrices.
6. Reject points outside the effective t-range, behind the camera, or outside
   clip space.
7. Quantize the projected position with `floor(x), floor(y)`.
8. Emit at most one candidate pixel for that root.
9. If another root reaches the same pixel, retain the candidate with the
   nearest positive float32 camera depth.

There is no multi-pixel root expansion and no attempt to reproduce a WebGL
sprite.

## 3. Score authority

The selected Color artifact is authoritative for color.

Required v3 raw-sidecar fields:

- `step_scores_key`;
- `step_count`;
- `step_scores_grid_n`;
- output channel count and interpretation;
- palette, background, format, quality, and viewport provenance.

The planner verifies:

- `step_scores_grid_n == calc.N`;
- `step_count == N*N*times`;
- object length is exactly `step_count * channels`;
- channels are one or three;
- all calculation chunks have complete step metadata;
- chunk step counts total `N*N*times`.

Each Raster Map item range-reads exactly:

```text
byte_start = step_start * channels
byte_count = step_count * channels
```

The solve-score program is not executed, recompiled, or required to parse.
Its source, chain, wire spec, and fingerprint are provenance only. This is
important for old Color artifacts: a historical program syntax change must
not make valid stored score bytes unusable.

Finalize references the original `step_scores.raw` in the new View sidecar.
It does not concatenate, copy, or upload a duplicate score object.

## 4. Parallel workflow

SnapRender uses the existing render Step Functions workflow.

Planner:

- resolves the selected Color artifact;
- fixes output resolution to `calc.N`;
- builds logical root ranges across all passes;
- attaches the stored-score contract;
- chooses enough Map sections to satisfy deterministic per-worker resource
  limits.

Workflow:

- skips the solve-score clipping Lambda for stored-score views;
- fans Raster Map items out through the existing Raster-MT Lambda;
- invokes Finalize-MT after all fragments are available.

Raster worker:

- downloads its root spans;
- downloads its exact score range;
- projects all roots;
- maintains one dense per-section depth plane and score plane;
- uploads one depth-bearing fragment.

Finalize:

- streams section fragments;
- resolves cross-section collisions by nearest depth;
- writes the normal raw view, image, preview, metadata, and raw sidecar;
- points the sidecar back to the source score object.

## 5. Fragment contract

Camera fragments use:

```text
u32 little-endian pixel index
float32 little-endian positive camera depth
1 or 3 score bytes
```

Encoding:

```text
u32le_f32depth_u8_channels_v1
```

Record size is `8 + channels`.

The fragment contract is generated by `view_camera.fragment_contract()` and
validated independently by Raster and Finalize. A camera render must not
accept a legacy depthless fragment.

## 6. Camera protocol

Current snapshot payload:

```json
{
  "version": 1,
  "projection": "perspective",
  "matrix_layout": "column_major",
  "model_view_matrix": [16],
  "projection_matrix": [16],
  "vertical": "t1|t2",
  "slices": 0,
  "effective_tlo": 0.0,
  "effective_thi": 1.0,
  "frame": {
    "aspect": 1.0,
    "crop": "center_square"
  }
}
```

Allowed slice counts remain registry-pinned.

Old immutable viewer pages may also send point-size, style, show, and
`debug.point_control` fields. The backend accepts and discards those fields
for compatibility. They are absent from normalized camera data, execution
identity, and new viewer snapshots.

`style` and `show` are intentionally not eligibility conditions. A camera
captured while splats, ribbons, or hidden points are displayed is still a
valid camera.

## 7. UI

The Sculpture tab exposes one action: `SnapRender`.

Enablement requires only:

- a selected Sculpture row — full save OR baked splat, either kind;
- a loaded viewer capable of returning a camera snapshot;
- no active render dispatch.

The action:

1. resolves the Sculpture's source Color artifact;
2. requests the camera snapshot;
3. dispatches `view_projection=camera`;
4. logs that stored scores are being reprojected;
5. returns the completed artifact through the existing Views inventory.

There is no Flat/Depth selector. SnapRender always uses nearest-depth
ownership. Fixed front/rear/left/right/radial/isometric ViewRender modes also
reuse the source artifact's stored scores and do not rerun the score VM.

## 8. Admission

Admission protects only deterministic resources:

- root bytes for the section;
- downloaded score bytes for the section;
- `4*N*N` bytes for the float32 depth plane;
- `channels*N*N` bytes for the score plane;
- occupancy bits and stream carry;
- worst-case fragment bytes;
- output raw/encode `/tmp` bounds.

One root can occupy at most one pixel, so:

```text
candidate_roots = section_steps * degree
occupied_pixels_upper = min(N*N, candidate_roots)
fragment_bytes_upper = occupied_pixels_upper * (8 + channels)
```

The planner increases logical section count until every Raster section fits
the deployed Lambda memory and `/tmp` limits. Finalize memory and `/tmp` are
checked separately.

Runtime duration is telemetry only. There is no timing calibration artifact,
calibration mode, calibration deployment command, throughput allowlist, or
timing-based admission rejection.

Useful telemetry includes:

- candidate roots;
- projected roots;
- behind-camera, clip, range, and invalid rejects;
- one-pixel candidates;
- depth replacements;
- occupied pixels and fragment bytes;
- score range-download bytes and duration;
- handler, download, native, and upload wall times.

## 9. Identity and metadata

Camera execution identity includes only rendering geometry:

- camera schema version;
- matrix layout;
- model-view and projection matrices;
- vertical axis;
- slices;
- effective t-range;
- square-frame contract.

Viewer appearance and debug fields are excluded.

View metadata records:

- source Color artifact id;
- source Sculpture id for camera views;
- camera snapshot;
- `camera_rasterization=one_root_one_pixel_nearest_depth`;
- source score object and sidecar keys;
- source color/palette/format/root-transform provenance.

The full camera JSON belongs in the View `meta.json` overlay. It must not be
forced into S3 user metadata beyond the existing metadata budget.

## 10. Failure rules

Fail before fan-out when:

- source Color identity does not match;
- the raw sidecar or score object is missing;
- score grid/count/channels/size do not match the calculation;
- chunk step coverage is incomplete;
- camera matrices or frame semantics are invalid;
- deterministic memory or `/tmp` limits cannot be met.

Fail a Raster section when:

- its S3 score range is short or oversized;
- root or score offsets escape the admitted section;
- native fragment telemetry exceeds the planner's hard bound;
- a depth-bearing fragment contains a non-positive or non-finite depth.

Do not fail because:

- viewer point size is unsupported;
- points are hidden in WebGL;
- splats/ribbons/threads/clusters are enabled;
- a timing calibration has not been run;
- historical solve-score source no longer recompiles.

## 11. Verification gates

Required tests:

- browser snapshot captures camera geometry and omits appearance/point fields;
- old snapshot fields are accepted but ignored and do not change the hash;
- native projection matches the browser camera oracle;
- one root produces at most one candidate pixel;
- nearer depth wins within and across sections;
- later passes are projected and use their own stored score bytes;
- stored-score Raster commands contain no score-program arguments;
- historical invalid score source remains renderable from valid stored bytes;
- exact S3 range math is pinned;
- Finalize references the source score key without copying it;
- deterministic memory and `/tmp` overflow fail closed;
- workflow skips the score-clipping Lambda for artifact-score views;
- fixed ViewRender projections use the same stored-score path;
- deploy packaging contains no calibration scripts or artifacts.

Run:

```bash
uv run python -m pytest \
  tests/test_view_camera.py \
  tests/test_view_snap_cost_model.py \
  tests/test_render_plan.py \
  tests/test_raster_mt.py \
  tests/test_raster_mt_parity.py \
  tests/test_finalize_mt_handler.py \
  tests/test_render_workflow_definition.py \
  tests/test_deploy_packaging.py

bash tests/test_frontend_js.sh
bash scripts/predeploy_check.sh
```

The native ARM64/Docker regression remains the deployment-level proof for
the packaged binary and depth-fragment merge.
