#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
HTML="$ROOT/index.html"

if [ ! -f "$HTML" ]; then
  echo "FATAL: $HTML not found"
  exit 1
fi

grep -q 'Color render is fused-only now. Solve score is the only supported mode.' "$HTML" || { echo "FATAL: fused-only color banner missing"; exit 1; }
grep -q 'id="render-mt-fused-raster-workers"' "$HTML" || { echo "FATAL: fused raster workers input missing"; exit 1; }
grep -q 'id="render-mt-fused-save-associated-palette"' "$HTML" || { echo "FATAL: fused associated palette checkbox missing"; exit 1; }
grep -q 'No temp files, no physical chunks, no separate histogram or palette-chunk stages.' "$HTML" || { echo "FATAL: ColorRender-MT intro missing"; exit 1; }
! grep -q 'id="render-mt-tab-classic"' "$HTML" || { echo "FATAL: classic render MT tab should be removed"; exit 1; }
! grep -q 'id="render-mt-classic-panel"' "$HTML" || { echo "FATAL: classic render MT panel should be removed"; exit 1; }
! grep -q 'id="render-mt-raster-input-mode"' "$HTML" || { echo "FATAL: classic raster input selector should be removed"; exit 1; }
! grep -q 'id="render-mt-hist-retries"' "$HTML" || { echo "FATAL: classic hist retries control should be removed"; exit 1; }
! grep -q 'id="render-generate-popup-overlay"' "$HTML" || { echo "FATAL: legacy color Generate popup should be removed"; exit 1; }
! grep -q 'id="btn-render-generate-from-palette"' "$HTML" || { echo "FATAL: GenerateFromPalette action should be removed"; exit 1; }
! grep -q 'id="generate-from-palette-popup-overlay"' "$HTML" || { echo "FATAL: GenerateFromPalette popup should be removed"; exit 1; }
! grep -q 'id="autolevel-auto-gamma"' "$HTML" || { echo "FATAL: autolevel auto-gamma should not expose a fake mode selector"; exit 1; }

node - "$HTML" <<'NODE'
const fs = require('fs');

const htmlPath = process.argv[2];
const htmlSrc = fs.readFileSync(htmlPath, 'utf8');
// Generated registry vocab: chip param shapes/descs/titles live there now
// (lambda/coeff_legacy_registry.json ui blocks -> coeff_vocab_js.js).
const vocabSrc = fs.readFileSync(require('path').join(require('path').dirname(htmlPath), 'coeff_vocab_js.js'), 'utf8');
const solveVocabSrc = fs.readFileSync(require('path').join(require('path').dirname(htmlPath), 'solve_score_vocab_js.js'), 'utf8');

function fail(message) {
  console.error('FATAL: ' + message);
  process.exit(1);
}

// The app's JS is split into ordered js/ parts (classic scripts; see the
// <script src="js/..."> tags). Assemble them in tag order — exactly what the
// browser executes — and fail if the tags and the files on disk disagree.
const path = require('path');
const baseDir = path.dirname(htmlPath);
const partNames = [...htmlSrc.matchAll(/<script src="js\/([^"?]+\.js)"><\/script>/g)].map(m => m[1]);
if (!partNames.length) fail('no js/ part tags found in index.html');
const diskParts = fs.readdirSync(path.join(baseDir, 'js')).filter(f => f.endsWith('.js')).sort();
if (JSON.stringify(diskParts) !== JSON.stringify([...partNames].sort())) {
  fail('js/ files on disk do not match index.html script tags: tags=' + partNames.join(',') + ' disk=' + diskParts.join(','));
}
const src = htmlSrc + '\n' + partNames.map(n => fs.readFileSync(path.join(baseDir, 'js', n), 'utf8')).join('\n');

function assertIncludes(snippet, message) {
  if (!src.includes(snippet)) fail(message);
}

function assertNotIncludes(snippet, message) {
  if (src.includes(snippet)) fail(message);
}

function assertSectionNotIncludes(section, snippet, message) {
  if (!section) fail('missing source section for: ' + message);
  if (section.includes(snippet)) fail(message);
}

