// PolyPaint 09-render-orchestration — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Deploy rewrites the script tags to build-versioned asset
// keys (assets/<BUILD_ID>/...), so a deploy flips atomically via index.html.
const RENDER_MAX_PIX = 32768;

function _clampRenderPix(value) {
    const pix = Number(value);
    if (!Number.isFinite(pix)) return 0;
    return Math.max(1, Math.min(RENDER_MAX_PIX, Math.round(pix)));
}

function _renderRtChipHtml(item, i, which, catalog, options = {}) {
    const spec = catalog[item.name] || {};
    const pDefs = spec.params || [];
    const isSolveScore = which === 'ss' || which === 'palette-ss' || !!options.solveScore;
    if (isSolveScore && _isSolveScoreGenericMetricChipName(item.name)) {
        const metric = _chipInputHtml(which, i, 0, item.params[0] || (_solveScoreGenericMetricNames[0] || 'proximity'), pDefs[0], options);
        const source = _chipInputHtml(which, i, 1, item.params[1] || 'slv', pDefs[1], options);
        const inheritedQ = _solveScoreInheritedLagQuantile(which, i, item, options);
        const q = inheritedQ == null
            ? _chipInputHtml(which, i, 2, item.params[2] || '', pDefs[2], options)
            : (options.readonly
                ? _chipReadonlyValueHtml(inheritedQ, pDefs[2], 'Inherited from the base metric slot')
                : `<input class="chip-input" type="text" value="${_escapeHtml(inheritedQ)}" disabled title="Inherited from the base metric slot">`);
        return _solveScoreChipShell(which, i, `<span class="chip-formula"><span>metric(</span><span>${metric}</span><span class="chip-op">,</span><span>${source}</span><span class="chip-op">,</span><span>q=</span><span>${q}</span><span>%)</span></span>`, '', options);
    }
    if (isSolveScore && _solveScoreMetricSet.has(item.name)) {
        const source = _chipInputHtml(which, i, 0, item.params[0] || 'slv', pDefs[0], options);
        const inheritedQ = _solveScoreInheritedLagQuantile(which, i, item, options);
        const q = inheritedQ == null
            ? _chipInputHtml(which, i, 1, item.params[1] || item.params[0] || '', pDefs[1], options)
            : (options.readonly
                ? _chipReadonlyValueHtml(inheritedQ, pDefs[1], 'Inherited from the base metric slot')
                : `<input class="chip-input" type="text" value="${_escapeHtml(inheritedQ)}" disabled title="Inherited from the base metric slot">`);
        return _solveScoreChipShell(which, i, `<span class="chip-formula"><span>${_escapeHtml(item.name)}(</span><span>${source}</span><span class="chip-op">,</span><span>q=</span><span>${q}</span><span>%)</span></span>`, '', options);
    }
    if (isSolveScore && item.name === 'omega_cosine') {
        const omega = _chipInputHtml(which, i, 0, item.params[0] || '', pDefs[0], options);
        const phase = _chipInputHtml(which, i, 1, item.params[1] || '0', pDefs[1], options);
        const tooltip = spec.tooltip ? ` title="${_escapeHtml(spec.tooltip)}"` : '';
        return _solveScoreChipShell(which, i, `<span class="chip-formula"><span>ω-cos(</span><span>${omega}</span><span class="chip-op">,</span><span>${phase}</span><span>)</span></span>`, tooltip, options);
    }
    if (isSolveScore && item.name === 'sawtooth') {
        const mult = _chipInputHtml(which, i, 0, item.params[0] || '', pDefs[0], options);
        const tooltip = spec.tooltip ? ` title="${_escapeHtml(spec.tooltip)}"` : '';
        return _solveScoreChipShell(which, i, `<span class="chip-formula"><span>sawtooth(</span><span>${mult}</span><span>)</span></span>`, tooltip, options);
    }
    if (isSolveScore && item.name === 'weighted_sum') {
        const a = _chipInputHtml(which, i, 0, item.params[0] || '', pDefs[0], options);
        const b = _chipInputHtml(which, i, 1, item.params[1] || '', pDefs[1], options);
        return _solveScoreChipShell(which, i, `<span class="chip-formula"><span>weighted_sum(</span><span>${a}</span><span class="chip-op">,</span><span>${b}</span><span>)</span></span>`, '', options);
    }
    if (isSolveScore && item.name === 'flip') {
        const tooltip = spec.tooltip ? ` title="${_escapeHtml(spec.tooltip)}"` : '';
        return _solveScoreChipShell(which, i, `<span class="chip-formula"><span>flip</span></span>`, tooltip, options);
    }
    if (isSolveScore && _solveScoreUnarySpecs[item.name]) {
        const inputs = (item.params || []).map((val, pi) => _chipInputHtml(which, i, pi, val, pDefs[pi] || {}, options)).join('');
        const tooltip = spec.tooltip ? ` title="${_escapeHtml(spec.tooltip)}"` : '';
        return _solveScoreChipShell(which, i, `<span class="chip-formula"><span>${_escapeHtml(item.name)}</span>${inputs}</span>`, tooltip, options);
    }
    if (isSolveScore && _solveScoreOutputSpecs[item.name]) {
        const tooltip = spec.tooltip ? ` title="${_escapeHtml(spec.tooltip)}"` : '';
        const mode = _chipInputHtml(which, i, 0, (item.name === 'emit_norm' ? 'norm' : ((item.params || [])[0] || 'raw')), pDefs[0], options);
        return _solveScoreChipShell(which, i, `<span class="chip-formula"><span>emit(</span><span>${mode}</span><span>)</span></span>`, tooltip, options);
    }
    if (isSolveScore && _solveScoreCombineSpecs[item.name]) {
        const tooltip = spec.tooltip ? ` title="${_escapeHtml(spec.tooltip)}"` : '';
        return _solveScoreChipShell(which, i, `<span class="chip-formula"><span>${_escapeHtml(item.name)}</span></span>`, tooltip, options);
    }
    if (item.name === 'moebius') {
        const a = _chipInputHtml(which, i, 0, item.params[0] || '', pDefs[0]);
        const b = _chipInputHtml(which, i, 1, item.params[1] || '', pDefs[1]);
        const c = _chipInputHtml(which, i, 2, item.params[2] || '', pDefs[2]);
        const d = _chipInputHtml(which, i, 3, item.params[3] || '', pDefs[3]);
        return `<span class="chip"><span class="chip-equals">f(z)=</span><span class="chip-formula">(<span>${a}</span><span class="chip-op">z+</span><span>${b}</span>)/(<span>${c}</span><span class="chip-op">z+</span><span>${d}</span>)</span><span class="chip-x" onclick="removeChip('${which}',${i})">x</span></span>`;
    }
    if (item.name === 'add_complex') {
        const a = _chipInputHtml(which, i, 0, item.params[0] || '', pDefs[0]);
        const b = _chipInputHtml(which, i, 1, item.params[1] || '', pDefs[1]);
        return `<span class="chip"><span class="chip-formula"><span>z+(</span><span>${a}</span><span class="chip-op">+i</span><span>${b}</span><span>)</span></span><span class="chip-x" onclick="removeChip('${which}',${i})">x</span></span>`;
    }
    if (item.name === 'mul_complex') {
        const a = _chipInputHtml(which, i, 0, item.params[0] || '', pDefs[0]);
        const b = _chipInputHtml(which, i, 1, item.params[1] || '', pDefs[1]);
        return `<span class="chip"><span class="chip-formula"><span>z*(</span><span>${a}</span><span class="chip-op">+i</span><span>${b}</span><span>)</span></span><span class="chip-x" onclick="removeChip('${which}',${i})">x</span></span>`;
    }
    const label = _escapeHtml(spec.label || item.name);
    const inputDefs = pDefs.length ? pDefs : (item.params || []).map(() => ({}));
    const inputsHtml = inputDefs.map((pDef, pi) => _chipInputHtml(which, i, pi, _paramValue(item, pDefs, pi), pDef || {})).join('');
    return `<span class="chip">${label}${inputsHtml}<span class="chip-x" onclick="removeChip('${which}',${i})">x</span></span>`;
}

function _paramProgramChipShellHtml(i, readonly, bodyHtml) {
    if (readonly) {
        return `<span class="chip score-chip score-chip-readonly">${bodyHtml}</span>`;
    }
    const count = _chainForWhich('pp').length;
    const selected = _paramProgramSelectedIndex === i ? ' selected' : '';
    const leftDisabled = i <= 0 ? ' disabled' : '';
    const rightDisabled = i >= count - 1 ? ' disabled' : '';
    const moves = `<span class="chip-actions"><button type="button" class="chip-move" onclick="event.stopPropagation();moveChip('pp',${i},-1)" title="Move left"${leftDisabled}>&lt;</button><button type="button" class="chip-move" onclick="event.stopPropagation();moveChip('pp',${i},1)" title="Move right"${rightDisabled}>&gt;</button></span>`;
    return `<span class="chip score-chip${selected}" onclick="selectParamProgramLine(${i},event)">${moves}${bodyHtml}<span class="chip-x" onclick="event.stopPropagation();removeChip('pp',${i})">x</span></span>`;
}

function _coeffProgramChipShellHtml(i, readonly, bodyHtml) {
    if (readonly) {
        return `<span class="chip score-chip score-chip-readonly">${bodyHtml}</span>`;
    }
    const count = _chainForWhich('cp').length;
    const selected = _coeffProgramSelectedIndex === i ? ' selected' : '';
    const leftDisabled = i <= 0 ? ' disabled' : '';
    const rightDisabled = i >= count - 1 ? ' disabled' : '';
    const moves = `<span class="chip-actions"><button type="button" class="chip-move" onclick="event.stopPropagation();moveChip('cp',${i},-1)" title="Move left"${leftDisabled}>&lt;</button><button type="button" class="chip-move" onclick="event.stopPropagation();moveChip('cp',${i},1)" title="Move right"${rightDisabled}>&gt;</button></span>`;
    return `<span class="chip score-chip${selected}" onclick="selectCoeffProgramLine(${i},event)">${moves}${bodyHtml}<span class="chip-x" onclick="event.stopPropagation();removeChip('cp',${i})">x</span></span>`;
}

// _coeffProgramLegacyInputDefs moved to js/07-transform-catalogs.js: the
// catalog IIFE there calls it at load time.

function _coeffProgramLegacyValues(params, legacyDefs) {
    const values = [
        String(params[0] || 'rev').trim() || 'rev',
        params[1] || 'poly',
        params[2] || 'poly',
    ];
    legacyDefs.forEach((pDef, idx) => {
        values.push(params[idx + 3] !== undefined && params[idx + 3] !== '' ? params[idx + 3] : String(pDef.def || ''));
    });
    return values;
}

