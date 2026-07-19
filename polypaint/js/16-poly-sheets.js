/* ---- Poly-Sheets tab: parameter-scan mosaic artifacts (poly-sheet.md) ----
 * One async lambda renders every frame of a sheet (the pipeline triple
 * with a $-token substituted per frame), stitched into one bilevel PNG.
 * The pipeline comes from the SAME shared controls the compute preview
 * reads; this tab adds the scan/frame/grid knobs, the run/cancel
 * buttons, and the gallery. */

let _sheetsInventory = [];
let _activeSheetRun = null;   // {sheetId, jobId, taskId}

function _sheetNewId() {
    return 'sheet_' + Date.now().toString(36) + '_' + Math.random().toString(36).slice(2, 8);
}

function _sheetVal(id, fallback) {
    const el = document.getElementById(id);
    if (!el) return fallback;
    const v = el.value;
    return v === '' || v == null ? fallback : v;
}

function _sheetSpacingChanged() {
    const isStep = String(_sheetVal('sheet-spacing', 'linear')) === 'step';
    for (const id of ['sheet-step', 'sheet-step-label']) {
        const el = document.getElementById(id);
        if (el) el.style.display = isStep ? '' : 'none';
    }
    const toEl = document.getElementById('sheet-to');
    if (toEl) toEl.disabled = isStep;
}

function _sheetInheritedFrame() {
    /* N, pixels, viewport mode/bounds, and rotation all come from the
     * COMPUTE preview controls — the sheet renders what Preview shows.
     * Throws with a user-facing message when explicit bounds are missing. */
    const n = Math.max(8, Math.min(256, parseInt(_sheetVal('compute-preview-n', 96), 10) || 96));
    const tile = Math.max(32, Math.min(1024, parseInt(_sheetVal('compute-preview-size', 256), 10) || 256));
    const rotate = parseInt(_sheetVal('compute-preview-rotate', 0), 10) || 0;
    const previewMode = (typeof _computePreviewViewportMode !== 'undefined') ? _computePreviewViewportMode : 'quantile';
    const freeze = !!document.getElementById('sheet-freeze')?.checked;
    let viewport;
    if (previewMode === 'quantile') {
        viewport = {
            mode: freeze ? 'frozen' : 'quantile',
            quantile: Math.max(0, Math.min(0.1, (parseFloat(_sheetVal('compute-preview-quantile', 0)) || 0) / 100)),
            shim: Math.max(0, Math.min(0.5, (parseFloat(_sheetVal('compute-preview-shim', 5)) || 5) / 100)),
        };
    } else {
        const b = _computePreviewExplicitBounds();
        if (!b) {
            throw new Error(previewMode === 'marquee'
                ? 'Marquee viewport: drag a rectangle on the Compute preview image first.'
                : 'Square viewport: enter a positive side length on the Compute preview.');
        }
        viewport = { mode: 'explicit', min_re: b.min_re, max_re: b.max_re, min_im: b.min_im, max_im: b.max_im };
    }
    return {
        n, tile_px: tile, rotate, viewport,
        polarity: String(_sheetVal('sheet-polarity', 'white_on_black')),
        margin_px: Math.max(0, Math.min(64, parseInt(_sheetVal('sheet-margin', 4), 10) || 0)),
    };
}

