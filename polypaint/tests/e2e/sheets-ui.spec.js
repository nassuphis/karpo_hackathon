// @ts-check
const { test, expect } = require('@playwright/test');

const MANIFEST = {
  sheet_id: 'sheet_test',
  frames: 6,
  grid: { cols: 2, rows: 3 },
  tile_px: 32,
  margin_px: 4,
  n: 96,
  solver_mode: 'ae64',
  rotate: 0,
  polarity: 'white_on_black',
  label: true,
  viewport: { mode: 'quantile', quantile: 0.01, shim: 0.05 },
  scans: [
    { token: '$T', from: 0.5, to: 1.0, steps: 2, spacing: 'linear', values: [0.5, 1.0] },
    { token: '$S', from: 1, to: 3, steps: 3, spacing: 'linear', values: [1, 2, 3] },
  ],
  scan: { token: '$T', from: 0.5, to: 1.0, steps: 2, spacing: 'linear' },
  pipeline: {
    function: 'const',
    cfpv: [1, 0, 0],
    coeff_program_source_text: 'poly = fill(6, $T)\npoly[1] = $S*exp(2i)\nemit\n',
  },
  frame_records: [
    { frame: 0, values: [0.5, 1], bounds: [-2, 2, -2, 2] },
    { frame: 1, values: [1.0, 1], bounds: [-2, 2, -2, 2] },
    { frame: 2, values: [0.5, 2], bounds: [-2, 2, -2, 2] },
    { frame: 3, values: [1.0, 2], bounds: [-2, 2, -2, 2] },
    { frame: 4, values: [0.5, 3], bounds: [-2, 2, -2, 2] },
    { frame: 5, values: [1.0, 3], bounds: [-2, 2, -2, 2] },
  ],
};

test.beforeEach(async ({ page }) => {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
});

test.describe('Sheets frame picking', () => {
  test('image point maps to frames, gutters and borders return null', async ({ page }) => {
    const picks = await page.evaluate((m) => {
      return [
        _sheetFramePickFromImagePoint(m, 5, 5),      // inside frame 0
        _sheetFramePickFromImagePoint(m, 42, 44),    // row 1, col 1 -> frame 3
        _sheetFramePickFromImagePoint(m, 2, 2),      // outer border (margin)
        _sheetFramePickFromImagePoint(m, 37, 10),    // vertical gutter
        _sheetFramePickFromImagePoint(m, 10, 37),    // horizontal gutter
        _sheetFramePickFromImagePoint(m, 200, 10),   // beyond the grid
        _sheetFramePickFromImagePoint(m, 5, 115),    // row 3 (past rows)
      ];
    }, MANIFEST);
    expect(picks).toEqual([0, 3, null, null, null, null, null]);
  });

  test('incomplete last row and zero margin', async ({ page }) => {
    const picks = await page.evaluate((m) => {
      const partial = { ...m, frames: 5 };                    // frame 5 missing
      const noMargin = { ...m, margin_px: 0 };
      return [
        _sheetFramePickFromImagePoint(partial, 42, 80),       // frame 5 slot -> null
        _sheetFramePickFromImagePoint(noMargin, 0, 0),        // corner is frame 0
        _sheetFramePickFromImagePoint(noMargin, 33, 65),      // col 1, row 2 -> frame 5
      ];
    }, MANIFEST);
    expect(picks).toEqual([null, 0, 5]);
  });

  test('value literals match the server spelling', async ({ page }) => {
    const lits = await page.evaluate(() => [
      _sheetValueLiteral(5),
      _sheetValueLiteral(0.25),
      _sheetValueLiteral(-0.5),
      _sheetValueLiteral(-3),
    ]);
    expect(lits).toEqual(['5', '0.25', '(0-0.5)', '(0-3)']);
  });
});

