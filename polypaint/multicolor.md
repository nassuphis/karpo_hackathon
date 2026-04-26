# Multicolor Solve-Score Programs

## Current Model

Color rendering is currently scalar:

1. A solve-score RPN program evaluates to one final score value per solve/root/pixel contribution.
2. That scalar score is clipped/normalized into one `u8` byte.
3. The byte indexes one 1D palette.
4. Palette lookup produces final RGB.

This forces every visual decision through one scalar. The new design should separate score calculation from color interpretation.

## Core Direction

The score program should emit bytes, not images.

The raster contract becomes:

```text
sparse record = uint32 pixel_idx + N u8 channels
```

The meaning of those `N` bytes is declared later by artifact metadata and display/repalette code:

- `N=1`, interpretation `scalar_palette`: current palette-indexed behavior.
- `N=3`, interpretation `direct_rgb`: bytes are displayed directly as RGB.
- `N=3`, interpretation `channel_lut`: bytes are remapped through three independent channel LUTs.
- Future interpretations can reuse the same raw bytes without changing raster.

This keeps the rasterizer generic. It evaluates score programs, normalizes requested outputs, and writes bytes. It does not need to know about RGB, HSV, palette mixing, or other image semantics.

## Output Chips

Explicit-output programs use output chips.

### `emit`

`emit` consumes the top scalar stack value and appends one output channel.

Semantics:

```text
byte = clamp(round(255 * value), 0, 255)
```

Use `emit` when the program already produces a meaningful `[0, 1]` value.

Properties:

- No range estimate is needed.
- Non-finite values must use a deterministic fallback and log diagnostics.
- The emitted channel is appended in execution order.
- The consumed value is popped from the stack.

### `emit_norm`

`emit_norm` consumes the top scalar stack value and appends one normalized output channel.

Semantics:

```text
byte = clamp(round(255 * (value - lo) / (hi - lo)), 0, 255)
```

Use `emit_norm` when the output needs an estimated range.

Properties:

- The summary/histogram stage captures raw emitted values for this channel.
- The summary emits per-channel range metadata, preferably quantile-based (`q05`/`q95`) rather than raw min/max.
- Raster uses the chosen per-channel `lo/hi` to convert to `u8`.
- The emitted channel is appended in execution order.
- The consumed value is popped from the stack.

Degenerate range fallback:

- If `hi > lo`, normalize normally.
- Otherwise, clamp identity if values are already in `[0, 1]`.
- Otherwise emit zero for that channel.
- The fallback must be visible in native logs and response metadata.

Native token names should be boring:

```text
emit
emit_norm
```

The UI can label them `emit` and `emit-norm`.

## Program Validity

There are two program modes.

### Legacy Scalar Mode

If a program has no explicit output chips:

- Final stack depth must be `1`.
- Existing scalar behavior remains valid.
- The compiler/handler appends exactly one implicit output chip to the end of the program.
- The existing score-normalization checkbox chooses which output chip is appended:

```text
score normalization off -> implicit emit
score normalization on  -> implicit emit_norm
```

Example:

```text
m0-0;sawtooth:10
```

becomes one of:

```text
m0-0;sawtooth:10;emit
m0-0;sawtooth:10;emit_norm
```

This preserves old saved programs and old render behavior.

### Explicit Output Mode

If a program has one or more output chips:

- The compiler counts output chips to determine `N`.
- Final stack depth must be `0` unless a future feature explicitly allows scratch leftovers.
- The global score-normalization checkbox is disabled/hidden/removed for this program.
- Normalization is per output channel and is controlled only by `emit` vs `emit_norm`.

This makes score normalization part of the score program instead of a separate render-time switch.

Example:

```text
metric(proximity,slv,0.1)
emit_norm
metric(spread,cf,0.5)
sawtooth(10)
emit_norm
metric(angular_entropy_16,cf,0.5)
emit
```

This emits three channels:

- Channel 0: normalized proximity.
- Channel 1: normalized sawtooth spread.
- Channel 2: direct angular entropy clamped as `[0, 1]`.

## RGB Sugar

Direct RGB should be UI/compiler sugar, not the fundamental native abstraction.

The explicit form is:

```text
red_expr
emit_norm
green_expr
emit_norm
blue_expr
emit_norm
```

with metadata:

```json
{
  "channels": 3,
  "interpretation": "direct_rgb",
  "channel_names": ["r", "g", "b"]
}
```

