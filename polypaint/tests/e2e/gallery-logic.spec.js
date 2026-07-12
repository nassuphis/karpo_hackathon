// @ts-check
// Wave 3a (virtual-gallery.md §16.1): pure-logic gates for the standalone
// viewer's manifest validation (gallery/manifest.js) and deterministic layout
// (gallery/layout.js). These modules have no DOM/WebGL dependency, so they are
// exercised directly as ES modules in the browser (the production load path).
// The WebGL scene shell is tested separately.
const { test, expect } = require('@playwright/test');

const ORIGIN = 'https://polypaint.s3.us-east-1.amazonaws.com';

// Import both modules once per page and run `body` with them, returning its
// result across the boundary. Pieces/URLs use a synthetic https origin so the
// pure validators run without any network fetch.
async function withModules(page, body, arg) {
  return page.evaluate(async ({ bodySrc, arg }) => {
    const M = await import('/gallery/manifest.js');
    const L = await import('/gallery/layout.js');
    // eslint-disable-next-line no-new-func
    const fn = new Function('M', 'L', 'arg', 'ORIGIN', `return (${bodySrc})(M, L, arg, ORIGIN);`);
    return fn(M, L, arg, 'https://polypaint.s3.us-east-1.amazonaws.com');
  }, { bodySrc: body.toString(), arg: arg ?? null });
}

function galleryDoc() {
  const dz = {
    export_id: 'dz_A', dzi_key: 'deepzoom/compute_a/dz_A/image.dzi',
    source_key: 'renders/compute_a/color/cA/image.jpeg', source_artifact_id: 'cA',
  };
  return {
    schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color',
    layout: { mode: 'auto', seed: 1 },
    pieces: [
      { ordinal: 1, job_id: 'compute_a', artifact_id: 'cA', preview_key: 'renders/compute_a/color/cA/preview.jpg', image_key: 'renders/compute_a/color/cA/image.jpeg', preview_width: 512, preview_height: 512, function: 'f', degree: 12, N: 2048, times: 1, created_at: '2026-05-01', title: 'Opening', deepzoom: dz },
      { ordinal: 0, job_id: 'compute_b', artifact_id: 'cB', preview_key: 'renders/compute_b/color/cB/preview.jpg', image_key: 'renders/compute_b/color/cB/image.jpeg', preview_width: 512, preview_height: 256, function: 'g', degree: 9, N: 1024, times: 1, created_at: '2026-05-02', deepzoom: null },
      { ordinal: 2, job_id: 'compute_x', artifact_id: 'cX', preview_key: 'renders/OTHERJOB/color/cX/preview.jpg', image_key: 'renders/compute_x/color/cX/image.jpeg', preview_width: 512, preview_height: 512 },
    ],
  };
}

test.beforeEach(async ({ page }) => {
  // Any same-origin page gives the modules a served origin to import from.
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
});

