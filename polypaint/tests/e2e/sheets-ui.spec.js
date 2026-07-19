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
