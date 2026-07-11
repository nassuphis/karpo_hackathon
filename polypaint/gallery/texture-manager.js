// Virtual Gallery — bounded preview texture manager (virtual-gallery.md §7/§7.1).
//
// One owner for all preview textures, with explicit limits. The scheduling
// policy (priority ranking, in-flight cap, resident budget, negative cache,
// generation guard, slot release on every outcome) is pure bookkeeping; the two
// side-effecting steps — decoding an image and creating a GPU texture — are
// injected, so this whole module is unit-testable with fakes and needs no WebGL.
//
// Injected collaborators:
//   loadImage(url, signal) -> Promise<{ bitmap, bytes }>
//       Rejects with err.permanent === true for a 404 (negative-cached), or a
//       transient error otherwise (bounded retry with cooldown).
//   makeTexture(bitmap) -> texture   (must expose .dispose(); may own the bitmap
//       and free it after GPU upload — the manager NEVER closes a bitmap that
//       became a live texture, only stale/never-uploaded ones).
//   closeBitmap(bitmap)              (frees a decoded image the manager still owns)

export const TEXTURE_LIMITS = Object.freeze({
  MAX_IN_FLIGHT: 4,
  MAX_RESIDENT: 48,
  MAX_BYTES: 128 * 1024 * 1024,
  MAX_RETRIES: 3,
  RETRY_COOLDOWN_MS: 10_000,
});

export class GalleryTextureManager {
  constructor({
    loadImage,
    makeTexture,
    closeBitmap = () => {},
    maxInFlight = TEXTURE_LIMITS.MAX_IN_FLIGHT,
    maxResident = TEXTURE_LIMITS.MAX_RESIDENT,
    maxBytes = TEXTURE_LIMITS.MAX_BYTES,
    now = () => 0,
  }) {
    this._loadImage = loadImage;
    this._makeTexture = makeTexture;
    this._closeBitmap = closeBitmap;
    this._maxInFlight = maxInFlight;
    this._maxResident = maxResident;
    this._maxBytes = maxBytes;
    this._now = now;

    this._want = new Map();        // id -> { url, priority }
    this._resident = new Map();    // id -> { texture, bytes }
    this._inflight = new Map();    // id -> AbortController
    this._promises = new Set();    // for tests to await settlement
    this._negative = new Set();    // id: permanent 404
    this._retry = new Map();       // id -> { count, until }
    this._generation = new Map();  // id -> monotonic counter
    this._pins = new Map();        // id -> refcount (focus + inspection can overlap)
    this._bytes = 0;
    this._hidden = false;
    this._destroyed = false;
  }

  // Declare the desired working set for this cadence (id -> priority, higher is
  // more important). Also drives eviction: a resident piece no longer wanted (or
  // wanted at low priority) is evicted before a high-priority one.
  setDesired(list) {
    this._want.clear();
    for (const { id, url, priority } of list) this._want.set(id, { url, priority });
  }

  // Reference-counted so an overlapping focus pin and inspection pin can't
  // unpin each other. A pinned piece is never evicted.
  pin(id) { this._pins.set(id, (this._pins.get(id) || 0) + 1); }
  unpin(id) {
    const n = (this._pins.get(id) || 0) - 1;
    if (n <= 0) this._pins.delete(id);
    else this._pins.set(id, n);
  }
  _isPinned(id) { return this._pins.has(id); }

  setHidden(hidden) { this._hidden = !!hidden; }

  // Return the resident texture or null. No side effects: safe to call for every
  // mesh every frame (eviction is priority-driven, not last-touch-driven).
  get(id) {
    const r = this._resident.get(id);
    return r ? r.texture : null;
  }

  // Start high-priority missing loads up to the in-flight cap. Idempotent and
  // cheap; call on a throttled cadence. Pauses scheduling while hidden.
  pump() {
    if (this._destroyed || this._hidden) return;
    const t = this._now();
    const candidates = [...this._want.entries()]
      .filter(([id]) =>
        !this._resident.has(id) &&
        !this._inflight.has(id) &&
        !this._negative.has(id) &&
        !this._retryCooling(id, t))
      .sort((a, b) => b[1].priority - a[1].priority);

    for (const [id, { url }] of candidates) {
      if (this._inflight.size >= this._maxInFlight) break;
      this._start(id, url);
    }
  }

  _retryCooling(id, t) {
    const r = this._retry.get(id);
    return !!(r && r.until > t);
  }

