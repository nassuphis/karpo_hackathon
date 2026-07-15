// PolyPaint 08-chip-editors — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Deploy rewrites the script tags to build-versioned asset
// keys (assets/<BUILD_ID>/...), so a deploy flips atomically via index.html.
function _paramValue(item, pDefs, idx) {
    const params = Array.isArray(item && item.params) ? item.params : [];
    if (params[idx] != null && params[idx] !== '') return params[idx];
    const def = pDefs && pDefs[idx] ? pDefs[idx].def : '';
    return def == null ? '' : String(def);
}

let _paramProgramEditorMode = 'text';
let _coeffProgramEditorMode = 'text';

function _paramProgramTextModeSelected() {
    return _paramProgramEditorMode === 'text';
}

function _coeffProgramTextModeSelected() {
    return _coeffProgramEditorMode === 'text';
}

function _paramProgramSourceTextarea() {
    return document.getElementById('pp-source-text');
}

function _coeffProgramSourceTextarea() {
    return document.getElementById('cp-source-text');
}

function _paramProgramSourceFromRows(chain) {
    const rows = Array.isArray(chain) ? chain : [];
    const lines = [];
    for (let idx = 0; idx < rows.length; idx++) {
        const row = Array.isArray(rows[idx]) ? rows[idx] : [String(rows[idx] || '')];
        const name = String(row[0] || '').trim();
        const params = row.slice(1).map(v => _str(v));
        const next = Array.isArray(rows[idx + 1]) ? rows[idx + 1] : null;
        // Old two-arg const rows carry (re, im); mirror the Python serializer
        // (param_source_text_from_chain), which emits `(re)+(im)*1j`.
        const constExpr = name === 'const' && params.length >= 2
            ? `(${params[0]})+(${params[1]})*1j`
            : params[0];
        if (name === 'const' && params.length >= 1 && next && next[0] === 'emit' && next[1]) {
            lines.push(`${String(next[1]).trim()} = ${constExpr}`);
            idx++;
            continue;
        }
        if (name === 'const' && params.length >= 1) {
            lines.push(`const(${constExpr})`);
        } else if (name === 'push') {
            lines.push(params[0] ? `push(${params[0]})` : 'push');
        } else if (name === 'emit') {
            const target = String(params[0] || 'p1').toLowerCase();
            if (target === 'p1' || target === 'p2') lines.push(target === 'p2' ? 'emit_p2' : 'emit_p1');
            else lines.push(`emit(${target})`);
        } else if (name === 'duplicate') {
            lines.push('dup');
        } else if (name === 'legacy') {
            lines.push(`legacy(${params.join(', ')})`);
        } else if (name === 'macro' && params[0]) {
            lines.push(`macro(${params[0]})`);
        } else if (params.length) {
            lines.push(`${name}(${params.join(', ')})`);
        } else if (name) {
            lines.push(name);
        }
    }
    return lines.filter(Boolean).join('\n');
}

function _setParamProgramSourceText(text, options = {}) {
    const value = String(text == null ? '' : text);
    const el = _paramProgramSourceTextarea();
    if (el && el.value !== value) el.value = value;
    if (options.auto === true) {
        _paramProgramSourceAutoSynthed = true;
    } else if (options.auto === false || value.trim()) {
        _paramProgramSourceAutoSynthed = false;
    }
}

function _getParamProgramSourceText() {
    const el = _paramProgramSourceTextarea();
    return String(el ? el.value : '');
}

let _paramProgramSourceAutoSynthed = false;

function _setParamProgramEditorMode(_mode) {
    // Param Program is text-only (the read-only chips tab was removed). Pin the
    // mode to 'text' so save/preview/debug/compute always send the source, and
    // still synthesize text from a loaded chain (chain-only programs) so the
    // editor is never empty when there is a program to show.
    _paramProgramEditorMode = 'text';
    const textPanel = document.getElementById('pp-text-panel');
    if (textPanel && textPanel.classList) textPanel.classList.add('active');
    if (_paramProgramSourceAutoSynthed && !_ppChain.length) {
        _setParamProgramSourceText('', { auto: false });
    } else if (_ppChain.length &&
        (!_getParamProgramSourceText().trim() || _paramProgramSourceAutoSynthed)) {
        _setParamProgramSourceText(_paramProgramSourceFromRows(_serializeParamProgramChain()), { auto: true });
    }
    _paramProgramStatus('Text source is authoritative for save, preview, debug, and compute.');
    _syncParamPipelineModeUi();
    if (typeof _paramProgramModalState !== 'undefined' && _paramProgramModalState.open) _renderParamProgramModal();
    if (_paramProgramModeSelected()) _markComputePreviewStale();
}

// One debounced advisory-validation engine for every program source
// editor. Each entry declares its textarea, backend compile route, status
// sink, and ok-line formatter (per-kind texts preserved). Responses are
// advisory — save/preview/render surface real errors.
function _programStatementCountOk(resp, withTokens = false) {
    const count = Number(resp.statement_count) || 0;
    const base = `Text source OK: ${count} statement${count === 1 ? '' : 's'}`;
    if (!withTokens) return `${base}.`;
    return `${base}, ${resp.program && resp.program.token_count || 0} tokens.`;
}

const _programSourceValidators = {
    pp: {
        textarea: () => _paramProgramSourceTextarea(),
        route: '/compile-param-program-source',
        gate: () => _paramProgramTextModeSelected(),
        status: (message, isError) => _paramProgramStatus(message, isError),
        okStatus: resp => _programStatementCountOk(resp, true),
    },
    cp: {
        textarea: () => _coeffProgramSourceTextarea(),
        route: '/compile-coeff-program-source',
        gate: () => _coeffProgramTextModeSelected(),
        status: (message, isError) => _coeffProgramStatus(message, isError),
        okStatus: resp => _programStatementCountOk(resp, true),
    },
    'render-rt': {
        textarea: () => _rootProgramSourceTextarea('render'),
        route: '/compile-root-program-source',
        status: (message, isError) => _rootProgramStatus('render', message, isError),
        okStatus: resp => _programStatementCountOk(resp),
    },
    'palette-rt': {
        textarea: () => _rootProgramSourceTextarea('palette'),
        route: '/compile-root-program-source',
        status: (message, isError) => _rootProgramStatus('palette', message, isError),
        okStatus: resp => _programStatementCountOk(resp),
    },
    'render-ss': {
        textarea: () => _solveScoreSourceTextarea('render'),
        route: '/compile-solve-score-program-source',
        status: (message, isError) => _setSolveScoreProgramStatus('render', message, isError),
        okStatus: resp => _programStatementCountOk(resp),
    },
    'palette-ss': {
        textarea: () => _solveScoreSourceTextarea('palette'),
        route: '/compile-solve-score-program-source',
        status: (message, isError) => _setSolveScoreProgramStatus('palette', message, isError),
        okStatus: resp => _programStatementCountOk(resp),
    },
};

let _programSourceValidationTimers = {};
let _programSourceValidationSeqs = {};
function _scheduleProgramSourceValidation(key) {
    const def = _programSourceValidators[key];
    if (!def) return;
    if (_programSourceValidationTimers[key]) clearTimeout(_programSourceValidationTimers[key]);
    _programSourceValidationTimers[key] = setTimeout(async () => {
        _programSourceValidationTimers[key] = null;
        const el = def.textarea();
        const sourceText = String(el ? el.value : '');
        if (!sourceText.trim()) return;
        if (def.gate && !def.gate()) return;
        const seq = (_programSourceValidationSeqs[key] = (_programSourceValidationSeqs[key] || 0) + 1);
        try {
            const resp = await lambdaPost('storage', { source_text: sourceText }, def.route);
            const now = def.textarea();
            if (seq !== _programSourceValidationSeqs[key] || sourceText !== String(now ? now.value : '')) return;
            if (resp && resp.ok) {
                def.status(def.okStatus(resp), false);
            } else if (resp && Array.isArray(resp.diagnostics)) {
                const first = resp.diagnostics.find(d => d && d.level === 'error') || resp.diagnostics[0];
                if (first) def.status(`Line ${first.line || '?'}: ${first.message}`, true);
            }
        } catch (_) {
            /* advisory only; save/preview/render surface real errors */
        }
    }, 900);
}

function _onParamProgramSourceInput() {
    _paramProgramSourceAutoSynthed = false;
    _scheduleProgramSourceValidation('pp');
    if (_paramProgramTextModeSelected() && _paramProgramModeSelected()) _markComputePreviewStale();
    _syncParamPipelineModeUi();
    if (typeof _paramProgramModalState !== 'undefined' && _paramProgramModalState.open) _renderParamProgramModal();
    _paramProgramStatus('Text source changed. It will be compiled by the backend on save/preview/compute.');
}

function _insertSourceTextSnippet(el, snippet) {
    if (!el) return false;
    const text = String(snippet || '');
    const start = Number.isFinite(el.selectionStart) ? el.selectionStart : el.value.length;
    const end = Number.isFinite(el.selectionEnd) ? el.selectionEnd : start;
    const before = el.value.slice(0, start);
    const after = el.value.slice(end);
    const lead = before && !before.endsWith('\n') ? '\n' : '';
    const tail = after && !text.endsWith('\n') ? '\n' : '';
    const inserted = `${lead}${text}`;
    el.value = before + inserted + tail + after;
    const pos = before.length + inserted.length;
    if (typeof el.setSelectionRange === 'function') el.setSelectionRange(pos, pos);
    if (typeof el.focus === 'function') el.focus();
    return true;
}

function _insertParamProgramSourceSnippet(snippet) {
    if (_insertSourceTextSnippet(_paramProgramSourceTextarea(), snippet)) _onParamProgramSourceInput();
}

function _insertCoeffProgramSourceSnippet(snippet) {
    if (_insertSourceTextSnippet(_coeffProgramSourceTextarea(), snippet)) _onCoeffProgramSourceInput();
}

function _insertRenderRootSourceSnippet(snippet) {
    if (_insertSourceTextSnippet(_rootProgramSourceTextarea('render'), snippet)) _onRootProgramSourceInput('render');
}

function _insertPaletteRootSourceSnippet(snippet) {
    if (_insertSourceTextSnippet(_rootProgramSourceTextarea('palette'), snippet)) _onRootProgramSourceInput('palette');
}

function _programSourceCheatButtonHtml(insertFn, label, snippet, title = '') {
    const safeLabel = _escapeHtml(label);
    const safeTitle = title ? ` title="${_escapeHtml(title)}"` : '';
    const safeSnippet = _escapeHtml(JSON.stringify(String(snippet || '')));
    return `<button type="button" class="btn-secondary program-source-cheat-button" onclick="${insertFn}(${safeSnippet})"${safeTitle}>${safeLabel}</button>`;
}

function _programSourceCheatSectionHtml(title, buttons) {
    if (!buttons.length) return '';
    return `<div class="program-source-cheat-section"><div class="program-source-cheat-title">${_escapeHtml(title)}</div><div class="program-source-cheat-buttons">${buttons.join('')}</div></div>`;
}

function _sourceCheatDefaultArgs(params, options = {}) {
    const includeAndy = !!options.includeAndy;
    return (params || [])
        .filter(p => includeAndy || !(typeof _isAndyParam === 'function' && _isAndyParam(p)))
        .map(p => String((p && p.def) || '0'));
}

function _paramProgramLegacySnippet(name) {
    const argDefs = _paramRegistryAdapter.params(name);
    const args = _sourceCheatDefaultArgs(argDefs);
    return `legacy(${name}, both, both${args.length ? ', ' + args.join(', ') : ''})`;
}

function _paramProgramLegacyUiSpec(name) {
    return _paramRegistryAdapter.spec(name);
}

function _paramProgramLegacyButton(name) {
    const spec = _paramProgramLegacyUiSpec(name);
    return {
        label: name,
        snippet: _paramProgramLegacySnippet(name),
        title: spec.desc || 'Legacy parameter transform with explicit source/target selectors.',
        registryName: name,
    };
}

function _paramProgramLegacyCheatSections() {
    const groups = {};
    (_paramProgramLegacyNames || []).forEach(name => {
        if (!name || name === 'none') return;
        const spec = _paramProgramLegacyUiSpec(name);
        const category = spec.category || 'legacy';
        if (!groups[category]) groups[category] = [];
        groups[category].push(_paramProgramLegacyButton(name));
    });
    const order = ['maps', 'arithmetic', 'shapes', 'roots', 'dither', 'legacy'];
    return order
        .filter(category => (groups[category] || []).length)
        .map(category => ({
            title: `Legacy: ${(_paramRegistryAdapter.category(category).title || category)}`,
            buttons: groups[category],
        }));
}

const _paramProgramCheatSections = [
    {
        title: 'Language',
        buttons: [
            { label: 'x = expr', snippet: 'w = 2*pi\np1 = sin(w*t1)\np2 = cos(w*t2)', title: 'Local alias: write-once, substituted at compile time. Any unreserved name; also valid in legacy(...) args.' },
            { label: 'r1..r8 registers', snippet: 'r1 = t1 + 1\nr1 = r1 * r1\np1 = r1\np2 = t2', title: 'Mutable scratch registers, zeroed per evaluation: r1 = f(r1) rebinds for real (aliases are write-once). Reads before any write are 0.' },
        ],
    },
    {
        title: 'Starters',
        buttons: [
            { label: 'identity', snippet: 'p1 = t1\np2 = t2', title: 'Pass through input parameters.' },
            { label: 'mix', snippet: 'p1 = t1 + t2\np2 = exp(t2*pi2i)', title: 'Assignment expressions can use t1, t2, p1, p2, pi, pi2, and pi2i.' },
            { label: 'unit circle', snippet: 'p1 = exp(t1*pi2i)\np2 = exp(t2*pi2i)', title: 'Map both inputs onto the unit circle.' },
        ],
    },
    {
        title: 'Assignments + Expressions',
        buttons: [
            { label: 'p1 = expr', snippet: 'p1 = t1 + t2', title: 'Assign a complex expression to p1.' },
            { label: 'p2 = expr', snippet: 'p2 = exp(t2*pi2i)', title: 'Assign a complex expression to p2.' },
            { label: 'const(expr)', snippet: 'const(p1+p2)\nemit_p1', title: 'Push a complex expression, then emit it.' },
            { label: 'macro(name)', snippet: 'macro(saved-param-program)', title: 'Inline a saved Param Program at compile time.' },
        ],
    },
    {
        title: 'Input + Output',
        buttons: [
            { label: 'push()', snippet: 'push()\nemit_p1', title: 'Push the default input value, then emit it.' },
            { label: 'push(t1)', snippet: 'push(t1)\nemit_p1', title: 'Push input t1, then emit it.' },
            { label: 'push(t2)', snippet: 'push(t2)\nemit_p2', title: 'Push input t2, then emit it.' },
            { label: 'emit_p1', snippet: 'push(t1)\nemit_p1', title: 'Pop the top value into p1.' },
            { label: 'emit_p2', snippet: 'push(t2)\nemit_p2', title: 'Pop the top value into p2.' },
        ],
    },
    {
        title: 'Stack',
        buttons: [
            { label: 'dup', snippet: 'push(t1)\ndup\nemit_p1\nemit_p2', title: 'Duplicate the top stack value.' },
            { label: 'swap', snippet: 'push(t1)\npush(t2)\nswap\nemit_p1\nemit_p2', title: 'Swap the top two stack values.' },
            { label: 'pop', snippet: 'push(t1)\npop', title: 'Discard the top stack value.' },
            { label: 'flush', snippet: 'push(t1)\nflush', title: 'Clear the temporary stack.' },
        ],
    },
    {
        title: 'Arithmetic',
        buttons: [
            { label: 'add', snippet: 'push(t1)\npush(t2)\nadd\nemit_p1', title: 'Pop a,b and push a+b.' },
            { label: 'subtract / sub', snippet: 'push(t1)\npush(t2)\nsubtract\nemit_p1', title: 'Pop a,b and push a-b.' },
            { label: 'mul', snippet: 'push(t1)\npush(t2)\nmul\nemit_p1', title: 'Pop a,b and push a*b.' },
            { label: 'ratio / div', snippet: 'push(t1)\npush(t2)\nratio\nemit_p1', title: 'Pop a,b and push a/b with zero policy.' },
        ],
    },
    {
        title: 'Unary',
        buttons: [
            { label: 'negate', snippet: 'push(t1)\nnegate\nemit_p1', title: 'Apply -z to the top stack value.' },
            { label: 'conj / conjugate', snippet: 'push(t1)\nconj\nemit_p1', title: 'Complex conjugate.' },
            { label: 'reciprocal', snippet: 'push(t1)\nreciprocal\nemit_p1', title: 'Apply 1/z.' },
            { label: 'unit_circle', snippet: 'push(t1)\nunit_circle\nemit_p1', title: 'Map through unit-circle transform.' },
            { label: 'square', snippet: 'push(t1)\nsquare\nemit_p1', title: 'Square the top stack value.' },
            { label: 'cube', snippet: 'push(t1)\ncube\nemit_p1', title: 'Cube the top stack value.' },
            { label: 'exp', snippet: 'push(t1)\nexp\nemit_p1', title: 'Complex exponential.' },
            { label: 'square(p1)', snippet: 'square(p1)', title: 'Targeted unary form: mutate p1 directly.' },
        ],
    },
].concat(_paramProgramLegacyCheatSections());

function _coeffStructuralChip(name) {
    return _coeffRegistryAdapter.structuralChip(name);
}

function _coeffFamilySubOps(familyName) {
    const chip = _coeffStructuralChip(familyName);
    return Array.isArray(chip && chip.sub_ops) ? chip.sub_ops.slice() : [];
}

function _coeffSubOpLabel(op) {
    const aliases = Array.isArray(op && op.source_aliases) ? op.source_aliases : [];
    return aliases.length ? `${op.name} / ${aliases.join(' / ')}` : op.name;
}

function _coeffRegistrySourceName(name) {
    return _coeffRegistryAdapter.sourceName(name);
}

function _coeffNativeTransformSnippet(name) {
    const sourceName = _coeffRegistrySourceName(name);
    const spec = _coeffRegistryAdapter.spec(name);
    const args = _sourceCheatDefaultArgs(spec.params || []);
    return `poly = ${sourceName}(poly${args.length ? ', ' + args.join(', ') : ''})\nemit`;
}

function _coeffNativeTransformButton(name) {
    const spec = _coeffRegistryAdapter.spec(name);
    const sourceName = _coeffRegistrySourceName(name);
    return {
        label: sourceName === name ? name : `${sourceName} (${name})`,
        snippet: _coeffNativeTransformSnippet(name),
        title: spec.desc || 'Native coefficient transform.',
        registryName: name,
    };
}

function _coeffNativeTransformCheatSections() {
    const groups = {};
    (_coeffProgramLegacyNames || []).forEach(name => {
        const spec = _coeffRegistryAdapter.spec(name);
        const category = spec.category || 'structural';
        if (!groups[category]) groups[category] = [];
        groups[category].push(_coeffNativeTransformButton(name));
    });
    const order = ['structural', 'accumulation', 'elementwise', 'roots'];
    return order
        .filter(category => (groups[category] || []).length)
        .map(category => ({
            title: `Native: ${(_coeffRegistryAdapter.category(category).title || category)}`,
            buttons: groups[category],
        }));
}