The UI may expose a convenience Direct RGB chip/preset that creates those three `emit_norm` outputs and direct-RGB metadata.

Recommended canonical saved form:

```text
red_expr
emit_norm
green_expr
emit_norm
blue_expr
emit_norm
```

with:

```json
{
  "interpretation": "direct_rgb",
  "channel_names": ["r", "g", "b"]
}
```

Trailing-token `rgb` syntax is not part of v1:

```text
metric(proximity,slv,0.1)
metric(spread,cf,0.5)
metric(angular_entropy_16,cf,0.5)
rgb
```

Reason:

- With plain RPN `emit_norm` pops the top value first.
- After pushing `R G B`, three plain emits would produce `B G R`.
- The current VM has no stack-reorder op such as `swap`, `dup`, or `roll3`.
- Adding a native color-specific `rgb` operation would undermine the generic `N`-channel raster contract.

V1 should ship only the explicit canonical form and the UI preset that generates it. A future trailing `rgb` token can be added after one of these exists:

- Stack-reorder ops.
- A generic multi-output chip such as `emit3_norm`.
- A compiler representation that preserves expression boundaries before lowering.

The desired channel mapping remains:

```text
stack[-3] -> r -> emit_norm
stack[-2] -> g -> emit_norm
stack[-1] -> b -> emit_norm
```

The compiled/native representation should still be output-channel based. Native raster should only need to know that the program emits `N=3` bytes.

Direct RGB requires no palette selection:

```text
out.r = raw.r
out.g = raw.g
out.b = raw.b
```

## Deferred Color Sugar

### `hsv`

Do not ship `hsv` in v1.

Reason:

- Hue normalization is not the same as scalar or RGB normalization.
- Linear min/max or q05/q95 normalization can produce unstable hue behavior for bimodal or outlier-heavy score distributions.
- A future `hsv` design must choose clipped hue, wrapped hue, or sawtooth-style hue semantics.

Candidate v2 policy:

- `h` may use wrap mode: `h = frac(raw_h)` with no clip range.
- `s` and `v` use normal per-channel quantile ranges.

### `palette_mix` / `palette_modulate`

Do not ship these in v1.

These names cover multiple possible operations:

- brightness modulation
- saturation modulation
- contrast modulation
- blend-with-background modulation
- palette blending

Before implementation, split them into explicit semantics such as `palette_brightness`, `palette_saturation`, or `palette_blend`.

## Native Evaluator Contract

This is the load-bearing native change.

The current solve-score evaluator is scalar: one program evaluation returns one `double` score.

The explicit-output model changes that contract:

```text
evaluate(inputs) -> output_count N, raw_values[N], emit_modes[N]
```

Summary and raster consume that output vector differently:

- Summary/histogram records raw values for `emit_norm` outputs to estimate ranges.
- Raster converts each raw value to one `u8` using that channel's emit mode and range metadata.
- Legacy scalar callers use the same path with `N=1` and an implicit `emit` or `emit_norm`.

Every current native caller that evaluates score programs must migrate to the vector contract:

- Summary/statistics.
- Raster.
- Render preview.
- Palette/chunk paths that remain scalar or reject explicit multi-output programs.

The VM should stay generic. It should know about output chips and output count, not image meanings such as RGB or HSV.

## Summary / Histogram Contract

The summary path must support multichannel output in one native invocation.

Do not run the stats binary once per output channel. That would add a factor of `N` to a latency-sensitive path.

Extend the native stats evaluator so one invocation can:

- Evaluate the full RPN program.
- Track every explicit output channel.
- Capture raw emitted values for `emit_norm` channels.
- Emit one JSON object with per-channel range metadata.

Scalar legacy mode must remain backward-compatible.

For legacy scalar mode, keep existing top-level fields:

- `clip_lo`
- `clip_hi`
- `q05`
- `q95`
- `min_score`
- `max_score`
- any existing fields consumed by current handlers

Also add a channel array:

```json
{
  "output_mode": "scalar_palette",
  "channels": 1,
  "score_output_channels": [
    {
      "name": "score",
      "emit": "emit_norm",
      "range_normalized": true,
      "clip_source": "lores_q05_q95",
      "clip_lo": 0.12,
      "clip_hi": 0.87,
      "q05": 0.18,
      "q95": 0.82
    }
  ]
}
```

For explicit RGB sugar lowered to three normalized channels:

