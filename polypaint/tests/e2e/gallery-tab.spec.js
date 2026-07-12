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
        const d = JSON.parse(JSON.stringify(window.__docs[body.gallery_id]));
        const t = body.pieces && body.pieces[0];
        for (const p of d.pieces) {
          if (t && p.job_id === t.job_id && p.artifact_id === t.artifact_id) p.title = 'Night Lattice';
        }
        window.__docs[body.gallery_id] = d;
        return { gallery: d, revision: 'r' + (++window._rev), described: 1, errors: [] };
      }
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
  await expect(page.locator('#gallery-modal-overlay')).toBeVisible();   // styled modal, not prompt()
  await page.fill('#gallery-modal-input', 'My Show');
  await page.click('#gallery-modal-ok');
  await expect(page.locator('#gallery-modal-overlay')).toBeHidden();
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
  const saved = await page.evaluate(() => window._galleryPosts.filter((p) => p.path === '/save-gallery').pop());
  expect(saved.body.gallery.settings.wall_layout).toBe('serpentine');
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
  expect(post.body.pieces).toEqual([{ job_id: 'jobA', family: 'color', artifact_id: 'artA' }]);
  expect(post.body.overwrite).toBe(true);
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
