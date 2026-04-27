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

- `N=1`, interpretation `scalar_lut`: one byte indexes a scalar LUT/palette.
- `N=3`, interpretation `rgb`: bytes are displayed directly as RGB.
- `N=3`, interpretation `hsv`: bytes are interpreted as HSV and converted to RGB.
- `N=3`, interpretation `rgb_lut`: bytes are three independent lookup
  coordinates into the selected palette. The first lookup contributes `R`, the
  second contributes `G`, and the third contributes `B`.
- `N=3`, interpretation `hsv_lut`: bytes are three independent lookup
  coordinates into the selected palette after palette stops are converted to HSV.
  The first lookup contributes `H`, the second contributes `S`, and the third
  contributes `V`, then the resulting HSV is converted to RGB.
- Future interpretations can reuse the same raw bytes without changing raster.

For LUT-backed interpretations, byte value `0` is reserved for background. The
palette ramp is addressed by byte values `1..255`, mapped as
`t = (byte - 1) / 254`, matching the existing scalar LUT path. In packed
three-channel raw, an absent pixel is encoded as `(0, 0, 0)` and must render as
`background_color`, not as the left edge of the selected palette.

This keeps the rasterizer generic. It evaluates score programs, normalizes requested outputs, and writes bytes. It does not need to know about RGB, HSV, palette mixing, or other image semantics.

## Stack And Output Chips

Explicit-output programs use output chips. The native program can keep the
boring token names `emit` and `emit_norm`, but the editor should expose this as
one chip:

```text
emit(mode = raw | norm | none)
```

Editor behavior:

- One visible `emit` chip has a dropdown with `raw`, `norm`, and `none`.
- `raw` lowers to native `emit`.
- `norm` lowers to native `emit_norm`.
- `none` lowers to native `emit_none`.
- Existing saved/native `emit`, `emit_norm`, and `emit_none` inputs remain
  accepted.
- Existing `emit_norm` chips should display as `emit(norm)` so users can flip
  the mode without deleting/reinserting the chip.

### `emit(raw)`

`emit(raw)` consumes the top scalar stack value and appends one output channel.

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

### `emit(none)`

`emit(none)` consumes the top scalar stack value but does not append an output
channel.

Use it as a debugging switch when trying multiple candidate values inside one
program. The branch stays in place, but its value is discarded.

Properties:

- It lowers to native token `emit_none`.
- It participates in explicit-output validation.
- It does not increment output-channel count `N`.
- A program containing only `emit(none)` chips is invalid for render because it
  emits no bytes.
- The consumed value is popped from the stack.

### `emit(norm)`

`emit(norm)` consumes the top scalar stack value and appends one normalized output channel.

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
emit_none
```

The UI label is `emit` with a mode dropdown.

## Stack Utility And Arithmetic Chips

The score editor needs a small set of general-purpose stack operations so color
channels can be composed without rewriting whole programs.

### `const`

`const(value)` pushes a numeric constant onto the stack.

Properties:

- Stack effect: `0 -> 1`.
- UI exposes one numeric input.
- Saved form can be `["const", "0.5"]`.
- Native/program-spec form can be `const:0.5`.
- Numeric parsing should accept scientific notation, e.g. `1e-6` and `1e6`.
- Non-finite constants are rejected at parse time with a clear error such as
  `const requires a finite numeric value`.
- The value is not interpreted as color by itself; it is just another scalar
  stack value that may later be emitted.

### `dup`

`dup` duplicates the top stack value.

```text
a -> a a
```

Use cases:

- Emit the same score into more than one channel with different transforms.
- Apply two operations to one metric without re-adding the metric chip.

### Arithmetic And Math

Add explicit arithmetic and math chips:

```text
add
mult
subtract
ratio
clamp
ema(alpha)
sin
cos
log
exp
pow(exponent)
```

Stack effects:

```text
a b add      -> (a + b)
a b mult     -> (a * b)
a b subtract -> (a - b)
a b ratio    -> (a / b) if b != 0 else 0
a clamp      -> clamp(a, 0, 1)
a b ema(k)   -> a * k + b * (1 - k)
a sin        -> sin(a)
a cos        -> cos(a)
a log        -> ln(a)
a exp        -> exp(a)
a pow(p)     -> pow(a, p)
```

Semantics:

- `add`, `mult`, `subtract`, `ratio`, `ema`, and the unary math transforms are
  raw. They do not clamp intermediate values to `[0, 1]`.
- `subtract` and `ratio` use normal RPN operand order: the second popped value
  is the left operand, the first popped value is the right operand.
- `ratio` returns `0` for denominator `0` or non-finite denominator.
- `ema(k)` is a binary EMA-style blend, not hidden state. It consumes two stack
  values. The older/lower value is `a`, the latest/top-of-stack value is `b`,
  and the result is `a*k + b*(1-k)`. Example: `ema(0.99)` means
  `a*0.99 + b*0.01`.
- `ema(k)` requires finite `k` in `[0, 1]`.
- `sin` and `cos` use radians. They are plain math transforms, not periodic
  score-normalization helpers.
- `log` is natural logarithm. `log(a)` returns `0` for `a <= 0` or non-finite
  input.
- `exp` is raw exponential. Overflow or non-finite result pushes `0`.
- `pow(exponent)` requires a finite exponent. Invalid domains, such as a
  negative base with a non-integer exponent, push `0`.
- Any arithmetic or math operation that produces a non-finite result pushes
  `0`.
- `clamp` is the explicit image-range clamp chip. In v1 it clamps to `[0, 1]`.
- V1 `clamp` takes no parameters. Future versions may add parameterized
  clamps, but `clamp` alone always means `[0, 1]`.
- Emitted bytes are always clamped when converted to `u8`; intermediate stack
  values are not clamped unless the program contains `clamp`.
- Arithmetic and math chips should avoid hidden image semantics. They operate
  on scalar stack values only.
- Be careful with backward compatibility for the existing native `mul` token.
  If `mul` currently clamps to `[0, 1]`, keep that legacy behavior for old
  saved programs only. The new editor `mult` chip must lower to raw
  multiplication, either through a new native token or an explicitly raw
  compatibility path. Do not silently change old saved-program behavior.
- Do not treat `add` as equivalent to `weighted_sum:1:1`. Existing
  `weighted_sum` is a legacy clamped blend; new arithmetic chips are raw.

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

- The compiler counts only emitting output chips (`raw`/`norm`) to determine
  `N`; `emit(none)` consumes a value but does not allocate a channel.
- Final stack depth must be `0` unless a future feature explicitly allows scratch leftovers.
- The global score-normalization checkbox is disabled/hidden/removed for this program.
- Normalization is per emitted output channel and is controlled only by `emit`
  vs `emit_norm`; `emit_none` has no channel to normalize.
- The program must emit at least one channel.

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
  "interpretation": "rgb",
  "channel_names": ["r", "g", "b"]
}
```

