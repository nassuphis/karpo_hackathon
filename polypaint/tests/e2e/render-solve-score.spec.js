// @ts-check
const { test, expect } = require('@playwright/test');

function renderPaletteContainer(mode) {
  if (mode === 'proximity') return '#palette-circles-root-proximity';
  if (mode === 'solve_score') return '#palette-circles-solve-score';
  return '#palette-circles-palette-tab';
}

async function openBuiltinPalettePopup(page, mode) {
  const swatch = page.locator(`${renderPaletteContainer(mode)} [data-palette-popup="builtin"]`);
  await swatch.click();
  await expect(page.locator('#builtin-popup-overlay')).toBeVisible();
  return swatch;
}

async function chooseBuiltinPalette(page, mode, name) {
  await openBuiltinPalettePopup(page, mode);
  const row = page.locator('#builtin-popup-body .tri-popup-row').filter({ hasText: name }).first();
  await expect(row).toBeVisible();
  await row.click();
}

async function chooseLongPalette(page, mode, name) {
  const swatch = page.locator(`${renderPaletteContainer(mode)} .pal-circle-long`);
  await swatch.click();
  await expect(page.locator('#long-popup-overlay')).toBeVisible();
  const row = page.locator('#long-popup-body .tri-popup-row').filter({ hasText: name }).first();
  await expect(row).toBeVisible();
  await row.click();
}

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

  test('render rows collapse built-ins into popup selectors', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const rootCircles = page.locator('#palette-circles-root-proximity .pal-circle');
    const solveCircles = page.locator('#palette-circles-solve-score .pal-circle');
    const rootCount = await rootCircles.count();
    const solveCount = await solveCircles.count();
    expect(rootCount).toBe(3);
    expect(solveCount).toBe(3);
  });

  test('render rows show built-in, TRI, and LONG swatches', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await expect(page.locator('#palette-circles-root-proximity [data-palette-popup="builtin"]')).toBeVisible();
    await expect(page.locator('#palette-circles-solve-score [data-palette-popup="builtin"]')).toBeVisible();
    await expect(page.locator('#palette-circles-root-proximity .pal-circle-tri')).toBeVisible();
    await expect(page.locator('#palette-circles-solve-score .pal-circle-tri')).toBeVisible();
    await expect(page.locator('#palette-circles-root-proximity .pal-circle-long')).toBeVisible();
    await expect(page.locator('#palette-circles-solve-score .pal-circle-long')).toBeVisible();
  });

  test('left-click built-in swatch opens popup and selecting a row activates palette', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const builtin = await openBuiltinPalettePopup(page, 'solve_score');
    await expect(page.locator('#builtin-popup-title')).toContainText('Solve score');
    await page.locator('#builtin-popup-filter').fill('viri');
    const row = page.locator('#builtin-popup-body .tri-popup-row').filter({ hasText: 'viridis' }).first();
    await expect(row).toBeVisible();
    await row.click();
    const palette = await page.evaluate(() => renderSolveScorePalette);
    const remembered = await page.evaluate(() => renderSolveScoreBuiltinPalette);
    expect(palette).toBe('viridis');
    expect(remembered).toBe('viridis');
    await expect(page.locator('#builtin-popup-overlay')).not.toBeVisible();
    await expect(builtin).toHaveAttribute('title', /viridis/);
  });

  test('left-click TRI opens popup and selecting a row activates tri palette', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const tri = page.locator('#palette-circles-solve-score .pal-circle-tri');
    await tri.click();
    await expect(page.locator('#tri-popup-overlay')).toBeVisible();
    await expect(page.locator('#tri-popup-title')).toContainText('Solve score');
    await page.locator('#tri-popup-filter').fill('rg');
    const firstRow = page.locator('#tri-popup-body .tri-popup-row').first();
    await expect(firstRow).toBeVisible();
    await firstRow.click();
    const palette = await page.evaluate(() => renderSolveScorePalette);
    const remembered = await page.evaluate(() => renderSolveScoreTriName);
    expect(palette).toBe('tri_redgold');
    expect(remembered).toBe('redgold');
    await expect(page.locator('#tri-popup-overlay')).not.toBeVisible();
    await expect(tri).toHaveAttribute('title', /redgold/);
  });

  test('left-click LONG opens popup and selecting a row activates long palette', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const longSwatch = page.locator('#palette-circles-solve-score .pal-circle-long');
    await longSwatch.click();
    await expect(page.locator('#long-popup-overlay')).toBeVisible();
    await expect(page.locator('#long-popup-title')).toContainText('Solve score');
    await page.locator('#long-popup-filter').fill('spider');
    const firstRow = page.locator('#long-popup-body .tri-popup-row').first();
    await expect(firstRow).toBeVisible();
    await firstRow.click();
    const palette = await page.evaluate(() => renderSolveScorePalette);
    const remembered = await page.evaluate(() => renderSolveScoreLongName);
    expect(palette).toBe('long_marvel_spiderman_long');
    expect(remembered).toBe('marvel_spiderman_long');
    await expect(page.locator('#long-popup-overlay')).not.toBeVisible();
    await expect(longSwatch).toHaveAttribute('title', /marvel_spiderman_long/);
  });

  test('right-click TRI activates remembered palette without opening popup', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      renderRootProximityTriName = 'redgold';
      renderRootProximityPalette = 'inferno';
      buildPaletteCircles('palette-circles-root-proximity', 'proximity', () => renderRootProximityPalette);
    });
    const tri = page.locator('#palette-circles-root-proximity .pal-circle-tri');
    await tri.click({ button: 'right' });
    const palette = await page.evaluate(() => renderRootProximityPalette);
    expect(palette).toBe('tri_redgold');
    await expect(page.locator('#tri-popup-overlay')).not.toBeVisible();
  });

  test('switching built-in palette does not erase remembered TRI selection', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const tri = page.locator('#palette-circles-solve-score .pal-circle-tri');
    await tri.click();
    await page.locator('#tri-popup-filter').fill('rg');
    await page.locator('#tri-popup-body .tri-popup-row').first().click();
    await chooseBuiltinPalette(page, 'solve_score', 'viridis');
    const remembered = await page.evaluate(() => renderSolveScoreTriName);
    const activePalette = await page.evaluate(() => renderSolveScorePalette);
    expect(remembered).toBe('redgold');
    expect(activePalette).not.toBe('tri_redgold');
    await expect(tri).toHaveAttribute('title', /redgold/);
  });

  test('switching built-in palette does not erase remembered LONG selection', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await chooseLongPalette(page, 'solve_score', 'marvel_spiderman_long');
    await chooseBuiltinPalette(page, 'solve_score', 'viridis');
    const remembered = await page.evaluate(() => renderSolveScoreLongName);
    const activePalette = await page.evaluate(() => renderSolveScorePalette);
    expect(remembered).toBe('marvel_spiderman_long');
    expect(activePalette).not.toBe('long_marvel_spiderman_long');
    await expect(page.locator('#palette-circles-solve-score .pal-circle-long')).toHaveAttribute('title', /marvel_spiderman_long/);
  });

  test('clicking root-proximity palette activates proximity color mode', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await chooseBuiltinPalette(page, 'proximity', 'viridis');
    const mode = await page.evaluate(() => renderColorMode);
    expect(mode).toBe('proximity');
  });

  test('Solve score dropdown has exact 5 metric options', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const dropdown = page.locator('#render-solve-score');
    await expect(dropdown).toBeVisible();
    const options = await dropdown.locator('option').allTextContents();
    expect(options).toEqual([
      'Proximity', 'Crowding', 'Spread', 'Anisotropy', 'Area',
      'Clusteriness', 'Shelliness', 'Outlierness', 'NN variation', 'Real-axis proximity',
    ]);
  });

  test('clicking solve-score palette activates solve_score mode', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await chooseBuiltinPalette(page, 'solve_score', 'inferno');
    const mode = await page.evaluate(() => renderColorMode);
    expect(mode).toBe('solve_score');
    const dot = page.locator('.color-dot[data-mode="solve_score"]');
    await expect(dot).toHaveClass(/active/);
  });

  test('selecting dropdown value updates renderSolveMetric', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    // Activate solve_score mode first
    await page.locator('.color-dot[data-mode="solve_score"]').click();

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
    await page.locator('.color-dot[data-mode="solve_score"]').click();

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
    await chooseBuiltinPalette(page, 'solve_score', 'plasma');
    const solveActive = await page.locator('#palette-circles-solve-score .pal-circle.active').getAttribute('title');

    // Click a root-proximity palette
    await chooseBuiltinPalette(page, 'proximity', 'viridis');

    // Solve-score active should be unchanged
    const solveActiveAfter = await page.locator('#palette-circles-solve-score .pal-circle.active').getAttribute('title');
    expect(solveActiveAfter).toBe(solveActive);
  });

  test('switching solve-score palette does not change root-proximity palette', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');

    // Click a root-proximity palette first
    await chooseBuiltinPalette(page, 'proximity', 'viridis');
    const rootActive = await page.locator('#palette-circles-root-proximity .pal-circle.active').getAttribute('title');

    // Click a solve-score palette
    await chooseBuiltinPalette(page, 'solve_score', 'plasma');

    // Root-proximity active should be unchanged
    const rootActiveAfter = await page.locator('#palette-circles-root-proximity .pal-circle.active').getAttribute('title');
    expect(rootActiveAfter).toBe(rootActive);
  });

  test('clusteriness dispatch sends correct solve_metric', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.locator('.color-dot[data-mode="solve_score"]').click();
    await page.locator('#render-solve-score').selectOption('clusteriness');

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
      document.getElementById('render-results-dir').value = 'test_cl';
      document.getElementById('render-pix').value = '512';
      window._viewMode = 'square';
      window._rtChain = [];
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const payload = await page.evaluate(() => window._orchPayload);
    expect(payload).not.toBeNull();
    expect(payload.params.color_mode).toBe('solve_score');
    expect(payload.params.solve_metric).toBe('clusteriness');
  });

  test('solve-score quantile slider present with 0.1% default', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const slider = page.locator('#render-solve-score-quantile');
    await expect(slider).toBeVisible();
    const val = await slider.inputValue();
    expect(val).toBe('0.1');
    const text = await page.locator('#render-solve-score-quantile-val').textContent();
    expect(text.trim()).toBe('0.1');
  });

  test('changing solve-score quantile slider updates displayed text', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      const s = document.getElementById('render-solve-score-quantile');
      s.value = '3.0';
      s.dispatchEvent(new Event('input'));
    });
    const text = await page.locator('#render-solve-score-quantile-val').textContent();
    expect(text.trim()).toBe('3.0');
  });

  test('dispatch payload includes solve_score_quantile', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.locator('.color-dot[data-mode="solve_score"]').click();
    await page.evaluate(() => {
      document.getElementById('render-solve-score-quantile').value = '2.0';
    });

    await page.evaluate(() => {
      window._qPayload = null;
      window.lambdaPost = async function(name, body, path) {
        if (name === 'dispatch' && body.target === 'render_orchestrator') {
          window._qPayload = body.jobs[0];
          return { fired: 1, errors: [] };
        }
        if (name === 'storage' && path === '/check-status') {
          return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
        }
        return {};
      };
      window.refreshRenderArtifacts = async function() {};
      document.getElementById('render-results-dir').value = 'test_q';
      document.getElementById('render-pix').value = '512';
      window._viewMode = 'square';
      window._rtChain = [];
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const payload = await page.evaluate(() => window._qPayload);
    expect(payload).not.toBeNull();
    expect(payload.params.solve_score_quantile).toBeCloseTo(0.02, 3);
  });

  test('viewport quantile and solve-score quantile are independent', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    // Set viewport quantile to 2.0, solve-score quantile to 4.0
    await page.evaluate(() => {
      document.getElementById('render-quantile').value = '2.0';
      document.getElementById('render-solve-score-quantile').value = '4.0';
    });
    const vq = await page.locator('#render-quantile').inputValue();
    const sq = await page.locator('#render-solve-score-quantile').inputValue();
    expect(vq).toBe('2');
    expect(sq).toBe('4');
  });

  test('real_axis_proximity dispatch sends correct solve_metric', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.locator('.color-dot[data-mode="solve_score"]').click();
    await page.locator('#render-solve-score').selectOption('real_axis_proximity');

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
      document.getElementById('render-results-dir').value = 'test_rap';
      document.getElementById('render-pix').value = '512';
      window._viewMode = 'square';
      window._rtChain = [];
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const payload = await page.evaluate(() => window._orchPayload);
    expect(payload).not.toBeNull();
    expect(payload.params.color_mode).toBe('solve_score');
    expect(payload.params.solve_metric).toBe('real_axis_proximity');
  });

  test('Histogram button is visible beside Score clip q', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const btn = page.locator('#btn-solve-histogram');
    await expect(btn).toBeVisible();
    expect(await btn.textContent()).toBe('Histogram');
  });

  test('Histogram button calls solve_proximity with summary phase', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    // Activate solve_score mode
    await page.locator('.color-dot[data-mode="solve_score"]').click();
    // Set quantile and enable buttons
    await page.evaluate(() => {
      document.getElementById('render-solve-score-quantile').value = '3.0';
      document.getElementById('render-results-dir').value = 'test_hist';
      _lastCalcHasLores = true;
      _updateSolveScoreButtons();
    });

    // Intercept lambdaPost calls
    await page.evaluate(() => {
      window._histPayload = null;
      window.lambdaPost = async function(name, body, path) {
        if (name === 'solve_proximity') {
          window._histPayload = body;
          return {
            mode: 'summary', metric: 'proximity', n_solves: 100, degree: 5,
            min_score: -1, max_score: 2, mean_score: 0.5, stddev_score: 0.3,
            q05: -0.5, q10: -0.3, q25: 0.1, q50: 0.5, q75: 0.9, q90: 1.2, q95: 1.5,
            clip_quantile: 0.03, clip_lo: -0.5, clip_hi: 1.5, full_range: 3, clip_range: 2,
            clip_below_count: 5, clip_inrange_count: 90, clip_above_count: 5,
            clip_below_frac: 0.05, clip_inrange_frac: 0.9, clip_above_frac: 0.05,
            clip_fallback: false, clip_fallback_reason: null,
            intermediate_hist_bins: 100, final_bins: 10,
            cuts_norm: [0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9],
            cuts_score: [-0.3,-0.1,0.1,0.3,0.5,0.7,0.9,1.1,1.3],
            final_bin_counts: [9,9,9,9,9,9,9,9,9,9],
            final_bin_fracs: [0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1],
            min_score_count: 1, max_score_count: 1, clip_lo_count: 0, clip_hi_count: 0,
            n_unique_scores: 90,
            dl_ms: 10, compute_ms: 5,
          };
        }
        if (name === 'storage' && path === '/detail') {
          return { calc: { degree: 5, lores: { bin_key: 'renders/test_hist/lores.bin' } } };
        }
        return {};
      };
    });

    await page.click('#btn-solve-histogram');
    // Wait a tick for async completion
    await page.waitForTimeout(500);

    const payload = await page.evaluate(() => window._histPayload);
    expect(payload).not.toBeNull();
    expect(payload.phase).toBe('summary');
    expect(payload.metric).toBe('proximity');
    expect(payload.solve_score_quantile).toBeCloseTo(0.03, 3);
    expect(payload.lores_bin_key).toBe('renders/test_hist/lores.bin');
    expect(payload.degree).toBe(5);

    // Check log output contains 10-bin table
    const logText = await page.locator('#render-log').textContent();
    expect(logText).toContain('Solve histogram');
    expect(logText).toContain('final color bins (10');
    expect(logText).toContain('b0');
    expect(logText).toContain('b9');
    expect(logText).toContain('clip');
    // Must NOT contain the old 32-bin full-range header
    expect(logText).not.toContain('32 bins');
    expect(logText).not.toContain('full range');
  });

  test('render panel shows family tabs and selected-artifact actions', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      renderArtifactPanel('test_job', {
        families: {
          color: [{ artifact_id: 'color_1', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/test_job/color/color_1/image.jpeg', image_url: 'https://example.com/c.jpeg', preview_url: 'https://example.com/c.png', viewer_url: 'https://example.com/c.png', file_size: 50000, width: 1000, height: 1000, color_mode: 'rainbow', format: 'jpeg' }],
          bilevel: [{ artifact_id: 'bilevel_1', created_at: '2026-03-30T11:00:00Z', image_key: 'renders/test_job/bilevel/bilevel_1/image.tif', image_url: 'https://example.com/b.tif', preview_url: 'https://example.com/b.png', viewer_url: 'https://example.com/b.png', file_size: 60000, width: 1000, height: 1000, format: 'tif' }],
          coeffs: [{ artifact_id: 'coeffs_1', created_at: '2026-03-30T11:30:00Z', image_key: 'renders/test_job/coeffs/coeffs_1/image.tif', image_url: 'https://example.com/co.tif', preview_url: 'https://example.com/co.png', viewer_url: 'https://example.com/co.png', file_size: 70000, width: 1000, height: 1000, format: 'tif' }],
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', created_at: '2026-03-30T12:00:00Z', image_key: 'renders/test_job/palettes/pal_1/image.jpeg', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png', viewer_url: 'https://example.com/p.png', file_size: 40000, width: 1000, height: 1000, metric: 'crowding', palette: 'reef', solve_score_quantile: 0.05 }],
        },
        calc: { exists: true, N: 1000, degree: 5 },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
    });

    const panel = page.locator('#render-preview');
    for (const label of ['Color', 'BiLevel', 'Coeffs', 'Palette']) {
      await expect(panel.locator('button:text("' + label + '")')).toBeVisible();
    }
    await expect(panel.locator('#btn-render-generate')).toBeVisible();
    await expect(panel.locator('#btn-render-populate')).toBeVisible();
    await expect(panel.locator('#btn-render-download')).toBeVisible();
    await expect(panel.locator('#btn-render-delete')).toBeVisible();
    await expect(panel.locator('#btn-render-deepzoom')).toBeVisible();
    await expect(panel.locator('text=color_1')).toBeVisible();
  });

  test('palette family generate is disabled outside solve_score mode', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      renderColorMode = 'rainbow';
      _renderActiveFamily = 'palette';
      renderArtifactPanel('test_job', {
        families: { color: [], bilevel: [], coeffs: [], palette: [] },
        calc: { exists: true, N: 1000, degree: 5 },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
    });
    const btn = page.locator('#btn-render-generate');
    await expect(btn).toBeDisabled();
  });

  test('switching family updates the selected catalog and viewer', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      renderArtifactPanel('test_job', {
        families: {
          color: [{ artifact_id: 'color_1', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/test_job/color/color_1/image.jpeg', image_url: 'https://example.com/c.jpeg', preview_url: 'https://example.com/c.png', viewer_url: 'https://example.com/c.png', file_size: 50000, width: 1000, height: 1000, color_mode: 'rainbow', format: 'jpeg' }],
          bilevel: [],
          coeffs: [],
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', created_at: '2026-03-30T12:00:00Z', image_key: 'renders/test_job/palettes/pal_1/image.jpeg', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png', viewer_url: 'https://example.com/p.png', file_size: 40000, width: 1000, height: 1000, metric: 'crowding', palette: 'reef', solve_score_quantile: 0.05 }],
        },
        calc: { exists: true, N: 1000, degree: 5 },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
    });

    await page.click('#render-preview button:text("Palette")');
    await expect(page.locator('#render-preview').getByText('pal_1')).toBeVisible();
    await expect(page.locator('#render-preview img[src="https://example.com/p.png"]')).toBeVisible();
  });

  test('palette family populate restores solve-score settings and switches to color', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      renderColorMode = 'rainbow';
      renderSolveMetric = 'proximity';
      renderSolveScorePalette = 'inferno';
      _rtChain = [];
      document.getElementById('render-solve-score').value = 'proximity';
      document.getElementById('render-solve-score-quantile').value = '0.1';
      document.getElementById('render-solve-score-quantile-val').textContent = '0.1';
      _renderActiveFamily = 'palette';
      renderArtifactPanel('test_job', {
        families: {
          color: [{ artifact_id: 'color_1', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/test_job/color/color_1/image.jpeg', image_url: 'https://example.com/c.jpeg', preview_url: 'https://example.com/c.png', viewer_url: 'https://example.com/c.png', file_size: 50000, width: 1000, height: 1000, color_mode: 'rainbow', format: 'jpeg' }],
          bilevel: [],
          coeffs: [],
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', created_at: '2026-03-30T12:00:00Z', image_key: 'renders/test_job/palettes/pal_1/image.jpeg', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png', viewer_url: 'https://example.com/p.png', file_size: 40000, width: 1000, height: 1000, metric: 'crowding', palette: 'reef', solve_score_quantile: 0.05, root_transforms: [['rotate_roots', '0.125']] }],
        },
        calc: { exists: true, N: 1000, degree: 5 },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
    });

    await page.click('#btn-render-populate');

    await expect(page.locator('#render-preview')).toContainText('color_1');
    const state = await page.evaluate(() => ({
      family: _renderActiveFamily,
      mode: renderColorMode,
      metric: renderSolveMetric,
      palette: renderSolveScorePalette,
      q: document.getElementById('render-solve-score-quantile').value,
      rt: JSON.stringify(_rtChain),
    }));
    expect(state.family).toBe('color');
    expect(state.mode).toBe('solve_score');
    expect(state.metric).toBe('crowding');
    expect(state.palette).toBe('reef');
    expect(state.q).toBe('5');
    expect(state.rt).toContain('rotate_roots');
  });

  test('empty family shows no saved artifacts message', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      renderArtifactPanel('empty_job', {
        families: { color: [], bilevel: [], coeffs: [], palette: [] },
        calc: { exists: false },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
    });
    await expect(page.locator('#render-preview')).toContainText('No saved artifacts yet.');
  });
});
