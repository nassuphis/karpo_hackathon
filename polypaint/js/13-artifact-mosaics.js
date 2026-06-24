// PolyPaint 13-artifact-mosaics — single-level OpenSeadragon walls for artifacts.
// Classic script: load order matters and is defined by index.html.
const ARTIFACT_MOSAIC_POLL_MS = 2000;
const ARTIFACT_MOSAIC_MAX_LOG_LINES = 80;
const ARTIFACT_MOSAIC_TRANSPARENT_TILE = 'data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///ywAAAAAAQABAAACAUwAOw==';

const ARTIFACT_MOSAICS = {
    color: {
        tabName: 'allcol',
        label: 'AllCol',
        statusPath: '/list-color-mosaic',
        family: 'color',
        fixedSizes: ['512', '1024'],
        selectArtifactId(tile) { return tile && tile.artifact_id; },
    },
    palette: {
        tabName: 'allpal',
        label: 'AllPal',
        statusPath: '/list-palette-mosaic',
        family: 'palette',
        fixedSizes: [],
        selectArtifactId(tile) { return tile && (tile.palette_id || tile.artifact_id); },
    },
};

const _artifactMosaicState = {
    color: _newArtifactMosaicState(),
    palette: _newArtifactMosaicState(),
};

function _newArtifactMosaicState() {
    return {
        viewer: null,
        status: null,
        manifest: null,
        tiles: [],
        pollTimer: null,
        loading: false,
        randomSeed: 1,
        lastRenderSignature: '',
        activeTileSource: null,
        lastLogSignature: '',
    };
}

function _mosaicConfig(kind) {
    return ARTIFACT_MOSAICS[kind] || ARTIFACT_MOSAICS.color;
}

function _mosaicState(kind) {
    return _artifactMosaicState[kind] || _artifactMosaicState.color;
}

function _mosaicEl(kind, suffix) {
    return document.getElementById(`${_mosaicConfig(kind).tabName}-${suffix}`);
}

function _mosaicStatusEl(kind) {
    return _mosaicEl(kind, 'status');
}

function _mosaicSummaryEl(kind) {
    return _mosaicEl(kind, 'summary');
}

function _setMosaicStatus(kind, text, cls = '') {
    const el = _mosaicStatusEl(kind);
    if (!el) return;
    el.textContent = text;
    el.className = cls ? `status ${cls}` : 'status';
}

function _setMosaicRefreshBusy(kind, busy) {
    const btn = document.getElementById(`btn-${_mosaicConfig(kind).tabName}-refresh`);
    if (!btn) return;
    btn.disabled = !!busy;
    btn.textContent = busy ? 'Refreshing...' : 'Refresh';
}

function _mosaicInt(value) {
    const n = Number(value);
    return Number.isFinite(n) ? Math.max(0, Math.floor(n)) : 0;
}

function _mosaicProgressText(kind, status) {
    status = status || {};
    const message = String(status.progress_message || '').trim();
    const jobsDone = _mosaicInt(status.progress_jobs_done);
    const jobsTotal = _mosaicInt(status.progress_jobs_total);
    const artifactsDone = _mosaicInt(status.progress_artifacts_done);
    const artifactsTotal = _mosaicInt(status.progress_artifacts_total);
    const tiles = _mosaicInt(status.progress_tiles);
    const lastJob = String(status.progress_last_job || '').trim();
    const parts = [];
    if (jobsTotal) parts.push(`jobs ${jobsDone.toLocaleString()}/${jobsTotal.toLocaleString()}`);
    if (artifactsTotal) parts.push(`artifacts ${artifactsDone.toLocaleString()}/${artifactsTotal.toLocaleString()}`);
    if (tiles) parts.push(`${tiles.toLocaleString()} tiles`);
    if (lastJob) parts.push(lastJob);
    const detail = parts.length ? ` · ${parts.join(' · ')}` : '';
    return `${message || `Computing ${_mosaicConfig(kind).label} mosaic`}${detail}`;
}

