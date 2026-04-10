// @ts-check
const { test, expect } = require('@playwright/test');

const FAVORITE_REFS = [
  {
    job_id: 'job_fav_a',
    artifact_id: 'color_a',
    family: 'color',
    added_at: '2026-04-08T12:00:00Z',
    display_name: 'Favorite A',
    image_key: 'renders/job_fav_a/color/color_a/image.jpeg',
    preview_key: 'renders/job_fav_a/color/color_a/preview.png',
  },
  {
    job_id: 'job_fav_b',
    artifact_id: 'color_b',
    family: 'color',
    added_at: '2026-04-08T13:00:00Z',
    display_name: 'Favorite B',
    image_key: 'renders/job_fav_b/color/color_b/image.jpeg',
    preview_key: 'renders/job_fav_b/color/color_b/preview.png',
  },
];

const FAVORITE_SUMMARIES = {
  job_fav_a: {
    calc: { exists: true, N: 2000, degree: 8 },
    families: {
      color: [
        {
          artifact_id: 'color_a',
          display_name: 'Favorite A',
          image_key: 'renders/job_fav_a/color/color_a/image.jpeg',
          image_url: 'https://example.com/favorite-a.jpeg',
          preview_key: 'renders/job_fav_a/color/color_a/preview.png',
          preview_url: 'https://example.com/favorite-a-preview.png',
          viewer_url: 'https://example.com/favorite-a-preview.png',
          width: 1200,
          height: 1200,
          file_size: 180000,
          color_mode: 'solve_score',
          palette: 'inferno',
          format: 'jpeg',
        },
      ],
      bilevel: [],
      coeffs: [],
      palette: [],
    },
    artifacts: {},
    deepzoom_latest: { exists: false },
  },
  job_fav_b: {
    calc: { exists: true, N: 3000, degree: 10 },
    families: {
      color: [
        {
          artifact_id: 'color_b',
          display_name: 'Favorite B',
          image_key: 'renders/job_fav_b/color/color_b/image.jpeg',
          image_url: 'https://example.com/favorite-b.jpeg',
          preview_key: 'renders/job_fav_b/color/color_b/preview.png',
          preview_url: 'https://example.com/favorite-b-preview.png',
          viewer_url: 'https://example.com/favorite-b-preview.png',
          width: 1600,
          height: 1600,
          file_size: 240000,
          color_mode: 'rainbow',
          format: 'jpeg',
        },
      ],
      bilevel: [],
      coeffs: [],
      palette: [],
    },
    artifacts: {},
    deepzoom_latest: { exists: false },
  },
};

const RESULT_DETAIL = {
  has_preview: true,
  preview_url: 'https://example.com/result-preview.png',
  file_count: 9,
  calc: { solver: 'aberth', function: 'poly_1' },
  pipeline: { function: 'poly_1', cfpv: [], coeff_transforms: [] },
  preview_stats: {
    n_roots: 80,
    n_roots_total: 100,
    q_re: [-1, 1],
    q_im: [-1, 1],
  },
};

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

