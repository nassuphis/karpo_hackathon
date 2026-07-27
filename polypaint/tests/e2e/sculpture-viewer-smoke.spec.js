// @ts-check
// Smoke coverage for the standalone sculpture viewer (sculpture.html): the
// (t1,t2) sweep's root sets lifted to z = t2. Mirrors the gallery-viewer
// smoke approach — SwiftShader forces a real WebGL scene in CI where
// possible, and every test degrades to asserting the readable fallback
// message when WebGL is genuinely unavailable. The data-shape checks (bad
// params, truncated bin) run before renderer creation, so they hold in any
// browser.
const { test, expect } = require('@playwright/test');
const VIEW_SNAP_ORACLE = require('../fixtures/view_snap_projection_oracle.json');

test.use({ launchOptions: { args: ['--enable-unsafe-swiftshader', '--use-gl=angle', '--use-angle=swiftshader', '--ignore-gpu-blocklist'] } });

const VIEWER = 'http://localhost:8765/sculpture.html';

// line-primitive topology (ribbons/threads/clu) is built in a Web Worker —
// after any control change that triggers a build, wait for the refresh
// chain (worker job + buffer emit) to drain before reading geometry
const waitBuildsIdle = (page) => page.waitForFunction(() => {
  const v = window.__sculptureViewer;
  return !!v && v.pendingBuilds === 0;
}, { timeout: 20000 });

