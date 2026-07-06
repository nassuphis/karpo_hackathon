// PolyPaint 03-program-modals — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Deploy rewrites the script tags to build-versioned asset
// keys (assets/<BUILD_ID>/...), so a deploy flips atomically via index.html.
let _solveScoreProgramRememberedNames = {
    render: '',
    palette: '',
};

function _solveScoreProgramDefaultName(prefix) {
    const remembered = String(_solveScoreProgramRememberedNames[prefix] || '').trim();
    if (remembered) return remembered;
    try {
        const state = _resolveSolveScoreState(prefix, { requireChain: true });
        const basis = String(state.display || state.metric || 'solve-score-program').trim().toLowerCase();
        return basis
            .replace(/[^a-z0-9]+/g, '-')
            .replace(/^-+|-+$/g, '')
            .slice(0, 48) || 'solve-score-program';
    } catch (e) {
        return 'solve-score-program';
    }
}

function _solveScoreProgramStatusId(prefix) {
    return `${prefix}-solve-score-program-status`;
}

function _setSolveScoreProgramStatus(prefix, message, isError = false) {
    const el = document.getElementById(_solveScoreProgramStatusId(prefix));
    if (!el) return;
    el.textContent = String(message || '');
    el.className = `solve-score-program-status${isError ? ' error' : ''}`;
}

function _parseSolveScoreProgram(raw) {
    if (!raw || typeof raw !== 'object') throw new Error('program JSON must be an object');
    const chain = Array.isArray(raw.chain) ? raw.chain : null;
    if (!chain || !chain.length) throw new Error('program JSON is missing a non-empty chain');
    const hasVersion = Object.prototype.hasOwnProperty.call(raw, 'version');
    const parsedVersion = Number(raw.version);
    if (hasVersion && !Number.isFinite(parsedVersion)) {
        throw new Error('program JSON version must be numeric when present');
    }
    const version = hasVersion ? parsedVersion : 1;
    if (version !== 1 && version !== 2) {
        throw new Error(`program JSON version ${version} is not supported`);
    }
    return {
        version,
        name: String(raw.name || '').trim(),
        description: String(raw.description || '').trim(),
        source_text: String(raw.source_text || raw.solve_score_program_source_text || '').trim(),
        chain: _serializeSolveScoreChain(chain),
        recommended_interpretation: raw.recommended_interpretation ? _normalizeColorInterpretation(raw.recommended_interpretation) : '',
    };
}

async function _solveScoreSourceTextFromChainRoute(chain) {
    const resp = await lambdaPost('storage', { chain }, '/solve-score-chain-to-source');
    if (!resp || resp.ok === false) {
        throw new Error((resp && (resp.error || resp.message)) || 'solve-score chain could not be converted to source');
    }
    const text = String(resp.source_text || '');
    if (!text.trim()) throw new Error('solve-score chain-to-source returned empty source');
    return text;
}

async function _applySolveScoreProgram(prefix, rawProgram) {
    const program = _parseSolveScoreProgram(rawProgram);
    const which = _solveScoreWhichForPrefix(prefix);
    const chain = _chainForWhich(which);
    const fallbackMetric = prefix === 'palette' ? paletteTabMetric : renderSolveMetric;
    _compileSolveScoreChain(program.chain, fallbackMetric);
    chain.splice(0, chain.length, ..._normalizeSolveScoreChain(program.chain, fallbackMetric));
    _syncSolveScoreUi(which);
    let sourceWarning = '';
    if (program.source_text) {
        _setSolveScoreProgramSourceText(prefix, program.source_text);
        _setSolveScoreProgramEditorMode(prefix, 'text');
    } else {
        try {
            const sourceText = await _solveScoreSourceTextFromChainRoute(program.chain);
            _setSolveScoreProgramSourceText(prefix, sourceText);
            _setSolveScoreProgramEditorMode(prefix, 'text');
        } catch (e) {
            _setSolveScoreProgramSourceText(prefix, '');
            _setSolveScoreProgramEditorMode(prefix, 'text');
            sourceWarning = `Loaded chain, but source reconstruction failed: ${e && e.message ? e.message : String(e)}`;
        }
    }
    const compiled = _syncSolveScoreLegacyInputs(prefix);
    if (prefix === 'render') {
        setColorMode('solve_score');
        if (program.recommended_interpretation) _setRenderColorInterpretation(program.recommended_interpretation);
    }
    _setSolveScoreProgramStatus(
        prefix,
        sourceWarning || (program.name ? `Loaded ${program.name}` : 'Loaded solve-score program'),
        !!sourceWarning
    );
    return compiled;
}

function _readTextFile(file) {
    if (!file) return Promise.reject(new Error('file missing'));
    if (typeof file.text === 'function') return file.text();
    return new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onload = () => resolve(String(reader.result || ''));
        reader.onerror = () => reject(new Error('failed to read file'));
        reader.readAsText(file);
    });
}

function _solveScoreProgramFilename(name) {
    const slug = String(name || '').trim().toLowerCase()
        .replace(/[^a-z0-9]+/g, '-')
        .replace(/^-+|-+$/g, '') || 'solve-score-program';
    return `${slug}.json`;
}

function _portableSolveScoreProgramPayload(prefix, nameOverride = '') {
    const state = _resolveSolveScoreState(prefix, { requireChain: true });
    const name = String(nameOverride || _solveScoreProgramDefaultName(prefix)).trim() || 'solve-score-program';
    return {
        version: 1,
        name,
        source_text: _effectiveSolveScoreProgramSourceText(prefix),
        source_text_authoritative: !!_effectiveSolveScoreProgramSourceText(prefix).trim(),
        chain: state.chain,
        recommended_interpretation: prefix === 'render' ? _selectedRenderColorInterpretation() : '',
        metric: state.metric,
        display: state.display,
        program_spec: state.program_spec,
    };
}

function _currentSolveScoreProgramSummary(prefix) {
    try {
        const payload = _portableSolveScoreProgramPayload(prefix, _solveScoreModalState.nameInput || _solveScoreProgramDefaultName(prefix));
        return {
            ...payload,
            statement_count: Array.isArray(payload.chain) ? payload.chain.length : 0,
        };
    } catch (e) {
        return { error: e && e.message ? e.message : String(e) };
    }
}

function _solveScoreModalMessageHtml(message, isError = false) {
    const cls = isError ? 'solve-score-program-status error' : 'solve-score-modal-selected-empty';
    return `<div class="${cls}">${_escapeHtml(message || '')}</div>`;
}

function _solveScoreProgramMetaHtml(program, options = {}) {
    const rows = [];
    const name = String(program && program.name || '').trim();
    const savedAt = String(program && program.saved_at || '').trim();
    const color = String(program && program.recommended_interpretation || '').trim();
    const statements = Number(
        program && program.statement_count != null
            ? program.statement_count
            : (Array.isArray(program && program.chain) ? program.chain.length : NaN)
    );
    if (name) rows.push(['Name', name]);
    if (options.selected && savedAt) rows.push(['Saved', savedAt]);
    if (color) rows.push(['Color', _colorInterpretationLabel(color)]);
    if (Number.isFinite(statements)) rows.push(['Statements', String(statements)]);
    if (!rows.length) return '';
    return `<div class="solve-score-modal-program-meta">${rows.map(([label, value]) => (
        `<div><span>${_escapeHtml(label)}</span>${_escapeHtml(value)}</div>`
    )).join('')}</div>`;
}

function _renderSolveScoreProgramChipsHtml(program, options = {}) {
    const fallbackMetric = options.prefix === 'palette' ? paletteTabMetric : renderSolveMetric;
    const normalized = _normalizeSolveScoreChain(program && program.chain, fallbackMetric || 'proximity');
    if (!normalized.length) return _solveScoreModalMessageHtml('No solve-score chips.');
    const renderOptions = { readonly: true, solveScore: true, chain: normalized };
    const chips = normalized.map((item, i) => _renderScoreChipHtml(item, i, 'modal-ss', _ssCatalog, renderOptions)).join('');
    return `<div class="chip-container solve-score-modal-chip-strip" aria-label="Solve score program chips">${chips}</div>`;
}

function _renderSolveScoreProgramCardHtml(program, options = {}) {
    const selected = !!options.selected;
    if (!program) {
        return _solveScoreModalMessageHtml(
            selected ? 'Select a saved program to inspect before loading.' : 'Current live program is unavailable.'
        );
    }
    if (program.error) return _solveScoreModalMessageHtml(`Error: ${program.error}`, true);
    try {
        return _solveScoreProgramMetaHtml(program, options) + _renderSolveScoreProgramChipsHtml(program, options);
    } catch (e) {
        return _solveScoreModalMessageHtml(`Error: ${e && e.message ? e.message : String(e)}`, true);
    }
}

function _setSolveScoreModalStatus(message, isError = false) {
    _solveScoreModalState.status = String(message || '');
    _solveScoreModalState.statusError = !!isError;
}

