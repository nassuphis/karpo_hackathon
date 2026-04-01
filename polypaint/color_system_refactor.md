# Color System Refactor

This document specifies a detailed refactor for adding selectable tri-palettes from [color_dicts.py](/Users/nicknassuphis/karpo_hackathon/polypaint/color_dicts.py) into the current Render and Palette UI.

The goal is not just a new popup.
The selected tri-palette must become a real render palette that works end-to-end in:

- Render root-proximity coloring
- Render solve-score coloring
- Palette-tab palette generation
- Render Color-family generation when `Solve score` is active and the current render uses palette ids normally

## 1. Product Goal

Today, the color system exposes a small hardcoded set of render palettes in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html), while [color_dicts.py](/Users/nicknassuphis/karpo_hackathon/polypaint/color_dicts.py) already contains a much richer tri-palette source in `COLOR_TRI_STRINGS`.

The target behavior is:

1. each palette picker row gets a dedicated `TRI` swatch
2. right-click on `TRI` selects the currently remembered tri-palette for that mode
3. left-click on `TRI` opens a popup listing all tri-palettes
4. selecting a tri-palette from the popup:
   - makes the `TRI` swatch adopt that palette’s gradient
   - activates that tri-palette for the current mode
   - updates hover/title text to the selected tri-palette name
5. the selected tri-palette is passed through the normal render pipeline as a real palette id

This is not a debug-only feature.

## 2. Scope

This refactor applies to all three palette selector rows currently present in the UI:

1. `Root proximity` palette row in Render
2. `Solve score` palette row in Render
3. Palette-tab palette row

The `TRI` swatch must be available in all three places.

Current UI file:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

Current entry points that this refactor must preserve:

- `buildPaletteCircles(...)`
- `setPaletteForMode(...)`
- `_activeRenderPalette()`
- `renderRootProximityPalette`
- `renderSolveScorePalette`
- `paletteTabPalette`

This refactor is about palette selection and palette ids.
It is not a render-family UI refactor.
It must fit the current family-tab Render UI without changing artifact catalogs or viewer structure.

## 3. Core Design Decision

The tri-palette source of truth is:

- [color_dicts.py](/Users/nicknassuphis/karpo_hackathon/polypaint/color_dicts.py)
  - `COLOR_NAME_MAP`
  - `COLOR_TRI_STRINGS`

The frontend must not hand-copy those tri-palette definitions.
The backend must not hand-copy those tri-palette definitions.
The generator must not import broad rendering behavior from [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py) either.
It should use `colors.py` only as the methodology reference for:

- color parsing expectations
- RGB-to-HSV conversion
- hue interpolation methodology
- HSV-to-RGB conversion

Instead:

1. read tri-palette definitions from `color_dicts.py`
2. expand each tri-palette into a 15-stop LUT
3. generate:
   - a browser catalog for popup rendering
   - a C header for render/palette binaries
   - a Python name list for validation

This avoids three-way drift between:

- JS swatch previews
- Python palette validation
- C render/palette LUTs

## 4. Tri-Palette Semantics

For an entry in `COLOR_TRI_STRINGS`:

- format: `left:mid:right`
- each token may be:
  - a named color from `COLOR_NAME_MAP`
  - a literal 6-digit hex RGB value

### 4.1 Canonical 15-stop expansion

Every tri-palette expands to exactly 15 stops.

This is intentional even though the current built-in palettes in [lambda/palette_lut.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_lut.h) mostly use 16 stops.

Reason:

- tri-palettes have a mathematically privileged midpoint
- using 15 stops gives one exact center stop at index `7`
- [lambda/palette_lut.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_lut.h) already supports variable `n_colors`, so no runtime API change is required

This 15-vs-16 difference is acceptable and not considered a problem to solve in this refactor.

Indices:

- `0` = left stop
- `7` = middle stop
- `14` = right stop

Left half:

- indices `0..7`
- interpolate from left to middle
- `t = i / 7`

Right half:

- indices `7..14`
- interpolate from middle to right
- `t = (i - 7) / 7`

The middle color is present exactly once:

- stop `7`

### 4.2 Interpolation rule

The 15 generated stops are created by interpolating in HSV space.

Required rule:

- convert each endpoint RGB to HSV
- interpolate `S` and `V` linearly
- interpolate `H` using the same methodology already used in [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py)
- convert the interpolated HSV values back to sRGB hex

Reference methodology in [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py):

- `parse_color_spec(...)`
- `_rgb255_to_hsv01(...)`
- `_interp_hue_circle(...)`
- `_hsv01_to_rgb255_batch(...)`

Important hue rule:

- do not use naive linear hue interpolation
- do not use modulo-based “shortest arc” hue interpolation
- instead, interpolate hue by blending unit-circle complex representations and converting the angle back to hue

Reason:

- [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py) explicitly documents that modulo-based shortest-arc logic reintroduces a branch-cut seam
- the unit-circle blend is the preferred methodology reference for this refactor

### 4.3 Runtime rendering rule

The HSV interpolation happens when building the 15-stop LUT.

After that, runtime rendering continues to use the existing LUT interpolation behavior in [lambda/palette_lut.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_lut.h).

This is deliberate.
The refactor does not require rewriting runtime palette interpolation math in C.

## 5. Canonical Naming

Tri-palettes must be exposed to render code with a distinct palette id namespace.

Required palette id shape:

- `tri_<canonical_name>`

Examples:

- `tri_redgold`
- `tri_bauhaus_primaries`
- `tri_warhol_marilyn_cyan`

The visible UI name remains:

- `redgold`
- `bauhaus_primaries`
- `warhol_marilyn_cyan`

The `TRI` swatch label remains `TRI`.

### 5.1 Alias handling

`COLOR_TRI_STRINGS` contains short aliases such as:

- `rg`
- `gc`
- `bp`

The popup must not show duplicate rows for aliases.

Required rule:

1. resolve each tri-palette entry to its final `left/mid/right` RGB triple
2. group entries with identical resolved triples
3. choose one canonical display name per group
4. keep alias names only as metadata, not as separate popup rows

Canonical-name selection rule:

1. prefer the longest non-abbreviation key
2. tie-break lexicographically

So:

- `redgold` wins over `rg`
- `greencopper` wins over `gc`

All generated runtime palette ids use canonical names only.

### 5.2 Key-shape rule

Generated canonical names must be safe to use in:

- JS ids/data attributes
- Python validation sets
- C palette lookup names
- S3 metadata strings

Required rule:

- generated canonical names must match `^[a-z0-9_]+$`

If a chosen canonical source key does not satisfy that rule, the generator must fail hard rather than silently rewriting names.

## 6. User Experience

## 6.1 TRI swatch

Each palette row gets one extra swatch at the end:

- visible text: `TRI`
- shape: rounded pill, not a tiny unlabeled circle
- background: the currently selected tri-palette’s 15-stop gradient
- hover/title: `TRI: <selected_name>`

Example:

- `TRI: redgold`

If the currently remembered tri-palette catalog entry also has aliases, they may be shown in the title as secondary metadata:

- `TRI: redgold (aliases: rg)`

but the primary visible/selected name remains the canonical name only.

### 6.2 Right click

Right-click on the `TRI` swatch must:

1. suppress the browser context menu
2. activate the remembered tri-palette for that mode
3. mark the `TRI` swatch active
4. if the row is:
   - `proximity`
   - `solve_score`
   then also activate that color mode

Right-click does not open the popup.

Required implementation detail:

- use the `contextmenu` event
- call `preventDefault()`
- do not rely on `mousedown.button === 2` alone

Recommended fallback:

- allow `Alt+Click` to perform the same direct-select action for trackpads or environments where right-click is awkward

### 6.3 Left click

Left-click on the `TRI` swatch must:

1. open the tri-palette popup
2. scope the popup to the row/mode that opened it
3. pre-highlight the currently remembered tri-palette for that mode

