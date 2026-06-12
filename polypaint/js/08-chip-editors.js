// PolyPaint 08-chip-editors — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Cache-busting: deploy appends ?v=<BUILD_ID> to the tags.
function _paramValue(item, pDefs, idx) {
    const params = Array.isArray(item && item.params) ? item.params : [];
    if (params[idx] != null && params[idx] !== '') return params[idx];
    const def = pDefs && pDefs[idx] ? pDefs[idx].def : '';
    return def == null ? '' : String(def);
}

function _chipMoveControlsHtml(which, idx) {
    if (!['pt', 'ct', 'rt', 'palette-rt'].includes(which)) return '';
    const count = _chainForWhich(which).length;
    const leftDisabled = idx <= 0 ? ' disabled' : '';
    const rightDisabled = idx >= count - 1 ? ' disabled' : '';
    return `<span class="chip-actions"><button type="button" class="chip-move" onclick="event.stopPropagation();moveChip('${which}',${idx},-1)" title="Move left"${leftDisabled}>&lt;</button><button type="button" class="chip-move" onclick="event.stopPropagation();moveChip('${which}',${idx},1)" title="Move right"${rightDisabled}>&gt;</button></span>`;
}

function _displayParamTransforms() {
    return _ptChain.map(item => {
        if (!item.params || !item.params.length) return [item.name];
        return [item.name, ...item.params];
    });
}

function _ptCategoryGroups() {
    const grouped = {};
    Object.keys(_ptCategoryMeta).forEach(key => { grouped[key] = []; });
    Object.keys(_ptCatalog).forEach(name => {
        const spec = _ptCatalog[name] || {};
        const key = grouped[spec.category] ? spec.category : 'maps';
        grouped[key].push(name);
    });
    return Object.keys(_ptCategoryMeta).map(key => ({
        key,
        ..._ptCategoryMeta[key],
        items: grouped[key] || [],
    }));
}

function _renderParamTransformAddPopup() {
    const popup = document.getElementById('pt-add-popup');
    if (!popup) return;
    const head = `<div class="score-chip-picker-head"><span class="score-chip-picker-title">Add param transform</span><span class="score-chip-picker-state">Transforms t1/t2 before coefficient generation</span></div>`;
    const body = _ptCategoryGroups().map(group => {
        const items = group.items.length
            ? group.items.map(name => {
                const spec = _ptCatalog[name] || {};
                const label = spec.label || name;
                const paramCount = (spec.params || []).length;
                const params = paramCount ? ` · ${_pluralize(paramCount, 'param')}` : '';
                return `<button type="button" class="score-chip-option score-chip-option-${_escapeHtml(group.key)}" onclick="selectParamTransformChip('${_escapeHtml(name)}',event)" title="${_escapeHtml(spec.desc || label)}"><span class="score-chip-option-name">${_escapeHtml(label)}</span><span class="score-chip-option-meta">${_escapeHtml((spec.desc || '') + params)}</span></button>`;
            }).join('')
            : '<span class="score-chip-empty">No transforms</span>';
        return `<div class="score-chip-category"><div class="score-chip-category-title">${_escapeHtml(group.title)}</div><div class="score-chip-category-help">${_escapeHtml(group.help)}</div><div class="score-chip-options">${items}</div></div>`;
    }).join('');
    popup.innerHTML = head + body;
}

function _setParamTransformPickerOpen(open) {
    const popup = document.getElementById('pt-add-popup');
    const btn = document.getElementById('pt-add-btn');
    if (!popup) return;
    if (open) _renderParamTransformAddPopup();
    popup._open = !!open;
    popup.style.display = open ? 'block' : 'none';
    if (popup.classList && popup.classList.toggle) popup.classList.toggle('active', !!open);
    if (popup.setAttribute) popup.setAttribute('aria-hidden', open ? 'false' : 'true');
    if (btn && btn.setAttribute) btn.setAttribute('aria-expanded', open ? 'true' : 'false');
}

function toggleParamTransformPicker(eventObj) {
    if (eventObj && eventObj.stopPropagation) eventObj.stopPropagation();
    const popup = document.getElementById('pt-add-popup');
    _setParamTransformPickerOpen(!(popup && popup._open));
}

