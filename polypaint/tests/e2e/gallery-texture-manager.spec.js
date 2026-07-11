// @ts-check
// Bounded preview texture manager policy (virtual-gallery.md §7/§7.1). The real
// module is imported in-browser and exercised with fakes — no WebGL. This gates
// the paths the viewer smoke test can't reach (eviction, pins, bitmap lifetime).
const { test, expect } = require('@playwright/test');

// Run `body(GalleryTextureManager, harnessFactory)` in the page and return its result.
async function run(page, body) {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
  return page.evaluate(async (bodySrc) => {
    const { GalleryTextureManager } = await import('/gallery/texture-manager.js');
    function harness() {
      const gate = new Map(); const disposed = []; const closed = []; let made = 0;
      const loadImage = (url) => new Promise((res, rej) => gate.set(url, { res, rej }));
      const makeTexture = (bitmap) => { made++; return { _id: made, image: bitmap, dispose() { disposed.push(this._id); } }; };
      const closeBitmap = (b) => { closed.push(b); };
      const resolve = (url, bytes = 10) => gate.get(url).res({ bitmap: { close() {} }, bytes });
      const reject = (url, permanent) => { const e = new Error('x'); if (permanent) e.permanent = true; gate.get(url).rej(e); };
      return { gate, disposed, closed, loadImage, makeTexture, closeBitmap, resolve, reject, get made() { return made; } };
    }
    // eslint-disable-next-line no-new-func
    const fn = new Function('GalleryTextureManager', 'harness', `return (${bodySrc})(GalleryTextureManager, harness);`);
    return fn(GalleryTextureManager, harness);
  }, body.toString());
}

test('success path keeps the bitmap alive (no premature close)', async ({ page }) => {
  const r = await run(page, async (TM, harness) => {
    const h = harness();
    const tm = new TM({ loadImage: h.loadImage, makeTexture: h.makeTexture, closeBitmap: h.closeBitmap, maxInFlight: 2 });
    tm.setDesired([{ id: 'a', url: 'ua', priority: 1 }]);
    tm.pump(); h.resolve('ua'); await tm.settle();
    return { hasTexture: !!tm.get('a'), closedCount: h.closed.length };
  });
  expect(r.hasTexture).toBe(true);
  expect(r.closedCount).toBe(0);   // texture owns the bitmap; manager must not close it
});

test('in-flight cap and priority eviction', async ({ page }) => {
  const r = await run(page, async (TM, harness) => {
    const h = harness();
    const capTm = new TM({ loadImage: h.loadImage, makeTexture: h.makeTexture, closeBitmap: h.closeBitmap, maxInFlight: 4, maxResident: 2 });
    capTm.setDesired(Array.from({ length: 10 }, (_, i) => ({ id: 'p' + i, url: 'u' + i, priority: 10 - i })));
    capTm.pump();
    const inFlight = capTm.stats().inFlight;

    const h2 = harness();
    const tm = new TM({ loadImage: h2.loadImage, makeTexture: h2.makeTexture, closeBitmap: h2.closeBitmap, maxInFlight: 5, maxResident: 2 });
    tm.setDesired([{ id: 'a', url: 'ua', priority: 1 }, { id: 'b', url: 'ub', priority: 2 }, { id: 'c', url: 'uc', priority: 3 }]);
    tm.pump();
    h2.resolve('ua'); await tm.settle(); h2.resolve('ub'); await tm.settle(); h2.resolve('uc'); await tm.settle();
    return { inFlight, resident: tm.stats().resident, a: !!tm.get('a'), b: !!tm.get('b'), c: !!tm.get('c'), disposedA: h2.disposed.includes(1) };
  });
  expect(r.inFlight).toBe(4);       // capped
  expect(r.resident).toBe(2);
  expect(r.a).toBe(false);          // lowest priority evicted
  expect(r.b).toBe(true);
  expect(r.c).toBe(true);
  expect(r.disposedA).toBe(true);   // disposed on eviction
});

test('refcounted pins survive overlapping focus + inspection', async ({ page }) => {
  const r = await run(page, async (TM, harness) => {
    const h = harness();
    const tm = new TM({ loadImage: h.loadImage, makeTexture: h.makeTexture, closeBitmap: h.closeBitmap, maxInFlight: 5, maxResident: 1 });
    tm.setDesired([{ id: 'a', url: 'ua', priority: 1 }, { id: 'b', url: 'ub', priority: 9 }]);
    tm.pin('a'); tm.pin('a');        // focus + inspection both pin
    tm.pump(); h.resolve('ua'); await tm.settle(); h.resolve('ub'); await tm.settle();
    const survivesTwoPins = !!tm.get('a');
    tm.unpin('a');
    const survivesOnePin = !!tm.get('a');
    tm.unpin('a');
    tm.setDesired([{ id: 'a', url: 'ua', priority: 1 }, { id: 'd', url: 'ud', priority: 5 }]);
    tm.pump(); h.resolve('ud'); await tm.settle();
    return { survivesTwoPins, survivesOnePin, evictableAtZero: !tm.get('a') };
  });
  expect(r.survivesTwoPins).toBe(true);
  expect(r.survivesOnePin).toBe(true);   // one unpin still leaves it pinned
  expect(r.evictableAtZero).toBe(true);
});

test('404 negative-cached and never requeued; reset reloads', async ({ page }) => {
  const r = await run(page, async (TM, harness) => {
    const h = harness();
    const tm = new TM({ loadImage: h.loadImage, makeTexture: h.makeTexture, closeBitmap: h.closeBitmap, maxInFlight: 2 });
    tm.setDesired([{ id: 'a', url: 'ua', priority: 1 }]);
    tm.pump(); h.reject('ua', true); await tm.settle();
    const negative = tm.stats().negative;
    h.gate.clear(); tm.pump();
    const requeued = h.gate.has('ua');
    // a resident then reset()
    const h2 = harness();
    const tm2 = new TM({ loadImage: h2.loadImage, makeTexture: h2.makeTexture, closeBitmap: h2.closeBitmap, maxInFlight: 2 });
    tm2.setDesired([{ id: 'a', url: 'ua', priority: 1 }]);
    tm2.pump(); h2.resolve('ua'); await tm2.settle();
    const before = !!tm2.get('a'); tm2.reset();
    return { negative, requeued, before, afterReset: !!tm2.get('a'), disposed: h2.disposed.includes(1) };
  });
  expect(r.negative).toBe(1);
  expect(r.requeued).toBe(false);
  expect(r.before).toBe(true);
  expect(r.afterReset).toBe(false);
  expect(r.disposed).toBe(true);
});

test('reset() invalidates in-flight generations — a late completion is dropped', async ({ page }) => {
  const r = await run(page, async (TM, harness) => {
    const h = harness();
    const tm = new TM({ loadImage: h.loadImage, makeTexture: h.makeTexture, closeBitmap: h.closeBitmap, maxInFlight: 2 });
    tm.setDesired([{ id: 'a', url: 'ua', priority: 1 }]);
    tm.pump();                 // 'a' now in flight
    tm.reset();                // reset while it is still loading
    h.resolve('ua');           // the load completes AFTER reset
    await tm.settle();
    return { resident: tm.stats().resident, hasA: !!tm.get('a') };
  });
  expect(r.resident).toBe(0);  // the stale completion did NOT repopulate resident
  expect(r.hasA).toBe(false);
});
