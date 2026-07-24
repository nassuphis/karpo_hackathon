// PolyPaint 11-artifacts — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Deploy rewrites the script tags to build-versioned asset
// keys (assets/<BUILD_ID>/...), so a deploy flips atomically via index.html.
function _renderPreviewViewportMeta(art) {
    if (!art || art.family !== 'color') return null;
    const width = Number(art.width);
    const height = Number(art.height);
    const minRe = Number(art.min_re);
    const maxRe = Number(art.max_re);
    const minIm = Number(art.min_im);
    const maxIm = Number(art.max_im);
    const rotation = Number(art.rotation);
    return {
        width,
        height,
        minRe,
        maxRe,
        minIm,
        maxIm,
        rotation: Number.isFinite(rotation) ? rotation : 0,
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

function _teardownRenderPreviewDrag() {
    const cleanup = _renderPreviewDragState.cleanup;
    _renderPreviewDragState.cleanup = null;
    _renderPreviewDragState.dragging = false;
    _renderPreviewDragState.artifactKey = '';
    _renderPreviewDragState.start = null;
    _renderPreviewDragState.rect = null;
    if (typeof cleanup === 'function') cleanup();
}

function _clearRenderPreviewSelection() {
    _renderPreviewSelectionState.artifactKey = '';
    _renderPreviewSelectionState.rect = null;
    _renderPreviewDragState.dragging = false;
    _renderPreviewDragState.artifactKey = '';
    _renderPreviewDragState.start = null;
    _renderPreviewDragState.rect = null;
    _syncRenderPreviewMarquee();
}

function _renderPreviewImageRect(stage, img) {
    if (!stage || !img) return null;
    const stageRect = stage.getBoundingClientRect();
    const imgRect = img.getBoundingClientRect();
    if (!(stageRect.width > 0 && stageRect.height > 0 && imgRect.width > 0 && imgRect.height > 0)) return null;
    return {
        x: imgRect.left - stageRect.left,
        y: imgRect.top - stageRect.top,
        width: imgRect.width,
        height: imgRect.height,
    };
}

function _renderPreviewPointerFractions(stage, event) {
    if (!stage || !event) return null;
    const img = document.getElementById('render-preview-image');
    const rect = img ? img.getBoundingClientRect() : stage.getBoundingClientRect();
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

function _normalizeRenderPreviewRect(start, end) {
    if (!start || !end) return null;
    const left = Math.min(start.x, end.x);
    const right = Math.max(start.x, end.x);
    const top = Math.min(start.y, end.y);
    const bottom = Math.max(start.y, end.y);
    return {
        x: left,
        y: top,
        w: Math.max(0, right - left),
        h: Math.max(0, bottom - top),
    };
}

function _syncRenderPreviewMarquee() {
    const marquee = document.getElementById('render-preview-marquee');
    if (!marquee) return;
    const stage = document.getElementById('render-preview-stage');
    const img = document.getElementById('render-preview-image');
    const imageRect = _renderPreviewImageRect(stage, img);
    const artKey = _renderArtifactStableKey(_renderSelectedArtifactEntry());
    const rect = _renderPreviewDragState.dragging && _renderPreviewDragState.artifactKey === artKey
        ? _renderPreviewDragState.rect
        : (_renderPreviewSelectionState.artifactKey === artKey ? _renderPreviewSelectionState.rect : null);
    if (!imageRect || !rect || !(rect.w > 0) || !(rect.h > 0)) {
        marquee.style.display = 'none';
        return;
    }
    marquee.style.display = 'block';
    marquee.style.left = `${imageRect.x + rect.x * imageRect.width}px`;
    marquee.style.top = `${imageRect.y + rect.y * imageRect.height}px`;
    marquee.style.width = `${rect.w * imageRect.width}px`;
    marquee.style.height = `${rect.h * imageRect.height}px`;
}

function _applyRenderPreviewSelectionBounds(meta, rect) {
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
    const statusEl = document.getElementById('render-status');
    const art = _renderSelectedArtifactEntry();
    if (statusEl && !_activeRenderRun) {
        statusEl.textContent = art && art.artifact_id
            ? `Preview subview selected from ${art.artifact_id}`
            : 'Preview subview selected';
        statusEl.className = 'status ok';
    }
}

function _initRenderPreviewMarquee(art) {
    _teardownRenderPreviewDrag();
    const stage = document.getElementById('render-preview-stage');
    const img = document.getElementById('render-preview-image');
    const marquee = document.getElementById('render-preview-marquee');
    if (!stage || !img || !marquee) return;
    const meta = _renderPreviewViewportMeta(art);
    const artKey = _renderArtifactStableKey(art);
    const interactive = !!(meta && meta.hasViewport);
    stage.classList.toggle('inactive', !interactive);
    _syncRenderPreviewMarquee();
    if (!interactive) return;

    const onMouseDown = (event) => {
        if (event.button !== 0) return;
        const start = _renderPreviewPointerFractions(stage, event);
        if (!start) return;
        event.preventDefault();
        _renderPreviewDragState.dragging = true;
        _renderPreviewDragState.artifactKey = artKey;
        _renderPreviewDragState.start = start;
        _renderPreviewDragState.rect = { x: start.x, y: start.y, w: 0, h: 0 };
        _syncRenderPreviewMarquee();
    };

    const onMouseMove = (event) => {
        if (!_renderPreviewDragState.dragging || _renderPreviewDragState.artifactKey !== artKey) return;
        const current = _renderPreviewPointerFractions(stage, event);
        if (!current) return;
        _renderPreviewDragState.rect = _normalizeRenderPreviewRect(_renderPreviewDragState.start, current);
        _syncRenderPreviewMarquee();
    };

    const onMouseUp = (event) => {
        if (!_renderPreviewDragState.dragging || _renderPreviewDragState.artifactKey !== artKey) return;
        const end = _renderPreviewPointerFractions(stage, event) || _renderPreviewDragState.start;
        const rect = _normalizeRenderPreviewRect(_renderPreviewDragState.start, end);
        _renderPreviewDragState.dragging = false;
        _renderPreviewDragState.artifactKey = '';
        _renderPreviewDragState.start = null;
        _renderPreviewDragState.rect = null;
        if (rect && rect.w > 0 && rect.h > 0) {
            _renderPreviewSelectionState.artifactKey = artKey;
            _renderPreviewSelectionState.rect = rect;
            _applyRenderPreviewSelectionBounds(meta, rect);
        }
        _syncRenderPreviewMarquee();
    };

    const onContextMenu = (event) => {
        event.preventDefault();
        if (_renderPreviewDragState.dragging) return;
        _clearRenderPreviewSelection();
    };

    const onDragStart = (event) => {
        event.preventDefault();
    };

    const onBlur = () => {
        if (!_renderPreviewDragState.dragging) return;
        _renderPreviewDragState.dragging = false;
        _renderPreviewDragState.artifactKey = '';
        _renderPreviewDragState.start = null;
        _renderPreviewDragState.rect = null;
        _syncRenderPreviewMarquee();
    };

    stage.addEventListener('mousedown', onMouseDown);
    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', onMouseUp);
    stage.addEventListener('contextmenu', onContextMenu);
    img.addEventListener('dragstart', onDragStart);
    window.addEventListener('blur', onBlur);
    _renderPreviewDragState.cleanup = () => {
        stage.removeEventListener('mousedown', onMouseDown);
        document.removeEventListener('mousemove', onMouseMove);
        document.removeEventListener('mouseup', onMouseUp);
        stage.removeEventListener('contextmenu', onContextMenu);
        img.removeEventListener('dragstart', onDragStart);
        window.removeEventListener('blur', onBlur);
    };
}

function _linkedPaletteIdForColorArtifact(art) {
    if (!art) return '';
    return String(art.associated_palette_id || art.palette_source_id || '').trim();
}

function _linkedColorIdForPaletteArtifact(art) {
    if (!art) return '';
    return String(art.derived_from_color_artifact_id || '').trim();
}

function _sourceColorArtifactIdForRenderArtifact(art) {
    if (!art) return '';
    if (art.family === 'palette') {
        return _linkedColorIdForPaletteArtifact(art)
            || String(art.source_color_artifact_id || art.artifact_id || '').trim();
    }
    if (art.family === 'color') {
        return String(art.derived_from_artifact_id || art.artifact_id || '').trim();
    }
    return String(art.derived_from_artifact_id || art.source_artifact_id || art.artifact_id || '').trim();
}

function _artifactSolveScoreChain(art) {
    if (!art) return [];
    const candidates = [
        art.solve_score_chain,
        art.palette_source_score_chain,
        art.associated_palette_score_chain,
    ];
    for (const chain of candidates) {
        if (Array.isArray(chain) && chain.length) return chain;
    }
    return [];
}

function _artifactSolveScoreSourceText(art) {
    if (!art) return '';
    const candidates = [
        art.solve_score_program_source_text,
        art.score_source_text,
        art.palette_source_score_source_text,
        art.palette_source_solve_score_program_source_text,
        art.associated_palette_score_source_text,
        art.associated_palette_solve_score_program_source_text,
    ];
    for (const raw of candidates) {
        const text = String(raw || '');
        if (text.trim()) return text;
    }
    return '';
}

function _restoreSolveScoreSourceFromArtifact(prefix, art) {
    const text = _artifactSolveScoreSourceText(art);
    if (!text.trim()) {
        // Text is the only editable solve-score surface now. Old chain-only
        // artifacts should have been reconstructed by the backend inventory
        // path; fail visibly rather than populating compiler-internal chips.
        _setSolveScoreProgramSourceText(prefix, '');
        _setSolveScoreProgramEditorMode(prefix, 'text');
        _setSolveScoreProgramStatus(
            prefix,
            'Selected artifact has no solve-score source text; refresh inventory or regenerate it before editing.',
            true
        );
        return false;
    }
    _setSolveScoreProgramSourceText(prefix, text);
    _setSolveScoreProgramEditorMode(prefix, 'text');
    return true;
}

function _restoreRootSourceFromArtifact(prefix, art) {
    // The text editor is the only root-transform authoring surface. Stored
    // source text wins; chain-only artifacts (pre-text era) are synthesized
    // into equivalent source; artifacts with no root transforms clear the
    // editor so stale text never leaks across populates.
    const text = String((art && art.root_program_source_text) || '');
    if (text.trim()) {
        _setRootProgramSourceText(prefix, text, { auto: false });
        _rootProgramStatus(prefix, 'Root source restored from artifact.');
        return true;
    }
    const transforms = art && Array.isArray(art.root_transforms) ? art.root_transforms : [];
    const synthesized = _rootSourceFromRows(transforms);
    _setRootProgramSourceText(prefix, synthesized, { auto: true });
    _rootProgramStatus(prefix, synthesized ? 'Root source synthesized from artifact transforms.' : '');
    return !!synthesized;
}

function _renderArtifactSolveDisplay(art) {
    if (!art) return '';
    const chain = _artifactSolveScoreChain(art);
    if (!chain.length) return '';
    try {
        return _compileSolveScoreChain(chain, 'proximity', 0.1).display;
    } catch (_) {
        return '';
    }
}

function _noteSolveScorePopulate(prefix, art) {
    const sourceColorId = _sourceColorArtifactIdForRenderArtifact(art) || 'saved artifact';
    const statusTarget = art && art.family === 'palette' && !_linkedColorIdForPaletteArtifact(art)
        ? `palette ${String(art.artifact_id || art.palette_id || 'saved artifact').trim()}`
        : sourceColorId;
    _solveScoreProgramRememberedNames[prefix] = '';
    _setSolveScoreProgramStatus(prefix, `Populated from ${statusTarget}`, false);
}

function _renderArtifactSummary(art) {
    if (!art) return '';
    let base = '';
    if (art.family === 'color') {
        if (art.color_mode === 'solve_score') {
            const solveDisplay = _renderArtifactSolveDisplay(art);
            const sourceColorId = _sourceColorArtifactIdForRenderArtifact(art);
            const colorInterpretation = _artifactColorInterpretation(art);
            base = [
                solveDisplay ? `solve:${solveDisplay}` : '',
                colorInterpretation ? `mode:${_colorInterpretationLabel(colorInterpretation)}` : '',
                art.palette ? `palette:${art.palette}` : '',
                sourceColorId ? `color:${sourceColorId}` : '',
            ].filter(Boolean).join(' · ');
        } else if (art.color_mode) {
            base = art.color_mode;
        }
        if (art.postprocess_kind === 'autolevels') {
            base = [base, 'autolevels'].filter(Boolean).join(' · ');
        }
        if (art.postprocess_kind === 'resize') {
            base = [base, 'resize'].filter(Boolean).join(' · ');
        }
        if (art.derivation_kind === 'color_repalette') {
            base = [base, 'repalette'].filter(Boolean).join(' · ');
        }
        if (art.artifact_id && art.color_mode !== 'solve_score') {
            return [base, `id=${art.artifact_id}`].filter(Boolean).join(' · ');
        }
        return base || art.artifact_id || '';
    }
    if (art.family === 'bilevel') {
        if (art.postprocess_kind === 'png_export') return 'PNG';
        if (art.postprocess_kind === 'tiff_compat') return 'Compat TIFF';
        if (art.postprocess_kind === 'color_to_bilevel') {
            const threshold = art.threshold != null && art.threshold !== ''
                ? `threshold>${art.threshold}`
                : 'threshold';
            return [threshold, art.derived_from_artifact_id ? `from ${art.derived_from_artifact_id}` : ''].filter(Boolean).join(' · ');
        }
        if (art.legacy) return 'legacy';
        if (art.bilevel_pipeline) return 'sparse fragments';
        return art.format || '';
    }
    if (art.family === 'palette') {
        const solveDisplay = _renderArtifactSolveDisplay(art);
        const derived = art.derivation_kind === 'repalette' ? 'repalette' : '';
        const sourceColorId = _sourceColorArtifactIdForRenderArtifact(art);
        const rawColorInterpretation = art.color_interpretation || art.score_output_interpretation || '';
        const colorInterpretation = rawColorInterpretation ? _normalizeColorInterpretation(rawColorInterpretation) : '';
        const reuseLabel = art.raw_key || Number(art.raw_channels || art.score_output_channel_count || 1) > 1
            ? 'raw'
            : (art.render_reusable === false ? 'pass0' : 'reusable');
        return [
            solveDisplay ? `solve:${solveDisplay}` : '',
            colorInterpretation ? `mode:${_colorInterpretationLabel(colorInterpretation)}` : '',
            reuseLabel,
            art.palette ? `palette:${art.palette}` : '',
            sourceColorId ? `color:${sourceColorId}` : '',
            derived,
        ].filter(Boolean).join(' · ');
    }
    if (art.family === 'pdf') {
        const kind = art.pdf_kind === 'color_spread' ? 'ColorSpread' : (art.pdf_kind || 'pdf');
        return [kind, art.source_display_name || art.source_artifact_id].filter(Boolean).join(' · ');
    }
    if (art.legacy) return 'legacy';
    return art.format || '';
}

function _renderArtifactFilename(jobId, art) {
    if (!art) return `${jobId}_artifact`;
    const ext = (art.image_key || '').split('.').pop() || (art.format || 'bin');
    return `${jobId}_${art.artifact_id || art.family}.${ext}`;
}

function _renderActionButtonRows(buttons, maxPerRow = 5) {
    const rows = [];
    for (let i = 0; i < buttons.length; i += maxPerRow) {
        rows.push(buttons.slice(i, i + maxPerRow));
    }
    return rows.map(row => `<div class="render-action-row" style="display:flex; flex-wrap:wrap; align-items:center; gap:8px">${row.join('')}</div>`).join('');
}

function _setRenderSliderValue(inputId, labelId, value, decimals) {
    const input = document.getElementById(inputId);
    const label = document.getElementById(labelId);
    if (!input) return;
    const n = Number(value);
    if (!Number.isFinite(n)) return;
    input.value = String(n);
    if (label) label.textContent = n.toFixed(decimals);
}

function _setRenderRotationFromRadians(rad) {
    const slider = document.getElementById('render-rotation');
    const dirSel = document.getElementById('render-rotation-dir');
    const label = document.getElementById('render-rotation-val');
    const n = Number(rad);
    if (!slider || !dirSel || !label || !Number.isFinite(n)) return;
    const turns = Math.min(1, Math.max(0, Math.abs(n) / (2 * Math.PI)));
    slider.value = String(turns);
    dirSel.value = n < 0 ? 'cw' : 'ccw';
    label.textContent = turns.toFixed(2);
}

function _setSolveScoreChainFromArtifact(prefix, chain) {
    const normalized = _normalizeSolveScoreChain(chain, 'proximity', 0.1);
    if (!normalized.length) return false;
    const target = _chainForWhich(_solveScoreWhichForPrefix(prefix));
    target.splice(0, target.length, ...normalized);
    _syncSolveScoreUi(_solveScoreWhichForPrefix(prefix));
    return true;
}

function _restoreRenderExecutionState(entry) {
    const cfg = entry && entry.render_execution && typeof entry.render_execution === 'object'
        ? entry.render_execution
        : null;
    if (!cfg) return;
    if (cfg.save_associated_palette != null) {
        const saveAssociatedPalette = !!cfg.save_associated_palette;
        _renderMtPopupState.saveAssociatedPalette = saveAssociatedPalette;
    }
    if (cfg.raster_mt_threads != null) _renderMtPopupState.rasterThreads = _clampRenderMtThreads(cfg.raster_mt_threads);
    if (cfg.raster_workers != null) _renderMtPopupState.rasterWorkers = _clampRenderMtWorkerCount(cfg.raster_workers);
    if (cfg.solve_score_threads != null) _renderMtPopupState.solveScoreThreads = _clampRenderMtThreads(cfg.solve_score_threads);
    if (cfg.raster_sectioned_retries != null) _renderMtPopupState.rasterRetries = _clampRenderMtRetries(cfg.raster_sectioned_retries);
    if (cfg.raster_section_mode != null) _renderMtPopupState.rasterSectionMode = _normalizeRenderMtSectionMode(cfg.raster_section_mode);
    if (cfg.raster_section_count != null) _renderMtPopupState.rasterSectionCount = _clampRenderMtSectionCount(cfg.raster_section_count);
    if (cfg.finalize_workers != null) _renderMtPopupState.finalizeWorkers = _clampRenderMtFinalizeWorkers(cfg.finalize_workers);
}

function populateSelectedRenderArtifact() {
    const art = _renderSelectedArtifactEntry();
    if (!art) return;

    const warnings = [];
    const pixEl = document.getElementById('render-pix');
    const formatEl = document.getElementById('render-format');
    const qualityEl = document.getElementById('render-quality');

    const restoreRenderOutputFields = (entry) => {
        const pixValue = Number(entry && entry.pix);
        if (pixEl) {
            if (Number.isFinite(pixValue) && pixValue > 0) {
                pixEl.value = String(Math.round(pixValue));
            } else {
                const widthValue = Number(entry && entry.width);
                if (Number.isFinite(widthValue) && widthValue > 0) pixEl.value = String(Math.round(widthValue));
                else warnings.push('pix');
            }
        }
        if (formatEl) {
            if (entry && entry.format) formatEl.value = String(entry.format);
            else warnings.push('format');
        }
        if (qualityEl) {
            const qualityValue = Number(entry && entry.quality);
            if (Number.isFinite(qualityValue) && qualityValue > 0) {
                qualityEl.value = String(Math.round(qualityValue));
            }
        }
    };

    const restoreCommonRenderFields = (entry) => {
        if (entry.view_mode === 'square') {
            if (Number.isFinite(entry.square_extent)) {
                const extentEl = document.getElementById('render-square-extent');
                if (extentEl) extentEl.value = String(entry.square_extent);
            } else {
                warnings.push('square extent');
            }
        } else {
            if (Number.isFinite(entry.quantile)) _setRenderSliderValue('render-quantile', 'render-quantile-val', entry.quantile * 100, 1);
            else warnings.push('auto quantile');
            if (Number.isFinite(entry.shim)) _setRenderSliderValue('render-shim', 'render-shim-val', entry.shim * 100, 1);
            else warnings.push('auto shim');
        }

        if (
            Number.isFinite(entry.min_re) &&
            Number.isFinite(entry.max_re) &&
            Number.isFinite(entry.min_im) &&
            Number.isFinite(entry.max_im)
        ) {
            _setRenderExplicitViewportBounds({
                minRe: entry.min_re,
                maxRe: entry.max_re,
                minIm: entry.min_im,
                maxIm: entry.max_im,
            });
            selectViewMode('explicit');
        } else if (entry.view_mode === 'square') {
            selectViewMode('square');
        } else {
            selectViewMode('auto');
        }

        if (Number.isFinite(entry.rotation)) _setRenderRotationFromRadians(entry.rotation);
        else warnings.push('rotation');

        restoreRenderOutputFields(entry);
        _restoreRenderExecutionState(entry);
        if (entry.background_color) _setRenderBackgroundColor(entry.background_color);

        _restoreRootSourceFromArtifact('render', entry);
    };

    const restoreBilevelSectionState = (entry) => {
        const cfg = entry && entry.render_execution && typeof entry.render_execution === 'object'
            ? entry.render_execution
            : null;
        const sectionMode = _normalizeRenderMtSectionMode(
            entry.bilevel_section_mode || (cfg && cfg.raster_section_mode) || 'logical_sections_auto'
        );
        _bilevelPopupState.sectionMode = sectionMode;
        _bilevelPopupState.sectionCount = sectionMode === 'logical_sections'
            ? _clampRenderMtSectionCount(
                entry.bilevel_section_count != null
                    ? entry.bilevel_section_count
                    : (cfg && cfg.raster_section_count)
            )
            : '';
    };

    const restoreSolveScoreNormalize = (entry) => {
        const el = document.getElementById('render-score-normalization');
        if (!el || !entry) return;
        if (entry.solve_score_normalize != null) el.checked = _boolish(entry.solve_score_normalize, false);
        else if (entry.score_output_normalize != null) el.checked = _boolish(entry.score_output_normalize, false);
    };

    const finishPopulate = (label) => {
        const msg = warnings.length
            ? `Populate complete (missing saved ${warnings.join(', ')})`
            : `Populate complete: ${label}`;
        const statusEl = document.getElementById('render-status');
        if (statusEl) {
            statusEl.textContent = msg;
            statusEl.className = warnings.length ? 'status' : 'status ok';
        }
        log(msg, warnings.length ? '' : 'ok', 'render-log');
    };

    if (_renderActiveFamily === 'palette') {
        setColorMode('solve_score');
        _restoreRenderExecutionState(art);
        if (!_setSolveScoreChainFromArtifact('render', _artifactSolveScoreChain(art))) {
            warnings.push('solve-score chain');
        }
        _restoreSolveScoreSourceFromArtifact('render', art);
        restoreSolveScoreNormalize(art);
        if (art.palette) setPaletteForMode('solve_score', art.palette, art.palette_display_name);
        else warnings.push('palette');
        if (art.background_color) _setRenderBackgroundColor(art.background_color);
        _restoreRootSourceFromArtifact('render', art);
        restoreRenderOutputFields(art);
        _renderSelectFamily('color');
        _noteSolveScorePopulate('render', art);
        finishPopulate(art.artifact_id || art.palette_id || 'selected palette artifact');
        return;
    }

    if (_renderActiveFamily !== 'color' && _renderActiveFamily !== 'bilevel' && _renderActiveFamily !== 'coeffs') return;

    restoreCommonRenderFields(art);

    if (_renderActiveFamily === 'bilevel') {
        restoreBilevelSectionState(art);
        finishPopulate(art.artifact_id || 'selected bilevel artifact');
        return;
    }

    if (_renderActiveFamily === 'coeffs') {
        finishPopulate(art.artifact_id || 'selected coeff artifact');
        return;
    }

    setMatch('none');
    if (art.match_mode && art.match_mode !== 'none') warnings.push(`unsupported match ${art.match_mode}`);

    setColorMode('solve_score');
    _setRenderColorInterpretation(_artifactColorInterpretation(art));
    if (!_setSolveScoreChainFromArtifact('render', _artifactSolveScoreChain(art))) {
        warnings.push('solve-score chain');
    }
    _restoreSolveScoreSourceFromArtifact('render', art);
    restoreSolveScoreNormalize(art);
    if (art.palette) setPaletteForMode('solve_score', art.palette, art.palette_display_name);
    _noteSolveScorePopulate('render', art);
    finishPopulate(art.artifact_id || 'selected color artifact');
}

function _updateRenderActionButtons() {
    const art = _renderSelectedArtifactEntry();
    const hasSelection = !!art;
    const jobId = document.getElementById('render-results-dir')?.value.trim() || '';
    const inventoryMatchesJob = !!jobId && jobId === _renderLoadedJobId;
    const genMtBtn = document.getElementById('btn-render-generate-mt');
    const dlBtn = document.getElementById('btn-render-download');
    const delBtn = document.getElementById('btn-render-delete');
    const dzBtn = document.getElementById('btn-render-deepzoom');
    const popBtn = document.getElementById('btn-render-populate');
    const autoBtn = document.getElementById('btn-render-autolevels');
    const resizeBtn = document.getElementById('btn-render-resize');
    const favBtn = document.getElementById('btn-render-favorite');
    const colorToBilevelBtn = document.getElementById('btn-render-color-to-bilevel');
    const goResultBtn = document.getElementById('btn-render-go-result');
    const goPaletteBtn = document.getElementById('btn-render-go-palette');
    const goColorBtn = document.getElementById('btn-render-go-color');
    const repalBtn = document.getElementById('btn-render-repalette');
    const colorRepalBtn = document.getElementById('btn-render-color-repalette');
    const extractPalBtn = document.getElementById('btn-render-extract-palette');
    const pdfBtn = document.getElementById('btn-render-pdf-colorspread');
    const tiffCompatBtn = document.getElementById('btn-tiff-compat');
    const pngExportBtn = document.getElementById('btn-png-export');
    const linkedPaletteId = _linkedPaletteIdForColorArtifact(art);
    const linkedColorId = _linkedColorIdForPaletteArtifact(art);
    const isTiffArtifact = !!(art && art.image_key && /\.tiff?$/i.test(String(art.image_key)));
    const canConvertBilevel = _renderActiveFamily === 'bilevel' && hasSelection && inventoryMatchesJob && isTiffArtifact;
    if (dlBtn) dlBtn.disabled = !(hasSelection && inventoryMatchesJob);
    if (genMtBtn) genMtBtn.disabled = !(_renderActiveFamily === 'color' && _renderColorMtEligible()) || !!_activeRenderRun;
    if (delBtn) delBtn.disabled = !(hasSelection && inventoryMatchesJob);
    if (dzBtn) dzBtn.disabled = !(hasSelection && inventoryMatchesJob) || _renderActiveFamily === 'pdf';
    if (popBtn) popBtn.disabled = !(hasSelection && inventoryMatchesJob && (_renderActiveFamily === 'color' || _renderActiveFamily === 'palette' || _renderActiveFamily === 'bilevel' || _renderActiveFamily === 'coeffs'));
    if (autoBtn) autoBtn.disabled = !(hasSelection && inventoryMatchesJob && _renderActiveFamily === 'color' && art && art.image_key) || !!_activeRenderRun;
    if (resizeBtn) resizeBtn.disabled = !(hasSelection && inventoryMatchesJob && _renderActiveFamily === 'color' && art && art.image_key) || !!_activeRenderRun;
    if (colorToBilevelBtn) colorToBilevelBtn.disabled = !(_renderActiveFamily === 'color' && hasSelection && inventoryMatchesJob && _hasColorRawSidecar(art)) || !!_activeRenderRun;
    if (colorToBilevelBtn) {
        colorToBilevelBtn.title = (_renderActiveFamily === 'color' && hasSelection && inventoryMatchesJob && art && !_hasColorRawSidecar(art))
            ? 'Color2Bilevel requires a greyscale raw sidecar; rerender with the Fused path.'
            : '';
    }
    if (repalBtn) repalBtn.disabled = !(_renderActiveFamily === 'palette' && hasSelection && inventoryMatchesJob && _canRepaletteArtifact(art)) || !!_activeRenderRun;
    if (colorRepalBtn) colorRepalBtn.disabled = !(_renderActiveFamily === 'color' && hasSelection && inventoryMatchesJob && _canColorRepaletteArtifact(art)) || !!_activeRenderRun;
    if (colorRepalBtn) {
        colorRepalBtn.title = (_renderActiveFamily === 'color' && hasSelection && inventoryMatchesJob && art && !_canColorRepaletteArtifact(art))
            ? 'RePalette requires a fused raw sidecar; rerender with the Fused path.'
            : '';
    }
    if (extractPalBtn) {
        const canOpenExtractPalette = _renderActiveFamily === 'color' && hasSelection && inventoryMatchesJob;
        const paletteRunBlocking = typeof _paletteRunBlocksNewRun === 'function' && _paletteRunBlocksNewRun();
        extractPalBtn.disabled = !canOpenExtractPalette || !!_activeRenderRun || paletteRunBlocking;
        extractPalBtn.title = paletteRunBlocking
            ? 'Palette/ExtractPalette run already in progress'
            : canOpenExtractPalette && !_canExtractPaletteArtifact(art)
            ? _extractPaletteLineageHint(art).label
            : '';
    }
    if (pdfBtn) pdfBtn.disabled = !(_renderActiveFamily === 'color' && hasSelection && inventoryMatchesJob && art && art.artifact_id && art.image_key && (art.content_type || '').indexOf('image/') === 0) || !!_activeRenderRun;
    if (tiffCompatBtn) tiffCompatBtn.disabled = !canConvertBilevel || !!_activeRenderRun;
    if (pngExportBtn) pngExportBtn.disabled = !canConvertBilevel || !!_activeRenderRun;
    if (goResultBtn) goResultBtn.disabled = !(_renderActiveFamily === 'color' && jobId) || !!_activeRenderRun;
    if (goPaletteBtn) goPaletteBtn.disabled = !(_renderActiveFamily === 'color' && hasSelection && inventoryMatchesJob && linkedPaletteId) || !!_activeRenderRun;
    if (goColorBtn) goColorBtn.disabled = !(_renderActiveFamily === 'palette' && hasSelection && inventoryMatchesJob && linkedColorId) || !!_activeRenderRun;
    if (favBtn) {
        const isFav = _renderActiveFamily === 'color' && hasSelection && inventoryMatchesJob && _isFavorite(jobId, art.artifact_id);
        favBtn.textContent = isFav ? 'Favorited' : 'Favorite';
        favBtn.disabled = !(_renderActiveFamily === 'color' && hasSelection && inventoryMatchesJob && !isFav) || !!_activeRenderRun;
    }
}

function _captureRenderCatalogScroll(family) {
    const fam = family || _renderActiveFamily;
    const catalog = document.getElementById('render-artifact-catalog');
    if (!catalog || fam == null) return;
    const top = Number(catalog.scrollTop);
    if (Number.isFinite(top)) _renderCatalogScrollTop[fam] = top;
}

function _renderRowElementId(family, idx) {
    return `render-art-row-${family}-${idx}`;
}

function _restoreRenderCatalogScroll(options = {}) {
    const family = options.family || _renderActiveFamily;
    const catalog = document.getElementById('render-artifact-catalog');
    if (!catalog) return;
    const top = Number(_renderCatalogScrollTop[family]);
    if (Number.isFinite(top)) catalog.scrollTop = top;
    catalog.onscroll = () => {
        const nextTop = Number(catalog.scrollTop);
        if (Number.isFinite(nextTop)) _renderCatalogScrollTop[family] = nextTop;
    };
    if (!options.ensureSelected) return;
    const idx = _renderSelectedArtifact[family];
    if (idx == null || idx < 0) return;
    const row = document.getElementById(_renderRowElementId(family, idx));
    if (row && typeof row.scrollIntoView === 'function') {
        row.scrollIntoView({ block: 'nearest' });
    }
}

function _renderSelectFamily(family) {
    _captureRenderCatalogScroll(_renderActiveFamily);
    if (family !== _renderActiveFamily) _clearRenderPreviewSelection();
    _renderActiveFamily = family;
    const activeJobId = document.getElementById('render-results-dir').value.trim();
    const inv = _renderArtifacts[family] || [];
    const idxByKey = _renderSelectedArtifactKey[family]
        ? inv.findIndex((art) => _renderArtifactStableKey(art) === _renderSelectedArtifactKey[family])
        : -1;
    if (idxByKey >= 0) _renderSelectedArtifact[family] = idxByKey;
    if (inv.length && (_renderSelectedArtifact[family] == null || _renderSelectedArtifact[family] < 0 || _renderSelectedArtifact[family] >= inv.length)) {
        _renderSelectedArtifact[family] = 0;
    }
    _renderSelectedArtifactKey[family] = _renderSelectedArtifact[family] >= 0 ? _renderArtifactStableKey(inv[_renderSelectedArtifact[family]]) : '';
    renderArtifactPanel(activeJobId, window._lastRenderSummary || { families: _renderArtifacts, calc: {} }, { preserveScroll: true, ensureSelected: true });
}

function _renderSelectArtifact(family, idx) {
    const inv = _renderArtifacts[family] || [];
    if (idx < 0 || idx >= inv.length) return;
    _renderSelectArtifactByKey(family, _renderArtifactStableKey(inv[idx]));
}

function _renderSelectArtifactByKey(family, key) {
    const inv = _renderArtifacts[family] || [];
    const nextKey = String(key || '');
    const idx = inv.findIndex((art) => _renderArtifactStableKey(art) === nextKey);
    if (idx < 0) return;
    if (family !== _renderActiveFamily || nextKey !== _renderSelectedArtifactKey[family]) _clearRenderPreviewSelection();
    _captureRenderCatalogScroll(family);
    _renderSelectedArtifact[family] = idx;
    _renderSelectedArtifactKey[family] = nextKey;
    if (_renderActiveFamily !== family) _renderActiveFamily = family;
    renderArtifactPanel(document.getElementById('render-results-dir').value.trim(), window._lastRenderSummary || { families: _renderArtifacts, calc: {} }, { preserveScroll: true, ensureSelected: true });
}

let _renderSaveDirHandle = null;

function _toggleDownloadMenu() {
    const menu = document.getElementById('download-menu');
    if (!menu) return;
    const show = menu.style.display === 'none';
    menu.style.display = show ? 'block' : 'none';
    if (show) {
        const fileItem = document.getElementById('dl-menu-file');
        // Update Dir label to show current directory
        const dirItem = document.getElementById('dl-menu-dir');
        if (fileItem) {
            fileItem.textContent = _renderActiveFamily === 'pdf' ? 'Save PDF' : 'Image';
        }
        if (dirItem) {
            const baseLabel = _renderActiveFamily === 'pdf' ? 'Select Dir\u2026' : 'Save to Dir\u2026';
            dirItem.textContent = _renderSaveDirHandle
                ? '\u2713 ' + (_renderSaveDirHandle.name || 'folder') + ' \u2014 change\u2026'
                : baseLabel;
        }
    }
}
// Close menu on outside click
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

async function _buildArtifactMeta(jobId, art, options = {}) {
    const imageFilename = String(options.imageFilename || '').trim() || String(art?.image_key || '').split('/').pop() || '';
    const paletteFilename = String(options.paletteFilename || '').trim() || String(art?.associated_palette_image_key || '').split('/').pop() || '';
    const metaObj = {};
    for (const [k, v] of Object.entries(art)) {
        if (v != null && v !== '' && typeof v !== 'function') metaObj[k] = v;
    }
    metaObj.job_id = jobId;
    if (imageFilename) metaObj.image_file = imageFilename;
    if (paletteFilename) metaObj.palette_file = paletteFilename;
    // Fetch full calc.json via /detail for complete compute metadata
    try {
        const detail = await lambdaPost('storage', { job_id: jobId }, '/detail');
        const calc = detail.calc || {};
        const pipeline = calc.pipeline || {};
        const pt = pipeline.param_transforms_display || pipeline.param_transforms || [];
        const ptStr = pt.map(a => Array.isArray(a) && a.length > 1 ? `${a[0]}(${a.slice(1).join(',')})` : (Array.isArray(a) ? a[0] : a)).join(', ');
        const ct = (pipeline.coeff_transforms || []).map(item => Array.isArray(item) && item.length > 1 ? `${item[0]}(${item.slice(1).join(',')})` : (Array.isArray(item) ? item[0] : item)).join(', ');
        metaObj.compute = {
            function: pipeline.function || calc.function || '',
            cfpv: pipeline.cfpv_display || (pipeline.cfpv && pipeline.cfpv.length ? (_formatCfpvForDisplay(pipeline.function || calc.function || '', pipeline.cfpv) || pipeline.cfpv.join(', ')) : ''),
            param_transforms: ptStr || 'none',
            coeff_transforms: ct || 'none',
            degree: calc.degree,
            N: calc.N || calc.n1,
            times: calc.times || 1,
            n_chunks: calc.n_chunks || calc.n_stripes,
            solver: calc.solver || '',
            total_roots: (calc.N || calc.n1 || 0) * (calc.N || calc.n1 || 0) * (calc.times || 1) * (calc.degree || 0),
        };
    } catch (e) {
        metaObj.compute = { error: 'Failed to load calc.json: ' + e.message };
    }
    return metaObj;
}

async function _dlMenuAction(mode) {
    document.getElementById('download-menu').style.display = 'none';
    const jobId = document.getElementById('render-results-dir').value.trim();
    const art = _renderSelectedArtifactEntry();
    if (!jobId || !art) return;

    const imageFilename = _renderArtifactFilename(jobId, art);
    const metaFilename = imageFilename.replace(/\.[^.]+$/, '_meta.json');
    const normalizedMode = mode === 'pdf_file' ? 'image' : mode;
    const associatedPaletteKey = (normalizedMode === 'image+meta') ? String(art.associated_palette_image_key || '') : '';
    const paletteExt = associatedPaletteKey ? associatedPaletteKey.split('.').pop().toLowerCase() : '';
    const paletteFilename = associatedPaletteKey
        ? imageFilename.replace(/\.[^.]+$/, '') + `_palette.${paletteExt || 'jpeg'}`
        : '';

    if (mode === 'dir') {
        if (typeof window.showDirectoryPicker !== 'function') {
            alert('Directory picker requires Chrome or Edge over HTTPS (or localhost).');
            return;
        }
        try {
            _renderSaveDirHandle = await window.showDirectoryPicker({ mode: 'readwrite' });
            log('Save directory: ' + _renderSaveDirHandle.name, 'ok', 'render-log');
        } catch (e) {
            if (e.name !== 'AbortError') alert('Directory picker failed: ' + e.message);
        }
        return;
    }

    const metaObj = (normalizedMode === 'image+meta')
        ? await _buildArtifactMeta(jobId, art, { imageFilename, paletteFilename })
        : null;
    const metaBlob = metaObj ? new Blob([JSON.stringify(metaObj, null, 2)], { type: 'application/json' }) : null;

    // Save to directory if one is set, otherwise browser download
    if (_renderSaveDirHandle) {
        const btn = document.getElementById('btn-render-download');
        const orig = btn ? btn.textContent : 'Download \u25bc';
        if (btn) { btn.disabled = true; }
        try {
            const dirHandle = _renderSaveDirHandle;
            if ((await dirHandle.queryPermission({ mode: 'readwrite' })) !== 'granted') {
                if ((await dirHandle.requestPermission({ mode: 'readwrite' })) !== 'granted') {
                    _renderSaveDirHandle = null;
                    throw new Error('Permission denied — directory cleared');
                }
            }

            if (normalizedMode === 'image+meta') {
                if (btn) btn.textContent = 'Saving metadata...';
                const metaHandle = await dirHandle.getFileHandle(metaFilename, { create: true });
                const metaWritable = await metaHandle.createWritable();
                await metaWritable.write(metaBlob);
                await metaWritable.close();
            }

            if (associatedPaletteKey) {
                if (btn) btn.textContent = 'Downloading palette...';
                const paletteBlob = await _fetchStorageBlob(associatedPaletteKey, paletteFilename);
                const palHandle = await dirHandle.getFileHandle(paletteFilename, { create: true });
                const palWritable = await palHandle.createWritable();
                await palWritable.write(paletteBlob);
                await palWritable.close();
            }

            if (btn) btn.textContent = 'Downloading image...';
            const imageBlob = await _fetchStorageBlob(art.image_key, imageFilename);
            const imgHandle = await dirHandle.getFileHandle(imageFilename, { create: true });
            const imgWritable = await imgHandle.createWritable();
            await imgWritable.write(imageBlob);
            await imgWritable.close();

            if (btn) { btn.textContent = 'Saved to ' + dirHandle.name; setTimeout(() => { btn.textContent = orig; }, 2000); }
        } catch (e) {
            if (e.name === 'AbortError') { /* cancelled */ }
            else alert('Save failed: ' + e.message);
        } finally {
            if (btn) btn.disabled = false;
        }
        return;
    }

    // No directory set — browser downloads
    if (normalizedMode === 'image+meta') {
        _downloadBlob(metaBlob, metaFilename);
        if (associatedPaletteKey) {
            await _downloadStorageKeyBlob(associatedPaletteKey, paletteFilename);
        }
        await _downloadStorageKeyBlob(art.image_key, imageFilename);
        return;
    }
    await downloadPresignedFile(art.image_url || art.viewer_url || '', imageFilename, art.image_key);
}

async function deleteSelectedRenderArtifact() {
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    if (!jobId || !art) return;
    if (!confirm(`Delete ${_renderFamilyLabel(_renderActiveFamily)} artifact ${art.artifact_id || ''}?`)) return;
    const btn = document.getElementById('btn-render-delete');
    const orig = btn ? btn.textContent : 'Delete';
    if (btn) { btn.disabled = true; btn.textContent = 'Deleting...'; }
    try {
        await lambdaPost('storage', {
            job_id: jobId,
            family: _renderActiveFamily,
            artifact_id: art.artifact_id,
        }, '/delete-render-artifact');
        log(`${_renderFamilyLabel(_renderActiveFamily)} deleted: ${art.artifact_id}`, 'ok', 'render-log');
        _renderSelectedArtifact[_renderActiveFamily] = -1;
        _renderSelectedArtifactKey[_renderActiveFamily] = '';
        await refreshRenderArtifacts(jobId);
    } catch (e) {
        log(`${_renderFamilyLabel(_renderActiveFamily)} delete failed: ${e.message}`, 'err', 'render-log');
    } finally {
        if (btn) btn.textContent = orig;
    }
}

async function deepZoomSelectedRenderArtifact() {
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    if (!jobId || !art) return;
    const btn = document.getElementById('btn-render-deepzoom');
    await runDeepZoomExport(jobId, art.image_key, btn, {
        rawKey: art.raw_key || '',
        rawMetaKey: art.raw_meta_key || '',
    });
}

async function generateSelectedRenderArtifact() {
    if (_renderActiveFamily === 'pdf') {
        openPdfColorSpreadPopup();
        return;
    }
    if (_renderActiveFamily === 'palette') {
        await runRenderPaletteArtifact();
        return;
    }
    if (_renderActiveFamily === 'bilevel') {
        openBilevelPopup();
        return;
    }
    if (_renderActiveFamily === 'coeffs') {
        await runCoeffBilevelPipeline();
        return;
    }
    if (_renderActiveFamily === 'color' && renderColorMode === 'solve_score') {
        await runRasterPipeline();
        return;
    }
    await runRasterPipeline();
}

async function generateSelectedRenderArtifactMT() {
    if (_renderActiveFamily !== 'color') return;
    openRenderMtPopup();
}

function renderArtifactPanel(jobId, summary, options = {}) {
    const preview = document.getElementById('render-preview');
    const info = document.getElementById('render-info');
    const families = summary.families || {};
    const calc = summary.calc || {};
    const panelViewportPx = 360;
    window._lastRenderSummary = summary;
    _teardownRenderPreviewDrag();

    const withFamily = (family, inv) => (Array.isArray(inv) ? inv : []).map((art) => ({
        ...(art || {}),
        family,
    }));
    _renderArtifacts = {
        color: withFamily('color', families.color),
        bilevel: withFamily('bilevel', families.bilevel),
        coeffs: withFamily('coeffs', families.coeffs),
        palette: withFamily('palette', families.palette),
        pdf: withFamily('pdf', families.pdf),
        // global saved-sculpture list (session cache) — registered here so
        // the family survives the rebuild and its tab count stays live
        sculpture: window._sculptureInventory || [],
    };
    if (!_renderArtifacts[_renderActiveFamily]) _renderActiveFamily = 'color';
    for (const family of ['color', 'bilevel', 'coeffs', 'palette', 'pdf']) {
        const inv = _renderArtifacts[family] || [];
        const keyIdx = _renderSelectedArtifactKey[family]
            ? inv.findIndex((art) => _renderArtifactStableKey(art) === _renderSelectedArtifactKey[family])
            : -1;
        if (keyIdx >= 0) _renderSelectedArtifact[family] = keyIdx;
        const idx = _renderSelectedArtifact[family];
        if (!inv.length) {
            _renderSelectedArtifact[family] = -1;
            _renderSelectedArtifactKey[family] = '';
        } else if (idx == null || idx < 0 || idx >= inv.length) {
            _renderSelectedArtifact[family] = 0;
            _renderSelectedArtifactKey[family] = _renderArtifactStableKey(inv[0]);
        } else {
            _renderSelectedArtifactKey[family] = _renderArtifactStableKey(inv[_renderSelectedArtifact[family]]);
        }
    }

    _lastCalcHasLores = !!(calc.exists && calc.degree);
    const activeFamilyInv = _renderArtifacts[_renderActiveFamily] || [];
    const activeIdx = _renderSelectedArtifact[_renderActiveFamily];
    const activeArt = activeIdx >= 0 ? activeFamilyInv[activeIdx] : null;
    const linkedPaletteId = _linkedPaletteIdForColorArtifact(activeArt);
    const linkedColorId = _linkedColorIdForPaletteArtifact(activeArt);
    const favoriteSelected = _renderActiveFamily === 'color' && activeArt && _isFavorite(jobId, activeArt.artifact_id);
    const familyTabs = ['color', 'bilevel', 'coeffs', 'palette', 'pdf', 'sculpture'].map((family) => {
        const active = family === _renderActiveFamily;
        const count = (_renderArtifacts[family] || []).length;
        return `<button type="button" role="tab" class="subtab-btn${active ? ' active' : ''}" data-render-family="${family}" onclick="_renderSelectFamily('${family}')" aria-selected="${active ? 'true' : 'false'}">${_renderFamilyLabel(family)} <span class="subtab-count">(${count})</span></button>`;
    }).join('');

    if (_renderActiveFamily === 'sculpture') {
        // saved-sculpture pane: DeepZoom-style — a create block bound to the
        // CURRENT render settings plus the global durable list. Selection /
        // marquee / viewer machinery does not apply here.
        preview.innerHTML = `
        <div style="border:1px solid #333; border-radius:6px; padding:10px; background:#141424">
            <div class="subtab-bar render-artifact-family-tabs" role="tablist" aria-label="Render artifact family tabs">${familyTabs}</div>
            <div style="display:flex; gap:8px; align-items:center; margin-bottom:8px; flex-wrap:wrap">
                <input type="text" id="sculpture-title" placeholder="sculpture title (optional)" style="flex:0 1 340px; background:#101020; border:1px solid #444; border-radius:4px; color:#eee; padding:5px 8px; font-family:monospace; font-size:12px">
                <button type="button" class="btn-secondary btn-inline" id="btn-sculpture-create" onclick="runSculptureSave()">Create</button>
                <button type="button" class="btn-secondary btn-inline" id="btn-sculpture-refresh" onclick="_sculptureEnsureInventory(true)">Refresh</button>
                <span style="font-size:11px; color:#666">Create solves with the current Solve-score settings and saves a permanent, shareable sculpture (frozen viewer + data under sculptures/).</span>
            </div>
            <div id="sculpture-list" style="max-height:520px; overflow-y:auto; border:1px solid #333; border-radius:4px">Loading…</div>
        </div>`;
        _sculptureRenderPane();
        void _sculptureEnsureInventory();
        info.textContent = 'Job: ' + jobId;
        return;
    }

    let controlsExtra = '';
    if (_renderActiveFamily === 'color') {
        const colorArtifactId = activeArt && activeArt.artifact_id ? String(activeArt.artifact_id) : '';
        controlsExtra = `<div style="display:flex; align-items:center; gap:8px; max-width:100%">
            <span style="font-size:11px; color:#888; text-transform:uppercase; letter-spacing:.05em; white-space:nowrap">Selected Color</span>
            <input type="text" readonly value="${_escapeHtml(colorArtifactId)}" placeholder="No Color artifact selected" onclick="this.select()" style="flex:1; min-width:220px; max-width:720px; background:#101020; border:1px solid #444; border-radius:4px; color:#eee; padding:5px 8px; font-family:monospace; font-size:12px">
        </div>`;
    } else if (_renderActiveFamily === 'palette') {
        controlsExtra = '<span style="font-size:11px; color:#666">Generate uses current Solve score metric, palette, q, and root transforms. RePalette reuses the selected palette artifact’s saved bins and only changes the final palette mapping. Populate copies the selected palette into Color settings and switches to Color. Palette summaries show the solve display, palette name, and source Color artifact id.</span>';
    } else if (_renderActiveFamily === 'bilevel') {
        controlsExtra = '<span style="font-size:11px; color:#666">Generate opens the BiLevel popup and runs the logical-section path: sectioned solve reads, sparse occupancy fragments, then one assemble+encode finalize. Color2Bilevel is the faster derived path from a fused Color artifact and appears on the Color tab.</span>';
    } else if (_renderActiveFamily === 'pdf') {
        controlsExtra = '<span style="font-size:11px; color:#666">PDF artifacts are generated from the selected Color artifact using the PDF button on the Color tab. This tab lists and previews completed PDFs.</span>';
    }

    let catalogHtml = '';
    if (!activeFamilyInv.length) {
        catalogHtml = '<div style="color:#666; padding:8px">No saved artifacts yet.</div>';
    } else {
        catalogHtml = '<table style="width:100%; border-collapse:collapse"><tr style="border-bottom:1px solid #333; position:sticky; top:0; background:#1a1a2e"><th style="text-align:left;padding:4px">Created</th><th>Dims</th><th>Size</th><th style="text-align:left">Summary</th></tr>';
        activeFamilyInv.forEach((art, i) => {
            const isSel = i === activeIdx;
            const dims = art.width && art.height ? `${art.width}x${art.height}` : '';
            const created = (art.created_at || '').replace('T', ' ').slice(0, 19);
            const key = _encodeStableRowKey(_renderArtifactStableKey(art));
            catalogHtml += `<tr id="${_renderRowElementId(_renderActiveFamily, i)}" class="render-art-row" data-family="${_renderActiveFamily}" data-key="${key}" onclick="_renderSelectArtifactByKey('${_renderActiveFamily}', _decodeStableRowKey(this.dataset.key || ''))" style="border-bottom:1px solid #222; cursor:pointer; background:${isSel ? '#2a2a4e' : ''}">`;
            catalogHtml += `<td style="padding:4px; font-size:11px">${created}</td>`;
            catalogHtml += `<td style="padding:4px; text-align:center">${dims}</td>`;
            catalogHtml += `<td style="padding:4px; text-align:center">${_fmtSize(art.file_size || art.size || 0)}</td>`;
            catalogHtml += `<td style="padding:4px">${_escapeHtml(_renderArtifactSummary(art))}</td>`;
            catalogHtml += '</tr>';
        });
        catalogHtml += '</table>';
    }

    let viewerHtml = '<div style="color:#444; font-size:12px; padding:12px 0; text-align:center">No artifact selected</div>';
    if (activeArt && activeArt.viewer_url) {
        if (activeArt.format === 'pdf' || (activeArt.content_type || '') === 'application/pdf') {
            const pdfSrc = `${activeArt.viewer_url}#toolbar=0&navpanes=0&view=FitH`;
            viewerHtml = `<iframe src="${pdfSrc}" style="width:100%; height:100%; border:0; background:#000"></iframe>`;
        } else {
            viewerHtml = `<div id="render-preview-stage" class="render-preview-stage inactive"><img id="render-preview-image" class="render-preview-image" src="${activeArt.viewer_url}" draggable="false"><div id="render-preview-marquee" class="render-preview-marquee" aria-hidden="true"></div></div>`;
        }
    }

    const actionButtons = [];
    if (_renderActiveFamily !== 'color' && _renderActiveFamily !== 'pdf') {
        actionButtons.push('<button class="btn-primary" id="btn-render-generate" onclick="generateSelectedRenderArtifact()" style="padding:4px 12px; font-size:11px">Generate</button>');
    }
    if (_renderActiveFamily === 'color') actionButtons.push('<button class="btn-primary" id="btn-render-generate-mt" onclick="generateSelectedRenderArtifactMT()" style="padding:4px 12px; font-size:11px">ColorRender-MT</button>');
    if (_renderActiveFamily === 'color') actionButtons.push('<button class="btn-secondary" id="btn-render-pdf-colorspread" onclick="runPdfColorSpreadSelectedRenderArtifact()" style="padding:4px 12px; font-size:11px" disabled>PDF</button>');
    if (_renderActiveFamily === 'color') actionButtons.push('<button class="btn-secondary" id="btn-render-extract-palette" onclick="openExtractPalettePopup()" style="padding:4px 12px; font-size:11px" disabled>ExtractPalette</button>');
    if (_renderActiveFamily === 'color') actionButtons.push('<button class="btn-secondary" id="btn-render-color-repalette" onclick="openColorRepalettePopup()" style="padding:4px 12px; font-size:11px" disabled>RePalette</button>');
    if (_renderActiveFamily === 'color') actionButtons.push('<button class="btn-secondary" id="btn-render-color-to-bilevel" onclick="openColorToBilevelPopup()" style="padding:4px 12px; font-size:11px" disabled>Color2Bilevel</button>');
    if (_renderActiveFamily === 'palette') actionButtons.push('<button class="btn-secondary" id="btn-render-repalette" onclick="openRepalettePopup()" style="padding:4px 12px; font-size:11px" disabled>RePalette</button>');
    if (_renderActiveFamily === 'bilevel') actionButtons.push('<button class="btn-secondary" id="btn-png-export" onclick="runPngExport(document.getElementById(\'render-results-dir\').value.trim(), (_renderSelectedArtifactEntry() || {}).image_key)" style="padding:4px 12px; font-size:11px" disabled>PNG</button>');
    if (_renderActiveFamily === 'bilevel') actionButtons.push('<button class="btn-secondary" id="btn-tiff-compat" onclick="runTiffCompat(document.getElementById(\'render-results-dir\').value.trim(), (_renderSelectedArtifactEntry() || {}).image_key)" style="padding:4px 12px; font-size:11px" disabled>Preview-Compatible TIFF</button>');
    if (_renderActiveFamily === 'color' || _renderActiveFamily === 'palette' || _renderActiveFamily === 'bilevel' || _renderActiveFamily === 'coeffs') actionButtons.push('<button class="btn-secondary" id="btn-render-populate" onclick="populateSelectedRenderArtifact()" style="padding:4px 12px; font-size:11px" disabled>Populate</button>');
    if (_renderActiveFamily === 'color') actionButtons.push('<button class="btn-secondary" id="btn-render-autolevels" onclick="openAutolevelPopup()" style="padding:4px 12px; font-size:11px" disabled>Autolevels</button>');
    if (_renderActiveFamily === 'color') actionButtons.push('<button class="btn-secondary" id="btn-render-resize" onclick="openResizePopup()" style="padding:4px 12px; font-size:11px" disabled>Resize</button>');
    if (_renderActiveFamily === 'color') actionButtons.push('<button class="btn-secondary" id="btn-render-go-result" onclick="goResultFromRender()" style="padding:4px 12px; font-size:11px" disabled>GoResult</button>');
    if (_renderActiveFamily === 'color') actionButtons.push(`<button class="btn-secondary" id="btn-render-go-palette" onclick="goPaletteFromRender()" style="padding:4px 12px; font-size:11px" disabled>GoPalette${linkedPaletteId ? ': ' + linkedPaletteId : ''}</button>`);
    if (_renderActiveFamily === 'color') actionButtons.push(`<button class="btn-secondary" id="btn-render-favorite" onclick="favoriteSelectedRenderArtifact()" style="padding:4px 12px; font-size:11px" ${favoriteSelected ? 'disabled' : ''}>${favoriteSelected ? 'Favorited' : 'Favorite'}</button>`);
    if (_renderActiveFamily === 'color') actionButtons.push(`<button class="btn-secondary" id="btn-render-add-book" onclick="addSelectedRenderArtifactToBook()" style="padding:4px 12px; font-size:11px">Add to Book</button>`);
    if (_renderActiveFamily === 'palette') actionButtons.push(`<button class="btn-secondary" id="btn-render-go-color" onclick="goColorFromPalette()" style="padding:4px 12px; font-size:11px" disabled>GoColor${linkedColorId ? ': ' + linkedColorId : ''}</button>`);
    actionButtons.push(`<div style="display:inline-block; position:relative">
        <button class="btn-secondary" id="btn-render-download" onclick="_toggleDownloadMenu()" style="padding:4px 12px; font-size:11px" disabled>Download \u25bc</button>
        <div id="download-menu" style="display:none; position:absolute; bottom:100%; left:0; background:#2a2a3e; border:1px solid #555; border-radius:4px; z-index:100; min-width:120px; margin-bottom:2px">
            ${_renderActiveFamily === 'pdf'
                ? `<div id="dl-menu-file" onclick="_dlMenuAction('pdf_file')" style="padding:6px 12px; cursor:pointer; font-size:11px; color:#ccc; white-space:nowrap" onmouseover="this.style.background='#3a3a5e'" onmouseout="this.style.background=''">Save PDF</div>
                   <div id="dl-menu-dir" onclick="_dlMenuAction('dir')" style="padding:6px 12px; cursor:pointer; font-size:11px; color:#ccc; white-space:nowrap; border-top:1px solid #444" onmouseover="this.style.background='#3a3a5e'" onmouseout="this.style.background=''">Select Dir\u2026</div>`
                : `<div id="dl-menu-file" onclick="_dlMenuAction('image')" style="padding:6px 12px; cursor:pointer; font-size:11px; color:#ccc; white-space:nowrap" onmouseover="this.style.background='#3a3a5e'" onmouseout="this.style.background=''">Image</div>
                   <div onclick="_dlMenuAction('image+meta')" style="padding:6px 12px; cursor:pointer; font-size:11px; color:#ccc; white-space:nowrap; border-top:1px solid #444" onmouseover="this.style.background='#3a3a5e'" onmouseout="this.style.background=''">Image + Meta</div>
                   <div id="dl-menu-dir" onclick="_dlMenuAction('dir')" style="padding:6px 12px; cursor:pointer; font-size:11px; color:#ccc; white-space:nowrap; border-top:1px solid #444" onmouseover="this.style.background='#3a3a5e'" onmouseout="this.style.background=''">Save to Dir\u2026</div>`}
        </div>
    </div>`);
    actionButtons.push('<button class="btn-secondary" id="btn-render-delete" onclick="deleteSelectedRenderArtifact()" style="padding:4px 12px; font-size:11px" disabled>Delete</button>');
    if (_renderActiveFamily !== 'pdf') actionButtons.push('<button class="btn-secondary" id="btn-render-deepzoom" onclick="deepZoomSelectedRenderArtifact()" style="padding:4px 12px; font-size:11px" disabled>DeepZoom</button>');
    const actionRowsHtml = _renderActionButtonRows(actionButtons, 5);
    const navigationHintHtml = _renderActiveFamily === 'color'
        ? ''
        : `<div style="font-size:11px; color:#666">Click row to view. Arrow keys navigate within ${_renderFamilyLabel(_renderActiveFamily)}.</div>`;

    preview.innerHTML = `
        <div style="border:1px solid #333; border-radius:6px; padding:10px; background:#141424">
            <div class="subtab-bar render-artifact-family-tabs" role="tablist" aria-label="Render artifact family tabs">${familyTabs}</div>
            <div style="display:flex; flex-direction:column; gap:8px; margin-bottom:8px">
                ${actionRowsHtml}
                ${navigationHintHtml}
            </div>
            <div style="font-size:11px; color:#666; margin-bottom:8px">${controlsExtra}</div>
            <div style="display:grid; grid-template-columns:minmax(340px, 44%) 1fr; gap:12px; align-items:start">
                <div id="render-artifact-catalog" style="height:${panelViewportPx}px; max-height:${panelViewportPx}px; overflow-y:auto; border:1px solid #333; border-radius:4px">${catalogHtml}</div>
                <div id="render-artifact-viewer" style="height:${panelViewportPx}px; max-height:${panelViewportPx}px; border:1px solid #333; border-radius:4px; background:#000; padding:8px; display:flex; align-items:center; justify-content:center; overflow:hidden">${viewerHtml}</div>
            </div>
        </div>
    `;

    _initRenderPreviewMarquee(activeArt);
    _updateSolveScoreButtons();
    _updateRenderActionButtons();
    if (options.preserveScroll) {
        _restoreRenderCatalogScroll({ family: _renderActiveFamily, ensureSelected: !!options.ensureSelected });
    } else {
        _renderCatalogScrollTop[_renderActiveFamily] = 0;
        _restoreRenderCatalogScroll({ family: _renderActiveFamily, ensureSelected: false });
    }

    // Info line
    const parts = ['Job: ' + jobId];
    if (calc.exists) {
        if (calc.N || calc.n1) parts.push('N=' + (calc.N || calc.n1));
        if (calc.degree) parts.push('degree ' + calc.degree);
    }
    info.textContent = parts.join(' | ');
}

async function runTiffCompat(jobId, sourceKey, pix) {
    const art = _renderSelectedArtifactEntry();
    jobId = jobId || document.getElementById('render-results-dir').value.trim();
    sourceKey = sourceKey || (art && art.image_key) || '';
    if (!jobId || !art || !sourceKey) return;
    const btn = document.getElementById('btn-tiff-compat');
    if (btn) btn.disabled = true;
    log(`TIFF compat: converting ${sourceKey}...`, '', 'render-log');
    try {
        const artifactId = 'compat_' + Date.now() + '_' + Math.random().toString(36).slice(2, 8);
        const result = await lambdaPost('tiff-compat', {
            job_id: jobId,
            source_key: sourceKey,
            artifact_id: artifactId,
            source_artifact_id: art.artifact_id || '',
            task_id: 'tiff_compat_' + artifactId,
        });
        log(`  compat TIFF: ${result.convert_ms}ms, ${(result.file_size/1024).toFixed(0)} KB -> ${artifactId}`, 'ok', 'render-log');
        await refreshRenderArtifacts(jobId, { selectFamily: 'bilevel', selectArtifactId: result.artifact_id || artifactId });
    } catch (e) {
        log(`  compat failed: ${e.message}`, 'err', 'render-log');
        if (btn) { btn.textContent = 'Compat failed'; btn.disabled = false; }
    }
}

async function runPngExport(jobId, sourceKey, pix) {
    const art = _renderSelectedArtifactEntry();
    jobId = jobId || document.getElementById('render-results-dir').value.trim();
    sourceKey = sourceKey || (art && art.image_key) || '';
    if (!jobId || !art || !sourceKey) return;
    const btn = document.getElementById('btn-png-export');
    if (btn) { btn.disabled = true; btn.textContent = 'Converting...'; }
    log(`PNG export: converting ${sourceKey}...`, '', 'render-log');
    try {
        const artifactId = 'png_' + Date.now() + '_' + Math.random().toString(36).slice(2, 8);
        const result = await lambdaPost('png-export', {
            job_id: jobId,
            source_key: sourceKey,
            artifact_id: artifactId,
            source_artifact_id: art.artifact_id || '',
            task_id: 'png_export_' + artifactId,
        });
        log(`  PNG export: ${result.convert_ms}ms, ${(result.file_size/1024).toFixed(0)} KB -> ${artifactId}`, 'ok', 'render-log');
        await refreshRenderArtifacts(jobId, { selectFamily: 'bilevel', selectArtifactId: result.artifact_id || artifactId });
    } catch (e) {
        log(`  PNG export failed: ${e.message}`, 'err', 'render-log');
        if (btn) { btn.textContent = 'PNG failed'; btn.disabled = false; }
    }
}

/* ---- DeepZoom export and viewer ---- */

async function runDeepZoomExport(jobId, sourceKey, btnEl, options = null) {
    const btn = btnEl || document.getElementById('btn-deepzoom');
    if (btn) { btn.disabled = true; btn.textContent = 'Creating...'; }
    const rawKey = options && typeof options === 'object' ? String(options.rawKey || '') : '';
    const rawMetaKey = options && typeof options === 'object' ? String(options.rawMetaKey || '') : '';
    const skipRenderRefresh = !!(options && options.skipRenderRefresh);
    const useExactSource = !!String(sourceKey || '').trim();
    const dispatchTarget = useExactSource ? 'deepzoom_export' : ((rawKey && rawMetaKey) ? 'deepzoom_from_raw' : 'deepzoom_export');
    // ONE operation identity end-to-end (code-review-29 F1): a unique export_id
    // (the S3 prefix) + task_id (the status row) minted here and threaded
    // through dispatch, so overlapping exports for one job can never share a
    // status row (false completion) or an export prefix (mixed tile sets).
    const opId = Date.now().toString(36) + Math.random().toString(36).slice(2, 10);
    const exportId = 'dz_' + opId;
    const taskId = dispatchTarget + '_' + opId;
    // Every dispatched job rides the jobs rail (user rule): card up before the
    // dispatch so even an immediate failure is visible, ticked while polling.
    const railId = 'deepzoom:' + jobId + ':' + Date.now().toString(36);
    const rail = (patch) => { if (typeof _jobsRailUpsert === 'function') _jobsRailUpsert({
        id: railId, kind: 'deepzoom', label: 'deepzoom · ' + jobId, jobId,
        tab: 'deepzoom', ...patch }); };
    rail({ state: 'running', startedAt: Date.now(), detail: 'dispatching' });
    log(`DeepZoom: exporting ${sourceKey}...`, '', 'render-log');
    try {
        // Dispatch async — API Gateway has a 30s hard limit, DeepZoom takes 30-120s.
        // No pre-dispatch delete-task: the task_id is unique per launch, so there
        // is no shared row to go stale against.
        const dispResult = await lambdaPost('dispatch', {
            target: dispatchTarget,
            jobs: [useExactSource ? {
                job_id: jobId,
                source_key: sourceKey,
                export_id: exportId,
                task_id: taskId,
            } : {
                job_id: jobId,
                source_key: sourceKey,
                raw_key: rawKey,
                raw_meta_key: rawMetaKey,
                export_id: exportId,
                task_id: taskId,
            }],
            expected_keys: [],
        });
        if ((dispResult.fired || 0) !== 1)
            throw new Error(`DeepZoom dispatch failed: fired ${dispResult.fired || 0}/1`);
        if (dispResult.non_202 && dispResult.non_202.length > 0)
            throw new Error(`DeepZoom invoke rejected: status ${dispResult.non_202[0].status}`);
        rail({ detail: 'dispatched' });

        // Poll DynamoDB for completion (task_id matches the dispatch target)
        const t0 = performance.now();
        const TIMEOUT_MS = 600000; // 10 min max
        while (true) {
            await new Promise(r => setTimeout(r, 3000));
            const check = await lambdaPost('storage', {
                job_id: jobId, task_prefix: taskId, expected: 1,
            }, '/check-status');
            if (check.errors > 0) {
                const msg = check.error_details?.[0]?.error_msg || 'unknown';
                throw new Error(`DeepZoom export failed: ${msg}`);
            }
            if (check.complete) break;
            const elapsed = performance.now() - t0;
            if (elapsed > TIMEOUT_MS)
                throw new Error(`DeepZoom timed out after ${_fmtMs(elapsed)}`);
            if (btn) btn.textContent = `Creating... ${check.status_counts?.started ? '(running)' : `(${_fmtMs(elapsed)})`}`;
            if (typeof _jobsRailProgress === 'function') _jobsRailProgress(railId, `tiling ${_fmtMs(elapsed)}`);
        }
        log(`  DeepZoom export complete (${_fmtMs(performance.now() - t0)})`, 'ok', 'render-log');
        rail({ state: 'done', detail: `export complete (${_fmtMs(performance.now() - t0)})` });
        // Completed rows are one-shot — clean up (failures keep theirs as evidence).
        void lambdaPost('storage', { job_id: jobId, task_id: taskId }, '/delete-task').catch(() => {});
        void _dzPatchInventoryAfterExport(jobId, exportId);
        if (!skipRenderRefresh) await refreshRenderArtifacts(jobId);
    } catch (e) {
        log(`  DeepZoom failed: ${e.message}`, 'err', 'render-log');
        rail({ state: 'failed', detail: String(e.message || e) });
        if (btn) { btn.textContent = 'DeepZoom failed'; btn.disabled = false; }
    }
}

let _osdViewer = null;
let _dzInventoryLoaded = false;   // session cache (favorites-speedup idea 1)
let _dzInventoryPromise = null;   // in-flight dedup: overlapping loads share one fetch
const _dzViewportReadoutState = { rafPending: false, meta: null, visibleBounds: null };
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

function _requestAnimationFrameCompat(fn) {
    if (typeof window.requestAnimationFrame === 'function') return window.requestAnimationFrame(fn);
    return window.setTimeout(fn, 0);
}

function _dzSelectedExportEntry() {
    const inv = window._dzInventory || [];
    const idx = window._dzSelectedIdx ?? -1;
    return idx >= 0 && idx < inv.length ? inv[idx] : null;
}

function _formatDeepZoomViewportNumber(value) {
    const num = Number(value);
    if (!Number.isFinite(num)) return '?';
    if (num === 0) return '0';
    const mag = Math.abs(num);
    return mag >= 1e-4 && mag < 1e6 ? num.toFixed(6) : num.toExponential(6);
}

function _setDeepZoomViewportReadout(text) {
    const el = document.getElementById('deepzoom-viewport-readout');
    if (el) el.textContent = String(text || 'Visible world viewport unavailable');
}

function _deepZoomViewportMeta(ex) {
    if (!ex) return null;
    const width = Number(ex.width);
    const height = Number(ex.height);
    const minRe = Number(ex.viewport_min_re);
    const maxRe = Number(ex.viewport_max_re);
    const minIm = Number(ex.viewport_min_im);
    const maxIm = Number(ex.viewport_max_im);
    const rotation = Number(ex.source_rotation);
    return {
        width,
        height,
        minRe,
        maxRe,
        minIm,
        maxIm,
        rotation: Number.isFinite(rotation) ? rotation : 0,
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

function _clampDeepZoomViewportCoord(value, lo, hi) {
    return Math.max(lo, Math.min(hi, value));
}

function _rotateDeepZoomPoint(re, im, pivotRe, pivotIm, angle) {
    const dx = Number(re) - Number(pivotRe);
    const dy = Number(im) - Number(pivotIm);
    const cosA = Math.cos(angle);
    const sinA = Math.sin(angle);
    return {
        re: Number(pivotRe) + (dx * cosA - dy * sinA),
        im: Number(pivotIm) + (dx * sinA + dy * cosA),
    };
}

function _deepZoomCameraRectFromImageRect(meta, imgRect) {
    const x0 = _clampDeepZoomViewportCoord(Number(imgRect.x) || 0, 0, meta.width);
    const x1 = _clampDeepZoomViewportCoord(x0 + (Number(imgRect.width) || 0), 0, meta.width);
    const y0 = _clampDeepZoomViewportCoord(Number(imgRect.y) || 0, 0, meta.height);
    const y1 = _clampDeepZoomViewportCoord(y0 + (Number(imgRect.height) || 0), 0, meta.height);
    const min_re = meta.minRe + (x0 / meta.width) * (meta.maxRe - meta.minRe);
    const max_re = meta.minRe + (x1 / meta.width) * (meta.maxRe - meta.minRe);
    const max_im = meta.maxIm - (y0 / meta.height) * (meta.maxIm - meta.minIm);
    const min_im = meta.maxIm - (y1 / meta.height) * (meta.maxIm - meta.minIm);
    if (![min_re, max_re, min_im, max_im].every((value) => Number.isFinite(value))) return null;
    return {
        min_re,
        max_re,
        min_im,
        max_im,
        center_re: (min_re + max_re) / 2,
        center_im: (min_im + max_im) / 2,
        span_re: max_re - min_re,
        span_im: max_im - min_im,
    };
}

function _deepZoomRenderBoundsFromCameraRect(meta, cameraRect) {
    if (!meta || !cameraRect) return null;
    const viewportCenterRe = (meta.minRe + meta.maxRe) / 2;
    const viewportCenterIm = (meta.minIm + meta.maxIm) / 2;
    let centerRe = cameraRect.center_re;
    let centerIm = cameraRect.center_im;
    if (meta.rotation !== 0) {
        const unrot = _rotateDeepZoomPoint(
            cameraRect.center_re,
            cameraRect.center_im,
            viewportCenterRe,
            viewportCenterIm,
            -meta.rotation
        );
        centerRe = unrot.re;
        centerIm = unrot.im;
    }
    const min_re = centerRe - (cameraRect.span_re / 2);
    const max_re = centerRe + (cameraRect.span_re / 2);
    const min_im = centerIm - (cameraRect.span_im / 2);
    const max_im = centerIm + (cameraRect.span_im / 2);
    if (![min_re, max_re, min_im, max_im].every((value) => Number.isFinite(value))) return null;
    return {
        min_re,
        max_re,
        min_im,
        max_im,
        center_re: centerRe,
        center_im: centerIm,
        span_re: cameraRect.span_re,
        span_im: cameraRect.span_im,
        rotation: meta.rotation,
    };
}

function _computeDeepZoomVisibleBounds(viewer, meta) {
    if (!viewer || !meta || !meta.hasViewport) return null;
    const vp = viewer.viewport;
    if (!vp || typeof vp.getBounds !== 'function' || typeof vp.viewportToImageRectangle !== 'function') return null;
    const bounds = vp.getBounds(true);
    if (!bounds) return null;
    const imgRect = vp.viewportToImageRectangle(bounds);
    if (!imgRect) return null;
    const cameraRect = _deepZoomCameraRectFromImageRect(meta, imgRect);
    if (!cameraRect) return null;
    return _deepZoomRenderBoundsFromCameraRect(meta, cameraRect);
}

function _renderDeepZoomViewportReadout() {
    const ex = _dzSelectedExportEntry();
    const meta = _deepZoomViewportMeta(ex);
    _dzViewportReadoutState.meta = meta;
    _dzViewportReadoutState.visibleBounds = null;
    if (!meta || !meta.hasViewport) {
        _setDeepZoomViewportReadout('Visible world viewport unavailable');
        return;
    }
    const visible = _computeDeepZoomVisibleBounds(_osdViewer, meta);
    if (!visible) {
        _setDeepZoomViewportReadout('Visible world viewport unavailable');
        return;
    }
    _dzViewportReadoutState.visibleBounds = visible;
    let text = `Visible world viewport\n`
        + `min_re=${_formatDeepZoomViewportNumber(visible.min_re)}  max_re=${_formatDeepZoomViewportNumber(visible.max_re)}\n`
        + `min_im=${_formatDeepZoomViewportNumber(visible.min_im)}  max_im=${_formatDeepZoomViewportNumber(visible.max_im)}\n`
        + `center=(${_formatDeepZoomViewportNumber(visible.center_re)}, ${_formatDeepZoomViewportNumber(visible.center_im)})\n`
        + `span=(${_formatDeepZoomViewportNumber(visible.span_re)} x ${_formatDeepZoomViewportNumber(visible.span_im)})`;
    if (meta.rotation !== 0) {
        text += `\nrotation=${_formatDeepZoomViewportNumber(meta.rotation)} (preserved)`;
    }
    _setDeepZoomViewportReadout(text);
}

function _scheduleDeepZoomViewportReadout() {
    if (_dzViewportReadoutState.rafPending) return;
    _dzViewportReadoutState.rafPending = true;
    _requestAnimationFrameCompat(() => {
        _dzViewportReadoutState.rafPending = false;
        _renderDeepZoomViewportReadout();
    });
}

async function loadDeepZoomInventory(options = {}) {
    // Session cache: exports are immutable and only change on OUR exports (which
    // patch the cache in place), so tab re-entry costs zero requests. Refresh
    // passes force. Cross-device exports appear on explicit Refresh — by design.
    const force = !!(options && options.force);
    if (_dzInventoryLoaded && !force) return;
    if (_dzInventoryPromise) return _dzInventoryPromise;
    _dzInventoryPromise = _dzLoadInventoryNow(options);
    try {
        return await _dzInventoryPromise;
    } finally {
        _dzInventoryPromise = null;
    }
}

async function _dzLoadInventoryNow(options) {
    const container = document.getElementById('deepzoom-inventory');
    const statusEl = document.getElementById('deepzoom-status');
    const hadInventory = (window._dzInventory || []).length > 0;
    _dzSetButtonsEnabled(false);
    if (statusEl) {
        statusEl.textContent = hadInventory ? 'Refreshing...' : 'Loading...';
        statusEl.className = 'status';
    }
    // Never blank a live table for a refresh — only for a cold load.
    if (!hadInventory) container.innerHTML = 'Loading...';

    try {
        // Single server-side call: lists all exports and reads their meta.json
        const data = await lambdaPost('storage', {}, '/list-deepzoom');
        const exports = data.exports || [];

        if (exports.length === 0) {
            _dzInventoryLoaded = true;   // EMPTY is a loaded result, not uninitialized (CR30 F12)
            window._dzInventory = [];
            window._dzSelectedIdx = -1;
            window._dzSelectedKey = '';
            _dzClearViewer();
            container.innerHTML = '<div style="color:#666">No DeepZoom exports yet. Generate one from the Render tab.</div>';
            if (statusEl) {
                statusEl.textContent = 'Ready';
                statusEl.className = 'status';
            }
            return;
        }

        // Sort newest first, store for keyboard navigation
        const sorted = exports.sort((a, b) => (b.created_at || '').localeCompare(a.created_at || ''));
        window._dzInventory = sorted;
        _dzInventoryLoaded = true;
        _dzRenderInventory();
        if (statusEl) {
            statusEl.textContent = 'Ready';
            statusEl.className = 'status ok';
        }
    } catch (e) {
        if ((window._dzInventory || []).length) {
            // Transient failure with a good list on screen: KEEP it (favorites
            // rule — a blip must never read as "no exports"). Selection and the
            // open viewer survive; only the status line reports the failure.
            _dzSetButtonsEnabled(true);
            if (statusEl) {
                statusEl.textContent = 'Refresh failed: ' + e.message + ' — showing cached list';
                statusEl.className = 'status error';
            }
        } else {
            window._dzInventory = [];
            window._dzSelectedIdx = -1;
            window._dzSelectedKey = '';
            _dzClearViewer();
            if (statusEl) {
                statusEl.textContent = 'Refresh failed: ' + e.message;
                statusEl.className = 'status error';
            }
            _setInlineError(container, e.message);
        }
        if (options.throwOnError) throw e;
    }
}

// After OUR export completes we know its exact identity — one public meta.json
// GET patches the cached inventory in place instead of a full /list-deepzoom
// rescan (favorites-speedup idea 4). On any failure the cache is marked stale
// so the next tab entry refetches everything.
async function _dzPatchInventoryAfterExport(jobId, exportId) {
    if (!_dzInventoryLoaded && !_dzInventoryPromise) return;   // tab never loaded
    try {
        if (_dzInventoryPromise) { try { await _dzInventoryPromise; } catch (e) {} }
        const resp = await fetch(_publicStorageUrl(`deepzoom/${jobId}/${exportId}/meta.json`), { cache: 'no-store' });
        if (!resp.ok) throw new Error('HTTP ' + resp.status);
        const meta = await resp.json();
        const inv = window._dzInventory || [];
        if (!inv.some((e) => e.job_id === meta.job_id && e.export_id === meta.export_id)) {
            inv.unshift(meta);   // newest export first — matches created_at ordering
            window._dzInventory = inv;
        }
        _dzRenderInventory();
    } catch (e) {
        _dzInventoryLoaded = false;   // stale — next tab entry does a full refresh
    }
}

function _sculptureShareUrl(meta) {
    const prefix = meta.prefix || `sculptures/${meta.id}/`;
    return _publicStorageUrl(prefix + 'viewer.html');
}

function _sculptureRenderPane() {
    const el = document.getElementById('sculpture-list');
    if (!el) return;
    if (!window._sculptureInventoryLoaded) {
        el.innerHTML = '<div style="padding:10px; color:#666">Loading…</div>';
        return;
    }
    const inv = window._sculptureInventory || [];
    if (!inv.length) {
        el.innerHTML = '<div style="padding:10px; color:#666">No saved sculptures yet. Set up a Solve-score render and press Create.</div>';
        return;
    }
    el.innerHTML = inv.map((m, i) => `
        <div style="display:flex; align-items:center; gap:10px; padding:6px 10px; border-bottom:1px solid #26263a; font-size:12px">
            <span style="flex:1; min-width:0; overflow:hidden; text-overflow:ellipsis; white-space:nowrap; color:#e8eef5">${_escapeHtml(m.title || m.id || '')}</span>
            <span style="color:#778599; font-family:monospace">${_escapeHtml(m.job_id || '')}</span>
            <span style="color:#778599; white-space:nowrap">${Number(m.grid_n) || '?'}×${Number(m.grid_n) || '?'} · d${Number(m.degree) || '?'}</span>
            <span style="color:#778599">${_escapeHtml(String(m.palette || ''))}</span>
            <span style="color:#556; font-size:11px; white-space:nowrap">${_escapeHtml(String(m.created_at || '').slice(0, 16).replace('T', ' '))}</span>
            <button type="button" class="btn-secondary btn-inline" onclick="_sculptureOpen(${i})">Open</button>
            <button type="button" class="btn-secondary btn-inline" onclick="_sculptureCopyLink(${i}, this)">Copy link</button>
            <button type="button" class="btn-secondary btn-inline" onclick="_sculptureDelete(${i}, this)">Delete</button>
        </div>`).join('');
}

async function _sculptureEnsureInventory(force) {
    if (window._sculptureInventoryLoaded && !force) return;
    const btn = document.getElementById('btn-sculpture-refresh');
    if (btn) btn.disabled = true;
    try {
        const data = await lambdaPost('storage', {}, '/list-sculptures');
        window._sculptureInventory = data.sculptures || [];
        window._sculptureInventoryLoaded = true;
        _renderArtifacts.sculpture = window._sculptureInventory;   // family-tab count
    } catch (e) {
        if (!window._sculptureInventoryLoaded) window._sculptureInventory = [];
        log(`Sculpture list failed: ${e.message}`, 'err', 'render-log');
    } finally {
        if (btn) btn.disabled = false;
    }
    _sculptureRenderPane();
}

function _sculptureOpen(idx) {
    const m = (window._sculptureInventory || [])[idx];
    if (m) window.open(_sculptureShareUrl(m), '_blank');
}

async function _sculptureCopyLink(idx, btn) {
    const m = (window._sculptureInventory || [])[idx];
    if (!m) return;
    const url = _sculptureShareUrl(m);
    const orig = btn ? btn.textContent : '';
    try {
        await navigator.clipboard.writeText(url);
        if (btn) btn.textContent = '\u2713 Copied';
    } catch (e) {
        log(`Sculpture link (copy failed, select manually): ${url}`, 'err', 'render-log');
        if (btn) btn.textContent = '\u2717 Copy';
    }
    if (btn) setTimeout(() => { btn.textContent = orig; }, 2000);
}

async function _sculptureDelete(idx, btn) {
    const m = (window._sculptureInventory || [])[idx];
    if (!m) return;
    if (!confirm(`Delete sculpture "${m.title || m.id}"? The share link stops working.`)) return;
    const prefix = m.prefix || `sculptures/${m.id}/`;
    const orig = btn ? btn.textContent : 'Delete';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Deleting…'; }
        await lambdaPost('storage', { prefix }, '/delete-prefix');
        window._sculptureInventory = (window._sculptureInventory || []).filter((_, i) => i !== idx);
        _renderArtifacts.sculpture = window._sculptureInventory;
        _sculptureRenderPane();
        log(`Sculpture deleted: ${m.title || m.id}`, 'ok', 'render-log');
    } catch (e) {
        log(`Sculpture delete failed: ${e.message}`, 'err', 'render-log');
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

async function runSculptureSave() {
    const btn = document.getElementById('btn-sculpture-create');
    const titleEl = document.getElementById('sculpture-title');
    const title = titleEl ? titleEl.value.trim() : '';
    if (btn) { btn.disabled = true; btn.textContent = 'Creating…'; }
    let ok = false;
    try {
        ok = await runRenderLoresSculpture({ save: true, title });
        if (ok) await _sculptureEnsureInventory(true);
    } finally {
        if (btn) {
            btn.disabled = false;
            btn.textContent = ok ? '\u2713 Created' : '\u2717 Create';
            setTimeout(() => { btn.textContent = 'Create'; }, 2500);
        }
    }
}

async function refreshDeepZoomInventory() {
    const btn = document.getElementById('btn-dz-refresh');
    const orig = btn ? btn.textContent : 'Refresh';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Refreshing...'; }
        await loadDeepZoomInventory({ force: true, throwOnError: true });
        log('DeepZoom refreshed', 'ok', 'deepzoom-log');
    } catch (e) {
        log(`DeepZoom refresh failed: ${e.message}`, 'err', 'deepzoom-log');
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

function _dzSetButtonsEnabled(enabled) {
    const gotoResultBtn = document.getElementById('btn-dz-goto-result');
    const gotoRenderBtn = document.getElementById('btn-dz-goto-render');
    const populateBtn = document.getElementById('btn-dz-populate');
    const deleteBtn = document.getElementById('btn-dz-delete');
    const inv = window._dzInventory || [];
    const idx = window._dzSelectedIdx ?? -1;
    const ex = enabled && idx >= 0 && idx < inv.length ? inv[idx] : null;
    if (gotoResultBtn) gotoResultBtn.disabled = !ex || !String(ex.job_id || '').trim();
    if (gotoRenderBtn) gotoRenderBtn.disabled = !_dzRenderSourceRef(ex).jobId;
    if (populateBtn) populateBtn.disabled = !ex || !String(ex.job_id || '').trim();
    if (deleteBtn) deleteBtn.disabled = !ex || !String(ex.export_id || '').trim();
    // "Add to Gallery" stays ENABLED for any selected row — a silently disabled
    // button is a dead click with no explanation (forbidden). When the export
    // can't be added, the click explains exactly why.
    const addGalleryBtn = document.getElementById('btn-dz-add-gallery');
    if (addGalleryBtn) addGalleryBtn.disabled = !ex;
}

function _dzClearViewer() {
    const viewerEl = document.getElementById('deepzoom-viewer');
    if (_osdViewer) {
        _osdViewer.destroy();
        _osdViewer = null;
    }
    window._osdViewer = null;
    if (viewerEl) viewerEl.style.display = 'none';
    _scheduleDeepZoomViewportReadout();
}

function _dzStableKey(ex) {
    if (!ex) return '';
    return `${String(ex.job_id || '')}::${String(ex.export_id || ex.dzi_key || '')}`;
}

;(window.__ppParts = window.__ppParts || []).push('11-artifacts');
