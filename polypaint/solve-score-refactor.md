# Solve-score refactor

Status: open. Running list of issues to fix in the solve-score path. Issues are
added as we find them; each issue has its own section with current-state notes,
why it is wrong, and a suggested fix. Nothing here is implemented yet.

The path in scope is the `color` branch of
[stepfunctions/render_workflow.asl.json.template](stepfunctions/render_workflow.asl.json.template):

- `ColorSolveScoreClipTask` → `ColorSolveScoreHistMap` → `ColorSolveScoreMergeTask`

and the handlers they invoke in
[lambda/handler_solve_proximity.py](lambda/handler_solve_proximity.py),
[lambda/handler_palette_chunk.py](lambda/handler_palette_chunk.py),
[lambda/handler_raster_mt.py](lambda/handler_raster_mt.py),
[lambda/handler_render_plan.py](lambda/handler_render_plan.py), plus the
chain machinery in [lambda/solve_score_chain.py](lambda/solve_score_chain.py)
and the native readers in `lambda/solve_palette_chunk_mt.c`,
`lambda/solve_proximity_hist_sectioned.c`, and `lambda/roots2pix_mt.c`.

---

## Issue 1 — `quantile` and `omega` are legacy workflow globals

### Current state

Three scalar fields are threaded end-to-end through the solve-score pipeline:

- `solve_score_quantile`
- `solve_score_omega`
- `solve_score_omega_enabled`

They appear:

- in the render plan (`plan.solve_score.quantile` / `.omega` / `.omega_enabled`)
- in every Map `ItemSelector` in the solve-score branch of
  `render_workflow.asl.json.template` (clip, hist, merge, associated palette,
  raster)
- in the solve-score handlers (`handler_solve_proximity.py` clip/hist/merge,
  `handler_palette_chunk.py`, `handler_raster_mt.py`,
  `handler_palette_finalize.py`)
- as CLI flags on the native binaries (`--omega=...`,
  `--omega_enabled=...`)
- as fields written into the clip JSON, per-section hist JSON, and
  per-section palette meta JSON, and re-validated on the consumer side
- as mirrored globals for the other two solve-score roles:
  `palette_source_quantile` / `omega` and
  `associated_palette_quantile` / `omega` — see the role table in
  [lambda/solve_score_chain.py](lambda/solve_score_chain.py) at
  `SOLVE_SCORE_ROLE_PARAM_KEYS` (~L82).

### How they are actually consumed

`handler_solve_proximity.py` L592–614 (and the same pattern in the merge and
hist handlers):

```python
solve_score_quantile = _validate_quantile(contract_param(params, "solve_score_quantile", 0.001, ...))
solve_score_omega = _validate_omega(contract_param(params, "solve_score_omega", 1.0, ...))
solve_score_omega_enabled = _validate_omega_enabled(contract_param(params, "solve_score_omega_enabled", True, ...))
...
compiled = compile_chain(..., solve_score_quantile, solve_score_omega, solve_score_omega_enabled, ...)
solve_score_quantile = compiled["quantile"]
solve_score_omega = compiled["omega"]
solve_score_omega_enabled = compiled["omega_enabled"]
```

The handler reads the scalars, feeds them into `compile_chain` as fallback for
a chain that does not specify them, then immediately overwrites the scalars
with values extracted back out of the compiled chain. The fallback path in
`solve_score_chain.py::_metric_items_with_fallback` (L263) only kicks in when
the chain is a legacy one-metric-chip shape with no `q` parameter.

### Why this is wrong

The chain (and the program spec it compiles to) is already the authoritative
representation of per-chip `q` and per-chip `omega`:

- every metric chip carries its own `q`, e.g. `spread(slv,0.1%)`
- every `omega_cosine` chip carries its own `omega` (and optional phase)
- the program spec in `solve_score_chain.py::_build_program_spec` encodes both

As soon as the chain has more than one metric chip or any non-trivial
`omega_cosine` chip configuration, the scalar globals collapse the chain down
to a single `q` and a single `omega` — semantically just the first chip's
values. The scalars therefore cannot faithfully represent the chain; they are
only a faithful representation in the one-metric / zero-omega legacy case.

After the post-compile overwrite, the scalars are used for:

1. **Artifact mismatch checks.** Clip JSON writes `clip_quantile`, `omega`,
   `omega_enabled`; hist JSON writes the same; merge and downstream consumers
   re-read them and assert equality (e.g. `handler_solve_proximity.py` L262–268
   and L566–571). This is a legacy consistency gate from before chains
   existed.
2. **C-binary CLI args.** `--omega=...` and `--omega_enabled=...` are passed to
   `solve_palette_chunk_mt`, `solve_proximity_hist_sectioned`, and
   `roots2pix_mt` (`handler_solve_proximity.py` L985, L1081, L1138, L1185).
   Redundant with `--program=...`, which already encodes the chain tokens.
3. **Defaults for legacy callers.** Anything that does not pass a chain.

None of these justify scalar workflow globals. The chain is the contract.

### Suggested fix

Three steps, in order of safety — each is independently shippable:

1. **Stop threading the scalars through the workflow.** Pass only the chain
   (and the compiled program spec) into the Lambda payloads. Drop the three
   scalar fields from the Map `ItemSelector` blocks in
   `render_workflow.asl.json.template` for clip, hist, merge, associated
   palette, and raster. Do the same for the parallel roles
   (`palette_source_*`, `associated_palette_*`) driven by
   `SOLVE_SCORE_ROLE_PARAM_KEYS`. Inside the handlers, stop reading scalars
   from `params` and stop extracting them from `compiled` — use the chain
   directly.

2. **Replace the artifact mismatch fields with a chain fingerprint.** Drop
   `clip_quantile`, `omega`, `omega_enabled` from the clip, hist, and palette
   meta JSON. Write a single `chain_fingerprint` instead (a stable hash of
   the compiled program spec + sources). Consumers assert equality on the
   fingerprint. This actually catches all chain drift, not just drift in the
   first chip's `q` and `omega`.

3. **Drop `--omega=` / `--omega_enabled=` on the native binaries.** They
   already take `--program=`; the native side should parse per-chip values
   from the program tokens. Confirm each binary's `--program=` parser covers
   the omega tokens before removing the CLI flags.

### Risks and open questions

- Any external tool or saved job-state that still sends the bare scalars will
  break. Audit: saved render params on S3, test fixtures under `tests/`, and
  anything in `handler_storage.py` that persists render params.
- The contract/payload tests (`test_render_plan.py`,
  `test_solve_proximity_handler.py`, `test_palette_chunk_handler.py`,
  `test_render_workflow_definition.py`) need to be updated to assert the new
  chain-only shape.
- Step 3 requires the native chain parser to cover everything
  `solve_score_chain.py::_build_program_spec` can emit; confirm before
  removing CLI flags.

---

## Issue 2 — Greyscale `.raw` as universal render intermediate

### Scope

This issue is broader than solve-score — it reshapes the entire render
workflow. It subsumes parts of Issue 1 (the chain fingerprint still has to
exist; everything else about omega/quantile is trivially handled by the
new raster contract).

### The key observation

The current color pipeline uses three fan-outs that each read the same
hires solve per section:

- `ColorSolveScoreHistMap` — reads solves, emits per-section histograms
- `ColorAssociatedPaletteMap` — reads solves, emits per-section palette
  fragments
- `ColorRasterMap{Sparse,Dense}` — reads solves, emits per-section pixel
  fragments (already pre-colored as RGB)

Solve-score binning is, structurally, histogram equalization. What we are
currently doing is histogram equalization the hard way: compute the
histogram by fan-out, merge it, compute the CDF, read the solves a second
time to map score → bin, read the solves a third time to paint RGB. If we
instead produce a raw greyscale image and run a standard image-processing
pipeline on it at the end, we get the same *kind* of output — a
solve-score-driven image — with one fan-out and no solve re-reads. The
equalization domain change called out below means the output is not
expected to be bit-identical or visually identical to historical renders.

### Canonical artifact: `greyscale.raw`

Before the architecture, pin the contract for the new intermediate, because
it is what the whole redesign hangs off of.

- **Shape:** `width × height × 1 uchar`, row-major, no header. Compatible
  with libvips `rawload(w, h, uchar, 1 band)`.
- **Encoding:**
  - byte `0` = background, "never hit" by any solve step.
  - bytes `1..255` = clipped score byte. Mapping from raw chain score is
    `byte = 1 + round(clamp((raw - clip_lo) / (clip_hi - clip_lo), 0, 1) * 254)`.
  - The image is pre-colorization and pre-equalization. It represents the
    *clipped* score domain, discretized.
- **Sidecar:** `greyscale.meta.json` written alongside. Fields:
  - `width`, `height`
  - `chain_fingerprint` (from Issue 1, step 2) — binds this raw to the
    exact solve-score chain used to produce it
  - `clip_lo`, `clip_hi` (per metric)
  - `background_color` (plumbed through from the existing detection step)
  - `plan_params_digest` — hash of canonical JSON for viewport + grid +
    root transforms + build-time `RASTER_BINARY_SHA256`, so downstream can
    assert consistency
- **Storage:** S3 object under the render family's artifact prefix. Sized
  per the Memory and size budget table below.
- **Immutability:** once written, the raw is addressable and reusable.
  Repalette, deepzoom, and alternate encoders consume it without
  recomputing any solve-side work.

### Equalization domain — explicit change

The current solve-score pipeline equalizes over the **solve step
histogram** (distribution of scores across all solve steps, across
sections). The new design equalizes over the **greyscale pixel
histogram** (distribution of score bytes across the final image pixels).

These are not the same distribution. Steps-per-pixel varies with viewport
geometry: dense regions of the image sample many steps per pixel, sparse
regions sample few. The old pipeline gave equal weight to each step; the
new pipeline gives equal weight to each pixel. With "any arrival wins" at
assembly, a dense pixel takes an essentially random sample from its steps.