test.describe('Sheets frame context menu', () => {
  test('menu renders frame values and Populate Frame substitutes tokens', async ({ page }) => {
    const result = await page.evaluate((m) => {
      _sheetViewerManifest = m;
      _sheetViewerId = m.sheet_id;
      _sheetContext = { sheetId: m.sheet_id, frame: 3 };
      _sheetRenderContextMenu(100, 100);
      const menu = document.getElementById('sheet-context-menu');
      const rendered = {
        visible: menu.style.display === 'block',
        text: menu.textContent,
        populateEnabled: !menu.querySelector('[data-sheet-action="populate-frame"]').disabled,
      };

      let captured = null;
      window._populateComputeFromDetail = (jobId, detail) => { captured = { jobId, detail }; };
      _sheetPopulateFrame();
      return {
        rendered,
        captured,
        menuClosed: menu.style.display === 'none',
        status: document.getElementById('sheets-status').textContent,
      };
    }, MANIFEST);

    expect(result.rendered.visible).toBe(true);
    expect(result.rendered.populateEnabled).toBe(true);
    expect(result.rendered.text).toContain('$T');
    expect(result.rendered.text).toContain('$S');
    expect(result.rendered.text).toContain('3 of 6');

    // frame 3 = ($T=1.0, $S=2): tokens replaced with the frame's literals
    expect(result.captured.detail.pipeline.coeff_program_source_text)
      .toBe('poly = fill(6, 1)\npoly[1] = 2*exp(2i)\nemit\n');
    expect(result.captured.detail.calc.solver).toBe('ae64');
    expect(result.menuClosed).toBe(true);
    expect(result.status).toContain('frame 3');
    expect(result.status).toContain('$T=1');
  });

  test('menu without pipeline disables Populate with a note', async ({ page }) => {
    const rendered = await page.evaluate((m) => {
      const legacy = { ...m };
      delete legacy.pipeline;
      _sheetViewerManifest = legacy;
      _sheetContext = { sheetId: legacy.sheet_id, frame: 0 };
      _sheetRenderContextMenu(50, 50);
      const menu = document.getElementById('sheet-context-menu');
      return {
        populateDisabled: menu.querySelector('[data-sheet-action="populate-frame"]').disabled,
        note: menu.textContent,
      };
    }, MANIFEST);
    expect(rendered.populateDisabled).toBe(true);
    expect(rendered.note).toContain('predates');
  });

  test('escape and outside click close the menu', async ({ page }) => {
    const states = await page.evaluate((m) => {
      _sheetViewerManifest = m;
      _sheetContext = { sheetId: m.sheet_id, frame: 1 };
      _sheetRenderContextMenu(60, 60);
      const menu = document.getElementById('sheet-context-menu');
      const afterOpen = menu.style.display;
      document.dispatchEvent(new KeyboardEvent('keydown', { key: 'Escape', bubbles: true }));
      const afterEscape = menu.style.display;

      _sheetContext = { sheetId: m.sheet_id, frame: 1 };
      _sheetRenderContextMenu(60, 60);
      document.body.click();
      const afterOutsideClick = menu.style.display;
      return { afterOpen, afterEscape, afterOutsideClick };
    }, MANIFEST);
    expect(states).toEqual({ afterOpen: 'block', afterEscape: 'none', afterOutsideClick: 'none' });
  });
});