const _coeffProgramCheatSections = [
    {
        title: 'Starters',
        buttons: [
            { label: 'emit cf', snippet: 'cf\nemit', title: 'Push the input coefficient vector and emit it.' },
            { label: 'reverse', snippet: 'poly = rev(poly)\nemit', title: 'Reverse the working coefficient vector.' },
            { label: 'range', snippet: 'poly = arange(1, poly_len+1)\nemit', title: 'Replace poly with 1..poly_len.' },
        ],
    },
    {
        title: 'Input + Output',
        buttons: [
            { label: 'cf', snippet: 'cf', title: 'Push the input coefficient vector.' },
            { label: 'poly', snippet: 'poly', title: 'Push the current working poly vector.' },
            { label: 'emit', snippet: 'emit', title: 'Pop/commit the top vector as output poly.' },
            { label: 'push(cf)', snippet: 'push(cf)', title: 'Push cf explicitly.' },
            { label: 'push(poly)', snippet: 'push(poly)', title: 'Push poly explicitly.' },
            { label: 'poly = cf', snippet: 'poly = cf\nemit', title: 'Copy cf into poly.' },
            { label: 'macro(name)', snippet: 'macro(saved-coeff-program)', title: 'Inline a saved Coeff Program at compile time.' },
        ],
    },
    {
        title: 'Language',
        buttons: [
            { label: 'x = expr', snippet: 'x1 = log(abs(p1 + p2) + 1)\npoly = multiply(range(1, 37), 1i * x1)', title: 'Local alias: write-once, substituted at compile time. Any unreserved name.' },
            { label: 'scan', snippet: 'poly = scan(36, 1, p1 + p2, (sin(k*prev) + 1) / abs(sin(k*prev) + 1))', title: 'Bounded recurrence: out[0] = init at k=k0; out[j] = step with prev = previous element, k = k0 + j.' },
            { label: 'poly[a:b]', snippet: 'poly[2:7] = multiply(poly[2:7], 5)', title: 'Slice read/write: poly[a:b] is a VECTOR, so combine it with call forms (multiply/add/divide...) — infix * + / is scalar-only. Slice assignment takes a vector of length b-a.' },
            { label: 'sum / prod', snippet: 'sum(poly[0:70])\npoly[70] = tos[0] + 1i*p1\ndrop', title: 'Reductions push a scalar; read it back with tos[0] in the next poke, then drop.' },
            { label: 'window', snippet: 'poly = multiply(poly, window(3, 6))', title: 'Exact 0/1 mask for slots [a, b) over the current poly length. step(a) is the one-sided form.' },
        ],
    },
    {
        title: 'Constants + Ranges',
        buttons: [
            { label: 'push_vec(value)', snippet: 'push_vec(0)\nemit', title: 'Push a constant vector of length poly_len.' },
            { label: 'push_vec(n,value)', snippet: 'push_vec(poly_len, p1)\nemit', title: 'Push a constant vector with explicit length.' },
            { label: 'fill', snippet: 'fill(poly_len, 0)\nemit', title: 'Alias for push_vec/fill vector construction.' },
            { label: 'vector_literal', snippet: 'poly = vector_literal(1, -3, 2)\nemit', title: 'Compile a static leading-first coefficient vector once and load it from the program constant pool.' },
            { label: 'roots_literal', snippet: 'poly = roots_literal(1, 2)\nemit', title: 'Expand the monic polynomial with these static roots once at compile time; the pool stores the resulting coefficients. Double-click the name to drag the roots on the root pad.' },
            { label: 'roots_chess_literal', snippet: 'poly = roots_chess_literal(5, 1, 0)\nemit', title: 'Dark cells of a d x d board (corners dark), full side w, centered on o. Scrub w on the 1D pad and o on the 2D pad.' },
            { label: 'roots_grid_literal', snippet: 'poly = roots_grid_literal(4, 1, 0)\nemit', title: 'The full d x d root lattice, side w, centered on o.' },
            { label: 'roots_ring_literal', snippet: 'poly = roots_ring_literal(7, 1, 0)\nemit', title: 'n roots on a circle: o + r*exp(2*pi*i*k/n). A complex r rotates the ring.' },
            { label: 'translate_roots', snippet: 'poly = translate_roots(poly, 0.1*exp(pi2i*t1))\nemit', title: 'Shift every root by delta without solving for roots: Q(z) = P(z-delta).' },
            { label: 'bimodal', snippet: 'push_scalar(bimodal(t2, 0.7))', title: 'Symmetric bimodal remapping of u in [0,1], shaped by a in [0,1).' },
            { label: 'push_scalar', snippet: 'push_scalar(p1+p2)', title: 'Push one scalar onto the typed stack.' },
            { label: 'arange', snippet: 'poly = arange(1, poly_len+1)\nemit', title: 'Range vector, stop-exclusive.' },
            { label: 'linspace', snippet: 'poly = linspace(0, 1, poly_len)\nemit', title: 'Linearly spaced vector.' },
            { label: 'littlewood', snippet: 'poly = littlewood(0, 1)\nemit', title: 'Randomly choose value1/value2 per coefficient.' },
        ],
    },
    {
        title: 'Stack',
        buttons: [
            { label: 'dup', snippet: 'dup', title: 'Duplicate the top stack slot.' },
            { label: 'swap', snippet: 'swap', title: 'Swap the top two stack slots.' },
            { label: 'drop', snippet: 'drop', title: 'Discard the top stack slot.' },
            { label: 'flush', snippet: 'flush', title: 'Clear the stack.' },
            { label: 'poly = pop', snippet: 'poly = pop\nemit', title: 'Pop a vector into poly.' },
            { label: 'poly = peek', snippet: 'poly = peek\nemit', title: 'Copy top vector into poly without popping.' },
            { label: 'blend', snippet: 'cf\npoly\npoly = blend(0.5)\nemit', title: 'Blend top two vectors using scalar t.' },
        ],
    },
    {
        title: 'Index + Scalar',
        buttons: [
            { label: 'poly[i] = expr', snippet: 'poly[0] = p1\nemit', title: 'Set one poly coefficient.' },
            { label: 'poly[poly_len-1]', snippet: 'poly[poly_len-1] = p2\nemit', title: 'Dynamic index using poly_len.' },
            { label: 'poke_poly', snippet: 'poke_poly(0, p1)\nemit', title: 'Statement form for setting a poly coefficient.' },
            { label: 'poke_tos', snippet: 'poly\npoke_tos(0, p2)\nemit', title: 'Set one element of the top stack vector.' },
            { label: 'poly[i] read', snippet: 'push_scalar(poly[0])', title: 'Read one poly coefficient as a scalar.' },
            { label: 'cf[i] read', snippet: 'push_scalar(cf[0])', title: 'Read one input coefficient as a scalar.' },
            { label: 'tos[i] read', snippet: 'push_scalar(tos[0])', title: 'Read one top-of-stack vector element as a scalar.' },
        ],
    },
    {
        title: 'Affine',
        buttons: [
            { label: 'scale', snippet: 'poly = scale(poly, p1)\nemit', title: 'Multiply vector/source by a scalar expression.' },
            { label: 'shift', snippet: 'poly = shift(poly, p2)\nemit', title: 'Add a scalar expression to vector/source.' },
            { label: 'linear', snippet: 'poly = linear(poly, p1, p2)\nemit', title: 'Apply vector*p1+p2.' },
            { label: 'affine', snippet: 'affine(poly, poly, p1, p2)\nemit', title: 'Explicit target/source affine form.' },
        ],
    },
    {
        title: 'Vector Binary',
        buttons: _coeffFamilySubOps('vector_binary').map(op => ({
            label: _coeffSubOpLabel(op),
            snippet: `poly = ${op.name}(poly, p1)\nemit`,
            title: `${op.name}(left, right) supports vector/scalar broadcasting.`,
        })),
    },
    {
        title: 'Vector Unary',
        buttons: _coeffFamilySubOps('vector_unary').map(op => ({
            label: _coeffSubOpLabel(op),
            snippet: `poly = ${op.name}(poly)\nemit`,
            title: `${op.name}(source) elementwise vector/scalar operation.`,
        })),
    },
    {
        title: 'Ordering',
        buttons: [
            ..._coeffFamilySubOps('vector_roll').map(op => ({
                label: op.name,
                snippet: `poly = ${op.name}(poly, 1)\nemit`,
                title: `${op.name}(source, n).`,
            })),
            { label: 'argsort', snippet: 'poly = argsort(pop, peek)\nemit', title: 'argsort(src1, src2) with pop/peek/poly vector sources.' },
        ],
    },
].concat(_coeffNativeTransformCheatSections());

function _rootTransformSnippet(name) {
    // Parens are required even with no args — the source parser rejects
    // bare transform names.
    const params = _rootRegistryAdapter.params(name);
    const args = params.map(p => String((p && p.def) || '0'));
    return `${name}(${args.join(', ')})`;
}

function _rootTransformCheatButtons() {
    return _rootRegistryAdapter.names.map(name => {
        const spec = _rootRegistryAdapter.spec(name);
        return {
            label: spec.label ? `${name} (${spec.label})` : name,
            snippet: _rootTransformSnippet(name),
            title: spec.desc || 'Root transform.',
        };
    });
}

const _rootProgramCheatSections = [
    {
        title: 'Language',
        buttons: [
            { label: 'x = value', snippet: 'k = 0.25\nadd_complex(k)', title: 'Local alias: write-once numeric constants, substituted into transform arguments at compile time.' },
        ],
    },
    {
        title: 'Starters',
        buttons: [
            { label: 'quarter turn', snippet: 'rotate_roots(0.25)', title: 'Rotate all roots a quarter turn about the origin.' },
            { label: 'snap to circle', snippet: 'pull_unit_circle(0.75, 1)', title: 'Pull roots near |z| = 1 onto the unit circle.' },
            { label: 'circle to line', snippet: 'roots_toline()', title: 'Cayley transform: unit circle to real line.' },
            { label: 'rotate + pull', snippet: 'rotate_roots(0.125)\npull_unit_circle(0.75, 1)', title: 'Statements apply in order, top to bottom.' },
        ],
    },
    { title: 'Transforms', buttons: _rootTransformCheatButtons() },
];

// Side-panel keys: pp = param, cp = coeff (Compute tab), rt = render root
// transforms, prt = palette root transforms, render-ss / palette-ss =
// solve-score editors. Twin editors share one generated help registry
// (rt+prt cached under 'rt', render-ss+palette-ss under 'ss').
let _programSourceSidePanelMode = { pp: 'starter', cp: 'starter', rt: 'starter', prt: 'starter', 'render-ss': 'starter', 'palette-ss': 'starter' };
let _programHelpRegistryCache = { pp: null, cp: null, rt: null, ss: null };
let _programHelpInspectorBound = false;

function _programSourceWhichKey(which) {
    return which === 'cp' || which === 'rt' || which === 'prt' || which === 'render-ss' || which === 'palette-ss'
        ? which
        : 'pp';
}

function _programHelpRegistryKey(key) {
    if (key === 'prt') return 'rt';
    if (key === 'render-ss' || key === 'palette-ss') return 'ss';
    return key;
}

function _programHelpParamName(param, idx = 0) {
    return String((param && (param.name || param.ph || param.label)) || `p${idx + 1}`);
}

function _programHelpParamDefault(param) {
    if (!param) return '';
    if (param.default !== undefined) return String(param.default);
    if (param.def !== undefined) return String(param.def);
    return '';
}

function _programHelpParamText(param, idx = 0) {
    const name = _programHelpParamName(param, idx);
    const def = _programHelpParamDefault(param);
    return def === '' ? name : `${name}=${def}`;
}

function _programHelpItem(name, signature, help = '', options = {}) {
    const aliases = Array.isArray(options.aliases) ? options.aliases.filter(Boolean).map(String) : [];
    const params = Array.isArray(options.params) ? options.params : [];
    const forms = Array.isArray(options.forms) ? options.forms.filter(Boolean).map(String) : [];
    const examples = Array.isArray(options.examples) ? options.examples.filter(Boolean).map(String) : [];
    const notes = Array.isArray(options.notes) ? options.notes.filter(Boolean).map(String) : [];
    return {
        name: String(name || ''),
        signature: String(signature || name || ''),
        help: String(help || ''),
        category: String(options.category || ''),
        aliases,
        params,
        forms,
        effect: String(options.effect || ''),
        examples,
        notes,
        insert: options.insert || '',
        missing: !!options.missing,
        lookup: options.lookup !== false,
    };
}

function _newProgramHelpRegistry() {
    return { sections: [], lookup: new Map() };
}

function _programHelpAddSection(registry, title, items) {
    const filtered = (items || []).filter(item => item && item.name);
    if (!filtered.length) return;
    registry.sections.push({ title, items: filtered });
    filtered.forEach(item => {
        if (item.lookup === false) return;
        const keys = [item.name].concat(item.aliases || []);
        keys.forEach(key => {
            const norm = _normalizeProgramHelpToken(key);
            if (!norm) return;
            const existing = registry.lookup.get(norm);
            if (!existing || _programHelpLookupShouldReplace(norm, existing, item)) {
                registry.lookup.set(norm, item);
            }
        });
    });
}

function _programHelpLookupScore(item) {
    if (!item) return 0;
    return (Array.isArray(item.params) ? item.params.length * 10 : 0) +
        (Array.isArray(item.forms) ? item.forms.length * 6 : 0) +
        (item.signature && item.signature !== item.name ? 4 : 0) +
        (item.effect ? 3 : 0) +
        (item.help ? 2 : 0) +
        (item.category ? 1 : 0);
}

function _programHelpLookupShouldReplace(norm, existing, next) {
    const existingExact = _normalizeProgramHelpToken(existing && existing.name) === norm;
    const nextExact = _normalizeProgramHelpToken(next && next.name) === norm;
    if (nextExact && !existingExact) return true;
    if (existingExact && !nextExact) return false;
    return _programHelpLookupScore(next) > _programHelpLookupScore(existing);
}

function _paramProfileSource() {
    return (((_programProfiles || {}).profiles || {}).param || {}).source || {};
}

function _paramHelpFormsArticle(name, signature, help, forms, options = {}) {
    return _programHelpItem(name, signature, help, {
        ...options,
        forms,
        examples: options.examples || forms.slice(0, 2),
    });
}

function _paramProgramLegacyCallParams(name) {
    return [
        { ph: 'src', def: 'both', choices: ['p1', 'p2', 'both', 'pop1', 'pop2'], selectorWide: true, title: 'Source selector for the legacy transform.' },
        { ph: 'tgt', def: 'both', choices: ['p1', 'p2', 'both', 'push1', 'push2'], selectorWide: true, title: 'Target selector for the legacy transform.' },
    ].concat(_paramRegistryAdapter.params(name));
}

function _paramProgramLegacyVariableFormNotes(name) {
    const spec = _paramRegistryAdapter.variableForms(name);
    if (!spec || !spec.counts.length) return [];
    const notes = [`Accepted legacy arg counts: ${spec.counts.join(', ')}.`];
    spec.forms.forEach((form, idx) => {
        const count = spec.counts[idx];
        if (Number.isFinite(count) && form) notes.push(`${count} args: ${form}.`);
    });
    return notes;
}

function _paramProgramGrammarHelpNameSet() {
    const source = _paramProfileSource();
    const names = new Set([
        'assignment', 'push', 'const', 'emit', 'macro',
        't1', 't2', 'p1', 'p2', 'pi', 'pi2', 'pi2i',
    ]);
    Object.keys(source.emit_aliases || {}).forEach(name => names.add(name));
    Object.keys(source.stack_op_aliases || {}).forEach(name => names.add(name));
    Object.values(source.stack_op_aliases || {}).forEach(name => names.add(name));
    (source.binary_ops || []).forEach(name => names.add(name));
    (source.unary_ops || []).forEach(name => names.add(name));
    (source.targetable_unary || []).forEach(name => names.add(name));
    return names;
}

function _paramProgramLegacyHelpAliases(name) {
    return _paramProgramGrammarHelpNameSet().has(String(name || '').toLowerCase())
        ? []
        : [name];
}

function _paramProgramLegacyHelpItem(name) {
    const spec = _paramProgramLegacyUiSpec(name);
    const params = _paramProgramLegacyCallParams(name);
    const sourceArgs = ['both', 'both'].concat((params.slice(2) || []).map((param, idx) => _programHelpParamDefault(param) || _programHelpParamName(param, idx)));
    const notes = Array.isArray(spec.notes) ? spec.notes.slice() : [];
    notes.push(..._paramProgramLegacyVariableFormNotes(name));
    notes.push('legacy(...) arguments are positional; use values like both, p1, p2 directly, not keyword syntax such as src=both.');
    return _programHelpItem(
        `legacy:${name}`,
        `legacy(${name}, src, tgt, ...)`,
        spec.desc || 'Legacy parameter transform with explicit source/target selectors.',
        {
            aliases: _paramProgramLegacyHelpAliases(name),
            category: `legacy transform${spec.category ? ': ' + spec.category : ''}`,
            params,
            effect: spec.effect || '',
            forms: [`legacy(${name}${sourceArgs.length ? ', ' + sourceArgs.join(', ') : ''})`],
            examples: [_paramProgramLegacySnippet(name)],
            notes,
        },
    );
}

function _coeffNativeTransformHelpItem(name) {
    const spec = _coeffRegistryAdapter.spec(name);
    const sourceName = _coeffRegistrySourceName(name);
    const params = _coeffTransformParams(name);
    const argText = params.map((param, idx) => _programHelpParamName(param, idx)).join(', ');
    const defaults = params.map((param, idx) => _programHelpParamDefault(param) || _programHelpParamName(param, idx));
    const form = `poly = ${sourceName}(poly${defaults.length ? ', ' + defaults.join(', ') : ''})`;
    const signature = `poly = ${sourceName}(poly${argText ? ', ' + argText : ''})`;
    return _programHelpItem(
        sourceName,
        signature,
        spec.desc || 'Native coefficient transform.',
        {
            aliases: sourceName === name ? [] : [name],
            category: spec.category || 'native',
            params,
            forms: [form],
            examples: [_coeffNativeTransformSnippet(name)],
        },
    );
}

function _programHelpBuildParamRegistry() {
    const registry = _newProgramHelpRegistry();
    const source = _paramProfileSource();
    if (!_paramRegistryAdapter.loaded) {
        _programHelpAddSection(registry, 'Registry Status', [
            _programHelpItem('registry', 'Param registry not loaded', 'Param registry-backed Help is unavailable; only built-in grammar Help may render.', { missing: true }),
        ]);
    }
    _programHelpAddSection(registry, 'Core Symbols', [
        _programHelpItem('t1', 't1', 'Input parameter 1.'),
        _programHelpItem('t2', 't2', 'Input parameter 2.'),
        _programHelpItem('p1', 'p1', 'Output/current parameter register 1.'),
        _programHelpItem('p2', 'p2', 'Output/current parameter register 2.'),
        _programHelpItem('r1', 'r1 .. r8', 'Mutable scratch registers (complex), zeroed at the start of every evaluation. Unlike write-once local aliases, they rebind: r1 = r1 * r1 is valid. Reading before any write gives 0.'),
        _programHelpItem('pi', 'pi', 'π constant.'),
        _programHelpItem('pi2', 'pi2', '2π constant.'),
        _programHelpItem('pi2i', 'pi2i', '2πi complex constant.'),
    ]);
    _programHelpAddSection(registry, 'Assignments + I/O', [
        _paramHelpFormsArticle('assignment', 'p1 = expr / p2 = expr', 'Assign a complex expression to p1 or p2.', ['p1 = expr', 'p2 = expr'], {
            aliases: ['p1', 'p2'],
            category: 'assignment',
            effect: 'push expression value, then emit to the selected output register',
        }),
        _paramHelpFormsArticle('push', 'push() / push(t1) / push(t2)', 'Push an input value onto the Param stack.', ['push()', 'push(t1)', 'push(t2)'], {
            category: 'input/output',
            params: [{ name: 'src', def: 't1', choices: source.push_sources || ['t1', 't2'], title: 'Optional input source; both is not valid here.' }],
            effect: '(-- z)',
            notes: ['Only the default input, t1, and t2 are valid push sources. Push t1 and t2 separately when you need both values.'],
        }),
        _paramHelpFormsArticle('const', 'const(expr)', 'Push a complex expression value.', ['const(expr)'], {
            category: 'input/output',
            params: [{ name: 'expr', title: 'Complex expression over t1, t2, p1, p2, pi, pi2, pi2i.' }],
            effect: '(-- z)',
        }),
        (() => {
            const emitAliases = source.emit_aliases || {};
            const emitForms = Object.keys(emitAliases);
            const emitTargets = Array.from(new Set(Object.values(emitAliases))).filter(Boolean);
            return _paramHelpFormsArticle('emit', emitForms.join(' / ') || 'emit', 'Pop one stack value into p1 or p2.', emitForms.length ? emitForms : ['emit_p1', 'emit_p2'], {
                aliases: emitForms,
                category: 'input/output',
                params: [{ name: 'target', choices: emitTargets.length ? emitTargets : ['p1', 'p2'] }],
                effect: '(z -- ), writes target register',
                notes: ['Use generated emit aliases or assignment syntax for targeted output.'],
            });
        })(),
    ]);
    const stackAliases = source.stack_op_aliases || {};
    const stackItems = Array.from(new Set(Object.values(stackAliases))).map(name => {
        const forms = Object.entries(stackAliases)
            .filter(([, canonical]) => canonical === name)
            .map(([alias]) => alias);
        return _paramHelpFormsArticle(
            name,
            forms.join(' / ') || name,
            `${name} stack operation.`,
            forms.length ? forms : [name],
            { aliases: forms.filter(form => form !== name), category: 'stack', effect: 'stack operation' },
        );
    });
    _programHelpAddSection(registry, 'Stack', stackItems);
    const binaryItems = (Array.isArray(source.binary_ops) ? source.binary_ops : [])
        .map(name => _paramHelpFormsArticle(
            name,
            name,
            `${name} binary stack operation.`,
            [name],
            { category: 'arithmetic', effect: '(a b -- result)' },
        ));
    _programHelpAddSection(registry, 'Arithmetic', binaryItems);
    const unaryItems = (Array.isArray(source.unary_ops) ? source.unary_ops : [])
        .map(name => {
            const targetable = (source.targetable_unary || []).includes(name);
            const targetForms = targetable
                ? (source.unary_targets || ['p1', 'p2']).map(target => `${name}(${target})`)
                : [];
            return _paramHelpFormsArticle(
                name,
                `${name}${targetForms.length ? '; ' + targetForms.join('; ') : ''}`,
                `${name} transforms the top stack value, or directly mutates p1/p2 in targeted form.`,
                [name].concat(targetForms),
                {
                    category: 'unary transform',
                    params: [{ name: 'target', choices: source.unary_targets || ['p1', 'p2'], title: 'Optional targeted-register form.' }],
                    effect: '(z -- f(z)); targeted form writes p1/p2 without using the stack',
                },
            );
        });
    _programHelpAddSection(registry, 'Unary', unaryItems);
    const rejectedItems = (Array.isArray(source.rejected_forms) ? source.rejected_forms : [])
        .map(item => _programHelpItem(
            item.form || item.code || 'rejected form',
            item.form || item.code || 'rejected form',
            `Rejected Param source form${item.code ? ' (' + item.code + ')' : ''}.`,
            {
                category: 'rejected form',
                forms: item.form ? [item.form] : [],
                notes: item.use ? [`Use ${item.use}.`] : [],
                lookup: false,
            },
        ));
    _programHelpAddSection(registry, 'Rejected Forms', rejectedItems);
    _programHelpAddSection(registry, 'Macro', [
        _paramHelpFormsArticle('macro', 'macro(name)', 'Inline a saved Param Program at compile time.', ['macro(name)'], {
            category: 'macro',
            params: [{ name: 'name', title: 'Saved Param Program id.' }],
        }),
    ]);
    const legacyItems = (_paramProgramLegacyNames || [])
        .filter(name => name && name !== 'none')
        .map(name => _paramProgramLegacyHelpItem(name));
    _programHelpAddSection(registry, 'Legacy Transform Reference', legacyItems);
    return registry;
}

