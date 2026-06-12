// PolyPaint 06-popup-init — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Cache-busting: deploy appends ?v=<BUILD_ID> to the tags.
function _initColorRepalettePopup() {
    const runBtn = document.getElementById('color-repalette-popup-run');
    _bindPopupShell({
        overlayId: 'color-repalette-popup-overlay',
        closeId: 'color-repalette-popup-close',
        cancelId: 'color-repalette-popup-cancel',
        isOpen: () => !!_colorRepalettePopupState.open,
        onClose: _closeColorRepalettePopup,
    });
    if (runBtn) runBtn.addEventListener('click', runColorRepaletteSelectedArtifact);
}

function _initRenderMtPopup() {
    const runBtn = document.getElementById('render-mt-popup-run');
    const fusedClipThreadsEl = document.getElementById('render-mt-fused-clip-threads');
    const fusedRasterThreadsEl = document.getElementById('render-mt-fused-raster-threads');
    const fusedRasterWorkersEl = document.getElementById('render-mt-fused-raster-workers');
    const fusedRasterRetriesEl = document.getElementById('render-mt-fused-raster-retries');
    const fusedFinalizeWorkersEl = document.getElementById('render-mt-fused-finalize-workers');
    const fusedRasterSectionModeEl = document.getElementById('render-mt-fused-raster-section-mode');
    const fusedRasterSectionCountEl = document.getElementById('render-mt-fused-raster-section-count');
    const fusedAssocEl = document.getElementById('render-mt-fused-save-associated-palette');
    const readCfg = () => ({
        rasterThreads: _clampRenderMtThreads(fusedRasterThreadsEl.value),
        rasterWorkers: _clampRenderMtWorkerCount(fusedRasterWorkersEl.value),
        solveScoreThreads: _clampRenderMtThreads(fusedClipThreadsEl.value),
        rasterRetries: _clampRenderMtRetries(fusedRasterRetriesEl.value),
        rasterSectionMode: _normalizeRenderMtSectionMode(fusedRasterSectionModeEl.value),
        rasterSectionCount: _clampRenderMtSectionCount(fusedRasterSectionCountEl.value),
        finalizeWorkers: _clampRenderMtFinalizeWorkers(fusedFinalizeWorkersEl.value),
        saveAssociatedPalette: !!fusedAssocEl.checked,
    });
    const persistCfg = (cfg) => {
        _renderMtPopupState.rasterThreads = cfg.rasterThreads;
        _renderMtPopupState.rasterWorkers = cfg.rasterWorkers;
        _renderMtPopupState.solveScoreThreads = cfg.solveScoreThreads;
        _renderMtPopupState.rasterRetries = cfg.rasterRetries;
        _renderMtPopupState.rasterSectionMode = cfg.rasterSectionMode;
        _renderMtPopupState.rasterSectionCount = cfg.rasterSectionCount;
        _renderMtPopupState.finalizeWorkers = cfg.finalizeWorkers;
        _renderMtPopupState.saveAssociatedPalette = !!cfg.saveAssociatedPalette;
    };
    const cfgToRun = (cfg) => ({
        rasterThreads: cfg.rasterThreads,
        rasterWorkers: cfg.rasterWorkers,
        solveScoreThreads: cfg.solveScoreThreads,
        rasterRetries: cfg.rasterRetries,
        rasterSectionMode: cfg.rasterSectionMode,
        rasterSectionCount: cfg.rasterSectionCount,
        finalizeWorkers: cfg.finalizeWorkers,
        saveAssociatedPalette: cfg.saveAssociatedPalette,
    });
    _bindPopupShell({
        overlayId: 'render-mt-popup-overlay',
        closeId: 'render-mt-popup-close',
        cancelId: 'render-mt-popup-cancel',
        isOpen: () => !!_renderMtPopupState.open,
        onClose: _closeRenderMtPopup,
        onEnter: () => {
            const cfg = readCfg();
            persistCfg(cfg);
            _closeRenderMtPopup();
            runRasterPipelineMT(cfgToRun(cfg));
        },
    });
    if (fusedClipThreadsEl) fusedClipThreadsEl.addEventListener('input', (ev) => { _renderMtPopupState.solveScoreThreads = _clampRenderMtThreads(ev.target.value); _renderRenderMtPopup(); });
    if (fusedRasterThreadsEl) fusedRasterThreadsEl.addEventListener('input', (ev) => { _renderMtPopupState.rasterThreads = _clampRenderMtThreads(ev.target.value); _renderRenderMtPopup(); });
    if (fusedRasterWorkersEl) fusedRasterWorkersEl.addEventListener('input', (ev) => { _renderMtPopupState.rasterWorkers = _clampRenderMtWorkerCount(ev.target.value); _renderRenderMtPopup(); });
    if (fusedRasterRetriesEl) fusedRasterRetriesEl.addEventListener('input', (ev) => { _renderMtPopupState.rasterRetries = _clampRenderMtRetries(ev.target.value); _renderRenderMtPopup(); });
    if (fusedFinalizeWorkersEl) fusedFinalizeWorkersEl.addEventListener('input', (ev) => { _renderMtPopupState.finalizeWorkers = _clampRenderMtFinalizeWorkers(ev.target.value); _renderRenderMtPopup(); });
    if (fusedRasterSectionModeEl) fusedRasterSectionModeEl.addEventListener('change', (ev) => { _renderMtPopupState.rasterSectionMode = _normalizeRenderMtSectionMode(ev.target.value); _renderRenderMtPopup(); });
    if (fusedRasterSectionCountEl) fusedRasterSectionCountEl.addEventListener('input', (ev) => { _renderMtPopupState.rasterSectionCount = _clampRenderMtSectionCount(ev.target.value); _renderRenderMtPopup(); });
    if (fusedAssocEl) fusedAssocEl.addEventListener('change', (ev) => { _renderMtPopupState.saveAssociatedPalette = !!ev.target.checked; _renderRenderMtPopup(); });
    if (runBtn) {
        runBtn.addEventListener('click', async () => {
            const btnText = runBtn.textContent;
            try {
                runBtn.disabled = true;
                runBtn.textContent = 'Executing...';
                const cfg = readCfg();
                persistCfg(cfg);
                _closeRenderMtPopup();
                await runRasterPipelineMT(cfgToRun(cfg));
            } finally {
                runBtn.disabled = false;
                runBtn.textContent = btnText;
            }
        });
    }
}

function _initComputeMtPopup() {
    const runBtn = document.getElementById('compute-mt-popup-run');
    const fusedChunksEl = document.getElementById('compute-mt-fused-chunks');
    const fusedThreadsEl = document.getElementById('compute-mt-fused-threads');
    const applySafeChunksBtn = document.getElementById('compute-mt-apply-safe-chunks');
    const loresThreadsFusedEl = document.getElementById('compute-mt-lores-param-gen-threads-fused');
    const loresCoeffThreadsFusedEl = document.getElementById('compute-mt-lores-coeffgen-threads-fused');
    const runComputeMtPopup = async () => {
        const solverMode = _computeMtPopupState.solverMode || 'aberth_mt';
        const nChunks = Math.max(1, parseInt(_computeMtPopupState.nChunks, 10) || 10);
        const fusedThreads = _clampRenderMtThreads(_computeMtPopupState.fusedThreads);
        const loresParamGenThreads = _clampRenderMtThreads(_computeMtPopupState.loresParamGenThreads);
        const loresCoeffgenThreads = _clampRenderMtThreads(_computeMtPopupState.loresCoeffgenThreads);
        if (!_computeMtPopupState.probe || !_computeMtPopupState.probe.fused_estimate) {
            await _refreshComputeMtProbe();
        }
        if (!_computeMtPopupState.probe || !_computeMtPopupState.probe.fused_estimate) {
            _renderComputeMtPopup();
            return false;
        }
        _computeMtPopupState.fused = true;
        _computeMtPopupState.nChunks = nChunks;
        _computeMtPopupState.fusedThreads = fusedThreads;
        _computeMtPopupState.loresParamGenThreads = loresParamGenThreads;
        _computeMtPopupState.loresCoeffgenThreads = loresCoeffgenThreads;
        document.getElementById('render-stripes').value = String(nChunks);
        _closeComputeMtPopup();
        await runCalculateWithSolver(solverMode, { nChunks, fused: true, fusedThreads, loresParamGenThreads, loresCoeffgenThreads });
        return true;
    };
    _bindPopupShell({
        overlayId: 'compute-mt-popup-overlay',
        closeId: 'compute-mt-popup-close',
        cancelId: 'compute-mt-popup-cancel',
        isOpen: () => !!_computeMtPopupState.open,
        onClose: _closeComputeMtPopup,
        onEnter: () => { void runComputeMtPopup(); },
    });
    if (fusedChunksEl) {
        fusedChunksEl.addEventListener('input', (ev) => {
            _computeMtPopupState.nChunks = Math.max(1, parseInt(ev.target.value, 10) || _computeMtPopupState.nChunks || 10);
            _computeMtPopupState.probe = null;
            _computeMtPopupState.probeError = '';
            _renderComputeMtPopup();
            void _refreshComputeMtProbe();
        });
    }
    if (fusedThreadsEl) {
        fusedThreadsEl.addEventListener('input', (ev) => {
            _computeMtPopupState.fusedThreads = _clampRenderMtThreads(ev.target.value);
            _computeMtPopupState.probe = null;
            _computeMtPopupState.probeError = '';
            _renderComputeMtPopup();
            void _refreshComputeMtProbe();
        });
    }
    if (applySafeChunksBtn) {
        applySafeChunksBtn.addEventListener('click', () => { void _applyComputeMtSafeChunks(); });
    }
    if (loresThreadsFusedEl) {
        loresThreadsFusedEl.addEventListener('input', (ev) => {
            _computeMtPopupState.loresParamGenThreads = _clampRenderMtThreads(ev.target.value);
            _renderComputeMtPopup();
        });
    }
    if (loresCoeffThreadsFusedEl) {
        loresCoeffThreadsFusedEl.addEventListener('input', (ev) => {
            _computeMtPopupState.loresCoeffgenThreads = _clampRenderMtThreads(ev.target.value);
            _renderComputeMtPopup();
        });
    }
    if (runBtn) {
        runBtn.addEventListener('click', async () => {
            const btnText = runBtn.textContent;
            let launched = false;
            try {
                runBtn.disabled = true;
                runBtn.textContent = 'Executing...';
                launched = await runComputeMtPopup();
            } finally {
                runBtn.textContent = btnText;
                if (launched || !_computeMtPopupState.open) {
                    runBtn.disabled = false;
                } else {
                    _renderComputeMtPopup();
                }
            }
        });
    }
}

