const { test, expect } = require('@playwright/test');

// Scrub pad: dblclick a numeric literal in any program editor to drag-edit
// it. Ephemeral by contract: only the pad writes while open, any external
// edit closes it, Escape reverts. Live preview is lores-only (compute
// preview for pp/cp, render lores for rt/render-ss) — never the full
// pipeline.

async function openPadOnCoeffLiteral(page, source, literal) {
  return page.evaluate(({ source, literal }) => {
    const ta = document.getElementById('cp-source-text');
    ta.value = source;
    const pos = source.indexOf(literal);
    ta.selectionStart = ta.selectionEnd = pos + 1;
    _onProgramSourceDblClick('cp', { clientX: 300, clientY: 300 });
    return { padVisible: document.getElementById('program-scrub-pad').style.display === 'block' };
  }, { source, literal });
}

test.beforeEach(async ({ page }) => {
  await page.goto('http://localhost:8765/index.html');
  await page.waitForLoadState('domcontentloaded');
});

test.describe('Scrub pad', () => {
  test('dblclick on a numeric literal opens the pad; on a word opens help', async ({ page }) => {
    const opened = await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    expect(opened.padVisible).toBe(true);
    await expect(page.locator('#program-scrub-value')).toHaveText('2');

    const wordCase = await page.evaluate(() => {
      const ta = document.getElementById('cp-source-text');
      ta.selectionStart = ta.selectionEnd = ta.value.indexOf('linear') + 2;
      _onProgramSourceDblClick('cp', { clientX: 300, clientY: 300 });
      return {
        padVisible: document.getElementById('program-scrub-pad').style.display === 'block',
        helpVisible: document.getElementById('program-help-inspector').style.display === 'block',
      };
    });
    expect(wordCase.padVisible).toBe(false);
    expect(wordCase.helpVisible).toBe(true);
  });

  test('dragging scrubs the literal in place and runs validation', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    const after = await page.evaluate(() => {
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      // Range defaults to value ± max(1, |value|) = [0, 4] for value 2;
      // drag to the far right = max.
      _scrubPadDragStart({ clientX: rect.right, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
      return {
        text: document.getElementById('cp-source-text').value,
        readout: document.getElementById('program-scrub-value').textContent,
        status: document.getElementById('coeff-program-status').textContent,
      };
    });
    expect(after.text).toBe('poly = linear(poly, 4, 3)\nemit');
    expect(after.readout).toBe('4');
    expect(after.status).toContain('Text source changed');
  });

  test('editable range widens the scrub span', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    const after = await page.evaluate(() => {
      document.getElementById('program-scrub-min').value = '0';
      document.getElementById('program-scrub-max').value = '100';
      _scrubPadSetRange();
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      _scrubPadDragStart({ clientX: rect.right, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
      return document.getElementById('cp-source-text').value;
    });
    expect(after).toBe('poly = linear(poly, 100, 3)\nemit');
  });

  test('Escape reverts to the original literal and closes the pad', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    await page.evaluate(() => {
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      _scrubPadDragStart({ clientX: rect.right, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
    });
    await page.keyboard.press('Escape');
    const after = await page.evaluate(() => ({
      text: document.getElementById('cp-source-text').value,
      padVisible: document.getElementById('program-scrub-pad').style.display === 'block',
    }));
    expect(after.text).toBe('poly = linear(poly, 2, 3)\nemit');
    expect(after.padVisible).toBe(false);
  });

  test('typing in the editor closes the pad (ephemeral binding)', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    await page.locator('#cp-source-text').focus();
    await page.keyboard.press('End');
    await page.keyboard.type('x');
    await expect.poll(async () => page.evaluate(() =>
      document.getElementById('program-scrub-pad').style.display
    )).not.toBe('block');
  });

  test('live preview routes to the lores endpoint only, never the full pipeline', async ({ page }) => {
    await page.evaluate(() => {
      window._previewCalls = 0;
      window._fullPipelineCalls = 0;
      window.runComputePreview = async () => { window._previewCalls += 1; };
      window.runRasterPipeline = async () => { window._fullPipelineCalls += 1; };
    });
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    await page.evaluate(() => {
      document.getElementById('program-scrub-live').checked = true;
      _scrubPadToggleLive(true);
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      _scrubPadDragStart({ clientX: rect.right, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
    });
    await expect.poll(async () => page.evaluate(() => window._previewCalls), { timeout: 3000 }).toBeGreaterThan(0);
    expect(await page.evaluate(() => window._fullPipelineCalls)).toBe(0);
  });

  test('arrow keys nudge the value with range-relative steps', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    // Default range for 2 is [0, 4]; 1% step = 0.04, Shift = 0.4.
    await page.keyboard.press('ArrowRight');
    await expect(page.locator('#program-scrub-value')).toHaveText('2.04');
    await page.keyboard.press('Shift+ArrowRight');
    await expect(page.locator('#program-scrub-value')).toHaveText('2.44');
    await page.keyboard.press('ArrowLeft');
    expect(await page.evaluate(() => document.getElementById('cp-source-text').value)).toBe('poly = linear(poly, 2.4, 3)\nemit');
  });

  test('root editor scrubs static args and offers the render lores preview', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const opened = await page.evaluate(() => {
      const ta = document.getElementById('render-rt-source-text');
      ta.value = 'pull_unit_circle(0.75, 1)';
      ta.selectionStart = ta.selectionEnd = ta.value.indexOf('0.75') + 1;
      _onProgramSourceDblClick('rt', { clientX: 300, clientY: 300 });
      return {
        padVisible: document.getElementById('program-scrub-pad').style.display === 'block',
        padHtml: document.getElementById('program-scrub-pad').innerHTML,
      };
    });
    expect(opened.padVisible).toBe(true);
    expect(opened.padHtml).toContain('live render lores preview');
  });

  test('render-side pad offers the lores view picker and drives the tab state', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const opened = await page.evaluate(() => {
      const ta = document.getElementById('render-rt-source-text');
      ta.value = 'pull_unit_circle(0.75, 1)';
      ta.selectionStart = ta.selectionEnd = ta.value.indexOf('0.75') + 1;
      _onProgramSourceDblClick('rt', { clientX: 300, clientY: 300 });
      const select = document.getElementById('program-scrub-view');
      return { hasPicker: !!select, initial: select ? select.value : '' };
    });
    expect(opened.hasPicker).toBe(true);
    expect(opened.initial).toBe('plot');

    // Palette pane not available yet: choosing it records intent but the
    // tab machinery falls back to plot.
    const beforePalette = await page.evaluate(() => {
      _scrubPadSetView('palette');
      return {
        active: _renderLoresPreviewActiveTab,
        remembered: _scrubPadState.view,
      };
    });
    expect(beforePalette.active).toBe('plot');
    expect(beforePalette.remembered).toBe('palette');

    // Once a preview supplies a palette image, re-asserting the choice wins.
    const afterPalette = await page.evaluate(() => {
      _renderLoresPreviewHasPalette = true;
      _selectRenderLoresPreviewTab(_scrubPadState.view);
      return {
        active: _renderLoresPreviewActiveTab,
        tabActive: document.getElementById('render-lores-preview-tab-palette').classList.contains('active'),
      };
    });
    expect(afterPalette.active).toBe('palette');
    expect(afterPalette.tabActive).toBe(true);
  });

  test('palette view stays sticky across preview recalculations (no plot flash)', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      setColorMode('solve_score');
      document.getElementById('render-results-dir').value = 'job_sticky';
      const PNG = 'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==';
      window.lambdaPost = async function(name, body, path) {
        if (name === 'storage' && path === '/detail') {
          return { calc: { exists: true, degree: 5, n_coeffs: 6, N: 100, lores: { bin_key: 'k', coeffs_key: 'c', params_key: 'p' } } };
        }
        if (name === 'render-lores-preview') {
          return {
            image_base64: PNG,
            content_type: 'image/png',
            palette_image_base64: PNG,
            palette_content_type: 'image/png',
            palette_pix: 1,
            emission_histograms: [],
            raster: { roots_plotted: 10 },
            timings_ms: { total: 5 },
            source: { mode: 'lores' },
            nonzero_pixels: 1,
            logs: [],
            min_re: -1, max_re: 1, min_im: -1, max_im: 1, pix: 1,
          };
        }
        return {};
      };
      const orig = _selectRenderLoresPreviewTab;
      window._tabResolutions = [];
      _selectRenderLoresPreviewTab = function(tab, opts) {
        orig(tab, opts);
        window._tabResolutions.push(_renderLoresPreviewActiveTab);
      };
    });

    await page.evaluate(async () => { await runRenderLoresPreview(); });
    await page.click('#render-lores-preview-tab-palette');
    expect(await page.evaluate(() => _renderLoresPreviewActiveTab)).toBe('palette');

    // Open the pad on the root source; it adopts the active view.
    const padView = await page.evaluate(() => {
      const ta = document.getElementById('render-rt-source-text');
      ta.value = 'rotate_roots(0.25)';
      ta.selectionStart = ta.selectionEnd = ta.value.indexOf('0.25') + 1;
      _onProgramSourceDblClick('rt', { clientX: 300, clientY: 300 });
      return document.getElementById('program-scrub-view').value;
    });
    expect(padView).toBe('palette');

    // A second preview run (a scrub tick) must never resolve to plot.
    const after = await page.evaluate(async () => {
      window._tabResolutions = [];
      await runRenderLoresPreview();
      return {
        resolutions: window._tabResolutions.slice(),
        active: _renderLoresPreviewActiveTab,
        paletteVisible: document.getElementById('render-lores-preview-tab-palette').style.display !== 'none',
        paletteActive: document.getElementById('render-lores-preview-tab-palette').classList.contains('active'),
      };
    });
    expect(after.active).toBe('palette');
    expect(after.paletteVisible).toBe(true);
    expect(after.paletteActive).toBe(true);
    // Non-vacuous: the run path must still route through the selector at
    // least once, and every resolution during the run must be palette.
    expect(after.resolutions.length).toBeGreaterThan(0);
    expect(after.resolutions.every(t => t === 'palette')).toBe(true);
    // The inline status beside Preview is a single short word; the detail
    // line lives in the render log (row height must never change).
    expect(await page.evaluate(() => document.getElementById('render-lores-preview-status').textContent)).toBe('done');

    // Error path: short word inline, full message in the log.
    const errored = await page.evaluate(async () => {
      const prev = window.lambdaPost;
      window.lambdaPost = async (name, body, path) => {
        if (name === 'render-lores-preview') throw new Error('boom for status test');
        return prev(name, body, path);
      };
      await runRenderLoresPreview();
      window.lambdaPost = prev;
      return {
        status: document.getElementById('render-lores-preview-status').textContent,
        logHasDetail: document.getElementById('render-log').textContent.includes('Render preview failed: boom for status test'),
      };
    });
    expect(errored.status).toBe('error');
    expect(errored.logHasDetail).toBe(true);
  });

  test('clicking a preview tab while the pad is open updates the pad view intent', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    await page.evaluate(() => {
      _renderLoresPreviewHasPalette = true;
      document.getElementById('render-lores-preview-tab-palette').style.display = '';
      const ta = document.getElementById('render-rt-source-text');
      ta.value = 'rotate_roots(0.25)';
      ta.selectionStart = ta.selectionEnd = ta.value.indexOf('0.25') + 1;
      _onProgramSourceDblClick('rt', { clientX: 300, clientY: 300 });
      _scrubPadSetView('palette');
    });
    await page.evaluate(() => { _selectRenderLoresPreviewTab('plot'); });
    const synced = await page.evaluate(() => ({
      intent: _scrubPadState.view,
      select: document.getElementById('program-scrub-view').value,
    }));
    expect(synced.intent).toBe('plot');
    expect(synced.select).toBe('plot');
  });

  test('dblclick a metric name opens the discrete metric scrubber', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const opened = await page.evaluate(() => {
      const ta = document.getElementById('render-ss-source-text');
      ta.value = 'score = metric(proximity, slv, q=0.1%)\n';
      ta.selectionStart = ta.selectionEnd = ta.value.indexOf('proximity') + 2;
      _onProgramSourceDblClick('render-ss', { clientX: 300, clientY: 300 });
      return {
        padVisible: document.getElementById('program-scrub-pad').style.display === 'block',
        title: document.getElementById('program-scrub-pad').innerHTML.includes('Metric'),
        hasRange: !!document.getElementById('program-scrub-min'),
        desc: document.getElementById('program-scrub-desc').textContent,
        pos: document.getElementById('program-scrub-pos').textContent,
        value: document.getElementById('program-scrub-value').textContent,
      };
    });
    expect(opened.padVisible).toBe(true);
    expect(opened.title).toBe(true);
    expect(opened.hasRange).toBe(false);
    expect(opened.value).toBe('proximity');
    expect(opened.desc).toContain('Near-collision detector');
    expect(opened.desc).toContain('Sources:');
    expect(opened.pos).toMatch(/^\d+\/41$/);

    // Arrow steps to the neighboring metric and the description follows.
    await page.keyboard.press('ArrowRight');
    const stepped = await page.evaluate(() => ({
      text: document.getElementById('render-ss-source-text').value,
      value: document.getElementById('program-scrub-value').textContent,
      desc: document.getElementById('program-scrub-desc').textContent,
    }));
    expect(stepped.text).not.toContain('metric(proximity');
    expect(stepped.text).toContain(`metric(${stepped.value},`);
    expect(stepped.desc.length).toBeGreaterThan(20);

    // Drag to the far right lands on the last metric in the vocabulary.
    const dragged = await page.evaluate(() => {
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      _scrubPadDragStart({ clientX: rect.right, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
      return {
        value: document.getElementById('program-scrub-value').textContent,
        last: _solveScoreMetricNames[_solveScoreMetricNames.length - 1],
        text: document.getElementById('render-ss-source-text').value,
      };
    });
    expect(dragged.value).toBe(dragged.last);
    expect(dragged.text).toContain(`metric(${dragged.last},`);

    // Escape restores the original metric.
    await page.keyboard.press('Escape');
    expect(await page.evaluate(() => document.getElementById('render-ss-source-text').value))
      .toBe('score = metric(proximity, slv, q=0.1%)\n');
  });

  test('metric names outside the solve-score editors are not scrub targets', async ({ page }) => {
    const opened = await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    expect(opened.padVisible).toBe(true);
    const wordCase = await page.evaluate(() => {
      const ta = document.getElementById('cp-source-text');
      ta.value = 'poly = proximity\nemit';
      ta.selectionStart = ta.selectionEnd = ta.value.indexOf('proximity') + 2;
      _onProgramSourceDblClick('cp', { clientX: 300, clientY: 300 });
      return {
        padVisible: document.getElementById('program-scrub-pad').style.display === 'block',
        helpVisible: document.getElementById('program-help-inspector').style.display === 'block',
      };
    });
    expect(wordCase.padVisible).toBe(false);
    expect(wordCase.helpVisible).toBe(true);
  });

  test('the Help tab explains metrics with real descriptions', async ({ page }) => {
    await page.click('.tab-btn:text("Render")');
    const help = await page.evaluate(() => {
      _setProgramSourceSidePanelMode('render-ss', 'help');
      return document.getElementById('render-ss-help').innerHTML;
    });
    expect(help).toContain('Near-collision detector');
    expect(help).toContain('Thin-shell detector');
    expect(help).toContain('rotational symmetry strength');
  });

  test('compute-side pads have no lores view picker', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    expect(await page.evaluate(() => !!document.getElementById('program-scrub-view'))).toBe(false);
  });
});