function _coeffTransformParams(name) {
    return _coeffRegistryAdapter.params(name);
}

function _programHelpBuildCoeffRegistry() {
    const registry = _newProgramHelpRegistry();
    if (!_coeffRegistryAdapter.loaded) {
        _programHelpAddSection(registry, 'Registry Status', [
            _programHelpItem('registry', 'Coeff registry not loaded', 'Coeff registry-backed help is unavailable; starter snippets and coefficient-function params may still render.', { missing: true }),
        ]);
    }
    _programHelpAddSection(registry, 'Core Symbols', [
        _programHelpItem('poly_len', 'poly_len', 'Length of the current coefficient vector.'),
        _programHelpItem('tos', 'tos[i]', 'Read one element from the top stack vector.'),
        _programHelpItem('t1', 't1', 'Input parameter 1 in scalar expressions.'),
        _programHelpItem('t2', 't2', 'Input parameter 2 in scalar expressions.'),
        _programHelpItem('p1', 'p1', 'Param Program output/register 1 in scalar expressions.'),
        _programHelpItem('p2', 'p2', 'Param Program output/register 2 in scalar expressions.'),
        _programHelpItem('andy', 'andy', 'Optional native-transform blend weight: 0 means pure transform, 1 means keep the input vector.', {
            category: 'native transform parameter',
            forms: ['poly = transform(poly, ..., andy)'],
        }),
    ]);
    _programHelpAddSection(registry, 'Language Rules', [
        _programHelpItem('infix', 'scalar infix: a + b, a * b, a**n', 'Infix arithmetic works on SCALARS only: p1, p2, t1, t2, literals, and element reads like poly[i] / cf[i] / tos[i]. ** needs an integer literal exponent (max 32); -x**n is rejected as ambiguous — write -(x**n).', {
            forms: ['poly[0] = p1 * p2 + 1', 'poly[10] = poly[10] / abs(poly[10])'],
        }),
        _programHelpItem('vector-math', 'vector math: multiply(a, b), add(a, b), ...', 'Vectors (poly, cf, slices like poly[29:40], range/fill results, pop) NEVER use infix — combine them with the call forms add / subtract / multiply / divide / power / ge / gt / le / lt / eq / rem / ipow. Scalar arguments broadcast: multiply(poly[29:40], 2.5) scales a window in place.', {
            forms: ['poly[29:40] = multiply(poly[29:40], 2.5)', 'poly = add(power(range(1, 37), 2), multiply(range(1, 37), p1 * p2))'],
        }),
        _programHelpItem('locals', 'name = expr', 'Write-once local alias, substituted at compile time. Any unreserved name; usable in expressions, call arguments, and indexes.', {
            forms: ['gain = abs(p1 - p2)**2 + 1', 'poly[29:40] = divide(abs(poly[29:40]), gain)'],
        }),
    ]);
    _programHelpAddSection(registry, 'Vectors + Windows', [
        _programHelpItem('vector_literal', 'vector_literal(c0, c1, ...)', 'Create a leading-first coefficient vector from static expressions. Values are compiled once into a deduplicated immutable pool; they are not rebuilt per row.', {
            forms: ['poly = vector_literal(1, -3, 2)'],
            params: [{ name: 'c0..cn', title: 'One or more finite static complex expressions, leading coefficient first.' }],
            effect: '(-- vector)',
        }),
        _programHelpItem('roots_literal', 'roots_literal(r0, r1, ...)', 'Expand the monic polynomial whose roots are these static expressions ONCE at compile time (exact rational arithmetic) into the same constant pool as vector_literal — the program source shows the root layout instead of expanded coefficients. Double-click the roots_literal name to arrange the roots geometrically on the root pad (plain literals only).', {
            forms: ['poly = roots_literal(1, 2)', 'poly = roots_literal(-8.5+3i, -7.5+3i, 8.5+0i)'],
            params: [{ name: 'r0..rk', title: 'One or more finite static complex roots (up to 255). The pushed vector has k+1 leading-first coefficients with leading coefficient 1.' }],
            effect: '(-- vector)',
        }),
        _programHelpItem('roots_chess_literal', 'roots_chess_literal(d, w, o)', 'The dark cells of a d x d chessboard (corners dark), expanded once at compile time into the constant pool. w is the FULL side of the board; o is its complex center. The parameters are plain literals: scrub w on the 1D pad and o on the 2D pad.', {
            forms: ['poly = roots_chess_literal(5, 1, 0)', 'poly = roots_chess_literal(5, 1, 1+1i)'],
            params: [
                { name: 'd', title: 'Board dimension, integer 1..22 (dark-cell count stays within the 255-root cap).' },
                { name: 'w', title: 'Full side length of the board, positive real.' },
                { name: 'o', title: 'Complex center of the board.' },
            ],
            effect: '(-- vector)',
        }),
        _programHelpItem('roots_grid_literal', 'roots_grid_literal(d, w, o)', 'The full d x d root lattice: every cell of the board, side w, centered on o. Expanded once at compile time into the constant pool.', {
            forms: ['poly = roots_grid_literal(4, 1, 0)'],
            params: [
                { name: 'd', title: 'Lattice dimension, integer 1..15 (d*d roots within the 255-root cap).' },
                { name: 'w', title: 'Full side length, positive real.' },
                { name: 'o', title: 'Complex center.' },
            ],
            effect: '(-- vector)',
        }),
        _programHelpItem('roots_ring_literal', 'roots_ring_literal(n, r, o)', 'n roots evenly spaced on a circle: o + r*exp(2*pi*i*k/n), expanded once at compile time. A COMPLEX r rotates the ring; cardinal angles are exact (ring(4, 1, 0) is exactly z^4 - 1).', {
            forms: ['poly = roots_ring_literal(7, 1, 0)', 'poly = roots_ring_literal(6, 0.5i, -1)'],
            params: [
                { name: 'n', title: 'Point count, integer 1..255.' },
                { name: 'r', title: 'Nonzero complex radius; the argument of r rotates the ring.' },
                { name: 'o', title: 'Complex center.' },
            ],
            effect: '(-- vector)',
        }),
        _programHelpItem('translate_roots', 'translate_roots(coefficients, delta)', 'Shift every root by delta using coefficient translation Q(z) = P(z-delta). This does not solve for roots and preserves the coefficient-vector length.', {
            forms: ['poly = translate_roots(poly, 0.1*exp(pi2i*t1))'],
            params: [
                { name: 'coefficients', title: 'Coefficient vector in leading-first order.' },
                { name: 'delta', title: 'Finite complex scalar expression added to every root.' },
            ],
            effect: '(vector scalar -- vector)',
        }),
        _programHelpItem('bimodal', 'bimodal(u, a)', 'Map u in [0,1] symmetrically toward the endpoints. a=0 is uniform; increasing a toward 1 makes the two endpoint modes sharper.', {
            forms: ['poly = blend(bimodal(t2, 0.7))'],
            params: [
                { name: 'u', title: 'Finite real scalar in [0,1].' },
                { name: 'a', title: 'Finite real shape in [0,1).' },
            ],
        }),
        _programHelpItem('slice', 'poly[a:b] / cf[a:b]', 'Slice read (a vector of length b-a; b exclusive) and slice write. The written value must be a VECTOR of exactly b-a elements — use fill(b-a, value) to broadcast a scalar.', {
            forms: ['poly[2:7] = multiply(poly[2:7], 5)', 'sum(poly[0:70])'],
        }),
        _programHelpItem('scan', 'scan(len, k0, init, step)', 'Bounded recurrence: out[0] = init at k = k0; out[j] = step with prev = previous element (prev2 with the five-arg form scan(len, k0, init1, init2, step)) and k = k0 + j. init/step are scalar expressions.', {
            forms: ['poly = scan(36, 1, p1 + p2, (sin(k*prev) + 1) / abs(sin(k*prev) + 1))'],
        }),
        _programHelpItem('sum', 'sum(vector) / prod(vector)', 'Reductions: pop a vector, push the scalar total/product. Read the result back with tos[0] in the next poke, then drop.', {
            forms: ['sum(poly[0:70])', 'poly[70] = tos[0] + 1i*p1', 'drop'],
        }),
        _programHelpItem('window', 'window(a, b) / step(a)', 'Exact 0/1 masks over the current poly length: window keeps slots [a, b), step keeps slots >= a. Multiply a vector by a mask to confine an effect. Arguments must be pure (no pop/tos).', {
            forms: ['poly = multiply(poly, window(3, 6))'],
        }),
        _programHelpItem('select', 'select(cond, a, b)', 'Elementwise choice: cond*a + (1-cond)*b. Build cond with the comparison ops (eq/ge/gt/le/lt, exact 0/1 on real parts) and rem for parity. Arguments must be pure (no pop/tos).', {
            forms: ['parity = rem(range(1, 72), 2)', 'poly = select(eq(rem(range(1, 72), 2), 0), poly, neg(poly))'],
        }),
    ]);
    _programHelpAddSection(registry, 'Statement Forms', [
        _programHelpItem('cf', 'cf', 'Push the immutable input coefficient vector.', { forms: ['cf'], effect: '(-- vector)' }),
        _programHelpItem('poly', 'poly', 'Push the current/output coefficient vector.', { forms: ['poly', 'poly = expr', 'poly[i] = expr'], effect: 'read/write the current vector' }),
        _programHelpItem('emit', 'emit', 'Commit the top vector as output poly.', { forms: ['emit'], effect: '(vector -- ), writes poly' }),
        _programHelpItem('macro', 'macro(name)', 'Inline a saved Coeff Program at compile time.', { forms: ['macro(name)'], params: [{ name: 'name' }] }),
    ]);
    _programHelpAddSection(registry, 'Vector Ops', [
        ..._coeffFamilySubOps('vector_binary').map(op => _programHelpItem(
            op.name,
            `${op.name}(left, right)`,
            `${op.name} combines two vector/scalar sources.`,
            { aliases: op.source_aliases || [], forms: [`poly = ${op.name}(poly, p1)`], effect: '(left right -- result)' },
        )),
        ..._coeffFamilySubOps('vector_unary').map(op => {
            const nativeParams = _coeffTransformParams(op.name);
            const nativeSourceName = nativeParams.length ? _coeffRegistrySourceName(op.name) : op.name;
            const nativeForm = nativeParams.length
                ? `poly = ${nativeSourceName}(poly, ${nativeParams.map((param, idx) => _programHelpParamDefault(param) || _programHelpParamName(param, idx)).join(', ')})`
                : '';
            return _programHelpItem(
                op.name,
                `${op.name}(source${nativeParams.length ? ', andy' : ''})`,
                `${op.name} applies elementwise to a vector/scalar source${nativeParams.length ? '; native form also accepts andy blending.' : '.'}`,
                {
                    aliases: op.source_aliases || [],
                    forms: [`poly = ${op.name}(poly)`].concat(nativeForm ? [nativeForm] : []),
                    params: nativeParams,
                    effect: '(source -- result)',
                },
            );
        }),
    ]);
    const vectorUnaryNames = new Set(_coeffFamilySubOps('vector_unary').map(op => op && op.name).filter(Boolean));
    const nativeItems = (_coeffProgramLegacyNames || [])
        .filter(name => !vectorUnaryNames.has(_coeffRegistrySourceName(name)))
        .map(name => _coeffNativeTransformHelpItem(name));
    _programHelpAddSection(registry, 'Native Transform Reference', nativeItems);
    const functionCatalog = (typeof window !== 'undefined' && window._coeffFuncCatalog) || [];
    const functionItems = functionCatalog
        .filter(entry => Array.isArray(entry.params) && entry.params.length)
        .map(entry => _programHelpItem(
            entry.name,
            `${entry.name}(${entry.params.map((param, idx) => _programHelpParamText(param, idx)).join(', ')})`,
            `Coefficient-function parameters from coeff_func_catalog.json. kind=${entry.kind || 'unknown'} source=${entry.source || 'unknown'}.`,
            { category: 'coefficient function', params: entry.params },
        ));
    _programHelpAddSection(registry, 'Coefficient Function Params', functionItems);
    return registry;
}

function _rootTransformHelpItem(name) {
    const spec = _rootRegistryAdapter.spec(name);
    const params = _rootRegistryAdapter.params(name);
    const argText = params.map((param, idx) => _programHelpParamName(param, idx)).join(', ');
    const defaults = params.map((param, idx) => _programHelpParamDefault(param) || _programHelpParamName(param, idx));
    const form = `${name}(${defaults.join(', ')})`;
    return _programHelpItem(
        name,
        `${name}(${argText})`,
        spec.desc || 'Root transform.',
        {
            category: `root transform${spec.label ? ': ' + spec.label : ''}`,
            params,
            forms: [form, `roots = ${name}(roots${defaults.length ? ', ' + defaults.join(', ') : ''})`],
            examples: [form],
        },
    );
}

function _solveScoreOpHelpItem(name, spec, category) {
    const params = (spec && spec.params) || [];
    const argNames = params.map((param, idx) => _programHelpParamName(param, idx));
    const arity = Number(spec && spec.arity) || 0;
    const stackArgs = Array.from({ length: arity }, (_, i) => (arity === 1 ? 'expr' : `expr${i + 1}`));
    const signature = `${name}(${stackArgs.concat(argNames).join(', ')})`;
    const snippet = String((spec && spec.snippet) || '');
    return _programHelpItem(name, signature, (spec && spec.tooltip) || '', {
        category,
        params,
        effect: arity ? `stack ${arity} -> ${category === 'combine' ? 1 : arity}` : '',
        examples: snippet ? [snippet] : [],
    });
}

function _programHelpBuildSolveScoreRegistry() {
    const registry = _newProgramHelpRegistry();
    if (!_solveScoreMetricNames.length) {
        _programHelpAddSection(registry, 'Registry Status', [
            _programHelpItem('registry', 'Solve-score vocabulary not loaded', 'solve_score_vocab_js.js did not load; solve-score Help is unavailable.', { missing: true }),
        ]);
        return registry;
    }
    const vocab = _solveScoreVocab || {};
    const qRange = Array.isArray(vocab.quantilePercentRange) ? vocab.quantilePercentRange : [];
    const lagDepths = Array.isArray(vocab.lagDepths) ? vocab.lagDepths : [];
    const sources = Array.isArray(vocab.sourceNames) ? vocab.sourceNames : [];
    _programHelpAddSection(registry, 'Statement Forms', [
        _programHelpItem('score', 'score = expr', 'Assign the score expression; the last assignment is the program result.', {
            category: 'statement form',
            forms: ['score = metric(proximity, slv, q=0.1%)'],
            aliases: ['assignment'],
        }),
        _programHelpItem('metric', `${_solveScoreGenericMetricPublicName}(name, src, q=..%)`, 'Evaluate one score metric from a solve/coeff/param source.', {
            category: 'statement form',
            params: [
                { name: 'name', title: 'Metric name; see the Metrics reference below.' },
                { name: 'src', choices: sources, title: 'Metric source; each metric allows a subset.' },
                { name: 'q', def: '0.1%', title: qRange.length === 2 ? `Quantile percent in [${qRange[0]}, ${qRange[1]}].` : 'Quantile percent.' },
                { name: 'lag', def: '0', title: lagDepths.length ? `Optional keyword; frame lag depth (${lagDepths.join(' or ')}).` : 'Optional keyword; frame lag depth.' },
            ],
            forms: ['metric(proximity, slv, q=0.1%)', 'metric(proximity, slv, q=0.1%, lag=1)'],
        }),
        _programHelpItem('push', 'push(expr)', 'Push a score expression onto the stack.', {
            category: 'statement form',
            forms: ['push(metric(proximity, slv, q=0.1%))'],
        }),
    ]);
    const metricItems = _solveScoreMetricNames.map(name => {
        const allowed = _solveScoreMetricAllowedSources(name);
        return _programHelpItem(
            name,
            `metric(${name}, ${allowed.join('|') || 'slv'}, q=..%)`,
            _solveScoreMetricDescriptions[name]
                || (_solveScoreParamMetricSet.has(name) ? 'Param-sourced metric.' : 'Root/coefficient metric.'),
            {
                category: 'metric',
                notes: allowed.length ? [`Sources: ${allowed.join(', ')}.`] : [],
                examples: [_solveScoreMetricSnippet(name)],
            },
        );
    });
    _programHelpAddSection(registry, 'Metrics', metricItems);
    _programHelpAddSection(
        registry,
        'Unary / Stack',
        Object.keys(_solveScoreUnarySpecs).map(name => _solveScoreOpHelpItem(name, _solveScoreUnarySpecs[name], 'unary/stack')),
    );
    _programHelpAddSection(
        registry,
        'Combine',
        Object.keys(_solveScoreCombineSpecs).map(name => _solveScoreOpHelpItem(name, _solveScoreCombineSpecs[name], 'combine')),
    );
    _programHelpAddSection(
        registry,
        'Outputs',
        Object.keys(_solveScoreOutputSpecs).map(name => _solveScoreOpHelpItem(name, _solveScoreOutputSpecs[name], 'output')),
    );
    return registry;
}

function _programHelpBuildRootRegistry() {
    const registry = _newProgramHelpRegistry();
    if (!_rootRegistryAdapter.loaded) {
        _programHelpAddSection(registry, 'Registry Status', [
            _programHelpItem('registry', 'Root registry not loaded', 'root_vocab_js.js did not load; root transform Help is unavailable.', { missing: true }),
        ]);
        return registry;
    }
    const cap = _rootRegistryAdapter.maxStatements;
    _programHelpAddSection(registry, 'Statement Forms', [
        _programHelpItem('call', 'fn(args)', 'Apply a transform to all roots, in place, one statement per line.', {
            category: 'statement form',
            forms: ['rotate_roots(0.25)', 'roots_toline()'],
            notes: [
                'Arguments are static finite real numbers; expressions and registers are not available.',
                'Parentheses are required even with no arguments.',
            ].concat(cap ? [`At most ${cap} statements per program.`] : []),
            lookup: false,
        }),
        _programHelpItem('roots', 'roots = fn(roots, args)', 'Assignment form; identical to the bare call.', {
            category: 'statement form',
            forms: ['roots = rotate_roots(roots, 0.25)'],
            notes: ['roots is the only assignable symbol; transforms mutate it in place.'],
        }),
    ]);
    _programHelpAddSection(
        registry,
        'Root Transform Reference',
        _rootRegistryAdapter.names.map(name => _rootTransformHelpItem(name)),
    );
    return registry;
}

function _programHelpRegistry(which) {
    const key = _programHelpRegistryKey(_programSourceWhichKey(which));
    if (!_programHelpRegistryCache[key]) {
        const builders = {
            cp: _programHelpBuildCoeffRegistry,
            rt: _programHelpBuildRootRegistry,
            ss: _programHelpBuildSolveScoreRegistry,
            pp: _programHelpBuildParamRegistry,
        };
        _programHelpRegistryCache[key] = (builders[key] || _programHelpBuildParamRegistry)();
    }
    return _programHelpRegistryCache[key];
}

function _programHelpParamsHtml(params) {
    const rows = (params || []).map((param, idx) => {
        const text = _programHelpParamText(param, idx);
        const help = param && (param.help || param.title) ? ` title="${_escapeHtml(param.help || param.title)}"` : '';
        return `<span class="program-help-param"${help}>${_escapeHtml(text)}</span>`;
    });
    return rows.length ? `<div class="program-help-meta">${rows.join('')}</div>` : '';
}

function _programHelpParamDetailsHtml(params) {
    const rows = (params || [])
        .map((param, idx) => {
            const detail = String((param && (param.help || param.title)) || '').trim();
            if (!detail) return '';
            return `<div class="program-help-meta"><strong>${_escapeHtml(_programHelpParamText(param, idx))}:</strong> ${_escapeHtml(detail)}</div>`;
        })
        .filter(Boolean);
    return rows.length ? rows.join('') : '';
}

function _programHelpListHtml(title, values) {
    const rows = (values || []).filter(Boolean).map(value => `<code>${_escapeHtml(value)}</code>`);
    return rows.length ? `<div class="program-help-meta"><strong>${_escapeHtml(title)}:</strong> ${rows.join(' ')}</div>` : '';
}

function _programHelpItemHtml(which, item) {
    const safeSignature = _escapeHtml(item.signature || item.name || '');
    const safeCategory = item.category ? `<div class="program-help-meta">${_escapeHtml(item.category)}</div>` : '';
    const safeHelp = item.help ? `<div class="program-help-meta">${_escapeHtml(item.help)}</div>` : '';
    const forms = _programHelpListHtml('Forms', item.forms || []);
    const params = _programHelpParamsHtml(item.params || []);
    const paramDetails = _programHelpParamDetailsHtml(item.params || []);
    const effect = item.effect ? `<div class="program-help-meta"><strong>Effect:</strong> ${_escapeHtml(item.effect)}</div>` : '';
    const examples = _programHelpListHtml('Examples', item.examples || []);
    const notes = (item.notes || []).filter(Boolean).map(note => `<div class="program-help-meta">${_escapeHtml(note)}</div>`).join('');
    const cls = item.missing ? ' program-help-item-missing' : '';
    return `<div class="program-help-item${cls}"><div class="program-help-signature">${safeSignature}</div>${safeCategory}${safeHelp}${forms}${params}${paramDetails}${effect}${examples}${notes}</div>`;
}

function _programHelpSectionHtml(which, section) {
    const items = (section.items || []).map(item => _programHelpItemHtml(which, item)).join('');
    if (!items) return '';
    return `<div class="program-source-cheat-section"><div class="program-source-cheat-title">${_escapeHtml(section.title)}</div>${items}</div>`;
}

function _renderProgramSourceHelp(which) {
    const key = _programSourceWhichKey(which);
    const el = document.getElementById(`${key}-help`);
    if (!el) return;
    const registry = _programHelpRegistry(key);
    el.innerHTML = (registry.sections || []).map(section => _programHelpSectionHtml(key, section)).join('');
}

