// PolyPaint 05-render-popups — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Deploy rewrites the script tags to build-versioned asset
// keys (assets/<BUILD_ID>/...), so a deploy flips atomically via index.html.
function _defaultColorRepaletteInterpretation(art) {
    const mode = _artifactColorInterpretation(art);
    return (mode === 'hsv' || mode === 'hsv_lut') ? 'hsv_lut' : 'rgb_lut';
}

function _selectedColorRepaletteInterpretation(art) {
    const channels = _artifactOutputChannelCount(art);
    if (channels !== 3) return '';
    const el = document.getElementById('color-repalette-interpretation');
    const selected = _normalizeColorInterpretation(el ? el.value : _colorRepalettePopupState.interpretation);
    return selected === 'hsv_lut' ? 'hsv_lut' : 'rgb_lut';
}

function _findColorArtifactById(artifactId) {
    return (_renderArtifacts.color || []).find(art => art && art.artifact_id === artifactId) || null;
}

function _artifactOutputChannelCount(art) {
    if (!art) return 1;
    const raw = art.raw_channels ?? art.score_output_channel_count ?? art.output_channel_count;
    const parsed = Number(raw);
    if (Number.isFinite(parsed) && parsed > 0) return Math.floor(parsed);
    return _artifactColorInterpretation(art) === 'scalar_lut' ? 1 : 3;
}

function _isScalarExtractPaletteSource(art) {
    return !!(
        art &&
        art.color_mode === 'solve_score' &&
        _artifactOutputChannelCount(art) === 1 &&
        _artifactColorInterpretation(art) === 'scalar_lut'
    );
}

function _hasFusedStepScorePaletteSource(art) {
    if (!art || art.color_mode !== 'solve_score') return false;
    const channels = _artifactOutputChannelCount(art);
    return !!(
        (channels === 1 || channels === 3) &&
        art.step_scores_key &&
        art.raw_key &&
        art.raw_meta_key
    );
}

function _canExtractPaletteArtifact(art) {
    if (!art || !art.artifact_id) return false;
    let current = art;
    const seen = new Set();
    while (current && current.artifact_id && !seen.has(current.artifact_id)) {
        seen.add(current.artifact_id);
        if (current.associated_palette_id) return true;
        if (current.color_mode === 'saved_palette' && current.palette_source_id) return true;
        if (current.color_mode === 'solve_score') {
            return _hasFusedStepScorePaletteSource(current) || _isScalarExtractPaletteSource(current);
        }
        const parentId = String(current.derived_from_artifact_id || '').trim();
        if (!parentId) break;
        current = _findColorArtifactById(parentId);
    }
    return false;
}

function _pdfColorSourceArtifacts() {
    return (_renderArtifacts.color || []).filter(art =>
        !!(art && art.artifact_id && art.image_key && (art.content_type || '').indexOf('image/') === 0)
    );
}