function _renderSolveScoreProgramModal() {
    const overlay = document.getElementById('solve-score-modal-overlay');
    const summaryEl = document.getElementById('solve-score-modal-summary');
    const bodyEl = document.getElementById('solve-score-modal-body');
    const nameEl = document.getElementById('solve-score-modal-name');
    const currentEl = document.getElementById('solve-score-modal-current');
    const selectedEl = document.getElementById('solve-score-modal-selected');
    const statusEl = document.getElementById('solve-score-modal-status');
    const loadBtn = document.getElementById('solve-score-modal-load');
    const saveBtn = document.getElementById('solve-score-modal-save');
    const deleteBtn = document.getElementById('solve-score-modal-delete');
    const migrateBtn = document.getElementById('solve-score-modal-migrate');
    const downloadBtn = document.getElementById('solve-score-modal-download');
    const uploadBtn = document.getElementById('solve-score-modal-upload');
    const currentProgram = _currentSolveScoreProgramSummary(_solveScoreModalState.prefix);
    const canLoad = _solveScoreModalState.tableState === 'loaded' &&
        !!_solveScoreModalState.selectedId &&
        !_solveScoreModalState.selectedLoading &&
        !_solveScoreModalState.actionBusy;
    const canDelete = canLoad;
    const canSave = !_solveScoreModalState.actionBusy &&
        !!String(_solveScoreModalState.nameInput || '').trim() &&
        !currentProgram.error;
    const canDownload = !_solveScoreModalState.actionBusy && !currentProgram.error;
    const canUpload = !_solveScoreModalState.actionBusy;
    if (overlay) {
        overlay.style.display = _solveScoreModalState.open ? 'flex' : 'none';
        overlay.setAttribute('aria-hidden', _solveScoreModalState.open ? 'false' : 'true');
    }
    if (summaryEl) {
        summaryEl.textContent = _solveScoreModalState.prefix === 'palette'
            ? 'Global saved solve-score programs on the left; current Palette-tab live chain on the right.'
            : 'Global saved solve-score programs on the left; current Render-tab live chain on the right.';
    }
    if (nameEl && nameEl.value !== _solveScoreModalState.nameInput) nameEl.value = _solveScoreModalState.nameInput;
    if (currentEl) currentEl.innerHTML = _renderSolveScoreProgramCardHtml(currentProgram, { prefix: _solveScoreModalState.prefix });
    if (selectedEl) {
        if (_solveScoreModalState.selectedLoading) {
            selectedEl.innerHTML = _solveScoreModalMessageHtml('Loading selected saved program...');
            selectedEl.classList.remove('solve-score-modal-selected-empty');
        } else if (_solveScoreModalState.selectedError) {
            selectedEl.innerHTML = _solveScoreModalMessageHtml(`Error: ${_solveScoreModalState.selectedError}`, true);
            selectedEl.classList.remove('solve-score-modal-selected-empty');
        } else if (_solveScoreModalState.selectedProgram) {
            selectedEl.innerHTML = _renderSolveScoreProgramCardHtml(_solveScoreModalState.selectedProgram, { prefix: _solveScoreModalState.prefix, selected: true });
            selectedEl.classList.remove('solve-score-modal-selected-empty');
        } else {
            selectedEl.innerHTML = _solveScoreModalMessageHtml('Select a saved program to inspect before loading.');
            selectedEl.classList.add('solve-score-modal-selected-empty');
        }
    }
    if (statusEl) {
        statusEl.textContent = _solveScoreModalState.status;
        statusEl.className = `solve-score-program-status${_solveScoreModalState.statusError ? ' error' : ''}`;
    }
    if (nameEl) nameEl.disabled = _solveScoreModalState.actionBusy;
    if (loadBtn) loadBtn.disabled = !canLoad;
    if (saveBtn) saveBtn.disabled = !canSave;
    if (deleteBtn) deleteBtn.disabled = !canDelete;
    if (migrateBtn) migrateBtn.disabled = !canLoad;
    if (downloadBtn) downloadBtn.disabled = !canDownload;
    if (uploadBtn) uploadBtn.disabled = !canUpload;
    if (!bodyEl) return;
    const rows = Array.isArray(_solveScoreModalState.rows) ? _solveScoreModalState.rows : [];
    if (_solveScoreModalState.tableState === 'loading') {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">Loading saved programs...</td></tr>';
        return;
    }
    if (_solveScoreModalState.tableState === 'error') {
        bodyEl.innerHTML = `<tr class="tri-popup-empty"><td colspan="3">${_escapeHtml(_solveScoreModalState.status || 'Failed to load saved programs.')}</td></tr>`;
        return;
    }
    const ssVisible = _programModalVisibleRows(_solveScoreModalState);
    _programModalSortArrows(_solveScoreModalState, 'solve-score-modal');
    if (!rows.length) {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">No saved solve-score programs found.</td></tr>';
        return;
    }
    if (!ssVisible.length) {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">No programs match the filter.</td></tr>';
        return;
    }
    bodyEl.innerHTML = ssVisible.map((row) => {
        const active = row.id === _solveScoreModalState.selectedId ? ' active' : '';
        return `
            <tr class="tri-popup-row${active}" data-solve-score-program-id="${_escapeHtml(row.id)}">
                <td><div class="tri-popup-name"><div>${_escapeHtml(row.name)}</div></div></td>
                <td>${Number(row.statement_count || 0)}</td>
                <td>${_escapeHtml(row.saved_at || '')}</td>
            </tr>
        `;
    }).join('');
    Array.from(bodyEl.querySelectorAll('[data-solve-score-program-id]')).forEach((rowEl) => {
        rowEl.addEventListener('click', () => {
            const id = rowEl.getAttribute('data-solve-score-program-id') || '';
            if (id) void _selectSolveScoreProgram(id);
        });
    });
}

async function _fetchSolveScoreProgramById(id) {
    const resp = await lambdaPost('storage', { id }, '/fetch-solve-score-program');
    return resp.program;
}

async function _refreshSolveScoreProgramRows(options = {}) {
    const preserveSelection = options.preserveSelection !== false;
    const keepStatus = !!options.keepStatus;
    const selectedId = preserveSelection ? _solveScoreModalState.selectedId : '';
    _solveScoreModalState.tableState = 'loading';
    if (!keepStatus) _setSolveScoreModalStatus('', false);
    _renderSolveScoreProgramModal();
    try {
        const resp = await lambdaPost('storage', {}, '/list-solve-score-programs');
        const rows = Array.isArray(resp.programs) ? resp.programs : [];
        _solveScoreModalState.rows = rows;
        _solveScoreModalState.tableState = 'loaded';
        if (!rows.length) {
            _solveScoreModalState.selectedId = '';
            _solveScoreModalState.selectedProgram = null;
            _solveScoreModalState.selectedError = '';
            _solveScoreModalState.selectedLoading = false;
        } else if (selectedId && rows.some((row) => row.id === selectedId)) {
            _solveScoreModalState.selectedId = selectedId;
            if (_solveScoreModalState.selectedProgram && _solveScoreModalState.selectedProgram.id !== selectedId) {
                _solveScoreModalState.selectedProgram = null;
            }
        } else {
            _solveScoreModalState.selectedId = '';
            _solveScoreModalState.selectedProgram = null;
            _solveScoreModalState.selectedError = '';
            _solveScoreModalState.selectedLoading = false;
        }
    } catch (e) {
        _solveScoreModalState.rows = [];
        _solveScoreModalState.tableState = 'error';
        _solveScoreModalState.selectedId = '';
        _solveScoreModalState.selectedProgram = null;
        _solveScoreModalState.selectedError = '';
        _solveScoreModalState.selectedLoading = false;
        _setSolveScoreModalStatus(e && e.message ? e.message : String(e), true);
    }
    _renderSolveScoreProgramModal();
}

async function _selectSolveScoreProgram(id) {
    const row = (_solveScoreModalState.rows || []).find((entry) => entry.id === id);
    if (!row) return;
    const currentName = String(_solveScoreModalState.nameInput || '').trim();
    if (!currentName || currentName === _solveScoreModalState.lastSelectedName) {
        _solveScoreModalState.nameInput = row.name;
    }
    _solveScoreModalState.lastSelectedName = row.name;
    _solveScoreModalState.selectedId = id;
    if (_solveScoreModalState.selectedProgram && _solveScoreModalState.selectedProgram.id === id) {
        _solveScoreModalState.selectedError = '';
        _solveScoreModalState.selectedLoading = false;
        _renderSolveScoreProgramModal();
        return;
    }
    _solveScoreModalState.selectedProgram = null;
    _solveScoreModalState.selectedError = '';
    _solveScoreModalState.selectedLoading = true;
    _renderSolveScoreProgramModal();
    try {
        const program = await _fetchSolveScoreProgramById(id);
        if (_solveScoreModalState.selectedId !== id) return;
        _solveScoreModalState.selectedProgram = program;
        _solveScoreModalState.selectedError = '';
        _solveScoreModalState.selectedLoading = false;
    } catch (e) {
        if (_solveScoreModalState.selectedId !== id) return;
        _solveScoreModalState.selectedProgram = null;
        _solveScoreModalState.selectedError = e && e.message ? e.message : String(e);
        _solveScoreModalState.selectedLoading = false;
    }
    _renderSolveScoreProgramModal();
}

function openSolveScoreProgramModal(prefix) {
    _solveScoreModalState.open = true;
    _solveScoreModalState.prefix = prefix === 'palette' ? 'palette' : 'render';
    _solveScoreModalState.actionBusy = false;
    _solveScoreModalState.nameInput = _solveScoreProgramDefaultName(_solveScoreModalState.prefix);
    _solveScoreModalState.lastSelectedName = '';
    _setSolveScoreModalStatus('', false);
    _renderSolveScoreProgramModal();
    if (_solveScoreModalState.tableState !== 'loaded') {
        void _refreshSolveScoreProgramRows({ preserveSelection: true });
    }
    const nameEl = document.getElementById('solve-score-modal-name');
    if (nameEl && typeof nameEl.focus === 'function') nameEl.focus();
}

