// PolyPaint 12-deepzoom-boot — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Deploy rewrites the script tags to build-versioned asset
// keys (assets/<BUILD_ID>/...), so a deploy flips atomically via index.html.
function _parseRenderSourceRef(sourceKey) {
    const key = String(sourceKey || '').trim();
    if (!key) return { jobId: '', family: '', artifactId: '' };
    const artifactMatch = key.match(/^renders\/([^/]+)\/([^/]+)\/([^/]+)\//);
    if (artifactMatch) {
        const dir = artifactMatch[2];
        const family = dir === 'palettes'
            ? 'palette'
            : (dir === 'color' || dir === 'bilevel' || dir === 'coeffs' || dir === 'pdf' ? dir : '');
        return {
            jobId: artifactMatch[1],
            family,
            artifactId: artifactMatch[3],
        };
    }
    const jobMatch = key.match(/^renders\/([^/]+)\//);
    return { jobId: jobMatch ? jobMatch[1] : '', family: '', artifactId: '' };
}

function _dzRenderSourceRef(ex) {
    const parsed = _parseRenderSourceRef(ex && ex.source_key);
    const explicitJobId = String((ex && ex.render_job_id) || '').trim();
    const jobId = explicitJobId || parsed.jobId;
    const canSelectArtifact = !!(parsed.family && parsed.artifactId && (!explicitJobId || explicitJobId === parsed.jobId));
    return {
        jobId,
        family: canSelectArtifact ? parsed.family : '',
        artifactId: canSelectArtifact ? parsed.artifactId : '',
    };
}

function _dzRenderSourceLabel(ex) {
    const renderRef = _dzRenderSourceRef(ex);
    const explicitArtifactId = String((ex && ex.render_artifact_id) || '').trim();
    return explicitArtifactId || renderRef.artifactId || renderRef.jobId || '?';
}

// ── Add a DeepZoom export to the ACTIVE gallery ─────────────────────────────
// The DeepZoom tab can only ADD; the Gallery tab creates + curates. "Add to
// Gallery" appends the selected COLOR export (a {job_id, artifact_id, export_id}
// pick) to the gallery chosen as active in the Gallery tab (localStorage
// 'polypaint_active_gallery'), via /add-to-gallery — which validates + enriches
// the single piece server-side and CAS-saves the editable gallery document.

function _dzGalleryPickForExport(ex) {
    if (!ex) return null;
    const exportId = String((ex && ex.export_id) || '').trim();
    if (!exportId) return null;
    // Rule: has a DZI => curatable. TWO distinct jobs travel with a pick: the
    // export OWNER (deepzoom/<export_job_id>/<export_id>/ — the row's own job)
    // and the render-source job (renders/<job_id>/ — parsed from source_key).
    // Collapsing them broke cross-job exports (review finding 1).
    const ref = _dzRenderSourceRef(ex);
    const exportJobId = String(ex.job_id || '').trim();
    const jobId = ref.jobId || exportJobId;
    const artifactId = ref.artifactId || String(ex.source_artifact_id || '').trim() || exportId;
    if (!jobId || !exportJobId) return null;
    return { job_id: jobId, export_job_id: exportJobId, artifact_id: artifactId, export_id: exportId };
}

// The active gallery id is owned by the Gallery tab (js/15); fall back to the
// shared localStorage key so this works even before that tab is opened.
function _dzActiveGalleryId() {
    if (typeof _galleryActiveId === 'function') return String(_galleryActiveId() || '').trim();
    try { return String(localStorage.getItem('polypaint_active_gallery') || '').trim(); } catch (e) { return ''; }
}

async function _dzAddSelectedToGallery() {
    const inv = window._dzInventory || [];
    const idx = window._dzSelectedIdx ?? -1;
    const ex = idx >= 0 && idx < inv.length ? inv[idx] : null;
    const pick = _dzGalleryPickForExport(ex);
    const statusEl = document.getElementById('deepzoom-status');
    const setStatus = (msg, cls) => { if (statusEl) { statusEl.textContent = msg; statusEl.className = cls || 'status'; } };
    if (!pick) {
        // NEVER silent: flash the button and say exactly what is missing.
        const btn0 = document.getElementById('btn-dz-add-gallery');
        if (btn0) {
            const orig0 = btn0.textContent;
            btn0.textContent = '✗ Not addable';
            setTimeout(() => { btn0.textContent = orig0; }, 1600);
        }
        const ref0 = ex ? _dzRenderSourceRef(ex) : {};
        let why;
        if (!ex) why = 'select a DeepZoom export first.';
        else if (!String(ex.export_id || '').trim()) why = 'this export has no export id (very old export) — re-export it from the Render tab.';
        else why = 'this export has no job id — re-export it from the Render tab.';
        setStatus('Cannot add: ' + why, 'status error');
        return;
    }
    const galleryId = _dzActiveGalleryId();
    if (!galleryId) {
        const btn1 = document.getElementById('btn-dz-add-gallery');
        if (btn1) { const o1 = btn1.textContent; btn1.textContent = '✗ No gallery'; setTimeout(() => { btn1.textContent = o1; }, 1600); }
        setStatus('No active gallery. Create or select one in the Gallery tab first.', 'status error');
        return;
    }

    const btn = document.getElementById('btn-dz-add-gallery');
    const origLabel = btn ? btn.textContent : '';
    // Immediate feedback while in flight, and a LINGERING result on the button
    // itself afterwards — a fast roundtrip otherwise flashes the busy label for
    // <300ms, which reads as "nothing happened".
    if (btn) { btn.disabled = true; btn.textContent = 'Adding…'; }
    const flash = (label) => {
        if (!btn) return;
        btn.textContent = label;
        setTimeout(() => {
            btn.textContent = origLabel;
            btn.disabled = !_dzGalleryPickForExport(ex);
        }, 1600);
    };
    try {
        const resp = await lambdaPost('storage',
            { gallery_id: galleryId, job_id: pick.job_id, export_job_id: pick.export_job_id,
              artifact_id: pick.artifact_id, export_id: pick.export_id },
            '/add-to-gallery', { idempotent: false });
        if (resp && resp.error) throw new Error(resp.error);
        const name = (resp && resp.gallery && resp.gallery.name) || 'gallery';
        const count = (resp && resp.gallery && (resp.gallery.pieces || []).length) || 0;
        if (resp && resp.added) {
            flash('✓ Added');
            setStatus(`Added ${pick.artifact_id} to “${name}” (${count})`, 'status ok');
        } else {
            const reason = (resp && resp.reason) || 'not added';
            const human = {
                duplicate: 'already in this gallery',
                gallery_full: 'gallery is full',
                non_square: 'render is not square',
                missing_image: 'render image is missing',
                missing_preview: 'render preview is missing',
                unknown_preview_dimensions: 'preview dimensions unreadable',
                export_not_found: 'DeepZoom export not found',
                export_identity_mismatch: 'export belongs to a different render',
                export_dzi_key_mismatch: 'export DZI key mismatch',
                export_dzi_absent: 'export DZI file is missing',
                export_dzi_invalid: 'export DZI descriptor is malformed',
                export_preview_tile_missing: 'export preview tile is missing',
            }[reason] || reason;
            if (reason === 'duplicate') { flash('✓ Already added'); setStatus('Not added: ' + human, 'status'); }
            else { flash('✗ Not added'); setStatus('Not added: ' + human, 'status error'); }
        }
        // Hand the updated gallery + its new revision to the Gallery tab so it
        // can merge the added piece without a stale-revision conflict.
        if (typeof _galleryNotifyChanged === 'function') {
            _galleryNotifyChanged(galleryId, resp && resp.gallery, resp && resp.revision);
        }
    } catch (e) {
        flash('✗ Failed');
        setStatus('Add to Gallery failed: ' + e.message, 'status error');
    }
}

function _dzSelectKey(key) {
    const inv = window._dzInventory || [];
    const nextKey = String(key || '');
    const idx = inv.findIndex((ex) => _dzStableKey(ex) === nextKey);
    if (idx < 0) return;
    window._dzSelectedIdx = idx;
    window._dzSelectedKey = nextKey;
    document.querySelectorAll('.dz-inv-row').forEach((r, i) => {
        r.style.background = i === idx ? '#2a2a4e' : '';
    });
    const row = document.querySelector(`.dz-inv-row[data-key="${_encodeStableRowKey(nextKey)}"]`);
    if (row) row.scrollIntoView({ block: 'nearest' });
    _dzSetButtonsEnabled(true);
    _scheduleDeepZoomViewportReadout();
    const dziUrl = _safeHttpUrl(inv[idx].dzi_url);
    if (!dziUrl) {
        _dzClearViewer();
        return;
    }
    viewDeepZoom(dziUrl);
}

function _dzRenderInventory(selectIdx = null) {
    const container = document.getElementById('deepzoom-inventory');
    const inv = window._dzInventory || [];
    if (!container) return;
    if (!inv.length) {
        window._dzSelectedIdx = -1;
        window._dzSelectedKey = '';
        _dzSetButtonsEnabled(false);
        _dzClearViewer();
        container.innerHTML = '<div style="color:#666">No DeepZoom exports yet. Generate one from the Render tab.</div>';
        return;
    }

    let html = '<table style="width:100%; border-collapse:collapse" id="dz-inv-table">';
    html += '<tr style="border-bottom:1px solid #333; position:sticky; top:0; background:#1a1a2e"><th style="text-align:left;padding:4px">Job</th><th style="text-align:left;padding:4px">Render</th><th>Size</th><th>Created</th><th>Tiles</th><th>Share</th></tr>';
    inv.forEach((ex, i) => {
        const shareUrl = _safeHttpUrl(ex.share_url);
        const renderLabel = _dzRenderSourceLabel(ex);
        const shareCell = shareUrl
            ? `<a href="${_escapeHtml(shareUrl)}" target="_blank" rel="noopener noreferrer" onclick="event.stopPropagation()" style="color:#4a9eff;text-decoration:none">Open</a>`
            : '<span style="color:#555">?</span>';
        const key = _encodeStableRowKey(_dzStableKey(ex));
        html += `<tr class="dz-inv-row" data-key="${key}" style="border-bottom:1px solid #222; cursor:pointer" onclick="_dzSelectKey(_decodeStableRowKey(this.dataset.key || ''))" tabindex="-1">`;
        html += `<td style="padding:4px; font-family:monospace; font-size:11px">${_escapeHtml(ex.job_id)}</td>`;
        html += `<td style="padding:4px; font-family:monospace; font-size:11px">${_escapeHtml(renderLabel)}</td>`;
        html += `<td style="padding:4px; text-align:center">${_escapeHtml(`${ex.width}×${ex.height}`)}</td>`;
        html += `<td style="padding:4px; font-size:11px">${_escapeHtml((ex.created_at || '').replace('T', ' ').slice(0, 19))}</td>`;
        html += `<td style="padding:4px; text-align:center">${_escapeHtml(ex.tiles_uploaded || '')}</td>`;
        html += `<td style="padding:4px; text-align:center">${shareCell}</td>`;
        html += `</tr>`;
    });
    html += '</table>';
    container.innerHTML = html;
    let idx = Number.isFinite(Number(selectIdx)) ? Number(selectIdx) : -1;
    if (window._dzSelectedKey) {
        const keyIdx = inv.findIndex((ex) => _dzStableKey(ex) === window._dzSelectedKey);
        if (keyIdx >= 0) idx = keyIdx;
    }
    if (idx < 0 || idx >= inv.length) idx = 0;
    _dzSelect(idx);
}

function _dzSelect(idx) {
    const inv = window._dzInventory || [];
    if (idx < 0 || idx >= inv.length) return;
    _dzSelectKey(_dzStableKey(inv[idx]));
}

async function _dzGotoSelectedResult() {
    const inv = window._dzInventory || [];
    const idx = window._dzSelectedIdx ?? -1;
    if (idx < 0 || idx >= inv.length) return;
    const ex = inv[idx];
    await _ensureResultsSelection(ex.job_id);
    switchTab('results');
}

async function _dzGotoSelectedRender() {
    const inv = window._dzInventory || [];
    const idx = window._dzSelectedIdx ?? -1;
    if (idx < 0 || idx >= inv.length) return;
    const ex = inv[idx];
    const renderRef = _dzRenderSourceRef(ex);
    if (!renderRef.jobId) return;
    const canPopulate = !!(renderRef.family && renderRef.artifactId);
    const btn = document.getElementById('btn-dz-goto-render');
    const orig = btn ? btn.textContent : 'GotoRender';
    const statusEl = document.getElementById('deepzoom-status');
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Opening...'; }
        _setRenderResultsJob(renderRef.jobId);
        const refreshOptions = renderRef.family && renderRef.artifactId
            ? { selectFamily: renderRef.family, selectArtifactId: renderRef.artifactId }
            : {};
        await refreshRenderArtifacts(renderRef.jobId, refreshOptions);
        switchTab('render');
        if (canPopulate) {
            populateSelectedRenderArtifact();
            const visible = _dzViewportReadoutState.visibleBounds;
            if (visible) {
                _setRenderExplicitViewportBounds({
                    minRe: visible.min_re,
                    maxRe: visible.max_re,
                    minIm: visible.min_im,
                    maxIm: visible.max_im,
                });
                selectViewMode('explicit');
                log(`DeepZoom GoRender: ${renderRef.jobId} with visible viewport`, 'ok', 'deepzoom-log');
            } else {
                log(`DeepZoom GoRender: ${renderRef.jobId}`, 'ok', 'deepzoom-log');
            }
        } else {
            const msg = `Opened render job ${renderRef.jobId} without populate: source artifact unavailable`;
            const renderStatusEl = document.getElementById('render-status');
            if (renderStatusEl) {
                renderStatusEl.textContent = msg;
                renderStatusEl.className = 'status';
            }
            log(msg, '', 'deepzoom-log');
            log(msg, '', 'render-log');
        }
    } catch (e) {
        log(`DeepZoom GoRender failed: ${e.message}`, 'err', 'deepzoom-log');
        if (statusEl) {
            statusEl.textContent = 'GotoRender error: ' + e.message;
            statusEl.className = 'status error';
        }
    } finally {
        if (btn) btn.textContent = orig;
        _dzSetButtonsEnabled(true);
    }
}

async function _dzPopulateSelectedResult() {
    const inv = window._dzInventory || [];
    const idx = window._dzSelectedIdx ?? -1;
    if (idx < 0 || idx >= inv.length) return;
    const ex = inv[idx];
    const btn = document.getElementById('btn-dz-populate');
    const orig = btn ? btn.textContent : 'PopulateResult';
    try {
        if (btn) { btn.disabled = true; btn.textContent = '...'; }
        const detail = await _getResultDetail(ex.job_id);
        _populateComputeFromDetail(ex.job_id, detail || {});
    } catch (e) {
        log(`DeepZoom populate failed: ${e.message}`, 'err', 'compute-log');
        const statusEl = document.getElementById('compute-status');
        if (statusEl) {
            statusEl.textContent = 'Populate error: ' + e.message;
            statusEl.className = 'status error';
        }
    } finally {
        if (btn) { btn.textContent = orig; }
        _dzSetButtonsEnabled(true);
    }
}

async function _dzDeleteSelected() {
    const inv = window._dzInventory || [];
    const idx = window._dzSelectedIdx ?? -1;
    if (idx < 0 || idx >= inv.length) return;
    const ex = inv[idx];
    const btn = document.getElementById('btn-dz-delete');
    const statusEl = document.getElementById('deepzoom-status');
    const orig = btn ? btn.textContent : 'Delete';
    if (!confirm(`Delete DeepZoom export ${ex.export_id} for ${ex.job_id}?`)) return;
    const prefix = `deepzoom/${ex.job_id}/${ex.export_id}/`;
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Deleting...'; }
        if (statusEl) {
            statusEl.textContent = `Deleting ${ex.export_id}...`;
            statusEl.className = 'status';
        }
        const resp = await lambdaPost('storage', { prefix }, '/delete-prefix');
        console.log(`Deleted ${resp.deleted} objects from ${prefix}`);
        // Also delete deepzoom_latest.json if it points to this export
        try {
            const ptr = await lambdaPost('storage', { key: `renders/${ex.job_id}/deepzoom_latest.json` }, '/presign');
            const ptrResp = await fetch(ptr.url);
            if (ptrResp.ok) {
                const ptrData = await ptrResp.json();
                if (ptrData.export_id === ex.export_id) {
                    await lambdaPost('storage', { keys: [`renders/${ex.job_id}/deepzoom_latest.json`] }, '/cleanup');
                }
            }
        } catch (e) { /* pointer may not exist */ }
        const nextInventory = inv.filter((_, i) => i !== idx);
        window._dzInventory = nextInventory;
        window._dzSelectedKey = '';
        const nextIdx = nextInventory.length ? Math.min(idx, nextInventory.length - 1) : -1;
        _dzRenderInventory(nextIdx);
        if (statusEl) {
            statusEl.textContent = `Deleted ${ex.export_id}`;
            statusEl.className = 'status ok';
        }
    } catch (e) {
        if (statusEl) {
            statusEl.textContent = 'Delete failed: ' + e.message;
            statusEl.className = 'status error';
        }
        alert('Delete failed: ' + e.message);
    } finally {
        if (btn) {
            btn.textContent = orig;
            const currentInv = window._dzInventory || [];
            const currentIdx = window._dzSelectedIdx ?? -1;
            btn.disabled = !(currentIdx >= 0 && currentIdx < currentInv.length);
        }
    }
}

