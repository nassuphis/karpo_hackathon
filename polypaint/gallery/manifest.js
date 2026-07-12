// Virtual Gallery — manifest validation and normalization (pure, no DOM/WebGL).
//
// This is the trust boundary for the standalone viewer (virtual-gallery.md
// §4/§4.1/§13.1). It turns an untrusted manifest URL + document into a
// normalized `GallerySceneSpec` (a flat, ordered piece list) or a precise
// error. It performs NO I/O: the streaming byte-cap fetch lives in the shell
// (gallery/app.js); everything here is synchronous and unit-testable.
//
// Everything the manifest declares is untrusted. Keys are validated to belong
// to the row's own declared (job_id, artifact_id, color) identity; asset URLs
// are rebuilt against a trusted origin (manifest.base is ignored); display
// metadata is passed through verbatim for the caller to render with
// textContent (never innerHTML).

export const GALLERY_LIMITS = Object.freeze({
  MAX_MANIFEST_TILES: 20_000,   // input rows accepted before the display cap
  MANIFEST_MAX_BYTES: 8 * 1024 * 1024,
  SCHEMA_VERSION: 1,
  ID_MAX: 64,
  LEAF_MAX: 96,
});

const ID_RE = /^[A-Za-z0-9_-]{1,64}$/;
const LEAF_RE = /^[A-Za-z0-9._-]{1,96}$/;
// Preview keys must be actual preview files — a crafted manifest pointing
// preview_key at the full-size image would make the texture loader decode a
// potentially multi-GB original in a load/evict loop.
const PREVIEW_LEAF_RE = /^preview[A-Za-z0-9._-]{0,90}\.(png|jpe?g)$/i;
const HEX_COLOR_RE = /^#[0-9a-fA-F]{6}$/;
const SKY_MODES = new Set(['stars', 'dark']);
const LAYOUT_MODES = new Set(['maze', 'serpentine', 'exhibition', 'spiral']);
// wall_coverage: null = "not specified" -> the viewer uses the LEGACY maze
// sizing, so shares written before the knob existed keep their layout.
const DEFAULT_SETTINGS = Object.freeze({ sky: 'stars', wall_color: '#ece4d6', wall_coverage: null, wall_self_tint: true, wall_edge_px: 1, wall_layout: 'maze' });

// Scene settings the viewer applies (untrusted → validated). Only two knobs:
// sky mode and wall colour.
export function validateGallerySettings(raw) {
  const s = (raw && typeof raw === 'object') ? raw : {};
  return {
    sky: SKY_MODES.has(s.sky) ? s.sky : DEFAULT_SETTINGS.sky,
    wall_color: (typeof s.wall_color === 'string' && HEX_COLOR_RE.test(s.wall_color))
      ? s.wall_color.toLowerCase() : DEFAULT_SETTINGS.wall_color,
    wall_coverage: Number.isFinite(s.wall_coverage)
      ? Math.max(5, Math.min(100, Math.round(s.wall_coverage))) : null,
    wall_self_tint: s.wall_self_tint !== false,   // default ON; only explicit false disables
    wall_edge_px: Number.isFinite(s.wall_edge_px)
      ? Math.max(0, Math.min(12, Math.round(s.wall_edge_px))) : 1,
    wall_layout: LAYOUT_MODES.has(s.wall_layout) ? s.wall_layout : 'maze',
  };
}

// The two accepted document types, each pinned to an exact same-origin path
// shape with a single {share_id} segment. The PATH decides which type is
// expected; the document's manifest_type must then agree (§4.1 step 4).
const PATH_KINDS = [
  { kind: 'virtual_gallery', re: /^\/renders\/_shared_mosaic\/gallery\/([A-Za-z0-9_-]{1,64})\/manifest\.json$/ },
  { kind: 'artifact_mosaic', re: /^\/renders\/_shared_mosaic\/color\/([A-Za-z0-9_-]{1,64})\/manifest\.json$/ },
];

export function isValidId(value) {
  return typeof value === 'string' && ID_RE.test(value);
}

function isFinitePositive(value) {
  return typeof value === 'number' && Number.isFinite(value) && value > 0;
}