// grid 4x4, degree 3: every step carries roots A=(0.4,0), B=(-0.4,0),
// C=(0,0.4) — all inside the [-1,1]^2 viewport, and the triangle's angular
// tour (C, A, B about the centroid) differs from file order (A, B, C), so
// the ribbon connection modes are distinguishable. Interleaved [re,im] f32.
const FIXTURE_ROOTS = [[0.4, 0.0], [-0.4, 0.0], [0.0, 0.4]];
function rootsBuffer(gridN) {
  const buf = Buffer.alloc(gridN * gridN * FIXTURE_ROOTS.length * 2 * 4);
  let o = 0;
  for (let step = 0; step < gridN * gridN; step++) {
    for (const [re, im] of FIXTURE_ROOTS) {
      buf.writeFloatLE(re, o); o += 4;
      buf.writeFloatLE(im, o); o += 4;
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

test('embedded viewer exposes a square, identity-scoped camera snapshot protocol', async ({ page }) => {
  await page.route('**/snap-parent.html', (route) => route.fulfill({
    status: 200,
    contentType: 'text/html',
    body: `<!doctype html><body>
      <script>
        window.snapshotMessages = [];
        addEventListener('message', (event) => {
          window.snapshotMessages.push({ origin: event.origin, data: event.data });
        });
      </script>
    </body>`,
  }));
  await page.route('**/fx/snap-roots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: rootsBuffer(4),
  }));
  await page.route('**/fx/snap-palette.png', (route) => route.fulfill({
    status: 200,
    contentType: 'image/png',
    body: Buffer.from(
      'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==',
      'base64',
    ),
  }));
  await page.goto('http://localhost:8765/snap-parent.html');
  await page.evaluate(() => {
    const qp = new URLSearchParams({
      r: '/fx/snap-roots.bin',
      p: '/fx/snap-palette.png',
      n: '4',
      d: '3',
      s: '16',
      x0: '-1',
      x1: '1',
      y0: '-1',
      y1: '1',
      sid: 'scu_snap',
    });
    const frame = document.createElement('iframe');
    frame.id = 'snap-frame';
    frame.style.cssText = 'width:640px;height:360px;border:0';
    frame.src = '/sculpture.html#' + qp.toString();
    document.body.appendChild(frame);
  });
  await page.waitForFunction(() => window.snapshotMessages.some(
    (row) => row.data && row.data.type === 'polypaint-sculpture-ready'
      && row.data.sculpture_id === 'scu_snap',
  ), { timeout: 10000 });
  const frame = page.frames().find((candidate) => candidate.url().includes('/sculpture.html#'));
  expect(frame).toBeTruthy();
  await frame.waitForFunction(() => !!window.__sculptureViewer);
  await expect(frame.locator('#view-snap-guide')).toBeVisible();

  await page.evaluate(() => {
    const frame = document.getElementById('snap-frame');
    frame.contentWindow.postMessage({
      type: 'polypaint-sculpture-snapshot-request',
      protocol_version: 1,
      request_id: 'req-1',
      sculpture_id: 'scu_snap',
      target_aspect: 1,
    }, location.origin);
  });
  await page.waitForFunction(() => window.snapshotMessages.some(
    (row) => row.data && row.data.request_id === 'req-1',
  ));
  const response = await page.evaluate(() => window.snapshotMessages.find(
    (row) => row.data && row.data.request_id === 'req-1',
  ));
  expect(response.origin).toBe('http://localhost:8765');
  expect(response.data.error).toBeUndefined();
  expect(response.data.snapshot).toMatchObject({
    version: 1,
    projection: 'perspective',
    matrix_layout: 'column_major',
    vertical: 't2',
    slices: 0,
    frame: { aspect: 1, crop: 'center_square' },
  });
  expect(response.data.snapshot.model_view_matrix).toHaveLength(16);
  expect(response.data.snapshot.projection_matrix).toHaveLength(16);
  expect(response.data.snapshot.projection_matrix[0]).toBeCloseTo(
    response.data.snapshot.projection_matrix[5],
    10,
  );

  await frame.evaluate(() => {
    const viewer = window.__sculptureViewer;
    viewer.controls.enabled = false;
    viewer.camera.position.set(0, 0, 2);
    viewer.camera.quaternion.set(0, 0, 0, 1);
    viewer.camera.scale.set(1, 1, 1);
    viewer.camera.fov = 90;
    viewer.camera.near = 0.1;
    viewer.camera.far = 100;
    viewer.camera.updateProjectionMatrix();
    viewer.camera.updateMatrixWorld(true);
    viewer.sculpt.position.set(0, 0.5, 0);
    viewer.sculpt.quaternion.set(0, 0, 0, 1);
    viewer.sculpt.scale.set(1, 1, 1);
    viewer.sculpt.updateMatrixWorld(true);
  });
  await page.evaluate(() => {
    document.getElementById('snap-frame').contentWindow.postMessage({
      type: 'polypaint-sculpture-snapshot-request',
      protocol_version: 1,
      request_id: 'req-oracle',
      sculpture_id: 'scu_snap',
      target_aspect: 1,
    }, location.origin);
  });
  await page.waitForFunction(() => window.snapshotMessages.some(
    (row) => row.data && row.data.request_id === 'req-oracle',
  ));
  const oracleSnapshot = await page.evaluate(() => window.snapshotMessages.find(
    (row) => row.data && row.data.request_id === 'req-oracle',
  ).data.snapshot);
  for (const [idx, value] of VIEW_SNAP_ORACLE.camera.model_view_matrix.entries()) {
    expect(oracleSnapshot.model_view_matrix[idx]).toBeCloseTo(value, 12);
  }
  for (const [idx, value] of VIEW_SNAP_ORACLE.camera.projection_matrix.entries()) {
    expect(oracleSnapshot.projection_matrix[idx]).toBeCloseTo(value, 12);
  }
  expect(oracleSnapshot).not.toHaveProperty('point_world_size');
  expect(oracleSnapshot).not.toHaveProperty('point_scale');
  expect(oracleSnapshot).not.toHaveProperty('point_min_fraction');
  expect(oracleSnapshot).not.toHaveProperty('point_max_fraction');
  expect(oracleSnapshot).not.toHaveProperty('style');
  expect(oracleSnapshot).not.toHaveProperty('show');

  const projected = VIEW_SNAP_ORACLE.points.map((point) => {
    const multiply = (matrix, vector) => [0, 1, 2, 3].map((row) => (
      matrix[row] * vector[0]
      + matrix[4 + row] * vector[1]
      + matrix[8 + row] * vector[2]
      + matrix[12 + row] * vector[3]
    ));
    const row = Math.floor(point.step_start / VIEW_SNAP_ORACLE.grid_n);
    const j = point.step_start % VIEW_SNAP_ORACLE.grid_n;
    const col = (row & 1) ? VIEW_SNAP_ORACLE.grid_n - 1 - j : j;
    const t = col / VIEW_SNAP_ORACLE.grid_n;
    const local = [point.re / 2, t - 0.5, -point.im / 2, 1];
    const view = multiply(oracleSnapshot.model_view_matrix, local);
    const clip = multiply(oracleSnapshot.projection_matrix, view);
    const depth = -view[2];
    const center = [
      (clip[0] / clip[3] * 0.5 + 0.5) * VIEW_SNAP_ORACLE.pix,
      (0.5 - clip[1] / clip[3] * 0.5) * VIEW_SNAP_ORACLE.pix,
    ];
    return {
      center,
      depth,
    };
  });
  projected.forEach((actual, idx) => {
    const expected = VIEW_SNAP_ORACLE.points[idx];
    expect(actual.center[0]).toBeCloseTo(expected.pixel_center[0], 10);
    expect(actual.center[1]).toBeCloseTo(expected.pixel_center[1], 10);
    expect(actual.depth).toBeCloseTo(expected.depth, 10);
  });

  await frame.evaluate(() => {
    const style = document.getElementById('ctl-style');
    style.value = 'ghost';
    style.dispatchEvent(new Event('change'));
    document.getElementById('ctl-show-points').checked = false;
    document.getElementById('ctl-show-ribbons').checked = true;
  });
  await page.evaluate(() => {
    document.getElementById('snap-frame').contentWindow.postMessage({
      type: 'polypaint-sculpture-snapshot-request',
      protocol_version: 1,
      request_id: 'req-appearance',
      sculpture_id: 'scu_snap',
      target_aspect: 1,
    }, location.origin);
  });
  await page.waitForFunction(() => window.snapshotMessages.some(
    (row) => row.data && row.data.request_id === 'req-appearance',
  ));
  const appearance = await page.evaluate(() => window.snapshotMessages.find(
    (row) => row.data && row.data.request_id === 'req-appearance',
  ).data);
  expect(appearance.error).toBeUndefined();
  // WebGL appearance controls are deliberately not part of the hi-res
  // ViewRender contract. The viewer supplies camera geometry only.
  expect(appearance.snapshot.style).toBeUndefined();
  expect(appearance.snapshot.show).toBeUndefined();
  expect(appearance.snapshot.point_fraction).toBeUndefined();
  expect(appearance.snapshot.model_view_matrix).toHaveLength(16);
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
    status: 200, contentType: 'application/octet-stream', body: rootsBuffer(4),
  }));
  await page.route('**/fx/palette.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/roots.bin', p: '/fx/palette.png', n: '4', d: '3', s: '16',
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
  expect(st.count).toBe(48);        // 16 steps x degree 3, nothing clipped
  expect(st.clipped).toBe(0);
  expect(st.title).toBe('fixture sculpture');
  // ephemeral preview: menu ON by default; the toggle hides it
  await expect(page.locator('#panel')).toBeVisible();
  await page.click('#btn-menu');
  await expect(page.locator('#panel')).toBeHidden();
  await page.click('#btn-menu');
  await expect(page.locator('#panel')).toBeVisible();
  expect(st.hud).toContain('48 roots');
  expect(st.hud).toContain('grid 4×4');
  // global step 4 = row 1 (odd, serpentine) j=0 -> col 3 -> t2=0.75 -> Y=0.25;
  // palette pixel (col 3, row 1) -> rgb(60, 180, 17)
  expect(st.y4).toBeCloseTo(0.25, 5);
  expect(st.x4).toBeCloseTo(0.2, 5);    // root A: re=0.4 in a side-2 viewport
  expect(st.c4).toEqual([60, 180, 17]);

  // defaults: show=points only — line topology is LAZY (worker-built on
  // first enable; at 384^2 the old eager nearest build froze boot), so the
  // ribbons geometry stays empty until rib is checked.
  const lazy = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const pos = v.ribbons.geometry.getAttribute('position');
    return {
      verts: pos ? pos.count : 0,
      pointsVis: v.points.visible, ribbonsVis: v.ribbons.visible,
      scaleY: v.sculpt.scale.y,
    };
  });
  expect(lazy.verts).toBe(0);           // nothing built before rib is shown
  expect(lazy.pointsVis).toBe(true);    // default show=points
  expect(lazy.ribbonsVis).toBe(false);
  expect(lazy.scaleY).toBeCloseTo(0.1, 5);   // default height=0.1
  // enable ribbons: connect=nearest. Nearest = greedy chain from the
  // farthest-from-centroid root: starts at A (tie with B broken by file
  // position), hops to C (0.08 away vs B at 0.16) then B — an OPEN path
  // A->C->B that neither file order (A->B->C) nor the angle tour
  // (C->A->B closed) produces.
  await page.evaluate(() => {
    const ctl = document.getElementById('ctl-show-ribbons');
    ctl.checked = true;
    ctl.dispatchEvent(new Event('change'));
  });
  await waitBuildsIdle(page);
  const rb = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const pos = v.ribbons.geometry.getAttribute('position');
    return {
      verts: pos.count,
      v0: [pos.array[0], pos.array[2]], v1: [pos.array[3], pos.array[5]],
      ribbonsVis: v.ribbons.visible,
      hud: document.getElementById('hud-stats').textContent || '',
    };
  });
  expect(rb.verts).toBe(64);            // 16 solves x 2 open segments x 2 verts
  expect(rb.ribbonsVis).toBe(true);
  expect(rb.hud).toContain('32 ribbon segments');
  expect(rb.v0[0]).toBeCloseTo(0.2, 5);    // A leads (farthest, tie-broken)
  expect(rb.v1[0]).toBeCloseTo(0.0, 5);    // nearest hop: C, not B
  expect(rb.v1[1]).toBeCloseTo(-0.2, 5);
  // angle mode tours the triangle around its centroid CLOSED (3 segments),
  // starting at C (lowest angle: z=-0.2) — the file order A,B,C reorders
  await page.evaluate(() => {
    const ctl = document.getElementById('ctl-order');
    ctl.value = 'angle';
    ctl.dispatchEvent(new Event('change'));
  });
  await waitBuildsIdle(page);
  const ao = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const pos = v.ribbons.geometry.getAttribute('position');
    return {
      verts: pos.count,
      v0: [pos.array[0], pos.array[2]], v1: [pos.array[3], pos.array[5]],
      hud: document.getElementById('hud-stats').textContent || '',
    };
  });
  expect(ao.verts).toBe(96);            // 16 solves x 3 closed segments x 2 verts
  expect(ao.hud).toContain('48 ribbon segments');
  expect(ao.v0[0]).toBeCloseTo(0.0, 5);
  expect(ao.v0[1]).toBeCloseTo(-0.2, 5);
  expect(ao.v1[0]).toBeCloseTo(0.2, 5);
  expect(ao.v1[1]).toBeCloseTo(0.0, 5);
  // file order draws the solver's own row order OPEN (A->B, B->C)
  await page.evaluate(() => {
    const ctl = document.getElementById('ctl-order');
    ctl.value = 'file';
    ctl.dispatchEvent(new Event('change'));
  });
  await waitBuildsIdle(page);
  const fl = await page.evaluate(() => {
    const pos = window.__sculptureViewer.ribbons.geometry.getAttribute('position');
    return { verts: pos.count, x0: pos.array[0], x1: pos.array[3], z1: pos.array[5] };
  });
  expect(fl.verts).toBe(64);
  // segments emit length-sorted (for the len% drawRange quantile): file
  // order's chain is A->B (0.4) then B->C (0.283), so the SHORTER B->C
  // lands first in the buffer — the pin identifies the file-order chain by
  // its unique B->C edge (nearest connects A->C,C->B; angle C->A,A->B,B->C
  // closed has 96 verts, distinguishing it by count)
  expect(fl.x0).toBeCloseTo(-0.2, 5);    // B
  expect(fl.x1).toBeCloseTo(0.0, 5);     // C
  expect(fl.z1).toBeCloseTo(-0.2, 5);
  // the show checkboxes toggle each primitive independently
  const vis = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const set = (id, on) => {
      const ctl = document.getElementById(id);
      ctl.checked = on;
      ctl.dispatchEvent(new Event('change'));
    };
    const out = [];
    set('ctl-show-points', false); set('ctl-show-ribbons', true);
    out.push([v.points.visible, v.ribbons.visible]);
    set('ctl-show-points', true); set('ctl-show-ribbons', false);
    out.push([v.points.visible, v.ribbons.visible]);
    set('ctl-show-ribbons', true);
    out.push([v.points.visible, v.ribbons.visible]);
    return out;
  });
  expect(vis).toEqual([[false, true], [true, false], [true, true]]);
  await waitBuildsIdle(page);   // re-checking rib re-emits from cached topology

  // len% quantile: segments emit length-sorted, so the slider is a pure
  // drawRange. Order is FILE here: 32 segs = 16 short (B->C 0.283) + 16
  // long (A->B 0.4); at 50% only the short edges survive — the "large
  // jumps" vanish first when scrubbing down, the whole point of the control.
  const lq = await page.evaluate(() => {
    const ctl = document.getElementById('ctl-lenq');
    ctl.value = '50';
    ctl.dispatchEvent(new Event('input'));
    const v = window.__sculptureViewer;
    const g = v.ribbons.geometry;
    const pos = g.getAttribute('position');
    let maxLen = 0;
    for (let i = 0; i < g.drawRange.count; i += 2) {
      const dx = pos.array[i * 3] - pos.array[(i + 1) * 3];
      const dy = pos.array[i * 3 + 1] - pos.array[(i + 1) * 3 + 1];
      const dz = pos.array[i * 3 + 2] - pos.array[(i + 1) * 3 + 2];
      maxLen = Math.max(maxLen, Math.hypot(dx, dy, dz));
    }
    const at50 = { count: g.drawRange.count, maxLen,
                   hud: document.getElementById('hud-stats').textContent };
    ctl.value = '100';
    ctl.dispatchEvent(new Event('input'));
    return { at50, at100: { count: v.ribbons.geometry.drawRange.count } };
  });
  expect(lq.at50.count).toBe(32);            // 16 of 32 segments drawn
  expect(lq.at50.maxLen).toBeLessThan(0.3);  // only short edges survive
  expect(lq.at50.hud).toContain('len \u2264 50%');
  expect(lq.at100.count).toBe(64);

  // slices: t2 binned onto discrete plates. Grid 4 has t2 in {0,.25,.5,.75};
  // 3 slices -> levels floor(t2*3) = {0,0,1,2} -> Y in {-0.5, 0, +0.5}; whole
  // ribbons land on their plate (ribbon Y matches its step's plate). Off
  // restores the continuous Y.
  const sl = await page.evaluate(async () => {
    const v = window.__sculptureViewer;
    const ctl = document.getElementById('ctl-slices');
    // points move synchronously; ribbons re-emit off the cached topology in
    // a microtask — drain the refresh chain before reading their Y
    const idle = () => new Promise((res) => {
      const chk = () => (v.pendingBuilds === 0 ? res() : setTimeout(chk, 10));
      chk();
    });
    const distinctY = (attr) => {
      const ys = new Set();
      for (let i = 0; i < attr.count; i++) ys.add(Math.round(attr.array[i * 3 + 1] * 1e5) / 1e5);
      return Array.from(ys).sort((a, b) => a - b);
    };
    ctl.value = '3';
    ctl.dispatchEvent(new Event('change'));
    await idle();
    const at3 = {
      points: distinctY(v.points.geometry.getAttribute('position')),
      ribbons: distinctY(v.ribbons.geometry.getAttribute('position')),
      hud: document.getElementById('hud-stats').textContent || '',
      y4: v.points.geometry.getAttribute('position').array[4 * v.degree * 3 + 1],
    };
    ctl.value = '0';
    ctl.dispatchEvent(new Event('change'));
    await idle();
    const off = {
      y4: v.points.geometry.getAttribute('position').array[4 * v.degree * 3 + 1],
      hud: document.getElementById('hud-stats').textContent || '',
    };
    return { at3, off };
  });
  expect(sl.at3.points).toEqual([-0.5, 0, 0.5]);
  expect(sl.at3.ribbons).toEqual([-0.5, 0, 0.5]);
  expect(sl.at3.hud).toContain('3 slices');
  expect(sl.at3.y4).toBeCloseTo(0.5, 5);    // step 4: t2=0.75 -> top plate
  expect(sl.off.y4).toBeCloseTo(0.25, 5);   // continuous restored
  expect(sl.off.hud).not.toContain('slices');

  // z-axis transpose: step 4 = (row 1, col 3) — t2 axis puts it at
  // Y = 3/4 - 0.5 = +0.25, t1 axis at Y = 1/4 - 0.5 = -0.25
  const zx = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const ctl = document.getElementById('ctl-zaxis');
    const y4 = () => v.points.geometry.getAttribute('position').array[4 * v.degree * 3 + 1];
    ctl.value = 't1';
    ctl.dispatchEvent(new Event('change'));
    const t1 = { y4: y4(), hud: document.getElementById('hud-stats').textContent };
    ctl.value = 't2';
    ctl.dispatchEvent(new Event('change'));
    return { t1, back: y4() };
  });
  expect(zx.t1.y4).toBeCloseTo(-0.25, 5);
  expect(zx.t1.hud).toContain('z = t1');
  expect(zx.back).toBeCloseTo(0.25, 5);

  // style: SOLID by default — opaque, depth-written, so nearer plates
  // occlude farther ones from every angle (depthWrite:false let draw order
  // beat distance: bottom-slice points painted over the top plate). ghost
  // keeps the translucent veil as an explicit choice.
  const style = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const read = () => ({
      pt: [v.material.transparent, v.material.depthWrite, v.material.opacity],
      rb: [v.ribbons.material.transparent, v.ribbons.material.depthWrite, v.ribbons.material.opacity],
    });
    const solid = read();
    const ctl = document.getElementById('ctl-style');
    ctl.value = 'ghost';
    ctl.dispatchEvent(new Event('change'));
    const ghost = read();
    ctl.value = 'solid';
    ctl.dispatchEvent(new Event('change'));
    const back = read();
    return { solid, ghost, back };
  });
  expect(style.solid.pt).toEqual([false, true, 1]);
  expect(style.solid.rb).toEqual([false, true, 1]);
  expect(style.ghost.pt).toEqual([true, false, 0.92]);
  expect(style.ghost.rb).toEqual([true, false, 0.55]);
  expect(style.back.pt).toEqual([false, true, 1]);

  // colors must reach the screen BYTE-FOR-BYTE (washed-out vertex colors —
  // three's default linear interpretation of sRGB bytes — were a real
  // user-facing bug): spread the stacks, enlarge + opacify the points,
  // render a frame, and hunt the topmost palette triples exactly. With the
  // shared-roots fixture the 4 stack winners are row 3's palette row.
  const px = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const heightCtl = document.getElementById('ctl-height');
    heightCtl.value = '100';
    heightCtl.dispatchEvent(new Event('input'));
    v.ribbons.visible = false;
    v.material.size = 0.12;
    v.material.opacity = 1;
    v.material.transparent = false;
    v.material.needsUpdate = true;
    v.renderer.render(v.scene, v.camera);
    const gl = v.renderer.getContext();
    const w = gl.drawingBufferWidth, h = gl.drawingBufferHeight;
    const buf = new Uint8Array(w * h * 4);
    gl.readPixels(0, 0, w, h, gl.RGBA, gl.UNSIGNED_BYTE, buf);
    const hits = { '180,0,17': 0, '180,60,17': 0, '180,120,17': 0, '180,180,17': 0 };
    for (let i = 0; i < buf.length; i += 4) {
      const key = buf[i] + ',' + buf[i + 1] + ',' + buf[i + 2];
      if (key in hits) hits[key]++;
    }
    return hits;
  });
  for (const [key, n] of Object.entries(px)) {
    expect(n, `exact on-screen pixels for rgb(${key})`).toBeGreaterThan(0);
  }

  // OCCLUSION (the user's report): slices=2, big points, camera straight
  // above — the top plate must fully hide the bottom plate. Fixture colors
  // are (row*60, col*60, 17); cols {0,1} live on the bottom plate, cols
  // {2,3} on top, so any exact bottom triple on screen is a depth failure
  // (with depthWrite:false, last-drawn step 15 painted bottom color
  // (180,0,17) straight over the top plate).
  const occl = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const slicesCtl = document.getElementById('ctl-slices');
    slicesCtl.value = '2';
    slicesCtl.dispatchEvent(new Event('change'));
    // zero-parallax by construction: the camera axis runs straight down
    // THROUGH root C (0, -0.2) — C's top-plate point projects to the exact
    // screen center directly over its bottom twin, so the center pixels
    // must carry a TOP color (g in {120,180}). With the depthWrite:false
    // bug, the last-drawn coincident C (step 15, bottom plate, g <= 60)
    // painted over it — the user's "parts on top of top slice".
    v.material.size = 0.5;
    v.camera.position.set(0, 8.0, -0.2);
    v.camera.lookAt(0, 0, -0.2);
    v.renderer.render(v.scene, v.camera);
    const gl = v.renderer.getContext();
    const w = gl.drawingBufferWidth, h = gl.drawingBufferHeight;
    const patch = new Uint8Array(5 * 5 * 4);
    gl.readPixels(Math.floor(w / 2) - 2, Math.floor(h / 2) - 2, 5, 5, gl.RGBA, gl.UNSIGNED_BYTE, patch);
    let topPx = 0, other = 0;
    for (let i = 0; i < patch.length; i += 4) {
      if (patch[i + 2] === 17 && patch[i + 1] >= 120) topPx++;
      else other++;
    }
    slicesCtl.value = '0';
    slicesCtl.dispatchEvent(new Event('change'));
    return { topPx, other };
  });
  expect(occl.topPx).toBe(25);
  expect(occl.other).toBe(0);

  // z-window: clipping the top plate away makes the BOTTOM plate visible
  // from straight above — the same zero-parallax camera now sees a
  // bottom-color (g <= 60) center patch; restoring [0,1] brings the top
  // plate back. Behavioral proof the slab actually cuts geometry.
  const slab = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const slicesCtl = document.getElementById('ctl-slices');
    slicesCtl.value = '2';
    slicesCtl.dispatchEvent(new Event('change'));
    v.camera.position.set(0, 8.0, -0.2);
    v.camera.lookAt(0, 0, -0.2);
    const read = () => {
      v.renderer.render(v.scene, v.camera);
      const gl = v.renderer.getContext();
      const w = gl.drawingBufferWidth, h = gl.drawingBufferHeight;
      const patch = new Uint8Array(5 * 5 * 4);
      gl.readPixels(Math.floor(w / 2) - 2, Math.floor(h / 2) - 2, 5, 5, gl.RGBA, gl.UNSIGNED_BYTE, patch);
      let top = 0, bottom = 0;
      for (let i = 0; i < patch.length; i += 4) {
        if (patch[i + 2] !== 17) continue;
        if (patch[i + 1] >= 120) top++;
        else bottom++;
      }
      return { top, bottom };
    };
    const zhi = document.getElementById('ctl-zhi');
    zhi.value = '60';
    zhi.dispatchEvent(new Event('input'));
    const cut = read();
    const cutPlanes = [v.material.clippingPlanes[0].constant, v.material.clippingPlanes[1].constant];
    const fill = document.getElementById('zrange-fill');
    const fillState = { left: fill.style.left, width: fill.style.width };
    zhi.value = '100';
    zhi.dispatchEvent(new Event('input'));
    const restored = read();
    slicesCtl.value = '0';
    slicesCtl.dispatchEvent(new Event('change'));
    return { cut, cutPlanes, restored, fillState };
  });
  expect(slab.cut.top).toBe(0);            // top plate clipped away
  expect(slab.cut.bottom).toBe(25);        // bottom plate revealed
  expect(slab.cutPlanes[1]).toBeCloseTo(0.105, 5);   // (0.6-0.5)*1 + margin
  expect(slab.fillState).toEqual({ left: '0%', width: '60%' });   // one-track fill
  expect(slab.restored.top).toBe(25);      // full window brings it back

  // the height slider flattens the sculpture onto its base plane — the 2D
  // art is literally this shape's shadow
  const flat = await page.evaluate(() => {
    const ctl = document.getElementById('ctl-height');
    ctl.value = '0';
    ctl.dispatchEvent(new Event('input'));
    return window.__sculptureViewer.sculpt.scale.y;
  });
  expect(flat).toBe(0);
});

