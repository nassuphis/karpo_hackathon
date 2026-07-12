// Virtual Gallery — standalone viewer shell (virtual-gallery.md Phase 0).
//
// Orchestrates the WebGL scene, navigation, texture scheduling, and DeepZoom
// inspection. The trust boundary (manifest validation) and the deterministic
// geometry/scheduling policy live in the sibling pure modules, which are
// unit-tested; this file wires them to Three.js + OpenSeadragon and is verified
// interactively. Everything the manifest declares is untrusted: metadata is
// rendered with textContent, asset URLs come only from the validated keys.

import * as THREE from 'three';
import { PointerLockControls } from 'three/addons/controls/PointerLockControls.js';
import { LineSegments2 } from 'three/addons/lines/LineSegments2.js';
import { LineSegmentsGeometry } from 'three/addons/lines/LineSegmentsGeometry.js';
import { LineMaterial } from 'three/addons/lines/LineMaterial.js';
import { parseTrustedManifestUrl, normalizeManifest, GALLERY_LIMITS, isValidId } from './manifest.js';
import { computeLayout, mazeClamp, mazeClampMove, MAZE, selectAndSort, gridPath } from './layout.js';
import { GalleryTextureManager, TEXTURE_LIMITS } from './texture-manager.js';

const PREVIEW_FETCH_TIMEOUT_MS = 12_000;
const SCHEDULE_INTERVAL_MS = 220;
const MOVE_SPEED = 3.2;              // metres / second
// Tour mode: an automatic walk-through — corridor speed a touch under walking
// pace, gentle steering, a slower final turn onto each piece, then a dwell.
const TOUR_SPEED = 2.3;              // metres / second
const TOUR_TURN_RATE = 2.6;          // rad / s while walking
const TOUR_FACE_RATE = 1.7;          // rad / s turning onto the art
const TOUR_DWELL_S = 3.0;            // seconds in front of each piece
const VIEW_DISTANCE_M = 2.4;        // guided-mode standoff from a piece
const VIEWER_MAX_PIECES = 64;       // hard cap on meshes/queued previews (§5/§8)
const ART_PLACEHOLDER_COLOR = 0x3a332a;
const DEFAULT_WALL_COLOR = 0xece4d6;
// Photographic skies (Gallery tab Sky selector): id -> skybox/<id>.jpg
const IMAGE_SKIES = new Set(['galaxies', 'milkyway', 'moonlit']);

const MOON_AZ = -Math.PI * 0.32;    // moon direction (a touch N of E), high in the sky
const MOON_ALT = 0.82;

// Deterministic [0,1) noise (Math.random is unavailable/undesired) for the
// scattered background stars, and a point on the sky dome from azimuth/altitude.
function _hash01(n) { const s = Math.sin(n) * 43758.5453; return s - Math.floor(s); }
function _domePoint(az, alt, R) {
  const h = Math.cos(alt) * R, y = Math.sin(alt) * R;
  return new THREE.Vector3(Math.cos(az) * h, y, Math.sin(az) * h);
}

// A few recognizable constellations, one over each cardinal wall, as fixed
// landmarks for orientation. Star coords are local (x right, y up), roughly
// centered; `over` picks the wall, `alt` the altitude.
const CONSTELLATIONS = [
  { name: 'Big Dipper', over: 'north', alt: 1.05,
    stars: [[-3, -0.7], [-0.6, -0.5], [-0.3, 0.9], [-2.8, 0.8], [0.9, 1.3], [2.2, 1.6], [3.4, 1.4]],
    edges: [[0, 1], [1, 2], [2, 3], [3, 0], [2, 4], [4, 5], [5, 6]] },
  { name: 'Orion', over: 'south', alt: 0.62,
    stars: [[-1.1, 1.5], [1.1, 1.6], [-0.4, 0.1], [0, 0], [0.4, -0.1], [-0.9, -1.5], [1.3, -1.4]],
    edges: [[0, 2], [1, 4], [2, 3], [3, 4], [2, 5], [4, 6]] },
  { name: 'Cassiopeia', over: 'east', alt: 0.92,
    stars: [[-2, 0.1], [-1, 1], [0, 0], [1, 1.1], [2, 0.05]],
    edges: [[0, 1], [1, 2], [2, 3], [3, 4]] },
  { name: 'Southern Cross', over: 'west', alt: 0.55,
    stars: [[0, 1.6], [0, -1.6], [-1.1, 0.1], [1.1, -0.1]],
    edges: [[0, 1], [2, 3]] },
];

const $ = (id) => document.getElementById(id);

function showMessage(html) {
  const box = $('message-box');
  box.innerHTML = '';                       // structured nodes only (no untrusted text here)
  box.appendChild(html);
  $('message').classList.add('show');
}

function textEl(tag, text, cls) {
  const el = document.createElement(tag);
  if (cls) el.className = cls;
  if (text != null) el.textContent = text;
  return el;
}

function hasWebGL() {
  try {
    const c = document.createElement('canvas');
    return !!(window.WebGLRenderingContext && (c.getContext('webgl2') || c.getContext('webgl')));
  } catch { return false; }
}

// Stream the manifest with a hard byte cap enforced BEFORE fully buffering
// (§4.1): reject on an oversized Content-Length, then count bytes as they
// arrive and abort past the cap.
async function fetchManifestCapped(url, maxBytes, signal) {
  const resp = await fetch(url, { cache: 'no-store', signal });
  if (!resp.ok) {
    const err = new Error('manifest fetch failed (HTTP ' + resp.status + ')');
    err.status = resp.status;
    throw err;
  }
  const declared = resp.headers.get('content-length');
  if (declared && Number(declared) > maxBytes) throw new Error('manifest exceeds the size cap');
  if (!resp.body || !resp.body.getReader) {          // no streaming: fall back to a capped text read
    const text = await resp.text();
    if (text.length > maxBytes) throw new Error('manifest exceeds the size cap');
    return JSON.parse(text);
  }
  const reader = resp.body.getReader();
  const chunks = [];
  let received = 0;
  for (;;) {
    const { done, value } = await reader.read();
    if (done) break;
    received += value.length;
    if (received > maxBytes) { try { await reader.cancel(); } catch {} throw new Error('manifest exceeds the size cap'); }
    chunks.push(value);
  }
  const buf = new Uint8Array(received);
  let offset = 0;
  for (const c of chunks) { buf.set(c, offset); offset += c.length; }
  return JSON.parse(new TextDecoder().decode(buf));
}

// imageOrientation:'flipY' support probe: per WebIDL, browsers that don't know
// the option IGNORE it (no throw), so a try/catch can't detect it — draw a 1x2
// probe and check whether the pixels actually flipped.
let _flipYSupport = null;
async function bitmapFlipYSupported() {
  if (_flipYSupport !== null) return _flipYSupport;
  try {
    const c = document.createElement('canvas'); c.width = 1; c.height = 2;
    const g = c.getContext('2d');
    g.fillStyle = '#f00'; g.fillRect(0, 0, 1, 1);
    g.fillStyle = '#000'; g.fillRect(0, 1, 1, 1);
    const bm = await createImageBitmap(c, { imageOrientation: 'flipY' });
    const c2 = document.createElement('canvas'); c2.width = 1; c2.height = 2;
    const g2 = c2.getContext('2d');
    g2.drawImage(bm, 0, 0);
    _flipYSupport = g2.getImageData(0, 0, 1, 1).data[0] < 128;   // red moved to the bottom
    try { bm.close(); } catch {}
  } catch { _flipYSupport = false; }
  return _flipYSupport;
}

