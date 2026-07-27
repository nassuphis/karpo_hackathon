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
        solve_score_chain: [['crowding', '5'], ['omega_cosine', '4']],
        render_reusable: true,
        data_layout: 'chunk_all_pass_v1',
        chunk_bins_prefix: 'renders/test_job/palettes/pal_1/chunks/palette_bins_chunk_',
        display_name: 'crowding q=5.0% w=4 reef',
      },
    ],
  },
  // Keep popup tests self-contained: an empty object means the summary was
  // loaded but does not provide enough data for an automatic section estimate.
  calc: { exists: true, N: 4000, degree: 8, job_size: {} },
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
    window._launchFusedRenderOrchestrator = async function(paramsPatch) {
      window._renderLaunches.push({ mode: 'color', paramsPatch: { ...(paramsPatch || {}) } });
      return {};
    };
    window._launchNonColorRenderOrchestrator = async function(mode, paramsPatch) {
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
  // selection keeps the popup open now — close so the page is clickable
  await page.click('#builtin-popup-close');
  await expect(page.locator('#builtin-popup-overlay')).not.toBeVisible();
}

async function chooseLongPalette(page, mode, name) {
  const swatch = page.locator(`${renderPaletteContainer(mode)} .pal-circle-long`);
  await swatch.click();
  await expect(page.locator('#long-popup-overlay')).toBeVisible();
  const row = page.locator('#long-popup-body .tri-popup-row').filter({ hasText: name }).first();
  await expect(row).toBeVisible();
  await row.click();
  await page.click('#long-popup-close');
  await expect(page.locator('#long-popup-overlay')).not.toBeVisible();
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

  test('Render tab shows the color section and fused-only banner', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await expect(page.locator('.color-title', { hasText: 'Color' })).toBeVisible();
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
        if (name === 'storage' && path === '/solve-score-chain-to-source') {
          return { ok: true, source_text: 'score = metric(proximity, slv, q=0.1%)' };
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
    // chain-only saved program: the card shows the SAME derived source text
    // that loading will put in the textbox (not a chip strip)
    await expect(page.locator('#solve-score-modal-selected .coeff-program-modal-source'))
      .toContainText('score = metric(proximity, slv, q=0.1%)');
    await page.click('#solve-score-modal-load');
    await expect(page.locator('#render-solve-score-program-status')).toContainText('Loaded Proximity q=0.1%');
    await expect(page.locator('#solve-score-modal-name')).toHaveValue('Proximity q=0.1%');
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

  test('render solve-score row collapses built-ins into popup selectors', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const solveCircles = page.locator('#palette-circles-solve-score .pal-circle');
    const solveCount = await solveCircles.count();
    expect(solveCount).toBe(6);   // PAL, TRI, LONG, HEX (custom), MIC, PIC
  });

  test('render solve-score row shows built-in, TRI, LONG, and HEX swatches', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await expect(page.locator('#palette-circles-solve-score [data-palette-popup="builtin"]')).toBeVisible();
    await expect(page.locator('#palette-circles-solve-score .pal-circle-tri')).toBeVisible();
    await expect(page.locator('#palette-circles-solve-score .pal-circle-long')).toBeVisible();
    await expect(page.locator('#palette-circles-solve-score .pal-circle-custom')).toBeVisible();
    await expect(page.locator('#palette-circles-solve-score .pal-custom-input')).toHaveCount(0);
  });

  test('named custom palette catalog selects, edits, and saves with CAS provenance', async ({ page }) => {
    await page.evaluate(() => {
      window._customPaletteRequests = [];
      window.lambdaPost = async function(name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list-custom-palettes') {
          return {
            revision: 'etag-1',
            palettes: [{
              name: 'Night reef',
              stops: ['879caa', 'aaa4a4', '0e3057'],
              palette: 'custom:879caa-aaa4a4-0e3057',
            }],
          };
        }
        if (path === '/save-custom-palettes') {
          window._customPaletteRequests.push(JSON.parse(JSON.stringify(body)));
          return {
            revision: 'etag-2',
            palettes: body.palettes.map((entry) => ({
              name: entry.name,
              stops: entry.stops,
              palette: 'custom:' + entry.stops.join('-'),
            })),
          };
        }
        throw new Error(`unexpected storage path ${path}`);
      };
    });

    await page.click('.tab-btn:text("Render")');
    await page.locator('#palette-circles-solve-score .pal-circle-custom').click();
    await expect(page.locator('#custom-palette-popup-overlay')).toBeVisible();
    await expect(page.locator('#custom-palette-popup-body .tri-popup-row')).toHaveCount(1);
    await page.locator('#custom-palette-popup-body .tri-popup-row').click();
    const active = await page.evaluate(() => ({
      palette: renderSolveScorePalette,
      displayName: _activeRenderPaletteDisplayName(),
    }));
    expect(active).toEqual({
      palette: 'custom:879caa-aaa4a4-0e3057',
      displayName: 'Night reef',
    });
    await expect(page.locator('#palette-circles-solve-score .pal-circle-custom')).toHaveClass(/active/);

    await page.locator('#custom-palette-popup-new').click();
    await page.locator('#custom-palette-popup-name').fill('Signal fire');
    await page.locator('#custom-palette-popup-hex').fill('#ff3300, #ffd166, #101820');
    await page.locator('#custom-palette-popup-save').click();
    await expect(page.locator('#custom-palette-popup-status')).toContainText('Saved 2 custom palettes');
    const request = await page.evaluate(() => window._customPaletteRequests[0]);
    expect(request.expected_revision).toBe('etag-1');
    expect(request.palettes[1]).toEqual({
      name: 'Signal fire',
      stops: ['ff3300', 'ffd166', '101820'],
    });
  });

  test('custom palette save conflict preserves the draft for retry', async ({ page }) => {
    await page.evaluate(() => {
      window._customPaletteListCount = 0;
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/list-custom-palettes') {
          window._customPaletteListCount += 1;
          return window._customPaletteListCount === 1
            ? { revision: 'old-etag', palettes: [] }
            : {
                revision: 'new-etag',
                palettes: [{
                  name: 'Server palette',
                  stops: ['102030', '405060'],
                  palette: 'custom:102030-405060',
                }],
              };
        }
        if (name === 'storage' && path === '/save-custom-palettes') {
          throw new Error('HTTP 409: custom palette catalog changed');
        }
        throw new Error(`unexpected ${name} ${path || ''}`);
      };
    });
    await page.click('.tab-btn:text("Render")');
    await page.locator('#palette-circles-solve-score .pal-circle-custom').click();
    await page.locator('#custom-palette-popup-new').click();
    await page.locator('#custom-palette-popup-name').fill('Unsaved draft');
    await page.locator('#custom-palette-popup-hex').fill('#112233, #445566');
    await page.locator('#custom-palette-popup-save').click();
    await expect(page.locator('#custom-palette-popup-status')).toContainText('HTTP 409');
    await expect(page.locator('#custom-palette-popup-name')).toHaveValue('Unsaved draft');
    await expect(page.locator('#custom-palette-popup-save')).toBeEnabled();

    page.once('dialog', dialog => dialog.accept());
    await page.locator('#custom-palette-popup-close').click();
    await page.locator('#palette-circles-solve-score .pal-circle-custom').click();
    await expect(page.locator('#custom-palette-popup-body')).toContainText('Server palette');
    expect(await page.evaluate(() => window._customPaletteListCount)).toBe(2);
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
    await expect(page.locator('#builtin-popup-overlay')).toBeVisible();   // stays open on select
    await page.click('#builtin-popup-close');
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
    await expect(page.locator('#tri-popup-overlay')).toBeVisible();   // stays open on select
    await page.click('#tri-popup-close');
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
    await expect(page.locator('#long-popup-overlay')).toBeVisible();   // stays open on select
    await page.click('#long-popup-close');
    await expect(page.locator('#long-popup-overlay')).not.toBeVisible();
    await expect(longSwatch).toHaveAttribute('title', /marvel_spiderman_long/);
  });

  test('right-click TRI activates remembered solve-score palette without opening popup', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      renderSolveScoreTriName = 'redgold';
      renderSolveScorePalette = 'inferno';
      buildPaletteCircles('palette-circles-solve-score', 'solve_score', () => renderSolveScorePalette);
    });
    const tri = page.locator('#palette-circles-solve-score .pal-circle-tri');
    await tri.click({ button: 'right' });
    const palette = await page.evaluate(() => renderSolveScorePalette);
    expect(palette).toBe('tri_redgold');
    await expect(page.locator('#tri-popup-overlay')).not.toBeVisible();
  });

  test('switching built-in palette does not erase remembered TRI selection', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const tri = page.locator('#palette-circles-solve-score .pal-circle-tri');
    await tri.click();
    await page.locator('#tri-popup-filter').fill('rg');
    await page.locator('#tri-popup-body .tri-popup-row').first().click();
    await page.click('#tri-popup-close');
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

  test('render tab does not expose a separate root-proximity palette row', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await expect(page.locator('#palette-circles-root-proximity')).toHaveCount(0);
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
            solve_score_chain: [['crowding', '5'], ['omega_cosine', '4']],
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
    await expect(swatches).toHaveCount(6);
    await expect(swatches.nth(0)).toContainText('PAL');
    await expect(swatches.nth(1)).toContainText('TRI');
    await expect(swatches.nth(2)).toContainText('LONG');
    await expect(swatches.nth(3)).toContainText('HEX');
    await expect(swatches.nth(4)).toContainText('MIC');
    await expect(swatches.nth(5)).toContainText('PIC');

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

    // MIC must stack ABOVE the repalette overlay like the other pickers
    // (z-index 1200 elevation list) — a covered popup fails these clicks
    // with "intercepts pointer events".
    await page.locator('#palette-circles-repalette .pal-circle-mic').click();
    await expect(page.locator('#mic-popup-overlay')).toBeVisible();
    await expect(page.locator('#mic-popup-title')).toContainText('RePalette');
    await expect(page.locator('#mic-popup-status')).toContainText('palettes', { timeout: 20000 });
    await page.fill('#mic-popup-filter', 'kandinsky points');
    await page.locator('#mic-popup-body .tri-popup-row').first().click();
    await expect(page.locator('#mic-popup-overlay')).toBeVisible();   // stays open on select
    await page.click('#mic-popup-close');
    await expect(page.locator('#mic-popup-overlay')).toBeHidden();
    const repaletteApplied = await page.evaluate(() => ({
      palette: _currentPaletteForMode('repalette'),
      displayName: _paletteDisplayNameForMode('repalette'),
    }));
    expect(repaletteApplied.palette).toMatch(/^custom:[0-9a-f]{6}(-[0-9a-f]{6})+$/);
    expect(repaletteApplied.displayName).toContain('Kandinsky');
    await expect(page.locator('#repalette-popup-overlay')).toBeVisible();
  });

  test('solve-score Help reference lists every metric in the editor sidepanel', async ({ page }) => {
    // The chip adder is gone (text-first editors); the vocabulary surface
    // is the generated Help tab. Every metric must appear there.
    await page.click('.tab-btn:text("Render")');
    const missing = await page.evaluate(() => {
      _setProgramSourceSidePanelMode('render-ss', 'help');
      const html = document.getElementById('render-ss-help').innerHTML;
      return _solveScoreMetricNames.filter(name => !html.includes(`metric(${name},`));
    });
    expect(missing).toEqual([]);
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

  test('render dispatch payload carries the solve-score source text', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');

    await chooseSolveMetric(page, 'spread');

    await page.evaluate(() => {
      _setSolveScoreProgramSourceText('render', 'score = metric(spread, slv, q=0.1%)\n');
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
      _setRootProgramSourceText('render', '');
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const payload = await page.evaluate(() => window._orchPayload);
    expect(payload).not.toBeNull();
    expect(payload.mode).toBe('color');
    expect(payload.params.color_mode).toBe('solve_score');
    expect(payload.params.solve_score_program_source_text).toContain('metric(spread');
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'solve_score_chain')).toBe(false);
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'solve_metric')).toBe(false);
  });

  test('clusteriness dispatch carries the solve-score source text', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await chooseSolveMetric(page, 'clusteriness');

    await page.evaluate(() => {
      _setSolveScoreProgramSourceText('render', 'score = metric(clusteriness, slv, q=0.1%)\n');
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
      _setRootProgramSourceText('render', '');
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const payload = await page.evaluate(() => window._orchPayload);
    expect(payload).not.toBeNull();
    expect(payload.params.color_mode).toBe('solve_score');
    expect(payload.params.solve_score_program_source_text).toContain('metric(clusteriness');
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'solve_score_chain')).toBe(false);
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'solve_metric')).toBe(false);
  });

  test('boot solve-score chain syncs the hidden compatibility fields', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    expect(await page.locator('#render-solve-score').inputValue()).toBe('proximity');
    expect(await page.locator('#render-solve-score-quantile').inputValue()).toBe('0.1');
  });

  test('compiling source with a new q updates the hidden compatibility field', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(async () => {
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/compile-solve-score-program-source') {
          return { ok: true, statement_count: 1, chain: [['proximity', '3']], program: { chain: [['proximity', '3']], statement_count: 1 } };
        }
        return {};
      };
      _setSolveScoreProgramSourceText('render', 'score = metric(proximity, slv, q=3%)\n');
      await _compileSolveScoreSourceEditor('render');
    });
    expect(await page.locator('#render-solve-score-quantile').inputValue()).toBe('3');
  });

  test('dispatch payload carries the edited solve-score quantile', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      _setSolveScoreProgramSourceText('render', 'score = metric(proximity, slv, q=2%)\n');
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
      _setRootProgramSourceText('render', '');
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const payload = await page.evaluate(() => window._qPayload);
    expect(payload).not.toBeNull();
    expect(payload.params.solve_score_program_source_text).toContain('q=2%');
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'solve_score_quantile')).toBe(false);
  });

  test('explicit viewport dispatch sends exact bounds', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      window._explicitPayload = null;
      window.lambdaPost = async function(name, body, path) {
        if (name === 'dispatch' && body.target === 'render_orchestrator') {
          window._explicitPayload = body.jobs[0];
          return { fired: 1, errors: [] };
        }
        if (name === 'storage' && path === '/check-status') {
          return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
        }
        return {};
      };
      window.refreshRenderArtifacts = async function() {};
      document.getElementById('render-results-dir').value = 'test_explicit';
      document.getElementById('render-pix').value = '512';
      selectViewMode('explicit');
      document.getElementById('render-min-re').value = '-3.5';
      document.getElementById('render-max-re').value = '1.25';
      document.getElementById('render-min-im').value = '-0.75';
      document.getElementById('render-max-im').value = '2.0';
      _updateRenderExplicitViewportAspect();
      _setRootProgramSourceText('render', '');
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const payload = await page.evaluate(() => window._explicitPayload);
    expect(payload).not.toBeNull();
    expect(payload.params.view_mode).toBe('explicit');
    expect(payload.params.min_re).toBeCloseTo(-3.5, 6);
    expect(payload.params.max_re).toBeCloseTo(1.25, 6);
    expect(payload.params.min_im).toBeCloseTo(-0.75, 6);
    expect(payload.params.max_im).toBeCloseTo(2.0, 6);
  });

  test('square viewport dispatch does not leak stale explicit bounds', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      window._squarePayload = null;
      window.lambdaPost = async function(name, body, path) {
        if (name === 'dispatch' && body.target === 'render_orchestrator') {
          window._squarePayload = body.jobs[0];
          return { fired: 1, errors: [] };
        }
        if (name === 'storage' && path === '/check-status') {
          return { errors: 0, done: 1, complete: true, results: [{ phase: 'done' }] };
        }
        return {};
      };
      window.refreshRenderArtifacts = async function() {};
      document.getElementById('render-results-dir').value = 'test_square';
      document.getElementById('render-pix').value = '512';
      document.getElementById('render-min-re').value = '-9';
      document.getElementById('render-max-re').value = '9';
      document.getElementById('render-min-im').value = '-7';
      document.getElementById('render-max-im').value = '7';
      selectViewMode('square');
      document.getElementById('render-square-extent').value = '2.5';
      _setRootProgramSourceText('render', '');
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const payload = await page.evaluate(() => window._squarePayload);
    expect(payload).not.toBeNull();
    expect(payload.params.view_mode).toBe('square');
    expect(payload.params.square_extent).toBeCloseTo(2.5, 6);
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'min_re')).toBe(false);
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'max_re')).toBe(false);
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'min_im')).toBe(false);
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'max_im')).toBe(false);
  });

  test('invalid explicit viewport blocks dispatch and surfaces error', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      window._explicitDispatchCalls = 0;
      window.lambdaPost = async function(name, body, path) {
        if (name === 'dispatch' && body.target === 'render_orchestrator') {
          window._explicitDispatchCalls += 1;
          return { fired: 1, errors: [] };
        }
        return {};
      };
      document.getElementById('render-results-dir').value = 'test_explicit_invalid';
      document.getElementById('render-pix').value = '512';
      selectViewMode('explicit');
      document.getElementById('render-min-re').value = '5';
      document.getElementById('render-max-re').value = '1';
      document.getElementById('render-min-im').value = '-1';
      document.getElementById('render-max-im').value = '1';
      _updateRenderExplicitViewportAspect();
      _setRootProgramSourceText('render', '');
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const dispatchCalls = await page.evaluate(() => window._explicitDispatchCalls);
    expect(dispatchCalls).toBe(0);
    await expect(page.locator('#render-status')).toContainText('Explicit viewport requires Max Re > Min Re');
  });

  test('populate restores explicit viewport bounds and preserved square controls from artifact metadata', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_bounds';
      _renderLoadedJobId = 'test_bounds';
      _renderActiveFamily = 'color';
      _setSolveScoreProgramStatus('render', 'Loaded pal5');
      _solveScoreProgramRememberedNames.render = 'pal5';
      renderArtifactPanel('test_bounds', {
        calc: { exists: true, N: 4000, degree: 8 },
        families: {
          color: [{
            artifact_id: 'color_bounds',
            created_at: '2026-04-20T12:00:00Z',
            image_key: 'renders/test_bounds/color/color_bounds/image.jpeg',
            image_url: 'https://example.com/bounds.jpeg',
            preview_url: 'https://example.com/bounds.png',
            viewer_url: 'https://example.com/bounds.png',
            file_size: 50000,
            width: 512,
            height: 512,
            color_mode: 'solve_score',
            format: 'jpeg',
            view_mode: 'square',
            square_extent: 3.25,
            min_re: -3.5,
            max_re: 1.25,
            min_im: -0.75,
            max_im: 2.0,
            rotation: 0,
            palette: 'inferno',
            solve_score_chain: [['proximity', '0.1']],
          }],
          bilevel: [],
          coeffs: [],
          palette: [],
          pdf: [],
        },
      });
      populateSelectedRenderArtifact();
    });

    expect(await page.locator('#render-min-re').inputValue()).toBe('-3.5');
    expect(await page.locator('#render-max-re').inputValue()).toBe('1.25');
    expect(await page.locator('#render-min-im').inputValue()).toBe('-0.75');
    expect(await page.locator('#render-max-im').inputValue()).toBe('2');
    expect(await page.locator('#render-square-extent').inputValue()).toBe('3.25');
    await expect(page.locator('#render-solve-score-program-status')).toContainText('Populated from color_bounds');
    const mode = await page.evaluate(() => _viewMode);
    const rememberedName = await page.evaluate(() => _solveScoreProgramRememberedNames.render);
    expect(mode).toBe('explicit');
    expect(rememberedName).toBe('');
  });

  test('preview marquee populates exact viewport and clears on right-click, Escape, and row change', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const viewerUrl = `data:image/svg+xml;utf8,${encodeURIComponent('<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200"><rect width="200" height="200" fill="black"/></svg>')}`;
    await page.evaluate((url) => {
      document.getElementById('render-results-dir').value = 'preview_bounds';
      _renderLoadedJobId = 'preview_bounds';
      _renderActiveFamily = 'color';
      renderArtifactPanel('preview_bounds', {
        calc: { exists: true, N: 4000, degree: 8 },
        families: {
          color: [
            {
              artifact_id: 'color_drag_1',
              created_at: '2026-04-20T12:00:00Z',
              image_key: 'renders/preview_bounds/color/color_drag_1/image.jpeg',
              image_url: url,
              preview_url: url,
              viewer_url: url,
              file_size: 50000,
              width: 200,
              height: 200,
              color_mode: 'solve_score',
              format: 'jpeg',
              min_re: -2,
              max_re: 2,
              min_im: -2,
              max_im: 2,
              rotation: 0,
            },
            {
              artifact_id: 'color_drag_2',
              created_at: '2026-04-20T12:01:00Z',
              image_key: 'renders/preview_bounds/color/color_drag_2/image.jpeg',
              image_url: url,
              preview_url: url,
              viewer_url: url,
              file_size: 50000,
              width: 200,
              height: 200,
              color_mode: 'solve_score',
              format: 'jpeg',
              min_re: -2,
              max_re: 2,
              min_im: -2,
              max_im: 2,
              rotation: 0,
            },
          ],
          bilevel: [],
          coeffs: [],
          palette: [],
          pdf: [],
        },
      });
    }, viewerUrl);

    const stage = page.locator('#render-preview-stage');
    await expect(stage).toBeVisible();
    const drag = async () => {
      await page.evaluate(() => {
        const stageEl = document.getElementById('render-preview-stage');
        const imageEl = document.getElementById('render-preview-image');
        if (!stageEl) throw new Error('render-preview-stage missing');
        if (!imageEl) throw new Error('render-preview-image missing');
        const rect = imageEl.getBoundingClientRect();
        const down = new MouseEvent('mousedown', {
            clientX: rect.left + rect.width * 0.1,
            clientY: rect.top + rect.height * 0.15,
          button: 0,
          bubbles: true,
          cancelable: true,
        });
        const move = new MouseEvent('mousemove', {
          clientX: rect.left + rect.width * 0.6,
          clientY: rect.top + rect.height * 0.7,
          bubbles: true,
          cancelable: true,
        });
        const up = new MouseEvent('mouseup', {
          clientX: rect.left + rect.width * 0.6,
          clientY: rect.top + rect.height * 0.7,
          button: 0,
          bubbles: true,
          cancelable: true,
        });
        stageEl.dispatchEvent(down);
        document.dispatchEvent(move);
        document.dispatchEvent(up);
      });
    };

    await drag();
    const firstBounds = await page.evaluate(() => ({
      minRe: Number(document.getElementById('render-min-re').value),
      maxRe: Number(document.getElementById('render-max-re').value),
      minIm: Number(document.getElementById('render-min-im').value),
      maxIm: Number(document.getElementById('render-max-im').value),
      mode: _viewMode,
      status: document.getElementById('render-status').textContent,
    }));
    expect(firstBounds.minRe).toBeCloseTo(-1.6, 1);
    expect(firstBounds.maxRe).toBeCloseTo(0.4, 1);
    expect(firstBounds.minIm).toBeCloseTo(-0.8, 1);
    expect(firstBounds.maxIm).toBeCloseTo(1.4, 1);
    expect(firstBounds.mode).toBe('explicit');
    expect(firstBounds.status).toContain('Preview subview selected from color_drag_1');
    await expect(page.locator('#render-preview-marquee')).toBeVisible();

    await page.evaluate(() => {
      const stageEl = document.getElementById('render-preview-stage');
      if (!stageEl) throw new Error('render-preview-stage missing');
      stageEl.dispatchEvent(new MouseEvent('contextmenu', {
        button: 2,
        bubbles: true,
        cancelable: true,
      }));
    });
    await expect(page.locator('#render-preview-marquee')).toBeHidden();

    await drag();
    await expect(page.locator('#render-preview-marquee')).toBeVisible();
    await page.keyboard.press('Escape');
    await expect(page.locator('#render-preview-marquee')).toBeHidden();

    await drag();
    await expect(page.locator('#render-preview-marquee')).toBeVisible();
    await page.click('#render-art-row-color-1');
    await expect(page.locator('#render-preview-marquee')).toBeHidden();
  });

  test('populate seeds auto controls even when canonical bounds restore explicit mode', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_auto_bounds';
      _renderLoadedJobId = 'test_auto_bounds';
      _renderActiveFamily = 'color';
      renderArtifactPanel('test_auto_bounds', {
        calc: { exists: true, N: 4000, degree: 8 },
        families: {
          color: [{
            artifact_id: 'color_auto_bounds',
            created_at: '2026-04-20T12:00:00Z',
            image_key: 'renders/test_auto_bounds/color/color_auto_bounds/image.jpeg',
            image_url: 'https://example.com/auto-bounds.jpeg',
            preview_url: 'https://example.com/auto-bounds.png',
            viewer_url: 'https://example.com/auto-bounds.png',
            file_size: 50000,
            width: 512,
            height: 512,
            color_mode: 'solve_score',
            format: 'jpeg',
            view_mode: 'auto',
            quantile: 0.012,
            shim: 0.17,
            min_re: -3.5,
            max_re: 1.25,
            min_im: -0.75,
            max_im: 2.0,
            rotation: 0,
            palette: 'inferno',
            solve_score_chain: [['proximity', '0.1']],
          }],
          bilevel: [],
          coeffs: [],
          palette: [],
          pdf: [],
        },
      });
      populateSelectedRenderArtifact();
    });

    expect(await page.locator('#render-quantile').inputValue()).toBe('1.2');
    expect(await page.locator('#render-shim').inputValue()).toBe('17');
    const mode = await page.evaluate(() => _viewMode);
    expect(mode).toBe('explicit');
  });

  test('populate clears color root transforms when the artifact saved none', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_rt_clear';
      _renderLoadedJobId = 'test_rt_clear';
      _renderActiveFamily = 'color';
      _setRootProgramSourceText('render', 'rotate_roots(0.25)');
      renderArtifactPanel('test_rt_clear', {
        calc: { exists: true, N: 4000, degree: 8 },
        families: {
          color: [{
            artifact_id: 'color_rt_clear',
            created_at: '2026-04-20T12:00:00Z',
            image_key: 'renders/test_rt_clear/color/color_rt_clear/image.jpeg',
            image_url: 'https://example.com/rt-clear.jpeg',
            preview_url: 'https://example.com/rt-clear.png',
            viewer_url: 'https://example.com/rt-clear.png',
            file_size: 50000,
            width: 512,
            height: 512,
            color_mode: 'solve_score',
            format: 'jpeg',
            view_mode: 'auto',
            quantile: 0.01,
            shim: 0.05,
            rotation: 0,
            root_transforms: [],
            palette: 'inferno',
            solve_score_chain: [['proximity', '0.1']],
          }],
          bilevel: [],
          coeffs: [],
          palette: [],
          pdf: [],
        },
      });
      populateSelectedRenderArtifact();
    });

    await expect.poll(async () => page.evaluate(() => document.getElementById('render-rt-source-text').value)).toBe('');
  });

  test('populate restores bilevel camera, root transforms, and section settings', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_bilevel_populate';
      _renderLoadedJobId = 'test_bilevel_populate';
      _renderActiveFamily = 'bilevel';
      _setRootProgramSourceText('render', 'rotate_roots(0.25)');
      renderArtifactPanel('test_bilevel_populate', {
        calc: { exists: true, N: 4000, degree: 8 },
        families: {
          color: [],
          bilevel: [{
            artifact_id: 'bilevel_bounds',
            created_at: '2026-04-20T12:00:00Z',
            image_key: 'renders/test_bilevel_populate/bilevel/bilevel_bounds/image.tif',
            image_url: 'https://example.com/bilevel.tif',
            preview_url: 'https://example.com/bilevel.png',
            viewer_url: 'https://example.com/bilevel.png',
            file_size: 50000,
            width: 1024,
            height: 1024,
            format: 'tif',
            pix: 1024,
            quality: 95,
            view_mode: 'explicit',
            quantile: 0.02,
            shim: 0.06,
            min_re: -3.5,
            max_re: 1.25,
            min_im: -0.75,
            max_im: 2.0,
            rotation: Math.PI / 2,
            root_transforms: [['rotate_roots', '0.5']],
            bilevel_section_mode: 'logical_sections',
            bilevel_section_count: 9,
            render_execution: { raster_section_mode: 'logical_sections', raster_section_count: 9 },
          }],
          coeffs: [],
          palette: [],
          pdf: [],
        },
      });
      populateSelectedRenderArtifact();
    });

    expect(await page.locator('#render-min-re').inputValue()).toBe('-3.5');
    expect(await page.locator('#render-max-re').inputValue()).toBe('1.25');
    expect(await page.locator('#render-min-im').inputValue()).toBe('-0.75');
    expect(await page.locator('#render-max-im').inputValue()).toBe('2');
    expect(await page.locator('#render-pix').inputValue()).toBe('1024');
    const state = await page.evaluate(() => ({
      viewMode: _viewMode,
      rtSource: document.getElementById('render-rt-source-text').value,
      bilevelPopupState: _bilevelPopupState,
    }));
    expect(state.viewMode).toBe('explicit');
    expect(state.rtSource).toBe('rotate_roots(0.5)');
    expect(state.bilevelPopupState.sectionMode).toBe('logical_sections');
    expect(state.bilevelPopupState.sectionCount).toBe(9);
  });

  test('populate restores coeff camera and clears root transforms when saved empty', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_coeff_populate';
      _renderLoadedJobId = 'test_coeff_populate';
      _renderActiveFamily = 'coeffs';
      _setRootProgramSourceText('render', 'rotate_roots(0.25)');
      renderArtifactPanel('test_coeff_populate', {
        calc: { exists: true, N: 4000, degree: 8 },
        families: {
          color: [],
          bilevel: [],
          coeffs: [{
            artifact_id: 'coeff_bounds',
            created_at: '2026-04-20T12:00:00Z',
            image_key: 'renders/test_coeff_populate/coeffs/coeff_bounds/image.tif',
            image_url: 'https://example.com/coeff.tif',
            preview_url: 'https://example.com/coeff.png',
            viewer_url: 'https://example.com/coeff.png',
            file_size: 50000,
            width: 768,
            height: 768,
            format: 'tif',
            pix: 768,
            view_mode: 'explicit',
            quantile: 0.02,
            shim: 0.06,
            min_re: -1.5,
            max_re: 2.25,
            min_im: -0.5,
            max_im: 1.75,
            rotation: 0,
            root_transforms: [],
          }],
          palette: [],
          pdf: [],
        },
      });
      populateSelectedRenderArtifact();
    });

    expect(await page.locator('#render-min-re').inputValue()).toBe('-1.5');
    expect(await page.locator('#render-max-re').inputValue()).toBe('2.25');
    expect(await page.locator('#render-min-im').inputValue()).toBe('-0.5');
    expect(await page.locator('#render-max-im').inputValue()).toBe('1.75');
    const state = await page.evaluate(() => ({
      viewMode: _viewMode,
      rtSource: document.getElementById('render-rt-source-text').value,
    }));
    expect(state.viewMode).toBe('explicit');
    expect(state.rtSource).toBe('');
  });

  test('viewport quantile and solve-score quantile are independent', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    // Set viewport quantile to 2.0, solve-score q to 4.0 via Compile Text
    await page.evaluate(async () => {
      document.getElementById('render-quantile').value = '2.0';
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/compile-solve-score-program-source') {
          return { ok: true, statement_count: 1, chain: [['proximity', '4']], program: { chain: [['proximity', '4']], statement_count: 1 } };
        }
        return {};
      };
      _setSolveScoreProgramSourceText('render', 'score = metric(proximity, slv, q=4%)\n');
      await _compileSolveScoreSourceEditor('render');
    });
    const vq = await page.locator('#render-quantile').inputValue();
    const sq = await page.locator('#render-solve-score-quantile').inputValue();
    expect(vq).toBe('2');
    expect(sq).toBe('4');
  });

  test('real_axis_proximity dispatch carries the solve-score source text', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await chooseSolveMetric(page, 'real_axis_proximity');

    await page.evaluate(() => {
      _setSolveScoreProgramSourceText('render', 'score = metric(real_axis_proximity, slv, q=0.1%)\n');
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
      _setRootProgramSourceText('render', '');
    });

    await page.evaluate(async () => {
      try { await runRasterPipeline(); } catch(e) {}
    });

    const payload = await page.evaluate(() => window._orchPayload);
    expect(payload).not.toBeNull();
    expect(payload.params.color_mode).toBe('solve_score');
    expect(payload.params.solve_score_program_source_text).toContain('metric(real_axis_proximity');
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'solve_score_chain')).toBe(false);
    expect(Object.prototype.hasOwnProperty.call(payload.params, 'solve_metric')).toBe(false);
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
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', created_at: '2026-03-30T12:00:00Z', image_key: 'renders/test_job/palettes/pal_1/image.jpeg', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png', viewer_url: 'https://example.com/p.png', file_size: 40000, width: 1000, height: 1000, metric: 'crowding', palette: 'reef', solve_score_chain: [['crowding', '5']], derived_from_color_artifact_id: 'color_1' }],
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
    await expect(panel.locator('#btn-render-generate')).toHaveCount(0);
    await expect(panel.locator('#btn-render-generate-mt')).toHaveText('ColorRender-MT');
    await expect(panel.locator('#btn-render-populate')).toBeVisible();
    await expect(panel.locator('#btn-render-download')).toBeVisible();
    await expect(panel.locator('#btn-render-delete')).toBeVisible();
    await expect(panel.locator('#btn-render-deepzoom')).toBeVisible();
    await expect(panel.locator('#btn-render-go-palette')).toHaveText('GoPalette: pal_1');
    await expect(panel.locator('img[src="https://example.com/c.png"]')).toBeVisible();
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

  test('palette family generate accepts RGB explicit output score programs in the frontend', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_job';
      renderColorMode = 'solve_score';
      _renderActiveFamily = 'palette';
      _lastCalcHasLores = true;
      renderArtifactPanel('test_job', {
        families: {
          color: [],
          bilevel: [],
          coeffs: [],
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png' }],
        },
        calc: { exists: true, N: 1000, degree: 5, lores: { bin_key: 'renders/test_job/lores.bin' } },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
      _renderScoreChain.splice(0, _renderScoreChain.length,
        { name: 'proximity', params: ['slv', '0.5'] },
        { name: 'emit', params: ['norm'] },
        { name: 'spread', params: ['slv', '0.5'] },
        { name: 'emit', params: ['norm'] },
        { name: 'angular_entropy_16', params: ['cf', '0.5'] },
        { name: 'emit', params: ['norm'] },
      );
      _setRenderColorInterpretation('rgb');
      _syncSolveScoreUi('ss');
      _updateSolveScoreButtons();
    });
    await expect(page.locator('#btn-render-generate')).toBeEnabled();
    await expect(page.locator('#render-solve-score-program-status')).toHaveText('');
  });

  test('palette family generate rejects 3-output programs in scalar LUT mode', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_job';
      renderColorMode = 'solve_score';
      _renderActiveFamily = 'palette';
      _lastCalcHasLores = true;
      renderArtifactPanel('test_job', {
        families: {
          color: [],
          bilevel: [],
          coeffs: [],
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png' }],
        },
        calc: { exists: true, N: 1000, degree: 5, lores: { bin_key: 'renders/test_job/lores.bin' } },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
      _renderScoreChain.splice(0, _renderScoreChain.length,
        { name: 'proximity', params: ['slv', '0.5'] },
        { name: 'emit', params: ['norm'] },
        { name: 'spread', params: ['slv', '0.5'] },
        { name: 'emit', params: ['norm'] },
        { name: 'angular_entropy_16', params: ['cf', '0.5'] },
        { name: 'emit', params: ['norm'] },
      );
      _setRenderColorInterpretation('scalar_lut');
      _syncSolveScoreUi('ss');
      _updateSolveScoreButtons();
    });
    await expect(page.locator('#btn-render-generate')).toBeDisabled();
    await expect(page.locator('#render-solve-score-program-status')).toContainText('program incompatible with Scalar LUT');
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
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', created_at: '2026-03-30T12:00:00Z', image_key: 'renders/test_job/palettes/pal_1/image.jpeg', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png', viewer_url: 'https://example.com/p.png', file_size: 40000, width: 1000, height: 1000, metric: 'crowding', palette: 'reef', solve_score_chain: [['crowding', '5']] }],
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
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', created_at: '2026-03-30T12:00:00Z', image_key: 'renders/test_job/palettes/pal_1/image.jpeg', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png', viewer_url: 'https://example.com/p.png', file_size: 40000, width: 1000, height: 1000, metric: 'crowding', palette: 'reef', solve_score_chain: [['crowding', '5']], derived_from_color_artifact_id: 'color_1' }],
        },
        calc: { exists: true, N: 1000, degree: 5 },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
    });

    await page.click('#btn-render-go-palette');
    await expect.poll(async () => page.evaluate(() => _renderActiveFamily)).toBe('palette');
    await expect(page.locator('#btn-render-go-color')).toHaveText('GoColor: color_1');
    await expect(page.locator('text=solve:crowding(q=5%) · reusable · palette:reef · color:color_1')).toBeVisible();

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
      _setRootProgramSourceText('render', '');
      document.getElementById('render-solve-score').value = 'proximity';
      document.getElementById('render-solve-score-quantile').value = '0.1';
      _renderActiveFamily = 'palette';
      renderArtifactPanel('test_job', {
        families: {
          color: [{ artifact_id: 'color_1', created_at: '2026-03-30T10:00:00Z', image_key: 'renders/test_job/color/color_1/image.jpeg', image_url: 'https://example.com/c.jpeg', preview_url: 'https://example.com/c.png', viewer_url: 'https://example.com/c.png', file_size: 50000, width: 1000, height: 1000, color_mode: 'rainbow', format: 'jpeg' }],
          bilevel: [],
          coeffs: [],
          palette: [{ artifact_id: 'pal_1', palette_id: 'pal_1', created_at: '2026-03-30T12:00:00Z', image_key: 'renders/test_job/palettes/pal_1/image.jpeg', image_url: 'https://example.com/p.jpeg', preview_url: 'https://example.com/p.png', viewer_url: 'https://example.com/p.png', file_size: 40000, width: 1000, height: 1000, metric: 'crowding', palette: 'reef', solve_score_chain: [['crowding', '5']], root_transforms: [['rotate_roots', '0.125']], derived_from_color_artifact_id: 'color_1' }],
        },
        calc: { exists: true, N: 1000, degree: 5 },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
      _setSolveScoreProgramStatus('render', 'Loaded pal5');
      _solveScoreProgramRememberedNames.render = 'pal5';
    });

    await page.click('#btn-render-populate');

    await expect(page.locator('#render-preview img[src="https://example.com/c.png"]')).toBeVisible();
    await expect(page.locator('#render-solve-score-program-status')).toContainText('Populated from color_1');
    const state = await page.evaluate(() => ({
      family: _renderActiveFamily,
      mode: renderColorMode,
      metric: renderSolveMetric,
      palette: renderSolveScorePalette,
      q: document.getElementById('render-solve-score-quantile').value,
      rt: document.getElementById('render-rt-source-text').value,
      rememberedName: _solveScoreProgramRememberedNames.render,
    }));
    expect(state.family).toBe('color');
    expect(state.mode).toBe('solve_score');
    expect(state.metric).toBe('crowding');
    expect(state.palette).toBe('reef');
    expect(state.q).toBe('5');
    expect(state.rt).toContain('rotate_roots(0.125)');
    expect(state.rememberedName).toBe('');
  });

  test('color summary shows solve display, palette name, and source color artifact id', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'summary_job';
      _renderLoadedJobId = 'summary_job';
      _renderActiveFamily = 'color';
      renderArtifactPanel('summary_job', {
        families: {
          color: [{
            artifact_id: 'color_1',
            created_at: '2026-03-30T10:00:00Z',
            image_key: 'renders/summary_job/color/color_1/image.jpeg',
            image_url: 'https://example.com/c.jpeg',
            preview_url: 'https://example.com/c.png',
            viewer_url: 'https://example.com/c.png',
            file_size: 50000,
            width: 1000,
            height: 1000,
            color_mode: 'solve_score',
            format: 'jpeg',
            palette: 'reef',
            solve_score_chain: [['crowding', '5'], ['omega_cosine', '4']],
          }],
          bilevel: [],
          coeffs: [],
          palette: [],
          pdf: [],
        },
        calc: { exists: true, N: 1000, degree: 5 },
        artifacts: {},
        deepzoom_latest: { exists: false },
      });
    });

    await expect(page.locator('text=solve:crowding(q=5%) ω-cos(4) · mode:Scalar LUT · palette:reef · color:color_1')).toBeVisible();
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

  test('ColorRender-MT popup exposes retries and associated palette and dispatches MT payload', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await seedRenderPopupState(page, 'solve_score');

    await page.click('#btn-render-generate-mt');
    const popup = page.locator('#render-mt-popup-overlay');
    await expect(popup).toBeVisible();
    await expect(page.locator('#render-mt-fused-raster-retries')).toBeVisible();
    await expect(page.locator('#render-mt-fused-save-associated-palette')).toBeVisible();

    await page.fill('#render-mt-fused-clip-threads', '6');
    await page.fill('#render-mt-fused-raster-threads', '7');
    await page.fill('#render-mt-fused-raster-workers', '18');
    await page.fill('#render-mt-fused-raster-retries', '4');
    await page.selectOption('#render-mt-fused-raster-section-mode', 'logical_sections');
    await page.fill('#render-mt-fused-raster-section-count', '12');
    await page.fill('#render-mt-fused-finalize-workers', '19');
    await page.check('#render-mt-fused-save-associated-palette');
    await page.click('#render-mt-popup-run');

    const launches = await page.evaluate(() => window._renderLaunches);
    expect(launches).toHaveLength(1);
    expect(launches[0]).toEqual({
      mode: 'color',
      paramsPatch: {
        raster_engine: 'mt',
        raster_mt_threads: 7,
        solve_score_threads: 6,
        raster_workers: 18,
        raster_sectioned_retries: 4,
        raster_section_mode: 'logical_sections',
        raster_section_count: 12,
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
          solve_score_chain: [['spread', '2'], ['omega_cosine', '6']],
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

  test('ExtractPalette popup explains unsupported HSV LUT artifacts instead of silently doing nothing', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate((summary) => {
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
          color_interpretation: 'hsv_lut',
          score_output_channel_count: 3,
          raw_channels: 3,
          solve_score_chain: [['spread', '2'], ['emit', 'norm'], ['crowding', '1'], ['emit', 'norm'], ['proximity', '1'], ['emit', 'norm']],
          palette: 'magma',
        }],
      },
    });

    const btn = page.locator('#btn-render-extract-palette');
    await expect(btn).toBeEnabled();
    await btn.click();
    await expect(page.locator('#extract-palette-popup-overlay')).toBeVisible();
    await expect(page.locator('#extract-palette-popup-summary')).toContainText('HSV LUT');
    await expect(page.locator('#extract-palette-popup-summary')).toContainText('existing associated palette artifact');
    await expect(page.locator('#extract-palette-popup-run')).toBeDisabled();
    await expect(page.locator('#render-status')).toContainText('ExtractPalette unavailable');
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

  test('Sculpture SaveFull: one server-side task, popup options, no viewer tab', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await seedRenderPopupState(page);
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'job_sc';
      window._storageCalls = [];
      window._openCount = 0;
      window.open = function () { window._openCount++; return { closed: false }; };
      window._releaseCheck = null;
      const SC = {
        format: 'u16', grid_n: 192, degree: 5, step_count: 36864, pass_count: 1,
        viewport: { min_re: -2, max_re: 2, min_im: -1.5, max_im: 2.5 },
        palette: 'inferno', source_artifact_id: 'color_1',
      };
      const SAVED = {
        version: 1, id: 'scu_full1', title: 'scu_full1', job_id: 'job_sc',
        grid_n: 192, degree: 5, palette: 'inferno', roots_bytes: 1474560,
        format: 'u16', created_at: '2026-07-28T12:00:00Z', prefix: 'sculptures/scu_full1/',
        share_url: 'https://bkt.s3.r.amazonaws.com/sculptures/scu_full1/viewer.html',
      };
      window.lambdaPost = async function (name, body, path) {
        window._storageCalls.push([path, JSON.parse(JSON.stringify(body))]);
        if (path === '/list-sculptures') return { sculptures: [], count: 0 };
        if (path === '/start-sculpture-artifact') return { task_id: 'sculpture_artifact_777' };
        if (path === '/check-status') {
          // hold the task open until the test releases it (busy-state pin)
          await new Promise((resolve) => { window._releaseCheck = resolve; });
          return { done: 1, errors: 0, error_details: [],
                   results: [{ sculpture: SC, saved_sculpture: SAVED, total_ms: 30000 }] };
        }
        return {};
      };
    });

    await page.click('[data-render-family="sculpture"]');
    // the old flow is GONE: no Generate button, no pane size dropdown, no title input
    await expect(page.locator('#btn-sculpture-savefull')).toBeVisible();
    await expect(page.locator('#btn-sculpture-generate')).toHaveCount(0);
    await expect(page.locator('#render-sculpture-n')).toHaveCount(0);
    await expect(page.locator('#sculpture-title')).toHaveCount(0);
    await expect(page.locator('#tab-render')).toContainText('SaveFull / SaveSplat source:');

    // the popup carries EVERY viewer option + the resolution
    await page.click('#btn-sculpture-savefull');
    await expect(page.locator('#save-full-modal-overlay')).toBeVisible();
    await expect(page.locator('#save-full-source')).toHaveText('source: color_1');
    await page.selectOption('#save-full-resolution', '192');
    await page.selectOption('#save-full-style', 'ghost');
    await page.selectOption('#save-full-tour', 'orbit');
    await page.selectOption('#save-full-tour-speed', '2');
    await page.locator('#save-full-height').fill('40');
    await page.locator('#save-full-point').fill('20');
    await page.check('#save-full-show-clu');
    await page.click('#save-full-go');

    // busy state survives a full panel re-render (CR: duplicate ops) —
    // the op is keyed by action, not by the DOM node that started it
    await expect(page.locator('#btn-sculpture-savefull')).toBeDisabled();
    await page.evaluate(() => renderArtifactPanel('job_sc',
      window._lastRenderSummary || { families: _renderArtifacts, calc: {} }, { preserveScroll: true }));
    await expect(page.locator('#btn-sculpture-savefull')).toBeDisabled();
    await expect(page.locator('#btn-sculpture-savefull')).toContainText('Saving');
    await page.evaluate(() => { window._releaseCheck && window._releaseCheck(); });
    await expect(page.locator('#btn-sculpture-savefull')).toHaveText('✓ Saved');

    const st = await page.evaluate(() => ({
      start: window._storageCalls.find((c) => c[0] === '/start-sculpture-artifact')[1],
      saveCalls: window._storageCalls.filter((c) => c[0] === '/save-sculpture').length,
      opens: window._openCount,
      selectedKey: _renderSelectedArtifactKey.sculpture,
      rail: (typeof _jobsRailJobs !== 'undefined' ? _jobsRailJobs : [])
        .filter((j) => j.kind === 'sculpture').map((j) => j.state),
    }));
    // ONE server-side task: the start payload carries the sanitizable
    // save_full block with the popup's exact view; there is NO client-side
    // /save-sculpture second step (that route is gone with its race)
    expect(st.start).toEqual({
      job_id: 'job_sc', artifact_id: 'color_1', n: 192,
      save_full: {
        view: {
          point: 20, height: 0.4, slices: 0,
          show: { points: true, ribbons: false, threads: false, clu: true, splats: false },
          splatRes: 96, style: 'ghost', glow: 30, order: 'nearest', lenq: 100,
          zaxis: 't2', zlo: 0, zhi: 1, tour: 'orbit', tourSpeed: 2,
        },
      },
    });
    expect(st.saveCalls).toBe(0);
    expect(st.opens).toBe(0);                       // NO viewer tab, ever
    expect(st.rail).toEqual(['complete']);          // complete == durable
    expect(st.selectedKey).toBe('sculpture::scu_full1');
    await expect(page.locator('#render-artifact-catalog')).toContainText('scu_full1');
    await expect(page.locator('#render-artifact-viewer iframe')).toHaveAttribute(
      'src', /sculpture\.html#meta=https%3A%2F%2Fpolypaint\.s3\.us-east-1\.amazonaws\.com%2Fsculptures%2Fscu_full1%2Fmeta\.json&sid=scu_full1/);
    await expect(page.locator('[data-render-family="sculpture"] .subtab-count').first()).toHaveText('(1)');
  });

  test('Views tab: Palette-style catalog, N x N render, selected actions, color purity', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await seedRenderPopupState(page);
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_job';
      // the seed's panel kick started a views fetch on the REAL lambdaPost;
      // drop it so the in-flight dedup can't hand its result to these stubs
      window._viewsInventoryInflight = null;
      window._viewsInventoryJob = '';
      window._storageCalls = [];
      window._viewDispatched = false;
      window._viewDeleted = false;
      window.confirm = () => true;
      const colorRows = [
        {
          artifact_id: 'color_1', palette: 'reef', palette_display_name: 'Reef',
          created_at: '2026-07-26T08:00:00Z', pix: '8192', width: 8192, height: 8192,
          format: 'jpeg', quality: 87, min_re: -3, max_re: 2, min_im: -1, max_im: 4,
          quantile: 0.02, shim: 0.04, square_extent: 3, rotation: 0.25,
          background_color: '101820', color_interpretation: 'scalar_lut',
          solve_score_program_source_text: 'score = metric(proximity, slv, q=0.1%)',
          solve_score_normalize: true, root_program_source_text: 'rotate(0.125)',
          step_scores_grid_n: 2000,
        },
        {
          artifact_id: 'color_2', palette: 'lava', created_at: '2026-07-26T07:00:00Z',
          pix: '4096', width: 4096, height: 4096,
        },
      ];
      const mkRow = (id, projection, vertical, created) => ({
        view_id: id, job_id: 'test_job', source_artifact_id: 'color_1',
        projection, vertical, lattice_n: 2000, pix: 2000, width: 2000, height: 2000,
        palette: 'reef', format: 'jpeg', content_type: 'image/jpeg',
        image_key: `renders/test_job/views/${id}/image.jpeg`,
        preview_key: `renders/test_job/views/${id}/preview.png`,
        image_url: `https://bucket.example/renders/test_job/views/${id}/image.jpeg`,
        prefix: `renders/test_job/views/${id}/`, created_at: created,
      });
      window.lambdaPost = async function (name, body, path) {
        window._storageCalls.push([path, JSON.parse(JSON.stringify(body || {})), name]);
        if (path === '/list-sculptures') return { sculptures: [], count: 0 };
        if (path === '/list-views') {
          const views = [mkRow('view_old_front_t2', 'front', 't2', '2026-07-26T09:00:00Z')];
          if (window._viewDispatched && !window._viewDeleted) {
            views.unshift(mkRow('view_new_isometric_t1', 'isometric', 't1', '2026-07-26T10:00:00Z'));
          }
          return { views, count: views.length };
        }
        if (path === '/render-summary') return { families: { color: colorRows }, calc: { N: 2000 } };
        if (name === 'dispatch') {
          window._viewDispatched = true;
          return { fired: 1 };
        }
        if (path === '/check-status') {
          return { complete: true, done: 1, errors: 0, results: [{
            phase: 'done', family: 'views', artifact_id: 'view_new_isometric_t1',
            image_key: 'renders/test_job/views/view_new_isometric_t1/image.jpeg',
          }] };
        }
        if (path === '/delete-render-artifact') {
          window._viewDeleted = true;
          return { family: body.family, artifact_id: body.artifact_id, deleted: 5 };
        }
        return {};
      };
      window._downloadCalls = [];
      window._downloadStorageObject = async (opts) => {
        window._downloadCalls.push(JSON.parse(JSON.stringify(opts)));
        return { url: 'https://signed.example/download' };
      };
      window._dzCalls = [];
      window.runDeepZoomExport = async (jobId, sourceKey, btn, options) => {
        window._dzCalls.push([jobId, sourceKey, options]);
        return true;
      };
      renderArtifactPanel('test_job', { families: { color: colorRows }, calc: { N: 2000 } });
    });

    // Views uses the same selected-row catalog + preview composition as
    // Palette. There are no action buttons duplicated into every row.
    await page.click('[data-render-family="views"]');
    const catalog = page.locator('#render-artifact-catalog');
    await expect(catalog).toContainText('front · t2 · color:color_1');
    await expect(catalog).toContainText('2000x2000');
    await expect(catalog.locator('button')).toHaveCount(0);
    await expect(page.locator('#render-artifact-viewer img')).toHaveAttribute(
      'src', /renders\/test_job\/views\/view_old_front_t2\/preview\.png/
    );
    await expect(page.locator('.render-artifact-family-tabs [data-render-family="views"] .subtab-count')).toHaveText('(1)');
    await expect(page.locator('#tab-render')).toContainText('ViewRender source:');
    await expect(page.locator('#tab-render')).toContainText('color_1');
    await expect(page.locator('#tab-render')).toContainText('2000×2000');
    const listCall = await page.evaluate(() => window._storageCalls.find((c) => c[0] === '/list-views')[1]);
    expect(listCall).toEqual({ job_id: 'test_job' });

    // ViewRender opens the MODAL; the render dispatches the EXACT payload —
    // source artifact + how to view it, nothing of the live render state
    await page.click('#btn-view-render');
    await expect(page.locator('#view-render-modal-overlay')).toBeVisible();
    await expect(page.locator('#view-render-source')).toHaveText('source: color_1 · N=2000 · output 2000×2000');
    await expect(page.locator('#view-render-lattice')).toHaveCount(0);
    await expect(page.locator('#view-render-pix')).toHaveCount(0);
    await page.selectOption('#view-render-projection', 'isometric');
    await page.selectOption('#view-render-vertical', 't1');
    await page.click('#view-render-go');
    const dispatch = await page.evaluate(() => window._storageCalls.find((c) => c[2] === 'dispatch')[1]);
    expect(dispatch.target).toBe('render_orchestrator');
    expect(dispatch.jobs).toHaveLength(1);
    const viewJob = dispatch.jobs[0];
    expect(viewJob.job_id).toBe('test_job');
    expect(viewJob.mode).toBe('color');
    expect(viewJob.params).toMatchObject({
      pix: 2000,
      fmt: 'jpeg',
      quality: 87,
      view_mode: 'explicit',
      min_re: -3,
      max_re: 2,
      min_im: -1,
      max_im: 4,
      rotation: 0.25,
      palette: 'reef',
      palette_display_name: 'Reef',
      background_color: '101820',
      solve_score_program_source_text: 'score = metric(proximity, slv, q=0.1%)',
      solve_score_normalize: true,
      root_program_source_text: 'rotate(0.125)',
      view_projection: 'isometric',
      view_vertical: 't1',
      source_color_artifact_id: 'color_1',
    });
    expect(viewJob.params).not.toHaveProperty('n');
    // The finished View lands on top, becomes the selected preview, and the
    // shared Arrow-key navigator moves between View rows.
    await expect(catalog).toContainText('isometric · t1 · color:color_1');
    await expect(page.locator('.render-artifact-family-tabs [data-render-family="views"] .subtab-count')).toHaveText('(2)');
    await expect(page.locator('#render-artifact-viewer img')).toHaveAttribute(
      'src', /renders\/test_job\/views\/view_new_isometric_t1\/preview\.png/
    );
    await page.keyboard.press('ArrowDown');
    await expect(page.locator('#render-artifact-viewer img')).toHaveAttribute(
      'src', /renders\/test_job\/views\/view_old_front_t2\/preview\.png/
    );
    await page.keyboard.press('ArrowUp');
    await expect(page.locator('#render-artifact-viewer img')).toHaveAttribute(
      'src', /renders\/test_job\/views\/view_new_isometric_t1\/preview\.png/
    );
    const rail = await page.evaluate(() => (typeof _jobsRailJobs !== 'undefined' ? _jobsRailJobs : [])
      .filter((j) => j.kind === 'render' && j.label.startsWith('view ·')).map((j) => j.state));
    expect(rail).toEqual(['done']);

    // GoColor is a selected-item toolbar action, not an action cloned into
    // every row.
    await page.click('#btn-render-go-color');
    const sel = await page.evaluate(() => ({
      family: _renderActiveFamily,
      key: _renderSelectedArtifactKey.color,
    }));
    expect(sel.family).toBe('color');
    expect(sel.key).toContain('color_1');

    // Back to Views: the shared Download and DeepZoom actions operate on the
    // selected View image key.
    await page.click('[data-render-family="views"]');
    await page.click('#btn-render-download');
    await page.click('#dl-menu-file');
    await expect
      .poll(async () => page.evaluate(() => window._downloadCalls.length))
      .toBe(1);
    expect(await page.evaluate(() => window._downloadCalls[0])).toMatchObject({
      key: 'renders/test_job/views/view_new_isometric_t1/image.jpeg',
      filename: 'test_job_view_new_isometric_t1.jpeg',
    });
    await page.click('#btn-render-deepzoom');
    await expect
      .poll(async () => page.evaluate(() => window._dzCalls.length))
      .toBe(1);
    expect(await page.evaluate(() => window._dzCalls[0])).toEqual([
      'test_job',
      'renders/test_job/views/view_new_isometric_t1/image.jpeg',
      { skipRenderRefresh: true },
    ]);

    // Delete uses the generic selected-artifact route and then refreshes the
    // job-scoped Views inventory.
    await page.click('#btn-render-delete');
    await expect(catalog).not.toContainText('isometric · t1');
    await expect(page.locator('.render-artifact-family-tabs [data-render-family="views"] .subtab-count')).toHaveText('(1)');
    const del = await page.evaluate(() => window._storageCalls.find((c) => c[0] === '/delete-render-artifact')[1]);
    expect(del).toEqual({
      job_id: 'test_job',
      family: 'views',
      artifact_id: 'view_new_isometric_t1',
    });

    // the hard-won clarity of the Color tab: views NEVER appear there
    await page.click('[data-render-family="color"]');
    await expect(page.locator('#tab-render')).not.toContainText('isometric · t1');
    const colorCount = await page.locator('.render-artifact-family-tabs [data-render-family="color"] .subtab-count').first().textContent();
    expect(colorCount).toBe('(2)');
  });

  test('Views refresh reports unreadable metadata instead of a silent short list', async ({ page }) => {
    // CR36 follow-up: partial success is not success — the backend's
    // metadata_error_count reaches the BUTTON and the log.
    await page.click('.tab-btn:text("Render")');
    await seedRenderPopupState(page);
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_job';
      // the seed's panel kick started a views fetch on the REAL lambdaPost;
      // drop it so the in-flight dedup can't hand its result to this stub
      window._viewsInventoryInflight = null;
      window._viewsInventoryJob = '';
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list-sculptures') return { sculptures: [], count: 0 };
        if (path === '/list-views') return {
          views: [{ view_id: 'view_ok', job_id: 'test_job', source_artifact_id: 'color_1',
                    projection: 'front', vertical: 't2', lattice_n: 2000, pix: 2000,
                    image_key: 'renders/test_job/views/view_ok/image.jpeg',
                    prefix: 'renders/test_job/views/view_ok/',
                    created_at: '2026-07-26T09:00:00Z' }],
          count: 1,
          metadata_error_count: 2,
        };
        if (path === '/render-summary') return { families: { color: [] }, calc: { N: 2000 } };
        return {};
      };
      renderArtifactPanel('test_job', { families: { color: [] }, calc: { N: 2000 } });
    });
    await page.click('[data-render-family="views"]');
    await expect(page.locator('#views-list, #render-artifact-catalog')).toContainText('front · t2');
    await page.click('#btn-views-refresh');
    await expect(page.locator('#btn-views-refresh')).toHaveText('✓ Refreshed (2 unreadable)');
    const logText = await page.evaluate(() => document.getElementById('render-log')?.textContent || '');
    expect(logText).toContain('2 meta.json unreadable');
  });

  test('overlapping views-inventory loads share one /list-views request', async ({ page }) => {
    // CR36 follow-up: panel kick + GotoRender + Refresh racing on one job
    // must not fan out duplicate list calls — the in-flight fetch is shared.
    const st = await page.evaluate(async () => {
      document.getElementById('render-results-dir').value = 'test_job';
      window._lvCalls = 0;
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list-views') {
          window._lvCalls += 1;
          await new Promise((r) => setTimeout(r, 25));
          return { views: [], count: 0 };
        }
        return {};
      };
      const results = await Promise.all([
        _viewsEnsureInventory(true),
        _viewsEnsureInventory(false),
        _viewsEnsureInventory(true),
      ]);
      const afterWarm = await _viewsEnsureInventory(false);   // cache hit, no fetch
      return { calls: window._lvCalls, oks: results.map((r) => r.ok), warmOk: afterWarm.ok };
    });
    expect(st.calls).toBe(1);
    expect(st.oks).toEqual([true, true, true]);
    expect(st.warmOk).toBe(true);
  });

  test('A-to-B-to-A job hops share the original in-flight views fetch per job', async ({ page }) => {
    // CR36 follow-up 3: a single global in-flight slot let the A->B->A hop
    // mint a SECOND A request whose slow twin could overwrite the newer
    // inventory. The per-job map makes the returning caller share the one
    // live A fetch — one request per job, no stale overwrite window.
    const st = await page.evaluate(async () => {
      window._viewsInventoryInflight = null;
      window._viewsInventoryJob = '';
      window._lvCalls = {};
      window._lvResolvers = {};
      window.lambdaPost = function (name, body, path) {
        if (path === '/list-views') {
          const job = body.job_id;
          window._lvCalls[job] = (window._lvCalls[job] || 0) + 1;
          return new Promise((resolve) => {
            (window._lvResolvers[job] = window._lvResolvers[job] || []).push(() => resolve({
              views: [{ view_id: `view_${job}`, job_id: job, projection: 'front', vertical: 't2',
                        image_key: `renders/${job}/views/view_${job}/image.jpeg`,
                        prefix: `renders/${job}/views/view_${job}/`,
                        created_at: '2026-07-28T10:00:00Z' }],
              count: 1,
            }));
          });
        }
        return Promise.resolve({});
      };
      const dir = document.getElementById('render-results-dir');
      dir.value = 'job_a';
      const p1 = _viewsEnsureInventory(true);
      dir.value = 'job_b';
      const p2 = _viewsEnsureInventory(true);
      dir.value = 'job_a';
      const p3 = _viewsEnsureInventory(true);   // must SHARE p1's fetch
      window._lvResolvers.job_b.forEach((f) => f());
      window._lvResolvers.job_a.forEach((f) => f());
      const [r1, r2, r3] = await Promise.all([p1, p2, p3]);
      return {
        calls: window._lvCalls,
        r1: { ok: r1.ok, reason: r1.reason },
        r2: { ok: r2.ok, reason: r2.reason },
        r3: { ok: r3.ok, reason: r3.reason },
        inventoryJob: window._viewsInventoryJob,
        rows: (window._viewsInventory || []).map((v) => v.view_id),
      };
    });
    expect(st.calls).toEqual({ job_a: 1, job_b: 1 });     // ONE fetch per job
    expect(st.r1).toEqual({ ok: true, reason: '' });
    expect(st.r2).toEqual({ ok: false, reason: 'job_changed' });   // B landed after the hop back
    expect(st.r3).toEqual({ ok: true, reason: '' });
    expect(st.inventoryJob).toBe('job_a');
    expect(st.rows).toEqual(['view_job_a']);              // A's data, never clobbered by B
  });

  test('DeepZoom from a view dispatches the image-based export with the exact payload', async ({ page }) => {
    // CR36-F7 truthful-behavior pin: DeepZoom is IMAGE-based by contract.
    // A selected view's image key dispatches deepzoom_export with the
    // four-field job — no raw keys exist anywhere in the payload.
    const calls = await page.evaluate(async () => {
      window._calls = [];
      window.lambdaPost = async (name, body, path) => {
        window._calls.push([name, path, JSON.parse(JSON.stringify(body || {}))]);
        if (path === '/check-status') return { complete: true };
        return {};
      };
      await runDeepZoomExport('test_job', 'renders/test_job/views/view_x/image.jpeg', null,
                              { skipRenderRefresh: true });
      return window._calls;
    });
    const disp = calls.find((c) => c[0] === 'dispatch');
    expect(disp[2].target).toBe('deepzoom_export');
    expect(disp[2].jobs).toHaveLength(1);
    const job = disp[2].jobs[0];
    expect(job.source_key).toBe('renders/test_job/views/view_x/image.jpeg');
    expect(Object.keys(job).sort()).toEqual(['export_id', 'job_id', 'source_key', 'task_id']);
    expect(job.task_id.startsWith('deepzoom_export_')).toBe(true);
  });

  test('Sculpture tab: SaveSplat modal — explicit settings, hardwired rest', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await seedRenderPopupState(page);
    await page.evaluate(() => {
      document.getElementById('render-results-dir').value = 'test_job';
      window._storageCalls = [];
      window._fetchCalls = 0;
      const realFetch = window.fetch;
      window.fetch = function () { window._fetchCalls++; return realFetch.apply(this, arguments); };
      window.lambdaPost = async function (name, body, path) {
        window._storageCalls.push([path, JSON.parse(JSON.stringify(body))]);
        if (path === '/list-sculptures') return { sculptures: [], count: 0 };
        if (path === '/start-splat-bake') return { task_id: 'splat_bake_777' };
        if (path === '/check-status') {
          return { done: 1, errors: 0, error_details: [], results: [{
            provenance: { generate_cache_hit: true, generate_ms: 1200 },
            sculpture: {
            version: 1, kind: 'splatbake', id: 'scu_bake1', title: 'param bake',
            job_id: 'test_job', splat_count: 157675, bytes: 3600000,
            source_artifact_id: 'color_1',
            created_at: '2026-07-25T12:00:00Z', prefix: 'sculptures/scu_bake1/',
            share_url: 'https://bkt.s3.r.amazonaws.com/sculptures/scu_bake1/viewer.html',
          } }] };
        }
        return {};
      };
    });
    await page.click('[data-render-family="sculpture"]');
    await expect(page.locator('#btn-sculpture-splatbake')).toHaveText('SaveSplat');

    // the popup IS the settings surface — no viewer, no saved row involved
    await page.click('#btn-sculpture-splatbake');
    await expect(page.locator('#save-splat-modal-overlay')).toBeVisible();
    await expect(page.locator('#save-splat-source')).toHaveText('source: color_1');
    await page.selectOption('#save-splat-resolution', '192');
    await page.selectOption('#save-splat-res', '128');
    await page.selectOption('#save-splat-zaxis', 't1');
    await page.locator('#save-splat-height').fill('40');
    await page.locator('#save-splat-point').fill('30');
    await expect(page.locator('#save-splat-height-val')).toHaveText('0.40');
    await expect(page.locator('#save-splat-point-val')).toHaveText('30');
    await page.fill('#save-splat-title', 'param bake');
    await page.click('#save-splat-go');
    await expect(page.locator('#save-splat-modal-overlay')).toBeHidden();
    await expect(page.locator('#btn-sculpture-splatbake')).toHaveText('\u2713 Baked');

    const st = await page.evaluate(() => ({
      calls: window._storageCalls.filter((c) => c[0] === '/start-splat-bake'),
      fetches: window._fetchCalls,
      bake: window.__lastSplatBake,
      rail: (typeof _jobsRailJobs !== 'undefined' ? _jobsRailJobs : [])
        .filter((j) => j.kind === 'splatbake')
        .map((j) => ({ id: j.id, state: j.state })),
    }));
    // EXACT payload: the five modal choices + the hardwired rest — show is
    // splats by construction, style solid (mode 2), no slices, no tour
    expect(st.fetches).toBe(0);
    expect(st.calls).toHaveLength(1);
    expect(st.calls[0][1]).toEqual({
      job_id: 'test_job',
      source: { kind: 'artifact', artifact_id: 'color_1', n: 192 },
      params: {
        res: 128, zaxis: 't1', slices: 0, mode: 2, intensity: 1,
        yscale: 0.4, scalemul: 3, cam: [1.25, 0.85, 1.25], target: [0, 0, 0],
        tour: 'off', tourSpeed: 1, title: 'param bake',
      },
    });
    expect(st.rail).toEqual([{ id: 'splatbake:splat_bake_777', state: 'complete' }]);
    expect(st.bake.id).toBe('scu_bake1');
    expect(st.bake.provenance).toEqual({ generate_cache_hit: true, generate_ms: 1200 });
    await expect(page.locator('#render-artifact-catalog')).toContainText('scu_bake1');
    await expect(page.locator('#render-artifact-catalog')).toContainText('baked \u00b7 157,675 splats');
    await expect(page.locator('[data-render-family="sculpture"] .subtab-count')).toHaveText('(1)');

    // defaults: reopen and bake untouched -> 384/96/t2, height 0.4 and
    // point 30 PERSIST from the last use (the modal keeps its state)
    await page.evaluate(() => { window._storageCalls = []; });
    await page.click('#btn-sculpture-splatbake');
    await page.fill('#save-splat-title', '');
    await page.selectOption('#save-splat-resolution', '384');
    await page.selectOption('#save-splat-res', '96');
    await page.selectOption('#save-splat-zaxis', 't2');
    await page.click('#save-splat-go');
    await expect(page.locator('#btn-sculpture-splatbake')).toHaveText('\u2713 Baked');
    const p2 = await page.evaluate(() =>
      window._storageCalls.find((c) => c[0] === '/start-splat-bake')[1]);
    expect(p2.source).toEqual({ kind: 'artifact', artifact_id: 'color_1', n: 384 });
    expect(p2.params.res).toBe(96);
    expect(p2.params.zaxis).toBe('t2');
    expect(p2.params.title).toBe('');

    // no color artifact -> SaveSplat refuses with a log, no modal opens
    await page.evaluate(() => {
      _renderArtifacts.color = [];
      _saveSplatClose();
    });
    await page.click('#btn-sculpture-splatbake');
    await expect(page.locator('#save-splat-modal-overlay')).toBeHidden();
    const noSrcLog = await page.evaluate(() => document.getElementById('render-log')?.textContent || '');
    expect(noSrcLog).toContain('SaveSplat: no color artifact selected');
  });

  test('a sculpture list outage backs off instead of looping requests', async ({ page }) => {
    // CR: failure -> pane re-render -> silent kick used to spin an
    // unbounded request loop during a storage outage. Silent kicks now
    // back off; an explicit Refresh always retries.
    const st = await page.evaluate(async () => {
      window._sculptureInventoryLoaded = false;
      window._lsCalls = 0;
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list-sculptures') { window._lsCalls += 1; throw new Error('storage outage'); }
        return {};
      };
      document.getElementById('render-results-dir').value = 'test_job';
      await _sculptureEnsureInventory();        // fails, arms the backoff
      await _sculptureEnsureInventory();        // silent kick: gated
      await _sculptureEnsureInventory();        // silent kick: gated
      const afterSilent = window._lsCalls;
      await _sculptureEnsureInventory(true);    // explicit Refresh retries
      return { afterSilent, total: window._lsCalls };
    });
    expect(st.afterSilent).toBe(1);
    expect(st.total).toBe(2);
  });

  test('Sculpture tab: shared catalog with frame preview, arrows, selection toolbar', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await seedRenderPopupState(page);
    await page.evaluate(() => {
      window._storageCalls = [];
      window._openCalls = [];
      window.open = function (u) { window._openCalls.push(String(u)); return { closed: false }; };
      window.confirm = () => true;
      Object.defineProperty(navigator, 'clipboard', {
        value: { writeText: async (t) => { window._copied = String(t); } },
        configurable: true,
      });
      document.getElementById('render-results-dir').value = 'test_job';
      window._sculptureInventory = [
        { id: 'scu_full', title: 'Mine', job_id: 'test_job', grid_n: 192, degree: 9,
          palette: 'inferno', roots_bytes: 1474560,
          created_at: '2026-07-28T10:00:00Z', prefix: 'sculptures/scu_full/' },
        { id: 'scu_bake', kind: 'splatbake', title: 'Bake', job_id: 'test_job',
          splat_count: 52000, bytes: 1200000, source_artifact_id: 'color_1',
          created_at: '2026-07-27T10:00:00Z', prefix: 'sculptures/scu_bake/' },
        { id: 'scu_other', title: 'Foreign', job_id: 'other_job', grid_n: 30, degree: 5,
          palette: 'reef', created_at: '2026-07-27T11:00:00Z', prefix: 'sculptures/scu_other/' },
      ];
      window._sculptureInventoryLoaded = true;
      window.lambdaPost = async function (name, body, path) {
        window._storageCalls.push([path, JSON.parse(JSON.stringify(body || {}))]);
        if (path === '/list-sculptures') return { sculptures: window._sculptureInventory, count: 3 };
        if (path === '/delete-prefix') return { prefix: body.prefix, deleted: 2 };
        return {};
      };
      renderArtifactPanel('test_job', { families: { color: [
        { artifact_id: 'color_1', palette: 'reef', created_at: '2026-07-28T08:00:00Z' },
      ] } });
    });

    // job-scoped catalog: both of this job's saves, the foreign one hidden,
    // NO per-row buttons — the toolbar drives the selection
    await page.click('[data-render-family="sculpture"]');
    const catalog = page.locator('#render-artifact-catalog');
    await expect(catalog).toContainText('scu_full \u00b7 full \u00b7 192\u00d7192 \u00b7 d9');
    await expect(catalog).toContainText('scu_bake \u00b7 baked \u00b7 52,000 splats');
    await expect(catalog).not.toContainText('scu_other');
    await expect(catalog.locator('button')).toHaveCount(0);
    await expect(page.locator('[data-render-family="sculpture"] .subtab-count').first()).toHaveText('(2)');

    // first row selected; its hosted viewer previews in the frame
    await expect(page.locator('#render-artifact-viewer iframe')).toHaveAttribute(
      'src', /sculpture\.html#meta=https%3A%2F%2Fpolypaint\.s3\.us-east-1\.amazonaws\.com%2Fsculptures%2Fscu_full%2Fmeta\.json&sid=scu_full/);

    // arrows move the selection AND the frame (the shared navigator)
    await page.keyboard.press('ArrowDown');
    await expect(page.locator('#render-artifact-viewer iframe')).toHaveAttribute(
      'src', /sculptures\/scu_bake\/viewer\.html/);
    await page.keyboard.press('ArrowUp');
    await expect(page.locator('#render-artifact-viewer iframe')).toHaveAttribute(
      'src', /sculpture\.html#meta=https%3A%2F%2Fpolypaint\.s3\.us-east-1\.amazonaws\.com%2Fsculptures%2Fscu_full%2Fmeta\.json&sid=scu_full/);

    // toolbar: Open + Copy Link act on the SELECTION
    await page.click('#btn-sculpture-open');
    expect(await page.evaluate(() => window._openCalls)).toEqual(
      [expect.stringContaining('/sculptures/scu_full/viewer.html')]);
    await page.click('#btn-sculpture-copy');
    await expect.poll(async () => page.evaluate(() => window._copied || ''))
      .toContain('/sculptures/scu_full/viewer.html');

    // Delete prunes the selected save and the count follows
    await page.click('#btn-sculpture-delete');
    await expect(catalog).not.toContainText('scu_full');
    await expect(catalog).toContainText('scu_bake');
    const del = await page.evaluate(() => window._storageCalls.find((c) => c[0] === '/delete-prefix')[1]);
    expect(del).toEqual({ prefix: 'sculptures/scu_full/' });
    await expect(page.locator('[data-render-family="sculpture"] .subtab-count').first()).toHaveText('(1)');
  });

  test('Sculpture Snap uses the saved source identity and source calculation N', async ({ page }) => {
    await page.route('**/sculpture.html*', (route) => route.fulfill({
      status: 200,
      contentType: 'text/html',
      body: `<!doctype html><script>
        const sid = new URLSearchParams(location.hash.slice(1)).get('sid') || '';
        parent.postMessage({
          type: 'polypaint-sculpture-ready',
          protocol_version: 1,
          sculpture_id: sid,
        }, location.origin);
        addEventListener('message', (event) => {
          const req = event.data || {};
          if (req.type !== 'polypaint-sculpture-snapshot-request') return;
          setTimeout(() => {
            parent.postMessage({
              type: 'polypaint-sculpture-snapshot-response',
              protocol_version: 1,
              sculpture_id: sid,
              request_id: req.request_id,
              snapshot: {
                version: 1,
                projection: 'perspective',
                matrix_layout: 'column_major',
                model_view_matrix: [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,-2,1],
                projection_matrix: [2,0,0,0, 0,2,0,0, 0,0,-1.0004,-1, 0,0,-0.020004,0],
                vertical: 't1',
                slices: 4,
                effective_tlo: 0.2,
                effective_thi: 0.8,
                point_world_size: 0.004,
                point_scale: 0.5,
                point_min_fraction: 0.001,
                point_max_fraction: 0.1,
                style: 'solid',
                show: { points: true, ribbons: false, threads: false, clu: false, splats: false },
                frame: { aspect: 1, crop: 'center_square' },
              },
            }, location.origin);
          }, 60);
        });
      </script>`,
    }));
    await page.click('.tab-btn:text("Render")');
    await seedRenderPopupState(page);
    await page.evaluate(() => {
      _activeRenderRun = null;
      window._snapDispatches = [];
      window._dispatchRenderOrchestrator = async function (mode, payload) {
        window._snapDispatches.push({ mode, payload: JSON.parse(JSON.stringify(payload)) });
      };
      window._sculptureInventory = [
        {
          id: 'scu_source',
          title: 'Source pose',
          job_id: 'source_job',
          source_artifact_id: 'color_source',
          grid_n: 192,
          degree: 9,
          created_at: '2026-07-28T10:00:00Z',
          prefix: 'sculptures/scu_source/',
        },
        {
          id: 'scu_baked',
          kind: 'splatbake',
          title: 'Baked',
          job_id: 'source_job',
          source_artifact_id: 'color_source',
          created_at: '2026-07-28T09:00:00Z',
          prefix: 'sculptures/scu_baked/',
        },
        {
          id: 'scu_legacy',
          title: 'Older full save',
          job_id: 'source_job',
          grid_n: 192,
          degree: 9,
          created_at: '2026-07-28T08:00:00Z',
          prefix: 'sculptures/scu_legacy/',
        },
      ];
      window._sculptureInventoryLoaded = true;
      window._snapSourceMode = 'ok';
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list-sculptures') {
          return { sculptures: window._sculptureInventory, count: 3 };
        }
        if (path === '/render-summary') {
          if (body.job_id !== 'source_job') throw new Error('wrong source job');
          const source = {
            artifact_id: 'color_source',
            min_re: -3,
            max_re: 2,
            min_im: -1,
            max_im: 4,
            format: 'jpeg',
            quality: 88,
            quantile: 0.02,
            shim: 0.04,
            square_extent: 3,
            rotation: 0.25,
            background_color: '101820',
            color_interpretation: 'scalar_lut',
            palette: 'reef',
            palette_display_name: 'Reef',
            solve_score_program_source_text: 'score = metric(proximity, slv, q=0.1%)',
            root_program_source_text: 'rotate(0.125)',
          };
          const selectedColor = { ...source, artifact_id: 'color_current' };
          const colors = window._snapSourceMode === 'missing'
            ? [selectedColor]
            : (window._snapSourceMode === 'ambiguous'
              ? [source, { ...source }, selectedColor]
              : [source, selectedColor]);
          return {
            calc: { exists: true, N: 1234, degree: 9 },
            families: { color: colors },
          };
        }
        throw new Error(`unexpected ${name} ${path || ''}`);
      };
      document.getElementById('render-results-dir').value = 'source_job';
      renderArtifactPanel('source_job', {
        families: {
          color: [{
            artifact_id: 'color_current',
            min_re: -1,
            max_re: 1,
            min_im: -1,
            max_im: 1,
            solve_score_program_source_text: 'score = metric(crowding, slv, q=1%)',
          }],
        },
        calc: { exists: true, N: 777, degree: 4 },
      });
    });

    await page.click('[data-render-family="sculpture"]');
    await expect(page.locator('#btn-sculpture-snap')).toBeDisabled();
    await expect(page.locator('#btn-sculpture-snap')).toBeEnabled();
    const embeddedViewerUrl = await page.locator(
      '#render-artifact-viewer iframe[data-viewer-src]',
    ).getAttribute('data-viewer-src');
    expect(embeddedViewerUrl).toContain('embed=1');
    await page.click('#btn-sculpture-snap');
    await expect.poll(() => page.evaluate(() => window._snapDispatches.length)).toBe(1);

    const dispatch = await page.evaluate(() => window._snapDispatches[0]);
    expect(dispatch.mode).toBe('color');
    expect(dispatch.payload.job_id).toBe('source_job');
    expect(dispatch.payload.mode).toBe('color');
    expect(dispatch.payload.params).toMatchObject({
      pix: 1234,
      view_projection: 'camera',
      view_vertical: 't1',
      source_color_artifact_id: 'color_source',
      source_sculpture_id: 'scu_source',
    });
    expect(dispatch.payload.params.fmt).toBe('jpeg');
    expect(dispatch.payload.params.quality).toBe(88);
    expect(dispatch.payload.params.view_camera).toMatchObject({
      version: 1,
      projection: 'perspective',
      vertical: 't1',
      slices: 4,
    });

    // A run that starts elsewhere while Sculpture is visible must disable
    // Snap before the backend's fail-closed 409 is needed.
    await page.evaluate(() => {
      _activeRenderRun = { run_id: 'run_busy', job_id: 'other_job', mode: 'color' };
      _updateRenderActionButtons();
    });
    await expect(page.locator('#btn-sculpture-snap')).toBeDisabled();
    await expect(page.locator('#btn-sculpture-snap')).toHaveAttribute(
      'title',
      'Wait for the active render to finish',
    );
    await page.evaluate(() => {
      _activeRenderRun = null;
      _updateRenderActionButtons();
    });
    await expect(page.locator('#btn-sculpture-snap')).toBeEnabled();

    // Missing and duplicate source identities are both terminal lookup
    // failures. Neither may fall back to the visible Color selection.
    for (const mode of ['missing', 'ambiguous']) {
      await page.evaluate((nextMode) => {
        window._snapSourceMode = nextMode;
        window._snapDispatches = [];
      }, mode);
      await page.click('#btn-sculpture-snap');
      await expect.poll(() => page.evaluate(() => (
        document.getElementById('render-log')?.textContent || ''
      ))).toContain(mode === 'missing' ? 'is unavailable' : 'is ambiguous');
      expect(await page.evaluate(() => window._snapDispatches.length)).toBe(0);
      await expect(page.locator('#btn-sculpture-snap')).toBeEnabled();
    }
    await page.evaluate(() => {
      window._snapSourceMode = 'ok';
      window._snapDispatches = [];
    });

    // The response is deliberately delayed by the iframe fixture. Replacing
    // that iframe before it replies must cancel the request and ignore the
    // old WindowProxy's eventual response.
    await page.click('#btn-sculpture-snap');
    await page.keyboard.press('ArrowDown');
    await expect(page.locator('#btn-sculpture-snap')).toBeDisabled();
    await page.waitForTimeout(150);
    expect(await page.evaluate(() => window._snapDispatches.length)).toBe(0);
    await page.keyboard.press('ArrowUp');
    await expect(page.locator('#btn-sculpture-snap')).toBeEnabled();

    await page.click('[data-render-family="color"]');
    await page.click('[data-render-family="sculpture"]');
    await expect(page.locator('#btn-sculpture-snap')).toBeDisabled();
    await expect(page.locator('#btn-sculpture-snap')).toBeEnabled();

    await page.keyboard.press('ArrowDown');
    await expect(page.locator('#btn-sculpture-snap')).toBeDisabled();
    await expect(page.locator('#btn-sculpture-snap')).toHaveAttribute(
      'title',
      'Baked splat snapshots are not supported in v1',
    );

    // Full saves created before source_artifact_id remain snappable. The
    // Color selection retained for this job supplies their missing lineage.
    await page.keyboard.press('ArrowDown');
    await expect(page.locator('#btn-sculpture-snap')).toBeDisabled();
    await expect(page.locator('#btn-sculpture-snap')).toBeEnabled();
    await page.evaluate(() => { window._snapDispatches = []; });
    await page.click('#btn-sculpture-snap');
    await expect.poll(() => page.evaluate(() => window._snapDispatches.length)).toBe(1);
    const legacyDispatch = await page.evaluate(() => window._snapDispatches[0]);
    expect(legacyDispatch.payload.params.source_color_artifact_id).toBe('color_current');
  });
});