```json
{
  "output_mode": "channels",
  "channels": 3,
  "interpretation": "direct_rgb",
  "score_output_channels": [
    {"name": "r", "emit": "emit_norm", "range_normalized": true, "clip_source": "lores_q05_q95", "clip_lo": 0.12, "clip_hi": 0.87, "q05": 0.18, "q95": 0.82},
    {"name": "g", "emit": "emit_norm", "range_normalized": true, "clip_source": "lores_q05_q95", "clip_lo": 0.02, "clip_hi": 0.64, "q05": 0.05, "q95": 0.52},
    {"name": "b", "emit": "emit_norm", "range_normalized": true, "clip_source": "lores_q05_q95", "clip_lo": 0.10, "clip_hi": 0.95, "q05": 0.22, "q95": 0.88}
  ]
}
```

For `emit` channels, the channel row should still be present but range fields may be `0..1` or explicitly marked as not range-normalized:

```json
{
  "name": "alpha",
  "emit": "emit",
  "clip_lo": 0.0,
  "clip_hi": 1.0,
  "range_normalized": false,
  "clip_source": "identity"
}
```

Use `range_normalized` and `clip_source` per channel for new metadata. Legacy top-level fields such as `score_output_clip_source` can remain mirrored for backward compatibility, but new consumers should prefer the channel rows.

## Raster / Fragment Storage Contract

Raster emits generic packed channel fragments.

Scalar fragment:

```text
uint32 pixel_idx
uint8 channel_0
```

Generic fragment:

```text
uint32 pixel_idx
uint8 channel_0
...
uint8 channel_N_minus_1
```

Record size:

```text
4 + N bytes
```

V1 validation contract:

- Fragment manifests declare `channels`, `record_size_bytes`, and `fragment_encoding`.
- `fragment_encoding` should be a versioned value such as `u32le_pixel_idx_plus_u8_channels_v1`.
- Fragment files do not need per-file headers in v1.
- Every fragment reader validates that `file_size % record_size_bytes == 0`.
- Every fragment reader uses the manifest channel count; no reader may assume 5-byte records.
- Mismatched `channels`, `record_size_bytes`, or raw metadata is a hard error, not a best-effort decode.

RGB sugar is just `N=3`:

```text
uint32 pixel_idx
uint8 r
uint8 g
uint8 b
```

Finalize assembles fragments into one packed row-major raw artifact:

```text
width * height * N bytes
byte order per pixel: channel_0, channel_1, ...
```

Metadata must include:

- `channels`
- `raw_layout: "packed_u8_channels"`
- `interpretation`
- `channel_names`
- per-channel emit mode
- per-channel clip/range metadata
- program fingerprint
- program spec

For direct RGB, metadata should be:

```text
channels: 3
interpretation: "direct_rgb"
channel_names: ["r", "g", "b"]
raw_layout: "packed_u8_channels"
```

Do not add `r.raw`, `g.raw`, or `b.raw` sidecars in v1. A single packed raw is enough.

## Repalette / LUTs / Autolevels

Packed channel raw can be interpreted later.

Scalar palette mode:

```text
out.rgb = palette[raw.channel_0]
```

Direct RGB mode:

```text
out.r = raw.r
out.g = raw.g
out.b = raw.b
```

RGB channel-LUT repalette:

```text
out.r = lut_r[raw.r]
out.g = lut_g[raw.g]
out.b = lut_b[raw.b]
```

V1 channel LUTs are one-channel LUTs:

```text
uint8 -> uint8
```

The UI can present those as three palettes or curves, but the v1 contract is channel-to-channel remapping.

Do not treat v1 RGB repalette as three full RGB palettes unless a combine rule is explicitly added:

```text
c_r = palette_r[raw.r]  // RGB
c_g = palette_g[raw.g]  // RGB
c_b = palette_b[raw.b]  // RGB
out = combine(c_r, c_g, c_b)
```

The combine rule could be average, additive clamp, screen, weighted sum, or something else. Defer that to v2.

Autolevels:

- Existing scalar autolevels operates on one channel.
- For `channels != 1`, reject with a clear error until channel-aware autolevels is designed.
- Future autolevels can operate per channel or on luminance, but the policy must be explicit.

## Export Paths

All export paths must branch on raw channel metadata.

V1 behavior:

- `channels == 1`, `interpretation == "scalar_palette"`: existing palette-to-RGB export path.
- `channels == 3`, `interpretation == "direct_rgb"`: treat packed raw as RGB and skip palette lookup.
- `channels == 3`, `interpretation == "channel_lut"`: apply the three channel LUTs, then export RGB.
- Any other channel count or interpretation is rejected with a clear error until implemented.

