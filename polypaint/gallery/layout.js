// Virtual Gallery — deterministic selection + maze layout (pure, no DOM/WebGL).
//
// Two concerns:
//   1. selectAndSort — size/sort/seed selection for the AllCol/mosaic source (the
//      viewer uses it to cap a large mosaic; curated shares keep ordinal order).
//   2. computeMaze / mazeClamp / mazeClampMove — the walkable maze the viewer
//      builds and collides against (below).
//
// Determinism is a hard requirement: the same manifest + URL must produce the
// same pieces and placement on every browser and locale. All string ordering
// uses explicit code-unit comparison (never localeCompare, which is
// locale-dependent), and randomness is a seeded PRNG.

// Code-unit string comparison — stable across browsers and locales, unlike
// String.prototype.localeCompare (js/13 uses localeCompare, which is NOT
// cross-browser deterministic; the shared helper must not).
function cmpStr(a, b) {
  const sa = String(a == null ? '' : a);
  const sb = String(b == null ? '' : b);
  return sa < sb ? -1 : sa > sb ? 1 : 0;
}

function cmpNum(a, b) {
  const na = Number.isFinite(a) ? a : -Infinity;
  const nb = Number.isFinite(b) ? b : -Infinity;
  return na < nb ? -1 : na > nb ? 1 : 0;
}

function identityTieBreak(a, b) {
  return cmpStr(a.job_id, b.job_id) || cmpStr(a.artifact_id, b.artifact_id);
}

// Deterministic 32-bit hash of a seed (number or string) -> mulberry32 PRNG.
function hashSeed(seed) {
  const s = String(seed == null ? '' : seed);
  let h = 2166136261 >>> 0;
  for (let i = 0; i < s.length; i++) {
    h ^= s.charCodeAt(i);
    h = Math.imul(h, 16777619) >>> 0;
  }
  return h >>> 0;
}