// Preview loader for the texture manager: cancellable, timed out, 404 -> permanent.
function makePreviewLoader() {
  return async function loadImage(url, signal) {
    const controller = new AbortController();
    const onAbort = () => controller.abort();
    if (signal) signal.addEventListener('abort', onAbort, { once: true });
    const timer = setTimeout(() => controller.abort(), PREVIEW_FETCH_TIMEOUT_MS);
    try {
      const resp = await fetch(url, { cache: 'force-cache', signal: controller.signal });
      if (resp.status === 404) { const e = new Error('preview 404'); e.permanent = true; throw e; }
      if (!resp.ok) throw new Error('preview HTTP ' + resp.status);
      const blob = await resp.blob();
      // Pre-flip the bitmap (WebGL can't flipY an ImageBitmap on upload, so a
      // plain createImageBitmap + Three's default flipY renders it upside down).
      // Paired with texture.flipY = false in makeTexture. On browsers without
      // imageOrientation support, flip through a canvas — a plain unflipped
      // bitmap would silently render inverted again.
      let bitmap;
      if (await bitmapFlipYSupported()) {
        bitmap = await createImageBitmap(blob, { imageOrientation: 'flipY' });
      } else {
        const raw = await createImageBitmap(blob);
        const c = document.createElement('canvas');
        c.width = raw.width; c.height = raw.height;
        const g = c.getContext('2d');
        g.translate(0, raw.height); g.scale(1, -1); g.drawImage(raw, 0, 0);
        try { raw.close(); } catch {}
        bitmap = await createImageBitmap(c);
      }
      return { bitmap, bytes: bitmap.width * bitmap.height * 4 };
    } finally {
      clearTimeout(timer);
      if (signal) signal.removeEventListener('abort', onAbort);
    }
  };
}

class GalleryViewer {
  constructor(scene) {
    this.spec = scene;
    this.pieces = scene.pieces;
    this._disposed = false;
    this._raf = 0;
    this._keys = new Set();
    this._lastTime = 0;
    this._lastSchedule = 0;
    this._focusIndex = -1;
    this._guidedIndex = -1;
    this._tour = null;               // active walk-through state, or null
    this._pinnedFocusId = null;     // the id currently pinned for focus (unpin on change)
    this._contextLost = false;
    this._abort = new AbortController();
    this._osd = null;
    this._listeners = [];
    this._reducedMotion = window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches;

    this._buildRenderer();
    this._buildScene();
    this._buildTextureManager();
    this._bindEvents();
    this._lastTime = performance.now();
    this._raf = requestAnimationFrame((t) => this._animate(t));
  }

  _buildRenderer() {
    this.renderer = new THREE.WebGLRenderer({ antialias: true, powerPreference: 'high-performance' });
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
    this.renderer.setSize(window.innerWidth, window.innerHeight);
    this.renderer.outputColorSpace = THREE.SRGBColorSpace;
    this._maxAniso = this.renderer.capabilities.getMaxAnisotropy ? Math.min(8, this.renderer.capabilities.getMaxAnisotropy()) : 1;
    $('gl-root').appendChild(this.renderer.domElement);
  }

  _buildScene() {
    const seed = (this.spec.layout && this.spec.layout.seed) || 1;
    const coverage = this.spec.settings ? this.spec.settings.wall_coverage : null;   // null -> legacy sizing
    const mode = (this.spec.settings && this.spec.settings.wall_layout) || 'maze';
    this.maze = computeLayout(this.pieces, { mode, seed, coverage });
    this.placements = this.maze.placements;

    this.scene = new THREE.Scene();
    this.scene.background = new THREE.Color(0x090c16);   // deep night, not pure black

    this.camera = new THREE.PerspectiveCamera(70, window.innerWidth / window.innerHeight, 0.05, 500);
    this.camera.position.set(this.maze.spawn.x, MAZE.EYE_HEIGHT_M, this.maze.spawn.z);
    this.camera.lookAt(0, MAZE.EYE_HEIGHT_M, 0);         // face into the maze
    this.scene.add(this.camera);
    this._spawn = this.camera.position.clone();

    // Moonlit night — art is unlit (MeshBasic) so it stays true-colour; walls and
    // floor take the moonlight. A bright directional comes FROM the moon, plus a
    // soft sky/ground fill so nothing reads as pure black.
    this.scene.add(new THREE.HemisphereLight(0x6274b0, 0x241c14, 1.1));
    this.scene.add(new THREE.AmbientLight(0x2b3a54, 0.5));
    const moon = new THREE.DirectionalLight(0xeaf0ff, 1.6);
    const md = _domePoint(MOON_AZ, MOON_ALT, 60);
    moon.position.set(md.x, md.y, md.z);
    this.scene.add(moon);

    this._skyMats = [];
    const skyMode = (this.spec.settings && this.spec.settings.sky) || 'stars';
    if (IMAGE_SKIES.has(skyMode)) {
      // Photographic sky: the equirect JPEG becomes the scene background. The
      // moon disc + procedural stars stay off (these skies bring their own);
      // the scene LIGHTS are unchanged, so the gallery reads the same.
      this._buildImageSky(skyMode);
    } else {
      this._buildMoon();                                  // the moon disc + halo
      if (skyMode !== 'dark') this._buildSky();
    }

    this.controls = new PointerLockControls(this.camera, this.renderer.domElement);
    this._raycaster = new THREE.Raycaster();
    this._artMeshes = [];
    this._labelMats = [];
    this._sharedPlane = new THREE.PlaneGeometry(1, 1);
    this._buildMazeShell(this.maze);
    this._buildPieces();
  }