test.describe('Scrub pad 2D (complex literals)', () => {
  test('dblclick a complex literal opens the 2D pad; drag writes both parts', async ({ page }) => {
    const opened = await openPadOnCoeffLiteral(page, 'poly[0] = 1.5+0.3i\nemit', '1.5');
    expect(opened.padVisible).toBe(true);
    await expect(page.locator('#program-scrub-surface')).toHaveClass(/program-scrub-surface-2d/);
    await expect(page.locator('#program-scrub-value')).toHaveText('1.5+0.3i');

    const after = await page.evaluate(() => {
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      // window is value ± span (span = max(1, |re|, |im|) = 1.5):
      // top-right corner = (re + span, im + span) = 3+1.8i
      _scrubPadDragStart({ clientX: rect.right, clientY: rect.top, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
      return {
        text: document.getElementById('cp-source-text').value,
        state: { re: _scrubPadState.re, im: _scrubPadState.im },
      };
    });
    expect(after.text).toContain('3+1.8i');
    expect(after.state.re).toBeCloseTo(3, 6);
    expect(after.state.im).toBeCloseTo(1.8, 6);

    // squiggle continues: second point mid-left = (re - span, im center)
    const second = await page.evaluate(() => {
      const surface = document.getElementById('program-scrub-surface');
      const rect = surface.getBoundingClientRect();
      _scrubPadDragStart({ clientX: rect.left, clientY: rect.top + rect.height / 2, preventDefault() {} });
      document.dispatchEvent(new PointerEvent('pointerup'));
      return document.getElementById('cp-source-text').value;
    });
    expect(second).toContain('0+0.3i');

    // Escape reverts the whole squiggle to the original literal
    await page.keyboard.press('Escape');
    const reverted = await page.evaluate(() => document.getElementById('cp-source-text').value);
    expect(reverted).toContain('1.5+0.3i');
  });

  test('pure-imaginary literals get the 2D pad; plain reals keep the 1D pad', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly[0] = 2i\nemit', '2i');
    await expect(page.locator('#program-scrub-surface')).toHaveClass(/program-scrub-surface-2d/);
    await expect(page.locator('#program-scrub-value')).toHaveText('2i');

    await openPadOnCoeffLiteral(page, 'poly = linear(poly, 2, 3)\nemit', '2,');
    const cls = await page.getAttribute('#program-scrub-surface', 'class');
    expect(cls).not.toContain('program-scrub-surface-2d');
  });

  test('the pad header drags the popup and the position sticks', async ({ page }) => {
    await openPadOnCoeffLiteral(page, 'poly[0] = 1.5+0.3i\nemit', '1.5');
    const moved = await page.evaluate(() => {
      const el = document.getElementById('program-scrub-pad');
      const head = el.querySelector('.program-scrub-head');
      const rect = el.getBoundingClientRect();
      _scrubPadHeadDragStart({ clientX: rect.left + 20, clientY: rect.top + 5, preventDefault() {}, target: head });
      document.dispatchEvent(new PointerEvent('pointermove', { clientX: rect.left + 220, clientY: rect.top + 105 }));
      document.dispatchEvent(new PointerEvent('pointerup'));
      return { left: el.style.left, top: el.style.top };
    });
    expect(parseInt(moved.left, 10)).toBeGreaterThan(150);
    expect(parseInt(moved.top, 10)).toBeGreaterThan(90);

    // reopening prefers the dragged position
    const reopened = await page.evaluate(() => {
      _closeProgramScrubPad();
      const ta = document.getElementById('cp-source-text');
      ta.selectionStart = ta.selectionEnd = ta.value.indexOf('1.5') + 1;
      _onProgramSourceDblClick('cp', { clientX: 10, clientY: 10 });
      return document.getElementById('program-scrub-pad').style.left;
    });
    expect(parseInt(reopened, 10)).toBeGreaterThan(150);
  });
});

test.describe('Root pad (roots_literal geometry)', () => {
  const CALL = 'roots_literal(\n    1,\n    2i,\n    -7.5+2j\n)';
  const SRC = CALL + '\npoly = blend(0.5)\nemit';

  test('dblclick the identifier opens the canvas pad and lists the roots', async ({ page }) => {
    const opened = await openPadOnCoeffLiteral(page, SRC, 'roots_literal');
    expect(opened.padVisible).toBe(true);
    await expect(page.locator('#program-scrub-canvas')).toBeVisible();
    await expect(page.locator('#program-scrub-value')).toHaveText('3 roots');
    await expect(page.locator('#program-scrub-snap')).toBeChecked();
  });

  test('dragging a point snaps to the 0.5 grid and rewrites the whole call in place', async ({ page }) => {
    await openPadOnCoeffLiteral(page, SRC, 'roots_literal');
    const after = await page.evaluate(() => {
      const st = _scrubPadState;
      const canvas = document.getElementById('program-scrub-canvas');
      const rect = canvas.getBoundingClientRect();
      const hit = {
        clientX: rect.left + st.plane.toX(st.roots[0].re),
        clientY: rect.top + st.plane.toY(st.roots[0].im),
        preventDefault() {},
      };
      _rootPadDragStart(hit);
      _rootPadDragMove({
        clientX: rect.left + st.plane.toX(2.1),
        clientY: rect.top + st.plane.toY(0.43),
      });
      document.dispatchEvent(new PointerEvent('pointerup'));
      return {
        text: document.getElementById('cp-source-text').value,
        readout: document.getElementById('program-scrub-value').textContent,
        active: st.activeRoot,
        root0: { ...st.roots[0] },
      };
    });
    expect(after.active).toBe(0);
    expect(after.root0).toEqual({ re: 2, im: 0.5 });
    expect(after.text.startsWith('roots_literal(\n    2+0.5i,\n    2i,\n    -7.5+2i\n)')).toBe(true);
    expect(after.text.endsWith('poly = blend(0.5)\nemit')).toBe(true);
    expect(after.readout).toBe('3 roots · 2+0.5i');
  });

  test('expression args refuse the pad; help opens instead', async ({ page }) => {
    const opened = await openPadOnCoeffLiteral(page, 'roots_literal(1, exp(1))\nemit', 'roots_literal');
    expect(opened.padVisible).toBe(false);
    const helpVisible = await page.evaluate(() =>
      document.getElementById('program-help-inspector').style.display === 'block');
    expect(helpVisible).toBe(true);
  });

  test('the window input reframes to a zero-centered square of side d', async ({ page }) => {
    await openPadOnCoeffLiteral(page, SRC, 'roots_literal');
    const initial = await page.evaluate(() => ({
      shown: document.getElementById('program-scrub-window').value,
      half: _scrubPadState.plane.half,
    }));
    expect(Number(initial.shown)).toBeCloseTo(2 * initial.half, 6);
    const after = await page.evaluate(() => {
      document.getElementById('program-scrub-window').value = '30';
      _rootPadSetWindow();
      const st = _scrubPadState;
      // drag root 1 (2i) under the new frame to 5-5i, snapped
      const canvas = document.getElementById('program-scrub-canvas');
      const rect = canvas.getBoundingClientRect();
      _rootPadDragStart({
        clientX: rect.left + st.plane.toX(st.roots[1].re),
        clientY: rect.top + st.plane.toY(st.roots[1].im),
        preventDefault() {},
      });
      _rootPadDragMove({
        clientX: rect.left + st.plane.toX(5.1),
        clientY: rect.top + st.plane.toY(-4.9),
      });
      document.dispatchEvent(new PointerEvent('pointerup'));
      return {
        cRe: st.plane.cRe, cIm: st.plane.cIm, half: st.plane.half,
        root1: { ...st.roots[1] },
        text: document.getElementById('cp-source-text').value,
      };
    });
    expect(after.cRe).toBe(0);
    expect(after.cIm).toBe(0);
    expect(after.half).toBe(15);
    expect(after.root1).toEqual({ re: 5, im: -5 });
    expect(after.text).toContain('5-5i,');
  });

  test('Escape reverts the entire call after a drag', async ({ page }) => {
    await openPadOnCoeffLiteral(page, SRC, 'roots_literal');
    await page.evaluate(() => {
      const st = _scrubPadState;
      const canvas = document.getElementById('program-scrub-canvas');
      const rect = canvas.getBoundingClientRect();
      _rootPadDragStart({
        clientX: rect.left + st.plane.toX(st.roots[1].re),
        clientY: rect.top + st.plane.toY(st.roots[1].im),
        preventDefault() {},
      });
      _rootPadDragMove({
        clientX: rect.left + st.plane.toX(-3),
        clientY: rect.top + st.plane.toY(-1),
      });
      document.dispatchEvent(new PointerEvent('pointerup'));
    });
    await page.keyboard.press('Escape');
    const text = await page.evaluate(() => document.getElementById('cp-source-text').value);
    expect(text).toBe(SRC);
  });
});

test.describe('Integer-context scrubbing', () => {
  test('the roots_ascii_literal code (and floor args) scrub in whole-number steps', async ({ page }) => {
    const opened = await openPadOnCoeffLiteral(
      page, 'poly = roots_ascii_literal(floor(11.2506))\nemit', '11.2506');
    expect(opened.padVisible).toBe(true);
    await expect(page.locator('.program-scrub-hint')).toContainText('integer steps');
    const after = await page.evaluate(() => {
      _scrubPadWrite(13.37);
      _scrubPadNudge(1, false);
      return document.getElementById('cp-source-text').value;
    });
    expect(after).toBe('poly = roots_ascii_literal(floor(14))\nemit');
  });
});