test.describe('Favorites UI', () => {
  test('favorites tab loads artifacts, auto-selects one, and enables actions', async ({ page }) => {
    await page.evaluate(({ refs, summaries }) => {
      window._mockFavoriteRefs = refs.slice();
      window._mockFavoriteSummaries = summaries;
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list-favorites') return { favorites: window._mockFavoriteRefs.slice() };
        if (path === '/render-summary') return window._mockFavoriteSummaries[body.job_id] || { families: { color: [] } };
        throw new Error(`unexpected storage path ${path}`);
      };
    }, { refs: FAVORITE_REFS, summaries: FAVORITE_SUMMARIES });

    await page.click('.tab-btn:text("Favorites")');
    await expect(page.locator('.favorite-art-row')).toHaveCount(2);
    await expect(page.locator('#favorites-preview img')).toHaveAttribute('src', 'https://example.com/favorite-a-preview.png');
    await expect(page.locator('#favorites-info')).toContainText('2 favorites loaded.');
    await expect(page.locator('#btn-favorites-go-render')).toBeEnabled();
    await expect(page.locator('#btn-favorites-download')).toBeEnabled();
    await expect(page.locator('#btn-favorites-delete')).toBeEnabled();
  });

  test('GoRender switches to Render and selects the artifact', async ({ page }) => {
    await page.evaluate(({ refs, summaries, detail }) => {
      window._mockFavoriteRefs = refs.slice();
      window._mockFavoriteSummaries = summaries;
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list-favorites') return { favorites: window._mockFavoriteRefs.slice() };
        if (path === '/render-summary') return window._mockFavoriteSummaries[body.job_id] || { families: { color: [] } };
        if (path === '/detail') return detail;
        throw new Error(`unexpected storage path ${path}`);
      };
      _resultsCache = [
        { job_id: 'job_fav_a', function: 'poly_1', degree: 8, N: 2000, times: 1, total_size: 1200000 },
      ];
      _selectedJobId = null;
    }, { refs: FAVORITE_REFS, summaries: FAVORITE_SUMMARIES, detail: RESULT_DETAIL });

    await page.click('.tab-btn:text("Favorites")');
    await page.click('#btn-favorites-go-render');

    await expect(page.locator('#tab-render')).toHaveClass(/active/);
    await expect(page.locator('#render-results-dir')).toHaveValue('job_fav_a');
    await expect(page.locator('#render-preview img')).toHaveAttribute('src', 'https://example.com/favorite-a-preview.png');
  });

  test('download menu stays wired and Image + Meta uses the selected artifact', async ({ page }) => {
    await page.evaluate(({ refs, summaries }) => {
      window._mockFavoriteRefs = refs.slice();
      window._mockFavoriteSummaries = summaries;
      window._favoriteDownloads = [];
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list-favorites') return { favorites: window._mockFavoriteRefs.slice() };
        if (path === '/render-summary') return window._mockFavoriteSummaries[body.job_id] || { families: { color: [] } };
        throw new Error(`unexpected storage path ${path}`);
      };
      window.downloadPresignedFile = async function (url, filename, explicitKey) {
        window._favoriteDownloads.push({ url, filename, explicitKey });
      };
      window._buildArtifactMeta = async function () {
        return { ok: true };
      };
    }, { refs: FAVORITE_REFS, summaries: FAVORITE_SUMMARIES });

    await page.click('.tab-btn:text("Favorites")');
    await page.click('#btn-favorites-download');
    await expect(page.locator('#favorites-download-menu')).toBeVisible();
    await page.click('#favorites-dl-menu-file');

    const downloads = await page.evaluate(() => window._favoriteDownloads.slice());
    expect(downloads).toHaveLength(1);
    expect(downloads[0].explicitKey).toBe('renders/job_fav_a/color/color_a/image.jpeg');
    expect(downloads[0].filename).toContain('job_fav_a_color_a');
  });

  test('delete removes the selected favorite from the inventory', async ({ page }) => {
    await page.evaluate(({ refs, summaries }) => {
      window.confirm = () => true;
      window._mockFavoriteRefs = refs.slice();
      window._mockFavoriteSummaries = summaries;
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list-favorites') return { favorites: window._mockFavoriteRefs.slice() };
        if (path === '/render-summary') return window._mockFavoriteSummaries[body.job_id] || { families: { color: [] } };
        if (path === '/delete-favorite') {
          window._mockFavoriteRefs = window._mockFavoriteRefs.filter(
            ref => !(ref.job_id === body.job_id && ref.artifact_id === body.artifact_id)
          );
          return { favorites: window._mockFavoriteRefs.slice() };
        }
        throw new Error(`unexpected storage path ${path}`);
      };
    }, { refs: FAVORITE_REFS, summaries: FAVORITE_SUMMARIES });

    await page.click('.tab-btn:text("Favorites")');
    await expect(page.locator('.favorite-art-row')).toHaveCount(2);
    await page.click('#btn-favorites-delete');
    await expect(page.locator('.favorite-art-row')).toHaveCount(1);
    await expect(page.locator('#favorites-log')).toContainText('Favorite removed');
  });
});