test.describe('Gallery viewer manifest validation (pure)', () => {
  test('accepts only the two exact same-origin share paths', async ({ page }) => {
    const r = await withModules(page, (M, L, a, ORIGIN) => ({
      gallery: M.parseTrustedManifestUrl(ORIGIN + '/renders/_shared_mosaic/gallery/share_1/manifest.json', { origin: ORIGIN }),
      mosaic: M.parseTrustedManifestUrl(ORIGIN + '/renders/_shared_mosaic/color/s2/manifest.json', { origin: ORIGIN }),
      extra: M.parseTrustedManifestUrl(ORIGIN + '/renders/_shared_mosaic/color/a/b/c/manifest.json', { origin: ORIGIN }).ok,
      crossOrigin: M.parseTrustedManifestUrl('https://evil.example/renders/_shared_mosaic/gallery/x/manifest.json', { origin: ORIGIN }).ok,
      query: M.parseTrustedManifestUrl(ORIGIN + '/renders/_shared_mosaic/gallery/x/manifest.json?a=1', { origin: ORIGIN }).ok,
      insecure: M.parseTrustedManifestUrl('http://polypaint.s3.us-east-1.amazonaws.com/renders/_shared_mosaic/gallery/x/manifest.json', { origin: 'http://polypaint.s3.us-east-1.amazonaws.com' }).ok,
    }));
    expect(r.gallery.ok).toBe(true);
    expect(r.gallery.pathKind).toBe('virtual_gallery');
    expect(r.gallery.shareId).toBe('share_1');
    expect(r.mosaic.ok).toBe(true);
    expect(r.mosaic.pathKind).toBe('artifact_mosaic');
    expect(r.extra).toBe(false);       // extra path segments rejected
    expect(r.crossOrigin).toBe(false); // cross-origin rejected
    expect(r.query).toBe(false);       // query string rejected
    expect(r.insecure).toBe(false);    // non-https rejected in production
  });

  test('normalizes a share, skips a cross-job row, and keeps ordinal order', async ({ page }) => {
    const r = await withModules(page, (M, L, doc, ORIGIN) => {
      const n = M.normalizeManifest(doc, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN });
      return {
        ok: n.ok, count: n.pieces.length, skipped: n.skipped.length,
        first: n.pieces[0] && { job: n.pieces[0].job_id, ordinal: n.pieces[0].ordinal, preview: n.pieces[0].preview_url, dz: n.pieces[0].deepzoom },
        second: n.pieces[1] && { job: n.pieces[1].job_id, ordinal: n.pieces[1].ordinal, title: n.pieces[1].title, dziUrl: n.pieces[1].deepzoom && n.pieces[1].deepzoom.dzi_url },
      };
    }, galleryDoc());
    expect(r.ok).toBe(true);
    expect(r.count).toBe(2);          // cross-job preview row dropped
    expect(r.skipped).toBe(1);
    expect(r.first.job).toBe('compute_b'); // ordinal 0 sorts first
    expect(r.first.ordinal).toBe(0);
    expect(r.first.preview).toBe(ORIGIN + '/renders/compute_b/color/cB/preview.jpg');
    expect(r.first.dz).toBeNull();     // null deepzoom stays zoomless
    expect(r.second.job).toBe('compute_a');
    expect(r.second.ordinal).toBe(1);  // dense reindex after skip
    expect(r.second.title).toBe('Opening');  // curator title carried through
    expect(r.second.dziUrl).toBe(ORIGIN + '/deepzoom/compute_a/dz_A/image.dzi');
  });

  test('re-validates deepzoom identity and rejects type/path disagreement', async ({ page }) => {
    const r = await withModules(page, (M, L, doc, ORIGIN) => {
      const base = { export_id: 'dz_A', dzi_key: 'deepzoom/compute_a/dz_A/image.dzi', source_key: 'renders/compute_a/color/cA/image.jpeg', source_artifact_id: 'cA' };
      const ctx = { dziJobId: 'compute_a', artifactId: 'cA', trustedOrigin: ORIGIN };
      return {
        good: M.validateDeepzoom(base, ctx) !== null,
        badArtifact: M.validateDeepzoom({ ...base, source_artifact_id: 'WRONG' }, ctx),
        // source_key is OPAQUE provenance now (never fetched) — any string passes
        opaqueSource: M.validateDeepzoom({ ...base, source_key: 'renders/legacy/root_key.tif' }, ctx) !== null,
        // the dzi key is owned by the EXPORT job — a different owner must not match
        crossOwner: M.validateDeepzoom(base, { ...ctx, dziJobId: 'someone_else' }),
        forgedDzi: M.validateDeepzoom({ ...base, dzi_key: 'deepzoom/compute_a/dz_A/EVIL.dzi' }, ctx),
        typeMismatch: M.normalizeManifest({ ...doc, manifest_type: 'artifact_mosaic' }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN }).ok,
        wrongKind: M.normalizeManifest({ ...doc, artifact_kind: 'bilevel' }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN }).ok,
        editableAtSharePath: M.normalizeManifest({ ...doc, document_kind: 'editable' }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN }).ok,
        explicitLayout: M.normalizeManifest({ ...doc, layout: { mode: 'explicit' } }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN }).ok,
      };
    }, galleryDoc());
    expect(r.good).toBe(true);
    expect(r.badArtifact).toBeNull();
    expect(r.opaqueSource).toBe(true);
    expect(r.crossOwner).toBeNull();
    expect(r.forgedDzi).toBeNull();
    expect(r.typeMismatch).toBe(false);
    expect(r.wrongKind).toBe(false);
    expect(r.editableAtSharePath).toBe(false);   // editable doc at a share path rejected
    expect(r.explicitLayout).toBe(false);        // auto-only until explicit layout ships
  });

  test('cross-job export pieces + image-key degrade normalize correctly', async ({ page }) => {
    const r = await withModules(page, (M, L, doc, ORIGIN) => {
      const piece = {
        ordinal: 0, job_id: 'rjobX', export_job_id: 'compute_x', family: 'color', artifact_id: 'cX',
        preview_key: 'deepzoom/compute_x/dz_X/image_files/7/0_0.jpeg',     // pyramid-tile preview under the OWNER
        image_key: 'renders/legacy_root.tif',                              // invalid shape -> degrade, not drop
        preview_width: 128, preview_height: 128, function: 'f', title: '',
        deepzoom: { export_id: 'dz_X', dzi_key: 'deepzoom/compute_x/dz_X/image.dzi',
                    source_key: 'renders/legacy_root.tif', source_artifact_id: 'cX' },
      };
      const n = M.normalizeManifest({ ...doc, pieces: [piece] }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN });
      const p = n.pieces[0];
      return { ok: n.ok, count: n.pieces.length, image: p && p.image_url, dzi: p && p.deepzoom && p.deepzoom.dzi_url };
    }, galleryDoc());
    expect(r.ok).toBe(true);
    expect(r.count).toBe(1);                     // piece KEPT despite the bad image key
    expect(r.image).toBeNull();                  // ...but the original is not linked
    expect(r.dzi).toBe(ORIGIN + '/deepzoom/compute_x/dz_X/image.dzi');   // owner-scoped DZI
  });

  test('carries validated scene settings; defaults bad/missing values', async ({ page }) => {
    const r = await withModules(page, (M, L, doc, ORIGIN) => {
      const norm = (s) => M.normalizeManifest(s ? { ...doc, settings: s } : { ...doc }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN }).settings;
      return { good: norm({ sky: 'dark', wall_color: '#123ABC', wall_coverage: 250, wall_self_tint: false, wall_edge_px: 40, wall_layout: 'exhibition' }), bad: norm({ sky: 'weird', wall_color: 'nope', wall_coverage: 'junk' }), none: norm(null) };
    }, galleryDoc());
    expect(r.good).toEqual({ sky: 'dark', wall_color: '#123abc', wall_coverage: 100, wall_self_tint: false, wall_edge_px: 12, wall_layout: 'exhibition' });  // valid kept + clamped; explicit false honored
    expect(r.bad).toEqual({ sky: 'stars', wall_color: '#ece4d6', wall_coverage: null, wall_self_tint: true, wall_edge_px: 1, wall_layout: 'maze' }); // junk -> defaults
    expect(r.none).toEqual({ sky: 'stars', wall_color: '#ece4d6', wall_coverage: null, wall_self_tint: true, wall_edge_px: 1, wall_layout: 'maze' });// absent -> defaults
  });

  test('enforces the manifest row cap', async ({ page }) => {
    const over = await withModules(page, (M) => {
      const rows = [];
      for (let i = 0; i < M.GALLERY_LIMITS.MAX_MANIFEST_TILES + 1; i++) {
        rows.push({ job_id: 'j', artifact_id: 'a', preview_key: 'renders/j/color/a/preview.jpg', image_key: 'renders/j/color/a/image.jpeg', preview_width: 512, preview_height: 512 });
      }
      const doc = { schema_version: 1, manifest_type: 'virtual_gallery', document_kind: 'share', artifact_kind: 'color', pieces: rows };
      const res = M.normalizeManifest(doc, { pathKind: 'virtual_gallery', trustedOrigin: 'https://x' });
      return res.ok === false && /row count/.test(res.error);   // rejected BY THE CAP, not earlier
    });
    expect(over).toBe(true);
  });
});

