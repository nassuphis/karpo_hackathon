// @ts-check
const { test, expect } = require('@playwright/test');

// favorites-speedup.md: /list-favorites now returns PANEL-READY rows (compact
// display snapshot + favorite_* identity). The favorites tab renders them
// directly and derives preview/image URLs from the keys — it must NOT call
// /render-summary. These specs assert that: the favorites-only flows make
// /render-summary throw.
const FAVORITE_ROWS = [
  {
    family: 'color', artifact_id: 'color_a',
    favorite_job_id: 'job_fav_a', favorite_artifact_id: 'color_a',
    favorite_added_at: '2026-04-08T12:00:00Z', display_name: 'Favorite A',
    image_key: 'renders/job_fav_a/color/color_a/image.jpeg',
    preview_key: 'renders/job_fav_a/color/color_a/preview.png',
    width: 1200, height: 1200, file_size: 180000,
    color_mode: 'solve_score', palette: 'inferno', format: 'jpeg',
    hydration_state: 'ready', missing: false,
  },
  {
    family: 'color', artifact_id: 'color_b',
    favorite_job_id: 'job_fav_b', favorite_artifact_id: 'color_b',
    favorite_added_at: '2026-04-08T13:00:00Z', display_name: 'Favorite B',
    image_key: 'renders/job_fav_b/color/color_b/image.jpeg',
    preview_key: 'renders/job_fav_b/color/color_b/preview.png',
    width: 1600, height: 1600, file_size: 240000,
    color_mode: 'rainbow', format: 'jpeg',
    hydration_state: 'ready', missing: false,
  },
];

// derived, stable public URL for color_a's preview (no presign, no expiry)
const COLOR_A_PREVIEW = 'https://polypaint.s3.us-east-1.amazonaws.com/renders/job_fav_a/color/color_a/preview.png';

// Render-tab inventory (GoRender navigates to the Render tab, which legitimately
// uses /render-summary — that is not a favorites concern).
const RENDER_SUMMARY_A = {
  calc: { exists: true, N: 2000, degree: 8 },
  families: {
    color: [{
      artifact_id: 'color_a', display_name: 'Favorite A',
      image_key: 'renders/job_fav_a/color/color_a/image.jpeg',
      image_url: 'https://example.com/favorite-a.jpeg',
      preview_key: 'renders/job_fav_a/color/color_a/preview.png',
      preview_url: 'https://example.com/favorite-a-preview.png',
      viewer_url: 'https://example.com/favorite-a-preview.png',
      width: 1200, height: 1200, file_size: 180000, color_mode: 'solve_score', format: 'jpeg',
    }],
    bilevel: [], coeffs: [], palette: [],
  },
  artifacts: {}, deepzoom_latest: { exists: false },
};

const RESULT_DETAIL = {
  has_preview: true,
  preview_url: 'https://example.com/result-preview.png',
  file_count: 9,
  calc: { solver: 'aberth', function: 'poly_1' },
  pipeline: { function: 'poly_1', cfpv: [], coeff_transforms: [] },
  preview_stats: { n_roots: 80, n_roots_total: 100, q_re: [-1, 1], q_im: [-1, 1] },
};

test.beforeEach(async ({ page }) => {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
  await page.evaluate(() => {
    window.OpenSeadragon = function () {
      return {
        addHandler() {}, destroy() {},
        world: { getItemAt() { return null; }, getItemCount() { return 0; } },
        viewport: { getZoom() { return 1; }, getCenter() { return { x: 0, y: 0 }; } },
      };
    };
  });
});

