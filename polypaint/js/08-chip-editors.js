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

function _chipMoveControlsHtml(which, idx) {
    if (!['rt', 'palette-rt'].includes(which)) return '';
    const count = _chainForWhich(which).length;
    const leftDisabled = idx <= 0 ? ' disabled' : '';
    const rightDisabled = idx >= count - 1 ? ' disabled' : '';
    return `<span class="chip-actions"><button type="button" class="chip-move" onclick="event.stopPropagation();moveChip('${which}',${idx},-1)" title="Move left"${leftDisabled}>&lt;</button><button type="button" class="chip-move" onclick="event.stopPropagation();moveChip('${which}',${idx},1)" title="Move right"${rightDisabled}>&gt;</button></span>`;
}

function selectParamProgramLine(idx, eventObj) {
    if (eventObj && eventObj.stopPropagation) eventObj.stopPropagation();
    _paramProgramSelectedIndex = Number(idx);
    _renderChips('pp');
}

function selectCoeffProgramLine(idx, eventObj) {
    if (eventObj && eventObj.stopPropagation) eventObj.stopPropagation();
    _coeffProgramSelectedIndex = Number(idx);
    _renderChips('cp');
}

let _paramProgramSelectedIndex = -1;
let _paramProgramEditorMode = 'text';
let _coeffProgramSelectedIndex = -1;
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
            const target = params[0] || 'p1';
            lines.push(target === 'p2' ? 'emit_p2' : 'emit_p1');
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

let _paramProgramSourceValidationTimer = null;
let _paramProgramSourceValidationSeq = 0;
function _scheduleParamProgramSourceValidation() {
    if (_paramProgramSourceValidationTimer) clearTimeout(_paramProgramSourceValidationTimer);
    _paramProgramSourceValidationTimer = setTimeout(async () => {
        _paramProgramSourceValidationTimer = null;
        const sourceText = _getParamProgramSourceText();
        if (!sourceText.trim() || !_paramProgramTextModeSelected()) return;
        const seq = ++_paramProgramSourceValidationSeq;
        try {
            const resp = await lambdaPost('storage', { source_text: sourceText }, '/compile-param-program-source');
            if (seq !== _paramProgramSourceValidationSeq || sourceText !== _getParamProgramSourceText()) return;
            if (resp && resp.ok) {
                _paramProgramStatus(`Text source OK: ${resp.statement_count} statement${resp.statement_count === 1 ? '' : 's'}, ${resp.program && resp.program.token_count || 0} tokens.`);
            } else if (resp && Array.isArray(resp.diagnostics)) {
                const first = resp.diagnostics.find(d => d && d.level === 'error') || resp.diagnostics[0];
                if (first) _paramProgramStatus(`Line ${first.line}: ${first.message}`, true);
            }
        } catch (_) {
            /* advisory only */
        }
    }, 900);
}