assertIncludes("function setColorMode(mode) {\n    renderColorMode = 'solve_score';\n    _updateSolveScoreButtons();", 'setColorMode should hard-lock solve_score without a fake mode toggle');
assertIncludes("<div class=\"color-title\" style=\"margin-top:10px\">Color</div>", 'render tab should merge color controls into the visual View/Rotation region');
assertIncludes("class=\"color-mode-choice\"><input type=\"radio\" name=\"render-color-interpretation\" value=\"scalar_lut\"", 'Scalar LUT selector should use aligned color-mode-choice markup');
assertIncludes(".color-mode-row label:not(.color-mode-choice)", 'render color generic label CSS must not override color-mode-choice grid alignment');
assertIncludes(".color-mode-row .color-mode-choice {\n    display: grid;", 'color mode choices should use fixed selector/text grid alignment with enough specificity');
assertIncludes(".color-mode-row .color-mode-choice input[type=\"radio\"] {\n    margin: 0;", 'color mode radio inputs should override browser/default label spacing');
assertIncludes(".color-mode-name {\n    display: block;\n    line-height: 13px;", 'color mode names should use explicit line-height to avoid low text alignment');
assertIncludes("id=\"render-background-color\" class=\"render-background-color\" value=\"#000000\"", 'render tab should expose a native background color picker');
assertIncludes("id=\"render-background-hex\" class=\"render-background-hex\" value=\"000000\"", 'render tab should expose exact background hex input');
assertIncludes("id=\"render-background-eyedropper\"", 'render tab should expose a background pipette button');
assertIncludes(".render-background-eye {\n    margin: 0;", 'background pipette button should override global button top margin');
assertIncludes("id=\"render-pix\" value=\"4096\" max=\"32768\"", 'render pix input should expose the backend maximum');
assertIncludes("const RENDER_MAX_PIX = 32768;", 'render orchestration should clamp pix before dispatch');
assertIncludes("id=\"btn-palette-create\" class=\"btn-secondary btn-inline\" onclick=\"runPaletteArtifact()\">Generate Artifact</button>", 'palette tab action should be a clearly labeled generate artifact button');
assertNotIncludes("btn-palette-create\" class=\"btn-inline-offset\"", 'palette generate action should not live in the compact stack row');
assertIncludes("function _paletteRunBlocksNewRun() {", 'frontend should centralize active Palette/ExtractPalette lock checks');
assertIncludes("function _blockPaletteActionIfActive(actionLabel) {", 'frontend should make active Palette/ExtractPalette locks visible instead of silently ignoring clicks');
assertIncludes("if (_blockPaletteActionIfActive('ExtractPalette')) return;", 'ExtractPalette popup/run path should use the shared active-palette lock helper');
assertIncludes("extractPalBtn.disabled = !canOpenExtractPalette || !!_activeRenderRun || paletteRunBlocking;", 'ExtractPalette toolbar button should use the same active-palette lock predicate as the click path');
assertNotIncludes("if (_activeRenderRun || _activePaletteRun || _loadActivePaletteRun() || !art || !art.artifact_id) return;", 'ExtractPalette popup opener must not silently return on a persisted active palette run');
assertIncludes("const _resultPreviewInFlight = new Map();", 'Results preview lazy generation should dedupe in-flight preview requests per job');
assertIncludes("const _resultPreviewInFlightMode = new Map();", 'Results preview lazy generation should track whether an in-flight request is manual or automatic');
assertIncludes("const RESULTS_LAZY_PREVIEW_DELAY_MS = 350;", 'Results lazy preview should be debounced so arrow-key navigation does not launch preview writes for every visited row');
assertIncludes("function _cancelPendingLazyResultPreview(jobId = null) {", 'Results lazy preview should have a cancellable pending state');
assertIncludes("id=\"res-color-renders\"", 'Results sidebar should expose selected-result color render count');
assertIncludes("const _resultColorRenderCountInFlight = new Map();", 'Results color render count should dedupe in-flight storage requests');
assertIncludes("const RESULTS_COLOR_RENDER_COUNT_DELAY_MS = 350;", 'Results color render count should be debounced like lazy preview');
assertIncludes("function _cancelPendingColorRenderCount(jobId = null) {", 'Results color render count should have a cancellable pending state');
assertIncludes("lambdaPost('storage', { job_id: jobId }, '/render-count')", 'Results color render count should use the selected-result storage endpoint');
assertIncludes("function _syncResultPreviewActionButtons(jobId) {", 'Results preview button/delete state should be synchronized from one helper');
assertIncludes("previewBtn.textContent = previewInFlight ? (mode === 'manual' ? '...' : 'auto...') : 'Preview';", 'Selecting a result should restore the correct Preview button label for manual vs automatic in-flight work');
assertIncludes("if (deleteBtn) deleteBtn.disabled = previewInFlight;", 'Results Delete should be disabled while a selected preview Lambda may still write S3');
assertIncludes("const promise = Promise.resolve().then(async () => {", 'Results preview generation should populate the in-flight map before the async body can clean it up');
assertIncludes("const cachedPreviewUrl = r && r.has_preview && r.preview_url;", 'Results detail rendering should preserve a preview generated while a stale detail response was in flight');
assertIncludes("let needsLazyPreview = false;", 'Results detail rendering should defer lazy preview launch until after detail text is populated');
assertIncludes("needsLazyPreview = true;", 'Results detail rendering should mark missing previews for lazy generation');
assertIncludes("if (needsLazyPreview) _lazyGenerateResultPreview(jobId);", 'Results selection should lazily generate a missing persisted preview after detail rendering');
assertIncludes("if (_selectedJobId !== jobId) return;", 'Debounced Results lazy preview should not start after selection has moved to another result');
assertIncludes("if (r && r._previewAutoFailed) return;", 'Results lazy preview should not retry endlessly after an automatic preview failure');
assertIncludes("await _generateResultPreview(_selectedJobId, { lazy: false });", 'Manual Results Preview should share the same generator as lazy preview creation');
assertIncludes("const normalized = _normalizeRenderBackgroundColor(value, '', { allowShort: !options.fromText });", 'background hex typing should not commit 3-digit shorthand before blur/commit');
assertNotIncludes("<label><input type=\"radio\" name=\"render-color-interpretation\"", 'render color radios must not use bare label/input markup');
assertNotIncludes("class=\"color-dot active\" data-mode=\"solve_score\"", 'render tab should not expose a fake solve-score mode toggle');
assertIncludes("id=\"view-row-explicit\"", 'render tab should expose explicit viewport row');
assertIncludes("id=\"render-min-re\"", 'render tab should expose explicit min_re input');
assertIncludes("id=\"render-max-re\"", 'render tab should expose explicit max_re input');
assertIncludes("id=\"render-min-im\"", 'render tab should expose explicit min_im input');
assertIncludes("id=\"render-max-im\"", 'render tab should expose explicit max_im input');
assertIncludes("World aspect Δre:Δim", 'render explicit viewport should show world-aspect helper text');
assertIncludes(".render-preview-stage {\n    position: relative;\n    display: flex;", 'render artifact preview stage should be a constrained flex viewer, not natural-size crop');
assertIncludes("object-fit: contain;", 'render artifact preview image should contain rather than crop');
assertIncludes("function _renderPreviewImageRect(stage, img) {", 'render preview selection should compute the displayed image rect');
assertIncludes("const imageRect = _renderPreviewImageRect(stage, img);", 'render preview marquee should align to displayed image rect');
assertIncludes("target: 'color_repalette'", 'Color RePalette UI should dispatch through the Color RePalette target');
assertIncludes("new_interpretation: newInterpretation", 'Color RePalette UI should forward the selected 3-channel interpretation');
assertIncludes("id=\"color-repalette-interpretation-row\"", 'Color RePalette popup should expose the interpretation row');
assertIncludes("id=\"color-repalette-interpretation\"", 'Color RePalette popup should expose the 3-channel interpretation selector');
assertIncludes("function _selectedColorRepaletteInterpretation(art) {", 'Color RePalette UI should centralize selected interpretation parsing');
assertIncludes("{ name: 'identity', stops: ['#000000','#ffffff'] }", 'PAL swatch should include the identity greyscale palette');
assertIncludes("{ name: 'identity_hsv', stops:", 'PAL swatch should include the identity HSV palette');
assertIncludes("unit_circle: { params: [{ph:'target', def:'both', target: true}] }", 'unit_circle param transform should expose target selection');
assertIncludes("rtheta: { params: [{ph:'p', def:'1'}, {ph:'target', def:'both', target: true}] }", 'rtheta target selector should preserve legacy p-first ordering');
assertIncludes("square: { params: [{ph:'target', def:'both', target: true}] }", 'square param transform should expose target selection');
assertIncludes("id=\"param-program-manage\" onclick=\"openParamProgramModal()\"", 'compute tab should expose Param Programs modal launcher');
assertNotIncludes("id=\"param-pipeline-mode\"", 'compute tab should not expose retired Chain/Program selector');
assertNotIncludes("id=\"pp-chips\"", 'Param Program read-only chip display removed (text-only editor)');
assertNotIncludes("param-program-tab-chips", 'Param Program read-only chips tab removed (text-only editor)');
assertIncludes("id=\"pp-text-panel\" class=\"coeff-program-editor-panel active\"", 'Param Program text panel should stay active in the text-only editor');
assertIncludes("id=\"pp-source-text\" class=\"coeff-program-source-text\"", 'Param Program should expose a source textarea');
assertIncludes("id=\"pp-cheatsheet\" class=\"program-source-cheatsheet\"", 'Param Program should expose a source insert cheatsheet');
assertIncludes("function _insertParamProgramSourceSnippet(snippet) {", 'Param Program cheatsheet should insert snippets through the source input path');
assertIncludes("function _renderParamProgramCheatsheet() {", 'Param Program cheatsheet renderer should exist');
assertIncludes("function _paramProgramTextModeSelected() {", 'Param Program text mode should have a single source of truth');
assertIncludes("function _paramProgramSourceFromRows(chain) {", 'Param Program should synthesize editable source from chip chains');
assertIncludes("lambdaPost('storage', { source_text: sourceText }, '/compile-param-program-source')", 'Param Program text editor should validate against the backend parser');
assertIncludes("function _selectedParamPipelineMode() {", 'frontend should centralize selected parameter pipeline mode');
assertIncludes("function _formatChainRowsForLog(chain, separator = ',') {", 'compute preview logging should use a shared safe chain formatter');
assertIncludes("return _formatCoeffProgramChainForLog(_serializeCoeffProgramChain(), separator);", 'Program-only coeff preview logging should format the read-only program chip view when text is inactive');
assertIncludes("function _computePreviewViewportInfoLines(result) {", 'compute preview plot info should format returned viewport bounds');
assertIncludes("viewport Re: ${minRe} .. ${maxRe}", 'compute preview plot info should show displayed Re viewport bounds');
assertIncludes("viewport Im: ${minIm} .. ${maxIm}", 'compute preview plot info should show displayed Im viewport bounds');
assertIncludes("const _paramProgramIndependentLegacyTargets = new Set([", 'frontend should know which legacy target args are redundant in Param Program bridge chips');
assertIncludes("'coeff2',\n    'coeff3',\n    'coeff3a',", 'Param Program legacy bridge should include legacy coeff-map names');
assertIncludes("coeff12: { category: 'bridge', desc: 'legacy mixed polynomial map' }", 'Param Program picker should expose legacy coeff-map chips directly');
assertIncludes("const _paramProgramLegacyArgSpecs = {", 'frontend should expose structured legacy argument specs in Param Program bridge chips');
assertIncludes("moebius: [\n        { ph: 'a', def: '1', scalarExpr: true, complexWide: true", 'Param Program moebius bridge should expose four wide complex expression coefficient inputs');
assertIncludes("function _paramProgramMoebiusArgsForUi(args) {", 'Param Program moebius bridge should convert legacy eight-real coefficients back to four complex UI fields');
assertIncludes("<span>t=(</span>${a}<span class=\"chip-op\">*t+</span>${b}<span>)/(</span>${c}<span class=\"chip-op\">*t+</span>${d}<span>)</span>", 'Param Program moebius bridge should render a formula-style chip');
assertIncludes("const _paramProgramLegacyTargetArgIndexes = {", 'Param Program should migrate legacy numeric target args into src/tgt selectors');
assertIncludes("crd: [{ ph: 'size', def: '1', scalarExpr: true, exprWide: true }]", 'Param Program targetable shape bridge should expose expression-sized shape fields without redundant target fields');
assertIncludes("function _paramProgramLegacyTakesNoArgs(legacyName) {", 'Param Program legacy bridge should know which selected legacy names take no args');
assertIncludes("inputDefs = pDefs.slice(0, 3);\n            inputValues = [legacyName, src, tgt];", 'Param Program legacy bridge should hide the args box for no-arg legacy functions');
assertIncludes("function _chipLabeledInputHtml(which, chipIdx, paramIdx, value, paramDef, options = {}) {", 'Param Program legacy bridge should render visible labels, not hidden placeholders');
assertIncludes("showParamLabels = item.name === 'legacy';", 'Param Program legacy bridge should label name/src/tgt and structured args');
assertIncludes(".chip .chip-input-expr-wide {\n    width: 28ch;", 'Chip expression fields should be wide enough for useful formulas');
assertIncludes(".chip .chip-input-selector-wide {\n    width: 8ch;", 'Chip selector fields should be compact but readable');
assertIncludes("{ ph: 'target', def: 'p1', choices: ['p1', 'p2'], selectorWide: true }", 'Param Program emit target selector should use compact selector sizing');
assertIncludes("{ ph: 'value', def: '0', scalarExpr: true, complexWide: true", 'Param Program const should use one wide expression field');
assertIncludes("Constants: pi, pi2, pi2i. Functions: exp, real, imag, abs, mod.", 'Param Program expression tooltips should advertise pi, pi2, and pi2i constants');
assertIncludes("{ ph: 'name', def: 'unit_circle', choices: _paramProgramLegacyNames, paramProgramWide: true }", 'Param Program legacy name selector should use the wider control');
assertIncludes("{ ph: 'src', def: 'both', choices: ['p1', 'p2', 'both', 'pop1', 'pop2'], selectorWide: true }", 'Param Program legacy src selector should use compact selector sizing');
assertIncludes("{ ph: 'tgt', def: 'both', choices: ['p1', 'p2', 'both', 'push1', 'push2'], selectorWide: true }", 'Param Program legacy tgt selector should use compact selector sizing');
assertIncludes("function _normalizeLegacyBridgeParams(legacyName, src, tgt, argsInput) {", 'frontend should strip redundant legacy target args from Param Program bridge chips');
assertIncludes("function _paramProgramBridgeParamsFromLegacyTransform(row) {", 'Copy legacy transforms should translate target args into src/tgt selectors');
assertIncludes("const bridgeArgs = _paramProgramLegacyArgsFromInput(name, name === 'moebius' ? args : migratedArgs,", 'Copy legacy moebius transforms should preserve four complex coefficients in the UI chain');
assertIncludes("function _serializeParamProgramChain() {", 'frontend should serialize param-program chips');
assertIncludes("function _effectiveParamTransformsForCompute() {", 'compute payload should centralize legacy-vs-param-program selection');
assertIncludes("function _effectiveParamProgramChainForCompute() {\n    return [];\n}", 'Param Program compute payload should never send editable chip chains');
assertIncludes("const sourceText = _getParamProgramSourceText();\n    return sourceText.trim() ? sourceText : null;", 'Param Program compute payload should send nonblank source text');
assertIncludes("param_program_chain: paramProgramChain,", 'compute/preview payloads should forward param_program_chain');
assertIncludes("savedParamProgramSourceText = _paramProgramSourceFromRows(savedParamProgramChain);", 'populate-from-result should synthesize Param source from legacy chain-only artifacts');
assertIncludes("if (paramSourceText !== null) payload.param_program_source_text = paramSourceText;", 'compute/preview payloads should forward param_program_source_text');
assertIncludes("lambdaPost('storage', {}, '/list-param-programs')", 'param-program modal should list saved programs through storage');
assertIncludes("lambdaPost('storage', { id }, '/fetch-param-program')", 'param-program modal should fetch saved programs through storage');
assertIncludes("lambdaPost('storage', { id }, '/delete-param-program')", 'param-program modal should delete saved programs through storage');
assertIncludes("}, '/save-param-program');", 'param-program modal should save programs through storage');
assertIncludes("id=\"param-program-modal-overlay\"", 'shared param-program modal markup missing');
assertIncludes("const hasSourceText = Object.prototype.hasOwnProperty.call(raw, 'source_text')", 'Param Program saved source_text payloads should load as text programs');
assertIncludes("if (program.has_source_text) {", 'Param Program source_text payloads should switch the editor to Text mode');
assertIncludes("<pre class=\"coeff-program-modal-source\" aria-label=\"Param program source\">", 'Param Program modal should render source-text programs as source');
assertIncludes("savedParamProgramSourceText", 'Compute result Populate should prefer stored param source text over lowered chip chains');
assertIncludes("const result = await lambdaPost('param-debug', _attachProgramSourcePayload({", 'Param Debug should forward Param source text through the shared payload helper');
assertIncludes("id=\"coeff-program-manage\" onclick=\"openCoeffProgramModal()\"", 'compute tab should expose Coeff Programs modal launcher');
assertNotIncludes("id=\"cp-chips\"", 'Coeff Program read-only chip display removed (text-only editor)');
assertNotIncludes("coeff-program-tab-chips", 'Coeff Program read-only chips tab removed (text-only editor)');
assertIncludes("id=\"cp-text-panel\" class=\"coeff-program-editor-panel active\"", 'Coeff Program text panel should stay active in the text-only editor');
assertIncludes("id=\"cp-source-text\" class=\"coeff-program-source-text\"", 'Coeff Program should expose a source textarea');
assertIncludes("id=\"cp-cheatsheet\" class=\"program-source-cheatsheet\"", 'Coeff Program should expose a source insert cheatsheet');
assertIncludes("function _insertCoeffProgramSourceSnippet(snippet) {", 'Coeff Program cheatsheet should insert snippets through the source input path');
assertIncludes("function _renderCoeffProgramCheatsheet() {", 'Coeff Program cheatsheet renderer should exist');
assertIncludes("const _coeffProgramCatalog = (() => {", 'frontend should build the coeff-program chip catalog');
assertIncludes("const _programProfiles = (typeof window !== 'undefined' && window._programProfiles)", 'frontend should consume the generated program profile mirror');
assertIncludes("function _profileSelectorChoices(name, fallback) {", 'Coeff Program selector choices should derive from profile metadata');
assertNotIncludes("function _coeffProgramPolySugarName(name) {", 'Coeff Program should not expose redundant poly-* legacy sugar helpers');
assertNotIncludes("const sugarName = _coeffProgramPolySugarName(name);", 'Coeff Program catalog should not create redundant poly-* direct sugars');
assertNotIncludes("label: sugarName", 'Coeff Program direct sugar labels should not expose poly-* names');
assertIncludes("label: 'push_vec'", 'Coeff Program vector-constant chip should be named push_vec');
assertIncludes("push_scalar: {", 'Coeff Program catalog should understand text-mode push_scalar');
assertIncludes("params: [{ ph: 'src', def: 'cf', choices: _profileSelectorChoices('push_src', ['cf', 'poly']), paramProgramWide: true }]", 'Coeff Program push source selector should derive from profile metadata and use the wider control');
assertIncludes("{ ph: 'length|poly_len', label: 'length', def: 'poly_len', lengthArg: true", 'Coeff Program const length should allow poly_len by default');
assertIncludes("{ ph: 'value expr', label: 'value', def: '1', scalarExpr: 'complex'", 'Coeff Program const value should be a scalar expression field');
assertIncludes("conj/real/imag/abs/angle/sqrt/log/exp/sin/cos/tan/sinh/cosh/tanh", 'Coeff Program scalar expression help should include elementary scalar functions');
assertIncludes("push a constant vector push_vec(length, value); value is a scalar expression", 'Coeff Program picker should explain vector constant length/value semantics clearly');
assertIncludes("push_linspace: {", 'Coeff Program picker should expose push_linspace');
assertIncludes("push linspace values from 0 to length using length entries", 'Coeff Program picker should explain push_linspace');
assertIncludes("push_range: {", 'Coeff Program picker should expose push_range');
assertIncludes("push Python-style range values 0, 1, ..., length-1", 'Coeff Program picker should explain push_range');
assertIncludes("poke_poly: {", 'Coeff Program picker should expose poke_poly');
assertIncludes("poke_tos: {", 'Coeff Program picker should expose poke_tos');
assertIncludes("write value into poly[index] without touching the stack", 'Coeff Program poke_poly should describe direct poly mutation');
assertIncludes("write value into the top stack vector without popping it", 'Coeff Program poke_tos should describe top-of-stack mutation');
assertIncludes("Program mode accepts t1/t2, p1/p2, poly_len, cfN, polyN, tosN, pi, pi2, pi2i, literals, + - * /, and conj/real/imag/abs/angle/sqrt/log/exp/sin/cos/tan/sinh/cosh/tanh.", 'Coeff Program scalar expression tooltip should name the allowed registers, constants, functions, and vector element reads');
assertIncludes("commit poly; pops stack top into poly when present", 'Coeff Program picker should describe emit commit semantics');
assertIncludes("blend below*(1-t) + top*t for same-length vectors", 'Coeff Program picker should expose vector blend chip');
assertIncludes("const _coeffProgramVectorBinaryNames = ['add', 'subtract', 'multiply', 'divide', 'power'];", 'Coeff Program picker should expose first-class vector binary ops');
assertIncludes("const _coeffProgramVectorUnaryNames = ['angle', 'mod', 'abs', 'neg', 'conj', 'sqrt', 'log', 'exp', 'sin', 'cos', 'tan', 'sinh', 'cosh', 'tanh'];", 'Coeff Program picker should expose first-class vector unary ops');
assertIncludes("argsort: {", 'Coeff Program picker should expose argsort');
assertIncludes("roll vector left by n positions", 'Coeff Program picker should expose roll-left');
assertIncludes("roll vector right by n positions", 'Coeff Program picker should expose roll-right');
assertIncludes("reorder src1 by ascending magnitude of src2", 'Coeff Program argsort should document key semantics');
assertIncludes("littlewood: {", 'Coeff Program picker should expose littlewood');
assertIncludes("generate a random Littlewood-style vector using current poly length", 'Coeff Program littlewood should document poly-length semantics');
assertIncludes("littlewood(</span>${input(1)}", 'Coeff Program littlewood should render as a formula chip');
assertIncludes("function _coeffProgramVectorFormulaHtml(item, i, pDefs, options = {})", 'Coeff Program vector chips should render as formula chips');
assertIncludes("id=\"compute-preview-tab-debug\"", 'Compute Preview should include a Compute Debug tab');
assertIncludes("id=\"compute-debug-u\"", 'Compute Debug should expose u input');
assertIncludes("id=\"compute-debug-v\"", 'Compute Debug should expose v input');
assertIncludes("runComputeDebug('param')", 'Compute Debug should expose Param eval action');
assertIncludes("runComputeDebug('poly')", 'Compute Debug should expose Poly eval action');
assertIncludes("runComputeDebug('solve_ae')", 'Compute Debug should expose SolveAE eval action');
assertIncludes("runComputeDebug('solve_cm')", 'Compute Debug should expose SolveCM eval action');
assertIncludes("debug_stage: stage", 'Compute Debug payload should route through compute-preview debug_stage');
assertIncludes("const PARAM_DEBUG_MAX_N = 512;", 'Param debug image should have a sync-safe N cap');
assertIncludes("function _paramDebugRequestSettings() {", 'Param debug image should derive preview-scale request settings');
assertIncludes("const { n, pix, notes } = _paramDebugRequestSettings();", 'Param debug image should use capped preview settings');
assertIncludes("pix,\n            job_id:", 'Param debug image should forward capped pix instead of N*2');
assertNotIncludes("pix: n * 2,", 'Param debug image must not derive pix from full Calculate N');
assertIncludes("function _formatComputeDebugResult(result) {", 'Compute Debug should format native single-point output');
assertIncludes("if (body.message) parts.push(_clipErrorText(body.message, 260));", 'frontend Lambda error summaries should include validation message bodies');
assertIncludes("function _showPdfHardStaleAbandon(statusEl, run, phase) {", 'PDF artifact stale handling should expose a PDF-specific abandon action');
assertIncludes("btn.textContent = 'Abandon stalled PDF job';", 'PDF artifact stale handling should show a local abandon button');
assertIncludes("_clearActiveRun();\n        stopActiveRenderObserver();", 'PDF artifact stale abandon should clear the local active render lock and observer');
assertIncludes("try { _updateRenderActionButtons(); } catch(e) {}", 'clearing an active render run should refresh render action buttons');
assertIncludes("PDF compose stalled - no update for 15+ min", 'PDF artifact hard-stale logging should use PDF-specific single-shot wording');
assertIncludes("PDF compose has not updated for 5+ min", 'PDF artifact warning stale logging should avoid worker-loop language');
assertIncludes("function _serializeCoeffProgramChain() {", 'frontend should serialize coeff-program chips');
assertIncludes("function _validateCoeffProgramUiChain(chain) {", 'Coeff Program saved-program load should validate stale/unknown chips immediately');
assertIncludes("unknown coeff program chip at", 'Coeff Program stale poly-* saved programs should fail with a clear load-time error');
assertIncludes("const normalizedChain = hasSourceText ? [] : _normalizeCoeffProgramChain(chain);", 'Coeff Program source_text payloads should not validate compiler-internal lowered chains as chip UI');
assertIncludes("if (!hasSourceText) _validateCoeffProgramUiChain(normalizedChain);", 'Coeff Program chip validation should apply only to chip-authored programs');
assertIncludes("if (program.has_source_text) {", 'Coeff Program source_text payloads should load into Text mode even when source_text is empty');
assertIncludes("pipeline.coeff_program_source_text,\n        detail.coeff_program_source_text,\n        calc.coeff_program_source_text,\n        pipeline.coeff_program && pipeline.coeff_program.source_text", 'Compute result Populate should prefer stored coeff source text over lowered chip chains');
assertIncludes("id=\"solve-score-modal-migrate\"", 'Solve-score saved-program modal should expose the Legacy v2 migration action');
assertIncludes("id=\"param-program-modal-migrate\"", 'Param saved-program modal should expose the Legacy v2 migration action');
assertIncludes("id=\"coeff-program-modal-migrate\"", 'Coeff saved-program modal should expose the Legacy v2 migration action');
assertIncludes("'/migrate-solve-score-program'", 'Solve-score modal should call the v2 migration route');
assertIncludes("'/migrate-param-program'", 'Param modal should call the v2 migration route');
assertIncludes("'/migrate-coeff-program'", 'Coeff modal should call the v2 migration route');
assertIncludes("} else if (options.auto === false || value.trim()) {\n        _coeffProgramSourceAutoSynthed = false;", 'Restored coeff source text should clear auto-synth state before switching to Text mode');
assertIncludes("function _effectiveCoeffProgramChainForCompute() {", 'compute payload should centralize coeff-program selection');
assertIncludes("function _effectiveCoeffProgramChainForCompute() {\n    return [];\n}", 'Coeff Program compute payload should never send editable chip chains');
assertIncludes("function _copyCoeffTransformsIntoCoeffProgram() {", 'Coeff Program UI should translate legacy transforms into program chips');
assertIncludes("const _coeffProgramRegistryChipNames = _coeffRegistryVocab ? _coeffRegistryVocab.chipNameByRegistryName : {};", 'normalize/copy should derive the registry-to-chip name map from the generated vocab');
assertIncludes('<script src="program_profiles_js.js"></script>', 'the generated program profile mirror must load before the main bundle');
assertIncludes('<script src="merged_opcodes_js.js"></script>', 'the generated merged opcode mirror must load before the main bundle');
assertIncludes('<script src="coeff_vocab_js.js"></script>', 'the generated registry vocab must load before the main bundle');
assertIncludes('<script src="solve_score_vocab_js.js"></script>', 'the generated solve-score vocab must load before the main bundle');
assertIncludes("return { name: _coeffProgramRegistryChipName(normalized.name), params: ['poly', 'poly', ...args] };", 'Copy legacy transforms should map shadowed registry names through the shared chip-name map');
assertIncludes("return [{ name: _coeffProgramRegistryChipName(legacyName), params: [legacyTgt, legacySrc, ...legacyArgs] }];", 'Normalize should map shadowed registry names through the shared chip-name map');
assertIncludes("// LAYOUT CONTRACT: legacy rows are source-first", 'The legacy-vs-chip param order flip must stay documented at the normalize seam');
assertIncludes("function _isAndyParam(pDef) {", 'andy identity should be real metadata (kind), not placeholder text');
assertIncludes("return name === 'const' ? 'push_const' : name;", 'coeff program chip canonicalizer must map const to push_const (a self-call here recursed forever)');
assertIncludes("kind: 'andy', ph: 'andy'", 'the base andy param def should carry the kind marker');
assertIncludes("catalog.power_series = {", 'Coeff Program catalog should expose the registry power transform as a power_series chip');
assertIncludes("return { name: 'legacy', params: [normalized.name, 'poly', 'poly', ...args] };", 'Copy legacy transforms must keep andy-carrying rows as legacy chips (named chips drop andy)');
assertIncludes("if (_coeffProgramSourceAutoSynthed && !_coeffProgramChain.length) {", 'Emptying the chip chain must clear stale auto-synthesized text');
assertIncludes("function _coeffProgramSourceStatements(sourceText) {", 'statement counter and display should share one backend-mirroring splitter');
assertIncludes("nativeTransform: true,", 'Coeff Program picker should expose direct native transform chips instead of a visible legacy wrapper');
assertIncludes("catalog.exp_affine = {", 'Coeff Program picker should expose affine exponential as exp_affine');
assertIncludes("desc: 'exp_affine(src, a, b): exp(src*a+b)'", 'Coeff Program exp_affine chip should document native affine exponential semantics');
assertIncludes("hidden: true,\n            params:", 'Coeff Program compatibility-only legacy chip should be hidden from authoring menus');
assertIncludes("coeff_program_chain: coeffProgramChain,", 'compute/preview payloads should forward coeff_program_chain');
assertIncludes("savedCoeffProgramSourceText = _coeffProgramSourceFromRows(savedCoeffProgramChain);", 'populate-from-result should synthesize Coeff source from legacy chain-only artifacts');
assertIncludes("Coeff Program scalar args are parsed by the compiler. Keep expressions", 'Coeff Program editor should not reject p1/p2 scalar expressions with legacy numeric validation');
assertIncludes("return { value: rawText || String(pDef.def || '') };", 'Coeff Program scalar expression fields should preserve raw expression text');
assertIncludes("function _coeffProgramLegacyFormulaHtml(i, legacyName, values, legacyDefs, options = {})", 'generic Coeff Program legacy chips should render as formulas instead of labeled dumps');
assertIncludes("chip-input-selector-wide", 'Coeff Program legacy source/target selectors should be wide enough to read');
assertIncludes("chip-input-function-wide", 'Coeff Program legacy function selectors should have a readable but bounded width');
assertIncludes("chip-input-program-wide", 'Program macro chip inputs should be wide enough for saved program ids');
assertIncludes("chip-input-complex-wide", 'Coeff Program complex expression fields should be wide enough to read');
assertIncludes("legacyName === 'exp'", 'Coeff Program legacy exp chip should render as a formula with source/target selectors');
assertIncludes("if (item.name === 'exp_affine') {", 'Coeff Program exp_affine chip should render the affine formula (only exp_affine reaches the nativeTransform renderer)');
if (!vocabSrc.includes("exp(z*field1+field2)")) fail('generated vocab should document complex multiplier plus offset semantics for the exp chip');
if (!solveVocabSrc.includes("mean_log_mod")) fail('generated solve-score vocab should expose solve-score metric names');
if (!solveVocabSrc.includes("angular_entropy_16")) fail('generated solve-score vocab should expose generic metric names');
assertIncludes("legacyName === 'round'", 'Coeff Program legacy round chip should render as a formula with source/target selectors');
if (!vocabSrc.includes("Complex multiplier in round(z*multiplier)")) fail('generated vocab should expose one compact complex multiplier field for round');
assertIncludes("${fn}<span>(</span>${src}<span class=\"chip-op\">*</span>${field1}<span>)</span>${andy}", 'Coeff Program round formula should not waste UI on field1+i*field2 formatting');
assertIncludes("Legacy coefficient transform function. Compiled to a stable numeric function index.", 'Coeff Program legacy function selector should explain what it selects');
assertIncludes("Input vector: cf read-only coefficients, current poly, pop stack, or peek stack.", 'Coeff Program legacy src selector should have a tooltip');
assertIncludes("Output target: write poly or push the result onto the stack.", 'Coeff Program legacy tgt selector should have a tooltip');
assertIncludes("Blend amount in [0,1]. ${_coeffProgramScalarExprHelp}", 'Coeff Program andy fields should advertise expression support');
assertIncludes("const _coeffProgramScalarExprHelp = 'Program mode accepts t1/t2, p1/p2, poly_len, cfN, polyN, tosN, pi, pi2, pi2i, literals, + - * /, and conj/real/imag/abs/angle/sqrt/log/exp/sin/cos/tan/sinh/cosh/tanh.';", 'Coeff Program expression help should advertise pi, pi2, pi2i, and elementary scalar functions');
assertIncludes("function _parseCtComplexConstant(value) {", 'frontend should parse complex constants consistently for coefficient inputs');
assertIncludes("function _formatCfpvForDisplay(funcName, cfpv) {", 'coefficient function parameters should have a logical display formatter');
assertIncludes("return `degree=${degree}, value=${_formatCfpvComplexValue(re, im)}`;", 'const coefficient function should display degree plus one complex value');
assertIncludes("if (_isConstCoeffFunction(entry)) {\n        const defaults = _constCoeffDefaults(entry);", 'const coefficient function should use a two-field logical UI');
assertIncludes("_cfpv = (degreeValid && valueValid) ? [degree + 1, value.re, value.im] : [];", 'const coefficient UI should serialize degree/value to native length/re/im CFPV');
assertIncludes("degreeInp.title = 'Polynomial degree; native coefficient length is degree + 1.';", 'const coefficient degree input should explain degree-to-length encoding');
assertIncludes("valueInp.title = 'Complex constant value, e.g. 1-2j or 10j-3.';", 'const coefficient value input should accept a single complex constant');
assertIncludes("_setConstCoeffInputsFromRaw(savedCfpv);", 'populate-from-result should convert raw const CFPV back to degree/value UI fields');
assertIncludes("lambdaPost('storage', {}, '/list-coeff-programs')", 'coeff-program modal should list saved programs through storage');
assertIncludes("lambdaPost('storage', { id }, '/fetch-coeff-program')", 'coeff-program modal should fetch saved programs through storage');
assertIncludes("lambdaPost('storage', { id }, '/delete-coeff-program')", 'coeff-program modal should delete saved programs through storage');
assertIncludes("'/save-coeff-program');", 'coeff-program modal should save programs through storage');
assertIncludes("id=\"coeff-program-modal-overlay\"", 'shared coeff-program modal markup missing');
assertIncludes(".coeff-program-modal-source {", 'Coeff Program modal should render source-text programs as fixed-width source, not stale chip cards');
assertIncludes("function _coeffProgramSourceStatementCount(sourceText) {", 'Coeff Program text mode should count source statements for modal/save display');
assertIncludes("function _coeffProgramSourceDisplay(sourceText, separator = ',') {", 'Coeff Program text mode should have a source-aware log/display formatter');
assertIncludes("if (name === 'push' && params[0] === 'cf') return 'cf';", 'Coeff Program chain-to-source renderer should render push(cf) as valid bare cf syntax');
assertIncludes("if (name === 'pop') return 'drop';", 'Coeff Program chain-to-source renderer should render drop as drop, not ambiguous standalone pop');
assertIncludes("if (name === 'poke_poly' && params.length >= 2) return `poly[${params[0]}] = ${params[1]}`;", 'Coeff Program chain-to-source renderer should render poke_poly as valid indexed assignment');
assertIncludes("if (name === 'legacy') {\n                const [legacyName, src, tgt, ...rest] = params;", 'Coeff Program chain-to-source renderer should unwrap old legacy-form saved chips');
assertIncludes("const callName = _coeffProgramSourceAliasNames[legacyName] || legacyName || 'rev';", 'Coeff Program legacy-form source rendering should map shadowed registry names (exp/pow/power) to parser aliases');
assertIncludes("const _coeffProgramSourceAliasNames = _coeffRegistryVocab ? _coeffRegistryVocab.sourceAliasByName : {};", 'Coeff Program synthesizer should derive the parser-alias map from the generated vocab');
assertIncludes("if (catalogName === 'linear') return [catalogName, ...values];", 'Coeff Program serializer must emit all linear args; the backend affine chip rejects trimmed forms');
assertIncludes("if (name === 'argsort' && params.length >= 3) {", 'Coeff Program chain-to-source renderer should synthesize argsort without the target selector');
assertIncludes("let _coeffProgramSourceAutoSynthed = false;", 'Coeff Program text tab should track auto-synthesized vs user-authored source');
assertIncludes("(!_getCoeffProgramSourceText().trim() || _coeffProgramSourceAutoSynthed)) {", 'Coeff Program tab switch should re-synthesize auto-generated text from the latest chips');
assertIncludes("&& String(raw.source_text || '').trim() !== '';", 'Coeff Program payload parsing should not let an empty source_text discard a non-empty chain');
assertIncludes("function _scheduleCoeffProgramSourceValidation() {", 'Coeff Program text editor should debounce advisory backend validation');
assertIncludes("'/compile-coeff-program-source');", 'Coeff Program text editor should validate via the compile-coeff-program-source route');
assertIncludes("return tgt === 'poly' ? `poly = ${callName}(${args})` : `${callName}(${args})`;", 'Coeff Program legacy-form source rendering should produce direct source syntax, not legacy(...)');
assertIncludes("if (sourceText.trim()) {\n            return _coeffProgramMetaHtml(program, options)", 'Coeff Program modal should prefer source_text display when a text program is active or saved');
assertIncludes("Text source changed. It will be compiled by the backend on save/preview/compute.", 'Coeff Program text editor should tell users save uses source text');
assertIncludes("function _chipMoveControlsHtml(which, idx) {", 'transform chip renderer should centralize move controls');
assertIncludes("chip.insertAdjacentHTML('afterbegin', _chipMoveControlsHtml(which, i));", 'param/transform chips should get move arrows');
assertIncludes("chip-input chip-input-target", 'target parameters should render as dropdown inputs');
assertIncludes("function _solveScorePaletteCompatibility(compiled, interpretation) {", 'palette generation should share the color-mode compatibility helper');
assertIncludes("const colorInterpretation = _selectedPaletteColorInterpretation();", 'Palette tab Generate should validate against the selected palette interpretation before dispatch');
assertIncludes("has_explicit_outputs: p.solveScoreHasExplicitOutputs,", 'Render Palette Generate should validate the render solve-score output mode before dispatch');
const renderCommonSection = src.split("function _renderCommonParams(options = {}) {")[1]?.split("function _renderColorMtEligible()")[0] || '';
assertSectionNotIncludes(renderCommonSection, "gamma:", '_renderCommonParams should not send dead gamma');
assertSectionNotIncludes(renderCommonSection, "constantColor:", '_renderCommonParams should not carry dead constant-color state');
assertIncludes("const explicitBounds = _viewMode === 'explicit'", '_renderCommonParams should validate exact viewport bounds when explicit mode is active');
assertIncludes("minRe: explicitBounds.minRe,", '_renderCommonParams should capture minRe');
assertIncludes("maxRe: explicitBounds.maxRe,", '_renderCommonParams should capture maxRe');
assertIncludes("minIm: explicitBounds.minIm,", '_renderCommonParams should capture minIm');
assertIncludes("maxIm: explicitBounds.maxIm,", '_renderCommonParams should capture maxIm');
assertIncludes("async function _launchNonColorRenderOrchestrator(mode, paramsPatch = null)", 'non-color render launcher missing');
assertNotIncludes("async function _launchClassicRenderOrchestrator", 'classic render launcher should be removed');
assertNotIncludes("async function runClassicRasterPipelineMT", 'classic MT launcher should be removed');
assertNotIncludes("function _initGenerateFromPalettePopup", 'GenerateFromPalette popup init should be removed');
assertNotIncludes("function _renderGenerateFromPalettePopup", 'GenerateFromPalette popup renderer should be removed');
assertNotIncludes("function openGenerateFromPalettePopup", 'GenerateFromPalette opener should be removed');
assertIncludes("if (mode === 'color') {\n        return _launchFusedRenderOrchestrator(paramsPatch);\n    }\n    return _launchNonColorRenderOrchestrator(mode, paramsPatch);", '_launchRenderOrchestrator should route color to fused only');
assertNotIncludes("function openRenderGeneratePopup()", 'legacy color Generate popup opener should be removed');
assertNotIncludes("function _initRenderGeneratePopup()", 'legacy color Generate popup init should be removed');
assertNotIncludes("let _renderGeneratePopupState =", 'legacy color Generate popup state should be removed');
assertIncludes("await _launchFusedRenderOrchestrator({", 'runRasterPipeline should dispatch through fused launcher');
assertIncludes("raster_workers: 10,", 'runRasterPipeline should default fused raster workers to 10');
assertIncludes("raster_section_mode: 'logical_sections_auto',", 'runRasterPipeline should default fused logical sections');
assertIncludes("async function runRasterPipelineMT(threadConfig = null) {\n    return runFusedRasterPipelineMT(threadConfig);\n}", 'runRasterPipelineMT should be fused-only wrapper');
const nonColorSection = src.split("async function _launchNonColorRenderOrchestrator(mode, paramsPatch = null) {")[1]?.split("async function _launchFusedRenderOrchestrator")[0] || '';
const fusedSection = src.split("async function _launchFusedRenderOrchestrator(paramsPatch = null) {")[1]?.split("async function runRasterPipeline(config = null)")[0] || '';
assertSectionNotIncludes(nonColorSection, "color_mode: 'solve_score'", 'non-color launcher should not send color mode');
assertSectionNotIncludes(nonColorSection, "color_pipeline: 'fused'", 'non-color launcher should not send color pipeline');
assertSectionNotIncludes(nonColorSection, "solve_metric:", 'non-color launcher should not send solve-score metric');
assertSectionNotIncludes(nonColorSection, "solve_score_chain:", 'non-color launcher should not send solve-score chain');
assertSectionNotIncludes(nonColorSection, "palette:", 'non-color launcher should not send palette');
assertSectionNotIncludes(nonColorSection, "save_associated_palette: false", 'non-color launcher should not send color-only associated palette flag');
assertSectionNotIncludes(fusedSection, "solve_metric:", 'fused color launcher should not send legacy solve-score metric');
assertSectionNotIncludes(fusedSection, "solve_score_quantile:", 'fused color launcher should not send legacy solve-score quantile');
assertSectionNotIncludes(fusedSection, "solve_score_omega:", 'fused color launcher should not send legacy solve-score omega');
assertSectionNotIncludes(fusedSection, "solve_score_omega_enabled:", 'fused color launcher should not send legacy solve-score omega_enabled');
assertNotIncludes("color_pipeline: 'fused'", 'frontend should not emit a color_pipeline field anywhere');
assertIncludes("setMatch('none');", 'artifact restore should force match mode back to none');
assertIncludes("Number.isFinite(entry.min_re) &&", 'render artifact populate should prefer canonical bounds');
assertIncludes("selectViewMode('explicit');", 'render artifact populate should restore explicit viewport mode from bounds');
assertNotIncludes("art.color_mode === 'saved_palette'", 'saved_palette compatibility should be removed from color UI');
assertIncludes("const phasePerfRows = subResults.length ? subResults : [rd];", 'render observer should use top-level phase rows when no subtasks exist');
assertIncludes("function _aggregateColorToBilevelPerf(results) {", 'Color2Bilevel perf aggregation missing');
assertIncludes("phase === 'bilevel_from_raw_render'", 'Color2Bilevel render phase summary/progress handling missing');
assertIncludes("phase: 'from_raw_color',", 'Color2Bilevel dispatch payload must include from_raw_color phase');
assertIncludes("sparse occupancy fragments", 'BiLevel UI should describe sparse fragment pipeline');
assertIncludes("pipeline=logical sections -> sparse fragments -> single finalize", 'BiLevel popup summary should describe sparse fragment finalize path');
assertNotIncludes("let _renderGeneratePopupState = { open: false, histInputMode:", 'Generate popup should not retain dead histogram-input state');
assertNotIncludes("_renderMtPopupState.rasterInputMode", 'Render-MT popup should not retain dead raster input mode state');
assertNotIncludes("_renderMtPopupState.pixelBinFragmentMode", 'Render-MT popup should not retain dead fragment mode state');
assertNotIncludes("_renderMtPopupState.rasterBinGroupSize", 'Render-MT popup should not retain dead bin group size state');
assertNotIncludes("_renderMtPopupState.colorPipeline", 'Render-MT popup should not retain dead colorPipeline state');
assertNotIncludes("_renderMtPopupState.rasterSectionCountAuto", 'Render-MT popup should not retain dead auto section-count state');
assertNotIncludes("id=\"render-solve-score-omega-val\"", 'render tab should not ship hidden omega display spans');
assertNotIncludes("id=\"render-solve-score-quantile-val\"", 'render tab should not ship hidden quantile display spans');
assertIncludes("_bilevelPopupState.sectionCount = '';", 'BiLevel popup should clear stale manual section count in auto mode');
assertIncludes("const sectionCount = sectionMode === 'logical_sections'\n        ? _clampRenderMtSectionCount(_bilevelPopupState.sectionCount)\n        : '';", 'BiLevel popup summary should ignore manual count in auto mode');
assertIncludes("const popupRunBtn = document.getElementById('bilevel-popup-run');", 'BiLevel popup run button should be controlled during dispatch');
assertIncludes("if (popupRunBtn) { popupRunBtn.disabled = true; popupRunBtn.textContent = 'Executing...'; }", 'BiLevel popup run button should disable during dispatch');
assertIncludes("if (popupRunBtn) { popupRunBtn.disabled = false; popupRunBtn.textContent = popupRunOrig; }", 'BiLevel popup run button should restore after dispatch');
assertIncludes("function _aggregateBilevelRasterPerf(results) {", 'BiLevel raster perf aggregation missing');
assertIncludes("function _aggregateBilevelFinalizePerf(results) {", 'BiLevel finalize perf aggregation missing');
assertIncludes("if (phase === 'bilevel_raster') {", 'BiLevel raster phase summary handling missing');
assertIncludes("if (phase === 'bilevel_finalize') {", 'BiLevel finalize phase summary/progress handling missing');
assertIncludes("function _renderRunErrorLabel(run) {", 'render error labeling helper missing');
assertIncludes("const errorLabel = _renderRunErrorLabel(run);", 'render observer should use mode-specific error labels');
assertIncludes("statusEl.textContent = errorLabel + ' error: ' + msg;", 'render observer should show mode-specific error text');
assertIncludes("log(errorLabel + ' failed: ' + msg, 'err', 'render-log');", 'render observer should log mode-specific failure text');
assertIncludes("if (rd.artifact_id) parts.push(`artifact=${rd.artifact_id}`);", 'error context should include artifact id');
assertIncludes("if (rd.source_artifact_id) parts.push(`source_artifact=${rd.source_artifact_id}`);", 'error context should include source artifact id');
assertIncludes("if (rd.threshold !== undefined && rd.threshold !== null && rd.threshold !== '') parts.push(`threshold=${rd.threshold}`);", 'error context should include threshold');
assertIncludes("if (rd.threshold_raw !== undefined && rd.threshold_raw !== null && rd.threshold_raw !== '') parts.push(`threshold_raw=${rd.threshold_raw}`);", 'error context should include raw threshold');
assertNotIncludes("setValue('autolevel-auto-gamma'", 'autolevel popup should not write a fake auto-gamma selector');
assertNotIncludes("auto_gamma: readSel('autolevel-auto-gamma', 'median')", 'autolevel popup should not read a fake auto-gamma selector');
assertIncludes("const effectiveParamGenThreads = fused ? fusedThreads : paramGenThreads;", 'fused compute should normalize hidden param-gen threads to shared hires threads');
assertIncludes("const effectiveCoeffgenThreads = fused ? fusedThreads : coeffgenThreads;", 'fused compute should normalize hidden coeffgen threads to shared hires threads');
assertIncludes("if (fused && fusedThreads != null) orchPayload.params.fused_threads = fusedThreads;", 'compute launch should only forward fused_threads for fused runs');
assertNotIncludes("if (fusedThreads != null) orchPayload.params.fused_threads = fusedThreads;", 'classic compute should not leak fused_threads');
assertNotIncludes("id=\"btn-calculate\" onclick=\"runCalculate()\"", 'single-thread AE calculate button should be removed');
assertNotIncludes("<option value=\"aberth\">AE</option>", 'compute preview should not offer removed single-thread AE solver');
assertNotIncludes("async function runCalculate() {", 'single-thread AE calculate handler should be removed');
assertNotIncludes("id=\"compute-mt-tab-classic\"", 'AE-MT compute popup should not expose a classic tab');
assertNotIncludes("id=\"compute-mt-tab-fused\"", 'AE-MT compute popup should not expose a redundant fused tab');
assertNotIncludes("id=\"compute-mt-classic-panel\"", 'AE-MT compute popup should not expose classic controls');
assertNotIncludes("id=\"compute-mt-classic-chunks\"", 'AE-MT compute popup should not expose classic chunk input');
assertNotIncludes("id=\"compute-mt-param-gen-threads\"", 'AE-MT compute popup should not expose classic param-gen thread input');
assertNotIncludes("id=\"compute-mt-coeffgen-threads\"", 'AE-MT compute popup should not expose classic coeffgen thread input');
assertIncludes("runCalculateWithSolver(solverMode, { nChunks, fused: true, fusedThreads, loresParamGenThreads, loresCoeffgenThreads });", 'AE-MT compute popup should launch fused explicitly');
assertIncludes("const coeffProgramSourceText = _effectiveCoeffProgramSourceTextForCompute() || '';\n    const cfpv = _cfpv.length > 0 ? [..._cfpv] : [];\n    const fusedThreads = _clampRenderMtThreads(_computeMtPopupState.fusedThreads || 4);", 'AE-MT degree probe should define coeffProgramSourceText before using it in the probe signature');
assertIncludes("function _normalizeComputeSolverMode(solver) {", 'compute should normalize legacy solver metadata to supported solvers');
assertIncludes("function _computeLoresPhaseTrackers(runId, solverMode) {", 'compute log should define deterministic lores phase trackers');
assertIncludes("phase: 'lores_param_gen',", 'compute log should track lores param-gen completion');
assertIncludes("phase: 'lores_coeffgen',", 'compute log should track lores coeffgen completion');
assertIncludes("phase: 'lores_solve',", 'compute log should track lores solve completion');
assertIncludes("await _logComputeKnownLoresCompletions(jobId, loresPhaseTrackers, loggedPhaseCompletions);", 'compute polling should log known lores completions even if top-level phases are missed');
assertIncludes(">Refresh...</button>", 'results tab refresh button should disclose that it opens the tuning popup');
assertIncludes("Preview Q <span id=\"res-q-val\">0</span>%", 'results preview quantile slider should be labeled as preview-only');
assertIncludes("Preview SHM <span id=\"res-s-val\">5</span>%", 'results preview shim slider should be labeled as preview-only');
assertIncludes("id=\"btn-render-result\" onclick=\"goRenderResult()\"", 'results Go Render button should use explicit go-render action');
assertIncludes(">Go Render</button>", 'results tab should label the tab-switch action as Go Render');
assertNotIncludes("id=\"palette-solve-score-quantile\"", 'palette tab should not ship hidden legacy quantile input');
assertNotIncludes("id=\"palette-solve-score\"", 'palette tab should not ship hidden legacy metric input');
assertNotIncludes("id=\"palette-solve-score-omega\"", 'palette tab should not ship hidden legacy omega input');
assertNotIncludes("id=\"palette-solve-score-omega-phase\"", 'palette tab should not ship hidden legacy omega phase input');
assertNotIncludes("id=\"palette-solve-score-omega-enabled\"", 'palette tab should not ship hidden legacy omega-enabled input');
assertNotIncludes("id=\"palette-solve-score-quantile-val\"", 'palette tab should not ship hidden legacy quantile display span');
assertNotIncludes("id=\"palette-solve-score-omega-val\"", 'palette tab should not ship hidden legacy omega display span');
assertIncludes("id=\"render-solve-score-program-manage\" onclick=\"openSolveScoreProgramModal('render')\"", 'render tab should expose Solve Scores modal launcher');
assertIncludes("id=\"render-solve-score-quantile\"", 'render solve-score quantile control should survive text-only relocation');
assertIncludes("id=\"render-score-normalization\"", 'render tab should expose score normalization checkbox');
assertIncludes("id=\"btn-solve-histogram\"", 'Solve histogram button should survive text-only relocation');
assertIncludes("solveScoreNormalize: !!document.getElementById('render-score-normalization')?.checked,", '_renderCommonParams should read score normalization checkbox');
assertIncludes("solve_score_normalize: !!p.solveScoreNormalize,", 'fused render payload should forward score normalization flag');
assertIncludes("name=\"render-color-interpretation\" value=\"scalar_lut\"", 'render tab should expose Scalar LUT color interpretation');
assertIncludes("name=\"render-color-interpretation\" value=\"rgb\"", 'render tab should expose RGB color interpretation');
assertIncludes("name=\"render-color-interpretation\" value=\"hsv\"", 'render tab should expose HSV color interpretation');
assertIncludes("name=\"render-color-interpretation\" value=\"rgb_lut\"", 'render tab should expose RGB LUT color interpretation');
assertIncludes("name=\"render-color-interpretation\" value=\"hsv_lut\"", 'render tab should expose HSV LUT color interpretation');
assertIncludes("id=\"render-color-lut-palette-row\"", 'render tab should expose shared LUT palette row');
assertIncludes("three emitted bytes sample R/G/B from the selected palette", 'RGB LUT row should explain palette-component lookup');
assertIncludes("three emitted bytes sample H/S/V from the selected palette in HSV space", 'HSV LUT row should explain HSV palette-component lookup');
assertIncludes("color_interpretation: p.colorInterpretation,", 'render payload should forward selected color interpretation');
assertIncludes("const backgroundColor = _readRenderBackgroundColor({ requireValid: true });", '_renderCommonParams should validate/read background color');
assertIncludes("background_color: p.backgroundColor,", 'render payloads should forward selected background color');
assertIncludes("function _artifactColorInterpretation(art) {", 'render populate should define artifact color interpretation resolver');
assertIncludes("_setRenderColorInterpretation(_artifactColorInterpretation(art));", 'render populate should restore selected artifact color interpretation');
assertIncludes("if (entry.background_color) _setRenderBackgroundColor(entry.background_color);", 'render populate should restore saved background color');
assertIncludes("mode:${_colorInterpretationLabel(colorInterpretation)}", 'render artifact summaries should disclose color interpretation');
assertIncludes("id=\"render-ss-source-text\"", 'render solve-score editor should expose source textarea');
assertIncludes("id=\"render-ss-cheatsheet\"", 'render solve-score editor should expose source insert cheatsheet');
assertIncludes("id=\"palette-ss-cheatsheet\"", 'palette solve-score editor should expose source insert cheatsheet');
assertIncludes(".program-source-grid,", 'generic source editor grid should style Param/Coeff source cheatsheets');
assertIncludes("_renderParamCoeffProgramCheatsheets();", 'boot should render Param/Coeff source cheatsheets');
assertNotIncludes("id=\"ss-insert-before-btn\"", 'solve-score editor should not expose chip insert-before button');
assertNotIncludes("id=\"ss-insert-after-btn\"", 'solve-score editor should not expose chip insert-after button');
assertNotIncludes("id=\"ss-chips\"", 'render solve-score editor should not expose editable chip strip');
assertNotIncludes("id=\"palette-ss-chips\"", 'palette solve-score editor should not expose editable chip strip');
assertNotIncludes("id=\"ss-direct-rgb-preset\"", 'solve-score editor should not expose Direct RGB preset; saved programs cover this');
assertIncludes(".solve-score-modal-popup {\n    width: min(1120px, calc(100vw - 24px));\n    height: min(92dvh, 820px);", 'Solve Scores modal should reserve a stable viewport-safe shell height');
assertIncludes("grid-template-rows: auto auto minmax(0, 1fr) auto auto;", 'Saved-program modals should keep header/status/actions visible and body scroll-contained');
assertIncludes("grid-template-columns: minmax(260px, 360px) minmax(0, 1fr);", 'Saved-program modals should keep the saved-program list visible instead of letting previews consume the shell');
assertIncludes(".solve-score-modal-actions {\n    display: flex;\n    justify-content: space-between;\n    gap: 8px;\n    padding: 10px 14px 14px;", 'Saved-program modal action rows should remain inside the visible shell');
assertIncludes("grid-template-rows: auto minmax(0, 1fr) minmax(0, 1fr);", 'Solve Scores modal detail panes should use stable allocated grid rows');
assertIncludes(".solve-score-modal-display {\n    flex: 1 1 auto;", 'Saved-program modal displays should own their scrolling area');
assertIncludes("overflow-x: hidden;", 'Saved-program modal displays should not show bottom scrollbars for wide program chips');
assertIncludes(".solve-score-modal-chip-strip {\n    display: grid;\n    grid-template-columns: minmax(0, 1fr);", 'Saved-program modal chip previews should be fixed-width grid rows');
assertIncludes(".solve-score-modal-chip-strip .score-chip-readonly {\n    width: 100%;", 'Saved-program modal readonly chips should wrap inside the preview width');
assertIncludes(".solve-score-modal-chip-strip .chip-input-readonly {\n    display: inline-block;", 'Saved-program modal readonly input values should wrap instead of widening the modal');
assertIncludes("function _renderSolveScoreProgramCardHtml(program, options = {}) {", 'Solve Scores modal should render selected programs through a card renderer');
assertIncludes("readonly: true, solveScore: true, chain: normalized", 'Solve Scores modal chip renderer should reuse solve-score chips in read-only mode');
assertIncludes("currentEl.innerHTML = _renderSolveScoreProgramCardHtml(currentProgram", 'Solve Scores modal current program should render chip markup, not debug text');
assertIncludes("selectedEl.innerHTML = _renderSolveScoreProgramCardHtml(_solveScoreModalState.selectedProgram", 'Solve Scores modal selected program should render chip markup, not debug text');
assertIncludes("score-chip-readonly", 'Solve Scores modal should expose read-only chip styling');
assertNotIncludes("solve-score-modal-display popup-stable-block", 'Solve Scores modal detail panes should not inherit generic min-height popup block sizing');
assertNotIncludes("Program spec:", 'Solve Scores modal should not show internal program_spec in the main display');
assertNotIncludes("JSON.stringify(program.chain || [], null, 2)", 'Solve Scores modal should not show saved program JSON in the main display');
assertIncludes("score normalization: lo=${fmt(s.score_output_clip_lo)}  hi=${fmt(s.score_output_clip_hi)}", 'histogram output should report score normalization range');
assertIncludes("if (s.raw_hist_space === 'score_output_normalized') rawLabel = 'score-output normalized program output';", 'histogram raw bins should label score-output normalized space');
if (!solveVocabSrc.includes("mean_log_mod")) fail('generated solve-score vocab should expose mean_log_mod');
if (!solveVocabSrc.includes("angular_entropy_16")) fail('generated solve-score vocab should expose angular_entropy_16');
if (!solveVocabSrc.includes("sector_max_share_16")) fail('generated solve-score vocab should expose sector_max_share_16');
if (!solveVocabSrc.includes("angular_order_4")) fail('generated solve-score vocab should expose angular_order_4');
assertIncludes("const _solveScoreGenericMetricPublicName = _solveScoreVocab.genericMetricPublicName || 'metric';", 'solve-score editor should derive public generic metric chip name from generated vocab');
assertIncludes("const _solveScoreGenericMetricChipName = _solveScoreVocab.genericMetricChipName || '__metric';", 'solve-score editor should derive generic metric internal name from generated vocab');
assertIncludes("function _requireSolveScoreProgramSourceText(prefix) {", 'solve-score dispatch should have a nonblank source guard');
assertIncludes("const solveScoreProgramSourceText = options.requireSolveScore", 'render common params should derive source text before fused dispatch');
assertIncludes("const scoreSourceText = _requireSolveScoreProgramSourceText('palette');", 'palette artifact dispatch should reject blank solve-score source client-side');
assertIncludes("catalog[_solveScoreGenericMetricChipName] = {", 'solve-score catalog should expose generic metric chip');
assertIncludes("return [_solveScoreGenericMetricPublicName, ...(item.params || [])];", 'generic metric chip should serialize publicly without desugaring in saved programs');
assertIncludes("id=\"render-preview-pix\" value=\"256\"", 'render output should expose default 256px lores preview size input');
assertIncludes("id=\"btn-render-lores-preview\" onclick=\"runRenderLoresPreview()\"", 'render output should expose lores preview button');
assertIncludes("id=\"render-lores-preview-stage\"", 'render output preview should expose a marquee stage wrapper');
assertIncludes("id=\"render-lores-preview-marquee\"", 'render output preview should expose a marquee overlay');
assertIncludes(".render-lores-preview-tabs {\n    width: 100%;", 'render output preview tabs should fill the Output box width');
assertIncludes(".render-lores-preview-box {\n    width: 100%;\n    aspect-ratio: 1 / 1;", 'render output preview plot should fill Output width as a square');
assertNotIncludes("id=\"render-lores-preview-box\" style=\"width:256px;", 'render output preview should not be hardcoded to 256px display width');
assertIncludes("id=\"render-lores-preview-tab-plot\"", 'render output preview should expose plot tab');
assertIncludes("id=\"render-lores-preview-tab-palette\"", 'render output preview should expose palette tab');
assertIncludes("id=\"render-lores-preview-palette-canvas\"", 'render output preview should expose palette canvas');
assertIncludes("id=\"render-lores-preview-tab-e1\"", 'render output preview should expose E1 histogram tab');
assertIncludes("id=\"render-lores-preview-hist-e1\"", 'render output preview should expose E1 histogram canvas');
assertIncludes("id=\"render-preview-source-mode\"", 'render output should expose preview source mode dropdown');
assertIncludes("<option value=\"lores\">use lores</option>", 'render preview source dropdown should support saved lores');
assertIncludes("<option value=\"logical\">logical</option>", 'render preview source dropdown should support logical lores');
assertIncludes("<option value=\"recompute\">recompute</option>", 'render preview source dropdown should support recompute');
assertIncludes("id=\"render-preview-source-size\" placeholder=\"lores_N\"", 'render output should expose preview source size input');
assertIncludes("if (sizeEl) sizeEl.disabled = mode === 'lores';", 'render preview source size should be disabled for saved lores');
assertIncludes("preview_pix: previewPix,", 'render lores preview payload should forward preview_pix');
assertIncludes("preview_source_mode: previewSourceMode,", 'render lores preview payload should forward preview source mode');
assertIncludes("preview_source_size: previewSourceSize,", 'render lores preview payload should forward preview source size');
assertIncludes("lores_N: calcLoresN,", 'render lores preview payload should forward lores_N for physical palette grid');
assertIncludes("previewSourceSize = Math.max(5, Math.min(256, previewSourceSize));", 'render lores preview should clamp preview source size');
assertIncludes("for (const line of (Array.isArray(result.logs) ? result.logs : []))", 'render lores preview should print backend logical lores logs');
assertIncludes("const nCoeffs = Number.isFinite(rawNCoeffs) && rawNCoeffs >= 1 ? rawNCoeffs : degree + 1;", 'render lores preview should default missing n_coeffs to degree+1');
assertIncludes("lores_bin_key: loresKey,", 'render lores preview payload should use the existing lores roots artifact');
assertIncludes("const result = await lambdaPost('render-lores-preview', payload);", 'render lores preview should call the direct preview endpoint');
assertIncludes("const ctx = canvas.getContext('2d');", 'render lores preview should draw the returned image onto the canvas');
assertIncludes("function _initRenderLoresPreviewMarquee(meta) {", 'render output preview marquee initializer missing');
assertIncludes("function _applyRenderLoresPreviewSelectionBounds(meta, rect) {", 'render output preview selection should populate exact viewport bounds');
assertIncludes("_initRenderLoresPreviewMarquee(_renderLoresPreviewMetaFromResult(result, p.rotation));", 'render lores preview should arm marquee after drawing the preview');
assertIncludes("_clearRenderLoresPreviewSelection();", 'Escape should clear output preview marquee selection');
assertIncludes("_setRenderLoresPreviewEmissionHistograms(result.emission_histograms || result.solve_score?.emission_histograms || []);", 'render lores preview should load per-emission histograms');
assertIncludes("await _setRenderLoresPreviewPaletteImage(result);", 'render lores preview should draw returned palette image');
if (!solveVocabSrc.includes('"none"')) fail('generated solve-score output vocab should expose none mode');
if (!solveVocabSrc.includes('"flush"')) fail('generated solve-score stack vocab should expose flush');
if (!solveVocabSrc.includes('"quantilePercentRange"')) fail('generated solve-score vocab should expose quantile percent range');
assertIncludes("omega_cosine requires one finite numeric omega", 'omega_cosine frontend validation should not cap frequency at 10');
assertNotIncludes("omega_cosine requires one numeric omega in [1, 10]", 'omega_cosine frontend validation should not retain old [1,10] range');
assertIncludes("id=\"palette-solve-score-program-manage\" onclick=\"openSolveScoreProgramModal('palette')\"", 'palette tab should expose Solve Scores modal launcher');
assertIncludes("id=\"solve-score-modal-overlay\"", 'shared solve-score modal markup missing');
assertIncludes("id=\"solve-score-modal-name\"", 'solve-score modal name input missing');
assertIncludes("id=\"solve-score-modal-current\"", 'solve-score modal current-program pane missing');
assertIncludes("id=\"solve-score-modal-selected\"", 'solve-score modal selected-program pane missing');
assertIncludes("id=\"solve-score-modal-load\"", 'solve-score modal load button missing');
assertIncludes("id=\"solve-score-modal-save\"", 'solve-score modal save button missing');
assertIncludes("id=\"solve-score-modal-delete\"", 'solve-score modal delete button missing');
assertIncludes("id=\"solve-score-modal-download\"", 'solve-score modal download button missing');
assertIncludes("id=\"solve-score-modal-upload\"", 'solve-score modal upload button missing');
assertIncludes("_solveScoreModalState.nameInput = program.name;", 'modal load should sync the loaded program name into the editable name field');
assertIncludes("id=\"render-preview-stage\"", 'render preview should expose a marquee stage wrapper');
assertIncludes("id=\"render-preview-marquee\"", 'render preview should expose a marquee overlay');
assertIncludes("function _renderPreviewViewportMeta(art) {", 'render preview viewport-meta helper missing');
assertIncludes("function _initRenderPreviewMarquee(art) {", 'render preview marquee initializer missing');
assertIncludes("Preview subview selected from", 'render preview marquee should write an actionable status message');
assertNotIncludes("id=\"resize-crop\"", 'resize popup should not expose a crop control');
assertNotIncludes("id=\"resize-vscale\"", 'resize popup should not expose a vscale control');
assertIncludes("Both engines produce a square pix x pix artifact", 'resize popup should document square pix output');
assertIncludes(">ColorRender-MT</button>", 'color render primary action should be labeled ColorRender-MT');
assertIncludes("id=\"btn-render-pdf-colorspread\" onclick=\"runPdfColorSpreadSelectedRenderArtifact()\"", 'Color tab should expose a direct PDF action for the selected Color artifact');
assertIncludes("PDF artifacts are generated from the selected Color artifact using the PDF button on the Color tab.", 'PDF tab copy should explain PDFs are generated from Color selection');
assertIncludes("async function runPdfColorSpreadForArtifact(art, btn) {", 'PDF ColorSpread dispatch should use a reusable selected-artifact runner');
assertIncludes("async function runPdfColorSpreadSelectedRenderArtifact() {", 'Render PDF action should dispatch from the selected Color artifact without a popup');
assertIncludes("Selected Color</span>", 'Color tab should show the selected artifact id instead of long help copy');
assertIncludes("placeholder=\"No Color artifact selected\"", 'Color selected-artifact field should have an empty-state placeholder');
assertIncludes("function _sourceColorArtifactIdForRenderArtifact(art) {", 'render artifact source-color helper missing');
assertIncludes("function _renderArtifactSolveDisplay(art) {", 'render artifact solve-display helper missing');
assertIncludes("_solveScoreProgramRememberedNames[prefix] = '';", 'populate should clear stale solve-score remembered names');
assertIncludes("_setSolveScoreProgramStatus(prefix, `Populated from ${statusTarget}`, false);", 'populate should overwrite stale solve-score status with the resolved source label');
assertNotIncludes("ColorRender-MT exposes fused clip/raster/finalize controls only", 'Color tab should not show the obsolete long action description');
assertIncludes("Palette summaries show the solve display, palette name, and source Color artifact id.", 'render tab copy should match the new palette summary contract');
assertNotIncludes("[P id] in Color summaries means the artifact has a palette link;", 'render tab should not describe removed [P id] summary tokens');
assertNotIncludes("[C id] means this palette is linked back to a Color artifact.", 'render tab should not describe removed [C id] summary tokens');
assertIncludes("id=\"btn-dz-goto-render\" onclick=\"_dzGotoSelectedRender()\"", 'DeepZoom tab should expose a GotoRender button');
assertIncludes(">PopulateResult</button>", 'DeepZoom tab Populate button should be labeled PopulateResult');
assertIncludes("id=\"deepzoom-viewport-readout\"", 'DeepZoom tab should expose a viewport readout block');
assertIncludes("id=\"deepzoom-viewer\" style=\"width:min(100%, 70vh); aspect-ratio:1 / 1;", 'DeepZoom tab should keep the main viewer square');
assertIncludes("navigatorMaintainSizeRatio: false,", 'DeepZoom viewer should disable navigator aspect lock');
assertIncludes("navigatorWidth: '160px',", 'DeepZoom viewer should set square navigator width');
assertIncludes("navigatorHeight: '160px',", 'DeepZoom viewer should set square navigator height');
assertIncludes("<th style=\"text-align:left;padding:4px\">Job</th><th style=\"text-align:left;padding:4px\">Render</th>", 'DeepZoom inventory should show a Render column after Job');
assertIncludes("function _parseRenderSourceRef(sourceKey) {", 'DeepZoom render-source parser missing');
assertIncludes("function _scheduleDeepZoomViewportReadout() {", 'DeepZoom viewport readout scheduler missing');
assertIncludes("function _renderDeepZoomViewportReadout() {", 'DeepZoom viewport readout renderer missing');
assertIncludes("function _dzGotoSelectedRender() {", 'DeepZoom GoRender handler missing');
assertNotIncludes("id=\"render-solve-score-program-select\"", 'render tab should not keep built-in solve-score preset select');
assertNotIncludes("id=\"palette-solve-score-program-select\"", 'palette tab should not keep built-in solve-score preset select');
assertNotIncludes("id=\"render-solve-score-program-load\"", 'render tab should not keep load-preset button');
assertNotIncludes("id=\"palette-solve-score-program-load\"", 'palette tab should not keep load-preset button');
assertNotIncludes("id=\"render-solve-score-program-load-file\"", 'render tab should not keep load-json button');
assertNotIncludes("id=\"palette-solve-score-program-load-file\"", 'palette tab should not keep load-json button');
assertNotIncludes("id=\"render-solve-score-program-save\"", 'render tab should not keep save-json button');
assertNotIncludes("id=\"palette-solve-score-program-save\"", 'palette tab should not keep save-json button');
assertNotIncludes("id=\"render-solve-score-program-file\"", 'render tab should not keep per-tab solve-score upload input');
assertNotIncludes("id=\"palette-solve-score-program-file\"", 'palette tab should not keep per-tab solve-score upload input');
assertIncludes("id=\"render-ss-text-panel\" class=\"coeff-program-editor-panel active\"", 'render solve-score editor should keep the Text panel active');
assertIncludes("id=\"palette-ss-text-panel\" class=\"coeff-program-editor-panel active\"", 'palette solve-score editor should keep the Text panel active');
assertNotIncludes("id=\"render-ss-tab-chips\"", 'render solve-score editor should not expose a Chips tab');
assertNotIncludes("id=\"palette-ss-tab-chips\"", 'palette solve-score editor should not expose a Chips tab');
assertIncludes("id=\"render-rt-tab-text\" class=\"compute-preview-tab-btn\" onclick=\"_setRootProgramEditorMode('render','text')\"", 'render root editor should expose a Text tab');
assertIncludes("id=\"palette-rt-tab-text\" class=\"compute-preview-tab-btn\" onclick=\"_setRootProgramEditorMode('palette','text')\"", 'palette root editor should expose a Text tab');
assertIncludes("lambdaPost('storage', { source_text: sourceText, strict: true }, '/compile-solve-score-program-source')", 'solve-score source editor should compile through the backend route');
assertIncludes("lambdaPost('storage', { source_text: sourceText, strict: true }, '/compile-root-program-source')", 'root source editor should compile through the backend route');
assertIncludes("solve_score_program_source_text: p.solveScoreProgramSourceText,", 'render/preview payloads should forward solve-score source text');
assertNotIncludes("solve_score_chain: p.solveScoreChain,", 'render/preview browser payloads should not send solve-score chip chains');
assertIncludes("root_program_source_text: p.rootProgramSourceText || undefined,", 'render/preview payloads should forward root source text');
assertIncludes("solve_score_program_source_text: scoreSourceText,", 'palette payload should forward guarded solve-score source text');
assertNotIncludes("solve_score_chain: score.chain,", 'palette browser payload should not send solve-score chip chains');
assertIncludes("_restoreSolveScoreSourceFromArtifact('render', art);", 'render Populate should restore solve-score source text when metadata has it');
assertIncludes("_restoreRootSourceFromArtifact('palette', pal);", 'palette Populate should restore root source text when metadata has it');
assertIncludes("lambdaPost('storage', {}, '/list-solve-score-programs')", 'solve-score modal should list saved programs through storage');
assertIncludes("lambdaPost('storage', { id }, '/fetch-solve-score-program')", 'solve-score modal should fetch saved programs through storage');
assertIncludes("lambdaPost('storage', { chain }, '/solve-score-chain-to-source')", 'solve-score modal should reconstruct legacy chain programs through backend source route');
assertIncludes("recommended_interpretation: payload.recommended_interpretation || undefined,", 'solve-score modal should save recommended color interpretation through storage');
assertIncludes("lambdaPost('storage', { id }, '/delete-solve-score-program')", 'solve-score modal should delete saved programs through storage');
assertNotIncludes("fetch('solve-score-programs/index.json'", 'frontend should not depend on repo-backed solve-score preset catalog');
assertNotIncludes("function loadSolveScoreProgramPreset(", 'old solve-score preset loader should be removed');
assertNotIncludes("function saveSolveScoreProgram(", 'old solve-score download helper should be removed');

