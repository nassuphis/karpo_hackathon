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
    describe: null,
    editingEntryId: '',
    dirty: false,
    hydrated: {},         // entry_id -> {preview_url, missing}
    selectedEntryId: '',
    subtab: 'content',    // 'content' | 'cover'
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
                ? { preview_url: match.preview_url || match.image_url || '', missing: false,
                    palette_preview_key: (match.associated_palette_preview_key
                                          || match.associated_palette_image_key || '').trim(),
                    palette_url: '' }
                : { preview_url: '', missing: true, palette_preview_key: '', palette_url: '' };
        }
    });
    // palette thumbnails: the bucket is public-read by design (deploy.sh
    // PublicReadSiteAssets on the whole bucket), so direct URLs — zero
    // presign round-trips. Same base the mosaics use.
    const s3Base = 'https://polypaint.s3.us-east-1.amazonaws.com/';
    for (const hyd of Object.values(_bookState.hydrated)) {
        if (hyd.palette_preview_key) hyd.palette_url = s3Base + hyd.palette_preview_key;
    }
    _renderBookTab();
}

function _bookEntryLabel(entry) {
    // described entries read by their artsy title; the artifact id stays
    // reachable via GoRender (and the … editor)
    return (entry.title_override || '').trim() || entry.display_name || entry.artifact_id;
}

function _bookEntryRow(entry, idx) {
    const hyd = _bookState.hydrated[entry.entry_id] || {};
    const selected = entry.entry_id === _bookState.selectedEntryId;
    const cover = _bookState.doc.cover_entry_id === entry.entry_id;
    const thumb = hyd.preview_url
        ? `<img src="${_escapeHtml(hyd.preview_url)}" style="width:48px;height:48px;object-fit:cover">`
        : `<div style="width:48px;height:48px;background:#222;display:flex;align-items:center;justify-content:center;font-size:9px;color:#888">${hyd.missing ? 'missing' : '...'}</div>`;
    // palette first, color render after; no palette = blank square
    const palThumb = hyd.palette_url
        ? `<img src="${_escapeHtml(hyd.palette_url)}" style="width:48px;height:48px;object-fit:cover">`
        : `<div style="width:48px;height:48px;background:#121829;border:1px solid #2b3a5e;box-sizing:border-box"></div>`;
    const title = _escapeHtml(_bookEntryLabel(entry));
    const eid = _escapeHtml(entry.entry_id);
    // the cover row's number turns red so it's visible at a glance
    const numColor = cover ? '#e94560' : '#666';
    const numWeight = cover ? 'font-weight:700;' : '';
    return `<div class="book-entry-row${selected ? ' selected' : ''}" data-entry="${eid}"
        style="display:flex;align-items:center;gap:8px;padding:4px 6px;border-bottom:1px solid #2b3a5e;cursor:pointer${selected ? ';background:#1c2742' : ''}"
        onclick="_bookSelectEntry(this.dataset.entry)">
        <span style="color:${numColor};width:24px;${numWeight}">${idx + 1}</span>${palThumb}${thumb}
        <span style="flex:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap">${title}${cover ? ' <span style="color:#e94560">— cover</span>' : ''}${hyd.missing ? ' <span style="color:#e94560">[missing]</span>' : ''}</span>
        <span style="color:#666;font-size:10px">${_escapeHtml(entry.job_id)}</span>
        <button class="btn-secondary" style="padding:1px 7px" onclick="event.stopPropagation();_bookMoveEntry(this.closest('.book-entry-row').dataset.entry,-1)">▲</button>
        <button class="btn-secondary" style="padding:1px 7px" onclick="event.stopPropagation();_bookMoveEntry(this.closest('.book-entry-row').dataset.entry,1)">▼</button>
        <button class="btn-secondary" style="padding:1px 7px" title="Edit title & text" onclick="event.stopPropagation();bookEditEntry(this.closest('.book-entry-row').dataset.entry)">…</button>
        <button class="btn-secondary" style="padding:1px 7px" onclick="event.stopPropagation();_bookRemoveEntry(this.closest('.book-entry-row').dataset.entry)">✕</button>
    </div>` + _bookEntryEditor(entry);
}

