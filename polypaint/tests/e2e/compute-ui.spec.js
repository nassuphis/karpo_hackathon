// @ts-check
const { test, expect } = require('@playwright/test');

const PREVIEW_PNG_BASE64 =
  'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+lmBkAAAAASUVORK5CYII=';

test.beforeEach(async ({ page }) => {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
  await page.evaluate(() => {
    window.OpenSeadragon = function () {
      return {
        addHandler() {},
        destroy() {},
        world: { getItemAt() { return null; }, getItemCount() { return 0; } },
        viewport: { getZoom() { return 1; }, getCenter() { return { x: 0, y: 0 }; } },
      };
    };
  });
});

test.describe('Compute UI', () => {
  test('compute tab exposes preview controls and calculate buttons', async ({ page }) => {
    await page.click('.tab-btn:text("Compute")');

    await expect(page.locator('#render-function-picker')).toBeVisible();
    await expect(page.locator('#compute-preview-n')).toBeVisible();
    await expect(page.locator('#compute-preview-solver')).toBeVisible();
    await expect(page.locator('#compute-preview-quantile')).toBeVisible();
    await expect(page.locator('#compute-preview-shim')).toBeVisible();
    await expect(page.locator('#compute-preview-size')).toBeVisible();
    await expect(page.locator('#btn-compute-preview')).toBeVisible();
    await expect(page.locator('#compute-preview-box')).toBeVisible();
    await expect(page.locator('#btn-calculate-mt')).toBeVisible();
    await expect(page.locator('#btn-calculate-cm')).toBeVisible();
  });

  test('compute preview posts current controls and renders inline image', async ({ page }) => {
    await page.click('.tab-btn:text("Compute")');
    await page.evaluate((previewBase64) => {
      window._computePreviewCalls = [];
      window.lambdaPost = async function (name, body) {
        if (name !== 'compute-preview') throw new Error(`unexpected ${name}`);
        window._computePreviewCalls.push(body);
        return {
          solver_mode: body.solver_mode,
          image_width: body.preview_size,
          image_height: body.preview_size,
          quantile: body.quantile,
          shim: body.shim,
          degree: 10,
          n_roots_in_view: 321,
          n_roots_total: 512,
          coeffs_size: 2200000,
          roots_size: 200000,
          coeffgen_ms: 11,
          solve_ms: 13,
          viewport_ms: 7,
          raster_ms: 9,
          total_ms: 40,
          image_png_base64: previewBase64,
        };
      };
      _setRenderFunction('poly_1');
    }, PREVIEW_PNG_BASE64);

    await page.fill('#compute-preview-n', '384');
    await page.selectOption('#compute-preview-solver', 'companion_matrix');
    await page.fill('#compute-preview-quantile', '2.5');
    await page.fill('#compute-preview-shim', '7.5');
    await page.fill('#compute-preview-size', '960');
    await page.click('#btn-compute-preview');

    await expect(page.locator('#compute-preview-status')).toContainText('Preview ready');
    await expect(page.locator('#compute-preview-box img')).toBeVisible();
    await expect(page.locator('#compute-preview-info')).toContainText('solver: CM');
    await expect(page.locator('#compute-preview-info')).toContainText('image: 960×960');
    await expect(page.locator('#compute-preview-info')).toContainText('view: q=2.5% · shim=7.5%');

    const call = await page.evaluate(() => window._computePreviewCalls[0]);
    expect(call).toMatchObject({
      solver_mode: 'companion_matrix',
      N_preview: 384,
      preview_size: 960,
      quantile: 0.025,
      shim: 0.075,
      function: 'poly_1',
    });
  });

  test('function picker works from compute tab', async ({ page }) => {
    await page.click('.tab-btn:text("Compute")');
    await page.click('#render-function-picker');
    await expect(page.locator('#function-popup-overlay')).toBeVisible();
    await page.fill('#function-popup-filter', 'poly_795');
    await expect(page.locator('#function-popup-body .tri-popup-row')).toHaveCount(1);
    await page.click('#function-popup-choose');
    await expect(page.locator('#function-popup-overlay')).toBeHidden();
    await expect(page.locator('#render-function-picker')).toContainText('poly_795');
  });

  test('Calculate-AE-MT opens popup and forwards compute thread settings', async ({ page }) => {
    await page.click('.tab-btn:text("Compute")');
    await page.evaluate(() => {
      window._computeMtCalls = [];
      window.runCalculateWithSolver = async function (solverMode, computeMtOptions) {
        window._computeMtCalls.push({ solverMode, computeMtOptions });
      };
    });

    await page.click('#btn-calculate-mt');
    await expect(page.locator('#compute-mt-popup-overlay')).toBeVisible();
    await expect(page.locator('#compute-mt-popup-summary')).toContainText('Function:');
    await page.fill('#compute-mt-param-gen-threads', '7');
    await page.fill('#compute-mt-coeffgen-threads', '5');
    await page.fill('#compute-mt-lores-param-gen-threads', '3');
    await page.fill('#compute-mt-lores-coeffgen-threads', '2');
    await page.click('#compute-mt-popup-run');

    await expect(page.locator('#compute-mt-popup-overlay')).toBeHidden();
    const call = await page.evaluate(() => window._computeMtCalls[0]);
    expect(call).toMatchObject({
      solverMode: 'aberth_mt',
      computeMtOptions: {
        paramGenThreads: 7,
        coeffgenThreads: 5,
        loresParamGenThreads: 3,
        loresCoeffgenThreads: 2,
      },
    });
  });
});
