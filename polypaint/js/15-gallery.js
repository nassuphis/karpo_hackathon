// js/15-gallery.js — the Gallery tab: create + curate editable galleries.
//
// Mirrors the Book tab (js/14): a selector of galleries, a piece list you
// reorder + retitle + remove, Save (ETag CAS), and Open Gallery (snapshot ->
// standalone 3D viewer). The active gallery id (localStorage
// 'polypaint_active_gallery') is what the DeepZoom tab's "Add to Gallery"
// appends to. This tab CREATES and CURATES; the DeepZoom tab only ADDS.

const GALLERY_ACTIVE_KEY = 'polypaint_active_gallery';

let _galleryState = {
    galleries: [],          // list rows {gallery_id, name, count, updated_at}
    listLoaded: false,
    activeId: (function () { try { return localStorage.getItem(GALLERY_ACTIVE_KEY) || ''; } catch (e) { return ''; } })(),
    doc: null,              // the loaded editable gallery (source of truth while editing)
    revision: '',           // ETag CAS token from fetch/save
    dirty: false,           // unsaved reorder/title/name edits
};

// Read by the DeepZoom tab's "Add to Gallery" (js/12). Reads localStorage (the
// persistent source of truth) so it is correct even before this tab has loaded.
function _galleryActiveId() {
    try { return String(localStorage.getItem(GALLERY_ACTIVE_KEY) || ''); } catch (e) { return _galleryState.activeId || ''; }
}

function _gallerySetActive(id) {
    _galleryState.activeId = id || '';
    try { localStorage.setItem(GALLERY_ACTIVE_KEY, _galleryState.activeId); } catch (e) {}
}

function _galleryStatus(msg, isError) {
    const el = document.getElementById('gallery-status');
    if (el) { el.textContent = msg || ''; el.className = 'status' + (isError ? ' error' : ''); }
}

// ── app-styled modal (replaces native prompt/confirm) ───────────────────────
// Promise-based: resolves to the trimmed input string (name prompt) or true
// (confirm) on OK, or null on Cancel/Escape/Close.
let _galleryModalResolve = null;

function _galleryModal(opts) {
    return new Promise((resolve) => {
        if (_galleryModalResolve) _galleryModalResolve(null);   // supersede any open one
        _galleryModalResolve = resolve;
        const overlay = document.getElementById('gallery-modal-overlay');
        const hasInput = opts.input !== false;
        document.getElementById('gallery-modal-title').textContent = opts.title || '';
        const labelEl = document.getElementById('gallery-modal-label');
        const inputEl = document.getElementById('gallery-modal-input');
        labelEl.style.display = hasInput ? 'block' : 'none';
        inputEl.style.display = hasInput ? 'block' : 'none';
        labelEl.textContent = opts.label || 'Name';
        inputEl.value = opts.value || '';
        document.getElementById('gallery-modal-message').textContent = opts.message || '';
        document.getElementById('gallery-modal-ok').textContent = opts.okLabel || 'OK';
        overlay.style.display = 'flex';
        overlay.setAttribute('aria-hidden', 'false');
        if (hasInput) { inputEl.focus(); inputEl.select(); }
        else document.getElementById('gallery-modal-ok').focus();
    });
}

function _galleryModalClose(result) {
    const overlay = document.getElementById('gallery-modal-overlay');
    if (overlay) { overlay.style.display = 'none'; overlay.setAttribute('aria-hidden', 'true'); }
    const r = _galleryModalResolve; _galleryModalResolve = null;
    if (r) r(result);
}

function _galleryModalConfirm() {
    const inputEl = document.getElementById('gallery-modal-input');
    const hasInput = inputEl && inputEl.style.display !== 'none';
    _galleryModalClose(hasInput ? String(inputEl.value || '').trim() : true);
}

function _galleryModalCancel() { _galleryModalClose(null); }

function _galleryModalKey(e) {
    if (e.key === 'Enter') { e.preventDefault(); _galleryModalConfirm(); }
    else if (e.key === 'Escape') { e.preventDefault(); _galleryModalCancel(); }
}

// The DeepZoom tab calls this after an add so the Gallery tab reflects it live —
// UNLESS there are unsaved edits, which must never be silently discarded.
function _galleryNotifyChanged(galleryId) {
    if (!galleryId || galleryId !== _galleryState.activeId) return;
    if (_galleryState.dirty) {
        _galleryStatus('A piece was added on the server. Save to keep your changes, or Refresh to reload.');
        return;
    }
    void _galleryLoadActive().then(_renderGalleryTab);
}