function _coeffProgramLegacyAndyHtml(i, values, legacyDefs, options = {}) {
    const andyIdx = legacyDefs.findIndex(_isAndyParam);
    if (andyIdx < 0) return '';
    const paramIdx = 3 + andyIdx;
    return `<span class="chip-op">andy=</span>${_chipInputHtml('cp', i, paramIdx, values[paramIdx], legacyDefs[andyIdx] || _ctAndyParam, options)}`;
}

function _coeffProgramLegacyFormulaHtml(i, legacyName, values, legacyDefs, options = {}) {
    const pDefs = _coeffProgramCatalog.legacy.params || [];
    const fn = _chipInputHtml('cp', i, 0, values[0], pDefs[0] || {}, options);
    const src = _chipInputHtml('cp', i, 1, values[1], pDefs[1] || {}, options);
    const tgt = _chipInputHtml('cp', i, 2, values[2], pDefs[2] || {}, options);
    const arg = (idx) => _chipInputHtml('cp', i, 3 + idx, values[3 + idx], legacyDefs[idx] || {}, options);
    const andy = _coeffProgramLegacyAndyHtml(i, values, legacyDefs, options);
    if (legacyName === 'pow') {
        const field1 = arg(0);
        const field2 = arg(1);
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span>${fn}<span>(</span>${src}<span class="chip-op">*</span>${field1}<span class="chip-op">,</span>${field2}<span>)</span>${andy}</span>`;
    }
    if (legacyName === 'linear') {
        const field1 = arg(0);
        const field2 = arg(1);
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span>${fn}<span>(</span>${src}<span class="chip-op">*</span>${field1}<span class="chip-op">+</span>${field2}<span>)</span>${andy}</span>`;
    }
    if (legacyName === 'exp') {
        const field1 = arg(0);
        const field2 = arg(1);
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span>${fn}<span>(</span>${src}<span class="chip-op">*</span>${field1}<span class="chip-op">+</span>${field2}<span>)</span>${andy}</span>`;
    }
    if (legacyName === 'round') {
        const field1 = arg(0);
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span>${fn}<span>(</span>${src}<span class="chip-op">*</span>${field1}<span>)</span>${andy}</span>`;
    }
    const nonAndyArgs = legacyDefs
        .map((pDef, idx) => ({ pDef, idx }))
        .filter(entry => entry.pDef && entry.pDef.ph !== 'andy')
        .map(entry => `<span class="chip-op">,</span>${arg(entry.idx)}`)
        .join('');
    return `<span class="chip-formula">${tgt}<span class="chip-op">=</span>${fn}<span>(</span>${src}${nonAndyArgs}<span>)</span>${andy}</span>`;
}

function _coeffProgramNativeTransformFormulaHtml(item, i, pDefs, options = {}) {
    const params = Array.isArray(item.params) ? item.params : [];
    const input = (idx) => _chipInputHtml('cp', i, idx, _paramValueOrDefault(params, idx, pDefs[idx]), pDefs[idx] || {}, options);
    const tgt = input(0);
    const src = input(1);
    const arg = (idx) => input(2 + idx);
    const label = _escapeHtml((_coeffProgramCatalog[item.name] && _coeffProgramCatalog[item.name].label) || item.name);
    if (item.name === 'pow') {
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span><span>${label}(</span>${src}<span class="chip-op">*</span>${arg(0)}<span class="chip-op">,</span>${arg(1)}<span>)</span></span>`;
    }
    if (item.name === 'linear') {
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span>${src}<span class="chip-op">*</span>${arg(0)}<span class="chip-op">+</span>${arg(1)}</span>`;
    }
    if (item.name === 'exp_affine') {
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span><span>${label}(</span>${src}<span class="chip-op">*</span>${arg(0)}<span class="chip-op">+</span>${arg(1)}<span>)</span></span>`;
    }
    if (item.name === 'round') {
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span><span>${label}(</span>${src}<span class="chip-op">*</span>${arg(0)}<span>)</span></span>`;
    }
    const rest = (pDefs || [])
        .slice(2)
        .map((pDef, idx) => `<span class="chip-op">,</span>${arg(idx)}`)
        .join('');
    return `<span class="chip-formula">${tgt}<span class="chip-op">=</span><span>${label}(</span>${src}${rest}<span>)</span></span>`;
}

