// PolyPaint 13-allrenders — single-level OpenSeadragon wall for color artifacts.
// Classic script: load order matters and is defined by index.html.
const ALLRENDERS_STATUS_PATH = '/list-color-mosaic';
const ALLRENDERS_POLL_MS = 2000;
const ALLRENDERS_TRANSPARENT_TILE = 'data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP///ywAAAAAAQABAAACAUwAOw==';

let _allRendersViewer = null;
let _allRendersStatus = null;
let _allRendersManifest = null;
let _allRendersTiles = [];
let _allRendersPollTimer = null;
let _allRendersLoading = false;
let _allRendersRandomSeed = 1;
let _allRendersLastRenderSignature = '';
let _allRendersActiveTileSource = null;

function _allRendersStatusEl() {
    return document.getElementById('allrenders-status');
}

function _allRendersSummaryEl() {
    return document.getElementById('allrenders-summary');
}

function _setAllRendersStatus(text, cls = '') {
    const el = _allRendersStatusEl();
    if (!el) return;
    el.textContent = text;
    el.className = cls ? `status ${cls}` : 'status';
}

function _setAllRendersRefreshBusy(busy) {
    const btn = document.getElementById('btn-allrenders-refresh');
    if (!btn) return;
    btn.disabled = !!busy;
    btn.textContent = busy ? 'Refreshing...' : 'Refresh';
}

function _allRendersManifestUrl(status) {
    if (!status || typeof status !== 'object') return '';
    return String(status.manifest_url || status.last_ready_manifest_url || '');
}

function _allRendersPublicUrl(key) {
    const base = (_allRendersManifest && _allRendersManifest.base) || 'https://polypaint.s3.us-east-1.amazonaws.com/';
    return base + encodeURI(String(key || ''));
}

function _stopAllRendersPoll() {
    if (_allRendersPollTimer) clearTimeout(_allRendersPollTimer);
    _allRendersPollTimer = null;
}

function _scheduleAllRendersPoll() {
    _stopAllRendersPoll();
    _allRendersPollTimer = setTimeout(() => {
        _allRendersPollTimer = null;
        loadAllRenders({ forceStatus: true });
    }, ALLRENDERS_POLL_MS);
}

async function _fetchAllRendersStatus() {
    return await lambdaPost('storage', {}, ALLRENDERS_STATUS_PATH);
}

async function _fetchAllRendersManifest(url) {
    const sep = String(url).includes('?') ? '&' : '?';
    const resp = await fetch(url + sep + 'ts=' + Date.now());
    if (!resp.ok) throw new Error(`manifest fetch failed: ${resp.status}`);
    const manifest = await resp.json();
    if (!manifest || !Array.isArray(manifest.tiles)) throw new Error('invalid AllRenders manifest');
    return manifest;
}

function _allRendersSelectedSize() {
    const el = document.getElementById('allrenders-size-filter');
    return el ? String(el.value || 'all') : 'all';
}

function _allRendersSortMode() {
    const el = document.getElementById('allrenders-sort-mode');
    return el ? String(el.value || 'date') : 'date';
}

function _allRendersRequestedCols(count) {
    const el = document.getElementById('allrenders-cols');
    const value = el ? Number(el.value) : 0;
    if (Number.isFinite(value) && value >= 1) return Math.floor(value);
    return Math.max(1, Math.ceil(Math.sqrt(Math.max(1, count || 1))));
}

function _allRendersSeededValue(tile) {
    const s = `${tile.job_id || ''}|${tile.artifact_id || ''}|${_allRendersRandomSeed}`;
    let h = 2166136261;
    for (let i = 0; i < s.length; i++) {
        h ^= s.charCodeAt(i);
        h = Math.imul(h, 16777619);
    }
    return h >>> 0;
}

