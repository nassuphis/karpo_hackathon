// @ts-check
const { test, expect } = require('@playwright/test');

// Stub API responses for the DeepZoom inventory
const MOCK_EXPORTS = [
  { job_id: 'compute_old', source_key: 'renders/render_old/color/color_old/image.jpeg', width: 4096, height: 4096, created_at: '2026-03-20T10:00:00', tiles_uploaded: 100, dzi_url: 'https://dz/job_old.dzi' },
  { job_id: 'compute_mid', source_key: 'renders/render_mid/bilevel/bilevel_mid/image.tif', width: 8192, height: 8192, created_at: '2026-03-22T12:00:00', tiles_uploaded: 400, dzi_url: 'https://dz/job_mid.dzi', share_url: 'https://dz/job_mid/viewer.html' },
  { job_id: 'compute_new', source_key: 'renders/render_new/color/color_new/image.jpeg', width: 16384, height: 16384, created_at: '2026-03-25T14:00:00', tiles_uploaded: 1600, dzi_url: 'https://dz/job_new.dzi', share_url: 'https://dz/job_new/viewer.html' },
];

test.beforeEach(async ({ page }) => {
  // Intercept ALL requests to stub Lambda API and meta.json fetches
  await page.route('**/*', async (route) => {
    const req = route.request();
    const url = req.url();

    // Let local static files through
    if (url.includes('localhost:8765') && !url.includes('/storage') && !url.includes('/dispatch')) {
      return route.continue();
    }

    // Intercept data: URLs (our mock presigned URLs) — let them through
    if (url.startsWith('data:')) {
      return route.continue();
    }

    // Intercept meta.json presigned URL fetches (mock presigned = data: URL, but
    // if lambdaPost hits a real URL pattern, return mock data)
    if (url.includes('meta.json') || url.includes('presign')) {
      // This shouldn't be reached with our setup, but just in case
      return route.fulfill({ json: MOCK_EXPORTS[0] });
    }

    // Let CDN scripts through (OpenSeadragon)
    if (url.includes('cdnjs.cloudflare.com')) {
      return route.continue();
    }

    // Default — let through
    return route.continue();
  });

  // Navigate
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');

  // Inject Lambda URL stubs and override lambdaPost to handle storage calls locally
  await page.evaluate((mockExports) => {
    // Stub OpenSeadragon
    window.OpenSeadragon = function () {
      return {
        addHandler() {}, destroy() {},
        world: { getItemAt() { return null; }, getItemCount() { return 0; } },
        viewport: { getZoom() { return 1; }, getCenter() { return { x: 0, y: 0 }; } },
      };
    };
    window._osdViewer = null;

    // Override lambdaPost to handle storage calls locally
    const origLambdaPost = window.lambdaPost;
    window.lambdaPost = async function (name, body, path) {
      if (name === 'storage' && path === '/list-deepzoom') {
        return { exports: mockExports, count: mockExports.length };
      }
      // Fall through for anything else
      return {};
    };
  }, MOCK_EXPORTS);
});

