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
| frames | number of tiles (= scan steps) |
| N | per-frame grid (preview-class, e.g. 64..192) |
| solver | any of the 7 solver modes (fused trio preferred) |
| viewport | ALL preview modes: per-frame quantile/q-shim (DEFAULT — each tile optimally framed, like the scrub popups) \| marquee (exact explicit bounds) \| square (side selector) \| frozen-from-first-frame |
| rotate | 0/90/180/270 (quarter turns — lossless on bilevel) |
| grid | mosaic columns (rows derived) |
| bilevel rule | hit-mask (pixel black iff >=1 root) v1; count threshold later |

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

One varying parameter in v1 (a 1-D sequence wrapped into the mosaic
grid). The obvious v2 is two tokens -> rows x columns.

## 4. Execution model

One **async lambda** (`polypaint-sheet`), invoked through the existing
dispatch fan-out, budget-checked up front:

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
- frame row: N, solver select (7 modes), viewport (explicit fields +
  "grab from compute preview" convenience), rotate;
- grid row: columns;
- Execute (async button discipline: busy + lingering result), rail
  card with frame progress + kill;
- gallery: list of sheets, click -> viewer. Small sheets pan/zoom a
  plain img; big sheets feed the EXISTING DeepZoom pipeline
  (generator + viewer.html + share links) for OpenSeadragon viewing —
  no new viewer code.

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
