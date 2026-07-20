// PolyPaint 10-status-results — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Deploy rewrites the script tags to build-versioned asset
// keys (assets/<BUILD_ID>/...), so a deploy flips atomically via index.html.
async function runBilevelPipeline(config = null) {
    const btn = document.getElementById('btn-render-generate');
    const renderBtn = document.getElementById('btn-render-generate');
    const popupRunBtn = document.getElementById('bilevel-popup-run');
    const popupRunOrig = popupRunBtn ? popupRunBtn.textContent : 'Execute';
    if (btn) btn.disabled = true;
    if (renderBtn) renderBtn.disabled = true;
    if (popupRunBtn) { popupRunBtn.disabled = true; popupRunBtn.textContent = 'Executing...'; }
    try {
        const sectionMode = _normalizeRenderMtSectionMode(
            config && typeof config === 'object' && config.sectionMode != null
                ? config.sectionMode
                : _bilevelPopupState.sectionMode
        );
        const requestedSectionCount = _clampRenderMtSectionCount(
            config && typeof config === 'object' && config.sectionCount != null
                ? config.sectionCount
                : _bilevelPopupState.sectionCount
        );
        const sectionCount = sectionMode === 'logical_sections' ? requestedSectionCount : '';
        _bilevelPopupState.sectionMode = sectionMode;
        _bilevelPopupState.sectionCount = sectionCount;
        log(
            `BiLevel: dispatching with sections=${_renderMtSectionSettingLabel(sectionMode === 'physical_chunks' ? 'logical_sections_auto' : sectionMode, sectionCount || '')}, sparse fragments, single finalize...`,
            '',
            'render-log'
        );
        await _launchRenderOrchestrator('bilevel', {
            raster_section_mode: sectionMode === 'physical_chunks' ? 'logical_sections_auto' : sectionMode,
            raster_section_count: sectionCount,
        });
        _closeBilevelPopup();
    } catch (e) {
        document.getElementById('render-status').textContent = 'BiLevel error: ' + e.message;
        document.getElementById('render-status').className = 'status error';
        log('BiLevel failed: ' + e.message, 'err', 'render-log');
    } finally {
        if (btn) btn.disabled = false;
        if (renderBtn) renderBtn.disabled = false;
        if (popupRunBtn) { popupRunBtn.disabled = false; popupRunBtn.textContent = popupRunOrig; }
    }
}

async function runCoeffBilevelPipeline() {
    const btn = document.getElementById('btn-render-generate');
    const renderBtn = document.getElementById('btn-render-generate');
    if (btn) btn.disabled = true;
    if (renderBtn) renderBtn.disabled = true;
    try {
        await _launchRenderOrchestrator('coeff_bilevel');
    } catch (e) {
        document.getElementById('render-status').textContent = 'Coeffs error: ' + e.message;
        document.getElementById('render-status').className = 'status error';
        log('Coeffs failed: ' + e.message, 'err', 'render-log');
    } finally {
        if (btn) btn.disabled = false;
        if (renderBtn) renderBtn.disabled = false;
    }
}

// ── Solve Histogram Debug ────────────────────────────────────────────────

function _renderPreviewSourceMode() {
    const mode = String(document.getElementById('render-preview-source-mode')?.value || 'lores').trim().toLowerCase();
    return mode === 'logical' || mode === 'recompute' ? mode : 'lores';
}

function _syncRenderPreviewSourceMode() {
    const mode = _renderPreviewSourceMode();
    const sizeEl = document.getElementById('render-preview-source-size');
    const noteEl = document.getElementById('render-preview-source-note');
    if (sizeEl) sizeEl.disabled = mode === 'lores';
    if (noteEl) {
        noteEl.textContent = mode === 'lores'
            ? 'size ignored for lores'
            : 'defaults to solve lores_N; max 256';
    }
}

function _renderLoresPreviewTabIndex(tab) {
    const match = /^e([1-3])$/.exec(String(tab || ''));
    return match ? Number(match[1]) - 1 : -1;
}

function _drawRenderLoresPreviewHistogram(index) {
    const row = _renderLoresPreviewEmissionHistograms[index] || null;
    const canvas = document.getElementById(`render-lores-preview-hist-e${index + 1}`);
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const w = canvas.width || 256;
    const h = canvas.height || 256;
    ctx.clearRect(0, 0, w, h);
    ctx.fillStyle = '#05070c';
    ctx.fillRect(0, 0, w, h);
    if (!row || !Array.isArray(row.histogram)) {
        ctx.fillStyle = '#778599';
        ctx.font = '12px monospace';
        ctx.fillText('No emission data', 14, 24);
        return;
    }
    const hist = row.histogram.slice(0, 256).map(v => Math.max(0, Number(v) || 0));
    const maxLog = Math.max(1, ...hist.map(v => Math.log1p(v)));
    const plotLeft = 12;
    const plotTop = 30;
    const plotW = w - 24;
    const plotH = h - 54;
    ctx.strokeStyle = 'rgba(255,255,255,0.10)';
    ctx.lineWidth = 1;
    for (let i = 0; i <= 4; i += 1) {
        const y = plotTop + (plotH * i / 4);
        ctx.beginPath();
        ctx.moveTo(plotLeft, y);
        ctx.lineTo(plotLeft + plotW, y);
        ctx.stroke();
    }
    ctx.fillStyle = '#e94560';
    for (let bin = 0; bin < 256; bin += 1) {
        const barH = Math.round((Math.log1p(hist[bin]) / maxLog) * plotH);
        const x = plotLeft + (bin / 256) * plotW;
        const bw = Math.max(1, Math.ceil(plotW / 256));
        ctx.fillRect(x, plotTop + plotH - barH, bw, barH);
    }
    ctx.fillStyle = '#d8e1f1';
    ctx.font = '11px monospace';
    const label = row.label || `E${index + 1}`;
    const name = row.name && row.name !== label ? ` · ${row.name}` : '';
    ctx.fillText(`${label}${name} · ${row.emit || 'emit'}`, 12, 18);
    ctx.fillStyle = '#778599';
    ctx.fillText(`samples=${Number(row.total || 0).toLocaleString()} nonzero=${Number(row.nonzero_count || 0).toLocaleString()}`, 12, h - 12);
}