test.describe('CR35 wave D regressions', () => {
  test('marquee unrotation maps corners for all quarter turns (F25)', async ({ page }) => {
    const results = await page.evaluate(() => {
      const corners = [[0, 0], [1, 0], [1, 1], [0, 1], [0.25, 0.5]];
      const out = {};
      for (const deg of [0, 90, 180, 270]) {
        out[deg] = corners.map(([x, y]) => {
          const p = _computePreviewUnrotateFrac(x, y, deg);
          return [Math.round(p.x * 100) / 100, Math.round(p.y * 100) / 100];
        });
      }
      return out;
    });
    // identity
    expect(results[0]).toEqual([[0, 0], [1, 0], [1, 1], [0, 1], [0.25, 0.5]]);
    // display top-left under a CCW-90 view came from image top-right
    expect(results[90][0]).toEqual([1, 0]);
    expect(results[180][0]).toEqual([1, 1]);
    expect(results[270][0]).toEqual([0, 1]);
    // round-trip: applying the forward map to the inverse restores identity
    const roundTrip = await page.evaluate(() => {
      const fwd = (x, y, d) => {
        // forward display position of an image point under CCW d
        if (d === 90) return { x: y, y: 1 - x };
        if (d === 180) return { x: 1 - x, y: 1 - y };
        if (d === 270) return { x: 1 - y, y: x };
        return { x, y };
      };
      for (const d of [0, 90, 180, 270]) {
        for (const [x, y] of [[0.1, 0.7], [0.9, 0.2]]) {
          const img = _computePreviewUnrotateFrac(x, y, d);
          const back = fwd(img.x, img.y, d);
          if (Math.abs(back.x - x) > 1e-12 || Math.abs(back.y - y) > 1e-12) return `fail d=${d}`;
        }
      }
      return 'ok';
    });
    expect(roundTrip).toBe('ok');
  });

  test('root pad preserves untouched tokens (F26)', async ({ page }) => {
    const out = await page.evaluate(() => {
      const rendered = _rootPadFormatCall([
        { re: 1.5, im: 0, raw: '1.5000' },          // untouched: keeps spelling
        { re: 2.5, im: 0.5, raw: null },            // moved: reformats
        { re: 0, im: -2, raw: '-2j' },              // untouched: keeps j suffix
      ]);
      return { rendered, fmtShort: _scrubFormatNumber(1.5) };
    });
    expect(out.rendered).toContain('1.5000');   // original token intact
    expect(out.rendered).toContain('2.5+0.5i'); // only the moved root reformats
    expect(out.rendered).toContain('-2j');
    expect(out.fmtShort).toBe('1.5');
  });

  test('dismissed live rail cards stay dismissed (F27)', async ({ page }) => {
    const out = await page.evaluate(() => {
      _jobsRailJobs.length = 0;
      _jobsRailUpsert({ id: 'sheet:t1', kind: 'sheet', state: 'running',
                        startedAt: Date.now(), label: 'T1', jobId: 'j' });
      _jobsRailDismiss('sheet:t1');
      const afterDismiss = _jobsRailJobs.some(j => j.id === 'sheet:t1');
      // the poll tick that used to resurrect it
      _jobsRailUpsert({ id: 'sheet:t1', state: 'running', detail: '3/8 frames' });
      const afterPoll = _jobsRailJobs.some(j => j.id === 'sheet:t1');
      // terminal update retires the tombstone silently
      _jobsRailUpsert({ id: 'sheet:t1', state: 'done', detail: 'done' });
      const afterTerminal = _jobsRailJobs.some(j => j.id === 'sheet:t1');
      // a NEW run with the same id (fresh startedAt) shows again
      _jobsRailUpsert({ id: 'sheet:t1', kind: 'sheet', state: 'running',
                        startedAt: Date.now(), label: 'T1b', jobId: 'j' });
      const afterNewRun = _jobsRailJobs.some(j => j.id === 'sheet:t1');
      _jobsRailJobs.length = 0;
      return { afterDismiss, afterPoll, afterTerminal, afterNewRun };
    });
    expect(out).toEqual({
      afterDismiss: false, afterPoll: false,
      afterTerminal: false, afterNewRun: true,
    });
  });
});