test('threads: mutual-nearest matching beats file order; slices split connectivity', async ({ page }) => {
  // grid 4, degree 2. Column c carries P=(0.1+0.2c, 0.1) and Q=(0.1+0.2c,
  // -0.3) — but ODD columns store them file-order-swapped [Q, P]. Correct
  // matching is same-letter (0.2 apart vs 0.447 crossed), so slot-identity
  // pairing is provably wrong at every even->odd boundary and the matcher
  // has to actually match.
  await page.goto('http://localhost:8765/sculpture.html');
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    g.fillStyle = 'rgb(10,20,30)';
    g.fillRect(0, 0, 4, 4);
    return c.toDataURL('image/png').split(',')[1];
  });
  const gridN = 4, degree = 2;
  const buf = Buffer.alloc(gridN * gridN * degree * 2 * 4);
  let o = 0;
  for (let step = 0; step < gridN * gridN; step++) {
    const row = Math.floor(step / gridN);
    const j = step % gridN;
    const col = (row & 1) ? (gridN - 1 - j) : j;
    const x = 0.1 + 0.2 * col;
    const P = [x, 0.1], Q = [x, -0.3];
    for (const [re, im] of (col & 1) ? [Q, P] : [P, Q]) {
      buf.writeFloatLE(re, o); o += 4;
      buf.writeFloatLE(im, o); o += 4;
    }
  }
  await page.route('**/fx/troots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: buf,
  }));
  await page.route('**/fx/tpal.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/troots.bin', p: '/fx/tpal.png', n: '4', d: '2', s: '16',
    x0: '-1', x1: '1', y0: '-1', y1: '1', t: 'threads fixture',
  });
  await page.goto('about:blank');
  await page.goto('http://localhost:8765/sculpture.html#' + frag.toString());
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const built = await page.evaluate(() => !!window.__sculptureViewer);
  if (!built) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }

  const st = await page.evaluate(async () => {
    const v = window.__sculptureViewer;
    const idle = () => new Promise((res) => {
      const chk = () => (v.pendingBuilds === 0 ? res() : setTimeout(chk, 10));
      chk();
    });
    const thr = document.getElementById('ctl-show-threads');
    thr.checked = true;
    thr.dispatchEvent(new Event('change'));
    await idle();                       // worker matching + emit
    const read = () => {
      const pos = v.threads.geometry.getAttribute('position');
      const segs = [];
      for (let i = 0; i < pos.count; i += 2) {
        segs.push([
          pos.array[i * 3], pos.array[i * 3 + 1], pos.array[i * 3 + 2],
          pos.array[(i + 1) * 3], pos.array[(i + 1) * 3 + 1], pos.array[(i + 1) * 3 + 2],
        ]);
      }
      return segs;
    };
    const continuous = read();
    const slicesCtl = document.getElementById('ctl-slices');
    slicesCtl.value = '2';
    slicesCtl.dispatchEvent(new Event('change'));
    await idle();                       // slice gate re-emits from cache
    const sliced = read();
    slicesCtl.value = '0';
    slicesCtl.dispatchEvent(new Event('change'));
    await idle();
    return { continuous, sliced, visible: v.threads.visible };
  });
  expect(st.visible).toBe(true);
  // no slicing: connect all — 3 t2 pairs x 2 roots x 4 rows = 24 segments
  expect(st.continuous).toHaveLength(24);
  // every segment connects SAME-letter roots: z endpoints equal (P z=-0.05,
  // Q z=+0.15) and x differs by exactly one column (0.1 normalized) — the
  // file-order-swapped odd columns would give z -0.05 -> +0.15 crossings
  for (const [x0, , z0, x1, , z1] of st.continuous) {
    expect(Math.abs(z1 - z0)).toBeLessThan(1e-6);
    expect(Math.abs(x1 - x0)).toBeCloseTo(0.1, 5);
  }
  // slices=2: per slice only — pairs (0,1) and (2,3); the 1->2 bridge is
  // GONE: 2 pairs x 2 roots x 4 rows = 16 segments, all flat on their plate
  expect(st.sliced).toHaveLength(16);
  for (const [, y0, , , y1] of st.sliced.map(sg => [sg[0], sg[1], sg[2], sg[3], sg[4], sg[5]])) {
    expect(Math.abs(y1 - y0)).toBeLessThan(1e-6);
  }
  const plateYs = new Set(st.sliced.map(sg => Math.round(sg[1] * 1e5) / 1e5));
  expect(Array.from(plateYs).sort((a, b) => a - b)).toEqual([-0.5, 0.5]);

  // z = t1 transposes the thread adjacency: matching runs along t1 within a
  // t2 column. This fixture's roots depend only on col, so t1-threads
  // connect IDENTICAL xz positions — pure vertical segments, dy = 0.25
  const zt = await page.evaluate(async () => {
    const v = window.__sculptureViewer;
    const idle = () => new Promise((res) => {
      const chk = () => (v.pendingBuilds === 0 ? res() : setTimeout(chk, 10));
      chk();
    });
    const ctl = document.getElementById('ctl-zaxis');
    ctl.value = 't1';
    ctl.dispatchEvent(new Event('change'));
    await idle();                       // t1 topology is a fresh worker build
    const pos = v.threads.geometry.getAttribute('position');
    const segs = [];
    for (let i = 0; i < pos.count; i += 2) {
      segs.push([
        Math.abs(pos.array[i * 3] - pos.array[(i + 1) * 3]),
        Math.abs(pos.array[i * 3 + 1] - pos.array[(i + 1) * 3 + 1]),
        Math.abs(pos.array[i * 3 + 2] - pos.array[(i + 1) * 3 + 2]),
      ]);
    }
    ctl.value = 't2';
    ctl.dispatchEvent(new Event('change'));
    await idle();
    return segs;
  });
  expect(zt).toHaveLength(24);   // 4 cols x 3 row-pairs x 2 roots
  for (const [dx, dy, dz] of zt) {
    expect(dx).toBeLessThan(1e-6);
    expect(dz).toBeLessThan(1e-6);
    expect(dy).toBeCloseTo(0.25, 5);
  }
});