  _buildMazeShell(maze) {
    const wallColor = (this.spec.settings && this.spec.settings.wall_color) || DEFAULT_WALL_COLOR;
    const wallMat = new THREE.MeshStandardMaterial({ color: new THREE.Color(wallColor), roughness: 0.95, metalness: 0.02 });
    // Self-tinted walls (default ON): the picked colour glows through the cool
    // moonlight, so white reads white instead of moon-blue — while the lit
    // component still shades corridors for depth. OFF = fully moonlit walls.
    if (!this.spec.settings || this.spec.settings.wall_self_tint !== false) {
      wallMat.emissive = new THREE.Color(wallColor);
      wallMat.emissiveIntensity = 0.45;
    }
    // Floor: a distinct cool slate with a grid aligned to the maze cells (one
    // tile per cell), emissive so the grid stays legible in the dark — clearly
    // different from the warm walls, and a strong orientation aid.
    this._floorTex = this._makeFloorTexture();
    this._floorTex.repeat.set(maze.cols, maze.rows);
    const floorMat = new THREE.MeshStandardMaterial({
      map: this._floorTex, emissiveMap: this._floorTex, emissive: 0x1c2740,
      emissiveIntensity: 0.5, roughness: 0.9, metalness: 0.0 });
    this._wallMat = wallMat;                    // exposed so a wall-color control can retint
    this._roomMats = [wallMat, floorMat];

    const W = maze.bounds.maxX - maze.bounds.minX, D = maze.bounds.maxZ - maze.bounds.minZ;
    const cx = (maze.bounds.minX + maze.bounds.maxX) / 2, cz = (maze.bounds.minZ + maze.bounds.maxZ) / 2;
    const floor = new THREE.Mesh(new THREE.PlaneGeometry(W, D), floorMat);
    floor.rotation.x = -Math.PI / 2; floor.position.set(cx, 0, cz);
    this.scene.add(floor);
    // Open to the sky (no ceiling). One shared unit box scaled per wall segment.
    this._wallGeo = new THREE.BoxGeometry(1, 1, 1);
    this._wallMeshes = [];
    const T = MAZE.WALL_THICKNESS_M, H = maze.height;
    for (const seg of maze.wallSegments) {
      const m = new THREE.Mesh(this._wallGeo, wallMat);
      if (seg.axis === 'z') m.scale.set(T, H, seg.len + T);   // wall runs along z
      else m.scale.set(seg.len + T, H, T);                    // wall runs along x
      m.position.set(seg.x, H / 2, seg.z);
      this._wallMeshes.push(m);
      this.scene.add(m);
    }
    this._buildWallEdges(maze, wallColor);
  }

  // Edge accent: fat lines (LineSegments2 — real pixel width; native GL lines
  // are stuck at 1px) along every wall-box edge, merged into ONE draw call, so
  // corners and junctions read instead of blending into a flat colour mass.
  // Width comes from settings.wall_edge_px; 0 disables the accent entirely.
  _buildWallEdges(maze, wallColor) {
    const widthPx = this.spec.settings && Number.isFinite(this.spec.settings.wall_edge_px)
      ? this.spec.settings.wall_edge_px : 1;
    if (widthPx <= 0) return;
    const unit = new THREE.EdgesGeometry(this._wallGeo);   // 12 edges of the unit box
    const pts = unit.getAttribute('position');
    unit.dispose();
    const T = MAZE.WALL_THICKNESS_M, H = maze.height;
    const positions = [];
    for (const seg of maze.wallSegments) {
      const sx = seg.axis === 'z' ? T : seg.len + T;
      const sz = seg.axis === 'z' ? seg.len + T : T;
      for (let i = 0; i < pts.count; i++) {
        positions.push(pts.getX(i) * sx + seg.x, pts.getY(i) * H + H / 2, pts.getZ(i) * sz + seg.z);
      }
    }
    const geo = new LineSegmentsGeometry();
    geo.setPositions(positions);
    const wc = new THREE.Color(wallColor);
    const lum = wc.r * 0.299 + wc.g * 0.587 + wc.b * 0.114;
    const edgeColor = lum < 0.25 ? wc.clone().lerp(new THREE.Color(0xffffff), 0.45)
                                 : wc.clone().multiplyScalar(0.35);
    this._wallEdgeMat = new LineMaterial({
      color: edgeColor.getHex(), linewidth: widthPx, worldUnits: false,
      transparent: true, opacity: 0.85,
    });
    this._wallEdgeMat.resolution.set(window.innerWidth, window.innerHeight);
    this._wallEdgeGeo = geo;
    const edges = new LineSegments2(geo, this._wallEdgeMat);
    edges.userData.wallEdge = true;
    this.scene.add(edges);
  }

  // Museum-style title placard: the piece title rendered to a small canvas,
  // centered, transparent background — hung beneath the artwork.
  _makeLabelTexture(text) {
    const label = String(text || '').slice(0, 60);
    const font = '500 44px -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif';
    const measure = document.createElement('canvas').getContext('2d');
    measure.font = font;
    const pad = 28;
    const w = Math.ceil(Math.min(1400, Math.max(120, measure.measureText(label).width + pad * 2)));
    const h = 72;
    const c = document.createElement('canvas'); c.width = w; c.height = h;
    const g = c.getContext('2d');
    g.font = font;
    g.textAlign = 'center'; g.textBaseline = 'middle';
    g.shadowColor = 'rgba(0,0,0,0.85)'; g.shadowBlur = 6;
    g.fillStyle = '#efe9df';
    g.fillText(label, w / 2, h / 2 + 2);
    const tex = new THREE.CanvasTexture(c);
    tex.colorSpace = THREE.SRGBColorSpace;
    tex.anisotropy = this._maxAniso;
    return { texture: tex, aspect: w / h };
  }

  // A cool slate floor with a grid (one tile per maze cell), drawn to a small
  // canvas and tiled — distinct from the walls and readable in the dark.
  _makeFloorTexture() {
    const S = 128;
    const c = document.createElement('canvas'); c.width = c.height = S;
    const g = c.getContext('2d');
    g.fillStyle = '#232c40'; g.fillRect(0, 0, S, S);           // cool slate base
    g.strokeStyle = '#5a678c'; g.lineWidth = 4; g.strokeRect(0, 0, S, S);   // cell grid
    g.strokeStyle = '#333e5a'; g.lineWidth = 1;                // faint centre cross
    g.beginPath(); g.moveTo(S / 2, 0); g.lineTo(S / 2, S); g.moveTo(0, S / 2); g.lineTo(S, S / 2); g.stroke();
    const tex = new THREE.CanvasTexture(c);
    tex.wrapS = tex.wrapT = THREE.RepeatWrapping;
    tex.colorSpace = THREE.SRGBColorSpace;
    tex.anisotropy = this._maxAniso;
    return tex;
  }

  // A large moon disc + soft halo (the actual light is a directional from the
  // same direction, set in _buildScene). Present even when the starfield is off.
  // Equirectangular photo sky — settings.sky ids map to <origin>/skybox/<id>.jpg
  // (published by deploy.sh from the converted skybox/ sources). Loads async:
  // the dark background stays until the texture arrives; a failed load keeps it.
  _buildImageSky(sky) {
    const tex = new THREE.TextureLoader().load(location.origin + '/skybox/' + sky + '.jpg');
    tex.mapping = THREE.EquirectangularReflectionMapping;
    tex.colorSpace = THREE.SRGBColorSpace;
    this.scene.background = tex;
  }

  _buildMoon() {
    const p = _domePoint(MOON_AZ, MOON_ALT, 285);
    const moonMat = new THREE.MeshBasicMaterial({ color: 0xf6f2e6, toneMapped: false });
    const moon = new THREE.Mesh(new THREE.SphereGeometry(13, 28, 18), moonMat);
    moon.position.copy(p); this.scene.add(moon);
    const haloMat = new THREE.MeshBasicMaterial({ color: 0xb6c2e6, transparent: true, opacity: 0.13, depthWrite: false, blending: THREE.AdditiveBlending, toneMapped: false });
    const halo = new THREE.Mesh(new THREE.SphereGeometry(24, 28, 18), haloMat);
    halo.position.copy(p); this.scene.add(halo);
    this._skyMats.push(moonMat, haloMat);
  }