// A color-artifact object key must be EXACTLY renders/<job>/color/<artifact>/<leaf>
// built from the row's own declared ids — this is what rejects a row that
// declares job A but points image_key at job B (§4.1 step 10). Returns the leaf
// on success, null on any violation.
function validateColorKey(key, jobId, artifactId) {
  if (typeof key !== 'string' || !key) return null;
  if (key.includes('..') || key.includes('\\') || key.includes('{') ||
      key.includes('}') || key.includes('?') || key.includes('#') ||
      key.includes('//')) {
    return null;
  }
  const parts = key.split('/');
  if (parts.length !== 5) return null;
  if (parts[0] !== 'renders' || parts[1] !== jobId ||
      parts[2] !== 'color' || parts[3] !== artifactId) {
    return null;
  }
  const leaf = parts[4];
  if (!LEAF_RE.test(leaf)) return null;
  return leaf;
}

// Any-family render key: renders/<job>/<family>/<artifact>/<leaf> (the original
// may be color/bilevel/greyscale — "link to the original if linkable").
function validateRenderKey(key, jobId, artifactId) {
  if (typeof key !== 'string' || !key) return null;
  if (key.includes('..') || key.includes('\\') || key.includes('{') ||
      key.includes('}') || key.includes('?') || key.includes('#') || key.includes('//')) return null;
  const parts = key.split('/');
  if (parts.length !== 5) return null;
  if (parts[0] !== 'renders' || parts[1] !== jobId || !/^[a-z]{1,24}$/.test(parts[2]) ||
      parts[3] !== artifactId) return null;
  return LEAF_RE.test(parts[4]) ? parts[4] : null;
}

// A DZI pyramid tile used as the piece preview when the piece was built from
// the export itself: deepzoom/<job>/<export>/image_files/<level>/0_0.<ext>.
function validExportPreviewKey(key, jobId, exportId) {
  if (typeof key !== 'string' || !exportId) return false;
  const re = new RegExp('^deepzoom/' + jobId + '/' + exportId + '/image_files/\\d{1,3}/0_0\\.(jpeg|jpg|png)$');
  return re.test(key);
}

// Re-validate a piece's `deepzoom` on load — the viewer must not trust the
// manifest blindly (§13.1). Returns a normalized zoom descriptor or null
// (zoomless). The dzi_key is reconstructed from validated ids, never read as a
// free string, and any absolute URL in the document is ignored.
export function validateDeepzoom(dz, { dziJobId, artifactId, trustedOrigin }) {
  if (!dz || typeof dz !== 'object') return null;
  const exportId = dz.export_id;
  if (!isValidId(exportId)) return null;
  if (dz.source_artifact_id != null && dz.source_artifact_id !== artifactId) return null;
  // The DZI lives under the export OWNER's job (which can differ from the
  // piece's render job); the key is rebuilt from validated ids, never read free.
  const expectedDziKey = `deepzoom/${dziJobId}/${exportId}/image.dzi`;
  if (dz.dzi_key !== expectedDziKey) return null;
  return {
    export_id: exportId,
    dzi_key: expectedDziKey,
    dzi_url: originAbsolute(trustedOrigin, expectedDziKey),
    // Opaque provenance: never used to build a URL, so any string/null is safe.
    source_key: typeof dz.source_key === 'string' ? dz.source_key : null,
    source_artifact_id: artifactId,
  };
}

function originAbsolute(trustedOrigin, key) {
  // key is already validated (no leading slash, no traversal). Resolve against
  // the trusted origin only — never against manifest.base.
  return trustedOrigin.replace(/\/+$/, '') + '/' + key;
}

// Validate the manifest URL the viewer was launched with. `origin` is the
// viewer's own location.origin (production requires same-origin HTTPS); pass
// { requireHttps:false } for local http dev. Returns
// { ok, url, pathKind, shareId } or { ok:false, error }.
export function parseTrustedManifestUrl(raw, { origin, requireHttps = true } = {}) {
  let url;
  try {
    url = new URL(String(raw), origin);
  } catch {
    return { ok: false, error: 'manifest url is not parseable' };
  }
  if (requireHttps && url.protocol !== 'https:') {
    return { ok: false, error: 'manifest url must be https' };
  }
  if (origin && url.origin !== new URL(origin).origin) {
    return { ok: false, error: 'manifest url must be same-origin as the viewer' };
  }
  if (url.search || url.hash) {
    return { ok: false, error: 'manifest url must not carry a query or fragment' };
  }
  for (const { kind, re } of PATH_KINDS) {
    const m = re.exec(url.pathname);
    if (m) return { ok: true, url, pathKind: kind, shareId: m[1] };
  }
  return { ok: false, error: 'manifest url is not an accepted gallery/mosaic path' };
}