function _bookEntryEditor(entry) {
    if (_bookState.editingEntryId !== entry.entry_id) return '';
    const eid = _escapeHtml(entry.entry_id);
    return `<div class="book-entry-editor" style="padding:8px 10px 10px 38px;background:#141c33;border-bottom:1px solid #2b3a5e">
        <input id="book-edit-title" type="text" placeholder="Title (verso heading; empty = compute id)"
            value="${_escapeHtml(entry.title_override || '')}"
            style="width:100%;margin-bottom:6px;background:#0d1320;color:#f2f2f7;border:1px solid #2b3a5e;border-radius:4px;padding:5px 8px;font-size:12px">
        <textarea id="book-edit-body" rows="3" placeholder="Description (verso body; empty = none)"
            style="width:100%;background:#0d1320;color:#f2f2f7;border:1px solid #2b3a5e;border-radius:4px;padding:5px 8px;font-size:12px;resize:vertical">${_escapeHtml(entry.body_override || '')}</textarea>
        <div style="display:flex;gap:8px;margin-top:6px">
            <button class="btn-secondary" style="padding:2px 12px" onclick="void bookEditEntrySave('${eid}', this)">Save</button>
            <button class="btn-secondary" style="padding:2px 12px" onclick="bookEditEntryCancel()">Cancel</button>
            <button class="btn-secondary" style="padding:2px 12px" title="Blank this entry's title + description (saves immediately)" onclick="void bookEditEntryClear('${eid}', this)">Clear</button>
        </div>
    </div>`;
}

function bookEditEntry(entryId) {
    _bookState.editingEntryId = _bookState.editingEntryId === entryId ? '' : entryId;
    _renderBookTab();
}

function bookEditEntryCancel() {
    _bookState.editingEntryId = '';
    _renderBookTab();
}

async function bookEditEntryClear(entryId, btn) {
    const entry = (_bookState.doc?.entries || []).find(e => e.entry_id === entryId);
    if (!entry) return;
    const orig = btn ? btn.textContent : 'Clear';
    if (btn) { btn.disabled = true; btn.textContent = 'Clearing…'; }
    entry.title_override = '';
    entry.body_override = '';
    _bookState.dirty = true;
    if (await bookSave()) {
        _bookState.editingEntryId = '';
        _renderBookTab();
        _bookStatus('Entry text cleared — Describe fills blanks, DescribeSelection redoes this row');
    } else if (btn) {
        btn.disabled = false; btn.textContent = orig;   // save failed: keep editor open
    }
}

