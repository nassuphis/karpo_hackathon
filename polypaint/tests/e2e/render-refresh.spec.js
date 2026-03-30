// @ts-check
const { test, expect } = require('@playwright/test');

const SUMMARY_RESPONSE = {
  job_id: 'test_refresh', schema_version: 1,
  calc: { exists: true, N: 5000, n1: 5000, degree: 70 },
  artifacts: {
    color_jpeg: { exists: true, key: 'renders/test_refresh/image.jpeg', url: 'https://fake/jpeg', size: 12345, type: 'image/jpeg', width: 4096, height: 4096 },
    color_png: { exists: false, key: 'x', url: null, size: 0, type: '', width: null, height: null },
    bilevel_tif: { exists: true, key: 'renders/test_refresh/image_bilevel.tif', url: 'https://fake/tif', size: 99999, type: 'image/tiff', width: 4096, height: 4096 },
    bilevel_preview_png: { exists: false, key: 'x', url: null, size: 0, type: '', width: null, height: null },
    bilevel_compat_tif: { exists: false, key: 'x', url: null, size: 0, type: '', width: null, height: null },
    bilevel_png: { exists: false, key: 'x', url: null, size: 0, type: '', width: null, height: null },
    coeff_tif: { exists: false, key: 'x', url: null, size: 0, type: '', width: null, height: null },
    coeff_preview_png: { exists: false, key: 'x', url: null, size: 0, type: '', width: null, height: null },
    preview_color_png: { exists: true, key: 'renders/test_refresh/preview_color.png', url: 'https://fake/prev', size: 500, type: 'image/png', width: null, height: null },
    preview_bilevel_png: { exists: false, key: 'x', url: null, size: 0, type: '', width: null, height: null },
  },
  deepzoom_latest: { exists: true, dzi_url: 'https://dz/test.dzi', export_id: 'dz1', created_at: '2026-03-26', width: 4096, height: 4096, tiles_uploaded: 100 },
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
    // Stub lambdaPost inside the real browser scope
    await page.evaluate((resp) => {
      window._refreshLog = [];
      const orig = window.lambdaPost;
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

  test('panel shows Color JPEG, BiLevel TIFF, and DeepZoom rows', async ({ page }) => {
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
    await expect(panel.locator(':text("Open Viewer")')).toBeVisible();
  });

  test('panel shows info line with N and degree', async ({ page }) => {
    await page.evaluate((resp) => {
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/render-summary') return resp;
        return {};
      };
    }, SUMMARY_RESPONSE);

    await page.click('.tab-btn:text("Render")');
    await page.evaluate(async () => { await refreshRenderArtifacts('test_refresh'); });

    const text = await page.locator('#render-info').textContent();
    expect(text).toContain('N=5000');
    expect(text).toContain('degree 70');
  });

  test('missing calc does not break panel', async ({ page }) => {
    const emptyResp = {
      job_id: 'test_nocal', schema_version: 1,
      calc: { exists: false, N: null, n1: null, degree: null },
      artifacts: Object.fromEntries(
        ['color_jpeg','color_png','bilevel_tif','bilevel_preview_png','bilevel_compat_tif',
         'bilevel_png','coeff_tif','coeff_preview_png','preview_color_png','preview_bilevel_png'
        ].map(k => [k, { exists: false, key: 'x', url: null, size: 0, type: '', width: null, height: null }])
      ),
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
  });

  test('no DeepZoom button when deepzoom_latest.exists is false', async ({ page }) => {
    const noDzResp = { ...SUMMARY_RESPONSE, deepzoom_latest: { exists: false } };
    await page.evaluate((resp) => {
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/render-summary') return resp;
        return {};
      };
    }, noDzResp);

    await page.click('.tab-btn:text("Render")');
    await page.evaluate(async () => { await refreshRenderArtifacts('test_refresh'); });

    const panel = page.locator('#render-preview');
    await expect(panel.locator(':text("Open Viewer")')).toHaveCount(0);
  });
});
