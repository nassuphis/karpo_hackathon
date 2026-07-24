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
      window._customSaves = [];
      window._customSaveBodies = [];
      document.getElementById('palette-results-dir').value = 'job_palette';
      window.lambdaPost = async function (name, body, path) {
        if (name === 'storage' && path === '/list-palettes') return { palettes: window._mockPalettes.slice() };
        if (name === 'storage' && path === '/list-custom-palettes') {
          return { revision: 'r1', palettes: window._customSaves.slice() };
        }
        if (name === 'storage' && path === '/save-custom-palettes') {
          window._customSaveBodies.push(JSON.parse(JSON.stringify(body)));
          window._customSaves = body.palettes.map(p => ({ name: p.name, stops: p.stops, palette: 'custom:' + p.stops.join('-') }));
          return { revision: 'r' + (window._customSaveBodies.length + 1), palettes: window._customSaves.slice() };
        }
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

    // drag-selecting the filter text and releasing over the backdrop must
    // NOT close the popup (the click lands on the overlay — press-origin guard)
    await page.fill('#mic-popup-filter', 'drag me');
    const filterBox = await page.locator('#mic-popup-filter').boundingBox();
    await page.mouse.move(filterBox.x + filterBox.width - 8, filterBox.y + filterBox.height / 2);
    await page.mouse.down();
    await page.mouse.move(8, 8, { steps: 4 });   // release over the dimmed backdrop
    await page.mouse.up();
    await expect(page.locator('#mic-popup-overlay')).toBeVisible();

    // Clear button empties the filter and returns to page 1 of the full list
    await page.click('#mic-popup-clear');
    await expect(page.locator('#mic-popup-filter')).toHaveValue('');
    await expect(pageDisplay).toHaveText(/^1 \/ \d\d+$/);
    await expect(page.locator('#mic-popup-overlay')).toBeVisible();

    // filtering resets to page 1 of the filtered set
    await page.fill('#mic-popup-filter', 'kandinsky points');
    await expect(pageDisplay).toHaveText(/^1 \/ \d+$/);
    const row = page.locator('#mic-popup-body .tri-popup-row').first();
    await expect(row).toContainText('Kandinsky');
    await row.click();
    // selection KEEPS the popup open (pick -> Copy to HEX -> pick again);
    // the applied marker and the banner update in place
    await expect(page.locator('#mic-popup-overlay')).toBeVisible();
    await expect(page.locator('#mic-popup-selected-name')).toContainText('Kandinsky');
    await expect(page.locator('#mic-popup-body .tri-popup-row.active')).toHaveCount(1);
    await page.click('#mic-popup-close');
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
    // ...and the HEX swatch does NOT adopt the MIC pick's face or name —
    // it only ever shows the line selected in its own popup
    await expect(page.locator('#palette-circles-palette-tab .pal-circle-custom'))
      .toHaveAttribute('title', 'Named custom palettes');
    expect(await page.evaluate(() => _customPaletteSelectionByMode['palette_tab'] || null)).toBeNull();

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

    // Copy to HEX: saves the selection into the named custom-palette catalog
    // with busy + lingering result on the button itself; second copy is a no-op
    const copyBtn = page.locator('#mic-popup-copy2hex');
    await expect(copyBtn).toBeEnabled();
    await copyBtn.click();
    await expect(copyBtn).toHaveText('✓ Saved to HEX');
    await expect(page.locator('#mic-popup-status')).toContainText('Saved to HEX as');
    const saved = await page.evaluate(() => window._customSaveBodies);
    expect(saved).toHaveLength(1);
    expect(saved[0].expected_revision).toBe('r1');
    expect(saved[0].palettes).toHaveLength(1);
    expect(saved[0].palettes[0].name).toContain('Kandinsky');
    expect(saved[0].palettes[0].stops.length).toBeGreaterThanOrEqual(3);
    await expect(copyBtn).toHaveText('Copy to HEX', { timeout: 5000 });
    await copyBtn.click();
    await expect(copyBtn).toHaveText('✓ Already in HEX');
    expect(await page.evaluate(() => window._customSaveBodies.length)).toBe(1);
    await expect(copyBtn).toHaveText('Copy to HEX', { timeout: 5000 });
    await page.click('#mic-popup-close');
  });

  test('HEX popup: sequential row clicks keep exactly one applied row', async ({ page }) => {
    await page.evaluate((palettes) => {
      window._mockPalettes = palettes.slice();
      document.getElementById('palette-results-dir').value = 'job_palette';
      window.lambdaPost = async function (name, body, path) {
        if (name === 'storage' && path === '/list-palettes') return { palettes: window._mockPalettes.slice() };
        if (name === 'storage' && path === '/list-custom-palettes') {
          return { revision: 'r1', palettes: [
            { name: 'Reef', stops: ['112233', '445566'], palette: 'custom:112233-445566' },
            { name: 'Ember', stops: ['aa1100', 'ffcc00'], palette: 'custom:aa1100-ffcc00' },
            { name: 'Moss', stops: ['0a3311', '88aa66'], palette: 'custom:0a3311-88aa66' },
          ] };
        }
        throw new Error(`unexpected storage path ${path}`);
      };
    }, PALETTES);
    await page.click('.tab-btn:text("Palette")');
    await page.click('#palette-circles-palette-tab .pal-circle-custom');
    await expect(page.locator('#custom-palette-popup-overlay')).toBeVisible();
    const rows = page.locator('#custom-palette-popup-body tr');
    await expect(rows).toHaveCount(3);
    // click row 1 then row 2 then row 3: after each click the APPLIED marker
    // must sit on the clicked row alone — no wandering pair (user bug)
    for (const idx of [0, 1, 2]) {
      await rows.nth(idx).click();
      await expect(page.locator('#custom-palette-popup-body tr.active')).toHaveCount(1);
      await expect(rows.nth(idx)).toHaveClass(/active/);
      await expect(rows.nth(idx)).toHaveClass(/highlight/);
    }
    const applied = await page.evaluate(() => ({
      palette: _currentPaletteForMode('palette_tab'),
      displayName: _paletteDisplayNameForMode('palette_tab'),
    }));
    expect(applied.palette).toBe('custom:0a3311-88aa66');
    expect(applied.displayName).toBe('Moss');
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
    // selection keeps the popup open; close it before using the page below
    await expect(page.locator('#builtin-popup-overlay')).toBeVisible();
    await page.click('#builtin-popup-close');
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

test.describe('PIC photo palette', () => {
  test('ultra sampling rescues a tiny compositional accent that med culls', async ({ page }) => {
    await page.goto('http://localhost:8765/index.html');
    await page.waitForLoadState('domcontentloaded');
    await page.evaluate(() => {
      window.lambdaPost = async function () { return {}; };
    });
    await page.click('.tab-btn:text("Palette")');
    await page.click('#palette-circles-palette-tab .pal-circle-pic');
    // brown/blue field with a 1x58px red sliver = 0.151% of the image:
    // below med's 0.2% cluster cull, above ultra's 0.05% floor
    const dataUrl = await page.evaluate(() => {
      const canvas = document.createElement('canvas');
      canvas.width = 240; canvas.height = 160;
      const ctx = canvas.getContext('2d');
      ctx.fillStyle = '#7a5c3a'; ctx.fillRect(0, 0, 144, 160);
      ctx.fillStyle = '#22447a'; ctx.fillRect(144, 0, 96, 160);
      ctx.fillStyle = '#e01030'; ctx.fillRect(120, 50, 1, 58);
      return canvas.toDataURL('image/png');
    });
    await page.setInputFiles('#pic-popup-file', {
      name: 'accent.png', mimeType: 'image/png',
      buffer: Buffer.from(dataUrl.split(',')[1], 'base64'),
    });
    await expect(page.locator('#pic-popup-status')).toContainText('colors', { timeout: 15000 });
    const redDistance = () => page.evaluate(() => {
      const dist = h => {
        const p = [1, 3, 5].map(i => parseInt(h.slice(i, i + 2), 16));
        return Math.hypot(p[0] - 224, p[1] - 16, p[2] - 48);   // #e01030
      };
      return Math.min(...(_picPopupState.palette.map(p => dist(p.hex))));
    });
    // med (default): the sliver's cluster dies at the 0.2% cull
    expect(await redDistance()).toBeGreaterThan(90);
    // ultra: same photo, the accent survives and lands in the palette
    await page.check('input[name="pic-popup-sampling"][value="ultra"]');
    await expect(page.locator('#pic-popup-status')).toContainText('colors', { timeout: 30000 });
    expect(await redDistance()).toBeLessThan(30);
    await page.click('#pic-popup-close');
  });

  test('photo -> extraction -> Use now + Save to HEX, styles reorder', async ({ page }) => {
    await page.goto('http://localhost:8765/index.html');
    await page.waitForLoadState('domcontentloaded');
    await page.evaluate((palettes) => {
      window._mockPalettes = palettes.slice();
      window._customSaveBodies = [];
      document.getElementById('palette-results-dir').value = 'job_palette';
      window.lambdaPost = async function (name, body, path) {
        if (name === 'storage' && path === '/list-palettes') return { palettes: window._mockPalettes.slice() };
        if (name === 'storage' && path === '/list-custom-palettes') return { revision: 'r1', palettes: [] };
        if (name === 'storage' && path === '/save-custom-palettes') {
          window._customSaveBodies.push(JSON.parse(JSON.stringify(body)));
          return { revision: 'r2', palettes: body.palettes.map(p => ({ name: p.name, stops: p.stops, palette: 'custom:' + p.stops.join('-') })) };
        }
        throw new Error(`unexpected storage path ${path}`);
      };
    }, PALETTES);
    await page.click('.tab-btn:text("Palette")');
    const pic = page.locator('#palette-circles-palette-tab .pal-circle-pic');
    await expect(pic).toBeVisible();
    await pic.click();
    await expect(page.locator('#pic-popup-overlay')).toBeVisible();
    await expect(page.locator('#pic-popup-status')).toContainText('processed locally');

    // fixture: six uniform color blocks with known areas, generated in-page
    const dataUrl = await page.evaluate(() => {
      const canvas = document.createElement('canvas');
      canvas.width = 240; canvas.height = 160;
      const ctx = canvas.getContext('2d');
      const blocks = [['#d62839', 0.30], ['#2255aa', 0.25], ['#1f9e8e', 0.15],
                      ['#e8a020', 0.10], ['#8a8a8a', 0.12], ['#7a5c3a', 0.08]];
      let x = 0;
      for (const [color, frac] of blocks) {
        ctx.fillStyle = color;
        const w = Math.round(240 * frac);
        ctx.fillRect(x, 0, w, 160);
        x += w;
      }
      ctx.fillStyle = '#7a5c3a';
      ctx.fillRect(x, 0, 240 - x, 160);
      return canvas.toDataURL('image/png');
    });
    await page.setInputFiles('#pic-popup-file', {
      name: 'Sunset_Beach-001.png', mimeType: 'image/png',
      buffer: Buffer.from(dataUrl.split(',')[1], 'base64'),
    });
    await expect(page.locator('#pic-popup-name')).toHaveValue('Sunset Beach 001');
    await expect(page.locator('#pic-popup-status')).toContainText('colors', { timeout: 15000 });
    expect(await page.locator('#pic-popup-strip .pic-popup-cell').count()).toBeGreaterThanOrEqual(5);

    // every block color recovered (nearest-hex distance), editorial default
    const order = await page.evaluate(() => {
      const hexes = _picPopupState.palette.map(p => p.hex);
      const dist = (a, b) => {
        const pa = [1, 3, 5].map(i => parseInt(a.slice(i, i + 2), 16));
        const pb = [1, 3, 5].map(i => parseInt(b.slice(i, i + 2), 16));
        return Math.hypot(pa[0] - pb[0], pa[1] - pb[1], pa[2] - pb[2]);
      };
      const nearest = target => hexes.reduce((best, h, i) =>
        dist(h, target) < dist(hexes[best], target) ? i : best, 0);
      return { hexes, grey: nearest('#8a8a8a'), orange: nearest('#e8a020') };
    });
    expect(order.hexes.length).toBeGreaterThanOrEqual(5);
    // editorial: the substrate penalty sinks grey below vivid orange
    expect(order.orange).toBeLessThan(order.grey);

    // literal: pure pixel share — grey (12%) outranks orange (10%)
    await page.check('input[name="pic-popup-style"][value="literal"]');
    await expect(page.locator('#pic-popup-status')).toContainText('colors', { timeout: 15000 });
    const literalOrder = await page.evaluate(() => {
      const hexes = _picPopupState.palette.map(p => p.hex);
      const dist = (a, b) => {
        const pa = [1, 3, 5].map(i => parseInt(a.slice(i, i + 2), 16));
        const pb = [1, 3, 5].map(i => parseInt(b.slice(i, i + 2), 16));
        return Math.hypot(pa[0] - pb[0], pa[1] - pb[1], pa[2] - pb[2]);
      };
      const nearest = target => hexes.reduce((best, h, i) =>
        dist(h, target) < dist(hexes[best], target) ? i : best, 0);
      return { grey: nearest('#8a8a8a'), orange: nearest('#e8a020') };
    });
    expect(literalOrder.grey).toBeLessThan(literalOrder.orange);

    // Use now applies the wire + the sanitized filename as display name
    await page.click('#pic-popup-use');
    const applied = await page.evaluate(() => ({
      palette: _currentPaletteForMode('palette_tab'),
      displayName: _paletteDisplayNameForMode('palette_tab'),
    }));
    expect(applied.palette).toMatch(/^custom:[0-9a-f]{6}(-[0-9a-f]{6})+$/);
    expect(applied.displayName).toBe('Sunset Beach 001');

    // Save to HEX persists via the shared CAS flow with button feedback
    await page.click('#pic-popup-save');
    await expect(page.locator('#pic-popup-save')).toHaveText('✓ Saved to HEX');
    const saves = await page.evaluate(() => window._customSaveBodies);
    expect(saves).toHaveLength(1);
    expect(saves[0].palettes[0].name).toBe('Sunset Beach 001');
    expect(saves[0].palettes[0].stops.length).toBeGreaterThanOrEqual(5);
    await page.click('#pic-popup-close');
    await expect(page.locator('#pic-popup-overlay')).toBeHidden();
  });
});