async function bookClearDescriptions(btn) {
    const doc = _bookState.doc;
    const entries = doc?.entries || [];
    if (!entries.length) { _bookStatus('No book loaded', true); return; }
    const n = entries.filter(e => (e.title_override || '').trim() || (e.body_override || '').trim()).length;
    if (!n) { _bookStatus('Nothing to clear — no entry has a title or description'); return; }
    if (!confirm(`Clear titles + descriptions on ${n} entr${n === 1 ? 'y' : 'ies'}? Saves immediately so Describe re-runs the whole book from scratch.`)) return;
    const orig = btn ? btn.textContent : '';
    if (btn) { btn.disabled = true; btn.textContent = 'Clearing…'; }
    try {
        for (const e of entries) { e.title_override = ''; e.body_override = ''; }
        _bookState.dirty = true;
        if (await bookSave()) {
            _renderBookTab();
            _bookStatus(`Cleared ${n} entries — pick a model in ⚙ and hit Describe to regenerate`);
        }   // bookSave already surfaced the error on failure
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

async function bookEditEntrySave(entryId, btn) {
    const entry = (_bookState.doc?.entries || []).find(e => e.entry_id === entryId);
    if (!entry) return;
    const orig = btn ? btn.textContent : 'Save';
    if (btn) { btn.disabled = true; btn.textContent = 'Saving…'; }
    entry.title_override = (document.getElementById('book-edit-title')?.value || '').trim();
    entry.body_override = (document.getElementById('book-edit-body')?.value || '').trim();
    _bookState.dirty = true;
    if (await bookSave()) {
        _bookState.editingEntryId = '';
        _renderBookTab();
        _bookStatus('Entry text saved — Compile to publish');
    } else if (btn) {
        btn.disabled = false; btn.textContent = orig;   // save failed: keep editor open, edits intact
    }
}

function bookSubtab(name) {
    _bookState.subtab = name === 'cover' ? 'cover' : 'content';
    document.getElementById('book-subtab-content')?.classList.toggle('active', _bookState.subtab === 'content');
    document.getElementById('book-subtab-cover')?.classList.toggle('active', _bookState.subtab === 'cover');
    const c = document.getElementById('book-sub-content');
    const v = document.getElementById('book-sub-cover');
    if (c) c.style.display = _bookState.subtab === 'content' ? 'block' : 'none';
    if (v) v.style.display = _bookState.subtab === 'cover' ? 'block' : 'none';
    _renderBookTab();
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
    // Cover sub-tab: preview of the selected cover artifact + book title fields
    const coverEntry = doc ? (doc.entries || []).find(e => e.entry_id === doc.cover_entry_id) : null;
    const coverPrev = document.getElementById('book-cover-preview');
    if (coverPrev) {
        const hyd = coverEntry ? (_bookState.hydrated[coverEntry.entry_id] || {}) : {};
        coverPrev.innerHTML = coverEntry && hyd.preview_url
            ? `<img src="${_escapeHtml(hyd.preview_url)}" style="width:100%;height:100%;object-fit:contain">`
            : (coverEntry ? '…' : 'No cover selected.<br>Pick a row in Content and press Cover.');
    }
    const hint = document.getElementById('book-cover-hint');
    if (hint) {
        hint.textContent = coverEntry
            ? `Cover: ${_bookEntryLabel(coverEntry)}`
            : (doc ? 'No cover chosen — the cover page will be typographic.' : '');
    }
    for (const [id, key] of [['book-title-input', 'title'], ['book-subtitle-input', 'subtitle'], ['book-author-input', 'author']]) {
        const el = document.getElementById(id);
        if (el && doc && el.value !== (doc[key] || '')) el.value = doc[key] || '';
    }
    const dl = document.getElementById('book-download-row');
    if (dl) dl.style.display = _bookState.latestOutput ? 'flex' : 'none';
    const out = _bookState.latestOutput;
    const outInfo = document.getElementById('book-output-info');
    if (outInfo && out) outInfo.textContent = `compiled ${out.compiled_at || ''}: ${out.content_pages} pages, ${out.spread_count} spreads`;
}

async function bookRefresh() {
    // full refresh: book list, the active book doc, and every thumbnail
    _bookState.listLoaded = false;
    _bookBtnBusy('btn-book-refresh', true, 'Refreshing…');
    try {
        await _bookRefreshList(true);
        await _bookLoadActive();
        _renderBookTab();
        if (_bookState.doc) await _bookHydrateEntries();
        _bookStatus('Refreshed');
    } catch (e) {
        _bookStatus(e.message, true);
    } finally {
        _bookBtnBusy('btn-book-refresh', false);
    }
}

async function loadBookTab() {
    try {
        await _bookRefreshList();
        if (_bookState.activeId && !_bookState.doc) {
            await _bookLoadActive();
        }
        _renderBookTab();
        if (_bookState.doc) void _bookHydrateEntries();
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
    _bookBtnBusy('btn-book-new', true, 'Creating…');
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
    } finally {
        _bookBtnBusy('btn-book-new', false);
    }
}

async function bookSave() {
    if (!_bookState.doc) return;
    const btn = document.getElementById('btn-book-save');
    const orig = btn ? btn.textContent : '';
    if (btn) { btn.disabled = true; btn.textContent = 'Saving…'; }
    try {
        const resp = await lambdaPost('storage', { book: _bookState.doc }, '/save-book');
        _bookState.doc = resp.book;
        _bookState.dirty = false;
        _bookState.listLoaded = false;
        await _bookRefreshList(true);
        _renderBookTab();
        _bookStatus('Saved');
        return true;   // callers that need persisted state gate on this (F12)
    } catch (e) {
        _bookStatus(e.message, true);
        return false;
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig || 'Save'; }
    }
}

async function bookDelete() {
    if (!_bookState.activeId) return;
    if (!confirm(`Delete book "${_bookState.activeId}" (doc + cached assets + outputs)?`)) return;
    _bookBtnBusy('btn-book-delete', true, 'Deleting…');
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
    } finally {
        _bookBtnBusy('btn-book-delete', false);
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
    const doc = _bookState.doc;
    if (!doc) { _bookStatus('No book loaded', true); return; }
    if (!_bookState.selectedEntryId) { _bookStatus('Select a row first, then press Cover', true); return; }
    const entry = (doc.entries || []).find(e => e.entry_id === _bookState.selectedEntryId);
    doc.cover_entry_id = _bookState.selectedEntryId;
    _bookState.dirty = true;
    _renderBookTab();
    _bookStatus(`Cover set to "${entry ? _bookEntryLabel(entry) : '?'}" (row turns red). Save to keep it.`);
}

async function bookGoRender() {
    const entry = (_bookState.doc || {}).entries?.find(e => e.entry_id === _bookState.selectedEntryId);
    if (!entry) { _bookStatus('Select a row first', true); return; }
    _bookBtnBusy('btn-book-go-render', true, 'Opening…');
    try {
        await _ensureResultsSelection(entry.job_id);
        switchTab('render');
        await refreshRenderArtifacts(entry.job_id, {
            selectFamily: 'color',
            selectArtifactId: entry.artifact_id || null,
        });
    } catch (e) {
        _bookStatus(`GoRender failed: ${e.message}`, true);
    } finally {
        _bookBtnBusy('btn-book-go-render', false);
    }
}

function bookMetaChanged() {
    const doc = _bookState.doc;
    if (!doc) return;
    doc.title = document.getElementById('book-title-input')?.value || '';
    doc.subtitle = document.getElementById('book-subtitle-input')?.value || '';
    doc.author = document.getElementById('book-author-input')?.value || '';
    _bookState.dirty = true;
    const info = document.getElementById('book-info');
    if (info && !info.textContent.includes('(unsaved)') && doc) {
        info.textContent = `${(doc.entries || []).length} entries (unsaved)`;
    }
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
        const candidate = {
            job_id: ref.jobId, artifact_id: ref.artifactId, image_key: ref.imageKey,
            display_name: ref.displayName || ref.artifactId,
            added_at: new Date().toISOString().slice(0, 19) + 'Z',
            title_override: '', body_override: '',
        };
        doc.entries.push(candidate);
        let resp;
        try {
            resp = await lambdaPost('storage', { book: doc }, '/save-book');
        } catch (saveErr) {
            // roll the optimistically-pushed entry back out so a failed add
            // can't leave a phantom that a later save publishes (F12)
            const at = doc.entries.indexOf(candidate);
            if (at !== -1) doc.entries.splice(at, 1);
            throw saveErr;
        }
        _bookState.doc = resp.book;
        _bookState.dirty = false;
        report(`Added entry ${resp.book.entries.length} to "${resp.book.name}"`);
        _renderBookTab();
        void _bookHydrateEntries();
        return true;
    } catch (e) {
        report(`Add to Book failed: ${e.message}`, true);
        return false;
    }
}

