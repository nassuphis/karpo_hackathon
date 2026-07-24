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
  expect(st.hud).toContain('48 roots');
  expect(st.hud).toContain('grid 4×4');
  // global step 4 = row 1 (odd, serpentine) j=0 -> col 3 -> t2=0.75 -> Y=0.25;
  // palette pixel (col 3, row 1) -> rgb(60, 180, 17)
  expect(st.y4).toBeCloseTo(0.25, 5);
  expect(st.x4).toBeCloseTo(0.2, 5);    // root A: re=0.4 in a side-2 viewport
  expect(st.c4).toEqual([60, 180, 17]);

  // defaults: show=points (ribbons built but hidden), connect=nearest,
  // height=0.1. Nearest = greedy chain from the farthest-from-centroid
  // root: starts at A (tie with B broken by file position), hops to C
  // (0.08 away vs B at 0.16) then B — an OPEN path A->C->B that neither
  // file order (A->B->C) nor the angle tour (C->A->B closed) produces.
  const rb = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const pos = v.ribbons.geometry.getAttribute('position');
    return {
      verts: pos.count,
      v0: [pos.array[0], pos.array[2]], v1: [pos.array[3], pos.array[5]],
      pointsVis: v.points.visible, ribbonsVis: v.ribbons.visible,
      scaleY: v.sculpt.scale.y,
      hud: document.getElementById('hud-stats').textContent || '',
    };
  });
  expect(rb.verts).toBe(64);            // 16 solves x 2 open segments x 2 verts
  expect(rb.pointsVis).toBe(true);      // default show=points
  expect(rb.ribbonsVis).toBe(false);
  expect(rb.scaleY).toBeCloseTo(0.1, 5);   // default height=0.1
  expect(rb.hud).toContain('32 ribbon segments');
  expect(rb.v0[0]).toBeCloseTo(0.2, 5);    // A leads (farthest, tie-broken)
  expect(rb.v1[0]).toBeCloseTo(0.0, 5);    // nearest hop: C, not B
  expect(rb.v1[1]).toBeCloseTo(-0.2, 5);
  // angle mode tours the triangle around its centroid CLOSED (3 segments),
  // starting at C (lowest angle: z=-0.2) — the file order A,B,C reorders
  const ao = await page.evaluate(() => {
    const ctl = document.getElementById('ctl-order');
    ctl.value = 'angle';
    ctl.dispatchEvent(new Event('change'));
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
  const fl = await page.evaluate(() => {
    const ctl = document.getElementById('ctl-order');
    ctl.value = 'file';
    ctl.dispatchEvent(new Event('change'));
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
  const sl = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const ctl = document.getElementById('ctl-slices');
    const distinctY = (attr) => {
      const ys = new Set();
      for (let i = 0; i < attr.count; i++) ys.add(Math.round(attr.array[i * 3 + 1] * 1e5) / 1e5);
      return Array.from(ys).sort((a, b) => a - b);
    };
    ctl.value = '3';
    ctl.dispatchEvent(new Event('change'));
    const at3 = {
      points: distinctY(v.points.geometry.getAttribute('position')),
      ribbons: distinctY(v.ribbons.geometry.getAttribute('position')),
      hud: document.getElementById('hud-stats').textContent || '',
      y4: v.points.geometry.getAttribute('position').array[4 * v.degree * 3 + 1],
    };
    ctl.value = '0';
    ctl.dispatchEvent(new Event('change'));
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

  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const thr = document.getElementById('ctl-show-threads');
    thr.checked = true;
    thr.dispatchEvent(new Event('change'));
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
    const sliced = read();
    slicesCtl.value = '0';
    slicesCtl.dispatchEvent(new Event('change'));
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
  const zt = await page.evaluate(() => {
    const ctl = document.getElementById('ctl-zaxis');
    ctl.value = 't1';
    ctl.dispatchEvent(new Event('change'));
    const pos = window.__sculptureViewer.threads.geometry.getAttribute('position');
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
      show: { points: false, ribbons: true, threads: false },
      style: 'ghost', order: 'angle', tour: 'weave', lenq: 50, zaxis: 't1',
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
  const st = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    if (!v) return { built: false, msg: document.getElementById('message-title').textContent || '' };
    const val = (id) => document.getElementById(id).value;
    const distinctY = () => {
      const a = v.points.geometry.getAttribute('position');
      const ys = new Set();
      for (let i = 0; i < a.count; i++) ys.add(Math.round(a.array[i * 3 + 1] * 1e5) / 1e5);
      return ys.size;
    };
    return {
      built: true, count: v.count,
      title: document.getElementById('hud-title').textContent,
      point: val('ctl-size'), height: val('ctl-height'), slices: val('ctl-slices'),
      style: val('ctl-style'), order: val('ctl-order'), tourMode: val('ctl-tour-mode'),
      lenq: val('ctl-lenq'), ribbonDraw: v.ribbons.geometry.drawRange.count,
      zaxis: val('ctl-zaxis'),
      pointsVis: v.points.visible, ribbonsVis: v.ribbons.visible, threadsVis: v.threads.visible,
      ghost: v.material.transparent === true && v.material.depthWrite === false,
      scaleY: v.sculpt.scale.y,
      plates: distinctY(),
      tourPlaying: v.tour.state.playing,
    };
  });
  if (!st.built) {
    expect(st.msg).toMatch(/WebGL/i);
    return;
  }
  expect(st.count).toBe(48);
  expect(st.title).toBe('saved fixture');
  // the saved view opens exactly as prepared: controls, materials, geometry
  expect(st.point).toBe('22');
  expect(st.height).toBe('40');
  expect(st.slices).toBe('3');
  expect(st.style).toBe('ghost');
  expect(st.order).toBe('angle');
  expect(st.pointsVis).toBe(false);
  expect(st.ribbonsVis).toBe(true);
  expect(st.threadsVis).toBe(false);
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

    // pointerdown on the canvas hands control back
    v.renderer.domElement.dispatchEvent(new PointerEvent('pointerdown', { bubbles: true }));
    out.afterPointer = { playing: v.tour.state.playing, btn: btn.textContent, orbit: v.controls.enabled };

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
  expect(st.orbitPath.radius).toBeCloseTo(1.35, 5);   // exterior turntable
  expect(st.weaveReset).toBe(0);
  expect(st.weavePath.matches).toBe(true);
  expect(st.weavePath.radius).toBeLessThan(1.0);      // interior pass
  expect(st.afterPointer).toEqual({ playing: false, btn: '\u25b6', orbit: true });
  expect(st.afterKey.playing).toBe(false);
  expect(st.afterKey.keyF).toBe(1);                   // the key still flies
});
