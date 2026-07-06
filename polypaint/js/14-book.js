// PolyPaint part 14: Book tab (book-maker-design.md §8).
// Classic script: load order matters and is defined by the <script src="js/...">
// tags in index.html (top-level functions and let/const bindings are shared
// across all parts).

let _bookState = {
    books: [],            // list rows {id, name, entry_count, saved_at}
    listLoaded: false,
    activeId: localStorage.getItem('polypaint_active_book') || '',
    doc: null,            // the loaded book document (source of truth)
    latestOutput: null,
    dirty: false,
    hydrated: {},         // entry_id -> {preview_url, missing}
    selectedEntryId: '',
    compile: null,        // {runId, phase, expected, done}
};

function _bookStatus(msg, isError = false) {
    const el = document.getElementById('book-status');
    if (el) { el.textContent = msg || ''; el.className = isError ? 'status error' : 'status'; }
}

function _bookLog(msg, cls = '') { log(msg, cls, 'book-log'); }

async function _bookRefreshList(force = false) {
    if (_bookState.listLoaded && !force) return;
    const resp = await lambdaPost('storage', {}, '/list-books');
    _bookState.books = Array.isArray(resp.books) ? resp.books : [];
    _bookState.listLoaded = true;
    if ((resp.error_count || 0) > 0) _bookLog(`list-books: ${resp.error_count} unreadable docs`, 'err');
}

function _bookSetActive(id) {
    _bookState.activeId = id || '';
    localStorage.setItem('polypaint_active_book', _bookState.activeId);
}

async function _bookLoadActive() {
    _bookState.doc = null;
    _bookState.latestOutput = null;
    _bookState.hydrated = {};
    _bookState.selectedEntryId = '';
    _bookState.dirty = false;
    if (!_bookState.activeId) return;
    try {
        const resp = await lambdaPost('storage', { id: _bookState.activeId }, '/fetch-book');
        _bookState.doc = resp.book;
        _bookState.latestOutput = resp.latest_output || null;
    } catch (e) {
        _bookStatus(`fetch-book: ${e.message}`, true);
        _bookSetActive('');
    }
}

async function _bookHydrateEntries() {
    const doc = _bookState.doc;
    if (!doc || !doc.entries || !doc.entries.length) return;
    const byJob = {};
    for (const entry of doc.entries) {
        (byJob[entry.job_id] = byJob[entry.job_id] || []).push(entry);
    }
    await asyncPool(4, Object.keys(byJob), async (jobId) => {
        let colorRows = [];
        try {
            const summary = await lambdaPost('storage', { job_id: jobId }, '/render-summary');
            colorRows = ((summary.families || {}).color) || [];
        } catch (e) { /* job gone: entries mark missing below */ }
        for (const entry of byJob[jobId]) {
            const match = colorRows.find(r => r.artifact_id === entry.artifact_id);
            _bookState.hydrated[entry.entry_id] = match
                ? { preview_url: match.preview_url || match.image_url || '', missing: false }
                : { preview_url: '', missing: true };
        }
    });
    _renderBookTab();
}

