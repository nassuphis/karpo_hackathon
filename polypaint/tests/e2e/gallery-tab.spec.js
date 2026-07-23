// @ts-check
// The Gallery tab (virtual-gallery.md §15 / Reshape B): create a gallery, curate
// it (reorder + titles + name), Save (CAS), and Open Gallery (snapshot -> 3D
// viewer). Mirrors the Book tab. The active gallery id (localStorage) is what the
// DeepZoom tab appends to.
const { test, expect } = require('@playwright/test');

// Install an in-memory fake of the gallery storage routes AFTER the page's real
// scripts have loaded (so it overrides the real lambdaPost), then open the tab.
async function setup(page, { docs = {}, active = '' } = {}) {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
  await page.evaluate(({ docs, active }) => {
    window._galleryPosts = [];
    window.__docs = docs;
    window._rev = 1;
    window._galleryNav = '';
    if (active) localStorage.setItem('polypaint_active_gallery', active);
    else localStorage.removeItem('polypaint_active_gallery');
    window._opened = [];
    window.open = function (url) {
      if (url) window._opened.push(String(url));
      const win = { closed: false, close() { this.closed = true; }, set opener(v) {} };
      Object.defineProperty(win, 'location', { set(v) { window._galleryNav = String(v); }, configurable: true });
      return win;
    };
    window.lambdaPost = async function (name, body, path) {
      window._galleryPosts.push({ path, body: JSON.parse(JSON.stringify(body || {})) });
      if (path === '/list-galleries') {
        return { galleries: Object.values(window.__docs).map((d) => ({ gallery_id: d.gallery_id, name: d.name, count: (d.pieces || []).length })) };
      }
      if (path === '/create-gallery') {
        const id = 'gal_' + (Object.keys(window.__docs).length + 1);
        const doc = { gallery_id: id, name: body.name, document_kind: 'editable', pieces: [] };
        window.__docs[id] = doc;
        return { gallery: doc, revision: 'r' + (++window._rev) };
      }
      if (path === '/fetch-gallery') {
        const d = window.__docs[body.gallery_id];
        return d ? { gallery: JSON.parse(JSON.stringify(d)), revision: 'r' + window._rev } : { error: 'gallery not found' };
      }
      if (path === '/save-gallery') {
        window.__docs[body.gallery.gallery_id] = JSON.parse(JSON.stringify(body.gallery));
        return { gallery: window.__docs[body.gallery.gallery_id], revision: 'r' + (++window._rev) };
      }
      if (path === '/describe-gallery') {
        // ASYNC contract: the route only dispatches; the worker titles the doc
        // server-side (simulated here at dispatch), and the client polls
        // /check-status then refetches the gallery.
        const d = JSON.parse(JSON.stringify(window.__docs[body.gallery_id]));
        const t = body.pieces && body.pieces[0];
        for (const p of d.pieces) {
          if (t && p.job_id === t.job_id && p.artifact_id === t.artifact_id) p.title = 'Night Lattice';
        }
        window.__docs[body.gallery_id] = d;
        window._rev++;
        return { dispatched: true, task_id: 'describe_t1', job_id: body.gallery_id };
      }
      if (path === '/check-status') {
        return { complete: true, errors: 0, status_counts: { done: 1 } };
      }
      if (path === '/delete-task') return { deleted: 1 };
      if (path === '/create-gallery-share') {
        const d = window.__docs[body.gallery_id];
        return { manifest_url: 'https://polypaint.s3.us-east-1.amazonaws.com/renders/_shared_mosaic/gallery/s1/manifest.json',
                 share_id: 's1', count: (d.pieces || []).length };
      }
      return {};
    };
  }, { docs, active });
  await page.click('.tab-btn:text("Gallery")');
}

function pieces() {
  const mk = (job, art, fn) => ({
    job_id: job, artifact_id: art, function: fn, title: '',
    preview_key: `renders/${job}/color/${art}/preview.jpg`,
    image_key: `renders/${job}/color/${art}/image.jpeg`,
    preview_width: 512, preview_height: 512, deepzoom: null,
  });
  return [mk('jobA', 'artA', 'poly_a'), mk('jobB', 'artB', 'poly_b')];
}

function docWith(pcs) {
  return { docs: { gal_x: { gallery_id: 'gal_x', name: 'Show', document_kind: 'editable', pieces: pcs } }, active: 'gal_x' };
}