Left-click alone does not finalize selection.

Selection is finalized by clicking a row in the popup.

### 6.4 Popup

The popup is a single reusable overlay component, not three copies.

It must contain:

- a small title showing the target mode, for example:
  - `Select TRI palette for Solve score`
- a filter input at the top
- a scrollable table

Table columns:

1. `Name`
2. `Palette`

Each table row shows:

- canonical tri-palette name
- aliases in smaller muted text if present
- a horizontal strip rendered from the exact 15 generated stops

Selecting a row:

1. stores that tri-palette as the remembered tri-palette for the active mode
2. sets the mode’s active palette id to `tri_<name>`
3. updates the `TRI` swatch gradient and title
4. for `proximity` / `solve_score`, activates that color mode
5. closes the popup

The table must be sorted by canonical name ascending.

The filter input must match against:

- canonical name
- aliases

This matters because [color_dicts.py](/Users/nicknassuphis/karpo_hackathon/polypaint/color_dicts.py) contains a large tri-palette catalog, and a plain long scroller will become cumbersome.

### 6.5 Popup dismissal

The popup must close on:

- outside click
- `Escape`
- explicit close button

Recommended keyboard behavior:

- `ArrowDown` / `ArrowUp` moves the highlighted row
- `Enter` selects the highlighted row
- `Escape` closes

This is not optional fluff.
It makes the popup usable for a large palette set.

## 7. Mode-Specific State

Tri-palette selection must be remembered independently per mode.

Reason:

- root proximity and solve score already keep independent palette state
- the Palette tab also has its own palette state
- selecting a TRI palette in one place must not silently overwrite the remembered TRI palette in another place

Required state additions in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html):

- `renderRootProximityTriName`
- `renderSolveScoreTriName`
- `paletteTabTriName`

Existing palette state variables remain, but they now store full palette ids:

- built-in palette:
  - `inferno`
- tri-palette:
  - `tri_redgold`

Recommended defaults:

- initialize all three remembered tri names to `redgold`

If the remembered tri name is missing from the generated catalog at runtime, the UI must fall back to the first catalog entry and must not leave the `TRI` swatch in a broken state.

## 8. Frontend Implementation

Primary file:

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)

### 8.1 New browser data file

Add a new generated browser asset:

- `tri_palette_catalog_js.js`

It should be loaded next to:

- `coeff_func_catalog_js.js`

Required shape:

```js
window._triPaletteCatalog = [
  {
    name: "redgold",
    palette_id: "tri_redgold",
    aliases: ["rg"],
    stops: ["#....", "... x15 total ..."],
    gradient_css: "linear-gradient(to right, ...)"
  }
];
```

`gradient_css` should be precomputed by the generator to keep the UI simple.

It should also include a lowercase `search_text` field so popup filtering does not recompute alias/name joins repeatedly:

```js
search_text: "redgold rg"
```

The generator should follow the parsing and HSV-conversion methodology from [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py), but it must not import rendering/colorizer behavior wholesale.
Only the color-resolution and HSV interpolation approach is the intended reference.

This file must be loaded before the main inline script in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html), just like [coeff_func_catalog_js.js](/Users/nicknassuphis/karpo_hackathon/polypaint/coeff_func_catalog_js.js).

### 8.1.1 Missing-catalog behavior

If `tri_palette_catalog_js.js` fails to load:

- the rest of the page must still work
- built-in palettes must still work
- `TRI` swatches must render in a disabled state
- hover/title should explain the problem:
  - `TRI catalog unavailable`

Do not let a missing generated asset break all palette selection.

### 8.2 UI HTML additions

Add one reusable popup near the end of the document:

- overlay container
- title
- scrollable table body
- close button

Do not inline all tri-palette rows statically in HTML.
They must be built from `window._triPaletteCatalog`.

### 8.3 CSS additions

Add styles for:

- `.pal-circle-tri`
- `.tri-popup-overlay`
- `.tri-popup`
- `.tri-popup-table`
- `.tri-popup-row`
- `.tri-popup-strip`

