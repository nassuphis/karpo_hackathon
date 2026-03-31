// @ts-check
const { test, expect } = require('@playwright/test');

const SUMMARY_RESPONSE = {
  job_id: 'test_refresh',
  schema_version: 2,
  calc: { exists: true, N: 5000, n1: 5000, degree: 70 },
  families: {
    color: [
      {
        family: 'color',
        artifact_id: 'color_run_1',
        created_at: '2026-03-30T10:00:00Z',
        image_key: 'renders/test_refresh/color/color_run_1/image.jpeg',
        image_url: 'https://fake/jpeg',
        preview_key: 'renders/test_refresh/color/color_run_1/preview.png',
        preview_url: 'https://fake/color-preview',
        viewer_url: 'https://fake/color-preview',
        file_size: 12345,
        width: 4096,
        height: 4096,
        color_mode: 'rainbow',
        format: 'jpeg',
      },
    ],
    bilevel: [
      {
        family: 'bilevel',
        artifact_id: 'bilevel_run_1',
        created_at: '2026-03-30T11:00:00Z',
        image_key: 'renders/test_refresh/bilevel/bilevel_run_1/image.tif',
        image_url: 'https://fake/tif',
        preview_key: 'renders/test_refresh/bilevel/bilevel_run_1/preview.png',
        preview_url: 'https://fake/bilevel-preview',
        viewer_url: 'https://fake/bilevel-preview',
        file_size: 99999,
        width: 4096,
        height: 4096,
        format: 'tif',
      },
    ],
    coeffs: [],
    palette: [
      {
        family: 'palette',
        artifact_id: 'pal_1',
        palette_id: 'pal_1',
        created_at: '2026-03-30T12:00:00Z',
        image_key: 'renders/test_refresh/palettes/pal_1/image.jpeg',
        image_url: 'https://fake/palette',
        preview_key: 'renders/test_refresh/palettes/pal_1/preview.png',
        preview_url: 'https://fake/palette-preview',
        viewer_url: 'https://fake/palette-preview',
        file_size: 40000,
        width: 4096,
        height: 4096,
        metric: 'crowding',
        palette: 'reef',
        solve_score_quantile: 0.05,
        format: 'jpeg',
      },
    ],
  },
  artifacts: {},
  deepzoom_latest: { exists: false },
};

test.beforeEach(async ({ page }) => {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
  await page.evaluate(() => {
    window.OpenSeadragon = function() {
      return { addHandler(){}, destroy(){}, world: { getItemAt(){ return null; }, getItemCount(){ return 0; } }, viewport: { getZoom(){ return 1; }, getCenter(){ return {x:0,y:0}; } } };
    };
  });
});

test.describe('Render Refresh', () => {

  test('refresh uses exactly one /render-summary call, no forbidden calls', async ({ page }) => {
    await page.evaluate((resp) => {
      window._refreshLog = [];
      window.lambdaPost = async function(name, body, path) {
        window._refreshLog.push({name, path});
        if (name === 'storage' && path === '/render-summary') return resp;
        return {};
      };
      document.getElementById('render-results-dir').value = 'test_refresh';
    }, SUMMARY_RESPONSE);

    await page.click('.tab-btn:text("Render")');
    await page.evaluate(async () => { await refreshRenderArtifacts('test_refresh'); });

    const log = await page.evaluate(() => window._refreshLog);
    const summaryCalls = log.filter(c => c.path === '/render-summary');
    expect(summaryCalls.length).toBe(1);

    const forbidden = log.filter(c =>
      c.path === '/head-keys' || c.path === '/list-prefix' || c.path === '/presign');
    expect(forbidden.length).toBe(0);
  });

  test('panel shows family tabs, selected artifact actions, and selected viewer', async ({ page }) => {
    await page.evaluate((resp) => {
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/render-summary') return resp;
        return {};
      };
    }, SUMMARY_RESPONSE);

    await page.click('.tab-btn:text("Render")');
    await page.evaluate(async () => { await refreshRenderArtifacts('test_refresh'); });

    const panel = page.locator('#render-preview');
    await expect(panel.locator('button:text("Color")')).toBeVisible();
    await expect(panel.locator('button:text("BiLevel")')).toBeVisible();
    await expect(panel.locator('button:text("Coeffs")')).toBeVisible();
    await expect(panel.locator('button:text("Palette")')).toBeVisible();
    await expect(panel.locator('#btn-render-generate')).toBeVisible();
    await expect(panel.locator('#btn-render-download')).toBeVisible();
    await expect(panel.locator('#btn-render-delete')).toBeVisible();
    await expect(panel.locator('#btn-render-deepzoom')).toBeVisible();
    await expect(panel.locator('text=color_run_1')).toBeVisible();
    await expect(panel.locator('img[src="https://fake/color-preview"]')).toBeVisible();
  });

  test('switching family updates the catalog', async ({ page }) => {
    await page.evaluate((resp) => {
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/render-summary') return resp;
        return {};
      };
    }, SUMMARY_RESPONSE);

    await page.click('.tab-btn:text("Render")');
    await page.evaluate(async () => { await refreshRenderArtifacts('test_refresh'); });
    await page.click('#render-preview button:text("Palette")');

    const panel = page.locator('#render-preview');
    await expect(panel.locator('text=pal_1')).toBeVisible();
    await expect(panel.locator('img[src="https://fake/palette-preview"]')).toBeVisible();
  });

  test('missing calc does not break panel', async ({ page }) => {
    const emptyResp = {
      job_id: 'test_nocal',
      schema_version: 2,
      calc: { exists: false, N: null, n1: null, degree: null },
      families: { color: [], bilevel: [], coeffs: [], palette: [] },
      artifacts: {},
      deepzoom_latest: { exists: false },
    };
    await page.evaluate((resp) => {
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/render-summary') return resp;
        return {};
      };
    }, emptyResp);

    await page.click('.tab-btn:text("Render")');
    await page.evaluate(async () => { await refreshRenderArtifacts('test_nocal'); });

    const status = await page.locator('#render-status').textContent();
    expect(status).not.toContain('error');
    await expect(page.locator('#render-preview')).toContainText('No saved artifacts yet.');
  });
});