function _clearRenderLoresPreviewPaletteCanvas(message = 'No palette preview') {
    const canvas = document.getElementById('render-lores-preview-palette-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const w = canvas.width || 256;
    const h = canvas.height || 256;
    ctx.clearRect(0, 0, w, h);
    ctx.fillStyle = '#05070c';
    ctx.fillRect(0, 0, w, h);
    ctx.fillStyle = '#778599';
    ctx.font = '12px monospace';
    ctx.fillText(message, 14, 24);
}

async function _setRenderLoresPreviewPaletteImage(result) {
    const btn = document.getElementById('render-lores-preview-tab-palette');
    const canvas = document.getElementById('render-lores-preview-palette-canvas');
    const imageB64 = result && result.palette_image_base64 ? String(result.palette_image_base64) : '';
    _renderLoresPreviewHasPalette = !!imageB64;
    if (btn) {
        btn.style.display = imageB64 ? '' : 'none';
        btn.title = imageB64 ? 'Dense preview-source palette in serpentine parameter-grid order' : '';
    }
    if (!canvas || !imageB64) {
        _clearRenderLoresPreviewPaletteCanvas();
        if (_renderLoresPreviewActiveTab === 'palette') _renderLoresPreviewActiveTab = 'plot';
        return;
    }
    await new Promise((resolve, reject) => {
        const img = new Image();
        img.onload = () => {
            canvas.width = img.naturalWidth || Number(result.palette_pix) || 256;
            canvas.height = img.naturalHeight || Number(result.palette_pix) || canvas.width;
            const ctx = canvas.getContext('2d');
            ctx.imageSmoothingEnabled = false;
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.drawImage(img, 0, 0);
            resolve();
        };
        img.onerror = () => reject(new Error('preview palette PNG decode failed'));
        img.src = `data:${result.palette_content_type || 'image/png'};base64,${imageB64}`;
    });
}

function _selectRenderLoresPreviewTab(tab, options = {}) {
    const requested = String(tab || 'plot');
    const idx = _renderLoresPreviewTabIndex(requested);
    const next = requested === 'palette' && !_renderLoresPreviewHasPalette
        ? 'plot'
        : (idx >= 0 && !_renderLoresPreviewEmissionHistograms[idx] ? 'plot' : requested);
    // A direct tab click while the scrub pad is open updates the pad's
    // remembered view; internal re-selects and fallback resolutions must
    // not (a temporarily unavailable pane cannot clobber intent).
    if (!options.fromScrubPad && !options.internal && next === requested &&
        typeof _scrubPadState !== 'undefined' && _scrubPadState && _scrubPadState.view) {
        _scrubPadState.view = next;
        const viewSel = document.getElementById('program-scrub-view');
        if (viewSel && viewSel.value !== next) viewSel.value = next;
    }
    _renderLoresPreviewActiveTab = next;
    ['plot', 'palette', 'e1', 'e2', 'e3'].forEach((name) => {
        const btn = document.getElementById(`render-lores-preview-tab-${name}`);
        const pane = document.getElementById(`render-lores-preview-pane-${name}`);
        if (btn) btn.classList.toggle('active', name === next);
        if (pane) pane.classList.toggle('active', name === next);
    });
    const activeIdx = _renderLoresPreviewTabIndex(next);
    if (activeIdx >= 0) _drawRenderLoresPreviewHistogram(activeIdx);
}

function _setRenderLoresPreviewEmissionHistograms(rows) {
    _renderLoresPreviewEmissionHistograms = Array.isArray(rows) ? rows.slice(0, 3) : [];
    for (let idx = 0; idx < 3; idx += 1) {
        const row = _renderLoresPreviewEmissionHistograms[idx] || null;
        const tab = `e${idx + 1}`;
        const btn = document.getElementById(`render-lores-preview-tab-${tab}`);
        if (btn) {
            btn.style.display = row ? '' : 'none';
            btn.textContent = row && row.label ? row.label : `E${idx + 1}`;
            btn.title = row ? `${row.name || row.label || `E${idx + 1}`} histogram` : '';
        }
        _drawRenderLoresPreviewHistogram(idx);
    }
    if (_renderLoresPreviewTabIndex(_renderLoresPreviewActiveTab) >= _renderLoresPreviewEmissionHistograms.length) {
        _renderLoresPreviewActiveTab = 'plot';
    }
    _selectRenderLoresPreviewTab(_renderLoresPreviewActiveTab, { internal: true });
}

function _teardownRenderLoresPreviewMarquee() {
    const cleanup = _renderLoresPreviewDragState.cleanup;
    _renderLoresPreviewDragState.cleanup = null;
    _renderLoresPreviewDragState.dragging = false;
    _renderLoresPreviewDragState.start = null;
    _renderLoresPreviewDragState.rect = null;
    if (typeof cleanup === 'function') cleanup();
}

function _clearRenderLoresPreviewSelection() {
    _renderLoresPreviewSelectionState.rect = null;
    _renderLoresPreviewDragState.dragging = false;
    _renderLoresPreviewDragState.start = null;
    _renderLoresPreviewDragState.rect = null;
    _syncRenderLoresPreviewMarquee();
}

function _renderLoresPreviewCanvasRect(stage, canvas) {
    if (!stage || !canvas) return null;
    const stageRect = stage.getBoundingClientRect();
    const canvasRect = canvas.getBoundingClientRect();
    if (!(stageRect.width > 0 && stageRect.height > 0 && canvasRect.width > 0 && canvasRect.height > 0)) return null;
    return {
        x: canvasRect.left - stageRect.left,
        y: canvasRect.top - stageRect.top,
        width: canvasRect.width,
        height: canvasRect.height,
    };
}

function _renderLoresPreviewPointerFractions(canvas, event) {
    if (!canvas || !event) return null;
    const rect = canvas.getBoundingClientRect();
    if (!(rect.width > 0 && rect.height > 0)) return null;
    if (
        Number(event.clientX) < rect.left ||
        Number(event.clientX) > rect.right ||
        Number(event.clientY) < rect.top ||
        Number(event.clientY) > rect.bottom
    ) return null;
    return {
        x: _clampDeepZoomViewportCoord((Number(event.clientX) - rect.left) / rect.width, 0, 1),
        y: _clampDeepZoomViewportCoord((Number(event.clientY) - rect.top) / rect.height, 0, 1),
    };
}

function _syncRenderLoresPreviewMarquee() {
    const marquee = document.getElementById('render-lores-preview-marquee');
    if (!marquee) return;
    const stage = document.getElementById('render-lores-preview-stage');
    const canvas = document.getElementById('render-lores-preview-canvas');
    const canvasRect = _renderLoresPreviewCanvasRect(stage, canvas);
    const rect = _renderLoresPreviewDragState.dragging
        ? _renderLoresPreviewDragState.rect
        : _renderLoresPreviewSelectionState.rect;
    if (!canvasRect || !rect || !(rect.w > 0) || !(rect.h > 0)) {
        marquee.style.display = 'none';
        return;
    }
    marquee.style.display = 'block';
    marquee.style.left = `${canvasRect.x + rect.x * canvasRect.width}px`;
    marquee.style.top = `${canvasRect.y + rect.y * canvasRect.height}px`;
    marquee.style.width = `${rect.w * canvasRect.width}px`;
    marquee.style.height = `${rect.h * canvasRect.height}px`;
}

function _renderLoresPreviewMetaFromResult(result, rotation) {
    const viewport = result && result.viewport ? result.viewport : {};
    const canvas = document.getElementById('render-lores-preview-canvas');
    const width = Number(canvas && canvas.width) || Number(result && result.preview_pix);
    const height = Number(canvas && canvas.height) || Number(result && result.preview_pix) || width;
    const minRe = Number(viewport.min_re);
    const maxRe = Number(viewport.max_re);
    const minIm = Number(viewport.min_im);
    const maxIm = Number(viewport.max_im);
    const rot = Number(rotation);
    return {
        width,
        height,
        minRe,
        maxRe,
        minIm,
        maxIm,
        rotation: Number.isFinite(rot) ? rot : 0,
        hasViewport: (
            Number.isFinite(width) &&
            Number.isFinite(height) &&
            width > 0 &&
            height > 0 &&
            Number.isFinite(minRe) &&
            Number.isFinite(maxRe) &&
            Number.isFinite(minIm) &&
            Number.isFinite(maxIm) &&
            maxRe > minRe &&
            maxIm > minIm
        ),
    };
}

function _applyRenderLoresPreviewSelectionBounds(meta, rect) {
    if (!meta || !rect) return;
    const imgRect = {
        x: rect.x * meta.width,
        y: rect.y * meta.height,
        width: rect.w * meta.width,
        height: rect.h * meta.height,
    };
    const cameraRect = _deepZoomCameraRectFromImageRect(meta, imgRect);
    const bounds = _deepZoomRenderBoundsFromCameraRect(meta, cameraRect);
    if (!bounds) return;
    _setRenderExplicitViewportBounds({
        minRe: bounds.min_re,
        maxRe: bounds.max_re,
        minIm: bounds.min_im,
        maxIm: bounds.max_im,
    });
    selectViewMode('explicit');
    const statusEl = document.getElementById('render-lores-preview-status');
    const renderStatusEl = document.getElementById('render-status');
    // One short word beside the Preview button (long text rewraps the
    // controls row and bumps the canvas); the exact bounds go to the log.
    if (statusEl) statusEl.textContent = 'subview';
    if (renderStatusEl && !_activeRenderRun) {
        renderStatusEl.textContent = 'Ephemeral preview subview selected';
        renderStatusEl.className = 'status ok';
    }
    log(`Render preview selection: exact viewport min_re=${bounds.min_re.toPrecision(8)} max_re=${bounds.max_re.toPrecision(8)} min_im=${bounds.min_im.toPrecision(8)} max_im=${bounds.max_im.toPrecision(8)}`, 'ok', 'render-log');
}

function _initRenderLoresPreviewMarquee(meta) {
    _teardownRenderLoresPreviewMarquee();
    _renderLoresPreviewMeta = meta || null;
    _clearRenderLoresPreviewSelection();
    const stage = document.getElementById('render-lores-preview-stage');
    const canvas = document.getElementById('render-lores-preview-canvas');
    const marquee = document.getElementById('render-lores-preview-marquee');
    if (!stage || !canvas || !marquee) return;
    const interactive = !!(meta && meta.hasViewport);
    stage.classList.toggle('inactive', !interactive);
    _syncRenderLoresPreviewMarquee();
    if (!interactive) return;

    const onMouseDown = (event) => {
        if (event.button !== 0) return;
        const start = _renderLoresPreviewPointerFractions(canvas, event);
        if (!start) return;
        event.preventDefault();
        _renderLoresPreviewDragState.dragging = true;
        _renderLoresPreviewDragState.start = start;
        _renderLoresPreviewDragState.rect = { x: start.x, y: start.y, w: 0, h: 0 };
        _syncRenderLoresPreviewMarquee();
    };

    const onMouseMove = (event) => {
        if (!_renderLoresPreviewDragState.dragging) return;
        const current = _renderLoresPreviewPointerFractions(canvas, event);
        if (!current) return;
        _renderLoresPreviewDragState.rect = _normalizeRenderPreviewRect(_renderLoresPreviewDragState.start, current);
        _syncRenderLoresPreviewMarquee();
    };

    const onMouseUp = (event) => {
        if (!_renderLoresPreviewDragState.dragging) return;
        const end = _renderLoresPreviewPointerFractions(canvas, event) || _renderLoresPreviewDragState.start;
        const rect = _normalizeRenderPreviewRect(_renderLoresPreviewDragState.start, end);
        _renderLoresPreviewDragState.dragging = false;
        _renderLoresPreviewDragState.start = null;
        _renderLoresPreviewDragState.rect = null;
        if (rect && rect.w > 0 && rect.h > 0) {
            _renderLoresPreviewSelectionState.rect = rect;
            _applyRenderLoresPreviewSelectionBounds(_renderLoresPreviewMeta, rect);
        }
        _syncRenderLoresPreviewMarquee();
    };

    const onContextMenu = (event) => {
        event.preventDefault();
        if (_renderLoresPreviewDragState.dragging) return;
        _clearRenderLoresPreviewSelection();
    };

    const onBlur = () => {
        if (!_renderLoresPreviewDragState.dragging) return;
        _renderLoresPreviewDragState.dragging = false;
        _renderLoresPreviewDragState.start = null;
        _renderLoresPreviewDragState.rect = null;
        _syncRenderLoresPreviewMarquee();
    };

    stage.addEventListener('mousedown', onMouseDown);
    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', onMouseUp);
    stage.addEventListener('contextmenu', onContextMenu);
    window.addEventListener('blur', onBlur);
    _renderLoresPreviewDragState.cleanup = () => {
        stage.removeEventListener('mousedown', onMouseDown);
        document.removeEventListener('mousemove', onMouseMove);
        document.removeEventListener('mouseup', onMouseUp);
        stage.removeEventListener('contextmenu', onContextMenu);
        window.removeEventListener('blur', onBlur);
    };
}

async function runRenderLoresPreview() {
    const btn = document.getElementById('btn-render-lores-preview');
    const statusEl = document.getElementById('render-lores-preview-status');
    const renderStatusEl = document.getElementById('render-status');
    const canvas = document.getElementById('render-lores-preview-canvas');
    try {
        if (renderColorMode !== 'solve_score') throw new Error('select Solve score mode first');
        const p = _renderCommonParams({ requireSolveScore: true });
        let previewPix = parseInt(document.getElementById('render-preview-pix')?.value || '256', 10);
        if (!Number.isFinite(previewPix)) previewPix = 256;
        previewPix = Math.max(16, Math.min(1024, previewPix));
        const pixInput = document.getElementById('render-preview-pix');
        if (pixInput) pixInput.value = String(previewPix);

        if (btn) { btn.disabled = true; btn.textContent = 'Preview...'; }
        if (statusEl) statusEl.textContent = 'calc';
        if (renderStatusEl) {
            renderStatusEl.textContent = 'Rendering lores preview...';
            renderStatusEl.className = 'status';
        }
        _initRenderLoresPreviewMarquee(null);
        // Panes stay sticky across recalculation: the last plot/palette/
        // histogram frames remain visible until the new result replaces
        // them (smooth scrubbing — no plot flash), and the completion
        // setters below own the fallbacks when a pane disappears.

        const detail = await lambdaPost('storage', { job_id: p.jobId }, '/detail');
        const calc = detail.calc || {};
        const degree = Number(calc.degree);
        const loresKey = calc.lores && calc.lores.bin_key;
        const loresCoeffsKey = (calc.lores && calc.lores.coeffs_key) || _fallbackLoresCoeffsKey(p.jobId, loresKey);
        const loresParamsKey = (calc.lores && calc.lores.params_key) || _fallbackLoresParamsKey(p.jobId, loresKey);
        const rawNCoeffs = Number(calc.n_coeffs);
        const nCoeffs = Number.isFinite(rawNCoeffs) && rawNCoeffs >= 1 ? rawNCoeffs : degree + 1;
        const previewSourceMode = _renderPreviewSourceMode();
        const calcTimes = Math.max(1, Number(calc.times) || 1);
        const calcLoresNRaw = Number(calc.lores && calc.lores.N);
        const calcLoresSteps = Number(calc.lores && calc.lores.n_steps);
        const calcLoresN = Number.isFinite(calcLoresNRaw) && calcLoresNRaw > 0
            ? Math.round(calcLoresNRaw)
            : (Number.isFinite(calcLoresSteps) && calcLoresSteps > 0 ? Math.max(5, Math.round(Math.sqrt(calcLoresSteps / calcTimes))) : 60);
        const sourceSizeInput = document.getElementById('render-preview-source-size');
        const previousDefault = parseInt(sourceSizeInput?.dataset.defaultLoresN || '', 10);
        const previousDefaultJob = sourceSizeInput?.dataset.defaultForJob || '';
        const sourceSizeText = String(sourceSizeInput?.value || '').trim();
        let previewSourceSize = parseInt(sourceSizeText, 10);
        const shouldDefaultSourceSize = !Number.isFinite(previewSourceSize)
            || !sourceSizeText
            || (previousDefaultJob && previousDefaultJob !== p.jobId && Number.isFinite(previousDefault) && previewSourceSize === previousDefault);
        if (shouldDefaultSourceSize) previewSourceSize = calcLoresN;
        previewSourceSize = Math.max(5, Math.min(256, previewSourceSize));
        if (sourceSizeInput) {
            sourceSizeInput.value = String(previewSourceSize);
            sourceSizeInput.dataset.defaultForJob = p.jobId;
            sourceSizeInput.dataset.defaultLoresN = String(calcLoresN);
        }
        const sourceNote = document.getElementById('render-preview-source-note');
        if (sourceNote) sourceNote.textContent = previewSourceMode === 'lores' ? `using saved lores_N=${calcLoresN}` : `solve lores_N=${calcLoresN}; max 256`;
        _syncRenderPreviewSourceMode();

        if (!Number.isFinite(degree) || degree < 1 || (!loresKey && previewSourceMode === 'lores')) {
            throw new Error('calc.json missing degree or lores.bin_key');
        }
        if (previewSourceMode === 'lores' && p.solveScoreUsesCoeffSource) {
            if (!loresCoeffsKey) throw new Error('calc.json missing lores.coeffs_key for coeff-source render preview');
            if (!Number.isFinite(nCoeffs) || nCoeffs < 1) throw new Error('calc.json missing n_coeffs for coeff-source render preview');
        }
        if (previewSourceMode === 'lores' && p.solveScoreUsesParamSource && !loresParamsKey) {
            throw new Error('calc.json missing lores.params_key for param-source render preview');
        }

        const payload = {
            job_id: p.jobId,
            degree: degree,
            n_coeffs: nCoeffs,
            preview_pix: previewPix,
            quality: p.quality,
            palette: _activeRenderPalette() || 'inferno',
            background_color: p.backgroundColor,
            view_mode: _viewMode,
            quantile: p.quantile,
            shim: p.shim,
            square_extent: p.squareExtent,
            rotation: p.rotation,
            metric: p.solveScoreMetric,
            solve_score_quantile: p.solveScoreQuantile,
            solve_score_omega: p.solveScoreOmega,
            solve_score_omega_enabled: p.solveScoreOmegaEnabled,
            solve_score_program_source_text: p.solveScoreProgramSourceText,
            solve_score_normalize: !!p.solveScoreNormalize,
            color_interpretation: p.colorInterpretation,
            preview_source_mode: previewSourceMode,
            preview_source_size: previewSourceSize,
            lores_N: calcLoresN,
            lores_bin_key: loresKey,
            lores_coeffs_key: p.solveScoreUsesCoeffSource ? loresCoeffsKey : undefined,
            lores_params_key: p.solveScoreUsesParamSource ? loresParamsKey : undefined,
            root_program_source_text: p.rootProgramSourceText || undefined,
            raster_mt_threads: 4,
            solve_score_threads: 4,
            raster_sectioned_retries: 2,
        };
        if (_viewMode === 'explicit') {
            payload.min_re = p.minRe;
            payload.max_re = p.maxRe;
            payload.min_im = p.minIm;
            payload.max_im = p.maxIm;
        }

        const sourceStartLabel = previewSourceMode === 'lores'
            ? 'saved lores artifacts'
            : `${previewSourceMode} ${previewSourceSize}x${previewSourceSize}`;
        log(`Render preview: colorizing ${sourceStartLabel} at ${previewPix}px...`, '', 'render-log');
        const result = await lambdaPost('render-lores-preview', payload);
        if (!result || !result.image_base64) throw new Error('preview response missing image_base64');
        _setRenderLoresPreviewEmissionHistograms(result.emission_histograms || result.solve_score?.emission_histograms || []);
        await _setRenderLoresPreviewPaletteImage(result);

        await new Promise((resolve, reject) => {
            const img = new Image();
            img.onload = () => {
                canvas.width = img.naturalWidth || previewPix;
                canvas.height = img.naturalHeight || previewPix;
                const ctx = canvas.getContext('2d');
                ctx.imageSmoothingEnabled = false;
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                ctx.drawImage(img, 0, 0);
                resolve();
            };
            img.onerror = () => reject(new Error('preview PNG decode failed'));
            img.src = `data:${result.content_type || 'image/png'};base64,${result.image_base64}`;
        });
        _initRenderLoresPreviewMarquee(_renderLoresPreviewMetaFromResult(result, p.rotation));

        const raster = result.raster || {};
        const timings = result.timings_ms || {};
        const source = result.source || {};
        for (const line of (Array.isArray(result.logs) ? result.logs : [])) {
            log(line, '', 'render-log');
        }
        const roots = Number(raster.roots_plotted || 0);
        const totalMs = Number(timings.total || 0);
        const sourceLabel = source.mode === 'logical' || source.mode === 'recompute' ? `${source.mode} ${source.view_N || previewSourceSize}` : 'lores';
        const msg = `${previewPix}px · ${sourceLabel} · roots=${roots.toLocaleString()} · nonzero=${Number(result.nonzero_pixels || 0).toLocaleString()} · ${(totalMs / 1000).toFixed(2)}s`;
        if (statusEl) statusEl.textContent = 'done';
        if (renderStatusEl) {
            renderStatusEl.textContent = 'Render preview ready';
            renderStatusEl.className = 'status success';
        }
        log(`Render preview: ${msg}`, 'ok', 'render-log');
    } catch (e) {
        _initRenderLoresPreviewMarquee(null);
        // Keep the panes: the plot canvas keeps its last good frame on
        // error, so palette/histograms do too (scrubbing routinely passes
        // through momentarily-invalid values).
        const msg = e && e.message ? e.message : String(e);
        if (statusEl) statusEl.textContent = 'error';
        if (renderStatusEl) {
            renderStatusEl.textContent = 'Render preview error: ' + msg;
            renderStatusEl.className = 'status error';
        }
        log('Render preview failed: ' + msg, 'err', 'render-log');
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = 'Preview'; }
    }
}

