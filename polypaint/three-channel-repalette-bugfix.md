# Three-Channel Repalette Bug Fix

## Status

Implemented in the current working tree.

The `Color RePalette` action fails with the error
`Recolor-from-raw requires a scalar (channels=1) raw artifact; got channels=3`
when the source artifact was rendered with any 3-channel interpretation
(`rgb`, `hsv`, `rgb_lut`, or `hsv_lut`).

This document specifies the backend fix: extend the recolor-from-raw path so
3-channel artifacts can be repaletted by re-running the color interpretation
through `score_raw_render` with a different palette, while preserving the
existing scalar repalette behavior unchanged.

Implementation notes:

- `color_recolor_raw.py` now validates the raw sidecar without the scalar
  gate, dispatches by channel count, and requires `new_interpretation` to be
  `rgb_lut` or `hsv_lut` for 3-channel sources.
- The raw bytes are copied unchanged. `step_scores.raw` is also copied when
  present; three-channel step-score bytes are consumed by the fused
  ExtractPalette path through `step_scores_to_palette_raw`.
- Associated palette artifacts are regenerated through the same scalar or
  channel-LUT interpretation, so the displayed dense palette follows the new
  RePalette choice.
- `raw_score_render.py` omits `--eq_lut` when no equalization LUT is needed.
  The native `score_raw_render` binary already only requires `--eq_lut` for
  scalar LUT mode.
- The Color RePalette popup now shows an RGB LUT / HSV LUT selector for
  3-channel sources and sends `new_interpretation` in the dispatch payload.

## Symptom

Observed UI log:

```text
[7:34:55] Color RePalette: dispatching color_run_1777300402149_lbi0v6 -> tri_classic_gucci_green_lemon
[7:34:56] working · wall=0.0s
[7:34:59] Color RePalette failed: Recolor-from-raw requires a scalar (channels=1) raw artifact; got channels=3
```

The dispatch reaches the Color RePalette Lambda, validation runs almost
immediately, and the request is rejected before any S3 reads happen.

## Failure Path

1. Frontend dispatches Color RePalette to `polypaint-color-repalette`.
2. `handler_color_repalette.handler` (`lambda/handler_color_repalette.py:14`)
   delegates to `handle_color_recolor_from_raw_request` in
   `lambda/color_recolor_raw.py:394`.
3. That handler downloads the source color artifact head, finds
   `raw_key` / `raw_meta_key`, and calls `validate_raw_sidecar` at
   `lambda/color_recolor_raw.py:447-453` with `require_scalar=True`,
   `feature="Recolor-from-raw"`.
4. `validate_raw_sidecar` (`lambda/raw_sidecar.py:355-359`) calls
   `require_scalar_raw` (`lambda/color_render_contract.py:95`), which raises
   the observed error when the sidecar reports `channels != 1`.

## Why The Rejection Is Correct, Not A Regression

The rejection is intentional. It was added in the multicolor v2 ship to
prevent silent data corruption.

A scalar repalette flow does this:

```text
for each pixel:
    byte = raw[pixel]              # one byte per pixel
    out.rgb = palette[byte]         # 1D LUT into selected palette
```

If the source raw is `width * height * 3` bytes, the scalar code path would
treat it as `width * height` bytes wide, reading three real pixels per slot
and producing garbage at one third of the row width. The pre-v2 behavior was
exactly that silent corruption.

So the fix is not to remove the `require_scalar` gate. The fix is to add a
new code path that repaletted multi-channel artifacts correctly.

## What Multicolor V2 Already Has

The multicolor v2 ship added two channel-aware interpretations that are
exactly the operation needed here:

- `rgb_lut`: each channel of a packed RGB raw is independently looked up
  through one chosen 1D palette. Output is RGB.
- `hsv_lut`: each channel is looked up through the HSV-projection of one
  chosen 1D palette. Output is RGB after HSV-to-RGB conversion.

Native support exists in `lambda/score_raw_render.c`:

- Argument: `--interpretation=rgb_lut|hsv_lut`.
- Argument: `--palette=<name>`.
- Implementation: `convert_palette_component_lut_raw_to_image` at
  `score_raw_render.c:234`.
- Validation at `score_raw_render.c:464-471` already rejects mismatched
  channels and missing palette.

What is missing is the Python wiring in `color_recolor_raw.py` to:

- detect `raw_channels = 3` on the source artifact;
- accept a target interpretation (`rgb`, `hsv`, `rgb_lut`, `hsv_lut`);
- dispatch the channel-LUT path for `rgb_lut`/`hsv_lut`;
- re-emit the raw, image, preview, and sidecar metadata for the new artifact.

## Design Goals

- Recolor-from-raw stays scalar-only for `channels=1` and continues to
  reject silently-incompatible flows.
- A new explicit channel-LUT repalette path handles `channels=3` artifacts.
- The new path is a pure re-render: same compiled score program, same raw
  bytes, different palette/interpretation.
- The new path may select between `rgb_lut` and `hsv_lut` for any
  `channels=3` source. It does not mutate the source raw bytes.
- For interpretation `rgb` or `hsv` (no LUT), repalette is a no-op style
  change; the request must promote interpretation to `rgb_lut` or `hsv_lut`
  to make a palette meaningful.
- All artifact lineage and identity rules from multicolor v2 are preserved.

## Non-Goals

- This change does not introduce per-channel-different palettes (one palette
  applied to one channel each). That is the future "three-palette
  combine" mode that multicolor v2 explicitly deferred.
- This change does not change the saved-program format, the chain identity,
  or the render plan / finalize contract.
- This change does not introduce new S3 routes or Lambdas. Existing routes
  remain authoritative.
