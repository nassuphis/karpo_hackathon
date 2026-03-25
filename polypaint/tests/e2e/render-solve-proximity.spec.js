// @ts-check
const { test, expect } = require('@playwright/test');

test.beforeEach(async ({ page }) => {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
  // Stub OpenSeadragon
  await page.evaluate(() => {
    window.OpenSeadragon = function () {
      return { addHandler(){}, destroy(){}, world: { getItemAt(){ return null; }, getItemCount(){ return 0; } }, viewport: { getZoom(){ return 1; }, getCenter(){ return {x:0,y:0}; } } };
    };
  });
});

test.describe('Solve Proximity UI', () => {

  test('Render tab shows both Root proximity and Solve proximity labels', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const rootLabel = page.locator('.color-row:has([data-mode="proximity"]) >> text=Root proximity');
    const solveLabel = page.locator('.color-row:has([data-mode="solve_proximity"]) >> text=Solve proximity');
    await expect(rootLabel).toBeVisible();
    await expect(solveLabel).toBeVisible();
  });

  test('both proximity rows have independent palette circles', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const rootCircles = page.locator('#palette-circles-root-proximity .pal-circle');
    const solveCircles = page.locator('#palette-circles-solve-proximity .pal-circle');
    const rootCount = await rootCircles.count();
    const solveCount = await solveCircles.count();
    expect(rootCount).toBeGreaterThanOrEqual(5);
    expect(solveCount).toBeGreaterThanOrEqual(5);
  });

  test('clicking solve-proximity palette does not change root-proximity selection', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    // Get initial root-proximity active palette
    const rootActive = await page.locator('#palette-circles-root-proximity .pal-circle.active').getAttribute('title');

    // Click a different palette in solve-proximity row
    const solveCircles = page.locator('#palette-circles-solve-proximity .pal-circle');
    await solveCircles.nth(2).click();

    // Root-proximity active should be unchanged
    const rootActiveAfter = await page.locator('#palette-circles-root-proximity .pal-circle.active').getAttribute('title');
    expect(rootActiveAfter).toBe(rootActive);

    // Solve-proximity should have changed
    const solveActive = await page.locator('#palette-circles-solve-proximity .pal-circle.active').getAttribute('title');
    const clickedTitle = await solveCircles.nth(2).getAttribute('title');
    expect(solveActive).toBe(clickedTitle);
  });

  test('clicking solve-proximity palette activates solve_proximity color mode', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const solveCircles = page.locator('#palette-circles-solve-proximity .pal-circle');
    await solveCircles.first().click();
    const mode = await page.evaluate(() => renderColorMode);
    expect(mode).toBe('solve_proximity');
    // The solve_proximity dot should be active
    const dot = page.locator('.color-dot[data-mode="solve_proximity"]');
    await expect(dot).toHaveClass(/active/);
  });

  test('clicking root-proximity palette activates proximity color mode', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const rootCircles = page.locator('#palette-circles-root-proximity .pal-circle');
    await rootCircles.nth(1).click();
    const mode = await page.evaluate(() => renderColorMode);
    expect(mode).toBe('proximity');
  });

  test('color rows have increased vertical spacing', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const row = page.locator('.color-row').first();
    const mb = await row.evaluate(el => getComputedStyle(el).marginBottom);
    expect(parseInt(mb)).toBeGreaterThanOrEqual(10);
  });

  test('dropdown has functions from catalog', async ({ page }) => {
    const optCount = await page.locator('#render-function option').count();
    expect(optCount).toBeGreaterThan(100);
  });

  test('solve_proximity render dispatches clip, hist, merge before raster', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');

    // Activate solve_proximity mode
    const solveCircle = page.locator('#palette-circles-solve-proximity .pal-circle').first();
    await solveCircle.click();

    // Stub the pipeline functions to track dispatch calls
    const dispatched = await page.evaluate(() => {
      window._testDispatches = [];
      window._lastCalcMeta = {
        job_id: 'test_sp', degree: 10, n_stripes: 2, n_chunks: 2,
        lores: { bin_key: 'renders/test_sp/lores.bin' }
      };
      // Stub lambdaPost
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/clean-render') return { deleted: 0 };
        if (name === 'storage' && path === '/delete-task') return {};
        if (name === 'storage' && path === '/detail') return { calc: window._lastCalcMeta };
        if (name === 'dispatch' && body.target === 'solve_proximity') {
          window._testDispatches.push({ target: 'solve_proximity', phase: body.jobs[0]?.phase, count: body.jobs.length });
          return { fired: body.jobs.length, errors: [] };
        }
        if (name === 'dispatch' && body.target === 'raster') {
          window._testDispatches.push({ target: 'raster', count: body.jobs.length });
          return { fired: body.jobs.length, errors: [] };
        }
        if (name === 'storage' && path === '/check-status') {
          return { errors: 0, done: body.expected || 1, complete: true, status_counts: { done: body.expected || 1 },
            results: [{ clip_lo: 2.5, clip_hi: 8.0, n_solves: 100, n_solves_total: 5000,
              cuts_norm: [0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9] }] };
        }
        if (name === 'viewport') return { q_re: [-2,2], q_im: [-2,2], scale: 256, pix: 512, n_roots: 100 };
        if (name === 'storage' && path === '/save-metadata') return {};
        return {};
      };
      // Stub helpers
      window._bilevelDispatchAndPoll = async function(opts) {
        window._testDispatches.push({ target: opts.target, _wave: true, count: opts.jobs.length, taskPrefix: opts.taskPrefix });
        return 1234;
      };
      window.refreshRenderArtifacts = async function() {};
      return true;
    });

    // Set up render params
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_sp';
      document.getElementById('render-pix').value = '512';
      document.getElementById('render-format').value = 'jpeg';
      document.getElementById('render-quality').value = '90';
      document.getElementById('render-square-extent').value = '2';
      document.getElementById('sparse-tile-size').value = '512';
      document.getElementById('render-rotation').value = '0';
      document.getElementById('render-rotation-dir').value = 'ccw';
      document.getElementById('render-quantile').value = '1';
      document.getElementById('render-shim').value = '5';
      window._viewMode = 'square';
      window._rtChain = [];
    });

    // Run the pipeline directly (not via button click which has async timing issues)
    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) { /* finalize/encode stubs may throw */ }
    });

    const dispatches = await page.evaluate(() => window._testDispatches);

    // Should have: clip, hist (wave), merge, then raster (wave)
    const phases = dispatches.map(d => d.phase || (d._wave ? 'wave:' + d.target : d.target));
    expect(phases.length).toBeGreaterThanOrEqual(3);

    // First: clip
    expect(dispatches[0].target).toBe('solve_proximity');
    expect(dispatches[0].phase).toBe('clip');

    // Second: hist wave dispatch
    expect(dispatches[1]._wave).toBe(true);
    expect(dispatches[1].target).toBe('solve_proximity');
    expect(dispatches[1].count).toBe(2); // n_stripes=2

    // Third: merge
    expect(dispatches[2].target).toBe('solve_proximity');
    expect(dispatches[2].phase).toBe('merge');
  });
});
