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
assertIncludes("<div class=\"color-title\" style=\"margin-top:10px\">Color</div>", 'render tab should merge color controls into the visual View/Rotation region');
assertIncludes("class=\"color-mode-choice\"><input type=\"radio\" name=\"render-color-interpretation\" value=\"scalar_lut\"", 'Scalar LUT selector should use aligned color-mode-choice markup');
assertIncludes(".color-mode-choice {\n    display: grid;", 'color mode choices should use fixed selector/text grid alignment');
assertIncludes(".color-mode-choice input[type=\"radio\"] {\n    margin: 0;", 'color mode radio inputs should override browser/default label spacing');
assertIncludes(".color-mode-name {\n    display: block;\n    line-height: 13px;", 'color mode names should use explicit line-height to avoid low text alignment');
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
assertIncludes("id=\"render-score-normalization\"", 'render tab should expose score normalization checkbox');
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
assertIncludes("function _artifactColorInterpretation(art) {", 'render populate should define artifact color interpretation resolver');
assertIncludes("_setRenderColorInterpretation(_artifactColorInterpretation(art));", 'render populate should restore selected artifact color interpretation');
assertIncludes("mode:${_colorInterpretationLabel(colorInterpretation)}", 'render artifact summaries should disclose color interpretation');
assertIncludes("id=\"ss-insert-before-btn\"", 'solve-score editor should expose insert-before button');
assertIncludes("id=\"ss-insert-after-btn\"", 'solve-score editor should expose insert-after button');
assertNotIncludes("id=\"ss-direct-rgb-preset\"", 'solve-score editor should not expose Direct RGB preset; saved programs cover this');
assertIncludes("score normalization: lo=${fmt(s.score_output_clip_lo)}  hi=${fmt(s.score_output_clip_hi)}", 'histogram output should report score normalization range');
assertIncludes("if (s.raw_hist_space === 'score_output_normalized') rawLabel = 'score-output normalized program output';", 'histogram raw bins should label score-output normalized space');
assertIncludes("'mean_log_mod',", 'solve-score catalog should expose mean_log_mod');
assertIncludes("'angular_entropy_16',", 'solve-score catalog should expose angular_entropy_16');
assertIncludes("'sector_max_share_16',", 'solve-score catalog should expose sector_max_share_16');
assertIncludes("'angular_order_4',", 'solve-score catalog should expose angular_order_4');
assertIncludes("const _solveScoreGenericMetricPublicName = 'metric';", 'solve-score editor should preserve public generic metric chip name');
assertIncludes("const _solveScoreGenericMetricChipName = '__metric';", 'solve-score editor should keep generic metric as an internal macro chip');
assertIncludes("catalog[_solveScoreGenericMetricChipName] = {", 'solve-score catalog should expose generic metric chip');
assertIncludes("return [_solveScoreGenericMetricPublicName, ...(item.params || [])];", 'generic metric chip should serialize publicly without desugaring in saved programs');
assertIncludes("id=\"render-preview-pix\" value=\"256\"", 'render output should expose default 256px lores preview size input');
assertIncludes("id=\"btn-render-lores-preview\" onclick=\"runRenderLoresPreview()\"", 'render output should expose lores preview button');
assertIncludes("id=\"render-lores-preview-stage\"", 'render output preview should expose a marquee stage wrapper');
assertIncludes("id=\"render-lores-preview-marquee\"", 'render output preview should expose a marquee overlay');
assertIncludes("id=\"render-lores-preview-tab-plot\"", 'render output preview should expose plot tab');
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
assertIncludes("choices: ['raw', 'norm', 'none']", 'emit chip mode dropdown should expose none mode');
assertIncludes("flush: { arity: 0, params: [], tooltip: 'clear the entire score stack' }", 'solve-score editor should expose flush chip');
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
assertIncludes("ColorRender-MT exposes fused clip/raster/finalize controls only", 'render tab copy should describe the ColorRender-MT action');
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
    "const _solveScoreMetricNames = [",
    "const _ctAndyParam = { ph: 'andy', def: '0' };",
    'solve-score catalog block'
  );
  const code = [
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
    extractFunction('_linkedColorIdForPaletteArtifact'),
    extractFunction('_sourceColorArtifactIdForRenderArtifact'),
    extractFunction('_noteSolveScorePopulate'),
    extractFunction('setColorMode'),
    extractFunction('_normalizeColorInterpretation'),
    extractFunction('_colorInterpretationLabel'),
    extractFunction('_artifactColorInterpretation'),
    extractFunction('_solveScoreColorCompatibility'),
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
    _renderMtPopupState: { saveAssociatedPalette: false },
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
        if (id === 'btn-render-generate-mt') return generateBtn;
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

echo "=== Frontend fused render source test passed ==="
