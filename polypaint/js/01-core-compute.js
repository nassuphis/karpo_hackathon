// PolyPaint 01-core-compute — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Deploy rewrites the script tags to build-versioned asset
// keys (assets/<BUILD_ID>/...), so a deploy flips atomically via index.html.
/* ---- Logging ---- */
function log(msg, cls='', target='compute-log') {
    const el = document.getElementById(target);
    if (!el) return;
    const d = document.createElement('div');
    d.className = 'log-entry ' + cls;
    d.textContent = `[${new Date().toLocaleTimeString()}] ${msg}`;
    el.prepend(d);
    // Cap at 500 entries
    while (el.children.length > 500) el.removeChild(el.lastChild);
}

const _loggedContractWarnings = new Set();

function _fmtContractDefault(value) {
    if (value === undefined) return 'undefined';
    if (typeof value === 'string') return value;
    try { return JSON.stringify(value); } catch (e) {}
    return String(value);
}

function _entryContractWarnings(entry) {
    if (!entry || typeof entry !== 'object') return [];
    const payload = entry.result_data && typeof entry.result_data === 'object' ? entry.result_data : entry;
    const warnings = Array.isArray(payload.contract_warnings) ? payload.contract_warnings : [];
    if (!warnings.length) return [];
    return warnings.map(w => ({
        task_id: entry.task_id || payload.task_id || '',
        phase: payload.phase || entry.phase || '',
        param: w && w.param ? String(w.param) : '',
        default_value: w && Object.prototype.hasOwnProperty.call(w, 'default') ? w.default : undefined,
    }));
}

function _logContractWarnings(entries, target='compute-log') {
    const list = Array.isArray(entries) ? entries : [entries];
    for (const entry of list) {
        for (const warning of _entryContractWarnings(entry)) {
            const dedupeKey = [
                target,
                warning.task_id || '',
                warning.phase || '',
                warning.param || '',
                _fmtContractDefault(warning.default_value),
            ].join('|');
            if (_loggedContractWarnings.has(dedupeKey)) continue;
            _loggedContractWarnings.add(dedupeKey);
            const ctx = [];
            if (warning.task_id) ctx.push(`task=${warning.task_id}`);
            if (warning.phase) ctx.push(`phase=${warning.phase}`);
            const suffix = ctx.length ? ` (${ctx.join(', ')})` : '';
            log(`Warning: parameter ${warning.param} missing, default ${_fmtContractDefault(warning.default_value)} used${suffix}`, '', target);
        }
    }
}

/* ---- Tabs ---- */
function switchTab(name) {
    document.querySelectorAll('.tab-btn').forEach(b => b.classList.toggle('active', b.textContent.toLowerCase() === name));
    document.querySelectorAll('.tab-content').forEach(t => t.classList.toggle('active', t.id === 'tab-' + name));
    if (name === 'results') loadResults();
    if (name === 'render') _maybeAutoRefreshRenderInventory();
    if (name === 'favorites') loadFavoritesInventory();
    if (name === 'palette') loadPaletteInventory();
    if (name === 'deepzoom') loadDeepZoomInventory();
}

/* ---- Results management ---- */
let _resultsCache = [];      // last fetched results
let _selectedJobId = null;   // currently selected job_id
let _resultsLoading = false;

function _clampResultsListWorkers(value) {
    const n = Math.round(Number(value));
    if (!Number.isFinite(n)) return 32;
    return Math.max(1, Math.min(64, n));
}