console.log('Frontend fused render source checks: OK');
NODE

node - "$HTML" <<'NODE'
const fs = require('fs');
const vm = require('vm');
const path = require('path');

const root = path.dirname(process.argv[2]);
function fail(message) {
  console.error('FATAL: ' + message);
  process.exit(1);
}
function makeEl(id) {
  return {
    id,
    textContent: '',
    innerHTML: '',
    disabled: false,
    value: '',
    className: '',
    dataset: {},
    style: {},
    children: [],
    classList: { toggle() {}, add() {}, remove() {} },
    focus() {},
    scrollIntoView() {},
    setAttribute() {},
    prepend(child) { this.children.unshift(child); },
    appendChild(child) { this.children.push(child); },
    removeChild(child) { this.children = this.children.filter(x => x !== child); },
  };
}
const elements = new Map();
function el(id) {
  if (!elements.has(id)) elements.set(id, makeEl(id));
  return elements.get(id);
}
const timers = new Map();
let nextTimerId = 1;
const ctx = {
  console,
  Date,
  Map,
  Set,
  Promise,
  Number,
  String,
  Array,
  Object,
  Math,
  parseFloat,
  parseInt,
  setTimeout: (fn, ms) => {
    const id = nextTimerId++;
    timers.set(id, { fn, ms });
    return id;
  },
  clearTimeout: (id) => {
    timers.delete(id);
  },
  __runNextTimer: () => {
    const next = timers.entries().next();
    if (next.done) return null;
    const [id, timer] = next.value;
    timers.delete(id);
    timer.fn();
    return timer.ms;
  },
  __timerCount: () => timers.size,
  performance: { now: () => 1000 },
  document: {
    getElementById: el,
    createElement: () => makeEl('created'),
    querySelectorAll: () => [],
  },
  window: null,
};
ctx.window = ctx;
vm.createContext(ctx);
vm.runInContext(fs.readFileSync(path.join(root, 'js/01-core-compute.js'), 'utf8'), ctx, {
  filename: 'js/01-core-compute.js',
});