The UI may expose a convenience Direct RGB chip/preset that creates those three `emit_norm` outputs and selects `RGB` in the Color section.

Recommended canonical saved/editor form uses the unified editor chip, not the
native `emit_norm` token:

```json
[
  ["metric", "red_metric", "slv", "0.1"],
  ["emit", "norm"],
  ["metric", "green_metric", "slv", "0.1"],
  ["emit", "norm"],
  ["metric", "blue_metric", "slv", "0.1"],
  ["emit", "norm"]
]
```

Old saved/native `emit_norm` tokens remain accepted on input and are rewritten
to the unified editor form on the next save.

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
- The current VM has no stack-reorder op such as `swap` or `roll3`.
- `dup` does not solve this because it can copy a value but cannot reorder
  `R G B` into emit order.
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

Direct RGB should no longer be inferred only from `N == 3`. The user chooses the
interpretation in the Color section. The score program only determines how many
bytes are emitted and what their values are. ColorRender-MT validates that the
selected interpretation is compatible with the compiled program.

Recommended rules:

- `Scalar LUT` requires exactly one output channel.
- `RGB` requires exactly three output channels and names the channels `r`, `g`,
  `b`.
- `HSV` requires exactly three output channels and names the channels `h`, `s`,
  `v`.
- `RGB LUT` requires exactly three output channels and names the channels
  `r_lookup`, `g_lookup`, `b_lookup`.
- `HSV LUT` requires exactly three output channels and names the channels
  `h_lookup`, `s_lookup`, `v_lookup`.
- New renders should always carry an explicit selected interpretation.
- Old artifacts can keep compatibility aliases such as `scalar_palette` and
  `direct_rgb`, but the UI labels should be `Scalar LUT`, `RGB`, `HSV`,
  `RGB LUT`, and `HSV LUT`.
- Saved program presets may carry `recommended_interpretation`. Loading a
  preset applies it as the starting Color section choice, but the user can
  change it. This hint is not part of the solve-score chain identity hash.

## Color Interpretations

### `hsv`

HSV is an interpretation over three emitted channels, not a native raster mode.

The score program still emits three `u8` values. Finalize/display interprets
them as HSV and converts to RGB:

```text
raw.h, raw.s, raw.v -> hsv_to_rgb(raw.h, raw.s, raw.v)
```

V1 policy:

- `h` is a byte interpreted as `h / 255`.
- `s` is interpreted as `s / 255`.
- `v` is interpreted as `v / 255`.
- Range behavior is still controlled by each emitted channel's `emit(raw|norm)`
  mode.
- No special cyclic quantile estimator is introduced in v1.
- The UI should recommend `emit(raw)` for hue after an explicit `sawtooth`
  transform, because hue normalization is often visually unstable.

Reason:

- Hue normalization is not the same as scalar or RGB normalization.
- Linear min/max or q05/q95 normalization can produce unstable hue behavior for bimodal or outlier-heavy score distributions.
- A future `hsv` design must choose clipped hue, wrapped hue, or sawtooth-style hue semantics.

Candidate v2 policy:

- `h` may use wrap mode: `h = frac(raw_h)` with no clip range.

### `rgb_lut`

`rgb_lut` is a palette-component interpretation over three emitted lookup
coordinates.

The selected palette is treated as a continuous RGB ramp. For each pixel:

```text
x0, x1, x2 = emitted bytes
c0 = palette_rgb((x0 - 1) / 254)
c1 = palette_rgb((x1 - 1) / 254)
c2 = palette_rgb((x2 - 1) / 254)
output_rgb = (c0.r, c1.g, c2.b)
```

The three emitted values are not direct RGB components. They are independent
palette lookup coordinates. If all three bytes are zero, output
`background_color`. A zero byte in a non-background pixel uses the matching
background component for that channel.

### `hsv_lut`

`hsv_lut` is the same component-lookup idea, but interpolation happens in HSV
space.

For each palette segment, the RGB endpoints are converted to HSV and interpolated
there. Hue interpolation should use the shortest circular arc across the
`0..1` wrap.

For each pixel:

```text
x0, x1, x2 = emitted bytes
c0 = palette_hsv((x0 - 1) / 254)
c1 = palette_hsv((x1 - 1) / 254)
c2 = palette_hsv((x2 - 1) / 254)
output_hsv = (c0.h, c1.s, c2.v)
output_rgb = hsv_to_rgb(output_hsv)
```

The three emitted values are not direct H/S/V components. They are independent
palette lookup coordinates. If all three bytes are zero, output
`background_color`. A zero byte in a non-background pixel uses the matching
background HSV component for that channel.

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
  "channels": 3,
  "score_output_channels": [
    {"name": "r", "emit": "emit_norm", "range_normalized": true, "clip_source": "lores_q05_q95", "clip_lo": 0.12, "clip_hi": 0.87, "q05": 0.18, "q95": 0.82},
    {"name": "g", "emit": "emit_norm", "range_normalized": true, "clip_source": "lores_q05_q95", "clip_lo": 0.02, "clip_hi": 0.64, "q05": 0.05, "q95": 0.52},
    {"name": "b", "emit": "emit_norm", "range_normalized": true, "clip_source": "lores_q05_q95", "clip_lo": 0.10, "clip_hi": 0.95, "q05": 0.22, "q95": 0.88}
  ]
}
```

The native summary/statistics result should not know whether the render will
interpret three channels as `rgb`, `hsv`, `rgb_lut`, `hsv_lut`, or a future
channel-remapping interpretation. It only reports output count, emit modes, and
ranges. Color interpretation is selected in the render request and attached by
the render plan/finalize metadata.

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

Storage-wise, RGB sugar is just `N=3`:

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
interpretation: "rgb"
channel_names: ["r", "g", "b"]
raw_layout: "packed_u8_channels"
```

Do not add `r.raw`, `g.raw`, or `b.raw` sidecars in v1. A single packed raw is enough.

## Repalette / LUTs / Autolevels