// Arrow key navigation for deepzoom inventory
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

function viewDeepZoom(dziUrl) {
    const viewerEl = document.getElementById('deepzoom-viewer');
    viewerEl.style.display = 'block';

    if (_osdViewer) {
        _osdViewer.destroy();
        _osdViewer = null;
    }

    _osdViewer = OpenSeadragon({
        element: viewerEl,
        tileSources: dziUrl,
        prefixUrl: 'https://cdnjs.cloudflare.com/ajax/libs/openseadragon/4.1.1/images/',
        showNavigator: true,
        navigatorPosition: 'BOTTOM_RIGHT',
        navigatorMaintainSizeRatio: false,
        navigatorWidth: '160px',
        navigatorHeight: '160px',
        maxZoomPixelRatio: 4,
    });
    window._osdViewer = _osdViewer;
    if (_osdViewer && typeof _osdViewer.addHandler === 'function') {
        ['open', 'animation', 'animation-finish', 'resize'].forEach((eventName) => {
            _osdViewer.addHandler(eventName, _scheduleDeepZoomViewportReadout);
        });
    }
    _scheduleDeepZoomViewportReadout();
}

/**
 * Wave-based dispatch + poll with missing-task re-dispatch.
 * Dispatches jobs in waves (MAX_INFLIGHT), polls DynamoDB for completion,
 * and re-dispatches any tasks that were accepted (202) but never executed.
 *
 * @param {Object} opts
 * @param {Array} opts.jobs - Array of job payloads
 * @param {string} opts.jobId - DynamoDB partition key
 * @param {string} opts.taskPrefix - DynamoDB task_id prefix (e.g. 'bilevel_raster_')
 * @param {string} opts.target - dispatch target (e.g. 'bilevel')
 * @param {string} opts.label - human label for logs
 * @param {string} opts.logTarget - log element id
 * @returns {Promise<number>} elapsed ms
 */
