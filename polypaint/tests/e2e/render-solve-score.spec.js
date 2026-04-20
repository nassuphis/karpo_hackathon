// @ts-check
const { test, expect } = require('@playwright/test');

const RENDER_POPUP_SUMMARY = {
  families: {
    color: [
      {
        artifact_id: 'color_1',
        created_at: '2026-03-30T10:00:00Z',
        image_key: 'renders/test_job/color/color_1/image.jpeg',
        image_url: 'https://example.com/c.jpeg',
        preview_url: 'https://example.com/c.png',
        viewer_url: 'https://example.com/c.png',
        file_size: 50000,
        width: 1000,
        height: 1000,
        color_mode: 'rainbow',
        format: 'jpeg',
      },
    ],
    bilevel: [],
    coeffs: [],
    palette: [
      {
        artifact_id: 'pal_1',
        palette_id: 'pal_1',
        created_at: '2026-03-30T12:00:00Z',
        image_key: 'renders/test_job/palettes/pal_1/image.jpeg',
        image_url: 'https://example.com/p.jpeg',
        preview_url: 'https://example.com/p.png',
        viewer_url: 'https://example.com/p.png',
        file_size: 40000,
        width: 1000,
        height: 1000,
        metric: 'crowding',
        palette: 'reef',
        solve_score_quantile: 0.05,
        solve_score_omega: 4,
        solve_score_omega_enabled: true,
        render_reusable: true,
        data_layout: 'chunk_all_pass_v1',
        chunk_bins_prefix: 'renders/test_job/palettes/pal_1/chunks/palette_bins_chunk_',
        display_name: 'crowding q=5.0% w=4 reef',
      },
    ],
  },
  calc: { exists: true, N: 4000, degree: 8 },
  artifacts: {},
  deepzoom_latest: { exists: false },
};

async function seedRenderPopupState(page, colorMode = 'solve_score') {
  await page.evaluate(({ summary, colorMode }) => {
    window._renderLaunches = [];
    window._launchRenderOrchestrator = async function(mode, paramsPatch) {
      window._renderLaunches.push({ mode, paramsPatch: { ...(paramsPatch || {}) } });
      return {};
    };
    document.getElementById('render-results-dir').value = 'test_job';
    _renderLoadedJobId = 'test_job';
    renderColorMode = colorMode;
    renderSolveMetric = 'crowding';
    renderSolveScorePalette = 'inferno';
    renderSavedPalette = 'pal_1';
    renderArtifactPanel('test_job', summary);
  }, { summary: RENDER_POPUP_SUMMARY, colorMode });
}

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