const runtimePromise = vm.runInContext(`
(async () => {
  function assertLocal(cond, message) {
    if (!cond) throw new Error(message);
  }
  _setPreviewPlaceholder = (el, message) => {
    el.textContent = message;
    el.innerHTML = message;
  };
  _setPreviewImage = (el, url) => {
    el.dataset.previewUrl = url;
    el.innerHTML = 'image:' + url;
  };
  _formatCfpvForDisplay = () => '';
  _solverShortLabel = (solver) => solver || '-';

  let countCalls = [];
  let resolveCount = null;
  lambdaPost = (service, payload, route) => {
    countCalls.push({ service, payload, route });
    return new Promise(resolve => { resolveCount = resolve; });
  };
  _selectedJobId = 'compute_count';
  _resultsCache = [{ job_id: 'compute_count' }];
  document.getElementById('res-color-renders').textContent = '-';
  _scheduleSelectedColorRenderCount('compute_count');
  assertLocal(document.getElementById('res-color-renders').textContent === '...', 'uncached color render count should show a pending label');
  assertLocal(countCalls.length === 0, 'color render count should not call storage before the debounce fires');
  assertLocal(__timerCount() === 1, 'color render count should schedule one debounce timer');
  assertLocal(__runNextTimer() === RESULTS_COLOR_RENDER_COUNT_DELAY_MS, 'color render count should use the configured debounce delay');
  await Promise.resolve();
  assertLocal(countCalls.length === 1, 'color render count should call storage after debounce');
  assertLocal(countCalls[0].service === 'storage', 'color render count should use the storage lambda');
  assertLocal(countCalls[0].route === '/render-count', 'color render count should use the render-count route');
  assertLocal(countCalls[0].payload.job_id === 'compute_count', 'color render count should use the selected job id');
  const countPromise = _resultColorRenderCountInFlight.get('compute_count');
  assertLocal(countPromise, 'color render count should be tracked in the in-flight map');
  resolveCount({ color_render_count: 3, color_artifact_count: 2, legacy_color_artifact_count: 1 });
  await countPromise;
  assertLocal(!_resultColorRenderCountInFlight.has('compute_count'), 'color render count should clear the in-flight map after success');
  assertLocal(document.getElementById('res-color-renders').textContent === '3', 'color render count should update the selected sidebar after success');
  assertLocal(_resultsCache[0].color_render_count === 3, 'color render count should cache the total on the result row');
  assertLocal(_resultsCache[0].color_artifact_count === 2, 'color render count should cache immutable artifact count on the result row');
  assertLocal(_resultsCache[0].legacy_color_artifact_count === 1, 'color render count should cache legacy artifact count on the result row');

  _scheduleSelectedColorRenderCount('compute_count');
  assertLocal(countCalls.length === 1, 'cached color render count should not refetch');
  assertLocal(__timerCount() === 0, 'cached color render count should not schedule a timer');

  _selectedJobId = 'compute_cancel';
  _resultsCache = [{ job_id: 'compute_cancel' }];
  _scheduleSelectedColorRenderCount('compute_cancel');
  assertLocal(__timerCount() === 1, 'uncached color render count should schedule a cancellable timer');
  _selectedJobId = 'compute_other';
  _cancelPendingColorRenderCount();
  assertLocal(__timerCount() === 0, 'selection changes should cancel the pending color render count timer');

  let resolveStaleCount = null;
  lambdaPost = (service, payload, route) => new Promise(resolve => { resolveStaleCount = resolve; });
  _selectedJobId = 'compute_stale';
  _resultsCache = [{ job_id: 'compute_stale' }];
  document.getElementById('res-color-renders').textContent = '-';
  _scheduleSelectedColorRenderCount('compute_stale');
  assertLocal(__runNextTimer() === RESULTS_COLOR_RENDER_COUNT_DELAY_MS, 'stale color render count should use the debounce delay');
  await Promise.resolve();
  const staleCountPromise = _resultColorRenderCountInFlight.get('compute_stale');
  assertLocal(staleCountPromise, 'stale color render count should be tracked in flight');
  _selectedJobId = 'compute_other';
  document.getElementById('res-color-renders').textContent = 'other';
  resolveStaleCount({ color_render_count: 9, color_artifact_count: 9, legacy_color_artifact_count: 0 });
  await staleCountPromise;
  assertLocal(document.getElementById('res-color-renders').textContent === 'other', 'stale color render count response should not update the moved sidebar');

  let calls = [];
  let resolvePreview = null;
  lambdaPost = (service, payload, route) => {
    calls.push({ service, payload, route });
    return new Promise(resolve => { resolvePreview = resolve; });
  };

  _selectedJobId = 'compute_a';
  _resultsCache = [{ job_id: 'compute_a' }];
  const row = _resultsCache[0];
  const detail = {
    has_preview: false,
    file_count: 7,
    param_transforms_display: [],
    coeff_transforms: [],
    pipeline: {},
    calc: {},
  };
  row._detail = detail;
  _applyDetail(row, detail, document.getElementById('results-preview'), document.getElementById('results-info'), 'compute_a');
  assertLocal(document.getElementById('results-info').textContent === '7 files', 'detail file count should render before the deferred lazy preview status');
  await Promise.resolve();
  assertLocal(calls.length === 0, 'missing preview should not call the preview lambda before the debounce fires');
  assertLocal(__timerCount() === 1, 'missing preview should schedule exactly one lazy debounce timer');
  assertLocal(__runNextTimer() === RESULTS_LAZY_PREVIEW_DELAY_MS, 'missing preview should use the configured debounce delay');
  await Promise.resolve();
  assertLocal(calls.length === 1, 'missing preview should start one lazy preview request');
  assertLocal(calls[0].service === 'preview', 'lazy preview should call the preview lambda');
  assertLocal(calls[0].payload.job_id === 'compute_a', 'lazy preview should use the selected job id');
  assertLocal(document.getElementById('btn-preview').disabled === true, 'lazy preview should disable the Preview button while in flight');
  assertLocal(document.getElementById('btn-delete').disabled === true, 'lazy preview should disable Delete while the preview Lambda may still write S3');
  assertLocal(document.getElementById('btn-preview').textContent === 'auto...', 'lazy preview should show the automatic in-flight label');
  assertLocal(document.getElementById('results-info').textContent === 'Generating preview automatically...', 'lazy preview status should not be overwritten by file count');
  const lazyPromise = _resultPreviewInFlight.get('compute_a');
  assertLocal(lazyPromise, 'lazy preview should be tracked in the in-flight map');
  resolvePreview({ image_url: 'https://example.invalid/preview.png', n_roots: 2, n_roots_total: 4, q_re: [0, 1], q_im: [2, 3] });
  await lazyPromise;
  assertLocal(!_resultPreviewInFlight.has('compute_a'), 'lazy preview should clear the in-flight map after success');
  assertLocal(!_resultPreviewInFlightMode.has('compute_a'), 'lazy preview should clear the in-flight mode after success');
  assertLocal(document.getElementById('btn-delete').disabled === false, 'lazy preview should re-enable Delete after the S3 write completes');
  assertLocal(row.has_preview === true && detail.has_preview === true, 'lazy preview success should update both row and cached detail state');

  detail.has_preview = false;
  detail.preview_url = '';
  _applyDetail(row, detail, document.getElementById('results-preview'), document.getElementById('results-info'), 'compute_a');
  assertLocal(calls.length === 1, 'cached generated preview should prevent a second lazy request after stale detail');
  assertLocal(document.getElementById('results-preview').dataset.previewUrl === 'https://example.invalid/preview.png', 'cached generated preview should remain visible after stale detail');

  row.has_preview = false;
  row.preview_url = '';
  detail.has_preview = false;
  detail.preview_url = '';
  _selectedJobId = 'compute_a';
  _applyDetail(row, detail, document.getElementById('results-preview'), document.getElementById('results-info'), 'compute_a');
  assertLocal(__timerCount() === 1, 'stale missing-preview detail should schedule a debounce timer');
  _selectedJobId = 'compute_other';
  _cancelPendingLazyResultPreview();
  assertLocal(__timerCount() === 0, 'selection changes should cancel the pending lazy preview timer before it starts a Lambda');

  let resolveManual = null;
  lambdaPost = (service, payload, route) => new Promise(resolve => { resolveManual = resolve; });
  _selectedJobId = 'compute_b';
  _resultsCache = [{ job_id: 'compute_b' }];
  document.getElementById('btn-preview').textContent = 'Preview';
  const manualPromise = _generateResultPreview('compute_b', { lazy: false });
  await Promise.resolve();
  assertLocal(_resultPreviewInFlightMode.get('compute_b') === 'manual', 'manual preview should be tagged as manual while in flight');
  assertLocal(document.getElementById('btn-preview').textContent === '...', 'manual preview should show the manual in-flight label');
  assertLocal(document.getElementById('btn-delete').disabled === true, 'manual preview should also disable Delete while the preview Lambda may still write S3');
  resolveManual({ image_url: 'https://example.invalid/manual.png', n_roots: 1, n_roots_total: 1, q_re: [0, 1], q_im: [0, 1] });
  await manualPromise;
  assertLocal(document.getElementById('btn-delete').disabled === false, 'manual preview should re-enable Delete after completion');

  lambdaPost = () => { throw new Error('sync preview failure'); };
  _selectedJobId = 'compute_sync';
  _resultsCache = [{ job_id: 'compute_sync' }];
  let failed = false;
  try {
    await _generateResultPreview('compute_sync', { lazy: true });
  } catch (e) {
    failed = true;
  }
  assertLocal(failed, 'synchronous preview failures should still reject');
  assertLocal(!_resultPreviewInFlight.has('compute_sync'), 'synchronous preview failures should not leave stale in-flight entries');
  assertLocal(!_resultPreviewInFlightMode.has('compute_sync'), 'synchronous preview failures should not leave stale in-flight mode entries');
})()
`, ctx, { filename: 'results-lazy-preview-runtime-test.js' });