- This change does not allow downgrading a `channels=3` artifact to a
  `channels=1` palette artifact. The output keeps the same channel count
  and dimensions.

## New Backend Path

Add a `multichannel_repalette` derivation alongside the existing scalar
`color_repalette`. It is exposed through the same Color RePalette Lambda and
the same wire request, gated on the source artifact's channel count.

### Request Shape

The Color RePalette payload is extended with two optional fields:

```json
{
  "job_id": "...",
  "task_id": "...",
  "artifact_id": "...",
  "source_artifact_id": "...",
  "source_image_key": "...",
  "new_palette": "tri_classic_gucci_green_lemon",
  "new_interpretation": "rgb_lut"
}
```

- `new_interpretation` is required when the source artifact has
  `raw_channels = 3`.
- Allowed values for `channels=3` sources: `rgb_lut`, `hsv_lut`.
- Disallowed values for `channels=3` sources: `rgb`, `hsv`,
  `scalar_lut`. These would produce no visible change or a contract
  violation, and rejecting them gives a clearer error than guessing.
- `new_interpretation` is ignored when the source artifact has
  `raw_channels = 1`. The existing scalar path runs as today.

The frontend should pre-select an interpretation matching the source
artifact's stored interpretation. Saved programs may carry a
`recommended_interpretation` (multicolor v2). For a 3-channel source whose
artifact metadata says `interpretation = rgb`, the natural default is
`rgb_lut`. For `interpretation = hsv`, the natural default is `hsv_lut`.

### Validation

- `new_palette` must be in `VALID_PALETTE_NAMES`.
- Source artifact head must have `raw_key` and `raw_meta_key`.
- Source raw sidecar must validate. `raw_key` / `raw_meta_key` are the
  authoritative capability signal; the legacy `repalette_capable` metadata
  flag is not used for this gate.
- For `raw_channels = 3`:
  - `new_interpretation` is required.
  - `new_interpretation` must be `rgb_lut` or `hsv_lut`.
- For `raw_channels = 1`:
  - The existing scalar path runs.
  - `new_interpretation` if present must be unset, empty, or equal to
    `scalar_lut`. Anything else is rejected with a clear error.

### Implementation Steps

1. Read the source raw sidecar without `require_scalar=True`. Use a new
   variant that returns `channels` in the validated record.
2. If `channels == 1`: run today's scalar path unchanged.
3. If `channels == 3`:
   - Determine the target interpretation.
   - Download the source raw to `/tmp`.
   - Skip the equalization LUT step. The raw is already a packed
     `width * height * 3` byte image; eq LUT is meaningless for channel-LUT
     lookups. Preserve the sidecar histogram when it is valid. If it must be
     recomputed, compute the channel-0 histogram only to match
     `assemble_greyscale`'s `histogram_channel:0` contract.
   - Run `score_raw_render` with `--channels=3 --interpretation=<rgb_lut|hsv_lut>
    --palette=<new_palette>` and the original `width`, `quality`, and
     `background_color`. Dense final color raw must keep
     `zero_background=True`, because absent pixels are represented by the
     all-zero byte tuple and there is no separate occupancy mask.
   - Skip the eq-LUT-from-source argument. Pass a synthetic identity LUT or
     extend `score_raw_render` to make `--eq_lut` optional when interpretation
     is `rgb_lut` / `hsv_lut`. Recommended: make `--eq_lut` optional in
     C-side parsing for non-scalar interpretations.
   - Copy the source raw object byte-for-byte into the new artifact's
     `greyscale.raw` location. The raw bytes are the artifact; the change
     is purely in the chosen interpretation and palette, so reuse via a
     pure copy is correct.
   - Build the new raw sidecar with:
     - `channels = 3`
     - `raw_layout = u8_packed_channels_row_major`
     - `interpretation = <rgb_lut|hsv_lut>`
     - same `width`, `height`, `chain_fingerprint`, `score_chain`,
       `score_program`, `clip_slots`, `score_output_normalize`,
       `score_output_clip_lo/hi`, `output_channels`, and any existing
       `step_scores_key` / `step_count` / `step_scores_grid_n` metadata.
   - Build the artifact metadata with:
     - `raw_channels = "3"`
     - `score_output_channel_count = "3"`
     - `score_output_interpretation = <rgb_lut|hsv_lut>`
     - `color_interpretation = <rgb_lut|hsv_lut>`
     - `palette = <new_palette>`
     - `derivation_kind = color_repalette`
     - `repalette_capable = true`; raw sidecar keys remain the authoritative
       capability signal, but the metadata flag should not lie.
     - `rgb_source = "<rgb_lut|hsv_lut>_raw"`
     - `derived_from_artifact_id = source_artifact_id`
   - Upload the new image, preview, raw, sidecar, and metadata as today.
4. Preserve `step_scores_key` / `step_count` / `step_scores_grid_n` when the
   source sidecar has them. Three-channel step scores are packed solve-order
   bytes and remain extractable.

### Why Copy The Raw Instead Of Re-Render It

For a channel-LUT repalette, the output bytes that end up on disk are
exactly the source raw bytes; only the *interpretation* applied at display
time changes. Copying the raw object preserves byte identity, avoids a
download/upload round trip, and keeps the artifact lineage trivially
verifiable: the new raw key has the same content hash as the source raw
key.

This is the same pattern as today's scalar path, which copies the source
raw without modification (`color_recolor_raw.py:540-541`). The only thing
that changes per-render is the rendered image and preview that come out of
`score_raw_render`.

### Why Eq-LUT Is Skipped

