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

  // defaults: show=points (ribbons built but hidden), connect=file order,
  // height=0.1. File order draws the solver's own row order OPEN (A->B,
  // B->C only — no closing chord).
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
  expect(rb.v0[0]).toBeCloseTo(0.2, 5);    // A leads in file order
  expect(rb.v1[0]).toBeCloseTo(-0.2, 5);   // then B
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
  // the show selector hides the other primitive
  const vis = await page.evaluate(() => {
    const v = window.__sculptureViewer;
    const ctl = document.getElementById('ctl-mode');
    const set = (m) => { ctl.value = m; ctl.dispatchEvent(new Event('change')); };
    const out = [];
    set('ribbons'); out.push([v.points.visible, v.ribbons.visible]);
    set('points'); out.push([v.points.visible, v.ribbons.visible]);
    set('both'); out.push([v.points.visible, v.ribbons.visible]);
    return out;
  });
  expect(vis).toEqual([[false, true], [true, false], [true, true]]);

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
