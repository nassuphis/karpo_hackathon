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

test('rejects a malformed share id (traversal)', async ({ page }) => {
  await page.goto(GALLERY + '?share=' + encodeURIComponent('../evil'));
  await expect(page.locator('#message-box h1')).toHaveText('That gallery link is not valid');
});

test('rejects an overlong share id', async ({ page }) => {
  await page.goto(GALLERY + '?share=' + 'a'.repeat(80));
  await expect(page.locator('#message-box h1')).toHaveText('That gallery link is not valid');
});

test('accepts a valid share: builds the scene or shows a WebGL fallback', async ({ page }) => {
  await routeManifest(page, validManifest());
  await page.goto(GALLERY + '?share=smoke');

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
    // every piece hangs with a centered title placard beneath it
    const labels = await page.evaluate(() => ({
      count: window.__galleryViewer._labelMats.length,
      below: window.__galleryViewer.scene.children.some((g) =>
        g.children && g.children.length >= 3 && g.children[2].position.y < 0),
    }));
    expect(labels.count).toBe(2);
    expect(labels.below).toBe(true);
  } else {
    // WebGL genuinely unavailable in this browser: the fallback, not a crash.
    expect(state.msg).toMatch(/WebGL/i);
  }
});

test('guided Next keeps focus, enables Inspect, and does not accumulate pins', async ({ page }) => {
  await routeManifest(page, validManifest());
  await page.goto(GALLERY + '?share=smoke');
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
  await page.goto(GALLERY + '?share=dz');
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  // Capture the OpenSeadragon options the viewer passes at Zoom time. OSD init is
  // deferred one frame (so the container is sized), so await the rAF.
  const opts = await page.evaluate(() => new Promise((resolve) => {
    let captured = null;
    window.OpenSeadragon = function (o) { captured = o; return { addHandler() {}, destroy() {}, viewport: { goHome() {} } }; };
    const v = window.__galleryViewer;
    v._inspecting = 0;
    document.getElementById('overlay').classList.add('open');   // deferred init requires the overlay open
    v._openDeepZoom();
    requestAnimationFrame(() => requestAnimationFrame(() => resolve(captured)));
  }));
  expect(opts).toBeTruthy();
  expect(opts.tileSources).toContain('/deepzoom/compute_a/dz_A/image.dzi');  // the piece's DZI
  expect(opts.crossOriginPolicy).toBeUndefined();   // must NOT force CORS — the bucket has none
});

test('Tour walks the gallery continuously and stops on demand', async ({ page }) => {
  const url = 'http://localhost:8765/renders/_shared_mosaic/gallery/tour/manifest.json';
  const mk = (j, a) => ({ ordinal: 0, job_id: j, artifact_id: a, preview_key: `renders/${j}/color/${a}/preview.jpg`, image_key: null, preview_width: 512, preview_height: 512, function: 'f', title: '', deepzoom: null });
  const pieces = Array.from({ length: 6 }, (_, i) => mk('j' + i, 'a' + i)); pieces.forEach((p, i) => (p.ordinal = i));
  await page.route(url, (route) => route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify({
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color',
    layout: { mode: 'auto', seed: 4 }, settings: { sky: 'dark' }, pieces }) }));
  await page.route('**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
  await page.goto(GALLERY + '?share=tour');
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');

  const before = await page.evaluate(() => { const p = window.__galleryViewer.camera.position; return { x: p.x, z: p.z }; });
  await page.click('#btn-tour');
  await expect(page.locator('#btn-tour')).toHaveText('Stop tour');   // busy/active feedback ON the button
  await expect(page.locator('#btn-tour')).toHaveClass(/active/);
  // it WALKS: the camera moves over frames, and guided state tracks the tour
  await page.waitForFunction(({ x, z }) => {
    const p = window.__galleryViewer.camera.position;
    return Math.hypot(p.x - x, p.z - z) > 0.4;
  }, before, { timeout: 8000 });
  const st = await page.evaluate(() => ({
    guided: window.__galleryViewer._guidedIndex,
    hud: document.getElementById('hud-title').textContent || '',
  }));
  expect(st.guided).toBeGreaterThanOrEqual(0);   // Next/Prev/Inspect continue from the tour
  expect(st.hud.length).toBeGreaterThan(0);      // walking toward a titled piece

  await page.click('#btn-tour');                 // toggle OFF
  await expect(page.locator('#btn-tour')).toHaveText('Tour');
  expect(await page.evaluate(() => window.__galleryViewer._tour)).toBeNull();

  // manual navigation also stops a running tour
  await page.click('#btn-tour');
  await expect(page.locator('#btn-tour')).toHaveText('Stop tour');
  await page.click('#btn-next');
  await expect(page.locator('#btn-tour')).toHaveText('Tour');
  expect(await page.evaluate(() => window.__galleryViewer._tour)).toBeNull();
});