test.describe('Round-4 resume + publish regressions', () => {
  test('redispatch relaunches every non-done worker; lease gates them (finding 1)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      window.lambdaPost = async (name, body, path) => {
        if (path === '/check-status') {
          return { errors: 0, results: [
            { task_id: 'sheet_tiles_s_g_w0', phase: 'done', frame: 2 },
            { task_id: 'sheet_tiles_s_g_w1', phase: 'sheet', frame: 1 },   // running (maybe crashed)
            { task_id: 'sheet_tiles_s_g_w2', phase: 'accepted', frame: 0 },
          ] };
        }
        return { fired: 1 };
      };
      const desc = {
        sheetId: 's', jobId: 'j', generation: 'g',
        workers: [
          { task_id: 'sheet_tiles_s_g_w0', frames: [0] },
          { task_id: 'sheet_tiles_s_g_w1', frames: [1] },
          { task_id: 'sheet_tiles_s_g_w2', frames: [2] },
        ],
      };
      const pending = await _sheetWorkersNeedingDispatch(desc);
      return pending.map(w => w.task_id);
    });
    // round-6: every NON-DONE worker is redispatched (the server lease
    // claim no-ops a live one and reclaims a crashed one) — skipping
    // 'running' stranded crashed workers forever
    expect(out).toEqual(['sheet_tiles_s_g_w1', 'sheet_tiles_s_g_w2']);
  });

  test('unknown stitch state does not dispatch (finding 7)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      let stitchDispatches = 0;
      window.lambdaPost = async (name, body, path) => {
        if (path === '/check-status') throw new Error('network down');
        if (body && body.jobs && body.jobs[0] && body.jobs[0].action === 'stitch') {
          stitchDispatches += 1;
        }
        return { fired: 1 };
      };
      const phase = await _sheetTaskPhase('j', 'stitch_task');
      return { phase, stitchDispatches };
    });
    // 3 failed polls -> null (unknown); the caller must not guess/dispatch
    expect(out.phase).toBe(null);
    expect(out.stitchDispatches).toBe(0);
  });

  test('cancel reaches a persisted-but-inactive run (finding 4)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      _activeSheetRun = null;
      _sheetRunSave({ sheetId: 'ps', jobId: 'j', generation: 'gp', steps: 4,
                      workers: [], stitchTask: 't', payload: {} });
      let cancelled = null;
      window.lambdaPost = async (name, body) => {
        if (body.jobs && body.jobs[0] && body.jobs[0].action === 'cancel') {
          cancelled = body.jobs[0];
        }
        return { fired: 1 };
      };
      // round-8 finding 5: the descriptor is only cleared once the cancel
      // MARKER is confirmed durable — serve it so the confirm succeeds
      const realFetch = window.fetch;
      window.fetch = async (url) => (String(url).includes('cancel_')
        ? { ok: true } : realFetch(url));
      await cancelPolySheet();
      window.fetch = realFetch;
      const cleared = _sheetRunLoad() === null;
      return { cancelled, cleared };
    });
    expect(out.cancelled).toEqual({ action: 'cancel', sheet_id: 'ps', generation: 'gp' });
    expect(out.cleared).toBe(true);
  });
});