Packed channel raw can be interpreted later.

Scalar LUT mode:

```text
out.rgb = palette[raw.channel_0]
```

Direct RGB mode:

```text
out.r = raw.r
out.g = raw.g
out.b = raw.b
```

Future RGB channel-LUT repalette:

```text
out.r = lut_r[raw.r]
out.g = lut_g[raw.g]
out.b = lut_b[raw.b]
```

When `channel_lut` ships, channel LUTs should be one-channel LUTs:

```text
uint8 -> uint8
```

The UI can present those as three palettes or curves, but the contract should
be channel-to-channel remapping.

Do not treat RGB repalette as three full RGB palettes unless a combine rule is explicitly added:

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

Required behavior for enabled interpretations:

- `channels == 1`, `interpretation == "scalar_lut"`: existing palette-to-RGB export path.
- `channels == 3`, `interpretation == "rgb"`: treat packed raw as RGB and skip palette lookup.
- `channels == 3`, `interpretation == "hsv"`: convert packed HSV bytes to RGB.
- `channels == 3`, `interpretation == "rgb_lut"`: sample the selected RGB palette three times with reserved-zero semantics and assemble `(sample0.r, sample1.g, sample2.b)`.
- `channels == 3`, `interpretation == "hsv_lut"`: sample the selected palette in HSV space three times with reserved-zero semantics and assemble `(sample0.h, sample1.s, sample2.v)`, then convert to RGB.
- Any other channel count or interpretation is rejected with a clear error until implemented.

Compatibility aliases:

- `scalar_palette` should be accepted as old metadata for `scalar_lut`.
- `direct_rgb` should be accepted as old metadata for `rgb`.

This applies to PNG, TIFF, PDF, DeepZoom export, and DeepZoom-from-raw. These paths must not silently read packed RGB as scalar palette bytes.

Scalar-only consumers must reject multi-channel raw before reading bytes.

Required clear-error behavior:

- Recolor-from-raw scalar palette path: reject `channels != 1` unless the
  request explicitly uses a channel-aware LUT mode.
- Color2Bilevel/from-raw bilevel: reject `channels != 1`.
- ExtractPalette-from-step-scores: reject `channels != 1` before checking
  `step_scores_key`.
- Autolevels: reject direct RGB/channel raw until a channel-aware policy exists.

Suggested error text:

```text
<feature> requires a scalar (channels=1) raw artifact; got channels=N
```

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

Single-output programs may still use `const`, `dup`, `add`, `mult`,
`subtract`, `ratio`, `ema`, `sin`, `cos`, `log`, `exp`, `pow`, and `clamp`.
These are scalar stack operations and do not make the program multi-output by
themselves.

Direct render and render preview can support explicit-output programs; palette extraction should stay scalar.

## Color Section

The Render Output area should become a `Color` section. This is visual output
configuration, not just palette selection.

The color interpretation controls should be mutually exclusive, even if they are
styled as checkbox rows:

- `Scalar LUT`: requires one output channel and shows the existing palette/LUT
  swatches on the same row.
- `RGB`: requires three output channels and uses packed bytes directly as RGB.
- `HSV`: requires three output channels and converts HSV bytes to RGB during
  display/finalize.

Layout requirement:

- The Color section should look intentional, not like the current cramped View
  controls.
- Each mode row should align labels, selectors, swatches, and helper text on a
  consistent vertical rhythm.
- If a mode needs multiple rows, those rows should be grouped under the mode and
  vertically aligned with the primary row, not scattered across the panel.
- `Scalar LUT` should keep the palette/LUT swatches on the same row when there
  is room, then wrap cleanly under the row on narrow screens.
- `RGB` and `HSV` rows should not leave empty placeholder controls where no
  palette selection is needed.

Do not rely on automatic interpretation as the primary UX. Automatic inference
from `N` is convenient for old artifacts, but new renders should carry the
explicit selected interpretation.

ColorRender-MT owns compatibility validation. Render preview should use the
same validator:

- Compile the score program.
- Count explicit or implicit output channels.
- Read the selected Color interpretation.
- Reject incompatible combinations before dispatching raster/finalize work.
- For `HSV`, if channel 0 (`h`) uses `emit(norm)`, warn but do not reject:
  `HSV hue uses normalized range; consider sawtooth + emit(raw)`.

Example validation:

```text
Scalar LUT requires 1 output channel, got 3
RGB requires 3 output channels, got 1
HSV requires 3 output channels, got 2
```

The frontend can mirror these checks to disable `Execute`, but the backend must
remain authoritative.

Warnings contract:

- Render plan and render preview should return `warnings: [string]`.
- The frontend should print those warnings in the render log/status area.
- Final color artifact metadata should persist the warning list so artifact
  cards and later debugging can show that the render used a questionable but
  allowed configuration.
- The HSV hue warning belongs in this list:
  `HSV hue uses normalized range; consider sawtooth + emit(raw)`.

## Score Editor UI

The score editor should expose:

- `emit(mode=raw|norm|none)` as one chip.
- Optional HSV starter preset.
- `const(value)`.
- `dup`.
- `flush`, which clears the entire stack without producing output.
- `add`, `mult`, `subtract`, `ratio`, `ema`, `sin`, `cos`, `log`, `exp`,
  `pow`, `clamp`.

For direct RGB, use a saved solve-score program with three output channels and
select `RGB` in the Color section. Example canonical form:

```text
metric(proximity,slv,0.1)
emit(norm)
metric(spread,slv,0.1)
emit(norm)
metric(angular_entropy_16,slv,0.1)
emit(norm)
```

An HSV starter preset is useful, but should be deferred unless HSV display is
implemented in the same change. It can use constants where useful:

```text
metric(proximity,slv,0.1)
sawtooth(10)
emit(raw)
const(1)
emit(raw)
metric(spread,slv,0.1)
emit(norm)
```

This maps hue from a sawtooth score, saturation to constant `1`, and value from
spread.

For explicit-output programs:

- Hide or disable the global score-normalization checkbox.
- Show normalization at the output-chip level (`emit(raw)` vs `emit(norm)`).
- If interpretation is `RGB` or `HSV`, no scalar palette selector is required.
- If interpretation is `Scalar LUT`, `RGB LUT`, or `HSV LUT`, show the shared
  palette selector because the selected palette affects final color.

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

## Score Editor Insertion UX

Appending every chip and moving it with arrows is too cumbersome for larger
programs.