runtimePromise.then(() => {
  console.log('Frontend results lazy preview runtime checks: OK');
}).catch(err => {
  fail(err && err.stack ? err.stack : String(err));
});
NODE

node - "$HTML" <<'NODE'
const fs = require('fs');
const vm = require('vm');

const htmlPath = process.argv[2];
const htmlSrc = fs.readFileSync(htmlPath, 'utf8');

// The real generated registry vocab (coeff_vocab_js.js): the runtime checks
// below exercise the same artifact the browser loads, so alias drift between
// the registry JSON and the deployed JS fails here.
const vocabSrc = fs.readFileSync(require('path').join(require('path').dirname(htmlPath), 'coeff_vocab_js.js'), 'utf8');
const coeffRegistryVocab = JSON.parse(vocabSrc.slice(vocabSrc.indexOf('{'), vocabSrc.lastIndexOf('}') + 1));
const solveVocabSrc = fs.readFileSync(require('path').join(require('path').dirname(htmlPath), 'solve_score_vocab_js.js'), 'utf8');
const solveScoreVocab = JSON.parse(solveVocabSrc.slice(solveVocabSrc.indexOf('{'), solveVocabSrc.lastIndexOf('}') + 1));

function fail(message) {
  console.error('FATAL: ' + message);
  process.exit(1);
}