function _initResultsRefreshPopup() {
    const runBtn = document.getElementById('results-refresh-popup-run');
    const workersEl = document.getElementById('results-refresh-workers');
    _bindPopupShell({
        overlayId: 'results-refresh-popup-overlay',
        closeId: 'results-refresh-popup-close',
        cancelId: 'results-refresh-popup-cancel',
        isOpen: () => !!_resultsRefreshPopupState.open,
        onClose: _closeResultsRefreshPopup,
        onEnter: () => {
            const workers = _clampResultsListWorkers(document.getElementById('results-refresh-workers').value);
            _resultsRefreshPopupState.workers = workers;
            _closeResultsRefreshPopup();
            loadResults({ listWorkers: workers });
        },
    });
    if (workersEl) {
        workersEl.addEventListener('input', (ev) => {
            _resultsRefreshPopupState.workers = _clampResultsListWorkers(ev.target.value);
            _renderResultsRefreshPopup();
        });
    }
    if (runBtn) {
        runBtn.addEventListener('click', async () => {
            const btnText = runBtn.textContent;
            try {
                runBtn.disabled = true;
                runBtn.textContent = 'Executing...';
                const workers = _clampResultsListWorkers(document.getElementById('results-refresh-workers').value);
                _resultsRefreshPopupState.workers = workers;
                _closeResultsRefreshPopup();
                await loadResults({ listWorkers: workers });
            } finally {
                runBtn.disabled = false;
                runBtn.textContent = btnText;
            }
        });
    }
}

function _initExtractPalettePopup() {
    const runBtn = document.getElementById('extract-palette-popup-run');
    const threadsEl = document.getElementById('extract-palette-solve-score-threads');
    const histInputModeEl = document.getElementById('extract-palette-hist-input-mode');
    const histRetriesEl = document.getElementById('extract-palette-hist-retries');
    const mergeWorkersEl = document.getElementById('extract-palette-merge-workers');
    const chunkThreadsEl = document.getElementById('extract-palette-chunk-threads');
    const chunkInputModeEl = document.getElementById('extract-palette-chunk-input-mode');
    const chunkRetriesEl = document.getElementById('extract-palette-chunk-retries');
    const chunkWorkersEl = document.getElementById('extract-palette-chunk-workers');
    _bindPopupShell({
        overlayId: 'extract-palette-popup-overlay',
        closeId: 'extract-palette-popup-close',
        cancelId: 'extract-palette-popup-cancel',
        isOpen: () => !!_extractPalettePopupState.open,
        onClose: _closeExtractPalettePopup,
        onEnter: () => {
            const solveScoreThreads = _clampRenderMtThreads(document.getElementById('extract-palette-solve-score-threads').value);
            const histInputMode = _normalizeSolveScoreHistInputMode(document.getElementById('extract-palette-hist-input-mode').value);
            const histRetries = _clampRenderMtRetries(document.getElementById('extract-palette-hist-retries').value);
            const mergeWorkers = _clampRenderMtMergeWorkers(document.getElementById('extract-palette-merge-workers').value);
            const chunkThreads = _clampRenderMtThreads(document.getElementById('extract-palette-chunk-threads').value);
            const chunkInputMode = _normalizeRasterInputMode(document.getElementById('extract-palette-chunk-input-mode').value);
            const chunkRetries = _clampRenderMtRetries(document.getElementById('extract-palette-chunk-retries').value);
            const chunkWorkers = _clampRenderMtWorkerCount(document.getElementById('extract-palette-chunk-workers').value);
            _extractPalettePopupState.solveScoreThreads = solveScoreThreads;
            _extractPalettePopupState.histInputMode = histInputMode;
            _extractPalettePopupState.histRetries = histRetries;
            _extractPalettePopupState.mergeWorkers = mergeWorkers;
            _extractPalettePopupState.chunkThreads = chunkThreads;
            _extractPalettePopupState.chunkInputMode = chunkInputMode;
            _extractPalettePopupState.chunkRetries = chunkRetries;
            _extractPalettePopupState.chunkWorkers = chunkWorkers;
            _closeExtractPalettePopup();
            runExtractPaletteArtifact({ solveScoreThreads, histInputMode, histRetries, mergeWorkers, chunkThreads, chunkInputMode, chunkRetries, chunkWorkers });
        },
    });
    if (threadsEl) {
        threadsEl.addEventListener('input', (ev) => {
            _extractPalettePopupState.solveScoreThreads = _clampRenderMtThreads(ev.target.value);
            _renderExtractPalettePopup();
        });
    }
    if (histInputModeEl) {
        histInputModeEl.addEventListener('change', (ev) => {
            _extractPalettePopupState.histInputMode = _normalizeSolveScoreHistInputMode(ev.target.value);
            _renderExtractPalettePopup();
        });
    }
    if (histRetriesEl) {
        histRetriesEl.addEventListener('input', (ev) => {
            _extractPalettePopupState.histRetries = _clampRenderMtRetries(ev.target.value);
            _renderExtractPalettePopup();
        });
    }
    if (mergeWorkersEl) {
        mergeWorkersEl.addEventListener('input', (ev) => {
            _extractPalettePopupState.mergeWorkers = _clampRenderMtMergeWorkers(ev.target.value);
            _renderExtractPalettePopup();
        });
    }
    if (chunkThreadsEl) {
        chunkThreadsEl.addEventListener('input', (ev) => {
            _extractPalettePopupState.chunkThreads = _clampRenderMtThreads(ev.target.value);
            _renderExtractPalettePopup();
        });
    }
    if (chunkInputModeEl) {
        chunkInputModeEl.addEventListener('change', (ev) => {
            _extractPalettePopupState.chunkInputMode = _normalizeRasterInputMode(ev.target.value);
            _renderExtractPalettePopup();
        });
    }
    if (chunkRetriesEl) {
        chunkRetriesEl.addEventListener('input', (ev) => {
            _extractPalettePopupState.chunkRetries = _clampRenderMtRetries(ev.target.value);
            _renderExtractPalettePopup();
        });
    }
    if (chunkWorkersEl) {
        chunkWorkersEl.addEventListener('input', (ev) => {
            _extractPalettePopupState.chunkWorkers = _clampRenderMtWorkerCount(ev.target.value);
            _renderExtractPalettePopup();
        });
    }
    if (runBtn) {
        runBtn.addEventListener('click', async () => {
            const btnText = runBtn.textContent;
            try {
                runBtn.disabled = true;
                runBtn.textContent = 'Executing...';
                const solveScoreThreads = _clampRenderMtThreads(document.getElementById('extract-palette-solve-score-threads').value);
                const histInputMode = _normalizeSolveScoreHistInputMode(document.getElementById('extract-palette-hist-input-mode').value);
                const histRetries = _clampRenderMtRetries(document.getElementById('extract-palette-hist-retries').value);
                const mergeWorkers = _clampRenderMtMergeWorkers(document.getElementById('extract-palette-merge-workers').value);
                const chunkThreads = _clampRenderMtThreads(document.getElementById('extract-palette-chunk-threads').value);
                const chunkInputMode = _normalizeRasterInputMode(document.getElementById('extract-palette-chunk-input-mode').value);
                const chunkRetries = _clampRenderMtRetries(document.getElementById('extract-palette-chunk-retries').value);
                const chunkWorkers = _clampRenderMtWorkerCount(document.getElementById('extract-palette-chunk-workers').value);
                _extractPalettePopupState.solveScoreThreads = solveScoreThreads;
                _extractPalettePopupState.histInputMode = histInputMode;
                _extractPalettePopupState.histRetries = histRetries;
                _extractPalettePopupState.mergeWorkers = mergeWorkers;
                _extractPalettePopupState.chunkThreads = chunkThreads;
                _extractPalettePopupState.chunkInputMode = chunkInputMode;
                _extractPalettePopupState.chunkRetries = chunkRetries;
                _extractPalettePopupState.chunkWorkers = chunkWorkers;
                _closeExtractPalettePopup();
                await runExtractPaletteArtifact({ solveScoreThreads, histInputMode, histRetries, mergeWorkers, chunkThreads, chunkInputMode, chunkRetries, chunkWorkers });
            } finally {
                runBtn.disabled = false;
                runBtn.textContent = btnText;
            }
        });
    }
}