test('New creates a gallery via the app-styled modal (not a native prompt)', async ({ page }) => {
  // Fail loudly if any native prompt/confirm sneaks back in.
  let nativeDialog = false;
  page.on('dialog', (d) => { nativeDialog = true; d.dismiss(); });
  await setup(page, {});
  await page.click('#btn-gallery-new');
  await expect(page.locator('#app-modal-overlay')).toBeVisible();   // styled modal, not prompt()
  await page.fill('#app-modal-input', 'My Show');
  await page.click('#app-modal-ok');
  await expect(page.locator('#app-modal-overlay')).toBeHidden();
  expect(nativeDialog).toBe(false);
  await expect(page.locator('#gallery-name')).toHaveValue('My Show');
  // Save must be usable immediately after creating (not a mysterious grey button).
  await expect(page.locator('#btn-gallery-save')).toBeEnabled();
  const active = await page.evaluate(() => localStorage.getItem('polypaint_active_gallery'));
  expect(active).toBe('gal_1');
  const created = await page.evaluate(() => window._galleryPosts.find((p) => p.path === '/create-gallery'));
  expect(created.body.name).toBe('My Show');
});

test('reorder + save posts the new order with a CAS token', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await expect(page.locator('#gallery-piece-list > div')).toHaveCount(2);
  await page.locator('#gallery-piece-list > div').nth(0).locator('button', { hasText: '↓' }).click();
  await expect(page.locator('#btn-gallery-save')).toBeEnabled();
  await page.click('#btn-gallery-save');
  const saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  expect(saved.body.gallery.pieces.map((p) => p.artifact_id)).toEqual(['artB', 'artA']);
  expect(saved.body.expected_revision).toBeTruthy();
});

test('retitle + save carries the title', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.locator('#gallery-piece-list input[type="text"]').nth(0).fill('Opening Work');
  await page.click('#btn-gallery-save');
  const saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  expect(saved.body.gallery.pieces[0].title).toBe('Opening Work');
});

test('editing wall colour marks dirty and Save carries the scene settings', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.fill('#gallery-wall-hex', '#a1b2c3');
  await expect(page.locator('#btn-gallery-save')).toBeEnabled();
  await page.click('#btn-gallery-save');
  const saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  expect(saved.body.gallery.settings.wall_color).toBe('#a1b2c3');
});

test('wall coverage input marks dirty and Save carries it', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.fill('#gallery-wall-coverage', '15');
  await expect(page.locator('#btn-gallery-save')).toBeEnabled();
  await page.click('#btn-gallery-save');
  const saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  expect(saved.body.gallery.settings.wall_coverage).toBe(15);
});

test('edge-line thickness input marks dirty and Save carries it', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.fill('#gallery-wall-edge', '4');
  await expect(page.locator('#btn-gallery-save')).toBeEnabled();
  await page.click('#btn-gallery-save');
  const saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  expect(saved.body.gallery.settings.wall_edge_px).toBe(4);
});

test('layout dropdown marks dirty and Save carries it', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.selectOption('#gallery-wall-layout', 'serpentine');
  await expect(page.locator('#btn-gallery-save')).toBeEnabled();
  await page.click('#btn-gallery-save');
  let saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  expect(saved.body.gallery.settings.wall_layout).toBe('serpentine');
  await page.selectOption('#gallery-wall-layout', 'standalone4');
  await page.click('#btn-gallery-save');
  saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  expect(saved.body.gallery.settings.wall_layout).toBe('standalone4');
});

test('sky dropdown marks dirty and Save carries it', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.selectOption('#gallery-sky', 'galaxies');
  await expect(page.locator('#btn-gallery-save')).toBeEnabled();
  await page.click('#btn-gallery-save');
  const saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  expect(saved.body.gallery.settings.sky).toBe('galaxies');
});

test('remove piece shrinks the list', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await expect(page.locator('#gallery-piece-list > div')).toHaveCount(2);
  await page.locator('#gallery-piece-list > div').nth(0).locator('button', { hasText: '✕' }).click();
  await expect(page.locator('#gallery-piece-list > div')).toHaveCount(1);
});

test('Open Gallery snapshots (revision-pinned) and navigates to the viewer', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await expect(page.locator('#btn-gallery-open')).toBeEnabled();   // loaded
  await page.click('#btn-gallery-open');
  const shared = await page.evaluate(() => window._galleryPosts.find((p) => p.path === '/create-gallery-share'));
  expect(shared.body.gallery_id).toBe('gal_x');
  expect(shared.body.expected_revision).toBeTruthy();   // share pinned to the reviewed revision
  const nav = await page.evaluate(() => window._galleryNav);
  expect(nav).toBe('https://polypaint.s3.us-east-1.amazonaws.com/gallery.html?share=s1');   // SHORT link
});