async function loadGalleryTab() {
    // Re-sync the active id from localStorage (it may have been set before this
    // module loaded, or changed in another browser tab).
    try { _galleryState.activeId = String(localStorage.getItem(GALLERY_ACTIVE_KEY) || ''); } catch (e) {}
    await _galleryRefreshList();
    // Preserve in-progress edits across tab switches: only refetch (which clears
    // dirty) when there are no unsaved changes for the active gallery.
    const keepDirty = _galleryState.dirty && _galleryState.doc &&
        _galleryState.doc.gallery_id === _galleryState.activeId;
    if (!keepDirty) await _galleryLoadActive();
    _renderGalleryTab();
}

async function _galleryRefreshList() {
    try {
        const resp = await lambdaPost('storage', {}, '/list-galleries');
        if (resp && resp.error) throw new Error(resp.error);
        _galleryState.galleries = (resp && resp.galleries) || [];
        _galleryState.listLoaded = true;
    } catch (e) {
        _galleryStatus('Could not list galleries: ' + e.message, true);
    }
}

async function _galleryLoadActive() {
    const id = _galleryState.activeId;
    if (!id) { _galleryState.doc = null; _galleryState.revision = ''; _galleryState.dirty = false; return; }
    try {
        const resp = await lambdaPost('storage', { gallery_id: id }, '/fetch-gallery');
        if (resp && resp.error) throw new Error(resp.error);
        _galleryState.doc = resp.gallery;
        _galleryState.revision = resp.revision || '';
        _galleryState.dirty = false;
    } catch (e) {
        // The active gallery is gone (deleted elsewhere): clear the selection.
        _galleryState.doc = null; _galleryState.revision = ''; _galleryState.dirty = false;
        _gallerySetActive('');
        _galleryStatus('Active gallery not found — pick or create another.', true);
    }
}

async function galleryNew() {
    if (_galleryState.dirty) {
        const ok = await _galleryModal({ title: 'Discard changes?', input: false, okLabel: 'Discard',
            message: 'You have unsaved changes to this gallery. Create a new one and discard them?' });
        if (!ok) return;
    }
    const name = await _galleryModal({ title: 'New gallery', label: 'Gallery name', value: 'Untitled gallery', okLabel: 'Create' });
    if (name === null) return;
    _galleryBtnBusy('btn-gallery-new', true, 'Creating…');
    try {
        const resp = await lambdaPost('storage', { name: name || 'Untitled gallery' }, '/create-gallery', { idempotent: false });
        if (resp && resp.error) throw new Error(resp.error);
        _gallerySetActive(resp.gallery.gallery_id);
        _galleryState.doc = resp.gallery;
        _galleryState.revision = resp.revision || '';
        _galleryState.dirty = false;
        await _galleryRefreshList();
        _galleryStatus('Created “' + resp.gallery.name + '”. Add color renders from the DeepZoom tab.');
    } catch (e) {
        _galleryStatus('Create failed: ' + e.message, true);
    } finally {
        _galleryBtnBusy('btn-gallery-new', false);
        _renderGalleryTab();
    }
}

async function gallerySelectorChanged() {
    const sel = document.getElementById('gallery-selector');
    const newId = sel ? sel.value : '';
    if (newId === _galleryState.activeId) return;
    if (_galleryState.dirty) {
        const ok = await _galleryModal({ title: 'Discard changes?', input: false, okLabel: 'Discard',
            message: 'Switch gallery and discard your unsaved changes?' });
        if (!ok) { if (sel) sel.value = _galleryState.activeId; return; }   // revert the visual selection
    }
    _gallerySetActive(newId);
    await _galleryLoadActive();
    _renderGalleryTab();
}

async function galleryDelete() {
    const id = _galleryState.activeId;
    if (!id) return;
    const name = (_galleryState.doc && _galleryState.doc.name) || 'this gallery';
    const ok = await _galleryModal({ title: 'Delete gallery', input: false, okLabel: 'Delete',
        message: 'Delete “' + name + '”? This cannot be undone.' });
    if (!ok) return;
    _galleryBtnBusy('btn-gallery-delete', true, 'Deleting…');
    try {
        const resp = await lambdaPost('storage', { gallery_id: id }, '/delete-gallery');
        if (resp && resp.error) throw new Error(resp.error);
        _gallerySetActive('');
        _galleryState.doc = null; _galleryState.revision = ''; _galleryState.dirty = false;
        await _galleryRefreshList();
        _galleryStatus('Gallery deleted.');
    } catch (e) {
        _galleryStatus('Delete failed: ' + e.message, true);
    } finally {
        _galleryBtnBusy('btn-gallery-delete', false);
        _renderGalleryTab();
    }
}