function _closeSolveScoreProgramModal() {
    _solveScoreModalState.open = false;
    _renderSolveScoreProgramModal();
}

async function _loadSelectedSolveScoreProgramFromModal() {
    if (!_solveScoreModalState.selectedId) return;
    _solveScoreModalState.actionBusy = true;
    _setSolveScoreModalStatus('', false);
    _renderSolveScoreProgramModal();
    try {
        let program = _solveScoreModalState.selectedProgram;
        if (!program || program.id !== _solveScoreModalState.selectedId) {
            program = await _fetchSolveScoreProgramById(_solveScoreModalState.selectedId);
            _solveScoreModalState.selectedProgram = program;
        }
        await _applySolveScoreProgram(_solveScoreModalState.prefix, program);
        if (program && program.name) {
            _solveScoreModalState.nameInput = program.name;
            _solveScoreModalState.lastSelectedName = program.name;
            _solveScoreProgramRememberedNames[_solveScoreModalState.prefix] = program.name;
        }
        _setSolveScoreModalStatus(program.name ? `Loaded ${program.name}` : 'Loaded solve-score program', false);
    } catch (e) {
        _setSolveScoreModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _solveScoreModalState.actionBusy = false;
        _renderSolveScoreProgramModal();
    }
}

async function _saveCurrentSolveScoreProgramFromModal() {
    const name = String(_solveScoreModalState.nameInput || '').trim();
    if (!name) return;
    let payload;
    try {
        payload = _portableSolveScoreProgramPayload(_solveScoreModalState.prefix, name);
    } catch (e) {
        _setSolveScoreModalStatus(e && e.message ? e.message : String(e), true);
        _renderSolveScoreProgramModal();
        return;
    }
    _solveScoreModalState.actionBusy = true;
    _setSolveScoreModalStatus('', false);
    _renderSolveScoreProgramModal();
    try {
        const resp = await lambdaPost('storage', {
            name: payload.name,
            chain: payload.chain,
            source_text: payload.source_text || undefined,
            recommended_interpretation: payload.recommended_interpretation || undefined,
        }, '/save-solve-score-program');
        const program = resp && resp.program ? resp.program : null;
        if (!program) throw new Error('save-solve-score-program returned no program');
        _solveScoreModalState.selectedId = program.id || '';
        _solveScoreModalState.selectedProgram = program;
        _solveScoreModalState.selectedError = '';
        _solveScoreModalState.selectedLoading = false;
        _solveScoreModalState.nameInput = program.name || payload.name;
        _solveScoreModalState.lastSelectedName = program.name || payload.name;
        _solveScoreProgramRememberedNames[_solveScoreModalState.prefix] = _solveScoreModalState.nameInput;
        const message = resp.overwritten
            ? `Saved ${program.name} (overwrote existing entry)`
            : `Saved ${program.name}`;
        _setSolveScoreModalStatus(message, false);
        _setSolveScoreProgramStatus(_solveScoreModalState.prefix, message, false);
        await _refreshSolveScoreProgramRows({ preserveSelection: true, keepStatus: true });
    } catch (e) {
        _setSolveScoreModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _solveScoreModalState.actionBusy = false;
        _renderSolveScoreProgramModal();
    }
}

async function _deleteSelectedSolveScoreProgramFromModal() {
    const id = String(_solveScoreModalState.selectedId || '').trim();
    if (!id) return;
    if (typeof confirm === 'function' && !confirm(`Delete solve-score program "${id}"?`)) return;
    _solveScoreModalState.actionBusy = true;
    _setSolveScoreModalStatus('', false);
    _renderSolveScoreProgramModal();
    try {
        await lambdaPost('storage', { id }, '/delete-solve-score-program');
        _solveScoreModalState.selectedId = '';
        _solveScoreModalState.selectedProgram = null;
        _solveScoreModalState.selectedError = '';
        _solveScoreModalState.selectedLoading = false;
        const message = `Deleted ${id}`;
        _setSolveScoreModalStatus(message, false);
        _setSolveScoreProgramStatus(_solveScoreModalState.prefix, message, false);
        await _refreshSolveScoreProgramRows({ preserveSelection: false, keepStatus: true });
    } catch (e) {
        _setSolveScoreModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _solveScoreModalState.actionBusy = false;
        _renderSolveScoreProgramModal();
    }
}

async function _migrateSelectedProgramFromModal(kind, state, setStatus, renderFn, refreshFn, liveStatusFn) {
    const id = String(state.selectedId || '').trim();
    if (!id) return;
    const route = {
        'solve-score': '/migrate-solve-score-program',
        param: '/migrate-param-program',
        coeff: '/migrate-coeff-program',
    }[kind];
    if (!route) return;
    state.actionBusy = true;
    setStatus('', false);
    renderFn();
    try {
        const resp = await lambdaPost('storage', { id, dry_run: false }, route);
        const migrated = resp && resp.migrated ? resp.migrated : {};
        const version = Number(migrated.spec_version || migrated.program_version || 2);
        const message = resp && resp.wrote
            ? `Migrated ${id} to v${version}`
            : `v${version} copy for ${id} is already current`;
        setStatus(message, false);
        if (typeof liveStatusFn === 'function') liveStatusFn(message, false);
        await refreshFn({ preserveSelection: true, keepStatus: true });
    } catch (e) {
        setStatus(e && e.message ? e.message : String(e), true);
    } finally {
        state.actionBusy = false;
        renderFn();
    }
}

async function _migrateSelectedSolveScoreProgramFromModal() {
    await _migrateSelectedProgramFromModal(
        'solve-score',
        _solveScoreModalState,
        _setSolveScoreModalStatus,
        _renderSolveScoreProgramModal,
        _refreshSolveScoreProgramRows,
        (message, isError) => _setSolveScoreProgramStatus(_solveScoreModalState.prefix, message, isError)
    );
}

function _downloadCurrentSolveScoreProgramFromModal() {
    try {
        const payload = _portableSolveScoreProgramPayload(
            _solveScoreModalState.prefix,
            String(_solveScoreModalState.nameInput || '').trim() || _solveScoreProgramDefaultName(_solveScoreModalState.prefix)
        );
        const filename = _solveScoreProgramFilename(payload.name);
        const blob = new Blob([`${JSON.stringify(payload, null, 2)}\n`], { type: 'application/json' });
        _downloadBlob(blob, filename);
        const message = `Downloaded ${filename}`;
        _setSolveScoreModalStatus(message, false);
        _setSolveScoreProgramStatus(_solveScoreModalState.prefix, message, false);
    } catch (e) {
        _setSolveScoreModalStatus(e && e.message ? e.message : String(e), true);
    }
    _renderSolveScoreProgramModal();
}

function _triggerSolveScoreProgramUploadFromModal() {
    const inputEl = document.getElementById('solve-score-modal-upload-file');
    if (!inputEl || _solveScoreModalState.actionBusy) return;
    inputEl.value = '';
    inputEl.click();
}

async function _importSolveScoreProgramFileFromModal(file) {
    _solveScoreModalState.actionBusy = true;
    _setSolveScoreModalStatus('', false);
    _renderSolveScoreProgramModal();
    try {
        const text = await _readTextFile(file);
        const raw = JSON.parse(text);
        await _applySolveScoreProgram(_solveScoreModalState.prefix, raw);
        const parsed = _parseSolveScoreProgram(raw);
        if (parsed.name) {
            _solveScoreModalState.nameInput = parsed.name;
            _solveScoreModalState.lastSelectedName = parsed.name;
            _solveScoreProgramRememberedNames[_solveScoreModalState.prefix] = parsed.name;
        } else if (!String(_solveScoreModalState.nameInput || '').trim()) {
            _solveScoreModalState.nameInput = _solveScoreProgramDefaultName(_solveScoreModalState.prefix);
        }
        const message = parsed.name ? `Loaded ${parsed.name} from JSON` : 'Loaded solve-score program from JSON';
        _setSolveScoreModalStatus(message, false);
    } catch (e) {
        _setSolveScoreModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _solveScoreModalState.actionBusy = false;
        _renderSolveScoreProgramModal();
    }
}

let _paramProgramModalState = {
    filterText: '',
    sortKey: 'name',
    sortDir: 1,
    open: false,
    tableState: 'idle',
    rows: [],
    selectedId: '',
    selectedProgram: null,
    selectedError: '',
    selectedLoading: false,
    actionBusy: false,
    status: '',
    statusError: false,
    nameInput: '',
    lastSelectedName: '',
};

function _paramProgramFilename(name) {
    const slug = String(name || '').trim().toLowerCase()
        .replace(/[^a-z0-9]+/g, '-')
        .replace(/^-+|-+$/g, '') || 'param-program';
    return `${slug}.json`;
}

function _setParamProgramModalStatus(message, isError = false) {
    _paramProgramModalState.status = String(message || '');
    _paramProgramModalState.statusError = !!isError;
}

function _currentParamProgramSummary() {
    try {
        const payload = _portableParamProgramPayload(_paramProgramModalState.nameInput || _paramProgramDefaultName());
        return {
            ...payload,
            statement_count: payload.source_text
                ? _paramProgramSourceStatementCount(payload.source_text)
                : (Array.isArray(payload.chain) ? payload.chain.length : 0),
        };
    } catch (e) {
        return { error: e && e.message ? e.message : String(e) };
    }
}

