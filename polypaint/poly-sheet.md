# Poly-Sheet: parameter-scan mosaic artifacts

## 1. What it is

A new artifact kind: a **sheet** — one stitched bilevel mosaic image
whose tiles are low-resolution renders of the SAME pipeline as one
parameter varies (the persisted version of the UI scrub popups).

Deliberately outside the compute->render workflow:

- no compute_xxxx tree, no stored root lists, no chunk fan-out;
- one async lambda renders every frame and uploads exactly TWO
  objects: the mosaic PNG and a small manifest;
- frames are preview-class (single-machine, compute-tab-preview
  sized), bilevel only.

## 2. Inputs

The standard pipeline triple, selected exactly as elsewhere:

- param program (source text)
- function (+ cfpv)
- coeff program (source text)

plus the sheet configuration:

| knob | meaning |
|---|---|
| scan tag | which value varies, and how (see §3) |
| spacing | linear/log (from..to), angle (end excluded), or STEP: from + k*step — the discrete form; step=1 walks integers (degrees, counts), and integral values substitute as integer literals so they compile in count positions |
| frames | number of tiles (= scan steps) |
| N | per-frame grid (preview-class, e.g. 64..192) |
| solver | any of the 7 solver modes (fused trio preferred) |
| viewport | ALL preview modes: per-frame quantile/q-shim (DEFAULT — each tile optimally framed, like the scrub popups) \| marquee (exact explicit bounds) \| square (side selector) \| frozen-from-first-frame |
| rotate | 0/90/180/270 (quarter turns — lossless on bilevel) |
| grid | mosaic columns (rows derived) |
| polarity | white-on-black (default, matches the compute preview) or black-on-white |
| margin | 0..64px gutter between tiles and around the border, background-colored |
| bilevel rule | hit-mask (pixel hit iff >=1 root) v1; count threshold later |

## 3. The scan tag

v1 mechanism: a **substitution token** written directly in any of the
three source texts:

```
poly = scan(35, 0, 0.5, prev*$T*(1-prev))
```

The sheet config binds it: `$T: from 0.1 to 0.9, 64 steps, linear`
(spacing linear|log|angle). Frame k compiles the sources with `$T`
replaced by the literal value — per-frame compile cost is negligible
next to the solve, and the mechanism covers every syntactic position
(scan constants, radii, exponents, pokes, cfpv entries) with zero new
compiler surface. The tag never reaches the compiler: the sheet
runner substitutes text BEFORE compile, so no grammar change and no
new chip.

Two scan LINES in the UI (v1.6): a blank Token deactivates a line.
One active line = the 1-D sequence wrapped into the grid (Cols knob
applies). Both active = CROSS PRODUCT: line 1 walks the COLUMNS
(steps <= 32, the column cap), line 2 walks the ROWS, frames row-major
(frame k -> col k mod s1, row k div s1), total frames = s1*s2 <= 256,
Cols ignored. Every active token must appear in a source; labels stamp
'v1,v2'. Manifest: 'scans' list (per-axis resolved values), legacy
'scan' = axis 0.

## 4. Execution model

One async lambda (`polypaint-poly-sheet`) with client-orchestrated
FAN-OUT (v1.3): the client splits the frames over up to 8 'frames'
worker jobs via the dispatch lambda, each worker renders its
contiguous share and uploads sheets/{id}/tiles/{k}.bin+.json, then a
'stitch' job assembles the mosaic + manifest and deletes the tiles.
Fan-out output is byte-identical to the single-shot 'run' action
(test-pinned). Frozen-first-frame viewport under fan-out: every
worker derives frame 0's bounds itself — the pipeline is
deterministic, so no cross-worker coordination. Per-worker budget
guard replaces the whole-sheet one. Single-frame flow:

```
for k in frames:
    sources_k = substitute(sources, token, value_k)
    compile server-side (existing compilers)
    sweep_coeffgen grid mode, fused_solver=<solver>, roots_file=...
    bin roots -> N_px x N_px bilevel tile (shared explicit viewport)
    report_status(phase=f"frame {k}/{K}")     # jobs-rail progress
    check cancel marker in DDB                # cheap kill between frames
stitch tiles (quarter-turn rotate applied per tile) -> 1 PNG
upload sheets/{sheet_id}/sheet.png + sheet.json
```

- Per-frame cost anchors (measured this week, deg-36): AE64 ~23us/row,
  JT64 ~84us/row, CM64 ~359us/row. A 128x128 frame = 16k rows: AE64
  ~0.4s, JT64 ~1.4s, CM64 ~6s. A 64-frame AE64 sheet ~25s + raster;
  comfortably one invocation. Budget guard: estimated total must fit
  ~12 of the lambda's 15 minutes, else 400 with the math shown.
- Viewport is a per-sheet knob exposing the preview's full mode set:
  per-frame QUANTILE/q-shim (default — each tile optimally framed,
  the scrub-popup behavior), MARQUEE (exact explicit bounds, shared
  by all frames), SQUARE (side selector, shared), and
  FROZEN-FROM-FIRST-FRAME. The trade-off: per-frame framing hides
  the parameter's effect on position/scale (a drifting, growing
  cloud renders as near-identical centered tiles), so the shared
  modes serve scans where the geometry motion IS the point. All
  paths already exist in the preview handler.