// Persist the current doc (CAS via revision). Throws on failure so callers can
// abort (e.g. Open won't snapshot a gallery whose save just failed).
async function _gallerySavePersist() {
    const resp = await lambdaPost('storage',
        { gallery: _galleryState.doc, expected_revision: _galleryState.revision }, '/save-gallery');
    if (resp && resp.error) throw new Error(resp.error);
    _galleryState.doc = resp.gallery;
    _galleryState.revision = resp.revision || '';
    _galleryState.dirty = false;
    return resp;
}

async function gallerySave() {
    if (!_galleryState.doc) return;
    _galleryBtnBusy('btn-gallery-save', true, 'Saving…');
    try {
        await _gallerySavePersist();
        await _galleryRefreshList();
        _galleryStatus('Saved.');
    } catch (e) {
        // Only a genuine CONFLICT (409) means the server moved under us — reload.
        // Any other failure (network, 5xx) keeps the local edits so nothing is lost.
        if (/HTTP 409|conflict/i.test(e.message || '')) {
            _galleryStatus('This gallery changed elsewhere — reloaded the latest; re-apply your edits.', true);
            await _galleryLoadActive();
        } else {
            _galleryStatus('Save failed: ' + e.message + ' — your edits are kept, try again.', true);
        }
    } finally {
        _galleryBtnBusy('btn-gallery-save', false);
        _renderGalleryTab();
    }
}

async function galleryOpen() {
    const doc = _galleryState.doc;
    if (!doc || !(doc.pieces || []).length) { _galleryStatus('Add pieces before opening.', true); return; }
    _galleryBtnBusy('btn-gallery-open', true, 'Opening…');
    // Open a blank window synchronously (popup-safe) and navigate after the POST.
    const win = window.open('', '_blank');
    try {
        // Persist any pending edits first so the snapshot matches what is shown.
        if (_galleryState.dirty) await _gallerySavePersist();
        // Pin the share to the reviewed revision (the server refuses if it moved).
        const resp = await lambdaPost('storage',
            { gallery_id: _galleryState.activeId, expected_revision: _galleryState.revision }, '/create-gallery-share');
        if (resp && resp.error) throw new Error(resp.error);
        const manifestUrl = String((resp && resp.manifest_url) || '');
        if (!manifestUrl) throw new Error('no manifest_url returned');
        // Derive gallery.html from the MANIFEST origin (the HTTPS REST endpoint).
        const galleryUrl = new URL('/gallery.html', new URL(manifestUrl).origin);
        galleryUrl.searchParams.set('manifest', manifestUrl);
        if (win) win.location = galleryUrl.toString();
        else if (typeof _copyTextToClipboard === 'function') {
            await _copyTextToClipboard(galleryUrl.toString());
            _galleryStatus('Link copied (popup blocked).');
        }
        _galleryStatus('Opened gallery — ' + resp.count + ' piece' + (resp.count === 1 ? '' : 's') + '.');
    } catch (e) {
        if (win) win.close();
        _galleryStatus('Open failed: ' + e.message, true);
    } finally {
        _galleryBtnBusy('btn-gallery-open', false);
        _renderGalleryTab();
    }
}

// ── in-memory edits (persisted on Save) ─────────────────────────────────────
function _galleryOnNameInput(value) {
    if (!_galleryState.doc) return;
    _galleryState.doc.name = value;
    _galleryState.dirty = true;
    _galleryUpdateActionButtons();
}

// Scene settings (wall colour + sky) live on the gallery doc and snapshot into
// the share manifest the viewer reads.
function _gallerySettings() {
    const doc = _galleryState.doc;
    if (!doc) return null;
    if (!doc.settings || typeof doc.settings !== 'object') doc.settings = { sky: 'stars', wall_color: '#ece4d6' };
    return doc.settings;
}