function _logMosaic(kind, message, cls = '', signature = '') {
    const state = _mosaicState(kind);
    const cfg = _mosaicConfig(kind);
    const sig = signature || message;
    if (state.lastLogSignature === sig) return;
    state.lastLogSignature = sig;
    const target = `${cfg.tabName}-log`;
    if (typeof log === 'function') {
        log(message, cls, target);
        return;
    }
    const el = document.getElementById(target);
    if (el) {
        const lines = (el.textContent || '').split('\n').filter(Boolean);
        lines.unshift(`[${new Date().toLocaleTimeString()}] ${message}`);
        el.textContent = lines.slice(0, ARTIFACT_MOSAIC_MAX_LOG_LINES).join('\n') + '\n';
    }
}

function _logMosaicProgress(kind, status) {
    status = status || {};
    const signature = [
        status.refresh_id || '',
        status.progress_stage || '',
        status.progress_jobs_done || 0,
        status.progress_jobs_total || 0,
        status.progress_artifacts_done || 0,
        status.progress_artifacts_total || 0,
        status.progress_tiles || 0,
    ].join('|');
    _logMosaic(kind, _mosaicProgressText(kind, status), '', signature);
}

function _mosaicManifestUrl(status) {
    if (!status || typeof status !== 'object') return '';
    return String(status.manifest_url || status.last_ready_manifest_url || '');
}

function _mosaicPublicUrl(kind, key) {
    const manifest = _mosaicState(kind).manifest;
    const base = (manifest && manifest.base) || 'https://polypaint.s3.us-east-1.amazonaws.com/';
    return base + encodeURI(String(key || ''));
}

function _stopMosaicPoll(kind) {
    const state = _mosaicState(kind);
    if (state.pollTimer) clearTimeout(state.pollTimer);
    state.pollTimer = null;
}

function _scheduleMosaicPoll(kind) {
    const state = _mosaicState(kind);
    _stopMosaicPoll(kind);
    state.pollTimer = setTimeout(() => {
        state.pollTimer = null;
        _loadArtifactMosaic(kind, { forceStatus: true, fromPoll: true });
    }, ARTIFACT_MOSAIC_POLL_MS);
}

function _mosaicRefreshIdFromManifestKey(key) {
    const parts = String(key || '').split('/').filter(Boolean);
    if (parts.length < 2 || parts[0] !== 'renders' || parts[1] !== '_index') return '';
    if (parts[parts.length - 1] === 'all.json') return parts[parts.length - 2] || '';
    return parts[parts.length - 1] || '';
}

async function _fetchMosaicStatus(kind) {
    return await lambdaPost('storage', {}, _mosaicConfig(kind).statusPath);
}

async function _fetchMosaicManifest(url) {
    const sep = String(url).includes('?') ? '&' : '?';
    const resp = await fetch(url + sep + 'ts=' + Date.now());
    if (!resp.ok) throw new Error(`manifest fetch failed: ${resp.status}`);
    const manifest = await resp.json();
    if (!manifest || !Array.isArray(manifest.tiles)) throw new Error('invalid artifact mosaic manifest');
    return manifest;
}

function _selectedMosaicSize(kind) {
    const el = _mosaicEl(kind, 'size-filter');
    return el ? String(el.value || 'all') : 'all';
}

function _mosaicSortMode(kind) {
    const el = _mosaicEl(kind, 'sort-mode');
    return el ? String(el.value || 'date') : 'date';
}

function _mosaicRequestedCols(kind, count) {
    const el = _mosaicEl(kind, 'cols');
    const value = el ? Number(el.value) : 0;
    if (Number.isFinite(value) && value >= 1) return Math.floor(value);
    return Math.max(1, Math.ceil(Math.sqrt(Math.max(1, count || 1))));
}

function _mosaicSeededValue(kind, tile) {
    const state = _mosaicState(kind);
    const artifactId = tile.palette_id || tile.artifact_id || '';
    const s = `${tile.job_id || ''}|${artifactId}|${state.randomSeed}`;
    let h = 2166136261;
    for (let i = 0; i < s.length; i++) {
        h ^= s.charCodeAt(i);
        h = Math.imul(h, 16777619);
    }
    return h >>> 0;
}

