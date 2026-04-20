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
! grep -q 'id="btn-render-generate-from-palette"' "$HTML" || { echo "FATAL: GenerateFromPalette action should be removed"; exit 1; }
! grep -q 'id="generate-from-palette-popup-overlay"' "$HTML" || { echo "FATAL: GenerateFromPalette popup should be removed"; exit 1; }

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

assertIncludes("function setColorMode(mode) {\n    renderColorMode = 'solve_score';", 'setColorMode should hard-lock solve_score');
assertIncludes("constantColor: 'ffffff',", '_renderCommonParams should not read removed constant-color DOM');
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
assertNotIncludes("art.color_mode === 'saved_palette'", 'saved_palette compatibility should be removed from color UI');
assertIncludes("const phasePerfRows = subResults.length ? subResults : [rd];", 'render observer should use top-level phase rows when no subtasks exist');
assertIncludes("function _aggregateColorToBilevelPerf(results) {", 'Color2Bilevel perf aggregation missing');
assertIncludes("phase === 'bilevel_from_raw_render'", 'Color2Bilevel render phase summary/progress handling missing');
assertIncludes("phase: 'from_raw_color',", 'Color2Bilevel dispatch payload must include from_raw_color phase');
assertIncludes("function _renderRunErrorLabel(run) {", 'render error labeling helper missing');
assertIncludes("const errorLabel = _renderRunErrorLabel(run);", 'render observer should use mode-specific error labels');
assertIncludes("statusEl.textContent = errorLabel + ' error: ' + msg;", 'render observer should show mode-specific error text');
assertIncludes("log(errorLabel + ' failed: ' + msg, 'err', 'render-log');", 'render observer should log mode-specific failure text');
assertIncludes("if (rd.artifact_id) parts.push(`artifact=${rd.artifact_id}`);", 'error context should include artifact id');
assertIncludes("if (rd.source_artifact_id) parts.push(`source_artifact=${rd.source_artifact_id}`);", 'error context should include source artifact id');
assertIncludes("if (rd.threshold !== undefined && rd.threshold !== null && rd.threshold !== '') parts.push(`threshold=${rd.threshold}`);", 'error context should include threshold');
assertIncludes("if (rd.threshold_raw !== undefined && rd.threshold_raw !== null && rd.threshold_raw !== '') parts.push(`threshold_raw=${rd.threshold_raw}`);", 'error context should include raw threshold');

console.log('Frontend fused render source checks: OK');
NODE

echo "=== Frontend fused render source test passed ==="