function _closeResultsRefreshPopup() {
    _resultsRefreshPopupState = {
        open: false,
        workers: _clampResultsListWorkers(_resultsRefreshPopupState.workers),
    };
    const overlay = document.getElementById('results-refresh-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _renderResultsRefreshPopup() {
    const overlay = document.getElementById('results-refresh-popup-overlay');
    const summaryEl = document.getElementById('results-refresh-popup-summary');
    const workersEl = document.getElementById('results-refresh-workers');
    const runBtn = document.getElementById('results-refresh-popup-run');
    if (!overlay || !summaryEl || !workersEl) return;
    if (!_resultsRefreshPopupState.open) {
        _closeResultsRefreshPopup();
        return;
    }
    const workers = _clampResultsListWorkers(_resultsRefreshPopupState.workers);
    _resultsRefreshPopupState.workers = workers;
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    workersEl.value = String(workers);
    summaryEl.textContent = `Refresh Results • workers=${workers} • timings shown after refresh come from AWS /list`;
    if (runBtn) runBtn.disabled = !!_resultsLoading;
}

function openResultsRefreshPopup() {
    _resultsRefreshPopupState = {
        open: true,
        workers: _clampResultsListWorkers(_resultsRefreshPopupState.workers || 32),
    };
    _renderResultsRefreshPopup();
    const workersEl = document.getElementById('results-refresh-workers');
    if (workersEl && typeof workersEl.focus === 'function') workersEl.focus();
}

async function loadResults(options = null) {
    if (_resultsLoading) return;
    _resultsLoading = true;
    const countEl = document.getElementById('results-count');
    const requestedWorkers = _clampResultsListWorkers(
        options && typeof options === 'object' && options.listWorkers != null
            ? options.listWorkers
            : _resultsRefreshPopupState.workers
    );
    _resultsRefreshPopupState.workers = requestedWorkers;
    countEl.textContent = 'Loading...';
    log(`Results refresh: loading... workers=${requestedWorkers}`, '', 'results-log');

    try {
        const data = await lambdaPost('storage', { list_workers: requestedWorkers }, '/list');
        _resultsCache = data.results || [];
        renderResultsTable();
        countEl.textContent = `${data.count} results (${(data.list_us/1e6).toFixed(1)}s)`;
        const fmtUs = (us) => _fmtMs((Number(us) || 0) / 1000);
        const parts = [
            `Results refresh: ${data.count || 0} jobs in ${fmtUs(data.list_us)}`
        ];
        const detail = [];
        if (data.prefix_list_us != null) detail.push(`prefix ${fmtUs(data.prefix_list_us)}`);
        if (data.calc_fetch_us != null) detail.push(`calc ${fmtUs(data.calc_fetch_us)}`);
        if (data.sort_us != null) detail.push(`sort ${fmtUs(data.sort_us)}`);
        if (detail.length) parts.push(detail.join(' + '));
        const tune = [];
        if (data.list_workers != null) tune.push(`workers=${data.list_workers}`);
        if (data.s3_pool_connections != null) tune.push(`pool=${data.s3_pool_connections}`);
        if (tune.length) parts.push(tune.join(' '));
        log(parts.join(' · '), 'ok', 'results-log');
    } catch (e) {
        countEl.textContent = 'Error: ' + e.message;
        log('Results refresh failed: ' + e.message, 'err', 'results-log');
    } finally {
        _resultsLoading = false;
    }
}

async function _getResultDetail(jobId) {
    const r = _resultsCache.find(x => x.job_id === jobId) || null;
    let detail = r && r._detail;
    if (!detail) {
        detail = await lambdaPost('storage', { job_id: jobId }, '/detail');
        if (r) r._detail = detail;
    }
    return detail || {};
}

async function _ensureResultsSelection(jobId) {
    if (!jobId) throw new Error('No result selected');
    const hasJob = _resultsCache.some(r => r.job_id === jobId);
    if (!hasJob) await loadResults();
    const found = _resultsCache.some(r => r.job_id === jobId);
    if (!found) throw new Error(`Result ${jobId} not found`);
    selectResult(jobId);
}

function _favoriteRefKey(jobId, artifactId) {
    return `${jobId}::${artifactId}`;
}

function _isFavorite(jobId, artifactId) {
    if (!jobId || !artifactId) return false;
    const key = _favoriteRefKey(jobId, artifactId);
    return (_favoriteRefs || []).some(ref => _favoriteRefKey(ref.job_id, ref.artifact_id) === key);
}

async function _loadFavoriteRefs(force = false) {
    if (_favoriteRefsLoaded && !force) return _favoriteRefs;
    const resp = await lambdaPost('storage', {}, '/list-favorites');
    _favoriteRefs = Array.isArray(resp.favorites) ? resp.favorites : [];
    _favoriteRefsLoaded = true;
    return _favoriteRefs;
}

async function _hydrateFavoriteArtifacts() {
    const refs = Array.isArray(_favoriteRefs) ? _favoriteRefs.slice() : [];
    if (!refs.length) return [];
    const grouped = new Map();
    refs.forEach(ref => {
        if (!ref || !ref.job_id) return;
        if (!grouped.has(ref.job_id)) grouped.set(ref.job_id, []);
        grouped.get(ref.job_id).push(ref);
    });
    const summaries = new Map();
    await asyncPool(4, Array.from(grouped.keys()), async (jobId) => {
        try {
            const summary = await lambdaPost('storage', { job_id: jobId }, '/render-summary');
            summaries.set(jobId, summary || {});
        } catch (e) {
            summaries.set(jobId, { _favorite_error: e.message });
        }
    });
    return refs.map(ref => {
        const summary = summaries.get(ref.job_id) || {};
        const inv = (summary.families && summary.families.color) || [];
        const match = inv.find(a => a.artifact_id === ref.artifact_id) || null;
        if (match) {
            return {
                ...match,
                favorite_ref: ref,
                favorite_job_id: ref.job_id,
                favorite_added_at: ref.added_at || '',
            };
        }
        return {
            family: 'color',
            artifact_id: ref.artifact_id,
            favorite_ref: ref,
            favorite_job_id: ref.job_id,
            favorite_added_at: ref.added_at || '',
            display_name: ref.display_name || ref.artifact_id,
            image_key: ref.image_key || '',
            preview_key: ref.preview_key || '',
            image_url: '',
            viewer_url: '',
            preview_url: '',
            width: 0,
            height: 0,
            file_size: 0,
            missing: true,
            missing_reason: summary._favorite_error || 'missing artifact',
        };
    });
}

async function favoriteSelectedRenderArtifact() {
    if (_renderActiveFamily !== 'color') return;
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    if (!jobId || !art || !art.artifact_id) return;
    if (_isFavorite(jobId, art.artifact_id)) {
        log(`Already in favorites: ${art.artifact_id}`, 'ok', 'render-log');
        _updateRenderActionButtons();
        return;
    }
    const btn = document.getElementById('btn-render-favorite');
    const orig = btn ? btn.textContent : 'Favorite';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Saving...'; }
        const resp = await lambdaPost('storage', {
            job_id: jobId,
            artifact_id: art.artifact_id,
            family: 'color',
            display_name: art.display_name || art.artifact_id,
            image_key: art.image_key || '',
            preview_key: art.preview_key || '',
        }, '/add-favorite');
        _favoriteRefs = Array.isArray(resp.favorites) ? resp.favorites : _favoriteRefs;
        _favoriteRefsLoaded = true;
        log(`Favorited: ${art.artifact_id}`, 'ok', 'render-log');
        _updateRenderActionButtons();
        if (document.getElementById('tab-favorites')?.classList.contains('active')) {
            await loadFavoritesInventory();
        }
    } catch (e) {
        log(`Favorite failed: ${e.message}`, 'err', 'render-log');
    } finally {
        if (btn) btn.textContent = orig;
        _updateRenderActionButtons();
    }
}

function _favoriteSelectedEntry() {
    if (_favoriteSelectedIdx == null || _favoriteSelectedIdx < 0 || _favoriteSelectedIdx >= _favoriteArtifacts.length) return null;
    return _favoriteArtifacts[_favoriteSelectedIdx] || null;
}

function _encodeStableRowKey(value) {
    return encodeURIComponent(String(value || ''));
}

function _decodeStableRowKey(value) {
    try {
        return decodeURIComponent(String(value || ''));
    } catch (_) {
        return String(value || '');
    }
}

function _favoriteArtifactKey(art) {
    if (!art) return '';
    return `${String(art.favorite_job_id || art.job_id || '')}::${String(art.artifact_id || art.favorite_artifact_id || art.image_key || '')}`;
}

function _favoriteSelectKey(key) {
    const nextKey = String(key || '');
    const idx = _favoriteArtifacts.findIndex((art) => _favoriteArtifactKey(art) === nextKey);
    if (idx < 0) return;
    _captureFavoritesCatalogScroll();
    _favoriteSelectedIdx = idx;
    _favoriteSelectedKey = nextKey;
    renderFavoritesPanel({ preserveScroll: true, ensureSelected: true });
}