// Normalize an already-fetched, size-checked manifest document into a
// GallerySceneSpec. `pathKind` comes from parseTrustedManifestUrl (the path and
// the document type must agree). A single malformed row is skipped and counted;
// only a structurally invalid document fails the whole scene.
export function normalizeManifest(doc, { pathKind, trustedOrigin }) {
  if (!doc || typeof doc !== 'object') {
    return { ok: false, error: 'manifest is not an object' };
  }
  if (doc.schema_version !== GALLERY_LIMITS.SCHEMA_VERSION) {
    return { ok: false, error: 'unsupported schema_version' };
  }
  if (doc.manifest_type !== pathKind) {
    return { ok: false, error: `manifest_type ${doc.manifest_type} does not match path type ${pathKind}` };
  }
  if (doc.artifact_kind !== 'color') {
    return { ok: false, error: 'only color artifacts are supported' };
  }
  // The viewer loads immutable SHARES with auto layout. Reject an editable
  // document copied under a share path, and an explicit-layout share the auto
  // placement would silently rearrange (§13). Mosaic tiles carry neither field.
  if (pathKind === 'virtual_gallery') {
    if (doc.document_kind !== 'share') {
      return { ok: false, error: `unsupported document_kind: ${doc.document_kind}` };
    }
    const mode = doc.layout && doc.layout.mode;
    if (mode != null && mode !== 'auto') {
      return { ok: false, error: `unsupported layout mode: ${mode}` };
    }
  }
  const rawRows = pathKind === 'virtual_gallery' ? doc.pieces : doc.tiles;
  if (!Array.isArray(rawRows)) {
    return { ok: false, error: 'manifest has no piece/tile array' };
  }
  if (rawRows.length > GALLERY_LIMITS.MAX_MANIFEST_TILES) {
    return { ok: false, error: 'manifest exceeds the maximum row count' };
  }

  const pieces = [];
  const skipped = [];
  for (let i = 0; i < rawRows.length; i++) {
    const piece = pathKind === 'virtual_gallery'
      ? normalizeGalleryPiece(rawRows[i], i, trustedOrigin)
      : normalizeMosaicTile(rawRows[i], i, trustedOrigin);
    if (piece.ok) pieces.push(piece.value);
    else skipped.push({ index: i, reason: piece.error });
  }

  // Order authority (§4/§13.1): a virtual_gallery share is fixed by curator
  // ordinal; a mosaic has no stored order (the caller may sort it).
  if (pathKind === 'virtual_gallery') {
    pieces.sort((a, b) => (a.ordinal - b.ordinal) ||
      (a.job_id < b.job_id ? -1 : a.job_id > b.job_id ? 1 : 0) ||
      (a.artifact_id < b.artifact_id ? -1 : a.artifact_id > b.artifact_id ? 1 : 0));
  }
  // Re-assign dense ordinals so a partially-skipped share stays 0..n-1.
  pieces.forEach((p, idx) => { p.ordinal = idx; });

  return {
    ok: true, kind: pathKind, artifactKind: 'color', pieces, skipped,
    settings: validateGallerySettings(doc.settings),
    layout: (doc.layout && typeof doc.layout === 'object') ? { seed: Number.isFinite(doc.layout.seed) ? doc.layout.seed : 1 } : { seed: 1 },
  };
}