async function _bilevelDispatchAndPoll(opts) {
    const { jobs, jobId, taskPrefix, target, label, logTarget } = opts;
    const MAX_INFLIGHT = 200;
    const BATCH_SIZE = 50;
    const POLL_MS = 3000;
    const GRACE_MS = 45000;      // stall grace before re-dispatch
    const MAX_REDISPATCH = 2;
    const POLL_TIMEOUT_MS = 600000;
    const STALL_LOG_MS = 30000;
    const statusEl = document.getElementById('render-status');
    const expected = jobs.length;
    const t0 = performance.now();
    let redispatchCount = 0;

    // Dispatch a list of jobs in batches of BATCH_SIZE
    async function dispatchBatch(jobList, batchLabel) {
        for (let i = 0; i < jobList.length; i += BATCH_SIZE) {
            const batch = jobList.slice(i, i + BATCH_SIZE);
            const r = await lambdaPost('dispatch', { target, jobs: batch, expected_keys: [] });
            if ((r.fired || 0) !== batch.length || (r.errors && r.errors.length))
                throw new Error(`${batchLabel} dispatch failed: fired ${r.fired || 0}/${batch.length}`);
        }
    }

    // Wave dispatch: keep MAX_INFLIGHT in-flight at a time
    let cursor = 0;
    let lastPollDone = 0;
    while (cursor < expected) {
        const inFlight = cursor - lastPollDone;
        const canSend = Math.min(MAX_INFLIGHT - inFlight, expected - cursor);
        if (canSend > 0) {
            const waveEnd = cursor + canSend;
            statusEl.textContent = `${label}: dispatching ${cursor + 1}-${waveEnd}/${expected}...`;
            await dispatchBatch(jobs.slice(cursor, waveEnd), label);
            log(`  ${label} dispatch: ${waveEnd}/${expected} sent`, '', logTarget);
            cursor = waveEnd;
        }
        // If more to send, poll until in-flight drops below threshold
        if (cursor < expected) {
            await new Promise(r => setTimeout(r, POLL_MS));
            const check = await lambdaPost('storage', {
                job_id: jobId, task_prefix: taskPrefix, expected
            }, '/check-status');
            lastPollDone = check.done + (check.errors || 0);
            statusEl.textContent = `${label}: ${check.done}/${cursor} done, dispatching...`;
        }
    }
    log(`  ${label}: all ${expected} dispatched`, '', logTarget);

    // Poll to completion with missing-task re-dispatch
    let lastDone = 0, lastProgressTime = performance.now(), lastStallLog = 0;
    while (true) {
        await new Promise(r => setTimeout(r, POLL_MS));
        const stallTime = performance.now() - lastProgressTime;
        const wantIds = stallTime > GRACE_MS && redispatchCount < MAX_REDISPATCH;
        const check = await lambdaPost('storage', {
            job_id: jobId, task_prefix: taskPrefix, expected,
            ...(wantIds ? { return_ids: true } : {})
        }, '/check-status');

        if (check.errors > 0) {
            const msgs = check.error_details.map(e => {
                const ctx = _formatTaskErrorContext(e);
                return e.task_id + ': ' + e.error_msg + (ctx ? `\n  context: ${ctx}` : '');
            }).join('\n');
            throw new Error(`${check.errors} ${label} tasks failed:\n${msgs}`);
        }
        if (check.done !== lastDone) {
            const delta = check.done - lastDone;
            log(`  ${label}: ${check.done}/${expected} done (+${delta}, ${_fmtMs(performance.now() - t0)})`, '', logTarget);
            lastDone = check.done;
            lastProgressTime = performance.now();
            lastStallLog = 0;
            statusEl.textContent = `${label}: ${check.done}/${expected}...`;
        }
        if (check.complete) break;

        // Re-dispatch missing tasks on stall
        if (wantIds && check.found_ids) {
            const foundSet = new Set(check.found_ids);
            const missingIndices = [];
            for (let i = 0; i < expected; i++) {
                if (!foundSet.has(`${taskPrefix}${i}`)) missingIndices.push(i);
            }
            if (missingIndices.length > 0) {
                redispatchCount++;
                const missingJobs = missingIndices.map(i => jobs[i]);
                const idPreview = missingIndices.length <= 20 ? missingIndices.join(',') : missingIndices.slice(0, 20).join(',') + '...';
                log(`  ⚠ ${label}: ${missingIndices.length} missing [${idPreview}], re-dispatching (attempt ${redispatchCount}/${MAX_REDISPATCH})`, 'warn', logTarget);
                await dispatchBatch(missingJobs, `${label} re-dispatch`);
                lastProgressTime = performance.now();
                lastStallLog = 0;
                continue;
            }
        }

        if (stallTime > STALL_LOG_MS && stallTime - lastStallLog > STALL_LOG_MS) {
            lastStallLog = stallTime;
            const sc = check.status_counts ? JSON.stringify(check.status_counts) : '{}';
            log(`  ⚠ ${label} stall ${_fmtMs(stallTime)}: ${lastDone}/${expected}, statuses=${sc}`, 'warn', logTarget);
        }
        if (stallTime > POLL_TIMEOUT_MS) {
            throw new Error(`${label} stalled: ${lastDone}/${expected} after ${_fmtMs(stallTime)}`);
        }
    }
    return performance.now() - t0;
}

