// PolyPaint 04-palette-color — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Deploy rewrites the script tags to build-versioned asset
// keys (assets/<BUILD_ID>/...), so a deploy flips atomically via index.html.
function _ensureSolveScoreChainDefaults() {
    if (!_normalizeSolveScoreChain(_renderScoreChain).length) {
        _renderScoreChain.splice(0, _renderScoreChain.length, ..._defaultSolveScoreChain(renderSolveMetric || 'proximity'));
    }
    if (!_normalizeSolveScoreChain(_paletteScoreChain).length) {
        _paletteScoreChain.splice(0, _paletteScoreChain.length, ..._defaultSolveScoreChain(paletteTabMetric || 'proximity'));
    }
}

function _chainHasTransformName(chain, name) {
    if (!Array.isArray(chain)) return false;
    return chain.some(item => {
        if (Array.isArray(item)) return item[0] === name;
        return item === name;
    });
}

function _safeHttpUrl(value) {
    const s = value == null ? '' : String(value).trim();
    if (!s) return '';
    try {
        const base = (window.location && window.location.href) ? window.location.href : 'http://localhost/';
        const resolved = new URL(s, base);
        if (resolved.protocol !== 'http:' && resolved.protocol !== 'https:') return '';
        return resolved.href;
    } catch (e) {
        return '';
    }
}

function _setInlineError(container, message, styleText) {
    if (!container) return;
    container.replaceChildren();
    const errorEl = document.createElement('div');
    errorEl.style.cssText = styleText || 'color:#e94560';
    errorEl.textContent = 'Error: ' + (message == null ? '' : String(message));
    container.appendChild(errorEl);
}

function _setPreviewPlaceholder(previewEl, message) {
    if (!previewEl) return;
    previewEl.replaceChildren();
    const placeholderEl = document.createElement('span');
    placeholderEl.style.cssText = 'color:#444; font-size:11px';
    placeholderEl.textContent = message;
    previewEl.appendChild(placeholderEl);
}

function _setPreviewImage(previewEl, url) {
    if (!previewEl) return;
    const safeUrl = _safeHttpUrl(url);
    if (!safeUrl) {
        _setPreviewPlaceholder(previewEl, 'No preview');
        return;
    }
    previewEl.replaceChildren();
    const imgEl = document.createElement('img');
    imgEl.src = safeUrl;
    imgEl.style.cssText = 'max-width:100%; max-height:100%; image-rendering:pixelated';
    previewEl.appendChild(imgEl);
}

// Download a file via presigned URL with Content-Disposition: attachment
// Gets a new presigned URL with download filename baked in, then navigates to it.
async function _downloadStorageObject({ key, filename, fallbackUrl = '' }) {
    if (key) {
        const result = await lambdaPost('storage', { key, filename }, '/presign');
        window.location.href = result.url;
        return result;
    }
    if (!fallbackUrl) throw new Error('Download requires a storage key or URL');
    const a = document.createElement('a');
    a.href = fallbackUrl;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    return { url: fallbackUrl };
}

async function downloadPresignedFile(originalUrl, filename, explicitKey, clickEvent) {
    const activeEvent = clickEvent || ((typeof window !== 'undefined' && window.event) ? window.event : null);
    const btn = activeEvent && activeEvent.target ? activeEvent.target : null;
    const origText = btn ? btn.textContent : '';
    if (btn) { btn.disabled = true; btn.textContent = 'Preparing download...'; }
    try {
        // Re-presign with Content-Disposition: attachment so browser saves instead of displaying
        const key = explicitKey || _lastDownloadKey;
        await _downloadStorageObject({ key, filename, fallbackUrl: originalUrl });
    } catch (e) {
        alert('Download failed: ' + e.message);
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = origText; }
    }
}

async function _presignStorageKey(key, filename) {
    return await lambdaPost('storage', { key, filename }, '/presign');
}

async function _fetchStorageBlob(key, filename) {
    const presign = await _presignStorageKey(key, filename);
    const resp = await fetch(presign.url);
    if (!resp.ok) throw new Error(`Download failed: ${resp.status}`);
    return await resp.blob();
}

function _downloadBlob(blob, filename) {
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
}

async function _downloadStorageKeyBlob(key, filename) {
    const blob = await _fetchStorageBlob(key, filename);
    _downloadBlob(blob, filename);
}
let _lastDownloadKey = null;

// Concurrency-limited parallel execution (prevents browser connection exhaustion)
async function asyncPool(limit, items, fn) {
    const results = [];
    const executing = new Set();
    for (const item of items) {
        const p = fn(item).then(r => { executing.delete(p); return r; });
        executing.add(p);
        results.push(p);
        if (executing.size >= limit) await Promise.race(executing);
    }
    return Promise.all(results);
}

// Load config on page load
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

function _elementContains(el, target) {
    return !!(el && typeof el.contains === 'function' && el.contains(target));
}

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

/* ---- Color controls ---- */
const DEFAULT_RENDER_BACKGROUND_COLOR = '000000';
let renderColorMode = 'solve_score';
let renderColorInterpretation = 'scalar_lut';
let renderMatchMode = 'none';
let renderBackgroundColor = DEFAULT_RENDER_BACKGROUND_COLOR;
let renderRootProximityPalette = 'inferno';
let renderSolveScorePalette = 'inferno';
let renderSolveMetric = 'proximity';
let paletteTabPalette = 'inferno';
let paletteTabMetric = 'proximity';
let paletteTabColorInterpretation = 'scalar_lut';
let renderRootProximityBuiltinPalette = 'inferno';
let renderSolveScoreBuiltinPalette = 'inferno';
let paletteTabBuiltinPalette = 'inferno';
let renderRootProximityTriName = 'redgold';
let renderSolveScoreTriName = 'redgold';
let paletteTabTriName = 'redgold';
let renderRootProximityLongName = '';
let renderSolveScoreLongName = '';
let paletteTabLongName = '';
let repalettePalette = 'inferno';
let repaletteBuiltinPalette = 'inferno';
let repaletteTriName = 'redgold';
let repaletteLongName = '';
let colorRepalettePalette = 'inferno';
let colorRepaletteBuiltinPalette = 'inferno';
let colorRepaletteTriName = 'redgold';
let colorRepaletteLongName = '';
let _renderActiveFamily = 'color';
let _renderArtifacts = { color: [], bilevel: [], coeffs: [], palette: [], pdf: [] };
let _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1, pdf: -1 };
let _renderSelectedArtifactKey = { color: '', bilevel: '', coeffs: '', palette: '', pdf: '' };
let _renderCatalogScrollTop = { color: 0, bilevel: 0, coeffs: 0, palette: 0, pdf: 0 };
let _renderPreviewSelectionState = {
    artifactKey: '',
    rect: null,
};
let _renderPreviewDragState = {
    cleanup: null,
    dragging: false,
    artifactKey: '',
    start: null,
    rect: null,
};
let _renderLoresPreviewMeta = null;
let _renderLoresPreviewSelectionState = {
    rect: null,
};
let _renderLoresPreviewDragState = {
    cleanup: null,
    dragging: false,
    start: null,
    rect: null,
};
let _renderLoresPreviewActiveTab = 'plot';
let _renderLoresPreviewEmissionHistograms = [];
let _renderLoresPreviewHasPalette = false;
let _renderLoadedJobId = '';
let _renderNeedsRefresh = false;
let _favoriteRefs = [];              // /list-favorites panel rows; also the _isFavorite index
let _favoriteArtifacts = [];         // display rows = refs + derived public URLs
let _favoriteSelectedIdx = -1;
let _favoriteSelectedKey = '';
let _favoriteCatalogScrollTop = 0;
let _favoriteSaveDirHandle = null;
let _favoriteRefsLoaded = false;     // refs fetched at least once this session
// favorites-speedup.md Phase 1: cache-aware, idempotent, dedup'd tab load.
let _favoriteArtifactsReady = false;      // display rows built + valid for current refs
let _favoriteArtifactsSignature = '';     // signature of the refs the rows were built from
let _favoriteLoadPromise = null;          // shared in-flight load (dedup rapid tab clicks)
let _favoriteLoadGeneration = 0;          // stale-completion guard (force refresh bumps it)
let _triPopupState = { open: false, mode: null, filter: '', highlightIdx: 0 };
let _builtinPopupState = { open: false, mode: null, filter: '', highlightIdx: 0 };
let _longPopupState = { open: false, mode: null, filter: '', highlightIdx: 0 };
let _customPalettePopupState = {
    open: false,
    mode: null,
    loading: false,
    saving: false,
    selectedIdx: -1,
    working: [],
    savedSignature: '[]',
    status: '',
    error: '',
};
let _customPaletteCatalog = [];
let _customPaletteCatalogRevision = '';
let _customPaletteCatalogLoaded = false;
let _customPaletteCatalogLoadPromise = null;
let _customPaletteSelectionByMode = {};
let _micPopupState = { open: false, mode: null, filter: '', highlightIdx: 0 };
let _micPaletteCatalog = null;
let _micPaletteCatalogLoadPromise = null;
let _micPaletteCatalogError = '';
let _micPaletteCredit = '';
let _micPaletteSelectionByMode = {};
let _autolevelPopupState = { open: false, sourceArtifactId: '', sourceImageKey: '' };
let _resizePopupState = { open: false, sourceArtifactId: '', sourceImageKey: '' };
let _repalettePopupState = { open: false, sourcePaletteId: '', sourceDisplayName: '' };
let _colorRepalettePopupState = { open: false, sourceArtifactId: '', sourceDisplayName: '', interpretation: '' };
let _bilevelPopupState = { open: false, sectionMode: 'logical_sections_auto', sectionCount: '' };
let _colorToBilevelPopupState = { open: false, sourceArtifactId: '', threshold: 0 };
let _resultsRefreshPopupState = { open: false, workers: 32 };
let _renderMtPopupState = {
    open: false,
    rasterThreads: 4,
    rasterWorkers: 10,
    solveScoreThreads: 4,
    rasterRetries: 2,
    finalizeWorkers: 16,
    rasterSectionMode: 'logical_sections_auto',
    rasterSectionCount: '',
    saveAssociatedPalette: false,
};
function _defaultComputePopupPrefs(solverMode) {
    return {
        solverMode: solverMode || 'aberth_mt',
        fused: true,
        nChunks: 10,
        fusedThreads: 4,
        loresParamGenThreads: 1,
        loresCoeffgenThreads: 1,
        probe: null,
        probeError: '',
        probeLoading: false,
        probeSignature: '',
    };
}

let _computePopupPrefsBySolver = {
    aberth_mt: _defaultComputePopupPrefs('aberth_mt'),
    companion_matrix: _defaultComputePopupPrefs('companion_matrix'),
};

let _computeMtPopupState = {
    open: false,
    ..._defaultComputePopupPrefs('aberth_mt'),
};
let _extractPalettePopupState = {
    open: false,
    solveScoreThreads: 4,
    histInputMode: 'tmpfile',
    histRetries: 2,
    mergeWorkers: 16,
    chunkThreads: 4,
    chunkInputMode: 'sectioned',
    chunkRetries: 2,
    chunkWorkers: 16,
};
let _pdfColorSpreadPopupState = { open: false, filter: '', highlightIdx: 0, sourceArtifactId: '' };
let _functionPopupState = { open: false, filter: '', highlightIdx: 0 };

function _stopsToGradient(stops) {
    return `linear-gradient(to right, ${stops.map((s, i) => s + ' ' + Math.round(i/(stops.length-1)*100) + '%').join(', ')})`;
}

