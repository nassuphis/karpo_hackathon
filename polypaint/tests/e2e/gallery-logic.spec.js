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
      const ctx = { jobId: 'compute_a', artifactId: 'cA', imageKey: base.source_key, trustedOrigin: ORIGIN };
      return {
        good: M.validateDeepzoom(base, ctx) !== null,
        badArtifact: M.validateDeepzoom({ ...base, source_artifact_id: 'WRONG' }, ctx),
        badKey: M.validateDeepzoom({ ...base, source_key: 'renders/compute_a/color/cB/image.jpeg' }, ctx),
        forgedDzi: M.validateDeepzoom({ ...base, dzi_key: 'deepzoom/compute_a/dz_A/EVIL.dzi' }, ctx),
        typeMismatch: M.normalizeManifest({ ...doc, manifest_type: 'artifact_mosaic' }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN }).ok,
        wrongKind: M.normalizeManifest({ ...doc, artifact_kind: 'bilevel' }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN }).ok,
        editableAtSharePath: M.normalizeManifest({ ...doc, document_kind: 'editable' }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN }).ok,
        explicitLayout: M.normalizeManifest({ ...doc, layout: { mode: 'explicit' } }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN }).ok,
      };
    }, galleryDoc());
    expect(r.good).toBe(true);
    expect(r.badArtifact).toBeNull();
    expect(r.badKey).toBeNull();
    expect(r.forgedDzi).toBeNull();
    expect(r.typeMismatch).toBe(false);
    expect(r.wrongKind).toBe(false);
    expect(r.editableAtSharePath).toBe(false);   // editable doc at a share path rejected
    expect(r.explicitLayout).toBe(false);        // auto-only until explicit layout ships
  });

  test('carries validated scene settings; defaults bad/missing values', async ({ page }) => {
    const r = await withModules(page, (M, L, doc, ORIGIN) => {
      const norm = (s) => M.normalizeManifest(s ? { ...doc, settings: s } : { ...doc }, { pathKind: 'virtual_gallery', trustedOrigin: ORIGIN }).settings;
      return { good: norm({ sky: 'dark', wall_color: '#123ABC' }), bad: norm({ sky: 'weird', wall_color: 'nope' }), none: norm(null) };
    }, galleryDoc());
    expect(r.good).toEqual({ sky: 'dark', wall_color: '#123abc' });   // valid kept, lower-cased
    expect(r.bad).toEqual({ sky: 'stars', wall_color: '#ece4d6' });   // invalid -> defaults
    expect(r.none).toEqual({ sky: 'stars', wall_color: '#ece4d6' });  // absent -> defaults
  });

  test('enforces the manifest row cap', async ({ page }) => {
    const over = await withModules(page, (M) => {
      const rows = [];
      for (let i = 0; i < M.GALLERY_LIMITS.MAX_MANIFEST_TILES + 1; i++) {
        rows.push({ job_id: 'j', artifact_id: 'a', preview_key: 'renders/j/color/a/preview.jpg', image_key: 'renders/j/color/a/image.jpeg', preview_width: 512, preview_height: 512 });
      }
      const doc = { schema_version: 1, manifest_type: 'virtual_gallery', artifact_kind: 'color', pieces: rows };
      return M.normalizeManifest(doc, { pathKind: 'virtual_gallery', trustedOrigin: 'https://x' }).ok;
    });
    expect(over).toBe(false);
  });
});

test.describe('Gallery viewer layout (pure)', () => {
  function eightPieces() {
    return Array.from({ length: 8 }, (_, i) => ({
      job_id: 'j' + i, artifact_id: 'a' + i, preview_width: 512, preview_height: 512,
      function: 'f', degree: i, N: 100 + i, created_at: '2026-05-0' + (i % 3),
    }));
  }

  test('round-robin walls, vertical bounds, non-overlap, inward facing', async ({ page }) => {
    const r = await withModules(page, (M, L, pieces) => {
      const { room, placements } = L.computeRoom(pieces);
      const inBounds = placements.every((p) => {
        const top = p.center_y_m + p.height_m / 2, bot = p.center_y_m - p.height_m / 2;
        return bot >= L.ROOM.FLOOR_MARGIN_M && top <= L.ROOM.WALL_HEIGHT_M - L.ROOM.FLOOR_MARGIN_M;
      });
      // north holds piece indices 0 and 4; their along-wall gap must exceed both widths
      const north = placements.filter((p) => p.wall === 'north').sort((a, b) => a.center_offset_m - b.center_offset_m);
      const noOverlap = Math.abs(north[1].center_offset_m - north[0].center_offset_m) >= Math.max(north[0].width_m, north[1].width_m);
      const inward = placements.every((p) => {
        const w = L.wallToWorld(p, room);
        return (w.normal.x * -w.position.x + w.normal.z * -w.position.z) > 0;
      });
      return {
        walls: placements.map((p) => p.wall),
        inBounds, noOverlap, inward,
        roomSquareish: room.width_m > 0 && room.depth_m > 0,
      };
    }, eightPieces());
    expect(r.walls.slice(0, 4)).toEqual(['north', 'east', 'south', 'west']);
    expect(r.inBounds).toBe(true);
    expect(r.noOverlap).toBe(true);
    expect(r.inward).toBe(true);
    expect(r.roomSquareish).toBe(true);
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