test('Open with unsaved edits auto-saves first, then snapshots', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.locator('#gallery-piece-list input[type="text"]').nth(0).fill('Late title');
  await page.click('#btn-gallery-open');
  const paths = await page.evaluate(() => window._galleryPosts.map((p) => p.path));
  const saveIdx = paths.indexOf('/save-gallery');
  const shareIdx = paths.indexOf('/create-gallery-share');
  expect(saveIdx).toBeGreaterThanOrEqual(0);        // pending edit was saved
  expect(shareIdx).toBeGreaterThan(saveIdx);        // ...before the snapshot
});

test('no active gallery shows a blank selector option', async ({ page }) => {
  await setup(page, { docs: { gal_x: { gallery_id: 'gal_x', name: 'Show', document_kind: 'editable', pieces: [] } }, active: '' });
  await expect(page.locator('#gallery-selector')).toHaveValue('');   // blank placeholder, not gal_x
  const firstText = await page.locator('#gallery-selector option').first().textContent();
  expect(firstText).toContain('select a gallery');
});

test('a DeepZoom add while editing merges without losing edits or conflicting', async ({ page }) => {
  await setup(page, docWith(pieces()));
  // start editing -> dirty
  await page.locator('#gallery-piece-list input[type="text"]').nth(0).fill('My title');
  // simulate the DeepZoom tab handing over the updated gallery + its new revision
  await page.evaluate(() => {
    const updated = JSON.parse(JSON.stringify(window.__docs.gal_x));
    updated.pieces.push({ job_id: 'jC', artifact_id: 'artC', preview_key: 'renders/jC/color/artC/preview.jpg', image_key: 'renders/jC/color/artC/image.jpeg', preview_width: 512, preview_height: 512, function: 'h', title: '', deepzoom: null });
    _galleryNotifyChanged('gal_x', updated, 'r-after-add');
  });
  await expect(page.locator('#gallery-piece-list > div')).toHaveCount(3);   // added piece merged in
  await page.click('#btn-gallery-save');
  const saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  expect(saved.body.expected_revision).toBe('r-after-add');                 // adopted revision -> no 409
  expect(saved.body.gallery.pieces[0].title).toBe('My title');             // local edit preserved
  expect(saved.body.gallery.pieces.map((p) => p.artifact_id)).toContain('artC');
});

test('FINDING 3 REGRESSION: concurrent adds survive a dirty merge + save', async ({ page }) => {
  // Base = {A}. Locally retitle A (dirty). Server meanwhile has {A, B(concurrent), C(this add)}.
  const one = pieces().slice(0, 1);
  await setup(page, { docs: { gal_x: { gallery_id: 'gal_x', name: 'Show', document_kind: 'editable', pieces: one } }, active: 'gal_x' });
  await page.locator('#gallery-piece-list input[type="text"]').nth(0).fill('Kept title');
  await page.evaluate(() => {
    const mk = (job, art) => ({ job_id: job, artifact_id: art, family: 'color', title: '',
      preview_key: `renders/${job}/color/${art}/preview.jpg`, image_key: `renders/${job}/color/${art}/image.jpeg`,
      preview_width: 512, preview_height: 512, deepzoom: null });
    const server = JSON.parse(JSON.stringify(window.__docs.gal_x));
    server.pieces.push(mk('jobB', 'artB'), mk('jobC', 'artC'));   // B = concurrent client, C = this add
    _galleryNotifyChanged('gal_x', server, 'r-after-adds');
  });
  await expect(page.locator('#gallery-piece-list > div')).toHaveCount(3);   // A + B + C all present
  await page.click('#btn-gallery-save');
  const saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  const ids = saved.body.gallery.pieces.map((p) => p.artifact_id);
  expect(ids).toContain('artB');                                  // the concurrent add is NOT deleted
  expect(ids).toContain('artC');
  expect(saved.body.gallery.pieces[0].title).toBe('Kept title');  // local edit preserved
  expect(saved.body.expected_revision).toBe('r-after-adds');
});