  // A starfield dome plus a few recognizable constellations placed toward the
  // cardinal directions, so they work as fixed landmarks for orientation.
  _buildSky() {
    const R = 300;
    const group = new THREE.Group();
    this._skyGroup = group;

    // Scattered background stars over the upper hemisphere.
    const N = 1600;
    const pos = new Float32Array(N * 3);
    for (let i = 0; i < N; i++) {
      const az = _hash01(i * 2.13 + 1) * Math.PI * 2;
      const alt = 0.04 + _hash01(i * 7.71 + 3) * (Math.PI / 2 - 0.04);
      const p = _domePoint(az, alt, R);
      pos[i * 3] = p.x; pos[i * 3 + 1] = p.y; pos[i * 3 + 2] = p.z;
    }
    const starGeo = new THREE.BufferGeometry();
    starGeo.setAttribute('position', new THREE.BufferAttribute(pos, 3));
    const starMat = new THREE.PointsMaterial({ color: 0xdfe6ff, size: 1.1, sizeAttenuation: true, transparent: true, opacity: 0.85, depthWrite: false });
    group.add(new THREE.Points(starGeo, starMat));
    this._skyMats.push(starMat);

    // az convention here: +x = east (az 0), +z = south (az +90), -z = north.
    const N_AZ = -Math.PI / 2, S_AZ = Math.PI / 2, E_AZ = 0, W_AZ = Math.PI;
    for (const c of CONSTELLATIONS) this._addConstellation(group, c, R,
      { north: N_AZ, south: S_AZ, east: E_AZ, west: W_AZ }[c.over]);

    this.scene.add(group);
  }

  _addConstellation(group, c, R, az0) {
    const alt0 = c.alt, scale = 0.085;
    const pts = c.stars.map(([x, y]) => _domePoint(az0 + x * scale, alt0 + y * scale, R));
    const starGeo = new THREE.BufferGeometry().setFromPoints(pts);
    const starMat = new THREE.PointsMaterial({ color: 0xfff2cf, size: 3.2, sizeAttenuation: true, transparent: true, opacity: 1, depthWrite: false });
    group.add(new THREE.Points(starGeo, starMat));
    const linePts = [];
    for (const [i, j] of c.edges) { linePts.push(pts[i], pts[j]); }
    const lineGeo = new THREE.BufferGeometry().setFromPoints(linePts);
    const lineMat = new THREE.LineBasicMaterial({ color: 0x6f80b8, transparent: true, opacity: 0.45 });
    group.add(new THREE.LineSegments(lineGeo, lineMat));
    this._skyMats.push(starMat, lineMat);
  }

  _buildPieces() {
    const frameMat = new THREE.MeshStandardMaterial({ color: 0x0c0a08, roughness: 0.7, metalness: 0.1 });
    this._frameMat = frameMat;
    this.maze.placements.forEach((pl) => {
      const i = pl.piece_index;
      const piece = this.pieces[i];
      const group = new THREE.Group();
      group.position.set(pl.position.x, pl.position.y, pl.position.z);
      group.rotation.y = pl.rotationY;

      const frame = new THREE.Mesh(this._sharedPlane, frameMat);
      frame.scale.set(pl.width_m + 0.1, pl.height_m + 0.1, 1);
      frame.position.z = -0.01;
      group.add(frame);

      const artMat = new THREE.MeshBasicMaterial({ color: ART_PLACEHOLDER_COLOR, toneMapped: false });
      const art = new THREE.Mesh(this._sharedPlane, artMat);
      art.scale.set(pl.width_m, pl.height_m, 1);
      art.userData = { pieceIndex: i, id: this._pieceId(piece), material: artMat, frame, baseFrame: 0x0c0a08 };
      group.add(art);
      // Centered title placard beneath the image (curator title or "image N").
      const labelInfo = this._makeLabelTexture(this._pieceTitle(piece));
      const labelH = 0.11;
      const labelW = Math.min(pl.width_m, labelH * labelInfo.aspect);
      const labelMat = new THREE.MeshBasicMaterial({ map: labelInfo.texture, transparent: true, toneMapped: false, depthWrite: false });
      const labelMesh = new THREE.Mesh(this._sharedPlane, labelMat);
      labelMesh.scale.set(labelW, labelH, 1);
      labelMesh.position.set(0, -(pl.height_m / 2 + 0.055 + labelH / 2), 0.004);
      group.add(labelMesh);
      this._labelMats.push(labelMat);
      this._artMeshes.push(art);
      this.scene.add(group);
    });
  }

  // Texture/pin identity mirrors the backend's piece identity (code-review-29
  // F3): family included, so same-ID artifacts across families can never share
  // texture, retry, or pin state.
  _pieceId(piece) { return piece.job_id + '/' + (piece.family || 'color') + '/' + piece.artifact_id; }

  // Display title: the curator title, else a stable default ("image 1", …).
  _pieceTitle(piece) { return piece.title || ('image ' + ((piece.ordinal ?? 0) + 1)); }

  _buildTextureManager() {
    this.tm = new GalleryTextureManager({
      loadImage: makePreviewLoader(),
      makeTexture: (bitmap) => {
        const tex = new THREE.Texture();
        tex.image = bitmap;
        tex.flipY = false;             // the bitmap is already pre-flipped (loader)
        tex.colorSpace = THREE.SRGBColorSpace;
        tex.anisotropy = this._maxAniso;
        tex.generateMipmaps = true;
        tex.minFilter = THREE.LinearMipmapLinearFilter;
        tex.needsUpdate = true;
        // Free the decoded bitmap only AFTER Three has uploaded it to the GPU
        // (onUpdate fires post-upload). Closing it earlier detaches the source
        // before the deferred upload and the artwork renders blank.
        tex.onUpdate = () => { try { tex.image && tex.image.close && tex.image.close(); } catch {} };
        return tex;
      },
      closeBitmap: (bitmap) => { try { bitmap && bitmap.close && bitmap.close(); } catch {} },
      now: () => performance.now(),
    });
  }

  _bindEvents() {
    const on = (target, type, fn, opts) => { target.addEventListener(type, fn, opts); this._listeners.push([target, type, fn, opts]); };
    on(window, 'resize', () => this._onResize());
    on(this.renderer.domElement, 'click', () => {
      if (this._overlayOpen()) return;
      // Locked: left click inspects the crosshair piece (same as Enter). Unlocked:
      // a click (re)enters pointer lock.
      if (this.controls.isLocked) { if (this._focusIndex >= 0) this._inspectFocused(); }
      else this.controls.lock();
    });
    on(document, 'keydown', (e) => this._onKey(e, true));
    on(document, 'keyup', (e) => this._onKey(e, false));
    on(window, 'blur', () => this._keys.clear());
    on(document, 'visibilitychange', () => { this._keys.clear(); this.tm.setHidden(document.hidden); });
    on(this.renderer.domElement, 'webglcontextlost', (e) => this._onContextLost(e));
    on(this.renderer.domElement, 'webglcontextrestored', () => this._onContextRestored());
    this.controls.addEventListener('lock', () => { this._tourStop(); document.body.classList.add('locked'); });
    this.controls.addEventListener('unlock', () => { document.body.classList.remove('locked'); this._keys.clear(); });

    on($('btn-prev'), 'click', () => this._guidedStep(-1));
    on($('btn-next'), 'click', () => this._guidedStep(1));
    on($('btn-tour'), 'click', () => this._tourToggle());
    on($('btn-inspect'), 'click', () => this._inspectFocused());
    on($('overlay-close'), 'click', () => this._closeOverlay());
    on($('overlay-copy'), 'click', () => this._copyFocusedRef());
  }