function _setProgramSourceSidePanelMode(which, mode) {
    const key = _programSourceWhichKey(which);
    _programSourceSidePanelMode[key] = mode === 'help' ? 'help' : 'starter';
    _renderProgramSourceSidePanel(key);
}

function _renderProgramSourceSidePanel(which) {
    const key = _programSourceWhichKey(which);
    _renderProgramSourceHelp(key);
    const mode = _programSourceSidePanelMode[key] || 'starter';
    const starter = document.getElementById(`${key}-cheatsheet`);
    const help = document.getElementById(`${key}-help`);
    const starterTab = document.getElementById(`${key}-help-tab-starter`);
    const helpTab = document.getElementById(`${key}-help-tab-help`);
    if (starter) starter.hidden = mode !== 'starter';
    if (help) help.hidden = mode !== 'help';
    if (starterTab && starterTab.classList) starterTab.classList.toggle('active', mode === 'starter');
    if (helpTab && helpTab.classList) helpTab.classList.toggle('active', mode === 'help');
}

function _renderProgramSourceSidePanels() {
    _renderProgramSourceSidePanel('pp');
    _renderProgramSourceSidePanel('cp');
    _renderProgramSourceSidePanel('rt');
    _renderProgramSourceSidePanel('prt');
    _renderProgramSourceSidePanel('render-ss');
    _renderProgramSourceSidePanel('palette-ss');
}

function _programSourceTextarea(which) {
    if (which === 'rt') return _rootProgramSourceTextarea('render');
    if (which === 'prt') return _rootProgramSourceTextarea('palette');
    if (which === 'render-ss') return _solveScoreSourceTextarea('render');
    if (which === 'palette-ss') return _solveScoreSourceTextarea('palette');
    return which === 'cp' ? _coeffProgramSourceTextarea() : _paramProgramSourceTextarea();
}

function _normalizeProgramHelpToken(token) {
    let raw = String(token || '').trim();
    if (!raw) return '';
    const paren = raw.indexOf('(');
    if (paren > 0) raw = raw.slice(0, paren);
    const bracket = raw.indexOf('[');
    if (bracket > 0) raw = raw.slice(0, bracket);
    raw = raw.replace(/^[^A-Za-z0-9_]+|[^A-Za-z0-9_]+$/g, '');
    return raw.toLowerCase();
}

function _programWordAtTextareaCursor(textarea) {
    if (!textarea) return '';
    const value = String(textarea.value || '');
    const start = Number.isFinite(textarea.selectionStart) ? textarea.selectionStart : 0;
    const end = Number.isFinite(textarea.selectionEnd) ? textarea.selectionEnd : start;
    const selected = start !== end ? value.slice(start, end) : '';
    if (selected.trim()) return selected.trim();
    let lo = Math.max(0, start);
    let hi = Math.max(0, start);
    const isToken = ch => /[A-Za-z0-9_\[\].]/.test(ch || '');
    while (lo > 0 && isToken(value[lo - 1])) lo--;
    while (hi < value.length && isToken(value[hi])) hi++;
    return value.slice(lo, hi).trim();
}

function _lookupProgramHelpToken(which, token) {
    const norm = _normalizeProgramHelpToken(token);
    if (!norm) return null;
    const registry = _programHelpRegistry(_programSourceWhichKey(which));
    return registry.lookup.get(norm) || null;
}

function _ensureProgramHelpInspectorHandlers() {
    if (_programHelpInspectorBound || typeof document === 'undefined' || typeof document.addEventListener !== 'function') return;
    document.addEventListener('keydown', event => {
        if (event && event.key === 'Escape') _closeProgramHelpInspector();
    });
    document.addEventListener('click', event => {
        const el = document.getElementById('program-help-inspector');
        if (!el || el.style.display === 'none' || !event || !event.target) return;
        if (typeof el.contains === 'function' && el.contains(event.target)) return;
        _closeProgramHelpInspector();
    });
    _programHelpInspectorBound = true;
}

function _programHelpInspectorContent(which, token, item) {
    if (!item) {
        return `
            <div class="program-help-inspector-head">
                <div class="program-help-inspector-title">${_escapeHtml(token || 'Unknown token')}</div>
                <button type="button" class="program-help-inspector-close" onclick="_closeProgramHelpInspector()" aria-label="Close">x</button>
            </div>
            <div class="program-help-meta">No generated help for "${_escapeHtml(token || '')}". Try the Help tab for available symbols.</div>
        `;
    }
    return `
        <div class="program-help-inspector-head">
            <div class="program-help-inspector-title">${_escapeHtml(item.name || token || '')}</div>
            <button type="button" class="program-help-inspector-close" onclick="_closeProgramHelpInspector()" aria-label="Close">x</button>
        </div>
        ${_programHelpItemHtml(which, item)}
    `;
}

function _openProgramHelpInspector(which, token, item, event) {
    const el = document.getElementById('program-help-inspector');
    if (!el) return;
    _ensureProgramHelpInspectorHandlers();
    el.innerHTML = _programHelpInspectorContent(which, token, item);
    el.style.display = 'block';
    if (typeof el.setAttribute === 'function') el.setAttribute('aria-hidden', 'false');
    const raw = event || {};
    const pinned = _scrubPadLastPos;
    const x = pinned ? pinned.x : (Number(raw.clientX || 0) || 12);
    const y = pinned ? pinned.y - 14 : (Number(raw.clientY || 0) || 12);
    el.style.left = `${Math.max(8, x)}px`;
    el.style.top = `${Math.max(8, y)}px`;
    const rect = typeof el.getBoundingClientRect === 'function' ? el.getBoundingClientRect() : { width: 320, height: 220 };
    const vw = (typeof window !== 'undefined' && window.innerWidth) || 1200;
    const vh = (typeof window !== 'undefined' && window.innerHeight) || 800;
    el.style.left = `${Math.max(8, Math.min(x, vw - Number(rect.width || 320) - 8))}px`;
    el.style.top = `${Math.max(8, Math.min(y, vh - Number(rect.height || 220) - 8))}px`;
}

function _closeProgramHelpInspector() {
    const el = document.getElementById('program-help-inspector');
    if (!el) return;
    el.style.display = 'none';
    if (typeof el.setAttribute === 'function') el.setAttribute('aria-hidden', 'true');
    el.innerHTML = '';
}

/* ---- Program scrub pad: dblclick a literal to drag-edit it ----
   Two modes over one core: 'number' scrubs a numeric literal across an
   editable range; 'choice' steps a solve-score metric name across the
   metric vocabulary (discrete), showing each metric's description.
   Ephemeral by design: the "binding" is just a text span in one textarea.
   Only the pad writes while open (every write re-checks a full-text
   snapshot), any external edit closes it, Escape reverts. Live preview
   routes ONLY to the lores preview endpoints — the compute preview for
   pp/cp and the render lores preview for rt/render-ss; the palette-tab
   editors have no preview surface so the toggle is hidden.
   NEVER wire this to the full pipeline (compute submit, render
   generate, palette create): far too slow to drive from a drag. */
let _scrubPadState = null;
let _scrubPadLastPos = null;
let _scrubPadHeadDragCleanup = null;
let _scrubPadHandlersBound = false;
let _scrubPreviewTimer = null;
let _scrubPreviewInFlight = false;
let _scrubPreviewDirty = false;

const _scrubPadPreviewByKey = {
    pp: { label: 'live compute preview', run: () => runComputePreview() },
    cp: { label: 'live compute preview', run: () => runComputePreview() },
    rt: { label: 'live render lores preview', run: () => runRenderLoresPreview(), loresViews: true },
    'render-ss': { label: 'live render lores preview', run: () => runRenderLoresPreview(), loresViews: true },
};

const _scrubPadLoresViews = [
    ['plot', 'Plot'],
    ['palette', 'Palette'],
    ['e1', 'E1'],
    ['e2', 'E2'],
    ['e3', 'E3'],
];

function _programTokenSpanAtCursor(textarea) {
    if (!textarea) return null;
    const value = String(textarea.value || '');
    const pos = Number.isFinite(textarea.selectionStart) ? textarea.selectionStart : 0;
    let lo = Math.max(0, Math.min(pos, value.length));
    let hi = lo;
    const isToken = ch => /[A-Za-z0-9_\[\].]/.test(ch || '');
    while (lo > 0 && isToken(value[lo - 1])) lo--;
    while (hi < value.length && isToken(value[hi])) hi++;
    if (hi <= lo) return null;
    return { raw: value.slice(lo, hi), start: lo, end: hi };
}

function _programNumberSpanAtCursor(textarea) {
    const span = _programTokenSpanAtCursor(textarea);
    if (!span) return null;
    const value = String(textarea.value || '');
    let { raw, start, end } = span;
    // A leading minus belongs to the literal when what precedes it is a
    // delimiter (start, comma, open paren, operator, =, whitespace) —
    // t1-5 keeps its binary minus, rotate_roots(-0.25) scrubs -0.25.
    if (start > 0 && value[start - 1] === '-') {
        const before = start >= 2 ? value[start - 2] : '';
        if (!/[A-Za-z0-9_).\]]/.test(before)) {
            start -= 1;
            raw = value.slice(start, end);
        }
    }
    if (!/^-?(\d+\.?\d*|\.\d+)$/.test(raw)) return null;
    const num = Number(raw);
    if (!Number.isFinite(num)) return null;
    return { raw, start, end, value: num };
}

const _SCRUB_NUM = String.raw`(\d+\.?\d*|\.\d+)`;
// The backend expression tokenizer accepts [ijIJ] imaginary suffixes; the
// pad must recognize all of them (generated/pasted source may carry j).
// Writes always normalize to the house-style 'i'.
const _SCRUB_IMAG_RE = new RegExp(`^-?${_SCRUB_NUM}[ijIJ]$`);

function _programComplexSpanAtCursor(textarea) {
    // Complex literals scrub in 2D. Shapes: A+Bi / A-Bi (cursor on either
    // part) and pure-imaginary Bi. Plain reals stay on the 1D pad.
    const span = _programTokenSpanAtCursor(textarea);
    if (!span) return null;
    const value = String(textarea.value || '');
    let { start, end } = span;
    let raw = span.raw;
    const numRe = new RegExp(`^-?${_SCRUB_NUM}$`);
    const leadMinus = (s) => {
        // same delimiter rule as the 1D matcher: t1-5 keeps its binary minus
        if (s > 0 && value[s - 1] === '-' && !/[A-Za-z0-9_).\]]/.test(s >= 2 ? value[s - 2] : '')) return s - 1;
        return s;
    };
    const isExponentSign = (signPos) => {
        // 1.5e+2i is a VALID backend literal (150i) that this pad does not
        // model; a sign directly after e/E (itself after a digit or dot)
        // must never be treated as the re/im separator.
        let q = signPos;
        while (q > 0 && value[q - 1] === ' ') q--;
        const prev = q > 0 ? value[q - 1] : '';
        if (prev !== 'e' && prev !== 'E') return false;
        const prev2 = q > 1 ? value[q - 2] : '';
        return /[0-9.]/.test(prev2);
    };
    if (_SCRUB_IMAG_RE.test(raw)) {
        // cursor on the imaginary part: widen left over [+-] to a real part
        let s = start;
        let realStart = -1;
        let sep = '';
        let probe = s;
        while (probe > 0 && value[probe - 1] === ' ') probe--;
        if (probe > 0 && (value[probe - 1] === '+' || value[probe - 1] === '-')) {
            if (isExponentSign(probe - 1)) return null;
            sep = value[probe - 1];
            let q = probe - 1;
            while (q > 0 && value[q - 1] === ' ') q--;
            let p = q;
            while (p > 0 && /[0-9.]/.test(value[p - 1])) p--;
            const realRaw = value.slice(p, q);
            if (numRe.test(realRaw) && !/[A-Za-z0-9_).\]]/.test(p > 0 ? value[p - 1] : '')) {
                realStart = leadMinus(p);
            }
        }
        if (realStart >= 0) {
            const wide = value.slice(realStart, end);
            const parsed = _parseComplexLiteral(wide);
            if (parsed) return { raw: wide, start: realStart, end, re: parsed.re, im: parsed.im };
        }
        const s2 = leadMinus(start);
        const alone = value.slice(s2, end);
        const parsed = _parseComplexLiteral(alone);
        if (parsed) return { raw: alone, start: s2, end, re: parsed.re, im: parsed.im };
        return null;
    }
    if (!numRe.test(raw)) return null;
    // cursor on a real number: widen right over [+-] to an imaginary part
    let probe = end;
    while (probe < value.length && value[probe] === ' ') probe++;
    if (probe < value.length && (value[probe] === '+' || value[probe] === '-')) {
        if (isExponentSign(probe)) return null;
        let q = probe + 1;
        while (q < value.length && value[q] === ' ') q++;
        let p = q;
        while (p < value.length && /[0-9.]/.test(value[p])) p++;
        if (p < value.length && /[ijIJ]/.test(value[p]) && !/[A-Za-z0-9_]/.test(value[p + 1] || '')) {
            const s2 = leadMinus(start);
            const wide = value.slice(s2, p + 1);
            const parsed = _parseComplexLiteral(wide);
            if (parsed) return { raw: wide, start: s2, end: p + 1, re: parsed.re, im: parsed.im };
        }
    }
    return null;
}

function _parseComplexLiteral(text) {
    const m = String(text || '').replace(/ /g, '').match(
        new RegExp(`^(-?${_SCRUB_NUM})([+-]${_SCRUB_NUM})?[ijIJ]?$`));
    if (!m) return null;
    if (!/[ijIJ]/.test(String(text).slice(-1))) return null;
    if (m[3] !== undefined) {
        const re = Number(m[1]);
        const im = Number(m[3].replace(/^([+-])/, '$1'));
        if (!Number.isFinite(re) || !Number.isFinite(im)) return null;
        return { re, im };
    }
    const im = Number(m[1]);
    return Number.isFinite(im) ? { re: 0, im } : null;
}

function _scrubFormatComplex(re, im) {
    const rePart = _scrubFormatNumber(re);
    const imMag = _scrubFormatNumber(Math.abs(im));
    return `${rePart}${im < 0 ? '-' : '+'}${imMag}i`;
}

/* ---- Root pad: geometric editing of roots_literal(...) ----
   dblclick the roots_literal identifier and every root plots as a
   draggable point on a complex-plane canvas; each drag rewrites the whole
   call (one root per line, house-style i). roots_literal ONLY — plotting
   coefficients as points would be geometric nonsense. The pad opens only
   when EVERY argument is a plain literal: an expression like exp(pi2i/3)
   has no faithful reverse mapping, so refuse rather than corrupt (same
   policy as the 2D pad's e-notation refusal). */

function _rootPadParseArg(text) {
    const raw = String(text || '').trim();
    if (!raw) return null;
    if (new RegExp(`^-?${_SCRUB_NUM}$`).test(raw)) {
        const v = Number(raw);
        return Number.isFinite(v) ? { re: v, im: 0 } : null;
    }
    return _parseComplexLiteral(raw);
}

function _programRootsLiteralSpanAtCursor(textarea) {
    const span = _programTokenSpanAtCursor(textarea);
    if (!span || span.raw !== 'roots_literal') return null;
    const value = String(textarea.value || '');
    let open = span.end;
    while (open < value.length && /\s/.test(value[open])) open++;
    if (value[open] !== '(') return null;
    let depth = 0;
    let close = open;
    for (; close < value.length; close++) {
        if (value[close] === '(') depth++;
        else if (value[close] === ')') { depth--; if (depth === 0) break; }
    }
    if (depth !== 0) return null;
    const inner = value.slice(open + 1, close);
    if (!inner.trim()) return null;
    // literal-only args can never contain parens or commas, so every
    // top-level comma is a separator and any unparseable part refuses
    const roots = [];
    for (const part of inner.split(',')) {
        const parsed = _rootPadParseArg(part);
        if (!parsed) return null;
        roots.push(parsed);
    }
    if (roots.length > 255) return null;
    return { raw: value.slice(span.start, close + 1), start: span.start, end: close + 1, roots };
}

function _scrubFormatRoot(re, im) {
    // minimal house-style spelling: 1.5 / 2i / -7.5+2i — the single-token
    // imaginary form keeps recompiles token-identical
    if (im === 0) return _scrubFormatNumber(re);
    if (re === 0) return `${im < 0 ? '-' : ''}${_scrubFormatNumber(Math.abs(im))}i`;
    return _scrubFormatComplex(re, im);
}

function _rootPadFormatCall(roots) {
    // one root per line, matching the generator layout (diff-friendly)
    const lines = roots.map((r, i) =>
        `    ${_scrubFormatRoot(r.re, r.im)}${i < roots.length - 1 ? ',' : ''}`);
    return `roots_literal(\n${lines.join('\n')}\n)`;
}

function _rootPadSnapValue(v, step) {
    if (!(step > 0)) return v;
    const snapped = Math.round(v / step) * step;
    return Number(snapped.toFixed(9));
}

function _rootPadPlaneAt(cRe, cIm, half, size) {
    const scale = size / (2 * half);
    return {
        cRe, cIm, half, size, scale,
        toX(re) { return (re - this.cRe) * this.scale + this.size / 2; },
        toY(im) { return this.size / 2 - (im - this.cIm) * this.scale; },
        toRe(x) { return (x - this.size / 2) / this.scale + this.cRe; },
        toIm(y) { return this.cIm - (y - this.size / 2) / this.scale; },
    };
}

function _rootPadPlane(roots, size) {
    // one square complex-plane window over all roots plus margin, FIXED at
    // open time so dragging one point never re-frames the others; the
    // window input replaces it with a zero-centered square of side d
    let minRe = Infinity, maxRe = -Infinity, minIm = Infinity, maxIm = -Infinity;
    for (const r of roots) {
        minRe = Math.min(minRe, r.re); maxRe = Math.max(maxRe, r.re);
        minIm = Math.min(minIm, r.im); maxIm = Math.max(maxIm, r.im);
    }
    if (!Number.isFinite(minRe)) { minRe = -1; maxRe = 1; minIm = -1; maxIm = 1; }
    const cRe = (minRe + maxRe) / 2;
    const cIm = (minIm + maxIm) / 2;
    const half = Math.max(1, (maxRe - minRe) / 2, (maxIm - minIm) / 2) * 1.25;
    return _rootPadPlaneAt(cRe, cIm, half, size);
}

function _programMetricSpanAtCursor(which, textarea) {
    // Discrete scrub targets: solve-score metric names in the ss editors.
    if (which !== 'render-ss' && which !== 'palette-ss') return null;
    const span = _programTokenSpanAtCursor(textarea);
    if (!span) return null;
    if (typeof _solveScoreMetricNames === 'undefined' || !_solveScoreMetricNames.includes(span.raw)) return null;
    return span;
}

function _scrubFormatNumber(v) {
    if (!Number.isFinite(v)) return '0';
    let s = String(Number(v.toPrecision(6)));
    if (s.includes('e') || s.includes('E')) {
        s = v.toFixed(12).replace(/0+$/, '').replace(/\.$/, '') || '0';
    }
    return s;
}

function _scrubPadEl() {
    return document.getElementById('program-scrub-pad');
}

function _scrubPadNotifyInput(which) {
    // Programmatic .value writes do not fire input events; run the same
    // handler the textarea's oninput would, so validation/debounce flows.
    if (which === 'pp') return _onParamProgramSourceInput();
    if (which === 'cp') return _onCoeffProgramSourceInput();
    if (which === 'rt') return _onRootProgramSourceInput('render');
    if (which === 'prt') return _onRootProgramSourceInput('palette');
    if (which === 'render-ss') return _onSolveScoreProgramSourceInput('render');
    if (which === 'palette-ss') return _onSolveScoreProgramSourceInput('palette');
}

function _scrubPadOnExternalInput() {
    // Pad writes never dispatch input events, so any input event means the
    // user (or another feature) edited the textarea: the span is no longer
    // trustworthy and the binding evaporates.
    _closeProgramScrubPad();
}

function _scrubPadNudge(direction, big) {
    const st = _scrubPadState;
    if (!st) return;
    if (st.mode === 'roots') {
        if (st.activeRoot < 0) return;
        const step = (st.snapOn ? st.snapStep : 0.05) * (big ? 5 : 1);
        const dx = (direction && direction.dx) || 0;
        const dy = (direction && direction.dy) || 0;
        const root = st.roots[st.activeRoot];
        root.re = Number((root.re + dx * step).toFixed(9));
        root.im = Number((root.im + dy * step).toFixed(9));
        _rootPadWrite();
        return;
    }
    if (st.mode === 'choice') {
        const step = big ? 5 : 1;
        const next = Math.min(st.choices.length - 1, Math.max(0, st.index + direction * step));
        if (next !== st.index) {
            st.index = next;
            _scrubPadWriteText(st.choices[next]);
        }
        return;
    }
    if (st.mode === 'complex') {
        const step = 2 * st.span * (big ? 0.10 : 0.01);
        // direction carries the axis: {dx, dy} from the keydown handler
        const dx = (direction && direction.dx) || 0;
        const dy = (direction && direction.dy) || 0;
        _scrubPadWriteComplex(st.re + dx * step, st.im + dy * step);
        return;
    }
    const step = (st.max - st.min) * (big ? 0.10 : 0.01);
    const next = Math.min(st.max, Math.max(st.min, st.value + direction * step));
    _scrubPadWrite(next);
}