test.describe('Favorites UI', () => {
  test('favorites tab renders panel-ready rows without calling render-summary', async ({ page }) => {
    await page.evaluate(({ rows }) => {
      window._mockFavoriteRows = rows.slice();
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list-favorites') return { favorites: window._mockFavoriteRows.slice(), count: window._mockFavoriteRows.length };
        if (path === '/render-summary') throw new Error('favorites must not call /render-summary');
        throw new Error(`unexpected storage path ${path}`);
      };
    }, { rows: FAVORITE_ROWS });

    await page.click('.tab-btn:text("Favorites")');
    await expect(page.locator('.favorite-art-row')).toHaveCount(2);
    await expect(page.locator('#favorites-preview img')).toHaveAttribute('src', COLOR_A_PREVIEW);
    await expect(page.locator('#favorites-info')).toContainText('2 favorites loaded.');
    await expect(page.locator('#btn-favorites-go-render')).toBeEnabled();
    await expect(page.locator('#btn-favorites-download')).toBeEnabled();
    await expect(page.locator('#btn-favorites-delete')).toBeEnabled();
  });

  test('reopening the favorites tab issues no second /list-favorites', async ({ page }) => {
    await page.evaluate(({ rows }) => {
      window._listCalls = 0;
      window._mockFavoriteRows = rows.slice();
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list-favorites') { window._listCalls += 1; return { favorites: window._mockFavoriteRows.slice() }; }
        if (path === '/render-summary') throw new Error('favorites must not call /render-summary');
        throw new Error(`unexpected storage path ${path}`);
      };
    }, { rows: FAVORITE_ROWS });

    await page.click('.tab-btn:text("Favorites")');
    await expect(page.locator('.favorite-art-row')).toHaveCount(2);
    await page.click('.tab-btn:text("Render")');
    await page.click('.tab-btn:text("Favorites")');
    await expect(page.locator('.favorite-art-row')).toHaveCount(2);
    // one list on first entry; the repeat visit renders from cache
    expect(await page.evaluate(() => window._listCalls)).toBe(1);
  });

  test('a superseded load cannot poison the refs cache or clear the newer promise slot', async ({ page }) => {
    // code-review-30 F5: slow non-forced request A vs fast forced request B.
    await page.evaluate((rows) => {
      window._resolvers = [];
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list-favorites') {
          return new Promise((resolve) => { window._resolvers.push({ body, resolve }); });
        }
        throw new Error('unexpected ' + path);
      };
      window._stale = [{ ...rows[0], display_name: 'STALE OLD ROW' }];
      window._fresh = rows;
    }, FAVORITE_ROWS);
    await page.click('.tab-btn:text("Favorites")');            // A starts (non-forced)
    await page.evaluate(() => { void loadFavoritesInventory({ force: true }); });   // B starts (forced)
    await page.waitForFunction(() => window._resolvers.length === 2);
    await page.evaluate(() => {
      window._resolvers[1].resolve({ favorites: window._fresh });   // B (forced) wins first
    });
    await expect(page.locator('.favorite-art-row')).toHaveCount(2);
    await page.evaluate(() => {
      window._resolvers[0].resolve({ favorites: window._stale });   // A resolves late
    });
    await page.waitForTimeout(150);
    const st = await page.evaluate(() => ({
      names: (_favoriteRefs || []).map((r) => r.display_name),
      rows: document.querySelectorAll('.favorite-art-row').length,
    }));
    expect(st.names).toEqual(['Favorite A', 'Favorite B']);    // stale A committed NOTHING
    expect(st.rows).toBe(2);
  });

  test('a failed refresh keeps the cached rows and is logged as a failure', async ({ page }) => {
    // code-review-30 F6: the DeepZoom keep-last-good rule applies to Favorites.
    await page.evaluate((rows) => {
      window._failNext = false;
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list-favorites') {
          if (window._failNext) throw new Error('boom 503');
          return { favorites: rows };
        }
        throw new Error('unexpected ' + path);
      };
    }, FAVORITE_ROWS);
    await page.click('.tab-btn:text("Favorites")');
    await expect(page.locator('.favorite-art-row')).toHaveCount(2);
    await page.evaluate(() => { window._failNext = true; window._logLines = []; 
      const origLog = window.log;
      window.log = (msg, cls, target) => { window._logLines.push({ msg, cls }); return origLog(msg, cls, target); };
    });
    await page.click('#btn-favorites-refresh');
    await expect(page.locator('#favorites-status')).toContainText('showing cached list');
    await expect(page.locator('.favorite-art-row')).toHaveCount(2);   // never blanked
    const lines = await page.evaluate(() => window._logLines);
    expect(lines.some((l) => /Favorites refresh failed/.test(l.msg) && l.cls === 'err')).toBe(true);
    expect(lines.some((l) => /Favorites refreshed/.test(l.msg))).toBe(false);
  });

  test('an add mid-load survives the older response (CR30 follow-up F3)', async ({ page }) => {
    await page.evaluate((rows) => {
      window._listResolvers = [];
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list-favorites') {
          return new Promise((resolve) => { window._listResolvers.push(resolve); });
        }
        if (path === '/add-favorite') {
          return { added: true, favorite: { family: 'color', artifact_id: 'color_new',
            favorite_job_id: 'job_new', favorite_artifact_id: 'color_new',
            favorite_added_at: '2026-05-01T00:00:00Z', display_name: 'Brand New',
            image_key: 'renders/job_new/color/color_new/image.jpeg',
            preview_key: 'renders/job_new/color/color_new/preview.png',
            hydration_state: 'ready', missing: false } };
        }
        throw new Error('unexpected ' + path);
      };
      window._old = rows;
    }, FAVORITE_ROWS);
    await page.click('.tab-btn:text("Favorites")');               // slow load in flight
    await page.waitForFunction(() => window._listResolvers.length === 1);
    await page.evaluate(() => _addColorFavorite({ job_id: 'job_new', artifact_id: 'color_new' }, { force: true }));
    await page.evaluate(() => { window._listResolvers[0]({ favorites: window._old }); });   // old world lands late
    await page.waitForTimeout(150);
    const names = await page.evaluate(() => (_favoriteRefs || []).map((x) => x.display_name));
    expect(names).toContain('Brand New');                         // the add was NOT discarded
    // and patching a never-loaded cache must not mark it complete
    const flags = await page.evaluate(() => ({ loaded: _favoriteRefsLoaded }));
    expect(flags.loaded).toBe(false);                             // full list still pending a real load
  });

  test('a duplicate add never reorders or renames the existing row (CR30 F8)', async ({ page }) => {
    await page.evaluate((rows) => {
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list-favorites') return { favorites: rows };
        if (path === '/add-favorite') {
          // backend returns the AUTHORITATIVE stored row for a duplicate
          return { added: false, favorite: { ...rows[1] } };
        }
        throw new Error('unexpected ' + path);
      };
    }, FAVORITE_ROWS);
    await page.click('.tab-btn:text("Favorites")');
    await expect(page.locator('.favorite-art-row')).toHaveCount(2);
    const before = await page.evaluate(() => (_favoriteRefs || []).map((r) => r.display_name));
    await page.evaluate(() => _addColorFavorite({ job_id: 'job_fav_b', artifact_id: 'color_b' }, { force: true }));
    const after = await page.evaluate(() => (_favoriteRefs || []).map((r) => r.display_name));
    expect(after).toEqual(before);          // no front-running, no rename
  });

  test('GoRender switches to Render and selects the artifact', async ({ page }) => {
    await page.evaluate(({ rows, summaryA, detail }) => {
      window._mockFavoriteRows = rows.slice();
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list-favorites') return { favorites: window._mockFavoriteRows.slice() };
        if (path === '/render-summary') return body.job_id === 'job_fav_a' ? summaryA : { families: { color: [] } };
        if (path === '/detail') return detail;
        throw new Error(`unexpected storage path ${path}`);
      };
      _resultsCache = [
        { job_id: 'job_fav_a', function: 'poly_1', degree: 8, N: 2000, times: 1, total_size: 1200000 },
      ];
      _selectedJobId = null;
    }, { rows: FAVORITE_ROWS, summaryA: RENDER_SUMMARY_A, detail: RESULT_DETAIL });

    await page.click('.tab-btn:text("Favorites")');
    await page.click('#btn-favorites-go-render');

    await expect(page.locator('#tab-render')).toHaveClass(/active/);
    await expect(page.locator('#render-results-dir')).toHaveValue('job_fav_a');
    await expect(page.locator('#render-preview img')).toHaveAttribute('src', 'https://example.com/favorite-a-preview.png');
  });

  test('download menu stays wired and Image + Meta uses the selected artifact', async ({ page }) => {
    await page.evaluate(({ rows }) => {
      window._mockFavoriteRows = rows.slice();
      window._favoriteDownloads = [];
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list-favorites') return { favorites: window._mockFavoriteRows.slice() };
        if (path === '/render-summary') throw new Error('favorites must not call /render-summary');
        throw new Error(`unexpected storage path ${path}`);
      };
      window.downloadPresignedFile = async function (url, filename, explicitKey) {
        window._favoriteDownloads.push({ url, filename, explicitKey });
      };
      window._buildArtifactMeta = async function () { return { ok: true }; };
    }, { rows: FAVORITE_ROWS });

    await page.click('.tab-btn:text("Favorites")');
    await page.click('#btn-favorites-download');
    await expect(page.locator('#favorites-download-menu')).toBeVisible();
    await page.click('#favorites-dl-menu-file');

    const downloads = await page.evaluate(() => window._favoriteDownloads.slice());
    expect(downloads).toHaveLength(1);
    expect(downloads[0].explicitKey).toBe('renders/job_fav_a/color/color_a/image.jpeg');
    expect(downloads[0].filename).toContain('job_fav_a_color_a');
  });

  test('delete removes the favorite locally from a single-row response', async ({ page }) => {
    await page.evaluate(({ rows }) => {
      window.confirm = () => true;
      window._mockFavoriteRows = rows.slice();
      window._deleteCalls = [];
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list-favorites') return { favorites: window._mockFavoriteRows.slice() };
        if (path === '/render-summary') throw new Error('favorites must not call /render-summary');
        if (path === '/delete-favorite') {
          window._deleteCalls.push(body);
          // new single-row response shape (no full partition)
          return { deleted: true, job_id: body.job_id, artifact_id: body.artifact_id };
        }
        throw new Error(`unexpected storage path ${path}`);
      };
    }, { rows: FAVORITE_ROWS });

    await page.click('.tab-btn:text("Favorites")');
    await expect(page.locator('.favorite-art-row')).toHaveCount(2);
    await page.click('#btn-favorites-delete');
    await expect(page.locator('.favorite-art-row')).toHaveCount(1);
    await expect(page.locator('#favorites-log')).toContainText('Favorite removed');
    // exactly one delete call, and the UI patched locally (no re-list)
    expect(await page.evaluate(() => window._deleteCalls.length)).toBe(1);
  });
});