Recommended editor model:

1. Each chip/line can be selected.
2. The toolbar has two add buttons: `+ before` and `+ after`.
3. Opening either button shows the same Add score chip popup, but scoped to the
   selected insertion boundary.
4. The editor is permissive: it should allow temporarily invalid intermediate
   programs while the user is constructing a final valid program.

If no chip is selected, insertion defaults to appending at the end. This
preserves the current workflow.

This is important. Users should not have to solve sequencing puzzles where the
only way to reach a desired final program is blocked because one intermediate
state is invalid.

Insertion hints should still be program-aware, but they are hints, not hard
gates:

- Compute stack depth at every boundary in the current program.
- For `+ before`, use the boundary before the selected chip.
- For `+ after`, use the boundary after the selected chip.
- For each candidate chip, optionally simulate insertion at that boundary.
- Mark candidates as `valid here`, `will underflow here`, or `may require more
  edits`, but do not prevent insertion solely because the resulting whole
  program is temporarily invalid.

Hint definitions:

- `valid here`: insertion is locally stack-valid and the resulting whole
  program is execution-valid for its current output mode.
- `will underflow here`: the chip needs more stack inputs than exist at that
  insertion boundary.
- `may require more edits`: insertion is locally stack-valid, but the resulting
  whole program fails final execution validation, usually because the final
  stack depth or output count is not yet compatible with the selected Color
  interpretation.

Execution validation is strict:

- The ColorRender-MT modal disables `Execute` if the current program is invalid
  or incompatible with the selected Color interpretation.
- The modal displays a concise reason, e.g. `invalid program: stack underflow at
  chip 7` or `program incompatible with RGB: expected 3 outputs, got 2`.
- If the user reaches execution through another path, the backend returns the
  same clear error and logs it.

Usability additions:

- Show the stack depth at each boundary.
- Highlight the selected insertion boundary.
- Highlight underflow chips and final-stack mismatch.
- Keep arrow buttons for quick adjacent moves, but do not make them the primary
  insertion workflow.
- Add keyboard shortcuts later: `b` for insert before, `a` for insert after,
  `Delete` to remove selected chip.

## Fingerprints And Cache Keys

Raw channel artifacts depend on:

- compiled score program
- emit mode per output channel
- source metrics and quantiles
- per-channel normalization ranges
- source artifact IDs

Display artifacts also depend on interpretation:

- scalar palette ID
- RGB metadata
- HSV metadata
- channel LUT IDs
- future compositor/blend settings

Use two identities:

- Source/editor identity hashes the saved user program before implicit legacy
  emit insertion. This preserves UI/history distinction between old scalar
  programs and explicitly emitted programs.
- Render-artifact identity hashes the effective `execution_program_spec`,
  selected interpretation, emit modes, ranges, and source artifacts.

For render artifacts, semantically identical implicit and explicit forms may
intentionally share cache keys. For editor/source identity, they may remain
distinct.

Draft editor state can still have a source identity if the chain is
syntactically parseable, even when it is not execution-valid. This supports
autosave, undo/redo, and dirty-state tracking during permissive editing. Fully
malformed chains, such as unknown chip names or unparseable parameters, have no
canonical source identity until repaired.

If a future RGB sugar token is stored in saved programs, the compiler should expand it before hashing or include the expanded output declaration in the hash. This keeps sugar and explicit form cache-compatible when they mean the same thing.

The render plan digest can keep `solve_score_normalize` for legacy scalar compatibility. Explicit-output programs ignore that flag because emit modes are already part of the compiled output fingerprint.

## Compiler Model

The compiler owns program syntax and output-count metadata. The Color section
owns output interpretation. Do not infer the user's intended color mode solely
from channel count except for old-artifact compatibility.

The pure compiler should not invent image interpretation. ColorRender-MT
combines compiler output with the selected Color section interpretation to
build the render-plan contract.

Keep source and execution forms distinct:

- `source_program_spec`: what the user saved or typed, before implicit legacy
  output insertion.
- `execution_program_spec`: the native program that summary/raster actually
  evaluate, after lowering editor sugar and appending any implicit legacy
  output chip.
- Render-artifact fingerprints and cache keys use `execution_program_spec`,
  selected interpretation, emit modes, and range metadata. This avoids
  accidental collisions between different executions while allowing deliberate
  reuse for semantically identical source forms.

Explicit RGB form can become this render-plan contract after ColorRender-MT
attaches the selected `rgb` interpretation:

```json
{
  "source_program_spec": "m0-0;emit_norm;m1-0;emit_norm;m2-0;emit_norm",
  "execution_program_spec": "m0-0;emit_norm;m1-0;emit_norm;m2-0;emit_norm",
  "interpretation": "rgb",
  "outputs": [
    {"name": "r", "emit": "emit_norm", "channel": 0},
    {"name": "g", "emit": "emit_norm", "channel": 1},
    {"name": "b", "emit": "emit_norm", "channel": 2}
  ],
  "output_channel_count": 3
}
```

The saved/editor form can still use the single visible `emit` chip:

```json
[
  ["metric", "proximity", "slv", "0.1"],
  ["emit", "norm"],
  ["metric", "spread", "slv", "0.1"],
  ["emit", "norm"],
  ["metric", "angular_entropy_16", "slv", "0.1"],
  ["emit", "norm"]
]
```

The compiler lowers `["emit", "norm"]` to `emit_norm` in
`execution_program_spec`.

Legacy scalar mode can become this render-plan contract after ColorRender-MT
attaches the selected `scalar_lut` interpretation:

```json
{
  "source_program_spec": "m0-0;sawtooth:10",
  "execution_program_spec": "m0-0;sawtooth:10;emit_norm",
  "interpretation": "scalar_lut",
  "implicit_emit": "emit_norm",
  "outputs": [
    {"name": "score", "emit": "emit_norm", "channel": 0}
  ],
  "output_channel_count": 1
}
```

Validation layers:

- The strict compiler validates syntax, chip arity, stack underflow, and final
  stack depth for execution.
- The editor may use a draft compiler that returns diagnostics without
  blocking temporary invalid states.
- The score editor may hold temporarily invalid programs while editing.
- ColorRender-MT validates that the compiled output count is compatible with the
  selected Color interpretation.
- Palette contexts reject unsupported multi-output programs.

Program validation rules:

- No explicit output chips: final stack depth must be `1`, then append one
  implicit `emit` or `emit_norm` from the score-normalization checkbox.