test('nearest ribbons never bridge clusters: long chain chords are cut', async ({ page }) => {
  // two tight pairs far apart: the greedy chain is FORCED to bridge them
  // (any full path is), and that bridge — 16x the intra-pair spacing — must
  // be cut at 2.5x the median nearest-neighbor distance, leaving one short
  // strand per cluster. This was the user-visible "ribbon jumping" on real
  // jobs whose root constellations split into 2-4 clusters.
  await page.goto('http://localhost:8765/sculpture.html');
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    g.fillStyle = 'rgb(10,20,30)';
    g.fillRect(0, 0, 4, 4);
    return c.toDataURL('image/png').split(',')[1];
  });
  const gridN = 4, degree = 4;
  const CLUSTERS = [[0.2, 0.2], [0.3, 0.2], [1.4, -1.0], [1.5, -1.0]];
  const buf = Buffer.alloc(gridN * gridN * degree * 2 * 4);
  let o = 0;
  for (let step = 0; step < gridN * gridN; step++) {
    for (const [re, im] of CLUSTERS) {
      buf.writeFloatLE(re, o); o += 4;
      buf.writeFloatLE(im, o); o += 4;
    }
  }
  await page.route('**/fx/croots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: buf,
  }));
  await page.route('**/fx/cpal.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/croots.bin', p: '/fx/cpal.png', n: '4', d: '4', s: '16',
    x0: '-2', x1: '2', y0: '-2', y1: '2', t: 'cluster fixture',
  });
  await page.goto('about:blank');
  await page.goto('http://localhost:8765/sculpture.html#' + frag.toString());
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const built = await page.evaluate(() => !!window.__sculptureViewer);
  if (!built) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }
  await page.evaluate(() => {
    const ctl = document.getElementById('ctl-show-ribbons');
    ctl.checked = true;
    ctl.dispatchEvent(new Event('change'));
  });
  await waitBuildsIdle(page);
  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const pos = v.ribbons.geometry.getAttribute('position');
    let maxLen = 0;
    for (let i = 0; i < pos.count; i += 2) {
      const dx = pos.array[i * 3] - pos.array[(i + 1) * 3];
      const dz = pos.array[i * 3 + 2] - pos.array[(i + 1) * 3 + 2];
      maxLen = Math.max(maxLen, Math.hypot(dx, dz));
    }
    return {
      verts: pos.count,
      maxLen,
      hud: document.getElementById('hud-stats').textContent || '',
    };
  });
  // 16 solves x 2 kept strand segments (the forced bridge is cut)
  expect(st.verts).toBe(64);
  expect(st.hud).toContain('32 ribbon segments');
  // every kept segment is intra-pair (0.025 normalized), never the bridge (~0.4)
  expect(st.maxLen).toBeLessThan(0.06);
});

test('fly mode: double-click requests pointer lock; WASD moves along the look direction', async ({ page }) => {
  await page.goto('http://localhost:8765/sculpture.html');
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    g.fillStyle = 'rgb(10,20,30)';
    g.fillRect(0, 0, 4, 4);
    return c.toDataURL('image/png').split(',')[1];
  });
  await page.route('**/fx/froots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: rootsBuffer(4),
  }));
  await page.route('**/fx/fpal.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/froots.bin', p: '/fx/fpal.png', n: '4', d: '3', s: '16',
    x0: '-1', x1: '1', y0: '-1', y1: '1', t: 'fly fixture',
  });
  await page.goto('about:blank');
  await page.goto('http://localhost:8765/sculpture.html#' + frag.toString());
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const built = await page.evaluate(() => !!window.__sculptureViewer);
  if (!built) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }

  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    // double-click must request pointer lock on the canvas
    let plkCalls = 0;
    Element.prototype.requestPointerLock = function () { plkCalls++; };
    document.querySelector('#gl-root canvas').dispatchEvent(new MouseEvent('dblclick', { bubbles: true }));

    // lock/unlock listeners hand off between fly and orbit
    v.flight.plc.dispatchEvent({ type: 'lock' });
    const orbitDuringFly = v.controls.enabled;
    v.flight.plc.dispatchEvent({ type: 'unlock' });
    const orbitAfterFly = v.controls.enabled;

    // movement math: forward displacement follows the look direction; the
    // test override stands in for a real OS pointer lock (headless CI)
    v.flight.forceActive = true;
    const dir = new (v.camera.position.constructor)();
    v.camera.getWorldDirection(dir);
    const before = v.camera.position.clone();
    v.flight.keys.f = 1;
    v.flight.tick(0.5);
    const dispF = v.camera.position.clone().sub(before);
    const alongLook = dispF.clone().normalize().dot(dir);
    const forwardLen = dispF.length();
    // sprint triples the speed
    const beforeSprint = v.camera.position.clone();
    v.flight.keys.sprint = 1;
    v.flight.tick(0.5);
    const sprintLen = v.camera.position.clone().sub(beforeSprint).length();
    v.flight.keys.f = 0; v.flight.keys.sprint = 0;
    // E climbs straight up
    const beforeUp = v.camera.position.y;
    v.flight.keys.u = 1;
    v.flight.tick(0.5);
    const upDelta = v.camera.position.y - beforeUp;
    v.flight.keys.u = 0;
    v.flight.forceActive = false;

    // typing in a panel control must not move the camera
    const sel = document.getElementById('ctl-order');
    sel.dispatchEvent(new KeyboardEvent('keydown', { code: 'KeyW', bubbles: true }));
    const keyAfterSelect = v.flight.keys.f;

    return { plkCalls, orbitDuringFly, orbitAfterFly, alongLook, forwardLen, sprintLen, upDelta, keyAfterSelect };
  });
  expect(st.plkCalls).toBe(1);
  expect(st.orbitDuringFly).toBe(false);
  expect(st.orbitAfterFly).toBe(true);
  expect(st.alongLook).toBeGreaterThan(0.999);          // true look-direction flight
  expect(st.forwardLen).toBeCloseTo(0.45 * 0.5, 3);
  expect(st.sprintLen).toBeCloseTo(0.45 * 3 * 0.5, 3);  // Shift = 3x
  expect(st.upDelta).toBeCloseTo(0.45 * 0.5, 3);
  expect(st.keyAfterSelect).toBe(0);                    // inputs keep their keys
});