This applies to PNG, TIFF, PDF, DeepZoom export, and DeepZoom-from-raw. These paths must not silently read packed RGB as scalar palette bytes.

## Lag Semantics

Lag bookkeeping is unchanged.

All output channels evaluate over the same metric slots, same source families, and same lagged metric buffers. If any channel uses `mN-1`, the same prelude-row contract applies:

- `prelude_rows` for solve roots
- `score_coeff_prelude_rows` for coeff source
- `score_param_prelude_rows` for param source

Thread/section boundary handling must not become per-channel. The lag context is shared by the whole program evaluation.

## Palette Extraction

Palette extraction is scalar-only in v1.

Reason:

- Palette extraction samples a scalar score field and builds a 1D palette artifact.
- A multi-output program has no single scalar output to rank or sample.

Required behavior:

- Reject explicit-output programs in palette-generation and palette-extraction contexts unless they emit exactly one scalar channel and the caller explicitly accepts it.
- Error message should be explicit:

```text
multi-output solve-score programs are not supported for palette extraction in v1
```

Direct render and render preview can support explicit-output programs; palette extraction should stay scalar.

## UI Model

The score editor should expose:

- `emit`
- `emit-norm`
- Direct RGB sugar/preset

For explicit-output programs:

- Hide or disable the global score-normalization checkbox.
- Show normalization at the output-chip level (`emit` vs `emit-norm`).
- If interpretation is `direct_rgb`, no scalar palette selector is required.
- Later, if interpretation is `channel_lut`, show three channel LUT/palette selectors.

Legacy scalar mode can keep the existing score-normalization checkbox until old saved programs are migrated. After migration, the checkbox can be removed entirely in favor of explicit or implicit output chips.

Do not auto-append `omega_cosine` for explicit-output programs. Compiler-side rule:

- Explicit-output programs are taken verbatim.
- `omega_cosine` is only applied when the user put it before an output chip.
- Legacy scalar programs may keep the existing omega auto-append behavior, but the auto-appended omega must occur before the implicit `emit` / `emit_norm`.

Unary transforms apply before `emit` / `emit_norm`.

Example:

```text
metric(proximity,slv,0.1)
omega_cosine(8,0)
emit_norm
```

Here omega transforms the value before it is emitted.

## Fingerprints And Cache Keys

Raw channel artifacts depend on:

- compiled score program
- emit mode per output channel
- source metrics and quantiles
- per-channel normalization ranges
- source artifact IDs

Display artifacts also depend on interpretation:

- scalar palette ID
- direct RGB metadata
- channel LUT IDs
- future compositor/blend settings

The compiled program fingerprint must include explicit output chips. A program ending in implicit scalar mode must not collide with an explicit `emit` or `emit_norm` program that happens to produce similar bytes.

If a future RGB sugar token is stored in saved programs, the compiler should expand it before hashing or include the expanded output declaration in the hash. This keeps sugar and explicit form cache-compatible when they mean the same thing.

The render plan digest can keep `solve_score_normalize` for legacy scalar compatibility. Explicit-output programs ignore that flag because emit modes are already part of the compiled output fingerprint.

## Compiler Model

Explicit RGB form can compile as:

```json
{
  "program_spec": "m0-0;emit_norm;m1-0;emit_norm;m2-0;emit_norm",
  "output_mode": "channels",
  "interpretation": "direct_rgb",
  "outputs": [
    {"name": "r", "emit": "emit_norm", "channel": 0},
    {"name": "g", "emit": "emit_norm", "channel": 1},
    {"name": "b", "emit": "emit_norm", "channel": 2}
  ],
  "output_channel_count": 3
}
```

Legacy scalar mode can compile as:

```json
{
  "program_spec": "m0-0;sawtooth:10",
  "output_mode": "scalar_palette",
  "implicit_emit": "emit_norm",
  "outputs": [
    {"name": "score", "emit": "emit_norm", "channel": 0}
  ],
  "output_channel_count": 1
}
```

Validation:

- No explicit output chips: final stack depth must be `1`, then append one implicit `emit` or `emit_norm` from the score-normalization checkbox.
- Explicit output chips: final stack depth must be `0`.
- `emit` / `emit_norm` require stack depth at least `1`.
- Direct RGB UI sugar requires three source expressions.
- Palette contexts reject unsupported multi-output programs.

