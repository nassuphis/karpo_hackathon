// @ts-check
const { test, expect } = require('@playwright/test');

const PALETTES = [
  {
    palette_id: 'pal_1',
    display_name: 'spread q=1.0% inferno',
    created_at: '2026-04-08T10:00:00Z',
    metric: 'spread',
    palette: 'inferno',
    solve_score_quantile: 0.01,
    clip_lo: 0.1,
    clip_hi: 0.9,
    image_key: 'renders/job_palette/palettes/pal_1/image.jpeg',
    image_url: 'https://example.com/palette-1.jpeg',
    preview_url: 'https://example.com/palette-1-preview.png',
    root_transforms: [['rotate_roots', '0.25']],
  },
  {
    palette_id: 'pal_2',
    display_name: 'crowding q=2.0% viridis',
    created_at: '2026-04-08T11:00:00Z',
    metric: 'crowding',
    palette: 'viridis',
    solve_score_quantile: 0.02,
    clip_lo: 0.2,
    clip_hi: 0.8,
    image_key: 'renders/job_palette/palettes/pal_2/image.jpeg',
    image_url: 'https://example.com/palette-2.jpeg',
    preview_url: 'https://example.com/palette-2-preview.png',
    root_transforms: [],
  },
];

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

test.describe('Palette UI', () => {
  test('palette tab loads inventory, auto-selects the first row, and exposes controls', async ({ page }) => {
    await page.evaluate((palettes) => {
      window._mockPalettes = palettes.slice();
      document.getElementById('palette-results-dir').value = 'job_palette';
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list-palettes') return { palettes: window._mockPalettes.slice() };
        throw new Error(`unexpected storage path ${path}`);
      };
    }, PALETTES);

    await page.click('.tab-btn:text("Palette")');
    await expect(page.locator('.palette-inv-row')).toHaveCount(2);
    await expect(page.locator('#btn-palette-download')).toBeEnabled();
    await expect(page.locator('#btn-palette-delete')).toBeEnabled();
    await expect(page.locator('#palette-info')).toContainText('spread q=1.0% inferno');
    await expect(page.locator('#palette-circles-palette-tab [data-palette-popup="builtin"]')).toBeVisible();
    await expect(page.locator('#palette-circles-palette-tab .pal-circle-tri')).toBeVisible();
    await expect(page.locator('#palette-circles-palette-tab .pal-circle-long')).toBeVisible();
  });

  test('MIC picker loads the artwork catalog, filters, and applies name + custom wire', async ({ page }) => {
    await page.evaluate((palettes) => {
      window._mockPalettes = palettes.slice();
      document.getElementById('palette-results-dir').value = 'job_palette';
      window.lambdaPost = async function (name, body, path) {
        if (name === 'storage' && path === '/list-palettes') return { palettes: window._mockPalettes.slice() };
        throw new Error(`unexpected storage path ${path}`);
      };
    }, PALETTES);

    await page.click('.tab-btn:text("Palette")');
    const mic = page.locator('#palette-circles-palette-tab .pal-circle-mic');
    await expect(mic).toBeVisible();
    await mic.click();
    await expect(page.locator('#mic-popup-overlay')).toBeVisible();
    // real catalog served relatively by the test server (~2.3 MB, 20k rows)
    await expect(page.locator('#mic-popup-status')).toContainText('palettes', { timeout: 20000 });
    await expect(page.locator('.mic-popup-credit')).toContainText('Meditations in Color');

    // pager: 250/page over the whole catalog; browse without any filter
    const pageDisplay = page.locator('#mic-popup-page-display');
    await expect(pageDisplay).toHaveText(/^1 \/ \d\d+$/);
    await expect(page.locator('#mic-popup-prev')).toBeDisabled();
    const firstOfPage1 = await page.locator('#mic-popup-body .tri-popup-row').first().textContent();
    await page.click('#mic-popup-next');
    await expect(pageDisplay).toHaveText(/^2 \/ \d\d+$/);
    await expect(page.locator('#mic-popup-prev')).toBeEnabled();
    const firstOfPage2 = await page.locator('#mic-popup-body .tri-popup-row').first().textContent();
    expect(firstOfPage2).not.toBe(firstOfPage1);
    await page.fill('#mic-popup-goto', '50');
    await page.press('#mic-popup-goto', 'Enter');
    await expect(pageDisplay).toHaveText(/^50 \/ \d\d+$/);
    await expect(page.locator('#mic-popup-overlay')).toBeVisible();   // Enter jumped, did not pick

    // filtering resets to page 1 of the filtered set
    await page.fill('#mic-popup-filter', 'kandinsky points');
    await expect(pageDisplay).toHaveText(/^1 \/ \d+$/);
    const row = page.locator('#mic-popup-body .tri-popup-row').first();
    await expect(row).toContainText('Kandinsky');
    await row.click();
    await expect(page.locator('#mic-popup-overlay')).toBeHidden();

    const applied = await page.evaluate(() => ({
      palette: _currentPaletteForMode('palette_tab'),
      displayName: _paletteDisplayNameForMode('palette_tab'),
    }));
    expect(applied.palette).toMatch(/^custom:[0-9a-f]{6}(-[0-9a-f]{6})+$/);
    expect(applied.displayName).toContain('Kandinsky');
    // exactly ONE swatch is live: MIC owns its selection, HEX yields
    await expect(page.locator('#palette-circles-palette-tab .pal-circle-mic.active')).toBeVisible();
    await expect(page.locator('#palette-circles-palette-tab .pal-circle-custom.active')).toHaveCount(0);
    await expect(page.locator('#palette-circles-palette-tab .pal-circle.active')).toHaveCount(1);

    // reopen: the banner names the selection, its strip shows, and the popup
    // lands on the page holding the selected row (highlighted + active)
    await mic.click();
    await expect(page.locator('#mic-popup-overlay')).toBeVisible();
    await expect(page.locator('#mic-popup-selected-name')).toContainText('Kandinsky');
    await expect(page.locator('#mic-popup-selected-strip')).toBeVisible();
    const landed = page.locator('#mic-popup-body .tri-popup-row.highlight');
    await expect(landed).toHaveClass(/active/);
    await expect(landed).toContainText('Kandinsky');

    // arrows work while the filter input keeps focus, and move the highlight
    await page.locator('#mic-popup-filter').focus();
    const before = await page.locator('#mic-popup-body .tri-popup-row.highlight').textContent();
    await page.keyboard.press('ArrowDown');
    const after = await page.locator('#mic-popup-body .tri-popup-row.highlight').textContent();
    expect(after).not.toBe(before);
    await page.keyboard.press('ArrowUp');
    await expect(page.locator('#mic-popup-body .tri-popup-row.highlight')).toHaveText(String(before));
    await page.click('#mic-popup-close');
  });

  test('palette create dispatches the current visible preset and palette selection', async ({ page }) => {
    await page.evaluate((palettes) => {
      window._mockPalettes = palettes.slice();
      window._paletteDispatches = [];
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
      document.getElementById('palette-results-dir').value = 'job_palette';
      window.lambdaPost = async function (name, body, path) {
        if (name === 'storage' && path === '/list-palettes') return { palettes: window._mockPalettes.slice() };
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
        if (name === 'dispatch') {
          window._paletteDispatches.push(body);
          return { fired: 1, non_202: [] };
        }
        throw new Error(`unexpected ${name} ${path || ''}`);
      };
      window._saveActivePaletteRun = function () {};
      window.startActivePaletteObserver = function () {};
      window._loadActivePaletteRun = function () { return null; };
    }, PALETTES);

    await page.click('.tab-btn:text("Palette")');
    await page.click('#palette-solve-score-program-manage');
    await expect(page.locator('#solve-score-modal-overlay')).toBeVisible();
    await page.locator('#solve-score-modal-body .tri-popup-row').filter({ hasText: 'Proximity q=0.1%' }).click();
    await page.click('#solve-score-modal-load');
    await page.click('#solve-score-modal-cancel');
    await expect(page.locator('#solve-score-modal-overlay')).not.toBeVisible();
    await page.locator('#palette-circles-palette-tab [data-palette-popup="builtin"]').click();
    await expect(page.locator('#builtin-popup-overlay')).toBeVisible();
    await page.locator('#builtin-popup-filter').fill('viri');
    await page.locator('#builtin-popup-body .tri-popup-row').filter({ hasText: 'viridis' }).first().click();
    await expect(page.locator('#builtin-popup-overlay')).not.toBeVisible();
    await page.click('#btn-palette-create');

    const dispatch = await page.evaluate(() => window._paletteDispatches[0]);
    expect(dispatch.target).toBe('palette_orchestrator');
    expect(dispatch.jobs).toHaveLength(1);
    expect(dispatch.jobs[0].params).toMatchObject({
      metric: 'proximity',
      palette: 'viridis',
      solve_score_quantile: 0.001,
      solve_score_omega: 1,
      solve_score_omega_enabled: false,
    });
    expect(dispatch.jobs[0].params.solve_score_program_source_text).toContain('metric(proximity');
    expect(Object.prototype.hasOwnProperty.call(dispatch.jobs[0].params, 'solve_score_chain')).toBe(false);
  });

  test('download and delete stay wired to the selected palette artifact', async ({ page }) => {
    await page.evaluate((palettes) => {
      window.confirm = () => true;
      window._mockPalettes = palettes.slice();
      window._paletteDownloads = [];
      document.getElementById('palette-results-dir').value = 'job_palette';
      window.lambdaPost = async function (name, body, path) {
        if (name === 'storage' && path === '/list-palettes') return { palettes: window._mockPalettes.slice() };
        if (name === 'storage' && path === '/delete-palette') {
          window._mockPalettes = window._mockPalettes.filter(p => p.palette_id !== body.palette_id);
          return { ok: true };
        }
        throw new Error(`unexpected ${name} ${path || ''}`);
      };
      window.downloadPresignedFile = async function (url, filename, explicitKey) {
        window._paletteDownloads.push({ url, filename, explicitKey });
      };
    }, PALETTES);

    await page.click('.tab-btn:text("Palette")');
    await page.click('#btn-palette-download');
    const downloads = await page.evaluate(() => window._paletteDownloads.slice());
    expect(downloads).toHaveLength(1);
    expect(downloads[0].explicitKey).toBe('renders/job_palette/palettes/pal_1/image.jpeg');
    expect(downloads[0].filename).toContain('job_palette_pal_1');

    await expect(page.locator('.palette-inv-row')).toHaveCount(2);
    await page.click('#btn-palette-delete');
    await expect(page.locator('.palette-inv-row')).toHaveCount(1);
    await expect(page.locator('#palette-log')).toContainText('Palette deleted: pal_1');
  });

  test('arrow keys move selection within the palette inventory', async ({ page }) => {
    await page.evaluate((palettes) => {
      window._mockPalettes = palettes.slice();
      document.getElementById('palette-results-dir').value = 'job_palette';
      window.lambdaPost = async function (name, body, path) {
        if (name === 'storage' && path === '/list-palettes') return { palettes: window._mockPalettes.slice() };
        throw new Error(`unexpected ${name} ${path || ''}`);
      };
    }, PALETTES);

    await page.click('.tab-btn:text("Palette")');
    await expect(page.locator('.palette-inv-row')).toHaveCount(2);
    await page.keyboard.press('ArrowDown');
    const bg = await page.locator('.palette-inv-row').nth(1).evaluate(el => el.style.background);
    expect(bg).toContain('rgb(42, 42, 78)');
  });
});