test.describe('DeepZoom Inventory', () => {

  test('switching to DeepZoom tab loads inventory', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    // Wait for inventory rows to appear
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
  });

  test('inventory is sorted newest first', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // First row should be compute_new (newest)
    const firstRow = page.locator('.dz-inv-row').first();
    await expect(firstRow).toContainText('compute_new');
    // Last row should be compute_old
    const lastRow = page.locator('.dz-inv-row').last();
    await expect(lastRow).toContainText('compute_old');
  });

  test('auto-selects newest entry and shows viewer', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // Viewer should be visible
    const viewer = page.locator('#deepzoom-viewer');
    await expect(viewer).toBeVisible();
    // First row should be highlighted (background style set)
    const firstRow = page.locator('.dz-inv-row').first();
    const bg = await firstRow.evaluate(el => el.style.background);
    expect(bg).toContain('rgb(42, 42, 78)');
  });

  test('clicking a row selects it and highlights it', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // Click the last row (compute_old)
    await page.locator('.dz-inv-row').last().click();
    // Last row should now be highlighted
    const lastBg = await page.locator('.dz-inv-row').last().evaluate(el => el.style.background);
    expect(lastBg).toContain('rgb(42, 42, 78)');
    // First row should NOT be highlighted
    const firstBg = await page.locator('.dz-inv-row').first().evaluate(el => el.style.background);
    expect(firstBg).toBe('');
  });

  test('clicking a row updates the viewer with correct dzi URL', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // Track viewDeepZoom calls
    await page.evaluate(() => {
      window._viewedUrls = [];
      const orig = window.viewDeepZoom;
      window.viewDeepZoom = function (url) { window._viewedUrls.push(url); orig(url); };
    });
    // Click middle row (compute_mid)
    await page.locator('.dz-inv-row').nth(1).click();
    const urls = await page.evaluate(() => window._viewedUrls);
    expect(urls[urls.length - 1]).toBe('https://dz/job_mid.dzi');
  });

  test('arrow down moves selection from first to second row', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // Initially first row (job_new) is selected
    const idx0 = await page.evaluate(() => window._dzSelectedIdx);
    expect(idx0).toBe(0);
    // Press ArrowDown
    await page.keyboard.press('ArrowDown');
    const idx1 = await page.evaluate(() => window._dzSelectedIdx);
    expect(idx1).toBe(1);
    // Second row should be highlighted
    const bg1 = await page.locator('.dz-inv-row').nth(1).evaluate(el => el.style.background);
    expect(bg1).toContain('rgb(42, 42, 78)');
    // First row should not
    const bg0 = await page.locator('.dz-inv-row').first().evaluate(el => el.style.background);
    expect(bg0).toBe('');
  });

  test('arrow up moves selection from second to first row', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // Move to second row first
    await page.keyboard.press('ArrowDown');
    expect(await page.evaluate(() => window._dzSelectedIdx)).toBe(1);
    // Press ArrowUp
    await page.keyboard.press('ArrowUp');
    expect(await page.evaluate(() => window._dzSelectedIdx)).toBe(0);
  });

  test('arrow up at top stays at top', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // Already at 0
    await page.keyboard.press('ArrowUp');
    expect(await page.evaluate(() => window._dzSelectedIdx)).toBe(0);
  });

  test('arrow down at bottom stays at bottom', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    await page.keyboard.press('ArrowDown');
    await page.keyboard.press('ArrowDown');
    expect(await page.evaluate(() => window._dzSelectedIdx)).toBe(2);
    // One more down — should stay at 2
    await page.keyboard.press('ArrowDown');
    expect(await page.evaluate(() => window._dzSelectedIdx)).toBe(2);
  });

  test('arrow keys do nothing when DeepZoom tab is not active', async ({ page }) => {
    // Stay on Compute tab (default)
    await page.keyboard.press('ArrowDown');
    await page.keyboard.press('ArrowDown');
    // No crash, and _dzSelectedIdx should be -1 (never set)
    const idx = await page.evaluate(() => window._dzSelectedIdx ?? -1);
    expect(idx).toBe(-1);
  });

  test('inventory shows dimensions and tile count', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // First row (compute_new) should show 16384×16384
    const firstText = await page.locator('.dz-inv-row').first().textContent();
    expect(firstText).toContain('16384×16384');
    expect(firstText).toContain('1600');
  });

  test('inventory header includes Render and Share columns', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    const headerText = await page.locator('#dz-inv-table tr').first().textContent();
    expect(headerText).toContain('Render');
    expect(headerText).toContain('Share');
  });

  test('inventory shows render artifact ids derived from source keys', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    const firstRow = page.locator('.dz-inv-row').first();
    await expect(firstRow).toContainText('compute_new');
    await expect(firstRow).toContainText('color_new');
  });

  test('PopulateResult button is labeled clearly', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('#btn-dz-populate')).toHaveText('PopulateResult');
  });

  test('PopulateResult loads result detail and populates Compute', async ({ page }) => {
    await page.evaluate(() => {
      window._dzPopulateCall = null;
      window._getResultDetail = async function (jobId) {
        return { solver: 'newton', view: { pix: 4096 }, requested_job_id: jobId };
      };
      window._populateComputeFromDetail = function (jobId, detail) {
        window._dzPopulateCall = { jobId, detail };
      };
    });
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    await page.click('#btn-dz-populate');
    const call = await page.evaluate(() => window._dzPopulateCall);
    expect(call).toEqual({
      jobId: 'compute_new',
      detail: { solver: 'newton', view: { pix: 4096 }, requested_job_id: 'compute_new' },
    });
  });

  test('GotoRender jumps to the source render job and artifact', async ({ page }) => {
    await page.evaluate(() => {
      window._dzRenderJumps = [];
      window.refreshRenderArtifacts = async function (jobId, opts) {
        window._dzRenderJumps.push({ jobId, opts: opts || null });
        return { families: { color: [], bilevel: [], coeffs: [], palette: [], pdf: [] }, calc: {} };
      };
    });
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    await page.click('#btn-dz-goto-render');
    await expect(page.locator('#render-results-dir')).toHaveValue('render_new');
    const jumps = await page.evaluate(() => window._dzRenderJumps);
    expect(jumps).toContainEqual({
      jobId: 'render_new',
      opts: { selectFamily: 'color', selectArtifactId: 'color_new' },
    });
  });

  test('GotoRender stays disabled for legacy exports without source keys', async ({ page }) => {
    await page.evaluate(() => {
      window.lambdaPost = async function (name, body, path) {
        if (name === 'storage' && path === '/list-deepzoom') {
          return {
            exports: [{
              job_id: 'legacy_compute',
              export_id: 'dz_legacy',
              width: 2048,
              height: 2048,
              created_at: '2026-03-26T09:00:00',
              tiles_uploaded: 12,
              dzi_url: 'https://dz/legacy.dzi',
            }],
            count: 1,
          };
        }
        return {};
      };
    });
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(1, { timeout: 10000 });
    await expect(page.locator('#btn-dz-goto-render')).toBeDisabled();
  });

  test('rows with share_url render an Open link', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // compute_new (first row, newest) has share_url
    const firstRow = page.locator('.dz-inv-row').first();
    const link = firstRow.locator('a');
    await expect(link).toHaveText('Open');
    const href = await link.getAttribute('href');
    expect(href).toContain('viewer.html');
    expect(href).toContain('job_new');
    // Security: must have rel="noopener noreferrer"
    const rel = await link.getAttribute('rel');
    expect(rel).toContain('noopener');
  });

  test('rows without share_url show question mark', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // compute_old (last row) has no share_url
    const lastRow = page.locator('.dz-inv-row').last();
    const lastText = await lastRow.textContent();
    expect(lastText).toContain('?');
    // Should NOT have an Open link
    const links = await lastRow.locator('a').count();
    expect(links).toBe(0);
  });

  test('clicking share link does not change row selection', async ({ page }) => {
    await page.click('.tab-btn:text("DeepZoom")');
    await expect(page.locator('.dz-inv-row')).toHaveCount(3, { timeout: 10000 });
    // Select first row (auto-selected)
    expect(await page.evaluate(() => window._dzSelectedIdx)).toBe(0);
    // Click the share link in first row — should not change selection
    // (link has event.stopPropagation)
    const link = page.locator('.dz-inv-row').first().locator('a');
    // Intercept navigation so we don't actually leave the page
    await page.evaluate(() => {
      document.querySelectorAll('.dz-inv-row a').forEach(a => {
        a.addEventListener('click', e => e.preventDefault(), true);
      });
    });
    await link.click();
    expect(await page.evaluate(() => window._dzSelectedIdx)).toBe(0);
  });

  test('function popup has functions from catalog', async ({ page }) => {
    await page.click('.tab-btn:text("Compute")');
    await page.click('#render-function-picker');
    await expect(page.locator('#function-popup-overlay')).toBeVisible();
    await expect(page.locator('#function-popup-body .tri-popup-row').first()).toBeVisible();
    const rowCount = await page.locator('#function-popup-body .tri-popup-row').count();
    expect(rowCount).toBeGreaterThan(100);
    await page.fill('#function-popup-filter', 'poly_795');
    await expect(page.locator('#function-popup-body .tri-popup-row')).toHaveCount(1);
    await page.click('#function-popup-choose');
    await expect(page.locator('#function-popup-overlay')).toBeHidden();
    await expect(page.locator('#render-function-picker')).toContainText('poly_795');
  });
});