test('saved-sculpture mode: no hash params, boots from sibling meta.json', async ({ page }) => {
  // the saved prefix serves viewer.html + meta.json + data side by side; the
  // viewer opened bare must read ./meta.json and resolve data relatively
  await page.goto('http://localhost:8765/sculpture.html');
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    g.fillStyle = 'rgb(10,20,30)';
    g.fillRect(0, 0, 4, 4);
    return c.toDataURL('image/png').split(',')[1];
  });
  const meta = {
    version: 1, id: 'scu_test', title: 'saved fixture', job_id: 'j',
    grid_n: 4, degree: 3, step_count: 16, pass_count: 1,
    roots_key: 'roots.bin', palette_key: 'palette.png',
    viewport: { min_re: -1, max_re: 1, min_im: -1, max_im: 1 },
    view: {
      point: 22, height: 0.4, slices: 3,
      show: { points: false, ribbons: true, threads: false, clu: true, splats: true },
      splatRes: 64,
      style: 'ghost', order: 'angle', tour: 'weave', lenq: 50, zaxis: 't1',
      zlo: 0.25, zhi: 0.8, tourSpeed: 2,
    },
  };
  await page.route('**/sc/viewer.html', (route) => {
    const fs = require('fs');
    route.fulfill({ status: 200, contentType: 'text/html',
      body: fs.readFileSync(require('path').join(__dirname, '..', '..', 'sculpture.html')) });
  });
  await page.route('**/sc/meta.json', (route) => route.fulfill({
    status: 200, contentType: 'application/json', body: JSON.stringify(meta),
  }));
  await page.route('**/sc/roots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: rootsBuffer(4),
  }));
  await page.route('**/sc/palette.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  await page.goto('http://localhost:8765/sc/viewer.html');
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const booted = await page.evaluate(() => !!window.__sculptureViewer);
  if (!booted) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }
  // the saved view shows ribbons + clu — their worker builds run at boot
  await waitBuildsIdle(page);
  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const val = (id) => document.getElementById(id).value;
    const distinctY = () => {
      const a = v.points.geometry.getAttribute('position');
      const ys = new Set();
      for (let i = 0; i < a.count; i++) ys.add(Math.round(a.array[i * 3 + 1] * 1e5) / 1e5);
      return ys.size;
    };
    return {
      count: v.count,
      title: document.getElementById('hud-title').textContent,
      point: val('ctl-size'), height: val('ctl-height'), slices: val('ctl-slices'),
      style: val('ctl-style'), order: val('ctl-order'), tourMode: val('ctl-tour-mode'),
      lenq: val('ctl-lenq'), ribbonDraw: v.ribbons.geometry.drawRange.count,
      zaxis: val('ctl-zaxis'), zlo: val('ctl-zlo'), zhi: val('ctl-zhi'),
      tourSpeed: val('ctl-tour-speed'),
      splatRes: val('ctl-splat-res'), splatsVis: v.splats.visible, splatCount: v.splatCount,
      clipLoC: v.material.clippingPlanes[0].constant,
      clipHiC: v.material.clippingPlanes[1].constant,
      pointsVis: v.points.visible, ribbonsVis: v.ribbons.visible, threadsVis: v.threads.visible,
      cluVis: v.clu.visible,
      ghost: v.material.transparent === true && v.material.depthWrite === false,
      scaleY: v.sculpt.scale.y,
      plates: distinctY(),
      tourPlaying: v.tour.state.playing,
    };
  });
  expect(st.count).toBe(48);
  expect(st.title).toBe('saved fixture');
  // shares open CLEAN: menu OFF by default (phone screens); tap to open
  await expect(page.locator('#panel')).toBeHidden();
  await page.click('#btn-menu');
  await expect(page.locator('#panel')).toBeVisible();
  // the saved view opens exactly as prepared: controls, materials, geometry
  expect(st.point).toBe('22');
  expect(st.height).toBe('40');
  expect(st.slices).toBe('3');
  expect(st.style).toBe('ghost');
  expect(st.order).toBe('angle');
  expect(st.pointsVis).toBe(false);
  expect(st.ribbonsVis).toBe(true);
  expect(st.threadsVis).toBe(false);
  expect(st.cluVis).toBe(true);
  expect(st.ghost).toBe(true);
  expect(st.scaleY).toBeCloseTo(0.4, 5);
  expect(st.plates).toBe(3);            // slices applied to the geometry
  // the captured tour autoplays for the recipient
  expect(st.tourMode).toBe('weave');
  expect(st.tourPlaying).toBe(true);
  // len% travels with the view: 50% of the 48 angle segments drawn
  expect(st.lenq).toBe('50');
  expect(st.ribbonDraw).toBe(48);
  expect(st.zaxis).toBe('t1');
  expect(st.zlo).toBe('25');
  expect(st.zhi).toBe('80');
  expect(st.tourSpeed).toBe('2');
  expect(st.splatRes).toBe('64');
  expect(st.splatsVis).toBe(true);
  expect(st.splatCount).toBeGreaterThan(0);   // built at boot from the view
  // world constants track the 0.4 height scale (+0.005 outward margin)
  expect(st.clipLoC).toBeCloseTo(-(0.25 - 0.5) * 0.4 + 0.005, 5);
  expect(st.clipHiC).toBeCloseTo((0.8 - 0.5) * 0.4 + 0.005, 5);
});