function _favoriteSelectEncodedKey(encodedKey) {
    _favoriteSelectKey(_decodeStableRowKey(encodedKey));
}

function _favoriteArtifactSummary(art) {
    if (!art) return '';
    if (art.missing) return 'missing artifact';
    return _renderArtifactSummary(art);
}

function _favoriteArtifactFilename(art) {
    if (!art) return 'favorite_artifact';
    const ext = (art.image_key || '').split('.').pop() || 'bin';
    return `${art.favorite_job_id || 'job'}_${art.artifact_id || 'favorite'}.${ext}`;
}

function _favoriteRowElementId(idx) {
    return `favorite-art-row-${idx}`;
}

function _captureFavoritesCatalogScroll() {
    const catalog = document.getElementById('favorites-artifact-catalog');
    if (!catalog) return;
    const top = Number(catalog.scrollTop);
    if (Number.isFinite(top)) _favoriteCatalogScrollTop = top;
}

function _restoreFavoritesCatalogScroll(options = {}) {
    const catalog = document.getElementById('favorites-artifact-catalog');
    if (!catalog) return;
    const top = Number(_favoriteCatalogScrollTop);
    if (Number.isFinite(top)) catalog.scrollTop = top;
    catalog.onscroll = () => {
        const nextTop = Number(catalog.scrollTop);
        if (Number.isFinite(nextTop)) _favoriteCatalogScrollTop = nextTop;
    };
    if (!options.ensureSelected) return;
    if (_favoriteSelectedIdx == null || _favoriteSelectedIdx < 0) return;
    const row = document.getElementById(_favoriteRowElementId(_favoriteSelectedIdx));
    if (row && typeof row.scrollIntoView === 'function') row.scrollIntoView({ block: 'nearest' });
}

function _updateFavoritesActionButtons() {
    const art = _favoriteSelectedEntry();
    const hasSelection = !!art;
    const goBtn = document.getElementById('btn-favorites-go-render');
    const dlBtn = document.getElementById('btn-favorites-download');
    const delBtn = document.getElementById('btn-favorites-delete');
    if (goBtn) goBtn.disabled = !(hasSelection && art.favorite_job_id);
    if (dlBtn) dlBtn.disabled = !(hasSelection && !art.missing && art.image_key);
    if (delBtn) delBtn.disabled = !hasSelection;
}

function _favoriteSelect(idx) {
    if (idx < 0 || idx >= _favoriteArtifacts.length) return;
    _favoriteSelectKey(_favoriteArtifactKey(_favoriteArtifacts[idx]));
}

function renderFavoritesPanel(options = {}) {
    const preview = document.getElementById('favorites-preview');
    const info = document.getElementById('favorites-info');
    if (!preview || !info) return;
    const inv = _favoriteArtifacts || [];
    if (!inv.length) {
        _favoriteSelectedIdx = -1;
        _favoriteSelectedKey = '';
        preview.innerHTML = '<div style="border:1px solid #333; border-radius:6px; padding:10px; background:#141424; color:#666">No favorite Color artifacts yet.</div>';
        info.textContent = 'Favorite Color artifacts across all jobs.';
        _updateFavoritesActionButtons();
        return;
    }
    const selectedIdxByKey = _favoriteSelectedKey
        ? inv.findIndex((art) => _favoriteArtifactKey(art) === _favoriteSelectedKey)
        : -1;
    if (selectedIdxByKey >= 0) _favoriteSelectedIdx = selectedIdxByKey;
    if (_favoriteSelectedIdx == null || _favoriteSelectedIdx < 0 || _favoriteSelectedIdx >= inv.length) _favoriteSelectedIdx = 0;
    _favoriteSelectedKey = _favoriteArtifactKey(inv[_favoriteSelectedIdx]);
    const activeArt = _favoriteSelectedEntry();
    let catalogHtml = '<table style="width:100%; border-collapse:collapse"><tr style="border-bottom:1px solid #333; position:sticky; top:0; background:#1a1a2e"><th style="text-align:left;padding:4px">Added</th><th style="text-align:left;padding:4px">Job</th><th>Dims</th><th>Size</th><th style="text-align:left">Summary</th></tr>';
    inv.forEach((art, i) => {
        const isSel = i === _favoriteSelectedIdx;
        const dims = art.width && art.height ? `${art.width}x${art.height}` : '';
        const added = (art.favorite_added_at || '').replace('T', ' ').slice(0, 19);
        const key = _encodeStableRowKey(_favoriteArtifactKey(art));
        catalogHtml += `<tr id="${_favoriteRowElementId(i)}" class="favorite-art-row" data-key="${key}" onclick="_favoriteSelectEncodedKey(this.dataset.key || '')" style="border-bottom:1px solid #222; cursor:pointer; background:${isSel ? '#2a2a4e' : ''}">`;
        catalogHtml += `<td style="padding:4px; font-size:11px">${added}</td>`;
        catalogHtml += `<td style="padding:4px; font-family:monospace; font-size:11px">${art.favorite_job_id || ''}</td>`;
        catalogHtml += `<td style="padding:4px; text-align:center">${dims}</td>`;
        catalogHtml += `<td style="padding:4px; text-align:center">${_fmtSize(art.file_size || art.size || 0)}</td>`;
        catalogHtml += `<td style="padding:4px">${_favoriteArtifactSummary(art)}</td>`;
        catalogHtml += '</tr>';
    });
    catalogHtml += '</table>';

    let viewerHtml = '<div style="color:#444; font-size:12px; padding:12px 0; text-align:center">No artifact selected</div>';
    if (activeArt) {
        if (activeArt.missing) viewerHtml = `<div style="color:#888; font-size:12px; padding:12px 0; text-align:center">Missing artifact<br><span style="color:#555">${activeArt.missing_reason || ''}</span></div>`;
        else if (activeArt.viewer_url) viewerHtml = `<img src="${activeArt.viewer_url}" style="max-width:100%; max-height:100%; background:#000; display:block; margin:0 auto">`;
    }

    preview.innerHTML = `
        <div style="border:1px solid #333; border-radius:6px; padding:10px; background:#141424">
            <div style="display:grid; grid-template-columns:minmax(340px, 44%) 1fr; gap:12px; align-items:start">
                <div id="favorites-artifact-catalog" style="height:360px; max-height:360px; overflow-y:auto; border:1px solid #333; border-radius:4px">${catalogHtml}</div>
                <div id="favorites-artifact-viewer" style="height:360px; max-height:360px; border:1px solid #333; border-radius:4px; background:#000; padding:8px; display:flex; align-items:center; justify-content:center; overflow:hidden">${viewerHtml}</div>
            </div>
        </div>
    `;
    info.textContent = `${inv.length} favorite${inv.length === 1 ? '' : 's'} loaded.`;
    _updateFavoritesActionButtons();
    if (options.preserveScroll) _restoreFavoritesCatalogScroll({ ensureSelected: !!options.ensureSelected });
    else {
        _favoriteCatalogScrollTop = 0;
        _restoreFavoritesCatalogScroll({ ensureSelected: false });
    }
}