function selectParamTransformChip(name, eventObj) {
    if (eventObj && eventObj.stopPropagation) eventObj.stopPropagation();
    addChip('pt', name);
    _setParamTransformPickerOpen(false);
}

function _syncParamTransformAddOptions() {
    const sel = document.getElementById('pt-add');
    if (sel) {
        const options = ['<option value="">+ add...</option>'].concat(
            Object.keys(_ptCatalog).map(name => {
                const spec = _ptCatalog[name] || {};
                return `<option value="${_escapeHtml(name)}">${_escapeHtml(spec.label || name)}</option>`;
            })
        );
        sel.innerHTML = options.join('');
    }
    _renderParamTransformAddPopup();
}

function _paramProgramCategoryGroups() {
    const grouped = {};
    Object.keys(_ppCategoryMeta).forEach(key => { grouped[key] = []; });
    Object.keys(_ppCatalog).forEach(name => {
        const spec = _ppCatalog[name] || {};
        const key = grouped[spec.category] ? spec.category : 'stack';
        grouped[key].push(name);
    });
    return Object.keys(_ppCategoryMeta).map(key => ({
        key,
        ..._ppCategoryMeta[key],
        items: grouped[key] || [],
    }));
}

// One picker implementation drives both program chip pickers; the
// descriptors carry everything that differs. The public function names
// below stay as thin wrappers because generated onclick strings and HTML
// reference them directly.
const _chipPickers = {
    pp: {
        which: 'pp',
        popupId: 'pp-add-popup',
        headTitle: 'Add param program chip',
        headHint: 'RPN over t1/t2, output registers p1/p2',
        catalog: () => _ppCatalog,
        groups: () => _paramProgramCategoryGroups(),
        selectFnName: 'selectParamProgramChip',
        getInsertMode: () => _paramProgramPickerInsertMode,
        setInsertMode: (mode) => { _paramProgramPickerInsertMode = mode; },
    },
    cp: {
        which: 'cp',
        popupId: 'cp-add-popup',
        headTitle: 'Add coeff program chip',
        headHint: 'RPN over coefficient vectors cf → poly',
        catalog: () => _coeffProgramCatalog,
        groups: () => _coeffProgramCategoryGroups(),
        selectFnName: 'selectCoeffProgramChip',
        getInsertMode: () => _coeffProgramPickerInsertMode,
        setInsertMode: (mode) => { _coeffProgramPickerInsertMode = mode; },
    },
};

function _renderChipPickerPopup(picker) {
    const popup = document.getElementById(picker.popupId);
    if (!popup) return;
    const catalog = picker.catalog();
    const head = `<div class="score-chip-picker-head"><span class="score-chip-picker-title">${_escapeHtml(picker.headTitle)}</span><span class="score-chip-picker-state">${_escapeHtml(picker.headHint)}</span></div>`;
    const body = picker.groups().map(group => {
        const items = group.items.length
            ? group.items.map(name => {
                const spec = catalog[name] || {};
                const label = spec.label || name;
                const paramCount = (spec.params || []).length;
                const params = paramCount ? ` · ${_pluralize(paramCount, 'param')}` : '';
                return `<button type="button" class="score-chip-option score-chip-option-${_escapeHtml(group.key)}" onclick="${picker.selectFnName}('${_escapeHtml(name)}',event)" title="${_escapeHtml(spec.desc || label)}"><span class="score-chip-option-name">${_escapeHtml(label)}</span><span class="score-chip-option-meta">${_escapeHtml((spec.desc || '') + params)}</span></button>`;
            }).join('')
            : '<span class="score-chip-empty">No chips</span>';
        return `<div class="score-chip-category"><div class="score-chip-category-title">${_escapeHtml(group.title)}</div><div class="score-chip-category-help">${_escapeHtml(group.help)}</div><div class="score-chip-options">${items}</div></div>`;
    }).join('');
    popup.innerHTML = head + body;
}

function _setChipPickerOpen(picker, open) {
    const popup = document.getElementById(picker.popupId);
    if (!popup) return;
    if (open) _renderChipPickerPopup(picker);
    popup._open = !!open;
    popup.style.display = open ? 'block' : 'none';
    if (popup.classList && popup.classList.toggle) popup.classList.toggle('active', !!open);
    if (popup.setAttribute) popup.setAttribute('aria-hidden', open ? 'false' : 'true');
}