test('a remote RENAME while dirty refuses the merge and keeps the old revision', async ({ page }) => {
  // code-review-29 F2: the dirty merge only knows additions. Any other remote
  // change must be refused (local doc + OLD revision kept) so the next Save
  // meets a clean 409 instead of silently overwriting the remote edit.
  await setup(page, docWith(pieces()));
  await page.locator('#gallery-piece-list input[type="text"]').nth(0).fill('My title');
  await page.evaluate(() => {
    const server = JSON.parse(JSON.stringify(window.__docs.gal_x));
    server.name = 'Renamed elsewhere';
    server.pieces.push({ job_id: 'jC', artifact_id: 'artC', preview_key: 'renders/jC/color/artC/preview.jpg', image_key: 'renders/jC/color/artC/image.jpeg', preview_width: 512, preview_height: 512, function: 'h', title: '', deepzoom: null });
    _galleryNotifyChanged('gal_x', server, 'r-remote');
  });
  await expect(page.locator('#gallery-status')).toContainText('changed elsewhere');
  await expect(page.locator('#gallery-piece-list > div')).toHaveCount(2);   // nothing silently merged
  const st = await page.evaluate(() => ({ rev: _galleryState.revision, name: _galleryState.doc.name, dirty: _galleryState.dirty }));
  expect(st.rev).not.toBe('r-remote');   // old revision kept -> Save conflicts cleanly
  expect(st.name).toBe('Show');
  expect(st.dirty).toBe(true);
});

test('a remote REMOVAL while dirty refuses the merge (no unsavable adopted state)', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.locator('#gallery-piece-list input[type="text"]').nth(0).fill('My title');
  await page.evaluate(() => {
    const server = JSON.parse(JSON.stringify(window.__docs.gal_x));
    server.pieces = server.pieces.filter((p) => p.artifact_id !== 'artB');
    _galleryNotifyChanged('gal_x', server, 'r-remote');
  });
  await expect(page.locator('#gallery-status')).toContainText('pieces removed');
  const rev = await page.evaluate(() => _galleryState.revision);
  expect(rev).not.toBe('r-remote');
});

test('save rejected with "unknown piece" says reload, never "try again"', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.evaluate(() => {
    const orig = window.lambdaPost;
    window.lambdaPost = async (name, body, path, opts) => {
      if (path === '/save-gallery') throw new Error('HTTP 400: unknown piece jobB/artB: add pieces via the DeepZoom tab');
      return orig(name, body, path, opts);
    };
  });
  await page.locator('#gallery-piece-list input[type="text"]').nth(0).fill('t');
  await page.click('#btn-gallery-save');
  await expect(page.locator('#gallery-status')).toContainText('removed elsewhere');
  const status = await page.locator('#gallery-status').textContent();
  expect(status).not.toContain('try again');   // retrying can never succeed here
});

test('Go DeepZoom refuses an export with no standalone viewer page', async ({ page }) => {
  // code-review-29 F5: viewer === false is RECORDED at admission (pre-share-links
  // exports never shipped viewer.html) — never open a known-dead URL.
  const pcs = pieces();
  pcs[0].deepzoom = { export_id: 'dzA', dzi_key: 'deepzoom/compute_a/dzA/image.dzi', source_key: null, source_artifact_id: 'artA', viewer: false };
  pcs[0].export_job_id = 'compute_a';
  await setup(page, docWith(pcs));
  await page.locator('#gallery-piece-list > div').nth(0).click();
  await page.click('#btn-gallery-godz');
  const opened = await page.evaluate(() => window._opened);
  expect(opened).toEqual([]);
  await expect(page.locator('#btn-gallery-godz')).toContainText('No viewer page');
});

test('untitled pieces default to "image N" placeholders', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await expect(page.locator('#gallery-piece-list input[type="text"]').nth(0)).toHaveAttribute('placeholder', 'image 1');
  await expect(page.locator('#gallery-piece-list input[type="text"]').nth(1)).toHaveAttribute('placeholder', 'image 2');
});

test('Describe Selection titles the selected piece via the vision route', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.locator('#gallery-piece-list > div').nth(0).click();          // select row 1
  await page.click('#btn-gallery-describe');
  await expect(page.locator('#gallery-piece-list input[type="text"]').nth(0)).toHaveValue('Night Lattice');
  await expect(page.locator('#gallery-status')).toContainText('Night Lattice');
  const post = await page.evaluate(() => window._galleryPosts.find((p) => p.path === '/describe-gallery'));
  // ownership contract (code-review-30 F1): base_title + reviewed revision travel with the dispatch
  expect(post.body.pieces).toEqual([{ job_id: 'jobA', family: 'color', artifact_id: 'artA', base_title: '' }]);
  expect(post.body.overwrite).toBe(true);
  expect(typeof post.body.expected_revision).toBe('string');
});