- Explicit output chips: final stack depth must be `0` at execution time.
- `emit(raw)` / `emit(norm)` require stack depth at least `1`.
- `const` always increases stack depth by one.
- `dup` requires stack depth at least `1`.
- Binary arithmetic chips require stack depth at least `2`.
- Unary math chips and `clamp` require stack depth at least `1`.

Draft diagnostics:

- The strict compiler raises on execution-invalid programs.
- A sibling diagnostics path should parse the chain and return diagnostics
  without raising for stack-effect problems. This is what the editor uses for
  insertion hints and disabled Execute messages.
- The diagnostics path may tolerate stack underflow and final-stack mismatch,
  but it should still reject or hard-error unparseable chip names, invalid
  metric names, invalid sources, invalid constants, and malformed parameters.

Color compatibility rules:

- `Scalar LUT` requires exactly one output.
- `RGB` requires exactly three outputs.
- `HSV` requires exactly three outputs.

Validation errors should include chip index and observed stack depth where
possible:

```text
emit(norm) at chip 7: stack depth is 0, need >= 1
explicit-output program ended with stack depth 2, need 0
RGB interpretation requires 3 outputs, got 2
Scalar LUT interpretation requires 1 output, got 3
```

## Proposed V1 Scope

Ship:

- Existing scalar LUT/palette mode unchanged for old programs.
- Unified editor `emit` chip with `raw` / `norm` mode, lowered to native
  `emit` / `emit_norm`.
- `const`, `dup`, `add`, `mult`, `subtract`, `ratio`, `ema`, `sin`, `cos`,
  `log`, `exp`, `pow`, and `clamp` stack chips.
- Selected-line insertion UX with `+ before` and `+ after`.
- Color section with explicit `Scalar LUT`, `RGB`, `HSV`, `RGB LUT`, and `HSV LUT`
  interpretations.
- HSV byte-to-RGB conversion for preview/render/export using the simple v1 byte
  interpretation.
- RGB LUT and HSV LUT palette-component lookup modes for three-output programs.
- Per-channel summary ranges from one native stats invocation.
- Generic packed sparse fragments: `pixel_idx + N bytes`.
- Generic packed raw artifacts: `width * height * N`.
- Direct RGB preview/render/export for `N=3`.
- Clear rejection in palette extraction/generation where unsupported.
- Clear rejection in scalar-only recolor, bilevel, ExtractPalette-from-raw, and
  autolevel paths.

Do not ship:

- Channel-LUT repalette over packed RGB raw unless the UI and metadata for that
  interpretation are implemented in the same change.
- `palette_mix`
- `palette_modulate`
- channel equalization
- full RGB-palette triplet blending
- channel-aware autolevels
- alternate hue normalization modes beyond the simple byte HSV interpretation
- HSV starter preset, unless it is trivial after HSV display lands

## Implementation Sketch

Stage 0: Native Storage Shape

- Define generic sparse record shape: `uint32 pixel_idx + N u8 channels`.
- Define packed raw metadata fields.
- Add tests for scalar, direct RGB, and generic channel metadata.

Stage 1: Compiler

- Add unified editor `emit(raw|norm|none)` and lower it to native `emit`,
  `emit_norm`, or `emit_none` in Python and JS compilers.
- Keep backward-compatible parsing for old explicit `emit`, `emit_norm`, and
  `emit_none` saved chains.
- Add `const`, `dup`, `flush`, `add`, `mult`, `subtract`, `ratio`, `ema`, `sin`,
  `cos`, `log`, `exp`, `pow`, and `clamp`.
- Add explicit-output validation.
- Add explicit color interpretation metadata, `outputs`, and
  `output_channel_count`.
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
- Apply per-channel `emit` / `emit_norm` conversion; `emit_none` discards its
  value and does not allocate an output byte.
- Support `flush` as a stack-clear operation.
- Preserve lag prelude behavior across all channels.
- Add runtime tests for `N=1` and `N=3` fragments.

Stage 4: Finalize / Preview / Export

- Teach finalize to assemble packed `N`-channel raw from fragments.
- Teach preview/export paths to display direct RGB raw and convert HSV raw.
- Update PNG, TIFF, PDF, DeepZoom, and DeepZoom-from-raw export paths for `channels == 3`.
- Reject multi-channel raw in scalar-only recolor, Color2Bilevel,
  ExtractPalette-from-step-scores, and autolevel paths.
- If channel-LUT repalette is included in the same ship, teach recolor to apply
  explicit RGB channel-LUTs to packed RGB raw. Otherwise reject with a clear
  "not supported yet" error.

Stage 5: Product UI

- Replace separate `emit` and `emit-norm` UI chips with one `emit` chip and a
  `raw` / `norm` dropdown.
- Add `const`, `dup`, `add`, `mult`, `subtract`, `ratio`, `ema`, `sin`,
  `cos`, `log`, `exp`, `pow`, and `clamp` chips.
- Add selected-line insertion with `+ before` / `+ after`.
- Add Color section with mutually-exclusive `Scalar LUT`, `RGB`,
  and `HSV` choices.
- Add direct RGB sugar/preset.
- Remove or disable score normalization for explicit-output programs.
- Hide scalar palette selection for RGB/HSV.
- Add optional RGB channel-LUT repalette controls later.
- Make logs print output count, interpretation, emit modes, and channel ranges.

Stage 6: Test / Deploy Gates

- Add workflow-contract tests for the per-channel payload shape.
- Add native docker runtime regressions for `N=1` legacy parity and `N=3` direct RGB fragments through finalize.
- Add unit tests for fragment record-size validation and metadata mismatch rejection.
- Add frontend/Playwright coverage for `emit(raw|norm|none)`, const/dup/flush/arithmetic,
  math, and clamp chips, insert-before/after, Color section interpretation
  changes, and saved-program interpretation hints.
- Run the project predeploy gate, including API manifest and workflow definition checks.

## Concrete Implementation File Map

Current code already has partial explicit-output support: native `emit` /
`emit_norm`, vector evaluation, per-channel summary rows, generic fragment
record sizing, and `channels=3` raw rendering exist. The work below is the
remaining implementation needed to match this document.

### Shared Color Contract

`lambda/color_render_contract.py` new shared helper

- Own interpretation normalization and compatibility validation:
  - `scalar_palette` -> `scalar_lut`
  - `direct_rgb` -> `rgb`
  - `scalar_lut` requires one output
  - `rgb` requires three outputs
  - `hsv` requires three outputs