function _syncMosaicSizeOptions(kind) {
    const cfg = _mosaicConfig(kind);
    const state = _mosaicState(kind);
    const el = _mosaicEl(kind, 'size-filter');
    if (!el) return;
    const current = String(el.value || 'all');
    const sizes = cfg.fixedSizes.length
        ? cfg.fixedSizes.map(Number)
        : (Array.isArray(state.manifest && state.manifest.sizes) ? state.manifest.sizes : [])
            .map(Number)
            .filter(n => Number.isFinite(n) && n > 0);
    const unique = Array.from(new Set(sizes)).sort((a, b) => a - b);
    const wanted = ['all'].concat(unique.map(String));
    const existing = Array.from(el.options || []).map(o => String(o.value || ''));
    if (wanted.join('|') !== existing.join('|')) {
        el.innerHTML = '';
        for (const value of wanted) {
            const opt = document.createElement ? document.createElement('option') : { value: '', textContent: '' };
            opt.value = value;
            opt.textContent = value === 'all' ? 'All' : value;
            if (typeof el.appendChild === 'function') el.appendChild(opt);
        }
    }
    el.value = wanted.includes(current) ? current : 'all';
    if (cfg.family === 'palette') el.disabled = unique.length <= 1;
}

function _mosaicFilteredSortedTiles(kind) {
    const state = _mosaicState(kind);
    const size = _selectedMosaicSize(kind);
    let tiles = Array.isArray(state.manifest && state.manifest.tiles)
        ? state.manifest.tiles.slice()
        : [];
    if (size !== 'all') {
        const n = Number(size);
        tiles = tiles.filter(t => Number(t.preview_width) === n && Number(t.preview_height) === n);
    }
    const mode = _mosaicSortMode(kind);
    tiles.sort((a, b) => {
        const aid = (x) => String(x.palette_id || x.artifact_id || '');
        if (mode === 'job') {
            return String(b.job_id || '').localeCompare(String(a.job_id || '')) ||
                String(b.created_at || '').localeCompare(String(a.created_at || '')) ||
                aid(a).localeCompare(aid(b));
        }
        if (mode === 'function') {
            return String(a.function || '').localeCompare(String(b.function || '')) ||
                String(b.created_at || '').localeCompare(String(a.created_at || ''));
        }
        if (mode === 'degree') {
            return (Number(a.degree || 0) - Number(b.degree || 0)) ||
                String(b.created_at || '').localeCompare(String(a.created_at || ''));
        }
        if (mode === 'N') {
            return (Number(a.N || 0) - Number(b.N || 0)) ||
                String(b.created_at || '').localeCompare(String(a.created_at || ''));
        }
        if (mode === 'random') {
            return _mosaicSeededValue(kind, a) - _mosaicSeededValue(kind, b);
        }
        return String(b.created_at || '').localeCompare(String(a.created_at || '')) ||
            String(a.job_id || '').localeCompare(String(b.job_id || '')) ||
            aid(a).localeCompare(aid(b));
    });
    return tiles;
}

function _ensureMosaicViewer(kind) {
    const state = _mosaicState(kind);
    const el = _mosaicEl(kind, 'viewer');
    if (!el || typeof OpenSeadragon !== 'function') return null;
    el.style.display = 'block';
    if (state.viewer) return state.viewer;
    state.viewer = OpenSeadragon({
        element: el,
        prefixUrl: 'https://cdnjs.cloudflare.com/ajax/libs/openseadragon/4.1.1/images/',
        showNavigator: true,
        navigatorPosition: 'BOTTOM_RIGHT',
        maxZoomPixelRatio: 2,
        imageLoaderLimit: 16,
    });
    if (state.viewer && typeof state.viewer.addHandler === 'function') {
        state.viewer.addHandler('canvas-click', (event) => _artifactMosaicCanvasClick(kind, event));
    }
    return state.viewer;
}

function _mosaicTileSource(kind, tiles) {
    const tileSize = _mosaicTileSize(kind, tiles);
    const cols = _mosaicRequestedCols(kind, tiles.length);
    const rows = Math.max(1, Math.ceil((tiles.length || 1) / cols));
    return {
        width: cols * tileSize,
        height: rows * tileSize,
        tileSize,
        tileOverlap: 0,
        minLevel: 0,
        maxLevel: 0,
        getTileUrl(level, x, y) {
            const idx = y * cols + x;
            const tile = tiles[idx];
            return tile ? _mosaicPublicUrl(kind, tile.key) : ARTIFACT_MOSAIC_TRANSPARENT_TILE;
        },
        _mosaicCols: cols,
        _mosaicRows: rows,
        _mosaicTileSize: tileSize,
    };
}