  _onKey(e, down) {
    if (e.key === 'Escape') { if (this._overlayOpen()) this._closeOverlay(); else this._tourStop(); return; }
    if (this._overlayOpen()) return;
    switch (e.code) {
      case 'KeyW': case 'ArrowUp': this._setKey('f', down); break;
      case 'KeyS': case 'ArrowDown': this._setKey('b', down); break;
      case 'KeyA': case 'ArrowLeft': this._setKey('l', down); break;
      case 'KeyD': case 'ArrowRight': this._setKey('r', down); break;
      case 'Enter':
        if (down && this.controls.isLocked) { this._inspectFocused(); break; }
        return;   // unlocked: let Enter activate the focused button natively
      default: return;
    }
    if (down) e.preventDefault();
  }
  _setKey(k, down) { if (down) this._keys.add(k); else this._keys.delete(k); }

  _onResize() {
    if (this._disposed) return;
    this.camera.aspect = window.innerWidth / window.innerHeight;
    this.camera.updateProjectionMatrix();
    this.renderer.setSize(window.innerWidth, window.innerHeight);
    if (this._wallEdgeMat && this._wallEdgeMat.resolution) this._wallEdgeMat.resolution.set(window.innerWidth, window.innerHeight);
  }

  // WebGL context loss (§10): preventDefault so the browser will fire 'restored',
  // stop the loop, and drop the now-invalid GPU textures so they reload. Three
  // re-uploads geometries/materials itself once rendering resumes.
  _onContextLost(e) {
    if (e && e.preventDefault) e.preventDefault();
    this._contextLost = true;
    cancelAnimationFrame(this._raf);
    this._raf = 0;
    try { this.tm.reset(); } catch {}
    for (const mesh of this._artMeshes || []) {
      mesh.userData.material.map = null;
      mesh.userData.material.color.set(ART_PLACEHOLDER_COLOR);
      mesh.userData.material.needsUpdate = true;
    }
    const box = document.createElement('div');
    box.appendChild(textEl('h1', 'Restoring graphics…'));
    box.appendChild(textEl('p', 'The 3D context was lost; it will resume automatically.'));
    showMessage(box);
  }

  _onContextRestored() {
    if (this._disposed) return;
    this._contextLost = false;
    hideMessage();
    this._lastTime = performance.now();
    if (!this._raf) this._raf = requestAnimationFrame((t) => this._animate(t));
  }

  _animate(now) {
    if (this._disposed) return;
    this._raf = requestAnimationFrame((t) => this._animate(t));
    let dt = (now - this._lastTime) / 1000;
    this._lastTime = now;
    if (dt > 0.1) dt = 0.1;                       // clamp after a long stall

    if (this.controls.isLocked && !this._overlayOpen()) {
      const px = this.camera.position.x, pz = this.camera.position.z;   // pre-move position
      const step = MOVE_SPEED * dt;
      if (this._keys.has('f')) this.controls.moveForward(step);
      if (this._keys.has('b')) this.controls.moveForward(-step);
      if (this._keys.has('l')) this.controls.moveRight(-step);
      if (this._keys.has('r')) this.controls.moveRight(step);
      // Swept collision from the pre-move position so a fast frame can't tunnel.
      const p = mazeClampMove(this.maze, px, pz, this.camera.position.x, this.camera.position.z, MAZE.COLLISION_RADIUS_M);
      this.camera.position.set(p.x, MAZE.EYE_HEIGHT_M, p.z);
    }

    if (this._tour && !this.controls.isLocked && !this._overlayOpen()) this._tourTick(dt);

    if (now - this._lastSchedule > SCHEDULE_INTERVAL_MS) {
      this._lastSchedule = now;
      this._scheduleTextures();
    }
    this._applyReadyTextures();
    // Crosshair focus is a pointer-lock concept. While unlocked, leave focus as
    // guided navigation set it — do NOT raycast it back to nothing every frame.
    if (this.controls.isLocked) this._updateFocus();
    this.renderer.render(this.scene, this.camera);
    if (now - (this._lastMinimap || 0) > 100) { this._lastMinimap = now; this._updateMinimap(); }
  }

  // Top-down maze overview: walls, art dots, and a red you-are-here dot with a
  // heading line. Cheap to redraw on a small canvas a few times a second.
  _updateMinimap() {
    const canvas = $('minimap'); if (!canvas || !this.maze) return;
    const g = canvas.getContext('2d');
    const W = canvas.width, H = canvas.height, pad = 8, b = this.maze.bounds;
    const sx = (x) => pad + (x - b.minX) / (b.maxX - b.minX) * (W - 2 * pad);
    const sy = (z) => pad + (z - b.minZ) / (b.maxZ - b.minZ) * (H - 2 * pad);   // world +z = south = down
    g.clearRect(0, 0, W, H);
    g.strokeStyle = 'rgba(150,164,210,0.85)'; g.lineWidth = 1; g.beginPath();
    for (const s of this.maze.wallSegments) {
      if (s.axis === 'z') { g.moveTo(sx(s.x), sy(s.z - s.len / 2)); g.lineTo(sx(s.x), sy(s.z + s.len / 2)); }
      else { g.moveTo(sx(s.x - s.len / 2), sy(s.z)); g.lineTo(sx(s.x + s.len / 2), sy(s.z)); }
    }
    g.stroke();
    g.fillStyle = 'rgba(224,184,119,0.9)';   // art pieces
    for (const pl of this.maze.placements) { g.beginPath(); g.arc(sx(pl.position.x), sy(pl.position.z), 1.6, 0, 7); g.fill(); }
    const cx = sx(this.camera.position.x), cy = sy(this.camera.position.z);
    const dir = new THREE.Vector3(); this.camera.getWorldDirection(dir);
    g.strokeStyle = '#ff5555'; g.lineWidth = 2; g.beginPath(); g.moveTo(cx, cy); g.lineTo(cx + dir.x * 11, cy + dir.z * 11); g.stroke();
    g.fillStyle = '#ff3b3b'; g.beginPath(); g.arc(cx, cy, 3.4, 0, 7); g.fill();   // you-are-here
  }

  _clampCamera() {
    // Corridor collision: cannot cross a closed maze wall of the current cell.
    const p = mazeClamp(this.maze, this.camera.position.x, this.camera.position.z, MAZE.COLLISION_RADIUS_M);
    this.camera.position.x = p.x;
    this.camera.position.z = p.z;
    this.camera.position.y = MAZE.EYE_HEIGHT_M;
  }

  // Rank pieces by proximity + focus and hand the working set to the manager.
  _scheduleTextures() {
    const cam = this.camera.position;
    const desired = this._artMeshes.map((mesh) => {
      const wp = new THREE.Vector3();
      mesh.getWorldPosition(wp);
      const dist = wp.distanceTo(cam);
      const focused = mesh.userData.pieceIndex === this._focusIndex;
      const priority = (focused ? 10_000 : 0) + 1000 / (1 + dist);
      return { id: mesh.userData.id, url: this.pieces[mesh.userData.pieceIndex].preview_url, priority };
    });
    // Only queue the highest-priority working set that FITS in the resident
    // budget — desiring all pieces above the cap makes the lowest-priority ones
    // load, evict, and reload every pump (thrash) with no benefit.
    desired.sort((a, b) => b.priority - a.priority);
    this.tm.setDesired(desired.slice(0, TEXTURE_LIMITS.MAX_RESIDENT));
    this.tm.pump();
    this._updateDebug();
  }