function _toggleChipPicker(picker, eventObj, insertMode) {
    if (eventObj && eventObj.stopPropagation) eventObj.stopPropagation();
    if (insertMode === 'before' || insertMode === 'after') picker.setInsertMode(insertMode);
    const popup = document.getElementById(picker.popupId);
    _setChipPickerOpen(picker, !(popup && popup._open));
}

function _selectChipFromPicker(picker, name, eventObj) {
    if (eventObj && eventObj.stopPropagation) eventObj.stopPropagation();
    addChip(picker.which, name, picker.getInsertMode() || 'after');
    _setChipPickerOpen(picker, false);
}

function _renderParamProgramAddPopup() {
    _renderChipPickerPopup(_chipPickers.pp);
}

function _setParamProgramPickerOpen(open) {
    _setChipPickerOpen(_chipPickers.pp, open);
}

function toggleParamProgramPicker(eventObj, insertMode = null) {
    _toggleChipPicker(_chipPickers.pp, eventObj, insertMode);
}

function selectParamProgramChip(name, eventObj) {
    _selectChipFromPicker(_chipPickers.pp, name, eventObj);
}

function selectParamProgramLine(idx, eventObj) {
    if (eventObj && eventObj.stopPropagation) eventObj.stopPropagation();
    _paramProgramSelectedIndex = Number(idx);
    _renderChips('pp');
}

function _syncParamProgramAddOptions() {
    const sel = document.getElementById('pp-add');
    if (sel) {
        const options = ['<option value="">+ add...</option>'].concat(
            Object.keys(_ppCatalog).map(name => {
                const spec = _ppCatalog[name] || {};
                return `<option value="${_escapeHtml(name)}">${_escapeHtml(spec.label || name)}</option>`;
            })
        );
        sel.innerHTML = options.join('');
    }
    _renderParamProgramAddPopup();
}

function _coeffProgramCategoryGroups() {
    const grouped = {};
    Object.keys(_coeffProgramCategoryMeta).forEach(key => { grouped[key] = []; });
    Object.keys(_coeffProgramCatalog).forEach(name => {
        const spec = _coeffProgramCatalog[name] || {};
        if (spec.hidden) return;
        const key = grouped[spec.category] ? spec.category : 'elementwise';
        grouped[key].push(name);
    });
    return Object.keys(_coeffProgramCategoryMeta).map(key => ({
        key,
        ..._coeffProgramCategoryMeta[key],
        items: grouped[key] || [],
    }));
}

function _renderCoeffProgramAddPopup() {
    _renderChipPickerPopup(_chipPickers.cp);
}

function _setCoeffProgramPickerOpen(open) {
    _setChipPickerOpen(_chipPickers.cp, open);
}

function toggleCoeffProgramPicker(eventObj, insertMode = null) {
    _toggleChipPicker(_chipPickers.cp, eventObj, insertMode);
}

function selectCoeffProgramChip(name, eventObj) {
    _selectChipFromPicker(_chipPickers.cp, name, eventObj);
}

function selectCoeffProgramLine(idx, eventObj) {
    if (eventObj && eventObj.stopPropagation) eventObj.stopPropagation();
    _coeffProgramSelectedIndex = Number(idx);
    _renderChips('cp');
}

// pp has a hidden fallback <select>; cp does not, so this only re-renders
// the popup. Kept for symmetry with _syncParamProgramAddOptions.
function _syncCoeffProgramAddOptions() {
    _renderCoeffProgramAddPopup();
}

function _serializeParamTransforms() {
    return _ptChain.map(item => {
        if (!item.params || !item.params.length) return [item.name];
        const spec = _ptCatalog[item.name] || {};
        const pDefs = spec.params || [];
        const wireParams = item.params.map((v, i) => (pDefs[i] && pDefs[i].target) ? _targetToWire(v) : v);
        return [item.name, ...wireParams];
    });
}

let _paramProgramSelectedIndex = -1;
let _paramProgramPickerInsertMode = 'after';
let _coeffProgramSelectedIndex = -1;
let _coeffProgramPickerInsertMode = 'after';
let _coeffProgramEditorMode = 'chips';

