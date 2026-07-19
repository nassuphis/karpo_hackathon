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
        solver_mode: String(_sheetVal('compute-preview-solver', 'ae64')),
        polarity: String(_sheetVal('sheet-polarity', 'white_on_black')),
        margin_px: Math.max(0, Math.min(64, parseInt(_sheetVal('sheet-margin', 4), 10) || 0)),
        label: !!document.getElementById('sheet-label')?.checked,
    };
}

const SHEET_MAX_WORKERS = 8;

function _sheetFrameRanges(steps, workers) {
    // contiguous, balanced: frame k goes to worker floor(k*W/steps)
    const ranges = Array.from({ length: workers }, () => []);
    for (let k = 0; k < steps; k++) ranges[Math.floor(k * workers / steps)].push(k);
    return ranges.filter(r => r.length > 0);
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
    } catch (e) {
        if (statusEl) { statusEl.textContent = e.message; statusEl.className = 'status error'; }
        return;
    }

    const payload = _attachProgramSourcePayload({
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

    const ranges = _sheetFrameRanges(steps, Math.min(SHEET_MAX_WORKERS, steps));
    const workerJobs = ranges.map((indices, w) => ({
        ...payload,
        action: 'frames',
        task_id: `sheet_tiles_${sheetId}_w${w}`,
        frame_indices: indices,
    }));

    const orig = btn ? btn.textContent : 'Run Sheet';
    const startedAtS = Date.now() / 1000;
    if (btn) { btn.disabled = true; btn.textContent = 'Dispatching...'; }
    if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: dispatching ${steps} frames to ${workerJobs.length} workers...`; statusEl.className = 'status'; }
    try {
        const disp = await lambdaPost('dispatch', {
            target: 'poly_sheet',
            jobs: workerJobs,
            expected_keys: [],
        });
        if ((disp.fired || 0) !== workerJobs.length) throw new Error('poly-sheet worker dispatch failed');
        _activeSheetRun = { sheetId, jobId, taskId };
        _jobsRailUpsert({
            id: 'sheet:' + sheetId, kind: 'sheet',
            label: `Sheet ${steps}f N=${frame.n} · ${funcName}`,
            jobId, tab: 'sheets', state: 'running', startedAt: Date.now(),
            detail: `dispatched (${workerJobs.length} workers)`,
        });
        if (btn) btn.textContent = 'Rendering...';
        await _pollSheetWorkers(sheetId, jobId, steps, workerJobs.length, statusEl);

        if (btn) btn.textContent = 'Stitching...';
        const stitchTask = `sheet_stitch_${sheetId}`;
        const stitch = await lambdaPost('dispatch', {
            target: 'poly_sheet',
            jobs: [{ ...payload, action: 'stitch', task_id: stitchTask,
                     started_at_s: startedAtS }],
            expected_keys: [],
        });
        if ((stitch.fired || 0) !== 1) throw new Error('poly-sheet stitch dispatch failed');
        const doneRd = await _pollSheetTask(sheetId, jobId, stitchTask, statusEl, 'stitching', 'sheet:' + sheetId);
        _jobsRailUpsert({ id: 'sheet:' + sheetId, state: 'done', detail: `${doneRd.frames} frames` });
        if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: done (${doneRd.frames} frames, ${doneRd.elapsed_ms}ms) — generating DeepZoom...`; statusEl.className = 'status ok'; }
        if (btn) { btn.textContent = '✓ Done'; setTimeout(() => { btn.textContent = orig; btn.disabled = false; }, 1500); }

        // DeepZoom pyramid right away — the sheet viewer of choice
        void _sheetGenerateDeepZoom(sheetId, statusEl).catch(() => {});
    } catch (e) {
        _jobsRailUpsert({ id: 'sheet:' + sheetId, state: 'error', detail: e.message });
        if (statusEl) { statusEl.textContent = 'Sheet failed: ' + e.message; statusEl.className = 'status error'; }
        if (btn) { btn.textContent = orig; btn.disabled = false; }
        return;
    }
    void loadSheetsTab();
}