function _initPdfColorSpreadPopup() {
    const runBtn = document.getElementById('pdf-colorspread-popup-run');
    const filterEl = document.getElementById('pdf-colorspread-popup-filter');
    _bindPopupShell({
        overlayId: 'pdf-colorspread-popup-overlay',
        closeId: 'pdf-colorspread-popup-close',
        cancelId: 'pdf-colorspread-popup-cancel',
        isOpen: () => !!_pdfColorSpreadPopupState.open,
        onClose: _closePdfColorSpreadPopup,
        onArrowDown: () => {
            const visible = _visiblePdfColorSpreadCatalog();
            if (!visible.length) return;
            _pdfColorSpreadPopupState.highlightIdx = Math.min((_pdfColorSpreadPopupState.highlightIdx || 0) + 1, visible.length - 1);
            _renderPdfColorSpreadPopup();
        },
        onArrowUp: () => {
            const visible = _visiblePdfColorSpreadCatalog();
            if (!visible.length) return;
            _pdfColorSpreadPopupState.highlightIdx = Math.max((_pdfColorSpreadPopupState.highlightIdx || 0) - 1, 0);
            _renderPdfColorSpreadPopup();
        },
        onEnter: () => {
            const visible = _visiblePdfColorSpreadCatalog();
            if (!visible.length) return;
            runPdfColorSpreadSelected();
        },
    });
    if (runBtn) runBtn.addEventListener('click', runPdfColorSpreadSelected);
    if (filterEl) filterEl.addEventListener('input', (ev) => _applyPdfColorSpreadFilter(ev.target.value));
}

function _closeFunctionPopup() {
    _functionPopupState = { open: false, filter: '', highlightIdx: 0 };
    const overlay = document.getElementById('function-popup-overlay');
    const filter = document.getElementById('function-popup-filter');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
    if (filter) filter.value = '';
}

function _renderFunctionPopup() {
    const overlay = document.getElementById('function-popup-overlay');
    const bodyEl = document.getElementById('function-popup-body');
    const filterEl = document.getElementById('function-popup-filter');
    const summaryEl = document.getElementById('function-popup-summary');
    const chooseBtn = document.getElementById('function-popup-choose');
    if (!overlay || !bodyEl || !filterEl || !summaryEl || !chooseBtn) return;
    if (!_functionPopupState.open) {
        _closeFunctionPopup();
        return;
    }
    const matcher = _functionFilterMatcher(_functionPopupState.filter || '');
    const visible = _visibleFunctionCatalog();
    const highlightIdx = visible.length ? Math.max(0, Math.min(_functionPopupState.highlightIdx || 0, visible.length - 1)) : 0;
    _functionPopupState.highlightIdx = highlightIdx;
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    filterEl.value = _functionPopupState.filter || '';
    bodyEl.replaceChildren();

    if (!visible.length) {
        const row = document.createElement('tr');
        row.className = 'tri-popup-empty';
        const cell = document.createElement('td');
        cell.colSpan = 5;
        cell.textContent = matcher.error
            ? `Invalid coefficient-function regex: ${matcher.error}`
            : 'No coefficient functions match this filter.';
        row.appendChild(cell);
        bodyEl.appendChild(row);
        summaryEl.textContent = matcher.error
            ? 'Fix the regex or clear the filter.'
            : 'Select a coefficient function, then press Choose to use it in Compute.';
        chooseBtn.disabled = true;
        return;
    }

    visible.forEach((entry, idx) => {
        const row = document.createElement('tr');
        row.className = 'tri-popup-row' + (idx === highlightIdx ? ' highlight' : '');
        row.onclick = () => {
            _functionPopupState.highlightIdx = idx;
            _renderFunctionPopup();
        };
        row.ondblclick = () => {
            _functionPopupState.highlightIdx = idx;
            chooseFunctionPopupSelection();
        };

        const nameCell = document.createElement('td');
        const nameWrap = document.createElement('div');
        nameWrap.className = 'tri-popup-name';
        const nameMain = document.createElement('div');
        nameMain.textContent = entry.name;
        const nameMeta = document.createElement('div');
        nameMeta.className = 'tri-popup-aliases';
        nameMeta.textContent = entry.source || '';
        nameWrap.appendChild(nameMain);
        nameWrap.appendChild(nameMeta);
        nameCell.appendChild(nameWrap);

        const degreeCell = document.createElement('td');
        degreeCell.textContent = entry.probe_failed ? '?' : String(entry.degree);

        const kindCell = document.createElement('td');
        kindCell.textContent = entry.kind || '';

        const agreeCell = document.createElement('td');
        agreeCell.textContent = entry.stubbed ? 'stub' : (entry.agreement_pct != null ? `${entry.agreement_pct}%` : '');

        const paramsCell = document.createElement('td');
        paramsCell.textContent = entry.params && entry.params.length ? `${entry.params.length}` : '';

        row.appendChild(nameCell);
        row.appendChild(degreeCell);
        row.appendChild(kindCell);
        row.appendChild(agreeCell);
        row.appendChild(paramsCell);
        bodyEl.appendChild(row);
    });

    const selected = visible[highlightIdx];
    summaryEl.textContent = selected
        ? `Selected: ${selected.name} • ${_coeffFuncMeta(selected)}`
        : 'Select a coefficient function, then press Choose to use it in Compute.';
    chooseBtn.disabled = !selected;
}

function openFunctionPopup() {
    const cat = window._coeffFuncCatalog || [];
    if (!cat.length) return;
    const current = document.getElementById('render-function')?.value || '';
    const currentIdx = cat.findIndex(entry => entry.name === current);
    _functionPopupState = { open: true, filter: '', highlightIdx: currentIdx >= 0 ? currentIdx : 0 };
    _renderFunctionPopup();
    const filter = document.getElementById('function-popup-filter');
    if (filter && typeof filter.focus === 'function') filter.focus();
}

function _applyFunctionFilter(text) {
    _functionPopupState.filter = String(text || '');
    _functionPopupState.highlightIdx = 0;
    _renderFunctionPopup();
}

function chooseFunctionPopupSelection() {
    const visible = _visibleFunctionCatalog();
    if (!visible.length) return;
    const idx = Math.max(0, Math.min(_functionPopupState.highlightIdx || 0, visible.length - 1));
    const entry = visible[idx];
    if (!entry) return;
    _setRenderFunction(entry.name);
    _closeFunctionPopup();
}

function _initFunctionPopup() {
    const chooseBtn = document.getElementById('function-popup-choose');
    const filterEl = document.getElementById('function-popup-filter');
    _bindPopupShell({
        overlayId: 'function-popup-overlay',
        closeId: 'function-popup-close',
        cancelId: 'function-popup-cancel',
        isOpen: () => !!_functionPopupState.open,
        onClose: _closeFunctionPopup,
        onArrowDown: () => {
            const visible = _visibleFunctionCatalog();
            if (!visible.length) return;
            _functionPopupState.highlightIdx = Math.min((_functionPopupState.highlightIdx || 0) + 1, visible.length - 1);
            _renderFunctionPopup();
        },
        onArrowUp: () => {
            const visible = _visibleFunctionCatalog();
            if (!visible.length) return;
            _functionPopupState.highlightIdx = Math.max((_functionPopupState.highlightIdx || 0) - 1, 0);
            _renderFunctionPopup();
        },
        onEnter: chooseFunctionPopupSelection,
    });
    if (chooseBtn) chooseBtn.addEventListener('click', chooseFunctionPopupSelection);
    if (filterEl) filterEl.addEventListener('input', (ev) => _applyFunctionFilter(ev.target.value));
}