- Kill: no Step Functions here, so the rail's stop routes a cancel
  marker the loop checks between frames (seconds-level latency, good
  enough for a <=15min job).

## 5. Storage + artifact surface

```
sheets/{sheet_id}/sheet.png    # the mosaic (1-bit content, PNG)
sheets/{sheet_id}/sheet.json   # inputs, scan spec, per-frame values,
                               # viewport, timings, solver, versions
```

Own routes (`/sheet` dispatch, `/list-sheets`, `/delete-sheet` or the
generic delete-prefix), own tab. No entry in the render artifact
lists.

## 6. UI: the "Sheets" tab

(NOT "Mosaic" — that name is taken by the allcol/allpal artifact
walls.) One new tab:

- pipeline pickers (reuse the existing program/function selectors);
- scan tag row: token, from, to, steps, spacing;
- frame row (v1.5): columns + polarity + margin + freeze-frame-0 +
  label-frames checkboxes. SOLVER, N, pixel size, viewport mode/bounds
  (Q-shim/marquee/square) and rotation are ALL inherited from the
  Compute preview controls — the sheet renders what Preview shows, no
  duplicated knobs. Rotation is a compute-preview select
  (0/90CW/90CCW/180, CSS-rotates the preview display, quarter-turn
  applied to sheet tiles server-side). Label-frames stamps each tile's
  scan value into its top-left corner (CP437 8x8 glyphs, scaled with
  the tile, drawn after rotation so labels read upright);
- Populate button: loads the selected sheet's manifest back into the
  Compute tab (function/programs/cfpv via the same populate path the
  Results tab uses) plus the preview + sheet controls — the manifest
  stores the UNSUBSTITUTED pipeline sources for this;
- the sheet list highlights the selected row (same convention as the
  Results table);
- right-click on a frame in the embedded DeepZoom viewer (v1.7,
  sheets tab only — bound on #sheet-osd, the DeepZoom tab is
  untouched) opens a context popup (artifact-mosaic menu styling)
  showing the frame index, each token's value, and the frame bounds,
  with a Populate Frame action: the compute tab is populated with the
  sheet's pipeline where every scan token is REPLACED by the literal
  value that rendered the clicked frame (server literal spelling —
  integers bare, negatives (0-x)). Gutter/border clicks close the
  menu; Escape/outside click dismiss; pre-v1.5 manifests (no
  pipeline) show a disabled action with a note;
- Execute (async button discipline: busy + lingering result), rail
  card with frame progress + kill;
- gallery: list of sheets, click -> viewer. The viewer (v1.4) IS an
  embedded OpenSeadragon pane on the tab (same OSD build as the
  DeepZoom tab): clicking a sheet in the list swaps the pyramid in
  place (a view-sequence guard makes the latest click win on rapid
  switching); a legacy sheet with no export generates one in place
  (deduped in-flight). Download (blob fetch -> save) is the one
  remaining button; the old img + 1:1/Fit/Open surface is gone.
- DeepZoom generation (v1.3): after the stitch the client auto-fires
  the deepzoom-export lambda's action='sheet' path — source key built
  server-side from the validated sheet_id, NON-SQUARE mosaics allowed
  (the render export path requires square), pyramid + viewer.html
  under deepzoom/{sheet_id}/{export_id}/ so /list-deepzoom discovers
  sheet exports with zero listing changes.

## 7. What already exists vs what is new

| piece | status |
|---|---|
| per-frame pipeline (triple -> coeffgen -> fused solve -> raster) | EXISTS (compute preview handler; grid mode + fused_solver) |
| explicit shared viewport | EXISTS (marquee wave) |
| solver plumbing incl. fused trio | EXISTS |
| server-side source compile | EXISTS |
| async dispatch + status polling + rail cards | EXISTS |
| scan-tag substitution | NEW (small: string substitute + validate) |
| bilevel binning + PNG stitcher | NEW (numpy binning; PNG encode as in the preview handler) |
| sheet routes + storage + listing | NEW (small, mirrors deepzoom share patterns) |
| Sheets tab | NEW (largest single piece) |
| cancel marker | NEW (small) |

## 8. Open questions (decide before build)

1. **Scan tag form**: `$T` substitution token (recommended, zero
   compiler surface) vs structured per-element UI tags?
2. **Bilevel rule**: pure hit-mask v1, threshold knob later — OK?
   (viewport question RESOLVED: per-frame quantile default, shared
   explicit + frozen-first as options)
3. **Rotation**: quarter-turns only in v1 (arbitrary angles need
   resampling, which is ugly on bilevel) — OK?
4. **Caps**: max frames (256?), max N (192?), plus the runtime budget
   guard — sane?
5. **Tab name**: "Sheets"?
6. Scan spacing set: linear/log/angle enough?

## 9. Build order (when approved)

1. Backend handler + scan substitution + bilevel stitcher + routes,
   with an end-to-end test against the local binary (the fused-lores
   lesson: tests must EXECUTE the handler path).
2. Budget guard + cancel marker + rail integration.
3. Sheets tab UI + payload contract tests.
4. Docs + memory.
