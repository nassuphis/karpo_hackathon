#!/bin/bash
# Frontend JS execution test.
# Loads index.html inline JS + catalog JS in a Node VM context,
# then verifies key functions exist and run without ReferenceErrors.
# Catches the class of bug where a refactor deletes a variable that
# another function still references (e.g. BATCH_SIZE).
#
# Run: bash tests/test_frontend_js.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
HTML="$ROOT/index.html"
CATALOG_JS="$ROOT/coeff_func_catalog_js.js"
TRI_CATALOG_JS="$ROOT/tri_palette_catalog_js.js"
LONG_CATALOG_JS="$ROOT/long_palette_catalog_js.js"

if [ ! -f "$HTML" ]; then echo "FATAL: $HTML not found"; exit 1; fi
if [ ! -f "$CATALOG_JS" ]; then echo "FATAL: $CATALOG_JS not found"; exit 1; fi
if [ ! -f "$TRI_CATALOG_JS" ]; then echo "FATAL: $TRI_CATALOG_JS not found"; exit 1; fi
if [ ! -f "$LONG_CATALOG_JS" ]; then echo "FATAL: $LONG_CATALOG_JS not found"; exit 1; fi
grep -q "switchTab('favorites')" "$HTML" || { echo "FATAL: Favorites tab button missing from index.html"; exit 1; }
grep -q 'id="tab-favorites"' "$HTML" || { echo "FATAL: Favorites tab content missing from index.html"; exit 1; }
grep -q 'id="btn-favorites-go-render"' "$HTML" || { echo "FATAL: Favorites GoRender button missing from index.html"; exit 1; }
grep -q 'id="btn-favorites-download"' "$HTML" || { echo "FATAL: Favorites Download button missing from index.html"; exit 1; }
grep -q 'id="results-filter-mode"' "$HTML" || { echo "FATAL: Results filter mode dropdown missing from index.html"; exit 1; }
grep -q 'id="build-id-label"' "$HTML" || { echo "FATAL: build id label missing from index.html"; exit 1; }
grep -q 'id="btn-config-toggle"' "$HTML" || { echo "FATAL: Config gear toggle missing from index.html"; exit 1; }
grep -q 'id="config-popup"' "$HTML" || { echo "FATAL: Config popup missing from index.html"; exit 1; }
grep -q '<option value="sectioned">sectioned native</option>' "$HTML" || { echo "FATAL: sectioned hist input option missing from index.html"; exit 1; }
grep -q 'id="render-mt-raster-input-mode"' "$HTML" || { echo "FATAL: render MT raster input selector missing from index.html"; exit 1; }
grep -q 'id="render-mt-merge-workers"' "$HTML" || { echo "FATAL: render MT merge workers input missing from index.html"; exit 1; }
grep -q 'onclick="loadLambdaConfig()" class="btn-secondary" style="margin:0; padding:4px 12px"' "$HTML" || { echo "FATAL: Config Load button should override global button margin"; exit 1; }
grep -q 'onclick="renderResultsTable()" style="margin:0; padding:3px 8px; font-size:10px"' "$HTML" || { echo "FATAL: Results Filter button should override global button margin"; exit 1; }
grep -q 'id="btn-solve-histogram" onclick="runSolveScoreHistogramDebug()" style="margin:0 0 0 8px; font-size:10px; padding:1px 8px"' "$HTML" || { echo "FATAL: Solve histogram button should override global button margin"; exit 1; }
grep -q 'id="btn-palette-create" onclick="runPaletteArtifact()" style="margin:0 0 0 8px; font-size:10px; padding:1px 8px"' "$HTML" || { echo "FATAL: Palette create button should override global button margin"; exit 1; }
grep -q 'Image + Meta' "$HTML" || { echo "FATAL: Favorites download menu missing Image + Meta"; exit 1; }
grep -q 'Select Dir…' "$HTML" || { echo "FATAL: Favorites download menu missing Select Dir…"; exit 1; }

echo "=== Frontend JS Execution Test ==="

# Extract inline JS from index.html
sed -n '/<script>$/,/<\/script>/p' "$HTML" | sed '1d;$d' > /tmp/_fe_test_app.js

cat > /tmp/_fe_test_harness.cjs << 'HARNESS_EOF'
const vm = require('vm');
const fs = require('fs');

// Minimal DOM stubs
const _elements = {};
const _mkEl = () => {
    const el = {
    value: '', textContent: '', style: {}, id: '', dataset: {},
    scrollTop: 0,
    appendChild(child) { el.children.push(child); },
    removeChild() {}, setAttribute() {}, insertBefore() {},
    prepend(child) { if (child && child.textContent) el.textContent = child.textContent + '\n' + (el.textContent || ''); },
    append() {},
    replaceChildren() { el.children = []; },
    selectedOptions: [{ textContent: '' }],
    options: [], children: [],
    get innerHTML() { return el._html || ''; },
    set innerHTML(v) { el._html = v; el.children = []; el._innerHTMLSet = true; },
    classList: { add(){}, remove(){}, toggle(){}, contains(){ return false; } },
    addEventListener() {}, removeEventListener() {},
    getBoundingClientRect() { return {top:0,left:0,width:100,height:100}; },
    getContext() { return { fillRect(){}, clearRect(){}, drawImage(){}, fillText(){}, set fillStyle(v){}, set font(v){}, set textAlign(v){}, set textBaseline(v){}, imageSmoothingEnabled: false }; },
    querySelectorAll() { return []; },
    querySelector() { return null; },
    focus() {}, blur() {},
    scrollIntoView(arg) { el._scrollIntoViewCalls = (el._scrollIntoViewCalls || 0) + 1; el._lastScrollIntoViewArg = arg; },
    checked: false,
    width: 512, height: 512,
    onchange: null,
    onclick: null,
    oninput: null,
    oncontextmenu: null,
    onscroll: null,
    };
    return el;
};

const docStub = {
    getElementById(id) {
        if (!_elements[id]) _elements[id] = _mkEl();
        return _elements[id];
    },
    createElement: _mkEl,
    createTextNode(t) { return { textContent: t }; },
    querySelectorAll() { return []; },
    querySelector() { return null; },
    body: _mkEl(),
    head: _mkEl(),
    addEventListener() {},
    visibilityState: 'visible',
    createEvent() { return { initEvent() {} }; },
};

// Build the VM context with browser-like globals
const ctx = {
    console,
    document: docStub,
    localStorage: (() => {
        const store = {};
        return {
            getItem(k) { return store[k] || null; },
            setItem(k, v) { store[k] = v; },
            removeItem(k) { delete store[k]; },
        };
    })(),
    performance: { now() { return Date.now(); } },
    AudioContext: class { constructor() {} },
    OfflineAudioContext: class { constructor() {} },
    Worker: class { constructor() {} postMessage() {} terminate() {} addEventListener() {} },
    Image: class {
        constructor() { this.onload = null; this.onerror = null; this.width = 512; this.height = 512; this.naturalWidth = 512; this.naturalHeight = 512; }
        set src(v) { this._src = v; if (this.onload) this.onload(); }
    },
    fetch: async () => ({ ok: true, json: async () => ({}) }),
    alert() {},
    confirm() { return true; },
    setTimeout: () => 0,
    clearTimeout: () => {},
    setInterval: () => 0,
    clearInterval: () => {},
    requestAnimationFrame: () => 0,
    cancelAnimationFrame: () => {},
    URL: { createObjectURL() { return ''; }, revokeObjectURL() {} },
    Blob: class {},
    FileReader: class { readAsText() {} readAsDataURL() {} },
    HTMLCanvasElement: class {},
    CanvasRenderingContext2D: class {},
    getComputedStyle: () => new Proxy({}, { get() { return ''; } }),
    matchMedia: () => ({ matches: false, addEventListener() {} }),
    OpenSeadragon: function() { return {
        addHandler(){}, destroy(){},
        world: { getItemAt(){ return null; }, getItemCount(){ return 0; } },
        viewport: { getZoom(){ return 1; }, getCenter(){ return {x:0,y:0}; } },
    }; },
    ResizeObserver: class { observe(){} disconnect(){} },
    navigator: { userAgent: '' },
    _elements,
    _mkEl,
};
ctx.addEventListener = function() {};
ctx.removeEventListener = function() {};
ctx.window = ctx;
ctx.globalThis = ctx;
ctx.self = ctx;
vm.createContext(ctx);

// Step 1: Load catalog JS
const catalogCode = fs.readFileSync(process.argv[2], 'utf8');
vm.runInContext(catalogCode, ctx, { filename: 'coeff_func_catalog_js.js' });
const catLen = (ctx._coeffFuncCatalog || []).length;
console.log('  catalog loaded: ' + catLen + ' functions');

// Step 1b: Load tri-palette catalog JS
const triCatalogCode = fs.readFileSync(process.argv[3], 'utf8');
vm.runInContext(triCatalogCode, ctx, { filename: 'tri_palette_catalog_js.js' });
const triCatLen = (ctx._triPaletteCatalog || []).length;
console.log('  tri catalog loaded: ' + triCatLen + ' palettes');

// Step 1c: Load long-palette catalog JS
const longCatalogCode = fs.readFileSync(process.argv[4], 'utf8');
vm.runInContext(longCatalogCode, ctx, { filename: 'long_palette_catalog_js.js' });
const longCatLen = (ctx._longPaletteCatalog || []).length;
console.log('  long catalog loaded: ' + longCatLen + ' palettes');

// Step 2: Load app JS (strip auto-init populateDropdown call)
let appCode = fs.readFileSync(process.argv[5], 'utf8');
appCode = appCode.replace(/^populateDropdown\(\);$/m, '// populateDropdown() — deferred to test');
appCode = appCode.replace(/^loadLambdaConfig\(\);$/m, '// loadLambdaConfig() — deferred to test');
try {
    vm.runInContext(appCode, ctx, { filename: 'index-inline.js' });
} catch (e) {
    console.error('FATAL: app load: ' + e.constructor.name + ': ' + e.message);
    console.error(e.stack.split('\n').slice(0, 5).join('\n'));
    process.exit(1);
}

// Step 3: Export and check key functions
const exportCode = `
globalThis.__exports = {
    populateDropdown: typeof populateDropdown === 'function' ? populateDropdown : null,
    updateCfpvRow: typeof updateCfpvRow === 'function' ? updateCfpvRow : null,
    parseCfpv: typeof parseCfpv === 'function' ? parseCfpv : null,
    addChip: typeof addChip === 'function' ? addChip : null,
    removeChip: typeof removeChip === 'function' ? removeChip : null,
    log: typeof log === 'function' ? log : null,
    _getCatalogEntry: typeof _getCatalogEntry === 'function' ? _getCatalogEntry : null,
    openFunctionPopup: typeof openFunctionPopup === 'function' ? openFunctionPopup : null,
    chooseFunctionPopupSelection: typeof chooseFunctionPopupSelection === 'function' ? chooseFunctionPopupSelection : null,
};
`;
vm.runInContext(exportCode, ctx);
const fns = ctx.__exports;
const missing = Object.entries(fns).filter(([, v]) => !v).map(([k]) => k);
if (missing.length) {
    console.error('FATAL: missing functions: ' + missing.join(', '));
    process.exit(1);
}
console.log('  required functions: all present');

// Step 4: Run populateDropdown and verify actual option count
try {
    vm.runInContext('populateDropdown()', ctx);
} catch (e) {
    console.error('FATAL: populateDropdown() crashed: ' + e.constructor.name + ': ' + e.message);
    console.error(e.stack.split('\n').slice(0, 5).join('\n'));
    process.exit(1);
}
const sel = ctx._elements['render-function'];
const optCount = sel ? sel.children.length : 0;
if (optCount < 100) {
    console.error('FATAL: populateDropdown() produced ' + optCount + ' options (expected 1000+)');
    process.exit(1);
}
console.log('  populateDropdown(): ' + optCount + ' options added');

// Step 4b: Negative test — empty catalog should produce error option
const savedCat = ctx._coeffFuncCatalog;
ctx._coeffFuncCatalog = [];
vm.runInContext('populateDropdown()', ctx);
const emptyOpts = ctx._elements['render-function'].children.length;
ctx._coeffFuncCatalog = savedCat;
if (emptyOpts !== 1) {
    console.error('FATAL: empty catalog should produce 1 error option, got ' + emptyOpts);
    process.exit(1);
}
console.log('  populateDropdown(empty): shows error option');
// Restore by re-populating
vm.runInContext('populateDropdown()', ctx);

// Step 4c: Function picker popup should filter and choose from catalog
try {
    vm.runInContext("document.getElementById('render-function').value = 'giga_1'; openFunctionPopup();", ctx);
    const popupRows0 = (ctx._elements['function-popup-body']?.children || []).length;
    if (popupRows0 < 100) {
        console.error('FATAL: function popup should render many rows, got ' + popupRows0);
        process.exit(1);
    }
    vm.runInContext("_applyFunctionFilter('poly_795')", ctx);
    const popupRows1 = (ctx._elements['function-popup-body']?.children || []).length;
    if (popupRows1 !== 1) {
        console.error('FATAL: function popup filter poly_795 should narrow to 1 row, got ' + popupRows1);
        process.exit(1);
    }
    vm.runInContext('chooseFunctionPopupSelection()', ctx);
    if (ctx._elements['render-function'].value !== 'poly_795') {
        console.error('FATAL: function popup choose should set render-function to poly_795, got ' + ctx._elements['render-function'].value);
        process.exit(1);
    }
    const pickerText = ctx._elements['render-function-picker']?.textContent || '';
    if (pickerText !== 'poly_795') {
        console.error('FATAL: function picker button should show poly_795, got ' + pickerText);
        process.exit(1);
    }
    console.log('  function popup filter/choose: OK');
} catch (e) {
    console.error('FATAL: function popup flow crashed: ' + e.message);
    process.exit(1);
}

// Step 5: Test updateCfpvRow for parametric function
ctx._elements['render-function'] = { ...ctx._mkEl(), value: 'creative9' };
try {
    vm.runInContext('updateCfpvRow()', ctx);
    console.log('  updateCfpvRow(creative9): OK');
} catch (e) {
    console.error('FATAL: updateCfpvRow(creative9): ' + e.message);
    process.exit(1);
}

// Step 6: Test updateCfpvRow for non-parametric function
ctx._elements['render-function'] = { ...ctx._mkEl(), value: 'g1' };
try {
    vm.runInContext('updateCfpvRow()', ctx);
    console.log('  updateCfpvRow(g1): OK');
} catch (e) {
    console.error('FATAL: updateCfpvRow(g1): ' + e.message);
    process.exit(1);
}

// Step 7: Render orchestrator launch smoke test
// Verifies that runRasterPipeline/runRasterPipelineMT/runBilevelPipeline/runCoeffBilevelPipeline
// each dispatch one render_orchestrator job and don't dispatch worker phases directly.
console.log('');
console.log('--- Render pipeline orchestration ---');

// Seed DOM state for Render tab
const renderEls = {
    'render-results-dir': { value: 'test_job' },
    'render-pix': { value: '1024' },
    'render-format': { value: 'jpeg' },
    'render-quality': { value: '90' },
    'render-square-extent': { value: '2' },
    'sparse-tile-size': { value: '512' },
    'render-rotation': { value: '0' },
    'render-rotation-dir': { value: 'ccw' },
    'render-color-mode': { value: 'rainbow' },
    'render-match-mode': { value: 'none' },
    'render-palette': { value: 'inferno' },
    'render-constant-color': { value: 'ffffff' },
    'render-quantile': { value: '0' },
    'render-shim': { value: '5.0' },
    'render-solve-score-quantile': { value: '0.1' },
    'render-solve-score-quantile-val': {},
    'render-solve-score-omega': { value: '1' },
    'render-solve-score-omega-val': {},
    'btn-solve-histogram': {},
    'btn-populate-result': {},
    'btn-render-generate': {},
    'btn-render-generate-mt': {},
    'btn-render-autolevels': {},
    'btn-render-download': {},
    'btn-render-delete': {},
    'btn-render-deepzoom': {},
    'render-status': {},
    'render-preview': {},
    'render-info': {},
    'render-log': {},
    'render-generate-popup-overlay': {},
    'render-generate-popup-summary': {},
    'render-generate-popup-close': {},
    'render-generate-popup-cancel': {},
    'render-generate-popup-run': {},
    'render-generate-hist-input-mode': { value: 'tmpfile' },
    'render-mt-popup-overlay': {},
    'render-mt-popup-summary': {},
    'render-mt-popup-close': {},
    'render-mt-popup-cancel': {},
    'render-mt-popup-run': {},
    'render-mt-threads': { value: '4' },
    'render-mt-solve-score-threads': { value: '4' },
    'render-mt-hist-input-mode': { value: 'tmpfile' },
    'render-mt-raster-input-mode': { value: 'tmpfile' },
    'render-mt-merge-workers': { value: '16' },
    'autolevel-popup-overlay': {},
    'autolevel-popup-title': {},
    'autolevel-popup-summary': {},
    'autolevel-popup-close': {},
    'autolevel-popup-revert': {},
    'autolevel-popup-cancel': {},
    'autolevel-popup-run': {},
    'autolevel-bins': { value: '256' },
    'autolevel-enable-levels': { checked: true },
    'autolevel-clip-low': { value: '0' },
    'autolevel-clip-high': { value: '1' },
    'autolevel-enable-peak-limit': { checked: false },
    'autolevel-peak-factor': { value: '0' },
    'autolevel-enable-gamma': { checked: false },
    'autolevel-gamma': { value: '1' },
    'autolevel-enable-auto-gamma': { checked: false },
    'autolevel-auto-gamma': { value: 'median' },
    'autolevel-target': { value: '0.5' },
    'autolevel-enable-sigmoid': { checked: false },
    'autolevel-sigmoid-strength': { value: '0' },
    'autolevel-sigmoid-mid': { value: '0.5' },
    'autolevel-enable-vibrance': { checked: false },
    'autolevel-vibrance': { value: '0' },
    'autolevel-enable-pooled-rgb': { checked: true },
    'autolevel-pooled-rgb': { value: '0.1' },
    'autolevel-quality': { value: '90' },
    'autolevel-jpeg-subsample': { value: 'on' },
    'autolevel-background-readout': { value: '#000000' },
    'autolevel-background-threshold': { value: '4' },
    'autolevel-exclude-background': { checked: true },
    'autolevel-jpeg-optimize': { checked: false },
    'autolevel-jpeg-interlace': { checked: false },
    'palette-results-dir': { value: '' },
    'palette-solve-score': { value: 'proximity' },
    'palette-solve-score-quantile': { value: '0.1' },
    'palette-solve-score-quantile-val': {},
    'palette-solve-score-omega': { value: '1' },
    'palette-solve-score-omega-val': {},
    'palette-status': {},
    'palette-info': {},
    'palette-log': {},
    'palette-inventory': {},
    'palette-canvas': {},
    'palette-rt-chips': {},
    'palette-circles-palette-tab': {},
    'btn-palette-create': {},
    'btn-palette-delete': {},
};
for (const [id, overrides] of Object.entries(renderEls)) {
    ctx._elements[id] = { ...ctx._mkEl(), ...overrides };
}
vm.runInContext('_renderAllPaletteRows()', ctx);

// Seed required global state
vm.runInContext(`
    _lastCalcMeta = { job_id: 'test_job', n_stripes: 4, degree: 7, n_coeffs: 8 };
    _rtChain = [];
    renderColorMode = 'rainbow';
    renderMatchMode = 'none';
    renderPalette = 'inferno';
`, ctx);

// Stub network + async helpers — MUST be inside VM to override function declarations
vm.runInContext(`
    var _tilePolls = 0, _encodePolls = 0;
    var _pipelineDispatchLogs = [];
    var _renderOrchestratorPayloads = [];
    var _pipelineOrigLog = log;

    _bilevelDispatchAndPoll = async () => 1234;
    refreshRenderArtifacts = async () => {};
    log = function(msg, cls, target) {
        if (target === 'render-log' && String(msg).includes('dispatching')) {
            _pipelineDispatchLogs.push({ msg, cls, target });
        }
        return _pipelineOrigLog(msg, cls, target);
    };

    lambdaPost = async function lambdaPost(name, body, path) {
        if (name === 'storage' && path === '/clean-render') return { deleted: 0 };
        if (name === 'viewport') return { q_re: [-2, 2], q_im: [-2, 2], scale: 256, pix: 1024, n_roots: 1000 };
        if (name === 'dispatch' && body.target === 'render_orchestrator') {
            _renderOrchestratorPayloads.push(body.jobs[0]);
            return { fired: 1, errors: [] };
        }
        if (name === 'dispatch' && body.target === 'autolevels') return { fired: 1, errors: [] };
        if (name === 'dispatch' && body.target === 'finalize') return { fired: body.jobs.length, errors: [] };
        if (name === 'dispatch' && body.target === 'bilevel_stitch') return { fired: 1, errors: [] };
        if (name === 'dispatch' && body.target === 'bilevel') return { fired: body.jobs.length, errors: [] };
        if (name === 'dispatch' && body.target === 'deepzoom_export') return { fired: 1, errors: [] };
        if (name === 'storage' && path === '/check-status' && body.task_prefix === 'tile_') {
            _tilePolls++;
            if (_tilePolls === 1) return { errors: 0, done: 0, complete: false, status_counts: {} };
            return { errors: 0, done: body.expected, complete: true, status_counts: { done: body.expected } };
        }
        if (name === 'dispatch' && body.target === 'encode') return { fired: 1, errors: [] };
        if (name === 'storage' && path === '/check-status' && body.task_prefix === 'encode') {
            _encodePolls++;
            return { errors: 0, done: 1, complete: true, status_counts: { done: 1 } };
        }
        if (name === 'storage' && path === '/check-status' && body.task_prefix === 'bilevel_stitch') {
            return { errors: 0, done: 1, complete: true, status_counts: { done: 1 } };
        }
        if (name === 'storage' && path === '/check-status' && body.task_prefix === 'coeff_bilevel_stitch') {
            return { errors: 0, done: 1, complete: true, status_counts: { done: 1 } };
        }
        if (name === 'storage' && path === '/check-status') {
            return { errors: 0, done: body.expected || 1, complete: true, status_counts: { done: body.expected || 1 },
                results: [{ phase: 'done', phase_label: 'Done' }] };
        }
        if (name === 'storage' && path === '/save-metadata') return { ok: true };
        if (name === 'storage') return { ok: true };
        return { ok: true };
    };

    // Fast fake time for stall checks
    var _fakeNow = 0;
    performance = { now: function() { _fakeNow += 35000; return _fakeNow; } };
`, ctx);

// setTimeout/setInterval must resolve promises and run callbacks for the observer
ctx.setTimeout = (fn) => { if (typeof fn === 'function') fn(); return 0; };
ctx.setInterval = (fn, ms) => { if (typeof fn === 'function') fn(); return 42; };
ctx.clearInterval = () => {};

async function testPipeline(name, call) {
    vm.runInContext('_tilePolls = 0; _encodePolls = 0; _fakeNow = 0; _pipelineDispatchLogs = []; _renderOrchestratorPayloads = [];', ctx);
    // Clear status element to detect pipeline-caught errors
    ctx._elements['render-status'].textContent = '';
    try {
        await vm.runInContext(call, ctx);
    } catch (e) {
        if (e instanceof ReferenceError) {
            console.error('FATAL: ' + name + ': ' + e.message);
            process.exit(1);
        }
    }
    // The pipeline has its own try/catch that swallows errors into render-status
    const statusText = ctx._elements['render-status'].textContent || '';
    if (statusText.includes('error:') || statusText.includes('Error:')) {
        console.error('FATAL: ' + name + ' failed: ' + statusText);
        process.exit(1);
    }
    console.log('  ' + name + ': OK');
}