This is a real aesthetic change — intentional, not incidental. In
practice the new behavior is closer to what a user expects from
"histogram equalize this image": the visible output's tonal distribution
is flattened, not the solve space's. This is the product decision we are
making. If it produces visibly different renders from historical ones,
that is accepted.

### Proposed architecture

Three stages in the hot path. The key point is that **the hot path fuses
assembly and colorization into a single Lambda** — no upload-then-download
of the raw on first render. Separate recolor / deepzoom-from-raw handlers
exist for reuse, not for the initial pass.

```
Plan → Clip → RasterMT → FinalizeMT
                           ├── assemble greyscale.raw on /tmp
                           ├── build 256-entry histogram + equalization LUT
                           ├── apply LUT + palette LUT → RGB encode
                           ├── (optional) DeepZoom tiles from same file
                           └── upload: greyscale.raw + sidecar + RGB + preview
```

Sibling handlers for later reuse:

- `recolor_from_raw` — downloads `greyscale.raw` + sidecar, builds LUT
  from a new palette spec, writes new RGB output. One invocation per
  repalette.
- `deepzoom_from_raw` — downloads raw + sidecar, runs deepzoom pyramid
  after the equalize + LUT. Runs on historical artifacts or when
  deepzoom is requested after the fact.

These sibling handlers are functionally subsets of FinalizeMT; they exist
for the case where the raw already exists on S3 and we want to reuse it.

#### 1. Clip

Unchanged. Single Lambda on the lores solve, computes per-metric threshold
values. Output is small JSON with the clip bounds and a chain
fingerprint (see Issue 1, step 2).

#### 2. RasterMT (per-section fan-out)

Each section worker:

- Reads its slice of the hires solve via the native multispan reader
  (`bin` + `coeffs` + `params` as the chain requires).
- Evaluates the compiled solve-score chain per step.
- Applies clip bounds per the `greyscale.raw` encoding contract above.
- Emits sparse fragments `(pixel_idx, score_byte)` per step.

No accumulation. Multiple steps landing the same pixel resolve by
"any arrival wins" at assembly time — explicitly tolerated, not a bug,
and NOT dependent on the current first-claim logic in
`lambda/roots2pix_mt.c` (that logic is incidental and does not constrain
the redesign). No palette, no bin mapping, no RGB.

This fan-out replaces both the current `ColorSolveScoreHistMap` and the
current `ColorRasterMap{Sparse,Dense}`. The solve-score hist-map and merge
stages disappear entirely.

#### 3. FinalizeMT (single fat Lambda, fused assemble + encode)

Thin Python handler, C binary for the assemble hot loop, libvips for the
LUT + encode tail. One Lambda invocation; the raw is produced, consumed,
and emitted (alongside the encoded outputs) without ever leaving the
Lambda's `/tmp` between assemble and encode.

Python handler:

1. Parse payload; resolve section fragment manifest.
2. `subprocess.run(['./assemble_greyscale', ...])` — produces
   `/tmp/greyscale.raw` and updates `/tmp/greyscale.meta.json`.