function mulberry32(a) {
  return function () {
    a |= 0;
    a = (a + 0x6D2B79F5) | 0;
    let t = Math.imul(a ^ (a >>> 15), 1 | a);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

// Sort a copy of `pieces` by mode, then clamp to `size`. Every mode has a stable
// (job_id, artifact_id) tie-break; `random` derives a stable per-seed key so the
// same seed reproduces the same order and a different seed changes it. The limit
// is applied AFTER sorting (§16.1).
export function selectAndSort(pieces, { size = pieces.length, sort = 'date', seed = 1 } = {}) {
  const rows = pieces.slice();
  let comparator;
  switch (sort) {
    case 'job':      comparator = (a, b) => cmpStr(a.job_id, b.job_id) || identityTieBreak(a, b); break;
    case 'function': comparator = (a, b) => cmpStr(a.function, b.function) || identityTieBreak(a, b); break;
    case 'degree':   comparator = (a, b) => -cmpNum(a.degree, b.degree) || identityTieBreak(a, b); break;
    case 'N':        comparator = (a, b) => -cmpNum(a.N, b.N) || identityTieBreak(a, b); break;
    case 'random': {
      const rnd = mulberry32(hashSeed(seed));
      // Assign each row one draw up front, keyed by identity so the mapping is
      // stable for a seed regardless of input order.
      const keyed = rows
        .map((p) => ({ p, k: p.job_id + ' ' + p.artifact_id }))
        .sort((x, y) => cmpStr(x.k, y.k));
      const draw = new Map();
      for (const { k } of keyed) draw.set(k, rnd());
      comparator = (a, b) => {
        const da = draw.get(a.job_id + ' ' + a.artifact_id);
        const db = draw.get(b.job_id + ' ' + b.artifact_id);
        return (da < db ? -1 : da > db ? 1 : 0) || identityTieBreak(a, b);
      };
      break;
    }
    case 'date':
    default:         comparator = (a, b) => -cmpStr(a.created_at, b.created_at) || identityTieBreak(a, b); break;
  }
  rows.sort(comparator);
  const cap = Number.isFinite(size) && size > 0 ? Math.floor(size) : rows.length;
  return rows.slice(0, cap);
}

function isFinitePositive(v) {
  return typeof v === 'number' && Number.isFinite(v) && v > 0;
}

// ── Maze layout (virtual-gallery.md §3 "something more interesting") ─────────
// A seeded perfect maze (recursive backtracker). The viewer walks the corridors
// (walls you can't pass through) and finds art hung on a spread of the interior
// wall faces. Everything here is pure + deterministic for a seed.
export const MAZE = Object.freeze({
  CELL_M: 3.6,             // corridor cell size
  WALL_HEIGHT_M: 3.6,
  WALL_THICKNESS_M: 0.16,
  ART_CENTER_Y_M: 1.65,
  ART_WIDTH_M: 1.5,        // nominal art width on a wall face
  MAX_ART_HEIGHT_M: 2.6,
  PLANE_OFFSET_M: 0.10,    // art sits this far off the wall, into the corridor
  EYE_HEIGHT_M: 1.65,
  COLLISION_RADIUS_M: 0.30,
  MIN_GRID: 3,
  MAX_GRID: 12,
});

// N/E/S/W in grid terms: N = -z (row-1), S = +z (row+1), E = +x (col+1), W = -x.
const MAZE_DIRS = [
  { k: 'N', dr: -1, dc: 0, opp: 'S' },
  { k: 'S', dr: 1, dc: 0, opp: 'N' },
  { k: 'E', dr: 0, dc: 1, opp: 'W' },
  { k: 'W', dr: 0, dc: -1, opp: 'E' },
];

function pieceSizeOnWall(previewWidth, previewHeight) {
  const aspect = (isFinitePositive(previewWidth) && isFinitePositive(previewHeight))
    ? previewWidth / previewHeight : 1;
  let width = MAZE.ART_WIDTH_M;
  let height = width / aspect;
  if (height > MAZE.MAX_ART_HEIGHT_M) { height = MAZE.MAX_ART_HEIGHT_M; width = height * aspect; }
  // never wider than a cell face minus a margin
  const maxW = MAZE.CELL_M - 0.5;
  if (width > maxW) { height *= maxW / width; width = maxW; }
  return { width_m: width, height_m: height };
}

// Build a maze sized to the piece count and place each piece on its own interior
// wall face. Returns grid (for collision), wall segments (for meshes), piece
// placements (world position + rotationY + inward normal), spawn, and bounds.
export function computeMaze(pieces, { seed = 1, coverage = 35 } = {}) {
  const n = Math.max(1, pieces.length);
  // coverage = desired % of interior wall faces carrying art. Interior faces
  // ~= 2*(G-1)^2, so solve 2*(G-1)^2 >= n/frac for G. Lower coverage -> bigger,
  // emptier maze; 100% -> the smallest maze that still fits every piece.
  const frac = Math.max(5, Math.min(100, Number(coverage) || 35)) / 100;
  const G = Math.max(MAZE.MIN_GRID, Math.min(MAZE.MAX_GRID,
    Math.ceil(Math.sqrt(n / (2 * frac))) + 1));
  const cols = G, rows = G, CELL = MAZE.CELL_M;
  const rnd = mulberry32(hashSeed(seed));
  const grid = [];
  for (let i = 0; i < cols * rows; i++) grid.push({ N: true, E: true, S: true, W: true, visited: false });
  const at = (r, c) => grid[r * cols + c];

  // recursive backtracker (iterative)
  const stack = [[0, 0]];
  at(0, 0).visited = true;
  while (stack.length) {
    const [r, c] = stack[stack.length - 1];
    const nbrs = [];
    for (const d of MAZE_DIRS) {
      const nr = r + d.dr, nc = c + d.dc;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && !at(nr, nc).visited) nbrs.push({ d, nr, nc });
    }
    if (!nbrs.length) { stack.pop(); continue; }
    const pick = nbrs[Math.floor(rnd() * nbrs.length)];
    at(r, c)[pick.d.k] = false;
    at(pick.nr, pick.nc)[pick.d.opp] = false;
    at(pick.nr, pick.nc).visited = true;
    stack.push([pick.nr, pick.nc]);
  }
  for (const cell of grid) delete cell.visited;

  const originX = -(cols * CELL) / 2, originZ = -(rows * CELL) / 2;
  const cellCenter = (r, c) => ({ x: originX + (c + 0.5) * CELL, z: originZ + (r + 0.5) * CELL });

  // Interior wall faces available for art (a closed wall bordering this cell's
  // corridor), each with the inward normal + facing rotation.
  const faces = [];
  for (let r = 0; r < rows; r++) {
    for (let c = 0; c < cols; c++) {
      const cell = at(r, c), cen = cellCenter(r, c);
      const interior = { N: r > 0, S: r < rows - 1, E: c < cols - 1, W: c > 0 };
      if (cell.N && interior.N) faces.push({ r, c, x: cen.x, z: originZ + r * CELL, normal: { x: 0, z: 1 }, rotationY: 0 });
      if (cell.S && interior.S) faces.push({ r, c, x: cen.x, z: originZ + (r + 1) * CELL, normal: { x: 0, z: -1 }, rotationY: Math.PI });
      if (cell.W && interior.W) faces.push({ r, c, x: originX + c * CELL, z: cen.z, normal: { x: 1, z: 0 }, rotationY: Math.PI / 2 });
      if (cell.E && interior.E) faces.push({ r, c, x: originX + (c + 1) * CELL, z: cen.z, normal: { x: -1, z: 0 }, rotationY: -Math.PI / 2 });
    }
  }
  // Deterministic spread: shuffle faces by seed, one piece per face.
  for (let i = faces.length - 1; i > 0; i--) {
    const j = Math.floor(rnd() * (i + 1));
    const t = faces[i]; faces[i] = faces[j]; faces[j] = t;
  }
  const placements = [];
  for (let i = 0; i < pieces.length && i < faces.length; i++) {
    const f = faces[i], size = pieceSizeOnWall(pieces[i].preview_width, pieces[i].preview_height);
    placements.push({
      piece_index: i,
      position: { x: f.x + f.normal.x * MAZE.PLANE_OFFSET_M, y: MAZE.ART_CENTER_Y_M, z: f.z + f.normal.z * MAZE.PLANE_OFFSET_M },
      rotationY: f.rotationY,
      normal: { x: f.normal.x, z: f.normal.z },
      width_m: size.width_m,
      height_m: size.height_m,
    });
  }

  // Unique wall segments for meshes (verticals at x = originX + c*CELL, spanning
  // one cell in z; horizontals at z = originZ + r*CELL, spanning one cell in x).
  const wallSegments = [];
  for (let r = 0; r < rows; r++) {
    for (let c = 0; c <= cols; c++) {
      const closed = c === 0 ? at(r, 0).W : c === cols ? at(r, cols - 1).E : at(r, c).W;
      if (closed) wallSegments.push({ axis: 'z', x: originX + c * CELL, z: originZ + (r + 0.5) * CELL, len: CELL });
    }
  }
  for (let c = 0; c < cols; c++) {
    for (let r = 0; r <= rows; r++) {
      const closed = r === 0 ? at(0, c).N : r === rows ? at(rows - 1, c).S : at(r, c).N;
      if (closed) wallSegments.push({ axis: 'x', x: originX + (c + 0.5) * CELL, z: originZ + r * CELL, len: CELL });
    }
  }

  const spawn = cellCenter(0, 0);
  return {
    cols, rows, cell: CELL, height: MAZE.WALL_HEIGHT_M,
    origin: { x: originX, z: originZ },
    bounds: { minX: originX, maxX: originX + cols * CELL, minZ: originZ, maxZ: originZ + rows * CELL },
    grid, wallSegments, placements, spawn,
    placedCount: placements.length, faceCount: faces.length,
  };
}

// Corridor collision: clamp a proposed (x,z) so the camera (radius r) cannot
// cross a CLOSED wall of the cell it is in; open sides let it pass to a neighbor.
export function mazeClamp(maze, x, z, r) {
  const CELL = maze.cell;
  x = Math.max(maze.bounds.minX + r, Math.min(maze.bounds.maxX - r, x));
  z = Math.max(maze.bounds.minZ + r, Math.min(maze.bounds.maxZ - r, z));
  let c = Math.floor((x - maze.origin.x) / CELL);
  let rr = Math.floor((z - maze.origin.z) / CELL);
  c = Math.max(0, Math.min(maze.cols - 1, c));
  rr = Math.max(0, Math.min(maze.rows - 1, rr));
  const cellMinX = maze.origin.x + c * CELL, cellMinZ = maze.origin.z + rr * CELL;
  const w = maze.grid[rr * maze.cols + c];
  if (w.W) x = Math.max(x, cellMinX + r);
  if (w.E) x = Math.min(x, cellMinX + CELL - r);
  if (w.N) z = Math.max(z, cellMinZ + r);
  if (w.S) z = Math.min(z, cellMinZ + CELL - r);
  return { x, z };
}

// Swept corridor collision: resolve X then Z against the walls of the cell the
// camera is CURRENTLY in (from the previous position), so a single large frame
// step cannot tunnel through a closed wall (the point-clamp picks the far cell
// once the step crosses a boundary). Used for continuous WASD movement; the
// point form mazeClamp() is for teleports/spawn.
export function mazeClampMove(maze, px, pz, x, z, r) {
  const CELL = maze.cell;
  const cellIndex = (wx, wz) => {
    const c = Math.max(0, Math.min(maze.cols - 1, Math.floor((wx - maze.origin.x) / CELL)));
    const rr = Math.max(0, Math.min(maze.rows - 1, Math.floor((wz - maze.origin.z) / CELL)));
    return { c, rr };
  };
  x = Math.max(maze.bounds.minX + r, Math.min(maze.bounds.maxX - r, x));
  z = Math.max(maze.bounds.minZ + r, Math.min(maze.bounds.maxZ - r, z));

  // X against the current cell's E/W walls.
  let { c, rr } = cellIndex(px, pz);
  let w = maze.grid[rr * maze.cols + c];
  const minX = maze.origin.x + c * CELL;
  if (x > px && w.E) x = Math.min(x, minX + CELL - r);
  if (x < px && w.W) x = Math.max(x, minX + r);

  // Z against the walls of the cell reached after the X move.
  ({ c, rr } = cellIndex(x, pz));
  w = maze.grid[rr * maze.cols + c];
  const minZ = maze.origin.z + rr * CELL;
  if (z > pz && w.S) z = Math.min(z, minZ + CELL - r);
  if (z < pz && w.N) z = Math.max(z, minZ + r);

  return { x, z };
}
