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
