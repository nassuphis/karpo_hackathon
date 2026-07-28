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

test('baked splat navigation: pan, zoom-to-cursor, fly toggle, center', async ({ page }) => {
  // vantage hunting is the point of a baked view (SnapRender doctrine):
  // right/shift-drag pans the orbit target, wheel zooms TOWARD the cursor,
  // "1" toggles wasd+mouselook fly, "2" re-aims at the bake center.
  // All reads go through __bakedSplatViewer.state() — live on call — not
  // the per-frame export fields, which lag a frame and freeze when rAF
  // throttles an occluded page.
  const splats = [
    { center: [0, 0, 0], axisA: [0.3, 0, 0], axisB: [0, 0.3, 0], color: [1, 0, 0], weight: 1 },
  ];
  const html = bakeHtml(splats, {
    mode: 2, intensity: 1, cam: [0, 0, 1.5], target: [0, 0, 0],
    tour: 'off', tourSpeed: 1, title: 'nav fixture', zaxis: 't2', slices: 0, yscale: 0.5,
  });
  await page.route('**/baked/nav.html', (route) => route.fulfill({
    status: 200, contentType: 'text/html', body: html,
  }));
  await page.goto('http://localhost:8765/baked/nav.html');
  await page.waitForFunction(() => {
    const v = window.__bakedSplatViewer;
    return (v && v.frames > 2) || document.getElementById('m').style.display === 'flex';
  }, { timeout: 8000 });
  if (await page.evaluate(() => document.getElementById('m').style.display === 'flex')) return;

  // in-page dispatch: the listeners read PointerEvent/WheelEvent init
  // fields (shiftKey, clientX/Y) that page.dispatchEvent does not carry
  await page.evaluate(() => {
    window.__navDispatch = {
      pointer(type, init) {
        document.getElementById('c').dispatchEvent(
          new PointerEvent(type, Object.assign({ bubbles: true, pointerId: 7 }, init)));
      },
      wheel(init) {
        document.getElementById('c').dispatchEvent(
          new WheelEvent('wheel', Object.assign({ bubbles: true, cancelable: true }, init)));
      },
    };
  });
  const live = () => page.evaluate(() => window.__bakedSplatViewer.state());
  // the eye the user SEES: fly renders from cam, orbit derives the eye
  // from target/radius/yaw/pitch (the cam field is stale during orbit)
  const eye = (st) => {
    if (st.fly) return st.cam;
    const cp = Math.cos(st.pitch);
    return [st.target[0] + st.radius * cp * Math.cos(st.yaw),
            st.target[1] + st.radius * Math.sin(st.pitch),
            st.target[2] + st.radius * cp * Math.sin(st.yaw)];
  };
  const st0 = await live();
  expect(st0.fly).toBe(false);
  expect(st0.target).toEqual([0, 0, 0]);

  // PAN: shift-drag moves the orbit target, not the angles
  const stPan = await page.evaluate(() => {
    window.__navDispatch.pointer('pointerdown', { clientX: 200, clientY: 200, shiftKey: true, button: 0 });
    window.__navDispatch.pointer('pointermove', { clientX: 260, clientY: 200, shiftKey: true });
    window.__navDispatch.pointer('pointerup', {});
    return window.__bakedSplatViewer.state();
  });
  expect(Math.hypot(
    stPan.target[0] - st0.target[0],
    stPan.target[1] - st0.target[1],
    stPan.target[2] - st0.target[2],
  )).toBeGreaterThan(0.01);
  expect(stPan.yaw).toBeCloseTo(st0.yaw, 6);       // pan never orbits
  expect(stPan.radius).toBeCloseTo(st0.radius, 6);

  // ZOOM-TO-CURSOR: wheel-in at an off-center point pulls the target
  // toward the cursor while shrinking the radius
  const stZoom = await page.evaluate(() => {
    window.__navDispatch.wheel({ deltaY: -480, clientX: 40, clientY: 40 });
    return window.__bakedSplatViewer.state();
  });
  expect(stZoom.radius).toBeLessThan(stPan.radius);
  expect(Math.hypot(
    stZoom.target[0] - stPan.target[0],
    stZoom.target[1] - stPan.target[1],
    stZoom.target[2] - stPan.target[2],
  )).toBeGreaterThan(0.001);                        // cursor pull, not center zoom

  // "1" -> FLY: wasd moves the eye along the look direction; the toggle
  // itself never jumps the pose
  await page.keyboard.press('1');
  await page.waitForFunction(() => window.__bakedSplatViewer.state().fly === true, { timeout: 5000 });
  const stFly = await live();
  const eyeZoom = eye(stZoom);
  expect(Math.hypot(
    stFly.cam[0] - eyeZoom[0],
    stFly.cam[1] - eyeZoom[1],
    stFly.cam[2] - eyeZoom[2],
  )).toBeLessThan(1e-4);                            // no jump entering fly
  await page.keyboard.down('w');
  await page.waitForFunction((c0) => {
    const c = window.__bakedSplatViewer.state().cam;
    return Math.hypot(c[0] - c0[0], c[1] - c0[1], c[2] - c0[2]) > 0.01;
  }, stFly.cam, { timeout: 5000 });                 // W actually moved
  await page.keyboard.up('w');

  // QUAKE MOUSELOOK — wiring pin. Headless chromium cannot actually
  // engage pointer lock (requests reject or resolve with a null
  // pointerLockElement), so the Pointer Lock API is stubbed and the
  // assertions cover OUR side of the contract: a fly click requests the
  // lock without starting a drag, a locked pointermove consumes RELATIVE
  // movementX/Y with no button held, and lock release restores the hint.
  await page.evaluate(() => {
    window.__lockStub = { requests: 0, exits: 0, el: null };
    Object.defineProperty(document, 'pointerLockElement', {
      configurable: true, get: () => window.__lockStub.el,
    });
    Element.prototype.requestPointerLock = function () {
      window.__lockStub.requests += 1;
      window.__lockStub.el = this;
      document.dispatchEvent(new Event('pointerlockchange'));
      return Promise.resolve();
    };
    document.exitPointerLock = function () {
      window.__lockStub.exits += 1;
      window.__lockStub.el = null;
      document.dispatchEvent(new Event('pointerlockchange'));
    };
  });
  const lockState = await page.evaluate(() => {
    window.__navDispatch.pointer('pointerdown', { clientX: 400, clientY: 300, button: 0, pointerType: 'mouse' });
    return { requests: window.__lockStub.requests, locked: window.__bakedSplatViewer.state().locked,
             hint: document.getElementById('h').textContent };
  });
  expect(lockState.requests).toBe(1);
  expect(lockState.locked).toBe(true);
  expect(lockState.hint).toContain('esc: release mouse');
  const preLock = await live();
  const postLock = await page.evaluate(() => {
    const c = document.getElementById('c');
    const ev = new PointerEvent('pointermove', { bubbles: true, pointerId: 11 });
    Object.defineProperty(ev, 'movementX', { value: 80 });
    Object.defineProperty(ev, 'movementY', { value: 0 });
    c.dispatchEvent(ev);
    return window.__bakedSplatViewer.state();
  });
  expect(Math.abs(postLock.yaw - preLock.yaw)).toBeCloseTo(0.24, 3);  // buttonless look
  expect(Math.hypot(
    postLock.cam[0] - preLock.cam[0],
    postLock.cam[1] - preLock.cam[1],
    postLock.cam[2] - preLock.cam[2],
  )).toBeLessThan(1e-9);

  // flying, not walking: pitch sails past the old +-1.5 walker clamp
  // (and back — a full vertical loop is legal in fly)
  const lookY = (my) => page.evaluate((v) => {
    const ev = new PointerEvent('pointermove', { bubbles: true, pointerId: 11 });
    Object.defineProperty(ev, 'movementX', { value: 0 });
    Object.defineProperty(ev, 'movementY', { value: v });
    document.getElementById('c').dispatchEvent(ev);
    return window.__bakedSplatViewer.state();
  }, my);
  const stUp = await lookY(700);
  expect(stUp.pitch).toBeGreaterThan(1.5);                 // unclamped
  const stBack = await lookY(-700);
  expect(Math.abs(stBack.pitch - postLock.pitch)).toBeLessThan(1e-9);

  // E/Q ride the CAMERA triad: pitched down, E moves along screen-up
  // (r x f), not world-up — W/S forward, A/D right, E/Q up, one frame
  const stDown = await lookY(300);                          // pitch ~0.9
  await page.keyboard.down('e');
  await page.waitForFunction((c0) => {
    const c = window.__bakedSplatViewer.state().cam;
    return Math.hypot(c[0] - c0[0], c[1] - c0[1], c[2] - c0[2]) > 0.02;
  }, stDown.cam, { timeout: 5000 });
  await page.keyboard.up('e');
  const stE = await live();
  {
    const d = [stE.cam[0] - stDown.cam[0], stE.cam[1] - stDown.cam[1], stE.cam[2] - stDown.cam[2]];
    const dl = Math.hypot(d[0], d[1], d[2]);
    const sp = Math.sin(stDown.pitch), cp = Math.cos(stDown.pitch);
    const u = [-sp * Math.cos(stDown.yaw), cp, -sp * Math.sin(stDown.yaw)];
    const dot = (d[0] * u[0] + d[1] * u[1] + d[2] * u[2]) / dl;
    expect(dot).toBeGreaterThan(0.999);                     // along camera-up
    expect(Math.abs(stE.cam[1] - stDown.cam[1] - dl)).toBeGreaterThan(1e-3); // NOT world-up
  }
  await lookY(-300);                                        // restore pitch
  await page.evaluate(() => document.exitPointerLock());   // browser's esc path
  await page.waitForFunction(() => window.__bakedSplatViewer.state().locked === false, { timeout: 5000 });
  const hintFree = await page.evaluate(() => document.getElementById('h').textContent);
  expect(hintFree).toContain('click: capture mouse');

  // in fly EVERY drag is mouselook: shift-drag must turn the view (yaw),
  // never pan it — shift is the sprint key, and sprint+steer froze the
  // view direction when the orbit pan binding leaked into fly
  const preLook = await live();
  const postLook = await page.evaluate(() => {
    window.__navDispatch.pointer('pointerdown', { clientX: 400, clientY: 300, shiftKey: true, button: 0 });
    window.__navDispatch.pointer('pointermove', { clientX: 340, clientY: 300, shiftKey: true });
    window.__navDispatch.pointer('pointerup', {});
    return window.__bakedSplatViewer.state();
  });
  expect(Math.abs(postLook.yaw - preLook.yaw)).toBeGreaterThan(0.05);
  expect(Math.hypot(
    postLook.cam[0] - preLook.cam[0],
    postLook.cam[1] - preLook.cam[1],
    postLook.cam[2] - preLook.cam[2],
  )).toBeLessThan(1e-6);                            // turned, not translated

  const preExit = await live();
  await page.keyboard.press('1');
  await page.waitForFunction(() => window.__bakedSplatViewer.state().fly === false, { timeout: 5000 });
  const postExit = await live();
  const eyeOut = eye(postExit);
  expect(Math.hypot(
    eyeOut[0] - preExit.cam[0],
    eyeOut[1] - preExit.cam[1],
    eyeOut[2] - preExit.cam[2],
  )).toBeLessThan(1e-4);                            // no jump leaving fly

  // "2" -> CENTER: the target returns to the bake center; the eye stays
  const preCenter = await live();
  await page.keyboard.press('2');
  await page.waitForFunction(() => {
    const t = window.__bakedSplatViewer.state().target;
    return Math.hypot(t[0], t[1], t[2]) < 1e-6;
  }, { timeout: 5000 });
  const stCenter = await live();
  const eyePre = eye(preCenter), eyePost = eye(stCenter);
  expect(Math.hypot(
    eyePost[0] - eyePre[0],
    eyePost[1] - eyePre[1],
    eyePost[2] - eyePre[2],
  )).toBeLessThan(1e-4);                            // re-aim, not teleport

  // the hint line teaches the controls in both modes
  const hintOrbit = await page.evaluate(() => document.getElementById('h').textContent);
  expect(hintOrbit).toContain('pan');
  expect(hintOrbit).toContain('zoom to cursor');
  await page.keyboard.press('1');
  await page.waitForFunction(() => window.__bakedSplatViewer.state().fly === true, { timeout: 5000 });
  const hintFly = await page.evaluate(() => document.getElementById('h').textContent);
  expect(hintFly).toContain('wasd');
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