test('Describe never claims success when the piece vanished (code-review-30 F3)', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.evaluate(() => {
    const orig = window.lambdaPost;
    window.lambdaPost = async (name, body, path, opts) => {
      if (path === '/describe-gallery') {
        // worker "succeeds" but the piece is deleted before the refetch
        window.__docs.gal_x.pieces = window.__docs.gal_x.pieces.filter((q) => q.artifact_id !== 'artA');
        return { dispatched: true, task_id: 'describe_t2', job_id: body.gallery_id };
      }
      return orig(name, body, path, opts);
    };
  });
  await page.locator('#gallery-piece-list > div').nth(0).click();
  await page.click('#btn-gallery-describe');
  await expect(page.locator('#btn-gallery-describe')).toContainText('Failed');
  await expect(page.locator('#gallery-status')).toContainText('no longer in this gallery');
});

test('Describe Selection without a selection flashes, never posts', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.click('#btn-gallery-describe');
  await expect(page.locator('#btn-gallery-describe')).toContainText('Select a piece');
  const post = await page.evaluate(() => window._galleryPosts.find((p) => p.path === '/describe-gallery'));
  expect(post).toBeFalsy();
});

test('Go DeepZoom opens the selected piece standalone viewer', async ({ page }) => {
  const pcs = pieces();
  pcs[0].deepzoom = { export_id: 'dzA', dzi_key: 'deepzoom/compute_a/dzA/image.dzi', source_key: null, source_artifact_id: 'artA' };
  pcs[0].export_job_id = 'compute_a';
  await setup(page, docWith(pcs));
  await page.locator('#gallery-piece-list > div').nth(0).click();
  await page.click('#btn-gallery-godz');
  const opened = await page.evaluate(() => window._opened);
  expect(opened[0]).toContain('/deepzoom/compute_a/dzA/viewer.html');
  await expect(page.locator('#btn-gallery-godz')).toContainText('Opened');
});

test('a network error on load keeps the active gallery (only 404 clears it)', async ({ page }) => {
  await setup(page, docWith(pieces()));
  await page.evaluate(async () => {
    const real = window.lambdaPost;
    window.lambdaPost = async (n, b, p) => {
      if (p === '/fetch-gallery') throw new Error('storage/fetch-gallery request failed: HTTP 503');
      return real(n, b, p);
    };
    await _galleryLoadActive();
  });
  const active = await page.evaluate(() => localStorage.getItem('polypaint_active_gallery'));
  expect(active).toBe('gal_x');   // a 503 must NOT clear the selection
});

// ── Book tab: multi-select + drag reordering (photo of the Gallery patterns) ──

async function bookSetup(page) {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
  await page.evaluate(() => {
    localStorage.setItem('polypaint_active_book', 'b1');
    _bookState.activeId = 'b1';   // state snapshot was taken at parse time
    const entries = ['e1', 'e2', 'e3', 'e4'].map((id, i) => ({
      entry_id: id, job_id: 'job_' + id, artifact_id: 'art_' + id, title: 'Entry ' + (i + 1),
    }));
    window.lambdaPost = async function (name, body, path) {
      if (path === '/list-books') return { books: [{ id: 'b1', name: 'B', title: 'Chromatic Fields', subtitle: 'roots in colour', entry_count: 4 }] };
      if (path === '/fetch-book') return { book: { id: 'b1', name: 'B', entries, cover_entry_id: '', cover_source: { kind: 'none' } } };
      if (path === '/render-summary') return { families: { color: [] } };
      return {};
    };
  });
  await page.click('.tab-btn:text("Book")');
  await expect(page.locator('.book-entry-row')).toHaveCount(4);
  // selector label: name — cover title — cover subtitle (count)
  await expect(page.locator('#book-selector option[value="b1"]'))
    .toHaveText('B — Chromatic Fields — roots in colour (4)');
}

function bookOrder(page) {
  return page.evaluate(() => (_bookState.doc.entries || []).map((e) => e.entry_id));
}