function _defaultAutolevelParams() {
    return {
        bins: 256,
        enable_levels: true,
        clip_low: 0.0,
        clip_high: 1.0,
        enable_peak_limit: false,
        peak_factor: 0.0,
        enable_gamma: false,
        gamma: 1.0,
        enable_auto_gamma: false,
        auto_gamma: 'median',
        target: 0.5,
        enable_sigmoid: false,
        sigmoid_strength: 0.0,
        sigmoid_mid: 0.5,
        enable_vibrance: false,
        vibrance: 0.0,
        enable_pooled_rgb: true,
        pooled_rgb: 0.1,
        quality: 90,
        jpeg_subsample_mode: 'on',
        jpeg_optimize_coding: false,
        jpeg_interlace: false,
        exclude_background: true,
        background_threshold: null,
    };
}

function _autolevelBackgroundInfo(art) {
    const bg = String((art && art.background_color) || '000000').trim() || '000000';
    const hex = bg.startsWith('#') ? bg.toLowerCase() : ('#' + bg.toLowerCase());
    const thresholdNum = Number(art && art.background_threshold);
    const threshold = Number.isFinite(thresholdNum) ? Math.max(0, Math.round(thresholdNum)) : 4;
    return { color: hex, threshold, label: `${hex} (tol ${threshold})` };
}

function _autolevelBaseParamsForArtifact(art) {
    const params = _defaultAutolevelParams();
    const quality = Number(art && art.quality);
    if (Number.isFinite(quality) && quality > 0) params.quality = Math.round(quality);
    const bgInfo = _autolevelBackgroundInfo(art);
    params.background_threshold = bgInfo.threshold;
    return params;
}

function _autolevelNormalizeStageFlags(params) {
    const out = { ...(params || {}) };
    const rawAutoGamma = String(out.auto_gamma || 'none').trim().toLowerCase();
    out.enable_levels = out.enable_levels !== false;
    out.enable_peak_limit = out.enable_peak_limit != null ? !!out.enable_peak_limit : Math.abs(Number(out.peak_factor || 0)) > 1e-12;
    out.enable_auto_gamma = out.enable_auto_gamma != null ? !!out.enable_auto_gamma : rawAutoGamma !== 'none';
    out.auto_gamma = rawAutoGamma === 'median' ? 'median' : 'median';
    out.enable_gamma = out.enable_gamma != null ? !!out.enable_gamma : (!out.enable_auto_gamma && Math.abs(Number((out.gamma == null ? 1.0 : out.gamma)) - 1.0) > 1e-12);
    out.enable_sigmoid = out.enable_sigmoid != null ? !!out.enable_sigmoid : Math.abs(Number(out.sigmoid_strength || 0)) > 1e-12;
    out.enable_vibrance = out.enable_vibrance != null ? !!out.enable_vibrance : Math.abs(Number(out.vibrance || 0)) > 1e-12;
    out.enable_pooled_rgb = out.enable_pooled_rgb != null ? !!out.enable_pooled_rgb : Number(out.pooled_rgb || 0) > 0;
    return out;
}

function _autolevelParamsForArtifact(art) {
    const params = _autolevelBaseParamsForArtifact(art);
    if (art && art.autolevels_params && typeof art.autolevels_params === 'object') {
        Object.assign(params, art.autolevels_params);
    }
    const normalized = _autolevelNormalizeStageFlags(params);
    const bgInfo = _autolevelBackgroundInfo(art);
    const rawThreshold = normalized.background_threshold;
    const threshold = Number(rawThreshold);
    normalized.background_threshold = (rawThreshold === null || rawThreshold === '' || rawThreshold === undefined || !Number.isFinite(threshold))
        ? bgInfo.threshold
        : Math.max(0, Math.round(threshold));
    return normalized;
}

function _autolevelPopupSourceArtifact() {
    const families = (window._lastRenderSummary && window._lastRenderSummary.families) || {};
    const colorInv = families.color || [];
    const targetId = (_autolevelPopupState && _autolevelPopupState.sourceArtifactId) || '';
    if (targetId) {
        const hit = colorInv.find((art) => art && art.artifact_id === targetId);
        if (hit) return hit;
    }
    return _renderSelectedArtifactEntry();
}

function _syncAutolevelBackgroundControls() {
    const enabled = !!(document.getElementById('autolevel-exclude-background') && document.getElementById('autolevel-exclude-background').checked);
    const thresholdEl = document.getElementById('autolevel-background-threshold');
    if (thresholdEl) thresholdEl.disabled = !enabled;
}

function _setAutolevelStageDisabled(bodyId, disabled) {
    const body = document.getElementById(bodyId);
    if (body) body.classList.toggle('disabled', !!disabled);
}

function _syncAutolevelStageControls() {
    const readChk = (id) => !!(document.getElementById(id) && document.getElementById(id).checked);
    const setDisabled = (ids, disabled) => ids.forEach((id) => {
        const el = document.getElementById(id);
        if (el) el.disabled = !!disabled;
    });
    const levelsOn = readChk('autolevel-enable-levels');
    const peakOn = readChk('autolevel-enable-peak-limit');
    const autoGammaOn = readChk('autolevel-enable-auto-gamma');
    const gammaToggleOn = readChk('autolevel-enable-gamma');
    const gammaInputOn = gammaToggleOn && !autoGammaOn;
    const sigmoidOn = readChk('autolevel-enable-sigmoid');
    const vibranceOn = readChk('autolevel-enable-vibrance');
    const pooledOn = readChk('autolevel-enable-pooled-rgb');
    setDisabled(['autolevel-clip-low', 'autolevel-clip-high'], !levelsOn);
    setDisabled(['autolevel-peak-factor'], !peakOn);
    setDisabled(['autolevel-gamma'], !gammaInputOn);
    setDisabled(['autolevel-target'], !autoGammaOn);
    setDisabled(['autolevel-sigmoid-strength', 'autolevel-sigmoid-mid'], !sigmoidOn);
    setDisabled(['autolevel-vibrance'], !vibranceOn);
    setDisabled(['autolevel-pooled-rgb'], !pooledOn);
    _setAutolevelStageDisabled('autolevel-stage-levels', !levelsOn);
    _setAutolevelStageDisabled('autolevel-stage-peak', !peakOn);
    _setAutolevelStageDisabled('autolevel-stage-gamma', !gammaInputOn);
    _setAutolevelStageDisabled('autolevel-stage-auto-gamma', !autoGammaOn);
    _setAutolevelStageDisabled('autolevel-stage-sigmoid', !sigmoidOn);
    _setAutolevelStageDisabled('autolevel-stage-vibrance', !vibranceOn);
    _setAutolevelStageDisabled('autolevel-stage-pooled', !pooledOn);
    _syncAutolevelBackgroundControls();
}

function _writeAutolevelPopupParams(art, params) {
    params = _autolevelNormalizeStageFlags(params || {});
    const fmt = (art && (art.format || (art.image_key || '').split('.').pop())) || 'jpeg';
    const dims = art && art.width && art.height ? `${art.width}x${art.height}` : '';
    const bgInfo = _autolevelBackgroundInfo(art);
    const summaryEl = document.getElementById('autolevel-popup-summary');
    if (summaryEl) {
        const parts = [
            art && art.artifact_id ? art.artifact_id : 'selected artifact',
            fmt ? `format=${fmt}` : '',
            dims ? `dims=${dims}` : '',
            `bg=${bgInfo.color}`,
            `tol/ch=${Number(params.background_threshold)}`,
        ].filter(Boolean);
        summaryEl.textContent = parts.join(' · ');
    }
    const setValue = (id, value) => {
        const text = String(value);
        const el = document.getElementById(id);
        if (el) el.value = text;
        const displayEl = document.getElementById(`${id}-display`);
        if (displayEl) displayEl.textContent = text;
    };
    const setChecked = (id, value) => {
        const el = document.getElementById(id);
        if (el) el.checked = !!value;
    };
    setValue('autolevel-bins', params.bins);
    setChecked('autolevel-enable-levels', params.enable_levels !== false);
    setValue('autolevel-clip-low', params.clip_low);
    setValue('autolevel-clip-high', params.clip_high);
    setChecked('autolevel-enable-peak-limit', params.enable_peak_limit);
    setValue('autolevel-peak-factor', params.peak_factor);
    setChecked('autolevel-enable-gamma', params.enable_gamma);
    setValue('autolevel-gamma', params.gamma);
    setChecked('autolevel-enable-auto-gamma', params.enable_auto_gamma);
    setValue('autolevel-target', params.target);
    setChecked('autolevel-enable-sigmoid', params.enable_sigmoid);
    setValue('autolevel-sigmoid-strength', params.sigmoid_strength);
    setValue('autolevel-sigmoid-mid', params.sigmoid_mid);
    setChecked('autolevel-enable-vibrance', params.enable_vibrance);
    setValue('autolevel-vibrance', params.vibrance);
    setChecked('autolevel-enable-pooled-rgb', params.enable_pooled_rgb !== false);
    setValue('autolevel-pooled-rgb', params.pooled_rgb);
    setValue('autolevel-quality', params.quality);
    setValue('autolevel-jpeg-subsample', params.jpeg_subsample_mode || 'on');
    setValue('autolevel-background-readout', bgInfo.color);
    setValue('autolevel-background-threshold', params.background_threshold);
    setChecked('autolevel-exclude-background', params.exclude_background !== false);
    setChecked('autolevel-jpeg-optimize', params.jpeg_optimize_coding);
    setChecked('autolevel-jpeg-interlace', params.jpeg_interlace);
    _syncAutolevelStageControls();
}