function _paramProgramMetaHtml(program, options = {}) {
    const rows = [];
    const name = String(program && program.name || '').trim();
    const savedAt = String(program && program.saved_at || '').trim();
    const statements = Number(
        program && program.statement_count != null
            ? program.statement_count
            : (Array.isArray(program && program.chain) ? program.chain.length : NaN)
    );
    const tokenCount = Number(program && program.token_count);
    if (name) rows.push(['Name', name]);
    if (options.selected && savedAt) rows.push(['Saved', savedAt]);
    if (Number.isFinite(statements)) rows.push(['Statements', String(statements)]);
    if (Number.isFinite(tokenCount)) rows.push(['Tokens', String(tokenCount)]);
    if (program && program.uses_legacy_fast_path === true) rows.push(['Path', 'legacy']);
    if (program && program.uses_legacy_fast_path === false) rows.push(['Path', 'VM']);
    if (!rows.length) return '';
    return `<div class="solve-score-modal-program-meta">${rows.map(([label, value]) => (
        `<div><span>${_escapeHtml(label)}</span>${_escapeHtml(value)}</div>`
    )).join('')}</div>`;
}

function _renderParamProgramCardHtml(program, options = {}) {
    const selected = !!options.selected;
    if (!program) {
        return _solveScoreModalMessageHtml(
            selected ? 'Select a saved program to inspect before loading.' : 'Current live program is unavailable.'
        );
    }
    if (program.error) return _solveScoreModalMessageHtml(`Error: ${program.error}`, true);
    try {
        const sourceText = String(program.source_text || '');
        if (sourceText.trim()) {
            return _paramProgramMetaHtml(program, options)
                + `<pre class="coeff-program-modal-source" aria-label="Param program source">${_escapeHtml(sourceText)}</pre>`;
        }
        return _paramProgramMetaHtml(program, options) + _renderParamProgramChipsHtml(program);
    } catch (e) {
        return _solveScoreModalMessageHtml(`Error: ${e && e.message ? e.message : String(e)}`, true);
    }
}

function _renderParamProgramModal() {
    const overlay = document.getElementById('param-program-modal-overlay');
    const summaryEl = document.getElementById('param-program-modal-summary');
    const bodyEl = document.getElementById('param-program-modal-body');
    const nameEl = document.getElementById('param-program-modal-name');
    const currentEl = document.getElementById('param-program-modal-current');
    const selectedEl = document.getElementById('param-program-modal-selected');
    const statusEl = document.getElementById('param-program-modal-status');
    const loadBtn = document.getElementById('param-program-modal-load');
    const saveBtn = document.getElementById('param-program-modal-save');
    const deleteBtn = document.getElementById('param-program-modal-delete');
    const migrateBtn = document.getElementById('param-program-modal-migrate');
    const downloadBtn = document.getElementById('param-program-modal-download');
    const uploadBtn = document.getElementById('param-program-modal-upload');
    const currentProgram = _currentParamProgramSummary();
    const canLoad = _paramProgramModalState.tableState === 'loaded' &&
        !!_paramProgramModalState.selectedId &&
        !_paramProgramModalState.selectedLoading &&
        !_paramProgramModalState.actionBusy;
    const canDelete = canLoad;
    const canSave = !_paramProgramModalState.actionBusy &&
        !!String(_paramProgramModalState.nameInput || '').trim() &&
        !currentProgram.error;
    const canDownload = !_paramProgramModalState.actionBusy && !currentProgram.error;
    const canUpload = !_paramProgramModalState.actionBusy;
    if (overlay) {
        overlay.style.display = _paramProgramModalState.open ? 'flex' : 'none';
        overlay.setAttribute('aria-hidden', _paramProgramModalState.open ? 'false' : 'true');
    }
    if (summaryEl) {
        summaryEl.textContent = 'Global saved parameter programs on the left; current Compute-tab live text program on the right.';
    }
    if (nameEl && nameEl.value !== _paramProgramModalState.nameInput) nameEl.value = _paramProgramModalState.nameInput;
    if (currentEl) currentEl.innerHTML = _renderParamProgramCardHtml(currentProgram);
    if (selectedEl) {
        if (_paramProgramModalState.selectedLoading) {
            selectedEl.innerHTML = _solveScoreModalMessageHtml('Loading selected saved program...');
        } else if (_paramProgramModalState.selectedError) {
            selectedEl.innerHTML = _solveScoreModalMessageHtml(`Error: ${_paramProgramModalState.selectedError}`, true);
        } else if (_paramProgramModalState.selectedProgram) {
            selectedEl.innerHTML = _renderParamProgramCardHtml(_paramProgramModalState.selectedProgram, { selected: true });
        } else {
            selectedEl.innerHTML = _solveScoreModalMessageHtml('Select a saved program to inspect before loading.');
        }
    }
    if (statusEl) {
        statusEl.textContent = _paramProgramModalState.status;
        statusEl.className = `solve-score-program-status${_paramProgramModalState.statusError ? ' error' : ''}`;
    }
    if (nameEl) nameEl.disabled = _paramProgramModalState.actionBusy;
    if (loadBtn) loadBtn.disabled = !canLoad;
    if (saveBtn) saveBtn.disabled = !canSave;
    if (deleteBtn) deleteBtn.disabled = !canDelete;
    if (migrateBtn) migrateBtn.disabled = !canLoad;
    if (downloadBtn) downloadBtn.disabled = !canDownload;
    if (uploadBtn) uploadBtn.disabled = !canUpload;
    if (!bodyEl) return;
    const rows = Array.isArray(_paramProgramModalState.rows) ? _paramProgramModalState.rows : [];
    if (_paramProgramModalState.tableState === 'loading') {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">Loading saved programs...</td></tr>';
        return;
    }
    if (_paramProgramModalState.tableState === 'error') {
        bodyEl.innerHTML = `<tr class="tri-popup-empty"><td colspan="3">${_escapeHtml(_paramProgramModalState.status || 'Failed to load saved programs.')}</td></tr>`;
        return;
    }
    const paramVisible = _programModalVisibleRows(_paramProgramModalState);
    _programModalSortArrows(_paramProgramModalState, 'param-program-modal');
    if (!rows.length) {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">No saved param programs found.</td></tr>';
        return;
    }
    if (!paramVisible.length) {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">No programs match the filter.</td></tr>';
        return;
    }
    bodyEl.innerHTML = paramVisible.map((row) => {
        const active = row.id === _paramProgramModalState.selectedId ? ' active' : '';
        return `
            <tr class="tri-popup-row${active}" data-param-program-id="${_escapeHtml(row.id)}">
                <td><div class="tri-popup-name"><div>${_escapeHtml(row.name)}</div></div></td>
                <td>${Number(row.statement_count || 0)}</td>
                <td>${_escapeHtml(row.saved_at || '')}</td>
            </tr>
        `;
    }).join('');
    Array.from(bodyEl.querySelectorAll('[data-param-program-id]')).forEach((rowEl) => {
        rowEl.addEventListener('click', () => {
            const id = rowEl.getAttribute('data-param-program-id') || '';
            if (id) void _selectParamProgram(id);
        });
    });
}

async function _fetchParamProgramById(id) {
    const resp = await lambdaPost('storage', { id }, '/fetch-param-program');
    return resp.program;
}

async function _refreshParamProgramRows(options = {}) {
    const preserveSelection = options.preserveSelection !== false;
    const keepStatus = !!options.keepStatus;
    const selectedId = preserveSelection ? _paramProgramModalState.selectedId : '';
    _paramProgramModalState.tableState = 'loading';
    if (!keepStatus) _setParamProgramModalStatus('', false);
    _renderParamProgramModal();
    try {
        const resp = await lambdaPost('storage', {}, '/list-param-programs');
        const rows = Array.isArray(resp.programs) ? resp.programs : [];
        _paramProgramModalState.rows = rows;
        _paramProgramModalState.tableState = 'loaded';
        if (selectedId && rows.some((row) => row.id === selectedId)) {
            _paramProgramModalState.selectedId = selectedId;
            if (_paramProgramModalState.selectedProgram && _paramProgramModalState.selectedProgram.id !== selectedId) {
                _paramProgramModalState.selectedProgram = null;
            }
        } else {
            _paramProgramModalState.selectedId = '';
            _paramProgramModalState.selectedProgram = null;
            _paramProgramModalState.selectedError = '';
            _paramProgramModalState.selectedLoading = false;
        }
    } catch (e) {
        _paramProgramModalState.rows = [];
        _paramProgramModalState.tableState = 'error';
        _paramProgramModalState.selectedId = '';
        _paramProgramModalState.selectedProgram = null;
        _paramProgramModalState.selectedError = '';
        _paramProgramModalState.selectedLoading = false;
        _setParamProgramModalStatus(e && e.message ? e.message : String(e), true);
    }
    _renderParamProgramModal();
}