// The app's JS is split into ordered js/ parts (classic scripts; see the
// <script src="js/..."> tags). Assemble them in tag order — exactly what the
// browser executes — and fail if the tags and the files on disk disagree.
const path = require('path');
const baseDir = path.dirname(htmlPath);
const partNames = [...htmlSrc.matchAll(/<script src="js\/([^"?]+\.js)"><\/script>/g)].map(m => m[1]);
if (!partNames.length) fail('no js/ part tags found in index.html');
const diskParts = fs.readdirSync(path.join(baseDir, 'js')).filter(f => f.endsWith('.js')).sort();
if (JSON.stringify(diskParts) !== JSON.stringify([...partNames].sort())) {
  fail('js/ files on disk do not match index.html script tags: tags=' + partNames.join(',') + ' disk=' + diskParts.join(','));
}
const src = htmlSrc + '\n' + partNames.map(n => fs.readFileSync(path.join(baseDir, 'js', n), 'utf8')).join('\n');

function fail(message) {
  console.error('FATAL: ' + message);
  process.exit(1);
}

function extractFunction(name) {
  const asyncMarker = `async function ${name}(`;
  const plainMarker = `function ${name}(`;
  let start = src.indexOf(asyncMarker);
  if (start < 0) start = src.indexOf(plainMarker);
  if (start < 0) fail(`missing function ${name}`);
  const brace = src.indexOf('{', start);
  if (brace < 0) fail(`missing opening brace for ${name}`);
  let depth = 0;
  for (let i = brace; i < src.length; i += 1) {
    const ch = src[i];
    if (ch === '{') depth += 1;
    else if (ch === '}') {
      depth -= 1;
      if (depth === 0) return src.slice(start, i + 1);
    }
  }
  fail(`missing closing brace for ${name}`);
}

function extractBetween(startMarker, endMarker, label) {
  const start = src.indexOf(startMarker);
  if (start < 0) fail(`missing start marker for ${label}`);
  const end = src.indexOf(endMarker, start);
  if (end < 0) fail(`missing end marker for ${label}`);
  return src.slice(start, end + endMarker.length);
}

function assert(cond, message) {
  if (!cond) fail(message);
}