function _ensureProgramScrubPadHandlers() {
    if (_scrubPadHandlersBound || typeof document === 'undefined' || typeof document.addEventListener !== 'function') return;
    document.addEventListener('keydown', event => {
        if (!event || !_scrubPadState) return;
        if (event.key === 'Escape') { _revertProgramScrubPad(); return; }
        const arrows = { ArrowLeft: [-1, 0], ArrowRight: [1, 0], ArrowUp: [0, 1], ArrowDown: [0, -1] };
        if (arrows[event.key]) {
            // Arrows nudge while the pad is open (typing closes it anyway);
            // leave them alone inside the pad's own range fields.
            const target = event.target;
            const targetId = target && target.id;
            if (targetId === 'program-scrub-min' || targetId === 'program-scrub-max' || targetId === 'program-scrub-span') return;
            if (target && target.tagName === 'SELECT') return;
            const [dx, dy] = arrows[event.key];
            const mode = _scrubPadState.mode;
            if (mode !== 'complex' && mode !== 'roots' && dx === 0) return;
            if (typeof event.preventDefault === 'function') event.preventDefault();
            if (mode === 'complex' || mode === 'roots') _scrubPadNudge({ dx, dy }, !!event.shiftKey);
            else _scrubPadNudge(dx, !!event.shiftKey);
        }
    });
    document.addEventListener('mousedown', event => {
        const st = _scrubPadState;
        if (!st || st.dragging) return;
        const el = _scrubPadEl();
        if (!el || !event || !event.target) return;
        if (typeof el.contains === 'function' && el.contains(event.target)) return;
        _closeProgramScrubPad();
    });
    _scrubPadHandlersBound = true;
}

function _scrubPadMetricDescription(name) {
    const desc = (typeof _solveScoreMetricDescriptions !== 'undefined' && _solveScoreMetricDescriptions[name]) || '';
    const sources = typeof _solveScoreMetricAllowedSources === 'function'
        ? _solveScoreMetricAllowedSources(name)
        : [];
    return sources.length ? `${desc} Sources: ${sources.join(', ')}.` : desc;
}

function _openProgramScrubPad(which, span, textarea, event, mode = 'number') {
    _closeProgramHelpInspector();
    _closeProgramScrubPad();
    _scrubPadState = {
        which,
        mode,
        textarea,
        start: span.start,
        end: span.end,
        original: span.raw,
        current: span.raw,
        // Staleness guard is the FULL text, not the slice: a programmatic
        // rewrite (populate, program load) could coincidentally leave the
        // same characters at the span positions.
        snapshot: String(textarea.value || ''),
        value: 0,
        min: 0,
        max: 1,
        choices: [],
        index: -1,
        livePreview: false,
        view: '',
        dragging: false,
    };
    const st = _scrubPadState;
    if (mode === 'roots') {
        st.roots = span.roots.map(r => ({ re: r.re, im: r.im }));
        st.plane = _rootPadPlane(st.roots, 260);
        st.snapStep = 0.5;
        st.snapOn = true;
        st.activeRoot = -1;
    } else if (mode === 'choice') {
        st.choices = _solveScoreMetricNames.slice();
        st.index = st.choices.indexOf(span.raw);
    } else if (mode === 'complex') {
        // one square window centered on the original value: squiggles stay
        // proportionate in both axes
        const spread = Math.max(1, Math.abs(span.re), Math.abs(span.im));
        st.re = span.re;
        st.im = span.im;
        st.reCenter = span.re;
        st.imCenter = span.im;
        st.span = spread;
    } else {
        const spread = Math.max(1, Math.abs(span.value));
        st.value = span.value;
        st.min = span.value - spread;
        st.max = span.value + spread;
    }
    const el = _scrubPadEl();
    if (!el) return;
    const preview = _scrubPadPreviewByKey[which];
    const liveRow = preview
        ? `<label class="program-scrub-row"><input type="checkbox" id="program-scrub-live" onchange="_scrubPadToggleLive(this.checked)"> ${_escapeHtml(preview.label)}</label>`
        : '';
    let viewRow = '';
    if (preview && preview.loresViews) {
        const activeView = (typeof _renderLoresPreviewActiveTab !== 'undefined' && _renderLoresPreviewActiveTab) || 'plot';
        st.view = String(activeView);
        const options = _scrubPadLoresViews.map(([value, label]) =>
            `<option value="${value}"${value === st.view ? ' selected' : ''}>${label}</option>`
        ).join('');
        viewRow = `<label class="program-scrub-row">view <select id="program-scrub-view" onchange="_scrubPadSetView(this.value)">${options}</select></label>`;
    }
    let modeRows;
    let hint;
    if (mode === 'roots') {
        modeRows = `<div class="program-scrub-row">
            window <input type="text" id="program-scrub-window" onchange="_rootPadSetWindow()" title="View range: a square of side d centered on 0">
            <label><input type="checkbox" id="program-scrub-snap" checked onchange="_rootPadSetSnap(this.checked)"> snap 0.5</label>
        </div>`;
        hint = 'drag a point &middot; arrows nudge it (Shift &times;5) &middot; Esc reverts &middot; any other edit closes';
    } else if (mode === 'choice') {
        modeRows = `<div class="program-scrub-row"><span id="program-scrub-pos"></span></div>
        <div id="program-scrub-desc" class="program-scrub-desc"></div>`;
        hint = 'drag or &larr;/&rarr; to step metrics (Shift &times;5) &middot; Esc reverts &middot; any other edit closes';
    } else if (mode === 'complex') {
        modeRows = `<div class="program-scrub-row">
            &plusmn; <input type="text" id="program-scrub-span" onchange="_scrubPadSetSpan()" title="Half-width of the window, both axes, centered on the original value">
        </div>`;
        hint = 'drag: x = re, y = im &middot; arrows nudge (Shift bigger) &middot; Esc reverts &middot; any other edit closes';
    } else {
        modeRows = `<div class="program-scrub-row">
            min <input type="text" id="program-scrub-min" onchange="_scrubPadSetRange()">
            max <input type="text" id="program-scrub-max" onchange="_scrubPadSetRange()">
        </div>`;
        hint = 'drag to scrub &middot; &larr;/&rarr; nudge (Shift bigger) &middot; Esc reverts &middot; any other edit closes';
    }
    const title = mode === 'roots' ? 'Roots' : (mode === 'choice' ? 'Metric' : (mode === 'complex' ? 'Scrub 2D' : 'Scrub'));
    const surfaceClass = mode === 'complex' ? 'program-scrub-surface program-scrub-surface-2d' : 'program-scrub-surface';
    const surface = mode === 'roots'
        ? `<canvas id="program-scrub-canvas" class="program-scrub-canvas" width="260" height="260" onpointerdown="_rootPadDragStart(event)"></canvas>`
        : `<div id="program-scrub-surface" class="${surfaceClass}" onpointerdown="_scrubPadDragStart(event)">
            <div id="program-scrub-handle" class="program-scrub-handle"></div>
        </div>`;
    el.innerHTML = `
        <div class="program-scrub-head" onpointerdown="_scrubPadHeadDragStart(event)" title="Drag to move">
            <span class="program-scrub-title">${title}</span>
            <span id="program-scrub-value" class="program-scrub-value"></span>
            <button type="button" class="btn-secondary program-scrub-close" onclick="_closeProgramScrubPad()" aria-label="Close">x</button>
        </div>
        ${surface}
        ${modeRows}
        ${viewRow}
        ${liveRow}
        <div class="program-scrub-hint">${hint}</div>
    `;
    if (mode === 'roots') {
        const windowEl = document.getElementById('program-scrub-window');
        if (windowEl) windowEl.value = _scrubFormatNumber(2 * st.plane.half);
    } else if (mode === 'complex') {
        const spanEl = document.getElementById('program-scrub-span');
        if (spanEl) spanEl.value = _scrubFormatNumber(st.span);
    } else if (mode !== 'choice') {
        const minEl = document.getElementById('program-scrub-min');
        const maxEl = document.getElementById('program-scrub-max');
        if (minEl) minEl.value = _scrubFormatNumber(st.min);
        if (maxEl) maxEl.value = _scrubFormatNumber(st.max);
    }
    el.style.display = 'block';
    if (typeof el.setAttribute === 'function') el.setAttribute('aria-hidden', 'false');
    const raw = event || {};
    const pinned = _scrubPadLastPos;
    const x = pinned ? pinned.x : (Number(raw.clientX || 0) || 12);
    const y = pinned ? pinned.y - 14 : (Number(raw.clientY || 0) || 12);
    el.style.left = `${Math.max(8, x)}px`;
    el.style.top = `${Math.max(8, y + 14)}px`;
    const rect = typeof el.getBoundingClientRect === 'function' ? el.getBoundingClientRect() : { width: 300, height: 160 };
    const vw = (typeof window !== 'undefined' && window.innerWidth) || 1200;
    const vh = (typeof window !== 'undefined' && window.innerHeight) || 800;
    el.style.left = `${Math.max(8, Math.min(x, vw - Number(rect.width || 300) - 8))}px`;
    el.style.top = `${Math.max(8, Math.min(y + 14, vh - Number(rect.height || 160) - 8))}px`;
    _ensureProgramScrubPadHandlers();
    if (typeof textarea.addEventListener === 'function') {
        textarea.addEventListener('input', _scrubPadOnExternalInput);
    }
    _renderProgramScrubPad();
}

function _renderProgramScrubPad() {
    const st = _scrubPadState;
    if (!st) return;
    const valueEl = document.getElementById('program-scrub-value');
    if (st.mode === 'roots') {
        if (valueEl) {
            const active = st.activeRoot >= 0 ? st.roots[st.activeRoot] : null;
            valueEl.textContent = active
                ? `${st.roots.length} roots · ${_scrubFormatRoot(active.re, active.im)}`
                : `${st.roots.length} roots`;
        }
        _rootPadDraw();
        return;
    }
    if (valueEl) valueEl.textContent = st.current;
    const handle = document.getElementById('program-scrub-handle');
    if (handle) {
        if (st.mode === 'complex') {
            const w = st.span > 0 ? st.span : 1;
            const fx = Math.min(1, Math.max(0, (st.re - (st.reCenter - w)) / (2 * w)));
            // screen y grows downward; +imag points up (complex plane)
            const fy = Math.min(1, Math.max(0, ((st.imCenter + w) - st.im) / (2 * w)));
            handle.style.left = `${(fx * 100).toFixed(2)}%`;
            handle.style.top = `${(fy * 100).toFixed(2)}%`;
        } else {
            let frac = 0.5;
            if (st.mode === 'choice') {
                frac = st.choices.length > 1 ? Math.min(1, Math.max(0, st.index / (st.choices.length - 1))) : 0.5;
            } else {
                const span = st.max - st.min;
                frac = span > 0 ? Math.min(1, Math.max(0, (st.value - st.min) / span)) : 0.5;
            }
            handle.style.left = `${(frac * 100).toFixed(2)}%`;
            handle.style.top = '';
        }
    }
    if (st.mode === 'choice') {
        const posEl = document.getElementById('program-scrub-pos');
        if (posEl) posEl.textContent = `${st.index + 1}/${st.choices.length}`;
        const descEl = document.getElementById('program-scrub-desc');
        if (descEl) descEl.textContent = _scrubPadMetricDescription(st.current);
    }
}

function _scrubPadWriteText(text) {
    const st = _scrubPadState;
    if (!st || !st.textarea) return;
    const t = st.textarea;
    if (String(t.value || '') !== st.snapshot) {
        // Something else rewrote the textarea without an input event
        // (populate, program load, clear): the span is stale — never
        // write through it.
        _closeProgramScrubPad();
        return;
    }
    if (text !== st.current) {
        t.value = st.snapshot.slice(0, st.start) + text + st.snapshot.slice(st.end);
        st.end = st.start + text.length;
        st.current = text;
        st.snapshot = String(t.value || '');
        try { if (typeof t.setSelectionRange === 'function') t.setSelectionRange(st.start, st.end); } catch (e) {}
        _scrubPadNotifyInput(st.which);
        _scrubScheduleLivePreview();
    }
    _renderProgramScrubPad();
}

function _scrubPadWriteComplex(re, im) {
    const st = _scrubPadState;
    if (!st || st.mode !== 'complex') return;
    const text = _scrubFormatComplex(re, im);
    const parsed = _parseComplexLiteral(text);
    if (!parsed) return; // formatter/parser asymmetry (>=1e21): skip the tick
    st.re = parsed.re;
    st.im = parsed.im;
    _scrubPadWriteText(text);
}

function _scrubPadSetSpan() {
    const st = _scrubPadState;
    if (!st || st.mode !== 'complex') return;
    const el = document.getElementById('program-scrub-span');
    const value = Number(el && el.value);
    if (Number.isFinite(value) && value > 0) st.span = value;
    if (el) el.value = _scrubFormatNumber(st.span);
    _renderProgramScrubPad();
}

function _scrubPadWrite(nextValue) {
    const st = _scrubPadState;
    if (!st || st.mode === 'choice') return;
    const text = _scrubFormatNumber(nextValue);
    st.value = Number(text);
    _scrubPadWriteText(text);
}

function _scrubPadHeadDragStart(event) {
    // Reposition the pad itself (e.g. next to the preview). The close
    // button keeps its click; anything else on the header drags.
    if (event && event.target && event.target.classList &&
        event.target.classList.contains('program-scrub-close')) return;
    const el = _scrubPadEl();
    if (!el || !event) return;
    if (typeof event.preventDefault === 'function') event.preventDefault();
    // Same pointer-capture discipline as the value drag: releasing outside
    // the window must still deliver pointerup or the pad glues to the
    // cursor on re-entry.
    if (event.target && typeof event.target.setPointerCapture === 'function' && event.pointerId != null) {
        try { event.target.setPointerCapture(event.pointerId); } catch (e) {}
    }
    const rect = typeof el.getBoundingClientRect === 'function' ? el.getBoundingClientRect() : { left: 0, top: 0 };
    const offX = (Number(event.clientX) || 0) - rect.left;
    const offY = (Number(event.clientY) || 0) - rect.top;
    const move = e => {
        const x = Math.max(0, (Number(e.clientX) || 0) - offX);
        const y = Math.max(0, (Number(e.clientY) || 0) - offY);
        el.style.left = `${x}px`;
        el.style.top = `${y}px`;
        _scrubPadLastPos = { x, y: y + 14 };
    };
    const up = () => {
        document.removeEventListener('pointermove', move);
        document.removeEventListener('pointerup', up);
        document.removeEventListener('pointercancel', up);
        _scrubPadHeadDragCleanup = null;
    };
    document.addEventListener('pointermove', move);
    document.addEventListener('pointerup', up);
    document.addEventListener('pointercancel', up);
    _scrubPadHeadDragCleanup = up;
}

function _scrubPadDragStart(event) {
    const st = _scrubPadState;
    if (!st) return;
    if (event && typeof event.preventDefault === 'function') event.preventDefault();
    st.dragging = true;
    // Pointer capture: releasing outside the window still delivers
    // pointerup, so listeners can never be stranded mid-drag (the classic
    // mouse-event drag bug). pointercancel is the backstop.
    const surface = document.getElementById('program-scrub-surface');
    if (surface && typeof surface.setPointerCapture === 'function' && event && event.pointerId != null) {
        try { surface.setPointerCapture(event.pointerId); } catch (e) {}
    }
    _scrubPadDragMove(event);
    const move = e => _scrubPadDragMove(e);
    const up = () => {
        if (_scrubPadState) _scrubPadState.dragging = false;
        document.removeEventListener('pointermove', move);
        document.removeEventListener('pointerup', up);
        document.removeEventListener('pointercancel', up);
    };
    document.addEventListener('pointermove', move);
    document.addEventListener('pointerup', up);
    document.addEventListener('pointercancel', up);
}

function _scrubPadDragMove(event) {
    const st = _scrubPadState;
    if (!st) return;
    const surface = document.getElementById('program-scrub-surface');
    if (!surface || typeof surface.getBoundingClientRect !== 'function') return;
    const rect = surface.getBoundingClientRect();
    if (!rect || !(rect.width > 0)) return;
    const frac = Math.min(1, Math.max(0, ((Number(event && event.clientX) || 0) - rect.left) / rect.width));
    if (st.mode === 'complex') {
        if (!(rect.height > 0)) return;
        const fy = Math.min(1, Math.max(0, ((Number(event && event.clientY) || 0) - rect.top) / rect.height));
        const w = st.span > 0 ? st.span : 1;
        _scrubPadWriteComplex(
            (st.reCenter - w) + frac * 2 * w,
            (st.imCenter + w) - fy * 2 * w,
        );
        return;
    }
    if (st.mode === 'choice') {
        const idx = Math.round(frac * (st.choices.length - 1));
        if (idx !== st.index) {
            st.index = idx;
            _scrubPadWriteText(st.choices[idx]);
        }
        return;
    }
    _scrubPadWrite(st.min + frac * (st.max - st.min));
}

function _rootPadDraw() {
    const st = _scrubPadState;
    if (!st || st.mode !== 'roots') return;
    const canvas = document.getElementById('program-scrub-canvas');
    // headless harnesses have no 2D context: all pad logic still runs
    const ctx = canvas && typeof canvas.getContext === 'function' ? canvas.getContext('2d') : null;
    if (!ctx) return;
    const plane = st.plane;
    const size = plane.size;
    ctx.clearRect(0, 0, size, size);
    ctx.fillStyle = '#101418';
    ctx.fillRect(0, 0, size, size);
    if (st.snapOn && st.snapStep > 0 && st.snapStep * plane.scale >= 7) {
        ctx.strokeStyle = '#1d242c';
        ctx.lineWidth = 1;
        const lo = plane.toRe(0), hi = plane.toRe(size);
        for (let g = Math.ceil(lo / st.snapStep) * st.snapStep; g <= hi; g += st.snapStep) {
            const x = plane.toX(g);
            ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, size); ctx.stroke();
        }
        const imHi = plane.toIm(0), imLo = plane.toIm(size);
        for (let g = Math.ceil(imLo / st.snapStep) * st.snapStep; g <= imHi; g += st.snapStep) {
            const y = plane.toY(g);
            ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(size, y); ctx.stroke();
        }
    }
    ctx.strokeStyle = '#39434e';
    ctx.lineWidth = 1;
    const axisX = plane.toX(0), axisY = plane.toY(0);
    if (axisX >= 0 && axisX <= size) { ctx.beginPath(); ctx.moveTo(axisX, 0); ctx.lineTo(axisX, size); ctx.stroke(); }
    if (axisY >= 0 && axisY <= size) { ctx.beginPath(); ctx.moveTo(0, axisY); ctx.lineTo(size, axisY); ctx.stroke(); }
    st.roots.forEach((r, i) => {
        const x = plane.toX(r.re), y = plane.toY(r.im);
        ctx.beginPath();
        ctx.arc(x, y, i === st.activeRoot ? 5.5 : 4, 0, 2 * Math.PI);
        ctx.fillStyle = i === st.activeRoot ? '#ff9d66' : '#6fc3ff';
        ctx.fill();
    });
}

function _rootPadSetSnap(checked) {
    const st = _scrubPadState;
    if (!st || st.mode !== 'roots') return;
    st.snapOn = !!checked;
    _rootPadDraw();
}

function _rootPadSetWindow() {
    // View-range control: a single number d frames a square of side d
    // CENTERED ON 0 (the open-time frame auto-fits the layout instead).
    // Points outside the window stay live in the text; enlarge d to see
    // them again. Invalid input re-displays the effective side.
    const st = _scrubPadState;
    if (!st || st.mode !== 'roots') return;
    const el = document.getElementById('program-scrub-window');
    const side = Number(el && el.value);
    if (Number.isFinite(side) && side > 0) {
        st.plane = _rootPadPlaneAt(0, 0, side / 2, st.plane.size);
    }
    if (el) el.value = _scrubFormatNumber(2 * st.plane.half);
    _rootPadDraw();
}

function _rootPadWrite() {
    const st = _scrubPadState;
    if (!st || st.mode !== 'roots') return;
    _scrubPadWriteText(_rootPadFormatCall(st.roots));
}

function _rootPadCanvasPoint(event) {
    const canvas = document.getElementById('program-scrub-canvas');
    if (!canvas || typeof canvas.getBoundingClientRect !== 'function') return null;
    const rect = canvas.getBoundingClientRect();
    if (!rect || !(rect.width > 0)) return null;
    const st = _scrubPadState;
    const scale = st.plane.size / rect.width;   // css px -> canvas px
    return {
        x: ((Number(event && event.clientX) || 0) - rect.left) * scale,
        y: ((Number(event && event.clientY) || 0) - rect.top) * scale,
    };
}

function _rootPadDragStart(event) {
    const st = _scrubPadState;
    if (!st || st.mode !== 'roots') return;
    if (event && typeof event.preventDefault === 'function') event.preventDefault();
    const pt = _rootPadCanvasPoint(event);
    if (!pt) return;
    // pick the nearest point within a 14px halo; empty space does nothing
    let best = -1;
    let bestD = 14 * 14;
    st.roots.forEach((r, i) => {
        const dx = st.plane.toX(r.re) - pt.x;
        const dy = st.plane.toY(r.im) - pt.y;
        const d = dx * dx + dy * dy;
        if (d <= bestD) { bestD = d; best = i; }
    });
    if (best < 0) return;
    st.activeRoot = best;
    st.dragging = true;
    const canvas = document.getElementById('program-scrub-canvas');
    if (canvas && typeof canvas.setPointerCapture === 'function' && event && event.pointerId != null) {
        try { canvas.setPointerCapture(event.pointerId); } catch (e) {}
    }
    _rootPadDragMove(event);
    const move = e => _rootPadDragMove(e);
    const up = () => {
        if (_scrubPadState) _scrubPadState.dragging = false;
        document.removeEventListener('pointermove', move);
        document.removeEventListener('pointerup', up);
        document.removeEventListener('pointercancel', up);
    };
    document.addEventListener('pointermove', move);
    document.addEventListener('pointerup', up);
    document.addEventListener('pointercancel', up);
}