async function runSolveScoreHistogramDebug() {
    const btn = document.getElementById('btn-solve-histogram');
    const statusEl = document.getElementById('render-status');
    const jobId = document.getElementById('render-results-dir').value.trim();

    if (!jobId) { log('Histogram: no results dir', 'err', 'render-log'); return; }
    if (renderColorMode !== 'solve_score') { log('Histogram: select Solve score mode first', 'err', 'render-log'); return; }
    if (_activeRenderRun) { log('Histogram: render in progress, wait for completion', 'err', 'render-log'); return; }

    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Histogram...'; }
        statusEl.textContent = 'Computing solve histogram...';
        statusEl.className = 'status';
        log('Solve histogram: starting...', '', 'render-log');

        // Load calc metadata
        const detail = await lambdaPost('storage', { job_id: jobId }, '/detail');
        const calc = detail.calc || {};
        const degree = calc.degree;
        const loresKey = calc.lores && calc.lores.bin_key;
        const loresCoeffsKey = (calc.lores && calc.lores.coeffs_key) || _fallbackLoresCoeffsKey(jobId, loresKey);
        const loresParamsKey = (calc.lores && calc.lores.params_key) || _fallbackLoresParamsKey(jobId, loresKey);
        const rawNCoeffs = Number(calc.n_coeffs);
        const nCoeffs = Number.isFinite(rawNCoeffs) && rawNCoeffs >= 1 ? rawNCoeffs : Number(degree) + 1;
        if (!degree || !loresKey) {
            throw new Error('calc.json missing degree or lores.bin_key');
        }

        // Build payload from current UI state
        const p = _renderCommonParams({ requireSolveScore: true });
        if (p.solveScoreUsesCoeffSource) {
            if (!loresCoeffsKey) throw new Error('calc.json missing lores.coeffs_key for coeff-source solve-score debug');
            if (!Number.isFinite(nCoeffs) || nCoeffs < 1) throw new Error('calc.json missing n_coeffs for coeff-source solve-score debug');
        }
        if (p.solveScoreUsesParamSource && !loresParamsKey) {
            throw new Error('calc.json missing lores.params_key for param-source solve-score debug');
        }
        const payload = {
            phase: 'summary',
            job_id: jobId,
            degree: degree,
            n_coeffs: nCoeffs,
            metric: p.solveScoreMetric,
            solve_score_quantile: p.solveScoreQuantile,
            solve_score_omega: p.solveScoreOmega,
            solve_score_omega_enabled: p.solveScoreOmegaEnabled,
            solve_score_program_source_text: p.solveScoreProgramSourceText,
            solve_score_normalize: !!p.solveScoreNormalize,
            lores_bin_key: loresKey,
            lores_coeffs_key: p.solveScoreUsesCoeffSource ? loresCoeffsKey : undefined,
            lores_params_key: p.solveScoreUsesParamSource ? loresParamsKey : undefined,
            root_program_source_text: p.rootProgramSourceText || undefined,
        };
        const chainSummary = _formatErrorChain(p.solveScoreChain);
        const metricsSummary = _solveScoreMetricSummary(p.solveScoreMetrics);
        log(
            `Solve histogram: request program=${p.solveScoreProgramSpec || 'm0'} · ${chainSummary ? `chain=${chainSummary} · ` : ''}${metricsSummary ? `metrics=${metricsSummary} · ` : ''}${_omegaSummaryLabel(p.solveScoreOmegaEnabled, p.solveScoreOmega)}`,
            '',
            'render-log'
        );

        const summary = await lambdaPost('solve_proximity', payload);

        // Format and log results
        const s = summary;
        const fmt = (v) => typeof v === 'number' ? v.toFixed(3) : v;
        const pct = (v) => typeof v === 'number' ? (v * 100).toFixed(1) + '%' : v;

        // Build the full log block as a <pre> for fixed-width alignment
        const lines = [];
        const summaryProgram = String(s.program || p.solveScoreProgramSpec || 'm0');
        const summaryMetrics = Array.isArray(s.score_metrics) && Array.isArray(s.score_quantiles)
            ? (() => {
                const showAllSources = Array.isArray(s.score_sources) && s.score_sources.some(src => _normalizeSolveScoreMetricSource(src, 'slv') !== 'slv');
                return s.score_metrics.map((name, idx) => {
                const source = Array.isArray(s.score_sources) ? s.score_sources[idx] : 'slv';
                const qPct = ((Number(s.score_quantiles[idx]) || 0) * 100).toFixed(1).replace(/\.0$/, '');
                return (source === 'slv' && !showAllSources)
                    ? `${name}(q=${qPct}%)`
                    : `${name}(${source},q=${qPct}%)`;
            }).join(', ');
            })()
            : metricsSummary;
        lines.push(`Solve histogram: program=${summaryProgram}, n=${s.n_solves}, degree=${s.degree}`);
        if (summaryMetrics) lines.push(`  metrics: ${summaryMetrics}`);
        if (s.solve_score_display) lines.push(`  chain: ${s.solve_score_display}`);
        lines.push(`  lores-only debug; raw score bins are pre-clip, final color bins mirror the actual 10 solve-score bins`);
        lines.push(`  min=${fmt(s.min_score)}  p05=${fmt(s.q05)}  p10=${fmt(s.q10)}  p25=${fmt(s.q25)}  p50=${fmt(s.q50)}  p75=${fmt(s.q75)}  p90=${fmt(s.q90)}  p95=${fmt(s.q95)}  max=${fmt(s.max_score)}`);
        lines.push(`  mean=${fmt(s.mean_score)}  std=${fmt(s.stddev_score)}`);
        lines.push(`  clip[q..1-q]: lo=${fmt(s.clip_lo)}  hi=${fmt(s.clip_hi)}  width=${fmt(s.clip_range)} / full=${fmt(s.full_range)}`);
        if (s.score_output_normalize) {
            lines.push(`  score normalization: lo=${fmt(s.score_output_clip_lo)}  hi=${fmt(s.score_output_clip_hi)}  source=${s.score_output_clip_source || 'lores_q05_q95'}`);
        }
        lines.push(`  occupancy: below=${s.clip_below_count} (${pct(s.clip_below_frac)})  in=${s.clip_inrange_count} (${pct(s.clip_inrange_frac)})  above=${s.clip_above_count} (${pct(s.clip_above_frac)})`);
        lines.push(`  fallback: ${s.clip_fallback ? s.clip_fallback_reason : 'none'}`);
        if (s.fully_finite_solve_count !== undefined) {
            lines.push(`  lores rows: total=${s.n_solves}  all_finite=${s.fully_finite_solve_count} (${pct(s.fully_finite_solve_frac)})  partial=${s.partial_finite_solve_count} (${pct(s.partial_finite_solve_frac)})  no_finite=${s.zero_finite_solve_count} (${pct(s.zero_finite_solve_frac)})`);
            lines.push(`  finite diagnostics: full=${s.fully_finite_solve_count} (${pct(s.fully_finite_solve_frac)})  partial=${s.partial_finite_solve_count} (${pct(s.partial_finite_solve_frac)})  zero=${s.zero_finite_solve_count} (${pct(s.zero_finite_solve_frac)})`);
            lines.push(`  finite roots: ${s.finite_root_count}/${s.total_root_slots} (${pct(s.finite_root_frac)})  per solve=${fmt(s.mean_finite_roots_per_solve)}  min=${s.min_finite_roots_per_solve}  max=${s.max_finite_roots_per_solve}`);
            if (s.rows_with_any_exact_zero_root_count !== undefined) {
                lines.push(`  exact zeros: roots=${s.exact_zero_root_count}/${s.total_root_slots} (${pct(s.exact_zero_root_frac)})  rows_any=${s.rows_with_any_exact_zero_root_count} (${pct(s.rows_with_any_exact_zero_root_frac)})  rows_all=${s.rows_all_exact_zero_roots_count} (${pct(s.rows_all_exact_zero_roots_frac)})`);
            }
            lines.push(`  scoring policy: ${s.metric_validity_policy || 'finite_only_min_roots'} (min finite roots=${s.metric_min_finite_roots ?? 0})  usable=${s.usable_solve_count} (${pct(s.usable_solve_frac)})  forced_zero=${s.forced_zero_score_count}`);
        }

        // Raw score histogram before clip/transfer/final color binning
        const rawBins = Array.isArray(s.raw_bin_counts) ? s.raw_bin_counts : [];
        const rawHistBins = Number.isFinite(Number(s.raw_hist_bins)) ? Number(s.raw_hist_bins) : rawBins.length;
        const rawHistLo = Number(s.raw_hist_lo);
        const rawHistHi = Number(s.raw_hist_hi);
        const rawHistRange = Number.isFinite(Number(s.raw_hist_range)) ? Number(s.raw_hist_range) : (rawHistHi - rawHistLo);
        if (rawHistBins > 0 && rawBins.length === rawHistBins && Number.isFinite(rawHistLo) && Number.isFinite(rawHistHi)) {
            const maxRawBin = Math.max(...rawBins);
            const rawBarW = 24;
            let rawLabel = 'raw metric space';
            if (s.raw_hist_space === 'program_output') rawLabel = 'program output before final bins';
            if (s.raw_hist_space === 'score_output_normalized') rawLabel = 'score-output normalized program output';
            lines.push(`  raw score bins (${rawHistBins}, ${rawLabel}${s.raw_hist_expanded ? ', range expanded' : ''}):`);
            for (let b = 0; b < rawHistBins; b++) {
                const lo = rawHistLo + (rawHistRange * b) / rawHistBins;
                const hi = rawHistLo + (rawHistRange * (b + 1)) / rawHistBins;
                const bracket = b === (rawHistBins - 1) ? ']' : ')';
                const cnt = String(rawBins[b]).padStart(6);
                const frac = s.n_solves > 0 ? ((rawBins[b] / s.n_solves) * 100).toFixed(1) : '0.0';
                const bar = maxRawBin > 0 ? '#'.repeat(Math.round(rawBins[b] / maxRawBin * rawBarW)) : '';
                lines.push(`    r${String(b).padStart(2, '0')} [${fmt(lo)} .. ${fmt(hi)}${bracket} ${cnt}  ${frac.padStart(5)}%  |${bar}`);
            }
        }

        // 10-bin final color table
        const bins = s.final_bin_counts || [];
        const cuts = s.cuts_score || [];
        if (bins.length === 10) {
            const maxBin = Math.max(...bins);
            const barW = 24;
            lines.push(`  final color bins (10, actual solve-score bins):`);
            for (let b = 0; b < 10; b++) {
                const lo = b === 0 ? s.clip_lo : cuts[b - 1];
                const hi = b === 9 ? s.clip_hi : cuts[b];
                const bracket = b === 9 ? ']' : ')';
                const cnt = String(bins[b]).padStart(6);
                const frac = s.clip_inrange_count > 0 ? ((bins[b] / s.clip_inrange_count) * 100).toFixed(1) : '0.0';
                const bar = maxBin > 0 ? '#'.repeat(Math.round(bins[b] / maxBin * barW)) : '';
                lines.push(`    b${b}  [${fmt(lo)} .. ${fmt(hi)}${bracket} ${cnt}  ${frac.padStart(5)}%  |${bar}`);
            }
        }

        // Outlier/saturation
        if (s.min_score_count !== undefined) {
            lines.push(`  extremes: min_count=${s.min_score_count}  max_count=${s.max_score_count}  clip_lo_count=${s.clip_lo_count}  clip_hi_count=${s.clip_hi_count}`);
        }
        lines.push(`  lores timings: download=${s.dl_ms}ms  compute=${s.compute_ms}ms`);

        // Append as a <pre> block for alignment
        const logEl = document.getElementById('render-log');
        if (logEl) {
            const pre = document.createElement('pre');
            pre.style.margin = '4px 0';
            pre.style.fontSize = '11px';
            pre.style.lineHeight = '1.4';
            pre.style.whiteSpace = 'pre-wrap';
            pre.textContent = lines.join('\n');
            logEl.prepend(pre);
        }

        statusEl.textContent = 'Ready';
        statusEl.className = 'status ok';

    } catch (e) {
        log('Solve histogram failed: ' + e.message, 'err', 'render-log');
        statusEl.textContent = 'Solve histogram failed';
        statusEl.className = 'status error';
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = 'Histogram'; }
    }
}

