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

// ── Shared grid -> layout engine ────────────────────────────────────────────
// All three layout modes (maze / serpentine / exhibition) are expressed as a
// cols x rows grid of cells with N/E/S/W closed-wall flags. This one builder
// derives everything else from the grid: dedup'd wall segments for meshes,
// interior art faces, piece placements, spawn, bounds. Collision (mazeClamp /
// mazeClampMove) reads the same grid, so every mode collides correctly for free.
function _layoutFromGrid(pieces, grid, cols, rows, { rnd = null, placement = 'shuffle', cellOrder = null } = {}) {
  const CELL = MAZE.CELL_M;
  const at = (r, c) => grid[r * cols + c];
  const originX = -(cols * CELL) / 2, originZ = -(rows * CELL) / 2;
  const cellCenter = (r, c) => ({ x: originX + (c + 0.5) * CELL, z: originZ + (r + 0.5) * CELL });

  // Interior wall faces available for art (a closed wall bordering this cell's
  // corridor), each with the inward normal + facing rotation. Row-major order,
  // or an explicit walk order (spiral: faces must follow the corridor).
  const cells = cellOrder || (() => {
    const o = [];
    for (let r = 0; r < rows; r++) for (let c = 0; c < cols; c++) o.push([r, c]);
    return o;
  })();
  const faces = [];
  for (const [r, c] of cells) {
    {
      const cell = at(r, c), cen = cellCenter(r, c);
      const interior = { N: r > 0, S: r < rows - 1, E: c < cols - 1, W: c > 0 };
      if (cell.N && interior.N) faces.push({ r, c, x: cen.x, z: originZ + r * CELL, normal: { x: 0, z: 1 }, rotationY: 0 });
      if (cell.S && interior.S) faces.push({ r, c, x: cen.x, z: originZ + (r + 1) * CELL, normal: { x: 0, z: -1 }, rotationY: Math.PI });
      if (cell.W && interior.W) faces.push({ r, c, x: originX + c * CELL, z: cen.z, normal: { x: 1, z: 0 }, rotationY: Math.PI / 2 });
      if (cell.E && interior.E) faces.push({ r, c, x: originX + (c + 1) * CELL, z: cen.z, normal: { x: -1, z: 0 }, rotationY: -Math.PI / 2 });
    }
  }
  let chosen = faces;
  if (placement === 'shuffle' && rnd) {
    // Deterministic spread (maze): seeded shuffle, one piece per face.
    for (let i = faces.length - 1; i > 0; i--) {
      const j = Math.floor(rnd() * (i + 1));
      const t = faces[i]; faces[i] = faces[j]; faces[j] = t;
    }
  } else if (placement === 'stride' && faces.length > pieces.length) {
    // Ordered walks (serpentine/exhibition): keep curator order, spread evenly.
    const stride = faces.length / Math.max(1, pieces.length);
    chosen = Array.from({ length: pieces.length }, (_, i) => faces[Math.floor(i * stride)]);
  }
  const placements = [];
  for (let i = 0; i < pieces.length && i < chosen.length; i++) {
    const f = chosen[i], size = pieceSizeOnWall(pieces[i].preview_width, pieces[i].preview_height);
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

function _closedGrid(cols, rows) {
  const grid = [];
  for (let i = 0; i < cols * rows; i++) grid.push({ N: true, E: true, S: true, W: true });
  return grid;
}

// Build a maze sized to the piece count and place each piece on its own interior
// wall face. Returns grid (for collision), wall segments (for meshes), piece
// placements (world position + rotationY + inward normal), spawn, and bounds.
export function computeMaze(pieces, { seed = 1, coverage = null } = {}) {
  const n = Math.max(1, pieces.length);
  // coverage = desired % of interior wall faces carrying art. Interior faces
  // ~= 2*(G-1)^2, so solve 2*(G-1)^2 >= n/frac for G. Lower coverage -> bigger,
  // emptier maze; 100% -> the smallest maze that still fits every piece.
  // coverage == null (shares written before the knob existed) keeps the LEGACY
  // sizing so existing galleries don't silently re-lay out.
  const G = coverage == null
    ? Math.max(MAZE.MIN_GRID, Math.min(MAZE.MAX_GRID, Math.ceil(Math.sqrt(n)) + 2))
    : Math.max(MAZE.MIN_GRID, Math.min(MAZE.MAX_GRID,
        Math.ceil(Math.sqrt(n / (2 * (Math.max(5, Math.min(100, Number(coverage) || 35)) / 100))) + 1)));
  const cols = G, rows = G;
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

  return _layoutFromGrid(pieces, grid, cols, rows, { rnd, placement: 'shuffle' });
}

// Serpentine: one continuous switchback corridor. Parallel lanes along x,
// connected alternately at opposite ends; art hangs (in curator order) on both
// sides of the lane dividers.
export function computeSerpentine(pieces, { coverage = null } = {}) {
  const n = Math.max(1, pieces.length);
  const frac = Math.max(5, Math.min(100, Number(coverage) || 35)) / 100;
  const need = n / frac;                            // interior faces wanted
  // faces = 2*(rows-1)*(cols-1); prefer lanes about twice as long as the count.
  const rows = Math.max(2, Math.min(12, Math.ceil(Math.sqrt(need / 4)) + 1));
  const cols = Math.max(3, Math.min(16, Math.ceil(need / (2 * (rows - 1))) + 1));
  const grid = _closedGrid(cols, rows);
  const at = (r, c) => grid[r * cols + c];
  for (let r = 0; r < rows; r++) {
    for (let c = 0; c < cols - 1; c++) { at(r, c).E = false; at(r, c + 1).W = false; }   // lane corridor
  }
  for (let r = 0; r < rows - 1; r++) {
    const gap = (r % 2 === 0) ? cols - 1 : 0;       // alternate connecting ends
    at(r, gap).S = false; at(r + 1, gap).N = false;
  }
  return _layoutFromGrid(pieces, grid, cols, rows, { placement: 'stride' });
}

// Exhibition: freestanding parallel partition walls — equal length, equal
// spacing, open at both ends so every aisle is reachable. Art hangs (in curator
// order) on both sides of each partition.
export function computeExhibition(pieces, { coverage = null } = {}) {
  const n = Math.max(1, pieces.length);
  const frac = Math.max(5, Math.min(100, Number(coverage) || 35)) / 100;
  const need = n / frac;
  // faces = 2*(rows-1)*(cols-2); prefer partitions a bit longer than the count.
  const rows = Math.max(2, Math.min(12, Math.ceil(Math.sqrt(need / 2))));
  const cols = Math.max(4, Math.min(16, Math.ceil(need / (2 * (rows - 1))) + 2));
  const grid = _closedGrid(cols, rows);
  const at = (r, c) => grid[r * cols + c];
  for (let r = 0; r < rows; r++) {
    for (let c = 0; c < cols - 1; c++) { at(r, c).E = false; at(r, c + 1).W = false; }   // open floor E/W
  }
  for (let r = 0; r < rows - 1; r++) {
    for (let c = 0; c < cols; c++) {
      if (c === 0 || c === cols - 1) { at(r, c).S = false; at(r + 1, c).N = false; }     // walk-around gaps
    }
  }
  return _layoutFromGrid(pieces, grid, cols, rows, { placement: 'stride' });
}

// Spiral: one square spiral corridor from the outer corner to the center. The
// walk order IS the curator order; art hangs on both sides of the spiral arms.
// Interior faces = 2*(G-1)^2 (same as the maze), so coverage sizing carries over.
export function computeSpiral(pieces, { coverage = null } = {}) {
  const n = Math.max(1, pieces.length);
  const frac = Math.max(5, Math.min(100, Number(coverage) || 35)) / 100;
  const G = Math.max(MAZE.MIN_GRID, Math.min(MAZE.MAX_GRID,
    Math.ceil(Math.sqrt(n / (2 * frac)) + 1)));
  const cols = G, rows = G;
  // Square-spiral cell walk covering every cell, outside -> center.
  const order = [];
  let top = 0, bottom = rows - 1, left = 0, right = cols - 1;
  while (top <= bottom && left <= right) {
    for (let c = left; c <= right; c++) order.push([top, c]);
    for (let r = top + 1; r <= bottom; r++) order.push([r, right]);
    if (top < bottom) for (let c = right - 1; c >= left; c--) order.push([bottom, c]);
    if (left < right) for (let r = bottom - 1; r > top; r--) order.push([r, left]);
    top++; bottom--; left++; right--;
  }
  const grid = _closedGrid(cols, rows);
  const at = (r, c) => grid[r * cols + c];
  for (let i = 1; i < order.length; i++) {
    const [r1, c1] = order[i - 1], [r2, c2] = order[i];
    if (r2 === r1 && c2 === c1 + 1) { at(r1, c1).E = false; at(r2, c2).W = false; }
    else if (r2 === r1 && c2 === c1 - 1) { at(r1, c1).W = false; at(r2, c2).E = false; }
    else if (c2 === c1 && r2 === r1 + 1) { at(r1, c1).S = false; at(r2, c2).N = false; }
    else { at(r1, c1).N = false; at(r2, c2).S = false; }
  }
  return _layoutFromGrid(pieces, grid, cols, rows, { placement: 'stride', cellOrder: order });
}

// Merge collinear, touching wall segments into continuous RUNS. wallSegments
// dedup by cell FACE, so a straight partition arrives as N cell-length pieces;
// building a box per piece overlaps neighbours by the wall thickness and
// outlines every construction seam — at distance those seams + per-box edges
// became a dense shimmering lattice (the far-wall flicker). Geometry builds
// from runs; collision is grid-based and unaffected.
export function mergeWallRuns(segments, eps = 1e-6) {
  const groups = new Map();
  for (const s of segments) {
    const fixed = s.axis === 'z' ? s.x : s.z;
    const key = s.axis + '@' + fixed.toFixed(5);
    if (!groups.has(key)) groups.set(key, { axis: s.axis, fixed, spans: [] });
    const c = s.axis === 'z' ? s.z : s.x;
    groups.get(key).spans.push([c - s.len / 2, c + s.len / 2]);
  }
  const runs = [];
  for (const g of groups.values()) {
    g.spans.sort((a, b) => a[0] - b[0]);
    let lo = g.spans[0][0], hi = g.spans[0][1];
    const flush = () => {
      runs.push(g.axis === 'z'
        ? { axis: 'z', x: g.fixed, z: (lo + hi) / 2, len: hi - lo }
        : { axis: 'x', x: (lo + hi) / 2, z: g.fixed, len: hi - lo });
    };
    for (let i = 1; i < g.spans.length; i++) {
      const [s0, s1] = g.spans[i];
      if (s0 <= hi + eps) { hi = Math.max(hi, s1); }
      else { flush(); lo = s0; hi = s1; }
    }
    flush();
  }
  return runs;
}

// BFS shortest corridor path between two cells through OPEN walls. Returns
// [{r,c}, ...] inclusive of both endpoints, or null when unreachable (layouts
// are connected, so null only means out-of-bounds/corrupt input). Powers the
// viewer's Tour mode: consecutive cells are always adjacent with the shared
// wall open, so walking cell centers can never cross a wall.
export function gridPath(maze, from, to) {
  const { grid, cols, rows } = maze;
  const id = (r, c) => r * cols + c;
  const inb = (r, c) => r >= 0 && r < rows && c >= 0 && c < cols;
  if (!inb(from.r, from.c) || !inb(to.r, to.c)) return null;
  if (from.r === to.r && from.c === to.c) return [{ r: from.r, c: from.c }];
  const prev = new Map([[id(from.r, from.c), null]]);
  const queue = [{ r: from.r, c: from.c }];
  for (let qi = 0; qi < queue.length; qi++) {
    const { r, c } = queue[qi];
    const cell = grid[id(r, c)];
    for (const [k, nr, nc] of [['N', r - 1, c], ['S', r + 1, c], ['W', r, c - 1], ['E', r, c + 1]]) {
      if (cell[k] || !inb(nr, nc) || prev.has(id(nr, nc))) continue;   // closed wall / seen
      prev.set(id(nr, nc), id(r, c));
      if (nr === to.r && nc === to.c) {
        const path = [{ r: nr, c: nc }];
        for (let cur = id(r, c); cur !== null; cur = prev.get(cur)) {
          path.push({ r: (cur / cols) | 0, c: cur % cols });
        }
        return path.reverse();
      }
      queue.push({ r: nr, c: nc });
    }
  }
  return null;
}

// Standalone: free-standing panels on a checkerboard — each work hangs on a
// single lonely stand (art on BOTH faces), detached from every other panel and
// from the field edge. NO perimeter wall at all: the sky runs to the horizon,
// and the bounds clamp (not geometry) keeps the visitor on the field.
export function computeStandalone(pieces, { coverage = null } = {}) {
  const n = Math.max(1, pieces.length);
  const frac = Math.max(5, Math.min(100, Number(coverage) || 35)) / 100;
  // Panels sit on interior row boundaries of checkerboard cells; each carries
  // two faces, so a G-grid offers ~G*(G-1) faces.
  const G = Math.max(MAZE.MIN_GRID, Math.min(MAZE.MAX_GRID,
    Math.ceil(Math.sqrt(n / frac)) + 1));
  const cols = G, rows = G;
  const grid = [];
  for (let i = 0; i < cols * rows; i++) grid.push({ N: false, S: false, E: false, W: false });
  const at = (r, c) => grid[r * cols + c];
  for (let r = 1; r < rows; r++) {
    for (let c = 0; c < cols; c++) {
      if ((r + c) % 2 === 0) { at(r, c).N = true; at(r - 1, c).S = true; }
    }
  }
  return _layoutFromGrid(pieces, grid, cols, rows, { placement: 'stride' });
}

// Mode dispatch — the viewer's single entry point.
export function computeLayout(pieces, { mode = 'maze', seed = 1, coverage = null } = {}) {
  if (mode === 'serpentine') return computeSerpentine(pieces, { coverage });
  if (mode === 'exhibition') return computeExhibition(pieces, { coverage });
  if (mode === 'spiral') return computeSpiral(pieces, { coverage });
  if (mode === 'standalone') return computeStandalone(pieces, { coverage });
  return computeMaze(pieces, { seed, coverage });
}

// Corridor collision: clamp a proposed (x,z) so the camera (radius r) cannot
// cross a CLOSED wall of the cell it is in; open sides let it pass to a neighbor.
// Point form — used for teleports/spawn; continuous movement uses mazeClampMove.
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