function _rootPadDragMove(event) {
    const st = _scrubPadState;
    if (!st || st.mode !== 'roots' || st.activeRoot < 0) return;
    const pt = _rootPadCanvasPoint(event);
    if (!pt) return;
    let re = st.plane.toRe(pt.x);
    let im = st.plane.toIm(pt.y);
    if (st.snapOn) {
        re = _rootPadSnapValue(re, st.snapStep);
        im = _rootPadSnapValue(im, st.snapStep);
    }
    const root = st.roots[st.activeRoot];
    if (root.re === re && root.im === im) return;
    root.re = re;
    root.im = im;
    _rootPadWrite();
}

function _scrubPadSetRange() {
    const st = _scrubPadState;
    if (!st || st.mode === 'choice') return;
    const minEl = document.getElementById('program-scrub-min');
    const maxEl = document.getElementById('program-scrub-max');
    const min = Number(minEl && minEl.value);
    const max = Number(maxEl && maxEl.value);
    if (Number.isFinite(min)) st.min = min;
    if (Number.isFinite(max)) st.max = max;
    if (st.max <= st.min) st.max = st.min + 1;
    if (minEl) minEl.value = _scrubFormatNumber(st.min);
    if (maxEl) maxEl.value = _scrubFormatNumber(st.max);
    _renderProgramScrubPad();
}

function _scrubPadSetView(view) {
    const st = _scrubPadState;
    if (!st) return;
    st.view = String(view || 'plot');
    // The tab function owns the fallbacks (palette/e1-3 unavailable ->
    // plot); the pad only records intent and re-asserts it after each
    // preview run, so choosing Palette before the first palette image
    // arrives still wins once it exists.
    if (typeof _selectRenderLoresPreviewTab === 'function') _selectRenderLoresPreviewTab(st.view, { fromScrubPad: true });
}

function _scrubPadToggleLive(checked) {
    const st = _scrubPadState;
    if (!st) return;
    st.livePreview = !!checked;
    if (st.livePreview) _scrubScheduleLivePreview();
}

function _scrubScheduleLivePreview() {
    const st = _scrubPadState;
    if (!st || !st.livePreview) return;
    const preview = _scrubPadPreviewByKey[st.which];
    if (!preview) return;
    if (_scrubPreviewTimer) clearTimeout(_scrubPreviewTimer);
    _scrubPreviewTimer = setTimeout(async () => {
        _scrubPreviewTimer = null;
        if (_scrubPreviewInFlight) { _scrubPreviewDirty = true; return; }
        _scrubPreviewInFlight = true;
        try {
            await preview.run();
            const st2 = _scrubPadState;
            if (st2 && st2.view && preview.loresViews && typeof _selectRenderLoresPreviewTab === 'function') {
                _selectRenderLoresPreviewTab(st2.view, { fromScrubPad: true });
            }
        } catch (e) {
            /* preview errors surface in their own status lines */
        }
        _scrubPreviewInFlight = false;
        if (_scrubPreviewDirty) {
            _scrubPreviewDirty = false;
            _scrubScheduleLivePreview();
        }
    }, 600);
}

function _revertProgramScrubPad() {
    const st = _scrubPadState;
    if (!st) return;
    const t = st.textarea;
    if (t && String(t.value || '') === st.snapshot && st.current !== st.original) {
        t.value = st.snapshot.slice(0, st.start) + st.original + st.snapshot.slice(st.end);
        _scrubPadNotifyInput(st.which);
    }
    _closeProgramScrubPad();
}

function _closeProgramScrubPad() {
    if (_scrubPadHeadDragCleanup) _scrubPadHeadDragCleanup();
    const st = _scrubPadState;
    _scrubPadState = null;
    if (_scrubPreviewTimer) { clearTimeout(_scrubPreviewTimer); _scrubPreviewTimer = null; }
    _scrubPreviewDirty = false;
    if (st && st.textarea && typeof st.textarea.removeEventListener === 'function') {
        st.textarea.removeEventListener('input', _scrubPadOnExternalInput);
    }
    const el = _scrubPadEl();
    if (el) {
        el.style.display = 'none';
        el.innerHTML = '';
        if (typeof el.setAttribute === 'function') el.setAttribute('aria-hidden', 'true');
    }
}


function _onProgramSourceDblClick(which, event) {
    const key = _programSourceWhichKey(which);
    const textarea = _programSourceTextarea(key);
    if (key === 'cp') {
        // roots_literal is coeff-program vocabulary only
        const rootsSpan = _programRootsLiteralSpanAtCursor(textarea);
        if (rootsSpan) {
            _openProgramScrubPad(key, rootsSpan, textarea, event || {}, 'roots');
            return;
        }
    }
    const complexSpan = _programComplexSpanAtCursor(textarea);
    if (complexSpan) {
        _openProgramScrubPad(key, complexSpan, textarea, event || {}, 'complex');
        return;
    }
    const span = _programNumberSpanAtCursor(textarea);
    if (span) {
        _openProgramScrubPad(key, span, textarea, event || {}, 'number');
        return;
    }
    const metricSpan = _programMetricSpanAtCursor(key, textarea);
    if (metricSpan) {
        _openProgramScrubPad(key, metricSpan, textarea, event || {}, 'choice');
        return;
    }
    _closeProgramScrubPad();
    const token = _programWordAtTextareaCursor(textarea);
    const item = _lookupProgramHelpToken(key, token);
    _openProgramHelpInspector(key, _normalizeProgramHelpToken(token) || token, item, event || {});
}

function _renderProgramSourceCheatsheet(elementId, insertFn, sections) {
    const el = document.getElementById(elementId);
    if (!el) return;
    el.innerHTML = (sections || []).map(section => {
        const buttons = (section.buttons || []).map(item => (
            _programSourceCheatButtonHtml(insertFn, item.label, item.snippet, item.title || '')
        ));
        return _programSourceCheatSectionHtml(section.title, buttons);
    }).join('');
}

function _renderParamProgramCheatsheet() {
    _renderProgramSourceCheatsheet('pp-cheatsheet', '_insertParamProgramSourceSnippet', _paramProgramCheatSections);
}

function _renderCoeffProgramCheatsheet() {
    _renderProgramSourceCheatsheet('cp-cheatsheet', '_insertCoeffProgramSourceSnippet', _coeffProgramCheatSections);
}

function _renderRootProgramCheatsheets() {
    _renderProgramSourceCheatsheet('rt-cheatsheet', '_insertRenderRootSourceSnippet', _rootProgramCheatSections);
    _renderProgramSourceCheatsheet('prt-cheatsheet', '_insertPaletteRootSourceSnippet', _rootProgramCheatSections);
}

function _renderParamCoeffProgramCheatsheets() {
    _renderParamProgramCheatsheet();
    _renderCoeffProgramCheatsheet();
    _renderRootProgramCheatsheets();
    _renderProgramSourceSidePanels();
}

let _solveScoreProgramEditorMode = { render: 'text', palette: 'text' };
function _editorPrefix(prefix) {
    return prefix === 'palette' ? 'palette' : 'render';
}

function _solveScoreSourceTextarea(prefix) {
    return document.getElementById(`${_editorPrefix(prefix)}-ss-source-text`);
}

function _rootProgramSourceTextarea(prefix) {
    return document.getElementById(`${_editorPrefix(prefix)}-rt-source-text`);
}

function _getSolveScoreProgramSourceText(prefix) {
    const el = _solveScoreSourceTextarea(prefix);
    return String(el ? el.value : '');
}

function _setSolveScoreProgramSourceText(prefix, text) {
    const el = _solveScoreSourceTextarea(prefix);
    const value = String(text == null ? '' : text);
    if (el && el.value !== value) el.value = value;
}

function _effectiveSolveScoreProgramSourceText(prefix) {
    const text = _getSolveScoreProgramSourceText(prefix);
    return text.trim() ? text : '';
}

function _requireSolveScoreProgramSourceText(prefix) {
    const p = _editorPrefix(prefix);
    const text = _effectiveSolveScoreProgramSourceText(p);
    if (text) return text;
    const message = 'Solve-score source is empty. Enter, load, or populate a text program before rendering.';
    _setSolveScoreProgramStatus(p, message, true);
    throw new Error(message);
}

function _defaultSolveScoreProgramSourceText(prefix) {
    const p = _editorPrefix(prefix);
    const metric = p === 'palette'
        ? String(paletteTabMetric || 'proximity')
        : String(renderSolveMetric || 'proximity');
    const choices = _solveScoreMetricAllowedSources(metric);
    const source = choices.includes('slv') ? 'slv' : (choices[0] || 'slv');
    return `score = metric(${metric}, ${source}, q=0.1%)\n`;
}

function _ensureSolveScoreSourceDefaults() {
    ['render', 'palette'].forEach((prefix) => {
        if (!_getSolveScoreProgramSourceText(prefix).trim()) {
            _setSolveScoreProgramSourceText(prefix, _defaultSolveScoreProgramSourceText(prefix));
        }
    });
}

function _getRootProgramSourceText(prefix) {
    const el = _rootProgramSourceTextarea(prefix);
    return String(el ? el.value : '');
}

let _rootProgramSourceAutoSynthed = { render: false, palette: false };

function _setRootProgramSourceText(prefix, text, options = {}) {
    const p = _editorPrefix(prefix);
    const el = _rootProgramSourceTextarea(p);
    const value = String(text == null ? '' : text);
    if (el && el.value !== value) el.value = value;
    if (options.auto === true) {
        _rootProgramSourceAutoSynthed[p] = true;
    } else if (options.auto === false || value.trim()) {
        _rootProgramSourceAutoSynthed[p] = false;
    }
}

function _effectiveRootProgramSourceText(prefix) {
    const text = _getRootProgramSourceText(prefix);
    return text.trim() ? text : '';
}

function _rootProgramStatus(prefix, message, isError = false) {
    const p = _editorPrefix(prefix);
    const el = document.getElementById(p === 'palette' ? 'palette-root-program-status' : 'root-program-status');
    if (!el) return;
    el.textContent = String(message || '');
    el.className = 'solve-score-program-status' + (isError ? ' error' : '');
}

function _setPanelTabActive(panelBase, mode) {
    const normalized = mode === 'text' ? 'text' : 'chips';
    const chipsPanel = document.getElementById(`${panelBase}-chips-panel`);
    const textPanel = document.getElementById(`${panelBase}-text-panel`);
    const chipsTab = document.getElementById(`${panelBase}-tab-chips`);
    const textTab = document.getElementById(`${panelBase}-tab-text`);
    if (chipsPanel && chipsPanel.classList) chipsPanel.classList.toggle('active', normalized === 'chips');
    if (textPanel && textPanel.classList) textPanel.classList.toggle('active', normalized === 'text');
    if (chipsTab && chipsTab.classList) chipsTab.classList.toggle('active', normalized === 'chips');
    if (textTab && textTab.classList) textTab.classList.toggle('active', normalized === 'text');
}

function _setSolveScoreProgramEditorMode(prefix, mode) {
    const p = _editorPrefix(prefix);
    const normalized = 'text';
    _solveScoreProgramEditorMode[p] = normalized;
    _setPanelTabActive(`${p}-ss`, normalized);
    _setSolveScoreProgramStatus(p, '');
}

function _onSolveScoreProgramSourceInput(prefix) {
    const p = _editorPrefix(prefix);
    _scheduleProgramSourceValidation(p === 'palette' ? 'palette-ss' : 'render-ss');
    _setSolveScoreProgramStatus(p, 'Text source changed. It will be compiled by the backend on render.', false);
}

function _insertSolveScoreSourceSnippet(prefix, snippet) {
    const p = _editorPrefix(prefix);
    const el = _solveScoreSourceTextarea(p);
    if (!el) return;
    const text = String(snippet || '');
    const start = Number.isFinite(el.selectionStart) ? el.selectionStart : el.value.length;
    const end = Number.isFinite(el.selectionEnd) ? el.selectionEnd : start;
    const before = el.value.slice(0, start);
    const after = el.value.slice(end);
    const lead = before && !before.endsWith('\n') ? '\n' : '';
    const tail = after && !text.endsWith('\n') ? '\n' : '';
    const inserted = `${lead}${text}`;
    el.value = before + inserted + tail + after;
    const pos = before.length + inserted.length;
    if (typeof el.setSelectionRange === 'function') el.setSelectionRange(pos, pos);
    if (typeof el.focus === 'function') el.focus();
    _onSolveScoreProgramSourceInput(p);
}

function _solveScoreMetricSnippet(name) {
    const metric = String(name || 'proximity');
    if (_solveScoreMetricSnippets[metric]) return String(_solveScoreMetricSnippets[metric]);
    const choices = _solveScoreMetricAllowedSources(metric);
    const source = choices.includes('slv') ? 'slv' : (choices[0] || 'slv');
    return `score = metric(${metric}, ${source}, q=0.1%)`;
}

function _solveScoreSnippetParamValues(params) {
    return (params || []).map(p => String((p && p.def) || '0'));
}

function _solveScoreUnarySnippet(name, spec) {
    if (spec && spec.snippet) return String(spec.snippet);
    if (name === 'const') return 'score = add(metric(proximity, slv, q=0.1%), const(0))';
    if (name === 'dup') return 'push(metric(proximity, slv, q=0.1%))\ndup()\nflush()\nscore = metric(proximity, slv, q=0.1%)';
    if (name === 'flush') return 'push(metric(proximity, slv, q=0.1%))\nflush()\nscore = metric(proximity, slv, q=0.1%)';
    const args = ['score'].concat(_solveScoreSnippetParamValues(spec && spec.params));
    args[0] = 'metric(proximity, slv, q=0.1%)';
    return `score = ${name}(${args.join(', ')})`;
}

function _solveScoreCombineSnippet(name, spec) {
    if (spec && spec.snippet) return String(spec.snippet);
    const arity = Number(spec && spec.arity) || 2;
    const metrics = ['proximity', 'spread', 'crowding', 'area'];
    const args = Array.from({ length: arity }, (_, i) => `metric(${metrics[i] || 'proximity'}, slv, q=0.1%)`)
        .concat(_solveScoreSnippetParamValues(spec && spec.params));
    return `score = ${name}(${args.join(', ')})`;
}

function _solveScoreCheatButtonHtml(prefix, label, snippet, title = '') {
    const safeLabel = _escapeHtml(label);
    const safeTitle = title ? ` title="${_escapeHtml(title)}"` : '';
    const safeSnippet = _escapeHtml(JSON.stringify(String(snippet || '')));
    return `<button type="button" class="btn-secondary solve-score-cheat-button" onclick="_insertSolveScoreSourceSnippet('${prefix}', ${safeSnippet})"${safeTitle}>${safeLabel}</button>`;
}

function _solveScoreCheatSectionHtml(title, buttons) {
    if (!buttons.length) return '';
    return `<div class="solve-score-cheat-section"><div class="solve-score-cheat-title">${_escapeHtml(title)}</div><div class="solve-score-cheat-buttons">${buttons.join('')}</div></div>`;
}

// Static snippet tables (gated: tests compile every snippet).
const _solveScoreFallbackStarters = [
    { label: 'score = proximity', snippet: 'score = metric(proximity, slv, q=0.1%)' },
    { label: 'emit_norm proximity', snippet: 'emit_norm(metric(proximity, slv, q=0.1%))' },
    { label: 'two channels', snippet: 'emit_norm(metric(proximity, slv, q=0.1%))\nemit_norm(metric(spread, slv, q=0.1%))' },
];
const _solveScoreLanguageSnippets = [
    { label: 'x1 = expr', snippet: 'x1 = metric(proximity, slv, q=0.1%)\nscore = (x1 + 1) * 0.5', title: 'Local alias: write-once, substituted at compile time.' },
    { label: 'infix + - * /', snippet: 'x1 = metric(proximity, slv, q=0.1%)\nx2 = metric(area, slv, q=1%)\nscore = x1 + x2 * 2', title: 'Infix arithmetic lowers to add/subtract/mul/ratio with standard precedence.' },
];

function _renderSolveScoreCheatsheet(prefix) {
    const p = _editorPrefix(prefix);
    const el = document.getElementById(`${p}-ss-cheatsheet`);
    if (!el) return;
    const metrics = _solveScoreMetricNames.slice();
    const starters = (_solveScoreStarterSnippets.length ? _solveScoreStarterSnippets : _solveScoreFallbackStarters)
        .map(item => _solveScoreCheatButtonHtml(p, item.label, item.snippet));
    const metricButtons = metrics.map(name => {
        const choices = _solveScoreMetricAllowedSources(name).join('/');
        return _solveScoreCheatButtonHtml(p, name, _solveScoreMetricSnippet(name), choices ? `${choices}; q=0.1%` : '');
    });
    const outputButtons = Object.keys(_solveScoreOutputSpecs).map(name => {
        const expr = 'metric(proximity, slv, q=0.1%)';
        const fn = _solveScoreOutputSpecs[name].snippet || (name === 'emit'
            ? `emit(${expr})`
            : (name === 'emit_none' ? `emit_none(${expr})\nemit_norm(${expr})` : `${name}(${expr})`));
        return _solveScoreCheatButtonHtml(p, name, fn, (_solveScoreOutputSpecs[name] || {}).tooltip || '');
    });
    const unaryButtons = Object.keys(_solveScoreUnarySpecs).map(name => (
        _solveScoreCheatButtonHtml(p, name, _solveScoreUnarySnippet(name, _solveScoreUnarySpecs[name]), (_solveScoreUnarySpecs[name] || {}).tooltip || '')
    ));
    const combineButtons = Object.keys(_solveScoreCombineSpecs).map(name => (
        _solveScoreCheatButtonHtml(p, name, _solveScoreCombineSnippet(name, _solveScoreCombineSpecs[name]), (_solveScoreCombineSpecs[name] || {}).tooltip || '')
    ));
    const languageButtons = _solveScoreLanguageSnippets.map(item =>
        _solveScoreCheatButtonHtml(p, item.label, item.snippet, item.title || ''));
    el.innerHTML = [
        _solveScoreCheatSectionHtml('Language', languageButtons),
        _solveScoreCheatSectionHtml('Starters', starters),
        _solveScoreCheatSectionHtml('Metrics', metricButtons),
        _solveScoreCheatSectionHtml('Outputs', outputButtons),
        _solveScoreCheatSectionHtml('Unary / Stack', unaryButtons),
        _solveScoreCheatSectionHtml('Combine', combineButtons),
    ].join('');
}

function _renderSolveScoreCheatsheets() {
    _renderSolveScoreCheatsheet('render');
    _renderSolveScoreCheatsheet('palette');
}

function _onRootProgramSourceInput(prefix) {
    const p = _editorPrefix(prefix);
    _rootProgramSourceAutoSynthed[p] = false;
    _scheduleProgramSourceValidation(p === 'palette' ? 'palette-rt' : 'render-rt');
    _rootProgramStatus(p, 'Text source changed. It will be compiled by the backend on render.');
}

function _rootSourceFromRows(chain) {
    // No-arg rows must serialize as name() — the source parser rejects bare
    // names (mirrors root_source_text_from_chain in root_program_source.py).
    return (chain || []).map(item => {
        if (Array.isArray(item) && item.length) {
            return `${item[0]}(${item.slice(1).join(', ')})`;
        }
        if (item && typeof item === 'object') {
            const name = String(item.name || '').trim();
            if (!name) return '';
            const params = Array.isArray(item.params) ? item.params : (Array.isArray(item.args) ? item.args : []);
            return `${name}(${params.join(', ')})`;
        }
        return '';
    }).filter(Boolean).join('\n');
}

async function _compileSolveScoreSourceEditor(prefix) {
    const p = _editorPrefix(prefix);
    const sourceText = _getSolveScoreProgramSourceText(p);
    if (!sourceText.trim()) {
        _setSolveScoreProgramStatus(p, 'Solve-score source is empty.', true);
        return null;
    }
    try {
        const resp = await lambdaPost('storage', { source_text: sourceText, strict: true }, '/compile-solve-score-program-source');
        if (!resp || !resp.ok) {
            const first = resp && Array.isArray(resp.diagnostics)
                ? (resp.diagnostics.find(d => d && d.level === 'error') || resp.diagnostics[0])
                : null;
            throw new Error(first ? `Line ${first.line || '?'}: ${first.message}` : 'solve-score source did not compile');
        }
        const program = resp.program || resp;
        const chain = Array.isArray(program.chain) ? program.chain : (Array.isArray(resp.chain) ? resp.chain : []);
        const which = _solveScoreWhichForPrefix(p);
        const target = _chainForWhich(which);
        target.splice(0, target.length, ..._normalizeSolveScoreChain(chain, p === 'palette' ? paletteTabMetric : renderSolveMetric));
        _syncSolveScoreUi(which);
        _setSolveScoreProgramEditorMode(p, 'text');
        _setSolveScoreProgramStatus(p, `Text source OK: ${program.statement_count || resp.statement_count || chain.length} statement${(program.statement_count || resp.statement_count || chain.length) === 1 ? '' : 's'}.`);
        return program;
    } catch (e) {
        _setSolveScoreProgramStatus(p, e && e.message ? e.message : String(e), true);
        throw e;
    }
}

// Registry names that the source parser shadows with typed builtins; the
// synthesizer must emit the parser-side aliases (mirrors
// _NATIVE_TRANSFORM_ALIASES in lambda/coeff_program_source.py).
const _coeffProgramSourceAliasNames = _coeffRegistryVocab ? _coeffRegistryVocab.sourceAliasByName : {};

