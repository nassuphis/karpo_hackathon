# Registry-Generated Editor Help Plan

## Objective

Add generated, context-sensitive help to the text-first program editors without introducing static help files that drift from the parser/registry.

Initial scope:

- Add `Starter | Help` tabs to the Param Program and Coeff Program side panels.
- Add a double-click inspector on the Param/Coeff textareas.
- Generate all help content from existing registries/catalogs.
- Extend registries only where metadata is missing; do not hand-maintain a separate prose help file.

Non-goals for this pass:

- No Monaco/CodeMirror migration.
- No autocomplete yet.
- No backend route for help metadata.
- No static Markdown rendered into the app.

## Current State

The Compute tab already has two text-first program editors:

- `#pp-source-text` with `#pp-cheatsheet`
- `#cp-source-text` with `#cp-cheatsheet`

The current cheatsheet renderer is in `js/08-chip-editors.js`:

- `_renderProgramSourceCheatsheet(...)`
- `_renderParamProgramCheatsheet()`
- `_renderCoeffProgramCheatsheet()`
- `_paramProgramCheatSections`
- `_coeffProgramCheatSections`

The current layout/CSS is in `index.html`:

- `.program-source-grid`
- `.program-source-cheatsheet`
- `.program-source-cheat-section`
- `.program-source-cheat-button`

Useful registries already available in the browser:

- Param legacy transforms:
  - `_ptCatalog`
  - `_ptInfo`
  - `_paramProgramLegacyNames`
  - `_ptCategoryMeta`
- Coeff native transforms:
  - `_ctCatalog`
  - `_ctCategoryMeta`
  - `_coeffProgramLegacyNames`
- Coeff structural grammar/vocabulary:
  - `_coeffRegistryVocab`
  - `_coeffStructuralChip(...)`
  - `_coeffFamilySubOps(...)`
  - `_coeffRegistrySourceName(...)`
- Coefficient function catalog:
  - `window._coeffFuncCatalog` generated from `lambda/coeff_func_catalog.json`
  - loaded through `coeff_func_catalog_js.js`
- Coefficient function parameter inputs:
  - `updateCfpvRow()`
  - `parseCfpv()`
  - `_cfpv`

Important distinction:

- `cfpv` parameters are coefficient-function parameters, e.g. `giga_139(int1,int2,int3,deg)`.
- `andy` is not a coefficient-function parameter. It is a coefficient-transform/program blend parameter for native transforms, exposed through `_ctAndyParam` and related transform metadata.

Current coefficient functions with real CFPV params:

- `const(length, value_re, value_im)`
- `creative8(n)`
- `creative9(n)`
- `giga_139(int1, int2, int3, deg)`

## Design Rule

All help must be registry-generated.

Allowed:

- Render help from `_paramProgramCheatSections`, `_coeffProgramCheatSections`, `_ptCatalog`, `_ctCatalog`, `_coeffRegistryVocab`, and `window._coeffFuncCatalog`.
- Add `help`, `signature`, `examples`, `returns`, `aliases`, `params`, `bounds`, or `category` fields to registries when missing.
- Generate a docs artifact from the same registry later if wanted.
- Render a clear degraded state if a registry root is absent, e.g. `_coeffRegistryVocab === null`. Missing registry roots are not the same as sparse per-item metadata and must not crash the editor.

Not allowed:

- Hand-maintained HTML help blobs.
- Separate static Markdown that must be kept in sync manually.
- Hardcoded per-token help in the inspector except as a temporary registry gap with a test forcing migration.

## UI: Starter | Help Tabs

Replace each cheatsheet box with a small tabbed side panel:

```html
<div class="program-source-sidepanel" data-program-help="pp">
  <div class="program-source-help-tabs">
    <button ...>Starter</button>
    <button ...>Help</button>
  </div>
  <div id="pp-cheatsheet" class="program-source-cheatsheet ..."></div>
  <div id="pp-help" class="program-source-help ..."></div>
</div>
```

Same for `cp`:

- `#cp-cheatsheet`
- `#cp-help`

Keep dimensions identical to the current cheatsheet:

- 220px height.
- Scrollable.
- Aligned with the textarea bottom.

Tab behavior:

- Default tab is `Starter`.
- `Starter` shows the existing snippet buttons unchanged.
- `Help` shows generated reference sections.
- Switching tabs should not change textarea content.
- Store tab state in a small JS object, not localStorage for v1.

Suggested names:

```js
let _programSourceSidePanelMode = { pp: 'starter', cp: 'starter' };
function _setProgramSourceSidePanelMode(which, mode) { ... }
function _renderProgramSourceSidePanel(which) { ... }
```

## Generated Help Content

### Param Program Help

Generate sections from registries and parser vocabulary:

- Core symbols:
  - `t1`, `t2`: input parameters.
  - `p1`, `p2`: output/current parameter registers.
  - `pi`, `pi2`, `pi2i`: constants.
