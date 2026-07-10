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

// The DeepZoom tab calls this after an add so the Gallery tab reflects it live.
function _galleryNotifyChanged(galleryId, gallery) {
    if (!galleryId || galleryId !== _galleryState.activeId) return;
    if (gallery && gallery.gallery_id === galleryId) {
        // Adopt the server's updated doc + its new revision would be unknown here,
        // so refetch to keep the CAS token correct for the next Save.
        void _galleryLoadActive().then(_renderGalleryTab);
    } else {
        void _galleryLoadActive().then(_renderGalleryTab);
    }
}

async function loadGalleryTab() {
    // Re-sync the active id from localStorage (it may have been set before this
    // module loaded, or changed in another browser tab).
    try { _galleryState.activeId = String(localStorage.getItem(GALLERY_ACTIVE_KEY) || ''); } catch (e) {}
    await _galleryRefreshList();
    await _galleryLoadActive();
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
    const raw = prompt('New gallery name:', 'Untitled gallery');
    if (raw === null) return;
    try {
        const resp = await lambdaPost('storage', { name: raw.trim() || 'Untitled gallery' }, '/create-gallery');
        if (resp && resp.error) throw new Error(resp.error);
        _gallerySetActive(resp.gallery.gallery_id);
        _galleryState.doc = resp.gallery;
        _galleryState.revision = resp.revision || '';
        _galleryState.dirty = false;
        await _galleryRefreshList();
        _renderGalleryTab();
        _galleryStatus('Created “' + resp.gallery.name + '”. Add color renders from the DeepZoom tab.');
    } catch (e) {
        _galleryStatus('Create failed: ' + e.message, true);
    }
}

async function gallerySelectorChanged() {
    const sel = document.getElementById('gallery-selector');
    _gallerySetActive(sel ? sel.value : '');
    await _galleryLoadActive();
    _renderGalleryTab();
}

async function galleryDelete() {
    const id = _galleryState.activeId;
    if (!id) return;
    if (!confirm('Delete this gallery? This cannot be undone.')) return;
    try {
        const resp = await lambdaPost('storage', { gallery_id: id }, '/delete-gallery');
        if (resp && resp.error) throw new Error(resp.error);
        _gallerySetActive('');
        _galleryState.doc = null; _galleryState.revision = ''; _galleryState.dirty = false;
        await _galleryRefreshList();
        _renderGalleryTab();
        _galleryStatus('Gallery deleted.');
    } catch (e) {
        _galleryStatus('Delete failed: ' + e.message, true);
    }
}

async function gallerySave() {
    const doc = _galleryState.doc;
    if (!doc) return;
    try {
        const resp = await lambdaPost('storage',
            { gallery: doc, expected_revision: _galleryState.revision }, '/save-gallery');
        if (resp && resp.error) throw new Error(resp.error);
        _galleryState.doc = resp.gallery;
        _galleryState.revision = resp.revision || '';
        _galleryState.dirty = false;
        await _galleryRefreshList();
        _renderGalleryTab();
        _galleryStatus('Saved.');
    } catch (e) {
        // A 409 means a concurrent change (e.g. a DeepZoom add) — reload latest.
        _galleryStatus('Save failed: ' + e.message + ' — reloading the latest version.', true);
        await _galleryLoadActive();
        _renderGalleryTab();
    }
}

async function galleryOpen() {
    const doc = _galleryState.doc;
    if (!doc || !(doc.pieces || []).length) { _galleryStatus('Add pieces before opening.', true); return; }
    if (_galleryState.dirty) { _galleryStatus('Save your changes before opening.', true); return; }
    // Open a blank window synchronously (popup-safe) and navigate after the POST.
    const win = window.open('', '_blank');
    try {
        const resp = await lambdaPost('storage', { gallery_id: _galleryState.activeId }, '/create-gallery-share');
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
    }
}

// ── in-memory edits (persisted on Save) ─────────────────────────────────────
function _galleryOnNameInput(value) {
    if (!_galleryState.doc) return;
    _galleryState.doc.name = value;
    _galleryState.dirty = true;
    _galleryUpdateActionButtons();
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
    if (saveBtn) saveBtn.disabled = !doc || !_galleryState.dirty;
    if (openBtn) openBtn.disabled = !doc || !pieces.length || _galleryState.dirty;
    if (delBtn) delBtn.disabled = !_galleryState.activeId;
    if (nameInp) nameInp.disabled = !doc;
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
    if (!list.length) {
        const opt = document.createElement('option');
        opt.value = ''; opt.textContent = '— no galleries yet —';
        sel.appendChild(opt);
        return;
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
