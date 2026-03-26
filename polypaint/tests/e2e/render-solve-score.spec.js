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

test.describe('Solve Score UI', () => {

  test('Render tab shows Root proximity and Solve score labels', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const rootLabel = page.locator('.color-row:has([data-mode="proximity"]) >> text=Root proximity');
    const solveLabel = page.locator('.color-row:has([data-mode="solve_score"]) >> text=Solve score');
    await expect(rootLabel).toBeVisible();
    await expect(solveLabel).toBeVisible();
  });

  test('both rows have independent palette circles', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const rootCircles = page.locator('#palette-circles-root-proximity .pal-circle');
    const solveCircles = page.locator('#palette-circles-solve-score .pal-circle');
    const rootCount = await rootCircles.count();
    const solveCount = await solveCircles.count();
    expect(rootCount).toBeGreaterThanOrEqual(5);
    expect(solveCount).toBeGreaterThanOrEqual(5);
  });

  test('clicking root-proximity palette activates proximity color mode', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const rootCircles = page.locator('#palette-circles-root-proximity .pal-circle');
    await rootCircles.nth(1).click();
    const mode = await page.evaluate(() => renderColorMode);
    expect(mode).toBe('proximity');
  });

  test('Solve score dropdown has exact 5 metric options', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const dropdown = page.locator('#render-solve-score');
    await expect(dropdown).toBeVisible();
    const options = await dropdown.locator('option').allTextContents();
    expect(options).toEqual(['Proximity', 'Crowding', 'Spread', 'Anisotropy', 'Area']);
  });

  test('clicking solve-score palette activates solve_score mode', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const solveCircles = page.locator('#palette-circles-solve-score .pal-circle');
    await solveCircles.first().click();
    const mode = await page.evaluate(() => renderColorMode);
    expect(mode).toBe('solve_score');
    const dot = page.locator('.color-dot[data-mode="solve_score"]');
    await expect(dot).toHaveClass(/active/);
  });

  test('selecting dropdown value updates renderSolveMetric', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    // Activate solve_score mode first
    const solveCircles = page.locator('#palette-circles-solve-score .pal-circle');
    await solveCircles.first().click();

    const dropdown = page.locator('#render-solve-score');
    await dropdown.selectOption('crowding');
    const metric = await page.evaluate(() => renderSolveMetric);
    expect(metric).toBe('crowding');

    await dropdown.selectOption('area');
    const metric2 = await page.evaluate(() => renderSolveMetric);
    expect(metric2).toBe('area');
  });

  test('render dispatch payload contains selected solve_metric', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const solveCircle = page.locator('#palette-circles-solve-score .pal-circle').first();
    await solveCircle.click();

    // Select a specific metric
    await page.locator('#render-solve-score').selectOption('spread');

    await page.evaluate(() => {
      window._orchPayload = null;
      window.lambdaPost = async function(name, body, path) {
        if (name === 'dispatch' && body.target === 'render_orchestrator') {
          window._orchPayload = body.jobs[0];
          return { fired: 1, errors: [] };
        }
        if (name === 'storage' && path === '/check-status') {
          return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
        }
        return {};
      };
      window.refreshRenderArtifacts = async function() {};
      document.getElementById('render-results-dir').value = 'test_ss';
      document.getElementById('render-pix').value = '512';
      window._viewMode = 'square';
      window._rtChain = [];
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const payload = await page.evaluate(() => window._orchPayload);
    expect(payload).not.toBeNull();
    expect(payload.mode).toBe('color');
    expect(payload.params.color_mode).toBe('solve_score');
    expect(payload.params.solve_metric).toBe('spread');
  });

  test('switching root-proximity palette does not change solve-score palette', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');

    // Click a solve-score palette first
    const solveCircles = page.locator('#palette-circles-solve-score .pal-circle');
    await solveCircles.nth(2).click();
    const solveActive = await page.locator('#palette-circles-solve-score .pal-circle.active').getAttribute('title');

    // Click a root-proximity palette
    const rootCircles = page.locator('#palette-circles-root-proximity .pal-circle');
    await rootCircles.nth(1).click();

    // Solve-score active should be unchanged
    const solveActiveAfter = await page.locator('#palette-circles-solve-score .pal-circle.active').getAttribute('title');
    expect(solveActiveAfter).toBe(solveActive);
  });

  test('switching solve-score palette does not change root-proximity palette', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');

    // Click a root-proximity palette first
    const rootCircles = page.locator('#palette-circles-root-proximity .pal-circle');
    await rootCircles.nth(1).click();
    const rootActive = await page.locator('#palette-circles-root-proximity .pal-circle.active').getAttribute('title');

    // Click a solve-score palette
    const solveCircles = page.locator('#palette-circles-solve-score .pal-circle');
    await solveCircles.nth(3).click();

    // Root-proximity active should be unchanged
    const rootActiveAfter = await page.locator('#palette-circles-root-proximity .pal-circle.active').getAttribute('title');
    expect(rootActiveAfter).toBe(rootActive);
  });
});