function _bookEntryRow(entry, idx) {
    const hyd = _bookState.hydrated[entry.entry_id] || {};
    const selected = entry.entry_id === _bookState.selectedEntryId;
    const cover = _bookState.doc.cover_entry_id === entry.entry_id;
    const thumb = hyd.preview_url
        ? `<img src="${_escapeHtml(hyd.preview_url)}" style="width:48px;height:48px;object-fit:cover">`
        : `<div style="width:48px;height:48px;background:#222;display:flex;align-items:center;justify-content:center;font-size:9px;color:#888">${hyd.missing ? 'missing' : '...'}</div>`;
    const title = _escapeHtml(entry.title_override || entry.display_name || entry.artifact_id);
    const eid = _escapeHtml(entry.entry_id);
    return `<div class="book-entry-row${selected ? ' selected' : ''}" data-entry="${eid}"
        style="display:flex;align-items:center;gap:8px;padding:4px 6px;border-bottom:1px solid #2b3a5e;cursor:pointer${selected ? ';background:#1c2742' : ''}"
        onclick="_bookSelectEntry(this.dataset.entry)">
        <span style="color:#666;width:24px">${idx + 1}</span>${thumb}
        <span style="flex:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap">${title}${cover ? ' <span style="color:#e94560">[cover]</span>' : ''}${hyd.missing ? ' <span style="color:#e94560">[missing]</span>' : ''}</span>
        <span style="color:#666;font-size:10px">${_escapeHtml(entry.job_id)}</span>
        <button class="btn-secondary" style="padding:1px 7px" onclick="event.stopPropagation();_bookMoveEntry(this.closest('.book-entry-row').dataset.entry,-1)">▲</button>
        <button class="btn-secondary" style="padding:1px 7px" onclick="event.stopPropagation();_bookMoveEntry(this.closest('.book-entry-row').dataset.entry,1)">▼</button>
        <button class="btn-secondary" style="padding:1px 7px" onclick="event.stopPropagation();_bookRemoveEntry(this.closest('.book-entry-row').dataset.entry)">✕</button>
    </div>`;
}

function _renderBookTab() {
    const sel = document.getElementById('book-selector');
    if (sel) {
        sel.innerHTML = '<option value="">(select book)</option>' + _bookState.books.map(b =>
            `<option value="${_escapeHtml(b.id)}"${b.id === _bookState.activeId ? ' selected' : ''}>${_escapeHtml(b.name)} (${b.entry_count})</option>`).join('');
    }
    const info = document.getElementById('book-info');
    const doc = _bookState.doc;
    if (info) {
        const missing = doc ? (doc.entries || []).filter(e => (_bookState.hydrated[e.entry_id] || {}).missing).length : 0;
        info.textContent = doc
            ? `${(doc.entries || []).length} entries${missing ? `, ${missing} missing` : ''}${_bookState.dirty ? ' (unsaved)' : ''}`
            : 'no book loaded';
    }
    const list = document.getElementById('book-entry-list');
    if (list) {
        list.innerHTML = doc && (doc.entries || []).length
            ? doc.entries.map((e, i) => _bookEntryRow(e, i)).join('')
            : '<div style="padding:14px;color:#888">No entries. Right-click a tile in AllCol, or use Add to Book on the Render/Favorites tabs.</div>';
    }
    const editor = document.getElementById('book-entry-editor');
    const entry = doc ? (doc.entries || []).find(e => e.entry_id === _bookState.selectedEntryId) : null;
    if (editor) editor.style.display = entry ? 'block' : 'none';
    if (entry) {
        const t = document.getElementById('book-entry-title');
        const b = document.getElementById('book-entry-body');
        if (t && t.value !== entry.title_override) t.value = entry.title_override || '';
        if (b && b.value !== entry.body_override) b.value = entry.body_override || '';
    }
    const dl = document.getElementById('book-download-row');
    if (dl) dl.style.display = _bookState.latestOutput ? 'flex' : 'none';
    const out = _bookState.latestOutput;
    const outInfo = document.getElementById('book-output-info');
    if (outInfo && out) outInfo.textContent = `compiled ${out.compiled_at || ''}: ${out.content_pages} pages, ${out.spread_count} spreads`;
}

async function loadBookTab() {
    try {
        await _bookRefreshList();
        if (_bookState.activeId && !_bookState.doc) {
            await _bookLoadActive();
            _renderBookTab();
            void _bookHydrateEntries();
        }
        _renderBookTab();
    } catch (e) {
        _bookStatus(e.message, true);
    }
}

async function bookSelectorChanged() {
    const sel = document.getElementById('book-selector');
    _bookSetActive(sel ? sel.value : '');
    await _bookLoadActive();
    _renderBookTab();
    void _bookHydrateEntries();
}