test('tours: orbit and weave follow their parametric paths; interaction stops them', async ({ page }) => {
  await page.goto('http://localhost:8765/sculpture.html');
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    g.fillStyle = 'rgb(10,20,30)';
    g.fillRect(0, 0, 4, 4);
    return c.toDataURL('image/png').split(',')[1];
  });
  await page.route('**/fx/troots2.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: rootsBuffer(4),
  }));
  await page.route('**/fx/tpal2.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/troots2.bin', p: '/fx/tpal2.png', n: '4', d: '3', s: '16',
    x0: '-1', x1: '1', y0: '-1', y1: '1', t: 'tour fixture',
  });
  await page.goto('about:blank');
  await page.goto('http://localhost:8765/sculpture.html#' + frag.toString());
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const built = await page.evaluate(() => !!window.__sculptureViewer);
  if (!built) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }

  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const btn = document.getElementById('btn-tour-play');
    const out = { initial: { playing: v.tour.state.playing, btn: btn.textContent } };

    // play: the tick drives the camera along the orbit parametrization
    btn.click();
    out.playing = { playing: v.tour.state.playing, btn: btn.textContent, orbit: v.controls.enabled };
    v.tour.tick(1.0);
    const p1 = v.camera.position.clone();
    const want1 = v.tour.pose('orbit', v.tour.state.t, v.sculpt.scale.y);
    v.tour.tick(1.0);
    const p2 = v.camera.position.clone();
    out.orbitPath = {
      matches: Math.hypot(p1.x - want1.pos[0], p1.y - want1.pos[1], p1.z - want1.pos[2]) < 1e-9,
      moved: p1.distanceTo(p2) > 0.01,
      radius: Math.hypot(p2.x, p2.z),
      y: p2.y,
    };

    // weave resets t and flies the interior
    const mode = document.getElementById('ctl-tour-mode');
    mode.value = 'weave';
    mode.dispatchEvent(new Event('change'));
    out.weaveReset = v.tour.state.t;
    v.tour.tick(1.0);
    const w = v.camera.position.clone();
    const wantW = v.tour.pose('weave', v.tour.state.t, v.sculpt.scale.y);
    out.weavePath = {
      matches: Math.hypot(w.x - wantW.pos[0], w.y - wantW.pos[1], w.z - wantW.pos[2]) < 1e-9,
      radius: Math.hypot(w.x, w.z),
    };

    // pointerdown on the canvas hands control back — and the pivot returns
    // to the ORBIT'S look point (the cube middle), never a point just ahead
    // of the camera (that made drags orbit the viewpoint, feeling reversed)
    const stopLook = v.tour.pose(mode.value, v.tour.state.t, v.sculpt.scale.y).look;
    v.renderer.domElement.dispatchEvent(new PointerEvent('pointerdown', { bubbles: true }));
    out.afterPointer = { playing: v.tour.state.playing, btn: btn.textContent, orbit: v.controls.enabled,
                         target: [v.controls.target.x, v.controls.target.y, v.controls.target.z],
                         stopLook };

    // movement keys stop a re-started tour too
    btn.click();
    document.dispatchEvent(new KeyboardEvent('keydown', { code: 'KeyW', bubbles: true }));
    out.afterKey = { playing: v.tour.state.playing, keyF: v.flight.keys.f };
    document.dispatchEvent(new KeyboardEvent('keyup', { code: 'KeyW', bubbles: true }));
    return out;
  });
  expect(st.initial).toEqual({ playing: false, btn: '\u25b6' });
  expect(st.playing).toEqual({ playing: true, btn: '\u25a0', orbit: false });
  expect(st.orbitPath.matches).toBe(true);
  expect(st.orbitPath.moved).toBe(true);
  // pose adoption: the orbit circles at the PLAY-TIME camera radius and
  // height (boot camera sits at (1.25, 0.85, 1.25))
  expect(st.orbitPath.radius).toBeCloseTo(Math.hypot(1.25, 1.25), 4);
  expect(st.orbitPath.y).toBeCloseTo(0.85, 5);
  expect(st.weaveReset).toBe(0);
  expect(st.weavePath.matches).toBe(true);
  expect(st.weavePath.radius).toBeLessThan(1.0);      // interior pass
  expect(st.afterPointer.playing).toBe(false);
  expect(st.afterPointer.btn).toBe('\u25b6');
  expect(st.afterPointer.orbit).toBe(true);
  expect(st.afterPointer.target).toEqual(st.afterPointer.stopLook);   // pivot = the tour's look point
  expect(st.afterKey.playing).toBe(false);
  expect(st.afterKey.keyF).toBe(1);                   // the key still flies

  // weave from a standing start EASES IN from the current pose (no snap),
  // converges onto the parametric path, and adopts the play-time azimuth
  const bl = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const mode = document.getElementById('ctl-tour-mode');
    mode.value = 'weave';
    mode.dispatchEvent(new Event('change'));
    v.camera.position.set(2.0, 1.2, 0);
    v.tour.setPlaying(true);
    const blendStart = v.tour.state.blend;
    v.tour.tick(0.5);
    const mid = v.camera.position.clone();
    const wantMid = v.tour.pose('weave', v.tour.state.t, v.sculpt.scale.y);
    const midDelta = Math.hypot(mid.x - wantMid.pos[0], mid.y - wantMid.pos[1], mid.z - wantMid.pos[2]);
    v.tour.tick(2.0);
    const end = v.camera.position.clone();
    const wantEnd = v.tour.pose('weave', v.tour.state.t, v.sculpt.scale.y);
    const endDelta = Math.hypot(end.x - wantEnd.pos[0], end.y - wantEnd.pos[1], end.z - wantEnd.pos[2]);
    v.tour.setPlaying(false);
    return { blendStart, midDelta, endDelta, r0: v.tour.state.r0, angle0: v.tour.state.angle0 };
  });
  expect(bl.blendStart).toBe(0);
  expect(bl.midDelta).toBeGreaterThan(0.05);   // easing, not snapped
  expect(bl.endDelta).toBeLessThan(1e-9);      // converged onto the path
  expect(bl.r0).toBeCloseTo(2.0, 5);           // adopted radius
  expect(bl.angle0).toBeCloseTo(0, 5);         // adopted azimuth

  // WAVE: starts at the play-time height, rises first, sweeps top-to-bottom
  // within the stack amplitude; radius constant; speed selector scales t
  const wv = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const mode = document.getElementById('ctl-tour-mode');
    mode.value = 'wave';
    mode.dispatchEvent(new Event('change'));
    v.camera.position.set(1.5, 0.05, 0);   // inside the amplitude (0.08 at height 0.1)
    v.tour.setPlaying(true);
    const y0 = v.tour.pose('wave', 0, v.sculpt.scale.y).pos[1];
    const yRise = v.tour.pose('wave', 0.1, v.sculpt.scale.y).pos[1];
    let yMin = Infinity, yMax = -Infinity, rBad = 0;
    for (let t = 0; t < 20; t += 0.1) {
      const p = v.tour.pose('wave', t, v.sculpt.scale.y).pos;
      yMin = Math.min(yMin, p[1]); yMax = Math.max(yMax, p[1]);
      if (Math.abs(Math.hypot(p[0], p[2]) - 1.5) > 1e-9) rBad++;
    }
    // speed: 2x advances the clock twice as fast
    const spd = document.getElementById('ctl-tour-speed');
    spd.value = '2';
    const tBefore = v.tour.state.t;
    v.tour.tick(1.0);
    const tAfter = v.tour.state.t;
    spd.value = '1';
    v.tour.setPlaying(false);
    return { y0, yRise, yMin, yMax, rBad, dt2x: tAfter - tBefore };
  });
  expect(wv.y0).toBeCloseTo(0.05, 5);          // starts AT the current height
  expect(wv.yRise).toBeGreaterThan(0.05);      // and rises first
  expect(wv.yMin).toBeCloseTo(-0.08, 3);       // sweeps to the bottom...
  expect(wv.yMax).toBeCloseTo(0.08, 3);        // ...and the top of the stack
  expect(wv.rBad).toBe(0);                     // circle radius constant
  expect(wv.dt2x).toBeCloseTo(2.0, 9);         // 2x speed doubles the clock

  // GRAND: plateau circles with smoothstep laps between, radius adopted,
  // always watching the center; plateaus default +1.5/-0.5, adopt the
  // play-time height (mirrored through the center) when starting beyond
  // them, and the cycle begins at the NEARER plateau
  const gr = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const mode = document.getElementById('ctl-tour-mode');
    mode.value = 'grand';
    mode.dispatchEvent(new Event('change'));
    const lapT = (2 * Math.PI) / 0.35;
    const run = (y0) => {
      v.camera.position.set(1.8, y0, 0);
      v.tour.setPlaying(true);
      const at = (laps) => v.tour.pose('grand', laps * lapT, v.sculpt.scale.y);
      const out = {
        blendStart: v.tour.state.blend,
        gTop: v.tour.state.gTop, gBot: v.tour.state.gBot, phase0: v.tour.state.gPhase0,
        first: at(0.5).pos[1], mid: at(1.5).pos[1], third: at(2.5).pos[1],
        cycle: at(4.5).pos[1],
        r: Math.hypot(at(0.5).pos[0], at(0.5).pos[2]),
        look: at(1.5).look,
      };
      v.tour.setPlaying(false);
      return out;
    };
    return { inside: run(0.3), high: run(2.0), low: run(-1.8) };
  });
  // inside the band: default plateaus; y=0.3 is nearer the BOTTOM (0.8 vs
  // 1.2), so the cycle begins with the bottom circle, then ascends
  expect(gr.inside.blendStart).toBe(0);           // eases in from the camera
  expect(gr.inside.gTop).toBeCloseTo(1.5, 9);
  expect(gr.inside.gBot).toBeCloseTo(-0.5, 9);
  expect(gr.inside.phase0).toBe(2);
  expect(gr.inside.first).toBeCloseTo(-0.5, 9);   // bottom circle lap first
  expect(gr.inside.mid).toBeCloseTo(0.5, 9);      // ascent smoothstep midpoint
  expect(gr.inside.third).toBeCloseTo(1.5, 9);    // then the overhead circle
  expect(gr.inside.cycle).toBeCloseTo(-0.5, 9);   // the cycle repeats
  expect(gr.inside.r).toBeCloseTo(1.8, 5);        // play-time radius adopted
  expect(gr.inside.look).toEqual([0, 0, 0]);      // always watching the center
  // starting high: your height IS the plateau, mirrored through the center
  expect(gr.high.gTop).toBeCloseTo(2.0, 9);
  expect(gr.high.gBot).toBeCloseTo(-2.0, 9);
  expect(gr.high.phase0).toBe(0);                 // top is nearer — start there
  expect(gr.high.first).toBeCloseTo(2.0, 9);
  // starting low: same magnitude, cycle begins at the bottom circle
  expect(gr.low.gTop).toBeCloseTo(1.8, 9);
  expect(gr.low.gBot).toBeCloseTo(-1.8, 9);
  expect(gr.low.phase0).toBe(2);
  expect(gr.low.first).toBeCloseTo(-1.8, 9);      // starts circling at your level
});

test('clu ribbons: per-column k-means arcs, chained along the principal axis, never bridged', async ({ page }) => {
  // per column, each solve carries one root near x=-0.5 and one near
  // x=+0.5, drifting slightly with t1 — two clean trajectory arcs. k-means
  // (k = degree = 2) must separate them; each cluster chains its 4 points
  // end-to-end along its principal axis (3 tiny segments), and no segment
  // may bridge the arcs.
  await page.goto('http://localhost:8765/sculpture.html');
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    g.fillStyle = 'rgb(10,20,30)';
    g.fillRect(0, 0, 4, 4);
    return c.toDataURL('image/png').split(',')[1];
  });
  const gridN = 4, degree = 2;
  const buf = Buffer.alloc(gridN * gridN * degree * 2 * 4);
  let o = 0;
  for (let step = 0; step < gridN * gridN; step++) {
    const row = Math.floor(step / gridN);
    for (const re of [-0.5 + 0.02 * row, 0.5 + 0.02 * row]) {
      buf.writeFloatLE(re, o); o += 4;
      buf.writeFloatLE(0.0, o); o += 4;
    }
  }
  await page.route('**/fx/clroots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: buf,
  }));
  await page.route('**/fx/clpal.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/clroots.bin', p: '/fx/clpal.png', n: '4', d: '2', s: '16',
    x0: '-1', x1: '1', y0: '-1', y1: '1', t: 'clu fixture',
  });
  await page.goto('about:blank');
  await page.goto('http://localhost:8765/sculpture.html#' + frag.toString());
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const built = await page.evaluate(() => !!window.__sculptureViewer);
  if (!built) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }
  await page.evaluate(() => {
    // rib on too: the untouched-ribbons pin needs nearest ribbons built
    for (const id of ['ctl-show-ribbons', 'ctl-show-clu']) {
      const ctl = document.getElementById(id);
      ctl.checked = true;
      ctl.dispatchEvent(new Event('change'));
    }
  });
  await waitBuildsIdle(page);
  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const pos = v.clu.geometry.getAttribute('position');
    let maxLen = 0, bridge = 0, notFlat = 0;
    for (let i = 0; i < pos.count; i += 2) {
      const x0 = pos.array[i * 3], x1 = pos.array[(i + 1) * 3];
      const dy = Math.abs(pos.array[i * 3 + 1] - pos.array[(i + 1) * 3 + 1]);
      const len = Math.hypot(x0 - x1, dy, pos.array[i * 3 + 2] - pos.array[(i + 1) * 3 + 2]);
      maxLen = Math.max(maxLen, len);
      if (Math.sign(x0) !== Math.sign(x1)) bridge++;
      if (dy > 1e-9) notFlat++;
    }
    return { verts: pos.count, maxLen, bridge, notFlat, visible: v.clu.visible,
             ribbonsUntouched: v.ribbons.geometry.getAttribute('position').count };
  });
  // 4 columns x 2 clusters x 3 chain segments = 24 segments
  expect(st.verts).toBe(48);
  expect(st.visible).toBe(true);
  // per-solve ribbons are a SEPARATE primitive, untouched by clu
  expect(st.ribbonsUntouched).toBe(32);   // 16 solves x 1 nearest segment x 2 verts
  expect(st.bridge).toBe(0);                 // arcs never cross-stitched
  expect(st.maxLen).toBeLessThan(0.015);     // 0.01 steps only — no chords
  expect(st.notFlat).toBe(0);                // slice curves are flat per column

  // cancel: request a FRESH topology (z=t1 is uncached) and uncheck clu in
  // the same synchronous block — the worker round trip cannot complete
  // within one tick, so the cancel always lands first. The refresh chain
  // must drain without emitting: the geometry keeps its t2 build.
  const cancelled = await page.evaluate(async () => {
    const v = window.__sculptureViewer;
    const idle = () => new Promise((res) => {
      const chk = () => (v.pendingBuilds === 0 ? res() : setTimeout(chk, 10));
      chk();
    });
    const zaxis = document.getElementById('ctl-zaxis');
    const clu = document.getElementById('ctl-show-clu');
    zaxis.value = 't1';
    zaxis.dispatchEvent(new Event('change'));   // kicks the t1 clu build
    clu.checked = false;
    clu.dispatchEvent(new Event('change'));     // cancels it, same tick
    await idle();
    const afterCancel = {
      verts: v.clu.geometry.getAttribute('position').count,
      visible: v.clu.visible,
      build: document.getElementById('hud-build').textContent || '',
    };
    clu.checked = true;
    clu.dispatchEvent(new Event('change'));     // fresh t1 build, for real
    await idle();
    return { afterCancel, rebuiltVerts: v.clu.geometry.getAttribute('position').count };
  });
  expect(cancelled.afterCancel.verts).toBe(48);   // t2 emission untouched
  expect(cancelled.afterCancel.visible).toBe(false);
  expect(cancelled.afterCancel.build).toBe('');   // no dangling progress line
  // t1 on this fixture: per-row coincident quadruples — still 2 clusters x
  // 3 chain segments x 4 columns after the real rebuild
  expect(cancelled.rebuiltVerts).toBe(48);
});