test('a photographic sky becomes the scene background texture', async ({ page }) => {
  const url = 'http://localhost:8765/renders/_shared_mosaic/gallery/sky/manifest.json';
  const mk = (j, a) => ({ ordinal: 0, job_id: j, artifact_id: a, preview_key: `renders/${j}/color/${a}/preview.jpg`, image_key: `renders/${j}/color/${a}/image.jpeg`, preview_width: 512, preview_height: 512, function: 'f', title: '', deepzoom: null });
  const pieces = [mk('j0', 'a0'), mk('j1', 'a1')]; pieces.forEach((p, i) => (p.ordinal = i));
  await page.route(url, (route) => route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify({
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color',
    layout: { mode: 'auto', seed: 1 }, settings: { sky: 'galaxies' }, pieces }) }));
  await page.route('**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
  await page.goto(GALLERY + '?share=sky');
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  // The equirect skybox JPEG (served from the repo's own skybox/ dir) loads
  // async — poll until the background is the texture.
  await page.waitForFunction(() => {
    const v = window.__galleryViewer;
    return v && v.scene.background && v.scene.background.isTexture === true;
  }, { timeout: 8000 });
  const st = await page.evaluate(() => {
    const v = window.__galleryViewer;
    return {
      mapping: v.scene.background.mapping,
      equirect: v.scene.background.mapping === 303,   // THREE.EquirectangularReflectionMapping
      skyGroup: !!v._skyGroup,                        // procedural stars must be OFF
    };
  });
  expect(st.equirect).toBe(true);
  expect(st.skyGroup).toBe(false);
});

test('builds a maze with settings applied and collision that clamps to bounds', async ({ page }) => {
  const url = 'http://localhost:8765/renders/_shared_mosaic/gallery/mz/manifest.json';
  const mk = (j, a) => ({ ordinal: 0, job_id: j, artifact_id: a, preview_key: `renders/${j}/color/${a}/preview.jpg`, image_key: `renders/${j}/color/${a}/image.jpeg`, preview_width: 512, preview_height: 512, function: 'f', title: '', deepzoom: null });
  const pieces = Array.from({ length: 8 }, (_, i) => mk('j' + i, 'a' + i)); pieces.forEach((p, i) => (p.ordinal = i));
  await page.route(url, (route) => route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify({
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color',
    layout: { mode: 'auto', seed: 2 }, settings: { sky: 'dark', wall_color: '#3366cc' }, pieces }) }));
  await page.route('**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
  await page.goto(GALLERY + '?share=mz');
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
  const edges = await page.evaluate(() => {
    const v = window.__galleryViewer;
    let n = 0;
    v.scene.traverse((o) => { if (o.userData && o.userData.wallEdge) n++; });
    // fat lines: ONE merged LineSegments2 with 12 edges per wall box
    const segs = v._wallEdgeGeo ? v._wallEdgeGeo.attributes.instanceStart.count : 0;
    return { n, walls: v.maze.wallSegments.length, segs, width: v._wallEdgeMat && v._wallEdgeMat.linewidth };
  });
  expect(edges.n).toBe(1);                        // single merged edge object
  expect(edges.segs).toBe(edges.walls * 12);      // every box edge represented
  expect(edges.width).toBe(1);                    // default thickness honored
});