function normalizeGalleryPiece(row, index, trustedOrigin) {
  if (!row || typeof row !== 'object') return { ok: false, error: 'row is not an object' };
  const jobId = row.job_id;
  const artifactId = row.artifact_id;
  if (!isValidId(jobId) || !isValidId(artifactId)) return { ok: false, error: 'bad job/artifact id' };
  // The export OWNER's job can differ from the render job (cross-job exports);
  // it scopes the DZI + pyramid-tile preview keys.
  const exportJobId = isValidId(row.export_job_id) ? row.export_job_id : jobId;
  const family = (typeof row.family === 'string' && /^[a-z]{1,24}$/.test(row.family)) ? row.family : '';
  const dzExportId = row.deepzoom && typeof row.deepzoom === 'object' && isValidId(row.deepzoom.export_id)
    ? row.deepzoom.export_id : null;
  const previewLeaf = validateColorKey(row.preview_key, jobId, artifactId);
  const previewOk = (previewLeaf && PREVIEW_LEAF_RE.test(previewLeaf))
    || validExportPreviewKey(row.preview_key, exportJobId, dzExportId);
  if (!previewOk) return { ok: false, error: 'invalid preview_key' };
  // The original is optional ("link if linkable"); an invalid/legacy-shaped
  // image key DEGRADES to "no original" instead of dropping the whole piece.
  const hasImage = row.image_key != null && row.image_key !== ''
    && !!validateRenderKey(row.image_key, jobId, artifactId);
  if (!isFinitePositive(row.preview_width) || !isFinitePositive(row.preview_height)) {
    return { ok: false, error: 'invalid preview dimensions' };
  }
  const ordinal = Number.isFinite(row.ordinal) ? row.ordinal : index;
  return {
    ok: true,
    value: {
      ordinal,
      job_id: jobId,
      artifact_id: artifactId,
      preview_key: row.preview_key,
      image_key: hasImage ? row.image_key : null,
      preview_url: originAbsolute(trustedOrigin, row.preview_key),
      image_url: hasImage ? originAbsolute(trustedOrigin, row.image_key) : null,
      preview_width: row.preview_width,
      preview_height: row.preview_height,
      function: typeof row.function === 'string' ? row.function : '',
      degree: Number.isFinite(row.degree) ? row.degree : null,
      N: Number.isFinite(row.N) ? row.N : null,
      times: Number.isFinite(row.times) ? row.times : null,
      created_at: typeof row.created_at === 'string' ? row.created_at : '',
      title: typeof row.title === 'string' ? row.title : '',
      export_job_id: exportJobId,
      family,
      deepzoom: validateDeepzoom(row.deepzoom, { dziJobId: exportJobId, artifactId, trustedOrigin }),
    },
  };
}

function normalizeMosaicTile(row, index, trustedOrigin) {
  if (!row || typeof row !== 'object') return { ok: false, error: 'row is not an object' };
  const jobId = row.job_id;
  const artifactId = row.artifact_id;
  if (!isValidId(jobId) || !isValidId(artifactId)) return { ok: false, error: 'bad job/artifact id' };
  const previewLeaf = validateColorKey(row.key, jobId, artifactId);
  const imageLeaf = validateColorKey(row.image_key, jobId, artifactId);
  if (!previewLeaf || !PREVIEW_LEAF_RE.test(previewLeaf)) return { ok: false, error: 'invalid preview key' };
  if (!imageLeaf) return { ok: false, error: 'invalid image_key' };
  if (!isFinitePositive(row.preview_width) || !isFinitePositive(row.preview_height)) {
    return { ok: false, error: 'invalid preview dimensions' };
  }
  return {
    ok: true,
    value: {
      ordinal: index,
      job_id: jobId,
      artifact_id: artifactId,
      preview_key: row.key,
      image_key: row.image_key,
      preview_url: originAbsolute(trustedOrigin, row.key),
      image_url: originAbsolute(trustedOrigin, row.image_key),
      preview_width: row.preview_width,
      preview_height: row.preview_height,
      function: typeof row.function === 'string' ? row.function : '',
      degree: Number.isFinite(row.degree) ? row.degree : null,
      N: Number.isFinite(row.N) ? row.N : null,
      times: Number.isFinite(row.times) ? row.times : null,
      created_at: typeof row.created_at === 'string' ? row.created_at : '',
      title: '',       // artifact_mosaic tiles carry no curator title
      deepzoom: null,  // artifact_mosaic never carries a DZI (§4)
    },
  };
}
