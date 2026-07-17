// PolyPaint 02-preview-solvescore — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Deploy rewrites the script tags to build-versioned asset
// keys (assets/<BUILD_ID>/...), so a deploy flips atomically via index.html.
// --- Preview viewport modes -----------------------------------------------
// quantile (Q%/Shim% auto view), marquee (drag-select on the preview image,
// translated to complex-plane bounds), square (centered side-S box).
let _computePreviewViewportMode = 'quantile';
let _computePreviewMarqueeBounds = null;   // {min_re,max_re,min_im,max_im}
let _computePreviewLastViewport = null;    // bounds of the shown image

function _setComputePreviewViewportMode(mode) {
    _computePreviewViewportMode = (mode === 'marquee' || mode === 'square') ? mode : 'quantile';
    _markComputePreviewStale();
}

function _computePreviewSquareBounds() {
    const side = Math.abs(parseFloat(document.getElementById('compute-preview-square-side')?.value)) || 0;
    if (!(side > 0)) return null;
    return { min_re: -side / 2, max_re: side / 2, min_im: -side / 2, max_im: side / 2 };
}

// sel: fractional image coords {x0,y0,x1,y1} in [0,1] (y down);
// viewport: the displayed image's bounds. Pure — pinned by the frontend harness.
function _computePreviewMarqueeToBounds(sel, viewport) {
    const fx0 = Math.min(sel.x0, sel.x1), fx1 = Math.max(sel.x0, sel.x1);
    const fy0 = Math.min(sel.y0, sel.y1), fy1 = Math.max(sel.y0, sel.y1);
    const w = viewport.max_re - viewport.min_re;
    const h = viewport.max_im - viewport.min_im;
    return {
        min_re: viewport.min_re + fx0 * w,
        max_re: viewport.min_re + fx1 * w,
        max_im: viewport.max_im - fy0 * h,
        min_im: viewport.max_im - fy1 * h,
    };
}

function _computePreviewExplicitBounds() {
    if (_computePreviewViewportMode === 'square') return _computePreviewSquareBounds();
    if (_computePreviewViewportMode === 'marquee') return _computePreviewMarqueeBounds;
    return null;
}

function _formatViewCoord(re, im) {
    const fmt = (x) => (Math.abs(x) >= 1000 ? x.toExponential(2) : x.toFixed(3));
    return `${fmt(re)}${im >= 0 ? '+' : ''}${fmt(im)}i`;
}

function _updateComputePreviewMarqueeInfo() {
    const el = document.getElementById('compute-preview-marquee-info');
    if (!el) return;
    const b = _computePreviewMarqueeBounds;
    el.textContent = b
        ? `UL ${_formatViewCoord(b.min_re, b.max_im)} · LR ${_formatViewCoord(b.max_re, b.min_im)}`
        : 'drag a rectangle on the preview image';
}

function _attachComputePreviewMarquee(img) {
    const box = _computePreviewBox();
    if (!box || !img) return;
    let start = null;
    let rectEl = null;
    const relFrac = (ev) => {
        const r = img.getBoundingClientRect();
        return {
            x: Math.min(1, Math.max(0, (ev.clientX - r.left) / r.width)),
            y: Math.min(1, Math.max(0, (ev.clientY - r.top) / r.height)),
            px: ev.clientX, py: ev.clientY, imgRect: r,
        };
    };
    img.addEventListener('mousedown', (ev) => {
        if (_computePreviewViewportMode !== 'marquee' || !_computePreviewLastViewport) return;
        ev.preventDefault();
        start = relFrac(ev);
        rectEl = document.createElement('div');
        rectEl.className = 'compute-preview-marquee-rect';
        box.appendChild(rectEl);
        const boxRect = box.getBoundingClientRect();
        const draw = (mv) => {
            const cur = relFrac(mv);
            const x0 = Math.min(start.px, cur.px) - boxRect.left;
            const y0 = Math.min(start.py, cur.py) - boxRect.top;
            rectEl.style.left = `${x0}px`;
            rectEl.style.top = `${y0}px`;
            rectEl.style.width = `${Math.abs(cur.px - start.px)}px`;
            rectEl.style.height = `${Math.abs(cur.py - start.py)}px`;
        };
        const finish = (up) => {
            document.removeEventListener('mousemove', draw);
            document.removeEventListener('mouseup', finish);
            if (rectEl) { rectEl.remove(); rectEl = null; }
            const end = relFrac(up);
            if (Math.abs(end.x - start.x) < 0.01 || Math.abs(end.y - start.y) < 0.01) return;
            _computePreviewMarqueeBounds = _computePreviewMarqueeToBounds(
                { x0: start.x, y0: start.y, x1: end.x, y1: end.y },
                _computePreviewLastViewport);
            start = null;
            _updateComputePreviewMarqueeInfo();
            _markComputePreviewStale();
        };
        document.addEventListener('mousemove', draw);
        document.addEventListener('mouseup', finish);
    });
}

function _computePreviewSignatureNow() {
    return JSON.stringify({
        n_preview: parseInt(document.getElementById('compute-preview-n')?.value) || 256,
        preview_size: parseInt(document.getElementById('compute-preview-size')?.value) || 1000,
        solver_mode: document.getElementById('compute-preview-solver')?.value || 'aberth_mt',
        solver_iters: Math.max(0, Math.min(64, parseInt(document.getElementById('compute-preview-iters')?.value, 10) || 0)),
        viewport_mode: _computePreviewViewportMode,
        viewport_bounds: _computePreviewExplicitBounds(),
        quantile: (parseFloat(document.getElementById('compute-preview-quantile')?.value) || 0) / 100,
        shim: (parseFloat(document.getElementById('compute-preview-shim')?.value) || 5) / 100,
        function: document.getElementById('render-function')?.value || '',
        cfpv: _cfpv.length ? [..._cfpv] : [],
        param_transforms: _effectiveParamTransformsForCompute(),
        param_program_chain: _effectiveParamProgramChainForCompute(),
        param_program_source_text: _effectiveParamProgramSourceTextForCompute() || '',
        coeff_transforms: _effectiveCoeffTransformsForCompute(),
        coeff_program_chain: _effectiveCoeffProgramChainForCompute(),
        coeff_program_source_text: _effectiveCoeffProgramSourceTextForCompute() || '',
    });
}

function _setComputePreviewPlaceholder(message) {
    const box = _computePreviewBox();
    if (!box) return;
    box.dataset.hasImage = '0';
    _setPreviewPlaceholder(box, message || 'No compute preview yet');
}

function _renderComputePreviewInfo(lines, stale) {
    const infoEl = _computePreviewInfoEl();
    if (!infoEl) return;
    infoEl.replaceChildren();
    if (stale) {
        const staleEl = document.createElement('div');
        staleEl.style.cssText = 'color:#e9c46a; margin-bottom:6px';
        staleEl.textContent = 'stale';
        infoEl.appendChild(staleEl);
    }
    for (const line of (lines || [])) {
        const row = document.createElement('div');
        row.textContent = line;
        infoEl.appendChild(row);
    }
}

function _markComputePreviewStale() {
    if (!_computePreviewHasImage()) return;
    const current = _computePreviewSignatureNow();
    if (current === _computePreviewSignature) return;
    _computePreviewIsStale = true;
    const statusEl = _computePreviewStatusEl();
    if (statusEl) {
        statusEl.textContent = 'Preview stale — rerun.';
        statusEl.className = 'status';
    }
    const infoEl = _computePreviewInfoEl();
    if (infoEl && !Array.from(infoEl.children || []).some(el => (el.textContent || '').trim() === 'stale')) {
        _renderComputePreviewInfo(Array.from(infoEl.children || []).map(el => el.textContent || ''), true);
    }
}

function _formatComputePreviewViewportNumber(value) {
    const num = Number(value);
    if (!Number.isFinite(num)) return null;
    const mag = Math.abs(num);
    if (mag === 0) return '0';
    return mag >= 1e-4 && mag < 1e6 ? Number(num.toPrecision(7)).toString() : num.toExponential(5);
}

function _computePreviewViewportInfoLines(result) {
    const vp = result && result.viewport;
    if (!vp) return [];
    const minRe = _formatComputePreviewViewportNumber(vp.min_re);
    const maxRe = _formatComputePreviewViewportNumber(vp.max_re);
    const minIm = _formatComputePreviewViewportNumber(vp.min_im);
    const maxIm = _formatComputePreviewViewportNumber(vp.max_im);
    if (minRe == null || maxRe == null || minIm == null || maxIm == null) return [];
    return [
        `viewport Re: ${minRe} .. ${maxRe}`,
        `viewport Im: ${minIm} .. ${maxIm}`,
    ];
}

function _applyComputePreviewResult(result) {
    const box = _computePreviewBox();
    const statusEl = _computePreviewStatusEl();
    const viewportLines = _computePreviewViewportInfoLines(result);
    const viewLine = result.viewport_mode === 'explicit'
        ? (_computePreviewViewportMode === 'square'
            ? `view: square side=${document.getElementById('compute-preview-square-side')?.value || '?'}`
            : 'view: marquee')
        : `view: q=${((Number(result.quantile) || 0) * 100).toFixed(1)}% · shim=${((Number(result.shim) || 0) * 100).toFixed(1)}%`;
    const infoLines = [
        `solver: ${_solverTag(result.solver_mode || 'aberth_mt')}`,
        `image: ${result.image_width}×${result.image_height}`,
        viewLine,
        ...viewportLines,
        `degree: ${result.degree}`,
        `roots: ${result.n_roots_in_view}/${result.n_roots_total} in view`,
        `coeffs: ${((Number(result.coeffs_size) || 0) / 1e6).toFixed(1)}MB`,
        `roots bin: ${((Number(result.roots_size) || 0) / 1e6).toFixed(1)}MB`,
        `coeffgen ${result.coeffgen_ms}ms`,
        `solve ${result.solve_ms}ms`,
        `viewport ${result.viewport_ms}ms`,
        `raster ${result.raster_ms}ms`,
        `total: ${result.total_ms}ms`,
    ];
    if (box) {
        box.replaceChildren();
        const img = document.createElement('img');
        img.src = `data:image/png;base64,${result.image_png_base64}`;
        img.width = result.image_width;
        img.height = result.image_height;
        img.style.cssText = 'max-width:100%; max-height:100%; width:100%; height:100%; object-fit:contain; image-rendering:pixelated; background:#000';
        box.appendChild(img);
        box.dataset.hasImage = '1';
        if (result.viewport && Number.isFinite(Number(result.viewport.min_re))) {
            _computePreviewLastViewport = {
                min_re: Number(result.viewport.min_re), max_re: Number(result.viewport.max_re),
                min_im: Number(result.viewport.min_im), max_im: Number(result.viewport.max_im),
            };
        }
        _attachComputePreviewMarquee(img);
    }
    _computePreviewSignature = _computePreviewSignatureNow();
    _computePreviewIsStale = false;
    if (statusEl) {
        statusEl.textContent = `Preview ready · ${result.image_width}×${result.image_height} · ${result.total_ms}ms`;
        statusEl.className = 'status ok';
    }
    _renderComputePreviewInfo(infoLines, false);
}