const PALETTE_DEFS = [
    { name: 'inferno',  stops: ['#000004','#420a68','#932667','#dd513a','#fca50a','#fcffa4'] },
    { name: 'viridis',  stops: ['#440154','#3b528b','#21918c','#5ec962','#fde725','#fde725'] },
    { name: 'magma',    stops: ['#000004','#51127c','#b73779','#fc8961','#fcfdbf','#fcfdbf'] },
    { name: 'plasma',   stops: ['#0d0887','#7e03a8','#cc4778','#f89540','#f0f921','#f0f921'] },
    { name: 'turbo',    stops: ['#30123b','#28bbec','#a2fc3c','#fb8022','#7a0403','#7a0403'] },
    { name: 'cividis',  stops: ['#00204c','#31446b','#666870','#958f78','#cebe5a','#fde724'] },
    { name: 'identity', stops: ['#000000','#ffffff'] },
    { name: 'identity_hsv', stops: ['#000000','#20201c','#384030','#3c6045','#40806e','#3c56a0','#8240c2','#d224d4','#ff0000'] },
    { name: 'warm',     stops: ['#6e40aa','#e04f7e','#f19938','#c3e032','#aff05b','#aff05b'] },
    { name: 'cool',     stops: ['#6e40aa','#4775de','#1bb5a8','#52f667','#aff05b','#aff05b'] },
    { name: 'bwred',    stops: ['#2166ac','#67a9cf','#c7e5f4','#fde0dd','#e57e6b','#960612'] },
    { name: 'neon_v',   stops: ['#ff0080','#550070','#0f0519','#1d1e4b','#5a8cd2','#8cffff'] },
    { name: 'gilded',   stops: ['#ffd700','#c89b14','#64142d','#2d0a19','#9119b4','#dc64ff'] },
    { name: 'reef',     stops: ['#00ffdc','#008c8c','#0a2340','#0c1223','#a04b0f','#ffc832'] },
    { name: 'abyss',    stops: ['#beff32','#6e9e1c','#1e2616','#0a0f14','#371c64','#d26eff'] },
    { name: 'rainbow_d3', stops: ['#6e40aa','#bd3caf','#ff5473','#ff853d','#d7c33b','#80f558','#24e794','#1ccbbb','#3a9ee0','#5b5bcf','#6e40aa'] },
];

const BUILTIN_PALETTE_ENTRIES = PALETTE_DEFS.map(def => ({
    ...def,
    palette_id: def.name,
    gradient_css: _stopsToGradient(def.stops),
    search_text: def.name.toLowerCase(),
}));

// --- Named custom hex-stop palettes -------------------------------------
// The renderer's canonical wire value remains custom:rrggbb-rrggbb-...
// Display names live in a separately persisted catalog and travel beside the
// wire value as immutable artifact provenance.
const CUSTOM_PALETTE_MAX_STOPS = 32;
const CUSTOM_PALETTE_MAX_ENTRIES = 256;
const CUSTOM_PALETTE_MAX_NAME_LEN = 80;

function _parseCustomPaletteStops(text) {
    const value = String(text || '').trim();
    if (!value) return null;
    if (value.toLowerCase().startsWith('custom:')) return _customStopsFromName(value);
    const tokens = value.split(/[\s,;]+/).filter(Boolean);
    const stops = [];
    for (const token of tokens) {
        const hex = token.startsWith('#') ? token.slice(1) : token;
        if (!/^[0-9a-fA-F]{6}$/.test(hex)) return null;
        stops.push('#' + hex.toLowerCase());
    }
    if (stops.length < 2 || stops.length > CUSTOM_PALETTE_MAX_STOPS) return null;
    return stops;
}

function _customPaletteNameFromStops(stops) {
    return 'custom:' + stops.map(s => s.slice(1)).join('-');
}

function _customStopsFromName(name) {
    if (typeof name !== 'string' || !name.toLowerCase().startsWith('custom:')) return null;
    const groups = name.slice(7).split('-');
    if (groups.length < 2 || groups.length > CUSTOM_PALETTE_MAX_STOPS) return null;
    if (!groups.every(g => /^[0-9a-fA-F]{6}$/.test(g))) return null;
    return groups.map(g => '#' + g.toLowerCase());
}

