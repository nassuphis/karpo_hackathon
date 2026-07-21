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

test.describe('Global Config popup', () => {
  test('opening loads the LLM section; Save posts the exact vision payload', async ({ page }) => {
    await page.evaluate(() => {
      window._visionPosts = [];
      window.lambdaPost = async function (name, body, path) {
        if (path === '/fetch-vision-config') {
          return { model: 'gemini-3.1-pro-preview',
                   providers: { gemini: { key_set: true, key_hint: '…h8' }, anthropic: { key_set: false }, openai: { key_set: false } } };
        }
        if (path === '/save-vision-config') {
          window._visionPosts.push(JSON.parse(JSON.stringify(body)));
          return { model: body.model, providers: { gemini: { key_set: true, key_hint: '…h8' }, anthropic: { key_set: true, key_hint: '…st' }, openai: { key_set: false } } };
        }
        return {};
      };
    });
    await page.click('#btn-config-toggle');
    await expect(page.locator('#config-popup')).toBeVisible();
    // open -> live state loads: the configured model is selected, keys summarized
    await expect(page.locator('#vision-model')).toHaveValue('gemini-3.1-pro-preview');
    await expect(page.locator('#vision-status')).toContainText('gemini ✓');
    // save carries the EXACT payload contract {model, api_key}
    await page.selectOption('#vision-model', 'claude-sonnet-4-6');
    await page.fill('#vision-key', 'sk-test');
    await page.click('#config-popup button:text("Save")');
    await expect(page.locator('#vision-status')).toContainText('saved');
    const posts = await page.evaluate(() => window._visionPosts);
    expect(posts).toEqual([{ model: 'claude-sonnet-4-6', api_key: 'sk-test' }]);
    await expect(page.locator('#vision-key')).toHaveValue('');   // key never lingers in the DOM
    // the Book toolbar no longer carries its own gear panel
    expect(await page.locator('#btn-book-vision').count()).toBe(0);
    expect(await page.locator('#book-vision-panel').count()).toBe(0);
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
          viewport_mode: body.viewport_mode || 'quantile',
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
          viewport: { min_re: -4, max_re: 4, min_im: -4, max_im: 4, center_re: 0, center_im: 0 },
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
    // diagnostics: one item per row, no "timing" label
    await expect(page.locator('#compute-preview-info')).toContainText('coeffgen 11ms');
    await expect(page.locator('#compute-preview-info')).toContainText('total: 40ms');
    await expect(page.locator('#compute-preview-info')).not.toContainText('timing:');

    const call = await page.evaluate(() => window._computePreviewCalls[0]);
    expect(call).toMatchObject({
      solver_mode: 'companion_matrix',
      N_preview: 384,
      preview_size: 960,
      quantile: 0.025,
      shim: 0.075,
      viewport_mode: 'quantile',
      function: 'poly_1',
    });

    // SQUARE viewport mode: extent = half-side (Render's convention), bounds +-extent
    await page.check('input[name="compute-preview-viewport-mode"][value="square"]');
    await page.fill('#compute-preview-square-extent', '5');
    await page.click('#btn-compute-preview');
    await expect(page.locator('#compute-preview-info')).toContainText('view: square extent=5');
    const squareCall = await page.evaluate(() => window._computePreviewCalls[1]);
    expect(squareCall).toMatchObject({
      viewport_mode: 'explicit',
      view_min_re: -5, view_max_re: 5, view_min_im: -5, view_max_im: 5,
    });

    // MARQUEE: drag a rectangle on the image; bounds translate through the
    // displayed viewport (stubbed as +-4) and land in the next payload
    await page.check('input[name="compute-preview-viewport-mode"][value="marquee"]');
    const img = page.locator('#compute-preview-box img');
    const rect = await img.boundingBox();
    await page.mouse.move(rect.x + rect.width * 0.25, rect.y + rect.height * 0.25);
    await page.mouse.down();
    await page.mouse.move(rect.x + rect.width * 0.75, rect.y + rect.height * 0.5);
    await page.mouse.up();
    await expect(page.locator('#compute-preview-marquee-info')).toContainText('UL');
    await page.click('#btn-compute-preview');
    const marqueeCall = await page.evaluate(() => window._computePreviewCalls[2]);
    expect(marqueeCall.viewport_mode).toBe('explicit');
    expect(marqueeCall.view_min_re).toBeCloseTo(-2, 1);   // 0.25 of [-4,4]
    expect(marqueeCall.view_max_re).toBeCloseTo(2, 1);    // 0.75
    expect(marqueeCall.view_max_im).toBeCloseTo(2, 1);    // y=0.25 from top
    expect(marqueeCall.view_min_im).toBeCloseTo(0, 1);    // y=0.50
  });

  test('coeff program loaded-name label follows load and clear', async ({ page }) => {
    await page.click('.tab-btn:text("Compute")');
    // Blank until something is loaded
    await expect(page.locator('#coeff-program-loaded-name')).toHaveText('');
    // _applyCoeffProgram is the single funnel behind modal Load and file
    // upload — driving it directly exercises the same label wiring.
    await page.evaluate(() => {
      _applyCoeffProgram({ version: 1, program_kind: 'coeff_program', name: 'giga_test_label', chain: [], source_text: 'cf\nrev\nemit\n' });
    });
    await expect(page.locator('#coeff-program-loaded-name')).toHaveText('giga_test_label');
    await expect(page.locator('#cp-source-text')).toHaveValue('cf\nrev\nemit\n');
    // Clear program blanks both the editor and the label
    await page.click('#coeff-program-clear');
    await expect(page.locator('#coeff-program-loaded-name')).toHaveText('');
    await expect(page.locator('#cp-source-text')).toHaveValue('');
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
      window.lambdaPost = async function (name, body) {
        if (name !== 'coeffgen' || body.phase !== 'degree_probe') throw new Error(`unexpected ${name}`);
        return {
          probe_stable: true,
          degree: 10,
          n_coeffs: 11,
          fused_estimate: {
            min_safe_chunks: 1,
            params_bytes: 1000,
            coeff_bytes: 2000,
            roots_bytes: 3000,
            estimated_peak_bytes: 4000,
            estimated_tmp_peak_bytes: 5000,
            safe_chunk_limit_reason: 'test',
          },
        };
      };
      window.runCalculateWithSolver = async function (solverMode, computeMtOptions) {
        window._computeMtCalls.push({ solverMode, computeMtOptions });
      };
    });

    await page.click('#btn-calculate-mt');
    await expect(page.locator('#compute-mt-popup-overlay')).toBeVisible();
    await expect(page.locator('#compute-mt-popup-summary')).toContainText('Function:');
    await expect(page.locator('#compute-mt-tab-classic')).toHaveCount(0);
    await expect(page.locator('#compute-mt-classic-panel')).toHaveCount(0);
    await page.fill('#compute-mt-fused-threads', '7');
    await page.fill('#compute-mt-lores-param-gen-threads-fused', '3');
    await page.fill('#compute-mt-lores-coeffgen-threads-fused', '2');
    await expect(page.locator('#compute-mt-solver-iters-row')).toBeVisible();
    await page.fill('#compute-mt-solver-iters', '5');
    await expect(page.locator('#compute-mt-popup-run')).toBeEnabled();
    await page.click('#compute-mt-popup-run');

    await expect(page.locator('#compute-mt-popup-overlay')).toBeHidden();
    const call = await page.evaluate(() => window._computeMtCalls[0]);
    expect(call).toMatchObject({
      solverMode: 'aberth_mt',
      computeMtOptions: {
        fused: true,
        fusedThreads: 7,
        loresParamGenThreads: 3,
        loresCoeffgenThreads: 2,
        solverIters: 5,
      },
    });
  });

  test('Calculate-CM uses the same fused popup contract', async ({ page }) => {
    await page.click('.tab-btn:text("Compute")');
    await page.evaluate(() => {
      window._computeMtCalls = [];
      window.lambdaPost = async function (name, body) {
        if (name !== 'coeffgen' || body.phase !== 'degree_probe') throw new Error(`unexpected ${name}`);
        return {
          probe_stable: true,
          degree: 10,
          n_coeffs: 11,
          fused_estimate: {
            min_safe_chunks: 1,
            params_bytes: 1000,
            coeff_bytes: 2000,
            roots_bytes: 3000,
            estimated_peak_bytes: 4000,
            estimated_tmp_peak_bytes: 5000,
            safe_chunk_limit_reason: 'test',
          },
        };
      };
      window.runCalculateWithSolver = async function (solverMode, computeMtOptions) {
        window._computeMtCalls.push({ solverMode, computeMtOptions });
      };
    });

    await page.click('#btn-calculate-cm');
    await expect(page.locator('#compute-mt-popup-overlay')).toBeVisible();
    await expect(page.locator('#compute-mt-popup-summary')).toContainText('Solver: CM');
    await expect(page.locator('#compute-mt-tab-classic')).toHaveCount(0);
    await expect(page.locator('#compute-mt-fused-solve-row')).toBeVisible();
    await expect(page.locator('#compute-mt-solver-iters-row')).toBeHidden();
    await page.fill('#compute-mt-fused-threads', '6');
    await expect(page.locator('#compute-mt-fused-solve-threads')).toHaveText('6');
    await page.click('#compute-mt-popup-run');

    await expect(page.locator('#compute-mt-popup-overlay')).toBeHidden();
    const call = await page.evaluate(() => window._computeMtCalls[0]);
    expect(call).toMatchObject({
      solverMode: 'companion_matrix',
      computeMtOptions: {
        fused: true,
        fusedThreads: 6,
      },
    });
  });
});