Requirements:

- popup should feel like an in-app tool panel, not browser default UI
- scrolling table height should comfortably handle large catalogs
- selected row should be visibly highlighted
- TRI swatch label must remain readable on top of the gradient

### 8.4 JS helper changes

Current relevant code:

- palette state variables around `PALETTE_DEFS`
- `setPaletteForMode(...)`
- `buildPaletteCircles(...)`
- `_activeRenderPalette()`

Required new helpers:

- `_triPaletteForMode(mode)`
- `_setTriPaletteForMode(mode, triName, activate = true)`
- `_openTriPalettePopup(mode)`
- `_closeTriPalettePopup()`
- `_renderTriPalettePopup(mode)`
- `_triSwatchTitle(mode)`
- `_triPaletteEntryByName(name)`
- `_triCatalogAvailable()`
- `_applyTriPopupFilter(text)`

### 8.5 `buildPaletteCircles(...)`

This function must be refactored.

Current behavior:

- renders one circle per built-in palette

New behavior:

1. render built-in palette circles exactly as today
2. append one extra `TRI` swatch
3. make that swatch:
   - left-click -> open popup
   - right-click -> activate remembered tri palette
4. set `title` to selected tri-palette name
5. set background to the selected tri-palette gradient

Implementation detail:

- keep the existing `.pal-circle` behavior for built-ins
- make the TRI swatch a separate element class such as `.pal-circle.pal-circle-tri`
- do not overload a normal built-in swatch with `data-palette="TRI"`

This keeps the built-in active-state logic simple and prevents tests from confusing the TRI control with real palette ids.

### 8.6 Payload behavior

No special new payload field is needed.

The existing palette field must carry the final selected palette id:

- built-ins:
  - `inferno`
- tri:
  - `tri_redgold`

This keeps the rest of the render dispatch path simple.

Required invariant:

- once selection is finalized, every dispatch path sees only the final palette id string
- no render/palette Lambda should need a special boolean like `is_tri_palette`

## 9. Backend Palette Source Of Truth

Primary runtime palette file:

- [lambda/palette_lut.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_lut.h)

Today this file hardcodes built-in palettes only.

It must be extended to include generated tri-palettes.

### 9.1 New generated C header

Add:

- `lambda/tri_palette_lut.generated.h`

This file is generated from `color_dicts.py`.

It must define:

- one `RGB[15]` array per canonical tri-palette
- a generated `PaletteDef` list or macro entries usable by `palette_lut.h`

Example shape:

```c
static const RGB PAL_TRI_REDGOLD[15] = { ... };
```

and entries:

```c
{"tri_redgold", PAL_TRI_REDGOLD, 15}
```

### 9.2 `palette_lut.h` changes

Modify [lambda/palette_lut.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_lut.h) to:

1. keep existing built-in palettes
2. include `tri_palette_lut.generated.h`
3. append generated tri-palette entries to `PALETTES`

The existing runtime API remains:

- `findPalette(name)`
- `paletteRGB(pal, t, ...)`

No caller-specific special case for tri-palette names is allowed.

Tri-palette names must behave like normal palette names at runtime.

### 9.3 Remove duplicated runtime palette registries

This refactor must also remove or neutralize any remaining palette registries that are not driven by [lambda/palette_lut.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_lut.h).

In particular:

- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c) still contains legacy inline palette definitions and help text assumptions

The active runtime lookup must come from the shared header only.
No second palette table is allowed to remain active in C code.

## 10. Python Validation

Current validation is hardcoded in:

- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- [lambda/handler_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_render_plan.py)
- legacy [lambda/handler_palette_debug.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_debug.py)

Render plan currently does not validate palette names early enough, and palette-name knowledge is duplicated.

### 10.1 New generated Python module

Add:

- `lambda/palette_names.py`

It should export:

```python
BUILTIN_PALETTE_NAMES = {
    "inferno",
    ...
}

TRI_PALETTE_NAMES = {
    "tri_redgold",
    "tri_bauhaus_primaries",
    ...
}

VALID_PALETTE_NAMES = BUILTIN_PALETTE_NAMES | TRI_PALETTE_NAMES
```