Validation errors should include chip index and observed stack depth:

```text
emit at chip 7: stack depth is 0, need >= 1
explicit-output program ended with stack depth 2, need 0
direct RGB preset requires exactly 3 source expressions
```

## Proposed V1 Scope

Ship:

- Existing scalar palette mode unchanged for old programs.
- New `emit` and `emit_norm` output chips.
- Direct RGB preset that generates three canonical `emit_norm` outputs.
- Per-channel summary ranges from one native stats invocation.
- Generic packed sparse fragments: `pixel_idx + N bytes`.
- Generic packed raw artifacts: `width * height * N`.
- Direct RGB preview/render/export for `N=3`.
- RGB channel-LUT repalette over packed RGB raw.
- Clear rejection in palette extraction/generation where unsupported.
- Clear rejection in scalar-only autolevel paths.

Do not ship:

- `hsv`
- `palette_mix`
- `palette_modulate`
- channel equalization
- full RGB-palette triplet blending
- channel-aware autolevels
- alternate hue normalization modes

## Implementation Sketch

Stage 0: Native Storage Shape

- Define generic sparse record shape: `uint32 pixel_idx + N u8 channels`.
- Define packed raw metadata fields.
- Add tests for scalar, direct RGB, and generic channel metadata.

Stage 1: Compiler

- Add `emit` and `emit_norm` to Python and JS compilers.
- Add explicit-output validation.
- Add Direct RGB preset generation.
- Add `output_mode`, `interpretation`, `outputs`, and `output_channel_count`.
- Keep legacy scalar behavior unchanged.
- Add palette-context rejection for unsupported explicit-output programs.

Stage 2: Summary

- Extend native summary in one invocation to track all output channels.
- Capture ranges for `emit_norm` channels.
- Keep scalar top-level summary fields for legacy consumers.
- Update handlers to consume per-channel ranges.

Stage 2.5: Workflow Payloads

- Thread `score_output_channels` through `lambda/workflow_contracts.py`.
- Update `stepfunctions/render_workflow.asl.json.template` JsonPath selectors from scalar clip fields to the channel array.
- Keep scalar `score_output_clip_lo/hi/source/normalize` fields only as backward-compatible mirrors for legacy consumers.
- Ensure finalize persists channel metadata into the color artifact metadata.

Stage 3: Raster

- Change the native score evaluator API from one scalar result to an output vector.
- Migrate summary, raster, preview, and scalar-only palette/chunk callers to the vector evaluator.
- Extend native raster score evaluator to emit `N` bytes per sparse record.
- Apply per-channel `emit` / `emit_norm` conversion.
- Preserve lag prelude behavior across all channels.
- Add runtime tests for `N=1` and `N=3` fragments.

Stage 4: Finalize / Preview / Export

- Teach finalize to assemble packed `N`-channel raw from fragments.
- Teach preview/export paths to display direct RGB raw.
- Update PNG, TIFF, PDF, DeepZoom, and DeepZoom-from-raw export paths for `channels == 3`.
- Teach recolor to apply explicit RGB channel-LUT repalette to packed RGB raw.
- Reject multi-channel raw in scalar-only autolevel paths.

Stage 5: Product UI

- Add `emit` and `emit-norm` chips.
- Add direct RGB sugar/preset.
- Remove or disable score normalization for explicit-output programs.
- Hide scalar palette selection for direct RGB.
- Add optional RGB channel-LUT repalette controls later.
- Make logs print output count, interpretation, emit modes, and channel ranges.

Stage 6: Test / Deploy Gates

- Add workflow-contract tests for the per-channel payload shape.
- Add native docker runtime regressions for `N=1` legacy parity and `N=3` direct RGB fragments through finalize.
- Add unit tests for fragment record-size validation and metadata mismatch rejection.
- Add frontend/Playwright coverage for `emit`, `emit-norm`, and Direct RGB preset insertion.
- Run the project predeploy gate, including API manifest and workflow definition checks.

## Open Questions For V2

- HSV hue policy: clipped, wrapped, sawtooth, or user-selectable.
- Channel equalization: per-channel, luminance-based, or none.
- Full RGB-palette/blend recolor semantics.
- Channel-aware autolevels semantics.
- Palette mix blend space: RGB, HSV, Lab, or other.
- Whether palette-based interpretation should reference global palette controls or carry palette params.
- Whether legacy `omega_cosine` auto-append remains forever or is removed after scalar programs migrate to explicit outputs.