function _coeffProgramVectorFormulaHtml(item, i, pDefs, options = {}) {
    const params = Array.isArray(item.params) ? item.params : [];
    const input = (idx) => _chipInputHtml('cp', i, idx, _paramValueOrDefault(params, idx, pDefs[idx]), pDefs[idx] || {}, options);
    const tgt = input(0);
    if (item.name === 'set') {
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span>${input(1)}</span>`;
    }
    if (item.name === 'affine') {
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span>${input(1)}<span class="chip-op">*</span>${input(2)}<span class="chip-op">+</span>${input(3)}</span>`;
    }
    if (_coeffProgramVectorBinaryNames.includes(item.name) || item.name === 'argsort') {
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span><span>${_escapeHtml(item.name)}(</span>${input(1)}<span class="chip-op">,</span>${input(2)}<span>)</span></span>`;
    }
    if (_coeffProgramVectorUnaryNames.includes(item.name)) {
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span><span>${_escapeHtml(item.name)}(</span>${input(1)}<span>)</span></span>`;
    }
    if (item.name === 'roll' || item.name === 'rolr') {
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span><span>${_escapeHtml(item.name)}(</span>${input(1)}<span class="chip-op">,</span>${input(2)}<span>)</span></span>`;
    }
    if (item.name === 'littlewood') {
        const andyIdx = _ctAndyIndex(pDefs);
        return `<span class="chip-formula">${tgt}<span class="chip-op">=</span><span>littlewood(</span>${input(1)}<span class="chip-op">,</span>${input(2)}<span>)</span><span class="chip-op">andy=</span>${input(andyIdx)}</span>`;
    }
    return '';
}

function _renderCoeffProgramChipHtml(item, i, options = {}) {
    const readonly = !!options.readonly;
    const spec = _coeffProgramCatalog[item.name] || {};
    const params = Array.isArray(item.params) ? item.params : [];
    const label = _escapeHtml(spec.label || item.name);
    if (item.name === 'legacy') {
        const legacyName = String(params[0] || 'rev').trim() || 'rev';
        const legacyDefs = _coeffProgramLegacyInputDefs(legacyName);
        const values = _coeffProgramLegacyValues(params, legacyDefs);
        return _coeffProgramChipShellHtml(i, readonly, _coeffProgramLegacyFormulaHtml(i, legacyName, values, legacyDefs, { readonly }));
    }
    const pDefs = spec.params || [];
    if (spec.nativeTransform) {
        return _coeffProgramChipShellHtml(i, readonly, _coeffProgramNativeTransformFormulaHtml(item, i, pDefs, { readonly }));
    }
    if (
        _coeffProgramVectorBinaryNames.includes(item.name)
        || _coeffProgramVectorUnaryNames.includes(item.name)
        || item.name === 'set'
        || item.name === 'affine'
        || item.name === 'argsort'
        || item.name === 'roll'
        || item.name === 'rolr'
        || item.name === 'littlewood'
    ) {
        return _coeffProgramChipShellHtml(i, readonly, _coeffProgramVectorFormulaHtml(item, i, pDefs, { readonly }));
    }
    const inputsHtml = pDefs.map((pDef, pi) => {
        const value = params[pi] !== undefined && params[pi] !== '' ? params[pi] : String(pDef.def || '');
        if (item.name === 'push_vec' || item.name === 'push_scalar' || item.name === 'push_const' || item.name === 'push_linspace' || item.name === 'push_range' || item.name === 'const' || item.name === 'poke_poly' || item.name === 'poke_tos') {
            return _chipLabeledInputHtml('cp', i, pi, value, pDef || {}, { readonly });
        }
        return _chipInputHtml('cp', i, pi, value, pDef || {}, { readonly });
    }).join('');
    return _coeffProgramChipShellHtml(i, readonly, `<span class="chip-formula"><span>${label}</span>${inputsHtml}</span>`);
}

function _renderParamProgramChipHtml(item, i, options = {}) {
    const readonly = !!options.readonly;
    const spec = _ppCatalog[item.name] || {};
    const pDefs = spec.params || [];
    const params = Array.isArray(item.params) ? item.params : [];
    const label = _escapeHtml(spec.label || item.name);
    let inputDefs = pDefs.length ? pDefs : params.map(() => ({}));
    let inputValues = params;
    let showParamLabels = item.name === 'legacy';
    if (item.name === 'legacy' && _paramProgramIndependentLegacyTargets.has(String(params[0] || ''))) {
        inputDefs = pDefs.slice(0, 3);
        inputValues = params.slice(0, 3);
    } else if (item.name === 'legacy') {
        const [legacyName, src, tgt, args] = _normalizeLegacyBridgeParams(params[0], params[1], params[2], params.slice(3));
        const argDefs = _paramProgramLegacyArgDefs(legacyName);
        if (legacyName === 'moebius') {
            const baseDefs = pDefs.slice(0, 3);
            const baseValues = [legacyName, src, tgt];
            const coeffDefs = argDefs || [];
            const coeffValues = _paramProgramMoebiusArgsForUi(args.length ? args : ['1', '0', '0', '1']);
            const baseInputs = baseDefs.map((pDef, pi) =>
                _chipLabeledInputHtml('pp', i, pi, baseValues[pi], pDef || {}, { readonly })
            ).join('');
            const coeffInputs = coeffDefs.map((pDef, ci) =>
                _chipLabeledInputHtml('pp', i, ci + 3, coeffValues[ci] || pDef.def || '', pDef || {}, { readonly })
            );
            const [a, b, c, d] = coeffInputs;
            const body = `<span class="chip-formula"><span>${label}</span>${baseInputs}<span class="chip-op">·</span><span>t=(</span>${a}<span class="chip-op">*t+</span>${b}<span>)/(</span>${c}<span class="chip-op">*t+</span>${d}<span>)</span></span>`;
            return _paramProgramChipShellHtml(i, readonly, body);
        } else if (argDefs) {
            inputDefs = pDefs.slice(0, 3).concat(argDefs);
            inputValues = [legacyName, src, tgt, ...args];
        } else if (_paramProgramLegacyTakesNoArgs(legacyName)) {
            inputDefs = pDefs.slice(0, 3);
            inputValues = [legacyName, src, tgt];
        }
    }
    const inputsHtml = inputDefs.map((pDef, pi) => {
        const value = inputValues[pi] != null && inputValues[pi] !== ''
            ? inputValues[pi]
            : (pDef && pDef.def != null ? String(pDef.def) : '');
        return showParamLabels
            ? _chipLabeledInputHtml('pp', i, pi, value, pDef || {}, { readonly })
            : _chipInputHtml('pp', i, pi, value, pDef || {}, { readonly });
    }).join('');
    return _paramProgramChipShellHtml(i, readonly, `<span class="chip-formula"><span>${label}</span>${inputsHtml}</span>`);
}

function _renderParamProgramChipsHtml(program) {
    const chain = _normalizeParamProgramChain(program && program.chain);
    if (!chain.length) return _solveScoreModalMessageHtml('No param-program chips.');
    const chips = chain.map((item, i) => _renderParamProgramChipHtml(item, i, { readonly: true })).join('');
    return `<div class="chip-container solve-score-modal-chip-strip" aria-label="Param program chips">${chips}</div>`;
}

function _renderCoeffProgramChipsHtml(program) {
    const chain = _normalizeCoeffProgramChain(program && program.chain);
    if (!chain.length) return _solveScoreModalMessageHtml('No coeff-program chips.');
    const chips = chain.map((item, i) => _renderCoeffProgramChipHtml(item, i, { readonly: true })).join('');
    return `<div class="chip-container solve-score-modal-chip-strip" aria-label="Coeff program chips">${chips}</div>`;
}

function _renderChips(which) {
    const chain = _chainForWhich(which);
    const el = document.getElementById(which + '-chips');
    if (el) {
        if (which === 'pp') {
            el.innerHTML = chain.map((item, i) => _renderParamProgramChipHtml(item, i, { readonly: true })).join('');
            _syncParamPipelineModeUi();
            const sourceLen = _paramProgramSourceStatementCount(_getParamProgramSourceText());
            const chainLen = _serializeParamProgramChain().length;
            if (_paramProgramTextModeSelected()) {
                _paramProgramStatus(sourceLen
                    ? `Param Program selected · ${sourceLen} source statement${sourceLen === 1 ? '' : 's'}`
                    : 'Param Program selected · empty identity');
            } else {
                _paramProgramStatus(chainLen
                    ? `Param Program selected · ${_pluralize(chainLen, 'chip')}`
                    : 'Param Program selected · empty identity');
            }
        } else if (which === 'cp') {
            el.innerHTML = chain.map((item, i) => _renderCoeffProgramChipHtml(item, i, { readonly: true })).join('');
            _syncParamPipelineModeUi();
            const sourceLen = _coeffProgramSourceStatementCount(_getCoeffProgramSourceText());
            const chainLen = _serializeCoeffProgramChain().length;
            if (_coeffProgramTextModeSelected()) {
                _coeffProgramStatus(sourceLen
                    ? `Coeff Program selected · ${sourceLen} source statement${sourceLen === 1 ? '' : 's'}`
                    : 'Coeff Program selected · empty text source');
            } else {
                _coeffProgramStatus(chainLen
                    ? `Coeff Program selected · ${_pluralize(chainLen, 'chip')}`
                    : 'Coeff Program selected · empty identity');
            }
        } else if (which === 'rt' || which === 'palette-rt' || which === 'ss' || which === 'palette-ss') {
            const catalog = _catalogForChain(which);
            el.innerHTML = chain.map((item, i) => _renderRtChipHtml(item, i, which, catalog)).join('');
            if (which === 'rt' || which === 'palette-rt') {
                Array.from(el.children).filter(chip => chip.classList && chip.classList.contains('chip')).forEach((chip, i) => {
                    chip.insertAdjacentHTML('afterbegin', _chipMoveControlsHtml(which, i));
                });
            }
        } else {
            el.innerHTML = chain.map((name, i) =>
                `<span class="chip">${_escapeHtml(name)}<span class="chip-x" onclick="removeChip('${which}',${i})">x</span></span>`
            ).join('');
        }
    } else if (which !== 'ss' && which !== 'palette-ss') {
        return;
    }
    if (which === 'ss' || which === 'palette-ss') {
        const prefix = _solveScorePrefixForWhich(which);
        _syncSolveScoreAddOptions(which);
        _updateSolveScoreStackUi(which);
        try {
            _syncSolveScoreLegacyInputs(prefix);
        } catch (e) {
            // Invalid chains are allowed transiently while replacing the metric chip.
        }
        if (prefix === 'render') _syncScoreNormalizationUi();
        if (prefix === 'render') _updateSolveScoreButtons();
        if (prefix === 'palette') _syncPaletteColorInterpretationUi();
    }
}
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

const _fmtMs = (ms) => (ms/1000).toFixed(1) + 's';

let _viewMode = 'auto';
function selectViewMode(mode) {
    _viewMode = mode;
    document.getElementById('view-row-auto').classList.toggle('active', mode === 'auto');
    document.getElementById('view-row-square').classList.toggle('active', mode === 'square');
    document.getElementById('view-row-explicit').classList.toggle('active', mode === 'explicit');
    _updateRenderExplicitViewportAspect();
}


function _readRenderExplicitViewportBounds(options = {}) {
    const requireValid = !!options.requireValid;
    const fields = {
        minRe: document.getElementById('render-min-re')?.value,
        maxRe: document.getElementById('render-max-re')?.value,
        minIm: document.getElementById('render-min-im')?.value,
        maxIm: document.getElementById('render-max-im')?.value,
    };
    const parsed = {};
    for (const [key, raw] of Object.entries(fields)) {
        const text = String(raw == null ? '' : raw).trim();
        if (!text) {
            if (requireValid) throw new Error(`Explicit viewport requires ${key}`);
            return { minRe: null, maxRe: null, minIm: null, maxIm: null };
        }
        const num = Number(text);
        if (!Number.isFinite(num)) {
            if (requireValid) throw new Error(`Explicit viewport requires numeric ${key}`);
            return { minRe: null, maxRe: null, minIm: null, maxIm: null };
        }
        parsed[key] = num;
    }
    if (!(parsed.maxRe > parsed.minRe)) {
        throw new Error('Explicit viewport requires Max Re > Min Re');
    }
    if (!(parsed.maxIm > parsed.minIm)) {
        throw new Error('Explicit viewport requires Max Im > Min Im');
    }
    return parsed;
}

function _setRenderExplicitViewportBounds(bounds = {}) {
    const pairs = [
        ['render-min-re', bounds.minRe],
        ['render-max-re', bounds.maxRe],
        ['render-min-im', bounds.minIm],
        ['render-max-im', bounds.maxIm],
    ];
    for (const [id, value] of pairs) {
        const el = document.getElementById(id);
        if (!el) continue;
        if (Number.isFinite(Number(value))) el.value = String(Number(value));
    }
    _updateRenderExplicitViewportAspect();
}

function _updateRenderExplicitViewportAspect() {
    const el = document.getElementById('render-explicit-aspect');
    if (!el) return;
    try {
        const bounds = _readRenderExplicitViewportBounds({ requireValid: true });
        const reSpan = bounds.maxRe - bounds.minRe;
        const imSpan = bounds.maxIm - bounds.minIm;
        const aspect = reSpan / imSpan;
        el.textContent = `World aspect Δre:Δim = ${reSpan.toFixed(4)} : ${imSpan.toFixed(4)}${aspect !== 1 ? ' · output stays square, domain stretches on one axis' : ''}`;
    } catch (_err) {
        el.textContent = 'World aspect Δre:Δim = invalid explicit bounds';
    }
}

function _onRenderExplicitViewportInput() {
    selectViewMode('explicit');
    _updateRenderExplicitViewportAspect();
}






function _renderCommonParams(options = {}) {
    const jobId = document.getElementById('render-results-dir').value.trim();
    if (!jobId) throw new Error('No results dir — run Calculate first');
    const solveScore = _resolveSolveScoreState('render', { requireChain: !!options.requireSolveScore });
    const solveScoreProgramSourceText = options.requireSolveScore
        ? _requireSolveScoreProgramSourceText('render')
        : _effectiveSolveScoreProgramSourceText('render');
    const colorInterpretation = _selectedRenderColorInterpretation();
    const colorIssue = _solveScoreColorCompatibility(solveScore, colorInterpretation);
    if (options.requireSolveScore && colorIssue) throw new Error(colorIssue);
    const backgroundColor = _readRenderBackgroundColor({ requireValid: true });
    const explicitBounds = _viewMode === 'explicit'
        ? _readRenderExplicitViewportBounds({ requireValid: true })
        : { minRe: null, maxRe: null, minIm: null, maxIm: null };
    const pixEl = document.getElementById('render-pix');
    const pix = _clampRenderPix(pixEl && pixEl.value);
    if (pixEl && pix > 0) pixEl.value = String(pix);
    return {
        jobId,
        pix,
        fmt: document.getElementById('render-format').value,
        quality: parseInt(document.getElementById('render-quality').value),
        viewMode: _viewMode,
        quantile: parseFloat(document.getElementById('render-quantile').value) / 100,
        shim: parseFloat(document.getElementById('render-shim').value) / 100,
        squareExtent: parseFloat(document.getElementById('render-square-extent').value) || 2.0,
        minRe: explicitBounds.minRe,
        maxRe: explicitBounds.maxRe,
        minIm: explicitBounds.minIm,
        maxIm: explicitBounds.maxIm,
        solveScoreQuantile: solveScore.quantile,
        solveScoreMetric: solveScore.metric,
        solveScoreOmega: solveScore.omega,
        solveScoreOmegaPhase: solveScore.omega_phase,
        solveScoreOmegaEnabled: solveScore.omega_enabled,
        solveScoreChain: solveScore.chain,
        solveScoreProgramSourceText,
        solveScoreProgramSpec: solveScore.program_spec,
        solveScoreDisplay: solveScore.display,
        solveScoreMetrics: solveScore.metrics,
        solveScoreHasExplicitOutputs: !!solveScore.has_explicit_outputs,
        solveScoreOutputChannelCount: Number(solveScore.output_channel_count) || 1,
        solveScoreOutputChannels: solveScore.output_channels || [],
        solveScoreUsesCoeffSource: !!solveScore.uses_coeff_source,
        solveScoreUsesParamSource: !!solveScore.uses_param_source,
        solveScoreUsesNonSolveSource: !!solveScore.uses_non_solve_source,
        colorInterpretation,
        backgroundColor,
        solveScoreNormalize: !!document.getElementById('render-score-normalization')?.checked,
        rotation: (() => {
            const turns = parseFloat(document.getElementById('render-rotation').value) || 0;
            const dir = document.getElementById('render-rotation-dir').value;
            return (dir === 'cw' ? -turns : turns) * 2 * Math.PI;
        })(),
        rootTransforms: _rtChain.map(item =>
            item.params && item.params.length ? [item.name, ...item.params] : [item.name]),
        rootProgramSourceText: _effectiveRootProgramSourceText('render'),
    };
}

function _renderColorMtEligible() {
    return renderColorMode === 'solve_score';
}

function _associatedPaletteAllowedForColorMode(mode) {
    return mode === 'solve_score';
}

function _paletteRootTransforms() {
    return _paletteRtChain.map(item =>
        item.params && item.params.length ? [item.name, ...item.params] : [item.name]);
}

let _paletteInventory = [];
let _paletteSelectedIdx = -1;
let _paletteSelectedKey = '';
let _paletteLoadedJobId = '';
let _activePaletteRun = null;
let _paletteObserverTimer = null;
let _lastPaletteLoggedPhase = null;
let _lastPaletteWarnState = null;
let _palettePhaseTracker = null;
const PALETTE_NO_ROW_STALE_MS = 120000;
const PALETTE_WARN_STALE_MS = 300000;
const PALETTE_HARD_STALE_MS = 900000;

// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

function _clearPaletteCanvas(msg) {
    const canvas = document.getElementById('palette-canvas');
    if (!canvas || !canvas.getContext) return;
    const ctx = canvas.getContext('2d');
    canvas.width = 512;
    canvas.height = 512;
    if (!ctx) return;
    ctx.fillStyle = '#000';
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    ctx.fillStyle = '#666';
    ctx.font = '14px monospace';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(msg || 'No palette selected', canvas.width / 2, canvas.height / 2);
}

function _drawPaletteCanvas(url) {
    const canvas = document.getElementById('palette-canvas');
    if (!canvas || !canvas.getContext || !url) { _clearPaletteCanvas('No palette image'); return; }
    const ctx = canvas.getContext('2d');
    if (!ctx) return;
    const img = new Image();
    img.onload = function() {
        const w = img.naturalWidth || img.width || 512;
        const h = img.naturalHeight || img.height || 512;
        canvas.width = w;
        canvas.height = h;
        ctx.imageSmoothingEnabled = false;
        ctx.clearRect(0, 0, w, h);
        ctx.drawImage(img, 0, 0, w, h);
    };
    img.onerror = function() {
        _clearPaletteCanvas('Failed to load palette image');
    };
    img.src = url;
}

function _paletteStableKey(pal) {
    if (!pal) return '';
    return String(pal.palette_id || pal.image_key || pal.created_at || '');
}

function _paletteSelectKey(key) {
    const nextKey = String(key || '');
    const idx = _paletteInventory.findIndex((pal) => _paletteStableKey(pal) === nextKey);
    if (idx < 0) return;
    _paletteSelectedIdx = idx;
    _paletteSelectedKey = nextKey;
    document.querySelectorAll('.palette-inv-row').forEach((r, i) => {
        r.style.background = i === idx ? '#2a2a4e' : '';
    });
    const row = document.querySelector(`.palette-inv-row[data-key="${_encodeStableRowKey(nextKey)}"]`);
    if (row) row.scrollIntoView({ block: 'nearest' });

    const pal = _paletteInventory[idx];
    const infoEl = document.getElementById('palette-info');
    const rtSummary = (pal.root_transforms || []).map(x => Array.isArray(x) ? (x[0] + (x.length > 1 ? '(' + x.slice(1).join(',') + ')' : '')) : x).join(', ');
    const clipLo = pal.clip_lo != null && pal.clip_lo.toFixed ? pal.clip_lo.toFixed(3) : pal.clip_lo;
    const clipHi = pal.clip_hi != null && pal.clip_hi.toFixed ? pal.clip_hi.toFixed(3) : pal.clip_hi;
    infoEl.textContent = `${pal.display_name || pal.palette_id || ''} | clip ${clipLo} .. ${clipHi} | RT ${rtSummary || 'none'}`;
    _drawPaletteCanvas(pal.image_url || pal.preview_url || '');
    const populateBtn = document.getElementById('btn-palette-populate');
    const dlBtn = document.getElementById('btn-palette-download');
    const delBtn = document.getElementById('btn-palette-delete');
    if (populateBtn) populateBtn.disabled = false;
    if (dlBtn) dlBtn.disabled = false;
    if (delBtn) delBtn.disabled = false;
}

async function loadPaletteInventory(opts) {
    const options = opts || {};
    const jobId = document.getElementById('palette-results-dir').value.trim();
    const container = document.getElementById('palette-inventory');
    const infoEl = document.getElementById('palette-info');
    const statusEl = document.getElementById('palette-status');
    const populateBtn = document.getElementById('btn-palette-populate');
    const dlBtn = document.getElementById('btn-palette-download');
    const delBtn = document.getElementById('btn-palette-delete');
    if (!container) return;
    if (!jobId) {
        container.innerHTML = '<div style="color:#666; padding:8px">No results dir selected.</div>';
        infoEl.textContent = '';
        if (populateBtn) populateBtn.disabled = true;
        if (dlBtn) dlBtn.disabled = true;
        if (delBtn) delBtn.disabled = true;
        _paletteInventory = [];
        _paletteSelectedIdx = -1;
        _paletteSelectedKey = '';
        _paletteLoadedJobId = '';
        _clearPaletteCanvas('No palette selected');
        return;
    }

    container.innerHTML = 'Loading...';
    statusEl.textContent = 'Loading...';
    statusEl.className = 'status';
    try {
        if (_paletteLoadedJobId !== jobId) _paletteSelectedKey = '';
        const data = await lambdaPost('storage', { job_id: jobId }, '/list-palettes');
        _paletteInventory = data.palettes || [];
        if (!_paletteInventory.length) {
            container.innerHTML = '<div style="color:#666; padding:8px">No saved palettes yet.</div>';
            infoEl.textContent = '';
            statusEl.textContent = 'Ready';
            statusEl.className = 'status ok';
            if (populateBtn) populateBtn.disabled = true;
            if (dlBtn) dlBtn.disabled = true;
            if (delBtn) delBtn.disabled = true;
            _paletteSelectedIdx = -1;
            _paletteSelectedKey = '';
            _paletteLoadedJobId = jobId;
            _clearPaletteCanvas('No palette selected');
            return;
        }

        let html = '<table style="width:100%; border-collapse:collapse" id="palette-inv-table">';
        html += '<tr style="border-bottom:1px solid #333; position:sticky; top:0; background:#1a1a2e"><th style="text-align:left;padding:4px">Created</th><th>Metric</th><th>q</th><th>Palette</th><th>RT</th></tr>';
        _paletteInventory.forEach((pal, i) => {
            const rtCount = (pal.root_transforms || []).length;
            const key = _encodeStableRowKey(_paletteStableKey(pal));
            html += `<tr class="palette-inv-row" data-key="${key}" style="border-bottom:1px solid #222; cursor:pointer" onclick="_paletteSelectKey(_decodeStableRowKey(this.dataset.key || ''))" tabindex="-1">`;
            html += `<td style="padding:4px; font-size:11px">${_escapeHtml((pal.created_at || '').replace('T', ' ').slice(0, 19))}</td>`;
            html += `<td style="padding:4px; text-align:center">${_escapeHtml(pal.metric || '')}</td>`;
            html += `<td style="padding:4px; text-align:center">${_escapeHtml(pal.solve_score_quantile != null ? (pal.solve_score_quantile * 100).toFixed(1) + '%' : '')}</td>`;
            html += `<td style="padding:4px; text-align:center">${_escapeHtml(pal.palette || '')}</td>`;
            html += `<td style="padding:4px; text-align:center">${rtCount}</td>`;
            html += '</tr>';
        });
        html += '</table>';
        container.innerHTML = html;

        let selectIdx = 0;
        if (options.selectPaletteId) {
            const matchIdx = _paletteInventory.findIndex(p => p.palette_id === options.selectPaletteId);
            if (matchIdx >= 0) selectIdx = matchIdx;
        } else if (_paletteSelectedKey) {
            const matchIdx = _paletteInventory.findIndex((pal) => _paletteStableKey(pal) === _paletteSelectedKey);
            if (matchIdx >= 0) selectIdx = matchIdx;
        }
        _paletteSelect(selectIdx);
        _paletteLoadedJobId = jobId;
        statusEl.textContent = 'Ready';
        statusEl.className = 'status ok';
    } catch (e) {
        _setInlineError(container, e.message, 'color:#e94560; padding:8px');
        infoEl.textContent = '';
        statusEl.textContent = 'Refresh failed: ' + e.message;
        statusEl.className = 'status error';
        if (populateBtn) populateBtn.disabled = true;
        if (dlBtn) dlBtn.disabled = true;
        if (delBtn) delBtn.disabled = true;
        _paletteInventory = [];
        _paletteSelectedIdx = -1;
        _paletteSelectedKey = '';
        _clearPaletteCanvas('No palette selected');
        if (options.throwOnError) throw e;
    }
}

async function refreshPaletteInventory() {
    const jobId = document.getElementById('palette-results-dir').value.trim();
    const btn = document.getElementById('btn-palette-refresh');
    const orig = btn ? btn.textContent : 'Refresh';
    if (!jobId) {
        log('Palette refresh failed: no results dir selected', 'err', 'palette-log');
        const statusEl = document.getElementById('palette-status');
        if (statusEl) {
            statusEl.textContent = 'Refresh failed: no results dir selected';
            statusEl.className = 'status error';
        }
        return;
    }
    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Refreshing...'; }
        await loadPaletteInventory({ throwOnError: true });
        log(`Palette refreshed: ${jobId}`, 'ok', 'palette-log');
    } catch (e) {
        log(`Palette refresh failed: ${e.message}`, 'err', 'palette-log');
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

function _paletteSelect(idx) {
    if (idx < 0 || idx >= _paletteInventory.length) return;
    _paletteSelectKey(_paletteStableKey(_paletteInventory[idx]));
}

async function downloadSelectedPalette() {
    const jobId = document.getElementById('palette-results-dir').value.trim();
    const idx = _paletteSelectedIdx;
    if (!jobId || idx < 0 || idx >= _paletteInventory.length) return;
    const pal = _paletteInventory[idx];
    const filename = `${jobId}_${pal.palette_id || 'palette'}.jpeg`;
    await downloadPresignedFile(pal.image_url || pal.preview_url || '', filename, pal.image_key);
}

function _paletteArtifactSolveScoreChain(pal) {
    if (!pal) return [];
    const candidates = [
        pal.solve_score_chain,
        pal.palette_source_score_chain,
        pal.associated_palette_score_chain,
    ];
    for (const chain of candidates) {
        if (Array.isArray(chain) && chain.length) return chain;
    }

    const metric = String(pal.metric || paletteTabMetric || 'proximity');
    const quantilePct = pal.solve_score_quantile != null
        ? Number(pal.solve_score_quantile) * 100
        : 0.1;
    return _defaultSolveScoreChain(
        metric,
        Number.isFinite(quantilePct) ? quantilePct : 0.1,
        pal.solve_score_omega != null ? pal.solve_score_omega : 1,
        pal.solve_score_omega_enabled != null ? _boolish(pal.solve_score_omega_enabled, true) : false,
        pal.solve_score_omega_phase != null ? pal.solve_score_omega_phase : 0
    );
}

function _setPaletteRootTransformsFromArtifact(transforms) {
    if (!Array.isArray(transforms)) return false;
    const next = transforms.map(item => {
        if (item && typeof item === 'object' && !Array.isArray(item)) {
            const name = String(item.name || '').trim();
            if (!name) return null;
            const args = Array.isArray(item.args) ? item.args : (Array.isArray(item.params) ? item.params : []);
            return { name, params: args.map(v => String(v)) };
        }
        if (!Array.isArray(item) || !item.length) return null;
        return { name: item[0], params: item.slice(1).map(v => String(v)) };
    }).filter(Boolean);
    _paletteRtChain.splice(0, _paletteRtChain.length, ...next);
    _renderChips('palette-rt');
    return true;
}

function populateSelectedPalette() {
    const jobId = document.getElementById('palette-results-dir')?.value.trim() || '';
    const idx = _paletteSelectedIdx;
    if (!jobId || idx < 0 || idx >= _paletteInventory.length) return;
    const pal = _paletteInventory[idx];
    const warnings = [];

    const fallbackMetric = String(pal.metric || paletteTabMetric || 'proximity');
    const quantilePct = pal.solve_score_quantile != null
        ? Number(pal.solve_score_quantile) * 100
        : 0.1;
    const chain = _normalizeSolveScoreChain(
        _paletteArtifactSolveScoreChain(pal),
        fallbackMetric,
        Number.isFinite(quantilePct) ? quantilePct : 0.1
    );
    if (chain.length) {
        const target = _chainForWhich('palette-ss');
        target.splice(0, target.length, ...chain);
        _renderChips('palette-ss');
        try {
            _syncSolveScoreLegacyInputs('palette');
        } catch (_) {
            paletteTabMetric = fallbackMetric;
        }
        _solveScoreProgramRememberedNames.palette = '';
        _setSolveScoreProgramStatus('palette', `Populated from ${pal.display_name || pal.palette_id || 'selected palette'}`, false);
        if (typeof _restoreSolveScoreSourceFromArtifact === 'function') {
            _restoreSolveScoreSourceFromArtifact('palette', pal);
        }
    } else {
        warnings.push('solve-score program');
    }

    if (pal.palette) {
        setPaletteForMode('palette_tab', pal.palette);
    } else {
        warnings.push('palette');
    }

    const colorInterpretation = pal.color_interpretation || pal.score_output_interpretation || pal.interpretation;
    if (colorInterpretation) {
        _setPaletteColorInterpretation(colorInterpretation);
    } else {
        warnings.push('mode');
    }

    if (Array.isArray(pal.root_transforms)) {
        _setPaletteRootTransformsFromArtifact(pal.root_transforms);
    }
    if (typeof _restoreRootSourceFromArtifact === 'function') {
        _restoreRootSourceFromArtifact('palette', pal);
    }

    _updatePaletteCreateButton();
    const label = pal.display_name || pal.palette_id || 'selected palette';
    const msg = warnings.length
        ? `Populate complete (missing saved ${warnings.join(', ')})`
        : `Populate complete: ${label}`;
    const statusEl = document.getElementById('palette-status');
    if (statusEl) {
        statusEl.textContent = msg;
        statusEl.className = warnings.length ? 'status' : 'status ok';
    }
    log(msg, warnings.length ? '' : 'ok', 'palette-log');
}

async function deleteSelectedPalette() {
    const jobId = document.getElementById('palette-results-dir').value.trim();
    const idx = _paletteSelectedIdx;
    if (!jobId || idx < 0 || idx >= _paletteInventory.length) return;
    const pal = _paletteInventory[idx];
    if (!confirm(`Delete palette ${pal.display_name || pal.palette_id}?`)) return;
    const btn = document.getElementById('btn-palette-delete');
    const orig = btn ? btn.textContent : '';
    if (btn) { btn.disabled = true; btn.textContent = 'Deleting...'; }
    try {
        await lambdaPost('storage', { job_id: jobId, palette_id: pal.palette_id }, '/delete-palette');
        log(`Palette deleted: ${pal.palette_id}`, 'ok', 'palette-log');
        await loadPaletteInventory();
    } catch (e) {
        log(`Palette delete failed: ${e.message}`, 'err', 'palette-log');
    } finally {
        if (btn) { btn.textContent = orig; }
    }
}

async function runPaletteArtifact() {
    const btn = document.getElementById('btn-palette-create');
    const statusEl = document.getElementById('palette-status');
    const jobId = document.getElementById('palette-results-dir').value.trim();
    if (!jobId) { log('Palette: no results dir', 'err', 'palette-log'); return; }
    if (_blockPaletteActionIfActive('Palette')) return;

    try {
        if (btn) { btn.disabled = true; btn.textContent = 'Palette...'; }
        statusEl.textContent = 'Dispatching orchestrator...';
        statusEl.className = 'status';
        log('Palette artifact: starting...', '', 'palette-log');

        const score = _resolveSolveScoreState('palette', { requireChain: true });
        const scoreSourceText = _requireSolveScoreProgramSourceText('palette');
        const colorInterpretation = _selectedPaletteColorInterpretation();
        const paletteIssue = _solveScorePaletteCompatibility(score, colorInterpretation);
        if (paletteIssue) throw new Error(paletteIssue);
        const runId = _generateRunId();
        const taskId = 'palette_run_' + runId;
        const orchPayload = {
            job_id: jobId,
            run_id: runId,
            task_id: taskId,
            params: {
                metric: score.metric,
                palette: paletteTabPalette,
                color_interpretation: colorInterpretation,
                solve_score_quantile: score.quantile,
                solve_score_omega: score.omega,
                solve_score_omega_enabled: score.omega_enabled,
                solve_score_program_source_text: scoreSourceText,
                root_transforms: _paletteRootTransforms(),
                root_program_source_text: _effectiveRootProgramSourceText('palette') || undefined,
            },
        };
        const dispResult = await lambdaPost('dispatch', {
            target: 'palette_orchestrator',
            jobs: [orchPayload],
            expected_keys: [],
        });
        if ((dispResult.fired || 0) !== 1) throw new Error('Palette orchestrator dispatch failed');

        _saveActivePaletteRun({
            job_id: jobId,
            run_id: runId,
            task_id: taskId,
            started_at_ms: Date.now(),
            mode: 'palette',
            origin: 'palette_tab',
        });
        log('  orchestrator dispatched: ' + runId, 'ok', 'palette-log');
        startActivePaletteObserver();
    } catch (e) {
        log('Palette artifact failed: ' + e.message, 'err', 'palette-log');
        statusEl.textContent = 'Palette failed';
        statusEl.className = 'status error';
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = 'Palette'; }
    }
}

function _saveActivePaletteRun(record) {
    _activePaletteRun = record;
    try { localStorage.setItem('polypaint_active_palette_run', JSON.stringify(record)); } catch(e) {}
    try { _updateRenderActionButtons(); } catch(e) {}
}

function _clearActivePaletteRun() {
    _activePaletteRun = null;
    _lastPaletteLoggedPhase = null;
    _lastPaletteWarnState = null;
    _palettePhaseTracker = null;
    try { localStorage.removeItem('polypaint_active_palette_run'); } catch(e) {}
    try { _updateRenderActionButtons(); } catch(e) {}
}

function _loadActivePaletteRun() {
    try {
        const s = localStorage.getItem('polypaint_active_palette_run');
        if (s) return JSON.parse(s);
    } catch(e) {}
    return null;
}

function _currentActivePaletteRun() {
    const run = _activePaletteRun || _loadActivePaletteRun();
    if (run && !_activePaletteRun) _activePaletteRun = run;
    return run || null;
}

function _paletteRunAgeMs(run) {
    if (!run || !run.started_at_ms) return null;
    const started = Number(run.started_at_ms);
    return Number.isFinite(started) ? Math.max(0, Date.now() - started) : null;
}

function _paletteRunIsHardStale(run) {
    const ageMs = _paletteRunAgeMs(run);
    return ageMs !== null && ageMs > PALETTE_HARD_STALE_MS;
}

function _paletteRunBlocksNewRun() {
    const run = _currentActivePaletteRun();
    return !!(run && !_paletteRunIsHardStale(run));
}

function _blockPaletteActionIfActive(actionLabel) {
    const run = _currentActivePaletteRun();
    if (!run) return false;
    const runLabel = _paletteRunLabel(run);
    const mirrorToRender = _shouldMirrorPaletteRunToRender(run);
    if (_paletteRunIsHardStale(run)) {
        _clearActivePaletteRun();
        stopActivePaletteObserver();
        const msg = `${runLabel} lock was stale for 15+ min; cleared before ${actionLabel}.`;
        log(msg, '', 'palette-log');
        if (mirrorToRender) log(msg, '', 'render-log');
        return false;
    }
    startActivePaletteObserver();
    const msg = `${actionLabel}: ${runLabel} already in progress; resumed status observer.`;
    const statusEl = document.getElementById('palette-status');
    if (statusEl) {
        statusEl.textContent = msg;
        statusEl.className = 'status';
    }
    log(msg, '', 'palette-log');
    if (mirrorToRender) {
        const renderStatusEl = document.getElementById('render-status');
        if (renderStatusEl) {
            renderStatusEl.textContent = msg;
            renderStatusEl.className = 'status';
        }
        log(msg, '', 'render-log');
    }
    return true;
}

function startActivePaletteObserver() {
    stopActivePaletteObserver();
    _paletteObserverTimer = setInterval(_pollActivePaletteRun, 3000);
    _pollActivePaletteRun();
}

function stopActivePaletteObserver() {
    if (_paletteObserverTimer) {
        clearInterval(_paletteObserverTimer);
        _paletteObserverTimer = null;
    }
}

function resumeActivePaletteObserver() {
    const run = _loadActivePaletteRun();
    if (!run) return;
    _activePaletteRun = run;
    startActivePaletteObserver();
}

function _paletteRunLabel(run) {
    return run && run.mode === 'extract_palette' ? 'ExtractPalette' : 'Palette';
}

function _paletteRunCompleteLog(run, rd) {
    const runLabel = _paletteRunLabel(run);
    const target = rd.palette_id || rd.image_key || (run ? run.run_id : '');
    const elapsedMs = _serverElapsedMs(
        rd && rd.run_started_at_ms ? rd.run_started_at_ms : (run ? run.server_started_at_ms : null),
        rd && rd.updated_at_ms ? rd.updated_at_ms : null
    );
    const elapsed = elapsedMs != null ? ` (${_fmtMs(elapsedMs)})` : '';
    return `${runLabel} complete: ${target}${elapsed}`;
}

function _shouldMirrorPaletteRunToRender(run) {
    const renderActive = !!document.getElementById('tab-render')?.classList.contains('active');
    if (!renderActive) return false;
    if (run && run.origin === 'render_extract_palette') return true;
    return _renderActiveFamily === 'palette';
}

function _aggregatePaletteChunkPerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (r.dl_ms != null || r.compute_ms != null || r.upload_ms != null || r.step_count != null)) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        acc.tasks += 1;
        acc.dl_ms += Number(row.dl_ms) || 0;
        acc.compute_ms += Number(row.compute_ms) || 0;
        acc.upload_ms += Number(row.upload_ms) || 0;
        acc.step_count += Number(row.step_count) || 0;
        acc.threads = Math.max(acc.threads, Number(row.threads) || 1);
        if (!acc.requested_input_mode && row.requested_input_mode) acc.requested_input_mode = String(row.requested_input_mode);
        if (!acc.input_mode && row.input_mode) acc.input_mode = String(row.input_mode);
        acc.retries = Math.max(acc.retries, Number(row.retries) || 0);
        acc.source_size += Number(row.source_size) || 0;
        acc.workers = Math.max(acc.workers, Number(row.workers) || 0);
        return acc;
    }, { tasks: 0, dl_ms: 0, compute_ms: 0, upload_ms: 0, step_count: 0, threads: 1, requested_input_mode: '', input_mode: '', retries: 0, source_size: 0, workers: 0 });
}

function _aggregatePaletteFinalizePerf(results) {
    const rows = Array.isArray(results) ? results.filter(r => r && (r.assemble_ms != null || r.render_ms != null || r.encode_ms != null)) : [];
    if (!rows.length) return null;
    return rows.reduce((acc, row) => {
        acc.tasks += 1;
        acc.assemble_ms += Number(row.assemble_ms) || 0;
        acc.render_ms += Number(row.render_ms) || 0;
        acc.encode_ms += Number(row.encode_ms) || 0;
        acc.file_size += Number(row.file_size) || 0;
        acc.pass0_chunks_read += Number(row.pass0_chunks_read) || 0;
        acc.pass0_chunks_skipped += Number(row.pass0_chunks_skipped) || 0;
        acc.pass0_chunk_count += Number(row.pass0_chunk_count) || 0;
        acc.pass0_bytes_read += Number(row.pass0_bytes_read) || 0;
        return acc;
    }, { tasks: 0, assemble_ms: 0, render_ms: 0, encode_ms: 0, file_size: 0, pass0_chunks_read: 0, pass0_chunks_skipped: 0, pass0_chunk_count: 0, pass0_bytes_read: 0 });
}

function _palettePhaseSubtaskInfo(run, rd) {
    if (!run || !rd) return null;
    const phase = rd.phase || '';
    if (rd.subtask_prefix && rd.expected) {
        return { prefix: rd.subtask_prefix, expected: rd.expected, phase };
    }
    if (phase === 'solve_score_clip') {
        return { prefix: `palette_${run.run_id}_solve_score_clip`, expected: 1, phase };
    }
    if (phase === 'solve_score_merge') {
        return { prefix: `palette_${run.run_id}_solve_score_merge`, expected: 1, phase };
    }
    if (phase === 'palette_finalize') {
        return { prefix: `palette_${run.run_id}_finalize`, expected: 1, phase };
    }
    if (phase === 'attach_associated_palette') {
        return { prefix: `palette_${run.run_id}_attach`, expected: 1, phase };
    }
    return null;
}

function _palettePhasePerfSummary(phase, results, wallMs) {
    if (phase === 'solve_score_clip' || phase === 'solve_score_hist' || phase === 'solve_score_merge') {
        const perf = _aggregateSolveScorePhasePerf(results);
        if (!perf) return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
        let msg = `wall=${_fmtSecondsMs(wallMs)} total=dl ${_fmtSecondsMs(perf.dl_ms)} + compute ${_fmtSecondsMs(perf.compute_ms)}`;
        if (perf.upload_ms > 0) msg += ` + up ${_fmtSecondsMs(perf.upload_ms)}`;
        msg += ` · threads=${perf.threads}`;
        if (phase === 'solve_score_hist') {
            const inputLabel = _formatRequestedEffectiveInputMode(perf);
            if (inputLabel) msg += ` · input=${inputLabel}`;
            if (perf.input_mode === 'sectioned' || perf.input_mode === 'multispan_sectioned') msg += ` · retries=${perf.retries}`;
        }
        if (phase === 'solve_score_merge' && perf.threads > 0) {
            msg += ` · workers=${perf.threads}`;
        }
        return msg;
    }
    if (phase === 'palette_chunk') {
        const perf = _aggregatePaletteChunkPerf(results);
        if (!perf) return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
        let msg = `wall=${_fmtSecondsMs(wallMs)} total=dl ${_fmtSecondsMs(perf.dl_ms)} + native ${_fmtSecondsMs(perf.compute_ms)} + up ${_fmtSecondsMs(perf.upload_ms)}`;
        msg += ` · threads=${perf.threads}`;
        const inputLabel = _formatRequestedEffectiveInputMode(perf);
        if (inputLabel) msg += ` · input=${inputLabel}`;
        if (perf.input_mode === 'sectioned' || perf.input_mode === 'multispan_sectioned') msg += ` · retries=${perf.retries}`;
        if (perf.workers > 0) msg += ` · workers=${perf.workers}`;
        if (perf.step_count > 0) msg += ` · solves=${perf.step_count}`;
        return msg;
    }
    if (phase === 'palette_finalize') {
        const perf = _aggregatePaletteFinalizePerf(results);
        if (!perf) return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
        let msg = `wall=${_fmtSecondsMs(wallMs)} total=assemble ${_fmtSecondsMs(perf.assemble_ms)} + render ${_fmtSecondsMs(perf.render_ms)} + encode ${_fmtSecondsMs(perf.encode_ms)}`;
        if (perf.pass0_chunk_count > 0) msg += ` · pass0=${perf.pass0_chunks_read}/${perf.pass0_chunk_count}`;
        if (perf.pass0_chunks_skipped > 0) msg += ` skipped=${perf.pass0_chunks_skipped}`;
        if (perf.pass0_bytes_read > 0) msg += ` · bins=${_fmtSize(perf.pass0_bytes_read)}`;
        if (perf.file_size > 0) msg += ` · size ${(perf.file_size / 1e6).toFixed(1)}MB`;
        return msg;
    }
    return wallMs != null ? `wall=${_fmtSecondsMs(wallMs)}` : '';
}

function _updatePalettePhaseTracker(run, rd) {
    if (!run || !rd) return;
    const phase = rd.phase || rd.phase_label || '';
    const startedAtMs = _coerceTimestampMs(rd.started_at_ms) || _coerceTimestampMs(rd.updated_at_ms) || Date.now();
    const updatedAtMs = _coerceTimestampMs(rd.updated_at_ms) || startedAtMs;
    if (!_palettePhaseTracker || _palettePhaseTracker.phase !== phase) {
        const info = _palettePhaseSubtaskInfo(run, rd);
        _palettePhaseTracker = {
            phase,
            phase_label: rd.phase_label || phase || 'working',
            started_at_ms: startedAtMs,
            last_server_update_ms: updatedAtMs,
            prefix: info ? info.prefix : '',
            expected: info ? info.expected : 0,
        };
    } else if (updatedAtMs != null) {
        _palettePhaseTracker.last_server_update_ms = updatedAtMs;
    }
}

async function _logPalettePhaseCompletion(run, tracker, mirrorToRender) {
    if (!run || !tracker || !tracker.prefix) return;
    try {
        const subcheck = await lambdaPost('storage', {
            job_id: run.job_id,
            task_prefix: tracker.prefix,
            expected: tracker.expected || 1,
        }, '/check-status');
        const wallMs = _serverElapsedMs(
            tracker.started_at_ms,
            _pickLatestTimestampMs(
                subcheck.latest_done_ms,
                subcheck.latest_update_ms,
                tracker.last_server_update_ms
            )
        );
        const perfText = _palettePhasePerfSummary(tracker.phase, subcheck.results || [], wallMs);
        if (!perfText) return;
        const msg = `${tracker.phase_label} complete · ${perfText}`;
        log(msg, '', 'palette-log');
        if (mirrorToRender) log(msg, '', 'render-log');
    } catch (e) {
        // best effort only
    }
}

async function _pollActivePaletteRun() {
    const run = _activePaletteRun || _loadActivePaletteRun();
    if (!run) { stopActivePaletteObserver(); return; }
    const statusEl = document.getElementById('palette-status');
    const runLabel = _paletteRunLabel(run);
    const mirrorToRender = _shouldMirrorPaletteRunToRender(run);

    try {
        const check = await lambdaPost('storage', {
            job_id: run.job_id, task_prefix: run.task_id, expected: 1,
        }, '/check-status');

        if (check.errors > 0) {
            _logContractWarnings(check.error_details || [], 'palette-log');
            const msg = check.error_details?.[0]?.error_msg || 'unknown error';
            const ctx = _formatTaskErrorContext(check.error_details?.[0] || {});
            statusEl.textContent = runLabel + ' error: ' + msg;
            statusEl.className = 'status error';
            log(runLabel + ' failed: ' + msg, 'err', 'palette-log');
            if (ctx) log('  context: ' + ctx, 'err', 'palette-log');
            if (mirrorToRender) {
                const renderStatusEl = document.getElementById('render-status');
                if (renderStatusEl) {
                    renderStatusEl.textContent = runLabel + ' error: ' + msg;
                    renderStatusEl.className = 'status error';
                }
                log(runLabel + ' failed: ' + msg, 'err', 'render-log');
                if (ctx) log('  context: ' + ctx, 'err', 'render-log');
            }
            _clearActivePaletteRun();
            stopActivePaletteObserver();
            return;
        }

        const rd = check.results?.[0] || {};
        _logContractWarnings([rd], 'palette-log');
        if (document.getElementById('tab-render')?.classList.contains('active') && _renderActiveFamily === 'palette') {
            _logContractWarnings([rd], 'render-log');
        }
        if (!rd.phase && !rd.phase_label) {
            const ageMs = Date.now() - (run.started_at_ms || 0);
            if (ageMs > PALETTE_NO_ROW_STALE_MS) {
                statusEl.textContent = 'Ready';
                statusEl.className = 'status';
                _clearActivePaletteRun();
                stopActivePaletteObserver();
                return;
            }
        }

        const phase = rd.phase_label || rd.phase || 'working';

        if (check.complete || rd.phase === 'done') {
            if (_palettePhaseTracker) {
                await _logPalettePhaseCompletion(run, _palettePhaseTracker, mirrorToRender);
            }
            document.getElementById('palette-results-dir').value = run.job_id;
            await loadPaletteInventory({ selectPaletteId: rd.palette_id });
            if (run.origin === 'render_extract_palette') {
                await refreshRenderArtifacts(run.job_id, {
                    selectFamily: 'color',
                    selectArtifactId: rd.artifact_id || run.source_artifact_id || null,
                });
            } else {
                await refreshRenderArtifacts(run.job_id, { selectFamily: 'palette', selectArtifactId: rd.palette_id });
            }
            statusEl.textContent = runLabel + ' complete';
            statusEl.className = 'status ok';
            const completeMsg = _paletteRunCompleteLog(run, rd);
            log(completeMsg, 'ok', 'palette-log');
            if (mirrorToRender) {
                const renderStatusEl = document.getElementById('render-status');
                if (renderStatusEl) {
                    renderStatusEl.textContent = runLabel + ' complete';
                    renderStatusEl.className = 'status ok';
                }
                log(completeMsg, 'ok', 'render-log');
            }
            _clearActivePaletteRun();
            stopActivePaletteObserver();
            return;
        }

        let done = 0, expected = 0, workerFreshMs = null, subResults = [], phaseLatestMs = _pickLatestTimestampMs(rd.updated_at_ms, check.latest_update_ms);
        if (rd.subtask_prefix && rd.expected) {
            try {
                const subcheck = await lambdaPost('storage', {
                    job_id: run.job_id, task_prefix: rd.subtask_prefix, expected: rd.expected,
                }, '/check-status');
                done = subcheck.done || 0;
                expected = subcheck.expected || rd.expected || 0;
                subResults = subcheck.results || [];
                _logContractWarnings(subResults, 'palette-log');
                if (mirrorToRender) {
                    _logContractWarnings(subResults, 'render-log');
                }
                phaseLatestMs = _pickLatestTimestampMs(
                    subcheck.latest_done_ms,
                    subcheck.latest_update_ms,
                    subcheck.latest_nonterminal_ms,
                    phaseLatestMs
                );
                if (subcheck.latest_update_ms) workerFreshMs = Date.now() - subcheck.latest_update_ms;
            } catch(e) {
                expected = rd.expected || 0;
            }
        } else {
            const phaseInfo = _palettePhaseSubtaskInfo(run, rd);
            if (phaseInfo && phaseInfo.prefix) {
                try {
                    const subcheck = await lambdaPost('storage', {
                        job_id: run.job_id, task_prefix: phaseInfo.prefix, expected: phaseInfo.expected || 1,
                    }, '/check-status');
                    done = subcheck.done || 0;
                    expected = subcheck.expected || phaseInfo.expected || 0;
                    subResults = subcheck.results || [];
                    _logContractWarnings(subResults, 'palette-log');
                    if (mirrorToRender) {
                        _logContractWarnings(subResults, 'render-log');
                    }
                    phaseLatestMs = _pickLatestTimestampMs(
                        subcheck.latest_done_ms,
                        subcheck.latest_update_ms,
                        subcheck.latest_nonterminal_ms,
                        phaseLatestMs
                    );
                    if (subcheck.latest_update_ms) workerFreshMs = Date.now() - subcheck.latest_update_ms;
                } catch (e) {
                    expected = phaseInfo.expected || 0;
                }
            }
        }

        if (_palettePhaseTracker && _palettePhaseTracker.phase !== (rd.phase || phase)) {
            await _logPalettePhaseCompletion(run, _palettePhaseTracker, mirrorToRender);
        }
        _updatePalettePhaseTracker(run, rd);
        const phaseWallMs = _palettePhaseTracker
            ? (_serverElapsedMs(
                _palettePhaseTracker.started_at_ms,
                _pickLatestTimestampMs(
                    phaseLatestMs,
                    _palettePhaseTracker.last_server_update_ms
                )
            ) || 0)
            : 0;
        const phasePerfText = _palettePhasePerfSummary(rd.phase || phase, subResults, phaseWallMs);

        let freshMs = workerFreshMs;
        if (freshMs === null && check.latest_update_ms) {
            freshMs = Date.now() - check.latest_update_ms;
        }

        let statusMsg = phase;
        if (expected > 0) statusMsg += ' ' + done + '/' + expected;
        if (phasePerfText) statusMsg += ' · ' + phasePerfText;
        if (freshMs !== null && freshMs > 10000) {
            statusMsg += ' · last update ' + _fmtAge(freshMs) + ' ago';
        }

        if (freshMs !== null && freshMs > PALETTE_HARD_STALE_MS) {
            statusEl.textContent = runLabel + ' stalled (no worker update for 15+ min)';
            statusEl.className = 'status error';
            if (_lastPaletteWarnState !== 'hard') {
                _lastPaletteWarnState = 'hard';
                log(runLabel + ' stalled — no worker update for 15+ min', 'err', 'palette-log');
                if (mirrorToRender) log(runLabel + ' stalled — no worker update for 15+ min', 'err', 'render-log');
            }
        } else if (freshMs !== null && freshMs > PALETTE_WARN_STALE_MS) {
            statusEl.textContent = statusMsg + ' · no worker update for 5+ min';
            statusEl.className = 'status';
            if (_lastPaletteWarnState !== 'warn') {
                _lastPaletteWarnState = 'warn';
                log('Warning: ' + runLabel.toLowerCase() + ' workers idle for 5+ min', '', 'palette-log');
                if (mirrorToRender) log('Warning: ' + runLabel.toLowerCase() + ' workers idle for 5+ min', '', 'render-log');
            }
        } else {
            statusEl.textContent = statusMsg;
            statusEl.className = 'status';
            if (_lastPaletteWarnState) {
                log(runLabel + ' workers active again', 'ok', 'palette-log');
                if (mirrorToRender) log(runLabel + ' workers active again', 'ok', 'render-log');
                _lastPaletteWarnState = null;
            }
        }

        const phaseKey = (rd.phase || '') + '_' + done;
        if (phaseKey !== _lastPaletteLoggedPhase) {
            _lastPaletteLoggedPhase = phaseKey;
            let logMsg = phase;
            if (expected > 0) logMsg += ' ' + done + '/' + expected;
            if (phasePerfText) logMsg += ' · ' + phasePerfText;
            log(logMsg, '', 'palette-log');
            if (mirrorToRender) log(logMsg, '', 'render-log');
        }
    } catch (e) {
        // keep polling
    }
}


// ── Render Orchestrator Launchers ────────────────────────────────────────
// Browser dispatches one orchestrator job, then observes via DDB polling.
// The backend orchestrator owns all phase transitions.

let _activeRenderRun = null;
let _renderObserverTimer = null;
let _lastLoggedPhase = null;
let _lastWarnState = null;  // null, 'warn', 'hard'
let _renderPhaseTracker = null;
let _renderCompletionRunId = null;
let _renderLoggedPhaseCompletions = new Set();
let _renderObserverPollActive = false;
const RENDER_NO_ROW_STALE_MS = 120000;
const RENDER_WARN_STALE_MS = 300000;
const RENDER_HARD_STALE_MS = 900000;

function _generateRunId() {
    const ms = Date.now();
    const rand = Math.random().toString(36).slice(2, 8);
    return 'run_' + ms + '_' + rand;
}

function _saveActiveRun(record) {
    if (!_activeRenderRun || _activeRenderRun.run_id !== record.run_id) {
        _renderCompletionRunId = null;
        _renderLoggedPhaseCompletions = new Set();
    }
    _activeRenderRun = record;
    try { localStorage.setItem('polypaint_active_render_run', JSON.stringify(record)); } catch(e) {}
}

function _clearActiveRun() {
    _activeRenderRun = null;
    _lastLoggedPhase = null;
    _lastWarnState = null;
    _renderPhaseTracker = null;
    try { localStorage.removeItem('polypaint_active_render_run'); } catch(e) {}
    try { _updateRenderActionButtons(); } catch(e) {}
}

function _clearRenderState() {
    _clearActiveRun();
    stopActiveRenderObserver();
    _invalidateRenderInventory();
    const statusEl = document.getElementById('render-status');
    if (statusEl) { statusEl.textContent = 'Ready'; statusEl.className = 'status'; }
    log('Render state cleared', '', 'render-log');
}

function _invalidateRenderInventory(jobId = null) {
    _renderLoadedJobId = '';
    _renderNeedsRefresh = !!(jobId || (document.getElementById('render-results-dir')?.value.trim() || ''));
    _renderArtifacts = { color: [], bilevel: [], coeffs: [], palette: [], pdf: [] };
    _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1, pdf: -1 };
    _renderSelectedArtifactKey = { color: '', bilevel: '', coeffs: '', palette: '', pdf: '' };
    window._lastRenderSummary = { families: _renderArtifacts, calc: {} };
    const preview = document.getElementById('render-preview');
    const info = document.getElementById('render-info');
    if (preview && info) {
        renderArtifactPanel(jobId || (document.getElementById('render-results-dir')?.value.trim() || ''), window._lastRenderSummary, { preserveScroll: false });
    }
}

function _setRenderResultsJob(jobId, options = {}) {
    const nextJobId = (jobId || '').trim();
    const input = document.getElementById('render-results-dir');
    const prevJobId = input ? String(input.value || '').trim() : '';
    if (input) input.value = nextJobId;
    if (options.invalidate !== false && prevJobId !== nextJobId) {
        _invalidateRenderInventory(nextJobId);
    }
}

async function _maybeAutoRefreshRenderInventory() {
    const jobId = document.getElementById('render-results-dir')?.value.trim() || '';
    if (!jobId || !(_renderNeedsRefresh || _renderLoadedJobId !== jobId) || _activeRenderRun) return;
    try {
        await refreshRenderArtifacts(jobId);
    } catch (e) {
        // refreshRenderArtifacts already updates render status on failure
    }
}

function _loadActiveRun() {
    try {
        const s = localStorage.getItem('polypaint_active_render_run');
        if (s) return JSON.parse(s);
    } catch(e) {}
    return null;
}

async function _dispatchRenderOrchestrator(mode, orchPayload) {
    const runId = _generateRunId();
    orchPayload.run_id = runId;
    const taskId = 'render_run_' + mode + '_' + runId;
    log('Render: dispatching ' + mode + ' orchestrator (' + runId + ')...', 'ok', 'render-log');
    document.getElementById('render-status').textContent = 'Dispatching orchestrator...';
    document.getElementById('render-status').className = 'status';

    // Dispatch via dispatch Lambda
    const dispResult = await lambdaPost('dispatch', {
        target: 'render_orchestrator',
        jobs: [orchPayload],
        expected_keys: [],
    });
    if ((dispResult.fired || 0) !== 1) throw new Error('Orchestrator dispatch failed');

    // Save active run record AFTER successful dispatch
    const record = {
        job_id: orchPayload.job_id, mode, run_id: runId, task_id: taskId,
        started_at_ms: Date.now(),
        raster_engine: orchPayload.params.raster_engine || 'single',
        raster_mt_threads: orchPayload.params.raster_mt_threads || 1,
    };
    _saveActiveRun(record);

    log('  orchestrator dispatched: ' + runId, 'ok', 'render-log');

    // Start observing
    startActiveRenderObserver();
}

async function _launchNonColorRenderOrchestrator(mode, paramsPatch = null) {
    const p = _renderCommonParams({ requireSolveScore: false });
    const params = {
        pix: p.pix,
        fmt: p.fmt,
        quality: p.quality,
        view_mode: _viewMode,
        quantile: p.quantile,
        shim: p.shim,
        square_extent: p.squareExtent,
        rotation: p.rotation,
        root_transforms: p.rootTransforms.length ? p.rootTransforms : undefined,
        root_program_source_text: p.rootProgramSourceText || undefined,
    };
    if (_viewMode === 'explicit') {
        params.min_re = p.minRe;
        params.max_re = p.maxRe;
        params.min_im = p.minIm;
        params.max_im = p.maxIm;
    }
    const orchPayload = {
        job_id: p.jobId,
        mode: mode,
        params,
    };
    if (paramsPatch && typeof paramsPatch === 'object') Object.assign(orchPayload.params, paramsPatch);
    return _dispatchRenderOrchestrator(mode, orchPayload);
}

async function _launchFusedRenderOrchestrator(paramsPatch = null) {
    const p = _renderCommonParams({ requireSolveScore: true });
    const params = {
        pix: p.pix,
        fmt: p.fmt,
        quality: p.quality,
        view_mode: _viewMode,
        quantile: p.quantile,
        shim: p.shim,
        square_extent: p.squareExtent,
        rotation: p.rotation,
        color_mode: 'solve_score',
        color_interpretation: p.colorInterpretation,
        background_color: p.backgroundColor,
        solve_score_program_source_text: p.solveScoreProgramSourceText,
        solve_score_normalize: !!p.solveScoreNormalize,
        palette: _activeRenderPalette() || 'inferno',
        root_transforms: p.rootTransforms.length ? p.rootTransforms : undefined,
        root_program_source_text: p.rootProgramSourceText || undefined,
        raster_engine: 'mt',
        raster_mt_threads: 4,
        solve_score_threads: 4,
        raster_sectioned_retries: 2,
        raster_section_mode: 'logical_sections_auto',
        finalize_workers: 16,
        save_associated_palette: false,
    };
    if (_viewMode === 'explicit') {
        params.min_re = p.minRe;
        params.max_re = p.maxRe;
        params.min_im = p.minIm;
        params.max_im = p.maxIm;
    }
    const orchPayload = {
        job_id: p.jobId,
        mode: 'color',
        params,
    };
    if (paramsPatch && typeof paramsPatch === 'object') Object.assign(orchPayload.params, paramsPatch);
    return _dispatchRenderOrchestrator('color', orchPayload);
}

async function _launchRenderOrchestrator(mode, paramsPatch = null) {
    if (mode === 'color') {
        return _launchFusedRenderOrchestrator(paramsPatch);
    }
    return _launchNonColorRenderOrchestrator(mode, paramsPatch);
}

async function runRasterPipeline(config = null) {
    const btn = document.getElementById('btn-render-generate')
        || document.getElementById('btn-render-generate-mt');
    if (btn) btn.disabled = true;
    try {
        if (renderColorMode !== 'solve_score') {
            throw new Error('Color render supports Solve score only');
        }
        const saveAssociatedPalette = !!(
            config && typeof config === 'object' && config.saveAssociatedPalette != null
                ? config.saveAssociatedPalette
                : _renderMtPopupState.saveAssociatedPalette
        );
        log(`Render (fused): dispatching with lores clip threads=4, raster threads=4, raster workers=10, raster sections=auto, finalize workers=16, associated palette=${saveAssociatedPalette ? 'yes' : 'no'}${saveAssociatedPalette ? ', artifact output' : ''}...`, 'ok', 'render-log');
        await _launchFusedRenderOrchestrator({
            raster_engine: 'mt',
            raster_mt_threads: 4,
            solve_score_threads: 4,
            raster_workers: 10,
            raster_section_mode: 'logical_sections_auto',
            raster_section_count: '',
            raster_sectioned_retries: 2,
            finalize_workers: 16,
            save_associated_palette: saveAssociatedPalette,
        });
    } catch (e) {
        document.getElementById('render-status').textContent = 'Render error: ' + e.message;
        document.getElementById('render-status').className = 'status error';
        log('Render failed: ' + e.message, 'err', 'render-log');
    } finally {
        if (btn) btn.disabled = false;
    }
}

async function runFusedRasterPipelineMT(threadConfig = null) {
    const btn = document.getElementById('btn-render-generate-mt');
    if (btn) btn.disabled = true;
    try {
        if (renderColorMode !== 'solve_score') {
            throw new Error('ColorRender-MT supports Solve score renders only');
        }
        const rasterThreads = _clampRenderMtThreads(
            threadConfig && typeof threadConfig === 'object' && threadConfig.rasterThreads != null
                ? threadConfig.rasterThreads
                : _renderMtPopupState.rasterThreads
        );
        const rasterWorkers = _clampRenderMtWorkerCount(
            threadConfig && typeof threadConfig === 'object' && threadConfig.rasterWorkers != null
                ? threadConfig.rasterWorkers
                : _renderMtPopupState.rasterWorkers
        );
        const clipThreads = _clampRenderMtThreads(
            threadConfig && typeof threadConfig === 'object' && threadConfig.solveScoreThreads != null
                ? threadConfig.solveScoreThreads
                : _renderMtPopupState.solveScoreThreads
        );
        const rasterRetries = _clampRenderMtRetries(
            threadConfig && typeof threadConfig === 'object' && threadConfig.rasterRetries != null
                ? threadConfig.rasterRetries
                : _renderMtPopupState.rasterRetries
        );
        const rasterSectionModeInput = _normalizeRenderMtSectionMode(
            threadConfig && typeof threadConfig === 'object' && threadConfig.rasterSectionMode != null
                ? threadConfig.rasterSectionMode
                : _renderMtPopupState.rasterSectionMode
        );
        const finalizeWorkers = _clampRenderMtFinalizeWorkers(
            threadConfig && typeof threadConfig === 'object' && threadConfig.finalizeWorkers != null
                ? threadConfig.finalizeWorkers
                : _renderMtPopupState.finalizeWorkers
        );
        const saveAssociatedPalette = !!(
            threadConfig && typeof threadConfig === 'object' && threadConfig.saveAssociatedPalette != null
                ? threadConfig.saveAssociatedPalette
                : _renderMtPopupState.saveAssociatedPalette
        );
        const rasterEstimate = _renderMtSectionEstimate('raster', rasterThreads);
        const rasterSectionMode = rasterSectionModeInput === 'logical_sections' ? 'logical_sections' : 'logical_sections_auto';
        let rasterSectionCount = _clampRenderMtSectionCount(
            threadConfig && typeof threadConfig === 'object' && threadConfig.rasterSectionCount != null
                ? threadConfig.rasterSectionCount
                : _renderMtPopupState.rasterSectionCount
        );
        if (rasterSectionMode === 'logical_sections_auto') {
            rasterSectionCount = rasterEstimate.available ? rasterEstimate.minSafeSections : '';
        } else if (rasterEstimate.available && rasterSectionCount !== '' && Number(rasterSectionCount) < Number(rasterEstimate.minSafeSections)) {
            rasterSectionCount = rasterEstimate.minSafeSections;
        }

        log(
            `ColorRender-MT (fused): dispatching with lores clip threads=${clipThreads}, raster input=sectioned, raster retries=${rasterRetries}, raster threads=${rasterThreads}, raster workers=${rasterWorkers}, raster sections=${_renderMtSectionSettingLabel(rasterSectionMode, rasterSectionCount)}, bin fragments=raw score bins, finalize workers=${finalizeWorkers}, associated palette=${saveAssociatedPalette ? 'yes' : 'no'}${saveAssociatedPalette ? ', artifact output' : ''}...`,
            'ok',
            'render-log'
        );

        const launchParams = {
            raster_engine: 'mt',
            raster_mt_threads: rasterThreads,
            raster_workers: rasterWorkers,
            solve_score_threads: clipThreads,
            raster_sectioned_retries: rasterRetries,
            raster_section_mode: rasterSectionMode,
            raster_section_count: rasterSectionCount,
            finalize_workers: finalizeWorkers,
            save_associated_palette: saveAssociatedPalette,
        };
        await _launchFusedRenderOrchestrator(launchParams);
    } catch (e) {
        document.getElementById('render-status').textContent = 'ColorRender-MT error: ' + e.message;
        document.getElementById('render-status').className = 'status error';
        log('ColorRender-MT failed: ' + e.message, 'err', 'render-log');
    } finally {
        if (btn) btn.disabled = false;
    }
}

async function runRasterPipelineMT(threadConfig = null) {
    return runFusedRasterPipelineMT(threadConfig);
}

function _closeBilevelPopup() {
    _bilevelPopupState = {
        open: false,
        sectionMode: _normalizeRenderMtSectionMode(_bilevelPopupState.sectionMode),
        sectionCount: _clampRenderMtSectionCount(_bilevelPopupState.sectionCount),
    };
    const overlay = document.getElementById('bilevel-popup-overlay');
    if (overlay) {
        overlay.style.display = 'none';
        overlay.setAttribute('aria-hidden', 'true');
    }
}

function _syncBilevelPopupControls() {
    const modeEl = document.getElementById('bilevel-section-mode');
    const countEl = document.getElementById('bilevel-section-count');
    if (!modeEl || !countEl) return;
    const mode = _normalizeRenderMtSectionMode(modeEl.value);
    countEl.disabled = mode !== 'logical_sections';
    if (mode !== 'logical_sections') {
        countEl.value = '';
        _bilevelPopupState.sectionCount = '';
    }
}

function _renderBilevelPopup() {
    const overlay = document.getElementById('bilevel-popup-overlay');
    const summaryEl = document.getElementById('bilevel-popup-summary');
    const modeEl = document.getElementById('bilevel-section-mode');
    const countEl = document.getElementById('bilevel-section-count');
    if (!overlay || !summaryEl || !modeEl || !countEl || !_bilevelPopupState.open) {
        _closeBilevelPopup();
        return;
    }
    const pixEl = document.getElementById('render-pix');
    const pix = Number(pixEl && pixEl.value);
    const sectionMode = _normalizeRenderMtSectionMode(_bilevelPopupState.sectionMode || 'logical_sections_auto');
    const sectionCount = sectionMode === 'logical_sections'
        ? _clampRenderMtSectionCount(_bilevelPopupState.sectionCount)
        : '';
    overlay.style.display = 'flex';
    overlay.setAttribute('aria-hidden', 'false');
    modeEl.value = sectionMode === 'physical_chunks' ? 'logical_sections_auto' : sectionMode;
    countEl.value = sectionCount;
    _syncBilevelPopupControls();
    summaryEl.textContent = `Output=${Number.isFinite(pix) && pix > 0 ? `${Math.round(pix)}x${Math.round(pix)}` : '?'} • sections=${_renderMtSectionSettingLabel(modeEl.value, sectionCount || '')} • pipeline=logical sections -> sparse fragments -> single finalize`;
}

function openBilevelPopup() {
    const sectionMode = _normalizeRenderMtSectionMode(_bilevelPopupState.sectionMode || 'logical_sections_auto');
    _bilevelPopupState = {
        open: true,
        sectionMode,
        sectionCount: sectionMode === 'logical_sections'
            ? _clampRenderMtSectionCount(_bilevelPopupState.sectionCount)
            : '',
    };
    _renderBilevelPopup();
    const modeEl = document.getElementById('bilevel-section-mode');
    if (modeEl && typeof modeEl.focus === 'function') modeEl.focus();
}

;(window.__ppParts = window.__ppParts || []).push('09-render-orchestration');