// ── Render Palette Artifact ──────────────────────────────────────────────

async function runRenderPaletteArtifact() {
    const btn = document.getElementById('btn-render-generate');
    const statusEl = document.getElementById('render-status');
    const jobId = document.getElementById('render-results-dir').value.trim();

    if (!jobId) { log('Palette: no results dir', 'err', 'render-log'); return; }
    if (renderColorMode !== 'solve_score') { log('Palette: select Solve score mode first', 'err', 'render-log'); return; }
    if (_activeRenderRun) { log('Palette: render in progress, wait for completion', 'err', 'render-log'); return; }
    if (_activePaletteRun || _loadActivePaletteRun()) { log('Palette: run already in progress, wait for completion', 'err', 'render-log'); return; }

    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Generating...'; }
        statusEl.textContent = 'Dispatching palette workflow...';
        statusEl.className = 'status';
        log('Palette artifact: starting...', '', 'render-log');

        const p = _renderCommonParams({ requireSolveScore: true });
        const colorInterpretation = _selectedRenderColorInterpretation();
        const paletteIssue = _solveScorePaletteCompatibility({
            has_explicit_outputs: p.solveScoreHasExplicitOutputs,
            output_channel_count: p.solveScoreOutputChannelCount,
        }, colorInterpretation);
        if (paletteIssue) throw new Error(paletteIssue);
        const runId = _generateRunId();
        const taskId = 'palette_run_' + runId;
        const orchPayload = {
            job_id: jobId,
            run_id: runId,
            task_id: taskId,
            params: {
                metric: p.solveScoreMetric,
                palette: _activeRenderPalette() || 'inferno',
                color_interpretation: colorInterpretation,
                solve_score_quantile: p.solveScoreQuantile,
                solve_score_omega: p.solveScoreOmega,
                solve_score_omega_enabled: p.solveScoreOmegaEnabled,
                solve_score_program_source_text: p.solveScoreProgramSourceText,
                root_program_source_text: p.rootProgramSourceText || undefined,
            },
        };
        const dispResult = await lambdaPost('dispatch', {
            target: 'palette_orchestrator',
            jobs: [orchPayload],
            expected_keys: [],
        });
        if ((dispResult.fired || 0) !== 1) throw new Error('Palette orchestrator dispatch failed');

        _saveActivePaletteRun({
            job_id: jobId,
            run_id: runId,
            task_id: taskId,
            started_at_ms: Date.now(),
        });
        log('  palette orchestrator dispatched: ' + runId, 'ok', 'render-log');
        startActivePaletteObserver();

    } catch (e) {
        log('Palette artifact failed: ' + e.message, 'err', 'render-log');
        statusEl.textContent = 'Palette failed';
        statusEl.className = 'status error';
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = 'Generate'; }
    }
}

// ── Render Observer ──────────────────────────────────────────────────────

function startActiveRenderObserver() {
    stopActiveRenderObserver();
    _renderObserverTimer = setInterval(_pollActiveRenderRun, 3000);
    _pollActiveRenderRun(); // immediate first poll
}

function stopActiveRenderObserver() {
    if (_renderObserverTimer) {
        clearInterval(_renderObserverTimer);
        _renderObserverTimer = null;
    }
}

function resumeActiveRenderObserver() {
    const run = _loadActiveRun();
    if (!run) return;
    _activeRenderRun = run;
    startActiveRenderObserver();
}

function _fmtAge(ms) {
    if (ms < 60000) return Math.round(ms / 1000) + 's';
    return Math.round(ms / 60000) + 'm';
}

function _fmtAutolevelDebugLine(rd) {
    const dbg = rd && rd.autolevel_debug;
    if (!dbg || typeof dbg !== 'object') return '';
    const parts = [];
    const ex = dbg.exclude_background ? 'on' : 'off';
    if (dbg.background_color) {
        parts.push(`bg=${dbg.background_color}`);
    }
    if (dbg.background_threshold != null) {
        parts.push(`tol/ch=${dbg.background_threshold}`);
    }
    parts.push(`exclude=${ex}`);
    if (dbg.r_min_bin != null && dbg.r_max_bin != null) parts.push(`R[${dbg.r_min_bin}..${dbg.r_max_bin}]`);
    if (dbg.g_min_bin != null && dbg.g_max_bin != null) parts.push(`G[${dbg.g_min_bin}..${dbg.g_max_bin}]`);
    if (dbg.b_min_bin != null && dbg.b_max_bin != null) parts.push(`B[${dbg.b_min_bin}..${dbg.b_max_bin}]`);
    if (dbg.black_bin != null && dbg.white_bin != null) parts.push(`pooled[${dbg.black_bin}..${dbg.white_bin}]`);
    if (dbg.black != null && dbg.white != null) parts.push(`levels=${Number(dbg.black).toFixed(3)}..${Number(dbg.white).toFixed(3)}`);
    if (dbg.gamma != null) parts.push(`gamma=${Number(dbg.gamma).toFixed(3)}`);
    if (dbg.final_stretch && dbg.final_lo_bin != null && dbg.final_hi_bin != null) {
        parts.push(`final[${dbg.final_lo_bin}..${dbg.final_hi_bin}]`);
    }
    if (dbg.included_pixels != null || dbg.excluded_pixels != null) {
        parts.push(`px in=${dbg.included_pixels || 0} out=${dbg.excluded_pixels || 0}`);
    }
    return parts.join(' · ');
}

function _fmtResizeDebugLine(rd) {
    const dbg = rd && rd.resize_debug;
    if (!dbg || typeof dbg !== 'object') return '';
    const parts = [];
    if (dbg.engine) parts.push(`engine=${dbg.engine}`);
    if (dbg.target_size != null) parts.push(`target=${dbg.target_size}`);
    if (dbg.size_mode) parts.push(`size=${dbg.size_mode}`);
    if (dbg.kernel) parts.push(`kernel=${dbg.kernel}`);
    if (dbg.gap != null && dbg.gap !== '') parts.push(`gap=${dbg.gap}`);
    if (dbg.linear != null) parts.push(`linear=${dbg.linear ? 'on' : 'off'}`);
    if (dbg.source_dims) parts.push(`src=${dbg.source_dims}`);
    if (dbg.out_dims) parts.push(`out=${dbg.out_dims}`);
    if (dbg.format) parts.push(`fmt=${dbg.format}`);
    if (dbg.process_ms != null) parts.push(`process=${_fmtSecondsMs(dbg.process_ms)}`);
    return parts.join(' · ');
}

function _fmtSecondsMs(ms) {
    const n = Number(ms || 0);
    if (!Number.isFinite(n) || n <= 0) return '0.0s';
    return (n / 1000).toFixed(1) + 's';
}

function _fmtSecondsUs(us) {
    const n = Number(us || 0);
    if (!Number.isFinite(n) || n <= 0) return '0.0s';
    return (n / 1000000).toFixed(1) + 's';
}

function _coerceTimestampMs(value) {
    const n = Number(value);
    return Number.isFinite(n) && n > 0 ? n : null;
}

function _pickLatestTimestampMs(...values) {
    let best = null;
    for (const value of values) {
        const n = _coerceTimestampMs(value);
        if (n != null && (best == null || n > best)) best = n;
    }
    return best;
}

function _serverElapsedMs(startMs, endMs) {
    const start = _coerceTimestampMs(startMs);
    const end = _coerceTimestampMs(endMs);
    if (start == null || end == null) return null;
    return Math.max(0, end - start);
}

function _syncRenderRunServerStart(run, rd) {
    if (!run || !rd) return;
    let serverStartMs = _coerceTimestampMs(rd.run_started_at_ms);
    if (serverStartMs == null && String(rd.phase || '') === 'queued') {
        serverStartMs = _coerceTimestampMs(rd.started_at_ms);
    }
    if (serverStartMs != null && run.server_started_at_ms !== serverStartMs) {
        run.server_started_at_ms = serverStartMs;
        _saveActiveRun(run);
    }
}

function _aggregateRasterPerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (r.engine || r.native_us != null || r.threads != null)) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        acc.engine = acc.engine || row.engine || 'single';
        acc.threads = Math.max(acc.threads, Number(row.threads) || 1);
        if (!acc.requested_input_mode && row.requested_input_mode) acc.requested_input_mode = String(row.requested_input_mode);
        if (!acc.input_mode && row.input_mode) acc.input_mode = String(row.input_mode);
        acc.retries = Math.max(acc.retries, Number(row.retries) || 0);
        acc.chunks += 1;
        acc.download_us += Number(row.download_us) || 0;
        acc.native_us += Number(row.native_us) || 0;
        acc.upload_us += Number(row.upload_us) || 0;
        acc.roots_plotted += Number(row.roots_plotted) || 0;
        acc.roots_clipped += Number(row.roots_clipped) || 0;
        acc.fragment_files_uploaded += Number(row.fragment_files_uploaded) || 0;
        acc.fragment_bytes_uploaded += Number(row.fragment_bytes_uploaded) || 0;
        acc.associated_palette_fragment_files_uploaded += Number(row.associated_palette_fragment_files_uploaded) || 0;
        acc.associated_palette_fragment_bytes_uploaded += Number(row.associated_palette_fragment_bytes_uploaded) || 0;
        const rowRgbSource = String(row.rgb_source || '').trim();
        if (!acc.rgb_source && rowRgbSource) acc.rgb_source = rowRgbSource;
        return acc;
    }, {
        engine: '',
        threads: 1,
        requested_input_mode: '',
        input_mode: '',
        retries: 0,
        chunks: 0,
        download_us: 0,
        native_us: 0,
        upload_us: 0,
        roots_plotted: 0,
        roots_clipped: 0,
        fragment_files_uploaded: 0,
        fragment_bytes_uploaded: 0,
        associated_palette_fragment_files_uploaded: 0,
        associated_palette_fragment_bytes_uploaded: 0,
        rgb_source: '',
    });
}

function _aggregateSolveScorePhasePerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (r.dl_ms != null || r.compute_ms != null || r.upload_ms != null)) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        acc.tasks += 1;
        acc.dl_ms += Number(row.dl_ms) || 0;
        acc.compute_ms += Number(row.compute_ms) || 0;
        acc.upload_ms += Number(row.upload_ms) || 0;
        acc.threads = Math.max(acc.threads, Number(row.threads) || 1);
        if (!acc.requested_input_mode && row.requested_input_mode) acc.requested_input_mode = String(row.requested_input_mode);
        if (!acc.input_mode && row.input_mode) acc.input_mode = String(row.input_mode);
        acc.retries = Math.max(acc.retries, Number(row.retries) || 0);
        return acc;
    }, { tasks: 0, dl_ms: 0, compute_ms: 0, upload_ms: 0, threads: 1, requested_input_mode: '', input_mode: '', retries: 0 });
}

function _formatRequestedEffectiveInputMode(perf) {
    if (!perf) return '';
    const requested = String(perf.requested_input_mode || '').trim();
    const effective = String(perf.input_mode || '').trim();
    if (requested && effective && requested !== effective) return `${requested}\u2192${effective}`;
    return effective || requested || '';
}