test.describe('Gallery viewer layout (pure)', () => {
  function eightPieces() {
    return Array.from({ length: 8 }, (_, i) => ({
      job_id: 'j' + i, artifact_id: 'a' + i, preview_width: 512, preview_height: 512,
      function: 'f', degree: i, N: 100 + i, created_at: '2026-05-0' + (i % 3),
    }));
  }

  test('maze is deterministic, fully connected, and places every piece uniquely', async ({ page }) => {
    const r = await withModules(page, (M, L, pieces) => {
      const a = L.computeMaze(pieces, { seed: 5 });
      const b = L.computeMaze(pieces, { seed: 5 });
      const c = L.computeMaze(pieces, { seed: 9 });
      // BFS through open walls must reach every cell (perfect maze).
      const idx = (rr, cc) => rr * a.cols + cc;
      const seen = new Set([0]); const q = [[0, 0]];
      while (q.length) {
        const [rr, cc] = q.pop(); const w = a.grid[idx(rr, cc)];
        for (const [k, dr, dc] of [['N', -1, 0], ['S', 1, 0], ['E', 0, 1], ['W', 0, -1]]) {
          if (!w[k]) { const nr = rr + dr, nc = cc + dc; if (nr >= 0 && nr < a.rows && nc >= 0 && nc < a.cols && !seen.has(idx(nr, nc))) { seen.add(idx(nr, nc)); q.push([nr, nc]); } }
        }
      }
      const posKeys = new Set(a.placements.map((p) => p.position.x.toFixed(2) + ',' + p.position.z.toFixed(2)));
      return {
        deterministic: JSON.stringify(a.wallSegments) === JSON.stringify(b.wallSegments),
        seedSensitive: JSON.stringify(a.wallSegments) !== JSON.stringify(c.wallSegments),
        connected: seen.size === a.cols * a.rows,
        placedAll: a.placedCount === pieces.length,
        uniquePos: posKeys.size === a.placements.length,
      };
    }, eightPieces());
    expect(r.deterministic).toBe(true);
    expect(r.seedSensitive).toBe(true);
    expect(r.connected).toBe(true);
    expect(r.placedAll).toBe(true);
    expect(r.uniquePos).toBe(true);
  });

  test('wall coverage sizes the maze (lower coverage -> bigger maze)', async ({ page }) => {
    const r = await withModules(page, (M, L, pieces) => {
      const dense = L.computeMaze(pieces, { seed: 3, coverage: 100 });
      const sparse = L.computeMaze(pieces, { seed: 3, coverage: 10 });
      return { dense: dense.cols, sparse: sparse.cols,
               densePlaced: dense.placedCount, sparsePlaced: sparse.placedCount };
    }, eightPieces());
    expect(r.sparse).toBeGreaterThan(r.dense);   // 10% coverage -> larger grid
    expect(r.densePlaced).toBe(8);               // every piece still placed
    expect(r.sparsePlaced).toBe(8);
  });

  test('serpentine and exhibition layouts: connected, ordered, all pieces placed', async ({ page }) => {
    const r = await withModules(page, (M, L, pieces) => {
      const bfs = (m) => { const seen = new Set([0]); const q = [[0, 0]];
        while (q.length) { const [rr, cc] = q.pop(); const w = m.grid[rr * m.cols + cc];
          for (const [k, dr, dc] of [['N', -1, 0], ['S', 1, 0], ['E', 0, 1], ['W', 0, -1]]) {
            if (!w[k]) { const nr = rr + dr, nc = cc + dc;
              if (nr >= 0 && nr < m.rows && nc >= 0 && nc < m.cols && !seen.has(nr * m.cols + nc)) { seen.add(nr * m.cols + nc); q.push([nr, nc]); } } } }
        return seen.size; };
      const serp = L.computeLayout(pieces, { mode: 'serpentine', coverage: 35 });
      const exhi = L.computeLayout(pieces, { mode: 'exhibition', coverage: 35 });
      const spir = L.computeLayout(pieces, { mode: 'spiral', coverage: 35 });
      const maze = L.computeLayout(pieces, { mode: 'maze', seed: 5, coverage: 35 });
      // curator ORDER preserved in the walk modes: placements indexed 0..n-1
      const ordered = serp.placements.every((p, i) => p.piece_index === i)
        && exhi.placements.every((p, i) => p.piece_index === i)
        && spir.placements.every((p, i) => p.piece_index === i);
      const sig = (m) => JSON.stringify(m.wallSegments);
      return {
        serpReach: bfs(serp) === serp.cols * serp.rows, serpPlaced: serp.placedCount,
        exhiReach: bfs(exhi) === exhi.cols * exhi.rows, exhiPlaced: exhi.placedCount,
        spirReach: bfs(spir) === spir.cols * spir.rows, spirPlaced: spir.placedCount,
        distinct: new Set([sig(serp), sig(exhi), sig(spir), sig(maze)]).size === 4,
        ordered,
      };
    }, eightPieces());
    expect(r.serpReach).toBe(true);      // one continuous corridor, fully reachable
    expect(r.exhiReach).toBe(true);      // aisles connect around the partitions
    expect(r.spirReach).toBe(true);      // the spiral walks outside -> center
    expect(r.serpPlaced).toBe(8);
    expect(r.exhiPlaced).toBe(8);
    expect(r.spirPlaced).toBe(8);
    expect(r.distinct).toBe(true);       // all four modes are genuinely different rooms
    expect(r.ordered).toBe(true);
  });

  test('swept collision blocks crossing a closed interior wall but passes open sides', async ({ page }) => {
    const r = await withModules(page, (M, L, pieces) => {
      const m = L.computeMaze(pieces, { seed: 5 });
      const CELL = m.cell, rad = L.MAZE.COLLISION_RADIUS_M;
      const findWall = (open) => {
        for (let rr = 0; rr < m.rows; rr++) for (let cc = 0; cc < m.cols - 1; cc++) {
          if (m.grid[rr * m.cols + cc].E === !open) return { rr, cc };
        }
        return null;
      };
      const closed = findWall(false), openc = findWall(true);
      let blocked = null, passed = null;
      if (closed) {
        const wallX = m.origin.x + (closed.cc + 1) * CELL, pz = m.origin.z + (closed.rr + 0.5) * CELL;
        const s = L.mazeClampMove(m, wallX - 0.05, pz, wallX + 0.1, pz, rad);   // step across a CLOSED wall
        blocked = s.x <= wallX - rad + 1e-6;
      }
      if (openc) {
        const wallX = m.origin.x + (openc.cc + 1) * CELL, pz = m.origin.z + (openc.rr + 0.5) * CELL;
        const s = L.mazeClampMove(m, wallX - 0.2, pz, wallX + 0.15, pz, rad);   // step across an OPEN side
        passed = s.x > wallX;
      }
      return { blocked, passed };
    }, eightPieces());
    expect(r.blocked).toBe(true);    // cannot tunnel through a closed wall on a fast step
    expect(r.passed).toBe(true);     // open corridors remain passable
  });

  test('selectAndSort is seed-stable, seed-sensitive, and caps after sorting', async ({ page }) => {
    const r = await withModules(page, (M, L, pieces) => {
      const j = (arr) => arr.map((p) => p.job_id).join(',');
      const a = j(L.selectAndSort(pieces, { sort: 'random', seed: 42 }));
      const b = j(L.selectAndSort(pieces.slice().reverse(), { sort: 'random', seed: 42 }));
      const c = j(L.selectAndSort(pieces, { sort: 'random', seed: 7 }));
      const capped = L.selectAndSort(pieces, { sort: 'date', size: 3 });
      // deterministic tie-break: identical sort keys fall back to (job_id, artifact_id)
      const tied = L.selectAndSort(
        [{ job_id: 'jB', artifact_id: 'a', created_at: 'X' }, { job_id: 'jA', artifact_id: 'a', created_at: 'X' }],
        { sort: 'date' },
      ).map((p) => p.job_id);
      return { stable: a === b, sensitive: a !== c, cappedLen: capped.length, tied };
    }, eightPieces());
    expect(r.stable).toBe(true);
    expect(r.sensitive).toBe(true);
    expect(r.cappedLen).toBe(3);
    expect(r.tied).toEqual(['jA', 'jB']);
  });
});