async function loadFavoritesInventory() {
    const statusEl = document.getElementById('favorites-status');
    if (statusEl) {
        statusEl.textContent = 'Loading...';
        statusEl.className = 'status';
    }
    try {
        if (!_favoriteRefsLoaded) await _loadFavoriteRefs();
        _favoriteArtifacts = await _hydrateFavoriteArtifacts();
        renderFavoritesPanel();
        if (statusEl) {
            statusEl.textContent = 'Ready';
            statusEl.className = 'status ok';
        }
    } catch (e) {
        _favoriteArtifacts = [];
        _favoriteSelectedIdx = -1;
        _favoriteSelectedKey = '';
        renderFavoritesPanel();
        if (statusEl) {
            statusEl.textContent = 'Refresh failed: ' + e.message;
            statusEl.className = 'status error';
        }
    }
}

async function refreshFavoritesInventory() {
    const btn = document.getElementById('btn-favorites-refresh');
    const orig = btn ? btn.textContent : 'Refresh';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Refreshing...'; }
        await _loadFavoriteRefs(true);
        await loadFavoritesInventory();
        log('Favorites refreshed', 'ok', 'favorites-log');
    } catch (e) {
        log(`Favorites refresh failed: ${e.message}`, 'err', 'favorites-log');
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

async function goRenderSelectedFavorite() {
    const art = _favoriteSelectedEntry();
    if (!art || !art.favorite_job_id) return;
    const btn = document.getElementById('btn-favorites-go-render');
    const orig = btn ? btn.textContent : 'GoRender';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Opening...'; }
        await _ensureResultsSelection(art.favorite_job_id);
        switchTab('render');
        await refreshRenderArtifacts(art.favorite_job_id, {
            selectFamily: 'color',
            selectArtifactId: art.artifact_id || null,
        });
        const inv = (_renderArtifacts && _renderArtifacts.color) || [];
        const found = inv.some(entry => entry && entry.artifact_id === art.artifact_id);
        if (found) log(`GoRender selected: ${art.artifact_id}`, 'ok', 'favorites-log');
        else log(`Selected artifact not in render table: ${art.artifact_id}`, '', 'favorites-log');
    } catch (e) {
        log(`GoRender failed: ${e.message}`, 'err', 'favorites-log');
    } finally {
        if (btn) btn.textContent = orig;
        _updateFavoritesActionButtons();
    }
}

// Arrow key navigation for favorites inventory
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

function _toggleFavoritesDownloadMenu() {
    const menu = document.getElementById('favorites-download-menu');
    if (!menu) return;
    const show = menu.style.display === 'none' || !menu.style.display;
    menu.style.display = show ? 'block' : 'none';
    if (show) {
        const dirItem = document.getElementById('favorites-dl-menu-dir');
        if (dirItem) {
            dirItem.textContent = _favoriteSaveDirHandle
                ? '\u2713 ' + (_favoriteSaveDirHandle.name || 'folder') + ' \u2014 change\u2026'
                : 'Select Dir\u2026';
        }
    }
}

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

async function _favoritesDlMenuAction(mode) {
    const menu = document.getElementById('favorites-download-menu');
    if (menu) menu.style.display = 'none';
    const art = _favoriteSelectedEntry();
    if (!art || art.missing || !art.image_key) return;
    if (mode === 'dir') {
        if (typeof window.showDirectoryPicker !== 'function') {
            alert('Directory picker requires Chrome or Edge over HTTPS (or localhost).');
            return;
        }
        try {
            _favoriteSaveDirHandle = await window.showDirectoryPicker({ mode: 'readwrite' });
            log('Favorites save directory: ' + _favoriteSaveDirHandle.name, 'ok', 'favorites-log');
        } catch (e) {
            if (e.name !== 'AbortError') alert('Directory picker failed: ' + e.message);
        }
        return;
    }
    const jobId = art.favorite_job_id;
    const imageFilename = _favoriteArtifactFilename(art);
    const metaFilename = imageFilename.replace(/\.[^.]+$/, '_meta.json');
    const metaObj = await _buildArtifactMeta(jobId, art, { imageFilename, paletteFilename: '' });
    const metaBlob = new Blob([JSON.stringify(metaObj, null, 2)], { type: 'application/json' });
    if (_favoriteSaveDirHandle) {
        const btn = document.getElementById('btn-favorites-download');
        const orig = btn ? btn.textContent : 'Download \u25bc';
        try {
            if (btn) btn.disabled = true;
            const dirHandle = _favoriteSaveDirHandle;
            if ((await dirHandle.queryPermission({ mode: 'readwrite' })) !== 'granted') {
                if ((await dirHandle.requestPermission({ mode: 'readwrite' })) !== 'granted') {
                    _favoriteSaveDirHandle = null;
                    throw new Error('Permission denied — directory cleared');
                }
            }
            if (btn) btn.textContent = 'Saving metadata...';
            const metaHandle = await dirHandle.getFileHandle(metaFilename, { create: true });
            const metaWritable = await metaHandle.createWritable();
            await metaWritable.write(metaBlob);
            await metaWritable.close();
            if (btn) btn.textContent = 'Downloading image...';
            const presign = await lambdaPost('storage', { key: art.image_key, filename: imageFilename }, '/presign');
            const resp = await fetch(presign.url);
            if (!resp.ok) throw new Error(`Image download failed: ${resp.status}`);
            const imageBlob = await resp.blob();
            const imgHandle = await dirHandle.getFileHandle(imageFilename, { create: true });
            const imgWritable = await imgHandle.createWritable();
            await imgWritable.write(imageBlob);
            await imgWritable.close();
            if (btn) { btn.textContent = 'Saved to ' + dirHandle.name; setTimeout(() => { btn.textContent = orig; }, 2000); }
        } catch (e) {
            if (e.name !== 'AbortError') alert('Save failed: ' + e.message);
        } finally {
            if (btn) btn.disabled = false;
        }
        return;
    }
    const metaUrl = URL.createObjectURL(metaBlob);
    const a = document.createElement('a');
    a.href = metaUrl;
    a.download = metaFilename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(metaUrl);
    await downloadPresignedFile(art.image_url || art.viewer_url || '', imageFilename, art.image_key);
}