function _aggregateFinalizePerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (
        r.read_ms != null || r.read_progress_ms != null || r.assemble_ms != null || r.upload_ms != null ||
        r.pix_bytes != null || r.pix_files_piped != null || r.raw_expected_size != null || r.workers != null
    )) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        acc.tasks += 1;
        acc.read_ms += Number(row.read_ms != null ? row.read_ms : row.read_progress_ms) || 0;
        acc.assemble_ms += Number(row.assemble_ms) || 0;
        acc.upload_ms += Number(row.upload_ms) || 0;
        acc.workers = Math.max(acc.workers, Number(row.workers) || 0);
        if (!acc.rgb_source && row.rgb_source) acc.rgb_source = String(row.rgb_source);
        if (!acc.access_mode && row.access_mode) acc.access_mode = String(row.access_mode);
        if (!acc.assemble_mode && row.assemble_mode) acc.assemble_mode = String(row.assemble_mode);
        acc.raw_expected_size += Number(row.raw_expected_size) || 0;
        acc.raw_size += Number(row.raw_size) || 0;
        acc.n_chunks += Number(row.n_chunks) || 0;
        acc.pix_files += Number(row.pix_files != null ? row.pix_files : row.pix_files_piped) || 0;
        acc.pix_files_seen += Number(row.pix_files_seen) || 0;
        acc.pix_files_missing += Number(row.pix_files_missing != null ? row.pix_files_missing : row.missing_pix_files) || 0;
        acc.pix_bytes += Number(row.pix_bytes) || 0;
        return acc;
    }, {
        tasks: 0,
        read_ms: 0,
        assemble_ms: 0,
        upload_ms: 0,
        workers: 0,
        rgb_source: '',
        access_mode: '',
        assemble_mode: '',
        raw_expected_size: 0,
        raw_size: 0,
        n_chunks: 0,
        pix_files: 0,
        pix_files_seen: 0,
        pix_files_missing: 0,
        pix_bytes: 0,
    });
}

function _aggregateColorToBilevelPerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (
        r.dl_ms != null || r.render_ms != null || r.upload_ms != null ||
        r.source_size != null || r.file_size != null || r.threshold != null
    )) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        acc.tasks += 1;
        acc.dl_ms = Math.max(acc.dl_ms, Number(row.dl_ms) || 0);
        acc.render_ms = Math.max(acc.render_ms, Number(row.render_ms) || 0);
        acc.upload_ms = Math.max(acc.upload_ms, Number(row.upload_ms) || 0);
        acc.source_size = Math.max(acc.source_size, Number(row.source_size) || 0);
        acc.file_size = Math.max(acc.file_size, Number(row.file_size) || 0);
        if (row.threshold != null && row.threshold !== '') acc.threshold = Number(row.threshold);
        if (row.width != null && row.width !== '') acc.width = Math.max(acc.width, Number(row.width) || 0);
        if (row.height != null && row.height !== '') acc.height = Math.max(acc.height, Number(row.height) || 0);
        return acc;
    }, {
        tasks: 0,
        dl_ms: 0,
        render_ms: 0,
        upload_ms: 0,
        source_size: 0,
        file_size: 0,
        threshold: null,
        width: 0,
        height: 0,
    });
}

function _aggregateBilevelRasterPerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (
        r.dl_ms != null || r.raster_ms != null || r.fragment_size != null
    )) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        acc.tasks += 1;
        acc.dl_ms += Number(row.dl_ms) || 0;
        acc.raster_ms += Number(row.raster_ms) || 0;
        acc.fragment_bytes += Number(row.fragment_size) || 0;
        return acc;
    }, {
        tasks: 0,
        dl_ms: 0,
        raster_ms: 0,
        fragment_bytes: 0,
    });
}

function _aggregateBilevelFinalizePerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (
        r.prep_ms != null || r.assemble_ms != null || r.render_ms != null ||
        r.upload_ms != null || r.workers != null || r.raw_size != null ||
        r.file_size != null || r.pixels_set != null
    )) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        acc.tasks += 1;
        acc.prep_ms = Math.max(acc.prep_ms, Number(row.prep_ms) || 0);
        acc.assemble_ms = Math.max(acc.assemble_ms, Number(row.assemble_ms) || 0);
        acc.render_ms = Math.max(acc.render_ms, Number(row.render_ms) || 0);
        acc.upload_ms = Math.max(acc.upload_ms, Number(row.upload_ms) || 0);
        acc.workers = Math.max(acc.workers, Number(row.workers) || 0);
        acc.raw_size = Math.max(acc.raw_size, Number(row.raw_size) || 0);
        acc.file_size = Math.max(acc.file_size, Number(row.file_size) || 0);
        acc.pixels_set = Math.max(acc.pixels_set, Number(row.pixels_set) || 0);
        return acc;
    }, {
        tasks: 0,
        prep_ms: 0,
        assemble_ms: 0,
        render_ms: 0,
        upload_ms: 0,
        workers: 0,
        raw_size: 0,
        file_size: 0,
        pixels_set: 0,
    });
}

function _renderPhaseSubtaskInfo(run, rd) {
    if (!run || !rd) return null;
    const phase = rd.phase || '';
    if (rd.subtask_prefix && rd.expected) {
        return { prefix: rd.subtask_prefix, expected: rd.expected, phase };
    }
    if (phase === 'solve_score_clip') {
        return { prefix: `render_${run.run_id}_solve_score_clip`, expected: 1, phase };
    }
    if (phase === 'solve_score_merge') {
        return { prefix: `render_${run.run_id}_solve_score_merge`, expected: 1, phase };
    }
    return null;
}

function _renderPhasePerfSummary(phase, results, wallMs) {
    if (phase === 'bilevel_raster') {
        const perf = _aggregateBilevelRasterPerf(results);
        if (!perf) return `wall=${_fmtSecondsMs(wallMs)}`;
        let msg = `wall=${_fmtSecondsMs(wallMs)} total=dl ${_fmtSecondsMs(perf.dl_ms)} + native ${_fmtSecondsMs(perf.raster_ms)}`;
        if (perf.fragment_bytes > 0) msg += ` · fragments=${_fmtSize(perf.fragment_bytes)}`;
        return msg;
    }
    if (phase === 'bilevel_finalize') {
        const perf = _aggregateBilevelFinalizePerf(results);
        if (!perf) return `wall=${_fmtSecondsMs(wallMs)}`;
        let msg = `wall=${_fmtSecondsMs(wallMs)} total=prep ${_fmtSecondsMs(perf.prep_ms)} + assemble ${_fmtSecondsMs(perf.assemble_ms)} + render ${_fmtSecondsMs(perf.render_ms)} + up ${_fmtSecondsMs(perf.upload_ms)}`;
        if (perf.workers > 0) msg += ` · workers=${perf.workers}`;
        if (perf.raw_size > 0) msg += ` · raw=${_fmtSize(perf.raw_size)}`;
        if (perf.file_size > 0) msg += ` · tif=${_fmtSize(perf.file_size)}`;
        if (perf.pixels_set > 0) msg += ` · pixels=${perf.pixels_set}`;
        return msg;
    }
    if (phase === 'raster') {
        const perf = _aggregateRasterPerf(results);
        if (!perf) return '';
        let msg = `wall=${_fmtSecondsMs(wallMs)} total=dl ${_fmtSecondsUs(perf.download_us)} + native ${_fmtSecondsUs(perf.native_us)} + up ${_fmtSecondsUs(perf.upload_us)} · threads=${perf.threads}`;
        const inputLabel = _formatRequestedEffectiveInputMode(perf);
        if (inputLabel) msg += ` · input=${inputLabel}`;
        if (perf.input_mode === 'sectioned' || perf.input_mode === 'multispan_sectioned') msg += ` · retries=${perf.retries}`;
        if (perf.rgb_source) msg += ` · rgb=${perf.rgb_source}`;
        if (perf.fragment_bytes_uploaded > 0) msg += ` · fragments=${_fmtSize(perf.fragment_bytes_uploaded)}`;
        return msg;
    }
    if (phase === 'associated_palette_chunk') {
        const perf = _aggregatePaletteChunkPerf(results);
        if (!perf) return `wall=${_fmtSecondsMs(wallMs)}`;
        let msg = `wall=${_fmtSecondsMs(wallMs)} total=dl ${_fmtSecondsMs(perf.dl_ms)} + native ${_fmtSecondsMs(perf.compute_ms)} + up ${_fmtSecondsMs(perf.upload_ms)} · threads=${perf.threads}`;
        const inputLabel = _formatRequestedEffectiveInputMode(perf);
        if (inputLabel) msg += ` · input=${inputLabel}`;
        if (perf.input_mode === 'sectioned' || perf.input_mode === 'multispan_sectioned') msg += ` · retries=${perf.retries}`;
        if (perf.workers > 0) msg += ` · workers=${perf.workers}`;
        if (perf.step_count > 0) msg += ` · solves=${perf.step_count}`;
        return msg;
    }
    if (phase === 'solve_score_clip' || phase === 'solve_score_hist' || phase === 'solve_score_merge') {
        const perf = _aggregateSolveScorePhasePerf(results);
        if (!perf) return `wall=${_fmtSecondsMs(wallMs)}`;
        let msg = `wall=${_fmtSecondsMs(wallMs)} total=dl ${_fmtSecondsMs(perf.dl_ms)} + compute ${_fmtSecondsMs(perf.compute_ms)}`;
        if (perf.upload_ms > 0) msg += ` + up ${_fmtSecondsMs(perf.upload_ms)}`;
        msg += ` · threads=${perf.threads}`;
        if (phase === 'solve_score_hist') {
            const inputLabel = _formatRequestedEffectiveInputMode(perf);
            if (inputLabel) msg += ` · input=${inputLabel}`;
            if (perf.input_mode === 'sectioned' || perf.input_mode === 'multispan_sectioned') msg += ` · retries=${perf.retries}`;
        }
        return msg;
    }
    if (phase === 'finalize' || phase === 'finalize_mt') {
        const perf = _aggregateFinalizePerf(results);
        if (!perf) return `wall=${_fmtSecondsMs(wallMs)}`;
        let msg = `wall=${_fmtSecondsMs(wallMs)} total=read ${_fmtSecondsMs(perf.read_ms)} + assemble ${_fmtSecondsMs(perf.assemble_ms)} + up ${_fmtSecondsMs(perf.upload_ms)}`;
        if (perf.workers > 0) msg += ` · workers=${perf.workers}`;
        if (perf.rgb_source) msg += ` · rgb=${perf.rgb_source}`;
        if (perf.access_mode) msg += ` · access=${perf.access_mode}`;
        if (perf.assemble_mode) msg += ` · assemble=${perf.assemble_mode}`;
        const fragmentCount = perf.pix_files || perf.pix_files_seen;
        if (perf.n_chunks > 0) msg += ` · fragments=${fragmentCount}/${perf.n_chunks}`;
        if (perf.pix_files_missing > 0) msg += ` missing=${perf.pix_files_missing}`;
        if (perf.pix_bytes > 0) msg += ` · pix=${_fmtSize(perf.pix_bytes)}`;
        const rawBytes = perf.raw_size || perf.raw_expected_size;
        if (rawBytes > 0) msg += ` · raw=${_fmtSize(rawBytes)}`;
        return msg;
    }
    if (
        phase === 'bilevel_from_raw_prepare' ||
        phase === 'bilevel_from_raw_download' ||
        phase === 'bilevel_from_raw_render' ||
        phase === 'bilevel_from_raw_upload'
    ) {
        const perf = _aggregateColorToBilevelPerf(results);
        if (!perf) return `wall=${_fmtSecondsMs(wallMs)}`;
        let msg = `wall=${_fmtSecondsMs(wallMs)}`;
        if (perf.dl_ms > 0 || perf.render_ms > 0 || perf.upload_ms > 0) {
            msg += ` total=dl ${_fmtSecondsMs(perf.dl_ms)} + render ${_fmtSecondsMs(perf.render_ms)} + up ${_fmtSecondsMs(perf.upload_ms)}`;
        }
        if (perf.width > 0 && perf.height > 0) msg += ` · size=${perf.width}x${perf.height}`;
        if (perf.source_size > 0) msg += ` · raw=${_fmtSize(perf.source_size)}`;
        if (perf.file_size > 0) msg += ` · tif=${_fmtSize(perf.file_size)}`;
        if (perf.threshold != null && Number.isFinite(perf.threshold)) msg += ` · threshold=${perf.threshold}`;
        return msg;
    }
    if (phase === 'associated_palette_finalize') {
        const perf = _aggregatePaletteFinalizePerf(results);
        if (!perf) return `wall=${_fmtSecondsMs(wallMs)}`;
        let msg = `wall=${_fmtSecondsMs(wallMs)} total=assemble ${_fmtSecondsMs(perf.assemble_ms)} + render ${_fmtSecondsMs(perf.render_ms)} + encode ${_fmtSecondsMs(perf.encode_ms)}`;
        if (perf.pass0_chunk_count > 0) msg += ` · pass0=${perf.pass0_chunks_read}/${perf.pass0_chunk_count}`;
        if (perf.pass0_chunks_skipped > 0) msg += ` skipped=${perf.pass0_chunks_skipped}`;
        if (perf.pass0_bytes_read > 0) msg += ` · bins=${_fmtSize(perf.pass0_bytes_read)}`;
        if (perf.file_size > 0) msg += ` · size ${(perf.file_size / 1e6).toFixed(1)}MB`;
        return msg;
    }
    return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
}