function _customPaletteEntry(raw) {
    if (!raw || typeof raw !== 'object') return null;
    const name = String(raw.name || '').trim();
    const sourceStops = Array.isArray(raw.stops) ? raw.stops : [];
    const stops = sourceStops.map(stop => {
        const value = String(stop || '').trim();
        return '#' + (value.startsWith('#') ? value.slice(1) : value).toLowerCase();
    });
    if (!name || name.length > CUSTOM_PALETTE_MAX_NAME_LEN) return null;
    if (stops.length < 2 || stops.length > CUSTOM_PALETTE_MAX_STOPS) return null;
    if (!stops.every(stop => /^#[0-9a-f]{6}$/.test(stop))) return null;
    return {
        name,
        stops,
        hexText: stops.join(', '),
        palette: _customPaletteNameFromStops(stops),
    };
}

function _customPaletteClone(entry) {
    return {
        name: String(entry.name || ''),
        stops: Array.isArray(entry.stops) ? entry.stops.slice() : [],
        hexText: String(entry.hexText || ''),
        palette: String(entry.palette || ''),
    };
}

function _customPaletteSignature(entries) {
    return JSON.stringify((entries || []).map(entry => ({
        name: String(entry.name || '').trim(),
        hexText: String(entry.hexText || '').trim(),
    })));
}

function _customPalettePopupDirty() {
    return _customPaletteSignature(_customPalettePopupState.working)
        !== _customPalettePopupState.savedSignature;
}

function _customPaletteEntryBySpec(palette) {
    return _customPaletteCatalog.find(entry => entry.palette === palette) || null;
}

function _rememberCustomPaletteForMode(mode, palette, displayName) {
    const spec = String(palette || '');
    if (!_customStopsFromName(spec)) return;
    const catalogEntry = _customPaletteEntryBySpec(spec);
    _customPaletteSelectionByMode[mode] = {
        palette: spec,
        displayName: String(displayName || (catalogEntry && catalogEntry.name) || '').trim(),
    };
}

function _customPaletteForMode(mode) {
    const current = _currentPaletteForMode(mode);
    const remembered = _customPaletteSelectionByMode[mode];
    if (remembered && remembered.palette) return remembered;
    if (_customStopsFromName(current)) {
        const entry = _customPaletteEntryBySpec(current);
        return {
            palette: current,
            displayName: String((entry && entry.name) || '').trim(),
        };
    }
    return null;
}

function _paletteDisplayNameForMode(mode) {
    const current = _currentPaletteForMode(mode);
    if (!_customStopsFromName(current)) return '';
    const remembered = _customPaletteSelectionByMode[mode];
    if (remembered && remembered.palette === current) {
        return String(remembered.displayName || '').trim();
    }
    const entry = _customPaletteEntryBySpec(current);
    return entry ? entry.name : '';
}

function _activeRenderPaletteDisplayName() {
    return _paletteDisplayNameForMode('solve_score');
}

function _paletteContainerId(mode) {
    return mode === 'proximity' ? 'palette-circles-root-proximity'
        : mode === 'solve_score' ? 'palette-circles-solve-score'
        : mode === 'repalette' ? 'palette-circles-repalette'
        : mode === 'color_repalette' ? 'palette-circles-color-repalette'
        : 'palette-circles-palette-tab';
}

function _triCatalog() {
    return Array.isArray(window._triPaletteCatalog) ? window._triPaletteCatalog : [];
}

function _longCatalog() {
    return Array.isArray(window._longPaletteCatalog) ? window._longPaletteCatalog : [];
}

function _modeUsesBuiltinPopup(mode) {
    return mode === 'proximity' || mode === 'solve_score' || mode === 'palette_tab' || mode === 'repalette' || mode === 'color_repalette';
}

function _builtinPaletteEntryByName(name) {
    return BUILTIN_PALETTE_ENTRIES.find(entry => entry.name === name) || null;
}

function _defaultBuiltinPaletteName() {
    return BUILTIN_PALETTE_ENTRIES.length ? BUILTIN_PALETTE_ENTRIES[0].name : 'inferno';
}

function _builtinPaletteForMode(mode) {
    if (mode === 'proximity') return renderRootProximityBuiltinPalette;
    if (mode === 'solve_score') return renderSolveScoreBuiltinPalette;
    if (mode === 'repalette') return repaletteBuiltinPalette;
    if (mode === 'color_repalette') return colorRepaletteBuiltinPalette;
    return paletteTabBuiltinPalette;
}

function _setRememberedBuiltinPalette(mode, paletteName) {
    if (mode === 'proximity') renderRootProximityBuiltinPalette = paletteName;
    else if (mode === 'solve_score') renderSolveScoreBuiltinPalette = paletteName;
    else if (mode === 'repalette') repaletteBuiltinPalette = paletteName;
    else if (mode === 'color_repalette') colorRepaletteBuiltinPalette = paletteName;
    else paletteTabBuiltinPalette = paletteName;
}

function _syncBuiltinDefaults() {
    const fallback = _defaultBuiltinPaletteName();
    ['proximity', 'solve_score', 'palette_tab', 'repalette', 'color_repalette'].forEach(mode => {
        const entry = _builtinPaletteEntryByName(_builtinPaletteForMode(mode));
        if (!entry) _setRememberedBuiltinPalette(mode, fallback);
    });
}

function _triCatalogAvailable() {
    return _triCatalog().length > 0;
}

function _longCatalogAvailable() {
    return _longCatalog().length > 0;
}

function _triPaletteEntryByName(name) {
    return _triCatalog().find(entry => entry.name === name) || null;
}

function _longPaletteEntryByName(name) {
    return _longCatalog().find(entry => entry.name === name) || null;
}

function _defaultTriPaletteName() {
    const cat = _triCatalog();
    return cat.length ? cat[0].name : 'redgold';
}

function _defaultLongPaletteName() {
    const cat = _longCatalog();
    return cat.length ? cat[0].name : '';
}

function _triPaletteForMode(mode) {
    if (mode === 'proximity') return renderRootProximityTriName;
    if (mode === 'solve_score') return renderSolveScoreTriName;
    if (mode === 'repalette') return repaletteTriName;
    if (mode === 'color_repalette') return colorRepaletteTriName;
    return paletteTabTriName;
}

function _longPaletteForMode(mode) {
    if (mode === 'proximity') return renderRootProximityLongName;
    if (mode === 'solve_score') return renderSolveScoreLongName;
    if (mode === 'repalette') return repaletteLongName;
    if (mode === 'color_repalette') return colorRepaletteLongName;
    return paletteTabLongName;
}

function _setRememberedTriPalette(mode, triName) {
    if (mode === 'proximity') renderRootProximityTriName = triName;
    else if (mode === 'solve_score') renderSolveScoreTriName = triName;
    else if (mode === 'repalette') repaletteTriName = triName;
    else if (mode === 'color_repalette') colorRepaletteTriName = triName;
    else paletteTabTriName = triName;
}

function _setRememberedLongPalette(mode, longName) {
    if (mode === 'proximity') renderRootProximityLongName = longName;
    else if (mode === 'solve_score') renderSolveScoreLongName = longName;
    else if (mode === 'repalette') repaletteLongName = longName;
    else if (mode === 'color_repalette') colorRepaletteLongName = longName;
    else paletteTabLongName = longName;
}

function _currentPaletteForMode(mode) {
    if (mode === 'proximity') return renderRootProximityPalette;
    if (mode === 'solve_score') return renderSolveScorePalette;
    if (mode === 'repalette') return repalettePalette;
    if (mode === 'color_repalette') return colorRepalettePalette;
    return paletteTabPalette;
}

function _syncTriDefaults() {
    const fallback = _defaultTriPaletteName();
    ['proximity', 'solve_score', 'palette_tab', 'repalette', 'color_repalette'].forEach(mode => {
        const name = _triPaletteForMode(mode);
        if (!_triPaletteEntryByName(name)) _setRememberedTriPalette(mode, fallback);
    });
}

function _syncLongDefaults() {
    const fallback = _defaultLongPaletteName();
    ['proximity', 'solve_score', 'palette_tab', 'repalette', 'color_repalette'].forEach(mode => {
        const name = _longPaletteForMode(mode);
        if (!_longPaletteEntryByName(name)) _setRememberedLongPalette(mode, fallback);
    });
}

function _triSwatchTitle(mode) {
    if (!_triCatalogAvailable()) return 'TRI catalog unavailable';
    const entry = _triPaletteEntryByName(_triPaletteForMode(mode));
    if (!entry) return 'TRI unavailable';
    let title = 'TRI: ' + entry.name;
    if (entry.aliases && entry.aliases.length) title += ' (aliases: ' + entry.aliases.join(', ') + ')';
    return title;
}

function _longSwatchTitle(mode) {
    if (!_longCatalogAvailable()) return 'LONG catalog unavailable';
    const entry = _longPaletteEntryByName(_longPaletteForMode(mode));
    if (!entry) return 'LONG unavailable';
    let title = 'LONG: ' + entry.name;
    if (entry.aliases && entry.aliases.length) title += ' (aliases: ' + entry.aliases.join(', ') + ')';
    return title;
}

function _visibleTriPaletteCatalog() {
    const filter = (_triPopupState.filter || '').trim().toLowerCase();
    const cat = _triCatalog();
    if (!filter) return cat;
    return cat.filter(entry => (entry.search_text || '').includes(filter));
}

function _visibleLongPaletteCatalog() {
    const filter = (_longPopupState.filter || '').trim().toLowerCase();
    const cat = _longCatalog();
    if (!filter) return cat;
    return cat.filter(entry => (entry.search_text || '').includes(filter));
}

function _builtinSwatchTitle(mode) {
    const entry = _builtinPaletteEntryByName(_builtinPaletteForMode(mode));
    return entry ? ('Palette: ' + entry.name) : 'Palette unavailable';
}

function _visibleBuiltinPaletteCatalog() {
    const filter = (_builtinPopupState.filter || '').trim().toLowerCase();
    if (!filter) return BUILTIN_PALETTE_ENTRIES;
    return BUILTIN_PALETTE_ENTRIES.filter(entry => entry.search_text.includes(filter));
}

function _popupModeLabel(mode) {
    return mode === 'proximity' ? 'Root proximity'
        : mode === 'solve_score' ? 'Solve score'
        : mode === 'repalette' ? 'RePalette'
        : mode === 'color_repalette' ? 'Color RePalette'
        : 'Palette tab';
}

function _renderPaletteRow(mode) {
    const container = document.getElementById(_paletteContainerId(mode));
    if (!container) return;
    const currentPalette = _currentPaletteForMode(mode);
    const children = [];
    if (_modeUsesBuiltinPopup(mode)) {
        const builtinEntry = _builtinPaletteEntryByName(_builtinPaletteForMode(mode)) || BUILTIN_PALETTE_ENTRIES[0];
        const builtinActive = !!_builtinPaletteEntryByName(currentPalette);
        const builtin = document.createElement('div');
        builtin.className = 'pal-circle pal-circle-builtin' + (builtinActive ? ' active' : '');
        builtin.dataset.palettePopup = 'builtin';
        builtin.textContent = 'PAL';
        builtin.title = _builtinSwatchTitle(mode);
        builtin.style.background = builtinEntry ? builtinEntry.gradient_css : '#555';
        builtin.onclick = () => _openBuiltinPalettePopup(mode);
        children.push(builtin);
    } else {
        PALETTE_DEFS.forEach(p => {
            const el = document.createElement('div');
            el.className = 'pal-circle' + (p.name === currentPalette ? ' active' : '');
            el.dataset.palette = p.name;
            el.title = p.name;
            el.style.background = _stopsToGradient(p.stops);
            el.onclick = () => {
                setPaletteForMode(mode, p.name);
                if (mode === 'proximity' || mode === 'solve_score') setColorMode(mode);
            };
            children.push(el);
        });
    }

    const tri = document.createElement('div');
    const triEntry = _triPaletteEntryByName(_triPaletteForMode(mode));
    const triActive = typeof currentPalette === 'string' && currentPalette.startsWith('tri_');
    tri.className = 'pal-circle pal-circle-tri' + (triActive ? ' active' : '') + (_triCatalogAvailable() ? '' : ' disabled');
    tri.textContent = 'TRI';
    tri.title = _triSwatchTitle(mode);
    tri.style.background = triEntry ? triEntry.gradient_css : '#555';
    tri.onclick = (ev) => {
        if (!_triCatalogAvailable()) return;
        if (ev && ev.altKey) {
            _setTriPaletteForMode(mode, _triPaletteForMode(mode), true);
            return;
        }
        _openTriPalettePopup(mode);
    };
    tri.oncontextmenu = (ev) => {
        if (ev && typeof ev.preventDefault === 'function') ev.preventDefault();
        if (_triCatalogAvailable()) _setTriPaletteForMode(mode, _triPaletteForMode(mode), true);
        return false;
    };
    children.push(tri);

    const longEntry = _longPaletteEntryByName(_longPaletteForMode(mode));
    const longActive = typeof currentPalette === 'string' && currentPalette.startsWith('long_');
    const longSwatch = document.createElement('div');
    longSwatch.className = 'pal-circle pal-circle-builtin pal-circle-long' + (longActive ? ' active' : '') + (_longCatalogAvailable() ? '' : ' disabled');
    longSwatch.dataset.palettePopup = 'long';
    longSwatch.textContent = 'LONG';
    longSwatch.title = _longSwatchTitle(mode);
    longSwatch.style.background = longEntry ? longEntry.gradient_css : '#555';
    longSwatch.onclick = () => {
        if (!_longCatalogAvailable()) return;
        _openLongPalettePopup(mode);
    };
    children.push(longSwatch);

    const micSel = _micPaletteSelectionByMode[mode];
    const micActive = !!(micSel && micSel.palette && micSel.palette === currentPalette);
    const rememberedCustom = _customPaletteForMode(mode);
    const customStops = rememberedCustom
        ? _customStopsFromName(rememberedCustom.palette)
        : _customStopsFromName(currentPalette);
    // A MIC pick is a custom: spec too — exactly one swatch may be live,
    // and the MIC swatch owns the active state for its own selection.
    const customActive = typeof currentPalette === 'string' && currentPalette.startsWith('custom:') && !micActive;
    const customSwatch = document.createElement('div');
    customSwatch.className = 'pal-circle pal-circle-builtin pal-circle-custom' + (customActive ? ' active' : '');
    customSwatch.textContent = 'HEX';
    customSwatch.title = customStops
        ? `${(rememberedCustom && rememberedCustom.displayName) || 'Custom palette'} (${customStops.length} stops)`
        : 'Named custom palettes';
    customSwatch.style.background = customStops ? _stopsToGradient(customStops) : '#555';
    customSwatch.onclick = () => _openCustomPalettePopup(mode);
    children.push(customSwatch);

    const micStops = micSel ? _customStopsFromName(micSel.palette) : null;
    const micSwatch = document.createElement('div');
    micSwatch.className = 'pal-circle pal-circle-builtin pal-circle-mic' + (micActive ? ' active' : '');
    micSwatch.dataset.palettePopup = 'mic';
    micSwatch.textContent = 'MIC';
    micSwatch.title = micSel && micSel.displayName
        ? `MIC: ${micSel.displayName}`
        : 'Meditations in Color: museum artwork palettes';
    micSwatch.style.background = micStops ? _stopsToGradient(micStops) : _stopsToGradient(MIC_SWATCH_DEFAULT_STOPS);
    micSwatch.onclick = () => _openMicPalettePopup(mode);
    children.push(micSwatch);

    container.replaceChildren();
    children.forEach(child => container.appendChild(child));
}

function _syncSolveScoreOmegaUi(prefix) {
    try {
        _syncSolveScoreLegacyInputs(prefix);
    } catch (e) {
        // no-op; invalid chains are prevented at add-time
    }
}

function _omegaSummaryLabel(enabled, omega) {
    return enabled ? `w=${Number(omega).toFixed(0)}` : 'w=off';
}

function setColorMode(mode) {
    renderColorMode = 'solve_score';
    _updateSolveScoreButtons();
}

function _normalizeColorInterpretation(value) {
    const v = String(value || '').trim().toLowerCase();
    if (v === 'scalar_palette' || v === 'palette' || v === 'scalar') return 'scalar_lut';
    if (v === 'direct_rgb') return 'rgb';
    if (v === 'rgb-lut') return 'rgb_lut';
    if (v === 'hsv-lut') return 'hsv_lut';
    if (v === 'rgb' || v === 'hsv' || v === 'scalar_lut' || v === 'rgb_lut' || v === 'hsv_lut') return v;
    return 'scalar_lut';
}

function _colorInterpretationLabel(value) {
    const mode = _normalizeColorInterpretation(value);
    if (mode === 'scalar_lut') return 'Scalar LUT';
    if (mode === 'rgb') return 'RGB';
    if (mode === 'hsv') return 'HSV';
    if (mode === 'rgb_lut') return 'RGB LUT';
    if (mode === 'hsv_lut') return 'HSV LUT';
    return mode;
}

function _artifactColorInterpretation(art) {
    const raw = String(
        (art && (
            art.color_interpretation ||
            art.score_output_interpretation ||
            art.raw_interpretation ||
            art.interpretation ||
            (art.render_execution && art.render_execution.color_interpretation)
        )) || ''
    ).trim();
    return raw ? _normalizeColorInterpretation(raw) : 'scalar_lut';
}

function _colorInterpretationUsesPalette(mode) {
    const normalized = _normalizeColorInterpretation(mode);
    return normalized === 'scalar_lut' || normalized === 'rgb_lut' || normalized === 'hsv_lut';
}

function _selectedRenderColorInterpretation() {
    const checked = document.querySelector('input[name="render-color-interpretation"]:checked');
    renderColorInterpretation = _normalizeColorInterpretation(checked ? checked.value : renderColorInterpretation);
    return renderColorInterpretation;
}

function _selectedPaletteColorInterpretation() {
    const el = document.getElementById('palette-color-interpretation');
    paletteTabColorInterpretation = _normalizeColorInterpretation(el ? el.value : paletteTabColorInterpretation);
    return paletteTabColorInterpretation;
}

function _solveScoreColorCompatibility(compiled, interpretation = _selectedRenderColorInterpretation()) {
    const count = Number(compiled && compiled.output_channel_count) || 1;
    const mode = _normalizeColorInterpretation(interpretation);
    if (mode === 'scalar_lut' && count !== 1) return `program incompatible with Scalar LUT: expected 1 output, got ${count}`;
    if (mode === 'rgb' && count !== 3) return `program incompatible with RGB: expected 3 outputs, got ${count}`;
    if (mode === 'hsv' && count !== 3) return `program incompatible with HSV: expected 3 outputs, got ${count}`;
    if (mode === 'rgb_lut' && count !== 3) return `program incompatible with RGB LUT: expected 3 outputs, got ${count}`;
    if (mode === 'hsv_lut' && count !== 3) return `program incompatible with HSV LUT: expected 3 outputs, got ${count}`;
    return '';
}

function _solveScorePaletteCompatibility(compiled, interpretation) {
    const mode = interpretation == null
        ? (typeof _selectedPaletteColorInterpretation === 'function' ? _selectedPaletteColorInterpretation() : 'scalar_lut')
        : interpretation;
    return _solveScoreColorCompatibility(compiled, mode);
}

function _syncRenderColorInterpretationUi() {
    const mode = _selectedRenderColorInterpretation();
    document.querySelectorAll('[data-color-interpretation-row]').forEach(row => {
        row.classList.toggle('active', row.getAttribute('data-color-interpretation-row') === mode);
    });
    const paletteRow = document.getElementById('render-color-lut-palette-row');
    if (paletteRow) paletteRow.style.display = _colorInterpretationUsesPalette(mode) ? 'grid' : 'none';
    _syncScoreNormalizationUi();
    _updateSolveScoreButtons();
}

function _setRenderColorInterpretation(value) {
    renderColorInterpretation = _normalizeColorInterpretation(value);
    const radio = document.querySelector(`input[name="render-color-interpretation"][value="${renderColorInterpretation}"]`);
    if (radio) radio.checked = true;
    _syncRenderColorInterpretationUi();
}

function _syncPaletteColorInterpretationUi() {
    const mode = _selectedPaletteColorInterpretation();
    const select = document.getElementById('palette-color-interpretation');
    if (select) select.value = mode;
    const paletteRow = document.getElementById('palette-circles-palette-tab')?.closest('.color-row');
    if (paletteRow) paletteRow.style.display = _colorInterpretationUsesPalette(mode) ? '' : 'none';
    const help = document.getElementById('palette-color-interpretation-help');
    if (help) {
        help.textContent = _colorInterpretationUsesPalette(mode)
            ? 'Uses the selected palette LUT.'
            : 'Direct color mode; palette name is ignored.';
    }
    _updatePaletteCreateButton();
}

function _setPaletteColorInterpretation(value) {
    paletteTabColorInterpretation = _normalizeColorInterpretation(value);
    _syncPaletteColorInterpretationUi();
}

function _syncScoreNormalizationUi() {
    const el = document.getElementById('render-score-normalization');
    if (!el) return;
    let hasExplicit = false;
    try {
        hasExplicit = !!_compileSolveScoreChain(_chainForWhich('ss'), renderSolveMetric).has_explicit_outputs;
    } catch (_) {
        try {
            hasExplicit = _normalizeSolveScoreChain(_chainForWhich('ss'), renderSolveMetric).some(item => item.name === 'emit');
        } catch (_) {
            hasExplicit = false;
        }
    }
    el.disabled = hasExplicit;
    if (hasExplicit) el.checked = false;
    const wrap = el.closest('label');
    if (wrap) {
        wrap.style.opacity = hasExplicit ? '0.45' : '';
        wrap.title = hasExplicit
            ? 'Explicit emit chips own output normalization'
            : 'Normalize the implicit scalar score output using lores q05/q95';
    }
}

let _lastCalcHasLores = false;  // set by renderArtifactPanel when calc is loaded

function _updateSolveScoreButtons() {
    const isSolve = renderColorMode === 'solve_score';
    const hasJob = !!(document.getElementById('render-results-dir')?.value?.trim());
    let chainValid = true;
    let issue = '';
    try {
        const compiled = _compileSolveScoreChain(_chainForWhich('ss'), renderSolveMetric);
        issue = _renderActiveFamily === 'palette'
            ? _solveScorePaletteCompatibility(compiled, _selectedRenderColorInterpretation())
            : _solveScoreColorCompatibility(compiled, _selectedRenderColorInterpretation());
        chainValid = !issue;
    }
    catch (e) {
        chainValid = false;
        issue = `invalid program: ${e && e.message ? e.message : String(e)}`;
    }
    const enabled = isSolve && hasJob && _lastCalcHasLores && chainValid;
    const palBtn = document.getElementById('btn-render-generate');
    const histBtn = document.getElementById('btn-solve-histogram');
    if (palBtn && _renderActiveFamily === 'palette') { palBtn.disabled = !enabled; palBtn.style.opacity = enabled ? '' : '0.4'; }
    if (histBtn) { histBtn.disabled = !enabled; histBtn.style.opacity = enabled ? '' : '0.4'; }
    const statusEl = document.getElementById('render-solve-score-program-status');
    if (statusEl) {
        statusEl.textContent = issue || '';
        statusEl.className = `solve-score-program-status${issue ? ' error' : ''}`;
    }
}

function _updatePaletteCreateButton() {
    const btn = document.getElementById('btn-palette-create');
    if (!btn) return;
    let enabled = true;
    let issue = '';
    try {
        const compiled = _compileSolveScoreChain(_chainForWhich('palette-ss'), paletteTabMetric);
        issue = _solveScorePaletteCompatibility(compiled, _selectedPaletteColorInterpretation());
        enabled = !issue;
    }
    catch (e) { enabled = false; issue = e && e.message ? e.message : String(e); }
    btn.disabled = !enabled || !!_activePaletteRun;
    btn.style.opacity = enabled ? '' : '0.4';
    btn.title = issue || 'Generate a solve-score palette/color artifact';
    const labelEl = document.getElementById('palette-ss-stack-label');
    if (labelEl && issue) labelEl.textContent = issue;
}

function setMatch(mode) {
    renderMatchMode = mode;
    document.querySelectorAll('.strat-chip').forEach(c => c.classList.toggle('active', c.dataset.match === mode));
}

function setPaletteForMode(mode, name, displayName = '') {
    if (mode === 'proximity') renderRootProximityPalette = name;
    else if (mode === 'solve_score') renderSolveScorePalette = name;
    else if (mode === 'repalette') repalettePalette = name;
    else if (mode === 'color_repalette') colorRepalettePalette = name;
    else if (mode === 'palette_tab') paletteTabPalette = name;
    const builtinEntry = _builtinPaletteEntryByName(name);
    if (builtinEntry) _setRememberedBuiltinPalette(mode, builtinEntry.name);
    else if (typeof name === 'string' && name.startsWith('tri_')) {
        const triName = name.slice(4);
        if (_triPaletteEntryByName(triName)) _setRememberedTriPalette(mode, triName);
    } else if (typeof name === 'string' && name.startsWith('long_')) {
        const longName = name.slice(5);
        if (_longPaletteEntryByName(longName)) _setRememberedLongPalette(mode, longName);
    } else if (typeof name === 'string' && name.startsWith('custom:')) {
        _rememberCustomPaletteForMode(mode, name, displayName);
    }
    _renderPaletteRow(mode);
    if (mode === 'repalette' && _repalettePopupState.open) _renderRepalettePopup();
    if (mode === 'color_repalette' && _colorRepalettePopupState.open) _renderColorRepalettePopup();
    if (_renderMtPopupState.open) _renderRenderMtPopup();
}

function _activeRenderPalette() {
    return renderSolveScorePalette;
}

function setSolveMetric(name) {
    if (!_solveScoreMetricSet.has(name)) return;
    const legacy = _legacySolveScoreState('render');
    _setSolveScoreChainFromLegacy('render', name, legacy.quantile * 100, legacy.omega, legacy.omega_enabled, legacy.omega_phase);
    setColorMode('solve_score');
}

function _setTriPaletteForMode(mode, triName, activate = true) {
    if (!_triCatalogAvailable()) return;
    const entry = _triPaletteEntryByName(triName) || _triCatalog()[0];
    if (!entry) return;
    _setRememberedTriPalette(mode, entry.name);
    if (activate) setPaletteForMode(mode, entry.palette_id);
    else _renderPaletteRow(mode);
    if (activate && (mode === 'proximity' || mode === 'solve_score')) setColorMode(mode);
}

function _setBuiltinPaletteForMode(mode, paletteName, activate = true) {
    const entry = _builtinPaletteEntryByName(paletteName) || BUILTIN_PALETTE_ENTRIES[0];
    if (!entry) return;
    _setRememberedBuiltinPalette(mode, entry.name);
    if (activate) setPaletteForMode(mode, entry.palette_id);
    else _renderPaletteRow(mode);
    if (activate && (mode === 'proximity' || mode === 'solve_score')) setColorMode(mode);
}

function _setLongPaletteForMode(mode, longName, activate = true) {
    if (!_longCatalogAvailable()) return;
    const entry = _longPaletteEntryByName(longName) || _longCatalog()[0];
    if (!entry) return;
    _setRememberedLongPalette(mode, entry.name);
    if (activate) setPaletteForMode(mode, entry.palette_id);
    else _renderPaletteRow(mode);
    if (activate && (mode === 'proximity' || mode === 'solve_score')) setColorMode(mode);
}

function _resetCustomPaletteWorking() {
    _customPalettePopupState.working = _customPaletteCatalog.map(_customPaletteClone);
    _customPalettePopupState.savedSignature = _customPaletteSignature(_customPalettePopupState.working);
    const remembered = _customPaletteForMode(_customPalettePopupState.mode);
    const selectedIdx = remembered
        ? _customPalettePopupState.working.findIndex(entry => entry.palette === remembered.palette)
        : -1;
    _customPalettePopupState.selectedIdx = selectedIdx;
    _customPalettePopupState.error = '';
    _customPalettePopupState.status = selectedIdx < 0 && remembered
        ? 'Current HEX colors are not saved. New will import them.'
        : '';
}

async function _loadCustomPaletteCatalog(force = false) {
    if (_customPaletteCatalogLoaded && !force) return _customPaletteCatalog;
    if (_customPaletteCatalogLoadPromise && !force) return _customPaletteCatalogLoadPromise;
    const promise = (async () => {
        const response = await lambdaPost('storage', {}, '/list-custom-palettes');
        const rows = Array.isArray(response && response.palettes) ? response.palettes : [];
        const entries = rows.map(_customPaletteEntry);
        if (entries.some(entry => !entry)) {
            throw new Error('Saved custom palette catalog contains an invalid row');
        }
        _customPaletteCatalog = entries;
        _customPaletteCatalogRevision = String((response && response.revision) || '');
        _customPaletteCatalogLoaded = true;
        ['proximity', 'solve_score', 'palette_tab', 'repalette', 'color_repalette']
            .forEach(mode => _renderPaletteRow(mode));
        return entries;
    })();
    _customPaletteCatalogLoadPromise = promise;
    try {
        return await promise;
    } finally {
        if (_customPaletteCatalogLoadPromise === promise) {
            _customPaletteCatalogLoadPromise = null;
        }
    }
}

function _customPaletteValidation(entries) {
    if (!Array.isArray(entries)) return { error: 'Palette list is invalid.' };
    if (entries.length > CUSTOM_PALETTE_MAX_ENTRIES) {
        return { error: `At most ${CUSTOM_PALETTE_MAX_ENTRIES} custom palettes are allowed.` };
    }
    const names = new Set();
    const palettes = new Set();
    const payload = [];
    for (let idx = 0; idx < entries.length; idx += 1) {
        const entry = entries[idx] || {};
        const name = String(entry.name || '').trim();
        if (!name) return { error: `Row ${idx + 1} requires a name.` };
        if (name.length > CUSTOM_PALETTE_MAX_NAME_LEN) {
            return { error: `"${name}" exceeds ${CUSTOM_PALETTE_MAX_NAME_LEN} characters.` };
        }
        if (/[\u0000-\u001f\u007f]/.test(name)) {
            return { error: `"${name}" contains control characters.` };
        }
        const stops = _parseCustomPaletteStops(entry.hexText);
        if (!stops) {
            return { error: `"${name}" requires 2–${CUSTOM_PALETTE_MAX_STOPS} six-digit hex colors.` };
        }
        const palette = _customPaletteNameFromStops(stops);
        const nameKey = name.toLocaleLowerCase();
        if (names.has(nameKey)) return { error: `Duplicate palette name: "${name}".` };
        if (palettes.has(palette)) return { error: `"${name}" duplicates another row's colors.` };
        names.add(nameKey);
        palettes.add(palette);
        payload.push({ name, stops: stops.map(stop => stop.slice(1)) });
    }
    return { payload };
}

function _renderCustomPaletteRows() {
    const body = document.getElementById('custom-palette-popup-body');
    if (!body) return;
    body.replaceChildren();
    const rows = _customPalettePopupState.working;
    if (!rows.length) {
        const row = document.createElement('tr');
        row.className = 'tri-popup-empty';
        const cell = document.createElement('td');
        cell.colSpan = 3;
        cell.textContent = _customPalettePopupState.loading
            ? 'Loading custom palettes...'
            : 'No custom palettes. Click New to add one.';
        row.appendChild(cell);
        body.appendChild(row);
        return;
    }
    const current = _currentPaletteForMode(_customPalettePopupState.mode);
    rows.forEach((entry, idx) => {
        const stops = _parseCustomPaletteStops(entry.hexText);
        const palette = stops ? _customPaletteNameFromStops(stops) : '';
        entry.stops = stops || [];
        entry.palette = palette;

        const row = document.createElement('tr');
        const classes = ['tri-popup-row'];
        if (palette && palette === current) classes.push('active');
        if (idx === _customPalettePopupState.selectedIdx) classes.push('highlight');
        if (!stops || !String(entry.name || '').trim()) classes.push('invalid');
        row.className = classes.join(' ');
        row.onclick = () => {
            _customPalettePopupState.selectedIdx = idx;
            _syncCustomPaletteEditor();
            _renderCustomPaletteRows();
            const displayName = String(entry.name || '').trim();
            if (!displayName) {
                _customPalettePopupState.error = 'Give this palette a name before activating it.';
                _renderCustomPaletteStatus();
                return;
            }
            if (!stops) {
                _customPalettePopupState.error = 'Fix the HEX values before activating this row.';
                _renderCustomPaletteStatus();
                return;
            }
            _customPalettePopupState.error = '';
            setPaletteForMode(
                _customPalettePopupState.mode,
                palette,
                displayName,
            );
            if (_customPalettePopupState.mode === 'proximity'
                    || _customPalettePopupState.mode === 'solve_score') {
                setColorMode(_customPalettePopupState.mode);
            }
            _renderCustomPaletteStatus();
        };

        const nameCell = document.createElement('td');
        nameCell.className = 'custom-palette-name';
        nameCell.textContent = String(entry.name || '').trim() || 'Unnamed palette';
        const hexCell = document.createElement('td');
        hexCell.className = 'custom-palette-hex';
        hexCell.textContent = stops ? stops.join(' ') : String(entry.hexText || '');
        const stripCell = document.createElement('td');
        const strip = document.createElement('div');
        strip.className = 'tri-popup-strip';
        strip.style.background = stops ? _stopsToGradient(stops) : '#555';
        stripCell.appendChild(strip);
        row.append(nameCell, hexCell, stripCell);
        body.appendChild(row);
    });
}

function _renderCustomPaletteStatus() {
    const status = document.getElementById('custom-palette-popup-status');
    const save = document.getElementById('custom-palette-popup-save');
    if (status) {
        const dirty = _customPalettePopupDirty();
        status.textContent = _customPalettePopupState.error
            || _customPalettePopupState.status
            || (dirty ? 'Unsaved changes.' : `${_customPalettePopupState.working.length} saved palettes.`);
        status.className = `custom-palette-status${_customPalettePopupState.error ? ' error' : ''}`;
    }
    if (save) {
        save.disabled = _customPalettePopupState.loading
            || _customPalettePopupState.saving
            || !_customPalettePopupDirty();
        save.textContent = _customPalettePopupState.saving ? 'Saving...' : 'Save';
    }
}

function _syncCustomPaletteEditor() {
    const nameInput = document.getElementById('custom-palette-popup-name');
    const hexInput = document.getElementById('custom-palette-popup-hex');
    const deleteBtn = document.getElementById('custom-palette-popup-delete');
    const idx = _customPalettePopupState.selectedIdx;
    const entry = idx >= 0 ? _customPalettePopupState.working[idx] : null;
    if (nameInput) {
        nameInput.disabled = !entry;
        nameInput.value = entry ? String(entry.name || '') : '';
    }
    if (hexInput) {
        hexInput.disabled = !entry;
        hexInput.value = entry ? String(entry.hexText || '') : '';
    }
    if (deleteBtn) deleteBtn.disabled = !entry || _customPalettePopupState.saving;
}

function _renderCustomPalettePopup() {
    const overlay = document.getElementById('custom-palette-popup-overlay');
    const title = document.getElementById('custom-palette-popup-title');
    if (!overlay || !title) return;
    if (!_customPalettePopupState.open) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
        return;
    }
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    title.textContent = 'Custom palettes for ' + _popupModeLabel(_customPalettePopupState.mode);
    _renderCustomPaletteRows();
    _syncCustomPaletteEditor();
    _renderCustomPaletteStatus();
}

async function _openCustomPalettePopup(mode) {
    _closeBuiltinPalettePopup();
    _closeTriPalettePopup();
    _closeLongPalettePopup();
    _closeMicPalettePopup();
    _customPalettePopupState.open = true;
    _customPalettePopupState.mode = mode;
    _customPalettePopupState.loading = !_customPaletteCatalogLoaded;
    _customPalettePopupState.saving = false;
    _customPalettePopupState.working = [];
    _customPalettePopupState.selectedIdx = -1;
    _customPalettePopupState.status = '';
    _customPalettePopupState.error = '';
    _renderCustomPalettePopup();
    try {
        await _loadCustomPaletteCatalog();
        if (!_customPalettePopupState.open || _customPalettePopupState.mode !== mode) return;
        _customPalettePopupState.loading = false;
        _resetCustomPaletteWorking();
        _renderCustomPalettePopup();
    } catch (error) {
        if (!_customPalettePopupState.open || _customPalettePopupState.mode !== mode) return;
        _customPalettePopupState.loading = false;
        _customPalettePopupState.error = `Load failed: ${error.message || error}`;
        _renderCustomPalettePopup();
    }
}

function _closeCustomPalettePopup(force = false) {
    if (!_customPalettePopupState.open) return true;
    if (!force && _customPalettePopupDirty()
            && !confirm('Discard unsaved custom palette changes?')) {
        return false;
    }
    _customPalettePopupState.open = false;
    _customPalettePopupState.loading = false;
    _customPalettePopupState.saving = false;
    _renderCustomPalettePopup();
    return true;
}

function _newCustomPaletteRow() {
    if (_customPalettePopupState.working.length >= CUSTOM_PALETTE_MAX_ENTRIES) {
        _customPalettePopupState.error = `At most ${CUSTOM_PALETTE_MAX_ENTRIES} custom palettes are allowed.`;
        _renderCustomPaletteStatus();
        return;
    }
    const existing = new Set(
        _customPalettePopupState.working.map(entry => String(entry.name || '').toLocaleLowerCase())
    );
    let name = 'New palette';
    let suffix = 2;
    while (existing.has(name.toLocaleLowerCase())) {
        name = `New palette ${suffix}`;
        suffix += 1;
    }
    const remembered = _customPaletteForMode(_customPalettePopupState.mode);
    const stops = remembered ? _customStopsFromName(remembered.palette) : null;
    const rowStops = stops || ['#000000', '#ffffff'];
    _customPalettePopupState.working.push({
        name,
        stops: rowStops,
        hexText: rowStops.join(', '),
        palette: _customPaletteNameFromStops(rowStops),
    });
    _customPalettePopupState.selectedIdx = _customPalettePopupState.working.length - 1;
    _customPalettePopupState.error = '';
    _customPalettePopupState.status = 'New row added. Edit it, then Save.';
    _renderCustomPalettePopup();
    const input = document.getElementById('custom-palette-popup-name');
    if (input) {
        input.focus();
        input.select();
    }
}

function _deleteSelectedCustomPalette() {
    const idx = _customPalettePopupState.selectedIdx;
    if (idx < 0 || idx >= _customPalettePopupState.working.length) return;
    _customPalettePopupState.working.splice(idx, 1);
    _customPalettePopupState.selectedIdx = Math.min(
        idx,
        _customPalettePopupState.working.length - 1,
    );
    _customPalettePopupState.error = '';
    _customPalettePopupState.status = 'Row removed. Save to persist the deletion.';
    _renderCustomPalettePopup();
}

function _customPaletteNameChanged(value) {
    const entry = _customPalettePopupState.working[_customPalettePopupState.selectedIdx];
    if (!entry) return;
    entry.name = String(value || '');
    _customPalettePopupState.error = '';
    _customPalettePopupState.status = '';
    _renderCustomPaletteRows();
    _renderCustomPaletteStatus();
}

function _customPaletteHexChanged(value) {
    const entry = _customPalettePopupState.working[_customPalettePopupState.selectedIdx];
    if (!entry) return;
    entry.hexText = String(value || '');
    const stops = _parseCustomPaletteStops(entry.hexText);
    entry.stops = stops || [];
    entry.palette = stops ? _customPaletteNameFromStops(stops) : '';
    _customPalettePopupState.error = '';
    _customPalettePopupState.status = '';
    _renderCustomPaletteRows();
    _renderCustomPaletteStatus();
}

function _reconcileCustomPaletteSelections() {
    ['proximity', 'solve_score', 'palette_tab', 'repalette', 'color_repalette'].forEach(mode => {
        const current = _currentPaletteForMode(mode);
        if (_customStopsFromName(current)) {
            const entry = _customPaletteEntryBySpec(current);
            // Deleting a catalog row must not erase the name already attached
            // to the active in-memory selection. It remains an ad-hoc custom
            // palette until another row is selected.
            if (entry) _rememberCustomPaletteForMode(mode, current, entry.name);
        }
        _renderPaletteRow(mode);
    });
}

async function _saveCustomPaletteCatalog() {
    if (_customPalettePopupState.saving) return;
    const validation = _customPaletteValidation(_customPalettePopupState.working);
    if (validation.error) {
        _customPalettePopupState.error = validation.error;
        _renderCustomPaletteStatus();
        return;
    }
    _customPalettePopupState.saving = true;
    _customPalettePopupState.error = '';
    _customPalettePopupState.status = '';
    _renderCustomPaletteStatus();
    try {
        const response = await lambdaPost('storage', {
            palettes: validation.payload,
            expected_revision: _customPaletteCatalogRevision,
        }, '/save-custom-palettes');
        const rows = Array.isArray(response && response.palettes) ? response.palettes : [];
        const entries = rows.map(_customPaletteEntry);
        if (entries.some(entry => !entry)) {
            throw new Error('Save returned an invalid custom palette catalog');
        }
        const selected = _customPalettePopupState.working[_customPalettePopupState.selectedIdx];
        _customPaletteCatalog = entries;
        _customPaletteCatalogRevision = String((response && response.revision) || '');
        _customPaletteCatalogLoaded = true;
        _customPalettePopupState.working = entries.map(_customPaletteClone);
        _customPalettePopupState.savedSignature = _customPaletteSignature(_customPalettePopupState.working);
        _customPalettePopupState.selectedIdx = selected
            ? _customPalettePopupState.working.findIndex(entry => (
                entry.name === String(selected.name || '').trim()
                && entry.palette === selected.palette
            ))
            : -1;
        _customPalettePopupState.status = `Saved ${entries.length} custom palettes.`;
        _reconcileCustomPaletteSelections();
    } catch (error) {
        const message = String((error && error.message) || error || '');
        if (/\b409\b|custom_palette_revision|custom palette catalog changed/i.test(message)) {
            // Preserve this draft, but force the next open to fetch the new
            // revision instead of presenting the stale catalog again.
            _customPaletteCatalogLoaded = false;
        }
        _customPalettePopupState.error = `Save failed: ${message}`;
    } finally {
        _customPalettePopupState.saving = false;
        _renderCustomPalettePopup();
    }
}

function _closeBuiltinPalettePopup() {
    _builtinPopupState.open = false;
    const overlay = document.getElementById('builtin-popup-overlay');
    const filter = document.getElementById('builtin-popup-filter');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
    if (filter) filter.value = '';
}

function _closeLongPalettePopup() {
    _longPopupState.open = false;
    const overlay = document.getElementById('long-popup-overlay');
    const filter = document.getElementById('long-popup-filter');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
    if (filter) filter.value = '';
}

function _closeTriPalettePopup() {
    _triPopupState.open = false;
    const overlay = document.getElementById('tri-popup-overlay');
    const filter = document.getElementById('tri-popup-filter');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
    if (filter) filter.value = '';
}

function _renderTriPalettePopup() {
    const overlay = document.getElementById('tri-popup-overlay');
    const titleEl = document.getElementById('tri-popup-title');
    const bodyEl = document.getElementById('tri-popup-body');
    const filterEl = document.getElementById('tri-popup-filter');
    if (!overlay || !titleEl || !bodyEl || !filterEl) return;
    if (!_triPopupState.open || !_triCatalogAvailable()) {
        _closeTriPalettePopup();
        return;
    }

    const visible = _visibleTriPaletteCatalog();
    const highlightIdx = visible.length ? Math.max(0, Math.min(_triPopupState.highlightIdx || 0, visible.length - 1)) : 0;
    _triPopupState.highlightIdx = highlightIdx;

    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    titleEl.textContent = 'Select TRI palette for ' + _popupModeLabel(_triPopupState.mode);
    filterEl.value = _triPopupState.filter || '';
    bodyEl.replaceChildren();

    if (!visible.length) {
        const row = document.createElement('tr');
        row.className = 'tri-popup-empty';
        const cell = document.createElement('td');
        cell.colSpan = 2;
        cell.textContent = 'No tri palettes match this filter.';
        row.appendChild(cell);
        bodyEl.appendChild(row);
        return;
    }

    visible.forEach((entry, idx) => {
        const row = document.createElement('tr');
        const activeName = _triPaletteForMode(_triPopupState.mode);
        const cls = ['tri-popup-row'];
        if (entry.name === activeName) cls.push('active');
        if (idx === highlightIdx) cls.push('highlight');
        row.className = cls.join(' ');
        row.onclick = () => {
            _setTriPaletteForMode(_triPopupState.mode, entry.name, true);
            _closeTriPalettePopup();
        };

        const nameCell = document.createElement('td');
        const nameWrap = document.createElement('div');
        nameWrap.className = 'tri-popup-name';
        const nameMain = document.createElement('div');
        nameMain.textContent = entry.name;
        nameWrap.appendChild(nameMain);
        if (entry.aliases && entry.aliases.length) {
            const aliases = document.createElement('div');
            aliases.className = 'tri-popup-aliases';
            aliases.textContent = 'aliases: ' + entry.aliases.join(', ');
            nameWrap.appendChild(aliases);
        }
        nameCell.appendChild(nameWrap);

        const stripCell = document.createElement('td');
        const strip = document.createElement('div');
        strip.className = 'tri-popup-strip';
        strip.style.background = entry.gradient_css;
        strip.title = entry.name;
        stripCell.appendChild(strip);

        row.appendChild(nameCell);
        row.appendChild(stripCell);
        bodyEl.appendChild(row);
    });
}

function _openTriPalettePopup(mode) {
    if (!_triCatalogAvailable()) return;
    if (!_closeCustomPalettePopup()) return;
    _closeBuiltinPalettePopup();
    _closeLongPalettePopup();
    _closeMicPalettePopup();
    _syncTriDefaults();
    const visible = _triCatalog();
    const activeName = _triPaletteForMode(mode);
    const activeIdx = Math.max(0, visible.findIndex(entry => entry.name === activeName));
    _triPopupState = { open: true, mode, filter: '', highlightIdx: activeIdx >= 0 ? activeIdx : 0 };
    _renderTriPalettePopup();
    const filter = document.getElementById('tri-popup-filter');
    if (filter && typeof filter.focus === 'function') filter.focus();
}

function _applyTriPopupFilter(text) {
    _triPopupState.filter = String(text || '');
    _triPopupState.highlightIdx = 0;
    _renderTriPalettePopup();
}

function _renderLongPalettePopup() {
    const overlay = document.getElementById('long-popup-overlay');
    const titleEl = document.getElementById('long-popup-title');
    const bodyEl = document.getElementById('long-popup-body');
    const filterEl = document.getElementById('long-popup-filter');
    if (!overlay || !titleEl || !bodyEl || !filterEl) return;
    if (!_longPopupState.open || !_longCatalogAvailable()) {
        _closeLongPalettePopup();
        return;
    }

    const visible = _visibleLongPaletteCatalog();
    const highlightIdx = visible.length ? Math.max(0, Math.min(_longPopupState.highlightIdx || 0, visible.length - 1)) : 0;
    _longPopupState.highlightIdx = highlightIdx;

    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    titleEl.textContent = 'Select LONG palette for ' + _popupModeLabel(_longPopupState.mode);
    filterEl.value = _longPopupState.filter || '';
    bodyEl.replaceChildren();

    if (!visible.length) {
        const row = document.createElement('tr');
        row.className = 'tri-popup-empty';
        const cell = document.createElement('td');
        cell.colSpan = 2;
        cell.textContent = 'No long palettes match this filter.';
        row.appendChild(cell);
        bodyEl.appendChild(row);
        return;
    }

    visible.forEach((entry, idx) => {
        const row = document.createElement('tr');
        const activeName = _longPaletteForMode(_longPopupState.mode);
        const cls = ['tri-popup-row'];
        if (entry.name === activeName) cls.push('active');
        if (idx === highlightIdx) cls.push('highlight');
        row.className = cls.join(' ');
        row.onclick = () => {
            _setLongPaletteForMode(_longPopupState.mode, entry.name, true);
            _closeLongPalettePopup();
        };

        const nameCell = document.createElement('td');
        const nameWrap = document.createElement('div');
        nameWrap.className = 'tri-popup-name';
        const nameMain = document.createElement('div');
        nameMain.textContent = entry.name;
        nameWrap.appendChild(nameMain);
        if (entry.aliases && entry.aliases.length) {
            const aliases = document.createElement('div');
            aliases.className = 'tri-popup-aliases';
            aliases.textContent = 'aliases: ' + entry.aliases.join(', ');
            nameWrap.appendChild(aliases);
        }
        nameCell.appendChild(nameWrap);

        const stripCell = document.createElement('td');
        const strip = document.createElement('div');
        strip.className = 'tri-popup-strip';
        strip.style.background = entry.gradient_css;
        strip.title = entry.name;
        stripCell.appendChild(strip);

        row.appendChild(nameCell);
        row.appendChild(stripCell);
        bodyEl.appendChild(row);
    });
}

function _openLongPalettePopup(mode) {
    if (!_longCatalogAvailable()) return;
    if (!_closeCustomPalettePopup()) return;
    _closeBuiltinPalettePopup();
    _closeTriPalettePopup();
    _closeMicPalettePopup();
    _syncLongDefaults();
    const visible = _longCatalog();
    const activeName = _longPaletteForMode(mode);
    const activeIdx = Math.max(0, visible.findIndex(entry => entry.name === activeName));
    _longPopupState = { open: true, mode, filter: '', highlightIdx: activeIdx >= 0 ? activeIdx : 0 };
    _renderLongPalettePopup();
    const filter = document.getElementById('long-popup-filter');
    if (filter && typeof filter.focus === 'function') filter.focus();
}

function _applyLongPopupFilter(text) {
    _longPopupState.filter = String(text || '');
    _longPopupState.highlightIdx = 0;
    _renderLongPalettePopup();
}

function _renderBuiltinPalettePopup() {
    const overlay = document.getElementById('builtin-popup-overlay');
    const titleEl = document.getElementById('builtin-popup-title');
    const bodyEl = document.getElementById('builtin-popup-body');
    const filterEl = document.getElementById('builtin-popup-filter');
    if (!overlay || !titleEl || !bodyEl || !filterEl) return;
    if (!_builtinPopupState.open) {
        _closeBuiltinPalettePopup();
        return;
    }

    const visible = _visibleBuiltinPaletteCatalog();
    const highlightIdx = visible.length ? Math.max(0, Math.min(_builtinPopupState.highlightIdx || 0, visible.length - 1)) : 0;
    _builtinPopupState.highlightIdx = highlightIdx;

    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    titleEl.textContent = 'Select palette for ' + _popupModeLabel(_builtinPopupState.mode);
    filterEl.value = _builtinPopupState.filter || '';
    bodyEl.replaceChildren();

    if (!visible.length) {
        const row = document.createElement('tr');
        row.className = 'tri-popup-empty';
        const cell = document.createElement('td');
        cell.colSpan = 2;
        cell.textContent = 'No built-in palettes match this filter.';
        row.appendChild(cell);
        bodyEl.appendChild(row);
        return;
    }

    visible.forEach((entry, idx) => {
        const row = document.createElement('tr');
        const activeName = _builtinPaletteForMode(_builtinPopupState.mode);
        const cls = ['tri-popup-row'];
        if (entry.name === activeName) cls.push('active');
        if (idx === highlightIdx) cls.push('highlight');
        row.className = cls.join(' ');
        row.onclick = () => {
            _setBuiltinPaletteForMode(_builtinPopupState.mode, entry.name, true);
            _closeBuiltinPalettePopup();
        };

        const nameCell = document.createElement('td');
        const nameWrap = document.createElement('div');
        nameWrap.className = 'tri-popup-name';
        const nameMain = document.createElement('div');
        nameMain.textContent = entry.name;
        nameWrap.appendChild(nameMain);
        nameCell.appendChild(nameWrap);

        const stripCell = document.createElement('td');
        const strip = document.createElement('div');
        strip.className = 'tri-popup-strip';
        strip.style.background = entry.gradient_css;
        strip.title = entry.name;
        stripCell.appendChild(strip);

        row.appendChild(nameCell);
        row.appendChild(stripCell);
        bodyEl.appendChild(row);
    });
}

function _openBuiltinPalettePopup(mode) {
    if (!_closeCustomPalettePopup()) return;
    _closeTriPalettePopup();
    _closeLongPalettePopup();
    _closeMicPalettePopup();
    _syncBuiltinDefaults();
    const visible = BUILTIN_PALETTE_ENTRIES;
    const activeName = _builtinPaletteForMode(mode);
    const activeIdx = Math.max(0, visible.findIndex(entry => entry.name === activeName));
    _builtinPopupState = { open: true, mode, filter: '', highlightIdx: activeIdx >= 0 ? activeIdx : 0 };
    _renderBuiltinPalettePopup();
    const filter = document.getElementById('builtin-popup-filter');
    if (filter && typeof filter.focus === 'function') filter.focus();
}

function _applyBuiltinPopupFilter(text) {
    _builtinPopupState.filter = String(text || '');
    _builtinPopupState.highlightIdx = 0;
    _renderBuiltinPalettePopup();
}

function _isTextInputFocused(multilineOnly) {
    const ae = document.activeElement;
    if (!ae) return false;
    if (ae.isContentEditable) return true;
    const tag = ae.tagName;
    if (tag === 'TEXTAREA') return true;
    return !multilineOnly && tag === 'INPUT';
}

/* ---- MIC popup: Meditations in Color artwork palettes ----
   ~20k museum-sourced palettes served as a static catalog JSON
   (data/mic_palette_catalog.json — same-origin both locally and on S3).
   A chosen entry rides the existing custom: wire + palette_display_name,
   so the render pipeline is untouched. scripts/fetch_mic_palettes.py
   rebuilds the catalog; its --check validates offline in the gate. */

const MIC_CATALOG_URL = 'data/mic_palette_catalog.json';
const MIC_POPUP_PAGE_SIZE = 250;
// Kandinsky — Points (1920): the MIC swatch's resting face
const MIC_SWATCH_DEFAULT_STOPS = ['#bc9f41', '#292b24', '#9a8a44', '#454833', '#552c2f', '#5f5337', '#776a3d', '#155c52'];

function _micCatalogReady() {
    return Array.isArray(_micPaletteCatalog) && _micPaletteCatalog.length > 0;
}

function _loadMicPaletteCatalog() {
    if (_micCatalogReady()) return Promise.resolve(_micPaletteCatalog);
    if (_micPaletteCatalogLoadPromise) return _micPaletteCatalogLoadPromise;
    _micPaletteCatalogError = '';
    _micPaletteCatalogLoadPromise = fetch(MIC_CATALOG_URL)
        .then(resp => {
            if (!resp.ok) throw new Error(`catalog fetch failed (HTTP ${resp.status})`);
            return resp.json();
        })
        .then(doc => {
            const rows = Array.isArray(doc && doc.palettes) ? doc.palettes : [];
            if (!rows.length) throw new Error('catalog is empty');
            _micPaletteCredit = String(doc.credit || '');
            _micPaletteCatalog = rows.map(row => {
                // v2 rows pack stops as one hex string, 6 chars per stop
                const stops = (String(row.c || '').toLowerCase().match(/[0-9a-f]{6}/g) || []).map(h => '#' + h);
                return {
                    name: String(row.n || ''),
                    stops,
                    gradient: _stopsToGradient(stops.length === 1 ? [stops[0], stops[0]] : stops),
                    search: String(row.n || '').toLowerCase(),
                };
            });
            return _micPaletteCatalog;
        })
        .catch(e => {
            _micPaletteCatalogError = e && e.message ? e.message : String(e);
            _micPaletteCatalogLoadPromise = null;
            throw e;
        });
    return _micPaletteCatalogLoadPromise;
}

function _micPaletteWire(stops) {
    const bare = (Array.isArray(stops) ? stops : [])
        .map(s => String(s || '').replace(/^#/, '').toLowerCase());
    if (!bare.length || bare.length > CUSTOM_PALETTE_MAX_STOPS) return '';
    if (!bare.every(s => /^[0-9a-f]{6}$/.test(s))) return '';
    // custom: wire requires >= 2 stops; a one-color palette is a flat ramp
    const wire = bare.length === 1 ? [bare[0], bare[0]] : bare;
    return 'custom:' + wire.join('-');
}

function _visibleMicPaletteCatalog() {
    if (!_micCatalogReady()) return [];
    const filter = String(_micPopupState.filter || '').trim().toLowerCase();
    if (!filter) return _micPaletteCatalog;
    const terms = filter.split(/\s+/);
    return _micPaletteCatalog.filter(entry => terms.every(t => entry.search.includes(t)));
}

function _micPopupShownEntries() {
    const visible = _visibleMicPaletteCatalog();
    const start = (_micPopupState.page || 0) * MIC_POPUP_PAGE_SIZE;
    return visible.slice(start, start + MIC_POPUP_PAGE_SIZE);
}

function _micPopupGoToPage(page) {
    const visible = _visibleMicPaletteCatalog();
    const totalPages = Math.max(1, Math.ceil(visible.length / MIC_POPUP_PAGE_SIZE));
    _micPopupState.page = Math.max(0, Math.min(page, totalPages - 1));
    _micPopupState.highlightIdx = 0;
    _renderMicPalettePopup();
}

function _micApplyEntry(mode, entry) {
    const wire = _micPaletteWire(entry && entry.stops);
    if (!wire) return;
    _micPaletteSelectionByMode[mode] = { palette: wire, displayName: entry.name };
    setPaletteForMode(mode, wire, entry.name);
    if (mode === 'proximity' || mode === 'solve_score') setColorMode(mode);
    _closeMicPalettePopup();
}

function _micEntryWire(entry) {
    return _micPaletteWire(entry && entry.stops);
}

function _micLocateSelection(mode, filter) {
    /* Index of the mode's remembered MIC pick under `filter`; -1 if absent. */
    const sel = _micPaletteSelectionByMode[mode];
    if (!sel || !sel.palette || !_micCatalogReady()) return -1;
    const terms = String(filter || '').trim().toLowerCase().split(/\s+/).filter(Boolean);
    let idx = -1;
    let seen = 0;
    for (const entry of _micPaletteCatalog) {
        if (terms.length && !terms.every(t => entry.search.includes(t))) continue;
        if (_micEntryWire(entry) === sel.palette) { idx = seen; break; }
        seen += 1;
    }
    return idx;
}

function _openMicPalettePopup(mode) {
    if (!_closeCustomPalettePopup()) return;
    _closeBuiltinPalettePopup();
    _closeTriPalettePopup();
    _closeLongPalettePopup();
    let filterText = String(_micPopupState.filter || '');
    let page = _micPopupState.page || 0;
    let highlightIdx = 0;
    // Reopen lands on the current selection: keep the remembered filter when
    // it still shows the pick, otherwise clear it so the pick is reachable.
    let idx = _micLocateSelection(mode, filterText);
    if (idx < 0 && filterText && _micLocateSelection(mode, '') >= 0) {
        filterText = '';
        idx = _micLocateSelection(mode, '');
    }
    if (idx >= 0) {
        page = Math.floor(idx / MIC_POPUP_PAGE_SIZE);
        highlightIdx = idx % MIC_POPUP_PAGE_SIZE;
    }
    _micPopupState = { open: true, mode, filter: filterText, page, highlightIdx };
    _renderMicPalettePopup();
    if (!_micCatalogReady()) {
        _loadMicPaletteCatalog()
            .then(() => { if (_micPopupState.open) _renderMicPalettePopup(); })
            .catch(() => { if (_micPopupState.open) _renderMicPalettePopup(); });
    }
    const filter = document.getElementById('mic-popup-filter');
    if (filter && typeof filter.focus === 'function') filter.focus();
}

function _applyMicPopupFilter(text) {
    _micPopupState.filter = String(text || '');
    _micPopupState.page = 0;
    _micPopupState.highlightIdx = 0;
    _renderMicPalettePopup();
}

function _closeMicPalettePopup() {
    _micPopupState.open = false;
    const overlay = document.getElementById('mic-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _renderMicPalettePopup() {
    const overlay = document.getElementById('mic-popup-overlay');
    const titleEl = document.getElementById('mic-popup-title');
    const statusEl = document.getElementById('mic-popup-status');
    const bodyEl = document.getElementById('mic-popup-body');
    const filterEl = document.getElementById('mic-popup-filter');
    if (!overlay || !titleEl || !statusEl || !bodyEl || !filterEl) return;
    if (!_micPopupState.open) {
        _closeMicPalettePopup();
        return;
    }
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    titleEl.textContent = 'Select artwork palette for ' + _popupModeLabel(_micPopupState.mode);
    const selNameEl = document.getElementById('mic-popup-selected-name');
    const selStripEl = document.getElementById('mic-popup-selected-strip');
    const sel = _micPaletteSelectionByMode[_micPopupState.mode];
    const selStops = sel && sel.palette ? _customStopsFromName(sel.palette) : null;
    if (selNameEl) selNameEl.textContent = sel && sel.displayName ? `Selected: ${sel.displayName}` : 'No artwork palette selected';
    if (selStripEl) {
        selStripEl.style.display = selStops ? '' : 'none';
        if (selStops) selStripEl.style.background = _stopsToGradient(selStops);
    }
    if (filterEl.value !== (_micPopupState.filter || '')) filterEl.value = _micPopupState.filter || '';
    bodyEl.replaceChildren();

    const prevBtn = document.getElementById('mic-popup-prev');
    const nextBtn = document.getElementById('mic-popup-next');
    const pageEl = document.getElementById('mic-popup-page-display');
    const gotoEl = document.getElementById('mic-popup-goto');
    if (!_micCatalogReady()) {
        if (pageEl) pageEl.textContent = '– / –';
        if (prevBtn) prevBtn.disabled = true;
        if (nextBtn) nextBtn.disabled = true;
        statusEl.textContent = _micPaletteCatalogError
            ? `Failed to load palette catalog: ${_micPaletteCatalogError}`
            : 'Loading artwork palettes…';
        const row = document.createElement('tr');
        row.className = 'tri-popup-empty';
        const cell = document.createElement('td');
        cell.colSpan = 2;
        cell.textContent = _micPaletteCatalogError ? 'Catalog unavailable.' : 'Loading…';
        row.appendChild(cell);
        bodyEl.appendChild(row);
        return;
    }

    const visible = _visibleMicPaletteCatalog();
    const totalPages = Math.max(1, Math.ceil(visible.length / MIC_POPUP_PAGE_SIZE));
    const page = Math.max(0, Math.min(_micPopupState.page || 0, totalPages - 1));
    _micPopupState.page = page;
    const start = page * MIC_POPUP_PAGE_SIZE;
    const shown = visible.slice(start, start + MIC_POPUP_PAGE_SIZE);
    if (pageEl) pageEl.textContent = `${page + 1} / ${totalPages}`;
    if (prevBtn) prevBtn.disabled = page <= 0;
    if (nextBtn) nextBtn.disabled = page >= totalPages - 1;
    if (gotoEl) gotoEl.max = String(totalPages);
    statusEl.textContent = visible.length
        ? `${visible.length.toLocaleString()} palette${visible.length === 1 ? '' : 's'} — showing ${(start + 1).toLocaleString()}–${(start + shown.length).toLocaleString()}`
        : 'No artwork palettes match this filter.';
    const highlightIdx = shown.length
        ? Math.max(0, Math.min(_micPopupState.highlightIdx || 0, shown.length - 1)) : 0;
    _micPopupState.highlightIdx = highlightIdx;

    if (!shown.length) {
        const row = document.createElement('tr');
        row.className = 'tri-popup-empty';
        const cell = document.createElement('td');
        cell.colSpan = 2;
        cell.textContent = 'No artwork palettes match this filter.';
        row.appendChild(cell);
        bodyEl.appendChild(row);
        return;
    }

    const current = _currentPaletteForMode(_micPopupState.mode);
    shown.forEach((entry, idx) => {
        const row = document.createElement('tr');
        const cls = ['tri-popup-row'];
        if (_micEntryWire(entry) === current) cls.push('active');
        if (idx === highlightIdx) cls.push('highlight');
        row.className = cls.join(' ');
        row.onclick = () => _micApplyEntry(_micPopupState.mode, entry);

        const nameCell = document.createElement('td');
        const nameWrap = document.createElement('div');
        nameWrap.className = 'tri-popup-name';
        const nameMain = document.createElement('div');
        nameMain.textContent = entry.name;
        nameWrap.appendChild(nameMain);
        nameCell.appendChild(nameWrap);

        const stripCell = document.createElement('td');
        const strip = document.createElement('div');
        strip.className = 'tri-popup-strip';
        strip.style.background = entry.gradient;
        strip.title = entry.name;
        stripCell.appendChild(strip);

        row.appendChild(nameCell);
        row.appendChild(stripCell);
        bodyEl.appendChild(row);
    });
    const highlighted = bodyEl.children[highlightIdx];
    if (highlighted && typeof highlighted.scrollIntoView === 'function') {
        highlighted.scrollIntoView({ block: 'nearest' });
    }
}

function _micPopupMoveHighlight(delta) {
    /* Arrow scrolling that walks ACROSS pages at the edges. */
    const shown = _micPopupShownEntries();
    if (!shown.length) return;
    const idx = Math.max(0, Math.min(_micPopupState.highlightIdx || 0, shown.length - 1));
    const next = idx + delta;
    if (next >= 0 && next < shown.length) {
        _micPopupState.highlightIdx = next;
        _renderMicPalettePopup();
        return;
    }
    const visible = _visibleMicPaletteCatalog();
    const totalPages = Math.max(1, Math.ceil(visible.length / MIC_POPUP_PAGE_SIZE));
    const page = _micPopupState.page || 0;
    if (next < 0 && page > 0) {
        _micPopupState.page = page - 1;
        _micPopupState.highlightIdx = MIC_POPUP_PAGE_SIZE - 1;
        _renderMicPalettePopup();
    } else if (next >= shown.length && page < totalPages - 1) {
        _micPopupState.page = page + 1;
        _micPopupState.highlightIdx = 0;
        _renderMicPalettePopup();
    }
}

function _initMicPalettePopup() {
    const filterEl = document.getElementById('mic-popup-filter');
    _bindPopupShell({
        overlayId: 'mic-popup-overlay',
        closeId: 'mic-popup-close',
        isOpen: () => !!_micPopupState.open,
        onClose: _closeMicPalettePopup,
        onArrowDown: () => _micPopupMoveHighlight(1),
        onArrowUp: () => _micPopupMoveHighlight(-1),
        onEnter: () => {
            const shown = _micPopupShownEntries();
            if (!shown.length) return;
            const entry = shown[Math.max(0, Math.min(_micPopupState.highlightIdx || 0, shown.length - 1))];
            if (entry) _micApplyEntry(_micPopupState.mode, entry);
        },
    });
    if (filterEl) filterEl.addEventListener('input', (ev) => _applyMicPopupFilter(ev.target.value));
    if (filterEl) filterEl.addEventListener('keydown', (ev) => {
        if (ev.key === 'ArrowDown' || ev.key === 'ArrowUp') {
            ev.preventDefault();
            _micPopupMoveHighlight(ev.key === 'ArrowDown' ? 1 : -1);
        }
    });
    const prevBtn = document.getElementById('mic-popup-prev');
    const nextBtn = document.getElementById('mic-popup-next');
    const gotoEl = document.getElementById('mic-popup-goto');
    if (prevBtn) prevBtn.addEventListener('click', () => _micPopupGoToPage((_micPopupState.page || 0) - 1));
    if (nextBtn) nextBtn.addEventListener('click', () => _micPopupGoToPage((_micPopupState.page || 0) + 1));
    if (gotoEl) {
        const jump = () => {
            const v = parseInt(gotoEl.value, 10);
            if (Number.isFinite(v)) _micPopupGoToPage(v - 1);
        };
        gotoEl.addEventListener('change', jump);
        gotoEl.addEventListener('keydown', (ev) => {
            if (ev.key === 'Enter') {
                // jump only — stop the popup shell's Enter-to-pick handler
                ev.preventDefault();
                ev.stopPropagation();
                jump();
            }
        });
    }
}

function _bindPopupShell({ overlayId, closeId, cancelId, isOpen, onClose, onEnter, onArrowDown, onArrowUp }) {
    const overlay = document.getElementById(overlayId);
    const closeBtn = closeId ? document.getElementById(closeId) : null;
    const cancelBtn = cancelId ? document.getElementById(cancelId) : null;
    if (overlay) {
        overlay.addEventListener('click', (ev) => {
            if (ev.target === overlay) onClose();
        });
    }
    if (closeBtn) closeBtn.addEventListener('click', onClose);
    if (cancelBtn) cancelBtn.addEventListener('click', onClose);
    document.addEventListener('keydown', (ev) => {
        if (!isOpen()) return;
        if (ev.key === 'Escape') {
            ev.preventDefault();
            onClose();
            return;
        }
        if (ev.key === 'ArrowDown' && onArrowDown && !_isTextInputFocused()) {
            ev.preventDefault();
            onArrowDown(ev);
            return;
        }
        if (ev.key === 'ArrowUp' && onArrowUp && !_isTextInputFocused()) {
            ev.preventDefault();
            onArrowUp(ev);
            return;
        }
        if (ev.key === 'Enter' && onEnter && !_isTextInputFocused(true)) {
            ev.preventDefault();
            onEnter(ev);
        }
    });
}

function _initTriPalettePopup() {
    const filterEl = document.getElementById('tri-popup-filter');
    _bindPopupShell({
        overlayId: 'tri-popup-overlay',
        closeId: 'tri-popup-close',
        isOpen: () => !!_triPopupState.open,
        onClose: _closeTriPalettePopup,
        onArrowDown: () => {
            const visible = _visibleTriPaletteCatalog();
            if (!visible.length) return;
            _triPopupState.highlightIdx = Math.min((_triPopupState.highlightIdx || 0) + 1, visible.length - 1);
            _renderTriPalettePopup();
        },
        onArrowUp: () => {
            const visible = _visibleTriPaletteCatalog();
            if (!visible.length) return;
            _triPopupState.highlightIdx = Math.max((_triPopupState.highlightIdx || 0) - 1, 0);
            _renderTriPalettePopup();
        },
        onEnter: () => {
            const visible = _visibleTriPaletteCatalog();
            if (!visible.length) return;
            const entry = visible[Math.max(0, Math.min(_triPopupState.highlightIdx || 0, visible.length - 1))];
            if (!entry) return;
            _setTriPaletteForMode(_triPopupState.mode, entry.name, true);
            _closeTriPalettePopup();
        },
    });
    if (filterEl) filterEl.addEventListener('input', (ev) => _applyTriPopupFilter(ev.target.value));
}

function _initBuiltinPalettePopup() {
    const filterEl = document.getElementById('builtin-popup-filter');
    _bindPopupShell({
        overlayId: 'builtin-popup-overlay',
        closeId: 'builtin-popup-close',
        isOpen: () => !!_builtinPopupState.open,
        onClose: _closeBuiltinPalettePopup,
        onArrowDown: () => {
            const visible = _visibleBuiltinPaletteCatalog();
            if (!visible.length) return;
            _builtinPopupState.highlightIdx = Math.min((_builtinPopupState.highlightIdx || 0) + 1, visible.length - 1);
            _renderBuiltinPalettePopup();
        },
        onArrowUp: () => {
            const visible = _visibleBuiltinPaletteCatalog();
            if (!visible.length) return;
            _builtinPopupState.highlightIdx = Math.max((_builtinPopupState.highlightIdx || 0) - 1, 0);
            _renderBuiltinPalettePopup();
        },
        onEnter: () => {
            const visible = _visibleBuiltinPaletteCatalog();
            if (!visible.length) return;
            const entry = visible[Math.max(0, Math.min(_builtinPopupState.highlightIdx || 0, visible.length - 1))];
            if (!entry) return;
            _setBuiltinPaletteForMode(_builtinPopupState.mode, entry.name, true);
            _closeBuiltinPalettePopup();
        },
    });
    if (filterEl) filterEl.addEventListener('input', (ev) => _applyBuiltinPopupFilter(ev.target.value));
}

function _initLongPalettePopup() {
    const filterEl = document.getElementById('long-popup-filter');
    _bindPopupShell({
        overlayId: 'long-popup-overlay',
        closeId: 'long-popup-close',
        isOpen: () => !!_longPopupState.open,
        onClose: _closeLongPalettePopup,
        onArrowDown: () => {
            const visible = _visibleLongPaletteCatalog();
            if (!visible.length) return;
            _longPopupState.highlightIdx = Math.min((_longPopupState.highlightIdx || 0) + 1, visible.length - 1);
            _renderLongPalettePopup();
        },
        onArrowUp: () => {
            const visible = _visibleLongPaletteCatalog();
            if (!visible.length) return;
            _longPopupState.highlightIdx = Math.max((_longPopupState.highlightIdx || 0) - 1, 0);
            _renderLongPalettePopup();
        },
        onEnter: () => {
            const visible = _visibleLongPaletteCatalog();
            if (!visible.length) return;
            const entry = visible[Math.max(0, Math.min(_longPopupState.highlightIdx || 0, visible.length - 1))];
            if (!entry) return;
            _setLongPaletteForMode(_longPopupState.mode, entry.name, true);
            _closeLongPalettePopup();
        },
    });
    if (filterEl) filterEl.addEventListener('input', (ev) => _applyLongPopupFilter(ev.target.value));
}

function _initCustomPalettePopup() {
    _bindPopupShell({
        overlayId: 'custom-palette-popup-overlay',
        closeId: 'custom-palette-popup-close',
        isOpen: () => !!_customPalettePopupState.open,
        onClose: () => _closeCustomPalettePopup(),
    });
    const nameInput = document.getElementById('custom-palette-popup-name');
    const hexInput = document.getElementById('custom-palette-popup-hex');
    const newBtn = document.getElementById('custom-palette-popup-new');
    const deleteBtn = document.getElementById('custom-palette-popup-delete');
    const saveBtn = document.getElementById('custom-palette-popup-save');
    if (nameInput) {
        nameInput.addEventListener('input', (event) => {
            _customPaletteNameChanged(event.target.value);
        });
    }
    if (hexInput) {
        hexInput.addEventListener('input', (event) => {
            _customPaletteHexChanged(event.target.value);
        });
    }
    if (newBtn) newBtn.addEventListener('click', _newCustomPaletteRow);
    if (deleteBtn) deleteBtn.addEventListener('click', _deleteSelectedCustomPalette);
    if (saveBtn) saveBtn.addEventListener('click', _saveCustomPaletteCatalog);
}

function _canRepaletteArtifact(art) {
    if (!art || !art.palette_id) return false;
    if (art.raw_key || Number(art.raw_channels || art.score_output_channel_count || 1) > 1) return false;
    if (art.render_reusable === false && !(art.palette_bins_key || art.chunk_bins_prefix || art.section_bins_prefix)) return false;
    return !!(art.palette_bins_key || art.chunk_bins_prefix || art.section_bins_prefix);
}

function _hasColorRawSidecar(art) {
    return !!(art && art.raw_key && art.raw_meta_key);
}

function _canColorRepaletteArtifact(art) {
    if (!art || art.family !== 'color') return false;
    if (art.postprocess_kind) return false;
    if (art.color_mode !== 'solve_score' && art.color_mode !== 'saved_palette') return false;
    if (![1, 3].includes(_artifactOutputChannelCount(art))) return false;
    return !!_hasColorRawSidecar(art);
}

;(window.__ppParts = window.__ppParts || []).push('04-palette-color');