async function deleteSelectedFavorite() {
    const art = _favoriteSelectedEntry();
    if (!art) return;
    if (!confirm(`Remove favorite reference for ${art.artifact_id || ''}?`)) return;
    const btn = document.getElementById('btn-favorites-delete');
    const orig = btn ? btn.textContent : 'Delete';
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Deleting...'; }
        const resp = await lambdaPost('storage', {
            job_id: art.favorite_job_id,
            artifact_id: art.artifact_id,
        }, '/delete-favorite');
        _favoriteRefs = Array.isArray(resp.favorites) ? resp.favorites : _favoriteRefs.filter(ref => _favoriteRefKey(ref.job_id, ref.artifact_id) !== _favoriteRefKey(art.favorite_job_id, art.artifact_id));
        _favoriteRefsLoaded = true;
        const nextIdx = _favoriteSelectedIdx;
        _favoriteArtifacts = await _hydrateFavoriteArtifacts();
        _favoriteSelectedIdx = _favoriteArtifacts.length ? Math.min(nextIdx, _favoriteArtifacts.length - 1) : -1;
        _favoriteSelectedKey = _favoriteSelectedIdx >= 0 ? _favoriteArtifactKey(_favoriteArtifacts[_favoriteSelectedIdx]) : '';
        renderFavoritesPanel({ preserveScroll: true, ensureSelected: true });
        log(`Favorite removed: ${art.artifact_id}`, 'ok', 'favorites-log');
        _updateRenderActionButtons();
    } catch (e) {
        log(`Favorite delete failed: ${e.message}`, 'err', 'favorites-log');
    } finally {
        if (btn) btn.textContent = orig;
        _updateFavoritesActionButtons();
    }
}

let _resultsSortCol = null;  // null | 'function' | 'degree' | 'N' | 'times' | 'total_size'
let _resultsSortDir = null;  // null | 'asc' | 'desc'

function _resultsFilterMode() {
    const el = document.getElementById('results-filter-mode');
    const value = (el && el.value) ? String(el.value) : 'function';
    return value === 'job_id' ? 'job_id' : 'function';
}

function _updateResultsFilterUi() {
    const mode = _resultsFilterMode();
    const filterEl = document.getElementById('results-filter');
    if (!filterEl) return;
    filterEl.placeholder = mode === 'job_id' ? 'Filter by job id...' : 'Filter by function...';
}

function _resultsToggleSort(col) {
    if (_resultsSortCol === col) {
        if (_resultsSortDir === 'asc') _resultsSortDir = 'desc';
        else if (_resultsSortDir === 'desc') { _resultsSortCol = null; _resultsSortDir = null; }
        else { _resultsSortDir = 'asc'; }
    } else {
        _resultsSortCol = col;
        _resultsSortDir = 'asc';
    }
    // Update sort indicators
    for (const c of ['function', 'degree', 'N', 'times', 'total_size']) {
        const el = document.getElementById('results-sort-' + c);
        if (el) el.textContent = _resultsSortCol === c ? (_resultsSortDir === 'asc' ? '\u25b2' : '\u25bc') : '';
    }
    renderResultsTable();
}

function renderResultsTable() {
    const tbody = document.getElementById('results-tbody');
    tbody.innerHTML = '';
    const fmtSize = (b) => b > 1e6 ? (b/1e6).toFixed(1)+'M' : (b/1e3).toFixed(0)+'K';

    // Filter
    const filterEl = document.getElementById('results-filter');
    const filter = (filterEl ? filterEl.value.trim().toLowerCase() : '');
    const filterMode = _resultsFilterMode();
    let rows = _resultsCache;
    if (filter) {
        if (filterMode === 'job_id') {
            rows = rows.filter(r => {
                const full = String(r.job_id || '').toLowerCase();
                const short = full.replace(/^compute_/, '');
                return full.includes(filter) || short.includes(filter);
            });
        } else {
            rows = rows.filter(r => (r.function || '').toLowerCase().includes(filter));
        }
    }

    // Sort
    if (_resultsSortCol && _resultsSortDir) {
        const col = _resultsSortCol;
        const dir = _resultsSortDir === 'asc' ? 1 : -1;
        rows = [...rows].sort((a, b) => {
            let va = col === 'function' ? (a[col] || '') : (Number(a[col]) || 0);
            let vb = col === 'function' ? (b[col] || '') : (Number(b[col]) || 0);
            if (col === 'N') { va = Number(a.N || a.n1 || 0); vb = Number(b.N || b.n1 || 0); }
            if (col === 'times') { va = Number(a.times || 1); vb = Number(b.times || 1); }
            if (typeof va === 'string') return dir * va.localeCompare(vb);
            return dir * (va - vb);
        });
    }

    for (const r of rows) {
        const tr = document.createElement('tr');
        tr.dataset.jobId = r.job_id;
        if (r.job_id === _selectedJobId) tr.className = 'selected';
        tr.onclick = () => selectResult(r.job_id);

        const shortId = r.job_id.replace(/^compute_/, '');

        tr.innerHTML =
            `<td style="color:#e0e0e0">${shortId}</td>` +
            `<td style="color:#4ecca3">${r.function || '?'}</td>` +
            `<td style="text-align:right">${r.degree || ''}</td>` +
            `<td style="text-align:right">${r.N || r.n1 || ''}</td>` +
            `<td style="text-align:right">${r.times || 1}</td>` +
            `<td style="text-align:right;color:#888">${fmtSize(r.total_size || 0)}</td>`;
        tbody.appendChild(tr);
    }
}