async function bookNew() {
    const name = prompt('Book name:');
    if (!name || !name.trim()) return;
    try {
        const resp = await lambdaPost('storage', { book: { name: name.trim(), entries: [] } }, '/save-book');
        _bookSetActive(resp.book.id);
        _bookState.listLoaded = false;
        await _bookRefreshList(true);
        await _bookLoadActive();
        _renderBookTab();
        _bookStatus(`Created ${resp.book.name}`);
    } catch (e) {
        _bookStatus(e.message, true);
    }
}

async function bookSave() {
    if (!_bookState.doc) return;
    try {
        const resp = await lambdaPost('storage', { book: _bookState.doc }, '/save-book');
        _bookState.doc = resp.book;
        _bookState.dirty = false;
        _bookState.listLoaded = false;
        await _bookRefreshList(true);
        _renderBookTab();
        _bookStatus('Saved');
    } catch (e) {
        _bookStatus(e.message, true);
    }
}

async function bookDelete() {
    if (!_bookState.activeId) return;
    if (!confirm(`Delete book "${_bookState.activeId}" (doc + cached assets + outputs)?`)) return;
    try {
        await lambdaPost('storage', { id: _bookState.activeId }, '/delete-book');
        _bookSetActive('');
        _bookState.doc = null;
        _bookState.listLoaded = false;
        await _bookRefreshList(true);
        _renderBookTab();
        _bookStatus('Deleted');
    } catch (e) {
        _bookStatus(e.message, true);
    }
}

function _bookSelectEntry(entryId) {
    _bookState.selectedEntryId = entryId;
    _renderBookTab();
}

function _bookMoveEntry(entryId, delta) {
    const entries = (_bookState.doc || {}).entries || [];
    const idx = entries.findIndex(e => e.entry_id === entryId);
    const to = idx + delta;
    if (idx < 0 || to < 0 || to >= entries.length) return;
    const [row] = entries.splice(idx, 1);
    entries.splice(to, 0, row);
    _bookState.dirty = true;
    _renderBookTab();
}

function _bookRemoveEntry(entryId) {
    const doc = _bookState.doc;
    if (!doc) return;
    doc.entries = (doc.entries || []).filter(e => e.entry_id !== entryId);
    if (doc.cover_entry_id === entryId) doc.cover_entry_id = '';
    if (_bookState.selectedEntryId === entryId) _bookState.selectedEntryId = '';
    _bookState.dirty = true;
    _renderBookTab();
}

function bookSetCover() {
    if (!_bookState.doc || !_bookState.selectedEntryId) return;
    _bookState.doc.cover_entry_id = _bookState.selectedEntryId;
    _bookState.dirty = true;
    _renderBookTab();
}

function bookEntryFieldChanged() {
    const entry = (_bookState.doc || {}).entries?.find(e => e.entry_id === _bookState.selectedEntryId);
    if (!entry) return;
    entry.title_override = document.getElementById('book-entry-title')?.value || '';
    entry.body_override = document.getElementById('book-entry-body')?.value || '';
    _bookState.dirty = true;
    _renderBookTab();
}

// --- collection surfaces (design §3): all funnel through here ---
let _bookAddInFlight = Promise.resolve();
async function _bookAddEntry(ref, surfaceStatus) {
    const chained = _bookAddInFlight.then(() => _bookAddEntryImpl(ref, surfaceStatus));
    _bookAddInFlight = chained.catch(() => {});
    return chained;
}