const PARAM_DEBUG_MAX_N = 512;
const PARAM_DEBUG_MAX_PIX = 2048;

function _paramDebugRequestSettings() {
    const previewNRaw = parseInt(document.getElementById('compute-preview-n')?.value, 10);
    const renderNRaw = parseInt(document.getElementById('render-n')?.value, 10);
    const previewPixRaw = parseInt(document.getElementById('compute-preview-size')?.value, 10);
    const sourceN = Number.isFinite(previewNRaw) && previewNRaw > 0
        ? previewNRaw
        : (Number.isFinite(renderNRaw) && renderNRaw > 0 ? renderNRaw : 256);
    const n = Math.max(8, Math.min(PARAM_DEBUG_MAX_N, sourceN));
    const sourcePix = Number.isFinite(previewPixRaw) && previewPixRaw > 0 ? previewPixRaw : n * 2;
    const pix = Math.max(64, Math.min(PARAM_DEBUG_MAX_PIX, sourcePix));
    const notes = [];
    if (sourceN !== n) notes.push(`N capped from ${sourceN} to ${n}`);
    if (sourcePix !== pix) notes.push(`pix capped from ${sourcePix} to ${pix}`);
    return { n, pix, notes };
}

async function runParamDebug() {
    const btn = document.getElementById('btn-param-debug');
    btn.disabled = true;
    try {
        const { n, pix, notes } = _paramDebugRequestSettings();
        const mode = document.getElementById('param-debug-mode').value;
        const paramTransforms = _effectiveParamTransformsForCompute();
        const paramProgramChain = _effectiveParamProgramChainForCompute();
        // Empty chain = identity: z1=x1, z2=x2 (no forced unit_circle)

        const ptDisplay = _displayActiveParamPipeline(' → ');
        log(`Param debug: N=${n}, pix=${pix}, mode=${mode}, transforms=[${ptDisplay}]...`);
        if (notes.length) log(`  param debug uses preview-scale limits: ${notes.join(', ')}`, 'warn');
        document.getElementById('compute-status').textContent = 'Generating param debug image...';

        const result = await lambdaPost('param-debug', _attachProgramSourcePayload({
            N: n,
            param_transforms: paramTransforms,
            param_program_chain: paramProgramChain,
            mode,
            pix,
            job_id: document.getElementById('results-dir').value.trim() || 'debug',
        }));

        const images = result.images || [];
        log(`  param debug: ${result.total_ms}ms, ${result.n_points} points, ${images.length} image(s)`, 'ok');
        const statusEl = document.getElementById('compute-status');
        statusEl.replaceChildren();
        const summaryEl = document.createElement('div');
        summaryEl.style.marginBottom = '8px';
        summaryEl.textContent = `Param debug done (${result.total_ms}ms, ${result.n_points} points)`;
        statusEl.appendChild(summaryEl);

        for (const img of images) {
            const wrapperEl = document.createElement('div');
            wrapperEl.style.cssText = 'display:inline-block; vertical-align:top; margin-right:12px';

            const previewUrl = _safeHttpUrl(img.preview_url);
            if (previewUrl) {
                const previewEl = document.createElement('img');
                previewEl.src = previewUrl;
                previewEl.style.cssText = 'max-width:512px; border:1px solid #333; background:#000; display:block; margin-bottom:6px';
                wrapperEl.appendChild(previewEl);
            }

            const buttonEl = document.createElement('button');
            buttonEl.className = 'btn-primary';
            buttonEl.style.cssText = 'padding:4px 12px; font-size:12px';
            const fileSizeKb = Number.isFinite(img.file_size) ? (img.file_size / 1024).toFixed(0) : '?';
            buttonEl.textContent = `Download ${img.label || ''} TIFF (${img.pixels_set || 0} px, ${fileSizeKb} KB)`;
            const downloadUrl = _safeHttpUrl(img.url);
            if (downloadUrl) {
                buttonEl.addEventListener('click', () => window.open(downloadUrl, '_blank', 'noopener,noreferrer'));
            } else {
                buttonEl.disabled = true;
                buttonEl.title = 'Invalid download URL';
            }
            wrapperEl.appendChild(buttonEl);
            statusEl.appendChild(wrapperEl);
        }

    } catch (e) {
        document.getElementById('compute-status').textContent = 'Param debug error: ' + e.message;
        log(`Param debug failed: ${e.message}`, 'err');
    } finally {
        btn.disabled = false;
    }
}

async function runCalculateAEMT() { return openComputeSolverPopup('aberth_mt'); }
async function runCalculateCM() { return openComputeSolverPopup('companion_matrix'); }
async function runCalculateJT() { return openComputeSolverPopup('jenkins_traub'); }
async function runCalculateNewton() { return openComputeSolverPopup('newton'); }
async function runCalculateJT64() { return openComputeSolverPopup('jt64'); }
async function runCalculateCM64() { return openComputeSolverPopup('cm64'); }
async function runCalculateAE64() { return openComputeSolverPopup('ae64'); }
function _aggregateComputeParamGenPerf(results) {
    const row = Array.isArray(results) && results.length ? results[0] : null;
    if (!row) return null;
    const threads = Math.max(1, Number(row.threads) || 1);
    const dataBytes = Number(row.data_bytes) || 0;
    const uploadedBytesRaw = Number(row.uploaded_bytes);
    const uploadedBytes = Number.isFinite(uploadedBytesRaw) ? Math.max(0, Math.min(dataBytes || uploadedBytesRaw, uploadedBytesRaw)) : dataBytes;
    const uploadedStepsRaw = Number(row.uploaded_steps_est);
    const totalSteps = Number(row.n_steps) || 0;
    const uploadedSteps = Number.isFinite(uploadedStepsRaw) ? Math.max(0, Math.min(totalSteps || uploadedStepsRaw, uploadedStepsRaw)) : totalSteps;
    return {
        elapsed_us: Number(row.elapsed_us) || 0,
        n_steps: totalSteps,
        data_bytes: dataBytes,
        threads,
        uploaded_bytes: uploadedBytes,
        uploaded_steps_est: uploadedSteps,
        progress: Number.isFinite(Number(row.progress)) ? Number(row.progress) : ((dataBytes > 0) ? (uploadedBytes / dataBytes) : 1),
    };
}

function _aggregateComputeCoeffgenPerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (r.elapsed_us != null || r.coeffs_size != null)) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        acc.tasks += 1;
        acc.elapsed_us += Number(row.elapsed_us) || 0;
        acc.coeffs_size += Number(row.coeffs_size) || 0;
        acc.threads = Math.max(acc.threads, Number(row.threads) || 0);
        if (!acc.degree && row.degree != null) acc.degree = Number(row.degree) || 0;
        return acc;
    }, { tasks: 0, elapsed_us: 0, coeffs_size: 0, degree: 0, threads: 0 });
}

function _aggregateComputeSolvePerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (r.compute_us != null || r.bin_size != null)) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        const nT = Number(row.n_t) || 0;
        acc.tasks += 1;
        acc.compute_us += Number(row.compute_us) || 0;
        acc.bin_size += Number(row.bin_size) || 0;
        acc.n_t += nT;
        acc.avg_iterations_weighted += (Number(row.avg_iterations) || 0) * nT;
        acc.skipped_overflow += Number(row.skipped_overflow) || 0;
        return acc;
    }, { tasks: 0, compute_us: 0, bin_size: 0, n_t: 0, avg_iterations_weighted: 0, skipped_overflow: 0 });
}

function _aggregateComputeFusedPerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (r.solve_us != null || r.param_gen_us != null || r.coeffs_size != null)) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        const nT = Number(row.n_t) || 0;
        acc.tasks += 1;
        acc.param_gen_us += Number(row.param_gen_us) || 0;
        acc.coeffgen_us += Number(row.coeffgen_us) || 0;
        acc.solve_us += Number(row.solve_us) || 0;
        acc.compute_us += Number(row.compute_us) || 0;
        acc.upload_params_us += Number(row.upload_params_us) || 0;
        acc.upload_coeffs_us += Number(row.upload_coeffs_us) || 0;
        acc.upload_roots_us += Number(row.upload_roots_us) || 0;
        acc.params_size += Number(row.params_size) || 0;
        acc.coeffs_size += Number(row.coeffs_size) || 0;
        acc.bin_size += Number(row.bin_size) || 0;
        acc.n_t += nT;
        acc.avg_iterations_weighted += (Number(row.avg_iterations) || 0) * nT;
        acc.fused_threads = Math.max(acc.fused_threads, Number(row.fused_threads) || 0);
        return acc;
    }, { tasks: 0, param_gen_us: 0, coeffgen_us: 0, solve_us: 0, compute_us: 0, upload_params_us: 0, upload_coeffs_us: 0, upload_roots_us: 0, params_size: 0, coeffs_size: 0, bin_size: 0, n_t: 0, avg_iterations_weighted: 0, fused_threads: 0 });
}