`TRI_PALETTE_NAMES` should be generated.
`BUILTIN_PALETTE_NAMES` can be checked in.
The important part is that handlers import one authoritative Python palette-name module instead of each keeping its own set literal.

### 10.2 Validation changes

Modify:

- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- [lambda/handler_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_render_plan.py)
- [lambda/handler_palette_debug.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_debug.py)

to validate:

- built-in palette names
- generated tri-palette names

Do not keep three hardcoded palette name sets that drift.

Recommended structure:

- one authoritative Python helper module exporting `VALID_PALETTE_NAMES`

This refactor should not add new duplicated `VALID_PALETTES = {...}` blocks.

## 11. Runtime Files Affected

These files must automatically pick up tri-palette support via `palette_lut.h` once the generated LUT is included:

- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
- [lambda/palette_bins_render.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_bins_render.c)
- [lambda/solve_palette_debug.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_debug.c)

### 11.1 Help text cleanup

Current usage text in some binaries still enumerates a short built-in palette list.

Update these usage/help strings to avoid hardcoding palette names.

Required change:

- use `--palette=<name>`

not:

- `--palette=inferno|viridis|...`

Reason:

- tri-palette count is large
- help text will otherwise rot immediately

## 12. Generation Pipeline

Add a checked-in generator:

- `scripts/generate_tri_palettes.py`

Responsibilities:

1. import `COLOR_NAME_MAP` and `COLOR_TRI_STRINGS` from `color_dicts.py`
2. resolve names and literal hex values to RGB
3. canonicalize aliases
4. build 15-stop HSV-expanded LUTs using the [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py) hue-circle interpolation approach
5. write:
   - `tri_palette_catalog_js.js`
   - `lambda/tri_palette_lut.generated.h`
   - tri-name data consumed by `lambda/palette_names.py`

The generator should be a small standalone script.
It must not import NumPy-heavy rendering code paths from [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py).
Reimplement only the tiny required pieces using the same methodology.

### 12.1 Failure rules

The generator must fail hard if:

- a tri-palette token is unknown
- a hex literal is malformed
- a palette resolves to anything other than exactly 3 stops
- generated canonical names collide
- a canonical name contains unsupported characters

Do not silently skip bad entries.

## 13. Deploy / Build Integration

Modify [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh) to run the generator before:

1. JS syntax check
2. C binary compilation
3. Lambda zip packaging

Required order:

1. generate tri assets
2. check `index.html`
3. compile binaries
4. package Lambdas

Reason:

- `index.html` will now load `tri_palette_catalog_js.js`
- C binaries will include `tri_palette_lut.generated.h`
- Python validation modules must exist before packaging

Recommended additional guard:

- `deploy.sh` should fail if the generator changes checked-in generated files and the user has not staged or accepted them

That prevents “local generator drift” from silently shipping stale assets.

## 14. Specific File Changes

### Must modify

- [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html)
- [deploy.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/deploy.sh)
- [lambda/palette_lut.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_lut.h)
- [lambda/palette_names.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_names.py)
- [lambda/handler_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_render_plan.py)
- [lambda/handler_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_render_plan.py)
- [lambda/handler_palette_debug.py](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/handler_palette_debug.py)
- [lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c)
- [lambda/palette_bins_render.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_bins_render.c)
- [lambda/solve_palette_debug.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/solve_palette_debug.c)

### Source-only references, not primary edit targets

- [color_dicts.py](/Users/nicknassuphis/karpo_hackathon/polypaint/color_dicts.py)
- [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py)

### New files

- `scripts/generate_tri_palettes.py`
- `tri_palette_catalog_js.js`
- `lambda/tri_palette_lut.generated.h`

### Must update tests

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)
- [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)
- [tests/test_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_render_plan.py)
- [tests/test_palette_debug_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_debug_handler.py)

### New tests