function selectResult(jobId) {
    _selectedJobId = jobId;

    // Update selection highlight
    const tbody = document.getElementById('results-tbody');
    const rows = Array.from((tbody && tbody.children) || []);
    let targetRow = null;
    for (const row of rows) {
        const isTarget = (row.dataset && row.dataset.jobId) === jobId;
        row.className = isTarget ? 'selected' : '';
        if (isTarget) targetRow = row;
    }
    if (targetRow) {
        targetRow.scrollIntoView({ block: 'nearest' });
    }

    // Focus the scroll container so arrow keys work
    document.getElementById('results-scroll').focus();

    // Enable buttons
    document.getElementById('btn-populate-result').disabled = false;
    document.getElementById('btn-preview').disabled = false;
    document.getElementById('btn-render-result').disabled = false;
    document.getElementById('btn-delete').disabled = false;
    // Migrate enables only after /detail reveals a legacy (v1) run.
    document.getElementById('btn-migrate-result').disabled = true;

    // Populate render-results-dir for easy render access
    _setRenderResultsJob(jobId);
    document.getElementById('palette-results-dir').value = jobId;
    document.getElementById('results-dir').value = jobId;

    const r = _resultsCache.find(r => r.job_id === jobId);
    const previewEl = document.getElementById('results-preview');
    const infoEl = document.getElementById('results-info');

    // Show defaults while /detail loads
    _setPreviewPlaceholder(previewEl, 'Loading...');
    document.getElementById('res-version').textContent = '-';
    document.getElementById('res-cfun').textContent = r ? (r.function || '?') : '?';
    document.getElementById('res-degree').textContent = r ? (r.degree || '-') : '-';
    document.getElementById('res-stripes').textContent = r ? (r.n_chunks || r.n_stripes || '-') : '-';
    document.getElementById('res-nroots').textContent = r && r.total_roots ? r.total_roots.toLocaleString() : '-';
    document.getElementById('res-pform').textContent = '-';
    document.getElementById('res-cfpv').textContent = '-';
    document.getElementById('res-cform').textContent = '-';
    document.getElementById('res-times').textContent = '-';
    document.getElementById('res-solver').textContent = '-';
    document.getElementById('res-prev-total').textContent = '-';
    document.getElementById('res-prev-good').textContent = '-';
    document.getElementById('res-ll').textContent = '-';
    document.getElementById('res-ur').textContent = '-';
    infoEl.textContent = 'loading...';

    // Use cached detail if available, otherwise fetch
    if (r && r._detail) {
        _applyDetail(r, r._detail, previewEl, infoEl, jobId);
    } else {
        lambdaPost('storage', { job_id: jobId }, '/detail').then(detail => {
            if (_selectedJobId !== jobId) return;
            if (r) r._detail = detail;
            _applyDetail(r, detail, previewEl, infoEl, jobId);
        }).catch(e => {
            if (_selectedJobId === jobId) {
                infoEl.textContent = `detail error: ${e.message}`;
                _setPreviewPlaceholder(previewEl, 'No preview');
            }
        });
    }
}

async function migrateResult() {
    if (!_selectedJobId) return;
    const jobId = _selectedJobId;
    const infoEl = document.getElementById('results-info');
    const btn = document.getElementById('btn-migrate-result');
    const orig = btn ? btn.textContent : 'Migrate v2';
    if (btn) { btn.disabled = true; btn.textContent = '...'; }
    try {
        if (infoEl) infoEl.textContent = 'Migrating...';
        const resp = await lambdaPost('storage', { job_id: jobId, dry_run: false }, '/migrate-compute');
        const v = (resp && resp.to_version) || 2;
        const msg = resp && resp.wrote ? `Migrated ${jobId} to v${v}` : `${jobId} already v${v}`;
        if (infoEl) infoEl.textContent = msg;
        log(msg, 'ok', 'results-log');
        // Invalidate cached detail so re-selection reflects the new version + button state.
        const r = _resultsCache.find(x => x.job_id === jobId);
        if (r) r._detail = null;
        selectResult(jobId);
    } catch (e) {
        if (infoEl) infoEl.textContent = `migrate error: ${e.message}`;
        log(`Migrate failed: ${e.message}`, 'err', 'results-log');
        if (btn) btn.disabled = false;
    } finally {
        if (btn) btn.textContent = orig;
    }
}

async function goResultFromRender() {
    const jobId = document.getElementById('render-results-dir')?.value.trim();
    if (!jobId) return;
    try {
        await _ensureResultsSelection(jobId);
        switchTab('results');
        log(`GoResult selected: ${jobId}`, 'ok', 'render-log');
    } catch (e) {
        log(`GoResult failed: ${e.message}`, 'err', 'render-log');
    }
}

async function goPaletteFromRender() {
    const jobId = document.getElementById('render-results-dir')?.value.trim();
    const art = _renderSelectedArtifactEntry();
    const paletteId = _linkedPaletteIdForColorArtifact(art);
    if (!jobId || !paletteId) return;
    try {
        const loadedIdx = (_renderArtifacts.palette || []).findIndex((item) =>
            item && (item.palette_id === paletteId || item.artifact_id === paletteId)
        );
        if (loadedIdx >= 0) {
            _renderSelectArtifact('palette', loadedIdx);
            log(`GoPalette selected: ${paletteId}`, 'ok', 'render-log');
            return;
        }
        await refreshRenderArtifacts(jobId, {
            selectFamily: 'palette',
            selectArtifactId: paletteId,
        });
        const selected = _renderSelectedArtifactEntry();
        if (_renderActiveFamily === 'palette' && selected && (selected.palette_id === paletteId || selected.artifact_id === paletteId)) {
            log(`GoPalette selected: ${paletteId}`, 'ok', 'render-log');
        } else {
            log(`GoPalette could not find ${paletteId} in Palette inventory`, 'err', 'render-log');
        }
    } catch (e) {
        log(`GoPalette failed: ${e.message}`, 'err', 'render-log');
    }
}