function _computeWallPerfPrefix(wallMs) {
    return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)} ` : '';
}

function _computePhasePerfSummary(phase, results, wallMs) {
    if (phase === 'degree_probe') {
        const row = Array.isArray(results) && results.length ? results[0] : null;
        if (!row) return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
        let msg = `${_computeWallPerfPrefix(wallMs)}total=probe ${_fmtSecondsUs(Number(row.elapsed_us) || 0)}`;
        if (Number(row.coeffgen_us) > 0) msg += ` · coeffgen=${_fmtSecondsUs(Number(row.coeffgen_us) || 0)}`;
        if (row.degree != null) msg += ` · degree=${Number(row.degree) || 0}`;
        if (row.n_coeffs != null) msg += ` · n_coeffs=${Number(row.n_coeffs) || 0}`;
        return msg;
    }
    if (phase === 'param_gen' || phase === 'lores_param_gen') {
        const perf = _aggregateComputeParamGenPerf(results);
        if (!perf) return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
        let msg = `${_computeWallPerfPrefix(wallMs)}total=gen ${_fmtSecondsUs(perf.elapsed_us)}`;
        if (perf.threads > 0) msg += ` · threads=${perf.threads}`;
        const stepsDone = Math.max(0, perf.uploaded_steps_est || 0);
        const bytesDone = Math.max(0, perf.uploaded_bytes || 0);
        const totalSteps = Math.max(0, perf.n_steps || 0);
        const totalBytes = Math.max(0, perf.data_bytes || 0);
        const isInFlight = totalBytes > 0 && bytesDone < totalBytes;
        if (isInFlight) {
            msg += ` · steps ${(stepsDone / 1e6).toFixed(1)}M/${(totalSteps / 1e6).toFixed(1)}M`;
            msg += ` · size ${(bytesDone / 1e6).toFixed(1)}/${(totalBytes / 1e6).toFixed(1)}MB`;
            msg += ` · ${(Math.max(0, Math.min(1, perf.progress || 0)) * 100).toFixed(1)}%`;
        } else {
            msg += ` · ${totalSteps} steps · ${(totalBytes / 1e6).toFixed(1)}MB`;
        }
        return msg;
    }
    if (phase === 'coeffgen' || phase === 'lores_coeffgen') {
        const perf = _aggregateComputeCoeffgenPerf(results);
        if (!perf) return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
        let msg = `${_computeWallPerfPrefix(wallMs)}total=gen ${_fmtSecondsUs(perf.elapsed_us)} · size ${(perf.coeffs_size/1e6).toFixed(1)}MB`;
        if (perf.threads > 0) msg += ` · threads=${perf.threads}`;
        if (perf.degree > 0) msg += ` · degree=${perf.degree}`;
        if (perf.tasks > 1) msg += ` · chunks=${perf.tasks}`;
        return msg;
    }
    if (phase === 'solve' || phase === 'lores_solve') {
        const perf = _aggregateComputeSolvePerf(results);
        if (!perf) return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
        let msg = `${_computeWallPerfPrefix(wallMs)}total=solve ${_fmtSecondsUs(perf.compute_us)} · bin ${(perf.bin_size/1e6).toFixed(1)}MB`;
        if (perf.n_t > 0) msg += ` · avg_iters=${(perf.avg_iterations_weighted / perf.n_t).toFixed(1)}`;
        if (perf.skipped_overflow > 0) msg += ` · skipped=${perf.skipped_overflow}`;
        return msg;
    }
    if (phase === 'compute_chunk_fused') {
        const perf = _aggregateComputeFusedPerf(results);
        if (!perf) return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
        let msg = `${_computeWallPerfPrefix(wallMs)}total=param ${_fmtSecondsUs(perf.param_gen_us)} + coeff ${_fmtSecondsUs(perf.coeffgen_us)} + solve ${_fmtSecondsUs(perf.solve_us)}`;
        msg += ` · up ${_fmtSecondsUs(perf.upload_params_us + perf.upload_coeffs_us + perf.upload_roots_us)}`;
        msg += ` · params ${(perf.params_size/1e6).toFixed(1)}MB · coeff ${(perf.coeffs_size/1e6).toFixed(1)}MB · bin ${(perf.bin_size/1e6).toFixed(1)}MB`;
        if (perf.fused_threads > 0) msg += ` · threads=${perf.fused_threads}`;
        if (perf.tasks > 1) msg += ` · chunks=${perf.tasks}`;
        if (perf.n_t > 0) msg += ` · avg_iters=${(perf.avg_iterations_weighted / perf.n_t).toFixed(1)}`;
        return msg;
    }
    return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
}

function _computePhaseLogLabel(phaseLabel, phase, solverMode) {
    const base = phaseLabel || phase || 'Working';
    if (phase === 'solve' || phase === 'lores_solve') {
        return `${base} (${_solverTag(solverMode)})`;
    }
    return base;
}

async function _logComputePhaseCompletion(jobId, tracker, loggedCompletions = null) {
    if (!jobId || !tracker || !tracker.prefix) return;
    const key = `${tracker.phase || ''}|${tracker.prefix || ''}|complete`;
    if (loggedCompletions && loggedCompletions.has(key)) return;
    try {
        const subcheck = await lambdaPost('storage', {
            job_id: jobId,
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
        const perfText = _computePhasePerfSummary(tracker.phase, subcheck.results || [], wallMs);
        if (perfText) {
            const label = _computePhaseLogLabel(tracker.phase_label, tracker.phase, tracker.solver_mode);
            log(`${label} complete · ${perfText}`, 'ok');
            if (loggedCompletions) loggedCompletions.add(key);
        }
    } catch (e) {
        // best effort only
    }
}

function _computeLoresPhaseTrackers(runId, solverMode) {
    return [
        {
            phase: 'lores_param_gen',
            phase_label: 'Lores param gen',
            solver_mode: solverMode,
            prefix: `compute_${runId}_lores_param_gen`,
            expected: 1,
        },
        {
            phase: 'lores_coeffgen',
            phase_label: 'Lores coeffgen',
            solver_mode: solverMode,
            prefix: `compute_${runId}_lores_coeffgen`,
            expected: 1,
        },
        {
            phase: 'lores_solve',
            phase_label: 'Lores solve',
            solver_mode: solverMode,
            prefix: `compute_${runId}_lores_solve`,
            expected: 1,
        },
    ];
}

function _computeShouldPollKnownLores(phase) {
    return [
        'lores_param_gen',
        'lores_coeffgen',
        'lores_solve',
        'compute_chunk_fused',
        'solve',
        'save_metadata',
        'done',
    ].includes(String(phase || ''));
}

async function _logComputeKnownLoresCompletions(jobId, trackers, loggedCompletions) {
    if (!jobId || !Array.isArray(trackers) || !trackers.length) return;
    for (const tracker of trackers) {
        const key = `${tracker.phase || ''}|${tracker.prefix || ''}|complete`;
        if (loggedCompletions && loggedCompletions.has(key)) continue;
        try {
            const subcheck = await lambdaPost('storage', {
                job_id: jobId,
                task_prefix: tracker.prefix,
                expected: tracker.expected || 1,
            }, '/check-status');
            _logContractWarnings(subcheck.results || [], 'compute-log');
            if (!subcheck.complete) continue;
            const perfText = _computePhasePerfSummary(tracker.phase, subcheck.results || [], null);
            const label = _computePhaseLogLabel(tracker.phase_label, tracker.phase, tracker.solver_mode);
            log(`${label} complete${perfText ? ' · ' + perfText : ''}`, 'ok');
            if (loggedCompletions) loggedCompletions.add(key);
        } catch (e) {
            // best effort only
        }
    }
}

async function runCalculateWithSolver(solverMode, computeMtOptions) {
    const funcName = document.getElementById('render-function').value;
    const n = parseInt(document.getElementById('render-n').value);
    const nChunks = computeMtOptions && computeMtOptions.nChunks != null
        ? Math.max(1, parseInt(computeMtOptions.nChunks, 10) || 10)
        : Math.max(1, parseInt(document.getElementById('render-stripes').value) || 10);
    const times = Math.max(1, parseInt(document.getElementById('render-times').value) || 1);
    const jobId = 'compute_' + Date.now().toString(36);
    const hasParamGenThreadOverride = !!(computeMtOptions && computeMtOptions.paramGenThreads != null);
    const hasCoeffgenThreadOverride = !!(computeMtOptions && computeMtOptions.coeffgenThreads != null);
    const hasLoresParamGenThreadOverride = !!(computeMtOptions && computeMtOptions.loresParamGenThreads != null);
    const hasLoresCoeffgenThreadOverride = !!(computeMtOptions && computeMtOptions.loresCoeffgenThreads != null);
    const fused = !!(computeMtOptions && computeMtOptions.fused);
    const fusedThreads = computeMtOptions && computeMtOptions.fusedThreads != null
        ? _clampRenderMtThreads(computeMtOptions.fusedThreads)
        : (fused ? 4 : null);
    const paramGenThreads = hasParamGenThreadOverride ? _clampRenderMtThreads(computeMtOptions.paramGenThreads) : null;
    const coeffgenThreads = hasCoeffgenThreadOverride ? _clampRenderMtThreads(computeMtOptions.coeffgenThreads) : null;
    const loresParamGenThreads = hasLoresParamGenThreadOverride ? _clampRenderMtThreads(computeMtOptions.loresParamGenThreads) : null;
    const loresCoeffgenThreads = hasLoresCoeffgenThreadOverride ? _clampRenderMtThreads(computeMtOptions.loresCoeffgenThreads) : null;
    const effectiveParamGenThreads = fused ? fusedThreads : paramGenThreads;
    const effectiveCoeffgenThreads = fused ? fusedThreads : coeffgenThreads;

    const paramTransforms = _effectiveParamTransformsForCompute();
    const paramProgramChain = _effectiveParamProgramChainForCompute();
    const coeffTransforms = _effectiveCoeffTransformsForCompute();
    const coeffProgramChain = _effectiveCoeffProgramChainForCompute();
    const cfpv = _cfpv.length > 0 ? [..._cfpv] : null;
    const cfpvDisplay = cfpv ? _formatCfpvForDisplay(funcName, cfpv) : '';
    const ptDisplay = _displayActiveParamPipeline(',');
    const coeffDisplay = _displayActiveCoeffPipeline(',');
    const cfpvStr = cfpvDisplay ? `(${cfpvDisplay})` : '';
    const pipelineStr = `[${ptDisplay||'none'}] ${funcName}${cfpvStr} [${coeffDisplay||'none'}]`;

    const btn = document.getElementById(_solverButtonId(solverMode));

    btn.disabled = true;
    document.getElementById('compute-status').textContent = `Computing ${pipelineStr} N=${n}...`;
    document.getElementById('compute-status').className = 'status';
    const solverTag = _solverTag(solverMode);
    let computeMsg = `Compute-${solverTag}: ${pipelineStr} N=${n}, chunks=${nChunks}`;
    if (times > 1) computeMsg += `, times=${times}`;
    if (computeMtOptions) computeMsg += `, method=${fused ? 'fused' : 'classic'}`;
    if (fusedThreads != null) computeMsg += `, fused threads=${fusedThreads}`;
    if (!fused && effectiveParamGenThreads != null) computeMsg += `, param threads=${effectiveParamGenThreads}`;
    if (!fused && effectiveCoeffgenThreads != null) computeMsg += `, coeffgen threads=${effectiveCoeffgenThreads}`;
    if (loresParamGenThreads != null) computeMsg += `, lores param threads=${loresParamGenThreads}`;
    if (loresCoeffgenThreads != null) computeMsg += `, lores coeffgen threads=${loresCoeffgenThreads}`;
    computeMsg += '...';
    log(computeMsg);
    let computeRailId = '';
    try {
        const runId = _generateRunId();
        const taskId = `compute_run_${solverMode}_${runId}`;
        let lastPhaseLogKey = '';
        const orchPayload = {
            job_id: jobId,
            run_id: runId,
            params: _attachProgramSourcePayload({
                solver_mode: solverMode,
                N: n,
                n_chunks: nChunks,
                times,
                function: funcName,
                param_transforms: paramTransforms,
                param_program_chain: paramProgramChain,
                coeff_transforms: coeffTransforms,
                coeff_program_chain: coeffProgramChain,
                cfpv: cfpv || [],
            }),
        };
        if (computeMtOptions) {
            orchPayload.params.execution_method = fused ? 'fused_chunk_pipeline' : 'classic_chunk_pipeline';
            if (fused && fusedThreads != null) orchPayload.params.fused_threads = fusedThreads;
            const solverIters = Math.max(0, Math.min(64, parseInt(computeMtOptions.solverIters, 10) || 0));
            if (solverIters > 0 && (solverMode === 'aberth_mt' || solverMode === 'newton')) orchPayload.params.solver_iters = solverIters;
        }
        if (effectiveParamGenThreads != null) orchPayload.params.param_gen_threads = effectiveParamGenThreads;
        if (effectiveCoeffgenThreads != null) orchPayload.params.coeffgen_threads = effectiveCoeffgenThreads;
        if (loresParamGenThreads != null) orchPayload.params.lores_param_gen_threads = loresParamGenThreads;
        if (loresCoeffgenThreads != null) orchPayload.params.lores_coeffgen_threads = loresCoeffgenThreads;

        document.getElementById('compute-status').textContent = 'Dispatching compute orchestrator...';
        const dispResult = await lambdaPost('dispatch', {
            target: 'compute_orchestrator',
            jobs: [orchPayload],
            expected_keys: [],
        });
        if ((dispResult.fired || 0) !== 1) throw new Error('Compute orchestrator dispatch failed');
        if (dispResult.non_202 && dispResult.non_202.length > 0) {
            throw new Error(`Compute orchestrator invoke rejected: status ${dispResult.non_202[0].status}`);
        }
        log(`  compute orchestrator dispatched: ${runId}`, 'ok');
        computeRailId = 'compute:' + runId;
        _jobsRailUpsert({
            id: computeRailId,
            kind: 'compute',
            label: `${solverTag} N=${n} · ${jobId}`,
            jobId,
            tab: 'compute',
            state: 'running',
            detail: 'dispatched',
        });

        let phaseTracker = null;
        const loggedPhaseCompletions = new Set();
        const loresPhaseTrackers = _computeLoresPhaseTrackers(runId, solverMode);
        let pollKnownLores = false;
        while (true) {
            await new Promise(r => setTimeout(r, 3000));
            const check = await lambdaPost('storage', {
                job_id: jobId,
                task_prefix: taskId,
                expected: 1,
            }, '/check-status');

            if (check.errors > 0) {
                _logContractWarnings(check.error_details || [], 'compute-log');
                const detail = check.error_details?.[0] || {};
                const msg = detail.error_msg || 'unknown error';
                const ctx = _formatTaskErrorContext(detail);
                throw new Error(ctx ? `${msg} (${ctx})` : msg);
            }

            const rd = check.results?.[0] || {};
            _logContractWarnings([rd], 'compute-log');
            if (rd.execution_arn) {
                // arm the rail's kill button once the orchestrator has
                // recorded the Step Functions execution
                _jobsRailUpsert({ id: computeRailId, executionArn: rd.execution_arn, taskId });
            }
            const phase = rd.phase || '';
            const phaseLabel = rd.phase_label || phase || 'Working';
            const phaseStartMs = _coerceTimestampMs(rd.started_at_ms) || _coerceTimestampMs(rd.updated_at_ms);
            pollKnownLores = pollKnownLores || _computeShouldPollKnownLores(phase);

            if (phaseTracker && phaseTracker.phase !== phase) {
                await _logComputePhaseCompletion(jobId, phaseTracker, loggedPhaseCompletions);
                phaseTracker = null;
            }

            if (!phaseTracker && phase) {
                phaseTracker = {
                    phase,
                    phase_label: phaseLabel,
                    solver_mode: rd.solver_mode || solverMode,
                    started_at_ms: phaseStartMs,
                    prefix: rd.subtask_prefix || '',
                    expected: Number(rd.expected) || 0,
                    last_server_update_ms: _coerceTimestampMs(rd.updated_at_ms),
                };
            } else if (phaseTracker) {
                phaseTracker.last_server_update_ms = _pickLatestTimestampMs(
                    phaseTracker.last_server_update_ms,
                    rd.updated_at_ms,
                    check.latest_update_ms
                );
            }

            let done = 0;
            let expected = Number(rd.expected) || 0;
            let subResults = [];
            if (rd.subtask_prefix && expected > 0) {
                const subcheck = await lambdaPost('storage', {
                    job_id: jobId,
                    task_prefix: rd.subtask_prefix,
                    expected,
                }, '/check-status');
                done = Number(subcheck.done) || 0;
                expected = Number(subcheck.expected) || expected;
                subResults = subcheck.results || [];
                _logContractWarnings(subResults, 'compute-log');
                if (phaseTracker) {
                    phaseTracker.last_server_update_ms = _pickLatestTimestampMs(
                        phaseTracker.last_server_update_ms,
                        subcheck.latest_update_ms,
                        subcheck.latest_done_ms,
                        subcheck.latest_nonterminal_ms
                    );
                }
            }

            const phaseWallMs = _serverElapsedMs(
                phaseTracker ? phaseTracker.started_at_ms : phaseStartMs,
                _pickLatestTimestampMs(
                    phaseTracker ? phaseTracker.last_server_update_ms : null,
                    check.latest_update_ms,
                    check.latest_done_ms
                )
            );
            const perfText = _computePhasePerfSummary(phase, subResults || [], phaseWallMs);
            const phaseLogLabel = _computePhaseLogLabel(phaseLabel, phase, rd.solver_mode || solverMode);
            const perfRow = Array.isArray(subResults) && subResults.length ? subResults[0] : {};
            const perfProgressKey = (phase === 'param_gen' || phase === 'lores_param_gen')
                ? `${Number(perfRow.uploaded_bytes) || 0}|${Number(perfRow.uploaded_steps_est) || 0}|${Number(perfRow.threads) || 1}`
                : '';
            const phaseLogKey = `${phase}|${done}|${expected}|${perfProgressKey}`;
            if (phaseLogKey !== lastPhaseLogKey) {
                lastPhaseLogKey = phaseLogKey;
                let logMsg = phaseLogLabel;
                if (expected > 0) logMsg += ` ${done}/${expected}`;
                if (perfText) logMsg += ` · ${perfText}`;
                log(logMsg, '', 'compute-log');
            }
            if (pollKnownLores) {
                await _logComputeKnownLoresCompletions(jobId, loresPhaseTrackers, loggedPhaseCompletions);
            }

            document.getElementById('compute-status').textContent = expected > 0
                ? `${phaseLabel} ${done}/${expected}...`
                : `${phaseLabel}...`;
            document.getElementById('compute-status').className = 'status';
            _jobsRailProgress(computeRailId, expected > 0 ? `${phaseLabel} ${done}/${expected}` : phaseLabel);

            if (check.complete || rd.phase === 'done') {
                if (phaseTracker) await _logComputePhaseCompletion(jobId, phaseTracker, loggedPhaseCompletions);
                await _logComputeKnownLoresCompletions(jobId, loresPhaseTrackers, loggedPhaseCompletions);
                const detail = await lambdaPost('storage', { job_id: jobId }, '/detail');
                const calc = detail.calc || {};
                const chunks = Array.isArray(calc.chunks) ? calc.chunks : [];
                const totalBinSize = chunks.reduce((sum, row) => sum + (Number(row.bin_size) || 0), 0);
                const totalComputeUs = chunks.reduce((sum, row) => sum + (Number(row.compute_us) || 0), 0);
                const totalSteps = chunks.reduce((sum, row) => sum + (Number(row.n_t) || 0), 0);
                const avgIters = totalSteps > 0
                    ? chunks.reduce((sum, row) => sum + (Number(row.avg_iterations) || 0) * (Number(row.n_t) || 0), 0) / totalSteps
                    : 0;
                const totalWallMs = _serverElapsedMs(
                    _coerceTimestampMs(rd.run_started_at_ms),
                    _coerceTimestampMs(rd.updated_at_ms)
                );

                _lastCalcMeta = calc;
                document.getElementById('results-dir').value = jobId;
                _setRenderResultsJob(jobId);

                document.getElementById('compute-status').textContent =
                    `Computed | ${calc.n_chunks || chunks.length || nChunks} chunks | deg ${calc.degree || '?'} | ${calc.execution_method === 'fused_chunk_pipeline' ? 'fused' : 'classic'} | total ${_fmtSecondsMs(totalWallMs || 0)}`;
                document.getElementById('compute-status').className = 'status ok';

                const lines = [];
                if (totalWallMs != null) lines.push(`  total:    ${_fmtSecondsMs(totalWallMs)}`);
                lines.push(`  coeffgen: ${((Number(calc.total_coeffs_size) || 0)/1e6).toFixed(1)}MB (${calc.n_chunks || chunks.length || nChunks} chunks)`);
                lines.push(`  total bin: ${(totalBinSize/1e6).toFixed(1)}MB + lores ${((Number(calc.lores && calc.lores.bin_size) || 0)/1e3).toFixed(0)}KB`);
                lines.push(`  solve (${solverTag}): ${(totalComputeUs/1e6).toFixed(1)}s (sum)`);
                lines.push(`Compute-${solverTag} ${jobId}: ${pipelineStr} deg${calc.degree || '?'} N=${calc.N || n}, avg_iters=${avgIters.toFixed(1)}`);
                for (const line of lines) log(line, 'ok');
                _jobsRailUpsert({
                    id: computeRailId,
                    state: 'done',
                    detail: `deg ${calc.degree || '?'} · ${_fmtSecondsMs(totalWallMs || 0)}`,
                });
                _resultsInvalidate();   // a new compute exists — next Results visit refetches; in-flight responses are stale (CR30 follow-up F1)
                break;
            }
        }

    } catch (e) {
        document.getElementById('compute-status').textContent = 'Error: ' + e.message;
        document.getElementById('compute-status').className = 'status error';
        log(`Compute failed: ${e.message}`, 'err');
        if (computeRailId) _jobsRailUpsert({ id: computeRailId, state: 'failed', detail: e.message });
    } finally {
        btn.disabled = false;
    }
}


// Compute-tab boot calls live here because all classic script parts have
// loaded by this point.
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// Populate function dropdown from generated catalog
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)


/* Statements relocated from parts 01-05 (ran before the popup-init
   calls in the monolith): global listeners and the config load. */
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

/* ---- Boot sequence ----
   Moved from the monolith's mid-file position (js/06 tail): these are
   the top-level initializer calls, and they reference functions from
   several parts, so they must run only after every part is parsed. */
buildPaletteCircles('palette-circles-root-proximity', 'proximity', () => renderRootProximityPalette);
buildPaletteCircles('palette-circles-solve-score', 'solve_score', () => renderSolveScorePalette);
buildPaletteCircles('palette-circles-palette-tab', 'palette_tab', () => paletteTabPalette);
_initTriPalettePopup();
_initBuiltinPalettePopup();
_initLongPalettePopup();
_initCustomPalettePopup();
_initMicPalettePopup();
_initPicPalettePopup();
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
_initRootProgramModal();
_initCoeffProgramModal();
_updateSolveScoreButtons();  // initial disabled state
_syncPaletteColorInterpretationUi();
_clearPaletteCanvas('No palette selected');


/* Statements relocated from parts 07-11 (ran after the popup-init
   calls in the monolith): initial chip renders, vocab-load guard,
   visibility-change listener. */
if (!_coeffRegistryVocab) console.error('coeff_vocab_js.js did not load — registry transform vocabulary unavailable');
if (!_solveScoreVocab || !_solveScoreMetricNames.length) console.error('solve_score_vocab_js.js did not load — solve-score vocabulary unavailable');
if (!_rootRegistryVocab) console.error('root_vocab_js.js did not load — root transform vocabulary unavailable');


_ensureSolveScoreChainDefaults();
_ensureSolveScoreSourceDefaults();
_renderParamCoeffProgramCheatsheets();
_renderSolveScoreCheatsheets();

_syncSolveScoreUi('ss');

_syncSolveScoreUi('palette-ss');

_initJobsRail();

_syncRenderColorInterpretationUi();

// round-15 finding 4 + round-16 finding 4: resumePersistedCancels at BOOT
// (not only when the Sheets tab opens) — Compute is the default tab, so a
// reload must still re-issue cancels the user requested. js/12 parses
// BEFORE js/16 (index.html order), so a direct call here sees the function
// undefined and silently no-ops (the round-15 bug: the typeof guard turned
// a load-order error into a permanent no-op). Register a LOAD listener
// instead: 'load' fires after every classic script has parsed, so the
// callback really sees js/16's definition.
window.addEventListener('load', function () {
    if (typeof _sheetResumePersistedCancels === 'function') {
        void _sheetResumePersistedCancels();
    }
    if (typeof _sheetBindGeometryControls === 'function') {
        _sheetBindGeometryControls();
    }
});

document.addEventListener('visibilitychange', function() {
    if (document.visibilityState === 'visible') {
        resumeActiveRenderObserver();
        resumeActivePaletteObserver();
    } else {
        stopActiveRenderObserver();
        stopActivePaletteObserver();
    }
});


/* Statements relocated from later parts (round 1): listeners,
   window exports, sync calls — monolith tail order preserved. */
window.addEventListener('focus', function() {
    resumeActiveRenderObserver();
    resumeActivePaletteObserver();
});

(function() {
    _syncSolveScoreOmegaUi('render');
    _syncSolveScoreOmegaUi('palette');
    _syncRenderPreviewSourceMode();
    _updateResultsFilterUi();
    const renderJobInput = document.getElementById('render-results-dir');
    if (renderJobInput && !renderJobInput._polypaintBound) {
        renderJobInput._polypaintBound = true;
        renderJobInput.addEventListener('input', function() {
            const current = String(renderJobInput.value || '').trim();
            if (current !== _renderLoadedJobId) _invalidateRenderInventory(current);
        });
    }
    const renderRun = _loadActiveRun();
    if (renderRun) {
        _activeRenderRun = renderRun;
        startActiveRenderObserver();
    }
    const paletteRun = _loadActivePaletteRun();
    if (paletteRun) {
        _activePaletteRun = paletteRun;
        startActivePaletteObserver();
    }
})();

document.addEventListener('click', function(e) {
    const menu = document.getElementById('download-menu');
    const btn = document.getElementById('btn-render-download');
    if (menu && btn && !btn.contains(e.target) && !menu.contains(e.target)) {
        menu.style.display = 'none';
    }
});

window._dzViewportReadoutState = _dzViewportReadoutState;

document.addEventListener('keydown', function(e) {
    if (_isTextInputFocused()) return;
    const dzTab = document.getElementById('tab-deepzoom');
    if (!dzTab || !dzTab.classList.contains('active')) return;
    const inv = window._dzInventory || [];
    if (!inv.length) return;
    let idx = window._dzSelectedIdx ?? -1;
    if (e.key === 'ArrowDown') { e.preventDefault(); _dzSelect(Math.min(idx + 1, inv.length - 1)); }
    else if (e.key === 'ArrowUp') { e.preventDefault(); _dzSelect(Math.max(idx - 1, 0)); }
});

_syncRenderBackgroundColorUi();

populateDropdown();

document.addEventListener('keydown', function(e) {
    if (_isTextInputFocused()) return;
    const splatsTab = document.getElementById('tab-splats');
    if (!splatsTab || !splatsTab.classList.contains('active')) return;
    if (e.key === 'ArrowDown') { e.preventDefault(); _splatsTabMove(1); }
    else if (e.key === 'ArrowUp') { e.preventDefault(); _splatsTabMove(-1); }
    else if (e.key === 'Enter') { e.preventDefault(); _splatsTabOpenSelected(); }
});

document.addEventListener('keydown', function(e) {
    if (_isTextInputFocused()) return;
    const favTab = document.getElementById('tab-favorites');
    if (!favTab || !favTab.classList.contains('active')) return;
    const inv = _favoriteArtifacts || [];
    if (!inv.length) return;
    let idx = _favoriteSelectedIdx ?? -1;
    if (e.key === 'ArrowDown') { e.preventDefault(); _favoriteSelect(Math.min(idx + 1, inv.length - 1)); }
    else if (e.key === 'ArrowUp') { e.preventDefault(); _favoriteSelect(Math.max(idx - 1, 0)); }
});

document.addEventListener('click', function(e) {
    const menu = document.getElementById('favorites-download-menu');
    const btn = document.getElementById('btn-favorites-download');
    if (menu && btn && !btn.contains(e.target) && !menu.contains(e.target)) menu.style.display = 'none';
});

document.addEventListener('keydown', (e) => {
    if (!document.getElementById('tab-results').classList.contains('active')) return;
    if (e.key !== 'ArrowUp' && e.key !== 'ArrowDown') return;
    if (_isTextInputFocused()) return;
    if (!_resultsCache.length) return;
    e.preventDefault();

    const idx = _selectedJobId ? _resultsCache.findIndex(r => r.job_id === _selectedJobId) : -1;
    let next;
    if (e.key === 'ArrowDown') {
        next = idx < _resultsCache.length - 1 ? idx + 1 : idx;
    } else {
        next = idx > 0 ? idx - 1 : 0;
    }
    selectResult(_resultsCache[next].job_id);
});

document.addEventListener('keydown', (e) => {
    if (!document.getElementById('tab-palette').classList.contains('active')) return;
    if (e.key !== 'ArrowUp' && e.key !== 'ArrowDown') return;
    if (_isTextInputFocused()) return;
    if (!_paletteInventory.length) return;
    e.preventDefault();

    const idx = _paletteSelectedIdx;
    let next;
    if (e.key === 'ArrowDown') next = idx < _paletteInventory.length - 1 ? idx + 1 : idx;
    else next = idx > 0 ? idx - 1 : 0;
    _paletteSelect(next);
});

document.addEventListener('keydown', (e) => {
    if (!document.getElementById('tab-render').classList.contains('active')) return;
    if (e.key !== 'ArrowUp' && e.key !== 'ArrowDown') return;
    if (_isTextInputFocused()) return;
    const inv = _renderArtifacts[_renderActiveFamily] || [];
    if (!inv.length) return;
    e.preventDefault();

    const idx = _renderSelectedArtifact[_renderActiveFamily];
    let next;
    if (e.key === 'ArrowDown') next = idx < inv.length - 1 ? idx + 1 : idx;
    else next = idx > 0 ? idx - 1 : 0;
    _renderSelectArtifact(_renderActiveFamily, next);
});

document.addEventListener('keydown', (e) => _sheetsArrowNav(e));
document.addEventListener('keydown', (e) => _sheetContextEscape(e));
document.addEventListener('click', (e) => _sheetContextDismissClick(e));

document.addEventListener('keydown', (e) => {
    if (!document.getElementById('tab-render').classList.contains('active')) return;
    if (e.key !== 'Escape') return;
    if (!_renderPreviewSelectionState.rect || !_renderPreviewSelectionState.artifactKey) return;
    _clearRenderPreviewSelection();
});

loadLambdaConfig();

document.addEventListener('click', function(e) {
    const popup = document.getElementById('config-popup');
    const btn = document.getElementById('btn-config-toggle');
    if (!popup || !popup._open) return;
    const target = e.target;
    if (popup.contains(target) || (btn && btn.contains(target))) return;
    _setConfigPopupOpen(false);
});

document.addEventListener('keydown', function(e) {
    if (e.key === 'Escape') _setConfigPopupOpen(false);
});


/* Statements relocated from later parts (round 2): listeners,
   window exports, sync calls — monolith tail order preserved. */
document.addEventListener('keydown', function(e) {
    if (e.key === 'Escape') {
        _clearRenderPreviewSelection();
        _clearRenderLoresPreviewSelection();
    }
});

;(window.__ppParts = window.__ppParts || []).push('12-deepzoom-boot');