function _galleryOnWallColor(value) {
    const s = _gallerySettings(); if (!s) return;
    const hex = String(value || '').toLowerCase();
    if (!/^#[0-9a-f]{6}$/.test(hex)) return;
    s.wall_color = hex;
    const hexInp = document.getElementById('gallery-wall-hex'); if (hexInp) hexInp.value = hex;
    _galleryState.dirty = true; _galleryUpdateActionButtons();
}

function _galleryOnWallHex(value) {
    const s = _gallerySettings(); if (!s) return;
    let hex = String(value || '').trim().toLowerCase();
    if (hex && hex[0] !== '#') hex = '#' + hex;
    if (!/^#[0-9a-f]{6}$/.test(hex)) return;   // wait for a complete 6-digit value
    s.wall_color = hex;
    const picker = document.getElementById('gallery-wall-color'); if (picker) picker.value = hex;
    _galleryState.dirty = true; _galleryUpdateActionButtons();
}

function _galleryOnSkyToggle(on) {
    const s = _gallerySettings(); if (!s) return;
    s.sky = on ? 'stars' : 'dark';
    _galleryState.dirty = true; _galleryUpdateActionButtons();
}

function _galleryRenderSceneControls() {
    const doc = _galleryState.doc;
    const s = (doc && doc.settings) || null;
    const wc = document.getElementById('gallery-wall-color');
    const wh = document.getElementById('gallery-wall-hex');
    const sk = document.getElementById('gallery-sky-stars');
    const color = (s && s.wall_color) || '#ece4d6';
    if (wc) { wc.disabled = !doc; wc.value = color; }
    if (wh) { wh.disabled = !doc; wh.value = color; }
    if (sk) { sk.disabled = !doc; sk.checked = !s || s.sky !== 'dark'; }
}

function _galleryMovePiece(index, dir) {
    const pieces = _galleryState.doc && _galleryState.doc.pieces;
    if (!pieces) return;
    const j = index + dir;
    if (j < 0 || j >= pieces.length) return;
    const tmp = pieces[index]; pieces[index] = pieces[j]; pieces[j] = tmp;
    _galleryState.dirty = true;
    _renderGalleryTab();
}

function _galleryRemovePiece(index) {
    const pieces = _galleryState.doc && _galleryState.doc.pieces;
    if (!pieces || !pieces[index]) return;
    pieces.splice(index, 1);
    _galleryState.dirty = true;
    _renderGalleryTab();
}

function _gallerySetTitle(index, value) {
    const pieces = _galleryState.doc && _galleryState.doc.pieces;
    if (!pieces || !pieces[index]) return;
    pieces[index].title = value;
    _galleryState.dirty = true;
    _galleryUpdateActionButtons();   // no re-render: keep input focus
}

// ── rendering ────────────────────────────────────────────────────────────
function _galleryUpdateActionButtons() {
    const doc = _galleryState.doc;
    const pieces = (doc && doc.pieces) || [];
    const saveBtn = document.getElementById('btn-gallery-save');
    const openBtn = document.getElementById('btn-gallery-open');
    const delBtn = document.getElementById('btn-gallery-delete');
    const nameInp = document.getElementById('gallery-name');
    // Save is enabled whenever a gallery is open (no "why is Save grey?" mystery);
    // a clean re-save is a harmless no-op. Open auto-saves pending edits, so it is
    // enabled as soon as there are pieces.
    if (saveBtn) saveBtn.disabled = !doc;
    if (openBtn) openBtn.disabled = !doc || !pieces.length;
    if (delBtn) delBtn.disabled = !_galleryState.activeId;
    if (nameInp) nameInp.disabled = !doc;
}

// Immediate button feedback for async actions (mirrors the Book tab's
// _bookBtnBusy): disable + relabel to a "…ing" verb, restore on completion.
function _galleryBtnBusy(id, busy, busyLabel) {
    const b = document.getElementById(id);
    if (!b) return;
    if (busy) { b.dataset.orig = b.textContent; b.disabled = true; b.textContent = busyLabel; }
    else { b.disabled = false; if (b.dataset.orig) b.textContent = b.dataset.orig; }
}

function _renderGallerySelector() {
    const sel = document.getElementById('gallery-selector');
    if (!sel) return;
    sel.innerHTML = '';
    const list = _galleryState.galleries.slice();
    // Ensure the active gallery appears even if the list is stale.
    if (_galleryState.activeId && !list.some((g) => g.gallery_id === _galleryState.activeId)) {
        const name = (_galleryState.doc && _galleryState.doc.name) || _galleryState.activeId;
        list.unshift({ gallery_id: _galleryState.activeId, name, count: (_galleryState.doc && (_galleryState.doc.pieces || []).length) || 0 });
    }
    // An explicit blank placeholder whenever nothing is active, so the browser
    // does not visually "select" the first gallery while app state has none —
    // which made Add-to-Gallery say "no active gallery" (finding 11).
    if (!_galleryState.activeId) {
        const opt = document.createElement('option');
        opt.value = '';
        opt.textContent = list.length ? '— select a gallery —' : '— no galleries yet —';
        opt.selected = true;
        sel.appendChild(opt);
    }
    for (const g of list) {
        const opt = document.createElement('option');
        opt.value = g.gallery_id;
        opt.textContent = (g.name || g.gallery_id) + ' (' + (g.count || 0) + ')';
        if (g.gallery_id === _galleryState.activeId) opt.selected = true;
        sel.appendChild(opt);
    }
}

function _renderGalleryPieceRow(piece, index, total) {
    const row = document.createElement('div');
    row.style.cssText = 'display:flex; gap:10px; align-items:center; padding:6px; background:#121829; border:1px solid #2b3a5e; border-radius:6px';

    const img = document.createElement('img');
    img.src = (typeof _publicStorageUrl === 'function' ? _publicStorageUrl(piece.preview_key) : '');
    img.alt = '';
    img.style.cssText = 'width:52px; height:52px; object-fit:cover; background:#000; border-radius:4px; flex:0 0 auto';
    img.loading = 'lazy';
    row.appendChild(img);

    const meta = document.createElement('div');
    meta.style.cssText = 'flex:1 1 auto; min-width:0; display:flex; flex-direction:column; gap:4px';
    const id = document.createElement('div');
    id.style.cssText = 'font-size:11px; color:#9aa0b4; white-space:nowrap; overflow:hidden; text-overflow:ellipsis';
    id.textContent = [piece.function, piece.artifact_id, piece.deepzoom ? 'zoom' : ''].filter(Boolean).join(' · ');
    const titleInp = document.createElement('input');
    titleInp.type = 'text';
    titleInp.value = piece.title || '';
    titleInp.placeholder = 'Title (optional)';
    titleInp.style.cssText = 'width:100%; background:#0d1320; color:#f2f2f7; border:1px solid #2b3a5e; border-radius:4px; padding:4px 6px; font-size:12px';
    titleInp.addEventListener('input', () => _gallerySetTitle(index, titleInp.value));
    meta.appendChild(id);
    meta.appendChild(titleInp);
    row.appendChild(meta);

    const actions = document.createElement('div');
    actions.style.cssText = 'flex:0 0 auto; display:flex; gap:4px';
    const mkBtn = (label, title, disabled, fn) => {
        const b = document.createElement('button');
        b.type = 'button'; b.textContent = label; b.title = title;
        b.className = 'btn-secondary';
        b.style.cssText = 'padding:3px 8px; font-size:12px';
        b.disabled = !!disabled;
        b.addEventListener('click', fn);
        return b;
    };
    actions.appendChild(mkBtn('↑', 'Move up', index === 0, () => _galleryMovePiece(index, -1)));
    actions.appendChild(mkBtn('↓', 'Move down', index === total - 1, () => _galleryMovePiece(index, 1)));
    actions.appendChild(mkBtn('✕', 'Remove from gallery', false, () => _galleryRemovePiece(index)));
    row.appendChild(actions);
    return row;
}

function _renderGalleryTab() {
    _renderGallerySelector();
    _galleryRenderSceneControls();
    const nameInp = document.getElementById('gallery-name');
    const info = document.getElementById('gallery-info');
    const list = document.getElementById('gallery-piece-list');
    const doc = _galleryState.doc;
    if (nameInp) nameInp.value = doc ? (doc.name || '') : '';
    if (info) {
        info.textContent = doc
            ? ((doc.pieces || []).length + ' piece' + ((doc.pieces || []).length === 1 ? '' : 's') + (_galleryState.dirty ? ' · unsaved' : ''))
            : (_galleryState.listLoaded ? 'No gallery selected.' : '');
    }
    if (list) {
        list.innerHTML = '';
        const pieces = (doc && doc.pieces) || [];
        if (!doc) {
            const p = document.createElement('div');
            p.style.cssText = 'color:#9aa0b4; font-size:12px; padding:8px';
            p.textContent = 'Create a gallery (New) or select one, then add color renders from the DeepZoom tab.';
            list.appendChild(p);
        } else if (!pieces.length) {
            const p = document.createElement('div');
            p.style.cssText = 'color:#9aa0b4; font-size:12px; padding:8px';
            p.textContent = 'Empty. Go to the DeepZoom tab, select a color export, and click “Add to Gallery”.';
            list.appendChild(p);
        } else {
            pieces.forEach((piece, i) => list.appendChild(_renderGalleryPieceRow(piece, i, pieces.length)));
        }
    }
    _galleryUpdateActionButtons();
}

;(window.__ppParts = window.__ppParts || []).push('15-gallery');