  // Re-sync every mesh with the manager each frame: bind a newly-resident
  // texture, and when a texture has been EVICTED (get returns null) restore the
  // neutral placeholder. This keeps a mesh from holding a disposed texture and
  // lets a reloaded preview rebind. Cheap: get() has no side effects.
  _applyReadyTextures() {
    for (const mesh of this._artMeshes) {
      const mat = mesh.userData.material;
      const tex = this.tm.get(mesh.userData.id);
      if (tex) {
        if (mat.map !== tex) {
          mat.map = tex;
          mat.color.set(0xffffff);
          mat.needsUpdate = true;
        }
      } else if (mat.map) {
        mat.map = null;
        mat.color.set(ART_PLACEHOLDER_COLOR);
        mat.needsUpdate = true;
      }
    }
  }

  _updateFocus() {
    // Raycast art AND walls so a wall occludes art in the corridor beyond it —
    // otherwise the HUD names, frames, and Enter-inspects pieces you can't see.
    this._raycaster.setFromCamera(new THREE.Vector2(0, 0), this.camera);
    this._raycaster.far = 8;
    const hit = this._raycaster.intersectObjects(this._artMeshes.concat(this._wallMeshes || []), false)[0];
    const isArt = hit && hit.object.userData && hit.object.userData.pieceIndex != null;
    this._setFocus(isArt ? hit.object.userData.pieceIndex : -1);
  }

  _setFocus(index) {
    if (index === this._focusIndex) return;
    // restore previous frame
    if (this._focusIndex >= 0) {
      const prev = this._artMeshes.find((m) => m.userData.pieceIndex === this._focusIndex);
      if (prev) prev.userData.frame.material = this._frameMat;
    }
    // Move the focus texture pin (refcounted, so an overlapping inspection pin is
    // unaffected). Without this, every piece ever focused stays pinned forever.
    if (this._pinnedFocusId) { this.tm.unpin(this._pinnedFocusId); this._pinnedFocusId = null; }
    this._focusIndex = index;
    if (index >= 0) {
      this._pinnedFocusId = this._pieceId(this.pieces[index]);
      this.tm.pin(this._pinnedFocusId);
    }
    const hud = { title: '', sub: '' };
    if (index >= 0) {
      const mesh = this._artMeshes.find((m) => m.userData.pieceIndex === index);
      if (mesh) {
        if (!this._focusFrameMat) this._focusFrameMat = new THREE.MeshStandardMaterial({ color: 0xe0b877, roughness: 0.5, metalness: 0.2 });
        mesh.userData.frame.material = this._focusFrameMat;
      }
      const p = this.pieces[index];
      hud.title = this._pieceTitle(p);
      hud.sub = [p.job_id, p.degree != null ? 'deg ' + p.degree : '', p.N != null ? 'N=' + p.N : ''].filter(Boolean).join(' · ');
    }
    $('hud-title').textContent = hud.title;
    $('hud-sub').textContent = hud.sub;
    $('btn-inspect').disabled = index < 0;
  }

  _guidedStep(dir) {
    this._tourStop();                          // manual navigation takes over
    const placements = this.maze.placements;
    if (!placements.length) return;
    // placements[i].piece_index === i, so this index is also the piece index.
    this._guidedIndex = ((this._guidedIndex < 0 ? (dir < 0 ? 0 : -1) : this._guidedIndex) + dir + placements.length) % placements.length;
    if (this.controls.isLocked) this.controls.unlock();
    const pl = placements[this._guidedIndex];
    const normal = new THREE.Vector3(pl.normal.x, 0, pl.normal.z);
    const artPos = new THREE.Vector3(pl.position.x, MAZE.EYE_HEIGHT_M, pl.position.z);
    const target = artPos.clone().add(normal.clone().multiplyScalar(VIEW_DISTANCE_M));
    this._moveCameraTo(target, artPos);
    this._setFocus(pl.piece_index);
  }

  // ---- Tour: continuous walk-through -----------------------------------------
  // Visits every placement in curator order on a loop: BFS the corridor cells
  // to the piece's guided standoff (gridPath — consecutive cells always share
  // an OPEN wall, so the walk cannot cross geometry), steer along the path,
  // turn onto the art, dwell, continue. Any manual act (Prev/Next/Inspect,
  // pointer lock, Esc) stops it.
  _tourToggle() {
    if (this._tour) { this._tourStop(); return; }
    if (!this.maze.placements.length) return;
    if (this.controls.isLocked) this.controls.unlock();
    this._tour = { index: this._guidedIndex, waypoints: [], wi: 0, phase: 'walk', dwellLeft: 0, artPos: null, faceQuat: null };
    const btn = $('btn-tour');
    btn.textContent = 'Stop tour'; btn.classList.add('active');
    this._tourNextLeg();
  }

  _tourStop() {
    if (!this._tour) return;
    this._tour = null;
    const btn = $('btn-tour');
    if (btn) { btn.textContent = 'Tour'; btn.classList.remove('active'); }
  }

  _cellOf(x, z) {
    const CELL = MAZE.CELL_M;
    const originX = -(this.maze.cols * CELL) / 2, originZ = -(this.maze.rows * CELL) / 2;
    return { r: Math.max(0, Math.min(this.maze.rows - 1, Math.floor((z - originZ) / CELL))),
             c: Math.max(0, Math.min(this.maze.cols - 1, Math.floor((x - originX) / CELL))) };
  }

  _cellCenterPoint(r, c) {
    const CELL = MAZE.CELL_M;
    const originX = -(this.maze.cols * CELL) / 2, originZ = -(this.maze.rows * CELL) / 2;
    return new THREE.Vector3(originX + (c + 0.5) * CELL, MAZE.EYE_HEIGHT_M, originZ + (r + 0.5) * CELL);
  }

  // Camera-convention look quaternion (−Z toward the target) without touching
  // the live camera: orient a scratch camera and copy its rotation.
  _lookQuat(from, to) {
    if (!this._scratchCam) this._scratchCam = new THREE.PerspectiveCamera();
    const s = this._scratchCam;
    s.position.copy(from); s.lookAt(to); s.updateMatrixWorld();
    return s.quaternion.clone();
  }