async function goColorFromPalette() {
    const jobId = document.getElementById('render-results-dir')?.value.trim();
    const art = _renderSelectedArtifactEntry();
    const colorId = _linkedColorIdForPaletteArtifact(art);
    if (!jobId || !colorId) return;
    try {
        const loadedIdx = (_renderArtifacts.color || []).findIndex((item) =>
            item && item.artifact_id === colorId
        );
        if (loadedIdx >= 0) {
            _renderSelectArtifact('color', loadedIdx);
            log(`GoColor selected: ${colorId}`, 'ok', 'render-log');
            return;
        }
        await refreshRenderArtifacts(jobId, {
            selectFamily: 'color',
            selectArtifactId: colorId,
        });
        const selected = _renderSelectedArtifactEntry();
        if (_renderActiveFamily === 'color' && selected && selected.artifact_id === colorId) {
            log(`GoColor selected: ${colorId}`, 'ok', 'render-log');
        } else {
            log(`GoColor could not find ${colorId} in Color inventory`, 'err', 'render-log');
        }
    } catch (e) {
        log(`GoColor failed: ${e.message}`, 'err', 'render-log');
    }
}

function _applyDetail(r, detail, previewEl, infoEl, jobId) {
    if (_selectedJobId !== jobId) return;

    // Preview
    if (detail.has_preview && detail.preview_url) {
        _setPreviewImage(previewEl, detail.preview_url);
    } else {
        _setPreviewPlaceholder(previewEl, 'No preview');
    }

    // Pipeline info
    const pt = detail.param_transforms_display || detail.param_transforms || [];
    const ct = detail.coeff_transforms || [];
    document.getElementById('res-pform').textContent = pt.length
        ? pt.map(a => a.length > 1 ? `${a[0]}(${a.slice(1).join(',')})` : a[0]).join(', ')
        : 'none';
    const pipeline = detail.pipeline || {};
    const resCfpv = pipeline.cfpv_display ? pipeline.cfpv_display
        : (pipeline.cfpv && pipeline.cfpv.length) ? (_formatCfpvForDisplay(pipeline.function || detail.function || detail.calc?.function || '', pipeline.cfpv) || pipeline.cfpv.join(', ')) : '-';
    document.getElementById('res-cfpv').textContent = resCfpv;
    document.getElementById('res-cform').textContent = ct.length
        ? ct.map(item => Array.isArray(item) && item.length > 1 ? `${item[0]}(${item.slice(1).join(',')})` : (Array.isArray(item) ? item[0] : item)).join(', ')
        : 'none';
    document.getElementById('res-times').textContent = detail.times || '-';
    const calc = detail.calc || {};
    const solver = calc.solver || detail.solver || '';
    document.getElementById('res-solver').textContent = _solverShortLabel(solver);

    // Program version + migrate availability (backend-computed; structural fallback)
    let version = detail.pipeline_program_version;
    if (version == null && detail.pipeline) {
        const pl = detail.pipeline;
        const hasLegacy = (pl.param_transforms && pl.param_transforms.length) || (pl.coeff_transforms && pl.coeff_transforms.length);
        version = hasLegacy ? 1 : 2;
    }
    const migratable = (detail.pipeline_migratable != null) ? !!detail.pipeline_migratable : (version === 1);
    document.getElementById('res-version').textContent = version ? (migratable ? `v${version} (legacy)` : `v${version}`) : '-';
    const migrateBtn = document.getElementById('btn-migrate-result');
    if (migrateBtn) migrateBtn.disabled = !migratable;

    // Preview stats from server or session cache
    const ps = detail.preview_stats || {};

    // Viewport — prefer session cache > preview_stats > compute view.json
    const qRe = (r && r.q_re) || ps.q_re || detail.compute_q_re;
    const qIm = (r && r.q_im) || ps.q_im || detail.compute_q_im;
    if (qRe && qIm) {
        document.getElementById('res-ll').textContent = `${qRe[0].toFixed(4)}, ${qIm[0].toFixed(4)}`;
        document.getElementById('res-ur').textContent = `${qRe[1].toFixed(4)}, ${qIm[1].toFixed(4)}`;
    }
    const prevTotal = (r && r._prevTotal != null) ? r._prevTotal : (ps.n_roots_total || null);
    const prevGood = (r && r._prevGood != null) ? r._prevGood : (ps.n_roots || null);
    if (prevTotal != null) document.getElementById('res-prev-total').textContent = prevTotal.toLocaleString();
    if (prevGood != null) document.getElementById('res-prev-good').textContent = prevGood.toLocaleString();

    // File count
    if (detail.file_count != null) {
        infoEl.textContent = `${detail.file_count} files`;
    }

    // Cache into results entry
    if (r) {
        r.file_count = detail.file_count;
        if (detail.compute_q_re) { r.q_re = detail.compute_q_re; r.q_im = detail.compute_q_im; }
    }
}

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

async function previewResult() {
    if (!_selectedJobId) return;
    const btn = document.getElementById('btn-preview');
    btn.disabled = true;
    btn.textContent = '...';
    const infoEl = document.getElementById('results-info');
    const t0 = performance.now();

    try {
        const quantile = parseFloat(document.getElementById('res-quantile').value) / 100;
        const shim = parseFloat(document.getElementById('res-shim').value) / 100;

        // Single Lambda call: viewport + raster + encode all in one
        infoEl.textContent = 'Generating preview...';
        const result = await lambdaPost('preview', {
            job_id: _selectedJobId, quantile, shim, preview_size: 256,
        });

        const totalMs = (performance.now() - t0).toFixed(0);

        // Update cache — clear stale _detail so next select refetches
        const r = _resultsCache.find(r => r.job_id === _selectedJobId);
        if (r) {
            delete r._detail;
            if (result.q_re) { r.q_re = result.q_re; r.q_im = result.q_im; }
            r._prevTotal = result.n_roots_total || 0;
            r._prevGood = result.n_roots || 0;
        }

        // Show preview
        document.getElementById('results-preview').innerHTML =
            `<img src="${result.image_url}" style="max-width:100%; max-height:100%; image-rendering:pixelated">`;
        document.getElementById('res-prev-total').textContent = (result.n_roots_total || 0).toLocaleString();
        document.getElementById('res-prev-good').textContent = result.n_roots.toLocaleString();
        infoEl.textContent = `preview ${(totalMs/1000).toFixed(1)}s`;

        // Update viewport bounds
        if (result.q_re && result.q_im) {
            document.getElementById('res-ll').textContent = `${result.q_re[0].toFixed(4)}, ${result.q_im[0].toFixed(4)}`;
            document.getElementById('res-ur').textContent = `${result.q_re[1].toFixed(4)}, ${result.q_im[1].toFixed(4)}`;
        }

        renderResultsTable();
    } catch (e) {
        infoEl.textContent = 'Preview error: ' + e.message;
    } finally {
        btn.disabled = false;
        btn.textContent = 'Preview';
    }
}