// Forward map from registry transform names to Coeff Program chip names.
// exp and power are shadowed in the chip catalog (exp by the vector unary,
// power by the vector binary), so normalize/copy must emit the dedicated
// chips or the rows compile as the wrong operation.
const _coeffProgramRegistryChipNames = _coeffRegistryVocab ? _coeffRegistryVocab.chipNameByRegistryName : {};
function _canonicalCoeffProgramChipName(name) {
    // 'const' is the historical alias of the push_const chip.
    return name === 'const' ? 'push_const' : name;
}
function _coeffProgramRegistryChipName(name) {
    return _coeffProgramRegistryChipNames[name] || name;
}

function _coeffProgramSourceFromRows(chain) {
    return (chain || []).map(row => {
        if (Array.isArray(row) && row.length) {
            const name = String(row[0] || '').trim();
            const params = row.slice(1).map(v => _str(v));
            if (name === 'push' && params[0] === 'cf') return 'cf';
            if (name === 'push' && params[0] === 'poly') return 'poly';
            if ((name === 'push_const' || name === 'const') && params.length >= 2) return `push_vec(${params.join(', ')})`;
            if (name === 'pop') return 'drop';
            if (name === 'set' && params[0] === 'poly' && params[1]) return `poly = ${params[1]}`;
            if (name === 'poke_poly' && params.length >= 2) return `poly[${params[0]}] = ${params[1]}`;
            if (name === 'poke_tos' && params.length >= 2) return `poke_tos(${params[0]}, ${params[1]})`;
            if (_coeffProgramVectorBinaryNames.includes(name) && params.length >= 3) {
                const [tgt, src1, src2] = params;
                if (tgt === 'poly') return `poly = ${name}(${src1}, ${src2})`;
                if (src1 === 'pop' && src2 === 'pop') return `${name}()`;
                return `${name}(${src1}, ${src2})`;
            }
            if (_coeffProgramVectorUnaryNames.includes(name) && params.length >= 2) {
                // Trailing params after tgt/src (e.g. andy on sin/cos) must
                // survive synthesis; the parser routes them to the native
                // transform path.
                const [tgt, src, ...rest] = params;
                const extras = rest.filter(v => v !== '');
                const args = [src, ...extras].join(', ');
                if (tgt === 'poly') return `poly = ${name}(${args})`;
                return (src === 'pop' && !extras.length) ? `${name}()` : `${name}(${args})`;
            }
            if ((name === 'roll' || name === 'rolr') && params.length >= 3) {
                const [tgt, src, n] = params;
                if (tgt === 'poly') return `poly = ${name}(${src}, ${n})`;
                return src === 'pop' ? `${name}(${n})` : `${name}(${src}, ${n})`;
            }
            if (name === 'argsort' && params.length >= 3) {
                const [tgt, src1, src2] = params;
                return tgt === 'poly' ? `poly = argsort(${src1}, ${src2})` : `argsort(${src1}, ${src2})`;
            }
            if (name === 'littlewood' && params.length >= 3) {
                const [tgt, a, b, andy] = params;
                const args = [a, b].concat(andy == null || andy === '' ? [] : [andy]).join(', ');
                return tgt === 'poly' ? `poly = littlewood(${args})` : `littlewood(${args})`;
            }
            const nativeSpec = _coeffProgramCatalog[name] || {};
            if (nativeSpec.nativeTransform && params.length >= 2) {
                const callName = _coeffProgramSourceAliasNames[name] || name;
                const [tgt, src, ...rest] = params;
                const args = [src || 'poly', ...rest].join(', ');
                return tgt === 'poly' ? `poly = ${callName}(${args})` : `${callName}(${args})`;
            }
            if (name === 'legacy') {
                // Keep selector defaults (legitimate), but never fabricate a
                // transform name — empty must fail backend compile loudly.
                const [legacyName, src, tgt, ...rest] = params;
                return `legacy(${[legacyName || '', src || 'poly', tgt || 'poly', ...rest].join(', ')})`;
            }
            return params.length ? `${name}(${params.join(', ')})` : name;
        }
        return String(row || '');
    }).filter(Boolean).join('\n');
}

function _setCoeffProgramSourceText(text, options = {}) {
    const value = String(text == null ? '' : text);
    const el = _coeffProgramSourceTextarea();
    if (el && el.value !== value) el.value = value;
    if (options.auto === true) {
        _coeffProgramSourceAutoSynthed = true;
    } else if (options.auto === false || value.trim()) {
        _coeffProgramSourceAutoSynthed = false;
    }
}

function _getCoeffProgramSourceText() {
    const el = _coeffProgramSourceTextarea();
    return String(el ? el.value : '');
}

// True while the text area holds an auto-synthesized rendering of the chip
// chain (not user-authored text). Cleared on any manual edit so switching
// back to the Text tab re-synthesizes from the latest chips instead of
// leaving stale text authoritative.
let _coeffProgramSourceAutoSynthed = false;

function _setCoeffProgramEditorMode(_mode) {
    // Coeff Program is text-only (the read-only chips tab was removed). Pin the
    // mode to 'text' so save/preview/debug/compute always send the source, and
    // still synthesize text from a loaded chain (chain-only programs) so the
    // editor is never empty when there is a program to show.
    _coeffProgramEditorMode = 'text';
    const textPanel = document.getElementById('cp-text-panel');
    if (textPanel && textPanel.classList) textPanel.classList.add('active');
    if (_coeffProgramSourceAutoSynthed && !_coeffProgramChain.length) {
        // Stale auto-synthesized text must not stay authoritative for compute.
        _setCoeffProgramSourceText('', { auto: false });
    } else if (_coeffProgramChain.length &&
        (!_getCoeffProgramSourceText().trim() || _coeffProgramSourceAutoSynthed)) {
        _setCoeffProgramSourceText(_coeffProgramSourceFromRows(_serializeCoeffProgramChain()), { auto: true });
    }
    _coeffProgramStatus('Text source is authoritative for save, preview, debug, and compute.');
    _syncParamPipelineModeUi();
    // typeof guard: _coeffProgramModalState is declared with `let` later in
    // the file; this can run during initial render before that declaration.
    if (typeof _coeffProgramModalState !== 'undefined' && _coeffProgramModalState.open) _renderCoeffProgramModal();
    if (_paramProgramModeSelected()) _markComputePreviewStale();
}

// Debounced advisory validation of text source against the authoritative
// backend parser; the status line shows the first diagnostic. Save/preview
// still compile strictly server-side — this is editor feedback only.
function _onCoeffProgramSourceInput() {
    _coeffProgramSourceAutoSynthed = false;
    _scheduleProgramSourceValidation('cp');
    if (_coeffProgramTextModeSelected() && _paramProgramModeSelected()) _markComputePreviewStale();
    _syncParamPipelineModeUi();
    // typeof guard: _coeffProgramModalState is declared with `let` later in
    // the file; this can run during initial render before that declaration.
    if (typeof _coeffProgramModalState !== 'undefined' && _coeffProgramModalState.open) _renderCoeffProgramModal();
    _coeffProgramStatus('Text source changed. It will be compiled by the backend on save/preview/compute.');
}

function _splitParamProgramLegacyArgs(value) {
    const text = _str(value).trim();
    if (!text) return [];
    return text.split(/[,\s]+/).map(part => part.trim()).filter(Boolean);
}

function _paramProgramLegacyArgDefs(legacyName) {
    const name = String(legacyName || '').trim();
    if (!_paramRegistryAdapter.hasParams(name)) return null;
    const defs = _paramRegistryAdapter.params(name);
    return Array.isArray(defs) ? defs : null;
}

function _paramProgramLegacyVariableCounts(legacyName) {
    const counts = _paramProgramLegacyVariableArgCounts[String(legacyName || '').trim()];
    return Array.isArray(counts) ? counts.map(Number).filter(Number.isFinite) : [];
}

function _paramProgramLegacyTakesNoArgs(legacyName) {
    const name = String(legacyName || '').trim();
    return _paramProgramLegacyNames.includes(name)
        && !_paramProgramLegacyArgDefs(name)
        && !_paramProgramLegacyVariableCounts(name).some(count => count > 0);
}

function _paramProgramLegacyTargetArgIndex(legacyName) {
    const idx = _paramProgramLegacyTargetArgIndexes[String(legacyName || '').trim()];
    return Number.isInteger(idx) ? idx : null;
}

function _migrateParamProgramLegacyTargetArgs(legacyName, src, tgt, args, options = {}) {
    const name = String(legacyName || '').trim();
    const values = Array.isArray(args) ? args.map(v => _str(v).trim()) : [];
    const targetIdx = _paramProgramLegacyTargetArgIndex(name);
    if (targetIdx == null || values.length <= targetIdx) return [src, tgt, values];
    if (!options.force && values.length <= ((_paramProgramLegacyArgDefs(name) || []).length)) {
        return [src, tgt, values];
    }
    const selector = _paramTargetToLegacySelector(values[targetIdx]);
    const nextSrc = String(src || 'both') === 'both' ? selector : src;
    const nextTgt = String(tgt || 'both') === 'both' ? selector : tgt;
    return [nextSrc, nextTgt, values.filter((_, idx) => idx !== targetIdx)];
}

function _formatParamProgramNumber(value) {
    const n = Number(value);
    if (!Number.isFinite(n)) return _str(value);
    return String(Object.is(n, -0) ? 0 : n);
}

function _formatParamProgramComplexArg(reValue, imValue) {
    const re = Number(reValue);
    const im = Number(imValue);
    if (!Number.isFinite(re) || !Number.isFinite(im)) return `${reValue}+${imValue}i`;
    if (im === 0) return _formatParamProgramNumber(re);
    if (re === 0) return `${_formatParamProgramNumber(im)}i`;
    const sign = im < 0 ? '' : '+';
    return `${_formatParamProgramNumber(re)}${sign}${_formatParamProgramNumber(im)}i`;
}

function _paramProgramMoebiusArgsForUi(args) {
    const defaults = [
        '1',
        '0',
        '0',
        '1',
    ];
    if (!Array.isArray(args) || !args.length) return [];
    const values = args.map(v => _str(v).trim());
    if (values.length >= 8) {
        return [
            _formatParamProgramComplexArg(values[0], values[1]),
            _formatParamProgramComplexArg(values[2], values[3]),
            _formatParamProgramComplexArg(values[4], values[5]),
            _formatParamProgramComplexArg(values[6], values[7]),
        ];
    }
    return defaults.map((fallback, idx) => values[idx] || fallback);
}

function _paramProgramInvTPlus2ArgsForUi(args) {
    const defaults = ['2', '2'];
    if (!Array.isArray(args) || !args.length) return [];
    const values = args.map(v => _str(v).trim());
    if (values.length >= 4) {
        return [
            _formatParamProgramComplexArg(values[0], values[1]),
            _formatParamProgramComplexArg(values[2], values[3]),
        ];
    }
    return defaults.map((fallback, idx) => values[idx] || fallback);
}

function _paramProgramLegacyArgsFromInput(legacyName, argsInput, options = {}) {
    const argDefs = _paramProgramLegacyArgDefs(legacyName);
    let rawArgs = [];
    if (Array.isArray(argsInput)) {
        const values = argsInput.map(v => _str(v).trim());
        if (argDefs && values.length > 1) {
            rawArgs = values;
        } else {
            // Array elements are already discrete args and may contain
            // whitespace inside expressions ("1 + 2"); split only on commas,
            // never on whitespace (whitespace-splitting mangled such args).
            rawArgs = values.flatMap(v => v.split(',')).map(part => part.trim()).filter(Boolean);
        }
    } else {
        rawArgs = _splitParamProgramLegacyArgs(argsInput);
    }
    if (_paramProgramLegacyTakesNoArgs(legacyName)) return [];
    if (String(legacyName || '').trim() === 'moebius') {
        return _paramProgramMoebiusArgsForUi(rawArgs.length ? rawArgs : (options.fillDefaults ? ['1', '0', '0', '1'] : []));
    }
    if (String(legacyName || '').trim() === 'inv_t_plus_2') {
        return _paramProgramInvTPlus2ArgsForUi(rawArgs.length ? rawArgs : (options.fillDefaults ? ['2', '2'] : []));
    }
    if (!argDefs) return rawArgs.filter(v => v !== '');
    if (!options.fillDefaults && !rawArgs.length) return [];
    return argDefs.map((argDef, idx) => {
        const value = rawArgs[idx] == null || rawArgs[idx] === '' ? (argDef.def == null ? '' : String(argDef.def)) : rawArgs[idx];
        return String(value);
    }).filter(v => v !== '');
}

function _paramTargetToLegacySelector(value) {
    const target = _normalizeTarget(String(value == null || value === '' ? 'both' : value));
    if (target === 't1') return 'p1';
    if (target === 't2') return 'p2';
    return 'both';
}

function _paramProgramBridgeParamsFromLegacyTransform(row) {
    const name = String((Array.isArray(row) ? row[0] : row) || '').trim();
    if (!name) return null;
    const args = Array.isArray(row) ? row.slice(1).map(v => String(v)) : [];
    if (_paramProgramIndependentLegacyTargets.has(name)) {
        const selector = _paramTargetToLegacySelector(args[0] || 'both');
        return [name, selector, selector, ''];
    }
    const [source, target, migratedArgs] = _migrateParamProgramLegacyTargetArgs(name, 'both', 'both', args, { force: true });
    const bridgeArgs = _paramProgramLegacyArgsFromInput(name, name === 'moebius' ? args : migratedArgs, { fillDefaults: !!_paramProgramLegacyArgDefs(name) });
    return _paramProgramLegacyArgDefs(name)
        ? [name, source, target, ...bridgeArgs]
        : [name, source, target, bridgeArgs.join(',')];
}

function _normalizeLegacyBridgeParams(legacyName, src, tgt, argsInput) {
    // No name fabrication: an empty legacy name must flow through and fail
    // backend validation with a coded unknown_legacy_transform, not silently
    // become a real transform (unit_circle) the user never wrote.
    const name = String(legacyName || '').trim();
    let source = String(src || 'both').trim() || 'both';
    let target = String(tgt || 'both').trim() || 'both';
    let rawArgs = Array.isArray(argsInput)
        ? argsInput.map(v => _str(v).trim())
        : _splitParamProgramLegacyArgs(argsInput);
    [source, target, rawArgs] = _migrateParamProgramLegacyTargetArgs(name, source, target, rawArgs);
    let args = _paramProgramLegacyArgsFromInput(name, rawArgs, { fillDefaults: !!_paramProgramLegacyArgDefs(name) });
    if (_paramProgramIndependentLegacyTargets.has(name) && args.length === 1) {
        const selector = _paramTargetToLegacySelector(args[0]);
        if (source === 'both' && target === 'both') {
            source = selector;
            target = selector;
        }
        args = [];
    }
    return [name, source, target, args];
}

function _normalizeParamProgramShortcutChip(name, args) {
    const chipName = String(name || '').trim();
    const chipArgs = Array.isArray(args) ? args.map(v => String(v)) : [];
    if (_paramProgramLegacyNames.includes(chipName) && !_ppCatalog[chipName]) {
        return [{ name: 'legacy', params: _paramProgramBridgeParamsFromLegacyTransform([chipName, ...chipArgs]) }];
    }
    if (!_paramProgramIndependentLegacyTargets.has(chipName) || chipArgs.length !== 1) {
        return [{ name: chipName, params: chipArgs }];
    }
    const selector = _paramTargetToLegacySelector(chipArgs[0]);
    if (selector === 'p1') {
        return [
            { name: 'push', params: ['t1'] },
            { name: chipName, params: [] },
            { name: 'emit', params: ['p1'] },
        ];
    }
    if (selector === 'p2') {
        return [
            { name: 'push', params: ['t2'] },
            { name: chipName, params: [] },
            { name: 'emit', params: ['p2'] },
        ];
    }
    return [
        { name: 'push', params: ['t1'] },
        { name: chipName, params: [] },
        { name: 'emit', params: ['p1'] },
        { name: 'push', params: ['t2'] },
        { name: chipName, params: [] },
        { name: 'emit', params: ['p2'] },
    ];
}

function _serializeParamProgramChain() {
    return _ppChain.map(item => {
        if (!item || !item.name) return null;
        const preserved = _serializeSavedRowIfPristine(item, _normalizeParamProgramChainItem);
        if (preserved) return preserved;
        const name = String(item.name || '').trim();
        const params = Array.isArray(item.params) ? item.params.map(v => String(v)) : [];
        if (name === 'legacy') {
            const [legacyName, src, tgt, args] = _normalizeLegacyBridgeParams(
                params[0],
                params[1],
                params[2],
                params.slice(3)
            );
            return ['legacy', legacyName, src, tgt, ...args];
        }
        if (name === 'const') {
            if (params.length >= 2) {
                const re = String(params[0] || '0').trim() || '0';
                const im = String(params[1] || '0').trim() || '0';
                return ['const', `(${re})+(${im})*1j`];
            }
            const value = String(params[0] || '0').trim() || '0';
            return ['const', value];
        }
        const spec = _ppCatalog[name] || {};
        const pDefs = spec.params || [];
        const values = pDefs.map((pDef, idx) => {
            const raw = params[idx] != null && params[idx] !== '' ? params[idx] : (pDef.def == null ? '' : pDef.def);
            return String(raw);
        }).filter(v => v !== '');
        return values.length ? [name, ...values] : [name];
    }).filter(Boolean);
}

function _selectedParamPipelineMode() {
    _paramPipelineMode = 'program';
    return 'program';
}

function _paramProgramModeSelected() {
    return true;
}

function _formatChainRowsForLog(chain, separator = ',') {
    const rows = Array.isArray(chain) ? chain : [];
    return rows.length
        ? rows.map(a => Array.isArray(a) && a.length > 1 ? `${a[0]}(${a.slice(1).join(',')})` : (Array.isArray(a) ? a[0] : String(a))).filter(Boolean).join(separator)
        : '';
}

function _formatParamProgramChainForLog(chain, separator = ',') {
    return _formatChainRowsForLog(chain, separator) || 'param_program(identity)';
}

function _paramProgramSourceStatements(sourceText) {
    return _coeffProgramSourceStatements(sourceText);
}

function _paramProgramSourceStatementCount(sourceText) {
    return _paramProgramSourceStatements(sourceText).length;
}

function _paramProgramSourceDisplay(sourceText, separator = ',') {
    const rows = _paramProgramSourceStatements(sourceText);
    if (!rows.length) return 'param_program(identity)';
    const head = rows.slice(0, 6).join(separator);
    return rows.length > 6 ? `${head}${separator}...` : head;
}

function _displayActiveParamPipeline(separator = ',') {
    const textMode = typeof _paramProgramTextModeSelected === 'function' && _paramProgramTextModeSelected();
    if (textMode) {
        return _paramProgramSourceDisplay(_getParamProgramSourceText(), separator);
    }
    return _formatParamProgramChainForLog(_serializeParamProgramChain(), separator);
}

function _formatCoeffProgramChainForLog(chain, separator = ',') {
    return _formatChainRowsForLog(chain, separator) || 'coeff_program(identity)';
}

function _coeffProgramSourceStatements(sourceText) {
    // Mirrors split_coeff_program_statements in lambda/coeff_program_source.py:
    // statements split on newlines AND semicolons at bracket depth zero, with
    // # comments stripped, so counts and displays match the backend.
    const text = String(sourceText == null ? '' : sourceText);
    const statements = [];
    let buffer = '';
    let depth = 0;
    let inComment = false;
    for (const ch of text) {
        if (inComment) {
            if (ch === '\n') {
                inComment = false;
                if (depth === 0) { if (buffer.trim()) statements.push(buffer.trim()); buffer = ''; }
                else buffer += ch;
            }
            continue;
        }
        if (ch === '#') { inComment = true; continue; }
        if (ch === '(' || ch === '[') depth++;
        else if (ch === ')' || ch === ']') depth = Math.max(0, depth - 1);
        if ((ch === ';' || ch === '\n') && depth === 0) {
            if (buffer.trim()) statements.push(buffer.trim());
            buffer = '';
            continue;
        }
        buffer += ch;
    }
    if (buffer.trim()) statements.push(buffer.trim());
    return statements;
}

function _coeffProgramSourceStatementCount(sourceText) {
    return _coeffProgramSourceStatements(sourceText).length;
}

function _coeffProgramSourceDisplay(sourceText, separator = ',') {
    const rows = _coeffProgramSourceStatements(sourceText);
    if (!rows.length) return 'coeff_program(identity)';
    // Preview cap: 6 statements (the modal list shows a short head only).
    const head = rows.slice(0, 6).join(separator);
    return rows.length > 6 ? `${head}${separator}...` : head;
}

function _displayActiveCoeffPipeline(separator = ',') {
    // typeof guard: the frontend test harness executes this function in
    // isolation, without the rest of the file's declarations.
    const textMode = typeof _coeffProgramTextModeSelected === 'function' && _coeffProgramTextModeSelected();
    if (textMode) {
        return _coeffProgramSourceDisplay(_getCoeffProgramSourceText(), separator);
    }
    return _formatCoeffProgramChainForLog(_serializeCoeffProgramChain(), separator);
}

function _syncParamPipelineModeUi() {
    const mode = 'program';
    _paramPipelineMode = mode;
    const programBox = document.querySelector('.param-program-box');
    const coeffProgramBox = document.querySelector('.coeff-program-box');
    if (programBox && programBox.classList) programBox.classList.remove('param-pipeline-inactive');
    if (coeffProgramBox && coeffProgramBox.classList) coeffProgramBox.classList.remove('param-pipeline-inactive');
}

function _setParamPipelineMode(mode, options = {}) {
    _paramPipelineMode = 'program';
    _syncParamPipelineModeUi();
    if (options.markStale) _markComputePreviewStale();
}

// Compute is program-only. Legacy transform rows are no longer an editable
// compute path; old payloads are translated at the Lambda boundary.
function _effectiveParamTransformsForCompute() {
    return [];
}