function _readAutolevelPopupParams() {
    const readNum = (id, fallback) => {
        const el = document.getElementById(id);
        const value = Number(el && el.value);
        return Number.isFinite(value) ? value : fallback;
    };
    const readSel = (id, fallback) => {
        const el = document.getElementById(id);
        return (el && el.value) ? String(el.value) : fallback;
    };
    const readChk = (id) => {
        const el = document.getElementById(id);
        return !!(el && el.checked);
    };
    return {
        bins: 256,
        enable_levels: readChk('autolevel-enable-levels'),
        clip_low: readNum('autolevel-clip-low', 0.0),
        clip_high: readNum('autolevel-clip-high', 1.0),
        enable_peak_limit: readChk('autolevel-enable-peak-limit'),
        peak_factor: readNum('autolevel-peak-factor', 0.0),
        enable_gamma: readChk('autolevel-enable-gamma'),
        gamma: readNum('autolevel-gamma', 1.0),
        enable_auto_gamma: readChk('autolevel-enable-auto-gamma'),
        auto_gamma: 'median',
        target: readNum('autolevel-target', 0.5),
        enable_sigmoid: readChk('autolevel-enable-sigmoid'),
        sigmoid_strength: readNum('autolevel-sigmoid-strength', 0.0),
        sigmoid_mid: readNum('autolevel-sigmoid-mid', 0.5),
        enable_vibrance: readChk('autolevel-enable-vibrance'),
        vibrance: readNum('autolevel-vibrance', 0.0),
        enable_pooled_rgb: readChk('autolevel-enable-pooled-rgb'),
        pooled_rgb: readNum('autolevel-pooled-rgb', 0.1),
        quality: Math.round(readNum('autolevel-quality', 90)),
        jpeg_subsample_mode: readSel('autolevel-jpeg-subsample', 'on'),
        exclude_background: readChk('autolevel-exclude-background'),
        background_threshold: Math.max(0, Math.round(readNum('autolevel-background-threshold', 4))),
        jpeg_optimize_coding: readChk('autolevel-jpeg-optimize'),
        jpeg_interlace: readChk('autolevel-jpeg-interlace'),
    };
}

function _closeAutolevelPopup() {
    _autolevelPopupState = { open: false, sourceArtifactId: '', sourceImageKey: '' };
    const overlay = document.getElementById('autolevel-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _revertAutolevelPopupDefaults() {
    const art = _autolevelPopupSourceArtifact();
    if (!art) return;
    _writeAutolevelPopupParams(art, _autolevelBaseParamsForArtifact(art));
}

function openAutolevelPopup() {
    if (_renderActiveFamily !== 'color') return;
    const art = _renderSelectedArtifactEntry();
    if (!art || !art.image_key) return;
    _autolevelPopupState = {
        open: true,
        sourceArtifactId: art.artifact_id || '',
        sourceImageKey: art.image_key || '',
    };
    _writeAutolevelPopupParams(art, _autolevelParamsForArtifact(art));
    const overlay = document.getElementById('autolevel-popup-overlay');
    if (overlay) {
        overlay.style.display = 'flex';
        overlay.setAttribute('aria-hidden', 'false');
    }
    const qualityEl = document.getElementById('autolevel-quality');
    if (qualityEl && typeof qualityEl.focus === 'function') qualityEl.focus();
}

async function runAutolevelSelectedRenderArtifact() {
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    const statusEl = document.getElementById('render-status');
    const btn = document.getElementById('autolevel-popup-run');

    if (_renderActiveFamily !== 'color') return;
    if (!jobId || !art || !art.image_key) return;
    if (_activeRenderRun) {
        log('Autolevels: render in progress, wait for completion', 'err', 'render-log');
        return;
    }

    const origText = btn ? btn.textContent : 'AutoLevel';
    if (btn) { btn.disabled = true; btn.textContent = 'Dispatching...'; }
    try {
        const params = _readAutolevelPopupParams();
        const runId = _generateRunId();
        const taskId = 'autolevels_' + runId;
        const artifactId = 'autolevels_' + Date.now() + '_' + Math.random().toString(36).slice(2, 8);

        statusEl.textContent = 'Dispatching autolevels...';
        statusEl.className = 'status';
        log('Autolevels: dispatching for ' + (art.artifact_id || 'selected color artifact') + '...', 'ok', 'render-log');

        const dispResult = await lambdaPost('dispatch', {
            target: 'autolevels',
            jobs: [{
                job_id: jobId,
                task_id: taskId,
                artifact_id: artifactId,
                source_artifact_id: _autolevelPopupState.sourceArtifactId || art.artifact_id || '',
                source_image_key: _autolevelPopupState.sourceImageKey || art.image_key,
                autolevels_params: params,
            }],
            expected_keys: [],
        });
        if ((dispResult.fired || 0) !== 1) throw new Error('Autolevel dispatch failed');

        _saveActiveRun({
            job_id: jobId,
            mode: 'autolevels',
            run_id: runId,
            task_id: taskId,
            started_at_ms: Date.now(),
        });
        _closeAutolevelPopup();
        log('  autolevels dispatched: ' + artifactId, 'ok', 'render-log');
        startActiveRenderObserver();
    } catch (e) {
        statusEl.textContent = 'Autolevels failed: ' + e.message;
        statusEl.className = 'status error';
        log('Autolevels failed: ' + e.message, 'err', 'render-log');
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = origText; }
    }
}

function _initAutolevelPopup() {
    const runBtn = document.getElementById('autolevel-popup-run');
    const revertBtn = document.getElementById('autolevel-popup-revert');
    _bindPopupShell({
        overlayId: 'autolevel-popup-overlay',
        closeId: 'autolevel-popup-close',
        cancelId: 'autolevel-popup-cancel',
        isOpen: () => !!_autolevelPopupState.open,
        onClose: _closeAutolevelPopup,
    });
    if (runBtn) runBtn.addEventListener('click', runAutolevelSelectedRenderArtifact);
    if (revertBtn) revertBtn.addEventListener('click', _revertAutolevelPopupDefaults);
    const excludeBg = document.getElementById('autolevel-exclude-background');
    if (excludeBg) excludeBg.addEventListener('change', _syncAutolevelBackgroundControls);
    ['autolevel-enable-levels', 'autolevel-enable-peak-limit', 'autolevel-enable-gamma', 'autolevel-enable-auto-gamma',
     'autolevel-enable-sigmoid', 'autolevel-enable-vibrance', 'autolevel-enable-pooled-rgb']
        .forEach((id) => {
            const el = document.getElementById(id);
            if (el) el.addEventListener('change', _syncAutolevelStageControls);
        });
}

function _resizeBaseParamsForArtifact(art) {
    const width = Number(art && art.width);
    const height = Number(art && art.height);
    const pix = Number(art && art.pix);
    const currentSize = Number.isFinite(width) && Number.isFinite(height) && width > 0 && height > 0
        ? Math.max(width, height)
        : (Number.isFinite(pix) && pix > 0 ? pix : 1024);
    const fmt = String((art && art.format) || ((art && art.image_key || '').split('.').pop()) || 'jpeg').toLowerCase() === 'png' ? 'png' : 'jpeg';
    const quality = Number(art && art.quality);
    return {
        engine: 'thumbnail',
        target_size: Math.round(currentSize),
        size_mode: 'both',
        linear: false,
        no_rotate: false,
        input_profile: '',
        output_profile: '',
        intent: 'perceptual',
        fail_on: 'none',
        kernel: 'lanczos3',
        gap: 2,
        format: fmt,
        quality: Number.isFinite(quality) && quality > 0 ? Math.round(quality) : 90,
        jpeg_subsample_mode: 'on',
        jpeg_optimize_coding: false,
        jpeg_interlace: false,
        png_compression: 6,
        png_interlace: false,
        png_palette: false,
        png_Q: 100,
        png_dither: 1,
        png_bitdepth: 8,
        png_effort: 7,
    };
}

