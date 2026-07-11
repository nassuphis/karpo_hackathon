// @ts-check
// Wave 3b (virtual-gallery.md §16.2): smoke coverage for the standalone viewer
// shell. The interactive 3D scene is verified manually; these tests gate the
// parts that CAN be checked headlessly — that the vendored module graph loads
// (import map -> three + PointerLockControls + the pure modules + app.js),
// that manifest-URL validation rejects bad links with a readable message, that
// a valid same-origin share is accepted, and that a WebGL-less browser gets a
// fallback rather than a broken canvas.
const { test, expect } = require('@playwright/test');

// Force software WebGL (SwiftShader) for THIS spec only, so the valid-share
// case exercises the real Three.js scene build in CI instead of the WebGL-less
// fallback. Confined to this file via test.use so the rest of the suite is
// unaffected.
test.use({ launchOptions: { args: ['--enable-unsafe-swiftshader', '--use-gl=angle', '--use-angle=swiftshader', '--ignore-gpu-blocklist'] } });

const GALLERY = 'http://localhost:8765/gallery.html';
const MANIFEST_URL = 'http://localhost:8765/renders/_shared_mosaic/gallery/smoke/manifest.json';

function validManifest() {
  return {
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color',
    layout: { mode: 'auto', seed: 1 },
    pieces: [
      { ordinal: 0, job_id: 'compute_a', artifact_id: 'cA', preview_key: 'renders/compute_a/color/cA/preview.jpg', image_key: 'renders/compute_a/color/cA/image.jpeg', preview_width: 512, preview_height: 512, function: 'demo_a', degree: 12, N: 2048, times: 1, created_at: '2026-05-01', deepzoom: null },
      { ordinal: 1, job_id: 'compute_b', artifact_id: 'cB', preview_key: 'renders/compute_b/color/cB/preview.jpg', image_key: 'renders/compute_b/color/cB/image.jpeg', preview_width: 512, preview_height: 384, function: 'demo_b', degree: 9, N: 1024, times: 1, created_at: '2026-05-02', deepzoom: null },
      // one structurally-invalid row -> should be skipped, not fatal
      { ordinal: 2, job_id: 'compute_c', artifact_id: 'cC', preview_key: 'renders/OTHER/color/cC/preview.jpg', image_key: 'renders/compute_c/color/cC/image.jpeg', preview_width: 512, preview_height: 512 },
    ],
  };
}

// Fulfil the manifest fetch and let preview image fetches 404 (the texture
// manager negative-caches them; placeholders remain — fine for a smoke test).
async function routeManifest(page, body) {
  await page.route(MANIFEST_URL, (route) =>
    route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify(body) }));
  await page.route('**/renders/**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
}

test('module graph loads and a missing manifest shows a readable message', async ({ page }) => {
  const errors = [];
  page.on('pageerror', (e) => errors.push(String(e)));
  await page.goto(GALLERY);
  // If the import map or any module failed, boot() never runs and this H1 is absent.
  await expect(page.locator('#message-box h1')).toHaveText('No gallery specified');
  expect(errors, 'no uncaught module/runtime errors').toEqual([]);
});

test('rejects a cross-origin manifest link', async ({ page }) => {
  await page.goto(GALLERY + '?manifest=' + encodeURIComponent('https://evil.example/renders/_shared_mosaic/gallery/x/manifest.json'));
  await expect(page.locator('#message-box h1')).toHaveText('That gallery link is not valid');
});

test('rejects a same-origin manifest at a non-gallery path', async ({ page }) => {
  await page.goto(GALLERY + '?manifest=' + encodeURIComponent('http://localhost:8765/renders/_shared_mosaic/color/a/b/c/manifest.json'));
  await expect(page.locator('#message-box h1')).toHaveText('That gallery link is not valid');
});

test('accepts a valid share: builds the scene or shows a WebGL fallback', async ({ page }) => {
  await routeManifest(page, validManifest());
  await page.goto(GALLERY + '?manifest=' + encodeURIComponent(MANIFEST_URL));

  // Wait until boot resolves to one of its two acceptable terminal states.
  await page.waitForFunction(() => {
    const built = !!window.__galleryViewer;
    const msg = document.getElementById('message');
    const shownWebGL = msg && msg.classList.contains('show') && /WebGL/i.test(msg.textContent || '');
    return built || shownWebGL;
  }, { timeout: 8000 });

  const state = await page.evaluate(() => ({
    built: !!window.__galleryViewer,
    hasCanvas: !!document.querySelector('#gl-root canvas'),
    msg: document.getElementById('message').classList.contains('show')
      ? (document.getElementById('message-box').textContent || '') : '',
    pieces: window.__galleryViewer ? window.__galleryViewer.pieces.length : 0,
    skipped: document.getElementById('debug').getAttribute('data-skipped'),
  }));

  if (state.built) {
    // The manifest was fetched, validated, and the room built.
    expect(state.hasCanvas).toBe(true);
    expect(state.pieces).toBe(2);        // two valid rows; the cross-job row skipped
    expect(state.skipped).toBe('1');
  } else {
    // WebGL genuinely unavailable in this browser: the fallback, not a crash.
    expect(state.msg).toMatch(/WebGL/i);
  }
});

