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

  test('solve_proximity render dispatches orchestrator with correct color_mode', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const solveCircle = page.locator('#palette-circles-solve-proximity .pal-circle').first();
    await solveCircle.click();

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
      document.getElementById('render-results-dir').value = 'test_sp';
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
    expect(payload.params.color_mode).toBe('solve_proximity');
  });
});