test.describe('Round-5 resume hardening', () => {
  test('reload during backoff reschedules a retry (finding 4)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      window.lambdaPost = async () => ({ fired: 1 });
      // a persisted descriptor mid-backoff, no live timer (simulates reload)
      _activeSheetRun = null;
      _sheetRunSave({ sheetId: 's', jobId: 'j', generation: 'g', steps: 4,
                      workers: [], stitchTask: 't', payload: {},
                      resumeAttempts: 1, nextResumeAt: Date.now() + 60000 });
      if (_sheetResumeTimer) { clearTimeout(_sheetResumeTimer); _sheetResumeTimer = null; }
      await resumeSheetRun();
      const scheduled = _sheetResumeTimer !== null;
      if (_sheetResumeTimer) { clearTimeout(_sheetResumeTimer); _sheetResumeTimer = null; }
      _sheetRunClear();
      return { scheduled };
    });
    // a timer is rebuilt from the persisted deadline
    expect(out.scheduled).toBe(true);
  });

  test('give-up abandons and discovery does not re-grab it (finding 5)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      let abandoned = null;
      window.lambdaPost = async (name, body) => {
        if (body && body.jobs && body.jobs[0] && body.jobs[0].action === 'abandon') {
          abandoned = body.jobs[0];
        }
        return { fired: 1 };
      };
      // round-9 finding 5: abandon clears local state only once run.json is
      // CONFIRMED terminal — serve it as abandoned so the confirm succeeds
      const realFetch = window.fetch;
      window.fetch = async (url) => (String(url).includes('run.json')
        ? { ok: true, json: async () => ({ generation: 'gg', status: 'abandoned' }) }
        : realFetch(url));
      _activeSheetRun = null;
      // round-8 finding 4: give-up now requires BOTH the attempt ceiling
      // AND a dispatch past the lease horizon — set the post-lease-
      // exhausted state (attempts >= max, first resume long ago, a
      // post-lease dispatch already recorded)
      _sheetRunSave({ sheetId: 'gv', jobId: 'j', generation: 'gg', steps: 4,
                      workers: [], stitchTask: 't', payload: {},
                      resumeAttempts: 6, firstResumeAt: Date.now() - 500000,
                      hadPostLeaseDispatch: true });
      await resumeSheetRun();
      window.fetch = realFetch;
      // the run is now abandoned locally + server dispatch sent + confirmed
      const cleared = _sheetRunLoad() === null;
      // discovery must skip a row for the just-abandoned generation
      _sheetsInventory = [];
      const before = _sheetRunLoad();
      await _sheetDiscoverServerRun([{ sheet_id: 'gv', run_status: 'running',
                                       run_generation: 'gg', run_key: 'sheets/gv/run.json' }]);
      const rediscovered = _sheetRunLoad() !== before;
      return { abandoned, cleared, rediscovered };
    });
    expect(out.abandoned).toEqual({ action: 'abandon', sheet_id: 'gv', generation: 'gg' });
    expect(out.cleared).toBe(true);
    expect(out.rediscovered).toBe(false);
  });
});

test.describe('Round-6 lease + reconciliation', () => {
  test('a published generation short-circuits the drive (finding 2)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      // run.json says THIS generation is published, but DDB has no done row
      const realFetch = window.fetch;
      window.fetch = async (url) => {
        if (String(url).includes('run.json')) {
          return { ok: true, json: async () => ({ published_generation: 'g6' }) };
        }
        return realFetch(url);
      };
      let generatedDeepZoom = false;
      window._sheetGenerateDeepZoom = async () => { generatedDeepZoom = true; };
      window.lambdaPost = async () => { throw new Error('DDB should not be polled'); };
      const desc = { sheetId: 's6', jobId: 'j', generation: 'g6', steps: 4,
                     workers: [], stitchTask: 't', payload: {} };
      let threw = false;
      try {
        await _sheetDriveRun(desc, null, { dispatchWorkers: false });
      } catch (e) { threw = true; }
      window.fetch = realFetch;
      return { threw, generatedDeepZoom };
    });
    // reconciled via run.json — no DDB poll, no throw, DeepZoom kicked off
    expect(out.threw).toBe(false);
    expect(out.generatedDeepZoom).toBe(true);
  });

  test('cancel guards the generation from rediscovery (finding 4)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      _activeSheetRun = null;
      _sheetRunSave({ sheetId: 'c6', jobId: 'j', generation: 'gc6', steps: 4,
                      workers: [], stitchTask: 't', payload: {} });
      window.lambdaPost = async () => ({ fired: 1 });
      // round-8 finding 5: confirm the cancel marker so the guard commits
      const realFetch = window.fetch;
      window.fetch = async (url) => (String(url).includes('cancel_')
        ? { ok: true } : realFetch(url));
      await cancelPolySheet();
      window.fetch = realFetch;
      _sheetsInventory = [];
      const before = _sheetRunLoad();
      await _sheetDiscoverServerRun([{ sheet_id: 'c6', run_status: 'running',
                                       run_generation: 'gc6', run_key: 'sheets/c6/run.json' }]);
      return { rediscovered: _sheetRunLoad() !== before };
    });
    expect(out.rediscovered).toBe(false);
  });
});