  _start(id, url) {
    const gen = (this._generation.get(id) || 0) + 1;
    this._generation.set(id, gen);
    const controller = new AbortController();
    this._inflight.set(id, controller);

    // Start the load immediately (synchronously) so the fetch is in flight the
    // moment the slot opens; guard a synchronous throw from the loader.
    let load;
    try { load = Promise.resolve(this._loadImage(url, controller.signal)); }
    catch (err) { load = Promise.reject(err); }
    const p = load
      .then((res) => {
        // Drop stale completions: the piece was re-requested or destroyed. No
        // texture was created, so the manager still owns the bitmap — close it.
        if (this._destroyed || this._generation.get(id) !== gen) {
          this._closeBitmap(res && res.bitmap);
          return;
        }
        // makeTexture takes ownership of the bitmap and frees it AFTER the GPU
        // upload (Three defers the upload to the next render). The manager must
        // NOT close it here — doing so detaches the source before upload and the
        // artwork renders blank ("source data has been detached").
        const texture = this._makeTexture(res.bitmap);
        this._resident.set(id, { texture, bytes: res.bytes || 0 });
        this._bytes += res.bytes || 0;
        this._retry.delete(id);
        this._evictIfNeeded();
      })
      .catch((err) => {
        if (this._destroyed || this._generation.get(id) !== gen) return;
        if (err && err.permanent) {
          this._negative.add(id);           // 404: never requeue until refresh
          return;
        }
        const r = this._retry.get(id) || { count: 0, until: 0 };
        r.count += 1;
        r.until = this._now() + TEXTURE_LIMITS.RETRY_COOLDOWN_MS;
        if (r.count >= TEXTURE_LIMITS.MAX_RETRIES) this._negative.add(id);
        else this._retry.set(id, r);
      })
      .finally(() => {
        // Release the slot on EVERY outcome (success/error/timeout/cancel).
        if (this._inflight.get(id) === controller) this._inflight.delete(id);
        this._promises.delete(p);
      });

    this._promises.add(p);
  }

  // Evict the lowest-PRIORITY non-pinned residents until both budgets are met.
  // Priority comes from the current desired set (a piece the camera moved away
  // from drops in priority and is reclaimed first); a resident no longer desired
  // has the lowest priority of all.
  _evictIfNeeded() {
    const overCount = () => this._resident.size > this._maxResident;
    const overBytes = () => this._bytes > this._maxBytes;
    if (!overCount() && !overBytes()) return;
    const pri = (id) => {
      const w = this._want.get(id);
      return w ? w.priority : -Infinity;
    };
    const evictable = [...this._resident.keys()]
      .filter((id) => !this._isPinned(id))
      .sort((a, b) => pri(a) - pri(b));   // lowest priority first
    for (const id of evictable) {
      if (!overCount() && !overBytes()) break;
      this._disposeResident(id, this._resident.get(id));
    }
  }

  _disposeResident(id, entry) {
    if (!entry) return;
    try { entry.texture.dispose(); } catch { /* already gone */ }
    // Fallback: free the decoded image if makeTexture's post-upload close never
    // ran (evicted before it was ever rendered). close() is a no-op if already
    // closed, so this is safe to call unconditionally.
    try {
      const img = entry.texture && entry.texture.image;
      if (img && img.close) img.close();
    } catch { /* already closed */ }
    this._bytes -= entry.bytes || 0;
    this._resident.delete(id);
  }

  // Explicitly forget one piece (e.g. removed during a future edit). Aborts an
  // in-flight load and disposes any resident texture.
  drop(id) {
    const inflight = this._inflight.get(id);
    if (inflight) { inflight.abort(); this._inflight.delete(id); }
    this._generation.set(id, (this._generation.get(id) || 0) + 1); // invalidate
    const entry = this._resident.get(id);
    if (entry) this._disposeResident(id, entry);
  }

  // Drop every resident texture + clear the negative cache (e.g. after a WebGL
  // context loss, where all GPU textures are gone and must reload).
  reset() {
    for (const controller of this._inflight.values()) controller.abort();
    this._inflight.clear();
    for (const [id, entry] of [...this._resident]) this._disposeResident(id, entry);
    this._resident.clear();
    this._negative.clear();
    this._retry.clear();
    this._bytes = 0;
  }

  stats() {
    return {
      queued: this._want.size,
      inFlight: this._inflight.size,
      resident: this._resident.size,
      bytes: this._bytes,
      negative: this._negative.size,
      pinned: this._pins.size,
    };
  }

  // For tests: yield past a macrotask so the chains of already-resolved loads
  // run to completion (resident/negative/in-flight reflect them).
  async settle() { await new Promise((resolve) => setTimeout(resolve, 0)); }

  destroy() {
    this._destroyed = true;
    for (const controller of this._inflight.values()) controller.abort();
    this._inflight.clear();
    for (const [id, entry] of this._resident) this._disposeResident(id, entry);
    this._resident.clear();
    this._want.clear();
    this._negative.clear();
    this._retry.clear();
    this._pins.clear();
    this._bytes = 0;
  }
}