async function _bookAddEntryImpl(ref, surfaceStatus) {
    const report = surfaceStatus || _bookStatus;
    if (!_bookState.activeId) {
        report('No active book: open the Book tab and create/select one first', true);
        return false;
    }
    try {
        if (!_bookState.doc) await _bookLoadActive();
        const doc = _bookState.doc;
        if (!doc) throw new Error('active book failed to load');
        doc.entries = doc.entries || [];
        doc.entries.push({
            job_id: ref.jobId, artifact_id: ref.artifactId, image_key: ref.imageKey,
            display_name: ref.displayName || ref.artifactId,
            added_at: new Date().toISOString().slice(0, 19) + 'Z',
            title_override: '', body_override: '',
        });
        const resp = await lambdaPost('storage', { book: doc }, '/save-book');
        _bookState.doc = resp.book;
        _bookState.dirty = false;
        report(`Added entry ${resp.book.entries.length} to "${resp.book.name}"`);
        if (document.getElementById('tab-book')?.classList.contains('active')) {
            _renderBookTab();
            void _bookHydrateEntries();
        }
        return true;
    } catch (e) {
        report(`Add to Book failed: ${e.message}`, true);
        return false;
    }
}

function addSelectedRenderArtifactToBook() {
    if (_renderActiveFamily !== 'color') return;
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    if (!art || !jobId) return;
    void _bookAddEntry(
        { jobId, artifactId: art.artifact_id, imageKey: art.image_key || '', displayName: art.display_name || art.artifact_id },
        (msg, err) => { const el = document.getElementById('render-status'); if (el) { el.textContent = msg; el.className = err ? 'status error' : 'status'; } });
}

function addSelectedFavoriteToBook() {
    const art = _favoriteArtifacts[_favoriteSelectedIdx];
    if (!art || art.missing) return;
    void _bookAddEntry(
        { jobId: art.favorite_job_id, artifactId: art.artifact_id, imageKey: art.image_key || '', displayName: art.display_name || art.artifact_id },
        (msg, err) => { const el = document.getElementById('favorites-status'); if (el) { el.textContent = msg; el.className = err ? 'status error' : 'status'; } });
}

// --- compile (design §5): prepare fan-out, then compose. Trigger is
// done >= N && errors === 0 — /check-status "complete" counts errored
// tasks as terminal, so it must never gate compose on its own. ---
function _bookCompileBtn(busy) {
    const b = document.getElementById('btn-book-compile');
    if (!b) return;
    b.disabled = busy;
    b.textContent = busy ? 'Compiling…' : 'Compile';
}

function _bookRailUpsert(state, detail) {
    const run = _bookState.compile;
    if (!run || typeof _jobsRailUpsert !== 'function') return;
    _jobsRailUpsert({
        id: 'book:' + run.runId,
        kind: 'book',
        label: 'book · ' + run.bookId,
        jobId: run.jobId,
        tab: 'book',
        state,
        startedAt: run.startedAt || Date.now(),
        detail: String(detail || ''),
    });
}

async function bookCompile() {
    const doc = _bookState.doc;
    if (!doc || !(doc.entries || []).length) { _bookStatus('No entries to compile', true); return; }
    if (_bookState.compile) { _bookStatus('Compile already running', true); return; }
    _bookCompileBtn(true);
    try {
        if (_bookState.dirty) await bookSave();
        const runId = 'bk_' + Date.now().toString(36) + Math.random().toString(36).slice(2, 6);
        const jobId = 'book#' + _bookState.activeId;
        const startedAt = Date.now();
        const jobs = _bookState.doc.entries.map(entry => ({
            op: 'prepare', job_id: jobId, task_id: `bookprep_${runId}_${entry.entry_id}`,
            book_id: _bookState.activeId, entry_id: entry.entry_id,
            source_job_id: entry.job_id, source_artifact_id: entry.artifact_id,
            source_image_key: entry.image_key,
        }));
        const disp = await lambdaPost('dispatch', { target: 'book_pdf', jobs, expected_keys: [] });
        if ((disp.fired || 0) !== jobs.length) throw new Error(`dispatch fired ${disp.fired}/${jobs.length}`);
        _bookState.compile = { runId, jobId, phase: 'prepare', expected: jobs.length,
                               bookId: _bookState.activeId, savedAt: _bookState.doc.saved_at,
                               startedAt };
        _bookRailUpsert('running', `preparing 0/${jobs.length}`);
        _bookStatus(`Preparing 0/${jobs.length}...`);
        _bookLog(`Compile ${runId}: ${jobs.length} prepare jobs dispatched`);
        _bookPollCompile();
    } catch (e) {
        _bookRailUpsert('failed', e.message);
        _bookState.compile = null;
        _bookCompileBtn(false);
        _bookStatus(e.message, true);
    }
}

