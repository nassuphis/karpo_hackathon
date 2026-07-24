// @ts-check
// Smoke coverage for the standalone sculpture viewer (sculpture.html): the
// (t1,t2) sweep's root sets lifted to z = t2. Mirrors the gallery-viewer
// smoke approach — SwiftShader forces a real WebGL scene in CI where
// possible, and every test degrades to asserting the readable fallback
// message when WebGL is genuinely unavailable. The data-shape checks (bad
// params, truncated bin) run before renderer creation, so they hold in any
// browser.
const { test, expect } = require('@playwright/test');

test.use({ launchOptions: { args: ['--enable-unsafe-swiftshader', '--use-gl=angle', '--use-angle=swiftshader', '--ignore-gpu-blocklist'] } });

const VIEWER = 'http://localhost:8765/sculpture.html';

// grid 4x4, degree 2: every step carries roots re=0.1,0.2 (im=0) so every
// point lands inside the [-1,1]^2 viewport and indexing is deterministic.
function rootsBuffer(gridN, degree) {
  const buf = Buffer.alloc(gridN * gridN * degree * 2 * 4);
  let o = 0;
  for (let step = 0; step < gridN * gridN; step++) {
    for (let r = 0; r < degree; r++) {
      buf.writeFloatLE(0.1 * (r + 1), o); o += 4;   // re
      buf.writeFloatLE(0.0, o); o += 4;             // im
    }
  }
  return buf;
}

test('no params shows a readable message (module graph loads)', async ({ page }) => {
  const errors = [];
  page.on('pageerror', (e) => errors.push(String(e)));
  await page.goto(VIEWER);
  await expect(page.locator('#message-title')).toHaveText('No sculpture specified');
  expect(errors, 'no uncaught module/runtime errors').toEqual([]);
});

test('a truncated roots bin fails with a readable size message', async ({ page }) => {
  await page.route('**/fx/roots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: Buffer.alloc(8),
  }));
  await page.route('**/fx/palette.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png',
    body: Buffer.from('iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==', 'base64'),
  }));
  const frag = new URLSearchParams({ v: '1', r: '/fx/roots.bin', p: '/fx/palette.png', n: '4', d: '2', s: '16', x0: '-1', x1: '1', y0: '-1', y1: '1' });
  await page.goto(VIEWER + '#' + frag.toString());
  await expect(page.locator('#message-title')).toHaveText('Sculpture data is too small');
});

test('builds the point cloud: serpentine z = t2, per-step palette colors, shadow flatten', async ({ page }) => {
  // synthesize the 4x4 palette PNG in-page (pixel (col,row) = rgb(row*60, col*60, 17))
  await page.goto(VIEWER);
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    for (let row = 0; row < 4; row++) {
      for (let col = 0; col < 4; col++) {
        g.fillStyle = `rgb(${row * 60},${col * 60},17)`;
        g.fillRect(col, row, 1, 1);
      }
    }
    return c.toDataURL('image/png').split(',')[1];
  });
  await page.route('**/fx/roots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: rootsBuffer(4, 2),
  }));
  await page.route('**/fx/palette.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/roots.bin', p: '/fx/palette.png', n: '4', d: '2', s: '16',
    x0: '-1', x1: '1', y0: '-1', y1: '1', t: 'fixture sculpture',
  });
  // hash-only navigation would not reload the already-open viewer — hop away
  await page.goto('about:blank');
  await page.goto(VIEWER + '#' + frag.toString());

  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    if (!v) return { built: false, msg: document.getElementById('message-title').textContent || '' };
    const pos = v.points.geometry.getAttribute('position').array;
    const col = v.points.geometry.getAttribute('color').array;
    const i4 = 4 * v.degree * 3;   // first root of global step 4
    return {
      built: true,
      count: v.count, clipped: v.clipped,
      hud: document.getElementById('hud-stats').textContent || '',
      title: document.getElementById('hud-title').textContent || '',
      x4: pos[i4], y4: pos[i4 + 1],
      c4: [col[i4], col[i4 + 1], col[i4 + 2]],
    };
  });
  if (!st.built) {
    expect(st.msg).toMatch(/WebGL/i);
    return;
  }
  expect(st.count).toBe(32);        // 16 steps x degree 2, nothing clipped
  expect(st.clipped).toBe(0);
  expect(st.title).toBe('fixture sculpture');
  expect(st.hud).toContain('32 roots');
  expect(st.hud).toContain('grid 4×4');
  // global step 4 = row 1 (odd, serpentine) j=0 -> col 3 -> t2=0.75 -> Y=0.25;
  // palette pixel (col 3, row 1) -> rgb(60, 180, 17)
  expect(st.y4).toBeCloseTo(0.25, 5);
  expect(st.x4).toBeCloseTo(0.05, 5);   // re=0.1 in a side-2 viewport
  expect(st.c4).toEqual([60, 180, 17]);

  // the height slider flattens the sculpture onto its base plane — the 2D
  // art is literally this shape's shadow
  const flat = await page.evaluate(() => {
    const ctl = document.getElementById('ctl-height');
    ctl.value = '0';
    ctl.dispatchEvent(new Event('input'));
    return window.__sculptureViewer.points.scale.y;
  });
  expect(flat).toBe(0);
});