function _mosaicTileSize(kind, tiles) {
    const size = _selectedMosaicSize(kind);
    if (size !== 'all') {
        const n = Number(size);
        if (Number.isFinite(n) && n > 0) return n;
    }
    const values = (Array.isArray(tiles) ? tiles : [])
        .map(t => Number(t && t.preview_width))
        .filter(n => Number.isFinite(n) && n > 0);
    return values.length ? Math.max(...values) : 512;
}

function _mosaicRenderSignature(kind, tiles, source) {
    const state = _mosaicState(kind);
    const manifestId = state.manifest && state.manifest.refresh_id;
    return [
        manifestId || '',
        _selectedMosaicSize(kind),
        _mosaicSortMode(kind),
        source && source._mosaicCols,
        Array.isArray(tiles) ? tiles.length : 0,
        state.randomSeed,
    ].join('|');
}

function _mosaicUpdateSummary(kind, tiles) {
    const el = _mosaicSummaryEl(kind);
    if (!el) return;
    const state = _mosaicState(kind);
    const status = state.status || {};
    const count = Array.isArray(tiles) ? tiles.length : 0;
    const total = state.manifest && Number(state.manifest.count || 0);
    const suffix = status.refresh_id ? ` · ${status.refresh_id}` : '';
    el.textContent = `${count.toLocaleString()} shown${total ? ` / ${total.toLocaleString()}` : ''}${suffix}`;
}

function _rebuildArtifactMosaic(kind) {
    const state = _mosaicState(kind);
    if (!state.manifest) return;
    _syncMosaicSizeOptions(kind);
    state.tiles = _mosaicFilteredSortedTiles(kind);
    const viewer = _ensureMosaicViewer(kind);
    const source = _mosaicTileSource(kind, state.tiles);
    const signature = _mosaicRenderSignature(kind, state.tiles, source);
    _mosaicUpdateSummary(kind, state.tiles, source);
    if (viewer && typeof viewer.open === 'function' && signature !== state.lastRenderSignature) {
        viewer.open(source);
        state.activeTileSource = source;
        state.lastRenderSignature = signature;
    }
}

function _tileFromMosaicClick(kind, event) {
    const state = _mosaicState(kind);
    if (!state.viewer || !state.tiles.length) return null;
    if (event && event.quick === false) return null;
    if (!event || !event.position) return null;
    const viewport = state.viewer.viewport;
    if (!viewport || typeof viewport.pointFromPixel !== 'function' || typeof viewport.viewportToImageCoordinates !== 'function') return null;
    const viewportPoint = viewport.pointFromPixel(event.position);
    const imagePoint = viewport.viewportToImageCoordinates(viewportPoint);
    const tileSize = Number(state.activeTileSource && state.activeTileSource._mosaicTileSize);
    const cols = Number(state.activeTileSource && state.activeTileSource._mosaicCols);
    if (!Number.isFinite(tileSize) || tileSize <= 0 || !Number.isFinite(cols) || cols <= 0) return null;
    const x = Math.floor(imagePoint.x / tileSize);
    const y = Math.floor(imagePoint.y / tileSize);
    return state.tiles[y * cols + x] || null;
}

async function _artifactMosaicCanvasClick(kind, event) {
    const cfg = _mosaicConfig(kind);
    const tile = _tileFromMosaicClick(kind, event);
    if (!cfg || !tile || !tile.job_id) return;
    const artifactId = cfg.selectArtifactId(tile);
    if (!artifactId) return;
    await _ensureResultsSelection(tile.job_id);
    switchTab('render');
    await refreshRenderArtifacts(tile.job_id, {
        selectFamily: cfg.family,
        selectArtifactId: artifactId,
    });
}

async function _loadMosaicManifestForStatus(kind, status) {
    const state = _mosaicState(kind);
    const url = _mosaicManifestUrl(status);
    if (!url) return false;
    if (state.manifest && state.manifest.refresh_id === status.refresh_id && status.state === 'ready') {
        state.status = status || state.status;
        _rebuildArtifactMosaic(kind);
        return true;
    }
    state.manifest = await _fetchMosaicManifest(url);
    state.status = status || state.status;
    state.randomSeed += 1;
    _rebuildArtifactMosaic(kind);
    return true;
}

