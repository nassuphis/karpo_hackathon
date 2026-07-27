// @ts-check
// The hosted SplatBake page: substitute the REAL template file (the single
// source of truth the lores lambda substitutes server-side) with a
// test-built 22-byte/splat pack, serve it, and assert it boots and renders
// — pixels read back, tour autoplay, interaction stop. The pack builder
// here mirrors the documented quantization; the C producer is pinned to
// the same oracle in tests/test_splat_bake_tool.py.
const fs = require('fs');
const path = require('path');
const { test, expect } = require('@playwright/test');

test.use({ launchOptions: { args: ['--enable-unsafe-swiftshader', '--use-gl=angle', '--use-angle=swiftshader', '--ignore-gpu-blocklist'] } });

const TEMPLATE = fs.readFileSync(path.join(__dirname, '..', '..', 'splat_bake_template.html'), 'utf8');

function buildPack(splats) {
  const n = splats.length;
  const cmin = [Infinity, Infinity, Infinity];
  const cmax = [-Infinity, -Infinity, -Infinity];
  let amax = 1e-6;
  for (const s of splats) {
    for (let k = 0; k < 3; k++) {
      cmin[k] = Math.min(cmin[k], s.center[k]);
      cmax[k] = Math.max(cmax[k], s.center[k]);
      amax = Math.max(amax, Math.abs(s.axisA[k]), Math.abs(s.axisB[k]));
    }
  }
  const buf = Buffer.alloc(n * 22);
  let o = 0;
  for (const s of splats) {
    for (let k = 0; k < 3; k++) {
      const span = cmax[k] - cmin[k] || 1;
      buf.writeUInt16LE(Math.round((s.center[k] - cmin[k]) / span * 65535), o); o += 2;
    }
  }
  for (const key of ['axisA', 'axisB']) {
    for (const s of splats) {
      for (let k = 0; k < 3; k++) {
        buf.writeInt16LE(Math.round(s[key][k] / amax * 32767), o); o += 2;
      }
    }
  }
  for (const s of splats) {
    for (let k = 0; k < 3; k++) { buf.writeUInt8(Math.round(s.color[k] * 255), o); o += 1; }
  }
  for (const s of splats) { buf.writeUInt8(Math.round(s.weight * 255), o); o += 1; }
  return { b64: buf.toString('base64'), cmin, cmax, amax, count: n };
}

function bakeHtml(splats, header) {
  const pack = buildPack(splats);
  const full = {
    v: 1, count: pack.count, cmin: pack.cmin, cmax: pack.cmax, amax: pack.amax,
    ...header,
  };
  return TEMPLATE
    .replace(/__TITLE_HTML__/g, String(header.title).replace(/&/g, '&amp;').replace(/</g, '&lt;'))
    .replace('__HEADER_JSON__', JSON.stringify(full))
    .replace('__B64__', pack.b64);
}

test('baked splat template: self-contained boot, solid render, tour stop', async ({ page }) => {
  const splats = [
    { center: [0, 0, 0], axisA: [0.4, 0, 0], axisB: [0, 0.4, 0], color: [1, 0, 0], weight: 1 },
    { center: [0.8, 0, 0], axisA: [0.05, 0, 0], axisB: [0, 0.05, 0], color: [0, 1, 0], weight: 1 },
  ];
  const html = bakeHtml(splats, {
    mode: 2, intensity: 1, cam: [0, 0, 1.5], target: [0, 0, 0],
    tour: 'orbit', tourSpeed: 1, title: 'bake fixture',
  });
  expect(html).toContain('bake fixture');
  expect(html).not.toContain('__HEADER_JSON__');
  expect(html).not.toContain('__B64__');
  expect(html).not.toContain('vendor/');          // truly self-contained
  expect(html).not.toContain('three.module');

  await page.route('**/baked/fixture.html', (route) => route.fulfill({
    status: 200, contentType: 'text/html', body: html,
  }));
  await page.goto('http://localhost:8765/baked/fixture.html');
  await page.waitForFunction(() => {
    const v = window.__bakedSplatViewer;
    return (v && v.frames > 2) || document.getElementById('m').style.display === 'flex';
  }, { timeout: 8000 });
  const noGl = await page.evaluate(() => document.getElementById('m').style.display === 'flex');
  if (noGl) return;   // environment without WebGL2 — nothing to pin

  const st = await page.evaluate(() => window.__bakedSplatViewer);
  expect(st.count).toBe(2);
  expect(st.mode).toBe(2);
  expect(st.playing).toBe(true);                  // baked tour autoplays

  const px = await page.evaluate(() => {
    const c = document.getElementById('c');
    const gl = c.getContext('webgl2');
    const buf = new Uint8Array(c.width * c.height * 4);
    gl.readPixels(0, 0, c.width, c.height, gl.RGBA, gl.UNSIGNED_BYTE, buf);
    let red = 0, green = 0;
    for (let i = 0; i < buf.length; i += 4) {
      if (buf[i] > 150 && buf[i + 1] < 60) red++;
      if (buf[i + 1] > 150 && buf[i] < 60) green++;
    }
    return { red, green };
  });
  expect(px.red).toBeGreaterThan(50);
  expect(px.green).toBeGreaterThan(0);

  // interaction hands the camera back: pointerdown stops the tour
  await page.dispatchEvent('#c', 'pointerdown', { pointerId: 1, clientX: 100, clientY: 100 });
  await page.waitForFunction(() => window.__bakedSplatViewer.playing === false, { timeout: 5000 });
  const title = await page.evaluate(() => document.getElementById('t').textContent);
  expect(title).toBe('bake fixture');
});