async function _selectParamProgram(id) {
    const row = (_paramProgramModalState.rows || []).find((entry) => entry.id === id);
    if (!row) return;
    const currentName = String(_paramProgramModalState.nameInput || '').trim();
    if (!currentName || currentName === _paramProgramModalState.lastSelectedName) {
        _paramProgramModalState.nameInput = row.name;
    }
    _paramProgramModalState.lastSelectedName = row.name;
    _paramProgramModalState.selectedId = id;
    if (_paramProgramModalState.selectedProgram && _paramProgramModalState.selectedProgram.id === id) {
        _paramProgramModalState.selectedError = '';
        _paramProgramModalState.selectedLoading = false;
        _renderParamProgramModal();
        return;
    }
    _paramProgramModalState.selectedProgram = null;
    _paramProgramModalState.selectedError = '';
    _paramProgramModalState.selectedLoading = true;
    _renderParamProgramModal();
    try {
        const program = await _fetchParamProgramById(id);
        if (_paramProgramModalState.selectedId !== id) return;
        _paramProgramModalState.selectedProgram = program;
        _paramProgramModalState.selectedError = '';
        _paramProgramModalState.selectedLoading = false;
    } catch (e) {
        if (_paramProgramModalState.selectedId !== id) return;
        _paramProgramModalState.selectedProgram = null;
        _paramProgramModalState.selectedError = e && e.message ? e.message : String(e);
        _paramProgramModalState.selectedLoading = false;
    }
    _renderParamProgramModal();
}

function openParamProgramModal() {
    _paramProgramModalState.open = true;
    _paramProgramModalState.actionBusy = false;
    _paramProgramModalState.nameInput = _paramProgramDefaultName();
    _paramProgramModalState.lastSelectedName = '';
    _setParamProgramModalStatus('', false);
    _renderParamProgramModal();
    if (_paramProgramModalState.tableState !== 'loaded') {
        void _refreshParamProgramRows({ preserveSelection: true });
    }
    const nameEl = document.getElementById('param-program-modal-name');
    if (nameEl && typeof nameEl.focus === 'function') nameEl.focus();
}

function _closeParamProgramModal() {
    _paramProgramModalState.open = false;
    _renderParamProgramModal();
}

async function _loadSelectedParamProgramFromModal() {
    if (!_paramProgramModalState.selectedId) return;
    _paramProgramModalState.actionBusy = true;
    _setParamProgramModalStatus('', false);
    _renderParamProgramModal();
    try {
        let program = _paramProgramModalState.selectedProgram;
        if (!program || program.id !== _paramProgramModalState.selectedId) {
            program = await _fetchParamProgramById(_paramProgramModalState.selectedId);
            _paramProgramModalState.selectedProgram = program;
        }
        _applyParamProgram(program);
        if (program && program.name) {
            _paramProgramModalState.nameInput = program.name;
            _paramProgramModalState.lastSelectedName = program.name;
        }
        _setParamProgramModalStatus(program.name ? `Loaded ${program.name}` : 'Loaded param program', false);
    } catch (e) {
        _setParamProgramModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _paramProgramModalState.actionBusy = false;
        _renderParamProgramModal();
    }
}

async function _saveCurrentParamProgramFromModal() {
    const name = String(_paramProgramModalState.nameInput || '').trim();
    if (!name) return;
    let payload;
    try {
        payload = _portableParamProgramPayload(name);
    } catch (e) {
        _setParamProgramModalStatus(e && e.message ? e.message : String(e), true);
        _renderParamProgramModal();
        return;
    }
    _paramProgramModalState.actionBusy = true;
    _setParamProgramModalStatus('', false);
    _renderParamProgramModal();
    try {
        const savePayload = { name: payload.name, chain: payload.chain };
        if (Object.prototype.hasOwnProperty.call(payload, 'source_text')) savePayload.source_text = payload.source_text;
        const resp = await lambdaPost('storage', savePayload, '/save-param-program');
        const program = resp && resp.program ? resp.program : null;
        if (!program) throw new Error('save-param-program returned no program');
        _paramProgramModalState.selectedId = program.id || '';
        _paramProgramModalState.selectedProgram = program;
        _paramProgramModalState.selectedError = '';
        _paramProgramModalState.selectedLoading = false;
        _paramProgramModalState.nameInput = program.name || payload.name;
        _paramProgramModalState.lastSelectedName = program.name || payload.name;
        const message = resp.overwritten
            ? `Saved ${program.name} (overwrote existing entry)`
            : `Saved ${program.name}`;
        _setParamProgramModalStatus(message, false);
        _paramProgramStatus(message, false);
        await _refreshParamProgramRows({ preserveSelection: true, keepStatus: true });
    } catch (e) {
        _setParamProgramModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _paramProgramModalState.actionBusy = false;
        _renderParamProgramModal();
    }
}

async function _deleteSelectedParamProgramFromModal() {
    const id = String(_paramProgramModalState.selectedId || '').trim();
    if (!id) return;
    if (typeof confirm === 'function' && !confirm(`Delete param program "${id}"?`)) return;
    _paramProgramModalState.actionBusy = true;
    _setParamProgramModalStatus('', false);
    _renderParamProgramModal();
    try {
        await lambdaPost('storage', { id }, '/delete-param-program');
        _paramProgramModalState.selectedId = '';
        _paramProgramModalState.selectedProgram = null;
        _paramProgramModalState.selectedError = '';
        _paramProgramModalState.selectedLoading = false;
        const message = `Deleted ${id}`;
        _setParamProgramModalStatus(message, false);
        _paramProgramStatus(message, false);
        await _refreshParamProgramRows({ preserveSelection: false, keepStatus: true });
    } catch (e) {
        _setParamProgramModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _paramProgramModalState.actionBusy = false;
        _renderParamProgramModal();
    }
}

async function _migrateSelectedParamProgramFromModal() {
    await _migrateSelectedProgramFromModal(
        'param',
        _paramProgramModalState,
        _setParamProgramModalStatus,
        _renderParamProgramModal,
        _refreshParamProgramRows,
        _paramProgramStatus
    );
}

function _downloadCurrentParamProgramFromModal() {
    try {
        const payload = _portableParamProgramPayload(
            String(_paramProgramModalState.nameInput || '').trim() || _paramProgramDefaultName()
        );
        const filename = _paramProgramFilename(payload.name);
        const blob = new Blob([`${JSON.stringify(payload, null, 2)}\n`], { type: 'application/json' });
        _downloadBlob(blob, filename);
        const message = `Downloaded ${filename}`;
        _setParamProgramModalStatus(message, false);
        _paramProgramStatus(message, false);
    } catch (e) {
        _setParamProgramModalStatus(e && e.message ? e.message : String(e), true);
    }
    _renderParamProgramModal();
}

function _triggerParamProgramUploadFromModal() {
    const inputEl = document.getElementById('param-program-modal-upload-file');
    if (!inputEl || _paramProgramModalState.actionBusy) return;
    inputEl.value = '';
    inputEl.click();
}

async function _importParamProgramFileFromModal(file) {
    _paramProgramModalState.actionBusy = true;
    _setParamProgramModalStatus('', false);
    _renderParamProgramModal();
    try {
        const text = await _readTextFile(file);
        const raw = JSON.parse(text);
        const parsed = _applyParamProgram(raw);
        if (parsed.name) {
            _paramProgramModalState.nameInput = parsed.name;
            _paramProgramModalState.lastSelectedName = parsed.name;
        } else if (!String(_paramProgramModalState.nameInput || '').trim()) {
            _paramProgramModalState.nameInput = _paramProgramDefaultName();
        }
        const message = parsed.name ? `Loaded ${parsed.name} from JSON` : 'Loaded param program from JSON';
        _setParamProgramModalStatus(message, false);
    } catch (e) {
        _setParamProgramModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _paramProgramModalState.actionBusy = false;
        _renderParamProgramModal();
    }
}

let _coeffProgramModalState = {
    open: false,
    tableState: 'idle',
    rows: [],
    selectedId: '',
    selectedProgram: null,
    selectedError: '',
    selectedLoading: false,
    actionBusy: false,
    status: '',
    statusError: false,
    nameInput: '',
    lastSelectedName: '',
    filterText: '',
    sortKey: 'name',
    sortDir: 1,
};

// Natural (arithmetic) name ordering: digit runs compare numerically, so
// poly_1 < poly_2 < poly_110. Case-insensitive; ties break on raw string.
function _naturalNameCompare(a, b) {
    const ax = String(a || '').toLowerCase().match(/\d+|\D+/g) || [];
    const bx = String(b || '').toLowerCase().match(/\d+|\D+/g) || [];
    for (let i = 0; i < Math.max(ax.length, bx.length); i++) {
        const av = ax[i]; const bv = bx[i];
        if (av === undefined) return -1;
        if (bv === undefined) return 1;
        const an = /^\d+$/.test(av); const bn = /^\d+$/.test(bv);
        if (an && bn) {
            const d = Number(av) - Number(bv);
            if (d) return d;
        } else if (av !== bv) {
            return av < bv ? -1 : 1;
        }
    }
    return String(a) < String(b) ? -1 : (String(a) > String(b) ? 1 : 0);
}

// Filter (regex when valid, else case-insensitive substring) + sort view of
// the loaded rows; never mutates state.rows. Shared by the coeff, param and
// solve-score program modals.
function _programModalVisibleRows(state) {
    let rows = Array.isArray(state.rows) ? state.rows.slice() : [];
    const raw = String(state.filterText || '').trim();
    if (raw) {
        let test;
        try {
            const rx = new RegExp(raw, 'i');
            test = name => rx.test(name);
        } catch (e) {
            const needle = raw.toLowerCase();
            test = name => String(name).toLowerCase().includes(needle);
        }
        rows = rows.filter(row => test(String(row.name || row.id || '')));
    }
    const dir = state.sortDir < 0 ? -1 : 1;
    if (state.sortKey === 'saved') {
        rows.sort((a, b) => dir * String(a.saved_at || '').localeCompare(String(b.saved_at || '')));
    } else {
        rows.sort((a, b) => dir * _naturalNameCompare(a.name || a.id, b.name || b.id));
    }
    return rows;
}