function _formatDebugComplex(pair) {
    if (!Array.isArray(pair) || pair.length < 2) return String(pair == null ? '' : pair);
    const re = Number(pair[0]);
    const im = Number(pair[1]);
    if (!Number.isFinite(re) || !Number.isFinite(im)) return `[${pair[0]}, ${pair[1]}]`;
    return _formatCfpvComplexValue(re, im);
}

function _formatDebugComplexRows(label, values, limit = 80) {
    const rows = Array.isArray(values) ? values : [];
    const shown = rows.slice(0, limit).map((pair, idx) => `${label}[${idx}] = ${_formatDebugComplex(pair)}`);
    if (rows.length > limit) shown.push(`${label}: ${rows.length - limit} more values omitted`);
    return shown;
}

function _formatComputeDebugResult(result) {
    const debug = result && result.debug ? result.debug : {};
    const param = debug.param || {};
    const coeff = debug.coeff || {};
    const lines = [
        `stage: ${result.stage || 'debug'}`,
        `mode: ${result.pipeline_mode || 'program'}`,
        `u,v: ${debug.u}, ${debug.v}`,
        `t1: ${_formatDebugComplex(param.t1)}`,
        `t2: ${_formatDebugComplex(param.t2)}`,
        `p1: ${_formatDebugComplex(param.p1)}`,
        `p2: ${_formatDebugComplex(param.p2)}`,
        `param tokens: ${result.param_program?.token_count || param.param_program_tokens || 0} · stack max: ${result.param_program?.stack_max || 0}`,
        `coeff tokens: ${result.coeff_program?.token_count || coeff.coeff_program_tokens || 0} · stack max: ${result.coeff_program?.stack_max || coeff.coeff_program_stack_max || 0} · final stack: 0`,
        `coeffs: cf_len=${coeff.cf_len || 0} · poly_len=${coeff.poly_len || 0} · degree=${coeff.degree || 0}`,
    ];
    const diagnostics = result.coeff_program && Array.isArray(result.coeff_program.diagnostics)
        ? result.coeff_program.diagnostics
        : [];
    diagnostics.forEach(d => lines.push(`diagnostic: ${d.level || 'info'} · ${d.message || ''}`));
    if (result.solve) {
        lines.push(`solve: ${JSON.stringify(result.solve)}`);
    }
    if (Array.isArray(result.roots)) {
        lines.push('');
        lines.push(..._formatDebugComplexRows('root', result.roots, 80));
    } else if (result.stage === 'param') {
        lines.push('');
        lines.push('Poly was evaluated to keep the native path identical; use Poly eval to inspect coefficients.');
    } else {
        lines.push('');
        lines.push(..._formatDebugComplexRows('cf', coeff.cf, 60));
        lines.push('');
        lines.push(..._formatDebugComplexRows('poly', coeff.poly, 80));
    }
    return lines.join('\n');
}

async function runComputeDebug(stage) {
    const buttons = Array.from(document.querySelectorAll('.compute-debug-action'));
    const statusEl = _computeDebugStatusEl();
    const outputEl = _computeDebugOutputEl();
    const funcName = document.getElementById('render-function').value;
    const u = Number(document.getElementById('compute-debug-u')?.value);
    const v = Number(document.getElementById('compute-debug-v')?.value);
    if (!Number.isFinite(u) || !Number.isFinite(v) || u < 0 || u > 1 || v < 0 || v > 1) {
        if (statusEl) {
            statusEl.textContent = 'u and v must be numeric values in [0,1].';
            statusEl.className = 'status error';
        }
        return;
    }
    const nPreview = Math.max(8, parseInt(document.getElementById('compute-preview-n')?.value) || 256);
    const paramTransforms = _effectiveParamTransformsForCompute();
    const paramProgramChain = _effectiveParamProgramChainForCompute();
    const coeffTransforms = _effectiveCoeffTransformsForCompute();
    const coeffProgramChain = _effectiveCoeffProgramChainForCompute();
    const coeffProgramSourceText = _effectiveCoeffProgramSourceTextForCompute() || '';
    const cfpv = _cfpv.length > 0 ? [..._cfpv] : [];
    const label = stage === 'solve_ae' ? 'SolveAE' : (stage === 'solve_cm' ? 'SolveCM' : (stage === 'poly' ? 'Poly' : 'Param'));
    buttons.forEach(btn => { btn.disabled = true; });
    if (statusEl) {
        statusEl.textContent = `${label} debug running...`;
        statusEl.className = 'status';
    }
    if (outputEl) outputEl.textContent = 'Working...';
    try {
        const result = await lambdaPost('compute-preview', _attachProgramSourcePayload({
            debug_stage: stage,
            N_preview: nPreview,
            function: funcName,
            u,
            v,
            param_transforms: paramTransforms,
            param_program_chain: paramProgramChain,
            coeff_transforms: coeffTransforms,
            coeff_program_chain: coeffProgramChain,
            cfpv,
        }));
        if (outputEl) outputEl.textContent = _formatComputeDebugResult(result);
        if (statusEl) {
            statusEl.textContent = `${label} debug ready.`;
            statusEl.className = 'status ok';
        }
        log(`Compute debug ${label}: u=${u}, v=${v}, degree=${result.debug?.coeff?.degree ?? '?'}`, 'ok', 'compute-log');
    } catch (e) {
        if (statusEl) {
            statusEl.textContent = `${label} debug failed: ${e.message}`;
            statusEl.className = 'status error';
        }
        if (outputEl) outputEl.textContent = e.message;
        log(`Compute debug ${label} failed: ${e.message}`, 'err', 'compute-log');
    } finally {
        buttons.forEach(btn => { btn.disabled = false; });
    }
}

async function runComputePreview() {
    const btn = document.getElementById('btn-compute-preview');
    const statusEl = _computePreviewStatusEl();
    const funcName = document.getElementById('render-function').value;
    let nPreview = Math.max(8, parseInt(document.getElementById('compute-preview-n').value) || 256);
    const previewSize = Math.max(64, parseInt(document.getElementById('compute-preview-size').value) || 1000);
    const solverMode = document.getElementById('compute-preview-solver').value || 'aberth_mt';
    const solverIters = Math.max(0, Math.min(64, parseInt(document.getElementById('compute-preview-iters')?.value, 10) || 0));
    const quantile = Math.max(0, parseFloat(document.getElementById('compute-preview-quantile').value) || 0) / 100;
    const shim = Math.max(0, parseFloat(document.getElementById('compute-preview-shim').value) || 5) / 100;
    const paramTransforms = _effectiveParamTransformsForCompute();
    const paramProgramChain = _effectiveParamProgramChainForCompute();
    const coeffTransforms = _effectiveCoeffTransformsForCompute();
    const coeffProgramChain = _effectiveCoeffProgramChainForCompute();
    const cfpv = _cfpv.length > 0 ? [..._cfpv] : [];
    const ptDisplay = _displayActiveParamPipeline(',');
    const ctDisplay = _displayActiveCoeffPipeline(',');
    const rootsCmSyncMaxN = 128;
    if ((_chainHasTransformName(coeffTransforms, 'roots_cm') || _chainHasTransformName(coeffTransforms, 'roots_jt')) && nPreview > rootsCmSyncMaxN) {
        const requestedN = nPreview;
        nPreview = rootsCmSyncMaxN;
        const nInput = document.getElementById('compute-preview-n');
        if (nInput) nInput.value = String(nPreview);
        log(`Compute preview: roots_cm is expensive; reduced N-preview from ${requestedN} to ${nPreview} to stay inside the synchronous preview timeout.`, 'warn', 'compute-log');
    }

    const explicitBounds = _computePreviewExplicitBounds();
    if (_computePreviewViewportMode === 'marquee' && !explicitBounds) {
        if (statusEl) {
            statusEl.textContent = 'Marquee viewport: drag a rectangle on the preview image first (run one Q-shim preview to get an image).';
            statusEl.className = 'status error';
        }
        return;
    }
    if (_computePreviewViewportMode === 'square' && !explicitBounds) {
        if (statusEl) {
            statusEl.textContent = 'Square viewport: enter a positive side length.';
            statusEl.className = 'status error';
        }
        return;
    }
    btn.disabled = true;
    if (statusEl) {
        statusEl.textContent = `Previewing ${funcName} with ${_solverTag(solverMode)}...`;
        statusEl.className = 'status';
    }
    _setComputePreviewPlaceholder('Working...');
    try {
        const cfpvDisplay = _formatCfpvForDisplay(funcName, cfpv);
        log(`Compute preview (${_solverTag(solverMode)}): [${ptDisplay || 'none'}] ${funcName}${cfpvDisplay ? '(' + cfpvDisplay + ')' : ''} [${ctDisplay || 'none'}] N-preview=${nPreview} · pix=${previewSize} · q=${(quantile * 100).toFixed(1)}% · shim=${(shim * 100).toFixed(1)}%...`, '', 'compute-log');
        const viewportPayload = explicitBounds
            ? { viewport_mode: 'explicit',
                view_min_re: explicitBounds.min_re, view_max_re: explicitBounds.max_re,
                view_min_im: explicitBounds.min_im, view_max_im: explicitBounds.max_im }
            : { viewport_mode: 'quantile' };
        const result = await lambdaPost('compute-preview', _attachProgramSourcePayload({
            ...viewportPayload,
            solver_mode: solverMode,
            solver_iters: (solverMode === 'aberth_mt' || solverMode === 'newton') ? solverIters : 0,
            N_preview: nPreview,
            preview_size: previewSize,
            quantile,
            shim,
            function: funcName,
            param_transforms: paramTransforms,
            param_program_chain: paramProgramChain,
            coeff_transforms: coeffTransforms,
            coeff_program_chain: coeffProgramChain,
            cfpv,
        }));
        _applyComputePreviewResult(result);
        log(`  preview (${_solverTag(solverMode)}): ${result.total_ms}ms, degree=${result.degree}, roots=${result.n_roots_in_view}/${result.n_roots_total}`, 'ok', 'compute-log');
    } catch (e) {
        if (statusEl) {
            statusEl.textContent = 'Compute preview failed: ' + e.message;
            statusEl.className = 'status error';
        }
        _setComputePreviewPlaceholder('Preview failed');
        _renderComputePreviewInfo([], _computePreviewIsStale);
        log(`Compute preview failed: ${e.message}`, 'err', 'compute-log');
    } finally {
        btn.disabled = false;
    }
}