function _renderPhaseProgressSignature(phase, results) {
    if (phase === 'bilevel_finalize') {
        const perf = _aggregateBilevelFinalizePerf(results);
        if (!perf) return '';
        return [
            perf.prep_ms || 0,
            perf.assemble_ms || 0,
            perf.render_ms || 0,
            perf.upload_ms || 0,
            perf.raw_size || 0,
            perf.file_size || 0,
            perf.pixels_set || 0,
        ].join(':');
    }
    if (phase === 'bilevel_from_raw_render') {
        const perf = _aggregateColorToBilevelPerf(results);
        if (!perf) return '';
        return [perf.render_ms || 0, perf.file_size || 0].join(':');
    }
    if (phase !== 'finalize' && phase !== 'finalize_mt') return '';
    const perf = _aggregateFinalizePerf(results);
    if (!perf) return '';
    return [
        perf.pix_files || perf.pix_files_seen || 0,
        perf.pix_bytes || 0,
        perf.raw_size || 0,
        perf.assemble_ms || 0,
        perf.upload_ms || 0,
    ].join(':');
}

function _updateRenderPhaseTracker(run, rd) {
    if (!run || !rd) return;
    const phase = rd.phase || rd.phase_label || '';
    const startedAtMs = _coerceTimestampMs(rd.started_at_ms) || _coerceTimestampMs(rd.updated_at_ms) || Date.now();
    const updatedAtMs = _coerceTimestampMs(rd.updated_at_ms) || startedAtMs;
    if (!_renderPhaseTracker || _renderPhaseTracker.phase !== phase) {
        const info = _renderPhaseSubtaskInfo(run, rd);
        _renderPhaseTracker = {
            phase,
            phase_label: rd.phase_label || phase || 'working',
            started_at_ms: startedAtMs,
            last_server_update_ms: updatedAtMs,
            prefix: info ? info.prefix : '',
            expected: info ? info.expected : 0,
        };
    } else if (updatedAtMs != null) {
        _renderPhaseTracker.last_server_update_ms = updatedAtMs;
    }
}

async function _logRenderPhaseCompletion(run, tracker) {
    if (!run || !tracker || !tracker.prefix) return;
    const completionKey = `${run.run_id || run.task_id || ''}:${tracker.phase || tracker.phase_label || tracker.prefix}`;
    if (_renderLoggedPhaseCompletions.has(completionKey)) return;
    _renderLoggedPhaseCompletions.add(completionKey);
    try {
        const subcheck = await lambdaPost('storage', {
            job_id: run.job_id,
            task_prefix: tracker.prefix,
            expected: tracker.expected || 1,
        }, '/check-status');
        const wallMs = _serverElapsedMs(
            tracker.started_at_ms,
            _pickLatestTimestampMs(
                subcheck.latest_done_ms,
                subcheck.latest_update_ms,
                tracker.last_server_update_ms
            )
        );
        const perfText = _renderPhasePerfSummary(tracker.phase, subcheck.results || [], wallMs);
        if (perfText) {
            log(`${tracker.phase_label} complete · ${perfText}`, '', 'render-log');
        } else {
            _renderLoggedPhaseCompletions.delete(completionKey);
        }
    } catch (e) {
        _renderLoggedPhaseCompletions.delete(completionKey);
        // best effort only
    }
}

async function _logRenderTerminalPhaseCatchup(run) {
    if (!run || run.mode !== 'color') return;
}

async function _logRenderRasterPerf(run) {
    if (!run || run.mode !== 'color') return;
    try {
        const subcheck = await lambdaPost('storage', {
            job_id: run.job_id,
            task_prefix: 'render_' + run.run_id + '_raster_',
            expected: 1000000,
        }, '/check-status');
        const perf = _aggregateRasterPerf(subcheck.results || []);
        if (!perf) return;
        const wallMs = _renderPhaseTracker && _renderPhaseTracker.phase === 'raster'
            ? (_serverElapsedMs(
                _renderPhaseTracker.started_at_ms,
                _pickLatestTimestampMs(
                    subcheck.latest_done_ms,
                    subcheck.latest_update_ms,
                    _renderPhaseTracker.last_server_update_ms
                )
            ) || 0)
            : 0;
        const artifactLabel = 'Fragment';
        const lines = [
            `Raster performance: engine=${perf.engine || 'single'} threads=${perf.threads}${perf.input_mode ? ` input=${perf.input_mode}` : ''} chunks=${perf.chunks} wall=${_fmtSecondsMs(wallMs)}`,
            `  Download time: ${_fmtSecondsUs(perf.download_us)}`,
            `  Native raster time: ${_fmtSecondsUs(perf.native_us)}`,
            `  Upload time: ${_fmtSecondsUs(perf.upload_us)}`,
            `  Emitted root hits: ${perf.roots_plotted}`,
            `  Clipped roots: ${perf.roots_clipped}`,
        ];
        if (perf.fragment_files_uploaded > 0) {
            lines.push(`  ${artifactLabel} files uploaded: ${perf.fragment_files_uploaded}`);
        }
        if (perf.fragment_bytes_uploaded > 0) {
            const avgBytes = perf.fragment_files_uploaded > 0 ? perf.fragment_bytes_uploaded / perf.fragment_files_uploaded : 0;
            lines.push(
                `  ${artifactLabel} sparse bytes uploaded: ${_fmtSize(perf.fragment_bytes_uploaded)}`
                + ` (avg/file ${_fmtSize(avgBytes)})`
            );
        }
        if (perf.associated_palette_fragment_files_uploaded > 0) {
            lines.push(`  Associated palette fragments uploaded: ${perf.associated_palette_fragment_files_uploaded}`);
        }
        if (perf.associated_palette_fragment_bytes_uploaded > 0) {
            lines.push(`  Associated palette fragment bytes uploaded: ${_fmtSize(perf.associated_palette_fragment_bytes_uploaded)}`);
        }
        if (perf.rgb_source) {
            lines.push(`  RGB source: ${perf.rgb_source || 'pix'}`);
        }
        const msg = lines.join('\n');
        log(msg, '', 'render-log');
    } catch (e) {
        // perf logging is best-effort only
    }
}

function _renderRunCompleteLabel(run) {
    if (!run) return 'Render complete';
    if (run.mode === 'repalette') return 'RePalette complete';
    if (run.mode === 'color_repalette') return 'Color RePalette complete';
    if (run.mode === 'color_to_bilevel') return 'Color2Bilevel complete';
    if (run.mode === 'autolevels') return 'Autolevels complete';
    if (run.mode === 'resize') return 'Resize complete';
    if (run.mode === 'pdf') return 'PDF complete';
    return 'Render complete';
}

function _renderRunErrorLabel(run) {
    if (!run) return 'Render';
    if (run.mode === 'repalette') return 'RePalette';
    if (run.mode === 'color_repalette') return 'Color RePalette';
    if (run.mode === 'color_to_bilevel') return 'Color2Bilevel';
    if (run.mode === 'autolevels') return 'Autolevels';
    if (run.mode === 'resize') return 'Resize';
    if (run.mode === 'pdf') return 'PDF';
    return 'Render';
}

function _formatTaskErrorContext(detail) {
    const rd = detail && detail.result_data;
    if (!rd || typeof rd !== 'object') return '';
    const parts = [];
    if (detail.task_id) parts.push(`task=${detail.task_id}`);
    if (rd.artifact_id) parts.push(`artifact=${rd.artifact_id}`);
    if (rd.source_artifact_id) parts.push(`source_artifact=${rd.source_artifact_id}`);
    if (rd.phase) parts.push(`phase=${rd.phase}`);
    if (rd.section_idx !== undefined && rd.section_idx !== null) parts.push(`section=${rd.section_idx}`);
    else if (rd.chunk_idx !== undefined && rd.chunk_idx !== null) parts.push(`chunk=${rd.chunk_idx}`);
    if (rd.metric) parts.push(`metric=${rd.metric}`);
    if (rd.threshold !== undefined && rd.threshold !== null && rd.threshold !== '') parts.push(`threshold=${rd.threshold}`);
    if (rd.threshold_raw !== undefined && rd.threshold_raw !== null && rd.threshold_raw !== '') parts.push(`threshold_raw=${rd.threshold_raw}`);
    if (rd.engine) parts.push(`engine=${rd.engine}`);
    if (rd.target_size !== undefined && rd.target_size !== null) parts.push(`target=${rd.target_size}`);
    if (rd.input_mode) parts.push(`input=${rd.input_mode}`);
    if (rd.threads !== undefined && rd.threads !== null) parts.push(`threads=${rd.threads}`);
    if (rd.workers !== undefined && rd.workers !== null) parts.push(`workers=${rd.workers}`);
    if (rd.source_bucket && rd.source_key) parts.push(`source=s3://${rd.source_bucket}/${rd.source_key}`);
    else if (rd.source_key) parts.push(`source=${rd.source_key}`);
    if (rd.source_size !== undefined && rd.source_size !== null) parts.push(`size=${rd.source_size}`);
    if (rd.clip_key) parts.push(`clip=${rd.clip_key}`);
    return parts.join(' · ');
}

function _renderRunCompleteLog(run, rd) {
    const target = rd.artifact_id || rd.palette_id || rd.image_key || run.mode;
    const elapsedMs = _serverElapsedMs(
        rd && rd.run_started_at_ms ? rd.run_started_at_ms : (run ? run.server_started_at_ms : null),
        rd && rd.updated_at_ms ? rd.updated_at_ms : null
    );
    const fallbackElapsedMs = elapsedMs == null && run && run.started_at_ms
        ? Math.max(0, Date.now() - run.started_at_ms)
        : null;
    const shownElapsedMs = elapsedMs != null ? elapsedMs : fallbackElapsedMs;
    const elapsed = shownElapsedMs != null ? ` (${_fmtMs(shownElapsedMs)})` : '';
    if (run && run.mode === 'repalette') return 'RePalette complete: ' + target + elapsed;
    if (run && run.mode === 'color_repalette') return 'Color RePalette complete: ' + target + elapsed;
    if (run && run.mode === 'color_to_bilevel') return 'Color2Bilevel complete: ' + target + elapsed;
    if (run && run.mode === 'autolevels') return 'Autolevels complete: ' + target + elapsed;
    if (run && run.mode === 'resize') return 'Resize complete: ' + target + elapsed;
    if (run && run.mode === 'pdf') return 'PDF complete: ' + target + elapsed;
    return 'Render complete: ' + target + elapsed;
}

async function _handleRenderRunCompletion(run, rd) {
    const runKey = run && (run.run_id || run.task_id || '');
    if (runKey && _renderCompletionRunId === runKey) return;
    _renderCompletionRunId = runKey;
    stopActiveRenderObserver();

    const statusEl = document.getElementById('render-status');
    const label = _renderRunCompleteLabel(run);
    if (statusEl) {
        statusEl.textContent = label;
        statusEl.className = 'status ok';
    }
    if (_renderPhaseTracker) {
        await _logRenderPhaseCompletion(run, _renderPhaseTracker);
    }
    await _logRenderTerminalPhaseCatchup(run);
    await _logRenderRasterPerf(run);
    log(_renderRunCompleteLog(run, rd), 'ok', 'render-log');
    const autoDbg = _fmtAutolevelDebugLine(rd);
    if (autoDbg) log('  autolevels: ' + autoDbg, '', 'render-log');
    const resizeDbg = _fmtResizeDebugLine(rd);
    if (resizeDbg) log('  resize: ' + resizeDbg, '', 'render-log');

    _clearActiveRun('done', _renderRunCompleteLog(run, rd));

    const family = rd.family || (run.mode === 'coeff_bilevel' ? 'coeffs' : (run.mode === 'repalette' ? 'palette' : (run.mode === 'color_repalette' ? 'color' : (run.mode === 'color_to_bilevel' ? 'bilevel' : run.mode))));
    const artifactId = rd.artifact_id || rd.palette_id || null;

    if (family === 'palette') {
        const paletteJobEl = document.getElementById('palette-results-dir');
        if (paletteJobEl) paletteJobEl.value = run.job_id;
        await loadPaletteInventory({ selectPaletteId: rd.palette_id || rd.artifact_id || null });
    }

    await refreshRenderArtifacts(run.job_id, {
        selectFamily: family,
        selectArtifactId: artifactId,
    });
}