function _coeffProgramVisibleRows() {
    return _programModalVisibleRows(_coeffProgramModalState);
}

function _programModalSortArrows(state, prefix) {
    const nameHdr = document.getElementById(`${prefix}-sort-name`);
    const savedHdr = document.getElementById(`${prefix}-sort-saved`);
    const arrow = state.sortDir < 0 ? ' \u25bc' : ' \u25b2';
    if (nameHdr) nameHdr.textContent = 'Name' + (state.sortKey === 'name' ? arrow : '');
    if (savedHdr) savedHdr.textContent = 'Saved' + (state.sortKey === 'saved' ? arrow : '');
}

function _coeffProgramFilename(name) {
    const slug = String(name || '').trim().toLowerCase()
        .replace(/[^a-z0-9]+/g, '-')
        .replace(/^-+|-+$/g, '') || 'coeff-program';
    return `${slug}.json`;
}

function _setCoeffProgramModalStatus(message, isError = false) {
    _coeffProgramModalState.status = String(message || '');
    _coeffProgramModalState.statusError = !!isError;
}

function _currentCoeffProgramSummary() {
    try {
        const payload = _portableCoeffProgramPayload(_coeffProgramModalState.nameInput || _coeffProgramDefaultName());
        const sourceCount = _coeffProgramSourceStatementCount(payload.source_text || '');
        return { ...payload, statement_count: sourceCount || (Array.isArray(payload.chain) ? payload.chain.length : 0) };
    } catch (e) {
        return { error: e && e.message ? e.message : String(e) };
    }
}

function _coeffProgramMetaHtml(program, options = {}) {
    const rows = [];
    const name = String(program && program.name || '').trim();
    const savedAt = String(program && program.saved_at || '').trim();
    const sourceCount = _coeffProgramSourceStatementCount(program && program.source_text || '');
    const statements = Number(program && program.statement_count != null ? program.statement_count : (sourceCount || (Array.isArray(program && program.chain) ? program.chain.length : NaN)));
    const tokenCount = Number(program && program.token_count);
    if (name) rows.push(['Name', name]);
    if (options.selected && savedAt) rows.push(['Saved', savedAt]);
    if (Number.isFinite(statements)) rows.push(['Statements', String(statements)]);
    if (Number.isFinite(tokenCount)) rows.push(['Tokens', String(tokenCount)]);
    if (program && program.uses_legacy_chain_equivalent === true) rows.push(['Path', 'legacy']);
    if (program && program.uses_legacy_chain_equivalent === false) rows.push(['Path', 'VM']);
    if (!rows.length) return '';
    return `<div class="solve-score-modal-program-meta">${rows.map(([label, value]) => (
        `<div><span>${_escapeHtml(label)}</span>${_escapeHtml(value)}</div>`
    )).join('')}</div>`;
}

function _renderCoeffProgramCardHtml(program, options = {}) {
    const selected = !!options.selected;
    if (!program) {
        return _solveScoreModalMessageHtml(selected ? 'Select a saved program to inspect before loading.' : 'Current live program is unavailable.');
    }
    if (program.error) return _solveScoreModalMessageHtml(`Error: ${program.error}`, true);
    try {
        const sourceText = String(program.source_text || '');
        if (sourceText.trim()) {
            return _coeffProgramMetaHtml(program, options)
                + `<pre class="coeff-program-modal-source" aria-label="Coeff program source">${_escapeHtml(sourceText)}</pre>`;
        }
        return _coeffProgramMetaHtml(program, options) + _renderCoeffProgramChipsHtml(program);
    } catch (e) {
        return _solveScoreModalMessageHtml(`Error: ${e && e.message ? e.message : String(e)}`, true);
    }
}

function _renderCoeffProgramModal() {
    const overlay = document.getElementById('coeff-program-modal-overlay');
    const summaryEl = document.getElementById('coeff-program-modal-summary');
    const bodyEl = document.getElementById('coeff-program-modal-body');
    const nameEl = document.getElementById('coeff-program-modal-name');
    const currentEl = document.getElementById('coeff-program-modal-current');
    const selectedEl = document.getElementById('coeff-program-modal-selected');
    const statusEl = document.getElementById('coeff-program-modal-status');
    const currentProgram = _currentCoeffProgramSummary();
    if (overlay) {
        overlay.style.display = _coeffProgramModalState.open ? 'flex' : 'none';
        overlay.setAttribute('aria-hidden', _coeffProgramModalState.open ? 'false' : 'true');
    }
    if (summaryEl) summaryEl.textContent = 'Global saved coefficient programs on the left; current Compute-tab live program on the right. Compute runs Param Program and Coeff Program together.';
    if (nameEl && nameEl.value !== _coeffProgramModalState.nameInput) nameEl.value = _coeffProgramModalState.nameInput;
    if (currentEl) currentEl.innerHTML = _renderCoeffProgramCardHtml(currentProgram);
    if (selectedEl) {
        if (_coeffProgramModalState.selectedLoading) selectedEl.innerHTML = _solveScoreModalMessageHtml('Loading selected saved program...');
        else if (_coeffProgramModalState.selectedError) selectedEl.innerHTML = _solveScoreModalMessageHtml(`Error: ${_coeffProgramModalState.selectedError}`, true);
        else if (_coeffProgramModalState.selectedProgram) selectedEl.innerHTML = _renderCoeffProgramCardHtml(_coeffProgramModalState.selectedProgram, { selected: true });
        else selectedEl.innerHTML = _solveScoreModalMessageHtml('Select a saved program to inspect before loading.');
    }
    if (statusEl) {
        statusEl.textContent = _coeffProgramModalState.status;
        statusEl.className = `solve-score-program-status${_coeffProgramModalState.statusError ? ' error' : ''}`;
    }
    const canLoad = _coeffProgramModalState.tableState === 'loaded' && !!_coeffProgramModalState.selectedId && !_coeffProgramModalState.selectedLoading && !_coeffProgramModalState.actionBusy;
    const canSave = !_coeffProgramModalState.actionBusy && !!String(_coeffProgramModalState.nameInput || '').trim() && !currentProgram.error;
    [['load', canLoad], ['delete', canLoad], ['migrate', canLoad], ['save', canSave], ['download', !_coeffProgramModalState.actionBusy && !currentProgram.error], ['upload', !_coeffProgramModalState.actionBusy]].forEach(([id, enabled]) => {
        const btn = document.getElementById(`coeff-program-modal-${id}`);
        if (btn) btn.disabled = !enabled;
    });
    if (nameEl) nameEl.disabled = _coeffProgramModalState.actionBusy;
    if (!bodyEl) return;
    const rows = Array.isArray(_coeffProgramModalState.rows) ? _coeffProgramModalState.rows : [];
    if (_coeffProgramModalState.tableState === 'loading') {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">Loading saved programs...</td></tr>';
        return;
    }
    if (_coeffProgramModalState.tableState === 'error') {
        bodyEl.innerHTML = `<tr class="tri-popup-empty"><td colspan="3">${_escapeHtml(_coeffProgramModalState.status || 'Failed to load saved programs.')}</td></tr>`;
        return;
    }
    const visible = _coeffProgramVisibleRows();
    const nameHdr = document.getElementById('coeff-program-modal-sort-name');
    const savedHdr = document.getElementById('coeff-program-modal-sort-saved');
    const arrow = _coeffProgramModalState.sortDir < 0 ? ' \u25bc' : ' \u25b2';
    if (nameHdr) nameHdr.textContent = 'Name' + (_coeffProgramModalState.sortKey === 'name' ? arrow : '');
    if (savedHdr) savedHdr.textContent = 'Saved' + (_coeffProgramModalState.sortKey === 'saved' ? arrow : '');
    if (!rows.length) {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">No saved coeff programs found.</td></tr>';
        return;
    }
    if (!visible.length) {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">No programs match the filter.</td></tr>';
        return;
    }
    bodyEl.innerHTML = visible.map(row => {
        const active = row.id === _coeffProgramModalState.selectedId ? ' active' : '';
        return `<tr class="tri-popup-row${active}" data-coeff-program-id="${_escapeHtml(row.id)}"><td><div class="tri-popup-name"><div>${_escapeHtml(row.name)}</div></div></td><td>${Number(row.statement_count || 0)}</td><td>${_escapeHtml(row.saved_at || '')}</td></tr>`;
    }).join('');
    Array.from(bodyEl.querySelectorAll('[data-coeff-program-id]')).forEach(rowEl => {
        rowEl.addEventListener('click', () => {
            const id = rowEl.getAttribute('data-coeff-program-id') || '';
            if (id) void _selectCoeffProgram(id);
        });
    });
}

async function _fetchCoeffProgramById(id) {
    const resp = await lambdaPost('storage', { id }, '/fetch-coeff-program');
    return resp.program;
}