function _populateComputeFromDetail(jobId, detail) {
    const calc = detail.calc || {};
    const pipeline = detail.pipeline || calc.pipeline || {};
    const solverMode = _normalizeComputeSolverMode(calc.solver || detail.solver || 'aberth_mt');
    const solverPrefs = { ..._computePopupPrefsForSolver(solverMode) };

    const nVal = calc.N || calc.n1;
    if (nVal != null) document.getElementById('render-n').value = String(nVal);
    const chunksVal = calc.n_chunks || calc.n_stripes;
    if (chunksVal != null) document.getElementById('render-stripes').value = String(chunksVal);
    document.getElementById('render-times').value = String(detail.times || calc.times || 1);
    if (chunksVal != null) solverPrefs.nChunks = Math.max(1, parseInt(chunksVal, 10) || solverPrefs.nChunks || 10);
    solverPrefs.fused = true;
    solverPrefs.fusedThreads = _clampRenderMtThreads(calc.fused_threads || solverPrefs.fusedThreads || 4);
    solverPrefs.solverIters = _clampSolverIters(calc.solver_iters);
    solverPrefs.loresParamGenThreads = _clampRenderMtThreads(calc.lores_param_gen_threads || solverPrefs.loresParamGenThreads || 1);
    solverPrefs.loresCoeffgenThreads = _clampRenderMtThreads(calc.lores_coeffgen_threads || solverPrefs.loresCoeffgenThreads || 1);
    if (calc.probe_degree && calc.probe_n_coeffs) {
        solverPrefs.probe = {
            degree: Number(calc.probe_degree) || 0,
            n_coeffs: Number(calc.probe_n_coeffs) || 0,
            probe_signature: calc.probe_signature || '',
            fused_estimate: calc.min_safe_chunks ? {
                min_safe_chunks: Number(calc.min_safe_chunks) || 0,
                safe_chunk_limit_reason: calc.safe_chunk_limit_reason || '',
                params_bytes: 0,
                coeff_bytes: 0,
                roots_bytes: 0,
                estimated_peak_bytes: 0,
                estimated_tmp_peak_bytes: 0,
            } : null,
        };
        solverPrefs.probeError = '';
    }
    _computePopupPrefsBySolver[solverMode] = solverPrefs;
    if ((_computeMtPopupState.solverMode || '') === solverMode) {
        _computeMtPopupState = { ..._computeMtPopupState, ...solverPrefs };
    }

    const functionName = pipeline.function || calc.function || '';
    if (functionName) {
        _setRenderFunction(functionName);
    }

    const savedParamProgramChain = pipeline.param_program_chain || detail.param_program_chain || [];
    let savedParamProgramSourceText = [
        pipeline.param_program_source_text,
        detail.param_program_source_text,
        calc.param_program_source_text,
    ].map(v => typeof v === 'string' ? v : '').find(v => v.trim()) || '';
    const savedCoeffProgramChain = pipeline.coeff_program_chain || detail.coeff_program_chain || [];
    let savedCoeffProgramSourceText = [
        pipeline.coeff_program_source_text,
        detail.coeff_program_source_text,
        calc.coeff_program_source_text,
    ].map(v => typeof v === 'string' ? v : '').find(v => v.trim()) || '';
    if (!savedParamProgramSourceText.trim() && Array.isArray(savedParamProgramChain) && savedParamProgramChain.length &&
        typeof _paramProgramSourceFromRows === 'function') {
        savedParamProgramSourceText = _paramProgramSourceFromRows(savedParamProgramChain);
    }
    if (!savedCoeffProgramSourceText.trim() && Array.isArray(savedCoeffProgramChain) && savedCoeffProgramChain.length &&
        typeof _coeffProgramSourceFromRows === 'function') {
        savedCoeffProgramSourceText = _coeffProgramSourceFromRows(savedCoeffProgramChain);
    }
    _setChainFromSaved('pp', savedParamProgramChain);
    _setParamProgramSourceText(savedParamProgramSourceText);
    _setParamProgramEditorMode('text');
    _setChainFromSaved('cp', savedCoeffProgramChain);
    _setCoeffProgramSourceText(savedCoeffProgramSourceText);
    _setCoeffProgramEditorMode('text');
    _setParamPipelineMode('program');

    const savedCfpv = Array.isArray(pipeline.cfpv) ? pipeline.cfpv : [];
    if (savedCfpv.length) {
        if (_isConstCoeffFunction(functionName)) {
            _setConstCoeffInputsFromRaw(savedCfpv);
        } else {
            for (let i = 0; i < savedCfpv.length; i++) {
                const inp = document.getElementById('cfpv-p' + i);
                if (inp) inp.value = String(savedCfpv[i]);
            }
        }
        parseCfpv();
    } else {
        _cfpv = [];
    }

    document.getElementById('results-dir').value = jobId;
    const solver = solverMode || '';
    const solverLabel = _solverRunLabel(solver);
    const statusEl = document.getElementById('compute-status');
    statusEl.textContent = `Populated from ${jobId}. Re-run with ${solverLabel}.`;
    statusEl.className = 'status ok';
    log(`Populate: restored compute settings from ${jobId}${solver ? ' (' + solver + ')' : ''}`, 'ok', 'compute-log');
    switchTab('compute');
}

