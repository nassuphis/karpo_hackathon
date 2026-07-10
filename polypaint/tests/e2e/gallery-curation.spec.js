// @ts-check
// Wave 2 (virtual-gallery.md Phase 0): the DeepZoom-tab "Add to Gallery" /
// "Create Gallery" curation flow. Only COLOR exports are addable; Create Gallery
// POSTs the picks to /share-gallery and opens gallery.html from the manifest
// origin, surfacing skipped[].
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
    window._sharePosts = [];
    window._galleryNav = '';
    window.open = function () {
      const win = { closed: false, close() { this.closed = true; } };
      Object.defineProperty(win, 'location', { set(v) { window._galleryNav = String(v); }, configurable: true });
      return win;
    };
    window.lambdaPost = async function (name, body, path) {
      if (name === 'storage' && path === '/list-deepzoom') {
        return { exports: exports.slice(), count: exports.length };
      }
      if (name === 'storage' && path === '/share-gallery') {
        window._sharePosts.push(body);
        return {
          manifest_url: 'https://polypaint.s3.us-east-1.amazonaws.com/renders/_shared_mosaic/gallery/share_1/manifest.json',
          share_id: 'share_1',
          count: (body.picks || []).length,
          skipped: [],
        };
      }
      return {};
    };
  }, EXPORTS);
});

test.describe('Gallery curation (DeepZoom tab)', () => {
  async function openTab(page) {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
  }

  // date-desc sort => rows are [bC (bilevel), cB (color), cA (color)].
  test('Add to Gallery enables only for color exports', async ({ page }) => {
    await openTab(page);
    // row 0 is the bilevel export — Add to Gallery must be disabled
    await page.locator('.dz-inv-row').nth(0).click();
    await expect(page.locator('#btn-dz-add-gallery')).toBeDisabled();
    // row 1 is a color export — enabled
    await page.locator('.dz-inv-row').nth(1).click();
    await expect(page.locator('#btn-dz-add-gallery')).toBeEnabled();
  });

  test('Add builds a draft and Create Gallery POSTs picks + opens the viewer', async ({ page }) => {
    await openTab(page);
    // add cA (row 2) then cB (row 1) — curator order is preserved
    await page.locator('.dz-inv-row').nth(2).click();
    await page.click('#btn-dz-add-gallery');
    await expect(page.locator('#btn-dz-create-gallery')).toHaveText('Create Gallery (1)');
    await page.locator('.dz-inv-row').nth(1).click();
    await page.click('#btn-dz-add-gallery');
    await expect(page.locator('#btn-dz-create-gallery')).toHaveText('Create Gallery (2)');

    await page.click('#btn-dz-create-gallery');

    const posts = await page.evaluate(() => window._sharePosts);
    expect(posts).toHaveLength(1);
    expect(posts[0].picks).toEqual([
      { job_id: 'rjobA', artifact_id: 'cA', export_id: 'dz_A' },
      { job_id: 'rjobB', artifact_id: 'cB', export_id: 'dz_B' },
    ]);
    const nav = await page.evaluate(() => window._galleryNav);
    expect(nav).toContain('https://polypaint.s3.us-east-1.amazonaws.com/gallery.html?manifest=');
    expect(decodeURIComponent(nav)).toContain('/renders/_shared_mosaic/gallery/share_1/manifest.json');
  });

  test('duplicate add is ignored and Clear resets the draft', async ({ page }) => {
    await openTab(page);
    await page.locator('.dz-inv-row').nth(2).click();  // color export cA
    await page.click('#btn-dz-add-gallery');
    await page.click('#btn-dz-add-gallery'); // duplicate
    await expect(page.locator('#btn-dz-create-gallery')).toHaveText('Create Gallery (1)');
    await page.click('#btn-dz-clear-gallery');
    await expect(page.locator('#btn-dz-create-gallery')).toHaveText('Create Gallery (0)');
    await expect(page.locator('#btn-dz-create-gallery')).toBeDisabled();
  });

  test('skipped pieces are surfaced in the status', async ({ page }) => {
    await page.evaluate(() => {
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list-deepzoom') return { exports: [], count: 0 };
        if (path === '/share-gallery') {
          window._sharePosts.push(body);
          return { manifest_url: 'https://polypaint.s3.us-east-1.amazonaws.com/renders/_shared_mosaic/gallery/s/manifest.json',
                   count: 1, skipped: [{ job_id: 'rjobA', artifact_id: 'cZ', reason: 'export_dzi_absent' }] };
        }
        return {};
      };
      window._galleryDraft = [{ job_id: 'rjobA', artifact_id: 'cZ', export_id: 'dz_Z' }];
      _dzUpdateGalleryButtons();
    });
    await page.click('.tab-btn:text("DeepZoom")');
    await page.click('#btn-dz-create-gallery');
    await expect(page.locator('#deepzoom-status')).toContainText('1 skipped');
    await expect(page.locator('#deepzoom-status')).toContainText('export_dzi_absent');
  });
});
