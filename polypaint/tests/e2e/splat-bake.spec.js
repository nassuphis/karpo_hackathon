// @ts-check
// The SplatBake artifact: one self-contained HTML file with the quantized
// splats embedded and a dependency-free WebGL2 renderer. This spec extracts
// the REAL generator from js/11 (marker-delimited), bakes a synthetic splat
// set, serves the resulting file, and asserts it boots and actually renders
// — pixels read back from the canvas, tour autoplay + interaction stop.
const fs = require('fs');
const path = require('path');
const { test, expect } = require('@playwright/test');

test.use({ launchOptions: { args: ['--enable-unsafe-swiftshader', '--use-gl=angle', '--use-angle=swiftshader', '--ignore-gpu-blocklist'] } });

const SRC = fs.readFileSync(path.join(__dirname, '..', '..', 'js', '11-artifacts.js'), 'utf8');
const I0 = SRC.indexOf('// SPLATBAKE_TEMPLATE_START');
const I1 = SRC.indexOf('// SPLATBAKE_TEMPLATE_END');
const GENERATOR = SRC.slice(I0, I1);

test('baked splat HTML: self-contained boot, solid render, tour stop', async ({ page }) => {
  expect(I0).toBeGreaterThan(-1);
  expect(I1).toBeGreaterThan(I0);

  // run the real generator in a blank page: one big red solid splat facing
  // the camera, plus a small green one off to the side, orbit tour playing
  await page.goto('about:blank');
  const html = await page.evaluate((gen) => {
    // eslint-disable-next-line no-eval
    eval(gen);
    // eslint-disable-next-line no-undef
    return _splatBakeHtml({
      title: 'bake fixture',
      count: 2,
      centers: new Float32Array([0, 0, 0, 0.8, 0, 0]),
      axisA: new Float32Array([0.4, 0, 0, 0.05, 0, 0]),
      axisB: new Float32Array([0, 0.4, 0, 0, 0.05, 0]),
      colors: new Float32Array([1, 0, 0, 0, 1, 0]),
      weights: new Float32Array([1, 1]),
      mode: 2,
      intensity: 1,
      cam: [0, 0, 1.5],
      target: [0, 0, 0],
      tour: 'orbit',
      tourSpeed: 1,
    });
  }, GENERATOR);
  expect(html.length).toBeGreaterThan(2000);
  expect(html).toContain('bake fixture');
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

  // the big solid splat must land red pixels on screen (dequantized
  // geometry + hand-rolled camera + instanced draw all working)
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
  expect(px.green).toBeGreaterThan(0);            // the small companion too

  // interaction hands the camera back: pointerdown stops the tour
  await page.dispatchEvent('#c', 'pointerdown', { pointerId: 1, clientX: 100, clientY: 100 });
  await page.waitForFunction(() => window.__bakedSplatViewer.playing === false, { timeout: 5000 });
  const title = await page.evaluate(() => document.getElementById('t').textContent);
  expect(title).toBe('bake fixture');
});