async function _refreshCoeffProgramRows(options = {}) {
    const selectedId = options.preserveSelection !== false ? _coeffProgramModalState.selectedId : '';
    _coeffProgramModalState.tableState = 'loading';
    if (!options.keepStatus) _setCoeffProgramModalStatus('', false);
    _renderCoeffProgramModal();
    try {
        const resp = await lambdaPost('storage', {}, '/list-coeff-programs');
        const rows = Array.isArray(resp.programs) ? resp.programs : [];
        _coeffProgramModalState.rows = rows;
        _coeffProgramModalState.tableState = 'loaded';
        _coeffProgramModalState.selectedId = selectedId && rows.some(row => row.id === selectedId) ? selectedId : '';
        if (!_coeffProgramModalState.selectedId) {
            _coeffProgramModalState.selectedProgram = null;
            _coeffProgramModalState.selectedError = '';
            _coeffProgramModalState.selectedLoading = false;
        }
    } catch (e) {
        _coeffProgramModalState.rows = [];
        _coeffProgramModalState.tableState = 'error';
        _coeffProgramModalState.selectedId = '';
        _coeffProgramModalState.selectedProgram = null;
        _coeffProgramModalState.selectedError = '';
        _coeffProgramModalState.selectedLoading = false;
        _setCoeffProgramModalStatus(e && e.message ? e.message : String(e), true);
    }
    _renderCoeffProgramModal();
}

async function _selectCoeffProgram(id) {
    const row = (_coeffProgramModalState.rows || []).find(entry => entry.id === id);
    if (!row) return;
    const currentName = String(_coeffProgramModalState.nameInput || '').trim();
    if (!currentName || currentName === _coeffProgramModalState.lastSelectedName) _coeffProgramModalState.nameInput = row.name;
    _coeffProgramModalState.lastSelectedName = row.name;
    _coeffProgramModalState.selectedId = id;
    _coeffProgramModalState.selectedProgram = null;
    _coeffProgramModalState.selectedError = '';
    _coeffProgramModalState.selectedLoading = true;
    _renderCoeffProgramModal();
    try {
        const program = await _fetchCoeffProgramById(id);
        if (_coeffProgramModalState.selectedId !== id) return;
        _coeffProgramModalState.selectedProgram = program;
        _coeffProgramModalState.selectedLoading = false;
    } catch (e) {
        if (_coeffProgramModalState.selectedId !== id) return;
        _coeffProgramModalState.selectedError = e && e.message ? e.message : String(e);
        _coeffProgramModalState.selectedLoading = false;
    }
    _renderCoeffProgramModal();
}

function openCoeffProgramModal() {
    _coeffProgramModalState.open = true;
    _coeffProgramModalState.actionBusy = false;
    _coeffProgramModalState.nameInput = _coeffProgramDefaultName();
    _coeffProgramModalState.lastSelectedName = '';
    _setCoeffProgramModalStatus('', false);
    _renderCoeffProgramModal();
    if (_coeffProgramModalState.tableState !== 'loaded') {
        void _refreshCoeffProgramRows({ preserveSelection: true });
    }
    const nameEl = document.getElementById('coeff-program-modal-name');
    if (nameEl && typeof nameEl.focus === 'function') nameEl.focus();
}

function _closeCoeffProgramModal() {
    _coeffProgramModalState.open = false;
    _renderCoeffProgramModal();
}

async function _loadSelectedCoeffProgramFromModal() {
    if (!_coeffProgramModalState.selectedId) return;
    _coeffProgramModalState.actionBusy = true;
    _renderCoeffProgramModal();
    try {
        const program = _coeffProgramModalState.selectedProgram || await _fetchCoeffProgramById(_coeffProgramModalState.selectedId);
        _coeffProgramModalState.selectedProgram = program;
        _applyCoeffProgram(program);
        if (program && program.name) {
            _coeffProgramModalState.nameInput = program.name;
            _coeffProgramModalState.lastSelectedName = program.name;
        }
        _setCoeffProgramModalStatus(program.name ? `Loaded ${program.name}` : 'Loaded coeff program', false);
    } catch (e) {
        _setCoeffProgramModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _coeffProgramModalState.actionBusy = false;
        _renderCoeffProgramModal();
    }
}

async function _saveCurrentCoeffProgramFromModal() {
    const name = String(_coeffProgramModalState.nameInput || '').trim();
    if (!name) return;
    let payload;
    try {
        payload = _portableCoeffProgramPayload(name);
    } catch (e) {
        _setCoeffProgramModalStatus(e && e.message ? e.message : String(e), true);
        _renderCoeffProgramModal();
        return;
    }
    _coeffProgramModalState.actionBusy = true;
    _renderCoeffProgramModal();
    try {
        const savePayload = { name: payload.name, chain: payload.chain };
        if (Object.prototype.hasOwnProperty.call(payload, 'source_text')) savePayload.source_text = payload.source_text;
        const resp = await lambdaPost('storage', savePayload, '/save-coeff-program');
        const program = resp && resp.program ? resp.program : null;
        if (!program) throw new Error('save-coeff-program returned no program');
        _coeffProgramModalState.selectedId = program.id || '';
        _coeffProgramModalState.selectedProgram = program;
        _coeffProgramModalState.nameInput = program.name || payload.name;
        _coeffProgramModalState.lastSelectedName = program.name || payload.name;
        const message = resp.overwritten ? `Saved ${program.name} (overwrote existing entry)` : `Saved ${program.name}`;
        _setCoeffProgramModalStatus(message, false);
        _coeffProgramStatus(message, false);
        await _refreshCoeffProgramRows({ preserveSelection: true, keepStatus: true });
    } catch (e) {
        _setCoeffProgramModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _coeffProgramModalState.actionBusy = false;
        _renderCoeffProgramModal();
    }
}

async function _deleteSelectedCoeffProgramFromModal() {
    const id = String(_coeffProgramModalState.selectedId || '').trim();
    if (!id) return;
    if (typeof confirm === 'function' && !confirm(`Delete coeff program "${id}"?`)) return;
    _coeffProgramModalState.actionBusy = true;
    _renderCoeffProgramModal();
    try {
        await lambdaPost('storage', { id }, '/delete-coeff-program');
        _coeffProgramModalState.selectedId = '';
        _coeffProgramModalState.selectedProgram = null;
        const message = `Deleted ${id}`;
        _setCoeffProgramModalStatus(message, false);
        _coeffProgramStatus(message, false);
        await _refreshCoeffProgramRows({ preserveSelection: false, keepStatus: true });
    } catch (e) {
        _setCoeffProgramModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _coeffProgramModalState.actionBusy = false;
        _renderCoeffProgramModal();
    }
}

async function _migrateSelectedCoeffProgramFromModal() {
    await _migrateSelectedProgramFromModal(
        'coeff',
        _coeffProgramModalState,
        _setCoeffProgramModalStatus,
        _renderCoeffProgramModal,
        _refreshCoeffProgramRows,
        _coeffProgramStatus
    );
}

function _downloadCurrentCoeffProgramFromModal() {
    try {
        const payload = _portableCoeffProgramPayload(String(_coeffProgramModalState.nameInput || '').trim() || _coeffProgramDefaultName());
        const filename = _coeffProgramFilename(payload.name);
        _downloadBlob(new Blob([`${JSON.stringify(payload, null, 2)}\n`], { type: 'application/json' }), filename);
        const message = `Downloaded ${filename}`;
        _setCoeffProgramModalStatus(message, false);
        _coeffProgramStatus(message, false);
    } catch (e) {
        _setCoeffProgramModalStatus(e && e.message ? e.message : String(e), true);
    }
    _renderCoeffProgramModal();
}

function _triggerCoeffProgramUploadFromModal() {
    const inputEl = document.getElementById('coeff-program-modal-upload-file');
    if (!inputEl || _coeffProgramModalState.actionBusy) return;
    inputEl.value = '';
    inputEl.click();
}

async function _importCoeffProgramFileFromModal(file) {
    _coeffProgramModalState.actionBusy = true;
    _renderCoeffProgramModal();
    try {
        const raw = JSON.parse(await _readTextFile(file));
        const parsed = _applyCoeffProgram(raw);
        if (parsed.name) {
            _coeffProgramModalState.nameInput = parsed.name;
            _coeffProgramModalState.lastSelectedName = parsed.name;
        } else if (!String(_coeffProgramModalState.nameInput || '').trim()) {
            _coeffProgramModalState.nameInput = _coeffProgramDefaultName();
        }
        _setCoeffProgramModalStatus(parsed.name ? `Loaded ${parsed.name} from JSON` : 'Loaded coeff program from JSON', false);
    } catch (e) {
        _setCoeffProgramModalStatus(e && e.message ? e.message : String(e), true);
    } finally {
        _coeffProgramModalState.actionBusy = false;
        _renderCoeffProgramModal();
    }
}

// The score-chip picker/add/select cluster was deleted with editable chips:
// solve-score authoring is text-only; the chain persists only for legacy
// input sync and read-only modal strips.


;(window.__ppParts = window.__ppParts || []).push('03-program-modals');

// ── Root Programs modal (shared by the render and palette root editors) ──
let _rootProgramModalState = {
    open: false,
    target: 'render',          // which editor invoked it: 'render' | 'palette'
    tableState: 'idle',
    rows: [],
    selectedId: '',
    selectedProgram: null,
    selectedLoading: false,
    actionBusy: false,
    status: '',
    statusError: false,
    nameInput: '',
    lastSelectedName: '',
    filterText: '',
    sortKey: 'name',
    sortDir: 1,
};

function _setRootProgramModalStatus(message, isError) {
    _rootProgramModalState.status = String(message || '');
    _rootProgramModalState.statusError = !!isError;
}

