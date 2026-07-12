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
    baseIds: new Set(),     // piece identities at the last server sync (three-way merge base)
    epoch: 0,               // bumped on every selection/load/adopt; async ops must match
    selectedKey: '',        // selected piece (row click) for Go DeepZoom / Describe Selection
};

function _galleryPieceKey(p) {
    return (p.job_id || '') + '::' + (p.family || 'color') + '::' + (p.artifact_id || '');
}

function _gallerySyncBase(gallery, revision) {
    _galleryState.revision = revision || '';
    _galleryState.baseIds = new Set(((gallery && gallery.pieces) || []).map(_galleryPieceKey));
}

// Read by the DeepZoom tab's "Add to Gallery" (js/12). Reads localStorage (the
// persistent source of truth) so it is correct even before this tab has loaded.
function _galleryActiveId() {
    try { return String(localStorage.getItem(GALLERY_ACTIVE_KEY) || ''); } catch (e) { return _galleryState.activeId || ''; }
}

function _gallerySetActive(id) {
    _galleryState.activeId = id || '';
    _galleryState.epoch++;   // invalidate in-flight loads/saves for the old selection
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

// The DeepZoom tab calls this after an add, handing over the updated gallery +
// its new revision. When the tab has unsaved edits we MERGE (adopt the new
// revision + append the added pieces, keeping local order/titles) so a later
// Save succeeds instead of hitting a stale-revision conflict; when it is clean
// we just adopt the server version.
function _galleryNotifyChanged(galleryId, gallery, revision) {
    if (!galleryId || galleryId !== _galleryState.activeId) return;
    const sameDoc = gallery && gallery.gallery_id === galleryId;
    if (_galleryState.dirty && _galleryState.doc && sameDoc) {
        // THREE-WAY merge against the base snapshot: anything on the server that
        // was NOT in our base was added since we loaded (this add AND any
        // concurrent client's adds) and must be kept; anything in the base but
        // not local was removed locally and stays removed. Adopting the newest
        // revision is then safe — Save can no longer delete concurrent adds.
        const baseIds = _galleryState.baseIds || new Set();
        const localIds = new Set((_galleryState.doc.pieces || []).map(_galleryPieceKey));
        const added = (gallery.pieces || []).filter((p) =>
            !baseIds.has(_galleryPieceKey(p)) && !localIds.has(_galleryPieceKey(p)));
        if (added.length) _galleryState.doc.pieces = (_galleryState.doc.pieces || []).concat(added);
        _gallerySyncBase(gallery, revision || _galleryState.revision);
        _renderGalleryTab();
        _galleryStatus(`Merged ${added.length} added piece${added.length === 1 ? '' : 's'}; Save to keep your changes.`);
        return;
    }
    if (sameDoc) {
        _galleryState.doc = gallery;
        _galleryState.dirty = false;
        _gallerySyncBase(gallery, revision || _galleryState.revision);
        _galleryState.epoch++;
        _renderGalleryTab();
    } else {
        void _galleryLoadActive().then(_renderGalleryTab);
    }
}

async function loadGalleryTab() {
    _galleryBtnBusy('btn-gallery-refresh', true, 'Refreshing…');
    try {
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
    } finally {
        _galleryBtnBusy('btn-gallery-refresh', false);
    }
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
    if (!id) { _galleryState.doc = null; _galleryState.revision = ''; _galleryState.dirty = false; _galleryState.baseIds = new Set(); return; }
    try {
        const resp = await lambdaPost('storage', { gallery_id: id }, '/fetch-gallery');
        if (id !== _galleryState.activeId) return;   // a newer selection superseded this load
        if (resp && resp.error) throw new Error(resp.error);
        _galleryState.doc = resp.gallery;
        _galleryState.dirty = false;
        _gallerySyncBase(resp.gallery, resp.revision);
        _galleryState.epoch++;                        // a fresh load supersedes older saves
    } catch (e) {
        if (id !== _galleryState.activeId) return;   // stale error for a superseded selection
        // INVARIANT: doc.gallery_id === activeId. If the failed load was a
        // SWITCH (doc still holds the previous gallery), clear it — otherwise
        // the selector says B while edits silently target A.
        if (_galleryState.doc && _galleryState.doc.gallery_id !== id) {
            _galleryState.doc = null; _galleryState.revision = ''; _galleryState.dirty = false;
            _galleryState.baseIds = new Set();
        }
        // Clear the selection ONLY on a genuine 404 (the gallery is gone). A
        // network/5xx blip keeps the selection so it isn't lost spuriously.
        if (/HTTP 404/i.test(e.message || '')) {
            _galleryState.doc = null; _galleryState.revision = ''; _galleryState.dirty = false;
            _galleryState.baseIds = new Set();
            _gallerySetActive('');
            _galleryStatus('That gallery no longer exists — pick or create another.', true);
        } else {
            _galleryStatus('Could not load the gallery: ' + e.message + ' (try Refresh).', true);
        }
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
        _galleryState.dirty = false;
        _gallerySyncBase(resp.gallery, resp.revision);
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
    // Bind the operation to the gallery + epoch it started under: a save for A
    // must never stamp its revision/doc onto B (or onto a fresher load of A).
    const gid = _galleryState.doc && _galleryState.doc.gallery_id;
    const epoch = _galleryState.epoch;
    // Snapshot what we submit: if the user keeps editing during the roundtrip
    // (inputs stay live), do NOT clobber those edits with the server copy.
    const submitted = JSON.stringify(_galleryState.doc);
    const resp = await lambdaPost('storage',
        { gallery: JSON.parse(submitted), expected_revision: _galleryState.revision }, '/save-gallery');
    if (resp && resp.error) throw new Error(resp.error);
    if (_galleryState.epoch !== epoch || !_galleryState.doc || _galleryState.doc.gallery_id !== gid) {
        return resp;   // superseded — do not touch global state with a stale response
    }
    _gallerySyncBase(resp.gallery, resp.revision);
    if (JSON.stringify(_galleryState.doc) === submitted) {
        _galleryState.doc = resp.gallery;
        _galleryState.dirty = false;
    } else {
        _galleryState.dirty = true;   // keep the newer local edits; revision is fresh
        _galleryStatus('Saved — new edits made while saving are still unsaved.');
    }
    return resp;
}

async function gallerySave() {
    if (!_galleryState.doc) return;
    _galleryBtnBusy('btn-gallery-save', true, 'Saving…');
    try {
        await _gallerySavePersist();
        await _galleryRefreshList();
        if (!_galleryState.dirty) _galleryStatus('Saved.');
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
    const gid = doc && doc.gallery_id;
    if (!doc || !(doc.pieces || []).length) { _galleryStatus('Add pieces before opening.', true); return; }
    _galleryBtnBusy('btn-gallery-open', true, 'Opening…');
    // Open a blank window synchronously (popup-safe) and navigate after the POST.
    const win = window.open('', '_blank');
    try {
        // Persist any pending edits first so the snapshot matches what is shown.
        if (_galleryState.dirty) await _gallerySavePersist();
        // If edits landed DURING the autosave (or the selection moved), the
        // snapshot would show older state than the screen — stop and let the
        // user review instead of silently sharing the stale save.
        if (_galleryState.dirty || _galleryState.activeId !== gid) {
            if (win) win.close();
            _galleryStatus('Edits changed while saving — review, then press Open Gallery again.', true);
            return;
        }
        // Pin the share to the reviewed revision (the server refuses if it moved).
        const resp = await lambdaPost('storage',
            { gallery_id: gid, expected_revision: _galleryState.revision }, '/create-gallery-share');
        if (resp && resp.error) throw new Error(resp.error);
        const manifestUrl = String((resp && resp.manifest_url) || '');
        const shareId = String((resp && resp.share_id) || '');
        if (!manifestUrl || !shareId) throw new Error('no share returned');
        // SHORT link: gallery.html?share=<id> on the manifest's origin (the
        // HTTPS REST endpoint) — the viewer reconstructs the manifest path.
        const galleryUrl = new URL('/gallery.html', new URL(manifestUrl).origin);
        galleryUrl.searchParams.set('share', shareId);
        if (win) {
            win.location = galleryUrl.toString();
            _galleryStatus('Opened gallery — ' + resp.count + ' piece' + (resp.count === 1 ? '' : 's') + '.');
        } else if (typeof _copyTextToClipboard === 'function') {
            await _copyTextToClipboard(galleryUrl.toString());
            _galleryStatus('Popup blocked — gallery link copied to the clipboard instead.');
        } else {
            _galleryStatus('Popup blocked — open this link: ' + galleryUrl.toString(), true);
        }
    } catch (e) {
        if (win) win.close();
        _galleryStatus('Open failed: ' + e.message, true);
    } finally {
        _galleryBtnBusy('btn-gallery-open', false);
        _renderGalleryTab();
    }
}

// Open the selected piece's standalone DeepZoom viewer (every export ships a
// viewer.html next to its image.dzi — the DeepZoom tab's Share column artifact).
function galleryGoDeepZoom() {
    const p = _gallerySelectedPiece();
    if (!p) { _galleryBtnFlash('btn-gallery-godz', '✗ Select a piece'); _galleryStatus('Click a row first, then Go DeepZoom.', true); return; }
    if (!p.deepzoom || !p.deepzoom.export_id) { _galleryBtnFlash('btn-gallery-godz', '✗ No DeepZoom'); _galleryStatus('This piece has no DeepZoom export.', true); return; }
    const url = _publicStorageUrl(`deepzoom/${p.export_job_id || p.job_id}/${p.deepzoom.export_id}/viewer.html`);
    _galleryBtnFlash('btn-gallery-godz', '✓ Opened');
    const win = window.open(url, '_blank');
    try { if (win) win.opener = null; } catch (e) {}
}

// Vision-generated title for the SELECTED piece (same model/key config as the
// Book tab's Describe; server-side via /describe-gallery, saved per piece).
async function galleryDescribeSelection() {
    const p = _gallerySelectedPiece();
    if (!p) { _galleryBtnFlash('btn-gallery-describe', '✗ Select a piece'); _galleryStatus('Click a row first, then Describe Selection.', true); return; }
    if (_galleryState.dirty) { _galleryBtnFlash('btn-gallery-describe', '✗ Save first'); _galleryStatus('Save your changes before describing (describe writes server-side).', true); return; }
    const gid = _galleryState.activeId;
    const epoch = _galleryState.epoch;
    _galleryBtnBusy('btn-gallery-describe', true, 'Describing…');
    try {
        const resp = await lambdaPost('storage', {
            gallery_id: gid, overwrite: true,
            pieces: [{ job_id: p.job_id, family: p.family || 'color', artifact_id: p.artifact_id }],
        }, '/describe-gallery', { idempotent: false });
        if (resp && resp.error) throw new Error(resp.error);
        if ((resp.errors || []).length) throw new Error(resp.errors[0].error || 'describe failed');
        if (_galleryState.epoch === epoch && _galleryState.activeId === gid && resp.gallery) {
            _galleryState.doc = resp.gallery;
            _galleryState.dirty = false;
            _gallerySyncBase(resp.gallery, resp.revision);
            _renderGalleryTab();
        }
        const titled = (resp.gallery.pieces || []).find((q) => _galleryPieceKey(q) === _galleryPieceKey(p));
        _galleryBtnBusy('btn-gallery-describe', false);
        _galleryBtnFlash('btn-gallery-describe', '✓ Titled');
        _galleryStatus('Titled: “' + ((titled && titled.title) || '?') + '”');
    } catch (e) {
        _galleryBtnBusy('btn-gallery-describe', false);
        _galleryBtnFlash('btn-gallery-describe', '✗ Failed');
        _galleryStatus('Describe failed: ' + e.message, true);
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

function _galleryOnWallCoverage(value) {
    const s = _gallerySettings(); if (!s) return;
    const n = Math.round(Number(value));
    if (!Number.isFinite(n) || n < 5 || n > 100) return;   // wait for a valid value
    s.wall_coverage = n;
    _galleryState.dirty = true; _galleryUpdateActionButtons();
}

function _galleryOnWallLayout(value) {
    const s = _gallerySettings(); if (!s) return;
    if (!['maze', 'serpentine', 'exhibition', 'spiral'].includes(value)) return;
    s.wall_layout = value;
    _galleryState.dirty = true; _galleryUpdateActionButtons();
}

function _galleryOnWallEdge(value) {
    const s = _gallerySettings(); if (!s) return;
    const n = Math.round(Number(value));
    if (!Number.isFinite(n) || n < 0 || n > 12) return;   // wait for a valid value
    s.wall_edge_px = n;
    _galleryState.dirty = true; _galleryUpdateActionButtons();
}

function _galleryOnSkyToggle(on) {
    const s = _gallerySettings(); if (!s) return;
    s.sky = on ? 'stars' : 'dark';
    _galleryState.dirty = true; _galleryUpdateActionButtons();
}

function _galleryOnWallSelfTint(on) {
    const s = _gallerySettings(); if (!s) return;
    s.wall_self_tint = !!on;
    _galleryState.dirty = true; _galleryUpdateActionButtons();
}

function _galleryRenderSceneControls() {
    const doc = _galleryState.doc;
    const s = (doc && doc.settings) || null;
    const wc = document.getElementById('gallery-wall-color');
    const wh = document.getElementById('gallery-wall-hex');
    const sk = document.getElementById('gallery-sky-stars');
    const cov = document.getElementById('gallery-wall-coverage');
    const color = (s && s.wall_color) || '#ece4d6';
    if (wc) { wc.disabled = !doc; wc.value = color; }
    if (wh) { wh.disabled = !doc; wh.value = color; }
    if (sk) { sk.disabled = !doc; sk.checked = !s || s.sky !== 'dark'; }
    if (cov) { cov.disabled = !doc; cov.value = (s && s.wall_coverage) || 35; }
    const st = document.getElementById('gallery-wall-selftint');
    if (st) { st.disabled = !doc; st.checked = !s || s.wall_self_tint !== false; }
    const ew = document.getElementById('gallery-wall-edge');
    if (ew) { ew.disabled = !doc; ew.value = (s && Number.isFinite(s.wall_edge_px)) ? s.wall_edge_px : 1; }
    const lay = document.getElementById('gallery-wall-layout');
    if (lay) { lay.disabled = !doc; lay.value = (s && s.wall_layout) || 'maze'; }
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

function _galleryRepaintSelection() {
    const list = document.getElementById('gallery-piece-list');
    if (!list) return;
    for (const row of list.children) {
        if (!row.dataset || !row.dataset.pieceKey) continue;
        row.style.borderColor = row.dataset.pieceKey === _galleryState.selectedKey ? '#e0b877' : '#2b3a5e';
    }
}

function _gallerySelectedPiece() {
    const pieces = (_galleryState.doc && _galleryState.doc.pieces) || [];
    return pieces.find((p) => _galleryPieceKey(p) === _galleryState.selectedKey) || null;
}

function _galleryBtnFlash(id, label, ms = 1600) {
    const b = document.getElementById(id);
    if (!b) return;
    const orig = b.dataset.orig || b.textContent;
    b.dataset.orig = orig;
    b.textContent = label;
    setTimeout(() => { b.textContent = orig; delete b.dataset.orig; }, ms);
}

function _renderGalleryPieceRow(piece, index, total) {
    const row = document.createElement('div');
    const selected = _galleryPieceKey(piece) === _galleryState.selectedKey;
    row.dataset.pieceKey = _galleryPieceKey(piece);
    row.style.cssText = 'display:flex; gap:10px; align-items:center; padding:6px; background:#121829; border-radius:6px; cursor:pointer; border:1px solid ' + (selected ? '#e0b877' : '#2b3a5e');
    row.addEventListener('click', (e) => {
        if (e.target && e.target.tagName === 'BUTTON') return;   // move/remove keep their own action
        _galleryState.selectedKey = _galleryPieceKey(piece);
        if (e.target && e.target.tagName === 'INPUT') {
            _galleryRepaintSelection();   // select, but keep the typing focus (no re-render)
            return;
        }
        _renderGalleryTab();
    });

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
    titleInp.placeholder = 'image ' + (index + 1);   // the display default when untitled
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