function _showPdfHardStaleAbandon(statusEl, run, phase) {
    if (!statusEl) return;
    statusEl.innerHTML = '';
    const text = document.createElement('span');
    text.textContent = 'PDF compose stalled (no update for 15+ min)';
    statusEl.appendChild(text);
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className = 'btn-secondary btn-inline';
    btn.style.marginLeft = '8px';
    btn.textContent = 'Abandon stalled PDF job';
    btn.onclick = () => {
        const target = (run && (run.task_id || run.run_id)) || 'PDF job';
        _clearActiveRun('failed', 'PDF compose stalled; abandoned locally');
        stopActiveRenderObserver();
        statusEl.textContent = 'PDF job abandoned locally';
        statusEl.className = 'status';
        log(`Abandoned stalled PDF job locally: ${target}`, 'err', 'render-log');
    };
    statusEl.appendChild(btn);
    statusEl.className = 'status error';
    if (_lastWarnState !== 'hard') {
        _lastWarnState = 'hard';
        log(`PDF compose stalled - no update for 15+ min${phase ? ' (' + phase + ')' : ''}`, 'err', 'render-log');
    }
}

async function _pollActiveRenderRun() {
    if (_renderObserverPollActive) return;
    _renderObserverPollActive = true;
    const run = _activeRenderRun || _loadActiveRun();
    if (!run) {
        _renderObserverPollActive = false;
        stopActiveRenderObserver();
        return;
    }
    const statusEl = document.getElementById('render-status');

    try {
        const check = await lambdaPost('storage', {
            job_id: run.job_id, task_prefix: run.task_id, expected: 1,
        }, '/check-status');

        // Terminal error
        if (check.errors > 0) {
            _logContractWarnings(check.error_details || [], 'render-log');
            const detail = check.error_details?.[0] || {};
            const msg = detail.error_msg || 'unknown error';
            const ctx = _formatTaskErrorContext(detail);
            const errorLabel = _renderRunErrorLabel(run);
            statusEl.textContent = errorLabel + ' error: ' + msg;
            statusEl.className = 'status error';
            log(errorLabel + ' failed: ' + msg, 'err', 'render-log');
            if (ctx) log('  context: ' + ctx, 'err', 'render-log');
            _clearActiveRun('failed', errorLabel + ' error: ' + msg);
            stopActiveRenderObserver();
            return;
        }

        const rd = check.results?.[0] || {};
        _logContractWarnings([rd], 'render-log');
        if (rd.execution_arn) {
            _jobsRailUpsert({ id: 'render:' + run.run_id, executionArn: rd.execution_arn, taskId: run.task_id });
        }
        _syncRenderRunServerStart(run, rd);

        // No DDB row found — abandoned run
        if (!rd.phase && !rd.phase_label) {
            const age_ms = Date.now() - ((run.server_started_at_ms || run.started_at_ms) || 0);
            if (age_ms > RENDER_NO_ROW_STALE_MS) {
                statusEl.textContent = 'Ready';
                statusEl.className = 'status';
                _clearActiveRun('done', 'no status row; assumed finished');
                stopActiveRenderObserver();
                return;
            }
        }

        const phase = rd.phase_label || rd.phase || 'working';
        const phaseCode = rd.phase || phase;

        // Terminal done
        if (check.complete || rd.phase === 'done') {
            await _handleRenderRunCompletion(run, rd);
            return;
        }

        // Secondary poll for subtask counts + freshness
        let done = 0, expected = 0, workerFreshMs = null, subResults = [], phaseLatestMs = _pickLatestTimestampMs(rd.updated_at_ms, check.latest_update_ms);
        if (rd.subtask_prefix && rd.expected) {
            try {
                const subcheck = await lambdaPost('storage', {
                    job_id: run.job_id, task_prefix: rd.subtask_prefix, expected: rd.expected,
                }, '/check-status');
                done = subcheck.done || 0;
                expected = subcheck.expected || rd.expected || 0;
                subResults = subcheck.results || [];
                _logContractWarnings(subResults, 'render-log');
                phaseLatestMs = _pickLatestTimestampMs(
                    subcheck.latest_done_ms,
                    subcheck.latest_update_ms,
                    subcheck.latest_nonterminal_ms,
                    phaseLatestMs
                );
                if (subcheck.latest_update_ms) {
                    workerFreshMs = Date.now() - subcheck.latest_update_ms;
                }
            } catch(e) {
                expected = rd.expected || 0;
            }
        } else {
            const phaseInfo = _renderPhaseSubtaskInfo(run, rd);
            if (phaseInfo && phaseInfo.prefix) {
                try {
                    const subcheck = await lambdaPost('storage', {
                    job_id: run.job_id, task_prefix: phaseInfo.prefix, expected: phaseInfo.expected || 1,
                }, '/check-status');
                done = subcheck.done || 0;
                expected = subcheck.expected || phaseInfo.expected || 0;
                subResults = subcheck.results || [];
                _logContractWarnings(subResults, 'render-log');
                phaseLatestMs = _pickLatestTimestampMs(
                    subcheck.latest_done_ms,
                    subcheck.latest_update_ms,
                    subcheck.latest_nonterminal_ms,
                    phaseLatestMs
                );
                if (subcheck.latest_update_ms) {
                    workerFreshMs = Date.now() - subcheck.latest_update_ms;
                }
                } catch (e) {
                    expected = phaseInfo.expected || 0;
                }
            }
        }

        if (_renderPhaseTracker && _renderPhaseTracker.phase !== phaseCode) {
            await _logRenderPhaseCompletion(run, _renderPhaseTracker);
        }
        _updateRenderPhaseTracker(run, rd);
        const phaseWallMs = _renderPhaseTracker
            ? (_serverElapsedMs(
                _renderPhaseTracker.started_at_ms,
                _pickLatestTimestampMs(
                    phaseLatestMs,
                    _renderPhaseTracker.last_server_update_ms
                )
            ) || 0)
            : 0;
        const phasePerfRows = subResults.length ? subResults : [rd];
        const phasePerfText = _renderPhasePerfSummary(phaseCode, phasePerfRows, phaseWallMs);

        // Determine liveness signal
        // Primary: worker freshness from secondary poll
        // Fallback: top-level row updated_at_ms (for non-subtask phases)
        let freshMs = workerFreshMs;
        if (freshMs === null && check.latest_update_ms) {
            freshMs = Date.now() - check.latest_update_ms;
        }

        // Build status text
        let statusMsg = phase;
        if (expected > 0) statusMsg += ' ' + done + '/' + expected;
        if (phasePerfText) statusMsg += ' · ' + phasePerfText;
        if (freshMs !== null && freshMs > 10000) {
            statusMsg += ' \u00b7 last update ' + _fmtAge(freshMs) + ' ago';
        }
        _jobsRailUpsert({
            id: 'render:' + run.run_id,
            kind: 'render',
            label: (run.mode || 'render') + ' · ' + run.job_id,
            jobId: run.job_id,
            tab: 'render',
            state: 'running',
            startedAt: run.started_at_ms || undefined,
            detail: statusMsg,
        });

        // Liveness: warning / hard stall. PDF is a single Lambda job, so a
        // hard-stale compose phase can leave only a local active-run lock.
        if (freshMs !== null && freshMs > RENDER_HARD_STALE_MS) {
            if (run && run.mode === 'pdf') {
                _showPdfHardStaleAbandon(statusEl, run, phase);
            } else {
                statusEl.textContent = 'Render stalled (no worker update for 15+ min)';
                statusEl.className = 'status error';
                if (_lastWarnState !== 'hard') {
                    _lastWarnState = 'hard';
                    log('Render stalled — no worker update for 15+ min', 'err', 'render-log');
                }
            }
        } else if (freshMs !== null && freshMs > RENDER_WARN_STALE_MS) {
            statusEl.textContent = run && run.mode === 'pdf'
                ? statusMsg + ' \u00b7 PDF compose has not updated for 5+ min'
                : statusMsg + ' \u00b7 no worker update for 5+ min';
            statusEl.className = 'status';
            if (_lastWarnState !== 'warn') {
                _lastWarnState = 'warn';
                log(run && run.mode === 'pdf'
                    ? 'Warning: PDF compose has not updated for 5+ min'
                    : 'Warning: no worker update for 5+ min', '', 'render-log');
            }
        } else {
            statusEl.textContent = statusMsg;
            statusEl.className = 'status';
            // Recovery from warn/hard
            if (_lastWarnState) {
                log('Workers active again', 'ok', 'render-log');
                _lastWarnState = null;
            }
        }

        // Log phase transitions once
        const progressSignature = _renderPhaseProgressSignature(phaseCode, phasePerfRows);
        const phaseKey = String(phase) + '_' + done + (progressSignature ? '_' + progressSignature : '');
        if (phaseKey !== _lastLoggedPhase) {
            _lastLoggedPhase = phaseKey;
            let logMsg = phase;
            if (expected > 0) logMsg += ' ' + done + '/' + expected;
            if (phasePerfText) logMsg += ' · ' + phasePerfText;
            log(logMsg, '', 'render-log');
        }

    } catch (e) {
        // Poll error — keep trying
    } finally {
        _renderObserverPollActive = false;
    }
}

// Visibility/focus handlers for observer
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// Restore active runs on page load
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

/* ---- Render artifact discovery and UI rendering ---- */