async function runPolySheet() {
    const btn = document.getElementById('btn-sheet-run');
    const statusEl = document.getElementById('sheets-status');
    const funcName = document.getElementById('render-function')?.value || '';
    if (!funcName) {
        if (statusEl) { statusEl.textContent = 'Select a function on the Render tab first.'; statusEl.className = 'status error'; }
        return;
    }
    const token = String(_sheetVal('sheet-token', '$T')).trim();
    const steps = Math.max(1, Math.min(256, parseInt(_sheetVal('sheet-steps', 16), 10) || 16));
    const sheetId = _sheetNewId();
    const jobId = sheetId;
    const taskId = 'sheet_run_' + sheetId;

    let frame;
    try {
        frame = _sheetInheritedFrame();
        frame.solver_mode = String(_sheetVal('sheet-solver', 'ae64'));
    } catch (e) {
        if (statusEl) { statusEl.textContent = e.message; statusEl.className = 'status error'; }
        return;
    }

    const payload = _attachProgramSourcePayload({
        action: 'run',
        job_id: jobId,
        task_id: taskId,
        sheet_id: sheetId,
        function: funcName,
        cfpv: _cfpv.length > 0 ? [..._cfpv] : [],
        param_transforms: _effectiveParamTransformsForCompute(),
        param_program_chain: _effectiveParamProgramChainForCompute(),
        coeff_transforms: _effectiveCoeffTransformsForCompute(),
        coeff_program_chain: _effectiveCoeffProgramChainForCompute(),
        scan: {
            token,
            from: parseFloat(_sheetVal('sheet-from', 0)),
            to: parseFloat(_sheetVal('sheet-to', 1)),
            step: parseFloat(_sheetVal('sheet-step', 1)),
            steps,
            spacing: String(_sheetVal('sheet-spacing', 'linear')),
        },
        frame,
        grid_cols: parseInt(_sheetVal('sheet-cols', 0), 10) || Math.ceil(Math.sqrt(steps)),
    });

    const orig = btn ? btn.textContent : 'Run Sheet';
    if (btn) { btn.disabled = true; btn.textContent = 'Dispatching...'; }
    if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: dispatching ${steps} frames...`; statusEl.className = 'status'; }
    try {
        const disp = await lambdaPost('dispatch', {
            target: 'poly_sheet',
            jobs: [payload],
            expected_keys: [],
        });
        if ((disp.fired || 0) !== 1) throw new Error('poly-sheet dispatch failed');
        _activeSheetRun = { sheetId, jobId, taskId };
        _jobsRailUpsert({
            id: 'sheet:' + sheetId, kind: 'sheet',
            label: `Sheet ${steps}f N=${frame.n} · ${funcName}`,
            jobId, tab: 'sheets', state: 'running', startedAt: Date.now(),
            detail: 'dispatched',
        });
        if (btn) btn.textContent = 'Rendering...';
        await _pollSheetRun(sheetId, jobId, taskId, statusEl);
        if (btn) { btn.textContent = '✓ Done'; setTimeout(() => { btn.textContent = orig; btn.disabled = false; }, 1500); }
    } catch (e) {
        _jobsRailUpsert({ id: 'sheet:' + sheetId, state: 'error', detail: e.message });
        if (statusEl) { statusEl.textContent = 'Sheet failed: ' + e.message; statusEl.className = 'status error'; }
        if (btn) { btn.textContent = orig; btn.disabled = false; }
        return;
    }
    void loadSheetsTab();
}

async function _pollSheetRun(sheetId, jobId, taskId, statusEl) {
    while (true) {
        await new Promise(r => setTimeout(r, 3000));
        const check = await lambdaPost('storage', {
            job_id: jobId, task_prefix: taskId, expected: 1,
        }, '/check-status');
        if (check.errors > 0) {
            const detail = check.error_details?.[0] || {};
            _jobsRailUpsert({ id: 'sheet:' + sheetId, state: 'error', detail: detail.error_msg || 'error' });
            throw new Error(detail.error_msg || 'sheet failed');
        }
        const rd = check.results?.[0] || {};
        const label = rd.phase_label || rd.phase || 'working';
        if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: ${label}`; statusEl.className = 'status'; }
        _jobsRailUpsert({ id: 'sheet:' + sheetId, state: 'running', detail: label });
        if (rd.phase === 'done') {
            _jobsRailUpsert({ id: 'sheet:' + sheetId, state: 'done', detail: `${rd.frames} frames` });
            if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: done (${rd.frames} frames, ${rd.elapsed_ms}ms)`; statusEl.className = 'status ok'; }
            return;
        }
    }
}

async function cancelPolySheet() {
    const statusEl = document.getElementById('sheets-status');
    if (!_activeSheetRun) {
        if (statusEl) statusEl.textContent = 'No active sheet run.';
        return;
    }
    const { sheetId } = _activeSheetRun;
    await lambdaPost('dispatch', {
        target: 'poly_sheet',
        jobs: [{ action: 'cancel', sheet_id: sheetId }],
        expected_keys: [],
    });
    if (statusEl) statusEl.textContent = `Sheet ${sheetId}: cancel requested (takes effect between frames).`;
}

async function loadSheetsTab() {
    const invEl = document.getElementById('sheets-inventory');
    try {
        const resp = await lambdaPost('storage', {}, '/list-sheets');
        _sheetsInventory = resp.sheets || [];
    } catch (e) {
        if (invEl) invEl.textContent = 'Failed to list sheets: ' + e.message;
        return;
    }
    if (!invEl) return;
    if (_sheetsInventory.length === 0) {
        invEl.innerHTML = '<div style="padding:8px; color:#888">No sheets yet.</div>';
        return;
    }
    invEl.innerHTML = _sheetsInventory.map(row =>
        `<div class="sheet-row" style="padding:4px 8px; cursor:pointer; border-bottom:1px solid #222" ` +
        `onclick="_viewSheet('${_escapeHtml(row.sheet_id)}')">` +
        `${_escapeHtml(row.sheet_id)} <span style="color:#666">${_escapeHtml(row.modified.slice(0, 19))}</span></div>`
    ).join('');
}

let _sheetViewerId = null;
let _sheetViewerFit = true;

function _sheetApplyFit() {
    const img = document.getElementById('sheet-viewer-img');
    const btn = document.getElementById('btn-sheet-fit');
    if (img) img.style.maxWidth = _sheetViewerFit ? '100%' : 'none';
    if (btn) btn.textContent = _sheetViewerFit ? '1:1' : 'Fit';
}

function _sheetToggleFit() {
    _sheetViewerFit = !_sheetViewerFit;
    _sheetApplyFit();
}

function _sheetOpenFull() {
    if (_sheetViewerId) window.open(_publicStorageUrl(`sheets/${_sheetViewerId}/sheet.png`), '_blank');
}

async function _sheetDownload(btn) {
    if (!_sheetViewerId) return;
    const orig = btn ? btn.textContent : 'Download';
    if (btn) { btn.disabled = true; btn.textContent = 'Fetching...'; }
    try {
        const resp = await fetch(_publicStorageUrl(`sheets/${_sheetViewerId}/sheet.png`));
        if (!resp.ok) throw new Error('HTTP ' + resp.status);
        const blob = await resp.blob();
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `${_sheetViewerId}.png`;
        document.body.appendChild(a);
        a.click();
        a.remove();
        setTimeout(() => URL.revokeObjectURL(url), 10000);
        if (btn) { btn.textContent = '✓ Saved'; setTimeout(() => { btn.textContent = orig; btn.disabled = false; }, 1500); }
    } catch (e) {
        if (btn) { btn.textContent = 'Failed: ' + e.message; setTimeout(() => { btn.textContent = orig; btn.disabled = false; }, 2500); }
    }
}

function _viewSheet(sheetId) {
    const viewer = document.getElementById('sheet-viewer');
    const img = document.getElementById('sheet-viewer-img');
    const meta = document.getElementById('sheet-viewer-meta');
    if (!viewer || !img) return;
    _sheetViewerId = sheetId;
    viewer.style.display = '';
    _sheetApplyFit();
    img.src = _publicStorageUrl(`sheets/${sheetId}/sheet.png`) + '?t=' + Date.now();
    if (meta) {
        meta.textContent = sheetId + ' (loading manifest...)';
        fetch(_publicStorageUrl(`sheets/${sheetId}/sheet.json`) + '?t=' + Date.now())
            .then(r => r.json())
            .then(m => {
                meta.textContent = `${sheetId}: ${m.frames} frames · ${m.grid.cols}x${m.grid.rows} · ` +
                    `N=${m.n} tile=${m.tile_px}px · ${m.solver_mode} · ` +
                    `${m.scan.token} ${m.scan.from}..${m.scan.to} (${m.scan.spacing}) · ` +
                    `viewport ${m.viewport.mode} · ${m.elapsed_ms}ms`;
            })
            .catch(() => { meta.textContent = sheetId; });
    }
}

;(window.__ppParts = window.__ppParts || []).push('16-poly-sheets');
