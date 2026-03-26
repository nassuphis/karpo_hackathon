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

if [ ! -f "$HTML" ]; then echo "FATAL: $HTML not found"; exit 1; fi
if [ ! -f "$CATALOG_JS" ]; then echo "FATAL: $CATALOG_JS not found"; exit 1; fi

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
    appendChild(child) { el.children.push(child); },
    removeChild() {}, setAttribute() {}, insertBefore() {}, prepend() {}, append() {},
    replaceChildren() { el.children = []; },
    selectedOptions: [{ textContent: '' }],
    options: [], children: [],
    get innerHTML() { return el._html || ''; },
    set innerHTML(v) { el._html = v; el.children = []; el._innerHTMLSet = true; },
    classList: { add(){}, remove(){}, toggle(){}, contains(){ return false; } },
    addEventListener() {}, removeEventListener() {},
    getBoundingClientRect() { return {top:0,left:0,width:100,height:100}; },
    querySelectorAll() { return []; },
    querySelector() { return null; },
    focus() {}, blur() {},
    checked: false,
    set onchange(v) {},
    set onclick(v) {},
    set oninput(v) {},
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
    Image: class { set src(v) {} },
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

// Step 2: Load app JS (strip auto-init populateDropdown call)
let appCode = fs.readFileSync(process.argv[3], 'utf8');
appCode = appCode.replace(/^populateDropdown\(\);$/m, '// populateDropdown() — deferred to test');
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
// Verifies that runRasterPipeline/runBilevelPipeline/runCoeffBilevelPipeline
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
    'render-status': {},
    'render-preview': {},
    'render-info': {},
    'render-log': {},
    'btn-raster-all': {},
    'btn-bilevel-all': {},
    'btn-coeff-bilevel-all': {},
};
for (const [id, overrides] of Object.entries(renderEls)) {
    ctx._elements[id] = { ...ctx._mkEl(), ...overrides };
}

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

    _bilevelDispatchAndPoll = async () => 1234;
    refreshRenderArtifacts = async () => {};

    lambdaPost = async function lambdaPost(name, body, path) {
        if (name === 'storage' && path === '/clean-render') return { deleted: 0 };
        if (name === 'viewport') return { q_re: [-2, 2], q_im: [-2, 2], scale: 256, pix: 1024, n_roots: 1000 };
        if (name === 'dispatch' && body.target === 'render_orchestrator') return { fired: 1, errors: [] };
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
    vm.runInContext('_tilePolls = 0; _encodePolls = 0; _fakeNow = 0;', ctx);
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
    await testPipeline('runRasterPipeline', '(async()=>{ await runRasterPipeline(); })()');
    await testPipeline('runBilevelPipeline', '(async()=>{ await runBilevelPipeline(); })()');
    await testPipeline('runCoeffBilevelPipeline', '(async()=>{ await runCoeffBilevelPipeline(); })()');

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

    // Step 9: On-demand preview generation tests
    console.log('');
    console.log('--- Preview generation ---');

    // Restore real lambdaPost for preview tests (override inside VM)
    // Simulate: no cached preview, but source image exists → generate on click
    {
        // Mock dispatch+poll flow for async preview generation
        vm.runInContext(`
            var _lastDispatchRequest = null;
            lambdaPost = async function lambdaPost(name, body, path) {
                // Delete stale task
                if (name === 'storage' && path === '/delete-task') return {};
                // Dispatch
                if (name === 'dispatch' && body.target === 'render_preview') {
                    _lastDispatchRequest = body.jobs[0];
                    return { fired: 1, errors: [] };
                }
                // Poll — complete immediately
                if (name === 'storage' && path === '/check-status') {
                    return { errors: 0, done: 1, complete: true, status_counts: { done: 1 } };
                }
                // Presign the generated preview
                if (name === 'storage' && path === '/head-keys' && body.presign) {
                    var meta = {};
                    (body.keys || []).forEach(function(k) {
                        meta[k] = { size: 50000, url: 'https://fake/' + k };
                    });
                    return { exists: body.keys || [], meta: meta };
                }
                if (name === 'storage' && path === '/head-keys') {
                    return { exists: [], meta: {} };
                }
                return {};
            };
        `, ctx);

        vm.runInContext(`
            window._previewUrls = { color: null, bilevel: null };
            window._previewSources = {
                color: { jobId: 'test_job', sourceKey: 'renders/test_job/image.jpeg', previewKey: 'renders/test_job/preview_color.png' },
                bilevel: { jobId: 'test_job', sourceKey: 'renders/test_job/image_bilevel.tif', previewKey: 'renders/test_job/preview_bilevel.png' },
            };
        `, ctx);

        // Test color preview generation — verify request shape
        try {
            await vm.runInContext('(async()=>{ await _showPreview("color"); })()', ctx);
            const colorUrl = vm.runInContext('window._previewUrls.color', ctx);
            const req = vm.runInContext('_lastDispatchRequest', ctx);
            if (!colorUrl) { console.error('FATAL: color preview did not set URL'); process.exit(1); }
            if (!req || req.source_key !== 'renders/test_job/image.jpeg') {
                console.error('FATAL: color preview sent wrong source_key: ' + JSON.stringify(req));
                process.exit(1);
            }
            if (req.preview_key !== 'renders/test_job/preview_color.png') {
                console.error('FATAL: color preview sent wrong preview_key: ' + req.preview_key);
                process.exit(1);
            }
            console.log('  color preview on-demand: OK (source=' + req.source_key + ', key=' + req.preview_key + ')');
        } catch (e) {
            console.error('FATAL: color preview generation: ' + e.message);
            process.exit(1);
        }

        // Test bilevel preview generation — verify request shape
        vm.runInContext('window._previewUrls.bilevel = null; _lastDispatchRequest = null;', ctx);
        try {
            await vm.runInContext('(async()=>{ await _showPreview("bilevel"); })()', ctx);
            const bilevelUrl = vm.runInContext('window._previewUrls.bilevel', ctx);
            const req = vm.runInContext('_lastDispatchRequest', ctx);
            if (!bilevelUrl) { console.error('FATAL: bilevel preview did not set URL'); process.exit(1); }
            if (!req || req.source_key !== 'renders/test_job/image_bilevel.tif') {
                console.error('FATAL: bilevel preview sent wrong source_key: ' + JSON.stringify(req));
                process.exit(1);
            }
            if (req.preview_key !== 'renders/test_job/preview_bilevel.png') {
                console.error('FATAL: bilevel preview sent wrong preview_key: ' + req.preview_key);
                process.exit(1);
            }
            console.log('  bilevel preview on-demand: OK (source=' + req.source_key + ', key=' + req.preview_key + ')');
        } catch (e) {
            console.error('FATAL: bilevel preview generation: ' + e.message);
            process.exit(1);
        }

        // Test cached URL — no lambdaPost call
        vm.runInContext(`
            lambdaPost = async function lambdaPost() {
                throw new Error('lambdaPost should not be called for cached preview');
            };
        `, ctx);
        vm.runInContext('window._previewUrls.color = "https://cached/color.png";', ctx);
        try {
            await vm.runInContext('(async()=>{ await _showPreview("color"); })()', ctx);
            console.log('  cached preview reuse: OK (no lambdaPost call)');
        } catch (e) {
            console.error('FATAL: cached preview should not call lambdaPost: ' + e.message);
            process.exit(1);
        }

        // Test no-source fallback — assert "No preview available" message
        vm.runInContext(`
            window._previewUrls = { color: null, bilevel: null };
            window._previewSources = {};
            lambdaPost = async function lambdaPost() { return {}; };
        `, ctx);
        try {
            await vm.runInContext('(async()=>{ await _showPreview("color"); })()', ctx);
            const container = ctx._elements['preview-container'];
            const text = (container.children.length && container.children[0].textContent) || container.innerHTML || '';
            if (!text.includes('No preview')) {
                console.error('FATAL: no-source should show "No preview available", got: ' + text.slice(0, 60));
                process.exit(1);
            }
            console.log('  no-source fallback: OK ("' + text.trim().slice(0, 30) + '")');
        } catch (e) {
            console.error('FATAL: no-source preview: ' + e.message);
            process.exit(1);
        }
    }

    // Step 10: DeepZoom inventory UI tests
    console.log('');
    console.log('--- DeepZoom inventory ---');

    // Stub lambdaPost for inventory loading
    vm.runInContext(`
        var _dzListCalls = 0;
        lambdaPost = async function lambdaPost(name, body, path) {
            if (name === 'storage' && path === '/list-prefix' && body.delimiter) {
                _dzListCalls++;
                if (body.prefix === 'deepzoom/') {
                    return { prefixes: ['deepzoom/job_a/', 'deepzoom/job_b/'] };
                }
                // Per-job export prefixes
                return { prefixes: [body.prefix + 'export_1/'] };
            }
            if (name === 'storage' && path === '/head-keys') {
                return { exists: body.keys || [], meta: {} };
            }
            if (name === 'storage' && path === '/presign') {
                return { url: 'https://fake/' + body.key };
            }
            return {};
        };
    `, ctx);

    // Mock fetch for meta.json loading
    ctx.fetch = async (url) => ({
        ok: true,
        json: async () => {
            if (url.includes('job_a')) return { job_id: 'job_a', width: 4096, height: 4096, created_at: '2026-03-25T10:00:00', tiles_uploaded: 100, dzi_url: 'https://dz/job_a.dzi' };
            return { job_id: 'job_b', width: 8192, height: 8192, created_at: '2026-03-25T12:00:00', tiles_uploaded: 400, dzi_url: 'https://dz/job_b.dzi' };
        }
    });

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
        const appCode = fs.readFileSync(process.argv[3], 'utf8');
        const handlerMatch = appCode.match(/\/\/ Arrow key navigation[\s\S]*?(?=\nfunction viewDeepZoom)/);
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

    // Step 11: Solve score UI + orchestration tests
    console.log('');
    console.log('--- Solve score ---');

    // 11a: both palette containers exist
    {
        const rpContainer = ctx._elements['palette-circles-root-proximity'];
        const ssContainer = ctx._elements['palette-circles-solve-score'];
        const rpCount = rpContainer ? rpContainer.children.length : 0;
        const ssCount = ssContainer ? ssContainer.children.length : 0;
        if (rpCount < 5) { console.error('FATAL: root-proximity palette has ' + rpCount + ' circles'); process.exit(1); }
        if (ssCount < 5) { console.error('FATAL: solve-score palette has ' + ssCount + ' circles'); process.exit(1); }
        console.log('  two palette containers: OK (root=' + rpCount + ', solve=' + ssCount + ')');
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

    // 11i: solveScoreQuantile in _renderCommonParams
    {
        // Set slider to 2.0 (= q=0.02)
        ctx._elements['render-solve-score-quantile'].value = '2.0';
        const cp = vm.runInContext('_renderCommonParams()', ctx);
        if (Math.abs(cp.solveScoreQuantile - 0.02) > 0.001) {
            console.error('FATAL: solveScoreQuantile should be 0.02, got ' + cp.solveScoreQuantile);
            process.exit(1);
        }
        console.log('  solveScoreQuantile in commonParams: OK (0.02)');
        ctx._elements['render-solve-score-quantile'].value = '0.1';
    }

    // 11j: solve_score_quantile in orchestrator payload only when solve_score mode
    {
        vm.runInContext("renderColorMode = 'solve_score'; renderSolveMetric = 'proximity';", ctx);
        ctx._elements['render-solve-score-quantile'].value = '3.0';
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
            renderColorMode = 'rainbow';
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
        vm.runInContext("_viewMode = 'square'; _rtChain = [];", ctx);
        await vm.runInContext('(async()=>{ await runRasterPipeline(); })()', ctx);
        orchDispatched = vm.runInContext('_orchDispatched', ctx);
        if (!orchDispatched) { console.error('FATAL: runRasterPipeline did not dispatch orchestrator'); process.exit(1); }
        if (orchDispatched.mode !== 'color') { console.error('FATAL: mode should be color, got ' + orchDispatched.mode); process.exit(1); }
        console.log('  12a runRasterPipeline dispatches orchestrator: OK (mode=color)');
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

    // Step 13: Render refresh — tested in Playwright (VM cannot override let-scoped lambdaPost/fetch)
    // See tests/e2e/render-refresh.spec.js for real browser coverage.

    console.log('=== Frontend JS Execution Test PASSED ===');
})().catch(e => { console.error('FATAL: ' + e.message); process.exit(1); });
HARNESS_EOF

node /tmp/_fe_test_harness.cjs "$CATALOG_JS" /tmp/_fe_test_app.js 2>&1
EXIT=$?
rm -f /tmp/_fe_test_app.js /tmp/_fe_test_harness.cjs
exit $EXIT