async function chooseSolveMetric(page, metric) {
  await page.evaluate((name) => {
    setSolveMetric(name);
  }, metric);
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

  test('Render tab shows the solve-score palette section and fused-only banner', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await expect(page.locator('.color-title', { hasText: 'Solve Score Palette' })).toBeVisible();
    await expect(page.locator('#palette-circles-solve-score')).toBeVisible();
    await expect(page.locator('text=Color render is fused-only now. Solve score is the only supported mode.')).toBeVisible();
  });

  test('solve-score modal lists and loads saved programs for the render tab', async ({ page }) => {
    await page.evaluate(() => {
      window._solveScorePrograms = {
        'proximity-q-0-1': {
          version: 1,
          id: 'proximity-q-0-1',
          name: 'Proximity q=0.1%',
          chain: [['proximity', '0.1']],
          metric: 'proximity',
          display: 'proximity(slv,0.1)',
          program_spec: 'm0',
          statement_count: 1,
          saved_at: '2026-04-20T12:00:00Z',
        },
      };
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/list-solve-score-programs') {
          return {
            programs: Object.values(window._solveScorePrograms).map((program) => ({
              id: program.id,
              name: program.name,
              statement_count: program.statement_count,
              saved_at: program.saved_at,
            })),
            count: 1,
            order: 'saved_at_desc',
          };
        }
        if (name === 'storage' && path === '/fetch-solve-score-program') {
          return { program: window._solveScorePrograms[body.id] };
        }
        throw new Error(`unexpected ${name} ${path || ''}`);
      };
    });

    await page.click('.tab-btn:text("Render")');
    await page.click('#render-solve-score-program-manage');
    await expect(page.locator('#solve-score-modal-overlay')).toBeVisible();
    await expect(page.locator('#solve-score-modal-body .tri-popup-row')).toHaveCount(1);
    await page.locator('#solve-score-modal-body .tri-popup-row').first().click();
    await expect(page.locator('#solve-score-modal-selected')).toContainText('Proximity q=0.1%');
    await page.click('#solve-score-modal-load');
    await expect(page.locator('#render-solve-score-program-status')).toContainText('Loaded Proximity q=0.1%');
    const chain = await page.evaluate(() => _serializeSolveScoreChain(_renderScoreChain));
    expect(chain).toEqual([['proximity', '0.1']]);
  });

  test('failed modal load preserves the current live solve-score chain', async ({ page }) => {
    await page.evaluate(() => {
      window._solveScorePrograms = {
        'broken-program': {
          version: 1,
          id: 'broken-program',
          name: 'Broken Program',
          chain: [['weighted_sum', '1', '2']],
          metric: 'proximity',
          display: 'broken',
          program_spec: 'broken',
          statement_count: 1,
          saved_at: '2026-04-20T12:00:00Z',
        },
      };
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/list-solve-score-programs') {
          return {
            programs: Object.values(window._solveScorePrograms).map((program) => ({
              id: program.id,
              name: program.name,
              statement_count: program.statement_count,
              saved_at: program.saved_at,
            })),
            count: 1,
            order: 'saved_at_desc',
          };
        }
        if (name === 'storage' && path === '/fetch-solve-score-program') {
          return { program: window._solveScorePrograms[body.id] };
        }
        throw new Error(`unexpected ${name} ${path || ''}`);
      };
    });

    await page.click('.tab-btn:text("Render")');
    const before = await page.evaluate(() => JSON.stringify(_serializeSolveScoreChain(_renderScoreChain)));
    await page.click('#render-solve-score-program-manage');
    await page.locator('#solve-score-modal-body .tri-popup-row').first().click();
    await page.click('#solve-score-modal-load');
    await expect(page.locator('#solve-score-modal-status')).toContainText('weighted_sum requires 2 inputs');
    const after = await page.evaluate(() => JSON.stringify(_serializeSolveScoreChain(_renderScoreChain)));
    expect(after).toBe(before);
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

  test('repalette popup uses PAL/TRI/LONG buttons and nested selectors stay accessible', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'j';
      _renderLoadedJobId = 'j';
      _renderActiveFamily = 'palette';
      _renderSelectedArtifact = { color: -1, bilevel: -1, coeffs: -1, palette: -1 };
      renderArtifactPanel('j', {
        calc: { exists: true, N: 4000, degree: 8 },
        families: {
          color: [],
          bilevel: [],
          coeffs: [],
          palette: [{
            artifact_id: 'pal_src',
            palette_id: 'pal_src',
            display_name: 'crowding q=5.0% w=4 reef',
            created_at: '2026-04-02T10:00:00Z',
            image_key: 'renders/j/palettes/pal_src/image.jpeg',
            image_url: 'https://img/pal_src.jpeg',
            preview_url: 'https://img/pal_src.png',
            viewer_url: 'https://img/pal_src.png',
            width: 4000,
            height: 4000,
            file_size: 90000,
            metric: 'crowding',
            palette: 'reef',
            solve_score_quantile: 0.05,
            solve_score_omega: 4,
            render_reusable: true,
            data_layout: 'chunk_all_pass_v1',
            chunk_bins_prefix: 'renders/j/palettes/pal_src/chunks/palette_bins_chunk_',
          }],
        },
      });
    });
    await page.locator('#btn-render-repalette').click();
    await expect(page.locator('#repalette-popup-overlay')).toBeVisible();
    const swatches = page.locator('#palette-circles-repalette .pal-circle');
    await expect(swatches).toHaveCount(3);
    await expect(swatches.nth(0)).toContainText('PAL');
    await expect(swatches.nth(1)).toContainText('TRI');
    await expect(swatches.nth(2)).toContainText('LONG');

    await page.locator('#palette-circles-repalette [data-palette-popup="builtin"]').click();
    await expect(page.locator('#builtin-popup-overlay')).toBeVisible();
    await expect(page.locator('#builtin-popup-title')).toContainText('RePalette');
    await page.locator('#builtin-popup-close').click();

    await page.locator('#palette-circles-repalette .pal-circle-tri').click();
    await expect(page.locator('#tri-popup-overlay')).toBeVisible();
    await expect(page.locator('#tri-popup-title')).toContainText('RePalette');
    await page.locator('#tri-popup-close').click();

    await page.locator('#palette-circles-repalette .pal-circle-long').click();
    await expect(page.locator('#long-popup-overlay')).toBeVisible();
    await expect(page.locator('#long-popup-title')).toContainText('RePalette');
    await page.locator('#long-popup-close').click();
  });

  test('Solve score chip adder shows metrics when empty, then metrics plus unary ops after one metric chip', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const state = await page.evaluate(() => {
      _renderScoreChain.splice(0, _renderScoreChain.length);
      _renderChips('ss');
      return Array.from(document.querySelectorAll('#ss-add option')).map(o => ({
        text: o.textContent.trim(),
        value: o.value,
      }));
    });
    expect(state).toEqual([
      { text: '+ add...', value: '' },
      { text: 'proximity', value: 'proximity' },
      { text: 'crowding', value: 'crowding' },
      { text: 'spread', value: 'spread' },
      { text: 'anisotropy', value: 'anisotropy' },
      { text: 'area', value: 'area' },
      { text: 'clusteriness', value: 'clusteriness' },
      { text: 'shelliness', value: 'shelliness' },
      { text: 'outlierness', value: 'outlierness' },
      { text: 'nn_variation', value: 'nn_variation' },
      { text: 'real_axis_proximity', value: 'real_axis_proximity' },
      { text: 'centroid_re', value: 'centroid_re' },
      { text: 'centroid_im', value: 'centroid_im' },
      { text: 'centroid_dist', value: 'centroid_dist' },
      { text: 'dist_unit_circle', value: 'dist_unit_circle' },
      { text: 'asymmetry_re', value: 'asymmetry_re' },
      { text: 'min_mod', value: 'min_mod' },
      { text: 'max_mod', value: 'max_mod' },
      { text: 'min_angular_separation', value: 'min_angular_separation' },
      { text: 't1_re', value: 't1_re' },
      { text: 't1_im', value: 't1_im' },
      { text: 't1_abs', value: 't1_abs' },
      { text: 't1_phase', value: 't1_phase' },
      { text: 't2_re', value: 't2_re' },
      { text: 't2_im', value: 't2_im' },
      { text: 't2_abs', value: 't2_abs' },
      { text: 't2_phase', value: 't2_phase' },
    ]);

    await chooseSolveMetric(page, 'crowding');
    const afterMetric = await page.evaluate(() =>
      Array.from(document.querySelectorAll('#ss-add option')).map(o => ({ text: o.textContent.trim(), value: o.value }))
    );
    expect(afterMetric).toEqual([
      { text: '+ add...', value: '' },
      { text: 'proximity', value: 'proximity' },
      { text: 'crowding', value: 'crowding' },
      { text: 'spread', value: 'spread' },
      { text: 'anisotropy', value: 'anisotropy' },
      { text: 'area', value: 'area' },
      { text: 'clusteriness', value: 'clusteriness' },
      { text: 'shelliness', value: 'shelliness' },
      { text: 'outlierness', value: 'outlierness' },
      { text: 'nn_variation', value: 'nn_variation' },
      { text: 'real_axis_proximity', value: 'real_axis_proximity' },
      { text: 'centroid_re', value: 'centroid_re' },
      { text: 'centroid_im', value: 'centroid_im' },
      { text: 'centroid_dist', value: 'centroid_dist' },
      { text: 'dist_unit_circle', value: 'dist_unit_circle' },
      { text: 'asymmetry_re', value: 'asymmetry_re' },
      { text: 'min_mod', value: 'min_mod' },
      { text: 'max_mod', value: 'max_mod' },
      { text: 'min_angular_separation', value: 'min_angular_separation' },
      { text: 't1_re', value: 't1_re' },
      { text: 't1_im', value: 't1_im' },
      { text: 't1_abs', value: 't1_abs' },
      { text: 't1_phase', value: 't1_phase' },
      { text: 't2_re', value: 't2_re' },
      { text: 't2_im', value: 't2_im' },
      { text: 't2_abs', value: 't2_abs' },
      { text: 't2_phase', value: 't2_phase' },
      { text: 'omega_cosine', value: 'omega_cosine' },
      { text: 'sawtooth', value: 'sawtooth' },
      { text: 'flip', value: 'flip' },
    ]);
  });

  test('choosing a solve-score palette keeps solve_score mode active', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await chooseBuiltinPalette(page, 'solve_score', 'inferno');
    const mode = await page.evaluate(() => renderColorMode);
    expect(mode).toBe('solve_score');
  });

  test('choosing solve-score metric updates renderSolveMetric', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await chooseSolveMetric(page, 'crowding');
    const metric = await page.evaluate(() => renderSolveMetric);
    expect(metric).toBe('crowding');

    await chooseSolveMetric(page, 'area');
    const metric2 = await page.evaluate(() => renderSolveMetric);
    expect(metric2).toBe('area');
  });

  test('render dispatch payload contains selected solve_metric', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');

    await chooseSolveMetric(page, 'spread');

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
    await chooseSolveMetric(page, 'clusteriness');

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

  test('solve-score metric chip shows source+q controls and syncs the hidden compatibility field', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const chipSource = page.locator('#ss-chips select').first();
    const chipQ = page.locator('#ss-chips input').first();
    await expect(chipSource).toBeVisible();
    await expect(chipQ).toBeVisible();
    expect(await chipSource.inputValue()).toBe('slv');
    expect(await chipQ.inputValue()).toBe('0.1');
    expect(await page.locator('#render-solve-score-quantile').inputValue()).toBe('0.1');
  });

  test('changing solve-score metric q updates the hidden compatibility field and the visible chip input', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      const input = document.querySelector('#ss-chips input');
      input.value = '3.0';
      input.dispatchEvent(new Event('change'));
    });
    expect(await page.locator('#render-solve-score-quantile').inputValue()).toBe('3');
    await expect(page.locator('#ss-chips input').first()).toHaveValue('3.0');
  });

  test('dispatch payload includes solve_score_quantile', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      const input = document.querySelector('#ss-chips input');
      input.value = '2.0';
      input.dispatchEvent(new Event('change'));
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
    // Set viewport quantile to 2.0, solve-score metric q to 4.0
    await page.evaluate(() => {
      document.getElementById('render-quantile').value = '2.0';
      const input = document.querySelector('#ss-chips input');
      input.value = '4.0';
      input.dispatchEvent(new Event('change'));
    });
    const vq = await page.locator('#render-quantile').inputValue();
    const sq = await page.locator('#render-solve-score-quantile').inputValue();
    expect(vq).toBe('2');
    expect(sq).toBe('4');
  });

  test('real_axis_proximity dispatch sends correct solve_metric', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await chooseSolveMetric(page, 'real_axis_proximity');

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
            fully_finite_solve_count: 80, partial_finite_solve_count: 15, zero_finite_solve_count: 5,
            usable_solve_count: 90, forced_zero_score_count: 10,
            finite_root_frac: 0.92, fully_finite_solve_frac: 0.8, partial_finite_solve_frac: 0.15, zero_finite_solve_frac: 0.05, usable_solve_frac: 0.9,
            exact_zero_root_count: 18,
            rows_with_any_exact_zero_root_count: 12,
            rows_all_exact_zero_roots_count: 2,
            exact_zero_root_frac: 0.036, rows_with_any_exact_zero_root_frac: 0.12, rows_all_exact_zero_roots_frac: 0.02,
            mean_finite_roots_per_solve: 4.6, min_finite_roots_per_solve: 0, max_finite_roots_per_solve: 5,
            metric_validity_policy: 'finite_only_min_roots',
            metric_min_finite_roots: 2,
            total_root_slots: 500, finite_root_count: 460,
            raw_hist_bins: 32,
            raw_hist_lo: -1,
            raw_hist_hi: 2,
            raw_hist_range: 3,
            raw_hist_space: 'metric_raw',
            raw_hist_expanded: false,
            raw_bin_counts: [4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,6],
            raw_bin_fracs: Array(32).fill(1 / 32),
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
    expect(logText).toContain('raw score bins (32');
    expect(logText).toContain('r00');
    expect(logText).toContain('r31');
    expect(logText).toContain('lores rows: total=100  all_finite=80 (80.0%)  partial=15 (15.0%)  no_finite=5 (5.0%)');
    expect(logText).toContain('exact zeros: roots=18/500 (3.6%)  rows_any=12 (12.0%)  rows_all=2 (2.0%)');
    expect(logText).toContain('final color bins (10');
    expect(logText).toContain('b0');
    expect(logText).toContain('b9');
    expect(logText).toContain('clip');
    expect(logText).not.toContain('full range');
  });

  test('render panel shows family tabs and selected-artifact actions', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_job';
      _renderLoadedJobId = 'test_job';
      renderArtifactPanel('test_job', {
        families: {
          color: [{ artifact_id: 'color_1', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/test_job/color/color_1/image.jpeg', image_url: 'https://example.com/c.jpeg', preview_url: 'https://example.com/c.png', viewer_url: 'https://example.com/c.png', file_size: 50000, width: 1000, height: 1000, color_mode: 'rainbow', format: 'jpeg', associated_palette_id: 'pal_1' }],
          bilevel: [{ artifact_id: 'bilevel_1', created_at: '2026-03-30T11:00:00Z', image_key: 'renders/test_job/bilevel/bilevel_1/image.tif', image_url: 'https://example.com/b.tif', preview_url: 'https://example.com/b.png', viewer_url: 'https://example.com/b.png', file_size: 60000, width: 1000, height: 1000, format: 'tif' }],
          coeffs: [{ artifact_id: 'coeffs_1', created_at: '2026-03-30T11:30:00Z', image_key: 'renders/test_job/coeffs/coeffs_1/image.tif', image_url: 'https://example.com/co.tif', preview_url: 'https://example.com/co.png', viewer_url: 'https://example.com/co.png', file_size: 70000, width: 1000, height: 1000, format: 'tif' }],
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', created_at: '2026-03-30T12:00:00Z', image_key: 'renders/test_job/palettes/pal_1/image.jpeg', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png', viewer_url: 'https://example.com/p.png', file_size: 40000, width: 1000, height: 1000, metric: 'crowding', palette: 'reef', solve_score_quantile: 0.05, derived_from_color_artifact_id: 'color_1' }],
        },
        calc: { exists: true, N: 1000, degree: 5 },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
    });

    const panel = page.locator('#render-preview');
    await expect(panel.locator('button[data-render-family="color"]')).toBeVisible();
    await expect(panel.locator('button[data-render-family="bilevel"]')).toBeVisible();
    await expect(panel.locator('button[data-render-family="coeffs"]')).toBeVisible();
    await expect(panel.locator('button[data-render-family="palette"]')).toBeVisible();
    await expect(panel.locator('#btn-render-generate')).toBeVisible();
    await expect(panel.locator('#btn-render-populate')).toBeVisible();
    await expect(panel.locator('#btn-render-download')).toBeVisible();
    await expect(panel.locator('#btn-render-delete')).toBeVisible();
    await expect(panel.locator('#btn-render-deepzoom')).toBeVisible();
    await expect(panel.locator('#btn-render-go-palette')).toHaveText('GoPalette: pal_1');
    await expect(panel.locator('img[src="https://example.com/c.png"]')).toBeVisible();
    await expect(panel.locator('text=[P pal_1]')).toBeVisible();
    await expect(panel.locator('text=id=color_1')).toBeVisible();
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
      document.getElementById('render-results-dir').value = 'test_job';
      _renderLoadedJobId = 'test_job';
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

    await page.click('#render-preview button[data-render-family="palette"]');
    await expect.poll(async () => page.evaluate(() => _renderActiveFamily)).toBe('palette');
    await expect(page.locator('#render-preview img[src="https://example.com/p.png"]')).toBeVisible();
  });

  test('GoPalette and GoColor navigate linked render artifacts', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_job';
      _renderLoadedJobId = 'test_job';
      renderArtifactPanel('test_job', {
        families: {
          color: [{ artifact_id: 'color_1', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/test_job/color/color_1/image.jpeg', image_url: 'https://example.com/c.jpeg', preview_url: 'https://example.com/c.png', viewer_url: 'https://example.com/c.png', file_size: 50000, width: 1000, height: 1000, color_mode: 'rainbow', format: 'jpeg', associated_palette_id: 'pal_1' }],
          bilevel: [],
          coeffs: [],
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', created_at: '2026-03-30T12:00:00Z', image_key: 'renders/test_job/palettes/pal_1/image.jpeg', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png', viewer_url: 'https://example.com/p.png', file_size: 40000, width: 1000, height: 1000, metric: 'crowding', palette: 'reef', solve_score_quantile: 0.05, derived_from_color_artifact_id: 'color_1' }],
        },
        calc: { exists: true, N: 1000, degree: 5 },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
    });

    await page.click('#btn-render-go-palette');
    await expect.poll(async () => page.evaluate(() => _renderActiveFamily)).toBe('palette');
    await expect(page.locator('#btn-render-go-color')).toHaveText('GoColor: color_1');
    await expect(page.locator('text=[C color_1]')).toBeVisible();

    await page.click('#btn-render-go-color');
    await expect.poll(async () => page.evaluate(() => _renderActiveFamily)).toBe('color');
    await expect(page.locator('#btn-render-go-palette')).toHaveText('GoPalette: pal_1');
  });

  test('palette family populate restores solve-score settings and switches to color', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_job';
      _renderLoadedJobId = 'test_job';
      renderColorMode = 'rainbow';
      renderSolveMetric = 'proximity';
      renderSolveScorePalette = 'inferno';
      _rtChain = [];
      document.getElementById('render-solve-score').value = 'proximity';
      document.getElementById('render-solve-score-quantile').value = '0.1';
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

    await expect(page.locator('#render-preview img[src="https://example.com/c.png"]')).toBeVisible();
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

  test('Generate popup exposes associated palette control and dispatches fused solve-score settings', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await seedRenderPopupState(page, 'solve_score');

    await page.click('#btn-render-generate');
    const popup = page.locator('#render-generate-popup-overlay');
    await expect(popup).toBeVisible();
    await expect(page.locator('#render-generate-save-associated-palette')).toBeVisible();

    await page.check('#render-generate-save-associated-palette');
    await page.click('#render-generate-popup-run');

    const launches = await page.evaluate(() => window._renderLaunches);
    expect(launches).toHaveLength(1);
    expect(launches[0]).toEqual({
      mode: 'color',
      paramsPatch: {
        raster_engine: 'single',
        save_associated_palette: true,
      },
    });
  });

  test('Generate-MT popup exposes retries and associated palette and dispatches MT payload', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await seedRenderPopupState(page, 'solve_score');

    await page.click('#btn-render-generate-mt');
    const popup = page.locator('#render-mt-popup-overlay');
    await expect(popup).toBeVisible();
    await expect(page.locator('#render-mt-hist-retries')).toBeVisible();
    await expect(page.locator('#render-mt-raster-retries')).toBeVisible();
    await expect(page.locator('#render-mt-save-associated-palette')).toBeVisible();

    await page.fill('#render-mt-solve-score-threads', '6');
    await page.selectOption('#render-mt-hist-input-mode', 'sectioned');
    await page.fill('#render-mt-hist-retries', '3');
    await page.fill('#render-mt-threads', '7');
    await page.selectOption('#render-mt-raster-input-mode', 'sectioned');
    await page.fill('#render-mt-raster-retries', '4');
    await page.fill('#render-mt-merge-workers', '18');
    await page.fill('#render-mt-finalize-workers', '19');
    await page.check('#render-mt-save-associated-palette');
    await page.click('#render-mt-popup-run');

    const launches = await page.evaluate(() => window._renderLaunches);
    expect(launches).toHaveLength(1);
    expect(launches[0]).toEqual({
      mode: 'color',
      paramsPatch: {
        raster_engine: 'mt',
        raster_mt_threads: 7,
        solve_score_threads: 6,
        solve_score_hist_input_mode: 'sectioned',
        solve_score_hist_retries: 3,
        raster_input_mode: 'sectioned',
        raster_sectioned_retries: 4,
        solve_score_merge_workers: 18,
        finalize_workers: 19,
        save_associated_palette: true,
      },
    });
  });

  test('ExtractPalette popup dispatches palette orchestrator for selected color artifact lineage', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate((summary) => {
      window._extractDispatch = null;
      window.lambdaPost = async function(name, body, path) {
        if (name === 'dispatch' && body.target === 'palette_orchestrator') {
          window._extractDispatch = body;
          return { fired: 1, errors: [] };
        }
        return {};
      };
      document.getElementById('render-results-dir').value = 'test_job';
      _renderLoadedJobId = 'test_job';
      _activeRenderRun = null;
      _activePaletteRun = null;
      renderArtifactPanel('test_job', summary);
    }, {
      ...RENDER_POPUP_SUMMARY,
      families: {
        ...RENDER_POPUP_SUMMARY.families,
        color: [{
          ...RENDER_POPUP_SUMMARY.families.color[0],
          color_mode: 'solve_score',
          solve_metric: 'spread',
          solve_score_quantile: 0.02,
          solve_score_omega: 6,
          palette: 'magma',
        }],
      },
    });

    const btn = page.locator('#btn-render-extract-palette');
    await expect(btn).toBeEnabled();
    await btn.click();
    await expect(page.locator('#extract-palette-popup-overlay')).toBeVisible();
    await page.fill('#extract-palette-solve-score-threads', '8');
    await page.selectOption('#extract-palette-hist-input-mode', 'sectioned');
    await page.fill('#extract-palette-hist-retries', '5');
    await page.fill('#extract-palette-merge-workers', '24');
    await page.fill('#extract-palette-chunk-threads', '6');
    await page.selectOption('#extract-palette-chunk-input-mode', 'sectioned');
    await page.fill('#extract-palette-chunk-retries', '4');
    await page.fill('#extract-palette-chunk-workers', '32');
    await page.click('#extract-palette-popup-run');

    const dispatch = await page.evaluate(() => window._extractDispatch);
    const activeRun = await page.evaluate(() => _loadActivePaletteRun());
    expect(dispatch).not.toBeNull();
    expect(dispatch.target).toBe('palette_orchestrator');
    expect(dispatch.jobs[0].artifact_id).toBe('color_1');
    expect(dispatch.jobs[0].params).toEqual({
      solve_score_threads: 8,
      solve_score_hist_input_mode: 'sectioned',
      solve_score_hist_retries: 5,
      solve_score_merge_workers: 24,
      palette_chunk_threads: 6,
      palette_chunk_input_mode: 'sectioned',
      palette_chunk_retries: 4,
      palette_chunk_workers: 32,
    });
    expect(activeRun.mode).toBe('extract_palette');
    expect(activeRun.origin).toBe('render_extract_palette');
    expect(activeRun.source_artifact_id).toBe('color_1');
  });

  test('ExtractPalette completion uses named elapsed log and Autolevels completion is named', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate((summary) => {
      window.refreshRenderArtifacts = async function() {};
      window.loadPaletteInventory = async function() {};
      window._logRenderRasterPerf = async function() {};
      window.renderArtifactPanel('test_job', summary);
      _renderLoadedJobId = 'test_job';
      document.getElementById('render-results-dir').value = 'test_job';
      _renderActiveFamily = 'color';
      _renderSelectedArtifact = { color: 0, bilevel: -1, coeffs: -1, palette: -1, pdf: -1 };
      _clearActivePaletteRun();
      _saveActivePaletteRun({
        job_id: 'test_job',
        run_id: 'run_extract_done',
        task_id: 'extract_palette_run_run_extract_done',
        started_at_ms: 1000,
        mode: 'extract_palette',
        origin: 'render_extract_palette',
        source_artifact_id: 'color_1',
        server_started_at_ms: 2000,
      });
    }, {
      ...RENDER_POPUP_SUMMARY,
      families: {
        ...RENDER_POPUP_SUMMARY.families,
        color: [{
          ...RENDER_POPUP_SUMMARY.families.color[0],
          associated_palette_id: 'pal_1',
        }],
      },
    });

    await page.evaluate(() => _pollActivePaletteRun = async function() {
      const run = _activePaletteRun || _loadActivePaletteRun();
      const statusEl = document.getElementById('palette-status');
      const mirrorToRender = true;
      const rd = {
        phase: 'done',
        phase_label: 'Done',
        family: 'palette',
        palette_id: 'pal_extract',
        artifact_id: 'color_1',
        updated_at_ms: 15000,
        run_started_at_ms: 2000,
      };
      document.getElementById('palette-results-dir').value = run.job_id;
      await loadPaletteInventory({ selectPaletteId: rd.palette_id });
      await refreshRenderArtifacts(run.job_id, { selectFamily: 'color', selectArtifactId: rd.artifact_id || run.source_artifact_id || null });
      statusEl.textContent = 'ExtractPalette complete';
      statusEl.className = 'status ok';
      const completeMsg = _paletteRunCompleteLog(run, rd);
      log(completeMsg, 'ok', 'palette-log');
      if (mirrorToRender) {
        const renderStatusEl = document.getElementById('render-status');
        if (renderStatusEl) {
          renderStatusEl.textContent = 'ExtractPalette complete';
          renderStatusEl.className = 'status ok';
        }
        log(completeMsg, 'ok', 'render-log');
      }
      _clearActivePaletteRun();
    });
    await page.evaluate(async () => { await _pollActivePaletteRun(); });
    await expect(page.locator('#render-status')).toHaveText('ExtractPalette complete');
    await expect(page.locator('#render-log')).toContainText('ExtractPalette complete: pal_extract (13.0s)');

    await page.evaluate(() => {
      window.refreshRenderArtifacts = async function() {};
      window._logRenderRasterPerf = async function() {};
      _saveActiveRun({
        job_id: 'test_job',
        run_id: 'run_auto_done',
        task_id: 'render_run_autolevels_run_auto_done',
        started_at_ms: 1000,
        server_started_at_ms: 2000,
        mode: 'autolevels',
      });
    });
    await page.evaluate(async () => {
      await _handleRenderRunCompletion(_activeRenderRun, {
        family: 'color',
        artifact_id: 'autolevels_done',
        updated_at_ms: 15000,
        run_started_at_ms: 2000,
      });
    });
    await expect(page.locator('#render-status')).toHaveText('Autolevels complete');
    await expect(page.locator('#render-log')).toContainText('Autolevels complete: autolevels_done (13.0s)');
  });
});
