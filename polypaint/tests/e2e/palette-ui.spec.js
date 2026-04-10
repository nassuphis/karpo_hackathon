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

  test('palette create dispatches the current metric, palette, and solve-score controls', async ({ page }) => {
    await page.evaluate((palettes) => {
      window._mockPalettes = palettes.slice();
      window._paletteDispatches = [];
      document.getElementById('palette-results-dir').value = 'job_palette';
      paletteTabPalette = 'viridis';
      buildPaletteCircles('palette-circles-palette-tab', 'palette_tab', () => paletteTabPalette);
      window.lambdaPost = async function (name, body, path) {
        if (name === 'storage' && path === '/list-palettes') return { palettes: window._mockPalettes.slice() };
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
    await page.selectOption('#palette-solve-score', 'spread');
    await page.fill('#palette-solve-score-quantile', '1.5');
    await page.fill('#palette-solve-score-omega', '4');
    await page.check('#palette-solve-score-omega-enabled');
    await page.click('#btn-palette-create');

    const dispatch = await page.evaluate(() => window._paletteDispatches[0]);
    expect(dispatch.target).toBe('palette_orchestrator');
    expect(dispatch.jobs).toHaveLength(1);
    expect(dispatch.jobs[0].params).toMatchObject({
      metric: 'spread',
      palette: 'viridis',
      solve_score_quantile: 0.015,
      solve_score_omega: 4,
      solve_score_omega_enabled: true,
    });
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