async function _bookAddWithButton(btnId, statusElId, ref) {
    const btn = document.getElementById(btnId);
    const orig = btn ? btn.textContent : '';
    if (btn) { btn.disabled = true; btn.textContent = 'Adding…'; }
    const ok = await _bookAddEntry(ref, (msg, err) => {
        const el = document.getElementById(statusElId);
        if (el) { el.textContent = msg; el.className = err ? 'status error' : 'status'; }
    });
    if (btn) {
        btn.textContent = ok ? 'Added ✓' : 'Failed';
        setTimeout(() => { btn.disabled = false; btn.textContent = orig; }, 1500);
    }
    return ok;
}

function addSelectedRenderArtifactToBook() {
    if (_renderActiveFamily !== 'color') return;
    const art = _renderSelectedArtifactEntry();
    const jobId = document.getElementById('render-results-dir').value.trim();
    if (!art || !jobId) return;
    void _bookAddWithButton('btn-render-add-book', 'render-status',
        { jobId, artifactId: art.artifact_id, imageKey: art.image_key || '', displayName: art.display_name || art.artifact_id });
}

function addSelectedFavoriteToBook() {
    const art = _favoriteArtifacts[_favoriteSelectedIdx];
    if (!art || art.missing) return;
    void _bookAddWithButton('btn-favorites-add-book', 'favorites-status',
        { jobId: art.favorite_job_id, artifactId: art.artifact_id, imageKey: art.image_key || '', displayName: art.display_name || art.artifact_id });
}