test('book rows: click toggles multi-selection with badge + Clear', async ({ page }) => {
  await bookSetup(page);
  await page.locator('.book-entry-row[data-entry="e1"]').click();
  await expect(page.locator('#book-selected-count')).toHaveText('1 selected');
  await page.locator('.book-entry-row[data-entry="e3"]').click();
  await expect(page.locator('#book-selected-count')).toHaveText('2 selected');
  await expect(page.locator('.book-entry-row.selected')).toHaveCount(2);
  await page.locator('.book-entry-row[data-entry="e1"]').click();   // toggle off
  await expect(page.locator('#book-selected-count')).toHaveText('1 selected');
  await page.click('#btn-book-clear-selection');
  await expect(page.locator('#book-selected-count')).toHaveText('');
  await expect(page.locator('.book-entry-row.selected')).toHaveCount(0);
});

test('book rows: Top moves the selection to the top in list order; cover = first selected', async ({ page }) => {
  await bookSetup(page);
  await page.locator('.book-entry-row[data-entry="e2"]').click();
  await page.locator('.book-entry-row[data-entry="e4"]').click();
  await page.click('#btn-book-top');
  expect(await bookOrder(page)).toEqual(['e2', 'e4', 'e1', 'e3']);
  expect(await page.evaluate(() => _bookState.dirty)).toBe(true);
  // Bottom mirrors Top: the same selection drops to the end, order kept
  await page.click('#btn-book-bottom');
  expect(await bookOrder(page)).toEqual(['e1', 'e3', 'e2', 'e4']);
  // cover uses the FIRST selected row in list order (e2)
  await page.click('#btn-book-set-cover');
  expect(await page.evaluate(() => _bookState.doc.cover_entry_id)).toBe('e2');
});

test('book Layout tab: palette-primary radio marks dirty and saves the field', async ({ page }) => {
  await bookSetup(page);
  await page.evaluate(() => {
    window._bookSaves = [];
    const prev = window.lambdaPost;
    window.lambdaPost = async function (name, body, path) {
      if (path === '/save-book') {
        window._bookSaves.push(JSON.parse(JSON.stringify(body)));
        return { book: body.book, overwritten: true, revision: 'r2' };
      }
      return prev(name, body, path);
    };
  });
  await page.click('#book-subtab-layout');
  await expect(page.locator('#book-sub-layout')).toBeVisible();
  await expect(page.locator('input[name="book-spread-layout"][value="color_primary"]')).toBeChecked();
  await page.check('input[name="book-spread-layout"][value="palette_primary"]');
  expect(await page.evaluate(() => ({ layout: _bookState.doc.spread_layout, dirty: _bookState.dirty })))
    .toEqual({ layout: 'palette_primary', dirty: true });
  await page.evaluate(() => bookSave());
  const saves = await page.evaluate(() => window._bookSaves);
  expect(saves).toHaveLength(1);
  expect(saves[0].book.spread_layout).toBe('palette_primary');
});

test('book rows: dragging reorders — single row and whole selected group', async ({ page }) => {
  await bookSetup(page);
  // single drag: e4 to the very top (no selection involved)
  const e4 = await page.locator('.book-entry-row[data-entry="e4"]').boundingBox();
  const e1 = await page.locator('.book-entry-row[data-entry="e1"]').boundingBox();
  await page.mouse.move(e4.x + e4.width / 2, e4.y + e4.height / 2);
  await page.mouse.down();
  await page.mouse.move(e1.x + e1.width / 2, e1.y + 2, { steps: 8 });
  await page.mouse.up();
  expect(await bookOrder(page)).toEqual(['e4', 'e1', 'e2', 'e3']);
  expect(await page.evaluate(() => _bookState.dirty)).toBe(true);

  // group drag: select e1 + e3, then drag e3 — the pair travels together
  await page.locator('.book-entry-row[data-entry="e1"]').click();
  await page.locator('.book-entry-row[data-entry="e3"]').click();
  await expect(page.locator('#book-selected-count')).toHaveText('2 selected');
  const e3 = await page.locator('.book-entry-row[data-entry="e3"]').boundingBox();
  const top = await page.locator('.book-entry-row[data-entry="e4"]').boundingBox();
  await page.mouse.move(e3.x + e3.width / 2, e3.y + e3.height / 2);
  await page.mouse.down();
  await page.mouse.move(top.x + top.width / 2, top.y + 2, { steps: 8 });
  await page.mouse.up();
  expect(await bookOrder(page)).toEqual(['e1', 'e3', 'e4', 'e2']);
  // the drag's trailing click must NOT have toggled the grabbed row
  await expect(page.locator('#book-selected-count')).toHaveText('2 selected');
});