test('guided Next keeps focus, enables Inspect, and does not accumulate pins', async ({ page }) => {
  await routeManifest(page, validManifest());
  await page.goto(GALLERY + '?manifest=' + encodeURIComponent(MANIFEST_URL));
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  // Advance guided navigation several times (unlocked) — focus must persist and
  // the focus pin must stay refcounted at 1, not grow per step.
  await page.click('#btn-next');
  await page.click('#btn-next');
  await page.click('#btn-next');
  await page.waitForTimeout(250);   // let animation frames run
  const st = await page.evaluate(() => {
    const v = window.__galleryViewer;
    return {
      guided: v._guidedIndex, focus: v._focusIndex,
      hud: document.getElementById('hud-title').textContent || '',
      inspectDisabled: document.getElementById('btn-inspect').disabled,
      pins: v.tm.stats().pinned,
    };
  });
  expect(st.focus).toBe(st.guided);        // focus is NOT reset to -1 while unlocked
  expect(st.focus).toBeGreaterThanOrEqual(0);
  expect(st.hud.length).toBeGreaterThan(0);
  expect(st.inspectDisabled).toBe(false);
  expect(st.pins).toBe(1);                 // exactly one focus pin, refcounted
});

test('Zoom opens OpenSeadragon on the piece DZI without forcing CORS', async ({ page }) => {
  const url = 'http://localhost:8765/renders/_shared_mosaic/gallery/dz/manifest.json';
  await page.route(url, (route) => route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify({
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color', layout: { mode: 'auto', seed: 1 },
    pieces: [{ ordinal: 0, job_id: 'compute_a', artifact_id: 'cA',
      preview_key: 'renders/compute_a/color/cA/preview.jpg', image_key: 'renders/compute_a/color/cA/image.jpeg',
      preview_width: 512, preview_height: 512, function: 'f', title: '',
      deepzoom: { export_id: 'dz_A', dzi_key: 'deepzoom/compute_a/dz_A/image.dzi', source_key: 'renders/compute_a/color/cA/image.jpeg', source_artifact_id: 'cA' } }],
  }) }));
  await page.route('**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
  await page.goto(GALLERY + '?manifest=' + encodeURIComponent(url));
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  // Capture the OpenSeadragon options the viewer passes at Zoom time.
  const opts = await page.evaluate(() => {
    let captured = null;
    window.OpenSeadragon = function (o) { captured = o; return { addHandler() {}, destroy() {}, viewport: {} }; };
    const v = window.__galleryViewer;
    v._inspecting = 0;
    v._openDeepZoom();
    return captured;
  });
  expect(opts).toBeTruthy();
  expect(opts.tileSources).toContain('/deepzoom/compute_a/dz_A/image.dzi');  // the piece's DZI
  expect(opts.crossOriginPolicy).toBeUndefined();   // must NOT force CORS — the bucket has none
});

test('builds a maze with settings applied and collision that clamps to bounds', async ({ page }) => {
  const url = 'http://localhost:8765/renders/_shared_mosaic/gallery/mz/manifest.json';
  const mk = (j, a) => ({ ordinal: 0, job_id: j, artifact_id: a, preview_key: `renders/${j}/color/${a}/preview.jpg`, image_key: `renders/${j}/color/${a}/image.jpeg`, preview_width: 512, preview_height: 512, function: 'f', title: '', deepzoom: null });
  const pieces = Array.from({ length: 8 }, (_, i) => mk('j' + i, 'a' + i)); pieces.forEach((p, i) => (p.ordinal = i));
  await page.route(url, (route) => route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify({
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color',
    layout: { mode: 'auto', seed: 2 }, settings: { sky: 'dark', wall_color: '#3366cc' }, pieces }) }));
  await page.route('**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
  await page.goto(GALLERY + '?manifest=' + encodeURIComponent(url));
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  const st = await page.evaluate(() => {
    const v = window.__galleryViewer, m = v.maze;
    v.camera.position.set(1e6, 1.65, 1e6); v._clampCamera();   // shove far — must clamp inside the maze
    return {
      wallHex: '#' + v._wallMat.color.getHexString(), sky: v.spec.settings.sky, skyBuilt: !!v._skyGroup,
      walls: m.wallSegments.length, placed: m.placedCount, artMeshes: v._artMeshes.length,
      inBounds: v.camera.position.x <= m.bounds.maxX && v.camera.position.z <= m.bounds.maxZ,
    };
  });
  expect(st.wallHex).toBe('#3366cc');   // wall colour from settings
  expect(st.sky).toBe('dark');
  expect(st.skyBuilt).toBe(false);      // sky:'dark' -> no constellations built
  expect(st.walls).toBeGreaterThan(0);  // maze walls exist
  expect(st.placed).toBe(8);
  expect(st.artMeshes).toBe(8);
  expect(st.inBounds).toBe(true);       // corridor collision clamps inside the maze
});

test('over the resident cap, only the top working set is queued (no thrash)', async ({ page }) => {
  const url = 'http://localhost:8765/renders/_shared_mosaic/gallery/big/manifest.json';
  const mk = (j, a) => ({ ordinal: 0, job_id: j, artifact_id: a, preview_key: `renders/${j}/color/${a}/preview.jpg`, image_key: `renders/${j}/color/${a}/image.jpeg`, preview_width: 512, preview_height: 512, function: 'f', title: '', deepzoom: null });
  const pieces = Array.from({ length: 60 }, (_, i) => mk('j' + i, 'a' + i)); pieces.forEach((p, i) => (p.ordinal = i));
  await page.route(url, (route) => route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify({
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color', layout: { mode: 'auto', seed: 1 }, pieces }) }));
  await page.route('**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
  await page.goto(GALLERY + '?manifest=' + encodeURIComponent(url));
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  const st = await page.evaluate(() => {
    const v = window.__galleryViewer;
    v._scheduleTextures();
    return { pieces: v.pieces.length, queued: v.tm.stats().queued };
  });
  expect(st.pieces).toBe(60);
  expect(st.queued).toBeLessThanOrEqual(48);   // desired capped to the resident budget — no eviction thrash
});