async function _pollSheetWorkers(sheetId, jobId, steps, workers, statusEl) {
    while (true) {
        await new Promise(r => setTimeout(r, 3000));
        const check = await lambdaPost('storage', {
            job_id: jobId, task_prefix: `sheet_tiles_${sheetId}`, expected: workers,
        }, '/check-status');
        if (check.errors > 0) {
            const detail = check.error_details?.[0] || {};
            throw new Error(detail.error_msg || 'sheet worker failed');
        }
        const rows = check.results || [];
        const doneFrames = rows.reduce((a, r) => a + (Number(r.frame) || 0), 0);
        const doneRows = rows.filter(r => r.phase === 'done').length;
        const label = `${doneFrames}/${steps} frames (${doneRows}/${workers} workers)`;
        if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: ${label}`; statusEl.className = 'status'; }
        _jobsRailUpsert({ id: 'sheet:' + sheetId, state: 'running', detail: label });
        if (doneRows >= workers) return;
    }
}

async function _pollSheetTask(sheetId, jobId, taskId, statusEl, phaseLabel, railId = null) {
    // railId: rail card to keep updating, or null — the deepzoom poll must
    // NOT touch the sheet's card (it would resurrect a done card to running)
    while (true) {
        await new Promise(r => setTimeout(r, 3000));
        const check = await lambdaPost('storage', {
            job_id: jobId, task_prefix: taskId, expected: 1,
        }, '/check-status');
        if (check.errors > 0) {
            const detail = check.error_details?.[0] || {};
            throw new Error(detail.error_msg || `sheet ${phaseLabel} failed`);
        }
        const rd = check.results?.[0] || {};
        const label = rd.phase_label || rd.phase || phaseLabel;
        if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: ${label}`; statusEl.className = 'status'; }
        if (railId) _jobsRailUpsert({ id: railId, state: 'running', detail: label });
        if (rd.phase === 'done') return rd;
    }
}

const _sheetDzExports = {};   // sheet_id -> {dzi_url, share_url}
const _sheetDzInflight = {};  // sheet_id -> pending generation promise

function _sheetGenerateDeepZoom(sheetId, statusEl) {
    // dedupe: rapid re-clicks on a legacy sheet must not fire twice
    if (!_sheetDzInflight[sheetId]) {
        _sheetDzInflight[sheetId] = _sheetGenerateDeepZoomInner(sheetId, statusEl)
            .finally(() => { delete _sheetDzInflight[sheetId]; });
    }
    return _sheetDzInflight[sheetId];
}

