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
grep -q 'No temp files, no physical chunks, no separate histogram or palette-chunk stages.' "$HTML" || { echo "FATAL: fused Generate-MT intro missing"; exit 1; }
! grep -q 'id="render-mt-tab-classic"' "$HTML" || { echo "FATAL: classic render MT tab should be removed"; exit 1; }
! grep -q 'id="render-mt-classic-panel"' "$HTML" || { echo "FATAL: classic render MT panel should be removed"; exit 1; }
! grep -q 'id="render-mt-raster-input-mode"' "$HTML" || { echo "FATAL: classic raster input selector should be removed"; exit 1; }
! grep -q 'id="render-mt-hist-retries"' "$HTML" || { echo "FATAL: classic hist retries control should be removed"; exit 1; }
! grep -q 'id="render-generate-hist-input-mode"' "$HTML" || { echo "FATAL: fused Generate popup should not show histogram-input control"; exit 1; }
! grep -q 'id="btn-render-generate-from-palette"' "$HTML" || { echo "FATAL: GenerateFromPalette action should be removed"; exit 1; }
! grep -q 'id="generate-from-palette-popup-overlay"' "$HTML" || { echo "FATAL: GenerateFromPalette popup should be removed"; exit 1; }
! grep -q 'id="autolevel-auto-gamma"' "$HTML" || { echo "FATAL: autolevel auto-gamma should not expose a fake mode selector"; exit 1; }

node - "$HTML" <<'NODE'
const fs = require('fs');

const htmlPath = process.argv[2];
const src = fs.readFileSync(htmlPath, 'utf8');

function fail(message) {
  console.error('FATAL: ' + message);
  process.exit(1);
}

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
assertIncludes("<div class=\"color-title\">Solve Score Palette</div>", 'render tab should expose a static solve-score palette section instead of a fake mode selector');
assertNotIncludes("class=\"color-dot active\" data-mode=\"solve_score\"", 'render tab should not expose a fake solve-score mode toggle');
assertIncludes("id=\"view-row-explicit\"", 'render tab should expose explicit viewport row');
assertIncludes("id=\"render-min-re\"", 'render tab should expose explicit min_re input');
assertIncludes("id=\"render-max-re\"", 'render tab should expose explicit max_re input');
assertIncludes("id=\"render-min-im\"", 'render tab should expose explicit min_im input');
assertIncludes("id=\"render-max-im\"", 'render tab should expose explicit max_im input');
assertIncludes("World aspect Δre:Δim", 'render explicit viewport should show world-aspect helper text');
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
assertIncludes("await _launchFusedRenderOrchestrator({", 'runRasterPipeline should dispatch through fused launcher');
assertIncludes("raster_workers: 10,", 'runRasterPipeline should default fused raster workers to 10');
assertIncludes("raster_section_mode: 'logical_sections_auto',", 'runRasterPipeline should default fused logical sections');
assertIncludes("async function runRasterPipelineMT(threadConfig = null) {\n    return runFusedRasterPipelineMT(threadConfig);\n}", 'runRasterPipelineMT should be fused-only wrapper');
const nonColorSection = src.split("async function _launchNonColorRenderOrchestrator(mode, paramsPatch = null) {")[1]?.split("async function _launchFusedRenderOrchestrator")[0] || '';
assertSectionNotIncludes(nonColorSection, "color_mode: 'solve_score'", 'non-color launcher should not send color mode');
assertSectionNotIncludes(nonColorSection, "color_pipeline: 'fused'", 'non-color launcher should not send color pipeline');
assertSectionNotIncludes(nonColorSection, "solve_metric:", 'non-color launcher should not send solve-score metric');
assertSectionNotIncludes(nonColorSection, "solve_score_chain:", 'non-color launcher should not send solve-score chain');
assertSectionNotIncludes(nonColorSection, "palette:", 'non-color launcher should not send palette');
assertSectionNotIncludes(nonColorSection, "save_associated_palette: false", 'non-color launcher should not send color-only associated palette flag');
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
assertIncludes("function _sourceColorArtifactIdForRenderArtifact(art) {", 'render artifact source-color helper missing');
assertIncludes("function _renderArtifactSolveDisplay(art) {", 'render artifact solve-display helper missing');
assertIncludes("_solveScoreProgramRememberedNames[prefix] = '';", 'populate should clear stale solve-score remembered names');
assertIncludes("_setSolveScoreProgramStatus(prefix, `Populated from ${statusTarget}`, false);", 'populate should overwrite stale solve-score status with the resolved source label');
assertIncludes("Color summaries show the solve display, palette name, and source Color artifact id.", 'render tab copy should match the new color summary contract');
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
assertIncludes("lambdaPost('storage', {}, '/list-solve-score-programs')", 'solve-score modal should list saved programs through storage');
assertIncludes("lambdaPost('storage', { id }, '/fetch-solve-score-program')", 'solve-score modal should fetch saved programs through storage');
assertIncludes("lambdaPost('storage', { name: payload.name, chain: payload.chain }, '/save-solve-score-program')", 'solve-score modal should save current program through storage');
assertIncludes("lambdaPost('storage', { id }, '/delete-solve-score-program')", 'solve-score modal should delete saved programs through storage');
assertNotIncludes("fetch('solve-score-programs/index.json'", 'frontend should not depend on repo-backed solve-score preset catalog');
assertNotIncludes("function loadSolveScoreProgramPreset(", 'old solve-score preset loader should be removed');
assertNotIncludes("function saveSolveScoreProgram(", 'old solve-score download helper should be removed');

console.log('Frontend fused render source checks: OK');
NODE

node - "$HTML" <<'NODE'
const fs = require('fs');
const vm = require('vm');

const htmlPath = process.argv[2];
const src = fs.readFileSync(htmlPath, 'utf8');

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

function assert(cond, message) {
  if (!cond) fail(message);
}

async function main() {
  const code = [
    extractFunction('_linkedColorIdForPaletteArtifact'),
    extractFunction('_sourceColorArtifactIdForRenderArtifact'),
    extractFunction('_noteSolveScorePopulate'),
    extractFunction('setColorMode'),
    extractFunction('_launchRenderOrchestrator'),
    extractFunction('runRasterPipeline'),
  ].join('\n\n');

  const renderStatus = { textContent: '', className: '' };
  const generateBtn = { disabled: false };
  const ctx = {
    console,
    Math,
    JSON,
    renderColorMode: 'rainbow',
    _renderGeneratePopupState: { saveAssociatedPalette: false },
    _solveScoreProgramRememberedNames: { render: 'pal5', palette: 'keep' },
    _statusCalls: [],
    _logs: [],
    _fusedCalls: [],
    _nonColorCalls: [],
    _updateSolveScoreButtonsCalls: 0,
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
        if (id === 'render-status') return renderStatus;
        return null;
      },
    },
  };
  ctx._updateSolveScoreButtons = () => {
    ctx._updateSolveScoreButtonsCalls += 1;
  };
  ctx._setSolveScoreProgramStatus = (prefix, message, isError) => {
    ctx._statusCalls.push({ prefix, message, isError });
  };
  ctx.log = (message, level, target) => {
    ctx._logs.push({ message, level, target });
  };

  vm.createContext(ctx);
  vm.runInContext(code, ctx);

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

  ctx.renderColorMode = 'solve_score';
  ctx._renderGeneratePopupState.saveAssociatedPalette = true;
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

echo "=== Frontend fused render source test passed ==="