The scalar path computes a histogram of the single-channel raw and writes
an equalization LUT, then `score_raw_render` applies palette LUT after
equalization. That makes sense for scalar palette mode where the output is
a 1D mapping from byte intensity to color.

Channel-LUT modes index each of the three raw channels into one chosen 1D
palette and combine the results either as RGB triplets or as HSV
components. There is no scalar histogram to equalize. Forcing an eq LUT
would either be a no-op (identity LUT) or distort the per-channel mapping
in a way the user did not request.

Recommended: make `--eq_lut` optional in `score_raw_render` when
`--interpretation` is `rgb_lut` or `hsv_lut`. The C-side already validates
required arguments per interpretation; this is a small extension.

### Cache Keys And Identity

- The new artifact gets a fresh `artifact_id` per the existing pattern.
- `chain_fingerprint` is preserved from the source raw sidecar.
- `score_program` is preserved.
- The artifact's metadata records the new palette and new interpretation.
- Two repalette runs of the same source artifact with the same
  `(new_palette, new_interpretation)` produce identical raw bytes and an
  equivalent rendered image. They do not need to dedupe automatically; the
  user explicitly requested the action. Existing scalar repalette also does
  not dedupe.

### Lineage And Re-Repalette

A `channels=3` artifact produced by this path keeps a valid raw sidecar and
may be repaletted again. The lineage chain is:

```text
fused color render (rgb / hsv / rgb_lut / hsv_lut)
  -> (this fix) channel-LUT repalette
    -> channel-LUT repalette
      -> ...
```

Each step preserves the source raw bytes and only changes the rendered
image and the recorded palette/interpretation. The chain length is
unbounded.

## File-By-File

### `lambda/color_recolor_raw.py`

- Split `handle_color_recolor_from_raw_request` into a dispatcher that
  reads channels from the source raw sidecar (without `require_scalar`)
  and routes to either:
  - `_handle_scalar_recolor_from_raw` (existing logic, hard-gated on
    `channels=1`).
  - `_handle_channel_lut_recolor_from_raw` (new).
- The new function:
  - Validates `new_interpretation` is in `{rgb_lut, hsv_lut}`.
  - Reuses `_phase`, `_download_key_to_path`, `_copy_raw_object`,
    `_copy_object`, `_apply_associated_palette_metadata`,
    `_metadata_size_bytes`, and the existing upload pattern.
  - Uses `_histogram_for_sidecar` only to preserve/recompute the channel-0
    sidecar histogram. It skips `write_equalization_lut`.
  - Calls `render_score_raw(..., channels=3, interpretation=<mode>,
    eq_lut_path=None or "")` and threads the existing `quality`,
    `background_color`, and `pix=width`.
  - Builds the new raw sidecar with `channels=3`, the new interpretation,
    and the appropriate `output_channels` array carried forward from the
    source sidecar.
- Drop `require_scalar=True` from the dispatcher's `validate_raw_sidecar`
  call so it accepts both `channels=1` and `channels=3`. Re-introduce the
  scalar gate inside `_handle_scalar_recolor_from_raw` so the scalar path
  still rejects `channels=3` defensively.

### `lambda/raw_sidecar.py`

No required changes. `validate_raw_sidecar` already returns `channels`,
`raw_layout`, and `interpretation` and accepts `channels=3` sidecars. The
existing `require_scalar=True` flag is still used by all four scalar-only
consumers (recolor scalar path, bilevel, ExtractPalette,
recolor-from-raw scalar branch). The new channel-LUT branch simply does
not pass `require_scalar=True`.

### `lambda/color_render_contract.py`

Add a small helper to centralize the "is this interpretation valid for
channel-LUT repalette?" check:

```python
CHANNEL_LUT_INTERPRETATIONS = {"rgb_lut", "hsv_lut"}


def validate_channel_lut_repalette(*, source_channels, new_interpretation):
    mode = normalize_color_interpretation(new_interpretation)
    if int(source_channels or 1) == 1:
        if mode != "scalar_lut":
            raise RuntimeError(
                f"Color RePalette on a scalar artifact requires interpretation=scalar_lut, got {new_interpretation!r}"
            )
        return mode
    if int(source_channels or 1) == 3:
        if mode not in CHANNEL_LUT_INTERPRETATIONS:
            raise RuntimeError(
                f"Color RePalette on a 3-channel artifact requires interpretation=rgb_lut or hsv_lut, got {new_interpretation!r}"
            )
        return mode
    raise RuntimeError(f"Color RePalette does not support channels={source_channels}")
```

### `lambda/raw_score_render.py`

- `render_score_raw` should accept `eq_lut_path=None` and emit
  `--eq_lut=` only when the path is non-empty. Today the wrapper always
  emits the flag. Adjust so channel-LUT calls can omit it.

### `lambda/score_raw_render.c`

- Make `--eq_lut` optional when `--interpretation` is `rgb_lut` or
  `hsv_lut`. Today the C-side hard-requires it for `channels=1` only.
  Confirm channel-LUT branches do not read the eq LUT. If they do, gate
  the read on interpretation.
- Rebuild the binary as part of this change. Native binary changes go
  through the docker runtime test gate.

### `lambda/handler_color_repalette.py`

No changes. It is a thin wrapper that forwards to
`handle_color_recolor_from_raw_request`. The dispatch logic moves into
`color_recolor_raw.py`.

### `lambda/handler_recolor_from_raw.py`

Same — a thin wrapper. No changes.

### `lambda/handler_storage.py`

No changes. The `/list-artifacts` path already exposes `raw_channels`,
`raw_key`, and `raw_meta_key` per-artifact. The frontend uses the raw sidecar
keys to gate the UI.