// --- compile (design §5): prepare fan-out, then compose. Trigger is
// done >= N && errors === 0 — /check-status "complete" counts errored
// tasks as terminal, so it must never gate compose on its own. ---
function _bookBtnBusy(id, busy, busyLabel) {
    const b = document.getElementById(id);
    if (!b) return;
    if (busy) { b.dataset.orig = b.textContent; b.disabled = true; b.textContent = busyLabel; }
    else { b.disabled = false; b.textContent = b.dataset.orig || b.textContent; }
}

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
        // never compile an unsaved book: a failed save would publish the old
        // S3 state while the UI shows the new one (code-review-26 F12)
        if (_bookState.dirty && !(await bookSave())) {
            _bookStatus('Not compiling: save failed — fix the error and retry', true);
            return;
        }
        const runId = 'bk_' + Date.now().toString(36) + Math.random().toString(36).slice(2, 6);
        const jobId = 'book#' + _bookState.activeId;
        const startedAt = Date.now();
        const jobs = _bookState.doc.entries.map(entry => ({
            op: 'prepare', job_id: jobId, task_id: `bookprep_${runId}_${entry.entry_id}`,
            book_id: _bookState.activeId, entry_id: entry.entry_id,
            source_job_id: entry.job_id, source_artifact_id: entry.artifact_id,
            source_image_key: entry.image_key,
            force: true,   // Compile = fresh: re-prepare every entry, never reuse stale assets
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
            const prepElapsed = Math.round((Date.now() - run.startedAt) / 1000);
            _bookRailUpsert('running', `preparing ${check.done || 0}/${run.expected} · ${prepElapsed}s`);
            _bookStatus(`Preparing ${check.done || 0}/${run.expected}... (${prepElapsed}s)`);
            if ((check.done || 0) !== run.lastPrepDone) {
                run.lastPrepDone = check.done || 0;
                _bookLog(`Prepared ${run.lastPrepDone}/${run.expected} entries (${prepElapsed}s)`);
            }
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
                const doneElapsed = Math.round((Date.now() - run.startedAt) / 1000);
                _bookRailUpsert('done', `${(rd.content_pages || '?')} pages · ${doneElapsed}s`);
                _bookState.compile = null;
                _bookCompileBtn(false);
                _bookLog(`Compose done for ${run.bookId}: ${rd.content_pages || '?'} pages in ${doneElapsed}s`);
                if (rd.flip_error) {
                    _bookLog(`Flipbook FAILED: ${rd.flip_error} — PDF is fine; recompile to retry`, 'err');
                    _bookStatus(`Compiled, but flipbook failed: ${rd.flip_error}`, true);
                } else if (rd.flip_page_count) {
                    _bookLog(`Flipbook ready: ${rd.flip_page_count} pages`);
                }
                if (run.bookId === _bookState.activeId) {
                    await _bookLoadActive();
                    _renderBookTab();
                    void _bookHydrateEntries();
                    _bookStatus(`Book compiled: ${(_bookState.latestOutput || {}).content_pages || '?'} pages`);
                }
                return;
            }
            const label = rd.phase_label || rd.phase || 'composing';
            const compElapsed = Math.round((Date.now() - run.startedAt) / 1000);
            _bookRailUpsert('running', `${label} · ${compElapsed}s`);
            _bookStatus(`Composing: ${label}... (${compElapsed}s)`);
            if (label !== run.lastComposeLabel) {
                run.lastComposeLabel = label;
                _bookLog(`Compose: ${label} (${compElapsed}s)`);
            }
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

function bookVisionModelChanged() {
    const sel = document.getElementById('book-vision-model');
    const custom = document.getElementById('book-vision-model-custom');
    if (custom) custom.style.display = sel && sel.value === '__custom__' ? 'inline-block' : 'none';
}

function _bookVisionSelectedModel() {
    const sel = document.getElementById('book-vision-model');
    if (!sel) return '';
    if (sel.value === '__custom__') {
        return (document.getElementById('book-vision-model-custom')?.value || '').trim();
    }
    return sel.value;
}

function _bookVisionShowModel(model) {
    const sel = document.getElementById('book-vision-model');
    const custom = document.getElementById('book-vision-model-custom');
    if (!sel) return;
    const known = Array.from(sel.options).some(o => o.value === model);
    if (model && !known) {
        sel.value = '__custom__';
        if (custom) { custom.value = model; custom.style.display = 'inline-block'; }
    } else if (model) {
        sel.value = model;
        if (custom) custom.style.display = 'none';
    }
}

function _bookVisionStatusText(cfg) {
    const provs = cfg.providers || {};
    const marks = ['gemini', 'anthropic', 'openai']
        .map(p => `${p} ${provs[p]?.key_set ? '✓' + (provs[p].key_hint || '') : '—'}`)
        .join(' · ');
    return `model ${cfg.model || 'gemini-2.5-flash (default)'} · keys: ${marks}`;
}

function bookVisionToggle() {
    const panel = document.getElementById('book-vision-panel');
    if (!panel) return;
    const opening = panel.style.display === 'none';
    panel.style.display = opening ? 'flex' : 'none';
    if (opening) void bookVisionLoad();
}

async function bookVisionLoad() {
    const status = document.getElementById('book-vision-status');
    try {
        const cfg = await lambdaPost('storage', {}, '/fetch-vision-config');
        _bookVisionShowModel(cfg.model || '');
        if (status) status.textContent = _bookVisionStatusText(cfg);
    } catch (e) {
        if (status) status.textContent = e.message;
    }
}

async function bookVisionSave(btn) {
    const orig = btn ? btn.textContent : 'Save Vision';
    if (btn) { btn.disabled = true; btn.textContent = 'Saving…'; }
    try {
        const model = _bookVisionSelectedModel();
        if (!model) throw new Error('pick or type a model id');
        const keyEl = document.getElementById('book-vision-key');
        const cfg = await lambdaPost('storage', { model, api_key: (keyEl?.value || '').trim() }, '/save-vision-config');
        if (keyEl) keyEl.value = '';
        const status = document.getElementById('book-vision-status');
        if (status) status.textContent = 'saved · ' + _bookVisionStatusText(cfg);
        if (btn) btn.textContent = 'Saved ✓';
    } catch (e) {
        _bookStatus(e.message, true);
        if (btn) btn.textContent = 'Failed';
    } finally {
        setTimeout(() => { if (btn) { btn.disabled = false; btn.textContent = orig; } }, 1500);
    }
}

async function bookDescribe(btn) {
    // all entries, skip-existing (hand prose survives)
    return _bookDescribeRun(btn, {});
}

async function bookDescribeSelection(btn) {
    // the selected row only — explicit intent, so regenerate even if it
    // already has prose
    const entry = (_bookState.doc?.entries || []).find(e => e.entry_id === _bookState.selectedEntryId);
    if (!entry) { _bookStatus('Select a row first', true); return; }
    return _bookDescribeRun(btn, { entry_ids: [entry.entry_id], overwrite: true });
}

async function _bookDescribeRun(btn, extra) {
    // server-side Gemini titles+descriptions (lambda/book_describe.py):
    // dispatch -> phase polling -> rail card, same shape as Compile
    const doc = _bookState.doc;
    if (!doc || !(doc.entries || []).length) { _bookStatus('No entries to describe', true); return; }
    if (_bookState.describe) { _bookStatus('Describe already running', true); return; }
    const orig = btn ? btn.textContent : 'Describe';
    if (btn) { btn.disabled = true; btn.textContent = 'Describing…'; }
    try {
        // describe operates on the SAVED book (expected_saved_at below); a
        // failed save must not let describe run on stale S3 state (F12)
        if (_bookState.dirty && !(await bookSave())) {
            _bookStatus('Not describing: save failed — fix the error and retry', true);
            if (btn) { btn.disabled = false; btn.textContent = orig; }
            return;
        }
        const runId = 'bd_' + Date.now().toString(36) + Math.random().toString(36).slice(2, 6);
        const jobId = 'book#' + _bookState.activeId;
        const taskId = `bookdesc_${runId}`;
        await lambdaPost('dispatch', { target: 'book_pdf', jobs: [{
            op: 'describe', job_id: jobId, task_id: taskId,
            book_id: _bookState.activeId, expected_saved_at: _bookState.doc.saved_at,
            ...extra,
        }], expected_keys: [] });
        _bookState.describe = { runId, jobId, taskId, startedAt: Date.now(), btnOrig: orig, btn };
        _bookRailDescribe('running', 'dispatched');
        _bookLog(`Describe ${runId}: dispatched for ${doc.entries.length} entries`);
        _bookStatus('Describing…');
        _bookPollDescribe();
    } catch (e) {
        _bookState.describe = null;
        if (btn) { btn.disabled = false; btn.textContent = orig; }
        _bookStatus(e.message, true);
    }
}

function _bookRailDescribe(state, detail) {
    const run = _bookState.describe;
    if (!run || typeof _jobsRailUpsert !== 'function') return;
    _jobsRailUpsert({
        id: 'bookdesc:' + run.runId, kind: 'book',
        label: 'describe · ' + _bookState.activeId,
        jobId: run.jobId, tab: 'book', state,
        startedAt: run.startedAt, detail: String(detail || ''),
    });
}

async function _bookPollDescribe() {
    const run = _bookState.describe;
    if (!run) return;
    const finish = () => {
        if (run.btn) { run.btn.disabled = false; run.btn.textContent = run.btnOrig; }
        _bookState.describe = null;
    };
    try {
        const check = await lambdaPost('storage', {
            job_id: run.jobId, task_prefix: run.taskId, expected: 1,
        }, '/check-status');
        if ((check.errors || 0) > 0) {
            const detail = (check.error_details || [])[0] || {};
            throw new Error(detail.error_msg || 'describe failed');
        }
        const rd = (check.results || [])[0] || {};
        const label = rd.phase_label || rd.phase || 'working';
        const elapsed = Math.round((Date.now() - run.startedAt) / 1000);
        if (rd.phase === 'done') {
            const failed = rd.failed || 0;
            _bookRailDescribe('done', `${rd.described || 0} described${failed ? ` · ${failed} failed` : ''} · ${elapsed}s`);
            _bookLog(`Describe done: ${rd.described || 0} described, ${rd.skipped || 0} skipped${failed ? `, ${failed} FAILED (${rd.first_error || '?'})` : ''} (${elapsed}s)`);
            finish();
            // reload drops the thumbnail cache + selection — rehydrate and
            // keep the row selected (DescribeSelection iterates on one row)
            const keepSelected = _bookState.selectedEntryId;
            await _bookLoadActive();
            if (keepSelected && (_bookState.doc?.entries || []).some(e => e.entry_id === keepSelected)) {
                _bookState.selectedEntryId = keepSelected;
            }
            _renderBookTab();
            if (_bookState.doc) void _bookHydrateEntries();
            if (failed) {
                _bookStatus(`Described ${rd.described || 0}, ${failed} failed — hit Describe again to retry the failures`, true);
            } else {
                _bookStatus(`Described ${rd.described || 0} entries — Compile to publish`);
            }
            return;
        }
        _bookRailDescribe('running', `${label} · ${elapsed}s`);
        if (label !== run.lastLabel) {
            run.lastLabel = label;
            _bookLog(`Describe: ${label} (${elapsed}s)`);
        }
        _bookStatus(`Describe: ${label}… (${elapsed}s)`);
        setTimeout(_bookPollDescribe, 3000);
    } catch (e) {
        _bookRailDescribe('failed', e.message);
        _bookLog(`Describe failed: ${e.message}`, 'err');
        finish();
        _bookStatus(e.message, true);
    }
}

function bookOpenFlipbook(btn) {
    const out = _bookState.latestOutput;
    const orig = btn ? btn.textContent : '';
    if (!out || !out.flip_key) {
        // older compiles predate the flipbook; flip_error carries the cause
        const why = out && out.flip_error ? `flipbook failed: ${out.flip_error}` : 'recompile to generate the flipbook';
        _bookStatus(`No flipbook for this compile — ${why}`, true);
        if (btn) {
            btn.textContent = 'Recompile first';
            setTimeout(() => { btn.textContent = orig; }, 1800);
        }
        return;
    }
    if (btn) {
        btn.textContent = 'Opening…';
        setTimeout(() => { btn.textContent = orig; }, 1500);
    }
    const url = `flipbook.html?book=${encodeURIComponent(_bookState.activeId)}`;
    const opened = window.open(url, '_blank');
    try { if (opened) opened.opener = null; } catch (e) {}
    _bookStatus('Flipbook opened in a new tab.');
}

async function bookDownload(kind, btn) {
    const out = _bookState.latestOutput;
    if (!out) return;
    const key = { cover: out.cover_key, content: out.content_key, source: out.source_key }[kind];
    if (!key) return;
    const orig = btn ? btn.textContent : '';
    if (btn) { btn.disabled = true; btn.textContent = 'Preparing…'; }
    _bookStatus(`Preparing ${kind} download…`);
    try {
        const resp = await lambdaPost('storage', { key, filename: `${_bookState.activeId}-${kind}.${kind === 'source' ? 'zip' : 'pdf'}` }, '/presign');
        window.location.href = resp.url;
        _bookStatus(`Downloading ${kind}…`);
    } catch (e) {
        _bookStatus(e.message, true);
    } finally {
        if (btn) { btn.disabled = false; btn.textContent = orig; }
    }
}

;(window.__ppParts = window.__ppParts || []).push('14-book');