- Produce normalized channel display names:
  - `score` for scalar LUT
  - `r`, `g`, `b` for RGB
  - `h`, `s`, `v` for HSV
- Produce non-fatal warnings such as HSV hue `emit(norm)`.
- Return a small contract object used by render plan, render preview, finalize,
  raw sidecar, and frontend tests.
- Do not import handler modules from this helper. Handler modules should import
  the helper, not each other.

### Compiler And Program Model

`lambda/solve_score_chain.py`

- Extend the score-chip catalog near `UNARY_CHIPS`, `COMBINE_CHIPS`, and
  `OUTPUT_CHIPS`.
- Add parser/compiler support for:
  - `["emit", "raw"]` -> native `emit`
  - `["emit", "norm"]` -> native `emit_norm`
  - legacy `["emit"]` and `["emit_norm"]` input, rewritten to the unified
    editor form on serialize
  - `["const", value]`
  - `["dup"]`
  - `["add"]`, `["mult"]`, `["subtract"]`, `["ratio"]`, `["clamp"]`
- Keep legacy `mul` behavior unchanged. New `mult` must be raw
  multiplication, not the existing clamped `mul`.
- Add finite-value validation for `const`; reject `nan`, `inf`, and `-inf`.
- Split identity helpers so source/editor identity can hash syntactically valid
  draft chains while render-artifact identity hashes the effective execution
  program.
- Add a diagnostics compiler path for editor use. It should share parser logic
  with the strict compiler but return diagnostics for stack-effect problems
  instead of raising.
- Keep the pure compiler generic. It should return output count and output
  channels, but not decide `scalar_lut` vs `rgb` vs `hsv`.
- If solve-score metadata helpers are used by saved presets, preserve optional
  top-level `recommended_interpretation`. It must not be embedded inside the
  chain JSON and must not affect `solve_score_chain_id`.

`lambda/solve_score.h`

- Add native RPN ops for `const`, `dup`, `add`, `mult`, `subtract`, `ratio`,
  `ema`, `sin`, `cos`, `log`, `exp`, `pow`, and `clamp`.
- Parse native tokens:
  - `const:<number>`
  - `dup`
  - `add`
  - `mult`
  - `subtract`
  - `ratio`
  - `ema:<alpha>`
  - `sin`
  - `cos`
  - `log`
  - `exp`
  - `pow:<exponent>`
  - `clamp`
- Preserve existing clamped `mul`, `avg`, `weighted_sum`, `abs_diff`, and
  `geometric_mean` semantics for backward compatibility.
- Implement new raw arithmetic/math as unclamped stack math. Only `clamp` and
  emit byte conversion clamp to `[0, 1]`.
- Keep `ratio` deterministic: denominator `0` or non-finite denominator returns
  `0`; non-finite results push `0`.
- Ensure stack-depth errors identify the failing token.

`lambda/solve_proximity_stats.c`

- No image interpretation should be added here.
- Reuse the updated native evaluator to summarize programs containing
  `const`, `dup`, raw arithmetic, unary math, `clamp`, and unified emit forms.
- Keep one invocation producing all `score_output_channels`.
- Keep top-level scalar fields for legacy consumers.
- Ensure channel rows report output count, emit mode, min/max, quantiles, and
  range metadata only; do not emit `rgb` or `hsv` interpretation from this
  binary.

`lambda/handler_solve_proximity.py`

- Update `_score_output_contract` and `_score_output_channels_from_summary` so
  they consume generic summary channel rows and do not infer image
  interpretation.
- Keep scalar compatibility mirrors: `score_output_clip_lo`,
  `score_output_clip_hi`, `score_output_normalize`, and
  `score_output_clip_source`.
- Preserve legacy histogram behavior for programs without explicit output chips.

### Render Planning And Workflow Payloads

`lambda/handler_render_plan.py`

- Add a selected Color interpretation input from the frontend, e.g.
  `color_interpretation`.
- Use `lambda/color_render_contract.py` for interpretation normalization,
  output-count compatibility, channel display names, and warnings.
- Reject `solve_score_normalize=true` when the compiled program has explicit
  outputs.
- For legacy scalar programs, append the implicit output mode from the checkbox
  before building the effective execution contract.
- Add the selected interpretation to the render-artifact digest and metadata.
- Add the HSV hue warning when interpretation is `hsv` and channel 0 uses
  `emit_norm`; do not reject.
- Return warnings in the render-plan response and persist them in the color
  artifact metadata.
- Rename channel display names after Color interpretation validation. Do not
  make the compiler do this.

`lambda/workflow_contracts.py`

- Thread selected interpretation and channel metadata through
  `RENDER_COLOR_RASTER_ITEM_SELECTOR`,
  `RENDER_FINALIZE_MT_FRAGMENT_MANIFEST`, and
  `RENDER_FINALIZE_MT_TASK_PAYLOAD`.
- Keep legacy scalar fields only as backward-compatible mirrors.
- Keep fragment manifest fields authoritative:
  `pair_encoding`, `fragment_encoding`, `channels`, and `record_size_bytes`.

`stepfunctions/render_workflow.asl.json.template`

- Mirror the `workflow_contracts.py` changes exactly.
- Update JSONPath selectors for any renamed fields.
- Add or keep tests that compare the template payload shape to
  `workflow_contracts.py`; do not hand-maintain divergent payloads.

### Raster And Native Fragment Production

`lambda/roots2pix_mt.c`

- It already emits vector score outputs and generic `uint32 pixel_idx + N bytes`
  fragments. Extend only as needed for new native stack ops.
- Keep `roots2pix_mt` image-agnostic. It should not know `rgb` or `hsv`.
- Keep per-channel clip arrays:
  `--score_output_clip_los` and `--score_output_clip_his`.
- Keep `--step_scores_output` and associated-palette fragment output restricted
  to one output channel.
- Ensure runtime JSON reports `fragment_channels` and
  `fragment_record_size_bytes` for finalize validation.

`lambda/handler_raster_mt.py`

- Continue building per-channel clip arrays from `score_output_channels`.
- Validate the channel array length matches the compiled/raster channel count.
- Upload fragment manifests with `channels` and `record_size_bytes`.
- Keep lag prelude behavior source-wide, not per-channel.

`lambda/assemble_greyscale.c`

