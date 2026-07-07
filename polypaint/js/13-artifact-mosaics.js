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

const _artifactMosaicContext = {
    open: false,
    kind: '',
    tile: null,
    tileKey: '',
    x: 0,
    y: 0,
    busy: false,
    message: '',
    error: '',
};
let _artifactMosaicContextGlobalBound = false;

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
        wall: null,
        wallPollTimer: null,
        lastLogSignature: '',
        contextMenuBound: false,
        sharing: false,
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

function _setMosaicShareBusy(kind, busy) {
    const btn = document.getElementById(`btn-${_mosaicConfig(kind).tabName}-share`);
    if (!btn) return;
    btn.disabled = !!busy;
    btn.textContent = busy ? 'Sharing...' : 'Share';
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

function _mosaicEscapeHtml(value) {
    if (typeof _escapeHtml === 'function') return _escapeHtml(value);
    return String(value == null ? '' : value)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
}

function _mosaicArtifactId(kind, tile) {
    if (!tile) return '';
    const cfg = _mosaicConfig(kind);
    if (cfg && typeof cfg.selectArtifactId === 'function') return String(cfg.selectArtifactId(tile) || '');
    return String(tile.palette_id || tile.artifact_id || '');
}

function _mosaicTileIdentity(kind, tile) {
    const state = _mosaicState(kind);
    return `${tile && tile.job_id || ''}|${_mosaicArtifactId(kind, tile)}|${state.manifest && state.manifest.refresh_id || ''}`;
}

function _mosaicExtensionFromKey(key) {
    const leaf = String(key || '').split('?')[0].split('#')[0].split('/').pop() || '';
    const idx = leaf.lastIndexOf('.');
    const ext = idx >= 0 ? leaf.slice(idx + 1).toLowerCase() : '';
    return ext && /^[a-z0-9]+$/.test(ext) ? ext : '';
}

async function _copyTextToClipboard(text) {
    const value = String(text || '');
    if (!value) throw new Error('Nothing to copy');
    if (typeof navigator !== 'undefined' && navigator.clipboard && typeof navigator.clipboard.writeText === 'function') {
        await navigator.clipboard.writeText(value);
        return;
    }
    const input = document.createElement('textarea');
    input.value = value;
    input.setAttribute('readonly', 'readonly');
    input.style.position = 'fixed';
    input.style.left = '-9999px';
    document.body.appendChild(input);
    input.select();
    const ok = document.execCommand && document.execCommand('copy');
    document.body.removeChild(input);
    if (!ok) throw new Error(value);
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

function _mosaicRequestedColsValue(kind) {
    const el = _mosaicEl(kind, 'cols');
    const value = el ? Number(el.value) : 0;
    return Number.isFinite(value) && value >= 1 ? String(Math.floor(value)) : '';
}

function _openMosaicShareUrl(url) {
    const shareUrl = String(url || '');
    if (!shareUrl) throw new Error('Share URL missing');
    if (typeof window !== 'undefined' && typeof window.open === 'function') {
        const opened = window.open(shareUrl, '_blank');
        if (opened) {
            try { opened.opener = null; } catch (e) {}
            return true;
        }
    }
    return false;
}

async function _shareArtifactMosaic(kind) {
    const state = _mosaicState(kind);
    const cfg = _mosaicConfig(kind);
    if (state.sharing) return null;
    state.sharing = true;
    _setMosaicShareBusy(kind, true);
    try {
        const result = await lambdaPost('storage', {
            kind,
            size: _selectedMosaicSize(kind),
            sort: _mosaicSortMode(kind),
            cols: _mosaicRequestedColsValue(kind),
        }, '/share-mosaic');
        const shareUrl = String(result && result.share_url || '');
        const opened = _openMosaicShareUrl(shareUrl);
        if (!opened) await _copyTextToClipboard(shareUrl);
        const message = opened ? 'Share opened' : 'Share link copied';
        _setMosaicStatus(kind, `${cfg.label} ${message.toLowerCase()}.`, 'ok');
        _logMosaic(kind, `${cfg.label} ${message}: ${shareUrl}`, 'ok', `share|${kind}|${result && result.share_id || shareUrl}`);
        return result;
    } catch (e) {
        const msg = e && e.message ? e.message : String(e);
        _setMosaicStatus(kind, `${cfg.label} share failed: ${msg}`, 'error');
        _logMosaic(kind, `${cfg.label} share failed: ${msg}`, 'err', `share-failed|${kind}|${msg}`);
        return null;
    } finally {
        state.sharing = false;
        _setMosaicShareBusy(kind, false);
    }
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
    _ensureMosaicContextGlobalHandlers();
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
        state.viewer.addHandler('canvas-contextmenu', (event) => _artifactMosaicContextMenuEvent(kind, event));
    }
    if (!state.contextMenuBound && typeof el.addEventListener === 'function') {
        el.addEventListener('contextmenu', (event) => _artifactMosaicNativeContextMenuEvent(kind, event));
        state.contextMenuBound = true;
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

// ── Composite wall pyramid (deepzoom-speed.md §7.1) ──────────────────────
// A refresh bakes the DEFAULT view (date sort, all sizes, auto cols) into a
// real DZI pyramid server-side; opening it costs ~a dozen tiles instead of
// one request per artifact. Any other sort/size/cols falls back to the
// per-tile grid below, so no feature is lost.

function _mosaicWallEligible(kind) {
    const state = _mosaicState(kind);
    const wall = state.wall;
    if (!wall || !state.manifest || !Array.isArray(wall.tiles) || !wall.tiles.length) return false;
    if (String(wall.refresh_id || '') !== String(state.manifest.refresh_id || '')) return false;
    if (_mosaicSortMode(kind) !== 'date') return false;
    if (_selectedMosaicSize(kind) !== 'all') return false;
    if (_mosaicRequestedColsValue(kind) !== '') return false;
    return true;
}

async function _maybeLoadMosaicWall(kind) {
    const state = _mosaicState(kind);
    const status = state.status || {};
    const key = String(status.wall_json_key || '');
    if (String(status.wall_state || '') !== 'ready' || !key) return;
    if (state.wall && String(state.wall.refresh_id || '') === String(status.wall_refresh_id || '')) return;
    try {
        const resp = await fetch(_mosaicPublicUrl(kind, key) + '?ts=' + Date.now());
        if (!resp.ok) throw new Error(`wall.json HTTP ${resp.status}`);
        state.wall = await resp.json();
    } catch (e) {
        state.wall = null;
    }
}

function _stopMosaicWallPoll(kind) {
    const state = _mosaicState(kind);
    if (state.wallPollTimer) {
        clearTimeout(state.wallPollTimer);
        state.wallPollTimer = null;
    }
}

function _scheduleMosaicWallPoll(kind, attempt = 0) {
    const state = _mosaicState(kind);
    _stopMosaicWallPoll(kind);
    const wallState = String((state.status || {}).wall_state || '');
    if (wallState !== 'computing' || attempt >= 40) return;
    state.wallPollTimer = setTimeout(async () => {
        state.wallPollTimer = null;
        try {
            state.status = (await _fetchMosaicStatus(kind)) || state.status;
            await _maybeLoadMosaicWall(kind);
            if (String((state.status || {}).wall_state || '') === 'ready') {
                _rebuildArtifactMosaic(kind);
                return;
            }
        } catch (e) { /* transient status fetch failure: keep polling */ }
        _scheduleMosaicWallPoll(kind, attempt + 1);
    }, 8000);
}

function _rebuildArtifactMosaic(kind) {
    const state = _mosaicState(kind);
    _closeMosaicContextMenu();
    if (!state.manifest) return;
    _syncMosaicSizeOptions(kind);
    if (_mosaicWallEligible(kind)) {
        const wall = state.wall;
        state.tiles = wall.tiles;
        const viewer = _ensureMosaicViewer(kind);
        const meta = {
            _mosaicCols: Number(wall.cols) || 1,
            _mosaicRows: Number(wall.rows) || 1,
            _mosaicTileSize: Number(wall.cell_px) || 512,
            _wallRefreshId: String(wall.refresh_id || ''),
        };
        const signature = `wall|${meta._wallRefreshId}|${state.tiles.length}`;
        _mosaicUpdateSummary(kind, state.tiles, meta);
        if (viewer && typeof viewer.open === 'function' && signature !== state.lastRenderSignature) {
            viewer.open(_mosaicPublicUrl(kind, wall.dzi_key));
            state.activeTileSource = meta;
            state.lastRenderSignature = signature;
        }
        return;
    }
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

function _tileFromMosaicPixel(kind, pixelPoint) {
    const state = _mosaicState(kind);
    if (!state.viewer || !state.tiles.length) return null;
    const viewport = state.viewer.viewport;
    if (!viewport || typeof viewport.pointFromPixel !== 'function' || typeof viewport.viewportToImageCoordinates !== 'function') return null;
    const viewportPoint = viewport.pointFromPixel(pixelPoint);
    const imagePoint = viewport.viewportToImageCoordinates(viewportPoint);
    const tileSize = Number(state.activeTileSource && state.activeTileSource._mosaicTileSize);
    const cols = Number(state.activeTileSource && state.activeTileSource._mosaicCols);
    const rows = Number(state.activeTileSource && state.activeTileSource._mosaicRows);
    if (!Number.isFinite(tileSize) || tileSize <= 0 || !Number.isFinite(cols) || cols <= 0) return null;
    if (!Number.isFinite(rows) || rows <= 0) return null;
    const x = Math.floor(imagePoint.x / tileSize);
    const y = Math.floor(imagePoint.y / tileSize);
    if (x < 0 || y < 0 || x >= cols || y >= rows) return null;
    return state.tiles[y * cols + x] || null;
}

function _tileFromMosaicClick(kind, event) {
    if (event && event.quick === false) return null;
    if (!event || !event.position) return null;
    return _tileFromMosaicPixel(kind, event.position);
}

function _tileFromMosaicDomEvent(kind, domEvent) {
    const state = _mosaicState(kind);
    const el = (state.viewer && state.viewer.element) || _mosaicEl(kind, 'viewer');
    if (!el || !domEvent || typeof OpenSeadragon !== 'function') return null;
    const rect = typeof el.getBoundingClientRect === 'function'
        ? el.getBoundingClientRect()
        : { left: 0, top: 0 };
    return _tileFromMosaicPixel(
        kind,
        new OpenSeadragon.Point(
            Number(domEvent.clientX || 0) - Number(rect.left || 0),
            Number(domEvent.clientY || 0) - Number(rect.top || 0),
        ),
    );
}

function _setRenderJobForMosaic(jobId) {
    if (typeof _setRenderResultsJob === 'function') {
        _setRenderResultsJob(jobId);
        return;
    }
    const el = document.getElementById('render-results-dir');
    if (el) el.value = jobId;
}

async function _goMosaicTileRender(kind, tile) {
    const cfg = _mosaicConfig(kind);
    const artifactId = _mosaicArtifactId(kind, tile);
    if (!cfg || !tile || !tile.job_id || !artifactId) throw new Error('Missing mosaic tile target');
    _setRenderJobForMosaic(tile.job_id);
    switchTab('render');
    await refreshRenderArtifacts(tile.job_id, {
        selectFamily: cfg.family,
        selectArtifactId: artifactId,
    });
    const selected = typeof _renderSelectedArtifactEntry === 'function' ? _renderSelectedArtifactEntry() : null;
    const selectedId = selected && (selected.palette_id || selected.artifact_id);
    if (selectedId !== artifactId) {
        throw new Error(`${artifactId} was not found in Render ${cfg.family}`);
    }
    _logMosaic(kind, `${cfg.label} selected ${artifactId}`, 'ok', `select|${kind}|${tile.job_id}|${artifactId}`);
}

async function populateComputeFromJob(jobId) {
    if (!jobId) throw new Error('Missing compute job id');
    const detail = await _getResultDetail(jobId);
    _populateComputeFromDetail(jobId, detail || {});
    switchTab('compute');
}

async function _artifactMosaicCanvasClick(kind, event) {
    const cfg = _mosaicConfig(kind);
    const tile = _tileFromMosaicClick(kind, event);
    if (!cfg || !tile || !tile.job_id) return;
    const artifactId = _mosaicArtifactId(kind, tile);
    if (!artifactId) return;
    if (event) event.preventDefaultAction = true;
    try {
        await _goMosaicTileRender(kind, tile);
    } catch (e) {
        _logMosaic(kind, `${cfg.label} click failed: ${e.message}`, 'err', `click-failed|${kind}|${tile.job_id}|${artifactId}|${e.message}`);
        _setMosaicStatus(kind, `${cfg.label} click failed: ${e.message}`, 'error');
    }
}

function _mosaicContextPointFromEvent(event) {
    const raw = (event && event.originalEvent) || event || {};
    return {
        x: Number(raw.clientX || 0) || 12,
        y: Number(raw.clientY || 0) || 12,
    };
}

function _mosaicContextButton(label, action, disabled = false) {
    return `<button type="button" class="artifact-mosaic-menu-action" data-mosaic-action="${_mosaicEscapeHtml(action)}"${disabled ? ' disabled' : ''}>${_mosaicEscapeHtml(label)}</button>`;
}

function _ensureMosaicContextGlobalHandlers() {
    if (_artifactMosaicContextGlobalBound || typeof document === 'undefined' || typeof document.addEventListener !== 'function') return;
    document.addEventListener('keydown', (event) => {
        if (event && event.key === 'Escape') _closeMosaicContextMenu();
    });
    document.addEventListener('click', (event) => {
        const menu = document.getElementById('artifact-mosaic-context-menu');
        if (!_artifactMosaicContext.open || !menu || !event || !event.target) return;
        if (typeof menu.contains === 'function' && menu.contains(event.target)) return;
        _closeMosaicContextMenu();
    });
    _artifactMosaicContextGlobalBound = true;
}

function _mosaicContextRow(label, value) {
    const text = String(value == null ? '' : value).trim();
    if (!text) return '';
    return `<div class="artifact-mosaic-menu-row"><span>${_mosaicEscapeHtml(label)}</span><code>${_mosaicEscapeHtml(text)}</code></div>`;
}

function _renderMosaicContextMenu() {
    const menu = document.getElementById('artifact-mosaic-context-menu');
    if (!menu) return;
    const ctx = _artifactMosaicContext;
    if (!ctx.open || !ctx.tile) {
        menu.style.display = 'none';
        if (typeof menu.setAttribute === 'function') menu.setAttribute('aria-hidden', 'true');
        menu.innerHTML = '';
        return;
    }
    const kind = ctx.kind;
    const cfg = _mosaicConfig(kind);
    const tile = ctx.tile;
    const artifactId = _mosaicArtifactId(kind, tile);
    const favoriteDisabled = kind !== 'color';
    const rows = [
        _mosaicContextRow('artifact', artifactId),
        _mosaicContextRow('job', tile.job_id),
        _mosaicContextRow('function', tile.function),
        _mosaicContextRow('created', tile.created_at),
        _mosaicContextRow('preview', tile.preview_width && tile.preview_height ? `${tile.preview_width} x ${tile.preview_height}` : ''),
        _mosaicContextRow('N', tile.N),
        _mosaicContextRow('degree', tile.degree),
        _mosaicContextRow('times', tile.times),
        kind === 'palette' ? _mosaicContextRow('metric', tile.metric || tile.solve_display) : '',
        kind === 'palette' ? _mosaicContextRow('palette', tile.palette || tile.palette_name) : '',
        kind === 'palette' ? _mosaicContextRow('channels', tile.score_output_channel_count || tile.raw_channels) : '',
    ].filter(Boolean).join('');
    menu.innerHTML = `
        <div class="artifact-mosaic-menu-head">
            <div class="artifact-mosaic-menu-title">${_mosaicEscapeHtml(cfg.label)} tile</div>
            <button type="button" class="artifact-mosaic-menu-close" data-mosaic-action="close" aria-label="Close">x</button>
        </div>
        <div class="artifact-mosaic-menu-meta">${rows}</div>
        <div class="artifact-mosaic-menu-actions">
            ${_mosaicContextButton('Go Render', 'go-render', ctx.busy)}
            ${_mosaicContextButton('Go Compute', 'go-compute', ctx.busy)}
            ${_mosaicContextButton('Go Result', 'go-result', ctx.busy)}
            ${_mosaicContextButton(favoriteDisabled ? 'Favorite (Color only)' : 'Favorite', 'favorite', ctx.busy || favoriteDisabled)}
            ${_mosaicContextButton(favoriteDisabled ? 'Add to Book (Color only)' : 'Add to Book', 'add-book', ctx.busy || favoriteDisabled)}
            ${_mosaicContextButton('Download', 'download', ctx.busy)}
            ${_mosaicContextButton('Copy Link', 'copy-link', ctx.busy)}
            ${_mosaicContextButton('Copy Job ID', 'copy-job', ctx.busy)}
            ${_mosaicContextButton('Copy Artifact ID', 'copy-artifact', ctx.busy)}
        </div>
        ${ctx.message ? `<div class="artifact-mosaic-menu-note ok">${_mosaicEscapeHtml(ctx.message)}</div>` : ''}
        ${ctx.error ? `<div class="artifact-mosaic-menu-note err">${_mosaicEscapeHtml(ctx.error)}</div>` : ''}
    `;
    menu.style.display = 'block';
    if (typeof menu.setAttribute === 'function') menu.setAttribute('aria-hidden', 'false');
    menu.style.left = `${Math.max(8, ctx.x)}px`;
    menu.style.top = `${Math.max(8, ctx.y)}px`;
    const rect = typeof menu.getBoundingClientRect === 'function' ? menu.getBoundingClientRect() : { width: 260, height: 220 };
    const vw = (typeof window !== 'undefined' && window.innerWidth) || 1200;
    const vh = (typeof window !== 'undefined' && window.innerHeight) || 800;
    const left = Math.max(8, Math.min(ctx.x, vw - Number(rect.width || 260) - 8));
    const top = Math.max(8, Math.min(ctx.y, vh - Number(rect.height || 220) - 8));
    menu.style.left = `${left}px`;
    menu.style.top = `${top}px`;
    if (!menu._artifactMosaicBound && typeof menu.addEventListener === 'function') {
        menu.addEventListener('click', (event) => {
            const btn = event.target && event.target.closest ? event.target.closest('[data-mosaic-action]') : null;
            if (!btn) return;
            event.preventDefault();
            _runMosaicContextAction(btn.getAttribute('data-mosaic-action'));
        });
        menu._artifactMosaicBound = true;
    }
}

function _openMosaicContextMenu(kind, tile, event) {
    const point = _mosaicContextPointFromEvent(event);
    _artifactMosaicContext.open = true;
    _artifactMosaicContext.kind = kind;
    _artifactMosaicContext.tile = tile;
    _artifactMosaicContext.tileKey = _mosaicTileIdentity(kind, tile);
    _artifactMosaicContext.x = point.x;
    _artifactMosaicContext.y = point.y;
    _artifactMosaicContext.busy = false;
    _artifactMosaicContext.message = '';
    _artifactMosaicContext.error = '';
    _renderMosaicContextMenu();
}

function _closeMosaicContextMenu() {
    _artifactMosaicContext.open = false;
    _artifactMosaicContext.kind = '';
    _artifactMosaicContext.tile = null;
    _artifactMosaicContext.tileKey = '';
    _artifactMosaicContext.busy = false;
    _artifactMosaicContext.message = '';
    _artifactMosaicContext.error = '';
    _renderMosaicContextMenu();
}

function _artifactMosaicContextMenuEvent(kind, event) {
    if (event) {
        event.preventDefaultAction = true;
        if (event.originalEvent && typeof event.originalEvent.preventDefault === 'function') event.originalEvent.preventDefault();
        if (event.originalEvent && typeof event.originalEvent.stopPropagation === 'function') event.originalEvent.stopPropagation();
    }
    const tile = _tileFromMosaicClick(kind, event);
    if (!tile) {
        _closeMosaicContextMenu();
        return;
    }
    _openMosaicContextMenu(kind, tile, event);
}

function _artifactMosaicNativeContextMenuEvent(kind, event) {
    if (event && typeof event.preventDefault === 'function') event.preventDefault();
    if (event && typeof event.stopPropagation === 'function') event.stopPropagation();
    const tile = _tileFromMosaicDomEvent(kind, event);
    if (!tile) {
        _closeMosaicContextMenu();
        return;
    }
    _openMosaicContextMenu(kind, tile, event);
}

function _mosaicContextStillCurrent(kind, tile, tileKey) {
    return _artifactMosaicContext.open &&
        _artifactMosaicContext.kind === kind &&
        _artifactMosaicContext.tile === tile &&
        _artifactMosaicContext.tileKey === tileKey &&
        _mosaicTileIdentity(kind, tile) === tileKey;
}

function _mosaicDownloadFilename(kind, tile) {
    const key = tile.image_key || tile.key || '';
    const ext = _mosaicExtensionFromKey(key) || 'png';
    return `${_mosaicArtifactId(kind, tile) || 'artifact'}.${ext}`;
}

async function _runMosaicContextAction(action) {
    const ctx = _artifactMosaicContext;
    const kind = ctx.kind;
    const tile = ctx.tile;
    const tileKey = ctx.tileKey;
    const cfg = _mosaicConfig(kind);
    if (action === 'close') {
        _closeMosaicContextMenu();
        return;
    }
    if (!_mosaicContextStillCurrent(kind, tile, tileKey)) {
        _closeMosaicContextMenu();
        return;
    }
    ctx.busy = true;
    ctx.message = '';
    ctx.error = '';
    _renderMosaicContextMenu();
    try {
        const artifactId = _mosaicArtifactId(kind, tile);
        if (action === 'go-render') {
            await _goMosaicTileRender(kind, tile);
            _closeMosaicContextMenu();
            return;
        }
        if (action === 'go-compute') {
            await populateComputeFromJob(tile.job_id);
            _closeMosaicContextMenu();
            return;
        }
        if (action === 'go-result') {
            await _ensureResultsSelection(tile.job_id);
            switchTab('results');
            _closeMosaicContextMenu();
            return;
        }
        if (action === 'favorite') {
            if (kind !== 'color') throw new Error('Favorites currently support Color artifacts only');
            const result = await _addColorFavorite({
                jobId: tile.job_id,
                artifactId,
                displayName: tile.display_name || artifactId,
                imageKey: tile.image_key || '',
                previewKey: tile.key || '',
            });
            ctx.message = result && result.already ? 'Already in favorites' : 'Favorited';
            _logMosaic(kind, `${cfg.label}: ${ctx.message} ${artifactId}`, 'ok', `favorite|${tile.job_id}|${artifactId}|${ctx.message}`);
        } else if (action === 'add-book') {
            if (kind !== 'color') throw new Error('Books currently support Color artifacts only');
            const ok = await _bookAddEntry({
                jobId: tile.job_id,
                artifactId,
                displayName: tile.display_name || artifactId,
                imageKey: tile.image_key || '',
            }, (msg, err) => { if (err) ctx.error = msg; else ctx.message = msg; });
            _logMosaic(kind, `${cfg.label}: ${(ok ? ctx.message : ctx.error) || 'Add to Book'} ${artifactId}`,
                       ok ? 'ok' : 'err', `add-book|${tile.job_id}|${artifactId}`);
        } else if (action === 'download') {
            const key = tile.image_key || tile.key || '';
            await _downloadStorageObject({ key, filename: _mosaicDownloadFilename(kind, tile), fallbackUrl: key ? '' : _mosaicPublicUrl(kind, tile.key) });
            ctx.message = 'Download started';
        } else if (action === 'copy-link') {
            const key = tile.image_key || tile.key || '';
            await _copyTextToClipboard(_mosaicPublicUrl(kind, key));
            ctx.message = 'Link copied';
        } else if (action === 'copy-job') {
            await _copyTextToClipboard(tile.job_id);
            ctx.message = 'Job ID copied';
        } else if (action === 'copy-artifact') {
            await _copyTextToClipboard(artifactId);
            ctx.message = 'Artifact ID copied';
        } else {
            throw new Error(`Unknown action: ${action}`);
        }
        ctx.error = '';
    } catch (e) {
        ctx.error = e.message || String(e);
        _logMosaic(kind, `${cfg.label} menu action failed: ${ctx.error}`, 'err', `menu-failed|${kind}|${action}|${ctx.error}`);
    } finally {
        if (_artifactMosaicContext.open) {
            _artifactMosaicContext.busy = false;
            _renderMosaicContextMenu();
        }
    }
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
            await _maybeLoadMosaicWall(kind);
            _rebuildArtifactMosaic(kind);
            _scheduleMosaicWallPoll(kind);
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
    _closeMosaicContextMenu();
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
function shareAllColMosaic() { return _shareArtifactMosaic('color'); }
function _allColRebuild() { return _rebuildArtifactMosaic('color'); }

function loadAllPal(opts = {}) { return _loadArtifactMosaic('palette', opts); }
function refreshAllPalMosaic() { return _refreshArtifactMosaic('palette'); }
function homeAllPal() { return _homeArtifactMosaic('palette'); }
function shareAllPalMosaic() { return _shareArtifactMosaic('palette'); }
function _allPalRebuild() { return _rebuildArtifactMosaic('palette'); }

;(window.__ppParts = window.__ppParts || []).push('13-artifact-mosaics');