  _tourNextLeg() {
    const placements = this.maze.placements;
    const t = this._tour;
    t.index = (t.index + 1 + placements.length) % placements.length;   // loops forever
    const pl = placements[t.index];
    const normal = new THREE.Vector3(pl.normal.x, 0, pl.normal.z);
    const artPos = new THREE.Vector3(pl.position.x, MAZE.EYE_HEIGHT_M, pl.position.z);
    const stand = artPos.clone().add(normal.clone().multiplyScalar(VIEW_DISTANCE_M));
    const clamped = mazeClamp(this.maze, stand.x, stand.z, MAZE.COLLISION_RADIUS_M);
    stand.set(clamped.x, MAZE.EYE_HEIGHT_M, clamped.z);
    const cells = gridPath(this.maze,
      this._cellOf(this.camera.position.x, this.camera.position.z),
      this._cellOf(stand.x, stand.z)) || [];
    const pts = cells.map((cc) => this._cellCenterPoint(cc.r, cc.c));
    pts.push(stand);
    while (pts.length > 1 && pts[0].distanceTo(this.camera.position) < 0.5) pts.shift();
    t.waypoints = pts; t.wi = 0; t.phase = 'walk';
    t.artPos = artPos; t.faceQuat = null;
    this._guidedIndex = t.index;          // Prev/Next/Inspect continue from here
    this._setFocus(pl.piece_index);       // HUD + pin now: the texture loads while we walk
  }

  _tourTick(dt) {
    const t = this._tour;
    if (t.phase === 'walk') {
      const target = t.waypoints[t.wi];
      const pos = this.camera.position;
      const dir = target.clone().sub(pos); dir.y = 0;
      const dist = dir.length();
      if (dist < 0.12) {
        t.wi++;
        if (t.wi >= t.waypoints.length) { t.phase = 'turn'; t.faceQuat = this._lookQuat(pos, t.artPos); }
        return;
      }
      dir.normalize();
      pos.addScaledVector(dir, Math.min(TOUR_SPEED * dt, dist));
      pos.y = MAZE.EYE_HEIGHT_M;
      if (dist > 0.3) {                    // steering is unstable when on top of the point
        this.camera.quaternion.rotateTowards(
          this._lookQuat(pos, target.clone().setY(MAZE.EYE_HEIGHT_M)), TOUR_TURN_RATE * dt);
      }
    } else if (t.phase === 'turn') {
      this.camera.quaternion.rotateTowards(t.faceQuat, TOUR_FACE_RATE * dt);
      if (this.camera.quaternion.angleTo(t.faceQuat) < 0.02) { t.phase = 'dwell'; t.dwellLeft = TOUR_DWELL_S; }
    } else {
      t.dwellLeft -= dt;
      if (t.dwellLeft <= 0) this._tourNextLeg();
    }
  }

  _moveCameraTo(target, lookAt) {
    // Reduced motion (or no animation budget): jump. Otherwise a short ease.
    const clampT = target.clone();
    this.camera.position.copy(clampT);
    this._clampCamera();
    this.camera.lookAt(lookAt);
  }

  // ---- Inspection overlay ---------------------------------------------------
  _overlayOpen() { return $('overlay').classList.contains('open'); }

  _inspectFocused() {
    this._tourStop();                          // inspection pauses the walk-through
    const index = this._focusIndex >= 0 ? this._focusIndex : this._guidedIndex;
    if (index < 0) return;
    if (this._inspecting != null) this.tm.unpin(this._pieceId(this.pieces[this._inspecting]));
    this._inspecting = index;
    const p = this.pieces[index];
    if (this.controls.isLocked) this.controls.unlock();
    this.tm.pin(this._pieceId(p));
    // Background guided controls must not be keyboard-activatable behind the modal.
    $('btn-prev').disabled = true; $('btn-next').disabled = true; $('btn-inspect').disabled = true;

    $('overlay-meta').textContent = [this._pieceTitle(p), p.job_id, p.artifact_id,
      p.degree != null ? 'deg ' + p.degree : '', p.N != null ? 'N=' + p.N : '',
      p.times != null ? '×' + p.times : '', p.created_at].filter(Boolean).join('  ·  ');
    const img = $('overlay-img');
    $('overlay-status').textContent = '';
    $('overlay').classList.add('open');
    $('overlay').setAttribute('aria-hidden', 'false');
    $('overlay-close').focus();
    if (p.deepzoom) {
      // Straight into DeepZoom — no intermediate preview step.
      img.style.display = 'none';
      img.src = '';
      this._openDeepZoom();
    } else {
      img.style.display = 'block';
      img.src = p.preview_url;
      img.alt = this._pieceTitle(p);
      $('osd').style.display = 'none';
    }
  }

  _closeOverlay() {
    $('overlay').classList.remove('open');
    $('overlay').setAttribute('aria-hidden', 'true');
    $('overlay-img').src = '';
    this._destroyOsd();
    if (this._inspecting != null) this.tm.unpin(this._pieceId(this.pieces[this._inspecting]));
    this._inspecting = null;
    $('btn-prev').disabled = false; $('btn-next').disabled = false;
    $('btn-inspect').disabled = this._focusIndex < 0 && this._guidedIndex < 0;
  }

  _openDeepZoom() {
    const p = this.pieces[this._inspecting];
    if (!p || !p.deepzoom || !window.OpenSeadragon) return;
    $('overlay-img').style.display = 'none';
    $('osd').style.display = 'block';
    $('overlay-status').textContent = 'loading zoom…';
    const dziUrl = p.deepzoom.dzi_url;
    const done = (ok) => { if (ok) $('overlay-status').textContent = ''; };
    const openToken = (this._osdOpenToken = (this._osdOpenToken || 0) + 1);
    // Defer creation one frame so the just-shown #osd container is laid out.
    // Same setup the DeepZoom tab uses (js/12 viewDeepZoom): the piece's own DZI,
    // NO crossOriginPolicy (the public bucket sends no CORS headers). #osd is
    // sized with explicit width/height because OSD rewrites the element's
    // position to "relative" (see gallery.html).
    requestAnimationFrame(() => {
      if (this._disposed || !this._overlayOpen() || openToken !== this._osdOpenToken) { done(false); return; }
      this._destroyOsd();
      let tileErrors = 0;
      this._osd = window.OpenSeadragon({
        element: $('osd'),
        prefixUrl: 'vendor/openseadragon-r411/images/',
        tileSources: dziUrl,
        showNavigator: true,
        navigatorPosition: 'BOTTOM_RIGHT',
        maxZoomPixelRatio: 4,
        gestureSettingsMouse: { clickToZoom: false },
      });
      this._osd.addHandler('open', () => {
        done(true);
        try { this._osd.viewport.goHome(true); } catch {}
      });
      this._osd.addHandler('open-failed', (e) => {
        done(false);
        $('overlay-status').textContent = 'Zoom unavailable: ' + ((e && e.message) || ('could not load ' + dziUrl));
      });
      this._osd.addHandler('tile-load-failed', () => {
        if (++tileErrors === 1) $('overlay-status').textContent = 'Zoom tiles failed to load (check the DeepZoom export).';
      });
    });
  }

  _destroyOsd() {
    if (this._osd) { try { this._osd.destroy(); } catch {} this._osd = null; }
    $('osd').innerHTML = '';
  }