async function refreshRenderInventory() {
    const jobId = document.getElementById('render-results-dir').value.trim();
    const btn = document.getElementById('btn-render-refresh');
    const orig = btn ? btn.textContent : 'Refresh';
    if (!jobId) {
        log('Render refresh failed: no results dir selected', 'err', 'render-log');
        const statusEl = document.getElementById('render-status');
        if (statusEl) {
            statusEl.textContent = 'Refresh failed: no results dir selected';
            statusEl.className = 'status error';
        }
        return;
    }
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Refreshing...'; }
        await refreshRenderArtifacts(jobId, { throwOnError: true });
        log(`Render refreshed: ${jobId}`, 'ok', 'render-log');
    } catch (e) {
        log(`Render refresh failed: ${e.message}`, 'err', 'render-log');
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

async function refreshRenderArtifacts(jobIdOverride, options = {}) {
    const jobId = jobIdOverride || document.getElementById('render-results-dir').value.trim();
    if (!jobId) return;
    if (_renderLoadedJobId && _renderLoadedJobId !== jobId) _clearRenderPreviewSelection();
    _captureRenderCatalogScroll(_renderActiveFamily);
    const statusEl = document.getElementById('render-status');
    const prevStatus = statusEl.textContent;
    const prevClass = statusEl.className;
    statusEl.textContent = 'Loading...';
    statusEl.className = 'status';
    try {
        await _refreshRenderSummaryData(jobId, options);
        if (!_activeRenderRun) {
            statusEl.textContent = 'Ready';
            statusEl.className = 'status ok';
        } else {
            statusEl.textContent = prevStatus;
            statusEl.className = prevClass;
        }
    } catch (e) {
        statusEl.textContent = 'Refresh failed: ' + e.message;
        statusEl.className = 'status error';
        if (options && options.throwOnError) throw e;
    }
}

async function _refreshRenderSummaryData(jobId, options = {}) {
    const summary = await lambdaPost('storage', { job_id: jobId }, '/render-summary');
    if (options.selectFamily) _renderActiveFamily = options.selectFamily;
    if (options.selectArtifactId && summary.families && summary.families[_renderActiveFamily]) {
        const inv = summary.families[_renderActiveFamily] || [];
        const matchIdx = inv.findIndex(a => a.artifact_id === options.selectArtifactId || a.palette_id === options.selectArtifactId);
        if (matchIdx >= 0) {
            _renderSelectedArtifact[_renderActiveFamily] = matchIdx;
            _renderSelectedArtifactKey[_renderActiveFamily] = _renderArtifactStableKey({ ...inv[matchIdx], family: _renderActiveFamily });
        }
    }
    _renderLoadedJobId = jobId;
    renderArtifactPanel(jobId, summary, {
        preserveScroll: true,
        ensureSelected: !!options.selectArtifactId,
    });
    _renderNeedsRefresh = false;
    return summary;
}
function _fmtSize(bytes) {
    if (!bytes || bytes <= 0) return '';
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(0) + ' KB';
    return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
}

function _renderArtifactStableKey(art) {
    if (!art) return '';
    return `${String(art.family || '')}::${String(art.artifact_id || art.palette_id || art.image_key || art.preview_key || '')}`;
}

function _renderFamilyLabel(family) {
    if (family === 'bilevel') return 'BiLevel';
    if (family === 'coeffs') return 'Coeffs';
    if (family === 'palette') return 'Palette';
    if (family === 'pdf') return 'PDF';
    return 'Color';
}

function _renderSelectedArtifactEntry() {
    const inv = _renderArtifacts[_renderActiveFamily] || [];
    const idx = _renderSelectedArtifact[_renderActiveFamily];
    if (idx == null || idx < 0 || idx >= inv.length) return null;
    return inv[idx];
}


/* ---- Jobs rail: persistent cross-tab job surface (poll-driven) ----
   Cards are fed by the existing run lifecycles (render/palette
   orchestrator runs, compute submissions) — no new backend. The pollers
   upsert full records every tick, so runs resumed from localStorage
   (which bypass _saveActive*Run) still materialize. Terminal cards
   persist in localStorage; a rail failure must never break a dispatch
   or poll path, so all DOM/storage work is guarded. */
const JOBS_RAIL_HISTORY_KEY = 'polypaint_jobs_rail';
const JOBS_RAIL_COLLAPSED_KEY = 'polypaint_jobs_rail_collapsed';
const JOBS_RAIL_MAX = 12;
let _jobsRailJobs = [];

function _jobsRailValidRecord(job) {
    return !!(job && typeof job === 'object' && typeof job.id === 'string' && job.id);
}

function _jobsRailLoadHistory() {
    try {
        const s = localStorage.getItem(JOBS_RAIL_HISTORY_KEY);
        if (s) {
            const parsed = JSON.parse(s);
            if (Array.isArray(parsed)) {
                return parsed.filter(_jobsRailValidRecord).slice(0, JOBS_RAIL_MAX);
            }
        }
    } catch (e) {}
    return [];
}

function _jobsRailPersistHistory() {
    try {
        localStorage.setItem(JOBS_RAIL_HISTORY_KEY, JSON.stringify(
            _jobsRailJobs.filter(j => j.state !== 'running').slice(0, JOBS_RAIL_MAX)
        ));
    } catch (e) {}
}

function _jobsRailCollapsed() {
    try { return localStorage.getItem(JOBS_RAIL_COLLAPSED_KEY) === '1'; } catch (e) { return false; }
}

function _jobsRailUpsert(job) {
    if (!_jobsRailValidRecord(job)) return;
    if (_jobsRailDismissed.has(job.id)) {
        const dismissedStart = _jobsRailDismissed.get(job.id);
        if (job.startedAt != null && job.startedAt !== dismissedStart) {
            // a DIFFERENT startedAt = genuinely new run of the same id
            _jobsRailDismissed.delete(job.id);
        } else if (job.state === 'running' || job.state == null) {
            return;                          // suppressed while dismissed
        } else {
            // terminal: the user dismissed this run — retire the
            // tombstone silently rather than resurrecting a stub card
            _jobsRailDismissed.delete(job.id);
            return;
        }
    }
    const idx = _jobsRailJobs.findIndex(j => j.id === job.id);
    const prev = idx >= 0 ? _jobsRailJobs[idx] : null;
    const next = prev ? { ...prev } : { startedAt: Date.now() };
    for (const key of Object.keys(job)) {
        // Skip undefined so a sparse patch never clobbers known fields, and
        // keep the last progress line when a terminal patch has no message.
        if (job[key] === undefined) continue;
        if (key === 'detail' && job[key] === '' && prev && prev.detail) continue;
        next[key] = job[key];
    }
    next.updatedAt = Date.now();
    if (prev) {
        // In-place update: no reorder churn from 3-second poll ticks.
        _jobsRailJobs[idx] = next;
    } else {
        _jobsRailJobs.unshift(next);
        if (_jobsRailJobs.length > JOBS_RAIL_MAX) {
            // Evict oldest terminal cards first; running cards are never
            // dropped (a dropped running card would go silent mid-run).
            for (let i = _jobsRailJobs.length - 1; i >= 0 && _jobsRailJobs.length > JOBS_RAIL_MAX; i--) {
                if (_jobsRailJobs[i].state !== 'running') _jobsRailJobs.splice(i, 1);
            }
        }
    }
    if (next.state !== 'running') _jobsRailPersistHistory();
    _renderJobsRail();
}

function _jobsRailProgress(id, detail) {
    const job = _jobsRailJobs.find(j => j.id === id);
    if (!job || job.state !== 'running') return;
    job.detail = String(detail || '');
    job.updatedAt = Date.now();
    _renderJobsRail();
}

const _JOBS_RAIL_KILL_TARGETS = {
    compute: 'compute_orchestrator',
    render: 'render_orchestrator',
    palette: 'palette_orchestrator',
};

const _jobsRailDismissed = new Map();   // CR35-F27: id -> startedAt at dismissal

function _jobsRailDismiss(id) {
    const idx = _jobsRailJobs.findIndex(j => j.id === id);
    if (idx < 0) return;
    // a dismissed LIVE card must stay gone: its poll loop keeps
    // upserting every interval (render/compute polls even re-send the
    // run's ORIGINAL startedAt each tick), so the tombstone remembers
    // which startedAt was dismissed — only a run with a DIFFERENT
    // startedAt counts as new.
    if (_jobsRailJobs[idx].state === 'running') {
        _jobsRailDismissed.set(id, _jobsRailJobs[idx].startedAt || 0);
    }
    _jobsRailJobs.splice(idx, 1);
    _jobsRailPersistHistory();
    _renderJobsRail();
}

async function _jobsRailKill(id) {
    const job = _jobsRailJobs.find(j => j.id === id);
    if (!job || job.state !== 'running') return;
    if (String(job.id).startsWith('sheet:')) {
        // sheets have no Step Functions execution — the kill is the S3
        // cancel marker; workers stop between frames and the run's poll
        // finishes the card through its normal error path. The marker is
        // GENERATION-scoped: the card carries the generation (a cancel
        // without it is rejected server-side, and async dispatch would
        // report success while the worker refused — the wave-B bug).
        if (!job.generation) {
            job.detail = 'cancel unavailable: run generation unknown (pre-upgrade card)';
            _renderJobsRail();
            return;
        }
        if (typeof window !== 'undefined' && typeof window.confirm === 'function'
            && !window.confirm(`Stop this sheet run?\n${job.label || job.id}`)) return;
        job.killRequested = true;
        job.detail = 'cancel requested…';
        _renderJobsRail();
        // round-12 finding 5 + round-13 finding 2: route through the DURABLE,
        // identity-scoped cancellation command as a DIRECT command — it
        // ESTABLISHES the cancel intent for this run (even without a matching
        // Sheets descriptor) and retries the authoritative transition until
        // run.json goes terminal. Use its STRUCTURED result to update the
        // card rather than leaving it stuck at "cancel requested…".
        try {
            let result;
            if (typeof _cancelSheetRun === 'function') {
                result = await _cancelSheetRun(job.jobId, job.generation, { direct: true });
            } else {
                const resp = await lambdaPost('dispatch', {
                    target: 'poly_sheet',
                    jobs: [{ action: 'cancel', sheet_id: job.jobId,
                             generation: job.generation }],
                    expected_keys: [],
                });
                if ((resp.fired || 0) !== 1) throw new Error('cancel dispatch failed');
                result = { ok: true, pending: true };
            }
            if (result && result.ok === false && !result.pending) {
                // not accepted at all — surface it so the user can retry
                job.killRequested = false;
                job.detail = `cancel failed: ${result.reason || 'not accepted'}`;
            } else if (result && result.status && result.status !== 'running') {
                // round-14 finding 3: a CONFIRMED terminal outcome must RESOLVE
                // the card (state + clear killRequested), not just change the
                // detail — otherwise it shows "running" forever.
                job.killRequested = false;
                job.state = result.status === 'done' ? 'done' : 'error';
                job.detail = result.status === 'done'
                    ? 'published before cancel' : `cancelled (${result.status})`;
                job.updatedAt = Date.now();
            } else {
                // still pending — the run goes terminal between frames; the
                // durable retry keeps trying and will resolve the card later
                job.detail = 'cancelling — takes effect between frames…';
            }
            _renderJobsRail();
        } catch (e) {
            job.killRequested = false;
            job.detail = `cancel failed: ${e.message}`;
            _renderJobsRail();
        }
        return;
    }
    if (!job.executionArn) return;
    const target = _JOBS_RAIL_KILL_TARGETS[job.kind];
    if (!target) return;
    if (typeof window !== 'undefined' && typeof window.confirm === 'function'
        && !window.confirm(`Stop this ${job.kind} run?
${job.label || job.id}`)) return;
    job.killRequested = true;
    job.detail = 'stop requested…';
    _renderJobsRail();
    try {
        const resp = await lambdaPost('dispatch', {
            target,
            jobs: [{
                action: 'stop',
                job_id: job.jobId,
                task_id: job.taskId,
                execution_arn: job.executionArn,
            }],
            expected_keys: [],
        });
        if ((resp.fired || 0) !== 1) throw new Error('stop dispatch failed');
        // the run's own poll loop sees the stopped status row and finishes
        // the card through its normal error path
    } catch (e) {
        job.killRequested = false;
        job.detail = `stop failed: ${e.message}`;
        _renderJobsRail();
    }
}

function _jobsRailOpen(id) {
    const job = _jobsRailJobs.find(j => j.id === id);
    if (!job) return;
    if (job.jobId && (job.kind === 'render' || job.kind === 'palette')) {
        _setRenderResultsJob(job.jobId);
    }
    if (job.jobId && job.kind === 'palette') {
        const dir = document.getElementById('palette-results-dir');
        if (dir) dir.value = job.jobId;
    }
    switchTab(job.tab || 'compute');
}

function _jobsRailClearHistory() {
    _jobsRailJobs = _jobsRailJobs.filter(j => j.state === 'running');
    _jobsRailPersistHistory();
    _renderJobsRail();
}

function _jobsRailToggle() {
    const collapsed = _jobsRailCollapsed();
    try { localStorage.setItem(JOBS_RAIL_COLLAPSED_KEY, collapsed ? '0' : '1'); } catch (e) {}
    _renderJobsRail();
}

function _jobsRailAge(ms) {
    const s = Math.max(0, Math.round((Date.now() - (ms || 0)) / 1000));
    if (s < 90) return s + 's';
    const m = Math.round(s / 60);
    if (m < 90) return m + 'm';
    return Math.round(m / 60) + 'h';
}

function _renderJobsRail() {
    // Guarded end to end: the rail is fed from dispatch and poll paths,
    // and a rendering bug must never break those.
    try {
        const rail = document.getElementById('jobs-rail');
        const cardsEl = document.getElementById('jobs-rail-cards');
        const titleEl = document.getElementById('jobs-rail-title');
        const toggleEl = document.getElementById('jobs-rail-toggle');
        if (!rail || !cardsEl) return;
        const collapsed = _jobsRailCollapsed();
        const running = _jobsRailJobs.filter(j => j.state === 'running').length;
        if (titleEl) titleEl.textContent = running ? `Jobs · ${running} running` : 'Jobs';
        if (toggleEl) toggleEl.textContent = collapsed ? 'show' : 'hide';
        rail.classList.toggle('jobs-rail-collapsed', collapsed);
        if (collapsed) { cardsEl.innerHTML = ''; return; }
        if (!_jobsRailJobs.length) {
            cardsEl.innerHTML = '<span class="jobs-rail-empty">No jobs yet. Compute, render, and palette runs appear here.</span>';
            return;
        }
        cardsEl.innerHTML = _jobsRailJobs.map(job => {
            const state = job.state === 'running' ? 'running'
                : (job.state === 'failed' || job.state === 'error' ? 'failed' : 'done');
            const age = _jobsRailAge(job.updatedAt || job.startedAt);
            const detail = String(job.detail || '');
            // Click routes through a data attribute: onclick-string
            // interpolation would decode &#39; back to a quote before the
            // JS parses, so ids must never be embedded in code text.
            // Cards are divs (not buttons) so the kill control can nest.
            const sheetKillable = String(job.id).startsWith('sheet:');
            const canKill = state === 'running' && !job.killRequested && (!!job.executionArn || sheetKillable);
            const killBtn = canKill
                ? `<button type="button" class="jobs-rail-kill" data-jobs-rail-id="${_escapeHtml(job.id)}" onclick="event.stopPropagation(); _jobsRailKill(this.dataset.jobsRailId)" title="Stop this run">kill</button>`
                : (job.killRequested && state === 'running' ? '<span class="jobs-rail-kill jobs-rail-kill-pending">stopping…</span>' : '');
            // Dismiss removes the card from the rail only (client state) —
            // the escape hatch for a card whose poll loop died mid-session.
            const dismissBtn = `<button type="button" class="jobs-rail-dismiss" data-jobs-rail-id="${_escapeHtml(job.id)}" onclick="event.stopPropagation(); _jobsRailDismiss(this.dataset.jobsRailId)" title="Remove this card from the rail (does not stop the run)">×</button>`;
            return `<div role="button" tabindex="0" class="jobs-rail-card jobs-rail-${state}" data-jobs-rail-id="${_escapeHtml(job.id)}" onclick="_jobsRailOpen(this.dataset.jobsRailId)" title="${_escapeHtml(detail)}">` +
                `<span class="jobs-rail-card-head"><span class="jobs-rail-kind">${_escapeHtml(job.kind || 'job')}</span>` +
                `<span class="jobs-rail-state">${_escapeHtml(state === 'running' ? 'running · ' + age : state + ' · ' + age + ' ago')}</span>${killBtn}${dismissBtn}</span>` +
                `<span class="jobs-rail-label">${_escapeHtml(job.label || job.id)}</span>` +
                (detail ? `<span class="jobs-rail-detail">${_escapeHtml(detail)}</span>` : '') +
                `</div>`;
        }).join('');
    } catch (e) {}
}

function _initJobsRail() {
    _jobsRailJobs = _jobsRailLoadHistory();
    _renderJobsRail();
}

;(window.__ppParts = window.__ppParts || []).push('10-status-results');