test('over the resident cap, only the top working set is queued (no thrash)', async ({ page }) => {
  const url = 'http://localhost:8765/renders/_shared_mosaic/gallery/big/manifest.json';
  const mk = (j, a) => ({ ordinal: 0, job_id: j, artifact_id: a, preview_key: `renders/${j}/color/${a}/preview.jpg`, image_key: `renders/${j}/color/${a}/image.jpeg`, preview_width: 512, preview_height: 512, function: 'f', title: '', deepzoom: null });
  const pieces = Array.from({ length: 60 }, (_, i) => mk('j' + i, 'a' + i)); pieces.forEach((p, i) => (p.ordinal = i));
  await page.route(url, (route) => route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify({
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color', layout: { mode: 'auto', seed: 1 }, pieces }) }));
  await page.route('**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
  await page.goto(GALLERY + '?share=big');
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

test('scene has moonlight, a moon disc, and a distinct textured floor', async ({ page }) => {
  await routeManifest(page, validManifest());
  await page.goto(GALLERY + '?share=smoke');
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  const st = await page.evaluate(() => {
    const v = window.__galleryViewer; let dir = 0, moon = false, floorMap = false;
    v.scene.traverse((o) => {
      if (o.isDirectionalLight) dir++;
      if (o.geometry && o.geometry.type === 'SphereGeometry' && o.position.y > 50) moon = true;   // moon high in the sky
      if (o.material && o.material.map && o.geometry && o.geometry.type === 'PlaneGeometry' && o.rotation.x < 0) floorMap = true;
    });
    return { dir, moon, floorMap };
  });
  expect(st.dir).toBeGreaterThanOrEqual(1);   // a directional (moon) light
  expect(st.moon).toBe(true);                 // the moon disc
  expect(st.floorMap).toBe(true);             // floor has a distinct grid texture
});

test('a serpentine share builds a serpentine room in the viewer', async ({ page }) => {
  const url = 'http://localhost:8765/renders/_shared_mosaic/gallery/serp/manifest.json';
  const mk = (j, a) => ({ ordinal: 0, job_id: j, artifact_id: a, preview_key: `renders/${j}/color/${a}/preview.jpg`, image_key: `renders/${j}/color/${a}/image.jpeg`, preview_width: 512, preview_height: 512, function: 'f', title: '', deepzoom: null });
  const pieces = Array.from({ length: 10 }, (_, i) => mk('j' + i, 'a' + i)); pieces.forEach((p, i) => (p.ordinal = i));
  await page.route(url, (route) => route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify({
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color',
    layout: { mode: 'auto', seed: 1 }, settings: { wall_layout: 'serpentine', wall_coverage: 35 }, pieces }) }));
  await page.route('**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
  await page.goto(GALLERY + '?share=serp');
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  const st = await page.evaluate(() => {
    const v = window.__galleryViewer, m = v.maze;
    return { placed: m.placedCount, meshes: v._artMeshes.length, nonSquareGrid: m.cols !== m.rows,
      layout: v.spec.settings.wall_layout };
  });
  expect(st.layout).toBe('serpentine');
  expect(st.placed).toBe(10);
  expect(st.meshes).toBe(10);
});

test('minimap renders the maze with a red you-are-here dot; textures are pre-flipped', async ({ page }) => {
  await routeManifest(page, validManifest());
  await page.goto(GALLERY + '?share=smoke');
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  const st = await page.evaluate(() => {
    const v = window.__galleryViewer;
    v._updateMinimap();
    const cvs = document.getElementById('minimap'); const g = cvs.getContext('2d');
    const d = g.getImageData(0, 0, cvs.width, cvs.height).data;
    let painted = 0, red = 0;
    for (let i = 0; i < d.length; i += 4) { if (d[i + 3] > 0) painted++; if (d[i] > 180 && d[i + 1] < 90 && d[i + 2] < 90 && d[i + 3] > 0) red++; }
    const tex = v.tm._makeTexture({ width: 4, height: 4, close() {} });   // exercise makeTexture
    return { painted, red, flipY: tex.flipY };
  });
  expect(st.painted).toBeGreaterThan(50);   // the maze is drawn on the minimap
  expect(st.red).toBeGreaterThan(0);        // the red you-are-here dot
  expect(st.flipY).toBe(false);             // textures pre-flipped -> art is right-side up
});

test('inspect goes STRAIGHT to zoom; overlay has only Copy link + Close', async ({ page }) => {
  const url = 'http://localhost:8765/renders/_shared_mosaic/gallery/dz/manifest.json';
  await page.route(url, (route) => route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify({
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color', layout: { mode: 'auto', seed: 1 },
    pieces: [{ ordinal: 0, job_id: 'compute_a', artifact_id: 'cA',
      preview_key: 'renders/compute_a/color/cA/preview.jpg', image_key: 'renders/compute_a/color/cA/image.jpeg',
      preview_width: 512, preview_height: 512, function: 'f', title: '',
      deepzoom: { export_id: 'dz_A', dzi_key: 'deepzoom/compute_a/dz_A/image.dzi', source_key: 'renders/compute_a/color/cA/image.jpeg', source_artifact_id: 'cA' } }],
  }) }));
  await page.route('**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
  await page.goto(GALLERY + '?share=dz');
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  await page.evaluate(() => { const v = window.__galleryViewer; v._guidedIndex = 0; v._inspectFocused(); });
  await expect(page.locator('#overlay')).toBeVisible();
  await expect(page.locator('#osd')).toBeVisible();               // zoom surface immediately
  await expect(page.locator('#overlay-img')).toBeHidden();        // no intermediate preview step
  await expect(page.locator('#overlay-bar button')).toHaveCount(2);   // Copy link + Close only
  await expect(page.locator('#overlay-copy')).toHaveText('Copy link');
  await expect(page.locator('#overlay-meta')).toContainText('image 1');  // default title
});

test('REGRESSION: inline zoom renders real DZI pixels with the vendored OpenSeadragon', async ({ page }) => {
  // The "empty zoom" bug: OSD rewrites its element's position to "relative", and
  // an inset-sized #osd collapsed to 0 height -> a 1px-tall canvas -> nothing
  // rendered. This drives the REAL vendored OSD over a real committed DZI
  // pyramid and asserts actual pixels land on the canvas.
  const fs = require('fs');
  const path = require('path');
  const FIXTURE = path.join(__dirname, '..', 'fixtures', 'dzT');
  const url = 'http://localhost:8765/renders/_shared_mosaic/gallery/dzreal/manifest.json';
  await page.route(url, (route) => route.fulfill({ status: 200, contentType: 'application/json', body: JSON.stringify({
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color', layout: { mode: 'auto', seed: 1 },
    pieces: [{ ordinal: 0, job_id: 'jobT', artifact_id: 'artT',
      preview_key: 'renders/jobT/color/artT/preview.jpg', image_key: 'renders/jobT/color/artT/image.jpeg',
      preview_width: 512, preview_height: 512, function: 't', title: 'T',
      deepzoom: { export_id: 'dzT', dzi_key: 'deepzoom/jobT/dzT/image.dzi', source_key: 'renders/jobT/color/artT/image.jpeg', source_artifact_id: 'artT' } }],
  }) }));
  await page.route('**/deepzoom/jobT/dzT/**', (route) => {
    const rel = new URL(route.request().url()).pathname.split('/deepzoom/jobT/dzT/')[1];
    const file = path.join(FIXTURE, rel);
    if (!fs.existsSync(file)) return route.fulfill({ status: 404, body: '' });
    route.fulfill({ status: 200, contentType: rel.endsWith('.dzi') ? 'application/xml' : 'image/jpeg', body: fs.readFileSync(file) });
  });
  await page.route('**/preview.jpg', (route) => route.fulfill({ status: 404, body: '' }));
  await page.goto(GALLERY + '?share=dzreal');
  const built = await page.waitForFunction(() => !!window.__galleryViewer, { timeout: 8000 }).then(() => true).catch(() => false);
  test.skip(!built, 'WebGL scene not built in this browser');
  await page.evaluate(() => {
    const v = window.__galleryViewer;
    v._inspecting = 0;
    document.getElementById('overlay').classList.add('open');
    v._openDeepZoom();
  });
  // Wait until the OSD canvas exists, is properly sized, and has real pixels.
  // Sample the CENTER: goHome fits + centers the image, so the corners are
  // legitimately empty margin.
  await page.waitForFunction(() => {
    const c = document.querySelector('#osd canvas');
    if (!c || c.height < 100) return false;
    const g = c.getContext('2d');
    const s = 200;
    const d = g.getImageData(Math.max(0, c.width / 2 - s / 2), Math.max(0, c.height / 2 - s / 2), s, s).data;
    let nonBlack = 0;
    for (let i = 0; i < d.length; i += 4) if (d[i] + d[i + 1] + d[i + 2] > 30) nonBlack++;
    return nonBlack > 1000;
  }, { timeout: 8000 });
  const st = await page.evaluate(() => ({
    osdH: document.getElementById('osd').clientHeight,
    canvasH: document.querySelector('#osd canvas').height,
    status: document.getElementById('overlay-status').textContent,
  }));
  expect(st.osdH).toBeGreaterThan(100);      // #osd keeps its size after OSD rewrites position
  expect(st.canvasH).toBeGreaterThan(100);   // not the 1px-tall collapsed canvas
  expect(st.status).toBe('');                // no zoom error surfaced
});