(async () => {
    {
        ctx._elements['config-url'] = ctx._elements['config-url'] || ctx._mkEl();
        ctx._elements['config-url'].value = 'https://cfg/build.json';
        vm.runInContext(`
            fetch = async function(url) {
                if (url !== 'https://cfg/build.json') throw new Error('unexpected config url ' + url);
                return {
                    ok: true,
                    json: async function() {
                        return {
                            sweep: 'https://api.example/sweep',
                            storage: 'https://api.example',
                            build: {
                                build_id: '20260407-120000-abc1234-deadbeefcafe',
                                deployed_at_utc: '2026-04-07T12:00:00Z',
                                git_rev: 'abc1234',
                                git_dirty: false,
                                frontend_sha256: 'deadbeefcafebabefeedface'
                            }
                        };
                    }
                };
            };
        `, ctx);
        try {
            await vm.runInContext('(async()=>{ await loadLambdaConfig(); })()', ctx);
            const label = ctx._elements['build-id-label'].textContent || '';
            const tooltip = ctx._elements['build-id-label'].title || '';
            const status = ctx._elements['config-status'].textContent || '';
            const title = ctx.document.title || '';
            const endpointKeys = vm.runInContext('Object.keys(_lambdaUrls).sort().join(",")', ctx);
            if (!label.includes('build 20260407-120000-abc1234-deadbeefcafe')) {
                console.error('FATAL: build id label not applied, got ' + label);
                process.exit(1);
            }
            if (!title.includes('build 20260407-120000-abc1234-deadbeefcafe')) {
                console.error('FATAL: document title should include build id, got ' + title);
                process.exit(1);
            }
            if (!tooltip.includes('frontend sha256: deadbeefcafebabefeedface')) {
                console.error('FATAL: build tooltip missing frontend hash, got ' + tooltip);
                process.exit(1);
            }
            if (status !== '2 endpoints loaded') {
                console.error('FATAL: config status should count only endpoint services, got ' + status);
                process.exit(1);
            }
            if (endpointKeys !== 'storage,sweep') {
                console.error('FATAL: _lambdaUrls should only contain endpoint services, got ' + endpointKeys);
                process.exit(1);
            }
            console.log('  config load applies build id to title: OK');
        } catch (e) {
            console.error('FATAL: build id config load: ' + e.message);
            process.exit(1);
        }

        try {
            vm.runInContext('_toggleConfigPopup(true)', ctx);
            const popupDisplay = ctx._elements['config-popup'].style.display || '';
            vm.runInContext('_toggleConfigPopup(false)', ctx);
            const popupDisplayAfterClose = ctx._elements['config-popup'].style.display || '';
            if (popupDisplay !== 'block' || popupDisplayAfterClose !== 'none') {
                console.error('FATAL: config popup should toggle open/closed, got open=' + popupDisplay + ' close=' + popupDisplayAfterClose);
                process.exit(1);
            }
            console.log('  config popup toggles: OK');
        } catch (e) {
            console.error('FATAL: config popup toggle: ' + e.message);
            process.exit(1);
        }
    }

    await testPipeline('runRasterPipeline', '(async()=>{ await runRasterPipeline(); })()');
    {
        const logs = vm.runInContext('_pipelineDispatchLogs', ctx);
        const hit = Array.isArray(logs) ? logs.find((row) => String(row.msg || '').includes('Render: dispatching color orchestrator')) : null;
        if (!hit || hit.cls !== 'ok') {
            console.error('FATAL: runRasterPipeline should log green render dispatch, got ' + JSON.stringify(logs));
            process.exit(1);
        }
    }
    vm.runInContext(`
        renderColorMode = 'solve_score';
        _activeRenderRun = null;
        generateSelectedRenderArtifact();
    `, ctx);
    if (ctx._elements['render-generate-popup-overlay'].style.display !== 'flex') {
        console.error('FATAL: solve-score Generate should open popup overlay');
        process.exit(1);
    }
    if (!(ctx._elements['render-generate-popup-summary'].textContent || '').includes('histogram input=')) {
        console.error('FATAL: solve-score Generate popup should show histogram input summary');
        process.exit(1);
    }
    console.log('  Generate popup opens for solve-score input A/B: OK');
    vm.runInContext('_closeRenderGeneratePopup()', ctx);
    await testPipeline('runRasterPipelineMT', '(async()=>{ renderColorMode = "solve_score"; await runRasterPipelineMT({ rasterThreads: 6, solveScoreThreads: 3, mergeWorkers: 12 }); })()');
    {
        const logs = vm.runInContext('_pipelineDispatchLogs', ctx);
        const payloads = vm.runInContext('_renderOrchestratorPayloads', ctx);
        const mtHit = Array.isArray(logs) ? logs.find((row) => String(row.msg || '').includes('Render-MT: dispatching with solve score threads=3, hist input=tmpfile, merge workers=12, raster input=tmpfile, raster threads=6')) : null;
        const orchHit = Array.isArray(logs) ? logs.find((row) => String(row.msg || '').includes('Render: dispatching color orchestrator')) : null;
        const mtPayload = Array.isArray(payloads) ? payloads.find((row) => row && row.params && row.params.raster_engine === 'mt') : null;
        if (!mtHit || mtHit.cls !== 'ok' || !orchHit || orchHit.cls !== 'ok' || !mtPayload ||
            mtPayload.params.raster_mt_threads !== 6 || mtPayload.params.solve_score_threads !== 3 ||
            mtPayload.params.solve_score_hist_input_mode !== 'tmpfile' ||
            mtPayload.params.solve_score_merge_workers !== 12 ||
            mtPayload.params.raster_input_mode !== 'tmpfile') {
            console.error('FATAL: runRasterPipelineMT should log green MT + orchestrator dispatch and pass thread counts, got logs=' + JSON.stringify(logs) + ' payloads=' + JSON.stringify(payloads));
            process.exit(1);
        }
    }
    await testPipeline('runBilevelPipeline', '(async()=>{ await runBilevelPipeline(); })()');
    await testPipeline('runCoeffBilevelPipeline', '(async()=>{ await runCoeffBilevelPipeline(); })()');

    vm.runInContext(`
        renderColorMode = 'solve_score';
        renderMatchMode = 'none';
        _activeRenderRun = null;
        openRenderMtPopup();
    `, ctx);
    if (ctx._elements['render-mt-popup-overlay'].style.display !== 'flex') {
        console.error('FATAL: Generate-MT should open popup overlay');
        process.exit(1);
    }
    if (!(ctx._elements['render-mt-popup-summary'].textContent || '').includes('merge workers=')) {
        console.error('FATAL: Generate-MT popup should show thread summary');
        process.exit(1);
    }
    if (ctx._elements['render-mt-solve-score-threads'].disabled !== false) {
        console.error('FATAL: Generate-MT popup should enable solve score thread input for solve_score mode');
        process.exit(1);
    }
    if (ctx._elements['render-mt-hist-input-mode'].disabled !== false) {
        console.error('FATAL: Generate-MT popup should enable hist input selector for solve_score mode');
        process.exit(1);
    }
    if (ctx._elements['render-mt-merge-workers'].disabled !== false) {
        console.error('FATAL: Generate-MT popup should enable merge workers input for solve_score mode');
        process.exit(1);
    }
    console.log('  Generate-MT popup opens with solve-score + raster thread summary: OK');
    vm.runInContext('_closeRenderMtPopup()', ctx);

    // Step 8: Direct _bilevelDispatchAndPoll tests
    console.log('');
    console.log('--- _bilevelDispatchAndPoll direct tests ---');

    // Restore the REAL _bilevelDispatchAndPoll (we stubbed it for pipeline tests)
    // Re-eval the app code to get it back, but first save our stubs
    vm.runInContext(`
        // Re-load from source to restore _bilevelDispatchAndPoll
        // (it was overwritten by stub above)
    `, ctx);
    // Actually, we need to re-eval just the function. Extract it from appCode.
    const bdpMatch = appCode.match(/async function _bilevelDispatchAndPoll\(opts\) \{[\s\S]*?^}/m);
    if (!bdpMatch) {
        console.error('FATAL: could not extract _bilevelDispatchAndPoll from app code');
        process.exit(1);
    }
    vm.runInContext(bdpMatch[0], ctx);
    console.log('  _bilevelDispatchAndPoll restored from app code');

    // --- Test 8a: basic completion (300 jobs, all succeed) ---
    {
        let dispatched = 0;
        let pollCount = 0;
        let fakeT = 0;
        vm.runInContext(`
            performance = { now: function() { return _bdp_fakeT; } };
            var _bdp_fakeT = 0;
            var _bdp_dispatched = 0;
            var _bdp_pollCount = 0;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch') {
                    _bdp_dispatched += body.jobs.length;
                    return { fired: body.jobs.length, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    _bdp_pollCount++;
                    _bdp_fakeT += 1000;
                    // Complete after first poll
                    return { errors: 0, done: body.expected, complete: true, status_counts: { done: body.expected } };
                }
                return {};
            };
        `, ctx);

        const jobs = Array.from({length: 300}, (_, i) => ({ idx: i }));
        try {
            const ms = await vm.runInContext(`
                (async () => {
                    const jobs = ${JSON.stringify(jobs)};
                    return await _bilevelDispatchAndPoll({
                        jobs, jobId: 'test1', taskPrefix: 'raster_',
                        target: 'raster', label: 'Test', logTarget: 'render-log'
                    });
                })()
            `, ctx);
            const d = vm.runInContext('_bdp_dispatched', ctx);
            if (d !== 300) {
                console.error('FATAL: expected 300 dispatched, got ' + d);
                process.exit(1);
            }
            console.log('  8a basic completion (300 jobs): OK, dispatched=' + d);
        } catch (e) {
            console.error('FATAL: 8a basic completion: ' + e.message);
            process.exit(1);
        }
    }

    // --- Test 8b: wave dispatch throttling (500 jobs, slow completion) ---
    {
        vm.runInContext(`
            var _bdp_fakeT = 0;
            var _bdp_dispatched = 0;
            var _bdp_dispatchCalls = 0;
            var _bdp_maxBatch = 0;
            var _bdp_pollDoneProgress = 0;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch') {
                    _bdp_dispatchCalls++;
                    _bdp_dispatched += body.jobs.length;
                    if (body.jobs.length > _bdp_maxBatch) _bdp_maxBatch = body.jobs.length;
                    return { fired: body.jobs.length, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    _bdp_fakeT += 1000;
                    // Simulate slow progress: report 50 more done each poll
                    _bdp_pollDoneProgress = Math.min(_bdp_pollDoneProgress + 50, body.expected);
                    return {
                        errors: 0,
                        done: _bdp_pollDoneProgress,
                        complete: _bdp_pollDoneProgress >= body.expected,
                        status_counts: { done: _bdp_pollDoneProgress }
                    };
                }
                return {};
            };
            performance = { now: function() { _bdp_fakeT += 100; return _bdp_fakeT; } };
        `, ctx);

        try {
            await vm.runInContext(`
                (async () => {
                    const jobs = Array.from({length: 500}, (_, i) => ({ idx: i }));
                    await _bilevelDispatchAndPoll({
                        jobs, jobId: 'test2', taskPrefix: 'raster_',
                        target: 'raster', label: 'Wave', logTarget: 'render-log'
                    });
                })()
            `, ctx);
            const d = vm.runInContext('_bdp_dispatched', ctx);
            const maxB = vm.runInContext('_bdp_maxBatch', ctx);
            const calls = vm.runInContext('_bdp_dispatchCalls', ctx);
            if (d !== 500) {
                console.error('FATAL: expected 500 dispatched, got ' + d);
                process.exit(1);
            }
            // Max batch should be <= 50 (BATCH_SIZE)
            if (maxB > 50) {
                console.error('FATAL: max dispatch batch ' + maxB + ' exceeds BATCH_SIZE=50');
                process.exit(1);
            }
            // Should have multiple dispatch calls (wave pattern, not single blast)
            if (calls < 5) {
                console.error('FATAL: only ' + calls + ' dispatch calls for 500 jobs (expected wave pattern)');
                process.exit(1);
            }
            console.log('  8b wave dispatch (500 jobs): OK, batches=' + calls + ', maxBatch=' + maxB);
        } catch (e) {
            console.error('FATAL: 8b wave dispatch: ' + e.message);
            process.exit(1);
        }
    }

    // --- Test 8c: stall triggers re-dispatch of missing tasks ---
    {
        vm.runInContext(`
            var _bdp_fakeT = 0;
            var _bdp_dispatched = 0;
            var _bdp_redispatched = 0;
            var _bdp_pollCount = 0;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch') {
                    if (_bdp_dispatched >= 10) {
                        _bdp_redispatched += body.jobs.length;
                    }
                    _bdp_dispatched += body.jobs.length;
                    return { fired: body.jobs.length, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    _bdp_pollCount++;
                    // Simulate stall: done stays at 5, time advances
                    // Poll 1 in completion phase: done=5 (sets lastDone=5, resets lastProgressTime)
                    // Poll 2+: done still 5, no progress, stallTime accumulates
                    // After GRACE_MS (45s): helper requests return_ids
                    _bdp_fakeT += 50000; // advance well past GRACE_MS each poll
                    if (body.return_ids) {
                        // Helper asked for IDs — return only 0-4, missing 5-9
                        return {
                            errors: 0, done: 5, complete: false,
                            status_counts: { done: 5 },
                            found_ids: ['raster_0','raster_1','raster_2','raster_3','raster_4']
                        };
                    }
                    if (_bdp_redispatched > 0) {
                        // After re-dispatch: complete
                        return {
                            errors: 0, done: 10, complete: true,
                            status_counts: { done: 10 }
                        };
                    }
                    return {
                        errors: 0, done: 5, complete: false,
                        status_counts: { done: 5 }
                    };
                }
                return {};
            };
            performance = { now: function() { _bdp_fakeT += 100; return _bdp_fakeT; } };
        `, ctx);

        try {
            await vm.runInContext(`
                (async () => {
                    const jobs = Array.from({length: 10}, (_, i) => ({ idx: i }));
                    await _bilevelDispatchAndPoll({
                        jobs, jobId: 'test3', taskPrefix: 'raster_',
                        target: 'raster', label: 'Redispatch', logTarget: 'render-log'
                    });
                })()
            `, ctx);
            const rd = vm.runInContext('_bdp_redispatched', ctx);
            if (rd < 1) {
                console.error('FATAL: expected re-dispatch of missing tasks, got 0');
                process.exit(1);
            }
            console.log('  8c stall re-dispatch (10 jobs, 5 missing): OK, re-dispatched=' + rd);
        } catch (e) {
            console.error('FATAL: 8c stall re-dispatch: ' + e.message);
            process.exit(1);
        }
    }

    // Step 9: Render family catalog UI
    console.log('');
    console.log('--- Render family catalogs ---');

    function assertActionButtons(panelHtml, mustHave, mustNotHave, label) {
        for (const text of mustHave) {
            if (!panelHtml.includes(text)) {
                console.error(`FATAL: ${label} should include button/control "${text}"`);
                process.exit(1);
            }
        }
        for (const text of mustNotHave) {
            if (panelHtml.includes(text)) {
                console.error(`FATAL: ${label} should not include button/control "${text}"`);
                process.exit(1);
            }
        }
    }

    {
        const summary = {
            calc: { exists: true, N: 1000, degree: 5 },
            families: {
                color: [
                    { artifact_id: 'color_a', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/j/color/color_a/image.jpeg', image_url: 'https://img/color.jpeg', preview_url: 'https://img/color.png', viewer_url: 'https://img/color.png', width: 1000, height: 1000, file_size: 50000, color_mode: 'rainbow', format: 'jpeg' }
                ],
                bilevel: [
                    { artifact_id: 'bil_a', created_at: '2026-03-30T11:00:00Z', image_key: 'renders/j/bilevel/bil_a/image.tif', image_url: 'https://img/bil.tif', preview_url: 'https://img/bil.png', viewer_url: 'https://img/bil.png', width: 1000, height: 1000, file_size: 60000, format: 'tif' }
                ],
                coeffs: [],
                palette: [
                    { artifact_id: 'pal_a', palette_id: 'pal_a', created_at: '2026-03-30T12:00:00Z', image_key: 'renders/j/palettes/pal_a/image.jpeg', image_url: 'https://img/pal.jpeg', preview_url: 'https://img/pal.png', viewer_url: 'https://img/pal.png', width: 1000, height: 1000, file_size: 70000, metric: 'crowding', palette: 'reef', solve_score_quantile: 0.05 }
                ],
                pdf: [
                    { artifact_id: 'pdf_a', created_at: '2026-03-30T13:00:00Z', document_key: 'renders/j/pdf/pdf_a/document.pdf', viewer_url: 'https://img/pdf.pdf', width: 586, height: 296, file_size: 80000, format: 'pdf', content_type: 'application/pdf', pdf_kind: 'color_spread' }
                ],
            },
        };
        vm.runInContext(`_renderActiveFamily = 'color'; _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1, pdf: -1 };`, ctx);
        vm.runInContext(`renderArtifactPanel('j', ${JSON.stringify(summary)})`, ctx);

        const panelHtml = ctx._elements['render-preview'].innerHTML;
        if (!panelHtml.includes('Color <span style="color:#777">(1)</span>')) { console.error('FATAL: color family tab missing'); process.exit(1); }
        if (!panelHtml.includes('BiLevel <span style="color:#777">(1)</span>')) { console.error('FATAL: bilevel family tab missing'); process.exit(1); }
        if (!panelHtml.includes('Palette <span style="color:#777">(1)</span>')) { console.error('FATAL: palette family tab missing'); process.exit(1); }
        if (!panelHtml.includes('PDF <span style="color:#777">(1)</span>')) { console.error('FATAL: pdf family tab missing'); process.exit(1); }
        const colorSel = vm.runInContext('_renderSelectedArtifact.color', ctx);
        if (colorSel !== 0) { console.error('FATAL: color family should auto-select first artifact'); process.exit(1); }
        if (!panelHtml.includes('2026-03-30 10:00:00')) { console.error('FATAL: color artifact row created timestamp missing'); process.exit(1); }
        if (!panelHtml.includes('https://img/color.png')) { console.error('FATAL: selected color viewer should use viewer_url'); process.exit(1); }
        if (!panelHtml.includes('id="render-artifact-viewer"')) { console.error('FATAL: render artifact viewer container missing'); process.exit(1); }
        if (!panelHtml.includes('height:360px') || !panelHtml.includes('background:#000')) { console.error('FATAL: render artifact panel should keep fixed black viewport height'); process.exit(1); }
        assertActionButtons(
            panelHtml,
            ['Generate', 'Generate-MT', 'GenerateFromPalette', 'RePalette', 'Populate', 'Autolevels', 'GoResult', 'Favorite', 'Download', 'Delete', 'DeepZoom'],
            ['ColorSpread'],
            'color action row'
        );
        const colorActionRowCount = (panelHtml.match(/class="render-action-row"/g) || []).length;
        if (colorActionRowCount !== 3) { console.error('FATAL: color action buttons should be split into exactly 3 rows of max 5'); process.exit(1); }
        console.log('  color family auto-select + viewer: OK');

        vm.runInContext(`_renderSelectFamily('palette')`, ctx);
        const palHtml = ctx._elements['render-preview'].innerHTML;
        if (!palHtml.includes('https://img/pal.png')) { console.error('FATAL: palette family should show selected palette viewer'); process.exit(1); }
        assertActionButtons(
            palHtml,
            ['Generate', 'RePalette', 'Populate', 'Download', 'Delete', 'DeepZoom'],
            ['Generate-MT', 'GenerateFromPalette', 'Autolevels', 'ColorSpread'],
            'palette action row'
        );
        console.log('  family switch updates catalog: OK');

        vm.runInContext(`_renderSelectFamily('pdf')`, ctx);
        const pdfHtml = ctx._elements['render-preview'].innerHTML;
        if (!pdfHtml.includes('https://img/pdf.pdf#toolbar=0&navpanes=0&view=FitH')) { console.error('FATAL: pdf family should show embedded pdf viewer'); process.exit(1); }
        assertActionButtons(
            pdfHtml,
            ['ColorSpread', 'Download', 'Delete'],
            ['Generate-MT', 'GenerateFromPalette', 'Populate', 'Autolevels', 'DeepZoom'],
            'pdf action row'
        );
        if (!pdfHtml.includes("Save PDF")) { console.error('FATAL: pdf download menu should offer Save PDF'); process.exit(1); }
        if (!pdfHtml.includes("Select Dir")) { console.error('FATAL: pdf download menu should offer Select Dir'); process.exit(1); }
        console.log('  pdf family controls + viewer: OK');

        vm.runInContext(`
            renderArtifactPanel('j', {
                calc: { exists: true, N: 1000, degree: 5 },
                families: {
                    color: [
                        { artifact_id: 'color_0', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/j/color/color_0/image.jpeg', image_url: 'https://img/color0.jpeg', preview_url: 'https://img/color0.png', viewer_url: 'https://img/color0.png', width: 1000, height: 1000, file_size: 50000, color_mode: 'rainbow', format: 'jpeg' },
                        { artifact_id: 'color_1', created_at: '2026-03-30T10:01:00Z', image_key: 'renders/j/color/color_1/image.jpeg', image_url: 'https://img/color1.jpeg', preview_url: 'https://img/color1.png', viewer_url: 'https://img/color1.png', width: 1000, height: 1000, file_size: 51000, color_mode: 'rainbow', format: 'jpeg' },
                        { artifact_id: 'color_2', created_at: '2026-03-30T10:02:00Z', image_key: 'renders/j/color/color_2/image.jpeg', image_url: 'https://img/color2.jpeg', preview_url: 'https://img/color2.png', viewer_url: 'https://img/color2.png', width: 1000, height: 1000, file_size: 52000, color_mode: 'rainbow', format: 'jpeg' }
                    ],
                    bilevel: [],
                    coeffs: [],
                    palette: [],
                    pdf: [],
                },
            });
        `, ctx);
        ctx._elements['render-artifact-catalog'].scrollTop = 240;
        vm.runInContext(`_renderSelectArtifact('color', 2)`, ctx);
        const preservedTop = ctx._elements['render-artifact-catalog'].scrollTop;
        const selectedRow = ctx._elements['render-art-row-color-2'];
        if (preservedTop !== 240) { console.error('FATAL: selecting render artifact should preserve catalog scroll, got ' + preservedTop); process.exit(1); }
        if (!selectedRow || !selectedRow._scrollIntoViewCalls) { console.error('FATAL: selected render row should be scrolled into view when needed'); process.exit(1); }
        console.log('  artifact selection preserves scroll + ensures visible row: OK');

        vm.runInContext(`_renderSelectFamily('coeffs')`, ctx);
        const coeffHtml = ctx._elements['render-preview'].innerHTML;
        if (!coeffHtml.includes('No saved artifacts yet.')) { console.error('FATAL: empty family should show empty state'); process.exit(1); }
        const dlDisabled = !!ctx._elements['btn-render-download'].disabled;
        const delDisabled = !!ctx._elements['btn-render-delete'].disabled;
        const dzDisabled = !!ctx._elements['btn-render-deepzoom'].disabled;
        if (!(dlDisabled && delDisabled && dzDisabled)) { console.error('FATAL: empty family should disable actions'); process.exit(1); }
        console.log('  empty family disables actions: OK');
    }

    {
        ctx._elements['render-results-dir'] = { ...ctx._mkEl(), value: 'j' };
        vm.runInContext(`
            _favoriteRefs = [];
            _favoriteRefsLoaded = true;
            _renderLoadedJobId = 'j';
            _renderActiveFamily = 'color';
            renderArtifactPanel('j', {
                calc: { exists: true, N: 1000, degree: 5 },
                families: {
                    color: [
                        { artifact_id: 'color_fav', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/j/color/color_fav/image.jpeg', image_url: 'https://img/color_fav.jpeg', preview_url: 'https://img/color_fav.png', viewer_url: 'https://img/color_fav.png', width: 1000, height: 1000, file_size: 50000, color_mode: 'rainbow', format: 'jpeg' }
                    ],
                    bilevel: [],
                    coeffs: [],
                    palette: [],
                    pdf: [],
                },
            });
            var _favoriteCall = null;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/add-favorite') {
                    _favoriteCall = { name, body, path };
                    return { added: true, count: 1, favorites: [{ job_id: 'j', artifact_id: 'color_fav', family: 'color', added_at: '2026-04-05T12:00:00Z' }] };
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await favoriteSelectedRenderArtifact(); })()', ctx); } catch(e) {}
        const favCall = vm.runInContext('_favoriteCall', ctx);
        const favText = ctx._elements['btn-render-favorite'].textContent;
        const favoriteCount = vm.runInContext('_favoriteRefs.length', ctx);
        if (!favCall || favCall.path !== '/add-favorite') { console.error('FATAL: favorite button should post to /add-favorite'); process.exit(1); }
        if (favCall.body.job_id !== 'j' || favCall.body.artifact_id !== 'color_fav') { console.error('FATAL: favorite payload should include selected job/artifact'); process.exit(1); }
        if (favText !== 'Favorited') { console.error('FATAL: favorite button should become Favorited after save, got ' + favText); process.exit(1); }
        if (favoriteCount !== 1) { console.error('FATAL: favorite action should update cached favorite refs'); process.exit(1); }
        console.log('  color favorite action posts + updates button state: OK');
    }

    {
        ctx._elements['favorites-status'] = ctx._elements['favorites-status'] || ctx._mkEl();
        ctx._elements['favorites-preview'] = ctx._elements['favorites-preview'] || ctx._mkEl();
        ctx._elements['favorites-info'] = ctx._elements['favorites-info'] || ctx._mkEl();
        ctx._elements['favorites-log'] = ctx._elements['favorites-log'] || ctx._mkEl();
        ctx._elements['btn-favorites-refresh'] = ctx._elements['btn-favorites-refresh'] || { ...ctx._mkEl(), textContent: 'Refresh', disabled: false };
        vm.runInContext(`
            _favoriteRefs = [];
            _favoriteRefsLoaded = false;
            _favoriteSelectedIdx = -1;
            var _favoriteListCalls = 0;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/list-favorites') {
                    _favoriteListCalls++;
                    return {
                        favorites: [
                            { job_id: 'job_a', artifact_id: 'color_a', family: 'color', added_at: '2026-04-05T12:00:00Z' },
                            { job_id: 'job_missing', artifact_id: 'color_missing', family: 'color', added_at: '2026-04-05T11:00:00Z' }
                        ],
                        count: 2
                    };
                }
                if (name === 'storage' && path === '/render-summary' && body.job_id === 'job_a') {
                    return {
                        calc: { exists: true, N: 1000, degree: 5 },
                        families: {
                            color: [
                                { artifact_id: 'color_a', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/job_a/color/color_a/image.jpeg', image_url: 'https://img/favorite_a.jpeg', preview_url: 'https://img/favorite_a.png', viewer_url: 'https://img/favorite_a.png', width: 1200, height: 900, file_size: 42000, color_mode: 'rainbow', format: 'jpeg' }
                            ]
                        }
                    };
                }
                if (name === 'storage' && path === '/render-summary' && body.job_id === 'job_missing') {
                    throw new Error('job missing');
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await loadFavoritesInventory(); })()', ctx); } catch(e) {}
        const favHtml = ctx._elements['favorites-preview'].innerHTML || '';
        const favInfo = ctx._elements['favorites-info'].textContent || '';
        const favListCalls = vm.runInContext('_favoriteListCalls', ctx);
        if (favListCalls !== 1) { console.error('FATAL: favorites should load refs once on first inventory load, got ' + favListCalls); process.exit(1); }
        if (!favHtml.includes('favorite-art-row-0')) { console.error('FATAL: favorites panel should render hydrated rows'); process.exit(1); }
        if (!favHtml.includes('favorite-art-row-1')) { console.error('FATAL: favorites panel should render stale rows too'); process.exit(1); }
        if (!favHtml.includes('https://img/favorite_a.png')) { console.error('FATAL: favorites panel should show selected artifact preview'); process.exit(1); }
        if (!favHtml.includes('missing artifact')) { console.error('FATAL: favorites panel should label stale refs as missing artifact'); process.exit(1); }
        if (!favInfo.includes('2 favorites loaded')) { console.error('FATAL: favorites info should show count, got ' + favInfo); process.exit(1); }
        console.log('  favorites inventory hydrates refs + keeps stale rows: OK');

        try { await vm.runInContext('(async()=>{ await refreshFavoritesInventory(); })()', ctx); } catch(e) {}
        const favListCallsAfterRefresh = vm.runInContext('_favoriteListCalls', ctx);
        if (favListCallsAfterRefresh !== 2) { console.error('FATAL: favorites refresh should force /list-favorites reload, got ' + favListCallsAfterRefresh); process.exit(1); }
        console.log('  favorites refresh forces refetch: OK');

        ctx._elements['btn-favorites-go-render'] = ctx._elements['btn-favorites-go-render'] || { ...ctx._mkEl(), textContent: 'GoRender', disabled: false };
        try {
            await vm.runInContext(`
                (async()=>{
                    _favoriteGoRenderOrigEnsure = _ensureResultsSelection;
                    _favoriteGoRenderOrigRefresh = refreshRenderArtifacts;
                    _favoriteGoRenderOrigSwitch = switchTab;
                    _favoriteGoRenderOrigLog = log;
                    _favoriteGoRenderCalls = { ensure: [], refresh: [], tabs: [], logs: [] };
                    _ensureResultsSelection = async function(jobId) { _favoriteGoRenderCalls.ensure.push(jobId); };
                    refreshRenderArtifacts = async function(jobId, opts) {
                        _favoriteGoRenderCalls.refresh.push({ jobId, opts });
                        _renderActiveFamily = (opts && opts.selectFamily) || 'color';
                        _renderArtifacts = {
                            color: [{ artifact_id: 'color_a' }],
                            bilevel: [],
                            coeffs: [],
                            palette: [],
                            pdf: [],
                        };
                    };
                    switchTab = function(name) { _favoriteGoRenderCalls.tabs.push(name); };
                    log = function(msg, cls, target) { _favoriteGoRenderCalls.logs.push({ msg, cls, target }); };
                    _favoriteSelectedIdx = 0;
                    await goRenderSelectedFavorite();
                    _ensureResultsSelection = _favoriteGoRenderOrigEnsure;
                    refreshRenderArtifacts = _favoriteGoRenderOrigRefresh;
                    switchTab = _favoriteGoRenderOrigSwitch;
                    log = _favoriteGoRenderOrigLog;
                })()
            `, ctx);
        } catch(e) { console.error('FATAL: goRenderSelectedFavorite(found): ' + e.message); process.exit(1); }
        const goFoundCalls = vm.runInContext('_favoriteGoRenderCalls', ctx);
        if (!goFoundCalls || goFoundCalls.ensure[0] !== 'job_a') { console.error('FATAL: GoRender should ensure selected job_a'); process.exit(1); }
        if (!goFoundCalls.tabs.includes('render')) { console.error('FATAL: GoRender should switch to render tab'); process.exit(1); }
        if (!goFoundCalls.refresh.length || goFoundCalls.refresh[0].jobId !== 'job_a') { console.error('FATAL: GoRender should refresh render artifacts for selected favorite job'); process.exit(1); }
        if ((goFoundCalls.refresh[0].opts || {}).selectArtifactId !== 'color_a') { console.error('FATAL: GoRender should request selected artifact id'); process.exit(1); }
        if (!goFoundCalls.logs.some(x => x.msg === 'GoRender selected: color_a' && x.target === 'favorites-log')) { console.error('FATAL: GoRender should log successful artifact selection'); process.exit(1); }
        console.log('  favorites GoRender switches to render and selects artifact when present: OK');

        try {
            await vm.runInContext(`
                (async()=>{
                    _favoriteGoRenderOrigEnsure = _ensureResultsSelection;
                    _favoriteGoRenderOrigRefresh = refreshRenderArtifacts;
                    _favoriteGoRenderOrigSwitch = switchTab;
                    _favoriteGoRenderOrigLog = log;
                    _favoriteGoRenderCalls = { ensure: [], refresh: [], tabs: [], logs: [] };
                    _ensureResultsSelection = async function(jobId) { _favoriteGoRenderCalls.ensure.push(jobId); };
                    refreshRenderArtifacts = async function(jobId, opts) {
                        _favoriteGoRenderCalls.refresh.push({ jobId, opts });
                        _renderActiveFamily = (opts && opts.selectFamily) || 'color';
                        _renderArtifacts = {
                            color: [{ artifact_id: 'different_artifact' }],
                            bilevel: [],
                            coeffs: [],
                            palette: [],
                            pdf: [],
                        };
                    };
                    switchTab = function(name) { _favoriteGoRenderCalls.tabs.push(name); };
                    log = function(msg, cls, target) { _favoriteGoRenderCalls.logs.push({ msg, cls, target }); };
                    _favoriteSelectedIdx = 1;
                    await goRenderSelectedFavorite();
                    _ensureResultsSelection = _favoriteGoRenderOrigEnsure;
                    refreshRenderArtifacts = _favoriteGoRenderOrigRefresh;
                    switchTab = _favoriteGoRenderOrigSwitch;
                    log = _favoriteGoRenderOrigLog;
                })()
            `, ctx);
        } catch(e) { console.error('FATAL: goRenderSelectedFavorite(missing): ' + e.message); process.exit(1); }
        const goMissingCalls = vm.runInContext('_favoriteGoRenderCalls', ctx);
        if (!goMissingCalls.logs.some(x => x.msg === 'Selected artifact not in render table: color_missing' && x.target === 'favorites-log')) { console.error('FATAL: GoRender should log missing artifact when not found in render table'); process.exit(1); }
        console.log('  favorites GoRender logs when selected artifact is missing from render table: OK');
    }

    {
        const summary = {
            calc: { exists: true, N: 1000, degree: 5 },
            families: {
                color: [
                    {
                        artifact_id: 'color_populate',
                        created_at: '2026-03-30T10:00:00Z',
                        image_key: 'renders/j/color/color_populate/image.jpeg',
                        image_url: 'https://img/pop.jpeg',
                        preview_url: 'https://img/pop.png',
                        viewer_url: 'https://img/pop.png',
                        width: 1000,
                        height: 1000,
                        file_size: 51000,
                        pix: 3000,
                        format: 'png',
                        quality: 82,
                        color_mode: 'solve_score',
                        solve_metric: 'anisotropy',
                        solve_score_quantile: 0.025,
                        solve_score_omega: 7,
                        match_mode: 'hungarian',
                        palette: 'tri_redgold',
                        view_mode: 'auto',
                        quantile: 0.4,
                        shim: 0.12,
                        rotation: -Math.PI / 2,
                        root_transforms: [['rotate_roots', '0.25'], ['roots_toline']],
                    }
                ],
                bilevel: [],
                coeffs: [],
                palette: [],
            },
        };
        vm.runInContext(`
            _renderActiveFamily = 'color';
            _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1 };
            renderColorMode = 'rainbow';
            renderSolveMetric = 'proximity';
            renderSolveScorePalette = 'inferno';
            renderMatchMode = 'none';
            _rtChain = [];
            document.getElementById('render-quantile').value = '0';
            document.getElementById('render-quantile-val').textContent = '0.0';
            document.getElementById('render-shim').value = '5';
            document.getElementById('render-shim-val').textContent = '5.0';
            document.getElementById('render-solve-score').value = 'proximity';
            document.getElementById('render-solve-score-quantile').value = '0.1';
            document.getElementById('render-solve-score-quantile-val').textContent = '0.1';
            document.getElementById('render-solve-score-omega').value = '1';
            document.getElementById('render-solve-score-omega-val').textContent = '1';
            document.getElementById('render-rotation').value = '0';
            document.getElementById('render-rotation-val').textContent = '0.00';
            document.getElementById('render-rotation-dir').value = 'ccw';
            document.getElementById('render-pix').value = '1024';
            document.getElementById('render-format').value = 'jpeg';
            document.getElementById('render-quality').value = '90';
            renderArtifactPanel('j', ${JSON.stringify(summary)});
            populateSelectedRenderArtifact();
        `, ctx);
        const mode = vm.runInContext('renderColorMode', ctx);
        const metric = vm.runInContext('renderSolveMetric', ctx);
        const palette = vm.runInContext('renderSolveScorePalette', ctx);
        const match = vm.runInContext('renderMatchMode', ctx);
        const q = vm.runInContext("document.getElementById('render-quantile').value", ctx);
        const shim = vm.runInContext("document.getElementById('render-shim').value", ctx);
        const sq = vm.runInContext("document.getElementById('render-solve-score-quantile').value", ctx);
        const so = vm.runInContext("document.getElementById('render-solve-score-omega').value", ctx);
        const rot = vm.runInContext("document.getElementById('render-rotation').value", ctx);
        const rotDir = vm.runInContext("document.getElementById('render-rotation-dir').value", ctx);
        const pix = vm.runInContext("document.getElementById('render-pix').value", ctx);
        const fmt = vm.runInContext("document.getElementById('render-format').value", ctx);
        const quality = vm.runInContext("document.getElementById('render-quality').value", ctx);
        const rt = vm.runInContext('JSON.stringify(_rtChain)', ctx);
        if (mode !== 'solve_score') { console.error('FATAL: populate should set solve_score mode, got ' + mode); process.exit(1); }
        if (metric !== 'anisotropy') { console.error('FATAL: populate should set anisotropy metric, got ' + metric); process.exit(1); }
        if (palette !== 'tri_redgold') { console.error('FATAL: populate should set palette tri_redgold, got ' + palette); process.exit(1); }
        if (match !== 'hungarian') { console.error('FATAL: populate should set hungarian match, got ' + match); process.exit(1); }
        if (q !== '40') { console.error('FATAL: populate should set quantile to 40, got ' + q); process.exit(1); }
        if (shim !== '12') { console.error('FATAL: populate should set shim to 12, got ' + shim); process.exit(1); }
        if (sq !== '2.5') { console.error('FATAL: populate should set solve-score q to 2.5, got ' + sq); process.exit(1); }
        if (so !== '7') { console.error('FATAL: populate should set solve-score omega to 7, got ' + so); process.exit(1); }
        if (rot !== '0.25') { console.error('FATAL: populate should set rotation turns to 0.25, got ' + rot); process.exit(1); }
        if (rotDir !== 'cw') { console.error('FATAL: populate should set rotation dir cw, got ' + rotDir); process.exit(1); }
        if (pix !== '3000') { console.error('FATAL: populate should set pix to 3000, got ' + pix); process.exit(1); }
        if (fmt !== 'png') { console.error('FATAL: populate should set format to png, got ' + fmt); process.exit(1); }
        if (quality !== '82') { console.error('FATAL: populate should set quality to 82, got ' + quality); process.exit(1); }
        if (!rt.includes('rotate_roots') || !rt.includes('roots_toline')) { console.error('FATAL: populate should restore root transforms, got ' + rt); process.exit(1); }
        vm.runInContext(`
            renderColorMode = 'rainbow';
            renderSolveMetric = 'proximity';
            renderSolveScorePalette = 'inferno';
            renderMatchMode = 'none';
            _rtChain = [];
            document.getElementById('render-quantile').value = '0';
            document.getElementById('render-quantile-val').textContent = '0.0';
            document.getElementById('render-shim').value = '5';
            document.getElementById('render-shim-val').textContent = '5.0';
            document.getElementById('render-solve-score').value = 'proximity';
            document.getElementById('render-solve-score-quantile').value = '0.1';
            document.getElementById('render-solve-score-quantile-val').textContent = '0.1';
            document.getElementById('render-solve-score-omega').value = '1';
            document.getElementById('render-solve-score-omega-val').textContent = '1';
            document.getElementById('render-rotation').value = '0';
            document.getElementById('render-rotation-val').textContent = '0.00';
            document.getElementById('render-rotation-dir').value = 'ccw';
            document.getElementById('render-pix').value = '1024';
            document.getElementById('render-format').value = 'jpeg';
            document.getElementById('render-quality').value = '90';
            _renderChips('rt');
        `, ctx);
        console.log('  color populate restores selected artifact settings: OK');
    }

    {
        const summary = {
            calc: { exists: true, N: 1000, degree: 5 },
            families: {
                color: [
                    { artifact_id: 'color_existing', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/j/color/color_existing/image.jpeg', image_url: 'https://img/color_existing.jpeg', preview_url: 'https://img/color_existing.png', viewer_url: 'https://img/color_existing.png', width: 1000, height: 1000, file_size: 50000, color_mode: 'rainbow', format: 'jpeg' }
                ],
                bilevel: [],
                coeffs: [],
                palette: [
                    {
                        artifact_id: 'pal_fill',
                        palette_id: 'pal_fill',
                        created_at: '2026-03-30T12:00:00Z',
                        image_key: 'renders/j/palettes/pal_fill/image.jpeg',
                        image_url: 'https://img/pal_fill.jpeg',
                        preview_url: 'https://img/pal_fill.png',
                        viewer_url: 'https://img/pal_fill.png',
                        width: 1000,
                        height: 1000,
                        pix: 4000,
                        format: 'jpeg',
                        file_size: 70000,
                        metric: 'crowding',
                        palette: 'reef',
                        solve_score_quantile: 0.05,
                        solve_score_omega: 4,
                        root_transforms: [['rotate_roots', '0.125']],
                    }
                ],
            },
        };
        vm.runInContext(`
            _renderActiveFamily = 'palette';
            _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1 };
            renderColorMode = 'rainbow';
            renderSolveMetric = 'proximity';
            renderSolveScorePalette = 'inferno';
            _rtChain = [];
            document.getElementById('render-solve-score').value = 'proximity';
            document.getElementById('render-solve-score-quantile').value = '0.1';
            document.getElementById('render-solve-score-quantile-val').textContent = '0.1';
            document.getElementById('render-solve-score-omega').value = '1';
            document.getElementById('render-solve-score-omega-val').textContent = '1';
            document.getElementById('render-pix').value = '1024';
            document.getElementById('render-format').value = 'png';
            renderArtifactPanel('j', ${JSON.stringify(summary)});
            populateSelectedRenderArtifact();
        `, ctx);
        const family = vm.runInContext('_renderActiveFamily', ctx);
        const mode = vm.runInContext('renderColorMode', ctx);
        const metric = vm.runInContext('renderSolveMetric', ctx);
        const palette = vm.runInContext('renderSolveScorePalette', ctx);
        const sq = vm.runInContext("document.getElementById('render-solve-score-quantile').value", ctx);
        const so = vm.runInContext("document.getElementById('render-solve-score-omega').value", ctx);
        const pix = vm.runInContext("document.getElementById('render-pix').value", ctx);
        const fmt = vm.runInContext("document.getElementById('render-format').value", ctx);
        const rt = vm.runInContext('JSON.stringify(_rtChain)', ctx);
        if (family !== 'color') { console.error('FATAL: palette populate should switch active family to color, got ' + family); process.exit(1); }
        if (mode !== 'solve_score') { console.error('FATAL: palette populate should set solve_score mode, got ' + mode); process.exit(1); }
        if (metric !== 'crowding') { console.error('FATAL: palette populate should set crowding metric, got ' + metric); process.exit(1); }
        if (palette !== 'reef') { console.error('FATAL: palette populate should set reef palette, got ' + palette); process.exit(1); }
        if (sq !== '5') { console.error('FATAL: palette populate should set solve-score q to 5, got ' + sq); process.exit(1); }
        if (so !== '4') { console.error('FATAL: palette populate should set solve-score omega to 4, got ' + so); process.exit(1); }
        if (pix !== '4000') { console.error('FATAL: palette populate should set pix to 4000, got ' + pix); process.exit(1); }
        if (fmt !== 'jpeg') { console.error('FATAL: palette populate should set format to jpeg, got ' + fmt); process.exit(1); }
        if (!rt.includes('rotate_roots') || !rt.includes('0.125')) { console.error('FATAL: palette populate should restore root transforms, got ' + rt); process.exit(1); }
        vm.runInContext(`
            _renderActiveFamily = 'color';
            renderColorMode = 'rainbow';
            renderSolveMetric = 'proximity';
            renderSolveScorePalette = 'inferno';
            _rtChain = [];
            document.getElementById('render-solve-score').value = 'proximity';
            document.getElementById('render-solve-score-quantile').value = '0.1';
            document.getElementById('render-solve-score-quantile-val').textContent = '0.1';
            document.getElementById('render-solve-score-omega').value = '1';
            document.getElementById('render-solve-score-omega-val').textContent = '1';
            document.getElementById('render-pix').value = '1024';
            document.getElementById('render-format').value = 'png';
            _renderChips('rt');
        `, ctx);
        console.log('  palette populate restores solve-score settings and switches to color: OK');
    }

    {
        const summary = {
            calc: { exists: true, N: 4000, degree: 8 },
            families: {
                color: [],
                bilevel: [],
                coeffs: [],
                palette: [
                    {
                        artifact_id: 'pal_src',
                        palette_id: 'pal_src',
                        display_name: 'crowding q=5.0% w=4 reef',
                        created_at: '2026-04-02T10:00:00Z',
                        image_key: 'renders/j/palettes/pal_src/image.jpeg',
                        image_url: 'https://img/pal_src.jpeg',
                        preview_url: 'https://img/pal_src.png',
                        viewer_url: 'https://img/pal_src.png',
                        width: 4000,
                        height: 4000,
                        file_size: 90000,
                        metric: 'crowding',
                        palette: 'reef',
                        solve_score_quantile: 0.05,
                        solve_score_omega: 4,
                        render_reusable: true,
                        data_layout: 'chunk_all_pass_v1',
                        chunk_bins_prefix: 'renders/j/palettes/pal_src/chunks/palette_bins_chunk_',
                    }
                ],
            },
        };
        vm.runInContext(`
            _renderActiveFamily = 'palette';
            _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1 };
            _repaletteDispatch = null;
            startActiveRenderObserver = function() { _repaletteObserverStarted = true; };
            _repaletteObserverStarted = false;
            lambdaPost = async function(name, body, path) {
                if (name === 'dispatch' && body.target === 'repalette') {
                    _repaletteDispatch = body;
                    return { fired: 1, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    return { errors: 0, done: 1, complete: true, status_counts: { done: 1 }, results: [{ phase: 'done', phase_label: 'Done', family: 'palette', palette_id: 'pal_new' }] };
                }
                return { ok: true };
            };
            renderArtifactPanel('j', ${JSON.stringify(summary)});
        `, ctx);
        const panelHtml = ctx._elements['render-preview'].innerHTML || '';
        if (!panelHtml.includes('RePalette')) { console.error('FATAL: palette artifact panel should show RePalette button'); process.exit(1); }
        vm.runInContext('openRepalettePopup()', ctx);
        const popupDisplay = vm.runInContext("document.getElementById('repalette-popup-overlay').style.display", ctx);
        const popupSummary = vm.runInContext("document.getElementById('repalette-popup-summary').textContent", ctx);
        if (popupDisplay !== 'flex') { console.error('FATAL: repalette popup should open'); process.exit(1); }
        if (!String(popupSummary).includes('Reusable all-pass data will be copied')) { console.error('FATAL: repalette popup should describe reusable copy path, got ' + popupSummary); process.exit(1); }
        const repaletteChildren = ctx._elements['palette-circles-repalette'].children || [];
        if (repaletteChildren.length !== 3) { console.error('FATAL: repalette palette row should collapse to PAL/TRI/LONG buttons, got ' + repaletteChildren.length); process.exit(1); }
        if ((repaletteChildren[0].textContent || '') !== 'PAL') { console.error('FATAL: repalette first swatch should be PAL button, got ' + (repaletteChildren[0].textContent || '')); process.exit(1); }
        vm.runInContext(`_openBuiltinPalettePopup('repalette')`, ctx);
        if (ctx._elements['builtin-popup-overlay'].style.display !== 'flex') { console.error('FATAL: repalette built-in popup should open on top'); process.exit(1); }
        const builtinTitle = ctx._elements['builtin-popup-title'].textContent || '';
        if (!builtinTitle.includes('RePalette')) { console.error('FATAL: repalette built-in popup title should mention RePalette, got ' + builtinTitle); process.exit(1); }
        vm.runInContext(`_closeBuiltinPalettePopup(); _openTriPalettePopup('repalette')`, ctx);
        if (ctx._elements['tri-popup-overlay'].style.display !== 'flex') { console.error('FATAL: repalette TRI popup should open on top'); process.exit(1); }
        const triTitle = ctx._elements['tri-popup-title'].textContent || '';
        if (!triTitle.includes('RePalette')) { console.error('FATAL: repalette TRI popup title should mention RePalette, got ' + triTitle); process.exit(1); }
        vm.runInContext(`_closeTriPalettePopup(); _openLongPalettePopup('repalette')`, ctx);
        if (ctx._elements['long-popup-overlay'].style.display !== 'flex') { console.error('FATAL: repalette LONG popup should open on top'); process.exit(1); }
        const longTitle = ctx._elements['long-popup-title'].textContent || '';
        if (!longTitle.includes('RePalette')) { console.error('FATAL: repalette LONG popup title should mention RePalette, got ' + longTitle); process.exit(1); }
        vm.runInContext(`_closeLongPalettePopup()`, ctx);
        vm.runInContext("setPaletteForMode('repalette', 'tri_redgold')", ctx);
        await vm.runInContext('runRepaletteSelectedArtifact()', ctx);
        const dispatch = vm.runInContext('_repaletteDispatch', ctx);
        const runMode = vm.runInContext('_activeRenderRun && _activeRenderRun.mode', ctx);
        if (!dispatch || dispatch.target !== 'repalette') { console.error('FATAL: repalette should dispatch repalette target'); process.exit(1); }
        if (dispatch.jobs[0].source_palette_id !== 'pal_src') { console.error('FATAL: repalette should send source palette id, got ' + dispatch.jobs[0].source_palette_id); process.exit(1); }
        if (dispatch.jobs[0].new_palette !== 'tri_redgold') { console.error('FATAL: repalette should send chosen palette, got ' + dispatch.jobs[0].new_palette); process.exit(1); }
        if (runMode !== 'repalette') { console.error('FATAL: repalette should save active run mode, got ' + runMode); process.exit(1); }
        console.log('  repalette popup dispatches palette reuse run: OK');
    }

    {
        vm.runInContext(`
            _saveActiveRun({ job_id: 'j', mode: 'repalette', run_id: 'run_repal', task_id: 'repalette_run', started_at_ms: Date.now() });
            _repaletteLoadOpts = null;
            _repaletteRefreshOpts = null;
            _repaletteLogText = '';
            _repaletteOrigLog = log;
            _repaletteOrigLoadPaletteInventory = loadPaletteInventory;
            _repaletteOrigRefreshRenderArtifacts = refreshRenderArtifacts;
            loadPaletteInventory = async function(opts) { _repaletteLoadOpts = opts; };
            refreshRenderArtifacts = async function(jobId, opts) { _repaletteRefreshOpts = { jobId, opts }; };
            log = function(msg, cls, target) {
                if (target === 'render-log' && String(msg).includes('RePalette complete')) _repaletteLogText = msg;
            };
            lambdaPost = async function(name, body, path) {
                if (name === 'storage' && path === '/check-status') {
                    return {
                        errors: 0,
                        done: 1,
                        complete: true,
                        status_counts: { done: 1 },
                        results: [{
                            phase: 'done',
                            phase_label: 'Done',
                            family: 'palette',
                            artifact_id: 'pal_new',
                            palette_id: 'pal_new',
                        }]
                    };
                }
                return { ok: true };
            };
        `, ctx);
        await vm.runInContext('(async()=>{ await _pollActiveRenderRun(); })()', ctx);
        const loadOpts = vm.runInContext('_repaletteLoadOpts', ctx);
        const refreshOpts = vm.runInContext('_repaletteRefreshOpts', ctx);
        const statusText = vm.runInContext("document.getElementById('render-status').textContent", ctx);
        const logText = vm.runInContext('_repaletteLogText', ctx);
        if (!loadOpts || loadOpts.selectPaletteId !== 'pal_new') { console.error('FATAL: repalette completion should refresh palette inventory with new palette selection'); process.exit(1); }
        if (!refreshOpts || refreshOpts.jobId !== 'j' || !refreshOpts.opts || refreshOpts.opts.selectFamily !== 'palette' || refreshOpts.opts.selectArtifactId !== 'pal_new') {
            console.error('FATAL: repalette completion should refresh render artifacts into palette family, got ' + JSON.stringify(refreshOpts));
            process.exit(1);
        }
        if (statusText !== 'RePalette complete') { console.error('FATAL: repalette completion status should be specific, got ' + statusText); process.exit(1); }
        if (!String(logText).includes('RePalette complete: pal_new')) { console.error('FATAL: repalette completion should log explicit completion, got ' + logText); process.exit(1); }
        vm.runInContext(`
            log = _repaletteOrigLog;
            loadPaletteInventory = _repaletteOrigLoadPaletteInventory;
            refreshRenderArtifacts = _repaletteOrigRefreshRenderArtifacts;
        `, ctx);
        console.log('  repalette completion refreshes palette inventory + render family: OK');
    }

    {
        const summary = {
            calc: { exists: true, N: 4000, degree: 8 },
            families: {
                color: [
                    {
                        artifact_id: 'color_src',
                        created_at: '2026-04-02T08:00:00Z',
                        image_key: 'renders/j/color/color_src/image.jpeg',
                        image_url: 'https://img/color_src.jpeg',
                        preview_url: 'https://img/color_src.png',
                        viewer_url: 'https://img/color_src.png',
                        width: 4000,
                        height: 4000,
                        pix: 4000,
                        format: 'jpeg',
                        quality: 90,
                        file_size: 10000,
                        color_mode: 'rainbow',
                    }
                ],
                bilevel: [],
                coeffs: [],
                palette: [
                    {
                        artifact_id: 'pal_legacy',
                        palette_id: 'pal_legacy',
                        display_name: 'legacy palette',
                        created_at: '2026-04-02T09:00:00Z',
                        image_key: 'renders/j/palettes/pal_legacy/image.jpeg',
                        image_url: 'https://img/pal_legacy.jpeg',
                        preview_url: 'https://img/pal_legacy.png',
                        viewer_url: 'https://img/pal_legacy.png',
                        width: 4000,
                        height: 4000,
                        metric: 'proximity',
                        palette: 'inferno',
                        render_reusable: false,
                        palette_bins_key: 'renders/j/palettes/pal_legacy/palette_bins.bin',
                    },
                    {
                        artifact_id: 'pal_reuse',
                        palette_id: 'pal_reuse',
                        display_name: 'anisotropy q=2.0% w=3 magma',
                        created_at: '2026-04-02T10:00:00Z',
                        image_key: 'renders/j/palettes/pal_reuse/image.jpeg',
                        image_url: 'https://img/pal_reuse.jpeg',
                        preview_url: 'https://img/pal_reuse.png',
                        viewer_url: 'https://img/pal_reuse.png',
                        width: 4000,
                        height: 4000,
                        metric: 'anisotropy',
                        palette: 'magma',
                        solve_score_quantile: 0.02,
                        solve_score_omega: 3,
                        render_reusable: true,
                        data_layout: 'chunk_all_pass_v1',
                        chunk_bins_prefix: 'renders/j/palettes/pal_reuse/chunks/palette_bins_chunk_',
                    }
                ],
            },
        };
        vm.runInContext(`
            _renderActiveFamily = 'color';
            _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1 };
            _clearActiveRun();
            _gfpDispatch = null;
            lambdaPost = async function(name, body, path) {
                if (name === 'dispatch' && body.target === 'render_orchestrator') {
                    _gfpDispatch = body;
                    return { fired: 1, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
                }
                return { ok: true };
            };
            renderArtifactPanel('j', ${JSON.stringify(summary)});
        `, ctx);
        const panelHtml = ctx._elements['render-preview'].innerHTML || '';
        if (!panelHtml.includes('GenerateFromPalette')) { console.error('FATAL: color artifact panel should show GenerateFromPalette button'); process.exit(1); }
        vm.runInContext('openGenerateFromPalettePopup()', ctx);
        const popupDisplay = vm.runInContext("document.getElementById('generate-from-palette-popup-overlay').style.display", ctx);
        const bodyRows = ctx._elements['generate-from-palette-popup-body'].children || [];
        const summaryText = vm.runInContext("document.getElementById('generate-from-palette-popup-summary').textContent", ctx);
        const paletteRow = ctx._elements['palette-circles-generate-from-palette'].children || [];
        if (popupDisplay !== 'flex') { console.error('FATAL: GenerateFromPalette popup should open'); process.exit(1); }
        if (bodyRows.length !== 1) { console.error('FATAL: GenerateFromPalette should show only reusable palettes, got ' + bodyRows.length); process.exit(1); }
        if (paletteRow.length !== 3) { console.error('FATAL: GenerateFromPalette palette row should collapse to PAL/TRI/LONG buttons, got ' + paletteRow.length); process.exit(1); }
        if ((paletteRow[0].textContent || '') !== 'PAL') { console.error('FATAL: GenerateFromPalette first swatch should be PAL button, got ' + (paletteRow[0].textContent || '')); process.exit(1); }
        if (!(String(paletteRow[0].className || '').includes('active'))) { console.error('FATAL: GenerateFromPalette should seed PAL as active from source palette magma'); process.exit(1); }
        if (!String(summaryText).includes('source palette=magma')) { console.error('FATAL: GenerateFromPalette summary should show source palette, got ' + summaryText); process.exit(1); }
        if (!String(summaryText).includes('output colorvector=magma')) { console.error('FATAL: GenerateFromPalette summary should seed output colorvector from source palette, got ' + summaryText); process.exit(1); }
        vm.runInContext(`_openTriPalettePopup('generate_from_palette')`, ctx);
        if (ctx._elements['tri-popup-overlay'].style.display !== 'flex') { console.error('FATAL: GenerateFromPalette TRI popup should open on top'); process.exit(1); }
        const triTitle = vm.runInContext("document.getElementById('tri-popup-title').textContent", ctx);
        if (!triTitle.includes('GenerateFromPalette')) { console.error('FATAL: GenerateFromPalette TRI popup title should mention GenerateFromPalette, got ' + triTitle); process.exit(1); }
        vm.runInContext(`_closeTriPalettePopup(); setPaletteForMode('generate_from_palette', 'tri_redgold')`, ctx);
        await vm.runInContext('runGenerateFromPaletteSelected()', ctx);
        const dispatch = vm.runInContext('_gfpDispatch', ctx);
        if (!dispatch || dispatch.target !== 'render_orchestrator') { console.error('FATAL: GenerateFromPalette should dispatch render orchestrator'); process.exit(1); }
        const params = dispatch.jobs[0].params || {};
        if (params.color_mode !== 'saved_palette') { console.error('FATAL: GenerateFromPalette should set color_mode=saved_palette, got ' + params.color_mode); process.exit(1); }
        if (params.saved_palette_id !== 'pal_reuse') { console.error('FATAL: GenerateFromPalette should send chosen palette id, got ' + params.saved_palette_id); process.exit(1); }
        if (params.palette !== 'tri_redgold') { console.error('FATAL: GenerateFromPalette should send chosen output palette, got ' + params.palette); process.exit(1); }
        console.log('  GenerateFromPalette popup dispatches saved-palette render: OK');
    }

    {
        const summary = {
            calc: { exists: true, N: 3000, degree: 7 },
            families: {
                color: [
                    {
                        artifact_id: 'color_src',
                        created_at: '2026-04-02T10:00:00Z',
                        image_key: 'renders/j/color/color_src/image.jpeg',
                        image_url: 'https://img/color_src.jpeg',
                        preview_url: 'https://img/color_src.png',
                        viewer_url: 'https://img/color_src.png',
                        width: 3000,
                        height: 3000,
                        pix: 3000,
                        tile_size: 2048,
                        format: 'jpeg',
                        quality: 90,
                        file_size: 64000,
                        family: 'color',
                        color_mode: 'solve_score',
                        solve_metric: 'anisotropy',
                        solve_score_quantile: 0.02,
                        solve_score_omega: 6,
                        palette: 'magma',
                        repalette_capable: true,
                        pixel_bins_prefix: 'renders/j/color/color_src/pixel_bins/tile_',
                    }
                ],
                bilevel: [],
                coeffs: [],
                palette: [],
            },
        };
        vm.runInContext(`
            _renderActiveFamily = 'color';
            _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1 };
            _clearActiveRun();
            _colorRepaletteDispatch = null;
            _colorRepaletteStartLog = null;
            _colorRepaletteOrigLog = log;
            log = function(msg, cls, target) {
                if (target === 'render-log' && String(msg).includes('Color RePalette: dispatching')) {
                    _colorRepaletteStartLog = { msg, cls, target };
                }
                return _colorRepaletteOrigLog(msg, cls, target);
            };
            lambdaPost = async function(name, body, path) {
                if (name === 'dispatch' && body.target === 'color_repalette') {
                    _colorRepaletteDispatch = body;
                    return { fired: 1, errors: [] };
                }
                return { ok: true };
            };
            renderArtifactPanel('j', ${JSON.stringify(summary)});
        `, ctx);
        const panelHtml = ctx._elements['render-preview'].innerHTML || '';
        if (!panelHtml.includes('btn-render-color-repalette')) { console.error('FATAL: color artifact panel should show Color RePalette button'); process.exit(1); }
        vm.runInContext('openColorRepalettePopup()', ctx);
        const popupDisplay = vm.runInContext("document.getElementById('color-repalette-popup-overlay').style.display", ctx);
        const popupSummary = vm.runInContext("document.getElementById('color-repalette-popup-summary').textContent", ctx);
        const paletteRow = ctx._elements['palette-circles-color-repalette'].children || [];
        if (popupDisplay !== 'flex') { console.error('FATAL: Color RePalette popup should open'); process.exit(1); }
        if (paletteRow.length !== 3) { console.error('FATAL: Color RePalette palette row should collapse to PAL/TRI/LONG buttons, got ' + paletteRow.length); process.exit(1); }
        if ((paletteRow[0].textContent || '') !== 'PAL') { console.error('FATAL: Color RePalette first swatch should be PAL button, got ' + (paletteRow[0].textContent || '')); process.exit(1); }
        if (!String(popupSummary).includes('current palette=magma')) { console.error('FATAL: Color RePalette summary should show source palette, got ' + popupSummary); process.exit(1); }
        if (!String(popupSummary).includes('output colorvector=magma')) { console.error('FATAL: Color RePalette summary should seed output colorvector, got ' + popupSummary); process.exit(1); }
        vm.runInContext(`_openTriPalettePopup('color_repalette')`, ctx);
        if (ctx._elements['tri-popup-overlay'].style.display !== 'flex') { console.error('FATAL: Color RePalette TRI popup should open on top'); process.exit(1); }
        const triTitle = vm.runInContext("document.getElementById('tri-popup-title').textContent", ctx);
        if (!triTitle.includes('Color RePalette')) { console.error('FATAL: Color RePalette TRI popup title should mention Color RePalette, got ' + triTitle); process.exit(1); }
        vm.runInContext(`_closeTriPalettePopup(); setPaletteForMode('color_repalette', 'tri_redgold')`, ctx);
        await vm.runInContext('runColorRepaletteSelectedArtifact()', ctx);
        const dispatch = vm.runInContext('_colorRepaletteDispatch', ctx);
        const runMode = vm.runInContext('_activeRenderRun && _activeRenderRun.mode', ctx);
        const startLog = vm.runInContext('_colorRepaletteStartLog', ctx);
        if (!dispatch || dispatch.target !== 'color_repalette') { console.error('FATAL: Color RePalette should dispatch color_repalette target'); process.exit(1); }
        if (dispatch.jobs[0].source_artifact_id !== 'color_src') { console.error('FATAL: Color RePalette should send source artifact id, got ' + dispatch.jobs[0].source_artifact_id); process.exit(1); }
        if (dispatch.jobs[0].new_palette !== 'tri_redgold') { console.error('FATAL: Color RePalette should send chosen palette, got ' + dispatch.jobs[0].new_palette); process.exit(1); }
        if (runMode !== 'color_repalette') { console.error('FATAL: Color RePalette should save active run mode, got ' + runMode); process.exit(1); }
        if (!startLog || startLog.cls !== 'ok') { console.error('FATAL: Color RePalette dispatch log should be green/ok, got ' + JSON.stringify(startLog)); process.exit(1); }
        vm.runInContext('log = _colorRepaletteOrigLog;', ctx);
        console.log('  Color RePalette popup dispatches fast color reuse run: OK');
    }

    {
        vm.runInContext(`
            _saveActiveRun({ job_id: 'j', mode: 'color_repalette', run_id: 'run_color_repal', task_id: 'color_repalette_run', started_at_ms: Date.now() });
            _colorRepaletteRefreshOpts = null;
            _colorRepaletteLogText = '';
            _colorRepaletteOrigLog = log;
            _colorRepaletteOrigRefreshRenderArtifacts = refreshRenderArtifacts;
            refreshRenderArtifacts = async function(jobId, opts) { _colorRepaletteRefreshOpts = { jobId, opts }; };
            log = function(msg, cls, target) {
                if (target === 'render-log' && String(msg).includes('Color RePalette complete')) _colorRepaletteLogText = msg;
            };
            lambdaPost = async function(name, body, path) {
                if (name === 'storage' && path === '/check-status') {
                    return {
                        errors: 0,
                        done: 1,
                        complete: true,
                        status_counts: { done: 1 },
                        results: [{
                            phase: 'done',
                            phase_label: 'Done',
                            family: 'color',
                            artifact_id: 'color_new',
                        }]
                    };
                }
                return { ok: true };
            };
        `, ctx);
        await vm.runInContext('(async()=>{ await _pollActiveRenderRun(); })()', ctx);
        const refreshOpts = vm.runInContext('_colorRepaletteRefreshOpts', ctx);
        const statusText = vm.runInContext("document.getElementById('render-status').textContent", ctx);
        const logText = vm.runInContext('_colorRepaletteLogText', ctx);
        if (!refreshOpts || refreshOpts.jobId !== 'j' || !refreshOpts.opts || refreshOpts.opts.selectFamily !== 'color' || refreshOpts.opts.selectArtifactId !== 'color_new') {
            console.error('FATAL: Color RePalette completion should refresh color artifacts with new selection, got ' + JSON.stringify(refreshOpts));
            process.exit(1);
        }
        if (statusText !== 'Color RePalette complete') { console.error('FATAL: Color RePalette completion status should be specific, got ' + statusText); process.exit(1); }
        if (!String(logText).includes('Color RePalette complete: color_new (')) { console.error('FATAL: Color RePalette completion should log explicit completion with elapsed seconds, got ' + logText); process.exit(1); }
        vm.runInContext(`
            log = _colorRepaletteOrigLog;
            refreshRenderArtifacts = _colorRepaletteOrigRefreshRenderArtifacts;
        `, ctx);
        console.log('  Color RePalette completion refreshes color inventory: OK');
    }

    {
        const summary = {
            calc: { exists: true, N: 3000, degree: 7 },
            families: {
                color: [
                    {
                        artifact_id: 'color_src',
                        created_at: '2026-04-02T10:00:00Z',
                        image_key: 'renders/j/color/color_src/image.jpeg',
                        image_url: 'https://img/color_src.jpeg',
                        preview_url: 'https://img/color_src.png',
                        viewer_url: 'https://img/color_src.png',
                        width: 3000,
                        height: 2000,
                        pix: 3000,
                        format: 'jpeg',
                        quality: 81,
                        file_size: 64000,
                        color_mode: 'solve_score',
                        solve_metric: 'anisotropy',
                        solve_score_quantile: 0.02,
                        solve_score_omega: 6,
                        palette: 'tri_redgold',
                        background_color: '101214',
                        background_threshold: 7,
                        root_transforms: [['rotate_roots', '0.125']],
                    }
                ],
                bilevel: [],
                coeffs: [],
                palette: [],
            },
        };
        vm.runInContext(`
            _renderActiveFamily = 'color';
            _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1 };
            _clearActiveRun();
            _autolevelDispatch = null;
            _autolevelStartLog = null;
            _autolevelOrigLog = log;
            startActiveRenderObserver = function() { _autolevelObserverStarted = true; };
            _autolevelObserverStarted = false;
            log = function(msg, cls, target) {
                if (target === 'render-log' && String(msg).includes('Autolevels: dispatching')) {
                    _autolevelStartLog = { msg, cls, target };
                }
                return _autolevelOrigLog(msg, cls, target);
            };
            lambdaPost = async function(name, body, path) {
                if (name === 'dispatch' && body.target === 'autolevels') {
                    _autolevelDispatch = body;
                    return { fired: 1, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    return { errors: 0, done: 1, complete: true, status_counts: { done: 1 }, results: [{ phase: 'done', phase_label: 'Done', family: 'color', artifact_id: 'autolevels_done' }] };
                }
                return { ok: true };
            };
            renderArtifactPanel('j', ${JSON.stringify(summary)});
        `, ctx);
        const panelHtml = ctx._elements['render-preview'].innerHTML || '';
        if (!panelHtml.includes('Autolevels')) { console.error('FATAL: color artifact panel should show Autolevels button'); process.exit(1); }
        vm.runInContext('openAutolevelPopup()', ctx);
        const overlayDisplay = vm.runInContext("document.getElementById('autolevel-popup-overlay').style.display", ctx);
        const popupSummary = vm.runInContext("document.getElementById('autolevel-popup-summary').textContent", ctx);
        const popupQuality = vm.runInContext("document.getElementById('autolevel-quality').value", ctx);
        const popupBackground = vm.runInContext("document.getElementById('autolevel-background-readout').value", ctx);
        const popupThreshold = vm.runInContext("document.getElementById('autolevel-background-threshold').value", ctx);
        const popupExclude = vm.runInContext("document.getElementById('autolevel-exclude-background').checked", ctx);
        const popupPooledToggle = vm.runInContext("document.getElementById('autolevel-enable-pooled-rgb').checked", ctx);
        const popupPooledQ = vm.runInContext("document.getElementById('autolevel-pooled-rgb').value", ctx);
        if (overlayDisplay !== 'flex') { console.error('FATAL: autolevel popup should open'); process.exit(1); }
        if (!String(popupSummary).includes('color_src')) { console.error('FATAL: autolevel popup should describe selected artifact, got ' + popupSummary); process.exit(1); }
        if (!String(popupSummary).includes('bg=#101214')) { console.error('FATAL: autolevel popup should describe background, got ' + popupSummary); process.exit(1); }
        if (!String(popupSummary).includes('tol/ch=7')) { console.error('FATAL: autolevel popup should describe per-channel tolerance, got ' + popupSummary); process.exit(1); }
        if (popupQuality !== '81') { console.error('FATAL: autolevel popup should seed quality from artifact, got ' + popupQuality); process.exit(1); }
        if (popupBackground !== '#101214') { console.error('FATAL: autolevel popup should show background readout, got ' + popupBackground); process.exit(1); }
        if (popupThreshold !== '7') { console.error('FATAL: autolevel popup should seed threshold from artifact, got ' + popupThreshold); process.exit(1); }
        if (!popupExclude) { console.error('FATAL: autolevel popup should default background exclusion on'); process.exit(1); }
        if (!popupPooledToggle || popupPooledQ !== '0.1') { console.error('FATAL: autolevel popup should default final pooled stretch to on with q=0.1, got toggle=' + popupPooledToggle + ' q=' + popupPooledQ); process.exit(1); }
        vm.runInContext(`
            document.getElementById('autolevel-exclude-background').checked = false;
            _syncAutolevelBackgroundControls();
        `, ctx);
        const thresholdDisabled = vm.runInContext("document.getElementById('autolevel-background-threshold').disabled", ctx);
        if (!thresholdDisabled) { console.error('FATAL: autolevel threshold should disable when background exclusion is off'); process.exit(1); }
        vm.runInContext(`
            document.getElementById('autolevel-exclude-background').checked = true;
            _syncAutolevelBackgroundControls();
        `, ctx);
        const thresholdEnabled = vm.runInContext("document.getElementById('autolevel-background-threshold').disabled", ctx);
        if (thresholdEnabled) { console.error('FATAL: autolevel threshold should re-enable when background exclusion is on'); process.exit(1); }
        vm.runInContext(`
            document.getElementById('autolevel-quality').value = '84';
            document.getElementById('autolevel-enable-gamma').checked = true;
            document.getElementById('autolevel-gamma').value = '1.2';
            document.getElementById('autolevel-enable-auto-gamma').checked = true;
            document.getElementById('autolevel-auto-gamma').value = 'median';
            document.getElementById('autolevel-background-threshold').value = '11';
            document.getElementById('autolevel-exclude-background').checked = false;
            document.getElementById('autolevel-jpeg-optimize').checked = true;
            document.getElementById('autolevel-jpeg-interlace').checked = true;
            _syncAutolevelStageControls();
        `, ctx);
        vm.runInContext('_revertAutolevelPopupDefaults()', ctx);
        const revertedQuality = vm.runInContext("document.getElementById('autolevel-quality').value", ctx);
        const revertedPooled = vm.runInContext("document.getElementById('autolevel-pooled-rgb').value", ctx);
        const revertedGammaToggle = vm.runInContext("document.getElementById('autolevel-enable-gamma').checked", ctx);
        if (revertedQuality !== '81' || revertedPooled !== '0.1' || revertedGammaToggle) { console.error('FATAL: autolevel revert should restore artifact defaults, got quality=' + revertedQuality + ' pooled=' + revertedPooled + ' gammaToggle=' + revertedGammaToggle); process.exit(1); }
        vm.runInContext(`
            document.getElementById('autolevel-quality').value = '84';
            document.getElementById('autolevel-enable-auto-gamma').checked = true;
            document.getElementById('autolevel-auto-gamma').value = 'median';
            document.getElementById('autolevel-background-threshold').value = '11';
            document.getElementById('autolevel-exclude-background').checked = false;
            document.getElementById('autolevel-jpeg-optimize').checked = true;
            document.getElementById('autolevel-jpeg-interlace').checked = true;
            _syncAutolevelStageControls();
        `, ctx);
        await vm.runInContext('runAutolevelSelectedRenderArtifact()', ctx);
        const dispatch = vm.runInContext('_autolevelDispatch', ctx);
        const startLog = vm.runInContext('_autolevelStartLog', ctx);
        const popupClosed = vm.runInContext("document.getElementById('autolevel-popup-overlay').style.display", ctx);
        const observerStarted = vm.runInContext('_autolevelObserverStarted', ctx);
        const runMode = vm.runInContext('_activeRenderRun && _activeRenderRun.mode', ctx);
        if (!dispatch || dispatch.target !== 'autolevels') { console.error('FATAL: autolevels should dispatch via dispatch Lambda'); process.exit(1); }
        if (!dispatch.jobs || dispatch.jobs.length !== 1) { console.error('FATAL: autolevels dispatch should send one job'); process.exit(1); }
        if (dispatch.jobs[0].source_artifact_id !== 'color_src') { console.error('FATAL: autolevels should target selected color artifact, got ' + dispatch.jobs[0].source_artifact_id); process.exit(1); }
        if (dispatch.jobs[0].autolevels_params.quality !== 84) { console.error('FATAL: autolevels should send edited quality, got ' + dispatch.jobs[0].autolevels_params.quality); process.exit(1); }
        if (dispatch.jobs[0].autolevels_params.enable_pooled_rgb !== true || dispatch.jobs[0].autolevels_params.pooled_rgb !== 0.1) { console.error('FATAL: autolevels should keep final pooled stretch default on with q=0.1'); process.exit(1); }
        if (dispatch.jobs[0].autolevels_params.auto_gamma !== 'median') { console.error('FATAL: autolevels should send edited auto_gamma'); process.exit(1); }
        if (dispatch.jobs[0].autolevels_params.enable_auto_gamma !== true) { console.error('FATAL: autolevels should send auto-gamma stage toggle'); process.exit(1); }
        if (dispatch.jobs[0].autolevels_params.background_threshold !== 11) { console.error('FATAL: autolevels should send edited background threshold, got ' + dispatch.jobs[0].autolevels_params.background_threshold); process.exit(1); }
        if (dispatch.jobs[0].autolevels_params.exclude_background !== false) { console.error('FATAL: autolevels should send edited exclude_background'); process.exit(1); }
        if (!dispatch.jobs[0].autolevels_params.jpeg_optimize_coding || !dispatch.jobs[0].autolevels_params.jpeg_interlace) { console.error('FATAL: autolevels should send checkbox params'); process.exit(1); }
        if (popupClosed !== 'none') { console.error('FATAL: autolevel popup should close after dispatch'); process.exit(1); }
        if (!observerStarted) { console.error('FATAL: autolevel dispatch should start render observer'); process.exit(1); }
        if (runMode !== 'autolevels') { console.error('FATAL: autolevel dispatch should save active render run mode, got ' + runMode); process.exit(1); }
        if (!startLog || startLog.cls !== 'ok') { console.error('FATAL: autolevel dispatch log should be green/ok, got ' + JSON.stringify(startLog)); process.exit(1); }
        vm.runInContext(`
            _autolevelDispatch = null;
            openAutolevelPopup();
            _closeAutolevelPopup();
            log = _autolevelOrigLog;
        `, ctx);
        const canceledDispatch = vm.runInContext('_autolevelDispatch', ctx);
        if (canceledDispatch !== null) { console.error('FATAL: closing autolevel popup should not dispatch'); process.exit(1); }
        console.log('  autolevel popup dispatches derived color artifact run: OK');
    }

    {
        const summary = {
            calc: { exists: true, N: 2000, degree: 5 },
            families: {
                color: [
                    {
                        artifact_id: 'color_old',
                        created_at: '2026-04-02T08:00:00Z',
                        image_key: 'renders/j/color/color_old/image.jpeg',
                        image_url: 'https://img/color_old.jpeg',
                        preview_url: 'https://img/color_old.png',
                        viewer_url: 'https://img/color_old.png',
                        width: 2000,
                        height: 2000,
                        pix: 2000,
                        format: 'jpeg',
                        quality: 90,
                        file_size: 32000,
                        color_mode: 'rainbow',
                        root_transforms: [],
                    }
                ],
                bilevel: [],
                coeffs: [],
                palette: [],
            },
        };
        vm.runInContext(`
            _renderActiveFamily = 'color';
            _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1 };
            renderArtifactPanel('j', ${JSON.stringify(summary)});
            openAutolevelPopup();
        `, ctx);
        const fallbackBackground = vm.runInContext("document.getElementById('autolevel-background-readout').value", ctx);
        const fallbackThreshold = vm.runInContext("document.getElementById('autolevel-background-threshold').value", ctx);
        if (fallbackBackground !== '#000000') { console.error('FATAL: autolevel popup should default background readout to black for old artifacts, got ' + fallbackBackground); process.exit(1); }
        if (fallbackThreshold !== '4') { console.error('FATAL: autolevel popup should default threshold to 4 for old artifacts, got ' + fallbackThreshold); process.exit(1); }
        vm.runInContext('_closeAutolevelPopup()', ctx);
        console.log('  autolevel popup fallback background defaults: OK');
    }

    // Step 9b: PDF family UI + ColorSpread popup
    {
        const summary = {
            calc: { exists: true, N: 500, degree: 24 },
            families: {
                color: [
                    {
                        artifact_id: 'color_src',
                        created_at: '2026-04-04T09:00:00Z',
                        image_key: 'renders/j/color/color_src/image.jpeg',
                        image_url: 'https://img/color_src.jpeg',
                        preview_url: 'https://img/color_src.png',
                        viewer_url: 'https://img/color_src.png',
                        content_type: 'image/jpeg',
                        width: 5000,
                        height: 5000,
                        format: 'jpeg',
                        file_size: 12345,
                        color_mode: 'solve_score',
                        solve_metric: 'clusteriness',
                        solve_score_quantile: 0.05,
                        solve_score_omega: 1,
                        palette: 'inferno',
                        root_transforms: [],
                    }
                ],
                bilevel: [],
                coeffs: [],
                palette: [],
                pdf: [
                    {
                        artifact_id: 'pdf_1',
                        created_at: '2026-04-04T10:00:00Z',
                        image_key: 'renders/j/pdf/pdf_1/document.pdf',
                        image_url: 'https://pdf/pdf_1.pdf',
                        viewer_url: 'https://pdf/pdf_1.pdf',
                        content_type: 'application/pdf',
                        format: 'pdf',
                        file_size: 9999,
                        pdf_kind: 'color_spread',
                        source_artifact_id: 'color_src',
                        source_display_name: 'solve:clusteriness q=5.0% w=1 inferno',
                    }
                ],
            },
        };
        await vm.runInContext(`
            window._lastDispatchBody = null;
            _activeRenderRun = null;
            lambdaPost = async function(name, body) {
                if (name === 'dispatch') {
                    window._lastDispatchBody = body;
                    return { fired: 1 };
                }
                return {};
            };
            _renderActiveFamily = 'pdf';
            _renderSelectedArtifact = { color: 0, bilevel: -1, coeffs: -1, palette: -1, pdf: 0 };
            renderArtifactPanel('j', ${JSON.stringify(summary)});
        `, ctx);
        const panelHtml = vm.runInContext("document.getElementById('render-preview').innerHTML", ctx);
        if (!panelHtml.includes('ColorSpread')) { console.error('FATAL: pdf artifact panel should show ColorSpread button'); process.exit(1); }
        if (!panelHtml.includes('<iframe')) { console.error('FATAL: pdf artifact viewer should use iframe/embed branch'); process.exit(1); }
        await vm.runInContext('openPdfColorSpreadPopup()', ctx);
        const popupDisplay = vm.runInContext("document.getElementById('pdf-colorspread-popup-overlay').style.display", ctx);
        const bodyRows = vm.runInContext("document.getElementById('pdf-colorspread-popup-body').children.length", ctx);
        const summaryText = vm.runInContext("document.getElementById('pdf-colorspread-popup-summary').textContent", ctx);
        if (popupDisplay !== 'flex') { console.error('FATAL: PDF ColorSpread popup should open'); process.exit(1); }
        if (bodyRows !== 1) { console.error('FATAL: PDF ColorSpread popup should list 1 color artifact, got ' + bodyRows); process.exit(1); }
        if (!String(summaryText).includes('color_src')) { console.error('FATAL: PDF ColorSpread summary should mention selected color source, got ' + summaryText); process.exit(1); }
        await vm.runInContext('runPdfColorSpreadSelected()', ctx);
        const dispatch = vm.runInContext('window._lastDispatchBody', ctx);
        const runMode = vm.runInContext('_activeRenderRun && _activeRenderRun.mode', ctx);
        if (!dispatch || dispatch.target !== 'pdf_artifact') { console.error('FATAL: PDF ColorSpread should dispatch pdf_artifact target'); process.exit(1); }
        if (dispatch.jobs[0].source_artifact_id !== 'color_src') { console.error('FATAL: PDF ColorSpread should send source artifact id, got ' + dispatch.jobs[0].source_artifact_id); process.exit(1); }
        if (dispatch.jobs[0].source_image_key !== 'renders/j/color/color_src/image.jpeg') { console.error('FATAL: PDF ColorSpread should send source image key, got ' + dispatch.jobs[0].source_image_key); process.exit(1); }
        if (runMode !== 'pdf') { console.error('FATAL: PDF ColorSpread should save active run mode pdf, got ' + runMode); process.exit(1); }
        console.log('  PDF family viewer + ColorSpread dispatch: OK');
    }

    // Step 10: DeepZoom inventory UI tests
    console.log('');
    console.log('--- DeepZoom inventory ---');

    // Stub lambdaPost for inventory loading (single /list-deepzoom call)
    vm.runInContext(`
        var _dzListCalls = 0;
        lambdaPost = async function lambdaPost(name, body, path) {
            if (name === 'storage' && path === '/list-deepzoom') {
                _dzListCalls++;
                return { exports: [
                    { job_id: 'job_b', width: 8192, height: 8192, created_at: '2026-03-25T12:00:00', tiles_uploaded: 400, dzi_url: 'https://dz/job_b.dzi' },
                    { job_id: 'job_a', width: 4096, height: 4096, created_at: '2026-03-25T10:00:00', tiles_uploaded: 100, dzi_url: 'https://dz/job_a.dzi' },
                ], count: 2 };
            }
            return {};
        };
    `, ctx);

    // Test: loadDeepZoomInventory populates the inventory
    try {
        await vm.runInContext('(async()=>{ await loadDeepZoomInventory(); })()', ctx);
        const inv = vm.runInContext('window._dzInventory', ctx);
        if (!inv || inv.length !== 2) {
            console.error('FATAL: inventory should have 2 entries, got ' + (inv ? inv.length : 'null'));
            process.exit(1);
        }
        // Sorted newest first — job_b (12:00) before job_a (10:00)
        if (inv[0].job_id !== 'job_b') {
            console.error('FATAL: newest should be first, got ' + inv[0].job_id);
            process.exit(1);
        }
        console.log('  inventory load: OK (2 entries, newest first)');
    } catch (e) {
        console.error('FATAL: inventory load: ' + e.message);
        process.exit(1);
    }

    // Test: auto-selects first (newest) entry
    {
        const idx = vm.runInContext('window._dzSelectedIdx', ctx);
        if (idx !== 0) {
            console.error('FATAL: auto-select should be 0, got ' + idx);
            process.exit(1);
        }
        console.log('  auto-select newest: OK (idx=0, job_b)');
    }

    // Test: _dzSelect changes selection and viewer
    try {
        vm.runInContext('_dzSelect(1)', ctx);
        const idx = vm.runInContext('window._dzSelectedIdx', ctx);
        if (idx !== 1) {
            console.error('FATAL: _dzSelect(1) should set idx=1, got ' + idx);
            process.exit(1);
        }
        console.log('  click select: OK (idx=1, job_a)');
    } catch (e) {
        console.error('FATAL: _dzSelect: ' + e.message);
        process.exit(1);
    }

    // Test: _dzSelect out of bounds is safe
    try {
        vm.runInContext('_dzSelect(-1)', ctx);
        vm.runInContext('_dzSelect(999)', ctx);
        const idx = vm.runInContext('window._dzSelectedIdx', ctx);
        if (idx !== 1) {
            console.error('FATAL: out-of-bounds select should not change idx, got ' + idx);
            process.exit(1);
        }
        console.log('  out-of-bounds select: OK (idx unchanged)');
    } catch (e) {
        console.error('FATAL: out-of-bounds _dzSelect: ' + e.message);
        process.exit(1);
    }

    // Test: switchTab('deepzoom') triggers inventory load
    {
        vm.runInContext('_dzListCalls = 0', ctx);
        try {
            await vm.runInContext("(async()=>{ switchTab('deepzoom'); await new Promise(r => setTimeout(r, 0)); })()", ctx);
            const calls = vm.runInContext('_dzListCalls', ctx);
            if (calls < 1) {
                console.error('FATAL: switchTab(deepzoom) should trigger loadDeepZoomInventory, got ' + calls + ' list calls');
                process.exit(1);
            }
            console.log('  tab switch auto-load: OK (' + calls + ' list calls)');
        } catch (e) {
            console.error('FATAL: tab switch auto-load: ' + e.message);
            process.exit(1);
        }
    }

    // Test: rendered inventory has correct row count
    // Re-load to ensure clean state after tab switch test
    await vm.runInContext('(async()=>{ await loadDeepZoomInventory(); })()', ctx);
    {
        const container = ctx._elements['deepzoom-inventory'];
        const html = container.innerHTML || '';
        const rowCount = (html.match(/dz-inv-row/g) || []).length;
        if (rowCount !== 2) {
            console.error('FATAL: rendered inventory should have 2 rows, got ' + rowCount);
            process.exit(1);
        }
        console.log('  rendered rows: OK (' + rowCount + ' dz-inv-row elements)');
    }

    // Test: viewer element made visible after auto-select
    {
        const viewer = ctx._elements['deepzoom-viewer'];
        if (viewer.style.display !== 'block') {
            console.error('FATAL: viewer should be visible after auto-select, display=' + viewer.style.display);
            process.exit(1);
        }
        console.log('  viewer visible after load: OK');
    }

    // Test: _dzSelect calls viewDeepZoom with correct URL
    {
        let viewedUrl = null;
        vm.runInContext(`
            var _lastViewedDziUrl = null;
            var _origViewDeepZoom = viewDeepZoom;
            viewDeepZoom = function(url) { _lastViewedDziUrl = url; _origViewDeepZoom(url); };
        `, ctx);
        vm.runInContext('_dzSelect(0)', ctx);
        const url = vm.runInContext('_lastViewedDziUrl', ctx);
        if (url !== 'https://dz/job_b.dzi') {
            console.error('FATAL: _dzSelect(0) should view job_b dzi, got ' + url);
            process.exit(1);
        }
        vm.runInContext('_dzSelect(1)', ctx);
        const url2 = vm.runInContext('_lastViewedDziUrl', ctx);
        if (url2 !== 'https://dz/job_a.dzi') {
            console.error('FATAL: _dzSelect(1) should view job_a dzi, got ' + url2);
            process.exit(1);
        }
        console.log('  select→viewDeepZoom URL: OK (job_b.dzi, job_a.dzi)');
    }

    // Test: arrow key events change selection
    {
        // Start at idx=1 (from previous test)
        // Simulate ArrowUp → should go to 0
        const KeyboardEvent = vm.runInContext('typeof KeyboardEvent', ctx);
        // Can't construct real KeyboardEvent in VM, but we can call the handler directly
        // The handler is registered via document.addEventListener('keydown', fn)
        // Our stub doesn't capture it, so test via direct function call
        vm.runInContext(`
            // Make the deepzoom tab "active" for the handler
            document.getElementById('tab-deepzoom').classList = { contains: function(c) { return c === 'active'; } };
            window._dzSelectedIdx = 1;
        `, ctx);
        // Simulate ArrowUp
        vm.runInContext(`
            // Find the keydown handler — it was registered but our stub discarded it
            // Instead, directly invoke the navigation logic
            var _fakeEvent = { key: 'ArrowUp', preventDefault: function(){} };
            // Re-register so we can call it
            var _dzKeyHandler = null;
            var _origAddEvt = document.addEventListener;
            document.addEventListener = function(evt, fn) { if (evt === 'keydown') _dzKeyHandler = fn; };
        `, ctx);
        // Re-eval just the arrow key handler to capture it
        const appCode = fs.readFileSync(process.argv[5], 'utf8');
        const handlerMatch = appCode.match(/\/\/ Arrow key navigation for deepzoom inventory[\s\S]*?(?=\nfunction viewDeepZoom)/);
        if (handlerMatch) {
            vm.runInContext(handlerMatch[0], ctx);
            // Now call it
            vm.runInContext('if (_dzKeyHandler) _dzKeyHandler({key:"ArrowUp",preventDefault:function(){}})', ctx);
            const idx = vm.runInContext('window._dzSelectedIdx', ctx);
            if (idx !== 0) {
                console.error('FATAL: ArrowUp from 1 should select 0, got ' + idx);
                process.exit(1);
            }
            vm.runInContext('if (_dzKeyHandler) _dzKeyHandler({key:"ArrowDown",preventDefault:function(){}})', ctx);
            const idx2 = vm.runInContext('window._dzSelectedIdx', ctx);
            if (idx2 !== 1) {
                console.error('FATAL: ArrowDown from 0 should select 1, got ' + idx2);
                process.exit(1);
            }
            // ArrowDown at end stays at end
            vm.runInContext('if (_dzKeyHandler) _dzKeyHandler({key:"ArrowDown",preventDefault:function(){}})', ctx);
            const idx3 = vm.runInContext('window._dzSelectedIdx', ctx);
            if (idx3 !== 1) {
                console.error('FATAL: ArrowDown at end should stay at 1, got ' + idx3);
                process.exit(1);
            }
            console.log('  arrow key navigation: OK (up:1→0, down:0→1, down:1→1)');
        } else {
            console.error('FATAL: could not extract arrow key handler from app code');
            process.exit(1);
        }
    }

    // Test: DeepZoom selected-row actions bridge to Results and Compute
    {
        ctx._elements['btn-dz-goto-result'] = { ...ctx._mkEl(), disabled: true, textContent: 'Goto Result' };
        ctx._elements['btn-dz-populate'] = { ...ctx._mkEl(), disabled: true, textContent: 'Populate' };
        ctx._elements['btn-dz-delete'] = { ...ctx._mkEl(), disabled: true, textContent: 'Delete' };
        ctx._elements['btn-populate-result'] = { ...ctx._mkEl(), disabled: true, textContent: 'Populate' };
        ctx._elements['btn-preview'] = { ...ctx._mkEl(), disabled: true, textContent: 'Preview' };
        ctx._elements['btn-render-result'] = { ...ctx._mkEl(), disabled: true, textContent: 'Render' };
        ctx._elements['btn-delete'] = { ...ctx._mkEl(), disabled: true, textContent: 'Delete' };
        ctx._elements['results-scroll'] = ctx._elements['results-scroll'] || ctx._mkEl();
        ctx._elements['results-preview'] = ctx._elements['results-preview'] || ctx._mkEl();
        ctx._elements['results-info'] = { ...ctx._mkEl(), textContent: '' };
        ctx._elements['compute-status'] = { ...ctx._mkEl(), textContent: '' };
        ctx._elements['compute-log'] = ctx._mkEl();
        ctx._elements['results-dir'] = { ...ctx._mkEl(), value: '' };
        ctx._elements['render-results-dir'] = ctx._elements['render-results-dir'] || { ...ctx._mkEl(), value: '' };
        ctx._elements['palette-results-dir'] = ctx._elements['palette-results-dir'] || { ...ctx._mkEl(), value: '' };
        ctx._elements['render-n'] = { ...ctx._mkEl(), value: '500' };
        ctx._elements['render-times'] = { ...ctx._mkEl(), value: '1' };
        ctx._elements['render-stripes'] = { ...ctx._mkEl(), value: '10' };
        ctx._elements['render-function'] = ctx._elements['render-function'] || { ...ctx._mkEl(), value: 'g1' };
        ctx._elements['pt-chips'] = ctx._elements['pt-chips'] || ctx._mkEl();
        ctx._elements['ct-chips'] = ctx._elements['ct-chips'] || ctx._mkEl();
        ctx._elements['cfpv-row'] = ctx._elements['cfpv-row'] || ctx._mkEl();
        ctx._elements['cfpv-inputs'] = ctx._elements['cfpv-inputs'] || ctx._mkEl();

        vm.runInContext(`
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/list-deepzoom') {
                    return { exports: [
                        { job_id: 'job_b', width: 8192, height: 8192, created_at: '2026-03-25T12:00:00', tiles_uploaded: 400, dzi_url: 'https://dz/job_b.dzi' },
                        { job_id: 'job_a', width: 4096, height: 4096, created_at: '2026-03-25T10:00:00', tiles_uploaded: 100, dzi_url: 'https://dz/job_a.dzi' },
                    ], count: 2 };
                }
                if (name === 'storage' && path === '/list') {
                    return {
                        count: 2,
                        list_us: 1000,
                        results: [
                            { job_id: 'job_a', function: 'creative9', degree: 7, N: 1200, n_chunks: 24, total_size: 1000 },
                            { job_id: 'job_b', function: 'g1', degree: 4, N: 800, n_chunks: 12, total_size: 500 },
                        ]
                    };
                }
                if (name === 'storage' && path === '/detail') {
                    if (body.job_id === 'job_a') {
                        return {
                            times: 3,
                            param_transforms_display: [['unit_circle'], ['rtheta', '2']],
                            coeff_transforms: ['rev', 'conj'],
                            pipeline: {
                                function: 'creative9',
                                param_transforms_display: [['unit_circle'], ['rtheta', '2']],
                                coeff_transforms: ['rev', 'conj'],
                                cfpv: [88]
                            },
                            calc: {
                                N: 1200,
                                n_chunks: 24,
                                solver: 'companion_matrix'
                            }
                        };
                    }
                    return {
                        times: 1,
                        pipeline: { function: 'g1' },
                        calc: { N: 800, n_chunks: 12, solver: 'aberth' }
                    };
                }
                return {};
            };
            _resultsCache = [{
                job_id: 'job_a',
                function: 'creative9',
                degree: 7,
                N: 1200,
                n_chunks: 24,
                _detail: {
                    times: 3,
                    param_transforms_display: [['unit_circle'], ['rtheta', '2']],
                    coeff_transforms: ['rev', 'conj'],
                    pipeline: {
                        function: 'creative9',
                        param_transforms_display: [['unit_circle'], ['rtheta', '2']],
                        coeff_transforms: ['rev', 'conj'],
                        cfpv: [88]
                    },
                    calc: {
                        N: 1200,
                        n_chunks: 24,
                        solver: 'companion_matrix'
                    }
                }
            }, {
                job_id: 'job_b',
                function: 'g1',
                degree: 4,
                N: 800,
                n_chunks: 12,
                _detail: {
                    times: 1,
                    pipeline: { function: 'g1' },
                    calc: { N: 800, n_chunks: 12, solver: 'aberth' }
                }
            }];
            _selectedJobId = null;
            _dzSelect(1);
        `, ctx);

        const dzGotoDisabled = ctx._elements['btn-dz-goto-result'].disabled;
        const dzPopulateDisabled = ctx._elements['btn-dz-populate'].disabled;
        const dzDeleteDisabled = ctx._elements['btn-dz-delete'].disabled;
        if (dzGotoDisabled || dzPopulateDisabled || dzDeleteDisabled) {
            console.error('FATAL: deepzoom action buttons should enable after selection');
            process.exit(1);
        }

        try {
            await vm.runInContext('(async()=>{ await _dzGotoSelectedResult(); })()', ctx);
            const selectedJob = vm.runInContext('_selectedJobId', ctx);
            const renderDirVal = ctx._elements['render-results-dir'].value;
            const paletteDirVal = ctx._elements['palette-results-dir'].value;
            const resultsDirVal = ctx._elements['results-dir'].value;
            if (selectedJob !== 'job_a') { console.error('FATAL: deepzoom goto should select job_a, got ' + selectedJob); process.exit(1); }
            if (renderDirVal !== 'job_a' || paletteDirVal !== 'job_a' || resultsDirVal !== 'job_a') {
                console.error('FATAL: deepzoom goto should populate result dirs with job_a');
                process.exit(1);
            }
            console.log('  deepzoom goto result selects matching result: OK');
        } catch (e) {
            console.error('FATAL: deepzoom goto result: ' + e.message);
            process.exit(1);
        }

        try {
            await vm.runInContext('(async()=>{ await _dzPopulateSelectedResult(); })()', ctx);
            const nVal = ctx._elements['render-n'].value;
            const timesVal = ctx._elements['render-times'].value;
            const chunksVal = ctx._elements['render-stripes'].value;
            const funcVal = ctx._elements['render-function'].value;
            const status = ctx._elements['compute-status'].textContent;
            if (nVal !== '1200') { console.error('FATAL: deepzoom populate should set N=1200, got ' + nVal); process.exit(1); }
            if (timesVal !== '3') { console.error('FATAL: deepzoom populate should set times=3, got ' + timesVal); process.exit(1); }
            if (chunksVal !== '24') { console.error('FATAL: deepzoom populate should set chunks=24, got ' + chunksVal); process.exit(1); }
            if (funcVal !== 'creative9') { console.error('FATAL: deepzoom populate should set function creative9, got ' + funcVal); process.exit(1); }
            if (!status.includes('Populated from job_a')) { console.error('FATAL: deepzoom populate should update compute status, got ' + status); process.exit(1); }
            console.log('  deepzoom populate restores compute settings: OK');
        } catch (e) {
            console.error('FATAL: deepzoom populate: ' + e.message);
            process.exit(1);
        }
    }

    // Test: DeepZoom delete removes only selected row and keeps nearby selection
    {
        ctx._elements['btn-dz-delete'] = { ...ctx._mkEl(), disabled: false, textContent: 'Delete' };
        ctx._elements['deepzoom-status'] = { ...ctx._mkEl(), textContent: '' };
        vm.runInContext(`
            var _dzDeletedPrefix = null;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/delete-prefix') {
                    _dzDeletedPrefix = body.prefix;
                    return { deleted: 5 };
                }
                if (name === 'storage' && path === '/presign') {
                    return { url: 'https://ptr/deepzoom_latest.json' };
                }
                if (name === 'storage' && path === '/cleanup') {
                    return { deleted: 1 };
                }
                return {};
            };
            fetch = async function(url) {
                return { ok: true, json: async function() { return { export_id: 'dz_mid' }; } };
            };
            window._dzInventory = [
                { job_id: 'job_top', export_id: 'dz_top', width: 4000, height: 4000, created_at: '2026-03-25T12:00:00', tiles_uploaded: 100, dzi_url: 'https://dz/job_top.dzi' },
                { job_id: 'job_mid', export_id: 'dz_mid', width: 3000, height: 3000, created_at: '2026-03-25T11:00:00', tiles_uploaded: 80, dzi_url: 'https://dz/job_mid.dzi' },
                { job_id: 'job_low', export_id: 'dz_low', width: 2000, height: 2000, created_at: '2026-03-25T10:00:00', tiles_uploaded: 60, dzi_url: 'https://dz/job_low.dzi' },
            ];
            _dzRenderInventory(1);
        `, ctx);
        try {
            await vm.runInContext('(async()=>{ await _dzDeleteSelected(); })()', ctx);
            const deletedPrefix = vm.runInContext('_dzDeletedPrefix', ctx);
            const invLen = vm.runInContext('window._dzInventory.length', ctx);
            const selectedIdx = vm.runInContext('window._dzSelectedIdx', ctx);
            const selectedJob = vm.runInContext('window._dzInventory[window._dzSelectedIdx].job_id', ctx);
            const status = ctx._elements['deepzoom-status'].textContent;
            const html = ctx._elements['deepzoom-inventory'].innerHTML || '';
            if (deletedPrefix !== 'deepzoom/job_mid/dz_mid/') { console.error('FATAL: deepzoom delete should target selected export prefix, got ' + deletedPrefix); process.exit(1); }
            if (invLen !== 2) { console.error('FATAL: deepzoom delete should remove one row, got len=' + invLen); process.exit(1); }
            if (selectedIdx !== 1) { console.error('FATAL: deepzoom delete should keep selection at neighbor idx=1, got ' + selectedIdx); process.exit(1); }
            if (selectedJob !== 'job_low') { console.error('FATAL: deepzoom delete should select row below deleted one, got ' + selectedJob); process.exit(1); }
            if (html.includes('job_mid')) { console.error('FATAL: deleted deepzoom row should disappear immediately'); process.exit(1); }
            if (!status.includes('Deleted dz_mid')) { console.error('FATAL: deepzoom delete should show success status, got ' + status); process.exit(1); }
            console.log('  deepzoom delete removes row in place and keeps adjacent selection: OK');
        } catch (e) {
            console.error('FATAL: deepzoom delete UX: ' + e.message);
            process.exit(1);
        }
    }

    // Step 11: Solve score UI + orchestration tests
    console.log('');
    console.log('--- Solve score ---');

    // 11a: render palette containers collapse built-ins into popup buttons
    {
        const rpContainer = ctx._elements['palette-circles-root-proximity'];
        const ssContainer = ctx._elements['palette-circles-solve-score'];
        const rpCount = rpContainer ? rpContainer.children.length : 0;
        const ssCount = ssContainer ? ssContainer.children.length : 0;
        if (rpCount !== 3) { console.error('FATAL: root-proximity palette row should collapse to 3 swatches, got ' + rpCount); process.exit(1); }
        if (ssCount !== 3) { console.error('FATAL: solve-score palette row should collapse to 3 swatches, got ' + ssCount); process.exit(1); }
        console.log('  render palette rows collapsed: OK (root=' + rpCount + ', solve=' + ssCount + ')');
    }

    // 11a2: built-in popup swatches exist in render rows; TRI and LONG exist in all rows
    {
        ctx._elements['palette-circles-palette-tab'] = ctx._mkEl();
        vm.runInContext("_renderPaletteRow('palette_tab')", ctx);
        ['palette-circles-root-proximity', 'palette-circles-solve-score'].forEach((id) => {
            const container = ctx._elements[id];
            const builtin = container.children.find(ch => ch.dataset && ch.dataset.palettePopup === 'builtin');
            if (!builtin) {
                console.error('FATAL: missing built-in popup swatch in ' + id);
                process.exit(1);
            }
            if (builtin.textContent !== 'PAL') {
                console.error('FATAL: built-in popup swatch text mismatch in ' + id + ': ' + builtin.textContent);
                process.exit(1);
            }
        });
        ['palette-circles-root-proximity', 'palette-circles-solve-score', 'palette-circles-palette-tab'].forEach((id) => {
            const container = ctx._elements[id];
            const tri = container.children.find(ch => String(ch.className || '').includes('pal-circle-tri'));
            const longSwatch = container.children.find(ch => String(ch.className || '').includes('pal-circle-long'));
            if (!tri) {
                console.error('FATAL: missing TRI swatch in ' + id);
                process.exit(1);
            }
            if (tri.textContent !== 'TRI') {
                console.error('FATAL: TRI swatch text mismatch in ' + id + ': ' + tri.textContent);
                process.exit(1);
            }
            if (!longSwatch) {
                console.error('FATAL: missing LONG swatch in ' + id);
                process.exit(1);
            }
            if (longSwatch.textContent !== 'LONG') {
                console.error('FATAL: LONG swatch text mismatch in ' + id + ': ' + longSwatch.textContent);
                process.exit(1);
            }
        });
        console.log('  built-in + TRI + LONG swatches: OK');
    }

    // 11a3: built-in popup opens and row selection updates remembered palette + active id
    {
        const solveContainer = ctx._elements['palette-circles-solve-score'];
        const builtin = solveContainer.children.find(ch => ch.dataset && ch.dataset.palettePopup === 'builtin');
        builtin.onclick();
        const overlay = ctx._elements['builtin-popup-overlay'];
        const title = ctx._elements['builtin-popup-title'].textContent || '';
        const rows = ctx._elements['builtin-popup-body'].children;
        if (overlay.style.display !== 'flex') { console.error('FATAL: built-in popup should be visible'); process.exit(1); }
        if (!title.includes('Solve score')) { console.error('FATAL: built-in popup title should mention Solve score, got ' + title); process.exit(1); }
        if (!rows.length) { console.error('FATAL: built-in popup should render rows'); process.exit(1); }
        const second = rows[1] || rows[0];
        second.onclick();
        const selectedPalette = vm.runInContext('renderSolveScorePalette', ctx);
        const remembered = vm.runInContext('renderSolveScoreBuiltinPalette', ctx);
        if (selectedPalette !== remembered) { console.error('FATAL: built-in popup should activate remembered palette, got ' + JSON.stringify({ selectedPalette, remembered })); process.exit(1); }
        console.log('  built-in popup selection: OK (' + remembered + ')');
    }

    // 11a4: LONG popup opens and row selection updates remembered palette + active id
    {
        const solveContainer = ctx._elements['palette-circles-solve-score'];
        const longSwatch = solveContainer.children.find(ch => String(ch.className || '').includes('pal-circle-long'));
        longSwatch.onclick();
        const overlay = ctx._elements['long-popup-overlay'];
        const title = ctx._elements['long-popup-title'].textContent || '';
        const rows = ctx._elements['long-popup-body'].children;
        if (overlay.style.display !== 'flex') { console.error('FATAL: LONG popup should be visible'); process.exit(1); }
        if (!title.includes('Solve score')) { console.error('FATAL: LONG popup title should mention Solve score, got ' + title); process.exit(1); }
        if (!rows.length) { console.error('FATAL: LONG popup should render rows'); process.exit(1); }
        const second = rows[1] || rows[0];
        second.onclick();
        const selectedPalette = vm.runInContext('renderSolveScorePalette', ctx);
        const remembered = vm.runInContext('renderSolveScoreLongName', ctx);
        if (!String(selectedPalette).startsWith('long_')) { console.error('FATAL: LONG selection should activate long palette, got ' + selectedPalette); process.exit(1); }
        if (!remembered) { console.error('FATAL: LONG selection should remember long name'); process.exit(1); }
        const refreshedContainer = ctx._elements['palette-circles-solve-score'];
        const activeSwatches = refreshedContainer.children.filter(ch => String(ch.className || '').includes(' active'));
        const activeLong = refreshedContainer.children.find(ch => String(ch.className || '').includes('pal-circle-long'));
        const activeBuiltin = refreshedContainer.children.find(ch => String(ch.className || '').includes('pal-circle-builtin') && !String(ch.className || '').includes('pal-circle-long'));
        if (!activeLong || !String(activeLong.className).includes(' active')) { console.error('FATAL: LONG swatch should be active after long selection'); process.exit(1); }
        if (activeBuiltin && String(activeBuiltin.className).includes(' active')) { console.error('FATAL: built-in PAL swatch should not stay active when LONG is selected'); process.exit(1); }
        if (activeSwatches.length !== 1) { console.error('FATAL: exactly one swatch should be active after LONG selection, got ' + activeSwatches.length); process.exit(1); }
        console.log('  LONG popup selection: OK (' + remembered + ')');
    }

    // 11a5: TRI popup opens and row selection updates remembered palette + active id
    {
        const solveContainer = ctx._elements['palette-circles-solve-score'];
        const tri = solveContainer.children.find(ch => String(ch.className || '').includes('pal-circle-tri'));
        tri.onclick({ altKey: false });
        const overlay = ctx._elements['tri-popup-overlay'];
        const title = ctx._elements['tri-popup-title'].textContent || '';
        const rows = ctx._elements['tri-popup-body'].children;
        if (overlay.style.display !== 'flex') { console.error('FATAL: TRI popup should be visible'); process.exit(1); }
        if (!title.includes('Solve score')) { console.error('FATAL: TRI popup title should mention Solve score, got ' + title); process.exit(1); }
        if (!rows.length) { console.error('FATAL: TRI popup should render rows'); process.exit(1); }
        const second = rows[1] || rows[0];
        second.onclick();
        const selectedPalette = vm.runInContext('renderSolveScorePalette', ctx);
        const remembered = vm.runInContext('renderSolveScoreTriName', ctx);
        if (!String(selectedPalette).startsWith('tri_')) { console.error('FATAL: TRI selection should activate tri palette, got ' + selectedPalette); process.exit(1); }
        if (!remembered) { console.error('FATAL: TRI selection should remember tri name'); process.exit(1); }
        console.log('  TRI popup selection: OK (' + remembered + ')');
    }

    // 11a6: right-click selects remembered TRI without reopening popup
    {
        vm.runInContext("renderRootProximityTriName = 'redgold'; renderRootProximityPalette = 'inferno';", ctx);
        vm.runInContext("_closeTriPalettePopup()", ctx);
        const rootContainer = ctx._elements['palette-circles-root-proximity'];
        const tri = rootContainer.children.find(ch => String(ch.className || '').includes('pal-circle-tri'));
        tri.oncontextmenu({ preventDefault() {} });
        const selected = vm.runInContext('renderRootProximityPalette', ctx);
        const popupOpen = vm.runInContext('_triPopupState.open', ctx);
        if (selected !== 'tri_redgold') { console.error('FATAL: right-click TRI should activate tri_redgold, got ' + selected); process.exit(1); }
        if (popupOpen) { console.error('FATAL: right-click TRI should not keep popup open'); process.exit(1); }
        console.log('  TRI right-click direct select: OK');
    }

    // 11a7: independent remembered TRI/LONG names survive per mode
    {
        vm.runInContext("_setTriPaletteForMode('proximity', 'redgold', false)", ctx);
        vm.runInContext("_setTriPaletteForMode('solve_score', 'greencopper', false)", ctx);
        vm.runInContext("_setTriPaletteForMode('palette_tab', 'retro_maroon_cream', false)", ctx);
        vm.runInContext("_setLongPaletteForMode('proximity', 'bauhaus_blue_yellow_13', false)", ctx);
        vm.runInContext("_setLongPaletteForMode('solve_score', 'metal_chrome_13', false)", ctx);
        vm.runInContext("_setLongPaletteForMode('palette_tab', 'marvel_spiderman_long', false)", ctx);
        const rp = vm.runInContext('renderRootProximityTriName', ctx);
        const sp = vm.runInContext('renderSolveScoreTriName', ctx);
        const pp = vm.runInContext('paletteTabTriName', ctx);
        const lrp = vm.runInContext('renderRootProximityLongName', ctx);
        const lsp = vm.runInContext('renderSolveScoreLongName', ctx);
        const lpp = vm.runInContext('paletteTabLongName', ctx);
        if (rp !== 'redgold' || sp !== 'greencopper' || pp !== 'retro_maroon_cream' || lrp !== 'bauhaus_blue_yellow_13' || lsp !== 'metal_chrome_13' || lpp !== 'marvel_spiderman_long') {
            console.error('FATAL: independent TRI/LONG memory broken: ' + JSON.stringify({ rp, sp, pp, lrp, lsp, lpp }));
            process.exit(1);
        }
        console.log('  TRI/LONG remembered names independent: OK');
    }

    // 11a8: filter matches aliases and graceful degradation does not break built-ins/LONG
    {
        vm.runInContext("_openTriPalettePopup('solve_score')", ctx);
        vm.runInContext("_applyTriPopupFilter('rg')", ctx);
        const rows = ctx._elements['tri-popup-body'].children;
        if (!rows.length) { console.error('FATAL: TRI filter should leave at least one row for alias rg'); process.exit(1); }
        const firstName = rows[0].children[0].children[0].children[0].textContent;
        if (firstName !== 'redgold') { console.error('FATAL: TRI alias filter should match redgold, got ' + firstName); process.exit(1); }
        const savedTri = ctx._triPaletteCatalog;
        const savedLong = ctx._longPaletteCatalog;
        ctx._triPaletteCatalog = null;
        ctx._longPaletteCatalog = null;
        vm.runInContext("buildPaletteCircles('palette-circles-root-proximity', 'proximity', () => renderRootProximityPalette)", ctx);
        const rootContainer = ctx._elements['palette-circles-root-proximity'];
        const tri = rootContainer.children.find(ch => String(ch.className || '').includes('pal-circle-tri'));
        const longSwatch = rootContainer.children.find(ch => String(ch.className || '').includes('pal-circle-long'));
        if (!String(tri.className).includes('disabled')) { console.error('FATAL: TRI should be disabled when catalog missing'); process.exit(1); }
        if (!String(longSwatch.className).includes('disabled')) { console.error('FATAL: LONG should be disabled when catalog missing'); process.exit(1); }
        rootContainer.children[0].onclick();
        const builtinRows = ctx._elements['builtin-popup-body'].children;
        if (!builtinRows.length) { console.error('FATAL: built-in popup should still work when TRI catalog missing'); process.exit(1); }
        builtinRows[0].onclick();
        const palette = vm.runInContext('renderRootProximityPalette', ctx);
        if (!palette || String(palette).startsWith('tri_')) { console.error('FATAL: built-in palette selection should still work when TRI catalog missing'); process.exit(1); }
        ctx._triPaletteCatalog = savedTri;
        ctx._longPaletteCatalog = savedLong;
        vm.runInContext("_renderAllPaletteRows()", ctx);
        console.log('  TRI/LONG filter + graceful degradation: OK');
    }

    // 11b: setPaletteForMode independence
    {
        vm.runInContext('renderRootProximityPalette = "inferno"; renderSolveScorePalette = "inferno";', ctx);
        vm.runInContext("setPaletteForMode('solve_score', 'viridis')", ctx);
        const rp = vm.runInContext('renderRootProximityPalette', ctx);
        const sp = vm.runInContext('renderSolveScorePalette', ctx);
        if (rp !== 'inferno') { console.error('FATAL: root palette changed to ' + rp); process.exit(1); }
        if (sp !== 'viridis') { console.error('FATAL: solve palette should be viridis, got ' + sp); process.exit(1); }
        console.log('  setPaletteForMode independence: OK');
    }

    // 11c: setColorMode activates solve_score
    {
        vm.runInContext("setColorMode('solve_score')", ctx);
        const mode = vm.runInContext('renderColorMode', ctx);
        if (mode !== 'solve_score') { console.error('FATAL: mode should be solve_score, got ' + mode); process.exit(1); }
        console.log('  setColorMode(solve_score): OK');
    }

    // 11d: _activeRenderPalette per mode
    {
        vm.runInContext("renderColorMode = 'proximity'; renderRootProximityPalette = 'magma';", ctx);
        const p1 = vm.runInContext('_activeRenderPalette()', ctx);
        if (p1 !== 'magma') { console.error('FATAL: proximity palette should be magma'); process.exit(1); }
        vm.runInContext("renderColorMode = 'solve_score'; renderSolveScorePalette = 'turbo';", ctx);
        const p2 = vm.runInContext('_activeRenderPalette()', ctx);
        if (p2 !== 'turbo') { console.error('FATAL: solve_score palette should be turbo'); process.exit(1); }
        vm.runInContext("renderColorMode = 'rainbow';", ctx);
        const p3 = vm.runInContext('_activeRenderPalette()', ctx);
        if (p3 !== null) { console.error('FATAL: rainbow palette should be null'); process.exit(1); }
        console.log('  _activeRenderPalette: OK');
    }

    // 11e: setSolveMetric changes state
    {
        vm.runInContext("setSolveMetric('crowding')", ctx);
        const m = vm.runInContext('renderSolveMetric', ctx);
        if (m !== 'crowding') { console.error('FATAL: metric should be crowding, got ' + m); process.exit(1); }
        const mode = vm.runInContext('renderColorMode', ctx);
        if (mode !== 'solve_score') { console.error('FATAL: setSolveMetric should activate solve_score mode'); process.exit(1); }
        console.log('  setSolveMetric: OK (crowding)');
    }

    // 11e2: setSolveMetric with new v2 metrics
    {
        vm.runInContext("setSolveMetric('clusteriness')", ctx);
        const m1 = vm.runInContext('renderSolveMetric', ctx);
        if (m1 !== 'clusteriness') { console.error('FATAL: metric should be clusteriness, got ' + m1); process.exit(1); }
        vm.runInContext("setSolveMetric('real_axis_proximity')", ctx);
        const m2 = vm.runInContext('renderSolveMetric', ctx);
        if (m2 !== 'real_axis_proximity') { console.error('FATAL: metric should be real_axis_proximity, got ' + m2); process.exit(1); }
        console.log('  setSolveMetric v2: OK (clusteriness, real_axis_proximity)');
    }

    // 11e2b: setSolveMetric / setPaletteMetric with new v3 metrics
    {
        vm.runInContext("setSolveMetric('centroid_dist')", ctx);
        const m1 = vm.runInContext('renderSolveMetric', ctx);
        if (m1 !== 'centroid_dist') { console.error('FATAL: metric should be centroid_dist, got ' + m1); process.exit(1); }
        vm.runInContext("setPaletteMetric('dist_unit_circle')", ctx);
        const m2 = vm.runInContext('paletteTabMetric', ctx);
        if (m2 !== 'dist_unit_circle') { console.error('FATAL: palette metric should be dist_unit_circle, got ' + m2); process.exit(1); }
        console.log('  setSolveMetric v3: OK (centroid_dist, dist_unit_circle)');
    }

    // 11e3: orchestrator dispatch with new metric carries it unchanged
    {
        vm.runInContext("renderColorMode = 'solve_score'; renderSolveMetric = 'nn_variation';", ctx);
        vm.runInContext(`
            var _nnOrchPayload = null;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch' && body.target === 'render_orchestrator') {
                    _nnOrchPayload = body.jobs[0];
                    return { fired: 1, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
                }
                return {};
            };
            refreshRenderArtifacts = async function() {};
        `, ctx);
        ctx._elements['render-results-dir'] = { ...ctx._mkEl(), value: 'test_nn' };
        ctx._elements['render-status'].textContent = '';
        ctx._elements['btn-raster-all'] = ctx._mkEl();
        vm.runInContext("_viewMode = 'square'; _rtChain = [];", ctx);
        try { await vm.runInContext('(async()=>{ await runRasterPipeline(); })()', ctx); } catch(e) {}
        const nnPayload = vm.runInContext('_nnOrchPayload', ctx);
        if (!nnPayload || nnPayload.params.solve_metric !== 'nn_variation') {
            console.error('FATAL: nn_variation dispatch failed: ' + JSON.stringify(nnPayload && nnPayload.params));
            process.exit(1);
        }
        console.log('  orchestrator dispatch v2: OK (solve_metric=nn_variation)');
        vm.runInContext("renderColorMode = 'rainbow';", ctx);
    }

    // 11e4: orchestrator dispatch with v3 metric carries it unchanged
    {
        vm.runInContext("renderColorMode = 'solve_score'; renderSolveMetric = 'asymmetry_re';", ctx);
        vm.runInContext(`
            var _v3OrchPayload = null;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch' && body.target === 'render_orchestrator') {
                    _v3OrchPayload = body.jobs[0];
                    return { fired: 1, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
                }
                return {};
            };
            refreshRenderArtifacts = async function() {};
        `, ctx);
        ctx._elements['render-results-dir'] = { ...ctx._mkEl(), value: 'test_v3' };
        ctx._elements['render-status'].textContent = '';
        ctx._elements['btn-raster-all'] = ctx._mkEl();
        vm.runInContext("_viewMode = 'square'; _rtChain = [];", ctx);
        try { await vm.runInContext('(async()=>{ await runRasterPipeline(); })()', ctx); } catch(e) {}
        const v3Payload = vm.runInContext('_v3OrchPayload', ctx);
        if (!v3Payload || v3Payload.params.solve_metric !== 'asymmetry_re') {
            console.error('FATAL: asymmetry_re dispatch failed: ' + JSON.stringify(v3Payload && v3Payload.params));
            process.exit(1);
        }
        console.log('  orchestrator dispatch v3: OK (solve_metric=asymmetry_re)');
        vm.runInContext("renderColorMode = 'rainbow';", ctx);
    }

    // 11f: orchestrator dispatch with solve_score contains metric
    {
        vm.runInContext("renderColorMode = 'solve_score'; renderSolveMetric = 'spread';", ctx);
        vm.runInContext(`
            var _lastOrchPayload = null;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch' && body.target === 'render_orchestrator') {
                    _lastOrchPayload = body.jobs[0];
                    return { fired: 1, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
                }
                return {};
            };
            refreshRenderArtifacts = async function() {};
        `, ctx);
        ctx._elements['render-results-dir'] = { ...ctx._mkEl(), value: 'test_ss' };
        ctx._elements['render-status'].textContent = '';
        ctx._elements['btn-raster-all'] = ctx._mkEl();
        vm.runInContext("_viewMode = 'square'; _rtChain = [];", ctx);
        try { await vm.runInContext('(async()=>{ await runRasterPipeline(); })()', ctx); } catch(e) {}
        const payload = vm.runInContext('_lastOrchPayload', ctx);
        if (!payload) { console.error('FATAL: no orchestrator dispatch'); process.exit(1); }
        if (payload.params.color_mode !== 'solve_score') { console.error('FATAL: color_mode should be solve_score'); process.exit(1); }
        if (payload.params.solve_metric !== 'spread') { console.error('FATAL: solve_metric should be spread, got ' + payload.params.solve_metric); process.exit(1); }
        console.log('  orchestrator dispatch: OK (color_mode=solve_score, solve_metric=spread)');
        vm.runInContext("renderColorMode = 'rainbow';", ctx);
    }

    // 11g: dead _ensureSolveProximityBins is gone
    {
        const exists = vm.runInContext('typeof _ensureSolveProximityBins', ctx);
        if (exists === 'function') { console.error('FATAL: dead _ensureSolveProximityBins should be deleted'); process.exit(1); }
        console.log('  dead _ensureSolveProximityBins removed: OK');
    }

    // 11h: solve-score quantile slider exists with default 0.1
    {
        const slider = ctx._elements['render-solve-score-quantile'];
        if (!slider) { console.error('FATAL: render-solve-score-quantile not found'); process.exit(1); }
        if (slider.value !== '0.1') { console.error('FATAL: default should be 0.1, got ' + slider.value); process.exit(1); }
        const valSpan = ctx._elements['render-solve-score-quantile-val'];
        if (!valSpan) { console.error('FATAL: render-solve-score-quantile-val not found'); process.exit(1); }
        console.log('  solve-score quantile slider: OK (default=0.1)');
    }

    {
        const slider = ctx._elements['render-solve-score-omega'];
        if (!slider) { console.error('FATAL: render-solve-score-omega not found'); process.exit(1); }
        if (slider.value !== '1') { console.error('FATAL: omega default should be 1, got ' + slider.value); process.exit(1); }
        const valSpan = ctx._elements['render-solve-score-omega-val'];
        if (!valSpan) { console.error('FATAL: render-solve-score-omega-val not found'); process.exit(1); }
        console.log('  solve-score omega slider: OK (default=1)');
    }

    // 11i: solveScoreQuantile in _renderCommonParams
    {
        // Set slider to 2.0 (= q=0.02)
        ctx._elements['render-solve-score-quantile'].value = '2.0';
        const cp = vm.runInContext('_renderCommonParams()', ctx);
        if (Math.abs(cp.solveScoreQuantile - 0.02) > 0.001) {
            console.error('FATAL: solveScoreQuantile should be 0.02, got ' + cp.solveScoreQuantile);
            process.exit(1);
        }
        ctx._elements['render-solve-score-omega'].value = '7';
        const cpOmega = vm.runInContext('_renderCommonParams()', ctx);
        if (Math.abs(cpOmega.solveScoreOmega - 7) > 0.001) {
            console.error('FATAL: solveScoreOmega should be 7, got ' + cpOmega.solveScoreOmega);
            process.exit(1);
        }
        console.log('  solveScoreQuantile in commonParams: OK (0.02)');
        ctx._elements['render-solve-score-quantile'].value = '0.1';
        ctx._elements['render-solve-score-omega'].value = '1';
    }

    // 11j: solve_score_quantile in orchestrator payload only when solve_score mode
    {
        vm.runInContext("renderColorMode = 'solve_score'; renderSolveMetric = 'proximity';", ctx);
        ctx._elements['render-solve-score-quantile'].value = '3.0';
        ctx._elements['render-solve-score-omega'].value = '6';
        vm.runInContext(`
            var _qOrchPayload = null;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch' && body.target === 'render_orchestrator') {
                    _qOrchPayload = body.jobs[0];
                    return { fired: 1, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
                }
                return {};
            };
            refreshRenderArtifacts = async function() {};
        `, ctx);
        ctx._elements['render-results-dir'] = { ...ctx._mkEl(), value: 'test_q' };
        ctx._elements['render-status'].textContent = '';
        ctx._elements['btn-raster-all'] = ctx._mkEl();
        vm.runInContext("_viewMode = 'square'; _rtChain = [];", ctx);
        try { await vm.runInContext('(async()=>{ await runRasterPipeline(); })()', ctx); } catch(e) {}
        const qp = vm.runInContext('_qOrchPayload', ctx);
        if (!qp || qp.params.solve_score_quantile === undefined) {
            console.error('FATAL: payload missing solve_score_quantile');
            process.exit(1);
        }
        if (Math.abs(qp.params.solve_score_quantile - 0.03) > 0.001) {
            console.error('FATAL: solve_score_quantile should be 0.03, got ' + qp.params.solve_score_quantile);
            process.exit(1);
        }
        if (Math.abs(qp.params.solve_score_omega - 6) > 0.001) {
            console.error('FATAL: solve_score_omega should be 6, got ' + qp.params.solve_score_omega);
            process.exit(1);
        }
        console.log('  solve_score_quantile in payload: OK (0.03)');
        // Now test rainbow mode — should NOT have solve_score_quantile
        vm.runInContext("renderColorMode = 'rainbow';", ctx);
        vm.runInContext('_qOrchPayload = null;', ctx);
        try { await vm.runInContext('(async()=>{ await runRasterPipeline(); })()', ctx); } catch(e) {}
        const rp = vm.runInContext('_qOrchPayload', ctx);
        if (rp && rp.params.solve_score_quantile !== undefined) {
            console.error('FATAL: rainbow mode should not have solve_score_quantile');
            process.exit(1);
        }
        console.log('  solve_score_quantile absent in rainbow: OK');
        vm.runInContext("renderColorMode = 'rainbow';", ctx);
        ctx._elements['render-solve-score-quantile'].value = '0.1';
        ctx._elements['render-solve-score-omega'].value = '1';
    }

    // 11k: viewport quantile and solve-score quantile are independent
    {
        ctx._elements['render-quantile'].value = '2.5';
        ctx._elements['render-solve-score-quantile'].value = '4.0';
        const cp2 = vm.runInContext('_renderCommonParams()', ctx);
        if (Math.abs(cp2.quantile - 0.025) > 0.001) {
            console.error('FATAL: viewport quantile should be 0.025'); process.exit(1);
        }
        if (Math.abs(cp2.solveScoreQuantile - 0.04) > 0.001) {
            console.error('FATAL: solveScoreQuantile should be 0.04'); process.exit(1);
        }
        console.log('  viewport q and solve-score q independent: OK');
        ctx._elements['render-quantile'].value = '0';
        ctx._elements['render-solve-score-quantile'].value = '0.1';
    }

    // Step 12: Orchestrator launch + observer tests (spec section 20.3)
    console.log('');
    console.log('--- Orchestrator launch + observer ---');

    // 12a: runRasterPipeline dispatches one render_orchestrator job
    {
        let orchDispatched = null;
        vm.runInContext(`
            renderColorMode = 'solve_score';
            var _orchDispatched = null;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch' && body.target === 'render_orchestrator') {
                    _orchDispatched = body.jobs[0];
                    return { fired: 1, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
                }
                return {};
            };
            refreshRenderArtifacts = async function() {};
        `, ctx);
        ctx._elements['render-results-dir'] = { ...ctx._mkEl(), value: 'test_orch' };
        ctx._elements['render-status'].textContent = '';
        ctx._elements['btn-raster-all'] = ctx._mkEl();
        ctx._elements['btn-render-generate'] = ctx._mkEl();
        vm.runInContext("_viewMode = 'square'; _rtChain = []; _renderGeneratePopupState.histInputMode = 'sectioned';", ctx);
        await vm.runInContext('(async()=>{ await runRasterPipeline(); })()', ctx);
        orchDispatched = vm.runInContext('_orchDispatched', ctx);
        if (!orchDispatched) { console.error('FATAL: runRasterPipeline did not dispatch orchestrator'); process.exit(1); }
        if (orchDispatched.params.raster_engine !== 'single') { console.error('FATAL: runRasterPipeline should request single raster engine, got ' + orchDispatched.params.raster_engine); process.exit(1); }
        if (orchDispatched.params.solve_score_hist_input_mode !== 'sectioned') { console.error('FATAL: runRasterPipeline should pass solve_score_hist_input_mode=sectioned, got ' + orchDispatched.params.solve_score_hist_input_mode); process.exit(1); }
        if (orchDispatched.mode !== 'color') { console.error('FATAL: mode should be color, got ' + orchDispatched.mode); process.exit(1); }
        console.log('  12a runRasterPipeline dispatches orchestrator: OK (mode=color, hist=sectioned)');
    }

    // 12a2: runRasterPipelineMT dispatches one render_orchestrator job with mt raster + solve-score thread counts
    {
        let orchDispatched = null;
        vm.runInContext(`
            renderColorMode = 'solve_score';
            var _orchDispatched = null;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch' && body.target === 'render_orchestrator') {
                    _orchDispatched = body.jobs[0];
                    return { fired: 1, errors: [] };
                }
                if (name === 'storage' && path === '/check-status') {
                    return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
                }
                return {};
            };
            refreshRenderArtifacts = async function() {};
        `, ctx);
        ctx._elements['btn-render-generate-mt'] = ctx._mkEl();
        await vm.runInContext('(async()=>{ await runRasterPipelineMT({ rasterThreads: 6, solveScoreThreads: 3, histInputMode: "sectioned", rasterInputMode: "sectioned", mergeWorkers: 14 }); })()', ctx);
        orchDispatched = vm.runInContext('_orchDispatched', ctx);
        if (!orchDispatched) { console.error('FATAL: runRasterPipelineMT did not dispatch orchestrator'); process.exit(1); }
        if (orchDispatched.params.raster_engine !== 'mt') { console.error('FATAL: runRasterPipelineMT should request mt raster engine, got ' + orchDispatched.params.raster_engine); process.exit(1); }
        if (orchDispatched.params.raster_mt_threads !== 6) { console.error('FATAL: runRasterPipelineMT should pass raster_mt_threads=6, got ' + orchDispatched.params.raster_mt_threads); process.exit(1); }
        if (orchDispatched.params.solve_score_threads !== 3) { console.error('FATAL: runRasterPipelineMT should pass solve_score_threads=3, got ' + orchDispatched.params.solve_score_threads); process.exit(1); }
        if (orchDispatched.params.solve_score_hist_input_mode !== 'sectioned') { console.error('FATAL: runRasterPipelineMT should pass solve_score_hist_input_mode=sectioned, got ' + orchDispatched.params.solve_score_hist_input_mode); process.exit(1); }
        if (orchDispatched.params.solve_score_merge_workers !== 14) { console.error('FATAL: runRasterPipelineMT should pass solve_score_merge_workers=14, got ' + orchDispatched.params.solve_score_merge_workers); process.exit(1); }
        if (orchDispatched.params.raster_input_mode !== 'sectioned') { console.error('FATAL: runRasterPipelineMT should pass raster_input_mode=sectioned, got ' + orchDispatched.params.raster_input_mode); process.exit(1); }
        if (orchDispatched.mode !== 'color') { console.error('FATAL: mode should be color, got ' + orchDispatched.mode); process.exit(1); }
        console.log('  12a2 runRasterPipelineMT dispatches orchestrator: OK (mode=color, raster_engine=mt, solve=3, hist=sectioned, merge=14, raster_input=sectioned, raster=6)');
    }

    // 12b: runBilevelPipeline dispatches one render_orchestrator job
    {
        vm.runInContext('_orchDispatched = null;', ctx);
        ctx._elements['btn-bilevel'] = ctx._mkEl();
        await vm.runInContext('(async()=>{ await runBilevelPipeline(); })()', ctx);
        const orch = vm.runInContext('_orchDispatched', ctx);
        if (!orch) { console.error('FATAL: runBilevelPipeline did not dispatch orchestrator'); process.exit(1); }
        if (orch.mode !== 'bilevel') { console.error('FATAL: mode should be bilevel, got ' + orch.mode); process.exit(1); }
        console.log('  12b runBilevelPipeline dispatches orchestrator: OK (mode=bilevel)');
    }

    // 12c: runCoeffBilevelPipeline dispatches one render_orchestrator job
    {
        vm.runInContext('_orchDispatched = null;', ctx);
        ctx._elements['btn-coeffs'] = ctx._mkEl();
        await vm.runInContext('(async()=>{ await runCoeffBilevelPipeline(); })()', ctx);
        const orch = vm.runInContext('_orchDispatched', ctx);
        if (!orch) { console.error('FATAL: runCoeffBilevelPipeline did not dispatch orchestrator'); process.exit(1); }
        if (orch.mode !== 'coeff_bilevel') { console.error('FATAL: mode should be coeff_bilevel, got ' + orch.mode); process.exit(1); }
        console.log('  12c runCoeffBilevelPipeline dispatches orchestrator: OK (mode=coeff_bilevel)');
    }

    // 12d: browser no longer dispatches raster/finalize/encode/stitch directly
    {
        // Search the source for direct dispatch of worker phases in the new launch functions
        const launchCode = [
            vm.runInContext('runRasterPipeline.toString()', ctx),
            vm.runInContext('runRasterPipelineMT.toString()', ctx),
            vm.runInContext('runBilevelPipeline.toString()', ctx),
            vm.runInContext('runCoeffBilevelPipeline.toString()', ctx),
        ].join('\n');
        const forbidden = ["target: 'raster'", "target: 'finalize'", "target: 'encode'", "target: 'bilevel_stitch'"];
        const found = forbidden.filter(f => launchCode.includes(f));
        if (found.length) {
            console.error('FATAL: launch functions still dispatch worker phases: ' + found.join(', '));
            process.exit(1);
        }
        console.log('  12d no direct worker dispatch in launch functions: OK');
    }

    // 12e: active run record written to localStorage
    {
        const stored = ctx.localStorage.getItem('polypaint_active_render_run');
        // After the runs above, localStorage should have been written (then cleared on 'done')
        // The mock returns done immediately, so it gets cleared. Check that _saveActiveRun exists and works.
        vm.runInContext("_saveActiveRun({job_id:'j',mode:'color',run_id:'r1',task_id:'t1'})", ctx);
        const val = ctx.localStorage.getItem('polypaint_active_render_run');
        if (!val) { console.error('FATAL: _saveActiveRun did not write to localStorage'); process.exit(1); }
        const parsed = JSON.parse(val);
        if (parsed.run_id !== 'r1') { console.error('FATAL: wrong run_id in localStorage'); process.exit(1); }
        console.log('  12e active run record in localStorage: OK');
    }

    // 12f: _clearActiveRun removes from localStorage
    {
        vm.runInContext('_clearActiveRun()', ctx);
        const val = ctx.localStorage.getItem('polypaint_active_render_run');
        if (val) { console.error('FATAL: _clearActiveRun did not remove from localStorage'); process.exit(1); }
        console.log('  12f _clearActiveRun removes localStorage: OK');
    }

    // 12g: observer secondary poll for subtask counts
    {
        // Set up active run and mock lambdaPost that returns subtask_prefix + expected
        vm.runInContext("_activeRenderRun = {job_id:'j', mode:'color', run_id:'r_sub', task_id:'render_run_color_r_sub'};", ctx);
        let secondaryPollCalled = false;
        vm.runInContext(`
            var _subPollCalled = false;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/check-status') {
                    if (body.task_prefix === 'render_run_color_r_sub') {
                        // Top-level row: in-progress with subtask info
                        return {
                            errors: 0, done: 0, complete: false,
                            results: [{
                                phase: 'raster', phase_label: 'Raster',
                                subtask_prefix: 'render_r_sub_raster_',
                                expected: 10
                            }]
                        };
                    }
                    if (body.task_prefix === 'render_r_sub_raster_') {
                        // Secondary poll — subtask counts
                        _subPollCalled = true;
                        return { errors: 0, done: 7, expected: 10, complete: false };
                    }
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await _pollActiveRenderRun(); })()', ctx); } catch(e) {}
        const subCalled = vm.runInContext('_subPollCalled', ctx);
        if (!subCalled) { console.error('FATAL: secondary subtask poll not called'); process.exit(1); }
        const statusText = ctx._elements['render-status'].textContent;
        if (!statusText.includes('7/10')) { console.error('FATAL: status should show 7/10, got: ' + statusText); process.exit(1); }
        console.log('  12g observer secondary subtask poll: OK (' + statusText + ')');
        vm.runInContext('_activeRenderRun = null;', ctx);
    }

    // 12h: stale top-level + fresh worker heartbeat => NOT stalled
    {
        vm.runInContext("_activeRenderRun = {job_id:'j', mode:'color', run_id:'r_fresh', task_id:'render_run_color_r_fresh', started_at_ms: Date.now() - 600000};", ctx);
        vm.runInContext("_lastWarnState = null;", ctx);
        vm.runInContext(`
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/check-status') {
                    if (body.task_prefix === 'render_run_color_r_fresh') {
                        return {
                            errors: 0, done: 0, complete: false,
                            latest_update_ms: Date.now() - 600000,
                            results: [{
                                phase: 'raster', phase_label: 'Raster',
                                subtask_prefix: 'render_r_fresh_raster_',
                                expected: 10, updated_at_ms: Date.now() - 600000
                            }]
                        };
                    }
                    if (body.task_prefix === 'render_r_fresh_raster_') {
                        return { errors: 0, done: 3, expected: 10, complete: false, latest_update_ms: Date.now() - 5000 };
                    }
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await _pollActiveRenderRun(); })()', ctx); } catch(e) {}
        const st = ctx._elements['render-status'].textContent;
        if (st.includes('stall') || st.includes('Stall')) { console.error('FATAL: fresh workers should not show stalled: ' + st); process.exit(1); }
        if (!st.includes('3/10')) { console.error('FATAL: should show 3/10: ' + st); process.exit(1); }
        console.log('  12h stale top + fresh workers => not stalled: OK');
        vm.runInContext('_activeRenderRun = null;', ctx);
    }

    // 12i: stale workers (>5 min, <15 min) => warning, does NOT clear active run
    {
        vm.runInContext("_activeRenderRun = {job_id:'j', mode:'color', run_id:'r_warn', task_id:'render_run_color_r_warn', started_at_ms: Date.now() - 900000};", ctx);
        vm.runInContext("_lastWarnState = null;", ctx);
        vm.runInContext(`
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/check-status') {
                    if (body.task_prefix === 'render_run_color_r_warn') {
                        return {
                            errors: 0, done: 0, complete: false,
                            latest_update_ms: Date.now() - 400000,
                            results: [{
                                phase: 'raster', phase_label: 'Raster',
                                subtask_prefix: 'render_r_warn_raster_',
                                expected: 10, updated_at_ms: Date.now() - 400000
                            }]
                        };
                    }
                    if (body.task_prefix === 'render_r_warn_raster_') {
                        return { errors: 0, done: 0, expected: 10, complete: false, latest_update_ms: Date.now() - 400000 };
                    }
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await _pollActiveRenderRun(); })()', ctx); } catch(e) {}
        const st = ctx._elements['render-status'].textContent;
        if (!st.includes('5+ min')) { console.error('FATAL: should show 5+ min warning: ' + st); process.exit(1); }
        const runStillActive = vm.runInContext('_activeRenderRun !== null', ctx);
        if (!runStillActive) { console.error('FATAL: warning should NOT clear active run'); process.exit(1); }
        console.log('  12i stale workers 5+ min => warning, run kept: OK');
        vm.runInContext('_activeRenderRun = null;', ctx);
    }

    // 12j: very stale workers (>15 min) => hard stall, does NOT clear active run
    {
        vm.runInContext("_activeRenderRun = {job_id:'j', mode:'color', run_id:'r_hard', task_id:'render_run_color_r_hard', started_at_ms: Date.now() - 1200000};", ctx);
        vm.runInContext("_lastWarnState = null;", ctx);
        vm.runInContext(`
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/check-status') {
                    if (body.task_prefix === 'render_run_color_r_hard') {
                        return {
                            errors: 0, done: 0, complete: false,
                            latest_update_ms: Date.now() - 1000000,
                            results: [{
                                phase: 'raster', phase_label: 'Raster',
                                subtask_prefix: 'render_r_hard_raster_',
                                expected: 10, updated_at_ms: Date.now() - 1000000
                            }]
                        };
                    }
                    if (body.task_prefix === 'render_r_hard_raster_') {
                        return { errors: 0, done: 0, expected: 10, complete: false, latest_update_ms: Date.now() - 1000000 };
                    }
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await _pollActiveRenderRun(); })()', ctx); } catch(e) {}
        const st = ctx._elements['render-status'].textContent;
        if (!st.includes('15+ min')) { console.error('FATAL: should show 15+ min hard stall: ' + st); process.exit(1); }
        const runStillActive = vm.runInContext('_activeRenderRun !== null', ctx);
        if (!runStillActive) { console.error('FATAL: hard stall should NOT clear active run'); process.exit(1); }
        console.log('  12j stale workers 15+ min => hard stall, run kept: OK');
        vm.runInContext('_activeRenderRun = null;', ctx);
    }

    // 12k: autolevel completion logs debug extents
    {
        ctx._elements['render-log'] = ctx._mkEl();
        vm.runInContext("_activeRenderRun = {job_id:'j', mode:'autolevels', run_id:'r_auto_dbg', task_id:'autolevels_r_auto_dbg', started_at_ms: Date.now() - 1000};", ctx);
        vm.runInContext(`
            refreshRenderArtifacts = async function() {};
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/check-status' && body.task_prefix === 'autolevels_r_auto_dbg') {
                    return {
                        errors: 0,
                        done: 1,
                        complete: true,
                        results: [{
                            phase: 'done',
                            phase_label: 'Done',
                            family: 'color',
                            artifact_id: 'autolevels_dbg',
                            postprocess_kind: 'autolevels',
                            autolevel_debug: {
                                background_color: '000000',
                                background_threshold: 11,
                                exclude_background: true,
                                r_min_bin: 16, r_max_bin: 210,
                                g_min_bin: 18, g_max_bin: 220,
                                b_min_bin: 20, b_max_bin: 240,
                                black_bin: 12, white_bin: 220,
                                black: 0.047,
                                white: 0.863,
                                gamma: 1.1,
                                final_stretch: true,
                                final_lo_bin: 4, final_hi_bin: 250,
                                included_pixels: 1000,
                                excluded_pixels: 2000
                            }
                        }]
                    };
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await _pollActiveRenderRun(); })()', ctx); } catch(e) {}
        const logText = ctx._elements['render-log'].textContent || '';
        if (!logText.includes('autolevels:')) { console.error('FATAL: autolevel completion should log debug line'); process.exit(1); }
        if (!logText.includes('R[16..210]') || !logText.includes('G[18..220]') || !logText.includes('B[20..240]')) { console.error('FATAL: autolevel debug log should include RGB extents, got ' + logText); process.exit(1); }
        if (!logText.includes('pooled[12..220]') || !logText.includes('tol/ch=11') || !logText.includes('levels=0.047..0.863')) { console.error('FATAL: autolevel debug log should include pooled extents, tolerance, and levels, got ' + logText); process.exit(1); }
        if (!logText.includes('Render complete: autolevels_dbg (')) { console.error('FATAL: autolevel completion should include elapsed seconds, got ' + logText); process.exit(1); }
        console.log('  12k autolevel completion logs debug extents: OK');
        vm.runInContext('_activeRenderRun = null;', ctx);
    }

    // 12l: solve-score progress log includes wall + total timing
    {
        ctx._elements['render-log'] = ctx._mkEl();
        ctx._elements['render-status'] = ctx._mkEl();
        vm.runInContext("_activeRenderRun = {job_id:'j', mode:'color', run_id:'r_hist', task_id:'render_run_color_r_hist', started_at_ms: Date.now() - 4000}; _renderPhaseTracker = null; _lastLoggedPhase = null;", ctx);
        vm.runInContext(`
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/check-status' && body.task_prefix === 'render_run_color_r_hist') {
                    return {
                        errors: 0,
                        done: 0,
                        complete: false,
                        latest_update_ms: Date.now() - 1500,
                        results: [{
                            phase: 'solve_score_hist',
                            phase_label: 'Solve score: hist',
                            expected: 100,
                            subtask_prefix: 'render_r_hist_solve_score_hist_',
                            updated_at_ms: Date.now() - 5000
                        }]
                    };
                }
                if (name === 'storage' && path === '/check-status' && body.task_prefix === 'render_r_hist_solve_score_hist_') {
                    return {
                        errors: 0,
                        done: 24,
                        expected: 100,
                        complete: false,
                        latest_update_ms: Date.now() - 1500,
                        results: [
                            { dl_ms: 1200, compute_ms: 3400, input_mode: 'sectioned' },
                            { dl_ms: 800, compute_ms: 2600, input_mode: 'sectioned' }
                        ]
                    };
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await _pollActiveRenderRun(); })()', ctx); } catch(e) {}
        const logText = ctx._elements['render-log'].textContent || '';
        if (!logText.includes('Solve score: hist 24/100')) { console.error('FATAL: solve-score hist progress should log count, got ' + logText); process.exit(1); }
        if (!logText.includes('wall=') || !logText.includes('total=dl')) { console.error('FATAL: solve-score hist progress should log wall + total timing, got ' + logText); process.exit(1); }
        if (!logText.includes('input=sectioned')) { console.error('FATAL: solve-score hist progress should log input mode, got ' + logText); process.exit(1); }
        console.log('  12l solve-score progress logs wall + total timing: OK');
        vm.runInContext('_activeRenderRun = null; _renderPhaseTracker = null;', ctx);
    }

    // 12m: color completion logs raster perf summary with engine + threads in seconds
    {
        ctx._elements['render-log'] = ctx._mkEl();
        vm.runInContext("_activeRenderRun = {job_id:'j', mode:'color', run_id:'r_perf', task_id:'render_run_color_r_perf', started_at_ms: Date.now() - 1000}; _renderPhaseTracker = { phase:'raster', phase_label:'Raster', started_at_ms: Date.now() - 2500, prefix:'render_r_perf_raster_', expected:2 };", ctx);
        vm.runInContext(`
            refreshRenderArtifacts = async function() {};
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/check-status' && body.task_prefix === 'render_run_color_r_perf') {
                    return {
                        errors: 0,
                        done: 1,
                        complete: true,
                        results: [{
                            phase: 'done',
                            phase_label: 'Done',
                            family: 'color',
                            artifact_id: 'color_perf'
                        }]
                    };
                }
                if (name === 'storage' && path === '/check-status' && body.task_prefix === 'render_r_perf_raster_') {
                    return {
                        errors: 0,
                        done: 2,
                        complete: false,
                        results: [
                            { engine: 'mt', threads: 2, input_mode: 'sectioned', download_us: 1200, native_us: 3400, upload_us: 800, roots_plotted: 50, roots_clipped: 2, tiles_uploaded: 3, pixel_bin_tiles_uploaded: 3 },
                            { engine: 'mt', threads: 2, input_mode: 'sectioned', download_us: 1400, native_us: 3600, upload_us: 900, roots_plotted: 60, roots_clipped: 1, tiles_uploaded: 4, pixel_bin_tiles_uploaded: 4 }
                        ]
                    };
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await _pollActiveRenderRun(); })()', ctx); } catch(e) {}
        const logText = ctx._elements['render-log'].textContent || '';
        if (!logText.includes('Raster performance: engine=mt threads=2 input=sectioned chunks=2')) { console.error('FATAL: color completion should log raster perf engine/threads/input/chunks, got ' + logText); process.exit(1); }
        if (!logText.includes('wall=')) { console.error('FATAL: raster perf log should include wall time in seconds, got ' + logText); process.exit(1); }
        if (!logText.includes('Download time: 0.0s') || !logText.includes('Native raster time: 0.0s') || !logText.includes('Upload time: 0.0s')) { console.error('FATAL: raster perf log should spell out timing labels, got ' + logText); process.exit(1); }
        if (!logText.includes('Emitted root hits: 110') || !logText.includes('Clipped roots: 3') || !logText.includes('Tile files uploaded: 7') || !logText.includes('Pixel-bin files uploaded: 7')) { console.error('FATAL: raster perf log should aggregate worker metrics with full labels, got ' + logText); process.exit(1); }
        if (!logText.includes('Render complete: color_perf (')) { console.error('FATAL: render completion log should include elapsed seconds, got ' + logText); process.exit(1); }
        console.log('  12m color completion logs raster perf summary: OK');
        vm.runInContext('_activeRenderRun = null; _renderPhaseTracker = null;', ctx);
    }

    // 12n: render wall timing uses server timestamps, not browser Date.now()
    {
        ctx._elements['render-log'] = ctx._mkEl();
        ctx._elements['render-status'] = ctx._mkEl();
        vm.runInContext(`
            if (!window._origDateNow) window._origDateNow = Date.now;
            Date.now = function() { return 900000; };
            _activeRenderRun = {
                job_id:'j',
                mode:'color',
                run_id:'r_server_time',
                task_id:'render_run_color_r_server_time',
                started_at_ms: 1000,
                server_started_at_ms: 100000
            };
            _renderPhaseTracker = {
                phase:'raster',
                phase_label:'Raster',
                started_at_ms: 110000,
                last_server_update_ms: 111000,
                prefix:'render_r_server_time_raster_',
                expected:2
            };
        `, ctx);
        vm.runInContext(`
            refreshRenderArtifacts = async function() {};
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/check-status' && body.task_prefix === 'render_run_color_r_server_time') {
                    return {
                        errors: 0,
                        done: 1,
                        complete: true,
                        results: [{
                            phase: 'done',
                            phase_label: 'Done',
                            family: 'color',
                            artifact_id: 'color_server_time',
                            run_started_at_ms: 100000,
                            updated_at_ms: 130000
                        }]
                    };
                }
                if (name === 'storage' && path === '/check-status' && body.task_prefix === 'render_r_server_time_raster_') {
                    return {
                        errors: 0,
                        done: 2,
                        latest_done_ms: 120000,
                        latest_update_ms: 120000,
                        results: [
                            { engine: 'mt', threads: 2, input_mode: 'sectioned', download_us: 1200, native_us: 3400, upload_us: 800, roots_plotted: 50, roots_clipped: 2, tiles_uploaded: 3, pixel_bin_tiles_uploaded: 3 },
                            { engine: 'mt', threads: 2, input_mode: 'sectioned', download_us: 1400, native_us: 3600, upload_us: 900, roots_plotted: 60, roots_clipped: 1, tiles_uploaded: 4, pixel_bin_tiles_uploaded: 4 }
                        ]
                    };
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await _pollActiveRenderRun(); })()', ctx); } catch(e) {}
        const logText = ctx._elements['render-log'].textContent || '';
        if (!logText.includes('Raster complete · wall=10.0s')) { console.error('FATAL: raster phase completion wall time should use server timestamps, got ' + logText); process.exit(1); }
        if (!logText.includes('Render complete: color_server_time (30.0s)')) { console.error('FATAL: render completion wall time should use server timestamps, got ' + logText); process.exit(1); }
        console.log('  12n render wall timing uses server timestamps: OK');
        vm.runInContext('Date.now = window._origDateNow; _activeRenderRun = null; _renderPhaseTracker = null;', ctx);
    }

    // Step 13: Solve histogram debug button
    console.log('');
    console.log('--- Solve histogram debug ---');

    // 13a: button exists
    {
        const btn = ctx._elements['btn-solve-histogram'];
        if (!btn) { console.error('FATAL: btn-solve-histogram not found'); process.exit(1); }
        console.log('  13a histogram button exists: OK');
    }

    // 13b: refuses when not in solve_score mode
    {
        vm.runInContext("renderColorMode = 'rainbow'; _activeRenderRun = null;", ctx);
        ctx._elements['render-results-dir'] = { ...ctx._mkEl(), value: 'test_job' };
        // Capture log output by wrapping log
        vm.runInContext(`
            var _histLogMsg = '';
            var _origLog = log;
            log = function(msg, cls, target) { _histLogMsg += msg + ' '; _origLog(msg, cls, target); };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await runSolveScoreHistogramDebug(); })()', ctx); } catch(e) {}
        const msg = vm.runInContext('_histLogMsg', ctx);
        if (!msg.includes('Solve score mode')) { console.error('FATAL: should reject non-solve mode: ' + msg); process.exit(1); }
        // Restore log
        vm.runInContext('log = _origLog;', ctx);
        console.log('  13b refuses in non-solve mode: OK');
    }

    // 13c: refuses when render is active
    {
        vm.runInContext("renderColorMode = 'solve_score'; _activeRenderRun = {job_id:'j'};", ctx);
        vm.runInContext(`
            _histLogMsg = '';
            var _origLog2 = log;
            log = function(msg, cls, target) { _histLogMsg += msg + ' '; _origLog2(msg, cls, target); };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await runSolveScoreHistogramDebug(); })()', ctx); } catch(e) {}
        const msg = vm.runInContext('_histLogMsg', ctx);
        if (!msg.includes('render in progress')) { console.error('FATAL: should refuse during active render: ' + msg); process.exit(1); }
        vm.runInContext('log = _origLog2;', ctx);
        console.log('  13c refuses during active render: OK');
        vm.runInContext('_activeRenderRun = null;', ctx);
    }

    // 13d: sends correct payload and does not dispatch orchestrator
    {
        vm.runInContext("renderColorMode = 'solve_score'; renderSolveMetric = 'crowding'; _activeRenderRun = null;", ctx);
        vm.runInContext("_lastCalcHasLores = true;", ctx);
        ctx._elements['render-solve-score-quantile'].value = '2.0';
        ctx._elements['render-results-dir'] = { ...ctx._mkEl(), value: 'test_hist_job' };
        ctx._elements['render-status'] = ctx._mkEl();
        ctx._elements['render-log'] = ctx._mkEl();
        ctx._elements['btn-solve-histogram'] = ctx._mkEl();
        vm.runInContext(`
            var _histTarget = null;
            var _histBody = null;
            var _histDispatchCalled = false;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch') { _histDispatchCalled = true; return {}; }
                if (name === 'solve_proximity') {
                    _histTarget = name;
                    _histBody = body;
                    return {
                        mode: 'summary', metric: 'crowding', n_solves: 50, degree: 5,
                        min_score: -1, max_score: 2, mean_score: 0.5, stddev_score: 0.3,
                        q05: -0.5, q10: -0.3, q25: 0.1, q50: 0.5, q75: 0.9, q90: 1.2, q95: 1.5,
                        omega: 8,
                        clip_quantile: 0.02, clip_lo: -0.5, clip_hi: 1.5, full_range: 3, clip_range: 2,
                        clip_below_count: 2, clip_inrange_count: 46, clip_above_count: 2,
                        clip_below_frac: 0.04, clip_inrange_frac: 0.92, clip_above_frac: 0.04,
                        clip_fallback: false, clip_fallback_reason: null,
                        metric_validity_policy: 'finite_only_min_roots',
                        metric_min_finite_roots: 2,
                        total_root_slots: 250, finite_root_count: 210,
                        fully_finite_solve_count: 40, partial_finite_solve_count: 8, zero_finite_solve_count: 2,
                        usable_solve_count: 45, forced_zero_score_count: 5,
                        finite_root_frac: 0.84, fully_finite_solve_frac: 0.8, partial_finite_solve_frac: 0.16, zero_finite_solve_frac: 0.04, usable_solve_frac: 0.9,
                        mean_finite_roots_per_solve: 4.2, min_finite_roots_per_solve: 0, max_finite_roots_per_solve: 5,
                        intermediate_hist_bins: 100, final_bins: 10,
                        cuts_norm: [0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9],
                        cuts_score: [-0.3,-0.1,0.1,0.3,0.5,0.7,0.9,1.1,1.3],
                        final_bin_counts: [5,5,5,5,4,5,5,4,4,4],
                        final_bin_fracs: [0.109,0.109,0.109,0.109,0.087,0.109,0.109,0.087,0.087,0.087],
                        min_score_count: 1, max_score_count: 3, clip_lo_count: 0, clip_hi_count: 3,
                        n_unique_scores: 48,
                        dl_ms: 5, compute_ms: 3,
                    };
                }
                if (name === 'storage' && path === '/detail') {
                    return { calc: { degree: 5, lores: { bin_key: 'renders/test_hist_job/lores.bin' } } };
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await runSolveScoreHistogramDebug(); })()', ctx); } catch(e) {}
        const target = vm.runInContext('_histTarget', ctx);
        const body = vm.runInContext('_histBody', ctx);
        const dispatchCalled = vm.runInContext('_histDispatchCalled', ctx);
        if (target !== 'solve_proximity') { console.error('FATAL: should call solve_proximity, got ' + target); process.exit(1); }
        if (body.phase !== 'summary') { console.error('FATAL: phase should be summary'); process.exit(1); }
        if (body.metric !== 'crowding') { console.error('FATAL: metric should be crowding'); process.exit(1); }
        if (Math.abs(body.solve_score_quantile - 0.02) > 0.001) { console.error('FATAL: q should be 0.02'); process.exit(1); }
        if (Math.abs(body.solve_score_omega - 1) > 0.001) { console.error('FATAL: omega should be 1 by default'); process.exit(1); }
        if (body.degree !== 5) { console.error('FATAL: degree should be 5'); process.exit(1); }
        if (dispatchCalled) { console.error('FATAL: histogram must not call dispatch'); process.exit(1); }
        // Verify _activeRenderRun was not set
        const runSet = vm.runInContext('_activeRenderRun !== null', ctx);
        if (runSet) { console.error('FATAL: histogram must not set _activeRenderRun'); process.exit(1); }
        // Verify log output contains the 10-bin table, not 32-bin full range
        const logHtml = ctx._elements['render-log'].innerHTML || '';
        const logText = ctx._elements['render-log'].textContent || '';
        if (!logText.includes('final color bins (10')) { console.error('FATAL: log should show final color bins (10)'); process.exit(1); }
        if (!logText.includes('b0')) { console.error('FATAL: log should show b0 row'); process.exit(1); }
        if (!logText.includes('b9')) { console.error('FATAL: log should show b9 row'); process.exit(1); }
        if (!logText.includes('w=8')) { console.error('FATAL: log should show omega'); process.exit(1); }
        if (logText.includes('32 bins')) { console.error('FATAL: log must not show 32 bins'); process.exit(1); }
        if (logText.includes('full range')) { console.error('FATAL: log must not show full range'); process.exit(1); }
        if (!logText.includes('extremes:')) { console.error('FATAL: log should show outlier extremes'); process.exit(1); }
        if (!logText.includes('max_count=3')) { console.error('FATAL: log should show max_count=3'); process.exit(1); }
        if (!logText.includes('finite diagnostics: full=40')) { console.error('FATAL: log should show finite diagnostics, got ' + logText); process.exit(1); }
        if (!logText.includes('scoring policy: finite_only_min_roots (min finite roots=2)  usable=45 (90.0%)  forced_zero=5')) { console.error('FATAL: log should show finite-only policy, got ' + logText); process.exit(1); }
        console.log('  13d correct payload, no dispatch, no activeRun: OK');
        console.log('  13e log shows 10-bin table, no 32-bin, has extremes + finite diagnostics: OK');
    }

    // Step 14: Render palette family generation
    console.log('');
    console.log('--- Render palette family ---');

    // 14a: generate rejects non-solve mode
    {
        vm.runInContext("_renderActiveFamily = 'palette'; renderColorMode = 'rainbow'; _activeRenderRun = null;", ctx);
        ctx._elements['render-results-dir'] = { ...ctx._mkEl(), value: 'test_pal_job' };
        vm.runInContext("var _palLogMsg = ''; var _palOrigLog = log; log = function(m,c,t){ _palLogMsg += m + ' '; _palOrigLog(m,c,t); };", ctx);
        try { await vm.runInContext('(async()=>{ await runRenderPaletteArtifact(); })()', ctx); } catch(e) {}
        const msg = vm.runInContext('_palLogMsg', ctx);
        if (!msg.includes('Solve score mode')) { console.error('FATAL: palette should reject non-solve mode: ' + msg); process.exit(1); }
        vm.runInContext('log = _palOrigLog;', ctx);
        console.log('  14a refuses in non-solve mode: OK');
    }

    // 14b: dispatches palette orchestrator and uses render solve-score settings
    {
        vm.runInContext("_renderActiveFamily = 'palette'; renderColorMode = 'solve_score'; renderSolveMetric = 'area'; _activeRenderRun = null;", ctx);
        vm.runInContext("renderSolveScorePalette = 'turbo';", ctx);
        ctx._elements['render-solve-score-quantile'].value = '1.0';
        ctx._elements['render-solve-score-omega'].value = '9';
        ctx._elements['render-results-dir'] = { ...ctx._mkEl(), value: 'pal_test' };
        vm.runInContext(`
            var _palTarget = null; var _palBody = null; var _palObserverStarted = false;
            startActivePaletteObserver = function() { _palObserverStarted = true; };
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch') {
                    _palTarget = name;
                    _palBody = body;
                    return { fired: 1, total: 1 };
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await runRenderPaletteArtifact(); })()', ctx); } catch(e) {}
        const target = vm.runInContext('_palTarget', ctx);
        const body = vm.runInContext('_palBody', ctx);
        const observerStarted = vm.runInContext('_palObserverStarted', ctx);
        if (target !== 'dispatch') { console.error('FATAL: render palette should dispatch orchestrator, got ' + target); process.exit(1); }
        if (body.target !== 'palette_orchestrator') { console.error('FATAL: should dispatch palette_orchestrator'); process.exit(1); }
        const job = body.jobs[0];
        if (job.params.metric !== 'area') { console.error('FATAL: metric should be area'); process.exit(1); }
        if (job.params.palette !== 'turbo') { console.error('FATAL: palette should be turbo'); process.exit(1); }
        if (Math.abs(job.params.solve_score_quantile - 0.01) > 0.001) { console.error('FATAL: q should be 0.01'); process.exit(1); }
        if (Math.abs(job.params.solve_score_omega - 9) > 0.001) { console.error('FATAL: omega should be 9'); process.exit(1); }
        if (!observerStarted) { console.error('FATAL: render palette should start palette observer'); process.exit(1); }
        const palRun = vm.runInContext('_activePaletteRun', ctx);
        if (!palRun || palRun.job_id !== 'pal_test') { console.error('FATAL: render palette should set _activePaletteRun'); process.exit(1); }
        const runSet = vm.runInContext('_activeRenderRun !== null', ctx);
        if (runSet) { console.error('FATAL: palette must not set _activeRenderRun'); process.exit(1); }
        console.log('  14b dispatches palette orchestrator from render settings: OK');
    }

    // Step 15: Palette tab inventory + creation
    console.log('');
    console.log('--- Palette tab ---');

    // 15a: inventory loads from single /list-palettes call and auto-selects newest
    {
        ctx._elements['palette-results-dir'].value = 'pal_job';
        vm.runInContext(`
            var _palInvCalls = 0;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/list-palettes') {
                    _palInvCalls++;
                    return {
                        count: 2,
                        palettes: [
                            { palette_id: 'pal_new', created_at: '2026-03-30T10:00:00Z', display_name: 'crowding q=5.0% reef',
                              metric: 'crowding', palette: 'reef', solve_score_quantile: 0.05, solve_score_omega: 4, root_transforms: [],
                              clip_lo: -0.1, clip_hi: 0.2, image_url: 'https://example.com/new.jpeg', preview_url: 'https://example.com/new.png' },
                            { palette_id: 'pal_old', created_at: '2026-03-29T10:00:00Z', display_name: 'proximity q=0.1% inferno',
                              metric: 'proximity', palette: 'inferno', solve_score_quantile: 0.001, solve_score_omega: 1, root_transforms: [],
                              clip_lo: -1, clip_hi: 1, image_url: 'https://example.com/old.jpeg', preview_url: 'https://example.com/old.png' }
                        ]
                    };
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await loadPaletteInventory(); })()', ctx); } catch(e) {}
        const invCount = vm.runInContext('_paletteInventory.length', ctx);
        const selIdx = vm.runInContext('_paletteSelectedIdx', ctx);
        const selId = vm.runInContext('_paletteInventory[0].palette_id', ctx);
        if (invCount !== 2) { console.error('FATAL: palette inventory should have 2 entries, got ' + invCount); process.exit(1); }
        if (selIdx !== 0) { console.error('FATAL: palette inventory should auto-select first row, got ' + selIdx); process.exit(1); }
        if (selId !== 'pal_new') { console.error('FATAL: newest palette should be first, got ' + selId); process.exit(1); }
        console.log('  15a palette inventory load + auto-select: OK');
    }

    // 15a2: selected palette actions include download before delete
    {
        const dlBtn = ctx._elements['btn-palette-download'];
        const delBtn = ctx._elements['btn-palette-delete'];
        if (!dlBtn) { console.error('FATAL: palette tab missing Download button'); process.exit(1); }
        if (!delBtn) { console.error('FATAL: palette tab missing Delete button'); process.exit(1); }
        vm.runInContext(`
            var _palDownloadArgs = null;
            downloadPresignedFile = async function(url, filename, key) {
                _palDownloadArgs = { url, filename, key };
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await downloadSelectedPalette(); })()', ctx); } catch(e) {}
        const args = vm.runInContext('_palDownloadArgs', ctx);
        if (!args) { console.error('FATAL: selected palette download should call downloadPresignedFile'); process.exit(1); }
        if (args.url !== 'https://example.com/new.jpeg') { console.error('FATAL: palette download should use selected image_url'); process.exit(1); }
        if (!args.filename || !args.filename.includes('pal_new')) { console.error('FATAL: palette download filename should include palette_id'); process.exit(1); }
        console.log('  15a2 selected palette download action: OK');
    }

    // 15b: palette creation dispatches orchestrator and sets active palette run
    {
        ctx._elements['palette-results-dir'].value = 'pal_job';
        ctx._elements['palette-solve-score-quantile'].value = '1.0';
        ctx._elements['palette-solve-score-omega'].value = '3';
        vm.runInContext(`
            _activePaletteRun = null;
            try { localStorage.removeItem('polypaint_active_palette_run'); } catch(e) {}
            paletteTabMetric = 'area';
            paletteTabPalette = 'turbo';
            _paletteRtChain = [{ name: 'rotate_roots', params: ['0.25'] }];
            var _paTarget = null, _paBody = null, _paObserverStarted = false;
            startActivePaletteObserver = function() { _paObserverStarted = true; };
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'dispatch') {
                    _paTarget = name;
                    _paBody = body;
                    return { fired: 1, total: 1 };
                }
                return {};
            };
        `, ctx);
        try { await vm.runInContext('(async()=>{ await runPaletteArtifact(); })()', ctx); } catch(e) {}
        const target = vm.runInContext('_paTarget', ctx);
        const body = vm.runInContext('_paBody', ctx);
        const observerStarted = vm.runInContext('_paObserverStarted', ctx);
        const activePaletteRun = vm.runInContext('_activePaletteRun', ctx);
        if (target !== 'dispatch') { console.error('FATAL: palette artifact should call dispatch, got ' + target); process.exit(1); }
        if (body.target !== 'palette_orchestrator') { console.error('FATAL: palette artifact should dispatch palette_orchestrator'); process.exit(1); }
        if (!body.jobs || body.jobs.length !== 1) { console.error('FATAL: palette artifact should dispatch exactly one job'); process.exit(1); }
        const job = body.jobs[0];
        if (job.params.metric !== 'area') { console.error('FATAL: palette artifact metric should be area'); process.exit(1); }
        if (job.params.palette !== 'turbo') { console.error('FATAL: palette artifact palette should be turbo'); process.exit(1); }
        if (Math.abs(job.params.solve_score_omega - 3) > 0.001) { console.error('FATAL: palette artifact omega should be 3'); process.exit(1); }
        if (!job.params.root_transforms || job.params.root_transforms.length !== 1) { console.error('FATAL: palette artifact should send root transforms'); process.exit(1); }
        if (!activePaletteRun || activePaletteRun.job_id !== 'pal_job') { console.error('FATAL: palette artifact should set _activePaletteRun'); process.exit(1); }
        if (!observerStarted) { console.error('FATAL: palette artifact should start observer'); process.exit(1); }
        console.log('  15b async dispatch + active palette run: OK');
    }

    // 15c: palette observer refreshes inventory and selects finished palette
    {
        ctx._elements['palette-results-dir'].value = 'pal_job';
        vm.runInContext(`
            _activePaletteRun = { job_id: 'pal_job', run_id: 'run_pal', task_id: 'palette_run_run_pal', started_at_ms: Date.now() };
            var _paCheckCalls = 0;
            lambdaPost = async function lambdaPost(name, body, path) {
                if (name === 'storage' && path === '/check-status') {
                    _paCheckCalls++;
                    if (body.task_prefix === 'palette_run_run_pal') {
                        return {
                            done: 1, expected: 1, complete: true, errors: 0,
                            results: [{ phase: 'done', phase_label: 'Done', palette_id: 'pal_done', image_key: 'renders/pal_job/palettes/pal_done/image.jpeg' }]
                        };
                    }
                    return { done: 0, expected: 0, complete: false, errors: 0 };
                }
                if (name === 'storage' && path === '/list-palettes') {
                    return {
                        count: 1,
                        palettes: [
                            { palette_id: 'pal_done', created_at: '2026-03-30T10:00:00Z', display_name: 'area q=1.0% turbo',
                              metric: 'area', palette: 'turbo', solve_score_quantile: 0.01,
                              root_transforms: [['rotate_roots', '0.25']],
                              clip_lo: -1, clip_hi: 1,
                              image_url: 'https://example.com/pal.jpeg', preview_url: 'https://example.com/pal.png' }
                        ]
                    };
                }
                return {};
            };
            stopActivePaletteObserver = function() {};
        `, ctx);
        try { await vm.runInContext('(async()=>{ await _pollActivePaletteRun(); })()', ctx); } catch(e) {}
        const selId = vm.runInContext('_paletteInventory[0] && _paletteInventory[0].palette_id', ctx);
        const selIdx = vm.runInContext('_paletteSelectedIdx', ctx);
        const activeCleared = vm.runInContext('_activePaletteRun === null', ctx);
        if (selId !== 'pal_done') { console.error('FATAL: palette observer should refresh inventory with completed palette'); process.exit(1); }
        if (selIdx !== 0) { console.error('FATAL: palette observer should auto-select completed palette'); process.exit(1); }
        if (!activeCleared) { console.error('FATAL: palette observer should clear active run on completion'); process.exit(1); }
        console.log('  15c observer completion refresh + auto-select: OK');
    }

    // Step 16: _applyDetail with preview_stats (regression: ps TDZ bug)
    console.log('');
    console.log('--- _applyDetail preview_stats ---');
    {
        // Set up mock DOM elements
        const detailFields = ['res-cfun','res-degree','res-stripes','res-nroots','res-pform',
            'res-cfpv','res-cform','res-times','res-solver','res-prev-total','res-prev-good',
            'res-ll','res-ur'];
        for (const id of detailFields) {
            if (!ctx._elements[id]) ctx._elements[id] = { textContent: '-', style: {} };
        }

        // Mock detail response with preview_stats
        const detail = {
            has_preview: true,
            preview_url: 'https://fake/preview.png',
            param_transforms_display: [['unit_circle']],
            coeff_transforms: ['rev'],
            times: 2,
            calc: { solver: 'companion_matrix' },
            q_re: [-1, 1],
            q_im: [-1, 1],
            file_count: 42,
            preview_stats: {
                n_roots: 5000,
                n_roots_total: 7500,
                q_re: [-0.5, 0.5],
                q_im: [-0.5, 0.5],
            },
        };

        try {
            vm.runInContext(`
                _selectedJobId = 'test_ps';
                var previewEl = document.getElementById('results-preview');
                var infoEl = document.getElementById('results-info');
                _applyDetail(null, ${JSON.stringify(detail)}, previewEl, infoEl, 'test_ps');
            `, ctx);

            // Verify preview stats populated from server
            const prevTotal = ctx._elements['res-prev-total'].textContent;
            const prevGood = ctx._elements['res-prev-good'].textContent;
            if (!prevTotal.includes('7,500') && !prevTotal.includes('7500')) {
                console.error('FATAL: prev total should be 7500, got: ' + prevTotal);
                process.exit(1);
            }
            if (!prevGood.includes('5,000') && !prevGood.includes('5000')) {
                console.error('FATAL: prev good should be 5000, got: ' + prevGood);
                process.exit(1);
            }

            // Verify viewport from preview_stats (not detail.q_re)
            const ll = ctx._elements['res-ll'].textContent;
            if (!ll.includes('-0.5000')) {
                console.error('FATAL: LL should use preview_stats q_re (-0.5), got: ' + ll);
                process.exit(1);
            }

            // Verify solver
            const solver = ctx._elements['res-solver'].textContent;
            if (solver !== 'CM') {
                console.error('FATAL: solver should be CM, got: ' + solver);
                process.exit(1);
            }

            console.log('  16a _applyDetail with preview_stats: OK (prev total, prev good, LL, solver)');
        } catch (e) {
            console.error('FATAL: _applyDetail preview_stats: ' + e.message);
            process.exit(1);
        }

        // Test without preview_stats (should not crash)
        try {
            vm.runInContext(`
                _selectedJobId = 'test_no_ps';
                var previewEl2 = document.getElementById('results-preview');
                var infoEl2 = document.getElementById('results-info');
                _applyDetail(null, {
                    has_preview: false,
                    param_transforms_display: [],
                    coeff_transforms: [],
                    times: 1,
                    calc: {},
                    q_re: [-2, 2],
                    q_im: [-2, 2],
                    file_count: 10,
                }, previewEl2, infoEl2, 'test_no_ps');
            `, ctx);
            console.log('  16b _applyDetail without preview_stats: OK (no crash)');
        } catch (e) {
            console.error('FATAL: _applyDetail without preview_stats crashed: ' + e.message);
            process.exit(1);
        }
    }

    console.log('');
    console.log('--- Results populate ---');
    {
        ctx._elements['btn-populate-result'] = { ...ctx._mkEl(), disabled: false, textContent: 'Populate' };
        ctx._elements['btn-preview'] = { ...ctx._mkEl(), disabled: true, textContent: 'Preview' };
        ctx._elements['btn-render-result'] = { ...ctx._mkEl(), disabled: true, textContent: 'Render' };
        ctx._elements['btn-delete'] = { ...ctx._mkEl(), disabled: true, textContent: 'Delete' };
        ctx._elements['compute-status'] = { ...ctx._mkEl(), textContent: '' };
        ctx._elements['compute-log'] = ctx._mkEl();
        ctx._elements['results-info'] = { ...ctx._mkEl(), textContent: '' };
        ctx._elements['results-dir'] = { ...ctx._mkEl(), value: '' };
        ctx._elements['render-n'] = { ...ctx._mkEl(), value: '500' };
        ctx._elements['render-times'] = { ...ctx._mkEl(), value: '1' };
        ctx._elements['render-stripes'] = { ...ctx._mkEl(), value: '10' };
        ctx._elements['render-function'] = ctx._elements['render-function'] || { ...ctx._mkEl(), value: 'g1' };
        ctx._elements['pt-chips'] = ctx._elements['pt-chips'] || ctx._mkEl();
        ctx._elements['ct-chips'] = ctx._elements['ct-chips'] || ctx._mkEl();
        ctx._elements['cfpv-row'] = ctx._elements['cfpv-row'] || ctx._mkEl();
        ctx._elements['cfpv-inputs'] = ctx._elements['cfpv-inputs'] || ctx._mkEl();

        try {
            vm.runInContext(`
                _selectedJobId = 'job_pop';
                _resultsCache = [{
                    job_id: 'job_pop',
                    _detail: {
                        times: 3,
                        param_transforms_display: [['unit_circle'], ['rtheta', '2']],
                        coeff_transforms: ['rev', 'conj'],
                        pipeline: {
                            function: 'creative9',
                            param_transforms_display: [['unit_circle'], ['rtheta', '2']],
                            coeff_transforms: ['rev', 'conj'],
                            cfpv: [88]
                        },
                        calc: {
                            N: 1200,
                            n_chunks: 24,
                            solver: 'companion_matrix'
                        }
                    }
                }];
            `, ctx);
            await vm.runInContext('(async()=>{ await populateSelectedResult(); })()', ctx);

            const nVal = ctx._elements['render-n'].value;
            const timesVal = ctx._elements['render-times'].value;
            const chunksVal = ctx._elements['render-stripes'].value;
            const funcVal = ctx._elements['render-function'].value;
            const resultsDirVal = ctx._elements['results-dir'].value;
            const cfpv0 = ctx._elements['cfpv-p0'] ? ctx._elements['cfpv-p0'].value : null;
            const ptChain = vm.runInContext('JSON.stringify(_ptChain)', ctx);
            const ctChain = vm.runInContext('JSON.stringify(_ctChain)', ctx);
            const status = ctx._elements['compute-status'].textContent;
            if (nVal !== '1200') { console.error('FATAL: populate result should set N=1200, got ' + nVal); process.exit(1); }
            if (timesVal !== '3') { console.error('FATAL: populate result should set times=3, got ' + timesVal); process.exit(1); }
            if (chunksVal !== '24') { console.error('FATAL: populate result should set chunks=24, got ' + chunksVal); process.exit(1); }
            if (funcVal !== 'creative9') { console.error('FATAL: populate result should set function creative9, got ' + funcVal); process.exit(1); }
            if (resultsDirVal !== 'job_pop') { console.error('FATAL: populate result should set results-dir job_pop, got ' + resultsDirVal); process.exit(1); }
            if (cfpv0 !== '88') { console.error('FATAL: populate result should restore cfpv 88, got ' + cfpv0); process.exit(1); }
            if (!ptChain.includes('unit_circle') || !ptChain.includes('rtheta')) { console.error('FATAL: populate result should restore param transforms, got ' + ptChain); process.exit(1); }
            if (!ctChain.includes('rev') || !ctChain.includes('conj')) { console.error('FATAL: populate result should restore coeff transforms, got ' + ctChain); process.exit(1); }
            if (!status.includes('Calculate-CM')) { console.error('FATAL: populate result should mention Calculate-CM, got ' + status); process.exit(1); }
            console.log('  results populate restores compute settings: OK');
        } catch (e) {
            console.error('FATAL: results populate: ' + e.message);
            process.exit(1);
        }
    }

    {
        ctx._elements['render-preview'] = ctx._elements['render-preview'] || ctx._mkEl();
        ctx._elements['render-info'] = ctx._elements['render-info'] || ctx._mkEl();
        ctx._elements['render-results-dir'] = ctx._elements['render-results-dir'] || { ...ctx._mkEl(), value: 'job_old' };
        ctx._elements['palette-results-dir'] = ctx._elements['palette-results-dir'] || { ...ctx._mkEl(), value: 'job_old' };
        ctx._elements['results-dir'] = ctx._elements['results-dir'] || { ...ctx._mkEl(), value: 'job_old' };
        ctx._elements['results-scroll'] = ctx._elements['results-scroll'] || { ...ctx._mkEl(), focus() {} };
        ctx._elements['results-preview'] = ctx._elements['results-preview'] || ctx._mkEl();
        ctx._elements['results-info'] = ctx._elements['results-info'] || ctx._mkEl();
        ctx._elements['btn-render-result'] = ctx._elements['btn-render-result'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['btn-populate-result'] = ctx._elements['btn-populate-result'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['btn-preview'] = ctx._elements['btn-preview'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['btn-delete'] = ctx._elements['btn-delete'] || { ...ctx._mkEl(), disabled: true };
        try {
            vm.runInContext(`
                _renderLoadedJobId = 'job_old';
                renderArtifactPanel('job_old', {
                    calc: { exists: true, N: 1000, degree: 5 },
                    families: {
                        color: [
                            { artifact_id: 'old_art', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/job_old/color/old_art/image.jpeg', image_url: 'https://img/old.jpeg', preview_url: 'https://img/old.png', viewer_url: 'https://img/old.png', width: 1000, height: 1000, file_size: 50000, color_mode: 'rainbow', format: 'jpeg' }
                        ],
                        bilevel: [],
                        coeffs: [],
                        palette: [],
                        pdf: []
                    }
                });
                _resultsCache = [{ job_id: 'job_new', function: 'g1', degree: 4, N: 100, times: 1, total_size: 1000 }];
                selectResult('job_new');
            `, ctx);
        } catch (e) {
            console.error('FATAL: render inventory invalidation on result select: ' + e.message);
            process.exit(1);
        }
        const renderDirValAfterSelect = ctx._elements['render-results-dir'].value;
        const loadedJobAfterSelect = vm.runInContext('_renderLoadedJobId', ctx);
        const renderColorCountAfterSelect = vm.runInContext('_renderArtifacts.color.length', ctx);
        const renderPreviewAfterSelect = ctx._elements['render-preview'].innerHTML || '';
        if (renderDirValAfterSelect !== 'job_new') { console.error('FATAL: selectResult should set render-results-dir job_new, got ' + renderDirValAfterSelect); process.exit(1); }
        if (loadedJobAfterSelect !== '') { console.error('FATAL: selecting a new result should invalidate loaded render job id, got ' + loadedJobAfterSelect); process.exit(1); }
        if (renderColorCountAfterSelect !== 0) { console.error('FATAL: selecting a new result should clear stale render artifacts'); process.exit(1); }
        if (!renderPreviewAfterSelect.includes('No saved artifacts yet.')) { console.error('FATAL: selecting a new result should clear stale render preview panel'); process.exit(1); }
        console.log('  selecting a new result invalidates stale render artifacts: OK');
    }

    {
        try {
            await vm.runInContext(`
                (async()=>{
                    _renderNeedsRefresh = true;
                    _renderLoadedJobId = '';
                    document.getElementById('render-results-dir').value = 'job_auto';
                    _autoRefreshCalls = [];
                    _autoRefreshOrig = refreshRenderArtifacts;
                    refreshRenderArtifacts = async function(jobId, opts) {
                        _autoRefreshCalls.push({ jobId, opts });
                        _renderLoadedJobId = jobId;
                        _renderNeedsRefresh = false;
                    };
                    switchTab('render');
                    await Promise.resolve();
                    refreshRenderArtifacts = _autoRefreshOrig;
                })()
            `, ctx);
        } catch (e) {
            console.error('FATAL: render auto refresh on tab switch: ' + e.message);
            process.exit(1);
        }
        const autoRefreshCalls = vm.runInContext('_autoRefreshCalls', ctx);
        if (!Array.isArray(autoRefreshCalls) || autoRefreshCalls.length !== 1 || autoRefreshCalls[0].jobId !== 'job_auto') {
            console.error('FATAL: switching to render with dirty job should auto-refresh that render inventory');
            process.exit(1);
        }
        console.log('  switching to render auto-refreshes dirty render inventory: OK');
    }

    {
        ctx._elements['results-filter'] = { ...ctx._mkEl(), value: 'creative', placeholder: 'Filter by function...' };
        ctx._elements['results-filter-mode'] = { ...ctx._mkEl(), value: 'function' };
        ctx._elements['results-tbody'] = ctx._elements['results-tbody'] || ctx._mkEl();
        vm.runInContext(`
            _resultsCache = [
                { job_id: 'compute_alpha123', function: 'creative9', degree: 7, N: 100, times: 1, total_size: 1000 },
                { job_id: 'compute_beta456', function: 'g1', degree: 4, N: 100, times: 1, total_size: 1000 }
            ];
        `, ctx);
        vm.runInContext(`_updateResultsFilterUi(); renderResultsTable();`, ctx);
        const byFunctionCount = vm.runInContext('document.getElementById("results-tbody").children.length', ctx);
        const byFunctionFirst = vm.runInContext('document.getElementById("results-tbody").children[0] ? document.getElementById("results-tbody").children[0].innerHTML : ""', ctx);
        const functionPlaceholder = ctx._elements['results-filter'].placeholder || '';
        if (byFunctionCount !== 1 || !byFunctionFirst.includes('alpha123') || byFunctionFirst.includes('beta456')) {
            console.error('FATAL: results function filter should match function names only');
            process.exit(1);
        }
        if (functionPlaceholder !== 'Filter by function...') {
            console.error('FATAL: results function filter placeholder mismatch: ' + functionPlaceholder);
            process.exit(1);
        }

        ctx._elements['results-filter-mode'].value = 'job_id';
        ctx._elements['results-filter'].value = 'beta456';
        vm.runInContext(`_updateResultsFilterUi(); renderResultsTable();`, ctx);
        const byJobCount = vm.runInContext('document.getElementById("results-tbody").children.length', ctx);
        const byJobFirst = vm.runInContext('document.getElementById("results-tbody").children[0] ? document.getElementById("results-tbody").children[0].innerHTML : ""', ctx);
        const jobPlaceholder = ctx._elements['results-filter'].placeholder || '';
        if (byJobCount !== 1 || !byJobFirst.includes('beta456') || byJobFirst.includes('alpha123')) {
            console.error('FATAL: results job-id filter should match job ids only');
            process.exit(1);
        }
        if (jobPlaceholder !== 'Filter by job id...') {
            console.error('FATAL: results job-id filter placeholder mismatch: ' + jobPlaceholder);
            process.exit(1);
        }
        console.log('  results filter mode switches between function and job id: OK');
    }

    {
        ctx._elements['results-filter'] = { ...ctx._mkEl(), value: '' };
        ctx._elements['results-filter-mode'] = { ...ctx._mkEl(), value: 'function' };
        ctx._elements['results-tbody'] = ctx._elements['results-tbody'] || ctx._mkEl();
        ctx._elements['results-scroll'] = ctx._elements['results-scroll'] || { ...ctx._mkEl(), focus() {} };
        ctx._elements['btn-populate-result'] = ctx._elements['btn-populate-result'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['btn-preview'] = ctx._elements['btn-preview'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['btn-render-result'] = ctx._elements['btn-render-result'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['btn-delete'] = ctx._elements['btn-delete'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['render-results-dir'] = ctx._elements['render-results-dir'] || { ...ctx._mkEl(), value: '' };
        ctx._elements['palette-results-dir'] = ctx._elements['palette-results-dir'] || { ...ctx._mkEl(), value: '' };
        ctx._elements['results-dir'] = ctx._elements['results-dir'] || { ...ctx._mkEl(), value: '' };
        ctx._elements['results-preview'] = ctx._elements['results-preview'] || ctx._mkEl();
        ctx._elements['results-info'] = ctx._elements['results-info'] || ctx._mkEl();
        try {
            vm.runInContext(`
                _resultsSortCol = 'total_size';
                _resultsSortDir = 'desc';
                _resultsCache = [
                    { job_id: 'job_small', function: 'g1', degree: 4, N: 100, times: 1, total_size: 1000 },
                    { job_id: 'job_big', function: 'creative9', degree: 7, N: 100, times: 1, total_size: 5000 }
                ];
                renderResultsTable();
                selectResult(document.getElementById('results-tbody').children[0].dataset.jobId);
            `, ctx);
        } catch (e) {
            console.error('FATAL: results selection after sort: ' + e.message);
            process.exit(1);
        }
        const firstRowJob = vm.runInContext('document.getElementById("results-tbody").children[0].dataset.jobId', ctx);
        const firstRowClass = vm.runInContext('document.getElementById("results-tbody").children[0].className || ""', ctx);
        const secondRowClass = vm.runInContext('document.getElementById("results-tbody").children[1].className || ""', ctx);
        const firstRowScrollCalls = vm.runInContext('document.getElementById("results-tbody").children[0]._scrollIntoViewCalls || 0', ctx);
        const secondRowScrollCalls = vm.runInContext('document.getElementById("results-tbody").children[1]._scrollIntoViewCalls || 0', ctx);
        if (firstRowJob !== 'job_big') {
            console.error('FATAL: descending size sort should render job_big first, got ' + firstRowJob);
            process.exit(1);
        }
        if (firstRowClass !== 'selected' || secondRowClass === 'selected') {
            console.error('FATAL: selecting the first visible row after sort should highlight that same row');
            process.exit(1);
        }
        if (firstRowScrollCalls !== 1 || secondRowScrollCalls !== 0) {
            console.error('FATAL: sorted selection should scroll the clicked visible row only');
            process.exit(1);
        }
        console.log('  results selection follows visible sorted rows: OK');
    }

    {
        ctx._elements['render-results-dir'] = ctx._elements['render-results-dir'] || { ...ctx._mkEl(), value: '' };
        ctx._elements['render-log'] = ctx._elements['render-log'] || ctx._mkEl();
        ctx._elements['results-scroll'] = ctx._elements['results-scroll'] || { ...ctx._mkEl(), focus() {} };
        ctx._elements['btn-populate-result'] = ctx._elements['btn-populate-result'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['btn-preview'] = ctx._elements['btn-preview'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['btn-render-result'] = ctx._elements['btn-render-result'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['btn-delete'] = ctx._elements['btn-delete'] || { ...ctx._mkEl(), disabled: true };
        ctx._elements['palette-results-dir'] = ctx._elements['palette-results-dir'] || { ...ctx._mkEl(), value: '' };
        ctx._elements['results-dir'] = ctx._elements['results-dir'] || { ...ctx._mkEl(), value: '' };
        try {
            await vm.runInContext(`
                (async()=>{
                    _resultsCache = [{ job_id: 'job_go', function: 'creative9', degree: 7, N: 100, times: 1, total_size: 1000 }];
                    _selectedJobId = null;
                    _goResultOrigSwitchTab = switchTab;
                    _goResultTabs = [];
                    switchTab = function(name) { _goResultTabs.push(name); };
                    document.getElementById('render-results-dir').value = 'job_go';
                    await goResultFromRender();
                    switchTab = _goResultOrigSwitchTab;
                })()
            `, ctx);
        } catch (e) {
            console.error('FATAL: goResultFromRender: ' + e.message);
            process.exit(1);
        }
        const selectedJob = vm.runInContext('_selectedJobId', ctx);
        const goResultTabs = vm.runInContext('_goResultTabs', ctx);
        const renderLogText = ctx._elements['render-log'].textContent || '';
        if (selectedJob !== 'job_go') {
            console.error('FATAL: GoResult should select job_go in results, got ' + selectedJob);
            process.exit(1);
        }
        if (!Array.isArray(goResultTabs) || !goResultTabs.includes('results')) {
            console.error('FATAL: GoResult should switch to results tab');
            process.exit(1);
        }
        if (!renderLogText.includes('GoResult selected: job_go')) {
            console.error('FATAL: GoResult should log successful selection');
            process.exit(1);
        }
        console.log('  render GoResult selects the result set and switches tabs: OK');
    }

    console.log('');
    console.log('--- AE-MT solver wiring ---');
    {
        ctx._elements['res-solver'] = ctx._elements['res-solver'] || { textContent: '-', style: {} };

        try {
            if (typeof vm.runInContext('runCalculateAEMT', ctx) !== 'function') {
                console.error('FATAL: runCalculateAEMT should exist');
                process.exit(1);
            }

            vm.runInContext(`
                _selectedJobId = 'job_mt';
                _resultsCache = [{
                    job_id: 'job_mt',
                    _detail: {
                        times: 2,
                        pipeline: { function: 'g1' },
                        calc: { N: 900, n_chunks: 18, solver: 'aberth_mt' }
                    }
                }];
            `, ctx);
            await vm.runInContext('(async()=>{ await populateSelectedResult(); })()', ctx);

            const status = ctx._elements['compute-status'].textContent;
            if (!status.includes('Calculate-AE-MT')) {
                console.error('FATAL: populate result should mention Calculate-AE-MT, got ' + status);
                process.exit(1);
            }

            vm.runInContext(`
                _applyDetail(null, {
                    times: 1,
                    calc: { solver: 'aberth_mt' },
                    param_transforms_display: [],
                    coeff_transforms: []
                }, document.getElementById('results-preview'), document.getElementById('results-info'), 'job_mt');
            `, ctx);
            const solverLabel = ctx._elements['res-solver'].textContent;
            if (solverLabel !== 'AE-MT') {
                console.error('FATAL: res-solver should show AE-MT, got ' + solverLabel);
                process.exit(1);
            }

            console.log('  AE-MT populate + detail labels: OK');
        } catch (e) {
            console.error('FATAL: AE-MT solver wiring: ' + e.message);
            process.exit(1);
        }
    }

    console.log('=== Frontend JS Execution Test PASSED ===');
})().catch(e => { console.error('FATAL: ' + e.message); process.exit(1); });
HARNESS_EOF

node /tmp/_fe_test_harness.cjs "$CATALOG_JS" "$TRI_CATALOG_JS" "$LONG_CATALOG_JS" /tmp/_fe_test_app.js 2>&1
EXIT=$?
rm -f /tmp/_fe_test_app.js /tmp/_fe_test_harness.cjs
exit $EXIT