test('cloud style: additive splats through the tone-mapped pipeline', async ({ page }) => {
  await page.goto('http://localhost:8765/sculpture.html');
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    for (let row = 0; row < 4; row++) for (let col = 0; col < 4; col++) {
      g.fillStyle = `rgb(${row * 60},${col * 60},60)`;
      g.fillRect(col, row, 1, 1);
    }
    return c.toDataURL('image/png').split(',')[1];
  });
  await page.route('**/fx/cdroots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: rootsBuffer(4),
  }));
  await page.route('**/fx/cdpal.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/cdroots.bin', p: '/fx/cdpal.png', n: '4', d: '3', s: '16',
    x0: '-1', x1: '1', y0: '-1', y1: '1', t: 'cloud fixture',
  });
  await page.goto('about:blank');
  await page.goto('http://localhost:8765/sculpture.html#' + frag.toString());
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const built = await page.evaluate(() => !!window.__sculptureViewer);
  if (!built) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }
  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const style = document.getElementById('ctl-style');
    style.value = 'cloud';
    style.dispatchEvent(new Event('change'));
    const read = () => {
      // render one explicit cloud frame via the loop path
      const gl = v.renderer.getContext();
      const w = gl.drawingBufferWidth, h = gl.drawingBufferHeight;
      const buf = new Uint8Array(w * h * 4);
      gl.readPixels(0, 0, w, h, gl.RGBA, gl.UNSIGNED_BYTE, buf);
      let lit = 0, maxV = 0;
      for (let i = 0; i < buf.length; i += 4) {
        const m = Math.max(buf[i], buf[i + 1], buf[i + 2]);
        if (m > 8) lit++;
        maxV = Math.max(maxV, m);
      }
      return { lit, maxV };
    };
    const cloudState = {
      swapped: v.points.material === v.cloudMat,
      hasUniforms: !!(v.points.material.uniforms && v.points.material.uniforms.uIntensity),
      ribbonsAdditive: v.ribbons.material.blending,
      clipPlanes: v.cloudMat.clippingPlanes.length,
    };
    // enlarge splats so the readback sees them regardless of dpr
    v.cloudMat.uniforms.uSize.value = 0.2;
    // drive a frame exactly as the loop does (tone-mapped two-pass)
    const glow = document.getElementById('ctl-glow');
    glow.value = '60';
    glow.dispatchEvent(new Event('input'));
    const exposure = v.tonemapMat.uniforms.uExposure.value;
    // force one frame through requestAnimationFrame timing
    return new Promise((resolve) => {
      requestAnimationFrame(() => {
        const shot = read();
        style.value = 'solid';
        style.dispatchEvent(new Event('change'));
        requestAnimationFrame(() => {
          resolve({ cloudState, exposure, shot,
                    restored: window.__sculptureViewer.points.material === window.__sculptureViewer.material });
        });
      });
    });
  });
  expect(st.cloudState.swapped).toBe(true);
  expect(st.cloudState.hasUniforms).toBe(true);
  expect(st.cloudState.ribbonsAdditive).toBe(2);   // THREE.AdditiveBlending
  expect(st.cloudState.clipPlanes).toBe(2);        // z-window still applies
  expect(st.exposure).toBeCloseTo(3.0, 5);         // glow 60 -> 60/20
  expect(st.shot.lit).toBeGreaterThan(50);         // splats visible on screen
  expect(st.shot.maxV).toBeLessThanOrEqual(255);   // tone map bounded
  expect(st.restored).toBe(true);                  // solid restores the material
});

test('u16 format: dequantizes over the viewport, sentinel pairs clip', async ({ page }) => {
  await page.goto('http://localhost:8765/sculpture.html');
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 2; c.height = 2;
    const g = c.getContext('2d');
    g.fillStyle = 'rgb(10,20,30)';
    g.fillRect(0, 0, 2, 2);
    return c.toDataURL('image/png').split(',')[1];
  });
  // grid 2, degree 2: quantized over viewport [-1,1]^2 — q = (v+1)/2*65534
  const gridN = 2, degree = 2;
  const q = (v) => Math.round((v + 1) / 2 * 65534);
  const vals = [];
  for (let step = 0; step < gridN * gridN; step++) {
    vals.push(q(0.5), q(-0.25));       // root 0: (0.5, -0.25)
    vals.push(0xFFFF, 0xFFFF);         // root 1: sentinel -> clipped
  }
  const buf = Buffer.alloc(vals.length * 2);
  vals.forEach((v, i) => buf.writeUInt16LE(v, i * 2));
  await page.route('**/fx/u16roots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: buf,
  }));
  await page.route('**/fx/u16pal.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/u16roots.bin', p: '/fx/u16pal.png', fmt: 'u16',
    n: '2', d: '2', s: '4', x0: '-1', x1: '1', y0: '-1', y1: '1', t: 'u16 fixture',
  });
  await page.goto('about:blank');
  await page.goto('http://localhost:8765/sculpture.html#' + frag.toString());
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const built = await page.evaluate(() => !!window.__sculptureViewer);
  if (!built) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }
  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const pos = v.points.geometry.getAttribute('position');
    return { count: v.count, clipped: v.clipped, x0: pos.array[0], z0: pos.array[2] };
  });
  expect(st.count).toBe(4);      // one live root per step
  expect(st.clipped).toBe(4);    // one sentinel per step
  // (0.5, -0.25) normalized in the side-2 viewport: X=0.25, Z=+0.125
  expect(st.x0).toBeCloseTo(0.25, 4);
  expect(st.z0).toBeCloseTo(0.125, 4);
});

test('recording: canvas stream to a downloadable video blob', async ({ page }) => {
  await page.goto(VIEWER);
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    g.fillStyle = 'rgb(10,20,30)';
    g.fillRect(0, 0, 4, 4);
    return c.toDataURL('image/png').split(',')[1];
  });
  await page.route('**/fx/recroots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: rootsBuffer(4),
  }));
  await page.route('**/fx/recpal.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/recroots.bin', p: '/fx/recpal.png', n: '4', d: '3', s: '16',
    x0: '-1', x1: '1', y0: '-1', y1: '1', t: 'rec fixture',
  });
  await page.goto('about:blank');
  await page.goto(VIEWER + '#' + frag.toString());
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const built = await page.evaluate(() => !!window.__sculptureViewer);
  if (!built) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }
  const supported = await page.evaluate(() =>
    !!window.MediaRecorder && !!HTMLCanvasElement.prototype.captureStream);
  if (!supported) return;   // environment without capture — nothing to pin

  // record ~1.5s of live frames (the loop renders continuously), then stop
  await page.click('#btn-tour-rec');
  await expect(page.locator('#btn-tour-rec')).toHaveText(/\u25a0/);
  const active = await page.evaluate(() => {
    const r = window.__sculptureViewer.recording;
    return !!(r.recorder && r.recorder.state === 'recording');
  });
  expect(active).toBe(true);
  await page.waitForTimeout(1500);
  await page.click('#btn-tour-rec');
  await page.waitForFunction(() => !!window.__lastSculptureRecording, { timeout: 10000 });
  const recd = await page.evaluate(() => window.__lastSculptureRecording);
  // a real encoded video came out of the canvas stream
  expect(recd.bytes).toBeGreaterThan(1000);
  expect(recd.mime).toMatch(/video\/(webm|mp4)/);
  // the button settles back to the idle dot after its lingering result
  await expect(page.locator('#btn-tour-rec')).toHaveText('\u25cf', { timeout: 5000 });
  // recorder fully released — a second recording can start
  const released = await page.evaluate(() => !window.__sculptureViewer.recording.recorder);
  expect(released).toBe(true);
});