- `tests/test_tri_palette_generation.py`
  - Python-only generator tests

## 15. Test Requirements

### 15.1 Generator tests

`tests/test_tri_palette_generation.py` must verify:

1. canonicalization removes duplicate alias rows
2. `redgold` resolves correctly
3. each generated palette has exactly 15 stops
4. stop `7` equals the middle color exactly
5. hue interpolation follows the unit-circle methodology from [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py), not modulo shortest-arc interpolation
6. unknown color names fail
7. unsupported canonical-name characters fail
8. `gradient_css` is derived from all 15 stops, not only the 3 source colors

### 15.2 Frontend harness

[tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh) must verify:

1. each of the 3 palette rows contains a `TRI` swatch
2. left-click on `TRI` opens the popup
3. popup rows render name + strip
4. selecting a popup row updates:
   - remembered tri name for that mode
   - active palette id to `tri_<name>`
   - swatch title
   - swatch background
5. right-click on `TRI` activates the remembered tri-palette without opening the popup
6. root proximity and solve score keep independent remembered tri names
7. Palette tab keeps its own remembered tri name
8. render/palette dispatch payloads carry `palette: "tri_<name>"`
9. missing `window._triPaletteCatalog` disables TRI without breaking built-in palette rows
10. filter input narrows popup rows by canonical name and alias

### 15.3 Browser tests

[tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js) must verify:

1. `TRI` swatch is visible in root proximity and solve score rows
2. left-click opens popup
3. popup row selection updates the visible `TRI` swatch
4. `title` / hover text includes the selected canonical name
5. right-click selects the remembered tri-palette
6. switching built-in palettes does not erase remembered tri selection
7. popup keyboard navigation works (`ArrowDown`, `Enter`, `Escape`)

### 15.4 Backend tests

Plan/debug handler tests must verify:

1. `tri_redgold` is accepted as a valid palette
2. a bogus tri palette is rejected
3. render plan passes `tri_redgold` through unchanged
4. palette render plan passes `tri_redgold` through unchanged
5. palette debug accepts `tri_redgold`
6. handlers reject unknown `tri_...` ids via the shared Python palette-name module, not local duplicated sets

## 15A. Detailed Test Spec

This section defines the exact intent of the required tests.
It is here to prevent shallow tests that only check string presence while missing the real behavior.

### 15A.1 Generator test file

Primary file:

- `tests/test_tri_palette_generation.py`

This test file should import the generator module or its small pure functions directly.
It should not shell out unless there is a good reason.

Recommended functions to test directly:

- color-token resolution
- alias canonicalization
- 15-stop expansion
- JS/C/Python artifact record generation

Required test groups:

1. token parsing
   - named colors from `COLOR_NAME_MAP`
   - literal uppercase hex
   - literal lowercase hex
   - malformed hex rejection
   - unknown color-name rejection
2. canonicalization
   - alias pair like `rg` / `redgold`
   - same resolved triple collapses to one popup/catalog row
   - canonical name selection uses the documented longest-name rule
3. HSV expansion
   - output has exactly 15 RGB stops
   - stop `0` equals left exactly
   - stop `7` equals middle exactly
   - stop `14` equals right exactly
   - generated stops remain in integer RGB `0..255`
4. hue methodology
   - use a case that crosses the hue seam
   - assert output follows the unit-circle methodology and not a modulo shortest-arc shortcut
5. emitted records
   - JS record includes `name`, `palette_id`, `aliases`, `stops`, `gradient_css`, `search_text`
   - C entries are emitted as `PaletteDef`-compatible arrays
   - Python validation export contains `tri_<canonical_name>`
6. hard-fail cases
   - malformed tri string
   - canonical name collision
   - unsupported canonical-name characters

Regression-critical requirement:

- this test file must verify the exact center stop behavior
- if the generator accidentally switches to 16 stops or loses the exact midpoint, the tests must fail

### 15A.2 Frontend harness test scope

Primary file:

- [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)