function _allRendersFilteredSortedTiles() {
    const size = _allRendersSelectedSize();
    let tiles = Array.isArray(_allRendersManifest && _allRendersManifest.tiles)
        ? _allRendersManifest.tiles.slice()
        : [];
    if (size === '512' || size === '1024') {
        const n = Number(size);
        tiles = tiles.filter(t => Number(t.preview_width) === n && Number(t.preview_height) === n);
    }
    const mode = _allRendersSortMode();
    tiles.sort((a, b) => {
        if (mode === 'job') {
            return String(b.job_id || '').localeCompare(String(a.job_id || '')) ||
                String(b.created_at || '').localeCompare(String(a.created_at || '')) ||
                String(a.artifact_id || '').localeCompare(String(b.artifact_id || ''));
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
            return _allRendersSeededValue(a) - _allRendersSeededValue(b);
        }
        return String(b.created_at || '').localeCompare(String(a.created_at || '')) ||
            String(a.job_id || '').localeCompare(String(b.job_id || '')) ||
            String(a.artifact_id || '').localeCompare(String(b.artifact_id || ''));
    });
    return tiles;
}

function _ensureAllRendersViewer() {
    const el = document.getElementById('allrenders-viewer');
    if (!el || typeof OpenSeadragon !== 'function') return null;
    el.style.display = 'block';
    if (_allRendersViewer) return _allRendersViewer;
    _allRendersViewer = OpenSeadragon({
        element: el,
        prefixUrl: 'https://cdnjs.cloudflare.com/ajax/libs/openseadragon/4.1.1/images/',
        showNavigator: true,
        navigatorPosition: 'BOTTOM_RIGHT',
        maxZoomPixelRatio: 2,
        imageLoaderLimit: 16,
    });
    if (_allRendersViewer && typeof _allRendersViewer.addHandler === 'function') {
        _allRendersViewer.addHandler('canvas-click', _allRendersCanvasClick);
    }
    return _allRendersViewer;
}

function _allRendersTileSource(tiles) {
    const tileSize = 512;
    const cols = _allRendersRequestedCols(tiles.length);
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
            return tile ? _allRendersPublicUrl(tile.key) : ALLRENDERS_TRANSPARENT_TILE;
        },
        _allRendersCols: cols,
        _allRendersRows: rows,
    };
}

function _allRendersRenderSignature(tiles, source) {
    const manifestId = _allRendersManifest && _allRendersManifest.refresh_id;
    return [
        manifestId || '',
        _allRendersSelectedSize(),
        _allRendersSortMode(),
        source && source._allRendersCols,
        Array.isArray(tiles) ? tiles.length : 0,
        _allRendersRandomSeed,
    ].join('|');
}

function _allRendersUpdateSummary(tiles, source) {
    const el = _allRendersSummaryEl();
    if (!el) return;
    const status = _allRendersStatus || {};
    const count = Array.isArray(tiles) ? tiles.length : 0;
    const total = _allRendersManifest && Number(_allRendersManifest.count || 0);
    const suffix = status.refresh_id ? ` · ${status.refresh_id}` : '';
    el.textContent = `${count.toLocaleString()} shown${total ? ` / ${total.toLocaleString()}` : ''}${suffix}`;
}

function _allRendersRebuild() {
    if (!_allRendersManifest) return;
    _allRendersTiles = _allRendersFilteredSortedTiles();
    const viewer = _ensureAllRendersViewer();
    const source = _allRendersTileSource(_allRendersTiles);
    const signature = _allRendersRenderSignature(_allRendersTiles, source);
    _allRendersUpdateSummary(_allRendersTiles, source);
    if (viewer && typeof viewer.open === 'function' && signature !== _allRendersLastRenderSignature) {
        viewer.open(source);
        _allRendersActiveTileSource = source;
        _allRendersLastRenderSignature = signature;
    }
}