- Keep generic record-size assembly for `4 + channels`.
- Ensure fragment-size validation stays strict:
  `file_size % record_size_bytes == 0`.
- Keep packed raw byte order as per-pixel channel order:
  `channel_0, channel_1, ...`.

### Finalize, Raw Metadata, And Display

`lambda/handler_finalize_mt.py`

- Normalize interpretation aliases on input.
- Validate compatibility before rendering:
  - `channels=1`, `interpretation=scalar_lut`
  - `channels=3`, `interpretation=rgb`
  - `channels=3`, `interpretation=hsv`
  - `channels=3`, `interpretation=rgb_lut`
  - `channels=3`, `interpretation=hsv_lut`
- Reject unsupported combinations with a clear error.
- Pass `channels` and `interpretation` into the raw renderer.
- Write `channels`, `raw_layout`, `interpretation`, `output_channels`,
  selected Color mode, and per-channel ranges into both sidecar JSON and color
  artifact metadata.
- Continue writing `step_scores.raw` only for scalar `channels=1`.

`lambda/raw_sidecar.py`

- Normalize interpretation names with compatibility aliases:
  - `scalar_palette` -> `scalar_lut`
  - `direct_rgb` -> `rgb`
  - `rgb-lut` -> `rgb_lut`
  - `hsv-lut` -> `hsv_lut`
- Store `channels`, `raw_layout`, `interpretation`, and `output_channels`.
- Add a shared scalar-only validation helper, or add `require_scalar=True` to
  `validate_raw_sidecar`.
- Error text should match:
  `<feature> requires a scalar (channels=1) raw artifact; got channels=N`.

`lambda/raw_score_render.py`

- Add an `interpretation` parameter to `render_score_raw`.
- Pass the interpretation to `score_raw_render`.
- Keep scalar histogram/equalization behavior restricted to `channels=1`.

`lambda/score_raw_render.c`

- Add `--interpretation=scalar_lut|rgb|hsv|rgb_lut|hsv_lut`.
- For `channels=1`, keep existing equalization + scalar LUT/palette path.
- For `channels=3`, `rgb`, keep current direct RGB packed-raw path.
- For `channels=3`, `hsv`, convert packed HSV bytes to RGB before saving
  image and preview.
- For `channels=3`, `rgb_lut`, use the three packed bytes as independent RGB
  palette-component lookup coordinates.
- For `channels=3`, `hsv_lut`, use the three packed bytes as independent HSV
  palette-component lookup coordinates.
- Reject mismatched `channels` and `interpretation` early.

### Preview Path

`lambda/handler_render_lores_preview.py`

- Accept the same selected Color interpretation as ColorRender-MT.
- Use `lambda/color_render_contract.py`; do not duplicate render-plan
  validation logic in this handler.
- Pass interpretation through summary, raster, raw assembly, and
  `render_score_raw`.
- Keep preview logs explicit: output count, interpretation, emit modes,
  channel clip ranges, and HSV hue warning if applicable.
- Return per-emission histograms built from sparse fragment records, not the
  full background-filled raw image.

`index.html`

- Include selected Color interpretation in the render-preview payload.
- Draw returned preview normally; no special frontend image interpretation is
  needed if backend returns PNG.
- Render the ephemeral preview area as tabs: `Plot`, then `E1`, `E2`, `E3`
  histogram tabs for the first three emitted channels returned by the preview
  handler.

### Scalar-Only Consumers

`lambda/color_recolor_raw.py`

- Require scalar raw for the existing Recolor-from-raw palette path.
- Use the shared scalar-only sidecar validation helper.
- Do not try to repalette `rgb` or `hsv` packed raw in v1.

`lambda/handler_recolor_from_raw.py`

- No business logic should be added here if it remains a wrapper; it should
  inherit the rejection from `color_recolor_raw.py`.

`lambda/handler_bilevel.py`

- Reject `channels != 1` immediately after raw-sidecar validation and before
  byte-looping the raw data.

`lambda/handler_extract_palette_from_step_scores.py`

- Reject `channels != 1` before checking `step_scores_key`, so direct RGB/HSV
  artifacts get a clear scalar-only error.

`lambda/handler_autolevels.py`

- Add raw-sidecar channel validation if absent on the selected path.
- Reject direct RGB/HSV raw until a channel-aware autolevel policy is designed.

### Export Paths

`lambda/handler_deepzoom_from_raw.py`

- Ensure the raw-sidecar path accepts `channels=3` with `rgb` or `hsv` and uses
  the same conversion path as final render.

`lambda/handler_deepzoom_export.py`

- If it reads image artifacts only, no raw-channel change is needed.
- If it takes `raw_key` / `raw_meta_key`, delegate to the same raw render
  interpretation logic used by `handler_deepzoom_from_raw.py`.

`lambda/handler_tiff_compat.py`

- This handler consumes a source image key and writes a preview-compatible TIFF.
  It does not read raw sidecars on the current path.
- No raw-channel conversion is needed for v1.
- Preserve/copy relevant Color metadata so derived artifacts still show the
  source interpretation.

`lambda/handler_pdf_artifact.py`

- ColorSpread consumes the final source image and optional associated palette
  image. It does not read raw sidecars on the current path.
- No raw-channel conversion is needed for v1.
- Update display text if useful so PDFs can mention `Scalar LUT`, `RGB`, or
  `HSV` source interpretation.

`lambda/handler_png_export.py`

- Current PNG export is bilevel-oriented. No Color raw change should be needed
  unless a Color raw PNG export path is added.
- If Color raw export is added later, use the same `channels` /
  `interpretation` branch as final render.

### Frontend UI

`index.html`

- Replace the current `Solve Score Palette` block with a `Color` section.
- Add mutually-exclusive Color choices:
  - `Scalar LUT` with existing palette swatches/selectors
  - `RGB` with no palette controls
  - `HSV` with no palette controls
  - `RGB LUT` with the shared palette selector
  - `HSV LUT` with the shared palette selector
- Use aligned mode rows; multi-row controls must stay vertically grouped under
  their mode.
- Disable or hide the global score-normalization checkbox when the program has
  explicit output chips.
- Keep the checkbox for legacy scalar programs and map it to implicit
  `emit` / `emit_norm`.
- Replace separate output chips with one visible `emit` chip carrying a
  `raw|norm` dropdown.
- Add `const`, `dup`, `add`, `mult`, `subtract`, `ratio`, `ema`, `sin`,
  `cos`, `log`, `exp`, `pow`, and `clamp` chips to the Add score chip popup.
