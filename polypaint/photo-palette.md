# Photo Palette: extract a palette from a local image, in the browser

## 1. Objective

A sixth palette swatch — **PIC** — after MIC on every palette row. It opens
a popup where the user picks (or drops) a local photo; the palette is
computed **entirely client-side** and saved under a name derived from the
sanitized filename. The photo itself never leaves the browser: no image
upload, no image storage, no backend compute. The only network write is the
tiny name+stops record into the existing named HEX custom-palette catalog.

The extraction algorithm is the one already validated in this repo:
`scripts/extract_image_palette.py` is the Python port of the client-side
pipeline meditationsincolor.com runs on its artwork pages (see
color-meditations.md §"artwork.html"); this feature ports it back to JS,
where it originally lived. Validation anchor (2026-07-23, Arthur Dove):
the pipeline reproduces the site's stored palettes at median 0.004–0.012
Oklab, 8/8 within the site's own 0.055 similarity threshold.

## 2. UX

### Swatch

- Circle labeled `PIC` (class `pal-circle-pic`, `data-palette-popup="pic"`),
  after MIC in `_renderPaletteRow`, all five modes.
- Title: "Extract palette from a photo". Face: gradient of the last
  extraction (per-session), else a fixed default gradient.

### Popup (`#pic-popup-overlay`, tri-popup shell like MIC)

```
┌───────────────────────────────────────────────┐
│ Extract palette from a photo          [Close] │
│ [ pick file… ]  (or drag & drop an image)     │
│ ┌────────────┐  name: [sunset beach 001     ] │
│ │  preview   │  colors:  [12]  (3..32)        │
│ │  (≤200px)  │  sampling: (low|MED|high)      │
│ └────────────┘  style: (EDITORIAL|literal)    │
│ ████▓▓▓▒▒░░ extracted strip (live)            │
│ [🎲 Re-roll]        [Use now]  [Save to HEX]  │
│ status line                                   │
└───────────────────────────────────────────────┘
```

- Any control change (colors / sampling / style) recomputes immediately;
  **Re-roll** draws a new random seed and recomputes (the k-means seeding
  is random by design — rolling variants and keeping the best is the
  intended workflow; repeatability is explicitly a non-goal).
