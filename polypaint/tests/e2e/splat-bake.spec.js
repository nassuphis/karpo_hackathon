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
  // the camera, one small green companion, and ~3k dust splats so the
  // payload spans MULTIPLE base64 chunks — the user-hit empty-viewer bug
  // was btoa padding ('=') at every 32,768-byte chunk boundary (not a
  // multiple of 3), which makes the concatenation invalid and kills atob
  // on the baked page's first line. Single-chunk fixtures can never see it.
  await page.goto('about:blank');
  const COUNT = 3000;
  const html = await page.evaluate(({ gen, count }) => {
    // eslint-disable-next-line no-eval
    eval(gen);
    const centers = new Float32Array(count * 3);
    const axisA = new Float32Array(count * 3);
    const axisB = new Float32Array(count * 3);
    const colors = new Float32Array(count * 3);
    const weights = new Float32Array(count);
    // splat 0: the big red one facing the camera
    axisA.set([0.4, 0, 0], 0); axisB.set([0, 0.4, 0], 0);
    colors.set([1, 0, 0], 0); weights[0] = 1;
    // splat 1: the small green companion
    centers.set([0.8, 0, 0], 3);
    axisA.set([0.05, 0, 0], 3); axisB.set([0, 0.05, 0], 3);
    colors.set([0, 1, 0], 3); weights[1] = 1;
    // 2..n: deterministic blue dust spread behind the camera plane
    for (let i = 2; i < count; i++) {
      centers[i * 3] = ((i * 37) % 100) / 100 - 0.5;
      centers[i * 3 + 1] = ((i * 61) % 100) / 100 - 0.5;
      centers[i * 3 + 2] = -0.5 - ((i * 13) % 50) / 100;
      axisA[i * 3] = 0.003; axisB[i * 3 + 1] = 0.003;
      colors[i * 3 + 2] = 1; weights[i] = 0.5;
    }
    // eslint-disable-next-line no-undef
    return _splatBakeHtml({
      title: 'bake fixture', count,
      centers, axisA, axisB, colors, weights,
      mode: 2, intensity: 1,
      cam: [0, 0, 1.5], target: [0, 0, 0],
      tour: 'orbit', tourSpeed: 1,
    });
  }, { gen: GENERATOR, count: COUNT });
  expect(html.length).toBeGreaterThan(22 * COUNT);
  expect(html).toContain('bake fixture');
  expect(html).not.toContain('vendor/');          // truly self-contained
  expect(html).not.toContain('three.module');
  // the embedded payload must decode: exactly 22 bytes/splat, no mid-stream
  // padding (atob THROWS here under the broken chunking)
  const decoded = await page.evaluate(({ doc, count }) => {
    const m = doc.match(/var B64 = "([^"]*)";/);
    return { chunks: Math.ceil((22 * count) / 32766), bytes: atob(m[1]).length };
  }, { doc: html, count: COUNT });
  expect(decoded.chunks).toBeGreaterThan(1);       // multi-chunk for real
  expect(decoded.bytes).toBe(22 * COUNT);

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
  expect(st.count).toBe(3000);
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