### `index.html`

- Color RePalette popup must:
  - Read `raw_channels` and `score_output_interpretation` from the source
    artifact metadata.
  - For `raw_channels = 1`: show only the palette dropdown. Submit
    payload includes `new_palette`, no `new_interpretation`.
  - For `raw_channels = 3`: show both the palette dropdown and an
    interpretation choice limited to `rgb_lut` and `hsv_lut`. Default to
    `rgb_lut` if the source interpretation was `rgb` or `rgb_lut`, and to
    `hsv_lut` if the source interpretation was `hsv` or `hsv_lut`. Submit
    payload includes both `new_palette` and `new_interpretation`.
  - Reject submission if no interpretation is selected for a 3-channel
    source.

## Proposed Code Changes

The diffs below are illustrative, not literal; line numbers will shift as the
surrounding files evolve. Treat them as the shape of the change. Each block
references the file in the file map above.

### `lambda/color_render_contract.py`

Add a small dispatcher used by both backend and frontend.

```python
# Append to lambda/color_render_contract.py

CHANNEL_LUT_INTERPRETATIONS = {"rgb_lut", "hsv_lut"}


def repalette_target_for_source(*, source_channels, requested_interpretation):
    """Validate and normalize the target interpretation for a Color RePalette.

    Returns the canonical target interpretation. Raises with a clear message
    when the request is incompatible with the source's channel count.
    """
    requested = "" if requested_interpretation in (None, "") else requested_interpretation
    channels = int(source_channels or 1)
    if channels == 1:
        mode = normalize_color_interpretation(requested or "scalar_lut")
        if mode != "scalar_lut":
            raise RuntimeError(
                "Color RePalette on a scalar (channels=1) artifact requires "
                f"interpretation=scalar_lut, got {requested_interpretation!r}"
            )
        return "scalar_lut"
    if channels == 3:
        if not requested:
            raise RuntimeError(
                "Color RePalette on a 3-channel artifact requires an explicit "
                "new_interpretation in {rgb_lut, hsv_lut}"
            )
        mode = normalize_color_interpretation(requested)
        if mode not in CHANNEL_LUT_INTERPRETATIONS:
            raise RuntimeError(
                "Color RePalette on a 3-channel artifact requires "
                f"interpretation=rgb_lut or hsv_lut, got {requested_interpretation!r}"
            )
        return mode
    raise RuntimeError(f"Color RePalette does not support channels={channels}")
```

### `lambda/raw_score_render.py`

Make `eq_lut_path` optional on the wrapper so channel-LUT calls do not pass
`--eq_lut`.

```python
# lambda/raw_score_render.py

def render_score_raw(
    *,
    raw_path,
    out_path,
    preview_path,
    pix,
    eq_lut_path,        # may be None or "" for channel-LUT modes
    palette,
    background_color,
    quality,
    channels=1,
    interpretation=None,
    zero_background=True,
):
    channel_count = int(channels or 1)
    mode = normalize_color_interpretation(
        interpretation or ("scalar_lut" if channel_count == 1 else "rgb")
    )
    cmd = [
        SCORE_RAW_RENDER,
        raw_path,
        out_path,
        f"--pix={int(pix)}",
        f"--channels={channel_count}",
        f"--interpretation={mode}",
        f"--palette={str(palette or 'inferno')}",
        f"--background_color={str(background_color or '000000')}",
        f"--quality={int(quality)}",
        f"--zero_background={1 if zero_background else 0}",
    ]
    if eq_lut_path:
        cmd.append(f"--eq_lut={eq_lut_path}")
    if preview_path:
        cmd.extend([f"--preview={preview_path}", "--preview_max=512"])
    # ...
```

### `lambda/score_raw_render.c`

The C side already gates `--eq_lut` requirement on `channels=1` (today's
check at the "Missing required scalar args" branch). The channel-LUT
implementation `convert_palette_component_lut_raw_to_image` does not read
the eq LUT. Confirm this and ensure the rgb / hsv (non-LUT) branches also
do not require eq LUT when invoked without it. If any branch reads
`eqLutPath` unconditionally, add a `channels == 1` guard in front of it.
No change should be required if the existing structure is

```c
if (channels == 1 && (!eqLutPath || !paletteName || !backgroundColor)) {
    fprintf(stderr, "Missing required scalar args: ...\n");
    return 2;
}
```

The binary still needs to be rebuilt as part of this change because the
Python wrapper now omits `--eq_lut` for non-scalar interpretations and the
docker runtime test will exercise the new invocation shape.

### `lambda/color_recolor_raw.py`

Split the dispatcher; keep all current behavior for scalar.