- **Use now**: applies to the row's mode via
  `setPaletteForMode(mode, wire, name)` — rides the `custom:` wire +
  `palette_display_name` exactly like MIC picks. HEX swatch lights up
  (a photo palette IS a custom palette; PIC keeps no selection memory —
  simpler than MIC's, revisit only if it grates).
- **Save to HEX**: persists into the named custom catalog via the same
  CAS flow as Copy2HEX. Busy + lingering result on both buttons
  ("Extracting…", "✓ Saved to HEX", "✗ …") per the standing button rule.

### Name derivation

`IMG_4521_Sunset-Beach.HEIC` → stem → underscores/dashes → spaces →
strip non-printables → collapse whitespace → clamp 80 → collision suffix
against the catalog (" (2)", clamped) — same rules as Copy2HEX.

## 3. Algorithm (JS port)

Port `scripts/extract_image_palette.py` back to JS in js/04 (~200 lines),
constants identical:

| stage | constants |
|---|---|
| downscale | ≤400 px wide, canvas drawImage |
| sample | grid step targeting N samples, skip alpha ≤128 |
| cluster | k-means++ k=28 in RGB, 25 Lloyd iterations |
| merge | Oklab distance < 0.04, count-weighted; drop < 0.2% clusters |
| score | sqrt(share) × chroma(1+min(c/0.055, 2.25)) × coolMuted 1.45 × blue 1.32/teal 1.18 × vivid 1.08 × earth 0.58/neutral 0.82 |
| select | best accent per family, blue guarantee ≥0.4%, caps (earth ≤3, neutral ≤2, family ≤3 at target>8), separation 0.055/0.07, two relaxation passes |

### Controls → parameters

| control | values | maps to |
|---|---|---|
| colors | 3..32 (default 12) | selection target (32 = `custom:` wire cap; 3 = our own MIC floor) |
| sampling | low / med / high | TARGET_SAMPLES 4,000 / 12,000 / 40,000 |
| style | editorial / literal | editorial = full scoring above; **literal** = same cluster+merge+separation but score = sqrt(share) only — no boosts, no substrate penalty, no blue guarantee. For photos, literal is often the honest choice; editorial is the site's painting aesthetic |
| 🎲 re-roll | — | new `Math.random()` seed for k-means++ |

## 4. Architecture

- **No backend changes at all.** Compute: browser. Storage: existing
  `/save-custom-palettes` CAS route (revision + one auto-retry on 409).
  Wire: `custom:rrggbb-...` (2..32 stops) + display name ≤80 printable.
- **Refactor**: extract the save core of `_micCopySelectionToHex` into a
  shared `_saveEntryToCustomCatalog(name, stops)` (load catalog →
  idempotency check by palette spec → name collision suffix → validation
  payload → CAS post → update globals + rerender rows). Copy2HEX and PIC
  both call it; behavior of Copy2HEX unchanged (its e2e must stay green).
- File read: `<input type="file" accept="image/*">` + drop handler →
  `createImageBitmap(file, { imageOrientation: 'from-image' })` (EXIF
  rotation honored — the classic phone-photo trap). Fallback for browsers
  without the option: plain Image + object URL.
- Compute runs on the main thread (~100–300 ms per roll at med sampling,
  same as the site's artwork pages; high ≈ 3×). Web-worker offload is
  explicitly out of scope for v1.

## 5. Edge cases

- **HEIC**: decodes in Safari, not Chrome — surface a clear status error
  ("this browser cannot decode HEIC; export as JPG"), don't crash.
- Transparent PNGs: alpha ≤128 pixels excluded (site behavior); a fully
  transparent image → "too few opaque samples" error.
- Tiny images (< 20 samples): same error path.
- Monochrome photos: extraction may yield < 3 distinct clusters; allow
  saving whatever comes out ≥2 stops (wire minimum), status-note if the
  requested color count wasn't reached. (MIC's 3-stop floor is a catalog
  curation rule, not a wire rule — a deliberate 2-color photo palette is
  the user's business.)
- Catalog full (256 entries): surface the validator's error on the button.
- Grayscale/CMYK JPEGs: canvas normalizes to RGBA — no special handling.

## 6. Tests

- **e2e (palette-ui.spec.js)**: `setInputFiles` with an in-spec base64 PNG
  fixture of known color blocks →
  - extracted strip appears; block colors recovered (loose Oklab check via
    `page.evaluate` against the js pipeline's own oklab fn);
  - name box prefilled with the sanitized fixture filename;
  - Save to HEX posts the expected `{palettes, expected_revision}` payload
    (stubbed routes, same harness as the Copy2HEX test) and the button
    cycles busy → "✓ Saved to HEX" → restore;
  - Use now sets `_currentPaletteForMode` to a valid `custom:` wire and
    `_paletteDisplayNameForMode` to the derived name;
  - literal vs editorial produce different orderings on a fixture with a
    dominant brown (substrate penalty visible).
- **Harness/unit**: none needed beyond e2e — the algorithm's semantics are
  already pinned offline by `tests/test_extract_image_palette.py` (Python
  side); the JS port is exercised end-to-end by the e2e. If drift paranoia
  strikes later: a shared JSON fixture (samples → expected clusters at a
  fixed seed) executable by both sides.
- Predeploy: palette-ui spec already gated; nothing new to register except
  keeping `test_frontend_parts_contract` green (declarations only).

## 7. Implementation checklist

```text
[ ] js/04: _picExtractPalette (pipeline port) + literal-mode scoring
[ ] js/04: _saveEntryToCustomCatalog refactor; Copy2HEX rewired, e2e green
[ ] js/04: PIC swatch in _renderPaletteRow (all modes)
[ ] js/04: popup quintet (_openPicPalettePopup/_render/_close/_init/handlers)
[ ] index.html: #pic-popup-overlay DOM + CSS (reuse tri-popup + mic classes)
[ ] js/12: _initPicPalettePopup() in the boot block
[ ] e2e: fixture PNG + flow test in palette-ui.spec.js
[ ] full predeploy gate, commit, push
```

## 8. Out of scope / future

- Web-worker compute (only if high-sampling rolls feel janky).
- "Extract from render": run the same pipeline on a compute/render
  artifact already in the app — palette feedback loops. Needs only an
  image source picker; the pipeline is shared.
- Weights → gradient positions: extracted shares could set non-uniform
  `custom:` stop spacing one day; the wire format has no weights today,
  so shares are display-only.