  _copyFocusedRef() {
    const p = this.pieces[this._inspecting != null ? this._inspecting : this._focusIndex];
    if (!p) return;
    // "Copy link": the piece's standalone DeepZoom viewer (each export ships a
    // viewer.html next to its image.dzi) — a directly shareable per-image link.
    // Fallback (no DZI): this gallery's own share link.
    // viewer === false: admission recorded that this export never shipped a
    // standalone viewer.html — fall back to this gallery's own share link.
    const link = (p.deepzoom && p.deepzoom.viewer !== false)
      ? location.origin + '/deepzoom/' + (p.export_job_id || p.job_id) + '/' + p.deepzoom.export_id + '/viewer.html'
      : location.href;
    const btn = $('overlay-copy');
    const orig = btn.textContent;
    const flash = (label) => { btn.textContent = label; setTimeout(() => { btn.textContent = orig; }, 1400); };
    if (!navigator.clipboard) { flash('✗ No clipboard'); $('overlay-status').textContent = 'copy unavailable (insecure context)'; return; }
    navigator.clipboard.writeText(link).then(
      () => { flash('✓ Copied'); $('overlay-status').textContent = 'copied ' + link; },
      () => { flash('✗ Copy failed'); $('overlay-status').textContent = 'copy failed'; });
  }

  _updateDebug() {
    const s = this.tm.stats();
    const eff = Math.round(100 * this.maze.placedCount / Math.max(1, this.maze.faceCount));
    $('debug').textContent = `pieces ${this.pieces.length}${this._dropped ? `  (+${this._dropped} not shown)` : ''}\n` +
      `art on ${this.maze.placedCount}/${this.maze.faceCount} faces (${eff}% effective)\n` +
      `queued ${s.queued}  inflight ${s.inFlight}\n` +
      `resident ${s.resident}/${TEXTURE_LIMITS.MAX_RESIDENT}  gpu ${this.renderer.info.memory.textures}`;
  }

  destroy() {
    if (this._disposed) return;     // idempotent: beforeunload + pagehide both fire
    this._disposed = true;
    cancelAnimationFrame(this._raf);
    this._abort.abort();
    for (const [t, type, fn, opts] of this._listeners) t.removeEventListener(type, fn, opts);
    this._listeners = [];
    try { if (this.controls.isLocked) this.controls.unlock(); } catch {}
    try { this.controls.dispose(); } catch {}
    this._destroyOsd();
    if (this.tm) this.tm.destroy();
    // dispose owned GPU resources
    this._sharedPlane && this._sharedPlane.dispose();
    for (const mesh of this._artMeshes || []) {
      mesh.geometry && mesh.geometry !== this._sharedPlane && mesh.geometry.dispose();
      mesh.userData.material && mesh.userData.material.dispose();
    }
    (this._roomMats || []).forEach((m) => m.dispose());
    (this._skyMats || []).forEach((m) => m.dispose());
    (this._labelMats || []).forEach((m) => { if (m.map) m.map.dispose(); m.dispose(); });
    this._floorTex && this._floorTex.dispose();
    this._frameMat && this._frameMat.dispose();
    this._focusFrameMat && this._focusFrameMat.dispose();
    this._wallGeo && this._wallGeo.dispose();
    this._wallEdgeGeo && this._wallEdgeGeo.dispose();
    this._wallEdgeMat && this._wallEdgeMat.dispose();
    this.scene && this.scene.traverse((o) => {
      if (o.geometry && o.geometry !== this._sharedPlane && o.geometry !== this._wallGeo
          && o.geometry !== this._wallEdgeGeo) o.geometry.dispose();
    });
    if (this.renderer) { this.renderer.dispose(); this.renderer.domElement.remove(); }
  }
}

// ---- Boot -------------------------------------------------------------------
let VIEWER = null;

async function boot() {
  const params = new URLSearchParams(location.search);
  // Short links: gallery.html?share=<id>. The id is the ONLY payload — the
  // manifest path is reconstructed here, same-origin, at the fixed share
  // location (strictly less URL surface than accepting a full manifest URL).
  const shareId = String(params.get('share') || '').trim();
  if (!shareId) {
    return failMessage('No gallery specified', 'This viewer needs a ?share= link produced by “Open Gallery”.');
  }
  if (!isValidId(shareId)) {
    return failMessage('That gallery link is not valid', 'malformed share id');
  }
  const raw = location.origin + '/renders/_shared_mosaic/gallery/' + shareId + '/manifest.json';
  const requireHttps = location.protocol === 'https:';
  const parsed = parseTrustedManifestUrl(raw, { origin: location.origin, requireHttps });
  if (!parsed.ok) {
    return failMessage('That gallery link is not valid', parsed.error);
  }
  if (!hasWebGL()) {
    return failMessage('3D is not available in this browser',
      'This gallery needs WebGL. You can still open the raw manifest.', parsed.url.href);
  }

  showLoading();
  try {
    const doc = await fetchManifestCapped(parsed.url.href, GALLERY_LIMITS.MANIFEST_MAX_BYTES);
    const norm = normalizeManifest(doc, { pathKind: parsed.pathKind, trustedOrigin: location.origin });
    if (!norm.ok) return failMessage('This gallery could not be loaded', norm.error);
    if (!norm.pieces.length) return failMessage('This gallery is empty', 'No valid pieces remained after validation.');
    // Hard cap on scene size (§5/§8): a share is already backend-capped, but a
    // large artifact_mosaic link could otherwise create thousands of meshes and
    // queue every preview. A gallery keeps curator order (slice); a mosaic has
    // no stored order (deterministic select).
    let truncated = 0;
    if (norm.pieces.length > VIEWER_MAX_PIECES) {
      truncated = norm.pieces.length - VIEWER_MAX_PIECES;
      norm.pieces = norm.kind === 'artifact_mosaic'
        ? selectAndSort(norm.pieces, { size: VIEWER_MAX_PIECES, sort: 'date', seed: 1 })
        : norm.pieces.slice(0, VIEWER_MAX_PIECES);
    }
    hideMessage();
    VIEWER = new GalleryViewer(norm);
    window.__galleryViewer = VIEWER;              // handle for manual inspection
    const dropped = norm.skipped.length + truncated;
    VIEWER._dropped = dropped;
    if (dropped) {
      const b = $('dropped-banner');
      b.textContent = dropped + ' piece' + (dropped === 1 ? '' : 's') + ' could not be shown (invalid entries — re-add or re-open after updating)';
      b.style.display = 'block';
    }
    if (dropped) {
      // Non-fatal: surface how many rows were dropped/truncated.
      $('debug').setAttribute('data-skipped', String(dropped));
    }
  } catch (err) {
    failMessage('This gallery could not be loaded', (err && err.message) || String(err));
  }
}

function showLoading() {
  const box = document.createElement('div');
  box.appendChild(textEl('h1', 'Opening gallery…'));
  showMessage(box);
}
function hideMessage() { $('message').classList.remove('show'); }

function failMessage(title, detail, rawLink) {
  const box = document.createElement('div');
  box.appendChild(textEl('h1', title));
  if (detail) box.appendChild(textEl('p', detail));
  if (rawLink) {
    const p = document.createElement('p');
    const a = document.createElement('a');
    a.href = rawLink; a.textContent = 'Open the raw manifest'; a.rel = 'noopener';
    p.appendChild(a); box.appendChild(p);
  }
  showMessage(box);
}

window.addEventListener('beforeunload', () => { if (VIEWER) VIEWER.destroy(); });
window.addEventListener('pagehide', () => { if (VIEWER) VIEWER.destroy(); });

// Expose selected internals for the smoke test (no behavioral effect).
window.__gallery = { parseTrustedManifestUrl, normalizeManifest, hasWebGL, boot };

boot();