```python
# lambda/color_recolor_raw.py

from color_render_contract import repalette_target_for_source

def handle_color_recolor_from_raw_request(params, *, source_head=None, already_started=False):
    job_id = params["job_id"]
    task_id = params["task_id"]
    artifact_id = params["artifact_id"]
    source_artifact_id = params["source_artifact_id"]
    source_image_key = params["source_image_key"]
    new_palette = str(params["new_palette"]).strip()
    if new_palette not in VALID_PALETTE_NAMES:
        raise RuntimeError(f"Invalid palette: {new_palette}")
    requested_interpretation = params.get("new_interpretation")

    progress = {
        "family": "color",
        "artifact_id": artifact_id,
        "source_artifact_id": source_artifact_id,
        "derivation_kind": "color_repalette",
    }
    if not already_started:
        _phase(job_id, task_id, "started", "color_repalette", "RePalette", **progress)

    if source_head is None:
        source_head = load_color_artifact_head(s3, BUCKET, job_id, source_artifact_id)
    source_meta = dict(source_head.get("metadata", {}) or {})
    source_image_key = str(source_head.get("image_key") or source_image_key)

    _validate_source_metadata_shape(source_meta, source_artifact_id)

    source_raw_key = str(source_meta.get("raw_key", "") or "").strip()
    source_raw_meta_key = str(source_meta.get("raw_meta_key", "") or "").strip()
    if not (source_raw_key and source_raw_meta_key):
        raise RuntimeError("Recolor-from-raw requires raw_key and raw_meta_key on the source artifact")

    raw_sidecar = validate_raw_sidecar(
        _load_json_key(source_raw_meta_key),
        expected_raw_key=source_raw_key,
        expected_artifact_family="color",
        # require_scalar removed here — gated per branch below.
    )
    source_channels = int(raw_sidecar.get("channels", 1) or 1)
    target_interpretation = repalette_target_for_source(
        source_channels=source_channels,
        requested_interpretation=requested_interpretation,
    )

    if source_channels == 1:
        return _handle_scalar_recolor_from_raw(
            params=params,
            source_head=source_head,
            source_meta=source_meta,
            source_image_key=source_image_key,
            new_palette=new_palette,
            raw_sidecar=raw_sidecar,
            progress=progress,
        )
    if source_channels == 3:
        return _handle_channel_lut_recolor_from_raw(
            params=params,
            source_head=source_head,
            source_meta=source_meta,
            source_image_key=source_image_key,
            new_palette=new_palette,
            new_interpretation=target_interpretation,
            raw_sidecar=raw_sidecar,
            progress=progress,
        )
    raise RuntimeError(f"Color RePalette does not support channels={source_channels}")
```

The scalar branch is the existing function body, lifted into a private
helper. It re-asserts `channels=1` defensively:

```python
def _handle_scalar_recolor_from_raw(
    *,
    params,
    source_head,
    source_meta,
    source_image_key,
    new_palette,
    raw_sidecar,
    progress,
):
    if int(raw_sidecar.get("channels", 1) or 1) != 1:
        raise RuntimeError(
            "Recolor-from-raw scalar path requires channels=1; "
            f"got channels={raw_sidecar.get('channels')}"
        )
    # ... existing body of handle_color_recolor_from_raw_request,
    #     unchanged from today's color_recolor_raw.py:438-end ...
```

The new channel-LUT branch:

```python
def _handle_channel_lut_recolor_from_raw(
    *,
    params,
    source_head,
    source_meta,
    source_image_key,
    new_palette,
    new_interpretation,   # "rgb_lut" or "hsv_lut"
    raw_sidecar,
    progress,
):
    job_id = params["job_id"]
    task_id = params["task_id"]
    artifact_id = params["artifact_id"]
    source_artifact_id = params["source_artifact_id"]

    width = int(raw_sidecar["width"])
    height = int(raw_sidecar["height"])
    if width != height:
        raise RuntimeError(f"channel-LUT repalette requires square raw, got {width}x{height}")
    quality = _parse_int(source_meta.get("quality"), 90)
    background_color = background_color_hex(
        raw_sidecar.get("background_color", _normalize_background_color(source_meta.get("background_color")))
    )
    created_at = _utc_now_iso()
    fmt = str(source_meta.get("format", source_image_key.rsplit(".", 1)[-1].lower()) or "jpeg").lower()
    ext = "png" if fmt == "png" else "jpeg"

    prefix = f"renders/{job_id}/color/{artifact_id}/"
    image_key = prefix + f"image.{ext}"
    preview_key = prefix + "preview.png"
    new_raw_key = prefix + "greyscale.raw"
    new_raw_meta_key = prefix + "greyscale.meta.json"
    new_meta_key = prefix + "meta.json"

    source_raw_key = raw_sidecar["keys"]["raw_key"] if "keys" in raw_sidecar else source_meta["raw_key"]
    source_raw_path = "/tmp/color_repalette_source_packed.raw"
    encode_out_path = f"/tmp/color_repalette_image.{ext}"
    preview_out_path = "/tmp/color_repalette_preview.png"
    temp_paths = [source_raw_path, encode_out_path, preview_out_path]
    temp_copy_keys = []

    try:
        _phase(job_id, task_id, "rendering", "render_raw", "RePalette raw", **progress)
        _download_key_to_path(source_raw_key, source_raw_path)

        encode_result = render_score_raw(
            raw_path=source_raw_path,
            out_path=encode_out_path,
            preview_path=preview_out_path,
            pix=width,
            eq_lut_path=None,
            palette=new_palette,
            background_color=background_color,
            quality=quality,
            channels=3,
            interpretation=new_interpretation,
            zero_background=True,
        )

        _phase(job_id, task_id, "encoding", "sidecar", "Raw sidecar", **progress)
        _copy_raw_object(source_raw_key, new_raw_key)
        temp_copy_keys.append(new_raw_key)

        updated_sidecar = build_raw_sidecar(
            job_id=job_id,
            run_id=task_id,
            artifact_family="color",
            artifact_id=artifact_id,
            width=width,
            height=height,
            chain_fingerprint=raw_sidecar["chain_fingerprint"],
            score_chain=raw_sidecar["score_chain"],
            score_program=raw_sidecar["score_program"],
            clip_slots=raw_sidecar["clip_slots"],
            score_output_normalize=raw_sidecar.get("score_output_normalize", False),
            score_output_clip_lo=raw_sidecar.get("score_output_clip_lo", 0.0),
            score_output_clip_hi=raw_sidecar.get("score_output_clip_hi", 1.0),
            background_color=raw_sidecar.get("background_color", background_color),
            plan_params_digest=raw_sidecar["plan_params_digest"],
            render_execution=raw_sidecar["render_execution"],
            raw_key=new_raw_key,
            image_key=image_key,
            preview_key=preview_key,
            meta_key=new_meta_key,
            created_at=created_at,
            histogram=raw_sidecar.get("histogram"),
            channels=3,
            raw_layout="u8_packed_channels_row_major",
            interpretation=new_interpretation,
            output_channels=raw_sidecar.get("output_channels") or [],
            # step_scores_* copied through when present.
        )
        s3.put_object(
            Bucket=BUCKET,
            Key=new_raw_meta_key,
            Body=json.dumps(updated_sidecar, separators=(",", ":")).encode("utf-8"),
            ContentType="application/json",
        )
        temp_copy_keys.append(new_raw_meta_key)

        # Build the artifact metadata
        metadata = dict(source_meta)
        metadata.update({
            "artifact_id": artifact_id,
            "family": "color",
            "created_at": created_at,
            "palette": new_palette,
            "derived_from_artifact_id": source_artifact_id,
            "derived_from_image_key": source_image_key,
            "derivation_kind": "color_repalette",
            "background_color": background_color,
            "format": ext,
            "quality": str(quality),
            "raw_key": new_raw_key,
            "raw_meta_key": new_raw_meta_key,
            "raw_channels": "3",
            "raw_layout": "u8_packed_channels_row_major",
            "score_output_channel_count": "3",
            "score_output_interpretation": new_interpretation,
            "color_interpretation": new_interpretation,
            "rgb_source": f"{new_interpretation}_raw",
            "repalette_capable": "true",
        })
        for key in (
            "postprocess_kind", "postprocess_profile", "autolevels_params",
            "step_scores_key", "step_count", "step_scores_grid_n",
        ):
            metadata.pop(key, None)
        _clear_associated_palette_metadata(metadata)

        _phase(job_id, task_id, "encoding", "upload", "Upload image", **progress)
        image_meta, overlay_meta = split_color_artifact_metadata(metadata)
        final_headers = {"pix": str(width), "width": str(width), "height": str(height), **image_meta}
        if _metadata_size_bytes(final_headers) > S3_USER_METADATA_LIMIT_BYTES:
            raise RuntimeError("image metadata too large before upload")
        content_type = "image/png" if ext == "png" else "image/jpeg"
        with open(encode_out_path, "rb") as out_fh:
            s3.put_object(
                Bucket=BUCKET,
                Key=image_key,
                Body=out_fh,
                ContentType=content_type,
                Metadata=final_headers,
            )
        with open(preview_out_path, "rb") as prev_fh:
            s3.put_object(
                Bucket=BUCKET,
                Key=preview_key,
                Body=prev_fh,
                ContentType="image/png",
            )
        write_color_artifact_meta_overlay(s3, BUCKET, job_id, artifact_id, overlay_meta)

        _phase(job_id, task_id, "succeeded", "color_repalette", "RePalette", **progress)
        return ok_response({
            "artifact_id": artifact_id,
            "image_key": image_key,
            "preview_key": preview_key,
            "raw_key": new_raw_key,
            "raw_meta_key": new_raw_meta_key,
            "channels": 3,
            "interpretation": new_interpretation,
            "palette": new_palette,
            "encode_result": encode_result,
        })
    finally:
        for path in temp_paths:
            try:
                os.remove(path)
            except OSError:
                pass
        # Note: S3 keys uploaded above are intentionally not cleaned up here.
        # If the request fails, the orchestrator's cleanup pass deletes them.
```

### `index.html` (Color RePalette popup)

Two surface changes: gate the popup on channel-aware logic, and include
`new_interpretation` in the dispatch payload.

```javascript
// Add near _canColorRepaletteArtifact (~index.html:6745)

function _colorRepaletteSourceInterpretation(art) {
    const stored = (art && (art.score_output_interpretation || art.color_interpretation)) || '';
    const normalized = _normalizeColorInterpretation(stored);
    if (_artifactOutputChannelCount(art) === 3) {
        if (normalized === 'rgb' || normalized === 'rgb_lut') return 'rgb_lut';
        if (normalized === 'hsv' || normalized === 'hsv_lut') return 'hsv_lut';
        return 'rgb_lut';
    }
    return 'scalar_lut';
}

function _colorRepaletteSelectedInterpretation() {
    const checked = document.querySelector('input[name="color-repalette-interpretation"]:checked');
    if (checked && checked.value) return _normalizeColorInterpretation(checked.value);
    return '';
}
```

Wire interpretation radios into the popup (in the `<div id="color-repalette-popup">`
markup near `index.html:2047`):

```html
<!-- Inside the Color RePalette popup body, shown only when source channels === 3 -->
<div id="color-repalette-interpretation-row" style="display:none; margin-top:8px">
    <label class="color-mode-choice">
        <input type="radio" name="color-repalette-interpretation" value="rgb_lut" checked>
        <span class="color-mode-name">RGB LUT</span>
    </label>
    <label class="color-mode-choice">
        <input type="radio" name="color-repalette-interpretation" value="hsv_lut">
        <span class="color-mode-name">HSV LUT</span>
    </label>
</div>
```

Show the interpretation row when opening the popup if the source artifact is
3-channel, and pre-select the natural target. Update the existing
`_renderColorRepalettePopup` (or the existing popup-open code path) to call:

```javascript
function _populateColorRepaletteInterpretationRow(art) {
    const row = document.getElementById('color-repalette-interpretation-row');
    if (!row) return;
    if (_artifactOutputChannelCount(art) !== 3) {
        row.style.display = 'none';
        return;
    }
    row.style.display = '';
    const target = _colorRepaletteSourceInterpretation(art);
    const radio = document.querySelector(
        `input[name="color-repalette-interpretation"][value="${target}"]`
    );
    if (radio) radio.checked = true;
}
```