async function _allRendersCanvasClick(event) {
    if (!_allRendersViewer || !_allRendersTiles.length) return;
    if (event && event.quick === false) return;
    if (!event || !event.position) return;
    const viewport = _allRendersViewer.viewport;
    if (!viewport || typeof viewport.pointFromPixel !== 'function' || typeof viewport.viewportToImageCoordinates !== 'function') return;
    const viewportPoint = viewport.pointFromPixel(event.position);
    const imagePoint = viewport.viewportToImageCoordinates(viewportPoint);
    const tileSize = 512;
    const cols = Number(_allRendersActiveTileSource && _allRendersActiveTileSource._allRendersCols) ||
        _allRendersRequestedCols(_allRendersTiles.length);
    const x = Math.floor(imagePoint.x / tileSize);
    const y = Math.floor(imagePoint.y / tileSize);
    const tile = _allRendersTiles[y * cols + x];
    if (!tile || !tile.job_id || !tile.artifact_id) return;
    await _ensureResultsSelection(tile.job_id);
    switchTab('render');
    await refreshRenderArtifacts(tile.job_id, {
        selectFamily: 'color',
        selectArtifactId: tile.artifact_id,
    });
}

async function _loadAllRendersManifestForStatus(status) {
    const url = _allRendersManifestUrl(status);
    if (!url) return false;
    if (_allRendersManifest && _allRendersManifest.refresh_id === status.refresh_id && status.state === 'ready') {
        _allRendersStatus = status || _allRendersStatus;
        _allRendersRebuild();
        return true;
    }
    _allRendersManifest = await _fetchAllRendersManifest(url);
    _allRendersStatus = status || _allRendersStatus;
    _allRendersRandomSeed += 1;
    _allRendersRebuild();
    return true;
}

async function loadAllRenders(opts = {}) {
    if (_allRendersLoading && !opts.forceStatus) return;
    _allRendersLoading = true;
    try {
        const status = await _fetchAllRendersStatus();
        _allRendersStatus = status || {};
        const state = String(_allRendersStatus.state || 'missing');
        if (state === 'computing') {
            _setAllRendersStatus(`Computing AllRenders mosaic... ${_allRendersStatus.refresh_id || ''}`, '');
            _setAllRendersRefreshBusy(true);
            _scheduleAllRendersPoll();
            return;
        }
        _stopAllRendersPoll();
        _setAllRendersRefreshBusy(false);
        if (state === 'ready') {
            await _loadAllRendersManifestForStatus(_allRendersStatus);
            _setAllRendersStatus(`Ready · ${Number(_allRendersStatus.count || 0).toLocaleString()} tiles`, 'ok');
            return;
        }
        if (state === 'error') {
            if (!_allRendersManifest && _allRendersStatus.last_ready_manifest_url) {
                await _loadAllRendersManifestForStatus({
                    ..._allRendersStatus,
                    state: 'ready',
                    manifest_url: _allRendersStatus.last_ready_manifest_url,
                    refresh_id: (_allRendersStatus.last_ready_manifest_key || '').split('/').slice(-2, -1)[0] || 'last-ready',
                });
            }
            _setAllRendersStatus(`Refresh failed: ${_allRendersStatus.error || 'unknown error'}`, 'error');
            return;
        }
        _setAllRendersStatus('Refresh to build the wall.', '');
    } catch (e) {
        _setAllRendersStatus(`AllRenders load failed: ${e.message}`, 'error');
    } finally {
        _allRendersLoading = false;
    }
}

async function refreshAllRendersMosaic() {
    _stopAllRendersPoll();
    _setAllRendersRefreshBusy(true);
    try {
        const status = await lambdaPost('storage', { refresh: true }, ALLRENDERS_STATUS_PATH);
        _allRendersStatus = status || {};
        if (String(_allRendersStatus.state || '') === 'computing') {
            _setAllRendersStatus(`Computing AllRenders mosaic... ${_allRendersStatus.refresh_id || ''}`, '');
            _scheduleAllRendersPoll();
        } else {
            await loadAllRenders({ forceStatus: true });
        }
    } catch (e) {
        _setAllRendersStatus(`Refresh failed: ${e.message}`, 'error');
        _setAllRendersRefreshBusy(false);
    }
}

function homeAllRenders() {
    if (_allRendersViewer && _allRendersViewer.viewport && typeof _allRendersViewer.viewport.goHome === 'function') {
        _allRendersViewer.viewport.goHome();
    }
}

;(window.__ppParts = window.__ppParts || []).push('13-allrenders');