- Add selected-chip state and `+ before` / `+ after` insertion. Default to
  append when nothing is selected.
- Add draft diagnostics for stack underflow, final stack mismatch, output count
  mismatch, and HSV hue normalization warning.
- Ensure `_renderCommonParams`, ColorRender-MT launch, and render preview
  payloads include the selected Color interpretation.
- If saved solve-score programs are loaded, apply
  `recommended_interpretation` as a UI hint when present.

### Saved Program Storage

`lambda/solve_score_chain.py`

- Extend `emit_solve_score_metadata` / `read_solve_score_metadata` to preserve
- optional top-level `recommended_interpretation`.
- Keep it out of `solve_score_chain_id`.
- Do not store `recommended_interpretation` inside the chain JSON.

`lambda/handler_storage.py`

- Update `/save-solve-score-program`, `/fetch-solve-score-program`, and
  `/list-solve-score-programs` handling so saved solve-score program objects
  round-trip top-level `recommended_interpretation`.
- Validate it against the known Color interpretation names if present.
- Keep it optional for old saved programs.

`tests/test_solve_score_program_storage.py`

- Add round-trip coverage for `recommended_interpretation`.
- Verify old saved programs without the field still load as before.

### Packaging And API Manifest

`deploy.sh`

- If no new API route is introduced, `api_manifest.json` should not change.
- If any deployed handler imports a new helper module, update every affected
  packaging block in the same change.
- If `lambda/color_render_contract.py` is added, package it with render plan,
  render lores preview, finalize, raw-sidecar users, and any other deployed
  handler that imports it.
- Native binary changes require rebuilding the shipped binaries in `lambda/`
  before runtime tests.

`api_manifest.json` and `api_manifest.py`

- No update is expected unless a new route/config key is added.
- If a new route or config key is added, regenerate with
  `python3 api_manifest.py --write` and verify with
  `python3 api_manifest.py --check`.

`deployment-checklist.md`

- Follow the checklist as a hard gate.
- `uv`, Playwright, `scripts/predeploy_check.sh`, and
  `scripts/test-docker-runtime.sh` require immediate escalation in this
  environment.

### Required Tests

`tests/test_solve_score_chain.py`

- Add Python compiler tests for unified emit input/output, legacy emit input,
  `const`, `dup`, raw arithmetic, `ema`, unary math, `clamp`, invalid
  constants, stack errors, source identity, execution identity, and
  `recommended_interpretation`.

`tests/test_frontend_js.sh`

- Add JS compiler/editor source checks for the same chip set.
- Assert the Color section controls exist.
- Assert `Scalar LUT`, `RGB`, `HSV`, `RGB LUT`, and `HSV LUT` are sent in render and preview
  payloads.
- Assert explicit-output programs disable/hide global score normalization.

`tests/e2e/render-solve-score.spec.js`

- Add Playwright coverage for selecting a chip line, using `+ before` /
  `+ after`, changing `emit` mode, inserting `const` / arithmetic / math /
  `clamp`, selecting RGB/HSV, and verifying execute-state diagnostics.

`tests/test_render_plan.py`

- Cover selected interpretation validation.
- Cover legacy scalar checkbox -> implicit emit mode.
- Cover explicit outputs rejecting global score normalization.
- Cover RGB and HSV metadata.
- Cover warnings array and artifact-metadata persistence.
- Cover HSV hue warning when channel 0 uses `emit_norm`.

`tests/test_render_lores_preview_handler.py`

- Cover preview payload interpretation for lores, logical, and recompute
  preview paths.
- Cover RGB and HSV preview logs.
- Cover returned per-emission histograms.
- Cover warnings array returned from the shared color contract.
- Cover incompatible output-count rejection.

`tests/test_raster_mt.py` and `tests/test_raster_mt_parity.py`

- Cover per-channel clip arrays and fragment record sizes.
- Preserve scalar byte parity for legacy programs.
- Cover new native stack ops, including EMA/math transforms, in raster output.

`tests/test_finalize_mt_handler.py`

- Cover `scalar_lut`, `rgb`, and `hsv` finalize paths.
- Cover `rgb_lut` and `hsv_lut` finalize paths.
- Cover alias normalization for old `scalar_palette` and `direct_rgb` metadata.
- Cover unsupported channel/interpretation combinations.
- Cover sidecar metadata fields.

`tests/test_recolor_from_raw.py`

- Add scalar-only rejection tests for RGB/HSV sidecars.

Tests for scalar-only consumers:

- Add or extend handler tests for `handler_bilevel.py`,
  `handler_extract_palette_from_step_scores.py`, and `handler_autolevels.py`
  to reject `channels != 1` with the shared clear error.

`tests/test_render_workflow_definition.py`

- Assert ASL payload selectors match `workflow_contracts.py` for all new or
  renamed channel/interpretation fields.

`tests/test_deploy_packaging.py`

- Assert any new helper import is packaged into every affected Lambda zip.
- Specifically assert `color_render_contract.py` is included everywhere it is
  imported.
- Assert native binaries still ship in raster/finalize/preview packages.

`tests/docker_runtime_regression.py`

- Add runtime coverage for:
  - legacy scalar render parity
  - explicit RGB render through raster -> finalize -> image
  - explicit HSV render through raster -> finalize -> image
  - new native stack ops, including EMA/math transforms, in a real binary path

Command gates before calling this ready:

```bash
uv run python -m pytest tests/test_solve_score_chain.py tests/test_render_plan.py tests/test_raster_mt.py tests/test_finalize_mt_handler.py tests/test_render_lores_preview_handler.py tests/test_recolor_from_raw.py -q
bash tests/test_frontend_js.sh
npx playwright test tests/e2e/render-solve-score.spec.js
bash scripts/predeploy_check.sh
bash scripts/test-docker-runtime.sh
```

The `uv`, Playwright, predeploy, and Docker commands need escalation in this
environment.

## Open Questions For V2

- HSV hue policy: clipped, wrapped, sawtooth, or user-selectable.
- Channel equalization: per-channel, luminance-based, or none.
- Full RGB-palette/blend recolor semantics.
- Channel-aware autolevels semantics.
- Palette mix blend space: RGB, HSV, Lab, or other.
- Whether palette-based interpretation should reference global palette controls or carry palette params.
- Whether legacy `omega_cosine` auto-append remains forever or is removed after scalar programs migrate to explicit outputs.