Update the dispatch in `runColorRepaletteSelectedArtifact` (around
`index.html:6945-6970`):

```javascript
const channelCount = _artifactOutputChannelCount(art);
const interpretation = channelCount === 3
    ? _colorRepaletteSelectedInterpretation()
    : '';
if (channelCount === 3 && !interpretation) {
    throw new Error('Color RePalette: pick an interpretation (RGB LUT or HSV LUT)');
}

const dispResult = await lambdaPost('dispatch', {
    target: 'color_repalette',
    jobs: [{
        job_id: jobId,
        task_id: taskId,
        artifact_id: artifactId,
        source_artifact_id: art.artifact_id,
        source_image_key: art.image_key,
        new_palette: paletteName,
        ...(interpretation ? { new_interpretation: interpretation } : {}),
    }],
    expected_keys: [],
});
```

The existing `_canColorRepaletteArtifact` does not need to change — both
scalar and 3-channel artifacts already pass its checks (it gates on
`color_mode === 'solve_score'` and the presence of a raw sidecar). Removing
any defensive scalar-only filter elsewhere in the UI is part of this change.

### `lambda/handler_color_repalette.py` and `lambda/handler_recolor_from_raw.py`

No body changes. Both are thin wrappers; the dispatcher logic moves into
`color_recolor_raw.py`. Verify the wrappers still pass `params` through
verbatim and do not strip `new_interpretation`.

### `deploy.sh`

No packaging changes. `color_render_contract.py` is already bundled into
both the `polypaint-color-repalette` and `polypaint-recolor-from-raw`
zips (per `tests/test_deploy_packaging.py`).

## Tests

### Unit Tests

`tests/test_color_repalette_handler.py` (new or extended)

- Scalar repalette of a `channels=1` artifact still works (regression).
- `channels=3` artifact + `new_interpretation=rgb_lut` produces a new
  artifact with `raw_channels=3`, `score_output_interpretation=rgb_lut`,
  `palette=<new_palette>`, and a copied raw key.
- `channels=3` artifact + `new_interpretation=hsv_lut` produces the same
  shape with `score_output_interpretation=hsv_lut`.
- `channels=3` artifact without `new_interpretation` is rejected with a
  clear error.
- `channels=3` artifact with `new_interpretation=rgb` or `hsv` or
  `scalar_lut` is rejected with a clear error.
- `channels=1` artifact with `new_interpretation=rgb_lut` is rejected.
- `score_raw_render` is invoked with `--channels=3 --interpretation=...
  --palette=<chosen>` and without `--eq_lut`.
- The new raw sidecar's `interpretation` field matches the new
  interpretation; `step_scores_key` / `step_count` /
  `step_scores_grid_n` are preserved when present on the source branch.

Sketch:

```python
# tests/test_color_repalette_handler.py

import json
from unittest.mock import MagicMock, patch


def _three_channel_sidecar(width=8):
    return {
        "version": 3,
        "job_id": "j",
        "run_id": "run",
        "artifact_family": "color",
        "artifact_id": "src",
        "width": width,
        "height": width,
        "channels": 3,
        "raw_layout": "u8_packed_channels_row_major",
        "interpretation": "rgb",
        "encoding": {"type": "u8_packed_channels_v1", "channels": 3, "background_byte": 0, "row_major": True},
        "chain_fingerprint": "fp",
        "score_chain": [],
        "score_program": "m0-0;emit_norm;m1-0;emit_norm;m2-0;emit_norm",
        "clip_slots": [],
        "background_color": "000000",
        "plan_params_digest": "digest",
        "render_execution": {},
        "keys": {"raw_key": "renders/j/color/src/greyscale.raw"},
        "histogram": [0] * 256,
        "output_channels": [
            {"channel": 0, "name": "r", "emit": "emit_norm", "range_normalized": True},
            {"channel": 1, "name": "g", "emit": "emit_norm", "range_normalized": True},
            {"channel": 2, "name": "b", "emit": "emit_norm", "range_normalized": True},
        ],
    }


@patch("color_recolor_raw._copy_raw_object")
@patch("color_recolor_raw._download_key_to_path")
@patch("color_recolor_raw._load_json_key")
@patch("color_recolor_raw.load_color_artifact_head")
@patch("color_recolor_raw.render_score_raw")
@patch("color_recolor_raw.s3")
def test_channel_lut_repalette_invokes_score_raw_render_with_interpretation(
    mock_s3, mock_render, mock_head, mock_load, mock_dl, mock_copy_raw
):
    from color_recolor_raw import handle_color_recolor_from_raw_request

    mock_head.return_value = {
        "metadata": {
            "family": "color", "color_mode": "solve_score",
            "artifact_id": "src", "raw_channels": "3",
            "score_output_interpretation": "rgb",
            "raw_key": "renders/j/color/src/greyscale.raw",
            "raw_meta_key": "renders/j/color/src/greyscale.meta.json",
            "width": "8", "height": "8", "format": "jpeg",
        },
        "image_key": "renders/j/color/src/image.jpeg",
    }
    mock_load.return_value = _three_channel_sidecar()
    mock_render.return_value = {"file_size": 100, "preview_file_size": 50}

    result = handle_color_recolor_from_raw_request({
        "job_id": "j",
        "task_id": "t",
        "artifact_id": "dst",
        "source_artifact_id": "src",
        "source_image_key": "renders/j/color/src/image.jpeg",
        "new_palette": "turbo",
        "new_interpretation": "hsv_lut",
    })

    assert result["statusCode"] == 200
    body = json.loads(result["body"])
    assert body["channels"] == 3
    assert body["interpretation"] == "hsv_lut"
    assert mock_render.call_count == 1
    kwargs = mock_render.call_args.kwargs
    assert kwargs["channels"] == 3
    assert kwargs["interpretation"] == "hsv_lut"
    assert kwargs["palette"] == "turbo"
    assert kwargs["eq_lut_path"] in (None, "")


def test_channel_lut_repalette_rejects_missing_interpretation():
    from color_recolor_raw import handle_color_recolor_from_raw_request
    # ... arrange head + sidecar to channels=3, omit new_interpretation,
    #     assert RuntimeError contains "requires an explicit new_interpretation".


def test_channel_lut_repalette_rejects_scalar_interpretation_on_three_channel_source():
    # new_interpretation=scalar_lut on a channels=3 source raises clearly.
    pass
```