async function _sheetGenerateDeepZoomInner(sheetId, statusEl) {
    const taskId = `sheet_dz_${sheetId}_${Date.now().toString(36)}`;
    const railId = 'sheetdz:' + sheetId;
    _jobsRailUpsert({
        id: railId, kind: 'sheet', label: `DeepZoom ${sheetId}`,
        jobId: sheetId, tab: 'sheets', state: 'running',
        startedAt: Date.now(), detail: 'dispatched',
    });
    try {
        const disp = await lambdaPost('dispatch', {
            target: 'deepzoom_export',
            jobs: [{ action: 'sheet', sheet_id: sheetId, job_id: sheetId,
                     export_id: 'dz_' + Date.now(), task_id: taskId }],
            expected_keys: [],
        });
        if ((disp.fired || 0) !== 1) throw new Error('deepzoom dispatch failed');
        const rd = await _pollSheetTask(sheetId, sheetId, taskId, statusEl, 'deepzoom', railId);
        if (rd.dzi_url) {
            _sheetDzExports[sheetId] = { dzi_url: rd.dzi_url, share_url: rd.share_url };
            if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: DeepZoom ready`; statusEl.className = 'status ok'; }
        }
        _jobsRailUpsert({ id: railId, state: 'done', detail: 'ready' });
        return _sheetDzExports[sheetId] || null;
    } catch (e) {
        _jobsRailUpsert({ id: railId, state: 'error', detail: e.message });
        throw e;
    }
}

async function _sheetFindDeepZoom(sheetId) {
    if (_sheetDzExports[sheetId]) return _sheetDzExports[sheetId];
    try {
        const resp = await lambdaPost('storage', {}, '/list-deepzoom');
        const hit = (resp.exports || []).find(e => e.job_id === sheetId && e.dzi_url);
        if (hit) {
            _sheetDzExports[sheetId] = { dzi_url: hit.dzi_url, share_url: hit.share_url };
            return _sheetDzExports[sheetId];
        }
    } catch (e) { /* listing failure -> treat as no export */ }
    return null;
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
        `<div class="sheet-row" data-sheet-id="${_escapeHtml(row.sheet_id)}" ` +
        `style="padding:4px 8px; cursor:pointer; border-bottom:1px solid #222" ` +
        `onclick="_viewSheet('${_escapeHtml(row.sheet_id)}')">` +
        `${_escapeHtml(row.sheet_id)} <span style="color:#666">${_escapeHtml(row.modified.slice(0, 19))}</span></div>`
    ).join('');
    _sheetMarkSelectedRow();
}

function _sheetMarkSelectedRow() {
    const invEl = document.getElementById('sheets-inventory');
    if (!invEl) return;
    for (const rowEl of invEl.querySelectorAll('.sheet-row')) {
        rowEl.classList.toggle('selected', rowEl.dataset.sheetId === _sheetViewerId);
    }
}

let _sheetViewerId = null;
let _sheetViewSeq = 0;
let _sheetOsd = null;

function _sheetShowOsd(dziUrl) {
    const el = document.getElementById('sheet-osd');
    if (!el) return;
    if (_sheetOsd) {
        _sheetOsd.destroy();
        _sheetOsd = null;
    }
    el.innerHTML = '';
    _sheetOsd = OpenSeadragon({
        element: el,
        tileSources: dziUrl,
        prefixUrl: 'https://cdnjs.cloudflare.com/ajax/libs/openseadragon/4.1.1/images/',
        showNavigator: true,
        navigatorPosition: 'BOTTOM_RIGHT',
        navigatorMaintainSizeRatio: false,
        navigatorWidth: '160px',
        navigatorHeight: '160px',
        maxZoomPixelRatio: 4,
    });
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
    const meta = document.getElementById('sheet-viewer-meta');
    if (!viewer) return;
    _sheetViewerId = sheetId;
    const seq = ++_sheetViewSeq;   // rapid list clicks: only the latest wins
    _sheetMarkSelectedRow();
    viewer.style.display = '';
    if (meta) {
        meta.textContent = sheetId + ' (loading manifest...)';
        fetch(_publicStorageUrl(`sheets/${sheetId}/sheet.json`) + '?t=' + Date.now())
            .then(r => r.json())
            .then(m => {
                if (seq !== _sheetViewSeq) return;
                meta.textContent = `${sheetId}: ${m.frames} frames · ${m.grid.cols}x${m.grid.rows} · ` +
                    `N=${m.n} tile=${m.tile_px}px · ${m.solver_mode} · ` +
                    `${m.scan.token} ${m.scan.from}..${m.scan.to} (${m.scan.spacing}) · ` +
                    `viewport ${m.viewport.mode} · ${m.elapsed_ms}ms`;
            })
            .catch(() => { if (seq === _sheetViewSeq) meta.textContent = sheetId; });
    }
    void _sheetViewDeepZoom(sheetId, seq);
}

async function _sheetViewDeepZoom(sheetId, seq) {
    const statusEl = document.getElementById('sheets-status');
    const osdEl = document.getElementById('sheet-osd');
    try {
        let exp = await _sheetFindDeepZoom(sheetId);
        if (!exp) {
            // legacy sheet with no pyramid yet: generate it in place
            if (seq === _sheetViewSeq && osdEl && !_sheetOsd) {
                osdEl.innerHTML = '<div style="color:#888; font-size:12px; padding:16px">Generating DeepZoom for ' + _escapeHtml(sheetId) + '...</div>';
            }
            if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: generating DeepZoom...`; statusEl.className = 'status'; }
            exp = await _sheetGenerateDeepZoom(sheetId, statusEl);
        }
        if (!exp || !exp.dzi_url) throw new Error('no DZI produced');
        if (seq !== _sheetViewSeq) return;   // user moved on to another sheet
        _sheetShowOsd(exp.dzi_url);
    } catch (e) {
        if (seq !== _sheetViewSeq) return;
        if (statusEl) { statusEl.textContent = `Sheet ${sheetId}: DeepZoom failed — ${e.message}`; statusEl.className = 'status error'; }
    }
}

function _setInputValue(id, value) {
    const el = document.getElementById(id);
    if (el != null && value != null) el.value = String(value);
}