test.describe('Round-7 cancel confirmation', () => {
  test('cancel that is not accepted keeps the descriptor (finding 5)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      _activeSheetRun = null;
      _sheetRunSave({ sheetId: 'nc', jobId: 'j', generation: 'gnc', steps: 4,
                      workers: [], stitchTask: 't', payload: {} });
      // dispatch returns fired:0 (not accepted)
      window.lambdaPost = async () => ({ fired: 0 });
      await cancelPolySheet();
      // the run descriptor MUST survive an unaccepted cancel
      return { survived: _sheetRunLoad() !== null };
    });
    expect(out.survived).toBe(true);
  });

  test('accepted cancel clears the descriptor', async ({ page }) => {
    const out = await page.evaluate(async () => {
      _activeSheetRun = null;
      _sheetRunSave({ sheetId: 'ok', jobId: 'j', generation: 'gok', steps: 4,
                      workers: [], stitchTask: 't', payload: {} });
      window.lambdaPost = async () => ({ fired: 1 });
      // round-8 finding 5: accepted AND marker confirmed -> descriptor clears
      const realFetch = window.fetch;
      window.fetch = async (url) => (String(url).includes('cancel_')
        ? { ok: true } : realFetch(url));
      await cancelPolySheet();
      window.fetch = realFetch;
      return { cleared: _sheetRunLoad() === null };
    });
    expect(out.cleared).toBe(true);
  });

  test('accepted cancel whose marker is NOT visible keeps the descriptor (finding 5)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      _activeSheetRun = null;
      _sheetRunSave({ sheetId: 'unconf', jobId: 'j', generation: 'gunc', steps: 4,
                      workers: [], stitchTask: 't', payload: {} });
      window.lambdaPost = async () => ({ fired: 1 });   // accepted...
      const realFetch = window.fetch;
      window.fetch = async (url) => (String(url).includes('cancel_')
        ? { ok: false } : realFetch(url));               // ...but marker not visible
      await cancelPolySheet();
      window.fetch = realFetch;
      const survived = _sheetRunLoad() !== null;
      if (_sheetResumeTimer) { clearTimeout(_sheetResumeTimer); _sheetResumeTimer = null; }
      _sheetRunClear();
      // a fired-but-unconfirmed cancel must NOT hide/clear the run — the
      // resume watch keeps it alive until the marker lands
      return { survived };
    });
    expect(out.survived).toBe(true);
  });
});

test.describe('Round-8 fencing', () => {
  test('give-up does NOT abandon before the lease horizon (finding 4)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      let abandonDispatched = false;
      // record abandon dispatches; make the worker-status probe fail so the
      // drive short-circuits into the (non-abandon) reschedule path fast
      window.lambdaPost = async (name, body) => {
        if (body && body.jobs && body.jobs[0] && body.jobs[0].action === 'abandon') {
          abandonDispatched = true;
          return { fired: 1 };
        }
        throw new Error('no drive in this test');
      };
      _activeSheetRun = null;
      // attempts exhausted, but the FIRST resume was just now — the lease
      // could not have expired yet, so we must keep trying, not abandon
      _sheetRunSave({ sheetId: 'early', jobId: 'j', generation: 'gearly', steps: 4,
                      workers: [], stitchTask: 't', payload: {},
                      resumeAttempts: 6, firstResumeAt: Date.now(),
                      hadPostLeaseDispatch: false });
      await resumeSheetRun();
      const stillPersisted = _sheetRunLoad() !== null;
      if (_sheetResumeTimer) { clearTimeout(_sheetResumeTimer); _sheetResumeTimer = null; }
      _sheetRunClear();
      return { abandonDispatched, stillPersisted };
    });
    // before the lease horizon we must NOT abandon — a crashed worker's
    // lease is still live and no reclaim could have succeeded yet
    expect(out.abandonDispatched).toBe(false);
    // and the run is NOT dropped: the retry path keeps it under watch
    expect(out.stillPersisted).toBe(true);
  });

  test('unaccepted abandon keeps the descriptor and reschedules (finding 5)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      window.lambdaPost = async () => ({ fired: 0 });   // abandon never accepted
      _activeSheetRun = null;
      _sheetRunSave({ sheetId: 'ab', jobId: 'j', generation: 'gab', steps: 4,
                      workers: [], stitchTask: 't', payload: {},
                      resumeAttempts: 6, firstResumeAt: Date.now() - 500000,
                      hadPostLeaseDispatch: true });
      await resumeSheetRun();
      const survived = _sheetRunLoad() !== null;
      const rescheduled = _sheetResumeTimer !== null;
      if (_sheetResumeTimer) { clearTimeout(_sheetResumeTimer); _sheetResumeTimer = null; }
      _sheetRunClear();
      return { survived, rescheduled };
    });
    // an abandon the server never accepted must not silently drop — the
    // descriptor stays and a retry is scheduled
    expect(out.survived).toBe(true);
    expect(out.rescheduled).toBe(true);
  });
});