### Docker Runtime Tests

`tests/docker_runtime_regression.py`

- Add a runtime test that:
  - Generates a synthetic `channels=3` raw artifact directly (no real
    render needed).
  - Runs `score_raw_render --channels=3 --interpretation=rgb_lut
    --palette=turbo` against it without `--eq_lut`.
  - Verifies the output PNG dimensions and that `read_png_dims` returns
    the expected size.
  - Repeats with `--interpretation=hsv_lut --palette=turbo`.
- This guards the optional-eq_lut C-side change.

Sketch:

```python
# tests/docker_runtime_regression.py — append within the score_raw_render block

def test_score_raw_render_channel_lut_without_eq_lut_runtime():
    print("\n--- score_raw_render channel-LUT without --eq_lut ---")
    width = 8
    raw_path = "/tmp/score_raw_render_lut_input.raw"
    rgb_lut_png = "/tmp/score_raw_render_rgb_lut.png"
    hsv_lut_png = "/tmp/score_raw_render_hsv_lut.png"
    cleanup(raw_path, rgb_lut_png, hsv_lut_png)

    payload = bytearray()
    for i in range(width * width):
        payload.extend([(i * 7) % 256, (i * 13) % 256, (i * 19) % 256])
    open(raw_path, "wb").write(bytes(payload))

    try:
        for interpretation, out_path in [
            ("rgb_lut", rgb_lut_png),
            ("hsv_lut", hsv_lut_png),
        ]:
            r = subprocess.run([
                "/src/score_raw_render",
                raw_path,
                out_path,
                f"--pix={width}",
                "--channels=3",
                f"--interpretation={interpretation}",
                "--palette=turbo",
                "--background_color=000000",
                "--quality=90",
            ], capture_output=True, text=True, timeout=15)
            assert r.returncode == 0, (
                f"score_raw_render {interpretation} failed without --eq_lut: "
                + r.stderr[:200]
            )
            assert read_png_dims(out_path) == (width, width)
        print("  score_raw_render channel-LUT without --eq_lut: OK")
    finally:
        cleanup(raw_path, rgb_lut_png, hsv_lut_png)
    print("=== score_raw_render channel-LUT without --eq_lut PASSED ===")
```

Add the call site in `__main__` near the existing
`test_roots2pix_mt_explicit_rgb_outputs_runtime()` invocation.

### Frontend Tests

`tests/test_frontend_js.sh`

- Assert the Color RePalette popup uses the source artifact's
  `raw_channels` to decide whether to show the interpretation chooser.
- Assert the submit payload includes `new_interpretation` when source
  channels is 3.

Sketch (append to `tests/test_frontend_js.sh`):

```bash
assertIncludes("function _colorRepaletteSourceInterpretation(art) {",
    'Color RePalette should derive a target interpretation from the source artifact');
assertIncludes("name=\"color-repalette-interpretation\"",
    'Color RePalette popup should expose interpretation radios');
assertIncludes("...(interpretation ? { new_interpretation: interpretation } : {}),",
    'Color RePalette dispatch should forward new_interpretation only when set');
assertIncludes("'Color RePalette: pick an interpretation (RGB LUT or HSV LUT)'",
    'Color RePalette should require an interpretation for 3-channel sources');
```

`tests/e2e/render-solve-score.spec.js` or a Color RePalette spec

- End-to-end: render a `channels=3` artifact, open Color RePalette, pick
  a different palette and `rgb_lut`, verify the dispatched payload and
  the resulting artifact metadata.

### Predeploy Gate

The predeploy gate runs the unit tests above plus
`scripts/predeploy_check.sh`. Native binary changes also require
`scripts/test-docker-runtime.sh`.

## Rollout

Single change. Native binary rebuild and Lambda packaging update happen
together. No staged migration is needed because:

- Existing scalar artifacts still go through the scalar path unchanged.
- Existing 3-channel artifacts that previously errored on Color RePalette
  start working when the user picks a channel-LUT interpretation.
- No saved program format changes.
- No ASL workflow contract changes.

## Open Questions

- Should the source-interpretation default for the popup be sticky across
  uses, or always default from the source artifact's stored
  interpretation? Recommended: derive from source for the first open and
  remember user choice for the session.
- Should we also expose `rgb_lut` / `hsv_lut` as an *initial render* mode
  on the Color section, where the user picks a palette at render time and
  the first render is already palette-baked? The native side already
  supports this; the Color section UI does not yet expose a palette
  selector for `RGB LUT` / `HSV LUT` modes. Out of scope for this fix.
- Future per-channel-different palettes (three palettes selected at once,
  combined per the multicolor v2 design) remain deferred.