function _resizeParamsForArtifact(art) {
    const params = _resizeBaseParamsForArtifact(art);
    if (art && art.resize_params && typeof art.resize_params === 'object') Object.assign(params, art.resize_params);
    return params;
}

function _resizePopupSourceArtifact() {
    const families = (window._lastRenderSummary && window._lastRenderSummary.families) || {};
    const colorInv = families.color || [];
    const targetId = (_resizePopupState && _resizePopupState.sourceArtifactId) || '';
    if (targetId) {
        const hit = colorInv.find((art) => art && art.artifact_id === targetId);
        if (hit) return hit;
    }
    return _renderSelectedArtifactEntry();
}

function _setResizeStageDisabled(bodyId, disabled) {
    const body = document.getElementById(bodyId);
    if (body) body.classList.toggle('disabled', !!disabled);
}

function _syncResizePopupControls() {
    const engine = (document.getElementById('resize-engine') && document.getElementById('resize-engine').value) || 'thumbnail';
    const fmt = (document.getElementById('resize-output-format') && document.getElementById('resize-output-format').value) || 'jpeg';
    const setDisabled = (ids, disabled) => ids.forEach((id) => {
        const el = document.getElementById(id);
        if (el) el.disabled = !!disabled;
    });
    const thumbnailIds = ['resize-size-mode', 'resize-linear', 'resize-no-rotate', 'resize-input-profile', 'resize-output-profile', 'resize-intent', 'resize-fail-on'];
    const resizeIds = ['resize-kernel', 'resize-gap'];
    const jpegIds = ['resize-quality', 'resize-jpeg-subsample', 'resize-jpeg-optimize', 'resize-jpeg-interlace'];
    const pngIds = ['resize-png-compression', 'resize-png-q', 'resize-png-dither', 'resize-png-bitdepth', 'resize-png-effort', 'resize-png-interlace', 'resize-png-palette'];
    setDisabled(thumbnailIds, engine !== 'thumbnail');
    setDisabled(resizeIds, engine !== 'resize');
    setDisabled(jpegIds, fmt !== 'jpeg');
    setDisabled(pngIds, fmt !== 'png');
    _setResizeStageDisabled('resize-stage-thumbnail', engine !== 'thumbnail');
    _setResizeStageDisabled('resize-stage-resize', engine !== 'resize');
    const pngRow = document.getElementById('resize-png-row');
    if (pngRow) pngRow.style.display = fmt === 'png' ? '' : 'none';
}

function _writeResizePopupParams(art, params) {
    const fmt = String((params && params.format) || ((art && art.format) || 'jpeg')).toLowerCase() === 'png' ? 'png' : 'jpeg';
    const width = Number(art && art.width);
    const height = Number(art && art.height);
    const currentSizeText = Number.isFinite(width) && Number.isFinite(height) && width > 0 && height > 0
        ? (width === height ? `${Math.round(width)}` : `${Math.round(width)}x${Math.round(height)}`)
        : String(Math.round(Number(params && params.target_size) || 0));
    const summaryEl = document.getElementById('resize-popup-summary');
    if (summaryEl) {
        const parts = [
            art && art.artifact_id ? art.artifact_id : 'selected artifact',
            fmt ? `format=${fmt}` : '',
            `current=${currentSizeText}`,
            `target=${Math.round(Number(params && params.target_size) || 0)}`,
            `engine=${params.engine || 'thumbnail'}`,
        ].filter(Boolean);
        summaryEl.textContent = parts.join(' · ');
    }
    const setValue = (id, value) => {
        const text = String(value);
        const el = document.getElementById(id);
        if (el) el.value = text;
        const displayEl = document.getElementById(`${id}-display`);
        if (displayEl) displayEl.textContent = text;
    };
    const setChecked = (id, value) => {
        const el = document.getElementById(id);
        if (el) el.checked = !!value;
    };
    setValue('resize-current-size', currentSizeText);
    setValue('resize-target-size', params.target_size);
    setValue('resize-engine', params.engine || 'thumbnail');
    setValue('resize-size-mode', params.size_mode || 'both');
    setChecked('resize-linear', !!params.linear);
    setChecked('resize-no-rotate', !!params.no_rotate);
    setValue('resize-input-profile', params.input_profile || '');
    setValue('resize-output-profile', params.output_profile || '');
    setValue('resize-intent', params.intent || 'perceptual');
    setValue('resize-fail-on', params.fail_on || 'none');
    setValue('resize-kernel', params.kernel || 'lanczos3');
    setValue('resize-gap', params.gap == null ? 2 : params.gap);
    setValue('resize-output-format', fmt);
    setValue('resize-quality', params.quality == null ? 90 : params.quality);
    setValue('resize-jpeg-subsample', params.jpeg_subsample_mode || 'on');
    setChecked('resize-jpeg-optimize', !!params.jpeg_optimize_coding);
    setChecked('resize-jpeg-interlace', !!params.jpeg_interlace);
    setValue('resize-png-compression', params.png_compression == null ? 6 : params.png_compression);
    setValue('resize-png-q', params.png_Q == null ? 100 : params.png_Q);
    setValue('resize-png-dither', params.png_dither == null ? 1 : params.png_dither);
    setValue('resize-png-bitdepth', params.png_bitdepth == null ? 8 : params.png_bitdepth);
    setValue('resize-png-effort', params.png_effort == null ? 7 : params.png_effort);
    setChecked('resize-png-interlace', !!params.png_interlace);
    setChecked('resize-png-palette', !!params.png_palette);
    _syncResizePopupControls();
}

function _readResizePopupParams() {
    const readNum = (id, fallback) => {
        const el = document.getElementById(id);
        const value = Number(el && el.value);
        return Number.isFinite(value) ? value : fallback;
    };
    const readSel = (id, fallback) => {
        const el = document.getElementById(id);
        return (el && el.value) ? String(el.value) : fallback;
    };
    const readChk = (id) => {
        const el = document.getElementById(id);
        return !!(el && el.checked);
    };
    return {
        engine: readSel('resize-engine', 'thumbnail'),
        target_size: Math.max(1, Math.round(readNum('resize-target-size', 1024))),
        size_mode: readSel('resize-size-mode', 'both'),
        linear: readChk('resize-linear'),
        no_rotate: readChk('resize-no-rotate'),
        input_profile: readSel('resize-input-profile', '').trim(),
        output_profile: readSel('resize-output-profile', '').trim(),
        intent: readSel('resize-intent', 'perceptual'),
        fail_on: readSel('resize-fail-on', 'none'),
        kernel: readSel('resize-kernel', 'lanczos3'),
        gap: readNum('resize-gap', 2),
        format: readSel('resize-output-format', 'jpeg'),
        quality: Math.round(readNum('resize-quality', 90)),
        jpeg_subsample_mode: readSel('resize-jpeg-subsample', 'on'),
        jpeg_optimize_coding: readChk('resize-jpeg-optimize'),
        jpeg_interlace: readChk('resize-jpeg-interlace'),
        png_compression: Math.round(readNum('resize-png-compression', 6)),
        png_Q: Math.round(readNum('resize-png-q', 100)),
        png_dither: readNum('resize-png-dither', 1),
        png_bitdepth: Math.round(readNum('resize-png-bitdepth', 8)),
        png_effort: Math.round(readNum('resize-png-effort', 7)),
        png_interlace: readChk('resize-png-interlace'),
        png_palette: readChk('resize-png-palette'),
    };
}