function _closeRepalettePopup() {
    _closeBuiltinPalettePopup();
    _closeTriPalettePopup();
    _closeLongPalettePopup();
    _repalettePopupState = { open: false, sourcePaletteId: '', sourceDisplayName: '' };
    const overlay = document.getElementById('repalette-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _renderRepalettePopup() {
    const overlay = document.getElementById('repalette-popup-overlay');
    const summaryEl = document.getElementById('repalette-popup-summary');
    const runBtn = document.getElementById('repalette-popup-run');
    const art = _renderSelectedArtifactEntry();
    if (!overlay || !summaryEl || !_repalettePopupState.open || !art || !_canRepaletteArtifact(art)) {
        _closeRepalettePopup();
        return;
    }
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    _renderPaletteRow('repalette');
    const sourceName = art.display_name || art.artifact_id || art.palette_id;
    const reuseMsg = art.render_reusable ? 'Reusable all-pass data will be copied.' : 'Legacy pass-0 data will be copied.';
    summaryEl.textContent = `Source: ${sourceName} • current palette: ${art.palette || '?'} • ${reuseMsg}`;
    if (runBtn) runBtn.disabled = !_currentPaletteForMode('repalette');
}

function openRepalettePopup() {
    const art = _renderSelectedArtifactEntry();
    if (!_canRepaletteArtifact(art)) return;
    _syncBuiltinDefaults();
    _syncTriDefaults();
    _syncLongDefaults();
    if (art.palette) setPaletteForMode('repalette', art.palette);
    _repalettePopupState = {
        open: true,
        sourcePaletteId: art.palette_id || art.artifact_id || '',
        sourceDisplayName: art.display_name || art.artifact_id || art.palette_id || '',
    };
    _renderRepalettePopup();
}

async function runRepaletteSelectedArtifact() {
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    if (!jobId || !_canRepaletteArtifact(art) || _activeRenderRun) return;
    const btn = document.getElementById('repalette-popup-run');
    const orig = btn ? btn.textContent : 'Execute';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Executing...'; }
        const runId = _generateRunId();
        const taskId = 'repalette_' + runId;
        const paletteName = _currentPaletteForMode('repalette') || art.palette || 'inferno';
        log(`RePalette: dispatching ${art.palette_id} -> ${paletteName}...`, '', 'render-log');
        const dispResult = await lambdaPost('dispatch', {
            target: 'repalette',
            jobs: [{
                job_id: jobId,
                task_id: taskId,
                source_palette_id: art.palette_id,
                new_palette: paletteName,
            }],
            expected_keys: [],
        });
        if ((dispResult.fired || 0) !== 1) throw new Error('RePalette dispatch failed');
        _saveActiveRun({
            job_id: jobId,
            mode: 'repalette',
            run_id: runId,
            task_id: taskId,
            started_at_ms: Date.now(),
        });
        document.getElementById('render-status').textContent = 'RePalette queued...';
        document.getElementById('render-status').className = 'status';
        _closeRepalettePopup();
        startActiveRenderObserver();
    } catch (e) {
        log('RePalette failed: ' + e.message, 'err', 'render-log');
        const statusEl = document.getElementById('render-status');
        if (statusEl) {
            statusEl.textContent = 'RePalette error: ' + e.message;
            statusEl.className = 'status error';
        }
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

function _closeColorRepalettePopup() {
    _closeBuiltinPalettePopup();
    _closeTriPalettePopup();
    _closeLongPalettePopup();
    _colorRepalettePopupState = { open: false, sourceArtifactId: '', sourceDisplayName: '', interpretation: '' };
    const overlay = document.getElementById('color-repalette-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _renderColorRepalettePopup() {
    const overlay = document.getElementById('color-repalette-popup-overlay');
    const summaryEl = document.getElementById('color-repalette-popup-summary');
    const interpretationEl = document.getElementById('color-repalette-interpretation-row');
    const runBtn = document.getElementById('color-repalette-popup-run');
    const art = _renderSelectedArtifactEntry();
    if (!overlay || !summaryEl || !_colorRepalettePopupState.open || !art || !_canColorRepaletteArtifact(art)) {
        _closeColorRepalettePopup();
        return;
    }
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    _renderPaletteRow('color_repalette');
    const sourceName = art.artifact_id || 'selected color artifact';
    const outputPalette = _currentPaletteForMode('color_repalette') || art.palette || '?';
    const channels = _artifactOutputChannelCount(art);
    const sourceMode = _artifactColorInterpretation(art);
    let modeLabel = channels === 3 ? `${channels}-byte ${_colorInterpretationLabel(sourceMode)} raw reused` : 'scalar raw reused';
    if (channels === 3) {
        const selected = _selectedColorRepaletteInterpretation(art);
        if (interpretationEl) {
            interpretationEl.style.display = 'block';
            interpretationEl.innerHTML = `
                <div style="display:grid; grid-template-columns:120px minmax(0,1fr); gap:10px; align-items:center">
                    <label for="color-repalette-interpretation" style="margin:0; color:#888; text-align:right">Output LUT</label>
                    <select id="color-repalette-interpretation" style="max-width:220px">
                        <option value="rgb_lut"${selected === 'rgb_lut' ? ' selected' : ''}>RGB LUT</option>
                        <option value="hsv_lut"${selected === 'hsv_lut' ? ' selected' : ''}>HSV LUT</option>
                    </select>
                </div>`;
            const selectEl = document.getElementById('color-repalette-interpretation');
            if (selectEl) selectEl.addEventListener('change', () => {
                _colorRepalettePopupState.interpretation = _selectedColorRepaletteInterpretation(art);
                _renderColorRepalettePopup();
            });
        }
        modeLabel += ` → ${_colorInterpretationLabel(selected)}`;
    } else if (interpretationEl) {
        interpretationEl.style.display = 'none';
        interpretationEl.innerHTML = '';
    }
    summaryEl.textContent = `Source: ${sourceName} • current palette=${art.palette || '?'} • ${modeLabel} • output colorvector=${outputPalette}`;
    if (runBtn) runBtn.disabled = !_currentPaletteForMode('color_repalette');
}

function openColorRepalettePopup() {
    const art = _renderSelectedArtifactEntry();
    if (!_canColorRepaletteArtifact(art)) return;
    _syncBuiltinDefaults();
    _syncTriDefaults();
    _syncLongDefaults();
    if (art.palette) setPaletteForMode('color_repalette', art.palette);
    _colorRepalettePopupState = {
        open: true,
        sourceArtifactId: art.artifact_id || '',
        sourceDisplayName: art.artifact_id || '',
        interpretation: _artifactOutputChannelCount(art) === 3 ? _defaultColorRepaletteInterpretation(art) : '',
    };
    _renderColorRepalettePopup();
}

async function runColorRepaletteSelectedArtifact() {
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    if (!jobId || !_canColorRepaletteArtifact(art) || _activeRenderRun) return;
    const btn = document.getElementById('color-repalette-popup-run');
    const orig = btn ? btn.textContent : 'Execute';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Executing...'; }
        const runId = _generateRunId();
        const taskId = 'color_repalette_' + runId;
        const artifactId = 'color_repalette_' + Date.now() + '_' + Math.random().toString(36).slice(2, 8);
        const paletteName = _currentPaletteForMode('color_repalette') || art.palette || 'inferno';
        const channels = _artifactOutputChannelCount(art);
        const newInterpretation = _selectedColorRepaletteInterpretation(art);
        const modeLabel = channels === 3 ? ` ${_colorInterpretationLabel(newInterpretation)}` : '';
        log(`Color RePalette: dispatching ${art.artifact_id} -> ${paletteName}${modeLabel}...`, 'ok', 'render-log');
        const dispResult = await lambdaPost('dispatch', {
            target: 'color_repalette',
            jobs: [{
                job_id: jobId,
                task_id: taskId,
                artifact_id: artifactId,
                source_artifact_id: art.artifact_id,
                source_image_key: art.image_key,
                new_palette: paletteName,
                ...(channels === 3 ? { new_interpretation: newInterpretation } : {}),
            }],
            expected_keys: [],
        });
        if ((dispResult.fired || 0) !== 1) throw new Error('Color RePalette dispatch failed');
        _saveActiveRun({
            job_id: jobId,
            mode: 'color_repalette',
            run_id: runId,
            task_id: taskId,
            started_at_ms: Date.now(),
        });
        document.getElementById('render-status').textContent = 'Color RePalette queued...';
        document.getElementById('render-status').className = 'status';
        _closeColorRepalettePopup();
        startActiveRenderObserver();
    } catch (e) {
        log('Color RePalette failed: ' + e.message, 'err', 'render-log');
        const statusEl = document.getElementById('render-status');
        if (statusEl) {
            statusEl.textContent = 'Color RePalette error: ' + e.message;
            statusEl.className = 'status error';
        }
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

function _closeColorToBilevelPopup() {
    _colorToBilevelPopupState = {
        open: false,
        sourceArtifactId: '',
        threshold: Math.max(0, Math.min(255, Math.round(Number(_colorToBilevelPopupState.threshold) || 0))),
    };
    const overlay = document.getElementById('color-to-bilevel-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _renderColorToBilevelPopup() {
    const overlay = document.getElementById('color-to-bilevel-popup-overlay');
    const summaryEl = document.getElementById('color-to-bilevel-popup-summary');
    const thresholdEl = document.getElementById('color-to-bilevel-threshold');
    const art = _renderSelectedArtifactEntry();
    if (!overlay || !summaryEl || !thresholdEl || !_colorToBilevelPopupState.open || !art || !_hasColorRawSidecar(art)) {
        _closeColorToBilevelPopup();
        return;
    }
    const threshold = Math.max(0, Math.min(255, Math.round(Number(_colorToBilevelPopupState.threshold) || 0)));
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    thresholdEl.value = String(threshold);
    summaryEl.textContent = `Source: ${art.artifact_id || 'selected color artifact'} • palette=${art.palette || '?'} • raw sidecar reused • white if raw > ${threshold}`;
}

function openColorToBilevelPopup() {
    const art = _renderSelectedArtifactEntry();
    if (_renderActiveFamily !== 'color' || !_hasColorRawSidecar(art)) return;
    _colorToBilevelPopupState = {
        open: true,
        sourceArtifactId: art.artifact_id || '',
        threshold: Math.max(0, Math.min(255, Math.round(Number(_colorToBilevelPopupState.threshold) || 0))),
    };
    _renderColorToBilevelPopup();
    const thresholdEl = document.getElementById('color-to-bilevel-threshold');
    if (thresholdEl && typeof thresholdEl.focus === 'function') thresholdEl.focus();
}

async function runColorToBilevelSelectedArtifact() {
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    if (_renderActiveFamily !== 'color' || !jobId || !_hasColorRawSidecar(art) || _activeRenderRun) return;
    const btn = document.getElementById('color-to-bilevel-popup-run');
    const statusEl = document.getElementById('render-status');
    const thresholdEl = document.getElementById('color-to-bilevel-threshold');
    const orig = btn ? btn.textContent : 'Execute';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Executing...'; }
        const threshold = Math.max(0, Math.min(255, Math.round(Number(thresholdEl && thresholdEl.value) || 0)));
        _colorToBilevelPopupState.threshold = threshold;
        const runId = _generateRunId();
        const taskId = 'color_to_bilevel_' + runId;
        const artifactId = 'color_to_bilevel_' + Date.now() + '_' + Math.random().toString(36).slice(2, 8);
        log(`Color2Bilevel: dispatching ${art.artifact_id} with threshold=${threshold}...`, 'ok', 'render-log');
        const dispResult = await lambdaPost('dispatch', {
            target: 'color_to_bilevel',
            jobs: [{
                phase: 'from_raw_color',
                job_id: jobId,
                task_id: taskId,
                artifact_id: artifactId,
                source_artifact_id: art.artifact_id,
                threshold,
            }],
            expected_keys: [],
        });
        if ((dispResult.fired || 0) !== 1) throw new Error('Color2Bilevel dispatch failed');
        _saveActiveRun({
            job_id: jobId,
            mode: 'color_to_bilevel',
            run_id: runId,
            task_id: taskId,
            started_at_ms: Date.now(),
        });
        if (statusEl) {
            statusEl.textContent = 'Color2Bilevel queued...';
            statusEl.className = 'status';
        }
        _closeColorToBilevelPopup();
        startActiveRenderObserver();
    } catch (e) {
        if (statusEl) {
            statusEl.textContent = 'Color2Bilevel error: ' + e.message;
            statusEl.className = 'status error';
        }
        log('Color2Bilevel failed: ' + e.message, 'err', 'render-log');
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

async function runExtractPaletteArtifact(config = null) {
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    const btn = document.getElementById('btn-render-extract-palette');
    const statusEl = document.getElementById('render-status');
    const paletteStatusEl = document.getElementById('palette-status');
    if (!jobId) return;
    if (!_canExtractPaletteArtifact(art)) {
        const lineage = _extractPaletteLineageHint(art);
        const msg = `ExtractPalette unavailable: ${lineage.label}`;
        if (statusEl) {
            statusEl.textContent = msg;
            statusEl.className = 'status error';
        }
        if (paletteStatusEl) {
            paletteStatusEl.textContent = msg;
            paletteStatusEl.className = 'status error';
        }
        log(msg, 'err', 'render-log');
        log(msg, 'err', 'palette-log');
        return;
    }
    const lineage = _extractPaletteLineageHint(art);
    if (_activeRenderRun || _activePaletteRun || _loadActivePaletteRun()) {
        log('ExtractPalette: another render or palette run is already in progress', 'err', 'render-log');
        return;
    }
    const orig = btn ? btn.textContent : 'ExtractPalette';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Extracting...'; }
        if (statusEl) {
            statusEl.textContent = 'ExtractPalette: dispatching...';
            statusEl.className = 'status';
        }
        if (paletteStatusEl) {
            paletteStatusEl.textContent = 'ExtractPalette: dispatching...';
            paletteStatusEl.className = 'status';
        }
        const solveScoreThreads = _clampRenderMtThreads(
            config && typeof config === 'object' && config.solveScoreThreads != null
                ? config.solveScoreThreads
                : _extractPalettePopupState.solveScoreThreads
        );
        const histInputMode = _normalizeSolveScoreHistInputMode(
            config && typeof config === 'object' && config.histInputMode != null
                ? config.histInputMode
                : _extractPalettePopupState.histInputMode
        );
        const histRetries = _clampRenderMtRetries(
            config && typeof config === 'object' && config.histRetries != null
                ? config.histRetries
                : _extractPalettePopupState.histRetries
        );
        const mergeWorkers = _clampRenderMtMergeWorkers(
            config && typeof config === 'object' && config.mergeWorkers != null
                ? config.mergeWorkers
                : _extractPalettePopupState.mergeWorkers
        );
        const chunkThreads = _clampRenderMtThreads(
            config && typeof config === 'object' && config.chunkThreads != null
                ? config.chunkThreads
                : _extractPalettePopupState.chunkThreads
        );
        const chunkInputMode = _normalizeRasterInputMode(
            config && typeof config === 'object' && config.chunkInputMode != null
                ? config.chunkInputMode
                : _extractPalettePopupState.chunkInputMode
        );
        const chunkRetries = _clampRenderMtRetries(
            config && typeof config === 'object' && config.chunkRetries != null
                ? config.chunkRetries
                : _extractPalettePopupState.chunkRetries
        );
        const chunkWorkers = _clampRenderMtWorkerCount(
            config && typeof config === 'object' && config.chunkWorkers != null
                ? config.chunkWorkers
                : _extractPalettePopupState.chunkWorkers
        );
        _extractPalettePopupState.solveScoreThreads = solveScoreThreads;
        _extractPalettePopupState.histInputMode = histInputMode;
        _extractPalettePopupState.histRetries = histRetries;
        _extractPalettePopupState.mergeWorkers = mergeWorkers;
        _extractPalettePopupState.chunkThreads = chunkThreads;
        _extractPalettePopupState.chunkInputMode = chunkInputMode;
        _extractPalettePopupState.chunkRetries = chunkRetries;
        _extractPalettePopupState.chunkWorkers = chunkWorkers;
        const dispatchMsg = lineage.kind === 'solve_score'
            ? `ExtractPalette-MT: dispatching with hist threads=${solveScoreThreads}, hist input=${histInputMode}, hist retries=${histRetries}, merge workers=${mergeWorkers}, chunk threads=${chunkThreads}, chunk input=${chunkInputMode}, chunk retries=${chunkRetries}, chunk workers=${chunkWorkers}...`
            : `ExtractPalette-fused: dispatching ${art.artifact_id} with cached palette lineage...`;
        log(dispatchMsg, '', 'render-log');
        log(dispatchMsg, '', 'palette-log');

        const runId = _generateRunId();
        const taskId = 'extract_palette_run_' + runId;
        const dispatchBody = lineage.kind === 'solve_score'
            ? {
                target: 'palette_orchestrator',
                jobs: [{
                    job_id: jobId,
                    run_id: runId,
                    task_id: taskId,
                    artifact_id: art.artifact_id,
                    params: {
                        solve_score_threads: solveScoreThreads,
                        solve_score_hist_input_mode: histInputMode,
                        solve_score_hist_retries: histRetries,
                        solve_score_merge_workers: mergeWorkers,
                        palette_chunk_threads: chunkThreads,
                        palette_chunk_input_mode: chunkInputMode,
                        palette_chunk_retries: chunkRetries,
                        palette_chunk_workers: chunkWorkers,
                    },
                }],
                expected_keys: [],
            }
            : {
                target: 'extract_palette_fused',
                jobs: [{
                    job_id: jobId,
                    run_id: runId,
                    task_id: taskId,
                    artifact_id: art.artifact_id,
                }],
                expected_keys: [],
            };
        const dispResult = await lambdaPost('dispatch', dispatchBody);
        if ((dispResult.fired || 0) !== 1) throw new Error('ExtractPalette dispatch failed');

        const paletteJobEl = document.getElementById('palette-results-dir');
        if (paletteJobEl) paletteJobEl.value = jobId;
        _saveActivePaletteRun({
            job_id: jobId,
            run_id: runId,
            task_id: taskId,
            started_at_ms: Date.now(),
            mode: 'extract_palette',
            origin: 'render_extract_palette',
            source_artifact_id: art.artifact_id,
        });
        log('  orchestrator dispatched: ' + runId, 'ok', 'render-log');
        log('  orchestrator dispatched: ' + runId, 'ok', 'palette-log');
        startActivePaletteObserver();
    } catch (e) {
        if (statusEl) {
            statusEl.textContent = 'ExtractPalette failed';
            statusEl.className = 'status error';
        }
        if (paletteStatusEl) {
            paletteStatusEl.textContent = 'ExtractPalette failed';
            paletteStatusEl.className = 'status error';
        }
        log('ExtractPalette failed: ' + e.message, 'err', 'render-log');
        log('ExtractPalette failed: ' + e.message, 'err', 'palette-log');
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

function _extractPaletteLineageHint(art) {
    if (!art || !art.artifact_id) return { kind: 'none', label: 'No Color artifact selected.' };
    let current = art;
    const seen = new Set();
    while (current && current.artifact_id && !seen.has(current.artifact_id)) {
        seen.add(current.artifact_id);
        if (current.associated_palette_id) {
            return current === art
                ? { kind: 'done', label: `Artifact ${art.artifact_id} already has an associated palette; extraction should be a no-op.` }
                : { kind: 'attach', label: `Artifact ${art.artifact_id} inherits an associated palette from ${current.artifact_id}; extraction should attach that palette without regeneration.` };
        }
        if (current.color_mode === 'saved_palette' && current.palette_source_id) {
            return current === art
                ? { kind: 'attach', label: `Artifact ${art.artifact_id} uses a saved palette; extraction should attach that dependency as its associated palette.` }
                : { kind: 'attach', label: `Artifact ${art.artifact_id} resolves to saved-palette ancestor ${current.artifact_id}; extraction should attach that dependency as its associated palette.` };
        }
        if (current.color_mode === 'solve_score') {
            if (_hasFusedStepScorePaletteSource(current)) {
                const count = _artifactOutputChannelCount(current);
                const mode = _colorInterpretationLabel(_artifactColorInterpretation(current));
                return current === art
                    ? { kind: 'fused', label: `Artifact ${art.artifact_id} carries ${count}-channel step_scores.raw metadata (${mode}); extraction will regenerate or refresh the associated palette from the fused raw contract.` }
                    : { kind: 'fused', label: `Artifact ${art.artifact_id} resolves to solve-score ancestor ${current.artifact_id} with ${count}-channel step_scores.raw metadata (${mode}); extraction will regenerate or refresh the associated palette from that fused raw contract.` };
            }
            if (!_isScalarExtractPaletteSource(current)) {
                const count = _artifactOutputChannelCount(current);
                const mode = _colorInterpretationLabel(_artifactColorInterpretation(current));
                return { kind: 'unsupported', label: `Artifact ${art.artifact_id} resolves to ${current.artifact_id}, which uses ${count} ${count === 1 ? 'output' : 'outputs'} (${mode}). ExtractPalette needs solve-order step_scores.raw or an existing associated palette artifact.` };
            }
            return current === art
                ? { kind: 'solve_score', label: `Artifact ${art.artifact_id} is a scalar solve-score render; extraction will reuse exact solve-score lineage and may rerun solve-score prepass if scratch is missing.` }
                : { kind: 'solve_score', label: `Artifact ${art.artifact_id} resolves to scalar solve-score ancestor ${current.artifact_id}; extraction will use that exact lineage and may rerun solve-score prepass if scratch is missing.` };
        }
        const parentId = String(current.derived_from_artifact_id || '').trim();
        if (!parentId) break;
        current = _findColorArtifactById(parentId);
    }
    return { kind: 'none', label: `Artifact ${art.artifact_id} does not expose reusable palette lineage.` };
}

function _closeExtractPalettePopup() {
    _extractPalettePopupState = {
        open: false,
        solveScoreThreads: _clampRenderMtThreads(_extractPalettePopupState.solveScoreThreads),
        histInputMode: _normalizeSolveScoreHistInputMode(_extractPalettePopupState.histInputMode),
        histRetries: _clampRenderMtRetries(_extractPalettePopupState.histRetries),
        mergeWorkers: _clampRenderMtMergeWorkers(_extractPalettePopupState.mergeWorkers),
        chunkThreads: _clampRenderMtThreads(_extractPalettePopupState.chunkThreads),
        chunkInputMode: _normalizeRasterInputMode(_extractPalettePopupState.chunkInputMode),
        chunkRetries: _clampRenderMtRetries(_extractPalettePopupState.chunkRetries),
        chunkWorkers: _clampRenderMtWorkerCount(_extractPalettePopupState.chunkWorkers),
    };
    const overlay = document.getElementById('extract-palette-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _renderExtractPalettePopup() {
    const overlay = document.getElementById('extract-palette-popup-overlay');
    const summaryEl = document.getElementById('extract-palette-popup-summary');
    const threadsEl = document.getElementById('extract-palette-solve-score-threads');
    const histInputModeEl = document.getElementById('extract-palette-hist-input-mode');
    const histRetriesEl = document.getElementById('extract-palette-hist-retries');
    const mergeWorkersEl = document.getElementById('extract-palette-merge-workers');
    const chunkThreadsEl = document.getElementById('extract-palette-chunk-threads');
    const chunkInputModeEl = document.getElementById('extract-palette-chunk-input-mode');
    const chunkRetriesEl = document.getElementById('extract-palette-chunk-retries');
    const chunkWorkersEl = document.getElementById('extract-palette-chunk-workers');
    const runBtn = document.getElementById('extract-palette-popup-run');
    if (!overlay || !summaryEl || !threadsEl || !histInputModeEl || !histRetriesEl || !mergeWorkersEl || !chunkThreadsEl || !chunkInputModeEl || !chunkRetriesEl || !chunkWorkersEl) return;
    if (!_extractPalettePopupState.open) {
        _closeExtractPalettePopup();
        return;
    }
    const art = _renderSelectedArtifactEntry();
    const lineage = _extractPaletteLineageHint(art);
    const runnable = _canExtractPaletteArtifact(art);
    const executionRelevant = lineage.kind === 'solve_score';
    const solveScoreThreads = _clampRenderMtThreads(_extractPalettePopupState.solveScoreThreads);
    const histInputMode = _normalizeSolveScoreHistInputMode(_extractPalettePopupState.histInputMode);
    const histRetries = _clampRenderMtRetries(_extractPalettePopupState.histRetries);
    const mergeWorkers = _clampRenderMtMergeWorkers(_extractPalettePopupState.mergeWorkers);
    const chunkThreads = _clampRenderMtThreads(_extractPalettePopupState.chunkThreads);
    const chunkInputMode = _normalizeRasterInputMode(_extractPalettePopupState.chunkInputMode);
    const chunkRetries = _clampRenderMtRetries(_extractPalettePopupState.chunkRetries);
    const chunkWorkers = _clampRenderMtWorkerCount(_extractPalettePopupState.chunkWorkers);
    _extractPalettePopupState.solveScoreThreads = solveScoreThreads;
    _extractPalettePopupState.histInputMode = histInputMode;
    _extractPalettePopupState.histRetries = histRetries;
    _extractPalettePopupState.mergeWorkers = mergeWorkers;
    _extractPalettePopupState.chunkThreads = chunkThreads;
    _extractPalettePopupState.chunkInputMode = chunkInputMode;
    _extractPalettePopupState.chunkRetries = chunkRetries;
    _extractPalettePopupState.chunkWorkers = chunkWorkers;
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    threadsEl.value = String(solveScoreThreads);
    histInputModeEl.value = histInputMode;
    histRetriesEl.value = String(histRetries);
    mergeWorkersEl.value = String(mergeWorkers);
    chunkThreadsEl.value = String(chunkThreads);
    chunkInputModeEl.value = chunkInputMode;
    chunkRetriesEl.value = String(chunkRetries);
    chunkWorkersEl.value = String(chunkWorkers);
    threadsEl.disabled = !executionRelevant;
    histInputModeEl.disabled = !executionRelevant;
    histRetriesEl.disabled = !executionRelevant || histInputMode !== 'sectioned';
    mergeWorkersEl.disabled = !executionRelevant;
    chunkThreadsEl.disabled = !executionRelevant;
    chunkInputModeEl.disabled = !executionRelevant;
    chunkRetriesEl.disabled = !executionRelevant || chunkInputMode !== 'sectioned';
    chunkWorkersEl.disabled = !executionRelevant;
    const artLabel = art && art.artifact_id ? art.artifact_id : '(none)';
    summaryEl.textContent = !runnable
        ? `Artifact: ${artLabel} • ${lineage.label}`
        : executionRelevant
        ? `Artifact: ${artLabel} • ${lineage.label} • hist threads=${solveScoreThreads} • hist input=${histInputMode} • hist retries=${histRetries} • merge workers=${mergeWorkers} • chunk threads=${chunkThreads} • chunk input=${chunkInputMode} • chunk retries=${chunkRetries} • chunk workers=${chunkWorkers}`
        : `Artifact: ${artLabel} • ${lineage.label} • solve-score and chunk execution controls are ignored for this fused or attach-only lineage.`;
    if (runBtn) {
        runBtn.disabled = !runnable || !!_activeRenderRun || !!_activePaletteRun || !!_loadActivePaletteRun();
        runBtn.title = runnable ? '' : lineage.label;
    }
}

function openExtractPalettePopup() {
    const art = _renderSelectedArtifactEntry();
    if (_activeRenderRun || _activePaletteRun || _loadActivePaletteRun() || !art || !art.artifact_id) return;
    _extractPalettePopupState = {
        open: true,
        solveScoreThreads: _clampRenderMtThreads(_extractPalettePopupState.solveScoreThreads || 4),
        histInputMode: _normalizeSolveScoreHistInputMode(_extractPalettePopupState.histInputMode || 'tmpfile'),
        histRetries: _clampRenderMtRetries(_extractPalettePopupState.histRetries ?? 2),
        mergeWorkers: _clampRenderMtMergeWorkers(_extractPalettePopupState.mergeWorkers || 16),
        chunkThreads: _clampRenderMtThreads(_extractPalettePopupState.chunkThreads || 4),
        chunkInputMode: _normalizeRasterInputMode(_extractPalettePopupState.chunkInputMode || 'sectioned'),
        chunkRetries: _clampRenderMtRetries(_extractPalettePopupState.chunkRetries ?? 2),
        chunkWorkers: _clampRenderMtWorkerCount(_extractPalettePopupState.chunkWorkers || 16),
    };
    _renderExtractPalettePopup();
    if (!_canExtractPaletteArtifact(art)) {
        const lineage = _extractPaletteLineageHint(art);
        const msg = `ExtractPalette unavailable: ${lineage.label}`;
        const statusEl = document.getElementById('render-status');
        if (statusEl) {
            statusEl.textContent = msg;
            statusEl.className = 'status error';
        }
        log(msg, 'err', 'render-log');
    }
    const threadsEl = document.getElementById('extract-palette-solve-score-threads');
    const runBtn = document.getElementById('extract-palette-popup-run');
    if (_canExtractPaletteArtifact(art) && threadsEl && typeof threadsEl.focus === 'function') threadsEl.focus();
    else if (runBtn && typeof runBtn.focus === 'function') runBtn.focus();
}

function _clampRenderMtThreads(value) {
    let n = parseInt(value, 10);
    if (!Number.isFinite(n)) n = 4;
    return Math.max(1, Math.min(16, n));
}

function _clampRenderMtWorkerCount(value) {
    let n = parseInt(value, 10);
    if (!Number.isFinite(n)) n = 16;
    return Math.max(1, Math.min(64, n));
}

function _clampRenderMtMergeWorkers(value) {
    return _clampRenderMtWorkerCount(value);
}

function _clampRenderMtFinalizeWorkers(value) {
    return _clampRenderMtWorkerCount(value);
}

function _clampRenderMtRetries(value) {
    let n = parseInt(value, 10);
    if (!Number.isFinite(n)) n = 2;
    return Math.max(0, Math.min(10, n));
}

function _normalizeSolveScoreHistInputMode(value) {
    const mode = String(value || 'tmpfile').trim().toLowerCase();
    if (mode === 'stdin') return 'stdin';
    if (mode === 'sectioned') return 'sectioned';
    return 'tmpfile';
}

function _normalizeRasterInputMode(value) {
    const mode = String(value || 'tmpfile').trim().toLowerCase();
    if (mode === 'sectioned') return 'sectioned';
    return 'tmpfile';
}

function _normalizeRenderMtSectionMode(value) {
    const mode = String(value || 'physical_chunks').trim().toLowerCase();
    if (mode === 'logical_sections' || mode === 'logical_sections_auto') return mode;
    return 'physical_chunks';
}

function _clampRenderMtSectionCount(value) {
    const raw = String(value ?? '').trim();
    if (!raw) return '';
    const n = parseInt(raw, 10);
    if (!Number.isFinite(n)) return '';
    return Math.max(1, Math.min(4096, n));
}

function _renderMtCurrentJobId() {
    return String(document.getElementById('render-results-dir')?.value || '').trim();
}

function _renderMtJobSizeData() {
    const jobId = _renderMtCurrentJobId();
    if (jobId && _renderLoadedJobId && _renderLoadedJobId !== jobId) return null;
    return (window._lastRenderSummary && window._lastRenderSummary.calc && window._lastRenderSummary.calc.job_size) || null;
}

function _renderMtSolveScoreCompiled() {
    if (renderColorMode !== 'solve_score') return null;
    try {
        return _compileSolveScoreChain(_chainForWhich('ss'), renderSolveMetric);
    } catch (_) {
        return null;
    }
}

function _renderMtSectionEstimate(kind, threads) {
    if (_renderMtPopupState.jobSizeLoadError) {
        return { available: false, reason: `summary load failed: ${_renderMtPopupState.jobSizeLoadError}` };
    }
    const jobId = _renderMtCurrentJobId();
    const jobSize = _renderMtJobSizeData();
    if (!jobId && !jobSize) {
        return { available: false, reason: 'no render job selected' };
    }
    if (jobId && _renderLoadedJobId && _renderLoadedJobId !== jobId) {
        return { available: false, reason: 'current job summary not loaded' };
    }
    if (!jobSize) {
        return { available: false, reason: 'calc job-size summary missing' };
    }
    if (!Number.isFinite(Number(jobSize.total_solves)) || Number(jobSize.total_solves) < 1) {
        return { available: false, reason: 'total solve count missing' };
    }
    if (!jobSize.chunk_step_metadata_complete) {
        return { available: false, reason: 'chunk step metadata missing' };
    }
    const compiled = _renderMtSolveScoreCompiled();
    const mixedScoreSources = renderColorMode === 'solve_score';
    const usesCoeff = mixedScoreSources && !!(compiled && compiled.uses_coeff_source);
    const usesParam = mixedScoreSources && !!(compiled && compiled.uses_param_source);
    const rootRowBytes = Number(jobSize.root_row_bytes) || 0;
    const coeffRowBytes = usesCoeff ? (Number(jobSize.coeff_row_bytes) || 0) : 0;
    const paramRowBytes = usesParam ? (Number(jobSize.param_row_bytes) || 0) : 0;
    const rowBytes = rootRowBytes + coeffRowBytes + paramRowBytes;
    if (!(rowBytes > 0)) {
        return { available: false, reason: 'row size unavailable' };
    }
    const memoryMb = kind === 'palette'
        ? (Number(jobSize.palette_chunk_memory_mb) || 0)
        : (kind === 'raster'
            ? (Number(jobSize.raster_memory_mb) || 0)
            : (Number(jobSize.solve_hist_memory_mb) || 0));
    const usableFraction = Number(jobSize.auto_usable_fraction) || 0.40;
    const fixedOverheadMb = Number(jobSize.auto_fixed_overhead_mb) || 96;
    const perThreadOverheadMb = Number(jobSize.auto_per_thread_overhead_mb) || 8;
    const budgetBytes = Math.max(
        0,
        Math.floor(memoryMb * 1024 * 1024 * usableFraction)
            - Math.floor(fixedOverheadMb * 1024 * 1024)
            - Math.max(1, Number(threads) || 1) * Math.floor(perThreadOverheadMb * 1024 * 1024)
    );
    if (!(budgetBytes > 0)) {
        return { available: false, reason: 'no usable memory budget', memoryMb, budgetBytes: 0 };
    }
    const totalSolves = Math.max(1, Number(jobSize.total_solves) || 1);
    const maxSolvesPerSection = Math.max(1, Math.floor(budgetBytes / rowBytes));
    const minSafeSections = Math.max(1, Math.min(4096, Math.ceil(totalSolves / maxSolvesPerSection)));
    const sectionSolveCount = Math.max(1, Math.ceil(totalSolves / minSafeSections));
    return {
        available: true,
        usesCoeff,
        usesParam,
        rowBytes,
        memoryMb,
        budgetBytes,
        maxSolvesPerSection,
        minSafeSections,
        estimatedSectionBytes: sectionSolveCount * rowBytes,
    };
}

function _renderMtSectionSourceLabel(estimate) {
    if (!estimate) return 'slv';
    const parts = ['slv'];
    if (estimate.usesCoeff) parts.push('cf');
    if (estimate.usesParam) parts.push('pm');
    return parts.join('+');
}

function _renderMtSectionModeLabel(mode) {
    if (mode === 'physical_chunks') return 'source-aligned';
    if (mode === 'logical_sections_auto') return 'auto split';
    if (mode === 'logical_sections') return 'manual split';
    return String(mode || '').replace(/_/g, ' ');
}

function _renderMtSectionSettingLabel(mode, count) {
    const base = _renderMtSectionModeLabel(mode);
    return count ? `${base}/${count}` : base;
}

function _closeRenderMtPopup() {
    _renderMtPopupState = {
        open: false,
        rasterThreads: _clampRenderMtThreads(_renderMtPopupState.rasterThreads),
        rasterWorkers: _clampRenderMtWorkerCount(_renderMtPopupState.rasterWorkers),
        solveScoreThreads: _clampRenderMtThreads(_renderMtPopupState.solveScoreThreads),
        rasterRetries: _clampRenderMtRetries(_renderMtPopupState.rasterRetries),
        finalizeWorkers: _clampRenderMtFinalizeWorkers(_renderMtPopupState.finalizeWorkers),
        rasterSectionMode: _normalizeRenderMtSectionMode(_renderMtPopupState.rasterSectionMode),
        rasterSectionCount: _clampRenderMtSectionCount(_renderMtPopupState.rasterSectionCount),
        saveAssociatedPalette: !!_renderMtPopupState.saveAssociatedPalette,
        jobSizeLoadError: '',
    };
    const overlay = document.getElementById('render-mt-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _renderRenderMtPopup() {
    const overlay = document.getElementById('render-mt-popup-overlay');
    const summaryEl = document.getElementById('render-mt-popup-summary');
    const jobSizeEl = document.getElementById('render-mt-job-size');
    const fusedReadoutEl = document.getElementById('render-mt-fused-readout');
    const fusedClipThreadsEl = document.getElementById('render-mt-fused-clip-threads');
    const fusedRasterThreadsEl = document.getElementById('render-mt-fused-raster-threads');
    const fusedRasterWorkersEl = document.getElementById('render-mt-fused-raster-workers');
    const fusedRasterRetriesEl = document.getElementById('render-mt-fused-raster-retries');
    const fusedFinalizeWorkersEl = document.getElementById('render-mt-fused-finalize-workers');
    const fusedRasterSectionModeEl = document.getElementById('render-mt-fused-raster-section-mode');
    const fusedRasterSectionCountEl = document.getElementById('render-mt-fused-raster-section-count');
    const fusedRasterSectionSummaryEl = document.getElementById('render-mt-fused-raster-section-summary');
    const fusedAssocRowEl = document.getElementById('render-mt-fused-associated-row');
    const fusedAssocEl = document.getElementById('render-mt-fused-save-associated-palette');
    const helpEl = document.getElementById('render-mt-popup-help');
    const pipelineBannerEl = document.getElementById('render-mt-pipeline-banner');
    const runBtn = document.getElementById('render-mt-popup-run');
    if (!overlay || !summaryEl || !jobSizeEl || !fusedReadoutEl || !fusedClipThreadsEl || !fusedRasterThreadsEl || !fusedRasterWorkersEl || !fusedRasterRetriesEl || !fusedFinalizeWorkersEl || !fusedRasterSectionModeEl || !fusedRasterSectionCountEl || !fusedRasterSectionSummaryEl || !fusedAssocRowEl || !fusedAssocEl || !pipelineBannerEl) return;
    if (!_renderMtPopupState.open) {
        _closeRenderMtPopup();
        return;
    }
    const solveScoreRelevant = renderColorMode === 'solve_score';
    const threads = _clampRenderMtThreads(_renderMtPopupState.rasterThreads);
    const rasterWorkers = _clampRenderMtWorkerCount(_renderMtPopupState.rasterWorkers);
    const solveScoreThreads = _clampRenderMtThreads(_renderMtPopupState.solveScoreThreads);
    const rasterRetries = _clampRenderMtRetries(_renderMtPopupState.rasterRetries);
    const finalizeWorkers = _clampRenderMtFinalizeWorkers(_renderMtPopupState.finalizeWorkers);
    const rasterSectionModeInput = _normalizeRenderMtSectionMode(_renderMtPopupState.rasterSectionMode);
    const saveAssociatedPalette = !!_renderMtPopupState.saveAssociatedPalette;
    const assocAllowed = _associatedPaletteAllowedForColorMode(renderColorMode);
    const rasterEstimate = _renderMtSectionEstimate('raster', threads);
    let rasterSectionCountAuto = rasterEstimate.available ? rasterEstimate.minSafeSections : '';
    let rasterSectionMode = rasterSectionModeInput === 'logical_sections' ? 'logical_sections' : 'logical_sections_auto';
    let rasterSectionCount = _clampRenderMtSectionCount(_renderMtPopupState.rasterSectionCount);
    if (rasterSectionMode === 'logical_sections_auto') rasterSectionCount = rasterSectionCountAuto;
    if (rasterEstimate.available && rasterSectionCount !== '' && Number(rasterSectionCount) < Number(rasterEstimate.minSafeSections)) {
        rasterSectionCount = rasterEstimate.minSafeSections;
    }
    _renderMtPopupState.rasterThreads = threads;
    _renderMtPopupState.rasterWorkers = rasterWorkers;
    _renderMtPopupState.solveScoreThreads = solveScoreThreads;
    _renderMtPopupState.rasterRetries = rasterRetries;
    _renderMtPopupState.finalizeWorkers = finalizeWorkers;
    _renderMtPopupState.rasterSectionMode = rasterSectionMode;
    _renderMtPopupState.rasterSectionCount = rasterSectionCount;
    _renderMtPopupState.saveAssociatedPalette = saveAssociatedPalette;
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    pipelineBannerEl.textContent = solveScoreRelevant
        ? (saveAssociatedPalette
            ? 'Fused path: logical-section lores clip -> raster -> FinalizeMT colorization plus associated palette artifact.'
            : 'Fused path: logical-section lores clip -> one raster pass -> FinalizeMT raw assembly/colorization.')
        : 'Fused path currently supports Solve score renders only.';
    fusedReadoutEl.textContent = !solveScoreRelevant
        ? 'Fused is only available for Solve score renders.'
        : saveAssociatedPalette
        ? `Lores clip runs first on the lores artifact only.\nAssociated palette saves a dense pass-0 parameter-grid artifact: scalar bytes for Scalar LUT, packed 3-channel bytes for RGB/HSV/RGB LUT/HSV LUT.\nlores clip threads=${solveScoreThreads} • raster threads=${threads} • raster workers=${rasterWorkers} • raster retries=${rasterRetries} • finalize workers=${finalizeWorkers}`
        : `Lores clip runs first on the lores artifact only.\nRaster reads logical solve sections via native multispan sectioned input and emits raw score fragments for the main image.\nlores clip threads=${solveScoreThreads} • raster threads=${threads} • raster workers=${rasterWorkers} • raster retries=${rasterRetries} • finalize workers=${finalizeWorkers}`;
    fusedClipThreadsEl.value = String(solveScoreThreads);
    fusedRasterThreadsEl.value = String(threads);
    fusedRasterWorkersEl.value = String(rasterWorkers);
    fusedRasterRetriesEl.value = String(rasterRetries);
    fusedFinalizeWorkersEl.value = String(finalizeWorkers);
    fusedRasterSectionModeEl.value = rasterSectionMode === 'logical_sections' ? 'logical_sections' : 'logical_sections_auto';
    fusedRasterSectionCountEl.value = rasterSectionCount === '' ? '' : String(rasterSectionCount);
    fusedClipThreadsEl.disabled = !solveScoreRelevant;
    fusedRasterThreadsEl.disabled = !solveScoreRelevant;
    fusedRasterWorkersEl.disabled = !solveScoreRelevant;
    fusedRasterRetriesEl.disabled = !solveScoreRelevant;
    fusedFinalizeWorkersEl.disabled = !solveScoreRelevant;
    fusedRasterSectionModeEl.disabled = !solveScoreRelevant;
    fusedRasterSectionCountEl.disabled = !solveScoreRelevant || fusedRasterSectionModeEl.value !== 'logical_sections';
    fusedAssocRowEl.style.display = assocAllowed ? '' : 'none';
    fusedAssocEl.checked = saveAssociatedPalette;
    fusedAssocEl.disabled = !assocAllowed;
    fusedRasterSectionCountEl.min = rasterEstimate.available ? String(rasterEstimate.minSafeSections) : '1';
    const jobSize = _renderMtJobSizeData();
    const totalSolvesLabel = jobSize && Number.isFinite(Number(jobSize.total_solves))
        ? Number(jobSize.total_solves).toLocaleString()
        : '?';
    const rootSizeLabel = jobSize ? (_fmtSize(Number(jobSize.total_root_bytes) || 0) || '?') : '?';
    const coeffSizeLabel = jobSize ? (_fmtSize(Number(jobSize.total_coeff_bytes) || 0) || '?') : '?';
    const paramSizeLabel = jobSize ? (_fmtSize(Number(jobSize.total_param_bytes) || 0) || '?') : '?';
    const rasterBudgetLabel = rasterEstimate.available ? `${_fmtSize(rasterEstimate.budgetBytes)} budget @ ${rasterEstimate.memoryMb}MB` : `auto unavailable (${rasterEstimate.reason})`;
    jobSizeEl.textContent = `Logical solve data: solves=${totalSolvesLabel} • roots=${rootSizeLabel} • coeff=${coeffSizeLabel} • params=${paramSizeLabel}\nClip: lores-only prepass\nRaster auto: ${rasterBudgetLabel}\nRaster workers: ${rasterWorkers}\nAssociated palette: ${saveAssociatedPalette ? 'save artifact' : 'off'}`;
    fusedRasterSectionSummaryEl.textContent = rasterEstimate.available
        ? `sources=${_renderMtSectionSourceLabel(rasterEstimate)} • min safe sections=${rasterEstimate.minSafeSections} • max solves/section=${rasterEstimate.maxSolvesPerSection.toLocaleString()} • est section=${_fmtSize(rasterEstimate.estimatedSectionBytes)}`
        : `auto unavailable: ${rasterEstimate.reason}`;
    const assocLabel = assocAllowed ? (saveAssociatedPalette ? 'yes' : 'no') : 'n/a';
    const rasterSectionLabel = _renderMtSectionSettingLabel(rasterSectionMode, rasterSectionCount);
    summaryEl.textContent = solveScoreRelevant
        ? `Path: fused • Mode: Solve score • output ${document.getElementById('render-pix').value || '?'} px • lores clip threads=${solveScoreThreads} • raster input=sectioned • raster retries=${rasterRetries} • raster threads=${threads} • raster workers=${rasterWorkers} • raster sections=${rasterSectionLabel} • bins=raw score bins • finalize workers=${finalizeWorkers} • associated palette=${assocLabel}${saveAssociatedPalette ? ' • artifact output' : ''}`
        : 'Path: fused • Color render supports Solve score only';
    if (helpEl) {
        helpEl.textContent = saveAssociatedPalette
            ? 'Fused is the only color render path. Associated palette saves a dense pass-0 parameter-grid artifact, not a copy of the final rendered image.'
            : 'Fused is the only color render path. It always uses sectioned native input and logical sections, then runs FinalizeMT raw assembly/colorization.';
    }
    if (runBtn) runBtn.disabled = !_renderColorMtEligible() || !!_activeRenderRun || !solveScoreRelevant;
}

async function openRenderMtPopup() {
    if (!_renderColorMtEligible() || _activeRenderRun) return;
    let jobSizeLoadError = '';
    const jobId = _renderMtCurrentJobId();
    if (jobId && (_renderLoadedJobId !== jobId || !_renderMtJobSizeData())) {
        try {
            await _refreshRenderSummaryData(jobId);
        } catch (e) {
            jobSizeLoadError = e && e.message ? e.message : String(e);
        }
    }
    _renderMtPopupState = {
        open: true,
        rasterThreads: _clampRenderMtThreads(_renderMtPopupState.rasterThreads || 4),
        rasterWorkers: _clampRenderMtWorkerCount(_renderMtPopupState.rasterWorkers || 10),
        solveScoreThreads: _clampRenderMtThreads(_renderMtPopupState.solveScoreThreads || 4),
        rasterRetries: _clampRenderMtRetries(_renderMtPopupState.rasterRetries ?? 2),
        finalizeWorkers: _clampRenderMtFinalizeWorkers(_renderMtPopupState.finalizeWorkers || 16),
        rasterSectionMode: _normalizeRenderMtSectionMode(_renderMtPopupState.rasterSectionMode || 'logical_sections_auto'),
        rasterSectionCount: _clampRenderMtSectionCount(_renderMtPopupState.rasterSectionCount),
        saveAssociatedPalette: !!_renderMtPopupState.saveAssociatedPalette,
        jobSizeLoadError,
    };
    _renderRenderMtPopup();
    const focusEl = document.getElementById('render-mt-fused-clip-threads');
    if (focusEl && typeof focusEl.focus === 'function') focusEl.focus();
}

function _closeComputeMtPopup() {
    const solverMode = _computeMtPopupState.solverMode || 'aberth_mt';
    _computePopupPrefsBySolver[solverMode] = {
        solverMode,
        fused: true,
        nChunks: Math.max(1, parseInt(_computeMtPopupState.nChunks, 10) || 10),
        fusedThreads: _clampRenderMtThreads(_computeMtPopupState.fusedThreads || 4),
        loresParamGenThreads: _clampRenderMtThreads(_computeMtPopupState.loresParamGenThreads),
        loresCoeffgenThreads: _clampRenderMtThreads(_computeMtPopupState.loresCoeffgenThreads),
        probe: _computeMtPopupState.probe || null,
        probeError: _computeMtPopupState.probeError || '',
        probeLoading: false,
        probeSignature: _computeMtPopupState.probeSignature || '',
    };
    _computeMtPopupState = { open: false, ..._computePopupPrefsBySolver[solverMode] };
    const overlay = document.getElementById('compute-mt-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _renderComputeMtPopup() {
    const overlay = document.getElementById('compute-mt-popup-overlay');
    const titleEl = document.getElementById('compute-mt-popup-title');
    const summaryEl = document.getElementById('compute-mt-popup-summary');
    const descEl = document.getElementById('compute-mt-popup-description');
    const footerHelpEl = document.getElementById('compute-mt-popup-footer-help');
    const fusedPanelEl = document.getElementById('compute-mt-fused-panel');
    const fusedThreadsLabelEl = document.getElementById('compute-mt-fused-threads-label');
    const fusedSolveRowEl = document.getElementById('compute-mt-fused-solve-row');
    const fusedChunksEl = document.getElementById('compute-mt-fused-chunks');
    const fusedMinChunksEl = document.getElementById('compute-mt-fused-min-chunks');
    const sizingEl = document.getElementById('compute-mt-sizing-summary');
    const fusedThreadsEl = document.getElementById('compute-mt-fused-threads');
    const fusedParamMirrorEl = document.getElementById('compute-mt-fused-param-gen-mirror');
    const fusedCoeffMirrorEl = document.getElementById('compute-mt-fused-coeffgen-mirror');
    const fusedSolveMirrorEl = document.getElementById('compute-mt-fused-solve-threads');
    const applySafeChunksBtn = document.getElementById('compute-mt-apply-safe-chunks');
    const loresThreadsFusedEl = document.getElementById('compute-mt-lores-param-gen-threads-fused');
    const loresCoeffThreadsFusedEl = document.getElementById('compute-mt-lores-coeffgen-threads-fused');
    const runBtn = document.getElementById('compute-mt-popup-run');
    if (!overlay || !titleEl || !summaryEl || !fusedPanelEl || !fusedChunksEl || !fusedMinChunksEl || !sizingEl || !fusedThreadsEl || !fusedParamMirrorEl || !fusedCoeffMirrorEl || !fusedSolveMirrorEl || !applySafeChunksBtn || !loresThreadsFusedEl || !loresCoeffThreadsFusedEl) return;
    if (!_computeMtPopupState.open) {
        _closeComputeMtPopup();
        return;
    }
    const solverMode = _computeMtPopupState.solverMode || 'aberth_mt';
    const nChunks = Math.max(1, parseInt(_computeMtPopupState.nChunks, 10) || 10);
    const fusedThreads = _clampRenderMtThreads(_computeMtPopupState.fusedThreads || 4);
    const loresParamGenThreads = _clampRenderMtThreads(_computeMtPopupState.loresParamGenThreads);
    const loresCoeffgenThreads = _clampRenderMtThreads(_computeMtPopupState.loresCoeffgenThreads);
    _computeMtPopupState.fused = true;
    _computeMtPopupState.nChunks = nChunks;
    _computeMtPopupState.fusedThreads = fusedThreads;
    _computeMtPopupState.loresParamGenThreads = loresParamGenThreads;
    _computeMtPopupState.loresCoeffgenThreads = loresCoeffgenThreads;
    titleEl.textContent = _solverRunLabel(solverMode);
    if (descEl) {
        descEl.textContent = 'Fused keeps the compute artifact contract but runs param gen, coeffgen, and solve in one Lambda per chunk.';
    }
    if (footerHelpEl) {
        footerHelpEl.textContent = _solverHasThreadedFusedSolve(solverMode)
            ? 'Fused adds a degree probe, validates the requested chunk count, runs lores separately, then processes each hires chunk in one Lambda with local param/coeff/solve handoff.'
            : 'Fused adds a degree probe, validates the requested chunk count, runs lores separately, then processes each hires chunk in one Lambda with local param/coeff/solve handoff. Shared hires generation threads affect param-gen and coeffgen; solve stays on the selected solver path.';
    }
    if (fusedThreadsLabelEl) fusedThreadsLabelEl.textContent = _computePopupSharedThreadsLabel(solverMode);
    if (fusedSolveRowEl) fusedSolveRowEl.style.display = _solverHasThreadedFusedSolve(solverMode) ? '' : 'none';
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    fusedPanelEl.className = 'compute-mt-tab-panel';
    fusedPanelEl.setAttribute('aria-hidden', 'false');
    fusedChunksEl.value = String(nChunks);
    fusedThreadsEl.value = String(fusedThreads);
    fusedParamMirrorEl.textContent = String(fusedThreads);
    fusedCoeffMirrorEl.textContent = String(fusedThreads);
    fusedSolveMirrorEl.textContent = String(fusedThreads);
    loresThreadsFusedEl.value = String(loresParamGenThreads);
    loresCoeffThreadsFusedEl.value = String(loresCoeffgenThreads);
    const funcName = document.getElementById('render-function')?.value || '?';
    const n = parseInt(document.getElementById('render-n')?.value, 10) || 0;
    const times = Math.max(1, parseInt(document.getElementById('render-times')?.value, 10) || 1);
    summaryEl.textContent = `Solver: ${_solverTag(solverMode)} • Function: ${funcName} • N=${n || '?'} • chunks=${nChunks} • times=${times} • method=fused`;
    const safeChunks = Number(_computeMtPopupState.probe?.fused_estimate?.min_safe_chunks) || 0;
    if (_computeMtPopupState.probeLoading) {
        sizingEl.textContent = 'Probe: loading...';
        fusedMinChunksEl.textContent = 'Min safe chunks=...';
    } else if (_computeMtPopupState.probeError) {
        sizingEl.textContent = `Probe failed: ${_computeMtPopupState.probeError}`;
        fusedMinChunksEl.textContent = 'Min safe chunks=?';
    } else if (_computeMtPopupState.probe && _computeMtPopupState.probe.fused_estimate) {
        const probe = _computeMtPopupState.probe;
        const est = probe.fused_estimate;
        sizingEl.textContent =
            `Probe: degree=${probe.degree}, n_coeffs=${probe.n_coeffs}, min safe chunks=${est.min_safe_chunks}, ` +
            `params=${(est.params_bytes/1e6).toFixed(1)}MB, coeff=${(est.coeff_bytes/1e6).toFixed(1)}MB, ` +
            `roots=${(est.roots_bytes/1e6).toFixed(1)}MB, peak=${(est.estimated_peak_bytes/1e6).toFixed(1)}MB, ` +
            `/tmp=${(est.estimated_tmp_peak_bytes/1e6).toFixed(1)}MB, limit=${est.safe_chunk_limit_reason}`;
        fusedMinChunksEl.textContent = `Min safe chunks=${est.min_safe_chunks}`;
        fusedChunksEl.min = String(est.min_safe_chunks);
    } else if (_computeMtPopupState.probe) {
        sizingEl.textContent = `Probe: degree=${_computeMtPopupState.probe.degree}, n_coeffs=${_computeMtPopupState.probe.n_coeffs}`;
        fusedMinChunksEl.textContent = 'Min safe chunks=?';
    } else {
        sizingEl.textContent = 'Probe not run yet.';
        fusedMinChunksEl.textContent = 'Min safe chunks=?';
    }
    applySafeChunksBtn.disabled = !(safeChunks > 0);
    if (runBtn) {
        runBtn.disabled = !!document.getElementById(_solverButtonId(solverMode))?.disabled || _computeMtPopupState.probeLoading || !_computeMtPopupState.probe || !_computeMtPopupState.probe.fused_estimate;
    }
}

async function _refreshComputeMtProbe() {
    if (!_computeMtPopupState.open) return;
    const funcName = document.getElementById('render-function')?.value || '';
    const n = Math.max(1, parseInt(document.getElementById('render-n')?.value, 10) || 0);
    const nChunks = Math.max(1, parseInt(_computeMtPopupState.nChunks, 10) || 10);
    const times = Math.max(1, parseInt(document.getElementById('render-times')?.value, 10) || 1);
    const paramTransforms = _effectiveParamTransformsForCompute();
    const paramProgramChain = _effectiveParamProgramChainForCompute();
    const coeffTransforms = _effectiveCoeffTransformsForCompute();
    const coeffProgramChain = _effectiveCoeffProgramChainForCompute();
    const coeffProgramSourceText = _effectiveCoeffProgramSourceTextForCompute() || '';
    const cfpv = _cfpv.length > 0 ? [..._cfpv] : [];
    const fusedThreads = _clampRenderMtThreads(_computeMtPopupState.fusedThreads || 4);
    const solverMode = _computeMtPopupState.solverMode || 'aberth_mt';
    const probeSignature = JSON.stringify([_selectedParamPipelineMode(), solverMode, funcName, paramTransforms, paramProgramChain, coeffTransforms, coeffProgramChain, coeffProgramSourceText, cfpv, n, times, nChunks, fusedThreads]);
    if (_computeMtPopupState.probeSignature === probeSignature && (_computeMtPopupState.probe || _computeMtPopupState.probeError)) {
        _renderComputeMtPopup();
        return;
    }
    _computeMtPopupState.probeSignature = probeSignature;
    _computeMtPopupState.probeLoading = true;
    _computeMtPopupState.probeError = '';
    _renderComputeMtPopup();
    try {
        const probe = await lambdaPost('coeffgen', _attachCoeffProgramSourcePayload({
            phase: 'degree_probe',
            pipeline_mode: _selectedParamPipelineMode(),
            function: funcName,
            N: n,
            times,
            n_chunks: nChunks,
            solver_mode: solverMode,
            fused_threads: fusedThreads,
            execution_method: 'fused_chunk_pipeline',
            param_transforms: paramTransforms,
            param_program_chain: paramProgramChain,
            coeff_transforms: coeffTransforms,
            coeff_program_chain: coeffProgramChain,
            cfpv,
        }));
        if (!probe || !probe.probe_stable) {
            throw new Error('unstable degree probe');
        }
        _computeMtPopupState.probe = probe;
    } catch (e) {
        _computeMtPopupState.probe = null;
        _computeMtPopupState.probeError = e.message || String(e);
    } finally {
        _computeMtPopupState.probeLoading = false;
        _renderComputeMtPopup();
    }
}

async function _applyComputeMtSafeChunks() {
    if (!_computeMtPopupState.probe || !_computeMtPopupState.probe.fused_estimate) {
        await _refreshComputeMtProbe();
    }
    const safeChunks = Number(_computeMtPopupState.probe?.fused_estimate?.min_safe_chunks) || 0;
    if (safeChunks <= 0) return 0;
    _computeMtPopupState.nChunks = safeChunks;
    _computeMtPopupState.probeSignature = '';
    _renderComputeMtPopup();
    void _refreshComputeMtProbe();
    return safeChunks;
}

async function openComputeSolverPopup(solverMode) {
    const prefs = { ..._computePopupPrefsForSolver(solverMode) };
    const sharedChunkState = Math.max(1, parseInt(document.getElementById('render-stripes')?.value, 10) || 0);
    _computeMtPopupState = {
        open: true,
        solverMode,
        fused: true,
        nChunks: sharedChunkState || Math.max(1, parseInt(prefs.nChunks, 10) || 10),
        fusedThreads: _clampRenderMtThreads(prefs.fusedThreads || 4),
        loresParamGenThreads: _clampRenderMtThreads(prefs.loresParamGenThreads || 1),
        loresCoeffgenThreads: _clampRenderMtThreads(prefs.loresCoeffgenThreads || 1),
        probe: prefs.probe || null,
        probeError: '',
        probeLoading: false,
        probeSignature: '',
    };
    _renderComputeMtPopup();
    void _refreshComputeMtProbe();
    const inputEl = document.getElementById('compute-mt-fused-threads');
    if (inputEl && typeof inputEl.focus === 'function') inputEl.focus();
}

function _visiblePdfColorSpreadCatalog() {
    const filter = (_pdfColorSpreadPopupState.filter || '').trim().toLowerCase();
    const items = _pdfColorSourceArtifacts();
    if (!filter) return items;
    return items.filter(art => {
        const text = [
            art.artifact_id || '',
            art.created_at || '',
            art.color_mode || '',
            art.palette || '',
            _renderArtifactSummary(art) || '',
        ].join(' ').toLowerCase();
        return text.includes(filter);
    });
}

function _closePdfColorSpreadPopup() {
    _pdfColorSpreadPopupState = { open: false, filter: '', highlightIdx: 0, sourceArtifactId: '' };
    const overlay = document.getElementById('pdf-colorspread-popup-overlay');
    const filter = document.getElementById('pdf-colorspread-popup-filter');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
    if (filter) filter.value = '';
}

function _renderPdfColorSpreadPopup() {
    const overlay = document.getElementById('pdf-colorspread-popup-overlay');
    const bodyEl = document.getElementById('pdf-colorspread-popup-body');
    const filterEl = document.getElementById('pdf-colorspread-popup-filter');
    const summaryEl = document.getElementById('pdf-colorspread-popup-summary');
    const runBtn = document.getElementById('pdf-colorspread-popup-run');
    if (!overlay || !bodyEl || !filterEl || !summaryEl) return;
    if (!_pdfColorSpreadPopupState.open) {
        _closePdfColorSpreadPopup();
        return;
    }

    const visible = _visiblePdfColorSpreadCatalog();
    const highlightIdx = visible.length ? Math.max(0, Math.min(_pdfColorSpreadPopupState.highlightIdx || 0, visible.length - 1)) : 0;
    _pdfColorSpreadPopupState.highlightIdx = highlightIdx;
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    filterEl.value = _pdfColorSpreadPopupState.filter || '';
    bodyEl.replaceChildren();

    if (!visible.length) {
        const row = document.createElement('tr');
        row.className = 'tri-popup-empty';
        const cell = document.createElement('td');
        cell.colSpan = 3;
        cell.textContent = 'No Color artifacts match this filter.';
        row.appendChild(cell);
        bodyEl.appendChild(row);
        summaryEl.textContent = 'Select a Color artifact to generate a PDF spread.';
        if (runBtn) runBtn.disabled = true;
        return;
    }

    visible.forEach((art, idx) => {
        const row = document.createElement('tr');
        const cls = ['tri-popup-row'];
        if (idx === highlightIdx) cls.push('highlight');
        row.className = cls.join(' ');
        row.onclick = () => {
            _pdfColorSpreadPopupState.highlightIdx = idx;
            _renderPdfColorSpreadPopup();
        };

        const createdCell = document.createElement('td');
        createdCell.textContent = (art.created_at || '').replace('T', ' ').slice(0, 19);
        const dimsCell = document.createElement('td');
        dimsCell.textContent = art.width && art.height ? `${art.width}x${art.height}` : '';
        const summaryCell = document.createElement('td');
        summaryCell.textContent = _renderArtifactSummary(art) || art.artifact_id || '';
        row.appendChild(createdCell);
        row.appendChild(dimsCell);
        row.appendChild(summaryCell);
        bodyEl.appendChild(row);
    });

    const selected = visible[highlightIdx];
    if (selected) _pdfColorSpreadPopupState.sourceArtifactId = selected.artifact_id || '';
    summaryEl.textContent = selected
        ? `Source: ${selected.artifact_id || '?'} • ${_renderArtifactSummary(selected) || 'Color artifact'} • output: PDF ColorSpread`
        : 'Select a Color artifact to generate a PDF spread.';
    if (runBtn) runBtn.disabled = !selected;
}

function openPdfColorSpreadPopup() {
    const items = _pdfColorSourceArtifacts();
    if (!items.length || _activeRenderRun) return;
    const activeColor = (_renderArtifacts.color || [])[Math.max(0, _renderSelectedArtifact.color || 0)] || null;
    let highlightIdx = 0;
    if (activeColor && activeColor.artifact_id) {
        const idx = items.findIndex(art => art.artifact_id === activeColor.artifact_id);
        if (idx >= 0) highlightIdx = idx;
    }
    _pdfColorSpreadPopupState = {
        open: true,
        filter: '',
        highlightIdx,
        sourceArtifactId: activeColor ? (activeColor.artifact_id || '') : '',
    };
    _renderPdfColorSpreadPopup();
    const filter = document.getElementById('pdf-colorspread-popup-filter');
    if (filter && typeof filter.focus === 'function') filter.focus();
}

function _applyPdfColorSpreadFilter(text) {
    _pdfColorSpreadPopupState.filter = String(text || '');
    _pdfColorSpreadPopupState.highlightIdx = 0;
    _renderPdfColorSpreadPopup();
}

async function runPdfColorSpreadSelected() {
    if (_activeRenderRun) return;
    const visible = _visiblePdfColorSpreadCatalog();
    if (!visible.length) return;
    const idx = Math.max(0, Math.min(_pdfColorSpreadPopupState.highlightIdx || 0, visible.length - 1));
    const art = visible[idx];
    const jobId = document.getElementById('render-results-dir').value.trim();
    const btn = document.getElementById('pdf-colorspread-popup-run');
    const orig = btn ? btn.textContent : 'Execute';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Executing...'; }
        const runId = _generateRunId();
        const taskId = 'pdf_' + runId;
        const artifactId = 'pdf_' + Date.now() + '_' + Math.random().toString(36).slice(2, 8);
        log(`PDF ColorSpread: dispatching ${art.artifact_id} -> ${artifactId}...`, '', 'render-log');
        const dispResult = await lambdaPost('dispatch', {
            target: 'pdf_artifact',
            jobs: [{
                job_id: jobId,
                task_id: taskId,
                artifact_id: artifactId,
                source_artifact_id: art.artifact_id,
                source_image_key: art.image_key,
            }],
            expected_keys: [],
        });
        if ((dispResult.fired || 0) !== 1) throw new Error('PDF ColorSpread dispatch failed');
        _saveActiveRun({
            job_id: jobId,
            mode: 'pdf',
            run_id: runId,
            task_id: taskId,
            started_at_ms: Date.now(),
        });
        document.getElementById('render-status').textContent = 'PDF ColorSpread queued...';
        document.getElementById('render-status').className = 'status';
        _closePdfColorSpreadPopup();
        startActiveRenderObserver();
    } catch (e) {
        const statusEl = document.getElementById('render-status');
        if (statusEl) {
            statusEl.textContent = 'PDF error: ' + e.message;
            statusEl.className = 'status error';
        }
        log('PDF ColorSpread failed: ' + e.message, 'err', 'render-log');
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

function _initRepalettePopup() {
    const runBtn = document.getElementById('repalette-popup-run');
    _bindPopupShell({
        overlayId: 'repalette-popup-overlay',
        closeId: 'repalette-popup-close',
        cancelId: 'repalette-popup-cancel',
        isOpen: () => !!_repalettePopupState.open,
        onClose: _closeRepalettePopup,
    });
    if (runBtn) runBtn.addEventListener('click', runRepaletteSelectedArtifact);
}

;(window.__ppParts = window.__ppParts || []).push('05-render-popups');