3. Build the 256-entry histogram + equalization LUT (see "Why not
   `hist_equal`" below). This is a trivial pass over the raw or can be
   done by the C binary as a side output.
4. Dispatch by output family (Python switch):
   - `color`: libvips `rawload → maplut(equalize_lut) → maplut(palette_lut256x3) → jpegsave/pngsave`.
   - `bilevel`: libvips `rawload → maplut(threshold_lut) → pngsave` (1-bit).
   - `deepzoom`: libvips `rawload → maplut(equalize) → maplut(palette) → dzsave`.
   - `color + deepzoom` (parallel combo): run both libvips pipelines
     sequentially from the same `/tmp/greyscale.raw` — both are streaming,
     total time is additive but inexpensive.
5. Upload outputs to S3:
   - `greyscale.raw` + `greyscale.meta.json` (always, for reuse)
   - encoded outputs per dispatched mode
   - preview

C binary `assemble_greyscale` — single-purpose, bat-out-of-hell:

- `mmap` a `width × height × 1 uchar` file on `/tmp` (zero-initialized).
- Spawn N threads matched to Lambda vCPU count (typically 6–8 at 10 GB
  memory configuration).
- Shared section queue; each thread pulls sections, does ranged GETs via
  thread-local curl (same pattern as `lambda/multispan_reader.c`), parses
  `(pixel_idx, byte)` pairs, does direct stores `buf[idx] = byte`.
- **No locks.** Byte stores are atomic on x86_64 and ARM64 — races on
  the same pixel resolve to one write cleanly, matching the "any arrival
  wins" semantic.
- Optional side output: write the 256-entry byte histogram as the final
  scan pass before exit, so the Python side does not need a second pass.
- Flush mmap, exit.

No histogram math beyond the optional 256-bin counter, no palette, no
color logic, no format conversion. Dedicated to assembly.

### Why not `hist_equal`

The raw is an 8-bit image — 256 possible values. There is no reason to
use a general-purpose histogram-equalization library for something
this small.

- **Histogram:** one pass over the raw, 256-entry counter array. A few
  lines of C or Python — integrate into the C binary as a free side
  output, or do it in Python after the subprocess returns.
- **Equalization LUT:** CDF of the non-zero bins, scaled to `[1, 255]`;
  `lut[0] = 0` preserves background. 256 floating-point operations total.
- **LUT application:** libvips `maplut` on the raw image — one streaming
  scan, produces the equalized image lazily.
- **Palette LUT:** a second `maplut` with a `256 × 3` byte table. `lut[0]`
  holds `background_color`; all other entries come from the active
  palette. Also streams.

So libvips' role narrows to `rawload` + two `maplut`s + encode. We keep
libvips for what it is good at (zero-copy streaming image I/O, tile
pyramids), not for histogram arithmetic on 256 bins. This avoids any
dependency on libvips-specific masked-histogram idioms, which vary in
ergonomics between libvips versions.

### Color-encode pipeline in detail

```
# Python side (or C side-output), one-time:
hist[0..255] = count of each byte in greyscale.raw
eq_lut[0] = 0                               # preserve background
for each non-zero bin i in 1..255:
  eq_lut[i] = 1 + round(254 * CDF(hist[1..i]) / CDF(hist[1..255]))

# libvips side, streaming:
rawload(greyscale.raw, w, h, uchar, 1 band)
  → maplut(eq_lut)                          # equalize foreground, bg stays 0
  → maplut(palette_lut256x3)                # palette_lut[0] = background_color
  → jpegsave(out_key, Q=plan.params.quality)
```

One scan if the histogram was computed during assembly; otherwise two
scans (one to count, one to encode). The full RGB image never exists as
a dense matrix.

### Background semantics

Byte 0 is reserved for "never hit". Raster discretizes into `[1, 255]`.
The equalization LUT preserves `lut[0] = 0`, so background bytes remain
`0` through the equalize step. `palette_lut[0] = background_color`
paints them in the final color pass. No masking primitives required.

### Memory and size budget

Only the greyscale needs to be resident during assembly. The encode tail
is a libvips streaming pipeline, so it does not materialize the full RGB
in memory.

Raw file sizes on `/tmp`:

| side   | greyscale `/tmp` footprint |
|--------|----------------------------|
| 10K    | 100 MB                     |
| 20K    | 400 MB                     |
| 30K    | 900 MB                     |
| 50K    | 2.5 GB                     |

Lambda `/tmp` caps at 10 GB, and the RSS of the assemble binary when the
buffer is `mmap`'d stays low (the kernel pages dirty regions as needed).
But `/tmp` also holds:

- the encoded RGB output file (streaming encode writes as it produces —
  peak file size ~10–40% of the raw for JPEG at reasonable quality,
  larger for PNG)
- the preview file
- deepzoom tile pyramid files if that encoder runs — can add up across
  many small files
- libvips scratch for multi-band / multi-operation pipelines
- operational slack for OS, logs, lambda runtime, subprocess overhead

**Realistic safe ceiling:** single-Lambda FinalizeMT is comfortable up to
**~30K per side** on a 10 GB `/tmp` Lambda with the full color + preview
path. 40K is feasible if deepzoom is not also running in the same
invocation. 50K is the point where `/tmp` budgeting becomes tight and
worth a pre-flight check. Beyond that, the design needs a strip/tile
assembly variant (see "Open questions" below) — assemble the raw in
horizontal strips, stream each strip to S3 as it completes, and encode
by concatenating strips server-side or via a second Lambda that reads
strips ranged. This is a known scale-out path, not a blocker for the
sizes we currently render (well under 20K typical).

The quoted 100K @ 10 GB figure in earlier drafts is not safe and has been
removed — it leaves no headroom for encoded outputs or operational slack
and is not supported by this design without the strip/tile extension.

### What this eliminates

- `ColorSolveScoreHistMap` — gone (replaced by the explicit 256-bin raw
  histogram pass during or after assembly).
- `ColorSolveScoreMergeTask` — gone (fan-in of section histograms no
  longer exists).
- `ColorAssociatedPaletteMap` + `ColorAssociatedPaletteFinalizeTask` —
  converges to the same `color_encode` Lambda operating on a smaller
  viewport greyscale (the palette image). Could be merged with
  `color_encode` or kept as a sibling.
- `BilevelRasterPhase` + `BilevelMergeMap` + `BilevelStitchTask` —
  collapses into the same RasterMT + AssembleMT + `bilevel_encode` chain.
  Bilevel stops being its own mode-specific render path.
- `CoeffRasterPhase` + `CoeffMergeMap` + `CoeffStitchTask` — same
  collapse, with a coeffs-driven RasterMT variant feeding the same
  AssembleMT.
- `ColorRasterMap{Sparse,Dense}` — collapses into one shape of fan-out.
  `pixel_bin_fragment_mode` goes away.
- `ColorFinalizeMap` — replaced by the single AssembleMT Lambda.

The `ModeChoice` state is absorbed into FinalizeMT in the target end
state. During migration, a top-level workflow choice may remain only to
route legacy bilevel / coeff_bilevel requests to their old branches. The
target render workflow ends up with one raster shape, one assemble shape,
and a thin Python dispatch inside FinalizeMT.

### Free wins

- **Repalette becomes trivial.** Save `greyscale.raw` alongside the RGB
  artifact; repalette is one call to `color_encode` with a different LUT.
  No resolve, no reraster, no reassembly. Today's repalette re-runs the
  whole raster because colors are baked into fragments.
- **Associated palette image** is a second inline encode over the
  pass-0 `N × N` palette greyscale — same code path, no separate fan-out.
- **Saved-palette artifacts** compress to `(greyscale.raw + lut256x3.bin + meta)`.
  Compact, reproducible, cheaply recomposable.
- **DeepZoom stops being a separate render family.** Same raster + assemble
  upstream; different encoder.

### Scope caveats

- **Rainbow color mode** fits this model naturally — hue is a scalar, so
  discretize angle to byte, use a hue-ramp LUT. No structural change.
- **Root-proximity color mode** carries two bands of information per
  pixel (`root_idx`, `distance`). Raster would emit a 2-band raw
  (`uchar × 2`), libvips handles multi-band natively. Minor extension,
  not a blocker. Lower priority than solve-score.
- **Other non-scalar per-pixel outputs** (if added later) would need
  additional bands or separate encoders. Not in scope now.

### Handler and binary work

- New C binary `assemble_greyscale` — mirrors shape of
  `lambda/multispan_reader.c` + existing sectioned readers. Thread-local
  curl, shared section queue, sparse-fragment parser, direct stores.
  Optional side output: 256-entry byte histogram.
- New handler `handler_finalize_mt.py` — fused assemble + encode. Python
  handler orchestrates subprocess → LUT build → libvips pipeline (mode
  dispatched) → upload of raw + sidecar + encoded outputs. Replaces the
  existing `handler_finalize.py` for color renders and eventually for
  bilevel / coeff_bilevel / deepzoom as they migrate over.
- New sibling handlers for reuse of the raw:
  - `handler_recolor_from_raw.py` — downloads raw + sidecar, re-runs
    LUT-build + libvips encode for a different palette. One invocation
    per repalette.
  - `handler_deepzoom_from_raw.py` — downloads raw + sidecar, runs
    libvips `dzsave` with equalize + palette LUT. For on-demand deepzoom
    on historical artifacts or as a separate workflow step.
- RasterMT binaries updated to emit `(idx, byte)` sparse fragments
  instead of RGB; drop palette application from `roots2pix_mt.c`.
- Workflow: `render_workflow.asl.json.template` reshaped. `ModeChoice`
  moves from *pre-raster* to *post-FinalizeMT* (or is absorbed into
  FinalizeMT as a Python dispatch, eliminating a Step Functions state
  entirely). The solve-score hist map, solve-score merge, associated
  palette chunk, associated palette finalize, bilevel raster/merge/stitch,
  coeff raster/merge/stitch, and color finalize map states collapse.

### Risks and open questions

- **Background color persistence.** Currently emitted into
  `plan.outputs.metadata.background_color` by a detection step. Must be
  written into the `greyscale.meta.json` sidecar so downstream encoders
  (including `recolor_from_raw`) paint background without having to
  redetect.
- **Clip bounds on the hires from lores thresholds.** Pre-existing
  assumption; confirm it still holds when the hires solve distribution
  differs significantly from lores. Not a regression, but worth testing
  under the new pipeline — especially because the equalization domain
  changes to pixel-space, which may expose clip-asymmetry that was
  previously absorbed by the step-space histogram fan-out.
- **Equalization domain change.** Called out explicitly in the section
  above. Named as an accepted aesthetic/product change, not a bug. Should
  be validated against reference renders before committing.
- **Compatibility with existing saved artifacts.** Old render outputs
  will not have a `greyscale.raw`. Repalette of historical artifacts
  falls back to a full re-render until they are re-rendered once under
  the new pipeline. Document this as part of the migration plan.
- **Chain fingerprint sidecar.** The fingerprint from Issue 1, step 2,
  must be written into `greyscale.meta.json` and asserted by
  `recolor_from_raw` and `deepzoom_from_raw`. Prevents silently
  recoloring a raw that was produced by a different chain than the
  caller thinks.
- **Strip/tile assembly for >50K images.** Not in scope for the first
  cut, but the plan should anticipate the extension path: assemble and
  encode in horizontal strips of `width × strip_height × 1` bytes,
  stream each strip through libvips incrementally. This keeps `/tmp`
  bounded and scales arbitrarily. Worth stubbing out the strip boundary
  in the C binary's interface so the extension is additive later.

### Fragment format

Raster-to-assemble wire format, pinned. Every RasterMT worker writes one
S3 object per section following this layout.

**Pair encoding.** Each emitted pair is 5 bytes, little-endian:

| field       | size    | meaning                                        |
|-------------|---------|------------------------------------------------|
| `pixel_idx` | `uint32`| row-major index into the `width × height` grid |
| `score_byte`| `uint8` | clipped score byte in `[1, 255]` (never 0)     |

No headers, no per-pair framing. A fragment object is a concatenation of
`N × 5` bytes, where `N` is the number of solve steps in the section that
produced a pixel inside the viewport. Object length modulo 5 must be 0;
anything else is a corruption signal.

**Ordering.** Pairs within a fragment are emitted in solve-step order.
The assembler does not rely on order (any-arrival-wins), but preserving
step order keeps the output reproducible up to race-resolution of
pixel-aliased steps, which helps debugging.

**Validity.** `pixel_idx < width * height` is required. `score_byte == 0`
is reserved for background and MUST NOT appear in a fragment. The
assembler rejects fragments violating either invariant — these indicate a
raster-side bug, not a tolerable race.

**Chunking and sizing.** Fragments are single S3 objects per section,
sized by the worker that produced them. Typical sections emit
`step_count ~= 10^5 to 10^7` pairs, so fragment sizes are ~0.5 MB to
~50 MB. No intra-fragment chunking; the multispan reader handles sectioned
reads naturally via ranged GETs on these objects.

**Compression.** None in the first cut. Rationale: the payload is
`(uint32, uint8)` pairs with no inherent compressibility (indices are
scattered, bytes are effectively random within the valid range). LZ4 gets
you single-digit percent savings at best on this data and adds a decode
pass on the assembler hot loop. Revisit only if S3 bandwidth becomes the
bottleneck — unlikely, since the multispan reader saturates network well
before the parser does.

**No section-end marker.** Each fragment is its own S3 object with a
known length; the reader knows when the section is done from the HTTP
`Content-Length`. No sentinel pairs, no trailer. Simpler than a stream
format.

**Manifest.** FinalizeMT receives a manifest JSON shaped like the
existing `solve_source_manifest` (see `lambda/logical_sections.py`), with
one entry per section:

```json
{
  "fragments": [
    {
      "section_idx": 0,
      "key": "renders/job_x/section_0.frag",
      "url": "https://presigned.example/section_0.frag?...",
      "size": 12345680
    },
    ...
  ],
  "width": 20000,
  "height": 20000,
  "chain_fingerprint": "...",
  "clip_lo": ...,
  "clip_hi": ...,
  "background_color": [r, g, b]
}
```

The manifest carries the same clip/fingerprint/background fields that end
up in `greyscale.meta.json` so FinalizeMT can write the sidecar without
extra S3 reads. `url` is a presigned HTTPS URL suitable for ranged GETs by
the C binary; `key` is retained for logging, error reporting, and
diagnostics. Do not pass bare `s3://...` URIs into the assembler hot loop.

### Associated palette as an inline second output

The associated palette image is the same pipeline as the main image, run
on a different coordinate function. It does NOT need its own RasterMT
fan-out. It piggybacks on the main raster pass.

#### Why it works

At every solve step the raster worker already has in hand:

- the **root cluster** — complex-plane coords, mapped to main image
  pixels via the viewport transform
- the **global solve-step index** — which already identifies one unique
  pass-0 palette cell in the `N × N` serpentine parameter grid
- the **score** — byte output of the clipped chain, identical for both
  images

The score is a property of the solve step, not of the output it is being
painted into. The same byte is valid to emit to both the main-image
pixel and the palette-image pixel in the same loop iteration.

#### Palette coordinate mapping

The associated palette keeps the existing product semantics:

- image size is exactly `N × N`
- only **pass 0** contributes to the palette image
- palette pixel identity is the pass-0 serpentine sweep position, not a
  transformed param-space resampling

For a pass-0 global solve-step index `g`:

```
row = g / N
j   = g % N
col = (row is odd) ? (N - 1 - j) : j
palette_pixel_idx = row * N + col
```

This exactly matches the current pass-0 palette artifact semantics
already implemented by the legacy palette finalize path. The fused
refactor changes **where** the palette is produced, not **what** palette
image it means.

`times > 1` does not expand the palette image. The palette image remains
pass-0-only in this refactor. Later passes do not emit palette
fragments.

#### Two fragment streams from one raster pass

When `render_execution.save_associated_palette=true`, the RasterMT worker
emits two fragment objects per section, each following the documented
`u32le_u8_v1` pair encoding:

- `fragments/section_{idx}.frag` — main image, root-space coord
- `palette_fragments/section_{idx}.frag` — palette image, pass-0
  serpentine `N × N` coord

Both streams share:

- the same pair-encoding contract
- the same `score_byte == 0` is reserved rule
- the same any-arrival-wins race tolerance semantics
- the same section count and indexing — one-to-one section correspondence

Both streams MUST be emitted in the same raster pass. There is no mode
in which the palette fragments are produced by a second raster invocation.

#### FinalizeMT consumes both streams inline

When the payload includes a `palette_fragment_manifest`, FinalizeMT
assembles a second greyscale buffer in the same Lambda invocation:

- `greyscale.raw` — `width × height × 1`, main image
- `palette.greyscale.raw` — `N × N × 1`, palette image

Both raws are equalized and LUT-encoded in the same Lambda, each with
its own independent equalization curve. Each raw gets its own sidecar:

- `greyscale.meta.json` — for the main image
- `palette.greyscale.meta.json` — for the palette image

Outputs uploaded by the same Lambda invocation:

- main: `image.{jpeg|png}`, `preview.png`, `greyscale.raw`,
  `greyscale.meta.json`, `meta.json`
- palette: `palette_image_key`, `palette_preview_key`,
  `palette_raw_key`, `palette_raw_meta_key`, `palette_meta_key`

The palette is NOT a sibling `ColorAssembleEncodeTask`. It is a
second inline output of the primary FinalizeMT invocation.

#### What it saves compared to the prior plan

The earlier text in this doc described the associated palette as a
second sibling `ColorAssembleEncodeTask` / FinalizeMT invocation with a
smaller viewport. That plan paid:

- a second RasterMT fan-out (N section workers at MaxConcurrency=10)
- a second set of per-section multispan solve-data reads
- a second AssembleMT + encode Lambda invocation

Piggybacking instead pays:

- ~2× fragment byte volume per section (main + palette pairs, small)
- ~2× fragment-manifest object count (one extra object per section)
- one additional palette raw buffer in FinalizeMT:
  `N × N × 1` bytes. Typical sizes are therefore:
  - `1000 × 1000 = 1.0 MB`
  - `5000 × 5000 = 25.0 MB`
  Still small relative to the main image buffer.

Zero additional Lambda invocations. Zero additional solve-data reads.

#### Semantic cleanups this unlocks

- **`root_transforms` do not apply to the palette.** They transform
  roots before painting. The palette is keyed on the pass-0 parameter
  grid, so
  `root_transforms` are correctly ignored. Remove the asymmetry that
  previously made the associated-palette code mirror these transforms.
- **Viewport rotation does not apply to the palette.** Same reason.
- **Background is vestigial in the palette.** Pass-0 `N × N` serpentine
  coverage visits every palette pixel exactly once, so essentially every
  palette pixel is hit. `byte == 0` handling stays for contract symmetry
  but rarely fires.
- **Independent equalization curves.** Main image and palette each
  equalize over their own pixel distribution. This is correct: they
  represent different distributions and should not share a curve.

#### Contract implications

Changes to the existing schemas elsewhere in this doc:

- **`fragment_manifest`** (Fragment format section) — unchanged. Applies
  to the main image.
- **`palette_fragment_manifest`** — new. Same shape as
  `fragment_manifest`, with `width` / `height` replaced by
  `N` / `N`. Present in the FinalizeMT payload
  only when `render_execution.save_associated_palette=true`.
- **`outputs` block** (FinalizeMT payload schema) — adds:
  - `palette_image_key`
  - `palette_preview_key`
  - `palette_raw_key`
  - `palette_raw_meta_key`
  - `palette_meta_key`
  These replace the prior `associated_palette.*` sibling-invocation
  fields. The `associated_palette.enabled` boolean stays and mirrors
  `render_execution.save_associated_palette` per the precedence rule
  already documented.
- **Response body** — adds:
  - `palette_image_key`
  - `palette_preview_key`
  - `palette_raw_key`
  - `palette_raw_meta_key`
  - `palette_meta_key`
  And the `timings` block grows:
  - `palette_assemble_ms`
  - `palette_histogram_ms`
  - `palette_encode_ms`
- **`greyscale.meta.json` schema** — applies unchanged to both the
  main raw and the palette raw. For the palette, `plan_params_digest`
  still captures `viewport`, `grid`, `root_transforms`,
  `raster_binary_sha256` even though root_transforms and viewport are
  not visually applied to the palette itself — they remain part of the
  render-identity that produced these params.
- **Key naming contract** — adds:
  - `palette_fragment_prefix = {artifact_prefix}palette_fragments/section_`
  - `palette_image_key = {artifact_prefix}palette_image.{jpeg|png}`
  - `palette_preview_key = {artifact_prefix}palette_preview.png`
  - `palette_raw_key = {artifact_prefix}palette.greyscale.raw`
  - `palette_raw_meta_key = {artifact_prefix}palette.greyscale.meta.json`
  - `palette_meta_key = {artifact_prefix}palette.meta.json`
  Palette outputs live under the same `artifact_prefix` as the main
  artifact, not under a separate `palettes/` namespace. This mirrors the
  "raw lives with the artifact" rule already documented.

#### Test assertions

Additions to the test file ownership map:

- `tests/test_raster_mt.py`
  - asserts two fragment streams are emitted when
    `save_associated_palette=true`
  - asserts palette fragment keys use
    `palette_fragment_prefix + section_{idx}.frag`
  - asserts palette pair encoding matches `u32le_u8_v1` identically to
    the main stream
  - asserts main and palette fragments come from the same raster
    invocation (same section count, same section indices)
- `tests/test_fragment_format.py`
  - extends pair-encoding invariants to cover palette fragments
    (bounds-checked against `N * N`)
- `tests/test_finalize_mt_handler.py`
  - asserts the second greyscale is produced inline, not via a separate
    Lambda invocation
  - asserts the palette sidecar matches the documented schema
  - asserts the palette RGB output exists and decodes cleanly
  - asserts `timings.palette_*_ms` fields are populated
- `tests/test_finalize_mt_reference.py`
  - locks in a palette reference image for each reference job
  - image-diff of the palette output against its baseline, same
    tolerance policy as the main image

#### Migration note

The prior doc text that described the associated palette as a "sibling
`ColorAssembleEncodeTask` / FinalizeMT invocation with a smaller
viewport" is superseded by this section. Any older wording that
describes the inline palette as `1024×1024` / `2048×2048` param-space
resampling is also superseded. The inline palette output in this
refactor uses the existing pass-0 `N × N` serpentine palette semantics.

### Workflow ownership

Which ASL states are added, deleted, and renamed, and who owns each.

**Added states:**

- `ColorAssembleEncodePhase` — phase marker before FinalizeMT.
- `ColorAssembleEncodeTask` — FinalizeMT Lambda invocation. Takes the
  fragment manifest + viewport + palette + output family from the plan;
  returns encoded output keys.

**Deleted states** (no replacement needed unless noted — all subsumed by
FinalizeMT or its surrounding flow):

- `ColorSolveScoreClipPhase` — NOT deleted in the first cut. Keep the clip
  phase marker and clip task as they are. A future optimization might fold
  clip into RasterMT's first pass, but that is explicitly out of scope for
  this refactor.
- `ColorSolveScoreHistPhase`, `ColorSolveScoreHistMap`,
  `ColorSolveScoreMergePhase`, `ColorSolveScoreMergeTask` — deleted.
  Histogram is built inside FinalizeMT as a 256-bin pass over the raw.
- `ColorAssociatedPalettePhase`, `ColorAssociatedPaletteMap`,
  `ColorAssociatedPaletteFinalizePhase`, `ColorAssociatedPaletteFinalizeTask`
  — deleted. Associated palette image is a second inline output of the
  primary FinalizeMT invocation over the pass-0 `N × N` palette grid.
- `ColorRasterItemsChoice`, `ColorRasterMapSparse`, `ColorRasterMapDense`
  — collapse to one `ColorRasterMap`. The `pixel_bin_fragment_mode`
  choice goes away; there is only one fragment format.
- `ColorFinalizePhase`, `ColorFinalizeMap` — replaced by the single
  FinalizeMT state.
- `BilevelRasterPhase`, `BilevelRasterMap`, `BilevelMergePhase`,
  `BilevelMergeMap`, `BilevelStitchPhase`, `BilevelStitchTask`,
  `BilevelPreviewTask`, `ReportDoneBilevel` — deleted; replaced by the
  color pipeline with `family=bilevel` dispatched inside FinalizeMT.
- `CoeffRasterPhase`, `CoeffRasterMap`, `CoeffMergePhase`, `CoeffMergeMap`,
  `CoeffStitchPhase`, `CoeffStitchTask`, `CoeffPreviewTask`,
  `ReportDoneCoeffBilevel` — deleted; same collapse with a coeffs-driven
  RasterMT variant.
- `ModeChoice` at workflow entry — kept only during migration while
  bilevel / coeff_bilevel still have legacy branches. Target end state:
  deleted, replaced by a Python dispatch inside FinalizeMT.

**Renamed / repurposed states:**

- `ColorEncodePhase` + `ColorEncodeTask` — deprecated. The encode happens
  inside FinalizeMT. Keep the phase marker if useful for UI visibility,
  otherwise delete.
- `ColorPreviewTask` — stays, but reads from the FinalizeMT output.
- `ReportDoneColor`, `ReportDoneBilevel`, `ReportDoneCoeffBilevel` —
  collapse to a single `ReportDone` since the output family is now a
  payload field, not a workflow branch.

**Lambda ARN ownership** (`deploy.sh` / CloudFormation):

- `FinalizeMTFunctionArn` — new Lambda, provisioned at 10 GB memory,
  `/tmp` sized to 10 GB. Packaged with libvips layer + the new
  `assemble_greyscale` C binary. Owns the fused assemble + encode path.
- `RecolorFromRawFunctionArn` — new Lambda, smaller (1–2 GB). Packaged
  with libvips layer. No assemble binary needed.
- `DeepzoomFromRawFunctionArn` — new Lambda. Packaged with libvips layer
  including `dzsave` dependencies.
- Existing `BilevelFunctionArn`, `BilevelStitchFunctionArn`,
  `FinalizeFunctionArn` — removed after migration completes.

**Ownership of the mode dispatch.** Absorb into FinalizeMT as a Python
switch on `plan.outputs.family`. Rationale: the dispatch is a few lines
of Python calling different libvips pipelines on the same in-memory raw;
exposing it as a Step Functions Choice state buys nothing and costs an
extra state transition plus a second Lambda cold start. The `ModeChoice`
state at the top of the workflow stays only as long as we need to route
legacy renders through the old bilevel / coeff_bilevel paths during
migration.

### Frontend operator control

The migration flag for the color render path MUST NOT remain a hidden
plan-only switch. It needs to be surfaced directly in the operator UI,
specifically in `Render -> Color -> Generate-MT`.

The required UI shape mirrors the compute popup:

- A `Classic` tab — routes to the legacy color render path
  (`ColorFinalizePhase` / `ColorEncodeTask`) while that path still exists.
- A `Fused` tab — routes to the new `FinalizeMT` path
  (`plan.render_execution.color_pipeline == "fused"` during migration).

Both tabs stay live in the same modal during migration so the operator can
switch between them directly and compare the old and new render execution
paths side by side, without editing hidden flags or using separate entry
points.

Rules:

- The tab choice is the user-facing contract; raw `plan.render_execution`
  strings are internal.
- The `Classic` and `Fused` tabs each own only the controls relevant to
  their path, following the same UI rule as the compute popup.
- The fused tab is where the new raw/FinalizeMT path lives; it is not a
  hidden replacement behind the existing classic controls.
- During Phase 2 and Phase 3, both tabs remain available. Only after the
  legacy path is fully retired may the classic tab be removed.

### Exact `render_execution` contract

The repo already uses `plan.render_execution` as an object. This refactor
must keep that shape and extend it, not replace it with a scalar. The
exact internal selector is:

```json
{
  "color_pipeline": "classic" | "fused"
}
```

`color_pipeline` is the only field the workflow uses to choose between the
legacy color branch and the new FinalizeMT branch.

The exact `render_execution` object shape after this refactor is:

```json
{
  "color_pipeline": "classic",
  "raster_engine": "mt",
  "save_associated_palette": true,
  "solve_score_hist_input_mode": "sectioned",
  "raster_mt_threads": 4,
  "solve_score_threads": 4,
  "solve_score_hist_retries": 2,
  "raster_input_mode": "sectioned",
  "raster_sectioned_retries": 2,
  "raster_section_mode": "logical_sections_auto",
  "raster_section_count": "",
  "raster_section_count_auto": 8,
  "pixel_bin_fragment_mode": "sparse_chunks",
  "raster_bin_group_size": "",
  "solve_score_merge_workers": 16,
  "finalize_workers": 16,
  "palette_chunk_threads": 4,
  "palette_chunk_input_mode": "sectioned",
  "palette_chunk_retries": 2,
  "palette_chunk_workers": 16,
  "solve_score_section_mode": "logical_sections_auto",
  "solve_score_section_count": "",
  "solve_score_section_count_auto": 8,
  "palette_section_mode": "logical_sections_auto",
  "palette_section_count": "",
  "palette_section_count_auto": 8
}
```

Rules:

- `color_pipeline` is required for color renders once Phase 2 lands.
- Valid values:
  - `"classic"` = legacy `ColorFinalizePhase` / `ColorEncodeTask`
  - `"fused"` = new `FinalizeMT`
- For non-color families during migration, the field may be absent or
  ignored. Once Phase 4 lands, it may be propagated uniformly.
- The object keeps the existing keys for metadata continuity. The fused
  path ignores legacy-only knobs that no longer apply.

Field usage by path:

- `classic` path reads:
  - `solve_score_threads`
  - `solve_score_hist_input_mode`
  - `solve_score_hist_retries`
  - `solve_score_section_mode`
  - `solve_score_section_count`
  - `solve_score_section_count_auto`
  - `solve_score_merge_workers`
  - `raster_mt_threads`
  - `raster_input_mode`
  - `raster_sectioned_retries`
  - `raster_section_mode`
  - `raster_section_count`
  - `raster_section_count_auto`
  - `pixel_bin_fragment_mode`
  - `raster_bin_group_size`
  - `finalize_workers`
  - `save_associated_palette`
  - `palette_chunk_threads`
  - `palette_chunk_input_mode`
  - `palette_chunk_retries`
  - `palette_chunk_workers`
  - `palette_section_mode`
  - `palette_section_count`
  - `palette_section_count_auto`

- `fused` path reads:
  - `raster_mt_threads`
  - `raster_input_mode`
  - `raster_sectioned_retries`
  - `raster_section_mode`
  - `raster_section_count`
  - `raster_section_count_auto`
  - `finalize_workers`
  - `save_associated_palette`

- `fused` path ignores:
  - `solve_score_threads`
  - `solve_score_hist_input_mode`
  - `solve_score_hist_retries`
  - `solve_score_section_mode`
  - `solve_score_section_count`
  - `solve_score_section_count_auto`
  - `solve_score_merge_workers`
  - `pixel_bin_fragment_mode`
  - `raster_bin_group_size`
  - `palette_chunk_threads`
  - `palette_chunk_input_mode`
  - `palette_chunk_retries`
  - `palette_chunk_workers`
  - `palette_section_mode`
  - `palette_section_count`
  - `palette_section_count_auto`

The frontend tab split maps exactly to this:

- `Classic` tab writes `color_pipeline = "classic"`
- `Fused` tab writes `color_pipeline = "fused"`

Normalization rule:

- Frontend requests may omit fields that are irrelevant to the active tab.
- `handler_render_plan.py` is responsible for normalizing that sparse
  request into the full stored `render_execution` object shape shown above
  before persisting metadata or emitting the workflow payload.

### Exact `FinalizeMT` payload schema

The new Step Functions task `ColorAssembleEncodeTask` invokes
`handler_finalize_mt.py` with the following exact payload body.

```json
{
  "phase": "finalize_mt",
  "job_id": "compute_xxxxxxxx",
  "run_id": "run_123",
  "task_id": "render_run_123_finalize_mt",
  "mode": "color",
  "viewport": {
    "center_re": 0.0,
    "center_im": 0.0,
    "scale": 1024.0
  },
  "grid": {
    "pix": 4096,
    "tile_size": 4096,
    "n_tile_cols": 1,
    "n_tile_rows": 1,
    "n_tiles": 1
  },
  "render_execution": {
    "color_pipeline": "fused",
    "raster_engine": "mt",
    "raster_mt_threads": 4,
    "raster_input_mode": "sectioned",
    "raster_sectioned_retries": 2,
    "raster_section_mode": "logical_sections_auto",
    "raster_section_count": "",
    "raster_section_count_auto": 8,
    "finalize_workers": 16,
    "save_associated_palette": true
  },
  "solve_score": {
    "enabled": true,
    "metric": "proximity",
    "chain": [["proximity", "0.1"]],
    "program_spec": "m0"
  },
  "fragment_manifest": {
    "version": 1,
    "width": 4096,
    "height": 4096,
    "pair_encoding": "u32le_u8_v1",
    "fragments": [
      {
        "section_idx": 0,
        "key": "renders/compute_x/color/color_run_123/fragments/section_0.frag",
        "url": "https://presigned.example/section_0.frag?...",
        "size": 12345680
      }
    ],
    "chain_fingerprint": "sha256:...",
    "clip_slots": [
      {
        "slot": 0,
        "metric": "proximity",
        "source": "slv",
        "clip_lo": 0.0123,
        "clip_hi": 0.9321
      }
    ],
    "background_color": [255, 255, 255]
  },
  "outputs": {
    "family": "color",
    "artifact_id": "color_run_123",
    "artifact_prefix": "renders/compute_x/color/color_run_123/",
    "image_key": "renders/compute_x/color/color_run_123/image.jpeg",
    "preview_key": "renders/compute_x/color/color_run_123/preview.png",
    "meta_key": "renders/compute_x/color/color_run_123/meta.json",
    "raw_key": "renders/compute_x/color/color_run_123/greyscale.raw",
    "raw_meta_key": "renders/compute_x/color/color_run_123/greyscale.meta.json",
    "deepzoom": {
      "enabled": false,
      "export_id": "",
      "prefix": "",
      "dzi_key": "",
      "viewer_key": "",
      "tile_prefix": ""
    }
  },
  "params": {
    "fmt": "jpeg",
    "quality": 90,
    "color_mode": "solve_score",
    "palette": "viridis",
    "constant_color": "ffffff",
    "root_transforms": []
  },
  "preview": {
    "enabled": true,
    "format": "png",
    "max_side": 512
  },
  "associated_palette": {
    "enabled": true,
    "mode": "generated",
    "palette_id": "pal_color_run_123",
    "display_name": "proximity(q=0.1%) · viridis",
    "image_key": "renders/compute_x/palettes/pal_color_run_123/image.jpeg",
    "preview_key": "renders/compute_x/palettes/pal_color_run_123/preview.png",
    "meta_key": "renders/compute_x/palettes/pal_color_run_123/meta.json"
  }
}
```

Rules:

- `phase` is always `"finalize_mt"`.
- `mode` is one of `"color"`, `"bilevel"`, `"coeff_bilevel"`.
- `render_execution.raster_engine` is required and is exactly `"mt"` in the
  first cut of this refactor. There is no serial FinalizeMT raster path to
  select in Phase 2 through Phase 5.
- `fragment_manifest.version` is required and starts at `1`.
- `fragment_manifest.pair_encoding` is required and is exactly
  `"u32le_u8_v1"` in the first cut.
- `fragment_manifest.clip_slots` is an array, not a keyed object. Slot
  order is the compiled metric-slot order and is stable for fingerprinting.
- `fragment_manifest.clip_slots` is the source of truth for clip metadata.
  `handler_finalize_mt.py` copies it into `greyscale.meta.json` unchanged;
  the contract is provenance, not independent recomputation.
- `outputs.raw_key` and `outputs.raw_meta_key` are always present.
- `outputs.deepzoom.*` may be empty strings when deepzoom is not requested.
- `preview.enabled=false` is allowed for families that do not emit a preview.
- `preview.max_side` is planner-derived, not operator-configurable in the
  first cut. The default policy value is `512` and is omitted only when
  `preview.enabled=false`.
- `associated_palette.enabled=false` is allowed; the remaining fields may
  be empty in that case.
- `render_execution.save_associated_palette` is the operator-intent flag.
- `associated_palette.*` is the resolved runtime state after planning:
  allocated palette artifact id, keys, and display metadata.
- Precedence is one-way and exact:
  - if `render_execution.save_associated_palette=false`, then
    `associated_palette.enabled` MUST be `false`
  - if `render_execution.save_associated_palette=true`, then the planner
    MUST either populate a complete `associated_palette` object or fail the
    plan; FinalizeMT MUST NOT silently downgrade it to disabled
- A payload that sets `render_execution.save_associated_palette=false` and
  `associated_palette.enabled=true` is invalid and rejected before work
  begins.
- In the fused path, `render_execution.save_associated_palette=true` means
  the planner emits a second sibling `ColorAssembleEncodeTask` /
  `FinalizeMT` invocation with a smaller associated-palette viewport and a
  distinct outputs block. It is not an internal fan-out hidden inside the
  main color FinalizeMT invocation.

The exact success response body from `handler_finalize_mt.py` is:

```json
{
  "artifact_id": "color_run_123",
  "family": "color",
  "image_key": "renders/compute_x/color/color_run_123/image.jpeg",
  "preview_key": "renders/compute_x/color/color_run_123/preview.png",
  "meta_key": "renders/compute_x/color/color_run_123/meta.json",
  "raw_key": "renders/compute_x/color/color_run_123/greyscale.raw",
  "raw_meta_key": "renders/compute_x/color/color_run_123/greyscale.meta.json",
  "deepzoom": {
    "enabled": false,
    "export_id": "",
    "dzi_key": "",
    "viewer_key": "",
    "tile_prefix": ""
  },
  "associated_palette": {
    "enabled": true,
    "palette_id": "pal_color_run_123",
    "image_key": "renders/compute_x/palettes/pal_color_run_123/image.jpeg",
    "preview_key": "renders/compute_x/palettes/pal_color_run_123/preview.png",
    "meta_key": "renders/compute_x/palettes/pal_color_run_123/meta.json"
  },
  "timings": {
    "assemble_ms": 12000,
    "histogram_ms": 80,
    "encode_ms": 650,
    "preview_ms": 120,
    "upload_ms": 900
  }
}
```

Response rules:

- `artifact_id`, `family`, `image_key`, `preview_key`, `meta_key`,
  `raw_key`, and `raw_meta_key` are always present for `mode="color"`.
- `timings` is required and is the source of truth for operator logs.
- `deepzoom.enabled=true` requires `export_id`, `dzi_key`, `viewer_key`,
  and `tile_prefix` to be non-empty.
- `associated_palette.enabled=true` requires `palette_id`, `image_key`,
  `preview_key`, and `meta_key` to be non-empty.

### Exact key naming contract

All keys are deterministic from `job_id`, `artifact_family`, and
`artifact_id`. No handler invents ad hoc names.

For a color artifact:

```text
artifact_prefix = renders/{job_id}/color/{artifact_id}/

image_key       = {artifact_prefix}image.{jpeg|png}
preview_key     = {artifact_prefix}preview.png
meta_key        = {artifact_prefix}meta.json
raw_key         = {artifact_prefix}greyscale.raw
raw_meta_key    = {artifact_prefix}greyscale.meta.json
fragment_prefix = {artifact_prefix}fragments/section_
```

Section fragments are exactly:

```text
{fragment_prefix}{section_idx}.frag
```

No zero-padding in the first cut.

For associated palette artifacts generated from the fused path:

```text
palette_prefix = renders/{job_id}/palettes/{palette_id}/

image_key    = {palette_prefix}image.jpeg
preview_key  = {palette_prefix}preview.png
meta_key     = {palette_prefix}meta.json
raw_key      = {palette_prefix}greyscale.raw
raw_meta_key = {palette_prefix}greyscale.meta.json
```

For inline DeepZoom output from FinalizeMT, keep the repo's existing
DeepZoom namespace:

```text
deepzoom_prefix = deepzoom/{job_id}/{export_id}/

dzi_key        = {deepzoom_prefix}image.dzi
viewer_key     = {deepzoom_prefix}viewer.html
tile_prefix    = {deepzoom_prefix}image_files/
latest_ptr_key = renders/{job_id}/deepzoom_latest.json
```

Rules:

- The raw lives with the artifact under `renders/{job_id}/...`, not in the
  top-level `deepzoom/` namespace.
- DeepZoom exports stay in the existing top-level `deepzoom/` namespace for
  storage/UI compatibility.
- `meta.json` remains the user-facing artifact metadata file.
- `greyscale.meta.json` is the raw-sidecar contract consumed by reuse
  handlers.
- `fragment_prefix` objects are intermediate but immutable for the life of
  the render run; they are deleted only when the artifact family is cleaned.

### Exact `greyscale.meta.json` schema

This sidecar is versioned and exact. Multimetric clips are represented as a
slot-ordered array, not a dict.

```json
{
  "version": 1,
  "job_id": "compute_xxxxxxxx",
  "run_id": "run_123",
  "artifact_family": "color",
  "artifact_id": "color_run_123",
  "width": 4096,
  "height": 4096,
  "encoding": {
    "type": "u8_clipped_score_v1",
    "background_byte": 0,
    "foreground_min": 1,
    "foreground_max": 255,
    "row_major": true
  },
  "chain_fingerprint": "sha256:...",
  "score_chain": [["proximity", "0.1"]],
  "score_program": "m0",
  "clip_slots": [
    {
      "slot": 0,
      "metric": "proximity",
      "source": "slv",
      "clip_lo": 0.0123,
      "clip_hi": 0.9321
    }
  ],
  "background_color": [255, 255, 255],
  "plan_params_digest": "sha256:...",
  "render_execution": {
    "color_pipeline": "fused",
    "raster_engine": "mt",
    "raster_mt_threads": 4,
    "raster_input_mode": "sectioned",
    "raster_sectioned_retries": 2,
    "raster_section_mode": "logical_sections_auto",
    "raster_section_count": "",
    "raster_section_count_auto": 8,
    "finalize_workers": 16,
    "save_associated_palette": true
  },
  "keys": {
    "raw_key": "renders/compute_x/color/color_run_123/greyscale.raw",
    "image_key": "renders/compute_x/color/color_run_123/image.jpeg",
    "preview_key": "renders/compute_x/color/color_run_123/preview.png",
    "meta_key": "renders/compute_x/color/color_run_123/meta.json"
  },
  "created_at": "2026-04-17T12:34:56Z"
}
```

Schema rules:

- `version` is required and starts at `1`.
- `background_color` is always a 3-element `[r, g, b]` array of integers
  in `[0,255]`.
- `clip_slots` is always an array. Empty is invalid for solve-score raws.
- `plan_params_digest` is required and hashes:
  - canonical JSON with sorted keys and compact separators for:
    - `viewport`
    - `grid.pix`
    - `grid.tile_size`
    - `params.root_transforms`
    - `raster_binary_sha256`
- `raster_binary_sha256` means the exact build-time
  `RASTER_BINARY_SHA256` string embedded into the Lambda package from the
  same ARM64 raster binary build that ships in production. It is not a
  filename, git rev, or ad hoc version label.
- `plan_params_digest` deliberately captures image-definition invariants,
  not race-resolution outcomes. It excludes execution-shape fields like:
  - `raster_section_mode`
  - `raster_section_count`
  - `raster_section_count_auto`
  - `raster_mt_threads`
  Because any-arrival-wins rasterization can produce byte differences under
  different fan-out shapes, the digest is not a byte-identity guarantee.
  Those execution-shape fields remain recorded under `render_execution`.
- `render_execution.color_pipeline` MUST be `"fused"` for raws created by
  this path.
- `keys.raw_key` MUST equal the S3 key the sidecar sits beside. Reuse
  handlers verify this before proceeding.
- `clip_slots` in the sidecar MUST equal the input `fragment_manifest.clip_slots`
  byte-for-byte after JSON serialization order normalization; FinalizeMT
  does not recompute or reorder them.

### Exact frontend tab contract

`Render -> Color -> Generate-MT` becomes a tabbed modal with path-specific
state. The contract is:

#### Classic tab

The `Classic` tab owns the current legacy controls:

- `Solve score threads`
- `Hist input`
- `Hist retries`
- `Hist sections`
- `Merge workers`
- `Raster threads`
- `Raster input`
- `Raster retries`
- `Raster sections`
- `Bin fragments`
- `Finalize workers`
- `Associated palette`
- `Palette chunk threads`
- `Palette chunk input`
- `Palette chunk retries`
- `Palette chunk workers`
- `Palette sections`

Executing from this tab writes:

```json
{
  "render_execution": {
    "color_pipeline": "classic",
    "...": "existing classic fields"
  }
}
```

#### Fused tab

The `Fused` tab owns only the controls used by FinalizeMT:

- `Raster threads`
- `Raster input`
- `Raster retries`
- `Raster sections`
- `Finalize workers`
- `Associated palette`

It does NOT expose:

- `Solve score threads`
- `Hist input`
- `Hist retries`
- `Hist sections`
- `Merge workers`
- `Bin fragments`
- `Palette chunk threads`
- `Palette chunk input`
- `Palette chunk retries`
- `Palette chunk workers`
- `Palette sections`

The `Associated palette` checkbox on this tab controls whether the planner
emits a second sibling FinalizeMT render for the associated-palette family.
It does not cause the main FinalizeMT invocation to produce two artifact
families internally.

Executing from this tab writes:

```json
{
  "render_execution": {
    "color_pipeline": "fused",
    "raster_engine": "mt",
    "raster_mt_threads": 4,
    "raster_input_mode": "sectioned",
    "raster_sectioned_retries": 2,
    "raster_section_mode": "logical_sections_auto",
    "raster_section_count": "",
    "raster_section_count_auto": 8,
    "finalize_workers": 16,
    "save_associated_palette": true
  }
}
```

Frontend state rules:

- The modal stores independent state per tab.
- Switching tabs never mutates the other tab's inputs.
- `Populate` / artifact restore chooses the tab from
  `render_execution.color_pipeline`.
- Phase defaults:
  - Phase 2: default-open tab = `Classic`
  - Phase 3: default-open tab = `Fused`
- The summary line in the modal must begin with:
  - `Path: classic`
  - `Path: fused`
- `Execute` reads only the active tab state.
- During Phase 2 and Phase 3, both tabs are rendered at all times.
- Once Phase 3 ships, the `Classic` tab renders a visible warning banner:
  - `Legacy path — retained for rollback only, slated for removal in Phase 5.`
- Only after Phase 5 may the `Classic` tab be removed.

### Exact recolor frontend contract

Phase 5 recolor is not a new top-level tab. It is an action on an existing
artifact entry that already has a reusable raw.

Rules:

- The operator starts from an existing color artifact row/card in the
  Render or Results surface.
- That surface exposes a `Recolor` action only when artifact metadata
  contains both:
  - `raw_key`
  - `raw_meta_key`
- Clicking `Recolor` opens a popup with:
  - source artifact summary
  - target palette selector / JSON loader
  - output format / quality controls
  - execute / cancel
- The popup does not expose solve, raster, section, or viewport controls.
  Recolor is raw reuse only.
- If an artifact lacks `raw_key` / `raw_meta_key`, the action is hidden or
  disabled with explicit wording:
  - `Recolor requires a greyscale raw sidecar; rerender with the Fused path.`
- The frontend dispatch target is `handler_recolor_from_raw.py`; it never
  re-enters the full render workflow.

### Exact test file map

Tests are mandatory and land in the same change as each phase. The file map
below is the implementation contract.

Existing files to update:

- `tests/test_render_plan.py`
- `tests/test_render_workflow_definition.py`
- `tests/test_frontend_js.sh`
- `tests/e2e/render-solve-score.spec.js`
- `tests/test_pipeline.py`
- `tests/test_raster_mt.py`
- `tests/test_raster_mt_parity.py`
- `tests/test_multispan_reader.py`
- `tests/test_deploy_packaging.py`

New files to add:

- `tests/test_fragment_format.py`
- `tests/test_assemble_greyscale.py`
- `tests/test_finalize_mt_handler.py`
- `tests/test_finalize_mt_reference.py`
- `tests/test_finalize_mt_bilevel_reference.py`
- `tests/test_finalize_mt_coeff_bilevel_reference.py`
- `tests/test_recolor_from_raw.py`
- `tests/test_deepzoom_from_raw.py`

Exact file ownership:

- `tests/test_render_plan.py`
  - adds assertions for `plan.render_execution.color_pipeline`
  - adds assertions for new output keys:
    - `raw_key`
    - `raw_meta_key`
    - `fragment_prefix`
  - adds assertions for the exact `fragment_manifest` schema placed in the
    plan / FinalizeMT payload
  - adds assertions that `Classic` and `Fused` populate distinct
    `render_execution` shapes
  - asserts the planner rejects
    `render_execution.save_associated_palette=true` when it cannot populate
    a complete resolved `associated_palette` object

- `tests/test_render_workflow_definition.py`
  - asserts `ColorAssembleEncodePhase` and `ColorAssembleEncodeTask` exist
  - asserts workflow choice routes on
    `$.plan.render_execution.color_pipeline`
  - asserts the exact `ColorAssembleEncodeTask` payload selectors match the
    schema documented in this file
  - asserts legacy `Classic` branch remains wired during Phase 2 and Phase 3
  - asserts `ModeChoice` is present in Phase 2 and Phase 3 workflow
    snapshots
  - asserts `ModeChoice` is absent in Phase 4 and Phase 5 workflow
    snapshots
  - asserts the three legacy `ReportDone*` states are removed once the
    unified path lands and exactly one `ReportDone` state remains
  - asserts the legacy branch is removed only in the phase that deletes it

- `tests/test_frontend_js.sh`
  - asserts `Render -> Color -> Generate-MT` has `Classic` and `Fused` tabs
  - asserts the active tab summary begins with `Path: classic` /
    `Path: fused`
  - asserts classic tab shows the legacy hist/merge/palette-chunk controls
  - asserts fused tab hides those controls and only shows the fused-path
    controls
  - asserts the Phase 3+ `Classic` banner text is visible when the legacy
    tab is rendered in deprecated mode
  - asserts switching tabs preserves independent state
  - asserts `Execute` dispatches the active tab's
    `render_execution.color_pipeline`
  - asserts `Recolor` is rendered as an artifact action, not a new tab
  - asserts `Recolor` is disabled/hidden when `raw_key` is absent
  - Phase 5 case: asserts the `Classic` tab no longer appears once the
    legacy path is retired
  - Phase 5 case: asserts the `Recolor` action is present when `raw_key`
    and `raw_meta_key` exist

- `tests/e2e/render-solve-score.spec.js`
  - clicks through the `Classic` and `Fused` tabs in the real browser
  - verifies the correct controls are visible on each tab
  - verifies tab switching does not clobber per-tab inputs
  - verifies the dispatch payload contains
    `render_execution.color_pipeline = classic|fused`
  - Phase 5 case: clicks artifact `Recolor`, selects a new palette, and
    verifies the recolor request is dispatched from artifact metadata

- `tests/test_pipeline.py`
  - parses stored `render_execution` metadata and asserts
    `color_pipeline` survives round-trip through storage
  - asserts render summaries surface raw artifact keys when present
  - asserts `deepzoom_latest.json` compatibility remains intact for the
    new deepzoom-from-raw path

- `tests/test_raster_mt.py`
  - asserts fused raster emits raw fragment objects with the exact 5-byte
    pair format
  - asserts fragment object keys use `fragment_prefix + section_{idx}.frag`
  - asserts `score_byte == 0` is never emitted

- `tests/test_raster_mt_parity.py`
  - keeps the existing parity harness for raster correctness where still
    applicable
  - adds a raw-fragment reference comparison:
    - same input section
    - same chain
    - emitted fragment stream matches the documented pair encoding

- `tests/test_multispan_reader.py`
  - extends the existing ranged-GET reader harness to fragment-manifest
    fetches used by `assemble_greyscale`
  - covers fragment reads that cross object boundaries, retry exhaustion,
    and truncated fragment object failures
  - pins the reader behavior shared by raster, hist/palette multispan, and
    raw assembly so those consumers cannot drift independently

- `tests/test_deploy_packaging.py`
  - asserts the new handlers and binaries are packaged:
    - `handler_finalize_mt.py`
    - `assemble_greyscale`
    - `handler_recolor_from_raw.py`
    - `handler_deepzoom_from_raw.py`
  - asserts legacy handlers remain packaged until their retirement phase

- `tests/test_fragment_format.py`
  - exact pair encoding round-trip
  - invalid object length (`size % 5 != 0`) rejected
  - `pixel_idx >= width * height` rejected
  - `score_byte == 0` rejected
  - empty fragment accepted as zero hits
  - solve-step-order preservation test

- `tests/test_assemble_greyscale.py`
  - single-thread reference assembly vs binary output
  - multi-thread assembly vs reference output
  - repeated aliased writes accepted under any-arrival-wins semantics
  - optional 256-bin histogram side output matches a Python reference
  - malformed manifest / missing URL / truncated fragment failures are clear

- `tests/test_finalize_mt_handler.py`
  - exact request schema validation
  - exact response schema validation
  - sidecar JSON matches the documented `greyscale.meta.json` schema
  - background color propagation
  - associated-palette secondary output handling
  - deepzoom inline output handling

- `tests/test_finalize_mt_reference.py`
  - end-to-end reference renders through FinalizeMT
  - RGB image-diff against the approved fused baseline images
  - preview image existence/shape
  - raw + sidecar existence and key naming

- `tests/test_recolor_from_raw.py`
  - recolor from an existing raw with a new palette
  - raw bytes unchanged after recolor
  - chain fingerprint mismatch rejected
  - output key naming under a recolor artifact prefix

- `tests/test_deepzoom_from_raw.py`
  - raw-to-deepzoom path produces:
    - `image.dzi`
    - `image_files/...`
    - `viewer.html`
    - `deepzoom_latest.json` pointer update
  - chain fingerprint mismatch rejected where applicable
  - key naming matches the contract in this doc

Per-phase mandatory file set:

- Phase 0
  - `tests/test_render_plan.py`
  - `tests/test_render_workflow_definition.py`
  - handler tests that currently assert `quantile` / `omega` fields:
    - `tests/test_solve_proximity_handler.py`
    - `tests/test_palette_chunk_handler.py`

- Phase 1
  - `tests/test_fragment_format.py`
  - `tests/test_assemble_greyscale.py`
  - `tests/test_raster_mt.py`
  - `tests/test_multispan_reader.py`
  - `tests/test_render_plan.py`
  - `tests/test_deploy_packaging.py`

- Phase 2
  - `tests/test_finalize_mt_handler.py`
  - `tests/test_finalize_mt_reference.py`
  - `tests/test_render_workflow_definition.py`
  - `tests/test_frontend_js.sh`
  - `tests/e2e/render-solve-score.spec.js`

- Phase 3
  - `tests/test_pipeline.py`
  - `tests/test_render_workflow_definition.py`
  - `tests/test_frontend_js.sh`
  - `tests/test_finalize_mt_reference.py`

- Phase 4
  - `tests/test_finalize_mt_handler.py`
  - `tests/test_render_workflow_definition.py`
  - `tests/test_finalize_mt_bilevel_reference.py`
  - `tests/test_finalize_mt_coeff_bilevel_reference.py`

- Phase 5
  - `tests/test_recolor_from_raw.py`
  - `tests/test_deepzoom_from_raw.py`
  - `tests/test_pipeline.py`
  - `tests/test_frontend_js.sh`
  - `tests/test_deploy_packaging.py`

### Implementation and test phases

Five phases, each independently shippable. The raw contract is introduced
alongside the existing pipeline behind a feature flag, so every phase can
be rolled back by flipping one parameter.

**Phase 0: Chain fingerprint** *(dependency from Issue 1)*

Ship Issue 1 step 2 before starting Phase 1. FinalizeMT and both reuse
Lambdas rely on the chain fingerprint to bind raw artifacts to the chain
that produced them. Phase 0 can be shipped as a standalone cleanup.

**Phase 1: Raw emission as side output (flagged, parity-testable)**

- Add `assemble_greyscale` C binary to the Lambda package.
- Modify `roots2pix_mt.c` to emit `(pixel_idx, byte)` sparse fragments
  **in addition to** the existing RGB fragments, gated by a
  `--emit_raw_fragments=1` flag.
- Extend `handler_finalize.py` to, when the flag is set, run
  `assemble_greyscale` over the raw fragments and write `greyscale.raw`
  + `greyscale.meta.json` alongside the existing RGB outputs.
- **Test:** bit-for-bit identity against the existing pipeline's RGB
  output (since we are only *adding* the raw artifact, not consuming it
  yet). Render 5-10 reference jobs both ways, confirm the RGB outputs
  match and the raw artifacts are well-formed (fragment format
  invariants, sidecar schema, chain fingerprint matches the plan).

This phase is pure additive work. Ship to prod with the flag off; turn
on for internal/test jobs; observe cost and latency impact of the extra
emission. Expected overhead: modest — the raw fragments are smaller than
the RGB fragments, but we write both.

**Phase 2: FinalizeMT as sibling path (flagged, diff-testable)**

- Add `handler_finalize_mt.py` and the FinalizeMT Lambda.
- Add a new workflow branch `ColorAssembleEncodePhase` →
  `ColorAssembleEncodeTask` in parallel to the existing
  `ColorFinalizePhase` → `ColorEncodeTask` path.
- Gate via `plan.render_execution.color_pipeline`:
  - `"fused"` routes to the new branch
  - `"classic"` routes to the old branch
- Frontend: `Render -> Color -> Generate-MT` grows `Classic` and `Fused`
  tabs. `Classic` targets the old branch; `Fused` targets the new
  `FinalizeMT` branch. This is the only supported operator-facing way to
  choose between the two paths; no hidden/manual flag twiddling.
- **Test:** render the same reference jobs through both branches
  side-by-side. Image-diff the outputs (RGB PNG) with a configurable
  tolerance. The new output WILL differ from the old output because the
  equalization domain changes (pixel-space vs step-space); the diff
  check is not for bit-identity but for "structurally the same image in
  the same palette, within aesthetic tolerance." Fixed reference images
  for the new path are established in this phase.
- **Payload contract tests:** assert the FinalizeMT Lambda payload
  matches the workflow ASL, and that manifest / sidecar JSON shapes
  match the documented schema.
- **Docker ARM64 binary tests:** `assemble_greyscale` runs inside the
  Docker ARM64 test harness (matching the Lambda runtime), with fixtures
  for fragment format invariants, section count scaling, and race
  tolerance (`N` threads writing the same pixels, confirm atomicity).

Ship with flag off; turn on for internal test jobs; promote once the
reference images are accepted as the new product baseline.

Phase 2 -> Phase 3 promotion gate:

- `tests/test_finalize_mt_reference.py` passes on the checked-in Phase 2
  reference corpus.
- The image-diff tolerance used by that suite is the promotion tolerance;
  no looser manual threshold is allowed.
- The repo owner or designated visual sign-off owner explicitly approves
  the fused outputs on that corpus as the new baseline for color renders.
- Only after that sign-off may the default `color_pipeline` flip to
  `"fused"` in Phase 3.

**Phase 3: Flip the default, deprecate the old color path**

- Switch the default `plan.render_execution.color_pipeline` to `"fused"`
  for color renders.
- Mark the old `ColorFinalizePhase` / `ColorEncodeTask` states as
  deprecated; keep them running behind an explicit opt-in flag for one
  release cycle to support rollback.
- `Rollback` in this phase means an operator-scope revert of the default:
  switch the default-open/default-dispatched path back to
  `render_execution.color_pipeline="classic"` without deleting the fused
  code path. It does not require reverting the Phase 3 code change unless
  the defect cannot be mitigated by configuration.
- Frontend: keep both `Classic` and `Fused` tabs in
  `Render -> Color -> Generate-MT`. `Fused` becomes the default-open tab,
  but `Classic` remains available for rollback and side-by-side operator
  comparison until the legacy branch is actually deleted.
- Frontend: the `Classic` tab shows the visible warning banner
  `Legacy path — retained for rollback only, slated for removal in Phase 5.`
  so operators understand that extra knobs do not mean preferred path.
- Delete the solve-score hist map / merge states.
- Delete the associated palette map / finalize states (replace with an
  inline second output in the primary FinalizeMT invocation).
- Remove the `--emit_raw_fragments` flag path from `roots2pix_mt.c`; raw
  emission is now the only mode. Drop RGB fragment emission.
- **Test:** all existing color render integration tests pass against the
  new default. No opt-in required.
- **Carry cost:** Phase 3 through Phase 5 pays for two live color paths in
  code, tests, packaging, and operational support. This is intentional but
  should be treated as a migration tax, not a steady-state design.
- **Exit trigger for Phase 5:** do not delete the Classic path on a
  calendar date alone. Retire it only after Classic usage recorded in DDB
  / artifact metadata drops to zero for `N` consecutive days
  (recommended `N = 14`) and no rollback incidents are open in the same
  window.

**Phase 4: Migrate bilevel and coeff_bilevel**

- Extend `handler_finalize_mt.py` dispatch to `family=bilevel` and
  `family=coeff_bilevel`. Each uses the same RasterMT output, just with
  a different libvips pipeline (threshold LUT instead of equalize +
  palette LUT).
- Add a coeffs-driven RasterMT variant for `coeff_bilevel` (separate
  binary or flag on the existing one).
- Delete the `BilevelRaster*` / `BilevelMerge*` / `BilevelStitch*`
  states and their handlers.
- Delete the `CoeffRaster*` / `CoeffMerge*` / `CoeffStitch*` states and
  their handlers.
- Delete the top-level `ModeChoice` state; output family becomes a
  payload field dispatched inside FinalizeMT.
- **Test:** bilevel and coeff_bilevel reference images regenerated
  through the new path; visual comparison against the old outputs.

**Phase 5: Delete dead code and reuse the raw**

- Enter Phase 5 only after both migration gates are satisfied:
  - the Phase 3 retirement trigger is satisfied:
    Classic usage is zero for `N` consecutive days (recommended `N = 14`)
    and rollback is no longer needed
  - Phase 4 is fully complete:
    bilevel and coeff_bilevel are already running on FinalizeMT in
    production, their reference suites are green, and no remaining
    workflow branches still depend on the pre-FinalizeMT path
- Remove all Lambda ARNs, handlers, C binaries, and workflow states
  retired in Phase 3 and Phase 4.
- Ship `handler_recolor_from_raw.py` and `handler_deepzoom_from_raw.py`.
  Frontend exposes a `Recolor` action on any existing artifact that has a
  `greyscale.raw` sidecar; this is an artifact action, not a new top-level
  tab.
- Ship the deepzoom-from-raw path as the default DeepZoom generator
  (replaces the existing `handler_deepzoom_export.py` pipeline).
- Existing DeepZoom artifacts already present in S3 are untouched by this
  migration. They remain readable at their current keys; Phase 5 does not
  regenerate or rewrite them in place.
- `Recolor` opens a small popup bound to the source artifact, lets the
  operator choose a new palette / format, and dispatches
  `handler_recolor_from_raw.py` using the stored `raw_key` +
  `raw_meta_key`.
- If an artifact lacks a raw sidecar, recolor is unavailable and the UI
  must say so explicitly instead of silently falling back to a full
  rerender.
- **Test:** recolor round-trip tests (render → recolor with a new
  palette → verify LUT applied correctly, raw unchanged, chain
  fingerprint unchanged). Deepzoom parity tests against the existing
  deepzoom output.

**Cross-phase test coverage**

- Fragment format unit tests: invariants, malformed inputs rejected,
  size scaling, race tolerance.
- Assemble binary: parity test that `N` threads produce the same byte
  matrix (up to race-resolution of pixel-aliased steps) as a
  single-threaded reference implementation.
- Sidecar schema: JSON schema validation on every written sidecar.
- Chain fingerprint enforcement: `recolor_from_raw` and
  `deepzoom_from_raw` refuse to operate on raws fingerprinted with a
  different chain than the caller specifies.
- End-to-end: reference job renders at each phase boundary, with
  explicit lock-in of the new equalization-domain outputs at Phase 2
  (this is where the aesthetic change is committed).