test.describe('Round-9 fencing', () => {
  test('a crashed stitch that reported progress is redispatched (finding 2)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      // the stitch phase is 'stitch' (it crashed mid-run AFTER reporting
      // progress, not 'accepted') — it must still be redispatched
      let stitchDispatched = false;
      window.lambdaPost = async (name, body, path) => {
        const job = body && body.jobs && body.jobs[0];
        if (path === '/check-status') {
          // workers all done; stitch task is mid-progress ('stitch')
          if (String(body.task_prefix || '').includes('stitch')) {
            return { errors: 0, results: [{ task_id: body.task_prefix, phase: 'stitch' }] };
          }
          return { errors: 0, results: [] };
        }
        if (job && job.action === 'stitch') { stitchDispatched = true; return { fired: 1 }; }
        return { fired: 1 };
      };
      // short-circuit the terminal poll so the test returns fast
      const realPoll = window._pollSheetTask;
      window._pollSheetTask = async () => ({ frames: 4, elapsed_ms: 1 });
      window._sheetGenerateDeepZoom = async () => {};
      const realPollWorkers = window._pollSheetWorkers;
      window._pollSheetWorkers = async () => {};
      const desc = { sheetId: 'st9', jobId: 'j', generation: 'gst9', steps: 4,
                     workers: [{ task_id: 'sheet_tiles_st9_gst9_w0', frames: [0, 1, 2, 3] }],
                     stitchTask: 'sheet_stitch_st9_gst9', payload: {}, startedAtS: 1 };
      await _sheetDriveRun(desc, null, { dispatchWorkers: false });
      window._pollSheetTask = realPoll;
      window._pollSheetWorkers = realPollWorkers;
      return { stitchDispatched };
    });
    // a non-'accepted' but non-terminal stitch state must be redispatched
    expect(out.stitchDispatched).toBe(true);
  });

  test('abandon fired but run.json not yet terminal keeps the descriptor (finding 5)', async ({ page }) => {
    const out = await page.evaluate(async () => {
      window.lambdaPost = async () => ({ fired: 1 });   // abandon accepted...
      // ...but run.json still reads 'running' (the hide has not landed)
      const realFetch = window.fetch;
      window.fetch = async (url) => (String(url).includes('run.json')
        ? { ok: true, json: async () => ({ generation: 'gnt', status: 'running' }) }
        : realFetch(url));
      _activeSheetRun = null;
      _sheetRunSave({ sheetId: 'nt', jobId: 'j', generation: 'gnt', steps: 4,
                      workers: [], stitchTask: 't', payload: {},
                      resumeAttempts: 6, firstResumeAt: Date.now() - 500000,
                      hadPostLeaseDispatch: true });
      await resumeSheetRun();
      window.fetch = realFetch;
      const survived = _sheetRunLoad() !== null;
      const rescheduled = _sheetResumeTimer !== null;
      if (_sheetResumeTimer) { clearTimeout(_sheetResumeTimer); _sheetResumeTimer = null; }
      _sheetRunClear();
      return { survived, rescheduled };
    });
    // an unconfirmed hide must not clear the run (a ghost would be
    // rediscovered) — it stays and retries
    expect(out.survived).toBe(true);
    expect(out.rescheduled).toBe(true);
  });
});
