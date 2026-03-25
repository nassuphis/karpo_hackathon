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
    removeChild() {}, setAttribute() {}, insertBefore() {},
    selectedOptions: [{ textContent: '' }],
    options: [], children: [],
    get innerHTML() { return el._html || ''; },
    set innerHTML(v) { el._html = v; el.children = []; },
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
    createEvent() { return { initEvent() {} }; },
};

// Build the VM context with browser-like globals
const ctx = {
    console,
    document: docStub,
    localStorage: { getItem() { return null; }, setItem() {}, removeItem() {} },
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

console.log('=== Frontend JS Execution Test PASSED ===');
HARNESS_EOF

node /tmp/_fe_test_harness.cjs "$CATALOG_JS" /tmp/_fe_test_app.js 2>&1
EXIT=$?
rm -f /tmp/_fe_test_app.js /tmp/_fe_test_harness.cjs
exit $EXIT