async function deleteResult() {
    if (!_selectedJobId) return;
    if (!confirm(`Delete all data for ${_selectedJobId}?`)) return;

    const btn = document.getElementById('btn-delete');
    btn.disabled = true;
    btn.textContent = '...';

    try {
        await lambdaPost('storage', { job_id: _selectedJobId }, '/delete');

        // Remove from cache and re-render
        _resultsCache = _resultsCache.filter(r => r.job_id !== _selectedJobId);
        _selectedJobId = null;
        renderResultsTable();

        document.getElementById('results-preview').innerHTML =
            '<span style="color:#444; font-size:11px">No preview</span>';
        document.getElementById('results-info').textContent = 'Deleted';
        document.getElementById('results-count').textContent = `${_resultsCache.length} results`;
        document.getElementById('btn-populate-result').disabled = true;
        document.getElementById('btn-preview').disabled = true;
        document.getElementById('btn-render-result').disabled = true;
        document.getElementById('btn-delete').disabled = true;
    } catch (e) {
        document.getElementById('results-info').textContent = 'Delete error: ' + e.message;
    } finally {
        btn.disabled = false;
        btn.textContent = 'Delete';
    }
}

function goRenderResult() {
    if (!_selectedJobId) return;
    _setRenderResultsJob(_selectedJobId);
    switchTab('render');
}

function _setChainFromSaved(which, values) {
    const normalized = Array.isArray(values) ? values : [];
    if (which === 'pp') {
        _ppChain = _normalizeParamProgramChain(normalized);
        _paramProgramSelectedIndex = _ppChain.length ? 0 : -1;
        _renderChips(which);
        return;
    }
    if (which === 'cp') {
        _coeffProgramChain = _normalizeCoeffProgramChain(normalized);
        _coeffProgramSelectedIndex = _coeffProgramChain.length ? 0 : -1;
        _renderChips(which);
        return;
    }
    const chain = normalized.map(item => {
        if (Array.isArray(item) && item.length) {
            return { name: String(item[0]), params: item.slice(1).map(v => String(v)) };
        }
        if (item == null || item === '') return null;
        return { name: String(item), params: [] };
    }).filter(Boolean);
    if (which === 'pt') _ptChain = chain;
    else if (which === 'ct') _ctChain = chain.map(_normalizeCoeffTransformItem);
    else if (which === 'rt') _rtChain = chain;
    else if (which === 'palette-rt') _paletteRtChain = chain;
    _renderChips(which);
}

function _solverShortLabel(solver) {
    return solver === 'companion_matrix' ? 'CM'
        : solver === 'aberth_mt' ? 'AE-MT'
        : solver === 'aberth' ? 'AE'
        : solver || '-';
}

function _normalizeComputeSolverMode(solver) {
    return solver === 'companion_matrix' ? 'companion_matrix' : 'aberth_mt';
}

function _solverRunLabel(solver) {
    return solver === 'companion_matrix' ? 'Calculate-CM'
        : 'Calculate-AE-MT';
}

function _solverButtonId(solver) {
    return solver === 'companion_matrix' ? 'btn-calculate-cm'
        : 'btn-calculate-mt';
}

function _solverTag(solver) {
    return solver === 'companion_matrix' ? 'CM'
        : 'AE-MT';
}

function _solverLoresEndpoint(solver) {
    return solver === 'companion_matrix' ? 'sweep-cm'
        : 'sweep-mt';
}

function _solverDispatchTarget(solver) {
    return solver === 'companion_matrix' ? 'sweep_cm'
        : 'sweep_mt';
}

function _computePopupPrefsForSolver(solverMode) {
    const key = _normalizeComputeSolverMode(solverMode);
    if (!_computePopupPrefsBySolver[key]) {
        _computePopupPrefsBySolver[key] = _defaultComputePopupPrefs(key);
    }
    return _computePopupPrefsBySolver[key];
}

function _solverHasThreadedFusedSolve(solverMode) {
    return solverMode === 'aberth_mt';
}

function _computePopupSharedThreadsLabel(solverMode) {
    return _solverHasThreadedFusedSolve(solverMode)
        ? 'Shared hires threads'
        : 'Shared hires generation threads';
}

let _computePreviewSignature = '';
let _computePreviewIsStale = false;

function _computePreviewBox() {
    return document.getElementById('compute-preview-box');
}

function _computePreviewStatusEl() {
    return document.getElementById('compute-preview-status');
}

function _computePreviewInfoEl() {
    return document.getElementById('compute-preview-info');
}

function _computeDebugStatusEl() {
    return document.getElementById('compute-debug-status');
}

function _computeDebugOutputEl() {
    return document.getElementById('compute-debug-output');
}

function _setComputePreviewTab(tab) {
    const normalized = tab === 'debug' ? 'debug' : 'plot';
    const plotPanel = document.getElementById('compute-preview-plot-panel');
    const debugPanel = document.getElementById('compute-preview-debug-panel');
    const plotTab = document.getElementById('compute-preview-tab-plot');
    const debugTab = document.getElementById('compute-preview-tab-debug');
    if (plotPanel && plotPanel.classList) plotPanel.classList.toggle('active', normalized === 'plot');
    if (debugPanel && debugPanel.classList) debugPanel.classList.toggle('active', normalized === 'debug');
    if (plotTab && plotTab.classList) plotTab.classList.toggle('active', normalized === 'plot');
    if (debugTab && debugTab.classList) debugTab.classList.toggle('active', normalized === 'debug');
}

function _computePreviewHasImage() {
    const box = _computePreviewBox();
    return !!(box && box.dataset && box.dataset.hasImage === '1');
}

;(window.__ppParts = window.__ppParts || []).push('01-core-compute');