This harness should test the UI logic in [index.html](/Users/nicknassuphis/karpo_hackathon/polypaint/index.html) without needing a real browser.

Required setup:

- inject a small fake `window._triPaletteCatalog`
- include at least:
  - one canonical palette with aliases
  - one second distinct palette
- also run one block with the catalog intentionally absent

Required assertions:

1. TRI swatch creation
   - all 3 palette rows render the TRI control
   - built-in swatches still render unchanged
2. left-click behavior
   - popup opens
   - popup title reflects the opening mode
   - popup rows reflect the injected catalog
3. selection behavior
   - selecting a row stores the remembered tri name for that mode
   - active palette id becomes `tri_<name>`
   - TRI swatch title updates
   - TRI swatch gradient updates
4. mode coupling
   - selecting TRI in `proximity` activates `renderColorMode = 'proximity'`
   - selecting TRI in `solve_score` activates `renderColorMode = 'solve_score'`
   - selecting TRI in `palette_tab` does not mutate render mode
5. right-click / alternate direct-select
   - direct-select activates remembered tri palette without opening popup
6. independent remembered names
   - `renderRootProximityTriName`
   - `renderSolveScoreTriName`
   - `paletteTabTriName`
   - all remain independent
7. filter behavior
   - filtering by canonical name works
   - filtering by alias works
8. graceful degradation
   - no catalog does not throw
   - built-in palette selection still works
   - TRI control is disabled and titled accordingly
9. payload wiring
   - render dispatch sends `palette: "tri_<name>"`
   - palette-tab dispatch sends `palette: "tri_<name>"`

Regression-critical requirement:

- at least one test must confirm that built-in palette selection after a TRI selection does not erase the remembered tri name

### 15A.3 Browser e2e spec

Primary file:

- [tests/e2e/render-solve-score.spec.js](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/e2e/render-solve-score.spec.js)

If the file becomes too crowded, splitting out:

- `tests/e2e/render-tri-palettes.spec.js`

is acceptable and may be cleaner.

Required e2e scenarios:

1. popup open/close
   - TRI visible
   - click opens overlay
   - Escape closes
   - outside click closes
2. visible selection
   - select a tri palette from popup
   - swatch visibly changes
   - title contains canonical name
3. mode-local memory
   - choose one tri palette in root proximity
   - choose a different one in solve score
   - switching between rows preserves each remembered selection
4. direct-select path
   - right-click TRI
   - popup does not open
   - remembered tri palette becomes active
5. filter usability
   - type in filter box
   - row list shrinks
   - alias match still finds canonical row
6. keyboard usability
   - ArrowDown highlights a row
   - Enter selects it
   - Escape closes

The browser test does not need to prove backend C rendering correctness.
It should focus on actual interactive behavior and visible state.

### 15A.4 Backend validation tests

Primary files:

- [tests/test_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_render_plan.py)
- [tests/test_palette_render_plan.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_render_plan.py)
- [tests/test_palette_debug_handler.py](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_palette_debug_handler.py)

Required assertions:

1. shared validation module
   - handlers import the same palette-name authority
   - no duplicated local `VALID_PALETTES` set remains authoritative
2. positive path
   - `tri_redgold` accepted in render plan
   - `tri_redgold` accepted in palette render plan
   - `tri_redgold` accepted in legacy palette debug path
3. negative path
   - `tri_not_real` rejected with a clear runtime error
4. payload preservation
   - handlers pass the tri palette id through unchanged
   - no normalization from `tri_redgold` back to `redgold`

Regression-critical requirement:

- the tests should fail if a handler silently falls back to `inferno` for an unknown tri palette

### 15A.5 Runtime palette lookup tests

This refactor should add one small focused runtime-lookup test file if needed:

- `tests/test_palette_lut_runtime.py`

This can be Python-driven and compile-free if it inspects generated metadata.
If a tiny C smoke test is easier, that is acceptable too.

What it must prove:

1. generated tri palette ids exist in the runtime palette registry
2. lookup is by final id `tri_<name>`
3. palette size is 15 for tri palettes
4. built-in palette lookup still works unchanged