function _closeResizePopup() {
    _resizePopupState = { open: false, sourceArtifactId: '', sourceImageKey: '' };
    const overlay = document.getElementById('resize-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _revertResizePopupDefaults() {
    const art = _resizePopupSourceArtifact();
    if (!art) return;
    _writeResizePopupParams(art, _resizeBaseParamsForArtifact(art));
}

function openResizePopup() {
    if (_renderActiveFamily !== 'color') return;
    const art = _renderSelectedArtifactEntry();
    if (!art || !art.image_key) return;
    _resizePopupState = {
        open: true,
        sourceArtifactId: art.artifact_id || '',
        sourceImageKey: art.image_key || '',
    };
    _writeResizePopupParams(art, _resizeParamsForArtifact(art));
    const overlay = document.getElementById('resize-popup-overlay');
    if (overlay) {
        overlay.style.display = 'flex';
        overlay.setAttribute('aria-hidden', 'false');
    }
    const targetEl = document.getElementById('resize-target-size');
    if (targetEl && typeof targetEl.focus === 'function') targetEl.focus();
}

async function runResizeSelectedRenderArtifact() {
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    const statusEl = document.getElementById('render-status');
    const btn = document.getElementById('resize-popup-run');

    if (_renderActiveFamily !== 'color') return;
    if (!jobId || !art || !art.image_key) return;
    if (_activeRenderRun) {
        log('Resize: render in progress, wait for completion', 'err', 'render-log');
        return;
    }

    const origText = btn ? btn.textContent : 'Resize';
    if (btn) { btn.disabled = true; btn.textContent = 'Dispatching...'; }
    try {
        const params = _readResizePopupParams();
        const runId = _generateRunId();
        const taskId = 'resize_artifact_' + runId;
        const artifactId = 'resize_' + Date.now() + '_' + Math.random().toString(36).slice(2, 8);

        statusEl.textContent = 'Dispatching resize...';
        statusEl.className = 'status';
        log('Resize: dispatching for ' + (art.artifact_id || 'selected color artifact') + '...', 'ok', 'render-log');

        const dispResult = await lambdaPost('dispatch', {
            target: 'resize_artifact',
            jobs: [{
                job_id: jobId,
                task_id: taskId,
                artifact_id: artifactId,
                source_artifact_id: _resizePopupState.sourceArtifactId || art.artifact_id || '',
                source_image_key: _resizePopupState.sourceImageKey || art.image_key,
                resize_params: params,
            }],
            expected_keys: [],
        });
        if ((dispResult.fired || 0) !== 1) throw new Error('Resize dispatch failed');

        _saveActiveRun({
            job_id: jobId,
            mode: 'resize',
            run_id: runId,
            task_id: taskId,
            started_at_ms: Date.now(),
        });
        _closeResizePopup();
        log('  resize dispatched: ' + artifactId, 'ok', 'render-log');
        startActiveRenderObserver();
    } catch (e) {
        statusEl.textContent = 'Resize failed: ' + e.message;
        statusEl.className = 'status error';
        log('Resize failed: ' + e.message, 'err', 'render-log');
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = origText; }
    }
}

function _initResizePopup() {
    const runBtn = document.getElementById('resize-popup-run');
    const revertBtn = document.getElementById('resize-popup-revert');
    const engineEl = document.getElementById('resize-engine');
    _bindPopupShell({
        overlayId: 'resize-popup-overlay',
        closeId: 'resize-popup-close',
        cancelId: 'resize-popup-cancel',
        isOpen: () => !!_resizePopupState.open,
        onClose: _closeResizePopup,
    });
    if (runBtn) runBtn.addEventListener('click', runResizeSelectedRenderArtifact);
    if (revertBtn) revertBtn.addEventListener('click', _revertResizePopupDefaults);
    if (engineEl) engineEl.addEventListener('change', _syncResizePopupControls);
}

function _initBilevelPopup() {
    const runBtn = document.getElementById('bilevel-popup-run');
    const modeEl = document.getElementById('bilevel-section-mode');
    _bindPopupShell({
        overlayId: 'bilevel-popup-overlay',
        closeId: 'bilevel-popup-close',
        cancelId: 'bilevel-popup-cancel',
        isOpen: () => !!_bilevelPopupState.open,
        onClose: _closeBilevelPopup,
    });
    if (runBtn) {
        runBtn.addEventListener('click', async () => {
            await runBilevelPipeline({
                sectionMode: (document.getElementById('bilevel-section-mode') || {}).value || 'logical_sections_auto',
                sectionCount: (document.getElementById('bilevel-section-count') || {}).value || '',
            });
        });
    }
    if (modeEl) modeEl.addEventListener('change', _syncBilevelPopupControls);
}

function _initColorToBilevelPopup() {
    const runBtn = document.getElementById('color-to-bilevel-popup-run');
    _bindPopupShell({
        overlayId: 'color-to-bilevel-popup-overlay',
        closeId: 'color-to-bilevel-popup-close',
        cancelId: 'color-to-bilevel-popup-cancel',
        isOpen: () => !!_colorToBilevelPopupState.open,
        onClose: _closeColorToBilevelPopup,
    });
    if (runBtn) runBtn.addEventListener('click', runColorToBilevelSelectedArtifact);
}

function _initSolveScoreProgramModal() {
    const nameEl = document.getElementById('solve-score-modal-name');
    const loadBtn = document.getElementById('solve-score-modal-load');
    const saveBtn = document.getElementById('solve-score-modal-save');
    const deleteBtn = document.getElementById('solve-score-modal-delete');
    const downloadBtn = document.getElementById('solve-score-modal-download');
    const uploadBtn = document.getElementById('solve-score-modal-upload');
    const uploadFileEl = document.getElementById('solve-score-modal-upload-file');
    _bindPopupShell({
        overlayId: 'solve-score-modal-overlay',
        closeId: 'solve-score-modal-close',
        cancelId: 'solve-score-modal-cancel',
        isOpen: () => !!_solveScoreModalState.open,
        onClose: _closeSolveScoreProgramModal,
        onEnter: () => {
            if (!_solveScoreModalState.open || _solveScoreModalState.actionBusy) return;
            if (_solveScoreModalState.selectedId && !_solveScoreModalState.selectedLoading) {
                void _loadSelectedSolveScoreProgramFromModal();
            }
        },
    });
    if (nameEl) {
        nameEl.addEventListener('input', (ev) => {
            _solveScoreModalState.nameInput = String(ev.target && ev.target.value || '');
            _solveScoreProgramRememberedNames[_solveScoreModalState.prefix] = _solveScoreModalState.nameInput;
            _renderSolveScoreProgramModal();
        });
    }
    if (loadBtn) loadBtn.addEventListener('click', () => { void _loadSelectedSolveScoreProgramFromModal(); });
    if (saveBtn) saveBtn.addEventListener('click', () => { void _saveCurrentSolveScoreProgramFromModal(); });
    if (deleteBtn) deleteBtn.addEventListener('click', () => { void _deleteSelectedSolveScoreProgramFromModal(); });
    if (downloadBtn) downloadBtn.addEventListener('click', _downloadCurrentSolveScoreProgramFromModal);
    if (uploadBtn) uploadBtn.addEventListener('click', _triggerSolveScoreProgramUploadFromModal);
    if (uploadFileEl) {
        uploadFileEl.addEventListener('change', async (ev) => {
            const file = ev.target && ev.target.files && ev.target.files[0];
            if (!file) return;
            await _importSolveScoreProgramFileFromModal(file);
        });
    }
}

function _initParamProgramModal() {
    const nameEl = document.getElementById('param-program-modal-name');
    const loadBtn = document.getElementById('param-program-modal-load');
    const saveBtn = document.getElementById('param-program-modal-save');
    const deleteBtn = document.getElementById('param-program-modal-delete');
    const downloadBtn = document.getElementById('param-program-modal-download');
    const uploadBtn = document.getElementById('param-program-modal-upload');
    const uploadFileEl = document.getElementById('param-program-modal-upload-file');
    _bindPopupShell({
        overlayId: 'param-program-modal-overlay',
        closeId: 'param-program-modal-close',
        cancelId: 'param-program-modal-cancel',
        isOpen: () => !!_paramProgramModalState.open,
        onClose: _closeParamProgramModal,
        onEnter: () => {
            if (!_paramProgramModalState.open || _paramProgramModalState.actionBusy) return;
            if (_paramProgramModalState.selectedId && !_paramProgramModalState.selectedLoading) {
                void _loadSelectedParamProgramFromModal();
            }
        },
    });
    if (nameEl) {
        nameEl.addEventListener('input', (ev) => {
            _paramProgramModalState.nameInput = String(ev.target && ev.target.value || '');
            _renderParamProgramModal();
        });
    }
    if (loadBtn) loadBtn.addEventListener('click', () => { void _loadSelectedParamProgramFromModal(); });
    if (saveBtn) saveBtn.addEventListener('click', () => { void _saveCurrentParamProgramFromModal(); });
    if (deleteBtn) deleteBtn.addEventListener('click', () => { void _deleteSelectedParamProgramFromModal(); });
    if (downloadBtn) downloadBtn.addEventListener('click', _downloadCurrentParamProgramFromModal);
    if (uploadBtn) uploadBtn.addEventListener('click', _triggerParamProgramUploadFromModal);
    if (uploadFileEl) {
        uploadFileEl.addEventListener('change', async (ev) => {
            const file = ev.target && ev.target.files && ev.target.files[0];
            if (!file) return;
            await _importParamProgramFileFromModal(file);
        });
    }
}