- Statements:
  - `p1 = expr`
  - `p2 = expr`
  - `push(expr)`
  - `emit_p1`
  - `emit_p2`
  - `macro(name)`
- Stack ops:
  - `dup`, `swap`, `pop`, `flush`
- Arithmetic/unary ops:
  - generated from the param source grammar/registry if available.
  - if not available, move the existing cheat section arrays into a shared help registry so snippets and help use the same source.
- Legacy transforms:
  - generated from `_ptCatalog` / `_ptInfo`.
  - include category, aliases if any, parameter defaults, and help text.

### Coeff Program Help

Generate sections from:

- `_coeffProgramCheatSections` for starter snippets.
- `_coeffRegistryVocab` for grammar vocabulary, through the existing accessor helpers where possible.
- `_ctCatalog` for native transform signatures and params.
- `window._coeffFuncCatalog` only for coefficient-function metadata, not coeff-program syntax.

If `_coeffRegistryVocab === null`, render a visible `Coeff registry not loaded` row in the Help tab and continue rendering any available static/derived sections that do not require the vocab. Do not render an empty panel and do not throw.

Required sections:

- Core symbols:
  - `cf`: immutable input coefficient vector.
  - `poly`: current/output coefficient vector.
  - `poly_len`: current coefficient vector length.
  - `t1`, `t2`, `p1`, `p2`: scalar context values.
  - `tos[i]`: top-of-stack indexed read.
- Statements:
  - `cf`
  - `poly`
  - `emit`
  - `poly = expr`
  - `poly[i] = expr`
  - `push_vec(...)`
  - `push_scalar(...)`
  - `macro(name)`
- Stack ops:
  - `dup`, `swap`, `drop`, `flush`, `pop`, `peek`
- Vector ops:
  - generated via `_coeffStructuralChip(...)`, `_coeffFamilySubOps(...)`, and `_coeffRegistrySourceName(...)`.
  - do not parse `_coeffRegistryVocab.structuralChips` fields directly in the help renderer.
  - include aliases through the existing source-name/alias accessors so help, snippets, and serialization do not drift.
- Native transforms:
  - generated from `_ctCatalog`.
  - include params and defaults.
  - render `_ctCatalog[name].params` or `_coeffProgramParamDefs(name)` directly. `andy` is already appended to every registry transform by catalog construction, so the help renderer should not special-case it.
- Coefficient function params:
  - generated from `window._coeffFuncCatalog`.
  - show only entries with `params.length > 0` in a compact "Function params" section.
  - do not imply these are coeff-program statements.

## Registry Metadata Additions

Add missing metadata to registries rather than hardcoding help text.

Recommended normalized help item shape:

```js
{
  name: 'roots',
  signature: 'roots(source, k, hi|lo)',
  aliases: ['roots_cm'],
  params: [
    { name: 'k', default: '6', help: 'Number of roots/terms.' },
    { name: 'side', default: 'hi', values: ['hi', 'lo'], help: 'Select high or low side.' },
    { name: 'andy', default: '0', help: 'Blend transformed output with original poly.' }
  ],
  category: 'roots',
  help: 'Replace coefficients with root-derived vector data.',
  examples: ['poly = roots(poly, 6, hi)\nemit']
}
```

Where metadata should live:

- Param transform help: `_ptCatalog` / `_ptInfo` source registry.
- Coeff native transform help: `_ctCatalog` / registry source.
- Coeff structural op help: `_coeffRegistryVocab`.
- Coeff function params: `lambda/coeff_func_catalog.json`, generated into `coeff_func_catalog_js.js`.

If a registry lacks a field:

- v1 may render a sparse row.
- Add a test that the help renderer tolerates missing fields.
- Add metadata incrementally where the UI feels too bare.

If a whole registry root is missing:

- `_coeffRegistryVocab === null` must produce a visible degraded help state.
- `_ctCatalog` / `_ctCategoryMeta` fallback empties are acceptable, but the Help tab should still explain that coeff registry-backed help is unavailable.
- Add a frontend test that forces `_coeffRegistryVocab = null` and verifies the Coeff Help tab does not crash.

## Double-Click Inspector

Add double-click handlers to:

- `#pp-source-text`
- `#cp-source-text`

Suggested functions:

```js
function _programWordAtTextareaCursor(textarea) { ... }
function _lookupProgramHelpToken(which, token) { ... }
function _openProgramHelpPopup(which, token, textarea) { ... }
function _closeProgramHelpPopup() { ... }
```

Interaction:

- Double-click a word/token in the textarea.
- First read the browser-selected text with `textarea.value.slice(selectionStart, selectionEnd)`. Native textarea double-click already selects the word/token in most browsers.
- Fall back to word-boundary scanning around `selectionStart` only when the selection is empty, e.g. for a future keyboard-triggered inspector.
- Normalize token:
  - strip whitespace.
  - for `poly[0]`, lookup `poly` and indexed-write/read help.
  - for `roots(...)`, lookup `roots`.
  - for `emit_norm(...)` style future cases, lookup function before `(`.
- Lookup generated help registry for the current editor kind:
  - `pp`: param symbols/statements/transforms.
  - `cp`: coeff symbols/statements/transforms/coefficient-function params.
- Show compact popup near the textarea selection or mouse position.
- Popup includes:
  - token name.
  - signature.
  - category.
  - parameter list/defaults.
  - one example/snippet button if available.
- Escape/click outside closes popup.

Fallback:

- If token unknown, show `No generated help for "token"` and a short hint: `Try Help tab for available symbols.`
- Do not silently do nothing; that makes the feature feel broken.

Plain textarea positioning:

- Exact caret coordinates are hard in a textarea.
- v1 can position at the double-click mouse event coordinates.
- If keyboard-triggered help is later added, position the popup near the textarea top-right.

## Help Registry Builder

Create a generated in-memory registry at render time:

```js
function _buildProgramHelpRegistry(which) {
  return {
    symbols: new Map(),
    statements: new Map(),
    ops: new Map(),
    transforms: new Map(),
    functions: new Map(),
  };
}
```

Do not store this as a static file. It is derived from loaded JS registries.

Cache it with invalidation:

```js
let _programHelpRegistryCache = { pp: null, cp: null };
function _programHelpRegistry(which) {
  if (!_programHelpRegistryCache[which]) {
    _programHelpRegistryCache[which] = _buildProgramHelpRegistry(which);
  }
  return _programHelpRegistryCache[which];
}
```

Invalidate after catalog load if needed:

- `populateDropdown()` currently runs after `window._coeffFuncCatalog` is available.
- The concrete hook is `populateDropdown()`: set `_programHelpRegistryCache.cp = null` there, immediately before re-rendering coeff help/side panels. That is the one place the UI knows the generated coefficient-function catalog has changed.

## Source Organization

Recommended code location:

- Keep renderer/helpers in `js/08-chip-editors.js` because it already owns program editor snippets and source UI.
- Keep raw catalog definitions where they are.
- Add CSS in `index.html` alongside current `.program-source-cheatsheet` styles.

New functions in `js/08-chip-editors.js`:

- `_renderProgramSourceSidePanels()`
- `_renderProgramSourceHelp(which)`
- `_programHelpRegistry(which)`
- `_buildParamProgramHelpRegistry()`
- `_buildCoeffProgramHelpRegistry()`
- `_programHelpItemHtml(item)`
- `_programHelpSectionHtml(title, items)`
- `_onProgramSourceDblClick(which, event)`
- `_lookupProgramHelpToken(which, token)`
- `_openProgramHelpInspector(which, token, item, event)`
- `_closeProgramHelpInspector()`

Update existing:

- `_renderParamCoeffProgramCheatsheets()` should render both side panels.
- `#pp-source-text` and `#cp-source-text` get `ondblclick` handlers.

## Test Plan

Add frontend harness checks in `tests/test_frontend_js.sh`:

- `Starter` tab renders existing snippet buttons.
- `Help` tab renders generated sections for Param and Coeff.
- Coeff Help tolerates `_coeffRegistryVocab === null`, shows a `Coeff registry not loaded` state, and does not throw.
- Coeff Help includes `andy` from registry-derived metadata, not hardcoded UI prose.
- Coeff Help includes coefficient-function params from `window._coeffFuncCatalog`, e.g. `giga_139` params.
- Double-click `poly` in `cp-source-text` opens inspector with `poly` help.
- Double-click `andy` or a native transform token opens transform help.
- Double-click unknown token opens an explicit unknown-token message.
- Existing sequential-load and part-registration checks still pass.

Optional Python/catalog tests:

- `tests/test_coeff_catalog_consistency.py` should assert that every `params` entry in `coeff_func_catalog.json` has a `name` and `default`. This passes today and becomes a cheap guard for generated coefficient-function help.
- Add registry consistency tests only if new required help fields are introduced.

## Implementation Order

1. Add side-panel HTML wrappers around existing `pp/cp` cheatsheet containers.
2. Add CSS for tabs/help rows/inspector popup.
3. Add side-panel mode state and render functions.
4. Build minimal generated help registries from existing sources.
5. Render Help tab for Param and Coeff.
6. Add double-click token extraction and popup.
7. Add tests for Help tab and double-click inspector.
8. Fill registry metadata gaps where the generated help is too sparse.

## Recommendation

Implement the Help tab first, then the double-click inspector.

The Help tab proves the registry generation and gives immediate value. The inspector should reuse the same registry, so it becomes a thin lookup/display layer rather than a second help system.