async function main() {
  const solveScoreCatalogBlock = extractBetween(
    "const _solveScoreVocab = (typeof window !== 'undefined' && window._solveScoreVocab) || {};",
    "const _ctAndyParam = { kind: 'andy', ph: 'andy', label: 'andy', def: '0', scalarExpr: 'real', title: `Blend amount in [0,1]. ${_coeffProgramScalarExprHelp}` };",
    'solve-score catalog block'
  );
  const code = [
    extractFunction('_str'),
    extractFunction('_pluralize'),
    extractFunction('_displayTransformEntry'),
    extractFunction('_isSolveScoreGenericMetricChipName'),
    extractFunction('_displaySolveScoreEntry'),
    extractFunction('_solveScoreQuantilePctText'),
    extractFunction('_normalizeSolveScoreMetricSource'),
    extractFunction('_splitSolveScoreMetricSourceLag'),
    extractFunction('_solveScoreMetricSourceChoices'),
    extractFunction('_solveScoreMetricsUseSource'),
    extractFunction('_solveScoreMetricsUseNonSolveSource'),
    extractFunction('_solveScoreMetricAllowedSources'),
    extractFunction('_solveScoreMetricCanUseGenericChip'),
    extractFunction('_formatSolveScoreSourceLag'),
    extractFunction('_solveScoreItemMetricDetails'),
    extractFunction('_normalizeSolveScoreChain'),
    extractFunction('_serializeSolveScoreChain'),
    extractFunction('_buildSolveScoreProgramSpec'),
    extractFunction('_compileSolveScoreChain'),
    solveScoreCatalogBlock,
    // The real registry-chip hydrator: builds _ctCatalog/_ctCategoryMeta and
    // the exp/round wide defs from the generated vocab, exactly as shipped.
    extractBetween(
      'function _hydrateCtParamDef(pDef) {',
      "const _coeffProgramRoundParamDefs = _coeffProgramWideParamDefs('round');",
      'registry chip hydrator block'
    ),
    // vm scripts keep top-level consts lexical; bridge them onto the context
    // so the assertions below can inspect the hydrated objects.
    'globalThis._ctCatalog = _ctCatalog; globalThis._ctCategoryMeta = _ctCategoryMeta;',
    extractFunction('_linkedColorIdForPaletteArtifact'),
    extractFunction('_sourceColorArtifactIdForRenderArtifact'),
    extractFunction('_noteSolveScorePopulate'),
    extractFunction('_editorPrefix'),
    extractFunction('_solveScoreSourceTextarea'),
    extractFunction('_getSolveScoreProgramSourceText'),
    extractFunction('_setSolveScoreProgramSourceText'),
    extractFunction('_effectiveSolveScoreProgramSourceText'),
    extractFunction('_requireSolveScoreProgramSourceText'),
    extractFunction('_defaultSolveScoreProgramSourceText'),
    extractFunction('_setPanelTabActive'),
    extractFunction('_setSolveScoreProgramEditorMode'),
    extractFunction('_renderChips'),
    extractFunction('_artifactSolveScoreSourceText'),
    extractFunction('_restoreSolveScoreSourceFromArtifact'),
    extractFunction('setColorMode'),
    extractFunction('_normalizeColorInterpretation'),
    extractFunction('_colorInterpretationLabel'),
    extractFunction('_artifactColorInterpretation'),
    extractFunction('_hasColorRawSidecar'),
    extractFunction('_canColorRepaletteArtifact'),
    extractFunction('_defaultColorRepaletteInterpretation'),
    extractFunction('_artifactOutputChannelCount'),
    extractFunction('_isScalarExtractPaletteSource'),
    extractFunction('_hasFusedStepScorePaletteSource'),
    extractFunction('_findColorArtifactById'),
    extractFunction('_canExtractPaletteArtifact'),
    extractFunction('_extractPaletteLineageHint'),
    'let _activePaletteRun = null; let _lastPaletteLoggedPhase = null; let _lastPaletteWarnState = null; let _palettePhaseTracker = null; const PALETTE_HARD_STALE_MS = 900000; function startActivePaletteObserver() { globalThis._paletteObserverStarts = (globalThis._paletteObserverStarts || 0) + 1; } function stopActivePaletteObserver() { globalThis._paletteObserverStops = (globalThis._paletteObserverStops || 0) + 1; }',
    extractFunction('_saveActivePaletteRun'),
    extractFunction('_clearActivePaletteRun'),
    extractFunction('_loadActivePaletteRun'),
    extractFunction('_currentActivePaletteRun'),
    extractFunction('_paletteRunAgeMs'),
    extractFunction('_paletteRunIsHardStale'),
    extractFunction('_paletteRunBlocksNewRun'),
    extractFunction('_paletteRunLabel'),
    extractFunction('_shouldMirrorPaletteRunToRender'),
    extractFunction('_blockPaletteActionIfActive'),
    extractFunction('_solveScoreColorCompatibility'),
    extractFunction('_solveScorePaletteCompatibility'),
    extractFunction('_launchRenderOrchestrator'),
    extractFunction('runRasterPipeline'),
    extractFunction('_formatCtConstant'),
    extractFunction('_parseCtRealConstant'),
    extractFunction('_hasCtExpressionOperator'),
    extractFunction('_parseCtComplexConstant'),
    extractFunction('_splitCtComplexInput'),
    extractFunction('_isAndyParam'),
    extractFunction('_ctAndyIsDefault'),
    extractFunction('_canonicalCoeffTransformName'),
    extractFunction('_coeffProgramLegacyInputDefs'),
    extractFunction('_canonicalCoeffProgramChipName'),
    extractFunction('_paramValueOrDefault'),
    extractFunction('_serializeCoeffProgramChain'),
    extractFunction('_getCatalogEntry'),
    extractFunction('_isConstCoeffFunction'),
    extractFunction('_constCoeffDefaults'),
    extractFunction('_coeffFuncUiParamCount'),
    extractFunction('_functionCatalogSearchFields'),
    extractFunction('_functionFilterMatcher'),
    extractFunction('_visibleFunctionCatalog'),
    extractFunction('_formatCfpvComplexValue'),
    extractFunction('_parseCfpvComplexValue'),
    extractFunction('_formatCfpvForDisplay'),
  ].join('\n\n');

  const renderStatus = { textContent: '', className: '' };
  const paletteStatus = { textContent: '', className: '' };
  const generateBtn = { disabled: false };
  const ssSourceTextarea = { value: '' };
  const localStorageData = new Map();
  const _ssFakeEls = {
    'render-ss-source-text': ssSourceTextarea,
    'render-ss-text-panel': { classList: { toggle() {} } },
  };
  const ctx = {
    console,
    Math,
    JSON,
    renderColorMode: 'rainbow',
    renderSolveMetric: 'proximity',
    paletteTabMetric: 'proximity',
    _renderArtifacts: { color: [] },
    _renderMtPopupState: { saveAssociatedPalette: false },
    _solveScoreProgramEditorMode: { render: 'text', palette: 'text' },
    _solveScoreProgramRememberedNames: { render: 'pal5', palette: 'keep' },
    _coeffRegistryVocab: coeffRegistryVocab,
    _coeffProgramChain: [],
    _coeffProgramCatalog: {
      push_vec: { params: [{ ph: 'length', def: 'poly_len' }, { ph: 'value', def: '0' }] },
      push_const: { params: [{ ph: 'length', def: 'poly_len' }, { ph: 'value', def: '0' }] },
    },
    _statusCalls: [],
    _logs: [],
    _fusedCalls: [],
    _nonColorCalls: [],
    _paletteObserverStarts: 0,
    _paletteObserverStops: 0,
    _updateSolveScoreButtonsCalls: 0,
    _syncSolveScoreLegacyInputsCalls: 0,
    _scoreNormalizationSyncCalls: 0,
    _stackUiCalls: 0,
    _paletteInterpretationSyncCalls: 0,
    window: {
      _solveScoreVocab: solveScoreVocab,
      _coeffFuncCatalog: [
        { name: 'const', params: [{ name: 'length', default: 35 }, { name: 'value_re', default: 1 }, { name: 'value_im', default: 0 }] },
        { name: 'poly_1', kind: 'legacy', source: 'poly.py', degree: 35 },
        { name: 'poly_10', kind: 'legacy', source: 'poly.py', degree: 35 },
        { name: 'poly_112', kind: 'legacy', source: 'poly.py', degree: 35 },
      ],
    },
    localStorage: {
      getItem(key) { return localStorageData.has(key) ? localStorageData.get(key) : null; },
      setItem(key, value) { localStorageData.set(key, String(value)); },
      removeItem(key) { localStorageData.delete(key); },
    },
    _functionPopupState: { filter: '', highlightIdx: 0 },
    _launchFusedRenderOrchestrator: async (paramsPatch) => {
      ctx._fusedCalls.push(paramsPatch);
      return {};
    },
    _launchNonColorRenderOrchestrator: async (mode, paramsPatch) => {
      ctx._nonColorCalls.push({ mode, paramsPatch });
      return {};
    },
    document: {
      getElementById(id) {
        if (id === 'btn-render-generate') return generateBtn;
        if (id === 'btn-render-generate-mt') return generateBtn;
        if (id === 'render-status') return renderStatus;
        if (id === 'palette-status') return paletteStatus;
        if (id === 'tab-render') return { classList: { contains() { return false; } } };
        if (_ssFakeEls[id]) return _ssFakeEls[id];
        return null;
      },
    },
  };
  ctx._updateRenderActionButtons = () => {};
  ctx._updateSolveScoreButtons = () => {
    ctx._updateSolveScoreButtonsCalls += 1;
  };
  ctx._chainForWhich = () => [];
  ctx._solveScorePrefixForWhich = () => 'render';
  ctx._syncSolveScoreLegacyInputs = () => {
    ctx._syncSolveScoreLegacyInputsCalls += 1;
    return ctx._compileSolveScoreChain([['proximity', 'slv', '0.1']], 'proximity', '0.1');
  };
  ctx._syncScoreNormalizationUi = () => {
    ctx._scoreNormalizationSyncCalls += 1;
  };
  ctx._syncSolveScoreAddOptions = () => {};
  ctx._updateSolveScoreStackUi = () => {
    ctx._stackUiCalls += 1;
  };
  ctx._syncPaletteColorInterpretationUi = () => {
    ctx._paletteInterpretationSyncCalls += 1;
  };
  ctx._setSolveScoreProgramStatus = (prefix, message, isError) => {
    ctx._statusCalls.push({ prefix, message, isError });
  };
  ctx.log = (message, level, target) => {
    ctx._logs.push({ message, level, target });
  };

  vm.createContext(ctx);
  vm.runInContext(code, ctx);

  assert(ctx._defaultSolveScoreProgramSourceText('render') === 'score = metric(proximity, slv, q=0.1%)\n', 'default solve-score source should be a single compilable implicit-score statement');
  assert(!ctx._defaultSolveScoreProgramSourceText('render').includes('emit_norm(score)'), 'default solve-score source must not mix score assignment with explicit emits');
  ssSourceTextarea.value = '';
  let blankGuarded = false;
  try {
    ctx._requireSolveScoreProgramSourceText('render');
  } catch (e) {
    blankGuarded = String(e.message || e).includes('Solve-score source is empty');
  }
  assert(blankGuarded, 'blank solve-score text should be rejected client-side before dispatch');
  ssSourceTextarea.value = 'score = metric(proximity, slv, q=0.1%)\n';
  assert(ctx._requireSolveScoreProgramSourceText('render') === ssSourceTextarea.value, 'nonblank solve-score text should pass the dispatch guard without rewriting source');
  ctx._renderChips('ss');
  assert(ctx._syncSolveScoreLegacyInputsCalls === 1, '_renderChips(ss) should sync derived solve-score state even without #ss-chips');
  assert(ctx._updateSolveScoreButtonsCalls === 1, '_renderChips(ss) should refresh solve-score buttons even without #ss-chips');

  assert(ctx._coeffFuncUiParamCount(ctx.window._coeffFuncCatalog[0]) === 2, 'const coefficient function should present two logical UI parameters');
  ctx._functionPopupState.filter = 'poly_1';
  assert(ctx._visibleFunctionCatalog().map(entry => entry.name).join(',') === 'poly_1,poly_10,poly_112', 'plain coefficient function filter should preserve substring matching');
  ctx._functionPopupState.filter = 'poly_1$';
  assert(ctx._visibleFunctionCatalog().map(entry => entry.name).join(',') === 'poly_1', 'regex coefficient function filter should support exact suffix matching');
  ctx._functionPopupState.filter = '^poly_10$';
  assert(ctx._visibleFunctionCatalog().map(entry => entry.name).join(',') === 'poly_10', 'regex coefficient function filter should support anchored exact matching');
  ctx._functionPopupState.filter = 'poly_[';
  assert(ctx._functionFilterMatcher(ctx._functionPopupState.filter).mode === 'invalid_regex', 'invalid coefficient function regex should be reported');
  assert(ctx._visibleFunctionCatalog().length === 0, 'invalid coefficient function regex should not fall back to broad substring matching');
  assert(ctx._formatCfpvForDisplay('const', [35, 1, -2]) === 'degree=34, value=1-2j', 'const CFPV display should hide native length/re/im slots');
  assert(ctx._formatCfpvForDisplay('const', [8, -3, 10]) === 'degree=7, value=-3+10j', 'const CFPV display should preserve complex values');
  const cfpvComplex = ctx._parseCfpvComplexValue('10j-3');
  assert(cfpvComplex && cfpvComplex.re === -3 && cfpvComplex.im === 10, 'const coefficient value parser should accept imag-first complex constants');
  assert(ctx._splitCtComplexInput('13-22j').re === '13' && ctx._splitCtComplexInput('13-22j').im === '-22', 'complex chip parser should preserve real-first complex constants');

  assert(JSON.stringify(Object.keys(coeffRegistryVocab.ctCatalog)) === JSON.stringify(coeffRegistryVocab.names), 'generated chip entries should cover every registry function in fn_index order');
  assert(JSON.stringify(Object.keys(ctx._ctCatalog)) === JSON.stringify(coeffRegistryVocab.names), 'hydrated _ctCatalog should preserve registry order (the transform picker order)');
  assert(JSON.stringify(Object.keys(ctx._ctCategoryMeta)) === JSON.stringify(['structural', 'accumulation', 'elementwise', 'roots']), 'category order is UI contract');
  assert(ctx._ctCatalog.linear.params.length === 3 && ctx._ctCatalog.rev.params.length === 1, 'hydration should append the shared andy param to every transform');
  assert(ctx._ctCatalog.linear.params[0].title.includes('Program mode accepts') && !ctx._ctCatalog.linear.params[0].title.includes('{SCALAR_EXPR_HELP}'), 'hydration should resolve the scalar-expr help placeholder');
  assert(ctx._coeffProgramWideParamDefs('exp').length === 3 && ctx._coeffProgramWideParamDefs('round').length === 2, 'program-mode exp/round wide defs should hydrate from the vocab');
  assert(ctx._ctCatalog.pow.params[0].def === '1' && ctx._ctCatalog.power.params[0].def === '8' && ctx._ctCatalog.roots.params.length === 3, 'hydrated param shapes should match the registry ui blocks');
  assert(ctx._canonicalCoeffTransformName('pow_affine') === 'pow', 'transform canonicalizer should mirror the backend pow_affine alias');
  assert(ctx._canonicalCoeffTransformName('power_series') === 'power', 'transform canonicalizer should mirror the backend power_series alias');
  assert(ctx._canonicalCoeffTransformName('exp_affine') === 'exp' && ctx._canonicalCoeffTransformName('scale100') === 'linear', 'transform canonicalizer should keep the existing backend aliases');
  assert(ctx._canonicalCoeffTransformName('pow') === 'pow' && ctx._canonicalCoeffTransformName('power') === 'power', 'transform canonicalizer should pass canonical names through');
  assert(ctx._canonicalCoeffProgramChipName('const') === 'push_const', 'chip canonicalizer should map the historical const alias');
  assert(ctx._canonicalCoeffProgramChipName('push_vec') === 'push_vec', 'chip canonicalizer should pass canonical names through');
  ctx._coeffProgramChain = [
    { name: 'push_vec', params: ['poly_len', '0'] },
    { name: 'const', params: ['8', '2'] },
    { name: 'legacy', params: ['cos', 'poly', 'poly', '0'] },
    { name: 'legacy', params: ['cos', 'poly', 'poly', '0.5'] },
    { name: 'legacy', params: ['pow_affine', 'poly', 'poly', '2', '3', '0'] },
    { name: 'legacy', params: ['power_series', 'poly', 'poly', '9'] },
  ];
  const serializedChips = ctx._serializeCoeffProgramChain();
  assert(JSON.stringify(serializedChips[0]) === JSON.stringify(['push_vec', 'poly_len', '0']), 'chip serialization should keep canonical chip rows intact');
  assert(JSON.stringify(serializedChips[1]) === JSON.stringify(['push_const', '8', '2']), 'chip serialization should canonicalize const rows to push_const');
  assert(JSON.stringify(serializedChips[2]) === JSON.stringify(['legacy', 'cos', 'poly', 'poly']), 'chip serialization should trim a default andy from legacy rows');
  assert(JSON.stringify(serializedChips[3]) === JSON.stringify(['legacy', 'cos', 'poly', 'poly', '0.5']), 'chip serialization should keep a non-default andy on legacy rows');
  assert(JSON.stringify(serializedChips[4]) === JSON.stringify(['legacy', 'pow', 'poly', 'poly', '2', '3']), 'imported pow_affine legacy rows should canonicalize to pow and use its catalog defs');
  assert(JSON.stringify(serializedChips[5]) === JSON.stringify(['legacy', 'power', 'poly', 'poly', '9']), 'imported power_series legacy rows should canonicalize to power and use its catalog defs');
  ctx._coeffProgramChain = [];

  const lagged = ctx._compileSolveScoreChain([
    ['spread', 'slv', '0.4'],
    ['spread', 'slv-1', '0.9'],
    ['abs_diff'],
  ], 'spread', '0.1');
  assert(lagged.program_spec === 'm0-0;m0-1;abs_diff', 'lagged UI compiler should lower current/previous refs to one base slot');
  assert(lagged.metrics.length === 1, 'lagged UI compiler should reuse the existing base metric slot');
  assert(Math.abs(lagged.metrics[0].quantile - 0.004) < 1e-12, 'lagged UI compiler should inherit the current slot quantile');
  assert(lagged.prelude_by_source.slv === 1 && lagged.max_lag === 1, 'lagged UI compiler should require one solve prelude row');

  const laggedOnly = ctx._compileSolveScoreChain([
    ['proximity', 'cf-1', '0.7'],
  ], 'proximity', '0.1');
  assert(laggedOnly.program_spec === 'm0-1', 'lagged-only UI compiler should emit a previous-ref token');
  assert(laggedOnly.metrics[0].source === 'cf', 'lagged-only UI compiler should preserve the source family');
  assert(Math.abs(laggedOnly.metrics[0].quantile - 0.007) < 1e-12, 'lagged-only UI compiler should seed the base slot quantile');
  assert(laggedOnly.prelude_by_source.cf === 1, 'lagged-only UI compiler should require a coeff prelude row');

  const genericMetric = ctx._compileSolveScoreChain([
    ['metric', 'angular_entropy_16', 'cf', '0.5'],
    ['metric', 'angular_entropy_16', 'cf-1', '0.5'],
    ['abs_diff'],
  ], 'proximity', '0.1');
  const genericNormalized = ctx._normalizeSolveScoreChain([
    ['metric', 'angular_entropy_16', 'cf', '0.5'],
  ], 'proximity', '0.1');
  assert(genericMetric.program_spec === 'm0-0;m0-1;abs_diff', 'generic metric chip should compile to normal metric refs');
  assert(genericMetric.metrics.length === 1, 'generic metric chip should reuse current/lagged base slot');
  assert(genericMetric.metrics[0].metric === 'angular_entropy_16', 'generic metric chip should compile selected metric name');
  assert(genericMetric.metrics[0].source === 'cf', 'generic metric chip should compile selected source');
  assert(genericNormalized[0].name === '__metric', 'public generic metric chip should normalize to internal macro name');
  assert(JSON.stringify(genericMetric.chain) === JSON.stringify([['metric', 'angular_entropy_16', 'cf', '0.5'], ['metric', 'angular_entropy_16', 'cf-1', '0.5'], ['abs_diff']]), 'generic metric chip should remain generic in serialized editor chain');

  const genericMetricInternal = ctx._compileSolveScoreChain([
    ['__metric', 'angular_entropy_16', 'cf', '0.5'],
  ], 'proximity', '0.1');
  assert(JSON.stringify(genericMetricInternal.chain) === JSON.stringify([['metric', 'angular_entropy_16', 'cf', '0.5']]), 'internal generic metric chip should serialize using the public saved-program name');

  const explicitEmit = ctx._compileSolveScoreChain([
    ['proximity', 'slv', '0.5'],
    ['emit', 'norm'],
    ['spread', 'cf', '0.5'],
    ['emit', 'raw'],
  ], 'proximity', '0.1');
  assert(explicitEmit.program_spec === 'm0-0;emit_norm;m1-0;emit', 'explicit output chips should compile into program_spec');
  assert(explicitEmit.has_explicit_outputs === true, 'explicit output chips should mark the compiled chain');
  assert(explicitEmit.output_channel_count === 2, 'explicit output chips should produce two output channels');
  assert(explicitEmit.output_channels[0].range_normalized === true, 'emit_norm should request per-channel range normalization');
  assert(explicitEmit.output_channels[1].range_normalized === false, 'emit should skip per-channel range normalization');
  assert(ctx._solveScorePaletteCompatibility(explicitEmit, 'scalar_lut').includes('expected 1 output'), 'scalar LUT palette generation should require one output');
  const explicitRgb = ctx._compileSolveScoreChain([
    ['proximity', 'slv', '0.5'],
    ['emit', 'norm'],
    ['spread', 'cf', '0.5'],
    ['emit', 'raw'],
    ['angular_entropy_16', 'slv', '0.5'],
    ['emit', 'raw'],
  ], 'proximity', '0.1');
  assert(ctx._solveScorePaletteCompatibility(explicitRgb, 'rgb') === '', 'RGB palette generation should accept explicit 3-output programs');
  assert(ctx._solveScorePaletteCompatibility(ctx._compileSolveScoreChain([['proximity', 'slv', '0.5']], 'proximity', '0.1')) === '', 'palette generation should accept scalar solve-score programs in the frontend');

  const emitNone = ctx._compileSolveScoreChain([
    ['proximity', 'slv', '0.5'],
    ['emit', 'none'],
    ['flush'],
    ['spread', 'slv', '0.5'],
    ['emit', 'norm'],
  ], 'proximity', '0.1');
  assert(emitNone.program_spec === 'm0-0;emit_none;flush;m1-0;emit_norm', 'emit none and flush should compile into native score program');
  assert(emitNone.has_explicit_outputs === true, 'emit none should keep explicit-output validation active');
  assert(emitNone.output_channel_count === 1, 'emit none should not allocate an output channel');
  assert(emitNone.output_channels[0].emit === 'emit_norm', 'later emissions should retain correct channel metadata');

  const mathEmit = ctx._compileSolveScoreChain([
    ['proximity', 'slv', '0.5'],
    ['const', '1e-3'],
    ['add'],
    ['dup'],
    ['ema', '0.99'],
    ['sin'],
    ['pow', '2'],
    ['clamp'],
  ], 'proximity', '0.1');
  assert(mathEmit.program_spec === 'm0-0;const:0.001;add;dup;ema:0.99;sin;pow:2;clamp', 'new stack/math chips should compile into program_spec');

  const rgbLutProgram = ctx._compileSolveScoreChain([
    ['proximity', 'slv', '0.5'],
    ['emit', 'norm'],
    ['spread', 'slv', '0.5'],
    ['emit', 'norm'],
    ['angular_entropy_16', 'slv', '0.5'],
    ['emit', 'norm'],
  ], 'proximity', '0.1');
  assert(ctx._normalizeColorInterpretation('rgb-lut') === 'rgb_lut', 'RGB LUT alias should normalize');
  assert(ctx._normalizeColorInterpretation('hsv-lut') === 'hsv_lut', 'HSV LUT alias should normalize');
  assert(ctx._solveScoreColorCompatibility(rgbLutProgram, 'rgb_lut') === '', 'RGB LUT should accept three emitted outputs');
  assert(ctx._solveScoreColorCompatibility(rgbLutProgram, 'hsv_lut') === '', 'HSV LUT should accept three emitted outputs');
  assert(ctx._artifactColorInterpretation({ color_interpretation: 'direct_rgb' }) === 'rgb', 'artifact color interpretation should normalize color_interpretation aliases');
  assert(ctx._artifactColorInterpretation({ score_output_interpretation: 'hsv-lut' }) === 'hsv_lut', 'artifact color interpretation should fall back to score_output_interpretation');
  assert(ctx._colorInterpretationLabel('rgb_lut') === 'RGB LUT', 'artifact summary labels should use user-facing color mode names');
  const scalarFusedColor = { family: 'color', artifact_id: 'scalar_fused', color_mode: 'solve_score', color_interpretation: 'scalar_lut', raw_channels: 1, raw_key: 'raw', raw_meta_key: 'raw.meta', step_scores_key: 'steps' };
  const scalarLegacyColor = { family: 'color', artifact_id: 'scalar_legacy', color_mode: 'solve_score' };
  const rgbColor = { family: 'color', artifact_id: 'rgb_color', color_mode: 'solve_score', color_interpretation: 'rgb', raw_channels: 3 };
  const rgbFusedColor = { family: 'color', artifact_id: 'rgb_fused', color_mode: 'solve_score', color_interpretation: 'rgb_lut', raw_channels: 3, raw_key: 'raw3', raw_meta_key: 'raw3.meta', step_scores_key: 'steps3' };
  const twoChannelColor = { family: 'color', artifact_id: 'two_channel', color_mode: 'solve_score', color_interpretation: 'rgb_lut', raw_channels: 2, raw_key: 'raw2', raw_meta_key: 'raw2.meta' };
  const hsvFusedColor = { family: 'color', artifact_id: 'hsv_fused', color_mode: 'solve_score', color_interpretation: 'hsv', raw_channels: 3, raw_key: 'rawh', raw_meta_key: 'rawh.meta' };
  const rgbAssociatedColor = { family: 'color', artifact_id: 'rgb_assoc', color_mode: 'solve_score', color_interpretation: 'rgb', raw_channels: 3, associated_palette_id: 'pal_rgb' };
  const savedPaletteColor = { artifact_id: 'saved_pal', color_mode: 'saved_palette', palette_source_id: 'pal_1' };
  const savedPaletteRawColor = { family: 'color', artifact_id: 'saved_pal_raw', color_mode: 'saved_palette', palette_source_id: 'pal_1', raw_channels: 1, raw_key: 'saved.raw', raw_meta_key: 'saved.meta' };
  const childScalarColor = { artifact_id: 'child_scalar', color_mode: 'postprocess', derived_from_artifact_id: 'scalar_fused' };
  const childRgbColor = { artifact_id: 'child_rgb', color_mode: 'postprocess', derived_from_artifact_id: 'rgb_color' };
  const childRgbFusedColor = { artifact_id: 'child_rgb_fused', color_mode: 'postprocess', derived_from_artifact_id: 'rgb_fused' };
  ctx._renderArtifacts.color = [scalarFusedColor, scalarLegacyColor, rgbColor, rgbFusedColor, rgbAssociatedColor, savedPaletteColor, childScalarColor, childRgbColor, childRgbFusedColor];
  assert(ctx._canColorRepaletteArtifact(scalarFusedColor), 'Color RePalette should accept scalar raw sidecars');
  assert(ctx._canColorRepaletteArtifact(rgbFusedColor), 'Color RePalette should accept three-channel raw sidecars');
  assert(ctx._canColorRepaletteArtifact(savedPaletteRawColor), 'Color RePalette should accept saved-palette raw sidecars');
  assert(!ctx._canColorRepaletteArtifact(twoChannelColor), 'Color RePalette should reject unsupported channel counts before dispatch');
  assert(ctx._defaultColorRepaletteInterpretation(rgbFusedColor) === 'rgb_lut', 'Color RePalette should default RGB sources to RGB LUT');
  assert(ctx._defaultColorRepaletteInterpretation(hsvFusedColor) === 'hsv_lut', 'Color RePalette should default HSV sources to HSV LUT');
  assert(ctx._canExtractPaletteArtifact(scalarFusedColor), 'ExtractPalette should be enabled for scalar fused step-score artifacts');
  assert(ctx._canExtractPaletteArtifact(rgbFusedColor), 'ExtractPalette should be enabled for three-channel fused step-score artifacts');
  assert(ctx._canExtractPaletteArtifact(scalarLegacyColor), 'ExtractPalette should keep the legacy scalar solve-score path enabled');
  assert(ctx._canExtractPaletteArtifact(rgbAssociatedColor), 'ExtractPalette should be enabled for existing associated RGB palette artifacts');
  assert(ctx._canExtractPaletteArtifact(savedPaletteColor), 'ExtractPalette should be enabled for saved-palette dependencies');
  assert(ctx._canExtractPaletteArtifact(childScalarColor), 'ExtractPalette should follow scalar extractable color lineage');
  assert(ctx._canExtractPaletteArtifact(childRgbFusedColor), 'ExtractPalette should follow three-channel extractable color lineage');
  assert(!ctx._canExtractPaletteArtifact(rgbColor), 'ExtractPalette should be disabled for RGB artifacts without an associated palette artifact');
  assert(!ctx._canExtractPaletteArtifact(childRgbColor), 'ExtractPalette should be disabled for descendants of unsupported RGB artifacts');
  assert(ctx._extractPaletteLineageHint(rgbColor).kind === 'unsupported', 'ExtractPalette lineage hint should explain unsupported multi-output artifacts');
  assert(ctx._extractPaletteLineageHint(scalarFusedColor).kind === 'fused', 'ExtractPalette lineage hint should prefer fused scalar step-score extraction');
  assert(ctx._extractPaletteLineageHint(rgbFusedColor).kind === 'fused', 'ExtractPalette lineage hint should prefer fused three-channel step-score extraction');
  assert(ctx._extractPaletteLineageHint(scalarLegacyColor).kind === 'solve_score', 'ExtractPalette lineage hint should retain legacy scalar dispatch');

  ctx._logs = [];
  paletteStatus.textContent = '';
  paletteStatus.className = '';
  ctx.localStorage.setItem('polypaint_active_palette_run', JSON.stringify({
    job_id: 'compute_mnrwtbnl',
    run_id: 'run_active',
    task_id: 'extract_palette_run_run_active',
    mode: 'extract_palette',
    origin: 'render_extract_palette',
    started_at_ms: Date.now(),
  }));
  assert(ctx._paletteRunBlocksNewRun() === true, 'fresh persisted ExtractPalette lock should disable new Palette/ExtractPalette actions');
  assert(ctx._blockPaletteActionIfActive('ExtractPalette') === true, 'fresh persisted ExtractPalette lock should visibly block the popup/run path');
  assert(paletteStatus.textContent.includes('already in progress'), 'fresh persisted ExtractPalette lock should write a visible palette status');
  assert(ctx._logs.some(entry => entry.target === 'palette-log' && entry.message.includes('already in progress')), 'fresh persisted ExtractPalette lock should log why the click was blocked');
  assert(ctx._paletteObserverStarts > 0, 'fresh persisted ExtractPalette lock should resume the observer instead of silently returning');
  assert(ctx.localStorage.getItem('polypaint_active_palette_run') !== null, 'fresh persisted ExtractPalette lock should remain stored while active');

  ctx._clearActivePaletteRun();
  ctx._logs = [];
  paletteStatus.textContent = '';
  paletteStatus.className = '';
  ctx.localStorage.setItem('polypaint_active_palette_run', JSON.stringify({
    job_id: 'compute_mnrwtbnl',
    run_id: 'run_stale',
    task_id: 'extract_palette_run_run_stale',
    mode: 'extract_palette',
    origin: 'render_extract_palette',
    started_at_ms: Date.now() - 16 * 60 * 1000,
  }));
  assert(ctx._paletteRunBlocksNewRun() === false, 'hard-stale persisted ExtractPalette lock should not keep the toolbar disabled');
  assert(ctx._blockPaletteActionIfActive('ExtractPalette') === false, 'hard-stale persisted ExtractPalette lock should clear and allow the popup/run path');
  assert(ctx.localStorage.getItem('polypaint_active_palette_run') === null, 'hard-stale persisted ExtractPalette lock should be removed from localStorage');
  assert(ctx._logs.some(entry => entry.target === 'palette-log' && entry.message.includes('lock was stale')), 'hard-stale persisted ExtractPalette lock should log that it was cleared');

  const previewLogCode = [
    extractFunction('_formatChainRowsForLog'),
    extractFunction('_formatParamProgramChainForLog'),
    extractFunction('_displayActiveParamPipeline'),
    extractFunction('_formatCoeffProgramChainForLog'),
    extractFunction('_displayActiveCoeffPipeline'),
  ].join('\n\n');
  const previewCtx = {
    _serializeParamProgramChain() { return [['push', 't1'], ['emit', 'p1']]; },
    _displayParamTransforms() { throw new Error('legacy param transforms should not be displayed'); },
    _serializeCoeffProgramChain() { return [['rev', 'poly', 'poly'], ['emit']]; },
    _serializeCoeffTransforms() { throw new Error('legacy coeff transforms should not be displayed'); },
  };
  vm.createContext(previewCtx);
  vm.runInContext(previewLogCode, previewCtx);
  assert(previewCtx._displayActiveCoeffPipeline(',') === 'rev(poly,poly),emit', 'Program-mode coeff preview log should format direct native program rows');
  assert(previewCtx._displayActiveParamPipeline(',') === 'push(t1),emit(p1)', 'Program-mode param preview log should format program rows');

  let genericPmRejected = false;
  try {
    ctx._compileSolveScoreChain([['metric', 'angular_entropy_16', 'pm', '0.5']], 'proximity', '0.1');
  } catch (err) {
    genericPmRejected = String(err && err.message || err).includes('source must be one of slv, cf');
  }
  assert(genericPmRejected, 'generic metric chip should reject pm source instead of silently coercing to slv');

  let genericParamMetricRejected = false;
  try {
    ctx._compileSolveScoreChain([['metric', 't1_abs', 'slv', '0.5']], 'proximity', '0.1');
  } catch (err) {
    genericParamMetricRejected = String(err && err.message || err).includes('slv/cf-capable metric');
  }
  assert(genericParamMetricRejected, 'generic metric chip should reject pm-only metrics');

  const laggedBeforeCurrent = ctx._compileSolveScoreChain([
    ['spread', 'slv-1', '0.9'],
    ['spread', 'slv', '0.4'],
    ['abs_diff'],
  ], 'spread', '0.1');
  assert(laggedBeforeCurrent.program_spec === 'm0-1;m0-0;abs_diff', 'lagged UI compiler should bind previous refs to later unambiguous current slots');
  assert(laggedBeforeCurrent.metrics.length === 1, 'lagged-before-current UI compiler should still use one base metric slot');
  assert(Math.abs(laggedBeforeCurrent.metrics[0].quantile - 0.004) < 1e-12, 'lagged-before-current UI compiler should inherit later current slot quantile');

  let ambiguousRejected = false;
  try {
    ctx._compileSolveScoreChain([
      ['spread', 'slv', '0.4'],
      ['spread', 'slv', '0.5'],
      ['spread', 'slv-1', '0.4'],
      ['avg'],
      ['avg'],
    ], 'spread', '0.1');
  } catch (err) {
    ambiguousRejected = String(err && err.message || err).includes('ambiguous');
  }
  assert(ambiguousRejected, 'lagged UI compiler should reject ambiguous previous refs');

  ctx._updateSolveScoreButtonsCalls = 0;
  ctx.setColorMode('proximity');
  assert(ctx.renderColorMode === 'solve_score', 'setColorMode should force solve_score at runtime');
  assert(ctx._updateSolveScoreButtonsCalls === 1, 'setColorMode should refresh solve-score buttons');

  await ctx._launchRenderOrchestrator('color', { sentinel: 1 });
  assert(ctx._fusedCalls.length === 1, 'color launch should route through fused orchestrator');
  assert(ctx._fusedCalls[0].sentinel === 1, 'color launch should preserve paramsPatch');

  await ctx._launchRenderOrchestrator('bilevel', { sentinel: 2 });
  assert(ctx._nonColorCalls.length === 1, 'non-color launch should route through non-color orchestrator');
  assert(ctx._nonColorCalls[0].mode === 'bilevel', 'non-color launch should preserve mode');

  ctx._statusCalls = [];
  ctx._solveScoreProgramRememberedNames.render = 'pal5';
  ctx._noteSolveScorePopulate('render', { family: 'color', artifact_id: 'color_run_42' });
  assert(ctx._solveScoreProgramRememberedNames.render === '', 'populate should clear stale remembered solve-score names');
  assert(ctx._statusCalls[0].message === 'Populated from color_run_42', 'color populate should report the source color artifact id');

  ctx._statusCalls = [];
  ctx._solveScoreProgramRememberedNames.render = 'pal7';
  ctx._noteSolveScorePopulate('render', { family: 'palette', artifact_id: 'pal_7' });
  assert(ctx._statusCalls[0].message === 'Populated from palette pal_7', 'standalone palette populate should not pretend to come from a color artifact');

  // Populate solve-score reset: a chain-only artifact must clear stale text and
  // stay text-only. Backend inventory should reconstruct old chains to source;
  // if it did not, the editor must fail visibly rather than show internal chips.
  ssSourceTextarea.value = 'STALE m0-0;emit(norm)';
  ctx._solveScoreProgramEditorMode.render = 'text';
  ctx._statusCalls = [];
  const ssCleared = ctx._restoreSolveScoreSourceFromArtifact('render', { solve_score_chain: [['proximity', 'slv', '0.1']] });
  assert(ssCleared === false, 'populate from a chip-only artifact should report no source restored');
  assert(ssSourceTextarea.value === '', 'populate must clear stale solve-score text when no reconstructed source exists');
  assert(ctx._solveScoreProgramEditorMode.render === 'text', 'populate from a chain-only artifact should remain in text mode');
  assert(ctx._statusCalls.some(call => call.isError === true), 'populate from a chain-only artifact should show a visible source-missing error');

  // An artifact that DOES carry a text program restores it verbatim and shows text.
  ssSourceTextarea.value = '';
  const ssRestored = ctx._restoreSolveScoreSourceFromArtifact('render', { solve_score_program_source_text: 'score = metric(proximity, slv, q=0.1%)\n' });
  assert(ssRestored === true, 'populate from a source-text artifact should report source restored');
  assert(ssSourceTextarea.value === 'score = metric(proximity, slv, q=0.1%)\n', 'populate should restore the artifact text program verbatim');
  assert(ctx._solveScoreProgramEditorMode.render === 'text', 'populate from a source-text artifact should switch the editor to text');

  ctx.renderColorMode = 'solve_score';
  ctx._renderMtPopupState.saveAssociatedPalette = true;
  ctx._fusedCalls = [];
  ctx._logs = [];
  generateBtn.disabled = false;
  renderStatus.textContent = '';
  renderStatus.className = '';
  await ctx.runRasterPipeline();
  assert(ctx._fusedCalls.length === 1, 'runRasterPipeline should dispatch exactly one fused launch');
  const launch = ctx._fusedCalls[0];
  assert(launch.raster_engine === 'mt', 'runRasterPipeline should use fused mt raster engine');
  assert(launch.raster_mt_threads === 4, 'runRasterPipeline should default raster threads to 4');
  assert(launch.solve_score_threads === 4, 'runRasterPipeline should default clip threads to 4');
  assert(launch.raster_workers === 10, 'runRasterPipeline should default raster workers to 10');
  assert(launch.raster_section_mode === 'logical_sections_auto', 'runRasterPipeline should default raster sections to auto');
  assert(launch.raster_sectioned_retries === 2, 'runRasterPipeline should default fused raster retries to 2');
  assert(launch.finalize_workers === 16, 'runRasterPipeline should default finalize workers to 16');
  assert(launch.save_associated_palette === true, 'runRasterPipeline should forward associated palette selection');
  assert(generateBtn.disabled === false, 'runRasterPipeline should re-enable the Generate button after dispatch');

  ctx.renderColorMode = 'rainbow';
  ctx._fusedCalls = [];
  renderStatus.textContent = '';
  renderStatus.className = '';
  await ctx.runRasterPipeline();
  assert(ctx._fusedCalls.length === 0, 'runRasterPipeline should reject unsupported color modes before dispatch');
  assert(renderStatus.textContent.includes('Solve score only'), 'runRasterPipeline should surface an actionable fused-only error');

  console.log('Frontend fused render runtime checks: OK');
}