async function _bookPollCompile() {
    const run = _bookState.compile;
    if (!run || run !== _bookState.compile) return;
    try {
        if (run.phase === 'prepare') {
            const check = await lambdaPost('storage', {
                job_id: run.jobId, task_prefix: `bookprep_${run.runId}`, expected: run.expected,
            }, '/check-status');
            if ((check.errors || 0) > 0) {
                const detail = (check.error_details || [])[0] || {};
                throw new Error(`prepare failed: ${detail.error_msg || 'unknown'}`);
            }
            _bookRailUpsert('running', `preparing ${check.done || 0}/${run.expected}`);
            _bookStatus(`Preparing ${check.done || 0}/${run.expected}...`);
            if ((check.done || 0) >= run.expected) {
                run.phase = 'compose';
                const composeTask = `bookcomp_${run.runId}`;
                run.composeTask = composeTask;
                await lambdaPost('dispatch', {
                    target: 'book_pdf',
                    jobs: [{
                        op: 'compose', job_id: run.jobId, task_id: composeTask,
                        book_id: run.bookId, compile_id: run.runId,
                        expected_saved_at: run.savedAt,
                    }],
                    expected_keys: [],
                });
                _bookRailUpsert('running', 'composing');
                _bookStatus('Composing (lualatex)...');
                _bookLog('Prepare complete; compose dispatched');
            }
        } else if (run.phase === 'compose') {
            const check = await lambdaPost('storage', {
                job_id: run.jobId, task_prefix: run.composeTask, expected: 1,
            }, '/check-status');
            if ((check.errors || 0) > 0) {
                const detail = (check.error_details || [])[0] || {};
                throw new Error(`compose failed: ${detail.error_msg || 'unknown'}`);
            }
            const rd = (check.results || [])[0] || {};
            if (rd.phase === 'done') {
                _bookRailUpsert('done', `${(rd.content_pages || '?')} pages`);
                _bookState.compile = null;
                _bookCompileBtn(false);
                _bookLog(`Compose done for ${run.bookId}`);
                if (run.bookId === _bookState.activeId) {
                    await _bookLoadActive();
                    _renderBookTab();
                    void _bookHydrateEntries();
                    _bookStatus(`Book compiled: ${(_bookState.latestOutput || {}).content_pages || '?'} pages`);
                }
                return;
            }
            _bookRailUpsert('running', rd.phase_label || rd.phase || 'composing');
            _bookStatus(`Composing: ${rd.phase_label || rd.phase || 'working'}...`);
        }
        setTimeout(_bookPollCompile, 3000);
    } catch (e) {
        _bookRailUpsert('failed', e.message);
        _bookState.compile = null;
        _bookCompileBtn(false);
        _bookStatus(e.message, true);
        _bookLog(`Compile failed: ${e.message}`, 'err');
    }
}

async function bookDownload(kind) {
    const out = _bookState.latestOutput;
    if (!out) return;
    const key = { cover: out.cover_key, content: out.content_key, source: out.source_key }[kind];
    if (!key) return;
    try {
        const resp = await lambdaPost('storage', { key, filename: `${_bookState.activeId}-${kind}.${kind === 'source' ? 'zip' : 'pdf'}` }, '/presign');
        window.location.href = resp.url;
    } catch (e) {
        _bookStatus(e.message, true);
    }
}

;(window.__ppParts = window.__ppParts || []).push('14-book');