test('baked splat template exposes its live camera as a ViewRender snapshot', async ({ page }) => {
  const html = bakeHtml([
    {
      center: [0, 0, 0],
      axisA: [0.2, 0, 0],
      axisB: [0, 0.2, 0],
      color: [1, 0.5, 0],
      weight: 1,
    },
  ], {
    sculptureId: 'scu_baked_snap',
    mode: 2,
    intensity: 1,
    zaxis: 't1',
    slices: 0,
    yscale: 0.25,
    pointWorldSize: 0.004,
    cam: [0, 0.4, 1.5],
    target: [0, 0, 0],
    tour: 'off',
    tourSpeed: 1,
    title: 'camera fixture',
  });
  await page.route('**/baked/snapshot.html*', (route) => route.fulfill({
    status: 200, contentType: 'text/html', body: html,
  }));
  await page.route('**/baked/parent.html', (route) => route.fulfill({
    status: 200,
    contentType: 'text/html',
    body: `<!doctype html><body><script>
      window.messages = [];
      addEventListener('message', (event) => window.messages.push(event.data));
      const frame = document.createElement('iframe');
      frame.id = 'baked-frame';
      frame.style.cssText = 'width:360px;height:640px;border:0';
      frame.src = '/baked/snapshot.html#sid=scu_baked_snap&embed=1';
      document.body.appendChild(frame);
    </script></body>`,
  }));
  await page.goto('http://localhost:8765/baked/parent.html');
  const frame = page.frames().find((candidate) => candidate.url().includes('/baked/snapshot.html'));
  expect(frame).toBeTruthy();
  await frame.waitForFunction(() => (
    (window.__bakedSplatViewer && window.__bakedSplatViewer.frames > 1)
    || document.getElementById('m').style.display === 'flex'
  ), { timeout: 8000 });
  if (await frame.evaluate(() => document.getElementById('m').style.display === 'flex')) return;

  await page.waitForFunction(() => window.messages.some(
    (message) => message.type === 'polypaint-sculpture-ready'
      && message.sculpture_id === 'scu_baked_snap',
  ));
  await page.evaluate(() => {
    document.getElementById('baked-frame').contentWindow.postMessage({
      type: 'polypaint-sculpture-snapshot-request',
      protocol_version: 1,
      request_id: 'baked-camera-1',
      sculpture_id: 'scu_baked_snap',
      target_aspect: 1,
    }, location.origin);
  });
  await page.waitForFunction(() => window.messages.some(
    (message) => message.request_id === 'baked-camera-1',
  ));
  const snapshot = await page.evaluate(() => window.messages.find(
    (message) => message.request_id === 'baked-camera-1',
  ).snapshot);
  expect(snapshot).toMatchObject({
    version: 1,
    projection: 'perspective',
    matrix_layout: 'column_major',
    vertical: 't1',
    slices: 0,
    frame: { aspect: 1, crop: 'center_square' },
  });
  expect(snapshot.projection_matrix[0]).toBeCloseTo(snapshot.projection_matrix[5], 7);
  expect(snapshot).not.toHaveProperty('point_world_size');
  expect(snapshot).not.toHaveProperty('point_scale');
  expect(snapshot).not.toHaveProperty('point_min_fraction');
  expect(snapshot).not.toHaveProperty('point_max_fraction');
  expect(snapshot).not.toHaveProperty('style');
  expect(snapshot).not.toHaveProperty('show');

  const liveView = await frame.evaluate(() => {
    const canvas = document.getElementById('c');
    const gl = canvas.getContext('webgl2');
    const program = gl.getParameter(gl.CURRENT_PROGRAM);
    return Array.from(gl.getUniform(program, gl.getUniformLocation(program, 'uView')));
  });
  for (let idx = 0; idx < 16; idx++) {
    const expected = idx >= 4 && idx < 8 ? liveView[idx] * 0.25 : liveView[idx];
    expect(snapshot.model_view_matrix[idx]).toBeCloseTo(expected, 7);
  }
});