main().catch((err) => fail(err && err.stack ? err.stack : String(err)));
NODE


# --- Sequential load gate (browser semantics) ---
# Classic scripts hoist per-file only: top-level code in one part must not
# reference functions defined in a later part. Concatenated extraction (the
# checks above) cannot catch that, so execute each script tag separately in
# index.html order with a lenient DOM stub — exactly how the browser loads.
node - "$HTML" <<'NODE'
const fs = require('fs'), vm = require('vm'), path = require('path');
const root = path.dirname(process.argv[2]);
const html = fs.readFileSync(process.argv[2], 'utf8');
const tags = [...html.matchAll(/<script src="(js\/[^"?]+\.js|[a-z_]+_js\.js)"><\/script>/g)].map(m => m[1]);
if (tags.length < 13) { console.error('FATAL: expected generated catalogs + js parts, found ' + tags.length + ' tags'); process.exit(1); }
function el() {
  return new Proxy(function () {}, {
    get(t, p) {
      if (p === Symbol.toPrimitive || p === 'toString') return () => '';
      if (p === 'length') return 0;
      return el();
    },
    apply() { return el(); },
    set() { return true; },
  });
}
const ctx = {
  console: { log() {}, warn() {}, error() {}, info() {} },
  document: el(), navigator: el(), location: { search: '', href: '', origin: '' },
  localStorage: { getItem: () => null, setItem() {}, removeItem() {} },
  setTimeout: () => 0, clearTimeout() {}, setInterval: () => 0, clearInterval() {},
  fetch: () => new Promise(() => {}), URLSearchParams, URL, Math, JSON, Date, RegExp,
  Promise, Array, Object, Number, String, Boolean, Map, Set, Symbol, Infinity, NaN,
  isFinite, isNaN, parseFloat, parseInt, structuredClone: x => x,
  requestAnimationFrame: () => 0, performance: { now: () => 0 },
  Image: function () { return el(); }, AudioContext: function () { return el(); },
  addEventListener() {}, removeEventListener() {}, alert() {}, confirm: () => false,
  Blob: function () {}, FileReader: function () { return el(); }, atob: s => s, btoa: s => s,
};
ctx.window = ctx; ctx.globalThis = ctx;
vm.createContext(ctx);
for (const tag of tags) {
  try {
    vm.runInContext(fs.readFileSync(path.join(root, tag), 'utf8'), ctx, { filename: tag });
  } catch (e) {
    console.error('FATAL: sequential load failed at ' + tag + ': ' + e.constructor.name + ': ' + e.message);
    console.error('(top-level code referencing a later part? move the call to js/12 boot or the definition earlier)');
    process.exit(1);
  }
}
const parts = ctx.__ppParts || [];
if (parts.length !== tags.filter(t => t.startsWith('js/')).length) {
  console.error('FATAL: part registrations (' + parts.length + ') do not match js/ tags');
  process.exit(1);
}
console.log('Frontend sequential load checks: OK (' + tags.length + ' scripts)');
NODE

node - "$HTML" <<'NODE'
const fs = require('fs'), vm = require('vm'), path = require('path');
const root = path.dirname(process.argv[2]);
const html = fs.readFileSync(process.argv[2], 'utf8');
const scripts = ['js/13-artifact-mosaics.js'];
function assert(cond, msg) { if (!cond) throw new Error(msg); }
function makeEl(id) {
  return {
    id,
    style: {},
    textContent: '',
    className: '',
    value: '',
    disabled: false,
    options: [],
    appendChild(opt) { this.options.push(opt); },
    set innerHTML(v) { this.options = []; },
    get innerHTML() { return ''; },
  };
}
const els = {
  'allcol-status': makeEl('allcol-status'),
  'allcol-summary': makeEl('allcol-summary'),
  'btn-allcol-refresh': makeEl('btn-allcol-refresh'),
  'allcol-size-filter': makeEl('allcol-size-filter'),
  'allcol-sort-mode': makeEl('allcol-sort-mode'),
  'allcol-cols': makeEl('allcol-cols'),
  'allcol-viewer': makeEl('allcol-viewer'),
  'allpal-status': makeEl('allpal-status'),
  'allpal-summary': makeEl('allpal-summary'),
  'btn-allpal-refresh': makeEl('btn-allpal-refresh'),
  'allpal-size-filter': makeEl('allpal-size-filter'),
  'allpal-sort-mode': makeEl('allpal-sort-mode'),
  'allpal-cols': makeEl('allpal-cols'),
  'allpal-viewer': makeEl('allpal-viewer'),
};
  els['allcol-size-filter'].value = 'all';
  els['allcol-sort-mode'].value = 'date';
  els['allpal-size-filter'].value = 'all';
  els['allpal-sort-mode'].value = 'date';
  const opened = [];
  const logs = [];
  const timers = [];
  let imagePoint = {x: 1, y: 1};
  let selectedJob = '';
  let selectedTab = '';
  let selectedArtifact = '';
  let selectedFamily = '';
  let statusFetchFails = false;
  const ctx = {
  console,
  document: {
    getElementById(id) { return els[id] || null; },
    createElement(tag) { return {tagName: tag, value: '', textContent: ''}; },
  },
  window: {},
  setTimeout(fn, ms) { timers.push({fn, ms}); return timers.length; },
  clearTimeout() {},
  Date: { now: () => 12345 },
  Math, JSON, Number, String, Boolean, Array, Object, Map, Set, Promise, URL,
	  OpenSeadragon(opts) {
	    return {
	      opts,
	      viewport: { goHome() {}, pointFromPixel() { return {}; }, viewportToImageCoordinates() { return imagePoint; } },
	      addHandler() {},
	      open(src) { opened.push(src); },
	    };
	  },
	  _ensureResultsSelection: async (jobId) => { selectedJob = jobId; },
	  switchTab: (name) => { selectedTab = name; },
	  refreshRenderArtifacts: async (jobId, opts) => {
	    selectedFamily = opts && opts.selectFamily;
	    selectedArtifact = opts && opts.selectArtifactId;
	  },
	  log: (msg, cls, target) => { logs.push({msg, cls, target}); },
	  lambdaPost: async (service, payload, pathName) => {
	    if (pathName !== '/list-color-mosaic' && pathName !== '/list-palette-mosaic') throw new Error('unexpected path ' + pathName);
	    if (!(payload && payload.refresh) && statusFetchFails) throw new Error('network blip');
	    if (payload && payload.refresh) return {
	      state: 'computing',
	      refresh_id: 'mosaic_x',
	      progress_message: pathName === '/list-palette-mosaic' ? 'Scanning palette jobs: 10/20' : 'Scanning jobs: 10/20',
	      progress_jobs_done: 10,
	      progress_jobs_total: 20,
	      progress_artifacts_total: 40,
	      progress_last_job: 'compute_demo',
	    };
    return {
      state: 'ready',
      refresh_id: pathName === '/list-palette-mosaic' ? 'palette_ready' : 'mosaic_ready',
      manifest_url: pathName === '/list-palette-mosaic' ? 'https://example.test/palette.json' : 'https://example.test/color.json',
      count: pathName === '/list-palette-mosaic' ? 2 : 2,
    };
  },
  fetch: async (url) => {
    const isPalette = String(url).includes('palette.json');
    return {
      ok: true,
      json: async () => isPalette ? ({
        refresh_id: 'palette_ready',
        base: 'https://bucket.test/',
        count: 2,
        sizes: [500, 512],
        tiles: [
          {key:'renders/j/palettes/pal_a/preview.png', job_id:'j', artifact_id:'pal_a', palette_id:'pal_a', created_at:'2026', preview_width:512, preview_height:512, N:512},
          {key:'renders/j/palettes/pal_b/preview.png', job_id:'j', artifact_id:'pal_b', palette_id:'pal_b', created_at:'2025', preview_width:500, preview_height:500, N:1024},
        ],
      }) : ({
        refresh_id: 'mosaic_ready',
        base: 'https://bucket.test/',
        count: 2,
        sizes: [512, 1024],
        tiles: [
          {key:'renders/j/color/a/preview.png', job_id:'j', artifact_id:'a', created_at:'2026', preview_width:512, preview_height:512},
          {key:'renders/j/color/b/preview.png', job_id:'j', artifact_id:'b', created_at:'2025', preview_width:1024, preview_height:1024},
        ],
      }),
    };
  },
};
ctx.window = ctx; ctx.globalThis = ctx;
vm.createContext(ctx);
for (const script of scripts) vm.runInContext(fs.readFileSync(path.join(root, script), 'utf8'), ctx, {filename: script});

(async () => {
	  await ctx.loadAllCol();
	  assert(opened.length === 1, 'loadAllCol should open a tile source');
	  assert(opened[0].getTileUrl(0, 0, 0) === 'https://bucket.test/renders/j/color/a/preview.png', 'tile 0 URL mismatch');
	  await ctx.loadAllCol();
	  assert(opened.length === 1, 'same manifest and controls should not reopen/reset the viewer');
	  els['allcol-size-filter'].value = '1024';
	  ctx._allColRebuild();
	  assert(opened.length === 2, 'size filter should reopen tile source');
	  assert(opened[1].tileSize === 1024, '1024 filter should use 1024 tileSize');
	  assert(opened[1].width === 1024 && opened[1].height === 1024, '1024 tile source dimensions should match 1024 tile size');
	  assert(opened[1].getTileUrl(0, 0, 0) === 'https://bucket.test/renders/j/color/b/preview.png', '1024 filter should select 1024 tile');
	  els['allcol-size-filter'].value = 'all';
	  els['allcol-cols'].value = '1';
	  ctx._allColRebuild();
	  assert(opened.length === 3, 'column-count change should reopen tile source');
	  els['allcol-cols'].value = '2';
	  imagePoint = {x: 1, y: 1025};
	  await ctx._artifactMosaicCanvasClick('color', {quick: true, position: {x: 0, y: 0}});
	  assert(selectedJob === 'j' && selectedTab === 'render' && selectedFamily === 'color' && selectedArtifact === 'b',
	    'click mapping should use rendered tile-source columns, not the current control value');
	  await ctx.refreshAllColMosaic();
	  assert(els['btn-allcol-refresh'].disabled === true, 'refresh should disable button while computing');
	  assert(els['allcol-status'].textContent.includes('jobs 10/20'), 'refresh status should show job progress');
	  assert(els['allcol-status'].textContent.includes('compute_demo'), 'refresh status should show last scanned job');
	  assert(logs.some(row => row.target === 'allcol-log' && row.msg.includes('jobs 10/20')),
	    'refresh should log AllCol progress');
	  const timerCountBeforeError = timers.length;
	  statusFetchFails = true;
	  await ctx.loadAllCol({forceStatus: true, fromPoll: true});
	  assert(timers.length > timerCountBeforeError, 'transient poll error should reschedule polling');
	  assert(els['btn-allcol-refresh'].disabled === true, 'transient poll error should keep refresh button busy');
	  statusFetchFails = false;
	  await ctx.loadAllPal();
	  assert(els['allpal-size-filter'].options.map(o => o.value).join('|') === 'all|500|512',
	    'AllPal should populate dynamic preview-size options');
	  els['allpal-size-filter'].value = '500';
	  ctx._allPalRebuild();
	  const palSource = opened[opened.length - 1];
	  assert(palSource.tileSize === 500, 'AllPal exact preview-size filter should use selected tile size');
	  assert(palSource.getTileUrl(0, 0, 0) === 'https://bucket.test/renders/j/palettes/pal_b/preview.png',
	    'AllPal 500 filter should select 500 preview tile');
	  imagePoint = {x: 1, y: 1};
	  await ctx._artifactMosaicCanvasClick('palette', {quick: true, position: {x: 0, y: 0}});
	  assert(selectedJob === 'j' && selectedTab === 'render' && selectedFamily === 'palette' && selectedArtifact === 'pal_b',
	    'AllPal click should select palette artifact in render tab');
	  console.log('Frontend artifact mosaic runtime checks: OK');
})().catch(e => { console.error(e.stack || String(e)); process.exit(1); });
NODE

echo "=== Frontend fused render source test passed ==="
