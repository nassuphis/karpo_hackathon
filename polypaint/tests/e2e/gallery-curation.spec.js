// @ts-check
// The DeepZoom-tab "Add to Gallery" action (virtual-gallery.md §15). The tab can
// only ADD: it appends the selected COLOR export to the ACTIVE gallery (chosen in
// the Gallery tab; stored in localStorage 'polypaint_active_gallery') via
// /add-to-gallery. No create/curate here.
const { test, expect } = require('@playwright/test');

const EXPORTS = [
  { job_id: 'compute_a', export_id: 'dz_A',
    source_key: 'renders/rjobA/color/cA/image.jpeg', source_family: 'color',
    source_artifact_id: 'cA', width: 4096, height: 4096,
    created_at: '2026-05-01T10:00:00', dzi_url: 'https://dz/a.dzi' },
  { job_id: 'compute_b', export_id: 'dz_B',
    source_key: 'renders/rjobB/color/cB/image.jpeg', source_family: 'color',
    source_artifact_id: 'cB', width: 8192, height: 8192,
    created_at: '2026-05-02T10:00:00', dzi_url: 'https://dz/b.dzi' },
  { job_id: 'compute_c', export_id: 'dz_C',
    source_key: 'renders/rjobC/bilevel/bC/image.tif', source_family: 'bilevel',
    source_artifact_id: 'bC', width: 8192, height: 8192,
    created_at: '2026-05-03T10:00:00', dzi_url: 'https://dz/c.dzi' },
];

test.beforeEach(async ({ page }) => {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
  await page.evaluate((exports) => {
    window.OpenSeadragon = function () {
      const v = { addHandler(e, f) { if (e === 'open') setTimeout(f, 0); }, destroy() {},
        world: { getItemAt() { return null; }, getItemCount() { return 0; } },
        viewport: { getZoom() { return 1; }, getCenter() { return { x: 0, y: 0 }; },
          getBounds() { return { x: 0, y: 0, width: 1, height: 1 }; },
          viewportToImageRectangle() { return { x: 0, y: 0, width: 4096, height: 4096 }; } } };
      return v;
    };
    window._osdViewer = null;
    window._addPosts = [];
    localStorage.removeItem('polypaint_active_gallery');
    // Default add response: success into gallery "Show".
    window.__addResponse = { added: true, gallery: { name: 'Show', pieces: [{}, {}] } };
    window.lambdaPost = async function (name, body, path) {
      if (name === 'storage' && path === '/list-deepzoom') {
        return { exports: exports.slice(), count: exports.length };
      }
      if (name === 'storage' && path === '/add-to-gallery') {
        window._addPosts.push(body);
        return window.__addResponse;
      }
      return {};
    };
  }, EXPORTS);
});

test.describe('Gallery curation (DeepZoom tab: add-only)', () => {
  async function openTab(page) {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
  }

  // date-desc sort => rows are [bC (bilevel), cB (color), cA (color)].
  test('Add to Gallery enables only for color exports', async ({ page }) => {
    await openTab(page);
    await page.locator('.dz-inv-row').nth(0).click();   // bilevel
    await expect(page.locator('#btn-dz-add-gallery')).toBeDisabled();
    await page.locator('.dz-inv-row').nth(1).click();   // color
    await expect(page.locator('#btn-dz-add-gallery')).toBeEnabled();
  });

  test('Add posts the pick to the active gallery', async ({ page }) => {
    await page.evaluate(() => localStorage.setItem('polypaint_active_gallery', 'gal_1'));
    await openTab(page);
    await page.locator('.dz-inv-row').nth(1).click();   // color cB
    await page.click('#btn-dz-add-gallery');

    await expect(page.locator('#deepzoom-status')).toContainText('Added');
    await expect(page.locator('#deepzoom-status')).toContainText('Show');
    const posts = await page.evaluate(() => window._addPosts);
    expect(posts).toEqual([
      { gallery_id: 'gal_1', job_id: 'rjobB', artifact_id: 'cB', export_id: 'dz_B' },
    ]);
  });

  test('Add with no active gallery is refused with a hint', async ({ page }) => {
    await openTab(page);
    await page.locator('.dz-inv-row').nth(1).click();   // color, but no active gallery
    await page.click('#btn-dz-add-gallery');
    await expect(page.locator('#deepzoom-status')).toContainText('No active gallery');
    const posts = await page.evaluate(() => window._addPosts);
    expect(posts).toHaveLength(0);   // never posted
  });

  test('a duplicate add is surfaced, not treated as success', async ({ page }) => {
    await page.evaluate(() => {
      localStorage.setItem('polypaint_active_gallery', 'gal_1');
      window.__addResponse = { added: false, reason: 'duplicate', gallery: { name: 'Show', pieces: [{}] } };
    });
    await openTab(page);
    await page.locator('.dz-inv-row').nth(1).click();
    await page.click('#btn-dz-add-gallery');
    await expect(page.locator('#deepzoom-status')).toContainText('already in this gallery');
  });
});