test('motion LOD: thin index while the camera moves, full geometry at rest', async ({ page }) => {
  await page.goto(VIEWER);
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    g.fillStyle = 'rgb(10,20,30)';
    g.fillRect(0, 0, 4, 4);
    return c.toDataURL('image/png').split(',')[1];
  });
  await page.route('**/fx/lodroots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: rootsBuffer(4),
  }));
  await page.route('**/fx/lodpal.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/lodroots.bin', p: '/fx/lodpal.png', n: '4', d: '3', s: '16',
    x0: '-1', x1: '1', y0: '-1', y1: '1', t: 'lod fixture',
  });
  await page.goto('about:blank');
  await page.goto(VIEWER + '#' + frag.toString());
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const built = await page.evaluate(() => !!window.__sculptureViewer);
  if (!built) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }
  await page.evaluate(() => {
    const ctl = document.getElementById('ctl-show-ribbons');
    ctl.checked = true;
    ctl.dispatchEvent(new Event('change'));
  });
  await waitBuildsIdle(page);

  // the real motion detector: nudge the camera across a few frames
  await page.evaluate(() => new Promise((res) => {
    const v = window.__sculptureViewer;
    let n = 0;
    const step = () => {
      v.camera.position.x += 0.002;
      if (++n < 6) requestAnimationFrame(step); else res();
    };
    requestAnimationFrame(step);
  }));
  await page.waitForFunction(() => window.__sculptureViewer.lod.state.active, { timeout: 5000 });
  // SMALL scene: k stays 1 — LOD active but NEVER indexed (a no-op below
  // the target, so tiny fixtures render identically during motion)
  const small = await page.evaluate(() => ({
    indexed: !!window.__sculptureViewer.ribbons.geometry.index,
    draw: window.__sculptureViewer.ribbons.geometry.drawRange.count,
  }));
  expect(small.indexed).toBe(false);
  expect(small.draw).toBe(64);   // full 32 segments, unchanged mid-motion
  // stillness for >250ms hands the full buffer back (already unindexed)
  await page.waitForFunction(() => !window.__sculptureViewer.lod.state.active, { timeout: 5000 });

  // boosted stride: force k = 1 * boost(8) — 32 segments thin to 4, the
  // index strides every 8th segment of the length-sorted buffer, the hud
  // announces the factor, and len% applies to the thin set. ONE synchronous
  // evaluate: with the camera still, the rest rule would legitimately
  // disarm a manually-armed LOD on the next animation frame.
  const thin = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    v.lod.state.boost = 8;
    v.lod.set(true);
    const idx = v.ribbons.geometry.index;
    const armed = {
      indexed: !!idx,
      idxCount: idx ? idx.count : 0,
      first4: idx ? [idx.array[0], idx.array[1], idx.array[2], idx.array[3]] : [],
      draw: v.ribbons.geometry.drawRange.count,
      hud: document.getElementById('hud-build').textContent || '',
    };
    const ctl = document.getElementById('ctl-lenq');
    ctl.value = '50';
    ctl.dispatchEvent(new Event('input'));
    armed.drawAt50 = v.ribbons.geometry.drawRange.count;
    return armed;
  });
  expect(thin.indexed).toBe(true);
  expect(thin.idxCount).toBe(8);              // ceil(32/8)=4 segments x 2
  expect(thin.first4).toEqual([0, 1, 16, 17]); // segment 0, then segment 8
  expect(thin.draw).toBe(8);                   // lenq 100% of the thin set
  expect(thin.hud).toContain('motion LOD \u00d78');
  expect(thin.drawAt50).toBe(4);               // 50% of 4 thin segments

  // rest: index drops, len% recomputes over the FULL set, hud clears
  // (the still-camera rest rule may have disarmed it already — set(false)
  // is idempotent and the pins must hold either way)
  const rest = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    v.lod.set(false);
    return {
      indexed: !!v.ribbons.geometry.index,
      draw: v.ribbons.geometry.drawRange.count,
      hud: document.getElementById('hud-build').textContent || '',
    };
  });
  expect(rest.indexed).toBe(false);
  expect(rest.draw).toBe(32);                  // 50% of the full 32 segments
  expect(rest.hud).not.toContain('LOD');
});

test('splats: voxel-binned anisotropic gaussians — elongation, weights, slices rebuild', async ({ page }) => {
  // the clu-style drift fixture: per column two x-clusters whose 4 points
  // drift 0.02/row — at res 64 each cluster spans exactly TWO voxels of
  // TWO points each, collinear along x, so every splat must come out
  // x-elongated with equal weights
  await page.goto(VIEWER);
  const palB64 = await page.evaluate(() => {
    const c = document.createElement('canvas');
    c.width = 4; c.height = 4;
    const g = c.getContext('2d');
    g.fillStyle = 'rgb(10,20,30)';
    g.fillRect(0, 0, 4, 4);
    return c.toDataURL('image/png').split(',')[1];
  });
  const gridN = 4, degree = 2;
  const buf = Buffer.alloc(gridN * gridN * degree * 2 * 4);
  let o = 0;
  for (let step = 0; step < gridN * gridN; step++) {
    const row = Math.floor(step / gridN);
    for (const re of [-0.5 + 0.02 * row, 0.5 + 0.02 * row]) {
      buf.writeFloatLE(re, o); o += 4;
      buf.writeFloatLE(0.0, o); o += 4;
    }
  }
  await page.route('**/fx/sproots.bin', (route) => route.fulfill({
    status: 200, contentType: 'application/octet-stream', body: buf,
  }));
  await page.route('**/fx/sppal.png', (route) => route.fulfill({
    status: 200, contentType: 'image/png', body: Buffer.from(palB64, 'base64'),
  }));
  const frag = new URLSearchParams({
    v: '1', r: '/fx/sproots.bin', p: '/fx/sppal.png', n: '4', d: '2', s: '16',
    x0: '-1', x1: '1', y0: '-1', y1: '1', t: 'splat fixture',
  });
  await page.goto('about:blank');
  await page.goto(VIEWER + '#' + frag.toString());
  await page.waitForFunction(() =>
    !!window.__sculptureViewer || document.getElementById('message-box').classList.contains('show'),
  { timeout: 8000 });
  const built = await page.evaluate(() => !!window.__sculptureViewer);
  if (!built) {
    const msg = await page.evaluate(() => document.getElementById('message-title').textContent || '');
    expect(msg).toMatch(/WebGL/i);
    return;
  }
  await page.evaluate(() => {
    document.getElementById('ctl-splat-res').value = '64';
    const ctl = document.getElementById('ctl-show-splats');
    ctl.checked = true;
    ctl.dispatchEvent(new Event('change'));
  });
  await waitBuildsIdle(page);
  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const g = v.splats.geometry;
    const A = g.getAttribute('iAxisA').array;
    const B = g.getAttribute('iAxisB').array;
    const W = g.getAttribute('iWeight').array;
    const C = g.getAttribute('iColor').array;
    const P = g.getAttribute('iCenter').array;
    const n = v.splatCount;
    let xDominant = 0, aLonger = 0, wOne = 0, centered = 0;
    for (let i = 0; i < n; i++) {
      const ax = Math.abs(A[i * 3]), ay = Math.abs(A[i * 3 + 1]), az = Math.abs(A[i * 3 + 2]);
      if (ax > ay && ax > az) xDominant++;
      const la = Math.hypot(A[i * 3], A[i * 3 + 1], A[i * 3 + 2]);
      const lb = Math.hypot(B[i * 3], B[i * 3 + 1], B[i * 3 + 2]);
      if (la >= lb - 1e-9) aLonger++;
      if (Math.abs(W[i] - 1) < 1e-6) wOne++;   // all voxels hold 2 points -> weight 1
      if (Math.abs(P[i * 3 + 2]) < 1e-6) centered++;   // im=0 fixture -> z=0
    }
    return {
      count: n, xDominant, aLonger, wOne, centered,
      visible: v.splats.visible,
      c0: [C[0], C[1], C[2]],
      hud: document.getElementById('hud-stats').textContent || '',
    };
  });
  // 2 clusters x 2 voxels x 4 columns = 16 splats
  expect(st.count).toBe(16);
  expect(st.visible).toBe(true);
  expect(st.xDominant).toBe(16);   // covariance elongated along the drift
  expect(st.aLonger).toBe(16);     // major axis is the longer one
  expect(st.wOne).toBe(16);        // uniform occupancy -> uniform weights
  expect(st.centered).toBe(16);
  expect(st.hud).toContain('16 splats');
  // mean color of identical members = the member color, in floats
  expect(st.c0[0]).toBeCloseTo(10 / 255, 5);
  expect(st.c0[1]).toBeCloseTo(20 / 255, 5);
  expect(st.c0[2]).toBeCloseTo(30 / 255, 5);

  // slices=2 collapses Y onto two plates: 4 y-levels merge pairwise, so
  // the same xz voxels now hold 4 points each -> 8 splats, still weight 1
  const sliced = await page.evaluate(async () => {
    const v = window.__sculptureViewer;
    const idle = () => new Promise((res) => {
      const chk = () => (v.pendingBuilds === 0 ? res() : setTimeout(chk, 10));
      chk();
    });
    const ctl = document.getElementById('ctl-slices');
    ctl.value = '2';
    ctl.dispatchEvent(new Event('change'));
    await idle();
    const P = v.splats.geometry.getAttribute('iCenter').array;
    const ys = new Set();
    for (let i = 0; i < v.splatCount; i++) ys.add(Math.round(P[i * 3 + 1] * 1e4) / 1e4);
    ctl.value = '0';
    ctl.dispatchEvent(new Event('change'));
    await idle();
    return { count: v.splatCount === undefined ? -1 : ys.size >= 0 ? Array.from(ys).sort((a, b) => a - b) : [], n: ys.size, restored: v.splatCount };
  });
  expect(sliced.count).toEqual([-0.5, 0.5]);   // splat centers ON the plates
  expect(sliced.restored).toBe(16);            // slices off -> rebuilt full

  // splats follow the STYLE: solid = truly opaque depth-written surfels
  // (the "more opaque" ask), cloud = additive glow scaled by the glow
  // slider, ghost = translucent alpha
  const styles = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const mat = v.splats.material;
    const styleCtl = document.getElementById('ctl-style');
    const read = () => ({
      mode: mat.uniforms.uMode.value,
      blending: mat.blending,
      depthWrite: mat.depthWrite,
      transparent: mat.transparent,
    });
    const solid = read();                      // default style = solid
    styleCtl.value = 'cloud';
    styleCtl.dispatchEvent(new Event('change'));
    const cloud = read();
    const glowCtl = document.getElementById('ctl-glow');
    glowCtl.value = '60';
    glowCtl.dispatchEvent(new Event('input'));
    const intensity = mat.uniforms.uIntensity.value;
    styleCtl.value = 'ghost';
    styleCtl.dispatchEvent(new Event('change'));
    const ghost = read();
    styleCtl.value = 'solid';
    styleCtl.dispatchEvent(new Event('change'));
    return { solid, cloud, ghost, intensity, back: read() };
  });
  expect(styles.solid).toEqual({ mode: 2, blending: 1, depthWrite: true, transparent: false });   // NormalBlending, opaque
  expect(styles.cloud).toEqual({ mode: 0, blending: 2, depthWrite: false, transparent: true });   // AdditiveBlending
  expect(styles.ghost).toEqual({ mode: 1, blending: 1, depthWrite: false, transparent: true });
  expect(styles.intensity).toBeCloseTo(2, 5);   // glow 60 -> 60/30
  expect(styles.back).toEqual(styles.solid);
});
