// @ts-check
const { test, expect } = require('@playwright/test');

const RESULTS = [
  {
    job_id: 'compute_job_alpha',
    function: 'poly_1',
    degree: 8,
    N: 2000,
    times: 1,
    total_size: 1200000,
    n_chunks: 10,
  },
  {
    job_id: 'compute_job_beta',
    function: 'poly_2',
    degree: 12,
    N: 3000,
    times: 2,
    total_size: 2200000,
    n_chunks: 12,
  },
];

const DETAILS = {
  compute_job_alpha: {
    has_preview: true,
    preview_url: 'https://example.com/alpha-preview.png',
    file_count: 12,
    times: 1,
    calc: { solver: 'aberth_mt', function: 'poly_1' },
    pipeline: {
      function: 'poly_1',
      cfpv: ['1', '2'],
      coeff_transforms: [['power', '8']],
    },
    param_transforms_display: [['z01']],
    preview_stats: {
      n_roots: 100,
      n_roots_total: 120,
      q_re: [-1, 1],
      q_im: [-2, 2],
    },
  },
  compute_job_beta: {
    has_preview: true,
    preview_url: 'https://example.com/beta-preview.png',
    file_count: 18,
    times: 2,
    calc: { solver: 'companion_matrix', function: 'poly_2', N: 3000, n_chunks: 12 },
    pipeline: {
      function: 'poly_2',
      cfpv: ['7'],
      coeff_transforms: [['roots', '6', 'hi']],
    },
    param_transforms_display: [['unit_circle']],
    preview_stats: {
      n_roots: 240,
      n_roots_total: 300,
      q_re: [-3, 3],
      q_im: [-1.5, 1.5],
    },
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

test.describe('Results UI', () => {
  test('results tab loads rows and selecting one populates preview and target dirs', async ({ page }) => {
    await page.evaluate(({ results, details }) => {
      window._resultsListBodies = [];
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list') {
          window._resultsListBodies.push(body || {});
          return {
            results,
            count: results.length,
            list_us: 5000000,
            prefix_list_us: 200000,
            calc_fetch_us: 4700000,
            sort_us: 10000,
            list_workers: body.list_workers || 32,
            s3_pool_connections: 64,
          };
        }
        if (path === '/detail') return details[body.job_id];
        throw new Error(`unexpected storage path ${path}`);
      };
    }, { results: RESULTS, details: DETAILS });

    await page.click('.tab-btn:text("Results")');
    await expect(page.locator('#results-tbody tr')).toHaveCount(2);

    await page.locator('#results-tbody tr').first().click();
    await expect(page.locator('#btn-populate-result')).toBeEnabled();
    await expect(page.locator('#btn-preview')).toBeEnabled();
    await expect(page.locator('#btn-render-result')).toBeEnabled();
    await expect(page.locator('#btn-delete')).toBeEnabled();
    await expect(page.locator('#results-preview img')).toHaveAttribute('src', 'https://example.com/alpha-preview.png');
    await expect(page.locator('#results-info')).toContainText('12 files');

    await expect(page.locator('#render-results-dir')).toHaveValue('compute_job_alpha');
    await expect(page.locator('#palette-results-dir')).toHaveValue('compute_job_alpha');
    await expect(page.locator('#results-dir')).toHaveValue('compute_job_alpha');
  });

  test('results tab re-entry is cache-served; popup Run and cache-miss force a fetch', async ({ page }) => {
    await page.evaluate(({ results, details }) => {
      window._resultsListBodies = [];
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list') {
          window._resultsListBodies.push(body || {});
          return { results, count: results.length, list_us: 100000, list_workers: body.list_workers || 32 };
        }
        if (path === '/detail') return details[body.job_id];
        throw new Error(`unexpected storage path ${path}`);
      };
    }, { results: RESULTS, details: DETAILS });

    await page.click('.tab-btn:text("Results")');
    await expect(page.locator('#results-tbody tr')).toHaveCount(2);
    await page.click('.tab-btn:text("Compute")');
    await page.click('.tab-btn:text("Results")');
    await expect(page.locator('#results-tbody tr')).toHaveCount(2);   // instant, from cache
    expect(await page.evaluate(() => window._resultsListBodies.length)).toBe(1);

    // The popup's Run is an explicit refresh — it must always refetch.
    await page.click('#tab-results button:text("Refresh...")');
    await page.click('#results-refresh-popup-run');
    await expect.poll(() => page.evaluate(() => window._resultsListBodies.length)).toBe(2);

    // A finished compute marks the cache stale (js/12 sets _resultsLoaded=false).
    await page.evaluate(() => { _resultsLoaded = false; });
    await page.click('.tab-btn:text("Compute")');
    await page.click('.tab-btn:text("Results")');
    await expect.poll(() => page.evaluate(() => window._resultsListBodies.length)).toBe(3);
  });

  test('selection during an in-flight refresh awaits it instead of failing (CR30 F4)', async ({ page }) => {
    await page.evaluate(({ results }) => {
      window._listResolvers = [];
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list') {
          return new Promise((resolve) => { window._listResolvers.push(resolve); });
        }
        if (path === '/detail') return { has_preview: false, file_count: 1, calc: {} };
        return {};
      };
      window._newResults = results.concat([{ job_id: 'compute_new', function: 'poly_new', degree: 5, N: 100, times: 1, total_size: 10, n_chunks: 1 }]);
    }, { results: RESULTS });
    await page.click('.tab-btn:text("Results")');              // refresh in flight
    await page.waitForFunction(() => window._listResolvers.length === 1);
    const pending = page.evaluate(() => _ensureResultsSelection('compute_new').then(() => 'ok', (e) => 'err:' + e.message));
    await page.waitForTimeout(120);                            // selection is now awaiting the join
    await page.evaluate(() => {
      const payload = { results: window._newResults, count: window._newResults.length, list_us: 1000 };
      window._listResolvers.forEach((res) => res(payload));
    });
    expect(await pending).toBe('ok');                          // selection waited for the refresh
    // JOIN, not double-fetch: the forced selection shared the in-flight request
    expect(await page.evaluate(() => window._listResolvers.length)).toBe(1);
    // a REJECTED load surfaces its own error, never a fake "not found"
    await page.evaluate(() => {
      window.lambdaPost = async function (name, body, path) {
        if (path === '/list') throw new Error('network down');
        return {};
      };
      _resultsLoaded = false; _resultsCache = [];
    });
    const err = await page.evaluate(() => _ensureResultsSelection('compute_other').then(() => 'ok', (e) => e.message));
    expect(err).toContain('network down');
  });

  test('results refresh popup forwards worker count and filter mode updates placeholder and rows', async ({ page }) => {
    await page.evaluate(({ results, details }) => {
      window._resultsListBodies = [];
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list') {
          window._resultsListBodies.push(body || {});
          return {
            results,
            count: results.length,
            list_us: 5600000,
            prefix_list_us: 200000,
            calc_fetch_us: 5300000,
            sort_us: 10000,
            list_workers: body.list_workers || 32,
            s3_pool_connections: 64,
          };
        }
        if (path === '/detail') return details[body.job_id];
        throw new Error(`unexpected storage path ${path}`);
      };
    }, { results: RESULTS, details: DETAILS });

    await page.click('.tab-btn:text("Results")');
    await expect(page.locator('#results-tbody tr')).toHaveCount(2);

    await page.click('#tab-results button:text("Refresh...")');
    await expect(page.locator('#results-refresh-popup-overlay')).toBeVisible();
    await page.fill('#results-refresh-workers', '48');
    await page.check('#results-refresh-rebuild');
    await page.click('#results-refresh-popup-run');

    await expect(page.locator('#results-tbody tr')).toHaveCount(2);
    const lastListBody = await page.evaluate(() => window._resultsListBodies.at(-1));
    expect(lastListBody.list_workers).toBe(48);
    expect(lastListBody.rebuild).toBe(true);   // catalog rebuild escape hatch

    await page.selectOption('#results-filter-mode', 'job_id');
    await expect(page.locator('#results-filter')).toHaveAttribute('placeholder', 'Filter by job id...');
    await page.fill('#results-filter', 'beta');
    await page.click('#tab-results button:text("Filter")');
    await expect(page.locator('#results-tbody tr')).toHaveCount(1);
    await expect(page.locator('#results-tbody tr').first()).toContainText('job_beta');
  });

  test('populate restores compute settings and Go Render button switches to Render tab', async ({ page }) => {
    await page.evaluate(({ results, details }) => {
      window.lambdaPost = async function (name, body, path) {
        if (name !== 'storage') throw new Error(`unexpected ${name}`);
        if (path === '/list') {
          return {
            results,
            count: results.length,
            list_us: 5000000,
            prefix_list_us: 200000,
            calc_fetch_us: 4700000,
            sort_us: 10000,
            list_workers: body.list_workers || 32,
            s3_pool_connections: 64,
          };
        }
        if (path === '/detail') return details[body.job_id];
        throw new Error(`unexpected storage path ${path}`);
      };
    }, { results: RESULTS, details: DETAILS });

    await page.click('.tab-btn:text("Results")');
    await page.locator('#results-tbody tr').nth(1).click();

    await page.click('#btn-populate-result');
    await expect(page.locator('#tab-compute')).toHaveClass(/active/);
    await expect(page.locator('#render-function-picker')).toContainText('poly_2');
    await expect(page.locator('#render-times')).toHaveValue('2');
    await expect(page.locator('#compute-status')).toContainText('Populated from compute_job_beta');

    await page.click('.tab-btn:text("Results")');
    await page.click('#btn-render-result');
    await expect(page.locator('#tab-render')).toHaveClass(/active/);
    await expect(page.locator('#render-results-dir')).toHaveValue('compute_job_beta');
  });
});