This test exists to catch drift between:

- generated JS catalog
- generated C LUT
- Python validation names

### 15A.6 Deploy/build integration test

The generator is part of the build contract.
Add at least one light test or script assertion to prove that generated assets are in sync.

Acceptable places:

- a dedicated pytest file
- a shell check in [tests/test_frontend_js.sh](/Users/nicknassuphis/karpo_hackathon/polypaint/tests/test_frontend_js.sh)
- a small build-consistency script

What it must check:

1. `tri_palette_catalog_js.js` exists
2. `lambda/tri_palette_lut.generated.h` exists
3. Python validation names include the same canonical ids as the JS catalog
4. a known palette such as `tri_redgold` exists in all generated outputs

### 15A.7 What does not count as sufficient coverage

The following are explicitly insufficient by themselves:

- only checking that the string `TRI` appears in HTML
- only checking that `tri_redgold` appears somewhere in a generated file
- only checking that popup DOM exists without selecting anything
- only checking happy-path JS without testing missing catalog behavior
- only checking handler acceptance without checking unknown-tri rejection

## 16. Non-Goals

This refactor does not do any of the following:

- replace existing built-in palettes
- move all color definitions out of JS immediately
- change the number of solve-score bins
- rewrite `paletteRGB()` to interpolate in HSV at runtime
- add tri-palette selection outside Render / Palette tab
- collapse built-in palettes into the TRI popup
- change render artifact families or artifact catalogs

## 17. Acceptance Criteria

This refactor is complete only if all of these are true:

1. `TRI` swatch exists in all three palette selector rows
2. left-click opens a popup with the tri-palette catalog
3. popup rows show canonical name and strip preview
4. right-click selects the remembered tri-palette directly
5. selected tri-palette id is passed to render code as `tri_<name>`
6. backend validators accept generated tri palette ids
7. C runtime palette lookup accepts generated tri palette ids
8. `TRI` hover/title shows the selected canonical name
9. tri-palette definitions come from `color_dicts.py`, not hand-copied JS/C lists
10. tests cover generation, frontend selection, and backend acceptance
11. missing `tri_palette_catalog_js.js` degrades gracefully instead of breaking palette selection
12. Python palette validation no longer duplicates palette-name sets across handlers

## 18. Implementation Notes

### 18.1 `COLOR_LONG_STRINGS`

[color_dicts.py](/Users/nicknassuphis/karpo_hackathon/polypaint/color_dicts.py) also contains longer multi-stop palettes in `COLOR_LONG_STRINGS`.

They are explicitly out of scope for this refactor.

This document only covers:

- `COLOR_TRI_STRINGS`

The long-string palettes are a possible follow-up once the tri-palette generator and validation path are stable.

### 18.2 Generator math dependency

The generator needs the hue-circle interpolation methodology from [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py), but it does not need to import the full colorization module.

Preferred implementation:

- use only the Python standard library for the generator if practical
- reimplement the small hue-circle interpolation math directly

Acceptable fallback:

- use NumPy inside the generator only

Constraints:

- no NumPy dependency in runtime JS/C/Lambda execution paths
- no import of broader rendering/colorization behavior from [colors.py](/Users/nicknassuphis/karpo_hackathon/polypaint/colors.py)

The important design rule is small, deterministic generation-time math, not strict avoidance of NumPy at build time.

### 18.3 `roots2pix.c`

[lambda/roots2pix.c](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/roots2pix.c) already includes [lambda/palette_lut.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_lut.h) and uses the shared runtime palette lookup.

So this refactor should not assume there is still an active second palette registry there.

The required cleanup in that file is narrower:

- verify no active duplicate palette table remains outside the shared header
- remove or leave inert any git-blame-only dead block if desired
- update usage/help text so it no longer enumerates a small hardcoded built-in palette list

The main runtime palette work remains in:

- [lambda/palette_lut.h](/Users/nicknassuphis/karpo_hackathon/polypaint/lambda/palette_lut.h)