async function populateSelectedSheet(btn) {
    const statusEl = document.getElementById('sheets-status');
    if (!_sheetViewerId) {
        if (statusEl) { statusEl.textContent = 'Select a sheet first.'; statusEl.className = 'status error'; }
        return;
    }
    const sheetId = _sheetViewerId;
    const orig = btn ? btn.textContent : 'Populate';
    if (btn) { btn.disabled = true; btn.textContent = 'Populating...'; }
    try {
        const resp = await fetch(_publicStorageUrl(`sheets/${sheetId}/sheet.json`) + '?t=' + Date.now());
        if (!resp.ok) throw new Error('manifest HTTP ' + resp.status);
        const m = await resp.json();

        // sheet scan + frame knobs first (the compute populate switches tabs)
        _setInputValue('sheet-token', m.scan?.token);
        _setInputValue('sheet-from', m.scan?.from);
        _setInputValue('sheet-to', m.scan?.to);
        if (m.scan?.spacing === 'step') _setInputValue('sheet-step', m.scan?.step);
        _setInputValue('sheet-steps', m.scan?.steps);
        _setInputValue('sheet-spacing', m.scan?.spacing);
        _sheetSpacingChanged();
        _setInputValue('sheet-cols', m.grid?.cols);
        _setInputValue('sheet-polarity', m.polarity);
        _setInputValue('sheet-margin', m.margin_px);
        const freezeEl = document.getElementById('sheet-freeze');
        if (freezeEl) freezeEl.checked = m.viewport?.mode === 'frozen';
        const labelEl = document.getElementById('sheet-label');
        if (labelEl) labelEl.checked = !!m.label;

        // the inherited compute-preview controls
        _setInputValue('compute-preview-n', m.n);
        _setInputValue('compute-preview-size', m.tile_px);
        _setInputValue('compute-preview-rotate', m.rotate);
        _setInputValue('compute-preview-solver', m.solver_mode);
        if (m.viewport) {
            _setInputValue('compute-preview-quantile', (m.viewport.quantile || 0) * 100);
            _setInputValue('compute-preview-shim', (m.viewport.shim || 0.05) * 100);
        }
        if (m.viewport?.mode !== 'explicit') {
            const radio = document.querySelector('input[name="compute-preview-viewport-mode"][value="quantile"]');
            if (radio) { radio.checked = true; _setComputePreviewViewportMode('quantile'); }
        }
        if (typeof _applyComputePreviewRotation === 'function') _applyComputePreviewRotation();

        // pipeline (function + programs + cfpv) into the Compute tab
        _populateComputeFromDetail(sheetId, {
            pipeline: m.pipeline || {},
            calc: { solver: m.solver_mode },
        });
        if (statusEl) {
            const note = m.viewport?.mode === 'explicit'
                ? ' (explicit viewport: re-drag the marquee on the preview)'
                : '';
            statusEl.textContent = `Populated Compute + Sheets controls from ${sheetId}${note}.`;
            statusEl.className = 'status ok';
        }
        if (btn) { btn.textContent = '✓ Populated'; setTimeout(() => { btn.textContent = orig; btn.disabled = false; }, 1500); }
    } catch (e) {
        if (statusEl) { statusEl.textContent = 'Populate failed: ' + e.message; statusEl.className = 'status error'; }
        if (btn) { btn.textContent = orig; btn.disabled = false; }
    }
}

function _sheetsArrowNav(e) {
    // same convention as the Results / Palette / Render artifact tables
    // (bound from js/12-deepzoom-boot.js — parts are declaration-only)
    const tab = document.getElementById('tab-sheets');
    if (!tab || !tab.classList.contains('active')) return;
    if (e.key !== 'ArrowUp' && e.key !== 'ArrowDown') return;
    if (_isTextInputFocused()) return;
    if (!_sheetsInventory.length) return;
    e.preventDefault();

    const idx = _sheetViewerId
        ? _sheetsInventory.findIndex(r => r.sheet_id === _sheetViewerId)
        : -1;
    let next;
    if (e.key === 'ArrowDown') next = idx < _sheetsInventory.length - 1 ? idx + 1 : idx;
    else next = idx > 0 ? idx - 1 : 0;
    if (next < 0) next = 0;
    const row = _sheetsInventory[next];
    if (!row || row.sheet_id === _sheetViewerId) return;
    _viewSheet(row.sheet_id);
    const rowEl = document.querySelector(
        `#sheets-inventory .sheet-row[data-sheet-id="${CSS.escape(row.sheet_id)}"]`);
    if (rowEl && typeof rowEl.scrollIntoView === 'function') rowEl.scrollIntoView({ block: 'nearest' });
}

;(window.__ppParts = window.__ppParts || []).push('16-poly-sheets');