async function _loadArtifactMosaic(kind, opts = {}) {
    const state = _mosaicState(kind);
    const cfg = _mosaicConfig(kind);
    if (state.loading && !opts.forceStatus) return;
    state.loading = true;
    try {
        const status = await _fetchMosaicStatus(kind);
        state.status = status || {};
        const statusState = String(state.status.state || 'missing');
        if (statusState === 'computing') {
            _setMosaicStatus(kind, _mosaicProgressText(kind, state.status), '');
            _logMosaicProgress(kind, state.status);
            _setMosaicRefreshBusy(kind, true);
            _scheduleMosaicPoll(kind);
            return;
        }
        _stopMosaicPoll(kind);
        _setMosaicRefreshBusy(kind, false);
        if (statusState === 'ready') {
            await _loadMosaicManifestForStatus(kind, state.status);
            _setMosaicStatus(kind, `Ready · ${Number(state.status.count || 0).toLocaleString()} tiles`, 'ok');
            _logMosaic(
                kind,
                `${cfg.label} ready: ${Number(state.status.count || 0).toLocaleString()} tiles`,
                'ok',
                `ready|${state.status.refresh_id || ''}|${state.status.count || 0}`,
            );
            return;
        }
        if (statusState === 'error') {
            if (!state.manifest && state.status.last_ready_manifest_url) {
                await _loadMosaicManifestForStatus(kind, {
                    ...state.status,
                    state: 'ready',
                    manifest_url: state.status.last_ready_manifest_url,
                    refresh_id: _mosaicRefreshIdFromManifestKey(state.status.last_ready_manifest_key) || 'last-ready',
                });
            }
            _setMosaicStatus(kind, `Refresh failed: ${state.status.error || 'unknown error'}`, 'error');
            _logMosaic(kind, `${cfg.label} refresh failed: ${state.status.error || 'unknown error'}`, 'err', `error|${state.status.refresh_id || ''}|${state.status.error || ''}`);
            return;
        }
        _setMosaicStatus(kind, `Refresh to build the ${cfg.family === 'palette' ? 'palette ' : ''}wall.`, '');
    } catch (e) {
        const message = `${cfg.label} load failed: ${e.message}`;
        _setMosaicStatus(kind, message, 'error');
        if (opts.fromPoll || String((state.status || {}).state || '') === 'computing') {
            _setMosaicRefreshBusy(kind, true);
            _logMosaic(kind, message + ' · retrying', 'err', `poll-error|${Date.now()}`);
            _scheduleMosaicPoll(kind);
        }
    } finally {
        state.loading = false;
    }
}

async function _refreshArtifactMosaic(kind) {
    const state = _mosaicState(kind);
    const cfg = _mosaicConfig(kind);
    _stopMosaicPoll(kind);
    _setMosaicRefreshBusy(kind, true);
    try {
        const status = await lambdaPost('storage', { refresh: true }, cfg.statusPath);
        state.status = status || {};
        if (String(state.status.state || '') === 'computing') {
            _setMosaicStatus(kind, _mosaicProgressText(kind, state.status), '');
            _logMosaic(kind, `${cfg.label} refresh started: ${state.status.refresh_id || ''}`, '', `start|${state.status.refresh_id || ''}`);
            _logMosaicProgress(kind, state.status);
            _scheduleMosaicPoll(kind);
        } else {
            await _loadArtifactMosaic(kind, { forceStatus: true });
        }
    } catch (e) {
        _setMosaicStatus(kind, `Refresh failed: ${e.message}`, 'error');
        _setMosaicRefreshBusy(kind, false);
    }
}

function _homeArtifactMosaic(kind) {
    const viewer = _mosaicState(kind).viewer;
    if (viewer && viewer.viewport && typeof viewer.viewport.goHome === 'function') {
        viewer.viewport.goHome();
    }
}

function loadAllCol(opts = {}) { return _loadArtifactMosaic('color', opts); }
function refreshAllColMosaic() { return _refreshArtifactMosaic('color'); }
function homeAllCol() { return _homeArtifactMosaic('color'); }
function _allColRebuild() { return _rebuildArtifactMosaic('color'); }

function loadAllPal(opts = {}) { return _loadArtifactMosaic('palette', opts); }
function refreshAllPalMosaic() { return _refreshArtifactMosaic('palette'); }
function homeAllPal() { return _homeArtifactMosaic('palette'); }
function _allPalRebuild() { return _rebuildArtifactMosaic('palette'); }

;(window.__ppParts = window.__ppParts || []).push('13-artifact-mosaics');