function _effectiveParamProgramChainForCompute() {
    return [];
}

function _effectiveParamProgramSourceTextForCompute() {
    const sourceText = _getParamProgramSourceText();
    return sourceText.trim() ? sourceText : null;
}

function _effectiveCoeffTransformsForCompute() {
    return [];
}

function _effectiveCoeffProgramChainForCompute() {
    return [];
}

function _effectiveCoeffProgramSourceTextForCompute() {
    const sourceText = _getCoeffProgramSourceText();
    return sourceText.trim() ? sourceText : null;
}

function _attachCoeffProgramSourcePayload(payload) {
    const sourceText = _effectiveCoeffProgramSourceTextForCompute();
    if (sourceText !== null) payload.coeff_program_source_text = sourceText;
    return payload;
}

function _attachProgramSourcePayload(payload) {
    const paramSourceText = _effectiveParamProgramSourceTextForCompute();
    if (paramSourceText !== null) payload.param_program_source_text = paramSourceText;
    return _attachCoeffProgramSourcePayload(payload);
}

function _paramProgramStatus(message, isError = false) {
    const el = document.getElementById('param-program-status');
    if (!el) return;
    el.textContent = String(message || '');
    el.className = `solve-score-program-status${isError ? ' error' : ''}`;
}

function _paramProgramDefaultName() {
    try {
        if (_paramProgramTextModeSelected()) {
            const basis = _paramProgramSourceDisplay(_getParamProgramSourceText(), '-');
            return basis.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '').slice(0, 48) || 'param-program';
        }
        const chain = _serializeParamProgramChain();
        const basis = chain.length ? chain.map(row => Array.isArray(row) ? row[0] : String(row)).join('-') : 'param-program';
        return basis.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '').slice(0, 48) || 'param-program';
    } catch (_) {
        return 'param-program';
    }
}

function _normalizeParamProgramChainItem(item) {
    if (Array.isArray(item) && item.length) {
        const name = String(item[0] || '').trim();
        if (!name) return [];
        if (name === 'const' && item.length >= 3) {
            const re = String(item[1] == null ? '0' : item[1]).trim() || '0';
            const im = String(item[2] == null ? '0' : item[2]).trim() || '0';
            return [{ name: 'const', params: [`(${re})+(${im})*1j`] }];
        }
        if (name === 'legacy') {
            const [legacyName, src, tgt, args] = _normalizeLegacyBridgeParams(
                item[1],
                item[2],
                item[3],
                item.slice(4).map(v => String(v))
            );
            const params = _paramProgramLegacyArgDefs(legacyName)
                ? [legacyName, src, tgt, ...args]
                : [legacyName, src, tgt, args.join(',')];
            return [{ name: 'legacy', params }];
        }
        return _normalizeParamProgramShortcutChip(name, item.slice(1));
    }
    if (item == null || item === '') return [];
    return [{ name: String(item), params: [] }];
}

function _normalizeParamProgramChain(rawChain) {
    if (!Array.isArray(rawChain)) return [];
    return rawChain.flatMap(item => {
        const chips = _normalizeParamProgramChainItem(item);
        // Spelling preservation: hydration can respell equivalent forms
        // (moebius 8-packed -> 4-complex), which changes the fingerprint of
        // an UNEDITED program on load->save. Stash the original row so the
        // serializer can emit it verbatim while the chip stays pristine.
        if (Array.isArray(item) && chips.length === 1 && chips[0] && chips[0].name) {
            chips[0].savedRow = item.slice();
        }
        return chips;
    }).filter(Boolean);
}

function _serializeSavedRowIfPristine(item, normalizeItem) {
    // Emit the original saved row verbatim when re-hydrating it reproduces
    // the chip's current state (i.e. the user never edited it) — spelling,
    // and therefore the compiled fingerprint, is preserved.
    if (!item || !Array.isArray(item.savedRow)) return null;
    const rehydrated = normalizeItem(item.savedRow);
    if (rehydrated.length !== 1 || !rehydrated[0] || rehydrated[0].name !== item.name) return null;
    const current = (Array.isArray(item.params) ? item.params : []).map(v => String(v));
    const original = (rehydrated[0].params || []).map(v => String(v));
    if (JSON.stringify(original) !== JSON.stringify(current)) return null;
    return item.savedRow.slice();
}

function _validateParamProgramUiChain(chain) {
    // Import-time analog of _validateCoeffProgramUiChain: unknown chips must
    // fail at import with context, not load silently and die at backend
    // compile.
    (chain || []).forEach((item, idx) => {
        const name = String(item && item.name || '').trim();
        if (!name || !_ppCatalog[name]) {
            throw new Error(`unknown param program chip at ${idx}: ${name || '(empty)'}`);
        }
        if (name === 'legacy') {
            const legacyName = String(Array.isArray(item.params) ? item.params[0] : '').trim();
            if (!legacyName || !_paramProgramLegacyNames.includes(legacyName)) {
                throw new Error(`unknown legacy param transform at ${idx}: ${legacyName || '(empty)'}`);
            }
        }
    });
}

function _parseParamProgramPayload(raw) {
    if (!raw || typeof raw !== 'object') throw new Error('program JSON must be an object');
    const hasSourceText = Object.prototype.hasOwnProperty.call(raw, 'source_text')
        && String(raw.source_text || '').trim() !== '';
    const chain = Array.isArray(raw.chain) ? raw.chain : [];
    if (!hasSourceText && !chain.length) throw new Error('program JSON is missing a chain or source_text');
    const hasVersion = Object.prototype.hasOwnProperty.call(raw, 'version');
    const parsedVersion = Number(raw.version);
    if (hasVersion && !Number.isFinite(parsedVersion)) throw new Error('program JSON version must be numeric when present');
    const version = hasVersion ? parsedVersion : 1;
    if (version !== 1 && version !== 2) throw new Error(`program JSON version ${version} is not supported`);
    const kind = String(raw.program_kind || 'param_program');
    if (kind !== 'param_program') throw new Error(`program JSON kind ${kind} is not param_program`);
    const normalizedChain = hasSourceText ? [] : _normalizeParamProgramChain(chain);
    if (!hasSourceText) _validateParamProgramUiChain(normalizedChain);
    return {
        version,
        program_kind: 'param_program',
        name: String(raw.name || '').trim(),
        description: String(raw.description || '').trim(),
        chain: normalizedChain,
        has_source_text: hasSourceText,
        source_text: hasSourceText ? String(raw.source_text || '') : '',
    };
}

function _applyParamProgram(rawProgram) {
    const program = _parseParamProgramPayload(rawProgram);
    _ppChain.splice(0, _ppChain.length, ...program.chain);
    if (program.has_source_text) {
        _setParamProgramSourceText(program.source_text);
        _paramProgramSourceAutoSynthed = false;
        _setParamProgramEditorMode('text');
    } else {
        _setParamProgramSourceText(_paramProgramSourceFromRows(_serializeParamProgramChain()), { auto: true });
        _setParamProgramEditorMode('text');
    }
    if (_paramProgramModeSelected()) _markComputePreviewStale();
    _paramProgramStatus(program.name ? `Loaded ${program.name}` : 'Loaded param program');
    return program;
}

function _clearParamProgramChain() {
    _ppChain.splice(0, _ppChain.length);
    _setParamProgramSourceText('');
    _paramProgramSourceAutoSynthed = false;
    if (_paramProgramModeSelected()) _markComputePreviewStale();
}

function _portableParamProgramPayload(nameOverride = '') {
    const sourceText = _paramProgramTextModeSelected() ? _getParamProgramSourceText() : '';
    const chain = _serializeParamProgramChain();
    if (!chain.length && !sourceText.trim()) throw new Error('Param program is empty');
    const payload = {
        version: 1,
        program_kind: 'param_program',
        name: String(nameOverride || _paramProgramDefaultName()).trim() || 'param-program',
        chain: sourceText.trim() ? [] : chain,
        display: sourceText.trim()
            ? sourceText.split(/\n+/).map(line => line.trim()).filter(Boolean).slice(0, 4).join('; ')
            : _chainDisplayString(chain),
    };
    if (sourceText.trim()) payload.source_text = sourceText;
    return payload;
}

function _normalizeCoeffProgramChain(rawChain) {
    if (!Array.isArray(rawChain)) return [];
    return rawChain.flatMap(item => {
        const chips = _normalizeCoeffProgramChainItem(item);
        // Spelling preservation — see _normalizeParamProgramChain: hydration
        // respells equivalent forms (packed linear/pow, native+andy ->
        // legacy chip); stash the original row so a pristine chip serializes
        // back to the identical fingerprint.
        if (Array.isArray(item) && chips.length === 1 && chips[0] && chips[0].name) {
            chips[0].savedRow = item.slice();
        }
        return chips;
    }).filter(Boolean);
}

function _normalizeCoeffProgramChainItem(item) {
    {
        if (Array.isArray(item) && item.length) {
            const name = String(item[0] || '').trim();
            if (!name) return [];
            if (name === 'legacy') {
                // No name fabrication (see _normalizeLegacyBridgeParams).
                const legacyName = _canonicalCoeffTransformName(item[1] || '') || String(item[1] || '').trim();
                const params = [legacyName, ...item.slice(2).map(v => String(v))];
                if (legacyName === 'linear' && params.length >= 7) {
                    params.splice(
                        3,
                        4,
                        _formatCoeffTransformComplexParts(params[3], params[4]),
                        _formatCoeffTransformComplexParts(params[5], params[6]),
                    );
                }
                if (legacyName === 'pow' && params.length >= 7) {
                    params.splice(
                        3,
                        4,
                        _formatCoeffTransformComplexParts(params[3], params[4]),
                        _formatCoeffTransformComplexParts(params[5], params[6]),
                    );
                }
                if (legacyName === 'round' && params.length >= 5) {
                    params.splice(
                        3,
                        2,
                        _formatCoeffTransformComplexParts(params[3], params[4]),
                    );
                }
                // The named-chip catalogs filter out the andy param, so a
                // legacy row carrying a non-default andy must stay a legacy
                // chip or the andy is silently dropped on the next serialize.
                const fullDefs = _coeffProgramLegacyInputDefs(legacyName);
                const legacyArgs = params.slice(3);
                const lastDef = fullDefs.length ? fullDefs[fullDefs.length - 1] : null;
                const carriesAndy = _isAndyParam(lastDef)
                    && legacyArgs.length === fullDefs.length
                    && !_ctAndyIsDefault(legacyArgs[legacyArgs.length - 1]);
                // LAYOUT CONTRACT: legacy rows are source-first
                // ['legacy', name, src, tgt, ...args]; named chip params are
                // target-first [tgt, src, ...args]. Converting between the
                // two REORDERS — keep the named locals, never bare indexes.
                const legacySrc = params[1] || 'poly';
                const legacyTgt = params[2] || 'poly';
                if (carriesAndy) {
                    return [{ name: 'legacy', params: [legacyName, legacySrc, legacyTgt, ...legacyArgs] }];
                }
                return [{ name: _coeffProgramRegistryChipName(legacyName), params: [legacyTgt, legacySrc, ...legacyArgs] }];
            }
            // Unary native transforms (sin/cos/...) with a trailing andy have
            // andy-less catalog params; keep them as legacy chips too.
            const rest = item.slice(1).map(v => String(v));
            if (_coeffProgramVectorUnaryNames.includes(name) && name !== 'exp' && rest.length > 2
                && _ctCatalog[name] && !_ctAndyIsDefault(rest[rest.length - 1])) {
                // Chip rows are target-first; the legacy row being built is
                // source-first (see LAYOUT CONTRACT above).
                const [chipTgt, chipSrc, ...unaryArgs] = rest;
                return [{ name: 'legacy', params: [name, chipSrc || 'poly', chipTgt || 'poly', ...unaryArgs] }];
            }
            const sourceName = name === 'const' ? 'push_const' : (name === 'exp' && item.length > 3 ? 'exp_affine' : name);
            return [{ name: sourceName, params: item.slice(1).map(v => String(v)) }];
        }
        if (item == null || item === '') return [];
        const name = String(item).trim() === 'const' ? 'push_const' : String(item).trim();
        return [{ name, params: [] }];
    }
}

function _coeffProgramStatus(message, isError = false) {
    const el = document.getElementById('coeff-program-status');
    if (!el) return;
    el.textContent = String(message || '');
    el.className = `solve-score-program-status${isError ? ' error' : ''}`;
}

function _coeffProgramDefaultName() {
    try {
        if (_coeffProgramTextModeSelected()) {
            const basis = _coeffProgramSourceDisplay(_getCoeffProgramSourceText(), '-');
            return basis.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '').slice(0, 48) || 'coeff-program';
        }
        const chain = _serializeCoeffProgramChain();
        const basis = chain.length ? chain.map(row => Array.isArray(row) ? row[0] : String(row)).join('-') : 'coeff-program';
        return basis.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '').slice(0, 48) || 'coeff-program';
    } catch (_) {
        return 'coeff-program';
    }
}

function _serializeCoeffProgramChain() {
    return _coeffProgramChain.map(item => {
        if (!item || !item.name) return null;
        const preserved = _serializeSavedRowIfPristine(item, _normalizeCoeffProgramChainItem);
        if (preserved) return preserved;
        const name = String(item.name || '').trim();
        const params = Array.isArray(item.params) ? item.params.map(v => _str(v)) : [];
        if (name === 'legacy') {
            // No name fabrication (see _normalizeLegacyBridgeParams).
            const legacyName = _canonicalCoeffTransformName(params[0] || '') || String(params[0] || '').trim();
            const pDefs = _coeffProgramLegacyInputDefs(legacyName);
            const values = pDefs.map((pDef, idx) => params[idx + 3] !== undefined && params[idx + 3] !== ''
                ? params[idx + 3]
                : String(pDef.def || ''));
            let last = values.length - 1;
            if (last >= 0 && _isAndyParam(pDefs[last]) && _ctAndyIsDefault(values[last])) last--;
            return ['legacy', legacyName, params[1] || 'poly', params[2] || 'poly', ...values.slice(0, last + 1)];
        }
        const catalogName = _canonicalCoeffProgramChipName(name);
        const spec = _coeffProgramCatalog[catalogName] || {};
        const pDefs = spec.params || [];
        const values = pDefs.map((pDef, idx) => _paramValueOrDefault(params, idx, pDef));
        let last = values.length - 1;
        if (spec.nativeTransform) {
            // linear compiles via the strict affine chip (tgt, src, multiplier,
            // offset); the backend rejects default-trimmed forms, so emit all args.
            if (catalogName === 'linear') return [catalogName, ...values];
            while (last >= 2 && (values[last] === '' || values[last] === String((pDefs[last] && pDefs[last].def) || ''))) last--;
            return [catalogName, ...values.slice(0, Math.max(last + 1, 2))];
        }
        return last >= 0 ? [catalogName, ...values.slice(0, last + 1)] : [catalogName];
    }).filter(Boolean);
}

function _validateCoeffProgramUiChain(chain) {
    (chain || []).forEach((item, idx) => {
        const name = String(item && item.name || '').trim();
        if (!name || !_coeffProgramCatalog[name]) {
            throw new Error(`unknown coeff program chip at ${idx}: ${name || '(empty)'}`);
        }
        if (name === 'legacy') {
            const legacyName = _canonicalCoeffTransformName(Array.isArray(item.params) ? item.params[0] : '');
            if (!legacyName || !_ctCatalog[legacyName]) {
                throw new Error(`unknown legacy coefficient transform at ${idx}: ${legacyName || '(empty)'}`);
            }
        }
    });
}

function _parseCoeffProgramPayload(raw) {
    if (!raw || typeof raw !== 'object') throw new Error('program JSON must be an object');
    // Key presence alone is not enough: an empty source_text with a non-empty
    // chain must load as a chain program, not silently discard the chain.
    const hasSourceText = Object.prototype.hasOwnProperty.call(raw, 'source_text')
        && String(raw.source_text || '').trim() !== '';
    const chain = Array.isArray(raw.chain) ? raw.chain : [];
    if (!hasSourceText && !chain.length) throw new Error('program JSON is missing a chain or source_text');
    const version = Object.prototype.hasOwnProperty.call(raw, 'version') ? Number(raw.version) : 1;
    if (!Number.isFinite(version)) throw new Error('program JSON version must be numeric when present');
    if (version !== 1 && version !== 2) throw new Error(`program JSON version ${version} is not supported`);
    const kind = String(raw.program_kind || 'coeff_program');
    if (kind !== 'coeff_program') throw new Error(`program JSON kind ${kind} is not coeff_program`);
    const normalizedChain = hasSourceText ? [] : _normalizeCoeffProgramChain(chain);
    if (!hasSourceText) _validateCoeffProgramUiChain(normalizedChain);
    return {
        version,
        program_kind: 'coeff_program',
        name: String(raw.name || '').trim(),
        description: String(raw.description || '').trim(),
        chain: normalizedChain,
        has_source_text: hasSourceText,
        source_text: hasSourceText ? String(raw.source_text || '') : '',
    };
}

function _applyCoeffProgram(rawProgram) {
    const program = _parseCoeffProgramPayload(rawProgram);
    _coeffProgramChain.splice(0, _coeffProgramChain.length, ...program.chain);
    if (program.has_source_text) {
        _setCoeffProgramSourceText(program.source_text);
        _coeffProgramSourceAutoSynthed = false;
        _setCoeffProgramEditorMode('text');
    } else {
        _setCoeffProgramSourceText(_coeffProgramSourceFromRows(_serializeCoeffProgramChain()), { auto: true });
        _setCoeffProgramEditorMode('text');
    }
    if (_paramProgramModeSelected()) _markComputePreviewStale();
    _coeffProgramStatus(program.name ? `Loaded ${program.name}` : 'Loaded coeff program');
    return program;
}

function _clearCoeffProgramChain() {
    _coeffProgramChain.splice(0, _coeffProgramChain.length);
    _setCoeffProgramSourceText('');
    if (_paramProgramModeSelected()) _markComputePreviewStale();
}

function _portableCoeffProgramPayload(nameOverride = '') {
    const sourceText = _coeffProgramTextModeSelected() ? _getCoeffProgramSourceText() : '';
    const chain = _serializeCoeffProgramChain();
    if (!chain.length && !sourceText.trim()) throw new Error('Coeff program is empty');
    const payload = {
        version: 1,
        program_kind: 'coeff_program',
        name: String(nameOverride || _coeffProgramDefaultName()).trim() || 'coeff-program',
        chain: sourceText.trim() ? [] : chain,
        display: sourceText.trim()
            // Preview cap: 4 lines for the compact payload display.
            ? sourceText.split(/\n+/).map(line => line.trim()).filter(Boolean).slice(0, 4).join('; ')
            : _chainDisplayString(chain),
    };
    if (sourceText.trim()) payload.source_text = sourceText;
    return payload;
}

// Validates one chip-param edit against its catalog param def. Returns
// { value } to store — plus { valueNext } when a complex constant spans the
// following slot — or { error } with the alert text. Kind checks run in
// catalog order: target, choices, then the cp/ct kind flags.
function _chipReadonlyValueHtml(value, paramDef = {}, title = '') {
    const pd = paramDef || {};
    const fallback = pd.def == null ? '' : pd.def;
    const text = String(value == null || value === '' ? fallback : value);
    const tooltip = title || pd.title || '';
    const titleAttr = tooltip ? ` title="${_escapeHtml(tooltip)}"` : '';
    const clsParts = ['chip-input', 'chip-input-readonly'];
    if (pd.selectorWide) clsParts.push('chip-input-selector-wide');
    if (pd.functionWide) clsParts.push('chip-input-function-wide');
    if (pd.complexWide) clsParts.push('chip-input-complex-wide');
    if (pd.exprWide) clsParts.push('chip-input-expr-wide');
    if (pd.programWide) clsParts.push('chip-input-program-wide');
    if (pd.paramProgramWide) clsParts.push('chip-input-param-program-wide');
    if (pd.wide === true || (pd.scalarExpr && !pd.complexWide && !pd.exprWide)) clsParts.push('chip-input-wide');
    if (_isAndyParam(pd)) clsParts.push('chip-input-andy');
    return `<span class="${clsParts.join(' ')}"${titleAttr}>${_escapeHtml(text)}</span>`;
}

// Chips are read-only everywhere; params render as static value spans.
function _chipInputHtml(which, chipIdx, paramIdx, value, paramDef, options = {}) {
    return _chipReadonlyValueHtml(value, paramDef || {});
}

function _chipLabeledInputHtml(which, chipIdx, paramIdx, value, paramDef, options = {}) {
    const pd = paramDef || {};
    const label = String(pd.label || pd.ph || '').trim();
    const input = _chipInputHtml(which, chipIdx, paramIdx, value, pd, options);
    if (!label) return input;
    return `<span class="chip-param-pair"><span class="chip-param-name">${_escapeHtml(label)}=</span>${input}</span>`;
}

function _solveScoreChipShell(which, i, bodyHtml, tooltipAttr = '', options = {}) {
    return `<span class="chip score-chip score-chip-readonly"${tooltipAttr}>${bodyHtml}</span>`;
}

function _solveScoreInheritedLagQuantile(which, chipIdx, item, options = {}) {
    const current = _solveScoreItemMetricDetails(item);
    if (!current || current.lag !== 1) return null;
    const chain = Array.isArray(options.chain) ? options.chain : _chainForWhich(which);
    const candidates = chain
        .slice(0, chipIdx)
        .map(row => ({ row, details: _solveScoreItemMetricDetails(row) }))
        .filter(entry => entry.details && entry.details.metric === current.metric && entry.details.source === current.source && entry.details.lag === 0);
    return candidates.length === 1 ? candidates[0].details.q : null;
}

;(window.__ppParts = window.__ppParts || []).push('08-chip-editors');