function _rootProgramSourceText(target) {
    const el = document.getElementById(`${target}-rt-source-text`);
    return el ? String(el.value || '') : '';
}

function _renderRootProgramModal() {
    const overlay = document.getElementById('root-program-modal-overlay');
    if (!overlay) return;
    overlay.classList.toggle('visible', !!_rootProgramModalState.open);
    overlay.setAttribute('aria-hidden', _rootProgramModalState.open ? 'false' : 'true');
    if (!_rootProgramModalState.open) return;
    const summaryEl = document.getElementById('root-program-modal-summary');
    if (summaryEl) summaryEl.textContent = `Target: ${_rootProgramModalState.target} root transforms`;
    const statusEl = document.getElementById('root-program-modal-status');
    if (statusEl) {
        statusEl.textContent = _rootProgramModalState.status;
        statusEl.className = `solve-score-program-status${_rootProgramModalState.statusError ? ' error' : ''}`;
    }
    const selectedEl = document.getElementById('root-program-modal-selected');
    if (selectedEl) {
        const program = _rootProgramModalState.selectedProgram;
        if (_rootProgramModalState.selectedLoading) selectedEl.textContent = 'Loading...';
        else if (program) selectedEl.textContent = program.source_text || program.display || '';
        else selectedEl.textContent = 'Select a saved program to inspect before loading.';
    }
    const nameEl = document.getElementById('root-program-modal-name');
    if (nameEl && nameEl.value !== _rootProgramModalState.nameInput) nameEl.value = _rootProgramModalState.nameInput;
    const canLoad = _rootProgramModalState.tableState === 'loaded' && !!_rootProgramModalState.selectedId
        && !_rootProgramModalState.selectedLoading && !_rootProgramModalState.actionBusy;
    const canSave = !_rootProgramModalState.actionBusy
        && !!String(_rootProgramModalState.nameInput || '').trim()
        && !!_rootProgramSourceText(_rootProgramModalState.target).trim();
    [['load', canLoad], ['delete', canLoad], ['save', canSave]].forEach(([id, enabled]) => {
        const btn = document.getElementById(`root-program-modal-${id}`);
        if (btn) btn.disabled = !enabled;
    });
    const bodyEl = document.getElementById('root-program-modal-body');
    if (!bodyEl) return;
    const rows = Array.isArray(_rootProgramModalState.rows) ? _rootProgramModalState.rows : [];
    if (_rootProgramModalState.tableState === 'loading') {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">Loading saved programs...</td></tr>';
        return;
    }
    if (_rootProgramModalState.tableState === 'error') {
        bodyEl.innerHTML = `<tr class="tri-popup-empty"><td colspan="3">${_escapeHtml(_rootProgramModalState.status || 'Failed to load saved programs.')}</td></tr>`;
        return;
    }
    const visible = _programModalVisibleRows(_rootProgramModalState);
    _programModalSortArrows(_rootProgramModalState, 'root-program-modal');
    if (!rows.length) {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">No saved root programs found.</td></tr>';
        return;
    }
    if (!visible.length) {
        bodyEl.innerHTML = '<tr class="tri-popup-empty"><td colspan="3">No programs match the filter.</td></tr>';
        return;
    }
    bodyEl.innerHTML = visible.map(row => {
        const active = row.id === _rootProgramModalState.selectedId ? ' active' : '';
        return `<tr class="tri-popup-row${active}" data-root-program-id="${_escapeHtml(row.id)}"><td><div class="tri-popup-name"><div>${_escapeHtml(row.name)}</div></div></td><td>${Number(row.statement_count || 0)}</td><td>${_escapeHtml(row.saved_at || '')}</td></tr>`;
    }).join('');
    Array.from(bodyEl.querySelectorAll('[data-root-program-id]')).forEach(rowEl => {
        rowEl.addEventListener('click', () => {
            const id = rowEl.getAttribute('data-root-program-id') || '';
            if (id) void _selectRootProgram(id);
        });
    });
}

async function _refreshRootProgramRows() {
    _rootProgramModalState.tableState = 'loading';
    _renderRootProgramModal();
    try {
        const resp = await lambdaPost('storage', {}, '/list-root-programs');
        _rootProgramModalState.rows = Array.isArray(resp.programs) ? resp.programs : [];
        _rootProgramModalState.tableState = 'loaded';
        if (_rootProgramModalState.selectedId &&
            !_rootProgramModalState.rows.some(row => row.id === _rootProgramModalState.selectedId)) {
            _rootProgramModalState.selectedId = '';
            _rootProgramModalState.selectedProgram = null;
            _rootProgramModalState.selectedLoading = false;
        }
        if (resp.error_count) {
            _setRootProgramModalStatus(`${resp.error_count} saved program(s) unreadable — see server logs`, true);
        }
    } catch (e) {
        _rootProgramModalState.rows = [];
        _rootProgramModalState.tableState = 'error';
        _setRootProgramModalStatus(e && e.message ? e.message : String(e), true);
    }
    _renderRootProgramModal();
}

async function _selectRootProgram(id) {
    const row = (_rootProgramModalState.rows || []).find(entry => entry.id === id);
    if (!row) return;
    const currentName = String(_rootProgramModalState.nameInput || '').trim();
    if (!currentName || currentName === _rootProgramModalState.lastSelectedName) _rootProgramModalState.nameInput = row.name;
    _rootProgramModalState.lastSelectedName = row.name;
    _rootProgramModalState.selectedId = id;
    _rootProgramModalState.selectedProgram = null;
    _rootProgramModalState.selectedLoading = true;
    _renderRootProgramModal();
    try {
        const resp = await lambdaPost('storage', { id }, '/fetch-root-program');
        if (_rootProgramModalState.selectedId !== id) return;
        _rootProgramModalState.selectedProgram = resp.program || null;
    } catch (e) {
        if (_rootProgramModalState.selectedId !== id) return;
        _setRootProgramModalStatus(e && e.message ? e.message : String(e), true);
    }
    _rootProgramModalState.selectedLoading = false;
    _renderRootProgramModal();
}

async function _loadSelectedRootProgramFromModal() {
    let program = _rootProgramModalState.selectedProgram;
    if (!program && _rootProgramModalState.selectedId) {
        // detail fetch failed earlier: retry on demand instead of a silently
        // inert Load button
        try {
            const resp = await lambdaPost('storage', { id: _rootProgramModalState.selectedId }, '/fetch-root-program');
            program = resp.program || null;
            _rootProgramModalState.selectedProgram = program;
        } catch (e) {
            _setRootProgramModalStatus(e && e.message ? e.message : String(e), true);
            _renderRootProgramModal();
            return;
        }
    }
    if (!program || !program.source_text) return;
    const target = _rootProgramModalState.target;
    const el = document.getElementById(`${target}-rt-source-text`);
    if (!el) return;
    el.value = String(program.source_text || '');
    _onRootProgramSourceInput(target);
    _setRootProgramModalStatus(`Loaded ${program.name || program.id} into ${target}`, false);
    _renderRootProgramModal();
}

async function _saveRootProgramFromModal() {
    const name = String(_rootProgramModalState.nameInput || '').trim();
    const source_text = _rootProgramSourceText(_rootProgramModalState.target);
    if (!name || !source_text.trim()) return;
    _rootProgramModalState.actionBusy = true;
    _renderRootProgramModal();
    try {
        const resp = await lambdaPost('storage', { name, source_text }, '/save-root-program');
        _setRootProgramModalStatus(`Saved ${resp.program && resp.program.name ? resp.program.name : name}${resp.overwritten ? ' (overwrote)' : ''}`, false);
        await _refreshRootProgramRows();
    } catch (e) {
        _setRootProgramModalStatus(e && e.message ? e.message : String(e), true);
    }
    _rootProgramModalState.actionBusy = false;
    _renderRootProgramModal();
}

async function _deleteSelectedRootProgram() {
    const id = _rootProgramModalState.selectedId;
    if (!id) return;
    const row = (_rootProgramModalState.rows || []).find(entry => entry.id === id);
    const label = row && row.name ? row.name : id;
    if (typeof confirm === 'function' && !confirm(`Delete saved root program "${label}"?`)) return;
    _rootProgramModalState.actionBusy = true;
    _renderRootProgramModal();
    try {
        await lambdaPost('storage', { id }, '/delete-root-program');
        _rootProgramModalState.selectedId = '';
        _rootProgramModalState.selectedProgram = null;
        _setRootProgramModalStatus(`Deleted ${id}`, false);
        await _refreshRootProgramRows();
    } catch (e) {
        _setRootProgramModalStatus(e && e.message ? e.message : String(e), true);
    }
    _rootProgramModalState.actionBusy = false;
    _renderRootProgramModal();
}

function _closeRootProgramModal() {
    _rootProgramModalState.open = false;
    _renderRootProgramModal();
}

function openRootProgramModal(target) {
    _rootProgramModalState.open = true;
    _rootProgramModalState.target = target === 'palette' ? 'palette' : 'render';
    _rootProgramModalState.actionBusy = false;
    _setRootProgramModalStatus('', false);
    _renderRootProgramModal();
    if (_rootProgramModalState.tableState !== 'loaded') {
        void _refreshRootProgramRows();
    }
    const nameEl = document.getElementById('root-program-modal-name');
    if (nameEl && typeof nameEl.focus === 'function') nameEl.focus();
}
