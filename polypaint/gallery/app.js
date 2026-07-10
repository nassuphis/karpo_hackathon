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
import { parseTrustedManifestUrl, normalizeManifest, GALLERY_LIMITS } from './manifest.js';
import { computeRoom, wallToWorld, ROOM } from './layout.js';
import { GalleryTextureManager, TEXTURE_LIMITS } from './texture-manager.js';

const PREVIEW_FETCH_TIMEOUT_MS = 12_000;
const SCHEDULE_INTERVAL_MS = 220;
const MOVE_SPEED = 3.2;              // metres / second
const VIEW_DISTANCE_M = 2.4;        // guided-mode standoff from a piece

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
      const bitmap = await createImageBitmap(blob);
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
    this._appliedTex = new Set();
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
    const { room, placements } = computeRoom(this.pieces);
    this.room = room;
    this.placements = placements;

    this.scene = new THREE.Scene();
    this.scene.background = new THREE.Color(0x14110d);

    this.camera = new THREE.PerspectiveCamera(70, window.innerWidth / window.innerHeight, 0.05, 200);
    this.camera.position.set(0, ROOM.EYE_HEIGHT_M, Math.min(room.depth_m / 2 - 1.2, room.depth_m / 4));
    this.scene.add(this.camera);
    this._spawn = this.camera.position.clone();

    // Quiet, deliberate lighting — art is unlit (MeshBasic), room is softly lit.
    this.scene.add(new THREE.HemisphereLight(0xfdf6ea, 0x20140c, 0.9));
    const key = new THREE.DirectionalLight(0xfff4e0, 0.35);
    key.position.set(2, 6, 3);
    this.scene.add(key);

    this._buildRoomShell(room);

    this.controls = new PointerLockControls(this.camera, this.renderer.domElement);
    this._raycaster = new THREE.Raycaster();
    this._artMeshes = [];
    this._sharedPlane = new THREE.PlaneGeometry(1, 1);
    this._buildPieces();
  }

  _buildRoomShell(room) {
    const hw = room.width_m / 2, hd = room.depth_m / 2, h = room.height_m;
    const wallMat = new THREE.MeshStandardMaterial({ color: 0xece4d6, roughness: 0.95, metalness: 0 });
    const floorMat = new THREE.MeshStandardMaterial({ color: 0x1b1712, roughness: 1, metalness: 0 });
    const ceilMat = new THREE.MeshStandardMaterial({ color: 0x0f0d0a, roughness: 1, metalness: 0 });
    this._roomMats = [wallMat, floorMat, ceilMat];

    const floor = new THREE.Mesh(new THREE.PlaneGeometry(room.width_m, room.depth_m), floorMat);
    floor.rotation.x = -Math.PI / 2;
    this.scene.add(floor);
    const ceil = new THREE.Mesh(new THREE.PlaneGeometry(room.width_m, room.depth_m), ceilMat);
    ceil.rotation.x = Math.PI / 2; ceil.position.y = h;
    this.scene.add(ceil);

    const mkWall = (w, x, z, ry) => {
      const m = new THREE.Mesh(new THREE.PlaneGeometry(w, h), wallMat);
      m.position.set(x, h / 2, z); m.rotation.y = ry; this.scene.add(m);
    };
    mkWall(room.width_m, 0, -hd, 0);            // north
    mkWall(room.width_m, 0, hd, Math.PI);       // south
    mkWall(room.depth_m, -hw, 0, Math.PI / 2);  // west
    mkWall(room.depth_m, hw, 0, -Math.PI / 2);  // east
  }

  _buildPieces() {
    const frameMat = new THREE.MeshStandardMaterial({ color: 0x0c0a08, roughness: 0.7, metalness: 0.1 });
    this._frameMat = frameMat;
    this.pieces.forEach((piece, i) => {
      const pl = this.placements[i];
      if (!pl) return;
      const w = wallToWorld(pl, this.room);
      const group = new THREE.Group();
      group.position.set(w.position.x, w.position.y, w.position.z);
      group.rotation.y = w.rotationY;

      const frame = new THREE.Mesh(this._sharedPlane, frameMat);
      frame.scale.set(pl.width_m + 0.08, pl.height_m + 0.08, 1);
      frame.position.z = -0.006;
      group.add(frame);

      const artMat = new THREE.MeshBasicMaterial({ color: 0x3a332a, toneMapped: false });
      const art = new THREE.Mesh(this._sharedPlane, artMat);
      art.scale.set(pl.width_m, pl.height_m, 1);
      art.userData = { pieceIndex: i, id: this._pieceId(piece), material: artMat, frame, baseFrame: 0x0c0a08 };
      group.add(art);
      this._artMeshes.push(art);
      this.scene.add(group);
    });
  }

  _pieceId(piece) { return piece.job_id + '/' + piece.artifact_id; }

  _buildTextureManager() {
    this.tm = new GalleryTextureManager({
      loadImage: makePreviewLoader(),
      makeTexture: (bitmap) => {
        const tex = new THREE.Texture();
        tex.image = bitmap;
        tex.colorSpace = THREE.SRGBColorSpace;
        tex.anisotropy = this._maxAniso;
        tex.generateMipmaps = true;
        tex.minFilter = THREE.LinearMipmapLinearFilter;
        tex.needsUpdate = true;
        return tex;
      },
      closeBitmap: (bitmap) => { try { bitmap && bitmap.close && bitmap.close(); } catch {} },
      now: () => performance.now(),
    });
  }

  _bindEvents() {
    const on = (target, type, fn, opts) => { target.addEventListener(type, fn, opts); this._listeners.push([target, type, fn, opts]); };
    on(window, 'resize', () => this._onResize());
    on(this.renderer.domElement, 'click', () => { if (!this.controls.isLocked && !this._overlayOpen()) this.controls.lock(); });
    on(document, 'keydown', (e) => this._onKey(e, true));
    on(document, 'keyup', (e) => this._onKey(e, false));
    on(window, 'blur', () => this._keys.clear());
    on(document, 'visibilitychange', () => { this._keys.clear(); this.tm.setHidden(document.hidden); });
    this.controls.addEventListener('lock', () => document.body.classList.add('locked'));
    this.controls.addEventListener('unlock', () => { document.body.classList.remove('locked'); this._keys.clear(); });

    on($('btn-prev'), 'click', () => this._guidedStep(-1));
    on($('btn-next'), 'click', () => this._guidedStep(1));
    on($('btn-inspect'), 'click', () => this._inspectFocused());
    on($('overlay-close'), 'click', () => this._closeOverlay());
    on($('overlay-copy'), 'click', () => this._copyFocusedRef());
    on($('overlay-original'), 'click', () => this._openOriginal());
    on($('overlay-zoom'), 'click', () => this._openDeepZoom());
  }

  _onKey(e, down) {
    if (e.key === 'Escape') { if (this._overlayOpen()) this._closeOverlay(); return; }
    if (this._overlayOpen()) return;
    switch (e.code) {
      case 'KeyW': case 'ArrowUp': this._setKey('f', down); break;
      case 'KeyS': case 'ArrowDown': this._setKey('b', down); break;
      case 'KeyA': case 'ArrowLeft': this._setKey('l', down); break;
      case 'KeyD': case 'ArrowRight': this._setKey('r', down); break;
      case 'Enter': if (down && this.controls.isLocked) this._inspectFocused(); break;
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
    if (this._osd) this._osd.viewport && this._osd.viewport.resize && this._osd.viewport.resize();
  }

  _animate(now) {
    if (this._disposed) return;
    this._raf = requestAnimationFrame((t) => this._animate(t));
    let dt = (now - this._lastTime) / 1000;
    this._lastTime = now;
    if (dt > 0.1) dt = 0.1;                       // clamp after a long stall

    if (this.controls.isLocked && !this._overlayOpen()) {
      const step = MOVE_SPEED * dt;
      if (this._keys.has('f')) this.controls.moveForward(step);
      if (this._keys.has('b')) this.controls.moveForward(-step);
      if (this._keys.has('l')) this.controls.moveRight(-step);
      if (this._keys.has('r')) this.controls.moveRight(step);
      this._clampCamera();
    }

    if (now - this._lastSchedule > SCHEDULE_INTERVAL_MS) {
      this._lastSchedule = now;
      this._scheduleTextures();
    }
    this._applyReadyTextures();
    this._updateFocus();
    this.renderer.render(this.scene, this.camera);
  }

  _clampCamera() {
    const hw = this.room.width_m / 2 - ROOM.COLLISION_RADIUS_M;
    const hd = this.room.depth_m / 2 - ROOM.COLLISION_RADIUS_M;
    this.camera.position.x = Math.max(-hw, Math.min(hw, this.camera.position.x));
    this.camera.position.z = Math.max(-hd, Math.min(hd, this.camera.position.z));
    this.camera.position.y = ROOM.EYE_HEIGHT_M;
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
    if (this._focusIndex >= 0) this.tm.pin(this._pieceId(this.pieces[this._focusIndex]));
    this.tm.setDesired(desired);
    this.tm.pump();
    this._updateDebug();
  }

  _applyReadyTextures() {
    for (const mesh of this._artMeshes) {
      if (this._appliedTex.has(mesh.userData.id)) continue;
      const tex = this.tm.get(mesh.userData.id);
      if (tex) {
        mesh.userData.material.map = tex;
        mesh.userData.material.color.set(0xffffff);
        mesh.userData.material.needsUpdate = true;
        this._appliedTex.add(mesh.userData.id);
      }
    }
  }

  _updateFocus() {
    if (!this.controls.isLocked) { this._setFocus(-1); return; }
    this._raycaster.setFromCamera(new THREE.Vector2(0, 0), this.camera);
    const hit = this._raycaster.intersectObjects(this._artMeshes, false)[0];
    this._setFocus(hit ? hit.object.userData.pieceIndex : -1);
  }

  _setFocus(index) {
    if (index === this._focusIndex) return;
    // restore previous frame
    if (this._focusIndex >= 0) {
      const prev = this._artMeshes.find((m) => m.userData.pieceIndex === this._focusIndex);
      if (prev) prev.userData.frame.material = this._frameMat;
    }
    this._focusIndex = index;
    const hud = { title: '', sub: '' };
    if (index >= 0) {
      const mesh = this._artMeshes.find((m) => m.userData.pieceIndex === index);
      if (mesh) {
        if (!this._focusFrameMat) this._focusFrameMat = new THREE.MeshStandardMaterial({ color: 0xe0b877, roughness: 0.5, metalness: 0.2 });
        mesh.userData.frame.material = this._focusFrameMat;
      }
      const p = this.pieces[index];
      hud.title = p.title || p.function || p.artifact_id;
      hud.sub = [p.job_id, p.degree != null ? 'deg ' + p.degree : '', p.N != null ? 'N=' + p.N : ''].filter(Boolean).join(' · ');
    }
    $('hud-title').textContent = hud.title;
    $('hud-sub').textContent = hud.sub;
    $('btn-inspect').disabled = index < 0;
  }

  _guidedStep(dir) {
    if (!this.pieces.length) return;
    this._guidedIndex = (this._guidedIndex + dir + this.pieces.length) % this.pieces.length;
    if (this.controls.isLocked) this.controls.unlock();
    const pl = this.placements[this._guidedIndex];
    const w = wallToWorld(pl, this.room);
    const normal = new THREE.Vector3(w.normal.x, 0, w.normal.z);
    const target = new THREE.Vector3(w.position.x, ROOM.EYE_HEIGHT_M, w.position.z).add(normal.multiplyScalar(VIEW_DISTANCE_M));
    const lookAt = new THREE.Vector3(w.position.x, ROOM.EYE_HEIGHT_M, w.position.z);
    this._moveCameraTo(target, lookAt);
    this._setFocus(this._guidedIndex);
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
    const index = this._focusIndex >= 0 ? this._focusIndex : this._guidedIndex;
    if (index < 0) return;
    this._inspecting = index;
    const p = this.pieces[index];
    if (this.controls.isLocked) this.controls.unlock();
    this.tm.pin(this._pieceId(p));

    $('overlay-meta').textContent = [p.title || p.function || p.artifact_id, p.job_id, p.artifact_id,
      p.degree != null ? 'deg ' + p.degree : '', p.N != null ? 'N=' + p.N : '',
      p.times != null ? '×' + p.times : '', p.created_at].filter(Boolean).join('  ·  ');
    const img = $('overlay-img');
    img.style.display = 'block';
    img.src = p.preview_url;
    img.alt = p.title || p.function || p.artifact_id;
    $('osd').style.display = 'none';
    $('overlay-status').textContent = '';
    $('overlay-zoom').style.display = p.deepzoom ? '' : 'none';
    $('overlay').classList.add('open');
    $('overlay').setAttribute('aria-hidden', 'false');
    $('overlay-close').focus();
  }

  _closeOverlay() {
    $('overlay').classList.remove('open');
    $('overlay').setAttribute('aria-hidden', 'true');
    $('overlay-img').src = '';
    this._destroyOsd();
    if (this._inspecting != null) this.tm.unpin(this._pieceId(this.pieces[this._inspecting]));
    this._inspecting = null;
  }

  _openDeepZoom() {
    const p = this.pieces[this._inspecting];
    if (!p || !p.deepzoom || !window.OpenSeadragon) return;
    $('overlay-img').style.display = 'none';
    $('osd').style.display = 'block';
    $('overlay-status').textContent = 'loading zoom…';
    this._destroyOsd();
    this._osd = window.OpenSeadragon({
      element: $('osd'),
      prefixUrl: 'vendor/openseadragon-r411/images/',
      tileSources: p.deepzoom.dzi_url,
      showNavigator: false,
      crossOriginPolicy: 'Anonymous',
      gestureSettingsMouse: { clickToZoom: false },
    });
    this._osd.addHandler('open', () => { $('overlay-status').textContent = ''; });
    this._osd.addHandler('open-failed', () => { $('overlay-status').textContent = 'zoom unavailable'; });
  }

  _destroyOsd() {
    if (this._osd) { try { this._osd.destroy(); } catch {} this._osd = null; }
    $('osd').innerHTML = '';
  }

  _openOriginal() {
    const p = this.pieces[this._inspecting != null ? this._inspecting : this._focusIndex];
    if (!p) return;
    // Full images can be enormous; never decode inline. Open in a new tab.
    window.open(p.image_url, '_blank', 'noopener');
  }

  _copyFocusedRef() {
    const p = this.pieces[this._inspecting != null ? this._inspecting : this._focusIndex];
    if (!p) return;
    const ref = p.job_id + ' / ' + p.artifact_id;
    if (navigator.clipboard) navigator.clipboard.writeText(ref).catch(() => {});
    $('overlay-status').textContent = 'copied ' + ref;
  }

  _updateDebug() {
    const s = this.tm.stats();
    $('debug').textContent = `pieces ${this.pieces.length}\nqueued ${s.queued}  inflight ${s.inFlight}\n` +
      `resident ${s.resident}/${TEXTURE_LIMITS.MAX_RESIDENT}  gpu ${this.renderer.info.memory.textures}`;
  }

  destroy() {
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
    this._frameMat && this._frameMat.dispose();
    this._focusFrameMat && this._focusFrameMat.dispose();
    this.scene && this.scene.traverse((o) => { if (o.geometry && o.geometry !== this._sharedPlane) o.geometry.dispose(); });
    if (this.renderer) { this.renderer.dispose(); this.renderer.domElement.remove(); }
  }
}

// ---- Boot -------------------------------------------------------------------
let VIEWER = null;

async function boot() {
  const params = new URLSearchParams(location.search);
  const raw = params.get('manifest');
  if (!raw) {
    return failMessage('No gallery specified', 'This viewer needs a ?manifest= share link produced by “Create Gallery”.');
  }
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
    hideMessage();
    VIEWER = new GalleryViewer(norm);
    window.__galleryViewer = VIEWER;              // handle for manual inspection
    if (norm.skipped.length) {
      // Non-fatal: surface how many rows were dropped, without blocking the scene.
      $('debug').setAttribute('data-skipped', String(norm.skipped.length));
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