function _initCoeffProgramModal() {
    const nameEl = document.getElementById('coeff-program-modal-name');
    const loadBtn = document.getElementById('coeff-program-modal-load');
    const saveBtn = document.getElementById('coeff-program-modal-save');
    const deleteBtn = document.getElementById('coeff-program-modal-delete');
    const downloadBtn = document.getElementById('coeff-program-modal-download');
    const uploadBtn = document.getElementById('coeff-program-modal-upload');
    const uploadFileEl = document.getElementById('coeff-program-modal-upload-file');
    _bindPopupShell({
        overlayId: 'coeff-program-modal-overlay',
        closeId: 'coeff-program-modal-close',
        cancelId: 'coeff-program-modal-cancel',
        isOpen: () => !!_coeffProgramModalState.open,
        onClose: _closeCoeffProgramModal,
        onEnter: () => {
            if (!_coeffProgramModalState.open || _coeffProgramModalState.actionBusy) return;
            if (_coeffProgramModalState.selectedId && !_coeffProgramModalState.selectedLoading) {
                void _loadSelectedCoeffProgramFromModal();
            }
        },
    });
    if (nameEl) {
        nameEl.addEventListener('input', (ev) => {
            _coeffProgramModalState.nameInput = String(ev.target && ev.target.value || '');
            _renderCoeffProgramModal();
        });
    }
    if (loadBtn) loadBtn.addEventListener('click', () => { void _loadSelectedCoeffProgramFromModal(); });
    if (saveBtn) saveBtn.addEventListener('click', () => { void _saveCurrentCoeffProgramFromModal(); });
    if (deleteBtn) deleteBtn.addEventListener('click', () => { void _deleteSelectedCoeffProgramFromModal(); });
    if (downloadBtn) downloadBtn.addEventListener('click', _downloadCurrentCoeffProgramFromModal);
    if (uploadBtn) uploadBtn.addEventListener('click', _triggerCoeffProgramUploadFromModal);
    if (uploadFileEl) {
        uploadFileEl.addEventListener('change', async (ev) => {
            const file = ev.target && ev.target.files && ev.target.files[0];
            if (!file) return;
            await _importCoeffProgramFileFromModal(file);
        });
    }
}

function buildPaletteCircles(containerId, mode, getCurrentPalette) {
    _syncTriDefaults();
    _renderPaletteRow(mode);
}
buildPaletteCircles('palette-circles-root-proximity', 'proximity', () => renderRootProximityPalette);
buildPaletteCircles('palette-circles-solve-score', 'solve_score', () => renderSolveScorePalette);
buildPaletteCircles('palette-circles-palette-tab', 'palette_tab', () => paletteTabPalette);
_initTriPalettePopup();
_initBuiltinPalettePopup();
_initLongPalettePopup();
_initFunctionPopup();
_initAutolevelPopup();
_initResizePopup();
_initRepalettePopup();
_initColorRepalettePopup();
_initBilevelPopup();
_initColorToBilevelPopup();
_initResultsRefreshPopup();
_initRenderMtPopup();
_initComputeMtPopup();
_initExtractPalettePopup();
_initPdfColorSpreadPopup();
_initSolveScoreProgramModal();
_initParamProgramModal();
_initCoeffProgramModal();
_updateSolveScoreButtons();  // initial disabled state
_clearPaletteCanvas('No palette selected');

/* ---- Render ---- */
/* ---- Chip-based transform chains ---- */
// params: array of {placeholder, defaultValue} — one input per param. Empty = no params.
const _ptCatalog = {
    unit_circle: { params: [{ph:'target', def:'both', target: true}] },
    rtheta: { params: [{ph:'p', def:'1'}, {ph:'target', def:'both', target: true}] },
    square: { params: [{ph:'target', def:'both', target: true}] },
    cube: { params: [{ph:'target', def:'both', target: true}] },
    reciprocal: { params: [{ph:'target', def:'both', target: true}] },
    conjugate: { params: [{ph:'target', def:'both', target: true}] },
    swap: {},
    add_sub: {}, mul_div: {},
    moebius: {
        params: [
            {ph:'a', def:'1', complex:true},
            {ph:'b', def:'0', complex:true},
            {ph:'c', def:'0', complex:true},
            {ph:'d', def:'1', complex:true},
        ],
        label: 'moebius',
    },
    negate: { params: [{ph:'target', def:'both', target: true}] },
    exp: { params: [{ph:'target', def:'both', target: true}] },
    xim: { params: [{ph:'target', def:'both', target: true}] },
    zzold: {}, zz1: {}, zz2: {}, zz3: {},
    inv_t_plus_2: { params: [{ph:'re1', def:'2'}, {ph:'im1', def:'0'}, {ph:'re2', def:'2'}, {ph:'im2', def:'0'}] },
    t1radd: { label: 't1.re = t1.re +', params: [{ph:'v', def:'0'}] },
    t1iadd: { label: 't1.im = t1.im +', params: [{ph:'v', def:'0'}] },
    t2radd: { label: 't2.re = t2.re +', params: [{ph:'v', def:'0'}] },
    t2iadd: { label: 't2.im = t2.im +', params: [{ph:'v', def:'0'}] },
    radd:   { params: [{ph:'1', def:'1'}] },
    iadd:   { params: [{ph:'1', def:'1'}] },
    add:    { params: [{ph:'c1', def:'0', complex:true}, {ph:'c2', def:'0', complex:true}], label: 'add' },
    cadd:   { params: [{ph:'re', def:'1'}, {ph:'im', def:'0'}] },
    rscale: { params: [{ph:'10', def:'10'}] },
    iscale: { params: [{ph:'10', def:'10'}] },
    scale:  { params: [{ph:'10', def:'10'}] },
    crd:    { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'size', def:'1'}] },
    hrt:    { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'size', def:'1'}, {ph:'turns', def:'0'}] },
    spdl:   { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'va', def:'0.5'}, {ph:'vb', def:'0.2'}, {ph:'vp', def:'1.5'}] },
    lmc:    { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'a', def:'0.3'}, {ph:'b', def:'0.5'}] },
    rsc:    { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'amp', def:'0.5'}, {ph:'k', def:'2'}] },
    lss:    { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'A', def:'0.5'}, {ph:'B', def:'0.5'}, {ph:'a', def:'3'}, {ph:'b', def:'2'}, {ph:'phase', def:'0.5'}] },
    ast:    { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'scale', def:'1'}] },
    asp:    { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'a', def:'0'}, {ph:'b', def:'0.1'}] },
    lsp:    { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'a', def:'0.1'}, {ph:'b', def:'0.15'}] },
    dlt:    { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'R', def:'1'}] },
    z01: {},
    sum_prod: {},
    roots2: {},
    roots3: {},
    roots5: {},
    roots6: {},
    rply:   { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'sides', def:'5'}, {ph:'radius', def:'1'}, {ph:'turns', def:'0'}] },
    star:   { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'points', def:'5'}, {ph:'outer', def:'1'}, {ph:'inner', def:'0.5'}] },
    rect:   { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'width', def:'2'}, {ph:'height', def:'1'}, {ph:'turns', def:'0'}] },
    rrect:  { params: [{ph:'t1|t2|both', def:'t1', target: true}, {ph:'width', def:'2'}, {ph:'height', def:'1'}, {ph:'m', def:'4'}] },
    sdith:  { params: [{ph:'1', def:'1'}] },
    ddith:  { params: [{ph:'t1|t2|both', def:'both', target: true}, {ph:'d', def:'1'}, {ph:'exp', def:'0.5'}] },
    adth:   { params: [{ph:'t1|t2|both', def:'both', target: true}, {ph:'d', def:'1'}, {ph:'inner', def:'0.4'}] },
    ldth:   { params: [{ph:'t1|t2|both', def:'both', target: true}, {ph:'d', def:'1'}, {ph:'len', def:'1'}, {ph:'angle', def:'0'}] },
    crdth:  { params: [{ph:'t1|t2|both', def:'both', target: true}, {ph:'d', def:'1'}] },
    scdth:  { params: [{ph:'t1|t2|both', def:'both', target: true}, {ph:'d', def:'1'}, {ph:'half_ap', def:'0.25'}, {ph:'center', def:'0'}] },
    ndith:  { params: [{ph:'1', def:'1'}] },
    coeff2: {}, coeff3: {}, coeff3a: {},
    coeff4: {}, coeff5: {}, coeff5a: {},
    coeff6: {}, coeff7: {}, coeff8: {},
    coeff9: {}, coeff10: {}, coeff11: {}, coeff12: {},
};

;(window.__ppParts = window.__ppParts || []).push('06-popup-init');