function _coeffProgramTextModeSelected() {
    return _coeffProgramEditorMode === 'text';
}

function _coeffProgramSourceTextarea() {
    return document.getElementById('cp-source-text');
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
                const [legacyName, src, tgt, ...rest] = params;
                const callName = _coeffProgramSourceAliasNames[legacyName] || legacyName || 'rev';
                const args = [src || 'poly', ...rest].join(', ');
                return tgt === 'poly' ? `poly = ${callName}(${args})` : `${callName}(${args})`;
            }
            return params.length ? `${name}(${params.join(', ')})` : name;
        }
        return String(row || '');
    }).filter(Boolean).join('\n');
}

function _setCoeffProgramSourceText(text) {
    const value = String(text == null ? '' : text);
    const el = _coeffProgramSourceTextarea();
    if (el && el.value !== value) el.value = value;
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

function _setCoeffProgramEditorMode(mode) {
    const normalized = mode === 'text' ? 'text' : 'chips';
    _coeffProgramEditorMode = normalized;
    const chipsPanel = document.getElementById('cp-chips-panel');
    const textPanel = document.getElementById('cp-text-panel');
    const chipsTab = document.getElementById('coeff-program-tab-chips');
    const textTab = document.getElementById('coeff-program-tab-text');
    if (chipsPanel && chipsPanel.classList) chipsPanel.classList.toggle('active', normalized === 'chips');
    if (textPanel && textPanel.classList) textPanel.classList.toggle('active', normalized === 'text');
    if (chipsTab && chipsTab.classList) chipsTab.classList.toggle('active', normalized === 'chips');
    if (textTab && textTab.classList) textTab.classList.toggle('active', normalized === 'text');
    if (normalized === 'text' && _coeffProgramSourceAutoSynthed && !_coeffProgramChain.length) {
        // The synthesized program's chips were all removed; stale auto-text
        // must not stay authoritative for compute.
        _setCoeffProgramSourceText('');
    } else if (normalized === 'text' && _coeffProgramChain.length &&
        (!_getCoeffProgramSourceText().trim() || _coeffProgramSourceAutoSynthed)) {
        _setCoeffProgramSourceText(_coeffProgramSourceFromRows(_serializeCoeffProgramChain()));
        _coeffProgramSourceAutoSynthed = true;
    }
    const computeScope = _paramProgramModeSelected()
        ? ', and compute'
        : '; compute ignores the program while the Chain pipeline is selected';
    _coeffProgramStatus(normalized === 'text'
        ? `Text source is authoritative for save, preview, debug${computeScope}.`
        : `Chip editor is authoritative for save, preview, debug${computeScope}.`);
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
    const defs = _paramProgramLegacyArgSpecs[name];
    return Array.isArray(defs) ? defs : null;
}

function _paramProgramLegacyTakesNoArgs(legacyName) {
    const name = String(legacyName || '').trim();
    return _paramProgramLegacyNames.includes(name) && !_paramProgramLegacyArgDefs(name);
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
            rawArgs = _splitParamProgramLegacyArgs(values.join(','));
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
    const name = String(legacyName || 'unit_circle').trim() || 'unit_circle';
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

function _hasParamProgramChain() {
    return _serializeParamProgramChain().length > 0;
}

function _selectedParamPipelineMode() {
    const el = document.getElementById('param-pipeline-mode');
    const raw = el && el.value ? el.value : _paramPipelineMode;
    if (raw === 'program' || raw === 'param_program') return 'program';
    return 'chain';
}

function _paramProgramModeSelected() {
    return _selectedParamPipelineMode() === 'program';
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

function _displayActiveParamPipeline(separator = ',') {
    if (_paramProgramModeSelected()) return _formatParamProgramChainForLog(_serializeParamProgramChain(), separator);
    return _formatChainRowsForLog(_displayParamTransforms(), separator) || 'none';
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
    if (_paramProgramModeSelected() && textMode) {
        return _coeffProgramSourceDisplay(_getCoeffProgramSourceText(), separator);
    }
    if (_paramProgramModeSelected()) return _formatCoeffProgramChainForLog(_serializeCoeffProgramChain(), separator);
    return _formatChainRowsForLog(_serializeCoeffTransforms(), separator) || 'none';
}

function _paramPipelineEditAffectsCompute(which) {
    if (which === 'ct') return !_paramProgramModeSelected();
    if (which === 'cp') return _paramProgramModeSelected();
    if (which === 'pt') return !_paramProgramModeSelected();
    if (which === 'pp') return _paramProgramModeSelected();
    return false;
}

function _syncParamPipelineModeUi() {
    const mode = _selectedParamPipelineMode();
    _paramPipelineMode = mode;
    const selectEl = document.getElementById('param-pipeline-mode');
    const noteEl = document.getElementById('param-pipeline-mode-note');
    const legacyRow = document.getElementById('param-transforms-row');
    const programBox = document.querySelector('.param-program-box');
    const coeffProgramBox = document.querySelector('.coeff-program-box');
    if (selectEl && selectEl.value !== mode) selectEl.value = mode;
    if (legacyRow && legacyRow.classList) legacyRow.classList.toggle('param-pipeline-inactive', mode !== 'chain');
    const coeffRow = document.getElementById('coeff-transforms-row');
    if (coeffRow && coeffRow.classList) coeffRow.classList.toggle('param-pipeline-inactive', mode !== 'chain');
    if (programBox && programBox.classList) programBox.classList.toggle('param-pipeline-inactive', mode !== 'program');
    if (coeffProgramBox && coeffProgramBox.classList) coeffProgramBox.classList.toggle('param-pipeline-inactive', mode !== 'program');
    const ppLen = _serializeParamProgramChain().length;
    const cpLen = _coeffProgramTextModeSelected()
        ? _coeffProgramSourceStatementCount(_getCoeffProgramSourceText())
        : _serializeCoeffProgramChain().length;
    const ptLen = _serializeParamTransforms().length;
    const ctLen = _serializeCoeffTransforms().length;
    if (noteEl) {
        noteEl.textContent = mode === 'program'
            ? `Program mode: Param Program ${ppLen ? `${_pluralize(ppLen, 'chip')}` : 'identity'}; Coeff Program ${cpLen ? `${cpLen} ${_coeffProgramTextModeSelected() ? 'source statement' : 'chip'}${cpLen === 1 ? '' : 's'}` : 'identity'}. Legacy chains are idle.`
            : `Chain mode: Param transforms ${ptLen ? `${_pluralize(ptLen, 'chip')}` : 'none'}; Coeff transforms ${ctLen ? `${_pluralize(ctLen, 'chip')}` : 'none'}. Programs are idle.`;
    }
}

function _setParamPipelineMode(mode, options = {}) {
    _paramPipelineMode = (mode === 'program' || mode === 'param_program') ? 'program' : 'chain';
    _syncParamPipelineModeUi();
    _renderChips('pp');
    _renderChips('cp');
    if (options.markStale) _markComputePreviewStale();
}

// Which pipeline is authoritative for a compute payload:
// - chain mode sends legacy transforms (pt/ct) and empty programs;
// - program mode sends programs (pp + cp-or-source) and empty transforms.
// Exactly one side is ever non-empty, so the backend needs no mode flag.
function _effectiveParamTransformsForCompute() {
    return _paramProgramModeSelected() ? [] : _serializeParamTransforms();
}

function _effectiveParamProgramChainForCompute() {
    return _paramProgramModeSelected() ? _serializeParamProgramChain() : [];
}

function _effectiveCoeffTransformsForCompute() {
    return _paramProgramModeSelected() ? [] : _serializeCoeffTransforms();
}

function _effectiveCoeffProgramChainForCompute() {
    return _paramProgramModeSelected() && !_coeffProgramTextModeSelected() ? _serializeCoeffProgramChain() : [];
}

function _effectiveCoeffProgramSourceTextForCompute() {
    return _paramProgramModeSelected() && _coeffProgramTextModeSelected() ? _getCoeffProgramSourceText() : null;
}

function _attachCoeffProgramSourcePayload(payload) {
    const sourceText = _effectiveCoeffProgramSourceTextForCompute();
    if (sourceText !== null) payload.coeff_program_source_text = sourceText;
    return payload;
}

function _displayParamProgramEntry(item) {
    if (!item || !item.name) return '';
    const params = Array.isArray(item.params) ? item.params.filter(v => String(v || '').trim() !== '') : [];
    return params.length ? `${item.name}(${params.join(',')})` : item.name;
}

function _paramProgramStatus(message, isError = false) {
    const el = document.getElementById('param-program-status');
    if (!el) return;
    el.textContent = String(message || '');
    el.className = `solve-score-program-status${isError ? ' error' : ''}`;
}

function _paramProgramDefaultName() {
    try {
        const chain = _serializeParamProgramChain();
        const basis = chain.length ? chain.map(row => Array.isArray(row) ? row[0] : String(row)).join('-') : 'param-program';
        return basis.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '').slice(0, 48) || 'param-program';
    } catch (_) {
        return 'param-program';
    }
}

function _normalizeParamProgramChain(rawChain) {
    if (!Array.isArray(rawChain)) return [];
    return rawChain.flatMap(item => {
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
    }).filter(Boolean);
}

function _parseParamProgramPayload(raw) {
    if (!raw || typeof raw !== 'object') throw new Error('program JSON must be an object');
    const chain = Array.isArray(raw.chain) ? raw.chain : null;
    if (!chain || !chain.length) throw new Error('program JSON is missing a non-empty chain');
    const hasVersion = Object.prototype.hasOwnProperty.call(raw, 'version');
    const parsedVersion = Number(raw.version);
    if (hasVersion && !Number.isFinite(parsedVersion)) throw new Error('program JSON version must be numeric when present');
    const version = hasVersion ? parsedVersion : 1;
    if (version !== 1) throw new Error(`program JSON version ${version} is not supported`);
    const kind = String(raw.program_kind || 'param_program');
    if (kind !== 'param_program') throw new Error(`program JSON kind ${kind} is not param_program`);
    return {
        version,
        program_kind: 'param_program',
        name: String(raw.name || '').trim(),
        description: String(raw.description || '').trim(),
        chain: _normalizeParamProgramChain(chain),
    };
}

function _applyParamProgram(rawProgram) {
    const program = _parseParamProgramPayload(rawProgram);
    _ppChain.splice(0, _ppChain.length, ...program.chain);
    _paramProgramSelectedIndex = _ppChain.length ? Math.min(_ppChain.length - 1, Math.max(0, _paramProgramSelectedIndex)) : -1;
    _renderChips('pp');
    if (_paramProgramModeSelected()) _markComputePreviewStale();
    _paramProgramStatus(program.name ? `Loaded ${program.name}` : 'Loaded param program');
    return program;
}

function _copyParamTransformsIntoParamProgram() {
    const transforms = _serializeParamTransforms();
    if (!transforms.length) {
        _paramProgramStatus('No legacy Param transforms to copy', true);
        return;
    }
    const chain = transforms.map(item => {
        const row = Array.isArray(item) ? item : [String(item)];
        return { name: 'legacy', params: _paramProgramBridgeParamsFromLegacyTransform(row) };
    }).filter(item => item && item.params);
    _ppChain.splice(0, _ppChain.length, ...chain);
    _paramProgramSelectedIndex = _ppChain.length ? 0 : -1;
    _renderChips('pp');
    if (_paramProgramModeSelected()) _markComputePreviewStale();
    _paramProgramStatus(`Copied ${chain.length} legacy transform${chain.length === 1 ? '' : 's'} into param program`);
}

function _clearParamProgramChain() {
    _ppChain.splice(0, _ppChain.length);
    _paramProgramSelectedIndex = -1;
    _renderChips('pp');
    if (_paramProgramModeSelected()) _markComputePreviewStale();
}

function _portableParamProgramPayload(nameOverride = '') {
    const chain = _serializeParamProgramChain();
    if (!chain.length) throw new Error('Param program is empty');
    return {
        version: 1,
        program_kind: 'param_program',
        name: String(nameOverride || _paramProgramDefaultName()).trim() || 'param-program',
        chain,
        display: _chainDisplayString(chain),
    };
}

function _normalizeCoeffProgramChain(rawChain) {
    if (!Array.isArray(rawChain)) return [];
    return rawChain.flatMap(item => {
        if (Array.isArray(item) && item.length) {
            const name = String(item[0] || '').trim();
            if (!name) return [];
            if (name === 'legacy') {
                const legacyName = _canonicalCoeffTransformName(item[1] || 'rev') || 'rev';
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
    }).filter(Boolean);
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
        const name = String(item.name || '').trim();
        const params = Array.isArray(item.params) ? item.params.map(v => _str(v)) : [];
        if (name === 'legacy') {
            const legacyName = _canonicalCoeffTransformName(params[0] || 'rev') || 'rev';
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
    if (version !== 1) throw new Error(`program JSON version ${version} is not supported`);
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
        // Clear any previous program's text so it cannot become authoritative
        // for this chain-only program when the user opens the Text tab.
        _setCoeffProgramSourceText('');
        _coeffProgramSourceAutoSynthed = false;
        _setCoeffProgramEditorMode('chips');
    }
    if (_paramProgramModeSelected()) _markComputePreviewStale();
    _coeffProgramStatus(program.name ? `Loaded ${program.name}` : 'Loaded coeff program');
    return program;
}

function _copyCoeffTransformsIntoCoeffProgram() {
    const transforms = _serializeCoeffTransforms();
    if (!transforms.length) {
        _coeffProgramStatus('No legacy Coeff transforms to copy', true);
        return;
    }
    const chain = transforms.map(row => {
        const values = Array.isArray(row) ? row : [String(row)];
        const normalized = _normalizeCoeffTransformItem({
            name: values[0],
            params: values.slice(1).map(v => String(v)),
        });
        const args = (normalized.params || []).map(v => String(v));
        // Named chips have andy-less params (the catalogs filter it), so a
        // transform carrying a non-default andy must stay a legacy chip or
        // the andy is silently dropped on the next serialize — same rule as
        // _normalizeCoeffProgramChain.
        const fullDefs = _coeffProgramLegacyInputDefs(normalized.name);
        const lastDef = fullDefs.length ? fullDefs[fullDefs.length - 1] : null;
        const carriesAndy = _isAndyParam(lastDef)
            && args.length === fullDefs.length
            && !_ctAndyIsDefault(args[args.length - 1]);
        if (carriesAndy) {
            // legacy rows are source-first: [name, src, tgt, ...args]
            return { name: 'legacy', params: [normalized.name, 'poly', 'poly', ...args] };
        }
        // named chip params are target-first: [tgt, src, ...args]
        return { name: _coeffProgramRegistryChipName(normalized.name), params: ['poly', 'poly', ...args] };
    });
    _coeffProgramChain.splice(0, _coeffProgramChain.length, ...chain);
    _coeffProgramSelectedIndex = _coeffProgramChain.length ? 0 : -1;
    // The copied transforms are now the program; stale text (user-authored or
    // synthesized from the previous chain) must not stay authoritative.
    _setCoeffProgramSourceText('');
    _coeffProgramSourceAutoSynthed = false;
    _setCoeffProgramEditorMode('chips');
    _renderChips('cp');
    if (_paramProgramModeSelected()) _markComputePreviewStale();
    _coeffProgramStatus(`Copied ${chain.length} coefficient transform${chain.length === 1 ? '' : 's'} into coeff program`);
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
    if (which === 'ct' && pDef) {
        if (pDef.complexPairNext) {
            const complex = _splitCtComplexInput(value);
            if (complex) return { value: complex.re, valueNext: complex.im };
            // not a pair: fall through and treat the edit as this slot only
        }
        if (pDef.complex) {
            const parsed = _parseCtComplexConstant(value);
            return parsed
                ? { value: _formatCfpvComplexValue(parsed.re, parsed.im) }
                : { error: `Invalid ${pDef.ph || 'value'}: "${value}". Use a finite complex number such as 2-3j.` };
        }
        const norm = _normalizeCtRealInput(value);
        return norm == null
            ? { error: `Invalid ${pDef.ph || 'value'}: "${value}". Use a number, scientific notation, or a simple +/- constant expression.` }
            : { value: norm };
    }
    return { value };
}

function updateChipParam(chipIdx, paramIdx, value, which) {
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

function _ctUnaryFormulaChip(which, i, item, fnName, pDefs) {
    return `<span class="chip"><span class="chip-formula"><span>${_escapeHtml(fnName)}(z)</span><span class="chip-op">·</span>${_ctAndyHtml(which, i, item, pDefs)}</span><span class="chip-x" onclick="removeChip('${which}',${i})">x</span></span>`;
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
