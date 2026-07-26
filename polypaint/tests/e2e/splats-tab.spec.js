// @ts-check
// The global Splats tab: every saved viewer across ALL jobs (the server
// list was always global; the job scoping elsewhere is client-side), with
// the selected BAKED viewer embedded live in an iframe and arrow keys
// flipping between rows — the DeepZoom browsing pattern for baked splats.
const { test, expect } = require('@playwright/test');

const ROWS = [
  { id: 'scu_b1', kind: 'splatbake', title: 'Reef Bake', job_id: 'compute_j1',
    splat_count: 157675, bytes: 3600000, source_artifact_id: 'color_run_a',
    created_at: '2026-07-25T10:00:00Z', prefix: 'sculptures/scu_b1/' },
  { id: 'scu_b2', kind: 'splatbake', title: 'Ironman Bake', job_id: 'compute_j2',
    splat_count: 52000, bytes: 1200000, source_artifact_id: 'color_run_b',
    created_at: '2026-07-24T10:00:00Z', prefix: 'sculptures/scu_b2/' },
  { id: 'scu_full', kind: undefined, title: 'Full Save', job_id: 'compute_j1',
    grid_n: 384, degree: 20, palette: 'reef', roots_bytes: 29491200,
    created_at: '2026-07-23T10:00:00Z', prefix: 'sculptures/scu_full/' },
];

test.beforeEach(async ({ page }) => {
  await page.route('**/sculptures/**/viewer.html', (route) => route.fulfill({
    status: 200, contentType: 'text/html',
    body: '<!DOCTYPE html><title>baked stub</title><body>baked stub</body>',
  }));
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
  await page.evaluate((rows) => {
    window._openCalls = [];
    window.open = function (url) { window._openCalls.push(String(url)); return { closed: false }; };
    window.confirm = () => true;
    window._storageCalls = [];
    window.lambdaPost = async function (name, body, path) {
      window._storageCalls.push([path, JSON.parse(JSON.stringify(body || {}))]);
      if (path === '/list-sculptures') return { sculptures: rows, count: rows.length };
      if (path === '/delete-prefix') return { prefix: body.prefix, deleted: 1 };
      return {};
    };
  }, ROWS);
});

test('global listing, embedded frame, arrow browsing, Enter opens', async ({ page }) => {
  await page.click('.tab-btn:text("Splats")');

  // default filter: baked only — BOTH jobs' bakes, the full save hidden
  await expect(page.locator('#splats-list')).toContainText('Reef Bake');
  await expect(page.locator('#splats-list')).toContainText('Ironman Bake');
  await expect(page.locator('#splats-list')).not.toContainText('Full Save');
  await expect(page.locator('#splats-list')).toContainText('compute_j1');
  await expect(page.locator('#splats-list')).toContainText('compute_j2');
  await expect(page.locator('#splats-list')).toContainText('baked · 157,675 splats · 3.4MB');
  await expect(page.locator('#splats-list')).toContainText('src color_run_a');

  // first row auto-selected: after the debounce the frame shows ITS page
  await expect(page.locator('#splats-frame')).toBeVisible({ timeout: 5000 });
  await expect(page.locator('#splats-frame')).toHaveAttribute('src', /sculptures\/scu_b1\/viewer\.html/);
  await expect(page.locator('#splats-frame-empty')).toBeHidden();

  // arrow down: selection + frame follow; arrow up returns
  await page.keyboard.press('ArrowDown');
  await expect(page.locator('#splats-frame')).toHaveAttribute('src', /sculptures\/scu_b2\/viewer\.html/, { timeout: 5000 });
  await expect(page.locator('.splats-row[data-id="scu_b2"]')).toHaveCSS('background-color', 'rgb(29, 36, 56)');
  await page.keyboard.press('ArrowUp');
  await expect(page.locator('#splats-frame')).toHaveAttribute('src', /sculptures\/scu_b1\/viewer\.html/, { timeout: 5000 });

  // Enter opens the SELECTED row's share in its own tab
  await page.keyboard.press('Enter');
  const opens = await page.evaluate(() => window._openCalls);
  expect(opens).toHaveLength(1);
  expect(opens[0]).toContain('/sculptures/scu_b1/viewer.html');
});

test('full saves list under the filter but never embed', async ({ page }) => {
  await page.click('.tab-btn:text("Splats")');
  await page.selectOption('#splats-kind-filter', 'all');
  await expect(page.locator('#splats-list')).toContainText('Full Save');
  await expect(page.locator('#splats-list')).toContainText('384×384 · d20');
  await page.click('.splats-row[data-id="scu_full"]');
  // no iframe for a 29MB roots download — the frame area says why
  await expect(page.locator('#splats-frame')).toBeHidden();
  await expect(page.locator('#splats-frame-empty')).toContainText('full save');
  await expect(page.locator('#splats-frame-empty')).toContainText('Enter or Open');
  // but Enter still opens it in a tab
  await page.keyboard.press('Enter');
  const opens = await page.evaluate(() => window._openCalls);
  expect(opens[0]).toContain('/sculptures/scu_full/viewer.html');
});

test('delete prunes the global list; refresh has feedback', async ({ page }) => {
  await page.click('.tab-btn:text("Splats")');
  await expect(page.locator('#splats-list')).toContainText('Reef Bake');
  await page.locator('.splats-row[data-id="scu_b1"] button', { hasText: 'Delete' }).click();
  await expect(page.locator('#splats-list')).not.toContainText('Reef Bake');
  // selection falls to the next row and the frame follows
  await expect(page.locator('#splats-frame')).toHaveAttribute('src', /sculptures\/scu_b2\/viewer\.html/, { timeout: 5000 });
  const del = await page.evaluate(() =>
    window._storageCalls.find((c) => c[0] === '/delete-prefix')[1]);
  expect(del).toEqual({ prefix: 'sculptures/scu_b1/' });

  // the Refresh button carries busy + lingering feedback (house rule)
  await page.click('#btn-splats-refresh');
  await expect(page.locator('#btn-splats-refresh')).toHaveText('✓ Refreshed');
});