function _onParamProgramSourceInput() {
    _paramProgramSourceAutoSynthed = false;
    _scheduleParamProgramSourceValidation();
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
        title: 'Constants + Ranges',
        buttons: [
            { label: 'push_vec(value)', snippet: 'push_vec(0)\nemit', title: 'Push a constant vector of length poly_len.' },
            { label: 'push_vec(n,value)', snippet: 'push_vec(poly_len, p1)\nemit', title: 'Push a constant vector with explicit length.' },
            { label: 'fill', snippet: 'fill(poly_len, 0)\nemit', title: 'Alias for push_vec/fill vector construction.' },
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

let _programSourceSidePanelMode = { pp: 'starter', cp: 'starter' };
let _programHelpRegistryCache = { pp: null, cp: null };
let _programHelpInspectorBound = false;

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

function _programHelpRegistry(which) {
    const key = which === 'cp' ? 'cp' : 'pp';
    if (!_programHelpRegistryCache[key]) {
        _programHelpRegistryCache[key] = key === 'cp'
            ? _programHelpBuildCoeffRegistry()
            : _programHelpBuildParamRegistry();
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
    const key = which === 'cp' ? 'cp' : 'pp';
    const el = document.getElementById(`${key}-help`);
    if (!el) return;
    const registry = _programHelpRegistry(key);
    el.innerHTML = (registry.sections || []).map(section => _programHelpSectionHtml(key, section)).join('');
}

function _setProgramSourceSidePanelMode(which, mode) {
    const key = which === 'cp' ? 'cp' : 'pp';
    _programSourceSidePanelMode[key] = mode === 'help' ? 'help' : 'starter';
    _renderProgramSourceSidePanel(key);
}

function _renderProgramSourceSidePanel(which) {
    const key = which === 'cp' ? 'cp' : 'pp';
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
}

function _programSourceTextarea(which) {
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
    const registry = _programHelpRegistry(which === 'cp' ? 'cp' : 'pp');
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
    const x = Number(raw.clientX || 0) || 12;
    const y = Number(raw.clientY || 0) || 12;
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

function _onProgramSourceDblClick(which, event) {
    const key = which === 'cp' ? 'cp' : 'pp';
    const textarea = _programSourceTextarea(key);
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

function _renderParamCoeffProgramCheatsheets() {
    _renderParamProgramCheatsheet();
    _renderCoeffProgramCheatsheet();
    _renderProgramSourceSidePanels();
}

let _solveScoreProgramEditorMode = { render: 'text', palette: 'text' };
let _rootProgramEditorMode = { render: 'chips', palette: 'chips' };

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

function _setRootProgramSourceText(prefix, text) {
    const el = _rootProgramSourceTextarea(prefix);
    const value = String(text == null ? '' : text);
    if (el && el.value !== value) el.value = value;
}

function _effectiveRootProgramSourceText(prefix) {
    const text = _getRootProgramSourceText(prefix);
    return text.trim() ? text : '';
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

function _setRootProgramEditorMode(prefix, mode) {
    const p = _editorPrefix(prefix);
    const normalized = mode === 'text' ? 'text' : 'chips';
    _rootProgramEditorMode[p] = normalized;
    _setPanelTabActive(`${p}-rt`, normalized);
}

function _onSolveScoreProgramSourceInput(prefix) {
    _setSolveScoreProgramStatus(_editorPrefix(prefix), 'Text source changed. Compile to validate; backend compiles it on render.', false);
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

function _renderSolveScoreCheatsheet(prefix) {
    const p = _editorPrefix(prefix);
    const el = document.getElementById(`${p}-ss-cheatsheet`);
    if (!el) return;
    const metrics = _solveScoreMetricNames.slice();
    const starters = (_solveScoreStarterSnippets.length ? _solveScoreStarterSnippets : [
        { label: 'score = proximity', snippet: 'score = metric(proximity, slv, q=0.1%)' },
        { label: 'emit_norm proximity', snippet: 'emit_norm(metric(proximity, slv, q=0.1%))' },
        { label: 'two channels', snippet: 'emit_norm(metric(proximity, slv, q=0.1%))\nemit_norm(metric(spread, slv, q=0.1%))' },
    ]).map(item => _solveScoreCheatButtonHtml(p, item.label, item.snippet));
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
    el.innerHTML = [
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
    const statusId = _editorPrefix(prefix) === 'palette' ? 'palette-status' : 'render-status';
    const el = document.getElementById(statusId);
    if (el) {
        el.textContent = 'Root transform text changed. Compile to refresh chip preview; backend compiles it on render.';
        el.className = 'status';
    }
}

function _rootWhichForPrefix(prefix) {
    return _editorPrefix(prefix) === 'palette' ? 'palette-rt' : 'rt';
}

function _rootSourceFromRows(chain) {
    return (chain || []).map(item => {
        if (Array.isArray(item) && item.length) {
            return item.length > 1 ? `${item[0]}(${item.slice(1).join(', ')})` : String(item[0]);
        }
        if (item && typeof item === 'object') {
            const name = String(item.name || '').trim();
            const params = Array.isArray(item.params) ? item.params : (Array.isArray(item.args) ? item.args : []);
            return params.length ? `${name}(${params.join(', ')})` : name;
        }
        return String(item || '');
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
        _renderChips(which);
        _setSolveScoreProgramEditorMode(p, 'text');
        _setSolveScoreProgramStatus(p, `Text source OK: ${program.statement_count || resp.statement_count || chain.length} statement${(program.statement_count || resp.statement_count || chain.length) === 1 ? '' : 's'}.`);
        return program;
    } catch (e) {
        _setSolveScoreProgramStatus(p, e && e.message ? e.message : String(e), true);
        throw e;
    }
}

async function _compileRootSourceEditor(prefix) {
    const p = _editorPrefix(prefix);
    const sourceText = _getRootProgramSourceText(p);
    if (!sourceText.trim()) return null;
    const statusId = p === 'palette' ? 'palette-status' : 'render-status';
    const statusEl = document.getElementById(statusId);
    try {
        const resp = await lambdaPost('storage', { source_text: sourceText, strict: true }, '/compile-root-program-source');
        if (!resp || !resp.ok) {
            const first = resp && Array.isArray(resp.diagnostics)
                ? (resp.diagnostics.find(d => d && d.level === 'error') || resp.diagnostics[0])
                : null;
            throw new Error(first ? `Line ${first.line || '?'}: ${first.message}` : 'root source did not compile');
        }
        const chain = Array.isArray(resp.root_transforms) ? resp.root_transforms : (Array.isArray(resp.chain) ? resp.chain : []);
        const which = _rootWhichForPrefix(p);
        const target = _chainForWhich(which);
        target.splice(0, target.length, ...chain.map(item => {
            if (Array.isArray(item) && item.length) return { name: item[0], params: item.slice(1).map(v => String(v)) };
            return item && typeof item === 'object' ? { name: item.name, params: (item.params || item.args || []).map(v => String(v)) } : null;
        }).filter(Boolean));
        _renderChips(which);
        _setRootProgramEditorMode(p, 'text');
        if (statusEl) {
            statusEl.textContent = `Root source OK: ${resp.statement_count || chain.length} statement${(resp.statement_count || chain.length) === 1 ? '' : 's'}.`;
            statusEl.className = 'status ok';
        }
        return resp.program || resp;
    } catch (e) {
        if (statusEl) {
            statusEl.textContent = e && e.message ? e.message : String(e);
            statusEl.className = 'status error';
        }
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
let _coeffProgramSourceValidationTimer = null;
let _coeffProgramSourceValidationSeq = 0;
function _scheduleCoeffProgramSourceValidation() {
    if (_coeffProgramSourceValidationTimer) clearTimeout(_coeffProgramSourceValidationTimer);
    _coeffProgramSourceValidationTimer = setTimeout(async () => {
        _coeffProgramSourceValidationTimer = null;
        const sourceText = _getCoeffProgramSourceText();
        if (!sourceText.trim() || !_coeffProgramTextModeSelected()) return;
        const seq = ++_coeffProgramSourceValidationSeq;
        try {
            const resp = await lambdaPost('storage', { source_text: sourceText }, '/compile-coeff-program-source');
            if (seq !== _coeffProgramSourceValidationSeq || sourceText !== _getCoeffProgramSourceText()) return;
            if (resp && resp.ok) {
                _coeffProgramStatus(`Text source OK: ${resp.statement_count} statement${resp.statement_count === 1 ? '' : 's'}, ${resp.program && resp.program.token_count || 0} tokens.`);
            } else if (resp && Array.isArray(resp.diagnostics)) {
                const first = resp.diagnostics.find(d => d && d.level === 'error') || resp.diagnostics[0];
                if (first) _coeffProgramStatus(`Line ${first.line}: ${first.message}`, true);
            }
        } catch (_) {
            /* advisory only; save/preview surface real errors */
        }
    }, 900);
}

function _onCoeffProgramSourceInput() {
    _coeffProgramSourceAutoSynthed = false;
    _scheduleCoeffProgramSourceValidation();
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

function _paramPipelineEditAffectsCompute(which) {
    if (which === 'cp' || which === 'pp') return true;
    return false;
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
    _renderChips('pp');
    _renderChips('cp');
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
    _paramProgramSelectedIndex = _ppChain.length ? Math.min(_ppChain.length - 1, Math.max(0, _paramProgramSelectedIndex)) : -1;
    _renderChips('pp');
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
    _paramProgramSelectedIndex = -1;
    _setParamProgramSourceText('');
    _paramProgramSourceAutoSynthed = false;
    _renderChips('pp');
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
    _coeffProgramSelectedIndex = _coeffProgramChain.length ? Math.min(_coeffProgramChain.length - 1, Math.max(0, _coeffProgramSelectedIndex)) : -1;
    _renderChips('cp');
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
    _coeffProgramSelectedIndex = -1;
    _setCoeffProgramSourceText('');
    _renderChips('cp');
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
function _validateChipParamValue(which, pDef, value) {
    const rawText = _str(value).trim();
    if (pDef && pDef.target) {
        const norm = _normalizeTarget(value);
        return norm ? { value: norm } : { error: `Invalid target: "${value}". Use t1, t2, or both.` };
    }
    if (pDef && Array.isArray(pDef.choices) && pDef.choices.length) {
        const norm = rawText.toLowerCase();
        return pDef.choices.includes(norm)
            ? { value: norm }
            : { error: `Invalid ${pDef.ph || 'value'}: "${value}". Use ${pDef.choices.join(' or ')}.` };
    }
    if (which === 'cp' && pDef) {
        if (pDef.lengthArg) {
            if (rawText.toLowerCase() === 'poly_len') return { value: 'poly_len' };
            const number = Number(rawText);
            const min = pDef.min == null ? 1 : Number(pDef.min);
            const max = pDef.max == null ? 256 : Number(pDef.max);
            if (!Number.isFinite(number) || !Number.isInteger(number) || number < min || number > max) {
                return { error: `Invalid ${pDef.label || pDef.ph || 'length'}: "${value}". Use poly_len or an integer in [${min}, ${max}].` };
            }
            return { value: String(number) };
        }
        if (pDef.intLiteral) {
            const number = Number(rawText);
            const min = pDef.min == null ? -Infinity : Number(pDef.min);
            const max = pDef.max == null ? Infinity : Number(pDef.max);
            if (!Number.isFinite(number) || !Number.isInteger(number) || number < min || number > max) {
                return { error: `Invalid ${pDef.label || pDef.ph || 'value'}: "${value}". Use an integer${Number.isFinite(min) || Number.isFinite(max) ? ` in [${min}, ${max}]` : ''}.` };
            }
            return { value: String(number) };
        }
        if (pDef.scalarExpr) {
            // Coeff Program scalar args are parsed by the compiler. Keep expressions
            // like p2, real(p1), or p1+conj(p2) intact instead of applying the
            // legacy numeric-only transform validator in the editor.
            return { value: rawText || String(pDef.def || '') };
        }
        if (_isAndyParam(pDef)) {
            const norm = _normalizeCtRealInput(value);
            return norm == null
                ? { error: `Invalid ${pDef.ph || 'value'}: "${value}". Use a finite numeric andy value.` }
                : { value: norm };
        }
        return { value: rawText || String(pDef.def || '') };
    }
    return { value };
}

function updateChipParam(chipIdx, paramIdx, value, which) {
    if (which === 'pp') {
        _paramProgramStatus('Param Program chips are read-only; edit the Text tab.');
        return;
    }
    if (which === 'cp') {
        _coeffProgramStatus('Coeff Program chips are read-only; edit the Text tab.');
        return;
    }
    const chain = _chainForWhich(which);
    if (chipIdx < 0 || chipIdx >= chain.length) return;
    const catalog = _catalogForChain(which);
    const spec = catalog[chain[chipIdx].name] || {};
    const pDef = (spec.params || [])[paramIdx];
    const result = _validateChipParamValue(which, pDef, value);
    if (result.error) { alert(result.error); return; }
    chain[chipIdx].params[paramIdx] = result.value;
    if (result.valueNext !== undefined && paramIdx + 1 < chain[chipIdx].params.length) {
        chain[chipIdx].params[paramIdx + 1] = result.valueNext;
    }
    _renderChips(which);
    if (_paramPipelineEditAffectsCompute(which)) _markComputePreviewStale();
}

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

function _chipInputHtml(which, chipIdx, paramIdx, value, paramDef, options = {}) {
    const pd = paramDef || {};
    if (options.readonly) return _chipReadonlyValueHtml(value, pd);
    const defaultExprTitle = pd.scalarExpr
        ? 'Expression. Registers: t1, t2, p1, p2. Constants: pi, pi2, pi2i. Functions: exp, real, imag, abs, mod.'
        : '';
    const titleAttr = (pd.title || defaultExprTitle) ? ` title="${_escapeHtml(pd.title || defaultExprTitle)}"` : '';
    if (pd.target) {
        const normalizedValue = _normalizeTarget(_str(value)) || _normalizeTarget(String(pd.def || 'both')) || 'both';
        const options = ['t1', 't2', 'both'].map(choice => {
            const selected = normalizedValue === choice ? ' selected' : '';
            return `<option value="${choice}"${selected}>${choice}</option>`;
        }).join('');
        return `<select class="chip-input chip-input-target"${titleAttr} onchange="updateChipParam(${chipIdx},${paramIdx},this.value,'${which}')" onclick="event.stopPropagation()">${options}</select>`;
    }
    if (Array.isArray(pd.choices) && pd.choices.length) {
        const normalizedValue = _str(value).trim().toLowerCase();
        const options = pd.choices.map(choice => {
            const selected = normalizedValue === choice ? ' selected' : '';
            return `<option value="${_escapeHtml(choice)}"${selected}>${_escapeHtml(choice)}</option>`;
        }).join('');
        const clsParts = ['chip-input'];
        if (pd.selectorWide) clsParts.push('chip-input-selector-wide');
        if (pd.functionWide) clsParts.push('chip-input-function-wide');
        if (pd.paramProgramWide) clsParts.push('chip-input-param-program-wide');
        if (pd.choiceWide || pd.wide === true) clsParts.push('chip-input-wide');
        return `<select class="${clsParts.join(' ')}"${titleAttr} onchange="updateChipParam(${chipIdx},${paramIdx},this.value,'${which}')" onclick="event.stopPropagation()">${options}</select>`;
    }
    const im = (pd.target || pd.choices || pd.complexPairNext || pd.complex || pd.scalarExpr || pd.lengthArg || _isAndyParam(pd)) ? 'text' : 'decimal';
    const clsParts = ['chip-input'];
    if (_isAndyParam(pd)) clsParts.push('chip-input-andy');
    if (pd.complexWide) clsParts.push('chip-input-complex-wide');
    if (pd.exprWide) clsParts.push('chip-input-expr-wide');
    if (pd.programWide) clsParts.push('chip-input-program-wide');
    if (pd.paramProgramWide) clsParts.push('chip-input-param-program-wide');
    if (pd.wide === true || (pd.scalarExpr && !pd.complexWide && !pd.exprWide)) clsParts.push('chip-input-wide');
    const cls = clsParts.join(' ');
    return `<input class="${cls}" type="text" inputmode="${im}" value="${_escapeHtml(value)}" placeholder="${_escapeHtml(pd.ph || '')}"${titleAttr} onchange="updateChipParam(${chipIdx},${paramIdx},this.value,'${which}')" onclick="event.stopPropagation()">`;
}

function _chipLabeledInputHtml(which, chipIdx, paramIdx, value, paramDef, options = {}) {
    const pd = paramDef || {};
    const label = String(pd.label || pd.ph || '').trim();
    const input = _chipInputHtml(which, chipIdx, paramIdx, value, pd, options);
    if (!label) return input;
    return `<span class="chip-param-pair"><span class="chip-param-name">${_escapeHtml(label)}=</span>${input}</span>`;
}

function _solveScoreChipShell(which, i, bodyHtml, tooltipAttr = '', options = {}) {
    if (options.readonly) {
        return `<span class="chip score-chip score-chip-readonly"${tooltipAttr}>${bodyHtml}</span>`;
    }
    const count = _chainForWhich(which).length;
    const leftDisabled = i <= 0 ? ' disabled' : '';
    const rightDisabled = i >= count - 1 ? ' disabled' : '';
    const moves = `<span class="chip-actions"><button type="button" class="chip-move" onclick="event.stopPropagation();moveChip('${which}',${i},-1)" title="Move left"${leftDisabled}>&lt;</button><button type="button" class="chip-move" onclick="event.stopPropagation();moveChip('${which}',${i},1)" title="Move right"${rightDisabled}>&gt;</button></span>`;
    const selected = _solveScoreSelectedIndex[which] === i ? ' selected' : '';
    return `<span class="chip score-chip${selected}" onclick="selectSolveScoreLine('${which}',${i},event)"${tooltipAttr}>${moves}${bodyHtml}<span class="chip-x" onclick="event.stopPropagation();removeChip('${which}',${i})">x</span></span>`;
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