async function populateSelectedResult() {
    if (!_selectedJobId) return;
    const jobId = _selectedJobId;
    const infoEl = document.getElementById('results-info');
    const btn = document.getElementById('btn-populate-result');
    const orig = btn ? btn.textContent : 'Populate';
    if (btn) { btn.disabled = true; btn.textContent = '...'; }
    try {
        infoEl.textContent = 'Loading details...';
        const detail = await _getResultDetail(jobId);
        _populateComputeFromDetail(jobId, detail || {});
    } catch (e) {
        infoEl.textContent = 'Populate error: ' + e.message;
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

/* ---- Lambda config ---- */
const APP_TITLE_BASE = 'PolyPaint Lambda 1.0';
let _lambdaUrls = {};  // {sweep, coeffgen, raster, finalize, encode, viewport, storage, dispatch}
let _buildInfo = {};

function _setConfigPopupOpen(open) {
    const popup = document.getElementById('config-popup');
    const btn = document.getElementById('btn-config-toggle');
    if (!popup) return;
    popup._open = !!open;
    popup.style.display = open ? 'block' : 'none';
    popup.classList.toggle('active', !!open);
    popup.setAttribute('aria-hidden', open ? 'false' : 'true');
    if (btn) btn.setAttribute('aria-expanded', open ? 'true' : 'false');
}

function _toggleConfigPopup(forceOpen) {
    const popup = document.getElementById('config-popup');
    if (!popup) return;
    const next = typeof forceOpen === 'boolean' ? forceOpen : !popup._open;
    _setConfigPopupOpen(next);
    if (next) void visionConfigLoad();     // the LLM section shows live state on open
}

// ── LLM (vision model) config — the Config popup's second section ──────────
// Shared by Book Describe and Gallery Describe (DDB __config__/vision_model
// via /fetch-vision-config + /save-vision-config; the key is never echoed).
function visionConfigModelChanged() {
    const sel = document.getElementById('vision-model');
    const custom = document.getElementById('vision-model-custom');
    if (custom) custom.style.display = sel && sel.value === '__custom__' ? 'block' : 'none';
}

function _visionConfigSelectedModel() {
    const sel = document.getElementById('vision-model');
    if (!sel) return '';
    if (sel.value === '__custom__') {
        return (document.getElementById('vision-model-custom')?.value || '').trim();
    }
    return sel.value;
}

function _visionConfigShowModel(model) {
    const sel = document.getElementById('vision-model');
    const custom = document.getElementById('vision-model-custom');
    if (!sel) return;
    const known = Array.from(sel.options).some(o => o.value === model);
    if (model && !known) {
        sel.value = '__custom__';
        if (custom) { custom.value = model; custom.style.display = 'block'; }
    } else if (model) {
        sel.value = model;
        if (custom) custom.style.display = 'none';
    }
}

function _visionConfigStatusText(cfg) {
    const provs = cfg.providers || {};
    const marks = ['gemini', 'anthropic', 'openai']
        .map(p => `${p} ${provs[p]?.key_set ? '✓' + (provs[p].key_hint || '') : '—'}`)
        .join(' · ');
    return `model ${cfg.model || 'gemini-2.5-flash (default)'} · keys: ${marks}`;
}

async function visionConfigLoad() {
    const status = document.getElementById('vision-status');
    try {
        const cfg = await lambdaPost('storage', {}, '/fetch-vision-config');
        _visionConfigShowModel(cfg.model || '');
        if (status) status.textContent = _visionConfigStatusText(cfg);
    } catch (e) {
        if (status) status.textContent = e.message;
    }
}

async function visionConfigSave(btn) {
    const orig = btn ? btn.textContent : 'Save';
    if (btn) { btn.disabled = true; btn.textContent = 'Saving…'; }
    const status = document.getElementById('vision-status');
    try {
        const model = _visionConfigSelectedModel();
        if (!model) throw new Error('pick or type a model id');
        const keyEl = document.getElementById('vision-key');
        const cfg = await lambdaPost('storage', { model, api_key: (keyEl?.value || '').trim() }, '/save-vision-config');
        if (keyEl) keyEl.value = '';
        if (status) status.textContent = 'saved · ' + _visionConfigStatusText(cfg);
        if (btn) btn.textContent = 'Saved ✓';
    } catch (e) {
        if (status) status.textContent = 'save failed: ' + e.message;
        if (btn) btn.textContent = 'Failed';
    } finally {
        setTimeout(() => { if (btn) { btn.disabled = false; btn.textContent = orig; } }, 1500);
    }
}

function _applyBuildInfo(build) {
    _buildInfo = (build && typeof build === 'object') ? build : {};
    const buildId = typeof _buildInfo.build_id === 'string' ? _buildInfo.build_id.trim() : '';
    const labelEl = document.getElementById('build-id-label');
    const metaEl = document.getElementById('config-build-meta');
    if (labelEl) {
        labelEl.textContent = buildId ? `· build ${buildId}` : '';
        if (buildId) {
            const tooltip = [
                `build: ${buildId}`,
                _buildInfo.deployed_at_utc ? `deployed: ${_buildInfo.deployed_at_utc}` : '',
                _buildInfo.git_rev ? `git: ${_buildInfo.git_rev}${_buildInfo.git_dirty ? ' (dirty)' : ''}` : '',
                _buildInfo.frontend_sha256 ? `frontend sha256: ${_buildInfo.frontend_sha256}` : '',
            ].filter(Boolean).join('\n');
            labelEl.title = tooltip;
        } else {
            labelEl.title = '';
        }
    }
    if (metaEl) {
        const parts = [];
        if (buildId) parts.push(`build: ${buildId}`);
        if (_buildInfo.deployed_at_utc) parts.push(`deployed: ${_buildInfo.deployed_at_utc}`);
        if (_buildInfo.git_rev) parts.push(`git: ${_buildInfo.git_rev}${_buildInfo.git_dirty ? ' (dirty)' : ''}`);
        if (_buildInfo.frontend_sha256) parts.push(`frontend sha256: ${_buildInfo.frontend_sha256}`);
        metaEl.innerHTML = parts.length
            ? parts.map(line => `<div>${line}</div>`).join('')
            : '<div>Build metadata unavailable.</div>';
    }
    document.title = buildId ? `${APP_TITLE_BASE} · build ${buildId}` : APP_TITLE_BASE;
}

async function loadLambdaConfig() {
    const url = document.getElementById('config-url').value.trim();
    const statusEl = document.getElementById('config-status');
    try {
        statusEl.textContent = 'Loading...';
        const resp = await fetch(url);
        if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
        const raw = await resp.json();
        _lambdaUrls = (raw && typeof raw.services === 'object' && raw.services)
            ? raw.services
            : Object.fromEntries(Object.entries(raw || {}).filter(([, value]) => typeof value === 'string'));
        _applyBuildInfo(raw && raw.build ? raw.build : null);
        const count = Object.keys(_lambdaUrls).length;
        statusEl.textContent = `${count} endpoints loaded`;
        statusEl.style.color = '#4ecca3';
        // One-time favorites index for the Render-tab Favorite button. Not
        // forced: use stored snapshots (0 S3) rather than re-resolving on boot.
        try { await _loadFavoriteRefs(); } catch (e) {}
    } catch (e) {
        statusEl.textContent = 'Error: ' + e.message;
        statusEl.style.color = '#ff5555';
    }
}

function lambdaUrl(name, path) {
    const base = (_lambdaUrls[name] || '').replace(/\/$/, '');
    if (!base) throw new Error(`Lambda URL not configured for "${name}". Load config.json first.`);
    return base + (path || '');
}

async function _ensureLambdaServiceConfigured(name) {
    if ((_lambdaUrls[name] || '').trim()) return;
    await loadLambdaConfig();
    if ((_lambdaUrls[name] || '').trim()) return;
    const loaded = Object.keys(_lambdaUrls || {}).sort();
    const loadedText = loaded.length ? ` loaded services=${loaded.join(',')}` : ' loaded services=none';
    const buildText = _buildInfo && _buildInfo.build_id ? ` build=${_buildInfo.build_id}` : '';
    throw new Error(`Lambda URL not configured for "${name}" after reloading config.json.${buildText}${loadedText}`);
}

function _clipErrorText(value, maxLen = 240) {
    const s = value == null ? '' : String(value).replace(/\s+/g, ' ').trim();
    if (!s) return '';
    return s.length > maxLen ? s.slice(0, maxLen - 3) + '...' : s;
}

function _formatErrorChain(chain) {
    if (!Array.isArray(chain) || !chain.length) return '';
    return chain.map(item => {
        if (Array.isArray(item)) {
            const name = item[0] == null ? '' : String(item[0]);
            const args = item.slice(1).map(v => String(v)).join(',');
            return args ? `${name}(${args})` : name;
        }
        if (item && typeof item === 'object' && item.name) {
            const params = Array.isArray(item.params) ? item.params.map(v => String(v)).join(',') : '';
            return params ? `${item.name}(${params})` : String(item.name);
        }
        return String(item);
    }).join(',');
}

function _parseLambdaErrorPayload(text) {
    if (!text) return null;
    try {
        const parsed = JSON.parse(text);
        if (parsed && typeof parsed === 'object' && typeof parsed.body === 'string') {
            try { return JSON.parse(parsed.body); } catch (e) { return parsed; }
        }
        return parsed;
    } catch (e) {
        return null;
    }
}

function _summarizeLambdaErrorPayload(body) {
    if (!body || typeof body !== 'object') return '';
    const parts = [];
    const push = (label, value, maxLen = 220) => {
        if (value == null || value === '') return;
        parts.push(`${label}=${_clipErrorText(value, maxLen)}`);
    };
    if (body.error) parts.push(_clipErrorText(body.error, 180));
    if (body.message) parts.push(_clipErrorText(body.message, 260));
    push('phase', body.phase, 80);
    push('metric', body.metric, 80);
    push('program', body.program, 200);
    if (Array.isArray(body.score_metrics) && body.score_metrics.length) {
        push('score_metrics', body.score_metrics.join(','), 180);
    }
    const chain = _formatErrorChain(body.solve_score_chain || body.chain);
    if (chain) push('chain', chain, 220);
    push('detail', body.detail, 320);
    push('compile_error', body.compile_error, 220);
    return parts.join(' | ');
}

function _summarizeLambdaBody(body) {
    if (body == null) return '';
    if (typeof body !== 'object') return _clipErrorText(body, 160);

    const parts = [];
    const push = (label, value) => {
        if (value == null || value === '') return;
        parts.push(`${label}=${_clipErrorText(value, 160)}`);
    };

    if (Array.isArray(body.jobs)) {
        push('target', body.target);
        push('jobs', body.jobs.length);
        if (body.jobs.length === 1 && body.jobs[0] && typeof body.jobs[0] === 'object') {
            const first = _summarizeLambdaBody(body.jobs[0]);
            if (first) parts.push(first);
        }
        return parts.join(', ');
    }

    push('phase', body.phase);
    push('job', body.job_id);
    push('task', body.task_id);
    push('task_prefix', body.task_prefix);
    if (body.section_idx != null) push('section', body.section_idx);
    else if (body.chunk_idx != null) push('chunk', body.chunk_idx);
    if (body.section_idx == null && body.chunk_idx == null && body.stripe_idx != null) push('section', body.stripe_idx);
    push('function', body.function);
    push('metric', body.metric);
    push('solver', body.solver);
    push('mode', body.mode);
    push('s3_key', body.s3_key);
    push('params_key', body.params_key);
    push('coeffs_key', body.coeffs_key);
    push('lores_bin_key', body.lores_bin_key);
    const coeffChain = _formatErrorChain(body.coeff_transforms);
    const paramChain = _formatErrorChain(body.param_transforms);
    const solveScoreChain = _formatErrorChain(body.solve_score_chain);
    if (coeffChain) push('coeff', coeffChain);
    if (paramChain) push('param', paramChain);
    if (solveScoreChain) push('score', solveScoreChain);
    return parts.join(', ');
}

function _formatLambdaRetryAttempt(kind, attempt, detail) {
    const suffix = _clipErrorText(detail, 220);
    return `attempt ${attempt}: ${kind}${suffix ? ` ${suffix}` : ''}`;
}

// Endpoints that mutate server state. A *network-level* failure against one of
// these is AMBIGUOUS — the request may already have been delivered and applied
// (a dispatch fan-out fired, a book saved) even though we never saw the
// response. Silently retrying would double-apply the side effect, so we do NOT
// retry mutations on ambiguous failures. An explicit HTTP 429/503 is different:
// the server told us it rejected the request without processing it, so retrying
// is safe even for a mutation. Callers can override via opts.idempotent.
function _lambdaEndpointIsMutation(name, path) {
    if (name === 'dispatch') return true;               // fan-out invoke
    const p = String(path || '');
    // save-*, delete-*, cleanup, migrate-*, create-* (non-idempotent: each call
    // makes a NEW gallery/share), add-to-* (a retried add is misreported as a
    // duplicate). A retry on an ambiguous network failure would double-apply.
    return /\/(save|delete|cleanup|migrate|create|add-to|describe)/i.test(p);
}

// Helper: POST JSON to a Lambda and parse response.
// opts: { idempotent?: boolean, timeoutMs?: number }
//   idempotent — override the auto-classification (true = safe to retry on an
//     ambiguous network failure; false = do not). Defaults to !isMutation.
//   timeoutMs — per-attempt abort deadline for a hung connection (default 120s).
async function lambdaPost(name, body, path, opts) {
    opts = opts || {};
    await _ensureLambdaServiceConfigured(name);
    const url = lambdaUrl(name, path);
    const endpoint = `${name}${path || ''}`;
    const isMutation = _lambdaEndpointIsMutation(name, path);
    const idempotent = opts.idempotent != null ? !!opts.idempotent : !isMutation;
    const timeoutMs = opts.timeoutMs != null ? opts.timeoutMs : 120000;
    const requestCtx = _summarizeLambdaBody(body);
    const attempts = [];
    for (let attempt = 0; attempt < 5; attempt++) {
        let resp;
        // Per-attempt timeout so a hung connection can't stall forever. An
        // abort surfaces as a network-level failure and follows the same
        // ambiguity rules below.
        const controller = (typeof AbortController !== 'undefined') ? new AbortController() : null;
        const timer = controller ? setTimeout(() => controller.abort(), timeoutMs) : null;
        const fetchOpts = {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(body),
        };
        if (controller) fetchOpts.signal = controller.signal;
        try {
            resp = await fetch(url, fetchOpts);
        } catch (e) {
            const aborted = e && e.name === 'AbortError';
            const kind = aborted ? `timeout>${timeoutMs}ms` : 'network';
            attempts.push(_formatLambdaRetryAttempt(kind, attempt + 1, e && e.message ? e.message : e));
            // Ambiguous failure: the request may already have been applied. Only
            // retry if the operation is idempotent (a read, or an explicitly
            // safe mutation). For a non-idempotent mutation, stop and surface a
            // clear error so the caller can decide, rather than double-firing.
            if (!idempotent) {
                const ctxSuffix = requestCtx ? ` (${requestCtx})` : '';
                throw new Error(`${endpoint} ${aborted ? 'timed out' : 'network failure'} after ` +
                    `${attempt + 1} attempt(s); not retried (non-idempotent mutation — ` +
                    `may or may not have been applied)${ctxSuffix}`);
            }
            if (attempt < 4) {
                const delay = 1000 * Math.pow(2, attempt) + Math.random() * 1000;
                await new Promise(r => setTimeout(r, delay));
                continue;
            }
            break;
        } finally {
            if (timer) clearTimeout(timer);
        }
        if (resp.status === 503 || resp.status === 429) {
            // Explicit server rejection — the request was NOT processed, so
            // retrying is safe even for a mutation.
            const retryText = await resp.text().catch(() => '');
            attempts.push(_formatLambdaRetryAttempt(`HTTP ${resp.status}`, attempt + 1, retryText));
            if (attempt < 4) {
                const delay = 1000 * Math.pow(2, attempt) + Math.random() * 1000;
                await new Promise(r => setTimeout(r, delay));
                continue;
            }
            break;
        }
        if (!resp.ok) {
            const text = await resp.text().catch(() => '');
            const ctxSuffix = requestCtx ? ` (${requestCtx})` : '';
            const errorPayload = _parseLambdaErrorPayload(text);
            const errorDetail = errorPayload
                ? _summarizeLambdaErrorPayload(errorPayload)
                : _clipErrorText(text, 320);
            throw new Error(`${endpoint} request failed${ctxSuffix}: HTTP ${resp.status}${errorDetail ? `: ${errorDetail}` : ''}`);
        }
        const raw = await resp.json();
        return typeof raw.body === 'string' ? JSON.parse(raw.body) : raw;
    }
    const ctxSuffix = requestCtx ? ` (${requestCtx})` : '';
    const attemptSuffix = attempts.length ? `: ${attempts.join(' ; ')}` : '';
    throw new Error(`${endpoint} request failed after 5 retries${ctxSuffix}${attemptSuffix}`);
}

function _escapeHtml(value) {
    const s = value == null ? '' : String(value);
    return s
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
}

function _catalogForChain(which) {
    if (which === 'pp') return _ppCatalog;
    if (which === 'cp') return _coeffProgramCatalog;
    if (which === 'ss' || which === 'palette-ss') return _ssCatalog;
    return {};
}

// `which` codes used across the chip-editor helpers:
//   pt = param transforms (legacy), pp = param program, ct = coeff
//   transforms (legacy), cp = coeff program, ss = solve score,
//   palette-ss = palette solve score. Root transforms are text-only
//   (rt/prt source editors); they have no chip chain.
function _chainForWhich(which) {
    if (which === 'pp') return _ppChain;
    if (which === 'cp') return _coeffProgramChain;
    if (which === 'ss') return _renderScoreChain;
    if (which === 'palette-ss') return _paletteScoreChain;
    return [];
}

function _displayTransformEntry(item) {
    if (!item || !item.name) return '';
    return item.params && item.params.length ? `${item.name}(${item.params.join(',')})` : item.name;
}

function _isSolveScoreGenericMetricChipName(name) {
    return name === _solveScoreGenericMetricChipName || name === _solveScoreGenericMetricPublicName;
}

function _displaySolveScoreEntry(item) {
    if (!item || !item.name) return '';
    const params = Array.isArray(item.params) ? item.params : [];
    if (_isSolveScoreGenericMetricChipName(item.name)) {
        const metric = params[0] || '?';
        const source = params[1] || 'slv';
        const q = params[2] || '?';
        return `metric(${metric},${source},q=${q}%)`;
    }
    if (_solveScoreMetricSet.has(item.name)) {
        const hasExplicitSource = params.length >= 2;
        const sourceInfo = _splitSolveScoreMetricSourceLag(hasExplicitSource ? params[0] : 'slv', 'slv');
        const source = sourceInfo.wire;
        const q = hasExplicitSource ? (params[1] || '?') : (params[0] || '?');
        return source === 'slv'
            ? `${item.name}(q=${q}%)`
            : `${item.name}(${source},q=${q}%)`;
    }
    if (item.name === 'omega_cosine') {
        const phase = Number(params[1] || '0');
        return Number.isFinite(phase) && Math.abs(phase) < 1e-12
            ? `ω-cos(${params[0] || '?'})`
            : `ω-cos(${params[0] || '?'},${params[1] || '0'})`;
    }
    if (item.name === 'sawtooth') {
        return `sawtooth(${params[0] || '?'})`;
    }
    if (item.name === 'weighted_sum') {
        return `weighted_sum(${params[0] || '?'},${params[1] || '?'})`;
    }
    if (_solveScoreOutputSpecs && _solveScoreOutputSpecs[item.name]) {
        const mode = item.name === 'emit_norm' ? 'norm' : (item.name === 'emit_none' ? 'none' : (params[0] || 'raw'));
        return `emit(${mode})`;
    }
    return _displayTransformEntry(item);
}

function _solveScorePrefixForWhich(which) {
    return which === 'palette-ss' ? 'palette' : 'render';
}

function _solveScoreWhichForPrefix(prefix) {
    return prefix === 'palette' ? 'palette-ss' : 'ss';
}

function _solveScoreQuantilePctText(value, fallback = '0.1') {
    const q = Number(value);
    if (!Number.isFinite(q)) return String(fallback);
    return q.toFixed(1).replace(/\.0$/, '');
}

function _boolish(value, fallback = false) {
    if (value == null || value === '') return !!fallback;
    if (typeof value === 'boolean') return value;
    if (typeof value === 'number') return value !== 0;
    const text = String(value).trim().toLowerCase();
    if (['1', 'true', 'yes', 'on'].includes(text)) return true;
    if (['0', 'false', 'no', 'off'].includes(text)) return false;
    return !!fallback;
}

function _normalizeRenderBackgroundColor(value, fallback = '', options = {}) {
    let text = _str(value).trim().toLowerCase();
    const allowShort = options.allowShort !== false;
    if (text.startsWith('#')) text = text.slice(1);
    if (allowShort && /^[0-9a-f]{3}$/.test(text)) text = text.split('').map(ch => ch + ch).join('');
    if (/^[0-9a-f]{6}$/.test(text)) return text;
    return fallback;
}

function _syncRenderBackgroundColorUi(message = '') {
    const normalized = _normalizeRenderBackgroundColor(renderBackgroundColor, DEFAULT_RENDER_BACKGROUND_COLOR);
    renderBackgroundColor = normalized;
    const colorEl = document.getElementById('render-background-color');
    const hexEl = document.getElementById('render-background-hex');
    const statusEl = document.getElementById('render-background-status');
    const eyeBtn = document.getElementById('render-background-eyedropper');
    if (colorEl) colorEl.value = '#' + normalized;
    if (hexEl) hexEl.value = normalized.toUpperCase();
    if (statusEl) statusEl.textContent = message || ('#' + normalized.toUpperCase());
    const supportsEyeDropper = typeof window !== 'undefined' && 'EyeDropper' in window;
    if (eyeBtn && !supportsEyeDropper) {
        eyeBtn.disabled = true;
        eyeBtn.title = 'Screen pipette is not supported by this browser';
    }
}

function _setRenderBackgroundColor(value, options = {}) {
    const normalized = _normalizeRenderBackgroundColor(value, '', { allowShort: !options.fromText });
    const statusEl = document.getElementById('render-background-status');
    if (!normalized) {
        if (statusEl) statusEl.textContent = 'Use 6-digit hex';
        if (!options.fromText) _syncRenderBackgroundColorUi();
        return false;
    }
    renderBackgroundColor = normalized;
    _syncRenderBackgroundColorUi();
    return true;
}

function _commitRenderBackgroundHex() {
    const hexEl = document.getElementById('render-background-hex');
    if (!_setRenderBackgroundColor(hexEl ? hexEl.value : renderBackgroundColor)) {
        _syncRenderBackgroundColorUi();
    }
}

function _readRenderBackgroundColor({ requireValid = false } = {}) {
    const hexEl = document.getElementById('render-background-hex');
    const value = hexEl ? hexEl.value : renderBackgroundColor;
    const normalized = _normalizeRenderBackgroundColor(value, '');
    if (!normalized) {
        if (requireValid) throw new Error('Background color must be a 6-digit hex color');
        return _normalizeRenderBackgroundColor(renderBackgroundColor, DEFAULT_RENDER_BACKGROUND_COLOR);
    }
    renderBackgroundColor = normalized;
    _syncRenderBackgroundColorUi();
    return normalized;
}

async function _pickRenderBackgroundColor() {
    if (typeof window === 'undefined' || !('EyeDropper' in window)) {
        const statusEl = document.getElementById('render-background-status');
        if (statusEl) statusEl.textContent = 'Pipette unsupported';
        return;
    }
    try {
        const dropper = new EyeDropper();
        const result = await dropper.open();
        if (result && result.sRGBHex) _setRenderBackgroundColor(result.sRGBHex);
    } catch (e) {
        const statusEl = document.getElementById('render-background-status');
        if (statusEl) statusEl.textContent = 'Pipette cancelled';
    }
}

function _normalizeSolveScoreMetricSource(value, fallback = 'slv') {
    const source = _str(value).trim().toLowerCase();
    return (source === 'slv' || source === 'cf' || source === 'pm') ? source : fallback;
}

function _splitSolveScoreMetricSourceLag(value, fallback = 'slv') {
    const raw = _str(value).trim().toLowerCase();
    if (raw === 'slv' || raw === 'cf' || raw === 'pm') return { source: raw, lag: 0, wire: raw };
    if (raw.endsWith('-1')) {
        const base = raw.slice(0, -2);
        if (base === 'slv' || base === 'cf' || base === 'pm') return { source: base, lag: 1, wire: `${base}-1` };
    }
    if (fallback === '') return { source: '', lag: 0, wire: '' };
    const fb = _normalizeSolveScoreMetricSource(fallback, 'slv');
    return { source: fb, lag: 0, wire: fb };
}

function _solveScoreMetricSourceChoices(name) {
    const base = _solveScoreMetricAllowedSources(name);
    return base.concat(base.map(source => `${source}-1`));
}

function _solveScoreMetricsUseSource(metrics, wantedSource) {
    const wanted = _normalizeSolveScoreMetricSource(wantedSource, '');
    return !!wanted && Array.isArray(metrics) && metrics.some(row => _normalizeSolveScoreMetricSource(row && row.source, 'slv') === wanted);
}

function _solveScoreMetricsUseNonSolveSource(metrics) {
    return Array.isArray(metrics) && metrics.some(row => _normalizeSolveScoreMetricSource(row && row.source, 'slv') !== 'slv');
}

function _solveScoreMetricAllowedSources(name) {
    const fromVocab = _solveScoreAllowedSourcesByMetric && _solveScoreAllowedSourcesByMetric[name];
    if (Array.isArray(fromVocab) && fromVocab.length) return fromVocab.slice();
    if (_solveScoreParamMetricSet.has(name)) return ['pm'];
    if (_solveScoreParamCapableMetricSet.has(name)) return ['slv', 'cf', 'pm'];
    return ['slv', 'cf'];
}

function _solveScoreMetricCanUseGenericChip(name) {
    const sources = _solveScoreMetricAllowedSources(name);
    return _solveScoreMetricSet.has(name) && sources.includes('slv') && sources.includes('cf');
}

function _solveScoreItemMetricDetails(item, legacyQuantilePct = '0.1') {
    if (!item || !item.name) return null;
    const params = Array.isArray(item.params) ? item.params : [];
    if (_isSolveScoreGenericMetricChipName(item.name)) {
        const metric = _solveScoreMetricSet.has(params[0]) ? params[0] : (_solveScoreGenericMetricNames[0] || 'proximity');
        if (!_solveScoreMetricCanUseGenericChip(metric)) return null;
        const sourceInfo = _splitSolveScoreMetricSourceLag(params[1], 'slv');
        const source = sourceInfo.source === 'cf' ? 'cf' : 'slv';
        const q = _solveScoreQuantilePctText(params[2], legacyQuantilePct);
        return { metric, source, lag: Number(sourceInfo.lag) || 0, sourceWire: _formatSolveScoreSourceLag(source, Number(sourceInfo.lag) || 0), q };
    }
    if (_solveScoreMetricSet.has(item.name)) {
        const defaultSource = _solveScoreParamMetricSet.has(item.name) ? 'pm' : 'slv';
        const sourceInfo = _splitSolveScoreMetricSourceLag(params.length >= 2 ? params[0] : defaultSource, defaultSource);
        const q = _solveScoreQuantilePctText(params.length >= 2 ? params[1] : params[0], legacyQuantilePct);
        return { metric: item.name, source: sourceInfo.source, lag: Number(sourceInfo.lag) || 0, sourceWire: sourceInfo.wire, q };
    }
    return null;
}

function _formatSolveScoreSourceLag(source, lag) {
    const base = _normalizeSolveScoreMetricSource(source, 'slv');
    return Number(lag) === 1 ? `${base}-1` : base;
}

function _solveScoreMetricSummary(rows) {
    if (!Array.isArray(rows)) return '';
    const showAllSources = _solveScoreMetricsUseNonSolveSource(rows);
    return rows.map((row) => {
        const source = _normalizeSolveScoreMetricSource(row && row.source, 'slv');
        const qPct = (Number(row && row.quantile_pct) || (Number(row && row.quantile) * 100) || 0)
            .toFixed(1)
            .replace(/\.0$/, '');
        return (source === 'slv' && !showAllSources)
            ? `${row.metric}(q=${qPct}%)`
            : `${row.metric}(${source},q=${qPct}%)`;
    }).join(', ');
}

function _fallbackLoresCoeffsKey(jobId, loresBinKey) {
    const key = String(loresBinKey || '').trim();
    if (key.endsWith('/lores.bin')) return key.slice(0, -'/lores.bin'.length) + '/lores_coeffs.bin';
    const job = String(jobId || '').trim();
    return job ? `renders/${job}/lores_coeffs.bin` : '';
}

function _fallbackLoresParamsKey(jobId, loresBinKey) {
    const key = String(loresBinKey || '').trim();
    if (key.endsWith('/lores.bin')) return key.slice(0, -'/lores.bin'.length) + '/lores_params.bin';
    const job = String(jobId || '').trim();
    return job ? `renders/${job}/lores_params.bin` : '';
}

function _defaultSolveScoreChain(metric = 'proximity', quantilePct = '0.1', omega = 1, omegaEnabled = false, phase = 0, source = 'slv') {
    const nextMetric = _solveScoreMetricSet.has(metric) ? metric : 'proximity';
    const allowedSources = _solveScoreMetricAllowedSources(nextMetric);
    const normalizedSource = _normalizeSolveScoreMetricSource(source, allowedSources[0]);
    const chain = [{ name: nextMetric, params: [allowedSources.includes(normalizedSource) ? normalizedSource : allowedSources[0], _solveScoreQuantilePctText(quantilePct)] }];
    if (omegaEnabled) chain.push({ name: 'omega_cosine', params: [String(omega), String(phase)] });
    return chain;
}

function _legacySolveScoreQuantilePct(prefix) {
    const qEl = document.getElementById(`${prefix}-solve-score-quantile`);
    const q = Number(qEl && qEl.value);
    if (Number.isFinite(q)) return q;
    const fallbackMetric = prefix === 'palette' ? paletteTabMetric : renderSolveMetric;
    try {
        const compiled = _compileSolveScoreChain(
            _chainForWhich(_solveScoreWhichForPrefix(prefix)),
            fallbackMetric,
            0.1
        );
        return Number(compiled.quantile) * 100;
    } catch (_) {
        return 0.1;
    }
}

function _solveScoreLegacyFieldsPresent(prefix) {
    return [
        `${prefix}-solve-score`,
        `${prefix}-solve-score-quantile`,
        `${prefix}-solve-score-omega`,
        `${prefix}-solve-score-omega-phase`,
        `${prefix}-solve-score-omega-enabled`,
    ].some((id) => !!document.getElementById(id));
}

function _defaultSolveScoreState(prefix) {
    const fallbackMetric = prefix === 'palette' ? paletteTabMetric : renderSolveMetric;
    const chain = _serializeSolveScoreChain(
        _defaultSolveScoreChain(fallbackMetric || 'proximity'),
        fallbackMetric || 'proximity'
    );
    return _compileSolveScoreChain(chain, fallbackMetric || 'proximity', 0.1);
}

function _normalizeSolveScoreChain(chain, fallbackMetric = 'proximity', legacyQuantilePct = '0.1') {
    const rows = Array.isArray(chain) ? chain : [];
    const normalized = [];
    rows.forEach((item) => {
        let name = '';
        let params = [];
        if (Array.isArray(item) && item.length) {
            name = String(item[0] || '').trim();
            params = item.slice(1).map(v => String(v));
        } else if (typeof item === 'string') {
            name = String(item).trim();
        } else if (item && typeof item === 'object') {
            name = String(item.name || '').trim();
            params = Array.isArray(item.params) ? item.params.map(v => String(v)) : [];
        }
        if (!name) return;
        if (name === _solveScoreGenericMetricPublicName) name = _solveScoreGenericMetricChipName;
        if (name === 'emit_norm') {
            name = 'emit';
            params = ['norm'];
        } else if (name === 'emit_none') {
            name = 'emit';
            params = ['none'];
        } else if (name === 'emit') {
            const mode = String(params[0] || 'raw').trim().toLowerCase();
            params = [(mode === 'norm' || mode === 'normalized' || mode === 'emit_norm') ? 'norm' : ((mode === 'none' || mode === 'skip' || mode === 'off' || mode === 'emit_none' || mode === 'emit-none') ? 'none' : 'raw')];
        }
        if (!_ssCatalog[name]) return;
        if (_isSolveScoreGenericMetricChipName(name)) {
            if (params.length !== 3) {
                throw new Error('Generic metric chip requires metric, source, and q parameters');
            }
            const metric = String(params[0] || '').trim();
            if (!_solveScoreMetricCanUseGenericChip(metric)) {
                throw new Error(`Generic metric chip requires a slv/cf-capable metric, got ${metric || '(empty)'}`);
            }
            const sourceInfo = _splitSolveScoreMetricSourceLag(params[1], '');
            if (sourceInfo.source !== 'slv' && sourceInfo.source !== 'cf') {
                throw new Error('Generic metric chip source must be one of slv, cf, slv-1, cf-1');
            }
            const source = sourceInfo.source;
            const sourceWire = _formatSolveScoreSourceLag(source, sourceInfo.lag);
            params = [
                metric,
                sourceWire,
                _solveScoreQuantilePctText(params[2], legacyQuantilePct),
            ];
        }
        if (_solveScoreMetricSet.has(name)) {
            const defaultSource = _solveScoreParamMetricSet.has(name) ? 'pm' : 'slv';
            if (params.length === 0) {
                params = [defaultSource, _solveScoreQuantilePctText(legacyQuantilePct)];
            } else if (params.length === 1) {
                const sourceInfo = _splitSolveScoreMetricSourceLag(params[0], '');
                const maybeSource = sourceInfo.source;
                const allowedSources = _solveScoreMetricAllowedSources(name);
                params = maybeSource
                    ? [allowedSources.includes(maybeSource) ? sourceInfo.wire : defaultSource, _solveScoreQuantilePctText(legacyQuantilePct)]
                    : [defaultSource, _solveScoreQuantilePctText(params[0], legacyQuantilePct)];
            } else {
                params = [
                    (() => {
                        const sourceInfo = _splitSolveScoreMetricSourceLag(params[0], defaultSource);
                        const allowedSources = _solveScoreMetricAllowedSources(name);
                        return allowedSources.includes(sourceInfo.source) ? sourceInfo.wire : defaultSource;
                    })(),
                    _solveScoreQuantilePctText(params[1], legacyQuantilePct),
                ];
            }
        }
        normalized.push({ name, params });
    });
    return normalized;
}

function _serializeSolveScoreChain(chain, fallbackMetric = 'proximity', legacyQuantilePct = '0.1') {
    return _normalizeSolveScoreChain(chain, fallbackMetric, legacyQuantilePct).map((item) => {
        if (_isSolveScoreGenericMetricChipName(item.name)) {
            return [_solveScoreGenericMetricPublicName, ...(item.params || [])];
        }
        if (_solveScoreMetricSet.has(item.name)) {
            const source = _splitSolveScoreMetricSourceLag(item.params[0], 'slv').wire;
            const q = item.params[1];
            return source === 'slv' ? [item.name, q] : [item.name, source, q];
        }
        if (item.name === 'emit') {
            const rawMode = String((item.params || [])[0] || 'raw').trim().toLowerCase();
            const mode = rawMode === 'norm' ? 'norm' : (rawMode === 'none' ? 'none' : 'raw');
            return ['emit', mode];
        }
        return item.params && item.params.length ? [item.name, ...item.params] : [item.name];
    });
}

function _buildSolveScoreProgramSpec(program) {
    return (Array.isArray(program) ? program : []).map((token) => {
        if (!token || !token.kind) return '';
        if (token.kind === 'metric') return `m${token.slot}-${Number(token.lag) || 0}`;
        if (token.kind === 'const') return `const:${String(token.value)}`;
        if (token.kind === 'omega_cosine') {
            return Math.abs(Number(token.phase) || 0) < 1e-12
                ? `omega_cosine:${String(token.omega)}`
                : `omega_cosine:${String(token.omega)}:${String(token.phase)}`;
        }
        if (token.kind === 'sawtooth') return `sawtooth:${String(token.mult)}`;
        if (token.kind === 'weighted_sum') return `weighted_sum:${String(token.a)}:${String(token.b)}`;
        if (token.kind === 'ema') return `ema:${String(token.alpha)}`;
        if (token.kind === 'pow') return `pow:${String(token.exponent)}`;
        if (token.kind === 'emit') {
            if (token.mode === 'norm') return 'emit_norm';
            if (token.mode === 'none') return 'emit_none';
            return 'emit';
        }
        return token.kind;
    }).filter(Boolean).join(';');
}

function _compileSolveScoreChain(chain, fallbackMetric = 'proximity', legacyQuantilePct = '0.1') {
    const normalized = _normalizeSolveScoreChain(chain, fallbackMetric, legacyQuantilePct);
    if (!normalized.length) {
        throw new Error('Solve score chain must contain at least one metric chip');
    }
    const currentSlotCandidates = new Map();
    normalized.forEach((item) => {
        const details = _solveScoreItemMetricDetails(item, legacyQuantilePct);
        if (!details || (Number(details.lag) || 0) !== 0) return;
        const source = details.source;
        const allowedSources = _solveScoreMetricAllowedSources(details.metric);
        if (!source || !allowedSources.includes(source)) return;
        const qPct = Number(details.q);
        if (!Number.isFinite(qPct)) return;
        const familyKey = `${details.metric}\u0000${source}`;
        const slotKey = `${details.metric}\u0000${source}\u0000${qPct / 100}`;
        if (!currentSlotCandidates.has(familyKey)) currentSlotCandidates.set(familyKey, []);
        const candidates = currentSlotCandidates.get(familyKey);
        if (!candidates.includes(slotKey)) candidates.push(slotKey);
    });
    const metrics = [];
    const baseSlotMap = new Map();
    let stackDepth = 0;
    let omega = 1;
    let omegaPhase = 0;
    let omegaEnabled = false;
    const program = [];
    const outputChannels = [];
    let hasExplicitOutputToken = false;
    normalized.forEach((item) => {
        const metricDetails = _solveScoreItemMetricDetails(item, legacyQuantilePct);
        if (metricDetails) {
            const metricName = metricDetails.metric;
            const source = metricDetails.source;
            const lag = Number(metricDetails.lag) || 0;
            const allowedSources = _solveScoreMetricAllowedSources(metricName);
            if (!source || !allowedSources.includes(source)) {
                throw new Error(`${metricName} source must be one of ${allowedSources.join(', ')}`);
            }
            const qPct = Number(metricDetails.q);
            if (!Number.isFinite(qPct) || qPct < 0.1 || qPct > 5.0) {
                throw new Error(`${metricName} q must be in [0.1, 5.0]%`);
            }
            let slot = -1;
            let slotKey = '';
            if (lag === 0) {
                slotKey = `${metricName}\u0000${source}\u0000${qPct / 100}`;
            } else if (lag === 1) {
                const candidates = currentSlotCandidates.get(`${metricName}\u0000${source}`) || [];
                if (candidates.length === 1) {
                    slotKey = candidates[0];
                } else if (candidates.length === 0) {
                    slotKey = `${metricName}\u0000${source}\u0000${qPct / 100}`;
                } else {
                    throw new Error(`Lagged ${metricName}(${source}-1) is ambiguous; choose an explicit base slot`);
                }
            } else {
                throw new Error('Solve-score lag depth other than 0 or 1 is unsupported');
            }
            if (baseSlotMap.has(slotKey)) {
                slot = baseSlotMap.get(slotKey);
            } else {
                const [slotMetric, slotSource, slotQuantile] = slotKey.split('\u0000');
                slot = metrics.length;
                metrics.push({
                    slot,
                    source: slotSource,
                    metric: slotMetric,
                    quantile_pct: Number(slotQuantile) * 100,
                    quantile: Number(slotQuantile),
                });
                baseSlotMap.set(slotKey, slot);
            }
            program.push({ kind: 'metric', slot, lag });
            stackDepth += 1;
            return;
        }
        if (item.name === 'const') {
            if (item.params.length !== 1) throw new Error('const requires exactly one parameter');
            const value = Number(item.params[0]);
            if (!Number.isFinite(value)) throw new Error('const requires a finite numeric value');
            program.push({ kind: 'const', value });
            stackDepth += 1;
            return;
        }
        if (item.name === 'dup') {
            if (stackDepth < 1) throw new Error('dup requires one score value on the stack');
            if (item.params.length !== 0) throw new Error('dup takes no parameters');
            program.push({ kind: 'dup' });
            stackDepth += 1;
            return;
        }
        if (item.name === 'flush') {
            if (item.params.length !== 0) throw new Error('flush takes no parameters');
            program.push({ kind: 'flush' });
            stackDepth = 0;
            return;
        }
        if (item.name === 'omega_cosine') {
            if (stackDepth < 1) throw new Error('omega_cosine requires one score value on the stack');
            if (item.params.length !== 1 && item.params.length !== 2) {
                throw new Error('omega_cosine requires one omega parameter and an optional phase parameter');
            }
            const omegaText = String(item.params[0] ?? '').trim();
            const rawOmega = Number(omegaText);
            const rawPhase = item.params.length > 1 ? Number(item.params[1]) : 0;
            if (!omegaText || !Number.isFinite(rawOmega)) {
                throw new Error('omega_cosine requires one finite numeric omega');
            }
            if (!Number.isFinite(rawPhase)) {
                throw new Error('omega_cosine requires one numeric phase');
            }
            omega = rawOmega;
            omegaPhase = rawPhase;
            omegaEnabled = true;
            program.push({ kind: 'omega_cosine', omega, phase: omegaPhase });
            return;
        }
        if (item.name === 'sawtooth') {
            if (stackDepth < 1) throw new Error('sawtooth requires one score value on the stack');
            if (item.params.length !== 1) throw new Error('sawtooth requires exactly one multiplier parameter');
            const rawMult = Number(item.params[0]);
            if (!Number.isFinite(rawMult)) {
                throw new Error('sawtooth requires one numeric multiplier');
            }
            program.push({ kind: 'sawtooth', mult: rawMult });
            return;
        }
        if (item.name === 'flip') {
            if (stackDepth < 1) throw new Error('flip requires one score value on the stack');
            if (item.params.length !== 0) throw new Error('flip takes no parameters');
            program.push({ kind: 'flip' });
            return;
        }
        if (['clamp', 'sin', 'cos', 'log', 'exp'].includes(item.name)) {
            if (stackDepth < 1) throw new Error(`${item.name} requires one score value on the stack`);
            if (item.params.length !== 0) throw new Error(`${item.name} takes no parameters`);
            program.push({ kind: item.name });
            return;
        }
        if (item.name === 'pow') {
            if (stackDepth < 1) throw new Error('pow requires one score value on the stack');
            if (item.params.length !== 1) throw new Error('pow requires exactly one exponent parameter');
            const exponent = Number(item.params[0]);
            if (!Number.isFinite(exponent)) throw new Error('pow requires a finite exponent');
            program.push({ kind: 'pow', exponent });
            return;
        }
        if (_solveScoreOutputSpecs[item.name]) {
            if (stackDepth < 1) throw new Error(`${item.name} requires one score value on the stack`);
            const rawMode = item.name === 'emit_norm' ? 'norm' : String((item.params || [])[0] || 'raw').trim().toLowerCase();
            const mode = rawMode === 'norm' || rawMode === 'normalized' || rawMode === 'emit_norm'
                ? 'norm'
                : ((rawMode === 'none' || rawMode === 'skip' || rawMode === 'off' || rawMode === 'emit_none' || rawMode === 'emit-none') ? 'none' : 'raw');
            hasExplicitOutputToken = true;
            const channel = outputChannels.length;
            program.push({ kind: 'emit', mode, channel });
            if (mode !== 'none') {
                if (outputChannels.length >= 8) throw new Error('Solve score supports at most 8 output channels');
                outputChannels.push({
                    name: `channel_${channel}`,
                    emit: mode === 'norm' ? 'emit_norm' : 'emit',
                    mode,
                    channel,
                    range_normalized: mode === 'norm',
                });
            }
            stackDepth -= 1;
            return;
        }
        const spec = _solveScoreCombineSpecs[item.name];
        if (!spec) throw new Error(`Invalid solve-score chip: ${item.name}`);
        if (stackDepth < spec.arity) throw new Error(`${item.name} requires ${spec.arity} inputs`);
        if ((item.params || []).length !== spec.params.length) {
            throw new Error(`${item.name} requires exactly ${spec.params.length} parameter(s)`);
        }
        const token = { kind: item.name };
        if (item.name === 'weighted_sum') {
            const a = Number(item.params[0]);
            const b = Number(item.params[1]);
            if (!Number.isFinite(a) || !Number.isFinite(b)) throw new Error('weighted_sum requires two numeric weights');
            if (!(Math.abs(a) > 0 || Math.abs(b) > 0)) throw new Error('weighted_sum requires at least one non-zero weight');
            token.a = a;
            token.b = b;
        } else if (item.name === 'ema') {
            const alpha = Number(item.params[0]);
            if (!Number.isFinite(alpha) || alpha < 0 || alpha > 1) throw new Error('ema alpha must be in [0,1]');
            token.alpha = alpha;
        }
        program.push(token);
        stackDepth -= spec.arity - 1;
    });
    if (!metrics.length) throw new Error('Solve score chain must contain at least one metric chip');
    const hasExplicitOutputs = hasExplicitOutputToken;
    if (hasExplicitOutputs) {
        if (outputChannels.length <= 0) throw new Error('Explicit-output solve score chain must emit at least one channel');
        if (stackDepth !== 0) throw new Error(`Explicit-output solve score chain must end with stack depth 0, got ${stackDepth}`);
    } else if (stackDepth !== 1) {
        throw new Error(`Solve score chain must end with stack depth 1, got ${stackDepth}`);
    } else {
        outputChannels.push({ name: 'score', emit: 'implicit', channel: 0, range_normalized: false });
    }
    const metric = metrics[0].metric;
    const quantile = metrics[0].quantile;
    const programSpec = _buildSolveScoreProgramSpec(program);
    const usesLag = program.some(token => token && token.kind === 'metric' && (Number(token.lag) || 0) > 0);
    const laggedSources = Array.from(new Set(program
        .filter(token => token && token.kind === 'metric' && (Number(token.lag) || 0) > 0)
        .map(token => metrics[token.slot] && metrics[token.slot].source)
        .filter(Boolean))).sort();
    const usesCoeffSource = _solveScoreMetricsUseSource(metrics, 'cf');
    const usesParamSource = _solveScoreMetricsUseSource(metrics, 'pm');
    return {
        metric,
        quantile,
        metrics,
        uses_coeff_source: usesCoeffSource,
        uses_param_source: usesParamSource,
        uses_non_solve_source: usesCoeffSource || usesParamSource,
        uses_lag: usesLag,
        max_lag: usesLag ? 1 : 0,
        prelude_by_source: {
            slv: laggedSources.includes('slv') ? 1 : 0,
            cf: laggedSources.includes('cf') ? 1 : 0,
            pm: laggedSources.includes('pm') ? 1 : 0,
        },
        omega,
        omega_phase: omegaPhase,
        omega_enabled: omegaEnabled,
        stack_depth: stackDepth,
        has_explicit_outputs: hasExplicitOutputs,
        output_channel_count: outputChannels.length,
        output_channels: outputChannels,
        output_interpretation: hasExplicitOutputs ? '' : 'scalar_lut',
        chain: _serializeSolveScoreChain(normalized, fallbackMetric, legacyQuantilePct),
        program_spec: programSpec,
        display: normalized.map(item => _displaySolveScoreEntry(item)).join(' '),
        legacy_compatible:
            !usesLag &&
            !(usesCoeffSource || usesParamSource) &&
            !hasExplicitOutputs &&
            (program.length === 1 && program[0].kind === 'metric') ||
            (
                !usesLag &&
                !(usesCoeffSource || usesParamSource) &&
                !hasExplicitOutputs &&
                program.length === 2 &&
                program[0].kind === 'metric' &&
                program[1].kind === 'omega_cosine' &&
                Math.abs(Number(program[1].phase) || 0) < 1e-12
            ),
    };
}

function _setSolveScoreLegacyField(id, value) {
    const el = document.getElementById(id);
    if (el) el.value = String(value);
}

function _legacySolveScoreState(prefix) {
    if (!_solveScoreLegacyFieldsPresent(prefix)) {
        const fallbackMetric = prefix === 'palette' ? paletteTabMetric : renderSolveMetric;
        try {
            return _compileSolveScoreChain(
                _chainForWhich(_solveScoreWhichForPrefix(prefix)),
                fallbackMetric,
                _legacySolveScoreQuantilePct(prefix)
            );
        } catch (_) {
            return _defaultSolveScoreState(prefix);
        }
    }
    const metricEl = document.getElementById(`${prefix}-solve-score`);
    const qEl = document.getElementById(`${prefix}-solve-score-quantile`);
    const omegaEl = document.getElementById(`${prefix}-solve-score-omega`);
    const phaseEl = document.getElementById(`${prefix}-solve-score-omega-phase`);
    const enabledEl = document.getElementById(`${prefix}-solve-score-omega-enabled`);
    const fallbackMetric = prefix === 'palette' ? paletteTabMetric : renderSolveMetric;
    const domMetric = String((metricEl && metricEl.value) || '').trim();
    const metric = _solveScoreMetricSet.has(fallbackMetric) && fallbackMetric !== domMetric
        ? fallbackMetric
        : String(domMetric || fallbackMetric || 'proximity').trim();
    const qPct = Number(qEl && qEl.value);
    const omega = Number(omegaEl && omegaEl.value);
    const omegaPhase = Number(phaseEl && phaseEl.value);
    const omegaEnabled = String((enabledEl && enabledEl.value) || '').trim().toLowerCase() === 'true';
    const chain = _solveScoreMetricSet.has(metric)
        ? _serializeSolveScoreChain(
            _defaultSolveScoreChain(
                metric,
                Number.isFinite(qPct) ? qPct : 0.1,
                omega,
                omegaEnabled,
                Number.isFinite(omegaPhase) ? omegaPhase : 0
            ),
            metric
        )
        : [];
    const compiled = chain.length ? _compileSolveScoreChain(chain, metric, Number.isFinite(qPct) ? qPct : 0.1) : null;
    return {
        metric,
        quantile: Number.isFinite(qPct) ? qPct / 100 : 0.001,
        omega: Number.isFinite(omega) ? omega : 1,
        omega_phase: Number.isFinite(omegaPhase) ? omegaPhase : 0,
        omega_enabled: omegaEnabled,
        chain,
        metrics: compiled ? compiled.metrics : [],
        uses_coeff_source: compiled ? compiled.uses_coeff_source : false,
        uses_param_source: compiled ? compiled.uses_param_source : false,
        uses_non_solve_source: compiled ? compiled.uses_non_solve_source : false,
        has_explicit_outputs: compiled ? !!compiled.has_explicit_outputs : false,
        output_channel_count: compiled ? Number(compiled.output_channel_count) || 1 : 1,
        output_channels: compiled ? (compiled.output_channels || []) : [{ name: 'score', emit: 'implicit', channel: 0, range_normalized: false }],
        program_spec: compiled ? compiled.program_spec : '',
        display: compiled ? compiled.display : '',
    };
}

function _resolveSolveScoreState(prefix, options = {}) {
    const requireChain = !!options.requireChain;
    const fallbackMetric = prefix === 'palette' ? paletteTabMetric : renderSolveMetric;
    const legacy = _legacySolveScoreState(prefix);
    let compiled = null;
    try {
        compiled = _compileSolveScoreChain(_chainForWhich(_solveScoreWhichForPrefix(prefix)), fallbackMetric);
    } catch (e) {
        if (legacy.chain.length) return legacy;
        if (requireChain) throw e;
        return legacy;
    }
    const legacyValid = legacy.chain.length > 0;
    const diverged = legacyValid && (
        legacy.metric !== compiled.metric ||
        legacy.quantile !== compiled.quantile ||
        legacy.omega !== compiled.omega ||
        legacy.omega_phase !== compiled.omega_phase ||
        legacy.omega_enabled !== compiled.omega_enabled
    );
    if (diverged) return legacy;
    return compiled;
}

function _syncSolveScoreLegacyInputs(prefix) {
    const compiled = _compileSolveScoreChain(
        _chainForWhich(_solveScoreWhichForPrefix(prefix)),
        prefix === 'palette' ? paletteTabMetric : renderSolveMetric,
        _legacySolveScoreQuantilePct(prefix)
    );
    _setSolveScoreLegacyField(`${prefix}-solve-score`, compiled.metric);
    _setSolveScoreLegacyField(`${prefix}-solve-score-quantile`, _solveScoreQuantilePctText(compiled.quantile * 100));
    _setSolveScoreLegacyField(`${prefix}-solve-score-omega`, compiled.omega);
    _setSolveScoreLegacyField(`${prefix}-solve-score-omega-phase`, compiled.omega_phase);
    _setSolveScoreLegacyField(`${prefix}-solve-score-omega-enabled`, compiled.omega_enabled ? 'true' : 'false');
    const qValEl = document.getElementById(`${prefix}-solve-score-quantile-val`);
    if (qValEl) qValEl.textContent = _solveScoreQuantilePctText(compiled.quantile * 100);
    const omegaValEl = document.getElementById(`${prefix}-solve-score-omega-val`);
    if (omegaValEl) omegaValEl.textContent = String(compiled.omega);
    if (prefix === 'palette') paletteTabMetric = compiled.metric;
    else renderSolveMetric = compiled.metric;
    return compiled;
}

function _setSolveScoreChainFromLegacy(prefix, metric, quantilePct = 0.1, omega = 1, omegaEnabled = false, omegaPhase = 0) {
    const which = _solveScoreWhichForPrefix(prefix);
    const chain = _chainForWhich(which);
    const next = _defaultSolveScoreChain(metric, quantilePct, omega, omegaEnabled, omegaPhase);
    chain.splice(0, chain.length, ...next);
    _syncSolveScoreUi(which);
    return _syncSolveScoreLegacyInputs(prefix);
}

let _solveScoreModalState = {
    filterText: '',
    sortKey: 'name',
    sortDir: 1,
    open: false,
    prefix: 'render',
    tableState: 'idle',
    rows: [],
    selectedId: '',
    selectedProgram: null,
    selectedError: '',
    selectedLoading: false,
    actionBusy: false,
    status: '',
    statusError: false,
    nameInput: '',
    lastSelectedName: '',
};
;(window.__ppParts = window.__ppParts || []).push('02-preview-solvescore');
